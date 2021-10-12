static char sccsid[] = "@(#)28	1.57  src/bos/usr/bin/que/exec.c, cmdque, bos411, 9428A410j 2/3/94 07:43:23";
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
#include <sys/utsname.h> 
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/param.h>
#include <IN/standard.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <IN/backend.h>
#include "common.h"
#include <usersec.h>
#include <sys/id.h>
#include <sys/priv.h>
#include <sys/audit.h>
#include <ctype.h>

#include "qmain_msg.h"
#define MSGSTR(num,str)	catgets(catd,MS_QMAIN,num,str)
#define QDFMSGSTR(num,str)	get_qdf_msg(num,str)


nl_catd	catd;
	 
#define QMSGWEXC	"Qdaemon cannot exec write -r.  Errno = %d."

#define STFD		3	/* designated status file descriptor */

/*
 * queueing daemon --
 *      routines that start up a job
 *      most of this depends on whether we're running
 *              on unix or in vms
 */

extern char  nulldev[];
extern int nchildren;
extern boolean qkillfile;	/* global control flag in common.c */

char *get_qdf_msg();

char sbuf[MAXLINE];

char    *scopy();
void set_env(), oper_msg();
static char ** expand_cred(char **,char *);/* create a new cred */

/*==== Report an exec error to user and console */
exec_error(e,eebuf)
struct e *e;
char *eebuf;
{	sysnot(e->e_from,NULL,eebuf,e->e_mail ?DOMAIL : DOWRITE);
	errno = 0;
	syserr((int)EXITEXEC,eebuf);
}

/*==== Send the user id to the status file.  (not found in libqb because it
       should only be changed by qdaemon */
log_uid(uid,dev,e)
int uid;
struct d *dev;
struct e *e;
{	struct stfile devicestat;


        lseek(STFD, 0L, 0);
	if (read(STFD,&devicestat,sizeof(struct stfile)) != sizeof(struct stfile))
	{	sprintf(sbuf,MSGSTR(MSGSRE2,"Error reading %s.\nErrno = %d: %s"),
			stname(dev), errno, strerror(errno));
		exec_error(e,sbuf);
	}
	devicestat.s_uid = uid;
        lseek(STFD, 0L, 0);
	if (write(STFD,&devicestat,sizeof(struct stfile)) == -1)
	{	sprintf(sbuf,MSGSTR(MSGSWR2,"Error writing to %s.\nErrno = %d: %s"),
			stname(dev), errno, strerror(errno));
		exec_error(e,sbuf);
	}
}


unixexec(dev,e)
struct e *e;
struct d *dev;
{       char *realvec[MAXARGS];
	char **cred = (char **)NULL, *user = (char *)NULL;/*Used for setpcred */
	char * envvar;
	int fd, pid, i, p[2], c_uid;
	int earlyabort(void);
	uid_t getuidx(int);
	priv_t	priv;
	int j;
	boolean job_cancel = FALSE;
	int pcred_result;
	int access;
	char outbuf[12], inbuf[12];

	/*----Set up argvec, stuff for backend */
	preexec(dev,e);

	/*---Fork and exec backend */
	pid = fork();
	if (pid == -1)
		/*----ERROR: send message */
		syserr((int)EXITFATAL,MSGSTR(MSGFORK,"Unable to fork a new process."));	

	if (pid != 0)
	{
		/*----PARENT: new child b.e., save pid, return */
		nchildren++;
		pidput(pid,pidname(dev));
		return(pid);
	}

	/* CHILD ------ */
	/* Set process group id of child so SIGTERM will pass through
	   if the job is running */

	if ( setpgid (pid, pid) == -1) {
		sprintf(sbuf,MSGSTR(MSGPGID,"Cannot set process group id.\nErrno = %d: %s"),
                       	errno, strerror(errno));
               	exec_error(e,sbuf);
	}
	/*Change name of child process to avoid conflict with parent */
	errno = 0;
	qkillfile = FALSE;
	changename();

	/*----Set up interrupt routine for aborting job in this stage */
	signal(SIGTERM,(void (*)(int))earlyabort);

