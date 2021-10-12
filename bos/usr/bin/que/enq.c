static char sccsid[] = "@(#)25	1.78  src/bos/usr/bin/que/enq.c, cmdque, bos41B, 9504A 12/19/94 15:13:36";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: enq
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

/* 	Throughout the code the terms JOB DESCRIPTION, REQUEST, and ENTRY 
	are used interchangably.  They are synonymns. JDF means Job Description
	File.

	It occasionally needs to be determined if the queue we are currently
	dealing with is locol or remote.  The q_hostname field in the q struct
	provides us with the hostname of the machine where the queue resides.
	It is never blank, and has no indication of locality.  
	The function remote(q) returns a boolean value indicating whether or
	not a queue is local.  It does a string comparison to accomplish this.
*/
	
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/param.h>
#include <errno.h>
#include <signal.h>
#include <IN/standard.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/fullstat.h>
#include <IN/backend.h>
#include <locale.h>
#include <unistd.h>
#include "common.h"
#include "enq.h"
#include <security.h>
#include <sys/access.h>
#include <sys/audit.h>
#include <sys/id.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <usersec.h>
#include <grp.h>
#include <time.h>

#include <ctype.h>
#include <nl_types.h>
#include "enq_msg.h"
#define MAXSTR 		10
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_ENQ,num,str)

char * progname = "enq";
char * scopy();
char intignore;			/* ignore SIGINT? */
char hupignore;			/* ignore SIGHUP? */
uid_t invokers_uid;
uid_t programs_uid;
int lpdID;
char jdf[JDF_NAM_LEN];          /* to hold name of current jdf */
extern boolean palladium_inst;
extern boolean palladium_que;

extern char	nolabs ;	/* -nl no page labels */
extern char	*labelfile ; 	/* default label stanza file */
extern struct uflags *uflags;   /* linked list of user flag options */

struct uflags *userflags;	/* structure for processing user flags */
struct q *qlist;	/* master list of queues and devices */

/* structure for list of jobs */
struct rlist
{
	struct rlist *next;
	char jdfname[JDF_NAM_LEN];
	long sortval;
};

/* save some space here */
char qdpid[] = QDPID;
char statdir[] = STATDIR;

char *addfile();
char *title();
int palladium_proc();

extern int qsetup();
extern int errno;
extern char jobid[];

extern void resetQCBFile();

/* a note on current directories: qu -q is done from QUEDIR.  All else */
/* is normally done from original directory; any function that chdir's */
/* somewhere else has to chdir back. */


