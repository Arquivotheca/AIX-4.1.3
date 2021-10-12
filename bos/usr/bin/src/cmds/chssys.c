static char sccsid[] = "@(#)67	1.16.1.1  src/bos/usr/bin/src/cmds/chssys.c, cmdsrc, bos411, 9428A410j 11/9/93 16:21:15";
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
**    Name:	chssys
**    Title:	Change Subsystem Object
** PURPOSE:
**	To Change subsystem difinitions for an existing subsystem object
**	in the subsystem object class.
** 
** SYNTAX:
**    chssys	-s old_subsystem_name [-s new_subsystem_name]\
**		[-t synonym] [-a command_aruments]\
**		[-p path] [-u userid] \
**		[-i stdin] [-o stdout] [-e stderr]\
**		[-R | -O] [-d | -D] [-q | -Q]\
**		[-I ipcquekey -m svrmtype | -S | -K]\
**		[-n signorm] [-f sigforce] [-E priority] [-G groupname]\
**		[-w wait_time]
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
**	-m server ICP Que mtype
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
#include <sys/signal.h>
#include <odmi.h>
#include "src.h"
#include "srcopt.h"
#include "srcobj.h"
#include "srcodm.h"
#include "srcaudit.h"

void finishupdate();

struct SRCsubsys subsys;
struct SRCsubsys Usubsys;
char subsysname[SRCNAMESZ];
char svr_keyname[SRCNAMESZ];

char synonym[SRCNAMESZ];
char tsubsysname[SRCNAMESZ];
char criteria[256];

struct argview argview[]=
{
	{sizeof(subsysname),(char *)subsysname,ODM_CHAR,'s',0,NOVIEWFIELD,0,SRCNAMESZ-1,SRC_SUBSYS2BIG},
	{sizeof(subsys.subsysname),(char *)subsys.subsysname,ODM_CHAR,'s',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_SUBSYS2BIG},
	{sizeof(subsys.synonym),(char *)subsys.synonym,ODM_CHAR,'t',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_SUBSYS2BIG},
	{sizeof(subsys.cmdargs),(char *)subsys.cmdargs,ODM_CHAR,'a',0,VIEWFIELD,0,SRCPATHSZ-1,SRC_CMDARG2BIG},
	{sizeof(subsys.path),(char *)subsys.path,ODM_CHAR,'p',0,VIEWFIELD,0,SRCPATHSZ-1,SRC_PATH2BIG},
	{sizeof(subsys.display),(char *)&subsys.display,FLAGSHORT,'d',0,VIEWFIELD},
	{sizeof(subsys.display),(char *)&subsys.display,FLAGFIELD,'D',0,VIEWFIELD},
	{sizeof(subsys.uid),(char *)&subsys.uid,ODM_LONG,'u',0,VIEWFIELD},
	{sizeof(subsys.multi),(char *)&subsys.multi,FLAGSHORT,'q',0,VIEWFIELD},
	{sizeof(subsys.multi),(char *)&subsys.multi,FLAGFIELD,'Q',0,VIEWFIELD},
	{sizeof(subsys.waittime),(char *)&subsys.waittime,ODM_SHORT,'w',0,VIEWFIELD},
	{sizeof(subsys.standin),(char *)subsys.standin,ODM_CHAR,'i',0,VIEWFIELD,0,SRCPATHSZ-1,SRC_STDIN2BIG},
	{sizeof(subsys.standout),(char *)subsys.standout,ODM_CHAR,'o',0,VIEWFIELD,0,SRCPATHSZ-1,SRC_STDOUT2BIG},
	{sizeof(subsys.standerr),(char *)subsys.standerr,ODM_CHAR,'e',0,VIEWFIELD,0,SRCPATHSZ-1,SRC_STDERR2BIG},
	{sizeof(subsys.action),(char *)&subsys.action,FLAGFIELD,'R',0,VIEWFIELD},
	{sizeof(subsys.action),(char *)&subsys.action,FLAGFIELD,'O',0,VIEWFIELD},
	{sizeof(subsys.signorm),(char *)&subsys.signorm,ODM_SHORT,'n',0,VIEWFIELD},
	{sizeof(subsys.sigforce),(char *)&subsys.sigforce,ODM_SHORT,'f',0,VIEWFIELD},
	{sizeof(subsys.grpname),(char *)subsys.grpname,ODM_CHAR,'G',0,VIEWFIELD,0,SRCNAMESZ-1,SRC_GRPNAM2BIG},
	{sizeof(subsys.contact),(char *)&subsys.contact,FLAGFIELD,'S',0,VIEWFIELD},
	{sizeof(subsys.svrkey),(char *)&subsys.svrkey,ODM_LONG,'I',0,VIEWFIELD},
	{sizeof(subsys.svrmtype),(char *)&subsys.svrmtype,ODM_LONG,'m',0,VIEWFIELD,1,0,SRC_MTYPE},
	{sizeof(subsys.contact),(char *)&subsys.contact,FLAGFIELD,'K',0,VIEWFIELD},
	{sizeof(subsys.priority),(char *)&subsys.priority,ODM_SHORT,'E',0,VIEWFIELD,MINPRIORITY,MAXPRIORITY,SRC_PRIORITY},
	{0}
};

/* valid user command line flags */
char argflags[]="s:t:p:u:i:o:e:ROn:f:I:SKm:E:G:a:DdQqw:";

struct objview sysview[]=
{
	{(char *)&subsys,(struct fieldview *)&Usubsys}
};
#define VSUBSYS	2
int n_sysview;

