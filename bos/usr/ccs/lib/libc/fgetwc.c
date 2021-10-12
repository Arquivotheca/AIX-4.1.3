static char sccsid[] = "@(#)00	1.4  src/bos/usr/ccs/lib/libc/fgetwc.c, libcio, bos411, 9428A410j 2/26/91 13:40:59";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: fgetwc 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>


wint_t fgetwc(FILE *stream)
{
        return (getwc(stream));
}
