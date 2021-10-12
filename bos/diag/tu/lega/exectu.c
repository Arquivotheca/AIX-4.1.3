static char sccsid[] = "@(#)12  1.31  src/bos/diag/tu/lega/exectu.c, tu_lega, bos411, 9428A410j 4/20/94 16:44:41";
/*
 * COMPONENT_NAME: (tu_lega) Low End Graphics Adapter Test Units
 *
 * FUNCTIONS: exectu(), restart_dsp(), dsp_execute(), load_microcode(),
 *            load_c30b_microcode(), get_secondary_info(),
 *            send_to_dsp(), receive_from_dsp().
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * MODULE NAME: exectu.c
 *
 * DEPENDENCIES:  None.
 *
 * RESTRICTIONS:  None.
 *
 * EXTERNAL REFERENCES
 *
 *     OTHER ROUTINES: load_executable_coff() in load_coff.c
 *                     partial_load_executable_coff() load_coff.c
 *                     setup() in dd_interface.c
 *                     bim_dma_test() in bim_dma.c
 *                     bim_dynamic_test() in h_dynamic.c
 *                     host_interrupt_test() in h_bim_intr.c
 *
 *     DATA AREAS:  Global Variable - bim_base_addr in dd_interface.c
 *
 *     TABLES:  None.
 *
 *     MACROS:  None.
 *
 * COMPILER / ASSEMBLER
 *
 *     TYPE, VERSION: AIX 3.2 XLC Compiler
 *
 *     OPTIONS:
 *
 * NOTES:  None.
 *
 */

#include   <stdio.h>
#include   <fcntl.h>
#include   <cf.h>
#include   "bim_defs.h"
#include   "host_dsp.h"
#include   "tu_type.h"

int dsp_execute(unsigned long command,long *secondary_ptr);
int get_secondary_info(long *secondary_ptr);
int load_microcode(FILE *coff_file,long *secondary_ptr);
int load_c30b_microcode(long *secondary_ptr);
int send_to_dsp(unsigned long data,long *secondary_ptr);

/* error codes returned from this module */
#define INITIAL_PROTOCOL_TIMED_OUT         0x30
#define GET_SECONDARY_ADDRESS_TIMED_OUT    0x31
#define COMMAND_TIMED_OUT                  0x32
#define READY_PROTOCOL_TIMED_OUT           0x33
#define GET_C30B_BOOTSTRAP_ADDRESS_TIMED_OUT  0x34
#define GET_C30B_DIAG_MCODE_ADDRESS_TIMED_OUT 0x35
#define GET_C30B_MEM_MCODE_ADDRESS_TIMED_OUT  0x36
#define ERROR_LOADING_C30B_BOOTSTRAP       0x37
#define ERROR_LOADING_C30B_DIAG_MCODE      0x38
#define ERROR_LOADING_C30B_MEM_MCODE       0x39
#define INVALID_TU_NUMBER                  0x3A
#define TU_OPEN_ALREADY_CALLED_BEFORE      0x3B
#define CANNOT_TU_CLOSE_BEFORE_TU_OPEN     0x3C
#define MICROCODE_FILE_NOT_FOUND           0x3F
#define BIM_FIX_FAILED                     0x40
#define NO_ERROR 0

/* constants used in this file */
#define INITIAL_PROTOCOL_TIMEOUT      0x10000
#define GET_ADDRESS_TIMEOUT           0x10000
#define TEST_RETURN_TIMEOUT           0x30000
#define READY_PROTOCOL_TIMEOUT        0x10000
#define U_SECS_BETWEEN_REG_ACCESSES   1000

#define GET_C30B_BOOTSTRAP_ADDRESS_TIMEOUT  0x10000
#define GET_C30B_DIAG_MCODE_ADDRESS_TIMEOUT 0x10000
#define GET_C30B_MEM_MCODE_ADDRESS_TIMEOUT  0x10000
#define GET_SECONDARY_ADDRESS_TIMEOUT 0x10000

#define DSP_IO_STATUS               0x400017  /*IO status register */
#define RESET_DSP_IO_STATUS     0x1FFFF   /*Reset command to io status */
#define RESET_HOST_INTR_MASK_REG 0x1FFFF  /*Reset cmd to host intr mask reg */

