static char sccsid[] = "@(#)19	1.17.2.2  src/bos/usr/lib/sendmail/queue.c, cmdsend, bos41J, 9510A_all 2/24/95 10:45:52";
/* 
 * COMPONENT_NAME: CMDSEND queue.c
 * 
 * FUNCTIONS: MSGSTR, dowork, freewq, orderq, plist, printqueue, 
 *            prtq, queuename, queueup, readqf, runqueue, trunqueue, 
 *            unlockqueue, workcmpf, getctluser, setctluser, clrctluser, 
 *	      setctladdr
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
*/


#include <nl_types.h>
#include "sendmail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

# include <stdio.h>
# include <ctype.h>
# include <time.h>
# include <memory.h>
# include <fcntl.h>

# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <netinet/in.h>

# include "sysexits.h"

# include "sendmail.h"

# include <sys/stat.h>
# include <sys/dir.h>
# include <unistd.h>			/* for access () call */

# include <errno.h>
# include <string.h>
# include <sys/access.h>		/* security */
# include <sys/file.h>
# include <pwd.h>

void  exit ();
void  qsort ();
int putbody();
int workcmpf();
long atol();
char *fgetfolded();
char  *queuename ();
char  *xalloc ();
char  *safestr ();
EVENT  *setevent ();

/*
**  Work queue.
*/

struct work
{
	char		*w_name;	/* name of control file */
	long		w_pri;		/* priority of message, see below */
	long		w_ctime;	/* creation time of message */
	char		*w_df;		/* name of data file */
	struct work	*w_next;	/* next in queue */
};

typedef struct work	WORK;

static WORK  *WorkQ;			/* list of things to be done */
static WORK *freewq (WORK *);
static orderq (int);
static int prtq (char *, int);
static int plist (char *, WORK **, int);

extern int  Sid;			/* semaphore id for cleanup */
extern int  Scount;			/* max count of users */
/*
**  QUEUEUP -- queue a message up for future transmission.
**
**	Parameters:
**		e -- the envelope to queue up.
**		queueall -- if TRUE, queue all addresses, rather than
**			just those with the QQUEUEUP flag set.
**		announce -- if TRUE, tell when you are queueing up.
**
**	Returns:
**		none.
**		The only report of error is through syserr. This is bad.
**
**	Side Effects:
**		The current request are saved in a control file.
*/

queueup(e, queueall, announce)
	register ENVELOPE *e;
	int queueall;
	int announce;
{
	char *tf;
	char *qf;
	char buf[MAXLINE];
	register FILE *tfp;
	register HDR *h;
	register ADDRESS *q;
	MAILER nullmailer;
	int fd, ret;
	char *ctluser, *getctluser();

	/*
	**  Create control file.
	*/
	/*
	 *  Should we do an unlink first?
	 *  Then if main umasked properly we wouldn't need the chmod.
	 */

	tf = newstr(queuename(e, 't'));
	tfp = fopen(tf, "w");
	if (tfp == NULL)
	{
		syserr(MSGSTR(QU_ETEMP, "queueup: cannot create temp file %s"), tf); /*MSG*/
		return;
	}
	(void) acl_set (tf, R_ACC | W_ACC, R_ACC | W_ACC, NO_ACC);
#ifdef DEBUG
	if (tTd(40, 1))
		(void) printf("queueing %s\n", e->e_id);
#endif DEBUG

	/*
	**  If there is no data file yet, create one.
	*/
	
	if (e->e_df == NULL)
	{
		register FILE *dfp;
		
		e->e_df = newstr(queuename(e, 'd'));
		dfp = fopen(e->e_df, "w");
		if (dfp == NULL)
		{
			syserr(MSGSTR(QU_CREAT, "queueup: cannot create %s"), e->e_df); /*MSG*/
			(void) fclose(tfp);
			return;
		}
		(void) acl_set(e->e_df, R_ACC | W_ACC, R_ACC | W_ACC, NO_ACC);
		(*e->e_putbody)(dfp, ProgMailer, e, e->e_btype);
		(void) fclose(dfp);
		e->e_putbody = putbody;
	}

	/*
	**  Output future work requests.
	**	Priority and creation time should be first, since
	**	they are required by orderq.
	*/

/**/
/* What about checking the return codes on the following fprintf's? */
	/* output message priority */
	fprintf(tfp, "P%ld\n", e->e_msgpriority);

	/* output creation time */
	fprintf(tfp, "T%ld\n", e->e_ctime);

	/* output name of data file */
	fprintf(tfp, "D%s\n", e->e_df);

	/* output body type if BT_NLS or BT_ESC */
	if (e->e_btype == BT_ESC)
	    fprintf(tfp, "Be type=NLesc\n");
	else if (e->e_btype == BT_NLS)
	    fprintf(tfp, "Bn type=NLS\n");

	/* output body type */
	if (e->e_btype == BT_MC)
	    fprintf(tfp, "Bs type=%s\n", MailCode);
	else if (e->e_btype == BT_NC)
	    fprintf(tfp, "Bj type=%s\n", NetCode);

	/* message from envelope, if it exists */
	if (e->e_message != NULL)
		fprintf(tfp, "M%s\n", safestr(e->e_message));

	/* output name of sender */
	fprintf(tfp, "S%s\n", safestr(e->e_from.q_paddr));

	/* output list of recipient addresses */
	for (q = e->e_sendqueue; q != NULL; q = q->q_next)
	{
		if (queueall ? !bitset(QDONTSEND, q->q_flags) :
			       bitset(QQUEUEUP, q->q_flags))
		{
			if ((ctluser = getctluser(q)) != NULL)
				fprintf(tfp, "C%s\n", safestr(ctluser));
			fprintf(tfp, "R%s\n", safestr(q->q_paddr));
			if (announce)
			{
				e->e_to = q->q_paddr;
				message(Arpa_Info, MSGSTR(QU_QUARPA, "queued")); /*MSG*/
				if (LogLevel > 4)
					logdelivery(MSGSTR(QU_QUARPA, "queued")); /*MSG*/
				e->e_to = NULL;
			}
#ifdef DEBUG
			if (tTd(40, 1))
			{
				(void) printf("queueing ");
				printaddr(q, FALSE);
			}
#endif DEBUG
		}
	}

	/* output list of error recipients */
	for (q = e->e_errorqueue; q != NULL; q = q->q_next)
	{
		if (!bitset(QDONTSEND, q->q_flags))
		{
			if ((ctluser = getctluser(q)) != NULL)
				fprintf(tfp, "C%s\n", safestr(ctluser));
			fprintf(tfp, "E%s\n", safestr(q->q_paddr));
		}
	}

	/*
	**  Output headers for this message.
	**	Expand macros completely here.  Queue run will deal with
	**	everything as absolute headers.
	**		All headers that must be relative to the recipient
	**		can be cracked later.
	**	We set up a "null mailer" -- i.e., a mailer that will have
	**	no effect on the addresses as they are output.
	*/

	(void) memset((char *) &nullmailer, 0, sizeof nullmailer);
	nullmailer.m_r_rwset = nullmailer.m_s_rwset = -1;
	nullmailer.m_eol = "\n";

	define('g', "\001f", e);
	for (h = e->e_header; h != NULL; h = h->h_link)
	{
		/* don't output null headers */
		if (h->h_value == NULL || h->h_value[0] == '\0')
			continue;

		/* don't output resent headers on non-resent messages */
		if (bitset(H_RESENT, h->h_flags) && !bitset(EF_RESENT, e->e_flags))
			continue;

		/* output this header */
		fprintf(tfp, "H");

		/* if conditional, output the set of conditions */
		if (!bitzerop(h->h_mflags) && bitset(H_CHECK|H_ACHECK, h->h_flags))
		{
			int j;

			(void) putc('?', tfp);
			for (j = '\0'; j <= '\177'; j++)
				if (bitnset(j, h->h_mflags))
					(void) putc(j, tfp);
			(void) putc('?', tfp);
		}

		/* output the header: expand macros, convert addresses */
		if (bitset(H_DEFAULT, h->h_flags))
		{
			(void) expand(h->h_value, buf, &buf[sizeof buf], e);
			fprintf(tfp, "%s: %s\n", h->h_field, buf);
		}
		else if (bitset(H_FROM|H_RCPT, h->h_flags))
		{
			commaize(h, h->h_value, tfp, bitset(EF_OLDSTYLE,  
				 e->e_flags), &nullmailer);
		}
		else
			fprintf(tfp, "%s: %s\n", h->h_field, h->h_value);
	}

	/*
	**  Clean up.
	*/

	(void) fclose(tfp);
	qf = queuename(e, 'q');
	if (tf != NULL)
	{
		(void) unlink(qf);
		if (rename(tf, qf) < 0)
			syserr(MSGSTR(QU_ERENAME, "cannot rename(%s, %s), df=%s"), tf, qf, e->e_df); /*MSG*/
		errno = 0;
	}
# ifdef LOG
	/* save log info */
	if (LogLevel > 15)
		syslog(LOG_DEBUG, "%s: queueup, qf=%s, df=%s\n", e->e_id, qf, e->e_df);
# endif LOG
}
/*
**  TRUNQUEUE -- handle timed queue runs.  This is called from daemon/queue 
**		 background (and possibly its clock signal handler).
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Calls runqueue.
*/

