/* @(#)99       1.4.4.3  src/bos/diag/da/scsitapes/dtape.h, datape, bos412, 9445C412a 11/9/94 09:13:50 */
/*
 * COMPONENT_NAME: datape (Tape Diagnostics header file)
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        <stdio.h>
#include        <signal.h>
#include        <sys/types.h>
#include        <sys/time.h>
#include        <sys/stat.h>
#include        <fcntl.h>
#include        <nl_types.h>
#include        <limits.h>
#include        <locale.h>
#include        <memory.h>
#include        <errno.h>
#include        <sys/buf.h>
#include        <asl.h>

#include        <diag/scsi_atu.h>
#include        <sys/cfgodm.h>
#include        <sys/errids.h>
#include        <sys/scsi.h>
#include        <sys/tape.h>

#include "diag/da.h"            /* FRU Bucket Database */
#include "diag/diag.h"
#include "diag/da_rc.h"
#include "diag/dcda_msg.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"     /* return codes for DC */
#include "diag/tmdefs.h"        /* diagnostic modes and variables */
#include "diag/tm_input.h"
#include "datape_msg.h"

/* Default Test Unit Sequences (refer to the DA CIS) */
	                 /* Self Test 2 with write protect test. */
#define SELF_TEST2_WP    "01 03 08 FC 04 01 FD FF FC 07"
	                 /* Self Test 2 without write protect test. */
#define SELF_TEST2_NO_WP "01 03 08 FC 04 01 FD FF"
	                 /* Self Test 1 only. */
#define SELF_TEST1       "01 03 08 FC 04 01 FE"

/* Local (DA) Test Unit defines. */
#define DA_W_R_C_TEST   0xFF    /* Write, Read and Compare */
#define SDTU_ST1        0xFE    /* Send Diag, Self Test 1  */
#define SDTU_ST2        0xFD    /* Send Diag, Self Test 2  */
#define LOAD            0xFC    /* Load Unit               */
#define UNLOAD          0xFB    /* Unload Unit             */
#define TUR_4MM4GB      0xFA    /* TUR for 4mm 4gb         */
#define TUR_4MM4GB1     0xF9    /* TUR for 4mm 4gb         */

/* Test sequence defines. */
#define ST1             1
#define ST2_NO_WP       2
#define ST2_WP          3

/*
 * Data Structures
 */

/* Test Unit Data Structure Type Definition */

typedef struct tu_data {
	int         tu_num;
	char         *erps;
	int    data_length;
	char *scsi_cmd_blk;
	char  *data_buffer;
} TU_DATA;

/* DA Data Structure Type Definition */

typedef struct da_data {
	int      dev_type;
	TU_DATA       *tu;
	int         tu_rc;
	int          skey;
	int         scode;
	char       *sdata;
} DA_DATA;

