static char sccsid[] = "@(#)49	1.9  src/bos/usr/bin/src/cmds/addssys.c, cmdsrc, bos411, 9428A410j 2/26/91 14:50:45";
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
**    Name:	addssys
**    Title:	Add Subsystem Object
** PURPOSE:
**	To add subsystem difinitions to the subsystem object class.
** 
** SYNTAX:
**    addssys	-s subsystem_name [-t synonym] -a command_aruments\
**		-u userid \
**		[-i stdin] [-o stdout] [-e stderr]\
**		[-R | -O] [-d | -D] [-q | -Q]\
**		[-I ipcquekey -m svrmtype | -S | -K]\
**		[-n signorm] [-f sigforce] [-E priority] [-G groupname]
**		[-w] wait_time\
**    Flags/Arguments:
**	-D Inactive subsystem not displayed on status all/group
**	-E Execution Priority
**	-I Contact by IPC Queue
**	-G Group name
**	-O Start Once
**	-Q No Multiple Instance support
**	-R Restart
**	-S Contact by Signals
**	-K Contact by Sockets
**	-d Inactive subsystem displayed on status all/group
**	-e Standard error
**	-f Stop FORCED signal
**	-i Standard input
**	-m server IPC Que mtype
**	-n Stop NORMAL signal
**	-o Standard output
**	-p Path to Subsystem
**	-q Multiple Instance support
**	-s Subsystem name
**	-t Synonym name
**	-u User id
**      -a command arguments
**	-w Wait time for stop cancel and respawn time limit
**
** INPUT/OUTPUT SECTION:
**	ODM object: SRCsystem
**
** PROCESSING:
**
** PROGRAMS/FUNCTIONS CALLED:
**
** OTHER:
**	Subsystem name must not already exist.
**	Synonym name must not already exist.
**
** RETURNS:
**
**/
#include <sys/signal.h>
#include <odmi.h>
#include "src.h"
#include "srcopt.h"
#include "srcobj.h"
#include "srcodm.h"

struct SRCsubsys subsys;

/* input argument view */
struct argview argview[]=
{
	{1,(char *)subsys.subsysname,ODM_CHAR,'s',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_SUBSYS2BIG},
	{1,(char *)subsys.synonym,ODM_CHAR,'t',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_SUBSYS2BIG},
	{1,(char *)subsys.cmdargs,ODM_CHAR,'a',0,VIEWFIELD,0,SRCPATHSZ-1,SRC_CMDARG2BIG},
	{1,(char *)subsys.path,ODM_CHAR,'p',0,VIEWFIELD,0,SRCPATHSZ-1,SRC_PATH2BIG},
	{1,(char *)&subsys.display,FLAGSHORT,'d',0,VIEWFIELD},
	{1,(char *)&subsys.display,FLAGFIELD,'D',0,VIEWFIELD},
	{1,(char *)&subsys.uid,ODM_LONG,'u',0,VIEWFIELD},
	{1,(char *)&subsys.multi,FLAGSHORT,'q',0,VIEWFIELD},
	{1,(char *)&subsys.multi,FLAGFIELD,'Q',0,VIEWFIELD},
	{1,(char *)&subsys.waittime,ODM_SHORT,'w',0,VIEWFIELD},
	{1,(char *)subsys.standin,ODM_CHAR,'i',0,VIEWFIELD,0,SRCPATHSZ-1,SRC_STDIN2BIG},
	{1,(char *)subsys.standout,ODM_CHAR,'o',0,VIEWFIELD,0,SRCPATHSZ-1,SRC_STDOUT2BIG},
	{1,(char *)subsys.standerr,ODM_CHAR,'e',0,VIEWFIELD,0,SRCPATHSZ-1,SRC_STDERR2BIG},
	{1,(char *)&subsys.action,FLAGFIELD,'R',0,VIEWFIELD},
	{1,(char *)&subsys.action,FLAGFIELD,'O',0,VIEWFIELD},
	{1,(char *)&subsys.signorm,ODM_SHORT,'n',0,VIEWFIELD},
	{1,(char *)&subsys.sigforce,ODM_SHORT,'f',0,VIEWFIELD},
	{1,(char *)subsys.grpname,ODM_CHAR,'G',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_GRPNAM2BIG},
	{1,(char *)&subsys.contact,FLAGFIELD,'S',0,VIEWFIELD},
	{1,(char *)&subsys.svrkey,ODM_LONG,'I',0,VIEWFIELD},
	{1,(char *)&subsys.svrmtype,ODM_LONG,'m',0,VIEWFIELD,1,0,SRC_MTYPE},
	{1,(char *)&subsys.contact,FLAGFIELD,'K',0,VIEWFIELD},
	{1,(char *)&subsys.priority,ODM_SHORT,'E',0,VIEWFIELD,MINPRIORITY,MAXPRIORITY,SRC_PRIORITY},
	{0}
};