trunqueue ()
{
	int pid;

	/*
	 *  When the clock event occurs, trunqueue is started
	 *  from the signal handler.  It forks, then returns,
	 *  terminating the signal handler.
	 *
	 *  Trunqueue is called first at base level by main.c.
	 */
	if (QueueIntvl != 0)
	    (void) setevent (QueueIntvl, trunqueue, TRUE);

	/*
	 *  If no work will ever be selected, don't even bother reading
	 *  the queue.
	 *
	 *  How can the value of shouldqueue EVER be TRUE?
	 *  Look at the code in it!
	 */
	if (shouldqueue(-100000000L))
	{
	    if (Verbose)
	        (void) printf(MSGSTR(QU_SKIP, "Skipping queue run -- load average too high\n")); /*MSG*/

	    return;
	}

	/*
	 *  Make child.  If fails, then forget it.  The alarm will trigger
	 *  another attempt later.  This used to be a fork with retry
	 *  to try to overcome lack of system resources, but now it just
	 *  waits for the next queue interval.  Nothing is lost but time.
	 */
	pid = fork();
	if (pid != 0)		/* parent or failed */
	    return;

	/*
	 *  "Wait" on sema4 to indicate that we are preparing to enter queue.
	 *  We can wait here if a clean is in progress.  Process exit
	 *  releases our "wait" operation.  This sets the semadj value
	 *  in this child process so that the semaphore is adjusted
	 *  properly upon process exit.  Runqueue may fiddle with the
	 *  semaphore, also.  This wait is done on the child side of
	 *  the above fork.
	 *
	 *  All failures cause syserr's.  However, it is assumed that once
	 *  semop's fail, they always fail.  This means that no queue
	 *  cleaning will take place in orderq.
	 */
	(void) semwait (Sid, 1, 0, 0);

#ifdef DEBUG
	if (tTd (0, 109))
	    exit (99);
#endif DEBUG

	runqueue ();

	/* exit without the usual cleanup */
	exit(ExitStat);
}
/*
**  RUNQUEUE -- run the jobs in the queue.
**
**	Gets the stuff out of the queue in some presumably logical
**	order and processes it.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		runs things in the mail queue.
*/
runqueue ()
{
	int  i;

	/*
	 *  If no work will ever be selected, don't even bother reading
	 *  the queue.
	 *
	 *  How can the value of shouldqueue EVER be TRUE?
	 *  Look at the code in it!
	 */
	if (shouldqueue(-100000000L))
	{
	    if (Verbose)
	        (void) printf(MSGSTR(QU_SKIP, "Skipping queue run -- load average too high\n")); /*MSG*/

	    return;
	}

	setproctitle(MSGSTR(QU_RUNQ, "running queue: %s"), QueueDir); /*MSG*/

# ifdef LOG
	if (LogLevel > 11)
		syslog(LOG_DEBUG, "runqueue %s, pid=%d", QueueDir, getpid());
# endif LOG

	/*
	 *  Release any resources used by the daemon code.
	 *
	 *  Right now this just assures that the daemon socket (in this
	 *  instantiation of sendmail) is closed.  If we have just been 
	 *  forked from the daemon, then our new instantiation is still 
	 *  doing the daemon thing and should be stopped.
	 *
	 *  This code is either executed unforked (-q with no time specs)
	 *  or forked (-q with time specs), but may or may not be from daemon.
	 */
/**/	clrdaemon();

	/*
	**  Make sure the alias database is open.
	*/

	if (i = openaliases(AliasFile))
	{
	    ExitStat = i;		/* inaccessible database	*/
#ifdef LOG
	    syslog(LOG_INFO, MSGSTR(QU_EALIAS, "alias file \"%s\" data base open failure"), AliasFile); /*MSG*/
#endif LOG
	    return;
	}

	/* order the existing work requests */
	(void) orderq(FALSE);

	/* process them one at a time */
	while (WorkQ != NULL)
	{
		WORK *w = WorkQ;

		WorkQ = WorkQ->w_next;
		dowork(w);
		(void) freewq (w);
	}
}
/*
**  ORDERQ -- order the work queue.
**
**	Parameters:
**		doall -- if set, include everything in the queue (even
**			the jobs that cannot be run because the load
**			average is too high).  Otherwise, exclude those
**			jobs.
**
**	Returns:
**		The number of request in the queue (not necessarily
**		the number of requests in WorkQ however).
**
**	Side Effects:
**		Sets WorkQ to the queue of available work, in order.
*/

