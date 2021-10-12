static char sccsid[] = "@(#)61	1.19  src/bos/usr/bin/src/cmds/srcdchd.c, cmdsrc, bos411, 9428A410j 12/19/91 13:32:35";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *	srcdchd,parce_argenv,create_subsys_socket
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/* faterr will of course cause this process to exit on an error
 * but first it will write an error code through our pipe back to the
 * parent process signaling that our exec failed 
 */
#define faterr(rc,stat,str1,str2,err1,err2) \
{\
	if(rc < 0)\
	{\
   		status = stat;\
   		write(fd, &status, sizeof(status));\
		exit(-1);\
	}\
}

/* external definitions                                                */

#include <fcntl.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <signal.h>
#include <usersec.h>
#include <pwd.h>
#include <nl_types.h>
#include "src.h"
#include "src11.h"
#include "src10.h"
#include "srcmstr.h"

/* server message flag to get a subsystem queue */
#define SRC_SVR_MSGFLG 0666

/* msgflg to create a subsystem queue     */
#define SRC_CREATE_FLG SRC_SVR_MSGFLG | IPC_CREAT | IPC_EXCL

extern int    setpgrp();         /* set pgroup function            */
extern void   exit()   ;         /* exit process                   */
extern char *malloc();
static int parce_argenv();
static void create_subsys_socket();

static int status;
static int fd;

