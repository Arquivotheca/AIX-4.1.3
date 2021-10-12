static char sccsid[] = "@(#)01  1.5  src/bos/usr/lib/nls/loc/iconv/misc/uucode_IBM-850.c, cmdiconv, bos411, 9428A410j 8/26/93 03:56:55";
/*
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS:		_iconv_exec
 *			_iconv_close
 *			init
 *			instantiate
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

#include <stdlib.h>
#include <iconv.h>
#include "misc.h"

#define	DEC(c)	(((c) -	' ') & 077)

/*
 *   NAME:      _iconv_exec
 *
 *   FUNCTION:  Conversion.
 *
 *   RETURNS:   >= 0    - Number of substitution.
 *              -1      - Error.
 *
 *   NOTE:      This routine returns only 0 value on successful condition,
 *              does not actually count the number of substitutions.
 */

static	size_t	_iconv_exec (
	_LC_misc_iconv_t *cd, 
	uchar_t **inbuf,  size_t *inbytesleft,
	uchar_t **outbuf, size_t *outbytesleft) {

	uchar_t		*in, *e_in, *out, *e_out;
	int		element, i_bytesneed, i;


	if ((cd == NULL) || (cd == -1)) {
		errno =	EBADF; return -1;
	}
	if (inbuf == NULL) return 0;

	e_in  = (in  = *inbuf)  + *inbytesleft;
	e_out =	(out = *outbuf)	+ *outbytesleft;

	while (in < e_in) {
		if ((element = DEC(in[0])) > 45) {
			*inbuf = in;
			*outbuf	= out;
			*inbytesleft = e_in - in;
			*outbytesleft =	e_out -	out;
			errno =	EILSEQ;	return -1;
		}
		i_bytesneed = (element % 3 != 0) ? 
			(element / 3 + 1) * 4 +	2 : (element / 3) * 4 +	2;
		if (e_in - in <	i_bytesneed) {
			*inbuf = in;
			*outbuf	= out;
			*inbytesleft = e_in - in;
			*outbytesleft =	e_out -	out;
			errno =	EINVAL;	return -1;
		}
		if (in[i_bytesneed - 1]	!= '\n') {
			*inbuf = in;
			*outbuf	= out;
			*inbytesleft = e_in - in;
			*outbytesleft =	e_out -	out;
			errno =	EILSEQ;	return -1;
		}
		if (e_out - out	< element) {
			*inbuf = in;
			*outbuf	= out;
			*inbytesleft = e_in - in;
			*outbytesleft =	e_out -	out;
			errno =	E2BIG; return -1;
		}
		in++;
		while (element > 2) {
			out[0] = DEC(in[0]) << 2 | DEC(in[1]) >> 4;
			out[1] = DEC(in[1]) << 4 | DEC(in[2]) >> 2;
			out[2] = DEC(in[2]) << 6 | DEC(in[3]);
			element	-= 3;
			in += 4;
			out += 3;
		}
		switch (element) {
		case 2 :
			out[0] = DEC(in[0]) << 2 | DEC(in[1]) >> 4;
			out[1] = DEC(in[1]) << 4 | DEC(in[2]) >> 2;
			in += 4;
			out += 2;
			break;
		case 1 :
			out[0] = DEC(in[0]) << 2 | DEC(in[1]) >> 4;
			in += 4;
			out++;
			break;
		default	:
			break;
		}
		in++;
	}
	*inbuf        = in;
	*outbuf	      = out;
	*inbytesleft  = e_in - in;
	*outbytesleft =	e_out -	out;

	return 0;
}

/*
 *   NAME:      _iconv_close
 *
 *   FUNCTION:  Termination.
 *
 *   RETURNS:   0       - Successful completion.
 *              -1      - Error.
 */

static	int	_iconv_close (iconv_t cd) {

	if ((cd != NULL) && (cd != -1)) {
		free (cd);
		return 0;
	}
	else {
		errno =	EBADF;
		return -1;
	}
}

/*
 *   NAME:      init
 *
 *   FUNCTION:  Initialization.
 *
 *   RETURNS:   Pointer to a descriptor, or -1 if error.
 */

static	_LC_misc_iconv_t	*init (
	_LC_core_iconv_t	*core_cd, 
	uchar_t			*toname,
	uchar_t			*fromname) {

	_LC_misc_iconv_t	*cd;

	if ((cd = malloc (
		sizeof (_LC_misc_iconv_t))) == NULL)
		return (_LC_misc_iconv_t*)-1;
	cd->core = *core_cd;
	return cd;
}

/*
 *   NAME:      instantiate
 *
 *   FUNCTION:  Instantiation method of this converter.
 *
 *   RETURNS:   Pointer to the descriptor.
 */

_LC_core_iconv_t	*instantiate(void) {

	static _LC_core_iconv_t	cd;

	cd.hdr.__magic   = _LC_MAGIC;
	cd.hdr.__version = _LC_VERSION;
	cd.hdr.__type_id = _LC_ICONV;
	cd.hdr.__size    = sizeof (_LC_core_iconv_t);
	cd.init          = (_LC_core_iconv_t*(*)())init;
	cd.exec          = (size_t(*)())_iconv_exec;
	cd.close         = (int(*)())_iconv_close;
	return &cd;
}
