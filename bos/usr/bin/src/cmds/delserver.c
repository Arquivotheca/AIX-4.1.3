static char sccsid[] = "@(#)54	1.8  src/bos/usr/bin/src/cmds/delserver.c, cmdsrc, bos411, 9428A410j 2/26/91 14:50:54";
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
**    Name:	delserver
**    Title:	delete subserver entry
** PURPOSE:
** 
** SYNTAX:
**    delserver -t subserver_type
**
**    Arguments:
**	-t subserver_type - subserver type to be deleted
**
** INPUT/OUTPUT SECTION:
**	ODM object: SRCsubsvr
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
#include "srcaudit.h"
#include "srcobj.h"

char sub_type[SRCNAMESZ];

struct argview argview[]=
{
	{1,(char *)sub_type,ODM_CHAR,'t',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_SUBTYP2BIG},
	{0}
};

int n_addview;

char argflags[]="t:";

main(argc,argv)
char *argv[];
int argc;
{
	int rc;
	char criteria[256];

	/* setup NLS support */
	setlocale(LC_ALL,"");

	n_addview = parseopt(argc,argv,argview,argflags);
	if(n_addview != 1)
	{
		srcerr(SRC_BASE, SRC_RMSERVER,SSHELL,0,0,0,0);
		exit(1);
	}

	/* init the odm envrionment */
	rc=src_odm_init();

	/* delete the object requested */
	sprintf(criteria,"sub_type = '%s'",sub_type);
	rc=odm_rm_obj(SRCSUBSVR,criteria);
	if(rc > 0)
	{
		src_odm_terminate(TRUE);
		srcerr(SRC_BASE, SRC_SERVERRM,SSHELL,sub_type,0,0,0);
		auditlog(AUDIT_SRC_DELSERVER,0,sub_type,strlen(sub_type));
		exit(0);
	}

	srcerr(odmerrset(rc),odmerrmap(rc,SRC_NOREC),SSHELL,sub_type,0,0,0);
	src_odm_terminate(TRUE);
	exit(1);
}
