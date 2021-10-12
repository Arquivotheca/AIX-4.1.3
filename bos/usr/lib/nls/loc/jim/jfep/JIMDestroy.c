static char sccsid[] = "@(#)04	1.4.1.2  src/bos/usr/lib/nls/loc/jim/jfep/JIMDestroy.c, libKJI, bos411, 9428A410j 5/18/93 05:33:52";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : JIMDestroy.c
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <stdlib.h>
#include "imjimP.h"		/* Japanese Input Method header file */

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/
void JIMDestroy(obj)
JIMOBJ obj;
{
    extern void jedClose();
    char **aux_str, **aux_atr;
    int  i;

    SetCurrentSDICTDATA(&(((JIMfep *)(obj->imobject.imfep))->sdictdata));
    SetCurrentUDICTINFO(&(((JIMfep *)(obj->imobject.imfep))->udictinfo));
    SetCurrentFDICTINFO(&(((JIMfep *)(obj->imobject.imfep))->fdictinfo));

    /* destroy text and auxiliary windows and free data */
    jimclear(obj, 1);

    /**********************/
    /* have editor closed */
    /**********************/
    jedClose(obj->jedinfo.jeid);

    /**************************************************/
    /* frees all resources allocated at creation time */
    /**************************************************/
    /* echo, echo attribute */
    free(obj->jedinfo.echobufs);
    free(obj->jedinfo.echobufa);

    /* aux buffers */
    aux_str = obj->jedinfo.auxbufs;
    for(i = 0; i < JIM_AUXROWMAX; i++) 
	free(*aux_str++);
    free(obj->jedinfo.auxbufs);
    aux_atr = obj->jedinfo.auxbufa;
    for(i = 0; i < JIM_AUXROWMAX; i++) 
	free(*aux_atr++);
    free(obj->jedinfo.auxbufa);

    /* string/attribute buffers for Text Info. */
    free(obj->textinfo.text.str);
    free(obj->textinfo.text.att);

    /* buffers for Aux Info. */
    for(i = 0; i < JIM_AUXROWMAX; i++) {
	free(obj->auxinfo.message.text[i].str);
	free(obj->auxinfo.message.text[i].att);
    }
    free(obj->auxinfo.message.text); 

    /* string buffer for GetString ioctl */
    free(obj->string.str);

    /* indicator string buffer */
    free(obj->indstr.str);

    /* output buffer for Process */
    free(obj->outstr);

    if (obj->output.data)
	free(obj->output.data);

    /* JIM object structure */ 
    free(obj);

} /* end of destroy */
