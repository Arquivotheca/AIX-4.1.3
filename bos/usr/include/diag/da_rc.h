/* @(#)23	1.2  src/bos/usr/include/diag/da_rc.h, cmddiag, bos411, 9428A410j 2/19/91 17:15:35 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_DA_RC
#define _H_DA_RC

#ifdef GOOD
#undef GOOD
#undef BAD
#endif

#define OLD_VERSION 0x60	/* temp, for staging */

#define	GOOD	1 | OLD_VERSION
#define	BAD	2 | OLD_VERSION
#define	ENDKEY	3 | OLD_VERSION
#define	ESCKEY	4 | OLD_VERSION
#define	FAILURE	5 | OLD_VERSION
#define	DDOPEN	6 | OLD_VERSION

#define DIAG_BADEXEC	7 | OLD_VERSION

#endif
