static char sccsid[] = "@(#)14  1.8.1.3  src/bos/diag/tu/lega/host_bim_dma.c, tu_lega, bos411, 9428A410j 12/10/93 15:37:12";
/*
 * COMPONENT_NAME: (tu_lega) Low End Graphics Adapter Test Units
 *
 * FUNCTIONS: do_dma(), bim_dma_test(), bim_get_hot(),
 *            send_to_dsp(), receive_from_dsp()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * MODULE NAME: host_bim_dma.c
 *
 * STATUS: Release 1, EC 00, EVT Version 1
 *
 * DEPENDENCIES: None
 *
 * RESTRICTIONS: None
 *
 * EXTERNAL REFERENCES:
 *
 *      OTHER ROUTINES: send_to_dsp() in exectu.c
 *                      receive_from_dsp() in exectu.c
 *
 *      DATA AREAS:  bim_base_address  defined in dd_interface.c
 *                   our_gsc_handle        defined in dd_interface.c
 *
 *      TABLES:
 *
 *      MACROS: None
 *
 * COMPILER/ASSEMBLER
 *
 *      TYPE, VERSION: AIX v3.1, C compiler
 *
 *      OPTIONS:
 *
 * NOTES: None
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/rcm_win.h>
#include <sys/rcmioctl.h>
#include <sys/aixgsc.h>
#include <mid/mid.h>

#include "bim_defs.h"
#include "host_dsp.h"

/* Error Codes returned by functions in this file */
#define HOST_BUFFER_MALLOC_ERROR     0x900
#define AIXGSC_DMA_SERVICE_ERROR     0x901
#define DMA_HOST_PATTERN_CHECK_ERROR 0x902

/* Constants used in this file */
#define DMA_TEST_1_BUFFER_SIZE 0x500
#define DMA_TEST_2_BUFFER_SIZE 0x500
#define DMA_TEST_3_BUFFER_SIZE 0x3000
#define DMA_TEST_4_BUFFER_SIZE 0x100
#define DMA_TEST_5_BUFFER_SIZE 0x4000
#define DMA_TEST_6_BUFFER_SIZE 0xC
#define DMA_TEST_7_BUFFER_SIZE 0x10
#define DMA_TEST_8_BUFFER_SIZE 0x30
#define DMA_TEST_9_BUFFER_SIZE 0x40
#define DMA_TEST_10_BUFFER_SIZE 0x4000
#define DMA_TEST_11_BUFFER_SIZE 0x300


   /* external data structures */
   extern volatile unsigned long *bim_base_addr;
   extern gsc_handle our_gsc_handle;

   /* external routines */
   extern int send_to_dsp();
   extern int receive_from_dsp();
   extern set_mc_speed();

int do_dma(int rw_flag, char *host_addr, int length, int width, int stride,
           int offset, int sw_length);

/* NAME: bim_dma_test()
 *
 * FUNCTION: This function will execute on the host and will communicate
 *           with its sister function on the adapter. It will initiate
 *           many DMA transfers of different types and sizes, in order to
 *           test BIM's Bus Master function.
 *
 * EXECUTION ENVIRONMENT: AIX during LEGA diagnostics
 *
 * NOTES: the following tests were selected from a group of tests
 *        used in simulation to verify the BIM Bus Master function
 *        on AUSSIM. The Ref. number in parenthesis refer to the
 *        original test case. The selection was made by consulting
 *        the BIM designer, and identifying the minimum subset of
 *        test cases required to verify the BUS MASTER function.
 *        Most of the HOST ADDRESSES have been changed due to some
 *        memory limitations, however, in all cases the lowest 4
 *        bits of the address have been retained.
 *
 * RECOVERY OPERATION: None
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This test will return zero if successful, else it will return
 *          non-zero.
 */
/*
 * INPUT:
 *         tu_number - test unit number for this test (passed down to mcode)
 *         secondary_ptr - pointer to secondary return buffer.
 *
 * OUTPUT: zero if no errors, else returns error code and
 *            secondary_ptr[0] - DMA test case number
 */

