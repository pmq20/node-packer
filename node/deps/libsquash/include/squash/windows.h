/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#ifndef WINDOWS_H_A80B5674
#define WINDOWS_H_A80B5674

#include <Windows.h>
#include <stdint.h>
#include <io.h>

typedef unsigned short sqfs_mode_t;
typedef uint32_t sqfs_id_t; /* Internal uids/gids are 32-bits */

typedef SSIZE_T ssize_t;
typedef DWORD64 sqfs_off_t;
typedef uint32_t uid_t;		/* [???] user IDs */

#ifndef S_IFIFO
#define	S_IFIFO		0010000		/* [XSI] named pipe (fifo) */
#endif

#ifndef S_IFBLK
#define	S_IFBLK		0060000		/* [XSI] block special */
#endif

#ifndef S_IFLNK
#define	S_IFLNK		0120000		/* [XSI] symbolic link */
#endif

#ifndef S_IFSOCK
#define	S_IFSOCK	0140000		/* [XSI] socket */
#endif

#ifndef S_ISBLK
#define	S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)	/* block special */
#endif

#ifndef S_ISCHR
#define	S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)	/* char special */
#endif

#ifndef S_ISDIR
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)	/* directory */
#endif

#ifndef S_ISFIFO
#define	S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)	/* fifo or socket */
#endif

#ifndef S_ISREG
#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)	/* regular file */
#endif

#ifndef S_ISLNK
#define	S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)	/* symbolic link */
#endif

#ifndef S_ISSOCK
#define	S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)	/* socket */
#endif

#ifndef makedev
#define	makedev(x,y)	((dev_t)(((x) << 24) | (y)))
#endif

#ifndef DT_UNKNOWN
#define	DT_UNKNOWN	 0
#endif

#ifndef	DT_FIFO
#define	DT_FIFO		 1
#endif

#ifndef	DT_CHR
#define	DT_CHR		 2
#endif

#ifndef	DT_DIR
#define	DT_DIR		 4
#endif

#ifndef	DT_BLK
#define	DT_BLK		 6
#endif

#ifndef	DT_REG
#define	DT_REG		 8
#endif

#ifndef	DT_LNK
#define	DT_LNK		10
#endif

#ifndef	DT_SOCK
#define	DT_SOCK		12
#endif

#ifndef	DT_WHT
#define	DT_WHT		14
#endif

#endif /* end of include guard: WINDOWS_H_A80B5674 */
