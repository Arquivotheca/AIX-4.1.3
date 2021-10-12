/* @(#)67	1.3  src/bos/diag/da/fd/fda_rc.h, dafd, bos411, 9428A410j 12/17/92 10:58:55 */
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_FDA_RC
#define _H_FDA_RC

#define NO_ERROR                       0
#define ADAPTER_NOT_PRESENT            1
#define RECALIBRATE_ERROR              2
#define INVALID_CMD_ERROR              3
#define TERMINAL_COUNT_ERROR           4
#define CRC_ADDRESS_ERROR              5
#define CRC_DATA_ERROR                 6
#define OVERRUN_ERROR                  7
#define NO_DATA_ERROR                  8
#define WRITE_PROTECTED_ERROR          9
#define MISSING_MARK_ADD_ERROR        10
#define MISSING_MARK_DATA_ERROR       11
#define CONTROL_MARK_ERROR            12    /* Bad Diskette   */
#define WRONG_TRACK_ERROR             13
#define BAD_TRACK_ERROR               14
#define DISK_NOT_DETECTED_ERROR       15


#define TRACK_STUCK_00_ERROR          80
#define NO_DISK_PRESENT_ERROR         81   /* Disk change Test             */ 
#define DISK_WAS_WRITE_PROTECTED      82   /* Diskette WAS Write Protected */
#define STEP_TEST_ERROR               83
#define DATA_COMPARE_ERROR            84
#define LOW_DENSITY_FORMAT_ERROR      85
#define LOW_DENSITY_NOT_LOW_ERROR     86
#define LOW_DENSITY_LAST_CYL_ERROR    87
#define DISK_NOT_REMOVED              88   /* Diskette Change Test */
#define MOTOR_SPEED_NOT_IN_SPEC       89
#define HEAD_SETTLE_NOT_IN_SPEC       90
#define UNDEFINED_DEVICE_DRIVER_ERROR 254 
#define INVALID_TEST_UNIT_PASSED     255
#define AIX_ERROR                     -1

#define DISK_WAS_NOT_WRITE_PROTECTED  99



#endif

