#ifndef lint
static char sccsid[] = "@(#)26 1.22.1.2 src/bos/kernext/tty/sptr.c, sysxsptr, bos41J, 9523C_all 6/9/95 15:12:49";
#endif
/*
 * COMPONENT_NAME: SYSXTTY (Streams Based Serial Line Printer module = sptr)
 * 
 * FUNCTIONS: sptr_config(), str_sptr_open(), str_sptr_close(), 
 *	      str_sptr_rput(), str_sptr_rsrv(), str_sptr_wput(),
 *	      str_sptr_wsrv(), tioc_handle(), sptr_ioctl(), sptr_mctl(),
 *	      write_mread(), send_Xctl_tiocgeta(), start_write(),
 *	      format_data(), sptr_prnformat(), addc(),
 *	      str_sp_sendff(), write_mflush(), read_mdata(), write_timeout(),
 *	      allocb_recovery(), cts_handle(), cts_timeout(), read_mpcproto(),
 *	      write_req_ioctl(), ioctl_reply(), read_miocack(), read_miocnak()
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include "sptr.h"
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/strconf.h>	/* for definition of STR_LOAD_MOD ... */
#include <sys/lockl.h>
#include <sys/lock_def.h>

#define	TRC_SPTR(subhookid)	((HKWD_STTY_SPTR)|subhookid)

#define	PREVQ(q)		(OTHERQ(OTHERQ(q)->q_next))
#define	NEXTQ(q)		(q->q_next)
#define	DIAGQ(q)		(OTHERQ(q->q_next))

	/* lock for manipulate sptr_config_list in the config/open routines */
int sptr_lock_config = LOCK_AVAIL;

/* function definitions of the sptr module */

int sptr_config(), str_sptr_open(), str_sptr_close(); 
int str_sptr_rput(), str_sptr_rsrv(), str_sptr_wput(), str_sptr_wsrv();

static int tioc_handle(), sptr_ioctl(), sptr_mctl(), write_mread();
static int send_Xctl_tiocgeta(), start_write(); 
static int format_data(), sptr_prnformat(), addc(), str_sp_sendff();
static void write_mflush(), read_mdata(), write_timeout(), allocb_recovery();
static void cts_handle(), cts_timeout(), read_mpcproto();
static void write_req_ioctl(), ioctl_reply(); 

/* functions in sptr_db.c */

#ifdef _KDB
extern	void	sptr_config_print(), sptr_line_print();
#endif	/* _KDB */

#define	SPTR_NAME	"sptr"

static struct module_info minfo = {
	7551,		/* module id     */
	SPTR_NAME,	/* module name   */
	0,		/* min pkt size  */
	INFPSZ,		/* max pkt size  */
	SPTR_IHIWAT,		/* hiwater       */
	SPTR_ILOWAT,		/* lowater       */
};

static struct qinit rinit = {
	str_sptr_rput, str_sptr_rsrv, str_sptr_open, str_sptr_close, NULL, &minfo, NULL
};

static struct qinit winit = {
	str_sptr_wput, str_sptr_wsrv, NULL, NULL, NULL, &minfo, NULL
};

struct streamtab str_sptr_info = { &rinit, &winit, NULL, NULL };

#ifdef DEBUG_SPTR
#define	SPTR_PRINTF_ON(X)	( (sptr_printf_on) ? (X) : 0 )
short int sptr_printf_on = 0;

			/* anchor of linked list of sptr_config structures */
struct sptr_config *sptr_config_list = 0;		/* seen by KDB */
#else
#define	SPTR_PRINTF_ON(X)

static struct sptr_config *sptr_config_list = 0;       /* local to the module */
#endif

/* The initialisation of str_sptr_tioc_reply[] done by the sptr module is : */

static struct tioc_reply 
       str_sptr_tioc_reply[ ] = { 
       { IOCINFO, sizeof(struct devinfo), TTYPE_COPYOUT },
       { LPRGET,  sizeof(struct lprio),   TTYPE_COPYOUT },
       { LPRSET,  sizeof(struct lprio),   TTYPE_COPYIN  },
       { LPRMODG, sizeof(struct lprmod),  TTYPE_COPYOUT },
       { LPRMODS, sizeof(struct lprmod),  TTYPE_COPYIN  },
       { LPRGTOV, sizeof(struct lptimer), TTYPE_COPYOUT },
       { LPRSTOV, sizeof(struct lptimer), TTYPE_COPYIN  },
       { LPQUERY, sizeof(struct lpquery), TTYPE_COPYOUT },
       { LPRGETA, sizeof(struct lpr232),  TTYPE_COPYOUT },
       { LPRSETA, sizeof(struct lpr232),  TTYPE_COPYIN  },
       { TCGETA, sizeof(struct termio),  TTYPE_COPYOUT },
       { TCSETA, sizeof(struct termio),  TTYPE_COPYIN  },
       { TCSETAW, sizeof(struct termio),  TTYPE_COPYIN  },
       { TCSETAF, sizeof(struct termio),  TTYPE_COPYIN  },
}; 

#ifdef _KDB
/* Definitions for sptr_kdb */

#define	SPTR_KDB_UNKNOW	-1
#define	SPTR_KDB_STD	0
#define	SPTR_KDB_C	1
#define	SPTR_KDB_c	2
#define	SPTR_KDB_l	3

struct names {
	char *n_str;
	unsigned n_mask;
	unsigned n_state;
};

#define TST_SPTR_KDB_NEXT_PARAM(buf, poff, len)		\
	if (!sptr_kdb_next_param(buf, poff, len)) {	\
		sptr_print_help();			\
		return;					\
	}
#endif	/* _KDB */


/*
 * NAME:     sptr_config 
 *
 * FUNCTION: 1.) Configuring a serial line printer or changing
 *               the configuration (CFG_INIT).
 *           2.) Unconfiguring a serial line printer (CFG_TERM).
 *
 * RETURNS: On success 0 is returned.
 *          EINVAL No valid sptr_dds DDS structure is available,
 *                 invalid values in DDS or command unknown.
 *          EBUSY  It is tried, to configure or unconfigure a line, which is
 *		   open (in use).
 *          ENXIO  It is tried, to unconfigure a line, which was not configured.
 *          ENOMEM The call to the he_alloc routine failed.
 *          Error  return value from uiomove, the call to the routine failed.
 *          Error  return value from str_install, the call to the routine failed
 */
int
sptr_config(cmd, uiop)
	int cmd;
	struct uio *uiop;
{
	int    error;
	struct sptr_dds sptrdd_init;
	dev_t  devno;
	struct sptr_config *config_p, *config_prev_p;

	static strconf_t conf = {
		SPTR_NAME, &str_sptr_info, STR_NEW_OPEN,
	};

#ifdef TTYDBG
	int	ttydbg_error = 0;
	static	struct str_module_conf sptr_str_module_conf = {
		SPTR_NAME, 'm', SPTR_PDPF
	};
#endif	/* TTYDBG */

	Enter(TRC_SPTR(TTY_CONFIG), 0, 0, cmd, 0, 0);

	/* valid sptr_dds (DDS) structure is available ? */
	if(uiop == NULL || uiop->uio_resid < sizeof(struct sptr_dds)) {
		Return(EINVAL);
	}

	/* Moving the DDS in to the kernel space. */
	if(error=uiomove(&sptrdd_init, sizeof(struct sptr_dds), UIO_WRITE, uiop)) {
		Return(error);
	}

	/* type of DDS must be SPTR_DDS */
	if(sptrdd_init.which_dds != SPTR_DDS ) {
		Return(EINVAL);
	}

	conf.sc_flags |= (STR_MPSAFE | STR_Q_SAFETY);
	conf.sc_sqlevel = SQLVL_QUEUEPAIR;

	error = 0;

	lockl(&sptr_lock_config, LOCK_SHORT);

	switch (cmd) {
	case CFG_INIT:
		
		/* are there invalid values in the DDS ? */
		if(sptrdd_init.plptimer.v_timout < 1 ||
		   sptrdd_init.plpst.col <= 0 || sptrdd_init.plpst.ind < 0 ||
		   sptrdd_init.plpst.ind >= sptrdd_init.plpst.col ||
		   sptrdd_init.plpst.line < 0) {
			error = EINVAL;
			break;
		}

		if(sptr_config_list == NULL) {
			/* registering the module into the STREAMS Framework */
			if(error = str_install(STR_LOAD_MOD, &conf))
				break;

#ifdef TTYDBG
			/* registering the module into the ttydbg extension */
			ttydbg_error = tty_db_register(&sptr_str_module_conf);
#endif	/* TTYDBG */
		}

		devno = sptrdd_init.devno;

		/* 
		 * the line is already configured (it exists an sptr_config
		 * structure corresponding to the devno)?
		 */
		for(config_p = sptr_config_list; config_p != NULL;
		    config_p = config_p->next) {
			if(config_p->devno == devno)
						/* line is already configured */
				break;		/* config. may be changed now */
		}

		if(config_p == NULL) {		/* line is yet not configured */
			/* allocate an sptr_config struct. for this line now  */
			config_p = (struct sptr_config *)
				    he_alloc(sizeof(struct sptr_config), BPRI_MED);
			if(config_p == NULL) {
				error = ENOMEM; /* allocation failed */
				break;
			}
			else {
				/* the allocated structure is chained */
				/* to the linked list */
				config_p->next = sptr_config_list;
				sptr_config_list = config_p;

				config_p->devno = devno;
				config_p->line_p = NULL;
			}
		}
		else { /* the line is already configured, that means */
		       /* an sptr_config structure corresponding     */
		       /* to the devno exists			     */
			if(config_p->line_p != NULL) { /* is the line open ?*/
				error = EBUSY;         /* yes it is         */
				break;
			}
		}

		/* Putting the modes, v_timout, ind, col and line values */
		/* of the DDS into the sptr_config structure. */
		config_p->modes    = sptrdd_init.plpmod.modes;
		config_p->timout   = sptrdd_init.plptimer.v_timout;
		config_p->ind      = sptrdd_init.plpst.ind;
		config_p->col      = sptrdd_init.plpst.col;
		config_p->line     = sptrdd_init.plpst.line;

		break;
	
	case CFG_TERM:

		/* Determination of the devno of this line. */
		devno = sptrdd_init.devno;
		
		/* Testing, whether a line corresponding to this devno is     */
		/* configured, that means that a sptr_config structure exists)*/
		for(config_p = config_prev_p = sptr_config_list; config_p != NULL;
		    config_p = config_p->next) {
			if(config_p->devno == devno)
				break;
			config_prev_p = config_p;
		}

		/* no sptr_config structure for this devno exists */
		if(config_p == NULL) { 
			error = ENXIO;
			break;
		}

		/* config_p points to an sptr_config structure
		   corresponding to this devno) */
		if(config_p->line_p != NULL) { /* is the line in use/open ? */
			error = EBUSY;         /* yes it is in use/open ?   */
			break;
		}

		/* the line is not open/in use!   */
		/* take it out of the linked list */
		/* anchored in sptr_config_list   */
		if(sptr_config_list == config_p) {
			/* found structure is first element in list */
			/* adjust sptr_config_list pointer */
			sptr_config_list = config_p->next;
		}
		else { /* found structure is not first element in linked list */
			config_prev_p->next = config_p->next;
		}
					/* now freed the sptr_config struct. */
		he_free((caddr_t)config_p);
					
		/* 
		 * If the sptr_config_list is empty (no lines are configured),
		 * the module is unconfigured from the STREAMS Framework
		 * by calling str_install.
		 */
		if(sptr_config_list == NULL) {
			if (error = str_install(STR_UNLOAD_MOD, &conf))
				break;
#ifdef TTYDBG
			/* unregistering the module into the ttydbg extension */
			ttydbg_error = tty_db_unregister(&sptr_str_module_conf);
#endif	/* TTYDBG */
		}

		break;

	default:
		error = EINVAL;
		break;
	}

