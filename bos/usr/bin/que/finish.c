static char sccsid[] = "@(#)29	1.35.1.1  src/bos/usr/bin/que/finish.c, cmdque, bos411, 9428A410j 2/4/94 17:56:09";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
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

#include <stdio.h>
#include <errno.h>
#include <usersec.h>  /* Defect 28341 */
#include <IN/standard.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <IN/backend.h>
#include "common.h"
/*#include "notify.h"       P15245 S.H.*/
#include <ctype.h>

#include "qmain_msg.h"
#define MSGSTR(num,str)	catgets(catd,MS_QMAIN,num,str)
nl_catd	catd;

#ifdef MAXRETRIES
#undef MAXRETRIES
#endif
#define	MAXRETRIES	2


/*
 * queueing daemon --
 *      routines to clean up when a job finishes
 */

extern int nchildren, request, errno;
int childterm;		/* to save exit status of child for file rm */


/************************************************************************/
/* rmlist                                                          	*/
/* This functions runs through a list of filenames and    		*/
/* unlinks them.  But only if the udel flag (per file) is set.		*/
/************************************************************************/

void rmlist(elist)
register struct e *elist;
{
	register struct str_list *fl, *prev_p ;
	register struct str_list *head = elist->e_fnames;  /* head of the list of filenames */
	int status;
	char buf[BUFSIZ], message[BUFSIZ]; /* temporary buffers for messages */
	int remote = 0;
	char **credit;

	/* see if this is remotly queued by checking if the REAL_USER 
	   credential is equal to lpd */

	for (credit = elist->e_pcred; *credit; credit++)
	{
		if (strcmp(*credit,"REAL_USER=lpd") == 0)
		{
			remote = 1;
			break;
		}
	}

	/* do not delete the file if the -r flag was used, this entry is
	   local to this machine and this process is deleted prematurely */

	if (!remote && ( childterm == SIGTERM ) && ( head->s_del == RM_FLAG ))
		return;

	for (prev_p=NULL,fl=head; fl; prev_p=fl,fl=fl->s_next)
	{
		/* unlink file */
		if ((fl->s_name) && (fl->s_del))
		{
                    if (unlink(fl->s_name) < 0) {
                        /* root unable to unlink file, possibly a remote
			   mounted file */
			uid_t	uid;
			int 	pid;
			if (getuserattr(elist->e_from,S_ID,&uid,SEC_INT) < 0) {
			    syswarn((int)EXITWARN,MSGSTR(MSGRUID,"Cannot get real uid."),errno,strerror(errno));
			    return;
			}
			pid = fork();
			switch (pid) {
				case -1:
					/*----ERROR: send message */
					sprintf(message,MSGSTR(MSGERMV,
					   "Unable to remove file: %s."),
					   fl->s_name);
					sprintf(buf,"%s: errno = %d: %s\n",
					   message,errno,strerror(errno));
					syswarn((int)EXITWARN,message);
					break;
				case 0: /* CHILD */
					/* This should never fail */
					setuid(uid);
					if (unlink(fl->s_name) < 0){
					    sprintf(message,MSGSTR(MSGERMV,
					       "Unable to remove file: %s."),
					       fl->s_name);
					    sprintf(buf,"%s: errno = %d: %s\n",message,errno,strerror(errno));
					    syswarn((int)EXITWARN,message);
					}
					exit(0);
					break;
				default:   /* PARENT */
					if (waitpid(pid,&status,WNOHANG|WUNTRACED) <= 0) {
					/* check status */
					    if (WEXITSTATUS(status) != 0) {
						/* error */
					        sprintf(message,MSGSTR(MSGERMV,
					          "Unable to remove file: %s."),
					          fl->s_name);
					        sprintf(buf,"%s: errno = %d: %s\n",
						 message,errno,strerror(errno));
					        syswarn((int)EXITWARN,message);
					    }	
					}	
					break;
			}					
                    }                
		}
	}
}

/*
 * job running on dev has just died.  things to do:
 *      0 -- delete the pid file/
 *      1 -- set device status depending on return code.
 *      2 -- notify user(s), depending on return code.
 *      3 -- charge for use of device
 *      4 -- free allocated fields in e struct -- to,from,cdir, request,
 *           and hostname. Don't free filename(s) yet if delete
 *           is FALSE, because we may need them in fclean.
 * return whether entry e is to be deleted.
 */
