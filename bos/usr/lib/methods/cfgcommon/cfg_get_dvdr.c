static char sccsid[] = "@(#)05  1.1  src/bos/usr/lib/methods/cfgcommon/cfg_get_dvdr.c, cfgmethods, bos411, 9428A410j 6/28/94 07:10:20";
/*
 *   COMPONENT_NAME: CFGMETHODS - cfg_get_dvdr.c - gets PdDv driver name
 *
 *   FUNCTIONS: get_dvdr_name
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include "cfgcommon.h"


int
get_dvdr_name()

{
	/* dvdr, pddv are globals */
	strcpy(dvdr, pddv.DvDr);
	return(0);
}
