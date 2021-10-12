static char sccsid[] = "@(#)89	1.13  src/bos/usr/ccs/lib/libodm/odmlock.c, libodm, bos411, 9428A410j 5/23/94 17:33:06";
/*
 * COMPONENT_NAME: (LIBODM) Object Data Manager library
 *
 * FUNCTIONS: pid_in_list, odm_lock, odm_unlock
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*------------------------------------------------------------*/
/* This program will use the fcntl() function and environment */
/* variables to allow child-processes to share their          */
/* parent's locks.                                            */
/*------------------------------------------------------------*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <odmi.h>
#include "odmtrace.h"

#define ODM_LOCK_VARIABLE "ODM_LOCK_IDS"
#define LOCK_AVAILABLE 0
#define LOCK_BLOCK 10

extern int errno;
int number_of_locks = 0;
int *lock_table = NULL;

/*
 * NAME:      odm_lock
 * FUNCTION:  Attempt to lock a token (file or directory) and wait a
 *            specified amount of time if the lock cannot be immediately
 *            granted.
 * RETURNS:   A lock id (positive number) if successful, -1 otherwise
 */
int odm_lock(token,timeout)
char *token; /* File or directory to lock                                    */
int timeout; /* WAIT=no wait, NOWAIT=wait forever, positive=#seconds to wait */
{
    struct flock lock_info;
    int returnstatus;
    int file_id;
    char environment_value[1024];      /* List of process ID's which hold lock */
    static char environment_variable[1024];
    long start_time;                   /* Beginning time.  Used for timeout */
    long current_time;
    int first_try;    /* Boolean indicating if one lock attempt has been made */
    int lock_attempts = 0;
    int index;
    mode_t old_mode;

    odmerrno = 0;

    first_try = TRUE;

    TRC("odm_lock","Timeout %d",timeout,"","");
    if (timeout < -1)
      {
        TRC("odm_lock","Invalid timeout %d",timeout,"","");
        odmerrno = ODMI_BAD_TIMEOUT;
        return(-1);
      } /* endif */

    lock_info.l_whence = 0;  /* Lock from the beginning of the file */
    lock_info.l_start  = 0;  /* Start the lock at byte 0            */
    lock_info.l_len    = 1;  /* Lock one byte                       */

    /* set the file mask to 000, umask() returns the current value of the file mask */
    old_mode = umask(0);

    file_id = open(token,O_RDONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);

    /* reset the file mask to the old one */
    umask(old_mode);

    TRC("odm_lock","Open file %s",token,"File id %d ",file_id);
    if (file_id < 0)
      {
        TRC("odm_lock","Cannot open file %d",errno,"","");
        odmerrno = ODMI_BAD_TOKEN;
        return(-1);
      } /* endif */

    /*---------------------------------------------------------------------------*/
    /*   1.  Attempt to get a read lock (F_RDLCK) on the token.  This will       */
    /*       block any other processes trying to lock the same token.            */
    /*                                                                           */
    /*   2.  While we have the read lock, find out if there is some other lock   */
    /*       which would prevent us from putting an exclusive lock (F_WRLCK) on  */
    /*       the token.                                                          */
    /*                                                                           */
    /*   3.  If there is no other process which would block the exclusive lock,  */
    /*       this process is the only process which has the token locked.  Set   */
    /*       the environment variable LOCK_IDS to the process id of this process,*/
    /*       grant the lock, and we are done.                                    */
    /*                                                                           */
    /*   4.  If there is some other process which would block the exclusive lock,*/
    /*       check the environment variable (LOCK_IDS) to determine if the       */
    /*       process id which would block the lock is in the LOCK_IDS list.      */
    /*                                                                           */
    /*   5.  If the process id is found, this is a child process of the process  */
    /*       which originally locked the token.  Add the process id of this      */
    /*       process to the environment variable LOCK_IDS, grant the lock,       */
    /*       and we are done.                                                    */
    /*                                                                           */
    /*   6.  If the process id is not found, this process is not in the same     */
    /*       chain of execution as the process which originally set the lock on  */
    /*       the token.  The lock is therefore denied.                           */
    /*---------------------------------------------------------------------------*/



    start_time = time((long *) 0);  /* Pass 0 so that current time is returned */

    while (first_try                                  ||
        timeout == ODM_WAIT                              ||
        timeout > (current_time = time( (long *) 0)) - start_time         )
      {
        if (!first_try)        /* Use 'sleep' between lock attempts so that */
            sleep(1);    /* another process can attempt the lock      */

        first_try = FALSE;
        lock_attempts++;

        /*----------------------------*/
        /* Step 1.  Get the read lock */
        /*----------------------------*/
        TRC("odm_lock","Performing read lock","","","");

        lock_info.l_type = F_RDLCK;

        returnstatus = fcntl(file_id,F_SETLK,&lock_info);


        if (returnstatus < 0)
          {
            /*-------------------------------------------------------------------*/
            /* The read lock has been denied.  This "normally" should not happen */
            /* but if there are too many locks, the file had been deleted, or    */
            /* a variety of other odd cases will make this occur.  For now we    */
            /* will not handle these error cases.                                */
            /*-------------------------------------------------------------------*/
            TRC("odm_lock","Read lock failed! %d",errno,"","");
            odmerrno = ODMI_BAD_LOCK;
            return(-1);
          } /* endif */

        /*----------------------------------------------------*/
        /* Step 2.  Find out if we can get an exclusive lock. */
        /*----------------------------------------------------*/
        TRC("odm_lock","calling GETLK fcntl","","","");
        lock_info.l_type = F_WRLCK;
        lock_info.l_sysid = 0;
        lock_info.l_pid = 0;

        returnstatus = fcntl(file_id,F_GETLK,&lock_info);
        if (returnstatus == -1)
          {
            TRC("odm_lock","GETLK fcntl failed! %d",errno,"","");
            close(file_id);
            odmerrno = ODMI_LOCK_BLOCKED;
            return(-1);
          } /* endif */

        if (lock_info.l_pid == 0 || /*l_pid is the id which would block the lock*/
        lock_info.l_type == F_UNLCK) /* Check the l_type also (13162) */
          {
            /*-----------------------------------------------------------------*/
            /* Step 3.  Grant the lock                                         */
            /*------------------------ ----------------------------------------*/
            /* The exclusive lock is not blocked.  This is the only process    */
            /* which has the token locked.  Set LOCK_IDS to this process id.   */
            /*-----------------------------------------------------------------*/
            TRC("odm_lock","Exclusive lock not blocked","","","");

            environment_variable[0] = '\0';
            strcpy(environment_variable,ODM_LOCK_VARIABLE);
            strcat(environment_variable,"=");

            returnstatus = putenv(environment_variable);
            if (returnstatus < 0)
              {
                TRC("odm_lock","Putenv failed ! %d",returnstatus,"","");
                odmerrno = ODMI_LOCK_ENV;
                close(file_id);
                return(-1);
              } /* endif */

            sprintf(environment_variable,"%s=%d:",ODM_LOCK_VARIABLE,getpid());
            TRC("odm_lock","Setting environment variable to %s\n",
                environment_variable,"","");

            /*---------------------------------------------------------------*/
            /* Set the environment variable to this process ID.  The         */
            /* putenv() routine is called twice so that the environment will */
            /* not try to use the automatic storage set up for               */
            /* 'environment_variable'.  See 'putenv' in IBM AIX Technical    */
            /* Reference v2.2.1 for more information.                        */
            /*---------------------------------------------------------------*/

            returnstatus = putenv(environment_variable);
            if (returnstatus < 0)
              {
                TRC("odm_lock","Putenv failed ! %d",returnstatus,"","");
                odmerrno = ODMI_LOCK_ENV;
                close(file_id);
                return(-1);
              } /* endif */

            returnstatus = add_lock_to_table(file_id);

            if (returnstatus < 0)
              {
                TRC("odm_lock","Could not add lock to table! odmerr %d",
                    odmerrno,"","");
                close(file_id);
                return(-1);
              } /* endif */

            return(file_id);
          } /* endif */

        /*---------------------------------------------------------------*/
        /* Step 4.  The lock was denied.  Check the environment variable */
        /*---------------------------------------------------------------*/
        strcpy(environment_value,getenv(ODM_LOCK_VARIABLE));
        TRC("odm_lock","Exclusive lock blocked by %d",lock_info.l_pid,
            "checking %s", environment_value);

        if (pid_in_list(lock_info.l_pid,environment_value))
          {
            /*------------------------------------------------------------------*/
            /* Step 5. Grant the lock since this is a child process of the      */
            /* process which set the lock.                                      */
            /*------------------------------------------------------------------*/
            TRC("odm_lock","Found id in environment list","","","");

            if (!pid_in_list(getpid(),environment_value))
              {
                /*------------------------------------------------------------*/
                /* Check to see if this process' process id is already in the */
                /* environment_value.  If it is not, add it to the list.      */
                /*------------------------------------------------------------*/
                sprintf(environment_variable,"%s=%s%d:",
                    ODM_LOCK_VARIABLE,environment_value,getpid());

                TRC("odm_lock","Setting environment variable %s",
                    environment_variable, "","");

                returnstatus = putenv(environment_variable);
                if (returnstatus < 0)
                  {
                    TRC("odm_lock","Add Putenv failed ! %d",returnstatus,
                        "","");
                    odmerrno = ODMI_LOCK_ENV;
                    close(file_id);
                    return(-1);
                  } /* endif */
              } /* endif */


            returnstatus = add_lock_to_table(file_id);

            if (returnstatus < 0)
              {
                TRC("odm_lock","Could not add lock to table! odmerr %d",
                    odmerrno,"","");
                close(file_id);
                return(-1);
              } /* endif */

            return(file_id);
          } /* endif */

        /*---------------------------------------------------------*/
        /* The lock was denied.  Unlock the F_RDLCK and try again. */
        /*---------------------------------------------------------*/
        lock_info.l_type = F_UNLCK;
        returnstatus = fcntl(file_id,F_SETLK,&lock_info);
        if (returnstatus == -1)
          {
            TRC("odm_lock","Lock denied unlock failed! %d",errno,"","");
            close(file_id);
            odmerrno = ODMI_UNLOCK;
            return(-1);
          } /* endif */

      } /* endwhile timeout < time - start_time */

    /*------------------------------------------------------------------*/
    /* Step 6.  Deny the lock since this process is not in the chain of */
    /* execution as the process which locked the token.                 */
    /*------------------------------------------------------------------*/


    TRC("odm_lock","Lock denied, tried %d",lock_attempts,"","");

    close(file_id);
    odmerrno = ODMI_LOCK_BLOCKED;
    return(-1);

} /* end of get_lock */

