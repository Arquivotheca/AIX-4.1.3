static char sccsid[] = "@(#)22  1.1  src/bos/diag/tu/ppckbd/sem.c, tu_ppckbd, bos41J, 9520A_all 5/6/95 14:32:36";
/*
 *
 * COMPONENT_NAME: tu_ppckbd 
 *
 * FUNCTIONS:   set_sem, rel_sem
 *              
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <time.h>
#include <sys/sem.h>

#include "tu_type.h"

/***************************************************************************/
/* FUNCTION NAME: set_sem(wait_time)

   DESCRIPTION: This function access the POS register 2 via the IPC
		semaphore which should be used by all programs desiring
		the access. The user passes in a "wait_time" to specify
		whether or not to "wait_forever" for the semaphore or
		retry MAX_SEM_RETRIES with a wait_interval of "wait_time" 

   NOTES: 	This function is used basically to overcome the concurrent 
		access of the POS register 2 by the Keyboard/Tablet,
		Diskette, Serial Port 1 and 2, Parallel Port and Mouse
		devices. This function is used only in NON INTERACTIVE
		TUs and in this case TU 01, 02, 03, 04, 05.      

***************************************************************************/

int set_sem(wait_time)
int wait_time;
{
	int 		semid;
        long 		long_time;
	struct sembuf	sembuf_s;
	int		retry_count = (MAX_SEM_RETRIES -1);
	int 		rc;
	static		first_time = 1;

	/* Obtain semaphore ID and create it if it does not already
	 *  exist */

	semid = semget(SEMKEY, 1, IPC_CREAT | IPC_EXCL | S_IRUSR |
			S_IWUSR | S_IRGRP | S_IWGRP);

	if (semid < 0)
	{

	  /* If the error from semget() reveals that we failed for any
	   * reason other than the fact that the semaphore already
	   * existed, indicate error and return */
	
	   if (errno != EEXIST)
		return(SYS_SEMID_NEXST);

	  /* Semaphore already exists. Because it already exists it is
	   * possible that it was JUST created by another process so
	   * let's sleep here for a few clock cycles to let the other
	   * process initialize everything properly */

	   if (first_time)
	   {
		sleep(4);
		first_time = 0;
	   }

	/* That should be enough time, so get the semaphore ID without
	 * CREATion flags */

	semid = semget(SEMKEY, 1, S_IRUSR | S_IWUSR | S_IRGRP |
			S_IWGRP);

	/* Make sure that we got a valid semaphore ID, else return error */
	
	if (semid < 0)
	    return(SYS_SEMID_NVALID);
      }
	else
      {
	/* Semaphore was nearly created so we need to initialize our
	 * semaphore value */

	if (semctl(semid, 0, SETVAL, 1))
	{
		return(SYS_SEMVAL_NVALID);
	}
      }

	/* At this point, we have our semaphore ID and is it was the
	 * first instance, it had been created and initialized for
	 * use */
	  
	/* Indicate semaphore number */

	sembuf_s.sem_num = 0;

	/* Set op to -1 indicating that we want to grab the semaphore */

	sembuf_s.sem_op = -1;

	/* If a non-negative wait_time was passed in, then indicate that
	 * we do not want the process to be blocked if the semaphore is
	 * unavailable. Note the SEM_UNDO flag. By including this, the
	 * semaphore will get properly released should this process be
	 * terminated by a signal or something */

	if (wait_time >= 0)
		sembuf_s.sem_flg = IPC_NOWAIT | SEM_UNDO;
	else
		sembuf_s.sem_flg = SEM_UNDO;

	/* See if we can get the semaphore. If the semaphore is available
	 * then it has a value of 1 which will get decremented to a value
	 * of 0 since our sem_op = -1. Else the semaphore is not available
	 * (thus a value of 0).*/

	while (retry_count > -1)
	{
		rc = semop(semid, &sembuf_s, 1);
		if (rc == 0)
		{
		  /* Got it, so return */
		
		  return(SUCCESS);
		}
	    if (errno == EAGAIN)
		{
	/* Semaphore held by someone else, but we indicated not to wait
	 * forever. If user specified wait_time of zero, then just
	 * return unsuccessfully without retries */	

 			if (wait_time == 0)
	     		return(SYS_SPND_SEM_OP);

	/* Sleep for the time specified by the user and then retry */	
	
			sleep(wait_time);
			retry_count --;
       		}
		else
	     		return(SYS_SPND_SEM_OP);
	}
	 return(SYS_SPND_SEM_OP);
}
 
/***************************************************************************/
/* FUNCTION NAME: rel_sem()

   DESCRIPTION: This function releases the semaphore indicating
		completion of access to POS register 2 by that particular
		device.
   NOTES:	This function is used only by NON INTERACTIVE TUs.  

***************************************************************************/

int rel_sem()
{
	int 		semid;
	struct sembuf	sembuf_s;
	int		pid;
	int 		rc;
	int		sempid;

	/* Obtain semaphore ID and create it if it does not already
	 *  exist */

	semid = semget(SEMKEY, 1, S_IRUSR |  S_IWUSR | S_IRGRP |
			 S_IWGRP);

	/* Make sure that we got a valid semaphore ID, else return
	 * error */

	if (semid < 0)
	   return(SYS_SEMID_NVALID);

	/* Now, we want to make sure that we do not attempt to release
	 * the semaphore if we don't already have it. This ensures that
	 * the semaphore value remains < 1, therefore binary */

	/* First, get the current process ID */

	pid = getpid();

	/* Next, get the process ID of the process which currently has
	 * the semaphore */

	sempid = semctl(semid, 0, GETPID, 0);
	if (sempid < 0)
	   return(SYS_PID_NVALID);

	/* If the current process ID does not equal the semaphore's 
	 * process ID, then we are not holding it so return an error */

	if (pid != sempid)
	   return(SYS_PID_NVALID);

	/* Release the semaphore by handing it a positive value which
	 * get added to the semaphore value indicating that it is now
	 * available. Note the SEM_UNDO flag. By including this, the
	 * semaphore will get properly handled should this process be
	 * terminated by a signal or something. */

	sembuf_s.sem_num = 0;
	sembuf_s.sem_op = 1;
	sembuf_s.sem_flg = SEM_UNDO;
	rc = semop(semid, &sembuf_s, 1);

	return(rc);
}

