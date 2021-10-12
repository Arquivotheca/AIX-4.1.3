static char sccsid[] = "@(#)95	1.11.1.1  src/bos/usr/bin/src/cmds/tracfresh.c, cmdsrc, bos411, 9428A410j 11/9/93 16:18:47";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	main,do_srcsrqt
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
**    Name:	traceson
**    Title:	Turn tracing on in subsystem or subserver
**    Name:	tracesoff 
**    Title:	Turn tracing off in subsystem or subserver
**    Name:	refresh
**    Title:	Refresh subsystem
** PURPOSE:
**	To do one of the following
**	   traceon: to send msg to subsystem to turn user defined tracing on
**	   traceoff: to send msg to subsystem to turn user defined tracing off
**	   refresh: to send msg to subsystem to do user defined 
**		refresh/restart of subsubsystem
** 
** SYNTAX:
**    traceon [-l] -s subsystem | -p subsys_pid | -g group_name \
**	| -t subserver_type [-o subserver_object | -p subserver_pid]
**    traceoff -s subsystem | -p subsys_pid | -g group_name \
**	| -t subserver_type [-o subserver_object | -p subserver_pid]
**    refresh -s subsystem | -p subsys_pid | -g group_name 
**
**    Arguments:
**	-l - long trace
**	-p subsys_pid - subsystem pid to act on
**	-s subsystem - subsystem to act on
**	-g group_name - subsystem group to act on
**	-t subserver_type - subserver_type to act on
**	-o subserver_object - subserver object to act on
**	-p subserver_pid - subserver pid to act on
**
** INPUT/OUTPUT SECTION:
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
#include "srcodm.h"
#include "srcobj.h"

/* this program will create one of three executables
**	1. traceson
**	2. tracesoff
**	3. refresh
**
** To create traceson one must place -DCMD_TRACESON on the complile line.
** To create tracesoff one must place -DCMD_TRACESOFF on the complile line.
** To create refresh one must place place -DCMD_REFRESH on the complile line.
*/

extern short Long;  /* long trace */
/* return request buffer */
static struct srcrep reqbuf;


