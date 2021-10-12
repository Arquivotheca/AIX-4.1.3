static char sccsid[] = "@(#)91	1.6  src/bos/usr/ccs/lib/libs/userlicense.c, libs, bos41B, 412_41B_sync 1/4/95 14:46:10";
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: _FixedGetAllLicenses
 *		_FixedLicenseCount
 *		_FixedReleaseLicense
 *		_FixedRequestLicense
 *		_FloatingLogTimeout
 *		_FloatingRequestLicense
 *		_GetLicense
 *		_LLAddData
 *		_LLDestroy
 *		_LLInitialize
 *		_LoggedOn
 *		_ReleaseLicense
 *		_failure_handler
 *		_success_handler
 *		_timeout_handler
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/sem.h>
#include <usersec.h>	/* For SEC_INT */
#include <userconf.h>	/* For SC_MAXLOGINS */
#include <utmp.h>
#include <sys/file.h>
#include <sys/lockf.h>
#include <sys/mode.h>
#include <sys/ipc.h>
#include <sys/license.h>
#include <uuid.h>
#include <libnetls.h>


enum { NoLicense, FixedLicense, FloatingLicense };

/*
 * LicenseType is set to one of the 3 enum values above.  It is
 * then used to determine what license type to free in _ReleaseLicense().
 */
static int LicenseType = NoLicense;


/***************************************************************************/
/***************************** Fixed License *******************************/
/***************************************************************************/

/*
 * LICENSE_MAXIMUM should always be the largest number that a semaphore
 * can handle.
 */
#define	LICENSE_MINIMUM	2
#define	LICENSE_TIERTOP	128
#define	LICENSE_MAXIMUM	SHRT_MAX

#define	INIT_PROGRAM		"/usr/sbin/init"
/*
 * Defect 168205: The CHLICENSE_PROGRAM is used to path
 * the chlicense command.  If chlicense exists, then we 
 * assume this is a server machine and everything is normal.
 * Else, if  chlicense doesn't exist (stat() fails) we
 * assume that this is a client machine and hardcode the
 * license count to 2. 
 */
#define	CHLICENSE_PROGRAM	"/usr/bin/chlicense"

#undef	NODELOCK_LIC

#ifdef  NODELOCK_LIC

/*
 * The following code supports nodelock licenses.
 */

/*
 * INFO_SZ defines the size of the iFOR/LS fetch array.
 */
#define	INFO_SIZE	50

typedef	struct	nodelock_info
{
	nls_lic_annot_t	lic_annot [INFO_SIZE];
	nls_time_t	start_date[INFO_SIZE];
	nls_time_t	exp_date  [INFO_SIZE];
	nls_time_t	ts        [INFO_SIZE];
	long		prodid    [INFO_SIZE];
} NLInfo;


/*
 * Structures used to store non-dup license information.
 */
typedef	struct per_license_data
{
	nls_time_t	start;
	nls_time_t	expire;
	nls_time_t	ts;
	int		annot;
} PLData;

typedef	struct	license_list
{
	int	Allocated;	/* Total entries allocated for PLD. */
	int	Used;		/* Total entries used in PLD.       */
	PLData	*PLD;
} LicenseList;


/*
 * NAME:     _LLInitialize
 *
 * FUNCTION: Initializes a LicenseList structure.
 *
 * RETURNS:  Nothing.
 */
static	void
_LLInitialize(LicenseList *ll)
{
	ll->PLD = (PLData *) 0;
	ll->Allocated = ll->Used = 0;
}


/*
 * NAME:     _LLDestroy
 *
 * FUNCTION: Resets the values of a LicenseList structure.
 *
 * RETURNS:  Nothing.
 */
static	void
_LLDestroy(LicenseList *ll)
{
	if (ll->PLD) free(ll->PLD);
	_LLInitialize(ll);
}


/*
 * NAME:     _LLAddData
 *
 * FUNCTION: Adds a PLData entry to a LicenseList.  It extends the list
 *           if the list is full.
 *
 * RETURNS:   0 success
 *           -1 failure
 */
