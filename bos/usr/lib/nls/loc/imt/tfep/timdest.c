static char sccsid[] = "@(#)19  1.4  src/bos/usr/lib/nls/loc/imt/tfep/timdest.c, libtw, bos411, 9428A410j 4/21/94 02:28:41";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: TIMDestroy
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
/* DESCRIPTIVE NAME:   Chinese Input Method Destruction                */
/*                                                                     */
/* FUNCTION:           TIMDestroy : Destroy TIMOBJ                     */
/*                                                                     */
/* TedClose                                                            */
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

TIMDestroy(obj)
   TIMOBJ          obj;
{
   extern void     TedClose();
   char          **aux_str, **aux_atr;
   int             i;

   TedClose(obj->tedinfo.TedID);                 /* free fepcb  */

   free(obj->tedinfo.echobufs);                  /* free echo buffer and fix buffer */
   free(obj->tedinfo.echobufa);
   free(obj->tedinfo.fixbuf);

   aux_str = obj->tedinfo.auxbufs;               /* free aux buffers */
   for (i = 0; i < TIM_AUXROWMAX; i++)
      free(*aux_str++);
   free(obj->tedinfo.auxbufs);
   aux_atr = obj->tedinfo.auxbufa;
   for (i = 0; i < TIM_AUXROWMAX; i++)
      free(*aux_atr++);
   free(obj->tedinfo.auxbufa);

   free(obj->textinfo.text.str);                 /* string/attribute buffers for Text Info. */
   free(obj->textinfo.text.att);

   for (i = 0; i < TIM_AUXROWMAX; i++)
   {                                             /* free buffers for Aux Info. */
      free(obj->auxinfo.message.text[i].str);
      free(obj->auxinfo.message.text[i].att);
   }

   free(obj->auxinfo.message.text);

   free(obj->auxinfo.selection.panel);           /* big5 debby */

   free(obj->string.str);                        /* free string buffer for GetString ioctl */

   if(obj->output.data) free(obj->output.data);

   free(obj->indstr.str);                        /* free indicator string buffer */

   free(obj->convert_str);

   free(obj);                                    /* free TIM object structure */

}                                                /* end of destroy */
