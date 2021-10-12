static char sccsid[] = "@(#)66	1.42  src/bos/usr/bin/que/libque/qdjdf.c, cmdque, bos411, 9428A410j 3/30/94 10:02:42";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: grjobnum, grreqinfo, grreqsize, grlog, grpcred, grpenv, grreqnam, 
 *            grforuser, gr2ndhost, get_msg, gropmsg, grargs2, grargs, jdfclose,
 *            jdfopen, badentry, jdfree, getjdf, jdfnewpri, grcmdline
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */


/* 
	No knowledge of the structure of the JDF (Job Description File)
	should exist outside of these source file.
	Any routine which reads or writes the JDF should use the routines in
	these modules.

	(outjdf(), and getjdf() know the order of records in the JDF.)

	Routines to Read and Write JDF file

(ROUTINES THAT READ THE JDF)
The following are routines visible from the outside:
grjobnum 			- read jobnum, qdaemon write jobnum
getjdf				- prepares to exec backend.

The following are not routines visible from the outside:
grreqinfo 			- (dev# -cp -no priority mail_only?)           
grreqsize 			- read # copies headers and trailers
gr2ndhost			- read hostname
grargs				- read filenames and backend arguments
AIX security enhancement - removed grcongrps(). pcred contains concurrent groups.
AIX security enhancement - removed grdir(). penv contains current directory.
grforuser			- read username to deliver to
grmoddate			- read file modification date
gropmsg				- read operator messages.
grreqnam			- read request name.
AIX security enhancement - renamed gruid() to grlog().
grlog				- read user logname and acct.
AIX security enhancement - added grpcred
grpcred				- read user process credentials
AIX security enhancement - added grpenv
grpenv				- read user process environment
grcmdline			- read enq command line


(ROUTINES THAT WRITE THE JDF)   - found in enqjdf.c
outjdf				- create a jdf.	

(MISCELLANEOUS ROUTINES FOR THE JDF)
The following are routines visible from the outside:
jdfclose			- close the jdf.
jdffree				- free all malloc'd space
jdfopen				- open the jdf.

			Structure of the JDF.
NEW					Job number (NEW means not assigned yet) 
X 0 1 15 1 XXXXXXX			dev# -cp -no priority mail_only?
					time submitted.
4 1 3 3					size copies header trailer
AIX security enhancement - removed uid and gid, pcred contains these
curt 0 					username acctfile
AIX security enhancement - removed concurrent group list, pcred contains this
"process credentials"			pcred
"process environment"			penv
wert					reqname
curt					for username
outpost					hostname
Mon Jan  9 13:30:02 1989		mod date
0					operator messages
AIX security enhancement - removed current directory, penv contains this
/tmp/wert 0				full filename + rm_at_end?

*/


#define _ILS_MACROS
#include <stdio.h>


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

#include "libq_msg.h"

extern int qdaemon;	/*set in qmain or qstatus.c enq.c. Global Control 
				Flag. sorry.*/
char    	*scopy();


/****************************************************************/
/* Get Record JOB NUMber		  			*/
/* If qdaemon, the presence of a job number means		*/
/* that we've already read this entry once, and can flag it	*/
/* if qstatus, we want to read it anyway			
		write a newjobnum into the jdf
****************************************************************/
char * grjobnum(e,efil,rc)
register struct e 	*e;
register FILE 		*efil;
int			*rc;
{
	char qeline[QELINE];
	char jobnum[QELINE];
	
#ifdef DEBUG
	if (getenv("GRJOBNUM"))
		sysraw("in grjobnum\n");
#endif
	if (!getline(qeline,QELINE,efil))
		return(GETMESG(MSGNJOB,"Job number record missing in Job Description File."));
	if (sscanf(qeline,"%s", jobnum) != 1)
		return(GETMESG(MSGRJOB,
			"Job number record must contain 1 element in Job Description File"));


	if (!(e->e_qorder=atol(jobnum)))
	{
		*rc=(NOTOK);
		return(GETMESG(MSGNONN,
			"JDF: Job number is nonnumeric, or 0."));
	}
	e->e_jobnum = JOBNUM(e->e_qorder);
	if (e->e_s)
		e->e_s->s_jobnum=e->e_jobnum;
	  
#ifdef DEBUG
	if(getenv("GRJOBNUM"))
	{
		sysraw("old jobnum %d\n",e->e_jobnum);
		if (e->e_s)
			sysraw("sj %d,ej %d\n",e->e_s->s_jobnum,e->e_jobnum);
		else
			sysraw("ej %d\n",e->e_s->s_jobnum,e->e_jobnum);
	}
#endif
		
	*rc=NOTNEW;
	return(OK);	/*return record ok*/
}