main(rargc,rargv)
int rargc;		/* real argument count */
register char **rargv;	/* real argument vector */
{
	int	argidx;
	char 	**copyargv;
	struct qe qentry;	/* queue entry info */
	int terminate(void);	/* clean up and die */
	boolean needtodoit;
	int argstogo = 0;
	static struct params par;
	struct params *p = &par;
	char *savedir;
	boolean chgrequest = FALSE;
	struct stat statb;
	int palrtn;

#ifdef DEBUG
        int j;

        if(getenv("ALLPARMS"))
        {
                sysraw("enq: ");
                for(j = 0; j < rargc; j++)
                        sysraw("[%s]",rargv[j]);
                sysraw("\n");
	}
#endif

	/* initialize invokers_uid and programs_uid for privilege toggling */
	invokers_uid = getuidx(ID_REAL);
	programs_uid = getuidx(ID_SAVED);

	/* suspend system call auditing */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	/* toggle to the privilege domain of the invoker */
	seteuid(invokers_uid);

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
        catd = catopen(MF_ENQ, NL_CAT_LOCALE);

	{
	    /* Get lpd's ID for to check priviledge for -Z flag */
	    /* and accept the remove flag ( remote jobs need to */
	    /* be removed					*/

	    int ret;

	    ret = getuserattr("lpd", S_ID, &lpdID, SEC_INT);
	    if ( ret == -1 )
	         perror("getuserattr lpd ");
	}

	/* check if the Palladium product is installed */
	if (!stat(PDENQ_PATH, &statb))
		palladium_inst = TRUE;

	copyargv=rargv;

	bzero((char *)p,sizeof(struct params)); 	/* nuke p struct */

	/* init qentry */
	qsetup(&qentry);

	resetQCBFile();
	needtodoit= needtodigest();

	/* make list of queues and devices from config file */
	qlist = readconfig(0);

	if (rdopts(rargc,copyargv,p,&argidx,&qentry,&chgrequest)== NOTOK) 
		return(1);

	if (needtodoit || p->p_dflg)
	{
		savedir = getcwd((char *)NULL,PATH_MAX);
		if( p->p_dflg && !checkPriv(TRUE) )
			qexit((int)EXITBAD);
		enq_qd_chg(ARGREAD);
		goose();
		if (p->p_dflg)
			qexit((int)EXITOK);
		else
			cd(savedir);
	}

	/* if get interrupt, be sure to delete all tmp files */

	if (signal(SIGHUP,SIG_IGN) != SIG_IGN)
		signal(SIGHUP,(void (*)(int))terminate);
	else
		hupignore++;

	if (signal(SIGINT,SIG_IGN) != SIG_IGN)
		signal(SIGINT,(void (*)(int))terminate);
	else
		intignore++;

	if (qentry.qe_queue == NULL && !(chgrequest))
		qentry.qe_queue = qlist;

	
/*
	There are several types of requests that we can discern at this point:
	1.	Status request.	 (enq -q or enq -A)
			Exec qstatus with the same args that you got.
				(except perhaps for -q.)
	2.	File request.  (enq filename)
			Generate job description file.  Goose qdaemon.
			Must have read privilege.
			enq_file_req()
	3.	Change file request (alter priority, cancel, move, or hold/rel)
			Generate job description file.  Goose qdaemon.
			Must be submitter or have privilege.
			enq_req_chg()
	4.	Change qdaemon request (enq -G or reread)
			Generate job description file.  Goose qdaemon.
			Check privilege.
			enq_qd_chg()
	5.	Change queue request (bring down/up/kill device)
			Generate job description file.  Goose qdaemon.
			Check privilege.
			enq_que_chg()

	   #2-#5 generate 4 different types of JDF's
	*/

	/*
	 * If we get a u flag without a -x or -X default to qstatus.
	 */
	if ( p->p_qflg || p->p_Aflg || p->p_Lflg || p->p_wflg || 
	     p->p_sflg || p->p_eflg || p->p_iflg ||
	     ( p->p_uflg && !(chgrequest) ) )
		{
			execvp(QSTATUS,rargv);
			syserr((int)EXITFATAL,MSGSTR(MSGEQST,"Cannot exec %s."),QSTATUS);
		}

	/*
	 * If queue selected is a Palladium logical printer, call code
	 * to do special Palladium processing.
	 */
	if (palladium_que) {
		palrtn = palladium_proc(p,rargc,copyargv,argidx);
		qexit(palrtn); 
	}

	/*
	* A queue has been selected.  Check on up/down status
	* of the queue.
	*/
	if( qentry.qe_queue && qdown(qentry.qe_queue) ) 
		qexit((int)EXITBAD);

#ifdef DEBUG
	if( getenv("ENQMAIN") )
	{
		sysraw("qentry.qe_queue: %s\n",qentry.qe_queue->q_name);
		if( qentry.qe_dev )
			sysraw("qentry.qe_dev: %s\n",qentry.qe_dev->d_name);
	}
#endif


	if (chgrequest) 
	{
		enq_req_chg(p,&qentry);
		/* audit the request */
		if (p->p_Xflg)
			admaudit(&qentry,AUD_CANCEL);
		goose();
		qexit((int)EXITOK);
	}

	if (p->p_Dflg || p->p_Kflg || p->p_Uflg) 
	{
		if( !checkPriv(TRUE) )
			qexit((int)EXITBAD);
		enq_que_chg(p,&qentry);
		/* audit the request */
		if (p->p_Dflg)
			admaudit(&qentry,AUD_DEVDOWN);
		else if (p->p_Uflg)
			admaudit(&qentry,AUD_DEVUP);
		else
			admaudit(&qentry,AUD_DEVDOWNKILL);
		
		request(jdf,SPECIAL);	
		goose();
		qexit((int)EXITOK);
	}

	if (p->p_Gflg ) 
	{
		if( !checkPriv(TRUE) )
			qexit((int)EXITBAD);
		enq_qd_chg(ARGDIE);
		/* audit the request */
		admaudit(&qentry,AUD_KILLQDAEMON,AUDIT_OK);
		goose();
		qexit((int)EXITOK);
	}

	if (enq_file_req(p,rargc,copyargv,&qentry,argidx))
	{
		request(jdf,NORMAL);
		goose();
		if (p->p_jflg)
			printf(MSGSTR(MSGJOBNO,"Job number is: %d\n"),JOBNUM(atol(jobid)));
		qexit((int)EXITOK);
	}
	else
		qexit((int)EXITBAD);
}

/* Check for zero length files */
int chk_zero_len_file(fname)
char *fname;
{
	struct stat statb;

	if (stat(fname,&statb) == -1)
	{
		perror(fname);
		return(TRUE);
	}
	if (statb.st_size == 0)
	{
		return(TRUE);
	}
	return (FALSE);
}


