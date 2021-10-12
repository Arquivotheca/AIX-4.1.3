/* @(#)55		1.1	src/bos/kernext/disp/ped/inc/mid_ras.h, peddd, bos411, 9428A410j 10/28/93 14:15:38 */
/*
 * COMPONENT_NAME: (PEDDD) PED Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS:	27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _MID_RAS
#define _MID_RAS


/*************************************************************************
	ERROR LOGGING
 *************************************************************************/

/*------------
 Unique RAS codes used to identify specific error locations for error logging
 ------------*/
#define RAS_UNIQUE_1		"1"
#define RAS_UNIQUE_2		"2"
#define RAS_UNIQUE_3		"3"
#define RAS_UNIQUE_4		"4"
#define RAS_UNIQUE_5		"5"
#define RAS_UNIQUE_6		"6"


#define MID_INVALID_MODE	1000
#define MID_BAD_XMALLOC         1001
#define MID_BAD_CMD             1002
#define MID_DEVSWADD            1003
#define MID_IO_EXCEPT           1004
#define MID_FONT_LOAD           1005
#define MID_I_INIT              1006
#define MID_NODEV               1007
#define MID_BAD_UCODE_FORMAT	1008
#define MID_BAD_UCODE_INPUT	1009
#define MID_INVALID_DISPLAY_MODE	1010

#endif /* _MID_RAS */
