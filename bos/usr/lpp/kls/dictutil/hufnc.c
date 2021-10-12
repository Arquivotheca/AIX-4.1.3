static char sccsid[] = "@(#)15	1.1  src/bos/usr/lpp/kls/dictutil/hufnc.c, cmdkr, bos411, 9428A410j 5/25/92 14:45:25";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hufnc.c
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
 *  Module:       hufnc.c
 *
 *  Description:  check input file name.
 *
 *  Functions:    hufnc()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include Standard.
 */
#include "hut.h"
#include <stdio.h>
#include <memory.h>


int hufnc(fname)
uchar    *fname;         /* file name    */
{
    uchar        wname[1024];    /* work name    */
    int         data_len;       /* length of data       */
    int         i;              /* work variable of integer     */

    /* set of data length       */
    data_len = strlen(fname);
    if(data_len < 1) {
      return(IUFAIL);
    };

    /* find blank code from top to last */
    i = 0;
    while(TRUE) {
      if( (*(fname+i) == U_2SPACEH) && (*(fname+i+1) == U_2SPACEL) ) {
	/* DBCS blank code    */
	i = i + 2;
	continue;
      } else if(*(fname+i) == 0x20) {
	/* SBCS blank code    */
	i++;
	continue;
      };
      break;
    };

    /* erase blank data */
    strcpy(wname,(char *)(fname+i));
    strcpy(fname,wname);

    /* find blank code from top to last */
    i = 0;
    while(*(fname+i) != NULL) {
      if( (*(fname+i) == U_2SPACEH) && (*(fname+i+1) == U_2SPACEL) ) {
	/* DBCS blank code    */
	return(IUFAIL);
      } else if(*(fname+i) == U_1SPACE) {
	/* SBCS blank code    */
	return(IUFAIL);
      };
      i++;
    };

    /* check of last character  */
    data_len = strlen(fname) - 1;
    if(*(fname+data_len) == '/') {
      return(IUFAIL);
    };


    return(IUSUCC);
};