	/*----At this point we are a backend, and should act as such.
	      Open file desc. 3 and use this to talk to the status file for this dev.
	      We should use the routines out of libqb to talk to the
	      status file. */
	close(STFD);
	if ((fd = open(stname(dev),O_RDWR)) != STFD)
	{	sprintf(sbuf,MSGSTR(MSGNOPN,"cannot open %s for writing.\nErrno = %d: %s"),
		stname(dev),errno,strerror(errno));
		exec_error(e,sbuf);
	}
	if (write(STFD,e->e_s,sizeof(struct stfile)) == -1)
	{	sprintf(sbuf,MSGSTR(MSGSWR2,"Error writing to %s.\nErrno = %d: %s"),
			stname(dev), errno, strerror(errno));
		exec_error(e,sbuf);
	}
	log_init();

	/*----If there is an operator message, write it and wait for operator */
        if ((e->e_msg) && !(remote(dev->d_q->q_hostname)))
	{	log_status(OPRWAIT);
		oper_msg(e->e_msg,e->e_from,e->e_mail,&job_cancel);
		if (job_cancel)
			qexit((int)EXITSIGNAL);
	}

	/*----Set status to INIT because we are initializing the logical device */
	if (e->e_s->s_feed == NOFEED)
		log_status(INIT);

	/*----Open the right files for the backend */
	openfiles(dev,e);

	/*----qdameon must bequeath (pass on) the NET_CONFIG privilege
	      to rembak, so that remback can open the socket even though
	      it runs as the user.
	      if we're not running remback, the backend should have no 
	      privileges. */

	/*----are we going to exec rembak? */
	if (remote(dev->d_q->q_hostname))
	{
		/*----Bequeath NET_CONFIG */
		/*    privileges are stored as bit positions in pv_priv[0] and pv_priv[1]*/
		if (NET_CONFIG < 32)
		{
			priv.pv_priv[0] = (1 <<(NET_CONFIG-1));	
			priv.pv_priv[1] = 0;
		}
		else
		{
			priv.pv_priv[0] = 0;
			priv.pv_priv[1] = (1 <<(NET_CONFIG-1));	
		}

		if (setpriv(PRIV_SET|PRIV_BEQUEATH,&priv,sizeof(priv_t)) == -1)
		{
			sprintf(sbuf,MSGSTR(MSGPRIV,"Could not set privilege.\nErrno = %d: %s"),
				errno, strerror(errno));
			exec_error(e,sbuf);
		}
	}
	else	/* don't give the backend any privileges */
	{
		priv.pv_priv[0] = 0;
		priv.pv_priv[1] = 0;
		if (setpriv(PRIV_SET|PRIV_BEQUEATH,&priv,sizeof(priv_t)) == -1)
		{	sprintf(sbuf,MSGSTR(MSGPRIV,"Could not set privilege.\nErrno = %d: %s"),
				errno, strerror(errno));
			exec_error(e,sbuf);
		}
	}

	/* set the process credentials: uid, gid, concurrent groups, etc.*/
	if (e->e_s->s_feed == NOFEED)
		cred = e->e_pcred;
	else
		/*----Special feed job run as user lpd */
		user = "lpd";

	/* if rembak is to be exec'd, then run as group printq too */
	if (remote(dev->d_q->q_hostname))
	{
		/* set the process credentials to RGID=printq
		 * so that rembak has both the user and printq groups.
		 * rembak needs this to be able to read spooled files
		 */
		cred = expand_cred(cred,"REAL_GROUP=printq");
	}

	/* Here's the fix for qdaemon fails on diskless printer server
	 * because after it calls serpcred(), it doesn't have write
	 * permission to log_uid() into NFS type status file,
	 * ex: /var/spool/stat/s.lp0.lp0
	 *
	 * The logic is:
	 *	# just before setpcred()
	 *	open a pipe
	 *	fork a child process
	 *	if parent process
	 *		setpcred() 
	 *		getuidx() to get user's id
	 *		transfer the id from int to string
	 *		write the string formatted user id to the pipe
	 *		wait until the child process die
	 *	fi
	 *	if child process
	 *		read the string formatted user id from the pipe
	 *		transfer the string formatted user id back to int
	 *		log_uid() # write the user id to s.lp0.lp0 status file
	 *		exit(0)
	 *	fi
	 *	# do the rest job unixexec should do
	 */


	/* open a pipe, so we might pass the user id from parent
	 * to the child later
	 */
	if(pipe(p) < 0)
	{
		sprintf(sbuf,MSGSTR(MSGPIPE,"Unable to open a pipe.\nErrno = %d: %s"),
				errno, strerror(errno));
		exec_error(e,sbuf);
		exit(1);
	};

	pid = fork();   /* create a child process */
	if(pid == -1)   /* fork failed */
		syserr((int)EXITFATAL,MSGSTR(MSGFORK,"Unable to fork a new process."));

