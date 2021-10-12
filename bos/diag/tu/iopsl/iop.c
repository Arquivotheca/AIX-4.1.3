static char sccsid[] = "@(#)31  1.1.2.4  src/bos/diag/tu/iopsl/iop.c, tu_iopsl, bos411, 9428A410j 5/18/94 15:08:10";
/*
 *   COMPONENT_NAME: TU_IOPSL
 *
 *   FUNCTIONS: *               crcgen
 *              exectu
 *              rd_byte
 *              rd_word
 *              tu10
 *              tu20
 *              tu40
 *              wr_byte
 *              wr_word
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
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <diag/atu.h>
#include "salioaddr.h"
#include "misc.h"

#include <sys/iplcb.h>
#include <sys/rosinfo.h>

static int get_machine_model_tu(int);

/***************************************************************************/
/* NOTE: This function is called by Hardware exerciser (HTX),Manufacturing */
/*       application and Diagnostic application to invoke a test unit (TU) */
/*                                                                         */
/***************************************************************************/

#define	LED_BYTES		2	/* LEDs use lower 3 nibbles	*/	

int exectu(long fd, TUCB *tucb_ptr)
   {
     short unsigned int i,j;  /* Loop Index */
           int rc = SUCCESS;  /* return code */
	  int machine_model;  /* Machine model */
               char msg[80];  /* Character buffer for htx message strings */

   /* Get machine model type */

     rc = machine_model= get_machine_model_tu(fd);

     power_flag = 0;

   /* Initialize variables for system register addresses with appropriate
      addresses according to machine type */
   if (rc >= SUCCESS) {

       if (machine_model == POWER) {
	 power_stat_reg = POWER_STAT_REG_POWER;
	 nvram_addr = NVRAM_ADDR_POWER;
	 led_reg = LED_REG_POWER;
         tod_data_reg = TOD_DATA_REG_POWER;
         tod_idx_reg = TOD_IDX_REG_POWER;
	 power_flag = 1;
       }
       else {
	 power_stat_reg = POWER_STAT_REG_RSC;
	 nvram_addr = NVRAM_ADDR_RSC;
	 led_reg = LED_REG_RSC;
         tod_data_reg = TOD_DATA_REG_RSC;
         tod_idx_reg = TOD_IDX_REG_RSC;
       }

     for (i=0; i<tucb_ptr->loop; i++)
     {
       switch(tucb_ptr->tu)
        {  case   10: PRINT("NVRAM test - Executing TU 10\n");
                     rc = tu10(fd,tucb_ptr);
                     break;
           case   20: PRINT("LED test - Executing TU 20\n");
                     rc = tu20(fd,tucb_ptr); 
                     break;
           case   40: PRINT("POWER Status/Keylock Decode Register test - Executing TU 40\n");
                     rc = tu40(fd,tucb_ptr);
                     break;
           case   BATTERY_TU: PRINT("BATTERY test \n");
                     rc = battery_tu(fd,tucb_ptr);
                     break;
           case   TOD_TU: PRINT("TIME OF DAY test \n");
                     rc = tod_tu(fd,tucb_ptr);
                     break;
           case   VPD_TU: PRINT("VPD test \n");
                     rc = vpd_tu(fd,tucb_ptr);
                     break;
           default : rc = WRONG_TU_NUMBER;
		     return(rc);
        }  /* end case */
       
	if ((rc != SUCCESS) && (tucb_ptr->tu != 40))
	   return(rc);

     }  /* i for loop */
   }  /* if */ 
   return(rc);   /* indicate there was no error */
 }  /* End function */

/* Define for crcgen function */
unsigned long crcgen();

int tu10(fd, tucb_ptr)
   long     fd ;
   TUCB *tucb_ptr;
{
   unsigned char nvram[768];	
   unsigned long crcval;
   unsigned int ldata;	
   MACH_DD_IO iob;
   int i, rc = SUCCESS;
   char msg[80];

  for (i = 0; i < 768; i++)
   {
   iob.md_data = &nvram[i];
   iob.md_incr = MV_BYTE;
   iob.md_size = 1;
   iob.md_addr = (ulong)(nvram_addr + i); 

   /* Obtain values from NVRAM */
     if ((rc = ioctl(fd, MIONVGET, &iob)) != SUCCESS) 
      return(rc);
   }

  /* Call crcgen routine to compute 32 bit crc */
   crcval = crcgen(nvram,768); 

   if (crcval != 0)
	{
	  sprintf(msg,"Calculated crc is not correct, crcval = %x\n",crcval);
      PRINTERR(msg);
	  rc = NVRAM_ERROR;
    }

  return(rc);
}


