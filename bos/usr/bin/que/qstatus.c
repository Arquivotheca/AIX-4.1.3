static char sccsid[] = "@(#)40	1.24.1.13  src/bos/usr/bin/que/qstatus.c, cmdque, bos41J, 9511A_all 3/7/95 11:43:09";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: qstatus
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <IN/standard.h>
#include <sys/fullstat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <IN/backend.h>
#include <locale.h>
#include "common.h"
#include "qstatus.h"
#include <ctype.h>
#include <sys/wait.h> 
#include <signal.h>

#include "qstat_msg.h"
#define MAXSTR 		10
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QSTAT,num,str)

extern void resetQCBFile();
extern boolean palladium_inst;

/*----Variables */
char 		*progname = "qstatus";
struct allparms	ourparms;		/* all parameters kept here */
struct q	*qlist;			/* linked list of q structures */
char		args[] = OURARGS;	/* command line arguments to search for */
boolean		disp_palladium_q = TRUE; /* should palladium q be displayed? */
boolean		aixq = FALSE;            /* Is AIX queue specified? */
int		exit_status;

/*==== MAIN */
main(argc, argv)
int argc;
char **argv;
{
	struct q	*read_quedev();
	struct stat 	statbuf;	/* buffer for stat call */

#ifdef DEBUG
	int j;

	if(getenv("ALLPARMS"))
	{
		sysraw("qstatus: ");
		for(j = 0; j < argc; j++)
			sysraw("[%s]",argv[j]);
		sysraw("\n");
	}
#endif

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
        catd = catopen(MF_QSTAT, NL_CAT_LOCALE);

	/* check if the Palladium product is installed */
	if (!stat(PDENQ_PATH, &statbuf))
		palladium_inst = TRUE;

	/*----put ourselves into proper directory */
	cd(QUEDIR);

	/*----Read in all queues and devices */
	qlist = read_quedev();

	/*----Read in all command line arguments */
	read_args(argc,argv,qlist,&ourparms);

	/*----Display all the requested data */
	display_all(qlist,&ourparms);

	/*----Display the Palladium data */
	if (disp_palladium_q)
		display_palladium(argc,argv);

	return(exit_status);
}

/*==== READ IN THE MAIN DATA STRUCTURE FOR QUEUES, DEVICES */
struct q *read_quedev()
{
	struct q 	*readconfig(); 
	struct q	*ra_qlist;

	/*----Read in the queue config file into memory data structure */
	resetQCBFile();
	ra_qlist = readconfig(CDIGEST);
	return(ra_qlist);
}

/*==== READ IN ALL COMMAND LINE ARGUMENTS */
read_args(ra_argc,ra_argv,ra_qlist,ra_parms)
int		ra_argc;
char		**ra_argv;
struct q	*ra_qlist;
struct allparms	*ra_parms;
{
	int		ra_thisarg;		/* current retrieved argument */
	int		ra_jobnum;		/* retrieved job number */
        struct q	*ra_thisq;		/* temp q pointer */
	struct d	*ra_thisd;
	boolean		ra_qselected;		/* Indicates Queue selection on cmd line*/
	boolean		qflag=FALSE;
	boolean		pflag=FALSE;
	char		ra_msgbuf[MAXLINE];	/* Place to construct error messages */
	extern char 	*optarg;		/* start of flag's parameter in getopt */
	register struct q *ra_qptr;		/* pointer to non-aix queue */
	int 		loopval;

	/*----Init */
	ra_qselected = FALSE;
	ra_parms->ap_allqflag = FALSE;
	ra_parms->ap_loopmode = FALSE;
	ra_parms->ap_delay = 0;
        ra_parms->ap_verbose = FALSE;
	ra_parms->ap_sflag = FALSE;
	ra_parms->ap_queues = NULL;
	ra_parms->ap_jobs = NULL;
	ra_parms->ap_users = NULL;
	ra_parms->ap_only_local_jobs = FALSE;

