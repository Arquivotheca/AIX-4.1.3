#ifndef lint
static char sccsid[] = "@(#)68 1.7 src/bos/kernext/tty/snls.c, sysxtty, bos41B, 9505A 1/20/95 08:28:59";
#endif
/*
 * COMPONENT_NAME: (sysxtty) Streams Based NLS module
 *
 * FUNCTIONS:   str_nls_config(), str_nls_open(), str_nls_close()
 *              str_nls_rput(), str_nls_rsrv(), str_nls_wput(), str_nls_wsrv(),
 *              str_handle_set_map(), str_handle_get_map(), str_clear_maps(),
 *              nls_send_data(), str_ttmap(), str_nls_mctl(),
 *              str_copyin_map(), freemap(), str_set_map(), find_map(),  
 *              skip_past_set(), char_in_set(), str_nls_err(), str_nls_maperr(), 
 *              str_nls_ioctl(), str_nls_iocdata(),
 *      	allocb_recovery(), str_record_map()
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <net/net_globals.h>
#ifdef _KDB
#include <pse/str_stream.h>
#include "POWER/srs.h"
#endif	/* _KDB */

#include <sys/stream.h> 
#include <sys/stropts.h>
#include <sys/device.h>
#include <sys/strconf.h>
#include <sys/termio.h>		/* for TCGMAP and TCSMAP; includes <sys/ttmap.h> */
#include <sys/errids.h>		/* for ERRID_NLS_BADMAP etc. */
#include <sys/lockl.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/uio.h>
#include <sys/intr.h>
#include <net/net_malloc.h>
#include "snls.h"

#define	NLS_NAME	"nls"
#define nlsprintf printf

#define	TRC_NLS(subhookid)	((HKWD_STTY_NLS)|subhookid)

static struct module_info minfo = {
	7650,		/* module id     */  
	NLS_NAME,	/* module name   */
	0,		/* min pkt size  */
	INFPSZ,		/* max pkt size  */
	0,		/* hiwater       */
	0,		/* lowater       */
};

		/* lock for manipulate nls_config_count in the config routine	*/
int nls_lock_config = LOCK_AVAIL;

/*										*/
/* ttmaplist        : pointer to shared map list indicates how many TTYs/lines	*/
/*		      are actually configured;					*/
/* nls_config_count : When nls_config_count is 0, this module may be		*/
/*		      unregistered from the Streams Framework.			*/
/* nlsmaplock       : lock for the map list.					*/
/*										*/

Simple_lock nlsmaplock;

#ifdef DEBUG_NLS

#define	NLS_PRINTF_ON(X)	( (nls_printf_on) ? (X) : 0 )
						/* Data could be seen by KDB */
mapp_t ttmaplist = 0;
long nls_config_count = 0;
short int nls_printf_on = 0;

#else

#define	NLS_PRINTF_ON(X)
						/* Data local to the module  */
static mapp_t ttmaplist = 0;
static long nls_config_count = 0;
static short int nls_printf_on = 0;

#endif

static struct tioc_reply      /* structure for tioc-module/M_CTL in strnls_rput */
nls_tioc_reply[] = {
        { TCGMAP, (sizeof(struct tty_map) + TTMAP_MAXSIZE), TTYPE_COPYINOUT },
	{ TCSMAP, sizeof(struct tty_map), TTYPE_COPYIN },
};

/* function definitions of the nls module */

int str_nls_config();

int str_nls_open(), str_nls_close(), str_nls_rput();

int str_nls_rsrv(), str_nls_wput(), str_nls_wsrv();

static int str_handle_set_map(), str_handle_get_map(), str_clear_maps();

static int nls_send_data(), str_ttmap(), str_nls_mctl(), str_nls_ioctl();

static int char_in_set(), str_nls_iocdata();

static int str_copyin_map(), str_record_map();

static void freemap(), str_set_map();

static mapp_t find_map(); 

static void skip_past_set(), str_nls_err(), str_nls_maperr();

static void allocb_recovery();

/* function definitions of the nls module */

#ifdef _KDB
extern	void	nls_line_print();
#endif	/* _KDB */

static struct qinit rinit = {
	str_nls_rput, str_nls_rsrv, str_nls_open, str_nls_close, NULL, &minfo, NULL
};

static struct qinit winit = {
	str_nls_wput, str_nls_wsrv, NULL, NULL, NULL, &minfo, NULL
};

struct streamtab nlsinfo = { &rinit, &winit, NULL, NULL };

#ifdef _KDB
/* Definitions for nls_kdb */

#define	NLS_KDB_UNKNOW	-1
#define	NLS_KDB_STD	0
#define	NLS_KDB_M	1
#define	NLS_KDB_m	2
#define	NLS_KDB_T	3
#define	NLS_KDB_t	4

#define	SRS_NAME	"srs"

struct names {
	char *n_str;
	unsigned n_mask;
	unsigned n_state;
};

#define TST_NLS_KDB_NEXT_PARAM(buf, poff, len)		\
	if (!nls_kdb_next_param(buf, poff, len)) {	\
		nls_print_help();			\
		return;					\
	}

#endif	/* _KDB */


/*
 * NAME:     str_nls_config 
 *
 * FUNCTION: 1.) Configuring a TTY line or changing the configuration (CFG_INIT).
 *           2.) Unconfiguring a TTY line (CFG_TERM).
 *
 * RETURNS:  On success 0 is returned.
 *	     EINVAL No valid nls_dds DDS structure is available or
 *                  invalid values in DDS.
 *	     Error return value from str_install, the call to the routine failed.
 *	     Error return value from uiomove, the call to the routine failed.       
 */

int
str_nls_config(cmd, uiop)
	int cmd;
	struct uio *uiop;
{
	int	error;
	struct nls_dds nlsdd_init;
	static strconf_t conf = {
		NLS_NAME, &nlsinfo, STR_NEW_OPEN,
	};

#ifdef TTYDBG
	int	ttydbg_error;
	static	struct str_module_conf nls_str_module_conf = {
		NLS_NAME, 'm', NLS_PDPF
	};
#endif	/* TTYDBG */

	Enter(TRC_NLS(TTY_CONFIG), 0, 0, cmd, 0, 0);

        error = 0;

	/* valid nlsdd_init (DDS) structure is available ? */
	if((uiop == NULL) || (uiop->uio_resid < sizeof(struct nls_dds))) {
		Return(EINVAL);
	}

	/* Moving the DDS in to the kernel space. */
	if(error=uiomove(&nlsdd_init, sizeof(struct nls_dds), UIO_WRITE, uiop)) {
		Return(error);
	}

	/* Is there an invalid value in the DDS ? */
	if(nlsdd_init.which_dds != NLS_DDS ) {
		Return(EINVAL);
	}

	/* valid DDS is available here */

	conf.sc_flags |= (STR_MPSAFE | STR_Q_SAFETY);
	conf.sc_sqlevel = SQLVL_QUEUEPAIR;

	lockl(&nls_lock_config, LOCK_SHORT);

	switch (cmd) {
	case CFG_INIT:

		/* register this module into the Streams Framework */
		
		if(nls_config_count == 0) {
			error = str_install(STR_LOAD_MOD, &conf);
			if(error != 0)
				break;

			nls_config_count++; /* nls module is configured now */ 
			lock_alloc(&nlsmaplock, LOCK_ALLOC_PIN,
							NLS_NLSMAPLOCK, -1);
			simple_lock_init(&nlsmaplock); 

#ifdef TTYDBG
			/* registering the module into the ttydbg extension */
			ttydbg_error = tty_db_register(&nls_str_module_conf);
#endif	/* TTYDBG */
		} else
					/* one more line is configured now */
			nls_config_count++;

		break;

	case CFG_TERM:
		
		/* str_nls_config(CFG_TERM, ) is called by the tty unconfig
		 * method for a Stream/line, which is closed.
		 */
	
		if(nls_config_count == 1) {
			error = str_install(STR_UNLOAD_MOD, &conf);
			if(error != 0)
				break;
						/* str_install succeeds */
			lock_free(&nlsmaplock);
			nls_config_count--;

#ifdef TTYDBG
			/* unregistering the module into the ttydbg extension */
			ttydbg_error = tty_db_unregister(&nls_str_module_conf);
#endif	/* TTYDBG */
		} else
			nls_config_count--;
			
		break;

	default:
		error = EINVAL;
                break;
	}

	unlockl(&nls_lock_config);

	Return(error);
}

/*
 * NAME:     str_nls_open 
 *
 * FUNCTION: 1.) When pushing nls onto a Stream (first open).
 *           2.) When opening a Stream, which has the str_nls module already
 *		 pushed (reopen).
 *
 * RETURNS:  On success 0 is returned.
 *	     ENOMEM The call to the he_alloc routine failed.
 *	     ENXIO  The q pointer is NULL or the pointer to "nls" structures in
 *		    the read and the write queues are not the same.
 *
 */

int
str_nls_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	STRNLSP nls_p;

#ifdef TTYDBG
	int	ttydbg_error = 0;
	struct	tty_to_reg nls_tty_to_reg;
#endif	/* TTYDBG */

	Enter(TRC_NLS(TTY_OPEN), *devp, q->q_ptr, flag, sflag, 0);

	if(q == NULL) {
		Return (ENXIO);
	}

	if(q->q_ptr) {      				/* reopen */
		if(q->q_ptr != OTHERQ(q)->q_ptr) {
			Return (ENXIO);
		}
		else {
			Return (0);
		}
	}

	/* allocation of an nls structure */
	if(!(nls_p = (STRNLSP) he_alloc(sizeof(STRNLS), BPRI_MED) )) {
		Return (ENOMEM);
	}

	bzero(nls_p, sizeof(STRNLS));

	q->q_ptr = OTHERQ(q)->q_ptr = (char *) nls_p; 

	nls_p->wbid = 0;
	nls_p->rbid = 0;
	nls_p->wtid = 0;
	nls_p->rtid = 0;

	nls_p->devno = *devp;			/* for tty trace purpose */

