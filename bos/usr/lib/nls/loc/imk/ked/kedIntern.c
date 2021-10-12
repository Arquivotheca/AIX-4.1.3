static char sccsid[] = "@(#)71	1.1  src/bos/usr/lib/nls/loc/imk/ked/kedIntern.c, libkr, bos411, 9428A410j 5/25/92 15:42:30";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		kedIntern.c
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
 *  Component:    Korean IM ED  
 *
 *  Module:       kedIntern.c  
 *
 *  Description:  
 *
 * 
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <im.h>
#include <imP.h>
#include "kedconst.h"
#include "ked.h"

/*-----------------------------------------------------------------------*
*      Defefine of REALLOC
*-----------------------------------------------------------------------*/
#define ALLOC_UNIT      512
#ifdef  REALLOC
#undef  REALLOC
#endif
#define REALLOC(s, l)   ((s) ? realloc(s, l) : malloc(l))

/*-----------------------------------------------------------------------*
*	Beginning of placestr
*-----------------------------------------------------------------------*/
void    placestr(IMBuffer *imb, unsigned char *str, int len)
{
        if (imb->len + len > imb->siz) {
                imb->siz = ((imb->len + len) / ALLOC_UNIT + 1) * ALLOC_UNIT;
                imb->data = (unsigned char *)REALLOC(imb->data, imb->siz);
        }
        memcpy(&imb->data[imb->len], str, len);
        imb->len += len;
	memset(&imb->data[imb->len], NULL, imb->siz - len);
}

/*-----------------------------------------------------------------------*
*	Beginning of placechar
*-----------------------------------------------------------------------*/
void    placechar(IMBuffer *imb, unsigned char c)
{
        if (imb->len >= imb->siz) {
                imb->siz += ALLOC_UNIT;
                imb->data = (unsigned char *)REALLOC(imb->data, imb->siz);
        }
        imb->data[imb->len++] = c;
}