main(argc,argv)
char *argv[];
int argc;
{
	int rc;
	void *getrc;
	struct SRCsubsys tsubsys;

	/* setup NLS support */
	setlocale(LC_ALL,"");

	/* init subsys */
	memset(&subsys,0,sizeof(subsys));

	/* parse input arguments */
	n_sysview = parseopt(argc,argv,argview,argflags);

	/* validate data entered on command line */
	if(n_sysview < 2 || /* must have specified a field to be changed */
	   (int)argview[0].newval == 0 ||	/* subsysname is required */
	   /* may not specify both RESPAWN or ONCE */
	   ((int)argview[14].newval > 0 && (int)argview[15].newval > 0) ||
	   /* may not specify SOCKETS, MESSAGE QUEUES and SIGNAL */
	   ((int)argview[20].newval + (int)argview[19].newval 
		+ (int)argview[22].newval > 1) ||
	   /* must specify signals when you are using signals */
	   ((int)argview[19].newval + (int)argview[16].newval 
		+ (int)argview[17].newval != 0 &&
	    (int)argview[19].newval + (int)argview[16].newval 
		+ (int)argview[17].newval != 3) ||
	   /* when ICP is entered svrmtype must be entered */
	   ((int)argview[21].newval > 0 && (int)argview[20].newval == 0) ||
	   ((int)argview[21].newval == 0 && (int)argview[20].newval > 0) ||
	   /* may not specify display and no display */
	   ((int)argview[5].newval > 0 && (int)argview[6].newval > 0) ||
	   /* may not specify multi and not multi */
	   ((int)argview[8].newval > 0 && (int)argview[9].newval > 0))
	{
		srcerr(SRC_BASE, SRC_CHSSYS,SSHELL,0,0,0,0);
		exit(1);
	}

	/* -O restart action of ONCE */
	if(argview[15].newval)
		subsys.action=ONCE;

	/* -R restart action of RESPAWN */
	if(argview[14].newval)
		subsys.action=RESPAWN;

	/* contact by IPC queue? */
	if(argview[20].newval)
	{
		argview[19].newval++;
		subsys.contact=SRCIPC;
		rc=ckcontact(&subsys);
	}
	/* contact by signals? */
	else if(argview[19].newval)
	{
		subsys.contact=SRCSIGNAL;
		argview[20].newval++;
		rc=ckcontact(&subsys);
	}
	/* contact by sockets */
	else if(argview[22].newval)
	{
		subsys.contact=SRCSOCKET;
		rc=ckcontact(&subsys);
	}
	else
		rc=0;
	
	if(rc!=0)
	{
		srcerr(SRC_BASE,rc,SSHELL,0,0,0,0);
		exit(1);
	}


	/* Build the object view */
	n_sysview=bldview(argview,&sysview->fieldview,&subsys,&Usubsys);
	if(n_sysview <= 0)
	{
		srcerr(SRC_BASE, SRC_CHSSYS,SSHELL,0,0,0,0);
		exit(1);
	}

	/* init ODM */
	if(src_odm_init() != 0)
	{
		srcerr(ODM_BASE, odmerrno,SSHELL,0,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	/* subsystem must already exist */
	sprintf(criteria,"subsysname = '%s'",subsysname);
	getrc = odm_get_first(SRCSYSTEM,criteria,&Usubsys);
	if((getrc == NULL) || (getrc == -1))
	{
		srcerr(odmerrset(rc),odmerrmap(rc,SRC_SSME),SSHELL,subsysname,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	/* new subsys name must not exist */
	if(argview[1].newval && strcmp(subsysname,subsys.subsysname) != 0)
	{
		sprintf(criteria,"subsysname = '%s'",subsys.subsysname);
		getrc = odm_get_first(SRCSYSTEM,criteria,(char *)0);
		if((getrc != NULL) && (getrc != -1))
		{
			srcerr(SRC_BASE,SRC_SUBEXIST,SSHELL,subsys.subsysname,0,0,0);
			src_odm_terminate(TRUE);
			exit(1);
		}
	}

	/* new synonym entered  must not exist*/
	if(*subsys.synonym != '\0' &&
		argview[2].newval && strcmp(Usubsys.synonym,subsys.synonym) != 0)
	{
		sprintf(criteria,"synonym = '%s'",subsys.synonym);
		getrc = odm_get_first(SRCSYSTEM,criteria,(char *)0);
		if((getrc != NULL) && (getrc != -1))
		{
			srcerr(SRC_BASE, SRC_SYNEXIST,SSHELL,subsys.synonym,0,0,0);
			src_odm_terminate(TRUE);
			exit(1);
		}
	}

	memcpy(&tsubsys,&Usubsys,sizeof(struct SRCsubsys));
	putdbfields(sysview->fieldview);
	rc=odm_change_obj(SRCSYSTEM,&Usubsys);
	free((char *) sysview->fieldview);

	/* we can only update a record if one existed */
	if(rc < 0)
	{
		srcerr(ODM_BASE,odmerrno,SSHELL,subsys.subsysname,0,0,0);
		src_odm_terminate(TRUE);
		exit(1);
	}

	src_odm_auditlog(AUDIT_SRC_CHSSYS,0,tsubsys.subsysname,
		SRCSYSTEM,&Usubsys,&tsubsys);

	finishupdate(subsysname,&subsys);

	src_odm_terminate(TRUE);
	srcerr(SRC_BASE, SRC_SUBCH,SSHELL,subsys.subsysname,0,0,0);
	exit(0);
}
