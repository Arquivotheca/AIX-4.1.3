static char sccsid[] = "@(#)70  1.2  src/bos/diag/tu/siosl/mousea/mou_io.c, cmddiag, bos41J, 9515A_all 4/7/95 11:04:58";
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: 
 *		get_machine_model_tum
 *		set_semm
 *		rel_semm
 *		setup_mouse
 *		rd_bytem
 *		rd_posm
 *		rd_wordm
 *		wr_2bytem
 *		wr_bytem
 *		wr_posm
 *		wr_wordm
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <diag/atu.h>
#include "tu_type.h"

/* For use in reading ipl control block info for machine model */

#include <sys/iplcb.h>
#include <sys/rosinfo.h>

uint semaflag = 0;      /* Used in set_sem and rel_sem - if true, this process
                           has obtained the kbd/mse/tab shared semaphore */

 /* Global masks for blocking and restoring signals */
int block_mask = SIGINT | SIGTERM;
int old_mask;

/* The following are defines for use in the get_machine_model_tum routine */

#define SAL1_MODEL                   0x41   /* SAL1_MODEL (rosinfo.h)*/
#define SAL2_MODEL                   0x45   /* SAL2_MODEL (rosinfo.h)*/
#define CAB_MODEL                   0x43   /* CAB_MODEL (rosinfo.h)*/
#define CHAP_MODEL                  0x47   /* CHAP_MODEL (rosinfo.h)*/
#define RBW_MODEL                   0x46   /* RBW_MODEL (rosinfo.h)*/
#define INVALID_MACHINE_MODEL       -1

/* The function below is called by ?exectu() routine */
/* Using the machine DD, this routine returns the appropriate mask data so
   the fuse/polyswitch status bit can be checked.  Also, it places which
   type of machine is being used in mach_info global structure.  Both machine 
   and mask data are contained in this structure. */

int get_machine_model_tum(int mdd_fdes)
{
  MACH_DD_IO    mdd;
  IPL_DIRECTORY *iplcb_dir;                                       /* iplcb.h */
  IPL_INFO      *iplcb_info;
  int rc,  machine_model;

  rc = SUCCESS;

    iplcb_dir = (IPL_DIRECTORY *) malloc (sizeof(IPL_DIRECTORY));
    iplcb_info = (IPL_INFO *) malloc (sizeof(IPL_INFO));
    if ( (iplcb_dir == ((IPL_DIRECTORY *) NULL))||
         (iplcb_info == ((IPL_INFO *) NULL)) ) {
        rc = INVALID_MACHINE_MODEL;
    } else {
        /* Obtain pointer to ROS directory */
        mdd.md_incr = MV_BYTE;
        mdd.md_addr = 128;
        mdd.md_data = (char *) iplcb_dir;
        mdd.md_size = sizeof(*iplcb_dir);
        if ( ioctl(mdd_fdes, MIOIPLCB, &mdd) ) {
          rc = INVALID_MACHINE_MODEL;
        } else {
           /* Obtain pointer to ipl_info_offset */
            mdd.md_incr = MV_BYTE;
            mdd.md_addr = iplcb_dir -> ipl_info_offset;
            mdd.md_data = (char *) iplcb_info;
            mdd.md_size = sizeof(*iplcb_info);
            if ( ioctl(mdd_fdes, MIOIPLCB, &mdd) ) {
              rc = INVALID_MACHINE_MODEL;
            } else {

/* ***************************************************************************
 *  The model field contains information which allows software to determine  *
 *  hardware type, data cache size, and instruction cache size.              *
 *                                                                           *
 *  The model field is decoded as follows:                                   *
 *        0xWWXXYYZZ                                                         *
 *                                                                           *
 *  case 2: WW is nonzero.                                                   *
 *          WW = 0x01 This means that the hardware is SGR ss32 or SGR ss64   *
 *                    (ss is speed in MH).                                   *
 *          WW = 0x02 means the hardware is RSC.                             *
 *          XX has the following bit definitions:                            *
 *                  bits 0 & 1 (low order bits) - indicate package type      *
 *                        00 = Tower     01 = Desktop                        *
 *                        10 = Rack      11 = Reserved                       *
 *                  bits 2 through 7 are reserved.                           *
 *          YY = reserved.                                                   *
 *          ZZ = the model code:                                             *
 *                  0x45  : SGA model                                        *
 *                  0x41  : RGA model                                        *
 *                                                                           *
 *          The instruction cache K byte size is obtained from entry icache. *
 *          The data cache K byte size is obtained from entry dcache.        *
 *                                                                           *
 *  refer to '/usr/include/sys/rosinfo.h' for more information.              *
 *************************************************************************** */

               machine_model = iplcb_info -> model & 0xff;  /* retain ZZ fld*/
               mach_info.mach_model = machine_model;

	       /* Put contents of 1st byte into mach_type field of structure */
	       mach_info.mach_type = (iplcb_info->model >> 24) & 0xff;

               if ((machine_model == CAB_MODEL) ||
                   (machine_model == CHAP_MODEL))
                {
                  mach_info.switch_stat_mask = 0x20;
                }
               else {
                  mach_info.switch_stat_mask = 0x10;
               }
               if ( (machine_model == SAL1_MODEL) || (machine_model == SAL2_MODEL) || (machine_model == CAB_MODEL) || (machine_model == CHAP_MODEL) )
               {
                  mach_info.machine = RSC;
               }
               else {
                  mach_info.machine = POWER;
               }

            } /* endif */
          } /* endif */

       } /* endif */

      free (iplcb_dir);
      free (iplcb_info);

  return (rc);

}

