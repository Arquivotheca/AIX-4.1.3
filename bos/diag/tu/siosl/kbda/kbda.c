static char sccsid[] = "@(#)27  1.1.1.9  src/bos/diag/tu/siosl/kbda/kbda.c, tu_siosl, bos41J, 9515A_all 4/7/95 11:02:42";
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: SendKbDatak
 *              SpeakerReadWriteTestk
 *              exectu
 *              get_machine_model_tuk
 *              kexectu
 *              rd_bytek
 *              rd_posk
 *              rd_wordk
 *              tu10k
 *              tu20k
 *              tu30k
 *              tu40k
 *              wr_bytek
 *              wr_posk
 *              wr_wordk
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
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
#include <sys/sysconfig.h>
#include <sys/intr.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <diag/atu.h>

#include "salioaddr.h"
#include "misc.h"
#include <sys/diagex.h>   /* include system file when available */
#include <sys/inputdd.h>   /* include system file when available */

/* For use in reading ipl control block info for machine model */
#include <sys/iplcb.h>
#include <sys/rosinfo.h>

/* Declare global variables for system registers */
unsigned int comp_reset_reg;

/* This structure will be used to store the machine type, and mse/kbd/tab
   polyswitch mask which will be used by tu20 */

struct info_struct {
  uchar switch_stat_mask;
  int machine;
  uchar mach_model;
} mach_info;

/* Initialize 'POWER' and 'RSC' flags */
enum machines { RSC = 1, POWER };


/* Path of interrupt handler for testing using DIAGEX */

#ifdef DIAGNOSTICS
#define KBD_DIAG_SLIH_PATH "/usr/lpp/diagnostics/slih/kbd_slih"
#else
#define KBD_DIAG_SLIH_PATH "/usr/lpp/htx/etc/kernext/kbd_slih"
#endif

uchar rx_kbd_status[5];     /* Kbd data array, used by interrupt handler */

diagex_dds_t kbd_dds;         /* Kbd device dependent structure */
diag_struc_t *kbd_diagex_handle;  /* kbd DIAGEX handle */
int int_handler_kmid = 0; /* holds the kmid for int handler */
uint diagflag;            /* Flag for setting kbd DD to DIAGNOSTIC DISABLE or */
                          /* ENABLE mode */

uint semaflag = 0;      /* Used in set_sem and rel_sem - if true, this process
			   has obtained the kbd/mse/tab shared semaphore */

 /* Global masks for blocking and restoring signals */
int block_mask = SIGINT | SIGTERM;
int old_mask;

int diagcounter = 0;     /* Keep track of when we're in kbd diagnostic mode */

  /* Following cmds for 101/102 kbds */
static unsigned char kdd_cmds[2][30] = {{0xf0,0x03,0xf3,0x2b,0xfa,0xfc,0x39,0x19
,
                             0x11,0x12,0x59,0x14,0x76,0x58,0xf4,0x00,
                             0,0,0,0,0,0,0,0,0,0,0,0,0,0},

 /* Following cmds for 106 kbd */

                           {0xf3,0x2b,0xfc,0x39,0x19,0x11,0x12,0x59,
                             0x14,0x76,0x58,0x0e,0x20,0x28,0x30,0xfa,
                             0x61, 0x6a, 0x63, 0x60, 0xf4, 0x00,
                             0,0,0,0,0,0,0,0}
                          };

/***************************************************************************
 NOTE: This function is called by Hardware exerciser (HTX),Manufacturing 
       application and Diagnostic application to invoke a test unit (TU) 
                                                                         
       The 'fd' parameter in exectu should be for the keyboard 
       device driver, '/dev/hft'.  The file descriptor for the machine
       device driver is passed in the TUCB structure.
                                                                         
**************************************************************************/