	palarg[palcnt++] = PDENQ_PATH;
	palarg[palcnt++] = PAL_STATUS;

	/*----Retrieve all options from command line */
	while((ra_thisarg = getopt(ra_argc,ra_argv,args)) != EOF) {
		switch(ra_thisarg) {

		/*----Don't display the palladium queues */
		case ARGNOPQ: 
			disp_palladium_q = FALSE;
			break;

		/*----Show status for all queues */
		case ARGALLQ: 
			ra_parms->ap_allqflag = TRUE;
			ra_qselected = TRUE;
			aixq = TRUE;
			palarg[palcnt++] = scopy("-A\0");
			break;

		/*----Show status for the default queue */
		case ARGDEFQ:
			qflag=TRUE;
			palarg[palcnt++] = scopy("-q\0");
			break;

      case ARGSHRT:
			ra_parms->ap_sflag=TRUE;
			palarg[palcnt++] = scopy("-s\0");
			break;

		case ARGPRNT:
			/*----Add only valid queue names to linked list */
			pflag = TRUE;
			get_queue(optarg,ra_qlist,&ra_thisq,&ra_thisd);
			if(ra_thisq != NULL)
			{
				add_qptr(ra_thisq,ra_thisd,ra_parms);
				ra_qselected = TRUE;
				aixq = TRUE;
			}
			/* The queue name provided is not an AIX queue,  */
			/* but we have to add it to the linked list      */
			/* (if Palladium is installed on the system)     */
			/* because we can't tell if it's a Palladium     */
			/* logical printer or an invalid queue.          */
			else if(palladium_inst)
			{
				ra_qptr = Qalloc(sizeof(struct q));
				strcpy (ra_qptr->q_name, optarg);
				add_qptr(ra_qptr, NULL, ra_parms);
				ra_qselected = TRUE;
				palarg[palcnt++] = sconcat("-P\0",optarg,0);
	
			}
			else
			{
				syswarn(MSGSTR(MSGPRNT,
					"Invalid printer name: %s"),optarg);
				exit_status = ((int)EXITWARN);
			}
			break;

		/*----Show status for specific job number */
		case ARGJOBN:
			/*----Test for numeric job number */
			if((ra_jobnum = checkjobnum(optarg)) == -1)
			{
				systell(MSGSTR(MSGJOBN,QMSGJOBN),MINJOB,MAXJOB);
				usage();
			}

			/*----Valid job number, add it to linked list */
			add_jobnum(ra_jobnum,ra_parms);
			palarg[palcnt++] = sconcat("-#\0",optarg,0);
			break;

		/*----User name */
		case ARGUSER:
			add_user(optarg,ra_parms);
			palarg[palcnt++] = sconcat("-u\0",optarg,0);
			break;

		/*----Loop until queue empty mode */
		case ARGLOOP:
			ra_parms->ap_loopmode = TRUE;
			if((loopval = atoi(optarg)) <= 0)
				{
				systell(MSGSTR(MSGBADA,QMSGBADA),optarg);
				usage();
				}
			else
				{
				ra_parms->ap_delay = loopval;
				palarg[palcnt++] = sconcat("-w\0",optarg,0);
				break;
				}

		/*----Verbose mode */
		case ARGVERB:
			/*----Set the global parameter */
			ra_parms->ap_verbose = TRUE;
			palarg[palcnt++] = scopy("-L\0");
			break;

		/*----Print only the local jobs not attached to a device ----*/
		case ARGIRMJ:
			ra_parms->ap_only_local_jobs = TRUE;	
			break;

		/*----Error, user question */
		case '?':
			systell(MSGSTR(MSGBADO,QMSGBADO),ra_thisarg);	
			usage();

		} /* switch */	
	} /* while */

