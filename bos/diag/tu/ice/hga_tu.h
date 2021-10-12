/* @(#)86	1.1  src/htx/usr/lpp/htx/lib/hga/hga_tu.h, tu_hga, htx410 6/2/94 11:37:23  */
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _HGA_TU_H
#define _HGA_TU_H

#include <sys/types.h>
#include <sys/mdio.h>


/* Test Unit number definitions follow */

/* Core Test Units */
#define QPAX_REGISTER_TEST      1       /* QPAX Registers Test */
#define FEATURE_ROM_TEST        2       /* ROM/VPD Test */
#define RAMDAC_BASIC_TEST       3       /* BLUELAKE BASIC Test */
#define VRAM_TEST               4       /* VRAM Test */
#define DFA_TEST                5       /* DFA Test */
#define INTERRUPT_TEST          6       /* Interrupt Test */
#define DMA_TEST                7       /* DMA Test */
#define BLIT_TEST               8       /* BLIT Test */
#define AREA_FILL_TEST          9       /* AREA Fill Test */
#define LINES_DRAW_TEST        10       /* LINES DRAW Test */
#define WINDOWING_TEST         11       /* Clipping, WIDs, Overlays Test */
#define SWIZZLE_TEST           12       /* Swizzle Test */
#define CURSOR_TEST            13       /* Cursor Test */
#define RAMDAC_OUTPUT_TEST     14       /* Bluelake Output Level Test */
#define VMUX_BASIC_TEST        15       /* Blue Doobie Register/RAM Test */
#define VMUX_FUNCTIONAL_TEST   16       /* Blue Doobie Functional Test */

/* Display Monitor Service Aids */
#define COLOR_BAR_SCREEN       30       /* Color bar Screen Service Aid */
#define RED_SCREEN             31       /* Red Screen Service Aid */
#define GREEN_SCREEN           32       /* Green Screen Service Aid */
#define BLUE_SCREEN            33       /* Blue Screen Service Aid */
#define WHITE_SCREEN           34       /* White Screen Service Aid */
#define BLACK_SCREEN           35       /* Black Screen Service Aid */
#define CROSS_HATCH_9x7        36       /* 9x7 Cross Hatch Service Aid */
#define CROSS_HATCH_11x9       37       /* 11x9 Cross Hatch Service Aid */
#define BB_50MM_BOX            38       /* 50mm Box Service Aid */
#define FOCUS_SCREEN           39       /* Focus Screen Service Aid */

/* Stress Utilities and Debug Aids */
#define SCROLLING_H_TEST       40       /* Scrolling H Test for EMI stress */
#define THERMAL_STRESS         41       /* Utility for Thermal stress test */
#define VRAM_DEBUG_TOOL        42       /* Debug Aid for VRAMs */
#define CARD_CONFIG_LEVEL      43       /* Determines adapter configuration */

#define GM_TEST	               44	/* Test S3 graphics modes */

/* Device access method OPEN and CLOSE utilities */
#define TU_OPEN                90
#define TU_CLOSE               91


