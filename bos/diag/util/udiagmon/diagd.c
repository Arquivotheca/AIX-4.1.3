static char sccsid[] = "@(#)00	1.8  src/bos/diag/util/udiagmon/diagd.c, dsaudiagmon, bos41J, 9513A_all 3/17/95 10:47:38";
/*   COMPONENT_NAME: DSAUDIAGMON
 *
 *   FUNCTIONS: main, init_sigaction, is_dctrl_running, lock_self,
 *		do_tests, perform_the_test, diff_in_time, seach_next_entry,
 *		search_above_curr_min, search_bl_or_eq_curr_min,
 *		search_equalentry, sigalrm_handler, sigusr1_handler,
 *		multi_sighandler, handle_signals, read_CDiagDev_list,
 *		build_ptl, init_min_entry, store_ptl, add_entry, free_ptl,
 *		free_min_ptl, how_long_to_sleep, daemon_engine
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*  */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <odmi.h>
#include <locale.h>

#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include <diag/diagodm.h>
#include <diag/diag_define.h>
#include <diag/diag.h>
#include <diag/tmdefs.h>

#include "diagd.h"
#include "udiagmon_msg.h"

		      /* checks if PDiagAtt's value field = 1		      */
#define test_val(val) (((int)atoi(val) & SUPTESTS_PERIODIC_MODE)? TRUE:FALSE)
#define	DIAG_CONVERSION_KEY	"DIAGS_VERSION_4"

#ifdef  LOG_DATA
FILE	*G_fd;			 /* fd for '/tmp/diagd.log' DEBUG log file    */
static  char *S_dbg_fname = "/tmp/diagd.log";
#endif

static	char S_diag_dir[255];	 /* holds the path for 'diagela' test program */
static	char *S_test_pgm = "diagela"; 

struct	hrmin	 G_curr;	 /* holds the current time: hour and minutes  */
struct 	min_list *G_hrentry[24]; /* each entry hold  ptr to minutes-job btree */
                                 /* NULL entry => no job for this whole hour! */
struct 	min_list *G_curr_entry,  /* tests to be done right this minute        */
		 *G_next_entry;  /* tests to be done next 		      */

char 	G_lockfile[255];
int  	G_lockfd;

int	G_sigalrm_received; 	 /* remembers the receipt of sigalrm signal   */
int	G_sigusr1_received; 	 /* remembers the receipt of sigusr1 signal   */
int	G_sigusr1_handled; 	 /* remembers the number of sigusr1 signals   */

void	sigalrm_handler();
void	sigusr1_handler();

/*  */
/*
 * NAME: main
 *
 * FUNCTION: Creates and runs the Periodic Diagnostic Daemon
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

main()
{
	int	pfd[2];

	setlocale(LC_ALL, "");

	/* close stdin, stdout and stderr to prevent hang */
	/* from call like odm_run_method.		  */

	close(0);
	close(1);
	close(2);

	if (fork() == 0)
		daemon_engine();	/* child becomes the daemon */
		
	exit(0); 			/* parent exits */
}