/***************************************************************************/
/* Get Record REQuest INFOrmation                                          */
/* 	(dev# -cp -no priority mail_only?)                                 */
/* 	                                                                   */
/* if you change this routine, take a look at everyone who calls him	   */
/***************************************************************************/
char * grreqinfo(e,efil,where)
register struct e 	*e;
int			*where;
FILE 			*efil;
{
	char qeline[QELINE];

#ifdef DEBUG
	if (getenv("GRREQINF"))
	{
		sysraw("in grreqinfo ftell=%d\n",ftell(efil));
		system("cat *>/tmp/bgr");
	}	
#endif
	if (where)
		*where=ftell(efil);

	if (!getline(qeline,QELINE,efil))
		return(GETMESG(MSGMREC,
			"Request info record missing in Job Description File."));

#ifdef DEBUG
	if (getenv("GRREQINF"))
	{
		system("cat *>/tmp/agr");
		sysraw("hello|%s|",qeline);
	}	
#endif

	if ((sscanf(qeline,"%s %d %d %d %d %d %d", 
			e->e_dnam, &(e->e_cp), &(e->e_no),
			&(e->e_pri1), &(e->e_mail), &(e->e_hold),
			&(e->e_time)
		  ))!=7)
		return(GETMESG(MSGSEV,
			"Request info record must contain 7 elements in Job Description File."));

	if (e->e_s)
		e->e_s->s_mailonly=e->e_mail;	/* put in statfile struct 4 backend*/

	return(OK);	/*return record ok*/
}

/******************************************/
/* Get Record REQuest SIZE                */
/* (size copies header trailer)           */
/******************************************/
char * grreqsize(e,efil)
register struct e               *e;
register FILE                   *efil;
{
        char qeline[QELINE];

        if (!getline(qeline,QELINE,efil))
                return(GETMESG(MSGMISS,
			"Request size record missing in Job Description File."));
        if (sscanf(qeline,"%d %d %d %d", &(e->e_size), &(e->e_nc),
				&(e->e_s->s_head), &(e->e_s->s_trail)) != 4)
                return(GETMESG(MSGSIZR,
			"Size record must contain 4 elements in Job Description File."));

	e->e_s->s_copies = e->e_nc;

        return(OK);     /*return record ok*/
}


 


/******************************************/
/* getrec -- user logname	          */
/* Get LOG record			  */
/* 	(logname and acctfile)  	  */
/******************************************/
/* AIX security enhancement 				*/
/* renamed gruid() to grlog()				*/
/* removed e_uid and e_gid - e_pcred includes these	*/
/* therefore, removed all references to uid and gid below */
char * grlog(e,efil)
register struct e 		*e;
register FILE 			*efil;
{
	char qeline[QELINE];
	char u_acct[2];         /* acct char, then null byte */
	char u_name[S_FROM];    /* must be 64 characters, 31 for nodeid(H)
	                           31 for user name(C) + @ + and null byte */
				/* RFC1179 documentation */


	if (!getline(qeline,QELINE,efil))
		return(GETMESG(MSGMLOG,"LOG record missing in Job Description File."));
	if (sscanf(qeline,"%s %s", u_name, u_acct) != 2)
		return(GETMESG(MSGTWO,"LOG record must contain 2 elements in Job Description File."));
	e->e_from = scopy(u_name);
	strncpy(e->e_s->s_from,e->e_from,S_FROM);
	e->e_acct = u_acct[0];

	return(OK);	/*return record ok*/
}