	if (ra_parms->ap_users && (ra_qselected == FALSE && !qflag)) {
		ra_parms->ap_allqflag = TRUE;
		ra_qselected = TRUE;
		aixq = TRUE;
	}
	/*----Display default queue if none selected */
	if (ra_qselected == FALSE)
	{
		default_queue(ra_qlist,&ra_thisq,&ra_thisd);
		add_qptr(ra_thisq,ra_thisd,ra_parms);
		if (aixq == TRUE)
			disp_palladium_q = FALSE;
	}
	if (ra_qselected && qflag && aixq)
		disp_palladium_q = FALSE;
	if (pflag && aixq && !(ra_parms->ap_allqflag))
		disp_palladium_q = FALSE;
	return(0);
}

/*==== DISPLAY ALL REQUESTED QUEUE DATA, LOOP IF REQUESTED */
display_all(da_qlist,da_parms)
struct q	*da_qlist;
struct allparms	*da_parms;
{
	int	da_pipe[2];
	boolean da_empty;
	int	da_waitstat = 0;

	/*----Ignore SIGPIPE */
	signal(SIGPIPE,SIG_IGN);

	do {
		pipe(da_pipe);
		if(fork())
		{	/*----PARENT: */
			/*----Get sleeping status from child */
			close(da_pipe[1]);
			if(read(da_pipe[0],&da_empty,sizeof(da_empty)) == -1)
				syserr((int)EXITFATAL,MSGSTR(MSGSFORK,"Fork failure."));
			if(read(da_pipe[0],&exit_status,sizeof(exit_status)) == -1)
				syserr((int)EXITFATAL,MSGSTR(MSGSFORK,"Fork failure."));
			close(da_pipe[0]);
			da_parms->ap_loopmode = da_parms->ap_loopmode && (!da_empty);

			/*----Wait for child to finish */
			wait(&da_waitstat);
			if(da_waitstat != 0){
#ifdef DEBUG
  				error_exit(da_waitstat);
#endif
				syserr((int)EXITFATAL,MSGSTR(MSGSFORK,"Fork failure."));
				}


			/*----Sleep for interval defined by user */
			if(da_parms->ap_loopmode == TRUE)
				sleep(da_parms->ap_delay);

		}
		else
		{	/*----CHILD: */
			close(da_pipe[0]);

			/*----Read in all desired jobs (see common.h) */
			if (!da_parms->ap_sflag || da_parms->ap_only_local_jobs )   
				read_jobs(da_parms,da_qlist);

			/*----Clear screen, Print the header lines for display output */
			display_header(da_parms);

			/*----Print status for selected jobs and queues */
			display_status(da_qlist,da_parms);

			/*----Check for stable (emptied) queue */
			stable_queue(da_qlist,&da_empty);

			/*----Send empty queue status back to parent */
			write(da_pipe[1],&da_empty,sizeof(da_empty));
			write(da_pipe[1],&exit_status,sizeof(exit_status));
			close(da_pipe[1]);

			qexit((int)EXITOK);
		}

	} while(da_parms->ap_loopmode == TRUE);
	return(0);
}

/*==== READ IN THE JOBS NEEDED */
read_jobs(rj_parms,rj_qlist)
struct allparms	*rj_parms;
struct q	*rj_qlist;
{
	register struct queptr *rj_thisqp;

	/*----If All flags set then read all jobs, all queues */
	if (rj_parms->ap_allqflag == TRUE)
		readqdir(NULL,rj_qlist,NULL);

	/*----Otherwise, read in only selected queues, jobs */
	else
		for (rj_thisqp = rj_parms->ap_queues;
		     rj_thisqp;
		     rj_thisqp = rj_thisqp->qp_next)
			readqdir(rj_thisqp->qp_qptr,rj_qlist,NULL);
	return(0);
}

/*==== DISPLAY THE HEADER */
display_header(dh_parms)
struct allparms	*dh_parms;
{
	if (aixq)
	{
		/*----Try to exec 'tput clear' to clear screen */
		if(dh_parms->ap_loopmode == TRUE)
		{
			if(fork())
				/*----PARENT: Wait for child to finish */
				wait(NULL);
			else
			{	/*----CHILD: Exec clear screen function, no error handling */
				execlp(TPUTPTH,TPUTPTH,"clear",0);
				qexit((int)EXITOK); /* just in case */
			}
		}
			
		/*----Display the header for output */
		if(dh_parms->ap_verbose == TRUE)
			display_vheader();
		else
			display_sheader();
		fflush(stdout);
		return(0);
	}
}