/* Array of default Test Unit structures. */
TU_DATA tus[] = {
	        /* TEST UNIT READY */
	{ 0x01 ,
	  "016:008 002:009 098:098 019:004 090:001" ,
	  0 ,
	  "00 00 00 00 00 00" ,
	  ""
	},
	        /* REQUEST SENSE */
	{ 0x02 ,
	  "098:098 090:001" ,
	  0xFF ,
	  "03 00 00 00 FF 00" ,
	  ""
	},
	        /* RESERVE */
	{ 0x03 ,
	  "098:098 090:001" ,
	  0 ,
	  "16 00 00 00 00 00" ,
	  ""
	},
	        /* MODE SELECT */
	{ 0x04 ,
	  "098:098 005:006 090:001" ,
	  0x0C ,
	  "15 10 00 00 0C 00" ,
	  "00 00 00 08 00 00 00 00 00 00 02 00"
	},
	        /* SEND DIAGNOSTIC (Self Test 1) */
	{ 0xFE ,
	  "098:098 090:001" ,
	  0 ,
	  "1D 06 00 00 00 00" ,
	  ""
	},
	        /* SEND DIAGNOSTIC (Self Test 2) */
	{ 0xFD ,
	  "003:052 007:003 006:001 098:098" ,
	  0 ,
	  "1D 07 00 00 00 00" ,
	  ""
	},
	        /* RELEASE UNIT */
	{ 0x06 ,
	  "098:098 090:001" ,
	  0 ,
	  "17 00 00 00 00 00" ,
	  ""
	},
	        /* MODE SENSE */
	{ 0x07 ,
	  "016:010 098:098 090:001" ,
	  0x04 ,
	  "1A 00 00 00 04 00" ,
	  ""
	},
	        /* INQUIRY */
	{ 0x08 ,
	  "016:198 098:098 090:001" ,
	  0xFF ,
	  "12 00 00 00 FF 00" ,
	  ""
	},
	        /* WRITE */
	{ 0x0A ,
	  "016:003 007:000 098:098 090:001" ,
	  512 ,
	  "0A 01 00 00 01 00" ,
	  ""
	},
	        /* LOAD UNIT */
	{ 0xFC ,
	  "002:007 004:199 098:098 090:001" ,
	  0 ,
	  "1B 00 00 00 01 00" ,
	  ""
	},
	        /* UNLOAD UNIT */
	{ 0xFB ,
	  "002:000 098:098 090:001" ,
	  0 ,
	  "1B 00 00 00 00 00" ,
	  ""
	},
	        /* DA_W_R_C_TEST (tar system call) */
	{ 0xFF ,
	  "016:000 128:128" ,
	  0 ,
	  "" ,
	  ""
	},
	{0,"",0,"",""} /* End of TU structure (tu_num == 0). */
};

/* Array of device specific Test Unit structures. */

struct dev_tu {
	long    led;
	TU_DATA  tu;
};

