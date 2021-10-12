static char sccsid[] = "@(#)60	1.1  src/bos/usr/lib/nls/loc/imk/kfep/KIMProcess.c, libkr, bos411, 9428A410j 5/25/92 15:40:36";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		KIMProcess.c
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
 *  Module:       KIMProcess.c
 *
 *  Description:  Korean Input Method Processing.
 *
 *  Functions:    KIMProcess()
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
 *       Beging of Procedure.
 *-----------------------------------------------------------------------*/

int          KIMProcess(obj, keysym, state, str, len)
KIMOBJ   	obj;
unsigned int    keysym;
unsigned int    state;
caddr_t  	*str;
unsigned int    *len;
{
        if (IsModifierKey(keysym)) {
                *str = 0;
                *len = 0;
                return obj->textauxstate;
        }

        if (state & Mod5Mask) {
                if (IsKeypadKey(keysym))
                        state ^= ShiftMask;
                state &= ~Mod5Mask;
        }

        state &= KIM_VALIDBITS;

        obj->output.len = 0;
        if (kimfilter(obj, keysym, state, &obj->output) == IMInputNotUsed)
                kimlookup(obj, keysym, state, &obj->output);
        *len = makeoutputstring(obj);
        *str = obj->outstr;
        return obj->textauxstate;
}

/*-----------------------------------------------------------------------*
 *       End of Procedure.
 *-----------------------------------------------------------------------*/
