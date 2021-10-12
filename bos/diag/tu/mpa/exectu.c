static char sccsid[] = "@(#)42  1.1  src/bos/diag/tu/mpa/exectu.c, mpatu, bos411, 9428A410j 4/30/93 12:20:55";
/*
 *   COMPONENT_NAME: (MPADIAG) MP/A DIAGNOSTICS
 *
 *   FUNCTIONS: Function
 *		Unit
 *		exectu
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************

Function(s) Exec TU for HTX Exerciser and Diagnostics

Module Name :  exectu.c
SCCS ID     :  1.7

Current Date:  6/20/91, 10:14:44
Newest Delta:  1/19/90, 16:48:47

Function called by both the Hardware Exercise, Manufacturing Application,
and Diagnostic Application to invoke a Test Unit (TU).

If the "mfg" member of the struct within TUTYPE is set to INVOKED_BY_HTX,
then exectu() is being invoked by the HTX Hardware Exerciser so
test units know to look at variables in TUTYPE for values from the
rule file.  Else, the test units use pre-defined values while testing.

*****************************************************************************/
#include <stdio.h>
#include "mpa_tu.h"     /* note that this also includes hxihtx.h */

int exectu (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
{
   static int rc;
   int        i;
   extern int tu001();
   extern int tu002();
   extern int tu003();
   extern int tu004();
   extern int tu005();
   extern int tu006();
   extern int tu007();

   for ( i = 0; i<tucb_ptr->header.loop; i++ ) {

	switch(tucb_ptr->header.tu) {
		case  1:
			rc = tu001(fdes, tucb_ptr);
			break;

		case  2:
			rc = tu002(fdes, tucb_ptr);
			break;

		case  3:
			rc = tu003(fdes, tucb_ptr);
			break;

#ifndef DIAGS
		case  4:
			rc = tu004(fdes, tucb_ptr);
			break;
#endif

		case  5:
			rc = tu005(fdes, tucb_ptr);
			break;

		case  6:
			rc = tu006(fdes, tucb_ptr);
			break;

		case  7:
			rc = tu007(fdes, tucb_ptr);
			break;

		default:
			return(-1);
	}       /* end of switch based on tu number */

	if(rc) break;    /* get out of loop on failure */


   }
	return(rc);
   }
