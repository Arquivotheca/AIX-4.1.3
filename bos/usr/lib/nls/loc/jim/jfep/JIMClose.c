static char sccsid[] = "@(#)02	1.5.1.2  src/bos/usr/lib/nls/loc/jim/jfep/JIMClose.c, libKJI, bos411, 9428A410j 1/7/93 01:33:42";
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS :		JIMClose
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992
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
void	JIMClose(JIMFEP fep)
{
	_IMCloseKeymap(fep->immap);
	if (fep->cd)
		iconv_close(fep->cd);

	CloseHostCodeConverter();

	CloseSdict(&(fep->sdictdata));
	CloseUdict(&(fep->udictinfo));
	CloseFdict(&(fep->fdictinfo));

	free(fep);
}