/* AIX security enhancement 		  */
/******************************************/
/* grpcred -- process credentials list   */
/* get pcred list			  */
/******************************************/
char * grpcred(e,efil)
register struct e 		*e;
register FILE 			*efil;
{
	char	**getarray();
 
	if ((e->e_pcred = getarray(efil)) == NULL)
		return(GETMESG(MSGPCRD,"pcred list missing in Job Description File."));

	return(OK);	/*return record ok*/
}

/* AIX security enhancement 		  */
/* added grpenv()			  */
/******************************************/
/* grpenv -- process environment list     */
/* get penv list			  */
/******************************************/
char * grpenv(e,efil)
register struct e 		*e;
register FILE 			*efil;
{
	char	**getarray();

	if ((e->e_penv = getarray(efil)) == NULL)
		return(GETMESG(MSGPENV,"penv list missing in Job Description File."));

	return(OK);	/*return record ok*/
}


/* AIX security enhancement 						*/
/* removed cgl_init(), the concurrent group set is stored in e_pcred	*/
/* removed cgl_clear()							*/
/* removed grcongrps()							*/

/***************************/
/* getrec4 -- request name */
/* Get Record REQuest NAMe */
/***************************/
char * grreqnam(e,efil)
register struct e 		*e;
register FILE 			*efil;
{
	int reqlen;
	char *reqstart;
	char qeline[QELINE];

	if (!getline(qeline,QELINE,efil))
		return(GETMESG(MSGNAME,"Request Name record missing in Job Description File."));
	e->e_request = scopy(qeline);
	/* left-truncate the request name in the status file */
	reqstart = qeline;
	reqlen = strlen(qeline);
	if (reqlen >= S_TITLE)
	    reqstart += (reqlen - (S_TITLE-1));
	strncpy(e->e_s->s_title,reqstart,S_TITLE);
	return(OK);	/*return record ok*/
}



/******************************/
/* grcmdline -- command line */
/******************************/
char * grcmdline(e, efil)
register struct e 		*e;
register FILE 			*efil;
{
	int reqlen;
	char *reqstart;
	char qeline[QELINE];
	register int  c, cnt=0;

	/*
	 * The command line argument is now a terminated by a "\0\n"
	 * string. If this changes, enq and qdaemon should be changed.
	 */
	while ((c = fgetc(efil)) != EOF)
	{
	    if (c == '\0')
	    {
		c = fgetc(efil);
		if ( c == '\n' ) {
		    qeline[cnt] = 0;
		    break;
		}
	    }
	    if (cnt < QELINE)
		qeline[cnt++] = c;
	}

	if (c == EOF)
	    return(GETMESG(MSGCMD,"Command Line record missing in Job Description File."));
	e->e_cmdline = scopy(qeline);
	reqstart = qeline;
	reqlen = strlen(qeline);
	/* right truncate command line */
	if (reqlen >= S_CMDLEN)
		qeline[S_CMDLEN-1] = '\0';
	strncpy(e->e_s->s_cmdline,reqstart,S_CMDLEN);
	return(OK);	/*return record ok*/
}





/**********************************/
/* getrec7 -- user request is for */
/* Get Record FOR USER 		  */
/**********************************/
char * grforuser(e,efil)
register struct e 		*e;
register FILE 			*efil;
{
	char qeline[QELINE];

	if (!getline(qeline,QELINE,efil))
		return(GETMESG(MSGUSER,"For-User record missing in Job Description File."));
	e->e_to = scopy(qeline);
	strncpy(e->e_s->s_to,e->e_to,S_TO);
	return(OK);	/*return record ok*/
}


