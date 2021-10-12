static char sccsid[] = "@(#)55  1.1.2.3  src/bos/diag/tu/siosl/tableta/tableta.c, tu_siosl, bos411, 9428A410j 3/21/94 12:51:51";
/*
 *   COMPONENT_NAME: TU_SIOSL
 *
 *   FUNCTIONS: GetLoopTbDatat
 *              SendLoopTbDatat
 *              SendRxTbDatat
 *              SendTbDatat
 *              exectu
 *              get_machine_model_tut
 *              rd_bytet
 *              rd_post
 *              rd_wordt
 *              texectu
 *              tu10t
 *              tu20t
 *              tu30t
 *              tu40t
 *              wr_bytet
 *              wr_post
 *              wr_wordt
 *
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
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
#include <sys/sem.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <diag/atu.h>
#include "salioaddr.h"
#include "misc.h"

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
} mach_info;

/* Initialize 'POWER' and 'RSC' flags */
enum machines { RSC = 1, POWER };

/***************************************************************************
* NOTE: This function is called by Hardware exerciser (HTX),Manufacturing 
*       application and Diagnostic application to invoke a test unit (TU)
*                                                                         
*       The 'fd' parameter in exectu should be for the tablet
*       device driver, '/dev/hft'.  The file descriptor for the machine
*       device driver is passed with the TUCB structure.
*                                                                         
***************************************************************************/

#ifdef DIAGNOSTICS
int texectu(fd,tucb_ptr) /* Compile for diagnostics */
#else
int exectu(fd,tucb_ptr) /* Compile for everyone else */
#endif
   long     fd ; /* Dummy file descriptor */

   struct TUCB *tucb_ptr;
   {
     short unsigned int i,j;  /* Loop Index */
           int rc = SUCCESS;  /* return code */
               char msg[80]; /* Character buffer for htx message strings */
     static int firsttime = 1;

    /* The first time through, we will enable tablet adapter and set up
       line parameters. */

     if (firsttime)
     {
        rc = setup_tablet(tucb_ptr->mach_fd);
        if (rc != SUCCESS) {
           return(rc);
        }

        firsttime = 0;
     }

   /* Get machine type and mse/kbd/tab polyswitch status check mask */

    rc = get_machine_model_tut(tucb_ptr->mach_fd, &mach_info);

   /* Initialize variables for system register addresses with appropriate
      addresses according to machine type */
   if (rc == SUCCESS) {

       if (mach_info.machine == POWER) {
         comp_reset_reg = COMP_RESET_REG_POWER;
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
        {  case   10: PRINT("Tablet adapter internal test - Executing TU 10\n");
                     rc = tu10t(tucb_ptr->mach_fd,tucb_ptr);
                     break;       
           case   20: PRINT("Fuse test - Executing TU 20\n");
                     rc = tu20t(tucb_ptr->mach_fd,tucb_ptr);
                     break; 
           case   30: PRINT("Tablet wrap 'H' test for EMC - Executing TU 30\n");
                     rc = tu30t(tucb_ptr->mach_fd,tucb_ptr);
                     break;       
           case   40: PRINT("Tablet wrap plug test for MFG - Executing TU 40\n");
                     rc = tu40t(tucb_ptr->mach_fd,tucb_ptr);
                     break;       
           default : rc = WRONG_TU_NUMBER;
		     return(rc);
        }  /* end case */

     }  /* i for loop */

   } /* if rc = SUCCESS */

   return(rc);   /* indicate there was no error */
 }  /* End function */

int tu10t(fd, tucb_ptr)
   long     fd ;
   struct TUCB *tucb_ptr;
{
   unsigned char status,data,cdata, baud = 52;	
   unsigned int ldata;	
   int i = 0, rc = SUCCESS;
   char msg[80];
   int numpattern = 4;
   static unsigned char datapattern[4] = { 0x55, 0xaa, 0xff, 0x00 };

