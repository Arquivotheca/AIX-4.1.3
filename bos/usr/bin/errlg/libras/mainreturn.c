static char sccsid[] = "@(#)94	1.1  src/bos/usr/bin/errlg/libras/mainreturn.c, cmderrlg, bos411, 9428A410j 3/2/93 09:01:42";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: mainreturn
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <setjmp.h>

jmp_buf eofjmp;

mainreturn(rv)
{

	longjmp(eofjmp,rv);
}