/* External routines used in this file */
   extern int tu_open(char *logical_devname);
   extern int tu_close(void);
   extern int load_executable_coff();
   extern int partial_load_executable_coff();
   extern int bim_dma_test();
   extern int bim_dynamic_test();
   extern int host_interrupt_test();

/* External GLOBAL variables accessed in this file */
   extern volatile unsigned long *bim_base_addr;

/* GLOBAL variables in this file which are accessed in other files */
   int c30b_bootstrap_ptr;  /* address of c30b bootstrap buffer */
   int c30b_diag_mcode_ptr; /* address of c30b diag. microcode buffer */
   int c30b_mem_mcode_ptr;  /* address of c30b mem. microcode buffer */
   int dsp_secondary_ptr;   /* address of the secondary buffer of */
                            /* the microcode on the adapter */
   int setup_performed = 0; /* Flag to tell us that setup has been performed */

   int crt_refresh_rate = 60; /* default CRT refresh rate is 60 */
   int reload_microcode = 1;

/*
 * NAME: exectu
 *
 * FUNCTION: This function is the external interface for the execution of
 *           the test units
 *
 * EXECUTION ENVIRONMENT: AIX v3.1 during LEGA diagnostics
 *
 * NOTES: None
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns zero if successful, else returns error code.
 */
/*
 * INPUT:
 *        fdes - file descriptor for the device driver
 *        tucb_ptr - pointer to the test unit control block
 * OUTPUT:
 *
 */