devfinish(dev,status)
struct d	*dev; 
int		status;
{
        int             exitarg;        /* exit condition */
        int             termstat;       /* signal that caused termination */
        boolean         delete;         /* do we delete job entry from queue? */
	int		devstat;	/* status to send to status file */
	int		i;


	childterm=0;			/* inittialize to 0*/
	exitarg = 0;
	if (WIFEXITED(status))		/* get rid of -1 */
		exitarg = WEXITSTATUS(status);
	termstat = 0;
	if (WIFSIGNALED(status))	/* get rid of -1 */
		termstat = WTERMSIG(status);

#ifdef DEBUG
        if (getenv("DEVFIN"))
	{
                sysraw("unrun: termstat = [%d]; exitarg = [%d]\n",termstat,exitarg);
		if(dev->d_e->e_s->s_feed != NOFEED)
			sysraw("       (unrunning a feed job)\n");
	}
#endif
        /*----The child process is not running anymore */
        unlink(pidname(dev));

        /*----Impose some regularity on backend exits
                unfriendly backends are made to look o.k., since we
                don't trust their exit codes.
                if backend was canceled, make it look like
                it terminated ok and exited EXITSIGNAL.  This way, a
                backend needn't catch SIGTRM if there's no cleanup to do.  */
        if (termstat == SIGTERM || exitarg == EXITSIGNAL)
        {
                termstat = 0;
                exitarg = EXITSIGNAL;
		childterm = SIGTERM;
        }

        /*----If funky signal caused termination, there is a backend problem (like time-out) */
        if (termstat != 0)
	{
		exitarg = EXITBAD;
                syswarn(MSGSTR(MSGBOMB,"Backend `%s'\nfailed with termination status 0%o"),
                         dev->d_backend,termstat);
		termstat = 0;
	}

	/*----Defaults actions for normal termination and enq -x,-X */
	devstat = READY;
	delete = TRUE;

        /*----If device is down, must have been turned off by oper ( enq -K or -D ) */
        if (dev->d_up == FALSE)
        {
                devstat = OFF;

                /*----If job didnt finish (usually EXITSIGNAL), leave in queue */
                if (exitarg != EXITOK)
                        delete = FALSE;
        }
	else
	{
       		/*----Turn device off if fatal exit ( problem within backend ) or exec problem */
/* Added for A12747 */
       		if (exitarg == EXITIO)
       		{
       	        	dev->d_up = TRUE;
        	       	devstat = WAITING;
			delete = FALSE;
       		}
       		if (exitarg == EXITFATAL ||
		    exitarg == EXITBAD   ||
		    exitarg == EXITEXEC)
       		{
       	        	dev->d_up = FALSE;
        	       	devstat = OFF;
			delete = FALSE;
       		}
                /* Defect 44281 */
		if (exitarg == EXITERROR)
                {
                        dev->d_up = TRUE;
                        if (dev->d_e->e_retries < MAXRETRIES)
                        {
                                /*----Try it again on the same device */
                                delete = FALSE;
                        	devstat = RUNNING;
                                strcpy(dev->d_e->e_dnam, dev->d_name);
                        }
                        else
                        {
                                /*----Remove it from the queue */
                                delete = TRUE;
                        	devstat = READY;
                        }
                        dev->d_e->e_retries++;
                }
	}
	/*----Prevent printing of feed pages if fatal exit ( printer busted ) */
	if (exitarg == EXITFATAL)
		dev->d_user[0] = '\0';

       	/*----Normal exit, delete it, device ready */
       	setstat(stname(dev),devstat);

        /*----Detach if not feed job */
        if (dev->d_e->e_s->s_feed == NOFEED)
	{
        	/*----Notify right people */
                unotify(dev->d_e,exitarg);

		/*----Do accounting */
        	if (termstat == 0 && (exitarg == EXITOK || exitarg == EXITSIGNAL))
                	charge(dev->d_e,dev);
 	
		/*----Delete and detach the e struct attached to dev, if we are supposed to */
		dev->d_e->e_device = NULL;      /* this job done */
		if (delete == TRUE)
			/*----Remove from queue */
			edelete(dev->d_e,dev->d_q);
		else
		{
		        /*----Dump old arguments from possible previous attempt to run job */
        		for(i = 0; i < (dev->d_e->e_argnum - dev->d_e->e_jdfnum); i++)
                		free((void *)dev->d_e->e_argvec[i]);
        		dev->d_e->e_argnum = 0;
		}
	}
	else
	{
		/*----Free stuff for feed job */
		jdffree(dev->d_e);		/* free all stuff within e struct */
		free((void *)dev->d_e);		/* job itself */
	}
	dev->d_e = NULL;                /* this device done */
	dev->d_pid = 0;                 /* show idle */
}

