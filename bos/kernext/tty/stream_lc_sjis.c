#ifndef lint
static char sccsid[] = "@(#)66 1.3 src/bos/kernext/tty/stream_lc_sjis.c, sysxldterm, bos411, 9438C411e 9/22/94 17:32:34";
#endif
/*
 * COMPONENT_NAME: (sysxtty)
 *
 * FUNCTIONS: lc_conv_ajec2sjis, lc_conv_sjis2ajec, lc_sjis_readdata,
 *            lc_sjis_writedata, lc_sjis_config
 *
 * ORIGINS: 40, 71, 83
 *
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sleep.h>
#include <sys/uio.h>

#include <termios.h>
#include <sys/sysconfig.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/device.h>
#include <sys/lockl.h>
#include <sys/strconf.h>
#include <sys/eucioctl.h>
#include <sys/sjisioctl.h>
#include <sys/str_tty.h>
#include "stream_sjis.h"

/* Include headers for IBM extended character codes. */
#include "iconv932.h"
#include "ibmsel.h"
#include "iconvP.h"

struct module_info lc_sjis_modinfo = {
        LC_SJIS_MODULE_ID, LC_SJIS_MODULE_NAME, 0, INFPSZ, LC_SJIS_HIWAT,
        LC_SJIS_LOWAT
};

struct qinit lc_sjis_rinit = {
        lc_sjis_rput, lc_sjis_rsrv,
        lc_sjis_open, lc_sjis_close, 0, &lc_sjis_modinfo, 0
};

struct qinit lc_sjis_winit = {
        lc_sjis_wput, lc_sjis_wsrv, 0, 0, 0, &lc_sjis_modinfo, 0
};

struct streamtab lc_sjis_info = { &lc_sjis_rinit, &lc_sjis_winit };

static	int	lc_sjis_count = 0;	/* config method loads count	*/

lock_t	lc_sjis_conf_lock = LOCK_AVAIL; /* lc_sjis configuration lock	*/

/*
 * Conversion routines for AJEC--->>>IBM 932 and vice-versa.
 */

