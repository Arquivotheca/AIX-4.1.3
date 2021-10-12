/* @(#)39	1.3  src/bos/usr/include/jfs/dir.h, syspfs, bos411, 9434A411a 8/19/94 09:22:55 */
/*
 * SYSPFS: journalled filesystem
 *
 * FUNCTIONS: dir.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_JFS_DIR
#define _H_JFS_DIR

/*
 * Internal directory routine argument type
 */
typedef	struct {
	caddr_t nm;
	int	nmlen;
} dname_t;

struct direct {
	ulong	d_ino;
	ushort	d_reclen;
	ushort	d_namlen;
	char	d_name[MAXNAMLEN+1];	/* NULL terminated	*/
};

typedef struct direct direct_t;

#	define	DIROUND	4
#	define	LDIRECLEN(dirp)	((struct direct *)dirp->d_reclen)
#	define	LDIRSIZE(len)	((sizeof(struct direct) - (MAXNAMLEN+1) + \
		((len)+1) + (DIROUND-1)) & ~(DIROUND-1))
#	define	LDIRNMLEN(dirp)	(dirp->d_namlen)

#define	DOT_EXIST	0x1
#define DOT_VALID	0x2
#define DDOT_EXIST	0x4
#define DDOT_VALID	0x8
#define DDOT_MASK	0xc
#define DDOT_STAT(d)	((d) & DDOT_MASK)

#endif /* _H_JFS_DIR */
