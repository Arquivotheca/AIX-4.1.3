static char sccsid[] = "@(#)29  1.7  src/bos/diag/tu/sun/hw_manage.c, tu_sunrise, bos411, 9437A411a 7/27/94 17:59:55";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: hw_clean
 *              hw_init
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
Module Name :  hw_manage.c
*****************************************************************************/

#include <odmi.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include "sundiag.h"
#include "suntu.h"
#include "sun_slih.h"
#include "error.h"
#include "sun_tu_type.h"

sundiag_dds
    sun_fld,                    /* dds for Sunrise          */
    * sun_dds = &sun_fld;       /* pointer to Sunrise dds   */

diag_struc_t
    *handle;                    /* handle for Sunrise       */

struct cfg_load
    cfg_ld_sun;                 /* for loading Sunrise interrupt routine  */

int getsun(char *log_name);     /* reading Sunrise parameters from ODM */

struct sun_intr_data
    sun_data;                   /* data for SUN interrupt routine       */

struct resource
    sun_resource;               /* resources, occupied by program       */

/*****************************************************************************
 * NAME: hw_init
 *
 * FUNCTION: Hardware initialization for Sunrise testing ...
 *
 * INPUT:
 *
 * OUTPUT:   return=0 if OK
 *           return=0x99  if CODEC daughter card not present  (NOT an error)
 *           return!=0 if ERROR
 *****************************************************************************/

