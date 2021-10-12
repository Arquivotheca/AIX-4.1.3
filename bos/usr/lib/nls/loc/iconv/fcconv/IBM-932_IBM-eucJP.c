static char sccsid[] = "@(#)25  1.8  src/bos/usr/lib/nls/loc/iconv/fcconv/IBM-932_IBM-eucJP.c, cmdiconv, bos41B, 9506B 1/31/95 12:55:29";
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
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1995
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
 *   RETURNS:	>= 0	- Number of substitution.
 *		-1	- Error.
 *
 *   NOTE:	This routine returns only 0 value on successful condition,
 *		does not actually count the number of substitutions.
 */

static	size_t	_iconv_exec (
	_LC_fcconv_iconv_t *cd,
	uchar_t **inbuf,  size_t *inbytesleft,
	uchar_t **outbuf, size_t *outbytesleft) {

	uchar_t		*in, *out, *e_in, *e_out, c1, c2;
	ushort_t	pccode, euccode;
	int		low, high, i, err_flag = FALSE;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return -1;
	}
	if (inbuf == NULL) return 0;

        e_in  = (in  = *inbuf)  + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
	while (in < e_in) {
		switch (index932[in[0]]) {

		case 0:	if (out >= e_out) {
				errno = E2BIG; err_flag = TRUE; break;
			}
			*out++ = *in++;
			break;

		case 1:	if (e_out - out < 2) {
				errno = E2BIG; err_flag = TRUE; break;
			}
			out[0] = EUCSS2;
			out[1] = *in++;
			out += 2;
			break;

		case 2:	if (e_in - in < 2) {
				errno = EINVAL; err_flag = TRUE; break;
			}
			c1 = in[0];
			c2 = in[1];
			if (c2 < 0x40 || 0xfc < c2 || c2 == 0x7f) {
				errno = EILSEQ; err_flag = TRUE; break;
			}

                        /* kludge for apar 345x2,617,760 (defect 168376) */
                        if ((c1==0xfc) && (c2==0xfc)) {
                                out[0]=0xf4;
                                out[1]=0xfe;
                                out+=2;
                                in+=2;
                                break;
                                }

			/*
			 *	IBM-932 to SJIS
			 */

			low = 0;
			high = (sizeof (CP932toSJIS) >> 1) / 
				sizeof (CP932toSJIS[0][FROM]);
			pccode = (c1 << 8 & 0xff00) + (c2 & 0xff);
			while (low <= high) {
				i = low + high >> 1;
				if (pccode < CP932toSJIS[i][FROM])
					high = i - 1;
				else if (pccode > CP932toSJIS[i][FROM])
					low = i + 1;
				else {
					pccode = CP932toSJIS[i][TO];
					c1 = pccode >> 8 & 0xff;
					c2 = pccode & 0xff;
					break;
				}
			}
			if (0xfa <= c1) {
				if (e_out - out < 3) {
					errno = E2BIG; err_flag = TRUE;
					break;
				}
				if (c2 >= 0x80) c2--;
				euccode = IBMSELECTED_932toEUC
					[(c1 - 0xfa) * 0xbc + c2 - 0x40];

				if ( euccode != 0xf4fe ) 
					*out++ = EUCSS3;
				*out++ = euccode >> 8 & 0xff;
				*out++ = euccode & 0xff;

				in += 2;
			}
			else if (c1 >= 0xf0) {
				if (c1 >= 0xf5) {
					if (e_out - out < 3) {
						errno = E2BIG; err_flag = TRUE;
						break;
					}
					c1 -= 0xf5;
					*out++ = EUCSS3;
				}
				else {
					c1 -= 0xf0;
				}
				c1 = (c1 << 1) + 0xf5;
				if (c2 >= 0x80)
					c2--;
				if (c2 >= 0x9e) {
					out[0] = c1 + 1;
					out[1] = c2 + 3;
				}
				else {
					out[0] = c1;
					out[1] = c2 + 0x61;
				}
				in += 2;
				out += 2;
			}
			else {
				if (e_out - out < 2) {
					errno = E2BIG; err_flag = TRUE;
					break;
				}

				/*
				 *	SJIS to EUCJP (kanji)
				 */

				c1 = (c1 - (c1 >= 0xe0 ? 0xc1 : 0x81) << 1) + 0xa1;
				if (c2 >= 0x80)
					c2--;
				if (c2 >= 0x9e) {
					out[0] = c1 + 1;
					out[1] = c2 + 3;
				}
				else {
					out[0] = c1;
					out[1] = c2 + 0x61;
				}
				in += 2;
				out += 2;
			}
			break;

		default:if (e_out - out < 2) {
				errno = E2BIG; err_flag = TRUE; break;
			}
			out[0] = EUCSS2;
			switch (in[0]) {
			case 0x80: out[1] = 0xe0; break;
			case 0xa0: out[1] = 0xe1; break;
			case 0xfd: out[1] = 0xe2; break;
			case 0xfe: out[1] = 0xe3; break;
			case 0xff: out[1] = 0xe4; break;
			}
			in  ++;
			out += 2;
			break;
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
 *   RETURNS:	Error status code.
 *		0	- Successful completion.
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
 *
 *   NOTES:	Method initializer returns NULL value when fails.
 *		If the iconv_open() routine gets NULL value from
 *		the mothod initializer, it returns (size_t)-1 for
 *		XPG/4 interface specification.
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