# define NEED_P		001
# define NEED_T		002
# define NEED_D		004

static
orderq(int doall)
{
	struct dirent  *dirent;
	WORK  we;
	WORK  *w, **wlist, **wl, *wq, **wqp;
	DIR  *fd;
	register int i;
	int  wn, pass2, err;
	int  clean;

	if (WorkQ != NULL)
	{
	    syserr (MSGSTR(QU_WORK, "orderq: WorkQ exists!")); /*MSG*/
	    exit (EX_SOFTWARE);
	}

	/* open the queue directory */
	fd = opendir (".");
	if (fd == NULL)
	{
	    syserr(MSGSTR(QU_EOPEN, "orderq: cannot open queue directory %s as \".\""), QueueDir); /*MSG*/
	    return (0);
	}

	/*
	 *  See if we do cleanup at this time.
	 *  Attempt to pick up all remaining available "wait"s in the
	 *  semaphore.  If successful, no one else is in the queue and
	 *  we may clean.  Furthermore, no one else may enter the queue
	 *  till we are through.  This also adjusts the process semadj
	 *  value to automatically release all these "wait" counts upon
	 *  process exit for any reason.
	 *
	 *  It is assumed that once a semop fails, all subsequent semops
	 *  will also fail.  All failures are syserr'd, but don't
	 *  otherwise interfere with execution.
	 */
	clean = 0;				/* assume no cleanup */
	if (semwait (Sid, Scount - 1, 1, 0) == EX_OK)
	{
	    clean = 1;				/* do cleanup */
	    if (Verbose)
		message (0, MSGSTR(QU_CLEAN, "000 Queue Directory \"%s\" will be cleaned"), QueueDir); /*MSG*/
#ifdef DEBUG
	    if (tTd (2, 1))
		(void) printf ("orderq: sema4 allows clean\n");
#endif DEBUG

	    /*
	     *  You MUST restore the semaphore count before returning from
	     *  this routine if clean is true!  Also, exit () will do it
	     *  automatically.
	     */
	}

#ifdef DEBUG			/* sema4 testing */
	if (tTd (2, 11))
	    while (1);
	if (tTd (2, 10))
	    exit (0);
#endif DEBUG

	/*
	**  Read the work directory.
	*/

	wn = 0;
	pass2 = 0;
	while (1)
	{
	    err = 0;			/* exit signal for inner loop below */

	    /*
	     *  Position to start of directory (especially for pass 2).
	     */
	    (void) rewinddir (fd);

	    while (1)
	    {
		FILE *cf;
		char lbuf[MAXNAME];

		/*
		 *  Read next directory entry
		 */
	        dirent = readdir (fd);
		if (dirent == NULL)		/* end?			*/
                    break;

		/*
		 *  Zero inode means this slot is empty.
		 */
		if (!dirent->d_ino)
		    continue;

		/*
		 *  We are only concerned with files w/ 2nd char 'f'.
		 */
		if (dirent->d_name[1] != 'f')
		    continue;

#ifdef DEBUG
		if (tTd (40, 1))
		    (void) printf ("orderq: file %s, pass2 = %d\n", 
							dirent->d_name, pass2);
#endif DEBUG

		/*
		 *  If pass 1.
		 */
		if (!pass2)
		{
		    int  eof;

		    if (clean)			/* if clean mode */
		    {
		        /*
		         *  Get rid of [lxt]f* files.
		         */
		        if (dirent->d_name[0] == 'l' ||
			    dirent->d_name[0] == 'x' ||
			    dirent->d_name[0] == 't'   )
		        {
#ifdef DEBUG
			    if (tTd (40, 1))
				(void) printf ("orderq: unlink (%s), scratch\n",
								dirent->d_name);
#endif DEBUG
			    (void) unlink (dirent->d_name);
			    continue;
		        }
		    }

		    /*
		     *  Only continue to look at qf files.
		     */
		    {
		    	if (dirent->d_name[0] != 'q')
			    continue;
		    }

		    /*
		     *  Open the qf* file.  Pass 1 only.
		     */
#ifdef DEBUG
		    if (tTd (40, 1))
			(void) printf ("orderq: fopen (%s)\n", dirent->d_name);
#endif DEBUG
		    cf = fopen(dirent->d_name, "r");
		    if (cf == NULL)
		    {
		        syserr(MSGSTR(QU_EOPEN2, "orderq: cannot open %s"), dirent->d_name); /*MSG*/
		        continue;
		    }

		    /*
		     *  Fill in the working entry.
		     */
		    we.w_name = newstr (dirent->d_name);

		    /*
		     *  Clear cells to be filled in, since corresponding
		     *  items may not be read from the file.  Some of this
		     *  old code assumes that qf files may exist which are
		     *  in a state of creation.  I don't think this is
		     *  possible any more, but the code and comments are
		     *  left here.
		     */
		    /* make sure jobs in creation don't clog queue */
		    we.w_pri = 0x7fffffff;
		    we.w_ctime = 0;

		    /* clear the 'D' name ptr */
		    we.w_df = NULL;

		    /* extract useful information */
		    i = NEED_P | NEED_T;
		    if (clean)
		        i |= NEED_D;

		    eof = 1;
		    while (i != 0 && fgets(lbuf, sizeof lbuf, cf) != NULL)
		    {
			eof = 0;
		        switch (lbuf[0])
		        {
			    case 'P':
			        we.w_pri = atol(&lbuf[1]);
			        i &= ~NEED_P;
			        break;

			    case 'T':
			        we.w_ctime = atol(&lbuf[1]);
			        i &= ~NEED_T;
			        break;

			    case 'D':
			        we.w_df = newstr (&lbuf[1]);
			        we.w_df[strlen (we.w_df)-1] = '\0'; /* fix nl */
			        i &= ~NEED_D;
			        break;
		        }
		    }
		    (void) fclose(cf);

		    /*
		     *  If file was empty, remove it.  It can't be in process
		     *  of creation by anybody.
		     */
		    if (clean && eof)
		    {
#ifdef DEBUG
			if (tTd (40, 1))
			    (void) printf ("orderq: unlink (%s), empty\n",
								dirent->d_name);
#endif DEBUG
			(void) unlink (dirent->d_name);
			continue;
		    }

		    /*
		     *  Include it in the work queue.
		     */
		    w = (WORK *) xalloc (sizeof (WORK));
#ifdef DEBUG
		    if (tTd (40, 1))
			(void) printf ("orderq: entry 0x%x: enqueue %s, pri %ld, time %ld, df %s\n",
				   w, we.w_name, we.w_pri, we.w_ctime, we.w_df);
#endif DEBUG
		    w->w_name  = we.w_name;
		    w->w_pri   = we.w_pri;
		    w->w_ctime = we.w_ctime;
		    w->w_df    = we.w_df;
		    w->w_next  = WorkQ;
		    WorkQ = w;
		    wn++;
		}
		else			/* pass 2  ->  clean */
		{
		    /*
		     *  If we are in pass2 of clean mode, make sure that
		     *  any df file is mentioned in a qf entry.
		     */
		    if (dirent->d_name[0] == 'd')
		    {
		        for (w = WorkQ; w != NULL; w = w->w_next)
		        {
			    if (strcmp (dirent->d_name, w->w_df) == 0)
			        break;
		        }
		        if (w == NULL)
			{
#ifdef DEBUG
			    if (tTd (40, 1))
				(void) printf ("orderq: unlink (%s), unused\n",
								dirent->d_name);
#endif DEBUG
			    (void) unlink (dirent->d_name);
			}
		    }
		}	/* end of else clause for pass 2 */
	    }	/* end inner (dir pass) loop */

	    /*
	     *  If inner loop broke on error instead of eof, just exit.
	     *  Also, break if we just finished pass2 or don't need it.
	     */
	    if (err || pass2 || !clean)
		break;

	    pass2 = !pass2;			/* go around again */
	}
	(void) closedir (fd);

	/*
	 *  If cleaned, remove excess semaphore lock counts.  This allows
	 *  other sendmail invocations to proceed.  This will adjust
	 *  the process semadj value back so that the single remaining
	 *  lock due to us will be removed on process exit.
	 *  Do nothing special if the unlock fails.
	 *
	 *  All semop exceptions cause syserr.
	 */
	if (clean)
	{
#ifdef DEBUG
	    if (tTd (40, 1))
		(void) printf ("orderq: remove exclusive clean lock\n");
#endif DEBUG
	    (void) semsig (Sid, Scount - 1, 0);
	}

	/*
	 *  Return if no entries were found.
	 */
# ifdef DEBUG
	if (tTd(40, 1))
	    (void) prtq ("orderq: preliminary WorkQ", wn);
# endif DEBUG

	if (wn <= 0)
	    return (0);

	/*
	 *  Create a sort record pointer list in contiguous memory.
	 *  Check shouldqueue ().
	 *  Release all WorkQ entries that aren't used.
	 */
	wlist = (WORK **) xalloc (wn * (int) sizeof (WORK *));

	wq = WorkQ;
	wl = wlist;
	while (wq != NULL)
	{
	    if (doall || !shouldqueue (wq->w_pri))
	    {
		*wl++ = wq;		/* put entry addr on list */
	        wq = wq->w_next;	/* get address of next one */
	    }
	    else
		wq = freewq (wq);	/* free entry; rtns addr of next one */
	}

	wn = wl - wlist;
	if (wn <= 0)
	    WorkQ = NULL;

	/*
	 *  Return if there are no entries left after shouldqueue.
	 */
#ifdef DEBUG
	if (tTd (40, 1))
	    (void) plist ("orderq: wlist before sort", wlist, wn);
#endif DEBUG

	if (wn <= 0)
	    return (0);

	/*
	 *  Sort what's there.
	 */
	qsort((char *) wlist, (unsigned) wn, sizeof (WORK *), workcmpf);

#ifdef DEBUG
	if (tTd (40, 1))
	    (void) plist ("orderq: wlist after sort", wlist, wn);
#endif DEBUG

	/*
	 *  Copy sorted links back onto WorkQ entries.
	 *
	 *	Should be turning it into a list of envelopes here perhaps.
	 */
	wqp = &WorkQ;
	wl = wlist;
	for (i = 0; i < wn ; i++)
	{
	    *wqp = *wl++;
	    wqp = &(*wqp)->w_next;
	}
	*wqp = NULL;			/* terminate the list */

# ifdef DEBUG
	if (tTd(40, 1))
	    (void) prtq ("orderq: final WorkQ", wn);
# endif DEBUG

	return (wn);
}