	unlockl(&sptr_lock_config);

	Return(error);

}

/*
 * NAME:     str_sptr_open 
 *
 * FUNCTION: 1.) When pushing sptr onto a Stream (first open).
 *           2.) When opening a Stream, which has the sptr module already
 *		 pushed (reopen).
 *
 * RETURNS:  Upon successful open.
 *           ENOMEM The call to the he_alloc routine failed.
 *           ENXIO  The serial line printer is not configured (that means that
 *	 	    no sptr_config structure corresponding to "devp" exists),
 *		    So, only configured lines may be opened.
 *
 */

int
str_sptr_open(q, devp, flag, sflag, credp)
	queue_t	*q;
	dev_t	*devp;
	int	flag;
	int	sflag;
	cred_t	*credp;
{
	dev_t devno;
				/* ptr to sptr_config struct. for this devno */
	struct sptr_config *config_p;
				/* pointer to line structure for this devno */
	struct line *line_p;
	struct iocblk *iocp;
	struct stroptions *sop;
	mblk_t *mp, *mp1;
	int error;

#ifdef TTYDBG
	int	ttydbg_error = 0;
	struct	tty_to_reg sptr_tty_to_reg;
#endif	/* TTYDBG */

	Enter(TRC_SPTR(TTY_OPEN), *devp, q->q_ptr, flag, sflag, 0);

	error = 0;
	if (devp == NULL) {
		error = ENXIO;					/* NODEV */
		goto end;
	}
	devno = *devp;

	/* 
	 * Is the line already configured (it exists an sptr_config structure
	 * corresponding to the devno)?
	 */

	lockl(&sptr_lock_config, LOCK_SHORT);

	for(config_p = sptr_config_list; config_p != NULL; config_p = config_p->next) {
		if(config_p->devno == devno) /* line is already configured    */
			break;               
	}

	if(config_p == NULL) {  /* line is yet not configured */
		unlockl(&sptr_lock_config);
		error = ENXIO; /* can not open a line which is not configured */
		goto end;
	}

	/*
	 * First open of a line/stream 
	 */
	if(q->q_ptr == NULL) {

		/*
		 * 1.) allocate a "line" structure for this line now.
		 */
		line_p = (struct line *)he_alloc(sizeof(struct line), BPRI_MED);
		if(line_p == NULL) {
			unlockl(&sptr_lock_config);
			error = ENOMEM;		/* allocation failed */
			goto end;
		}

		bzero(line_p, sizeof(struct line));

#ifdef TTYDBG
		/*
		 * 1bis.) Open the ttydbg extension for the module.
		 */
		sptr_tty_to_reg.dev=devno;
		sptr_tty_to_reg.ttyname[0]='\0';
		bcopy(SPTR_NAME, &sptr_tty_to_reg.name,sizeof(SPTR_NAME));
		sptr_tty_to_reg.private_data=line_p;

		ttydbg_error = tty_db_open(&sptr_tty_to_reg);
#endif	/* TTYDBG */

		/*
		 * 2.) Storing of the pointer to this structure in the line_p
		 * field of the corresponding sptr_config structure and in the
		 * q->q_ptr of the read/write queue.
		 */
		config_p->line_p = line_p;
		unlockl(&sptr_lock_config);

		q->q_ptr = OTHERQ(q)->q_ptr = (char *)line_p;

		/*
		 * 3.) Placing the values of the corresponding sptr_config
		 * structures (modes, timout, ind, col and line) into the
		 * "line" structure.
		 */
		line_p->sp_modes = config_p->modes;
		line_p->v_timout = config_p->timout;
		line_p->ind = config_p->ind;
		line_p->col = config_p->col;
		line_p->line = config_p->line;

		/*
		 * 4.) Setting ihog of "line" to SPTR_IHIWAT.
		 */
		line_p->ihog = SPTR_IHIWAT;

		/*
		 * 5.) Setting "flags", "write_status" and "mp_stored_status"
		 * of "line" to "0".
		 */
		line_p->flags = line_p->write_status = line_p->mp_stored_status = 0;
		
		/*
		 * 6.) Setting "ccc" of "line" to "ind".
		 */
		line_p->ccc = line_p->ind;

		/*
		 * 7.) Setting "mcc" and "mlc" of "line" to "0".
		 */
		line_p->mcc = line_p->mlc = 0;

		/*
		 * 8.) Allocation of a new message block with ihog as size and
		 * storing the received pointer into input_data_mp of "line".
		 * Allocation of an M_DATA message block with "MODBLKSZ" as size
		 * and storing the received pointer into sendff_mp of "line".
		 */
		if((line_p->input_data_mp = allocb(line_p->ihog, BPRI_MED)) == NULL ||
		  (line_p->sendff_mp = allocb(SENDFFBLKSZ, BPRI_MED)) == NULL) {
			/* allocation failed */

			if(line_p->input_data_mp)
				freemsg(line_p->input_data_mp);

			he_free((caddr_t) line_p); 
			config_p->line_p = NULL;
			q->q_ptr = OTHERQ(q)->q_ptr = NULL;
			error = ENXIO;  
			goto end;
		}

		/*
		 * 9.) Sending upstream an M_SETOPTS message with so_flags of
		 * the stroptions structure set to SO_READOPT | SO_MREADON |
		 * SO_ISTTY | SO_NDELON and so_readopt to RMSGN | RPROTNORM.
		 */
		if((mp = allocb(sizeof(struct stroptions), BPRI_MED)) == NULL) {
			freemsg(line_p->input_data_mp);
			freemsg(line_p->sendff_mp);
			he_free((caddr_t) line_p); /* allocation failed */
			config_p->line_p = NULL;
			q->q_ptr = OTHERQ(q)->q_ptr = NULL;
			error = ENXIO;  
			goto end;
		}
		mp->b_datap->db_type = M_SETOPTS;
		sop = (struct stroptions *)mp->b_rptr;
		mp->b_wptr =  mp->b_rptr + sizeof(struct stroptions);
		sop->so_flags = SO_READOPT | SO_MREADON | SO_ISTTY | SO_NDELON ;
		sop->so_readopt = RMSGN | RPROTNORM;
		putnext(q, mp);
	
		/*
		 * 10.) Send a TIOCMGET M_CTL message to the driver to obtain
		 * the actual status of the CTS. If the allocation fails, the
		 * open routine returns with ENXIO.
		 * If the module is not opened in DNDELAY (FNDELAY|FNONBLOCK)
		 * mode, do this.
		 */
		if(!(flag & DNDELAY)) {
		   if((mp = allocb(sizeof(struct iocblk), BPRI_MED)) == NULL ||
		      (mp1 = allocb(sizeof(int), BPRI_MED)) == NULL) {
			if (mp) freemsg(mp);
			freemsg(line_p->input_data_mp);
			freemsg(line_p->sendff_mp);
			he_free((caddr_t) line_p); /* allocation failed */
			config_p->line_p = NULL;
			q->q_ptr = OTHERQ(q)->q_ptr = NULL;
			error = ENXIO;  
			goto end;
		   }
		   mp->b_cont = mp1;
		   mp->b_datap->db_type = M_CTL;
		   iocp = (struct iocblk *)mp->b_rptr;
		   iocp->ioc_cmd = TIOCMGET;
		   iocp->ioc_count = sizeof(int);
		   putnext(WR(q), mp);
		   line_p->mp_stored_status |= SP_IOCTL_TIOCMGET_STORED;
		}
	
		/*
		 * 11.) sp_cts of "line" is set to SP_CTS_ON.
		 */
		line_p->sp_cts = SP_CTS_ON;

		/*
		 * 12.) config_p of "line" is set to the address of the
		 * corresponding sptr_config structure.
		 */
		line_p->config_p = config_p;

		/*
		 * 13.) Setting the open mode bit in "flags" of "line"
		 * (SP_OPEN_READ if open in read mode or SP_OPEN_WRITE if
		 * open in write mode).
		 */
		if(flag & FREAD)
			line_p->flags |= SP_OPEN_READ;
		if(flag & FWRITE)
			line_p->flags |= SP_OPEN_WRITE;

		/* set write/read-side bufcall and timer ids to 0 */
		line_p->wbid = line_p->rbid = line_p->wtid = line_p->rtid = 0;
		line_p->sp_timerid = line_p->sp_ctsid = 0;
		line_p->sp_timerset = FALSE;

		/*
		 * 14.) The routine returns with "0".
		 */
		goto end;

	} /* End: first open of a line/stream: if(q->q_ptr == NULL) */
	
	/*
	 * Reopen of a line/stream 
	*/
	else {
		unlockl(&sptr_lock_config);

		line_p = (struct line *)q->q_ptr;
		
		/* If the module is not opened in DNDELAY (FNDELAY|FNONBLOCK)
		 * mode and if CTS has not been to "on" during  the last 10
		 * seconds (that means sp_cts is "SP_CTS_OFF2"), the open
		 * returns ENXIO.
		 */
		if(!(flag & DNDELAY) && (line_p->sp_cts & SP_CTS_OFF2)) {
			error = ENXIO;
			goto end;
		}

		/* open in write mode */
		if(flag & FWRITE) {
			/* line is alreday opened in write mode ? */
			if(line_p->flags & SP_OPEN_WRITE) { 
				error = ENXIO; /* yes, open fails */
				goto end;      
			}
			else { /* no, first open in write mode */
				line_p->flags |= SP_OPEN_WRITE;
			}
		}

		/* open in read mode */
		if(flag & FREAD) 
			line_p->flags |= SP_OPEN_READ;
		/* The routine returns with "0" */

	} /* Reopen of a line/stream */
		
end:
	Return(error);

}

/*
 * NAME:     str_sptr_close 
 *
 * FUNCTION: This routine is called, when the sptr module is popped from
 *	     a Stream.
 *
 * RETURNS:  always 0
 *
 */