	/* This is the parent process, setpcred, getuidx, pass uid
	 * to the child, then wait until the child die.
	 */
	if(pid > 0)
	{
		/* Finally, call setpcred. */
		if (setpcred(user,cred) != 0)
		{	sprintf(sbuf,MSGSTR(MSGSPCR,"Could not set process credentials (pcred).\nErrno = %d: %s"),
				errno, strerror(errno));
			exec_error(e,sbuf);
		}
	
		/*----Write out the real uid to the status file */
		if ((e->e_s->s_uid = getuidx(ID_REAL)) == -1)
		{	sprintf(sbuf,MSGSTR(MSGRUID,"Cannot get real uid.\nErrno = %d: %s"),
				errno, strerror(errno));
			exec_error(e,sbuf);
		}

		sprintf(outbuf, "%d", e->e_s->s_uid);
#ifdef DEBUG
		fprintf(stderr, "unixexec: parent process. user uid = %s\n", outbuf);
#endif
		write(p[1], outbuf, 12);
		wait((int *) 0);
	}

	/* This is the child process, get uid, log_uid then go away
	 * let the parent take care the rest of the job
	 */
	if(pid == 0)   /* child process */
	{
		read(p[0], inbuf, 12);
		c_uid = atoi(inbuf);
#ifdef DEBUG
		fprintf(stderr, "unixexec: child process. user's uid = %d\n", c_uid);
#endif
		log_uid(c_uid, dev, e); 
		exit(0);
	}

	/*----Audit the request */
	(void) qaudit (dev, e, NULL);

	envvar = sconcat("QUEUE_BACKEND=",e->e_argvec[0],0);
	if(e->e_s->s_feed == NOFEED)
	{
		/*----Set the environment variable so that backend knows he is coming
		      from qdaemon */
		for(j = 0; e->e_penv[j] != NULL; j++);
		for(; j != 0; j--)
			e->e_penv[j + 1] = e->e_penv[j];
		e->e_penv[1] = envvar;
		setpenv(NULL, PENV_ARGV | PENV_RESET, (char **)e->e_penv,(char *)&(e->e_argvec[0]));
	}
	else
	{
		/*----For a feed job, just add QUEUE_BACKEND to the existing env. */
		e->e_penv = Qalloc(3 * sizeof(char *));
		e->e_penv[0] = scopy("USRENVIRON:");
		e->e_penv[1] = envvar;
		e->e_penv[2] = NULL;
		setpenv(NULL, PENV_ARGV | PENV_DELTA, (char **)e->e_penv,(char *)&(e->e_argvec[0]));
	}
		
	/*----Failure if reach here */
	sprintf(sbuf,MSGSTR(MSGQEX1,"Qdaemon couldn't exec %s.\nErrno = %d: %s"),
		e->e_argvec[0], errno, strerror(errno));
	exec_error(e,sbuf);
}


/* change our name since we might hang around a while before exec'ing; */
/* don't want a ps to show 2 qdaemons. */
changename()
{       register char *newname = "qd fork";
	register char *mine;
	extern char *myname;

	mine = myname;
	while (*mine)
	    *mine++ = (*newname? *newname++: ' ');
}

/*
 * open files for the backend.
 * stdin to /dev/null, or device if requested
 * stdout similarly
 * fd 3 to the status file
 * if device open hangs, note this fact and ask for help.
 * it would be simpler just to close all files, THEN do the opens
 * since this would avoid getting status file fd != 3
 * but if this happens, it's a problem we should know about,
 * so we explicitly bet on only 3 files being open.  if we lose,
 * qdaemon bombs, and we have to find the lost fd.
 */

openfiles(dev,e)
struct d *dev;
struct e *e;
{
	register int fd;
	int access;

	close(fileno(stdin));       /* stdin */
	open (nulldev,O_RDONLY);

	if (dev->d_file[0])             /* open this file as stdout */
	{
		access = ((dev->d_access == BOTH)? O_RDWR: O_WRONLY);
		dopen(dev->d_file,access,stname(dev),e);
		lseek(1,0L,2);              /* in case using disk file */
		if (dev->d_access == BOTH)
		{
			close(fileno(stdin));
			dup(fileno(stdout));
		}
	} else {
		close(fileno(stdout));
		open(nulldev,O_WRONLY);
		log_status(RUNNING);
	}
}


/* job was canceled before we could even exec the backend */
earlyabort(void)
{
	qexit((int)EXITSIGNAL);
}


