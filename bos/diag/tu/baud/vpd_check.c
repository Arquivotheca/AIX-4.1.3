static char sccsid[] = "@(#)80  1.2  src/bos/diag/tu/baud/vpd_check.c, tu_baud, bos411, 9439B411a 9/29/94 10:57:41";
/*
 * COMPONENT_NAME: tu_baud
 *
 * FUNCTIONS: adapter_reset, vpd_check, vpd_test(), accum_crc()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * MODULE NAME: vpd_check.c
 *
 */
#include   <stdio.h>
#include   <fcntl.h>
#include   <sys/rcm_win.h>
#include   <sys/aixgsc.h>
#define HFRDATA 1024

#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/mdio.h>
#include   <signal.h>
#include "baud_exectu.h"
#include "baudregs.h"


/********************************************************/
/*           Macro Definitions                          */
/********************************************************/
/* used to assign an integer into an array */
/* pass in the address of the first byte   */
/* in the array to be assigned into and the*/
/* integer your are assigning to it.       */
#define INT_TO_ARRAY(a,i)  *((char *) (a))     = (i) >> 24;\
                           *((char *) (a) + 1) = (i) >> 16;\
                           *((char *) (a) + 2) = (i) >> 8; \
                           *((char *) (a) + 3) = (i);

/* used to assign a short  into  an  array */
/* pass in the address of the first byte   */
/* in the array to be assigned into and the*/
/* short your are assigning to it.         */
#define SHORT_TO_ARRAY(a,i)  *((char *) (a))     = (i) >> 8;\
                             *((char *) (a) + 1) = (i);


   union {              /* data structure used to accumulate CRC */
      ushort  whole;
      struct {
         char msb;
         char lsb;
      } bite;
   } avalue, dvalue;


void accum_crc(char data)
   {
   avalue.bite.lsb = (data ^ dvalue.bite.lsb);
   dvalue.bite.lsb = avalue.bite.lsb;
   avalue.whole    = ((avalue.whole << 4) ^ dvalue.bite.lsb);
   dvalue.bite.lsb = avalue.bite.lsb;
   avalue.whole   <<= 8;

   avalue.whole   >>= 1;
   avalue.bite.lsb ^= dvalue.bite.lsb;
   avalue.whole   >>= 4;

   avalue.whole     = (avalue.bite.lsb << 8) | avalue.bite.msb;
   avalue.whole     = ((avalue.whole & 0xFF07) ^ dvalue.bite.lsb);
   avalue.whole     = (avalue.bite.lsb << 8) | avalue.bite.msb;
   avalue.bite.lsb ^= dvalue.bite.msb;
   dvalue.whole     = avalue.whole;

   }


/*
 * NAME: adapter_reset()
 *
 * FUNCTION: Software reset of adapter and initialize registers.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 */
int adapter_reset()
{
extern char *ioa;

   *(ioa + MCI_REG0) = SW_RESET | 0x30;
   *(ioa + MCI_REG0) = 0x30;
   wait(100);
   *(ioa + MCI_INTR_EN) = 0;
   *(ioa + MCI_INTR_AK) = TMR_INTR | INT4 | INT3 | INT2 | INT1;
   return(0);

}  /* end adapter reset */

/*
 * NAME: pos_test()
 *
 * FUNCTION: tests BIM's POS register by making an ioctl() call.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 */
pos_test(int *secondary_ptr)
{
   return(0);

}  /* end pos_test()  */

/*
 * NAME: check_vpd()
 *
 * FUNCTION: This function tests a vpd ROM.  It check for the word 'VPD'
 *           in the VPD ROM data and it calculates CRC for the vpd data in ROM
 *           and compares it against a pre-stored CRC value in VPD ROM.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * RETURNS : Zero if successful, non-zero if error.
 *
 * OUTPUT: If return code the following secondary
 *
 */