static	int
_LLAddData(LicenseList *ll, PLData *pld)
{
	PLData	*tmp;
	int	rc = 0;

	/*
	 * Increase the size of ll->PLD if the new pld won't fit.
	 */
	if (ll->Used >= ll->Allocated)
	{
		if (!ll->Allocated)
		{
			if (!(ll->PLD = (PLData *) malloc(
					INFO_SIZE * sizeof(PLData))))
			{
				rc = -1;
				goto fast_exit;
			}
		}
		else
		{
			if (!(tmp = (PLData *) realloc(ll->PLD,
				(ll->Allocated + INFO_SIZE) * sizeof(PLData))))
			{
				rc = -1;
				goto fast_exit;
			}
			ll->PLD = tmp;
		}
		ll->Allocated += INFO_SIZE;
	}

	memcpy(&ll->PLD[ ll->Used++ ], pld, sizeof(PLData));

fast_exit:
	return(rc);
}


/*
 * NAME:     _FixedGetAllLicenses
 *
 * FUNCTION: Get all fixed licenses.  If MaxLicense is a postive value,
 *           stop once MaxLicense is reached.
 *
 * NOTES:    Fetches information for the valid nodelocked licenses for this
 *           product into arrays in INFO_SIZE chunks removing duplicates as
 *           it goes.
 *
 * RETURNS:   0 or greater - The number of licenses.
 *           -1            - Failure.
 */
static	int
_FixedGetAllLicenses(int MaxLicenses)
{
	nls_vnd_id_t	vid = VENDOR_ID;
	nls_job_id_t	job_id;
	LicenseList	ll;
	PLData		pld;
	NLInfo		*nl = (NLInfo *) 0;
	struct stat	sbuf;
	long		nl_recs;   /* number of nodelocked files fetched */
	long		remaining;
	long		pntr = 0;
	long		status;
	long		i;
	int		j;
	int		Licenses = 0;

	_LLInitialize(&ll);

	/*
	 * If the monitord program does not exist, then ignore the
	 * nodelock file data.
	 * netls_init() will take 75 seconds to timeout if NODELOCK_FILE
	 * does not exist.
	 */
	if (stat(MONITORD_PROGRAM, &sbuf) || stat(NODELOCK_FILE, &sbuf))
		goto fast_exit;

	/*
	 * Initialize NetLS nodelock information.
	 */
	memset(&job_id, 0, sizeof(job_id));

	netls_init(vid, (long) VENDOR_KEY, &job_id, &status);

	if (status < 0L)
	{
		Licenses = -1;
		goto fast_exit;
	}

	/*
	 * Get memory for the netls_get_nodelock_info() data.
	 */
	if (!(nl = (NLInfo *) malloc(sizeof(NLInfo))))
	{
		Licenses = -1;
		goto fast_exit;
	}

	do
	{
		nl_recs = netls_get_nodelock_info(&job_id, (long) INFO_SIZE,
				&pntr, nl->prodid, nl->start_date,
				nl->exp_date, nl->ts, nl->lic_annot,
				&remaining, &status);

		if (status < 0L)
		{
			Licenses = -1;
			goto fast_exit;
		}

		/*
		 * For each entry in nl, if it is not already in ll->PLD,
		 * then add it to ll->PLD.
		 */
		for (i = 0L; i < nl_recs; i++)
		{
			/* Only AIX nodelock product IDs are to be counted */
			if (nl->prodid[i] != NL_PRODUCT_ID)
				continue;

			/* Convert to a pld */
			pld.start  = nl->start_date[i];
			pld.expire = nl->exp_date[i];
			pld.ts     = nl->ts[i];
			pld.annot  = atoi(&nl->lic_annot[i][sizeof("AIX ")-1]);

			/* Ignore negative license values. */
			if (pld.annot < 0)
				continue;

			for (j = 0; j < ll.Used; j++)
			{
				if (!memcmp(&pld, &ll.PLD[j], sizeof(PLData)))
					break;
			}

			if (j >= ll.Used)
			{
				if (_LLAddData(&ll, &pld))
				{
					Licenses = -1;
					goto fast_exit;
				}
				Licenses += pld.annot;

				/*
				 * If we don't need to know about any more
				 * licenses, then stop.
				 */
				if (Licenses >= MaxLicenses && MaxLicenses > 0)
					goto fast_exit;
			}
		}
	} while (pntr);

fast_exit:
	if (nl) free(nl);
	_LLDestroy(&ll);
	return(Licenses);
}


