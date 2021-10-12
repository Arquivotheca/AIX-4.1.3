static char sccsid[] = "@(#)57	1.3  src/bos/usr/lib/nls/loc/iconv/fold_lower/IBM-eucTW_CNS11643.1992-4.c, cmdiconv, bos411, 9428A410j 2/27/94 23:37:26";
/*
 *   COMPONENT_NAME: CMDICONV
 *
 *   FUNCTIONS:	_iconv_close
 *		_iconv_exec
 *		init
 *		instantiate
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights	Reserved
 *   US	Government Users Restricted Rights - Use, duplication or
 *   disclosure	restricted by GSA ADP Schedule Contract	with IBM Corp.
 */

#include <stdlib.h>
#include <iconv.h>
#include <iconvP.h>
#include "fold_lower.h"


static	size_t	_iconv_exec (_LC_fold_lower_iconv_t *cd,
	uchar_t** inbuf,  size_t *inbytesleft,
	uchar_t** outbuf, size_t *outbytesleft)
{
	uchar_t		*in, *e_in, *out, *e_out;
	int		err_flag=FALSE;


	if ((cd	== NULL) || (cd	== -1))	{
		errno =	EBADF; return -1;
	}
	if (inbuf == NULL) return 0;
	e_in = (in = *inbuf) + *inbytesleft;
	e_out =	(out = *outbuf)	+ *outbytesleft;
	while (in < e_in) {
		if (*in	== EUCSS2) {
			if (e_in - in <	2) {
				errno =	EINVAL;	err_flag = TRUE;
				break;
			}
			if (*(in + 1) != 0xa4 )	{
				errno =	EILSEQ;	err_flag = TRUE;
				break;
			}
			if (e_in - in <	3) {
				errno =	EINVAL;	err_flag = TRUE;
				break;
			}
			if (*(in + 2) <	0xa1 ||	*(in + 2) == 0xff) {
				errno =	EILSEQ;	err_flag = TRUE;
				break;
			}
			if (e_in - in <	4) {
				errno =	EINVAL;	err_flag = TRUE;
				break;
			}
			if (*(in + 3) <	0xa1 ||	*(in + 3) == 0xff) {
				errno =	EILSEQ;	err_flag = TRUE;
				break;
			}
			if (e_out - out	< 2) {
				errno =	E2BIG; err_flag	= TRUE;
				break;
			}
			*out = *(in + 2) & 0x7f;
			*(out +	1) = *(in + 3) & 0x7f;
			in += 4;
			out += 2;
		}
		else {
			errno =	EILSEQ;	err_flag = TRUE;
			break;
		}
	}   /* while */
	*inbuf = in;
	*outbuf	= out;
	*inbytesleft = e_in - in;
	*outbytesleft =	e_out -	out;
	if (err_flag) return  -1;
	else return 0;
}

static int	_iconv_close (iconv_t cd)
{
	if ((cd	!= NULL) && (cd	!= -1)) {
		free(cd);
		return 0;
	}
	else {
		errno = EBADF;
		return -1;
	}
}

static	_LC_fold_lower_iconv_t	*init (
	_LC_core_iconv_t	*core_cd,
	char			*toname,
	char			*fromname)
{
	_LC_fold_lower_iconv_t	*cd;

	if ((cd	= (_LC_fold_lower_iconv_t *) malloc (
		sizeof (_LC_fold_lower_iconv_t))) == NULL)
		return (_LC_fold_lower_iconv_t*)-1;
	cd->core = *core_cd;
	return cd;
}

_LC_core_iconv_t	*instantiate (void)
{
	static _LC_core_iconv_t	cd;

	cd.hdr.__magic   = _LC_MAGIC;
	cd.hdr.__version = _LC_VERSION;
	cd.hdr.__type_id = _LC_ICONV;
	cd.hdr.__size    = sizeof (_LC_core_iconv_t);
	cd.init	         = (_LC_core_iconv_t*(*)())init;
	cd.exec	         = (size_t(*)())_iconv_exec;
	cd.close         = (int(*)())_iconv_close;
	return &cd;
}