srcdchd(fdes)
int  fdes;
{

	char *envp[256];
	char *cred[3];
	char cred0[256];
	char cred1[256];
	char *cmdp[256];
	int      rc;                   /* Return code variable         */
	int      svr_qid=0;
	struct	msqid_ds msgbuf;
	struct passwd *passwd;
	char realusr[256];
	char auditusr[256];
	int i;
        struct sigvec newvec,oldvec;

        /* Our Alarm Handler */
        void alarm_nopen();
	
	/* let this file know what file number our pipe is */
	fd=fdes;

	/*   Become head of own Process Group  */
	(void) setpgrp();

	/* close our communication ports */
	close_src_sockets();

	/* Reset any signals caught by SRCMSTR to the default signal handler. */
	if(fd!=3)
	{
		close(3);
		dup2(fd,3);
		fd=3;
		fcntl(3, F_SETFD, 1);
	}
	kleenup(4,0,0);


	/* set the nice priority of the process */
	if(curraste->subsys->priority != 0) /* we allow prior 1-39 */
	{
		rc=nice(0)+20;	/* nice returns the priority -20 */
		if(curraste->subsys->priority != rc)
			nice((int)(curraste->subsys->priority-rc));
	}

	/* ckeck the realuser account */
	if((passwd=getpwuid((int)curraste->subsys->uid)) == NULL)
		faterr(-1,SRC_SUBSYSID,curraste->subsys->subsysname,0,SRC_SUBSYSID,errno);
	strcpy(realusr,passwd->pw_name);
	if(ckuseracct(realusr,0,0)!=0)
		faterr(-1,SRC_SUBSYSID,curraste->subsys->subsysname,0,SRC_SUBSYSID,errno);

	/* ckeck the audituser account */
	if((passwd=getpwuid((int)curraste->subsys->auditid)) == NULL)
		faterr(-1,SRC_AUDITID,curraste->subsys->subsysname,0,SRC_AUDITID,errno);
	strcpy(auditusr,passwd->pw_name);
	if(ckuseracct(auditusr,0,0)!=0)
		faterr(-1,SRC_AUDITID,curraste->subsys->subsysname,0,SRC_AUDITID,errno);

	sprintf(cred0,"LOGIN_USER=%s",auditusr);
	sprintf(cred1,"REAL_USER=%s",realusr);
	cred[0]=cred0;
	cred[1]=cred1;
	cred[2]=(char *)0;


	/* create the socket to receive packets from SRC on
	 *	always will be file number zero
	 */
	if(curraste->subsys->contact == SRCSOCKET)
		create_subsys_socket();


	rc=setpcred(realusr,cred);

	faterr(rc,SRC_SUBSYSID,curraste->subsys->subsysname,0,SRC_SUBSYSID,errno);

        /* D43120, Set the alarm so that we don't get stuck in open for-ever
         * we will open the device with O_NDELAY if we get failed once. So
         * if the device is available then let it be opened without O_NDELAY
         * will have to use one alarm before each open call, to make sure that
         * if more than one fails, we deal with it.
         */
        bzero(&newvec,sizeof(newvec));    /*  set */
        newvec.sv_handler = alarm_nopen;  /*  our */
        sigvec(SIGALRM,&newvec,&oldvec);  /* alarm handler */
        alarm((unsigned int)OPENWAIT);

	/* setup standard file descriptors */

	/* ignore stdin if communications are by socket */
	if (curraste->subsys->contact != SRCSOCKET)
	{
		(void) close(0);
		/* stdin */
		rc=open(curraste->subsys->standin,O_CREAT | O_RDONLY | O_TRUNC | O_NOCTTY,0777);
		if (rc == -1)
		   rc=open(curraste->subsys->standin,O_NDELAY | O_CREAT | O_RDONLY | O_TRUNC | O_NOCTTY,0777);
		faterr(rc,SRC_INPT,curraste->subsys->subsysname,0,SRC_INPT,errno);
	}

	(void) close(1);
	(void) close(2);

        alarm((unsigned int)OPENWAIT);
	/* stdout */
	rc=open(curraste->subsys->standout,O_CREAT | O_WRONLY | O_TRUNC | O_NOCTTY,0777);
	if (rc == -1)
           rc=open(curraste->subsys->standout,O_NDELAY | O_CREAT | O_WRONLY | O_TRUNC | O_NOCTTY,0777);
	faterr(rc,SRC_OUT,curraste->subsys->subsysname,0,SRC_OUT,errno);

	alarm((unsigned int)OPENWAIT);
	/* stderr */
	rc=open(curraste->subsys->standerr,O_CREAT | O_WRONLY | O_TRUNC | O_NOCTTY,0777);
	if (rc == -1)
           rc=open(curraste->subsys->standerr,O_NDELAY | O_CREAT | O_WRONLY | O_TRUNC | O_NOCTTY,0777);
	faterr(rc,SRC_SERR,curraste->subsys->subsysname,0,SRC_SERR,errno);
	
	/* reset */
	alarm((unsigned int)0);
        sigvec(SIGALRM,&oldvec,(struct sigvec *)0);

	/* create IPC queue for those subsystems that expect to find
	 * requests from SRC on that queue
	 */
	if (curraste->subsys->contact == SRCIPC)
	{
		svr_qid = msgget(curraste->subsys->svrkey, SRC_CREATE_FLG);
		if (svr_qid == -1 && errno == EEXIST)
		{
			svr_qid=msgget(curraste->subsys->svrkey,SRC_SVR_MSGFLG);
			rc=msgctl(svr_qid,IPC_RMID,&msgbuf);
			faterr(rc,SRC_SVRQ,curraste->subsys->subsysname,0,SRC_SVRQ,errno);
			svr_qid=msgget(curraste->subsys->svrkey,SRC_CREATE_FLG);
		}
		faterr(svr_qid,SRC_SVRQ,curraste->subsys->subsysname,0,SRC_SVRQ,errno);
	}

	/* subsystem command line arguments will appear as follows
	 *	argv[0] - subsystem command being execed.
	 *	argv[1 to n] - command line arguments from the subsystems object
	 *	argv[n+1 to m] - arguments passed with the start request
	 */
	bzero(cmdp,sizeof(cmdp));
	cmdp[0]=curraste->subsys->path;
	i=1+parce_argenv(curraste->subsys->cmdargs,strlen(curraste->subsys->cmdargs),&cmdp[1]);
	parce_argenv(curraste->parm,(int)curraste->parmlen,&cmdp[i]);

	/* exec subsystem
	 *	the subsystgem must be an a.out executable or a 
	 *	file that names another executable in a.out format
	 */
	bzero(envp,sizeof(envp));
	envp[0]=PENV_USRSTR;
	parce_argenv(curraste->env,(int)curraste->envlen,&envp[1]);
	rc=setpenv(NULL,PENV_INIT|PENV_ARGV,envp,(char *) cmdp);

	/* exec of subsystem failed
	 *	1. destroy message queue if created
	 *	2. exit
	 */
	{
		int sverrno;

		sverrno=errno;
		if(svr_qid > 0)
			msgctl(svr_qid, IPC_RMID, &msgbuf);
		faterr(-1,SRC_FEXE,curraste->subsys->subsysname,0,SRC_FEXE,sverrno);
	}
}

