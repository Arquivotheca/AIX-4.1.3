static char sccsid[] = "@(#)26  1.14  src/bos/diag/tu/corv/corv_atu.c, tu_corv, bos41J, 9511A_all 2/28/95 16:40:39";
/*
 * COMPONENT_NAME: (TU_CORV) Corvette adapter Test Units
 *
 * FUNCTIONS: exectu
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*   FILE NAME: corv_atu.c                                                */
/*   FUNCTION:  Corvette adapter Application Test Units.                  */
/*                                                                        */
/*    This source file contains source code for the Corvette adapter's    */
/*    Application Test Units to aid in various testing environments       */
/*    of the corvette adapter. These test units provide a basic inter-    */
/*    face between the diagnostic application program and functions       */
/*    written in the diagnostic extension (diagex) which provide direct   */
/*    access to the device without the need for a device driver.          */
/*                                                                        */
/*                                                                        */
/*   EXTERNAL PROCEDURES CALLED:                                          */
/*                                                                        */

/* INCLUDED FILES */
#include <sys/cfgdb.h>
#include <sys/sysconfig.h>
#include <cf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>

#include "corv_atu.h"

/*- #define CORVETTE_DEBUG -*/    /*- remove only to cause screen printing -*/
                                  /*- during corvette diagex code for debug -*/

/* Defined ranges for POS Register 5: Master Control */
#define ENABLE_STREAMING 1
#define DISABLE_STREAMING 0
#define ENABLE_RETURN_CHECKING 1
#define DISABLE_RETURN_CHECKING 0
#define ENABLE_DATA_PARITY 1
#define DISABLE_DATA_PARITY 0
#define ENABLE_WAIT_STATE 1
#define DISABLE_WAIT_STATE 0
#define CONCAT_IO_ADDRESS 1
#define SELECT_IO_RANGE 0

#define CRC_MASK        0XFF07

/*- global variables -*/
static adapter_diagnose_state = 0;  /*- set = 1 when in Diagnose state -*/


/*
 * NAME: db()
 *
 * FUNCTION: write debug trace information to a file.
 *
 * The calls to db() are used for debug. If the debug file exists,
 * debug information will be written to it. If the debug file does
 * not exist, db() will simply return. To turn on debug, touch
 * "/tmp/.TU_CORV_DEBUG" and that will be the debug file. To append
 * to the debug file each time the test units are ran, export
 * "TU_CORV_DEBUG=APPEND" before running.
 *
 */
 void db(ptx_corv,sinfo,actn)
  CORV_TUTYPE  *ptx_corv;
     char *sinfo;       /* passed text information */
      int actn;         /* 0 = start new file      */
 {
      int rc;
     char fname[256];
     FILE *dbfptr;
   struct stat file_status;

  strcpy(fname,"/tmp/.TU_CORV_DEBUG");

  if ((rc = stat(fname,&file_status)) == -1) {
   errno = 0;
   return;     /* debug file not present */
  }

  if (!strcmp((char *)getenv("TU_CORV_DBUG"),"APPEND") || (actn!=0))
    dbfptr = fopen(fname,"a+");
  else
    dbfptr = fopen(fname,"w+");

  switch(actn) {
   case 0 : {
    fprintf(dbfptr,"--start debug----\n");
    fprintf(dbfptr,"tu# %d : %s",ptx_corv->tu,sinfo);
    break; }
   case 1 : {
    fprintf(dbfptr,"tu# %d : %s",ptx_corv->tu,sinfo);
    break; }
  default : {
    fprintf(dbfptr,"[%s]\n",sinfo);
    break; }
  }
  fflush(dbfptr);
  if (dbfptr != NULL)
    fclose(dbfptr);
  return;
}

/*
 * NAME: init_env()
 *
 * FUNCTION: initialize environment variables used within corvette
 *  diagex interface code.
 *
 * RETURNS:
 *          0 = no error encountered setting environment.
 *         -1 = error encountered setting environment.
 */
 int init_env(ptx_corv)
  CORV_TUTYPE *ptx_corv;
 {
  int rc;
  static char tstr[100];
  static char xstr[100];

  /*- do CORVETTE_INTERRUPT var separately -*/
  strcpy(tstr,getenv("DIAGX_SLIH_DIR"));
  strcpy(xstr,"CORVETTE_INTERRUPT=\0");
  strcat(xstr,tstr);
  strcat(xstr,"/corv_slih\0");
  rc = putenv(xstr);              /*- set CORVETTE_INTERRUPT= to      */
                                  /*- $DIAGX_SLIH_DIR/corvette_intr   */
  (void) db(ptx_corv,xstr,1);     /*- log env var in debug file */
  (void) db(ptx_corv,"\n",1);

  if (rc == 0) {
   rc = -1;
   /*-- initialize corvette diagex interface environment variables --*/
   if (!putenv("TRACE_PRINTING=OFF"));
    if (!putenv("DIAGEX=/usr/lib/drivers/diagex"));
     if (!putenv("TIMEOUT_VALUE=3000000"));
      if (!putenv("ERROR_PRINTING=OFF"));
       if (!putenv("ERROR_FORMAT=VERBOSE"));
        if (!putenv("TRACE_FORMAT=VERBOSE"));
         if (!putenv("TESTCASE_RECORDING=OFF"));
          rc = 0;
  }

   /*-- set to cause trace printing to screen within corvette diagex code -*/
#ifdef CORVETTE_DEBUG
   if (rc == 0) {
    rc = -1;
    if (!putenv("TRACE_PRINTING=SCREEN"));
     if (!putenv("ERROR_PRINTING=SCREEN"));
      if (!putenv("TESTCASE_RECORDING=SCREEN"));
       rc = 0;
   }
#endif
  return(rc);
 }

