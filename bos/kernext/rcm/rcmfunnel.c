static char sccsid[] = "@(#)53	1.1  src/bos/kernext/rcm/rcmfunnel.c, rcm, bos41J, 9513A_all 3/16/95 16:45:58";

/*
 *   COMPONENT_NAME: (rcm) Rendering Context Manager Funnelling Code
 *
 *   FUNCTIONS: enter_funnel_nest
 *		exit_funnel_nest
 *		am_i_funnelled
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  NOTE:  This gets compiled with _KERNSYS defined by the
 *  the Makefile.  Else, sys/processor.h won't define the
 *  symbols we need.
 *
 *  Makefile also has special include directives to pick up
 *  thread.h from the innards of the kernel.
 *
 *  Permission for these irregularities has been granted by
 *  the kernel people.
 */

#ifndef  _KERNSYS
#define  _KERNSYS
#endif

#include <sys/processor.h>
#include <sys/thread.h>

#define  ILLEGAL_PROCESSOR  -1

int enter_funnel_nest ()
{
    return  switch_cpu (MP_MASTER, SET_PROCESSOR_ID) ? 1 : 0;
}


int exit_funnel_nest ()
{
    (void) switch_cpu (0, RESET_PROCESSOR_ID);

    return  0;
}


int am_i_funnelled ()
{
	return  switch_cpu (ILLEGAL_PROCESSOR, SET_PROCESSOR_ID) ? 1 : 0;
}