/* Definitions for BUS types supported */
#define BBLUE_6XX 1      /* attached to 6xx bus */
#define BBLUE_MC  2      /* attached to microchannel bus */
#define BBLUE_PCI 3      /* attached to PCI bus */


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
      unsigned long int miscompare_address;
      unsigned long int expected_data;
      unsigned long int actual_data;
      unsigned long int compare_mask;
   } REGISTER_ERROR_DETAILS, DFA_ERROR_DETAILS;

   typedef struct {
     ulong error_code;
     ulong miscompare_address;
     ulong expected_data;
     ulong actual_data;
     ulong compare_mask;
     ushort misr_usage; /* # times MISR was used */
     uchar nonmasked;   /* # MISR errors - non-masked screen frames */
     uchar pix_00_07;   /* # MISR errors - pixel VRAM, data lines 00-07 */
     uchar pix_08_15;   /* # MISR errors - pixel VRAM, data lines 08-15 */
     uchar pix_16_23;   /* # MISR errors - pixel VRAM, data lines 16-23 */
     uchar pix_24_31;   /* # MISR errors - pixel VRAM, data lines 24-31 */
     uchar pix_32_39;   /* # MISR errors - pixel VRAM, data lines 32-39 */
     uchar pix_40_47;   /* # MISR errors - pixel VRAM, data lines 40-47 */
     uchar pix_48_55;   /* # MISR errors - pixel VRAM, data lines 48-55 */
     uchar pix_56_63;   /* # MISR errors - pixel VRAM, data lines 56-63 */
     uchar wid_00_07;   /* # MISR errors - WID VRAM, data lines 00-07 */
     uchar wid_08_15;   /* # MISR errors - WID VRAM, data lines 08-15 */
     uchar wid_16_23;   /* # MISR errors - WID VRAM, data lines 16-23 */
     uchar wid_24_31;   /* # MISR errors - WID VRAM, data lines 24-31 */
   } VRAM_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int crc_expected;
      unsigned long int crc_actual;
   } FEATURE_ROM_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int interrupts_expected;
      unsigned long int interrupts_received;
   } INTERRUPT_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int miscompare_address;
      unsigned long int expected_data;
      unsigned long int actual_data;
      unsigned long int buffer_address;
   } DMA_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int monitor_id_value;
   } MONITOR_ID_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      int current_mode;
      int requested_mode;
   } DEVICE_MODE_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      int diagex_error_code;
      int system_error_code;
   } DIAGEX_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      int ioctl_type;
      int mdd_error_code;
      int errno;
      MACH_DD_IO parameters;
   } MDD_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      int errno;
   } SYSTEM_CALL_ERROR_DETAILS;

   typedef struct {
     ulong error_code;             /* error code used */
     char hga_qpax_rev_level;    /* QPAX revision level on this adapter */
     char qpax_rev_level_latest;   /* latest known QPAX revision level */
     char hga_bl_rev_level;      /* Bluelake revision level on this adapter */
     char bl_rev_level_latest;     /* latest known Bluelake revision level */
     char hga_bd_rev_level;      /* Blue Doobie rev. level on this adapter */
     char bd_rev_level_latest;     /* latest known Blue Doobie revision level */
     short monitor_id;             /* monitor ID info read from adapter */
     short monitor_index;          /* monitor index info for this adapter */
     short hga_configured_buid;  /* Bus unit ID */
     short hga_configured_xivr;  /* x interrupt vector register*/
     char hga_configured_address; /* Addr the card is configured to use */
     char hga_int_level;         /* Interrupt level */
     char hga_vram_mb_size;      /* number of MB of VRAM installed */
     char hga_bus_type;          /* Bus type of adapter */
     short hga_device_id;        /* Device ID */
           /* to compare against latest known chip revision levels: */
     char last_changed_date_m;  /* month of last update of hga_card_level() */
     char last_changed_date_d;  /* day of last update of hga_card_level() */
     char last_changed_date_y;  /* year of last update of hga_card_level() */
   } BBLUE_CARD_LEVEL_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int status;
      unsigned long int poll_mask;
      unsigned long int condition;
   } POLL_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int crc_expected;
      unsigned long int crc_actual;
      unsigned long int buffer;
   } CRC_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int case_number;
      unsigned long int expected_data;
      unsigned long int actual_data;
      unsigned long int compare_mask;
   } CASE_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int case_number;
      unsigned long int expected_data;
      unsigned long int misr1_data;
      unsigned long int misr2_data;
   } MISR2_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int miscompare_address;
      unsigned long int expected_data;
      unsigned long int actual_data;
      unsigned long int array_index;
   } SWIZZLE_ERROR_DETAILS;

   typedef struct {
      unsigned long int error_code;
      unsigned long int expected;
      unsigned long int actual;
   } DEVICE_ID_ERROR_DETAILS;

   typedef struct {
     unsigned long int error_code;
     unsigned long int case_number;
     unsigned long int expected_data;
     unsigned long int actual_data;
     unsigned long int compare_mask;
     ushort misr_usage; /* # times MISR was used */
     uchar nonmasked;   /* # MISR errors - non-masked screen frames */
     uchar pix_00_07;   /* # MISR errors - pixel VRAM, data lines 00-07 */
     uchar pix_08_15;   /* # MISR errors - pixel VRAM, data lines 08-15 */
     uchar pix_16_23;   /* # MISR errors - pixel VRAM, data lines 16-23 */
     uchar pix_24_31;   /* # MISR errors - pixel VRAM, data lines 24-31 */
     uchar pix_32_39;   /* # MISR errors - pixel VRAM, data lines 32-39 */
     uchar pix_40_47;   /* # MISR errors - pixel VRAM, data lines 40-47 */
     uchar pix_48_55;   /* # MISR errors - pixel VRAM, data lines 48-55 */
     uchar pix_56_63;   /* # MISR errors - pixel VRAM, data lines 56-63 */
     uchar wid_00_07;   /* # MISR errors - WID VRAM, data lines 00-07 */
     uchar wid_08_15;   /* # MISR errors - WID VRAM, data lines 08-15 */
     uchar wid_16_23;   /* # MISR errors - WID VRAM, data lines 16-23 */
     uchar wid_24_31;   /* # MISR errors - WID VRAM, data lines 24-31 */
   } MISR_CASE_ERROR_DETAILS;

   typedef union {
      unsigned long int         error_code;
      REGISTER_ERROR_DETAILS    register_test;
      FEATURE_ROM_ERROR_DETAILS feature_rom_test;
      VRAM_ERROR_DETAILS        vram_test;
      DFA_ERROR_DETAILS         dfa_test;
      INTERRUPT_ERROR_DETAILS   interrupt_test;
      DMA_ERROR_DETAILS         dma_test;
      MONITOR_ID_ERROR_DETAILS  monitor_id_read;
      BBLUE_CARD_LEVEL_DETAILS  card_level_info;
      DEVICE_MODE_ERROR_DETAILS device_mode;
      DIAGEX_ERROR_DETAILS      diagex_call;
      MDD_ERROR_DETAILS         mdd_ioctl;
      SYSTEM_CALL_ERROR_DETAILS system_call;
      POLL_ERROR_DETAILS        poll;
      CRC_ERROR_DETAILS         crc;
      CASE_ERROR_DETAILS        case_tests;
      MISR2_ERROR_DETAILS       misr2_tests;
      SWIZZLE_ERROR_DETAILS     swizzle;
      DEVICE_ID_ERROR_DETAILS   device_id;
      MISR_CASE_ERROR_DETAILS   misr_case_tests;
   } ERROR_DETAILS;



/*
 * TUCB / TU_TYPE member explanations.
 *
 *  tu - Input parameter; Selects a Test Unit to be executed.
 *
 *  loop - Input parameter; Specifies number of times to execute the TU.
 *         Zero (0) means none and returns immediately.
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

/* Test Unit Control Block (TUCB) definition */

   typedef struct tu_hga {
      unsigned      tu;              /* test unit number */
      unsigned long loop;            /* tu loop count */
      unsigned      error_log_count; /* number of errors (to be) logged */
      ERROR_DETAILS *error_log;      /* error details log */

#ifdef TU_DEBUG_MSG
      FILE *msg_file;                /* message file pointer */
#endif

   } TU_TYPE;


#endif 
