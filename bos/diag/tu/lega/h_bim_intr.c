static char sccsid[] = "@(#)13  1.6.1.3  src/bos/diag/tu/lega/h_bim_intr.c, tu_lega, bos411, 9428A410j 12/10/93 15:34:05";
/*
 * COMPONENT_NAME: (tu_lega) Low End Graphics Adapter Test Units
 *
 * FUNCTIONS: host_interrupt_test(), event_handler()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * MODULE NAME: h_bim_intr.c
 *
 * STATUS: Release 1, EC 00, EVT Version 1
 *
 * DEPENDENCIES:  None.
 *
 * RESTRICTIONS:  None.
 *
 * EXTERNAL REFERENCES
 *
 *     OTHER ROUTINES:
 *
 *     DATA AREAS:  bim_base_addr from dd_interface.c
 *                  gsc_handle from dd_interface.c
 *
 *     TABLES:  None.
 *
 *     MACROS:  None.
 *
 * COMPILER / ASSEMBLER
 *
 *     TYPE, VERSION: AIX C Compiler, version 3
 *
 *     OPTIONS:
 *
 * NOTES:  None.
 *
 */

#include   <stdio.h>
#include   <signal.h>
#include   <fcntl.h>
#include   <sys/rcm_win.h>
#include   <sys/rcmioctl.h>
#include   <sys/aixgsc.h>
#include   <mid/mid.h>
#include  "bim_defs.h"
#include  "host_dsp.h"
#include  "host_debug.h"

/* error codes returned by this module */
#define EVENT_SYSTEM_CALL_ERROR 0x800
#define TIME_OUT_HOST_INT       0x801
#define HOST_INTERRUPT_ERROR    0x802

#define EVENT_MASK_ERROR            0xA0A
#define DYN_EVENT_SYSTEM_CALL_ERROR 0xA0B
#define DYN_TIME_OUT_HOST_INT       0xA0C
#define DYN_HOST_INTERRUPT_ERROR    0xA0D
#define DYNAMIC_TEST_RETURN_TIMEOUT 0xA0E

/* For (DYN_)EVENT_SYSTEM_CALL_ERROR in secondary[0] : */
#define ASYNC_EVENTS_ERROR 0x8001
#define GET_EVENTS_ERROR   0x8002


/* other constants used in this file */
#define WAIT_ON_INTERRUPT_TIME  10000
#define SLEEP_MICRO_SECONDS 10
#define TEST_RETURN_TIMEOUT           0x30000
#define U_SECS_BETWEEN_REG_ACCESSES   1000

   extern gsc_handle our_gsc_handle;
   extern volatile unsigned long *bim_base_addr;
   extern int dsp_secondary_ptr;   /* address of the sec. buffer on adapter */
   extern set_mc_speed();
   extern ready_protocol();
   extern send_tu_number();

   struct get_events our_get_events;
   struct async_event our_async_event;
   eventArray event_array;


   int event_received = 0;
   void event_handler();
   static int ret_code;

   int dynamic_patterns[32] =
                 {  PAT5,  PAT6,  PAT7,  PAT8,  PAT9,  PAT10, PAT11, PAT12,
                    PAT13, PAT14, PAT15, PAT16, PAT17, PAT18, PAT19, PAT20,
                    PAT21, PAT22, PAT23, PAT24, PAT25, PAT26, PAT27, PAT28,
                    PAT29, PAT30, PAT31, PAT32, PAT33, PAT34, PAT35, PAT36  };

int get_dynamic_return_info(long *secondary_ptr);

/*
 * NAME: host_interrupt_test()
 *
 * FUNCTION: call the device driver to get events corresponding to hardware
 *           interrupts.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: Signal handler is set up as part of enter_mom() in dd_interface.c
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: dd, bim_base_addr, makemap, our_gsc_handle, middat
 *                  (declared in dd_interface.c)
 *
 * RETURNS : see below
 *
 * INPUT:
 *       tu_number - the TU number for this test passed down to C30M
 *       secondary_ptr - pointer to secondary information array
 * OUTPUT:
 *      returns zero if load was successful, otherwise will return
 *      non-zero.
 */

