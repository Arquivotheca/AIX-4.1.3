static char sccsid[] = "@(#)37	1.49  src/bos/usr/bin/que/qmain.c, cmdque, bos411, 9428A410j 3/10/94 11:06:16";
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

/*
 * queuing daemon -- main routines
 * others are in
 *      start.c         start up a job
 *      finish.c        clean up once it has finished
 *      status.c        manipulate status files
 *      exec.c          run a backend
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <IN/standard.h>
#include <signal.h>
#include <sys/stat.h>
#include <IN/backend.h>
#include <fcntl.h>
#include <locale.h>
#include "common.h"
#include <termio.h>
#include <sys/access.h>
#include <sys/audit.h>
#include <sys/id.h>
#include <security.h>
#include <ctype.h>
#include <sys/wait.h>
/* Message Queue stuff */
#include <sys/ipc.h>
#include <sys/msg.h>

#include "qmain_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QMAIN,num,str)



extern  boolean qdaemon;	/* GLOBAL CONTROL FLAG IN COMMON.C */
extern  boolean qkillfile;	/* GLOBAL CONTROL FLAG IN COMMON.C */

extern void resetQCBFile();

char nulldev[] = "/dev/null";

#ifdef DEBUG
int debug = 0;
#endif

char * progname="qdaemon";
int nchildren = 0;                      /* number of children alive */
int slowdeath = FALSE;                  /* die gracefully */
char signaldeath = FALSE;               /* die gracefully */
int reread = FALSE;                     /* reread config file */
char *myname;                           /* so can change name for ps and */
char *ruser;                            /* user who enqueued -rr request */
long renq_nid;                          /* -rr enqueued nid              */
					/* ss after fork */

int readid;                             /*    for the message queue */

int gotalarm(void), gotrequest(void);   /* functions to catch signals */

int alarmReExec(void);
int setSlowDeath(void);

struct e *findentry();

static char *memBot;
/*
 * Setting the slowdeath flag makes qdaemon quit after it has finished
 * it's current jobs.
 */
int
setSlowDeath(void)
{
	signaldeath = slowdeath = TRUE;
}

/*
 * the main loop.  initialize, then start up jobs as requests come in,
 * and wait for them to finish when we've got nothing else to do.
 */

main(argc,argv)
int  argc;
char **argv;
{       struct q *qlist;       	/*----List of queues, devices, jobs */
        struct d *dev;		/*----Device that process is running on */
	int i;
	int specreq();		/*----Routine to handle special requests */
        int status;             /*----Status of child process termination */
        int pid;		/*----Process ID of terminated process */
	int messages;

	/*----Set global control variables */
	qdaemon = TRUE;

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
        catd = catopen(MF_QMAIN, NL_CAT_LOCALE);

	/*----Auditing */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);	

/*	Change process group leader to be init,
	and disassociates from /dev/console 		*/
	i = open("/dev/tty", O_RDWR);
        if (i >= 0) {
                ioctl(i, (int) TIOCNOTTY, (char *)0);
                close(i);
        }

 
#ifdef DEBUG
	if (argc > 1)
	    debug = 1;
