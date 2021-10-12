/* @(#)21	1.4  src/bos/kernel/sys/id.h, syssauth, bos411, 9428A410j 10/23/90 18:51:06 */
/*
 * COMPONENT_NAME: (id.h) 
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

#ifndef _H_ID
#define _H_ID

#include <sys/types.h>

#define	ID_EFFECTIVE	0x01
#define	ID_REAL		0x02
#define	ID_SAVED	0x04
#define	ID_LOGIN	0x08
#define ID_ACCT		0x10

/*
 * The following prototypes declare the functions which use the
 * constants defined above.
 */

#ifdef	_NO_PROTO
extern	uid_t	getuidx();
extern	gid_t	getgidx();
extern	int	setuidx();
extern	int	setgidx();
#else
extern	uid_t	getuidx (int which);
extern	gid_t	getgidx (int which);
extern	int	setuidx (int which, uid_t uid);
extern	int	setgidx (int which, gid_t gid);
#endif

#endif /* _H_ID */
