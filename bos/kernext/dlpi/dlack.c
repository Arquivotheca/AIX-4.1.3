static char sccsid[] = "@(#)15  1.1  src/bos/kernext/dlpi/dlack.c, sysxdlpi, bos41J, 9514A_all 3/31/95 16:20:21";
/*
 *   COMPONENT_NAME: SYSXDLPI
 *
 *   FUNCTIONS: errack
 *		okack
 *		uderrack
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * dlack.c - dlpi acknowledgement routines
 */

#include "include.h"

/* public routines */
mblk_t *okack();
mblk_t *errack(), *uderrack();

/*
 * okack - generate DL_OK_ACK
 */

mblk_t *
okack(mp, prim)
	mblk_t *mp;
	ulong prim;
{
	dl_ok_ack_t *ack;

	/* NB: the smallest mblk will hold a DL_OK_ACK */

	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(dl_ok_ack_t);
	mp->b_datap->db_type = M_PCPROTO;
	ack = (dl_ok_ack_t *)mp->b_rptr;
	ack->dl_primitive = DL_OK_ACK;
	ack->dl_correct_primitive = prim;
	return mp;
}

/*
 * errack - generate DL_ERROR_ACK for both DLPI and system errors
 *
 * if dlerr < 0, interpret it as a negated unix errno
 */

mblk_t *
errack(mp, prim, dlerr)
	mblk_t	*mp;
	ulong	prim;
	int	dlerr;
{
	dl_error_ack_t *err;
	int unixerr = 0;

	if (dlerr < 0) {
		unixerr = -dlerr;
		dlerr = DL_SYSERR;
	}

	/* NB: the smallest mblk will hold a DL_ERROR_ACK */

	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + sizeof(dl_error_ack_t);
	mp->b_datap->db_type = M_PCPROTO;
	err = (dl_error_ack_t *)mp->b_rptr;
	err->dl_primitive = DL_ERROR_ACK;
	err->dl_error_primitive = prim;
	err->dl_errno = dlerr;
	err->dl_unix_errno = unixerr;
	return mp;
}

/*
 * uderrack - generate a DL_UDERROR_ACK for fauly DL_UNITDATA_REQ
 *
 * if dlerr < 0, interpret it as a negated unix errno
 */

mblk_t *
uderrack(mp, dlerr)
	mblk_t *mp;
	int dlerr;
{
	dl_uderror_ind_t *err;
	int unixerr = 0;

	if (dlerr < 0) {
		unixerr = -dlerr;
		dlerr = DL_SYSERR;
	}

	err = (dl_uderror_ind_t *)mp->b_rptr;
	err->dl_primitive = DL_UDERROR_IND;
	err->dl_unix_errno = unixerr;
	err->dl_errno = dlerr;
	return mp;
}
