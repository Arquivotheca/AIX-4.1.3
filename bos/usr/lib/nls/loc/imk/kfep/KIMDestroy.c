static char sccsid[] = "@(#)54	1.2  src/bos/usr/lib/nls/loc/imk/kfep/KIMDestroy.c, libkr, bos411, 9428A410j 7/21/92 00:35:05";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		KIMDestroy.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991
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
 *  Module:       KIMDestroy.c
 *
 *  Description:  Korean Input Method Destruction
 *
 *  Functions:    KIMDestroy()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <sys/types.h>
#include <im.h>
#include <imP.h>
#include "kfep.h"		/* Korean Input Method header file */
#include "kedconst.h"
#include "ked.h"

/*-----------------------------------------------------------------------*
 *	Begining of Procedure.
 *-----------------------------------------------------------------------*/
void KIMDestroy(obj)
KIMOBJ obj;
{
    /**********************/
    /*			  */
    /* External Reference */
    /*			  */
    /**********************/
    extern void		kedClose();
    char 		**aux_str, **aux_atr;
    register int  	i;
    KIMED    		*kimed;

    /**************/
    /*		  */
    /* Local Copy */
    /*		  */
    /**************/
    kimed = (KIMED *)obj->kimed;

    kedClose(kimed);

    /****************************************************/
    /* string/attribute buffers for Text Info. 		*/
    /****************************************************/
    free(obj->textinfo.text.str);
    free(obj->textinfo.text.att);

    /****************************************************/
    /* buffers for Aux Info. 				*/
    /****************************************************/
    for(i = 0; i < KIM_MSGTXT_MAXROW; i++) {
	free(obj->auxinfo.message.text[i].str);
	free(obj->auxinfo.message.text[i].att);
    }
    free(obj->auxinfo.message.text); 

    /****************************************************/
    /* string buffer for GetString ioctl 		*/
    /****************************************************/
    free(obj->string.str);

    /****************************************************/
    /* indicator string buffer 				*/
    /****************************************************/
    free(obj->indstr.str);

    /****************************************************/
    /* output buffer for Process 			*/
    /****************************************************/
    free(obj->outstr);

    free((caddr_t)obj->output.data);

    /****************************************************/
    /* KIM object structure 				*/ 
    /****************************************************/
    free(obj);

} /* end of destroy */
/*-----------------------------------------------------------------------*
 *	End of Procedure.
 *-----------------------------------------------------------------------*/