int host_interrupt_test(int tu_number, int *secondary_ptr)
{
   int time_out;
   int expected, actual;

   ret_code = ready_protocol(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* clear the return code from the event handler */
   ret_code = 0;
   event_received = 0;

   *(bim_base_addr + HOST_INTR_MASK) = 0;  /* disable all interrupts */
   *(bim_base_addr + HOST_STATUS) = 0x3FF; /* clear all pending interrupts */

   /* Clear the event_array */
   our_async_event.mask = DD_DIAGNOSTICS_COMPLETE;
   our_async_event.num_events = 0;
   our_async_event.error = 0;

   our_get_events.array = &event_array;

   if (aixgsc(our_gsc_handle.handle, ASYNC_EVENT , &our_async_event)){
      PRINT(("\nhost_bim_intr : ASYNC_EVENTS ERROR\n"));
      *secondary_ptr++ = ASYNC_EVENTS_ERROR;
      return(EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   ret_code = set_mc_speed(300);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* clear the return code from the event handler */
   ret_code = 0;
   event_received = 0;

   /* Set up for WRITE TO DSP_COMMO interrupt */

   *(bim_base_addr + HOST_INTR_MASK) = 0;  /* disable all interrupts */
   *(bim_base_addr + HOST_STATUS) = 0x3FF; /* clear all pending interrupts */

   our_async_event.mask = DD_DIAGNOSTICS_COMPLETE | DD_WRITE_TO_DSP_COMMO;
   our_async_event.num_events = 1;
   our_async_event.error = 0;

   if (aixgsc(our_gsc_handle.handle, ASYNC_EVENT , &our_async_event)){
      PRINT(("\nhost_bim_intr : ASYNC_EVENTS ERROR\n"));
      *secondary_ptr = ASYNC_EVENTS_ERROR;
      return(EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   ret_code = set_mc_speed(300);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   ret_code = send_tu_number(tu_number);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   time_out = WAIT_ON_INTERRUPT_TIME;
   while (!event_received && time_out) {    /* wait for interrupt */
      time_out--;
      usleep(SLEEP_MICRO_SECONDS);
   } /* endwhile */

   if (!time_out) {
      return(TIME_OUT_HOST_INT);
   } /* endif */

   /* check the return from the event_handler */
   if (ret_code) {
      *secondary_ptr = ret_code;
      return(EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   expected = DSP_COMMO_INT_VALUE;
   actual = event_array.event[0].data[0];
   if (expected != actual) {
      *secondary_ptr++ = expected;
      *secondary_ptr++ = actual;
      return(HOST_INTERRUPT_ERROR);
   } /* endif */

   *(bim_base_addr + HOST_INTR_MASK) = 0;  /* disable all interrupts */

   return(0);   /* if we got here then all is well */

}  /* end host_interrupt_test() */

/*
 * NAME: bim_dynamic_test
 *
 * FUNCTION: handshakes with sister module on the adapter
 *
 *
 * EXECUTION ENVIRONMENT:  AIX  during LEGA diagnostics
 *
 * NOTES: Signal handler is set up as part of enter_mom() in dd_interface.c
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES:
 *
 * RETURNS : see below
 *
 * INPUT:
 *       tu_number - the TU number for this test passed down to C30M
 *       secondary_ptr - pointer to secondary information array
 * OUTPUT:
 *      returns zero if load was successful, otherwise will return
 *      non-zero.
 */

bim_dynamic_test(int tu_number, long *secondary_ptr)
{
   int i, k, time_out;
   unsigned actual, expected;

   volatile unsigned long *host_intr_mask = bim_base_addr + HOST_INTR_MASK;
   volatile unsigned long *host_status    = bim_base_addr + HOST_STATUS;
   volatile unsigned long *host_commo     = bim_base_addr + HOST_COMMO;
   volatile unsigned long *dsp_commo      = bim_base_addr + DSP_COMMO;
   volatile unsigned long *pcb            = bim_base_addr + PRIORITY_IN;
   volatile unsigned long *pio_free       = bim_base_addr + PIO_FREE;
   volatile unsigned long *pio            = bim_base_addr + PIO_DATA_IN;
   volatile unsigned long *ind_control    = bim_base_addr + IND_CONTROL;
   volatile unsigned long *dsp_control    = bim_base_addr + DSP_CONTROL;
   volatile unsigned long *ind_address    = bim_base_addr + IND_ADDRESS;
   volatile unsigned long *ind_data       = bim_base_addr + IND_DATA;
   volatile unsigned long *io_intr_mask   = bim_base_addr + IO_INTR_MASK;
   volatile unsigned long *pcb_status     = bim_base_addr + PRIORITY_STATUS;

   /* clear the return code from the event handler */
   ret_code = 0;
   event_received = 0;

   *host_intr_mask = 0;  /* disable all interrupts */
   *host_status = 0x3FF; /* clear all pending interrupts */

   /* Clear the event_array */
   our_async_event.mask = DD_DIAGNOSTICS_COMPLETE;
   our_async_event.num_events = 0;
   our_async_event.error = 0;

   our_get_events.array = &event_array;

   if (aixgsc(our_gsc_handle.handle, ASYNC_EVENT , &our_async_event)){
      PRINT(("\nhost_bim_intr : ASYNC_EVENTS ERROR\n"));
      *secondary_ptr++ = ASYNC_EVENTS_ERROR;
      *secondary_ptr++ = DD_DIAGNOSTICS_COMPLETE;
      *secondary_ptr++ = 0xC001;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   ret_code = set_mc_speed(300);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   ret_code = ready_protocol(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* clear the return code from the event handler */
   ret_code = 0;
   event_received = 0;

   *host_intr_mask = 0;  /* disable all interrupts */
   *host_status = 0x3FF; /* clear all pending interrupts */

   /* Set up for WRITE TO DSP_COMMO interrupt                  (TEST 1)  */
   our_async_event.mask = DD_DIAGNOSTICS_COMPLETE | DD_WRITE_TO_DSP_COMMO;
   our_async_event.num_events = 1;
   our_async_event.error = 0;

   if (aixgsc(our_gsc_handle.handle, ASYNC_EVENT , &our_async_event)){
      PRINT(("\nhost_bim_intr : ASYNC_EVENTS ERROR\n"));
      *secondary_ptr++ = ASYNC_EVENTS_ERROR;
      *secondary_ptr++ = DD_WRITE_TO_DSP_COMMO;
      *secondary_ptr++ = 0xC001;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   ret_code = set_mc_speed(300);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   ret_code = send_tu_number(tu_number);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   time_out = WAIT_ON_INTERRUPT_TIME;
   while (!event_received && time_out) {    /* wait for interrupt */
      time_out--;
      usleep(SLEEP_MICRO_SECONDS);
   } /* endwhile */
                                                 /* SYNC POINT (1) */
   if (!time_out) {
      *secondary_ptr++ = 0xC001;
      return(DYN_TIME_OUT_HOST_INT);
   } /* endif */

   /* check the return from the event_handler */
   if (ret_code) {
      *secondary_ptr++ = ret_code;
      *secondary_ptr++ = DD_WRITE_TO_DSP_COMMO;
      *secondary_ptr++ = 0xC001;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   expected = SEND_DATA;
   actual = event_array.event[0].data[0];
   if (expected != actual) {
      *secondary_ptr++ = expected;
      *secondary_ptr++ = actual;
      *secondary_ptr++ = 0xC001;
      return(DYN_HOST_INTERRUPT_ERROR);
   } /* endif */

   /* check return code from adapter */          /* SYNC POINT (2) */
   ret_code = get_dynamic_return_info(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   *pcb = PAT5;  /* this will cause interrupt on the adapter   (TEST 2)  */

   /* check return code from adapter */          /* SYNC POINT (3) */
   ret_code = get_dynamic_return_info(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* clear the return code from the event handler */
   ret_code = 0;
   event_received = 0;

   *host_intr_mask = 0;  /* disable all interrupts */
   *host_status = 0x3FF; /* clear all pending interrupts */

   /* Set up for DSP_READ_HOST_COMMO interrupt                 (TEST 3)  */
   our_async_event.mask = DD_DIAGNOSTICS_COMPLETE | DD_DSP_HAS_READ_HCR;
   our_async_event.num_events = 1;
   our_async_event.error = 0;

   if (aixgsc(our_gsc_handle.handle, ASYNC_EVENT , &our_async_event)){
      PRINT(("\nhost_bim_intr : ASYNC_EVENTS ERROR\n"));
      *secondary_ptr++ = ASYNC_EVENTS_ERROR;
      *secondary_ptr++ = DD_DSP_HAS_READ_HCR;
      *secondary_ptr++ = 0xC002;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   ret_code = set_mc_speed(300);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* setup PCB for PCB status check on DSP side               (TEST 4)  */
   *pcb = PAT6;
   *pcb = PAT7;
   *pcb = PAT8;
   *pcb = PAT9;
   *pcb = PAT10;
   *pcb = PAT11;

   *host_commo = CONTINUE;  /* cause HOST_COMMO_WRITE intr to DSP */

   time_out = WAIT_ON_INTERRUPT_TIME;
   while (!event_received && time_out) {    /* wait for interrupt */
      time_out--;
      usleep(SLEEP_MICRO_SECONDS);
   } /* endwhile */

   if (!time_out) {
      *secondary_ptr++ = DD_DSP_HAS_READ_HCR;
      *secondary_ptr++ = 0xC002;
      return(DYN_TIME_OUT_HOST_INT);
   } /* endif */

   /* check the return from the event_handler */
   if (ret_code) {
      *secondary_ptr++ = ret_code;
      *secondary_ptr++ = DD_DSP_HAS_READ_HCR;
      *secondary_ptr++ = 0xC002;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   /* check return code from adapter */          /* SYNC POINT (4) */
   ret_code = get_dynamic_return_info(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* clear the return code from the event handler */
   ret_code = 0;
   event_received = 0;

   *host_intr_mask = 0;  /* disable all interrupts */
   *host_status = 0x3FF; /* clear all pending interrupts */

   /* Set up for PCB_FIFO_FULL interrupt                       (TEST 5)  */
   our_async_event.mask = DD_DIAGNOSTICS_COMPLETE | DD_PRIORITY_FIFO_FULL;
   our_async_event.num_events = 1;
   our_async_event.error = 0;

   if (aixgsc(our_gsc_handle.handle, ASYNC_EVENT , &our_async_event)){
      PRINT(("\nhost_bim_intr : ASYNC_EVENTS ERROR\n"));
      *secondary_ptr++ = ASYNC_EVENTS_ERROR;
      *secondary_ptr++ = DD_PRIORITY_FIFO_FULL;
      *secondary_ptr++ = 0xC003;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   ret_code = set_mc_speed(300);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   *pcb = PAT12;

   time_out = WAIT_ON_INTERRUPT_TIME;
   while (!event_received && time_out) {    /* wait for interrupt */
      time_out--;
      usleep(SLEEP_MICRO_SECONDS);
   } /* endwhile */

   if (!time_out) {
      *secondary_ptr++ = DD_PRIORITY_FIFO_FULL;
      *secondary_ptr++ = 0xC003;
      return(DYN_TIME_OUT_HOST_INT);
   } /* endif */

   /* check the return from the event_handler */
   if (ret_code) {
      *secondary_ptr++ = ret_code;
      *secondary_ptr++ = DD_PRIORITY_FIFO_FULL;
      *secondary_ptr++ = 0xC003;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   /* check return code from adapter */          /* SYNC POINT (5) */
   ret_code = get_dynamic_return_info(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* clear the return code from the event handler */
   ret_code = 0;
   event_received = 0;

   *host_intr_mask = 0;  /* disable all interrupts */
   *host_status = 0x3FF; /* clear all pending interrupts */

   /* Set up for PCB_FIFO_EMPTY interrupt                      (TEST 6)  */

   our_async_event.mask = DD_DIAGNOSTICS_COMPLETE | DD_PRIORITY_FIFO_EMPTY;
   our_async_event.num_events = 1;
   our_async_event.error = 0;

   if (aixgsc(our_gsc_handle.handle, ASYNC_EVENT , &our_async_event)){
      PRINT(("\nhost_bim_intr : ASYNC_EVENTS ERROR\n"));
      *secondary_ptr++ = ASYNC_EVENTS_ERROR;
      *secondary_ptr++ = DD_PRIORITY_FIFO_EMPTY;
      *secondary_ptr++ = 0xC004;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   ret_code = set_mc_speed(300);
   if (ret_code) {
      return(ret_code);
   } /* endif */
   *host_commo = CONTINUE;                       /* SYNC POINT (6) */

   time_out = WAIT_ON_INTERRUPT_TIME;
   while (!event_received && time_out) {    /* wait for interrupt */
      time_out--;
      usleep(SLEEP_MICRO_SECONDS);
   } /* endwhile */

   if (!time_out) {
      *secondary_ptr++ = DD_PRIORITY_FIFO_EMPTY;
      *secondary_ptr++ = 0xC004;
      return(DYN_TIME_OUT_HOST_INT);
   } /* endif */

   /* check the return from the event_handler */
   if (ret_code) {
      *secondary_ptr++ = ret_code;
      *secondary_ptr++ = DD_PRIORITY_FIFO_EMPTY;
      *secondary_ptr++ = 0xC004;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   /* check return code from adapter */          /* SYNC POINT (7) */
   ret_code = get_dynamic_return_info(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* START testing PIO */
   k = 0;

   /* write 4 words to PIO causing DSP interrupt               (TEST 7)  */
   for (i = 0 ; i < 4 ; i++) {
      *pio = dynamic_patterns[k++];
   } /* endfor */

   /* check return code from adapter */          /* SYNC POINT (8) */
   ret_code = get_dynamic_return_info(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */


   /* clear the return code from the event handler */
   ret_code = 0;
   event_received = 0;

   *host_intr_mask = 0;  /* disable all interrupts */
   *host_status = 0x3FF; /* clear all pending interrupts */

   /* Set up for HIGH WATER REACHED interrupt                  (TEST 8)  */

   our_async_event.mask = DD_DIAGNOSTICS_COMPLETE | DD_PIO_HIGH_WATER;
   our_async_event.num_events = 1;
   our_async_event.error = 0;

   if (aixgsc(our_gsc_handle.handle, ASYNC_EVENT , &our_async_event)){
      PRINT(("\nhost_bim_intr : ASYNC_EVENTS ERROR\n"));
      *secondary_ptr++ = ASYNC_EVENTS_ERROR;
      *secondary_ptr++ = DD_WRITE_TO_DSP_COMMO;
      *secondary_ptr++ = 0xC005;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   ret_code = set_mc_speed(300);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* write 2 words to PIO before sleeping for interrupt */
   for (i = 0 ; i < 2 ; i++) {
      *pio = dynamic_patterns[k++];
   } /* endfor */

   time_out = WAIT_ON_INTERRUPT_TIME;
   while (!event_received && time_out) {    /* wait for interrupt */
      time_out--;
      usleep(SLEEP_MICRO_SECONDS);
   } /* endwhile */

   if (!time_out) {
      *secondary_ptr++ = DD_PIO_HIGH_WATER;
      *secondary_ptr++ = 0xC005;
      return(DYN_TIME_OUT_HOST_INT);
   } /* endif */

   /* check the return from the event_handler */
   if (ret_code) {
      *secondary_ptr++ = ret_code;
      *secondary_ptr++ = DD_PIO_HIGH_WATER;
      *secondary_ptr++ = 0xC005;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   /* check return code from adapter */          /* SYNC POINT (9) */
   ret_code = get_dynamic_return_info(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* write 10 words to the PIO causing DSP interrupt          (TEST 9)  */
   for (i = 0 ; i < 10 ; i++) {
      *pio = dynamic_patterns[k++];
   } /* endfor */

   *host_commo = CONTINUE;                       /* SYNC POINT (9.5) */

   /* check return code from adapter */          /* SYNC POINT (10) */
   ret_code = get_dynamic_return_info(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* write 32 words to the PIO causing DSP interrupt          (TEST 10)  */
   for (i = 0 ; i < 32 ; i++) {
      *pio = dynamic_patterns[i];
   } /* endfor */

   /* check return code from adapter */          /* SYNC POINT (11) */
   ret_code = get_dynamic_return_info(secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   /* clear the return code from the event handler */
   ret_code = 0;
   event_received = 0;

   *host_intr_mask = 0;  /* disable all interrupts */
   *host_status = 0x3FF; /* clear all pending interrupts */

   /* Set up for DSP SOFT interrupts                           (TEST 11)  */
   our_async_event.mask = DD_DIAGNOSTICS_COMPLETE |
                          DD_DSP_SOFT_INTR_0 |
                          DD_DSP_SOFT_INTR_1 |
                          DD_DSP_SOFT_INTR_2 |
                          DD_DSP_SOFT_INTR_3;
   our_async_event.num_events = 1;
   our_async_event.error = 0;

   if (aixgsc(our_gsc_handle.handle, ASYNC_EVENT , &our_async_event)){
      PRINT(("\nhost_bim_intr : ASYNC_EVENTS ERROR\n"));
      *secondary_ptr++ = ASYNC_EVENTS_ERROR;
      *secondary_ptr++ = DD_DSP_SOFT_INTR_0 |
                         DD_DSP_SOFT_INTR_1 |
                         DD_DSP_SOFT_INTR_2 |
                         DD_DSP_SOFT_INTR_3;
      *secondary_ptr++ = 0xC006;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   ret_code = set_mc_speed(300);
   if (ret_code) {
      return(ret_code);
   } /* endif */
   *host_commo = CONTINUE;                       /* SYNC POINT (12) */

   time_out = WAIT_ON_INTERRUPT_TIME;
   while (!event_received && time_out) {    /* wait for interrupt */
      time_out--;
      usleep(SLEEP_MICRO_SECONDS);
   } /* endwhile */

   if (!time_out) {
      *secondary_ptr++ = DD_DSP_SOFT_INTR_0 |
                         DD_DSP_SOFT_INTR_1 |
                         DD_DSP_SOFT_INTR_2 |
                         DD_DSP_SOFT_INTR_3;
      *secondary_ptr++ = 0xC006;
      return(DYN_TIME_OUT_HOST_INT);
   } /* endif */

   /* check the return from the event_handler */
   if (ret_code) {
      *secondary_ptr++ = ret_code;
      *secondary_ptr++ = DD_DSP_SOFT_INTR_0 |
                         DD_DSP_SOFT_INTR_1 |
                         DD_DSP_SOFT_INTR_2 |
                         DD_DSP_SOFT_INTR_3;
      *secondary_ptr++ = 0xC006;
      return(DYN_EVENT_SYSTEM_CALL_ERROR);
   } /* endif */

   /* check the events mask */
   expected = DD_DSP_SOFT_INTR_0 |
              DD_DSP_SOFT_INTR_1 |
              DD_DSP_SOFT_INTR_2 |
              DD_DSP_SOFT_INTR_3;
   actual = event_array.event[0].event;

   if (actual ^ expected) {
      *secondary_ptr++ = expected;
      *secondary_ptr++ = actual;
      *secondary_ptr++ = 0xC006;
      actual = event_array.event[0].data[0]; /* check the return from DSP */
      *secondary_ptr++ = actual;
      return(EVENT_MASK_ERROR);
   } /* endif */

   /* check final return code from adapter */
   ret_code = get_dynamic_return_info(secondary_ptr);

   return(ret_code);

} /* end bim_dynamic_test() */

/*
 * NAME: get_dynamic_return_info
 *
 * FUNCTION:  Waits for the return information from the bim_dynamic_test to be
 *            written into a special memory location on the adapter. Once
 *            the information is received, if it is non-zero it will copy
 *            the secondary return information into an array passed as an
 *            argument. It will always return as return code the return
 *            information it finds in the special memory location.
 *
 * EXECUTION ENVIRONMENT: AIX 3.1 during diagnostics
 *
 * NOTES:  The special memory location is currently defined as the tenth
 *         location of the secondary buffer on the adapter. It is assumed
 *         that before the call to this routine, the special memory location
 *         contains 0xFFFFFFFF. Any change from that value will be interpreted
 *         as a write by the microcode of the return info.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: Global variable dsp_secondary_ptr (in this file)
 *                  Global variable bim_base_addr (dd_interface.c)
 *
 * RETURNS: Value read from adapter or timeout error
 *
 * INPUT: secondary_ptr - address of array to hold secondary return information
 *
 * OUTPUT: None.
 */

int get_dynamic_return_info(long *secondary_ptr)
{
   volatile unsigned long *ind_control, *ind_address, *ind_data;
   int i, time_out;
   unsigned actual, expected;

   ind_control = bim_base_addr + IND_CONTROL;
   ind_address = bim_base_addr + IND_ADDRESS;
   ind_data = bim_base_addr + IND_DATA;

   /* wait for return information to be written into the special memory */
   /* location on the adapater.      (See NOTES: in the prolog)         */

   *ind_control = IND_READ | IND_DSPMEM;
   *ind_address = dsp_secondary_ptr + SPECIAL_SECONDARY_OFFSET;

   time_out = TEST_RETURN_TIMEOUT;

   actual = *ind_data;

   while ((actual == (unsigned)DYNAMIC_TEST_SPECIAL_VALUE) && time_out) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      actual = *ind_data;
   } /* endwhile */

   if (!time_out) {
      return(DYNAMIC_TEST_RETURN_TIMEOUT);
   } /* endif */

   if (actual) {
      *ind_control = IND_WRITE | IND_DSPMEM;
      *ind_address = dsp_secondary_ptr + SPECIAL_SECONDARY_OFFSET;
      *ind_data = 0;   /* clear the location in the secondary buffer */
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      /* Read secondary information from adapter using BIM's indirect mode  */
      *ind_control = IND_READ | IND_DSPMEM | IND_AUTOINC;
      *ind_address = dsp_secondary_ptr;
      for (i = 0 ; i < SEC_BUF_LEN ; i++) {
         secondary_ptr[i] = *ind_data;
         usleep(U_SECS_BETWEEN_REG_ACCESSES);
      } /* endfor */
   } else {
      *ind_control = IND_WRITE | IND_DSPMEM;
      *ind_address = dsp_secondary_ptr + SPECIAL_SECONDARY_OFFSET;
      *ind_data = DYNAMIC_TEST_SPECIAL_VALUE; /* reset the initial value */
   } /* endif */

   return(actual);     /* always return value read from the adapter */

} /* end get_dynamic_return_info() */


/*
 * NAME: event_handler()
 *
 * FUNCTION: handles the signal SIGMSG which is sent by the device driver
 *           when a requested event occurs. This is used in BIM host interrupt
 *           testing.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: ret_code (static global in this file)
 *                  our_get_events (global in this file)
 *
 * RETURNS : see below
 *
 * INPUT:
 *       None.
 * OUTPUT:
 *      Sets ret_code to zero if getting event report is successful, else sets
 *      ret_code to non-zero.
 */

void event_handler()
{
   PRINT(("\nInside event_handler()\n"));

   our_get_events.num_events = our_async_event.num_events;

   if (aixgsc(our_gsc_handle.handle, GET_EVENTS, &our_get_events)){
      PRINT(("\nevent_handler : GET_EVENTS ERROR\n"));
      ret_code = GET_EVENTS_ERROR;
   } else {
      ret_code = 0;
   } /* endif */

   if (our_get_events.num_events) {
      event_received++;
      if (!ret_code) {
         ret_code = set_mc_speed(300);
      } else {
         ret_code = set_mc_speed(300);
         ret_code = GET_EVENTS_ERROR;
      } /* endif */
   } /* endif */

   (void)signal(SIGMSG,event_handler);

} /* end event_handler() */