/* 	Do Request for UnFriendly backend part 2.
	This routine assumes than the input filename exists relative the the 
	current directory.
	The purpose of this routine is to do most of the work necessary to
	create the data for a job description file. PER FILE.
*/
char * enq_file_1(fn,qf_cp,qe)
char qf_cp;
register struct qe *qe;
char * fn;	/*filename*/
{
	struct stat statb;
	char * nam;	/*filename*/

	if (fn)		/* if not stdin (a real file given)*/
	{
		/* be sure have permission to print and delete it */
		/* turns off DR if no rm permission        */
		/* returns whether we can read it */
		/* prints error messages */
		if (chkacc(fn, qe, &statb) == FALSE)
			return(FALSE);
		
		/* If it is a zero length file, do not print */	
		if (chk_zero_len_file(fn)) {
			if ( qe->qe_rm )
				unlink(fn);
			return(FALSE);
		}
	}

	/* copies if fn=null or qf_cp.  Adds to arglist*/
	nam=addfile(fn,qf_cp,qe); 	

	if (!fn) 	/* if stdin (a real file not given)*/
	{
		if (chkacc(nam, qe, &statb) == FALSE)
			return(FALSE);

		/* If it is a zero length file, do not print */	
		if (chk_zero_len_file(nam)) {
			unlink(nam);
			return(FALSE);
		}
	}

	/* plug in the size in blocks (I prefer bytes myself...) */
	/* Round the result up to the next block. */
	qe->qe_numblks += ((statb.st_size + (SYSBLKSIZ - 1)) / SYSBLKSIZ);

	qe->qe_date = time(0);
	if (!qe->qe_reqname)
	{
		if (!fn)
			qe->qe_reqname = title();
		else
			qe->qe_reqname = scopy(fn);
	}
	return(nam);

}

/* DO REQuest for UNFRIENDly backend.*/
/* queue an entry for the file whose name is */
/* qe has all the default info in it.           */
/* return TRUE if file queued, FALSE otherwise.         */
enq_file_req(p,argc,argv,qe,argidx)
register char **argv;
register struct params *p;
register struct qe *qe;
int	argidx,argc;
{
	int i=0, size=0, argidx_save;
	boolean isstdin=FALSE;
	char *outnam=NULL;
	char *tmpoutnam=NULL;
	char *cmd_line=NULL;

	argidx_save = argidx;
	if (argc==argidx)	/* no file arguments */
		outnam=enq_file_1(NULL,p->p_cflg,qe); 	/* read stdin */
	else
	{	
	for( ; argidx < argc; argidx++ )
		switch (argtype(argv[argidx]))	
		{
			case ARGFILE: 
				tmpoutnam=enq_file_1(argv[argidx],p->p_cflg,qe);
				if(tmpoutnam)
					outnam=tmpoutnam;
				continue;
			case ARGSTDIN: 
				/* copies if str=null,NFS, or copyflg  adds to arglist*/
				outnam=enq_file_1(NULL,p->p_cflg,qe); 	
				isstdin=TRUE;
				continue;
			default:
				syserr((int)EXITFATAL,MSGSTR(MSGEEFR,"Bad stdin file request."));
				break;
		}
	}

	if (outnam)
	{
		/* allocate memory for command line string to put in jdf */
		for (i=0; *argv[i]; i++)
			size += strlen(argv[i]);
		size++;				/* add the terminating NULL */
		if ( (cmd_line = malloc((size_t)size)) == NULL)
			syserr((int)EXITFATAL,MSGSTR(MSGNOMEM,"Memory allocation failure."));
		bzero(cmd_line, (size_t)size);
		convert_cmdline(cmd_line, argv, argidx_save);

		outjdf(p,qe,outnam,cmd_line); 	/* now queue that sucker */
		return(TRUE);
	}
	else
		return(FALSE);
}



/* If str is null read stdin */
char *copyfile(str,qe)
char *str;
register struct qe *qe;
{
	static char outnam[MAXPATHLEN];

	register FILE *inp, *outp;
	register c;
	register char *tmpcopy;
	gid_t	effgid, getgidx(int);
	uid_t	realuid, getuidx(int);

	if( str == NULL && qe == NULL )
		return(0);

	if (str == NULL)
		inp = stdin;
	else
	{
		if ((inp = fopen(str,"r")) == NULL)
			/* can't happen */
			syserr((int)EXITBAD,MSGSTR(MSGNOPN,"Unable to open %s."),str);
	}

	tmpcopy = TMPCOPY;

	/*
	* Attempt to create the spool file.
	* Note: gettmp now creates and closes the file.
	* It also does automatic retry if the name
	* it generates is not unique.
	*/

	

	if( gettmp(tmpcopy,outnam) < 0 )
		badtmp(TMPCOPY);

	/* toggle to the privilege domain of the program */
	seteuid(programs_uid);

	/* get the real uid to chown() the temp file	*/
	if ((realuid=getuidx(ID_REAL)) == -1)
		syserr((int)EXITBAD,MSGSTR(MSGRUID,"Cannot get real uid."));

	/* get the effective gid to chown() the temp file	*/
	if ((effgid=getgidx(ID_EFFECTIVE)) == -1)
		syserr((int)EXITBAD,MSGSTR(MSGEGID,"Cannot get effective gid."));
		
	/* set ownership to caller's uid and caller's gid	*/
	if (chown(outnam, realuid, effgid) == -1)
		syserr((int)EXITBAD,MSGSTR(MSGCHWN,"Cannot chown %s. Errno = %d"),outnam,errno);

	/* set the permissions on the temp file	*/
	if (acl_set(outnam, W_ACC | R_ACC, R_ACC, NO_ACC) == -1)
		syserr((int)EXITBAD,MSGSTR(MSGPERM,"Cannot set permissions on %s. Errno = %d"),outnam, errno);

	/* toggle to the privilege domain of the invoker */
	seteuid(invokers_uid);


	if ((outp = fopen(outnam, "w")) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGROPN,"Cannot re-open temp file %s."), outnam);

	while ((c = getc(inp)) != EOF)
		if (EOF==putc(c,outp)) {
			fclose(outp);
			unlink(outnam);
			syserr((int)EXITFATAL,MSGSTR(MSGULMT,
				"Cannot write any more to %s, file has gotten too large."),
				outnam);
		}

	if (str != NULL)	/* real input file */
		fclose(inp);
	if ( fclose(outp) == EOF ) {
	    unlink(outnam);
	    syserr((int)EXITFATAL,MSGSTR(MSGNCLS,"Unable to close file : %s."),outnam);
	}
	return(outnam);
}


