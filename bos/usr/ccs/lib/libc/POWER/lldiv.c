static char sccsid[] = "@(#)06	1.1  src/bos/usr/ccs/lib/libc/POWER/lldiv.c, libccnv, bos411, 9428A410j 9/10/93 13:42:16";
/*
 * COMPONENT_NAME: (LIBCCNV) LIB C CoNVersion
 *
 * FUNCTIONS: lldiv
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <limits.h>	/* LONGLONG_MIN, LONGLONG_MAX */
#include <stdlib.h>	/* lldiv_t, lldiv */

lldiv_t
lldiv( long long dvd, long long dvs )
{
lldiv_t		answer;

	if ( dvs == 0LL ){
		if ( dvd >= 0LL )
			answer.quot = LONGLONG_MAX;
		else
			answer.quot = LONGLONG_MIN;
		answer.rem  = 0LL;
	}else if( (dvs == -1LL) && (dvd == LONGLONG_MIN) ){
		answer.quot = LONGLONG_MIN;
		answer.rem  = 0LL;
	}else{
		answer.quot = dvd / dvs;
		answer.rem  = dvd % dvs;
	};
	return answer;
}
