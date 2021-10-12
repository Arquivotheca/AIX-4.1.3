static char sccsid[] = "@(#)30        1.5  src/bos/kernext/pse/nd.c, sysxpse, bos411, 9428A410j 11/2/93 11:54:27";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      nd_free
 *                 nd_getset
 *                 nd_get_default
 *                 nd_get_long
 *                 nd_get_names
 *                 nd_load
 *                 nd_set_default
 *                 nd_set_long
 *                 
 * 
 * ORIGINS: 63, 71, 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */
/** Copyright (c) 1988  Mentat Inc.
 ** nd.c 1.5, last change 1/2/90
 **/


#include <pse/str_stream.h>
#include <pse/nd.h>

#include <sys/strstat.h>
#include <pse/str_debug.h>

#ifndef staticf
#define staticf static
#endif

/* Named dispatch table entry */
typedef	struct	nde_s {
	char	* nde_name;
	int	(*nde_get_pfi)();
	int	(*nde_set_pfi)();
	caddr_t	nde_data;
} NDE;

/* Name dispatch table */
typedef	struct	nd_s {
	int	nd_free_count; /* number of unused nd table entries */
	int	nd_size;	/* size (in bytes) of current table */
	NDE	* nd_tbl;	/* pointer to table in heap */
} ND;

typedef	struct iocblk	* IOCP;

#define	NDE_ALLOC_COUNT	4
#define	NDE_ALLOC_SIZE	(sizeof(NDE) * NDE_ALLOC_COUNT)

extern	int	nd_get_names(queue_t *, mblk_t *, ND *);
extern	int	nd_set_default(queue_t *, mblk_t *, char *, caddr_t);

/** Free the table pointed to by 'ndp' */
void
nd_free (nd_pparam)
	caddr_t	* nd_pparam;
{
	ND	** ndp;
	ND	* nd;

	ENTER_FUNC(nd_pparam, 0, 0, 0, 0, 0, 0);

	if ((ndp = (ND **)nd_pparam)  &&  (nd = *ndp)) {
		if (nd->nd_tbl)
			he_free((char *)nd->nd_tbl);
		he_free((char *)nd);
		*ndp = nilp(ND);
	}

	LEAVE_FUNC(nd_free, 0);
}

int
nd_getset (q, nd_param, mp)
	queue_t	* q;
	caddr_t	nd_param;
reg	mblk_t *	mp;
{
	int	err;
	IOCP	iocp;
	mblk_t *	mp1;
	ND	* nd;
reg	NDE	* nde;
	unsigned char	* value;


	ENTER_FUNC(nd_getset, q, nd_param, mp, 0, 0, 0);

	if (!(nd = (ND *)nd_param)) {
		LEAVE_FUNC(nd_getset, FALSE);
		return FALSE;
	}
	iocp = (IOCP)mp->b_rptr;
	if (iocp->ioc_count == 0  ||  !(mp1 = mp->b_cont)) {
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		iocp->ioc_error = EINVAL;
		LEAVE_FUNC(nd_getset, TRUE);
		return TRUE;
	}
	mp1->b_datap->db_lim[-1] = '\0';	/* Force null termination */
	err = ENOENT;
	for (nde = nd->nd_tbl; nde->nde_name; nde++) {
		if (strcmp(nde->nde_name, (char *)mp1->b_rptr))
			continue;
		switch (iocp->ioc_cmd) {
		case ND_GET:
			mp1->b_rptr = mp1->b_datap->db_base;
			mp1->b_wptr = mp1->b_rptr;
			err = (*nde->nde_get_pfi)(q,mp1,nde->nde_data);
			if (!err) {
				while (mp1->b_cont)
					mp1 = mp1->b_cont;
				if (mp1->b_wptr < mp1->b_datap->db_lim)
					*mp1->b_wptr++ = '\0';
				iocp->ioc_count = msgdsize(mp->b_cont);
			}
			break;
		case ND_SET:
			err = EINVAL;
			for (value = mp1->b_rptr; ++value < mp1->b_datap->db_lim;) {
				if (!value[-1]) {
					err = (*nde->nde_set_pfi)(q,mp1,value,nde->nde_data);
					iocp->ioc_count = 0;
					if (mp1->b_cont) {
						freemsg(mp1->b_cont);
						mp1->b_cont = nil(mblk_t *);
					}
					break;
				}
			}
			break;
		default:
			err = EINVAL;
			break;
		}
		break;
	}
	iocp->ioc_error = err;
	if (err == ENOENT) {
		LEAVE_FUNC(nd_getset, FALSE);
		return FALSE;
	}
	mp->b_datap->db_type = M_IOCACK;
	LEAVE_FUNC(nd_getset, TRUE);
	return TRUE;
}

int
nd_get_default (q, mp, data)
	queue_t	* q;
	mblk_t *	mp;
	caddr_t	data;
{
	ENTER_FUNC(nd_get_default, q, mp, data, 0, 0, 0);
	LEAVE_FUNC(nd_get_default, EACCES);
	return EACCES;
}

/** This routine may be used as the get dispatch routine in nd tables
 ** for long variables.  To use this routine instead of a module
 ** specific routine, call nd_load as
 **	nd_load(&nd_ptr, "name", nd_get_long, set_pfi, &long_variable)
 ** The name of the variable followed by a space and the value of the
 ** variable will be printed in response to a get_status call.
 */
