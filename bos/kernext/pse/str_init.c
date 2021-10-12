static char sccsid[] = "@(#)21        1.13  src/bos/kernext/pse/str_init.c, sysxpse, bos412, 9446B 11/9/94 16:39:34";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      strmod_add
 *                 strmod_del
 *                 add_device
 *                 del_device
 *                 add_module
 *                 del_module
 *                 pse_init
 *                 pse_term
 *                 str_install
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
 **/

#include <sys/errno.h>
#include <sys/systemcfg.h>

#include <pse/str_stream.h>
#include <pse/str_config.h>
#include <pse/str_proto.h>

#include <sys/strconf.h>
#include <sys/stropts.h>

extern int str_count;
extern int init_state;
extern int thewall;
extern dev_t clonedev;
extern void str_modsw_term();
extern void str_open_term();
extern void sth_iocblk_term();

extern struct modsw** dbp_fmodsw;
extern struct modsw** dbp_dmodsw;
extern struct sth_s*** dbp_sth_open_streams;
extern struct msgb** dbp_mh_freelater;
extern struct sqh_s* dbp_streams_runq;

extern struct sqh_s streams_runq;
extern struct modsw* fmodsw;
extern struct modsw* dmodsw;
extern struct sth_s** sth_open_streams;
extern struct msgb* mh_freelater;


int throttle;
char *pse_stack[MAXCPU];                       /* private interrupt stack */
staticf	dev_t	add_device(dev_t, struct streamtab *, struct streamadm *);
staticf	dev_t	add_module(struct streamtab *, struct streamadm *);
staticf	int	del_device(dev_t, struct streamtab *, struct streamadm *);
staticf	int	del_module(struct streamtab *, struct streamadm *);

staticf struct streamtab str;

dev_t
strmod_add(devno, streamtab, streamadm)
	dev_t			devno;
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	switch (streamadm->sa_flags & STR_TYPE_MASK) {
	case STR_IS_DEVICE:
		return add_device(devno, streamtab, streamadm);
	case STR_IS_MODULE:
		return add_module(streamtab, streamadm);
	default:
		return ENODEV;
	}
}

int
strmod_del(devno, streamtab, streamadm)
	dev_t			devno;
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	switch (streamadm->sa_flags & STR_TYPE_MASK) {
	default:
		return EINVAL;
	case STR_IS_DEVICE:
		return del_device(devno, streamtab, streamadm);
	case STR_IS_MODULE:
		return del_module(streamtab, streamadm);
	}
}

static int	idnum = 0;
static int      did_clone = 0;

staticf dev_t
add_device(devno, streamtab, streamadm)
	dev_t			devno;
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	struct cdevsw		str_entry;

extern  int     nulldev();
extern  int     nodev();
        int             err;


        ENTER_FUNC(add_device, devno, streamtab, streamadm, 0, 0, 0);

        if (! streamtab) {
             str_entry.d_open   = pse_clone_open;
        } else {
             str_entry.d_open   = pse_open;
        }
	str_entry.d_close	= pse_close;
	str_entry.d_read	= pse_read;
	str_entry.d_write	= pse_write;
	str_entry.d_ioctl	= pse_ioctl;
        str_entry.d_strategy	= nodev;
	str_entry.d_ttys	= (struct tty *)streamadm->sa_ttys;
	str_entry.d_select	= (int)pse_select;
        str_entry.d_config	= nodev;
        str_entry.d_print	= nodev;
        str_entry.d_dump	= nodev;
	str_entry.d_mpx		= nodev;
        str_entry.d_revoke	= pse_revoke;
	str_entry.d_dsdptr	= NULL;
        str_entry.d_selptr	= NULL;
        str_entry.d_opts	= DEV_MPSAFE;

        /*
         * Must have a streamtab if not clone
         * Only one clone allowed
         */
        if (! streamtab ) {
		if (did_clone) {
                	LEAVE_FUNC(add_device, EINVAL);
                	return EINVAL;
		}
		if ( !(err = devswadd(devno, &str_entry))) did_clone = 1;
		LEAVE_FUNC(add_device, err);
		return err;
        }

	if (!(err = dmodsw_install(streamadm, streamtab, major(devno)))) {
		if ( !(err = devswadd(devno, &str_entry))) {
			if (!streamtab->st_rdinit->qi_minfo->mi_idnum)
			     streamtab->st_rdinit->qi_minfo->mi_idnum = ++idnum;
		}
		else (void)dmodsw_remove(streamadm->sa_name);
		LEAVE_FUNC(add_device, err);
		return err;
	}
	LEAVE_FUNC(add_device, err);
	return err;
}

staticf int
del_device(devno, streamtab, streamadm)
	dev_t			devno;
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	int error = 0;

	ENTER_FUNC(del_device, devno, streamtab, streamadm, 0, 0, 0);

        if (!streamtab) {
		if (did_clone) {
			did_clone = 0;
			error = devswdel(devno);
		}
		else error = EINVAL;
		LEAVE_FUNC(del_device, error);
		return error;
	}
	if (!(error = dmodsw_remove(streamadm->sa_name))) {
		error = devswdel(devno);
	}
	LEAVE_FUNC(del_device, error);
	return error;
}

staticf dev_t
add_module(streamtab, streamadm)
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	int error;

	ENTER_FUNC(add_module, streamtab, streamadm, 0, 0, 0, 0); 
	error = fmodsw_install( streamadm, streamtab );
	LEAVE_FUNC(add_module, error);
	return error;
}

staticf int
del_module(streamtab, streamadm)
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	int error;

	ENTER_FUNC(del_module, streamtab, streamadm, 0, 0, 0, 0);
	error = fmodsw_remove(streamadm->sa_name);
	LEAVE_FUNC(del_module, error);
	return error;
}