/************************************/
/* getrec secondary host-name		*/
/* Get Record 2NDary HOST name		*/
/************************************/
char * gr2ndhost(e,efil)
register struct e 		*e;
register FILE 			*efil;
{
	char host_name[HOSTNAME];
	char qeline[QELINE];

	if (!getline(qeline,QELINE,efil))
		return(GETMESG(MSGHOST,"Secondary Host Name record missing in Job Description File."));

	if (sscanf(qeline,"%s",host_name) != 1)
	{
		syswarn(GETMESG(MSGBLNK,"Secondary Host Name record blank in Job Description File.  Gethostname() is probably returning null."));
		e->e_hostname = NULL;
	}
	
	e->e_hostname = scopy(host_name);
	return(OK);	/*return record ok*/
}



/*****************************************/
/* gets the message from the entry file  */
/* and places it in one string           */
/*****************************************/
char *get_msg(c,qeline,efil,e)
register int c;
register char *qeline;
register FILE *efil;
register struct e *e;
{
	register int i;
	register size_t size;
	register char *msg=NULL;
	char hdr[MAXLINE];

	size = c * (QELINE + 2);
	msg = Qalloc(size);
/* AIX security enhancement			*/
/* added logname (e_from) to operator message	*/
	sprintf(hdr,GETMESG(MSGMESS,"\nMessage from host %s user %s...\n\n"), e->e_hostname,e->e_from);

	strcpy(msg,hdr);

	for (i=0; i < c; i++)
	{
		if (!getline(qeline,QELINE,efil))
		    return(0);
		strcat(msg,qeline);
		strcat(msg,"\n");
	}    /* end of for */
	return(msg);
}

/****************************************************************/
/* Number of operator message lines 				*/
/* this record is different from other records in that may be	*/
/* multiline.  line 9 contains the number of operator message	*/
/* lines.  If it is >0, this record is multiline.		*/
/* Get Record OPerator MeSsaGe					*/
/****************************************************************/
char * gropmsg(e,efil)
register struct e 		*e;
register FILE 			*efil;
{
	char qeline[QELINE];
	int opm_count;          /* number of lines for operator message */

	if (!getline(qeline,QELINE,efil))
		return(GETMESG(MSGOPER,
			"Operator Message Count record missing in Job Description File."));
	if (sscanf(qeline,"%d",&opm_count) != 1)
		return(GETMESG(MSGINTG,"Operator Message Count record is not integer."));
	if (opm_count != 0)
	{
		if ((e->e_msg = get_msg(opm_count,qeline,efil,e)) == NULL)
			return(GETMESG(MSGMBAD,"Operator Message record is bad."));
	}
	else
		e->e_msg = NULL;
	return(OK);	/*return record ok*/
}


/* AIX security enhancement 				*/
/* removed grdir(). e_penv contains the directory	*/

/* The purpose of this routine is to 				*/
/* store the pathname in the 'e' struct.			*/
/* it also stores it in e_jdfvec[argnum]			*/
/* CALLED BY grargs						*/
/* Get Record ARGuments	part 2					*/
char * grargs2(e,qeline,lf)
register struct e 	*e;
char 			*qeline;
struct str_list 	**lf ;			/* list of filenames */
{
	struct str_list *thisf,*lastf;		/* list of filenames */
	char 		*filename[QELINE];
	boolean		del; 		/* delete file after backend death?*/
	
#ifdef DEBUG
	if (getenv("GRARGST"))
		sysraw("in grargs2, qeline=%s\n",qeline);
#endif

	lastf = *lf ;	/* list of filenames */


	if (sscanf(qeline,"%s %d",filename,&del)!=2)
		return(GETMESG(MSGBFAD,"Bad filename in jdf.  Should have 2 fields."));

