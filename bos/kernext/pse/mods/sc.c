static char sccsid[] = "@(#)42        1.3  src/bos/kernext/pse/mods/sc.c, sysxpse, bos411, 9428A410j 8/27/93 09:40:59";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      sc_config
 *                 sc_close
 *                 sc_get_dinfo
 *                 sc_get_info
 *                 sc_get_minfo
 *                 sc_get_dnames
 *                 sc_get_mnames
 *                 sc_get_names
 *                 sc_open
 *                 sc_rput
 *                 sc_wput
 *                 get_device_names
 *                 get_module_names
 *                 get_mod_and_dev_names
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
/** Copyright (c) 1988-1991  Mentat Inc.
 ** sc.c 2.2, last change 11/19/90
 **/


#include <sys/stream.h>
#include <sys/strstat.h>
#include <sys/sysconfig.h>

#include <sys/device.h>
#include <sys/strconf.h>

#include <pse/nd.h>
#include <pse/str_debug.h>

#ifndef staticf
#define staticf static
#endif

typedef	struct iocblk	* IOCP;

extern	struct streamtab *	dname_to_str(char * name);
extern	struct streamtab *	fname_to_str(char * name);

#define mpsprintf	mi_mpprintf

staticf	int	sc_get_dinfo(   queue_t * q, mblk_t * mp, caddr_t data   );
staticf	int	sc_get_info(   queue_t * q, mblk_t * mp, caddr_t data   );
staticf	int	sc_get_minfo(   queue_t * q, mblk_t * mp, caddr_t data   );
staticf	int	sc_get_dnames(   queue_t * q, mblk_t * mp, caddr_t data   );
staticf	int	sc_get_mnames(   queue_t * q, mblk_t * mp, caddr_t data   );
staticf	int	sc_get_names(   queue_t * q, mblk_t * mp, caddr_t data   );
staticf	int	sc_open(queue_t * q, dev_t *dev, int flag, int sflag, cred_t *credp);
staticf	int	sc_wput(queue_t * q, mblk_t * mp);

staticf	int	sc_close(queue_t * q);
staticf	int	sc_rput(queue_t * q, mblk_t * mp);
staticf	void	get_device_names(struct msgb * mp);
staticf	void	get_module_names(struct msgb * mp);
staticf	void	get_mod_and_dev_names(struct msgb * mp);

static struct module_info minfo =  {
	5002, "sc", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	sc_rput, NULL, sc_open, sc_close, NULL, &minfo
};

static struct qinit winit = {
	sc_wput, NULL, NULL, NULL, NULL, &minfo
};

struct streamtab scinfo = { &rinit, &winit };

int
sc_config(cmd, uiop)
        int cmd;
        struct uio *uiop;
{
        static strconf_t conf = {
                "sc", &scinfo, STR_NEW_OPEN,
        };

        switch (cmd) {
        case CFG_INIT:  return str_install(STR_LOAD_MOD, &conf);
        case CFG_TERM:  return str_install(STR_UNLOAD_MOD, &conf);
        default:        return EINVAL;
        }
}

static	caddr_t	sc_g_nd;

staticf int
sc_close (q)
	queue_t	* q;
{
	return 0;
}

staticf int
sc_get_dinfo (q, mp, data)
	queue_t	* q;
	mblk_t	* mp;
	caddr_t	data;
{
	char	* name;
	char	* cp;
	int	dev_num;
	struct streamtab	* st;
	struct module_info	* minfo;
	struct module_stat	* mstat;

	for (name = (char *)mp->b_rptr; *name++; ) {
		if (name >= (char *)mp->b_datap->db_lim)
			return EINVAL;
	}
	st = dname_to_str(name);
	if ( !st )
		return ENOENT;
	cp = st->st_muxrinit ? "mux" : "device";

	dev_num = dname_to_dcookie(name);

	if (!st->st_rdinit
	||  !(minfo = st->st_rdinit->qi_minfo)) 
		return EINVAL;
	mpsprintf(mp, "%s", cp);
	mpsprintf(mp, "device number == %d", dev_num);
	mpsprintf(mp, "idnum == %d", minfo->mi_idnum);
	mpsprintf(mp, "idname == %s", minfo->mi_idname);
	mpsprintf(mp, "minpsz == %d", minfo->mi_minpsz);
	mpsprintf(mp, "maxpsz == %d", minfo->mi_maxpsz);
	mpsprintf(mp, "hiwat == %d", minfo->mi_hiwat);
	mpsprintf(mp, "lowat == %d", minfo->mi_lowat);
	if (mstat = st->st_rdinit->qi_mstat) {
		mpsprintf(mp, "put count == %ld", mstat->ms_pcnt);
		mpsprintf(mp, "service count == %ld", mstat->ms_scnt);
		mpsprintf(mp, "open count == %ld", mstat->ms_ocnt);
		mpsprintf(mp, "close count == %ld", mstat->ms_ccnt);
		mpsprintf(mp, "admin count == %ld", mstat->ms_acnt);
	}
	return 0;
}

