static char sccsid[] = "@(#)56  1.1  src/bos/usr/lib/nls/loc/imk/kfep/KIMInit.c, libkr, bos411, 9428A410j 5/25/92 15:39:57";
/*
 * COMPONENT_NAME :	(LIBKR) - AIX Input Method
 *
 * FUNCTIONS :		KIMInit.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM FEP
 *
 *  Module:       KIMInit.c
 *
 *  Description:  Korean Input Method Initialize
 *
 *  Functions:    KIMInitialize()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*-----------------------------------------------------------------------*
 *		Include headers.
 *-----------------------------------------------------------------------*/

#include <sys/types.h>
#include <im.h>
#include <imP.h>
#include <imlang.h>
#include "kfep.h"              /* Korean Input Method header file */

/*-----------------------------------------------------------------------*
 *	Beginning of procedure
 *-----------------------------------------------------------------------*/

IMFep KIMInitialize( language )
IMLanguage	     language;
{
    /**********************/
    /* external reference */
    /**********************/
    extern  IMObject 	KIMCreate();
    extern  void   	KIMClose();
    extern  void   	KIMDestroy();
    extern  int    	KIMProcess();
    extern  int         KIMFilter();
    extern  int         KIMLookup();
    extern  int    	KIMProcessAuxiliary();
    extern  int    	KIMIoctl();
    extern  IMMap  	InitializeKeymap(); 

    /*******************/
    /* local variables */
    /*******************/
    KIMFEP  	fep;

    /***************************/
    /*			       */
    /* allocate Korean IMFEP   */
    /*			       */
    /***************************/
    fep = (KIMFEP)malloc(sizeof(KIMfep));

    /******************************/
    /* initialize allocated above */
    /******************************/
    fep->common.imerrno      = 		IMNoError;
    fep->common.iminitialize = 		KIMInitialize;
    fep->common.imclose      = 		KIMClose;
    fep->common.imcreate     = 		KIMCreate;
    fep->common.imdestroy    = 		KIMDestroy;
    fep->common.improcess    = 		KIMProcess;
    fep->common.imfilter     = 		KIMFilter;
    fep->common.imlookup     = 		KIMLookup;
    fep->common.improcessaux = 		KIMProcessAuxiliary;
    fep->common.imioctl      = 		KIMIoctl;
    fep->kimver              = 		KIM_VERSION;

    /*************************************************/
    /*						     */
    /*      have keymap routine initialized	     */
    /*      note: passes the given language as is.   */
    /*						     */
    /*************************************************/
    fep->immap = _IMInitializeKeymap(language);

    if (fep->immap == NULL) 
    { 

       /*************************/
       /*			*/
       /* initialization failed */
       /*			*/
       /*************************/
       free(fep);
       fep->common.imerrno = IMKeymapInitializeError;
       return NULL;
    }
    else
    {
       return (IMFep)fep;
    }

} 
/*-----------------------------------------------------------------------*
 *	end of IMInitialize 
 *-----------------------------------------------------------------------*/