void
lc_conv_ajec2sjis(
	register mblk_t *from,
	register mblk_t *to,
	register struct sjis_s *jp
	)
{
	unsigned char uc, uc1, uc2, n1, n2;
	unsigned short code;
	int found, low, high, i;

	while (from) {
		while (from->b_rptr < from->b_wptr) {
			uc = *from->b_rptr++;
			uc1 = jp->asc[0];
			uc2 = jp->asc[1];
			if (!uc1) {
				if (INRANGE(0x00, 0x7f, uc)) {
					/*
					 * ascii and C0
					 */
					*to->b_wptr++ = uc;
				} else if (INRANGE(0x81, 0x8d, uc) ||
					 INRANGE(0x90, 0xa0, uc)) {
					/*
					 * C1
					 */
					if (jp->c1state == SJIS_C1_PASS) {
						/*
						 * pass it as is
						 */
						*to->b_wptr++ = uc;
					} else if (jp->c1state == SJIS_C1_C0) {
						/*
						 * convert to C0
						 */
						*to->b_wptr++ = 0x1b;
						*to->b_wptr++ = uc - 0x40;
					} else if (jp->c1state == SJIS_C1_THROW)						{
						/*
						 * throw away
						 */
					} else {
						*to->b_wptr++ = uc & 0x7f;
					}
				} else if ((uc == 0xff) || (uc == 0x80)) {
					*to->b_wptr++ = uc;
				} else {
					/*
					 * could be first byte of kanji or
					 * hankaku-kana
					 */
					jp->asc[0] = uc;
				}
			/* 
			 * The following code is essentially the same as
			 * that in _iconv_exec() in IBM-eucJP_IBM-932.c.  If
			 * this code changes, it may be necessary to change
			 * that code also;
			 */

			} else if (INRANGE(0xa1, 0xfe, uc1)) {

			    if (uc < 0xa1 || uc == 0xff) {
				n1 = 0x7f & uc1;
				n2 = 0x7f & uc;

			    } else if (uc1 == 0xf4 && uc > 0xfa6) {
				n1 = IBM932_D_SUBCHAR_1;
				n2 = IBM932_D_SUBCHAR_2;

			    } else {
				n1 = uc1;
				n2 = uc & 0x7f;
				n2 += n1 & 1 ? 0x1f : 0x7d;
				/*** JIS to SJIS ***/
				if (n2 >= 0x7f)
				    n2++;
				if (n1 < 0xf5) {
				    n1 = (n1 - 0xa1 >> 1) + 0x81;
				    if (n1 >= 0xa0)
					n1 += 0x40;

				} else {
				    n1 = (n1 - 0xf5 >> 1) + 0xf0;
				}
				/*** SJIS to IBM932 ***/
				low = 0;
				high = (sizeof (SJIStoCP932) >> 1) /
					sizeof (SJIStoCP932[0][FROM]);
				code = (n1 << 8 & 0xff00) + (n2 & 0xff);
				while (low <= high) {
				    i = low + high >> 1;
				    if (code < SJIStoCP932[i][FROM])
					high = i - 1;
				    else if (code > SJIStoCP932[i][FROM])
					low = i + 1;
				    else {
					code = SJIStoCP932[i][TO];
					n1 = code >> 8 & 0xff;
					n2 = code & 0xff;
					break;
				    }
				}

			    }
			    *to->b_wptr++ = n1;
			    *to->b_wptr++ = n2;
			    jp->asc[0] = 0;

			} else if (uc1 == SS2) {

			    if (uc < 0xa1 || uc == 0xff) {
				n1 = uc & 0x7f;

			    } else if (uc < 0xe0) {
				n1 = uc;

			    } else switch (uc) {
				case 0xe0:	n1 = 0x80; break;
				case 0xe1:	n1 = 0xa0; break;
				case 0xe2:	n1 = 0xfd; break;
				case 0xe3:	n1 = 0xfe; break;
				case 0xe4:	n1 = 0xff; break;
				default:	n1 = IBM932_SUBCHAR; break;
			    }

			    *to->b_wptr++ = n1;
			    jp->asc[0] = 0;

			} else if (uc1 == SS3) {

			    if (uc2) {
	
				if (!INRANGE(0xa1, 0xfe, uc)) {
				    n1 = 0x7f & uc2;
				    n2 = 0x7f & uc;

				} else if (uc2 >= 0xf5) {
				    n1 = uc2;
				    n2 = uc & 0x7f;
				    /*** JIS to SJIS ***/
				    n2 += n1 & 1 ? 0x1f : 0x7d;
				    if (n2 >= 0x7f)
					n2++;
				    n1 = (n1 - 0xf5 >> 1) + 0xf5;

				} else if (uc2 == 0xf3 || uc2 == 0xf4) {
				    code = IBMSELECTED_EUCto932[(uc2 - 0xf3)
					    * 0x5e + uc - 0xa1];
				    n1 = code >> 8 & 0xff;
				    n2 = code & 0xff;

				} else {
				    found = FALSE;
				    low = 0;
				    high = (sizeof (IBMSEL0212_EUCto932) >> 1)
					    / sizeof (IBMSEL0212_EUCto932[0]
					    [FROM]);
				    code = (uc2 << 8 & 0xff00) + (uc & 0xff);
				    while (low <= high) {
					i = low + high >> 1;
					if (code < IBMSEL0212_EUCto932[i][FROM])
					    high = i - 1;
					else if (code >
						IBMSEL0212_EUCto932[i][FROM])
					    low = i + 1;
					else {
					    code = IBMSEL0212_EUCto932[i][TO];
					    found = TRUE;
					    break;
					}
				    }
				    if (found) {
					n1 = code >> 8 & 0xff;
					n2 = code & 0xff;

				    } else {
					n1 = IBM932_D_SUBCHAR_1;
					n2 = IBM932_D_SUBCHAR_2;
				    }

				}
				*to->b_wptr++ = n1;
				*to->b_wptr++ = n2;
				jp->asc[0] = jp->asc[1] = 0;

			    } else {
				if (INRANGE(0xa1, 0xfe, uc)) {
				    jp->asc[1] = uc;
				} else {
				    /* error */
				    *to->b_wptr = uc & 0x7f;
				    jp->asc[0] = 0;
				}

			    }

			} else {
				/*
				 * error
				 */
				*to->b_wptr++ = uc1 & 0x7f;
				*to->b_wptr++ = uc & 0x7f;
				jp->asc[0] = 0;
			}
		}
		from = from->b_cont;
	}
}