/*==== DISPLAY STATUS FOR SELECTED JOBS AND DEV:QUEUES */
display_status(ds_qlist,ds_parms)
struct q	*ds_qlist;
struct allparms	*ds_parms;
{
	register struct q	*ds_thisq;
	register struct d	*ds_thisd;
	register struct jobnum  *jd_thisjob;
	unsigned	ds_rank;

	/*----Display all que:devs as desired by user */
	for(ds_thisq = ds_qlist;
	    ds_thisq != NULL;
	    ds_thisq = ds_thisq->q_next)
	{
		/*
		 * Display local queue, or the local side of a remote queue.
		 */
		/*----Display each device with possible attached job */
		for(ds_thisd = ds_thisq->q_devlist,	ds_rank = 1;
		    ds_thisd != NULL;
		    ds_thisd = ds_thisd->d_next)
			if(queue_desired(ds_thisq,ds_thisd,ds_parms->ap_queues))
				display_quedev(ds_thisq,
					       ds_thisd,
					       ds_parms,
					       &ds_rank);

		/*----Print other jobs waiting to be run depending 
		      on the value of -i and -s flags */ 
		if (!ds_parms->ap_sflag || ds_parms->ap_only_local_jobs)
			print_remjobs(ds_thisq,ds_parms,&ds_rank);
		/*
		 * If queue has a remote side, display it.
		 */
		if(!ds_parms->ap_sflag && remote(ds_thisq->q_hostname))
		{
			if(queue_desired(ds_thisq,NULL,ds_parms->ap_queues)) {
				display_remstat(ds_thisq,ds_parms);
				fflush(stdout);
			}
				
		}
	}
        for(jd_thisjob = ds_parms->ap_jobs; jd_thisjob != NULL;
                 jd_thisjob = jd_thisjob->jn_next)
        {
                if (jd_thisjob->jn_found == FALSE)
                {
                        syswarn(MSGSTR(MSGJBID,
                                "Job %d not found -- perhaps it's done?"),
                                jd_thisjob->jn_num);
                        exit_status = ((int)EXITWARN);
                }
	}
	return(0);
}

/*==== DISPLAY THE STATUS OF A REMOTE QUEUE */
display_remstat(dr_queue,dr_parms)
struct q	*dr_queue;
struct allparms *dr_parms;
{
	int	dr_pipe[2];		/* Pipe for remote status text to child */
	int	dr_waitstat = 0;	/* Status returned from wait() call */
	int	dr_fresult;		/* Status from fork() call */
	int	dr_count;		/* Byte count from read() call */
	char	dr_msgbuf[MAXLINE];	/* Buffer for messages to user */
	char	*dr_thisfilt;		/* Name of remote qstatus filter to use if any */
	struct	jobnum *show_thisjob;	/* Job number of job to show status */
	struct	username *show_thisname;	/* User name of job to show */
	char	*shjb[PATH_MAX];	/* Array for rembak call */
	char	tmpjb[PATH_MAX][4];	/* Temp array for job number */
	int	r;
	int	tmpr;

