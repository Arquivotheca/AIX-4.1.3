static char sccsid[] = "@(#)45  1.6  src/bos/diag/tu/sun/tvidsetup.c, tu_sunrise, bos411, 9437A411a 8/29/94 15:48:59";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: i2c_lut_write99
 *              i2c_master_negative_ack
 *              i2c_read
 *              i2c_receive_data
 *              i2c_slave_positive_ack
 *              i2c_start
 *              i2c_trans_fin
 *              i2c_write
 *              i2c_write99
 *              i2c_xmit_data
 *              init_pcd
 *              vidsetup
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************

Module Name :  tvidsetup.c

Description :  Test the video front end (7191) setup and communication.
               Also set up the operating mode of the video front end.

               INPUT:
                     mode = 0-NTSC_NOINPUT   (NTSC with no video input)
                     mode = 1-NTSC_INPUT     (NTSC with video input)
                     mode = 2-SVHS_NOINPUT   (SVHS with no video input)
                     mode = 3-SVHS_INPUT     (SVHS with video input)
                     mode = 4-PAL_NOINPUT    (PAL with no video input)
                     mode = 5-PAL_INPUT      (PAL with video input)


Modified    :  000      08/25/94
               - Write the 'chk_vsync' routine to use interrupt polling
                 to check vsync toggling  -->  Fix the intermittent fail.!!?
               - Add the video input modes.

*****************************************************************************/
#include <stdio.h>
#include <sys/times.h>
#include "vidsetup.h"
#include "suntu.h"
#include "error.h"
#include "sun_tu_type.h"

#define I2C12MHZ
#define VID_TIMEOUT     200     /* 2 seconds timeout */

/* Functions prototypes */
int vidsetup(int mode);
int init_pcd();
int i2c_trans_fin();
int i2c_slave_positive_ack();
unsigned int i2c_master_negative_ack();
int i2c_start(unsigned int address);
unsigned int i2c_xmit_data(unsigned int data_value);
int i2c_receive_data(unsigned int *data_value);
int i2c_read(unsigned int ucDevAddr, unsigned int *data);
int i2c_write(unsigned int ucAddress, unsigned int ucSubAddress,
              unsigned int *ucDataIn, int numbytes);
int i2c_write99(unsigned int ucAddress,unsigned int ucSubAddress,
                unsigned int ucIndex,unsigned int *ucDataIn,int numbytes);
int i2c_lut_write99(unsigned int ucAddress, unsigned int ucSubAddress);