int exectu(char *logical_device_name, LEGA_TU_TYPE *tucb_ptr)
{
   int ret_code = 0;
   int loop_count, tu_number;
   char mc_file[50];
   int i;


   /* clear secondary return info buffer */
   for (i = 0; i < SEC_BUF_LEN ; i++) {
      tucb_ptr->secondary_ptr[i] = 0;
   } /* endfor */

   /* Extract TU number and number of times to execute */
   loop_count = tucb_ptr->loop;
   tu_number = tucb_ptr->tu;

   /* Due to a bug in the BIM chip, set the bus cycles to 300 ns */
   if (setup_performed) {
     if (ret_code = set_mc_speed(300)) {
       return(ret_code);
     } /* endif */
   } /* endif */

   switch (tu_number) {
   case TU_02:
      *(bim_base_addr + DSP_CONTROL) = RESET_DSP;
      while (!ret_code && loop_count) {
         ret_code = vpd_test(tucb_ptr->secondary_ptr);
         loop_count--;
      } /* endwhile */
      reload_microcode = 1;
      break;
   case TU_03:
      *(bim_base_addr + DSP_CONTROL) = RESET_DSP;
      while (!ret_code && loop_count) {
         ret_code = host_bim_test(tucb_ptr->secondary_ptr);
         loop_count--;
      } /* endwhile */
      reload_microcode = 1;
      break;
   case TU_04:  /* host_mem_test for LEGA and LEGA3 only (test 256K words) */
      *(bim_base_addr + DSP_CONTROL) = RESET_DSP;
      while (!ret_code && loop_count) {
         ret_code = host_mem_test(tucb_ptr->secondary_ptr, 0x00040000,
                                  tu_number);
         loop_count--;
      } /* endwhile */
      reload_microcode = 1;
      break;
   case TU_05:      /* LEGA and LEGA3 TU number */
   case TU_35:      /* LEGA2 TU number */
      ret_code = findmcode(C30M_MEM_MCODE,mc_file,VERSIONING,0);
      if (!ret_code) {
         return(MICROCODE_FILE_NOT_FOUND);
      } /* endif */
      ret_code = load_microcode((FILE *)mc_file,tucb_ptr->secondary_ptr);
      while (!ret_code && loop_count) {
         ret_code = dsp_execute(tu_number,tucb_ptr->secondary_ptr);
         loop_count--;
      } /* endwhile */
      reload_microcode = 1;
      break;
   case TU_08:
      if (reload_microcode) {
         ret_code = findmcode(C30M_DIAG_MCODE,mc_file,VERSIONING,0);
         if (!ret_code) {
            return(MICROCODE_FILE_NOT_FOUND);
         } /* endif */
         ret_code = load_microcode((FILE *)mc_file,tucb_ptr->secondary_ptr);
      } /* endif */
      while (!ret_code && loop_count) {
         ret_code = host_interrupt_test(tu_number,tucb_ptr->secondary_ptr);
         loop_count--;
      } /* endwhile */
      break;
   case TU_09:
      if (reload_microcode) {
         ret_code = findmcode(C30M_DIAG_MCODE,mc_file,VERSIONING,0);
         if (!ret_code) {
            return(MICROCODE_FILE_NOT_FOUND);
         } /* endif */
         ret_code = load_microcode((FILE *)mc_file,tucb_ptr->secondary_ptr);
      } /* endif */
      while (!ret_code && loop_count) {
         ret_code = bim_dma_test(tu_number,tucb_ptr->secondary_ptr);
         loop_count--;
      } /* endwhile */
      break;
   case TU_10:
      if (reload_microcode) {
         ret_code = findmcode(C30M_DIAG_MCODE,mc_file,VERSIONING,0);
         if (!ret_code) {
            return(MICROCODE_FILE_NOT_FOUND);
         } /* endif */
         ret_code = load_microcode((FILE *)mc_file,tucb_ptr->secondary_ptr);
      } /* endif */
      while (!ret_code && loop_count) {
         ret_code = bim_dynamic_test(tu_number,tucb_ptr->secondary_ptr);
         loop_count--;
      } /* endwhile */
      break;
   case TU_06:
   case TU_07:
   case TU_11:
   case TU_12:
   case TU_13:
   case TU_14:
   case TU_15:
   case TU_16:
   case TU_17:
   case TU_18:
   case TU_19:
   case TU_20:
   case TU_21:
   case TU_22:
   case TU_23:
   case TU_24:
   case TU_25:
   case TU_26:
   case TU_27:
   case TU_28:
   case TU_29:
   case TU_30:
   case TU_31:
   case TU_32:
   case TU_33:
   case TU_36:
   case TU_40:
   case TU_41:
   case TU_42:
   case TU_45:
   case TU_60:
   case TU_77:
      if (reload_microcode || (tu_number == TU_06) || (tu_number == TU_13)) {
         ret_code = findmcode(C30M_DIAG_MCODE,mc_file,VERSIONING,0);
         if (!ret_code) {
            return(MICROCODE_FILE_NOT_FOUND);
         } /* endif */
         ret_code = load_microcode((FILE *)mc_file,tucb_ptr->secondary_ptr);
         if (!ret_code) {
            ret_code = load_c30b_microcode(tucb_ptr->secondary_ptr);
         } /* endif */

         /* Check to see if we need to set the refresh rate. Every time */
         /* the microcode is reloaded, the crt refresh rate defaults to */
         /* 60 Hz. Therefore, if the global variable says that we need  */
         /* to be running at 77 Hz. and the TU requested is neither     */
         /* TU_60 or TU_77, which will change the setting anyways, then */
         /* execute TU_77 to set the refresh rate in the microcode.     */
         /* Also for memory test (TU_06) we cannot set the refresh rate */
         /* due to the limitation of file c30b_mem.c .                  */

         if ( (!ret_code) && (crt_refresh_rate == 77) &&
              (tu_number != TU_60) && (tu_number != TU_77) &&
              (tu_number != TU_06) ) {

            ret_code = dsp_execute(TU_77,tucb_ptr->secondary_ptr);

         } /* endif */

         reload_microcode = 0;
      } /* endif */
      while (!ret_code && loop_count) {
         ret_code = dsp_execute(tu_number,tucb_ptr->secondary_ptr);
         loop_count--;
      } /* endwhile */

      switch (tu_number) {
      case TU_06:
      case TU_40:
      case TU_41:
      case TU_42:                /* cause reload after memory test or */
      case TU_45:                /* infinite loop tests               */
        reload_microcode = 1;
        break;
      case TU_60:
      case TU_77:
        crt_refresh_rate = tu_number;
        break;
      } /* endswitch */

      break;
   case TU_34:  /* host_mem_test for LEGA2 only (test 128K words) */
      *(bim_base_addr + DSP_CONTROL) = RESET_DSP;
      while (!ret_code && loop_count) {
         ret_code = host_mem_test(tucb_ptr->secondary_ptr, 0x00020000,
                                  tu_number);
         loop_count--;
      } /* endwhile */
      reload_microcode = 1;
      break;
   case TU_43:
   case TU_44:
      ret_code = findmcode(C30M_DIAG_MCODE,mc_file,VERSIONING,0);
      if (!ret_code) {
         return(MICROCODE_FILE_NOT_FOUND);
      } /* endif */
      ret_code = load_microcode((FILE *)mc_file,tucb_ptr->secondary_ptr);
      if (!ret_code) {
         ret_code = load_c30b_microcode(tucb_ptr->secondary_ptr);
      } /* endif */
      if (!ret_code) {
         ret_code = dsp_execute(tu_number,tucb_ptr->secondary_ptr);
      } /* endif */
      if (!ret_code) {
         ret_code = bim_get_hot();
      } /* endif */
      reload_microcode = 1;
      break;

    case TU_OPEN: /* open path thru OS, initialize and obtain base address */
      if (!setup_performed) {
         setup_performed = 1;
	 reload_microcode = 1;
         ret_code = tu_open(logical_device_name);
      } else {
         ret_code = TU_OPEN_ALREADY_CALLED_BEFORE;
      } /* endif */
      break;

    case TU_CLOSE: /* clean up and close path thru OS */
      if (setup_performed) {
         setup_performed = 0;
         ret_code = tu_close();
      } else {
         ret_code = CANNOT_TU_CLOSE_BEFORE_TU_OPEN;
      } /* endif */
      break;

   default:
     ret_code = INVALID_TU_NUMBER;
   } /* endswitch */

   if (ret_code) {
     reload_microcode = 1;   /* cause reload if an error occured */
   } /* endif */

   return(ret_code);

} /* end exectu() */

