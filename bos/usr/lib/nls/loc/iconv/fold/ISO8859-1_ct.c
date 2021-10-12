static char sccsid[] = "@(#)29  1.9  src/bos/usr/lib/nls/loc/iconv/fold/ISO8859-1_ct.c, cmdiconv, bos411, 9428A410j 4/7/94 01:49:12";
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

static	iconv_rec	invalid;

static	int			_reset_state (
	_LC_fold_iconv_t	*cd,
	uchar_t			**out_buf,
	size_t			*out_left) {

	EscTbl			*etbl;
	size_t			len;

	if (cd->curcd == cd->defgl) return TRUE;

	cd->curcd = cd->defgl;
	cd->gl    = cd->defgl;
	cd->gr    = cd->defgr;

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
	uchar_t **inbuf,  size_t *inbytesleft, 
	uchar_t **outbuf, size_t *outbytesleft) {

	uchar_t 	*in, *out, *ptr, *pre_inbuf;
	size_t		inlen, outlen, len, nsubs;
	int		idx;
	EscTbl		*etbl;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return (size_t)-1;
	}
	if (inbuf == NULL) {
		if (_reset_state (cd, outbuf, outbytesleft))
			return (size_t)0;
		else	return (size_t)-1;
	}
	etbl = cd->ett->etbl;
	pre_inbuf = NULL;

	while (1) {
		if (cd->curcd == NULL)
			nsubs = _ascii_exec (
				inbuf, inbytesleft, outbuf, outbytesleft);
		else	nsubs = iconv (cd->curcd,
				inbuf, inbytesleft, outbuf, outbytesleft);

		/*
		 *	Return with normal completion, or error exept EILSEQ
		 *	(invalid character) condition.
		 */

		if (!((nsubs == -1) && (errno == EILSEQ))) return nsubs;

		if (*inbuf == pre_inbuf) {
			errno = EILSEQ; return (size_t)-1;
		}
		pre_inbuf = *inbuf;

		/*
		 *	Get what code set current character belongs to.
		 */

		idx = (*cd->ett->csidx)(*inbuf, *inbytesleft);
		if (idx == NEEDMORE) {
			errno = EINVAL; return (size_t)-1;
		}
		if (idx == INVALIDCSID) {
			errno = EILSEQ; return (size_t)-1;
		}
		if (idx == CONTROLCSID) {
			if (*outbytesleft == 0) {
				errno = E2BIG; return (size_t)-1;
			}
			*(*outbuf) = *(*inbuf);
			(*inbuf) ++;
			(*outbuf) ++;
			(*inbytesleft) --;
			(*outbytesleft) --;
			continue;
		}

		/*
		 *	Put escape sequence, and change converter.
		 */

		len = (size_t)(etbl[idx].len);
		if (*outbytesleft < len) {
			errno = E2BIG; return (size_t)-1;
		}
		if (etbl[idx].seg == NULL) {
			memcpy (*outbuf, etbl[idx].str, len);
			(*outbuf) += len;
			(*outbytesleft) -= len;
			cd->curcd = cd->cds[idx];
			continue;
		}

		/*
		 *	Make an extended segment.
		 */

		len += (size_t)(etbl[idx].seglen + 2);
		while (1) {
			if (*outbytesleft < len) {
				errno = E2BIG; return (size_t)-1;
			}
			in = *inbuf;
			inlen = *inbytesleft;
			out = *outbuf + len;
			outlen = *outbytesleft - len;
			if (outlen > 0x3fff - etbl[idx].seglen)
				outlen = 0x3fff - etbl[idx].seglen;
			nsubs = iconv (cd->cds[idx], &in, &inlen, &out, &outlen);
			if ((in == *inbuf) && (nsubs == -1)) return nsubs;
			ptr = *outbuf;
			memcpy(ptr, etbl[idx].str, etbl[idx].len);
			ptr += etbl[idx].len + 2;
			ptr[-2] = (out - ptr) / 128 | 0x80;
			ptr[-1] = (out - ptr) % 128 | 0x80;
			memcpy(ptr, etbl[idx].seg, etbl[idx].seglen);
			*inbuf = in;
			*inbytesleft = inlen;
			ptr = *outbuf;
			*outbuf = out;
			*outbytesleft -= out - ptr;
			if (nsubs != -1) return nsubs;
			if (errno == EINVAL) return nsubs;
			if (errno == EILSEQ) break;
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
	uchar_t			*toname,
	uchar_t			*fromname) {

        _LC_fold_iconv_t	*cd;
	int			i, j;
	EscTblTbl		*ett;


	if      (strcmp (toname, "ct"   ) == 0) ett = _iconv_ct_ett;
	else if (strcmp (toname, "fold7") == 0) ett = _iconv_fold7_ett;
	else if (strcmp (toname, "fold8") == 0) ett = _iconv_fold8_ett;
	else return (_LC_fold_iconv_t*)-1;

	for (i = 0; 1; i++) {
		if (ett[i].name == NULL) return (_LC_fold_iconv_t*)-1;
		if (strcmp (fromname, ett[i].name) == 0) break;
	}
	if ((cd = malloc (
		sizeof (_LC_fold_iconv_t) + ett[i].netbl * sizeof (iconv_t))) == NULL)
		return (_LC_fold_iconv_t*)-1;

	cd->core = *core_cd;
	cd->ncds = ett[i].netbl;
	cd->cds  = (iconv_t*)&((char*)cd)[sizeof (_LC_fold_iconv_t)];

	for (j = 0; j < cd->ncds; j++) {
		if ((toname = ett[i].etbl[j].name) == NULL) {
			cd->cds[j] = NULL;
		}
		else if ((cd->cds[j] = iconv_open (toname, fromname)) == -1) {
			while (j--) if (cd->cds[j] != NULL)
				iconv_close (cd->cds[j]);
			return (_LC_fold_iconv_t*)-1;
		}
	}
	cd->ett   = &ett[i];
	cd->defgl = cd->cds[ett[i].defgl];
	cd->defgr = cd->cds[ett[i].defgr];
	if (cd->defgl == cd->defgr) cd->defgr = &invalid;
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
