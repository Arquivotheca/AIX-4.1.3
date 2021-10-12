static char sccsid[] = "@(#)21  1.5  src/bos/usr/lib/nls/loc/iconv/host/IBM-930_IBM-932.c, cmdiconv, bos411, 9428A410j 8/26/93 01:53:50";
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
#include <sys/types.h>
#include <iconv.h>
#include "host.h"
#include <hcs.h>

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

static	size_t	_iconv_exec (
	_LC_host_iconv_t *cd,
	uchar_t **inbuf,  size_t *inbytesleft,
	uchar_t **outbuf, size_t *outbytesleft) {

	uchar_t 	*in, *out;
	size_t		ret;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return -1;
	}
	if (inbuf == NULL) {
		cd->curcd = cd->sb_cd;
		return 0;
	}
	while (1) {
		ret = iconv (cd->curcd, inbuf, inbytesleft, outbuf, outbytesleft);
		if ((ret != -1) ||
		   ((ret == -1) && (errno != EILSEQ)))
			return ret;

		/*
		 *	Conversion was stopped at a character that is not
		 *	valid for current converter.
		 */

		in = *inbuf;
		out = *outbuf;
		if (in[0] == SO) {

			/*
			 *	Shift OUT
			 *	Change to DBCS converter.
			 */

			if (cd->curcd == cd->db_cd) {
				errno = EILSEQ; return -1;
			}
			cd->curcd = cd->db_cd;
		}
		else if (in[0] == SI) {

			/*
			 *	Shift IN
			 *	Change to SBCS converter.
			 */

			if (cd->curcd == cd->sb_cd) {
				errno = EILSEQ; return -1;
			}
			cd->curcd = cd->sb_cd;
		}
		else if ((in[0] <= 0x3f) && (cd->curcd == cd->sb_cd)) {

			/*
			 *	Control code.
			 */

			if (*outbytesleft < 1) {
				errno = E2BIG; return -1;
			}
			out[0] = cd->cntl[in[0]];
			*outbuf = ++out;
			(*outbytesleft)--;
		}
		else {	/*
			 *	Invalid character.
			 */

			errno = EILSEQ; return -1;
		}
		*inbuf = ++in;
		(*inbytesleft)--;
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

		if ((strcmp (t_name, _iconv_host[i].local) == 0) &&
		    (strcmp (f_name, _iconv_host[i].host)  == 0))
			break;
	}
        if ((cd = malloc (sizeof (_LC_host_iconv_t))) == NULL)
		return (_LC_host_iconv_t*)-1;

	if ((cd->sb_cd = iconv_open (t_name, _iconv_host[i].sbcs)) == -1) {
		free (cd);
		return (_LC_host_iconv_t*)-1;
	}
	if ((cd->db_cd = iconv_open (t_name, _iconv_host[i].dbcs)) == -1) {
		iconv_close (cd->sb_cd);
		free (cd);
		return (_LC_host_iconv_t*)-1;
	}
        cd->core  = *core_cd;
	cd->cntl  = _iconv_host[i].fcntl;
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
