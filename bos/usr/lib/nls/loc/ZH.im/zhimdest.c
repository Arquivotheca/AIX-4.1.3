static char sccsid[] = "@(#)68	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhimdest.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:39:06";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: ZHIMDestroy
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
/* DESCRIPTIVE NAME:   Chinese Input Method Destruction                */
/*                                                                     */
/* FUNCTION:           ZHIMDestroy : Destroy ZHIMOBJ                   */
/*                                                                     */
/* ZHedClose                                                           */
/*                                                                     */
/********************* END OF SPECIFICATIONS ***************************/

#include "chinese.h"              /* Chinese Input Method header file */

ZHIMDestroy(obj)
   ZHIMOBJ          obj;
{
   extern void     ZHedClose();
   char          **aux_str, **aux_atr;
   int             i;

   ZHedClose(obj->zhedinfo.ZHedID);      /* free fepcb  */

   free(obj->zhedinfo.echobufs);         /* free echo buffer and fix buffer */
   free(obj->zhedinfo.echobufa);
   free(obj->zhedinfo.fixbuf);

   aux_str = obj->zhedinfo.auxbufs;      /* free aux buffers */
   for (i = 0; i < ZHIM_AUXROWMAX; i++)
      free(*aux_str++);
   free(obj->zhedinfo.auxbufs);
   aux_atr = obj->zhedinfo.auxbufa;
   for (i = 0; i < ZHIM_AUXROWMAX; i++)
      free(*aux_atr++);
   free(obj->zhedinfo.auxbufa);

   free(obj->textinfo.text.str);         /* string/attribute buffers for Text Info. */
   free(obj->textinfo.text.att);

   for (i = 0; i < ZHIM_AUXROWMAX; i++)
   {                                     /* free buffers for Aux Info. */
      free(obj->auxinfo.message.text[i].str);
      free(obj->auxinfo.message.text[i].att);
   }

   free(obj->auxinfo.message.text);

   free(obj->string.str);                /* free string buffer for GetString ioctl */

   if(obj->output.data) free(obj->output.data);

   free(obj->indstr.str);                /* free indicator string buffer */

   free(obj->convert_str);

   free(obj);                            /* free ZHIM object structure*/

}                                        /* end of destroy */
