static char sccsid[] = "@(#)22  1.6  src/bos/usr/lib/nls/loc/iconv/host/IBM-932_IBM-930.c, cmdiconv, bos411, 9428A410j 3/22/94 21:01:26";
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
#include <hcs.h>
#include "host.h"

/*
 *   NAME:      _iconv_exec
 *
 *   FUNCTION:  Conversion.
 *
 *   RETURNS:   >= 0    - Number of substitutions.
 *              -1      - Error.
 *
 *   NOTES:     This routine returns only 0 on successful condition,
 *              does not actually count the number of substitutions.
 */

static	size_t		_iconv_exec (
	_LC_host_iconv_t *cd,
	uchar_t **in_buf,  size_t *in_bytes_left,
	uchar_t **out_buf, size_t *out_bytes_left) {

	uchar_t 	*in_ptr, *out_ptr, *save_in_ptr, *save_out_ptr;
	size_t		in_left, out_left, save_out_left, ret;
	int		cd_switched = FALSE;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return (size_t)-1;
	}
	if (in_buf == NULL) {

		/*
		 *	Reset shift status to SBCS.
		 */

		if (cd->curcd == cd->sb_cd) return (size_t)0;

		cd->curcd = cd->sb_cd;

		if (out_buf == NULL) {
			errno = EINVAL; return (size_t)-1;
		}
		if (*out_bytes_left < 1) {
			errno = E2BIG; return (size_t)-1;
		}
		*(*out_buf) = SI;
		(*out_buf) ++;
		(*out_bytes_left) --;

		return (size_t)0;
	}
	in_ptr   = *in_buf;
	in_left  = *in_bytes_left;
	out_ptr  = *out_buf;
	out_left = *out_bytes_left;

	while (TRUE) {

		save_in_ptr = in_ptr;

		ret = iconv (cd->curcd, &in_ptr, &in_left, &out_ptr, &out_left);

		if ((cd_switched) && (in_ptr == save_in_ptr)) {
			if (cd->curcd == cd->sb_cd)
				cd->curcd = cd->db_cd;
			else	cd->curcd = cd->sb_cd;
			*in_buf         = in_ptr;
			*in_bytes_left  = in_left;
			*out_buf        = save_out_ptr;
			*out_bytes_left = save_out_left;
			return (size_t)-1;
		}
		if (!((ret == (size_t)-1) && (errno == EILSEQ))) {
			*in_buf         = in_ptr;
			*in_bytes_left  = in_left;
			*out_buf        = out_ptr;
			*out_bytes_left = out_left;
			return ret;
		}

		/*
		 *	Conversion was stopped at the character
		 *	that is invalid for current converter.
		 */

		if (cd->curcd == cd->sb_cd) {

			if (*in_ptr < 0x20) {

				/*
				 *	Control character.
				 */

				if (out_left < 1) {
					errno = E2BIG;
					return (size_t)-1;
				}
				*out_ptr = cd->cntl[*in_ptr];
				in_ptr  ++; out_ptr  ++;
				in_left --; out_left --;
				cd_switched = FALSE;
			}
			else {	/*
				 *	Change state to DBCS.
				 */

				save_out_ptr  = out_ptr;
				save_out_left = out_left;

				if (out_left >= 3) {
					*out_ptr = SO;
					out_ptr ++;
					out_left --;
				}
				else	out_left = 0;

				cd->curcd = cd->db_cd;
				cd_switched = TRUE;
			}
		}
		else {	/*
			 *	Change state to SBCS.
			 */

			save_out_ptr  = out_ptr;
			save_out_left = out_left;

			if (out_left >= 2) {
				*out_ptr = SI;
				out_ptr ++;
				out_left --;
			}
			else	out_left = 0;

			if (*in_ptr < 0x20) {

				/*
				 *	Control character.
				 */

				if (out_left < 1) {
					errno = E2BIG;
					return (size_t)-1;
				}
				*out_ptr = cd->cntl[*in_ptr];
				in_ptr  ++; out_ptr  ++;
				in_left --; out_left --;
				cd_switched = FALSE;
			} else	cd_switched = TRUE;
			cd->curcd = cd->sb_cd;
		}
	}
}

/*
 *   NAME:      _iconv_close
 *
 *   FUNCTION:  Termination.
 *
 *   RETURNS:   0       - Successful completion.
 *              -1      - Error.
 */

static	int	_iconv_close (_LC_host_iconv_t *cd) {

	int	err_flag = FALSE;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return -1;
	}
	if (cd->sb_cd != NULL) {
		if (iconv_close (cd->sb_cd) == -1) err_flag = TRUE;
	}
	if (cd->db_cd != NULL) {
		if (iconv_close (cd->db_cd) == -1) err_flag = TRUE;
	}
	free (cd);
	if (!err_flag) return 0;
	else           return -1;
}

/*
 *   NAME:      init
 *
 *   FUNCTION:  Initialization.
 *
 *   RETURNS:   Pointer to a descriptor, or -1 if error.
 */

static	_LC_host_iconv_t	*init (
	_LC_core_iconv_t	*core_cd, 
	uchar_t			*t_name,
	uchar_t			*f_name) {

        _LC_host_iconv_t	*cd;
	int			i;


	for (i = 0; 1; i++) {

		if (!_iconv_host[i].local) return (_LC_host_iconv_t*)-1;

		if ((strcmp (f_name, _iconv_host[i].local) == 0) &&
		    (strcmp (t_name, _iconv_host[i].host ) == 0))
			break;
	}
	if ((cd = malloc (sizeof (_LC_host_iconv_t))) == NULL)
                return (_LC_host_iconv_t*)-1;

	if ((cd->sb_cd = iconv_open (_iconv_host[i].sbcs, f_name)) == -1) {
		free (cd);
		return (_LC_host_iconv_t*)-1;
	}
	if ((cd->db_cd = iconv_open (_iconv_host[i].dbcs, f_name)) == -1) {
		iconv_close (cd->sb_cd);
		free (cd);
		return (_LC_host_iconv_t*)-1;
	}
        cd->core  = *core_cd;
	cd->cntl  = _iconv_host[i].tcntl;
	cd->curcd = cd->sb_cd;
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
