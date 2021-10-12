static char sccsid[] = "@(#)36	1.1  src/bos/usr/ccs/lib/libc/AFclose.c, libcgen, bos411, 9428A410j 12/14/89 17:34:02";
/*
 * LIBIN: AFclose
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
 * FUNCTION: Close an Attribute File.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <stdio.h>
#include <stdlib.h>
#include <IN/standard.h>
#include <IN/AFdefs.h>

AFclose(AFILE_t af)
{
	fclose(af->AF_iop);
	free((void *)af->AF_datr);
	free((void *)af->AF_catr);
	free((void *)af->AF_dbuf);
	free((void *)af->AF_cbuf);
	free((void *)af);
}