/*  */
/*
 * NAME: init_sigaction
 *
 * FUNCTION: Register an action routine for a signal
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

init_sigaction(sig, sact, handler)
int 	sig;
struct  sigaction *sact;
void 	(handler)();
{
short 	i = sizeof(struct sigaction);
char 	*sp = (char *)sact;
	
	/* init sigaction structure */
	while(i--)	
		*(sp+i) = 0;

	sact->sa_handler = handler;
	sigaction(sig, sact, (struct sigaction *)NULL);

	DBGPR1("init_sigaction:Registering the handler for signal: %d\n", sig);
}
/*  */
/*
 * NAME: lock_self
 *
 * FUNCTION: creates a diagd lock file to avoid multiple sessions of diagd.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

lock_self()
{
char 	*lckdir;
int  	lockpid;

        /* get directory path for Diagd lock file */
        if((lckdir = (char *)getenv("DIAGDATADIR")) == NULL )
                lckdir = DIAGDATA;

	/* Disallow more than one copy of Diagd at a time. */
	sprintf(G_lockfile, "%s/diagd.lck", lckdir);

	for (; (G_lockfd = open(G_lockfile, O_CREAT | O_EXCL | O_WRONLY |
					O_NSHARE, 0600)) < 0;) 
	{
		char    buf[255];
		int 	fdes;

		fdes = diag_catopen(MF_UDIAGMON, 0);
		if (errno != EEXIST) /* lock file does not exist, then... */
		{
			/* print msg and exit */
#ifdef	LOG_DATA
			DBGPR(diag_cat_gets(fdes, DIAGD_SET, NO_DIAGD_LOCK));
#endif
			catclose(fdes);
			exit(-1);
		}

		/* G_lockfile exists, make sure locking process does */
		if ((G_lockfd = open(G_lockfile, O_RDWR)) > 0 &&
			read(G_lockfd, &lockpid, sizeof(lockpid)) ==
				sizeof(lockpid) && kill(lockpid, 0) < 0) 
		{

			/* locking process does not exist */
			catclose(fdes);
			break;
		}
		/* Another process is running with locking process' PID.
		Make sure G_lockfile isn't from a previous boot. */
		sprintf(buf,
			"%s -p%d | %s  diagd > /dev/null 2>&1",
							PS, lockpid, GREP);
		if (lockpid == getpid() || system(buf))
		{
			catclose(fdes);
			break;
		}

		/* print msg and exit */
#ifdef	LOG_DATA
		DBGPR1(diag_cat_gets(fdes, DIAGD_SET, NO_DIAGD), lockpid);
#endif
		catclose(fdes);
		
		if (G_lockfd > 0)
			close(G_lockfd);
		G_lockfd = -1;
		exit(-1);
	}
	/* create G_lockfile */
	lockpid = getpid();
	lseek(G_lockfd, 0, SEEK_SET);
	write(G_lockfd, &lockpid, sizeof(lockpid));
	close(G_lockfd);
}

/*  */
/*
 * NAME: do_tests
 *
 * FUNCTION: Create a child to test all the devices set for current minute.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

do_tests(curr_entry)  
struct 	min_list *curr_entry;
{
char 	**dn = curr_entry->devname;
short 	i;

#ifdef LOG_DATA
fflush(G_fd); /* flush before fork(), to avoid dup of data */
#endif
	
	/* Do the tests of all the devices from the current entry */
        if (fork() == 0)
        {	/* child */

		char 	**argsv;
		char	*ddir;

		/* get directory path for Diagela test program file */
		ddir = DEFAULT_UTILDIR;

		sprintf(S_diag_dir, "%s/%s", ddir, S_test_pgm);

		DBGPR1("do_tests: child running, pid: %d\n", getpid());

		argsv = (char **)calloc((3+(curr_entry->ndev)), sizeof(char *));
		argsv[0] = S_test_pgm;		/* 'diagela' program */
		argsv[1] = "-t";		/* diagtest option   */
		DBGPR1("do_tests: %s", argsv[0]);
		for (i=2; (i-2) < curr_entry->ndev; i++,dn++)
		{
			argsv[i] = *dn;
			DBGPR1(" %s", argsv[i]);
		}
		argsv[i] = NULL;
		DBGPR("\n");

		sleep(4); 	/* sleep so parent can wait() on this exit */

		execvp(S_diag_dir, argsv);

		exit(0);
        }
	/* parent */
        /* the Daemon will continue... */
}
/*  */
/*
 * NAME: diff_in_time
 *
 * FUNCTION: Finds the difference in seconds, between two time specifications.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: difference in seconds.
 */

unsigned int
diff_in_time(curr, next)
struct 	hrmin curr, 
	      next;
{
unsigned 	diffmin;

	/* Find the diff in time in seconds, in the whole list */

	DBGPR2("diff_in_time: curr.hour = %d, curr.minutes = %d\n",curr.hour,
								curr.minutes);
	DBGPR2("diff_in_time: next.hour = %d, next.minutes = %d\n",next.hour,
								next.minutes);
	if (next.hour > curr.hour)
		diffmin = (next.hour - curr.hour)*60;
	else
	if (next.hour < curr.hour)
		diffmin = (24 - curr.hour + next.hour)*60;
	else
	if (next.hour == curr.hour)
	{
		if (next.minutes <= curr.minutes)
			diffmin = 24*60;
		else
			diffmin = 0;
	}

	if (next.minutes > curr.minutes)
		diffmin += (next.minutes - curr.minutes);
	else
	if (next.minutes < curr.minutes)
		diffmin -= (curr.minutes - next.minutes);

	DBGPR1("diff_in_time: diffmin = %d\n", diffmin);

return diffmin*60;
}