#endif

	unixinit(argc,argv);
	init();

	resetQCBFile();
	qlist = readconfig(1);          /* get queues and devices */
	qdkill(qlist);                  /* kill all old ones */
	initstat(qlist);                /* get initial status files */
	signal(SIGUSR1, (void (*)(int))setSlowDeath);
	signal(SIGQUEUE, (void (*)(int))gotrequest); /* to catch geese */

	memBot = (char *)sbrk(0);  /* so we can later compute our memory use. */

        /** delete any old left over message queue */
        if ((readid = msgget(ENQ_QUE, 0)) >= 0)
           msgctl(readid, IPC_RMID, (struct msqid_ds *) 0);

        readqdir(NULL,qlist,specreq);

        /* D45466, set message queue, to receive requests */
        if ((readid = msgget(ENQ_QUE, PERMS | IPC_CREAT)) < 0)
           syserr((int)EXITFATAL,MSGSTR(MSGGETERR,"Unable to create message queue.  Errno = %d."),errno);

	while (1)
	{
		errno = 0;
                messages = read_msg(qlist,IPC_NOWAIT);   /* read messages off the queue */

		/*----Get all possible devices busy */
		workdevices(qlist);

		/*----Check for children to die, if any */
		if (nchildren)
		{
			if((pid=waitpid(ANYPID,&status,WNOHANG)) == 0){
				if(messages != -1)
					messages = read_msg(qlist,IPC_NOWAIT);
				else 
					pid = waitpid(ANYPID,&status,0); 
			}

			alarm(0);
			/*----Get the device, if any, associated with this child, and cleanup job */
			if ((pid == -1) && (errno == ECHILD)) {
			    nchildren = 0;
			    syswarn(MSGSTR(MSGCHLD,"Problem in main queueing loop, Errno = %d."), errno);
			    continue;	/* this applies to the outer while loop */
			}
			while ((pid != -1) && (pid != 0)) {
			    int readdone = 0;
			    if ((dev = (struct d *)devpid(qlist,pid)) != NULL)
			    {
				    nchildren--;
				    /* 
				     * devfinish() needs to be called after the if
				     * due to the fact that it updates the dev
				     * structure. It should also be called before
				     * readqdir.
				     */

				    if (!readdone && (dev->d_q->q_top == TRUE) &&
					(dev->d_q->q_entcount < LOCOUNT)){
					    dev->d_q->q_top = FALSE;
					    devfinish(dev,status);
					    readdone=1;
					    readqdir(NULL, qlist, specreq);
				    }
				    else
					    devfinish(dev,status);
			    }
			    pid = waitpid(ANYPID,&status,WNOHANG);
			}

			if ((pid == -1) && (errno != ECHILD)) /* child didn't die */
			{
				switch(errno)
				{
				/*----Wait system call interrupted ( by goose() ) */
				case EINTR:
					continue;	/* this applies to the outer while loop */
				/*----All child problems go here */
				default:
					syswarn(MSGSTR(MSGCHLD,"Problem in main queueing loop, Errno = %d."), errno);
					continue;	/* this applies to the outer while loop */
				}
			}
		}
		else
		{
			/*----No children processes on devices */
			if (slowdeath)
			{
				/*----Shutdown and reread /etc/qconfig */
				if (reread)
				{
					/*----Last official duty: force */
					/*    re-digestion to qconfig.bin */
					/*    ignore requests signals for exec */
					signal(SIGQUEUE,SIG_IGN);
					execl(QDPROG, "qdaemon",
#ifdef DEBUG
					      debug? "debug": 0,
#endif
					      0);
					syserr((int)EXITEXEC,MSGSTR(MSGREXE,"Unable to re-exec myself."));
				}

				/*----Signalled to die */
				if (signaldeath)
				{
					systell(MSGSTR(MSGTERS,"Terminated due to signal."));
					qexit1((int)EXITSIGNAL);
				}

				/*----Die gracefully */
				systell(MSGSTR(MSGTERN,"Terminated normally."));
				qexit1((int)EXITOK);
			}
			/*----No children, so just cleanup the queues,
			      go to sleep,
			      and wait for goose signal */
			cleanupq(qlist);
                        signal(SIGALRM,(void (*)(int))alarmReExec);
                        alarm(1800);
 			read_msg(qlist,0);    /** wait for a request **/
                        alarm(0);
                        signal(SIGALRM,(void (*)(int))gotalarm);
		}
	}
}



