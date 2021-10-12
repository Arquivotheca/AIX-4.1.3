static char sccsid[] = "@(#)09  1.3 src/bos/usr/lib/nls/loc/iconv/misc/Universal_UCS_Conv.c, cmdiconv, bos41J, 9509A_all 2/19/95 23:27:07";
/*
 *   COMPONENT_NAME:	CMDICONV
 *
 *   FUNCTIONS:		_iconv_exec
 *			_iconv_close
 *			_iconv_init
 *			instantiate
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <iconv.h>

#include "iconvP.h"
#include "Universal_UCS_Conv.h"

/*
 *   NAME:	_iconv_exec
 *
 *   FUNCTION:	Conversion executor
 *
 *   RETURNS:	>=0	- Number of non-identical conversions performed.
 *		-1	- Error.
 */

static	size_t	_iconv_exec (_LC_UnivUCSConv_iconv_t *cd,
	const uchar_t	**inbuf,
	size_t		*inbytesleft,
	uchar_t		**outbuf,
	size_t		*outbytesleft) {

	uchar_t		*ucs_buf;
	size_t		ucs_left, subs, ret;
	int		err_save, err_flag = FALSE;


	if ((cd == (_LC_UnivUCSConv_iconv_t*)NULL) 
		    || (cd == (_LC_UnivUCSConv_iconv_t*)-1)) {
		errno = EBADF; return -1;
	}

	if (inbuf == NULL) return 0;

	/*
	 *	Convert UCS characters left in the interchange buffer.
	 */

	if (cd->ucs_left > 0) {
		if (subs = iconv (cd->cd_from_ucs,
			(const uchar_t**)&(cd->ucs_buf),
			&(cd->ucs_left),
			outbuf,
			outbytesleft) == -1)

			return (size_t)-1;
		else {
			/* Return E2BIG error so that the outbuf is 
			 * flushed, otherwise, a case can occur where the
			 * inbytesleft = 0 and E2BIG (from UCS->target) is
			 * returned; but since inbytesleft = 0 caller 
			 * does not believe there is more conversion to do... 
		    	 * so we hope this will force them to flush the
			 * output and recall with inbuf intact.
		 	 */
			errno = E2BIG;
			return (size_t)-1;
		}	
	}
	else    subs = 0;

	/*
	 *	Expand the interchange buffer if necessary.
	 */

	if (cd->ucs_buf_size < *inbytesleft) {
		free (cd->ucs_buf_top);
		if  ((cd->ucs_buf_top = malloc (
			*inbytesleft * sizeof (UniChar))) == NULL) {
			errno = ENOMEM;
			return (size_t)-1;
		}
		cd->ucs_buf_size = *inbytesleft;
	}

	/*
	 *	Input --> Interchange
	 */

	ucs_buf  = cd->ucs_buf_top;
	ucs_left = cd->ucs_buf_size;
	if ((ret = (size_t) iconv (cd->cd_to_ucs,
		inbuf, inbytesleft, &ucs_buf, &ucs_left)) == -1)
		err_flag = TRUE;
	else	subs += ret;

	/*
	 *	Interchange --> Output
	 */

	cd->ucs_buf  = cd->ucs_buf_top;
	cd->ucs_left = cd->ucs_buf_size - ucs_left;
	if ((ret = (size_t)iconv (cd->cd_from_ucs,
		(const uchar_t**)&(cd->ucs_buf), &(cd->ucs_left), 
			outbuf, outbytesleft)) == -1)
		err_flag = TRUE;
	else	subs += ret;

	if (!err_flag)	return subs;
	else 		return -1;
}

/*
 *   NAME:	init
 *
 *   FUNCTION:	Open the converter.
 *
 *   RETURNS:	Pointer to the conversion descriptor.
 */

static	_LC_UnivUCSConv_iconv_t	*_iconv_init (
	_LC_core_iconv_t	*core,
	uchar_t			*t_name,
	uchar_t			*f_name) {

	_LC_UnivUCSConv_iconv_t	*cd = NULL;


	if ((cd = malloc (sizeof (_LC_UnivUCSConv_iconv_t))) == NULL) {
		errno = ENOMEM;
		goto Bail;
	}
	cd->cd_to_ucs = cd->cd_from_ucs = (iconv_t) -1;

	if (((cd->cd_to_ucs   = iconv_open (DEF_UCS_NAME, f_name)) == (iconv_t)-1) ||
	    ((cd->cd_from_ucs = iconv_open (t_name, DEF_UCS_NAME)) == (iconv_t)-1)) {
		goto Bail;
	}
	if ((cd->ucs_buf_top = malloc (
		sizeof (UniChar) * DEF_UCS_BUFSIZE)) == NULL) {
		errno = ENOMEM;
		goto Bail;
	}
	cd->ucs_buf_size = DEF_UCS_BUFSIZE;
	cd->ucs_buf      = cd->ucs_buf_top;
	cd->ucs_left     = 0;
	cd->core         = *core;

	return cd;

Bail:	if (cd->cd_to_ucs   != (iconv_t)-1) iconv_close (cd->cd_to_ucs);
	if (cd->cd_from_ucs != (iconv_t)-1) iconv_close (cd->cd_from_ucs);
	if (cd != (iconv_t)NULL) free (cd);

	return (_LC_UnivUCSConv_iconv_t*)-1;
}

/*
 *   NAME:	_iconv_close
 *
 *   FUNCTION:	Cose the converter.
 *
 *   RETURN VALUE DESCRIPTION:
 *	0 if successful completion, -1 if error.
 */

static	int	_iconv_close (_LC_UnivUCSConv_iconv_t *cd) {

	int	err_flag;

	if ((cd == (_LC_UnivUCSConv_iconv_t*)NULL) 
		     || (cd == (_LC_UnivUCSConv_iconv_t*)-1)) {
		errno = EBADF;
		return -1;
	}
	err_flag = FALSE;

	if (iconv_close (cd->cd_from_ucs) == -1) err_flag = TRUE;
	if (iconv_close (cd->cd_to_ucs)   == -1) err_flag = TRUE;

	free (cd->ucs_buf_top);
	free (cd);

	if (!err_flag)	return 0;
	else		return -1;
}

/*
 *   NAME:	instantiate
 *
 *   FUNCTION:	Instantiation method of this converter.
 *
 *   RETURNS:	Pointer to the descriptor.
 */

_LC_core_iconv_t	*instantiate () {

	static	_LC_core_iconv_t	core;

	core.hdr.__magic   = _LC_MAGIC;
	core.hdr.__version = _LC_VERSION;
	core.hdr.__type_id = _LC_ICONV;
	core.hdr.__size    = sizeof (_LC_core_iconv_t);
	core.init          = (_LC_core_iconv_t*(*)())_iconv_init;
	core.close         = (int(*)())_iconv_close;
	core.exec          = (size_t(*)())_iconv_exec;

	return &core;
}