/*
 * open file str for access.  send help messages until succeed.
 * stfile is status file to complain into
 */
dopen(str,access,stfil,e)
char	*str;
int	access;
char	*stfil;
struct e *e;
{
	int	ans;
	boolean	waited;
	char *pchTmp;
	char caLockFile[MAXPATHLEN];
	int iLF;
        struct flock flock_struct;
	int fdLock;

	waited = FALSE;
	/*
	 * Create a possibly non-unique name, of the form
	 * /var/spool/lpd/stat/_dev_lp0
	 * for use as a lock file.
	 */
	strcpy(caLockFile, STATDIR);
	iLF = strlen(STATDIR);
	caLockFile[iLF++] = '/';
	for ( pchTmp = str; '\0' != *pchTmp, iLF < MAXPATHLEN; pchTmp++) {
		if ( '/' == *pchTmp ) {
			caLockFile[iLF++] = '_';
		} else {
			caLockFile[iLF++] = *pchTmp;
		}
	}
	caLockFile[iLF++] = '\0';

	/*
	 * Create the lock file.
	 */
	if ((fdLock = open(caLockFile, access | O_CREAT, 0 )) == -1)
	{	sprintf(sbuf,MSGSTR(MSGNOPN,"cannot open %s for writing.\nErrno = %d: %s"),
			caLockFile,errno,strerror(errno));
		exec_error(e,sbuf);
	}

	/*
	 *  Lock the file.  qdaemon should block here until it has
	 *  Exclusive access to /dev entry.
	 */
	log_status(BUSY);
	flock_struct.l_type = F_WRLCK;
	flock_struct.l_whence = 0;
	flock_struct.l_start = 0;
	flock_struct.l_len = 0;
	if ( -1 == fcntl(fdLock, F_SETLKW, &flock_struct ) )
	{	sprintf(sbuf,MSGSTR(MSGNLCK,"cannot lock %s for writing.\nErrno = %d: %s"),
			caLockFile,errno,strerror(errno));
		exec_error(e,sbuf);
	}

	close(fileno(stdout));
	while (1)
        {
		errno = 0;
		ans = open(str,access);

		if ( -1 != ans )
		{
			/*----File error if file is not given stdout (sanity) */
			if ( fileno(stdout) != ans )
			{
				syswarn(MSGSTR(MSGSTDO,"File, %s opened for backend, but not given to standard out.  Job terminated."),str);
				close(ans);
				close(fdLock);
				qexit((int)EXITFILE);
			}
			else
				/*----everything OK, break from loop */
				break;
		}

		/*----Open purely failed, send message, retry */
		if (waited == FALSE)
		{
			waited = TRUE;
			log_status(WAITING);
			syswarn(QDFMSGSTR(MSGUNAB,"Unable to open %s as standard out for backend."),str);
			errno = 0;	/* to avoid unecessary messages */
			systell(QDFMSGSTR(MSGRTRY,"Retrying..."));
		}
		else if ( errno == EBUSY ) 
			sleep(10);
		else if ( errno == ENXIO ) 
			sleep(60);
		else {
			close(fdLock);
			syserr((int)EXITFATAL,QDFMSGSTR(MSGUNAB,
				"Error: Unable to open device %s."),str);
		}

		/*----Impossible situation handling (is this needed?) */
		if (access == O_RDWR && errno == ENODEV)
		{
			access = O_WRONLY;
			continue;
		}

		/*----Wait for help, then retry */
		sleep(1); 
	}

	/*----Reset status if wait happened */
	if (waited == TRUE)
		systell(QDFMSGSTR(MSGSUCO,"Successfully opened %s as standard out for backend."),str);
	log_status(RUNNING);
}




