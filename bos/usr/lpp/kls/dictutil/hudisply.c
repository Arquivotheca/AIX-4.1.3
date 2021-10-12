static char sccsid[] = "@(#)14	1.1  src/bos/usr/lpp/kls/dictutil/hudisply.c, cmdkr, bos411, 9428A410j 5/25/92 14:45:14";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudisply.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hudisply.c
 *
 *  Description:  display the string at given line and column.
 *
 *  Functions:    hudisply()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include Standard.
 */
#include <stdio.h>
#include <string.h>
#include "hut.h"


int hudisply(udcb,line,col,data,data_len)
UDCB    *udcb;
short   line;
short   col;
char    *data;
short   data_len;
{
    char        format[80];
    char        disarea[U_MAXCOL];
    int         i;
    int         drlen;	/* data real length */

    if( (line < 0) || (udcb->ymax < line) ||
        (col < 0)  || (U_MAXCOL + udcb->xoff < col)         ) {
      return( -1 );
    };
    data_len = MIN( data_len , (U_MAXCOL + udcb->xoff - col) );
    drlen = strlen(data);
    memcpy(disarea,data,data_len);
    if(data_len > drlen) {
      for(i=drlen;i<data_len;i++) {
        disarea[i] = 0x20;
      };
    };

    sprintf(format,"%%B.%ds",data_len);
    CURSOR_MOVE(line,col);
    printf(format,disarea);
    return( 0 );
}
