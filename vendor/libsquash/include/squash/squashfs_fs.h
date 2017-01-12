/*
 * squashfs_fs.h
 *
 * Copyright (c) 2012  Phillip Lougher <phillip@squashfs.org.uk>
 *
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
#ifndef SQUASHFS_FS
#define SQUASHFS_FS

#ifdef __linux__
	#include <linux/types.h>
#else
	#include <stdint.h>
	typedef uint16_t __le16;
	typedef uint32_t __le32;
	typedef uint64_t __le64;
#endif

#define SQUASHFS_MAGIC 0x73717368


#define SQUASHFS_CACHED_FRAGMENTS	CONFIG_SQUASHFS_FRAGMENT_CACHE_SIZE
#define SQUASHFS_MAJOR			4
#define SQUASHFS_MINOR			0
#define SQUASHFS_START			0

/* size of metadata (inode and directory) blocks */
#define SQUASHFS_METADATA_SIZE		8192
#define SQUASHFS_METADATA_LOG		13

/* default size of data blocks */
#define SQUASHFS_FILE_SIZE		131072
#define SQUASHFS_FILE_LOG		17

#define SQUASHFS_FILE_MAX_SIZE		1048576
#define SQUASHFS_FILE_MAX_LOG		20

/* Max number of uids and gids */
#define SQUASHFS_IDS			65536

/* Max length of filename (not 255) */
#define SQUASHFS_NAME_LEN		256
#define SQUASHFS_PATH_LEN		2048
#define SQUASHFS_MAX_LINK_LEVEL	32
#define SQUASHFS_INVALID_FRAG		(0xffffffffU)
#define SQUASHFS_INVALID_XATTR		(0xffffffffU)
#define SQUASHFS_INVALID_BLK		((int64_t)-1)

/* Filesystem flags */
#define SQUASHFS_NOI			0
#define SQUASHFS_NOD			1
#define SQUASHFS_NOF			3
#define SQUASHFS_NO_FRAG		4
#define SQUASHFS_ALWAYS_FRAG		5
#define SQUASHFS_DUPLICATE		6
#define SQUASHFS_EXPORT			7
#define SQUASHFS_COMP_OPT		10

/* Max number of types and file types */
#define SQUASHFS_DIR_TYPE		1
#define SQUASHFS_REG_TYPE		2
#define SQUASHFS_SYMLINK_TYPE		3
#define SQUASHFS_BLKDEV_TYPE		4
#define SQUASHFS_CHRDEV_TYPE		5
#define SQUASHFS_FIFO_TYPE		6
#define SQUASHFS_SOCKET_TYPE		7
#define SQUASHFS_LDIR_TYPE		8
#define SQUASHFS_LREG_TYPE		9
#define SQUASHFS_LSYMLINK_TYPE		10
#define SQUASHFS_LBLKDEV_TYPE		11
#define SQUASHFS_LCHRDEV_TYPE		12
#define SQUASHFS_LFIFO_TYPE		13
#define SQUASHFS_LSOCKET_TYPE		14

/* Xattr types */
#define SQUASHFS_XATTR_USER             0
#define SQUASHFS_XATTR_TRUSTED          1
#define SQUASHFS_XATTR_SECURITY         2
#define SQUASHFS_XATTR_VALUE_OOL        256
#define SQUASHFS_XATTR_PREFIX_MASK      0xff

#define SQUASHFS_COMPRESSED_BIT		(1 << 15)

#define SQUASHFS_COMPRESSED_BIT_BLOCK	(1 << 24)


/* cached data constants for filesystem */
#define SQUASHFS_CACHED_BLKS		8

#define SQUASHFS_MAX_FILE_SIZE_LOG	64

#define SQUASHFS_MAX_FILE_SIZE		(1LL << \
					(SQUASHFS_MAX_FILE_SIZE_LOG - 2))

/* meta index cache */
#define SQUASHFS_META_INDEXES	(SQUASHFS_METADATA_SIZE / sizeof(unsigned int))
#define SQUASHFS_META_ENTRIES	127
#define SQUASHFS_META_SLOTS	8


/*
 * definitions for structures on disk
 */
#define ZLIB_COMPRESSION	1