/*
 * NAME:     _FixedLicenseCount
 *
 * FUNCTION: Returns the number of fixed licenses available.
 *
 * RETURNS:  LICENSE_MINIMUM or greater (Number of licenses)
 */
int
_FixedLicenseCount(void)
{
	register	int	rc;
	register	int	Licenses = LICENSE_MINIMUM;

	if ((rc=_FixedGetAllLicenses(LICENSE_TIERTOP - LICENSE_MINIMUM)) >= 0)
	{
		Licenses += rc;
		if (Licenses >= LICENSE_TIERTOP)
			Licenses = LICENSE_MAXIMUM;
	}

	return(Licenses);
}

#else /* !NODELOCK_LIC */

/*
 * NAME:     _FixedLicenseCount
 *
 * FUNCTION: Returns the number of fixed licenses available.
 *
 * RETURNS:  maxlogins or greater (Number of licenses)
 */
int
_FixedLicenseCount(void)
{
	int	maxlogins;
	int	Licenses = LICENSE_MAXIMUM;

	if (!getconfattr("usw", SC_MAXLOGINS, &maxlogins, SEC_INT) &&
	    (maxlogins > 0 && maxlogins <= LICENSE_MAXIMUM))
	{
			Licenses = maxlogins;
	}

	return(Licenses);
}

#endif /* NODELOCK_LIC */


/*
 * NAME:     _FixedReleaseLicense
 *
 * FUNCTION: Releases a fixed license.
 *
 * RETURNS:  Nothing.
 */
static	void
_FixedReleaseLicense(void)
{
	key_t	key;
	struct	sembuf	sop = { 0, 1, (SEM_UNDO|IPC_NOWAIT) };
	int	semid;

	/*
	 * Create the semaphore key from the init program.
	 */
	if ((key = ftok(INIT_PROGRAM, 1)) != (key_t) -1)
		if ((semid = semget(key, 1, 0)) != -1)
			semop(semid, &sop, 1); /* Increment the semaphore */
}


/*
 * NAME:     _FixedRequestLicense
 *
 * FUNCTION: Obtains a fixed user license, if one is available.
 *
 * RETURNS:  1 on success
 *           0 on failure
 */