/* check access to file named nam.  DR flag tells whether we */
/* will want to remove it.  return TRUE if we can read it, FALSE */
/* otherwise.  turn off DR if no remove permission. */
chkacc(nam, qe, statb)
char *nam;
struct qe *qe;
struct stat 	*statb;
{
	register char  *cp, *cp2, c;
	char *dnam;

	if (stat(nam,statb) == -1)
	{
		syswarn(MSGSTR(MSGEPRT,"Failure accessing %s."), nam);
		return(FALSE);
	}

	/* Fix for A15871 */
	if ((statb->st_mode & S_IFMT) == S_IFDIR)
	{
		syswarn(MSGSTR(MSGDIR,"Cannot print directory: %s."),nam);
		return(FALSE);
	}
	/* End of Fix for A15971 */
	/* can we read it?  must have read access to the file. */
	if (accessx(nam,R_ACC,ACC_INVOKER) == -1)
	{   
		syswarn(MSGSTR(MSGERDP,"%s has no read permission."),nam);
		return(FALSE);
	}

	/* can we delete it? must have write access to directory. */
	if (qe->qe_rm)
	{   for (cp = cp2 = nam; c = *cp++;)	/* find dir name */
	    {   if (c == '/')
		    cp2 = cp;
	    }
	    c = *cp2;
	    *cp2 = 0;
	    if (cp2 == nam)
		dnam = ".";
	    else
		dnam = nam;
	    if (accessx(dnam,W_ACC,ACC_INVOKER) == -1)
	    {
		*cp2 = c;

		/* If this is lpd accept the remove flag; otherwise, turn it off */
		if ( invokers_uid != lpdID ) {
		     syswarn(MSGSTR(MSGERMV,"%s has no remove permission."),nam);
		     systell(MSGSTR(MSGPRAN," (Printing it anyway)"));
		     qe->qe_rm= FALSE;	/* turn -rm off */
		     return(TRUE);
		}
	    }
	    else
		*cp2 = c;
	}


	/* even if we still have remove permission, don't */
	/* remove anything but a plain file.           */
	if ((qe->qe_rm) && (statb->st_mode&S_IFMT)!=S_IFREG)
	{
		syswarn(MSGSTR(MSGENRF,"%s: Cannot remove a nonregular file."),nam);
		systell(MSGSTR(MSGPRAN," (Printing it anyway)"));
		qe->qe_rm= FALSE;	/* turn DR off */
	}
	return(TRUE);
}


