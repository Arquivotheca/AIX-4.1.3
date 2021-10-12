/* @(#)98	1.2  src/bos/diag/da/corvette/common.h, dascsi, bos411, 9428A410j 8/18/93 10:51:10 */
/*
 *   COMPONENT_NAME: DASCSI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include	<stdio.h>
struct  task
{
        char    task_code[5];
        short   retry_count;
};
#define	EXIT_NOW	TRUE
