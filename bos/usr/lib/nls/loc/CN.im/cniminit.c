static char sccsid[] = "@(#)25	1.1  src/bos/usr/lib/nls/loc/CN.im/cniminit.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:34:08";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CNIMInitialize
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
*       Module Name      : CNIMInit                                     *
*                                                                       *
*       Description Name : Chinese Input Method Initialize              *
*                                                                       *
*       Function         : CNIMInitialize : Malloc CNIMfep data struct  *
*                          and hook those TIMFep subroutines.           *
*                                                                       *
*************************************************************************/

#include "chinese.h"
#include "stdio.h"
#include "unistd.h"

/* ------------------------------------------------------------- */
/* *************  CNIMInitialize()  **************************** */
/* ------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : CNIMInitialize()                                           *
* Description : allocate CNIMFep Data structure and hook necessary      *
*               Fep subroutines.                                        *
* External Reference :                                                  *
* Input : Language                                                      *
* Output: The pointer of CNIMFep data structure                         *
*                                                                       *
************************************************************************/

IMFep           CNIMInitialize(language)
   IMLanguage      language;
{

   extern void     CNIMClose();
   extern IMObject CNIMCreate();
   extern void     CNIMDestroy();
   extern int      CNIMProcess();
   extern int      CNIMProcessAuxiliary();
   extern int      CNIMFilter();
   extern int      CNIMLookup();
   extern int      CNIMIoctl();
   extern IMKeymap *_IMInitializeKeymap();

   CNIMFEP          cnimfep;


   cnimfep = (CNIMFEP) malloc(sizeof(CNIMfep));     /* allocate memory for */
   /* CNIMFep data struct  */

   cnimfep->common.imerrno = IMNoError;           /* hook the pointer */
   cnimfep->common.iminitialize = CNIMInitialize;  /* Fep subroutines  */
   cnimfep->common.imclose = CNIMClose;
   cnimfep->common.imcreate = CNIMCreate;
   cnimfep->common.imdestroy = CNIMDestroy;
   cnimfep->common.improcess = CNIMProcess;
   cnimfep->common.improcessaux = CNIMProcessAuxiliary;
   cnimfep->common.imioctl = CNIMIoctl;
   cnimfep->common.imfilter = CNIMFilter;
   cnimfep->common.imlookup = CNIMLookup;

   cnimfep->cnimver = 1;                           /* Version No.-> 1 */

   if ((cnimfep->keymap = _IMInitializeKeymap(language)) == NULL) 
                                                   /* CNim keymap     */ 
   {
      free(cnimfep);                      /* Free CNIMfep data structure */
      cnimfep->common.imerrno = IMKeymapInitializeError;
                                          /* set error                   */
      return (NULL);
   }
    /* if */
   else
      return ((IMFep) cnimfep);

}                                                /* CNIMInitialize() */
