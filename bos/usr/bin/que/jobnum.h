/* @(#)34	1.3  src/bos/usr/bin/que/jobnum.h, cmdque, bos411, 9428A410j 1/28/93 10:59:52 */
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


		/* MINJOB better never be made zero since that's what atoi()
		   returns for bad numbers.  All the code would break. */
#define MINJOB  1               /* Boundaries for job numbers */
#define MAXJOB  999
#define MAXORDER 999999999