#ifdef TTYDBG
	/* Open the ttydbg extension for the module */

	nls_tty_to_reg.dev=*devp;
	nls_tty_to_reg.ttyname[0]='\0';
	bcopy(NLS_NAME, &nls_tty_to_reg.name,sizeof(NLS_NAME));
	nls_tty_to_reg.private_data=nls_p;

	ttydbg_error = tty_db_open(&nls_tty_to_reg);
#endif	/* TTYDBG */

	Return (0);
}

/*
 * NAME:     str_nls_close 
 *
 * FUNCTION: This routine is called, when the nls module is popped from a Stream.
 *
 * RETURNS:  always 0
 *
 */

int
str_nls_close (q, flag, credp)
	register queue_t *q;
	int flag;
	cred_t *credp;
{
	register STRNLSP strnlsp = (STRNLSP)q->q_ptr;

#ifdef TTYDBG
	int	ttydbg_error = 0;
	struct	tty_to_reg nls_tty_to_reg;
#endif	/* TTYDBG */

	Enter(TRC_NLS(TTY_CLOSE), strnlsp->devno, strnlsp, flag, 0, 0);

	if(q->q_ptr == NULL) {
		Return (0);
	}

	if (strnlsp->wbid)
		unbufcall(strnlsp->wbid);
	if (strnlsp->rbid)
		unbufcall(strnlsp->rbid);
	if (strnlsp->wtid)
		untimeout(strnlsp->wtid);
	if (strnlsp->rtid)
		untimeout(strnlsp->rtid);
#ifdef TTYDBG
	/*
	 * Close the ttydbg extension for the module.
	 */
	nls_tty_to_reg.dev=strnlsp->devno;
	nls_tty_to_reg.ttyname[0]='\0';
	bcopy(NLS_NAME, &nls_tty_to_reg.name,sizeof(NLS_NAME));
	nls_tty_to_reg.private_data=strnlsp;

	ttydbg_error = tty_db_close(&nls_tty_to_reg);
#endif	/* TTYDBG */

	he_free((caddr_t)q->q_ptr);

	q->q_ptr = OTHERQ(q)->q_ptr = 0;

	Return (0);
}

/*
 * NAME:     str_nls_rput
 *
 * FUNCTION: This routine handles the following message types arriving at the read
 *	     side: M_DATA, M_FLUSH, M_CTL and "default".
 *
 * RETURNS:  always 0
 *
 */

int
str_nls_rput (q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	STRNLSP strnlsp = (STRNLSP)q->q_ptr;

	Enter(TRC_NLS(TTY_RPUT), strnlsp->devno, strnlsp, mp, mp->b_datap->db_type, 0);

	switch (mp->b_datap->db_type) {

	case M_DATA:
		/* map characters in the original message and pass the new 
		   message upstream */
		if(q->q_first || !nls_send_data(&strnlsp->nls_mapin, q, mp))

			/* There is allready at least 1 message of type
			   M_DATA queued or nls_send_data returns with 0 */
			putq(q, mp);
		break;

	case M_FLUSH:
		if(*mp->b_rptr & FLUSHR)
			flushq(q, FLUSHDATA);
		putnext(q, mp);
		break;

	case M_CTL:
		if(q->q_first || !str_nls_mctl(q, mp))
		   putq(q, mp);
		break;
			
	default:
		/* If high priority message or flow control allows it
		   -> putnext(q, mp) */
		if((mp->b_datap->db_type >= QPCTL) || (canput(q->q_next)))
			putnext(q, mp); 
		else
			putq(q, mp); /* otherwise queueing of this message */

		break;
	

	} /* switch (mp->b_datap->db_type) */

	Return (0);
}

/*
 * NAME:     str_nls_rsrv
 *
 * FUNCTION: This routine handles the following message types queued at the read
 *	     side: M_DATA, M_CTL and "default".
 *
 * RETURNS:  always 0
 *
 */

int
str_nls_rsrv (q)
	register queue_t *q;
{
	register mblk_t *mp;
	STRNLSP strnlsp = (STRNLSP)q->q_ptr;

	Enter(TRC_NLS(TTY_RSRV), strnlsp->devno, strnlsp, q->q_count, 0, 0);

	while(mp = getq(q)) {
	switch (mp->b_datap->db_type) {

	case M_DATA:
		/* map characters in the original message and pass the new 
		   message upstream */
		if(!nls_send_data(&strnlsp->nls_mapin, q, mp)) {
			/* nls_send_data returns with 0 */
			putbq(q, mp);
			Return(0);

		}
		break;

	case M_CTL:
		if(!str_nls_mctl(q, mp)) {
		   putbq(q, mp);
		   Return(0);
	        }
		break;

	default:
		/* If flow control does not allow it -> put back to message queue */
		if(!canput(q->q_next)) {
			putbq(q, mp);
			Return (0);
		}
		putnext(q, mp); 
		break;
	

	  } /* switch (mp->b_datap->db_type) */
        } /* while */

	Return (0);
}

/*
 * NAME:     str_nls_wput
 *
 * FUNCTION: This routine handles the following message types arriving at the write
 *	     side: M_DATA, M_IOCDATA, M_FLUSH, M_IOCTL and "default".
 *
 * RETURNS:  always 0
 *
 */

int
str_nls_wput (q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	int err;
	STRNLSP strnlsp = (STRNLSP)q->q_ptr;


	Enter(TRC_NLS(TTY_WPUT), strnlsp->devno, strnlsp, mp, mp->b_datap->db_type, 0);

	NLS_PRINTF_ON(printf("in str_nls_wput: q == %d  mp == %d\n",q, mp));

	switch (mp->b_datap->db_type) {

	case M_DATA:
		NLS_PRINTF_ON(printf("in str_nls_wput: case M_DATA\n"));
		/* map characters in the original message and write the new 
		   message downstream */
		if(q->q_first || !nls_send_data(&strnlsp->nls_mapout, q, mp))

			/* There is allready at least 1 message of type
			   M_DATA queued or nls_send_data returns with 0 */
			putq(q, mp);
		break;

	case M_IOCDATA:
		NLS_PRINTF_ON(printf("in str_nls_wput: case M_IOCDATA\n"));
		str_nls_iocdata(q, mp);
		break;

	case M_FLUSH:
		NLS_PRINTF_ON(printf("in str_nls_wput: case M_FLUSH\n"));
		if(*mp->b_rptr & FLUSHW) 
			flushq(q, FLUSHDATA);
		putnext(q, mp);
		break;

	case M_IOCTL:

		NLS_PRINTF_ON(printf("in str_nls_wput: case M_IOCTL\n"));

		if (q->q_first || !str_nls_ioctl(q, mp)) {
			NLS_PRINTF_ON(printf("in str_nls_wput: case M_IOCTL: putq(q, mp)!!!\n"));
			putq(q, mp);
		}
		break;
		 			
	default:
		NLS_PRINTF_ON(printf("in str_nls_wput: case default:\n"));
		/* If high priority message or flow control allows it
		   -> putnext(q, mp) */
		if((mp->b_datap->db_type >= QPCTL) || (canput(q->q_next)))
			putnext(q, mp);
		else
			putq(q, mp); /* otherwise queueing of this message */

		break;
	

	} /* switch (mp->b_datap->db_type) */

	Return (0);
	
} /* end str_nls_wput */

/*
 * NAME:     str_nls_wsrv
 *
 * FUNCTION: This routine handles the following message types queued at the write
 *	     side: M_DATA, M_IOCTL and "default".
 *
 * RETURNS:  always 0
 *
 */

int
str_nls_wsrv (q)
	register queue_t *q;
{
	register mblk_t *mp;
	STRNLSP strnlsp = (STRNLSP)q->q_ptr;

	Enter(TRC_NLS(TTY_WSRV), strnlsp->devno, strnlsp, q->q_count, 0, 0);

	while(mp = getq(q)) {

	NLS_PRINTF_ON(printf("in str_nls_wsrv: q == %d  mp == %d\n",q, mp));

	switch (mp->b_datap->db_type) {
	case M_DATA:
		NLS_PRINTF_ON(printf("in str_nls_wsrv: case M_DATA\n"));
		/* map characters in the original message and write the new 
		   message downstream */
		if(!nls_send_data(&strnlsp->nls_mapout, q, mp)) {
			/* nls_send_data returns with 0 */
			putbq(q, mp);
			Return(0);
		}
		break;

	case M_IOCTL:
		NLS_PRINTF_ON(printf("in str_nls_wsrv: case M_IOCTL\n"));
		if (!str_nls_ioctl(q, mp)){
			putbq(q, mp);
			Return(0);
		}
		break;

	default:
		NLS_PRINTF_ON(printf("in str_nls_wsrv: case default:\n"));
		/* If flow control does not allow it -> put back to message queue */
		if(!canput(q->q_next)) {
			putbq(q, mp);
			Return (0);
		}
		putnext(q, mp); 
		break;
	} /* switch (mp->b_datap->db_type) */
        } /* while */

	Return (0);
}

/*
 * NAME:     str_nls_mctl
 *
 * FUNCTION: This routine handles the M_CTL message arriving at the read side
 *
 * RETURNS:  1 on success
 *	     0 if mp has to be (re)queued
 *
 */

