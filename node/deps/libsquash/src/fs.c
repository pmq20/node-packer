/*
 * Copyright (c) 2012 Dave Vasilevsky <dave@vasilevsky.ca>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "squash/fs.h"

#include "squash/file.h"


#include <stdlib.h>
#include <string.h>



#define DATA_CACHED_BLKS 1
#define FRAG_CACHED_BLKS 3

void sqfs_version_supported(int *min_major, int *min_minor, int *max_major,
		int *max_minor) {
	*min_major = *max_major = SQUASHFS_MAJOR;
	*min_minor = 0;
	*max_minor = SQUASHFS_MINOR;
}

void sqfs_version(sqfs *fs, int *major, int *minor) {
	*major = fs->sb->s_major;
	*minor = fs->sb->s_minor;
}

sqfs_compression_type sqfs_compression(sqfs *fs) {
	return fs->sb->compression;
}

sqfs_err sqfs_init(sqfs *fs, sqfs_fd_t fd, size_t offset) {
	sqfs_err err;
	memset(fs, 0, sizeof(*fs));
	
	fs->fd = fd;
	fs->offset = offset;

	fs->sb = (struct squashfs_super_block *)(fd + fs->offset);
	
	if (fs->sb->s_magic != SQUASHFS_MAGIC) {
		return SQFS_BADFORMAT;
	}
	if (fs->sb->s_major != SQUASHFS_MAJOR || fs->sb->s_minor > SQUASHFS_MINOR)
		return SQFS_BADVERSION;
	
	if (!(fs->decompressor = sqfs_decompressor_get(fs->sb->compression)))
		return SQFS_BADCOMP;
	
	err = sqfs_table_init(&fs->id_table, fd, fs->sb->id_table_start + fs->offset,
		sizeof(uint32_t), fs->sb->no_ids);
	err |= sqfs_table_init(&fs->frag_table, fd, fs->sb->fragment_table_start + fs->offset,
		sizeof(struct squashfs_fragment_entry), fs->sb->fragments);
	if (sqfs_export_ok(fs)) {
		err |= sqfs_table_init(&fs->export_table, fd, fs->sb->lookup_table_start + fs->offset,
			sizeof(uint64_t), fs->sb->inodes);
	}
	err |= sqfs_block_cache_init(&fs->md_cache, SQUASHFS_CACHED_BLKS);
	err |= sqfs_block_cache_init(&fs->data_cache, DATA_CACHED_BLKS);
	err |= sqfs_block_cache_init(&fs->frag_cache, FRAG_CACHED_BLKS);
	err |= sqfs_blockidx_init(&fs->blockidx);
	if (err) {
		sqfs_destroy(fs);
		return SQFS_ERR;
	}
	
	return SQFS_OK;
}

void sqfs_destroy(sqfs *fs) {
	sqfs_table_destroy(&fs->id_table);
	sqfs_table_destroy(&fs->frag_table);
	if (sqfs_export_ok(fs))
		sqfs_table_destroy(&fs->export_table);
	sqfs_cache_destroy(&fs->md_cache);
	sqfs_cache_destroy(&fs->data_cache);
	sqfs_cache_destroy(&fs->frag_cache);
	sqfs_cache_destroy(&fs->blockidx);
}

void sqfs_md_header(uint16_t hdr, short *compressed, uint16_t *size) {
	*compressed = !(hdr & SQUASHFS_COMPRESSED_BIT);
	*size = hdr & ~SQUASHFS_COMPRESSED_BIT;
	if (!*size)
		*size = SQUASHFS_COMPRESSED_BIT;
}

void sqfs_data_header(uint32_t hdr, short *compressed, uint32_t *size) {
	*compressed = !(hdr & SQUASHFS_COMPRESSED_BIT_BLOCK);
	*size = hdr & ~SQUASHFS_COMPRESSED_BIT_BLOCK;
}

sqfs_err sqfs_block_read(sqfs *fs, sqfs_off_t pos, short compressed,
		uint32_t size, size_t outsize, sqfs_block **block) {
	sqfs_err err = SQFS_ERR;
	if (!(*block = malloc(sizeof(**block))))
		return SQFS_ERR;
	
	(*block)->data = (void *)((fs->fd) + (pos + fs->offset));
	(*block)->data_need_freeing = 0;

	if (compressed) {
		char *decomp = malloc(outsize);
		if (!decomp)
			goto error;
		
		err = fs->decompressor((*block)->data, size, decomp, &outsize);
		if (err) {
			free(decomp);
			goto error;
		}
		(*block)->data = decomp;
		(*block)->size = outsize;
		(*block)->data_need_freeing = 1;
	} else {
		(*block)->size = size;
	}

	return SQFS_OK;

error:
	sqfs_block_dispose(*block);
	*block = NULL;
	return err;
}

sqfs_err sqfs_md_block_read(sqfs *fs, sqfs_off_t pos, size_t *data_size,
		sqfs_block **block) {
	sqfs_err err;
	uint16_t hdr;
	short compressed;
	uint16_t size;
	
	*data_size = 0;
	
	hdr = *(uint16_t *)(fs->fd + pos + fs->offset);
	pos += sizeof(hdr);
	*data_size += sizeof(hdr);
	
	sqfs_md_header(hdr, &compressed, &size);
	
	err = sqfs_block_read(fs, pos, compressed, size,
		SQUASHFS_METADATA_SIZE, block);
	*data_size += size;
	return err;
}

sqfs_err sqfs_data_block_read(sqfs *fs, sqfs_off_t pos, uint32_t hdr,
		sqfs_block **block) {
	short compressed;
	uint32_t size;
	sqfs_data_header(hdr, &compressed, &size);
	return sqfs_block_read(fs, pos, compressed, size,
		fs->sb->block_size, block);
}

sqfs_err sqfs_md_cache(sqfs *fs, sqfs_off_t *pos, sqfs_block **block) {
	sqfs_block_cache_entry *entry;
	sqfs_err ret;
	MUTEX_LOCK(&fs->md_cache.mutex);

	entry = sqfs_cache_get(&fs->md_cache, *pos);
	if (!entry) {
		entry = sqfs_cache_add(&fs->md_cache, *pos);
		ret = sqfs_md_block_read(fs, *pos, &entry->data_size, &entry->block);
		if (ret) {
			goto exit;
		}
	}
	*block = entry->block;
	*pos += entry->data_size;
	ret = SQFS_OK;
exit:
	MUTEX_UNLOCK(&fs->md_cache.mutex);
	return ret;
}

sqfs_err sqfs_data_cache(sqfs *fs, sqfs_cache *cache, sqfs_off_t pos,
		uint32_t hdr, sqfs_block **block) {
	sqfs_block_cache_entry *entry;
	sqfs_err ret;
	MUTEX_LOCK(&cache->mutex);

	entry = sqfs_cache_get(cache, pos);
	if (!entry) {
		entry = sqfs_cache_add(cache, pos);
		ret = sqfs_data_block_read(fs, pos, hdr, &entry->block);
		if (ret) {
			goto exit;
		}
	}
	*block = entry->block;
	ret = SQFS_OK;
exit:
	MUTEX_UNLOCK(&cache->mutex);
	return ret;
}

void sqfs_block_dispose(sqfs_block *block) {
	if (block->data_need_freeing) {
		free(block->data);
	}
	free(block);
}

void sqfs_md_cursor_inode(sqfs_md_cursor *cur, sqfs_inode_id id, sqfs_off_t base) {
	cur->block = (id >> 16) + base;
	cur->offset = id & 0xffff;
}

sqfs_err sqfs_md_read(sqfs *fs, sqfs_md_cursor *cur, void *buf, size_t size) {
	sqfs_off_t pos = cur->block;
	while (size > 0) {
		sqfs_block *block;
		size_t take;
		sqfs_err err = sqfs_md_cache(fs, &pos, &block);
		if (err)
			return err;
		
		take = block->size - cur->offset;
		if (take > size)
			take = size;
		if (buf)
			memcpy(buf, (char*)block->data + cur->offset, take);
		/* BLOCK CACHED, DON'T DISPOSE */
		
		if (buf)
			buf = (char*)buf + take;
		size -= take;
		cur->offset += take;
		if (cur->offset == block->size) {
			cur->block = pos;
			cur->offset = 0;
		}
	}
	return SQFS_OK;
}