void
lc_conv_sjis2ajec(
	register mblk_t *from,
	register mblk_t *to,
	register struct sjis_s *jp
	)
{
	unsigned char uc, uc1, n1, n2;
	int	 low, high, i;
	unsigned short pccode, euccode;

	while (from) {
		while (from->b_rptr < from->b_wptr) {
			uc = *from->b_rptr++;
			uc1 = jp->sac;
			if (!uc1) {
				if (INRANGE(0x00, 0x7f, uc)) {
					/*
					 * ascii and C0
					 */
					*to->b_wptr++ = uc;
				} else if (INRANGE(0xa1, 0xdf, uc)) {
					/*
					 * hankaku-kana
					 */
					*to->b_wptr++ = SS2;
					*to->b_wptr++ = uc;
				/*
				 * The following code is essentially the same
				 * as that in _iconv_exec() in
				 * IBM-932_IBM-eucJP.c.  If this code changes,
				 * it may be necessary to change that code also.
				 */

				} else if (INRANGE(0x81, 0x9f, uc) ||
					    INRANGE(0xe0, 0xfc, uc)
					   ) {
					/*
					 * First byte of a two-byte character.
					 *
					 * If the first byte is in the range,
					 * [0x81, 0x9f] or [0xe0, 0xef], it is
					 * a kanji character.
					 *
					 * If the first byte is in the range,
					 * [0xf0, 0xf9], it is a user-defined
					 * character.
					 *
					 * If the first byte is in the range,
					 * [0xfa, 0xfc], it is an IBM extended
					 * character.
					 */
					jp->sac = uc;
				} else 
					/*
					 * uc is one of 0x80, 0xa0, 0xfd, 0xfe,
					 *	or 0xff.
					 */

					*to->b_wptr++ = SS2;
					switch (uc) {
					case 0x80:
						*to->b_wptr++ = 0xe0;
						break;
					case 0xa0:
						*to->b_wptr++ = 0xe1;
						break;
					case 0xfd:
					case 0xfe:
					case 0xff:
						*to->b_wptr++ = (uc - 0x1b);
						break;
				}
			} else if (INRANGE(0x81, 0x9f, uc1) ||
				   INRANGE(0xe0, 0xfc, uc1)) {

				/*
				 * The first byte (which was saved in jp->sac
				 * on the last time through the inner loop)
				 * indicates that we are looking at the second
				 * byte of a multiple byte character.
				 */

				n1 = uc1;	/* First byte of mbyte char */
				n2 = uc;	/* Second byte of mbyte char */

				if (n2 < 0x40 || n2 == 0x7f || 0xfc < n2) {

					/*
					 * The second byte of the mbyte char
					 * is not valid.  Treat it the same
					 * way OSF treats invalid mbyte
					 * sequences.  (Turn off the high order
					 * bit in eahc byte.)
					 */
					*to->b_wptr++ = n1 & 0x7f;
					*to->b_wptr++ = n2 & 0x7f;
					jp->sac = 0;

				} else {

					/*** IBM932 to SJIS ***/

					low = 0;
					high = (sizeof (CP932toSJIS) >> 1) /
						sizeof (CP932toSJIS[0][FROM]);
					pccode = (n1 << 8 & 0xff00) +
						(n2 & 0xff) ;
					while (low <= high) {
					    i = low + high >> 1;
					    if (pccode < CP932toSJIS[i][FROM])
						high = i - 1;
					    else if (pccode > CP932toSJIS[i]
						    [FROM])
						low = i + 1;
					    else {
						pccode = CP932toSJIS[i][TO];
						n1 = pccode >> 8 & 0xff;
						n2 = pccode & 0xff;
						break;
					    }

					} if (0xfa <= n1) {
					    *to->b_wptr++ = EUCSS3;
					    if (n2 >= 0x80) n2--;
					    euccode = IBMSELECTED_932toEUC[(n1
						    - 0xfa) * 0xbc + n2 - 0x40];
					    *to->b_wptr++ = euccode >> 8 & 0xff;
					    *to->b_wptr++ = euccode & 0xff;
					    jp->sac = 0;

					} else if (n1 >= 0xf0) {

					    if (n1 >= 0xf5) {
						n1 -= 0xf5;
						*to->b_wptr++ = EUCSS3;

					    } else {
						n1 -= 0xf0;
					    }
					    n1 = (n1 << 1) + 0xf5;
					    if (n2 >= 0x80)
						n2--;
					    if (n2 >= 0x9e) {
						*to->b_wptr++ = n1 + 1;
						*to->b_wptr++ = n2 + 3;
					    } else {
						*to->b_wptr++ = n1;
						*to->b_wptr++ = n2 + 0x61;
					    }
					    jp->sac = 0;

					} else {

					    /*** SJIS to EUCJP (kanji) ***/

					    n1 = (n1 - (n1 >= 0xe0 ? 0xc1 :
						    0x81) << 1) + 0xa1;
					    if (n2 >= 0x80)
						n2--;
					    if (n2 >= 0x9e) {
						*to->b_wptr++ = n1 + 1;
						*to->b_wptr++ = n2 + 3;
					    } else {
						*to->b_wptr++ = n1;
						*to->b_wptr++ = n2 + 0x61;
					    }
					    jp->sac = 0;

					}

				}

			} else {
				*to->b_wptr++ = uc1 & 0x7f;
				*to->b_wptr++ = uc & 0x7f;
				jp->sac = 0;
			}
		} /* inner while */
		from = from->b_cont;
	} /* outer while */
}