/* read messages off the message queue */
read_msg(qlist,mode)
struct q *qlist;
int mode;        /* wait or not to wait */
{
       int count;              /*    max. no. of messages read at one time */
       Message msg;            /*    message structure     */
       int moremsg;

       count =  MESS_COUNT;
       errno = 0;


       while (count--)     /* process at the most MESS_COUNT messages in one shot */
       {
          /* wait for/read a request to do something */
           if ((moremsg=msgrcv(readid,&msg,sizeof(msg.jdfname),0L,mode | MSG_NOERROR)) == -1)
              if (mode || (errno == EINTR)){
                 break;
	      }
              else
                 syserr((int)EXITFATAL,MSGSTR(MSGRCVERR,"Error in message receive.  Errno = %d."),errno);

           /* process the JDF received */
           if ((msg.mtype == SPECIAL) && qdaemon && specreq)
              specreq(msg.jdfname,qlist);
           else   /* right now NORMAL is the only other request  but it won't hurt to check */
              if (msg.mtype == NORMAL)
                 queueit(msg.jdfname,qlist,TRUE);
           mode = IPC_NOWAIT;  /* if there is additional message at this time, then process it */

      }

      /* better reset these as these may cause error messages in other func */
      if ((errno == ENOMSG) || (errno == EINTR))
         errno = 0;
      return(moremsg); 
}
/*** read_msg() ends ***/


/* do some once-only initialization */
unixinit(ac,av)
int  ac;
char **av;
{
	int pid;

#ifdef NOSRC  /* for SRC control we need to run in the "foreground" */
#ifndef DEBUG
	/* return, in case we haven't been run with "&" */
        pid = fork();
        if (pid == -1)
                /*----ERROR: send message */
                syserr((int)EXITFATAL,MSGSTR(MSGFORK,"Unable to fork a new process."));

	if (pid != 0)
	    qexit1((int)EXITOK);               /* (parent exits) */
#endif
#endif
	myname = *av;
	freopen(nulldev,"w",stdout);
#ifndef DEBUG
	freopen(nulldev,"w",stderr);
#endif
}

init()
{
	freopen(nulldev,"r",stdin);

#ifndef DEBUG
	signal(SIGINT,SIG_IGN);         /* ignore interrupts */
#endif
	signal(SIGQUIT,SIG_IGN);        /* and quits */
	signal(SIGHUP,SIG_IGN);         /* and hangups */

	signal(SIGALRM,(void (*)(int))gotalarm);       /* handle alarms */

#ifdef DEBUG
	if (getenv("INITQM"))
		sysraw("QUEDIR = %s\n", QUEDIR);
#endif
	cd(QUEDIR);
	writepid();                     /* leave pid for goosability */
	qkillfile = TRUE;
}


/*
 * catch alarm.  don't care about recording it; just want wait()
 * or pause() to fail, which happens automatically
 */
int
gotalarm(void)
{
	signal(SIGALRM,(void (*)(int))gotalarm);
}

int
alarmReExec(void)
{
	/*
	 * If we get an ALARM while paused & we are using more than
	 * 1 meg of memory we exec ourself so that memory will be
	 * returned to the system.
	 */
        if ( ((char *)sbrk(0) - memBot) > 1048576 ) {
                execlp("/usr/sbin/qdaemon", "/usr/sbin/qdaemon", NULL);
        }
        return 0;
}


/*
 * leave my pid in QDPID so print can find it and signal requests
 * if old pid there, assume there's another qdaemon and kill it.
 */
writepid()
{       int mypid = getpid();
        register FILE *pidfil;
	int pid;

	if ((pidfil = fopen(QDPID,"r")) != NULL)
	{   if (fscanf(pidfil,PID_FMT,&pid) == 1) 
	    {
		if (pid != mypid) {
			kill (pid, SIGKILL);
			unlink(QDPID);
			pidput(mypid,QDPID);
		}
	     }
	     else
	     {
		unlink(QDPID);
		pidput(mypid,QDPID);
	     }
	     fclose(pidfil);
	}
	else
	     pidput(mypid,QDPID);
}




/*
 * take care of special request -- device up or down, or qdaemon suicide,
 * a cancel a job, alter its priority, move a job, or hold or release a job.
 */