/*  */
/*
 * NAME: search_next_entry
 *
 * FUNCTION:  Find the next higher entry in time, in the whole list. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

search_next_entry(hrentry, found)
struct 	min_list *hrentry;
int 	*found;
{
	if (hrentry->left)
		search_next_entry(hrentry->left, found);
	else
	{
		DBGPR2("search_next_entry: hour = %d, min = %d\n", 
				(hrentry->hm).hour, (hrentry->hm).minutes);

		G_next_entry = hrentry;
		*found = TRUE;
	}	
}

/*  */
/*
 * NAME: search_above_curr_min
 *
 * FUNCTION: Finds next higher entry in time, in the current hour list. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

search_above_curr_min(hrentry, found)
struct 	min_list *hrentry;
int 	*found;
{
short 	min; 	/* For the first time after build_ptl(), it may be NULL */

	if (hrentry->left)
		search_above_curr_min(hrentry->left, found);

	if (*found == FALSE)
	{ 
		min = (G_curr_entry)? (G_curr_entry->hm).minutes:G_curr.minutes;
		if (min < (hrentry->hm).minutes) 
		{
			DBGPR2("search_above_curr_min: hour = %d, min = %d\n", 
				(hrentry->hm).hour, (hrentry->hm).minutes);
			G_next_entry = hrentry;
			return *found = TRUE;
		}
	}

	if (hrentry->right)
		search_above_curr_min(hrentry->right, found);
}

/*  */
/*
 * NAME: search_bl_or_eq_curr_min
 *
 * FUNCTION: Find first lower entry in time, in the current hour list. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

search_bl_or_eq_curr_min(hrentry, found)
struct 	min_list *hrentry;
int 	*found;
{
short 	min;	/* For the first time after build_ptl(), it may be NULL */

	if (hrentry->left)
		search_bl_or_eq_curr_min(hrentry->left, found);

	if (*found == FALSE)
	{ 
		min = (G_curr_entry)? G_curr_entry->hm.minutes:G_curr.minutes;
 		if (min >= hrentry->hm.minutes)
		{
			DBGPR2("search_bl_or_eq_curr_min: hour= %d,min= %d\n", 
					hrentry->hm.hour, hrentry->hm.minutes);
			G_next_entry = hrentry;
			return *found = TRUE;
		}
	}	

	if (hrentry->right)
		search_bl_or_eq_curr_min(hrentry->right, found);
}

/*  */
/*
 * NAME: search_equalentry
 *
 * FUNCTION: Initial search for current hour:minute match. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

search_equalentry(hrentry)
struct 	min_list *hrentry;
{
	if (G_curr.minutes < (hrentry->hm).minutes)
	{
		if (hrentry->left)
			search_equalentry(hrentry->left);
	}
	else
	if (G_curr.minutes > (hrentry->hm).minutes)
	{
		if (hrentry->right)
			search_equalentry(hrentry->right);
	}
	else
	if (G_curr.minutes == (hrentry->hm).minutes)
		G_curr_entry = hrentry;
}

/*  */
/*
 * NAME: sigalrm_handler
 *
 * FUNCTION: Handler for SIGALARM signal 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

void 
sigalrm_handler()
{
struct 	sigaction sigalarm;

	G_sigalrm_received = TRUE;
	DBGPR("sigalrm_handler: SIGALRM\n");
} 

/*  */
/*
 * NAME: sigusr1_handler
 *
 * FUNCTION: Handler for SIGUSR1 signal.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

void 
sigusr1_handler()
{
struct 	sigaction sigusr1;

	DBGPR("sigusr1_handler: SIGUSR1\n");
	alarm(0);
	G_sigusr1_received = TRUE;/* may get another sigusr1 while build */
	G_sigusr1_handled++;	/* more than 1 sigusr1 will be noted */

	while(TRUE)
	{
		/* no sigusr1 will interrupt this build */
		if (G_sigusr1_handled == 1) 
		{
			build_ptl();	/* signal from SA - build new pt list */	
			/* process recent sigusr1 and build
			   recent modifications to pt list 
			*/
			if (G_sigusr1_handled > 1)  
			{
				G_sigusr1_handled = 1;
				continue;
			}
			else
				G_sigusr1_handled = 0;
		}
		break;
	}
}

