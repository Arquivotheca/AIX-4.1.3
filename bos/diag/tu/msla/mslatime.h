/* @(#)61	1.2  src/bos/diag/tu/msla/mslatime.h, tu_msla, bos411, 9428A410j 6/15/90 17:24:03 */
/*
 * COMPONENT_NAME: (mslatime.h) header file for MSLA diagnostic
 *			application.
 *
 * FUNCTIONS: none
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


#ifdef R2
#define MS              4000       /* ?milisecond delay                      */
#else
#define MS              1000       /* milisecond delay                       */
#endif
 
#define TIME_OUT (100 * MS)
 
#define MODELOADWAIT   100*MS       /* wait for vram initialization to finish */
#define DELAY_MS(n) {                                                          \
                      volatile unsigned long i_modeload;                       \
                      for(i_modeload=1; (i_modeload < (n)*MS);i_modeload++ );  \
                    }
/* 
#define DELAY_MS(x,n) {                            \
                      for(x=1; (x < (n)*MS);x++ );  \
                    }
*/
#ifdef R2
#define DELAY_MODELOAD DELAY_MS(100)
#else
#define DELAY_MODELOAD DELAY_MS(100)
#endif
