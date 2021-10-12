/* @(#)18	1.2  src/bos/usr/include/diag/atu.h, cmddiag, bos411, 9428A410j 2/19/91 17:12:38 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/* tu is the test unit number.                                               */
/* mfg is manufacturing mode.                                                */
/* loop is number of times to repeat the test unit. If an error occurs       */
/* the application test unit exits.                                          */
/* r1 and r2 is reserved.                                                    */
/*****************************************************************************/
typedef struct tucb_t {
        long tu,mfg,loop;
        long r1,r2;
};