int bim_dma_test(int tu_number, int *secondary_ptr)
{
   int ret_code, dsp_return, i, j, k, l;
   char *host_buffer, *host_address, pattern;
   int length, width, stride, host_offset;
   int error;
   unsigned int actual, expected, increment;

   ret_code = 0;

   /****************************************************************/
   /* DMA TEST 1, From RIOS memory, To DSP I/O RAM, (Ref. BIM2D65) */
   /****************************************************************/

   host_buffer = (char *)malloc(DMA_TEST_1_BUFFER_SIZE);

   if (!host_buffer) {
      return(HOST_BUFFER_MALLOC_ERROR);
   } /* endif */

   for (i = 0 ; i < DMA_TEST_1_BUFFER_SIZE ; i++) {
      host_buffer[i] = 0;
   } /* endfor */

   ret_code = send_to_dsp(tu_number,secondary_ptr);

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 1; /* indicate first test case */
      return(ret_code);
   } /* endif */

   length = 0x200;
   width = 0x40;
   stride = 0x40;
   host_address = host_buffer;
   host_offset = 0xC;

   /* initialize the buffer to be sent */
   for (i = 0 ; i < length ; i++) {
      *(host_address + host_offset + i) = i & 0xFF;
   } /* endfor */

   ret_code =  do_dma( DMA_WRITE, host_address, length, width, stride,
                       host_offset, DMA_TEST_1_BUFFER_SIZE );

   free(host_buffer);

   if (ret_code) {
      secondary_ptr[0] = 1; /* indicate first test case */
      return(ret_code);
   } /* endif */

   /* get the results of the microcode side of this test */
   ret_code = receive_from_dsp(&dsp_return,secondary_ptr);
   if (ret_code) {
      secondary_ptr[0] = 1; /* indicate first test case */
      return(ret_code);
   } /* endif */
   if (dsp_return) {
      secondary_ptr[0] = 1; /* indicate first test case */
      return(dsp_return);
   } /* endif */

   /****************************************************************/
   /* DMA TEST 2, From RIOS memory, To DSP I/O RAM, (Ref. BIM2D50) */
   /****************************************************************/

   host_buffer = (char *)malloc(DMA_TEST_2_BUFFER_SIZE);

   if (!host_buffer) {
      return(HOST_BUFFER_MALLOC_ERROR);
   } /* endif */

   for (i = 0 ; i < DMA_TEST_2_BUFFER_SIZE ; i++) {
      host_buffer[i] = 0;
   } /* endfor */

   /* hand shake with microcode */
   ret_code = send_to_dsp(tu_number,secondary_ptr);

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 2; /* indicate second test case */
      return(ret_code);
   } /* endif */

   length = 0x168;
   width = 0xC;
   stride = 0x4;
   host_address = host_buffer;
   host_offset = 0xC;

   /* initialize the buffer to be sent */
   for (i = 0 ; i < 0x80 ; i++) {
      *(host_address + host_offset + i) = i;
   } /* endfor */

   ret_code =  do_dma( DMA_WRITE, host_address, length, width, stride,
                       host_offset, DMA_TEST_2_BUFFER_SIZE );

   free(host_buffer);

   if (ret_code) {
      secondary_ptr[0] = 2; /* indicate second test case */
      return(ret_code);
   } /* endif */

   /* get the results of the microcode side of this test */
   ret_code = receive_from_dsp(&dsp_return,secondary_ptr);
   if (ret_code) {
      secondary_ptr[0] = 2; /* indicate second test case */
      return(ret_code);
   } /* endif */
   if (dsp_return) {
      secondary_ptr[0] = 2; /* indicate second test case */
      return(dsp_return);
   } /* endif */

   /*******************************************************************/
   /* DMA TEST 3, From RIOS memory, To DSP SLAVE MODE, (Ref. BIM2D87) */
   /*******************************************************************/

   host_buffer = (char *)malloc(DMA_TEST_3_BUFFER_SIZE);

   if (!host_buffer) {
      return(HOST_BUFFER_MALLOC_ERROR);
   } /* endif */

   for (i = 0 ; i < DMA_TEST_3_BUFFER_SIZE ; i++) {
      host_buffer[i] = 0;
   } /* endfor */

   /* hand shake with microcode */
   ret_code = send_to_dsp(tu_number,secondary_ptr);

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 3; /* indicate third test case */
      return(ret_code);
   } /* endif */

   length = 0x76;
   width = 0xB;
   stride = 0x400;
   host_address = host_buffer;
   host_offset = 0x4;

   /* initialize the buffer to be sent */
   k = 0;
   j = 0;
   l = 0;
   for (i = 0 ; i < 0x80 ; i++) {
      *(host_address + host_offset + k++) = i;
      l += 1;
      if (!(l%12)) {
         j += stride;
         k = j;
      } /* endif */
   } /* endfor */

   ret_code =  do_dma( DMA_WRITE, host_address, length, width, stride,
                       host_offset, DMA_TEST_3_BUFFER_SIZE );

   free(host_buffer);

   if (ret_code) {
      secondary_ptr[0] = 3; /* indicate third test case */
      return(ret_code);
   } /* endif */

   /* get the results of the microcode side of this test */
   ret_code = receive_from_dsp(&dsp_return,secondary_ptr);
   if (ret_code) {
      secondary_ptr[0] = 3; /* indicate third test case */
      return(ret_code);
   } /* endif */
   if (dsp_return) {
      secondary_ptr[0] = 3; /* indicate third test case */
      return(dsp_return);
   } /* endif */

   /*******************************************************************/
   /* DMA TEST 4, From RIOS memory, To DSP SLAVE MODE, (Ref. BIM2D84) */
   /*******************************************************************/

   host_buffer = (char *)malloc(DMA_TEST_4_BUFFER_SIZE);

   if (!host_buffer) {
      return(HOST_BUFFER_MALLOC_ERROR);
   } /* endif */

   for (i = 0 ; i < DMA_TEST_4_BUFFER_SIZE ; i++) {
      host_buffer[i] = 0;
   } /* endfor */

   /* hand shake with microcode */
   ret_code = send_to_dsp(tu_number,secondary_ptr);

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 4; /* indicate fourth test case */
      return(ret_code);
   } /* endif */

   length = 0x20;
   width = 0x1;
   stride = 0x1;
   host_address = host_buffer;
   host_offset = 0x4;

   /* initialize the buffer to be sent */
   for (i = 0 ; i < 0x20 ; i++) {
      *(host_address + host_offset + i) = i;
   } /* endfor */

   ret_code =  do_dma( DMA_WRITE, host_address, length, width, stride,
                       host_offset, DMA_TEST_4_BUFFER_SIZE );

   free(host_buffer);

   if (ret_code) {
      secondary_ptr[0] = 4; /* indicate fourth test case */
      return(ret_code);
   } /* endif */

   /* get the results of the microcode side of this test */
   ret_code = receive_from_dsp(&dsp_return,secondary_ptr);
   if (ret_code) {
      secondary_ptr[0] = 4; /* indicate fourth test case */
      return(ret_code);
   } /* endif */
   if (dsp_return) {
      secondary_ptr[0] = 4; /* indicate fourth test case */
      return(dsp_return);
   } /* endif */

   /*******************************************************************/
   /* DMA TEST 5, From DSP I/O RAM, To RIOS MEMORY,    (Ref. BIM2D49) */
   /*******************************************************************/


   host_buffer = (char *)malloc(DMA_TEST_5_BUFFER_SIZE);

   if (!host_buffer) {
      return(HOST_BUFFER_MALLOC_ERROR);
   } /* endif */

   for (i = 0 ; i < DMA_TEST_5_BUFFER_SIZE ; i++) {
      host_buffer[i] = 0xFF;
   } /* endfor */

   /* hand shake with microcode */
   ret_code = send_to_dsp(tu_number,secondary_ptr);

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 5; /* indicate fifth test case */
      return(ret_code);
   } /* endif */

   length = 0x40;
   width = 0x2;
   stride = 0x200;
   host_address = host_buffer;
   host_offset = 0x0;

   ret_code =  do_dma( DMA_READ, host_address, length, width, stride,
                       host_offset, DMA_TEST_5_BUFFER_SIZE );

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 5; /* indicate fifth test case */
      return(ret_code);
   } /* endif */

   /* check the DMA pattern received */
   error = 0;
   k = 0;
   expected = 0x0001FFFF;
   increment = 0x04040000;
   for (i = 0 ; (i < 32) && !error  ; i++) {
      actual = (host_buffer[k] << 24) | (host_buffer[k+1] << 16) |
               (host_buffer[k+2] << 8) | (host_buffer[k+3]) ;
      error = actual ^ expected;
      k += stride;
      expected += increment;
   } /* endfor */

   free(host_buffer);

   /* get the results of the microcode side of this test */
   ret_code = receive_from_dsp(&dsp_return,secondary_ptr);
   if (ret_code) {
      secondary_ptr[0] = 5; /* indicate fifth test case */
      return(ret_code);
   } /* endif */
   if (dsp_return) {
      secondary_ptr[0] = 5; /* indicate fifth test case */
      return(dsp_return);
   } /* endif */

   if (error) {
      secondary_ptr[0] = 5; /* indicate fifth test case */
      return(DMA_HOST_PATTERN_CHECK_ERROR);
   } /* endif */


   /*******************************************************************/
   /* DMA TEST 6, From DSP I/O RAM, To RIOS MEMORY,    (Ref. BIM2D48) */
   /*******************************************************************/

   host_buffer = (char *)malloc(DMA_TEST_6_BUFFER_SIZE);

   if (!host_buffer) {
      return(HOST_BUFFER_MALLOC_ERROR);
   } /* endif */

   k = 0;
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0xAA;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0xFF;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0xBB;
   } /* endfor */

   /* hand shake with microcode */
   ret_code = send_to_dsp(tu_number,secondary_ptr);

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 6; /* indicate sixth test case */
      return(ret_code);
   } /* endif */

   length = 0x1;
   width = 0x1;
   stride = 0x0;
   host_address = host_buffer;
   host_offset = 0x7;

   ret_code =  do_dma( DMA_READ, host_address, length, width, stride,
                       host_offset, DMA_TEST_6_BUFFER_SIZE );

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 6; /* indicate sixth test case */
      return(ret_code);
   } /* endif */

   /* check the DMA pattern received */
   error = 0;
   k = 0;
   for (i = 0 ; (i < 4) && !error  ; i++) {
      error = host_buffer[k++] != 0xAA;
   } /* endfor */
   for (i = 0 ; (i < 3) && !error  ; i++) {
      error = host_buffer[k++] != 0xFF;
   } /* endfor */
   error = host_buffer[k++] != 0x03;
   for (i = 0 ; (i < 4) && !error  ; i++) {
      error = host_buffer[k++] != 0xBB;
   } /* endfor */

   free(host_buffer);

   /* get the results of the microcode side of this test */
   ret_code = receive_from_dsp(&dsp_return,secondary_ptr);
   if (ret_code) {
      secondary_ptr[0] = 6; /* indicate sixth test case */
      return(ret_code);
   } /* endif */
   if (dsp_return) {
      secondary_ptr[0] = 6; /* indicate sixth test case */
      return(dsp_return);
   } /* endif */

   if (error) {
      secondary_ptr[0] = 6; /* indicate sixth test case */
      return(DMA_HOST_PATTERN_CHECK_ERROR);
   } /* endif */

   /*******************************************************************/
   /* DMA TEST 8, From DSP I/O RAM, To RIOS MEMORY,    (Ref. BIM2D41) */
   /*******************************************************************/

   host_buffer = (char *)malloc(DMA_TEST_8_BUFFER_SIZE);

   if (!host_buffer) {
      return(HOST_BUFFER_MALLOC_ERROR);
   } /* endif */

   k = 0;
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0xFF;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0x11;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0x22;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0x33;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0x44;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0x55;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0x66;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0x77;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0x88;
   } /* endfor */

   /* hand shake with microcode */
   ret_code = send_to_dsp(tu_number,secondary_ptr);

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 8; /* indicate eighth test case */
      return(ret_code);
   } /* endif */

   length = 0x9;
   width = 0x1;
   stride = 0x4;
   host_address = host_buffer;
   host_offset = 0x3;

   ret_code =  do_dma( DMA_READ, host_address, length, width, stride,
                       host_offset, DMA_TEST_8_BUFFER_SIZE );

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 8; /* indicate eighth test case */
      return(ret_code);
   } /* endif */

   /* check the DMA pattern received */
   error = 0;
   k = 0;
   expected = 0xFFFFFF03;
   increment = 0x11111104;
   for (i = 0 ; (i < 9) && !error  ; i++) {
      actual = (host_buffer[k] << 24) | (host_buffer[k+1] << 16) |
               (host_buffer[k+2] << 8) | (host_buffer[k+3]) ;
      error = actual ^ expected;
      k += stride;
      if (i == 0) {
         expected = 0x11111107;
      } else {
         expected += increment;
      } /* endif */
   } /* endfor */

   free(host_buffer);

   /* get the results of the microcode side of this test */
   ret_code = receive_from_dsp(&dsp_return,secondary_ptr);
   if (ret_code) {
      secondary_ptr[0] = 8; /* indicate eighth test case */
      return(ret_code);
   } /* endif */
   if (dsp_return) {
      secondary_ptr[0] = 8; /* indicate eighth test case */
      return(dsp_return);
   } /* endif */

   if (error) {
      secondary_ptr[0] = 8; /* indicate eighth test case */
      return(DMA_HOST_PATTERN_CHECK_ERROR);
   } /* endif */

   /*******************************************************************/
   /* DMA TEST 9, From DSP I/O RAM, To RIOS MEMORY,    (Ref. BIM2D42) */
   /*******************************************************************/

   host_buffer = (char *)malloc(DMA_TEST_9_BUFFER_SIZE);

   if (!host_buffer) {
      return(HOST_BUFFER_MALLOC_ERROR);
   } /* endif */

   k = 0;
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0xFF;
   } /* endfor */
   pattern = 0x11;
   for (i = 0 ; i < 15 ; i++) {
      for (j = 0 ; j < 4 ; j++) {
         host_buffer[k++] = pattern;
      } /* endfor */
      pattern += 0x11;
   } /* endfor */
   for (i = 0 ; i < 4 ; i++) {
      host_buffer[k++] = 0x10;
   } /* endfor */

   /* hand shake with microcode */
   ret_code = send_to_dsp(tu_number,secondary_ptr);

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 9; /* indicate ninth test case */
      return(ret_code);
   } /* endif */

   length = 0x9;
   width = 0x1;
   stride = 0x8;
   host_address = host_buffer;
   host_offset = 0x3;

   ret_code =  do_dma( DMA_READ, host_address, length, width, stride,
                       host_offset, DMA_TEST_9_BUFFER_SIZE );

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 9; /* indicate ninth test case */
      return(ret_code);
   } /* endif */

   /* check the DMA pattern received */
   error = 0;
   k = 0;
   expected = 0xFFFFFF03;
   increment = 0x22222204;
   for (i = 0 ; (i < 8) && !error  ; i++) {
      actual = (host_buffer[k] << 24) | (host_buffer[k+1] << 16) |
               (host_buffer[k+2] << 8) | (host_buffer[k+3]) ;
      error = actual ^ expected;
      k += stride;
      if (i == 0) {
         expected = 0x22222207;
      } else {
         expected += increment;
      } /* endif */
   } /* endfor */
   if (!error) {
      actual = (host_buffer[k] << 24) | (host_buffer[k+1] << 16) |
               (host_buffer[k+2] << 8) | (host_buffer[k+3]) ;
      expected = 0x10101023;
      error = actual ^ expected;
   } /* endif */
   k = 4;
   expected = 0x11111111;
   increment = 0x22222222;
   for (i = 0 ; (i < 8) && !error  ; i++) {
      actual = (host_buffer[k] << 24) | (host_buffer[k+1] << 16) |
               (host_buffer[k+2] << 8) | (host_buffer[k+3]) ;
      error = actual ^ expected;
      k += stride;
      expected += increment;
   } /* endfor */

   free(host_buffer);

   /* get the results of the microcode side of this test */
   ret_code = receive_from_dsp(&dsp_return,secondary_ptr);
   if (ret_code) {
      secondary_ptr[0] = 9; /* indicate ninth test case */
      return(ret_code);
   } /* endif */
   if (dsp_return) {
      secondary_ptr[0] = 9; /* indicate ninth test case */
      return(dsp_return);
   } /* endif */

   if (error) {
      secondary_ptr[0] = 9; /* indicate ninth test case */
      return(DMA_HOST_PATTERN_CHECK_ERROR);
   } /* endif */


   /***********************************************************************/
   /* DMA TEST 10, From DSP SLAVE MODE, To RIOS MEMORY,    (Ref. BIM2D86) */
   /***********************************************************************/

   host_buffer = (char *)malloc(DMA_TEST_10_BUFFER_SIZE);

   if (!host_buffer) {
      return(HOST_BUFFER_MALLOC_ERROR);
   } /* endif */

   k = 0;
   l = k;
   for (i = 0 ; i < 11 ; i++) {
      for (j = 0 ; j < 12 ; j++) {
         host_buffer[l++] = 0xFF;
      } /* endfor */
      k += 0x400;  /* stride */
      l = k;
   } /* endfor */

   /* hand shake with microcode */
   ret_code = send_to_dsp(tu_number,secondary_ptr);

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 10; /* indicate tenth test case */
      return(ret_code);
   } /* endif */

   length = 0x76;
   width = 0xB;
   stride = 0x400;
   host_address = host_buffer;
   host_offset = 0x0;

   ret_code =  do_dma( DMA_READ, host_address, length, width, stride,
                       host_offset, DMA_TEST_10_BUFFER_SIZE );

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 10; /* indicate tenth test case */
      return(ret_code);
   } /* endif */

   /* check the DMA pattern received */
   error = 0;
   k = 0;
   l = k;
   expected = 0x00;
   increment = 0x01;
   for (i = 0 ; (i < 10) && !error  ; i++) {
      for (j = 0 ; (j < 11) && !error ; j++) {
         actual = host_buffer[l++];
         error = actual ^ expected;
         expected += increment;
      } /* endfor */
      actual = host_buffer[l++];
      if (!error) {
         error = actual ^ 0xFF;
      } /* endif */
      k += stride;
      l = k;
      expected += increment;
   } /* endfor */
   for (i = 0 ; (i < 8) && !error ; i++ ) {
      actual = host_buffer[l++];
      error = actual ^ expected;
      expected += increment;
   } /* endfor */
   actual = host_buffer[l++];
   if (!error) {
      error = actual ^ 0xFF;
   } /* endif */

   free(host_buffer);

   /* get the results of the microcode side of this test */
   ret_code = receive_from_dsp(&dsp_return,secondary_ptr);
   if (ret_code) {
      secondary_ptr[0] = 10; /* indicate tenth test case */
      return(ret_code);
   } /* endif */
   if (dsp_return) {
      secondary_ptr[0] = 10; /* indicate tenth test case */
      return(dsp_return);
   } /* endif */

   if (error) {
      secondary_ptr[0] = 10; /* indicate tenth test case */
      return(DMA_HOST_PATTERN_CHECK_ERROR);
   } /* endif */

   /***********************************************************************/
   /* DMA TEST 11, From DSP SLAVE MODE, To RIOS MEMORY,    (Ref. BIM2D60) */
   /***********************************************************************/

   host_buffer = (char *)malloc(DMA_TEST_11_BUFFER_SIZE);

   if (!host_buffer) {
      return(HOST_BUFFER_MALLOC_ERROR);
   } /* endif */

   k = 0;

   for (i = 0 ; i < DMA_TEST_11_BUFFER_SIZE ; i++) {
      host_buffer[i] = 0xFF;
   } /* endfor */

   /* hand shake with microcode */
   ret_code = send_to_dsp(tu_number,secondary_ptr);

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 11; /* indicate eleventh test case */
      return(ret_code);
   } /* endif */

   length = 0x80;
   width = 0x1;
   stride = 0x5;
   host_address = host_buffer;
   host_offset = 0x0;

   ret_code =  do_dma( DMA_READ, host_address, length, width, stride,
                       host_offset, DMA_TEST_11_BUFFER_SIZE );

   if (ret_code) {
      free(host_buffer);
      secondary_ptr[0] = 11; /* indicate eleventh test case */
      return(ret_code);
   } /* endif */

   /* check the DMA pattern received */
   error = 0;
   k = 0;
   expected = 0x00;
   increment = 0x01;
   for (i = 0 ; (i < 32) && !error  ; i++) {
      for (j = 0 ; (j < 16) && !error ; j++) {
         expected &= 0xFF;
         actual = host_buffer[k++];
         error = actual ^ ( ((k - 1)%5) ? 0xFF : expected );
         expected += increment;
      } /* endfor */
      for (j = 0 ; (j < 4) && !error ; j++) {
         actual = host_buffer[k++];
         error = actual ^ 0xFF;
      } /* endfor */
   } /* endfor */

   free(host_buffer);

   /* get the results of the microcode side of this test */
   ret_code = receive_from_dsp(&dsp_return,secondary_ptr);
   if (ret_code) {
      secondary_ptr[0] = 11; /* indicate eleventh test case */
      return(ret_code);
   } /* endif */
   if (dsp_return) {
      secondary_ptr[0] = 11; /* indicate eleventh test case */
      return(dsp_return);
   } /* endif */

   if (error) {
      secondary_ptr[0] = 11; /* indicate eleventh test case */
      return(DMA_HOST_PATTERN_CHECK_ERROR);
   } /* endif */

   return(0);

} /* end bim_dma_test() */