/*
 *  freewq - free work queue entry.  Return link value.
 */
static WORK *
freewq (WORK *w)
{
	WORK  *nw;

#ifdef DEBUG
	if (tTd (40, 1))
	    (void) printf ("freewq: free control name %s\n", w->w_name);
#endif DEBUG
	free (w->w_name);

	if (w->w_df != NULL)
	{
#ifdef DEBUG
	    if (tTd (40, 1))
		(void) printf ("freewq: free data name %s\n", w->w_df);
#endif DEBUG
	    free (w->w_df);
	}

	nw = w->w_next;

#ifdef DEBUG
	if (tTd (40, 1))
	    (void) printf ("freewq: free workq entry 0x%x\n", w);
#endif DEBUG
	free ((char *) w);

	return (nw);
}

/*
 *  prtq - print the work queue (debug only)
 */
static int
prtq (char *s, int wn)
{
    WORK *w;

    (void) printf ("%s (%d entries):\n", s, wn);
    for (w = WorkQ; w != NULL; w = w->w_next)
	(void) printf("\t%s: pri=%ld\n", w->w_name, w->w_pri);
}

/*
 *  plist - print the sort list (debug only)
 */
static int
plist (char *s, WORK **wlist, int wn)
{
    int  i;

    (void) printf ("%s (%d entries):\n", s, wn);
    for (i=0; i<wn; i++)
	(void) printf ("\t0x%x\n", wlist[i]);
}
/*
**  WORKCMPF -- compare function for ordering work.
**
**	Parameters:
**		a -- the first argument.
**		b -- the second argument.
**
**	Returns:
**		-1 if a < b
**		 0 if a == b
**		+1 if a > b
**
**	Side Effects:
**		none.
*/

