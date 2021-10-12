static char sccsid[] = "@(#)24        1.12  src/bos/kernext/pse/str_modsw.c, sysxpse, bos41J, 9521B_all 5/25/95 15:32:41";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      str_modsw_init
 *                 str_modsw_term
 *                 sqe_add
 *                 sqe_del
 *                 modsw_ref
 *                 dcookie_to_dindex
 *                 dindex_to_str
 *                 dname_to_dcookie
 *                 dname_to_dindex
 *                 dname_to_str
 *                 dqinfo_to_str 
 *                 fname_to_str
 *                 dmodsw_install
 *                 dmodsw_search
 *                 dmodsw_remove
 *                 fmodsw_install
 *                 fmodsw_remove
 *                 qinfo_to_name
 *                 mid_to_str
 *                 sqh_set_parent
 *                 modsw_next
 *                 dmodsw_next
 *                 fmodsw_next
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
 **/

#include <pse/str_stream.h>
#include <pse/str_config.h>
#include <pse/str_proto.h>
#include <sys/systemcfg.h>
#include <sys/stropts.h>
#include <pse/str_funnel.h>

struct modsw *	dmodsw;
struct modsw *	fmodsw;
simple_lock_data_t modsw_lock = {SIMPLE_LOCK_AVAIL};

SQH		sq_sqh;		/* Global Synch Queue */

/* Allocated when SQLVL_ELSEWHERE requested */
struct sq_elsewhere {
	struct	sq_elsewhere *sqe_next;
	struct	sq_elsewhere *sqe_prev;
	char	sqe_info[FMNAMESZ+1];
	int	sqe_refcnt;
	SQH	sqe_sqh;
} *sqe_list;

staticf SQHP sqe_add(caddr_t);
staticf void sqe_del(SQHP);

void
str_modsw_init ()
{
	ENTER_FUNC(str_modsw_init, 0, 0, 0, 0, 0, 0);
	lock_alloc((&modsw_lock), LOCK_ALLOC_PIN, PSE_MODSW_LOCK, -1);
	simple_lock_init(&modsw_lock);
	LEAVE_FUNC(str_modsw_init, 0);
}

void
str_modsw_term ()
{
	lock_free(&modsw_lock);
}


/** Allocate and/or reference sq_elsewhere by name */
staticf SQHP
sqe_add (info)
	caddr_t info;
{
	struct sq_elsewhere *sqe, *new_sqe;;

	ENTER_FUNC(sqe_add, info, 0, 0, 0, 0, 0);
	if (info == 0) {
		LEAVE_FUNC(sqe_add, 0);
		return 0;
	}
	NET_MALLOC(new_sqe, struct sq_elsewhere *, sizeof *sqe,
					M_STRSQ, M_WAITOK);
	SIMPLE_LOCK(&modsw_lock);
	sqe = sqe_list;
	while (sqe) {
		if (bcmp(info, sqe->sqe_info, FMNAMESZ) == 0)
			break;
		if ((sqe = sqe->sqe_next) == sqe_list) {
			sqe = 0;
			break;
		}
	}
	if (sqe == 0 && (sqe = new_sqe)) {
		new_sqe = 0;
		bzero((caddr_t)sqe, sizeof *sqe);
		bcopy(info, sqe->sqe_info, FMNAMESZ);
		sqh_init(&sqe->sqe_sqh);
		if (sqe_list)
			insque(sqe, sqe_list->sqe_prev);
		else
			sqe_list = sqe->sqe_next = sqe->sqe_prev = sqe;
	}
	if (sqe)
		++sqe->sqe_refcnt;
	SIMPLE_UNLOCK(&modsw_lock);
	if (new_sqe)
		NET_FREE(new_sqe, M_STRSQ);
	LEAVE_FUNC(sqe_add, sqe ? &sqe->sqe_sqh : 0); 
	return (sqe ? &sqe->sqe_sqh : 0);
}

