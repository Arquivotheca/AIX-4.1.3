static char sccsid[] = "@(#)55	1.8  src/bos/usr/bin/src/cmds/delssys.c, cmdsrc, bos411, 9428A410j 2/26/91 14:50:56";
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
**    Name:	delssys
**    Title:	Delete Subsystem Object
** PURPOSE:
**	To delete subsystem from the subsystem object
** 
** SYNTAX:
**    delssys -s subsystem_name 
**
**    Flags:
**	-s Subsystem name
**    Arguments:
**      subsyste_name - Subsystem name
**
** INPUT/OUTPUT SECTION:
**	ODM object: SRCsystem
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
#include "srcobj.h"

char	criteria[256];
char	subsysname[SRCNAMESZ];

struct argview argview[]=
{
	{SRCNAMESZ,(char *)subsysname,ODM_CHAR,'s',0,VIEWFIELD,0,
		SRCNAMESZ-1,SRC_SUBSYS2BIG},
	{0}
};

char argflags[]="s:";

main(argc,argv)
char *argv[];
int argc;
{
	int rc;

	/* setup NLS support */
	setlocale(LC_ALL,"");

	/* parse command arguments */
	rc = parseopt(argc,argv,argview,argflags);
	if(rc==0 || argview[0].newval == 0)
	{
		srcerr(SRC_BASE, SRC_RMSSYS,SSHELL,0,0,0,0);
		exit(1);
	}

	/* init the odm environment */
	rc=src_odm_init();
	if(rc != 0)
	{
		srcerr(ODM_BASE, odmerrno,SSHELL,0,0,0,0);
		exit(1);
	}

	rc=delssys(subsysname);
	if(rc<=0)
	{
		srcerr(odmerrset(rc),odmerrmap(rc,SRC_NOREC),SSHELL,subsysname,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}
	srcerr(SRC_BASE, SRC_SSYSRM,SSHELL,subsysname,0,0,0);
	src_odm_terminate(TRUE);
	exit(0);
}