#ifdef DIAGNOSTICS
int kexectu(fd, tucb_ptr) /* Compile for diagnostics */
#else
int exectu(fd, tucb_ptr)  /* Compile for everyone else */
#endif
   long     fd ;    /* File descriptor for /dev/kbd# */
   struct TUCB *tucb_ptr;
   {

     short unsigned int i,j;  /* Loop Index */
     int rc = SUCCESS;       /* return code */
     char msg[80];           /* Character buffer for htx message strings */

   /* Get machine type and mse/kbd/tab polyswitch status check mask */

    rc = get_machine_model_tuk(tucb_ptr->mach_fd, &mach_info);
   
    power_flag = 0;

   /* Initialize variables for system register addresses with appropriate
      addresses according to machine type */
   if (rc == SUCCESS) {

       if (mach_info.machine == POWER) {
 	 comp_reset_reg = COMP_RESET_REG_POWER;
	 power_flag = 1;
       }
       else {
 	 comp_reset_reg = COMP_RESET_REG_RSC;
       }

    /* Set loop to 1 if preset to 0 */
    if (tucb_ptr->header.loop == 0)
	tucb_ptr->header.loop = 1;
 
    for (i=0; (i<tucb_ptr->header.loop) && (rc == SUCCESS); i++)
     {
       switch(tucb_ptr->header.tu)
        {  case   10: PRINT("Kbd adapter internal test - Executing TU 10\n");
                     rc = tu10k(fd,tucb_ptr->mach_fd, tucb_ptr);
                     break; 
           case   20: PRINT("Keyboard fuse test - Executing TU 20\n");
                     rc = tu20k(tucb_ptr->mach_fd, tucb_ptr);
                     break; 
           case   30: PRINT("Speaker write/read/verify logic test - Executing TU 30\n");
                     rc = tu30k(tucb_ptr->mach_fd, tucb_ptr);
                     break;
	   case   40: PRINT("Keyboard reset test - Executing TU 40\n");
	             rc = tu40k(fd,tucb_ptr->mach_fd, tucb_ptr);
		     break;       
	   case   99: PRINT("Test cleanup - Executing TU 99\n");
	             rc = tu99k(fd,tucb_ptr->mach_fd);
		     break;       
           default : rc = WRONG_TU_NUMBER;
		     return(rc);
        }  /* end case */
   
     }  /* i for loop */
  } /* if rc = SUCCESS */
  return(rc);   /* indicate there was no error */
}  /* End function */

/* Define for SendKbDatak function called by TU's 10 and 40 */
int SendKbDatak(int fd, unsigned char, char *);

/* Kbd adapter internal test TU */

int tu10k(kbdfd,fd, tucb_ptr)
long     fd;	      /* File descriptor for machine device */
long     kbdfd;    /* File descriptor for kbd device */
struct TUCB *tucb_ptr;
{
   unsigned char datarray[4],data,cdata,Status;	
   unsigned int ldata;	
   int i, j, status, rc = SUCCESS;
   MACH_DD_IO iob;
   char msg[80];
   int numpattern = 4, count = 0, inttimeout;
   static unsigned char datapattern[4] = { 0x55, 0xaa, 0xef, 0x00 };
   uint  ctlreg;

   /* Setup to use DIAGEX */
   rc = start_diagexk(kbdfd,fd);

   if (rc != SUCCESS) {
      return(rc);
   }

  if (!power_flag)    /* Do NOT do this if POWER_PC product */
  {
   /* Reset kbd in stat/cmd reg */
    if (rc == SUCCESS) {
      rd_bytek(fd, &data, KBD_STAT_CMD_REG);
     /* Reset kbd in stat/cmd reg */
      data = data | 0x02;
      wr_bytek(fd, &data, KBD_STAT_CMD_REG);
      usleep(100);
     /* Take kbd out of reset */
      data = data & ~0x02;
      rc = wr_bytek(fd, &data, KBD_STAT_CMD_REG);
      usleep(100);
    }
  }

   /* Make sure kbd TX buffer empty and RX buffer empty */
   clear_bufferk(fd);

   if (rc == SUCCESS) {
   /* Issue a read to kbd status/command reg */
     rc = rd_bytek(fd, &data, KBD_STAT_CMD_REG);
   }

   if (rc == SUCCESS)
   {
      /* Put keyboard adapter in loop mode */
       data |= KBD_LOOP_MODE;
       wr_bytek(fd, &data, KBD_STAT_CMD_REG);
       rc = rd_bytek(fd, &data, KBD_STAT_CMD_REG);
   }

   if (rc == SUCCESS)
   {
     /* Check stat/cmd reg for loop mode */
     if ((data & 0x04) != 0x04)
     {
        sprintf(msg,"Not able to put kbd adapter in loop mode:%2X\n",data);
	PRINTERR(msg);
        if (mach_info.mach_model == FIREB_MODEL)
           ctlreg = SIO_CTL_G30_REG;
        else
           ctlreg = SIO_CONTROL_REG;
	rd_posk(fd, &cdata, ctlreg);
        sprintf(msg,"Read SIO ctrl reg, POS2 : %2x\n",cdata);
        PRINTERR(msg);
        rc = KBD_LOGIC_ERROR; 
	cleanupk(kbdfd,fd);
	return(rc);
     } 
   }

