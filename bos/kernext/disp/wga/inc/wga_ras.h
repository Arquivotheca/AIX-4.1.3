/* @(#)89		1.1	src/bos/kernext/disp/wga/inc/wga_ras.h, wgadd, bos411, 9428A410j 10/28/93 18:27:25 */
/*
 * COMPONENT_NAME: (WGADD) Whiteoak Graphics Adapter Device Driver
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


#ifndef _WGA_RAS
#define _WGA_RAS


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


#define WGA_INVALID_MODE	1000
#define WGA_BAD_XMALLOC         1001
#define WGA_BAD_CMD             1002
#define WGA_DEVSWADD            1003
#define WGA_IO_EXCEPT           1004
#define WGA_FONT_LOAD           1005
#define WGA_I_INIT              1006
#define WGA_NODEV               1007

#endif /* _WGA_RAS */
