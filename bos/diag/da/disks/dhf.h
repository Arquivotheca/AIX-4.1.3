/* @(#)43       1.5.2.4  src/bos/diag/da/disks/dhf.h, dadisks, bos41J, 9509A_all 2/16/95 09:54:31 */
/*
 *   COMPONENT_NAME: DADISKS
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#ifndef _H_DHF
#define _H_DHF

#define CMD_TIMEOUT 30   /* Timeout(secs) to wait for cmd completion   */
#define DIAG_TIMEOUT 60  /* Seconds to wait for diag cmd completion    */

/* Reason codes for inclusion in FruBucket */
#define ATU_ERROR          0x99
#define MISSING_OPTION    0x100
#define NR_MEDIUM_ERR     0x102
#define MOTOR_FAILURE     0x104
#define DRIVE_NREADY      0x105
#define CARD_FAILED       0x106
#define BUS_FAILURE       0x108
#define FORMAT_BAD        0x110
#define DIAG_FAILURE      0x112
#define HARDWARE_ERROR    0x114
#define PROTOCOL_ERROR    0x116
#define WRT_PROTECT_ERR   0x117
#define SCSI_TIMEOUT      0x118
#define SCSI_BUSY         0x120
#define SCSI_RESERVE      0x122
#define SCSI_CHK_COND     0x124
#define DD_CONFIG         0x126
#define ELA_ERROR1        0x128
#define ELA_ERROR3        0x129
#define DEF_ATU_ERROR     0x132
#define	SUBSYSTEM_SERVICE 0x133
#define	ADAPTER_CONFIG_FAIL 0x134
#define DEVICE_CONFIG_FAIL  0x135
#define DEVICE_CERTIFY_FAIL  0x136



/* confidence levels to be put in FruBucket */
#define Conf100  100
#define Conf80   80
#define Conf60   60
#define Conf50   50
#define Conf40   40
#define Conf30	 30
#define Conf20	 20
#define Conf1    1
#define Conf0    0
#define Conf2	 2

#define bad_da_error -1
#define hex1400  0x1400
#define hex1500  0x1500
#define hex1900  0x1900
#define	hex3100  0x3100
#define	hex3200  0x3200
#define hex4000  0x4000

#define YES    2                                /* Menu selection returns   */
#define NO     3

#define BAD_FORMAT_MENUGOAL	-99

#define	num_errs	5
#define srchstr   "%s -N %s"          /* basic srchstr...used in ela function */

long            damode;                        /* Diagnostic Application mode */
int             menu_return;              /* return value from menu selection */
int             fdes;                           /* file descriptor for device */
int             conf_level;                     /* indicates confidence level */
                                                /* 1 = 100%, 0 = <100%        */
uchar           tu_buffer[65536];              /* buff for wrt and req sense */

#endif /* _H_DHF */