/*
 * NAME: restart_dsp
 *
 * FUNCTION:  Resets and starts the DSP microcode. This is done by toggling
 *            the DSP reset bit in the DSP CONTROL register of the BIM.
 *            After releasing the microcode to run, it will handshake with
 *            the microcode to see if it is up and running, and will obtain
 *            the address of the secondary return information area on the
 *            adapter.
 *
 * EXECUTION ENVIRONMENT: AIX 3.1 during diagnostics
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS: Zero if no error, non-zero if error.
 *
 * INPUT: secondary_ptr - address of secondary return information array
 *
 * OUTPUT: None.
 */

int restart_dsp(long *secondary_ptr)
{
   volatile unsigned long *dsp_commo, *host_commo, *dsp_control;
   volatile unsigned long *ind_control,*ind_address, *ind_data, *host_status;
   unsigned long actual, expected;
   register unsigned status;
   int time_out;

   dsp_control = bim_base_addr + DSP_CONTROL;
   host_commo = bim_base_addr + HOST_COMMO;
   dsp_commo = bim_base_addr + DSP_COMMO;

   host_status = bim_base_addr + HOST_STATUS;
   ind_control = bim_base_addr + IND_CONTROL; /* set up pointers */
   ind_address = bim_base_addr + IND_ADDRESS;
   ind_data =    bim_base_addr + IND_DATA;

   *dsp_control = RESET_DSP;
   usleep(100000);

   /* Clear io status register so when on DSP side, all ints are cleared */
   *ind_control = IND_WRAPMODE | IND_WRITE;
   *ind_address = DSP_IO_STATUS;   /* write to io_status register */
   *ind_data = RESET_DSP_IO_STATUS;  /* clear io status register */

   *dsp_control = RELEASE_DSP;

   /* Perform initial protocol : Request pattern sequence in DSP_COMMO by */
   /*                            issuing command through HOST_COMMO       */


   *host_status = RESET_HOST_INTR_MASK_REG; /*reset host_intr mask register */
   *host_commo = NEXT_PAT; /* request next pattern */

   time_out = INITIAL_PROTOCOL_TIMEOUT;
   expected = PAT0;
   /* Check the status bit to see if bim notified host it's done */
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;       /*read data fro DSP */
   if (!time_out || (actual != expected) ) {
      *secondary_ptr++ = expected;
      *secondary_ptr++ = actual;
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC01;  /* Check Point 1 */
      return(INITIAL_PROTOCOL_TIMED_OUT);
   } /* endif */


   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = NEXT_PAT2; /* request next pattern */

   time_out = INITIAL_PROTOCOL_TIMEOUT;
   expected = PAT1;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;       /*read data fro DSP */

   if (!time_out || (actual != expected) ) {
      *secondary_ptr++ = expected;
      *secondary_ptr++ = actual;
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC02;  /* Check Point 2 */
      return(INITIAL_PROTOCOL_TIMED_OUT);
   } /* endif */


   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = NEXT_PAT; /* request next pattern */

   time_out = INITIAL_PROTOCOL_TIMEOUT;
   expected = PAT2;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;
   if (!time_out && (actual != expected) ) {
      *secondary_ptr++ = expected;
      *secondary_ptr++ = actual;
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC03;  /* Check Point 3 */
      return(INITIAL_PROTOCOL_TIMED_OUT);
   } /* endif */


   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = NEXT_PAT2; /* request next pattern */

   time_out = INITIAL_PROTOCOL_TIMEOUT;
   expected = PAT3;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;
   if (!time_out || (actual != expected) ) {
      *secondary_ptr++ = expected;
      *secondary_ptr++ = actual;
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC04;  /* Check Point 4 */
      return(INITIAL_PROTOCOL_TIMED_OUT);
   } /* endif */


   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = GIVE_SECONDARY_ADDRESS; /* request secondary address */

   time_out = GET_ADDRESS_TIMEOUT;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;
   if (!time_out) {
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC05;  /* Check Point 5 */
      return(GET_SECONDARY_ADDRESS_TIMED_OUT);
   } else {
      dsp_secondary_ptr = actual;
   } /* endif */

   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = GIVE_C30B_BOOTSTRAP_ADDRESS; /* request address */

   time_out = GET_ADDRESS_TIMEOUT;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;
   if (!time_out) {
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC06;  /* Check Point 6 */
      return(GET_C30B_BOOTSTRAP_ADDRESS_TIMED_OUT);
   } else {
      c30b_bootstrap_ptr = actual;
   } /* endif */


   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = GIVE_C30B_DIAG_MCODE_ADDRESS; /* request address */

   time_out = GET_ADDRESS_TIMEOUT;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;
   if (!time_out) {
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC07;  /* Check Point 7 */
      return(GET_C30B_DIAG_MCODE_ADDRESS_TIMED_OUT);
   } else {
      c30b_diag_mcode_ptr = actual;
   } /* endif */


   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = GIVE_C30B_MEM_MCODE_ADDRESS; /* request address */

   time_out = GET_ADDRESS_TIMEOUT;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;
   if (!time_out) {
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC08;  /* Check Point 8 */
      return(GET_C30B_MEM_MCODE_ADDRESS_TIMED_OUT);
   } else {
      c30b_mem_mcode_ptr = actual;
   } /* endif */


   return(NO_ERROR);     /* if we got here then all is well */

} /* end restart_dsp() */

