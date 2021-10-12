/* @(#)77	1.7.1.1  src/bos/usr/include/sysck.h, cmdsadm, bos411, 9428A410j 3/6/94 16:15:03 */
/*
 * COMPONENT_NAME: (sysck.h) 
 *
 * FUNCTIONS: 
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

#ifndef _H_SYSCK    
#define _H_SYSCK

#define	S_CLASS		"class"
#define	S_TYPE		"type"
#define S_TCB		"tcb"
#define	S_OWNER		"owner"
#define	S_GROUP		"group"
#define	S_ACL		"acl"
#define	S_PCL		"pcl"
#define S_MODE		"mode"
#define	S_LINKS		"links"
#define	S_SYMLINKS	"symlinks"
#define	S_CHECKSUM	"checksum"
#define	S_PROGRAM	"program"
#define	S_SOURCE	"source"
#define	S_SIZE		"size"
#define	S_TARGET	"target"

#ifdef _NO_PROTO
	extern  int	settcbattr ();
	extern	int	gettcbattr ();
	extern	int	puttcbattr ();
	extern	int	endtcbattr ();
#else	/* _NO_PROTO */
	extern  int	settcbattr (int mode);
	extern	int	gettcbattr (char *tcb, char *atnam, void *val, int type);
	extern	int	puttcbattr (char *tcb, char *atnam, void *val, int type);
	extern	int	endtcbattr (void);
#endif	/* _NO_PROTO */

#endif /* _H_SYSCK */