/*
 * NAME:     pid_in_list
 * FUNCTION: Determines if a process id is in the list of process id's.
 *           The process id list contains process id's separated by a colon.
 *           Example:
 *                 id_list = 123:456:7890
 * RETURNS:  TRUE if the pid is in the id_list, FALSE otherwise.
 */
int pid_in_list(pid,id_list)
int pid;       /* Process ID to search for.  */
char *id_list; /* Process ID list to look in */
{
    char *list_ptr;
    char *colon_ptr;



    list_ptr = id_list;

    while ((colon_ptr = strchr(list_ptr,':') ) != (char *) NULL)
      {
        if (pid == atoi(list_ptr))
          {
            /*----------------------------------------*/
            /* Found this process' id in the id list. */
            /*----------------------------------------*/
            return(TRUE);
          } /* endif */

        list_ptr = colon_ptr + 1;
      } /* endwhile */

    return(FALSE);
} /* end of my_id_in_list */

/*
 * NAME:     odm_unlock
 * FUNCTION: Unlocks a lock granted by the odm_lock() function.
 * NOTES:
 *     This function will unlock the ID which was passed back by the odm_lock()
 *     function.  The only error checking is to determine that the token_id
 *     is non-negative.
 * RETURNS: A 0 if successful, -1 otherwise.
 */