static int
str_nls_mctl(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register mblk_t *new_mp;
	register IOCP iocp;
	register int r_size;
	register STRNLSP strnlsp;
	strnlsp = (STRNLSP) q->q_ptr;
	iocp = (IOCP)mp->b_rptr;

	NLS_PRINTF_ON(printf("iocp->ioc_cmd %x %d\n",iocp->ioc_cmd, iocp->ioc_cmd ));

	if(iocp->ioc_cmd != TIOC_REPLY) {
		NLS_PRINTF_ON(printf("iocp->ioc_cmd != TIOC_REPLY => putnext(q, mp); \n"));
		putnext(q, mp);
		return (1);
	}

	if(!canput(q->q_next)) {
		return (0); /* mp will be put to the queue in str_nls_rput()
			     or str_nls_rsrv() */
	}

	r_size = sizeof(nls_tioc_reply);
	new_mp = allocb(r_size, BPRI_MED);
	
	if(!new_mp) { /* error recovery with bufcall() */
		allocb_recovery(q, &strnlsp->rbid,  &strnlsp->rtid, r_size);
		return (0); /* mp will be put to the queue in str_nls_rput()
			     or str_nls_rsrv() */
	}
	else{ /* allocb() did not fail */
                new_mp->b_cont = mp->b_cont;
                mp->b_cont = new_mp;
                new_mp->b_datap->db_type = M_DATA;
                new_mp->b_wptr = new_mp->b_rptr + r_size;
                bcopy(nls_tioc_reply, new_mp->b_rptr, r_size);
        }
	putnext(q, mp);
	return (1);
}

/*
 * NAME:     str_nls_ioctl
 *
 * FUNCTION: This routine handles the M_IOCTL message arriving at the write side
 *	     == the ioctls destined to the nls module.
 *
 * RETURNS:  1 on success = putnext or positive/negative acknowledgement
 *           0 if mp has to be (re)queued
 *
 * COMMENTS: When str_nls_ioctl returns with strnlsp->nlsmap != 0 that means
 *	     that we are waiting for an M_IOCDATA message comming from the
 *	     stream head in answer to a M_COPYIN or M_COPYOUT message :
 *	     In case TCSMAP, str_copyin_map (called by str_handle_set_map) can put
 *	     an M_COPYIN message on the read side to make the stream head copy in
 *	     the ttmap from the user space. If there is any problem (err!=0) or
 *	     the ttmap is already present in this M_IOCTL message (new interface,
 *	     err==0) the M_COPYIN message is not sent and the strnlsp->nlsmap is
 *	     equal to 0.
 *	     In case TCGMAP, str_handle_get_map can put an M_COPYOUT message on
 *	     the read side to make the stream head copy out a ttmap to the user
 *	     space. If there is any problem (err!=0) or the caller want only the
 *	     ttmap structure (err==0) the M_COPYOUT message is not sent and the
 *	     strnlsp->nlsmap is equal to 0.
 *
 */

static int
str_nls_ioctl(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	int err = 0;
	STRNLSP strnlsp = (STRNLSP)q->q_ptr;
	IOCP iocp = (IOCP)mp->b_rptr;

	Enter(TRC_NLS(TTY_IOCTL), strnlsp->devno, strnlsp, iocp->ioc_cmd, 0, 0);

	/*
	 * transparent ioctls are not handled in snls.
	 * Push "the tioc" module to convert transparent to I_STR ioctls!
	 *
	 */
	if ( (iocp->ioc_cmd == TCSMAP || iocp->ioc_cmd == TCGMAP)
	    && iocp->ioc_count == TRANSPARENT )
	{
		NLS_NACK(mp, EINVAL);
		qreply(q, mp);
		Return (1);
	}
	switch (iocp->ioc_cmd) {
	case TCSMAP:
		NLS_PRINTF_ON(printf("In str_nls_ioctl: In case TCSMAP: \n"));

		err = str_handle_set_map(q, mp);

		NLS_PRINTF_ON(printf("In case TCSMAP: After err = str_handle_set_map(q, mp), err = %x\n", err));

		break;

	case TCGMAP:

		NLS_PRINTF_ON(printf("In str_nls_ioctl: In case TCGMAP: \n"));

		err = str_handle_get_map(q, mp);

		NLS_PRINTF_ON(printf("In case TCGMAP: After err = str_handle_get_map(q, mp), err = %x\n", err));
		break;

	default:
		NLS_PRINTF_ON(printf("In str_nls_ioctl: In case default: %x\n", iocp->ioc_cmd));
		putnext(q, mp);
		Return (1);
	}

	/* See also COMMENTS in this str_nls_ioctl function header */

	if (!strnlsp->nlsmap) { /* will a ttmap be copied to/from user space ? */
		/* No ttmap at all or given in the same message with the tty_map */
		NLS_PRINTF_ON(printf("In str_nls_ioctl: /* No ttmap will be copied to/from user space */\n"));
		if(err) {  
			NLS_NACK(mp, err); /* prepare a not acknowledge message */
		}
		else {
			if(iocp->ioc_cmd == TCGMAP) {
				/*
				 * change the message type to M_IOCACK,
				 * the ioc_count to sizeof(struct tty_map)
				 * and set the ioc_error and ioc_rval to 0.
				 * The mp->b_cont->b_rptr pointer points to
				 * the wanted tty_map structure!
				 */
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_error = 0;
				iocp->ioc_rval = 0;
				iocp->ioc_count = sizeof(struct tty_map);
			}
			else {
				NLS_ACK(mp, 0); /* acknowledge the former
						ioctl message */
			}
		}
		qreply(q, mp);
	}else {         /* ttmap will be copied to/from user space */
                NLS_PRINTF_ON(printf("In str_nls_ioctl: /* ttmap will be copied to/from user space */\n"));
		/* answer to ioctl will be sent under M_IOCDATA */
		NLS_PRINTF_ON(printf("In str_nls_ioctl:/* answer to ioctl will be sent under M_IOCDATA */\n"));
	}

	Return (1);
}

/*
 * NAME:     str_nls_iocdata
 *
 * FUNCTION: This routine handles the M_IOCDATA message arriving at the write side
 *
 * RETURNS:  Always 0.
 *
 */

static int
str_nls_iocdata(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
 
	register struct copyresp *csp;
	register IOCP iocp;
	int err;
	infop_t ttmapinfo;
	STRNLSP strnlsp;

	strnlsp = (STRNLSP)q->q_ptr;
	err = 0;

	/*
	 * We are waiting for an answer to a previous  M_COPYIN or M_COPYOUT message?
	 */ 

	NLS_PRINTF_ON(printf("In str_nls_iocdata: q == %x mp = %x\n", q, mp));
	if (!strnlsp->nlsmap) {

		NLS_PRINTF_ON(printf("In str_nls_iocdata: we are not waiting!!! for an M_IOCDATA\n", q, mp));
		putnext(q, mp);  /* we are not waiting! -> putnext(q, mp); */
		return (0);  
	}
	csp = (struct copyresp*)mp->b_rptr; /* we are waiting!  */

	switch (csp->cp_cmd) {
	case TCSMAP: 
		if (csp->cp_rval) {
			NLSMAP_FREE(strnlsp->nlsmap);
			err = csp->cp_rval;
		}
		else {
			if (mp->b_cont == 0) {
				NLSMAP_FREE(strnlsp->nlsmap);
				err = EIO;
			} else
				err = str_record_map(q,mp);
		}

					/* lock taken in str_handle_set_map */
		unlock_enable(strnlsp->oldpri, &nlsmaplock);
		break;

	case TCGMAP:
		err = csp->cp_rval;
		break;
	default:  /* M_IOCDATA message is not for us */
		putnext(q, mp);

		strnlsp->nlsmap = NULL;
		return (0); 

	}

	if (err) {
		NLS_NACK(mp, err);
	} else {
		NLS_ACK(mp, err);
	}
	qreply(q, mp);

	strnlsp->nlsmap = NULL;

	return (0);

}

/*
 * NAME:     str_handle_set_map
 *
 * FUNCTION: Load a map into memory and/or assign a map to a tty.
 *
 * RETURNS:  0 on success
 *	     EINVAL It is tried to treat a bad version map, a map with no name
 *		    or the TM_DEFUNCT flag is turn on.
 *		    It is tried to reload and load a map at the same time or reload
 *		    a map with TM_INPUT and TM_OUTPUT both specified.
 *		    It is tried to load/reload a map and use it at the same time
 *		    with TM_INPUT and TM_OUTPUT both specified or none.
 *		    We don't know which map to treat, neither TM_INPUT nor TM_OUTPUT
 *		    is specified or they are both specified.
 *		    flags in tty_map has a bad value.
 *	     EPERM  It is tried to load/reload by a non-super user.
 *	     EEXIST It is tried to load a map that is already loaded.
 *	     ENOENT It is tried to use a map that is not loaded.
 *	     Return value from str_clear_maps.
 *	     Return value from str_copyin_map.
 *
 * FLAGS MEANING:
 *	     TM_INPUT   - specifies input map
 *	     TM_OUTPUT  - specifies output map
 *	     TM_CLEAR   - remove map from user's tty
 *	     TM_RELOAD  - forced reload of an existing map. Only su can do this.
 *	     TM_LOAD    - load a new map. Only su can do this.
 *	     TM_USE     - assign map to user's tty.  Map must be loaded already.
 *	     TM_STICKY  - leave a map loaded after last user lets go of it
 *	     TM_DEFUNCT - map has been replaced, waiting for user(s) to let go
 */
static int
str_handle_set_map(q, mp )
	register queue_t *q;
	register mblk_t *mp;
	