static	int
_FixedRequestLicense(void)
{
	key_t		key;
	struct	sembuf	sop     = { 0, -1, (SEM_UNDO|IPC_NOWAIT) };
	struct	sembuf	initsop = { 0,  0, IPC_NOWAIT };
	struct	stat	sbuf;
	int		semid;
	int		rc = 1;

	/*
	 * If we can't create a semaphore key from the init program,
	 * then allow the user access.
	 */
	if ((key = ftok(INIT_PROGRAM, 1)) == (key_t) -1)
		goto fast_exit;

	/*
	 * Create a semaphore ID.  If none existed, then initialize the
	 * semaphore.  If one existed, then call semget() again with no flags.
	 * This ordering is purposeful to prevent competing logins from
	 * conflicting.  (If semget() came before semget(CREAT), competing
	 * logins may both fail the first semget() and only 1 login would
	 * succeed on the semget(CREAT).  With the ordering below, they
	 * both should always succeed.)
	 */
	if ((semid = semget(key, 1, IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR)) != -1)
	{
#ifdef NODELOCK_LIC
#ifdef FUTURE
		int	maxlogins;
#endif /* FUTURE */
		initsop.sem_op = _FixedLicenseCount();
#ifdef FUTURE
		/*
		 * maxlogins is used to limit the number of fixed licenses
		 * the administrator wants to use on his/her system.  The
		 * system may have 100 licenses, but the administrator
		 * may only want 50 logged in at any given time.  This
		 * assumes floating licensing is disabled.
		 */
		if (!getconfattr("usw", SC_MAXLOGINS, &maxlogins, SEC_INT))
			if (maxlogins >= 0 && maxlogins < initsop.sem_op)
				initsop.sem_op = maxlogins;
#endif /* FUTURE */

		semop(semid, &initsop, 1);
#else /* !NODELOCK_LIC */

		int	maxlogins;

		/* Defect 168205: Check for existence of the
		 * chlicense command .  If
		 * the stat() fails (chlicense doesn't exist)
		 * assume this is a client machine and set the
		 * license count to LICENSE_MINIMUM (2)
		 * (see defect).
		 */
		if (stat(CHLICENSE_PROGRAM, &sbuf))  
		{
			initsop.sem_op = LICENSE_MINIMUM;
			semop(semid, &initsop, 1);
		}
		else
		{
			if (!getconfattr("usw", SC_MAXLOGINS, &maxlogins, SEC_INT))
			{
				initsop.sem_op =
				(maxlogins > 0 && maxlogins <= LICENSE_MAXIMUM)
					? maxlogins : LICENSE_MAXIMUM;
				semop(semid, &initsop, 1);
			}
			else
			{
				/*
				 * Couldn't get the maxlogins attribute, so
				 * remove the semaphore and perform no
				 * licensing checks.
				 */
				semctl(semid, 1, IPC_RMID, 0);
				goto fast_exit;
			}
		}
		
#endif /* NODELOCK_LIC */
	}
	else
	if ((semid = semget(key, 1, 0)) == -1)
	{
		/*
		 * If we can't access the semaphore, then allow the user access.
		 */
		goto fast_exit;
	}

	/*
	 * Decrement the semaphore counter and fail if it reaches zero.
	 */
	if (semop(semid, &sop, 1) != -1)
		LicenseType = FixedLicense;
	else
		rc = 0;	/* Fail */

fast_exit:
	return(rc);
}


/***************************************************************************/
/**************************** Floating License *****************************/
/***************************************************************************/


#define	NETLS_WAITING		2
#define	NETLS_SUCCESS		1
#define	NETLS_FAILURE		0
#define	NETLS_TIMEOUT		(-1)

#define	REQUEST_TIMEOUT		60

/*
 * This variable is volatile because the value will be changed by
 * interrupts.  Since the compiler may change the ordering of when
 * this variable is set with respect to other lines of code and I
 * don't want it to, I've declared it volatile to force the compiler
 * to keep the ordering as I've specified.
 */
static	volatile	int	lic_rc;


/*
 * NAME:     _success_handler
 *
 * FUNCTION: On SIGUSR1, sets lic_rc to NETLS_SUCCESS.
 *
 * RETURNS:  Nothing.
 */
static	void
_success_handler(void)
{
	lic_rc = NETLS_SUCCESS;
}


/*
 * NAME:     _failure_handler
 *
 * FUNCTION: On SIGUSR2, sets lic_rc to NETLS_FAILURE.
 *
 * RETURNS:  Nothing.
 */
static	void
_failure_handler(void)
{
	lic_rc = NETLS_FAILURE;
}


/*
 * NAME:     _timeout_handler
 *
 * FUNCTION: On SIGALRM, sets lic_rc to NETLS_TIMEOUT.
 *
 * RETURNS:  Nothing.
 */
static	void
_timeout_handler(void)
{
	/* Don't wipe out a good answer if we received one. */
	if (lic_rc == NETLS_WAITING)
		lic_rc = NETLS_TIMEOUT;
}


/*
 * NAME:     _FloatingLogTimeout
 *
 * FUNCTION: Log timeouts in the netls log file.
 *
 * RETURNS:  Nothing.
 */