/* see if this file name could be mine -- i.e., my name is in it, */
/* and it is on my queue if i've specified one. */
boolean ismine(ename,qe,multimatch,name2look4,priv)
register char *ename;
register struct qe *qe;
boolean multimatch;	/* if name2look4 is null, then all users match */
register char *name2look4;
boolean priv;		/* is this user privileged to see other users jobs?*/
{
	register char *myname;
	register char *lognm;
	char *tmpptr;
	char *tmpaddr;
	boolean anybodyok;	
	char host_JDF[5];	/* address of hostname - 4 bytes */
	char user_JDF[10];	/* user name length is 8 */
	char host_user[5];	/* address of hostname - 4 bytes */
	char user_user[10];	/* user name length is 8 */
	struct hostent *host = NULL;

	host_JDF[0]='\0'; host_user[0]='\0';
	user_JDF[0]='\0'; user_user[0]='\0';

	if (!priv && name2look4)
		if (strcmp(name2look4,qe->qe_logname))
			return (FALSE);

	myname =  (priv && name2look4) ? name2look4 : qe->qe_logname;

	anybodyok = multimatch && (name2look4 == NULL || name2look4 == qe->qe_logname) && priv;

#ifdef DEBUG
	if (getenv("ISMINE"))
		sysraw("ename %s,logname %s,multimatch %d,name2look4 %s,priv %d\n",
		        ename,qe->qe_logname,multimatch,name2look4,priv);
#endif

	lognm = getln(ename);

	/* get information on remote host from JDF file ( if any ) */
	if (( tmpptr = (char *)strchr (lognm, '@')) != NULL ) {
		*tmpptr = '\0';
		strcpy(user_JDF, lognm);
		*tmpptr = '@';
		tmpptr++;
		if (!(isinet_addr(tmpptr)))
			host = gethostbyname(tmpptr);
		else {
			tmpaddr = (char *)inet_addr(tmpptr);
			host = gethostbyaddr(&tmpaddr,sizeof(tmpaddr),AF_INET);
		}
		if ( host != NULL ) {
		    memcpy(host_JDF, *host->h_addr_list,4);
		    host_JDF[4] = '\0';
		}
	}

	/* get information on remote host from supplied info ( if any ) */
	if (( tmpptr = (char *)strchr (myname, '@')) != NULL ) {
		*tmpptr = '\0';
		strcpy(user_user, myname);
		*tmpptr = '@';
		tmpptr++;
		if (!(isinet_addr(tmpptr)))
			host = gethostbyname(tmpptr);
		else {
			tmpaddr = (char *)inet_addr(tmpptr);
			host = gethostbyaddr(&tmpaddr,sizeof(tmpaddr),AF_INET);
		}
		if ( host != NULL ) {
		    memcpy(host_user, *host->h_addr_list,4);
		    host_user[4] = '\0';
		}
	}

	/* name match? or we want anyname */
	if (!anybodyok && strcmp(myname,lognm)) {
	    /*
	     * If we failed the first check, then we check the host
	     * and user names ( if any ) that we previously obtained
	     */
	    if ( ! ( ( host_JDF[0] !=  '\0' ) && ( host_user[0] != '\0') &&
		     (memcmp(host_JDF, host_user, 4) == 0) &&
		     (strcmp(user_JDF, user_user) == 0) ) ) {

			if (!priv)
				return(FALSE);
			else if (name2look4)
				return(FALSE);
	    }
	}

	if( qe->qe_queue && strncmp(qe->qe_queue->q_name, getqn(ename), QNAME) )
		return(FALSE);

	return(TRUE);
}

/* send a special message to the qdaemon about the job str. */
/* or, in the case of -X, about a group of jobs */
/* return success -- TRUE or FALSE */
enq_req_chg(p,qe)
register struct params *p;
register struct qe *qe;
{
       	DIR             *dirp;
       	struct dirent   *dire;

	int jobnumfromfile;
	register int answer = FALSE;
	boolean priv;
	char que_name[QNAME];
	register int remstat = 0;
	long sortnum;
	int group = 0;
	struct rlist *slist = NULL;
	register struct rlist *s;

	priv = checkPriv(FALSE);

	errno=0;

	/* read queue directory, file by file.  if device matches */
	/* or none given, and if logname matches, and if jobnum */
	/* matches, make a special entry for this name. */

	if (!(p->p_xflg || p->p_numflg))
		group++;

	cd(QUEDIR);

       	if ((dirp=opendir(".")) == NULL)
		/* directory open failed */
		syserr((int)EXITFATAL,MSGSTR(MSGNOQD,"Cannot open qdir %s."),QUEDIR);

        while ((dire = readdir(dirp)) != NULL)
	{
		register char *name = dire->d_name;
		long timbef, modtime();
		char line[QELINE];

		/* must be a real queue entry */
		/* not a special request or a tmp file */
		if (name[0] == 'r' || name[0] == 't')
			continue;

                if (name[0] == '.' && (name[1] == '\0' || name[1] == '.'))
                        continue;

		if (strcmp(name,"core") == 0 || strcmp(name,".core") == 0)
			continue;

		/* keep time before and after to be sure file */
		/* stays same while we're checking it out. */
		if ((timbef = modtime(name)) < 0)
			continue;

		/* name must be right (as much as is visible) */
		if (!ismine(	name,
				qe, 
				group ,
				p->p_urem,
				priv  ) )
			continue;

		/* job number and device number must be right */
		if (!(sortnum=ismatch(name,qe,group)))
			continue;

		jobnumfromfile = JOBNUM(sortnum);

                /*
                * Check on up/down status of the queue if not previously
                * checked.  This occurs if an enq -x was issued and no
		* queue was specified.  In this case, qe->qe_queue is null.
		* This is because cancel is by job number, which is unique
		* across all local queues.  A queue must be specified to
		* cancel a remote job.
                */
                strcpy (que_name, getqn(name));
                if( !qe->qe_queue && qdown(get_queue(que_name,qlist)) )
                        qexit((int)EXITBAD);

		/* file must be same */
		if (timbef != modtime(name))
			continue;

		/* got it! */
		if (p->p_xflg || p->p_Xflg)	/* cancelling?*/
			sprintf(line,"x%s\n%03d\n",name,jobnumfromfile);
		else if (p->p_hflg)		/* holding */
			sprintf(line,"h%s\n%03d\n",name,jobnumfromfile);
		else if (p->p_pflg)		/* releasing */
			sprintf(line,"R%s\n%03d\n",name,jobnumfromfile);
		else if (p->p_Qflg) 		/* moving */
			sprintf(line,"m%s\n%03d\n%s\n%s\n",
			name,jobnumfromfile,qe->qe_movqueue,
			qe->qe_movdev);
		else		  	 	/* change priority */
		{
			sprintf(line,"p%s\n%d\n%s\n",
			name,qe->qe_priority,p->p_numrem);
		}

		if (qe->qe_queue)
			strcpy (que_name,qe->qe_queue->q_name);
		outsr(line,que_name,qe);

                /* At this point we have the JDF in qdir */
		if (group)
		{
			s = Qalloc(sizeof(struct rlist));
			strcpy(s->jdfname,jdf);
			s->sortval = sortnum;
			submitlist(&slist,s);
		} 
		else
		{
			request(jdf,SPECIAL);
			answer = TRUE;
			break;
		}
	}

	/* if a group of jobs, submit all of them now */
	if (group && slist)
	{
		for(slist; slist; slist=slist->next)
			request(slist->jdfname,SPECIAL);
		answer = TRUE;
	}

	if (qe->qe_queue && remote(qe->qe_queue->q_hostname))
	{
		if (p->p_Xflg) {
			errno = 0;
			syswarn(MSGSTR(MSGREMCAN,"Cancel all not supported on remote queues."));
			qexit((int)EXITOK);
		}
		if (p->p_aflg || p->p_hflg || p->p_pflg || p->p_Qflg){
			if(!answer){
				errno = 0;
				syswarn(MSGSTR(MSGNOAC,"Requested action not supported for the remote side of a queue."));
			}
			qexit((int)EXITOK);
		}
		rem_cancel(p,qe);
		answer = TRUE;
	}

	if (!answer)
		if (qe->qe_queue)
			syswarn(MSGSTR(MSGNORQ,"No such request in queue %s -- perhaps it's done?"),p->p_Prem);
		else
			syswarn(MSGSTR(MSGNORLQ,"No such request in any local queue -- perhaps it's done?"));
	else
		closedir(dirp);

	return(answer);
}