{
	IOCP iocp;
	STRNLSP strnlsp;
	struct tty_map *inbound_tty_map_p;	/* user structure */
	mapp_t nlsmap;				/* single shared tty map */
	infop_t ttmapinfo;			/* state info for map in use */
	int flags;				/* flags provided by user */
	char *map_name;				/* map name from user */
	char dummy;
	int err;
	int	oldpri;

	iocp = (IOCP)mp->b_rptr;
	strnlsp = (STRNLSP) q->q_ptr;
	inbound_tty_map_p = &strnlsp->nls_tty_map ;

	/* copy the tty_map user structure from the message to kernel space */
	bcopy(mp->b_cont->b_rptr, inbound_tty_map_p, sizeof(struct tty_map)); 
	flags = inbound_tty_map_p->tm_flags;	  /* put in easier to use var */
	map_name = inbound_tty_map_p->tm_mapname; /* easier to handle name */

	/*
	 * User wants to remove mapping on this tty.  Only the flags field is
	 * meaningful on this call so this precedes the checks below.  Call a
	 * special function for this operation so that we can handle the case
	 * of clearing both maps at once in a rational manner without messy
	 * special cases here.
	 */
	if (flags & TM_CLEAR)
		return(str_clear_maps(strnlsp, flags));

	/*
	 * Only know about one kind of map, make sure she's using the one we
	 * know about.  TTMAP_VERSION is defined in nlsmap.h where the various
	 * map data structures are defined.  This code is written for version
	 * 1 maps, and is no longer compatible with the original version 0
	 * maps.  If Yet Another map version is created, most of the code here
	 * will still work as long as it is recompiled with the new
	 * definitions. The nasty stuff is in the routines which actually do
	 * the rule translations.  We also throw in a check to see if the user
	 * turned on the TM_DEFUNCT flag, since this is a good place to check
	 * it.  If it is on, it's very likely that they've botched something
	 * because it's only used by the kernel and is not user settable.  If
	 * they are trying to set it then that's an error anyway.
	 */
	if ((inbound_tty_map_p->tm_version != TTMAP_VERSION) ||
	    (map_name [0] == '\0') || flags & TM_DEFUNCT)
		return (EINVAL);

	/*
	 * Forced reload of a map.  Only root can do this.  If the map doesn't
	 * already exist, this becomes a plain load.  If it does exist, then
	 * we mark the pre-existing one so that we'll know it has been
	 * replaced, then it becomes a plain load.
	 */
	if (flags & TM_RELOAD) {
		int both = TM_INPUT | TM_OUTPUT;
		char buf [TTMAP_NAMELEN + 10];

		/* sanity check to be sure we don't have trouble later */
		if ((flags & TM_LOAD) || ((flags & both) == both))
			return (EINVAL);

		if(iocp->ioc_uid != (uid_t)0)
			return (EPERM);

		/*
		 * If a map with this name already exists, then modify its name so
		 * that it won't match again.  We DON'T want to make the name a
		 * null string, because then it will appear from the outside as if
		 * there is no map on this tty, when in fact there really is.  We
		 * change the name by pre-pending "OLD." to the front and
		 * truncating if necessary.
		 */
		oldpri = disable_lock(NLS_PRI, &nlsmaplock);
		if ((nlsmap = find_map(map_name)) != 0) {
			bcopy ("OLD.", buf, 4);
			bcopy (nlsmap->tm_mapname, &buf [4], TTMAP_NAMELEN - 4);
			buf [TTMAP_NAMELEN - 1] = '\0';
			bcopy (buf, nlsmap->tm_mapname, TTMAP_NAMELEN);
			/*
			 * This map has been replaced, turn off any stickiness so
			 * that it will be flushed when last user finishes with it.
			 * Mark as defunct so it becomes invisible to find_map.
			 */
			nlsmap->tm_flags &= ~TM_STICKY;
			nlsmap->tm_flags |= TM_DEFUNCT;
		}
		unlock_enable(oldpri, &nlsmaplock);
		flags &= ~TM_RELOAD;			/* turn off TM_RELOAD */
		flags |= TM_LOAD;			/* turn on TM_LOAD */
	/* fall through and do regular load operation */
	}

	/*
	 * Load a map.  Only root can do this.  The map must not already
	 * exist.  If the load succeeds, and if the TM_USE flag is also
	 * provided, then we fall thru and do a normal TM_USE request.  We do
	 * some extra pre-checking to be sure both the load and use operation
	 * will work before attempting the load, to avoid confusion about
	 * which part failed.
	 */
	if (flags & TM_LOAD) {
		int both = TM_INPUT | TM_OUTPUT;

		/*
		 * sanity check to be sure we don't have trouble later If TM_USE
		 * flag is present, make sure one and only one of TM_INPUT or
		 * TM_OUTPUT is present.
		 */
		if ((flags & TM_USE) &&
		   ((flags & both) == 0 || (flags & both) == both))
			return (EINVAL);

		if(iocp->ioc_uid != (uid_t)0)
			return (EPERM);	

		oldpri = disable_lock(NLS_PRI, &nlsmaplock);
		if ((nlsmap = find_map(map_name)) != 0)  {
			unlock_enable(oldpri, &nlsmaplock);
			return (EEXIST);  /* map with this name exists already */
		}

		err = str_copyin_map(inbound_tty_map_p, q, mp); 

		if (err || (strnlsp->nlsmap == NULL))
			unlock_enable(oldpri, &nlsmaplock);
		 else
			strnlsp->oldpri = oldpri;

		return (err);	/* if no error, map will come in M_IOCDATA part ! */

	}

	/*
	 * Set ttmapinfo to point to tty's map info pertaining to this
	 * request.  The struct pointed to is embedded in in the tty struct,
	 * it is NOT the tty's map, it is a struct which contains state info
	 * about the map being used by the tty.  It also contains a pointer to
	 * the actual map itself if one is in use.
	 */
	switch (flags & (TM_INPUT | TM_OUTPUT)) {
	case TM_INPUT:					/* input map */
		ttmapinfo = &strnlsp->nls_mapin;	/* tty's current map info */
		break;
	case TM_OUTPUT:					/* output map */
		ttmapinfo = &strnlsp->nls_mapout;	/* tty's current map info */
		break;
	default:
		return (EINVAL);
	}

	/* user wants to use specified map on this tty (must already exist) */
	if (flags & TM_USE) {
		oldpri = disable_lock(NLS_PRI, &nlsmaplock);
		if ((nlsmap = find_map(map_name)) == 0) {
			unlock_enable(oldpri, &nlsmaplock);
			return (ENOENT);
		}

		/* plug nlsmap into the struct pointed to by ttmapinfo */
		str_set_map(nlsmap, ttmapinfo);
		unlock_enable(oldpri, &nlsmaplock);

		return (0);				/* dat's all folks */
	}

	/* Whoa!  Fell out the bottom, must have been a bad argument */

	return (EINVAL);

}

/*
 * NAME:     str_handle_get_map
 *
 * FUNCTION: Copy a map from memory to user space.
 *
 * RETURNS:  0 on success : fixe informations are copied into the tty_map struct
 *	     and succeed in putting the M_COPYOUT message on the read side if the
 *	     map itself is required.
 *	     EINVAL It is tried to treat a bad version map.
 *		    We don't know which map to treat, neither TM_INPUT nor TM_OUTPUT
 *		    is specified or they are both specified.
 *	     ENOENT No input or output map is used by this tty.
 *	     EAGAIN The call to the allocb routine failed.
 *
 * FLAGS MEANING:
 *	     TM_INPUT   - specifies input map
 *	     TM_OUTPUT  - specifies output map
 */
static int
str_handle_get_map(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	STRNLSP strnlsp;
	int size;
	struct copyreq *cqp;
	mapp_t nlsmap;
	register struct tty_map *tty_map_p;

	strnlsp = (STRNLSP) q->q_ptr;
	/* the wanted tty_map user structure is already in this message */
	tty_map_p = (struct tty_map *)mp->b_cont->b_rptr;

	/* We only know about one flavor of map */
	if (tty_map_p->tm_version != TTMAP_VERSION)
		return (EINVAL);

	switch (tty_map_p->tm_flags) {
	case TM_INPUT:				/* user wants input map info */
		nlsmap = strnlsp->nls_mapin.tm_map;
		break;
	case TM_OUTPUT:				/* user wants output map */
		nlsmap = strnlsp->nls_mapout.tm_map;
		break;
	default:				/* say what?  1 and only 1 */
		return (EINVAL);
	}

	if (nlsmap == 0)
		return (ENOENT);

	/* Did the user provide a pointer to a buffer for the actual map? */
	if (tty_map_p->tm_addr != NULL) {		/* yes */

		/* If I have less, adjust her length, else trunc to her len */
		if (nlsmap->tm_len < tty_map_p->tm_len) {
			size = nlsmap->tm_len;
		} else {
			size = tty_map_p->tm_len;
		}
		/* preparing a M_COPYOUT message */
		cqp = (struct copyreq *) mp->b_rptr;
							/* address in user space */
		cqp->cq_addr = (caddr_t) tty_map_p->tm_addr;
		cqp->cq_size = size;
		cqp->cq_flag = 0;
					/* to copy out the actual map the message */
		mp->b_datap->db_type = M_COPYOUT; /* type is changed to M_COPYOUT */
		mp->b_wptr = (unsigned char *) (cqp + 1);

		freemsg(mp->b_cont);
		/* allocation of a message block with the size of the ttmap struct */
		if ( (mp->b_cont = allocb(size, BPRI_MED)) == NULL )
			return (EAGAIN);
		/* copy the actual map into this message */
		bcopy(nlsmap, mp->b_cont->b_rptr, size);

		mp->b_cont->b_wptr = mp->b_cont->b_rptr + size;

				/* for knowing that we are waiting afterwards for */
		strnlsp->nlsmap = nlsmap;	/* an M_IOCDATA message to come   */

		qreply(q, mp);		/* copyout the actual map into her buffer */
		return (0);
	}
	/*
	 * for getting only the mapname, the length and the flags of the actual map
	 * these values are copied to the tty_map structure in the actual message. 
	 * the copy out of this wanted user structure is done under the M_IOCTL part
	 * of the write side.
	 */

	bcopy(nlsmap->tm_mapname, tty_map_p->tm_mapname, TTMAP_NAMELEN);
	tty_map_p->tm_len = nlsmap->tm_len;
	tty_map_p->tm_flags = nlsmap->tm_flags;

	return (0);
}

/*
 * NAME:     find_map
 *
 * FUNCTION: Search the list of loaded tty maps looking for one with the
 *	     specified name.
 *
 * RETURNS:  A pointer to a mapp_t if found.
 *	     0 if not found
 */

