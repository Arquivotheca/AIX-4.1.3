/* @(#)86	1.8  src/bos/kernel/sys/lkup.h, syslfs, bos411, 9428A410j 12/9/92 08:13:58 */
#ifndef _H_LKUP
#define _H_LKUP

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define	L_NOFOLLOW	0x0080			/* for symbolic links */
#define	L_NOXLINK	L_NOFOLLOW
#define L_NOXMOUNT	0x0040			/* for mount stubs */
#define L_NOXHIDDEN	0x0020			/* for hidden directories */
#define	L_LOC		0x0010
#define	L_OPEN 		0x0008
#define	L_EROFS		0x0004
#define L_SEARCH	0x0002
#define	L_CRT		0x0001
#define	L_CREATE	(L_CRT | L_EROFS)
#define	L_SEARCHLOC	(L_SEARCH | L_LOC)
#define	L_DEL		(L_SEARCH | L_EROFS)

#endif /* _H_LKUP */