static	void
_FloatingLogTimeout(void)
{
        char	log_msg[250];   /* err msg and some other logging information */
	int	log_msgl;       /* parameters for api calls                   */
	int	timeout_log;
	int	lockptr;
	time_t	now;

	/*
	 * Add the current time.
	 */
	time(&now);
	strcpy(log_msg, ctime(&now));
	log_msg[strlen(log_msg) - 1] = '\0';

	/*
	 * Add message.
	 */
	strcat(log_msg, "  ");
	strcat(log_msg, "RESPONSE FROM LICENSE SERVER TIMED OUT\n");
	strcat(log_msg, " : hardstop policy.  login denied\n");
	log_msgl= strlen(log_msg);

	/*
	 * If we can't open(), dup(), lseek(), lockf(), and/or write(),
	 * don't worry about it.  It's not the end of the world!
	 */
	timeout_log = open(NETLS_ERROR_LOG, O_WRONLY|O_CREAT|O_APPEND,
							S_IRUSR | S_IWUSR);
	if (timeout_log >= 0)
	{
		if ((lockptr = dup(timeout_log)) >= 0)
		{
			lseek(lockptr, 0L, SEEK_SET);
			lockf(lockptr, F_LOCK, 0L);
		}

		write(timeout_log, log_msg, log_msgl);

		if (lockptr >= 0)
		{
			lseek(lockptr, 0L, SEEK_SET);
			lockf(lockptr, F_ULOCK, 0L);
			close(lockptr);
		}
		close(timeout_log);
	}
}


/*
 * NAME:     _FloatingRequestLicense
 *
 * FUNCTION: Obtains a floating user license, if one is available.
 *
 * RETURNS:  1 on success (NETLS_SUCCESS).
 *           0 on failure (NETLS_FAILURE).
 */
static	int
_FloatingRequestLicense(void)
{
	struct	monitord_request client_request;
	struct	sigaction	 new_alarm, old_alarm;
	struct	sigaction	 new_user1, old_user1;
	struct	sigaction	 new_user2, old_user2;
	int	beforetime, aftertime;
	int	fd;

	lic_rc = NETLS_FAILURE;

	if ((fd = open(MONITORD_PIPE, O_RDWR|O_NONBLOCK, S_IRUSR|S_IWUSR)) >= 0)
	{
		/* Create the request data. */
		client_request.login_pid    = getpid();
		client_request.request_type = REQUEST_LIC;

		/*
		 * Setup signals SIGUSR1, SIGUSR2, and SIGALRM.
		 */
        	new_user1.sa_handler = _success_handler;
        	new_user1.sa_flags   = 0;
        	sigemptyset(&(new_user1.sa_mask));
        	sigaction(SIGUSR1, &new_user1, &old_user1);

        	new_user2.sa_handler = _failure_handler;
        	new_user2.sa_flags   = 0;
        	sigemptyset(&(new_user2.sa_mask));
        	sigaction(SIGUSR2, &new_user2, &old_user2);

        	new_alarm.sa_handler = _timeout_handler;
        	new_alarm.sa_flags   = 0;
        	sigemptyset(&(new_alarm.sa_mask));
        	sigaction(SIGALRM, &new_alarm, &old_alarm);

		/*
		 * Set lic_rc after the signals have been initialized,
		 * just in case erroneous signals change the value.
		 */
		lic_rc = NETLS_WAITING;

		/* Turn on the timer. */
		beforetime = alarm(REQUEST_TIMEOUT);

		/*
		 * Send the message to monitord and wait for response.
		 */
		write(fd, &client_request, sizeof(client_request));

		while (lic_rc == NETLS_WAITING)
			pause();

		/* Turn off the alarm. */
		aftertime = alarm(0);

		/*
		 * Reset the signals.
		 */
		sigaction(SIGUSR1, &old_user1, (struct sigaction *) 0);
		sigaction(SIGUSR2, &old_user2, (struct sigaction *) 0);
		sigaction(SIGALRM, &old_alarm, (struct sigaction *) 0);

		/*
		 * If there was a timer in progress before we set our timer,
		 * then restore the original timer and account for our usage.
		 */
		if (beforetime > 0)
		{
			aftertime = beforetime - (REQUEST_TIMEOUT - aftertime);
			if (aftertime <= 0) aftertime = 1;
			alarm(aftertime);
		}

		/*
		 * Handle timeouts.
		 */
		if (lic_rc == NETLS_TIMEOUT)
		{
			/* Send a release message. */
			client_request.request_type = RELEASE_LIC;
			write(fd, &client_request, sizeof(client_request));

			_FloatingLogTimeout();

			lic_rc = NETLS_FAILURE;
		}

		close(fd);
	}

	if (lic_rc == NETLS_SUCCESS)
		LicenseType = FloatingLicense;

	return(lic_rc);
}


