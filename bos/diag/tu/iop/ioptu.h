/* @(#)37	1.5  src/bos/diag/tu/iop/ioptu.h, tu_iop, bos41J, 9513A_all 3/9/95 09:01:56 */
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS:  header file 
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */
#ifndef _H_IOPTU
#define _H_IOPTU

/* these error codes are used as indexes for the frub array */
#define EC_REG_TEST_FAILURE        1
#define NVRAM_TEST_FAILURE         2
#define TIME_OF_DAY_TEST_FAILURE   3 
#define LED_TEST_FAILURE           4
#define BATTERY_TEST_FAILURE       5
#define TOD_AT_POR_FAILURE         6
#define TOD_NOT_RUNNING_FAILURE    7
#define KEYLOCK_TEST_FAILURE       8
#define EPOW_CONN_FAILURE          9
/* frub[10] is no more used */
#define IOD_NVRAM_TEST_FAILURE    11
#define LCD_TEST_FAILURE          12
#define IOD_KEYLOCK_TEST_FAILURE  13
#define IOD_TOD_AT_POR_FAILURE    14
#define IOD_TOD_NOT_RUNNING_FAIL  15
#define IOD_TIME_OF_DAY_FAILURE   16
#define BBU_FAILURE   		  17 /* this is the battery backup test which I
				      * run from my DA.  This is actually just
				      * a check of the power status registers 
				      */
#define CPU_FAN_FAILURE   	  18 /* this is a check of the power status 
					register to make sure the cpu fans are 
					working */
#define MEDIA_FAN_FAILURE   	  19 /* this is a check of the power status 
					register to make sure the media fans
					are working */
#define PS1_FAN_FAILURE   	  20 /* this is a check of the power status 
					register to make sure the power supply
					fans in the 1st supply are working */
#define PS2_FAN_FAILURE   	  21 /* this is a check of the power status 
					register to make sure the power supply
					fans in the 2nd supply are working */
#define EXP_CAB_PS_FAILURE        22 /* this is a check of the power status
                                        table of expansion cabinets */

/* these TU return values will be remapped by the DA to one of */
/* the previous ones if considered as failures */
#define TOD_AT_POR_STATE          10
#define BATTERY_LOW               12
#define TOD_NOT_RUNNING           13

#endif

