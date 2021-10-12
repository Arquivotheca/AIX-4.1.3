static char sccsid[] = "@(#)27	1.28  src/bos/usr/bin/que/enqjdf.c, cmdque, bos411, 9428A410j 1/28/93 08:15:46";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: outjdf ismatch
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* 
	No knowledge of the structure of the JDF (Job Description File)
	should exist outside of this source file or qdjdf.c.
	Any routine which reads or writes the JDF should use the routines in
	these modules.

	(outjdf(), and getjdf() know the order of records in the JDF.)

	see qdjdf.c for more info

*/


#include <stdio.h>
#include <unistd.h>
#include <IN/standard.h>
#include <sys/param.h>
#include <IN/backend.h>
#include <sys/utsname.h>
#include "common.h"
#include <errno.h>
#include "enq.h"
#include <security.h>
#include <sys/access.h>
#include <sys/id.h>
#include <ctype.h>

#include "enq_msg.h"
#define MSGSTR(num,str)	catgets(catd,MS_ENQ,num,str)
nl_catd	catd;

extern int qdaemon;	/*set in qmain or qstatus.c enq.c. Global Control 
				Flag. sorry.*/
extern char jdf[JDF_NAM_LEN];   /* to hold jdf name */

char    	*scopy();

char * hostname();
char * grreqinfo();
char * incjobnum();
extern uid_t  invokers_uid;
extern uid_t  programs_uid;
extern char  jobid[]="XXXXXXXXXX";



/* queue the entry described in qe.  str, if non-NULL, is name of file. */
/* see qentry [files] for format description. */
/* OUT Job Description File*/
outjdf(p,qe,filename,cmd_line)
register struct qe *qe;
register struct params *p;
char * filename;
char * cmd_line;
{
	int groups[17], i, ngroups;
	register FILE *tp;
	char tnam[MAXPATHLEN], qnam[MAXPATHLEN];
/*	struct file_id *f, *f1; */  /* Not used or declared */

	gid_t	effgid,getgidx(int);


#ifdef  DEBUG
	if( getenv("OUTJDF") )
		sysraw("outent\n" );
#endif

	cd(QUEDIR);

	/* get a tmp file */
	/* qdaemon ignores names that begin with a 't' */

	if( gettmp(NULL,tnam) < 0 )
		badtmp(QUEDIR);
	
	/* acquire the privilege to make the chown() call	*/
	/* toggle to the privilege domain of the program */
	seteuid(programs_uid);


	/* get the effective gid to chown() the JDF file	*/
	if ((effgid=getgidx(ID_EFFECTIVE)) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGGIDX,"Cannot get effective gid. Errno = %d"),errno);

	/* change the owner of JDF file from usr to a system user (root)
	 * and effective group (printq)
	 * this will protect the JDF file from tampering by a user.	
	 */
	if (chown(tnam, (uid_t)OWNER_ADMDATA, effgid) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGCHWN,"Cannot chown %s. Errno = %d"),tnam,errno);

	/* set the permissions on the JDF file	*/
	if (acl_set(tnam, R_ACC|W_ACC, R_ACC|W_ACC, NO_ACC) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGPERM,"Cannot set permissions on %s. Errno = %d"),tnam,errno);


	/* get the job number to put in the jdf file */
	incjobnum(jobid);

	/* redrop our privileges until we need them again	*/
	/* toggle to the privilege domain of the invoker */
	seteuid(invokers_uid);



	if ((tp = fopen(tnam,"w")) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGOOPN,"Unable to open %s."),tnam);

	/* and put the entry into file */
	/* warning -- anything we want to read in with scanf must end */
	/* with a blank, since we throw away newlines on input */


	fprintf(tp,"%s",jobid);		/* put jobnum here */

	/* XXXXXXX 0 1 15			devname -cp -no priority mailonly?*/
	fprintf(tp,"%s %d %d %d %d %d %d \n",
		(qe->qe_dev ? qe->qe_dev->d_name : ANYDEV),
		(int)p->p_cflg,
		(int)p->p_nflg,
		(int)qe->qe_priority,
		(int)p->p_Cflg,
		(int)p->p_Hflg,
		(int)qe->qe_date);


	/* size of request, bp stuff */

	fprintf(tp,"%d %s %d %d\n",
		   qe->qe_numblks,
		   p->p_Nrem? p->p_Nrem : "1",	/* default number of copies=1*/
		   qe->qe_head,
		   qe->qe_trail);

	/* identify user */

	fprintf(tp,"%s %c \n",
		qe->qe_logname,
		qe->qe_acct);

	/* add pcred (credentials) list. Contains uids,gids,groups,etc.	
	 * The list is null separated, double-null terminated		
	 */
	while(*(qe->qe_pcred))
		fprintf(tp,"%s%c",*qe->qe_pcred++,NULL); 
	putc('\0',tp);	/* add the terminator */
	putc('\n',tp);	/* add newline	*/

	/* add penv (environment) list. Contains the process enviromental 
	 * variables.
	 * The list is null separated, double-null terminated		
	 */
	while(*(qe->qe_penv))
		fprintf(tp,"%s%c",*qe->qe_penv++,NULL); 
	putc('\0',tp);	/* add the terminator */
	putc('\n',tp);	/* add newline	*/

	/*  enq command line - needs to be put in status file for piobe */
	/*  this is now terminated by "\0\n". */
	fprintf (tp, "%s\n", cmd_line);
	putc('\0',tp);	/* add the terminator */
	putc('\n',tp);	/* add newline	*/

	/*  request */

	fprintf(tp,"%s\n",qe->qe_reqname);

	/* line 13 -- user output is for */

	fprintf(tp,"%s\n",qe->qe_to);

	/* line 14 -- secondary host name */

	fprintf(tp,"%s\n", localhost());

	/* line 15,... -- operator messages, variable length */

	getopmsg(tp);



