/* @(#)24       1.4.1.3  src/bos/kernext/disp/ped/pedmacro/hw_trace.h, pedmacro, bos411, 9428A410j 3/23/94 17:07:34 */

/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS:
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include   <sys/trcctl.h>
#include   <sys/trcmacros.h>
#include   <sys/trchkid.h>

/*****************************************************************************/
/*                                                                           */
/*                PEDERNALES HW TRACE PROGRAMMING INTERFACE                  */
/*                                                                           */
/*****************************************************************************/


#ifndef _H_MID_HW_TRACE
#define _H_MID_HW_TRACE

      /********************************************************************
       ********************************************************************

	  These 6 define statements control the macro layer traces.
	  The comments which follow explain the 5 steps required for
	  using these defines to generate the desired trace output.
	  No other define statements in this file should be altered.

	  It may be more convenient for a user to define these 6 control
	  statements within their own personal include files.  This would
	  keep them from having to set up these control statements
	  each time they get a new copy of hw_trace.h.  Therefore, these
	  control statements are only used if the corresponding variables
	  have not been previously defined.

       ********************************************************************
       ********************************************************************/

#ifndef MID_TRACE

#define MID_TRACE                       0  /* 0 = Disable trace feature      */
	                                   /* 1 = Enable trace feature       */
#endif /* MID_TRACE */


#ifndef MID_TRACE_TIME_STAMP

#define MID_TRACE_TIME_STAMP            0  /* 0 = Do not time stamp trace    */
	                                   /* 1 = Time stamp trace           */
#endif /* MID_TRACE_TIME_STAMP */


#ifndef MID_TRACE_FILE_SYS_ENABLE

#define MID_TRACE_FILE_SYS_ENABLE       0  /* 0 = Disable system file trace  */
	                                   /* 1 = Enable system file trace   */
#endif /* MID_TRACE_FILE_SYS_ENABLE */


#ifndef MID_TRACE_SHOW_ASC_ENABLE

#define MID_TRACE_SHOW_ASC_ENABLE       0  /* 0 = Disable ASCII screen trace */
	                                   /* 1 = Enable ASCII screen trace  */
#endif /* MID_TRACE_SHOW_ASC_ENABLE */


#ifndef MID_TRACE_FILE_BIN_ENABLE

#define MID_TRACE_FILE_BIN_ENABLE       0  /* 0 = Disable binary file trace  */
	                                   /* 1 = Enable binary file trace   */
#endif /* MID_TRACE_FILE_BIN_ENABLE */


#ifndef MID_TRACE_FILE_ASC_ENABLE

#define MID_TRACE_FILE_ASC_ENABLE       0  /* 0 = Disable ASCII file trace   */
	                                   /* 1 = Enable ASCII file trace    */
#endif /* MID_TRACE_FILE_ASC_ENABLE */


      /*---------------------------------------------------------------------
	  1.  Set the value of MID_TRACE to 1 to enable the trace or
	      set the value of MID_TRACE to 0 to disable the trace.

	      If the trace is enabled, it may be turned off and on
	      dynamically with subroutine calls (see step 6), with the
	      default being that the trace is always on.  There is a small
	      performance hit if trace is enabled, even if it is turned
	      off with a subroutine call.

	      If the trace is disabled, it may not be turned on dynamically
	      with subroutine calls.  There is no performance hit if trace
	      is disabled.  If trace is disabled, you may skip the remaining
	      steps.
       *---------------------------------------------------------------------*/

      /*---------------------------------------------------------------------
	  2.  Set MID_TRACE_TIME_STAMP a 1 if you want all trace
	      entries to be time stamped, or set MID_TRACE_TIME_STAMP
	      to a 0 if you do not want any trace entries to be time
	      stamped.

	      Time stamping trace entries has slightly more overhead and
	      produces longer traces than not time stamping trace entries.
	      However, "ted" has the ability to use the time stamps to
	      approximate the timing of the original command sequence
	      when it "replays" traces to the adapter.
       *---------------------------------------------------------------------*/

      /*---------------------------------------------------------------------
	  3.  You now have a choice of enabling any combination of
	      four types of traces if this IS NOT a device driver
	      trace (MID_DD is not defined) or two types of traces if
	      this IS a device driver trace (MID_DD is defined):

	      a.  If MID_TRACE_SHOW_SYS_ENABLE is set to a 1, a system
	          trace is generated if trace has been enable either
	          from the command line or within the calling program.
	          A recommended command sequence to capture a system
	          trace is:

	               trace -a -j 518 -o mid_trace.sys
	               < run program to be traced >
	               trcoff

	          This command only traces the macro layer (hook = 518)
	          and directs the output to the file "mid_trace.sys".
	          The -a option allows the command to run asynchronously.
	          If MID_TRACE_SHOW_SYS_ENABLE is set to a 0, no system
	          trace will be generated.  This trace IS valid inside
	          the device driver (MID_DD is defined).

	      b.  If MID_TRACE_SHOW_ASC_ENABLE is set to a 1, the ASCII
	          screen trace is enabled, and the trace output will be
	          displayed on the screen.  If MID_TRACE_SHOW_ASC_ENABLE
	          is set to a 0, no ASCII screen trace will be displayed.
	          This trace IS valid inside the device driver
	          (MID_DD is defined).

	      c.  If MID_TRACE_FILE_ASC_ENABLE is set to a 1, the ASCII
	          file trace is enabled, and the trace output will be
	          directed to the file "mid_trace.asc".  This file is opened
	          automatically the first time the program generates trace
	          data for it.  If the file already exists, it will be
	          overwritten.  If MID_TRACE_SHOW_ASC_ENABLE is set to a 0,
	          no ASCII file trace will be generated, and any existing
	          mid_trace.asc file will not be altered.  This trace has
	          the same format as the ASCII screen trace.  This trace
	          IS NOT valid inside the device driver (MID_DD is defined).

	      d.  If MID_TRACE_FILE_BIN_ENABLE is set to a 1, the binary
	          file trace is enabled, and the trace output will be
	          directed to the file "mid_trace.bin".  This file is opened
	          automatically the first time the program generates trace
	          data for it.  If the file already exists, it will be
	          overwritten.  If MID_TRACE_SHOW_BIN_ENABLE is set to a 0,
	          no binary file trace will be generated, and any existing
	          mid_trace.bin file will not be altered.  This trace produces
	          a binary file which is much smaller than the ASCII file trace
	          but it cannot be viewed with a text editor.  This trace
	          IS NOT valid inside the device driver (MID_DD is defined).

	      All three types of file traces provide some nice debug
	      capabilities since they may be "replayed" to the adapter
	      by the program "ted".  Consequently, trace files generated on
	      machines without emulators may be replayed on machines which
	      have emulators.  Also, "ted" provides the capability to single
	      step, and to convert file traces from one type to another.
	      In addition, traces containing time stamps may be replayed
	      approximating the original timing.

	---------------------------------------------------------------------*/


      /*---------------------------------------------------------------------
	  4.  The sytem trace does not require any additional object modules.
	      If you have enabled any of the other traces, get the following
	      files from the library and link the ".c" files with your program:

	      a.  hw_trace.c     (generates trace data)
	      b.  hw_opname.c    (interprets opcodes)
	      c.  hw_names.h     (define names for BIM regs)
	      d.  hw_addrs.h     (define addrs for BIM regs)
	      e.  hw_seops.h     (define SE opcodes)
	      f.  hw_trace.h     (configure trace)
	      g.  hw_timer.h     (timing routines)
       *---------------------------------------------------------------------*/


      /*---------------------------------------------------------------------
	  5.  Except for System Trace which may be enabled or disabled from
	      the command line, all of the enabled traces are automatically
	      "on" when your program is loaded.  You may turn the enabled
	      traces off and on dynamically by inserting the corresponding
	      subroutine calls in your program:

	      a. mid_trace_file_sys_on();    # Turn system trace on
	      b. mid_trace_file_sys_off();   # Turn system trace off
	      c. mid_trace_show_asc_on();    # Turn ASCII screen trace on
	      d. mid_trace_show_asc_off();   # Turn ASCII screen trace off
	      e. mid_trace_file_asc_on();    # Turn ASCII file trace on
	      f. mid_trace_file_asc_off();   # Turn ASCII file trace off
	      g. mid_trace_file_bin_on();    # Turn binary file trace on
	      h. mid_trace_file_bin_off();   # Turn binary file trace off
       ----------------------------------------------------------------------*/


      /********************************************************************
       ********************************************************************

	  The following defines are used by the trace routines.  Do not
	  make changes beyond this point (under penalty of severe ridicule).

       ********************************************************************
       ********************************************************************/