static mapp_t
find_map( p )
	char *p;
{
    mapp_t nlsmap;

    for (nlsmap = ttmaplist; nlsmap != 0; nlsmap = nlsmap->tm_next) {
	if (nlsmap->tm_flags & TM_DEFUNCT) {
	    /* you're hallucinating, it isn't really there */
	    continue;
	}

	/* be careful, tm_key may not have a terminating null */
	if (bcmp(nlsmap->tm_mapname, p, TTMAP_NAMELEN) == 0) {
	    return (nlsmap);		/* found it */
	}
    }
    return (0);				/* no joy */
}

/*
 * NAME:     str_copyin_map
 *
 * FUNCTION: Asks the stream-head to copy a real map into the kernel space using
 *	     the info provided in the tty_map struct provided by the user on the
 *	     ioctl call. The answer to the M_COPYIN message put on the read side
 *	     will be an M_IOCDATA.
 *
 * RETURNS:  0 succeed in putting the M_COPYIN message on the read side.
 *	     EINVAL Bad arguments provided in the tty_map struct by the user.
 *	     ENOMEM The call to the 'alloc' routine failed.
 *
 */

static int
str_copyin_map(tty_map_p, q, mp)
	struct tty_map *tty_map_p; /* user tty_map structure copied into the kernel */
	register queue_t *q;
	register mblk_t *mp;
{
	STRNLSP strnlsp;
	struct copyreq *cqp;
	mapp_t nlsmap;
	int err, nomorecopyin;

	strnlsp = (STRNLSP) q->q_ptr;
	nomorecopyin = 0;

	NLS_PRINTF_ON(printf("str_copyin_map: tty_map_p == %x q == %x mp == %x\n", tty_map_p,q,mp));

	if ((tty_map_p->tm_len < 0) || (tty_map_p->tm_len > TTMAP_MAXSIZE)) 
		return (EINVAL);

	if (tty_map_p->tm_addr == 0) {
		/* No more copyin to do : the incomming message should contains
		   at least a tty_map and a ttmap structure			*/
		if (msgdsize(mp->b_cont) < sizeof(struct tty_map) + sizeof(struct ttmap))
			return (EINVAL);
		nomorecopyin = 1;
	}
   
	NLSMAP_MALLOC(nlsmap, mapp_t, tty_map_p->tm_len);
	if (!nlsmap)
		return (ENOMEM);

	NLS_PRINTF_ON(printf("str_copyin_map: nlsmap = %x\n", nlsmap));

	/* setting of strnlsp->nlsmap for knowing in M_IOCDATA part in
	   write side that we are waiting for such a  message type */

	strnlsp->nlsmap = nlsmap;

	NLS_PRINTF_ON(printf("str_copyin_map: strnlsp->nlsmap = %x\n", strnlsp->nlsmap));

	NLS_PRINTF_ON(printf("str_copyin_map: q->q_ptr = %x\n", q->q_ptr));
	NLS_PRINTF_ON(printf("str_copyin_map: mp->b_rptr = %x\n", mp->b_rptr));
	NLS_PRINTF_ON(printf("str_copyin_map: mp->b_cont->b_rptr = %x\n", mp->b_cont->b_rptr));
	NLS_PRINTF_ON(printf("str_copyin_map: nlsmap = %x\n", nlsmap));

	if (nomorecopyin) {
		/* pass the tty_map structure already treated	*/
		/* --> adjust both address and size		*/
		mp->b_cont->b_rptr += sizeof(struct tty_map);
		err = str_record_map(q, mp);
		strnlsp->nlsmap = NULL;
		return(err);
	}

	/* preparing the M_COPYIN message */
	/* setting cqp->cq_addr, cqp->cq_size, cqp->cq_flag and mp->b_wptr to their
	   appropiate values */

	cqp = (struct copyreq *) mp->b_rptr;
	cqp->cq_addr = (caddr_t) tty_map_p->tm_addr;
	NLS_PRINTF_ON(printf("cqp->cq_addr = %x\n", cqp->cq_addr));
	cqp->cq_size = tty_map_p->tm_len;
	cqp->cq_flag = 0;

	mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);

	/* changing the message type to M_COPYIN */
	mp->b_datap->db_type = M_COPYIN;	
	freemsg(mp->b_cont);
	mp->b_cont = NULL;

	NLS_PRINTF_ON(printf("In str_copyin_map:Before qreply: strnlsp->nlsmap = %x\n\n",strnlsp->nlsmap));
					/* required mapfile will come		*/
	qreply(q, mp);			/* under M_IOCDATA in "str_nls_wput"	*/

	NLS_PRINTF_ON(printf("In str_copyin_map:After qreply: strnlsp->nlsmap = %x\n\n",strnlsp->nlsmap));

	return (0);
}

/*
 * NAME:     str_record_map
 *
 * FUNCTION: Register a map = link it to ttmaplist.
 *
 * RETURNS:  0 succeed.
 *	     EINVAL Bad arguments provided in the tty_map struct by the user.
 *	    	    (concern only TM_bits in tm_flags but the map is really
 *		    loaded and linked in ttmaplist). 
 */

static int
str_record_map(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	STRNLSP strnlsp;
	int both, err;

	err = 0;
	strnlsp = (STRNLSP) q->q_ptr;

	/* copy the incomming ttmap structure into the reserved
	   area which was allocated before */
	bcopy(mp->b_cont->b_rptr, strnlsp->nlsmap,msgdsize(mp->b_cont));

	/* plug in the name of the mapfile */
	bcopy(strnlsp->nls_tty_map.tm_mapname,
	      strnlsp->nlsmap->tm_mapname, TTMAP_NAMELEN);

	/* set tm_len, tm_count, tm_next and tm_flags */
	strnlsp->nlsmap->tm_len = strnlsp->nls_tty_map.tm_len;
	strnlsp->nlsmap->tm_count = 0;
	strnlsp->nlsmap->tm_next = 0;
	both = TM_INPUT | TM_OUTPUT;
	strnlsp->nlsmap->tm_flags = strnlsp->nls_tty_map.tm_flags
							&(TM_STICKY | both);

	/* locking is done before! */
	strnlsp->nlsmap->tm_next = ttmaplist;
	ttmaplist = strnlsp->nlsmap;

	if (strnlsp->nls_tty_map.tm_flags & TM_USE) {
		switch (strnlsp->nls_tty_map.tm_flags & both) {
		case TM_INPUT:
			str_set_map(strnlsp->nlsmap, &strnlsp->nls_mapin);
			break;

		case TM_OUTPUT:
			str_set_map(strnlsp->nlsmap, &strnlsp->nls_mapout);
			break;

		default:
			err = EINVAL;
		}
	}

	return(err);
}

/*
 * NAME:     str_set_map
 *
 * FUNCTION: Set the specified map on a tty.  nlsmap points to the real map to use
 *	     (it may be null, which means remove mapping), ttmapinfo points into the
 *	     tty struct at the info which contains the relevant map state info and
 *	     the pointer to the map in use.
 *
 * RETURNS:  No return value
 *
 * COMMENTS: It is assumed that ttmaplist (linked list head of the ttmap loaded)
 *	     is locked via nlsmaplock.
 *
 */

static void
str_set_map(nlsmap, ttmapinfo)
	mapp_t nlsmap;
	infop_t ttmapinfo;
{

    /*
     * Before we do anything, check to see if the map pointer on this side
     * of the tty is already set to the requested value.  If they match,
     * then either both are NULL (clear the map, but no map in effect), or
     * the map addresses are the same (set mapping to the one already in
     * use).  In either case nothing needs to be done.  This test is
     * necessary, because if we blindly discard the map in use, and we are
     * already the only user of it, we'll throw away the map and then try
     * to use it again.  Can you say "time-bomb"?
     */
    if (ttmapinfo->tm_map == nlsmap) {
	return ;
    }

    /*
     * The ttmapinfo pointer points at the map state info for either the
     * input or output side of the tty (it points at a struct which is
     * imbedded in the tty struct itself, not the map struct) If this
     * ttmapinfo struct has a pointer to a map in it, then it currently
     * has mapping in effect and we need to remove the old map reference
     * before putting the new one into effect.  We first decrement the use
     * count then try to free the map.  The map will only be freed if the
     * use count has become zero, otherwise it is left in place for the
     * other tty(s) that are using it.
     */

    if (ttmapinfo->tm_map != 0) {
	ttmapinfo->tm_map->tm_count--;	/* one less user of this map */
	freemap(ttmapinfo->tm_map);	/* attempt to free old map */
    }

    ttmapinfo->tm_map = nlsmap;		/* new map (may be 0) */

    /*
     * Initialize state vars for this side of this tty.  These three
     * values need to be set so that the code which does the actual
     * mapping of the data stream starts in the proper state.  Setting
     * tm_bufindx to 0 makes the look-ahead buffer empty.  Setting
     * tm_rulindx to -1 indicates there is no pending rule state which
     * needs to be restored when the next data char is received.  A value
     * of '0' (ascii zero char, not the value zero) is the defined initial
     * value of of the "user map state".  This is basically a variable
     * that can be set and tested by the rules in the map, it is not
     * related in any way to mapping states or rule states as described in
     * other comments in this code.
     */
    ttmapinfo->tm_trouble = 0;		/* no trouble yet */
    ttmapinfo->tm_state = '0';		/* set base mapping state */
    ttmapinfo->tm_bufindx = 0;		/* buffer index */
    ttmapinfo->tm_rulindx = -1;		/* no rule in progress */

    /*
     * If we actually connected to a new map, rather than clearing the map
     * pointer for this side of this tty, bump the usage count so the map
     * won't be freed until we're done with it.
     */

    if (nlsmap) 
	nlsmap->tm_count++;	/* one more user of this map */

    return ;
}