/** Dereference and/or deallocate sq_elsewhere by SQH */
staticf void
sqe_del (sqh)
	SQHP sqh;
{
	struct sq_elsewhere *sqe;

	ENTER_FUNC(sqe_del, sqh, 0, 0, 0, 0, 0);

	SIMPLE_LOCK(&modsw_lock);
	sqe = sqe_list;
	while (sqe) {
		if (sqh == &sqe->sqe_sqh)
			break;
		if ((sqe = sqe->sqe_next) == sqe_list) {
			sqe = 0;
			break;
		}
	}
	if (sqe && --sqe->sqe_refcnt <= 0) {
		if (sqe == sqe_list) {
			sqe_list = sqe->sqe_next;
			if (sqe == sqe_list)
				sqe_list = 0;
		}
		remque(sqe);
	}
	SIMPLE_UNLOCK(&modsw_lock);
	if (sqe)
		NET_FREE(sqe, M_STRSQ);

	LEAVE_FUNC(sqe_del, 0);
}

/** Take or release reference on driver or module */
int
modsw_ref (qi, count)
	struct qinit *	qi;
	int		count;
{
reg	struct modsw	* dmp;
	int	ret = 0;

	ENTER_FUNC(modsw_ref, qi, count, 0, 0, 0, 0);
	SIMPLE_LOCK(&modsw_lock);
	/*
	 * Qi may well be shared. Take or remove all references.
	 */
	dmp = fmodsw;
	while (dmp) {
		if (dmp->d_str->st_rdinit == qi) {
			dmp->d_refcnt += count;
			++ret;
		}
		if ((dmp = dmp->d_next) == fmodsw)
			break;
	}
	dmp = dmodsw;
	while (dmp) {
		if (dmp->d_str->st_rdinit == qi) {
			dmp->d_refcnt += count;
			++ret;
		}
		if ((dmp = dmp->d_next) == dmodsw)
			break;
	}
	SIMPLE_UNLOCK(&modsw_lock);
	LEAVE_FUNC(modsw_ref, ret);
	return ret;
}