/*  */
/*
 * NAME: multi_sighandler
 *
 * FUNCTION: Empty handler: just to avoid performing SIGDFL action. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

void
multi_sighandler(sig)
int	sig;
{
	/* empty handler: just to avoid performing SIGDFL action */
	DBGPR1("multi_sighandler:Received signal #: %d\n", sig);

	return;
}

/*  */
/*
 * NAME: handle_signals
 *
 * FUNCTION: Handle only signals mostly familiar to the process.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

handle_signals()
{
int	sig;
struct 	sigaction siga;

	for (sig=1; sig <= SIGMAX; sig++)
	{
		switch(sig)
		{
			case SIGUSR2:
			case SIGILL:
			case SIGQUIT:
			case SIGINT:
			case SIGABRT:
			case SIGTERM:
			case SIGHUP:
				init_sigaction(sig, &siga, multi_sighandler);
			break;
			default:
			break;
		}

	}
	DBGPR("handle_signals: done!\n");
}

/* ^L */
/*
 * NAME: get_iop_disk
 *
 * FUNCTION: Gets the ioplanar and disks which are supported for Periodic
 *	     Diagnostic testing.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURN: Ptr to the periodic test list(ptl).
 *
 *
 *
 */

struct ptl *
get_iop_disk(nents)
int	*nents;
{
struct	CuDv	 *ioplanar_cudv = NULL;
struct	listinfo c_info;
struct	CuDv	 *disk_cudv = NULL;
struct	listinfo d_info;
struct	ptl	 *t_list=NULL;


struct	PDiagAtt *iopa=NULL;	/* list of PDiagAtt for ioplanars	*/
struct	listinfo a_info;
char 	*iopa_done;	/* parallel with iopa, says matched Att and CuDv */

struct	PDiagAtt *diskb=NULL;	/* list of PDiagAtt for disks 		*/
struct	listinfo b_info;
char 	*diskb_done;	/* parallel with diskb, says matched Att and CuDv */
char 	pvdvln[128];	/* To store DC/DS/DType string from PDiagAtt 	*/

char	criteria[255];
int	i, j, index;


		/* Get the PDiagAtt of all ioplanars			*/
		sprintf(criteria, "%s", 
			"DClass=ioplanar AND attribute=test_mode");

		iopa = 
		   get_PDiagAtt_list(PDiagAtt_CLASS, criteria, &a_info, 1, 1);
		iopa_done = 
			(char *)calloc(a_info.num, sizeof(char));

		/* Get the PDiagAtt of all ioplanars			*/
		sprintf(criteria, "%s", 
			"DClass=disk AND attribute=test_mode");
		diskb = 
		   get_PDiagAtt_list(PDiagAtt_CLASS, criteria, &b_info, 1, 1);
		diskb_done =
			(char *)calloc(b_info.num, sizeof(char));

		/* No CDiagDev object class, Diagnostics has not been run  */
		/* Get all ioplanar and hdisk entries			   */

		sprintf(criteria, "%s", "PdDvLn LIKE ioplanar/*");
		ioplanar_cudv = get_CuDv_list(CuDv_CLASS, criteria, &c_info,
				1, 2);
		*nents=*nents + c_info.num;

		sprintf(criteria, "%s", "PdDvLn LIKE disk/*");
		disk_cudv = get_CuDv_list(CuDv_CLASS, criteria, &d_info,
				1, 2);
		*nents=*nents + d_info.num;

		if(*nents > 0)
			t_list = (struct ptl *)calloc
				(*nents,sizeof(struct ptl));

		/* Now update the list, with ioplanar and all disks.	     */	

		index=0;
		if (iopa != NULL)
		{
		DBGPR("PDiagAtt exists for ioplanar\n");
		for(i=0; i<c_info.num; i++){
			for (j=0; j < a_info.num; j++)
			{
				if (iopa_done[j] == FALSE)
					sprintf(pvdvln, "%s/%s/%s", 
							    iopa[j].DClass,
						    	    iopa[j].DSClass,
						    	    iopa[j].DType);
				else 
					continue;

				if( !strcmp(ioplanar_cudv[i].PdDvLn_Lvalue, 
					pvdvln) && (test_val(iopa[j].value)) )
				{
				/* device is supported for Periodic Diag test */
					strcpy(t_list[index].name, 
							ioplanar_cudv[i].name);
					t_list[index].periodic=
							DEFAULT_IOP_TESTTIME;
					index++;
				
					/* wont be checked for next iop */
					iopa_done[j] = TRUE; 
					break;	/* go to next ioplanar device */
				}
			}
		}
		} else {
		/* This is the case where the diagnostics portion for the */
		/* device has not yet been installed. By default, these   */
		/* devices need to be in the test list.			  */
		/* If these devices need to be removed from the test list */
		/* then the Service Aid needs to be used to delete them   */
		/* from the list.					  */
			DBGPR("No PDiagAtt for ioplanar\n");
			for (i=0; i< c_info.num; i++){
				DBGPR1("Putting %s in test list\n",
						ioplanar_cudv[i].name);
				strcpy(t_list[index].name, 
						ioplanar_cudv[i].name);
				t_list[index].periodic=
						DEFAULT_IOP_TESTTIME;
				index++;
			}
		}
		if (diskb != NULL)
		{
		DBGPR("PDiagAtt exists for disks\n");
		for(i=0; i<d_info.num; i++){
			for (j=0; j < b_info.num; j++)
			{
				if (diskb_done[j] == FALSE)
					sprintf(pvdvln, "%s/%s/%s", 
							    diskb[j].DClass,
						    	    diskb[j].DSClass,
						    	    diskb[j].DType);
				else 
					continue;

				if( !strcmp(disk_cudv[i].PdDvLn_Lvalue, 
					pvdvln) && (test_val(diskb[j].value)) )
				{
					/* device is supported for Periodic 
					   Diag test */
					strcpy(t_list[index].name, 
							disk_cudv[i].name);
					t_list[index].periodic=
							DEFAULT_DISK_TESTTIME;
					index++;
				
					/* wont be checked for next iop */
					diskb_done[j] = TRUE; 
					break;	/* go to next ioplanar device */
				}
			}
		}
		} else {
		/* This is the case where the diagnostics portion for the */
		/* device has not yet been installed. By default, these   */
		/* devices need to be in the test list.			  */
		/* If these devices need to be removed from the test list */
		/* then the Service Aid will be used to delete them       */
		/* from the list.					  */
			DBGPR("No PDiagAtt for disks\n");
			for (i=0; i< d_info.num; i++){
				DBGPR1("Putting %s in test list\n",
						disk_cudv[i].name);
				strcpy(t_list[index].name, 
						disk_cudv[i].name);
				t_list[index].periodic=
						DEFAULT_DISK_TESTTIME;
				index++;
			}
		}
		odm_free_list(ioplanar_cudv, &c_info);
		odm_free_list(disk_cudv, &d_info);
		odm_free_list(iopa, &a_info);
		odm_free_list(diskb, &b_info);
		if (iopa_done)
			free(iopa_done);
		if (diskb_done)
			free(diskb_done);
			
		*nents = index;		
		
return ((t_list)?(struct ptl *)realloc(t_list,(index*sizeof(struct ptl))):NULL);
}

