static char sccsid[] = "@(#)66	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhimclose.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:39:03";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: ZHIMClose
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************* START OF MODULE SPECIFICATION *******************/
/*                                                                     */
/* MODULE NAME:        ZHIMClose                                       */
/*                                                                     */
/* DESCRIPTIVE NAME:   Chinese Input Method Close                      */
/*                                                                     */
/* FUNCTION:           ZHIMClose   :  Close imfep                      */
/*                                                                     */
/* IMFreeKeymap                                                        */
/*                                                                     */
/********************* END OF SPECIFICATIONS ***************************/

#include "chinese.h"                              /* Chinese Input Method header file */

void            ZHIMClose(imfep)
   ZHIMFEP          imfep;
{
   _IMCloseKeymap(imfep->keymap);        /* Close the ZHim keymap , and free  */
                                         /* the ZHIMFEP data structure        */
   free(imfep);
}