struct dev_tu dev_tus[] = {
	{ 0x970, /* 9348 */
	                /* MODE SELECT */
	          { 0x04 ,
	            "098:098 090:001" ,
	            0x0C ,
	            "15 00 00 00 0C 00" ,
	            "00 00 00 08 00 00 00 00 00 00 04 00"
	           },
	},
	{ 0x970, /* 9348 */
	                /* SEND DIAGNOSTIC (Self Test 2) */
	        { 0xFD ,
	          "003:052 018:003 006:001 098:098" ,
	          0 ,
	          "1D 07 00 00 00 00" ,
	          ""
	        },
	},
	{ 0x970, /* 9348 */
	                /* LOAD UNIT */
	        { 0xFC ,
	          "002:007 017:007 098:098 090:001" ,
	          0 ,
	          "1B 00 00 00 01 00" ,
	          ""
	         },
	},
	{ 0x971, /* 3600 */
	                /* MODE SELECT */
	          { 0x04 ,
	            "098:098 090:001" ,
	            0x18 ,
	            "15 00 00 00 18 00" ,
	            "00 00 00 08 00 00 00 00 00 00 02 00 00 02 02 78 00 00 08 00 00 00 01 64"
	           },
	},
	{ 0x991, /* 3800 */
	                /* MODE SELECT */
	          { 0x04 ,
	            "098:098 090:001" ,
	            0x18 ,
	            "15 10 00 00 18 00" ,
	            "00 00 00 08 11 00 00 00 00 00 04 00 20 0A 00 00 00 00 01 10 80 00 00 00"
	           },
	},
	{ 0x995, /* 4100 */
	                /* MODE SELECT */
	          { 0x04 ,
	            "098:098 090:001" ,
	            0x18 ,
	            "15 10 00 00 18 00" ,
	            "00 00 00 08 15 00 00 00 00 00 04 00 20 0A 00 00 00 00 01 10 80 00 00 00"
	          },
	},
	{ 0x972, /* 8200 */
	                /* MODE SENSE */
	          { 0x07 ,
	            "016:010 098:098 090:001" ,
	            0x04 ,
	            "1A 10 00 00 04 00" ,
	            ""
	          },
	},
	{ 0x972, /* 8200 */
	                /* REQUEST SENSE */
	        { 0x02 ,
	          "098:098 090:001" ,
	          0xFF ,
	          "03 00 00 00 FF 80" ,
	          ""
	        },
	},
	{ 0x972, /* 8200 */
	                /* MODE SELECT */
	          { 0x04 ,
	            "098:098 090:001" ,
	            0x1E ,
	            "15 10 00 00 1E 00" ,
	            "00 00 00 08 00 00 00 00 00 00 04 00 00 04 8E 02 80 00 02 0A A0 A0 00 00 00 00 00 00 00 00"
	           },
	},
	{ 0x994, /* 8505 SE */
	                /* REQUEST SENSE */
	        { 0x02 ,
	          "098:098 090:001" ,
	          0xFF ,
	          "03 00 00 00 FF 80" ,
	          ""
	        },
	},
	{ 0x994, /* 8505 SE */
	                /* MODE SELECT */
	          { 0x04 ,
	            "098:098 090:001" ,
	            0x0C ,
	            "15 10 00 00 0C 00" ,
	            "00 00 00 08 14 00 00 00 00 00 04 00"
	           },
	},
	{ 0x914, /* 8505 DE */
	                /* REQUEST SENSE */
	        { 0x02 ,
	          "098:098 090:001" ,
	          0xFF ,
	          "03 00 00 00 FF 80" ,
	          ""
	        },
	},
	{ 0x914, /* 8505 DE */
	                /* MODE SELECT */
	          { 0x04 ,
	            "098:098 090:001" ,
	            0x0C ,
	            "15 10 00 00 0C 00" ,
	            "00 00 00 08 14 00 00 00 00 00 04 00"
	           },
	},
	{ 0x998, /* 4mm */
	                /* MODE SELECT */
	          { 0x04 ,
	            "098:098 090:001" ,
	            0x0C ,
	            "15 10 00 00 0C 00" ,
	            "00 00 00 08 00 00 00 00 00 00 04 00"
	           },
	},
	{ 0x998, /* 4mm */
	                /* SEND DIAGNOSTIC (Self Test 1) */
	           { SDTU_ST1 ,
	            "003:052 098:098 006:001" ,
	            0 ,
	            "1D 04 00 00 00 00" ,
	            ""
	           },
	},
	{ 0x998, /* 4mm */
	                /* SEND DIAGNOSTIC (Self Test 2) */
	           { SDTU_ST2 ,
	            "003:052 098:098 006:001" ,
	            0 ,
	            "1D 05 00 00 00 00" ,
	            ""
	           },
	},
	{ 0x998, /* 4mm */
	                /* RECEIVE DIAGNOSTICS */
	           { 0x09 ,
	            "016:450 098:098 090:001" ,
	            0x06 ,
	            "1C 00 00 00 06 00" ,
	            ""
	           },
	},
	{ 0x915, /* 4mm4gb */
	                /* MODE SELECT */
	          { 0x04 ,
	            "098:098 090:001" ,
	            0x0C ,
	            "15 10 00 00 0C 00" ,
	            "00 00 00 08 00 00 00 00 00 00 04 00"
	           },
	},
	{ 0x915, /* 4mm4gb */
	                /* INQUIRY */
	        { 0x08 ,
	          "016:198 098:098 090:001" ,
	          0xFF ,
	          "12 01 00 00 FF 00" ,
	          ""
	        },
	},
	{ 0x915, /* 4mm4gb */
	                /* SEND DIAGNOSTIC (Self Test 1) */
	           { SDTU_ST1 ,
	            "003:052 098:098 006:001" ,
	            0 ,
	            "1D 04 00 00 00 00" ,
	            ""
	           },
	},
	{ 0x915, /* 4mm4gb */
	                /* SEND DIAGNOSTIC (Self Test 2) */
	           { 0xFD ,
	            "003:052 007:003 098:098 006:001" ,
	            0 ,
	            "1D 07 00 00 00 00" ,
	            ""
	           },
	},
	{ 0x915, /* 4mm4gb */
	                /* RECEIVE DIAGNOSTICS */
	           { 0x09 ,
	            "016:451 098:098 090:001" ,
	            0x07 ,
	            "1C 00 00 00 07 00" ,
	            ""
	           },
	},
	{ 0x915, /* 4mm4gb */
	                /* TUR for test tape check. */
	           { 0xFA ,
	            "450:452 451:000 098:098 090:001" ,
	            0 ,
	            "00 00 00 00 00 00" ,
	            ""
	           },
	},
	{ 0x915, /* 4mm4gb */
	                /* TUR for test tape check. */
	           { 0xF9 ,
	            "450:000 098:098" ,
	            0 ,
	            "00 00 00 00 00 00" ,
	            ""
	           },
	},
	{ 0x733, /* 7GB 8 mm 8505XL */
	                /* REQUEST SENSE */
	        { 0x02 ,
	          "098:098 090:001" ,
	          0xFF ,
	          "03 00 00 00 FF 80" ,
	          ""
	        },
	},
	{ 0x733, /* 7GB 8 mm 8505XL */
	                /* MODE SELECT */
	          { 0x04 ,
	            "098:098 090:001" ,
	            0x0C ,
	            "15 10 00 00 0C 00" ,
	            "00 00 00 08 14 00 00 00 00 00 04 00"
	           },
	},
	{ 0 , /* Zero marks the end of this structure. */
	           {0,"",0,"",""},
	}
};