int odm_unlock(token_id)
int token_id;  /* The lock id passed back from odm_lock() */
{
    struct flock lock_info;       /* Lock info passed to fcntl */

    odmerrno = 0;
    TRC("odm_unlock","Token id %d",token_id,"","");
    if (token_id < 0 || remove_lock_from_table(token_id) < 0)
      {
        TRC("odm_unlock","Invalid token","","","");
        odmerrno = ODMI_LOCK_ID;
        return(-1);
      } /* endif */

    lock_info.l_whence = 0;         /* Unlock from the beginning of the file */
    lock_info.l_start  = 0;         /* Start the lock at byte 0              */
    lock_info.l_len    = 1;         /* Unlock one byte                       */
    lock_info.l_type   = F_UNLCK;   /* The unlock command                    */


    if (fcntl(token_id,F_SETLK,&lock_info)  < 0)
      {
        TRC("odm_unlock","Unlock failed! errno: %d",errno,"","");
        odmerrno = ODMI_UNLOCK;
        return(-1);
      } /* endif */


    close(token_id);

    return(0);
} /* end of odm_unlock */

/*
 * NAME:     add_lock_to_table
 * FUNCTION: Adds a lock ID to a table so ODM can keep track of locks it
 *           has granted. The lock table will never decrease in size even
 *           if locks are removed.
 * RETURNS: A 0 if successful, -1 otherwise.
 */
