static char sccsid[] = "@(#)70	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhiminit.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:39:09";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: ZHIMInitialize
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
/************************************************************************
*                                                                       *
*       Module Name      : ZHIMInit                                     *
*                                                                       *
*       Description Name : Chinese Input Method Initialize              *
*                                                                       *
*       Function         : ZHIMInitialize : Malloc ZHIMfep data struct  *
*                          and hook those ZHIMFep subroutines.          *
*                                                                       *
*************************************************************************/

#include "chinese.h"
#include "stdio.h"
#include "unistd.h"

/* ------------------------------------------------------------- */
/* *************  ZHIMInitialize()  **************************** */
/* ------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : ZHIMInitialize()                                           *
* Description : allocate ZHIMFep Data structure and hook necessary      *
*               Fep subroutines.                                        *
* External Reference :                                                  *
* Input : Language                                                      *
* Output: The pointer of ZHIMFep data structure                         *
*                                                                       *
************************************************************************/

IMFep           ZHIMInitialize(language)
   IMLanguage      language;
{

   extern void     ZHIMClose();
   extern IMObject ZHIMCreate();
   extern void     ZHIMDestroy();
   extern int      ZHIMProcess();
   extern int      ZHIMProcessAuxiliary();
   extern int      ZHIMFilter();
   extern int      ZHIMLookup();
   extern int      ZHIMIoctl();
   extern IMKeymap *_IMInitializeKeymap();

   ZHIMFEP          zhimfep;


   zhimfep = (ZHIMFEP) malloc(sizeof(ZHIMfep));     /* allocate memory for */
   /* ZHIMFep data struct  */

   zhimfep->common.imerrno = IMNoError;           /* hook the pointer */
   zhimfep->common.iminitialize = ZHIMInitialize;  /* Fep subroutines  */
   zhimfep->common.imclose = ZHIMClose;
   zhimfep->common.imcreate = ZHIMCreate;
   zhimfep->common.imdestroy = ZHIMDestroy;
   zhimfep->common.improcess = ZHIMProcess;
   zhimfep->common.improcessaux = ZHIMProcessAuxiliary;
   zhimfep->common.imioctl = ZHIMIoctl;
   zhimfep->common.imfilter = ZHIMFilter;
   zhimfep->common.imlookup = ZHIMLookup;

   zhimfep->zhimver = 1;                           /* Version No.-> 1 */

   if ((zhimfep->keymap = _IMInitializeKeymap(language)) == NULL) 
                                                   /* ZHim keymap     */ 
   {
      free(zhimfep);                      /* Free ZHIMfep data structure */
      zhimfep->common.imerrno = IMKeymapInitializeError;
                                          /* set error                   */
      return (NULL);
   }
    /* if */
   else
      return ((IMFep) zhimfep);

}                                                /* ZHIMInitialize() */
