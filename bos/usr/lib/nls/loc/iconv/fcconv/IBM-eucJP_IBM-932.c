static char sccsid[] = "@(#)26  1.8  src/bos/usr/lib/nls/loc/iconv/fcconv/IBM-eucJP_IBM-932.c, cmdiconv, bos411, 9428A410j 8/24/93 07:12:28";
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
#include <iconvP.h>
#include "fcconv.h"
#include <iconv932.h>
#include "ibmsel.h"

/*
 *   NAME:	_iconv_exec
 *
 *   FUNCTION:	Conversion.
 *
 *   RETURNS:	>= 0	- Number of substitutions.
 *		-1	- Error.
 *
 *   NOTES:	This routine returns only 0 on successful condition,
 *		does not actually count the number of substitutions.
 */

static	size_t	_iconv_exec (
	_LC_fcconv_iconv_t *cd, 
	uchar_t **inbuf,  size_t *inbytesleft,
	uchar_t **outbuf, size_t *outbytesleft) {

	uchar_t		*in, *out, *e_in, *e_out, c1, c2;
	ushort_t	code;
	int		i, low, high, found, err_flag = FALSE;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return -1;
	}
        if (inbuf == NULL) return 0;

        e_in  = (in  = *inbuf)  + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
	while (in < e_in) {
		switch (indexEUC[in[0]]) {

		case 0:	if (out >= e_out) {
				errno = E2BIG; err_flag = TRUE; break;
			}
			*out++ = *in++;
			break;

		case 1:	if (e_in - in < 2) {
				errno = EINVAL; err_flag = TRUE; break;
			}
			if (in[1] < 0xa1 || in[1] == 0xff) {
				errno = EILSEQ; err_flag = TRUE; break;
			}
			if (e_out - out < 2) {
				errno = E2BIG; err_flag = TRUE; break;
			}
			if (in[0] == 0xf4 && in[1] > 0xa6) {
				out[0] = IBM932_D_SUBCHAR_1;
				out[1] = IBM932_D_SUBCHAR_2;
				out += 2;
				in += 2;
				continue;
			}
			c1 = in[0];
			c2 = in[1] & 0x7f;
			c2 += c1 & 1 ? 0x1f : 0x7d;

			/*
			 *	JIS to SJIS
			 */

			if (c2 >= 0x7f)
				c2++;
			if (c1 < 0xf5) {
				c1 = (c1 - 0xa1 >> 1) + 0x81;
				if (c1 >= 0xa0) c1 += 0x40;
			}
			else {
				c1 = (c1 - 0xf5 >> 1) + 0xf0;
			}

			/*
			 *	SJIS to IBM-932
			 */

			low = 0;
			high = (sizeof (SJIStoCP932) >> 1) /
				sizeof (SJIStoCP932[0][FROM]);
			code = (c1 << 8 & 0xff00) + (c2 & 0xff);
			while (low <= high) {
				i = low + high >> 1;
				if (code < SJIStoCP932[i][FROM]) 
					high = i - 1;
				else if (code > SJIStoCP932[i][FROM])
					low = i + 1;
				else {
                                        code = SJIStoCP932[i][TO];
                                        c1 = code >> 8 & 0xff;
                                        c2 = code & 0xff;
                                        break;
				}
			}
			out[0] = c1;
			out[1] = c2;
			in  += 2;
			out += 2;
			break;

		case 2:	if (e_in - in < 2) {
				errno = EINVAL; err_flag = TRUE; break;
			}
                        if (in[1] < 0xa1 || in[1] == 0xff) {
				errno = EILSEQ; err_flag = TRUE; break;
                        }
			if (out >= e_out) {
				errno = E2BIG; err_flag = TRUE; break;
			}
			if (in[1] < 0xe0) {
				*out++ = in[1];
			}
			else {
				switch (in[1]) {
				case 0xE0: *out++ = 0x80; break;
				case 0xE1: *out++ = 0xa0; break;
				case 0xE2: *out++ = 0xfd; break;
				case 0xE3: *out++ = 0xfe; break;
				case 0xE4: *out++ = 0xff; break;
				default  : *out++ = IBM932_SUBCHAR;
				}
			}
			in += 2;
			break;

		case 3:	if (e_in - in < 3) {
				errno = EINVAL; err_flag = TRUE; break;
			}
			if (in[1]  < 0xa1 || in[2]  < 0xa1 ||
			    in[2] == 0xff || in[1] == 0xff) {
				errno = EILSEQ; err_flag = TRUE; break;
			}
			if (e_out - out < 2) {
				errno = E2BIG; err_flag = TRUE; break;
			}
			if (in[1] >= 0xf5) {
				c1 = in[1];
				c2 = in[2] & 0x7f;

				/*
				 *	JIS to SJIS
				 */

				c2 += c1 & 1 ? 0x1f : 0x7d;
				if (c2 >= 0x7f) c2++;
				out[0] = (c1 - 0xf5 >> 1) + 0xf5;
				out[1] = c2;
			}
			else if (in[1] == 0xf3 || in[1] == 0xf4) {
				code = IBMSELECTED_EUCto932
					[(in[1] - 0xf3) * 0x5e + in[2] - 0xa1];
				out[0] = code >> 8 & 0xff;
				out[1] = code & 0xff;
			}
			else {
				found = FALSE;
				low = 0;
				high = (sizeof (IBMSEL0212_EUCto932) >> 1)
				      / sizeof (IBMSEL0212_EUCto932[0][FROM]);
				code = (in[1] << 8 & 0xff00) + (in[2] & 0xff);
				while (low <= high) {
					i = low + high >> 1;
					if (code < IBMSEL0212_EUCto932[i][FROM])
						high = i - 1;
					else if (code > IBMSEL0212_EUCto932[i][FROM])
						low = i + 1;
					else {
						code = IBMSEL0212_EUCto932[i][TO];
						found = TRUE;
						break;
					}
				}
				if (found) {
					out[0] = code >> 8 & 0xff;
					out[1] = code & 0xff;
				}
				else {
					out[0] = IBM932_D_SUBCHAR_1;
					out[1] = IBM932_D_SUBCHAR_2;
				}
			}
			in  += 3;
			out += 2;
			break;

		default:errno = EILSEQ; err_flag = TRUE; break;
		}
		if (err_flag) break;
	}
	*inbuf        = in;
        *outbuf       = out;
	*inbytesleft  = e_in  - in;
	*outbytesleft = e_out - out;

	if (err_flag) return -1;
	else          return 0;
}

/*
 *   NAME:	_iconv_close
 *
 *   FUNCTION:	Termination.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

static int	_iconv_close (iconv_t cd) {

	if ((cd != NULL) && (cd != -1)) {
		free (cd);
		return 0;
	}
	else {	errno = EBADF;
		return -1;
	}
}

/*
 *   NAME:	init
 *
 *   FUNCTION:	Initialization.
 *
 *   RETURNS:	Pointer to a descriptor, or -1 if error.
 */

static	_LC_fcconv_iconv_t	*init (
	_LC_core_iconv_t	*core_cd, 
	uchar_t			*toname,
	uchar_t			*fromname) {

        _LC_fcconv_iconv_t	*cd;

        if ((cd = (_LC_fcconv_iconv_t*)malloc (
		sizeof (_LC_fcconv_iconv_t))) == NULL)
		return (_LC_fcconv_iconv_t*)-1;
	cd->core = *core_cd;
	return cd;
}

/*
 *   NAME:	instantiate
 *
 *   FUNCTION:	Instantiation.
 *
 *   RETURNS:	Pointer to a descriptor.
 */

_LC_core_iconv_t	*instantiate (void) {

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