/* Define Macro for converting host addresses to bim addresses */
#define _MID_BIM_ADR( host_adr )        ((int)(host_adr) & 0x0000FFFF)

#if     MID_TRACE

#ifdef  MID_DD

#define MID_TRACE_FILE_BIN_SWITCH  0
#define MID_TRACE_FILE_ASC_SWITCH  0
#define MID_TRACE_SHOW_ASC_SWITCH  MID_TRACE_SHOW_ASC_ENABLE
#define MID_TRACE_FILE_SYS_SWITCH  MID_TRACE_FILE_SYS_ENABLE

#else   /* not MID_DD */

#define MID_TRACE_FILE_BIN_SWITCH  MID_TRACE_FILE_BIN_ENABLE
#define MID_TRACE_FILE_ASC_SWITCH  MID_TRACE_FILE_ASC_ENABLE
#define MID_TRACE_SHOW_ASC_SWITCH  MID_TRACE_SHOW_ASC_ENABLE
#define MID_TRACE_FILE_SYS_SWITCH  MID_TRACE_FILE_SYS_ENABLE

#endif  /* not MID_DD */

#else /*  MID_TRACE == 0  */

#define MID_TRACE_FILE_BIN_SWITCH  0
#define MID_TRACE_FILE_ASC_SWITCH  0
#define MID_TRACE_SHOW_ASC_SWITCH  0
#define MID_TRACE_FILE_SYS_SWITCH  0

#endif  /* MID_TRACE == 0 */

#define MID_TRACE_SYS          MID_TRACE_FILE_SYS_SWITCH

#define MID_TRACE_SUB          MID_TRACE_FILE_BIN_SWITCH+  \
	                       MID_TRACE_FILE_ASC_SWITCH+  \
	                       MID_TRACE_SHOW_ASC_SWITCH

#if     MID_TRACE_FILE_BIN_SWITCH
#define MID_TRACE_FILE_BIN      1
#else /* MID_TRACE_FILE_BIN_SWITCH == 0 */
#define MID_TRACE_FILE_BIN      0
#endif /* MID_TRACE_FILE_BIN_SWITCH == 0 */

#if     MID_TRACE_FILE_ASC_SWITCH
#define MID_TRACE_FILE_ASC      2
#else /* MID_TRACE_FILE_ASC_SWITCH == 0 */
#define MID_TRACE_FILE_ASC      0
#endif /* MID_TRACE_FILE_ASC_SWITCH == 0 */

#if     MID_TRACE_SHOW_ASC_SWITCH
#define MID_TRACE_SHOW_ASC      4
#else /* MID_TRACE_SHOW_ASC_SWITCH == 0 */
#define MID_TRACE_SHOW_ASC      0
#endif /* MID_TRACE_SHOW_ASC_SWITCH == 0 */