int tu20(fd, tucb_ptr)
long fd;
TUCB *tucb_ptr;
{
   unsigned int ldata, lsavdata, lled;	
   MACH_DD_IO iob;
   int i, rc = SUCCESS;
   char msg[80];

/* Save original LED's contents   */
	
	iob.md_data = (char *) &lsavdata;
	iob.md_incr = MV_WORD;
	iob.md_size = 1;
 	iob.md_addr = (ulong) led_reg;

 /* Obtain current led values from NVRAM */
     if ((rc = ioctl(fd, MIONVGET, &iob)) != SUCCESS) {
      rc = -1;
      return(rc);
     }

 /* Write LED value '666' to LED NVRAM */

   if ((rc = ioctl(fd, MIONVLED, 0x00000666)) != SUCCESS) {
     sprintf(msg,"Putting 666 in NVRAM, errno value : %d\n",errno);
     PRINTERR(msg);
     rc = -1;
     return(rc);
   }

 /* Read back data from LED NVRAM */
   iob.md_addr = (ulong) led_reg;
   iob.md_data = (char *) &lled;
   if((rc = ioctl(fd, MIONVGET, &iob)) != SUCCESS) {
    rc = -1;
    return(rc);
   }
  
   sleep(2);
   
 /* Verify test led values are in LED NVRAM */

   if ( (lled & 0xfff00000) != 0x66600000)
   {
    sprintf(msg,"Incorrect led value in NVRAM, data = %X\n",lled);
    PRINTERR(msg);
    rc = LED_NVRAM_ERROR;
    return(rc);
   }

 /* Write LED value '999' to LED NVRAM */

   if((rc = ioctl(fd, MIONVLED, 0x00000999)) != SUCCESS) {
    sprintf(msg,"Putting 999 into LED NVRAM, errno value : %d\n",errno);
    PRINTERR(msg);
    rc = -1;
    return(rc);
   }

  /* Read back data from LED NVRAM */
   iob.md_data = (char *) &lled;
   iob.md_addr = (ulong) led_reg;
   if((rc = ioctl(fd, MIONVGET, &iob)) != SUCCESS) {
    rc = -1;
    return(rc);
   }

   sleep(2);
   
 /* Verify test led values are in LED NVRAM */


   if ( (lled & 0xfff00000) != 0x99900000)
   {
    sprintf(msg,"Incorrect led value in NVRAM, data = %X\n",lled);
    PRINTERR(msg);
    rc = LED_NVRAM_ERROR;
    return(rc);
   }

 /* Return original led values to NVRAM */
   
   lsavdata = lsavdata >> 20;
   rc = ioctl(fd, MIONVLED, (ulong)lsavdata);

  return(rc);
}


int tu40(fd, tucb_ptr)
long fd;
TUCB *tucb_ptr;
{
   unsigned int ldata;	
   MACH_DD_IO iob;
   int rc = SUCCESS;

 /* Read from the POWER_STAT_REG */
  iob.md_data = (char *)&ldata; 
  iob.md_incr = MV_WORD;
  iob.md_size = 1;
  iob.md_addr = power_stat_reg; 

  if (power_flag) {
   if ((rc = ioctl(fd, MIOGETKEY, &iob)) != SUCCESS) {
    PRINTERR("Unsuccessful read of one word from POWER_STAT_REG\n");
    rc = -1;
    return(rc);
   }
  }

  else {
   if ((rc = ioctl(fd, MIOCCGET, &iob)) != SUCCESS) {
    PRINTERR("Unsuccessful read of one word from POWER_STAT_REG\n");
    rc = -1;
    return(rc);
   }
  }

 /* Initialize TUCB structure with keylock position for DA purposes */
   switch (ldata & 0x00000003)
      /* key is in 'secure' position */
    {  case 0x00000001: rc = 1;
			PRINT("Key is in 'secure' position");
		        break;
      /* key is in 'service' position */
       case 0x00000002: rc = 2;
			PRINT("Key is in 'service' position");
			break;
      /* key is in 'normal' position */
       case 0x00000003: rc = 3;
			PRINT("Key is in 'normal' position");
			break;
      /* reserved value '00' */
       case 0x00000000: rc = 0;
			break;
    }

  return(rc);
}

/* This function uses the machine device driver to read one byte from
   the specified address, returning the information to pdata */

int rd_byte(int fd, unsigned char *pdata,unsigned int addr)
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

/* This function uses the machine device driver to read one word from
   the specified address, returning the information to pldata */

