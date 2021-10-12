static char sccsid[] = "@(#)37	1.2  src/bos/diag/tu/msla/mkerr.c, tu_msla, bos411, 9428A410j 6/15/90 17:22:41";
/*
 * COMPONENT_NAME: ( mkerr ) 
 *
 * FUNCTIONS: mkerr
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

#include "mslaerrdef.h"

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  mkerr                                                 */
/*                                                                           */
/*         PURPOSE :  Creates long-word error value from arguments           */
/*                                                                           */
/*            INPUT :  testid,subtestid,errid,rc                             */
/*                                                                           */
/*           OUTPUT :  							     */
/*                                                                           */
/* FUNCTIONS CALLED : 							     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

void
mkerr(testid,subtestid,errid,rc)
unsigned short testid;
char subtestid;
unsigned short errid;
unsigned long *rc;
{
    struct error_id error;

    error.erru_id.errors.testid_word = testid ;
    error.erru_id.errors.subtestid_char = subtestid;
    error.erru_id.errors.err_id = errid;
    *rc = error.erru_id.errval;

}