/* 
 * NAME: read_CDiagDev_list
 *
 * FUNCTION: build the periodic test list.
 *	     Go through the CDiagDev object class and get all
 *	     entries whose Periodic field is valid (!= DEFAULT_TESTTIME).
 *	     If the CDiagDev object class is empty, i.e the 
 *	     diagnostic controller hasn't been invoked, then
 *	     the test list will default to the ioplanar, and 
 *	     all disks present in the system.
 *	
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: Pointer to test list.
 */

struct	ptl *
read_CDiagDev_list(num_entries)
int 	*num_entries;
{
struct	CDiagDev *T_CDiagDev = NULL;
struct	CDiagDev *cdiagdev;
struct	listinfo e_info;
struct	listinfo cdiag_info;
struct	ptl *tst_list=NULL;
char	criteria[255];
short   old_CDiagDev = 0;
int	i, index, num_cdiag;

	/* Determine version of CDiagDev currently on system.     */

        sprintf(criteria, "DType = %s", DIAG_CONVERSION_KEY);
	cdiagdev = (struct CDiagDev *)get_CDiagDev_list(CDiagDev_CLASS,
		criteria, &cdiag_info, 1, 1);
	if(cdiag_info.num == 0)
		old_CDiagDev=1;
	else
		odm_free_list(cdiagdev, &cdiag_info);

	/* Initialize the periodic test list by going through the */
	/* CDiagDev Object Class.				  */

	T_CDiagDev = get_CDiagDev_list(CDiagDev_CLASS, "", &e_info,
					MAX_EXPECT, 1);
	num_cdiag = e_info.num;
	*num_entries = 0;

	if (num_cdiag == 0)
		tst_list = get_iop_disk(num_entries);
	else 
	{
	
		/* check which entry has a periodic != DEFAULT_TESTTIME */
		for(index=0; index < num_cdiag; index++)
		{
			if(T_CDiagDev[index].Periodic != DEFAULT_TESTTIME) 
				(*num_entries)++;
		}

		if(*num_entries != 0)
		{
			/* Now allocate local structure */

			tst_list = (struct ptl *)calloc
				(*num_entries,sizeof(struct ptl));
			i=0;
			for(index=0; index < num_cdiag; index++)
			{
				if(T_CDiagDev[index].Periodic != 
							DEFAULT_TESTTIME)
				{
					char	criteria[128];
					struct 	CuDv	*cudv;
					struct	listinfo obj_info;

					/* Depending on the version of */
					/* CDiagDev, use name or type  */
					/* as a search criteria.       */

					if(old_CDiagDev)
						sprintf(criteria, 
							"location = %s AND PdDvLn LIKE *%s*", 
							T_CDiagDev[index].Location,
							T_CDiagDev[index].DType);
					else
						sprintf(criteria,
							"location = %s AND name = %s",
							T_CDiagDev[index].Location,
							T_CDiagDev[index].DType);
					cudv = get_CuDv_list(CuDv_CLASS, 
						criteria, &obj_info, 1, 1);

					if(cudv != NULL)
					{
						strcpy( tst_list[i].name,
								cudv->name);
						tst_list[i].periodic = 
						    T_CDiagDev[index].Periodic;
						i++;
						odm_free_list(cudv, &obj_info);
					}
				}
			}
		}
		odm_free_list(T_CDiagDev, &e_info);
	}
	return(tst_list);
}
/* ^L */
/*
 * NAME: build_ptl
 *
 * FUNCTION: Build the periodic test list either at SIGUSR1 signal or, 
 * 	     when the daemon starts for the first time. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

build_ptl()
{
struct 	sigaction sigalarm;
struct 	tm  *t;
struct 	ptl *ptl_list;
time_t 	cur_seconds;	/* current time in seconds! */

