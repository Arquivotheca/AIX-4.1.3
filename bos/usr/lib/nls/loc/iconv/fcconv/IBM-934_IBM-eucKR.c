static char sccsid[] = "@(#)56  1.4  src/bos/usr/lib/nls/loc/iconv/fcconv/IBM-934_IBM-eucKR.c, cmdiconv, bos411, 9428A410j 2/1/94 06:45:26";
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
#include "pcks.h"

#define CLOVER_SIGN 0xA2C0

/*
 *   NAME:	_iconv_exec
 *
 *   FUNCTION:	Conversion.
 *
 *   RETURNS:	>= 0    - Number of substitution.
 *		-1      - Error.
 *
 *   NOTE:	This routine returns only 0 on successful condition,
 *		does not actually count the number of substitutions.
 */

static size_t	_iconv_exec (
	_LC_fcconv_iconv_t *cd,
	uchar_t **inbuf,  size_t *inbytesleft,
	uchar_t **outbuf, size_t *outbytesleft) {

	uchar_t		*in, *out, *e_in, *e_out, c1, c2;
	ushort_t	pccode;
	int		i, err_flag = FALSE;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return -1;
        }
        if (inbuf == NULL) return 0;

        e_in  = (in  = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
	while (in < e_in) {
		switch (index934[in[0]]) {

		case 0:	if (out >= e_out) {
				errno = E2BIG; err_flag = TRUE; break;
			}
			*out++ = *in++;
			break;

		case 1:	/*
			 *	euc-KR does not support sbcs-jamo,
			 *	the chars are converted to spaces.
			 */

			if (out >= e_out) {
				errno = E2BIG; err_flag = TRUE; break;
			}
			*out++ = ' ';
                        in++;
			break;

		case 2:	if (e_in - in < 2) {
				errno = EINVAL; err_flag = TRUE; break;
			}
			if (in[0] > 0xaf) {
				if ((in[0] < 0xba) || (in[0] > 0xbe) ||
				    (in[1] < 0x40) || (in[1] > 0xfc) ||
				   ((in[0] == 0xbe) && (in[1] > 0x4c))) {
					errno = EILSEQ; err_flag = TRUE; break;
				}
				i = (in[0] - 0x81 - 10) * 189 + in[1] -0x40;
			}
			else {
				if ((in[0] < 0x81) ||
				    (in[1] < 0x40) || (in[1] > 0xfc)) {
					errno = EILSEQ; err_flag = TRUE; break;
				}
				i = (in[0] - 0x81) * 189 + in[1] -0x40;
			}
			pccode = IBMPC_KSC5601[i];
			if (pccode == 0xffff) {
				errno = EILSEQ; err_flag = TRUE; break;
			}

			/*
			 *	for chars which exist in PC codeset but
			 *	do not exist in KS codeset, clover sign
			 *	(0xa2c0) is returned.
			 */

			if (pccode == 0xfefe) pccode = CLOVER_SIGN;

			c1 = pccode >> 8 & 0xff;
			c2 = pccode & 0xff;
			if (e_out - out < 2) {
				errno = E2BIG; err_flag = TRUE; break;
			}
			out[0] = c1;
			out[1] = c2;
			in  += 2;
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

	_LC_fcconv_iconv_t      *cd;

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

        static _LC_core_iconv_t cd;

	cd.hdr.__magic   = _LC_MAGIC;
	cd.hdr.__version = _LC_VERSION;
	cd.hdr.__type_id = _LC_ICONV;
	cd.hdr.__size    = sizeof (_LC_core_iconv_t);
	cd.init          = (_LC_core_iconv_t*(*)())init;
	cd.exec          = (size_t(*)())_iconv_exec;
	cd.close         = (int(*)())_iconv_close;
	return &cd;
}
