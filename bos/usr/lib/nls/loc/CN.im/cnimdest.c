static char sccsid[] = "@(#)23	1.1  src/bos/usr/lib/nls/loc/CN.im/cnimdest.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:34:03";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CNIMDestroy
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
/* MODULE NAME:        CNIMClose                                        */
/*                                                                     */
/* DESCRIPTIVE NAME:   Chinese Input Method Destruction                */
/*                                                                     */
/* FUNCTION:           CNIMDestroy : Destroy CNIMOBJ                     */
/*                                                                     */
/* CNedClose                                                            */
/*                                                                     */
/********************* END OF SPECIFICATIONS ***************************/

#include "chinese.h"                     /* Chinese Input Method header file */

CNIMDestroy(obj)
   CNIMOBJ          obj;
{
   extern void     CNedClose();
   char          **aux_str, **aux_atr;
   int             i;

   CNedClose(obj->cnedinfo.CNedID);      /* free fepcb  */

   free(obj->cnedinfo.echobufs);         /* free echo buffer and fix buffer */
   free(obj->cnedinfo.echobufa);
   free(obj->cnedinfo.fixbuf);

   aux_str = obj->cnedinfo.auxbufs;      /* free aux buffers */
   for (i = 0; i < CNIM_AUXROWMAX; i++)
      free(*aux_str++);
   free(obj->cnedinfo.auxbufs);
   aux_atr = obj->cnedinfo.auxbufa;
   for (i = 0; i < CNIM_AUXROWMAX; i++)
      free(*aux_atr++);
   free(obj->cnedinfo.auxbufa);

   free(obj->textinfo.text.str);         /* string/attribute buffers for Text Info. */
   free(obj->textinfo.text.att);

   for (i = 0; i < CNIM_AUXROWMAX; i++)
   {                                     /* free buffers for Aux Info. */
      free(obj->auxinfo.message.text[i].str);
      free(obj->auxinfo.message.text[i].att);
   }

   free(obj->auxinfo.message.text);

   free(obj->string.str);                /* free string buffer for GetString ioctl */

   if(obj->output.data) free(obj->output.data);

   free(obj->indstr.str);                /* free indicator string buffer */

   free(obj->convert_str);

   free(obj);                            /* free CNIM object structure*/

}                                        /* end of destroy */
