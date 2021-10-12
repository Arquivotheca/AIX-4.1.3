static char sccsid[] = "@(#)51	1.10.1.2  src/bos/usr/bin/src/cmds/chserver.c, cmdsrc, bos411, 9433B411a 8/17/94 15:54:23";
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
**    Name:	chserver
**    Title:	Change Subserver Object
** PURPOSE:
**	To change Subserver definitions to the subserver object class.
** 
** SYNTAX:
**    chserver -s subsysname [-t sub_type] [-t sub_type] [-c sub_code]
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
struct SRCsubsvr tsubsvr;
struct SRCsubsvr Usubsvr;
struct SRCsubsys subsys;
char sub_type[SRCNAMESZ];

struct argview argview[]=
{
	{1,(char *)sub_type,ODM_CHAR,'t',0,NOVIEWFIELD,0,SRCNAMESZ-1,SRC_SUBTYP2BIG},
	{sizeof(subsvr.sub_type),(char *)subsvr.sub_type,ODM_CHAR,'t',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_SUBTYP2BIG},
	{sizeof(subsvr.subsysname),(char *)subsvr.subsysname,ODM_CHAR,'s',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_SUBSYS2BIG},
	{sizeof(subsvr.sub_code),(char *)&subsvr.sub_code,ODM_SHORT,'c',0,VIEWFIELD},
	{0}
};


struct objview subview = {(char *)&subsvr,(struct fieldview *)&Usubsvr};

int n_subview;

/* valid input flags */
char argflags[]="t:s:c:";

main(argc,argv)
char *argv[];
int argc;
{
	int rc;
	void *getrc;
	char criteria[256];

	/* setup NLS support */
	setlocale(LC_ALL,"");

	n_subview = parseopt(argc,argv,argview,argflags);
	if(n_subview < 2 || argview[0].newval == 0)
	{
		srcerr(SRC_BASE,SRC_CHSERVER,SSHELL,0,0,0,0);
		exit(1);
	}

	n_subview=bldview(argview,&subview.fieldview,&subsvr,&Usubsvr);
	if(n_subview <= 0)
	{
		srcerr(SRC_BASE,SRC_CHSERVER,SSHELL,0,0,0,0);
		exit(1);
	}

	/* the subserver entery must exist! */
	rc=src_odm_init();

	sprintf(criteria,"sub_type = '%s'",sub_type);
	getrc = odm_get_first(SRCSUBSVR,criteria,&Usubsvr);
	if((getrc == NULL) || (getrc == -1))
	{
		srcerr(odmerrset(rc),odmerrmap(rc,SRC_TYPE),SSHELL,sub_type,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	/* if a new subsystem was specified the subsystem must exist */
	if(argview[2].newval != 0 && strcmp(Usubsvr.subsysname,subsvr.subsysname) != 0)
	{
		sprintf(criteria,"subsysname = '%s'",subsvr.subsysname);
		getrc = odm_get_first(SRCSYSTEM,criteria,&subsys);
		if((getrc == NULL) || (getrc == -1))
		{
			srcerr(odmerrset(rc),odmerrmap(rc,SRC_SSME),SSHELL,subsvr.subsysname,0,0,0);
			src_odm_terminate(TRUE);
			exit(1);
		}
		if(subsys.contact == SRCSIGNAL)
		{
			srcerr(SRC_BASE, SRC_CONT,SSHELL,subsvr.subsysname,0,0,0);
			src_odm_terminate(TRUE);
			exit(1);
		}
	}
	else
		strcpy(subsvr.subsysname,Usubsvr.subsysname);

	/* when the user entered a new subserver code for this subserver
	**   make sure that the new subserver code does not already exist
	**/
	if(argview[3].newval != 0 && Usubsvr.sub_code != subsvr.sub_code)
	{

		/* has user ented a reservered code */
		if(subsvr.sub_code==SUBSYSTEM || subsvr.sub_code==SUBSVR)
		{
			srcerr(SRC_BASE,SRC_SUBCODE,SSHELL,0,0,0,0);
			src_odm_terminate(TRUE);
			exit(1);
		}
	}
	else
		subsvr.sub_code=Usubsvr.sub_code;

	if((argview[3].newval != 0 && Usubsvr.sub_code != subsvr.sub_code) || (argview[2].newval != 0 && strcmp(Usubsvr.subsysname,subsvr.subsysname) != 0))
	{
		/* the combination of subsystem and code_point must not
		** already exist on file
		**/
		sprintf(criteria,"subsysname = '%s' and sub_code = %d",subsvr.subsysname,subsvr.sub_code);
		getrc = odm_get_first(SRCSUBSVR,criteria,(char *)0);
		if((getrc != NULL) && (getrc != -1))
		{
			srcerr(SRC_BASE, SRC_SUBCODE,SSHELL,0,0,0,0);
			src_odm_terminate(TRUE);
			exit(1);
		}
	}

	/* when the user entered a new subserver name for this subserver
	**   make sure that the new subserver does not already exist
	**/
	if(argview[1].newval != 0 && strcmp(Usubsvr.sub_type,subsvr.sub_type) != 0)
	{
		sprintf(criteria,"sub_type = '%s'",subsvr.sub_type);
		getrc = odm_get_first(SRCSUBSVR,criteria,(char *)0);
		if((getrc != NULL) && (getrc != -1))
		{
			srcerr(SRC_BASE,SRC_SVREXIST,SSHELL,0,0,0,0);
			src_odm_terminate(TRUE);
			exit(1);
		}
	}

	memcpy(&tsubsvr,&Usubsvr,sizeof(struct SRCsubsvr));
	putdbfields(subview.fieldview);
	rc=odm_change_obj(SRCSUBSVR,&Usubsvr);
	free((char *) subview.fieldview);
	if(rc == 0)
	{
		
		src_odm_auditlog(AUDIT_SRC_CHSERVER,0,Usubsvr.subsysname,
		    SRCSUBSVR,&Usubsvr,&tsubsvr);
		src_odm_terminate(TRUE);
		srcerr(SRC_BASE,SRC_SERVERCH,SSHELL,sub_type,0,0,0);
		exit(0);
	}
	srcerr(odmerrset(rc),odmerrmap(rc,SRC_NOREC),SSHELL,sub_type,0,0,0);
	src_odm_terminate(TRUE);
	exit(1);
}
