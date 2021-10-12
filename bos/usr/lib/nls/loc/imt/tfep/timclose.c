static char sccsid[] = "@(#)17	1.2  src/bos/usr/lib/nls/loc/imt/tfep/timclose.c, libtw, bos411, 9428A410j 9/17/93 09:20:03";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: TIMClose
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************* START OF MODULE SPECIFICATION *******************/
/*                                                                     */
/* MODULE NAME:        TIMClose                                        */
/*                                                                     */
/* DESCRIPTIVE NAME:   Chinese Input Method Close                      */
/*                                                                     */
/* FUNCTION:           TIMClose   :  Close imfep                       */
/*                                                                     */
/* IMFreeKeymap                                                        */
/*                                                                     */
/* MODULE TYPE:        C                                               */
/*                                                                     */
/* COMPILER:           AIX C                                           */
/*                                                                     */
/* AUTHOR:             Terry Chou                                      */
/*                                                                     */
/* STATUS:             Chinese Input Method Version 1.0                */
/*                                                                     */
/* CHANGE ACTIVITY:                                                    */
/********************* END OF SPECIFICATIONS ***************************/

#include "taiwan.h"                              /* Chinese Input Method header file */

void            TIMClose(imfep)
   TIMFEP          imfep;
{
   _IMCloseKeymap(imfep->keymap);
   free(imfep);
}