#if     MID_TRACE_FILE_SYS_SWITCH
#define MID_TRACE_FILE_SYS      8
#else /* MID_TRACE_FILE_SYS_SWITCH == 0 */
#define MID_TRACE_FILE_SYS      0
#endif /* MID_TRACE_FILE_SYS_SWITCH == 0 */

#if     MID_TRACE_TIME_STAMP
#define MID_TRACE_TIME_MASK    0x00080000
#else /* MID_TRACE_TIME_STAMP == 0 */
#define MID_TRACE_TIME_MASK    0x00000000
#endif /* MID_TRACE_TIME_STAMP == 0 */

#define MID_TRACE_HOOK_ID_MASK         0xFFF00000
#define MID_TRACE_HOOK_TIME_MASK       0x00080000
#define MID_TRACE_HOOK_TYPE_MASK       0x00070000
#define MID_TRACE_HOOK_BUFLEN_MASK     0x0000FFFC
#define MID_TRACE_HOOK_HOOKLEN_MASK    0x00000007

#define MID_TRACE_PARM_IND_MASK        0xFFFF0000
#define MID_TRACE_PARM_IND_OP_MASK     0xF0000000
#define MID_TRACE_PARM_IND_NUMBER_MASK 0x0F000000
#define MID_TRACE_PARM_IND_OFFSET_MASK 0x00FF0000
#define MID_TRACE_PARM_USER_MASK       0x00008003
#define MID_TRACE_PARM_OP_MASK         0x00004F00
#define MID_TRACE_PARM_ADDR_MASK       0x000030FC

#define MAX_TYPE_X_SIZE                0x00001000

#define HKWD_ID                        HKWD_DISPLAY_RESERVED_518

#define MID_TRACE_TYPE_X               0x00000000
#define MID_TRACE_TYPE_0               0x00010000
#define MID_TRACE_TYPE_1               0x00020000
#define MID_TRACE_TYPE_2               0x00060000
#define MID_TRACE_TYPE_3               0x00060000
#define MID_TRACE_TYPE_4               0x00060000
#define MID_TRACE_TYPE_5               0x00060000

#define MID_HOOKLEN_0                   0x00000000
#define MID_HOOKLEN_1                   0x00000001
#define MID_HOOKLEN_2                   0x00000002
#define MID_HOOKLEN_3                   0x00000003
#define MID_HOOKLEN_4                   0x00000004
#define MID_HOOKLEN_5                   0x00000005

#define MID_TRACE_USER_OP_OTHER 0x0000    /* User id for OTHER traces        */
#define MID_TRACE_USER_OP_TED   0x0001    /* User id for TED   traces        */
#define MID_TRACE_USER_OP_CDD   0x0002    /* User id for CDD   traces        */
#define MID_TRACE_USER_OP_DD    0x0003    /* User id for DD    traces        */
#define MID_TRACE_USER_OP_3DM2  0x8000    /* User id for 3DM2  traces        */
#define MID_TRACE_USER_OP_3DM1  0x8001    /* User id for 3DM1  traces        */
#define MID_TRACE_USER_OP_2D    0x8002    /* User id for 2D    traces        */
#define MID_TRACE_USER_OP_RMS   0x8003    /* User id for RMS   traces        */

#define MID_TRACE_USER_ID_RMS       'r'   /* User id for RMS   traces        */
#define MID_TRACE_USER_ID_2D        'x'   /* User id for 2D    traces        */
#define MID_TRACE_USER_ID_3DM1      '1'   /* User id for 3DM1  traces        */
#define MID_TRACE_USER_ID_3DM2      '2'   /* User id for 3DM2  traces        */
#define MID_TRACE_USER_ID_DD        'd'   /* User id for DD    traces        */
#define MID_TRACE_USER_ID_CDD       'c'   /* User id for CDD   traces        */
#define MID_TRACE_USER_ID_TED       't'   /* User id for TED   traces        */
#define MID_TRACE_USER_ID_OTHER     '?'   /* User id for OTHER traces        */

#ifdef MID_RMS
#  define       MID_TRACE_USER_OP       MID_TRACE_USER_OP_RMS
#elif  MID_2D
#  define       MID_TRACE_USER_OP       MID_TRACE_USER_OP_2D
#elif  MID_3DM2
#  define       MID_TRACE_USER_OP       MID_TRACE_USER_OP_3DM2
#elif  MID_3DM1
#  define       MID_TRACE_USER_OP       MID_TRACE_USER_OP_3DM1
#elif  MID_DD
#  define       MID_TRACE_USER_OP       MID_TRACE_USER_OP_DD
#elif  MID_CDD
#  define       MID_TRACE_USER_OP       MID_TRACE_USER_OP_CDD
#elif  MID_TED
#  define       MID_TRACE_USER_OP       MID_TRACE_USER_OP_TED
#else
#  define       MID_TRACE_USER_OP       MID_TRACE_USER_OP_OTHER
#endif

#define MID_TRACE_HOOK_X (HKWD_ID | MID_TRACE_TIME_MASK | MID_TRACE_TYPE_X)
#define MID_TRACE_HOOK_0 (HKWD_ID | MID_TRACE_TIME_MASK | MID_TRACE_TYPE_0 | MID_HOOKLEN_0)
#define MID_TRACE_HOOK_1 (HKWD_ID | MID_TRACE_TIME_MASK | MID_TRACE_TYPE_1 | MID_HOOKLEN_1)
#define MID_TRACE_HOOK_2 (HKWD_ID | MID_TRACE_TIME_MASK | MID_TRACE_TYPE_2 | MID_HOOKLEN_2)
#define MID_TRACE_HOOK_3 (HKWD_ID | MID_TRACE_TIME_MASK | MID_TRACE_TYPE_3 | MID_HOOKLEN_3)
#define MID_TRACE_HOOK_4 (HKWD_ID | MID_TRACE_TIME_MASK | MID_TRACE_TYPE_4 | MID_HOOKLEN_4)
#define MID_TRACE_HOOK_5 (HKWD_ID | MID_TRACE_TIME_MASK | MID_TRACE_TYPE_5 | MID_HOOKLEN_5)