  /* Send data patterns to test loop mode */
   if (rc == SUCCESS)
   { 
    for (i=0; i<numpattern; i++)
     {

       if (power_flag)
       {
         /* Put keyboard adapter in loop mode - this is done before sending
  	   each byte of loop data.  It is a RBW DD2 workaround. */
         /* Issue a read to kbd status/command reg */
         rd_bytek(fd, &data, KBD_STAT_CMD_REG);

         /* Put keyboard adapter in loop mode */
         data |= KBD_LOOP_MODE;
         wr_bytek(fd, &data, KBD_STAT_CMD_REG);
       }

      /* Make sure kbd buffer is clear before sending data */
      clear_bufferk(fd);

      if (rc == SUCCESS)
      {
       /* Set global interrupt ptr contents to 0 */
        memset(rx_kbd_status, 0, sizeof(uchar) * 5);
        rx_kbd_status[1] = 0xff;  /* So that test using data '0' won't fail */

       /* Send data pattern 'datapattern[i]'  */
	wr_bytek(fd, &datapattern[i], KBD_DATA_REG);

	inttimeout = 600;
   	while ((rx_kbd_status[1] == 0xff) && --inttimeout) {
            usleep(1000);  /* sleep 1000 usec. between checks */
   	}

   	if (inttimeout == 0) {
	   sprintf(msg,"Timed out getting kbd interrupt data, rx_kbd_status[1] = %x\n",rx_kbd_status[1]);
	   PRINTERR(msg);
           data = rx_kbd_status[1];
   	}

   	else {
           data = rx_kbd_status[1];
   	}

       /* datapattern[i] is test pattern  */	
        if (data != datapattern[i])
        {
         sprintf(msg,"Loopback data is not matched, data[0]:%2X,datapat:%2X\n",data,datapattern[i]);
         PRINTERR(msg);
         rd_posk(fd, &cdata, ctlreg);
         sprintf(msg,"Read SIO ctrl reg, POS2 : %2x\n",cdata);
         PRINTERR(msg);
         rc = KBD_LOGIC_ERROR; 
        }
      
      } /* if rc == SUCCESS */

      if (power_flag)
      {
        /* Put keyboard adapter out of loop mode - this is done after getting
  	 each byte of looped data.  It is a RBW DD2 workaround. */

         rd_bytek(fd, &data, KBD_STAT_CMD_REG);
         data &= ~KBD_LOOP_MODE;

         wr_bytek(fd, &data, KBD_STAT_CMD_REG);

      }

   } /* for loop */
  } /* rc = SUCCESS, if OK to send data patterns */

   /* Put keyboard adapter out of loop mode */

   rd_bytek(fd, &data, KBD_STAT_CMD_REG);
   data &= ~KBD_LOOP_MODE;

   wr_bytek(fd, &data, KBD_STAT_CMD_REG);

  /* Restore kbd DD to normal operation */
   cleanupk(kbdfd,fd);

return(rc);
}

int tu20k(fd, tucb_ptr)
long     fd ;
struct TUCB *tucb_ptr;
{
   unsigned char data, bad_switch_status;	
   unsigned int ldata;
   int status, rc = SUCCESS;
   char msg[80];
 
   /* Read tablet modem status register */
   rc = rd_bytek(fd, &data, TABLET_MODEM_ST_REG); 

   if (rc == SUCCESS)
   {
     /* The "bad_switch_status" variable is the kbd/mse/tab bad polyswitch
         status mask */

       bad_switch_status = mach_info.switch_stat_mask;

     /* Check for bad fuse/switch status */
       if (data & bad_switch_status)
       {
        sprintf(msg,"Fuse or polyswitch status is bad, tablet modem status reg = :%2X\n",data);
        PRINTERR(msg);
        rc = FUSE_BAD_ERROR;
       }

   }

   return (rc);
}

/* Definition for SpeakerReadWriteTestk function which the following
   TU uses */

int SpeakerReadWriteTestk(int, ulong, unsigned char);

int tu30k(fd, tucb_ptr)
long     fd ;
struct TUCB *tucb_ptr;
{
  unsigned short i;
  unsigned char reg1, reg2, reg0sav, reg1sav, data;
  int rc = SUCCESS, status;
  unsigned int ldata;
  char msg[80];
  int numpattern = 4;
  static unsigned char datapattern[4] = { 0x55, 0xaa, 0xff, 0x00 };


 /* Read from speaker controller registers */

  if ((rc=rd_bytek(fd, &reg0sav, SPK_CTRL0_REG)) != SUCCESS) {
   PRINTERR("Unsuccessful read of one byte from SPK_CTRL0_REG\n");
   return(rc);
  }

  if ((rc=rd_bytek(fd, &reg1sav, SPK_CTRL1_REG)) != SUCCESS) {
   PRINTERR("Unsuccessful read of one byte from SPK_CTRL0_REG\n");
   return(rc);
  }