/*****************************************************************************
*   Function name - combine
*
*   Purpose - This function can be used to swap the two bytes of a two byte
*             value, or to concatenate two separate bytes into a two byte
*             value.  The bytes are passed in the order they are to be
*             combined in.
*
*   Inputs - byte1:  most significant byte
*            byte2:  least significant byte
*
*   Return - 16 bit value make up of the passed bytes.
*
*   Mods -
*
*****************************************************************************/
ushort combine(uchar byte1, uchar byte2)
                 /* byte1 msb */
                 /* byte2 lsb */
   {   /* start of combine() */
        static unsigned short ret_value;

        ret_value = (byte1 << 8) | byte2;
        return(ret_value);
   }  /* end of combine() */

/****************************************************************************
 * Function: int crc_gen()
 *
 * Function calculates CRC.  Based on modified algorithms by
 * listing from PC/RT Hardware Technical Reference Manual,
 * Volume I.
 *****************************************************************************/
int crc_gen(uchar *buf, int len)
   {
        union accum
           {
           ushort whole;        /* defines entire 16 bits */
           struct bytes
                {                       /* used to define 2 bytes */
                uchar msb;
                uchar lsb;
                } byte;
           } avalue, dvalue;

        static ushort ret_crc;  /* value to be returned */
        uchar datav;
        int i;

        dvalue.whole = 0xffff;
        avalue.whole = 0x0000;  /* rfc */

/**************************************************************************
  Operate on each byte of data in the passed array, sending the data through
  the algorithm to generate a crc.  Then use the crc just generated as the
  base for the next pass of the data through the algorithm.
***************************************************************************/

        for (i = 0; len > 0; i++, len--)
           {                            /* step through the CRC area */
           datav = *(buf + i);  /* GET BYTE */
           avalue.byte.lsb = datav ^ dvalue.byte.lsb;
           dvalue.byte.lsb = avalue.byte.lsb;
           avalue.whole = (avalue.whole * 16) ^ dvalue.byte.lsb;
           dvalue.byte.lsb = avalue.byte.lsb;
           avalue.whole <<= 8;

           avalue.whole >>= 1;
           avalue.byte.lsb ^= dvalue.byte.lsb;

           avalue.whole >>=4;

           avalue.whole = combine(avalue.byte.lsb, avalue.byte.msb);
           avalue.whole = (avalue.whole & CRC_MASK) ^ dvalue.byte.lsb;
           avalue.whole = combine(avalue.byte.lsb, avalue.byte.msb);
           avalue.byte.lsb ^= dvalue.byte.msb;
           dvalue.whole = avalue.whole;
           }                            /* end step through the CRC area */
        ret_crc = dvalue.whole;
        return(ret_crc);
   } /* end of crc_gen */

