/* @(#)11		1.1	src/bos/kernext/disp/sga/inc/sga_ras.h, sgadd, bos411, 9428A410j 10/29/93 10:46:29 */
/*
 * COMPONENT_NAME: (SGADD) Salmon Graphics Adapter Device Driver
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


#ifndef _SGA_RAS
#define _SGA_RAS


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
#define RAS_UNIQUE_7		"7"


#define SGA_INVALID_MODE	1000
#define SGA_BAD_MALLOC          1001
#define SGA_BAD_CMD             1002
#define SGA_BAD_DEVSWADD        1003
#define SGA_IO_EXCEPT           1004
#define SGA_FONT_LOAD           1005
#define SGA_I_INIT              1006
#define SGA_UIOMOVE             1007
#define SGA_PINCODE             1008
#define SGA_QVPD                1009
#define SGA_DEVSWDEL            1010

#endif /* _SGA_RAS */
