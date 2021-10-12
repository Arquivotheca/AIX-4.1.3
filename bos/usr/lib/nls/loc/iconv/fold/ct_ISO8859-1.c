static char sccsid[] = "@(#)31  1.5  src/bos/usr/lib/nls/loc/iconv/fold/ct_ISO8859-1.c, cmdiconv, bos411, 9428A410j 8/24/93 07:31:38";
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
#include <sys/types.h>
#include <iconv.h>
#include <iconvP.h>
#include <fcs.h>
#include "fold.h"

static iconv_rec	invalid;

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

static	size_t	_iconv_exec (
	_LC_fold_iconv_t *cd, 
	uchar_t **inbuf,  size_t *inbytesleft, 
	uchar_t **outbuf, size_t *outbytesleft) {

	uchar_t 	*in, *e_in, *out, *e_out;
	size_t		inlen, outlen, ret_value;
	int		i;
	EscTbl		*etbl;


	if ((cd == NULL) || (cd == -1)) {
		errno = EBADF; return -1;
	}
	if (inbuf == NULL) {
		cd->curcd = cd->gl = cd->defgl;
		cd->gr = cd->defgr;
		return 0;
	}
	etbl = cd->ett->etbl;
	while (1) {
		if (cd->curcd) {
			ret_value = iconv (cd->curcd,
				inbuf, inbytesleft, outbuf, outbytesleft);
		}
		else {
			ret_value = _ascii_exec (
				inbuf, inbytesleft, outbuf, outbytesleft);
		}
		if ((ret_value != -1) ||
		   ((ret_value == -1) && (errno != EILSEQ))) {
			return ret_value;
		}
		in = *inbuf;
		if (*in == 0x1b) {
			for (i = 0; 1; i++) {
				if (i >= cd->ncds){
					errno = EILSEQ; return -1;
				}
				if (etbl[i].len - 1 >= *inbytesleft){
					errno = EINVAL; return -1;
				}
				if (memcmp(*inbuf + 1, &etbl[i].str[1],
					etbl[i].len - 1))
					continue;
				if (!etbl[i].seg) {
					*inbuf += etbl[i].len;
					*inbytesleft -= etbl[i].len;
					if (etbl[i].gl)
						cd->curcd = cd->gl = cd->cds[i];
					else
						cd->curcd = cd->gr = cd->cds[i];
					break;
				}
				if (*inbytesleft < 2) {
					errno = EINVAL; return -1;
				}
				in = *inbuf + etbl[i].len;
				inlen = (in[0] & 0x7f) * 128 + (in[1] & 0x7f);
				if (*inbytesleft < inlen + etbl[i].len + 2){
					errno = EINVAL; return -1;
				}
				if (inlen < etbl[i].seglen){
					errno = EINVAL; return -1;
				}
				if (memcmp(in + 2,
					etbl[i].seg, etbl[i].seglen))
					continue;
				in += etbl[i].seglen + 2;
				inlen -= etbl[i].seglen;
				out = *outbuf;
				outlen = *outbytesleft;
				if ((ret_value = iconv (cd->cds[i],
					&in, &inlen, &out, &outlen)) == -1)
					return ret_value;
				inlen = in - *inbuf;
				*inbuf = in;
				*inbytesleft -= inlen;
				*outbuf = out;
				*outbytesleft = outlen;
				break;
			}
		}
		else if (cd->ett->isctl[*in]) {
			if (cd->ett->isctl[*in] != 1){
				errno = EILSEQ; return -1;
			}
			if (*outbytesleft == 0){
				errno = E2BIG; return -1;
			}
			*(*outbuf)++ = *(*inbuf)++;
			(*inbytesleft)--;
			(*outbytesleft)--;
		}
		else if (*in < 0x80) {
			if (cd->curcd == cd->gl){
				errno = EILSEQ; return -1;
			}
			cd->curcd = cd->gl;
		}
		else {
			if (cd->curcd == cd->gr || cd->gr == &invalid){
				errno = EILSEQ; return -1;
			}
			cd->curcd = cd->gr;
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
	while (i--) if (cd->cds[i]) iconv_close(cd->cds[i]);

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

static	_LC_fold_iconv_t	*init(
	_LC_core_iconv_t	*core_cd, 
	uchar_t			*toname,
	uchar_t			*fromname) {

	_LC_fold_iconv_t	*cd;
	int			i, j;
	EscTblTbl		*ett;


	if      (strcmp ("ct"   , fromname) == 0) ett = _iconv_ct_ett;
	else if (strcmp ("fold7", fromname) == 0) ett = _iconv_fold7_ett;
	else if (strcmp ("fold8", fromname) == 0) ett = _iconv_fold8_ett;
	else return (_LC_fold_iconv_t*)-1;

	for (i = 0; 1; i++) {
		if (ett[i].name == NULL) return (_LC_fold_iconv_t*)-1;
		if (strcmp (toname, ett[i].name) == 0) break;
	}
	if ((cd = malloc(
		sizeof (_LC_fold_iconv_t) + ett[i].netbl * sizeof (iconv_t))) == NULL)
		return (_LC_fold_iconv_t*)-1;

        cd->core = *core_cd;
	cd->ncds = ett[i].netbl;
	cd->cds  = (iconv_t *)&((char *)cd)[sizeof (_LC_fold_iconv_t)];

	for (j = 0; j < cd->ncds; j++) {
		if ((fromname = ett[i].etbl[j].name) == NULL)
			cd->cds[j] = NULL;
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
 *   NAME:	instantiate
 *
 *   FUNCTION:	Instantiation.
 *
 *   RETURNS:	Pointer to a descriptor.
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