	thisf =  Qalloc( sizeof(struct str_list));

#ifdef DEBUG
	if (getenv("GRARGST"))
		sysraw("in grargs2, filename=%s,del=%d\n",filename,del);
#endif

	/* if the at the head of the list */
	if (lastf == NULL)
		e->e_fnames = lastf = thisf;
	else
	{ 	lastf->s_next = thisf;
		lastf = lastf->s_next;
	}
		/* make last next pointer be NULL */
	lastf->s_next = NULL;

#ifdef DEBUG
	if (getenv("GRARGST"))
		sysraw("in grargs2, to argvec\n");
#endif
	e->e_jdfvec[(e->e_jdfnum)++] = scopy(filename);

		/* e_fnames should get freed too */
	lastf->s_name = scopy(filename);
	lastf->s_del = del;
	*lf=lastf;
	return(OK);			/*return record ok*/
}

/*      Put all filenames into e_fnames and all fnames and backend options
	into e_jdfvec.
	CALLED BY grargs 
	CALLS grargs2 							
*/

char * grargs(q,e,efil)
register struct q *q;	/* used by grargs to determine remoteness of queue */
register struct e 		*e;
register FILE 			*efil;
{
	struct str_list *lastf = NULL;   /* list of filenames */
	char qeline[QELINE];
 	char * en=0;			/*error notification string*/
	boolean is_rem_q;		/* is this a surrogate queue */

	/*----Put the arguments and the file names in argvec.
	      they will be put in the proper spot when the backend
	      program is known at exec time. (from /etc/qconfig 
	      "backend = " line ) */
	is_rem_q = remote(q->q_hostname);
	e->e_jdfnum = 0;
	e->e_argnum = 0;		/* sanity */

	while (getline(qeline,QELINE,efil))
	{		 
		/*----If it's a flag or filename */
		/*----See if flag is prefaced with BOPTMARK (in the jdf. c common.h) */
		if (!strncmp(qeline,BOPTMARK,BOPTLEN))
		{
		 	/*----Copy the flag to argvec */
			if (is_rem_q)		/* if remote, preface flag */
			{
				e->e_jdfvec[(e->e_jdfnum)++] = scopy("-o");
				e->e_jdfvec[(e->e_jdfnum)++] = scopy(qeline + BOPTLEN);
			}
			else
				e->e_jdfvec[(e->e_jdfnum)++] = scopy(qeline + BOPTLEN);
		}
		else
			/*----Otherwise a file and remove flag in that line */ 		
			if (en = grargs2(e,qeline,&lastf))
				return(en);
	}
	return(OK);	/* Return value of OK  */
}


/* This routine closes the Job Description file.	*/
jdfclose(efil)
FILE   	*efil; 		  	    /* entry file pointer */
{      
	fclose(efil);
}      

/* This routine opens the Job Description file.	*/
char * jdfopen(ename,efil)
char	*ename;
FILE   	**efil; 		  	    /* entry file pointer */
{     
	if ((*efil = fopen(ename,"r+")) == NULL)
	        return(GETMESG(MSGQOPN,"Qdaemon couldn't open Job Description file"));
	else
		return(NULL);
}


/*
 * we have a badly-formatted entry in the queue directory.  identify
 * it, and rename it so it looks like a tmp file and won't get reread
 * over and over again.
 */
badentry(e,efil,errnot)
struct e		*e;
	 /* e is passed to free e_filename.unffp */	
register FILE   	*efil;		/* entry file pointer */
char 			*errnot;	/* error notification string */
{      
	char outofway[MAXPATHLEN];

#ifdef DEBUG
	if (getenv("BADENTRY"))
		sysraw("outofway=%s e->e_name=%s\n",outofway,e->e_name);
#endif

	strcpy(outofway,e->e_name);
	outofway[0] = 't';
	unlink(outofway);
	renamefile(e->e_name,outofway);
	syswarn(errnot);
	syswarn(GETMESG(MSGBFMT,"Entry %s/%s: bad format.\n Name changed to %s."),
					QUEDIR,e->e_name,outofway);
	jdffree(e);
	jdfclose(efil);
}