workcmpf(a, b)
	WORK **a;
	WORK **b;
{
	WORK *wa, *wb;
	long  pa,  pb;

	wa = *a;
	wb = *b;

	pa = wa->w_pri + wa->w_ctime;
	pb = wb->w_pri + wb->w_ctime;

	if (pa == pb)
		return (0);
	else if (pa > pb)
		return (1);
	else
		return (-1);
}
/*
**  DOWORK -- do a work request.
**
**	Parameters:
**		w -- the work request to be satisfied.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The work request is satisfied if possible.
*/

dowork(w)
	register WORK *w;
{
	register int i;

# ifdef DEBUG
	if (tTd(40, 1))
		(void) printf("dowork: %s pri %ld\n", w->w_name, w->w_pri);
# endif DEBUG

	/*
	**  Ignore jobs that are too expensive for the moment.
	*/

	if (shouldqueue(w->w_pri))
	{
		if (Verbose)
			(void) printf(MSGSTR(QU_ESKIP, "\nSkipping %s\n"), w->w_name + 2); /*MSG*/
		return;
	}

	/*
	**  Fork for work.
	*/

	/*
	 *  Forking here makes each queue item into a separate process.
	 *  However, we wait for the completion of each process, one-by-one.
	 *  If the fork fails, then we might be in a mode where we go down
	 *  the work queue and fail on fork each time.  This is unnecessary,
	 *  but that's how we do it now.  No work is lost.  It remains in the
	 *  disk queue.  Currently there is no error indication.  However, 
	 *  SOME indication might be nice.
	 *
	 *  The real purpose of this fork is to conserve memory.  A long queue
	 *  run can cause a lot of memory to be taken up because sendmail 
	 *  doesn't do a good job of free'ing all memory after use.  (See the
	 *  newstr function.)  The idea here is that after each queue msg
	 *  the forked child returns, thus releasing all the memory allocated
	 *  to send the message.  However, this is probably unnecessary on
	 *  a virtual memory system like AIX.  It has the added disadvantage
	 *  of losing information in the symbol table about remote host
	 *  status.
	 *
	 *  Since the child status isn't used, we may continue in zombiless
	 *  mode and merely perform a global wait to synchronize with child.
	 */
	i = 0;			/* pseudochild */
	if (ForkQueueRuns)
	{
	    i = fork();
	    if (i < 0)
	    	return;		/* failed, just drop this one */

	    if (i == 0)		/* child? */
	    {
	        /*
	         *  "Wait" on semaphore to indicate that we are preparing to 
		 *  enter queue.  Process exit releases our "wait" operation.  
		 *  This sets the semadj value in this child process so that 
		 *  the semaphore is adjusted properly upon process exit.
	         *  This is necessary because the parent process may die and
	         *  adjust the semaphore.  We must have our count in place if
	         *  that happens.  There is no race condition here between the
	         *  parent adjusting the semaphore and our wait, since no
	         *  mail job has been opened yet.
	 	 *
		 *  All semop failures cause syserr.  It is assumed that once
		 *  a semop fails, all subsequent semops fail.  This means
		 *  that orderq can't clean the queue.
	         */
#ifdef DEBUG
		if (tTd (40, 111))		/* sema4 test */
		    exit (99);
#endif DEBUG
	        (void) semwait (Sid, 1, 0, 0);
#ifdef DEBUG
		if (tTd (40, 110))		/* sema4 test */
		    exit (99);
#endif DEBUG
	    }
	}

	if (i == 0)
	{
	    /*
	     *  (PSEUDO)CHILD
	     *	Lock the control file to avoid duplicate deliveries.
	     *		Then run the file as though we had just read it.
	     *	We save an idea of the temporary name so we
	     *		can recover on interrupt (what?).
	     */

	    /* set basic modes, etc. */
	    /* stopping events unnecessary since none are ours */
	    stopevents ();		/* stop event queue		*/
	    clearenvelope(CurEnv, FALSE);
	    QueueRun = TRUE;
	    ErrorMode = EM_MAIL;
	    CurEnv->e_id = &w->w_name[2];
# ifdef LOG
	    if (LogLevel > 11)
	    	syslog(LOG_DEBUG, MSGSTR(QU_DOWORK, "%s: dowork, pid=%d"), CurEnv->e_id, getpid()); /*MSG*/
# endif LOG

	    /* don't use the headers from sendmail.cf... */
	    CurEnv->e_header = NULL;

	    /* lock the control file during processing */
	    if (link(w->w_name, queuename(CurEnv, 'l')) < 0)
	    {
#ifdef DEBUG
		if (tTd(40, 1))
			(void) printf("dowork: %s, locked\n", w->w_name);
#endif DEBUG
#ifdef LOG
		if (LogLevel > 4)
	    		syslog(LOG_DEBUG, MSGSTR(QU_LOCKED, "%s: locked"), CurEnv->e_id); /*MSG*/
#endif LOG
	    	if (ForkQueueRuns)
	    		exit(EX_OK);
	    	else
		{
			CurEnv->e_id = NULL;	/* logically detach frm qf */
	    		return;
		}
	    }

	    /* do basic system initialization */
	    initsys();

	    readqf(CurEnv, TRUE);
	    CurEnv->e_flags |= EF_INQUEUE;
	    eatheader(CurEnv);

	    /* do the delivery */
#ifdef DEBUG
	    if (tTd (41, 1))
	        (void) printf ("dowork: prepare to sendall\n");
#endif DEBUG   
	    /* kill this test since immediate/fork modes don't use it */
	    /* if (!bitset(EF_FATALERRS, CurEnv->e_flags)) */
	    	sendall(CurEnv, SM_DELIVER);

	    /* finish up and exit */
	    if (ForkQueueRuns)
	    	finis();
	    else
	    	dropenvelope(CurEnv);
	}
	else
	{
	    /*
	     *  Parent -- wait for all children to exit (there's only one).
	     *  The argument one is simply a dummy in the pid numeric range.
	     *  Since SIGCLD is SIG_IGN, no zombies are formed.
	     */
	    (void) waitfor (1);
	    errno = 0;
	}
}
/*
**  READQF -- read queue file and set up environment.
**
**	Parameters:
**		e -- the envelope of the job to run.
**		full -- if set, read in all information.  Otherwise just
**			read in info needed for a queue print.
**
**	Returns:
**		none.	
**
**	Side Effects:
**		cf is read and created as the current job, as though
**		we had been invoked by argument.
*/