int
lc_sjis_readdata(
	register struct sjis_s *jp,
	queue_t *q,
	mblk_t *mp,
	void (*conv_func)(),
	int mem_coeff
	)
{
	register mblk_t *next;
	register int maxmsg;
	int flag = (mp->b_flag & (~MSGNOTIFY));
	mblk_t *mp1, *mp2;
	int notify_count = 0;

	for (mp1 = mp; mp1; mp1 = mp1->b_cont)
		if (mp1->b_flag & MSGNOTIFY)
			notify_count += (mp1->b_wptr - mp1->b_rptr);

	if (notify_count) {
		mp2 = allocb(sizeof(int), BPRI_HI);
		if (!mp2) {
			if (jp->rbid)
				unbufcall(jp->rbid);
			if (!(jp->rbid = 
				bufcall(sizeof(int), BPRI_HI, qenable, q))) {
				if (jp->rtid)
					pse_untimeout(jp->rtid);
				jp->rtid = pse_timeout(qenable, q, hz*2);
			}
			return(0);
		}
		for (mp1 = mp; mp1; mp1 = mp1->b_cont)
			mp1->b_flag &= ~MSGNOTIFY;
		*((int *)mp2->b_rptr) = notify_count;
		mp2->b_wptr = mp2->b_rptr + sizeof(int);
		mp2->b_datap->db_type = M_NOTIFY;
		putnext(OTHERQ(q), mp2);
	}

	if (!canput(q->q_next))
		return(0);

	if (!(jp->flags & KS_ICONV) ||
	    !(jp->flags & KS_IEXTEN)) {
		putnext(q, mp);
		return(1);
	}

	/*
	 * pass blank messages up as they are
	 */
	if (msgdsize(mp) == 0) {
		putnext(q, mp);
		return(1);
	}
	maxmsg = msgdsize(mp) * mem_coeff; /* sjis -> ajec */
	if (jp->rspare && (jp->rspare->b_datap->db_size >= maxmsg)) {
		next = jp->rspare;
		jp->rspare = 0;
	} else
		next = allocb(maxmsg, BPRI_MED);

	if (!next) {
		if (jp->rbid)
			unbufcall(jp->rbid);
		if (!(jp->rbid = bufcall(maxmsg, BPRI_MED, qenable, q))) {
			if (jp->rtid)
				pse_untimeout(jp->rtid);
			jp->rtid = pse_timeout(qenable, q, hz*2);
		}
		return(0);
	}

	(*conv_func)(mp, next, jp);

	if (jp->rspare || (mp->b_datap->db_size < 2))
		freemsg(mp);
	else {
		mblk_t *mp1;

		if (mp1 = unlinkb(mp))
			freemsg(mp1);

		mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		jp->rspare = mp;
	}

	if (next->b_rptr < next->b_wptr) {
		next->b_flag |= flag;
		putnext(q, next);
	} else
		freemsg(next);
	return(1);
}