char 	ttime[5];
int 	i, fdes;
int 	nentries=0;


	/* reset the alarm, incase it is set! */
	alarm(0); 

	free_ptl();

	ptl_list = read_CDiagDev_list(&nentries);  /* get the whole odm list */

	/* store each entry in the binary minute tree */
	if (ptl_list)
	{
		for(i=0; i<nentries; i++)
		{
			sprintf(&ttime[0], "%04d", ptl_list[i].periodic);
			DBGPR2("build_ptl: dev: %s, ttime: %04d\n", 
					ptl_list[i].name, ptl_list[i].periodic);
			store_ptl(ptl_list[i].name, ttime);
		}
		free(ptl_list);
	}
	else
	{
		DBGPR("build_ptl: NULL ptl_list encountered!\n");

#ifdef	LOG_DATA
		if (G_fd > 0)	
			close(G_fd);
#endif
		if (G_lockfd > 0)
		{	
			close(G_lockfd);
			unlink(G_lockfile);
		}
		/* print msg and exit */
		fdes = diag_catopen(MF_UDIAGMON, 0);
#ifdef	LOG_DATA
		DBGPR(diag_cat_gets(fdes, DIAGD_SET, NO_DIAGD_RES));
#endif
		catclose(fdes);
		exit(-1);
	}
			
	/* set the curr.hour, curr.minutes */
	/* and get curr_entry ptr */

	/* get current time */
	cur_seconds = time((long *)0);
	t = localtime(&cur_seconds);

	/* set curr.hour, curr.minutes */
	G_curr.hour = t->tm_hour;
	G_curr.minutes = t->tm_min;

	DBGPR2("build_ptl: Current time: %d:%d\n", t->tm_hour,t->tm_min);

	G_curr_entry = NULL;
	if (G_hrentry[G_curr.hour] != NULL)
	{
		/* find a jobentry = curr.minutes */
		/* atleast there is a minute-job in list */
		/* search for curr.minutes */

		search_equalentry(G_hrentry[G_curr.hour]);  
	}

}