int
nd_get_long (q, mp, lp)
	queue_t	* q;
	mblk_t *	mp;
	ulong	* lp;
{
	ENTER_FUNC(nd_get_long, q, mp, lp, 0, 0, 0);

	mi_mpprintf(mp, "%ld", *lp);

	LEAVE_FUNC(nd_get_long, 0);
	return 0;
}

int
nd_get_names (q, mp, nd)
	queue_t	* q;
	mblk_t *	mp;
	ND	* nd;
{
reg	NDE	* nde;
	char	* rwtag;
	int	get_ok, set_ok;

        ENTER_FUNC(nd_get_names, q, mp, nd, 0, 0, 0);
	if (!nd) {
		LEAVE_FUNC(nd_get_names, ENOENT);
		return ENOENT;
	}
	for (nde = nd->nd_tbl; nde->nde_name; nde++) {
		get_ok = nde->nde_get_pfi != nd_get_default;
		set_ok = nde->nde_set_pfi != nd_set_default;
		if (get_ok) {
			if (set_ok)
				rwtag = "read and write";
			else
				rwtag = "read only";
		} else if (set_ok)
			rwtag = "write only";
		else
			rwtag = "no read or write";
		mi_mpprintf(mp, "%s (%s)", nde->nde_name, rwtag);
	}

        LEAVE_FUNC(nd_get_names, 0);
	return 0;
}

/** Load 'name' into the named dispatch table pointed to by 'ndp'.
 ** 'ndp' should be the address of a char pointer cell.  If the table
 ** does not exist (*ndp == 0), a new table is allocated and 'ndp'
 ** is stuffed.  If there is not enough space in the table for a new
 ** entry, more space is allocated.
 */
int
nd_load (nd_pparam, name, get_pfi, set_pfi, data)
	caddr_t	* nd_pparam;
	char	* name;
	int	(*get_pfi)();
	int	(*set_pfi)();
	caddr_t	data;
{
	ND	** ndp = (ND **)nd_pparam;
	ND	* nd;
reg	NDE	* nde;

	ENTER_FUNC(nd_load, nd_pparam, name, get_pfi, set_pfi, data, 0);
	if (!ndp) {
		LEAVE_FUNC(nd_load, FALSE);
		return FALSE;
	}
	if (!(nd = (ND *)*ndp)) {
		if (!(nd = (ND *)he_alloc(sizeof(ND), BPRI_MED))) {
			LEAVE_FUNC(nd_load, FALSE);
			return FALSE;
		}
		bzero((caddr_t)nd, sizeof(ND));
		*ndp = nd;
	}
	if (nd->nd_tbl) {
		for (nde = nd->nd_tbl; nde->nde_name; nde++) {
			if (strcmp(name, (char *)nde->nde_name) == 0)
				goto fill_it;
		}
	}
	if (nd->nd_free_count <= 1) {
		if (!(nde = (NDE *)he_alloc(nd->nd_size+NDE_ALLOC_SIZE, BPRI_MED))) {
			LEAVE_FUNC(nd_load, FALSE);
			return FALSE;
		}
		bzero((char *)nde, nd->nd_size + NDE_ALLOC_SIZE);
		nd->nd_free_count += NDE_ALLOC_COUNT;
		if (nd->nd_tbl) {
			bcopy((char *)nd->nd_tbl, (char *)nde, nd->nd_size);
			he_free((char *)nd->nd_tbl);
		} else {
			nd->nd_free_count--;
			nde->nde_name = "?";
			nde->nde_get_pfi = nd_get_names;
			nde->nde_set_pfi = nd_set_default;
		}
		nde->nde_data = (caddr_t)nd;
		nd->nd_tbl = nde;
		nd->nd_size += NDE_ALLOC_SIZE;
	}
	for (nde = nd->nd_tbl; nde->nde_name; nde++)
		;
	nd->nd_free_count--;
fill_it:
	nde->nde_name = name;
	nde->nde_get_pfi = get_pfi ? get_pfi : nd_get_default;
	nde->nde_set_pfi = set_pfi ? set_pfi : nd_set_default;
	nde->nde_data = data;

	LEAVE_FUNC(nd_load, TRUE);
	return TRUE;
}

int
nd_set_default (q, mp, value, data)
	queue_t	* q;
	mblk_t *	mp;
	char	* value;
	caddr_t	data;
{
	ENTER_FUNC(nd_set_default, q, mp, value, data, 0, 0);
	LEAVE_FUNC(nd_set_default, EACCES);
	return EACCES;
}

int
nd_set_long (q, mp, value, lp)
	queue_t	* q;
	mblk_t	* mp;
	char	* value;
	ulong	* lp;
{
	char	* cp;
	long	new_value;
	int	is_negative;

	ENTER_FUNC(nd_set_long, q, mp, value, lp, 0, 0);
	cp = value;
	while (*cp == ' '  ||  *cp == '\t'  ||  *cp == '\n')
		cp++;
	if (is_negative = (*cp == '-'))
		cp++;
	new_value = 0;
	for (; *cp; cp++) {
		if (*cp >= '0'  &&  *cp <= '9')
			new_value = (new_value * 10) + (*cp - '0');
		else
			break;
	}
	if (is_negative)
		new_value = -new_value;
	if (new_value == 0  &&  cp == value) {
		LEAVE_FUNC(nd_set_long, EINVAL);
		return EINVAL;
	}
	*lp = new_value;
	LEAVE_FUNC(nd_set_long, 0);
	return 0;
}