specreq(fnam,ql)
register struct q *ql;
char *fnam;
{       char line1[QELINE], line2[QELINE], line3[QELINE], line4[QELINE];
	char *qname;
	char *dname;
	register struct d *dl;
	register FILE *spfile;

	if ((spfile = fopen(fnam,"r")) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGSOPN,"Error opening %s.  Errno = %d"),fnam,errno);

	if (!getline(line1,QELINE,spfile))
		syserr((int)EXITFATAL,MSGSTR(MSGSRED,"Error reading %s.  Errno = %d"),fnam,errno);

	getline(line2,QELINE,spfile);           /* missing in some cases */
	getline(line3,QELINE,spfile);           /* missing in some cases */
	getline(line4,QELINE,spfile);		/* missing in some cases */

	fclose(spfile);
	unlink(fnam);

#ifdef DEBUG
	if (getenv("SPECREQ"))
	    sysraw("specreq: %s\n",line1);
#endif
	
	switch(line1[0])
	{   case 'r':           /* reread config file */
		reread = TRUE;
		slowdeath = TRUE;
               	sscanf(line3,"%lx",&renq_nid);
		ruser = line2;
		return(0);
	    case 's':           /* suicide */
		if (!slowdeath)
		{	slowdeath = TRUE;
			systell(MSGSTR(MSGSDIE,"Starting to die at request of %s."),line2);
		}
		return(0);
	    case 'x':           /* cancel */
		cancel(line1+1,line2,ql);
		return(0);
	    case 'p':           /* alter priority */
		alter(line1+1,line2,line3,ql);
		return(0);
	    case 'h':		/* hold a job */
		hold(line1+1,line2,ql,1);
		return(0);
	    case 'R':		/* release a job */
		hold(line1+1,line2,ql,0);
		return(0);
	    case 'm':		/* move a job */
		move(line1+1,line2,line3,line4,ql);
		return(0);

	    case 'u':           /* up, down, kill */
	    case 'k':
	    case 'd':
		break;      /* we do this below */
	    default:
		syserr((int)EXITFATAL,MSGSTR(MSGBSPC,"Bad special request switch in file %s."),fnam);
	}

	/* fetch out the queue and device names */

	dname = line1+2;	/* skip reqtypechar and ':' */
	qname=line2;

	/* now find this device */
	for (; ql; ql = ql->q_next)
	{   if (strncmp(ql->q_name,qname,QNAME))
		continue;
	    for (dl = ql->q_devlist; dl; dl = dl->d_next)
	    {   
		if (strcmp(dl->d_name,dname) == 0)
			goto good;
	    }
	}

	/* couldn't find it */
	syswarn(MSGSTR(MSGUNDR,"Special request `%s' not understood"), line1);

	return(0);

good:
	/* now dl is the device requested */
	switch(line1[0])
	{   case 'u':
		/* up.  if no job running, make stfile say READY. */
		/* otherwise, qu -q would say DOWN or OFF until */
		/* someone tried to run a job. */
		dl->d_up = TRUE;
		if (!dl->d_pid)
		    setstat(stname(dl),READY);
		break;
	    case 'd':
	    case 'k':
		/* off. if no job running, make stfile say OFF. */
		/* if job, kill it if "-dk"; just wait for it */
		/* to die otherwise. */
		dl->d_up = FALSE;
		if (!dl->d_pid)
		    setstat(stname(dl),OFF);
		else
		{   if (line1[0] == 'k')
			kill((0 - dl->d_pid),SIGTERM);
		}
		break;
	}
}


/*
 * return name of pid file for device dev.  name is pidqueue:dev
 * assume we're in qdir, which makes name "../stat/p.QUEUE.DEVICE
 */
pidname(dev)
register struct d *dev;
{       static char name[MAXPATHLEN];

	sprintf(name,"../stat/p.%s.%s",dev->d_q->q_name,dev->d_name);
	return((int)name);
}


/* kill the process whose pid might be in name */
fkill(name, mypid)
register char *name;
{       register FILE *pidfil;
	int pid;

	if ((pidfil = fopen(name,"r")) != NULL)
	{   if (fscanf(pidfil,PID_FMT,&pid) == 1)
	    {   /* if we're responding to "-rr", we'll be in the */
		/* same process slot. */
		if (pid != mypid)
		    kill(pid,SIGKILL);
	    }
	    fclose(pidfil);
	    unlink(name);
	}
}