/*
 * NAME: dsp_execute
 *
 * FUNCTION:  Handshakes with the DSP microcode on the adapter and then
 *            issues the command to be executed by the microcode through
 *            the HOST_COMMO register. The command usually is a request for
 *            a particular test to be run, and afterwards the return code
 *            is sent back to this function through the DSP_COMMO.
 *            The protocol is as follows :
 *
 *              HOST                                DSP
 *
 *     1.   "GET_READY" ------------------------->   X
 *
 *     2.       X   <---------------------------- "READY"
 *
 *     3.   Command Code ------------------------>   X
 *
 *     4.      HOST                               DSP TEST
 *           WAITING                               RUNNING
 *
 *     5.       X   <-------------------------- Return Code
 *
 *     6.    GO BACK TO 1 ABOVE
 *
 *           note : X designates one side waiting on the other for a specific
 *                  value.
 *
 * EXECUTION ENVIRONMENT: AIX 3.1 during diagnostics
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS: Zero if no error, non-zero if error.
 *
 * INPUT: command - to be executed by the DSP
 *        secondary_ptr - address of secondary return information array
 *
 * OUTPUT: None.
 */

int dsp_execute(unsigned long command,long *secondary_ptr)
{
   volatile unsigned long *dsp_commo, *host_commo, *dsp_control, *host_status;
   unsigned long actual, expected;
   register unsigned status;
   int time_out, ret_code;

   dsp_control = bim_base_addr + DSP_CONTROL;
   host_commo = bim_base_addr + HOST_COMMO;
   dsp_commo = bim_base_addr + DSP_COMMO;

   host_status = bim_base_addr + HOST_STATUS;

   /* send command (TU number) to be executed */
   /* This call also clears the status for write to DSP COOMO reg */
   ret_code = send_to_dsp(command,secondary_ptr);
   if (ret_code) {
      return(ret_code);
   } /* endif */

   time_out = TEST_RETURN_TIMEOUT;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   ret_code = *dsp_commo;

   if (!time_out) {
      secondary_ptr[0] = command;
      secondary_ptr[1] = ret_code;
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC09;  /* Check Point 9 */
      return(COMMAND_TIMED_OUT);
   } /* endif */

   if (ret_code) {
      get_secondary_info(secondary_ptr);
   } /* endif */

   return(ret_code);

} /* end dsp_execute() */

