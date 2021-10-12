static char sccsid[] = "@(#)11  1.4  src/bos/usr/lib/nls/loc/iconv/fold_lower/IBM-eucJP_JISX0208.1978-GL.c, cmdiconv, bos411, 9428A410j 8/26/93 01:06:05";
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
#include "iconvEUC.h"
#include "fold_lower.h"

/*
 *   NAME:	_iconv_exec
 *
 *   FUNCTION:	Conversion.
 *
 *   RETURNS:	>= 0	- Number of substitutions.
 *		-1	- Error.
 *
 *   NOTE:      This routine returns always 0 on successful condition,
 *              does actually not count number of substitutions.
 */

static	size_t	_iconv_exec (
	_LC_fold_lower_iconv_t *cd, 
	uchar_t **inbuf,  size_t *inbytesleft,
	uchar_t **outbuf, size_t *outbytesleft) {

	uchar_t		*in, *out, *e_in, *e_out, c1, c2;
	ushort_t	jiscode;
	int		i, low, high, err_flag = FALSE;


        if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return -1;
	}
        if (inbuf == NULL) return 0;

        e_in  = (in  = *inbuf)  + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;

	while (1) {
		if (e_in - in < 2) {
			if (e_in - in == 1) {
				if (in[0] < 0xa1 || in[0] == 0xff){
					errno = EILSEQ; err_flag = TRUE;
				} else {
					errno = EINVAL; err_flag = TRUE;
				}
			}
			break;
		}
		if (in[0] < 0xa1 || in[1] < 0xa1 ||
		    in[0] == 0xff || in[1] == 0xff) {
			errno = EILSEQ; err_flag = TRUE; break;
		}
		if (e_out - out < 2) {
			errno = E2BIG;  err_flag = TRUE; break;
		}
		c1 = in[0];
		c2 = in[1];
		low = 0;
		high = sizeof (EUCJP_JIS78_from) / sizeof (EUCJP_JIS78_from[0]);
		jiscode = (c1 << 8 & 0xff00) + (c2 & 0xff) ;
		while (low <= high) {
			i = low + high >> 1;
			if (jiscode < EUCJP_JIS78_from[i])
				high = i - 1;
			else if (jiscode > EUCJP_JIS78_from[i])
				low = i + 1;
			else {
				jiscode = EUCJP_JIS78_to[i];
				c1 = jiscode >> 8 & 0xff;
				c2 = jiscode & 0xff;
				break;
			}
		}
		out[0] = c1 & 0x7f;
		out[1] = c2 & 0x7f;
		in += 2;
		out += 2;
	}
	*inbuf        = in;
        *outbuf       = out;
	*inbytesleft  = e_in - in;
	*outbytesleft = e_out - out;

	if (!err_flag) return 0;
	else           return -1;
}

/*
 *   NAME:	_iconv_close
 *
 *   FUNCTION:	Termination.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

static	int	_iconv_close (iconv_t cd) {

	if ((cd != NULL) && (cd != -1)) {
		free (cd);
		return 0;
	}
	else {
		errno = EBADF;
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

static	_LC_fold_lower_iconv_t	*init (
	_LC_core_iconv_t	*core_cd, 
	uchar_t			*toname,
	uchar_t			*fromname) {

        _LC_fold_lower_iconv_t	*cd;

        if ((cd = malloc (
		sizeof (_LC_fold_lower_iconv_t))) == NULL)
                return (_LC_fold_lower_iconv_t*)-1;
        cd->core = *core_cd;
        return cd;
}

/*
 *   NAME:	instantiate
 *
 *   FUNCTION:	Instantiation method of this converter.
 *
 *   RETURNS:	Pointer to the descriptor.
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
