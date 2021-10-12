static char sccsid[] = "@(#)21	1.1  src/bos/usr/lib/nls/loc/CN.im/cnimclose.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:33:58";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CNIMClose
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
/* MODULE NAME:        CNIMClose                                       */
/*                                                                     */
/* DESCRIPTIVE NAME:   Chinese Input Method Close                      */
/*                                                                     */
/* FUNCTION:           CNIMClose   :  Close imfep                      */
/*                                                                     */
/*                                                                     */
/********************* END OF SPECIFICATIONS ***************************/

#include "chinese.h"                              /* Chinese Input Method header file */

void            CNIMClose(imfep)
   CNIMFEP          imfep;
{
   _IMCloseKeymap(imfep->keymap);        /* Close the CNim keymap , and free  */
                                         /* the CNIMFEP data structure        */
   free(imfep);
}
