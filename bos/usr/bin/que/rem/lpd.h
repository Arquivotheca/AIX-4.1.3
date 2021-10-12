/* @(#)43	1.4  src/bos/usr/bin/que/rem/lpd.h, cmdque, bos411, 9428A410j 1/30/93 09:53:24 */
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#define PR		"/usr/bin/pr"
#define MAIL		"/usr/sbin/sendmail"
#define	LPD_LOCKNAME	"/etc/locks/lpd"	/* lock file for lpd	*/
#define	LPD_DIRECTORY	"/var/spool/lpd"	/* spool dir for lpd	*/
#define	LPD_HOSTS	"/etc/hosts.lpd"	/* permissions file for lpd  */
#define	EQUIV_HOSTS	"/etc/hosts.equiv"	/* rcmd.c permissions file */