#define BLK_NONE                  0x0000  /* None Specified                  */
#define BLK_ASCB                  0x1000  /* Adapter Status Control Block    */
#define BLK_FBCSB                 0x2000  /* Frame Buffer Control Status Blk */
#define BLK_CSB                   0x3000  /* Current Context Status Block    */
#define BLK_CSB_X                 0x4000  /* Context Status Block (0-15)     */
#define BLK_CMRB                  0x5000  /* Context Memory Request Block    */
#define BLK_FRB                   0x6000  /* Font Request Block              */
#define BLK_CCB                   0x7000  /* Context Command Block           */
#define BLK_CTCB_X                0x8000  /* Color Table Command Block (0-4) */
#define BLK_CICB                  0x9000  /* Cursor Image Command Block      */
#define BLK_M1SB                  0xA000  /* Current 3DM1 Status Block       */
#define BLK_M1SB_X                0xB000  /* 3DM1 Status Block (0-15)        */
#define BLK_M1MSB                 0xC000  /* Current 3DM1M Status Block      */
#define BLK_M1MSB_X               0xD000  /* 3DM1M Status Block (0-15)       */

#define MID_TRACE_OP_TIME_STAMP   0x0000  /* T = Time Stamp                  */
#define MID_TRACE_OP_FIFO_BUFFER  0x0100  /* + = Buffer of FIFO data         */
#define MID_TRACE_OP_PCB_BUFFER   0x0200  /* - = Buffer of PCB data          */
#define MID_TRACE_OP_IRD_BUFFER   0x0300  /* < = Buffer of INDIRECT READ data*/
#define MID_TRACE_OP_IWR_BUFFER   0x0400  /* > = Buffer of INDIRECT WRITE dat*/
#define MID_TRACE_OP_TED_BUFFER   0x0500  /* * = Buffer of TED data          */
#define MID_TRACE_OP_HCR_BUFFER   0x0600  /* $ = Buffer of HCR data          */
#define MID_TRACE_OP_READ_REG     0x0700  /* R = Read Register Value         */
#define MID_TRACE_OP_WRITE_REG    0x0800  /* W = Write Register Value        */
#define MID_TRACE_OP_FREE_SPACE   0x0900  /* F = Poll for Free Space Comment */
#define MID_TRACE_OP_PCB_FREE     0x0A00  /* Q = Query for PCB Free          */
#define MID_TRACE_OP_POLL_COUNT   0x0B00  /* @ = Waiting for lock            */
#define MID_TRACE_OP_DELAY        0x0C00  /* D = Delay for msec Specified    */
#define MID_TRACE_OP_ECHO_COMMENT 0x0D00  /* C = Comment (Echoed to Screen)  */
#define MID_TRACE_OP_COMMENT      0x0E00  /* # = Comment (Not Printed)       */
#define MID_TRACE_OP_VALUE        0x4000  /* V = Last Value Read Comment     */
#define MID_TRACE_OP_EQUAL        0x4100  /* E = Poll Until Equal To         */
#define MID_TRACE_OP_NOT_EQUAL    0x4200  /* N = Poll Until Not Equal To     */
#define MID_TRACE_OP_LESS_EQUAL   0x4300  /* L = Poll Until Less or Equal To */
#define MID_TRACE_OP_MORE_EQUAL   0x4400  /* M = Poll Until More or Equal To */

#define MID_TRACE_OP_GET_POS      0x4500  /* G = Get POS Register Value      */
#define MID_TRACE_OP_PUT_POS      0x4600  /* P = Put POS Register Value      */

#define MID_TRACE_OP_SYSTEM_FILE  0x4700  /* Y = Control System Trace        */
#define MID_TRACE_OP_ASCII_FILE   0x4800  /* A = Control ASCII File Trace    */
#define MID_TRACE_OP_BINARY_FILE  0x4900  /* B = Control Binary File Trace   */
#define MID_TRACE_OP_ASCII_SCREEN 0x4A00  /* X = Control ASCII Screen Trace  */
#define MID_TRACE_OP_SINGLE_STEP  0x4B00  /* S = Control Single Step Mode    */
#define MID_TRACE_OP_RING_BELL    0x4C00  /* Z = Ring Bell                   */
#define MID_TRACE_OP_NAMES        0x4D00  /* ? = Print Register Names        */

#define MID_TRACE_ID_TIME_STAMP     'T'   /* T = Time Stamp                  */
#define MID_TRACE_ID_FIFO_BUFFER    '+'   /* + = Buffer of FIFO data         */
#define MID_TRACE_ID_PCB_BUFFER     '-'   /* - = Buffer of PCB data          */
#define MID_TRACE_ID_IRD_BUFFER     '<'   /* < = Buffer of INDIRECT READ data*/
#define MID_TRACE_ID_IWR_BUFFER     '>'   /* > = Buffer of INDIRECT WRITE dat*/
#define MID_TRACE_ID_TED_BUFFER     '*'   /* * = Buffer of TED data          */
#define MID_TRACE_ID_HCR_BUFFER     '$'   /* $ = Buffer of HCR data          */
#define MID_TRACE_ID_READ_REG       'R'   /* R = Read Register Value         */
#define MID_TRACE_ID_WRITE_REG      'W'   /* W = Write Register Value        */
#define MID_TRACE_ID_FREE_SPACE     'F'   /* F = Poll for Free Space Comment */
#define MID_TRACE_ID_PCB_FREE       'Q'   /* Q = Query for PCB Free          */
#define MID_TRACE_ID_POLL_COUNT     '@'   /* @ = Waiting for lock            */
#define MID_TRACE_ID_DELAY          'D'   /* D = Delay for msec Specified    */
#define MID_TRACE_ID_ECHO_COMMENT   'C'   /* C = Comment (Echoed to Screen)  */
#define MID_TRACE_ID_COMMENT        '#'   /* # = Comment (Not Printed)       */

