/* @(#)59       1.4  src/bos/diag/tu/sun/sun_tu_type.h, tu_sunrise, bos411, 9437A411a 8/29/94 15:50:39 */
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: none
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
/****************************************************************************
        File:   sun_tu_type.h
*****************************************************************************/

#include <sys/types.h>
#include <sys/mdio.h>


/* Test Unit number definitions follow */

/* Core Test Units */
#define POS_CHK_TEST            1
#define BASE_VPD_TEST           2
#define MIAMIREG_TEST           3
#define PIXELREG_TEST           4
#define VIDSETUP_TEST           5
#define FRAMEBUFFER_TEST        6
#define DMA_TEST                7
#define SCALER_TEST             8
#define CODEC_VPD_TEST          9
#define CODECREG_TEST           10
#define FIELDMEM_TEST           11
#define COMPRESS_TEST           12
#define DECOMPRESS_TEST         13
#define EMC_DMA_TEST            14

/* Device access method OPEN and CLOSE utilities */
#define TU_OPEN            90
#define TU_CLOSE           91

/*
 * ERROR_DETAILS structure and related definitions follow.
 *
 * These structures are used to provide detailed error information
 * for some of the errors that are detected by the test units.
 * Whether the detailed error is available for a particular TU and error
 * code is documented in the TU Component Interface Specification, and
 * the actual source files where that error code is defined.
 */

   typedef struct {
      unsigned long int error_code;
      int diagex_error_code;
      int system_error_code;
   } HWINIT_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int sub_error_code;
      unsigned long int crc_expected;
      unsigned long int crc_actual;
   } CRC_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int sub_error_code;
      unsigned long int expected_data;
      unsigned long int actual_data;
      unsigned long int address;
   } PX2070REG_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int io_dgx_rc;
      unsigned long int address;
      unsigned long int data;
   } IO_RW_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int sub_error_code;
      unsigned long int expected_data;
      unsigned long int actual_data;
      unsigned long int address;
   } REGISTER_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int sub_error_code;
      unsigned long int expected_data;
      unsigned long int actual_data;
      unsigned long int row;
      unsigned long int column;
   } FRAMEBUFFER_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int sub_error_code;
      unsigned long int expected_data;
      unsigned long int actual_data;
      unsigned long int buffer_address;
   } MEM_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int sub_error_code;
   } VIDSETUP_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int sub_error_code;
      unsigned long int crc_expected;
      unsigned long int crc_actual;
      unsigned long int cnt_expected;
      unsigned long int cnt_actual;
   } COMP_ERROR_DETAILS;

   typedef union {
      unsigned long int         error_code;
      REGISTER_ERROR_DETAILS    register_test;
      MEM_ERROR_DETAILS         mem_test;
      HWINIT_ERROR_DETAILS      hw_init;
      CRC_ERROR_DETAILS         crc;
      PX2070REG_ERROR_DETAILS   px2070;
      IO_RW_ERROR_DETAILS       io_readwrite;
      FRAMEBUFFER_ERROR_DETAILS fb_test;
      VIDSETUP_ERROR_DETAILS    vidsetup_test;
      COMP_ERROR_DETAILS        comp;
   } ERROR_DETAILS;

/* Test Unit Control Block (TUCB) definition */

   typedef struct tu_sun {
      unsigned      tu;              /* test unit number */
      unsigned long loop;            /* tu loop count */
      char          *device_name;    /* logical device name */
      unsigned      error_log_count; /* number of errors (to be) logged */
      ERROR_DETAILS *error_log;      /* error details log */
      int           codec_present;   /* CODEC daughter card exists flag */

#ifdef TU_DEBUG_MSG
      FILE *msg_file;                /* message file pointer */
#endif

   } TU_TYPE;

/*
 * TUCB / TU_TYPE member explanations.
 *
 *  tu - Input parameter; Selects a Test Unit to be executed.
 *
 *  loop - Input parameter; Specifies number of times to execute the TU.
 *         Zero (0) means none and returns immediately.
 *
 *  device_name - Input parameter; used only for DIAGEX access; if compile flag
 *                MOM_ACCESS is used, then this member can be left
 *                uninitialized, else it should be initialized to the logical
 *                device name for the instance of the Baby Blue adapter to be
 *                tested.
 *
 *  error_log_count - Both Input and Output parameter; As input it specifies
 *                    maximum number of errors desired to be logged;
 *                    As output it returns the actual number logged. This
 *                    should always be initialized by the calling application.
 *                    A value of Zero (0) results in no error being logged and
 *                    just a return code is returned.
 *
 *  error_log - Both Input and Output parameter; As input it points to an
 *              empty array of ERROR_DETAILS structures; As output it points
 *              to the same array which is filled with pertinent error details
 *              of all errors detected. This parameter should always be
 *              initialized by the calling application to point to a defined
 *              array.  The size of the defined array should be large
 *              enough to hold at least error_log_count ERROR_DETAILS.
 *
 *  msg_file - NOTE : THIS IS A CONDITIONALLY COMPILED MEMBER OF TUCB.
 *             Message file pointer which must be intialized by the
 *             calling application if the conditional compile variable
 *             TU_DEBUG_MSG is turned on during the compile.
 *             To have messages logged to a file, set this variable to the
 *             file pointer returned by the fopen() system call. To have
 *             messages displayed on the screen, set this variable to stdout.
 *
 */

ERROR_DETAILS error;            /* Structure used to load with error
                                   information and passed to the
                                   log_error() function in order to log an
                                   error. */