int
lc_sjis_writedata(
	register struct sjis_s *jp,
	queue_t *q,
	mblk_t *mp,
	void (*conv_func)(),
	int mem_coeff
	)
{
	register mblk_t *next;
	register int maxmsg;

	if (!canput(q->q_next))
		return(0);

	if (!(jp->flags & KS_OCONV)) {
		putnext(q, mp);
		return(1);
	}

	maxmsg = msgdsize(mp) * mem_coeff;
	if (jp->wspare && (jp->wspare->b_datap->db_size >= maxmsg)) {
		next = jp->wspare;
		jp->wspare = 0;
	} else
		next = allocb(maxmsg, BPRI_MED);

	if (!next) {
		if (jp->wbid)
			unbufcall(jp->wbid);
		if (!(jp->wbid = bufcall(maxmsg, BPRI_MED, qenable, q))) {
			if (jp->wtid)
				pse_untimeout(jp->wtid);
			jp->wtid = pse_timeout(qenable, q, hz*2);
		}
		return(0);
	}

	(*conv_func)(mp, next, jp);

	if (jp->wspare || (mp->b_datap->db_size == 0))
		freemsg(mp);
	else {
		mblk_t *mp1;

		if (mp1 = unlinkb(mp))
			freemsg(mp1);

		mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		jp->wspare = mp;
	}

	if (next->b_rptr < next->b_wptr)
		putnext(q, next);
	else
		freemsg(next);
	return(1);
}



/*
 * lc_sjis_config() - sjis lower code converter entry point.
 */
int
lc_sjis_config(cmd, uiop)
	int	cmd;
	struct	uio	*uiop;
{
	static	strconf_t	lc_conf = {
		"lc_sjis", &lc_sjis_info, (STR_NEW_OPEN|STR_MPSAFE),
	};
	int	error = 0, locked;
	struct	lc_sjis_dds	init_lc_sjis_dds;

	locked = lockl(&lc_sjis_conf_lock, LOCK_SHORT);
	lc_conf.sc_sqlevel = SQLVL_QUEUEPAIR;
	switch (cmd) {
	case CFG_INIT:
		if (uiop) {
			if (uiomove(&init_lc_sjis_dds, 
				    sizeof(struct lc_sjis_dds),
				    UIO_WRITE, uiop) ||
			    (init_lc_sjis_dds.which_dds != LC_SJIS_DDS))
				break;
			else {
				if (lc_sjis_count == 0)
					error=
					str_install(STR_LOAD_MOD,&lc_conf);
				if (!error)
					lc_sjis_count++;
			}
		}
		else
			error = str_install(STR_LOAD_MOD, &lc_conf);
		break;
	case CFG_TERM:
		if (uiop) {
			if (uiomove(&init_lc_sjis_dds,
				    sizeof(struct lc_sjis_dds),
				    UIO_WRITE, uiop) ||
			    (init_lc_sjis_dds.which_dds != LC_SJIS_DDS))
				break;
			else {
				if (lc_sjis_count == 1)
					error = str_install(STR_UNLOAD_MOD,
								&lc_conf);
				if (!error)
					lc_sjis_count--;
			}
		}
		else
			error = str_install(STR_UNLOAD_MOD, &lc_conf);
		break;
	default:
		error = EINVAL;
		break;
	}
	if (locked != LOCK_NEST)
		unlockl(&lc_sjis_conf_lock);
	return(error);
}
