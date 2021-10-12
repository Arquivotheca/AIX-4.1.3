/* @(#)83	1.2  src/bos/usr/include/crypt.h, libcs, bos411, 9428A410j 10/20/93 14:15:00 */
/*
 *   COMPONENT_NAME: LIBCTHRD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27,71
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*  crypt.h,v $ $Revision: 1.4 $ (OSF) */

/*
 * definitions for the reentrant versions of crypt()
 */

#ifndef	_CRYPT_H_
#define	_CRYPT_H_

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
typedef	struct crypt_data {
/*
From OSF, Not needed in AIX
	char C[28], D[28];
*/
	char E[48];
	char KS[16][48];
	char block[66];
	char iobuf[16];
} CRYPTD;

#ifdef _NO_PROTO
extern	char	*crypt_r();
extern	int	setkey_r();
extern	int	encrypt_r();
#else
extern	char	*crypt_r(char *, char *, CRYPTD *);
extern	int	setkey_r(char *, CRYPTD *);
extern	int	encrypt_r(char *, int, CRYPTD *);
#endif /* _NO_PROTO */
#endif
#endif	/* _CRYPT_H_ */
