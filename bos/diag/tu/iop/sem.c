static char sccsid[] = " @(#)71	1.2  src/bos/diag/tu/iop/sem.c, tu_iop, bos411, 9428A410j 4/14/94 10:38:15";
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: get_sem_tod, release_sem_tod
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <sys/mode.h>
#include <sys/sem.h>
#include <errno.h>

/*
 * Arbitrarily selected key for generating semaphore ID.
 */
#define TOD_SEMKEY		0x3141592
#define MAX_SEM_RETRIES		3

/*****************************************************************************

Function attempts to get COOPERATIVE access to TOD memory range via
IPC semaphore which SHOULD be used by all programs desiring the access.
User passes in "wait_time" to specify whether or not to "wait_forever"
for the semaphore or retry MAX_SEM_RETRIES with a wait interval of "wait_time".

IF successful
THEN RETURNs 0
ELSE RETURNs -1

*****************************************************************************/

int get_sem_tod (
	int			wait_time)
{
	int			semid;
	long			long_time;
	struct sembuf		sembuf_s;
	int			retry_count = (MAX_SEM_RETRIES - 1);
	int			rc;
	static int		sem_first_time = 1;

#ifdef SEMAPHORE_DEBUG
	printf("enter get_sem_tod()\n");
#endif /* SEMAPHORE_DEBUG */
	/*
	 * Obtain semaphore ID and create it if doesn't already exist.
	 */
	semid = semget(TOD_SEMKEY, 1, IPC_CREAT | IPC_EXCL |
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	
	if (semid < 0)
	{
		/*
		 * If the error from the semget() reveals that we failed
		 * for any reason other than the fact that the semaphore
		 * already existed, indicate error and return.
		 */
		if (errno != EEXIST)
			return(-1);
		
		/*
		 * Semaphore already exists.  Because it already exists
		 * it is possible that it was JUST created by another
		 * process so let's sleep here for a few clock ticks
		 * to let the other process initialize everything
		 * properly.
		 */
		if (sem_first_time)
		{
			sleep(4);
			sem_first_time = 0;
		}

		/*
		 * That should be enough time, so get the semaphore ID
		 * without CREATion flags.
		 */
		semid = semget(TOD_SEMKEY, 1, S_IRUSR | S_IWUSR |
						S_IRGRP | S_IWGRP);
		
		/*
		 * Make sure that we got a valid semaphore ID, else
		 * return error.
		 */
		if (semid < 0)
			return(-1);
	}
	else
	{
		/*
		 * Semaphore was newly created so we need to init
		 * our semaphore value.
		 */
		if (semctl(semid, 0, SETVAL, 1))
		{
			return(-1);
		}
	}

	/*
	 * At this point, we have our semaphore ID and if it was
	 * the first instance, it has been created and initialized
	 * for use.
	 */
	
	/*
	 * Indicate semaphore number (we're only dealing with one).
	 */
	sembuf_s.sem_num = 0;

	/*
	 * Set op to -1 indicating that we want to grab the
	 * semaphore.
	 */
	sembuf_s.sem_op = -1;

	/*
	 * If a non-negative wait_time was passed in, then indicate
	 * that we do not want the process to be blocked if the
	 * semaphore is unavailable.
	 *
	 * Note the SEM_UNDO flag.  By including this, the semaphore
	 * will get properly released should this process be terminated
	 * by a signal or something.
	 */
	if (wait_time >= 0)
		sembuf_s.sem_flg = IPC_NOWAIT | SEM_UNDO;
	else
		sembuf_s.sem_flg = SEM_UNDO;
	
	/*
	 * See if we can get the semaphore.  If the semaphore is
	 * available, then it has a value of 1 which will get decremented
	 * to a value of 0 since our sem_op = -1.  Else the semaphore is
	 * not available (thus a value of 0).
	 */
	while (retry_count > -1)
	{
		rc = semop(semid, &sembuf_s, 1);
		if (rc == 0)
		{
			/*
			 * Got it, so return.
			 */
#ifdef SEMAPHORE_DEBUG
	printf("exit get_sem_tod(0)\n");
#endif /* SEMAPHORE_DEBUG */
			return(0);
		}

		if (errno == EAGAIN)
		{
			/*
			 * Semaphore held by someone else, but
			 * we indicated not to wait forever.
			 */

			/*
			 * If user specified wait_time of zero,
			 * then just return unsuccessfully without
			 * retries.
			 */
			if (wait_time == 0)
				return(-1);

			/*
			 * Sleep for time specified by user and
			 * then go retry.
			 */
			sleep(wait_time);
			retry_count--;
		}
		else
			return(-1);
	}
	return(-1);
}

/*****************************************************************************

release_sem_tod

Function releases semaphore indicating completion of access to TOD
space.

IF successful
THEN RETURNs 0
ELSE RETURNs -1

*****************************************************************************/

int release_sem_tod ()
{
	int			semid;
	struct sembuf		sembuf_s;
	int			rc;
	int			pid;
	int			sempid;

#ifdef SEMAPHORE_DEBUG
	printf("enter release_sem_tod()\n");
#endif /* SEMAPHORE_DEBUG */
	/*
	 * Obtain the semaphore ID.
	 */
	semid = semget(TOD_SEMKEY, 1, S_IRUSR | S_IWUSR |
					S_IRGRP | S_IWGRP);
	
	/*
	 * Make sure that we got a valid semaphore ID, else
	 * return error.
	 */
	if (semid < 0)
		return(-1);
	
	/*
	 * Now, we want to make sure that we don't attempt to release
	 * the semaphore if we don't already have it.  This ensures
	 * that the semaphore value remains < 1, therefore binary.
	 */
	
	/*
	 * First, get the current process ID.
	 */
	pid = getpid();

	/*
	 * Next, get the process ID of the process which currently
	 * has the semaphore.
	 */
	sempid = semctl(semid, 0, GETPID, 0);
	if (sempid < 0)
		return(-1);
	
	/*
	 * If the current process ID does not equal the semaphore's
	 * process ID, then we're not holding it so return error.
	 */
	if (pid != sempid)
		return(-1);
	
	/*
	 * Release the semaphore by handing it a positive value (1)
	 * which gets added to the semaphore value (0) indicating
	 * that it is now available.
	 *
	 * Note the SEM_UNDO flag.  By including this, the semaphore
	 * will get properly handled should this process be terminated
	 * by a signal or something.
	 */
	sembuf_s.sem_num = 0;
	sembuf_s.sem_op = 1;
	sembuf_s.sem_flg = SEM_UNDO;
	rc = semop(semid, &sembuf_s, 1);
#ifdef SEMAPHORE_DEBUG
	printf("exit release_sem_tod(%d)\n", rc);
#endif /* SEMAPHORE_DEBUG */
	return(rc);
}