/** Convert cookie to device index - used to verify in table. */
int
dcookie_to_dindex (cookie)
	int	cookie;
{
reg	struct modsw	* dmp;

	ENTER_FUNC(dcookie_to_dindex, cookie, 0, 0, 0, 0, 0);

	if (cookie < 0) {
		LEAVE_FUNC(dcookie_to_dindex, -1);
		return -1;
	}
	SIMPLE_LOCK(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if ( cookie == dmp->d_major )
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	SIMPLE_UNLOCK(&modsw_lock);
	LEAVE_FUNC(dcookie_to_dindex, (dmp ? cookie : -1));
	return (dmp ? cookie : -1);
}

/** Convert device index to streamtab */
struct streamtab *
dindex_to_str (index)
	int	index;
{
reg	struct modsw	* dmp;

	ENTER_FUNC(dindex_to_str, index, 0, 0, 0, 0, 0);
	if (index < 0) {
		LEAVE_FUNC(dindex_to_str, 0);
		return 0;
	}
	SIMPLE_LOCK(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if ( index == dmp->d_major )
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	SIMPLE_UNLOCK(&modsw_lock);
	if (dmp) {
          LEAVE_FUNC(dindex_to_str, dmp->d_str);
        }
        else LEAVE_FUNC(dindex_to_str, 0);
	return (dmp ? dmp->d_str : 0);
}

/** Convert device name to device cookie */
int
dname_to_dcookie (name)
	char	* name;
{
reg	struct modsw	* dmp;

        ENTER_FUNC(dname_to_dcookie, name, 0, 0, 0, 0, 0);

	SIMPLE_LOCK(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if ( strcmp(name, dmp->d_name) == 0 )
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	SIMPLE_UNLOCK(&modsw_lock);
        LEAVE_FUNC(dname_to_dcookie, (dmp ? dmp->d_major : -1));
	return (dmp ? dmp->d_major : -1);
}

/** Convert device name to device index */
int
dname_to_dindex (name)
	char	* name;
{
	ENTER_FUNC(dname_to_dindex, name, 0, 0, 0, 0, 0);
	LEAVE_FUNC(dname_to_dcookie, 0);
	return dname_to_dcookie(name);
}

/** Convert device name to streamtab */
struct streamtab *
dname_to_str (name)
	char	* name;
{
reg	struct modsw	* dmp;

        ENTER_FUNC(dname_to_str, name, 0, 0, 0, 0, 0);

	SIMPLE_LOCK(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if ( strcmp(name, dmp->d_name) == 0 )
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	/* If no match, try for an unambiguous idname */
	if (dmp == 0) {
		struct modsw *maybe = 0;
		dmp = dmodsw;
		while (dmp) {
			if (dmp->d_str->st_rdinit
			&&  dmp->d_str->st_rdinit->qi_minfo
			&&  dmp->d_str->st_rdinit->qi_minfo->mi_idname
			&&  strcmp(name, dmp->d_str->st_rdinit->qi_minfo->mi_idname) == 0) {
				if (maybe) {
					dmp = 0;
					break;
				}
				maybe = dmp;
			}
			if ((dmp = dmp->d_next) == dmodsw) {
				dmp = maybe;
				break;
			}
		}
	}
	SIMPLE_UNLOCK(&modsw_lock);
        LEAVE_FUNC(dname_to_str, (dmp ? dmp->d_str : 0));
	return (dmp ? dmp->d_str : 0);
}

/** Convert rqinit pointer to streamtab from dmodsw */
struct streamtab *
dqinfo_to_str (qi, o_st)
	struct qinit	* qi;
	struct streamtab **o_st;
{
reg	struct modsw	* dmp;

        ENTER_FUNC(dqinfo_to_str, qi, o_st, 0, 0, 0, 0);

	SIMPLE_LOCK(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if (dmp->d_str->st_rdinit == qi)
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	SIMPLE_UNLOCK(&modsw_lock);
        LEAVE_FUNC(dqinfo_to_str, (dmp ? dmp->d_str : 0));

	/*
	 * In case of multiplexor drivers the true streamtab pointer is given
	 * d_funnel_str contains it.  Save off the original streamtab also
	 * because csq_newparent() needs to pass the original streamtab on to
	 * sqh_set_parent() so that for a multiplexor, the streamtab can be
	 * found in the dmodsw.  This is necessary in order to run a
	 * multiplexor funneled.
	 */
	if (dmp)
		*o_st = dmp->d_str;
	return (dmp ? dmp->d_funnel_str : 0);
}

/** Convert module name to streamtab */
struct streamtab *
fname_to_str (name)
	char	* name;
{
	struct modsw	* fmp;

	ENTER_FUNC(fname_to_str, name, 0, 0, 0 ,0, 0);

	if (name == 0) {
		LEAVE_FUNC(fname_to_str, 0);
		return 0;
	}
	SIMPLE_LOCK(&modsw_lock);
	fmp = fmodsw;
	while (fmp) {
		if (strcmp(fmp->d_name, name) == 0)
			break;
		if ((fmp = fmp->d_next) == fmodsw) {
			fmp = 0;
			break;
		}
	}
	/* If no match, try for an unambiguous idname */
	if (fmp == 0) {
		struct modsw *maybe = 0;
		fmp = fmodsw;
		while (fmp) {
			if (fmp->d_str->st_rdinit
			&&  fmp->d_str->st_rdinit->qi_minfo
			&&  fmp->d_str->st_rdinit->qi_minfo->mi_idname
			&&  strcmp(name, fmp->d_str->st_rdinit->qi_minfo->mi_idname) == 0) {
				if (maybe) {
					fmp = 0;
					break;
				}
				maybe = fmp;
			}
			if ((fmp = fmp->d_next) == fmodsw) {
				fmp = maybe;
				break;
			}
		}
	}
	SIMPLE_UNLOCK(&modsw_lock);
	LEAVE_FUNC(fname_to_str, (fmp ? fmp->d_str : 0));
	return (fmp ? fmp->d_str : 0);
}

int
dmodsw_install (adm, str, cookie)
	struct streamadm	* adm;
	struct streamtab	* str;
	int	cookie;
{
reg	struct modsw	* dmp, * new_dmp;
	SQHP	sqh;
	int	error = 0;

	ENTER_FUNC(dmodsw_install, adm, str, cookie, 0, 0, 0);

	if ( !str || !adm ) {
		STR_DEBUG(printf("dmodsw_install: nil streamadm/streamtab pointer for driver '%s'\n", adm ? adm->sa_name : "???"));
		LEAVE_FUNC(dmodsw_install, EINVAL);
		return EINVAL;
	}
	if (strlen(adm->sa_name) > FMNAMESZ) {
		STR_DEBUG(printf("dmodsw_install: name too long\n"));
		LEAVE_FUNC(dmodsw_install, EINVAL);
		return EINVAL;
	}

	NET_MALLOC(new_dmp, struct modsw *, sizeof *dmp, M_STRMODSW, M_WAITOK);
	switch (adm->sa_sync_level) {
	case SQLVL_ELSEWHERE:
		sqh = sqe_add(adm->sa_sync_info);
		break;
	case SQLVL_MODULE:
		NET_MALLOC(sqh, SQHP, sizeof *sqh, M_STRSQ, M_WAITOK);
		break;
	default:
		sqh = 0;
		break;
	}
	SIMPLE_LOCK(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if (strcmp(adm->sa_name, dmp->d_name) == 0)
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	if (dmp) {
		error = EEXIST;
		goto out;
	}
	if ((dmp = new_dmp) == 0) {
		error = ENOMEM;
		goto out;
	}
	new_dmp = 0;
	bzero((caddr_t)dmp, sizeof *dmp);
	bcopy(adm->sa_name, dmp->d_name, FMNAMESZ);
	dmp->d_str = str;
	dmp->d_funnel_str = str;
	if (__power_mp()) {
		if (adm->sa_flags & STR_IS_MPSAFE) {
			dmp->d_flags |= F_MODSW_MPSAFE;
			if (adm->sa_flags & STR_QSAFETY)
				dmp->d_flags |= F_MODSW_QSAFETY;
		} else {
			if (error = funnel_init(&dmp->d_str, adm))
				goto out;
			dmp->d_flags |= F_MODSW_QSAFETY;
		}
	}
	dmp->d_major = cookie;
	if ((adm->sa_flags & STR_SYSV4_OPEN) == 0)
		dmp->d_flags |= F_MODSW_OLD_OPEN;
	if (adm->sa_flags & STR_NOTTOSPEC)
                dmp->d_flags |= F_MODSW_NOTTOSPEC;
	switch (dmp->d_sq_level = adm->sa_sync_level) {
	case SQLVL_MODULE:
		if (sqh) sqh_init(sqh);
		/* fall through */
	case SQLVL_ELSEWHERE:
		if (sqh == 0) {
			error = ENOMEM;
			goto out;
		}
		dmp->d_sqh = sqh;
		sqh = 0;
		break;
	}
	if (dmodsw)
		insque(dmp, dmodsw->d_prev);
	else
		dmodsw = dmp->d_next = dmp->d_prev = dmp;
out:
	SIMPLE_UNLOCK(&modsw_lock);
	if (sqh) {
		switch (adm->sa_sync_level) {
		case SQLVL_ELSEWHERE:
			sqe_del(sqh);
			break;
		case SQLVL_MODULE:
			NET_FREE(sqh, M_STRSQ);
			break;
		}
	}
	if (new_dmp)
		NET_FREE(new_dmp, M_STRMODSW);
	LEAVE_FUNC(dmodsw_install, error);
	return error;
}

int
dmodsw_search()
{
reg     struct modsw    * dmp;
        int     error = 0;

        ENTER_FUNC(dmodsw_search, 0, 0, 0, 0, 0, 0);

        SIMPLE_LOCK(&modsw_lock);
        dmp = dmodsw;
        while (dmp) {
                if (dmp->d_refcnt > 0) {
                        error = EBUSY;
                        break;
                }
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
        SIMPLE_UNLOCK(&modsw_lock);

        LEAVE_FUNC(dmodsw_search, error);
	return error;
}

int
dmodsw_remove (name)
	char	* name;
{
reg	struct modsw	* dmp;
	int	error = 0;

	ENTER_FUNC(dmodsw_remove, name, 0, 0, 0, 0, 0);

	SIMPLE_LOCK(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if ( strcmp(name, dmp->d_name) == 0 ) {
			if (dmp->d_refcnt > 0) {
				error = EBUSY;
				break;
			}
			if (dmp == dmodsw) {
				dmodsw = dmp->d_next;
				if (dmp == dmodsw)
					dmodsw = 0;
			}
			remque(dmp);
			break;
		} 
		if ((dmp = dmp->d_next) == dmodsw) {
			error = ENOENT;
			break;
		}
	}
	SIMPLE_UNLOCK(&modsw_lock);
	if (error == 0) {
		if (__power_mp()) {
			if (!(dmp->d_flags & F_MODSW_MPSAFE)) {
				funnel_term(dmp->d_str);
				dmp->d_str = dmp->d_funnel_str;
			}
		}
		switch (dmp->d_sq_level) {
		case SQLVL_ELSEWHERE:
			sqe_del(dmp->d_sqh);
			break;
		case SQLVL_MODULE:
			NET_FREE(dmp->d_sqh, M_STRSQ);
			break;
		}
		NET_FREE(dmp, M_STRMODSW);
	}
	LEAVE_FUNC(dmodsw_remove, error);
	return error;
}

int
fmodsw_install (adm, str)
	struct streamadm	* adm;
	struct streamtab	* str;
{
reg	struct modsw	* fmp, * new_fmp;
	SQHP	sqh;
	int	error = 0;

        ENTER_FUNC(fmodsw_install, adm, str, 0, 0, 0, 0);
	if ( !str || !adm ) {
		STR_DEBUG(printf("fmodsw_install: nil streamadm/streamtab pointer for module '%s'\n", adm ? adm->sa_name : "???"));
		LEAVE_FUNC(fmodsw_install, EINVAL);
		return EINVAL;
	}
	if (strlen(adm->sa_name) > FMNAMESZ) {
		STR_DEBUG(printf("fmodsw_install: name too long\n"));
		LEAVE_FUNC(fmodsw_install, EINVAL);
		return EINVAL;
	}

	NET_MALLOC(new_fmp, struct modsw *, sizeof *fmp, M_STRMODSW, M_WAITOK);
	switch (adm->sa_sync_level) {
	case SQLVL_ELSEWHERE:
		sqh = sqe_add(adm->sa_sync_info);
		break;
	case SQLVL_MODULE:
		NET_MALLOC(sqh, SQHP, sizeof *sqh, M_STRSQ, M_WAITOK);
		break;
	default:
		sqh = 0;
		break;
	}
	SIMPLE_LOCK(&modsw_lock);
	fmp = fmodsw;
	while (fmp) {
		if (strcmp(adm->sa_name, fmp->d_name) == 0)
			break;
		if ((fmp = fmp->d_next) == fmodsw) {
			fmp = 0;
			break;
		}
	}
	if (fmp) {
		error = EEXIST;
		goto out;
	}
	if ((fmp = new_fmp) == 0) {
		error = ENOMEM;
		goto out;
	}
	new_fmp = 0;
	bzero((caddr_t)fmp, sizeof *fmp);
	bcopy(adm->sa_name, fmp->d_name, FMNAMESZ);
	fmp->d_str = str;
	fmp->d_major = NODEV;
	if ((adm->sa_flags & STR_SYSV4_OPEN) == 0)
		fmp->d_flags |= F_MODSW_OLD_OPEN;
	if (adm->sa_flags & STR_QSAFETY)
		fmp->d_flags |= F_MODSW_QSAFETY;
	if (adm->sa_flags & STR_NOTTOSPEC)
		fmp->d_flags |= F_MODSW_NOTTOSPEC;
	switch (fmp->d_sq_level = adm->sa_sync_level) {
	case SQLVL_MODULE:
		if (sqh) sqh_init(sqh);
		/* fall through */
	case SQLVL_ELSEWHERE:
		if (sqh == 0) {
			error = ENOMEM;
			goto out;
		}
		fmp->d_sqh = sqh;
		sqh = 0;
		break;
	}
	if (fmodsw)
		insque(fmp, fmodsw->d_prev);
	else
		fmodsw = fmp->d_next = fmp->d_prev = fmp;
out:
	SIMPLE_UNLOCK(&modsw_lock);
	if (sqh) {
		switch (adm->sa_sync_level) {
		case SQLVL_ELSEWHERE:
			sqe_del(sqh);
			break;
		case SQLVL_MODULE:
			NET_FREE(sqh, M_STRSQ);
			break;
		}
	}
	if (new_fmp)
		NET_FREE(new_fmp, M_STRMODSW);
        LEAVE_FUNC(fmodsw_install, error);
	return error;
}

int
fmodsw_remove (name)
	char	* name;
{
	struct modsw	* fmp;
	int	error = 0;

	ENTER_FUNC(fmodsw_remove, name, 0, 0, 0, 0, 0);

	SIMPLE_LOCK(&modsw_lock);
	fmp = fmodsw;
	while (fmp) {
		if ( strcmp(name, fmp->d_name) == 0 ) {
			if (fmp->d_refcnt > 0) {
				error = EBUSY;
				break;
			}
			if (fmp == fmodsw) {
				fmodsw = fmp->d_next;
				if (fmp == fmodsw)
					fmodsw = 0;
			}
			remque(fmp);
			break;
		}
		if ((fmp = fmp->d_next) == fmodsw) {
			error = ENOENT;
			break;
		}
	}
	SIMPLE_UNLOCK(&modsw_lock);
	if (error == 0) {
		switch (fmp->d_sq_level) {
		case SQLVL_ELSEWHERE:
			sqe_del(fmp->d_sqh);
			break;
		case SQLVL_MODULE:
			NET_FREE(fmp->d_sqh, M_STRSQ);
			break;
		}
		NET_FREE(fmp, M_STRMODSW);
	}
	LEAVE_FUNC(fmodsw_remove, error);
	return error;
}

/** Convert rqinit pointer to fmodsw name */
char *
qinfo_to_name (qi)
	struct qinit	* qi;
{
reg	struct modsw	* fmp;

        ENTER_FUNC(qinfo_to_name, qi, 0, 0, 0, 0, 0);

	SIMPLE_LOCK(&modsw_lock);
	fmp = fmodsw;
	while (fmp) {
		if (fmp->d_str->st_rdinit == qi)
			break;
		if ((fmp = fmp->d_next) == fmodsw) {
			fmp = 0;
			break;
		}
	}
	SIMPLE_UNLOCK(&modsw_lock);

        LEAVE_FUNC(qinfo_to_name, (fmp ? fmp->d_name : 0));
	return (fmp ? fmp->d_name : 0);
}

struct streamtab *
mid_to_str (ushort mid)
{
reg	struct modsw	* dmp;
	struct streamtab	* str = 0;

        ENTER_FUNC(mid_to_str, mid, 0, 0, 0, 0, 0);

	SIMPLE_LOCK(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if (dmp->d_str->st_rdinit
		&&  dmp->d_str->st_rdinit->qi_minfo->mi_idnum == mid) {
			str = dmp->d_str;
			break;
		}
		if ((dmp = dmp->d_next) == dmodsw)
			break;
	}
	if (str == 0) {
		dmp = fmodsw;
		while (dmp) {
			if (dmp->d_str->st_rdinit
			&&  dmp->d_str->st_rdinit->qi_minfo->mi_idnum == mid) {
				str = dmp->d_str;
				break;
			}
			if ((dmp = dmp->d_next) == fmodsw)
				break;
		}
	}
	SIMPLE_UNLOCK(&modsw_lock);
        LEAVE_FUNC(mid_to_str, str);
	return str;
}

/*
 * Reset the synch queue subordination for a queue based on the
 * synchronization level corresponding to the specified
 * streamtab pointer.  A pointer to the new parent is returned.
 */
SQHP
sqh_set_parent (q, str)
	queue_t	*		q;
	struct streamtab *	str;
{
reg	struct modsw *		dmp;
	SQHP			sqhp;
	int			sq_lvl = 0;
	DISABLE_LOCK_DECL

	ENTER_FUNC(sqh_set_parent, q, str, 0, 0, 0, 0);

	if ( str == &sthinfo ) {
		sq_lvl = SQLVL_QUEUEPAIR;
		DISABLE_LOCK(&q->q_qlock);
		q->q_flag &= ~(QOLD|QSAFE);
		DISABLE_UNLOCK(&q->q_qlock);
	} else {
		/* Check for entry in fmodsw first... */
		SIMPLE_LOCK(&modsw_lock);
		dmp = fmodsw;
		while (dmp) {
			if (dmp->d_str == str)
				break;
			if ((dmp = dmp->d_next) == fmodsw) {
				dmp = 0;
				break;
			}
		}
		if (dmp) {
			sq_lvl = dmp->d_sq_level;
			sqhp = dmp->d_sqh;
		}
		if ( sq_lvl == 0 ) {
			/* Now try to find it in dmodsw... */
			dmp = dmodsw;
			while (dmp) {
				if (dmp->d_str == str)
					break;
				if ((dmp = dmp->d_next) == dmodsw) {
					dmp = 0;
					break;
				}
			}
			if (dmp) {
				sq_lvl = dmp->d_sq_level;
				sqhp = dmp->d_sqh;
				if (__power_mp() && !(dmp->d_flags & F_MODSW_MPSAFE)) {
					q->q_runq_sq->sq_entry = (sq_entry_t) funnel_sq_wrapper;
					q->q_flag |= QFUN;
				}
			}
		}
		/*
		 * Set qflag(s) per modsw.
		 */
		if (dmp) {
			DISABLE_LOCK(&q->q_qlock);
			if (dmp->d_flags & F_MODSW_OLD_OPEN)
				q->q_flag |= QOLD;
			if (dmp->d_flags & F_MODSW_QSAFETY)
				q->q_flag |= QSAFE;
			if (dmp->d_flags & F_MODSW_NOTTOSPEC) {
                                q->q_flag |= QNOTTOSPEC;
                                OTHERQ(q)->q_flag |= QNOTTOSPEC;
                        }
			DISABLE_UNLOCK(&q->q_qlock);
		}
		SIMPLE_UNLOCK(&modsw_lock);
	}

	switch (sq_lvl) {
	case SQLVL_QUEUE:
		sqhp = &q->q_sqh;
		break;
	case SQLVL_QUEUEPAIR:
		sqhp = (q->q_flag & QREADR) ? &q->q_sqh : &RD(q)->q_sqh;
		break;
	case SQLVL_ELSEWHERE:
	case SQLVL_MODULE:
		break;
	case SQLVL_DEFAULT:
	case SQLVL_GLOBAL:
	default:
		sqhp = &sq_sqh;
		break;
	}
	q->q_sqh.sqh_parent = sqhp;

        LEAVE_FUNC(sqh_set_parent, sqhp);
	return sqhp;
}

/*
 * Following lookups used by "sc" to find all modules/devices.
 */

staticf int
modsw_next (modsw, nextp, namep)
	struct modsw **modsw;
	void **	nextp;
	char **	namep;
{
	struct modsw *dmp;

	ENTER_FUNC(modsw_next, modsw, nextp, namep, 0, 0, 0);

	SIMPLE_LOCK(&modsw_lock);
	dmp = *modsw;
	/* Get back to previous "next". */
	if (*nextp) while (dmp) {
		if (dmp == (struct modsw *)(*nextp))
			break;
		if ((dmp = dmp->d_next) == *modsw) {
			dmp = 0;
			break;
		}
	}
	if (dmp) {
		*namep = dmp->d_name;
		if ((dmp = dmp->d_next) == *modsw)
			*nextp = (void *)-1;
		else
			*nextp = (void *)dmp;
	}
	SIMPLE_UNLOCK(&modsw_lock);

	LEAVE_FUNC(modsw_next, (dmp != 0));
	return (dmp != 0);
}

int
dmodsw_next (nextp, namep)
	void **	nextp;
	char **	namep;
{
	ENTER_FUNC(dmodsw_next, nextp, namep, 0, 0, 0, 0);
	LEAVE_FUNC(dmodsw_next, 0);
	return modsw_next(&dmodsw, nextp, namep);
}

int
fmodsw_next (nextp, namep)
	void **	nextp;
	char **	namep;
{
	ENTER_FUNC(fmodsw_next, nextp, namep, 0, 0, 0, 0);
	LEAVE_FUNC(fmodsw_next, 0);
	return modsw_next(&fmodsw, nextp, namep);
}