/**********************************************************/
/* oper_msg                                               */
/* if there is an operator message create a pipe and fork */
/* The parent writes the message onto the pipe. The child */
/* reads from the pipe and execs a write with reply to the*/
/* console.                                               */
void
oper_msg(message,from,mail,j_cancel)
char *message;
char *from;		/* user request is from */
boolean mail;		/* TRUE = use mail instead of write for communicating */
boolean *j_cancel;
{
	char	sbuf9[MAXLINE];
	int	status,
		pid,
		p[2],
		rc;

	/*----Pipe and fork */
	pipe(p);
	switch( pid = fork() )
	{
		case -1:                        /* can't fork */
			syswarn(MSGSTR(MSGFKWT,
				"Cannot fork write process.  Errno = %d."),errno);
			break;

		case 0:                         /* child */
			close(fileno(stdin));
			dup(p[0]);
			close(p[0]);
			close(p[1]);

			/*----Exec the write -r command */
			execlp("/usr/bin/write","write","-r","-","console",0);

			/* if got this far there's a problem */
			syserr((int)EXITEXEC,MSGSTR(MSGWEXC,QMSGWEXC),errno);

		default:                        /* parent */
			close(p[0]);
			write(p[1],message,strlen(message));
			close(p[1]);

			errno = 0;

			/* wait for the right child to die      */
			while( (rc = waitpid(pid,&status,0)) != pid )
			{
				if( rc == -1 && errno == EINTR )
				{
					/* Interrupted by a signal, keep going */
					errno = 0;
					continue;
				}
				syswarn(MSGSTR(MSGWPID,
					"Wrong pid received while waiting for operator message"));
			}

			if (status == 0)
				/* ok,  exec backend now */
				break;
			else if (WEXITSTATUS(status) == 1) {
				/* cancel request */
				sprintf(sbuf9,MSGSTR(MSGOPCN,"Operator canceled your job."));
				*j_cancel = TRUE;
			}


			/* maybe server's not running or node down */
			else if ( WIFEXITED(status) ) {
				sprintf(sbuf9,MSGSTR(MSGEXIT, "Write failed, exit = %d."), WEXITSTATUS(status));
			}
			else if ( WIFSIGNALED(status) ) {
				sprintf(sbuf9,MSGSTR(MSGSIG, "Write failed, signal = %d."), WTERMSIG(status));
			}
			else {
				sprintf(sbuf9,MSGSTR(MSGSTOP, "Write stopped, signal = %d."), WSTOPSIG(status));
			}
			sysnot(from,NULL,sbuf9,mail ? DOMAIL : DOWRITE);
			break;

		}  /* end of switch */
}


/*----This routine is run just prior to execing the backend.
	It sets up the stat and dev structs correctly.  Only things
	that can be done only at execution time belong in here.
*/
preexec(dev,e)
struct d	*dev;
struct e	*e;
{
	int	i;			/* for counting argv in remote backend parameters */

	/*----Insure that backend has correct device and queue information from libqb */
	/*    s_title, s_to, and s_from are given at job submission time, not here */
	strncpy(e->e_s->s_device_name,dev->d_name,S_DNAME);
	strncpy(e->e_s->s_queue_name,dev->d_q->q_name,S_QNAME);

	/*----Determine the nature of this job */
  	e->e_s->s_was_idle = FALSE;
	if (e->e_s->s_feed == NOFEED)
	{
		/*----First job after an idle state (feed job) */
		if (dev->d_user[0] == '\0')
  			e->e_s->s_was_idle = TRUE;

		/*----Give all the normal info for a regular (non-feed) job */
		strncpy(dev->d_user,e->e_to,LOGNAME);
		strncpy(e->e_s->s_qdate,dqstr(e->e_name),S_DATE);
		e->e_s->s_jobnum = e->e_jobnum;
		e->e_s->s_align  = dev->d_align;
		if (e->e_s->s_head == USEDEF) 
			e->e_s->s_head = dev->d_head;
		if (e->e_s->s_trail == USEDEF)
			e->e_s->s_trail = dev->d_trail;
	}
	/*----Set up the arguments for the backend, remote or local */
	if (remote(dev->d_q->q_hostname))
	{
		/*----Construct the arguments for rembak */
		i = separate(e->e_argvec,dev->d_backend);

		/*----Server */
		e->e_argvec[i++] = scopy("-S");
		e->e_argvec[i++] = scopy(dev->d_q->q_hostname);

		/*----Queue */
		e->e_argvec[i++] = scopy("-P");
		e->e_argvec[i++] = scopy(dev->d_q->q_rname);

		e->e_argvec[i++] = scopy("-N");
		e->e_argvec[i++] = scopy(dev->d_q->q_sros);

		/*----Operator message */
		if (e->e_msg != NULL)
		{
			e->e_argvec[i++] = scopy("-o");
			e->e_argvec[i++] = sconcat("-M",e->e_msg,0);
		}

		/*----Notify user of job status */
		if (e->e_no)
		{
			e->e_argvec[i++] = scopy("-o");
			e->e_argvec[i++] = scopy("-m");
		}
		e->e_argvec[i] = NULL;
		e->e_argnum = i;

		/*----Since remote, will be taken care of by remote machine  */
		if (e->e_no)
			e->e_no=FALSE;
	}
	else
		/*----This is a local backend, take args from "devices = " clause in /etc/qconfig */
		e->e_argnum = separate(e->e_argvec,dev->d_backend);

