static char sccsid[] = "@(#)34	1.7.1.1  src/bos/usr/ccs/lib/libsrc/addssys.c, libsrc, bos411, 9428A410j 11/9/93 16:25:32";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	addssys
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
#include "srcaudit.h"
int addssys(subsys)
struct SRCsubsys *subsys;
{
	char criteria1[100];
	char criteria2[100];
	char *getrc;
	int rc;

	rc=checkssys(subsys);
	if(rc!=0)
		return(rc);

	if(src_odm_init() < 0)
		return(-1);

	sprintf(criteria1,"subsysname = '%s'",subsys->subsysname); 
	sprintf(criteria2,"synonym = '%s'",subsys->synonym); 

	/* does the subsystem or synonym already exist? */
	getrc=odm_get_first(SRCSYSTEM,criteria1,(char *)0);
	if(getrc != NULL && getrc != -1)
	{
		free((void *)getrc);
		src_odm_terminate(FALSE);
		return(SRC_SUBEXIST);
	}

	getrc=odm_get_first(SRCSYSTEM,criteria2,(char *)0);
	if(*subsys->synonym != '\0'
	    && getrc != NULL && getrc != -1)
	{
		free((void *)getrc);
		src_odm_terminate(FALSE);
		return(SRC_SYNEXIST);
	}

	/* add the subsystem to SRCSYSTEM class */
	subsys->auditid=getuid();
	rc=odm_add_obj(SRCSYSTEM,subsys);
	src_odm_terminate(FALSE);
	if(rc >= 0)
	{
		src_odm_auditlog(AUDIT_SRC_ADDSSYS,0,subsys->subsysname,
		    SRCSYSTEM,subsys,(char *)0);
		tellsrc(ADDSUBSYS,subsys->subsysname);
		return(0);
	}
	return(-1);
}