/* Tape drive model types (used to index dev array). */

#define OST       0     /* LED 973 UNKNOWN MODEL TYPE.       */
#define T3600     1     /* LED 971 1/4" Model 7207-001 150MB */
#define T3800     2     /* LED 991 1/4" Model 7207-011 500MB */
#define T4100     3     /* LED 995 1/4" Model 7207-012 1GB   */
#define T8200     4     /* LED 972 8mm Model 8200 2.3GB      */
#define T8505_SE  5     /* LED 994 8mm Model 8500 5GB        */
#define T9348     6     /* LED 970 1/2" Model 9348           */
#define T4MM      7     /* LED 998 (973) 4mm Model 35480     */
#define T3490     8     /* LED 899 1/2" Model 3490           */
#define T8505_DE  9     /* LED 914 8mm Model 8500 5GB DE     */
#define T4MM4GB   10    /* LED 915 4mm4gb Model 4326NP/RP    */
#define T8505XL     11    /* LED 733 8mm Model 8505XL 7GB        */

/* Test Times in minutes. */
#define OST_TT    "25"
#define T3600_TT  "25"
#define T3800_TT  "25"
#define T4100_TT  "25"
#define T8200_TT  "10"
#define T8505_TT  "10"
#define T9348_TT  "30"
#define T4MM_TT   "10"
#define T4326_TT  "2"
/* Load Times in minutes. */
#define OST_LT    "8"
#define T3600_LT  "5"
#define T3800_LT  "5"
#define T4100_LT  "5"
#define T8200_LT  "5"
#define T8505_LT  "7"
#define T9348_LT  "5"
#define T4MM_LT   "3"
#define T4326_LT  "3"

/* OST 4MM VPD search criteria. */
#define T4MM_VPD "IBM35480A"

/* Structure for device specific data. */

typedef struct dev_data {
	long      led;                  /* System LED value.            */
	int       we_msg;               /* Write Enable catalog number. */
	int       wp_msg;               /* Write Protect catalog number.*/
	char      *test_time;           /* Test time in minutes.        */
	char      *load_time;           /* Tape load time in minutes.   */
	int       test_tape_pn;         /* Test tape PN catalog number. */
	char      *st2_wp;              /* Self test 2 with wp test.    */
	char      *st2_no_wp;           /* Self test 2 without wp test. */
	char      *st1;                 /* Self test 1.                 */
} DEV_DATA;

/* This array is indexed by the drive model types above. */
/* The structs are in order by device type !             */

