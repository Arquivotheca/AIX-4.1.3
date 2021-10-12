/* @(#)65       1.4  src/bos/diag/da/fd/fd_set.h, dafd, bos411, 9428A410j 12/17/92 10:58:22 */
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
#ifndef _H_FD_SET
#define _H_FD_SET



#define NO_VIDEO_TEST_MODE         1
#define INTERACTIVE_TEST_MODE      2
#define TITLES_ONLY_TEST_MODE      3

#define  INVALID_TM_INPUT        -1
#define  ANSWERED_YES             1
#define  A_LINE                  80

#define NAME_OF_4MB_DRIVE     "3.5inch4Mb"
#define TEST_DISKETTE_PN_4MB  "00G3352"

#define DRIVE_0               0x00000001
#define DRIVE_1               0x00000002

#define THREE5                0x00000010
#define FIVE25                0x00000020

#define HIGH_DENSITY_4MB      0x00000100
#define HIGH_DENSITY          0x00000200
#define LOW_DENSITY           0x00000400

#define SINGLE_SIDED          0x00001000
#define DOUBLE_SIDED          0x00002000
#define HIGH_CAPACITY         0x00004000
#define LOW_CAPACITY          0x00008000

#define EXTERNAL_DISKETTE_DRIVE   0x02000000
#define INTERNAL_DISKETTE_DRIVE   0x01000000

#define ESIZE_160K            0x00010000
#define ESIZE_320K            0x00020000
#define ESIZE_180K            0x00040000
#define ESIZE_360K            0x00080000
#define ESIZE_12M             0x00100000
#define ESIZE_720K            0x00200000
#define ESIZE_144M            0x00400000
#define ESIZE_288M            0x00800000

struct fd_drive {
       int location;
       int size;
       int capacity;
       int drive_density;
       int disk_density;
       int exORin;
};

#endif /* _H_FD_SET */
