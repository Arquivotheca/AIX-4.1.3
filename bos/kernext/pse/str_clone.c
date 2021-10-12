static char sccsid[] = "@(#)11        1.15  src/bos/kernext/pse/str_clone.c, sysxpse, bos411, 9428A410j 7/1/94 16:13:22";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      clone_configure
 *                 pse_clone_open
 *                 
 * 
 * ORIGINS: 83
 * 
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <pse/str_stream.h>
#include <pse/str_config.h>
#include <pse/str_proto.h>
#include <pse/str_ioctl.h>
#include <sys/stropts.h>
#include <sys/termio.h>
#include <sys/trchkid.h>

dev_t	clonedev;

int
clone_configure(op, indata, indatalen, outdata, outdatalen)
	int op;
	str_config_t *  indata;
        size_t          indatalen;
        str_config_t *  outdata;
        size_t          outdatalen;
{
	struct streamadm        sa;
	dev_t			devno;
	int			error;

	 ENTER_FUNC(clone_configure, op, indata, indatalen, outdata, outdatalen, 0);
	sa.sa_flags	    = STR_IS_DEVICE | STR_SYSV4_OPEN | STR_IS_MPSAFE;
	sa.sa_ttys	    = nil(caddr_t);
	sa.sa_sync_level    = SQLVL_QUEUE;
	sa.sa_sync_info	    = nil(caddr_t);
	strcpy(sa.sa_name,  "clone");

	switch(op)
	{
	case CFG_INIT:
		error = strmod_add(indata->sc_devnum, (struct streamtab *) NULL, &sa);
		if (error) {
			LEAVE_FUNC(clone_configure, error);
			return error;
		}
		
		clonedev = indata->sc_devnum;

		break;
	case CFG_TERM:
		error = strmod_del(clonedev, (struct streamtab *) NULL, &sa);
		if (error) {
			LEAVE_FUNC(clone_configure, error);
			return error;
		}

		break;
	default:
		LEAVE_FUNC(clone_configure, EINVAL);
		return EINVAL;
	}

	if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
		outdata->sc_devnum = clonedev;
		outdata->sc_sa_flags = sa.sa_flags;
		strcpy(outdata->sc_sa_name, sa.sa_name);
	}
	LEAVE_FUNC(clone_configure, 0);
	return 0;
}

/*
 * pse_clone_open - normal clone open of a streams device.
 *
 * This level distinguishes the various open cases
 *
 *		first open of a new device
 *
 */

int
pse_clone_open (dev, flags, private, newdev)
	dev_t	dev;
	int	flags;
	void	**private;
	dev_t	*newdev;
{
	OSRP	osr;
	STHP	sth = NULL;
	int	reopen, error = 0;
	STHPP   sthp;
	DISABLE_LOCK_DECL
extern STHP sth_open_streams[];
extern simple_lock_data_t streams_open_lock;

#define	STH_HASH(dev)		&sth_open_streams\
	[((unsigned)major(dev) ^ (unsigned)minor(dev)) % STH_HASH_TBL_SIZE]

	ENTER_FUNC(pse_clone_open, dev, flags, private, newdev, 0, 0);
	
	TRCHKL4T(HKWD_PSE | hkwd_pse_clone_in, dev, flags, private, newdev);

	if (!(flags & O_DOCLONE))
	{
		error = ECLONEME;
		goto out;
	}

	dev = makedev(minor(dev), 0);

	if (error = sth_test_and_set_sth(dev, TRUE, &sth, &reopen))
		goto out;

	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_open_dev    = dev;
	osr->osr_open_fflag  = flags;
	osr->osr_creds = crref();

	if (flags & O_NDELAY)
		osr->osr_flags |= F_OSR_NDELAY;
	if (flags & O_NONBLOCK)
		osr->osr_flags |= F_OSR_NONBLOCK;

	/*
	 * New open goes via osr_open, with CLONEOPEN flag.
	 */
	if ((error = osr_open(osr)) != 0) {
		osr_free(osr);
		(void) modsw_ref(RD(sth->sth_wq->q_next)->q_qinfo, -1);
		q_free(RD(sth->sth_wq->q_next));
		q_free(sth->sth_rq);
		sth_free(sth);
		goto out;
	}

	osr->osr_open_dev = makedev(major(dev), minor(osr->osr_open_dev));
	sth->sth_dev = osr->osr_open_dev;
		
	DISABLE_LOCK(&streams_open_lock);
	sthp = STH_HASH(sth->sth_dev);
	sth->sth_next = *sthp;
	*sthp = sth;
	DISABLE_UNLOCK(&streams_open_lock);

	if (error = osr_add_modules(osr)) {
		osr_free(osr);
		(void)pse_close((dev_t)-1,sth);
		goto out;
	}

	*private = (void *)sth;
	*newdev = sth->sth_dev;

	/*
	 * While we were opening the stream, someone might have
	 * notified us that we are a controlling tty.
	 * We then have to do some special business...
	 */
	if ( !(flags & O_NOCTTY) && (sth->sth_flags & F_STH_ISATTY) ) {
		csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
			error = sth_ttyopen(sth, flags);
		csq_release(&sth->sth_rq->q_sqh);
		if (error) {
			 osr_free(osr);
			 (void)pse_close((dev_t)-1,sth);
			 goto out;
		}
	}
	osr_free(osr);

	DISABLE_LOCK(&streams_open_lock);
	sth->sth_open_count--;
	DISABLE_UNLOCK(&streams_open_lock);

	DB_isopen(sth);
	DB_check_streams("CLONE_OPEN");

out:
	TRCHKL1T(HKWD_PSE | hkwd_pse_clone_out, error);

	LEAVE_FUNC(pse_clone_open, error);
	return error;
}