	/*----Fork and exec */
	if(dr_fresult = fork())
	{
		/*----PARENT: */
		if(dr_fresult == -1)
			syserr((int)EXITFATAL,MSGSTR(MSGSFORK,"Fork failure."));

		/*----Wait for child */
		wait(&dr_waitstat);	
		if(dr_waitstat != 0){
#ifdef DEBUG
 			error_exit(dr_waitstat);
#endif   
			syserr((int)EXITFATAL,MSGSTR(MSGSFORK,"Fork failure."));
		}
	}
	else
	{
		/*----CHILD: */
		pipe(dr_pipe);
		if(dr_fresult = fork())
		{	
			/*----SUB-PARENT: */
			if(dr_fresult == -1)
				qexit((int)EXITBAD);

			/*----Redirect stdin from pipe */
			close(0);
			dup(dr_pipe[0]);
			close(dr_pipe[0]);
			close(dr_pipe[1]);

			
			/*----Determine filter to use, short or verbose */
			if(dr_parms->ap_verbose == TRUE)
				dr_thisfilt = dr_queue->q_lros;
			else
				dr_thisfilt = dr_queue->q_sros;


			/*----Exec the AWK shell script, if one exists */
			/*    Calling syntax:	filtname qname remquename */
			if(dr_thisfilt != NULL && dr_thisfilt[0] != '\0')
			{
				execlp(dr_thisfilt,
				       dr_thisfilt,	   /* Name of the filter (AWK script) */
				       dr_queue->q_name,   /* Name of the queue for filter */
				       dr_queue->q_rname,  /* rem name of q for dev */
				       0);

				/*----Problem execing filter if got to here */
				sprintf(dr_msgbuf,MSGSTR(MSGFRST,QMSGFRST),dr_thisfilt);
			}
			else
				sprintf(dr_msgbuf,MSGSTR(MSGBRST,QMSGBRST));

			/*----Print raw remote output from stdin:pipe with messages */
			printf("%s\n",dr_msgbuf);
			fflush(stdout);
			while((dr_count = read(0,dr_msgbuf,sizeof(dr_msgbuf))) > 0)
				write(1,dr_msgbuf,dr_count);
			printf("%s\n",MSGSTR(MSGERST,QMSGERST));
			fflush(stdout);
			qexit((int)EXITOK);
		}
		else
		{
			/* Build the rembak call command */
			r = 0;
			shjb[r++] = REM_BACKEND;
			shjb[r++] = "-P";
			shjb[r++] = dr_queue->q_rname;	/* Printer queue name */
			shjb[r++] = "-S";
			shjb[r++] = dr_queue->q_hostname;	/* Hostname */
			if(dr_parms->ap_verbose == FALSE)
				shjb[r++] = "-q";	/* Short Request */
			else
				shjb[r++] = "-L";	/* Long Request */
			/* Add the user names if there are any */
			for( show_thisname = dr_parms->ap_users;
 			     show_thisname != NULL && r <= PATH_MAX-2;
			     show_thisname = show_thisname->un_next ) {
				shjb[r++] = "-u";
				shjb[r++] = show_thisname->un_name;
			}
			tmpr = 0;
			/* Add the job numbers if there are any */
			for( show_thisjob = dr_parms->ap_jobs;
		  show_thisjob != NULL && r <= PATH_MAX-2 && tmpr <= PATH_MAX-2;
			     show_thisjob = show_thisjob->jn_next ) {
				shjb[r++] = "-#";
				sprintf(tmpjb[tmpr],"%d",show_thisjob->jn_num);
				shjb[r++] = tmpjb[tmpr];
				tmpr++;
			}
			shjb[r] = '\0';

			/*----SUB-CHILD:
			  ----Redirect stdout to pipe */
			close(1);
			dup(dr_pipe[1]);
			close(dr_pipe[1]);
			close(dr_pipe[0]);

                        /*----Exec rembak for this queue */
                        execvp(REM_BACKEND,shjb);

			/*----Problems if we get here */
			qexit((int)EXITBAD);
		}
	}
	return(0);
}