int vidsetup(int mode)
{
   int i, rc;
   unsigned int bValue[256];

   /* Initialize 8584 */
   if (rc=init_pcd())
   {
      LOG_VIDSETUP_ERROR(ERRORVID_INIT8584, rc);
   }
/*    pio_write(0x1bff0000, 0x4000ac00,1); /* Initialize config register */

/*  load 7191  with default as NTSC with no input */

  bValue[0] = VAL_idel;
  bValue[1] = VAL_hsb5;
  bValue[2] = VAL_hss5;
  bValue[3] = VAL_hcb5;
  bValue[4] = VAL_hcs5;
  bValue[5] = VAL_hsp5;
  bValue[6] = VAL_lum;
  bValue[7] = VAL_hue;
  bValue[8] = VAL_cktq;
  bValue[9] = VAL_ckts;
  bValue[10] = VAL_plse;
  bValue[11] = VAL_sese;
  bValue[12] = VAL_gain;
  bValue[13] = VAL_mode_VTR;
  bValue[14] = VAL_noinput;
  bValue[15] = VAL_ctl4_422;
  bValue[16] = VAL_ctl4;
  bValue[17] = VAL_chrm_NTSC;
  bValue[18] = 0x00;
  bValue[19] = 0x00;;
  bValue[20] = VAL_hsb6;
  bValue[21] = VAL_hss6;
  bValue[22] = VAL_hcb6;
  bValue[23] = VAL_hcs6;
  bValue[24] = VAL_hsp6;

  switch(mode) {
     case NTSC_NOINPUT:    /* NTSC with no input */
        break;
     case NTSC_INPUT:    /* NTSC with input */
        bValue[14] = VAL_iock_port0;
        bValue[15] = VAL_ctl3_422;
        break;
     case SVHS_NOINPUT:    /* SVHS with no input */
        bValue[6] = VAL_lum_SVHS;
        bValue[15] = VAL_ctl3_422;
        break;
     case SVHS_INPUT:    /* SVHS with input */
        bValue[6] = VAL_lum_SVHS;
        bValue[14] = VAL_iock_port1;
        bValue[15] = VAL_ctl3_422;
        break;
     case PAL_NOINPUT:    /* PAL with no input */
        bValue[15] = VAL_ctl4_50Hz;
        bValue[17] = VAL_chrm_PAL;
        break;
     case PAL_INPUT:    /* PAL with input */
        bValue[14] = VAL_iock_port0;
        bValue[15] = VAL_ctl3_50Hz;
        bValue[17] = VAL_chrm_PAL;
        break;
   }    /* end switch */

  if (rc=i2c_write(SAA7191WrAddr,0x00,bValue,25))
   {
      LOG_VIDSETUP_ERROR(ERRORVID_LOAD7191, rc);
   }

/* load 7199  */

  bValue[0] = VAL_fmt1;
  bValue[1] = VAL_trer;
  bValue[2] = VAL_treg;
  bValue[3] = VAL_treb;
  bValue[4] = VAL_sync_VTR;
  bValue[5] = VAL_gdc;
  bValue[6] = VAL_incd;
  bValue[7] = VAL_pso;
  bValue[8] = VAL_ctl1;
  bValue[9] = VAL_ctl3;
  bValue[10] = 0x00;
  bValue[11] = 0x00;
  bValue[12] = VAL_chps;
/*  bValue[13] = 0x00; */
  bValue[13] = VAL_NTSC_fsc;
  bValue[14] = VAL_NTSC_std;

  switch(mode) {
     case NTSC_NOINPUT:
        break;
     case NTSC_INPUT:
        bValue[9] = VAL_ctl2;  /* for with input */
        break;
     case SVHS_NOINPUT:    /* SVHS with no input */
/*      bValue[13] = ???? */
/*      bValue[14] = ???? */
        break;
     case SVHS_INPUT:    /* SVHS with input */
        bValue[9] = VAL_ctl2;  /* for with input */
        break;
     case PAL_NOINPUT:    /* PAL with no input */
        bValue[13] = VAL_PAL_fsc;
        bValue[14] = VAL_PAL_std;
        break;
     case PAL_INPUT:    /* PAL with input */
        bValue[9] = VAL_ctl2;  /* for with input */
        bValue[13] = VAL_PAL_fsc;
        bValue[14] = VAL_PAL_std;
        break;
   }

  if (rc=i2c_write99(SAA7199WrAddr,SAA7199CtlAddr,0x00,bValue,15))
   {
      LOG_VIDSETUP_ERROR(ERRORVID_LOAD7199, rc);
   }

   /* Check the toggling of 7191's ODD signals */
   if (rc =  chk_odd(ODD_MASK))
   {
      LOG_VIDSETUP_ERROR(ERRORVID_CHKODD, rc);
   }

   /* Check the toggling of 7191's VSYNC signals */
   if (rc =  chk_vsync())
   {
      LOG_VIDSETUP_ERROR(ERRORVID_CHKVSYNC, rc);
   }

/*
  if (rc=i2c_write99(SAA7199WrAddr,SAA7199CtlAddr,0x0C,bValue,3))
   {
      LOG_VIDSETUP_ERROR(ERRORVID_LOAD7199, rc);
   }
*/
  return (OK);
}

int init_pcd()
{
  int rc;

     /* Initialization of PCD8584   */
     if (rc=pio_write(_Pcd_statreg, 0x80, 1)) return(rc);
     if (rc=pio_write(_Pcd_datareg, 0x55, 1)) return(rc);
     if (rc=pio_write(_Pcd_statreg, 0xA0, 1)) return(rc);

#ifdef I2C12MHZ
     if (rc=pio_write(_Pcd_datareg, 0x1C, 1)) return(rc);
#else
     if (rc=pio_write(_Pcd_datareg, 0x14, 1)) return(rc);
#endif

     if (rc=pio_write(_Pcd_statreg, 0xC1, 1)) return(rc);

     /* Initialize Local Bus Control register */
     if (rc=pio_write((_LBControl<<16), 0x0,1)) return(rc);

     return(OK);
}

/**************************************************************************
**
**      FUNCTION NAME:
**
**        i2c_trans_fin
**
**      FUNCTIONAL DESCRIPTION:
**
**        Waits for transmission complete from 8584.
**
**      RETURN VALUE:
**
**        OK OR FAILURE.
**
****************************************************************************/