/*
 * NAME: get_secondary_info
 *
 * FUNCTION:  Copies the secondary return information from the area on the
 *            adapter into the array passed as parameter.
 *
 * EXECUTION ENVIRONMENT: AIX 3.1 during diagnostics
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: Global variable dsp_secondary_ptr (in this file)
 *                  Global variable bim_base_addr (dd_interface.c)
 *
 * RETURNS: Always Zero.
 *
 * INPUT: secondary_ptr - address of array to hold secondary return information
 *
 * OUTPUT: None.
 */

int get_secondary_info(long *secondary_ptr)
{
   volatile unsigned long *ind_control, *ind_address, *ind_data;
   int i;

   ind_control = bim_base_addr + IND_CONTROL;
   ind_address = bim_base_addr + IND_ADDRESS;
   ind_data = bim_base_addr + IND_DATA;

   /* Read secondary information from adapter using BIM's indirect mode  */

   *ind_control = IND_READ | IND_DSPMEM | IND_AUTOINC;
   *ind_address = dsp_secondary_ptr;
   for (i = 0 ; i < SEC_BUF_LEN ; i++) {
      secondary_ptr[i] = *ind_data;
   } /* endfor */

   return(0);     /* always return 0 */

} /* end get_secondary_info() */

/*
 * NAME: load_microcode
 *
 * FUNCTION:  Loads the specified microcode file into the C30M memory and
 *            then restarts the microcode.
 *
 * EXECUTION ENVIRONMENT: AIX 3.1 during diagnostics
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: Zero if successful, else error code.
 *
 * INPUT: coff_file - name of COFF format microcode file to be loaded.
 *        secondary_ptr - address of secondary return information array
 *
 * OUTPUT: None.
 */

int load_microcode(FILE *coff_file,long *secondary_ptr)
{
   int ret_code;

   ret_code = load_executable_coff(coff_file);
   if (!ret_code) {
      ret_code = restart_dsp(secondary_ptr);
   } /* endif */
   return(ret_code);

} /* end load_microcode() */

/*
 * NAME: load_c30b_microcode
 *
 * FUNCTION:  Obtains the addresses of the buffers reserved for the C30B
 *            microcode set from the C30M controller and loads those buffers
 *            with the code.
 *
 * EXECUTION ENVIRONMENT: AIX 3.1 during diagnostics
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: Zero if successful, else error code.
 *
 * INPUT: secondary_ptr - address of secondary return information array
 *
 * OUTPUT: None.
 */