 /* Write and read data patterns to speaker I/O registers */

  for (i=0; i<numpattern; i++) {
    data = datapattern[i];
    if ((rc = SpeakerReadWriteTestk(fd, SPK_CTRL0_REG, data)) != SUCCESS) {
     PRINTERR("Unsuccessful write to SPK_CTRL0_REG\n");
     return(rc);
    }
      
    if ((rc = SpeakerReadWriteTestk(fd, SPK_CTRL1_REG, data)) != SUCCESS) {
     PRINTERR("Unsuccessful write to SPK_CTRL1_REG\n");
     return(rc);
    }
  }

  /* Restore original data */
    if ((rc = wr_bytek(fd, &reg0sav, SPK_CTRL0_REG)) != SUCCESS) {
     PRINTERR("Unsuccessful write of one byte to SPK_CTRL0_REG\n");
     return(rc);
    }
    if ((rc = wr_bytek(fd, &reg1sav, SPK_CTRL1_REG)) != SUCCESS) {
     PRINTERR("Unsuccessful write of one byte to SPK_CTRL1_REG\n");
     return(rc);
   } 

  return(rc);
}

/* The following TU requires a keyboard to be attached.  This TU is used
   by EMC and Mfg. */

int tu40k(kbdfd,fd, tucb_ptr)
long     fd;	      /* File descriptor for machine device */
long     kbdfd;       /* File descriptor for kbd device */
struct TUCB *tucb_ptr;
{

   unsigned char cdata, data;
   int status, rc = SUCCESS;
   unsigned int ldata;
   uchar kbddata[3];
   char msg[80];

  /* Initialize kbddata array */
   kbddata[0] = 0xff;
   kbddata[1] = 0xff;
   kbddata[2] = 0xff;

   /* Setup to use DIAGEX */
   rc = start_diagexk(kbdfd,fd);

   if (rc != SUCCESS) {
      return(rc);
   }

  /* Reset keyboard device test */

   if ((rc = SendKbDatak(fd, (unsigned char)KBD_RESET_CMD, kbddata)) != SUCCESS)
   {
    PRINTERR("Unsuccessful at sending keyboard reset command to keyboard\n");
    cleanupk(kbdfd,fd);
    return(rc);
   }

 /* get keyboard acknowledge first  */
  if(kbddata[0] != KBD_ACK) 
    {
     sprintf(msg,"Keyboard does not acknowledge,data : %2X\n",kbddata[0]);
     PRINTERR(msg);
     rc = KBD_CMD_NOT_ACK_ERROR;
     cleanupk(kbdfd,fd);
     return(rc);
    }
   else
   {
    /* get keyboard BAT completion code */
    if (kbddata[1] != KBD_BAT_CC) {
     sprintf(msg,"Unsuccessful at receiving data from keyboard,data : %2X\n",kbddata[1]);
     PRINTERR(msg);
     rc = KBD_EXTERNAL_BAT_ERROR;
    }
   }

  cleanupk(kbdfd,fd);

  return(rc);
} 


/* The following TU is special in that it isn't for testing, but to perform 
   clean up if testing is aborted because of an interrupt.  This TU should
   only be called from inside the DA interrupt handler.  */

