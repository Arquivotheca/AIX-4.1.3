static char sccsid[] = "@(#)11  1.1  src/bos/usr/lib/nls/loc/iconv/fold/IBM-932_fold7.c, cmdiconv, bos411, 9428A410j 4/7/94 01:50:32";
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

#include <stdio.h>
#include <string.h>
#include <iconv.h>
#include <iconvP.h>
#include <fcs.h>
#include "fold.h"

static	iconv_rec		_invalid_cd;

static	int			_reset_state (
	_LC_fold_iconv_t	*cd,
	uchar_t			**out_buf,
	size_t			*out_left) {

	EscTbl			*etbl;
	size_t			len;

	if (cd->curcd == cd->defgl) return TRUE;

	cd->curcd = cd->defgl;
	cd->gl    = cd->defgl;
	cd->gr    = &_invalid_cd;

	if (out_buf == NULL) {
		errno = EINVAL; return FALSE;
	}
	etbl = cd->ett->etbl;
	len  = (size_t)(etbl[cd->ett->defgl].len);
	if (len > *out_left) {
		errno = E2BIG; return FALSE;
	}
	memcpy (*out_buf, etbl[cd->ett->defgl].str, len);
	*out_buf  += len;
	*out_left -= len;
	return TRUE;
}

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

static	size_t		_iconv_exec(
	_LC_fold_iconv_t *cd, 
	uchar_t **in_buf,  size_t *in_left, 
	uchar_t **out_buf, size_t *out_left) {

	uchar_t 	*in_buf_save;
	size_t		nsubs, len;
	int		cs_index;
	EscTbl		*etbl;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return (size_t)-1;
	}
	if (in_buf == NULL) {
		if (_reset_state (cd, out_buf, out_left))
			return (size_t)0;
		else	return (size_t)-1;
	}
	etbl = cd->ett->etbl;
	in_buf_save = NULL;

	while (TRUE) {
		if (cd->curcd == NULL)
			nsubs = _ascii_exec (
				in_buf, in_left, out_buf, out_left);
		else	nsubs = iconv (cd->curcd,
				in_buf, in_left, out_buf, out_left);

		/*
		 *	Return with normal completion, or error exept EILSEQ
		 *	(invalid character) condition.
		 */

		if (!((nsubs == -1) && (errno == EILSEQ))) return nsubs;

		if (*in_buf == in_buf_save) {
			errno = EILSEQ; return (size_t)-1;
		}
		in_buf_save = *in_buf;

		/*
		 *	Get what code set current character belongs to.
		 */

		cs_index = (*cd->ett->csidx)(*in_buf, *in_left);
		if (cs_index == NEEDMORE) {
			errno = EINVAL; return (size_t)-1;
		}
		if (cs_index == INVALIDCSID) {
			errno = EILSEQ; return (size_t)-1;
		}
		if (cs_index == CONTROLCSID) {
			if (*(*in_buf) == 0x0a) {	/* NL */
				if (!_reset_state (cd, out_buf, out_left))
					return (size_t)-1;
			}
			if (*out_left == 0) {
				errno = E2BIG; return (size_t)-1;
			}
			*(*out_buf) = *(*in_buf);
			(*in_buf ) ++; (*in_left ) --;
			(*out_buf) ++; (*out_left) --;
			continue;
		}

		/*
		 *	Put escape sequence, and change converter.
		 */

		len = (size_t)(etbl[cs_index].len);
		if (*out_left < len) {
			errno = E2BIG; return (size_t)-1;
		}
		if (etbl[cs_index].seg == NULL) {
			memcpy (*out_buf, etbl[cs_index].str, len);
			(*out_buf)  += len;
			(*out_left) -= len;
			cd->curcd = cd->cds[cs_index];
			continue;
		}

		/*
		 *	Make an extended segment.
		 */

		len += (size_t)(etbl[cs_index].seglen + 2);
		while (TRUE) {

			uchar_t		*in, *out, *p;
			size_t		in_len, out_len;

			if (*out_left < len) {
				errno = E2BIG; return (size_t)-1;
			}
			in      = *in_buf;
			in_len  = *in_left;
			out     = *out_buf  + len;
			out_len = *out_left - len;
			if (out_len > (0x3fff - etbl[cs_index].seglen))
				out_len = 0x3fff - etbl[cs_index].seglen;

			nsubs = iconv (cd->cds[cs_index], &in, &in_len, &out, &out_len);
			if ((nsubs == -1) && (in == *in_buf)) return (size_t)-1;

			p = *out_buf;
			memcpy (p, etbl[cs_index].str, etbl[cs_index].len);
			p += etbl[cs_index].len + 2;
			p[-2] = (out - p) / 128 | 0x80;
			p[-1] = (out - p) % 128 | 0x80;
			memcpy (p, etbl[cs_index].seg, etbl[cs_index].seglen);

			*in_left  = in_len;
			*in_buf   = in;
			*out_left -=(out - *out_buf);
			*out_buf  = out;
			if (!((nsubs == -1) && (errno == EILSEQ))) return nsubs;
			break;
		}
	}
}

/*
 *   NAME:	_iconv_close
 *
 *   FUNCTION:	Termination.
 *
 *   RETURNS:	0	- Successful completion.
 *		-1	- Error.
 */

static int	_iconv_close (_LC_fold_iconv_t *cd) {

	int	i;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF;
		return -1;
	}
	i = cd->ncds;
	while (i--) if (cd->cds[i]) iconv_close (cd->cds[i]);

	free (cd);
	return 0;
}

/*
 *   NAME:	init
 *
 *   FUNCTION:	Initialization.
 *
 *   RETURNS:	Pointer to a descriptor, or -1 if error.
 */

static	_LC_fold_iconv_t	*init (
	_LC_core_iconv_t	*core_cd, 
	uchar_t			*t_name,
	uchar_t			*f_name) {

        _LC_fold_iconv_t	*cd;
	int			i, j;
	EscTblTbl		*ett;


	if (strcmp (t_name, "fold7") == 0)
		ett = _iconv_fold7_ett;
	else	return (_LC_fold_iconv_t*)-1;

	for (i = 0; 1; i++) {
		if (ett[i].name == NULL) return (_LC_fold_iconv_t*)-1;
		if (strcmp (f_name, ett[i].name) == 0) break;
	}
	if ((cd = (_LC_fold_iconv_t*)malloc (
		sizeof (iconv_t) * ett[i].netbl +
		sizeof (_LC_fold_iconv_t))) == NULL) {
		return (_LC_fold_iconv_t*)-1;
	}
	cd->core = *core_cd;
	cd->ncds = ett[i].netbl;
	cd->cds  = (iconv_t*)((char*)cd + sizeof (_LC_fold_iconv_t));

	for (j = 0; j < cd->ncds; j++) {
		if ((t_name = ett[i].etbl[j].name) == NULL) {
			cd->cds[j] = NULL;
		}
		else if ((cd->cds[j] = iconv_open (t_name, f_name)) == -1) {
			while (j--) if (cd->cds[j] != NULL)
				iconv_close (cd->cds[j]);
			return (_LC_fold_iconv_t*)-1;
		}
	}
	cd->ett   = &ett[i];
	cd->defgl = cd->cds[ett[i].defgl];
	cd->defgr = &_invalid_cd;
	cd->gl    = cd->defgl;
	cd->gr    = cd->defgr;
	cd->curcd = cd->gl;

	return cd;
}

/*
 *   NAME:	instantiate.
 *
 *   FUNCTION:	Instrantiation.
 *
 *   RETURNS:	Pointer to a descriptor.
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
