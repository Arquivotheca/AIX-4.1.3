static char sccsid[] = "@(#)69	1.29  src/bos/usr/bin/que/libque/subr.c, cmdque, bos411, 9428A410j 1/29/93 12:19:33";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: flgmatch, qsetup, argtype,  request, saveints, isafile,
 *            qlink, nammatch, goose
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

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <IN/standard.h>
#include <sys/types.h>
/* #include <IN/DRdefs.h> */
#include <sys/fullstat.h>
#include <sys/dir.h>
#include <IN/backend.h>
#include "common.h"
#include "enq.h"
#include <fcntl.h>
#include <sys/id.h>
#include <sys/priv.h>
/* AIX security enhancement 	*/
#include <usersec.h>
#include <ctype.h>
/* Message queue addition */
#include <sys/ipc.h>
#include <sys/msg.h>

#include "libq_msg.h"

#include <time.h>

extern char intignore;
extern char hupignore;
extern char gotint;
extern FILE *console;
extern char qdpid[];
extern char statdir[];
extern int terminate(void);
extern char * getlgdir();
extern uid_t  invokers_uid;
extern uid_t  programs_uid;


/* 0 if no, length of match if yes. */
flgmatch(usr,flg)
register char *usr;
register char *flg;
{
	register len = 0;

	while (*usr++ == *flg)
	{
		if (*flg++ == '\0')      /* end of both */
			return(len);
		len++;
	}
	/* first difference */
	if (*--usr == '=' && *flg == '\0')
		return(len);
	else
		return(0);
}

/* AIX security change 	*/
/* removed char *cwd  (qe_curdir was removed) */

/* plug in all the fields */
qsetup(qe)
register struct qe *qe;
{
/* AIX security enhancement 	*/
	char	**getpenv(int);

	qe->qe_numcops = 1;                     /* number of copies */
	qe->qe_rm = 0;
	qe->qe_numblks = 0;

/* AIX security enhancement 	*/
/* removed qe_uid and qe_gid. qe_pcred below will take care of these */
/*	qe->qe_uid = getuid();
	qe->qe_gid = getgid(); */

	/* AIX security enhancement 	*/
	/* toggle to the privilege domain of the program */
	seteuid(programs_uid);


	/* get all of the process credentials */
	if ((qe->qe_pcred = getpcred((int)NULL)) == NULL) 
		syserr((int)EXITBAD,GETMESG(MSGGPCR,"Could not get process credentials. Errno = %d"),errno);

	/* get the process environment */
	if ((qe->qe_penv = getpenv(PENV_USR | PENV_SYS)) == NULL) 
		syserr((int)EXITBAD,GETMESG(MSGGPNV,"Could not get process environment. Errno = %d"),errno);

	/* toggle to the privilege domain of the invoker */
	seteuid(invokers_uid);


#ifdef LOGACCT
	qe->qe_acct = logacct();                /* account */
#else
	qe->qe_acct = '0';                      /* account */
#endif
	qe->qe_numblks = 0;                     /* size of file */
	qe->qe_logname = getlgnam();             /* log name */
	qe->qe_logdir = getlgdir();               /* log directory */
	errno = 0;


/* AIX security enhancement 	*/
/* removed DEBUG code		*/
/* removed qe_curdir. qe_penv will include the current dir */
/* removed beuser() and besuperuser() */

	qe->qe_to = qe->qe_logname;             /* to me */
	qe->qe_reqname = NULL;
	qe->qe_priority = P_DEFAULT;
	qe->qe_trail = qe->qe_head = USEDEF;
	qe->qe_queue = NULL;
	qe->qe_dev = NULL;
	qe->qe_movqueue = NULL;
	qe->qe_movdev = NULL;
}



/* lex arg by returning ARGsomething, possibly setting *argrem (if */
/* arg of form "to=santa") to the stuff after the = .                */
/* if argrem is NULL, don't set it */
argtype(arg)
register char *arg;
{
	static char *qarg;
	/* register struct flags *flagp; */ /* Not used */

	/* so arg[0] is '-' */

	if ((arg[1] == '\0')&& (arg[0] == '-')) 	/* "-" by itself */
		return(ARGSTDIN);

	if (isafile(arg))
		return(ARGFILE);
	else
		syserr((int)EXITFATAL,GETMESG(MSGACCS,"Argument %s is not an accessible file."),arg);
}