/*
 * remove entry e from queue q.
 * clean up any tmp files.  unlink the name from the directory.
	Entry DELETE ( jdf delete or request delete)
 */
edelete(e,q)
struct e *e;
struct q *q;
{
	edel1(e,q);		/* unhook the e from the queue */
	rmlist(e);		/* delete files */
	unlink(e->e_name);
	jdffree(e);
	q->q_entcount--;
	free((void *)e);
}


/*
 * given an e and the q it's on, delete the e from the q_entlist
 */
edel1(e,q)
struct e *e;
register struct q *q;
{
	struct e *p1, *p2;

	if (q->q_entlist == e)
	{
		q->q_entlist = e->e_next;       /* unlink it */
		return(0);
	}
	for (p1 = q->q_entlist; (p2 = p1->e_next) != NULL; p1 = p2)
	{   
 		if (p2 == e)
		{
			p1->e_next = p2->e_next;
			return(0);
		}
	}
}

/* is there an up dev on queue q to run job e? */
isdev(e,q)
struct e *e;
struct q *q;
{
	register struct d *d;
	char *devnam;

	devnam = e->e_dnam;
	for (d = q->q_devlist; d; d = d->d_next)
	{
		if (d->d_up == FALSE)       /* must be up */
			continue;
		if (!strcmp(devnam,ANYDEV) || !strcmp(devnam,d->d_name))
			return(TRUE);
	}
	return(FALSE);
}

/*
 * notify proper people on termination of job e
 *      OK -- if -no (user and to if different).
 *      BAD -- user
 *      WARN -- user 
 *      ERROR -- user (if number of retries exceeded)
 *      FATAL -- user
 *      SIGNAL -- nobody (user killed it)
 */
unotify(e,exitarg)
register struct e *e;
{
	char umessage[MAXLINE];
	char devname[QNAME + DNAME + 2];
	char *queue;

	/*----Derive the queue:device name for messages */
	queue = getqn(e->e_name);
	sprintf(devname,"%s:%s",queue, e->e_device->d_name);

	/*----Sucessful completion of backend */
	if (exitarg == EXITOK && e->e_no)	/* notify? */
	{
		sprintf(umessage,MSGSTR(MSGFINI,"Request %s has finished on device %s."),
			e->e_request,devname);
		sysnot(e->e_from, NULL, umessage, e->e_mail ? DOMAIL : DOWRITE);
		if (e->e_to == NULL || strcmp(e->e_to,e->e_from) == 0)
			return(0);

		sprintf(umessage,MSGSTR(MSGSENT,
			"Request %s sent to you by %s\nhas finished on device %s."),
			e->e_request,e->e_from,devname);
		sysnot(e->e_to, NULL, umessage, e->e_mail ? DOMAIL : DOWRITE);
		return(0);
	}

	/* Defect 11226 */
	/*----Problem with backend, could not open or stat file----*/
        if (exitarg == EXITWARN)
        {
                sprintf(umessage,MSGSTR(MSGBADFILE,
                        "Request %s removed from queue %s.  Could not open or stat file(s)."),
                        e->e_request,devname);
                sysnot(e->e_from, NULL, umessage, e->e_mail ? DOMAIL : DOWRITE);

                syswarn(umessage);
                return(0);
        }

	/*----Problem with backend, and no other devices to run on */
	if ((exitarg == EXITBAD || exitarg == EXITFATAL) && !isdev(e, e->e_device->d_q))
	{
		sprintf(umessage,MSGSTR(MSGDIED,
			"Device %s died running request %s which is still queued."),
			devname, e->e_request);
		sysnot(e->e_from, NULL, umessage, e->e_mail ? DOMAIL : DOWRITE); 
		syswarn(umessage);
		return(0);
	}

        /* Defect 44281 */
        /*----Problem with backend, failed after max retries */
        if ((exitarg == EXITERROR) && (e->e_retries > MAXRETRIES))
        {
                sprintf(umessage,MSGSTR(MSGMAXRTRY,
                        "Request %s removed from queue %s.  Failed initial attempt and %d retries."),
                        e->e_request,devname, MAXRETRIES);
                sysnot(e->e_from, NULL, umessage, e->e_mail ? DOMAIL : DOWRITE);

                syswarn(umessage);
                return(0);
        }

	/*----Problem with (setting up to) execute backend */
	if (exitarg == EXITEXEC)
	{
		sprintf(umessage,MSGSTR(MSGRQST,
			"Could not run your request %s for device %s."),
			e->e_request,devname);
		sysnot(e->e_from, NULL, umessage, e->e_mail ? DOMAIL : DOWRITE);
		return(0);
	}
}


