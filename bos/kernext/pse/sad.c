static char sccsid[] = "@(#)31        1.3.1.2  src/bos/kernext/pse/sad.c, sysxpse, bos411, 9428A410j 3/3/94 04:10:53";
/*
 * COMPONENT_NAME: SYSXPSE - STREAMS framework
 * 
 * FUNCTIONS:      sad_init
 *                 sad_term
 *                 sad_configure
 *                 sad_get_autopush
 *                 sad_close
 *                 sad_open 
 *                 sad_wput
 *                 
 * 
 * ORIGINS: 63, 71, 83 
 * 
 */

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/** Copyright (c) 1989-1991  Mentat Inc. **/
#include <pse/str_system.h>
#include <pse/str_lock.h>
#include <net/net_malloc.h>

#include <sys/strconf.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strstat.h>
#include <sys/sad.h>
#include <sys/lock_alloc.h>

#include <pse/str_config.h>
#include <pse/mi.h>

#ifndef staticf
#define staticf	static
#endif

#define A_CNT(arr)      (sizeof(arr)/sizeof(arr[0]))
#define	SAD_HASH(maj)	&sad_hash_tbl[((unsigned int)maj) % A_CNT(sad_hash_tbl)]

static caddr_t sad_g_head;     /* Head of linked list for mi_open_comm. */

typedef	struct msgb	* MBLKP;
typedef	struct msgb	** MBLKPP;

/* Structure of each element in the sad_hash_tbl */
typedef struct sad_hash_s {
	struct sad_hash_s	* sad_next;
	int			sad_privileged;
	struct strapush		sad_strapush;
} SAD, * SADP, ** SADPP;

staticf	int	sad_close(queue_t *, int, cred_t *);
staticf	int	sad_open(queue_t *, dev_t *, int, int, cred_t *);
staticf	int	sad_wput(queue_t *, MBLKP);

static struct module_info minfo =  {
#define	MODULE_ID	45
	MODULE_ID, "sad", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	NULL, NULL, sad_open, sad_close, NULL, &minfo
};

static struct qinit winit = {
	sad_wput, NULL, NULL, NULL, NULL, &minfo
};

struct streamtab sadinfo = { &rinit, &winit };

static	SADP	sad_hash_tbl[32];

/*
 * We depend on module synch to prevent multiple sad updates, but
 * still require a lock to protect sad_get_autopush() lookups.
 */
decl_simple_lock_data(static, sad_lock)

void
sad_init()
{
	lock_alloc((&sad_lock), LOCK_ALLOC_PIN, PSE_SAD_LOCK, -1);
	simple_lock_init(&sad_lock);
}

void
sad_term()
{
	lock_free(&sad_lock);
}

extern dev_t clonedev;

int
sad_configure (op, indata, indatalen, outdata, outdatalen)
	uint         op;
	str_config_t *  indata;
	size_t          indatalen;
	str_config_t *  outdata;
	size_t          outdatalen;
{
	struct streamadm        sa;
	static dev_t            devno;
	int                     error = 0;

        sa.sa_flags         = STR_IS_DEVICE | STR_SYSV4_OPEN | STR_IS_MPSAFE;
        sa.sa_ttys          = 0;
        sa.sa_sync_level    = SQLVL_MODULE;
        sa.sa_sync_info     = 0;
        strcpy(sa.sa_name,  "sad");

	switch (op) {
	case CFG_INIT:
	    sad_init();

	    if (error = strmod_add(indata->sc_devnum, &sadinfo, &sa)) {
		goto out;
	    }
	    devno = indata->sc_devnum;
	    break;
	case CFG_TERM:
	    if (( error = strmod_del(devno, &sadinfo, &sa))) goto out;
	    sad_term();
	    break;
	default:
	    error = EINVAL;
	    goto out;
	}
	if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
	    outdata->sc_devnum = makedev(major(clonedev), major(devno));
	    outdata->sc_sa_flags = sa.sa_flags;
	    strcpy(outdata->sc_sa_name,sa.sa_name);
	}

out:
	return error;

}