staticf int
sc_get_info (q, mp, data)
	queue_t	* q;
	mblk_t	* mp;
	caddr_t	data;
{
	char	* name;
	char	* cp;
	int	dev_num;
	struct streamtab	* st;
	struct module_info	* minfo;
	struct module_stat	* mstat;

	for (name = (char *)mp->b_rptr; *name++; ) {
		if (name >= (char *)mp->b_datap->db_lim)
			return EINVAL;
	}
	dev_num = -1;
	if (st = fname_to_str(name))
		cp = "module";
	else if (st = dname_to_str(name)) {
		cp = st->st_muxrinit ? "mux" : "device";
		dev_num = dname_to_dcookie(name);
	} else 
	   return ENOENT;
	if (!st->st_rdinit
	||  !(minfo = st->st_rdinit->qi_minfo)) 
		return EINVAL;
	mpsprintf(mp, "%s", cp);
	if (dev_num != -1)
		mpsprintf(mp, "device number == %d", dev_num);
	mpsprintf(mp, "idnum == %d", minfo->mi_idnum);
	mpsprintf(mp, "idname == %s", minfo->mi_idname);
	mpsprintf(mp, "minpsz == %d", minfo->mi_minpsz);
	mpsprintf(mp, "maxpsz == %d", minfo->mi_maxpsz);
	mpsprintf(mp, "hiwat == %d", minfo->mi_hiwat);
	mpsprintf(mp, "lowat == %d", minfo->mi_lowat);
	if (mstat = st->st_rdinit->qi_mstat) {
		mpsprintf(mp, "put count == %ld", mstat->ms_pcnt);
		mpsprintf(mp, "service count == %ld", mstat->ms_scnt);
		mpsprintf(mp, "open count == %ld", mstat->ms_ocnt);
		mpsprintf(mp, "close count == %ld", mstat->ms_ccnt);
		mpsprintf(mp, "admin count == %ld", mstat->ms_acnt);
	}
	return 0;
}

staticf int
sc_get_minfo (q, mp, data)
	queue_t	* q;
	mblk_t	* mp;
	caddr_t	data;
{
	char	* name;
	char	* cp;
	struct streamtab	* st;
	struct module_info	* minfo;
	struct module_stat	* mstat;

	for (name = (char *)mp->b_rptr; *name++; ) {
		if (name >= (char *)mp->b_datap->db_lim)
			return EINVAL;
	}
	st = fname_to_str(name);
	if ( !st )
		return ENOENT;
	cp = "module";
	if (!st->st_rdinit
	||  !(minfo = st->st_rdinit->qi_minfo))
		return EINVAL;
	mpsprintf(mp, "%s", cp);
	mpsprintf(mp, "idnum == %d", minfo->mi_idnum);
	mpsprintf(mp, "idname == %s", minfo->mi_idname);
	mpsprintf(mp, "minpsz == %d", minfo->mi_minpsz);
	mpsprintf(mp, "maxpsz == %d", minfo->mi_maxpsz);
	mpsprintf(mp, "hiwat == %d", minfo->mi_hiwat);
	mpsprintf(mp, "lowat == %d", minfo->mi_lowat);
	if (mstat = st->st_rdinit->qi_mstat) {
		mpsprintf(mp, "put count == %ld", mstat->ms_pcnt);
		mpsprintf(mp, "service count == %ld", mstat->ms_scnt);
		mpsprintf(mp, "open count == %ld", mstat->ms_ocnt);
		mpsprintf(mp, "close count == %ld", mstat->ms_ccnt);
		mpsprintf(mp, "admin count == %ld", mstat->ms_acnt);
	}
	return 0;
}

staticf int
sc_get_dnames (q, mp, data)
	queue_t	* q;
	mblk_t	* mp;
	caddr_t	data;
{
	get_device_names(mp);
	return 0;
}

staticf int
sc_get_mnames (q, mp, data)
	queue_t	* q;
	mblk_t	* mp;
	caddr_t	data;
{
	get_module_names(mp);
	return 0;
}

staticf int
sc_get_names (q, mp, data)
	queue_t	* q;
	mblk_t	* mp;
	caddr_t	data;
{
	get_mod_and_dev_names(mp);
	return 0;
}

staticf int
sc_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{

	if (!sc_g_nd) {
		nd_load(&sc_g_nd, "names", sc_get_names, (int (*)())0, (caddr_t)0);
		nd_load(&sc_g_nd, "dnames", sc_get_dnames, (int (*)())0, (caddr_t)0);
		nd_load(&sc_g_nd, "mnames", sc_get_mnames, (int (*)())0, (caddr_t)0);
		nd_load(&sc_g_nd, "info", sc_get_info, (int (*)())0, (caddr_t)0);
		nd_load(&sc_g_nd, "dinfo", sc_get_dinfo, (int (*)())0, (caddr_t)0);
		nd_load(&sc_g_nd, "minfo", sc_get_minfo, (int (*)())0, (caddr_t)0);
	}
	return 0;
}

staticf int
sc_rput (q, mp)
	queue_t	* q;
	mblk_t *	mp;
{
	putnext(q, mp);
}

staticf int
sc_wput (q, mp)
	queue_t	* q;
	mblk_t *	mp;
{
	IOCP	iocp;

	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		iocp = (IOCP)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		case ND_GET:
		case ND_SET:
			if (nd_getset(q, sc_g_nd, mp)) {
				qreply(q, mp);
				return;
			}
			break;
		}
		/* FALLS THROUGH */
	default:
		putnext(q, mp);
		return;
	}
}

staticf void
get_device_names (mp)
	struct msgb	* mp;
{
	char *name;
	void *next = 0;

	while (dmodsw_next(&next, &name))
		mpsprintf(mp, "%s", name);
}

staticf void
get_module_names (mp)
	struct msgb	* mp;
{
	char *name;
	void *next = 0;

	while (fmodsw_next(&next, &name))
		mpsprintf(mp, "%s", name);
}

staticf void
get_mod_and_dev_names (mp)
	struct msgb	* mp;
{
	get_device_names(mp);
	get_module_names(mp);
}