/*==== DISPLAY THE STATUS OF A PARTICULAR LOCAL DEV:QUEUE */
display_quedev(dq_queue,dq_dev,dq_parms,dq_rank)
struct q 	*dq_queue;
struct d	*dq_dev;
struct allparms *dq_parms;
unsigned	*dq_rank;
{
	struct e		*jobnum_to_ent();
	register struct e	*dq_thise;
	struct stfile		dq_statblk;

	if (getstatblk(stname(dq_dev),&dq_statblk))
	{
		dq_thise = jobnum_to_ent(dq_queue,dq_statblk.s_jobnum);
		display_line(dq_dev,
			     &dq_statblk,
			     dq_thise,
			     *dq_rank,
			     dq_parms);
		if(dq_thise != NULL)
		{
			(*dq_rank)++;
			dq_thise->e_jobnum = PRINTED;
		}
	}

	/*----Special case for no status file found */
	else
		display_line(dq_dev,
			     NULL,
			     NULL,
			     *dq_rank,
			     dq_parms);
	return(0);
}

/*==== PRINT REMAINING JOBS ON QUE NOT ATTACHED TO A DEV */
print_remjobs(pr_queue,pr_parms,pr_rank)
struct q	*pr_queue;
struct allparms	*pr_parms;
unsigned	*pr_rank;
{
	register struct e	*pr_thise;

	for (pr_thise = pr_queue->q_entlist;
	     pr_thise != NULL;
	     pr_thise = pr_thise->e_next)

		/*----Print the job's status line */
		if (pr_thise->e_jobnum != PRINTED)
		{
			if (job_desired(pr_thise,pr_parms->ap_jobs) &&
			    user_desired(pr_thise,pr_parms->ap_users))
				display_line(NULL,
					     NULL,
					     pr_thise,
					     *pr_rank,
					     pr_parms);
			(*pr_rank)++;
		}
	return(0);
}

/*==== DISPLAY THE STATUS OF A SPECIFIC JOB ON A QUEUE */
display_line(ds_dev,ds_statblk,ds_ent,ds_rank,ds_parms)
struct d	*ds_dev;
struct stfile	*ds_statblk;
struct e	*ds_ent;
unsigned	ds_rank;
struct allparms *ds_parms;
{
 	if(ds_parms->ap_verbose == TRUE)
		display_vline(ds_dev,ds_statblk,ds_ent,ds_rank);
	else
		display_sline(ds_dev,ds_statblk,ds_ent,ds_rank);
	fflush(stdout);
	return(0);
}

error_exit(status)
int status;
{
        int x;

        sysraw("the child exited with status=%d\n", (status>>8 & 0xff) );
        sysraw("the LSB of the status was=%x\n", status &0xff);
        /* Call the sys/wait.h macros to figure out waht happended ..*/

	if(status != 0) 
		sysraw("waitstat is %x errno = %d\n",status,errno);
	if (WEXITSTATUS(status))
		sysraw("wexitstatus is %d\n",WEXITSTATUS(status));
	if (WIFSIGNALED(status))
		sysraw("wifsignaled is %d\n",WIFSIGNALED(status));
	if (WTERMSIG(status))
		sysraw("wtermsig is %d\n",WTERMSIG(status));


        /* Then exit ... */
}

/*==== DISPLAY THE STATUS OF THE PALLADIUM QUEUES */
display_palladium(int argc, char **argv)
{

	int	status=0;	/* exit status of child */
	int	pid;			/* process id of child */
	int	retcode=0;	/* return code of waitpid */

	/* if the user doesn't want to display the Palladium queues or
	   Palladium is not installed, don't do anything. 
	 */
	if (!disp_palladium_q || !palladium_inst)
		return (0);	

	palarg[palcnt] = NULL;

	switch(pid = fork())
	{
		/* give an error if fork failed */
		case -1:
			syserr((int)EXITFATAL,MSGSTR(MSGSFORK,"Unable to fork a new process"));

		/* child exec the pdenq command */
		case 0:
			execvp(PDENQ_PATH,palarg);
			exit((int)EXITBAD);

		/* parent wait for child to finish and check status */
		default:
			while ((retcode = waitpid(pid, &status, 0)) == -1)
				if (errno != EINTR)
					break;
			if ((retcode == -1) || status)
				syserr((int)EXITFATAL,MSGSTR(MSGPAL1,
					"Unable to list another spooler's queues."));
			return(1);
	}
}