int i2c_trans_fin()
{
   unsigned int pin, cnt, tbyte ;
   int rc;

   /* check PIN bit = 0 */

        pin = 1;
        cnt = 0;
        while (pin != _Pin_Bit_Zero){
            if (rc=pio_read(_Pcd_statreg, &tbyte, 1)) return(rc);
            pin = tbyte & 0x80;
            cnt++;
            if (cnt == _Loop_Brk){
                 /* I2C bus transmission not finished */
                 return(ERRORVID_I2CXMIT);
            }
         }
         return(OK);
}
/**************************************************************************
**
**      FUNCTION NAME:
**
**        i2c_slave_positive_ack
**
**      FUNCTIONAL DESCRIPTION:
**
**        Waits for an acknowledgement from the slave device.
**
**      RETURN VALUE:
**
**        OK OR FAILURE.
**
****************************************************************************/


int i2c_slave_positive_ack()
{
   unsigned int pin, lrb, cnt, tbyte ;
   int rc;

   /* check PIN bit = 0 */

        pin = 1;
        cnt = 0;
        while (pin != _Pin_Bit_Zero){
            if (rc=pio_read(_Pcd_statreg, &tbyte,1)) return(rc);
            pin = tbyte & 0x80;
            lrb = tbyte & 0x08;
            cnt++;
            if (cnt == _Loop_Brk){
                 /* I2C bus transmission not finished */
                 return(ERRORVID_I2CACK);
            }
         }

         lrb = 1;
         cnt = 0;
         while (lrb != _Slave_Ack){
             pio_read(_Pcd_statreg, &tbyte,1);
             lrb = tbyte & 0x08;
             cnt++;
             if (cnt == _Loop_Brk)
                 return(ERRORVID_I2CACK);
          }

         return(OK);

}

/****************************************************************************
**
**      FUNCTION NAME:
**
**        i2c_master_negative_ack
**
**      FUNCTIONAL DESCRIPTION:
**        Checks transmission completion and
**        Generates a negative acknowledgement from the master
**        receiving device.  This informs the slave transmitter that
**        all data has been read.
**
**      RETURN VALUE:
**
**        OK OR FAILURE.
**
****************************************************************************/
unsigned int i2c_master_negative_ack()

{
   unsigned int pin, cnt, dummy, tbyte ;

   /* check PIN bit = 0 */

        pio_write(_Pcd_statreg, 0x40, 1);
        pio_read(_Pcd_datareg, &dummy,1);
        pin = 1;
        cnt = 0;
        while (pin != _Pin_Bit_Zero){
            pio_read(_Pcd_statreg,  &tbyte,1);
            pin = tbyte & 0x80;
            cnt++;
            if (cnt == _Loop_Brk){
                 return(EROR5);
                 break;
            }
         }
        return(OK);
}

/****************************************************************************
**
**      FUNCTION NAME:
**
**        i2c_start
**
**      FUNCTIONAL DESCRIPTION:
**
**        Generates the start sequence.
**
**      RETURN VALUE:
**
**        OK OR FAILURE.
**
****************************************************************************/
int i2c_start(address)
unsigned int address;
{
  unsigned int rval, cnt;
  int rc;

     rval = 0 ;
     cnt = 0;

 /* check BBb = 1  */

    while (rval == _Bus_Busy){
         if (rc=pio_read(_Pcd_statreg, &rval,1)) return(rc);
         rval = rval & 0x01;
         cnt++;
         if (cnt == _Loop_Brk){
             /* I2C bus is busy */
             return(ERRORVID_I2CBUSY);
          }
     }
     if (rc=pio_write(_Pcd_datareg, address, 1)) return(rc);
     if (rc=pio_write(_Pcd_statreg, 0xC5, 1)) return(rc);
     return(OK);
}


/****************************************************************************
**
**      FUNCTION NAME:
**
**        i2c_xmit_data
**
**      FUNCTIONAL DESCRIPTION:
**
**        Transmits a byte of data to an I2C device.
**
**      RETURN VALUE:
**
**        OK OR FAILURE.
**
****************************************************************************/
unsigned int i2c_xmit_data(data_value)
unsigned int data_value;
{
  int rc;
        if(rc=i2c_slave_positive_ack())
        {
         /* Slave did not acknowledge transfered data byte */
         return(rc);
        } else {
           if (rc=pio_write(_Pcd_datareg, data_value, 1)) return(rc);
           return(OK);
        }
}
/****************************************************************************
**
**      FUNCTION NAME:
**
**        i2c_receive_data
**
**      FUNCTIONAL DESCRIPTION:
**
**        Reads in a byte of data from an I2C device.
**
**      RETURN VALUE:
**
**        OK OR FAILURE.
**
****************************************************************************/
int i2c_receive_data(data_value)
unsigned int *data_value;
{
         if (i2c_master_negative_ack()){
              return(EROR3);
         } else {
          pio_write(_Pcd_statreg, 0xC3, 1);
          pio_read(_Pcd_datareg,data_value,1);
          return(OK);
        }
}

