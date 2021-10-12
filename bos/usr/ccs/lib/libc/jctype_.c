static char sccsid[] = "@(#)27	1.1  src/bos/usr/ccs/lib/libc/jctype_.c, libcnls, bos411, 9428A410j 2/26/91 17:42:35";
/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library Conversion Functions
 *
 * FUNCTIONS: none.
 *
 * ORIGINS: 10
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*	jctype_ - character classification tables for KJI
 *	Two-level indexing (based on first, second bytes of character)
 *	is used to reduce data requirement.  This scheme relies on the
 *	fact that for most characters, the upper byte determines the
 *	characteristics.
 *						rcd  12-Apr-88
 *      These structures are isolated in include files so that they
 *	can be used by kernel extensions.  Bill Kennedy 19-Oct-89 
 */
#include <NLctype.h>
#include <sys/jctype0.h>
#include <sys/jctype1.h>