int
sad_get_autopush (major_num, minor_num, stra)
	long	major_num;
	long	minor_num;
	struct strapush * stra;
{
	SADP	sad;
	int	ret = 0;

	sad = *SAD_HASH(major_num);
	SIMPLE_LOCK(&sad_lock);
	for ( ; sad; sad = sad->sad_next) {
		if (sad->sad_strapush.sap_major == major_num
		&& (sad->sad_strapush.sap_minor == -1
		|| (sad->sad_strapush.sap_minor <= minor_num
		&&  sad->sad_strapush.sap_lastminor >= minor_num))) {
			if (stra)
				*stra = sad->sad_strapush;
			ret = 1;
			break;
		}
	}
	SIMPLE_UNLOCK(&sad_lock);
	return ret;
}

staticf int
sad_close (q, flag, credp)
	queue_t	* q;
	int	flag;
	cred_t	* credp;
{
        return mi_close_comm((caddr_t *)&sad_g_head, q);
}

staticf int
sad_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	int	err;
	err= mi_open_comm((caddr_t *)&sad_g_head, sizeof (int), q, devp, flag, sflag, credp);
	if (err == 0 && drv_priv(credp) == 0)
		*(int *)q->q_ptr = 1;
	return err;
}

staticf int
sad_wput (q, mp)
	queue_t	* q;
	MBLKP	mp;
{
	int	copyin_size;
	int	err;
	int	i1;
	struct iocblk	* iocp;
	MBLKP	mp1;
	struct str_list	* sl;
	struct str_mlist	* sml;
	struct strapush	* stra;
	SADP	sad;
	SADPP	sadp;

	switch (mp->b_datap->db_type) {
	case M_PCPROTO:
	case M_PROTO:
	case M_DATA:
		break;
	case M_IOCTL:
		/* All SAD IOCTLs are handled both in I_STR and 
		 * TRANSPARENT form using the sad_copy facility.
		 */
		iocp = (struct iocblk *)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		case SAD_SAP:
		case SAD_GAP:
			copyin_size = sizeof(struct strapush);
			break;
		case SAD_VML:
			copyin_size = sizeof(struct str_list);
			break;
		default:
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_count = 0;
			qreply(q, mp);
			return 0;
		}
		mi_copyin(q, mp, (char *)0, copyin_size);
		return 0;
	case M_IOCDATA:
		iocp = (struct iocblk *)mp->b_rptr;
		err = 0;
		switch ( mi_copy_state(q, mp, &mp1) ) {
		case -1:
			/* mi_copy_state cleans up and completes the ioctl. */
			return 0;
		case MI_COPY_CASE(MI_COPY_IN, 1):
			/* Completed first copyin. */
			switch ( iocp->ioc_cmd ) {
			case SAD_SAP:
				stra = (struct strapush *)mp1->b_rptr;
				switch (stra->sap_cmd) {
				case SAP_ONE:
					stra->sap_lastminor = stra->sap_minor;
					if (sad_get_autopush(stra->sap_major,
							stra->sap_minor,
							(struct strapush *)0)) {
						err = EEXIST;
						goto iocack;
					}
					break;
				case SAP_ALL:
					stra->sap_minor = -1;
					stra->sap_lastminor = -1;
					/* Check to make sure there are no
					 * settings for this major number */
					sad = *SAD_HASH(stra->sap_major);
					for ( ; sad; sad = sad->sad_next) {
						if (sad->sad_strapush.sap_major
						==  stra->sap_major) {
							err = EEXIST;
							goto iocack;
						}
					}
					break;
				case SAP_RANGE:
					if (stra->sap_lastminor <
							stra->sap_minor) {
						err = ERANGE;
						goto iocack;
					}
					/* Check to make sure there are no
					 * overlapping settings for this
					 * major number */
					sad = *SAD_HASH(stra->sap_major);
					for ( ; sad; sad = sad->sad_next) {
						if (sad->sad_strapush.sap_major
						!=  stra->sap_major)
							continue;
						if (stra->sap_lastminor >=
						    sad->sad_strapush.sap_minor
						||  stra->sap_minor <
						    sad->sad_strapush.sap_lastminor) {
							err = EEXIST;
							goto iocack;
						}
					}
					break;
				case SAP_CLEAR:
					sadp = SAD_HASH(stra->sap_major);
					SIMPLE_LOCK(&sad_lock);
for ( ; sad = *sadp; sadp = &sad->sad_next) {
	if (stra->sap_major == sad->sad_strapush.sap_major) {
		if (stra->sap_minor == sad->sad_strapush.sap_minor
		|| (stra->sap_minor == 0 && sad->sad_strapush.sap_minor == -1)){
			*sadp = sad->sad_next;
			SIMPLE_UNLOCK(&sad_lock);
			FREE(sad, M_STREAMS);
			goto iocack;
		}
		if (stra->sap_minor >= sad->sad_strapush.sap_minor
		&&  stra->sap_minor <= sad->sad_strapush.sap_lastminor) {
			err = ERANGE;
			SIMPLE_UNLOCK(&sad_lock);
			goto iocack;
		}
	}
}
					SIMPLE_UNLOCK(&sad_lock);
					err = ENODEV;
					goto iocack;
				default:
					err = EINVAL;
					goto iocack;
				}
				if (dcookie_to_dindex(stra->sap_major) == -1
				||  stra->sap_npush < 0
				||  stra->sap_npush > MAXAPUSH) {
					err = EINVAL;
					break;
				}
				for (i1 = 0; i1 < stra->sap_npush; i1++) {
					if (!fname_to_str(stra->sap_list[i1])) {
						err = EINVAL;
						goto iocack;
					}
				}
				MALLOC(sad,SADP,sizeof(SAD),M_STREAMS,M_NOWAIT);
				if (sad == NULL) {
					err = ENOSR;
					break;
				}
				sad->sad_privileged = *(int *)q->q_ptr;
				sad->sad_strapush = *stra;
				sadp = SAD_HASH(stra->sap_major);
				SIMPLE_LOCK(&sad_lock);
				sad->sad_next = *sadp;
				*sadp = sad;
				SIMPLE_UNLOCK(&sad_lock);
				break;
			case SAD_GAP:
				/* We have the input strapush structure, find
				 * the information requested and copyout */
				stra = (struct strapush *)mp1->b_rptr;
				if (sad_get_autopush(stra->sap_major,
						stra->sap_minor, stra)) {
					mp1->b_wptr = mp1->b_rptr +
							sizeof(struct strapush);
					mi_copyout(q, mp);
					return 0;
				}
				if (dcookie_to_dindex(stra->sap_major) == -1)
					err = ENOSTR;
				else
					err = ENODEV;
				break;
			case SAD_VML:
				sl = (struct str_list *)mp1->b_rptr;
				if (sl->sl_nmods <= 0) {
					err = EINVAL;
					break;
				}
				/* Copy in the module list. */
				mi_copyin(q, mp, (char *)sl->sl_modlist,
				    sl->sl_nmods * sizeof(struct str_mlist));
				return 0;
			default:
				err = EPROTO;
				goto iocack;
			}
			break;
		case MI_COPY_CASE(MI_COPY_IN, 2):
			/* Completed second copyin. */
			switch ( iocp->ioc_cmd ) {
			case SAD_VML:
				/* Now we have the module list. */
				if ( !mp->b_cont  ||  !mp->b_cont->b_cont ) {
					err = EPROTO;
					break;
				}
				sl = (struct str_list *)mp->b_cont->b_cont->b_rptr;
				sml = (struct str_mlist *)mp1->b_rptr;
				iocp->ioc_rval = 0;
				for (i1 = 0; i1 < sl->sl_nmods; i1++) {
					if (!fname_to_str(sml[i1].l_name)) {
						iocp->ioc_rval = 1;
						break;
					}
				}
				break;
			default:
				err = EPROTO;
				break;
			}
			break;
		case MI_COPY_CASE(MI_COPY_OUT, 1):
			/* Completed copyout. */
			break;
		default:
			err = EPROTO;
			break;
		}
iocack:;
		mi_copy_done(q, mp, err);
		return 0;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			return 0;
		}
		break;
	}
	freemsg(mp);
	return 0;
}
