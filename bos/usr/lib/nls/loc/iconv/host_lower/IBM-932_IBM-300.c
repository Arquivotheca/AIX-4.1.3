static char sccsid[] = "@(#)81  1.5  src/bos/usr/lib/nls/loc/iconv/host_lower/IBM-932_IBM-300.c, cmdiconv, bos411, 9428A410j 8/26/93 03:38:43";
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
#include "host_lower.h"
#include "IBM-932_IBM-300.h"

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
	uchar_t		c1, c2;
	ushort_t	hostcode;
	int		err_flag = FALSE;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return -1;
	}
        if (inbuf == NULL) return 0;

	e_in  = (in  = *inbuf)  + *inbytesleft;
	e_out = (out = *outbuf) + *outbytesleft;

	while (in < e_in) {
		if (in[0] < 0x81 ||
		   (0x9f < in[0] && in[0] < 0xe0) ||
		    0xfc < in[0]) {
			errno = EILSEQ; err_flag = TRUE; break;
		}
		if (e_in - in < 2) {
			errno = EINVAL; err_flag = TRUE; break;
		}
		c1 = in[0];
		c2 = in[1];
		if (c2 < 0x40 || 0xfc < c2 || c2 == 0x7f) {
			errno = EILSEQ; err_flag = TRUE; break;
		}
		if (e_out - out < 2) {
			errno = E2BIG; err_flag = TRUE; break;
		}
		if (c1 >= 0xe0)
			c1 -= 0x40;
		c1 = c1 - 0x81 << 1;
		if (c2 >= 0x80)
			c2--;
		if (c2 >= 0x9e) {
			c1++;
			c2 -= 0x9e;
		}
		else {
			c2 -= 0x40;
		}
		hostcode = CP301_CP300[c1 * 0x5e + c2];
		*out = hostcode >> 8 & 0xff;
		*(out + 1) = hostcode & 0xff;
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
 *   NAME:      _iconv_close
 *
 *   FUNCTION:  Termination.
 *
 *   RETURNS:   0       - Successful completion.
 *              -1      - Error.
 */

static int	_iconv_close (iconv_t cd) {

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
