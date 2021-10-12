static char sccsid[] = "@(#)85	1.5  src/bos/usr/bin/src/cmds/status.c, cmdsrc, bos411, 9428A410j 2/26/91 14:52:59";
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
**    Name:	Status
**    Title:	Status Subsystem/Subserver
** PURPOSE:
**	To send a request to SRC for status of a subsystem.
**	To send a request through SRC to a subsystem for status of the
**	subsystem or a subserver.
** 
** SYNTAX:
**	status [-h host] -a 
**	status [-h host] -g grpname
**	status [-h host] [-l] -s subsystem_name
**	status [-h host] [-l] -p subsystem_pid
**	status [-h host] [-l] -t subserver_type [-o subserver_object] [-p subsystem_pid]
**	status [-h host] [-l] -t subserver_type [-P subserver_pid] [-p subsystem_pid]
**    Arguments:
**	-a : short status of all subsystem is requested
**	-l : long status is requested
**	-h host: target host status request applies to
**	-s subsystem_name: subsystem status requested for
**	-g groupname: subsystem group that is status requested for
**	-t subserver_type: subserver that is status requested for
**	-o subserver_object: subserver defined object ect
**	-P subserver_pid: subserver pid status is requested on
**	-p subsystem_pid: PID of subsystem that this status request is
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
#include "srcopt.h"

#define NEWSTAT    NEWREQUEST              /* 1st time calling status  */
#define ENDSTAT    END                     /* No more status buffers   */

/* prstatus will print the status buffer if it points to something and then
** free the storage for that formated status buffer
**/
#define	prstatus(statbuff)\
{\
	if (statbuff != 0) \
	{\
		printf(statbuff);\
		free(statbuff);\
		statbuff=(char *)0;\
	}\
}
extern short Long;	/* Long status */
extern short Contact;	/* Contact type for subsystems */
extern short Smitformat;


main(argc,argv)            /* input from keyboard                   */
int argc ;                 /*  number of parameter arguments        */
char *argv[];              /*  array pointing to strings of args    */
{
	int cont;
	int rc;                       /* return code              */
	short object;
	long subsys_pid = 0;          /* pid for this subsys      */
	short stattype;               /* status type - long/short */
	char objname[SRCNAMESZ];      /* subserver profile name   */
	char subsystem[SRCNAMESZ];    /* profile name for subsys  */
	char host[HOSTSIZE];          /* host (default is local)  */
	char *statbuff;            /* status buffer to print   */

	stattype = SHORTSTAT;         /* default status to short  */

	/* setup NLS support */
	setlocale(LC_ALL,"");

	setgid(0);

	/* parse the command line flags and there arguments */
	rc=cmdargs(argc,argv,host,subsystem,&subsys_pid,&object,objname,0L,0L,"alp:s:g:o:t:P:h:NSTn:d");
	if (rc < 0)
	{
		if (rc == SRC_PARM)
			srcerr(SRC_BASE, SRC_STAT, SSHELL, 0, 0,0,0);
		(void) exit(1);
	}

	/* print odm files in smit format */
	switch(Smitformat)
	{

		case PRINTDEFAULTSUBSYSTEM:
			src_print_default_subsystem();
			exit(0);

		case PRINTSUBSYSTEM:
			if(*subsystem == '\0')
				src_print_all_subsystem();
			else
				src_print_one_subsystem(subsystem);
			exit(0);

		case PRINTSUBSERVER:
			if(*objname == '\0')
				src_print_all_subsvr();
			else
				src_print_one_subsvr(objname);
			exit(0);

		case PRINTNOTIFY:
			if(*objname == '\0')
				src_print_all_notify();
			else
				src_print_one_notify(objname);
			exit(0);

		default:
			break;
	}

	/* long status specified */
	if(Long)
	{
		/* when subsystem entered and Contact is signals no need to 
		** pass it on to src since SRC will reject it; however if the
		** subsystem has changed contact to signals we force the user
		** to do status long of subsystem by pid (very unlikely) this
		** assumes that the subsystem was active when it's object was
		** updated.
		**/
		if(object == SUBSYSTEM && Contact == SRCSIGNAL)
		{
			srcerr(SRC_BASE, SRC_CONT,SSHELL,subsystem,0,0,0);
			exit(1);
		}
		stattype = LONGSTAT;
	}

	/*---------------------------------------------------------------*/
	/* call srcsbuf to get buffer mngt and text processing           */
	/*---------------------------------------------------------------*/

	statbuff=(char *)0;                     /* init pointer to NULL     */
	cont = NEWSTAT ;                     /* init continued to new    */
	rc = 0;                              /* init return code to OK   */
	/* get our status */
	while (((rc = srcsbuf(host, object,
	    subsystem, objname, (int) subsys_pid, stattype,
	    SSHELL, &statbuff, &cont)) >= 0)
	    && (cont != ENDSTAT)) 
	{
		/* print formated status buffer if one was returned by srcsbufs */
		prstatus(statbuff);
	}

	/* print formated status buffer if one was returned by srcsbuf */
	prstatus(statbuff);

	/* exit 1 on srcsbuf error */
	if (rc < 0)
		exit(1);

	/* normal exit */
	exit(0);
}