/*
 * NAME: do_dma()
 *
 * FUNCTION: This function sets the up the appropriate data structures and then
 *           calls the device driver to initiate a DMA. The device driver is
 *           passed a DMA Structure Element (SE) from this function, which then
 *           will be written into the Priority Command Buffer (PCB) of the BIM.
 *
 *           The SE contains a field called host_address, which is first
 *           set by this function to the address of the allocated memory for
 *           the DMA buffer. The device driver will pin that buffer in physical
 *           memory and then change that field of the SE to the real address of
 *           the pinned buffer in memory. It will then send the SE to the PCB.
 *           Then the device driver will wait for DMA COMPLETE interrupt. That
 *           interrupt is defined as a write to the DSP COMMO register by the
 *           micro code on the adapter with the special value 0x0006RRRR,
 *           where RRRR is a special correlator supplied by the device driver.
 *
 *           For more, refer to the DMA SE definition in the file "host_dsp.h".
 *
 * EXECUTION ENVIRONMENT:  AIX during LEGA diagnostics
 *
 * NOTES: None
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This test will return zero if successful, else it will return
 *          a non-zero value.
 */
/*
 * INPUT:   rw_flag     - READ or WRITE Device Driver flag.
 *          host_addr   - Host DMA BUFFER address.
 *          length      - length of DMA transfer in bytes.
 *          width       - width factor for DMA.
 *          stride      - stride factor for DMA.
 *          offset      - Host address offset used to test odd boundaries.
 *          sw_length   - Pinned Buffer (sub-window) length.
 *    For further notes on the above parameters, refer to the WOLF spec.
 *    by David Peterson.
 *
 * OUTPUT: return code.
 */

