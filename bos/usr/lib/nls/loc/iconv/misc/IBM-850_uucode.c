static char sccsid[] = "@(#)98  1.5  src/bos/usr/lib/nls/loc/iconv/misc/IBM-850_uucode.c, cmdiconv, bos411, 9428A410j 8/26/93 03:56:22";
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

#define	ENC(c) ((c) ? ((c) & 0x3f) + ' ': '`')

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
	int		d_cnt, o_bytesneed, tail, i;


	if ((cd == NULL) || (cd == -1)) {
		errno =	EBADF; return -1;
	}
	if (inbuf == NULL) return 0;

	e_in  = (in  = *inbuf)  + *inbytesleft;
	e_out =	(out = *outbuf)	+ *outbytesleft;

	while (in < e_in) {
		if (e_in - in >= 45) {
			d_cnt =	45;
			o_bytesneed = 62;
			tail = 0;
		}
		else {
			if (*inbytesleft > 45) {
				*inbuf = in;
				*outbuf	= out;
				*inbytesleft = e_in - in;
				*outbytesleft =	e_out -	out;
				errno =	EINVAL;	return -1;
			}
			d_cnt =	e_in - in;
			if ((tail = d_cnt % 3) != 0)
				o_bytesneed = (d_cnt / 3 + 1) *	4 + 2;
			else
				o_bytesneed = (d_cnt / 3) * 4 +	2;
		}
		if (e_out - out	< o_bytesneed) {
			*inbuf = in;
			*outbuf	= out;
			*inbytesleft = e_in - in;
			*outbytesleft =	e_out -	out;
			errno =	E2BIG; return -1;
		}
		*out++ = ENC(d_cnt);
		for (i = 0; i <	d_cnt -	tail; i	+= 3) {
			out[0] = ENC(in[i] >> 2	& 0x3f);
			out[1] = ENC(in[i] << 4	& 0x30 
			       | in[i + 1] >> 4 & 0x0f);
			out[2] = ENC(in[i + 1] << 2 & 0x3c 
			       | in[i + 2] >> 6 & 0x03);
			out[3] = ENC(in[i + 2] & 0x3f);
			out += 4;
		}
		in += d_cnt - tail;
		switch (tail) {
		case 2 :	out[0] = ENC(in[0] >> 2	& 0x3f);
				out[1] = ENC(in[0] << 4	& 0x30 
				       | in[1]     >> 4 & 0x0f);
				out[2] = ENC(in[1] << 2	& 0x3c);
				out[3] = ENC(0x00);
				in += 2;
				out += 4;
				break;
		case 1 :	out[0] = ENC(in[0] >> 2	& 0x3f);
				out[1] = ENC(in[0] << 4	& 0x30);
				out[2] = ENC(0x00);
				out[3] = ENC(0x00);
				in += 1;
				out += 4;
		default	:	break;
		}
		*out++ = '\n';
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