#define MAX_SEM_RETRIES 5
#define POS_SEMKEY      0x3141592

/***************************************************************************
 FUNCTION NAME: set_semm(wait_time)

   DESCRIPTION: This function access the POS register 2 via the IPC
                semaphore which should be used by all programs desiring
                the access. The user passes in a "wait_time" to specify
                whether or not to "wait_forever" for the semaphore or
                retry MAX_SEM_RETRIES with a wait_interval of "wait_time"

   NOTES:       This function is used basically to overcome the concurrent
                access of the POS register 2 by the Keyboard/Tablet,
                Diskette, Serial Port 1 and 2, Parallel Port and Mouse
                devices.

***************************************************************************/

int set_semm(int wait_time)
{
        int             semid;
        long            long_time;
        struct sembuf   sembuf_s;
        int             retry_count = (MAX_SEM_RETRIES -1);
        int             rc;
        static          first_time = 1;

        /* Obtain semaphore ID and create it if it does not already
         *  exist */

        semid = semget( (key_t) POS_SEMKEY, 1, IPC_CREAT | IPC_EXCL | S_IRUSR |
                        S_IWUSR | S_IRGRP | S_IWGRP);

        if (semid < 0)
        {
          /* If the error from semget() reveals that we failed for any
           * reason other than the fact that the semaphore already
           * existed, indicate error and return */

           if (errno != EEXIST)
                return(201);

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

        semid = semget( (key_t) POS_SEMKEY, 1, S_IRUSR | S_IWUSR | S_IRGRP |
                        S_IWGRP);

        /* Make sure that we got a valid semaphore ID, else return error */

        if (semid < 0)
            return(201);
      }
        else
      {
        /* Semaphore was nearly created so we need to initialize our
         * semaphore value */

        if (semctl(semid, 0, SETVAL, 1))
        {
                return(201);
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
                /* Make sure semop and semaflag increment both get executed
                   without common SIGINT & SIGTERM interruption */
                old_mask = sigblock(block_mask);
                rc = semop(semid, &sembuf_s, 1);
                if (rc == 0)
                {
                  /* Got it, so return */

                  semaflag++;
                  /* unblock signals after getting semaphore */
                  block_mask = sigsetmask(old_mask);

                  return(SUCCESS);
                }
            if (errno == EAGAIN)
                {
        /* Semaphore held by someone else, but we indicated not to wait
         * forever. If user specified wait_time of zero, then just
         * return unsuccessfully without retries */


                        if (wait_time == 0)
                        return(201);

        /* Sleep for the time specified by the user and then retry */

                        sleep(wait_time);
                        retry_count --;
                }
                else
                        return(201);
        }
         return(201);
}


/***************************************************************************
 FUNCTION NAME: rel_semm()

   DESCRIPTION: This function releases the semaphore indicating
                completion of access to POS register 2 by that particular
                device.
   NOTES:

***************************************************************************/

int rel_semm(void)
{
        int             semid;
        struct sembuf   sembuf_s;
        int             pid;
        int             rc = SUCCESS;
        int             sempid;

        /* Obtain semaphore ID and create it if it does not already
         *  exist */

        semid = semget( (key_t) POS_SEMKEY, 1, S_IRUSR |  S_IWUSR | S_IRGRP |
                         S_IWGRP);


        /* Make sure that we got a valid semaphore ID, else return
         * error */

        if (semid < 0)
           return(201);

        /* Now, we want to make sure that we do not attempt to release
         * the semaphore if we don't already have it. This ensures that
         * the semaphore value remains < 1, therefore binary */

        /* First, get the current process ID */

        pid = getpid();

        /* Next, get the process ID of the process which currently has
         * the semaphore */

        sempid = semctl(semid, 0, GETPID, 0);
        if (sempid < 0)
           return(201);

        /* If the current process ID does not equal the semaphore's
         * process ID, then we are not holding it so return an error */

        /* if (pid != sempid)
           return(201);
	*/

        /* If this process has semaphore, release it */

        if (semaflag)
        {

        /* Release the semaphore by handing it a positive value which
         * get added to the semaphore value indicating that it is now
         * available. Note the SEM_UNDO flag. By including this, the
         * semaphore will get properly handled should this process be
         * terminated by a signal or something. */

          sembuf_s.sem_num = 0;
          sembuf_s.sem_op = 1;
          sembuf_s.sem_flg = SEM_UNDO;

          /* Make sure semop and semaflag decrement both get executed
             without common SIGINT & SIGTERM interruption */
          old_mask = sigblock(block_mask);
          rc = semop(semid, &sembuf_s, 1);
          semaflag--;

          /* unblock signals after releasing semaphore */
          block_mask = sigsetmask(old_mask);
	}

          return(rc);
}


/* This function insures that the mouse adapter is enabled */

int setup_mouse(int fd)
{
   unsigned char data,cdata;
   int rc = SUCCESS;

   /* Grab the semaphore before accessing POS2 */
   rc = set_semm(1);

   if (rc != SUCCESS) {
     PRINTERR("Error grabbing semaphore\n");
     return(rc);
   }

  /* Put mouse adapter in reset */
  /* Read the current setting of SIO control reg (POS2) */

   if (mach_info.mach_model == FIREB_MODEL)
       rd_posm(fd, &cdata, SIO_CTL_G30REG);
   else
       rd_posm(fd, &cdata, SIO_CONTROL_REG);

  /* disable mouse adapter in SIO control reg (POS2) */

   cdata = cdata | SIO_REG_RESET_MOUSE;
   if (mach_info.mach_model == FIREB_MODEL)
       rd_posm(fd, &cdata, SIO_CTL_G30REG);
   else
       wr_posm(fd, &cdata, SIO_CONTROL_REG);

   usleep(500*1000);

  /* Make sure mouse adapter is enabled in SIO control reg (POS2) */

   cdata = cdata & ~SIO_REG_RESET_MOUSE;
   if (mach_info.mach_model == FIREB_MODEL)
       rd_posm(fd, &cdata, SIO_CTL_G30REG);
   else
       wr_posm(fd, &cdata, SIO_CONTROL_REG);

   usleep(500*1000);

   /* Now release the semaphore after accessing POS2 */
   rc = rel_semm();

   return(rc);

}

  /* This function uses the machine device driver to read one byte from
   the specified address, returning the information to pdata */

int rd_bytem(int fd, uchar *pdata, uint addr)
{ 
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOBUSGET, &iob);
 /* printf("Read byte = %2X\n",*pdata);
  printf("Read addr = %4X\n",addr);
 */
  return (rc);
}


