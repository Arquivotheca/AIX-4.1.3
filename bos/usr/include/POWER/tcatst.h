/* @(#)90	1.2  src/bos/usr/include/POWER/tcatst.h, dadca, bos411, 9428A410j 6/15/90 17:45:49 */
/* @(#) HTX tcatst.h	1.3  1/11/90 15:18:26 */
/*
 * COMPONENT_NAME: (TCATU) TCA/DCA Test Unit
 *
 * FUNCTIONS: TCA/DCA Test Unit Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Header HTX/Mfg. 3270 (TCA) Adapter Definitions File

Module Name :  tcatst.h
SCCS ID     :  1.3

Current Date:  3/2/90, 16:15:49
Newest Delta:  1/11/90, 15:18:26

Header file contains basic definitions needed by three applications for
testing the 3270 (TCA) Adapter:

	1)  Hardware exerciser invoked by HTX,
	2)  Manufacturing application, and
	3)  Diagnostic application.


*****************************************************************************/
/*
 * We include the HTX header file hxihtx.h because we have a need
 * to access this structure within test units in case the invoker
 * is an HTX Hardware Exerciser application (i.e., NOT the
 * manufacturing diagnostic application).  Since, the main driver
 * function, exectu(), has already been defined for use by both
 * types of applications, we will "sneak" in this structure 
 * inside the TUTYPE one that we are allowed to define and pass in.
 */
#include "hxihtx.h"

/*
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 */
#define INVOKED_BY_HTX   99

/*
 * standard structure used by manufacturing diagnostics.
 */
struct tucb_t
   {
	long tu,	/* test unit number   */
	     loop,	/* loop test of tu    */
	     mfg;	/* flag = 1 if running mfg. diagnostics, else 0  */

	long r1,	/* reserved */
	     r2;	/* reserved */
   };

/*
 * error types and error codes for tca test units.
 */
#define SYS_ERR     0x00
#define LOG_ERR     0x02

/*
 * LOG_ERR (logic errors)
 */

/*
 * adapter specific definitions for token ring test units.
 */
#define RULE_LEN      8
#define RETRIES_LEN   3
#define YES_NO_LEN    3

struct _tca_htx
   {
	char rule_id[RULE_LEN+1];
	char retries[RETRIES_LEN+1];
	char show[YES_NO_LEN+1];
	struct htx_data *htx_sp;
   };

/*
 * definition of structure passed by BOTH hardware exerciser and
 * manufacturing diagnostics to "exectu()" function for invoking
 * test units.
 */
#define TUTYPE struct _tca_tu

struct _tca_tu
   {
	struct tucb_t header;
	struct _tca_htx tca_htx_s;
   };

#define BOOLEAN	unsigned char
#define BYTE	unsigned char
#define HALFWD	unsigned short
#define WORD	unsigned long