#define MID_TRACE_ID_GET_POS        'G'   /* G = Get POS Register Value      */
#define MID_TRACE_ID_PUT_POS        'P'   /* P = Put POS Register Value      */

#define MID_TRACE_ID_SYSTEM_FILE    'Y'   /* Y = Control System Trace        */
#define MID_TRACE_ID_ASCII_FILE     'A'   /* A = Control ASCII File Trace    */
#define MID_TRACE_ID_BINARY_FILE    'B'   /* B = Control Binary File Trace   */
#define MID_TRACE_ID_ASCII_SCREEN   'X'   /* X = Control ASCII Screen Trace  */
#define MID_TRACE_ID_SINGLE_STEP    'S'   /* S = Control Single Step Mode    */
#define MID_TRACE_ID_RING_BELL      'Z'   /* Z = Ring Bell                   */
#define MID_TRACE_ID_NAMES          '?'   /* ? = Print Register Names        */

#define MID_TRACE_ID_VALUE          'V'   /* V = Last Value Read Comment     */
#define MID_TRACE_ID_EQUAL          'E'   /* E = Poll Until Equal To         */
#define MID_TRACE_ID_NOT_EQUAL      'N'   /* N = Poll Until Not Equal To     */
#define MID_TRACE_ID_LESS_EQUAL     'L'   /* L = Poll Until Less or Equal To */
#define MID_TRACE_ID_MORE_EQUAL     'M'   /* M = Poll Until More or Equal To */

#define MID_TRACE_NAME_TIME_STAMP   "TIME_STAMP   "  /* T */
#define MID_TRACE_NAME_FIFO_BUFFER  "FIFO_BUFFER  "  /* + */
#define MID_TRACE_NAME_PCB_BUFFER   "PCB_BUFFER   "  /* - */
#define MID_TRACE_NAME_IRD_BUFFER   "R.XXXX.BUFFER"  /* < */
#define MID_TRACE_NAME_IWR_BUFFER   "W.XXXX.BUFFER"  /* > */
#define MID_TRACE_NAME_TED_BUFFER   "TED_BUFFER   "  /* * */
#define MID_TRACE_NAME_HCR_BUFFER   "HCR_BUFFER   "  /* $ */
#define MID_TRACE_NAME_READ_REG                      /* R */
#define MID_TRACE_NAME_WRITE_REG                     /* W */
#define MID_TRACE_NAME_FREE_SPACE   "FREE_SPACE   "  /* F */
#define MID_TRACE_NAME_PCB_FREE     "PCB_AVAILABLE"  /* Q */
#define MID_TRACE_NAME_POLL_COUNT                    /* @ */
#define MID_TRACE_NAME_DELAY        "TIME_DELAY   "  /* D */
#define MID_TRACE_NAME_ECHO_COMMENT                  /* C */
#define MID_TRACE_NAME_COMMENT                       /* # */

#define MID_TRACE_NAME_GET_POS                       /* G */
#define MID_TRACE_NAME_PUT_POS                       /* P */

#define MID_TRACE_NAME_SYSTEM_FILE  "SYSTEM_TRACE "  /* Y */
#define MID_TRACE_NAME_ASCII_FILE   "ASCII_TRACE  "  /* A */
#define MID_TRACE_NAME_BINARY_FILE  "BINARY_TRACE "  /* B */
#define MID_TRACE_NAME_ASCII_SCREEN "SCREEN_TRACE "  /* X */
#define MID_TRACE_NAME_SINGLE_STEP  "SINGLE_STEP  "  /* S */
#define MID_TRACE_NAME_RING_BELL    "RING_BELL    "  /* Z */
#define MID_TRACE_NAME_NAMES        "PRINT_NAMES  "  /* ? */

#define MID_TRACE_NAME_VALUE                         /* V */
#define MID_TRACE_NAME_EQUAL                         /* E */
#define MID_TRACE_NAME_NOT_EQUAL                     /* N */
#define MID_TRACE_NAME_LESS_EQUAL                    /* L */
#define MID_TRACE_NAME_MORE_EQUAL                    /* M */

#define MID_TRACE_HOOK_FIFO_BUFFER  MID_TRACE_HOOK_X
#define MID_TRACE_HOOK_PCB_BUFFER   MID_TRACE_HOOK_X
#define MID_TRACE_HOOK_IRD_BUFFER   MID_TRACE_HOOK_X
#define MID_TRACE_HOOK_IWR_BUFFER   MID_TRACE_HOOK_X
#define MID_TRACE_HOOK_TED_BUFFER   MID_TRACE_HOOK_X
#define MID_TRACE_HOOK_HCR_BUFFER   MID_TRACE_HOOK_X
#define MID_TRACE_HOOK_TIME_STAMP   MID_TRACE_HOOK_0
#define MID_TRACE_HOOK_READ_REG     MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_WRITE_REG    MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_FREE_SPACE   MID_TRACE_HOOK_1
#define MID_TRACE_HOOK_PCB_FREE     MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_POLL_COUNT   MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_DELAY        MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_ECHO_COMMENT MID_TRACE_HOOK_X
#define MID_TRACE_HOOK_COMMENT      MID_TRACE_HOOK_X

#define MID_TRACE_HOOK_GET_POS      MID_TRACE_HOOK_3
#define MID_TRACE_HOOK_PUT_POS      MID_TRACE_HOOK_3