/*free up malloc'd structs. */
jdffree(e)
register struct e *e;
{
 	int i=0;
	register char **pTmp;

	for (i = 0; i < e->e_argnum; i++)
		if(e->e_argvec[i]) {
			free((void *)e->e_argvec[i]);
			e->e_argvec[i] = NULL;
		}
		
	for (i = 0; i < e->e_jdfnum; i++)
		if(e->e_jdfvec[i]) {
			free((void *)e->e_jdfvec[i]);
			e->e_jdfvec[i] = NULL;
		}
		
	if (e->e_msg) {
		free((void *)e->e_msg);
		e->e_msg = NULL;
	}
/* AIX security enhancement	*/
/* removed e_directory reference*/
	if (e->e_fnames) {
		 cleanlist(e->e_fnames);
		e->e_fnames = NULL;
	}
	if (e->e_s) {
		 free((void *)e->e_s);
		 e->e_s = NULL;
	}
	if (e->e_from) {
		 free((void *)e->e_from);
		 e->e_from = NULL;
	}
	if (e->e_to) {
		 free((void *)e->e_to);
		 e->e_to = NULL;
	}
	if (e->e_request) {
		 free((void *)(e->e_request));
		 e->e_request = NULL;
	}
	if (e->e_hostname) {
		 free((void *)(e->e_hostname));
		 e->e_hostname = NULL;
	}
	if (e->e_pcred) {
		for (pTmp=e->e_pcred; NULL != *pTmp; pTmp++) {
			free( (void *)*pTmp );
		}
		free( (void *)e->e_pcred );
		e->e_pcred = NULL;
	}
	if (e->e_penv) {
		for (pTmp=e->e_penv; NULL != *pTmp; pTmp++) {
			free( (void *)*pTmp );
		}
		free( (void *)e->e_penv );
		e->e_penv = NULL;
	}
}

/*Given an entry file name, return the q that it is on.*/
struct q * whatq(ql,name)
struct q * ql;
register char *name;
{
	struct q * qlp;
	for (qlp = ql; qlp; qlp = qlp->q_next)
		if( !strcmp(getqn(name),qlp->q_name ) )
			return(qlp);

/*	If this point reached, there is a weird name in the qdir*/
	return(NULL);
}




/* AIX security enhancement 			*/
/* removed --> char * get_security_labels (); 	*/


	/* open the jdf file, read everything and close it.*/
getjdf(q,e)
register struct q *q;	/* used by grargs to determine remoteness of queue */
register struct e *e;
{   
	FILE *efil;			/* entry file pointer */
	char * en;			/*error notification string*/
	int rc;


	rc = NEW;
	e->e_s = Qalloc(sizeof( struct stfile));
/* AIX security enhancement 	*/
/* removed struct con_groups	*/
	stclean(e->e_s,NULL);   /* initialize status file struct */

	/* open entry file for various info */
	if (en = jdfopen(e->e_name,&efil))
	{
		if (errno == ENOENT)
			return(NOTTHERE);
		else {
			badentry(e,efil,en);
			return(NOTOK);
		}
	}

/* 	NOTE -- WARNING -- the order of the following function calls	*/
/*	es importante.		*/
/* qdaemon is a GLOBAL CONTROL FLAG*/

	if (en =  grjobnum(e,efil,&rc))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}

	if (en = grreqinfo(e,efil,NULL))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}

	if (en =  grreqsize(e,efil))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}

/* AIX security enhancement 	*/
/* renamed gruid() to grlog()	*/
	if (en = grlog(e,efil))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}

/* AIX security enhancement 	*/
/* added grpcred()		*/
	if (en = grpcred(e,efil))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}
	