/* put pid in file named name */
pidput(pid,name)
register char *name;
{       register FILE *pidfil;
	gid_t	effgid;		/* to use for chown() */
	char buf[MAXLINE];

	if ((pidfil = fopen(name,"w")) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGSOPN,"Error opening %s.  Errno = %d"), name,errno);

	/* get the effective gid to chown() */
	if ((effgid=getgidx(ID_EFFECTIVE)) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGEGID,"Cannot get effective gid. Errno=%d"),errno);
		
	/* set ownership to administrative user (root) and
	 * effective gid (printq)
	 */
	if (chown(name, OWNER_ADMDATA, effgid) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGCHWN,"Cannot chown %s. Errno=%d"),name,errno);

	/* set permissions to owner/group rw only	*/
	if (acl_set(name, R_ACC|W_ACC, R_ACC|W_ACC, NO_ACC) == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGPERM,"Could not set permissions on %s. Errno = %d"),errno);

	fprintf(pidfil,PID_FMT,pid);
	putc('\n',pidfil);
	if(fclose(pidfil) == EOF) {
		sprintf(buf,MSGSTR(MSGSWR2,"Error writing to %s.\nErrno = %d: %s"),
			name, errno, strerror(errno));
		syserr((int)EXITFATAL,buf);
	}
}


/*
 * kill all old backends that might still be around
 * might be better to scan stat dir for s* files
 */
qdkill(ql)
register struct q *ql;
{       register struct d *dl;

	for (; ql; ql = ql->q_next)
	{   for (dl = ql->q_devlist; dl; dl = dl->d_next)
		fkill(pidname(dl),0);
	}
}


extern int childterm;
/*
 * cancel the job whose name is ename;
 * if in qlist
 *      if running, kill it and wait for it later
 *      else (not running,) edelete
 * else (not in qlist,) but make sure time matches and then delete ename
 */
cancel(ename,jobnumstring,ql)
struct q *ql;
char *jobnumstring;
register char *ename;
{       
	register struct e *el;
	register struct d *d;
	int jnum;
	char sbuf[1024];
	struct q *tmpq;

	tmpq = ql;	/* save the top of the qlist in case we need to
			 * read qdir
			 */

#ifdef DEBUG
	if (getenv("CANCEL"))
	    sysraw("cancel %s\n",ename);
#endif

	jnum=checkjobnum(jobnumstring);
/* new jobs may have job number -1*/
	if((jnum == -1) && strcmp(jobnumstring,"-01"))
		syserr((int)EXITFATAL,MSGSTR(MSGBJOB,"Job number (%s) out of range in (cancel())."),jobnumstring);

	/* find the entry in question */
	if (el = findentry(ename,jnum,&ql))
	{   /* got it.  running? */
	    if ((d = el->e_device) != NULL)     /* yep */
	    {   if (d->d_pid == 0)          
		    return(0);
		kill( (0 - d->d_pid) ,SIGTERM);
		sprintf(sbuf,MSGSTR(MSGDELETE,"Job number %d has been deleted from the queue."),el->e_jobnum);
		sysnot(el->e_from, NULL, sbuf, el->e_mail ? DOMAIL : DOWRITE);

		return(0);
	    }
	    else
	    {
#ifdef DEBUG
		if(getenv("CANCEL"))
			sysraw("in cancel  \n");
#endif

		sprintf(sbuf,MSGSTR(MSGDELETE,"Job number %d has been deleted from the queue."),el->e_jobnum);
		sysnot(el->e_from, NULL, sbuf, el->e_mail ? DOMAIL : DOWRITE);
		childterm = SIGTERM; /* used for information only on rmlist() */
		edelete(el, ql );
                systell(MSGSTR(MSGDELETE,"Job number %d has been deleted from the queue."),el->e_jobnum);
		if ((ql->q_top == TRUE) && (ql->q_entcount < LOCOUNT)) {
			ql->q_top = FALSE;
			freeql(tmpq);
			readqdir (NULL, tmpq, specreq);
		}
		return(0);
	    }
	}
	else
	{
	/* couldn't find it, so it must have finished, or 
		if remote, have been sent across the network.*/
	
		return(0);                     /* must have finished it */
	}
}