/*
 * NAME:     str_clear_maps
 *
 * FUNCTION: Remove mappings from he input or output or both sides of a tty. 
 *
 * RETURNS:  On success 0 is returned.
 *	     EINVAL neither input nor output flag.
 *
 */

static int
str_clear_maps(STRNLSP strnlsp, int flags)
{
	int	oldpri;
	/* Must provide at least one of the input/output flags */
	if ((flags & (TM_INPUT | TM_OUTPUT)) == 0)
		return (EINVAL);

	oldpri = disable_lock(NLS_PRI, &nlsmaplock);

	if (flags & TM_INPUT)			/* Clear the input map? */
		str_set_map((mapp_t)0, &strnlsp->nls_mapin);

	if (flags & TM_OUTPUT)			/* how about the output map? */
		str_set_map((mapp_t)0, &strnlsp->nls_mapout);

	unlock_enable(oldpri, &nlsmaplock);

	return (0);

}

/*
 * NAME:     freemap
 *
 * FUNCTION: free a map if no one is using it and it's not sticky 
 *
 * RETURNS:  No return value
 *
 */

static void
freemap(mapp_t nlsmap)
{
    register mapp_t *tt2;

    if (!nlsmap || !ttmaplist || nlsmap->tm_count ||
	(nlsmap->tm_flags & TM_STICKY))
	return;

    for (tt2 = &ttmaplist; *tt2; tt2 = &(*tt2)->tm_next )
	if (*tt2 == nlsmap) {
	    *tt2 = (*tt2)->tm_next;
	    NLSMAP_FREE(nlsmap);
	    return;
	}
}

/*
 * NAME:     nls_send_data
 *
 * FUNCTION: This routine handles the M_DATA message arriving at the write side
 *
 * RETURNS:  1 on success
 *	     0 if mp has to be (re)queued
 *
 */

static int
nls_send_data(ttmapinfo, q, mp)
	infop_t ttmapinfo;
	register queue_t *q;
	register mblk_t *mp;
{    register mblk_t *bp;
     int ret;

    /*
     * nmp is the message pointer to the new message which will hold the
     * resultat of the mapped characters;
     * nbp is the pointer to one message block in this new message
    */
    mblk_t *nmp = NULL, *nbp = NULL;
    
    /* Does flow control allows passing a message to the next queue ? */
    if(!canput(q->q_next))
	    return (0);       /* No */

    /* Is character mapping switched on ? */
    if(ttmapinfo->tm_map == (mapp_t) NULL) {
	    putnext(q, mp);               /* character mapping not switched on */
	    return (1);
    }

    /* Looking for all message blocks in the original message */ 
    for(bp = mp; bp != NULL; bp = bp->b_cont) {

	    /* Looking for all characters in a message block */
	    while(bp->b_rptr < bp->b_wptr) {

		    /* Try to mapp one character of the original message */
		    if((ret=str_ttmap(&nbp, *bp->b_rptr, ttmapinfo, q)) == -1) {
			    /* allocb failed in str_ttmap */

			    if(nmp != NULL)  /* send characters already mapped! */
				    putnext(q, nmp); 
			    return (0);
		    }
		    else if(ret)
			    /*
			     * There is not enough space in the new message 
			     * block for at least TTMAP_RE_SIZE characters!
			    */
			    goto newblk; 
		    else {  /* ret == 0 */
			    bp->b_rptr++; /* treating next character of the original message */
			    continue;
		    }

            newblk: /* for building the new message from the new message blocks */
		    if(nmp == NULL)
			    nmp = nbp; /* if nbp points to the first message block
					  in the new message */
		    else
			     /* if nbp does not point to the first message block
				in the new message */
			    linkb(nmp, nbp);
		    nbp = NULL;
	    } /* while */
    } /* for */

    /* all characters now mapped! */
    if(nmp == NULL)
	    nmp = nbp; /* if nbp points to the first message block
			  in the new message */
    else
		    /* if nbp does not point to the first message block
		       in the new message */
	    linkb(nmp, nbp);
    freemsg(mp);    /* free original message - it is no longer necessary */

    if(nmp)
	    putnext(q, nmp);

    return (1);
}

/*
 * NAME:      str_ttmap
 *
 * FUNCTION:  Map terminal characters for National Language Support.
 *	      Subroutine to translate i/o characters according to tty i/o maps.  
 *	      After mapping a character, the mapping resultat will be added to the
 *	      output block( *bp->b_wptr++ ). 
 *
 * RETURNS:   0 succeed in treating the character.
 *	      1 There is not enough space in the new message block for at least
 *		TTMAP_RE_SIZE characters!
 *	     -1 The call to the allocb routine failed.
 *
 */