/* AIX security enhancement 	*/
/* added grpenv()		*/
	if (en = grpenv(e,efil))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}

/* AIX security enhancement 	*/
/* removed grcongrps()		*/

	if (en = grcmdline(e,efil))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}

	if (en =  grreqnam(e,efil))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}

	if (en =  grforuser(e,efil))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}

	if (en =  gr2ndhost(e,efil))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}
	if (en =  gropmsg(e,efil))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}

/* AIX security enhancement 	*/
/* removed grdir()		*/

	if (en = grargs(q,e,efil))
	{
		badentry(e,efil,en);
		return(NOTOK);
	}
	jdfclose(efil);          /* close entry file */

	/* finish off the arglist */
	e->e_jdfvec[e->e_jdfnum] = NULL;


#ifdef DEBUG
	if (getenv("GETJDF"))
	{
		system("cat *>/tmp/ac");
	 	prtfnames(e->e_fnames);
		prtargs(e);
	}
#endif


	return (OK);
}


	/* open the jdf file, read everything and close it.*/
char * jdfnewpri(ename,newpri)
char *ename;
int newpri;
{   
	FILE *efil;			/* entry file pointer */
	FILE *tfil;
	char * en;			/*error notification string*/
	char tname[MAXPATHLEN];
	int rc;
	gid_t	effgid, getgidx(int);

	/* open entry file for various info */
	if (en = jdfopen(ename,&efil))
		return(en);

	/* make temp file */
	gettmp(NULL,tname);
	if ((tfil = fopen(tname, "w")) == NULL)
		syserr((int)EXITFATAL,GETMESG(MSGNOPEN,"Error opening temporary file %s."),tname);

	/* get the effective gid to chown() the JDF file	*/
	if ((effgid=getgidx(ID_EFFECTIVE)) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGEGID,"Cannot get effective gid. Errno = %d"),errno);

	/* change the owner of JDF file from usr to a system user (root)
	 * and effective group (printq)
	 * this will protect the JDF file from tampering by a user.	
	 */
	if (chown(tname, (uid_t)OWNER_ADMDATA, effgid) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGCHWN,"Cannot chown %s. Errno = %d"),tname,errno);

	/* set the permissions on the JDF file	*/
	if (acl_set(tname, R_ACC|W_ACC, R_ACC|W_ACC, NO_ACC) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGPERM,"Cannot set permissions on %s. Errno = %d"),tname,errno);

	/* first record -- Job Number */
	while( (rc = fgetc(efil)) != EOF )
	{	
		fputc(rc,tfil);
		if( rc == '\n' ) break;
	}
	/* Second record  */
	/* X 0 1 15 1	dev# -cp -no priority mail_only? */
	/* dev #*/
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* number of copies */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* notification flag */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* skip old priority */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) ; /* NO WRITE */
	/* write priority */
	fprintf( tfil, "%d ", newpri );
	/* copy rest of file */
	while( (rc = fgetc(efil)) != EOF ) fputc(rc,tfil);
	jdfclose(efil);
	fclose(tfil);
	unlink(ename);
	link(tname,ename);
	unlink(tname);
}
	/* open the jdf file, read everything and close it.*/
