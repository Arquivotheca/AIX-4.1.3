static char sccsid[] = "@(#)27  1.3.3.1  src/bos/kernext/rcm/gsctrace.c, rcm, bos41J, 9520A_all 5/3/95 14:02:15";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Performance Trace
 *
 * FUNCTIONS:
 *		gsctrace
 *		gscdtrace
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989-1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>                 /* includes for all lft related data */
#include <sys/trchkid.h>
#include <sys/syspest.h>
#include "xmalloc_trace.h"

int
gsctrace(int component, int trace_id)
{
	trchkt(HKWD_DISPLAY_RCM_D | ((component & 0xff) << 4) | trace_id);
	return 1;
}

int
gscdtrace(int component, int trace_id)
{
	trchkt(HKWD_DISPLAY_RCM_D | ((component & 0xff) << 4) | trace_id);
	return 1;
}
