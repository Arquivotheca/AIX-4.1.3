/* @(#)04       1.2  src/bos/kernel/io/machdd/md_nvram.h, machdd, bos41J, bai15 4/4/95 08:28:17 */
/*
 *   COMPONENT_NAME: machdd
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _NVRAMMD_
#define _NVRAMMD_

typedef enum _OSAREA_USAGE {
  Empty = 0,
  Used = 1
  } OSAREA_USAGE;

typedef enum _OS_ID {
  Unknown = 0,
  Firmware = 1,
  AIX = 2,
  NT = 3,
  WPOS2 = 4,
  WPX = 5,
  Taligent = 6,
  Solaris = 7,
  Netware = 8,
  USL = 9,
  Low_End_Client = 10,
  SCO = 11
  } OS_ID;

typedef struct _RESTART_BLOCK {
    unsigned short Version;
    unsigned short Revision;
    unsigned long BootMasterId;
    unsigned long ProcessorId;
    volatile unsigned long BootStatus;
    unsigned long CheckSum; /* Checksum of RESTART_BLOCK */
    void * RestartAddress;
    void * SaveAreaAddr;
    unsigned long SaveAreaLength;
  } RESTART_BLOCK;


typedef struct _SECURITY {
  unsigned long BootErrCnt;         /* Count of boot password errors */
  unsigned long ConfigErrCnt;       /* Count of config password errors */
  unsigned long BootErrorDT[2];     /* Date&Time from RTC of last error in pw */
  unsigned long ConfigErrorDT[2];   /* Date&Time from RTC of last error in pw */
  unsigned long BootCorrectDT[2];   /* Date&Time from RTC of last correct pw */
  unsigned long ConfigCorrectDT[2]; /* Date&Time from RTC of last correct pw */
  unsigned long BootSetDT[2];       /* Date&Time from RTC of last set of pw */
  unsigned long ConfigSetDT[2];     /* Date&Time from RTC of last set of pw */
  unsigned char Serial[16];         /* Box serial number */
  } SECURITY;

typedef struct _ERROR_LOG {
  unsigned char Status;    /* ERROR_STATUS */
  unsigned char Os;        /* OS_ID */
  unsigned char Type;      /* H for Hardware, S for Software */
  unsigned char Severity;  /* S - severe, E - error */
    /* The severity should be set S if the OS cannot proceed unless
    fixed.  It should be set to E otherwise.  If the boot process sees
    an S severity it should not attempt to boot the operating system
    that caused it but remain in firmware state.  If it runs the
    diagnostics and gets DiagnosedOK, it may proceed.  */

  unsigned long ErrDT[2];  /* Date&Time from RTC */
  unsigned char code[8];   /* detailed classification of error */
  union {
    unsigned char detail[20];
    } data;
  } ERROR_LOG;

typedef struct _HEADER {
  unsigned short Size;    /* NVRAM size in K(1024) */
  unsigned char Version;  /* Structure map different */
  unsigned char Revision; /* Structure map the same -
                             may be new values in old fields
                             in other words old code still works */
  unsigned short Crc1;    /* check sum from beginning of nvram to OSArea */
  unsigned short Crc2;    /* check sum of config */
  unsigned char LastOS;   /* OS_ID */
  unsigned char Endian;   /* B if big endian, L if little endian */
  unsigned char OSAreaUsage; /* OSAREA_USAGE */
  unsigned char PMMode;   /* Shutdown mode */
  RESTART_BLOCK RestartBlock;
  SECURITY Security;
  ERROR_LOG ErrorLog[2];

/* Global Environment information */
  void * GEAddress;
  unsigned long GELength;
  /* Date&Time from RTC of last change to Global Environment */
  unsigned long GELastWriteDT[2];

/* Configuration information */
  void * ConfigAddress;
  unsigned long ConfigLength;
  /* Date&Time from RTC of last change to Configuration */
  unsigned long ConfigLastWriteDT[2];
  unsigned long ConfigCount; /* Count of entries in Configuration */

/* OS dependent temp area */
  void * OSAreaAddress;
  unsigned long OSAreaLength;
  /* Date&Time from RTC of last change to OSAreaArea */
  unsigned long OSAreaLastWriteDT[2];
  } HEADER;

#endif

