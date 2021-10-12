static char sccsid[] = "@(#)06	1.1  src/bos/usr/ccs/lib/libc/__getmbcurmax.c, libccppc, bos411, 9428A410j 1/12/93 12:41:17";
/* 
 * COMPONENT_NAME: LIBCCPPC
 * 
 * FUNCTIONS: __getmbcurmax
 *
 * ORIGINS: 71, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

/*
 * Routine which return the maximum number of bytes 
 * in a multibyte character aqccording to the current
 * locale. MB_CUR_MAX is defined as a call to this 
 * in stdlib.h.
 * 
 * Written to avoid referencing __lc_charmap in stdlib.h
 */

#include <sys/localedef.h>

int __getmbcurmax(void)
{
	return (__OBJ_DATA(__lc_charmap)->cm_mb_cur_max);
}
