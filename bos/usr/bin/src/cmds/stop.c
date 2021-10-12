static char sccsid[] = "@(#)87	1.8  src/bos/usr/bin/src/cmds/stop.c, cmdsrc, bos411, 9428A410j 2/26/91 14:51:29";
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
**    Name:	Stop
**    Title:	Stop Subsystem/Subserver
** PURPOSE:
**	To send a request to SRC to stop a subsystem.
**	To send a request through SRC to a subsystem for the stop 
**	or a subserver.
** 
** SYNTAX:
**	stop [-h host] [-f | -c] -a 
**	stop [-h host] [-f | -c] -g grpname
**	stop [-h host] [-f | -c] -s subsystem_name
**	stop [-h host] [-f | -c] -p subsystem_pid
**	stop [-h host] [-f] -t subserver_type [-o subserver_object] [-p subsystem_pid]
**	stop [-h host] [-f] -t subserver_type [-P subserver_pid] [-p subsystem_pid]
**    Arguments:
**	-a : stop of all subsystem is requested
**	-f : FORCED stop is requested
**	-c : CANCEL stop is requested
**	-h host: target host stop request applies to
**	-s subsystem_name: subsystem stop requested for
**	-g groupname: subsystem group that is stop requested for
**	-t subserver_type: subserver that is stop requested for
**	-o subserver_object: subserver defined object ect
**	-P subserver_pid: subserver pid stop is requested on
**	-p subsystem_pid: PID of subsystem that this stop request is
**		to be sent to (only need when multipule instances of the
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

/*--------------------------*/
/* External Functions Used  */
/*--------------------------*/
extern int cmdargs() ;                     /* Parse command            */
extern int  srcstop();                     /* src stop subsys routine  */
extern int  srcsrqt();                     /* src send request         */
extern void  srcerr();                     /* src error routine        */
extern void exit();                        /* exit function            */


main(argc,argv)            /* input from keyboard                   */
int argc ;                 /*  number of parameter arguments        */
char *argv[];              /*  array pointing to strings of args    */
{
	struct subreq subreq ;            /* storage for send request */
	struct svrreply reqbuf;           /* storage for reply        */
	struct svrreply *reqptr;          /* pointer to request       */

	struct srcrep subreqbuf;

	int rc;                           /* return code              */
	int msg_base;                     /* base # of error code     */
	char   host[HOSTSIZE];            /* host (default is local)  */
	char   subsystem[SRCNAMESZ];      /* profile name for subsys  */
	char objname[SRCNAMESZ];          /* subserver profile name   */
	short  object;
	long   subsys_pid;                /* pid for this subsys      */
	short  reqlen;                    /* length of request        */
	short  stoptype;                  /* stop options parm        */
	short  replen;                    /* length of reply          */
	int cont;			/* continuetion */

	extern short Stopforce;
	extern short Stopcancel;

	stoptype = NORMAL;                /* set stoptype to normal   */

	/* setup NLS support */
	setlocale(LC_ALL,"");

	rc=cmdargs(argc,argv,host,subsystem,&subsys_pid,&object,objname,0L,0L,"acfp:s:g:o:t:h:P:");

	if(rc < 0)
	{
		if(rc == SRC_PARM)
			srcerr(SRC_BASE, SRC_STOP, SSHELL, 0, 0,0,0);
		(void) exit(1);
	}

	if(Stopforce)
		stoptype = FORCED;
	else if(Stopcancel)
		stoptype = CANCEL;

	if (object == SUBSYSTEM)
	{
		/* we are going to stop a subsystem */

		replen = (short) sizeof(struct svrreply);
		reqptr = &reqbuf;

		/* request the subsystem to stop */
		rc = srcstop(host, subsystem, (int)subsys_pid,stoptype, &replen, reqptr, SSHELL);
		if(rc<0)
		{

			if(subsys_pid == 0)
				srcerr(SUBSYS_BASE, rc, SSHELL, whattoken(rc,SRC_UHOST,host,subsystem), 0,0,0);
			else
			{
				char tpid[20];
				sprintf(tpid,"%d",(int)subsys_pid);
				srcerr(SUBSYS_BASE, rc, SSHELL, whattoken(rc,SRC_UHOST,host,tpid), 0,0,0);
			}
			(void) exit(1);
		}
		exit(0);
	}
	else
	{
		/* we are going to request the stop of a subserver */

		replen = (short) (sizeof(struct svrreply) + sizeof(struct srchdr));
		bzero(&subreqbuf,sizeof(struct srcrep));
		bzero(&subreq,sizeof(subreq));
		subreq.object = object; /* subserver code point */
		subreq.action = STOP;
		(void) strcpy (subreq.objname ,objname );
		subreq.parm1 = stoptype;
		reqlen = (short)sizeof(struct srcreq); /*size of request*/
		cont = NEWREQUEST;

		/* send stop subserver request to SRC so SRC can forward the
		** stop request on to the subsystem
		**/
		rc = srcsrqt(host, subsystem, subsys_pid, reqlen, &subreq, &replen, &subreqbuf, SRCNO, &cont);
		if(rc!=SRC_OK)
		{
			srcerr(SUBSYS_BASE, rc, SSHELL, whattoken(rc,SRC_UHOST,host,subreqbuf.svrreply.objname), 0, 0, subreqbuf.svrreply.rtnmsg);
			exit(1);
		}
		srcerr(SUBSYS_BASE, SRC_STOPSVROK, SSHELL, whattoken(rc,SRC_UHOST,host,subreqbuf.svrreply.objname), 0, 0, subreqbuf.svrreply.rtnmsg);
		exit(0);
	}
}