/* D45466, send a request with jdf name on a message que to qdaemon */
request(jdf, type)
char *jdf;
long type;
{
        int writeid;
        int times;
        Message msg;


#ifdef  DEBUG
        if( getenv("REQUEST") )
                sysraw("request\n");
#endif

        /* make sure that qdaemon is not there, and not Re-Execing */
        times = 0;
        do
        {
          if ((writeid = msgget(ENQ_QUE, 0)) >= 0)
             break;
          sleep(1);
        }
        while (times++ < 5);

        if (writeid >= 0)  /** success **/
        {
           /* build and put it on the message queue */
           msg.mtype = type;
           strcpy(msg.jdfname,jdf);
           if (msgsnd(writeid,&msg,sizeof(msg),0))
              syserr((int)EXITFATAL,GETMESG(MSGSNDERR,"Error in msgsnd.  Errno = %d."),errno);
        }
        else
           syswarn(GETMESG(MSGWAKE,"Cannot awaken qdaemon. (request accepted anyway)"));

        return(0);
}
/*** request() ends ***/



/* show we got an interrupt */
saveints(void)
{
	signal(SIGHUP,(void (*)(int))saveints);
	signal(SIGINT,(void (*)(int))saveints);
	gotint = TRUE;
}




/* given a file tnam, write it into qdir with name realnam.  */
qlink(tnam, realnam)
register char *tnam;
register char *realnam;
{
	register char *actual;
	
#ifdef	DEBUG
	if( getenv("QLINK") )
		sysraw("qlink(%s,%s)\n",
		        tnam ? tnam : "NULL", realnam ? realnam : "NULL" );
#endif	DEBUG

	/* don't take interrupts until we have remembered file name */
	if (!hupignore)
		signal(SIGHUP,(void (*)(int))saveints);
	if (!intignore)
		signal(SIGINT,(void (*)(int))saveints);

	if ((actual = (char *)qentry(tnam,realnam)) == NULL)    /* failed */
	{
		extern int errno;
		switch(errno)               /* failure */
		{
		case EINTR:             /* DEL typed */
			unlink(tnam);
			terminate();
		default:                /* something weird */
			syserr((int)EXITFATAL,GETMESG(MSGQLNK,"%s qlink link error.  Errno = %d."),tnam,errno);
		}
	}

	/* the rename was successful */
	/* numfiles++; */
	remember(actual);
	if (!hupignore)
		signal(SIGHUP,(void (*)(int))terminate);
	if (!intignore)
		signal(SIGINT,(void (*)(int))terminate);
	if (gotint == TRUE)
		terminate();
}


/*	The purpose of this routine is to determine if the given word
 *	is a file.   If the file exists, this routine returns TRUE.
 * 	Whether or not the code is running as root may make a difference
 * 	as to whether doing a stat on the file is possible since root
 * 	may not map to another machine (in DS).
 *	Make sure that the routine is being called from the user's
 *	directory since the pathname is likely to be directory relative.
 *	The routine is used primarily to determine whether to copy or
 *	remove a file when the -rm or -cp options are used with an 
 *	unfriendly backend.  finch Thu Aug  4 14:10:31 CDT 1988
*/
boolean isafile(word)
register char * word;
{
	struct fullstat statb;

        if (fullstat(word, STX_NORMAL, &statb) < 0)
                return(FALSE);
        else
                return(TRUE);
}


/* goose the qdaemon
 * we need to keep this routine to have enq interupt qdaemon 
 * in case it is in a wait() call
 */
goose()
{
	register FILE *pidfile;
	register int  ans;
	int  pidnum;
	/* AIX security enhancement	*/
	void	privilege();

#ifdef	DEBUG
	if( getenv("GOOSE") )
		sysraw("goose\n");
#endif

	if ((pidfile = fopen(qdpid,"r")) != NULL)
	{
		ans = fscanf(pidfile,PID_FMT,&pidnum);
		fclose(pidfile);
		if (ans == 1)
		{
			/* AIX security enhancement						*/
			/* acquire the privilege to make the kill() call - BYPASS_DAC_KILL	*/
			privilege(PRIV_ACQUIRE);

			if (kill(pidnum,SIGQUEUE) == 0)
			{
				/* redrop our privileges until we need them again	*/
				privilege(PRIV_LAPSE);
				return(0);
			}
			else
				syswarn(GETMESG(MSGQSIG,"Cannot signal qdaemon. Errno = %d"),errno);
		}
	}

	syswarn(GETMESG(MSGWAKE,"Cannot awaken qdaemon. (request accepted anyway)"));
	privilege(PRIV_LAPSE);
	return(0);
}

