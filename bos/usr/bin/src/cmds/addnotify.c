static char sccsid[] = "@(#)47	1.9.1.1  src/bos/usr/bin/src/cmds/addnotify.c, cmdsrc, bos411, 9428A410j 11/9/93 16:18:26";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	main
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
/*
** IDENTIFICATION:
**    Name:	Addnotify
**    Title:	Add Notify Object
** PURPOSE:
**	To establish subsystem/group notify method.
** 
** SYNTAX:
**    addnotify -n [subsystem_name | group_name] -m method
**
**    Flags:
**	-n Subsystem name/Group name
**	-m Method
**    Arguments:
**      subsystem_name - Name of failed Subsystem
**	group_name - Name of failed Group 
**	method - method to execute on failure of subsys or group.
**
** INPUT/OUTPUT SECTION:
**	ODM object: SRCnotify
**
** PROCESSING:
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**	subsystem/group name must not exit in the notify object.
**
** RETURNS:
**
**/
#include <odmi.h>
#include "src.h"
#include "srcopt.h"
#include "srcodm.h"
#include "srcobj.h"


struct SRCnotify notify;
struct SRCsubsys subsys;

/* view into valid input arguments */
struct argview argview[]=
{
	{1,(char *)notify.notifyname,ODM_CHAR,'n',0,
		VIEWFIELD,0,(SRCNAMESZ-1),SRC_NOTENAME2BIG},
	{1,(char *)notify.notifymethod,ODM_METHOD,'m',0,
		VIEWFIELD,0,sizeof(notify.notifymethod)-1,SRC_METHOD2BIG},
	{0}
};
/* Valid input arguments */
char argflags[]="n:m:";

int n_addview;

main(argc,argv)
char *argv[];
int argc;
{
	int rc;
	void *getrc;
	void *getrc2;
	char criteria[256];
	char criteria2[256];

	/* setup NLS support */
	setlocale(LC_ALL,"");

	/* parse the input arguments */
	n_addview = parseopt(argc,argv,argview,argflags);

	/* all arguments must have been entered */
	if(n_addview != 2)
	{
		srcerr(SRC_BASE,SRC_MKNOTIFY,SSHELL,0,0,0,0);
		exit(1);
	}

	/* notify object must not already exit with the subsystem/group name */
	rc=src_odm_init();
	if(rc > 0)
	{
		srcerr(ODM_BASE,odmerrno,SSHELL,0,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}
	sprintf(criteria,"notifyname = '%s'",notify.notifyname);
	getrc = odm_get_first(SRCNOTIFY,criteria,(char *)0);
	if((getrc != NULL) && (getrc != -1))
	{
		srcerr(SRC_BASE,SRC_NOTEXIST,SSHELL,0,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	/* subsystem or group must exist with subsysname or groupname */
	sprintf(criteria,"subsysname = '%s'",notify.notifyname);
	sprintf(criteria2,"grpname = '%s'",notify.notifyname);

	getrc = odm_get_first(SRCSYSTEM,criteria,&subsys);
	getrc2 = odm_get_first(SRCSYSTEM,criteria2,&subsys);
	if(((getrc == NULL) || (getrc == -1))
		&& ((getrc2 == NULL) || (getrc2 == -1)))
	{
		srcerr(SRC_BASE,SRC_NOSUBSYS,SSHELL,0,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	/* add the notify method */
	rc=odm_add_obj(SRCNOTIFY,&notify);
	if(rc >= 0)
	{
		src_odm_terminate(TRUE);
		srcerr(SRC_BASE,SRC_NOTADD,SSHELL,notify.notifyname,0,0,0);
		exit(0);
	}
	srcerr(ODM_BASE,odmerrno,SSHELL,notify.notifyname,0,0,0);
	src_odm_terminate(TRUE);
	exit(1);
}