/*  */
/*
 * NAME: init_min_entry
 *
 * FUNCTION: Initialize the min_list structure for every new entry.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: pointer to the min_list structure
 */

struct min_list *
init_min_entry(t, dn)
unsigned int t[];
char *dn;
{
struct min_list *tminp;

	/* tminp = min_list structure with name, hour,minutes */

	tminp = (struct min_list *)calloc(1, sizeof(struct min_list));
	tminp->devname = (char **)calloc(MAX_DEV_ALLOC, sizeof(char *));
	tminp->left = tminp->right = NULL;

	(tminp->hm).hour = t[0];	
	(tminp->hm).minutes = t[1];	

	*(tminp->devname) = (char *)calloc(1, strlen(dn)+1);
	strcpy(*(tminp->devname), dn);
	tminp->ndev = 1;

return tminp;
}

/*  */
/*
 * NAME: store_ptl
 *
 * FUNCTION: Store the periodic test list in the memory. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

store_ptl(dname, periodic)
char *dname, *periodic;
{
struct	min_list *tminp;
struct 	hrmin hm;

unsigned tmp[2];

	/* get hour:minute from Periodic */
	sscanf(periodic, "%2d%2d", &tmp[0],&tmp[1]);
	
	if (G_hrentry[tmp[0]] == NULL)
		G_hrentry[tmp[0]] = init_min_entry(tmp, dname);
	else
		add_entry(G_hrentry[tmp[0]], tmp, dname);

}

/*  */
/*
 * NAME: add_entry
 *
 * FUNCTION: Add an entry to the  minute-binary tree list. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

add_entry(hrentry, tmp, dname)
struct 	min_list *hrentry;
char 	*dname;
unsigned int tmp[];
{
	if (tmp[1] < (hrentry->hm).minutes)
	{
		if (hrentry->left)
			add_entry(hrentry->left, tmp, dname);
		else
			hrentry->left = init_min_entry(tmp, dname);
	}

	if (tmp[1] == (hrentry->hm).minutes)
	{
		/* add the devname for this minute */
		if (hrentry->ndev%MAX_DEV_ALLOC == 0)
			hrentry->devname = (char **)realloc(hrentry->devname,
				sizeof(char *)*(hrentry->ndev+MAX_DEV_ALLOC));
		*((hrentry->devname)+(hrentry->ndev)) = 
					(char *)calloc(1, strlen(dname)+1);
		strcpy(*((hrentry->devname)+(hrentry->ndev)), dname);
		hrentry->ndev++;
	}

	if (tmp[1] > (hrentry->hm).minutes)
	{
		if (hrentry->right)
			add_entry(hrentry->right, tmp, dname);
		else
			hrentry->right = init_min_entry(tmp, dname);
	}
}

/*  */
/*
 * NAME: free_ptl
 *
 * FUNCTION: Free the whole periodic test list. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

free_ptl()
{
int i;

	for (i=0; i < HOURS; i++)
	{
		if (G_hrentry[i] != NULL)
		{
			DBGPR1("free_ptl: i = %d hours\n", i);
			free_min_ptl(G_hrentry[i]);
			G_hrentry[i] = NULL;
		}
	}
	G_curr_entry = G_next_entry = NULL;
	G_curr.hour = G_curr.minutes = 0;
}

/*  */
/*
 * NAME: free_min_ptl
 *
 * FUNCTION: Free just the minute-binary tree, given the hour. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

free_min_ptl(mlent)
struct 	min_list *mlent;
{
char 	**dp;
short 	i;


	if (mlent->left != NULL)
		free_min_ptl(mlent->left);

	if (mlent->right != NULL)
		free_min_ptl(mlent->right);

	DBGPR2("free_min_ptl: time: %d:%d\n", (mlent->hm).hour,
						(mlent->hm).minutes); 

	dp = mlent->devname;

	/* free each devname in mlent */
	for (i=0; i < mlent->ndev; i++)
	{
		DBGPR1("free_min_ptl: device: %s\n", *(dp+i));
		free(*(dp+i));
	}

	/* free device list in mlent */
	free(dp); 

	free(mlent);
}

/*  */
/*
 * NAME: how_long_to_sleep 
 *
 * FUNCTION: Find next entry containing the device(s) to test and the amount
 *	     of sleep-time. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: amount of sleep-time
 */

