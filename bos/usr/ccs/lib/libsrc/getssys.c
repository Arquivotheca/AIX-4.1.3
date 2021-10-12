static char sccsid[] = "@(#)36	1.4  src/bos/usr/ccs/lib/libsrc/getssys.c, libsrc, bos411, 9428A410j 11/9/93 16:22:24";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	getssys,getsubsvr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include <odmi.h>
#include "src.h"
#include "srcobj.h"
#include "srcodm.h"

static char criteria1[100];
int getssys(subsysname,subsys)
struct SRCsubsys *subsys;
char *subsysname;
{
	char *getrc;

	if(src_odm_init() < 0)
		return(-1);

	sprintf(criteria1,"subsysname = '%s'",subsysname); 

	/* does the subsystem or synonym already exist? */
	getrc = odm_get_first(SRCSYSTEM,criteria1,subsys);
	src_odm_terminate(FALSE);
	if(getrc != NULL && getrc != -1) {
		return(0);
	}
	else if(getrc == (char *)0)
		return(SRC_NOREC);
	else
		return(-1);
}

int getsubsvr(sub_type,subsvr)
struct SRCsubsvr *subsvr;
char *sub_type;
{
	char *getrc;

	if(src_odm_init() < 0)
		return(-1);

	sprintf(criteria1,"sub_type = '%s'",sub_type); 

	/* does the subsystem or synonym already exist? */
	getrc=odm_get_first(SRCSUBSVR,criteria1,subsvr);
	src_odm_terminate(FALSE);
	if(getrc != NULL && getrc != -1) {
		return(0);
	}
	else if(getrc==(char *)0)
		return(SRC_NOREC);
	else
		return(-1);
}