DEV_DATA dev[] = {
	{ /* OST */
	        0x973,   OST_WE,   OST_WP,   OST_TT,   OST_LT,   OST_PN,
	        NULL, NULL, NULL /* Use defaults */
	},
	{ /* T3600 */
	        0x971, T3600_WE, T3600_WP, T3600_TT, T3600_LT, T3600_PN,
	        NULL, NULL, NULL /* Use defaults */
	},
	{ /* T3800 */
	        0x991, T3800_WE, T3800_WP, T3800_TT, T3800_LT, T3800_PN,
	        NULL, NULL, NULL /* Use defaults */
	},
	{ /* T4100 */
	        0x995, T4100_WE, T4100_WP, T4100_TT, T4100_LT, T4100_PN,
	        NULL, NULL, NULL /* Use defaults */
	},
	{ /* T8200 */
	        0x972, T8200_WE, T8200_WP, T8200_TT, T8200_LT, T8200_PN,
	        "01 03 08 FB FC 04 01 FD FF FB FC 07 FB" ,
	        "01 03 08 FB FC 04 01 FD FF" ,
	        "01 03 08 FB 01 FE"
	},
	{ /* T8505 SE */
	        0x994, T8505_WE, T8505_WP, T8505_TT, T8505_LT, T8505_PN,
	        "01 03 08 FB FC 04 01 FD FF FB FC 07 FB" ,
	        "01 03 08 FB FC 04 01 FD FF" ,
	        "01 03 08 FB 01 FE"
	},
	{ /* T9348 */
	        0x970, T9348_WE, T9348_WP, T9348_TT, T9348_LT, T9348_PN,
	        NULL, NULL,
	        "01 03 08 04 01 FE"
	},
	{ /* T4MM */
	        0x998, T4MM_WE, T4MM_WP, T4MM_TT, T4MM_LT, T4MM_PN,
	        "01 03 08 FB FC 09" ,
	        "01 03 08 FB FE" ,
	        "01 03 08 FB FE"
	},
	{ /* T3490 */
	        0x899, 0,0,NULL,NULL,0,NULL,NULL,NULL /* Not used. */
	},
	{ /* T8505 DE */
	        0x914, T8505_WE, T8505_WP, T8505_TT, T8505_LT, T8505_PN,
	        "01 03 08 FB FC 04 01 FD FF FB FC 07 FB" ,
	        "01 03 08 FB FC 04 01 FD FF" ,
	        "01 03 08 FB 01 FE"
	},
	{ /* T4MM4GB */
	        0x915, T4MM_WE, T4MM_WP, T4326_TT, T4326_LT, T4326_PN,
	        "01 03 08 FB FC F9 FA 09" ,
	        "01 03 08 FB FE" ,
	        "01 03 08 FB FE"
	},
	{ /* T8505XL 7GB 8mm */
	        0x733, T8505_WE, T8505_WP, T8505_TT, T8505_LT, T8505_PN,
	        "01 03 08 FB FC 04 01 FD FF FB FC 07 FB" ,
	        "01 03 08 FB FC 04 01 FD FF" ,
	        "01 03 08 FB 01 FE"
	}
};

/* Tape DA FRU Bucket Data Structure Initialization */

