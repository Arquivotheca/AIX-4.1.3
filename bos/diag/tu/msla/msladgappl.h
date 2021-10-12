/* @(#)28	1.2  src/bos/diag/tu/msla/msladgappl.h, tu_msla, bos411, 9428A410j 6/15/90 17:23:03 */
/*
 * COMPONENT_NAME: (msladgappl.h) header file for MSLA diagnostic
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

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <stdio.h>
#include <fcntl.h>

struct opnparms {                       /* struct for 'ext' on OPENX    */
    unsigned rsvd        : 2;           /* reserved. Set to 0.          */
    unsigned diag_mode   : 1;           /* used with diagnostics only   */
    unsigned mode        : 1;           /* reserved. Set to 0.          */
    unsigned change_adrs : 1;           /* reserved. Set to 0.          */
    unsigned start_msla  : 1;           /* used with 'start_msla' only  */
    unsigned stop_msla   : 1;           /* used with 'stop_msla'  only  */
    unsigned link_sw     : 1;           /* capture link switch intrpts  */
    char devmode;                       /* device mode.                 */
    char rvd[2];                        /* reserved. Set to 0.          */
    int signal;                         /* signal for intrpt q non-empty*/
    char *u_err_area;                   /* usr bfr adr for error data   */
};