	/*----Now put the args from job invocation (JDF) after the stuff we just put in */
	for(i = 0; i < e->e_jdfnum; i++)
		e->e_argvec[(e->e_argnum)++] = scopy(e->e_jdfvec[i]);
	e->e_argvec[e->e_argnum] = NULL;

#ifdef DEBUG
        if(getenv("PREEXEC"))
        {       
                sysraw("preexec: ");
                for(i = 0; e->e_argvec[i] != NULL; i++)
                        sysraw("[%s]",e->e_argvec[i]);
                sysraw("\n");
        }
#endif
	return(0);
}


myos(rostr)
char * rostr;	/* Remote Operating System STRing*/
{

	if ((strcmp(rostr,S_DEFAULT_STATFILTER))== 0 )
		return (TRUE);
	else
		return (FALSE);
}
		
	
/* auditing command */
int
qaudit (d, e, str)
struct  d *d;
struct  e *e;
char	*str;
{
	char * 	host;

	host = localhost();
	auditwrite (	"ENQUE_exec", AUDIT_OK, 
			d->d_q->q_name, strlen(d->d_q->q_name)+1,
			e->e_request, strlen(e->e_request)+1, 
			host, strlen(host)+1,
			d->d_file, strlen (d->d_file)+1,
			e->e_to, strlen(e->e_to)+1,
			str, strlen(str)+1, NULL );

}

/*
 * Name:  expand_cred
 *
 * Function:
 *	Allocate storage for an expanded cred structure.
 *	The one pointed to be e->e_cred may not be big enough if we
 *	have to add another credential.  Also, if the initial structure
 *	isn't present, just the new one is allocated.
 *
 * Input:
 *	cred - The original cred structure, may be NULL.
 *	new  - A new string to add to the end.
 *
 * Return:
 *	A pointer to the new cred array is returned.
 */
static char **
expand_cred(char **cred,char *new)
{
	int i,j,L;
	char **p, *s, *d;

	/* Setup minimal length. */
	i = 2;			/* Need at least 2 array elements. */
	L = strlen(new)+2;	/* Add trailing zeros. */

	/* Get length and no. of elements if cred non-NULL. */
	for (p=cred; (*p!=(char *)NULL) && (**p!='\0'); p++) {
		i+=1;
		L+=(strlen(*p)+1);
	}

	/* Allocate the new cred. */
	p = Qalloc(L+(i*sizeof(char *)));

	/* d is the destination string pointer. */
	d = (char *)(p+i);

	/* Copy in the old if applicable. */
	for (j=0; j<(i-1); j++) {
		/* Set the string pointer in the array. */
		p[j] = d;
		/* Get the source from either the old cred or new string. */
		s = (j<(i-2))? cred[j]: new;
		/* Copy the string and a trailing NULL. */
		for (; *s; *d++=*s++);
		*d++='\0';
	}
	/* Add the null string. */
	p[j] = (char *)NULL;

	return(p);
}

/*
 * Name:  get_qdf_msg
 *
 * Function:
 *      get_qdf_msg() retrieves a message from the QMAIN msg catalog.
 *      This routine should only be used in the child process, after the
 *      child is forked, and only when the child process will continue
 *      to run; if the message needed is for a fatal error - one that will
 *      subsequently exit - this routine is not necessary.
 *      The problem we are trying to avoid is due to the fact that the
 *      catgets() libc routine checks for the PID of the process that opened
 *      opened the catalog ( in this case the parent process ); if it is
 *      different than the current process, then it closes the catalog and
 *      reopens it again, which takes FD 1 and it results on qdaemon not
 *      getting file descriptor 1, which it needs for the backend.
 *
 * Input:
 *      num - number of message
 *      str  - default string, if message is not found
 *
 * Return:
 *      A pointer to a buffer with message requested
 */

char *get_qdf_msg(num, str)
int num;
char *str;
{
        char *p, *dest;
        nl_catd ncatd;

        ncatd = catopen(MF_QMAIN,0);
        p = catgets(ncatd,MS_QMAIN,num,str);
        dest = Qalloc(strlen(p)+1);
        strcpy(dest,p);
        catclose(ncatd);
        return(dest);

}