char * jdfnewhol(ename,newhol)
char *ename;
int newhol;
{   
	FILE *efil;			/* entry file pointer */
	FILE *tfil;
	char * en;			/*error notification string*/
	char tname[MAXPATHLEN];
	int rc;
	gid_t	effgid, getgidx(int);

	/* open entry file for various info */
	if (en = jdfopen(ename,&efil))
		return(en);

	/* make temp file */
	gettmp(NULL,tname);
	if ((tfil = fopen(tname, "w")) == NULL)
		syserr((int)EXITFATAL,GETMESG(MSGNOPEN,"Error opening temporary file %s."),tname);

	/* get the effective gid to chown() the JDF file	*/
	if ((effgid=getgidx(ID_EFFECTIVE)) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGEGID,"Cannot get effective gid. Errno = %d"),errno);

	/* change the owner of JDF file from usr to a system user (root)
	 * and effective group (printq)
	 * this will protect the JDF file from tampering by a user.	
	 */
	if (chown(tname, (uid_t)OWNER_ADMDATA, effgid) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGCHWN,"Cannot chown %s. Errno = %d"),tname,errno);

	/* set the permissions on the JDF file	*/
	if (acl_set(tname, R_ACC|W_ACC, R_ACC|W_ACC, NO_ACC) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGPERM,"Cannot set permissions on %s. Errno = %d"),tname,errno);

	/* first record -- Job Number */
	while( (rc = fgetc(efil)) != EOF )
	{	
		fputc(rc,tfil);
		if( rc == '\n' ) break;
	}
	/* Second record  */
	/* X 0 1 15 1	dev# -cp -no priority mail_only? */
	/* dev #*/
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* number of copies */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* notification flag */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/*priority*/
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* mail only */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* skip old hold/release */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) ; /* NO WRITE */
	/* write priority */
	fprintf( tfil, "%d ", newhol );
	/* copy rest of file */
	while( (rc = fgetc(efil)) != EOF ) fputc(rc,tfil);
	jdfclose(efil);
	fclose(tfil);
	unlink(ename);
	link(tname,ename);
	unlink(tname);
}
	/* open the jdf file, read everything and close it.*/
char * jdfnewtime(ename,newdev,newtime)
char *ename;
char *newdev;
int newtime;
{   
	FILE *efil;			/* entry file pointer */
	FILE *tfil;
	char * en;			/*error notification string*/
	char tname[MAXPATHLEN];
	int rc;
	gid_t	effgid, getgidx(int);

	/* open entry file for various info */
	if (en = jdfopen(ename,&efil))
		return(en);

	/* make temp file */
	gettmp(NULL,tname);
	if ((tfil = fopen(tname, "w")) == NULL)
		syserr((int)EXITFATAL,GETMESG(MSGNOPEN,"Error opening temporary file %s."),tname);

	/* get the effective gid to chown() the JDF file	*/
	if ((effgid=getgidx(ID_EFFECTIVE)) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGEGID,"Cannot get effective gid. Errno = %d"),errno);

	/* change the owner of JDF file from usr to a system user (root)
	 * and effective group (printq)
	 * this will protect the JDF file from tampering by a user.	
	 */
	if (chown(tname, (uid_t)OWNER_ADMDATA, effgid) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGCHWN,"Cannot chown %s. Errno = %d"),tname,errno);

	/* set the permissions on the JDF file	*/
	if (acl_set(tname, R_ACC|W_ACC, R_ACC|W_ACC, NO_ACC) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGPERM,"Cannot set permissions on %s. Errno = %d"),tname,errno);


	/* first record -- Job Number */
	while( (rc = fgetc(efil)) != EOF )
	{	
		fputc(rc,tfil);
		if( rc == '\n' ) break;
	}
	/* Second record  */
	/* X 0 1 15 1	dev# -cp -no priority mail_only? */
	/* dev #*/
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) ; /* NO WRITE */
	/* write new device */
	fprintf( tfil, "%s ", newdev );
	/* number of copies */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* notification flag */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/*priority*/
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* mail only */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* hold/release */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) fputc(rc,tfil);
	/* space */
	fputc(' ',tfil);
	/* skip old time */
	while( ((rc = fgetc(efil)) != EOF) && !isspace(rc) ) ; /* NO WRITE */
	/* write newtime */
	fprintf( tfil, "%d ", newtime );
	/* copy rest of file */
	while( (rc = fgetc(efil)) != EOF ) fputc(rc,tfil);
	jdfclose(efil);
	fclose(tfil);
	unlink(ename);
	link(tname,ename);
	unlink(tname);
}
 
