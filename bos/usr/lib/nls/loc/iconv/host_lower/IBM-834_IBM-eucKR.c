static char sccsid[] = "@(#)00  1.4  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-834_IBM-eucKR.c, cmdiconv, bos411, 9428A410j 3/22/94 08:52:20";
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
#include "host_lower.h"
#include "IBM-834_KSC5601.h"

#define	CLOVER_SIGN 0xA2C0

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
	_LC_host_lower_iconv_t *cd,
	uchar_t **inbuf,  size_t *inbytesleft,
	uchar_t **outbuf, size_t *outbytesleft) {

	uchar_t		*in, *out, *e_in, *e_out;
	ushort_t	code;
	int		i, found, high,	low, err_flag = FALSE;


	if ((cd == NULL) || (cd == -1)) {
		errno =	EBADF; return -1;
	}
	if (inbuf == NULL) return 0;

	e_in  = (in  = *inbuf)  + *inbytesleft;
	e_out =	(out = *outbuf)	+ *outbytesleft;

	while (in < e_in) {
		if (e_in - in <	2) {
			errno =	EINVAL;	err_flag = TRUE; break;
		}
		if ((in[0] < 0x40 ) || (in[1] <	0x40) || (in[1]	== 0xff)) {
			errno =	EILSEQ;	err_flag = TRUE; break;
		}
		if (e_out - out	< 2) {
			errno =	E2BIG; err_flag = TRUE; break;
		}
		if (in[0] == 0x40) {
			if (in[1] != 0x40) {
				errno =	EILSEQ;	err_flag = TRUE; break;
			}

			/*
			 *	Double byte space character
			 */

			out[0] = 0xA1;
			out[1] = 0xA1;
		}
		else {
			if ((in[0] >= 0x50) && (in[0] <= 0x6c)) {
				if (((in[0] == 0x6c) &&
				    ((in[1] < 0x41) || (in[1] >	0x45)))	||
				    ((in[0] != 0x6c) &&
				    ((in[1] < 0x41) || (in[1] > 0xfe)))) {
					errno =	EILSEQ;	err_flag = TRUE; break;
				}
				i = (in[0] - 0x50) * 190 + in[1] - 0x41;
				code = IBM834_KSC5601_HJ[i]; 

				/*
				 *	For chars which exist in PC code
				 *	but do no exist in KS code,
				 *	clover sign(0xA2C0) is returned
				 */

				if (code == 0xffff) {
					errno =	EILSEQ;	err_flag = TRUE; break;
				}
				if (code == 0xfefe)
					code = CLOVER_SIGN;
				out[0] = code >> 8 &	0xff;
				out[1] = code & 0xff;
			}
			else {	/*
				 *	IBM834 to eucKR
				 */

				int	found = 0;

				code = (in[0] << 8 & 0xff00) + (in[1] &	0xff);
				low = 0;
				high = sizeof (IBM834_KSC5601_HGSP) / 
				       sizeof (IBM834_KSC5601_HGSP[0]);
				while (low <= high) {
					i = low	+ high >> 1;
					if (code < IBM834_KSC5601_HGSP[i][0])
						high = i - 1;
					else if	(code >	IBM834_KSC5601_HGSP[i][0])
						low = i	+ 1;
					else {
						found =	1;
						code = IBM834_KSC5601_HGSP[i][1];

						/*
						 * For chars which exist in PC code
						 * but do no exist in KS code,
						 * clover sign(0xA2C0) is returned.
						 */

						if (code == 0xfefe)
							code = CLOVER_SIGN;
						out[0] = code >> 8 & 0xff;
						out[1] = code &	0xff;
						break ;
					}
				}
				if (!found) {
					errno =	EILSEQ;	err_flag = TRUE; break;
				}
			}
		}
		in += 2;
		out += 2;
	}
	*inbytesleft = e_in - in;
	*outbytesleft =	e_out -	out;
	*inbuf = in;
	*outbuf	= out;

	if (!err_flag) return 0;
	else           return -1;
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

static	_LC_host_lower_iconv_t	*init (
	_LC_core_iconv_t	*core_cd,
	uchar_t			*toname,
	uchar_t			*fromname) {

	_LC_host_lower_iconv_t	*cd;

	if ((cd = malloc (
		sizeof (_LC_host_lower_iconv_t))) == NULL)
		return (_LC_host_lower_iconv_t*)-1;
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