static int
str_ttmap(bpp, ch, ttmapinfo, q)
	mblk_t **bpp;
	int ch;
	infop_t ttmapinfo;
	queue_t *q;
{   mapp_t nlsmap;			/* ptr to map (rules) */
    char *pat, *rep;			/* pattern/replace strings */
    char *buf_base;			/* base of hold buffer */
    char *buf_ptr;			/* next position in hold buf */
    char *source;			/* for walking src stream */
    char *msg;
    int chars_buffered;			/* chars in the hold buf */
    int rule_index;			/* rule we're currently using */
    int default_rule;			/* index of default rule */
    int pat_state;			/* state of pattern */
    int err = 0;
    int i;
    char *msg1;    
    mblk_t *bp;  /* message block pointer for holding the mapped 
		    characters */
    register STRNLSP strnlsp;
    int	oldpri;

    strnlsp = (STRNLSP)q->q_ptr;

    /* bp points to an existing message block pointer ? */
    if((bp = *bpp) != NULL) { 
	/*
	 * Is there enough space in the message block for at least
	 * TTMAP_RE_SIZE characters ? Otherwise -> return 1
	*/
	    if((bp->b_datap->db_lim - bp->b_wptr) < TTMAP_REP_SIZE)
		    return (1);
    }
    /* message block pointer is NULL */
    else
	if((*bpp = bp = allocb(MODBLKSZ, BPRI_MED)) == NULL) {
		/* if allocb failed => recovery from failure with bufcall */

		if(ttmapinfo == &strnlsp->nls_mapin) { /* mapping at read/input side */
			allocb_recovery(q, &strnlsp->rbid,  &strnlsp->rtid, MODBLKSZ);
		}
		else {	/* mapping at write/output side */ 
			allocb_recovery(q, &strnlsp->wbid,  &strnlsp->wtid, MODBLKSZ);
		}
		
		return (-1); /* mp will be put to the queue in str_nls_rput/wput()
			      * or in str_nls_rsrv/wsrv(). May be, that the rptr
			      * pointer has changed so that some characters have
			      * already been mapped .
			      */
	}


    /*
     * Setup various state variables from saved information in the
     * ttmapinfo struct. A key piece of information here is the
     * variable ttmapinfo->tm_rulindx, if it is -1, then there is no
     * pending mapping state, so the first input char will be used as
     * a hash key to find the first rule to apply. This code assumes
     * that ttmapinfo->tm_bufindx is consistent with tm_rulindx. This
     * routine sets it properly when a rule is finished, but the map
     * loading routines must also set things up properly for the
     * initial base state.
     */
    nlsmap = ttmapinfo->tm_map;
    rule_index = ttmapinfo->tm_rulindx; /* restore rule index */
    chars_buffered = ttmapinfo->tm_bufindx; /* restore buf size */
    buf_ptr = (buf_base = ttmapinfo->tm_buffer) + chars_buffered;
    default_rule = nlsmap->tm_default;	/* note def rule # */
					/* conflict with int ch ? */
    *buf_ptr++ = ch;			/* get next char from orig. message to map */
    chars_buffered++;			/* one more char in hold buf */
    ttmapinfo->tm_bufindx = chars_buffered; /* save state */
    goto try_pattern;			/* try matching the pattern */

 next_pattern:
    ttmapinfo->tm_rulindx = rule_index = nlsmap->tm_rule[rule_index].tm_next;

    /*
     * Only the default rule has a next value of -1, if we try to step
     * past it, then something is wrong, since it should always match.
     * The first time we complain, the second time we remove the map.
     * Hopefully, this will give the user a chance to replace the map,
     * if we do a forced remove, the map will be orphaned and float
     * around in the kernel until reboot.
     * 
     * (bad_rule_gripe)
     */
    if (rule_index == -1) {
	if (ttmapinfo->tm_trouble++) {
	    msg = " twice, map removed";
	    msg1 = "nls: str_ttmap: next_pattern: ";
	    str_nls_maperr(msg1, ERRID_NLS_BADMAP, NLS_BADRULE, 0, 0, 
		nlsmap->tm_mapname);

	    /* critical section ! */
	    oldpri = disable_lock(NLS_PRI, &nlsmaplock);

	    ttmapinfo->tm_map->tm_count--;	/* one less user of this map */
	    freemap(ttmapinfo->tm_map);		/* attempt to free old map */
	    unlock_enable(oldpri, &nlsmaplock);

	    ttmapinfo->tm_map = 0;
	} else {
	    msg = ", map may be corrupted";
	    str_nls_err(msg1, ERRID_NLS_MAP, NLS_WARNRULE);
	}

	nlsprintf("ttmap: map %s: ran past default rule%s\n",
	    nlsmap->tm_mapname, msg);
	goto flush_map_state;
    }

 try_pattern:
    /* point at source chars we'll be looking at */
    source = buf_base;
    pat_state = '0';			/* assume pattern matches in state 0 */

    /* if rule_index is -1, no rule in process, get hash value to start */
    if (rule_index == -1)
	ttmapinfo->tm_rulindx = rule_index =
	    nlsmap->tm_hash[(int) *buf_base];

    /* sanity check the rule index to be used */
    if ((rule_index < 0) || (rule_index >= nlsmap->tm_num_rules)) {

	/*
	 * let's hope we don't seg fault on references thru nlsmap...
	 * (bad_map_gripe)
	 */
	msg1 = "nls: str_ttmap: try_pattern:";
	str_nls_maperr(msg1, ERRID_NLS_BADMAP, NLS_BADMAP, rule_index, 
	    nlsmap->tm_num_rules - 1, nlsmap->tm_mapname);
	nlsprintf("ttmap: map %s was removed, rule %d out of range (max=%d)\n",
	    nlsmap->tm_mapname, rule_index, nlsmap->tm_num_rules - 1);

	/* critical section ! */
	oldpri = disable_lock(NLS_PRI, &nlsmaplock);

	ttmapinfo->tm_map->tm_count--;	/* one less user of this map */
	freemap(ttmapinfo->tm_map);		/* attempt to free old map */
	unlock_enable(oldpri, &nlsmaplock);

	ttmapinfo->tm_map = 0;
	goto flush_map_state;
    }

    /*
     * Heuristic. If the rule we're about to try is the default rule,
     * short-circuit the pattern parsing and send the character out
     * directly. This will work properly even if the default rule
     * doesn't live in slot zero, as long as the proper index is in
     * nlsmap->tm_default. Note that this code assumes the default rule
     * is ?:$1 (map char to itself). If the default rule should ever
     * be changed to something else, this heuristic will defeat it.
     * In that case, just remove this if statement and the default
     * rule will be parsed like all the others.
     */
    if (rule_index == default_rule) {
	    *bp->b_wptr++ = *source++; /* put this character into the new message */
	    goto shift_buffer;
    }

    pat = nlsmap->tm_rule[rule_index].tm_pattern; /* pat string to use */
    rep = nlsmap->tm_rule[rule_index].tm_replace ; /* rep str if match */

    /* walk thru the pattern attempting to match the source stream */
    while (TRUE) {
	register char pat_char = *pat++; /* next char from pat string */
	char *newp;			/* avoid doing &pat, so it can be */

	if (pat_char == '\0') {		/* pattern exhausted? */
	    if (pat_state == ttmapinfo->tm_state)
		goto do_replacement;	/* yes, matched */
	    else
		goto next_pattern;
	}

	if (source == buf_ptr) {	/* anymore chars in hold buf? */
	    if (pat_char != '@')
		return (0);

	    /*
	     * Special case for trailing state spec. If we've run out
	     * of input, but the next char in the pattern is a state
	     * spec (which doesn't consume any input) then go ahead
	     * and fall into the switch to see if the state matches.
	     * If the state doesn't match, then we can give up on this
	     * pattern and start the next one. If state matches, and
	     * there is no more pattern following the state spec,
	     * we'll get a pattern match at the top of the loop. If
	     * state matches, and there are more pattern chars, we'll
	     * come through here again and wind up trying for more
	     * input.
	     */
	}

	switch (pat_char) {
	case '\\':			/* next char escaped */
	    pat_char = *pat++;		/* look at next */
	    if (pat_char == '0')	/* special case for */
		pat_char = 0;		/* escaped null */
	    break;

	case '?':			/* matches anything */
	    pat_char = *source;		/* plug in the char from the */
	    break;			/* src for compare below */

	case '[':			/* match against set of chars */
	    if (char_in_set(*source, pat, &newp)) {
		pat_char = *source;	/* setup for compare */
		pat = newp;		/* move ptr past set */
		break;
	    }
	    goto next_pattern;		/* match failed, next rule */

	case '@':			/* state setting correct? */
	    pat_state = *pat++;
	    if (pat_state != ttmapinfo->tm_state) {
		goto next_pattern;	/* nope */
	    }
	    continue;			/* state ok, next pat char */

	    /* no default case, compare char to pattern char */
	}				/* end of switch (pat_char) */

	if (pat_char != *source++) {	/* does src match pattern? */
	    goto next_pattern;
	}
    }					/* end of while (TRUE) */


 do_replacement:
    {
	register char rep_char;

	if (source == buf_base) {	/* nothing was snarfed */
	    msg1 = "nls: str_ttmap: do_replacement: ";
	    str_nls_err(msg1, ERRID_NLS_MAP, NLS_NULLPAT);
	    nlsprintf("ttmap: pattern exhausted but no chars snarfed\n");

	    *bp->b_wptr++ = *source++;	/* put this character into the new message */
	    goto shift_buffer;

	}

	while (rep_char = *rep++) {	/* until we hit the null */
	    switch (rep_char) {
	    case '\\':			/* next char is escaped */
		rep_char = *rep++;	/* the actual char to use */
		if (rep_char == '0')	/* special case for NULL */
		    rep_char = '\0';	/* replace with actual null */
		break;

	    case '@':			/* state indicator */
		ttmapinfo->tm_state = *rep++;
		continue;

	    case '$':			/* use a char from source */
		rep_char = buf_base[*rep++ - '1'];
		break;

		/* no default case, fall out */
	    }

	    *bp->b_wptr++ = rep_char;	/* put this character into the new message */

	}
    }

 shift_buffer:
    {
	register int chars_snarfed = source - buf_base;
	register char *to, *from, *end;

	if (chars_snarfed < chars_buffered)
	    for (to = buf_base, from = source, end = buf_base + chars_buffered;
		 from < end;
		 *to++ = *from++);

	chars_buffered -= chars_snarfed; /* this many less buffered */
	ttmapinfo->tm_bufindx = chars_buffered; /* save new state */
	ttmapinfo->tm_rulindx = rule_index = -1; /* clear rule state */
	buf_ptr -= chars_snarfed;	/* backup ptr to next slot */
/*	if (chars_buffered) 
	    nlsprintf("ttmap: lookahead wasn't emptied (%d, %d)\n",
		chars_snarfed, chars_buffered);*/
    }
    if (chars_buffered) {		/* if lookahead wasn't */
	goto try_pattern;		/* emptied, try matching */
    }
    return (0);

 flush_map_state:
    ttmapinfo->tm_bufindx = 0;		/* clear hold buffer */
    ttmapinfo->tm_rulindx = -1;		/* no pending rule */
    ttmapinfo->tm_state = '0';		/* clear rule state */
    nlsprintf("ttmap: mapping state flushed\n");
    return (0);				/* err */
}

/*
 * NAME:     char_in_set
 *
 * FUNCTION: Parse a set of chars to see if c is in it. The pointer pat points at
 *	     the first char in the set, just past the [ char. The parm newp is a
 *	     pointer to a pointer which should be set to the address of the char
 *	     following the closing ] of the set.
 *
 * RETURNS:  TRUE  if c is in the set of chars,
 *	     FALSE if not.
 *
 */

static int
char_in_set(c, pat, newp)
	char c;	char *pat;	char **newp;
{   char *p = pat;			/* working pointer into the set */
    char pat_char;			/* temp char from the set for compare */
    char end_char;

    while (TRUE) {
	pat_char = *p++;		/* pull a char from the set */
	if (pat_char == '\\') {		/* escaped? */
	    pat_char = *p++;		/* yes, get the real value */
	    if (pat_char == '0') {	/* special case for null */
		pat_char = 0;		/* \0 is really zero */
	    }
	} else {
	    if (pat_char == ']') {	/* not escaped, end of set? */
		*newp = p;		/* yes, update scan ptr */
		return (FALSE);		/* exhausted set, no joy */
	    }
	}
	if (c == pat_char) {		/* does char match this one */
	    skip_past_set(p, newp);	/* yes, skip rest of set */
	    return (TRUE);		/* no sweat, kimosabe */
	}
	if (*p != '-') {		/* first char of a range? */
	    continue;			/* no */
	}
	p++;				/* yes, get end of range */
	end_char = *p++;
	if (end_char == '\\') {		/* escaped? */
	    end_char = *p++;		/* yes, get the real value */
	    if (end_char == '0') {	/* special case for null */
		end_char = 0;		/* \0 is really zero */
	    }
	}

	if ((c >= pat_char) && (c <= end_char)) { /* is it in range? */
	    skip_past_set(p, newp);	/* yes */
	    return (TRUE);		/* success */
	}
    }
}

/*
 * NAME:     skip_past_set
 *
 * FUNCTION: Skip past the remainder of a set spec pointed at by p.
 *
 * RETURNS:  No return value
 *
 */

static void
skip_past_set(p, newp)
	char *p; char **newp;
{   char c;				/* temp char */

    while (TRUE) {
	c = *p++;			/* look at char, bump pointer */
	if (c == '\\') {		/* watch out for escaped chars */
	    p++;			/* skip over char following \ */
	    continue;			/* loop around for next char */
	}
	if (c == ']') {			/* is it a right bracket */
	    *newp = p;			/* p already points just past c */
	    return;			/* that's it */
	}
    }
}

/*
 * NAME:     str_nls_err
 *
 * FUNCTION: Service function for str_ttmap 
 *
 * RETURNS:  No return value
 *
 */

static void
str_nls_err(msg, code, data)
	char *msg; int code; int data;
{   ERR_REC(sizeof(int)) err;

    err.error_id = code;
    bcopy(msg, err.resource_name, sizeof(err.resource_name));
    *(int *)err.detail_data = data;
    errsave(&err, sizeof(err));
}

/*
 * NAME:     str_nls_maperr
 *
 * FUNCTION: Service function for str_ttmap 
 *
 * RETURNS:  No return value
 *
 */

static void
str_nls_maperr(msg, code, data0, data1, data2, errstr)
	char *msg; int code; int data0; int data1; int data2; char *errstr;
{   struct errmsg {
	struct err_rec0 err;
	int data[3];
	char errstr[TTMAP_NAMELEN];
    } errmsg;

    errmsg.err.error_id = code;
    bcopy(msg, errmsg.err.resource_name, 
	sizeof(errmsg.err.resource_name));
    errmsg.data[0] = data0;
    errmsg.data[1] = data1;
    errmsg.data[2] = data2;
    bcopy(errstr, errmsg.errstr, TTMAP_NAMELEN);
    errmsg.errstr [TTMAP_NAMELEN - 1] = '\0';
    errsave(&errmsg, sizeof(errmsg));
}

