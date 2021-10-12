static char sccsid[] = "@(#)53	1.6  src/bos/usr/bin/src/cmds/delnotify.c, cmdsrc, bos411, 9428A410j 2/26/91 14:50:51";
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
**    Name:	Delnotify
**    Title:	Delete Notify Object
** PURPOSE:
**	To delete subsystem/group notify method.
** 
** SYNTAX:
**    delnotify -n subsystem_name | group_name
**
**    Flags:
**	-n Subsystem name/Group name
**    Arguments:
**      subsystem_name - Name of failed Subsystem
**	group_name - Name of failed Group 
**
** INPUT/OUTPUT SECTION:
**	ODM object: SRCnotify
**
** PROCESSING:
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**
** RETURNS:
**
**/
#include <odmi.h>
#include "src.h"
#include "srcopt.h"
#include "srcodm.h"
#include "srcobj.h"

char notifyname[SRCNAMESZ];

/* view into valid input arguments */
struct argview argview[]=
{
	{1,(char *)notifyname,ODM_CHAR,'n',0,VIEWFIELD,0,(SRCNAMESZ-1),SRC_NOTENAME2BIG},
	{0}
};
/* Valid input arguments */
char argflags[]="n:";

int n_addview;

main(argc,argv)
char *argv[];
int argc;
{
	int rc;
	char criteria[256];

	/* setup NLS support */
	setlocale(LC_ALL,"");

	/* parse the input arguments */
	n_addview = parseopt(argc,argv,argview,argflags);

	/* all arguments must have been entered */
	if(n_addview != 1)
	{
		srcerr(SRC_BASE, SRC_RMNOTIFY,SSHELL,0,0,0,0);
		exit(1);
	}

	/* init odm environment */
	rc=src_odm_init();
	if(rc < 0)
	{
		srcerr(ODM_BASE, odmerrno,SSHELL,0,0,0,0);
		exit(1);
	}

	/* delete the notify object */
	sprintf(criteria,"notifyname = '%s'",notifyname);
	rc=odm_rm_obj(SRCNOTIFY,criteria);
	if(rc > 0)
	{
		src_odm_terminate(TRUE);
		srcerr(SRC_BASE, SRC_NOTRM,SSHELL,notifyname,0,0,0);
		exit(0);
	}
	srcerr(odmerrset(rc),odmerrmap(rc,SRC_NOREC),SSHELL,notifyname,0,0,0);
	src_odm_terminate(TRUE);
	exit(1);
}
