/* @(#)68	1.3  src/bos/usr/lib/nls/loc/jim/jed/jedClose.c, libKJI, bos411, 9428A410j 6/6/91 11:00:20 */
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include "jedexm.h"
#include "jed.h"
#include "jedint.h"

/*
 *	jedClose()
 */
int     jedClose(FEPCB *fepcb)
{
	(void)exkjterm(fepcb->kcb);
	free(fepcb->kcb->string);

	(void)exkjclos(fepcb->kcb);
	free(fepcb);

	return KP_OK;
}