/*
 * NAME:     allocb_recovery
 *
 * FUNCTION: This routine does the "error" recovery after an allocb() failure.
 *
 * RETURNS:  No return value
 *
 */

static void
allocb_recovery(q, bid_p, tid_p, num)
	queue_t	*q;
	int *bid_p;
	int *tid_p;
	unsigned num;
{
	if(*bid_p) /* error recovery with bufcall() */ 
		unbufcall(*bid_p);

	if(!(*bid_p = bufcall(num, BPRI_MED, qenable, q))) {
		if(*tid_p)
			untimeout(*tid_p);
		*tid_p = timeout(qenable, q, hz*2);
	}
}	

#ifdef _KDB

/*
 * NAME:     search_module
 *
 * FUNCTION: Search the queue of a module using its name in a stream.
 *	     The queue pointer given as argument is the one of the write side.
 *
 * RETURNS:  /= 0 : The read side queue pointer of the module "module_name"
 *		    in the stream.
 *	      = 0 : There is no module "module_name" in this stream.
 *
 */

queue_t *
search_module(queue_t *qp, char *module_name)
{
	queue_t *qp1;
	struct module_info *mi;

	qp1 = qp;
	while (qp1) {
		mi = qp1->q_qinfo->qi_minfo;
		if (strcmp(mi->mi_idname, module_name) == 0) {
			return (OTHERQ(qp1));
		}
		qp1 = qp1->q_next;
	}
	return(0);
}

/*
 * NAME:     nls_chk_nls_ttmapp
 *
 * FUNCTION:
 *
 * RETURNS:  = 1 : The parameter passed as argument is really a struct
 *		   ttmap pointer.
 *	     = 0 : is not.
 *
 */

int
nls_chk_nls_ttmapp(struct ttmap *chk_ttmapp)
{
	struct ttmap *ttmapp;

	for(ttmapp = ttmaplist; ttmapp != NULL; ttmapp = ttmapp->tm_next) {
		if(ttmapp == chk_ttmapp)
			break;
	}

	return((ttmapp == NULL) ? 0 : 1);
}

/*
 * NAME:     nls_chk_nlsp
 *
 * FUNCTION:
 *
 * RETURNS:  = 1 : The parameter passed as argument is a struct nls pointer.
 *	     = 0 : is not.
 *
 */

int
nls_chk_nlsp(struct nls *chk_nlsp)
{
	extern STHP	sth_open_streams[];
	#define STH_HASH_TBL_SIZE	32

	int i;
	STHP sth;
	queue_t *qp;

	for (i = 0; i < STH_HASH_TBL_SIZE; i++) {
	    for (sth = sth_open_streams[i]; sth; sth = sth->sth_next) {
		if (qp = search_module(sth->sth_wq, NLS_NAME))
		{
			if ((qp->q_ptr == (char *) chk_nlsp) ||
			    (OTHERQ(qp)->q_ptr == (char *) chk_nlsp))
				return(1);
		}
	    }
	}
	return(0);
}

/*
 * NAME:     nls_ttmaplist_print
 *
 * FUNCTION: Print the linked list of ttmap structure(s)
 *
 * RETURNS:  none
 *
 */

void
nls_ttmaplist_print()
{
	int i = 7;
	struct ttmap *ttmapp;

	printf("nls_ttmaplist:");

	for(ttmapp = ttmaplist; ttmapp != NULL; ttmapp = ttmapp->tm_next) {
		if (i == 7) {
			printf("\n   ");
			i = 0;
		}
		printf("0x%08X  ", ttmapp);
		i++;
	}
}

/*
 * NAME:     nls_ttmap_print
 *
 * FUNCTION: Print a ttmap structure
 *
 * RETURNS:  none
 *
 */

void
nls_ttmap_print(struct ttmap *ttmapp)
{
	printf("nls_ttmap:\n");

	printf("   ");
	printf("tm_len=%d tm_count=%d tm_flags=0x%02X tm_num_rules=%d\n",
		ttmapp->tm_len, ttmapp->tm_count, ttmapp->tm_flags,
		ttmapp->tm_num_rules);

	printf("   ");
        printf("tm_default=%d tm_first_wild=%d tm_mapname=%s\n",
		ttmapp->tm_default, ttmapp->tm_first_wild,
		&ttmapp->tm_mapname);  
	printf("   ");
	printf("tm_next=0x%08X tm_hash=0x%08X tm_rule=0x%08X\n",
		ttmapp->tm_next, ttmapp->tm_hash, ttmapp->tm_rule);
}

/*
 * NAME:     nls_line_list_print
 *
 * FUNCTION: Print the list of all the nls lines registered in the STREAMS
 *	     framework. 
 *
 * RETURNS:  none
 *
 */

void
nls_line_list_print()
{
	extern STHP	sth_open_streams[];
	#define STH_HASH_TBL_SIZE	32

	int i;
	STHP sth;
	queue_t *qp;

	queue_t *qp1;
	str_rsp_t str_rsp;

	printf("nls_line_list:\n");

	for (i = 0; i < STH_HASH_TBL_SIZE; i++) {
	    for (sth = sth_open_streams[i]; sth; sth = sth->sth_next) {
		if (qp = search_module(sth->sth_wq, NLS_NAME))
		{
			qp1 = search_module(sth->sth_wq, SRS_NAME);
			printf("   ");
			if (qp1) {
				str_rsp = (str_rsp_t)qp1->q_ptr;
				printf("%s : ", str_rsp->t_name);
			} else {
				printf("tty? : ");
			}
			printf("dev=%d,%d ",
				major(sth->sth_dev), minor(sth->sth_dev));
			printf("rq=0x%08X q_ptr=0x%08X wq=0x%08X q_ptr=0x%08X\n",
				qp, qp->q_ptr, OTHERQ(qp), OTHERQ(qp)->q_ptr);
		}
	    }
	}
}

/*
 * NAME:     kdb_nls_print
 *
 * FUNCTION: Print a nls structure
 *
 * RETURNS:  none
 *
 */
void
kdb_nls_line_print(STRNLSP nlsp)
{
	nls_line_print(nlsp, nlsp->nls_mapin.tm_map, nlsp->nls_mapout.tm_map);
}

/*
 * NAME:     kdb_nls_print
 *
 * FUNCTION: Call the specific print routines.
 *
 * RETURNS:  Always 0
 *
 */

int
kdb_nls_print(int v, void *tp, int argv)
{

	switch (v) {
	case NLS_KDB_M: 
		nls_ttmaplist_print();
		break;

	case NLS_KDB_m: 
		nls_ttmap_print(tp);
		break;

	case NLS_KDB_T: 
		nls_line_list_print();
		break;

	case NLS_KDB_STD: 
	case NLS_KDB_t: 
		kdb_nls_line_print(tp);
		break;

	default:
		break;
	}
    return(0);
}

/*
 * NAME:     nls_print_help
 *
 * FUNCTION: print the usage of "call nls_kdb".
 *
 * RETURNS:  no return value
 *
 */

void
nls_print_help()
{
	printf("\nUsage:\tcall nls_kdb M/m/T/t/K[v] no_param/@(ttmap struct)/no_param/@(nls struct)/@(nls struct)\n");
}

/*
 * NAME:      nls_kdb_next_param
 *
 * FUNCTION:  Read all blanks between current character and next parameter
 *
 * ARGUMENTS: buf  = kdb sub-command line.
 *	      poff = index in buf string.
 *	      len  = length of buf.
 *
 * RETURNS:   1 if buf is not empty
 *	      0 if it is.
 *
 */

nls_kdb_next_param(unsigned char *buf, int *poff, int len)
{

	while ((len - *poff) > 0 && (buf[*poff] == ' ')) (*poff)++;

	return(((len - *poff) <= 0) ? 0 : 1);
}

/*
 * NAME:      nls_kdb
 *
 * FUNCTION:  Print function for kdb debugger :
 *	      Converts the string to parameter list,
 *	      Prints some local information,
 *	      Calls the lldb print function nls_print.
 *
 * ARGUMENTS: buf  = kdb sub-command line.
 *	      poff = index in buf string.
 *	      len  = length of buf.
 *
 * RETURNS:  no return value
 *
 */

void
nls_kdb(unsigned char *buf, int *poff, int len)
{
	int ch;
	long tp;
	int v;
	int argv = 0;

	/*
	 * Read all blanks between function name and first parameter
	 */
	nls_kdb_next_param(buf, poff, len);
	ch = buf[*poff] & 0xFF;
	(*poff)++;

	switch (ch) {
	case 'M':
						/* address of all maps */
		tp = 0;				/* for call nls_print reason */
		v = NLS_KDB_M;
		break;

	case 'm':
						/* data of one map */
		TST_NLS_KDB_NEXT_PARAM(buf, poff, len);
		tp = mi_strtol (buf + *poff, NULL, 16);

		if (!nls_chk_nls_ttmapp(tp)) {
			printf("0x%08X is not a valid struct ttmap pointer\n", tp);
			return;
		}

		v = NLS_KDB_m;
		break;

	case 'T':
						/* all the tty line with nls */
		tp = 0;				/* for call nls_print reason */
		v = NLS_KDB_T;
		break;

	case 't':
						/* one tty line with nls */
		TST_NLS_KDB_NEXT_PARAM(buf, poff, len);
		tp = mi_strtol (buf + *poff, NULL, 16);

		if (!nls_chk_nlsp(tp)) {
			printf("0x%08X is not a valid struct nls pointer\n", tp);
			return;
		}

		v = NLS_KDB_t;
		break;

	default:
		nls_print_help();
		v = NLS_KDB_UNKNOW;
		break;
	}

	if (v != NLS_KDB_UNKNOW) {
		kdb_nls_print(v, tp, argv);
	}
	return;
}

#endif	/* _KDB */