int
str_sptr_close(q, flag, credp)
	register queue_t *q;
	int flag;
	cred_t *credp;
{
	struct line *line_p = (struct line *)q->q_ptr;
	struct sptr_config *config_p = line_p->config_p;

#ifdef TTYDBG
	int	ttydbg_error = 0;
	struct	tty_to_reg sptr_tty_to_reg;
#endif	/* TTYDBG */
	mblk_t *mp;

	Enter(TRC_SPTR(TTY_CLOSE), config_p->devno, line_p, flag, 0, 0);
	/* make sure that all data queued up for the printer goes out */
	/* the door before completing the close.  Otherwise it is lost! */

	if (line_p->write_mdata_mp)
	{
		putnext(OTHERQ(q), line_p->write_mdata_mp);
		line_p->write_mdata_mp = NULL;
	}
	if (OTHERQ(q)->q_first)
	{
		while (mp = getq(OTHERQ(q)))
		{
			if (mp->b_datap->db_type == M_DATA)
				start_write(OTHERQ(q), mp);
			else
				freemsg(mp);
		}
	}

	/* If the last output has not been in PLOT mode (that means that the
	 * SP_IODONE bit is set in "flags" of the "line" structure),
	 * a form feed is sent to the printer (!@#&).
	 */
	if(line_p->line && (line_p->flags & SP_IODONE) && line_p->sp_cts & SP_CTS_ON) {
		if (str_sp_sendff(OTHERQ(q), &line_p->sendff_mp)) {
			/* sendff_mp will be freed by the driver */
			line_p->sendff_mp = NULL;
		} else {
			/* sendff_mp will be freed by sptr below */
			SPTR_PRINTF_ON(printf("str_sptr_close : failed in str_sp_sendff\n"));
		}
	}

	/* If there are still messages stored locally, they are freed.
	 * Perhaps it is better to test the correponding bits 
	 */
	if(line_p->write_mdata_mp)
		freemsg(line_p->write_mdata_mp);
	if(line_p->write_mioctl_mp)
		freemsg(line_p->write_mioctl_mp);
	if(line_p->mioctl_mp)
		freemsg(line_p->mioctl_mp);
	if(line_p->input_data_mp)
		freemsg(line_p->input_data_mp);
	if(line_p->sendff_mp)
		freemsg(line_p->sendff_mp);

	/* If there are still some bufcall ids set */
	if (line_p->wbid)
		unbufcall(line_p->wbid);
	if (line_p->rbid)
		unbufcall(line_p->rbid);

	/* If there are still some timer ids set
	 */
	if (line_p->wtid)
		untimeout(line_p->wtid);
	if (line_p->rtid)
		untimeout(line_p->rtid);

	/* Perhaps it is better to test the correponding bits/variables */
	if (line_p->sp_timerid)
		untimeout(line_p->sp_timerid);
	if (line_p->sp_ctsid)
		untimeout(line_p->sp_ctsid);

	/* Before freeing the "line" structure some values of the "line"
	 * structure are saved in the corresponding sptr_config structure.
	 */
	config_p->modes  = line_p->sp_modes;
	config_p->timout = line_p->v_timout;
	config_p->ind    = line_p->ind;
	config_p->col    = line_p->col;
	config_p->line   = line_p->line;

#ifdef TTYDBG
	/*
	 * Close the ttydbg extension for the module.
	 */
	sptr_tty_to_reg.dev=config_p->devno;
	sptr_tty_to_reg.ttyname[0]='\0';
	bcopy(SPTR_NAME, &sptr_tty_to_reg.name,sizeof(SPTR_NAME));
	sptr_tty_to_reg.private_data=line_p;

	ttydbg_error = tty_db_close(&sptr_tty_to_reg);
#endif	/* TTYDBG */

	/* The "line_p" pointer in the corresponding sptr_config structure
	 * is set to NULL, the "line" structure is freed and the routine
	 * returns with "0".
	 */
	config_p->line_p = NULL;
	he_free(line_p);

	Return(0);
}

/*
 * NAME:     str_sptr_rput
 *
 * FUNCTION: This routine handles the following message types arriving at the
 *           read side: M_DATA, M_FLUSH, M_IOCACK, M_IOCNAK, M_CTL, M_PCPROTO
 *	     and M_IOCACK.
 *
 * RETURNS:  always 0
 *
 */

int
str_sptr_rput(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{
	struct line *line_p = (struct line *)q->q_ptr;
	struct sptr_config *config_p = line_p->config_p;

	Enter(TRC_SPTR(TTY_RPUT), config_p->devno, line_p, mp, mp->b_datap->db_type, 0);
	
	switch (mp->b_datap->db_type) {
	case M_DATA:
		read_mdata(q, mp);
		break;

	case M_FLUSH:
		if(*mp->b_rptr & FLUSHR)
			/* reset the b_wptr of stored input data to b_rptr */
			line_p->input_data_mp->b_wptr = line_p->input_data_mp->b_rptr;
		putnext(q, mp);
		break;

	case M_IOCACK:
		/* Process the message immediately.
		 */
		if(!read_miocack(q, mp))
			putq(q, mp);
		
		break;

	case M_IOCNAK:
		/* Process the message immediately.
		 */
		if(!read_miocnak(q, mp))
			putq(q, mp);
		
		break;

	case M_CTL:
		/* If a message is already queued or flow control does not
		 * allow passing messages upstream or message could not be
		 * processed, mp is queued.
		 */
		if(q->q_first || !canput(q->q_next) || !sptr_mctl(q, mp))
			putq(q, mp);
		
		break;

	case M_PCPROTO:
		read_mpcproto(q, mp);
		break;

		/* instead of M_IOCACK messages there are M_CTL messages
		 * coming(TIOCSETA, TIOCGETA) -> are treated in sptr_mctl()
		 */
		
	default:
		/* If high priority message or flow control allows it,
		   -> putnext(q, mp) */
		if((mp->b_datap->db_type >= QPCTL) || (canput(q->q_next)))
			putnext(q, mp); 
		else
			putq(q, mp); /* otherwise queueing of this message */

		break;
	}

	Return(0);
}

/*
 * NAME:     str_sptr_rsrv
 *
 * FUNCTION: This routine handles the M_IOCACK, M_IOCNAK and M_CTL message
 *	     types queued at the read side.
 *
 * RETURNS:  always 0
 *
 */

int
str_sptr_rsrv(q)
	queue_t	*q;
{
	mblk_t	*mp;

	struct line *line_p = (struct line *)q->q_ptr;
	struct sptr_config *config_p = line_p->config_p;

	Enter(TRC_SPTR(TTY_RSRV), config_p->devno, line_p, q->q_count, 0, 0);

	while(mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_CTL: 
			/* If flow control does not allow passing messages
			 * upstream, or message could not be processed,
			 * requeue the message.
			 */
			if(!canput(q->q_next) || !sptr_mctl(q, mp)) {
				putbq(q, mp);
				Return(0);
			}
			break;

		case M_IOCACK:
			/* Process the message.
			 */
			if(!read_miocack(q, mp)) {
				putbq(q, mp);
				Return(0);
			}

			break;

		case M_IOCNAK:
			/* Process the message.
			 */
			if(!read_miocnak(q, mp)) {
				putbq(q, mp);
				Return(0);
			}

			break;

		default:
			/* If flow control does not allow passing messages 
			 * upstream, requeue the message */
			if(!canput(q->q_next)) { 
				putbq(q, mp);
				Return(0);
			} else {
				putnext(q, mp); /* forward message */
			}
			break;
		}
        }

	Return(0);
}

/*
 * NAME:     str_sptr_wput
 *
 * FUNCTION: This routine handles the following message types arriving at the
 *           write side: M_DATA, M_READ, M_FLUSH, and M_IOCTL.
 *
 * RETURNS:  always 0
 *
 */

int
str_sptr_wput(q, mp)
	queue_t	*q;
	mblk_t *mp;
{
	mblk_t	*nmp;

	struct line *line_p = (struct line *)q->q_ptr;
	struct sptr_config *config_p = line_p->config_p;

	Enter(TRC_SPTR(TTY_WPUT), config_p->devno, line_p, mp, mp->b_datap->db_type, 0);
	
	nmp = NULL;
	
	switch (mp->b_datap->db_type) {
	case M_DATA:
		if ((line_p->flags & SP_DUMMYSEND) && !(line_p->sp_modes & PLOT)) {
			format_data(q, mp, &nmp);
			line_p->write_status |= SP_WRITE_FINISHED;
			break;
		}
		/* If a message is already queued or flow control does not
		 * allow passing messages downstream or the message
		 * could not be processed, mp is queued and the
		 * SP_WRITE_DATA_QUEUED bit is set.
		 */
		 /* if(q->q_first || !canput(q->q_next) || !(line_p->sp_cts & SP_CTS_ON)) { */
		 if(q->q_first || !canput(q->q_next)) {
			 putq(q, mp);
			 line_p->write_status |= SP_WRITE_DATA_QUEUED;
			 break;
		 }
		 if(!start_write(q, mp)) 
			 putq(q, mp);
		 break;

	case M_READ: /* process the M_READ message */
		 if(!write_mread(q, mp)) 
			 putq(q, mp);

		 break;

	case M_FLUSH: /* process the M_FLUSH message */
		write_mflush(q, mp); 
		break;

	case M_IOCTL:
	{
		struct iocblk *iocp = (struct iocblk *)mp->b_rptr;

		/* If a message is already queued or flow control
		 * does not allow passing messages downstream or
		 * the message could not be processed, mp is queued.
		 */
		switch (iocp->ioc_cmd) {
		case IOCINFO:
		case LPRGET:
		case LPRSET:
		case LPRMODG:
		case LPRMODS:
		case LPRGTOV:
		case LPRSTOV:
		case LPQUERY:
			if(q->q_first || !sptr_ioctl(q,mp))
				putq(q, mp);
			break;
		default:
			if(q->q_first || !canput(q->q_next) ||
			   !sptr_ioctl(q, mp))
				putq(q, mp);
			break;
		}
		break;
	}

	default:
		/* If high priority message or flow control allows it,
		 * -> putnext(q, mp)
		 */
		if((mp->b_datap->db_type >= QPCTL) || (canput(q->q_next)))
			putnext(q, mp); 
		else
			putq(q, mp); /* otherwise queueing of this message */

		break;
        }

	Return(0);


}

/*
 * NAME:     str_sptr_wsrv
 *
 * FUNCTION: This routine handles the following message types queued at the
 *           write side: M_DATA, M_IOCTL and M_READ.
 *
 * RETURNS:  always 0
 *
 */

int
str_sptr_wsrv(q)
	queue_t	*q;
{
	mblk_t	*mp;

	struct line *line_p = (struct line *)q->q_ptr;
	struct sptr_config *config_p = line_p->config_p;

	Enter(TRC_SPTR(TTY_WSRV), config_p->devno, line_p, q->q_count, 0, 0);

	while(mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			/* If flow control doesn't allow passing messages
			 * downstream or message could not be processed,
			 * requeue the message.
			 */
			/* if(!canput(q->q_next) || !(line_p->sp_cts & SP_CTS_ON)) { */
			if(!canput(q->q_next)) {
				putbq(q, mp);
				Return(0);
			}
		 	if(!start_write(q, mp)) {
				putbq(q, mp);
				Return(0);
			}
			line_p->write_status &= ~SP_WRITE_DATA_QUEUED;
			break;

		case M_IOCTL:
		{
			struct iocblk *iocp = (struct iocblk *)mp->b_rptr;

			/* If flow control doesn't allow passing messages
			 * downstream or message could not be processed,
			 * requeue the message.
			 */
			switch (iocp->ioc_cmd) {
			case IOCINFO:
			case LPRGET:
			case LPRSET:
			case LPRMODG:
			case LPRMODS:
			case LPRGTOV:
			case LPRSTOV:
			case LPQUERY:
				if(!sptr_ioctl(q,mp)) {
					putbq(q, mp);
					Return(0);
				}
				break;
			default:
				if(!canput(q->q_next) || !sptr_ioctl(q, mp)) {
					putbq(q, mp);
					Return(0);
				}
				break;
			}
			break;
		}

		case M_READ:
			/* If message could not be processed, requeue it */
			if(!write_mread(q, mp)) {
				putbq(q, mp);
				Return(0);
			}
			break;

		default:
			/* If flow control does not allow passing messages
			 * downstream, requeue the message
			 */
			if(!canput(q->q_next)) { 
				putbq(q, mp);
				Return(0);
			} else {
				putnext(q, mp); 
			}
			break;
		}
	}
 
	Return(0);
}

/*
 * NAME:     start_write
 *
 * FUNCTION: Allocate a M_PROTO message, call the format_data routine if
 *	     the data have to be formatted (no PLOT mode), link the
 *	     [un]/formatted message to the M_PROTO and send this message
 *	     upstream. Moreover, set the timeout if it is required.
 *
 * RETURNS:  1 on success
 *           0 if mp has to be (re)queued
 *
 */