int add_lock_to_table(lock_id)
int lock_id;          /* The lock ID to save */
{
    register int available_slot;

    TRC("add_lock_to_table","Saving lock id %d",lock_id,"number_of_locks",
        number_of_locks);

    /*--------------------------------------------------------------------*/
    /* First check to see if there is an available slot in the lock table */
    /* to add this lock ID.                                               */
    /*--------------------------------------------------------------------*/
    for (available_slot = 0; available_slot < number_of_locks; available_slot++)
      {

        if ( lock_table[available_slot] == LOCK_AVAILABLE)
            break;

      } /* endfor */

    /*------------------------------*/
    /* Now see if a slot was found. */
    /*------------------------------*/
    if (number_of_locks == 0 || available_slot == number_of_locks)
      {
        TRC("add_lock_to_table","Allocating more space ","","","");
        /*---------------------------------------------------------------*/
        /* There was not enough room in the lock table to add this lock. */
        /* Allocate some (more) storage.                                 */
        /*---------------------------------------------------------------*/
        if (available_slot == 0)
          {
            /*-----------------------*/
            /* Need to malloc space. */
            /*-----------------------*/
            lock_table = (int *) malloc(LOCK_BLOCK * sizeof(int));
          }
        else
          {
            lock_table = (int *) realloc(lock_table,
                (LOCK_BLOCK + number_of_locks ) * sizeof (int) );
          } /* endif */

        if (lock_table == NULL)
          {
            /*----------------------------------------*/
            /* Could not allocate additional storage. */
            /*----------------------------------------*/
            TRC("add_lock_to_table","Could not allocate storage!","","","");
            number_of_locks = 0;
            odmerrno = ODMI_MALLOC_ERR;
            return(-1);
          } /* endif */

        memset((void *)(lock_table + number_of_locks),'\0',
            (size_t)(LOCK_BLOCK * sizeof(int)));

        /*-----------------------------------------------------------*/
        /* Increment the number_of_locks to indicate the size of the */
        /* current lock table                                        */
        /*-----------------------------------------------------------*/
        available_slot = number_of_locks;

        number_of_locks += LOCK_BLOCK;
      } /* endif */

    TRC("add_lock_to_table","available slot %d",available_slot,"","");

    lock_table[available_slot] = lock_id;
    return(0);

} /* end of add_lock_to_table */

/*
 * NAME:     remove_lock_from_table
 * FUNCTION: Removes a lock ID from  the lock table so ODM can make sure that
 *           a lock is freed only if it has been granted.
 * RETURNS: A 0 if successful, -1 otherwise.
 */
int remove_lock_from_table(lock_id)
int lock_id;         /* The lock id to remove from the lock table */
{
    register int index;

    TRC("remove_lock_from_table","Removing lock id %d",lock_id,
        "number_of_locks %d",number_of_locks);

    if (number_of_locks == 0)
      {
        /*----------------------------------------------------------------*/
        /* The user is trying to free a lock when no locks have been set. */
        /*----------------------------------------------------------------*/
        TRC("remove_lock_from_table","No locks set!","","","");
        odmerrno = ODMI_LOCK_ID;
        return(-1);
      } /* endif */

    /*---------------------------------------------------------------*/
    /* Go through the lock table and see if the given lock_id exits. */
    /*---------------------------------------------------------------*/
    for (index = 0; index < number_of_locks; index++)
      {
        if (lock_table[index] == lock_id)
            break;                             /* Found the lock ID */
      } /* endfor */

    if (index == number_of_locks )
      {
        TRC("remove_lock_from_table","Could not find lock in table!","","","");
        odmerrno = ODMI_LOCK_ID;
        return(-1);
      } /* endif */

    TRC("remove_lock_from_table","Found id in slot %d",index,"","");
    lock_table[index] = LOCK_AVAILABLE;
    return(0);
} /* end of remove_lock_from_table */




