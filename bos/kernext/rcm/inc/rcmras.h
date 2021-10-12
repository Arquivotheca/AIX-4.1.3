/* @(#)57	1.1.1.1  src/bos/kernext/rcm/inc/rcmras.h, rcm, bos411, 9428A410j 10/27/93 16:18:07 */

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager
 *
 * FUNCTIONS: rcmerr
 *
 * ORIGINS: 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985-1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_RAS
#define _H_RAS

/*************************************************************************
  ERROR LOGGING
 *************************************************************************/

/*------------
 Unique RAS codes used to identify specific error locations for error logging
  ------------*/
#define UNIQUE_1        "1"
#define UNIQUE_2        "2"
#define UNIQUE_3        "3"
#define UNIQUE_4        "4"
#define UNIQUE_5        "5"
#define UNIQUE_6        "6"
#define UNIQUE_7        "7"
#define UNIQUE_8        "8"
#define UNIQUE_9        "9"
#define UNIQUE_10       "10"
#define UNIQUE_11       "11"
#define UNIQUE_12       "12"
#define UNIQUE_13       "13"
#define UNIQUE_14       "14"
#define UNIQUE_15       "15"
#define UNIQUE_16       "16"
#define UNIQUE_17       "17"
#define UNIQUE_18       "18"
#define UNIQUE_19       "19"
#define UNIQUE_20       "20"

/*------------
  Generic return codes
  ------------*/
#define UTIL_FAILED             998  /* a kernel function locally by this
	                                   code failed */

#define LOCAL_FUNC              997  /* a utility, such as free_line_data,
	                                   alloc_line_data, init_line_data,
	                                   etc. failed */

/*------------
  Error logging
  ------------*/
#define BAD_HANDLE      0x1000          /* bad gsc handle received */
#define INVALID_CMD     0x1002          /* invalid GSC command */
#define INVALID_STATE   0x1003          /* not active VT or not in MOM */

#endif /* _H_RAS */