readqf(e, full)
	register ENVELOPE *e;
	int full;
{
	char *qf;
	register FILE *qfp;
	char buf[MAXFIELD];
	int gotctluser = 0;
	int fd;

	/*
	**  Read and process the file.
	*/

	qf = queuename(e, 'q');
	qfp = fopen(qf, "r");
	if (qfp == NULL)
	{
		syserr(MSGSTR(QU_ECONTROL, "readqf: no control file %s"), qf); /*MSG*/
		return;
	}
	FileName = qf;
	LineNumber = 0;
	if (Verbose && full)
		(void) printf(MSGSTR(QU_RUN, "\nRunning %s\n"), e->e_id); /*MSG*/
	while (fgetfolded(buf, sizeof buf, qfp) != NULL)
	{
# ifdef DEBUG
		if (tTd(40, 4))
			(void) printf("+++++ %s\n", buf);
# endif DEBUG
		switch (buf[0])
		{
		  case 'C':		/* specify controlling user */
			setctluser(&buf[1]);
			gotctluser = 1;
			break;

		  case 'R':		/* specify recipient */
			sendtolist(&buf[1], (ADDRESS *) NULL, &e->e_sendqueue);
			break;

		  case 'E':		/* specify error recipient */
			sendtolist(&buf[1], (ADDRESS *) NULL, &e->e_errorqueue);
			break;

		  case 'H':		/* header */
			if (full)
				(void) chompheader(&buf[1], FALSE);
			break;

		  case 'M':		/* message */
			e->e_message = newstr(&buf[1]);
			break;

		  case 'S':		/* sender */
			setsender(newstr(&buf[1]));
			break;

		  case 'D':		/* data file name */
			if (!full)
				break;
			e->e_df = newstr(&buf[1]);
			e->e_dfp = fopen(e->e_df, "r");
			if (e->e_dfp == NULL)
				syserr(MSGSTR(QU_EOPEN3, "readqf: cannot open %s"), e->e_df); /*MSG*/
			break;

		  case 'T':		/* init time */
			e->e_ctime = atol(&buf[1]);
			break;

		  case 'P':		/* message priority */
			e->e_msgpriority = atol(&buf[1]) + WkTimeFact;
			break;

		  case 'B':		/* body type */
			if (buf[1] == 'e')
			    e->e_btype = BT_ESC;
			else if (buf[1] == 'n')
			    e->e_btype = BT_NLS;
			else if (buf[1] == 'j')
			    e->e_btype = BT_NC;
			else if (buf[1] == 's')
			    e->e_btype = BT_MC;

		  case '\0':		/* blank line; ignore */
			break;

		  default:
			syserr(MSGSTR(QU_ELINE, "readqf(%s:%d): bad line \"%s\""), e->e_id, LineNumber, buf); /*MSG*/
			break;
		}
		/*  The 'C' queue file command operates on the next line,
		**  so we use 'gotctluser' to maintain state as follows:
		**	0 - no controlling user,
		**	1 - controlling user has been set but not used,
		**	2 - controlling user must be used on next iteration.
		*/
		if (gotctluser == 1)
			gotctluser++;
		else if (gotctluser == 2)
		{
			clrctluser();
			gotctluser = 0;
		}
	}
	/* clear controlling user in case we break out prematurely */
	clrctluser();

	(void) fclose(qfp);
	FileName = NULL;

	/*
	**  If we haven't read any lines, this queue file is empty.
	**  Arrange to remove it without referencing any null pointers.
	*/

	if (LineNumber == 0)
	{
		errno = 0;
		e->e_flags |= EF_CLRQUEUE | EF_FATALERRS | EF_RESPONSE;
	}
}
/*
**  PRINTQUEUE -- print out a representation of the mail queue
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Prints a listing of the mail queue on the standard output.
*/