int tu99k(long kbdfd, long machfd)
{
  int rc = SUCCESS;
  char msg[80];
  struct cfg_load cfg_ld;

  cfg_ld.path = KBD_DIAG_SLIH_PATH;
  cfg_ld.libpath = NULL;
  cfg_ld.kmid = 0;
  errno = 0;

  /* Make sure the semaphore is released by process */
  rel_semk();

  /* Grab the semaphore before accessing POS2 */
  rc = set_semk(1);

  if (rc != SUCCESS) {
    PRINTERR("Couldn't capture semaphore, exit'ing without cleanup\n");
    return(rc);
  }

  if ( diagcounter > 0 )
  {
    /* Make sure following lines of code get executed
       without common SIGINT & SIGTERM interruption */
    old_mask = sigblock(block_mask);

    disable_kbdk(machfd);

    close_diagexk();

    /* If there is a kbd device attached, run following code */
    if ( kbdfd != -1 ) {
       diagflag = KSDDISABLE;
       ioctl(kbdfd,KSDIAGMODE,&diagflag);
    }
    diagcounter--;

    /* unblock signals after disabling kbd diag mode */
    block_mask = sigsetmask(old_mask);

  }

  /* Unload interrupt handler from kernel */
  sysconfig(SYS_QUERYLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
  if (cfg_ld.kmid)
  {
     rc = sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
  }

  /* Now release the semaphore after accessing POS2 */
  rc = rel_semk();

  return (rc);
}


/* The following TU cleans up at end of testing, disabling kbd adapter,
   closing DIAGEX, and restoring kbd DD back to normal operation.
   This function is the same as tu99k(). */

int cleanupk(long kbdfd, long machfd)
{
  int rc = SUCCESS;
  char msg[80];
 
   /* Make sure the semaphore is released by process */
   rel_semk();

   /* Grab the semaphore before accessing POS2 */
   rc = set_semk(1);

   if (rc != SUCCESS) {
     PRINTERR("Couldn't capture semaphore, exit'ing without cleanup\n"); 
     return(rc);
   }

  if ( diagcounter > 0 )
  {
    /* Make sure following lines of code get executed
       without common SIGINT & SIGTERM interruption */
    old_mask = sigblock(block_mask);

    disable_kbdk(machfd);

    close_diagexk();

    /* If there is a kbd device attached, run following code */
    if ( kbdfd != -1 ) {
       diagflag = KSDDISABLE;
       ioctl(kbdfd,KSDIAGMODE,&diagflag);
    }
     diagcounter--;
    /* unblock signals after disabling kbd diag mode */
    block_mask = sigsetmask(old_mask);

  }

  /* Now release the semaphore after accessing POS2 */
  rc = rel_semk();

  return (rc);
}

#define MAX_SEM_RETRIES 5
#define POS_SEMKEY      0x3141592

/***************************************************************************
   FUNCTION NAME: set_semk(wait_time)

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

int set_semk(wait_time)
int wait_time;
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
            /* Make sure and unblock signals even if didn't get semaphore - do
               not feel like changing logic flow of this code */
            block_mask = sigsetmask(old_mask);

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
 FUNCTION NAME: rel_semk()

   DESCRIPTION: This function releases the semaphore indicating
                completion of access to POS register 2 by that particular
                device.
   NOTES:

***************************************************************************/

int rel_semk()
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


/* This function writes data to the keyboard speaker register specified
   by the 'addr' parameter and reads the data back to make sure the write
   was successful. */

int SpeakerReadWriteTestk(int fd, ulong addr, unsigned char data)
{
  unsigned char tmp;
  int rc;
	
  rc = wr_bytek(fd, &data, addr);
	
  if (rc == SUCCESS)
  {
    rc = rd_bytek(fd, &tmp, addr);
    {
      if(data != tmp)
        rc = SPK_LOGIC_ERROR;
    }
  }
  return(rc);
}

/* This function sends data to the keyboard and reads back the returned data
   from the keyboard buffer using the test kbd interrupt handler. */

int SendKbDatak(int fd, unsigned char value, uchar *kbddata)
{
  unsigned int count, pollcount = 20;
  unsigned char Status, data;
  int j,rc = SUCCESS;
  int inttimeout;
  char msg[80];           /* Character buffer for htx message strings */

  clear_bufferk(fd);

  kbddata[0] = 0;
  kbddata[1] = 0;
  kbddata[2] = 0;
  kbddata[3] = 0;


   if ((rc = clear_bufferk(fd)) != SUCCESS) {   /* Make sure kbd buffer clear */
        return(rc);
   }

 /* Set global interrupt ptr contents to 0 */
   memset(rx_kbd_status,0, sizeof(uchar) * 5);

                                       /*       send command/data           */
  if ((rc = wr_bytek(fd, &value, KBD_DATA_REG)) != SUCCESS) {
            sprintf(msg,"Unsuccessful writing %x to KBD_DATA_REG\n",value);
	    PRINTERR(msg);
            return(rc);
  }

  for (j = 0; j < 2; j++) {  /* Get potentially 2 kbd data bytes */

   /* Get byte of returned kbd data */

           inttimeout = 600;
           while ((rx_kbd_status[j+1] == 0) && --inttimeout) {
              usleep(1000);  /* sleep 1000 usec. between checks */
           }

           if (inttimeout == 0) {
             sprintf(msg,"Timed out getting kbd interrupt data, rx_kbd_status[%d] = %x\n" ,j+1,rx_kbd_status[j+1]);
	     PRINTERR(msg);
             kbddata[j] = rx_kbd_status[j+1];
             return(-1);
           }

           else {
             kbddata[j] = rx_kbd_status[j+1];
           }

  } /* for j */

 return(rc);
}
 
/* This function disables the kbd DD and calls open_diagexk */    

int start_diagexk(long kbdfd,long machfd)
{
  int rc = SUCCESS;
  char msg[80];           /* Character buffer for htx message strings */

   /* Grab the semaphore before accessing POS2 */
   rc = set_semk(1);

   if (rc != SUCCESS) {
     PRINTERR("Couldn't capture semaphore, exit'ing test\n"); 
     return(rc);
   }

    /* Make sure following lines of code get executed
       without common SIGINT & SIGTERM interruption */
   old_mask = sigblock(block_mask);

 /* If there is a kbd device attached, run following code */
  if ( kbdfd != -1 ) {
    /* PRINT("Disabling kbd DD\n");*/
     errno = 0;
     diagflag = KSDENABLE;

     rc = ioctl(kbdfd,KSDIAGMODE,&diagflag);

     if (rc != SUCCESS)
     {
        sprintf(msg,"Enabling diagnostic mode KSDENABLE failed, errno = %d, rc = %d\n",errno,rc);
        PRINTERR(msg);
        rc = 200;
        /* unblock signals before calling rel_semk */
        block_mask = sigsetmask(old_mask);
        rel_semk();
        return(rc);
     }
  }
  diagcounter++;

  rc = open_diagexk();
  if (rc != SUCCESS)
  {
     if (kbdfd != -1) {
        diagflag = KSDDISABLE;
        ioctl(kbdfd,KSDIAGMODE,&diagflag);
     }
     diagcounter--;
     rc = 200;
     /* unblock signals before calling rel_semk */
     block_mask = sigsetmask(old_mask);
     rel_semk();
     return(rc);
  }

  enable_kbdk(machfd);

  /* unblock signals */
  block_mask = sigsetmask(old_mask);

  /* Now release the semaphore after accessing POS2 */
   rc = rel_semk();


  return(rc);

}

/* This function insures the kbd adapter Rx and Tx buffers are ready for
   new kbd data to be sent */

int clear_bufferk(fd)
int fd;
{
   int i = 0, rc = SUCCESS;
   uchar data;
  char msg[80];           /* Character buffer for htx message strings */

   if ((rc = rd_bytek(fd, &data, KBD_STAT_CMD_REG)) != SUCCESS) {
      PRINTERR("Unsuccessful read from KBD_STAT_CMD_REG\n");
      return(rc);
   }

  /* Make sure keyboard TX & RX buffers are ready before sending more keyboard
     data */

   while ( ( (data & KBD_RX_BUF_FULL) || (!(data & KBD_TX_BUF_EMPTY)) ) && (i < 200))
   {
      if ((rc = rd_bytek(fd, &data, KBD_DATA_REG)) != SUCCESS) {
         PRINTERR("Unsuccessful read from KBD_DATA_REG\n");
         return(rc);
      }
      usleep(25 * 1000);

      if ((rc = rd_bytek(fd, &data, KBD_STAT_CMD_REG)) != SUCCESS) {
         PRINTERR("Unsuccessful read from KBD_STAT_CMD_REG\n");
         return(rc);
      }
      usleep(25 * 1000);

      ++i;
   }

   if ( i == 200 ) {  /* Not able to clear kbd TX buffer */
        PRINTERR("Unable to clear kbd TX buffer\n");
        rc = -1;
   }
 
   return(rc);

}


/* This function makes sure kbd adapter is enabled */

int enable_kbdk(fd)
int fd;
{
  uchar cdata;
  int rc = SUCCESS;
  char msg[80];           /* Character buffer for htx message strings */
  uint ctlreg;

  /* Read the current setting of SIO control reg (POS2) */

   if (mach_info.mach_model == FIREB_MODEL)
      ctlreg = SIO_CTL_G30_REG;
   else
      ctlreg = SIO_CONTROL_REG;

   if ((rc = rd_posk(fd, &cdata, ctlreg)) != SUCCESS) 
     {
        PRINTERR("Unsuccessful read of one byte from SIO_CONTROL_REG\n");
        return(rc);
     }

  /* Take keyboard adapter out of reset */
   cdata = cdata & ~0x04;

   if ((rc = wr_posk(fd, &cdata, ctlreg)) != SUCCESS) 
     {
        PRINTERR("Unsuccessful write of one byte to SIO_CONTROL_REG\n");
        return(rc);
     }

   usleep(500*1000);

 /* Read again the current setting of SIO control reg (POS2) */

   if ((rc = rd_posk(fd, &cdata, ctlreg)) != SUCCESS) 
     {
       PRINTERR("Unsuccessful read of one byte from SIO_CONTROL_REG\n");
       return(rc);
     }

  /* If keyboard adapter still in reset state */

   if (cdata & 0x04) {
        sprintf(msg,"POS2 still in reset for keyboard, POS2 : %2x\n",cdata);
	PRINTERR(msg);
        rc = -1;
        return(rc);
   }

   return(rc);

} /* enable_kbd */

/* This function makes sure that kbd adapter is disabled before closing 
   DIAGEX */

int disable_kbdk(fd)
int fd;
{
  uchar cdata;
  int rc = SUCCESS;
  char msg[80];           /* Character buffer for htx message strings */
  uint ctlreg;

  /* Read the current setting of SIO control reg (POS2) */

   if (mach_info.mach_model == FIREB_MODEL)
      ctlreg = SIO_CTL_G30_REG;
   else
      ctlreg = SIO_CONTROL_REG;

   if ((rc = rd_posk(fd, &cdata, ctlreg)) != SUCCESS) 
      {
         PRINTERR("Unsuccessful read of one byte from SIO_CONTROL_REG\n");
         return(rc);
      }

  /* Put keyboard adapter in reset */
   cdata = cdata | 0x04;
   if ((rc = wr_posk(fd, &cdata, ctlreg)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to SIO_CONTROL_REG\n");
    return(rc);
   }

  return(rc);

} /* disable_kbd */


/* This function loads up test kbd interrupt handler routine (if not already
   loaded) and opens DIAGEX */

int open_diagexk(void)
{
  int rc = 0;
  struct cfg_load cfg_ld;
  char msg[80];           /* Character buffer for htx message strings */

  /* load interrupt handler */
  cfg_ld.path = KBD_DIAG_SLIH_PATH;
  cfg_ld.libpath = NULL;
  cfg_ld.kmid = 0;

  errno = 0;
  sysconfig(SYS_QUERYLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));

  if (!(cfg_ld.kmid))
  {
    rc = sysconfig(SYS_SINGLELOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
    if (rc != SUCCESS) {
       sprintf(msg,"\n\nLOAD ./slih FAILED, errno = %d, rc =%d\n\n",errno,rc);
       PRINTERR(msg);
       rc = 200;
       return(rc);
    } else {
       int_handler_kmid = cfg_ld.kmid;
    } /* endif */
  }

  /* Clear 1st 5 bytes. 1st byte to store index of current byte being processed
     by interrupt handler.  Up to four bytes of data returned from kbd after a
     command is sent. */

  memset(rx_kbd_status, 0, sizeof(uchar) * 5);

  /* initialize Device Dependent Structure info */
  rc = get_kbd_ddsk();
  if (rc) {
    return(rc);
  } /* endif */

  errno = 0;
  rc = diag_open( &kbd_dds, &kbd_diagex_handle );

  if (rc != SUCCESS) {
    sprintf(msg,"diag_open failed, rc = %x, errno = %d\n",rc,errno);
    PRINTERR(msg);
    rc = 200;
  }

  return(rc);

} /* end open_diagex() */

/* This function closes DIAGEX */

int close_diagexk(void)
{
  int rc = 0;
  char msg[80];           /* Character buffer for htx message strings */

  errno = 0;
  rc = diag_close(kbd_diagex_handle);

  if (errno != SUCCESS) {
    sprintf(msg,"diag_close failed, rc = %d, errno = %d\n",rc,errno);
    PRINTERR(msg);
    rc = 200;
  }

  return(rc);

} /* end close_diagex() */


/* This function get kbd dds info and puts in kbd_dds structure for use with
   DIAGEX.  Some of the info is hard-coded because it wasn't found in ODM
   (any ideas on this??) */

int get_kbd_ddsk(void)
{
  int rc = 0;

  /* initialize Device Dependent Structure info */

  kbd_dds.slot_num = 0;
  kbd_dds.bus_intr_lvl = 1;
  kbd_dds.intr_priority = INTCLASS3;
  kbd_dds.intr_flags = 0;
  kbd_dds.dma_lvl = 0;
  kbd_dds.bus_io_addr = 0xf0000000;
  kbd_dds.bus_io_length = 0x60;
  kbd_dds.bus_mem_addr = 0;
  kbd_dds.bus_mem_length = 0;
  kbd_dds.dma_bus_mem = 0;
  kbd_dds.dma_bus_length = 0;
  kbd_dds.dma_flags = 0;
  kbd_dds.bus_id = 0x820c0060;
  kbd_dds.bus_type = BUS_MICRO_CHANNEL;
  kbd_dds.kmid = int_handler_kmid;
  kbd_dds.data_ptr = rx_kbd_status;
  kbd_dds.d_count = 5;
  kbd_dds.maxmaster = 0;

  return(rc);

} /* end get_kbd_dds() */

/* The following are defines for use in the get_machine_model_tuk routine */

#define SAL1_MODEL                   0x41   /* SAL1_MODEL (rosinfo.h)*/
#define SAL2_MODEL                   0x45   /* SAL2_MODEL (rosinfo.h)*/
#define CAB_MODEL                   0x43   /* CAB_MODEL (rosinfo.h)*/
#define CHAP_MODEL                  0x47   /* CHAP_MODEL (rosinfo.h)*/
#define RBW_MODEL                   0x46   /* RBW_MODEL (rosinfo.h)*/
#define INVALID_MACHINE_MODEL       -1

/* The function below is called by ?exectu() routine */
/* Using the machine DD, this routine returns the appropriate mask data so 
   the fuse/polyswitch status bit can be checked.  Also, it returns which
   type of machine is being used.  Both machine and mask data are contained
   in a structure of type info_struct. */

int get_machine_model_tuk(int mdd_fdes, struct info_struct *mach_ptr)
{
  MACH_DD_IO    mdd;
  IPL_DIRECTORY *iplcb_dir;                                        /* iplcb.h */
  IPL_INFO      *iplcb_info;
  int rc;
  int  machine_model;

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

               mach_ptr->mach_model = machine_model;

               if ((machine_model == CAB_MODEL) ||
                   (machine_model == CHAP_MODEL))
                {
                  mach_ptr->switch_stat_mask = 0x20;
                }
               else {
                  mach_ptr->switch_stat_mask = 0x10;
               }
               if ( (machine_model == SAL1_MODEL) || (machine_model == SAL2_MODEL) || (machine_model == CAB_MODEL) || (machine_model == CHAP_MODEL) )
               {
                  mach_ptr->machine = RSC;
               }
               else {
                  mach_ptr->machine = POWER;
               }

	    } /* endif */

	  } /* endif */
 
       } /* endif */

      free (iplcb_dir);
      free (iplcb_info);

  return (rc);

}