unsigned int 
how_long_to_sleep()
{
int no_entries;
int i, j;
unsigned found;
short hour;

	no_entries = TRUE;
	found = FALSE;

   	/* one cycle till you hit the same hrentry */
	/* for each hrentry starting from curr.hour, end in curr.hour */

	/* For the first time after build_ptl(), it may be NULL */
	hour = (G_curr_entry)? G_curr_entry->hm.hour:G_curr.hour;

	for (i=hour,j=1; (j <= HOURS+1); i=(i+1)%HOURS, j++)
	{
		/* if not NULL, there is atleast a minute-job in list */
		/* Then, find a jobentry closest > curr.minutes */

		if (G_hrentry[i] != NULL)
		{
			no_entries = FALSE;

			if (j == 1)		/* first time in loop */
				search_above_curr_min(G_hrentry[i], &found);
			else
				search_next_entry(G_hrentry[i], &found);  

			if (found)
				break;
			
		}
	}
	DBGPR1("how_long_to_sleep: j = %d\n", j);

	if (no_entries)    		/* curr_entry = next_entry = NULL */
		return (unsigned int)~0;/* infinite sleep time */

	/* find the diff in time: If only one entry present in whole list,
	   curr_entry = next_entry, sleep for 24 hours! 
	   curr_entry = NULL, next_entry valid, take curr.hour,curr.minutes 
	*/

	if (G_curr_entry == NULL) /* possible only the first time after build */
		return diff_in_time(G_curr, G_next_entry->hm);
	else
		return diff_in_time(G_curr_entry->hm, G_next_entry->hm);
	
}

/*  */
/*
 * NAME: daemon_engine
 *
 * FUNCTION: In an infinite loop, delegates the tests to be performed at 
 * 	     the current hour:minute and sleeps till the next one.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */

daemon_engine()
{
unsigned int sleep_time;
struct sigaction sigusr1, 
		 sigalarm;
int	status;
int fd, dev_fd;
struct stat stat_buf;


#ifdef LOG_DATA
	G_fd = fopen(S_dbg_fname, "w+a");
#endif

	lock_self();


	DBGPR1("lock_self: lockpid: %d\n", getpid());

	/* handle all signals except SIGALRM, SIGUSR1 */
	handle_signals();
	init_sigaction(SIGUSR1, &sigusr1, sigusr1_handler);
	init_sigaction(SIGALRM, &sigalarm, sigalrm_handler);

	odm_initialize();	/* check for error?? */

	build_ptl();		/* build periodic test list (ptl) */

	while(1)
	{
		if (G_curr_entry)  /* Only for first time, this may be null */
		{
			DBGPR2("daemon_engine: Curr entry: %d:%d\n", 
			   G_curr_entry->hm.hour, G_curr_entry->hm.minutes); 
			do_tests(G_curr_entry);
		}
		sleep_time = how_long_to_sleep();

		DBGPR1("Sleep Time: %d\n", sleep_time);

		alarm(sleep_time);
		G_sigusr1_received = FALSE;

#ifdef LOG_DATA
DBGPR("================Shhhhhh! The daemon is in sleep!===========\n");
fflush(G_fd); /* flush before fork(), to avoid dup of data */
#endif
		/* just wait for SOME time for do_tests() to finish the test! */
		if (wait(&status))	
		{
			if (G_sigusr1_received)
                        {
                                /* handler would have built pt list */
                                DBGPR("daemon_engine:G_sigusr1_received=1\n");

                                alarm(0);
                                G_sigusr1_received = FALSE;
                                continue;
                        }

                        if (G_sigalrm_received) /* time is out! get next job */
			{
				G_sigalrm_received = FALSE;
				G_curr_entry = G_next_entry;
                                continue;
			}

			/* any other signal; set alarm for remaining time */
			sleep_time = alarm(0);
			alarm(sleep_time);
		}
		while (pause())
		{
			if (G_sigusr1_received) 	
			{
				/* handler would have built pt list */

				DBGPR("daemon_engine:G_sigusr1_received=1\n");
				alarm(0);
				G_sigusr1_received = FALSE;
				break;
			}

			if (G_sigalrm_received) /* if (SIGALRM) go ahead */	
				break;

			/* if any other signal, it is ignored */
		}
		if (G_sigalrm_received)
		{
			G_sigalrm_received = FALSE;
			G_curr_entry = G_next_entry;
		}
	}
}