int load_c30b_microcode(long *secondary_ptr)
{
   int ret_code;
   char mc_file[50];

   ret_code = findmcode(C30B_BOOTSTRAP,mc_file,VERSIONING,0);
   if (!ret_code) {
      return(MICROCODE_FILE_NOT_FOUND);
   } /* endif */
   ret_code = partial_load_executable_coff(mc_file,c30b_bootstrap_ptr);
   if (ret_code) {
      *secondary_ptr++ = ret_code;
      return(ERROR_LOADING_C30B_BOOTSTRAP);
   } /* endif */

   ret_code = findmcode(C30B_DIAG_MCODE,mc_file,VERSIONING,0);
   if (!ret_code) {
      return(MICROCODE_FILE_NOT_FOUND);
   } /* endif */
   ret_code = partial_load_executable_coff(mc_file,c30b_diag_mcode_ptr);
   if (ret_code) {
      *secondary_ptr++ = ret_code;
      return(ERROR_LOADING_C30B_DIAG_MCODE);
   } /* endif */

   ret_code = findmcode(C30B_MEM_MCODE,mc_file,VERSIONING,0);
   if (!ret_code) {
      return(MICROCODE_FILE_NOT_FOUND);
   } /* endif */
   ret_code = partial_load_executable_coff(mc_file,c30b_mem_mcode_ptr);
   if (ret_code) {
      *secondary_ptr++ = ret_code;
      return(ERROR_LOADING_C30B_MEM_MCODE);
   } /* endif */

} /* end load_c30b_microcode() */

/*
 * NAME: send_to_dsp
 *
 * FUNCTION:  Handshakes with the DSP microcode on the adapter, and then
 *            sends the data passed as a parameter to this function through
 *            the HOST_COMMO register to the DSP.
 *            The protocol is as follows :
 *
 *              HOST                                DSP
 *
 *     1.   "GET_READY" ------------------------->   X
 *
 *     2.       X   <---------------------------- "READY"
 *
 *     3.   Command Code ------------------------>   X
 *
 *           note : X designates one side waiting on the other for a specific
 *                  value.
 *
 * EXECUTION ENVIRONMENT: AIX 3.1 during LEGA diagnostics
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS: Zero if no error, non-zero if error.
 *
 * INPUT:
 *        data          - data to be sent to the DSP
 *
 * OUTPUT:
 *       return code
 */

int send_to_dsp(unsigned long data,long *secondary_ptr)
{
   volatile unsigned long *dsp_commo, *host_commo,*host_status;
   unsigned long actual, expected;
   register unsigned status;
   int time_out;

   host_commo = bim_base_addr + HOST_COMMO;
   dsp_commo = bim_base_addr + DSP_COMMO;
   host_status = bim_base_addr + HOST_STATUS;

   /* Tell microcode to GET READY and wait on READY */
   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = GET_READY;

   time_out = READY_PROTOCOL_TIMEOUT;
   expected = READY;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;
   if (!time_out || (actual != expected) ) {
      *secondary_ptr++ = expected;
      *secondary_ptr++ = actual;
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC0A;  /* Check Point 10 */
      return(READY_PROTOCOL_TIMED_OUT);
   } /* endif */

   /* Send the data to dsp */
   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = data;

   return(0);

} /* send_to_dsp() */

/*
 * NAME: receive_from_dsp
 *
 * FUNCTION:  Waits for data to be sent from the DSP microcode on the adapter
 *            via the DSP_COMMO register. Then it will store that data into
 *            location pointed to by a pointer passed as parameter.
 *            The protocol is as follows :
 *
 *              HOST                                DSP
 *
 *     4.      HOST                               DSP TEST
 *           WAITING                               RUNNING
 *
 *     5.       X   <-------------------------- Return Code
 *
 *     6.    GO BACK TO 1 ABOVE
 *
 *           note : X designates one side waiting on the other for a specific
 *                  value.
 *
 * EXECUTION ENVIRONMENT: AIX 3.1 during diagnostics
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS: Zero if no error, non-zero if error.
 *
 * INPUT:
 *      data_ptr - pointer to data word where received data is to be stored
 *
 * OUTPUT: None.
 */