#define MID_TRACE_HOOK_SYSTEM_FILE  MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_ASCII_FILE   MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_BINARY_FILE  MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_ASCII_SCREEN MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_SINGLE_STEP  MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_RING_BELL    MID_TRACE_HOOK_1
#define MID_TRACE_HOOK_NAMES        MID_TRACE_HOOK_1

#define MID_TRACE_HOOK_VALUE        MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_EQUAL        MID_TRACE_HOOK_3
#define MID_TRACE_HOOK_NOT_EQUAL    MID_TRACE_HOOK_3
#define MID_TRACE_HOOK_LESS_EQUAL   MID_TRACE_HOOK_2
#define MID_TRACE_HOOK_MORE_EQUAL   MID_TRACE_HOOK_2


#if     MID_TRACE_SYS

#ifdef  MID_DD
#define _KERNEL         1
#endif  /* MID_DD */

#define MID_TRACE_SYS_X_MAX( parm, length, buffer )                           \
{                                                                             \
	if (TRC_ISON(0))                                                      \
	{                                                                     \
	        int     _trc_offset;                                          \
	        int     _trc_blocks = ((int)(length) * 4) - MAX_TYPE_X_SIZE;  \
	                                                                      \
	        for ( _trc_offset=0; _trc_offset<_trc_blocks;                 \
	              _trc_offset+=MAX_TYPE_X_SIZE )                          \
	        {                                                             \
	                MID_TRACE_SYS_X( parm,                                \
	                        MAX_TYPE_X_SIZE,                              \
	                        ((char *) buffer) + _trc_offset )             \
	        }                                                             \
	        MID_TRACE_SYS_X( parm,                                        \
	                ((int)(length) * 4) - _trc_offset,                    \
	                ((char *) buffer) + _trc_offset)                      \
	}                                                                     \
}

#if    MID_TRACE_TIME_STAMP

#define MID_TRACE_SYS_X( parm, len, buf)                                      \
	TRCGENT( 0, MID_TRACE_HOOK_X, (parm), (len), (buf) );

#define MID_TRACE_SYS_X_1( parm, data)                                        \
{                                                                             \
	int     _trc_data = data;                                             \
	TRCGENT( 0, MID_TRACE_HOOK_X, (parm), 1, &_trc_data );                \
}

#define MID_TRACE_SYS_0( )                                                    \
	TRCHKL0T( MID_TRACE_HOOK_0 );

#define MID_TRACE_SYS_1( parm )                                               \
	TRCHKL1T( MID_TRACE_HOOK_1, (parm) );

#define MID_TRACE_SYS_2( parm, v1 )                                           \
	TRCHKL2T( MID_TRACE_HOOK_2, (parm), (v1) );

#define MID_TRACE_SYS_3( parm, v1, v2 )                                       \
	TRCHKL3T( MID_TRACE_HOOK_3, (parm), (v1), (v2) );

#define MID_TRACE_SYS_4( parm, v1, v2, v3 )                                   \
	TRCHKL4T( MID_TRACE_HOOK_4, (parm), (v1), (v2), (v3) );

#define MID_TRACE_SYS_5( parm, v1, v2, v3, v4 )                               \
	TRCHKL5T( MID_TRACE_HOOK_5, (parm), (v1), (v2), (v3), (v4) );

#else  /* MID_TRACE_TIME_STAMP == 0 */

#define MID_TRACE_SYS_X( parm, len, buf)                                      \
	TRCGEN( 0, MID_TRACE_HOOK_X, (parm), (len), (buf) );

#define MID_TRACE_SYS_X_1( parm, data)                                        \
{                                                                             \
	int     _trc_data = data;                                             \
	TRCGEN( 0, MID_TRACE_HOOK_X, (parm), 1, &_trc_data );                 \
}

#define MID_TRACE_SYS_0( )                                                    \
	TRCHKL0( MID_TRACE_HOOK_0 );

#define MID_TRACE_SYS_1( parm )                                               \
	TRCHKL1( MID_TRACE_HOOK_1, (parm) );

#define MID_TRACE_SYS_2( parm, v1 )                                           \
	TRCHKL2( MID_TRACE_HOOK_2, (parm), (v1) );

#define MID_TRACE_SYS_3( parm, v1, v2 )                                       \
	TRCHKL3( MID_TRACE_HOOK_3, (parm), (v1), (v2) );

#define MID_TRACE_SYS_4( parm, v1, v2, v3 )                                   \
	TRCHKL4( MID_TRACE_HOOK_4, (parm), (v1), (v2), (v3) );

#define MID_TRACE_SYS_5( parm, v1, v2, v3, v4 )                               \
	TRCHKL5( MID_TRACE_HOOK_5, (parm), (v1), (v2), (v3), (v4) );

#endif /* MID_TRACE_TIME_STAMP == 0 */

#else  /* MID_TRACE_SYS == 0 */

#define MID_TRACE_SYS_X_MAX( parm, len, buf )
#define MID_TRACE_SYS_X( parm, len, buf )
#define MID_TRACE_SYS_X_1( parm, data )
#define MID_TRACE_SYS_0( )
#define MID_TRACE_SYS_1( parm )
#define MID_TRACE_SYS_2( parm, v1 )
#define MID_TRACE_SYS_3( parm, v1, v2 )
#define MID_TRACE_SYS_4( parm, v1, v2, v3 )
#define MID_TRACE_SYS_5( parm, v1, v2, v3, v4 )

#endif /* MID_TRACE_SYS == 0 */

#if    MID_TRACE_SUB

extern  int     mid_trace_flags;

#define MID_TRACE_SUB_CALL( hook, parm, v1, v2, v3, v4, data)                 \
{                                                                             \
	if (mid_trace_flags)                                                  \
	{                                                                     \
	        mid_trace( hook, parm, v1, v2, v3, v4, -1, data);             \
	}                                                                     \
}

