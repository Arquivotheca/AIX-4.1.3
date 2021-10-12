/* @(#)90	1.9  src/bos/usr/include/IN/CSdefs.h, libIN, bos411, 9428A410j 3/22/93 11:31:24 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_CSDEFS
#define _H_CSDEFS

/*
 * Type definitions of the CA and CS routines of libIN
 */

extern char *CAcpy(), *CAfill(), *CAtr(), *CScat(),
	*CScpy(), *CScpym(), *CScpyn(), *CScpyu(), *CScpyum(),
	*CSdevname(), *CSdname(), *CSgetl(), *CSgets(), *CSloca(),
	*CSlocc(), *CSlocs(), *CSloct(), *CSprintf(), *CSskpa(),
	*CSskpc(), *CSskps(), *CSskpt(), *CSsname();
extern int CScmp(), CScmpp(), CScurdir(), CSlen(), CSnil();
extern long CStol();

#define CSequal(a,b) (CScmp(a,b)==0)

#endif /* _H_CSDEFS */
