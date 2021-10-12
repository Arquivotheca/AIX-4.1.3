static char sccsid[] = "@(#)09  1.5  src/bos/usr/ccs/lib/libiconv/ascii.c, libiconv, bos411, 9428A410j 9/16/93 08:33:03";
/*
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:		_ascii_exec
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include "iconvP.h"

static	uchar_t	valid[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/*
 *   NAME:	_ascii_exec
 *
 *   FUNCTION:	Conversion in ascii range.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

size_t	_ascii_exec (
	uchar_t **inbuf,  size_t *inbytesleft, 
	uchar_t **outbuf, size_t *outbytesleft) {

	uchar_t		*in, *out, *e_in, *e_out;
	size_t		ret_value;


	e_in  = (in  = *inbuf)  + *inbytesleft;
	e_out = (out = *outbuf) + *outbytesleft;

	while (TRUE) {
		if (in >= e_in) {
			ret_value = 0; break;
		}
		if (out >= e_out) {
			errno = E2BIG;
			ret_value = -1; break;
		}
		if (!valid[*in]) {
			errno = EILSEQ;
			ret_value = -1; break;
		}
		*out++ = *in++;
	}
	*inbuf        = in;
	*outbuf       = out;
	*inbytesleft  = e_in - in;
	*outbytesleft = e_out - out;

	return ret_value;
}