/* This function uses the machine device driver to read one byte from
   the specified address, returning the information to pdata */

int rd_bytek(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{  
  MACH_DD_IO iob;
  int rc;
	
  iob.md_data = pdata; 
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr; 

  rc = ioctl(fd, MIOBUSGET, &iob);
/*  printf("Read byte = %2X\n",*pdata);
  printf("Read addr = %4X\n",addr);
*/
  return (rc);
}

/* This function uses the machine device driver to read one byte from
   the specified pos register, returning the information to pdata */

int rd_posk(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{  
  MACH_DD_IO iob;
  int rc;
	
  iob.md_data = pdata; 
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr; 

  rc = ioctl(fd, MIOCCGET, &iob);
/*  printf("Read byte = %2X\n",*pdata);
  printf("Read addr = %4X\n",addr);
*/
  return (rc);
}

/* This function uses the machine device driver to read one word from
   the specified address, returning the information to pldata */

int rd_wordk(fd, pldata, addr)
int fd;
unsigned int *pldata;
unsigned int addr;
{  
  MACH_DD_IO iob;
  int rc;
	
  iob.md_data = (char *)pldata; 
  iob.md_incr = MV_WORD;
  iob.md_size = 1;
  iob.md_addr = addr; 

  rc = ioctl(fd, MIOBUSGET, &iob);
/*  printf("Read word = %4X\n",*pldata);
  printf("Read addr = %4X\n",addr);
*/
  return (rc);
}

/* This function uses the machine device driver to write one byte from
   pdata to the specified address */

int wr_bytek(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{  
  MACH_DD_IO iob;
  int rc;
	
  iob.md_data = pdata; 
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr; 

  rc = ioctl(fd, MIOBUSPUT, &iob);
/*  printf("Write byte = %2X\n",*pdata);
  printf("Write addr = %4X\n",addr);
*/
  return (rc);
}

/* This function uses the machine device driver to write one byte from
   pdata to the specified pos register address */

int wr_posk(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{  
  MACH_DD_IO iob;
  int rc;
	
  iob.md_data = pdata; 
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr; 

  rc = ioctl(fd, MIOCCPUT, &iob);
/*  printf("Write byte = %2X\n",*pdata);
  printf("Write addr = %4X\n",addr);
*/
  return (rc);
}

/* This function uses the machine device driver to write one word from
   pldata to the specified address */

int wr_wordk(fd, pldata, addr)
int fd;
unsigned int *pldata;
unsigned int addr;
{  
  MACH_DD_IO iob;
  int rc;
	
  iob.md_data = (char *)pldata; 
  iob.md_incr = MV_WORD;
  iob.md_size = 1;
  iob.md_addr = addr; 

  rc = ioctl(fd, MIOBUSPUT, &iob);
/*  printf("Write word = %4X\n",*pldata);
  printf("Write addr = %4X\n",addr);
*/
  return (rc);
}


