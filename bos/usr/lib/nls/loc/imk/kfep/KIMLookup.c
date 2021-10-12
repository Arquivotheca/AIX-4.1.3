static char sccsid[] = "@(#)58	1.1  src/bos/usr/lib/nls/loc/imk/kfep/KIMLookup.c, libkr, bos411, 9428A410j 5/25/92 15:40:17";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		KIMLookup.c
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
 *  Module:       KIMLookup.c
 *
 *  Description:  Korean Input Method Look Up Process.
 *
 *  Functions:    KIMLookup()
 *                kimlookup()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*-----------------------------------------------------------------------*
*       Include files
*-----------------------------------------------------------------------*/

#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <im.h>
#include <imP.h>
#include "kimerrno.h"
#include "kfep.h"               /* Korean Input Method header file */
#include "kedconst.h"
#include "ked.h"

/*-----------------------------------------------------------------------*
 *       Begining of procedure
 *-----------------------------------------------------------------------*/

int     KIMLookup(obj, keysym, state, str, len)
KIMOBJ          obj;
unsigned int    keysym;
unsigned int    state;
caddr_t 	*str;
unsigned int    *len;
{
extern  void 	kimlookup();

        if (state & Mod5Mask) {
                if (IsKeypadKey(keysym))
                        state ^= ShiftMask;
                state &= ~Mod5Mask;
        }

        state &= KIM_VALIDBITS;

        obj->output.len = 0;
        (void)kimlookup(obj, keysym, state, &obj->output);
        *len = makeoutputstring(obj);
        *str = obj->outstr;
        return (*len != 0) ? IMReturnString : IMReturnNothing;
}

/*-----------------------------------------------------------------------*
 *       Subroutine of KIMLookup
 *-----------------------------------------------------------------------*/

void     kimlookup(obj, key, shift, imb)
KIMOBJ   	obj;
unsigned int    key;
unsigned int    shift;
IMBuffer        *imb;
{
IMFep		imfep;
KIMED		*kimed;

	/**************/
	/*	      */
	/* Local Copy */
	/*	      */
	/**************/
	imfep =          obj->imobject.imfep;
	kimed = (KIMED*) obj->kimed;

        kedLookup(kimed, ((KIMFEP)imfep)->immap, key, shift, imb);
}