/* charge user for running e on d */

#include "accrec.h"
#include <fcntl.h>
#define OPENMODE (O_WRONLY|O_APPEND|O_CREAT)

charge(e,d)
register struct e *e;
register struct d *d;
{

	struct stfile stfile;
	register int  stfd, acctfd;
	register char *acctf;
	register long npages;

	/* see if we are to do accounting */
	acctf = d->d_q->q_acctf;
	if (!acctf[0])                  /* no account file specified */
	    return(0);

	if ((acctfd = open(acctf,OPENMODE,700)) == -1)
	    return(0);                     /* account file not there */


	/* dig out the info to go into the file */

	/* read the charge.  reset to 0 to prevent accidents. */
	if ((stfd = open(stname(d),2)) == -1)
	    syserr((int)EXITFATAL,MSGSTR(MSGSOPN,"Error opening %s.  Errno = %d."),stname(d),errno);
	if (read(stfd,&stfile,sizeof(stfile)) != sizeof(stfile))
	    syserr((int)EXITFATAL,MSGSTR(MSGSRED,"Error reading %s.  Errno = %d."),stname(d),errno);
	npages = stfile.s_charge;
	stfile.s_charge = 0;
	lseek(stfd,0L,0);
	write(stfd,&stfile,sizeof(stfile));
	close(stfd);
	if (npages == 0)         /* no 0 charges */
	{   close(acctfd);
	    return(0);
	}

#ifdef DEBUG
	  if (getenv("CHARGE"))
		 sysraw("charging %s for %ld pages\n",e->e_from,npages);
#endif

	if (lseek(acctfd,0L,2) == -1)
	    syserr((int)EXITFATAL,MSGSTR(MSGSSEK,"Seek error on file %s.  Errno = %d."),acctf,errno);
/* AIX security enhancement 								*/
/* we'll get the uid from the status file now instead of e->e_uid (it was removed). 	*/
	strcpy (acctrec.from, stfile.s_from);	/* who printed this */
	acctrec.acctchar = e->e_acct;
	acctrec.acctdate = e->e_time;
	acctrec.pages = npages;
	acctrec.numjobs = 1;
	if (write(acctfd,&acctrec,sizeof(acctrec)) != sizeof(acctrec))
	    syserr((int)EXITFATAL,MSGSTR(MSGSWRT,"Error writing to %s.  Errno = %d."),acctf,errno);
#ifdef NOTDEF
	{   char sbuf9[MAXLINE];
	    int len;
/* AIX security enhancement 								*/
/* we'll get the uid from the status file now instead of e->e_uid (it was removed). 	*/
	    sprintf(sbuf9,"%u\t%-9s %ld\n", stfile.s_uid, e->e_from, npages);
	    len = strlen(sbuf9);
	    if (write(acctfd,sbuf9,len) != len)
	    	syserr((int)EXITFATAL,MSGSTR(MSGSWRT,"Error writing to %s.  Errno = %d."),acctf,errno);
	}
#endif /*NOTDEF*/
	close(acctfd);
}