#define MID_TRACE_SUB_X_MAX( parm, length, buffer )                           \
{                                                                             \
	if (mid_trace_flags)                                                  \
	{                                                                     \
	        int     _trc_offset;                                          \
	        int     _trc_blocks = ((int)(length) * 4) - MAX_TYPE_X_SIZE;  \
	                                                                      \
	        for ( _trc_offset=0; _trc_offset<_trc_blocks;                 \
	              _trc_offset+=MAX_TYPE_X_SIZE )                          \
	        {                                                             \
	                MID_TRACE_SUB_X( parm, MAX_TYPE_X_SIZE,               \
	                        ((char *) buffer) + _trc_offset )             \
	        }                                                             \
	        MID_TRACE_SUB_X( parm, ((int)(length) * 4) - _trc_offset,     \
	                ((char *) buffer) + _trc_offset)                      \
	}                                                                     \
}

#define MID_TRACE_SUB_X( parm, len, buf )                                     \
	MID_TRACE_SUB_CALL( MID_TRACE_HOOK_X | (len), parm, 0, 0, 0, 0, buf )

#define MID_TRACE_SUB_X_1( parm, data )                                       \
{                                                                             \
	int     _trc_data = data;                                             \
	MID_TRACE_SUB_CALL( MID_TRACE_HOOK_X | 1, parm, 0, 0, 0, 0, &data )   \
}

#define MID_TRACE_SUB_0( )                                                    \
	MID_TRACE_SUB_CALL( MID_TRACE_HOOK_0, 0, 0, 0, 0, 0, 0 )

#define MID_TRACE_SUB_1( parm )                                               \
	MID_TRACE_SUB_CALL( MID_TRACE_HOOK_1, parm, 0, 0, 0, 0, 0 )

#define MID_TRACE_SUB_2( parm, v1 )                                           \
	MID_TRACE_SUB_CALL( MID_TRACE_HOOK_2, parm, v1, 0, 0, 0, 0 )

#define MID_TRACE_SUB_3( parm, v1, v2 )                                       \
	MID_TRACE_SUB_CALL( MID_TRACE_HOOK_3, parm, v1, v2, 0, 0, 0 )

#define MID_TRACE_SUB_4( parm, v1, v2, v3 )                                   \
	MID_TRACE_SUB_CALL( MID_TRACE_HOOK_4, parm, v1, v2, v3, 0, 0 )

#define MID_TRACE_SUB_5( parm, v1, v2, v3, v4 )                               \
	MID_TRACE_SUB_CALL( MID_TRACE_HOOK_5, parm, v1, v2, v3, v4, 0)

#else  /* MID_TRACE_SUB == 0 */

#define MID_TRACE_SUB_CALL( hook, parm, v1, v2, v3, v4, data )
#define MID_TRACE_SUB_X_MAX( parm, len, buf )
#define MID_TRACE_SUB_X( parm, len, buf )
#define MID_TRACE_SUB_X_1( parm, data )
#define MID_TRACE_SUB_0( )
#define MID_TRACE_SUB_1( parm )
#define MID_TRACE_SUB_2( parm, v1 )
#define MID_TRACE_SUB_3( parm, v1, v2 )
#define MID_TRACE_SUB_4( parm, v1, v2, v3 )
#define MID_TRACE_SUB_5( parm, v1, v2, v3, v4 )

#endif /* MID_TRACE_SUB == 0 */


#define MID_TRACE_MAC_X( parm, len, buf )                                     \
{                                                                             \
	MID_TRACE_SYS_X_MAX( (parm) | (MID_TRACE_USER_OP), len, buf )         \
	MID_TRACE_SUB_X_MAX( (parm) | (MID_TRACE_USER_OP), len, buf )         \
}                                                                             \

#define MID_WR_TRACE_MAC_X_1( parm, data )                                    \
{                                                                             \
	MID_TRACE_SYS_X_1( (parm) | (MID_TRACE_USER_OP), data )               \
	MID_TRACE_SUB_X_1( (parm) | (MID_TRACE_USER_OP), data )               \
}

#define MID_TRACE_MAC_0( )                                                    \
{                                                                             \
	MID_TRACE_SYS_0( )                                                    \
	MID_TRACE_SUB_0( )                                                    \
}

#define MID_TRACE_MAC_1( parm )                                               \
{                                                                             \
	MID_TRACE_SYS_1( (parm) | (MID_TRACE_USER_OP) )                       \
	MID_TRACE_SUB_1( (parm) | (MID_TRACE_USER_OP) )                       \
}

#define MID_TRACE_MAC_2( parm, v1 )                                           \
{                                                                             \
	MID_TRACE_SYS_2( (parm) | (MID_TRACE_USER_OP), v1 )                   \
	MID_TRACE_SUB_2( (parm) | (MID_TRACE_USER_OP), v1 )                   \
}

#define MID_TRACE_MAC_3( parm, v1, v2)                                        \
{                                                                             \
	MID_TRACE_SYS_3( (parm) | (MID_TRACE_USER_OP), v1, v2)                \
	MID_TRACE_SUB_3( (parm) | (MID_TRACE_USER_OP), v1, v2)                \
}

#define MID_TRACE_MAC_4( parm, v1, v2, v3)                                    \
{                                                                             \
	MID_TRACE_SYS_4( (parm) | (MID_TRACE_USER_OP), v1, v2, v3)            \
	MID_TRACE_SUB_4( (parm) | (MID_TRACE_USER_OP), v1, v2, v3)            \
}