/*
 * pse_init - global initialization routine for streams module.
 */

int
pse_init ()
{
	int i;

	ENTER_FUNC(pse_init, 0, 0, 0, 0, 0, 0);
	sqh_init(&sq_sqh);
	sqh_init(&mult_sqh);
	str_to_init();
	act_q_init();
	sth_iocblk_init();
#ifdef	STREAMS_DEBUG
	DB_init();
#endif
	runq_init();
	(void) weldq_init();
	str_open_init();
	str_modsw_init();
	/* define lldb shaddow variables */
	dbp_fmodsw = &fmodsw;
	dbp_dmodsw = &dmodsw;
	dbp_sth_open_streams = &sth_open_streams;
	dbp_mh_freelater = &mh_freelater;
	dbp_streams_runq = &streams_runq;
	throttle = 922 * thewall;
	for (i=0; i < _system_configuration.ncpus; i++) {
        	if (!(pse_stack[i] = xmalloc((INTR_STACK), 2, pinned_heap))) 
               		 return(ENOMEM);
	}
        LEAVE_FUNC(pse_init, 0);
	return 0;
}

/*
 * pse_term - global termination routine for streams framework
 */

int
pse_term()
{
        ENTER_FUNC(pse_term, 0, 0, 0, 0, 0, 0);

	str_modsw_term();
	str_open_term();
        weldq_term();
        runq_term();
	sth_iocblk_term();
	sqh_term(&sq_sqh);
	sqh_term(&mult_sqh);
        act_q_term();
        str_to_term();
        assert(!xmfree(pse_stack, pinned_heap));

        LEAVE_FUNC(pse_term, 0);

	/* undefine lldb shaddow variables */
	dbp_fmodsw = NULL;
	dbp_dmodsw = NULL;
	dbp_sth_open_streams = NULL;
	dbp_mh_freelater = NULL;
	dbp_streams_runq = NULL;

        return 0;
}

int
str_install(cmd, cp)
        int cmd;
        strconf_t *cp;
{
        int err = 0;
        struct streamadm adm;

        dev_t devno = makedev(cp->sc_major, 0);

        ENTER_FUNC(str_install, cmd, cp, 0, 0, 0, 0);

        err = 0;

	if (!(init_state & INITDONE)) {
		err = ENOTREADY;
		goto out;
	}
	
	if (!(cp) || !(cp->sc_str)) {
		err = EINVAL;
		goto out;
	}

        if ((cmd == STR_LOAD_DEV || cmd == STR_UNLOAD_DEV)
					&& (cp->sc_major <= 0)) {
                err = EINVAL;
                goto out;
        }

	bzero((char *)&adm, sizeof(struct streamadm));

        adm.sa_ttys = nil(caddr_t);

	switch (cp->sc_sqlevel) {
	case SQLVL_DEFAULT:
		adm.sa_sync_level = SQLVL_MODULE;
		adm.sa_sync_info = nil(caddr_t);
		break;
	case SQLVL_ELSEWHERE:
		adm.sa_sync_level = cp->sc_sqlevel;
		adm.sa_sync_info = cp->sc_sqinfo;
		break;
	default:
		adm.sa_sync_level = cp->sc_sqlevel;
		adm.sa_sync_info = nil(caddr_t);
	}

        if (!(cp->sc_flags & STR_OLD_OPEN)) adm.sa_flags |= STR_SYSV4_OPEN;
        if (cp->sc_flags & STR_Q_NOTTOSPEC) 
		adm.sa_flags |= STR_NOTTOSPEC;

	/* 
	 * we assume weither STR_MPSAFE is set than sc_sqlevel is also
	 * set 
	 */

        if (cp->sc_flags & STR_MPSAFE) adm.sa_flags |= STR_IS_MPSAFE;
	else adm.sa_sync_level = SQLVL_MODULE;

        if (cp->sc_flags & STR_Q_SAFETY) adm.sa_flags |= STR_QSAFETY;
        strncpy(adm.sa_name,cp->sc_name,FMNAMESZ);

        switch(cmd) {
        case STR_LOAD_DEV:
		adm.sa_flags |= STR_IS_DEVICE;
		if (!(err = pincode(cp->sc_str))) {
		    if (err = strmod_add(devno, cp->sc_str, &adm))
			(void)unpincode(cp->sc_str);
		    else str_count ++;
                    strdev_print(&cp, cp->sc_name, err);
		}
                break;
        case STR_LOAD_MOD:
		adm.sa_flags |= STR_IS_MODULE;
		if (!(err = pincode(cp->sc_str))) {
		    if (err = strmod_add(devno, cp->sc_str, &adm))
			(void)unpincode(cp->sc_str);
		    else str_count ++;
                    strmod_print(&cp, cp->sc_name, err);
		}
                break;
        case STR_UNLOAD_DEV:
		adm.sa_flags |= STR_IS_DEVICE;
                err = strmod_del(devno, cp->sc_str, &adm);
                strdev_print_term(&cp, cp->sc_name, err);
		if (err) goto out;
		(void)unpincode(cp->sc_str);
		str_count --;
                break;
        case STR_UNLOAD_MOD:
		adm.sa_flags |= STR_IS_MODULE;
                err = strmod_del(devno, cp->sc_str,&adm);
                strmod_print_term(&cp, cp->sc_name, err);
		if (err) goto out;
		(void)unpincode(cp->sc_str);
		str_count --;
                break;
        default:
                err = EINVAL;
        }
out:
        LEAVE_FUNC(str_install, err);
        return err ;
}