int hw_init(char *devid)
{
   int RetCode = 0;
   int i, intr_bits;
   unsigned int StatusVal;

                      /* clean up structure with program resources */
   strcpy(sun_resource.mode, " ");
   sun_resource.odm = '0';
   sun_resource.diag = '0';
   sun_resource.intr = '0';
   sun_resource.handle = '0';

   RetCode = (int)odm_initialize();
   if (RetCode) {
       DEBUG_1("   ERROR> ODM initialization failed, RC=%d\n", RetCode);
       LOG_HWINIT_ERROR(ERRORHW_INIT, RetCode, 0);
   }
   sun_resource.odm = '1';

  /* put device to be tested in Diagnose state */
   RetCode = diagex_cfg_state(devid);  /* from libdiag.a */
   if ( RetCode != 0 ) {
       DEBUG_1("   ERROR> diagex_cfg_state was not succesful, RC=%d\n", RetCode);
       LOG_HWINIT_ERROR(ERRORHW_INIT, RetCode, 0);
   }
   sun_resource.diag = '1';

   RetCode = getsun(devid);
   if ( RetCode != 0 ) {
       DEBUG_1("   ERROR> sun_dds building was not succesful, RC=%d\n", RetCode);
       LOG_HWINIT_ERROR(ERRORGETSUN, RetCode, 0);
   }

                /* now open a handle for diagex operations on sunrise */
                /* implement interrupt for Sunrise later - dv */
     sun_dds->d_dds.data_ptr = &sun_data.filler;
     sun_dds->d_dds.d_count = sizeof(sun_data);

                /* Loading interrupt handlers for Sunrise                  */

   cfg_ld_sun.path = "/usr/lpp/diagnostics/slih/sun_slih";
   if (sysconfig(SYS_KLOAD, (void *)&cfg_ld_sun, (int)sizeof(cfg_ld_sun))) {
       DEBUG_0("   ERROR> Loading Sunrise interrupt routine was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, RetCode, 0);
   }
   sun_resource.intr = '1';
   sun_dds->d_dds.kmid = cfg_ld_sun.kmid;

   sun_dds->d_dds.intr_flags = 0;
   sun_dds->d_dds.maxmaster = 1;   /* One Concarent DMA */

#ifdef DEBUG_SUN
                 /* print device attributes */
   DEBUG_8 ("   hw_init:  bus_id=0x%x\n", sun_dds->d_dds.bus_id);
   DEBUG_8 ("   hw_init:  bus_type=%d\n", sun_dds->d_dds.bus_type);
   DEBUG_8 ("   hw_init:  bus_io_addr=0x%x\n", sun_dds->d_dds.bus_io_addr);
   DEBUG_8 ("   hw_init:  bus_io_length=%d\n", sun_dds->d_dds.bus_io_length);
   DEBUG_8 ("   hw_init:  bus_intr_level=%d\n", sun_dds->d_dds.bus_intr_lvl);
   DEBUG_8 ("   hw_init:  intr_priority=%d\n", sun_dds->d_dds.intr_priority);
   DEBUG_8 ("   hw_init:  dma_level=%d\n", sun_dds->d_dds.dma_lvl);
#endif

                /* Open diagex handle for SUNRISE  */
   RetCode = diag_open( &sun_dds->d_dds, &handle);
   if (RetCode != 0)  {
       DEBUG_1("   ERROR> Diag_open SUNRISE not succesfull, rc=0x%x\n", RetCode);
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }
   sun_resource.handle = '1';

                /* SUNRISE card initialization  */
   /* Initialize POS register #5 = Binary (0 0 I15 I14 I13 x x x) */
   i = (sun_dds->d_dds.bus_io_addr & 0xe000) >> 10;
   RetCode = diag_pos_write(handle, 5, i, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x05 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }

   /* Initialize POS register #4 = Binary (0 0 0 0 0 0 0 1) */
   RetCode = diag_pos_write(handle, 4, 0x1, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x04 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }

   /* Initialize POS register #3 = Binary (1 0 1 0 B3 B2 B1 B0) */
   /* Enable Streaming Data & Micro-channel Parity, B3-0=Arbitration level */
   /* To access POS #3, write 0x0 to POS #6, 0x0 to POS #7, and value to POS #3 */
   RetCode = diag_pos_write(handle, 6, 0x0, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x06 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }
   RetCode = diag_pos_write(handle, 7, 0x0, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x07 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }
   RetCode = diag_pos_write(handle, 3, (sun_dds->d_dds.dma_lvl | 0xA0), NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x03 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }

   /* Initialize POS register #3A = Binary (I12 I11 I10 0 0 0 0 0) */
   /* To access POS #3A, write 0x0 to POS #6, 0x1 to POS #7, and value to POS #3 */
   RetCode = diag_pos_write(handle, 6, 0x0, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x06 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }
   RetCode = diag_pos_write(handle, 7, 0x1, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x07 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }
   i = (sun_dds->d_dds.bus_io_addr & 0x1C00) >> 5;
   RetCode = diag_pos_write(handle, 3, i, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x03 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }

   /* Initialize POS register #3B = Binary (0 0 0 0 0 0 0 0) */
   /* To access POS #3B, write 0x1 to POS #6, 0x1 to POS #7, and value to POS #3 */
   RetCode = diag_pos_write(handle, 6, 0x1, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x06 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }
   RetCode = diag_pos_write(handle, 7, 0x1, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x07 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }
   RetCode = diag_pos_write(handle, 3, 0x0, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR> Diag_write 0x03 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }

   /* Initialize POS register #2 = Binary (L1 L0 0 1 0 0 1 1) */
   if (sun_dds->d_dds.bus_intr_lvl == 6)
     intr_bits = 0x0;
   else
     if (sun_dds->d_dds.bus_intr_lvl == 7)
       intr_bits = 0x1;
     else
       if (sun_dds->d_dds.bus_intr_lvl == 11)
         intr_bits = 0x2;
       else     /* set default as intr lvl 15 */
         intr_bits = 0x3;
   i = (intr_bits << 6) | 0x13;
   RetCode = diag_pos_write(handle, 2, i, NULL, PROCLEV);
   if (RetCode < 0) {
       DEBUG_0("   ERROR>Diag_write 0x02 for SUNRISE was not succesfull\n");
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }

   /* Reset the Sunrise card */
   if (RetCode=pio_mcwrite (0x5, 0x80, 1)) {
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }
   usleep(100);  /* required at least 50us delay */
   if (RetCode=pio_mcwrite (0x5, 0x0, 1)) {
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }


   /* Check to see if CODEC daughter card is attached */
   if(RetCode=pio_read(LBStatus, &StatusVal, 1)) {
       LOG_HWINIT_ERROR(ERRORHW_INIT, 0, RetCode);
   }
   if (StatusVal & CODEC_NOTPRESENT)
      return (NO_CODEC);  /* If CODEC is NOT present, pass this error code */
                          /* to the main apps */

                /* Initialization completed OK, returns 0       */
   return (RetCode);
}

/*****************************************************************************
 * NAME: hw_clean
 *
 * FUNCTION: Hardware cleanup for Sunrise before exit.
 *
 * INPUT:
 *
 * OUTPUT: Error Code
 *
 *****************************************************************************/

int hw_clean(char *devid)
{
   int RetCode = 0;

                        /* Reset SUNRISE card                  */
                        /* need to modify this later - dv */

                        /* Close diagex handles           */
   if (sun_resource.handle == '1') {
       RetCode = diag_close(handle);
       if (RetCode != 0) {
           DEBUG_1("   ERROR> Diag_close SUNRISE not succesfull, rc=0x%x\n",
                   RetCode);
           LOG_HWINIT_ERROR(ERRORHW_CLEAN, 0, RetCode);
       }
       sun_resource.handle = '0';
   }

                        /* Unload interrupt routines            */
   if (sun_resource.intr == '1') {
       if (RetCode=sysconfig(SYS_KULOAD, (void *)&cfg_ld_sun, (int)sizeof(cfg_ld_sun))) {
           DEBUG_0("   ERROR> Unloading Sunrise interrupt routine was not succesfull\n");
           LOG_HWINIT_ERROR(ERRORHW_CLEAN, RetCode, 0);
       }
       sun_resource.intr = '0';
   }

  /* put device to be tested back to its original state */
   if (sun_resource.diag == '1') {
     RetCode = diagex_initial_state(devid);  /* from libdiag.a */
      if ( RetCode != 0 ) {
          DEBUG_1("   ERROR> diagex_initial_state was not succesful, RC=%d\n",
               RetCode);
          LOG_HWINIT_ERROR(ERRORHW_CLEAN, RetCode, 0);
      }
      sun_resource.diag = '0';
   }

   if (sun_resource.odm == '1') {
     RetCode = (int)odm_terminate();
     if (RetCode) {
         DEBUG_1("   ERROR> ODM termination failed, RC=%d\n", RetCode);
         LOG_HWINIT_ERROR(ERRORHW_CLEAN, RetCode, 0);
     }
     sun_resource.odm = '0';
   }
                        /* Cleaning completed                   */
   return (OK);
}

