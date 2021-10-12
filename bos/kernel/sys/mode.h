/* @(#)86	1.11  src/bos/kernel/sys/mode.h, syssdac, bos411, 9428A410j 6/16/90 00:32:21 */
/*
 * COMPONENT_NAME: SYSSEC - Security Component
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_MODE
#define _H_MODE


/*
 * POSIX requires that certain values effectively be included in stat.h.
 * It also requires that when _POSIX_SOURCE is defined, only those standard
 * specific values are present.  Since mode.h defines values on behalf of
 * stat.h and since stat.h includes mode.h, this header adheres to the
 * POSIX requirements.
 */

#include <standards.h>
#ifdef _POSIX_SOURCE 

/*
 *	(stat) st_mode bit values
 */

#define _S_IFMT		0170000		/* type of file */
#define   _S_IFREG	0100000		/*   regular */
#define   _S_IFDIR	0040000		/*   directory */
#define   _S_IFBLK	0060000		/*   block special */
#define   _S_IFCHR	0020000		/*   character special */
#define   _S_IFIFO	0010000		/*   fifo */

#define S_ISUID		0004000		/* set user id on execution */
#define S_ISGID		0002000		/* set group id on execution */

					/* ->>> /usr/group definitions <<<- */
#define S_IRWXU		0000700		/* read,write,execute perm: owner */
#define S_IRUSR		0000400		/* read permission: owner */
#define S_IWUSR		0000200		/* write permission: owner */
#define S_IXUSR		0000100		/* execute/search permission: owner */
#define S_IRWXG		0000070		/* read,write,execute perm: group */
#define S_IRGRP		0000040		/* read permission: group */
#define S_IWGRP		0000020		/* write permission: group */
#define S_IXGRP		0000010		/* execute/search permission: group */
#define S_IRWXO		0000007		/* read,write,execute perm: other */
#define S_IROTH		0000004		/* read permission: other */
#define S_IWOTH		0000002		/* write permission: other */
#define S_IXOTH		0000001		/* execute/search permission: other */

/*
 *	File type macros
 */

#define S_ISFIFO(m)	(((m)&(_S_IFMT)) == (_S_IFIFO))
#define S_ISDIR(m)	(((m)&(_S_IFMT)) == (_S_IFDIR))
#define S_ISCHR(m)	(((m)&(_S_IFMT)) == (_S_IFCHR))
#define S_ISBLK(m)	(((m)&(_S_IFMT)) == (_S_IFBLK))
#define S_ISREG(m)	(((m)&(_S_IFMT)) == (_S_IFREG))

#endif /* _POSIX_SOURCE */


/*
 *	Additional mode bit values
 *	(Macros are separated because they're not strictly part of the standard)
 */

#ifdef _XOPEN_SOURCE

#define S_ISVTX		0001000		/* save text even after use */

/*
 *	(stat) st_mode bit values
 */
#define S_IFMT		_S_IFMT		/* type of file */
#define   S_IFREG	_S_IFREG	/*   regular */
#define   S_IFDIR	_S_IFDIR	/*   directory */
#define   S_IFBLK	_S_IFBLK	/*   block special */
#define   S_IFCHR	_S_IFCHR	/*   character special */
#define   S_IFIFO	_S_IFIFO	/*   fifo */

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

/*
 *	(stat) st_mode bit values
 */

#define S_IFSOCK	0140000		/* socket */
#define S_IFLNK		0120000		/* symbolic link */
#define S_IREAD		0000400		/* read permission, owner */
#define S_IWRITE	0000200		/* write permission, owner */
#define S_IEXEC		0000100		/* execute/search permission, owner */

#define S_ENFMT		S_ISGID		/* record locking enforcement flag */

#define S_IFMPX		(S_IFCHR|S_ISVTX) /* multiplex character special file */
#define S_ISMPX(m)	(((m)&(S_IFMT|S_ISVTX)) == (S_IFMPX))
#define S_ISLNK(m)	(((m)&(S_IFMT)) == (S_IFLNK))
#define S_ISSOCK(m)	(((m)&(S_IFMT)) == (S_IFSOCK))


/*
 *	Equivalent mode macros (from sys/inode.h)
 */

#define	IFMT	S_IFMT		/* type of file */
#define	IFDIR	S_IFDIR		/* directory */
#define	IFCHR	S_IFCHR		/* character special */
#define	IFBLK	S_IFBLK		/* block special */
#define	IFREG	S_IFREG		/* regular */
#define	IFIFO	S_IFIFO		/* fifo */
#define	IFSOCK	S_IFSOCK	/* socket */
#define	IFLNK	S_IFLNK		/* symbolic link */
#define	ISUID	S_ISUID		/* set user id on execution */
#define	ISGID	S_ISGID		/* set group id on execution */
#define	ISVTX	S_ISVTX		/* save swapped text even after use */
#define	IREAD	S_IREAD		/* read permission, owner */
#define	IWRITE	S_IWRITE	/* write permission, owner */
#define	IEXEC	S_IEXEC		/* execute/search permission, owner */


/*
 *	High order mode bit definitions (for security)
 */

/* version information */
#define S_INMOD		0xC0000000	/* normal mode */
#define S_IXMOD		0x40000000	/* extended mode */

#define S_IJRNL		0x04000000	/* journalled */
#define S_IXACL		0x02000000	/* extended ACL */
#define S_ITCB		0x01000000	/* Trusted Computing Base */
#define S_ITP		0x00800000	/* Trusted Process */

#endif /* _ALL_SOURCE */
#endif /* _H_MODE */