#define MID_TRACE_MAC_5( parm, v1, v2, v3, v4)                                \
{                                                                             \
	MID_TRACE_SYS_5( (parm) | (MID_TRACE_USER_OP), v1, v2, v3, v4)        \
	MID_TRACE_SUB_5( (parm) | (MID_TRACE_USER_OP), v1, v2, v3, v4)        \
}

#define MID_WR_TRACE_FIFO_1( data )                                           \
	MID_TRACE_MAC_X_1( MID_TRACE_OP_FIFO_BUFFER, data )
	
#define MID_WR_TRACE_FIFO( buffer, length )                                   \
	MID_TRACE_MAC_X( MID_TRACE_OP_FIFO_BUFFER, length, buffer )
	
#define MID_WR_TRACE_PCB( buffer, length )                                    \
	MID_TRACE_MAC_X( MID_TRACE_OP_PCB_BUFFER, length, buffer )

#define MID_RD_TRACE_IRD( buffer, length, parm )                              \
	MID_TRACE_MAC_X( MID_TRACE_OP_IRD_BUFFER | (parm) | MID_ADR_IND_DATA, \
	        length, buffer )

#define MID_WR_TRACE_IWR( buffer, length, parm )                              \
	MID_TRACE_MAC_X( MID_TRACE_OP_IWR_BUFFER | (parm) | MID_ADR_IND_DATA, \
	        length, buffer )

#define MID_WR_TRACE_TED( buffer, length )                                    \
	MID_TRACE_MAC_X( MID_TRACE_OP_TED_BUFFER, length, buffer )

#define MID_WR_TRACE_HCR( buffer, length )                                    \
	MID_TRACE_MAC_X( MID_TRACE_OP_HCR_BUFFER, length, buffer )

#define MID_TS_TRACE( )                                                       \
	MID_TRACE_MAC_0( )

#define MID_RD_TRACE( adr, value )                                            \
	MID_TRACE_MAC_2( (MID_TRACE_OP_READ_REG | adr), value )

#define MID_WR_TRACE( adr, value )                                            \
	MID_TRACE_MAC_2( (MID_TRACE_OP_WRITE_REG | adr), value )

#define MID_FS_TRACE( )                                                       \
	MID_TRACE_MAC_1( MID_TRACE_OP_FREE_SPACE )

#define MID_QP_TRACE( count )                                                 \
	MID_TRACE_MAC_2( MID_TRACE_OP_PCB_FREE, count )

#define MID_DL_TRACE( value )                                                 \
	MID_TRACE_MAC_2( MID_TRACE_OP_DELAY, value )

#define MID_EC_TRACE( buffer, length )                                        \
	MID_TRACE_MAC_X( MID_TRACE_OP_ECHO_COMMENT, length, buffer )

#define MID_CM_TRACE( buffer, length )                                        \
	MID_TRACE_MAC_X( MID_TRACE_OP_COMMENT, length, buffer )

#define MID_GT_TRACE( adr, value, mask )                                      \
	MID_TRACE_MAC_3( (MID_TRACE_OP_GET_POS | ((adr) << 2)), value, mask )

#define MID_PT_TRACE( adr, value, mask )                                      \
	MID_TRACE_MAC_3( (MID_TRACE_OP_PUT_POS | ((adr) << 2)), value, mask )

#define MID_SF_TRACE( value )                                                 \
	MID_TRACE_MAC_2( MID_TRACE_OP_SYSTEM_FILE, value )

#define MID_AF_TRACE( value )                                                 \
	MID_TRACE_MAC_2( MID_TRACE_OP_ASCII_FILE, value )

#define MID_BF_TRACE( value )                                                 \
	MID_TRACE_MAC_2( MID_TRACE_OP_BINARY_FILE, value )

#define MID_AS_TRACE( value )                                                 \
	MID_TRACE_MAC_2( MID_TRACE_OP_ASCII_SCREEN, value )

#define MID_SS_TRACE( value )                                                 \
	MID_TRACE_MAC_2( MID_TRACE_OP_SINGLE_STEP, value )

#define MID_RB_TRACE( )                                                       \
	MID_TRACE_MAC_1( MID_TRACE_OP_RING_BELL )

#define MID_NA_TRACE( )                                                       \
	MID_TRACE_MAC_1( MID_TRACE_OP_NAMES )

#define MID_FV_TRACE( adr, value )                                            \
	MID_TRACE_MAC_2( (MID_TRACE_OP_VALUE | adr), value )

#define MID_EQ_TRACE( adr, value, mask )                                      \
	MID_TRACE_MAC_3( (MID_TRACE_OP_EQUAL | adr), value, mask )

#define MID_NE_TRACE( adr, value, mask )                                      \
	MID_TRACE_MAC_3( (MID_TRACE_OP_NOT_EQUAL | adr), value, mask )

#define MID_LE_TRACE( adr, value, mask )                                      \
	MID_TRACE_MAC_2( (MID_TRACE_OP_LESS_EQUAL | adr), value )

#define MID_GE_TRACE( adr, value, mask )                                      \
	MID_TRACE_MAC_2( (MID_TRACE_OP_MORE_EQUAL | adr), value )

#if     MID_TRACE

#define DEFINE_POLL_COUNT( init_val )   ulong   _trc_poll_count = init_val;

#define INCREMENT_POLL_COUNT    _trc_poll_count++;

#define MID_PC_TRACE( adr )                                                   \
	if (_trc_poll_count)                                                  \
	{                                                                     \
	        MID_TRACE_MAC_2( (MID_TRACE_OP_POLL_COUNT | adr),             \
	                _trc_poll_count )                                     \
	}

#else /* MID_TRACE == 0 */

#define DEFINE_POLL_COUNT( init_val )

#define INCREMENT_POLL_COUNT

#define MID_PC_TRACE( adr )

#endif /* MID_TRACE == 0 */

#endif  /* _H_MID_HW_TRACE */