int receive_from_dsp(long *data_ptr,long *secondary_ptr)
{
   volatile unsigned long *dsp_commo, *host_commo, *host_status;
   unsigned long actual, expected;
   register unsigned status;
   int time_out;

   dsp_commo = bim_base_addr + DSP_COMMO;
   host_commo = bim_base_addr + HOST_COMMO;
   host_status = bim_base_addr + HOST_STATUS;

   /* Tell microcode to GET READY and wait on READY */
   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = GET_READY;

   time_out = READY_PROTOCOL_TIMEOUT;
   expected = READY;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;
   if (!time_out || (actual != expected) ) {
      *secondary_ptr++ = expected;
      *secondary_ptr++ = actual;
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC0B;  /* Check Point 11 */
      return(READY_PROTOCOL_TIMED_OUT);
   } /* endif */

   /* Send dummy data to dsp */
   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = ~(GET_READY);

   /* Wait for write by dsp (dsp_commo value to change from READY) */

   time_out = TEST_RETURN_TIMEOUT;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;

   if (!time_out) {
      *data_ptr = 0xDEADBEEF;
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC0C;  /* Check Point 12 */
      return(COMMAND_TIMED_OUT);
   } /* endif */

   *data_ptr = actual;

   return(0);

} /* end receive_from_dsp() */

/*
 * NAME: ready_protocol
 *
 * FUNCTION:  Handshakes with the DSP microcode on the adapter, using the
 *            GET_READY / READY protocol.
 *            The protocol is as follows :
 *
 *              HOST                                DSP
 *
 *     1.   "GET_READY" ------------------------->   X
 *
 *     2.       X   <---------------------------- "READY"
 *
 *
 *           note : X designates one side waiting on the other for a specific
 *                  value.
 *
 * EXECUTION ENVIRONMENT: AIX 3.1 during LEGA diagnostics
 *
 * NOTES:  None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS: Zero if no error, non-zero if error.
 *
 * INPUT: None.
 *
 * OUTPUT:
 *       return code
 */

int ready_protocol(long *secondary_ptr)
{
   volatile unsigned long *dsp_commo, *host_commo, *host_status;
   unsigned long actual, expected;
   register unsigned status;
   int time_out;

   host_commo = bim_base_addr + HOST_COMMO;
   dsp_commo = bim_base_addr + DSP_COMMO;
   host_status = bim_base_addr + HOST_STATUS;

   /* Tell microcode to GET READY and wait on READY */
   *host_status = WRITE_TO_DSP_COMMO; /* clear status bit */
   *host_commo = GET_READY;

   time_out = READY_PROTOCOL_TIMEOUT;
   expected = READY;
   status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   while (time_out && (!status) ) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      status = ( (*host_status) & WRITE_TO_DSP_COMMO);
   } /* endwhile */
   actual = *dsp_commo;
   if (!time_out || (actual != expected) ) {
      *secondary_ptr++ = expected;
      *secondary_ptr++ = actual;
      *secondary_ptr++ = time_out;
      *secondary_ptr++ = 0xCEC0D;  /* Check Point 13 */
      return(READY_PROTOCOL_TIMED_OUT);
   } /* endif */


   return(0);

} /* ready_protocol() */

/*
 * NAME: send_tu_number
 *
 * FUNCTION:  sends the data passed as a parameter to this function through
 *            the HOST_COMMO register to the DSP.
 *
 * EXECUTION ENVIRONMENT: AIX 3.1 during LEGA diagnostics
 *
 * NOTES:  This function is meant to be called after the ready_protocol().
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: None.
 *
 * RETURNS: Zero if no error, non-zero if error.
 *
 * INPUT:
 *        data          - data to be sent to the DSP
 *
 * OUTPUT:
 *       return code
 */

int send_tu_number(unsigned long data)
{
   volatile unsigned long *dsp_commo, *host_commo;
   unsigned long actual, expected;
   int time_out;

   host_commo = bim_base_addr + HOST_COMMO;
   dsp_commo = bim_base_addr + DSP_COMMO;

   /* Send the data to dsp */
   *host_commo = data;

   time_out = 25;
   while ((*host_commo != data) && time_out) {
      time_out--;
      usleep(U_SECS_BETWEEN_REG_ACCESSES);
      *host_commo = data;
   } /* endwhile */
   if (!time_out) {
      return(BIM_FIX_FAILED);
   } /* endif */

   return(0);

} /* send_tu_number() */