/****************************************************************************
**
**      FUNCTION NAME:
**
**        i2c_read
**
**      FUNCTIONAL DESCRIPTION:
**
**        Reads data from I2C slave device.
**
**      RETURN VALUE:
**
**        OK OR FAILURE.
**
***************************************************************************/
int i2c_read(unsigned int ucDevAddr, unsigned int *data)
{
unsigned int *result;
int rc;

     if (rc=i2c_start(ucDevAddr))
        return (rc);
     if (rc=i2c_slave_positive_ack())
        return (rc);
     if (rc=i2c_receive_data(data))
        return (rc);

     return(0);
}

/****************************************************************************
**
**      FUNCTION NAME:
**
**        i2c_write
**
**      FUNCTIONAL DESCRIPTION:
**
**        Writes data to I2C slave device.
**
**      RETURN VALUE:
**
**        OK OR FAILURE.
**
****************************************************************************/
int i2c_write(ucAddress,ucSubAddress,ucDataIn,numbytes)
unsigned int ucAddress;
unsigned int ucSubAddress;
unsigned int *ucDataIn;
int numbytes;
{      int i, rc;

  if (rc=i2c_start(ucAddress)) return(rc);

/* Check Salve acknowledge and  Send subaddress. */
  if (rc=i2c_xmit_data(ucSubAddress)) return(rc);

/* Check Slave acknowledge and Transmit data. */
  for (i=0; i < numbytes; i++)
    if (rc=i2c_xmit_data(ucDataIn[i])) return(rc);

/* Send stop sequence. */
  if (rc=i2c_trans_fin())
     return(rc);
  else
  {
     if (rc=pio_write(_Pcd_statreg, 0xC3, 1)) return(rc);
     return(OK);
  }
}
/****************************************************************************
**
**      FUNCTION NAME:
**
**        i2c_write99
**
**      FUNCTIONAL DESCRIPTION:
**
**        Writes initial address, subaddress,index to I2C slave device.
**
**      RETURN VALUE:
**
**        OK OR FAILURE.
**
****************************************************************************/
int i2c_write99(ucAddress,ucSubAddress,ucIndex,ucDataIn,numbytes)
unsigned int ucAddress;
unsigned int ucSubAddress;
unsigned int ucIndex;
unsigned int *ucDataIn;
int numbytes;
{    int i, rc;

  if (rc=i2c_start(ucAddress)) return(rc);

/* Check Slave acknowledge and  Send subaddress. */
  if (rc=i2c_xmit_data(ucSubAddress)) return(rc);

/* Check Slave acknowledge and Transmit index. */
  if (rc=i2c_xmit_data(ucIndex)) return(rc);

/* Check Slave acknowledge and send data */
  for (i=0; i < numbytes; i++)
    if (rc=i2c_xmit_data(ucDataIn[i])) return(rc);

/* Send stop sequence. */
  if (rc=i2c_trans_fin())
    return(rc);
  else
  {
     if (rc=pio_write(_Pcd_statreg, 0xC3, 1)) return(rc);
     return(OK);
  }
}

/****************************************************************************
**
**      FUNCTION NAME:
**
**        i2c_lut_write99
**
**      FUNCTIONAL DESCRIPTION:
**
**        Writes lut initial address, subaddress,index to I2C slave device.
**
**      RETURN VALUE:
**
**        OK OR FAILURE.
**
****************************************************************************/
int i2c_lut_write99(ucAddress,ucSubAddress)
unsigned int ucAddress;
unsigned int ucSubAddress;
{    int i, rc;
     unsigned int ucadrRdata;

  if (i2c_start(ucAddress)) return(EROR1);

/*
*      Check Slave acknowledge and  Send subaddress.
*/

  if (i2c_xmit_data(ucSubAddress)) return(EROR1);

/*
*       Check Slave acknowledge and Transmit index.
*/


/*
*       Check Salve acknowledge and send data.
*/

  for (i=0; i < 255 ; i++){
    ucadrRdata = (unsigned int) i  ;
    if (i2c_xmit_data(ucadrRdata)) return(EROR1);
    if (i2c_xmit_data(ucadrRdata)) return(EROR1);
    if (i2c_xmit_data(ucadrRdata)) return(EROR1);
    if (i2c_xmit_data(ucadrRdata)) return(EROR1);
   }
/*
*       Send stop sequence.
*/

  if (!i2c_trans_fin()){
     pio_write(_Pcd_statreg, 0xC3, 1);
     return(OK);
  } else return(EROR5);
}