struct squashfs_super_block {
	__le32			s_magic;
	__le32			inodes;
	__le32			mkfs_time;
	__le32			block_size;
	__le32			fragments;
	__le16			compression;
	__le16			block_log;
	__le16			flags;
	__le16			no_ids;
	__le16			s_major;
	__le16			s_minor;
	__le64			root_inode;
	__le64			bytes_used;
	__le64			id_table_start;
	__le64			xattr_id_table_start;
	__le64			inode_table_start;
	__le64			directory_table_start;
	__le64			fragment_table_start;
	__le64			lookup_table_start;
};

struct squashfs_dir_index {
	__le32			index;
	__le32			start_block;
	__le32			size;
};

struct squashfs_base_inode {
	__le16			inode_type;
	__le16			mode;
	__le16			uid;
	__le16			guid;
	__le32			mtime;
	__le32			inode_number;
};

struct squashfs_ipc_inode {
	__le16			inode_type;
	__le16			mode;
	__le16			uid;
	__le16			guid;
	__le32			mtime;
	__le32			inode_number;
	__le32			nlink;
};

struct squashfs_lipc_inode {
	__le16			inode_type;
	__le16			mode;
	__le16			uid;
	__le16			guid;
	__le32			mtime;
	__le32			inode_number;
	__le32			nlink;
	__le32			xattr;
};

struct squashfs_dev_inode {
	__le16			inode_type;
	__le16			mode;
	__le16			uid;
	__le16			guid;
	__le32			mtime;
	__le32			inode_number;
	__le32			nlink;
	__le32			rdev;
};

struct squashfs_ldev_inode {
	__le16			inode_type;
	__le16			mode;
	__le16			uid;
	__le16			guid;
	__le32			mtime;
	__le32			inode_number;
	__le32			nlink;
	__le32			rdev;
	__le32			xattr;
};

struct squashfs_symlink_inode {
	__le16			inode_type;
	__le16			mode;
	__le16			uid;
	__le16			guid;
	__le32			mtime;
	__le32			inode_number;
	__le32			nlink;
	__le32			symlink_size;
};

struct squashfs_reg_inode {
	__le16			inode_type;
	__le16			mode;
	__le16			uid;
	__le16			guid;
	__le32			mtime;
	__le32			inode_number;
	__le32			start_block;
	__le32			fragment;
	__le32			offset;
	__le32			file_size;
};

struct squashfs_lreg_inode {
	__le16			inode_type;
	__le16			mode;
	__le16			uid;
	__le16			guid;
	__le32			mtime;
	__le32			inode_number;
	__le64			start_block;
	__le64			file_size;
	__le64			sparse;
	__le32			nlink;
	__le32			fragment;
	__le32			offset;
	__le32			xattr;
};

struct squashfs_dir_inode {
	__le16			inode_type;
	__le16			mode;
	__le16			uid;
	__le16			guid;
	__le32			mtime;
	__le32			inode_number;
	__le32			start_block;
	__le32			nlink;
	__le16			file_size;
	__le16			offset;
	__le32			parent_inode;
};

struct squashfs_ldir_inode {
	__le16			inode_type;
	__le16			mode;
	__le16			uid;
	__le16			guid;
	__le32			mtime;
	__le32			inode_number;
	__le32			nlink;
	__le32			file_size;
	__le32			start_block;
	__le32			parent_inode;
	__le16			i_count;
	__le16			offset;
	__le32			xattr;
};

struct squashfs_dir_entry {
	__le16			offset;
	__le16			inode_number;
	__le16			type;
	__le16			size;
};

struct squashfs_dir_header {
	__le32			count;
	__le32			start_block;
	__le32			inode_number;
};

struct squashfs_fragment_entry {
	__le64			start_block;
	__le32			size;
	unsigned int		unused;
};

struct squashfs_xattr_entry {
	__le16			type;
	__le16			size;
};

struct squashfs_xattr_val {
	__le32			vsize;
};

struct squashfs_xattr_id {
	__le64			xattr;
	__le32			count;
	__le32			size;
};

struct squashfs_xattr_id_table {
	__le64			xattr_table_start;
	__le32			xattr_ids;
	__le32			unused;
};

#endif