int  vpd_check(int *secondary_ptr)
{
extern char *pos;

   int ret_code;
   int     j, k;
   char    data, field[4], vpdid[5], val;
   int     vpdlen = 0;
   int     fldlen = 0;
   int     index  = 0;   /* Begin reading vpd here */
   int     datalen = 0;
   ushort  crc = 0;
   char    orig_pos3;

   field[3] = '\0';
   vpdid[4] = '\0';

   orig_pos3 = *(pos+3);

   *(pos+6) = index++;  val = *(pos+3);
   *(pos+6) = index++;  val = *(pos+3);
   vpdid[0] = val; /* ignore First Byte  - Read 'V' from VPD ROM. */

   *(pos+6) = index++;  val = *(pos+3);
   vpdid[1] = val;                           /* Read 'P' from VPD ROM */

   *(pos+6) = index++;  val = *(pos+3);
   vpdid[2] = val;                           /* Read 'D' from VPD */
   vpdid[3] = '\0';

   if (strncmp(vpdid,"VPD",3) != 0) {
      secondary_ptr[0] = 0;
      secondary_ptr[1] = ('V' << 16) | ('P' << 8) | 'D';
      secondary_ptr[2] = (vpdid[0] << 16) | (vpdid[1] << 8) | (vpdid[2]);
      *(pos+3) = orig_pos3;
      *(pos+6) = 0;
      return (VPD_READ);
      } /* end if */

   *(pos+6) = index++;  val = *(pos+3);
   (*(((char *) &vpdlen) + 2)) = val;     /* Get 1st byte of length. */

   *(pos+6) = index++;  val = *(pos+3);
   (*(((char *) &vpdlen) + 3)) = val;     /* Get 2nd byte of length. */
   vpdlen = vpdlen << 1;                   /* Byte count = 2 * length */

   *(pos+6) = index++;  val = *(pos+3);
   (* (((char *) &crc) + 0)) = val;       /* Get 1st byte of CRC. */

   *(pos+6) = index++;  val = *(pos+3);
   (* (((char *) &crc) + 1)) = val;       /* Get 2nd byte of CRC. */

   dvalue.whole = 0xFFFF;

   for (j=0; j<vpdlen; j+=4) {             /* Start Reading Field Data. */
     *(pos+6) = index++;  val = *(pos+3);  /*  Read 1st value. */
     accum_crc(val);                         /* Accumulate CRC. */
     field[0] = val;
     if (field[0] != '*') {                /* Verify Field Start. */
        secondary_ptr[0] = 0;
        secondary_ptr[1] = '*';
        secondary_ptr[2] = field[0];
        *(pos+3) = orig_pos3;
        *(pos+6) = 0;
        return(VPD_READ);
        };/* endif */

     /* Read Field ID's first letter */
     if (j < (vpdlen - 3)) {
        *(pos+6) = index++;  val = *(pos+3);  /* Read 1rst field letter */
        field[1] = val;                       /* and accumulate CRC. */
        accum_crc(val);

        /* Read Field ID's second letter */
        *(pos+6) = index++;  val = *(pos+3);
        field[2] = val;                  /* and accumulate CRC. */
        accum_crc(val);

        /* read field length */
        *(pos+6) = index++;  val = *(pos+3);
        fldlen = (int)val;
        accum_crc(val);                      /* accumulate CRC. */

        datalen = (fldlen<<1) - 4;        /* Calculate data length */

        /* Read the remainder of the field */
        for (k=0; k<datalen; k++,j++) {   /* Read/print field remainder. */
          *(pos+6) = index++;  val = *(pos+3);
          accum_crc(val);

          }; /* endfor */
        }; /* endif */
     };/* endfor  */

   if (crc != dvalue.whole) {
     secondary_ptr[0] = 0;
     secondary_ptr[1] = (int)crc;
     secondary_ptr[2] = (int)dvalue.whole;
     *(pos+3) = orig_pos3;
     *(pos+6) = 0;
     return (VPD_CHECKSUM);
     };/* endif */

   *(pos+3) = orig_pos3;
   *(pos+6) = 0;
   return (0);

}  /* end vpd_check */



/*
 * NAME: mci_timer_test()
 *
 * FUNCTION: Tests MCI chips timer function and system interrupt path.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 */
int mci_timer_test(int *secondary_ptr)
{
extern   char  *ioa;
int error = 0;

/* DISABLE TIMER AND INTERRUPTS */
*(ioa + MCI_REG0) = *(ioa + MCI_REG0) & ~EN_TMR;
*(ioa + MCI_INTR_EN) = 0;
*(ioa + MCI_INTR_AK) = TMR_INTR;    /* CLEAR PENDING INTERRUPTS */

/* TEST TO SEE IF INTERRUPTS ARE CLEARED */
if( *(ioa + MCI_INTR_ST) != 0)
  {
  error = MCI94C18A_BAD;
  secondary_ptr[0] = 0x1;
  };

/* RUN COUNTER AND CHECK FOR STATUS SET (NOT ENABLED) */
if(error == 0)
  {
  *(ioa + MCI_TIMER) = 0x55;                        /* PRELOAD TIMER */
  *(ioa + MCI_REG0) = *(ioa + MCI_REG0) | EN_TMR;   /* LET TIMER RUN */
  sleep(1);
  *(ioa + MCI_REG0) = *(ioa + MCI_REG0) & ~EN_TMR;   /* stop timer */
  if(*(ioa+MCI_INTR_ST) != 0xC0)
    {
/*  printf("Status not set\n");   */
    error = MCI94C18A_BAD;
    secondary_ptr[0] = 0x2;
    }
  else
    {
    /* CAUSE INTERRUPT AND CHECK FOR HAPPENING */
    *(ioa + MCI_INTR_EN) = TMR_INTR;    /* SET ENABLE TO CAUSE INTERRUPT */
    sleep(1);
    /* VERIFY THAT INTERRUPT HANDLER HAS CLEARED THE INTERRUPT */
    /* AND DISABLED THE TIMER FROM RUNNING */
    if(*(ioa+MCI_INTR_ST) != 0)
      {
      error = MCI94C18A_BAD;
      secondary_ptr[0] = 0x3;
      };
    };
  };

/* DISABLE TIMER AND INTERRUPTS */
*(ioa + MCI_REG0) = *(ioa + MCI_REG0) & ~EN_TMR;
*(ioa + MCI_INTR_EN) = 0;
*(ioa + MCI_INTR_AK) = 0xff;     /* CLEAR PENDING INTERRUPTS */

return(error);

}  /* end pos_test()  */