/* this routine is used to put special request in the order the jobs
   were queued */
submitlist(slist,s)
register struct rlist **slist;
struct rlist *s;
{
	register struct rlist *p1, *p2;

	p1 = *slist;
	if((*slist == NULL) || (s->sortval < p1->sortval)){
		s->next = *slist;
		*slist = s;
		return;
	}
	for (; (p2=p1->next) != NULL; p1 = p2){
		if (s->sortval < p2->sortval) {
			p1->next = s;
			s->next = p2;
			return;
		}
	}
	p1->next = s;
	return;
}

/*----Invoke the remote backend to cancel job(s) */
boolean rem_cancel(p,qe)
register struct params *p;
register struct qe *qe;
{
	char *argvec[MAXARGS];
	char jobnum[4];
	int pid,status;
	int i;
	char usr_nam[USERNAMELEN + 1];

	/*----Transform the job number */
	sprintf(jobnum,"%d",qe->qe_jobnum);

	/*----Construct the arguments for rembak */
	i = 0;
	argvec[i++] = scopy(REM_BACKEND);
	argvec[i++] = scopy("-S");
	argvec[i++] = scopy(qe->qe_queue->q_hostname);
	argvec[i++] = scopy("-P");
	argvec[i++] = scopy(qe->qe_queue->q_rname);
	argvec[i++] = scopy("-x");
	argvec[i++] = scopy("-#");
	if (p->p_Xflg)
		argvec[i++] = scopy(REM_ALLNUM_VALUE);
	else
		argvec[i++] = scopy(jobnum);
	argvec[i++] = scopy("-u");
	if (!p->p_urem)
		strcpy (usr_nam, qe->qe_logname);
	else strcpy (usr_nam, p->p_urem);
	/* D42772 - removed since this was not necessary for lpd
	** strcat (usr_nam,"@");
	** strcat(usr_nam, localhost());
	*/
	argvec[i++] = scopy(usr_nam);
	argvec[i] = NULL;
#ifdef DEBUG
	if(getenv("REMCAN"))
	{
		sysraw("rem_cancel: ");
		for(i = 0; argvec[i] != NULL; i++)
			sysraw("[%s]",argvec[i]);
		sysraw("\n");
	}
#endif

	/*----Fork and exec rembak */
	pid = fork();
	switch(pid)
	{
	case -1:	/* error */
		syserr((int)EXITFATAL,MSGSTR(MSGEFRR,"Cannot fork to run backend.  Errno = %d."),errno);

	case  0:	/* child */
		execv(argvec[0],&argvec[0]);
	
	default:	/* parent */
		return(0);
	}
}

