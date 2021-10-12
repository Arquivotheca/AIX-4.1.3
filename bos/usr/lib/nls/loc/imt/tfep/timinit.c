static char sccsid[] = "@(#)21	1.2  src/bos/usr/lib/nls/loc/imt/tfep/timinit.c, libtw, bos411, 9428A410j 9/17/93 09:20:24";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: TIMInitialize
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
/************************************************************************
*                                                                       *
*       Module Name      : TIMInit                                      *
*                                                                       *
*       Description Name : Chinese Input Method Initialize              *
*                                                                       *
*       Function         : TIMInitialize : Malloc TIMfep data struct    *
*                          and hook those TIMFep subroutines.           *
*                                                                       *
*       Module Type      : C                                            *
*                                                                       *
*       Compiler         : AIX C                                        *
*                                                                       *
*       Author           : Jim Roung                                    *
*                                                                       *
*       Status           : Chinese Input Method Version 1.0             *
*                                                                       *
*       Change Activity  : T.B.W.                                       *
*                                                                       *
*************************************************************************/

#include "taiwan.h"
#include "stdio.h"
#include "unistd.h"

/* ------------------------------------------------------------- */
/* *************  TIMInitialize()  ***************************** */
/* ------------------------------------------------------------- */

/************************************************************************
*                                                                       *
* Function : TIMInitialize()                                            *
* Description : allocate TIMFep Data structure and hook necessary       *
*               Fep subroutines.                                        *
* External Reference :                                                  *
* Input : Language                                                      *
* Output: The pointer of TIMFep data structure                          *
*                                                                       *
************************************************************************/

IMFep           TIMInitialize(language)
   IMLanguage      language;
{

   extern void     TIMClose();
   extern IMObject TIMCreate();
   extern void     TIMDestroy();
   extern int      TIMProcess();
   extern int      TIMProcessAuxiliary();
   extern int      TIMFilter();
   extern int      TIMLookup();
   extern int      TIMIoctl();
   extern IMKeymap *_IMInitializeKeymap();

   TIMFEP          timfep;

   timfep = (TIMFEP) malloc(sizeof(TIMfep));     /* allocate memory for */
   /* TIMFep data struct  */

   timfep->common.imerrno = IMNoError;           /* hook the pointer */
   timfep->common.iminitialize = TIMInitialize;  /* Fep subroutines  */
   timfep->common.imclose = TIMClose;
   timfep->common.imcreate = TIMCreate;
   timfep->common.imdestroy = TIMDestroy;
   timfep->common.improcess = TIMProcess;
   timfep->common.improcessaux = TIMProcessAuxiliary;
   timfep->common.imioctl = TIMIoctl;
   timfep->common.imfilter = TIMFilter;
   timfep->common.imlookup = TIMLookup;

   timfep->timver = 1;                           /* Version No.-> 1 */

   if ((timfep->keymap = _IMInitializeKeymap(language)) == NULL)
   {
      free(timfep);
      timfep->common.imerrno = IMKeymapInitializeError;
      return (NULL);
   }
    /* if */
   else
      return ((IMFep) timfep);

}                                                /* TIMInitialize() */