/***************************************************************************/
/*************************** License Manager *******************************/
/***************************************************************************/

/*
 * NAME:    _LoggedOn
 *
 * FUNCTION: Determines if the user is already logged onto this device.
 *
 * RETURNS:  1 if user is     logged on.
 *           0 if user is not logged on.
 */
static	int
_LoggedOn(char *tty)
{
	struct	utmp	*ut;
	int		rc = 0;

	/*
	 * If the first process associated with this device is a
	 * USER_PROCESS, then no license is required.
	 */
	if (!strncmp(tty, "/dev/", 5))
		tty += 5;
	setutent();
	while (ut = getutent())
	{
		if (((ut->ut_type == LOGIN_PROCESS) ||
		     (ut->ut_type == USER_PROCESS)  ||
		     (ut->ut_type == INIT_PROCESS)) &&
		    !strncmp(ut->ut_line, tty, sizeof(ut->ut_line)))
		{
			if (ut->ut_type == USER_PROCESS)
				rc = 1;
			break;
		}
	}
	endutent();

	return(rc);
}


/*
 * NAME:     _GetLicense
 *
 * FUNCTION: Try to obtain a fixed license.  If that fails try to obtain
 *           a floating license.  If both fail return failure; otherwise
 *           return success.
 *
 * RETURNS:  1 on success.
 *           0 on failure.
 */
int
_GetLicense(char *tty, char *uname)
{
	struct	stat	sbuf;
	int	rc = 1;

	/*
	 * If we already have a license for this process, we don't need
	 * a new one.  This check prevents the same process from obtaining
	 * multiple licenses.
	 */
	if (LicenseType != NoLicense)
		goto fast_exit;


#ifdef MUTUALLY_EXCLUSIVE
#error Wasn't expecting MUTUALLY_EXCLUSIVE to be defined.
	/*
	 * Either FIXED or FLOAT, but NOT BOTH.
	 * o) The "root" user name does not require a license.
	 * o) If the user is already logged onto this device,
	 *    no license is needed.
	 * o) For non-root users, if the FLOATING_ON file does not exist,
	 *    then obtain a fixed license.
	 * o) Else obtain a floating license.
	 */
	if (strcmp(uname, "root") && !_LoggedOn(tty))
		rc = (stat(FLOATING_ON, &sbuf)) ? _FixedRequestLicense()
						: _FloatingRequestLicense();
#else
	/*
	 * FIXED and/or FLOAT (ROLLOVER).
	 * o) The "root" user name does not require a license.
	 * o) If the user is already logged onto this device,
	 *    no license is needed.
	 * o) For non-root users, if a fixed license is available
	 *    then succeed.
	 * o) Else, if the FLOATING_ON file does exist and a floating
	 *    license exists, then succeed.
	 * o) If either the FLOATING_ON file does not exist or a floating
	 *    license does not exist, then fail.
	 */
	if (strcmp(uname, "root") && !_LoggedOn(tty) &&
					!_FixedRequestLicense() &&
	   (stat(FLOATING_ON, &sbuf) || !_FloatingRequestLicense()) )
		rc = 0;
#endif /* MUTUALLY_EXCLUSIVE */

fast_exit:
	return(rc);
}


/*
 * NAME:     _ReleaseLicense
 *
 * FUNCTION: Releases a user license, if one was obtained by this process.
 *
 * RETURNS:  Nothing
 */
void
_ReleaseLicense(void)
{
	if (LicenseType == FixedLicense)
		_FixedReleaseLicense();
	else
	if (LicenseType == FloatingLicense)
		_FloatingReleaseLicense(getpid());

	LicenseType = NoLicense;
}