/* untransformed current directory, needed by backup queue */
/*
#ifdef DEBUG 
	if (getenv("OUTJDF"))
		sysraw("curdir = (%s)\n",qe->qe_curdir);
#endif
	fprintf(tp,"%s\n", qe->qe_curdir);
*/
	/* transformed current directory */

	/*file name(s) and unrecognized args and keywords */
	dumpargs(tp);

	/* entry complete */
	if ( fclose(tp) == EOF ) {
	    unlink(tnam);
	    syserr((int)EXITFATAL,MSGSTR(MSGNCLS,"Unable to close file : %s."),tnam);
	}
	sprintf(qnam, "qa%s:%s",
		qe->qe_logname,
		qe->qe_queue->q_name);
	
	qlink(tnam,qnam);
	strncpy(jdf,qnam,JDF_NAM_LEN-1);

}


/*Are the 	jobnumber,    (line 1 of jdf) and the 
		device name   (line 2 field 1) correct.
return the jobnumber we found or zero if no match.
*/
int ismatch(ename,qe,anynum)
struct qe *qe;
char *ename;
boolean anynum;			/* is anynumber a match? */
{
	FILE *qefil;
	register int answer;
	register char *myname;
	struct e estruct, *e = &estruct;
	int jobnuminfile;
	char * en;

	bzero((char *)e,sizeof(struct e)); 	/* nuke e struct */
	myname =  qe->qe_logname;


	/* open the entry file */
	if ( jdfopen(ename,&qefil))
	{   
	    answer = FALSE;	/* couldn't open*/
	    goto noclose;
	}
		/* need 2 get jobnuminfile incase -X specified */
	if  ( !rightjob1(qe->qe_jobnum,qefil,&jobnuminfile))
		if (!anynum)
		{   answer = FALSE;	/* wrong jobnumber */
		    goto end;
		}

	/* device match? */
	if (qe->qe_dev == NULL)       /* none given */
	{   answer = jobnuminfile;		/* i.e. TRUE */
	    goto end;
	}

	if (en = grreqinfo(e,qefil,NULL))	/* get device number*/
	{
		badentry(e,qefil,en);
		return(FALSE);
	}

	if (strcmp(qe->qe_dev->d_name,e->e_dnam))   /* strings don't match */
	    answer = FALSE;
	else
	    answer = jobnuminfile;			/* i.e. TRUE */

end:    jdfclose(qefil);
noclose:
	return(answer);
}