main(argc,argv)
int argc;
char *argv[];
{
	int rc;
	void *addr_rc;
	short reply_len;    /* length of reply          */
	short request_len;  /* length of request        */
	char host[HOSTSIZE];
	char subsystem[SRCNAMESZ];
	short object;
	char objname[SRCNAMESZ];
	long subsys_pid;

	struct subreq subreq;

	/* traceson allows a -l option */
#ifdef CMD_TRACESON	
#define CMDARGS	"h:ls:g:o:t:p:P:"
	extern short Nolong;
	Nolong=FALSE;
#else
#ifdef CMD_TRACESOFF	
#define CMDARGS	"h:s:g:o:t:p:P:"
#else
#ifdef CMD_REFRESH
#define CMDARGS	"h:s:g:p:"
#endif
#endif
#endif

	/* setup NLS support */
	setlocale(LC_ALL,"");

	/* parse those command line arguments */
	rc=cmdargs(argc,argv,host,subsystem,&subsys_pid,&object,objname,(char *)0,(char *)0,CMDARGS);

	/* something was not liked with the input arguments */
	if(rc!=SRC_OK)
	{
		if(rc==SRC_PARM)
		{
#ifdef CMD_TRACESON
			srcerr(SRC_BASE, SRC_TRACEON, SSHELL, 0,0,0,0);
#else
#ifdef CMD_TRACESOFF
			srcerr(SRC_BASE, SRC_TRACEOFF, SSHELL, 0,0,0,0);
#else
#ifdef CMD_REFRESH
			srcerr(SRC_BASE, SRC_REFRESH, SSHELL, 0,0,0,0);
#endif
#endif
#endif
		}
		exit(1);
	}

	bzero(&subreq,sizeof(subreq));
	subreq.object = object;
	strcpy(subreq.objname,objname);
#ifdef CMD_TRACESON
	subreq.action = TRACE;
	subreq.parm2 = TRACEON;
	if(Long)
		subreq.parm1 = LONGTRACE;
	else
		subreq.parm1 = SHORTTRACE;
#else 
#ifdef CMD_TRACESOFF
	subreq.action = TRACE;
	subreq.parm2 = TRACEOFF;
#else
	subreq.action = REFRESH;
#endif
#endif

	/* send msg to group of subsystems */
	if(* subsystem == * SRCGROUP)
	{
		struct SRCsubsys subsys;
		char criteria[256];

		rc = src_odm_init();
		if(rc < 0)
		{
			srcerr(ODM_BASE,odmerrno,SSHELL,0,0,0,0);
			exit(1);
		}
		addr_rc = odm_open_class(SRCSYSTEM);
		if(addr_rc == -1)
		{
			srcerr(ODM_BASE,odmerrno,SSHELL,SRCSYSTEM,0,0,0);
			exit(1);
		}
		sprintf(criteria,"grpname = '%s'",&subsystem[1]);
		addr_rc = odm_get_first(SRCSYSTEM,criteria,&subsys);
		while((addr_rc != NULL) && (addr_rc != -1))
		{
			request_len = (short) sizeof(struct subreq);
			reply_len = (short) sizeof(struct svrreply);
			do_srcsrqt(host,subsys.subsysname, (int)subsys_pid, request_len, &subreq, &reply_len, &reqbuf);
			addr_rc = odm_get_next(SRCSYSTEM,&subsys);
		}
		src_odm_terminate(TRUE);
		exit(0);
	}

	src_odm_terminate(TRUE);

	request_len = (short) sizeof(struct subreq);
	reply_len = (short) sizeof(struct svrreply);

	/* send msg to single subsystem */
	rc = do_srcsrqt(host,subsystem, (int)subsys_pid, request_len, &subreq, &reply_len, &reqbuf);

	/* error on our command ? */
	if(rc != SRC_OK)
		exit(1);
	exit(0);
}

int do_srcsrqt(host,subsystem, subsys_pid, request_len, subreq, reply_len, reqbuf)
char *host;
char *subsystem;
int subsys_pid;
short request_len;
struct subreq *subreq;
short *reply_len;
struct srcrep *reqbuf;
{
	int rc;
	int cont=NEWREQUEST;

	/* send msg to subsystem */
	rc = srcsrqt(host,subsystem, subsys_pid, request_len, subreq, reply_len, reqbuf,SRCNO,&cont);
	/* error on our command ? */
	switch(rc)
	{
	case SRC_OK:
#ifdef CMD_TRACESON
		srcerr(SRC_BASE, SRC_TRACONOK, SSHELL, reqbuf->svrreply.objname,0,0,reqbuf->svrreply.rtnmsg);
#else
#ifdef CMD_TRACESOFF
		srcerr(SRC_BASE, SRC_TRACOFFOK, SSHELL, reqbuf->svrreply.objname,0,0,reqbuf->svrreply.rtnmsg);
#else
#ifdef CMD_REFRESH
		srcerr(SRC_BASE, SRC_REFOK, SSHELL, reqbuf->svrreply.objname,0,0,reqbuf->svrreply.rtnmsg);
#endif
#endif
#endif
		break;
	case SRC_CONT:
		if(subsys_pid==0)
			srcerr(SRC_BASE, rc, SSHELL, subsystem,0,0,0);
		else
		{
			char pid[30];
			sprintf(pid,"%d",subsys_pid);
			srcerr(SRC_BASE, rc, SSHELL, pid,0,0,0);
		}
		break;
	default:
		srcerr(SUBSYS_BASE, rc, SSHELL, whattoken(rc,SRC_UHOST,host,reqbuf->svrreply.objname), host,0,reqbuf->svrreply.rtnmsg);
		break;
	}
	return(rc);
}