printqueue()
{
	register WORK *w;
	FILE *f;
	int nrequests;
	char buf[MAXLINE];

	/*
	**  Read and order the queue.
	*/

	nrequests = orderq(TRUE);

	/*
	**  Print the work list that we have read.
	*/

	/* first see if there is anything */
	if (nrequests <= 0)
	{
		(void) printf(MSGSTR(QU_EMPTY, "Mail queue is empty\n")); /*MSG*/
		return;
	}

	if (nrequests == 1)
	    (void) printf(MSGSTR(QU_REQ, "\t\tMail Queue (1 request")); /*MSG*/
	else
	    (void) printf(MSGSTR(QU_REQS, "\t\tMail Queue (%d requests"), nrequests); /*MSG*/
	if (nrequests > QUEUESIZE)
		(void) printf(MSGSTR(QU_PRINT, ", only %d printed"), QUEUESIZE); /*MSG*/
	if (Verbose)
	    (void) printf(MSGSTR(QU_HDR, ")\n---QID---- --Size-- -Priority- ---Q-Time--- ----------Sender/Recipient----------\n")); /*MSG*/
	else
	    (void) printf(MSGSTR(QU_HDR2, ")\n---QID---- --Size-- -----Q-Time----- --------------Sender/Recipient-------------\n")); /*MSG*/
	for (w = WorkQ; w != NULL; w = w->w_next)
	{
		struct stat st;
		long  submittime = 0;
		long dfsize = -1;
		char lf[20];
		char line[MAXLINE];
		char cbuf[MAXLINE];

		f = fopen(w->w_name, "r");
		if (f == NULL)
		{
			errno = 0;
			continue;
		}
		(void) printf("%-10s", w->w_name + 2);
		(void) strcpy(lf, w->w_name);
		lf[0] = 'l';
		if (stat(lf, &st) >= 0)
			(void) printf("*");
		else if (shouldqueue(w->w_pri))
			(void) printf("X");
		else
			(void) printf(" ");
		errno = 0;
		
		line[0] = '\0';
		cbuf[0] = '\0';
		while (fgets(buf, sizeof buf, f) != NULL)
		{
			fixcrlf(buf, TRUE);
			switch (buf[0])
			{
			  case 'M':	/* error message */
				(void) strcpy(line, &buf[1]);
				break;

			  case 'S':	/* sender name */
				if (Verbose)
				    (void) printf ("%8ld %10ld %.12s %.36s",
				       dfsize, w->w_pri, ctime(&submittime) + 4,
					 &buf[1]);
				else
				    (void) printf ("%8ld %.16s %.43s", dfsize,
					         ctime(&submittime), &buf[1]);
				if (line[0] != '\0')  /* error: indent it */
				    (void) printf ("\n%*s(%.59s)",
					11, "", line);
				break;

			  case 'C':	/* controlling user */
				if (strlen(buf) < MAXLINE - 3)  /* sanity */
					(void) strcat(buf, ") ");
				cbuf[0] = cbuf[1] = '(';
				(void) strncpy(&cbuf[2], &buf[1], MAXLINE-1);
				cbuf[MAXLINE-1] = '\0';
				break;

			  case 'R':	/* recipient name: indent and print */
				if (cbuf[0] != '\0')
				{
					/* prepend controlling user to 'buf' */
					(void) strncat(cbuf, &buf[1],     
							MAXLINE-strlen(cbuf));
					cbuf[MAXLINE-1] = '\0';
					(void) strcpy(buf, cbuf);
					cbuf[0] = '\0';
				}
				if (Verbose)
				  (void) printf ("\n%*s%.36s", 44, "", &buf[1]);
				else
				  (void) printf ("\n%*s%.43s", 37, "", &buf[1]);
				break;

			  case 'T':	/* creation time */
				submittime = atol(&buf[1]);
				break;

			  case 'D':	/* data file name */
				if (stat(&buf[1], &st) >= 0)
					dfsize = st.st_size;
				break;
			}
		}
		if (submittime == (long) 0)
		    (void) printf (MSGSTR(QU_ECONTR, " (no control file)")); /*MSG*/
		(void) printf ("\n");
		(void) fclose(f);
	}
}

/*
**  QUEUENAME -- build a file name in the queue directory for this envelope.
**
**	Assigns an id code if one does not already exist.
**	This code is very careful to avoid trashing existing files
**	under any circumstances.
**
**	Parameters:
**		e -- envelope to build it in/from.
**		type -- the file type, used as the first character
**			of the file name.
**
**	Returns:
**		a pointer to the new file name (in a static buffer).
**
**	Side Effects:
**		Will create the qf files if no id code is
**		already assigned.  This will cause the envelope
**		to be modified.
*/