/****************************************************************************
**
**      FUNCTION NAME:
**
**        chk_odd
**
**      FUNCTIONAL DESCRIPTION:
**
**        After the video setup to the 7191, then this routine will test
**        for the ODD signal coming out of 7191 by checking this
**        bit in the Local Bus Status register.  It will check for the
**        toggling of bit 2 times.
**
**      RETURN VALUE:
**
**        OK OR TIMEOUT FAILURE.
**
****************************************************************************/
int chk_odd(unsigned int StatusMask)
{
    clock_t startTime;
    struct tms tmsstart, tmsnow;
    int i, toggle, rc;
    unsigned int StatusVal;

    /* Check for the 7191's ODD signal toggling 2 times */

    for (i=0;i<2; ) {
       startTime=times(&tmsstart);
       toggle = 0;
       while(!toggle) {
         if(rc=pio_read(LBStatus, &StatusVal, 1)) return(rc);
         StatusVal &= StatusMask;   /* Look for 1 first, */
         if (StatusVal) {           /* then look for 0   */
           while(1) {
              if(rc=pio_read(LBStatus, &StatusVal, 1)) return(rc);
              StatusVal &= StatusMask;
              if (!StatusVal)  {
                 toggle = 1;
                 i++;
                 break;
              }
              if ((times(&tmsnow)-startTime) >= VID_TIMEOUT)
                  return(ERRORVID_TIMEOUT0);
           }
         }
         if ((times(&tmsnow)-startTime) >= VID_TIMEOUT)
             return(ERRORVID_TIMEOUT1);
       }
    } /* endfor */

    return(OK);
}


/****************************************************************************
**
**      FUNCTION NAME:
**
**        chk_vsync
**
**      FUNCTIONAL DESCRIPTION:
**
**        After the video setup to the 7191, then this routine will test
**        for the VSYNC signal coming out of 7191 by using the local
**        interrupt enablement.   If Vsync is active, interrupt will be set
**        in the IV bit.
**
**      RETURN VALUE:
**
**        OK OR TIMEOUT FAILURE.
**
****************************************************************************/
int chk_vsync()
{
    clock_t startTime;
    struct tms tmsstart, tmsnow;
    int i, rc;
    unsigned int val;

  /* Disable Micro Channel interrupt thru Miami */
   pio_mcwrite (_SCP, 0x0, 1);
   pio_read(MIbaseaddr+_PROC_CFG,&val,1);
   val = val & 0xFBFF;
   pio_write (MIbaseaddr+_PROC_CFG,val,1);

   /* Clear the CBSP register */
   pio_write (MIbaseaddr+_CBSP, 0, 1);

   /* Check for the 7191's VSYNC signal toggling 2 times */
   for (i=0;i<2; ) {
       startTime=times(&tmsstart);

       /* Enable 7191 vsync interrupt */
       if (rc = pio_write (LBControl,0x40020c00,1)) return(rc);
       while(1) {
          pio_read(MIbaseaddr+_CBSP,&val,1);
          val = val & 0x2;
          /* looking for IV bit set if interrupt occurs */
          if (val == 0x2)  {  /* interrupt occured */
             /* Disable 7191 vsync interrupt */
             if (rc = pio_write (LBControl,0x00000c00,1)) return(rc);
             /* Clear the CBSP register */
             pio_write (MIbaseaddr+_CBSP, 0, 1);
             /* Write any value to Local bus status reg. to reset intr */
             if(rc=pio_write(LBStatus, 0x0, 1)) return(rc);
             usleep(500000);
             i++;
             break;
           }
           if ((times(&tmsnow)-startTime) >= VID_TIMEOUT)
              return(ERRORVID_TIMEOUT1);
        }
    } /* endfor */

    return(OK);
}