int rd_word(int fd, unsigned int *pldata,unsigned int addr)
{  
  MACH_DD_IO iob;
  int rc;
	
  iob.md_data = (char *)pldata; 
  iob.md_incr = MV_WORD;
  iob.md_size = sizeof(*pldata);
  iob.md_addr = addr; 

  rc = ioctl(fd, MIOBUSGET, &iob);
/*  printf("Read word = %4X\n",*pldata);
  printf("Read addr = %4X\n",addr);
*/
  return (rc);
}

/* This function uses the machine device driver to write one byte from
   pdata to the specified address */

int wr_byte(int fd, unsigned char *pdata,unsigned int addr)
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

/* This function uses the machine device driver to write one word from
   pldata to the specified address */

int wr_word(int fd, unsigned int *pldata,unsigned int addr)
{  
  MACH_DD_IO iob;
  int rc;
	
  iob.md_data = (char *)pldata; 
  iob.md_incr = MV_WORD;
  iob.md_size = sizeof(*pldata);
  iob.md_addr = addr; 

  rc = ioctl(fd, MIOBUSPUT, &iob);
/*  printf("Write word = %4X\n",*pldata);
  printf("Write addr = %4X\n",addr);
*/
  return (rc);
}

 /* The two following arrays are used in the crcgen routine to compute
	the 32 bit crc for NVRAM */

static ulong crctl[16] = {  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
            				0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
		            		0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
				            0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd };
static ulong crcth[16] = { 0x00000000, 0x4c11db70, 0x9823b6e0, 0xd4326d90,
				           0x34867077, 0x7897ab07, 0xaca5c697, 0xe0b41de7,
				           0x690ce0ee, 0x251d3b9e, 0xf12f560e, 0xbd3e8d7e,
				           0x5d8a9099, 0x119b4be9, 0xc5a92679, 0x89b8fd09 };

unsigned long crcgen(buf,len)
unsigned char *buf;
unsigned int len;
{

  ulong i;
  ulong temp, accum;

  accum = 0; 
  for (i = 0; i < len; i++)
  {
	temp = (accum >> 24) ^ *buf++;
	accum <<= 8;
	accum ^= crcth[ temp/16 ];
	accum ^= crctl[ temp%16 ];
  }
  return(accum);
}

#define SAL1_MODEL                   0x41   /* SAL1_MODEL (rosinfo.h)*/
#define SAL2_MODEL                   0x45   /* SAL2_MODEL (rosinfo.h)*/
#define CAB_MODEL                   0x43   /* CAB_MODEL (rosinfo.h)*/
#define CHAP_MODEL                  0x47   /* CHAP_MODEL (rosinfo.h)*/
#define RBW_MODEL                   0x46   /* RBW_MODEL (rosinfo.h)*/
#define INVALID_MACHINE_MODEL       -1

int get_machine_model_tu(int mdd_fdes)
{
  MACH_DD_IO    mdd;
  IPL_DIRECTORY *iplcb_dir;                                       /* iplcb.h */
  IPL_INFO      *iplcb_info;
  int  rc;
  int  machine_model;

  machine_model = INVALID_MACHINE_MODEL;       /* default, invalid model   */

  rc = SUCCESS;

    iplcb_dir = (IPL_DIRECTORY *) malloc (sizeof(IPL_DIRECTORY));
    iplcb_info = (IPL_INFO *) malloc (sizeof(IPL_INFO));
    if ( (iplcb_dir == ((IPL_DIRECTORY *) NULL))||
         (iplcb_info == ((IPL_INFO *) NULL)) ) {
        machine_model = INVALID_MACHINE_MODEL;
    } else {
        mdd.md_incr = MV_BYTE;
        mdd.md_addr = 128;
        mdd.md_data = (char *) iplcb_dir;
        mdd.md_size = sizeof(*iplcb_dir);
        if ( ioctl(mdd_fdes, MIOIPLCB, &mdd) ) {
          machine_model = INVALID_MACHINE_MODEL;
        } else {
            mdd.md_incr = MV_BYTE;
            mdd.md_addr = iplcb_dir -> ipl_info_offset;
            mdd.md_data = (char *) iplcb_info;
            mdd.md_size = sizeof(*iplcb_info);
            if ( ioctl(mdd_fdes, MIOIPLCB, &mdd) ) {
              machine_model = INVALID_MACHINE_MODEL;
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
               if ( (machine_model == SAL1_MODEL) || (machine_model == SAL2_MODEL) || (machine_model == CAB_MODEL) || (machine_model == CHAP_MODEL) )
               {
                 machine_model = RSC;
               }
               else {
                 machine_model = POWER;
               }

                free (iplcb_dir);
                free (iplcb_info);
            } /* endif */
        }  /* end if */
    }  /* end if */
    rc = machine_model;

  return  (rc);
}


