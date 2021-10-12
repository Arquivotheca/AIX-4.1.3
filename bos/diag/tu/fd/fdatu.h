/* @(#)49       1.6  src/bos/diag/tu/fd/fdatu.h, tu_fd, bos411, 9428A410j 10/7/91 13:23:30 */
/*
 * COMPONENT_NAME: (niodskt) NIO diskette tu's
 *
 * FUNCTIONS: exectu
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

typedef struct tucb_t {
        long tu,mfg,loop;
        long r1,r2;
};

/******************************************************************************/
/* tu is the test unit number.                                                */
/* mfg is manufacturing mode.                                                 */
/* loop is number of times to repeat the test unit. If an error occurs        */
/* the application test unit exits.                                           */
/* r1 and r2 is reserved.                                                     */
/******************************************************************************/

#define SECTOR_SIZE 512
#define DISKETTE_TUTYPE diskette_tucb_t

typedef struct diskette_tucb_t {
        struct tucb_t tucb;
        unsigned long ip1,ip2,ip3,ip4;
        struct fd_status diskette_status;
        struct fdparms diskette_parms;
        union {
          char datac[SECTOR_SIZE];
          unsigned long datal[SECTOR_SIZE/4];
        } datacl;
};

/******************************************************************************/
/* ip1, ip2, ip3, ip4 are input required for some test units.                 */
/* diskette_status is defined below.                                          */
/* data[SECTOR_size ] is reserved.                                            */
/******************************************************************************/

#define ADAPTER_TEST       0
#define SELECT_TEST        1
#define DESELECT_TEST      2
#define RECALIB_TEST       3
#define DISK_CHANGE_TEST   4
#define DISK_WR_PROT_TEST  5
#define INDEX_TEST         6
#define STEP_TEST          7
#define READ_TEST          8
#define WRITE_TEST         9
#define WR_READ_CMP_TEST  10
#define LDSTY_1MB_TEST    11
#define LDSTY_2MB_TEST    12
#define VERIFY_TEST       13
#define SPEED_TEST        14
#define HEAD_SETTLE_TEST  15
#define ENABLE_RETRY      16
#define DISABLE_RETRY     17

#define EQUIP_CHECK       0x08
#define TRACK00           0x10
#define DISK_CHANGED      0x80
#define WRITE_PROTECT     0x40
#define LAST_SECTOR       36
#define SIZE              2
#define INV_COM_MASK      0xC0
#define INV_CMND          0x80
#define STATUS_ERROR      0x40
#define EOT               0x80
#define CRC_ERR           0x20
#define CRC_ERR_DATA      0x20
#define OVR_RUN_ERR       0x10
#define NO_DATA           0x04
#define WRITE_PROTECTED   0x02
#define NO_ADDRESS_MARK   0x01
#define NO_ADDR_MARK_DATA 0x01
#define CONTROL_MARK      0x40
#define WRONG_TRACK       0x10
#define BAD_TRACK         0x02
#define HIGH_DENS_SECTOR  0x0A*SECTOR_SIZE
#define MOTOR_SPEED       300
#define MOTOR_SPEED1.2M   300

#ifndef FD8PRTRCK
#define FD8PRTRCK     4     /* 8 sectors per track */
#endif /* FD8PRTRCK  */