static int parce_argenv(in_argenv,len,out_argenv)
char *in_argenv;    	/* input string to be parced */
int len;        	/* length of input string */
char **out_argenv;  	/* pointer to an array of pointer */
{
	int wchars;
	int chars;
	int totwchars;
	int totchars;
	int totargs;
	int wc_len;
	wchar_t *args;
	char *out_args;
	char quote;
	wchar_t target[10];
	wchar_t single_quote;
	wchar_t double_quote;
	wchar_t backslash;


	/* passed a null string? */
	if(*in_argenv == 0 || len == 0)
		return(0);
	/* allocate memory for the aruments */
	args=(wchar_t *)malloc((len+1)*sizeof(wchar_t));
	if(args==(wchar_t *)NULL)
	{
		faterr(-1,SRC_MMRY,curraste->subsys->subsysname,0,SRC_MMRY,SRC_MMRY);
	}
	out_args=malloc(len+1);
	if(out_args==(char *)0)
	{
		free(args);
		faterr(-1,SRC_MMRY,curraste->subsys->subsysname,0,SRC_MMRY,SRC_MMRY);
	}
	wc_len=mbstowcs(args,in_argenv,len);

	/* make sure input argenv is null terminated */
	args[wc_len]=(wchar_t)0;

	/* seperate all args/envs entries
	**   '-q -s -m -x"dog day"'
	**   gets parced into
	**      -q
	**  -s
	**  -m
	**  -x"dog day"
	**/
	mbstowcs(target," \\\"\'",sizeof(target));
	mbtowc(&single_quote,"\'",1);
	mbtowc(&double_quote,"\"",1);
	mbtowc(&backslash,"\\",1);
	for(totwchars=0,totargs=0;wc_len > totwchars;)
	{
		/* skip starting blanks */
		while(iswspace((int)*args))
		{
			args++;
			totwchars++;
		}

		/* argument null? */
		if(*args==(wchar_t)0)
		{
			free(args);
			return(totargs);
		}

		/* find argument delimiter */
		wchars=wcscspn(args,target);

		/* skip backslashed delimiters */
		while(*(args+wchars) == backslash && totwchars + wchars <= len)
		{
			/* skip the quote */
			wchars++;
			/* skip the next character */
			wchars++;
			/* find the delimiter */
			wchars=wchars+wcscspn(&args[wchars],target);
		}

		/* arg is quoted string */
		if(args[wchars] == single_quote || args[wchars] == double_quote)
		{
			quote=args[wchars];
			/* find next quote */
			for(++wchars; args[wchars]!=(wchar_t)0 && args[wchars] != quote;wchars++)
			{
				/* skip over backslashes */
				if(args[wchars] == backslash)
					++wchars;
			}
			/* start with a quote, quotes disappear */
			if(*args==quote)
			{
				++args; /* don't want quote in arg */
				totwchars++; /* the skiped quote */
				wchars--;  /* don't want to see the second quote either*/
			}
			else
				++wchars; /* we want to see the quote */
		}

		/* make arg known through out_argenv array of pointers */
		args[wchars]=(wchar_t)0; /* must be null terminated */
		totwchars=totwchars+1+wchars;
		out_argenv[totargs++]=out_args;
		chars=wcstombs(out_args,args,len+1);
		len=len-chars;
		out_args[chars]='\0';
		out_args=out_args+chars+1;
		args=args+wchars+1; /* start of next arg */
	}
	free(args);
	return(totargs);
}
static void create_subsys_socket()
{
	int subsys_socket;

	subsys_socket=src_setup_socket(&curraste->sun,SOCK_DGRAM,SRCPKTMAX,0);
	faterr(subsys_socket,SRC_SUBSOCK,curraste->subsys->subsysname,0,SRC_SUBSOCK,errno);

	/* make it file number zero */
	if(subsys_socket != 0)
	{
		faterr(dup2(subsys_socket,0),SRC_SUBSOCK,curraste->subsys->subsysname,0,SRC_SUBSOCK,errno);
		close(subsys_socket);
	}
}

/* A do-nothing alarm handler */
void alarm_nopen()
{
}