/* valid option flags */
char argflags[]="s:t:a:p:u:i:o:e:ROI:SKm:E:f:n:G:DdQqw:";

int n_addview;

main(argc,argv)
char *argv[];
int argc;
{
	int rc;
	char criteria1[100];
	char criteria2[100];

	/* setup NLS support */
	setlocale(LC_ALL,"");

	/* init subsys and set defaults*/
	defssys(&subsys);

	/* parse the command arguments entered */
	n_addview = parseopt(argc,argv,argview,argflags);
	
	/* error parseing the option */
	if(n_addview < 1)
	{
		srcerr(SRC_BASE,SRC_MKSSYS,SSHELL,0,0,0,0);
		exit(1);
	}

	/* validate data entered on command line */
	if((int)argview[0].newval == 0 ||	/* subsysname is required */
	   (int)argview[3].newval == 0 ||	/* path is required */
	   (int)argview[6].newval == 0 ||	/* user id is required */
	   /* RESTART or ONCE */
	   ((int)argview[14].newval > 0 && (int)argview[13].newval > 0) ||
	   /* may not specify IPC and SIGNAL and SOCKETS*/
	   ((int)argview[19].newval + (int)argview[18].newval 
		+ (int)argview[21].newval > 1) ||
	   /* must specify signals when you are using signals */
	   ((int)argview[18].newval + (int)argview[15].newval 
		+ (int)argview[16].newval != 0 &&
	    (int)argview[18].newval + (int)argview[15].newval 
		+ (int)argview[16].newval != 3) ||
	   /* when ICP is entered svrmtype must be entered */
	   ((int)argview[19].newval > 0 && (int)argview[20].newval == 0) ||
	   ((int)argview[19].newval == 0 && (int)argview[20].newval > 0) ||
	   /* may not specify display and no display */
	   ((int)argview[4].newval > 0 && (int)argview[5].newval > 0) ||
	   /* may not specify multi and not multi */
	   ((int)argview[7].newval > 0 && (int)argview[8].newval > 0))
	{
		srcerr(SRC_BASE,SRC_MKSSYS,SSHELL,0,0,0,0);
		exit(1);
	}

	/* -O restart action of ONCE */
	if(argview[14].newval)
		subsys.action=ONCE;

	/* -R restart action of RESPAWN */
	if(argview[13].newval)
		subsys.action=RESPAWN;

	/* -I contact by IPC queue? */
	if(argview[19].newval)
		subsys.contact=SRCIPC;
	/* -S contact by sinals? */
	else if(argview[18].newval)
		subsys.contact=SRCSIGNAL;
	/* -K contact by sockets */
	else if(argview[21].newval)
		subsys.contact=SRCSOCKET;

	/* display or not to display? */
	if(argview[4].newval)
		subsys.display=SRCYES;
	else if(argview[5].newval)
		subsys.display=SRCNO;

	/* add our little subsystem */
	rc=addssys(&subsys);
	switch(rc)
	{
	case 0:
		srcerr(SRC_BASE,SRC_SUBADD,SSHELL,subsys.subsysname,0,0,0);
		exit(0);
	case -1:
		srcerr(ODM_BASE,odmerrno,SSHELL,0,0,0,0);
		exit(1);
	default:
		srcerr(SRC_BASE,rc,SSHELL,subsys.subsysname,0,0,0);
		exit(1);
	}
}

