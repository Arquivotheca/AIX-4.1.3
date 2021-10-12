/* @(#)20	1.3  src/bos/usr/include/security.h, cmdsadm, bos411, 9428A410j 6/16/90 00:13:34 */
 
/*
 * COMPONENT_NAME: (TCBADM) Administrative User IDs for commands and data
 *
 * ORIGIN: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SECURITY
#define _H_SECURITY

/*
 * Defines for ownership of data files and commands
 */

#define	OWNER_USRDATA	3	/* owner of user data files  	     sys    */
#define	OWNER_USRCMD	2	/* owner of user commands            bin    */
#define	OWNER_ADMDATA	0	/* owner of administrator data       root   */
#define	OWNER_ADMCMD	0	/* owner of administrator commands   root   */

#endif /* _H_SECURITY */