/* is user privileged (to make special requests?) */
checkPriv(warn)
boolean warn;
{
	struct group *grp;
	int ngroups, gidset[NGROUPS];
	register int i;
	boolean match = FALSE;
	/* determine if user is root or a member of printq group */

	/* get printq group gid */
	grp = getgrnam("printq");

	/* toggle to the privilege domain of the invoker */
	seteuid(invokers_uid);

	if(invokers_uid == 0)
	{
		return(TRUE);
	}
	else 
	{
		/* get a list of the user groups and see if printq is in it */
		ngroups = getgroups(NGROUPS,gidset);
		if (ngroups < 0)
			syserr((int)EXITFATAL,MSGSTR(MSGGGRP,"Cannot get list of groups."));
		for (i=0; i<ngroups; i++)
		{
			if ((grp != NULL) && (grp->gr_gid == gidset[i]))
			{
				return(TRUE);
			}
		}
	}

	if (warn) 
		syswarn(MSGSTR(MSGROOT,"You must belong to the printq group."));
	return(FALSE);
}


/* put out a special request; str is the line to go in the file. */
outsr(str1,str2,qe)
register char *str1,*str2;
register struct qe *qe;
{
	char tnam[MAXPATHLEN];
	register FILE *fil;
	char reqname[15];
	gid_t	effgid,getgidx(int);

	cd(QUEDIR);

	if( gettmp(NULL,tnam) < 0 )
		badtmp(QUEDIR);

	
	/* toggle to the privilege domain of the program */
	seteuid(programs_uid);

	/* get the effective gid to chown() the JDF file	*/
	if ((effgid=getgidx(ID_EFFECTIVE)) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGGIDX,"Cannot get effective gid. Errno = %d"),errno);

	/* change the owner of JDF file from usr to a system user (root)
	 * and effective group (printq)
	 * this will protect the JDF file from tampering by a user.
	 */
	if (chown(tnam, OWNER_ADMDATA, effgid) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGCHWN,"Cannot chown %s. Errno = %d"),tnam,errno);

	/* set the permissions on the JDF file	*/
	if (acl_set(tnam, R_ACC|W_ACC, R_ACC|W_ACC, NO_ACC) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGPERM,"Cannot set permissions on %s. Errno = %d"),tnam,errno);

	/* toggle to the privilege domain of the invoker */
	seteuid(invokers_uid);


	if ((fil = fopen(tnam,"w")) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGOOPN,"Unable to open %s."),tnam);
	fputs(str1,fil);
	fputs(str2,fil);
	if ( fclose(fil) == EOF ) {
	    unlink(tnam);
	    syserr((int)EXITFATAL,MSGSTR(MSGNCLS,"Unable to close file : %s."), tnam);
	}
	sprintf(reqname,"raa%.6s",qe->qe_logname);
	qlink(tnam,reqname);
	strncpy(jdf,reqname,JDF_NAM_LEN-1);
}


/* 
   ENQ a QUeue CHanGe request. Queue Change requests affect the queue in 
   some manner.
*/
enq_que_chg(p,qe)
register struct params *p;
register struct qe *qe;
{
	char code;
	char line1[QELINE];
	char line2[QELINE];

	if (p->p_Uflg)
		code = 'u';
	else
	if (p->p_Dflg)
		code = 'd';
	else
		code = 'k';

#ifdef DEBUG
	if(getenv("ENQQCHG"))
		dumpdl(qe->qe_queue->q_devlist);
#endif	

	if (qe->qe_queue == NULL || (qe->qe_dev == NULL &&
				     qe->qe_queue->q_devcount > 1))
	{
		syswarn(MSGSTR(MSGNDEV,"No device specified."));
		return(0);
	}

	sprintf(line1,"%c:%s\n",
		code, 
		qe->qe_dev == NULL? qe->qe_queue->q_devlist->d_name:
				    qe->qe_dev->d_name);

	sprintf(line2,"%s\n",qe->qe_queue->q_name);

	outsr(line1,line2,qe);
}

 
qddead()
{
        syswarn(MSGSTR(MSGQDED,"Qdaemon appears to be dead."));
}
 
 
/*
* Do an extra special request, namely REREAD or DIE GRACEFULLY.
*
* These requests are termed "extra special" because they are not associated
* with any queue, unlike other print requests.  
*/

enq_qd_chg(req)
int req;
{
	char *qdp, line[MAXLINE];
	struct qe qentry;
	register struct qe *qe = &qentry;
	struct fullstat statb;

	qe->qe_logname = getlgnam();

	if( req == ARGREAD )
	{
		redigest();		/* redigest local qconfig */
		sprintf(line,"r\n%s\n",qe->qe_logname);
	}
	else	/* ARGDIE */
		sprintf(line,"s\n%s\n",qe->qe_logname);

	qdp = qdpid;

	/* don't leave message if qdaemon not alive */
	if (fullstat(qdp, FL_STAT, &statb) == -1)
		qddead();
	else
	{
		/* set up phony qe structure to pass outsr */
		outsr(line,"all queues",qe);  /* this request has nothing todo
						 with specific queues*/
		request(jdf,SPECIAL);
		goose();
	}

	return(0);
}

char *
title()
{
	static char ttl[20];

	sprintf(ttl,"STDIN.%d", getpid());
	return(ttl);
}

/*
* qdown - check on the up/down status of a queue.  Return TRUE if
* the local queue is down or FALSE if its up.  The up/down status
* of remote queues can't be checked.
*/