struct fru_bucket da_frub[] = {
	{ "", FRUB1, 0x973 , 0x110 , DTAPE_RESERVE ,
	        {
	                { 100, "", "", 0, DA_NAME, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x120 , DTAPE_INQUIRY ,
	        {
	                { 100, "", "", 0, DA_NAME, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x130 , DTAPE_LOAD ,
	        {
	                { 90, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x135 , DTAPE_UNLOAD ,
	        {
	                { 90, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x140 , DTAPE_MODE_SELECT ,
	        {
	                { 100, "", "", 0, DA_NAME, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x150 , DTAPE_TEST_UNIT_READY ,
	        {
	                { 90, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x160 , DTAPE_SEND_DIAGNOSTICS ,
	        {
	                { 60, "", "", 0, DA_NAME, EXEMPT },
	                { 40, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x169 , DTAPE_SEND_DIAGNOSTICS ,
	        {
	                { 60, "", "", 0, DA_NAME, EXEMPT },
	                { 40, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x170 , DTAPE_TAR_W_R_COMPARE ,
	        {
	                { 40, "", "", 0, DA_NAME, EXEMPT },
	                { 40, "", "", 0, PARENT_NAME, EXEMPT },
	                { 20, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x180 , DTAPE_LOAD ,
	        {
	                { 90, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x185 , DTAPE_UNLOAD ,
	        {
	                { 90, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x190 , DTAPE_MODE_SELECT ,
	        {
	                 { 100, "", "", 0, DA_NAME, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x200 , DTAPE_TEST_UNIT_READY ,
	        {
	                { 90, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x210 , DTAPE_WRITE_PROTECT ,
	        {
	                { 90, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x220 , DTAPE_RELEASE ,
	        {
	                { 100, "", "", 0, DA_NAME, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x230 , DTAPE_REQUEST_SENSE ,
	        {
	               { 100, "", "", 0, DA_NAME, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x240 , DTAPE_OPENX_FAILED ,
	        {
	                { 100, "", "", 0, DA_NAME, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x300 , DTAPE_CFG_DEVICE ,
	        {
	                { 80, "", "", 0, DA_NAME, EXEMPT },
	                { 20, "", "", DTAPE_SW_ERROR, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x310 , DTAPE_CFG_PARENT ,
	        {
	                { 80, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", 0, PARENT_NAME, EXEMPT },
	                { 10, "", "", DTAPE_SW_ERROR, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x320 , DTAPE_ELA_SRN ,
	        {
	                { 60, "", "", 0, DA_NAME, EXEMPT },
	                { 40, "", "", DTAPE_MEDIA, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x400 , DTAPE_RESERVATION ,
	        {
	                { 80, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", 0, PARENT_NAME, EXEMPT },
	                { 10, "", "", DTAPE_SW_ERROR, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x500 , DTAPE_NONEXTENDED ,
	        {
	                { 90, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", 0, PARENT_NAME, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x600 , DTAPE_BUS_IO ,
	        {
	                { 60, "", "", 0, PARENT_NAME, EXEMPT },
	                { 30, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", DTAPE_SW_ERROR, NOT_IN_DB, EXEMPT },
	        },
	},
	{ "", FRUB1, 0x973 , 0x700 , DTAPE_TIMEOUT ,
	        {
	                { 80, "", "", 0, DA_NAME, EXEMPT },
	                { 10, "", "", 0, PARENT_NAME, EXEMPT },
	                { 10, "", "", DTAPE_SW_ERROR, NOT_IN_DB, EXEMPT },
	        },
	},
	/* *** End of Structure (rcode == 0) *** */
	{ "", FRUB1, 0 , 0 , 0 ,
	        {
/* END */               { 0, "", "", 0, NOT_IN_DB, EXEMPT },
	        },
	}
};

/* Exit flag status defines. */
#define    EXIT_NTF        0     /* No Trouble Found.            */
#define    EXIT_MNG        1     /* Display Menu Goal.           */
#define    EXIT_SRN        2     /* Display SRN.                 */
#define    EXIT_SW_ERR     3     /* Exit DA with SW Error.       */
#define    EXIT_ERR_OPEN   4     /* Exit DA with DD Open Error.  */
#define    EXIT_SUB        5     /* Exit DA with DD Open Error.  */

/*
 * MACROs for the various TU check conditions.
 * NOTE: x = TU RC , y = Sense Key , z = Sense Code.
 */

#define CHECK_CONDITION(x)    (int)(x == SCATU_CHECK_CONDITION)
#define CLN_BIT_8505(x,y,z)   (int)(CHECK_CONDITION(x) && \
	                            (y == 0x00) && (z == 0xE8))
#define CLND_BIT_8505(x,y,z)  (int)(CHECK_CONDITION(x) && \
	                            (y == 0x00) && (z == 0xE9))
#define NO_SENSE(x,y)         (int)(CHECK_CONDITION(x) && (y == 0x00))
#define NOT_READY(x,y)        (int)(CHECK_CONDITION(x) && (y == 0x02))
#define MEDIA_ERROR(x,y)      (int)(CHECK_CONDITION(x) && (y == 0x03))
#define HARDWARE_ERROR(x,y)   (int)(CHECK_CONDITION(x) && (y == 0x04))
#define ILLEGAL_REQUEST(x,y)  (int)(CHECK_CONDITION(x) && (y == 0x04))
#define DATA_PROTECT(x,y)     (int)(CHECK_CONDITION(x) && (y == 0x07))
#define UNIT_ATTENTION(x,y)   (int)(CHECK_CONDITION(x) && (y == 0x06))

/*
 * MACROs for the various tm_input modes.
 */

#define CONSOLE     (int)(tm_input.console == CONSOLE_TRUE)
#define SYSTEM      (int)(tm_input.system == SYSTEM_TRUE)
#define ADVANCED    (int)(tm_input.advanced == ADVANCED_TRUE)
#define NOTLM       (int)(tm_input.loopmode == LOOPMODE_NOTLM)
#define ENTERLM     (int)(tm_input.loopmode == LOOPMODE_ENTERLM)
#define INLM        (int)(tm_input.loopmode == LOOPMODE_INLM)
#define EXITLM      (int)(tm_input.loopmode == LOOPMODE_EXITLM)
#define ELA         (int)(tm_input.dmode == DMODE_ELA)
#define PD          (int)(tm_input.dmode == DMODE_PD)
#define REPAIR      (int)(tm_input.dmode == DMODE_REPAIR)
#define SYSX        (int)(tm_input.exenv == EXENV_SYSX)

#define INTERACTIVE (int)( CONSOLE && ( (SYSX && ENTERLM) || \
	                                (!SYSTEM && (NOTLM || ENTERLM)) ) )

/*
 * Error Path Recovery Defines
 */

#define ERP_RETRY       0
#define ERP_GOOD        1
#define ERP_FAIL        90
#define ERP_PREV_STEP  -1
#define ERP_PREV_LOAD  -3
#define MAX_RETRYS      3

/*
 * Misc. defines
 */

#define USERS_TAPE      -1           /* If users data tape in drive.    */
#define W_R_C_SIZE      128          /* write, read, compare file size  */
#define PASS_PARAM      STIOCMD      /* parameter for passthrough ioctl */
#define CLEAN_UP        0
#define RETURN          1
#define FSC_8505        28      /* 8505 FSC byte location.     */
#define CLN_ME_4MM      0x8282  /* 4mm ASC/ASCQ for CLEAN bit. */
#define ST1_TIME        "2"
#define ST2_TIME        "8"
#define T4MM_SLEEP      360
#define DATE_SIZE       16
#define SEND_DIAG_MEDIA_ERROR         0x169

/*
 * CuVPD search criteria defines.
 */

#define CuVPD_3490_CRITERIA    "name=%s and vpd like *3490C*"
#define CuVPD_4mm_CRITERIA     "name=%s and vpd like *IBM35480A*"
#define CuVPD_525MB_CRITERIA   "name=%s and vpd like *3800*"

/*
 * 3490 defines
 */     /*
	 * Offset into error log detail data for 3490 extended sense.
	 */
#define EL_SENSE_OFFSET        20
#define EL_3490_OFFSET         42
#define MAX_3490_SENSE_BYTES   54
	/*
	 * Create a message buff large enough to display the maximum
	 * number of error log entries per screen. Each entry is given
	 * 512 bytes to allow for translation.
	 */
#define MAX_3490_ENTRIES       3    /* Error log entries per message. */
#define BUFF_SIZE_3490         (MAX_3490_ENTRIES * 1024)

char msg_3490_buff[BUFF_SIZE_3490]; /* Used in SENSE_3490_N message. */

/* end dtape.h */