int do_dma(int rw_flag, char *host_addr, int length, int width, int stride,
           int offset, int sw_length)
{
   struct _gscdma gsc_dma;   /* device independent dma structure */
   mid_dma_t      middma;    /* device dependent dma structure */
   DMA_SE_TYPE    dma_se;  /* DMA Structure Element to be sent to PCB */
   volatile unsigned long *host_int_mask, *host_int_status; /* was int */
   int ret_code = 0;

   host_int_mask = (HOST_INTR_MASK + bim_base_addr);
   host_int_status = (HOST_STATUS + bim_base_addr);

   /* SETUP the DMA SE to be sent to the Priority Command Buffer in BIM */
   /* by the device driver.                                             */
   dma_se.length_opcode = (DMA_SE_LENGTH << 16) | DMA_SE_OPCODE ;
   dma_se.corr_flags = 0;
   dma_se.host_address = (int)(host_addr);
   dma_se.dma_length = length;
   dma_se.width = width;
   dma_se.stride = stride;
   dma_se.host_offset = offset;

   /* Set up Device Dependant structure for DMA transfer */
   middma.flags = MID_DMA_DIAGNOSTICS;
   middma.se_size = (unsigned int)DMA_SE_LENGTH;  /* size of the SE */
   middma.se_data = (void *)&dma_se;

   /* Set up Device Independent structure for DMA transfer */
   gsc_dma.num_sw = 1;
   gsc_dma.subwindow[0].sw_addr = (char*)(host_addr);
   gsc_dma.subwindow[0].sw_length = sw_length;
   gsc_dma.subwindow[0].sw_flag = 0;
   gsc_dma.cmd_length = sizeof(mid_dma_t);
   gsc_dma.dma_cmd = (char *)&middma;
   gsc_dma.flags = rw_flag | DMA_WAIT | DMA_DIAGNOSTICS;

   /* Clear interrupts on BIM */
   *host_int_status = 0xFFFF;

   /* Enable DSP Commo Write Interrupt so that the microcode can send   */
   /* the DMA COMPLETE interrupt to the Device Driver. The DMA COMPLETE */
   /* interrupt is sent by writing 0x0006RRRR to the dsp commo register */
   /* where RRRR is the correlator (upper 16 bits of the corr_flags     */
   /* field of the DMA SE) supplied by the device driver.               */

   *host_int_mask |= WRITE_TO_DSP_COMMO;

   /* Call the Device Driver to initiate the DMA transfer. The device   */
   /* driver will not return until it receives a DMA COMPLETE interrupt */
   /* from the microcode.                                               */

   ret_code = aixgsc( our_gsc_handle.handle, DMA_SERVICE, &gsc_dma);
   if (ret_code) {
      return(AIXGSC_DMA_SERVICE_ERROR);
   } /* endif */

   /* Disable DSP Commo Write Interrupt */
   *host_int_mask = 0;

   ret_code = set_mc_speed(300);

   return(ret_code);

} /* end do_dma() */