/*
 * NAME: exectu
 *
 * FUNCTION:  Execute a specific CORV Test Unit.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called as a subroutine of a diagnostic
 *      application.
 *
 * NOTES:  This routine will accept commands to perform specific test
 *         units on CORVETTE adapters. Supported test units are:
 *
 *         COATU_INIT_TUS          (0x01) - intialize adapter for test units
 *         COATU_TERM_TUS          (0x02) - terminate test / resore adapter/children
 *         COATU_REG_TEST          (0x03) - command interface register test
 *         COATU_PACE_CMD          (0x04) - pace command test
 *         COATU_SCB_CMD           (0x05) - subsystem control block test
 *         COATU_ADDR_TEST         (0x06) - card address line test
 *         COATU_MCODE_DWNLD       (0x08) - download microcode to the adapter
 *
 * DATA STRUCTURES:
 *
 * INPUTS:
 *
 * RETURNS:
 * RETURN VALUE DESCRIPTION:
 *
 *      COATU_SUCCESS           (0)    - test unit completed nornmally.
 *
 *      COATU_TIMEOUT           (-10)  - timeout occurred before test
 *                                       unit completion.
 *      COATU_NOT_DIAGNOSE      (-11)  - adapter not set to diagnose
 *                                       state.
 *      COATU_FAILED            (-12)  - test unit failed to complete
 *                                       normally.
 *      COATU_INVALID_PARAM     (-13)  - invalid parameter passed to
 *                                       test unit.
 *      COATU_UNABLE_UNCONFIG   (-14)  - adapter or child can not be
 *                                       unconfigured.
 *
 *      COATU_ALREADY_DIAGNOSE  (-20)  - attempted to set diagnose when
 *                                       already in diagnose state.
 *      COATU_UNABLE_DIAGNOSE   (-21)  - unable to set adapter to diagnose
 *                                       state.
 *      COATU_UNABLE_OPEN       (-24)  - unable to open the adapter in diagex
 *                                       mode.
 *      COATU_UNABLE_DEFINE     (-25)  - unable to place adapter in to
 *                                       defined state.
 *      COATU_UNABLE_RECONFIG   (-26)  - unable to reconfigure adapter
 *                                       or children to their original state.
 *
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 *
 * LOCAL PROCEDURES CALLED:
 *
 *
 */

 int exectu(ptx_corv)
  CORV_TUTYPE *ptx_corv;
 {
  int i;
  int loopcount;
  int response;
  int completion_status;
  int ret_status;

  DMA_STRUCT buffer, TSB;
  ADAPTER_COMMAND corvette_command;

  loopcount = ptx_corv->loop;      /* number of times to repeat a command */
  loopcount++;                     /* zero means one */

  ptx_corv->isr = 0;
  ret_status = COATU_FAILED;

  /*---------------------------------------*/
  /* assure adapter is proper state        */
  /* before attempting test unit           */
  /*---------------------------------------*/
  if ((ptx_corv->tu != 1) &&          /*- for tus other than init tu -*/
      (adapter_diagnose_state != 1))  /*- test for NOT Diagnose state -*/
     return(COATU_NOT_DIAGNOSE);      /*- must be in diagnose state   -*/
  else
   if ((ptx_corv->tu == 1) &&         /*- for tu 1 only           -*/
       (adapter_diagnose_state == 1)) /*- test for Diagnose state -*/
     return(COATU_ALREADY_DIAGNOSE);  /*- already diagnose state  -*/

  switch (ptx_corv->tu) {

   /*--------------------------------------*/
   /*- INITIALIZE Test Units #1           -*/
   /*--------------------------------------*/
    case COATU_INIT_TUS: {
     ret_status = Do_INIT_TUS(ptx_corv);
     if (ret_status == COATU_SUCCESS)
      adapter_diagnose_state = 1;   /*- flag Diagnose state -*/
     return (ret_status);
     break;
    }

   /*--------------------------------------*/
   /*- TERMINATE Test Units #2            -*/
   /*--------------------------------------*/
    case COATU_TERM_TUS: {
     ret_status = Do_TERM_TUS(ptx_corv);
     if (ret_status == COATU_SUCCESS)
      adapter_diagnose_state = 0;   /*- reset Diagnose state -*/
     return (ret_status);
     break;
    }

   /*-----------------------------------------*/
   /*- Command Interface Register Test TU #3 -*/
   /*-----------------------------------------*/
    case COATU_REG_TEST: {
     ret_status = Do_REG_TEST(ptx_corv);
     return (ret_status);
     break;
    }

   /*--------------------------------------*/
   /*- Test Immediate Pacing Command TU #4-*/
   /*--------------------------------------*/
    case COATU_PACE_CMD: {
     ret_status = Do_PACE_CMD(ptx_corv);
     return (ret_status);
     break;
    }

   /*--------------------------------------*/
   /*- Subsystem Control Block Test TU #5 -*/
   /*--------------------------------------*/
    case COATU_SCB_CMD:  {
     ret_status = Do_SCB_CMD(ptx_corv);
     return (ret_status);
     break;
    }

    /*---------------------------------------*/
    /* Address Tests TU #6                   */
    /*---------------------------------------*/
    case COATU_ADDR_TEST: {
     ret_status = Do_ADDR_TEST(ptx_corv);
     return (ret_status);
     break;
    }

    /*---------------------------------------*/
    /* Microcode Download Test Unit #8       */
    /*---------------------------------------*/
    case COATU_MCODE_DWNLD: {
     ret_status = Do_MCODE_DWNLD(ptx_corv);
     return (ret_status);
     break;
    }

    /*---------------------------------------*/
    /* Unknown tu number                     */
    /*---------------------------------------*/
    default:
     return (COATU_INVALID_PARAM);

  }  /* end of switch on tu number */

  return (COATU_FAILED);

 }   /* end of exectu() --------------------------------------------------------*/

 /*****************************************/
 /*- INITIALIZE Test Units #1            -*/
 /*****************************************/
 int Do_INIT_TUS(ptx_corv)
  CORV_TUTYPE *ptx_corv;
 {
  int rc;
  static char xstr[20];

  ptx_corv->isr = 0;

  (void) db(ptx_corv,"init_env\n",0);
  rc = init_env(ptx_corv);
  if (rc == -1)
   return(COATU_FAILED);               /*- error initilizing environment -*/

  /*- unconfigure adapter/children and place adapter in diagnose state -*/
    (void) db(ptx_corv,"diagex_cfg_state  ",1);
    rc = diagex_cfg_state(ptx_corv->ldev_name);
    sprintf(xstr,"%d",rc);
    (void) db(ptx_corv,xstr,2);
    if ((rc == 1) || (rc == 2))            /*- adapter or child can not be unconfigured -*/
     return(COATU_UNABLE_UNCONFIG);
    if (rc == 3)                           /*- unable to set adapter to diagnose state -*/
     return(COATU_UNABLE_DIAGNOSE);
    if (rc != 0)                           /*- test unit failed to complete normally -*/
     return(COATU_FAILED);

  /*- open the adapter handle and initialize internal data structures -*/
  (void) db(ptx_corv,"adapter_open\n",1);
  if ((ahandle = adapter_open(ptx_corv->ldev_name)) == 0) {
   /*- if unable to open adapter in diagex mode then          -*/
   /*- reconfigure adapter/children to their original state   -*/
   (void) db(ptx_corv,"diagex_initial_state  ",1);
   rc = diagex_initial_state(ptx_corv->ldev_name);
   sprintf(xstr,"%d",rc);
   (void) db(ptx_corv,xstr,2);

   return(COATU_UNABLE_OPEN);         /*-adapter open failed -*/
  }

  /*- initialize the adapter -*/
  ptx_corv->isr = 0;                  /* isr value set to 0 to ensure that DA does not find an error -*/

  return(COATU_SUCCESS);               /*- normal completion -*/
 }

 /*****************************************/
 /*- TERMINATE Test Units #2             -*/
 /*****************************************/
 int Do_TERM_TUS(ptx_corv)
  CORV_TUTYPE *ptx_corv;
 {
  int rc;
  static char xstr[20];

  ptx_corv->isr = 0;                   /* set isr value */

  (void) db(ptx_corv,"adapter_close\n",1);
  rc = adapter_close(ahandle);
  if (rc != 0)
   return(COATU_FAILED);               /*- close adapter error -*/

  /*- reconfigure adapter/children to their original state -*/
    (void) db(ptx_corv,"diagex_initial_state  ",1);
    rc = diagex_initial_state(ptx_corv->ldev_name);
    sprintf(xstr,"%d",rc);
    (void) db(ptx_corv,xstr,2);
    if (rc == 4)                           /*- unable to place adapter in to defined state -*/
     return(COATU_UNABLE_DEFINE);
    if ((rc == 5) || (rc == 6))            /*- unable to reconfigure adapter or children to their original state -*/
     return(COATU_UNABLE_RECONFIG);
    if (rc != 0)                           /*- test unit failed to complete normally -*/
     return(COATU_FAILED);

  return(COATU_SUCCESS);               /*- normal completion -*/
 }

 /*******************************************/
 /*- Command Interface Register Test TU #3 -*/
 /*******************************************/
 int Do_REG_TEST(ptx_corv)
  CORV_TUTYPE *ptx_corv;
 {
  int loopcount;
  int rc,i;
  int bsr;
  int completion_status;
  ADAPTER_COMMAND corvette_command;
  unsigned long cir;
  char tstr[100];
  char error_bytes[8];

  loopcount = (ptx_corv->loop) + 1;
  while (loopcount--) {

   completion_status = COATU_SUCCESS;

   /*- initialize the adapter to cause an adapter reset deeren -*/
  (void) db(ptx_corv,"initialize_corvette\n",1);
   rc = initialize_corvette(ahandle);
   rc = clear_warm_start(ahandle,error_bytes); /* deeren */

   /*- initialize the adapter to cause an adapter reset -*/
  (void) db(ptx_corv,"initialize_corvette\n",1);
   rc = initialize_corvette(ahandle);
   ptx_corv->isr = rc;                  /* rc = isr value from adapter internal diagnostics -*/
   sprintf(tstr,"  isr = x%02X\n",rc);
   (void) db(ptx_corv,tstr,1);
   if (rc == ADAPTER_TIMEOUT_ERROR)
    completion_status = COATU_TIMEOUT;  /*- adapter timed out on reset -*/
   else if (( rc >> 4 ) == 3 ) {
     (void) db(ptx_corv,"run self test\n",1);
     for (i=0; i<=7; i++) error_bytes[i] = 0x00;
     rc = run_self_test(ahandle,error_bytes);
     sprintf(tstr,"  rc = x%02X\n",rc);
     (void) db(ptx_corv,tstr,1);
     sprintf(tstr,"  error bytes: %02X %02X %02X %02X\n",error_bytes[0],error_bytes[1],
       error_bytes[2],error_bytes[3]);
     (void) db(ptx_corv,tstr,1);
     ptx_corv->io_buff[0] = error_bytes[0];
     completion_status = COATU_FAILED;
   }
   else if ((rc >> 4) != 0)
    completion_status = COATU_FAILED;   /*- intialize adapter failed -*/

   if (completion_status == COATU_SUCCESS) {
    /*- chk for command interface registers reset values == 0 -*/
    (void) db(ptx_corv,"get_corvette_cir\n",1);
    cir = get_corvette_command_interface(ahandle);  /*- get current CIR 0,1,2,3 values -*/
    sprintf(tstr,"  cir = x%08X\n",cir);
    (void) db(ptx_corv,tstr,1);
    if ((((cir >> 24) & 0xff) != 0) ||             /*-  chk CIR 0 = 0          -*/
        (((cir >> 16) & 0xff) != 0) ||             /*-  chk CIR 1 = 0          -*/
        (((cir >> 8) & 0xff) != 0) ||              /*-  chk CIR 2 = 0          -*/
         ((cir & 0xff) != 0) )                     /*-  chk CIR 3 = 0          -*/
         completion_status = COATU_FAILED;         /*- set failure if not zeros-*/
    else {
     /*- chk basic status register for full=0 / empty=1 -*/
    (void) db(ptx_corv,"get_corvette_bsr\n",1);
     bsr = get_corvette_basic_status_register(ahandle); /*- get basic status reg    -*/
     sprintf(tstr,"  bsr = x%02X\n",bsr);
     (void) db(ptx_corv,tstr,1);
     if ((bsr & 0x04) != 0x04)                     /*- CIR full=0, CIR empty=1 -*/
        completion_status = COATU_FAILED;          /*- set failure if not 04   -*/
     else {
      (void) db(ptx_corv,"set_corvette_CIR0\n",1);
      bsr = set_corvette_CIR(ahandle, 0x55,0);      /*- set CIR 0 = 0x55        -*/
      (void) db(ptx_corv,"get_corvette_bsr\n",1);
      bsr = get_corvette_basic_status_register(ahandle); /*- get basic status reg    -*/
      sprintf(tstr,"  bsr = x%02X\n",bsr);
      (void) db(ptx_corv,tstr,1);
      if ((bsr & 0x0c) != 0)                       /*- CIR full=0, CIR empty=0 -*/
        completion_status = COATU_FAILED;          /*- set failure if not 00   -*/
      else {
       (void) db(ptx_corv,"set_corvette_CIR123\n",1);
       bsr = set_corvette_CIR(ahandle, 0x55,1);     /*- CIR 1     = 0x55      -*/
       bsr = set_corvette_CIR(ahandle, 0x55,2);     /*- CIR   2   = 0x55      -*/
       bsr = set_corvette_CIR(ahandle, 0x55,3);     /*- CIR     3 = 0x55      -*/
       (void) db(ptx_corv,"get_corvette_bsr\n",1);
       bsr = get_corvette_basic_status_register(ahandle); /*- get basic status reg    -*/
       sprintf(tstr,"  bsr = x%02X\n",bsr);
       (void) db(ptx_corv,tstr,1);
       if ((bsr & 0x08) != 0x08)                   /*- CIR full=1, CIR empty=0 -*/
        completion_status = COATU_FAILED;          /*- set failure if not 08   -*/
       else {
        (void) db(ptx_corv,"set_corvette_CIR0123\n",1);
        bsr = set_corvette_CIR(ahandle, 0xAA,0);    /*- CIR 0       = 0xAA    -*/
        bsr = set_corvette_CIR(ahandle, 0xAA,1);    /*- CIR   1     = 0xAA    -*/
        bsr = set_corvette_CIR(ahandle, 0xAA,2);    /*- CIR     2   = 0xAA    -*/
        bsr = set_corvette_CIR(ahandle, 0xAA,3);    /*- CIR       3 = 0xAA    -*/
        (void) db(ptx_corv,"get_corvette_cir\n",1);
        cir = get_corvette_command_interface(ahandle);  /*- get current CIR 0,1,2,3 values -*/
        sprintf(tstr,"  cir = x%08X\n",cir);
        (void) db(ptx_corv,tstr,1);
        if ((((cir >> 24) & 0xff) != 0xAA) ||      /*-  chk CIR 0 = 0xAA     -*/
            (((cir >> 16) & 0xff) != 0xAA) ||      /*-  chk CIR 1 = 0xAA     -*/
            (((cir >> 8) & 0xff) != 0xAA) ||       /*-  chk CIR 2 = 0xAA     -*/
             ((cir & 0xff) != 0xAA))               /*-  chk CIR 3 = 0xAA     -*/
         completion_status = COATU_FAILED;         /*- set failure if not AA   -*/
        else {
         (void) db(ptx_corv,"set_corvette_CIR0123\n",1);
         bsr = set_corvette_CIR(ahandle, 0xFF,0);   /*- CIR 0       = 0xFF    -*/
         bsr = set_corvette_CIR(ahandle, 0xFF,1);   /*- CIR   1     = 0xFF    -*/
         bsr = set_corvette_CIR(ahandle, 0xFF,2);   /*- CIR     2   = 0xFF    -*/
         bsr = set_corvette_CIR(ahandle, 0xFF,3);   /*- CIR       3 = 0xFF    -*/
         (void) db(ptx_corv,"get_corvette_cir\n",1);
         cir = get_corvette_command_interface(ahandle);  /*- get current CIR 0,1,2,3 values -*/
         sprintf(tstr,"  cir = x%08X\n",cir);
         (void) db(ptx_corv,tstr,1);
         if ((((cir >> 24) & 0xff) != 0xFF) ||     /*-  chk CIR 0 = 0xFF     -*/
             (((cir >> 16) & 0xff) != 0xFF) ||     /*-  chk CIR 1 = 0xFF     -*/
             (((cir >> 8) & 0xff) != 0xFF) ||      /*-  chk CIR 2 = 0xFF     -*/
              ((cir & 0xff) != 0xFF))              /*-  chk CIR 3 = 0xFF     -*/
         completion_status = COATU_FAILED;         /*- set failure if not FF   -*/
    }}}}}
   } /* endif */

  }  /*- end while (loopcount--) -*/
  return(completion_status);
 }

 /*****************************************/
 /*- Test Immediate Pacing Command TU #4 -*/
 /*****************************************/
 int Do_PACE_CMD(ptx_corv)
  CORV_TUTYPE *ptx_corv;
 {
  int loopcount;
  int response;
  int completion_status;
  ADAPTER_COMMAND corvette_command;
  char tstr[100];

  loopcount = (ptx_corv->loop) + 1;
  while (loopcount--) {

   /* build corvette immediate pace command */
   corvette_command = corvette_immediate_pacing_control(_50_PERCENT,_RESERVED);

   /* send command to adapter */
   (void) db(ptx_corv,"send_corvette_pace_command\n",1);
   response = send_corvette_immediate_command(ahandle, corvette_command, _DEVICE_ID_15);
   sprintf(tstr,"  response = x%02X\n",response);
   (void) db(ptx_corv,tstr,1);
   ptx_corv->isr = 0;

   if (response == _IMMEDIATE_SUCCESS) {
    ptx_corv->isr = response;        /* return isr value to caller */
    completion_status = COATU_SUCCESS;
   }
   else
    if (response == ADAPTER_TIMEOUT_ERROR)
         completion_status = COATU_TIMEOUT;
   else
    completion_status = COATU_FAILED;
  }
  return(completion_status);
 }

 /*****************************************/
 /*- Subsystem Control Block Test TU #5  -*/
 /*****************************************/
 int Do_SCB_CMD(ptx_corv)
  CORV_TUTYPE *ptx_corv;
 {
  int loopcount;
  int response;
  int completion_status;
  ADAPTER_COMMAND corvette_command;
  char tstr[200];

  DMA_STRUCT buffer, TSB;

  loopcount = (ptx_corv->loop) + 1;

  TSB = dma_allocate(ahandle,1);
  buffer = dma_allocate(ahandle,1);

  while (loopcount--) {
   /* build corvette command */
   corvette_command =corvette_get_POS_and_adapter_information(_AUTO_TSB,
                                     _UNSUPPRESSED_DATA,
                                     _RESERVED,
                                     buffer.dma_address,
                                     _BUFFER_BYTE_COUNT_256,
                                     TSB.dma_address,
                                     _SCB_CHAIN_ADDR_0,
                                     _RESERVED);

   /* send command to adapter */
   (void) db(ptx_corv,"send_get_POS/adapter_info_cmd\n",1);
   response = send_corvette_command(ahandle, corvette_command, _DEVICE_ID_15);
   ptx_corv->isr = 0;
   sprintf(tstr,"  isr = x%02X\n",response);
   (void) db(ptx_corv,tstr,1);

   completion_status = COATU_FAILED;  /*- prime return status -*/

   dma_flush(ahandle, TSB);
   sprintf(tstr,"TSB: %02X, %02X, %02X, %02X\n",
                 *(TSB.page_address+14),
                 *(TSB.page_address+15),
                 *(TSB.page_address+16),
                 *(TSB.page_address+17));
   (void) db(ptx_corv,tstr,1);

   if (response == _SCB_SUCCESS) {
    ptx_corv->isr = response;        /* return isr value to caller */

    (void) db(ptx_corv,"POS regs 0,1 Pace Factor\n",1);
    sprintf(tstr," %02X, %02X, %02X\n",
         (*(buffer.page_address+0)),                 /* POS reg 0 */
         (*(buffer.page_address+1)),                 /* POS reg 1 */
         (*(buffer.page_address+10)));               /* Pacing factor */
    (void) db(ptx_corv,tstr,1);

    /*- validate POS registers 0,1, Pace Factor -*/
    /*- validate POS registers 2,3,4, Interrupt level and Pace Factor -*/
    if ( /*((*(buffer.page_address+3)) == 0x01) && */        /* POS reg 2 */
         /*((*(buffer.page_address+2)) == 0xc9) && */        /* POS reg 3 */
         /*((*(buffer.page_address+5)) == 0x34) && */        /* POS reg 4 */
         /*((*(buffer.page_address+4)) == 0x0e) && */      /* Interrupt level */
         ((*(buffer.page_address+0)) == 0xfc) &&         /* POS reg 0 */
         ((*(buffer.page_address+1)) == 0x8e) &&         /* POS reg 1 */
         ((*(buffer.page_address+10)) == 0x32) )         /* Pacing factor */
      completion_status = COATU_SUCCESS;
   }
   else
    if (response == ADAPTER_TIMEOUT_ERROR)
     completion_status = COATU_TIMEOUT;

  }
  dma_free(ahandle,TSB);
  dma_free(ahandle,buffer);

  return(completion_status);
 }

 /*****************************************/
 /* Address Test TU #6                    */
 /*****************************************/
 int Do_ADDR_TEST(ptx_corv)
  CORV_TUTYPE *ptx_corv;
 {

  DMA_STRUCT buffer1, buffer2;
  DMA_STRUCT TSB;
  int offset;
  int dblocks;
  int response;
  int completion_status;
  int i;
  int lcl_ram_addr;
  char data_buf[300];
  char clr_buf[300];

  static char tstr[100];

  ADAPTER_COMMAND corvette_command;

  /*-------------------------------------------------------------------------*/
  lcl_ram_addr = 0x0100;          /*- adapter loacal ram r/w area -*/

  dblocks = 256;                  /*- test 16 address bits -*/
  completion_status = 0;

  buffer1 = dma_allocate(ahandle,dblocks);
  buffer2 = dma_allocate(ahandle,1);

  TSB = dma_allocate(ahandle,1);
  (void) db(ptx_corv,"address test, memory allocated\n",1);

  for (i=0; i<=255; i++) { data_buf[i] = i; clr_buf[i] = 0; }

  for (offset=0; offset<(dblocks * 256); offset=offset+256) {

    memcpy(buffer2.page_address, clr_buf, 256);
    dma_flush(ahandle,buffer2);

    memcpy(buffer1.page_address+offset, data_buf, 256);
    dma_flush(ahandle,buffer1);

    corvette_command = corvette_write_adapter_local_RAM(_AUTO_TSB,
                 _RETRY_ENABLE,
                 lcl_ram_addr,
                 buffer1.dma_address+offset,
                 _BUFFER_BYTE_COUNT_2560,
                 TSB.dma_address,
                 _SCB_CHAIN_ADDR_0);

/*   sprintf(tstr,"write to lcl ram from offset %02X  ",offset);
     (void) db(ptx_corv,tstr,1);  */
     response = send_corvette_command(ahandle, corvette_command, _DEVICE_ID_15);
/*   sprintf(tstr,"%d",response);
     (void) db(ptx_corv,tstr,2);  */
     if (response != _SCB_SUCCESS) {
      completion_status = COATU_FAILED;
      break;
     }

     /*-- clear read buffer --*/
     memset(buffer2.page_address, 0, 256);
     dma_flush(ahandle,buffer2);

     corvette_command = corvette_read_adapter_local_RAM(_AUTO_TSB,
                  _RETRY_ENABLE,
                 lcl_ram_addr,
                  buffer2.dma_address,
                  _BUFFER_BYTE_COUNT_2560,
                  TSB.dma_address,
                  _SCB_CHAIN_ADDR_0);

/*   (void) db(ptx_corv,"read lcl ram  ",1);  */
     response = send_corvette_command(ahandle, corvette_command, _DEVICE_ID_15);
/*   sprintf(tstr,"%d",response);
     (void) db(ptx_corv,tstr,2); */
     if (response != _SCB_SUCCESS) {
      completion_status = COATU_FAILED;
      break;
     }

    dma_flush(ahandle,buffer2);

/*  (void) db(ptx_corv,"memory compare  ",1);  */
    if (memcmp(buffer2.page_address, buffer1.page_address+offset, 256) != 0) {
     completion_status = COATU_FAILED;
     (void) db(ptx_corv,"ERROR!",2);
     break;
    }
/*  (void) db(ptx_corv,"ok!",2);  */
  }
  (void) db(ptx_corv,"done...\n",1);

  dma_free(ahandle,buffer1);
  dma_free(ahandle,buffer2);
  dma_free(ahandle,TSB);

  return completion_status;

 }  /* end Do_ADDR_TEST */

 /*****************************************/
 /* Microcode Download Function TU #8     */
 /*****************************************/
 int Do_MCODE_DWNLD(ptx_corv)
  CORV_TUTYPE *ptx_corv;
 {
  int i,rc;
  int response;
  int completion_status;

  ADAPTER_COMMAND corvette_command;
  char tstr[100];

   ADAPTER_RESPONSE corvette_response;
   SCSI_COMMAND scsi_command;

   DMA_STRUCT ucode_data;
   DMA_STRUCT buffer, TSB;
   int loop;

   FILE *mcode_file;
   char *microcode;
   int name_len;
   unsigned int microcode_length;
   int swap_array[4] = {3, 2, 1, 0};
   int index;
   uchar *vpd;
   unsigned int vpd_len;
   char ll_c[6];
   char header[] = "SCSI2_VPD1";
   char hex_zero = '\0';
   int c, even, odd, count;
   int num_zero, even_check_bit, odd_check_bit;

   char typ_char;
   int zb_ptr, rl_ptr;

/*---start of code---------------------------------------------------*/
  completion_status = COATU_SUCCESS;
  /*----------------------------------------*/
  /*- validate last character of file name -*/
  /*----------------------------------------*/
  name_len = strlen(ptx_corv->io_buff);
  if ((name_len < 12) || (name_len > 256)) {  /*- name should be at least 12 chars -*/
   completion_status = COATU_FAILED;
   sprintf(tstr,"invalid microcode name= %s \n",ptx_corv->io_buff);
   (void) db(ptx_corv,tstr,1);
  }
  else {
   sprintf(tstr,"microcode name= %s \n",ptx_corv->io_buff);
   (void) db(ptx_corv,tstr,1);
   typ_char = toupper(ptx_corv->io_buff[name_len-1]);
   switch (typ_char) {
    case 'B' : completion_status = COATU_SUCCESS;
               break;
    case 'M' : completion_status = COATU_SUCCESS;
               break;
     default : completion_status = COATU_FAILED;
   } /* endswitch */
  }
  if (completion_status == COATU_SUCCESS) {  /*- perform download using passed name */
   /*------------------------------------------------------*/
   /*- initialize adapter                                 -*/
   /*------------------------------------------------------*/
   (void) db(ptx_corv,"initialize adapter\n",1);
   rc = initialize_corvette(ahandle);
   ptx_corv->isr = rc;                  /* rc = isr value from adapter internal diagnostics -*/
   sprintf(tstr,"  rc = x%02X\n",rc);
   (void) db(ptx_corv,tstr,1);

   if (rc == ADAPTER_TIMEOUT_ERROR)
    completion_status = COATU_TIMEOUT;  /*- adapter timed out on reset -*/
   else if ((rc >> 4) != 0)
    completion_status = COATU_FAILED;   /*- intialize adapter failed -*/
   else {                               /*- adapter reset succeeded  -*/
    /*---------------------------------------*/
    /* download the micro/boot code          */
    /*---------------------------------------*/
    /*- open microcode data file -*/
    sprintf(tstr,"open_mcode_file >%s<\n",ptx_corv->io_buff);
    (void) db(ptx_corv,tstr,1);
    mcode_file = fopen(ptx_corv->io_buff,"r");
    if (mcode_file==NULL) {  /*- unable to open microcode data file -*/
      completion_status = COATU_FAILED;
      (void) db(ptx_corv," open failed!\n",1);
    }
    else {   /*- read in open microcode data file */
     microcode = (char *)malloc(300000);
     if(microcode==NULL) {  /*- unable to allocate memory for microcode -*/
      completion_status = COATU_FAILED;
      (void) db(ptx_corv," malloc failed!\n",1);
     }
     else {   /*- allocate memory for microcode -*/
      (void) db(ptx_corv,"read microcode data\n",1);
      microcode_length = 0;
      while (!feof(mcode_file)) {
        microcode[ microcode_length ] = fgetc(mcode_file);
        microcode_length++;
      }
      microcode_length--;   /*- set total # bytes to be downloaded -*/
      fclose(mcode_file);

      ucode_data = dma_allocate(ahandle, 33);
      memcpy((ucode_data.page_address+0x100), microcode, microcode_length);
      dma_flush(ahandle, ucode_data);

      corvette_command = corvette_download_prepare();
      send_corvette_immediate_command(ahandle, corvette_command, 0xf);

      (void) db(ptx_corv,"download microcode\n",1);
      corvette_command = corvette_download_microcode(_AUTO_TSB,
                                                _RESERVED,
                                                ucode_data.dma_address+0x100,
                                                microcode_length,
                                                ucode_data.dma_address,
                                                _RESERVED,
                                                _RESERVED);

      response = send_corvette_command(ahandle, corvette_command, 0xf);
      sprintf(tstr,"  response = x%02X\n",response);
      (void) db(ptx_corv,tstr,1);

      dma_flush(ahandle, ucode_data);
      if (response == _SCB_SUCCESS)
       completion_status = COATU_SUCCESS;
      else
       completion_status = COATU_FAILED;
      dma_free(ahandle, ucode_data);
     }   /*- allocate memory for microcode -*/
     free(microcode);
    }   /*- read in open microcode data file */
    if (completion_status == COATU_SUCCESS) {  /*- update vpd data */
     /*-allocate buffer space for vpd data -*/
     vpd = (uchar *)calloc(9000,1);
     if(vpd==NULL) {  /*- unable to allocate memory for vpd -*/
      completion_status = COATU_FAILED;
     }
     else {   /*- continue vpd update -*/
      /*-place SCSI2_VPD1 into vpd header area -*/
      for (i=0; i<10; i++) vpd[i] = header[i];
      for (i=10; i<9000; i++) vpd[i] = 0;

      /*----------------------------------------------------*/
      /*- get current vpd data from adapter                -*/
      /*----------------------------------------------------*/
      TSB = dma_allocate(ahandle,1);
      buffer = dma_allocate(ahandle,1);

      (void) db(ptx_corv,"get vpd info\n",1);
      corvette_command =corvette_get_POS_and_adapter_information(_AUTO_TSB,
                                         _UNSUPPRESSED_DATA,
                                         _RESERVED,
                                         buffer.dma_address,
                                         _BUFFER_BYTE_COUNT_256,
                                         TSB.dma_address,
                                         _SCB_CHAIN_ADDR_0,
                                         _RESERVED);

      response = send_corvette_command(ahandle, corvette_command, _DEVICE_ID_15);
      ptx_corv->isr = 0;
      sprintf(tstr,"  response = x%02X\n",response);
      (void) db(ptx_corv,tstr,1);

      completion_status = COATU_FAILED;  /*- prime return status -*/

      if (response == _SCB_SUCCESS) {   /* good get of vpd data */
       ptx_corv->isr = response;        /* return isr value to caller */

       /*-set vpd_len to vpd length -*/
       vpd_len = ((*(buffer.page_address+32+3)) & 0xff);
       vpd_len = (vpd_len << 8);
       vpd_len = ((vpd_len + ((*(buffer.page_address+32+4)) & 0xff)) * 2);
       sprintf(tstr,"vpd_len = %d\n",vpd_len);
       (void) db(ptx_corv,tstr,1);

       /*-move vpd data to vpd buffer -*/
       for (i=0; i<(vpd_len+7); i++) vpd[i+10] = (*(buffer.page_address+32+i));
       completion_status = COATU_SUCCESS;
      }
      else
       if (response == ADAPTER_TIMEOUT_ERROR)
        completion_status = COATU_TIMEOUT;

      dma_free(ahandle,TSB);
      dma_free(ahandle,buffer);

      if (completion_status == COATU_SUCCESS) {  /*- update rl/zb fields */
       /*- locate RL starting position -*/
       rl_ptr = 0;
       for (i=0; i<(vpd_len+7); i++) {
        if ((vpd[i+9] == '*') &&
            (vpd[i+10] == 'R') && (vpd[i+11] == 'L')) {
         rl_ptr = i+13;
        }
       }

       /*- locate ZB starting position -*/
       zb_ptr = 0;
       for (i=0; i<(vpd_len+7); i++) {
        if ((vpd[i+9] == '*') &&
            (vpd[i+10] == 'Z') && (vpd[i+11] == 'B')) {
         zb_ptr = i+13;
        }
       }
       sprintf(tstr,"rl_ptr[%d]  zb_ptr[%d]\n",rl_ptr,zb_ptr);
       (void) db(ptx_corv,tstr,1);

       /*---------------------------------------*/
       /* check passed filename ending char     */
       /* for type of download data.            */
       /* set ZB field if Boot code else        */
       /* set RL field for Microcode            */
       /*---------------------------------------*/
       if (typ_char == 'B') {
        (void) db(ptx_corv,"typ_char = B\n",1);
        if (zb_ptr > 0) {
         vpd[zb_ptr] = ptx_corv->io_buff[name_len-3];
         vpd[zb_ptr+1] = ptx_corv->io_buff[name_len-2];
        }
       }
       else {
        (void) db(ptx_corv,"typ_char = M\n",1);
        if (rl_ptr > 0) {
         vpd[rl_ptr] = ptx_corv->io_buff[name_len-3];
         vpd[rl_ptr+1] = ptx_corv->io_buff[name_len-2];
        }
       }

       /*----------------------------------*/
       /* calculate crc and chksum bytes   */
       /*----------------------------------*/
       /*-calculate crc byte and update buffer byte -*/
       c = crc_gen(&vpd[17], vpd_len);
       vpd[15] = ((c >> 8) & 0xff);
       vpd[16] = (c & 0xff);

       sprintf(tstr," crc: %02X  %02X\n",vpd[15],vpd[16]);
       (void) db(ptx_corv,tstr,1);

       even = 0; odd = 0; count = 0;
       /*- calculate chksum byte from buffer data -*/
       /*- after header through end of buffer     -*/
       for (i=0; i<8192; i++) {
          c = vpd[i+10];
          if (count%2)
             odd += c;
          else
             even += c;
          count++;
       }
       even_check_bit = 0x100 - (even & 0xff);
       odd_check_bit = 0x100 - (odd & 0xff);

       vpd[8200] = even_check_bit;
       vpd[8201] = odd_check_bit;
       sprintf(tstr," new vpd chksum: %02X  %02X\n",vpd[8200],vpd[8201]);
       (void) db(ptx_corv,tstr,1);

       /*---------------------------------------*/
       /* download new vpd info                 */
       /*---------------------------------------*/
       microcode_length = 8202;
       /*- # of bytes to be downloaded = microcode_length  -*/

       ucode_data = dma_allocate(ahandle, 3);
       memcpy((ucode_data.page_address+0x100), vpd, microcode_length);
       dma_flush(ahandle, ucode_data);

       corvette_command = corvette_download_prepare();
       send_corvette_immediate_command(ahandle, corvette_command, 0xf);

       (void) db(ptx_corv,"download new vpd\n",1);
       corvette_command = corvette_download_microcode(_AUTO_TSB,
                                                 _RESERVED,
                                                 ucode_data.dma_address+0x100,
                                                 microcode_length,
                                                 ucode_data.dma_address,
                                                 _RESERVED,
                                                 _RESERVED);

       response = send_corvette_command(ahandle, corvette_command, 0xf);
       sprintf(tstr,"  response = x%02X\n",response);
       (void) db(ptx_corv,tstr,1);

       dma_flush(ahandle, ucode_data);
       if (response == _SCB_SUCCESS)
        completion_status = COATU_SUCCESS;
       else {
        completion_status = COATU_FAILED;
       }
       dma_free(ahandle, ucode_data);
      }   /*- end of update rl/zb fields */
      free(vpd);
     }   /*- end of continue vpd update -*/
    }    /*- end of update vpd data */
   }   /*- adapter reset succeeded  -*/
  }   /*- perform download using passed name */

  return(completion_status);

 }  /* end Do_MCODE_DWNLD */