size_t sqfs_divceil(uint64_t total, size_t group) {
	size_t q = (size_t)(total / group);
	if (total % group)
		q += 1;
	return q;
}

sqfs_err sqfs_id_get(sqfs *fs, uint16_t idx, sqfs_id_t *id) {
	uint32_t rid;
	sqfs_err err = sqfs_table_get(&fs->id_table, fs, idx, &rid);
	if (err)
		return err;
	*id = (sqfs_id_t)rid;
	return SQFS_OK;
}

sqfs_err sqfs_readlink(sqfs *fs, sqfs_inode *inode, char *buf, size_t *size) {
	size_t want;
	sqfs_md_cursor cur;
	sqfs_err err;
	if (!S_ISLNK(inode->base.mode))
		return SQFS_ERR;

	want = inode->xtra.symlink_size;
	if (!buf) {
		*size = want + 1;
		return SQFS_OK;
	}

	if (want > *size - 1)
		want = *size - 1;
	cur = inode->next;
	err = sqfs_md_read(fs, &cur, buf, want);
	buf[want] = '\0';
	return err;
}

int sqfs_export_ok(sqfs *fs) {
	return fs->sb->lookup_table_start != SQUASHFS_INVALID_BLK;
}

sqfs_err sqfs_export_inode(sqfs *fs, sqfs_inode_num n, sqfs_inode_id *i) {
	uint64_t r;
	sqfs_err err;
	
	if (!sqfs_export_ok(fs))
		return SQFS_UNSUP;
	
	err = sqfs_table_get(&fs->export_table, fs, n - 1, &r);
	if (err)
		return err;
	*i = r;
	return SQFS_OK;
}