qdown(q)
struct q *q;
{
	if( q->q_up == FALSE )
	{
		syswarn(MSGSTR(MSGLDED,
			"Local queue %s is dead -- no requests being accepted."),
			q->q_name);

		return(1);
	}
	return(0);
}

/* audit the privileged requests */
int
admaudit(qent,str)
struct	qe	*qent;
char		*str;
{
	/* toggle to the privilege domain of the program */
	seteuid(programs_uid);

	auditwrite("ENQUE_admin",  AUDIT_OK, 
			qent->qe_queue->q_name,strlen(qent->qe_queue->q_name)+1,
			qent->qe_dev->d_name,strlen(qent->qe_dev->d_name)+1,
			qent->qe_reqname, strlen(qent->qe_reqname)+1,
			qent->qe_to, strlen(qent->qe_to)+1, 
			str, strlen(str)+1, NULL);

	/* toggle to the privilege domain of the invoker */
	seteuid(invokers_uid);

}

/* convert the command line from argv into string format */
/* leave off the following:
	command name (argv[0])
	file names (start at argv[index])
	-o options to backend (backend already gets those at exec time)
*/
convert_cmdline(enq_cmd,argv,index)
char *enq_cmd;
char **argv;
int index;
{
	int i;

	i=1;				/* leave off command name */
	while ( i < index )		/* stop at file names */
	{
		if ( strncmp(argv[i], "-o", 2) )
		{
			if ( i == 1 )
				strcpy (enq_cmd, argv[i++]);
			else
				strcat (enq_cmd, argv[i++]);
		}
		else {			/* leave off -o */
			if ( strlen(argv[i]) > 2 )	/* no space between -o and backend option */
				i++;
			else i+=2;	/* backend option separated by space */
		}
	}
}

/*
 * Routine to do Palladium processing. This code will handle the canceling
 * and submitting jobs to Palladium spooler.  The pdenq() will be forked and
 * execed to do the job.
 */
int palladium_proc(struct params *p, int argc, char **argv, int argidx)
{
	int pid;			/* Process ID of child		*/
	int status = 0;			/* Exit status of child		*/
	int retcode = 0;		/* Return code of waitpid()	*/
	int palfn;					/* fine name cnt */
	char *stdfn;				/* stdin file name */

	/* 
	 * If attempt is made to alter, hold. release, or move a job,
	 * report that this is not supported for Palladium.
	 */
	if (p->p_aflg || p->p_hflg || p->p_pflg || p->p_Qflg || p->p_numflg)
		syserr((int)EXITFATAL,MSGSTR(MSGPAL1,
		     "Cannot alter, hold, release, or move another spooler's job."));

	/* 
	 * If an attempt is made to bring a queue up or down, report that
	 * this is not supported for Palladium.
	 */
	if (p->p_Dflg || p->p_Gflg || p->p_Kflg || p->p_Uflg)
		syserr((int)EXITFATAL,MSGSTR(MSGPAL2,
		     "Cannot alter state of another spooler's queues."));

	/* 
	 * If attempt is made to cancel all jobs on a Palladium queue,
	 * report that this is not allowed.
	 */
	if (p->p_Xflg)
		syserr((int)EXITFATAL,MSGSTR(MSGPAL3,
		     "Cancel all not supported on another spooler's queues."));

	if (p->p_xflg)
		palarg[1] = PAL_CANCEL;	/*this is cancel request*/
	else if (!p->p_Yflg)
	/* now handle the files for palladium */
	{
		if (argc == argidx)
		{
			stdfn = copyfile(NULL,NULL);	/* give stdin a filename */
			palarg[palcnt++] = scopy(stdfn);
		}
		else
		{
			for (palfn = argidx; palfn < argc; palfn++)
			{
				switch(argtype(argv[palfn]))
				{
					case ARGFILE:
						palarg[palcnt++] = scopy(argv[palfn]);
						break;
					case ARGSTDIN:
						stdfn = copyfile(NULL,NULL);
						palarg[palcnt++] = scopy(stdfn);
						break;
					default:
						syserr((int)EXITFATAL, MSGSTR(MSGEEFR,
							"Bad stdin file request."));
						break;
				}
			}
	 	}
	}

	palarg[palcnt] = NULL;

	/* Let's submit the job */
	/* Fork a child to exec the Palladium pdenq command */
	switch (pid = fork())
	{
		/* problem then kick off an error */
		case -1:
			syserr((int)EXITFATAL,MSGSTR(MSGFORK,"Unable to fork a new process"));

		/* Child -- Execute the pdenq command */

		case 0:
			execvp(PDENQ_PATH,palarg);
			exit((int)EXITBAD);

		/* Parent waiting on the child to finish and check exit status */

		default:
			while ((retcode = waitpid(pid, &status, 0)) == -1)
				if (errno != EINTR)
					break;
			if ((retcode == -1) || status)
				syserr((int)EXITFATAL,MSGSTR(MSGPAL5,
					"Request failed."));
			return(WEXITSTATUS(status));
	}
}