/* NAME: bim_get_hot()
 *
 * FUNCTION: This function will execute on the host and will communicate
 *           with its sister function on the adapter. It will continuosly
 *           do DMA transfers in order to make the BIM hot.
 *
 * EXECUTION ENVIRONMENT: RIOS
 *
 * NOTES: None
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This test will return zero if successful, else it will return
 *          non-zero.
 */
/*
 * INPUT: None.
 *
 * OUTPUT: zero if no errors, else returns error code.
 */

int bim_get_hot()
{
   unsigned int  ret_code, size, count, i;
   char *host_buf;

   ret_code = 0;
   count = 0;     /* number of DMA's performed */
   size = 0x100;

   host_buf = (char *)malloc( size );

   if (!host_buf) {
      printf("\nERROR ALLOCATING MEMORY FOR host_buf\n");
      return(1);
   } /* endif */

   while (!ret_code) {        /* infinite loop */
      count++;
      ret_code =  do_dma( DMA_READ, host_buf, size, size, 0, 0, size );
      if (!(count % 100)) {
         printf("\n Number of successful DMA transfers : %d",count);
      } /* endif */
      if ((count == 1) || !(count % 1000000)) {
         printf("\n");
         for (i = 1 ; i <= size ; i++) {
            printf(" %2.2x ",host_buf[i-1]);
            if (!(i % 8)) {
               printf("\n");
            } /* endif */
         } /* endfor */
      } /* endif */
   } /* endwhile */

   if (ret_code) {
       printf("\nDMA error !!!\n");
       printf("\n Number of successful DMA transfers : %d",--count);
   } /* endif */

   free(host_buf);

   return(ret_code);

} /* end bim_get_hot() */