/*
 * alter the priority of entry ename.  timstr is its mod date (to prevent
 * race conditions), pristr is its new priority.
 */


alter(ename,pristr,jobnumstr,ql)
char *ename, *pristr, *jobnumstr;
struct q *ql;
{
	register struct e *e;
	register struct q *qlp;
	int jnum;
	struct stat statb;
	char * en;
	char * jdfnewpri();

#ifdef DEBUG
	if (getenv("ALTER"))
	    sysraw("alter() pri of %s to %s\n",ename,pristr);
#endif

	if((-1)==(jnum=checkjobnum(jobnumstr)))
		syserr((int)EXITFATAL,MSGSTR(MSGBALT,"Job number (%s) out of range in (alter())."),jobnumstr);

	/* find the current entry */
	if (e = findentry(ename,jnum,&ql))
	{   /* we found it */
	    if (e->e_device != NULL)       /* too late -- running */
		return(0);
	} else
	{
#ifdef DEBUG
	if (getenv("ALTER"))
		sysraw("%s not queued\n", ename);
#endif
	    /* not there.  we probably haven't read it yet. */
	    if (stat(ename,&statb) == -1 )
		return(0);         /* done? */
	}

	/* get it out of the queue */
	edel1(e,ql);
	e->e_pri1 = atoi(pristr);

	/* now open the file and change the priority record */
	if (en=jdfnewpri(ename,e->e_pri1))
		syswarn(MSGSTR(MSGEPRI,"Could not change job %s to new priority %d because %s"),
			ename,e->e_pri1,en);

	/* requeue the entry */
	for (qlp = ql; qlp; qlp = qlp->q_next)
	{
		if (strncmp(getqn(ename),qlp->q_name,QNAME) == 0 )
		{
			qadd(qlp,e);
		}
	}
}

hold(ename, jobnumstr, ql, holdrel)
char *ename;
char *jobnumstr;
struct q *ql;
int  holdrel;
{
	register struct e *el;
	struct stat statb;
	int jnum;
	char *en;
	char * jdfnewhol();


	jnum=checkjobnum(jobnumstr);
	if (jnum == -1)
		syserr((int)EXITFATAL,MSGSTR(MSGBALT,"Job number (%s) out of range in (hold())."), jobnumstr);

	/* find current entry */
	if (el = findentry(ename,jnum,&ql)){
	
		/* if job entry is already what users wants to change it to 
	   	get out now */
		if (el->e_hold == holdrel)
			return(0);
		
		if (el->e_device != NULL)
			syswarn(MSGSTR(MSGRUNH,"Job already running, unable to hold job number %d."),jnum);  
		else {

			/* get it out of the queue */
			edel1(el,ql);
			el->e_hold = holdrel;

			/* now open and change the hold/release record */
			if(en = jdfnewhol(ename,el->e_hold))		
				syswarn(MSGSTR(MSGEHOL,"Could not change job %s hold/release status."), ename);

			/* add the entry back to the queue */
			qadd(ql,el);
		}
	} else 
		syserr((int)EXITFATAL,MSGSTR(MSGNOENT,"Could not find job %d in the queue"),jnum);
} 

/* function to move a queued job from one queue to another */
move(ename,jobnumstr,newqueue,newdev,ql)
register char *ename;
char *jobnumstr, *newqueue, *newdev;
struct q *ql;
{

	register struct e *el;
	register struct q *qlp;
	int jnum;
	char qnam[MAXPATHLEN];
	char *en;
	char *jdfnewtime();
	long newtime;
	
 
	qlp = ql;
	jnum=checkjobnum(jobnumstr);
	if((jnum == -1) && strcmp(jobnumstr,"-01"))
		syserr((int)EXITFATAL,MSGSTR(MSGBJOB,"Job number (%s) out of range in (move())."),jobnumstr);

