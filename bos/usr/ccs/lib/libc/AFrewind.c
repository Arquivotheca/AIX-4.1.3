static char sccsid[] = "@(#)44	1.1  src/bos/usr/ccs/lib/libc/AFrewind.c, libcgen, bos411, 9428A410j 12/14/89 17:42:52";
/*
 * LIBIN: AFrewind
 *
 * ORIGIN: ISC
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Move to the beginning of an Attribute File.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/AFdefs.h>

AFrewind(AFILE_t af)
{
	fseek(af->AF_iop,0L,0);
}
