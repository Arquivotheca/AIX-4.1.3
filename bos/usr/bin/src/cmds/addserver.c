static char sccsid[] = "@(#)48	1.10.1.1  src/bos/usr/bin/src/cmds/addserver.c, cmdsrc, bos411, 9428A410j 11/9/93 16:18:31";
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
**    Name:	addserver
**    Title:	Add Subserver Object
** PURPOSE:
**	To add Subserver definitions to the subserver object class.
** 
** SYNTAX:
**    addserver -s subsysname -t sub_type -c sub_code
**
**    Flags:
**	-s Subsystem name
**	-t Subserver ID
**	-c Subserver code point
**    Arguments:
**      subsysname - Subsystem name
**	sub_type - Subserver Id
**	sub_code - Subserver code point
**
** INPUT/OUTPUT SECTION:
**	ODM object: SRCsubsvr
**	ODM object: SRCsystem
**
** PROCESSING:
**
** PROGRAMS/FUNCTIONS CALLED:
**	ODM object: SRCsubsvr
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

struct SRCsubsvr subsvr;
struct SRCsubsys subsys;

short contact;

/* view into valid input arguments */
struct argview argview[]=
{
	{1,(char *)subsvr.sub_type,ODM_CHAR,'t',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_SUBTYP2BIG},
	{1,(char *)subsvr.subsysname,ODM_CHAR,'s',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_SUBSYS2BIG},
	{1,(char *)&subsvr.sub_code,ODM_SHORT,'c',0,VIEWFIELD},
	{0}
};
/* Valid input arguments */
char argflags[]="t:s:c:";

int n_addview;

main(argc,argv)
char *argv[];
int argc;
{
	int rc;
	void *getrc;
	char criteria[256];

	/* setup NLS support */
	setlocale(LC_ALL,"");

	/* parse the input arguments */
	n_addview = parseopt(argc,argv,argview,argflags);

	/* all arguments must have been entered */
	if(n_addview != 3)
	{
		srcerr(SRC_BASE,SRC_MKSERVER,SSHELL,0,0,0,0);
		exit(1);
	}

	/* can not use a reserved code point */
	if(subsvr.sub_code==SUBSVR || subsvr.sub_code==SUBSYSTEM)
	{
		srcerr(SRC_BASE,SRC_SUBCODE,SSHELL,0,0,0,0);
		exit(1);
	}

	rc=src_odm_init();
	if(rc < 0)
	{
		srcerr(ODM_BASE,odmerrno,SSHELL,0,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	/* the subserver's parent system must exist in the SRCSYSTEM class */
	sprintf(criteria,"subsysname = '%s'",subsvr.subsysname);
	getrc = odm_get_first(SRCSYSTEM,criteria,&subsys);
	if((getrc == NULL) || (getrc == -1))
	{
		srcerr(SRC_BASE,SRC_SSME,SSHELL,subsvr.subsysname,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	/* you can not define subservers if your contact method is signals */
	if(subsys.contact == SRCSIGNAL)
	{
		srcerr(SRC_BASE,SRC_CONT,SSHELL,subsvr.subsysname,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	/*  the new sub_type must not exist in SRCSUBSVR class */
	sprintf(criteria,"sub_type = '%s'",subsvr.sub_type);
	getrc = odm_get_first(SRCSUBSVR,criteria,(char *)0);
	if((getrc != NULL) && (getrc != -1))
	{
		srcerr(SRC_BASE,SRC_SVREXIST,SSHELL,subsvr.sub_type,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	/*  the new subsystem-sub_code must not exist in SRCSUBSVR class */
	sprintf(criteria,"subsysname = '%s' and sub_code = %d",subsvr.subsysname,subsvr.sub_code);
	getrc = odm_get_first(SRCSUBSVR,criteria,(char *)0);
	if((getrc != NULL) && (getrc != -1))
	{
		srcerr(SRC_BASE,SRC_SUBCODE,SSHELL,subsvr.subsysname,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	/* add the new subserver to SRCSUBSVR */
	rc=odm_add_obj(SRCSUBSVR,&subsvr);
	if(rc >= 0)
	{
		src_odm_auditlog(AUDIT_SRC_ADDSERVER,0,subsvr.subsysname,
		    SRCSUBSVR,&subsvr,(char *)0);
		src_odm_terminate(TRUE);
		srcerr(SRC_BASE,SRC_SVRADD,SSHELL,subsvr.sub_type,0,0,0);
		exit(0);
	}
	srcerr(ODM_BASE,odmerrno,SSHELL,subsvr.sub_type,0,0,0);
	src_odm_terminate(TRUE);
	exit(1);
}
