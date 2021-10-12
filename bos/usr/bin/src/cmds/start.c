static char sccsid[] = "@(#)83	1.10  src/bos/usr/bin/src/cmds/start.c, cmdsrc, bos411, 9428A410j 2/26/91 14:51:23";
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
 * (C) COPYRIGHT International Business Machines Corp. 1984,1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
** IDENTIFICATION:
**    Name:	start
**    Title:	Start Subsystem/Subserver
** PURPOSE:
**	To send a request to SRC to start a subsystem.
**	To send a request through SRC to the subsystem to start a subserver.
** 
** SYNTAX:
**	start [-h host] [-e env] [-a args] -s subsystem_name
**	start [-h host] [-e env] [-a args] -g groupname
**	start [-h host] -t subserver_type [-o subserver_object] [-p subsystem_pid]
**    Arguments:
**	-h host: target host start request applies to
**	-e env: evironment that subsystem will be started with
**	-a args: arguments that the subsystem will be passed on exec
**	-s subsystem_name: subsystem that is to be started
**	-g groupname: subsystem group that is to be started
**	-t subserver_type: subserver that is to be started
**	-o subserver_object: subserver defined object ect
**	-p subsystem_pid: PID of subsystem that this subserver is to be 
**		started for(only need when multipule instances of the
**		subsystem are running)
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

#include "src.h"
#include "src10.h"

main(argc,argv)            /* input from keyboard                   */
int argc ;                 /*  number of parameter arguments        */
char *argv[];              /*  array pointing to strings of args    */
{
	/* subserver start request buffer */
	struct subreq subreq;

	/* reply storage buffer for starting subserver */
	struct srcrep reqbuf;

	int rc;

	/* command line parameters */
	char objname[SRCNAMESZ];	/* subserver object (name or pid) */
	char subsystem[SRCNAMESZ];	/* subsystem name	*/
	char host[HOSTSIZE];		/* host (default is local)  */
	char env[ENVSIZE];		/* environment string       */
	char parms[PARMSIZE];		/* parameter strng          */
	int cont;			/* continueation */

	long subsys_pid;	/* subsystem pid used for starting subserver
				**   running on a particular active copy of 
				**   subsystem 
				**/

	/* object to be acted on(SUBSYSTEM or subserver code point)*/
	short  object;

	short  requestlen;	/* length of request        */
	short  replylen;	/* length of reply          */

	extern short Justpid;   /* just subsys pid is not allowed */

	Justpid=FALSE;

	/* setup NLS support */
	setlocale(LC_ALL,"");

	/* We have to make sure that the scrmstr is ready,
	   this is only a problem when system is first started
	   up and startsrc got fired up by init the same time
	   that srcmstr got started by init.
	 */
	sleep(3);

	/* parse the command line flags and there arguments */
	rc=cmdargs(argc,argv,host,subsystem,&subsys_pid,&object,objname,env,parms,"h:e:p:s:g:o:t:a:");

	/* error parsing the command line? */
	if (rc < 0)
	{
		if(rc==SRC_PARM)
			srcerr(SRC_BASE, SRC_STRT, SSHELL, 0, 0,0,0);
		(void) exit(1);
	}

	/* start of SUBSYSTEM calls srcstrt
	** start of SUBSERVER calls srcsrqt
	**/

	/* subsystem start request */
	if (object == SUBSYSTEM)
	{
		/* we are going to start a subsystem */

		/* send start subsystem request to SRC on the proper host */
		rc = srcstrt(host,subsystem,env,parms,RESP_USE,SSHELL);
		if(rc<0)
		{
			srcerr(SUBSYS_BASE, rc, SSHELL, whattoken(rc,SRC_UHOST,host,subsystem), 0,0,0);
			exit(1);
		}
		exit(0);
	}
	else 
	{

		/* we are going to start a subserver */

		/* init our request structure */
		bzero(&subreq,sizeof(subreq));

		/* set object to be the code point for the subserver to be started */
		subreq.object=object;

		/* subservers owner */
		strcpy(subreq.objname,objname);

		/* size of request */
		requestlen =  (short) sizeof(struct subreq);
		/* max size of reply */
		replylen =  (short) sizeof(struct srcrep);

		/* specify start action on subserver */
		subreq.action = START;
		/* initial request is being made */
		cont = NEWREQUEST;

		/* send request to SRC on the desired host
      		**    subserver start request will go directly to SRC on 
		**    the host to which the request will be serviced,
		**    bypassing the local SRC
		** Note: if the subsystem to which our subserver belongs is
		** is inoperative then SRC will attempt to start the subsystem
		** and then pass the start subserver command to the subsystem
      		**/
		rc=srcsrqt(host,subsystem,(int)subsys_pid,requestlen,&subreq,&replylen,&reqbuf,SRCON,&cont);
		if (rc != SRC_OK)
		{
			srcerr(SUBSYS_BASE, rc, SSHELL, whattoken(rc,SRC_UHOST,host,reqbuf.svrreply.objname), 0, 0, reqbuf.svrreply.rtnmsg);
			exit(1);
		} 
		srcerr(SUBSYS_BASE, SRC_STRTSVROK, SSHELL, whattoken(rc,SRC_UHOST,host,reqbuf.svrreply.objname), 0, 0, reqbuf.svrreply.rtnmsg);
		exit(0);
	}
}      