static int
start_write(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{
	mblk_t	*nmp;		/* pointer to message with formatted data */
	mblk_t	*mproto_mp;	/* pointer to an M_PROTO message block */
	struct line *line_p;

	nmp = NULL; 
	line_p = (struct line *)q->q_ptr;

	/* allocate an M_PROTO message with sizeof(int) as size to put the
	 * constant LPWRITE_REQ into it.
	 */

	mproto_mp = allocb(sizeof(int), BPRI_MED);
	if(!mproto_mp) { /* allocation failed */
			
		allocb_recovery(q, &line_p->wbid,  &line_p->wtid, sizeof(int));
		return(0);	/* mp will be put [back] to the queue in
				   str_sptr_wput() or str_sptr_wsrv() */
	}

	else { /*  allocation of the M_PROTO message succeeded */
		mproto_mp->b_datap->db_type = M_PROTO;	/* set the right type */
		*(int *)mproto_mp->b_rptr = LPWRITE_REQ;
		mproto_mp->b_wptr = mproto_mp->b_rptr + sizeof(int);
	}


	/* if PLOT mode */
	if(line_p->sp_modes & PLOT) {

		/* switched off the SP_IODONE bit */
		line_p->flags &= ~SP_IODONE;
		linkb(mproto_mp, mp);
	}

	else { /* no PLOT mode -> data have to be formatted */
		if(!format_data(q, mp, &nmp)) {

			/* free allocated M_PROTO message */
			freemsg(mproto_mp); 
			return(0); /* If formatting failed because of an
				    * allocb() failure, mp will be (re)queued
				    * in str_sptr_wput or str_sptr_wsrv.
				    */
		}

		line_p->flags |= SP_IODONE;
		linkb(mproto_mp, nmp);
	}

	/* the SP_WRITE_IN_PROCESS bit is set and the M_PROTO message with
	 * the data (formatted or unformatted) is sent downstream.
	 */
	line_p->write_status |= SP_WRITE_IN_PROCESS;
	putnext(q, mproto_mp);

	/* if error report is set and there is a valid write timeout value */
	if(line_p->sp_modes & RPTERR && line_p->v_timout &&
							!line_p->sp_timerset) {

		/* start the write timer and store the returned timer id */
		/* (*HZ) : with pse_timeout the unit value is a tick     */
		line_p->sp_timerid = timeout(write_timeout, q,
					     line_p->v_timout*HZ);

		/* Set the sp_timerset field to TRUE and sp_timeout to FALSE. */
		line_p->sp_timerset = TRUE;
		line_p->sp_timeout = FALSE;
	}
	return(1);
}

/*
 * NAME:	format_data
 *
 * FUNCTION:	Calls sptr_prnformat routine for all characters of all
 *		message blocks.
 *
 * RETURNS:	1 on success
 *		0 if mp has to be (re)queued
 *
 */

static int
format_data(q, mp, nmpp)
	queue_t	*q;
	mblk_t	*mp;
	mblk_t	**nmpp;
{
	register mblk_t *bp;

	/* Looking for all message blocks in the original message */
	for(bp = mp; bp != NULL; bp = bp->b_cont) {

		/* Looking for all characters in a message block */
		while(bp->b_rptr < bp->b_wptr) {

			/* Try to mapp one character of the original message */
			if(!sptr_prnformat(q, nmpp, *bp->b_rptr)) {
			/* formatting failed because of an allocb failure */
				if (*nmpp != 0) {
					freemsg(*nmpp);
					*nmpp = 0;
				}
				return(0);
			}

			bp->b_rptr++; /* next char of the original message */
		} /* while */
	} /* for */

	freemsg(mp);
	return(1);
}

/*
 * NAME:	sptr_prnformat
 *
 * FUNCTION:	Format the data to be sent to the printer.
 *
 * RETURNS:	1 on success
 *		0 if mp has to be (re)queued
 *
 */

static int
sptr_prnformat(q, nmpp, c)
	queue_t	*q;
	mblk_t	**nmpp;
	int c;
{
	struct line *line_p;
	register char *mark = "\n...";

	line_p = (struct line *)q->q_ptr;

	if (line_p->sp_modes&CAPS) {       
		if (c >= 'a' && c <= 'z')
			c += 'A'-'a';
		else {       
			int oldc = c;
			switch (c) {       
			case '{': c = '('; break;
			case '}': c = ')'; break;
			case '`': c = '\''; break;
			case '|': c = '!'; break;
			case '~': c = '^'; break;
			}
			/*
			 * if we changed the current character,
			 * then print the new character followed
			 * by a dash (-) to denote that a character
			 * was remapped
			*/
			if (oldc != c) {
				SPTR_PRNFORMAT(q, nmpp, c);
				line_p->ccc--;
				c = '-';
			}
		}
	}

	switch (c) {       
	case '\r':
		if (line_p->sp_modes&NOCR)
			c = '\n';
		else {      
			line_p->mcc = 0;
			line_p->ccc = line_p->ind;
			ADDC(nmpp, line_p, c);
			return(1);			/* Returnv; */
		}
	case '\n':
		if (line_p->sp_modes&NONL)
			c = '\r';
		if (++line_p->mlc >= line_p->line) {       
			if (line_p->line && ((line_p->sp_modes&NOFF) == 0))
				c = FF;
			line_p->mlc = 0;
		}
		line_p->mcc = 0;
		line_p->ccc = line_p->ind;
		if (!( line_p->sp_modes & NOCL)) {
			ADDC(nmpp, line_p,(int) '\r');
		}
		ADDC(nmpp, line_p, c);
		return(1);				/* Returnv; */
	case FF:
		if (line_p->mlc == 0 && line_p->mcc == 0) {
			line_p->ccc = line_p->ind;
			return(1);			/* Returnv; */
		}
		if (line_p->sp_modes&NOFF) {       
			SPTR_PRNFORMAT(q, nmpp,(int)'\n');
			while (line_p->mlc)
				SPTR_PRNFORMAT(q, nmpp,(int)'\n');
		return(1);				/* Returnv; */
		}
		line_p->mlc = 0;
		line_p->mcc = 0;
		line_p->ccc = line_p->ind;
		ADDC(nmpp, line_p, c);
		return(1);				/* Returnv; */
	case '\t':
		while (line_p->ccc > line_p->mcc) { /* Bring mcc up to tab */
			ADDC(nmpp, line_p,(int)' ');
			line_p->mcc++;
		}
		line_p->ccc += (8 - (line_p->ccc  & 7));/* add tab to ccc */
		if (line_p->sp_modes&NOTB) {		/* IF NOTB set */
			line_p->mcc = line_p->ccc ;	/* update mcc to ccc */
			ADDC(nmpp, line_p, c);		/* send tab out */
		}
		return(1);				/* Returnv; */
	case ' ':
		line_p->ccc++;
		return(1);				/* Returnv; */
	case '\b':
		if (line_p->sp_modes&NOBS) {       
			if (line_p->ccc > line_p->ind)
				line_p->ccc--;
			return(1);			/* Returnv; */
		}
	} /*switch */

	if (line_p->ccc < line_p->mcc) {       
		if (line_p->sp_modes&NOCR) {       
			ADDC(nmpp, line_p,(int)'\n');
			++line_p->mlc;
		} else
			ADDC(nmpp, line_p,(int)'\r');
		line_p->mcc = 0;
	}
	if (line_p->ccc < line_p->col) {       
		while (line_p->ccc > line_p->mcc) {       
			ADDC(nmpp, line_p,(int)' ');
			line_p->mcc++;
		}
		ADDC(nmpp, line_p, c);
		line_p->mcc++;
	} else {       
		if (line_p->sp_modes & WRAP) {       
			while (*mark)
				SPTR_PRNFORMAT(q, nmpp, (int)*mark++);
			ADDC(nmpp, line_p, c);
			line_p->mcc++;
		}
	}
	line_p->ccc++;
	return(1);

}

/*
 * NAME:	addc
 *
 * FUNCTION:	add a mapped character in the formatted message
 *		  . initialize the pointer, nmpp, to the first message block
 *		    of the linked list.
 *		  . allocate a new message block if the current is full.
 *
 * RETURNS:	1 on success
 *		0 allocb() failure -> if mp has to be (re)queued
 *
 */

static int
addc(nmpp, line_p, c)
	mblk_t	**nmpp;
	struct line *line_p;
	int c;
{
	mblk_t	*last_nmp;

	if ( line_p->flags & SP_DUMMYSEND ) {
		line_p->chars_sent++;
		return(1);
	}

	if (*nmpp == NULL) {
		*nmpp = allocb(MODBLKSZ, BPRI_MED);
		if (*nmpp == 0) return(0); /* allocb failure on the 1st mess */
	}

	/* search the last message block */
	for (last_nmp = *nmpp; last_nmp->b_cont; last_nmp = last_nmp->b_cont);

	/* is the last message block full ? */
	if (last_nmp->b_wptr == last_nmp->b_datap->db_lim) {
		last_nmp->b_cont = allocb(MODBLKSZ, BPRI_MED);
		if (last_nmp->b_cont == NULL) {	/* allocb failure */
			return(0);
		}
		last_nmp = last_nmp->b_cont ;
	}

	*last_nmp->b_wptr++ = c;

	return(1);
}

/*
 * NAME:     write_mread  
 *
 * FUNCTION: This routine handles the M_READ messages at the write side of the
 *	     module which contain the number of characters to be read.
 *
 * RETURNS:  1 on success
 *           0 if mp has to be (re)queued
 *
 */

static int
write_mread(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{
	mblk_t *i_mp, *bp;
	unsigned num_wanted, num_stored, num_rest_stored;
	struct line *line_p;

	line_p = (struct line *)q->q_ptr;

	/* "i_mp" is set to input_data_mp of "line" and "num_wanted"
	 * to the value of the "wanted" characters stored in the data part
	 * of the M_READ message.
	 */
	i_mp = line_p->input_data_mp;
	num_wanted = *(unsigned *)mp->b_rptr;

	/* "num_stored" (the number of read characters, which are stored
	 * in the message pointed to by input_data_mp) is calculated.
	 */
	num_stored = (unsigned) (i_mp->b_wptr - i_mp->b_rptr);

	if(num_wanted >= num_stored) {

		/* allocate a message with line_p->ihog as size to put
		 * the data, coming from the driver, in it.
		 */
		bp = allocb(line_p->ihog, BPRI_MED);
		if(!bp) { /* allocation failed */
			
			allocb_recovery(q, &line_p->wbid,  &line_p->wtid, line_p->ihog);
			return(0); /* mp will be put [back] to the queue in
				      str_sptr_rput() or str_sptr_rsrv() */
		}
		else{ /*  allocation succeeded */

			/* The message pointed to by i_mp is sent upstream as
			 * a response to the M_READ message.*/
			qreply(q, i_mp);

			/* put the pointer to the allocated message into 
			 * line_p->input_data_mp for characters coming
			 * from the driver.
			 */
			line_p->input_data_mp = bp;
		}
	}

	else { /* num_wanted is less than num_stored */
		/* allocate a message with num_wanted as size to put the
		 * received input data in it.
		 */
		bp = allocb(num_wanted, BPRI_MED);
		if(!bp) { /* allocation failed */

			allocb_recovery(q, &line_p->wbid,  &line_p->wtid, num_wanted); 
			return(0); /* mp will be put [back] to the queue in
				      str_sptr_rput() or str_sptr_rsrv() */
		}
		else{ /*  allocation succeeded */

			/* the amount of "wanted" characters is copied from the
			 * message pointed to by input_data_mp to bp and
			 *  bp->b_wptr is incremented by num_wanted.
			 */
			bcopy(i_mp->b_rptr, bp->b_rptr, num_wanted);
			bp->b_wptr +=  num_wanted;

			/* "num_rest_stored" (the number of stored characters
			 * of the input_data_mp message, which have not been
			 * taken out) is calculated (num_stored - num_wanted).
			 */
			num_rest_stored = num_stored - num_wanted;

			/* The num_rest_stored characters are copied to the
			 * beginning of the data part of the input_data_mp
			 * message and its b_wptr pointer is updated
			 * (b_rptr + num_rest_stored).
			 */
			bcopy(i_mp->b_rptr + num_wanted, i_mp->b_rptr, num_rest_stored);
			i_mp->b_wptr = i_mp->b_rptr + num_rest_stored;

			/* send the message with data upstream */
			qreply(q, bp);

		}
	} /* else {  num_wanted is less than num_stored */

	freemsg(mp);
	return(1);
}


/*
 * NAME:     read_mdata  
 *
 * FUNCTION: This routine handles the M_DATA messages at the read side of the
 *	     module which contain data coming from a connected printer.
 *	     The data are stored locally with a pointer to them in
 *	     input_data_mp of the actual line structure.
 *
 * RETURNS:  no return value
 *
 */

static void
read_mdata(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{
	mblk_t *i_mp, *orig_mp;
	long space_free, num,read_data;
	struct line *line_p;

	line_p = (struct line *)q->q_ptr;

	/* "i_mp" is set to input_data_mp of "line" and "space_free" is set
	 * to the free space in i_mp (db_lim - b_wptr).
	 */
	i_mp = line_p->input_data_mp;
	space_free = i_mp->b_datap->db_lim - i_mp->b_wptr;

	/* If space_free is equal to 0, the original M_DATA message pointed to
	 * by mp is discarded and the routine returns.
	 */
	if(space_free == 0) {
		freemsg(mp);
		return;
	}

	orig_mp = mp;			       /* pointer to original message */

	/* "read_data" is set to the number of data bytes in the mp message */
	read_data = msgdsize(mp);

	if(read_data <= space_free) {

		/* add all read data to the end of input_data_mp */
		while(mp) {
			num = mp->b_wptr - mp->b_rptr;
			bcopy(mp->b_rptr, i_mp->b_wptr, num);
			i_mp->b_wptr += num;
			mp = mp->b_cont;
		}
	}

	else { /* read data > space free */
		while(mp) {
			num = mp->b_wptr - mp->b_rptr;
			if(num <= space_free) {
				bcopy(mp->b_rptr, i_mp->b_wptr, num);
				i_mp->b_wptr += num;
				space_free -= num;
				mp = mp->b_cont;
				continue;
			}
			else { /* num > space free */
				bcopy(mp->b_rptr, i_mp->b_wptr, space_free);
				i_mp->b_wptr += space_free;
				/* leave while(mp) => input_data_mp is full */
				break;
			}
		}
	}

	freemsg(orig_mp);
}

/*
 * NAME:     write_timeout
 *
 * FUNCTION: This routine is called, when the "write timer" has expired (see
 *	     line "line_p->sp_timerid = timeout(...) in start_write routine).
 *	     If there is a pending LPWRITE_REQ ioctl message waiting for the
 *	     completion of the write (SP_IOCTL_WREQ_STORED) this message,
 *	     stored in the write_mioctl_m pointer (by the write_req_ioctl
 *	     routine), is put on the queue.
 *
 * RETURNS:  no return value 
 *
 */

static void
write_timeout(q)
	queue_t	*q;
{
	struct line *line_p;

	line_p = (struct line *)q->q_ptr;
	
	line_p->sp_timerset = FALSE;
	line_p->sp_timeout = TRUE;
	line_p->sp_timerid = 0;

	line_p->write_status &= ~SP_WRITE_IN_PROCESS;

	if(line_p->flags & SP_IODONE)
		line_p->flags &= ~SP_IODONE;

	line_p->write_status |= SP_WRITE_TIMEDOUT;

	if(line_p->mp_stored_status & SP_IOCTL_WREQ_STORED) {
		line_p->mp_stored_status &= ~SP_IOCTL_WREQ_STORED;
		write_req_ioctl(q,line_p->write_mioctl_mp);
		line_p->write_mioctl_mp = NULL;
	}
	else {
		mblk_t *mp;

		if (mp = getq(q)) {
		   if (mp->b_datap->db_type == M_IOCTL) {
		   	struct iocblk	*iocp = (struct iocblk *)mp->b_rptr;

		   	if (iocp->ioc_cmd == LPWRITE_REQ) {
				write_req_ioctl(q,mp);
			}
			else {
				putbq(q,mp);
		   		qenable(q);	/* Force schedule the write serv routine */
			}
		   }
		   else {
			putbq(q,mp);
		   	qenable(q);	/* Force schedule the write serv routine */
		   }
		}
	}

	return;

}

/*
 * NAME:     read_miocack
 *
 * FUNCTION: This routine handles the TIOCGETA and TIOCSETA M_IOCACK messages.
 *
 * RETURNS:  always 1
 *
 */

static int
read_miocack(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{

	struct iocblk *iocp;
	struct line *line_p;

	line_p = (struct line *)q->q_ptr;
	iocp = (struct iocblk *)mp->b_rptr;

	SPTR_PRINTF_ON(printf("read_miocack : iocp->ioc_cmd == %x \n",iocp->ioc_cmd));

	switch(iocp->ioc_cmd) {
	case TIOCGETA: {
		/* if an LPRGETA M_IOCTL message is locally stored */
		if( line_p->mp_stored_status & SP_IOCTL_LPRGETA_STORED ) {
			struct termios *termios_p;
			struct lpr232  *lpr232_p;

			/* termios structure in mp message  */
			termios_p = (struct termios *)mp->b_cont->b_rptr;

			/* lpr232 structure in LPRGETA M_IOCTL message
			 * locally stored
			 */
			lpr232_p  = (struct lpr232 *)line_p->mioctl_mp->b_cont->b_rptr;

			/* setting the c_cflag value */
			lpr232_p->c_cflag = termios_p->c_cflag;
			
			/* change message type of stored LPRGETA M_IOCTL
			 * message to M_IOCACK.
			 */
			line_p->mioctl_mp->b_datap->db_type = M_IOCACK;

			/* send original LPRGETA M_IOCTL message upstream */
			putnext(q, line_p->mioctl_mp);

			/* switch off the SP_IOCTL_LPRGETA_STORED bit */
			line_p->mp_stored_status &= ~SP_IOCTL_LPRGETA_STORED;
			line_p->mioctl_mp = NULL;

			/* mp can be freed now */
			freemsg(mp);
			
			return(1);
			
		}

		/* if an TCGETA M_IOCTL message is locally stored */
		if( line_p->mp_stored_status & SP_IOCTL_TCGETA_STORED ) {
			struct termios *termios_p;
			struct termio *termio_p;

			/* termios structure in mp message  */
			termios_p = (struct termios *)mp->b_cont->b_rptr;

			/* termio structure in TCGETA M_IOCTL message
			 * locally stored
			 */
			termio_p  = (struct termio *)line_p->mioctl_mp->b_cont->b_rptr;

			/* setting the corresponding values */
			termio_p->c_iflag = termios_p->c_iflag;
			termio_p->c_oflag = termios_p->c_oflag;
			termio_p->c_cflag = termios_p->c_cflag;
			termio_p->c_lflag = termios_p->c_lflag;
			termio_p->c_line = 0;
			bcopy(termios_p->c_cc,termio_p->c_cc,NCC<NCCS?NCC:NCCS);
			
			/* change message type of stored TCGETA M_IOCTL
			 * message to M_IOCACK.
			 */
			line_p->mioctl_mp->b_datap->db_type = M_IOCACK;

			/* send original TCGETA M_IOCTL message upstream */
			putnext(q, line_p->mioctl_mp);

			/* switch off the SP_IOCTL_TCGETA_STORED bit */
			line_p->mp_stored_status &= ~SP_IOCTL_TCGETA_STORED;
			line_p->mioctl_mp = NULL;

			/* mp can be freed now */
			freemsg(mp);
			
			return(1);
			
		}

		/* we are not waiting for such a message:
		 * the SP_IOCTL_LPRGETA_STORED or SP_IOCTL_TCGETA_STORED
		 * is not set.
		 */
		
		break;
	}

	case TIOCSETAW:
	case TIOCSETAF:
	case TIOCSETA: {
		/* if an LPRSETA M_IOCTL message is locally stored */
		if( line_p->mp_stored_status & SP_IOCTL_LPRSETA_STORED ) {
			struct iocblk *iocp_mioctl;
			
			/* change message type of stored LPRSETA M_IOCTL
			 * message to M_IOCACK.
			 */
			line_p->mioctl_mp->b_datap->db_type = M_IOCACK;

			/* erase ioc_error */
			iocp_mioctl = (struct iocblk *)line_p->mioctl_mp->b_rptr;
			iocp_mioctl->ioc_error = 0;

			/* send original LPRSETA M_IOCTL message upstream
			 * to acknowledge it.
			 */
			putnext(q, line_p->mioctl_mp);

			/* switch off the SP_IOCTL_LPRSETA_STORED bit */
			line_p->mp_stored_status &= ~SP_IOCTL_LPRSETA_STORED;
			line_p->mioctl_mp = NULL;

			/* mp can be freed now */
			freemsg(mp);
			
			return(1);
		}

		/* if an TCSETA M_IOCTL message is locally stored */
		if( line_p->mp_stored_status & SP_IOCTL_TCSETA_STORED ) {
			struct iocblk *iocp_mioctl;
			
			/* change message type of stored TCSETA M_IOCTL
			 * message to M_IOCACK.
			 */
			line_p->mioctl_mp->b_datap->db_type = M_IOCACK;

			/* erase ioc_error */
			iocp_mioctl = (struct iocblk *)line_p->mioctl_mp->b_rptr;
			iocp_mioctl->ioc_error = 0;

			/* send original TCSETA M_IOCTL message upstream
			 * to acknowledge it.
			 */
			putnext(q, line_p->mioctl_mp);

			/* switch off the SP_IOCTL_TCSETA_STORED bit */
			line_p->mp_stored_status &= ~SP_IOCTL_TCSETA_STORED;
			line_p->mioctl_mp = NULL;

			/* mp can be freed now */
			freemsg(mp);
			
			return(1);
		}

		/* we are not waiting for such a message:
		 * the SP_IOCTL_LPRSETA_STORED or SP_IOCTL_TCSETA_STORED
		 * is not set.
		 */
		
		break;
	}

	case TIOCMGET: {
		if(*(int *)mp->b_cont->b_rptr & TIOCM_CTS)
			cts_handle(cts_on, q);
		else
			cts_handle(cts_off, q);
		/*
		 * A cts_on or cts_off does not provoke any kind of signal
		 * at the Stream head -> we do not want a putnext now
		 * if we got a reply for our previously issued TIOCMGET
		 * M_CTL message.
		 */

		break;
	}

		
	default:
		break;
	}

	putnext(q, mp);
	return(1);
}

/*
 * NAME:     read_miocnak
 *
 * FUNCTION: This routine handles the TIOCGETA and TIOCSETA M_IOCNAK messages.
 *
 * RETURNS:  always 1
 *
 */

static int
read_miocnak(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{

	struct iocblk *iocp;
	struct line *line_p;

	line_p = (struct line *)q->q_ptr;
	iocp = (struct iocblk *)mp->b_rptr;

	SPTR_PRINTF_ON(printf("read_miocnak : iocp->ioc_cmd == %x \n",iocp->ioc_cmd));

	switch(iocp->ioc_cmd) {
	case TIOCGETA:
	case TIOCSETAW:
	case TIOCSETAF:
	case TIOCSETA: {

		/* if an LPRGETA/LPRSETA/TCGETA/TCSETA M_IOCTL message is
		 * locally stored */
		if( (line_p->mp_stored_status & SP_IOCTL_LPRGETA_STORED ) ||
			(line_p->mp_stored_status & SP_IOCTL_LPRSETA_STORED) ||
			(line_p->mp_stored_status & SP_IOCTL_TCGETA_STORED) ||
			(line_p->mp_stored_status & SP_IOCTL_TCSETA_STORED) ) {
			struct iocblk *iocp_mioctl;

			/* change message type of stored LPRGETA/LPRSETA/
			 * TCGETA/TCSETA M_IOCTL message to M_IOCACK */
			line_p->mioctl_mp->b_datap->db_type = M_IOCNAK;

			/* report driver ioc_error */
			iocp_mioctl = (struct iocblk *)line_p->mioctl_mp->b_rptr;
			iocp_mioctl->ioc_error = iocp->ioc_error;

			/* send original LPRGETA/TC[GS]ETA M_IOCTL message
			 * upstream */
			putnext(q, line_p->mioctl_mp);

			/* switch off the SP_IOCTL_LPRGETA_STORED and
			   the SP_IOCTL_LPRSETA_STORED and 
			   the SP_IOCTL_TCGETA_STORED and 
			   the SP_IOCTL_TCSETA_STORED bits */
			line_p->mp_stored_status &= ~SP_IOCTL_LPRGETA_STORED;
			line_p->mp_stored_status &= ~SP_IOCTL_LPRSETA_STORED;
			line_p->mp_stored_status &= ~SP_IOCTL_TCGETA_STORED;
			line_p->mp_stored_status &= ~SP_IOCTL_TCSETA_STORED;
			line_p->mioctl_mp = NULL;

			/* mp can be freed now */
			freemsg(mp);
			
			return(1);
			
		}
		

		/* we are not waiting for such a message: neither the
		 * SP_IOCTL_LPRGETA_STORED nor the SP_IOCTL_LPRSETA_STORED
		 * nor SP_IOCTL_TCGETA_STORED nor the SP_IOCTL_TCSETA_STORED
		 * is set.
		 */
		
		break;
	}
		
	default:
		break;
	}

	putnext(q, mp);
	return(1);
}

/*
 * NAME:     sptr_mctl
 *
 * FUNCTION: This routine handles the TIOC_REPLY, cts_on, cts_off, TIOCMGET
 *	     and TIOCGETA M_CTL messages.
 *
 * RETURNS:  1 on success
 *           0 if mp has to be (re)queued
 *
 */

static int
sptr_mctl(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{

	struct iocblk *iocp;
	struct line *line_p;
	enum status *st;

	line_p = (struct line *)q->q_ptr;
	if (mp->b_wptr - mp->b_rptr < sizeof(struct iocblk)) {
		st = (enum status *)mp->b_rptr;

		SPTR_PRINTF_ON(printf("sptr_mctl : status == %x \n",(int)*st));

		switch (*st) {
		case cts_on:
		case cts_off:
			cts_handle(*st, q); 
			/*
		 	* A cts_on or cts_off does not provoke any kind of
			* signal at the Stream head -> we do not want a
			* putnext now
		 	*/
			freemsg(mp);
			return(1);

		default:
			break;
		}
	}	/* end -- mp->b_wptr - mp->b_rptr < sizeof(struct iocblk) */
	else {
	iocp = (struct iocblk *)mp->b_rptr;

	SPTR_PRINTF_ON(printf("sptr_mctl : iocp->ioc_cmd == %x \n",iocp->ioc_cmd));

	switch(iocp->ioc_cmd) {
	case TIOC_REPLY:
		if (!tioc_handle(q, mp))
			return(0);
		break;

	case cts_on:
	case cts_off:
		cts_handle(iocp->ioc_cmd, q); 
		/*
		 * A cts_on or cts_off does not provoke any kind of signal
		 * at the Stream head -> we do not want a putnext now
		 */
		freemsg(mp);
		return(1);

		break;

	case TIOCMGET:
		if(*(int *)mp->b_cont->b_rptr & TIOCM_CTS)
			cts_handle(cts_on, q);
		else
			cts_handle(cts_off, q);
		/*
		 * A cts_on or cts_off does not provoke any kind of signal
		 * at the Stream head -> we do not want a putnext now
		 */
		if( line_p->mp_stored_status & SP_IOCTL_TIOCMGET_STORED ) {
			line_p->mp_stored_status &= ~SP_IOCTL_TIOCMGET_STORED;
			freemsg(mp);
			return(1);
		}

		break;

	case TIOCGETA: {
		/* if an LPRSETA M_IOCTL message is locally stored */
		if( line_p->mp_stored_status & SP_IOCTL_LPRSETA_STORED ) {
			struct termios *termios_p;
			struct lpr232  *lpr232_p;

			/* termios structure in mp message  */
			termios_p = (struct termios *)mp->b_cont->b_rptr;

			/* lpr232 structure in LPRGETA M_IOCTL message
			 * locally stored.
			 */
			lpr232_p  = (struct lpr232 *)line_p->mioctl_mp->b_cont->b_rptr;

			
			/* set the c_cflag value in the termios structure
			 * to the c_cflag value of the LPRSETA M_IOCTL message
			 * locally stored.
			 */
			termios_p->c_cflag = lpr232_p->c_cflag;

			/* change the message type */
			mp->b_datap->db_type = M_IOCTL;

			/* change the control command of mp to TIOCSETA */
			iocp->ioc_cmd = TIOCSETA;

			/* set the right size */
			iocp->ioc_count = sizeof(struct termios);

			/* send mp message downstream */
			qreply(q, mp);

			return(1);
		}

		/* if an TCSETA M_IOCTL message is locally stored */
		if( line_p->mp_stored_status & SP_IOCTL_TCSETA_STORED ) {
			struct termios *termios_p;
			struct termio *termio_p;
			struct iocblk *iocp_mioctl;

			/* termios structure in mp message  */
			termios_p = (struct termios *)mp->b_cont->b_rptr;
			iocp_mioctl = (struct iocblk *)line_p->mioctl_mp->b_rptr;

			/* termio structure in TCSETA M_IOCTL message
			 * locally stored.
			 */
			termio_p  = (struct termio *)line_p->mioctl_mp->b_cont->b_rptr;

			
			/* set the values in the termios structure
			 * to the corresponding values of the TCSETA M_IOCTL
			 * message locally stored.
			 */
			termios_p->c_iflag = termio_p->c_iflag;
			termios_p->c_oflag = termio_p->c_oflag;
			termios_p->c_cflag = termio_p->c_cflag;
			termios_p->c_lflag = termio_p->c_lflag;
			bcopy(termio_p->c_cc,termios_p->c_cc,NCC<NCCS?NCC:NCCS);

			/* change the message type */
			mp->b_datap->db_type = M_IOCTL;

			/* change the control command of mp to TIOCSETA */
			switch(iocp_mioctl->ioc_cmd) {
				case TCSETAF:
					iocp->ioc_cmd = TIOCSETAF; break;
				case TCSETAW:
					iocp->ioc_cmd = TIOCSETAW; break;
				default:
					iocp->ioc_cmd = TIOCSETA; break;
			}

			/* set the right size */
			iocp->ioc_count = sizeof(struct termios);

			/* send mp message downstream */
			qreply(q, mp);

			return(1);
		}

		/* we are not waiting for such a message:
		 * the SP_IOCTL_LPRSETA_STORED or SP_IOCTL_TCSETA_STORED
		 * is not set.
		 */
		
		break;
	}
		
	default:
		break;
	}	/* end -- switch(iocp->ioc_cmd) */
	}	/* end -- mp->b_wptr - mp->b_rptr == sizeof(struct iocblk) */

	putnext(q, mp);
	return(1);
}

/*
 * NAME:     cts_handle
 *
 * FUNCTION: This routine acts on the changes to the CTS.
 *           
 * RETURNS:  no return value 
 *
 */

static void
cts_handle(cts_type, q)
	enum status cts_type;
	queue_t	*q;
{
	struct line *line_p;

	line_p = (struct line *)q->q_ptr;

	switch(cts_type) {
	case cts_on:

		/* Is the SP_CTS_OFF1 bit set (that means that a timeout
		 * has been started to test, whether CTS changes to "on")?
		 */
		if(line_p->sp_ctsid) {
			untimeout(line_p->sp_ctsid);
			line_p->sp_ctsid = 0;
		}
		putctl((OTHERQ(q))->q_next,M_START);
		line_p->sp_cts = SP_CTS_ON;
		qenable(OTHERQ(q));
		break;

	case cts_off:
		putctl((OTHERQ(q))->q_next,M_STOP);
		line_p->sp_cts = SP_CTS_OFF1;
		/* (*HZ) : with pse_timeout the unit value is a tick */
		if(!line_p->sp_ctsid)
		   line_p->sp_ctsid = timeout(cts_timeout, q, NO_CTS_DELAY*HZ);
		break;

	default:
		break; /* other values are not valid - are not treated */
	}
	
	return;
}
		
	
/*
 * NAME:     cts_timeout
 *
 * FUNCTION: This routine is called, when the "CTS timer" has expired.
 *
 * RETURNS:  no return value 
 *
 */

static void
cts_timeout(q)
	queue_t	*q;
{
	struct line *line_p;

	line_p = (struct line *)q->q_ptr;
	line_p->sp_cts = SP_CTS_OFF2;
	line_p->sp_ctsid = 0;
	line_p->write_status |= SP_WRITE_NOT_POSSIBLE;
	qenable(WR(q));
	return;
}

/*
 * NAME:     read_mpcproto
 *
 * FUNCTION: This routine handles the M_PCPROTO messages at the read side.
 *
 * RETURNS:  no return value
 *
 */

static void
read_mpcproto(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{
	struct line *line_p;
	
	line_p = (struct line *)q->q_ptr;

	/* If the command of the M_PCPROTO message is not equal to LPWRITE_ACK,
	 * the message pointed to by mp is sent upstream and the routine returns
	 */
	if(*(int *)mp->b_rptr != LPWRITE_ACK) {
		putnext(q, mp);
		return;
	}

	/* If the SP_WRITE_IN_PROCESS bit is cleared (not set), the M_PCPROTO
	 * message is discarded and the routine returns.
	 */
	if( !(line_p->write_status & SP_WRITE_IN_PROCESS)) {
		freemsg(mp);
		return;
	}

	/* the SP_WRITE_IN_PROCESS bit is switched off and
	 * the SP_WRITE_FINISHED bit is set.
	 */
	line_p->write_status &= ~SP_WRITE_IN_PROCESS;
	line_p->write_status |= SP_WRITE_FINISHED;
	
	/* If sp_timerset is equal to TRUE (that means  a "write timer" has
	 * been set before and it has not yet expired), the "write timer" is
	 * stopped by calling the untimeout routine with sp_timerid as argument.
	 */
	if(line_p->sp_timerset == TRUE) {
		untimeout(line_p->sp_timerid);
		line_p->sp_timerid = 0;

		/* The sp_timerset and sp_timeout are set to FALSE */
		line_p->sp_timerset = FALSE;
		line_p->sp_timeout = FALSE;
	}

	/* If the SP_IOCTL_WREQ_STORED bit is set, this bit is switched off and
	 * the mess. pointed to by write_mioctl_mp is queued to the write queue.
	 */
	if(line_p->mp_stored_status & SP_IOCTL_WREQ_STORED) {
		line_p->mp_stored_status &= ~SP_IOCTL_WREQ_STORED;
		putq(OTHERQ(q), line_p->write_mioctl_mp);
		line_p->write_mioctl_mp = NULL;
	}

	/* Free this message. */
	freemsg(mp);

	return;

}

/*
 * NAME:     sptr_ioctl
 *
 * FUNCTION: This routine treats the ioctls destined to the sptr module.
 *
 * RETURNS:  1 on success
 *           0 if mp has to be (re)queued
 *
 */

static int
sptr_ioctl(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{
	struct line *line_p = (struct line *)q->q_ptr;
	struct iocblk *iocp = (struct iocblk *)mp->b_rptr;
	struct sptr_config *config_p = line_p->config_p;

	Enter(TRC_SPTR(TTY_IOCTL), config_p->devno, line_p, iocp->ioc_cmd, 0, 0);

	switch (iocp->ioc_cmd) {
	case IOCINFO: {					/* get device info */
		struct devinfo *devinfo_p;
		devinfo_p = (struct devinfo *)mp->b_cont->b_rptr;
		devinfo_p->devtype = DD_LP;		/* device type */
		devinfo_p->devsubtype = DS_SP;		/* device sub type */
		devinfo_p->flags = 0;

		ioctl_reply(q, mp, M_IOCACK, 0); /* positive acknowledgement */
		break;
	}

	/* Returns the page length, width and indentation values. */
	case LPRGET: {		
		struct lprio *lprio_p;
		lprio_p 	= (struct lprio *)mp->b_cont->b_rptr;
		lprio_p->ind 	= line_p->ind;	       /* get indent level */
		lprio_p->col 	= line_p->col;	       /* get # of columns */
		lprio_p->line 	= line_p->line;	       /* get # of lines/page */

		ioctl_reply(q, mp, M_IOCACK, 0); /* positive acknowledgement */
		break;
	}
	
	/* The page length, width and indent values are set in the local data
	 * structure "line" for the particular Stream.
	 */
	case LPRSET: {
		struct lprio *lprio_p;
		lprio_p	= (struct lprio *)mp->b_cont->b_rptr;
		
		/* test whether the ind, col and line fields have good values */
		if (lprio_p->col > 0 && lprio_p->ind >= 0 &&
		    lprio_p->ind < lprio_p->col && lprio_p->line >= 0) {
			line_p->ind = lprio_p->ind;	/* set indent level */
			line_p->col = lprio_p->col;	/* set # of columns */
			line_p->line = lprio_p->line;	/* set # lines/page */
			if (line_p->mcc == 0)
				line_p->ccc = line_p->ind;
						/* positive acknowledgement */
			ioctl_reply(q, mp, M_IOCACK, 0);

		} else { /* invalid values => negative acknowledgement */
			ioctl_reply(q, mp, M_IOCNAK, EINVAL);
		}
			
		break;
	}

	case LPRMODG: {				     /* get the printer modes */
		struct lprmod *lprmod_p;
		lprmod_p = (struct lprmod *)mp->b_cont->b_rptr;
		lprmod_p->modes = line_p->sp_modes;  /* get the printer modes */

		ioctl_reply(q, mp, M_IOCACK, 0);     /* pos. acknowledgement */
		break;
	}

	case LPRMODS: {				     /* set the printer modes */
		struct lprmod *lprmod_p;
		lprmod_p = (struct lprmod *)mp->b_cont->b_rptr;
		line_p->sp_modes = lprmod_p->modes;  /* set the printer modes */
		ioctl_reply(q, mp, M_IOCACK, 0);     /* pos. acknowledgement */
		break;
	}

	case LPRGTOV: {				        /* get timeout value */
		struct lptimer *lptimer_p;
		lptimer_p = (struct lptimer *)mp->b_cont->b_rptr;
		lptimer_p->v_timout = line_p->v_timout;	/* get timeout value */

		ioctl_reply(q, mp, M_IOCACK, 0);      /* pos. acknowledgement */
		break;
	}
	
	case LPRSTOV: {					/* set timeout value */
		struct lptimer *lptimer_p;
		lptimer_p = (struct lptimer *)mp->b_cont->b_rptr;

		if(lptimer_p->v_timout < 1) {	     /* valid time-out value? */
						      /* neg. acknowledgement */
			ioctl_reply(q, mp, M_IOCNAK, EINVAL);
			break;
		}
		line_p->v_timout = lptimer_p->v_timout;  /* set timeout value */
		ioctl_reply(q, mp, M_IOCACK, 0);      /* pos. acknowledgement */
		break;
	}
	
	case LPQUERY: {				        /* get lpquery struct */
		struct lpquery *lpquery_p;
		lpquery_p = (struct lpquery *)mp->b_cont->b_rptr;

		lpquery_p->tadapt = DD_LP;		/* get adapter type */

		/* number of available characters stored locally
		 * at the read side.
		 */
		lpquery_p->reccnt = msgdsize(line_p->input_data_mp);

		lpquery_p->status = 0;
		if(line_p->sp_cts & SP_CTS_OFF2) 
			lpquery_p->status |= LPST_NOSLCT;

		if(line_p->write_status & SP_WRITE_IN_PROCESS) 
			lpquery_p->status |= LPST_BUSY;

		if(line_p->sp_timeout) 
			lpquery_p->status |= LPST_TOUT;

		ioctl_reply(q, mp, M_IOCACK, 0);      /* pos. acknowledgement */
	
		break;
	}

	case LPRGETA: {				      /* get lpr232 structure */

		/* send an TIOCGETA M_IOCTL message downstream to get
		 * the termios structure from the driver.
		 */
		if(!send_Xctl_tiocgeta(q, mp,LPRGETA))
			Return(0);  /* if sending failed, mp will be (re)queued
				     * in str_sptr_wput() or str_sptr_wsrv() 
				     */

		/* this M_IOCTL will be stored locally and acknowledged when
		 * the answer to the TIOCGETA M_IOCACK/M_IOCNAK message comes
		 * on the read side.
		 */
		line_p->mioctl_mp = mp;
		line_p->mp_stored_status |= SP_IOCTL_LPRGETA_STORED;
		
		break;
	}

	case LPRSETA: {
		struct termios termios;
		struct lpr232  *lpr232_p;

		/* firstly, match the input and output speeds
		 */
		lpr232_p  = (struct lpr232 *)mp->b_cont->b_rptr;
		termios.c_cflag = lpr232_p->c_cflag;
		cfsetispeed(&termios, cfgetospeed(&termios));
		/*
			Currently, speeds take values in the bits of
			_CBAUD(0xf), and all values (B0 to B38400) are valid.
			Pass the speed downstream and let the driver decide
			if the specified speed is supported by the device.
			In future, if failure above, do the following
			ioctl_reply(q, mp, M_IOCNAK, EINVAL); break;
		 */
		lpr232_p->c_cflag = termios.c_cflag;

                /* secondly, send an TIOCGETA M_CTL message downstream to get
		 * the termios structure from the driver.
		 */
		if(!send_Xctl_tiocgeta(q, mp,LPRSETA))
			Return(0); /* if sending failed, mp will be (re)queued
				    * in str_sptr_wput()  or str_sptr_wsrv() 
				    */

		/* this M_IOCTL will be stored locally and acknowledged later,
		 * when an appropriate M_IOCACK/M_IOCNAK message comes on the
		 * read side.
		 */
		line_p->mioctl_mp = mp;
		line_p->mp_stored_status |= SP_IOCTL_LPRSETA_STORED;

		break;
	}	

	case TCGETA: {				      /* get lpr232 structure */

		/* send an TIOCGETA M_IOCTL message downstream to get
		 * the termios structure from the driver.
		 */
		if(!send_Xctl_tiocgeta(q, mp,TCGETA))
			Return(0);  /* if sending failed, mp will be (re)queued
				     * in str_sptr_wput() or str_sptr_wsrv() 
				     */

		/* this M_IOCTL will be stored locally and acknowledged when
		 * the answer to the TIOCGETA M_IOCACK/M_IOCNAK message comes
		 * on the read side.
		 */
		line_p->mioctl_mp = mp;
		line_p->mp_stored_status |= SP_IOCTL_TCGETA_STORED;
		
		break;
	}

	case TCSETA:
	case TCSETAF:
	case TCSETAW:
	{
                /* firstly, send an TIOCGETA M_CTL message downstream to get
		 * the termios structure from the driver.
		 */
		if(!send_Xctl_tiocgeta(q, mp,TCSETA))
			Return(0); /* if sending failed, mp will be (re)queued
				    * in str_sptr_wput()  or str_sptr_wsrv() 
				    */

		/* this M_IOCTL will be stored locally and acknowledged later,
		 * when an appropriate M_IOCACK/M_IOCNAK message comes on the
		 * read side.
		 */
		line_p->mioctl_mp = mp;
		line_p->mp_stored_status |= SP_IOCTL_TCSETA_STORED;

		break;
	}	

	case LPWRITE_REQ: {
		write_req_ioctl(q, mp);
		
		break;	
	}

	default:
		/* flow control or message is already queued are yet
		 * tested in str_sptr_wput() and str_sptr_wsrv().
		 */
		putnext(q, mp); 
		break;
	} /* end: switch (iocp->ioc_cmd) { */

	Return(1);

}

/*
 * NAME:     write_req_ioctl
 *
 * FUNCTION: This routine handles the LPWRITE_REQ M_IOCTL messages.
 *           
 * RETURNS:  no return value
 *
 */

static void
write_req_ioctl(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{
	struct line *line_p;

	line_p = (struct line *) q->q_ptr;

	/* write is not possible */
	if(line_p->write_status & SP_WRITE_NOT_POSSIBLE) {
		ioctl_reply(q, mp, M_IOCNAK, ENXIO);  /* neg. acknowledgement */
	}

	/* the write timed out */
	else if(line_p->write_status & SP_WRITE_TIMEDOUT) {
		line_p->write_status &= ~SP_WRITE_TIMEDOUT;
		ioctl_reply(q, mp, M_IOCNAK, EBUSY);  /* neg. acknowledgement */
	}

	/* write has finished correctly */
	else if(line_p->write_status & SP_WRITE_FINISHED) {
		line_p->write_status &= ~SP_WRITE_FINISHED;
		ioctl_reply(q, mp, M_IOCACK, 0);      /* pos. acknowledgement */
	}

	/* data destined for writing has been flushed before */
	else if(line_p->write_status & SP_WRITE_DATA_FLUSHED) {
		line_p->write_status &= ~SP_WRITE_DATA_FLUSHED;
		ioctl_reply(q, mp, M_IOCNAK, ENXIO);  /* neg. acknowledgement */
	}

	else {
		if(line_p->write_status & SP_WRITE_IN_PROCESS) {
					 /* store mp ptr into write_mioctl_mp */
			line_p->write_mioctl_mp = mp;
			line_p->mp_stored_status |= SP_IOCTL_WREQ_STORED;
		}
	}

}

/*
 * NAME:     write_mflush
 *
 * FUNCTION: This routine handles the M_FLUSH messages arriving
 *	      at the write side.
 *           
 * RETURNS:  no return value
 *
 */

static void
write_mflush(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{
	
	struct line *line_p;

	line_p = (struct line *) q->q_ptr;

	/* If it is not a flush for the write side, the message
	 * is sent downstream and the routine returns.
	 */
	if(!(*mp->b_rptr & FLUSHW)) { 
		putnext(q, mp);
		return;
	}

	/* If an M_DATA message is queued */
	flushq(q, FLUSHDATA);
	if(line_p->write_status & SP_WRITE_DATA_QUEUED) {
		line_p->write_status &= ~SP_WRITE_DATA_QUEUED;
		line_p->write_status |= SP_WRITE_DATA_FLUSHED;
	}

	/* if data is stored locally pointed to by line_p->write_mdata_mp */
	/* if an LPWRITE_REQ M_IOCTL message is stored locally pointed to by
	 * line_p->write_mioctl_mp */
	if(line_p->mp_stored_status & SP_IOCTL_WREQ_STORED) {
		line_p->mp_stored_status &= ~SP_IOCTL_WREQ_STORED;
		freemsg(line_p->write_mioctl_mp);
		line_p->write_mioctl_mp = NULL;
	}

	/* remove the write and cts timers if running */
	if (line_p->sp_timerid)
		untimeout(line_p->sp_timerid);
	if (line_p->sp_ctsid)
		untimeout(line_p->sp_ctsid);

	putnext(q, mp);
	return;
}

/*
 * NAME:     tioc_handle
 *
 * FUNCTION: This routine handles the TIOC_REPLY M_CTL message.
 *           It adds some information to this message, to inform the 
 *           Streams module "tioc" how to transform the transparent ioctls,
 *           directed to the "sptr" module, into I_STR ioctls.
 *           
 * RETURNS:  1 on success
 *           0 if mp has to be (re)queued
 *
 */

static int
tioc_handle(q, mp)
	queue_t	*q;
	mblk_t	*mp;
{

	register mblk_t *new_mp;
	register int r_size;
	struct line *line_p;

	line_p = (struct line *) q->q_ptr;
	
	r_size = sizeof(str_sptr_tioc_reply);

	/* allocate a message to put the information in it */
	new_mp = allocb(r_size, BPRI_MED);
	
	if(!new_mp) { /* allocation failed */

		allocb_recovery(q, &line_p->rbid,  &line_p->rtid, r_size);
		return(0); /* mp will be put to the queue in str_sptr_rput()
			     or str_sptr_rsrv() */
	}
	else{ /*  allocation succeeded */
                new_mp->b_cont = mp->b_cont;
                mp->b_cont = new_mp;
                new_mp->b_datap->db_type = M_DATA;
                new_mp->b_wptr = new_mp->b_rptr + r_size;

		/* copy str_sptr_tioc_reply[] into the new message */
                bcopy(str_sptr_tioc_reply, new_mp->b_rptr, r_size);
        }
	/* Set the tioc q_maxpsz to INFPSZ both in read and write queues */
	/* to make sure that the frame work will not split the data      */
	strqset(NEXTQ(q), QMAXPSZ, 0, INFPSZ);
	strqset(DIAGQ(q), QMAXPSZ, 0, INFPSZ);

	return(1);
}

/*
 * NAME:	str_sp_sendff 
 *
 * FUNCTION:	This routine has to send a form feed to the serial line.
 *
 * RETURNS:	1 on success
 *	   	0 allocb() failure
 *
 * COMMENTS :	Contrary to the sp_line.c module it's not necessary to test
 *		again if the last outpout was in PLOT mode or not because
 *		str_sp_sendff() could be only called by str_sptr_close.
 *		str_sptr_close freed the "line" structure so not any
 *		information remains available to the next "write" to known
 *		if it has to send a form feed again as in sp_line.c.
 *		SEE ALSO (!@#&) comment in str_sptr_close.
 */

str_sp_sendff(q, mpp)
	queue_t	*q;
	mblk_t **mpp;
{
	mblk_t *mp;
	int    error;

	mp = *mpp;

	if (error = sptr_prnformat(q, mpp, FF)) {
		putnext(q, mp);
	} else {
		if (mp->b_cont != NULL)
			freemsg(mp->b_cont);

		mp->b_wptr = mp->b_rptr;
	}

	return(error);
}

/*
 * NAME: ioctl_reply
 *
 * FUNCTION: This routine changes the M_IOCTL message to an db_type (M_IOCACK
 *	     or M_IOCNAK) message, set the given ioc_error  and sends the
 *	     message upstream.
 *
 * RETURNS:  no return value
 *
 */

static void
ioctl_reply(q, mp, db_type, ioc_error)
	queue_t *q;
	mblk_t *mp;
	unsigned char db_type;
	int ioc_error;
{
	struct iocblk *iocp;
	
	iocp = (struct iocblk *)mp->b_rptr;
					/* change message type: positive */
	mp->b_datap->db_type = db_type; /* or negative acknowledgement   */
					  
	iocp->ioc_error = ioc_error;
	iocp->ioc_rval = 0;
	qreply(q, mp);
}

/*
 * NAME:     send_Xctl_tiocgeta
 *
 * FUNCTION: This routine allocates and sends a TIOCGETA M_IOCTL ([LPR|TC]GETA)
 *	     or a TIOCGETA M_CTL ([LPR|TC]SETA) message downstream, to get the
 *	     termios structure from the driver.
 *
 * RETURNS:  1 on success
 *           0 if mp has to be (re)queued
 *
 */

static int
send_Xctl_tiocgeta(q, mp,cmd)
	queue_t *q;
	mblk_t *mp;
	int cmd;
{
	mblk_t *new_mp,  *new_mp_cont;
	struct iocblk *iocp;
	struct line *line_p;

	line_p = (struct line *) q->q_ptr;
	
	/* allocation of first message block (for iocblk structure) */
	new_mp = allocb(sizeof(struct iocblk), BPRI_MED);
	/* allocation of second message block (for termios structure) */
	new_mp_cont = allocb(sizeof(struct termios), BPRI_MED);
	
	if(!new_mp || !new_mp_cont) { 			 /* allocation failed */
		if(new_mp)
			freemsg(new_mp);
		if(new_mp_cont)
			freemsg(new_mp_cont);

		allocb_recovery(q, &line_p->wbid,  &line_p->wtid,
				max(sizeof(struct iocblk), (sizeof(struct termios))));

		return(0);	/* mp will be put [back] to the queue in
				 * str_sptr_rput() or str_sptr_rsrv() */
	}
	else{					     /*  allocation succeeded */
		new_mp->b_wptr = new_mp->b_rptr + sizeof(struct iocblk);
		iocp = (struct iocblk *) new_mp->b_rptr;

		switch (cmd) {
		case LPRGETA:
		case TCGETA:
			new_mp->b_datap->db_type = M_IOCTL;
			iocp->ioc_count = sizeof(struct termios);
			break;

		case LPRSETA:
		case TCSETA:
			new_mp->b_datap->db_type = M_CTL;
			iocp->ioc_count = sizeof(struct iocblk);
			break;
		}

		iocp->ioc_cmd = TIOCGETA;
		
		new_mp_cont->b_wptr = new_mp_cont->b_rptr + sizeof(struct termios);
		new_mp->b_cont = new_mp_cont;
		
		putnext(q, new_mp);	
	}
	return(1);
}

/*
 * NAME:     allocb_recovery
 *
 * FUNCTION: This routine does the "error" recovery after an allocb() failure.
 *
 * RETURNS:  no return value
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
 * NAME:     sptr_print_help
 *
 * FUNCTION: print the usage of "call sptr_kdb".
 *
 * RETURNS:  no return value
 *
 */

void
sptr_print_help()
{
	printf("\nUsage:\tcall sptr_kdb C/c/l/K[v] no_param/@(sptr_config struct)/@(line struct)/@(line struct)\n");
}

/*
 * NAME:      sptr_kdb
 *
 * FUNCTION:  Print function for kdb debugger :
 *	      Converts the string to parameter list,
 *	      Prints some local information,
 *	      Calls the lldb print function sptr_print.
 *
 * ARGUMENTS: buf  = kdb sub-command line.
 *	      poff = index in buf string.
 *	      len  = length of buf.
 *
 * RETURNS:  no return value
 *
 */

void
sptr_kdb(unsigned char *buf, int *poff, int len)
{
	int ch;
	long tp;
	int v;
	int argv = 0;

	/*
	 * Read all blanks between function name and first parameter
	 */
	sptr_kdb_next_param(buf, poff, len);
	ch = buf[*poff] & 0xFF;
	(*poff)++;

	switch (ch) {
	case 'C':
		tp = 0;				/* for call sptr_print reason */
		v = SPTR_KDB_C;
		break;
	case 'c':
		TST_SPTR_KDB_NEXT_PARAM(buf, poff, len);
		tp = mi_strtol (buf + *poff, NULL, 16);

		if (!sptr_chk_sptr_configp(tp)) {
			printf("0x%08x is not a valid struct sptr_config pointer\n", tp);
			return;
		}

		v = SPTR_KDB_c;
		break;
	case 'l':
		TST_SPTR_KDB_NEXT_PARAM(buf, poff, len);
		tp = mi_strtol (buf + *poff, NULL, 16);

		if (!sptr_chk_linep(tp)) {
			printf("0x%08x is not a valid struct line pointer\n", tp);
			return;
		}

		v = SPTR_KDB_l;
		break;
	default:
		sptr_print_help();
		v = SPTR_KDB_UNKNOW;
		break;
	}

	if (v != SPTR_KDB_UNKNOW) {
		kdb_sptr_print(v, tp, argv);
	}
	return;
}

/*
 * NAME:      sptr_kdb_next_param
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

sptr_kdb_next_param(unsigned char *buf, int *poff, int len)
{

	while ((len - *poff) > 0 && (buf[*poff] == ' ')) (*poff)++;

	return(((len - *poff) <= 0) ? 0 : 1);
}

/*
 * NAME:     sptr_chk_sptr_configp
 *
 * FUNCTION:
 *
 * RETURNS:  = 1 : The parameter passed as argument is really a struct
 *		   sptr_config pointer.
 *	     = 0 : is not.
 *
 */

int
sptr_chk_sptr_configp(struct sptr_config *chk_configp)
{
	struct sptr_config *configp;

	for(configp = sptr_config_list; configp != NULL; configp = configp->next) {
		if(configp == chk_configp)
			break;
	}

	return((configp == NULL) ? 0 : 1);
}

/*
 * NAME:     sptr_chk_linep
 *
 * FUNCTION:
 *
 * RETURNS:  = 1 : The parameter passed as argument is a struct line pointer.
 *	     = 0 : is not.
 *
 */

int
sptr_chk_linep(struct line *chk_linep)
{
	struct sptr_config *configp;

	for(configp = sptr_config_list; configp != NULL; configp = configp->next) {
		if(configp->line_p == chk_linep)
			break;
	}

	return((configp == NULL) ? 0 : 1);
}

/*
 * NAME:     kdb_sptr_print
 *
 * FUNCTION:
 *
 * RETURNS:  Always 0
 *
 */

int
kdb_sptr_print(int v, void *tp, int argv)
{

	switch (v) {
	case SPTR_KDB_C: 
		sptr_config_list_print();
		break;

	case SPTR_KDB_c: 
		sptr_config_print(tp);
		break;

	case SPTR_KDB_STD: 
	case SPTR_KDB_l: 
		sptr_line_print(tp);
		break;

	default:
		break;
	}
    return(0);
}

/*
 * NAME:     sptr_config_list_print
 *
 * FUNCTION: Print the linked list of config structure(s)
 *
 * RETURNS:  none
 *
 */

sptr_config_list_print()
{
	int i = 7;
	struct sptr_config *configp;

	printf("sptr_config_list:  ");
	for(configp = sptr_config_list; configp != NULL; configp = configp->next) {
		if (i == 7) {
			printf("\n        ");
			i = 0;
		}
		printf("0x%08x  ", configp);
		i++;
	}

}

#endif	/* _KDB */
