static char sccsid[] = "@(#)17	1.1  src/bos/usr/lpp/kls/dictutil/hugetc.c, cmdkr, bos411, 9428A410j 5/25/92 14:45:44";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hugetc.c
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
 *  Module:       hugetc.c
 *
 *  Description:  get character for user dictionary utility.
 *
 *  Functions:    hugetc()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include file.
 */
#include "huke.h"

int hugetc()
{
  int           hugetkey();

  return( hugetkey(GET_CHAR) );

};