sqfs_inode_id sqfs_inode_root(sqfs *fs) {
	return fs->sb->root_inode;
}

/* Turn the internal format of a device number to our system's dev_t
 * It looks like rdev is just what the Linux kernel uses: 20 bit minor,
 * split in two around a 12 bit major
 */
static void sqfs_decode_dev(sqfs_inode *i, uint32_t rdev) {
	i->xtra.dev.major = (rdev >> 8) & 0xfff;
	i->xtra.dev.minor = (rdev & 0xff) | ((rdev >> 12) & 0xfff00);
}

#define INODE_TYPE(_type) \
	struct squashfs_##_type##_inode x; \
	err = sqfs_md_read(fs, &inode->next, &x, sizeof(x)); \
	if (err) return err

sqfs_err sqfs_inode_get(sqfs *fs, sqfs_inode *inode, sqfs_inode_id id) {
	sqfs_md_cursor cur;
	sqfs_err err;
	
	memset(inode, 0, sizeof(*inode));
	
	sqfs_md_cursor_inode(&cur, id, fs->sb->inode_table_start);
	inode->next = cur;
	
	err = sqfs_md_read(fs, &cur, &inode->base, sizeof(inode->base));
	if (err)
		return err;
	
	inode->base.mode |= sqfs_mode(inode->base.inode_type);
	switch (inode->base.inode_type) {
		case SQUASHFS_REG_TYPE: {
			INODE_TYPE(reg);
			inode->nlink = 1;
			inode->xtra.reg.start_block = x.start_block;
			inode->xtra.reg.file_size = x.file_size;
			inode->xtra.reg.frag_idx = x.fragment;
			inode->xtra.reg.frag_off = x.offset;
			break;
		}
		case SQUASHFS_LREG_TYPE: {
			INODE_TYPE(lreg);
			inode->nlink = x.nlink;
			inode->xtra.reg.start_block = x.start_block;
			inode->xtra.reg.file_size = x.file_size;
			inode->xtra.reg.frag_idx = x.fragment;
			inode->xtra.reg.frag_off = x.offset;
			break;
		}
		case SQUASHFS_DIR_TYPE: {
			INODE_TYPE(dir);
			inode->nlink = x.nlink;
			inode->xtra.dir.start_block = x.start_block;
			inode->xtra.dir.offset = x.offset;
			inode->xtra.dir.dir_size = x.file_size;
			inode->xtra.dir.idx_count = 0;
			inode->xtra.dir.parent_inode = x.parent_inode;
			break;
		}
		case SQUASHFS_LDIR_TYPE: {
			INODE_TYPE(ldir);
			inode->nlink = x.nlink;
			inode->xtra.dir.start_block = x.start_block;
			inode->xtra.dir.offset = x.offset;
			inode->xtra.dir.dir_size = x.file_size;
			inode->xtra.dir.idx_count = x.i_count;
			inode->xtra.dir.parent_inode = x.parent_inode;
			break;
		}
		case SQUASHFS_SYMLINK_TYPE:
		case SQUASHFS_LSYMLINK_TYPE: {
			INODE_TYPE(symlink);
			inode->nlink = x.nlink;
			inode->xtra.symlink_size = x.symlink_size;
			
			if (inode->base.inode_type == SQUASHFS_LSYMLINK_TYPE) {
				/* skip symlink target */
				cur = inode->next;
				err = sqfs_md_read(fs, &cur, NULL, inode->xtra.symlink_size);
				if (err)
					return err;
			}
			break;
		}
		case SQUASHFS_BLKDEV_TYPE:
		case SQUASHFS_CHRDEV_TYPE: {
			INODE_TYPE(dev);
			inode->nlink = x.nlink;
			sqfs_decode_dev(inode, x.rdev);
			break;
		}
		case SQUASHFS_LBLKDEV_TYPE:
		case SQUASHFS_LCHRDEV_TYPE: {
			INODE_TYPE(ldev);
			inode->nlink = x.nlink;
			sqfs_decode_dev(inode, x.rdev);
			break;
		}
		case SQUASHFS_SOCKET_TYPE:
		case SQUASHFS_FIFO_TYPE: {
			INODE_TYPE(ipc);
			inode->nlink = x.nlink;
			break;
		}
		case SQUASHFS_LSOCKET_TYPE:
		case SQUASHFS_LFIFO_TYPE: {
			INODE_TYPE(lipc);
			inode->nlink = x.nlink;
			break;
		}
		
		default: return SQFS_ERR;
	}
	
	return SQFS_OK;
}
#undef INODE_TYPE