  /* Grab semaphore before accessing POS 2 */
   rc = set_semt(1);
  
   if (rc != SUCCESS) {
      PRINTERR("Error grabbing semaphore\n");
      return(rc);
   }

  /* Read the current setting of SIO control reg (POS2) */
   if ((rc = rd_post(fd, &cdata, SIO_CONTROL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful read of one byte from SIO_CONTROL_REG\n");
    rel_semt();
    return(rc);
   }
    
  /* Enable Standard I/O and reset tablet adapter in SIO control reg (POS2) */
   cdata = cdata | SIO_REG_ENABLE | SIO_REG_RESERVED;
   if ((rc = wr_post(fd, &cdata, SIO_CONTROL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to SIO_CONTROL_REG\n");
    rel_semt();
    return(rc);
   }

   sleep(1);

   cdata = cdata & ~SIO_REG_RESERVED;
   if ((rc = wr_post(fd, &cdata, SIO_CONTROL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to SIO_CONTROL_REG\n");
    rel_semt();
    return(rc);
   }
   usleep(500*1000);

  /* Double check POS2 */
  /* Read the current setting of SIO control reg (POS2) */
   if ((rc = rd_post(fd, &cdata, SIO_CONTROL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful read of one byte from SIO_CONTROL_REG\n");
    rel_semt();
    return(rc);
   }

   if (cdata & SIO_REG_RESERVED) {
   cdata = cdata & ~SIO_REG_RESERVED;
    if ((rc = wr_post(fd, &cdata, SIO_CONTROL_REG)) != SUCCESS) {
     PRINTERR("Unsuccessful write of one byte to SIO_CONTROL_REG\n");
     rel_semt();
     return(rc);
    }
   sleep(3);
   }

 /* Read again the current setting of SIO control reg (POS2) */
   if ((rc = rd_post(fd, &cdata, SIO_CONTROL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful read of one byte from SIO_CONTROL_REG\n");
    rel_semt();
    return(rc);
   }
   if (cdata & SIO_REG_RESERVED) {
        sprintf(msg,"POS2 still in reset for tablet, POS2 : %2x\n",cdata);
        PRINTERR(msg);
        rc = -1;
        rel_semt();
 	return(rc);
   }

  /* Release semaphore after accessing POS 2 */
   rc = rel_semt();
  
   if (rc != SUCCESS) {
      PRINTERR("Error releasing semaphore\n");
      return(rc);
   }

  /* read from Line status reg */
   if ((rc = rd_bytet(fd, &data, TABLET_LINE_ST_REG)) != SUCCESS) { 
     PRINTERR("Unsuccessful write of one byte to SIO_CONTROL_REG\n");
     return(rc);
   }

 /* Make sure that adapter was reset */

  /* read from Line control reg */
   if ((rc = rd_bytet(fd, &data, TABLET_LINE_CTRL_REG)) != SUCCESS) { 
     PRINTERR("Unsuccessful read of one byte from TABLET_LINE_CTRL_REG\n");
     return(rc);
   }

   if (data != 0x00) { 
     sprintf(msg,"Line control register not in reset state, data = %2X \n",data);
     PRINTERR(msg);
     rc = TABLET_RESET_ERROR;
     return(rc);
   }
 
 /* Set DLAB to 1 so baud rate can be set */
  data = data | 0x80;
  if ((rc = wr_bytet(fd, &data, TABLET_LINE_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_LINE_CTRL_REG\n");
    return(rc);
  }
  usleep(10 * 1000);

 /* Set baud rate to 9600 */
  if ((rc = wr_bytet(fd, &baud, TABLET_TX_BUFF_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_LINE_CTRL_REG\n");
    return(rc);
  }
  usleep(10 * 1000);

 /* Set Line parameters - DLAB bit to 0, 8 bits/char, 1 stop bit, odd parity */
  data = 0x0b;
  if ((rc = wr_bytet(fd, &data, TABLET_LINE_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_LINE_CTRL_REG\n");
    return(rc);
  }
  usleep(10 * 1000);

  /* read from Line status reg to clear it */
   if ((rc = rd_bytet(fd, &data, TABLET_LINE_ST_REG)) != SUCCESS) {
     PRINTERR("Unsuccessful write of one byte to SIO_CONTROL_REG\n");
     return(rc);
   }

  /* put the adapter in fifo mode       */

  data = 0x07;
  if ((rc = wr_bytet(fd, &data, TABLET_FIFO_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_FIFO_CTRL_REG\n");
    return(rc);
  }

  /* Clear fifo's, one more time to be sure */
  data = 0x07;
  if ((rc = wr_bytet(fd, &data, TABLET_FIFO_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_FIFO_CTRL_REG\n");
    return(rc);
  }

 /* write all 1's to scratch reg */
  data = 0xff;
 /* Scratch reg address */
  if ((rc = wr_bytet(fd, &data, TABLET_SCRATCH_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_SCRATCH_REG\n");
    return(rc);
  }

 /* Set Modem control reg for loop mode */

 /* Modem control reg address */

 if ((rc = rd_bytet(fd, &data, TABLET_MODEM_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful read of one byte from TABLET_MODEM_CTRL_REG\n");
    return(rc);
  }
  data = data | 0x10;
  if ((rc = wr_bytet(fd, &data, TABLET_MODEM_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_MODEM_CTRL_REG\n");
    return(rc);
  }
  usleep(10 * 1000);

 /* Check Modem control reg for loop mode */
  if ((rc = rd_bytet(fd, &data, TABLET_MODEM_CTRL_REG)) != SUCCESS) {
   PRINTERR("Unsuccessful read of one byte from TABLET_MODEM_CTRL_REG\n");
   return(rc);
  }

  if ((data & 0x10) != 0x10)
  {
    sprintf(msg,"Adapter is not in loop mode, data = %2X\n",data);
    PRINTERR(msg);
    rc = TABLET_RESET_ERROR;
    return(rc); 
  } 

  for (i=0; i<numpattern; i++) 
  {

 /* Send and receive back data pattern 'datapattern[i]'  */

  if (rc == SUCCESS) {
   if ((rc = SendRxTbDatat(fd,datapattern[i],&data)) != SUCCESS) {
    PRINTERR("Unsuccessful at sending loop data to tablet\n");
   }
  }

  if (rc == SUCCESS) {
   if (data != datapattern[i])	/* datapattern[i] is test pattern  */	
   {
    sprintf(msg,"Loopback data is not matched, data : %2X, datapat : %2X\n",data,datapattern[i]); 
    PRINTERR(msg);
   /* Read the current setting of SIO control reg (POS2) */
    rd_post(fd, &cdata, SIO_CONTROL_REG);
    sprintf(msg,"POS 2 contents : %2x\n",cdata);
    PRINTERR(msg);
    rc = TABLET_RESET_ERROR;
   }
  }
 }

 /* Put tablet adapter out of loop mode */

  rd_bytet(fd, &data, TABLET_MODEM_CTRL_REG);
  data = data & 0xef;
  wr_bytet(fd, &data, TABLET_MODEM_CTRL_REG);

  /* read from Line status reg to clear it */
   if ((rc = rd_bytet(fd, &data, TABLET_LINE_ST_REG)) != SUCCESS) {
     PRINTERR("Unsuccessful write of one byte to SIO_CONTROL_REG\n");
     return(rc);
   }

  /* put the adapter in fifo mode       */

  data = 0x07;
  if ((rc = wr_bytet(fd, &data, TABLET_FIFO_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_FIFO_CTRL_REG\n");
    return(rc);
  }

  /* Clear and enable fifo again to make sure */
  data = 0x07;
  if ((rc = wr_bytet(fd, &data, TABLET_FIFO_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_FIFO_CTRL_REG\n");
    return(rc);
  }

return(rc);
}

int tu20t(fd, tucb_ptr)
   long     fd ;
   struct TUCB *tucb_ptr;
{
   unsigned char data, bad_switch_status;	
   unsigned int ldata;
   int type, status, rc = SUCCESS;
   char msg[80];
 
   /* Read tablet modem status register */
   if ((rc = rd_bytet(fd, &data, TABLET_MODEM_ST_REG)) != SUCCESS) { 
    PRINTERR("Unsuccessful read of one word from TABLET_MODEM_ST_REG\n"); 
   }

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

/* This TU should only be run by EMC - (Must have tablet attached) */

int tu30t(fd, tucb_ptr)
   long     fd ;
   struct TUCB *tucb_ptr;
{
   unsigned char data, rcvdata;	
   int status;
   int rc = SUCCESS;
   char msg[80];

 /* Disable tablet */
   data = 0x09;
 
   if ((rc = SendTbDatat(fd, data)) != SUCCESS) { 
    PRINTERR("Unsuccessful at disabling tablet\n");
    rc = -1;
   }
   usleep(500 * 1000);

  if (rc == SUCCESS) {
  /* Send 'set wrap mode' command */
   data = 0x0E;
   if ((rc = SendTbDatat(fd, data)) != SUCCESS) {
    PRINTERR("Unsuccessful at sending data wrap mode command to tablet\n");
    rc = SET_WRAP_ERROR;
   }
  }
  usleep(500 * 1000);

 if (rc == SUCCESS) {
 /* Send data pattern 'H' */
  data = 'H';
  if ((rc = SendRxTbDatat(fd,data,&rcvdata)) != SUCCESS) {
     PRINTERR("Unsuccessful at sending data pattern to tablet\n");
     /* Send 'reset wrap mode' command */
     data = 0x0F;
     SendTbDatat(fd, data);
   }
 }

 if (rc == SUCCESS) {
  if (rcvdata != data) /* datapattern[i] is test pattern  */
  {
   sprintf(msg,"Wrapped data is not matched, data : %c\n",rcvdata);
   PRINTERR(msg);
   rc = BAD_WRAP_DATA;
  /* Send 'reset wrap mode' command */
   data = 0x0F;
   SendTbDatat(fd, data);
  }
 }

  /* Send 'reset wrap mode' command */
  if (rc == SUCCESS) {
    data = 0x0F;
    SendTbDatat(fd, data);

    /* Enable tablet */
    data = 0x08;

    if ((rc = SendTbDatat(fd, data)) != SUCCESS) {
     PRINTERR("Unsuccessful at enabling tablet\n");
     rc = TAB_ENABLE_ERROR;
    }
    usleep(500 * 1000);

  }

 return(rc);
}

/* This TU should only be run by Mfg. - (Must have tablet wrap plug) */

int tu40t(fd, tucb_ptr)
   long     fd;
   struct TUCB *tucb_ptr;
{
   unsigned char cdata,data, baud = 52;  /* Use baud=52 to set 9600 baud rate */
   int i,status;
   int rc = SUCCESS;
   char msg[80];

 /* Read from line ctrl reg */
  if ((rc = rd_bytet(fd, &data, TABLET_LINE_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful read of one byte from TABLET_LINE_CTRL_REG\n");
    return(rc);
  }

 /* Set DLAB to 1 so baud rate can be set */
  data = data | 0x80;
  if ((rc = wr_bytet(fd, &data, TABLET_LINE_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_LINE_CTRL_REG\n");
    return(rc);
  }
  usleep(10 * 1000);

 /* Set baud rate to 9600 */
  if ((rc = wr_bytet(fd, &baud, TABLET_TX_BUFF_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_LINE_CTRL_REG\n");
    return(rc);
  }
  usleep(10 * 1000);

 /* Set Line parameters - DLAB bit to 0, 8 bits/char, 1 stop bit, odd parity */
  data = 0x0b;
  if ((rc = wr_bytet(fd, &data, TABLET_LINE_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_LINE_CTRL_REG\n");
    return(rc);
  }
  usleep(10 * 1000);

 /* Disable interrupts */
  data = 0x00;
  if ((rc = wr_bytet(fd, &data, TABLET_INT_ENBL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful read of one byte from TABLET_LINE_CTRL_REG\n");
    return(rc);
  }
  usleep(10 * 1000);

 /* Disable loop mode in case it is enabled */
  data = 0x00;
  if ((rc = wr_bytet(fd, &data, TABLET_MODEM_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful read of one byte from TABLET_LINE_CTRL_REG\n");
    return(rc);
  }
  usleep(10 * 1000);

  /* put the adapter in fifo mode       */

  data = 0x01;
  if ((rc = wr_bytet(fd, &data, TABLET_FIFO_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_FIFO_CTRL_REG\n");
    return(rc);
  }

  data = 0x00;
  if ((rc = wr_bytet(fd, &data, TABLET_FIFO_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_FIFO_CTRL_REG\n");
    return(rc);
  }

  data = 0x01;
  if ((rc = wr_bytet(fd, &data, TABLET_FIFO_CTRL_REG)) != SUCCESS) {
    PRINTERR("Unsuccessful write of one byte to TABLET_FIFO_CTRL_REG\n");
    return(rc);
  }

 /* Send and receive back data */
   data = 0x5A;
   rc = SendRxTbDatat(fd, data, &cdata);
   usleep(10 * 1000);

   if (rc == SUCCESS) {
    if (cdata != 0x5A) /* datapattern 0x5A is test pattern  */
    {
     sprintf(msg,"Wrapped data is not 0x5A, data : %2x\n",cdata);
     PRINTERR(msg);
     rc = 40;
    }
   }

 return(rc);
}

/* The following are defines for use in the get_machine_model_tut routine */

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

int get_machine_model_tut(mdd_fdes, mach_ptr)
int mdd_fdes;
struct info_struct *mach_ptr;
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

#define MAX_SEM_RETRIES 5
#define POS_SEMKEY	0x3141592

/***************************************************************************
 FUNCTION NAME: set_semt(wait_time)

   DESCRIPTION: This function access the POS register 2 via the IPC
		semaphore which should be used by all programs desiring
		the access. The user passes in a "wait_time" to specify
		whether or not to "wait_forever" for the semaphore or
		retry MAX_SEM_RETRIES with a wait_interval of "wait_time" 

   NOTES: 	This function is used basically to overcome the concurrent 
		access of the POS register 2 by the Keyboard/Tablet,
		Diskette, Serial Port 1 and 2, Parallel Port and Mouse
		devices. 

***************************************************************************/

int set_semt(wait_time)
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
 FUNCTION NAME: rel_semt()

   DESCRIPTION: This function releases the semaphore indicating
		completion of access to POS register 2 by that particular
		device.
   NOTES:	  

***************************************************************************/

int rel_semt()
{
	int 		semid;
	struct sembuf	sembuf_s;
	int		pid;
	int 		rc;
	int		sempid;

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

	if (pid != sempid)
	   return(201);

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

/* This function insures that the tablet adapter is enabled and line
   parameters are setup. */

int setup_tablet(fd)
int fd;    /* File descriptor for machine DD */
{
   unsigned char data,cdata, baud = 52;
   int rc = SUCCESS;

   /* Grab the semaphore before accessing POS2 */
   rc = set_semt(1);

   if (rc != SUCCESS) {
     PRINTERR("Error grabbing semaphore\n");
     return(rc);
   }

  /* Read the current setting of SIO control reg (POS2) */
   rd_post(fd, &cdata, SIO_CONTROL_REG);

   /* Put tablet adapter in reset */
   cdata = cdata | SIO_REG_RESERVED;
   wr_post(fd, &cdata, SIO_CONTROL_REG);
   usleep(500 * 1000);

   /* Take tablet adapter out of reset */
   cdata = cdata & ~SIO_REG_RESERVED;
   wr_post(fd, &cdata, SIO_CONTROL_REG);

   usleep(500*1000);

   /* Now release the semaphore after accessing POS2 */
   rc = rel_semt();

   if (rc != SUCCESS) {
      PRINTERR("Error releasing semaphore\n");
      return(rc);
   }

  /* read from Line control reg */
   rd_bytet(fd, &data, TABLET_LINE_CTRL_REG);

  /* Set DLAB to 1 so baud rate can be set */
  data = data | 0x80;
  wr_bytet(fd, &data, TABLET_LINE_CTRL_REG);
  usleep(10 * 1000);

  /* Set baud rate to 9600 */
  wr_bytet(fd, &baud, TABLET_TX_BUFF_REG);
  usleep(10 * 1000);

  /* Set Line parameters - DLAB bit to 0, 8 bits/char, 1 stop bit, odd parity */
  data = 0x0b;
  wr_bytet(fd, &data, TABLET_LINE_CTRL_REG);
  usleep(10 * 1000);

  /* read from Line status reg to clear it */
  rd_bytet(fd, &data, TABLET_LINE_ST_REG);

  /* put the adapter in fifo mode       */

  data = 0x07;
  wr_bytet(fd, &data, TABLET_FIFO_CTRL_REG);

  /* Clear fifo's, one more time to be sure */
  data = 0x07;
  wr_bytet(fd, &data, TABLET_FIFO_CTRL_REG);

 /* write all 1's to scratch reg */
  data = 0xff;
  wr_bytet(fd, &data, TABLET_SCRATCH_REG);

  return(rc);

}

/* This function sends data, provided by 'value' parameter, to the tablet,
   and it puts received data from tablet into the tabdata array. */

int SendRxTbDatat(fd, value, tabdata)
int fd;
unsigned char value, *tabdata;
{
  uchar Status, data;
  int count = 1, rc = SUCCESS;

  rc = SendTbDatat(fd, value);

  if (rc == SUCCESS) {

    if ((rc = rd_bytet(fd, &data, TABLET_LINE_CTRL_REG)) != SUCCESS) {
      PRINTERR("Unsuccessful read of one byte from TABLET_LINE_CTRL_REG\n");
      return(rc);
    }

    /* Set DLAB of Line control reg to 0 */

    data = data & 0x7f;
    if ((rc = wr_bytet(fd, &data, TABLET_LINE_CTRL_REG)) != SUCCESS) {
      PRINTERR("Unsuccessful write of one byte to TABLET_LINE_CTRL_REG\n");
      return(rc);
    }

    while( count > 0 && polltabt(fd) != SUCCESS )
       count-- ;
 
    if ( polltabt(fd) == SUCCESS) {    /* Read from Recv. buffer */
       if ((rc = rd_bytet(fd, &data, TABLET_RX_BUFF_REG)) != SUCCESS) {
         PRINTERR("Unsuccessful read of one byte from TABLET_RX_BUFF_REG\n");
         return(rc);
       }

       *tabdata = data;
    }
    else 
    {
       return(-1);
    }


 } /* if rc = SUCCESS */

return(rc);
}

/* This function returns SUCCESS if data is in the tablet Rx buffer. */

polltabt(fd)
int fd;   /* File descriptor for machine DD */
{
   int rc = SUCCESS;
   uchar data;
   long  count  = 2000;

                                       /* BEGIN polling           */
   while (count)
   {                                   /* read line status                   */

      if ((rc = rd_bytet(fd, &data, TABLET_LINE_ST_REG)) != SUCCESS) {
        PRINTERR("Unsuccessful read of one byte from TABLET_LINE_ST_REG\n");
        return(rc);
      }

                                       /* if there is any data ready         */
      if  (data & 0x01 )
      {
                                       /* if no errors on the line           */
         if (!(data & 0x1e)) /* check for overrun, parity, break intrpt. and
                                framing errors */
            break ;
      }
      usleep(2000);    /* Sleep 2 ms */
      count--;                         /* decrement count                    */
   }
                                       /* returning from polltabt           */
   return((int) (count ? SUCCESS : -1 ) );
                                       /* END polltabt()                    */
} /* polltabt() */
   

/* This function just sends data, which is stored in the 'value' parameter,
   to the tablet. */

int SendTbDatat(fd, value)
int fd;
unsigned char value;
{  
  unsigned int count;
  unsigned char Status, cvalue;
  char msg[80];
  int rc = SUCCESS;

  for(count = 0; count < REPEAT_COUNT; count++)
  {

    clear_transmitt(fd);  /* Clear transmitter */

  /* Read from Line status reg. */
    rc = rd_bytet(fd, &Status, TABLET_LINE_ST_REG);
    if (rc == SUCCESS)
    {
      if (Status & 0x20)  /* Is Transmitter empty? */
      {
        if ((rc = rd_bytet(fd, &cvalue, TABLET_LINE_CTRL_REG)) != SUCCESS) {
         PRINTERR("Unsuccessful read of one byte from TABLET_LINE_CTRL_REG\n");
         return(rc);
        }
      /* Set DLAB of Line control reg to 0 */

        cvalue = cvalue & 0x7f;
        if ((rc = wr_bytet(fd, &cvalue, TABLET_LINE_CTRL_REG)) != SUCCESS) {
         PRINTERR("Unsuccessful write of one byte to TABLET_LINE_CTRL_REG\n");
         return(rc);
        }
        rc = wr_bytet(fd, &value, TABLET_TX_BUFF_REG);  /* Write to r/w reg */
      }
    }
    if ((rc == SUCCESS) && ((Status & 0x20) == 0))
      usleep(10000);
    else
      break;
  }
  if((Status & 0x20) == 0)
  {
    sprintf(msg,"Could not send data : TX always busy - Not able to send %x\n",value);
    PRINTERR(msg);
    rc = -1;
  }
return(rc);
}

/* This function checks if the tablet adapter is ready to receive data. */

clear_transmitt(fd)
int fd;   /* File descriptor for machine DD */
{
  int i;
  uchar data;

  /* Read from Line status reg. */
  i = 0;
  data = 0;

 /* Is transmitter not empty or a character in Recv. buffer reg.? */

  rd_bytet(fd, &data, TABLET_LINE_ST_REG);

  while ( ( (!(data & 0x20)) || (data & 0x01) ) && (i < 200) ) {  /* If transmit
ter not empty or data is ready */
   /* Read from Recv. buffer */
    rd_bytet(fd, &data, TABLET_RX_BUFF_REG);
    usleep(10 * 1000);

    rd_bytet(fd, &data, TABLET_LINE_ST_REG);
    usleep(10 * 1000);
    i++;
  }

}


/* This function uses the machine device driver to read one byte from
   the specified address, returning the information to pdata */

int rd_bytet(fd, pdata, addr)
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
/*
  printf("Read byte = %2X\n",*pdata);
  printf("Read addr = %4X\n",addr);
*/

  return (rc);
}

/* This function uses the machine device driver to read one byte from
   the specified pos register, returning the information to pdata */

int rd_post(fd, pdata, addr)
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

int rd_wordt(fd, pldata, addr)
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

int wr_bytet(fd, pdata, addr)
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
/*
  printf("Write byte = %2X\n",*pdata);
  printf("Write addr = %4X\n",addr);
*/

  return (rc);
}

/* This function uses the machine device driver to write one byte from
   pdata to the specified pos register address */

int wr_post(fd, pdata, addr)
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

int wr_wordt(fd, pldata, addr)
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