	/* find the entry we want */
	if(el = findentry(ename,jnum,&ql))
	{
		if((el->e_device) != NULL)
		{
			syswarn(MSGSTR(MSGARUN,"Job %d is already running unable to move the job."),jnum);
		} else {
			sprintf(qnam,"qe%s:%s",getln(ename),newqueue);
			qentry(ename,qnam);
			edel1(el,ql);
			newtime = time(0);
			strncpy(el->e_dnam,newdev,DNAME);
			if(en=jdfnewtime(qnam,el->e_dnam,newtime))
				syswarn(MSGSTR(MSGTIME,"Could not change job %s to new time %d because %s."),ename,newtime,en);
			queueit(qnam,qlp);
		}
	} else {
		return(0);
	}
}
			
	
	


/*
 * given an entry name and a mod time, return pointer to the entry.
 * return NULL if we can't find it.  used for cancel and alter requests.
 */
struct e * findentry(name,jnum,qlp)
register char *name;
int jnum;
struct q **qlp;
{       register struct e *el;
	register struct q *ql;
	

	for (ql = *qlp; ql; ql = ql->q_next)
	{   for (el = ql->q_entlist; el; el = el->e_next)
            {
			if (strcmp(name,el->e_name) == 0 )
		    	{
				if (jnum == el->e_jobnum)
				{   
					*qlp = ql;
					return(el);
				}
				else	/* stop if ename is rite & jnum is not*/
					return((struct e *)NULL);
			}
	    }
	}
	return((struct e *)NULL);       /* not found */
}

/* run through all the jobs, deleting all invalid requests */
cleanupq(qlist)
register struct q *qlist;
{
	struct q	*ql;
	struct d	*dl;
	struct e	*el;
	boolean		valid_dev;
	char		buf[MAXLINE];

	for (ql = qlist ; ql; ql = ql->q_next) 
		for (el = ql->q_entlist; el; el = el->e_next)
		{
			valid_dev = FALSE;
			if (!strcmp(el->e_dnam,ANYDEV))
				valid_dev = TRUE;
			else
				for (dl = ql->q_devlist; dl; dl = dl->d_next)
					if (!strcmp(dl->d_name,el->e_dnam))
						valid_dev = TRUE;
			if (valid_dev == FALSE)
			{
				sprintf(buf,MSGSTR(MSGBJDEL,"Job %d (%s) with invalid device name has been deleted."), el->e_jobnum, el->e_request);
				sysnot(el->e_from,NULL,buf,el->e_mail ? DOMAIL : DOWRITE);
				edelete(el,ql);
			}
		}
		
}


qexit1(exitcode)
int exitcode;
{
        /* delete the message queue, no need to check for return code
         * as we are exiting anyway */
        msgctl(readid, IPC_RMID, (struct msqid_ds *) 0);

        qexit(exitcode);
}


/*
 * freeql frees the space allocated for the e structures on each
 * q structure on the link list. This is called before readqdir()
 * ( except for the first call from qmain ).
 */
freeql(struct q *ql)
{
	struct q *qtmp=ql;

        /* free all the memory allocated for the
	 * e structures before re-reading the
	 * qdir directory
	 */
	while (qtmp) {
	    struct e *etmp = qtmp->q_entlist;
	    struct e *lastptr = qtmp->q_entlist;

	    while (etmp) {
		lastptr = etmp->e_next;
		if ( etmp->e_device == NULL ) {
		    edel1(etmp, qtmp);
		    jdffree(etmp);
		    free((void *) etmp);
		}
		etmp = lastptr;
	    }
	    qtmp = qtmp->q_next;
	}
}

/*
 * Signal handler for SIGQUEUE signal from enq
 */
gotrequest(void)
{
    signal(SIGQUEUE,(void (*)(int))gotrequest); 
}