char *
queuename(e, type)
	register ENVELOPE *e;
	char type;
{
	static char buf[MAXNAME];
	static int pid = -1;
	char c1 = 'A';
	char c2 = 'A';

	if (e->e_id == NULL)
	{
		char qf[20];
		char lf[20];

		/* find a unique id */
		if (pid != getpid())
		{
			/* new process -- start back at "AA" */
			pid = getpid();
			c1 = 'A';
			c2 = 'A' - 1;
		}
		(void) sprintf(qf, "qfAA%05d", pid);
		(void) strcpy(lf, qf);
		lf[0] = 'l';

		while (c1 < '~' || c2 < 'Z')
		{
			int i;

			if (c2 >= 'Z')
			{
				c1++;
				c2 = 'A' - 1;
			}
			lf[2] = qf[2] = c1;
			lf[3] = qf[3] = ++c2;
# ifdef DEBUG
			if (tTd(7, 20))
			    (void) printf("queuename: trying \"%s\"\n", qf);
# endif DEBUG

			/*
			 *  Test for existence of either of the pair.
			 *  No other active invocation of sendmail has our
			 *  pid, so any true access refers to a queue file
			 *  created by a previous invocation of sendmail
			 *  using the same pid.  No one else can be
			 *  attempting to create links using our name,
			 *  since the name contains the pid.
			 */
			if (access(lf, F_OK) >= 0 || access(qf, F_OK) >= 0)
				continue;
			i = creat(lf, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
			if (i < 0)
			{
				i = errno;	/* save error */
				(void) unlink(lf);	/* kernel bug */
				if (i == ENOSPC)
				{
					syserr(MSGSTR(QU_ECREAT, "queuename: Cannot create \"%s\" in \"%s\""), lf, QueueDir); /*MSG*/
					exit(EX_UNAVAILABLE);
				}
				continue;
			}
			(void) close(i);	/* we don't need it open */

			/*
			 *  Link qf to the same inode and we are through.
			 */
			if (link(lf, qf) >= 0)
				break;
			
			/*
			 *  Link failed. Clean up remaining file and try
			 * the next one.
			 */
			(void) unlink(lf);
		}

		errno = 0;
		if (c1 >= '~' && c2 >= 'Z')
		{
			syserr(MSGSTR(QU_ECREAT, "queuename: Cannot create \"%s\" in \"%s\""), qf, QueueDir); /*MSG*/
			exit(EX_OSERR);
		}
		e->e_id = newstr(&qf[2]);
		define('i', e->e_id, e);
# ifdef DEBUG
		if (tTd(7, 1))
		    (void) printf("queuename: assigned id %s, env=%x\n", 
								e->e_id, e);
# ifdef LOG
		if (LogLevel > 16)
			syslog(LOG_DEBUG, "%s: assigned id", e->e_id);
# endif LOG
# endif DEBUG
	}

	if (type == '\0')
		return (NULL);
	(void) sprintf(buf, "%cf%s", type, e->e_id);
# ifdef DEBUG
	if (tTd(7, 2))
		(void) printf("queuename: %s\n", buf);
# endif DEBUG
	return (buf);
}
/*
**  UNLOCKQUEUE -- unlock the queue entry for a specified envelope
**
**	Parameters:
**		e -- the envelope to unlock.
**
**	Returns:
**		none
**
**	Side Effects:
**		unlocks the queue for `e'.
*/

unlockqueue(e)
	ENVELOPE *e;
{
	/* remove the transcript */
#ifdef DEBUG
# ifdef LOG
	if (LogLevel > 19)
		syslog(LOG_DEBUG, "%s: unlock", e->e_id);
# endif LOG
	if (!tTd(51, 4))
#endif DEBUG
		xunlink(queuename(e, 'x'));

	/* last but not least, remove the lock */
	xunlink(queuename(e, 'l'));
}
/*
**  GETCTLUSER -- return controlling user if mailing to prog or file
**
**	Check for a "|" or "/" at the beginning of the address. If
**	found, return a controlling username.
**
**	Parameters:	
**		a - the address to check out
**
**	Returns:
**		Either NULL, if we weren't mailing to a program or file,
**		or a controlling user name (possibly in getpwuid's
**		static buffer).
**
**	Side Effects:
**		none.
*/

char *
getctluser(a)
	ADDRESS *a;
{
	extern ADDRESS *getctladdr();
	struct passwd *pw;
	char *retstr = NULL;

	/*
	**  Get unquoted user for files, program or user.name check.
	**  N.B. remove this code block to always emit controlling
	**  addresses (at the expense of backward compatibility).
	*/

	{
		char buf[MAXNAME];
		if (a->q_alias == NULL && !bitset(QGOODUID, a->q_flags))
			return((char *)NULL);
		(void) strncpy(buf, a->q_paddr, MAXNAME);
		buf[MAXNAME-1] = '\0';
		stripquotes(buf, TRUE);

		if (buf[0] != '|' && buf[0] != '/')
			return ((char *)NULL);
	}

	a = getctladdr(a);	/* find controlling address */
	if (a != NULL && a->q_uid != 0 && (pw = getpwuid(a->q_uid)) != NULL)
		retstr = pw->pw_name;
	else			/* use default user */
		retstr = DefUser;

# ifdef DEBUG
	if (tTd(40, 5))
		printf("Set controlling user for '%s' to '%s'\n", 
			(a == NULL) ? "<null>": a->q_paddr, retstr);
# endif DEBUG

	return(retstr);
}
/*
**  SETCTLUSER - sets 'CtlUser' to controlling user
**  CLRCTLUSER - clears controlling user (no params, nothing returned)
**
**	These routines manipulate 'CtlUser'
**
**	Parameters:
**		str - controlling user as passed to setctluser()
**
**	Returns:
**		None.
**
**	Side Effects:
**		'CtlUser' is changed
*/

static char CtlUser[MAXNAME];

setctluser(str)
	register char *str;
{
	(void) strncpy(CtlUser, str, MAXNAME);
	CtlUser[MAXNAME-1] = '\0';
}

clrctluser()
{
	CtlUser[0] = '\0';
}
/*
**  SETCTLADDR -- create a controlling address
**
**	If global variable 'CtlUser' is set and we are given a valid
**	address, make that address a controlling address; change the
**	'q_uid', 'q_gid', and 'q_ruser' fields and set QGOODUID.
**
**	Parameters:
**		a - address for which control uid/gid info may apply
**
**	Returns:
**		None.
**
**	Side Effects:
**		Fills in uid/gid fields in address and sets QGOODUID
**		flag if appropriate.
*/

setctladdr(a)
	ADDRESS *a;
{
	struct passwd *pw;

	/*
	**  If there is no current controlling user, or we were passed a
	**  NULL addr ptr or we already have a controlling user, return.
	*/

	if (CtlUser[0] == '\0' || a == NULL || a->q_ruser)
		return;

	/*
	**  Set up addr fields for controlling user. If 'CtlUser' is no
	**  longer valid, use the default user/group.
	*/

	if ((pw = _getpwnam_shadow(CtlUser,0)) != NULL)
	{
		if (a->q_home)
			free(a->q_home);
		a->q_home = newstr(pw->pw_dir);
		a->q_uid = pw->pw_uid;
		a->q_gid = pw->pw_gid;
		a->q_ruser = newstr(CtlUser);
	}
	else
	{
		a->q_uid = DefUid;
		a->q_gid = DefGid;
		a->q_ruser = newstr(DefUser);
	}
	
	a->q_flags |= QGOODUID;		/* flag as a "ctladdr" */

# ifdef DEBUG
	if (tTd(40, 5))
	    (void) printf("Restored controlling user for '%s' to '%s'\n",
				a->q_paddr, a->q_ruser);
# endif DEBUG

}
	