/* This function uses the machine device driver to read one byte from
   the specified pos register, returning the information to pdata */

int rd_posm(int fd, uchar *pdata, uint addr)
{ 
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOCCGET, &iob);
 /* printf("Read byte = %2X\n",*pdata);
  printf("Read addr = %4X\n",addr);
 */
  return (rc);
}


/* This function uses the machine device driver to read one word from
   the specified address, returning the information to pldata */

int rd_wordm(int fd, uint *pldata, uint addr)
{ 
  MACH_DD_IO iob;
  int rc;

  iob.md_data = (char *)pldata;
  iob.md_incr = MV_WORD;
  iob.md_size = 1;
  iob.md_addr = addr;

  rc = ioctl(fd, MIOBUSGET, &iob);
 /* printf("Read word = %4X\n",*pldata);
  printf("Read addr = %4X\n",addr);
 */
  return (rc);
}


/* This function uses the machine device driver to write two bytes from
   pdata to the specified address */

int wr_2bytem(int fd, ushort *pdata, uint addr)
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = (char *)pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOBUSPUT, &iob);
 /* printf("Write byte = %2X\n",*pdata);
  printf("Write addr = %4X\n",addr);
 */
  return (rc);
}



/* This function uses the machine device driver to write one byte from
   pdata to the specified address */

int wr_bytem(int fd, uchar *pdata, uint addr)
{ 
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOBUSPUT, &iob);
 /* printf("Write byte = %2X\n",*pdata);
  printf("Write addr = %4X\n",addr);
 */
  return (rc);
}



/* This function uses the machine device driver to write one byte from
   pdata to the specified pos register address */

int wr_posm(int fd, uchar *pdata, uint addr)
{ 
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOCCPUT, &iob);
 /* printf("Write byte = %2X\n",*pdata);
  printf("Write addr = %4X\n",addr);
 */
  return (rc);
}



/* This function uses the machine device driver to write one word from
   pldata to the specified address */

int wr_wordm(int fd, uint *pldata, uint addr)
{ 
  MACH_DD_IO iob;
  int rc;

  iob.md_data = (char *)pldata;
  iob.md_incr = MV_WORD;
  iob.md_size = 1;
  iob.md_addr = addr;

  rc = ioctl(fd, MIOBUSPUT, &iob);
 /* printf("Write word = %4X\n",*pldata);
  printf("Write addr = %4X\n",addr);
 */
  return (rc);
}


