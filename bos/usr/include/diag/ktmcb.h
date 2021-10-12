/* @(#)70	1.3  src/bos/usr/include/diag/ktmcb.h, cmddiag, bos411, 9428A410j 12/8/92 08:55:26 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: Diagnostic header file. 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_KTMCB
#define _H_KTMCB

#include "diag/atu.h"

struct ktmcb_data
{
	int     unit;                 /* type of device to be tested   */
};
struct tucb_data
{
	struct  tucb_t          header; /* Standard TU header structure */
	struct  ktmcb_data      ktmcb;  /* ktm TU specific structure    */
};

#endif
