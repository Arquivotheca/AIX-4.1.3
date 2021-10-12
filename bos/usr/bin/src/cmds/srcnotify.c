static char sccsid[] = "@(#)76	1.9  src/bos/usr/bin/src/cmds/srcnotify.c, cmdsrc, bos411, 9428A410j 11/9/93 16:18:41";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcnotify
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


#include <stdio.h>
#include <errno.h>
#include <odmi.h>
#include "srcodm.h"
#include "srcobj.h"
void srcnotify(subsysname,groupname)
char *subsysname;
char *groupname;
{
	int rc;
	void *getrc;
	struct SRCnotify method;
	char criteria[256];
	char parms[256];

	if(fork()!=0)
		return;

	/* read that notify record to trigger method */
	sprintf(criteria,"notifyname = '%s'",subsysname);

	src_odm_init();

	/* method on the subsystem name we stop here */
	getrc = odm_get_first(SRCNOTIFY,criteria,&method);
	if((getrc != NULL) && (getrc != -1))
	{
		src_odm_terminate(TRUE);
		sprintf(parms,"%s %s",subsysname,groupname);
		odm_run_method(method.notifymethod,parms,0,0);
		exit(0);
	}

	/* well their wasn't a notify method for the subsystem */

	/* can only do a group method if a name exists */
	if(strlen(groupname)==0)
	{
		src_odm_terminate(TRUE);
		exit(0);
	}

	/* we have a group name */

	/* read that notify record to trigger method */
	sprintf(criteria,"notifyname = '%s'",groupname);

	/* method on the subsystem name we stop here */
	getrc = odm_get_first(SRCNOTIFY,criteria,&method);
	if((getrc != NULL) && (getrc != -1))
	{
		src_odm_terminate(TRUE);
		sprintf(parms,"%s %s",subsysname,groupname);
		odm_run_method(method.notifymethod,parms,0,0);
		exit(0);
	}
	src_odm_terminate(TRUE);

	/* don't care whether we had notify method or not at this point */
	exit(0);
}
