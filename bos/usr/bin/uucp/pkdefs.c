static char sccsid[] = "@(#)12	1.3  src/bos/usr/bin/uucp/pkdefs.c, cmduucp, bos411, 9428A410j 6/16/90 00:00:30";
/* 
 * COMPONENT_NAME: UUCP pkdefs.c
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.pkdefs.c
	pkdefs.c	1.1	7/29/85 16:33:24
*/
#include "uucp.h"
/* VERSION( pkdefs.c	5.2 -  -  ); */

#define USER 1
#include "pk.h"
char next[8]	={ 1,2,3,4,5,6,7,0};	/* packet sequence numbers */
char mask[8]	={ 1,2,4,010,020,040,0100,0200 };
int	pkactive;
int pkdebug;
int pksizes[] = {
	1, 32, 64, 128, 256, 512, 1024, 2048, 4096, 1
};
struct pack *pklines[NPLINES];
