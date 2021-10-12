#ifndef lint
static char sccsid[] = "@(#)92 1.23 src/bos/kernext/tty/POWER/slion.c, sysxs64, bos411, 9438A411a 9/16/94 08:21:24";
#endif
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS:	
 *  myttyinput		gcomm_tmr_f
 *  sbox_reset		sscan_baud_table	sct_unlink	sGotHilink	
 *  sl_dram_wr		slioc_dram		slioc_dslp	slioc_pres	
 *  slioc_slpx		slioc_trng		slionadd	slionadd1	
 *  slionclose		slionconfig		sliondel
 *  slionopen		slionproc		slionrsrv	slionservice	
 *  slionwput		slionwsrv
 *  slion_comms		slion_ctl
 *  slion_do_comms	slion_err		slion_flush
 *  slion_ioctl		slion_offlev		slion_pio		
 *  slion_recv		_slion_recv		slion_send	slion_set_gcomm
 *  slion_slih		slion_termios_get	slion_termios_set		
 *  slion_termiox_get	slion_termiox_set	slion_wake_tmr	strttyinput*
 *  sxu_close		sxu_getpos	
 *  sxu_open		sxu_mod64		sxu_setpos	sxvscpy		
 *  dslp_do_offlevel    dslpofflevel            dslp_tmr_f
 *  slion_break_set	slion_break_clear	slion_recover
 *
 * ORIGINS: 27 83
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

/* This is a streams driver which supports 64_Port Async Controller. 
 */
/*
 * This piece of code originated at IBM. The Conversion from traditional 
 * type driver to Streams driver is done by BULL, Echirolles.
 */

#include <sys/ioctl.h>			/* Ioctl declerations*/
#include <sys/types.h>
#include <sys/uio.h> 			/* Struct uio */
#include <sys/lockl.h>			/* For lockl and unlockl */
#include <sys/pri.h>
#include <sys/sleep.h>			/* EVENT_NULL */
#include <sys/sysmacros.h>		/* Minor and major macros */
#include <sys/errno.h>			/* Error codes */
#include <sys/mstsave.h>		/* Temp for debugger thing */
#include <sys/low.h>			/* Temp for debugger thing */
#include <sys/signal.h>			/* SIGSAK */
#include <sys/device.h>			/* For config stuff */ 
#include <sys/fp_io.h>
#include <sys/li.h>			/* Ioctl's for 64 port adapter */
#include <sys/trchkid.h>		/* Trace hook id's */
#include <sys/sysinfo.h>		/* Sysinfo stuff */
#include <sys/dump.h>			/* Dump structures */
#include <sys/str_tty.h>		/* Streams defines */
#include <sys/stream.h>			/* Streams defines */
#include <sys/stropts.h>		/* Streams defines */
#include <sys/strlog.h>			/* Streams defines */
#include <sys/strconf.h>		/* Streams defines */
#include "ttydrv.h"			/* Substitute by stty.h */
#include "slion.h"			/* Lion driver definitions */
#include "common_open.h"                /* Common open discipline functions */
#include <sys/lpio.h>                   /* LPWRITE_REQ and LPWRITE_ACK */

#ifdef SLI_TRACE
/************** DBG DEFINITION ONLY **************************/
ushort	DBFLG = 0;
#define CNFG		1		/* Trace slion_pio	*/
#define MSRV		0x2		/* Trace slionservice	*/
#define LVL1		0x10		/* Trace level 1
					 *   sGotHilink, slion_slih
 					 */
#define LVL2		0x20		/* Trace level 2
					 * Box_reset
					 */
#define LVL3		0x40		/* Trace level 3
					 * Slion_comm, slion_set_gcom
					 * sgcomm_tmr_f, slion_do_comms
					 */	
#define LVL4		0x80		/* Rx data trace
					 */
#define	TRCHA		0x200		/* Display rcv char */
#define LVL5		0x100		/* Tx data trace
					 */
/******END***** DBG DEFINITION ONLY **************************/
#endif /* SLI_TRACE */

#define CONFIG_TIME 	-1
						/* Def. of device number */
#define ADP(dev)	(minor(dev)>>8)		/*  bit(8-15):adap. id(slot)*/
#define PRT(dev)	(minor(dev)&0x3f)	/*  bit(0-5): port number*/
#define CHN(dev)	((minor(dev)&0xc0)>>6)	/*  bit(6-7): feature val * 
						 *   0=main 1=vterm 2=xpar 
						 */

#ifdef FEATURES
#define MK_TP(dev, chan)	switch (chan) {			\
	case CHAN_MAIN: tp = adap_list[ADP(dev)]->m_ttyp[PRT(dev)]; break;\
	case CHAN_VGER: tp = adap_list[ADP(dev)]->v_ttyp[PRT(dev)]; break;\
	case CHAN_XPAR: tp = adap_list[ADP(dev)]->x_ttyp[PRT(dev)]; break;\
	default: Return(ENODEV); }
#else /*~FEATURES*/
#define MK_TP(dev, chan) tp = adap_list[ADP(dev)]->m_ttyp[PRT(dev)];
#endif /* FEATURES */

#ifdef FEATURES
#define MK_TTY(dev, chan)	switch (chan) {			\
	case CHAN_MAIN: tp = &(adap_list[ADP(dev)]->m_tty[PRT(dev)]); break;\
	case CHAN_VGER: tp = &(adap_list[ADP(dev)]->v_tty[PRT(dev)]); break;\
	case CHAN_XPAR: tp = &(adap_list[ADP(dev)]->x_tty[PRT(dev)]); break;\
	default: Return(ENODEV); }
#else /*~FEATURES*/
#define MK_TTY(dev, chan) tp = &(adap_list[ADP(dev)]->m_tty[PRT(dev)]);
#endif /* FEATURES */

int (*sxu_assist)(int *mask) = (int(*)())0;

#ifdef TTYDBG
static struct str_module_conf slion_str_module_conf = 
				{ "lion", 'd', (int(*)())LION_PDPF };
#endif /* TTYDBG */

/* ====================== */
/* For configuration time */
/* ====================== */
/*
 * This structure is used to keep adapter configuration information
 * to be used for all the lines on it.
 */
struct adap_conf {
    struct lion_adap_dds dds;
    struct adap_conf 	 *next;
};

/* Local data structures */
struct adapters {
    enum adap_type type;
    uchar pos0, pos1;
    char *name;
    struct intr *intr;
    int ports;
};

/* Local function declarations */
static	int  slionrsrv(queue_t *q);
static	int  slionwput(queue_t *q,mblk_t *mp);
static	int  slionwsrv(queue_t *q);
static	int  slion_ctl(queue_t *q, mblk_t *mp);
	int  slion_timeout(str_lionp_t tp);
mblk_t	    *slion_allocmsg(str_lionp_t tp);
	int  slion_recover(str_lionp_t tp);
	int  slion_termiox_set(str_lionp_t  tp,struct termiox * tiox);
	int  slion_termiox_get(str_lionp_t  tp,struct termiox * tiox);
	int  slion_termios_set(str_lionp_t  tp,struct termios * tios);
	int  slion_termios_get(str_lionp_t  tp,struct termios * tios);
	int  slion_break_set(str_lionp_t tp, int duration);
	int  slion_break_clear(str_lionp_t tp);
static	int  slion_flush(str_lionp_t tp,int rw, int recycle);
static	int  myttyinput(str_lionp_t tp, queue_t *ctl, enum status ss);
static	int  strttyinput(queue_t *q, mblk_t *mp, char c, enum status status,
	     str_lionp_t tp);

static	int  slionadd(dev_t dev,struct lion_adap_dds *Aconf,
	     struct lion_line_dds *Lconf);
static	     slionadd1(dev_t dev, struct sli_adap *adap,
	     struct lion_line_dds *Lconf);
static	int  sliondel(dev_t dev);
static       sct_unlink(struct sli_adap *adap);
static	int  slionopen(queue_t *q,dev_t *devp, int mode, int sflag,
	     cred_t *credp);
static	int  slionclose(queue_t *q, int mode, cred_t *credp);
static	int  slion_ioctl(queue_t *q, mblk_t *mp,int fromput);
static	     sxu_open(dev_t parent);
static	     sxu_close();
static	     sxu_setpos(uchar where, uchar which, uchar how);
static	     sxu_getpos(uchar where, uchar which, uchar *value);
static	     sxu_mod64(int slot, int pos3, int cmd);
static	int  slioc_slpx(struct sliondata *ld, int *result, int cmd);
static	int  slioc_dslp(struct sliondata *ld);
void	     dslp_tmr_f(struct sliondata *ld);
static	     sxvscpy(char *dst, char *src, int len);
static	     slioc_pres(struct sliondata *ld, int *result);
static	     slioc_dram(struct sliondata *ld, int *result);
static	     sl_dram_wr(struct sliondata *ld);
static	     slioc_trng(struct sliondata *ld, int recv);
static	int  slionproc(str_lionp_t tp, int cmd);
static	int  slionservice(str_lionp_t tp, enum service_commands cmd, 
	     void *varg);
	int  slion_slih(struct sli_slih *intr);
	int  slion_offlev(struct sli_adap *adap);
static	void sGotHilink(struct sli_adap *adap, int port);
	void sbox_reset(struct sliondata *ld);
static	     slion_send(struct sliondata *ld, str_lionp_t tp, uchar *BusMem);
	int  slion_comm(struct sliondata *ld, int comm);
	void slion_do_comms(struct sliondata *ld);
int	_slion_recv(struct sliondata *ld, str_lionp_t tp);
	void slion_wake_tmr(struct sliondata *ld);
	void slion_set_gcomm(struct sliondata *ld, int mask);
	void sgcomm_tmr_f(struct sliondata *ld);
static	int  sscan_baud_table(int arg);
	void slion_err(char *name, int code, int err);
	void slion_pio(struct sliondata *ld, pud_t *pud, int logit);

/* pinned variable definitions */
struct	sli_adap **adap_list = 0;
char	resource[16] = "liondd";

/* Pointer on adapter configuration structures */
struct adap_conf *adap_conf_ss = 0;

/* Variable declarations */
static int adp_cnt = 0;
static lock_t oc_lock = LOCK_AVAIL;	/* open lock */
static struct file *sxu_fp;		/* file pointer to expansion box */
static int sxu_opencnt = 0;		/* expansion box open/close count */
static lock_t sxu_lock = LOCK_AVAIL;	/* expansion box lock */

struct {
    mid_t lion_mid;
    ushort anums;
} lion_mid = { 0, 0, };

static struct slionprms prm_proto = {
    {'\001','g','1','\377'},
    {'\001','g','2','\377'},
    {'s','c','r','1','\377','\377','\377','\377','\377','\377','\377'},
    {'s','c','r','2','\377','\377','\377','\377','\377','\377','\377'},
    {'P','o','n','\377','\377','\377','\377','\377','\377','\377','\377'},
    {'P','O','F','F','\377','\377','\377','\377','\377','\377','\377'},
    30, 0x42, 0x10,  0x24, 19200, bits8, nopar, stop1,
    0,0, 0,0, 0,0,0,0,0,0,0, 0,0, 0x11, 0x13, 0x11, 0x13,
    0, 0, EVENT_NULL, EVENT_NULL, 0, };

static struct sli_adap *adap_root;
static struct sli_slih *slih_root;

static struct intr slion_intr = {
    0, slion_slih, BUS_MICRO_CHANNEL, 0, CONFIG_TIME, INTCLASS0, 
    CONFIG_TIME,
};

static struct adapters adap_types[] = {
    { SixtyFourPort, 0xfd, 0x61, "The Lion", &slion_intr, 64 },
    { Unknown_Adap, },
};

static struct module_info slionm_info = {
	DRIVER_ID,"lion",0,INFPSZ,2048,256	/* 1024 128: problem*/	
};

static struct qinit slion_rinit = {
	0,slionrsrv,slionopen,slionclose,0,&slionm_info,0
};

static struct qinit slion_winit = {
	slionwput,slionwsrv,0,0,0,&slionm_info,0
};

static struct streamtab slioninfo = { 
	&slion_rinit,&slion_winit,0,0
};
	
/* 
 * KB April, 14th, 1993. SQLVL_QUEUEPAIR in the sc_sqlevel instead of 0
 * for 4.1 pse compatibility	
 */

static strconf_t slionconf = {
	"lion",&slioninfo,STR_NEW_OPEN|STR_MPSAFE,0,SQLVL_QUEUEPAIR, 0 
};

/*
 * tioc_reply structures array. It is given as an answer to TIOC_REQUEST
 * M_CTLs.  Only answer with null.
 */
static struct tioc_reply
slion_tioc_reply[] = {
        { LI_PRES, sizeof(int), TTYPE_COPYOUT },
        { LI_DRAM, sizeof(int), TTYPE_COPYOUT },
        { LI_SLP0, sizeof(int), TTYPE_COPYOUT },
        { LI_SLP1, sizeof(int), TTYPE_COPYOUT },
        { LI_DSLP, sizeof(struct slp_data), TTYPE_COPYINOUT },
        { LI_GETVT, sizeof(struct vter_parms), TTYPE_COPYOUT },
        { LI_SETVT, sizeof(struct vter_parms), TTYPE_COPYIN },
        { LI_GETXP, sizeof(struct xpar_parms), TTYPE_COPYOUT },
        { LI_SETXP, sizeof(struct xpar_parms), TTYPE_COPYIN },
        { LI_GETTBC, sizeof(int), TTYPE_COPYOUT },
        { LI_SETTBC, sizeof(int), TTYPE_COPYIN }
};

/**********************************************************
 *	Upper interface:	New defined functions			
 **********************************************************
 */

/*
 * FUNCTION: 	slionrsrv
 *
 * PURPOSE:	Driver read service routine .
 *
 * INPUT:
 *	q:	Input queue pointer.
 *
 * RETURN:
 *	1:	Always.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	getq		canput		putnext 
 *
 * DESCRIPTION:
 * 	Completely pure STREAMS handling only here.
 * 	If priority message or upstream not blocked by flow control, 
 *	pass the message upstreams. Else put it back in the queue and 
 *	exit for back enabling.
 *	Before return, read queue is checked if Q is more than HWM or less
 *	than LWM. Based on the condition, T_BLOCK or T_UNBLOCK is sent. 
 *	This check is done by slion_recv,too.
 *
 * CALLED BY:
 *	 After calling putq in _slion_recv().
 */
static int 
slionrsrv(queue_t *q)
{
	register str_lionp_t	tp = (str_lionp_t)q->q_ptr;
	mblk_t 			*mp;
	int			old_intr;
	struct sliondata 	*ld = (struct sliondata *)tp->t_hptr;

	Enter(TRC_LION(TTY_RSRV), tp->t_dev, (int)q->q_ptr, q->q_count, 0, 0);

	SL_printf("\nSrsr.%x|", PRT(tp->t_dev));			

	while (mp = getq(q)) 
	{
		SL_printf("cmd=%x|",mp->b_datap->db_type);
		switch (mp->b_datap->db_type) 
		{
		   case M_DATA:
		   default:
			if((QPCTL <= mp->b_datap->db_type)|| canput(q->q_next))
			{
				putnext(q,mp);
				SL_printf("pnext|");			
			}
			else 
			{
				SL_printf("putbq|");
				putbq(q,mp);
				goto rqueue_flow_check;
			}
			break;
		}
	}
	/* Immediate local input queue flow control handling: */

rqueue_flow_check:

	old_intr = i_disable(INT_TTY);
	if((q->q_count > ((q->q_lowat + q->q_hiwat)/2)) && !(ld->block)) 
		slionproc(tp, T_BLOCK);
	if((q->q_count <= q->q_lowat) && (ld->block)) 
		slionproc(tp, T_UNBLOCK);
	i_enable(old_intr);
	Return(1);

} /* slionrsrv */

/*
 * FUNCTION: 	slionwput
 *
 * PURPOSE:	Driver write put routine .
 *
 * INPUT:
 *	q:	Output queue pointer.
 *	mp:	Message pointer on queue.
 *
 * RETURN:
 *	0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_ioctl	slion_ctl	slion_flush 	slionproc
 *
 * EXTERNAL PROCEDURES CALLED:
 *	putq		unlinkb		qreply 		i_disable
 *	i_enable
 *
 * DESCRIPTION:
 * 	Processes received messages. 
 *	The Data messages are put into the queue. However, 
 *	if o/p data is small(ie:data is not yet queued), data is sent
 *	directly on line if Tx condition is ready(see below coding). 
 *	The IOCTL message which requires immediate handling 
 *	are done here. 
 *
 * CALLED BY:
 *	Streams Framework.
 */
static int
slionwput(queue_t *q, mblk_t *mp)
{
	register uchar		msgtyp = mp->b_datap->db_type; 
	register str_lionp_t	tp = (str_lionp_t)q->q_ptr;
	register mblk_t		*nmp;
	int			flag = 0;
	int			old_intr;
	struct sliondata 	*ld = (struct sliondata *)tp->t_hptr;
	int			nn;

	Enter(TRC_LION(TTY_WPUT), tp->t_dev, (int)tp, mp, msgtyp, 0);

	SL_printf("\nSwp.%x|",PRT(tp->t_dev));
	switch (msgtyp) 
	{

	case M_BREAK:
	case M_DELAY:
		SL_printf("BK/DLY|");
		putq(q,mp);
		break;

	case M_DATA:
		SL_printf("Dt|");
		/* Send characters directly down 
		 * to board if possible
		 */
		old_intr = i_disable(INT_TTY);
		if( !(q->q_first) &&			/* nothing queued */
		   !(tp->t_stop) && ld &&		/* output not stopped */
		   !(tp->t_outmsg) && 			/* prev msg not free*/
		   !(ld->flags & BUSY_MASK) &&		/* port not busy */
		   !(tp->t_busy) &&
		   !(mp->b_cont) &&			/* only one part msg */
		   ((nn=(mp->b_wptr - mp->b_rptr)) > 0)) /* chars to tx */ 
		{
			ld->txcnt = nn; 
			ld->txbuf = mp->b_rptr;
			tp->t_outmsg = mp;
			slionproc(tp,T_OUTPUT);		 /* output the msg */
			i_enable(old_intr);
		}
		else
		{
			/* Decompose msg and queue to service rtn */
			i_enable(old_intr);
			while (mp)		/* if no more left, return */ 
			{
				nmp = unlinkb(mp);
				/* Ensure that each fragmenhas 
				 * type M_DATA; this is *not*
 				 * garanteed otherwise 
				 */
				mp->b_datap->db_type = M_DATA;
				putq(q, mp);
				mp = nmp;
			}
        	} /* if */
		break;

	case M_IOCTL:					/*0x16*/
		SL_printf("IOCT|");
		slion_ioctl(q,mp,1);
		break;

	case M_CTL:
		SL_printf("CTL|");
		slion_ctl(q,mp);
		break;

	case M_FLUSH:
		SL_printf("FLS|");
		if(*mp->b_rptr & FLUSHW) 
		{
			flag |= FLUSHW;
			*mp->b_rptr &= ~FLUSHW;
		}
		if( *mp->b_rptr & FLUSHR) 
		{
			flag |= FLUSHR;
			qreply(q,mp);
		}
		else 
			freemsg(mp);
		slion_flush(tp, flag, 0);	
		break;

	case M_STOP:
		SL_printf("STP|");
	        old_intr = i_disable(INT_TTY);
		slionproc(tp,T_SUSPEND);
		tp->t_localstop = 1;
	        i_enable(old_intr);
		freemsg(mp);
		break;

	case M_START:
		SL_printf("STRT|");
	        old_intr = i_disable(INT_TTY);
		slionproc(tp,T_RESUME);
		tp->t_localstop = 0;
	        i_enable(old_intr);
		freemsg(mp);
		break;

	case M_STOPI:
		SL_printf("STPI|");
	        old_intr = i_disable(INT_TTY);
		slionproc(tp,T_BLOCK);
	        i_enable(old_intr);
		freemsg(mp);
		break;

	case M_STARTI:
		SL_printf("STRTI|");
	        old_intr = i_disable(INT_TTY);
		slionproc(tp,T_UNBLOCK);
	        i_enable(old_intr);
		freemsg(mp);
		break;

        /* M_PROTO messages: sent by sptr module with LPWRITE_REQ cmd*/
        case M_PROTO: 
	{
		mblk_t *mp1;   
		if (*(int *)mp->b_rptr != LPWRITE_REQ) 
		{
		    freemsg(mp);
		    break;
		}
		mp1 = unlinkb(mp); 	/* Save the M_PROTO block to be 
					 * enqueued at the end */
		while (mp1) 
		{
		    nmp = unlinkb(mp1);
		    putq(q,mp1);
		    mp1 = nmp;
		}
		putq(q,mp);
		break;
	}
	default:
		SL_printf("DF|");
		freemsg(mp);
		break;
	}
	Return(0);
} /* slionwput */

/*
 * FUNCTION: 	slionwsrv
 *
 * PURPOSE:	Driver write service routine.
 *
 * INPUT:
 *	q:	Output queue pointer.
 *
 * RETURN:
 *	0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_ioctl	slionservice	slionproc
 *
 * EXTERNAL PROCEDURES CALLED:
 *	getq		freemsg		i_disable 	i_enable
 *	putbq		untimeout
 *
 * DESCRIPTION:
 *	Get the message from the output queue. The IOCTLs are 
 *	handled here. The DATA blocks are set up for transmission 
 *	if the driver is not busy and previous msg is already free(ie:t_outmsg
 *	is 0). 
 *	The BREAK  is set or cleared based on the flag(ld->block). 
 *
 * CALLED BY:
 *	 After calling  putq() in slionwput.
 */
int 
slionwsrv(queue_t *q)
{
	DoingPio;
	str_lionp_t	  tp = (str_lionp_t)q->q_ptr;
	struct	sliondata *ld = (struct sliondata *) tp->t_hptr;
	mblk_t 		  *mp;
	int 		  old_intr;

	Enter(TRC_LION(TTY_WSRV), tp->t_dev, (int)q->q_ptr, q->q_count, 0, 0);

	SL_printf("\nSwsr.%x|",PRT(tp->t_dev));
	while (mp=getq(q)) 
	{
	     switch(mp->b_datap->db_type) 
	     {
		case M_DELAY:
			SL_printf("DLY|");
			if (!(tp->t_stop)) 
			{
			    if (tp->t_timeout)
				untimeout(tp->t_timeout);
			    tp->t_timeout = timeout(slion_timeout,
					(caddr_t) tp, *((int *)mp->b_rptr));
			    old_intr = i_disable(INT_TTY);
			    slionproc(tp,T_SUSPEND);
			    i_enable(old_intr);
			}
			freemsg(mp);
			break;

		case M_IOCTL:
			SL_printf("IOCT|");
			old_intr = i_disable(INT_TTY);
                        if (tp->t_busy)
                        {
                                putbq(q,mp);
                                i_enable(old_intr);
                                Return(0);
                        }
                        if (slion_ioctl(q,mp,0) > 0)
                        {
                                putbq(q,mp);
                                i_enable(old_intr);
                                Return(0);
                        }
                        i_enable(old_intr);
			break;

        	case M_BREAK:
			SL_printf("BRK|");
			old_intr = i_disable(INT_TTY);
                        if (tp->t_busy)
                        {
                                putbq(q,mp);
                                i_enable(old_intr);
                                Return(0);
                        }
                        i_enable(old_intr);
               		if (*(int *)mp->b_rptr)
			{
				SL_printf("set|");
				/* send a break */
				slion_break_set(tp, 0);
			}
               		else
			{
				SL_printf("clr|");
				/* clear the break */
				slion_break_clear(tp);
			}
               		freemsg(mp);
               		break;

		case M_DATA: 
		{
			SL_printf("DT|");
			old_intr = i_disable(INT_TTY); 
			if ((tp->t_stop) || (ld->flags & BUSY_MASK) ||
				 (tp->t_outmsg)  || (tp->t_busy))
			{
				SL_printf("stp|");
				putbq(q,mp);
				i_enable(old_intr);
				Return(0);
			}
			if(mp->b_wptr > mp->b_rptr) 
			{
			    ld->txcnt = (mp->b_wptr - mp->b_rptr);
			    ld->txbuf = mp->b_rptr;
			    tp->t_outmsg = mp; 		/*to free msg later */
    			    slionproc(tp,T_OUTPUT);	/* output the message*/
			    i_enable(old_intr);
			    break;
			}
			i_enable(old_intr);		/**txcnt=0*/
			freemsg(mp);
			break;
		}

          	/* Inform the serial printer module that its last 
		 * output request was terminated 
		 */
		case M_PROTO:
			/* last M_DATA not completely transmitted yet */
			if (tp->t_outmsg) 
			{
			    putbq(q,mp);
			    Return(0);
			}
			mp->b_datap->db_type = M_PCPROTO;
			*(int *)mp->b_rptr = LPWRITE_ACK;
			qreply(q, mp);
			break;

		default:
			freemsg(mp);
			break;

	     }/* end of switch */
	}/* end of while */

	/* All pending data at close time was drained down:
	 * wake up closing thread.
	 * This is needed here because the last can other than M_DATA.
	 */
	if (!(mp) && (tp->t_draining) && !tp->t_busy)
	{
	        old_intr = i_disable(INT_TTY);
		tp->t_draining = 0;
		TTY_WAKE(tp);
	        i_enable(old_intr);
	}

	SL_printf("ESwsrv|");					
	Return(0);

} /*slionwsrv */

/*
 * FUNCTION: 	slion_ctl
 *
 * PURPOSE:	M_CTL message process routine.
 *
 * INPUT:
 *      q:	Write queue pointer.
 *	mp:	Message pointer on queue including ioctl argument..
 *
 * RETURN:
 *	0:	Normal return.
 *	-1:	Message allocation error.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_allocmsg		slion_termios_set	slion_termix_set
 *	slion_termiox_get
 *
 * EXTERNAL PROCEDURES CALLED:
 *	bzero		freemsg			qreply
 *	printf
 *
 * DESCRIPTION:
 *      If successfully processed, the same  M_CTL is possibly sent 
 *	upstream, otherwise it is freed.
 *
 * CALLED BY:
 *	slionwput
 */
int
slion_ctl(queue_t *q, mblk_t *mp)
{
	register str_lionp_t	tp = (str_lionp_t)q->q_ptr;
	mblk_t		  *mp1;
	struct iocblk	  *iocp;				/*stream.h*/
	struct termios	  *tios;
	struct termiox	  *tiox;
	struct sliondata  *ld = tp->t_hptr;
	int 		  signal_temp;
	int 		  curr_stat;

	iocp = (struct iocblk *)mp->b_rptr;

	SL_printf("S_ctl(%x)|",iocp->ioc_cmd);
	switch (iocp->ioc_cmd) 
	{
	    case TIOCGETA:
		SL_printf("TGETA|");
		if ((mp1 = mp->b_cont) &&
		    (mp1->b_datap->db_lim - mp1->b_rptr >= 
						sizeof(struct termios))) 
                {
                        tios= (struct termios *)mp1->b_rptr;
                        bcopy((caddr_t) &(tp->t_termios),
                                (caddr_t) tios, sizeof(struct termios));
                        mp1->b_wptr = mp1->b_rptr + sizeof( struct termios);
                }
		else 
		{
			freemsg(mp);
			return(0);
		}
		break;
 
	    case TIOC_REQUEST:
	    case TIOC_REPLY:
		{
			register int reply_size = sizeof(slion_tioc_reply);

			SL_printf("RQ/RPLY|");
			iocp->ioc_cmd = TIOC_REPLY;
			if (!(mp1 = allocb(reply_size, BPRI_MED)))
			    break; 	
			iocp->ioc_count = reply_size;
			bcopy(slion_tioc_reply, mp1->b_rptr, reply_size);
			mp1->b_wptr = mp1->b_rptr + reply_size;
			mp->b_cont = mp1;
			break;
		}

	    case TIOCGETMODEM:
		SL_printf("GMDM|");
                if((mp1 = mp->b_cont) &&
                    (mp1->b_datap->db_lim - mp1->b_rptr >= sizeof(char)))
                {
                        *((char *) mp1->b_rptr) =
                                 ((struct sliondata *) tp->t_hptr)->prms->dcd;
                        mp1->b_wptr = mp1->b_rptr + sizeof(char);
                }
		else 
		{
			freemsg(mp);
			return(0);
		}
		break;

	    case TCGETX:
		SL_printf("GTX|");
		if ((mp1 = mp->b_cont) &&
		    (mp1->b_datap->db_lim - mp1->b_rptr >= 
						sizeof(struct termiox))) 
		{
			tiox = (struct termiox *)mp1->b_rptr;
			bzero((char * )tiox,sizeof(struct termiox));
			slion_termiox_get(tp,tiox);	
			mp1->b_wptr = mp1->b_rptr + sizeof( struct termiox);
		} 
		else 
		{
			freemsg(mp);
			return(0);
		}
		break;

      	    case MC_CANONQUERY:
		if ((mp1 = mp->b_cont) &&
		    (mp1->b_datap->db_lim - mp1->b_rptr >= 
						sizeof(struct termios))) 
		{
			tios = (struct termios *)mp1->b_rptr;
			bzero((char * )tios,sizeof(struct termios));
			tios->c_iflag |= (IXON | IXOFF | IXANY);
			iocp->ioc_cmd = MC_PART_CANON;
			mp1->b_wptr = mp1->b_rptr + sizeof( struct termios);
        	} 
		else 
		{
			freemsg(mp);
			return(0);
        	}
        	break;

      case TXTTYNAME:
                SL_printf("TYNAME|");

		if (!(mp1 = mp->b_cont) ||
		    (mp1->b_datap->db_lim - mp1->b_rptr < TTNAMEMAX)) {
			freemsg(mp);
			return(0);
		}
                bcopy(tp->t_name,(char *)mp1->b_rptr,sizeof(tp->t_name));
		mp1->b_wptr = mp1->b_rptr + sizeof(tp->t_name);
                break;

      case TIOCMGET:
		if ((mp1 = mp->b_cont) &&
		    (mp1->b_datap->db_lim - mp1->b_rptr >= sizeof(int))) 
		{
			signal_temp = 0;
			curr_stat = 0;
			slionservice(tp, TS_GCONTROL, &curr_stat);
			if (curr_stat & TSDTR) 
				signal_temp |= TIOCM_DTR;
			if (curr_stat & TSRTS) 
				signal_temp |= TIOCM_RTS;

			curr_stat = 0;
			slionservice(tp, TS_GSTATUS, &curr_stat);
			/* no check:DSR/RI not supported */
			if (curr_stat & TSCD) 
				signal_temp |= (TIOCM_CAR | TIOCM_DSR);
			if (curr_stat & TSCTS) 
				signal_temp |= TIOCM_CTS;
            		*((int *)mp1->b_rptr) = signal_temp;
			mp1->b_wptr = mp1->b_rptr + sizeof(int);
		} 
		else 
		{
			freemsg(mp);
			return(0);
		}
		break;

	default:
		SL_printf("DF|");
		freemsg(mp);
		return(0);
	}
	SL_printf("ESctl|");
	qreply(q,mp);
	return(0);

} /* slion_ctl */

/*
 * FUNCTION: 	slion_timeout
 *
 * PURPOSE:	Reset the flag t_timeout in the str_lion struct.
 *
 * INPUT:
 *	tp:	Pointer to str_lion structure.
 * 
 * Return 
 *	0:	Always.
 *
 * DESCRIPTION:
 *      Function argument to timeout utility. The flag t_timeout is reset in
 *	str_lion structure. If T_SUSPEND flag(t_stop) is ON, this flag
 * 	is reset via lionproc with T_RESUME command.
 *
 * CAlled by:
 *	 Via timer in slionwsrv(M_DELAY).
 */

int
slion_timeout(str_lionp_t tp)
{
	int	old_intr;

	/* Warning! for MP safety, must be an atomic primitive */
	tp->t_timeout = 0;
		      
	/* timeout call is due to M_DELAY processing */
	if (tp->t_stop) 
	{
		old_intr = i_disable(INT_TTY);
		slionproc(tp,T_RESUME);
		i_enable(old_intr);
	}    
	return(0);

} /* slion_timeout */

/*
 * FUNCTION: 	slion_allocmsg
 *
 * PURPOSE:	Allocate a  receive data message block.
 *
 * INPUT:
 *	tp:	Pointer to str_lion structure.
 * 
 * RETURN: 
 *	0:	Not allocated.
 *	mp:	Allocated message address.
 *
 * DESCRIPTION:
 *		Allocate a new message for received input. If allocation is 
 *		failed, bufcal() is called to recover from failure of 
 *		allocb().
 * CALLED BY:
 *	slionopen	slion_recover		_slion_recev
 */
mblk_t *
slion_allocmsg(str_lionp_t tp)
{
	register mblk_t 	*mp;
	/* Already allocated?*/	
	if(mp = tp->t_inmsg)
		return(mp);

	if (mp = allocb(SLRDBUFSZ, BPRI_MED))
	{
		SL_printf("Almsg|");
		mp->b_datap->db_type = M_DATA;
		tp->t_inmsg = mp;
		return(mp);
	}
	else 
	{
	    SL_printf("Alc-bfcl|");
            if (!(tp->t_bufcall)) 
	    {
    	        if (!(tp->t_bufcall = bufcall(SLRDBUFSZ, BPRI_MED,
			                      slion_recover, (caddr_t)tp))) 
		{
                   if (!(tp->t_alloctimer))
		       tp->t_alloctimer = timeout(slion_allocmsg,
							(caddr_t)tp, hz);
		   return(0);    
		} 
		else 
		{    
		   /* bufcall request recorded: give up calling it */
                   if (tp->t_alloctimer) 
		   {
		       untimeout(tp->t_alloctimer);
                       tp->t_alloctimer = 0;
                   }
                }
            }
	}
	return (0);

} /* slion_allocmsg */

/*
 * FUNCTION: 	slion_recover
 *
 * PURPOSE:	Recover of bufcall().
 *
 * INPUT:
 *	tp:	Pointer to str_lion structure.
 * 
 * RETURN: 
 *	0:	Always.
 *
 * DESCRIPTION:
 *      Called through bufcall utility to recover allocb failure.
 *
 * CALLED BY:
 *	slion_allocmsg
 */
int
slion_recover(str_lionp_t tp)
{
	register mblk_t *mp;
	struct	 sli_adap  *adap = adap_list[ADP(tp->t_dev)];
	
	tp->t_bufcall = 0;

	/* If allocb fails, try again, Forever ?! */
	if (mp = slion_allocmsg(tp))
	{
		mp->b_datap->db_type = M_DATA;
		tp->t_inmsg = mp;
		return(0);
	}
	if (tp->t_sched) 
	{
		tp->t_sched = 0;
		i_sched((struct intr *)&adap->intr);
	}
	return(0);

} /* slion_recover */

/*
 * FUNCTION: 	slion_termiox_set
 *
 * PURPOSE:	Set the current  hardware flow control parameters based 
 *		on the given termiox. 
 *		structure.
 *
 * INPUT:
 *	tp:	Pointer to str_lion structure.
 *	tios:	Pointer to termiox structure.
 *
 * RETURN:
 *	 0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_comm	
 *
 * DESCRIPTION:
 *	This routine sets the current hardware flow control parameters 
 *	based on the given termiox structure.
 *
 * CALLED BY:
 *	slion_ioctl(TCSETX TCSETXW TCSETXF).
 */
int
slion_termiox_set(str_lionp_t tp, struct termiox * tiox)
{
	struct sliondata  *ld = tp->t_hptr;

	SL_printf("S.trmx.S|");	

	if((tiox->x_hflag & RTSXOFF) != (ld->prms->rtsp))
	{
		/* Select the rts local pacing */
		if(tiox->x_hflag & RTSXOFF)
		{
			SL_printf("RTSXOFF on|");

			slion_comm(ld, RTS1);
			ld->prms->rtsp = 1;
		}
		else
		{
			SL_printf("RTSXOFF off|");

			slion_comm(ld, RTS0);
			ld->prms->rtsp = 0;
		}
	}
	
	if((tiox->x_hflag & CTSXON) != (ld->prms->ctsp))
	{
		/* Select the dtr remote pacing */
		if(tiox->x_hflag & CTSXON)
		{
			SL_printf("CTSXON on|");

			slion_comm(ld, CTS1);
			ld->prms->ctsp = 1;
		}
		else
		{
			SL_printf("CTSXON off|");

			slion_comm(ld, CTS0);
			ld->prms->ctsp = 0;
		}
	}

	if((tiox->x_hflag & DTRXOFF) != (ld->prms->dtrp))
	{
		if(tiox->x_hflag & DTRXOFF)
		{
			SL_printf("DTRXOFF on|");

			slion_comm(ld, DTR1);
			ld->prms->dtrp = 1;
		}
		else
		{
			SL_printf("DTRXOFF off|");

			slion_comm(ld, DTR0);
			ld->prms->dtrp = 0;
		}
	}
	
	if((tiox->x_hflag & CDXON) != (ld->prms->dcdp))
	{
		if(tiox->x_hflag & CDXON)
		{
			SL_printf("CDXON on|");	

			slion_comm(ld, DCD1);
			ld->prms->dcdp = 1;
		}
		else
		{
			SL_printf("CDXON off|");

			slion_comm(ld, DCD0);
			ld->prms->dcdp = 0;
		}
	}
	return(0);

} /* slion_termiox_set */

/*
 * FUNCTION: 	slion_termiox_get
 *
 * PURPOSE:	Get the current hardware flow control parameters.
 *
 * INPUT:
 *	tp:	Pointer to str_lion structure.
 *
 * OUTPUT
 *	tiox:	Pointer to the termiox structure.
 *
 * RETURN:
 *	 0:	Always.
 *
 * DESCRIPTION:
 *	This routine gets the hardware flow control parameters,
 *	and writes it into the given termiox structure.
 *
 * CALLED BY:
 *	slion_ioctl(TCGETX)
 *	slion_ctl(TCGETX)
 */
int
slion_termiox_get(str_lionp_t  tp,struct termiox * tiox)
{
	struct sliondata  *ld = tp->t_hptr;

	SL_printf("S.trmx.G|");

	if(ld->prms->rtsp)
		tiox->x_hflag |= RTSXOFF;
	if(ld->prms->ctsp)
		tiox->x_hflag |= CTSXON;
	if(ld->prms->dtrp)
		tiox->x_hflag |= DTRXOFF;
	if(ld->prms->dcdp)
		tiox->x_hflag |= CDXON;
	tiox->x_sflag = tp->t_termiox.x_sflag;
	return(0);

} /* slion_termiox_get */

/*
 * FUNCTION: 	slion_termios_set
 *
 * PURPOSE:	Set the current parameters based on the given termios 
 *		structure.
 *
 * INPUT:
 *	tp:	Pointer to str_lion structure.
 *	tios:	Pointer to termios structure.
 *
 * RETURN:
 *	0:	On success.
 *	-1:	Value is out of range.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slionservice	
 *
 * DESCRIPTION:
 *	This routine sets the bauderate, number of bits per character,
 *	parity,  stop bits and software flowcontrol from the given 
 *	termios structure to the terminal.
 *
 * CALLED BY:
 *	slion_ioctl(TIOCSETA TIOCSETAW TIOCSETAF)
 */

int
slion_termios_set(str_lionp_t  tp,struct termios * tios)
{
	int 		 NewBPC;
	int 		 NewParity;
	int 		 NewStops;
	struct sliondata *ld = (struct sliondata *)tp->t_hptr;

	SL_printf("Trms.S|");

	if(((cfgetospeed(tios) == cfgetispeed(tios)) || 
		(!cfgetospeed(tios)) || (!cfgetispeed(tios))) && 
		(ld->prms->baud !=  compatspcodes[cfgetospeed(tios)])) 
	{
	   slionservice(tp,TS_SBAUD, (void *)compatspcodes[cfgetospeed(tios)]);
	}

	switch (tios->c_cflag & CSIZE) 
	{
		case CS5:
			NewBPC = bits5;
			break;
		case CS6:
			NewBPC = bits6;
			break;
		case CS7:
			NewBPC = bits7;
			break;
		case CS8:
			NewBPC = bits8;
			break;
		default:
			return(-1);
	}
	if(ld->prms->char_size != NewBPC)
		slionservice(tp, TS_SBPC, (void *)NewBPC);

	NewStops = (tios->c_cflag & CSTOPB)? stop2: stop1;
	if(ld->prms->stop_bits != NewStops)
		slionservice(tp, TS_SSTOPS, (void *)NewStops);
	
	if(tios->c_cflag & PARENB)
	{
             if(tios->c_cflag & PAREXT)
                NewParity = (tios->c_cflag&PARODD)?markpar:spacepar;
             else
                NewParity = (tios->c_cflag&PARODD)?oddpar:evenpar;
        }
	else
		NewParity = (tios->c_cflag&PARODD)?oddonly:nopar;
		
	if(ld->prms->parity != NewParity)
		slionservice(tp, TS_SPARITY, (void *)NewParity);

	/* Update the CLOCAL flag */
	if(tios->c_cflag & CLOCAL) 
		tp->t_clocal = 1;
	else 
		tp->t_clocal = 0;


	/* Update the CREAD flag */
	if(tios->c_cflag & CREAD) 
		tp->t_cread = 1;
	else 
		tp->t_cread = 0;

	 /* Update the HUPCL flag */
	 if(tios->c_cflag & HUPCL) 
		tp->t_hupcl = 1; 
	 else 
		tp->t_hupcl = 0; 

	/* Set the xon/xoff remote/local flow control modes */
	if((tios->c_iflag & IXOFF) != (ld->prms->xoff))  	
	{
		/* Select xon local pacing */
        	if (tios->c_iflag & IXOFF)    /* XON i/p pacing requested */
		{
			slionservice(tp, TS_SOFTPACE, (void *)SOFTPACE_LON);

			/* Set the control characters, assuming that VSTART, 
			 * VSTOP  are successive 
			 */
			slionservice(tp, TS_SOFTLCHAR, &(tios->c_cc[VSTART]));
		}
		else
			slionservice(tp, TS_SOFTPACE, (void *)SOFTPACE_LOFF);
	}

	if((tios->c_iflag & IXON) != (ld->prms->xonp))  	
	{ 	
		/* Select the xon remote pacing */
        	if (tios->c_iflag & IXON) 	/* xon o/p pacing requested*/
		{
			slionservice(tp, TS_SOFTPACE, (void *)SOFTPACE_RON);

			/* Set the control characters,same as those for 
			 * local pacing, assuming that VSTART, VSTOP are 
			 * successive. 
			 */
			slionservice(tp, TS_SOFTRCHAR, &(tios->c_cc[VSTART]));
		}
		else
			slionservice(tp, TS_SOFTPACE, (void *)SOFTPACE_ROFF);
	}

	if((tios->c_iflag & IXANY) != (ld->prms->xany))  	
	{ 	
		/* Select the xon remote pacing */
        	if (tios->c_iflag & IXANY) 	/* xon o/p pacing requested*/
		{
			slionservice(tp, TS_SOFTPACE, (void *)SOFTPACE_RANY);

			/* Set the control characters,same as those for 
			 * local pacing, assuming that VSTART, VSTOP are 
			 * successive. 
			 */
			slionservice(tp, TS_SOFTRCHAR, &(tios->c_cc[VSTART]));
		}
		else
		{
			/* Don't turn off output flow control unless 
			 * it is requested for both IXON and IXANY 
			 */
			if(!(ld->prms->xonp & IXON))
			   slionservice(tp, TS_SOFTPACE, (void *)SOFTPACE_ROFF);
		}
	}

	SL_printf("(%x %x %x %x)",
		tios->c_iflag, tios->c_oflag,
		tios->c_cflag, tios->c_lflag);

	return(0);

} /* slion_termios_set */

/*
 * FUNCTION:    slion_termios_get
 *
 * PURPOSE:     Get the current terminal parameters in termios structure.
 *
 * INPUT:
 *      tp:     Pointer to str_lion structure.
 *
 * OUTPUT
 *      tios:   Pointer to termios structure.
 *
 * RETURN:
 *      0:   On success.
 *
 *
 * DESCRIPTION:
 *	This routine gets the bauderate, number of bits per character,
 *	parity and stop bits in the current parameters of terminal and
 *	writes it into the given termios structure.
 *
 * CALLED BY:
 *	slion_ioctl(TIOCGETA)	
 *	slion_ctl(TIOCGETA)
 */
int
slion_termios_get(str_lionp_t tp, struct termios * tios)
{
	tcflag_t   	 speed;
	int	   	 ii;
	struct sliondata *ld = (struct sliondata *)tp->t_hptr;

	SL_printf("S.trms.G|");

	for(ii=0;(ii<=15 && compatspeeds[ii].sp_speed !=ld->prms->baud);ii++);
	if (ii <= 15) 
	{ 
		speed =  compatspeeds[ii].sp_code;
		cfsetospeed(tios, speed);
		cfsetispeed(tios, speed);
	}

	switch (ld->prms->char_size) 
	{
		case bits5:
			tios->c_cflag |= CS5;
			break;
		case bits6:
			tios->c_cflag |= CS6;
			break;
		case bits7:
			tios->c_cflag |= CS7;
			break;
		case bits8:
			tios->c_cflag |= CS8;
			break;
		default:
			return(-1);
	}
	/* 0:stop 1 1:stop 1.5 2: stop 2 */
	switch (ld->prms->stop_bits) 
	{
		case stop2+1:                   /*x2*/
			tios->c_cflag |= CSTOPB;
			break;
		case stop1:                      /*x0*/
			tios->c_cflag &= ~CSTOPB;
			break;
		default:
			return(-1);
	}
	
	switch (ld->prms->parity) 
	{
		case nopar:
			tios->c_cflag &= ~PARENB;
			break;
		case oddpar:
			tios->c_cflag |= (PARENB | PARODD);
			break;
		case evenpar:
			tios->c_cflag |= PARENB;
			tios->c_cflag &= ~PARODD;
			break;
		case oddonly:
			tios->c_cflag &= ~PARENB;
			tios->c_cflag |= PARODD;
			break;
		case markpar:
                        tios->c_cflag |= PAREXT | PARENB | PARODD;
                        break;
                case spacepar:
                        tios->c_cflag |= PAREXT | PARENB;
                        break;
		default:
			return(-1);
	}
	if (tp->t_clocal)
		tios->c_cflag |= CLOCAL;

	if (tp->t_cread) 
		tios->c_cflag |= CREAD;
	
	if (tp->t_hupcl) 
		tios->c_cflag |= HUPCL;

	/* Get c_iflag */
	if(ld->prms->xoff)
		tios->c_iflag |= IXOFF;
	if(ld->prms->xonp)
		tios->c_iflag |= IXON;
	if(ld->prms->xany)
		tios->c_iflag |= IXANY;

	tios->c_cc[VSTART] = ld->prms->lxac;	/* Same as lxac */
	tios->c_cc[VSTOP] = ld->prms->lxoc;	/* Same as lxoc */

	SL_printf("(%x %x %x %x)",
		tios->c_iflag, tios->c_oflag,
		tios->c_cflag, tios->c_lflag);

	return(0);

} /* slion_termios_get */

/*
 * FUNCTION: 	slion_break_set
 *
 * PURPOSE:	Sets break condition for the requested duration.
 *
 * INPUT:
 *      tp:      Pointer to str_lion structure.
 *      duration Time for which the break condition is set.
 *
 * RETURN:
 *	0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slionservice 
 *
 * EXTERNAL PROCEDURES CALLED:
 *	timeout	
 *
 * DESCRIPTION:
 *		Sets break condition via slionproc() for the requested
 *		duration.	
 * CALLED BY:
 *      slion_ioctl(TCSBREAK)   slionwsrv(M_BREAK)
 */
int 
slion_break_set(str_lionp_t tp, int duration)
{
	slionservice(tp,TS_SBREAK,0);
	if (duration && (!tp->t_brktimeout)) 
	{
            tp->t_brktimeout = timeout(slion_break_clear, (caddr_t)tp, 
								duration);
	}
	return(0);
}

/*
 * FUNCTION: 	slion_break_clear
 *
 * PURPOSE:	Clears break condition.
 *
 * INPUT:
 *      tp:     Pointer to str_lion structure.
 *
 * RETURN:
 *	0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slionservice 
 *
 * DESCRIPTION:
 *		Clear break condition after the timeout  of the 
 *		specified duration, which is set by slion_break_set().
 *		Then answer back with ACK message to upper stream.
 * CALLED BY:
 *       Timer via slion_set_break      slion_ioctl. 
 *
 */
int 
slion_break_clear(str_lionp_t tp)
{
	mblk_t	*mp;			/* See note with TCSBRK processing */

	SL_printf("Cbrk|");
	tp->t_brktimeout = 0;
	slionservice(tp,TS_CBREAK,0);
	if (mp = tp->t_ioctlmsg) 
	{
		SL_printf("ACK|");
		tp->t_ioctlmsg = 0;
		mp->b_datap->db_type = M_IOCACK;
		((struct iocblk *)(mp->b_rptr))->ioc_count = 0;
		qreply(WR(tp->t_ctl), mp);
	}
	return(0);

} /* slion_break_clear */


/*
 * FUNCTION:	slion_flush
 *
 * PURPOSE:	Flush read and write queuese.
 *
 * INPUT:
 *      tp:      Pointer to str_lion structure.
 *      rw:      FLUSHW or FLUSHR flags.
 *      recycle: if set, sent an M_FLUSH with FLUSHR.
 *
 * RETURN:
 *	0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slionproc
 *
 * EXTERNAL PROCEDURES CALLED:
 *	flushq		putcltl1
 *
 * DESCRIPTION:
 *	Flush read and write queue 
 *	Flush ouput data in the driver.
 *
 * CALLED by:
 *	slionwput	slionclose
 */
static int
slion_flush(str_lionp_t tp,int rw, int recycle)
{
	int	old_intr;

	SL_printf("S_fls|");

	if (rw & FLUSHW) 
	{
		flushq(WR(tp->t_ctl),FLUSHDATA);
		old_intr = i_disable(INT_TTY);
		slionproc(tp,T_WFLUSH);
		i_enable(old_intr);
	}
	if (rw & FLUSHR) 
	{
		flushq(tp->t_ctl,FLUSHDATA);
		if (recycle)
		    putctl1(tp->t_ctl->q_next,M_FLUSH,FLUSHR);
		old_intr = i_disable(INT_TTY);
		slionproc(tp,T_RFLUSH);
		i_enable(old_intr);
	}
	return(0);

} /* slion_flush */

/*
 * FUNCTION: 	myttyinput
 *
 * PURPOSE:	Alloc and put a control block on the read queue.
 *
 * INPUT:
 *      tp:     Pointer to str_lion structure.
 *	q:	Pointer to read queue.
 *	cc:	Character to be processed.
 *	ss:	status of character.
 *
 * RETURN:
 *	0:	On success.
 * ENOMEM:	Failed(no message block).
 *
 * EXTERNAL PROCEDURES CALLED:
 *	allocb		putq		freemsg
 *
 * DESCRIPTION:
 *	This function allocates a control block for M_CTL and writes the
 *	specified status in this block.
 *	if this block cannot be put on the queue, it is freed and return 
 *	to ENOMEM. 
 *
 * CALLED BY:
 *	slion_offlev	sGotHilink
 */
static int  
myttyinput(str_lionp_t tp, queue_t *q, enum status ss)
{
	mblk_t	*mp;		/* To contain currently transmitted message */
	int	err=0;

	SL_printf("mtyip(%x)",ss);

	/* If not opened, q is 0 or in close processing*/
	if(!q || (!(tp->t_isopen) && !(tp->t_wopen)))
		return(0);

	if(mp = allocb(sizeof(enum status),BPRI_MED))
	{
	   mp->b_datap->db_type = M_CTL;
	   if (tp->t_OpenRetrieve) 
	   {
		if(err=openDisc_input(tp->t_OpenRetrieve,&tp->t_event,0,ss)) 
		{
			SL_printf("ERR of input open discipline.\n");
	    		slion_err(tp->t_name, ERRID_TTY_BADINPUT, err); 
		}
	   }
	   /* if carrier lost while sleeping in srsclose() waiting for
            * the data to drain out, wake up the closing process
            */
           if((ss == cd_off) && tp->t_draining)
           {
                tp->t_draining = 0;
                TTY_WAKE(tp);
           }
	   *(enum status *)mp->b_rptr = ss;
	   mp->b_wptr = mp->b_rptr+ sizeof (enum status);

	   /* Don't need canput testing since M_CTL is a
	    * high priority message */
	   putq(q,mp);
	}
	else
	   return(ENOMEM);

	return(0);

} /*myttyinput */

/*
 * FUNCTION: 	strttyinput
 *
 * PURPOSE:	Write a character on the data message block.
 *
 * INPUT:
 *	q:	Pointer to read queue.
 *	mp:	Pointer to  message block.
 *	c:	Character to be processed.
 *	status:	Status of character.
 *      tp:     Pointer to str_lion structure.
 *
 * RETURN:
 *	 0:	Always.
 *   ENOMEM:	Failed(no M_BREAK message block).
 *
 * EXTERNAL PROCEDURES CALLED:
 *	allocb		putq
 *
 * DESCRIPTION:
 *	When the status is good_char and RX message block space is enough, 
 *	an char(c) is written on the given message block.
 *	If not enough space, an new space is allocated.  
 *	Othercase(error character), the already collected data is send up
 *	then  an message block is allocated for M_BREAK and it is put on 
 *	the read queue. 
 *	In the case of allocation fail, return ENOMEM2 for M_BREAK message .
 *
 * CALLED BY:
 *	_slion_recv(macro CHECK_ERROS)
 */
static int
strttyinput(queue_t *q, mblk_t *mp, char c, enum status status,str_lionp_t tp)
{
	mblk_t *mp1;

#ifdef SLI_TRACE
  if(status)
    SL_printf("ttyip(%x %x)",c,status);
#endif

    switch (status) 
    {
	case good_char:					/*0x00*/
	    tp->t_rxcnt++;
	    *((mp->b_wptr)++) = c;
	    /* Check space of msg blk */
	    if(tp->t_rxcnt >= SLRDBUFSZ)
	    {
		/* Is ldterm Q already allocated? 
		 * Send chars directly up if nothing
		 * else on read queue */
		if(!(q->q_first) && canput(q->q_next) && (q->q_next)->q_ptr)
			putnext(q, mp);
		else
			putq(q, mp);
		tp->t_inmsg = 0;
		tp->t_rxcnt = 0;
		slion_allocmsg(tp);
	    }
	    break;

	case framing_error:				/*0x03*/
	case parity_error:				/*0x02*/
	case overrun:					/*0x01*/
	case break_interrupt:				/*0x04*/
	    SL_printf("Erch|");

	    /* send up already collected data before notifying status change */
	    if(tp->t_rxcnt)
	    {
		putq(q, mp);
		tp->t_inmsg = 0;
		tp->t_rxcnt = 0;
		slion_allocmsg(tp);
	    }

	    /* Put the error condition inside an M_BREAK and queue 
	     * it up, if possible 
	     */
	    if(mp1 = allocb(COMM_SIZE,BPRI_MED))
	    {
		mp1->b_datap->db_type = M_BREAK;
		*(enum status *)mp1->b_wptr = status;
		mp1->b_wptr = mp1->b_wptr+ sizeof (enum status);
		*(mp1->b_wptr++) = c;
		putq(q,mp1);
	    }
	    else
		return(ENOMEM);

	default:
	    ;
    }
    return(0);

} /* strttyinput */


/******************************************************************
 *	Upper interface:	corresponds to lion.c
 ******************************************************************
*/

/*
 * FUNCTION: 	slionconfig
 *
 * PURPOSE:	Stream lion driver configuration routine.
 *
 * INPUT:
 *      dev:	Device number to  (un)configure.
 *      cmd:	Command(CFG_INIT, CFG_TERM or CFG_QVPD).
 *      uiop:	Pointer to uio stucture containing the DDS.
 *
 * RETURN:
 *	0:	On succes.
 *
 *  EINVAL:	DDS format or type error.
 *  EBUSY:	Adapter is already configured.
 *  ENODEV:	Device number error.
 *  err:	Result of slionadd()/ureadc().
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_err	slionadd	sliondel	sxu_open
 *	sxu_getpos	sxu_close	slionadd1		
 *
 * EXTERNAL PROCEDURES CALLED:
 *	uiomove		lockl		
 *	printf		ureadc		io_det
 *	str_install	unlockl	
 *
 * DESCRIPTION:
 *      This is the external entry point to the driver.
 *      It is called to configure, and unconfigure adappters and lines, and
 *      to query the Vital Product Data.
 *      One structure per new adapter name is allocated. When a line is
 *      configured, these structures are used to really configure the card.
 *      The configuration of a card is actually made only at the first
 *      configuration of one of its lines at the CFG_INIT. In the case
 *	of CFG_TERM, When the last line under the adapter was requested,
 *	the resource of the adapter will be released.
 *
 *
 * CALLED BY:
 *	System Configurater.
 *
 */
slionconfig(dev_t dev, int cmd, struct uio *uiop)
{
    DoingPio;
    int			 nodev();
    static int		 lock = LOCK_AVAIL;
    int			 ret, result, rcd,index;
    int 		 min, maj;
    dev_t		 newmin, chkmin;
    struct sli_adap 	 *adap;
    struct adapters 	 *adp_typ;	     /* Ptr to an entry of adap_types*/
    enum   dds_type	 which_dds;          /* DDS id. for CFG_TERM */
    struct lion_adap_dds adap_dds;	     /* Current adap_dds area */ 
    struct lion_line_dds line_dds;	     /* Current line_dds area */
    struct adap_conf 	 *current_adap_conf; /* Ptr to a saved DDS list*/
    struct adap_conf 	 *tmp_adap_conf;     /* Pointer for new allocation*/
    struct sliondata 	 *ld;
    chan_t 		 chan; 
    str_lionp_t	  	 tp;

#ifdef TTYDBG
    int			 ttydbg_error = 0;
    struct tty_to_reg	 slion_tty_to_reg = { dev, "", "lion", 0 };
#endif /* TTYDBG */

    Enter(TRC_LION(TTY_CONFIG), dev, 0, cmd, 0, 0);

    chan = CHN(dev);			     /* get channel from dev */

    ret = lockl(&lock, LOCK_SHORT);

    SL_printf("\nSconfig|");
    switch (cmd)
    {
      case CFG_INIT:

	SL_printf("CFG_INIT(P%x A%x L%x)",uiop->uio_resid,
		sizeof(struct lion_adap_dds),
			sizeof(struct lion_line_dds));

	switch (uiop->uio_resid) 
	{
          case sizeof(struct lion_adap_dds):	/* Adpater configuration */
	
		SL_printf("A_DDS|");

        	if (result = uiomove((unsigned char *)&adap_dds, 
			sizeof(struct lion_adap_dds), UIO_WRITE, uiop)) 
		{
            		slion_err(resource, ERRID_COM_CFG_UIO, result);
            		break;
        	}
        	if (adap_dds.which_dds != LION_ADAP_DDS) 
		{
           		slion_err(resource, ERRID_COM_CFG_RESID,
							uiop->uio_resid);
           		result = EINVAL;
           		break;
        	};

		/* Check if adapter is known */
	    	for (adp_typ=adap_types ; adp_typ->type != Unknown_Adap && 
				adp_typ->type != adap_dds.lc_type; ++adp_typ);

		/* If Unknown_Adap is found means that the given adapter 
		 * is unknown 
		 */
		if (adp_typ->type == Unknown_Adap) 
		{
			slion_err(adap_dds.lc_name, ERRID_COM_CFG_UNK, 
							adap_dds.lc_type);
           		result = EINVAL;
           		break;
		}

		/* Check to see if the adapter is already configured */
		current_adap_conf = adap_conf_ss;
		while (current_adap_conf &&
		   strcmp(current_adap_conf->dds.lc_name, adap_dds.lc_name)) 
		{
			current_adap_conf = current_adap_conf->next;
		}
		if (current_adap_conf) 
		{
			/* Line must not be currently configured. */
			index = 0;
			while(index < adp_cnt)
                        {
                          if(adap_list[index] &&
                             !strcmp(adap_list[index]->adap_name,
                                adap_dds.lc_name))
                          {
                            /*
                             * we have found an entry for this adapter. we will
                             * exit this loop with index < adap_cnt. that will
                             * tell the code below that this adapter is already
                             * configured.
                             */
                            break;
                          }
                          /* look next entry */
                          index ++;
                        }
			/* If no line is found, I can update existing adapter
			 * configuration informations. Otherwise, I return 
			 * an error.
			 */
			if (index >= adp_cnt) 
			{
			     /* Clean before writing new values */
			     bzero((char *)&(current_adap_conf->dds),
					sizeof(current_adap_conf->dds));
			     bcopy((char *)&adap_dds, 
					(char *)&(current_adap_conf->dds),
					sizeof(current_adap_conf->dds));
			}
			else 
			{
			     SL_printf("Some ports are still configured\n");
			     result = EBUSY;
			     slion_err(adap_dds.lc_name,ERRID_COM_CFG_PORT,0);
			     break;
			} 
            	}
		else 
		{
			/* Create one adap DDS area  to save.*/
                	if (tmp_adap_conf = (struct adap_conf *)
					pinmalloc(sizeof(struct adap_conf))) 
			{
			     /* Clears the struct */
			     bzero((char *)tmp_adap_conf, 
					sizeof(struct adap_conf));
			     /* Save current dds to a create area(only 
			      * dds part) 
			      */
			     bcopy((char *)&adap_dds, 
					(char *)&(tmp_adap_conf->dds),
					sizeof(tmp_adap_conf->dds));
			     /* Updates adapter  dds configuration list */
			     current_adap_conf = adap_conf_ss;
			     adap_conf_ss = tmp_adap_conf;
			     tmp_adap_conf->next = current_adap_conf;
                	}
			else 
			{
			     SL_printf("Can't malloc adap_conf\n");
			     slion_err(adap_dds.lc_name, ERRID_RS_MEM_EDGE, 0);
			     result = ENOMEM;
			     break;
                	} /* End if (tmp_adap_conf = ...) */
		} /* End if (current_adap_conf) */
		break;
            
	  case sizeof(struct lion_line_dds):	/* Line configuration */
		SL_printf("L_DDS|");

		if (result = uiomove((unsigned char *)&line_dds, 
			sizeof(struct lion_line_dds), UIO_WRITE, uiop)) 
		{
			slion_err(resource, ERRID_COM_CFG_UIO, result);
			break;
		};

		if (line_dds.which_dds != LION_LINE_DDS) 
		{
			slion_err(resource, ERRID_COM_CFG_RESID, 
							uiop->uio_resid);
			result = EINVAL;
			break;
		};

		/* When the first line of an adapter is initialized, 
		 * declare to the Streams framework.
		 */
		if (!adap_list) 
		{

			SL_printf("STRLOAD");

			slionconf.sc_major = major(dev);
			if (result = str_install(STR_LOAD_DEV,&slionconf)) 
			{
			    SL_printf("After str_install:err rtn cd=%d\n",
			    			"str_install",result);
			    slion_err(line_dds.line_name, ERRID_COM_CFG_DEVA, 
							result);
			    break;
			}

#ifdef TTYDBG
			/* Registering the driver into the ttydbg extension */
			ttydbg_error = tty_db_register(&slion_str_module_conf);
#endif  /* TTYDBG */

	    	}

		/* To verify if adapter (parent) is configured. */
		current_adap_conf = adap_conf_ss;
		while (current_adap_conf &&
			strcmp(current_adap_conf->dds.lc_name, 
						line_dds.adap_name)) 
		{
			current_adap_conf = current_adap_conf->next;
		}
		if (!current_adap_conf) 
		{
			SL_printf("Adapter(%s) not found as configured\n",
				line_dds.adap_name);
			result = ENOTREADY;
			break;
		};

		/* Try to find the tty structure. If on success, the 
		 * adapter has been configured.  Otherwise call 
		 * slionadd to setup the adapter. 
		 */
		if (!adap_list || ADP(dev)>=adp_cnt || !adap_list[ADP(dev)]) 
			tp = 0;
		else
			MK_TTY(dev, chan);	/* Get tp pointer but A.._ttyp
						 * was not yet compiled.*/
		if (!tp || !tp->t_hptr)
		{
			/* Call slionadd with adapter and line DDS */
			if (result = slionadd(dev, &(current_adap_conf->dds),
						&(line_dds)))  
			{ 	/* bad status */
				SL_printf("Slionadd not happy\n");
				break;
			};
			MK_TP(dev, chan);	/* Get tp pointer:set by
						 * slionadd1. */
		};

		SL_printf("TP=%x|",tp);

		/* If adapter has already been configured, t_OpenRetrieve 
		 * must not have value. This field is updated only on 
		 * first open and clear at close time.
		 */
		if (!tp || !(ld = tp->t_hptr)) 
		{
			slion_err(line_dds.line_name, ERRID_TTY_PROG_PTR, 0);
			result = ENXIO;       /* Somewhat random return code */
			break;
		};
		if(tp->t_config)
		{
			SL_printf("Port(%x) already configured\n",dev);
			result = EBUSY;
			slion_err(tp->t_name, ERRID_COM_CFG_PORT, 0);
			break;
		}

		/* Move the termios/termiox structure brought by the dds. */
		bcopy(&(line_dds.ctl_modes), &(tp->t_termios), 
			(sizeof(struct termios) + sizeof(struct termiox)));

		/* This chip does not support different I/O speed */
		if((cfgetospeed(&(line_dds.ctl_modes)) != 
				cfgetispeed(&(line_dds.ctl_modes))) || 
			(!(cfgetispeed(&(line_dds.ctl_modes)) && 
				cfgetospeed(&(line_dds.ctl_modes)))))
		{
			cfsetispeed(&tp->t_termios, 
					(tp->t_termios.c_cflag) & 0xf);
		}
		else 
		{
		     if(!(cfgetospeed(&(line_dds.ctl_modes))))
		     {
			cfsetospeed(&tp->t_termios, B19200);
			cfsetispeed(&tp->t_termios, B19200);
		     }
		}

		/* Set START/STOP character */
		ld->prms->lxoc = ld->prms->rxoc =  tp->t_termios.c_cc[VSTOP];
		ld->prms->lxac = ld->prms->rxac =  tp->t_termios.c_cc[VSTART];

		ld->prms->tr_tbc = line_dds.tbc;
		bcopy(line_dds.in_xpar, ld->prms->in_xpar, 
			(sizeof(line_dds.in_xpar) + sizeof(line_dds.lv_xpar)));
		ld->prms->priority = line_dds.priority;

		if(tp->t_termiox.x_sflag & DTR_OPEN)
			tp->t_disctype = DTRO_OPEN;
		else
			tp->t_disctype = OTHER_OPEN;

		/* Check if  line name/ tty pointer already set. */
		if(tp->t_name != line_dds.line_name)
                        slionadd1(dev, (adap_list[ADP(dev)]), &line_dds);

		tp->t_config = 1;
		tp->t_lock = LOCK_AVAIL;
		tp->t_event = EVENT_NULL;

#ifdef TTYDBG
		/* Open the ttydbg extension for the driver.  */
		bcopy(line_dds.line_name, &(slion_tty_to_reg.ttyname),
					sizeof(slion_tty_to_reg.ttyname));
		slion_tty_to_reg.private_data = tp;
		ttydbg_error = tty_db_open(&slion_tty_to_reg);
#endif	/* TTYDBG */

		break;

          default:
		slion_err(resource, ERRID_COM_CFG_RESID, uiop->uio_resid);
		result = EINVAL;
		break;

        } /* End switch (uiop->uio_resid) */
	break;

    case CFG_TERM:	
	SL_printf("CFG_TERM(P%x A%x)|",uiop->uio_resid,
				sizeof(struct lion_adap_dds));

        switch (uiop->uio_resid)
	{
          /* Adpater configuration */
          case sizeof(struct lion_adap_dds):
		SL_printf("A_DDS|");
        	if (result = uiomove((unsigned char *)&adap_dds, 
			sizeof(struct lion_adap_dds), UIO_WRITE, uiop)) 
		{
            		slion_err(resource, ERRID_COM_CFG_UIO, result);
            		break;
        	}

        	if (adap_dds.which_dds != LION_ADAP_DDS) 
		{
           		slion_err(resource,ERRID_COM_CFG_RESID,
							uiop->uio_resid);
           		result = EINVAL;
           		break;
        	};

		/* Check to see if the adapter is already configured 
		 * CFG_TERM: it will be called one time per each line 
		 * and one time per each adapter.
		 */
		SL_printf("dev=%x A_cnt=%x A_list=%x|",dev,adp_cnt, adap_list);

		tmp_adap_conf = adap_conf_ss;
		current_adap_conf = adap_conf_ss;
		while (current_adap_conf &&
		   strcmp(current_adap_conf->dds.lc_name, adap_dds.lc_name)) 
		{
			tmp_adap_conf = current_adap_conf;
			current_adap_conf = current_adap_conf->next;
		}
		if (current_adap_conf) 
		{
			/* No line must be currently configured.
			 * It is the case if no sli_adap structure 
			 * exists for this adapter name. 
			 */
			index = 0;
			while(index < adp_cnt)
                        {
                          if (adap_list[index] &&
                                !strcmp(adap_list[index]->adap_name,
                                adap_dds.lc_name))
                          {
                            /*
                             * we have found an entry for this adapter. we will
                             * exit this loop with index < adap_cnt. that will
                             * tell the code below that this adapter still has
                             * configured devices attached to it..
                             */
                            break;
			  }
                          /* look at next entry */
                          index ++;
                        }
			/* If no line is found, free the structure in
			 * adap_conf_ss list Otherwise, return an error.
			 */
			if (index >= adp_cnt) 
			{
				if (current_adap_conf == adap_conf_ss) 
				  adap_conf_ss = current_adap_conf->next;
				else
				  tmp_adap_conf->next = current_adap_conf->next;
				assert(!pinfree(current_adap_conf));
			}
			else 
			{
			    SL_printf("Some ports are still configured\n");
			    result = EBUSY;
			    slion_err(adap_dds.lc_name, ERRID_COM_CFG_PORT, 0);
			} /* End if (index >= adp_cnt) */
		}
		else 
		{
			SL_printf("Adapter(%s) was not configured\n", 
							adap_dds.lc_name);
			result = EINVAL;
		} /* End if (current_adap_conf) */

		break;
            
          /* Line configuration */
          case sizeof(which_dds):
		SL_printf("L_DDS|");
		if (result = uiomove((unsigned char *)&which_dds, 
			sizeof(which_dds), UIO_WRITE, uiop)) 
		{
			slion_err(resource, ERRID_COM_CFG_UIO, result);
			break;
		};

		if (which_dds != LION_LINE_DDS) 
		{
			slion_err(resource, ERRID_COM_CFG_RESID, 
							uiop->uio_resid);
			result = EINVAL;
			break;
		};

#ifdef TTYDBG
		/* Close the ttydbg extension for the driver.
		 * Do it here, before deletion of structures. */
		MK_TP(dev, chan);
		slion_tty_to_reg.private_data = tp;
		bcopy(tp->t_name, &(slion_tty_to_reg.ttyname),
					sizeof(slion_tty_to_reg.ttyname));
		ttydbg_error = tty_db_close(&slion_tty_to_reg);
#endif	/* TTYDBG */

		if(result = sliondel(dev))
			break;

		if(!adap_list)
		{
			if(result = str_install(STR_UNLOAD_DEV,&slionconf)) 
			{
				SL_printf("str_install unload failed with %d\n"
							, result);
				slion_err(adap_dds.lc_name, ERRID_COM_CFG_DEVA,
							 result);
	    		}
	    		SL_printf("Install(UNLD)");
#ifdef TTYDBG
			/* Unregistering the module into the ttydbg extension*/
			ttydbg_error = 
				tty_db_unregister(&slion_str_module_conf);
#endif	/* TTYDBG */

                };
	}

	break;

    case CFG_QVPD: 
    {
	str_lionp_t		tp;
	struct sliondata	*ld;
	uchar			*vpd, *BusMem, *pos_ptr;
	struct sli_adap		*adap;
	volatile int		ii, jj;

#ifdef SLI_TRACE
	int	slot,xx,*vv;
	str_lionp_t zz;
	slot=ADP(dev);
	xx=(int)adap_list[slot];
	zz=adap_list[slot]->m_ttyp[PRT(dev)];
	vv=(int *)zz->t_hptr;
	SL_printf("CFG_QVPD(dev=%x adp=%x ld=%x)", dev,xx,vv);
#endif

	if (ADP(dev) >= adp_cnt || !(adap=adap_list[ADP(dev)]) ||
		!(tp = adap_list[ADP(dev)]->m_ttyp[PRT(dev)]) ||
		!(ld = (struct sliondata *)tp->t_hptr))
	{

	    result = ENODEV;

	} 
	else if (adap->inxbox) 
	{
	    int xerr, yerr;
	    uchar pos[8];

	    SL_printf("IBX:=%x|",adap->inxbox);

	    if (result = sxu_open(adap->parent))
		break;
	    for (ii=0 ; ii<8 ; ii++)
		if (xerr = sxu_getpos(adap->slot, ii, &pos[ii]))
		    break;
	    yerr = sxu_close();
	    if (xerr || (xerr=yerr))
		result = xerr;
	    else
		for (ii = 0; ii<8 && !(result = ureadc(pos[ii], uiop)); ii++);
	} 
	else 
	{
	    SL_printf("PSP:iseg=%x ibs=%x|",adap->iseg,adap->ibase);

	    pos_ptr = io_att(adap->iseg, adap->ibase);

	    ii = 0;
	    StartPio(slion_pio, ld, result = EIO; break);
	    for (; ii<8 && !(result = ureadc(BUSIO_GETC(pos_ptr), uiop));
		 pos_ptr++, ii++);
	    EndPio();
	    io_det(pos_ptr);
	}

	/* The VPD starts at 0xfe00 offset from the beginning of each
	 ** 64k block.  The first 128 bytes are the VPD for the adapter
	 ** and the next 128 bytes are the VPD for the box.  The byte
	 ** at 0xff00 will be 0xff when the VPD for the box is valid.
	 */
	if (!result) 
	{
	    ATT_BD(ld);
	    vpd = MK_ADDR(ld->board[CRD_VPD_BYTE1]);
	    SL_printf("RD adp VPD|");

	    /* Read adapter VPD */
	    ii = 0;
	    StartPio(slion_pio, ld, result = EIO; break);
	    for ( ; ii<127 && uiop->uio_resid && !result ; ii++, vpd++)
		result = ureadc(BUS_GETC(vpd), uiop);
	    EndPio();

	    /* Read Box VPD */
	    vpd = MK_ADDR(ld->board[BOX_VPD_BYTE1]);
	    if (!result && !ld->prms->has_died) {
		ii = 0;
		StartPio(slion_pio, ld, result = EIO; break);
		for ( ; ii<127 && uiop->uio_resid && !result ; ii++, vpd++)
		    result = ureadc(BUS_GETC(vpd), uiop);
		EndPio();
	    }
	    DET_BD();
	}
	break; 

     } /* CFG_QVPD */
     default:
	result = EINVAL;
	break;
    } /* switch(cmd) */

    if (ret == LOCK_SUCC)
	unlockl(&lock);

    SL_printf("ESconfig(%x)",result);

    Return(result);

} /* slionconfig */

/*
 * FUNCTION: 	slionadd
 *
 * PURPOSE:	Allocate and initialize the internal used tables associated 
 *		with the adapter.
 *
 * INPUT:
 *      dev:    Device number to configure.
 *	Aconf:	Pointer to adapter DDS structure.
 *	Lconf:	Pointer to line DDS structure.
 *
 * RETURN:
 *	0:	On success.
 *
 *	EINVAL:	Unkown adapter, wrong value in DDS,
 *		already configured to the adapter.
 *	EBUSY:	Already configured to the port.
 *	EIO:	BUSIO_GETC error or  error of copy from the
 *		BusMem.
 *	ENODEV:	Adapter pos value error.
 *	ENOMEM:	Not enough space to the internal use table.
 *	err:	Result of sxu_xxxx().
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_err	sxu_open	sxu_getpos
 *	sxu_close	sxu_mod64	slion_setpos	slion_pio
 *	sct_unlink	slionservice	sbox_reset
 *
 * EXTERNAL PROCEDURES CALLED:
 *	io_att		io_det		pinmalloc	bzero
 *	bcopy		pinfree		i_disable	i_enable
 *	copyrt
 *
 * DESCRIPTION:
 *
 *	This function is sorta big and ugly but it is only called once for
 *	each adapter configured into the system.
 *	Things malloc'd: 
 *	1) The edge vector to go from minor number to the edge structure.
 *	   This is grown to the largest slot we have an adapter in. If it 
 *	   wasn't for the watchdog timer, this would not be pinned.
 *	   The edge structure is a list of pointers to tty
 *	   structures which is indexed by port number thus giving a two
 *	   dimensional effect index by slot and port.  
 *	2) The sli_adap struct is pinned.  It is released only after all 
 *	   ports on the adapter have been un-configured.  
 *	3) Slih is also pinned and 
 *	   when an adapter that uses it is unconfigured, it is released.  
 *	4) Each corresponding table(struct str_lionp_t, sliondata,
 *	   intr)  in the allocated adapters is initialized. 
 *
 * CALLED BY:
 *	slionconfig
 */
static int
slionadd(dev_t dev, struct lion_adap_dds *Aconf, struct lion_line_dds *Lconf)
{
    DoingPio;
    caddr_t 		pos_ptr;
    uchar 		pos[4];
    str_lionp_t 	tp;
    struct sliondata 	*ld;
    struct adapters 	*adp_typ;
    struct intr 	*intr;
    struct sli_adap 	*adap;
    struct sli_slih 	*slih;
    int 		copyrt[8], jj, int_level, old_intr;
    volatile int 	ii;
    volatile int 	kk;

    int 		err = 0, err2;
    uchar 		*BusMem;
    dev_t 		min=minor(dev), maj=major(dev);
    uchar 		inxbox = Aconf->lc_xbox;
    chan_t 		chan;

    Enter(TRC_LION(LION_ADD), dev, 0, 0, 0, 0);

    chan = CHN(dev);				/* get channel from dev */

    SL_printf("\nSladd(%x)|",Lconf);

    /* Get an entry of adap_types table */
    for (adp_typ=adap_types ;
	    adp_typ->type != Unknown_Adap && adp_typ->type != Aconf->lc_type;
	    ++adp_typ) ;
    if (adp_typ->type == Unknown_Adap) 
    {
	slion_err(Aconf->lc_name, ERRID_COM_CFG_UNK, Aconf->lc_type);
	Return(EINVAL);
    }

    /* Now we have got a proper adap info */
    intr = adp_typ->intr;

    /*
     * Check any and all reasons this could fail because of bad config
     * parameters before we start malloc'ing stuff.  The init could
     * still fail due to system problems.
     * Also note bus_type,flags,level are set in intr struct declared 
     * Only priority,bid are set for CONFIG_TIME 
     * For the declared once match with dds struct passed must be okay.
     * Check slih parameters. 
     */
    if (intr->bus_type != CONFIG_TIME && intr->bus_type != Aconf->lc_bus) 
    {
	slion_err(Aconf->lc_name, ERRID_COM_CFG_BUST, Aconf->lc_bus);
	Return(EINVAL);
    }

    if (intr->flags != CONFIG_TIME && intr->flags != Aconf->lc_flags) 
    {
	slion_err(Aconf->lc_name, ERRID_COM_CFG_IFLG, Aconf->lc_flags);
	Return(EINVAL);
    }

    if (intr->level != CONFIG_TIME && intr->level != Aconf->lc_level) 
    {
	slion_err(Aconf->lc_name, ERRID_COM_CFG_ILVL, Aconf->lc_level);
	Return(EINVAL);
    }

    if (intr->priority != CONFIG_TIME && intr->priority != Aconf->lc_priority) 
    {
	slion_err(Aconf->lc_name, ERRID_COM_CFG_INTR, Aconf->lc_priority);
	Return(EINVAL);
    }

    if (intr->bid != CONFIG_TIME && intr->bid != Aconf->lc_nseg) 
    {
	slion_err(Aconf->lc_name, ERRID_COM_CFG_BUSI, Aconf->lc_level);
	Return(EINVAL);
    }

    if ((int)(Aconf->lc_base) & 0xff03ffff) /* sanity check the base address*/
	Return(EINVAL);

    for (int_level=0 ; int_level<8 ; int_level++)
	if (ccIntTbl[int_level] == Aconf->lc_level)
	    break;

    if (int_level == 8)
	Return(EINVAL);

    if (inxbox) 
    {
	int xerr, yerr;
	if (xerr = sxu_open(Aconf->lc_parent))
	    Return(xerr);
	for (ii=0 ; ii<4 ; ii++)
	    if (xerr = sxu_getpos(Aconf->lc_slot, ii, &pos[ii]))
		break;
	yerr = sxu_close();
	if (xerr || (xerr=yerr))
	    Return(xerr);
    } else {

	pos_ptr = io_att(Aconf->lc_iseg, Aconf->lc_ibase);
	StartPio(slion_pio, 0, err = EIO; break);
	pos[0] = BUSIO_GETC(pos_ptr);
	pos[1] = BUSIO_GETC(pos_ptr+1);
	pos[2] = BUSIO_GETC(pos_ptr+2);
	pos[3] = BUSIO_GETC(pos_ptr+3);
	EndPio();
	io_det(pos_ptr);
	if (err)
	    Return(err);
    }

    if (pos[0] != adp_typ->pos0 && pos[1] != adp_typ->pos1)	/* fd 61 */ 
    {
	slion_err(Aconf->lc_name, ERRID_COM_CFG_NADP, ENODEV);
	Return(ENODEV);
    }
    pos[2] = (CC_INT_BASE|CC_CRD_ENAB) | (int_level<<1);
    pos[3] = (int)(Aconf->lc_base) >> 17;

    SL_printf("set pos2/3=[%x %x]",pos[2],pos[3]);


    /* Completely initialize adap before chaining it on.
     * We will need 64 tty structures and 64 sliondata structures
     * for the feature ports, we will need 128 more sliondata and tty
     * and we will want the slionprms structures.
     */

    /* First, check whether or no adap_list space is enpugh.*/
    if ( ADP(dev) >= adp_cnt ) 
    {
	struct sli_adap **new_list;
	int new_max = ADP(dev) + 1;
	int size = new_max * sizeof(struct sli_adap *);

	/* Alloc for new adap. */
	if (!(new_list = (struct sli_adap **)pinmalloc(size))) 
	{
	    slion_err(Aconf->lc_name, ERRID_LION_MEM_LIST, 0);
	    Return(ENOMEM);
	}
	bzero(new_list, size);
	if(adap_list) 
 	{
	    /* Copy the existing  onto new one */
	    bcopy(adap_list, new_list, adp_cnt * sizeof(struct sli_adap *));
	    pinfree((caddr_t)adap_list);
	}
	adap_list = new_list;
	adp_cnt = new_max;
    }

    if (adap_list[ADP(dev)])
	Return(EINVAL);			/* Already configured */

    /* Create an adap area */
    if (!(adap = (struct sli_adap *)pinmalloc(sizeof(struct sli_adap))) ) 
    {
	slion_err(Aconf->lc_name, ERRID_LION_MEM_ADAP, 0);
	Return(ENOMEM);
    }
    bzero(adap, sizeof(*adap));

    /* Look for identical slih */
    for (slih=slih_root ; slih ; slih = slih->next) 
    {
	if (slih->intr.handler == intr->handler &&
	    slih->intr.bus_type == Aconf->lc_bus &&
	    slih->intr.flags == Aconf->lc_flags &&
	    slih->intr.level == Aconf->lc_level &&
	    slih->intr.priority == Aconf->lc_priority &&
	    slih->intr.bid == Aconf->lc_nseg )
	    break;
    }
    if (!slih) 			/* create slih definition, and initialize. */
    {
	if (!(slih = (struct sli_slih *)pinmalloc(sizeof(struct sli_slih))) ) 
 	{
	    slion_err(Aconf->lc_name, ERRID_COM_MEM_SLIH, 0);
	    Return(ENOMEM);
	}

	/* Initialize all of the fields */
	slih->intr.next = 0;
	slih->intr.handler = intr->handler;
	slih->intr.bus_type = Aconf->lc_bus;
	slih->intr.flags = Aconf->lc_flags;
	slih->intr.level = Aconf->lc_level;
	slih->intr.priority = Aconf->lc_priority; /* INTCLASS0 */
	slih->intr.bid = Aconf->lc_nseg;
	slih->adap_chain = (struct sli_adap *)0;
	slih->next = 0;

	/* Put on the global (AIX) slih chain */
	if (i_init((struct intr *)slih) != INTR_SUCC) 
	{
	    slion_err(Aconf->lc_name, ERRID_COM_CFG_SLIH, 0);
	    pinfree((caddr_t)slih);
	    Return(EIO);
	}
	/* Chain this guy onto the slih list */
	slih->next = slih_root;
	slih_root = slih;
    } /* !slih */

    INIT_TTY_OFFL(&adap->intr, slion_offlev, Aconf->lc_nseg);

    adap->board = (ipmem)Aconf->lc_base;
    adap->slot = Aconf->lc_slot;
    adap->nseg = Aconf->lc_nseg;
    adap->iseg = Aconf->lc_iseg;
    adap->ibase = Aconf->lc_ibase;
    adap->inconfig = 1;
    if (inxbox) 
    {
	adap->inxbox = 1;
	adap->parent = Aconf->lc_parent;
    }
    adap_list[ADP(dev)] = adap;

    old_intr = i_disable(INTCLASS0);

    adap->list_next = adap_root;
    adap->slih_next = slih->adap_chain;
    adap_root = adap;
    slih->adap_chain = adap;

    i_enable(old_intr);

    /* OK now we can enable the adapter.
    ** This will be really painful, cause we'll have to undo all of the
    ** slih and adap stuff if anything fails.  But that's really OK
    ** cause we can just call sct_unlink!!
    */
    /* setup pos3 before pos2, cause pos2 can cause interrupt
    ** and we need to be able to access the adapter to find out
    ** that an interrupt is being generated.
    */
    if (inxbox) 
    {
	int xerr, yerr;
	if (xerr = sxu_open(adap->parent))
	    err = xerr;
	else if (!(xerr = sxu_setpos(Aconf->lc_slot, 3, pos[3])))
	    if (!(xerr = sxu_mod64(Aconf->lc_slot, pos[3], EU_ADD64)) )
		xerr = sxu_setpos(Aconf->lc_slot, 2, pos[2]);
	    else
		sxu_mod64(Aconf->lc_slot, pos[3], EU_DEL64);

	yerr = sxu_close();
	if (xerr || (xerr=yerr))
		err = xerr;
    } else {
	pos_ptr = io_att(Aconf->lc_iseg, Aconf->lc_ibase);
	StartPio(slion_pio, 0, err = EIO; break);
	BUSIO_PUTC(pos_ptr+3, pos[3]);
	BUSIO_PUTC(pos_ptr+2, pos[2]);
	EndPio();
	io_det(pos_ptr);
    }

    if (!err) 
    {
	/* Let's make sure we can access the DRAM */
	BusMem = (uchar *)( (int)io_att(Aconf->lc_nseg, 0) | 
                          (int)Aconf->lc_base | 0xa0);
	ii = 0;
	StartPio(slion_pio, inxbox?(void *)1:(void *)0, err = EIO; break);
	for ( ; ii<8 ; ii++)
	    copyrt[ii] = *(uint *)(BusMem + (ii<<2));
	EndPio();
	io_det(BusMem);
    }
    if (!err) 
    {
	for (ii=0 ; ii<8 ; ii++)
	    if (copyrt[ii] != lion_copyrt[ii]) 
	    {
		SL_printf("M=%x|",copyrt[ii]);

		slion_err(Aconf->lc_name, ERRID_LION_HRDWRE, 0);
		err = EIO;
		break;
	    }
    }
    if (err) 
    {
	pos[2] &= ~CC_CRD_ENAB;		/* Clear card enable pos bit */
	if (adap->inxbox) 
	{
	    if (!sxu_open(adap->parent) &&
		!sxu_setpos(Aconf->lc_slot, 2, pos[2]))
		sxu_mod64(Aconf->lc_slot, pos[3], EU_DEL64);
	    sxu_close();
	} 
	else 
	{
	    pos_ptr = io_att(Aconf->lc_iseg, Aconf->lc_ibase);
	    StartPio(slion_pio, 0, ;);
	    BUSIO_PUTC(pos_ptr+2, pos[2]);
	    EndPio();
	    io_det(pos_ptr);
	}
	sct_unlink(adap);
	Return(err);
    }

    ATT_BD(adap);

    SL_printf("init LNs");

    /* By this time everything should be malloc'd 
     * So let's initialize everything and link them together 
     */
    /* configur line with the value of line dds. */
    for ( ii = min & (~0x3f), jj=0 ; jj < adp_typ->ports; ii++, jj++) 
    {
	tp = adap->m_tty + jj;
	ld = adap->main + jj;

#ifdef COMM_DEBUG
	ld->pad_curr = 0;
#endif

	ld->tp = tp;
	tp->t_hptr = (caddr_t)ld;
	tp->t_dev = makedev(maj, ii);
	tp->t_channel = CHAN_MAIN;

	ld->prms = adap->prms + jj;
	*ld->prms = prm_proto;		/* Initialize prm */
	ld->prms->tp = tp;
	ld->prms->rtsp = ld->prms->ctsp = 1;
	ld->prms->vger_defd = ld->prms->xpar_defd = 0;


	/*  Set board(struct ipmem) address */
	ld->board = (uchar *)adap->board + ((jj>>4) * (64*1024));

	StartPio(slion_pio, ld, err = EIO; break);

#ifdef VPD_BOX_RESET
	if (!(jj & 0x0f)) 
	{
	    WR_CHAR(&ld->board[BOX_VPD_BYTE0], 0xAA);
	    WR_CHAR(&ld->board[BOX_VPD_BYTEL], 0xAA);
	}
#endif /* VPD_BOX_RESET */

	ld->ip = (struct ipu *)(ld->board +
			RD_SHORT(&(((ipmem)ld->board)->ch_adds[jj&0xf])) );

	ld->prms->dcd = RD_CHAR(&ld->ip->cur_dcd) ? 1 : 0;
	ld->prms->cts = RD_CHAR(&ld->ip->cur_cts) ? 1 : 0;

	ld->rc_buf = &ld->ip->b_rc_main;
	ld->tr_buf = &ld->ip->b_tr_main;

	ld->tr_bb = RD_SHORT(&ld->tr_buf->buf_beg);
	ld->tr_ba = RD_SHORT(&ld->tr_buf->buf_almost);
	ld->rc_bb = RD_SHORT(&ld->rc_buf->buf_beg);
	ld->rc_ba = RD_SHORT(&ld->rc_buf->buf_almost);
	ld->prms->tr_width = RD_SHORT(&ld->ip->tr_width);
	ld->prms->rc_width = RD_SHORT(&ld->ip->rc_width);
	EndPio();

	ld->nseg = adap->nseg;		/* For attach and detach stuff */

	if (inxbox)
	    ld->flags |= XBOX_FLAG;


#ifdef FEATURES
	/* initialization for alternate terminals */
	tp = adap->v_tty + jj;
	ld = adap->vger + jj;

#ifdef COMM_DEBUG
	ld->pad_curr = 0;
#endif

	ld->tp = tp;
	tp->t_hptr = (caddr_t)ld;

	ld->prms = adap->prms + jj;
	ld->prms->vtp = tp;

	tp->t_dev = makedev(maj, ii);
	tp->t_channel = CHAN_VGER;

	ld->board = (uchar *)adap->board + ((jj>>4) * (64*1024));

	StartPio(slion_pio, ld, err = EIO; break);

	ld->ip = (struct ipu *)(ld->board +
		RD_SHORT(&(((ipmem)ld->board)->ch_adds[jj&0xf])) );

	ld->rc_buf = &ld->ip->b_rc_alt;
	ld->tr_buf = &ld->ip->b_tr_alt;

	ld->tr_bb = RD_SHORT(&ld->tr_buf->buf_beg);
	ld->tr_ba = RD_SHORT(&ld->tr_buf->buf_almost);
	ld->rc_bb = RD_SHORT(&ld->rc_buf->buf_beg);
	ld->rc_ba = RD_SHORT(&ld->rc_buf->buf_almost);

	EndPio();

	ld->nseg = adap->nseg;		/* For attach and detach stuff */
	if (inxbox)
	    ld->flags |= XBOX_FLAG;

	/* initilization for xparent printers */
	tp = adap->x_tty + jj;
	ld = adap->xpar + jj;

#ifdef COMM_DEBUG
	ld->pad_curr = 0;
#endif

	ld->tp = tp;
	tp->t_hptr = (caddr_t)ld;

	ld->prms = adap->prms + jj;
	ld->prms->xtp = tp;

	tp->t_dev = makedev(maj, ii);
	tp->t_channel = CHAN_XPAR;

	ld->board = (uchar *)adap->board + ((jj>>4) * (64*1024));

	StartPio(slion_pio, ld, err = EIO; break);

	ld->ip = (struct ipu *)(ld->board +
		RD_SHORT(&(((ipmem)ld->board)->ch_adds[jj&0xf])) );

	ld->rc_buf = (struct ring *)0;	/* No receive for xprinter */
	ld->tr_buf = &ld->ip->b_tr_xpar;

	ld->tr_bb = RD_SHORT(&ld->tr_buf->buf_beg);
	ld->tr_ba = RD_SHORT(&ld->tr_buf->buf_almost);

	EndPio();

	ld->rc_bb = ld->rc_ba = 0;	/* No receive for xprinter */

	ld->nseg = adap->nseg;		/* For attach and detach stuff */
	if (inxbox)
	    ld->flags |= XBOX_FLAG;

#endif /* FEATURES */

	ld = adap->main + jj;
	StartPio(slion_pio, ld, err = EIO; break);
	if (RD_SHORT(&((ipmem)(ld->board))->live_box) == 0x0004) 
	{
	    ld->prms->has_died2 = 0;
	    sbox_reset(ld);
	} else
	    ld->prms->has_died2 = ld->prms->has_died = 1;
	EndPio();
    }
    if (err) 
    {
	DET_BD();
	Return(err);
    }
    /* put adapter name in this sli_adap struct */
    bcopy(Aconf->lc_name,adap->adap_name,sizeof(adap->adap_name));

    slionadd1(dev, adap, Lconf);

    old_intr = i_disable(INTCLASS0); 		/** from new lion.c ****/
    adap->inconfig = 0;		/* OK to process interrupts now! */
    StartPio(slion_pio, ld, err = EIO; break);
    WR_CHAR(&adap->board->int_mask, 0);	/* ENABLE INTERRUPTS */
    EndPio();
    i_enable(old_intr); 			/** from new lion.c ****/

    SL_printf("ESladd|");

    DET_BD();
    Return(err);

} /* slionadd */

/*
 * FUNCTION: 	slionadd1
 *
 * PURPOSE:	Save line/adapter name into the str_lion structure associated
 *		with the port.
 *
 * INPUT:
 *      dev:    Device number to configure.
 *	Lconf:	Pointer to line DDS structure.
 * 
 * RETURN:
 *	0:	Always.
 *
 * CALLED by:
 *	slionconfig(case:line DDS)
 */
static
slionadd1(dev_t dev, struct sli_adap *adap, struct lion_line_dds *Lconf)
{
	str_lionp_t		tp;

	SL_printf("Sadd1|");

	tp = adap->m_ttyp[PRT(dev)] = adap->m_tty+PRT(dev);
	bcopy(Lconf->line_name, tp->t_name, sizeof(tp->t_name));

#ifdef FEATURES
	tp = adap->v_ttyp[PRT(dev)] = adap->v_tty+PRT(dev);
	bcopy(Lconf->line_name, &tp->t_name[1], sizeof(tp->t_name));
	tp->t_name[0] = 'v';

	tp = adap->x_ttyp[PRT(dev)] = adap->x_tty+PRT(dev);
	bcopy(Lconf->line_name, &tp->t_name[1], sizeof(tp->t_name));
	tp->t_name[0] = 'x';

#endif /* FEATURES */

	return(0);

} /* slionadd1 */

/*
 * FUNCTION: 	sliondel
 *
 * PURPOSE:	Release the all allocated resouces associated 
 *		with the adapter.
 *
 * INPUT:
 *      dev:    Device number to configure.
 *
 * RETURN:
 *	0:	On success.
 *
 *	EINVAL:	Adapter not yet unconfigured.
 *	EBUSY:	Not yet closed  to the port.
 *	EIO:	BUSIO_GETC error or  error of copy from the
 *		BusMem.
 *	err:	Result of sxu_open()/slion_pio().
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_err	sxu_open	sxu_setpos
 *	sxu_getpos	sxu_close	sxu_mod64	slion_pio
 *	sct_unlink
 *
 * EXTERNAL PROCEDURES CALLED:
 *	io_att		io_det		bzero
 *	pinfree		i_disable	i_enable
 *
 * DESCRIPTION:
 *	When the last port associated with an adapter has been unconfigured, 
 *	the all resources which were allocated at the configuration time, 
 *	are released.
 *
 * CALLED BY:
 *	slionconfig
 */
static int
sliondel(dev_t dev)
{
    DoingPio;
    caddr_t 		pos_ptr;
    uchar 		pos2, pos3;
    str_lionp_t 	tp;
    struct sliondata 	*ld;
    struct adapters 	*adp_typ;
    struct sli_adap 	*adap;
    struct sli_slih 	*slih;
    int 		copyrt[8], ii, jj;
    int 		old_intr;
    uchar 		*BusMem;
    dev_t 		prt=PRT(dev), adp=ADP(dev);
    int 		err = 0;
    chan_t 		chan;

    Enter(TRC_LION(LION_DEL), dev, 0, 0, 0, 0);

    SL_printf("Sdel|");

    /* Check to make sure the adapter is configured */
    if (adp >= adp_cnt || !(adap=adap_list[adp]))
    {
	SL_printf("Adapter not configuered|");
	Return(EINVAL);
    }

    old_intr = i_disable(INT_TTY);

    if ((adap->m_tty+PRT(dev))->t_isopen || (adap->m_tty+PRT(dev))->t_wopen ||
	(adap->v_tty+PRT(dev))->t_isopen || (adap->v_tty+PRT(dev))->t_wopen ||
	(adap->x_tty+PRT(dev))->t_isopen || (adap->x_tty+PRT(dev))->t_wopen) 
    {
	i_enable(old_intr);

	SL_printf("Line not closed|");

	Return(EBUSY);
    }

    chan = CHN(dev);				/* Get channel from dev */
    MK_TP(dev,chan);				/* Get tp pointer */	
    if (!tp || !(ld = (struct sliondata *)tp->t_hptr) || !(tp->t_config)) 
	Return(ENODEV);

    tp->t_config = 0;
    adap->m_ttyp[PRT(dev)] = (str_lionp_t)0;

#ifdef FEATURES
    adap->v_ttyp[PRT(dev)] = (str_lionp_t)0;
    adap->x_ttyp[PRT(dev)] = (str_lionp_t)0;
#endif /* FEATURES */

    i_enable(old_intr);

    for (ii=0 ; ii<64 ; ii++)	/* If they're all gone, zapAdap */
    {
	if (adap->m_ttyp[ii])
	{
	    SL_printf("tp not yet clrd|");

	    Return(0);
	}
    }
    if (adap->inxbox)		/* Read pos2 register */
    {
	int	xerr, yerr;

	if (xerr = sxu_open(adap->parent))
	    Return(xerr);

	if (!(xerr = sxu_getpos(adap->slot, 2, &pos2)))
	    xerr = sxu_getpos(adap->slot, 3, &pos3);
	yerr = sxu_close();
	if (xerr || (xerr=yerr))
	    Return(xerr);
    } 
    else 
    {
	pos_ptr = io_att(adap->iseg, adap->ibase);
	StartPio(slion_pio, 0, err = EIO; break);
	pos2 = BUSIO_GETC(pos_ptr+2);
	pos3 = BUSIO_GETC(pos_ptr+3);
	EndPio();
	io_det(pos_ptr);
	if (err)
	    Return(err);
    }

    adap->inconfig = 1;

    ATT_BD(adap);			/* Kludgy way to disable interrupts */
    StartPio(slion_pio, adap->inxbox?(void *)1:(void *)0, err = EIO; break);
    WR_CHAR(&adap->board->int_mask, 0x20);
    EndPio();
    DET_BD();
    if (err)
	Return(err);

    pos2 &= ~CC_CRD_ENAB;		/* Clear card enable pos bit */
    if (adap->inxbox) 
    {
	int	xerr, yerr;

	if (xerr = sxu_open(adap->parent))
	    Return(xerr);
	if (!(xerr = sxu_setpos(adap->slot, 2, pos2)))
	    xerr = sxu_mod64(adap->slot, pos3, EU_DEL64);
	yerr = sxu_close();
	if (xerr || (xerr=yerr))
	    Return(xerr);
    } 
    else 
    {
	pos_ptr = io_att(adap->iseg, adap->ibase);
	StartPio(slion_pio, 0, err = EIO; break);
	BUSIO_PUTC(pos_ptr+2, pos2);
	EndPio();
	io_det(pos_ptr);
	if (err)
	    Return(err);
    }

    /* We don't config an adapter with pos[3]==(63<<1)
     * et cetera   (note: the least sig bit of pos[3] == 0)
     */

    sct_unlink(adap);
    adap_list[adp] = (struct sli_adap *)0;
    SL_printf("adap_lst[%x]=%x, adap_list=%x|",
				adp, adap_list[adp],adap_list);

    for (ii=0 ; ii<adp_cnt ; ii++)
	if (adap_list[ii])
	    break;
    if (ii == adp_cnt) 
    {

	pinfree(adap_list);

	SL_printf("free:adp_list/cnt=0|\n");

	adap_list = 0;
	adp_cnt = 0;
    }
    Return(0);

} /* sliondel */

/*
 * Release the specified adapter from the adapter_list.
 * called by sliondel.
 */
static 
sct_unlink(struct sli_adap *adap)
{
    struct sli_adap **ct_adap, **ct_root = &adap_root;
    struct sli_slih **ct_slih;

    SL_printf("s_ulnk(%x)",adap);


    /* Chris Torek pointer->pointer->trick */
    for (ct_slih=&slih_root ; *ct_slih ; ct_slih = &(*ct_slih)->next)
    {
	/* Chris Torek pointer->pointer->trick again */
	for (ct_adap = &(*ct_slih)->adap_chain ; *ct_adap ;
	     ct_adap = &(*ct_adap)->slih_next)
	{
	    if (*ct_adap == adap) 
	    {
		*ct_adap = adap->slih_next;
		/* Chris Torek pointer->pointer->trick one more time */
		for (; *ct_root ; ct_root = &(*ct_root)->list_next)
		    if (*ct_root == adap) 
		    {
			*ct_root = adap->list_next;
			break;
		    }
		pinfree(adap);
		if (!(*ct_slih)->adap_chain) 
		{
		    struct sli_slih *slih = *ct_slih;
		    i_clear((struct intr *)slih);
		    *ct_slih = (*ct_slih)->next;
		    pinfree(slih);
		}
		return(0);
	    }
	}
    }

} /* sct_unlink */

/*
 * FUNCTION: 	slionopen
 *
 * PURPOSE:	Lion driver open interface.
 *
 * INPUT:
 *	q:	STREAMS read queue pointer.
 *	devp:	Pointer to device number.
 *	mode:	Open(2) flags (Read|Write).
 *	sflag:	Type of STREAMS open (MODOPEN, 0[normal], CLONEOPEN)
 *	credp:	Pointer to cred structure.
 *
 * RETURN:
 *	0:	On success.
 *
 *	ENODEV:	No such port(line) or adapter(slot) .
 *	EINTR:	Error return from e_sleepl(only for FEATURE specified).
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_err	slion_comm	slion_set_gcomm
 *	openDisc_open	slion_allocmsg  slion_termios_set
 *      slionservice	
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lockl		e_sleepl	unlockl		openDisc_open
 *	e_wakeup	
 *
 * DESCRIPTION:
 *	Checks if the major number corresponds to a configured adapter.
 *	Calls the open entrypont of the open discipline.
 *	Allocate the data message area to receive data from the terminal.
 *
 * CALLED BY:
 *	 Streams Framework(pse_open).
 */
static int 
slionopen(queue_t *q, dev_t *devp, int mode, int sflag, cred_t *credp)
{
    str_lionp_t 	tp;
    dev_t dev = 	*devp;
    struct sliondata 	*ld;
    int 		lrc, olrc, err = 0;
    mblk_t 		*mp;
    chan_t 		chan;
    int			old_hupcl;

    Enter(TRC_LION(TTY_OPEN), dev, 0, mode, sflag, 0);

    SL_printf("\nSopen:q=%x dev=%x flg=%x n",q,dev,sflag);

    if (ADP(dev) >= adp_cnt || !adap_list[ADP(dev)])
	Return(ENODEV);

    chan = CHN(dev);				/* get channel from dev */

    /* chan is no longer used, new designation for features */
    MK_TP(dev,chan);	

    if (!tp)
	Return(ENODEV);

    if (!(ld = (struct sliondata *)tp->t_hptr)) 
    {
	slion_err(resource, ERRID_TTY_PROG_PTR, 0);
	Return(ENODEV);
    }
    SL_printf("ld=%x|",ld);

    if(!(tp->t_config) || (tp->t_dev != *devp))
	Return(ENODEV);

    SL_printf("CHN=%x|",chan);
 
    olrc = lockl(&oc_lock, LOCK_SHORT);
    switch (chan) 
    {
      case CHAN_MAIN:
	break;

      case CHAN_VGER:
      case CHAN_XPAR:
	/* MAIN already open? */ 
	if (!(ld->prms->tp->t_isopen))
	    if (e_sleepl(&oc_lock, &ld->prms->m_open_evt, EVENT_SIGRET) ==
		    						EVENT_SIG) 
	    {
		if(olrc == LOCK_SUCC)
			unlockl(&oc_lock);
		Return(EINTR);
	    }
	break;

      default:
	break;
    }
    if(olrc == LOCK_SUCC)
	unlockl(&oc_lock);

    lrc = lockl(&tp->t_lock, LOCK_SHORT);  
    switch (chan) 
    {
      case CHAN_MAIN:
	e_wakeup(&ld->prms->m_open_evt);

	if(!(tp->t_isopen) && !(tp->t_wopen))		/* First open */
	{
		slionservice(tp, TS_DOPACE, DOPACE_AGAIN);
		slion_termiox_set(tp, &tp->t_termiox);
		slion_termios_set(tp, &tp->t_termios);

		/* Link streams data structure to lion structure */
		q->q_ptr = (caddr_t)tp;
		WR(q)->q_ptr = (caddr_t)tp;
		tp->t_ctl = q;

		/* Allocate a message block to contain 
		 * received later */
		slion_allocmsg(tp);
	}
	else
	{
		if((tp->t_excl) && (credp->cr_uid != 0))
		{
			SL_printf("Open EXCLm BUSY|");
			if (lrc == LOCK_SUCC)
				unlockl(&tp->t_lock);
			Return(EBUSY);
		}
	}
	break;

      case CHAN_VGER:
	if (!ld->prms->vger_defd) 
	{
	    ld->prms->vger_defd = 1;
	    slion_comm((struct sliondata *)ld->prms->tp->t_hptr, ALT1);
	    slion_set_gcomm(ld, GC_VGER_DEFS);
	}
	if(!(tp->t_isopen) && !(tp->t_wopen))		/* First open */
	{
	    /* Link streams data structure to lion structure */
	    q->q_ptr = (caddr_t)tp;
	    WR(q)->q_ptr = (caddr_t)tp;
	    tp->t_ctl = q;

	    /* Allocate a message block to contain characters 
	     * received later */
	    slion_allocmsg(tp);
	}
	else
	{
		if((tp->t_excl) && (credp->cr_uid != 0))
		{
			SL_printf("Open EXCLv BUSY|");
			if (lrc == LOCK_SUCC)
				unlockl(&tp->t_lock);
			Return(EBUSY);
		}
	}
	break;

      case CHAN_XPAR:
	if (!ld->prms->xpar_defd) 
	{
	    ld->prms->xpar_defd = 1;
	    slion_set_gcomm(ld, GC_GOTO_XPAR|GC_LEAV_XPAR);
	}
	if(!(tp->t_isopen) && !(tp->t_wopen))		/* First open */
	{
	    /* Link streams data structure to lion structure */
	    q->q_ptr = (caddr_t)tp;
	    WR(q)->q_ptr = (caddr_t)tp;
	    tp->t_ctl = q;
	}
	break;
    }

    if (lrc == LOCK_SUCC)
    	unlockl(&tp->t_lock);

    tp->t_wopen++;

    /* open discipline: will wit for the CD_ON if OPEN_REMOTE 
     * and mode is not DNDELAY */
    if (err = openDisc_open(&tp->t_OpenRetrieve, tp, tp->t_disctype,
		slionservice, (tp->t_clocal) ? OPEN_LOCAL: OPEN_REMOTE,
		mode, !tp->t_isopen, &tp->t_event)) 
    {
       	SL_printf("Error:open of open discipline.dev=x%x\n", dev);
	tp->t_wopen--;
	if(!(tp->t_wopen)) 		
	{
	    /* The only open in progress.  Just to make slionclose() make 
	     * the full cleaning because not sure how will the interprete 
	     * it to ensure dropping DTR and RTS. 
	     */
	    old_hupcl = (tp->t_hupcl); 	
	    tp->t_hupcl = 1;		
	    slionclose(q, mode, credp);
	    tp->t_hupcl = old_hupcl;
	}
	Return(err);
    };

    tp->t_wopen--;
    tp->t_isopen = 1;

    SL_printf("ESopen|");

    Return(err);

} /* slionopen */

/*
 * FUNCTION: 	slionclose
 *
 * PURPOSE:	Lion driver close interface.
 *
 * INPUT:
 *      q:	STREAMS read queue pointer.
 *      mode:	Type of STREAMS open (MODOPEN, 0[normal], CLONEOPEN).
 *      credp:	Cred structure pointer.
 *
 * RETURN:
 *	0:	On success.
 *
 *	ENODEV:	No such line or slot .
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_err	slion_flush?
 *
 * EXTERNAL PROCEDURES CALLED:
 *	openDisc_close	lockl	unlockl
 *
 * DESCRIPTION:
 *	Checks if the major number corresponds to a configured adapter.
 *      Closes current open discipline if all opened ports are closed.
 *      Flushs queues and disconnects them from private structures.
 *	During open time, if openDisc_open return error and the opened file
 * 	is the last, the slionclose is called to cleanup the file.
 *
 * CALLED BY:
 *	 Streams Framework.	slionopen
 */
static int
slionclose(queue_t *q, int mode, cred_t *credp)
{
    str_lionp_t		tp = (str_lionp_t)q->q_ptr;
    dev_t		dev = tp->t_dev;
    struct sliondata 	*ld;
    int 		lret, olrc, old_intr;
    int			err = 0;
    chan_t 		chan; 
    volatile		must_sleep = 0;	/* Force reading of tp->t_draining
					 * each tine we are waked up */

    Enter(TRC_LION(TTY_CLOSE), dev, (int)q->q_ptr, mode, 0, 0);

    SL_printf("\nSclose dev=%x|",tp->t_dev);

    chan = CHN(dev);				/* get channel from dev */

    if (ADP(dev) >= adp_cnt || !adap_list[ADP(dev)])
    {
    	SL_printf("ErScls:!adplst|");
	Return(ENODEV);
    }

    /* Get the corresponding tp pointer */
    MK_TP(dev,chan);

    if (!tp)
    {
	SL_printf("ErScls:!tp|");
	Return(ENODEV);
    }

    if (!(ld = (struct sliondata *)tp->t_hptr)) 
    {
	SL_printf("EScls:!ld|");
	slion_err(resource, ERRID_TTY_PROG_PTR, 1);
	Return(ENODEV);
    }

    if(tp->t_wopen) 
        Return(0);

    olrc = lockl(&oc_lock, LOCK_SHORT);
    switch(chan)
    {
    	case CHAN_MAIN:
		/* Other channels closed?*/
		while(((adap_list[ADP(dev)]->v_ttyp[PRT(dev)])->t_isopen) ||
			((adap_list[ADP(dev)]->x_ttyp[PRT(dev)])->t_isopen))
		{
	             if(e_sleepl(&oc_lock,&ld->prms->m_open_evt,EVENT_SIGRET) 
						== EVENT_SIG) 
	    	     {
			if(olrc == LOCK_SUCC)
				unlockl(&oc_lock);
			Return(EINTR);
		     }
		}
		break;
	case CHAN_VGER:
	case CHAN_XPAR:
		break;
	default:
		break;
    }
    if(olrc == LOCK_SUCC)
	unlockl(&oc_lock);

    lret = lockl(&tp->t_lock, LOCK_SHORT);  
    switch (chan) 
    {
	case CHAN_VGER:
	        /*
	         * If vt closes, we flip the screen back to the main.  
		 * If we don't, the port is closed and there is no way 
		 * for the user to flip screens.
	         */
		slion_set_gcomm(ld, GC_FRCE_MAIN);
		slion_comm(ld, ALT0);
		ld->prms->vger_defd = 0;
	case CHAN_XPAR:
		e_wakeup(&ld->prms->m_open_evt);
		break;
	default:
		break;
    }
    if (lret == LOCK_SUCC)
	unlockl(&tp->t_lock);

    /* Drain data in write queue before leaving
     * Output is resumed if t_localstop is set, that is, an M_STOP
     * message was recieved. If t_localstop is clear, that means we
     * are flow-controlled by a remote device, which will unblock
     * the situation when it is ready.
     */
    old_intr = i_disable(INT_TTY);
    if(tp->t_draining = (tp->t_outmsg) || (WR(q)->q_first))
    {
	if (tp->t_localstop)
                slionproc(tp, T_RESUME);
        while (must_sleep = tp->t_draining) 
	{
           e_assert_wait(&(tp->t_event), INTERRUPTIBLE);
	   i_enable(old_intr);
           if (e_block_thread() != THREAD_AWAKENED) 
	   {
		old_intr = i_disable(INT_TTY);
		tp->t_draining = 0;
		err = EINTR;
		break;
           }
	   old_intr = i_disable(INT_TTY);
        }
    }
    slionproc(tp, T_WFLUSH);
    slionproc(tp, T_UNBLOCK);
    slionproc(tp, T_RFLUSH);
    tp->t_isopen = 0;
    tp->t_ctl = 0;
    tp->t_busy = 0;
    tp->t_localstop = 0;
    i_enable(old_intr);

    /* Call open discipline close ******/
    tp->t_wclose = 1;				/* waiting close */
    if(openDisc_close(tp->t_OpenRetrieve, tp->t_hupcl)) 
    {
        SL_printf("Problem during close of open discipline:dev=x%x.\n",dev);
    };

    /* Verify whether STREAMS CALLS close ONLY IN THE END */
    tp->t_OpenRetrieve = 0;
    tp->t_ctlx = 0;
    tp->t_sak = 0;
    tp->t_cread = 0;			/* termios cread flag */
    tp->t_wclose = 0;			/* waiting close */

    /* Don't forget bufcall and timeout pending requests */
    tp->t_sched = 0;
    if (tp->t_timeout) 
    {
        untimeout(tp->t_timeout);
        tp->t_timeout = 0;
    }
    if (tp->t_bufcall) 			
    {
        unbufcall(tp->t_bufcall);
        tp->t_bufcall = 0;
    }
    if (tp->t_alloctimer) 
    {
        untimeout(tp->t_alloctimer);
        tp->t_alloctimer = 0;
    }
    if (tp->t_brktimeout) 
    {
        untimeout(tp->t_brktimeout);
        tp->t_brktimeout = 0;
    }
    if (tp->t_slpxtmr) 
    {
        untimeout(tp->t_slpxtmr);
        tp->t_slpxtmr = 0;
    }
    if (tp->t_dslptmr) 
    {
        untimeout(tp->t_dslptmr);
        tp->t_dslptmr = 0;
    }
    if (tp->t_gcommtmr) 
    {
        untimeout(tp->t_gcommtmr);
        tp->t_gcommtmr = 0;
    }

    /* In normal cases this is not needed, since we drop output, before
       leaving. But if we exit the e_block_theread with THREAD_INTERRUPTED, and
       interrupts are disabled, t_outmsg isn't freed.
    */
    if (tp->t_outmsg) 
    {
        freemsg(tp->t_outmsg);
        tp->t_outmsg = 0;
    }
    if (tp->t_inmsg) 
    {
        freemsg(tp->t_inmsg);
        tp->t_inmsg = 0;
    }

    SL_printf("ESclose|");

    Return(err);

} /* slionclose */

/*
 * FUNCTION: 	slion_ioctl
 *
 * PURPOSE:	Processes the M_IOCTL messages..
 *
 * INPUT:
 *      q       STREAMS write queue pointer.
 *      mp      M_IOCTL to process.
 *      fromput 1:  Called by put procedure.
 *		0: Called by service procedure.
 *
 * RETURN:
 *	0:	Ioctl message is successfully processed or delayed to
 *      	the service procedure. 
 *	-1:	Othercase.
 *
 *      Note: This value is not used by the caller , but it is usefull to
 *      record a failure in an ioctl processing.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_termios_set	slion_termiosget	slionservice
 *	slion_break_set		slion_termiox_set
 *	slion_termiox_get	slioc+pres		slioc_dram	
 *	slioc_slpx		slioc_dslpx		sxvscpy		
 *	slion_set_gcomm		
 *
 * EXTERNAL PROCEDURES CALLED:
 *	qreply		flushq		putq		bzero	
 *	bcopy		i_disable	i_enable	putctl1	
 *
 * DESCRIPTION:
 *      Commands to be perfomed by service procedure are put
 *      back in the queue.
 *      An M_IOCAK is sent upstreams if successfull processing,
 *      otherwise an M_IONACK with error code is done.
 *		EINVAL:		Argument size equal to TRANSPARENT.
 *		err:		Return from slioc_dram  LI_DRAM.
 *				Return from slion_pres  LI_PRES.
 *				Return from slion_slpx  LI_SLP0/1.
 *				Return from slion_dslp  LI_DSLP.
 * CALLED BY:
 *	slionwput	slionwsrv
 */

static int 
slion_ioctl(queue_t *q, mblk_t *mp,int fromput)
{
    DoingPio;
    struct iocblk	*iocp;					/*stream.h*/
    register str_lionp_t tp = (str_lionp_t)q->q_ptr;
    int 		err = 0, er2;
    int 		old_intr;
    int 		result, rc, ii, tbc;
    unsigned char 	outtype;
    unsigned long 	outcnt;
    struct termios 	*tios;
    struct termiox 	*tiox;
    int 		signal_temp;
    int 		curr_stat;

    struct xpar_parms	xparms;					/*sys/li.h*/
    struct vter_parms	vterms;					/*sys/li.h*/
    struct slp_data	sd;
    register int	arg;
    struct sliondata	*ld = tp->t_hptr;

    int			cmd = ((struct iocblk *)mp->b_rptr)->ioc_cmd;
    int			ag = (int)mp->b_cont->b_rptr;
    Enter(TRC_LION(TTY_IOCTL), tp->t_dev, (int)tp, cmd, ag, 0);

    iocp = (struct iocblk *)mp->b_rptr;

    SL_printf("S_ioctl(%x)",iocp->ioc_cmd);

    if (iocp->ioc_count == TRANSPARENT) 
    {
    		SL_printf("TRNSPRNT|");
		mp->b_datap->db_type = M_IOCNAK; /*Not Okay */
		iocp->ioc_error = EINVAL;
		qreply(q,mp);
		Return(-1);
    }
    iocp->ioc_error =0;
    outtype = M_IOCACK;
    outcnt = 0;

    if (!fromput) 			/* from service routine */
    {					/* slionwsrv **/
      SL_printf("F.Wsv|");

      switch (iocp->ioc_cmd) 
      {
    	case TIOCSETAW:
    	case TIOCSETAF:
		SL_printf("TISETW/F|");

		if (iocp->ioc_count != sizeof (struct termios) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		/* Wait for the o/p to drain and flush I/p and set params */
		if (iocp->ioc_cmd == TIOCSETAF)
		{
			flushq(RD(q),FLUSHDATA);
			putctl1(tp->t_ctl->q_next,M_FLUSH,FLUSHR);
		}
		slion_termios_set(tp,(struct termios *)mp->b_cont->b_rptr);
		break;

    	case TCSBRK:
	case TCSBREAK:
        {   
		int arg;

		SL_printf("TCBRK|");
		if(iocp->ioc_count != sizeof (int) || !mp->b_cont) 
		{
			SL_printf("NAK|");
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		arg = *(int *)mp->b_cont->b_rptr;
		SL_printf("ag=%x|",arg);
		if (iocp->ioc_cmd == TCSBRK && arg)
			break;

		if (arg == 0) 
			arg = (25 * HZ)/1000; /* break for 0.25 seconds. */
		else 
			arg = (arg < 10 ? 10 : arg)*HZ/1000;

		tp->t_ioctlmsg = mp;	/* Will reappear, when PSE handling of
					 * timeouts on ioctls is clearer. */
		slion_break_set(tp, arg);
		Return(0);		/* M_IOCACK will be replyed after 
					 * a timeout */
        }   
	case TCSETXW:
	case TCSETXF:
		SL_printf("TCSTXW/F|");

		if (iocp->ioc_count != sizeof (struct termiox) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		/* Wait for the o/p to drain and flush I/p and set params */
		if (iocp->ioc_cmd == TCSETXF)
		{
			flushq(RD(q),FLUSHDATA);
	    	  	putctl1(tp->t_ctl->q_next,M_FLUSH,FLUSHR);
		}
		slion_termiox_set(tp,(struct termiox *)mp->b_cont->b_rptr);
		break;

    	default:
	/* I should never be here */

		SL_printf("DF:ERR!!!!|");
		break;
      }

    }
    else /* From Put routine */
    {					/* slionwput: from the upper module*/
      SL_printf("F.Wpt|");

      switch (iocp->ioc_cmd) 
      {
    	case TIOCSETAW:
    	case TIOCSETAF:
	case TCSETXW:
	case TCSETXF:
    	case TCSBRK:
    	case TCSBREAK:

		SL_printf("SETA/F STXW/F BRK|");

		putq(q,mp);
		Return(0);

    	case TIOCGETA:
		SL_printf("TCGETA|");

		if (iocp->ioc_count < sizeof(struct termios) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		tios= (struct termios *)mp->b_cont->b_rptr;
		bzero((char * )tios,sizeof(struct termios));
		slion_termios_get(tp,tios);	
		outcnt = sizeof(struct termios);
		break;

    	case TIOCSETA:

		SL_printf("TCSETA|");

		if (iocp->ioc_count < sizeof(struct termios) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		tios= (struct termios *)mp->b_cont->b_rptr;
		if(slion_termios_set(tp,tios))		/* return(0):OK */
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
		}
		break;

    	case TXTTYNAME:

		SL_printf("TYNAME|");

		if (iocp->ioc_count < TTNAMEMAX || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		bcopy(tp->t_name,(char *)mp->b_cont->b_rptr,sizeof(tp->t_name));
		outcnt=sizeof(tp->t_name);
		break;

	case TCGETX:
		SL_printf("TCGTX|");

		if (iocp->ioc_count < sizeof(struct termiox) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}

		tiox= (struct termiox *)mp->b_cont->b_rptr;
		bzero((char * )tiox,sizeof(struct termiox));
		slion_termiox_get(tp,tiox);	
		outcnt = sizeof(struct termiox);
        	break;

	case TCSETX:
		SL_printf("TCSTX|");

		if (iocp->ioc_count < sizeof(struct termiox) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		tiox= (struct termiox *)mp->b_cont->b_rptr;
		slion_termiox_set(tp,tiox);
        	break;

	case TCSAK: 
		SL_printf("TCSAK|");
		if (iocp->ioc_count < sizeof(int) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		tp->t_sak = (*((int *)(mp->b_cont->b_rptr))) ? 1 : 0;
		break;

    	case TIOCSWINSZ:		/* Only send ACK message */
		SL_printf("TCSWDSZ|");

        	break;

    	case TIOCSDTR:     
		slionservice(tp, TS_GCONTROL, &signal_temp);
		if (!(signal_temp & TSDTR)) 
		    slionservice(tp, TS_SCONTROL, (void *)(signal_temp|TSDTR));
		break;

        case TIOCCDTR:
		slionservice(tp, TS_GCONTROL, &signal_temp);
		if (signal_temp & TSDTR)
		    slionservice(tp,TS_SCONTROL,(void *)(signal_temp & ~TSDTR));
		break;
			
        case TIOCMBIS:
		SL_printf("MBIS|");
		if(iocp->ioc_count != sizeof (int) || !mp->b_cont)
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		slionservice(tp, TS_GCONTROL, &signal_temp);
		if (*((int *)mp->b_cont->b_rptr) & TIOCM_DTR) 	/*ioctl.h*/
			signal_temp |= TSDTR;
		if (*((int *)mp->b_cont->b_rptr) & TIOCM_RTS)
			signal_temp |= TSRTS;
		slionservice(tp, TS_SCONTROL, (void *)signal_temp);
		outcnt = 0;
		break;
			
        case TIOCMBIC:
		SL_printf("MBIC|");
		if(iocp->ioc_count != sizeof (int) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		slionservice(tp, TS_GCONTROL, &signal_temp);
		if (*((int *)mp->b_cont->b_rptr) & TIOCM_DTR)
			signal_temp &= ~TSDTR;
		if (*((int *)mp->b_cont->b_rptr) & TIOCM_RTS)
			signal_temp &= ~TSRTS;
		slionservice(tp, TS_SCONTROL, (void *)signal_temp);
		outcnt = 0;
		break;
			
        case TIOCMSET:
		SL_printf("MSET|");
		if(iocp->ioc_count != sizeof (int) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		signal_temp = 0;
		if (*((int *)mp->b_cont->b_rptr) & TIOCM_DTR)
			signal_temp |= TSDTR;
		if (*((int *)mp->b_cont->b_rptr) & TIOCM_RTS)
			signal_temp |= TSRTS;
		slionservice(tp, TS_SCONTROL, (void *)signal_temp);
		outcnt = 0;
		break;
			
	case TIOCMGET:
		SL_printf("MGET|");
		if(iocp->ioc_count != sizeof (int) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		signal_temp = 0;
		curr_stat = 0;
		slionservice(tp, TS_GCONTROL, &curr_stat);
		if (curr_stat & TSDTR) 
			signal_temp |= TIOCM_DTR;
		if (curr_stat & TSRTS) 
			signal_temp |= TIOCM_RTS;

		curr_stat = 0;
		slionservice(tp, TS_GSTATUS, &curr_stat);
		/* no check:DSR/RI not supported */
		if (curr_stat & TSCD) 
			signal_temp |= (TIOCM_CAR | TIOCM_DSR);
		if (curr_stat & TSCTS) 
			signal_temp |= TIOCM_CTS;
		*((int *)mp->b_cont->b_rptr) = signal_temp;
		outcnt = sizeof(int);
		break;

    	case TIOCOUTQ: 
	{
		mblk_t *mp1;
		int outsize = 0;

		SL_printf("TOUTQ|");
		if (iocp->ioc_count < sizeof(int) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		for (mp1 = q->q_first; mp1; mp1 = mp1->b_next) 
		{
			if (mp1->b_datap->db_type == M_DATA) 
			{
		    		outsize += mp1->b_wptr - mp1->b_rptr;
			}
		}
		ld = tp->t_hptr;
		outsize += ld->txcnt;
		* (int *) mp->b_cont->b_rptr = outsize;
		outcnt = sizeof(int);
		SL_printf("<%x %x>",ld->txcnt,outsize);
		break;
	}

        case TIOCEXCL:
		SL_printf("CEXCL|");
		tp->t_excl  = 1;
		break;

        case TIOCNXCL:
		SL_printf("CNXCL|");
		tp->t_excl  = 0;
		break;

	case LI_PRES:			/* presence confidence check */
		SL_printf("L_PRE|");

		if (iocp->ioc_count < sizeof(int) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		if (!(rc=slioc_pres(tp->t_hptr, &result)))
		{
			*(int *) mp->b_cont->b_rptr = result;
	    		outcnt=sizeof(int);
		}
		else
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = rc;
		}
		break;

    	case LI_DRAM:			/* dram integrity check */
		SL_printf("L_RDM|");

		if (iocp->ioc_count < sizeof(int) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		if (!(rc=slioc_dram(tp->t_hptr, &result)))
		{
			*(int *) mp->b_cont->b_rptr = result;
	    		outcnt=sizeof(int);
		}
		else
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = rc;
		}
		break;

    	case LI_SLP1:			/* turn on sync loop mode */
    	case LI_SLP0:			/* turn off sync loop mode */
		SL_printf("L_SLP|");
		/* Can't sleep in slioc_slpx()if cmd is LI_SLP1. 
		 * Then M_IOCACK msg is put in slion_wake_tmr(), 
		 * which timer is issued in sliob_slpx().
		 */ 
		if (iocp->ioc_count < sizeof(int) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		tp->t_ctloutmsg = mp;
		if (!(rc=slioc_slpx(tp->t_hptr, &result, iocp->ioc_cmd)))
		{
			if ((iocp->ioc_cmd == LI_SLP1) && (result == 0))
			{
				/* qreply will be done by slion_wake_tmr */
				Return(0);
			}
			else
			{
				/* do qreply at the end of this routine */
				*(int *) mp->b_cont->b_rptr = result;
	    			outcnt=sizeof(int);
				tp->t_ctloutmsg = (mblk_t *)0;
			}
		}
		else if(rc == EIO)
		{
			tp->t_ctloutmsg = (mblk_t *)0;
			outtype = M_IOCNAK;
			iocp->ioc_error = rc;
		}
		break;

    	case LI_DSLP:			/* do sync loop test */
		SL_printf("L_DSL|");

		if(iocp->ioc_count != sizeof (sd) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		tp->t_ctloutmsg = mp;
		if (!(rc=slioc_dslp(tp->t_hptr)))
		{
			/* on success, qreply will be handled by 
			   slioc_dslp or dslp_tmr_f */
			Return(0);
		}
		else if (rc == EIO)
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EIO;
			tp->t_ctloutmsg = (mblk_t *)0;
		}
		break;

#ifdef FEATURES
    	case LI_GETVT:			/* get params for virtual terminal */

		SL_printf("L_GVT|");

		for (ii=0 ; ii<4 ; ii++)
	    		vterms.goto1[ii] = ld->prms->goto1[ii];
		for (ii=0 ; ii<4 ; ii++)
	    		vterms.goto2[ii] = ld->prms->goto2[ii];
		for (ii=0 ; ii<10 ; ii++)
	    		vterms.screen1[ii] = ld->prms->screen1[ii];
		for (ii=0 ; ii<10 ; ii++)
	    		vterms.screen2[ii] = ld->prms->screen2[ii];
		vterms.goto1[4] = vterms.goto2[4] =
	    		vterms.screen1[10] = vterms.screen2[10] = 0xff;

		if (iocp->ioc_count < sizeof(struct vter_parms) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
	    	bcopy(&vterms, mp->b_cont->b_rptr, sizeof(struct vter_parms));
	    	outcnt=sizeof(struct vter_parms);
		break;

    	case LI_SETVT:			/* set params for virtual terminal */
		SL_printf("L_SVT|");

		if (iocp->ioc_count < sizeof(struct vter_parms) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
	    	bcopy( mp->b_cont->b_rptr, &vterms,sizeof(struct vter_parms));
		old_intr = i_disable(INT_TTY);
		sxvscpy(ld->prms->goto1, vterms.goto1, 4);
		sxvscpy(ld->prms->goto2, vterms.goto2, 4);
		sxvscpy(ld->prms->screen1, vterms.screen1, 10);
		sxvscpy(ld->prms->screen2, vterms.screen2, 10);
		if (ld->prms->vger_defd) 
		{
			tp->t_liflg |= FLGLISET;
			tp->t_ctloutmsg = mp;
			slion_set_gcomm(ld, GC_VGER_DEFS);

			/* Can't sleep here.  Mp address is saved into 
			* tp->t_ctloutmsg. The M_IOCACK msg will be put 
			* into the queue by sgcomm_tmr_f() if all mask 
			* are reset.
			*/
			if(ld->prms->gcomm_mask)
			{
				i_enable(old_intr);
				Return(0);
			}
		
		}
		tp->t_liflg = 0;
		i_enable(old_intr);
		break;

    	case LI_GETXP:			/* get params for xparent printer */
		SL_printf("L_GXP|");

		for (ii=0 ; ii<10 ; ii++)
	    		xparms.in_xpar[ii] = ld->prms->in_xpar[ii];
		for (ii=0 ; ii<10 ; ii++)
	    		xparms.lv_xpar[ii] = ld->prms->lv_xpar[ii];
		xparms.in_xpar[10] = xparms.lv_xpar[10] = 0xff;
		xparms.priority = ld->prms->priority;

		if (iocp->ioc_count < sizeof(struct xpar_parms) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
	    	bcopy(&xparms, mp->b_cont->b_rptr, sizeof(struct xpar_parms));
	    	outcnt=sizeof(struct xpar_parms);
		break;

	case LI_SETXP:			/* set params for xparent printer */
		SL_printf("L_SXP|");

		if (iocp->ioc_count < sizeof(struct xpar_parms) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
	    	bcopy( mp->b_cont->b_rptr, &xparms,sizeof(struct xpar_parms));

		old_intr = i_disable(INT_TTY);
		sxvscpy(ld->prms->in_xpar, xparms.in_xpar, 10);
		sxvscpy(ld->prms->lv_xpar, xparms.lv_xpar, 10);
		ld->prms->priority = xparms.priority & 63;
		if (ld->prms->xpar_defd) 
		{
			tp->t_liflg |= FLGLISET;
			tp->t_ctloutmsg = mp;
			slion_set_gcomm(ld, 
				GC_GOTO_XPAR|GC_LEAV_XPAR|GC_SET_PARMS);
			/* Can't sleep here.  Mp address is saved into 
			 * tp->t_ctloutmsg. The M_IOCACK msg will be put 
			 * into the queue by sgcomm_tmr_f() if all mask 
			 * are reset.
			 */
			if (ld->prms->gcomm_mask)
			{
				i_enable(old_intr);
				Return(0);
			}
		}
		tp->t_liflg = 0;
		i_enable(old_intr);
		break;

#endif /* FEATURES */

    	case LI_GETTBC:
		SL_printf("L_GBC|");

		if (iocp->ioc_count < sizeof(tbc) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}

		tbc = ((struct sliondata *)tp->t_hptr)->prms->tr_tbc;
		*(int *) mp->b_cont->b_rptr = tbc;
	    	outcnt=sizeof(tbc);
		break;

    	case LI_SETTBC:
		SL_printf("L_SBC|");

		if (iocp->ioc_count < sizeof(tbc) || !mp->b_cont) 
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		tbc = *(int *) mp->b_cont->b_rptr;
		if (tbc > (ld->prms->tr_width-2) || (tbc & 1))
		{
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
                }
		ld->prms->tr_tbc = tbc;
		break;


    	default:
		outtype = M_IOCNAK;
		break;
     }
    }

    SL_printf("ESioct(%x)|",outtype);

    mp->b_datap->db_type = outtype;
    iocp->ioc_count = outcnt;
    if (mp->b_cont)
	  mp->b_cont->b_wptr = mp->b_cont->b_rptr + outcnt;
    qreply(q,mp);
    Return((outtype == M_IOCACK)?0: -1);

} /* slion_ioctl */

/*
 * FUNCTION: 	sxu_open
 *
 * PURPOSE:	Enable adapter.
 *
 * INPUT:
 *	parent:	Adapter name of the liner.
 *
 * RETURN:
 *	0:	On success.
 *	err:	Result of fp_opendev()/fp_ioctl().
 *
 * INTERNAL PROCEDURES CALLED:
 *	sxu_close
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lockl		unlockl		fp_opendev	fp_ioctl
 *
 * DESCRIPTION:
 *		Enable adapter if it is the first call.
 *		
 * CALLED BY:
 *	slionconfig	slionadd	sliondel
 */

static 
sxu_open(dev_t parent)
{
	int		ret, err = 0;

	SL_printf("X_opn|");
	ret = lockl(&sxu_lock, LOCK_SHORT);
	if (sxu_opencnt++ > 0) 
	{
		if (ret == LOCK_SUCC)
		    unlockl(&sxu_lock);
		return err;
	}

	if (err = fp_opendev(parent, DREAD|DWRITE|DKERNEL, 0, 0, &sxu_fp)) 
	{
		sxu_opencnt = 0;
		if (ret == LOCK_SUCC)
		    unlockl(&sxu_lock);
		return err;
	}
	if (ret == LOCK_SUCC)
		unlockl(&sxu_lock);

	if (!err && !sxu_assist &&
	    (err = fp_ioctl(sxu_fp,EU_ASSIST,(unsigned char *)&sxu_assist, 0)))
		sxu_close();
	return err;

} /* sxu_open */

/*
 * FUNCTION: 	sxu_close
 *
 * PURPOSE:	Disable adapter.
 *
 * RETURN:
 *	0:	On success.
 *	err:	Result of fp_close().
 *
 * EXTERNAL PROCEDURES CALLED:
 *	lockl		unlockl		fp_close
 *
 * DESCRIPTION:
 *		Disable adapter if it is the last call.
 * CALLED BY:
 *	slionconfig	slionadd	sliondel	sxu_open
 */
static 
sxu_close()
{
	int		ret, err = 0;

	SL_printf("X_cls|");
	ret = lockl(&sxu_lock, LOCK_SHORT);
	if (--sxu_opencnt > 0) 
	{
		if (ret == LOCK_SUCC)
		    unlockl(&sxu_lock);
		return err;
	}

	/* don't want sxu_open to see a negative count */
	sxu_opencnt = 0;			

	/* make sure we have an sxu_fp in case too many 
	* closes were called 
	*/
	if (sxu_fp) 
	{
		err = fp_close(sxu_fp);
		sxu_fp = 0;
	}

	if (ret == LOCK_SUCC)
		unlockl(&sxu_lock);

	return err;

} /* sxu_close */

/*
 * FUNCTION: 	sxu_setpos
 *
 * PURPOSE:	Set to the pos register.
 *
 * INPUT:
 *	where:	The slot number that the adapter is connected.
 *	which:	The register number.
 *	how:	The value.
 *
 * RETURN:
 *	0:	On success
 *	err:	Result of fp_ioctl().
 *
 * EXTERNAL PROCEDURES CALLED:
 *	fp_ioctl
 *
 * DESCRIPTION:
 * 		Set the value(how) to pos register specified by 
 *		input parameters(where and which).
 *		
 * CALLED BY:
 *	slionadd	sliondel
 */

static 
sxu_setpos(uchar where, uchar which, uchar how)
{
	struct euposcb	arg;

	SL_printf("X_spos|");
	arg.slot = where;
	arg.reg = which;
	arg.value = how;

	return fp_ioctl(sxu_fp, EU_SETPOS, (unsigned char *)&arg, 0);

} /* sxu_setpos */

/*
 * FUNCTION: 	sxu_getpos
 *
 * PURPOSE:	Read pos register
 *
 * INPUT:
 *	where:	The slot number that the adapter is connected.
 *	which:	The register number
 * OUTPUT:
 *	value:	The value of register specified by input parameter which.
 *
 * RETURN:
 *	0:	On success
 *	err:	Result of fp_ioctl()
 *
 * EXTERNAL PROCEDURES CALLED:
 *	fp_ioctl
 *
 * DESCRIPTION:
 *		Read pos register specified by input parameters(where and
 *		which)and it puts into the output parameter(value).	
 *
 * CALLED BY:
 *	slionconfig	slionadd	sliondel
 */
static 
sxu_getpos(uchar where, uchar which, uchar *value)
{
	struct euposcb 	arg;
	int 		err;

	SL_printf("X_gpos|");

	arg.slot = where;
	arg.reg = which;
	err = fp_ioctl(sxu_fp, EU_GETPOS, (unsigned char *)&arg, 0);
	*value = arg.value;

	return err;

} /* sxu_getpos */

/*
 * FUNCTION: 	sxu_mod64
 *
 * PURPOSE:	Specifies partition.
 *
 * INPUT:
 *	slot:	The slot number.
 *	pos3:	The value of partition.
 *	cmd:	Command(EU_ADD64 or EU_DEL64).
 *
 * RETURN:
 *	0:	On success
 *	err:	Result of fp_ioctl().
 *
 * EXTERNAL PROCEDURES CALLED:
 *	fp_ioctl
 *
 * DESCRIPTION:
 * 		Specifies the partition indicated by parameter(slot) 
 *		based on the cmd. 
 *
 * CALLED BY:
 *	slionadd	sliondel
 */
static 
sxu_mod64(int slot, int pos3, int cmd)
{
	struct eu64cb	arg;					/*eu.h*/

	SL_printf("X_m64|");
	arg.slot = slot;
	arg.partition = pos3>>1;

	return fp_ioctl(sxu_fp, cmd, (unsigned char *)&arg, 0);

} /* sxu_mod64 */

/*
 * FUNCTION: 	slioc_slpx
 *
 * PURPOSE:	Put in/take out sync channel in local loop_back mode.
 *
 * INPUT:
 *	ld:	Pointer to sliondata structure.
 *	cmd:	Command(LI_SLP1 or LI_SLP0).
 *
 * OUTPUT:
 *	result:	0:	0xff is set to diag_comm if command is LI_SLP1 .
 *			0x0 is set to diag_comm if command is LI_SLP0.
 *		1:	Diag_comm value is not zero in command LI_SLP1.
 *		2:	Diag_comm value is not 0xff in command LI_SLP0.
 *
 * RETURN:
 *	0:	On success.
 *
 *	EIO:	I/O error.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_pio
 *
 * EXTERNAL PROCEDURES CALLED:
 *	timeout
 *
 * DESCRIPTION
 *	Synchronous channel local loop back_mode.
 *	This stops normal communications with the cluster box for a 
 *	given synchronous channel so that the card loop-back tests
 *	can be run without screwing up the sync port on the card.
 *
 * CALLED BY:
 *	slion_ioctl(LI_SLP0/1)
 */
static int 
slioc_slpx(struct sliondata *ld, int *result, int cmd)
{
    DoingPio;
    uchar	  *BusMem;
    uchar 	  ii;

    /* Put M_IOCACK into the queue if mask 
     * is reset: instead of e_wakeup
     */
    str_lionp_t	  tp = (str_lionp_t)ld->tp;
    mblk_t 	  *mp = tp->t_ctloutmsg;
    struct iocblk *iocp = (struct iocblk *)mp->b_rptr;

    SL_printf("sl_slpx|");

    tp->t_slpxtmr = 0;
    *result = 0;
    ATT_BD(ld);
    if (cmd == LI_SLP1) 
    {
	StartPio(slion_pio, ld, DET_BD(); return EIO);
	ii = RD_CHAR(((ipmem)ld->board)->diag_comm);
	EndPio();
	if (ii != 0)
	    *result = 1;
	else {
	    StartPio(slion_pio, ld, DET_BD(); return EIO);
	    WR_CHAR(((ipmem)ld->board)->diag_comm, 0xff);
	    EndPio();
	}
    } else {
	StartPio(slion_pio, ld, DET_BD(); return EIO);
	ii = RD_CHAR(((ipmem)ld->board)->diag_comm);
	EndPio();
	if (ii != 0xff)
	    *result = 2;
	else {
	    StartPio(slion_pio, ld, DET_BD(); return EIO);
	    WR_CHAR(((ipmem)ld->board)->diag_comm, 0);
	    EndPio();
	}
    }
    DET_BD();

    /* for 10ms++.  Wait for command to take effect */
    if (cmd == LI_SLP1 && *result == 0) 
    {
	/* Build M_IOCACK message */
	mp->b_datap->db_type = M_IOCACK;
	iocp->ioc_count = sizeof(int);
	*(int *)mp->b_cont->b_rptr = *result;
	mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(int);
	if(!(tp->t_slpxtmr))
		tp->t_slpxtmr = timeout(slion_wake_tmr, (caddr_t)ld, hz/100);
    }
    return(0);

} /* slioc_slpx */

/*
 * FUNCTION: 	slioc_dslp
 *
 * PURPOSE:	Runs the asynchronous port loop_back test.
 *
 * INPUT:
 *	ld:	Pointer to sliondata structure.
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	0:	On success.
 *	EIO:	I/O error.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_pio	dslp_tmr_f
 *
 * EXTERNAL PROCEDURES CALLED:
 *	timeout
 *
 * DESCRIPTION
 *	The tests are actually run in async mode due to the nature of 
 *	the USART.  The input is a structure which contains 16 bytes 
 *	of data to be looped, and space for the input data and terminal 
 *	counts(how many bytes transmitted/received).
 *	
 *
 * CALLED BY:
 *	slion_ioctl(LI_DSLP)
 */

#define SYLPDLY1	20
#define SYLPDLY2	20
#define SYLPDLY3	20

static int 
slioc_dslp(struct sliondata *ld)
{
	DoingPio;
	uchar		*BusMem, tmp_char;
	ipmem 		board = (ipmem)ld->board;
	volatile int 	ii;

	str_lionp_t	tp = (str_lionp_t)ld->tp;
	queue_t		*q = WR(tp->t_ctl);
	mblk_t		*mp = tp->t_ctloutmsg;
	struct slp_data *slp = (struct slp_data *)mp->b_cont->b_rptr;

	SL_printf("sl_dslp|");

	/* result is initially 0 */
	slp->result = 0;

	ATT_BD(ld);

	StartPio(slion_pio, ld, DET_BD(); return EIO);
	for (ii=0 ; ii<16 ; ii++)
		WR_CHAR(board->diag_comm+ii+2, slp->out_data[ii]);
	WR_CHAR(board->diag_comm+1, 0x22);
	EndPio();

	StartPio(slion_pio, ld, DET_BD(); return EIO);
	tmp_char = RD_CHAR(board->diag_comm+1);
	EndPio();
	
	tp->t_dslpcnt = 0;
	if(tmp_char)
	{
		tp->t_liflg = DSLPDLY1;
		tp->t_dslptmr = timeout(dslp_tmr_f, (caddr_t)ld,hz/100);
		DET_BD();
		return (0);
	}

	StartPio(slion_pio, ld, DET_BD();return EIO);
	tmp_char = RD_CHAR(board->diag_stat);
	EndPio();

	if(!(tmp_char & 0x80))
	{
		tp->t_liflg = DSLPDLY2;
		tp->t_dslptmr = timeout(dslp_tmr_f, (caddr_t)ld,hz/100);
		DET_BD();
		return (0);
	}

	StartPio(slion_pio, ld, DET_BD(); return EIO);
	for (ii=0 ; ii<16 ; ii++)
		slp->in_data[ii] = RD_CHAR(board->diag_stat+ii+2);
	slp->out_count = RD_CHAR(board->diag_stat) & 0x7f;
	slp->in_count = RD_CHAR(board->diag_stat+1) & 0x7f;
	WR_CHAR(board->diag_stat, 0);
	EndPio();

	/* cleanup */
	DET_BD();
	tp->t_ctloutmsg = (mblk_t *)0;

	/* send message upstream with result */
	mp->b_datap->db_type = M_IOCACK;
	qreply(q, mp);

	return(0);

} /*slioc_dslp */

/*
 * FUNCTION: 	dslp_tmr_f
 *
 * PURPOSE:	Read/write data in the board for LI_DSLP cmnd.
 *
 * INPUT:
 *	ld:	Pointer to sliondata structure.
 *
 * RETURN:
 *	0:	On success.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_pio
 *
 * EXTERNAL PROCEDURES CALLED:
 *	timeout
 *
 * DESCRIPTION
 *	The tests are actually run in async mode due to the nature of 
 *	the USART.  The input is a structure which contains 16 bytes 
 *	of data to be looped, and space for the input data and terminal 
 *	counts(how many bytes transmitted/received).
 *	To wait for the I/O on the board to take effect, timer is set. 
 *	If all io has been finished or  error has been occured, a message
 *	is put into read queue.
 *	
 * CALLED BY:
 *	Via timeout in slioc_dslp.
 */
void
dslp_tmr_f(struct sliondata *ld)
{
	DoingPio;
	uchar		*BusMem, tmp_char;
	volatile int	ii;
	ipmem 		board = (ipmem)ld->board;

	str_lionp_t	tp = (str_lionp_t)ld->tp;
	queue_t		*q = WR(tp->t_ctl);
	mblk_t 		*mp = tp->t_ctloutmsg;
	struct iocblk	*iocp = (struct iocblk *)mp->b_rptr;
	struct slp_data *slp = (struct slp_data *)mp->b_cont->b_rptr;


#ifdef SLI_TRACE
    if(DBFLG & LVL3)
	SL_printf("dslp_offlv|");
#endif

	ATT_BD(ld);
	tp->t_dslpcnt++;
	switch(tp->t_liflg)
	{
	    case DSLPDLY1:
		StartPio(slion_pio, ld,  goto nack;);
		tmp_char = RD_CHAR(board->diag_comm+1);
		EndPio();
		
    		if ((tp->t_dslpcnt < SYLPDLY1) && tmp_char) 
		{
			tp->t_dslptmr = timeout(dslp_tmr_f,(caddr_t)ld,hz/100);
			DET_BD();
			return;
		}
    		if (tp->t_dslpcnt == SYLPDLY1) 
		{
			slp->result = 1;
			break;
		}
		else
		{
			tp->t_dslpcnt = 0;
			tp->t_liflg = DSLPDLY2;
		}
		/* CAUTION:  fall through to DSLPDLY2 */

	    case DSLPDLY2:
		StartPio(slion_pio, ld,  goto nack;);
		tmp_char = RD_CHAR(board->diag_stat);
		EndPio();

		if((tp->t_dslpcnt < SYLPDLY2) && !(tmp_char & 0x80)) 
		{
			tp->t_dslptmr = timeout(dslp_tmr_f,(caddr_t)ld,hz/100);
			DET_BD();
			return;
		}
		if (tp->t_dslpcnt == SYLPDLY2) 
		{
			tp->t_dslpcnt = 0;
			tp->t_liflg = DSLPDLY3;
			StartPio(slion_pio, ld,  goto nack;);
			WR_CHAR(board->diag_comm+1, 0xff);
			EndPio();
		}
		else
		{
			break;
		}
		/* CAUTION:  fall through to DSLPDLY3 */

	    case DSLPDLY3:
		StartPio(slion_pio, ld,  goto nack;);
		tmp_char = RD_CHAR(board->diag_stat);
		EndPio();

		if((tp->t_dslpcnt < SYLPDLY3) && !(tmp_char & 0x80)) 
		{
			tp->t_dslptmr = timeout(dslp_tmr_f, (caddr_t)ld,hz/100);
			DET_BD();
			return ;
		}
		if (tp->t_dslpcnt == SYLPDLY3) 
			slp->result = 2;
		else
			slp->result = 3;
		break;

	    default:
nack:
		/* cleanup */
		tp->t_ctloutmsg = (mblk_t *)0;
		tp->t_liflg = 0;
		tp->t_dslpcnt = 0;

		/* send message up indicating failure */
		mp->b_datap->db_type = M_IOCNAK;
		iocp->ioc_error = EIO;
		qreply(q, mp);

		DET_BD();
		return;
	}

	if ((slp->result == 0) || (slp->result == 3))
	{
		StartPio(slion_pio, ld,  goto nack;);
		for (ii=0 ; ii<16 ; ii++)
		        slp->in_data[ii] = RD_CHAR(board->diag_stat+ii+2);
		slp->out_count = RD_CHAR(board->diag_stat) & 0x7f;
		slp->in_count = RD_CHAR(board->diag_stat+1) & 0x7f;
		WR_CHAR(board->diag_stat, 0);
		EndPio();
	}

	/* cleanup */
	tp->t_ctloutmsg = (mblk_t *)0;
	tp->t_liflg = 0;
	tp->t_dslpcnt = 0;

	/* send message up indicating result */
	mp->b_datap->db_type = M_IOCACK;
	qreply(q,mp);

	DET_BD();
	return;

} /* dslp_tmr_f */

/*
 * String copy for virtual terminal/Xparent printer.x.
 * Called by slion_ioctl(LI_SETVT LI_SETXP).
 */
static 
sxvscpy(char *dst, char *src, int len)
{
    int		ii;

    SL_printf("sxvscpy|");
    for (ii=0 ; ii<len && *src != 0xff ; ii++)
	*dst++ = *src++;
    while (ii++<len)
	*dst++ = 0xff;
    return(0);

} /* sxvscpy */

/*
 * FUNCTION: 	slioc_pres
 *
 * PURPOSE:	Verifies the presence of an adapter.
 *
 * INPUT:
 *	ld:	Pointer to sliondata structure.
 *
 * OUTPUT:
 *	result:	Result of process
 *		0:	POS is OK.
 *		1:	Error.
 *		2:	POSID error.
 *		3:	POS[2] not correct.
 *		4:	Invalid address bit set.
 *		5:	POS[3] not correct.
 *
 * RETURN:
 *	0:	On success.
 *
 *	ENODEV:	No adapter.
 *	xerr:	Result of sxu_open()/sxu_getpos()/sxu_close().
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_pio	sxu_open	sxu_getpos	sxu_close
 *
 * EXTERNAL PROCEDURES CALLED:
 *	io_att		io_det
 *
 * DESCRIPTION:
 *	This really means that it check to see that the POS registers 
 *	haven't died, because we wouldn't be able to get to the adapter 
 *	unless it was configured, and it couldn't be configured if the 
 *	POSID was not valid.  The pos[2] and pos[3] values are checked 
 *	to see if they are valid, which does have its merits, since they 
 *	are not really checked (at this time) in lionadd.
 *
 * CALLED BY:
 *	slion_ioctl(LI_PRES)
 */
static 
slioc_pres(struct sliondata *ld, int *result)
{
    DoingPio;
    uchar		*pos_ptr, pos[4];
    struct sli_adap	*adap;

    SL_printf("sl_pres|");

    if (!(adap = adap_list[ADP(ld->tp->t_dev)])) 
    {
	*result = 1;
	return ENODEV;
    }

    if (adap->inxbox) 
    {
	int	xerr, yerr, ii;
	ii = 0;			/* dummy */
/**** At now commented: because sxu_xx() is inhibited.****
*	if (xerr = sxu_open(adap->parent)) 
*	{
*	    *result = 1;
*	    return xerr;
*	}
*	for (ii=0 ; ii<4 ; ii++)
*	    if (xerr = sxu_getpos(adap->slot, ii, &pos[ii]))
*		break;
*	yerr = sxu_close();
*	if (xerr || (xerr=yerr)) 
*	{
*	    *result = 1;
*	    return xerr;
*	}
********************************************************/
	return *result = 0;
    } 
    else 
    {
	int err = 0;
	pos_ptr = io_att(adap->iseg, adap->ibase);
	StartPio(slion_pio, 0, err = EIO; break);
	pos[0] = BUSIO_GETC(pos_ptr);
	pos[1] = BUSIO_GETC(pos_ptr+1);
	pos[2] = BUSIO_GETC(pos_ptr+2);
	pos[3] = BUSIO_GETC(pos_ptr+3);
	EndPio();
	io_det(pos_ptr);
	if (err) 
	{
	    *result = 1;
	    return err;
	}
    }

    if (pos[0] != 0xfd || pos[1] != 0x61)  /* Check the POSID */
    {
	*result = 2;
	return 0;
    }
    if ( (pos[2] & (CC_INT_BASE|CC_CRD_ENAB)) != (CC_INT_BASE|CC_CRD_ENAB)) 
    {
	*result = 3;
	return 0;
    }
    if (pos[3] & 0x81) 			/* Invalid address bit set? */
    {
	*result = 4;
	return 0;
    }
    if ((int)adap->board != (pos[3] << 17)) 	/* pos[3] correct ? */
    {
	*result = 5;
	return 0;
    }
    return *result = 0;			/* Card is there and pos is Okay */

} /* slioc_pres */

/*
 * FUNCTION: 	slioc_dram
 *
 * PURPOSE:	Check some data integrity of the DRAM.
 *
 * INPUT:
 *	ld:	Pointer to sliondata structure.
 *
 * OUTPUT:
 *	result:	Result of process.
 *		0:	OK.
 *		2:	Error in copyright message.
 *		3:	Problem in ring structure test..
 *		4:	Problem in DRAM.
 *
 * RETURN:
 *	0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_pio	sl_dram_wr	slioc_trng
 *
 * DESCRIPTION:
 *	First the copyright message is compared to the supposedly correct 
 *	value.  Then  each of the ring buffer structures is checked. 
 *	And then write/read  checks are done in the main channel transmit 
 *	ring buffers.  This test cannot be run if there is activity on 
 *	the cluster, since output data is overwritten.
 *
 * CALLED BY:
 *	slion_ioctl(LI_DRAM)
 */
static 
slioc_dram(struct sliondata *ld, int *result)
{
    DoingPio;
    uchar		*BusMem;
    struct sliondata	*t_ld;
    volatile uint	*ptr;
    int			copyrt[8], virt_bd, port, state;
    volatile	int	ii;

    SL_printf("sl_drm|");

    ATT_BD(ld);
    ptr = (uint *)MK_ADDR(((ipmem)ld->board)->copyrt[0]);
    ii = 0;
    StartPio(slion_pio, ld, DET_BD(); return EIO);
    for ( ; ii<8 ; ii++, ptr++)
	copyrt[ii] = BUS_GETL(ptr);
    EndPio();
    DET_BD();

    for (ii=0 ; ii<8 ; ii++)	/* Make sure copyright message is correct */
	if (copyrt[ii] != lion_copyrt[ii]) 
	{
	    *result = 2;
	    return 0;
	}

    if (slioc_trng(ld, 1)) 
    {
	*result = 3;
	return 0;
    }

#ifdef FEATURES
    t_ld = (struct sliondata *)ld->prms->vtp->t_hptr;
    if (slioc_trng(t_ld, 1)) 
    {
	*result = 3;
	return 0;
    }

    t_ld = (struct sliondata *)ld->prms->xtp->t_hptr;
    if (slioc_trng(t_ld, 0)) 
    {
	*result = 3;
	return 0;
    }
#endif /* FEATURES */

    if (sl_dram_wr(ld)) 
    {
	*result = 4;
	return 0;
    }

    return *result = 0;

} /* slioc_dram */

/*
 * FUNCTION: 	sl_dram_wr
 *
 * PURPOSE:	Check a write/read of the transmit data ring buffer DRAM.
 *
 * INPUT:
 *	ld:	Pointer to sliondata structure.
 *
 * RETURN:
 *	0:	On success.
 *	1:	Problem in a DRAM.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_pio	slion_do_comms
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_disable	i_enable
 *
 * DESCRIPTION:
 *	This will not completely verify that the DRAM is good, but will 
 *	give an error in most cases since when there is a problem in a 
 *	DRAM, usually the entire module is effected.  The problem is that 
 *	we cannot write to all locations of the DRAM and check the value 
 *	since the firmware might change the values and/or crash due to bad 
 *	pointers in structures, therefore, we cannot check for bit errors 
 *	in the entire DRAM.  Note that this routine is called on a per 
 *	port basis.  The idea of this test is to generate two fairly 
 *	pseudo-random sequences with the different bit positions having
 *	different cycle rates.
 *
 * CALLED BY:
 *	slioc_dram
 */
static 
sl_dram_wr(struct sliondata *ld)
{
    DoingPio;
    uchar		*BusMem;
    ushort		offset;
    int			err=0;
    volatile int	ii;
    volatile uchar	data;
    int			old_intr;

    SL_printf("sl_drm_wr|");

    old_intr = i_disable(INT_TTY);
    ld->flags |= XMIT_FLAG|COMM_FLAG|DIAG_FLAG;
    ld->tp->t_busy = 1;

    ATT_BD(ld);
    StartPio(slion_pio, ld, DET_BD(); i_enable(old_intr); return EIO);
    /* For each chunk of data in the ring buffer, */
    for (offset=ld->tr_bb; offset <= ld->tr_ba; offset += ld->prms->tr_width) 
    {
	for (data=0x23,ii=0 ; ii<(ld->prms->tr_width-2) ; ii++, data += 0x31)
	    WR_CHAR(ld->board+offset+2+ii, data);
	for (data=0x23,ii=0 ; ii<(ld->prms->tr_width-2) ; ii++, data += 0x31)
	    if (RD_CHAR(ld->board+offset+2+ii) != data) 
	    {
		err = 1;
		goto endpio;
	    }
	for (data=0x30,ii=0 ; ii<(ld->prms->tr_width-2) ; ii++, data += 0x13)
	    WR_CHAR(ld->board+offset+2+ii, data);
	for (data=0x30,ii=0 ; ii<(ld->prms->tr_width-2) ; ii++, data += 0x13)
	    if (RD_CHAR(ld->board+offset+2+ii) != data) 
	    {
		err = 1;
		goto endpio;
	    }
    }
endpio:
    EndPio();
    DET_BD();
    i_enable(old_intr);
    ld->flags &= ~DIAG_FLAG;
    ld->tp->t_busy = 0;
    slion_do_comms(ld);
    return err;

} /* sl_dram_wr */

/*
 * FUNCTION: 	slioc_trng
 *
 * PURPOSE:	Checks out various variables in the ring buffer structure.
 *
 * INPUT:
 *	ld:	Pointer to sliondata structure.
 *	recv:	1: test the input ring structure.
 *		0: no test.
 *
 * RETURN:
 *	0:	On success.
 *	1:	Error..
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_pio
 *
 * DESCRIPTION:
 *	This routine checks out various variables in the ring buffer 
 *	structure to insure that they match previously read and
 *	stored values and that they are relatively valid. If recv is 
 *	true, the input ring structure is tested, otherwise it is not.
 *	This is due to the transparent printer not having a receive data 
 *	ring buffer.
 *
 * CALLED BY:
 *	slion_dram
 */
static 
slioc_trng(struct sliondata *ld, int recv)
{
    DoingPio;
    uchar	*BusMem;
    int		rc = 0;
    ushort	val;
    int		old_intr;

    SL_printf("sl_trng|");

    ATT_BD(ld);
    old_intr = i_disable(INT_TTY);
    StartPio(slion_pio, ld, DET_BD(); i_enable(old_intr); return EIO);

    rc |= (ld->tr_bb != RD_SHORT(&ld->tr_buf->buf_beg));
    rc |= (ld->tr_ba != RD_SHORT(&ld->tr_buf->buf_almost));
    rc |= (ld->prms->tr_width != RD_SHORT(&ld->ip->tr_width));
    rc |= (ld->tr_ba != (ld->tr_bb +
		 (ld->prms->tr_width * (RD_SHORT(&ld->tr_buf->buf_max)))));
    rc |= ( (ld->tr_ba + ld->prms->tr_width) != 
					RD_SHORT(&ld->tr_buf->buf_end));
    val = RD_SHORT(&ld->tr_buf->buf_head);
    rc |= ( (val - ld->tr_bb) % ld->prms->tr_width) != 0;
    val = RD_SHORT(&ld->tr_buf->buf_tail);
    rc |= ( (val - ld->tr_bb) % ld->prms->tr_width) != 0;

    if (recv) 
    {
	rc |= (ld->rc_bb != RD_SHORT(&ld->rc_buf->buf_beg));
	rc |= (ld->rc_ba != RD_SHORT(&ld->rc_buf->buf_almost));
	rc |= (ld->prms->rc_width != RD_SHORT(&ld->ip->rc_width));
	val = RD_SHORT(&ld->rc_buf->buf_max);
	rc |= (ld->rc_ba != (ld->rc_bb + (ld->prms->rc_width * val)));
	val = RD_SHORT(&ld->rc_buf->buf_end);
	rc |= ( (ld->rc_ba + ld->prms->rc_width) != val);
	val = RD_SHORT(&ld->rc_buf->buf_head);
	rc |= ( (val - ld->rc_bb) % ld->prms->rc_width) != 0;
	val = RD_SHORT(&ld->rc_buf->buf_tail);
	rc |= ( (val - ld->rc_bb) % ld->prms->rc_width) != 0;
    }
    EndPio();
    i_enable(old_intr);
    DET_BD();

    return rc;

} /* slioc_trng */

/********************************************************************
 *	Lower interface:	corresponds to lionl.c
 ********************************************************************/

/*
 * FUNCTION:	slionproc
 *
 * PURPOSE:	TTY process management.
 *
 * INPUT:
 *      tp:     Pointer to str_lion structure.
 *	cmd:	Request type.
 *
 * RETURN:
 *	0:	On success.
 *	ENODEV:	No such port(line) or adapter(slot).
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_err	slion_pio	slion_send	slion_set_gcomm
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_sched		i_enable	i_disable
 *
 * DESCRIPTION:
 *      The following services are processed:
 *		T_SUSPEND, T_RESUME, T_WFLUSH, T_OUTPUT, T_RFLUSH, T_BLOCK 
 *		T_UNBLOCK.
 *
 *	When M_STOP, M_START, M_STOPI or M_STARTI ioctl is received in 
 *	slionwput(), T_SUSPENd, T_RESUME,T_UNBLOCK or T_BLOCK is processed.
 *
 *	When M_DATA or M_DELAY ioctl is received in slionwsrv(), T_OUTPUT 
 *	or T_SUSPEND is processed.
 *
 *	When M_FLUSH ioctl is received in slionwput, T_WFLUSH/T_RFLUSH
 *	is processed via slion_flush().
 *
 *	When TS_PROC command is received in slionservice, the service 
 *	based on the specified argument is done.
 *
 *	When the output data is transmitted in the  off_level via
 *	slion_do_comms(), T_OUTPUT service is done.
 *
 * CALLED BY:
 *	slionwput	slionwsrv	slion_flush 	slion_termios_set	
 *	slion_do_comms 	slionservice
 *	slion_termiox_set
 */
static int
slionproc(str_lionp_t tp, int cmd)
{
	DoingPio;
	int 		 result = 0;
	struct sliondata *ld = (struct sliondata *)tp->t_hptr;
	uchar 		 *BusMem;
	int		 old_intr;
	uchar		 port, boxn, vcs, box_vector;

	Enter(TRC_LION(TTY_PROC), tp->t_dev, (int)tp, cmd, 0, 0);

	if (!ld) 
	{
		slion_err(resource, ERRID_TTY_PROG_PTR, 2);
		Return(ENODEV);
	}

	switch (cmd) 
	{
	    case T_WFLUSH:
		SL_printf("T_WF|");
		ld->txcnt = 0;
		/* Flush the message in progress */
		if (tp->t_outmsg)
		{
		    freemsg(tp->t_outmsg);
		    tp->t_outmsg = 0;
		}
		/* fall through */

            case T_RESUME:
		SL_printf("T_RE(%x)",ld->flags);
		/* slion_set_gcomm(ld, GC_CLR_INHIB); *Clear 
		 * host defined  inhib.
		 * No check t_stop flg. If both side is BLOCK
		 * status, it enters deadlock status.*/
		slion_set_gcomm(ld, GC_ZAP_INHIB); 
		tp->t_stop = 0;
		qenable(WR(ld->tp->t_ctl));

            case T_OUTPUT:
        /* OK - here's the deal.  tp->t_busy tracks whether anything is
        ** on the adapter.  We turn it on anytime we put anything out
        ** there, and turn it off when we get an empty interrupt.  This
        ** is the best we can hope for right now in regards to psx_wait
        ** and other things that want to know when the data has drained.
        ** BUT we don't do transmit stuff if ANY of the busy flags are
        ** set in ld->flags, since these flags track the busy state for
        ** diagnostics, in-line commands which do things like set the
        ** baud rate and state of RTS/DTR, and also for the flush stuff.
        ** If we are flushing, we don't want to send any data until we
        ** have sent the "End Flush" in-line command to the box.
        ** Whenever a BUSY_MASK flag is cleared, slion_do_comms should
        ** be called, which does any undone work and will eventually
        ** call slionproc(...,T_OUTPUT) if we're ready to do more transmit 
	** output.
	*/

		SL_printf("T_OT|");
		/* tp->t_busy is the same as XMIT_FLAG which is in BUSY_MASK */
		if (tp->t_stop || (ld->flags & BUSY_MASK))
		    break;

		ATT_BD(ld);

		if (ld->txcnt)
		slion_send(ld, tp, BusMem);		/* tp->t_busy = 1; */
		DET_BD();
		break;

            case T_SUSPEND:
		if(!(tp->t_stop))
		{
			SL_printf("T_SP(%x)",ld->flags);
			slion_set_gcomm(ld, GC_SET_INHIB);
			tp->t_stop = 1;	/* WHAT ELSE NEEDS TO BE DONE ? */
		}
		break;

            case T_RFLUSH:
		SL_printf("T_RF|");
		if (tp->t_channel != CHAN_XPAR) 
		{
		    ATT_BD(ld);
		    StartPio(slion_pio, ld, result = EIO; break);
		    WR_SHORT(&ld->rc_buf->buf_tail, 
					RD_SHORT(&ld->rc_buf->buf_head));
		    EndPio();
		    DET_BD();
		}
		break;

            case T_BLOCK:
		SL_printf("T_BK|");
		ld->block = 1;
		break;

            case T_UNBLOCK:
		SL_printf("T_UBK|");
		ld->block = 0;

	/* The old code used to do this:
	 * ATT_BD(ld); slion_recv(ld, tp, BusMem); DET_BD();
	 * Only this is a problem if we're already been down this path
	 * before, ie slion_recv call ttyinput, does UNBLOCK call --
	 * calls lionrecv, calls ttyiput ...
	 * sort of the lion chasing its own tail!
	 * So we solve this problem by scheduling an offlevel service
	 * to do the lionrecv.
	 * But when slion_offlevel is called, the interrupt type and interrupt
	 * box number was already reset in previous calling. So the
	 * corresponding box number and rx interrupt type must be set before
	 * entering off level function.
	 */
		{

			struct sli_adap *adap = adap_list[ADP(tp->t_dev)];

			port = PRT(tp->t_dev);
			boxn = (port/16) + 1;	 /* Get box numb */
			old_intr = i_disable(INTCLASS0); /* WITH SLIH !!! */
			ATT_BD(adap);

			StartPio(slion_pio, ld, result = EIO; DET_BD(); break);
			box_vector = RD_CHAR(&adap->board->int_mask); 
			vcs = RD_CHAR(&adap->board->int_vectors[port]); 
			EndPio();

			SL_printf("bx=%x vcs=%x|",box_vector,vcs);
			if(!(box_vector & boxn))
			{
				box_vector |= boxn;
				SL_printf("BX(%x)",box_vector);
				StartPio(slion_pio, ld, result = EIO; 
							DET_BD();  break);
				WR_CHAR(&adap->board->int_mask, (box_vector));
				EndPio();
			}
			if(!(vcs & CC_INT_RCMAIN))
			{
				vcs |= (uchar)(CC_INT_RCMAIN);
				SL_printf("VC(%x)",vcs);
				StartPio(slion_pio, ld, result = EIO; 
							DET_BD(); break);
				WR_CHAR(&adap->board->int_vectors[port], vcs);
				EndPio();
			}
			DET_BD();

			i_sched((struct intr *)adap);
			i_enable(old_intr);
	     	}

		break;

	    default:
		result = -1;
		break;
	}

	Return(result);

} /* slionproc */

/*
 * FUNCTION:	slionservice
 *
 * PURPOSE:	TTY main service management.
 *
 * INPUT:
 *      tp:     Pointer to str_lion structure.
 *	cmd:	Request type.
 *	varg:	Request subtype.
 *
 * OUTPUT:
 *	varg:	Result of Get command.
 *
 * RETURN:
 *	0:	On success.
 *	ENODEV:	No such port(line) or adapter(slot).
 *	EINVAL:	Invalid value (line definitions : baud, parity..).
 *
 * INTERNAL PROCEDURES CALLED:
 *	slionproc  slion_err  slion_comm  sscan_baud_table
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_disable  i_enable
 *
 * DESCRIPTION:
 *	This function pushes values down into and gets them from
 *      the chip.
 *      It is called by other parts of the driver on some M_IOCTL
 *      or M_CTL, or M_BREAK processing.
 *
 *	- slionwsrv routine when M_BREAK ioctl is received.
 *
 *	- slion_termios_set routine when termios ioctl are received.
 *
 *	- slion_termiox_set routine when termiox ioctl are received.
 *
 *	- slionadd routine on configuration case.
 *
 *	- slionopen routine indirectly from openDisc_open on open 
 *		line case.
 *
 *	- slionioctl routine when ioctl is received.
 *
 * CALLED BY:
 *	slionwserv	slionadd	slionopen	slion_termios_set
 *	slion_ioctl	slion_termiox_set
 */
static int
slionservice(str_lionp_t tp, enum service_commands cmd, void *varg)
{
    DoingPio;
    struct sliondata	*ld = (struct sliondata *)tp->t_hptr;
    char 		*ptr;
    int 		result = 0, ii;
    int 		old_intr;
    int 		arg = (int)varg;
    uchar 		*BusMem;

    Enter(TRC_LION(TTY_SERVICE), tp->t_dev, (int)tp, cmd, varg, 0);

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("\nSsrv.%x(cmd=%x)",PRT(tp->t_dev),cmd);
#endif

    if (!ld) 
    {
	slion_err(resource, ERRID_TTY_PROG_PTR, 0x80);
	Return(ENODEV);
    }

    switch (cmd) 
    {
       case TS_PROC:
#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TPROC|");
#endif

	old_intr = i_disable(INT_TTY);
	result = slionproc(tp, arg);
	i_enable(old_intr);
	break;

       case TS_SCONTROL:			/* CHECK CURRENT STATUS */
#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TSCTL|");
#endif

	if ( !(arg & TSDTR) == ld->prms->dtr )                  /*TSDTR:1 */
            slion_comm(ld, (ld->prms->dtr ^= 1) ? ADTR : DDTR);
        if ( !(arg & TSRTS) == ld->prms->rts )                  /*TSRTS:2 */
	    slion_comm(ld, (ld->prms->rts ^= 1) ? ARTS : DRTS);

	/* if dropping dtr, pause to allow modem to see it */
	if ( !ld->prms->dtr )
	    slion_comm(ld, PAUS | 2);
	break;

       case TS_GCONTROL:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TGCTL|");
#endif

	*(int *)varg = (ld->prms->dtr?TSDTR:0) | (ld->prms->rts?TSRTS:0);
	break;

       case TS_GSTATUS:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TGSTAT|");
#endif

	*(int *)varg = (ld->prms->dcd?TSCD:0) | (ld->prms->cts?TSCTS:0);
	break;

       case TS_SBAUD:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TSBAUD|");
#endif

	/* Check speed code 0. If yes, drop DTR/DTS */
	if(!(arg))
	{
		if (ld->prms->dtr)
		{
			slion_comm(ld, DDTR);
			/* if dropping dtr, pause to allow modem to see it */
			slion_comm(ld, PAUS | 2);
		}
		if (ld->prms->rts)
			slion_comm(ld, DRTS);
		ld->prms->rts = ld->prms->dtr = 0;
		ld->prms->baud = 0;
		break;
	}

	if ( (ii=sscan_baud_table(arg)) != -1) 
	{
	    slion_comm(ld, BAUD | ii);
		ld->prms->baud =  arg;
	} else
	    result = EINVAL;
	break;

       case TS_GIBAUD:
       case TS_GBAUD:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TGB/IBD|");
#endif
	*(int *)varg = ld->prms->baud;
	break;

       case TS_SBPC:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TSBPC|");
#endif
	if ( (arg&3) != ld->prms->char_size) 
	{
	    slion_comm(ld, SCLN|(arg&3));
	    ld->prms->char_size = (arg&3);
	}
	break;

       case TS_GBPC:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TGBPC|");
#endif

	*(int *)varg = ld->prms->char_size;
	break;

       case TS_SPARITY:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TSPRTY(%x)|",arg);
#endif

       switch (arg) 
       {
	       case nopar:
	       case oddonly:
		slion_comm(ld, SPAR|0);
		break;

	       case oddpar:
		slion_comm(ld, SPAR|1);
		break;

	       case evenpar:
		slion_comm(ld, SPAR|2);
		break;

	       default:
		result = EINVAL;
		break;
        }
        if (result != EINVAL)
		ld->prms->parity = arg;

	break;

       case TS_GPARITY:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TGPRTY|");
#endif
	*(int *)varg = ld->prms->parity;
	break;

       case TS_SSTOPS:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TSSTP|");
#endif

	/* stop_bits for lion are 0==1, 1==1.5, and 2==2 stopbits */
	if (arg == stop1)
	    ii = 0;		/* 1 stop bits */
	else if (arg == stop2)
	    ii = 2;		/* 2 stop bits */
	else 
	{
	    result = EINVAL;
	    break;
	}
	if (ii != ld->prms->stop_bits) 
	{
	    slion_comm(ld, SNSB|ii);
	    ld->prms->stop_bits = ii;
	}
	break;

       case TS_GSTOPS:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TGSTP|");
#endif

	*(int *)varg = ld->prms->stop_bits;
	break;

       case TS_SBREAK:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TSBRK|");
#endif

	slion_comm(ld, SBRK);
	break;

       case TS_CBREAK:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TCBRK|");
#endif

	break;

       case TS_DOPACE:
	switch (arg) 
	{
	   case DOPACE_AGAIN:			/* called by slionadd() */
#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("DAGN|");
#endif
	    if (ld->prms->dtrp)
		slion_comm(ld, DTR0);
	    if (ld->prms->dcdp)
		slion_comm(ld, DCD0);
	    if (ld->prms->rtsp)
		slion_comm(ld, RTS0);
	    if (ld->prms->ctsp)
		slion_comm(ld, CTS0);
	    if (ld->prms->xonp || ld->prms->xoff)
		slion_comm(ld, XON0);
	    if (ld->prms->xany)
		slion_comm(ld, XOF0);
	    ld->prms->dtrp = ld->prms->dcdp = ld->prms->rtsp =
		ld->prms->ctsp = ld->prms->xonp = ld->prms->xoff =
		    ld->prms->xany = 0;
	    break;

	   default:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("DDF|");
#endif

	    result = EINVAL;
	    break;
	}
	break;

       case TS_SOFTPACE:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("TSFPAC(%x)|",arg);
#endif

	switch (arg) 
	{
	   case SOFTPACE_ROFF:
	   case SOFTPACE_RSTR:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("SROF/RSTR|");
#endif

	    ld->prms->xonp = ld->prms->xany = 0;
	    slion_comm(ld, XON0);
	    break;

	   case SOFTPACE_RANY:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("SRANY|");
#endif

	    ld->prms->xany = 1;
	    ld->prms->xonp = 0;
	    slion_comm(ld, XANY);
	    break;

	   case SOFTPACE_RON:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("SRON|");
#endif

	    ld->prms->xany = 0;
	    ld->prms->xonp = 1;
	    slion_comm(ld, XON1);
	    break;

	   case SOFTPACE_LOFF:
	   case SOFTPACE_LSTR:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("SOF/STR|");
#endif

	    ld->prms->xoff = 0;
	    slion_comm(ld, XOF0);
	    break;

	   case SOFTPACE_LON:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("S1|");
#endif

	    ld->prms->xoff = 1;
	    slion_comm(ld, XOF1);
	    break;
	}
	break;

       case TS_SOFTLCHAR:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("SLCHR|");
#endif

	ptr = (char *)arg;
	slion_comm(ld, LXOC | (ld->prms->lxoc = ptr[1]));     /*^S*/
	slion_comm(ld, LXAC | (ld->prms->lxac = ptr[0]));     /*^Q*/
	break;

       case TS_SOFTRCHAR:

#ifdef SLI_TRACE
    if(DBFLG & MSRV)
	SL_printf("SRCHR|");
#endif

	ptr = (char *)arg;
	slion_comm(ld, RXOC | (ld->prms->rxoc = ptr[1]));
	slion_comm(ld, RXAC | (ld->prms->rxac = ptr[0]));
	break;

    default:
	result = -1;
	break;
    }
    Return(result);

} /* slionservice */


/*
 * FUNCTION:	slion_slih
 *
 * PURPOSE:	RX interrupt level 2 handling.
 *
 * INPUT:
 *      intr:	Pointer to the sli_slih structure.
 *
 * RETURN:
 *	INTR_SUCC: On success.
 *	INTR_FAIL: Fail interrupt.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_pio
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_reset		i_sched
 *
 * DESCRIPTION:
 *	When the second level interrupt has been occured, the posted flag 
 *	is set to the corresponding adapter. Then the adapter will be 
 *	processed by slion_offlev via scheduler.
 *
 * CALLED BY:
 *	Hardware when interrupt occur.
 */
int 
slion_slih(struct sli_slih *intr)
{
    DoingPio;
    struct sli_adap	*adap;
    uchar		int_vector;
    uchar		*BusMem;
    int			result = INTR_FAIL;
    char		type[16] = "SixtyFourPort";

    Enter(TRC_LION(TTY_SLIH), 0, (int)intr, type, 0, 0);

#ifdef SLI_TRACE
    if(DBFLG & LVL1)
	SL_printf("\nIH(%x)",intr);
#endif

    for (adap = intr->adap_chain; adap ; adap = adap->slih_next) 
    {
	ATT_BD(adap);
	StartPio(slion_pio, 0, continue);
	int_vector = RD_CHAR(&adap->board->int_mask);

#ifdef NO_HWARE_PROB
	    if (int_vector)
#endif

		WR_CHAR(adap->board->copyrt, '('); /*)*/
	EndPio();
	DET_BD();
	if (int_vector || adap->inconfig) 
	{
	    result = INTR_SUCC;
	    if (!adap->posted && !adap->inconfig) 
	    {
		adap->posted = 1;
		i_sched((struct intr *)adap);
	    }
	}
    }
    if (result == INTR_SUCC)
	i_reset((struct intr *)intr);

    Return(result);

} /* slion_slih */

/*
 * FUNCTION:	slion_offlev
 *
 * PURPOSE:	Off-level interrupt handler.
 *
 * INPUT:
 *      adap:	Pointer to sli_adap structure.
 *
 * RETURN:
 *	0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_recv  slion_err	slion_do_comms  slion_set_gcomm
 *	myttyinput  sGotHilink	slion_pio
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_enable    i_disable
 *
 * DESCRIPTION:
 *      This routine completes interruption handling rescheduled
 *      by slih.
 *      Correctly received characters are put inside M_DATA messages
 *	by slion_recv routine and status change is set to  an M_CTL 
 *	message by myttyinput routine.
 * CALLED BY:
 *	i_sched
 */ 
int 
slion_offlev(struct sli_adap *adap)
{
    DoingPio;
    struct sliondata	*ld;
    str_lionp_t		tp;
    uchar		vectors[64], vector, box_vector, accum_vec;
    int			box, port, port_last;
    volatile ushort	*vec_in, *vec_out, *vec_last;
    uchar		tmp;
    uchar		*BusMem;
    int			err, old_intr;
    queue_t		*q ;

    extern  slion_recv(struct sliondata *ld, str_lionp_t tp, uchar *BusMem);

    Enter(TRC_LION(TTY_OFFL), 0, (int)&adap->intr, 0, 0, 0);

    ATT_BD(adap);

    StartPio(slion_pio, 0, DET_BD(); Return(0));
    box_vector = RD_CHAR(&adap->board->int_mask); 
    EndPio();


    /* process input vectors */
    accum_vec = 0;

    SL_printf("\nOFL bv=%x|",box_vector);

    port = 0;
    for (box_vector &= 0xf; box_vector; box_vector >>= 1) 
    {
	if (!(box_vector & 1)) 
	{
	    port += 16;
	    continue;
	}
	vec_in = (ushort *)MK_ADDR(adap->board->int_vectors[port]);
	vec_out = (ushort *)(vectors+port);
	vec_last = vec_out + 8;

	StartPio(slion_pio, 0, DET_BD(); Return(0));
	while (vec_out < vec_last) 
	{
	    *vec_out = BUS_GETS(vec_in);
	    vec_out++;
	    vec_in++;
	}
	EndPio();

	for (port_last=port+16 ; port < port_last ; port++) 
	{
	    /*get active port**/
	    if (!(vector = vectors[port]))
		continue;

	    accum_vec |= vector;


	    SL_printf("S_ofl.%x(v=%x)",port,vector);

	    if (vector & (CC_INT_HILINK))	     /*0x40* adp lst comm */
		sGotHilink(adap, port&0x30);

	    if (vector & (CC_INT_RCMAIN|CC_INT_TXMAIN))  /* 0x01 0x04 */
	    {
		ld = adap->main+port;
		if (vector & CC_INT_RCMAIN && adap->m_ttyp[port]) 
		{
		    SL_printf("RX|");
		    tp = ld->tp;
		    tp->t_sched = 1;
		    if(!(ld->block))			
		    { 					/* T_BLOCK(TCIOFF) on*/ 
    			    tp->t_busmem = BusMem;    /* Save BusMem address */
			    slion_recv(ld, tp, BusMem);
			    q = tp->t_ctl;
			    if((q->q_count > (q->q_lowat + q->q_hiwat)/2) && 
							!(ld->block)) 
				slionproc(tp, T_BLOCK);
			    if((q->q_count <= q->q_lowat) && (ld->block)) 
				slionproc(tp, T_UNBLOCK);
		    }
		}

		if (vector & CC_INT_TXMAIN) 
		{
		    SL_printf("TX|");
                    ld->tp->t_busy = 0;
		    slion_do_comms(ld);
		}
	    }

#ifdef FEATURES
	    if (vector & (CC_INT_RCALT|CC_INT_TXALT) ) 		/* 0x02 0x08*/
	    {
		ld = adap->vger+port;
		if (vector & CC_INT_RCALT && adap->v_ttyp[port])
		{
		    if(!(ld->block))			
		    { 					/* T_BLOCK(TCIOFF) on*/ 
			    ld->tp->t_busmem = BusMem;/* Save BusMem address */
			    slion_recv(ld, ld->tp, BusMem);
		    	    q = ld->tp->t_ctl;
			    if((q->q_count > (q->q_lowat + q->q_hiwat)/2) && 
								!(ld->block)) 
				slionproc(ld->tp, T_BLOCK);
			    if((q->q_count <= q->q_lowat) && (ld->block)) 
				slionproc(ld->tp, T_UNBLOCK);
		    }
		}
		if (vector & CC_INT_TXALT) 
		{
                    ld->tp->t_busy = 0;
		    slion_do_comms(ld);
		}
	    }

	    if (vector & CC_INT_TXPAR) 			/* 0x10*/
	    {
		ld = adap->xpar + port;
		ld->tp->t_busy = 0;
		slion_do_comms(ld);
	    }
#endif /*FEATURES */

	    if (vector & (CC_INT_DCD|CC_INT_CTS) )	/* 0x20 0x80 */
	    {
		register char tmp_dss;

#ifdef FEATURES
		str_lionp_t tq;
#endif /*FEATURES */

		ld = adap->main+port;

#ifdef VPD_BOX_RESET

		StartPio(slion_pio, ld, tmp_dss = 0xAA; break);
		tmp_dss = RD_CHAR(&ld->board[BOX_VPD_BYTE0]);
		tmp_dss |= RD_CHAR(&ld->board[BOX_VPD_BYTEL]);
		EndPio();

		if (tmp_dss == 0xFF) 
		{
		    StartPio(slion_pio, ld, tmp_dss = 0; break);
		    WR_CHAR(&ld->board[BOX_VPD_BYTE0], 0xAA);
		    WR_CHAR(&ld->board[BOX_VPD_BYTEL], 0xAA);
		    EndPio();
		    sGotHilink(adap, port&0x30);
		}
#endif /* VPD_BOX_RESET */

		if (ld->prms->has_died2) 	/* RESET EVERYTHING */
		{
		    int	t_port, ii;

		    SL_printf("reset|");
		    for (t_port = port&0x30, ii=0 ; ii<16 ; ii++, t_port++) 
		    {
			ld = adap->main + t_port;
			ld->prms->has_died2 = 0;
			slion_set_gcomm(ld, GC_BOX_RST_1|GC_BOX_RST_2);
		    }
		}
		tp = adap->m_ttyp[port];

		/* If close in process or not opened yet, bypass this port
		 * handling :this check will be done by myttyinput */
		/* if(!(tp->t_isopen) && !(tp->t_wopen))
		 *	 continue;	***/

#ifdef FEATURES
		tq = adap->v_ttyp[port];
#endif /*FEATURES */

		StartPio(slion_pio, ld, tmp_dss = 0; break);
		tmp_dss = RD_CHAR(&ld->ip->cur_dcd);
		EndPio();
	    
		if((tmp_dss ? 1 : 0) != ld->prms->dcd && !ld->prms->dcdp) 
		{
		    tmp = (ld->prms->dcd ^= 1) ? cd_on : cd_off;
		    if (tp && (err = myttyinput(tp, tp->t_ctl, tmp)))
			slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
		}
		StartPio(slion_pio, ld, tmp_dss = 0; break);
		tmp_dss = RD_CHAR(&ld->ip->cur_cts);
		EndPio();
		if((tmp_dss ? 1 : 0) != ld->prms->cts && !ld->prms->ctsp) 
		{
		    tmp = (ld->prms->cts ^= 1) ? cts_on : cts_off;
		    if (tp && (err = myttyinput(tp, tp->t_ctl, tmp)))
			slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
		}
	    }	/* if vector ..*/
	} 	/* for port_last...*/
    }		/* for box_vector..*/

    if (accum_vec & (CC_INT_RCMAIN|CC_INT_RCALT))
	sysinfo.rcvint++;
    if (accum_vec & (CC_INT_TXMAIN|CC_INT_TXALT|CC_INT_TXPAR))
	sysinfo.xmtint++;
    if (accum_vec & (CC_INT_DCD|CC_INT_CTS))
	sysinfo.mdmint++;

    old_intr = i_disable(INTCLASS0);	/* WITH SLIH !!! */
    StartPio(slion_pio, 0, adap->posted = 1; break);
    WR_CHAR(&adap->board->int_mask, 0);
    adap->posted = 0;
    EndPio();
    i_enable(old_intr);

    DET_BD();

    SL_printf("ES_ofl|");

    Return(0);

} /* slion_offlev */

/*
 * FUNCTION:	sGotHilink
 *
 * PURPOSE:	Reset DCD/CTS of board.
 *
 * INPUT:
 *      intr:   Pointer to sli_adap structure.
 *      port:   Adapter port entry.
 *
 * RETURN:
 *	None.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_err	myttyinput
 *
 * DESCRIPTION:
 *      This routine completes interrupt handling on status change,
 *	and  enqueus an M_CTL message via myttyinput routine.
 *
 * CALLED BY:
 *	slion_offlev
 */
static void 
sGotHilink(struct sli_adap *adap, int port)
{
    int			ii, err, t_port;
    str_lionp_t		tp;
    struct sliondata	*ld;

    char		errtxt[64];

    SL_printf("\nsGH|");

    for (t_port=port, ii=0 ; ii<16 ; ii++, t_port++) 
    {

#ifdef SLI_TRACE
    if(DBFLG & LVL1)
	SL_printf("NOTopend(%x)",t_port);
#endif

	adap->prms[t_port].has_died = 1;
	adap->prms[t_port].has_died2 = 1;
	adap->prms[t_port].gcomm_mask = 0;
	if (tp = adap->m_ttyp[t_port]) 
	{
	  if(tp->t_isopen)
	  {
	    if (err = myttyinput(tp, tp->t_ctl, cd_off))
		slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
	    if (err = myttyinput(tp, tp->t_ctl, cts_off))
		slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
	  }
	}

#ifdef FEATURES	
	if (tp = adap->v_ttyp[t_port]) 
	{
	  if(tp->t_isopen)
	  {
	    if (err = myttyinput(tp, tp->t_ctl, cd_off))
		slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
	    if (err = myttyinput(tp, tp->t_ctl, cts_off))
		slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
	  }
	}
	if (tp = adap->x_ttyp[t_port]) 
	{
	  if(tp->t_isopen)
	  {
	    if (err = myttyinput(tp, tp->t_ctl, cd_off))
		slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
	    if (err = myttyinput(tp, tp->t_ctl, cts_off))
		slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
	  }
	}
#endif /*FEATURES */

	ld = adap->main+t_port;
	ld->prms->dcd = ld->prms->cts = 0;

	ld->flags &= ~(COMM_FLAG|FLSH_FLAG);
	ld->comm_head = ld->comm_tail = 0;

#ifdef FEATURES
	ld = adap->vger+t_port;
	ld->flags &= ~(COMM_FLAG|FLSH_FLAG);
	ld->comm_head = ld->comm_tail = 0;
	ld = adap->xpar+t_port;
	ld->flags &= ~(COMM_FLAG|FLSH_FLAG);
	ld->comm_head = ld->comm_tail = 0;
#endif /*FEATURES */

#ifdef SLI_TRACE
    if(DBFLG & LVL1)
	SL_printf("set ld|");
#endif

    }

    (void)sprintf(errtxt,"%s conc %d",adap->adap_name,(port/16)+1);
    slion_err(errtxt, ERRID_LION_BOX_DIED, 0);

#ifdef SLI_TRACE
    if(DBFLG & LVL1)
	SL_printf("EsGHl|");
#endif

} /* sGotHilink */

/*
 * FUNCTION:	sbox_reset
 *
 * PURPOSE:	Reset all parameters for a box.
 *
 * INPUT:
 *      ld:     Pointer to sliondata structure.
 *
 * RETURN:
 *	None.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_set_gcomm   slion_comm
 *
 * DESCRIPTION:
 *      This routine resets all of the paramters for a box after it has 
 *	been reset. Like if it gets unplugged and replugged.  When the 
 *	adapter loses communications with the box, a HILINK interrupt 
 *	is issued.  When the box comes back on-line, there is a 
 *	modem status interrupt for each port on	the box. This routine 
 *	is also called at config time to setup initial parameters.
 *
 * CALLED BY:
 *	slionadd	sgcomm_tmr_f
 */
void 
sbox_reset(struct sliondata *ld)
{
    int		jj;

#ifdef SLI_TRACE
    if(DBFLG & LVL2)
	SL_printf("Bx_rst|");
#endif

    ld->prms->has_died = 0;

    slion_set_gcomm(ld, GC_DEF_ERROR|GC_SET_PARMS);

    if ( (jj=sscan_baud_table(ld->prms->baud)) == -1) 
    {
	slion_comm(ld, BAUD|sscan_baud_table(19200));
	ld->prms->baud = 19200;
    } 
    else
	slion_comm(ld, BAUD|jj);

    if (ld->prms->dtr)
	slion_comm(ld, ADTR);
    if (ld->prms->rts)
	slion_comm(ld, ARTS);

    if (ld->prms->dtrp)
	slion_comm(ld, DTR1);
    if (ld->prms->dcdp)
	slion_comm(ld, DCD1);
    if (ld->prms->rtsp)
	slion_comm(ld, RTS1);
    if (ld->prms->ctsp)
	slion_comm(ld, CTS1);
    if (ld->prms->xonp)
	slion_comm(ld, XON1);
    if (ld->prms->xoff)
	slion_comm(ld, XOF1);
    if (ld->prms->xany)
	slion_comm(ld, XANY);

    if (ld->prms->vger_defd) 
    {
	slion_comm(ld, ALT1);
	slion_set_gcomm(ld, GC_VGER_DEFS);
    }
    if (ld->prms->xpar_defd)
	slion_set_gcomm(ld, GC_GOTO_XPAR|GC_LEAV_XPAR);

} /* sbox_reset */

/*
 * FUNCTION:	slion_send
 *
 * PURPOSE:	Write data on the output ring buffer.
 *
 * INPUT:
 *      ld:	Pointer to sliondata structure.
 *      tp:	Pointer to str_lion structure.
 *      BusMem:	Pointer to BusMem.
 *
 * RETURN:
 *	0:	On success.
 *	1:	Already busy or no enough room on board.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_pio.
 *
 * DESCRIPTION:
 *	Copy the data from a STREAMS message to the ring buffer of the 
 *	port referred to by ld.
 *	If the message was successfully transmitted then frees it.
 *	Other messages can be transmitted.
 *	If the message was partially transmitted and the ring buffer got
 *	full then sets the t_busy flag of the str_tty structure.
 *
 * CALLED BY:
 *	slionproc(T_OUTPUT, T_RESUME or T_WFLUSH)
 */
static
slion_send(struct sliondata *ld, str_lionp_t tp, uchar *BusMem)
{
    DoingPio;
    struct	ring 	*tr_buf = ld->tr_buf;
    ushort 		t_len;
    volatile	uchar 	*ram_ptr, *str, *ptr_end;
    ushort		bh, bt;
    int 		rc = 0;

    /* reset interrupt expected flag */

#ifdef SLI_TRACE
    if(DBFLG & LVL5)
    	SL_printf("\nSsnd.%x)",PRT(tp->t_dev));
#endif

    if (ld->prms->has_died) 
    {
	ld->txcnt = 0;
	SL_printf("tx dead|");
	return 0;
    }

    /* already busy */
    if (ld->flags & DIAG_FLAG)	    /* after return, msg is free?? or no??*/
	return 1;

    StartPio(slion_pio, ld, return EIO);
    bh = RD_SHORT(&tr_buf->buf_head);
    bt = RD_SHORT(&tr_buf->buf_tail);
    EndPio();

#ifdef COMM_DEBUG
    ld->pad_entry[ld->pad_curr++] = 0xf200 | ld->flags;   ld->pad_curr %= 256;
    ld->pad_entry[ld->pad_curr++] = bh;    ld->pad_curr %= 256;
    ld->pad_entry[ld->pad_curr++] = bt;    ld->pad_curr %= 256;
#endif

    /* while data to send and board chunk available */

    while (ld->txcnt) 
    {
	/* check for room, if no enough room on board wait for interrupt */
	if ( ((bh + ld->prms->tr_width) == bt) ||
	    ((bh == ld->tr_ba) && (bt == ld->tr_bb)) ) 
	{
	    StartPio(slion_pio, ld, return EIO);
	    WR_SHORT(&tr_buf->buf_head, bh);
	    EndPio();
	    ld->flags |= XMIT_FLAG;

#ifdef SLI_TRACE
    if(DBFLG & LVL5)
	    SL_printf("nospace|");
#endif

	    rc = 1; 
	    break;

	}

	/* send a chunk of data at most (ld->prms->tr_tbc) long */
	t_len = MIN(ld->txcnt, ld->prms->tr_tbc);

#ifdef SLI_TRACE
    if(DBFLG & LVL5)
	SL_printf("ll=%x|",t_len);
#endif

	ram_ptr = MK_ADDR(ld->board[bh]);
	StartPio(slion_pio, ld, return EIO);
	BUS_PUTSR(ram_ptr, t_len);
	EndPio();

	ptr_end = (ram_ptr += 2) + t_len;

	str = ld->txbuf;
	StartPio(slion_pio, ld, return EIO);
	while (ram_ptr < ptr_end) 	/* CHANGE INTO SHORTS ???  */
	{
	    BUS_PUTC(ram_ptr, *str);	/* if it's hot outside */
	    ram_ptr++;
	    str++;
	}
	EndPio();

	bh = (bh == ld->tr_ba) ? ld->tr_bb : (bh + ld->prms->tr_width);

	ld->o_count += t_len;
	ld->txbuf += t_len;
	ld->txcnt -= t_len;

        tp->t_busy = 1;         /* I put something on the adapter */

	StartPio(slion_pio, ld, return EIO);
	bt = RD_SHORT(&tr_buf->buf_tail); /* Could have moved!! */
	EndPio();
    }

    if (tp->t_busy) 
    {
	StartPio(slion_pio, ld, return EIO);
	WR_SHORT(&tr_buf->buf_head, bh);
	EndPio();
    }
#ifdef SLI_TRACE
    if(DBFLG & LVL5)
    	SL_printf("ESsnd|");
#endif

    return rc;

} /* slion_send */

/*
 * FUNCTION:	slion_comm
 *
 * PURPOSE:	Set status flag.
 *
 * INPUT:
 *      ld:     Pointer to sliondata structure.
 *      comm:   In_line command.
 *
 * RETURN:
 *	0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_pio
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_enable	i_disable
 *
 * DESCRIPTION:
 *	Set in_line commands.
 *
 * CALLED BY:
 *	slionopen	slionservice	slion_termiox_set  sgcomm_tmr_f
 *	sbox_reset
 */
int 
slion_comm(struct sliondata *ld, int comm)
{
    DoingPio;
    register ushort	b_h, b_t;
    struct ring		*tr_buf = ld->tr_buf;
    int			old_intr;
    uchar		*BusMem;


#ifdef SLI_TRACE
    if(DBFLG & LVL3)
	SL_printf("S_com.%x)",PRT(ld->tp->t_dev));
#endif

    if (ld->prms->has_died)
	return;

    old_intr = i_disable(INT_TTY);
    ATT_BD(ld);

    StartPio(slion_pio, ld, DET_BD(); i_enable(old_intr); return EIO);
    b_h = RD_SHORT(&tr_buf->buf_head);
    b_t = RD_SHORT(&tr_buf->buf_tail);
    EndPio();

#ifdef COMM_DEBUG
    ld->pad_entry[ld->pad_curr++] = 0xf000 | ld->flags;   ld->pad_curr %= 256;
    ld->pad_entry[ld->pad_curr++] = (ld->comm_head << 8)| ld->comm_tail;
    ld->pad_curr %= 256;
    ld->pad_entry[ld->pad_curr++] = comm;   ld->pad_curr %= 256;
    ld->pad_entry[ld->pad_curr++] = b_h;    ld->pad_curr %= 256;
    ld->pad_entry[ld->pad_curr++] = b_t;    ld->pad_curr %= 256;
#endif
    if ((ld->flags&COMM_FLAG) ||
		((b_h+ld->prms->tr_width) == b_t) ||
		((b_h==ld->tr_ba) && (b_t==ld->tr_bb)) ) 
    {							    /* buffer full */
	uchar	ii;

	for(ii=ld->comm_tail; ii != ld->comm_head; ii = (ii+1) & (COMM_SIZE-1))
	    if ( (ld->comm_list[ii] & 0x3f00) == (comm & 0x3f00) )
		break;

	if (ii == ld->comm_head) 
	{
	    ld->comm_list[ld->comm_head] = comm;
	    ld->comm_head = (ld->comm_head+1) & (COMM_SIZE-1);
	} else
	    ld->comm_list[ii] = comm;

	ld->flags |= COMM_FLAG;

	DET_BD();
	i_enable(old_intr);
	return (0);
    }

    StartPio(slion_pio, ld, DET_BD(); i_enable(old_intr); return EIO);
    WR_SHORT(ld->board+b_h, comm);
    EndPio();

    b_h = (b_h == ld->tr_ba) ? ld->tr_bb : (b_h + ld->prms->tr_width);
    ld->tp->t_busy = 1;         /* Input something on the adapter */

    StartPio(slion_pio, ld, DET_BD(); i_enable(old_intr); return EIO);
    WR_SHORT(&tr_buf->buf_head, b_h);
    EndPio();

    DET_BD();
    i_enable(old_intr);
    return(0);

} /* slion_comm */

/*
 * FUNCTION:	slion_do_comms
 *
 * PURPOSE:	Write data to the output ring buffer.
 *
 * INPUT:
 *      ld:	Pointer to sliondata structure.
 *
 * RETURN:
 *	0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slionproc	slion_pio
 *
 * EXTERNAL PROCEDURES CALLED:
 *	qenable		freemsg
 *
 * DESCRIPTION:
 *	Get ring buffer head/tail and set them into the sliondata structure
 *	associated with the port. 
 *	If the port is opened(t_isopen) or close processing is active
 *	(t_wclose), slionproc(T_OUTPUT) s called to put onto the ring buffer. 
 *	If completed, data message is released and set the queue for 
 *	scheduling.
 *
 * CALLED BY:
 *	sl_dram_wr	slion_offlev	sgcomm_tmr_f
 */
void
slion_do_comms(struct sliondata *ld)
{
    DoingPio;
    register ushort	b_h, b_t;
    struct ring		*tr_buf = ld->tr_buf;
    uchar		*BusMem;
    mblk_t		*mp;	/* To contain currently transmitted message */
    int			old_intr;

#ifdef SLI_TRACE
    if(DBFLG & LVL5)
	SL_printf("S_do_com|");
#endif

#ifdef COMM_DEBUG
    ld->pad_entry[ld->pad_curr++] = 0xf100 | ld->flags;   ld->pad_curr %= 256;
    ld->pad_entry[ld->pad_curr++] = (ld->comm_head << 8)| ld->comm_tail; 
    ld->pad_curr %= 256;
#endif

    if (ld->flags & DIAG_FLAG)
	return;

    if (ld->prms->has_died) 
    {
	ld->flags &= ~COMM_FLAG;
	ld->comm_head = ld->comm_tail = 0;
    } 
    else if (ld->flags & COMM_FLAG) 
    {
	ATT_BD(ld);
	old_intr = i_disable(INT_TTY);
	StartPio(slion_pio, ld, i_enable(old_intr); DET_BD(); return);
	b_h = RD_SHORT(&tr_buf->buf_head);
	EndPio();
	i_enable(old_intr);

	while (ld->comm_tail != ld->comm_head) 
	{

	    old_intr = i_disable(INT_TTY);
	    StartPio(slion_pio, ld, i_enable(old_intr); DET_BD(); return);
	    b_t = RD_SHORT(&tr_buf->buf_tail);
	    EndPio();
	    i_enable(old_intr);

#ifdef COMM_DEBUG
    ld->pad_entry[ld->pad_curr++] = b_h;    ld->pad_curr %= 256;
    ld->pad_entry[ld->pad_curr++] = b_t;    ld->pad_curr %= 256;
#endif

	    if ( ((b_h+ld->prms->tr_width) == b_t) ||
			    ( (b_h==ld->tr_ba) && (b_t==ld->tr_bb) ) ) 
	    {
		DET_BD();
		return;
	    }

#ifdef COMM_DEBUG
    ld->pad_entry[ld->pad_curr++] = ld->comm_list[ld->comm_tail];
    ld->pad_curr %= 256;
#endif
	    old_intr = i_disable(INT_TTY);
	    StartPio(slion_pio, ld, i_enable(old_intr); i_enable(old_intr); 
							DET_BD(); return);
	    WR_SHORT(ld->board+b_h, ld->comm_list[ld->comm_tail]);
	    EndPio();

	    ld->comm_tail = (ld->comm_tail+1) & (COMM_SIZE-1);
	    ld->tp->t_busy = 1;         /* Input something on the adapter */

	    b_h = (b_h == ld->tr_ba) ? ld->tr_bb : (b_h + ld->prms->tr_width);
	    StartPio(slion_pio, ld, i_enable(old_intr); DET_BD(); return);

	    WR_SHORT(&tr_buf->buf_head, b_h);
	    EndPio();
	    i_enable(old_intr);
	}
	ld->flags &= ~COMM_FLAG;

	DET_BD();
    }

    if ((adap_list[ADP(ld->tp->t_dev)]->m_ttyp[PRT(ld->tp->t_dev)])) 
    {

	if (ld->flags & XMIT_FLAG)
	    ld->flags &= ~XMIT_FLAG;

	/* Carry on transmission of the current message, if any,
         * free it if completed, and set the queue for scheduling.
	 */


#ifdef SLI_TRACE
    if(DBFLG & LVL5)
	SL_printf("Txcnt=%x|",ld->txcnt);
#endif
	if (ld->tp->t_isopen) 
	{
		if (ld->txcnt) 
		{
			old_intr = i_disable(INT_TTY);
			slionproc(ld->tp, T_OUTPUT);
			i_enable(old_intr);
		}
		else 
		{
			if (mp = ld->tp->t_outmsg) 
			{
				freemsg(mp);
				ld->tp->t_outmsg = (mblk_t *) 0;
			}
			/* Enable Q if there's a msg queued */
			if((WR(ld->tp->t_ctl))->q_first)
				 qenable(WR(ld->tp->t_ctl));

			/* Wakeup the sleeping process at close time, 
			 * for draining. 
			 */
			if(ld->tp->t_draining && !(WR(ld->tp->t_ctl))->q_first) 
			{
				old_intr = i_disable(INT_TTY);
				ld->tp->t_draining = 0;
				TTY_WAKE(ld->tp);
				i_enable(old_intr);
			}
		}
	}
    }

} /* slion_do_comms */


/* called by slion_recv */
#define CHECK_ERRORS	{ \
    if (cc & CC_BREAK) { \
	if (err = strttyinput(q, mp, *c_ptr++, break_interrupt,tp)) \
	    slion_err(tp->t_name, ERRID_TTY_BADINPUT, err); \
    } else if (cc & CC_FRAMING) { \
	if (err = strttyinput(q, mp, *c_ptr++, framing_error,tp)) \
	    slion_err(tp->t_name, ERRID_TTY_BADINPUT, err); \
    } else if (cc & CC_PARITY) { \
	if (err = strttyinput(q, mp, *c_ptr++, parity_error,tp)) \
	    slion_err(tp->t_name, ERRID_TTY_BADINPUT, err); \
    } else if (cc & CC_OVERRUN) { \
	if (err = strttyinput(q, mp, *c_ptr++, overrun,tp)) \
	    slion_err(tp->t_name, ERRID_TTY_BADINPUT, err); \
    } else if (cc & CC_BUFFERO) { \
	slion_err(tp->t_name, ERRID_LION_BUFFERO, 0); \
    } else { \
	slion_err(tp->t_name, ERRID_LION_UNKCHUNK, cc); \
    } \
}

/*
 * FUNCTION:	slion_recv		see sfix.s
 */

/*
 * FUNCTION:	_slion_recv
 *
 * PURPOSE:	Receive data handling.
 *
 * INPUT:
 *      ld:     Pointer to sliondata structure.
 *      tp:     Pointer to str_lion structure.
 *
 * RETURN:
 *	0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *	slion_err	strttyinput	slion_pio	slion_allocmsg
 *
 * EXTERNAL PROCEDURES CALLED:
 *	putq		putnext
 *
 * DESCRIPTION:
 *	Busmem value is catched from tp:tp->t_busmem which is loaded by
 *	slion_recv.
 *	When the flag "t_isopen" or "t_cread" is off, the received data is 
 *	ignored.
 *	Receives the incomming data from hardware (Ring Buffer), checks
 *	it for errors and transforms it into STREAMS message(s) enqued.
 *	Correctly received data are put in M_DATA messages, control X
 *	will issue un M_SIG message and errors will be notified in M_BREAK
 *	messages.
 *	These mesages will be sent upstreams by the read service routine,
 *	when allowed by flow control conditions.
 *	If Q is empty and Q is less than HWM, the data is sent directly
 *	to upper stream.
 *
 * CALLED BY:
 *	slion_offlev(via slion_recv[sfix.s])
 */
int 
_slion_recv(struct sliondata *ld, str_lionp_t tp)
{

    DoingPio;
    ushort		rh, rt, numc;
    struct ring		*rc_buf = ld->rc_buf;
    uchar		cc;
    volatile uchar	*c_ptr, *e_ptr, buf[SLRDBUFSZ];
    volatile int	ii;
    register int	ctlx;
    int			err =0;
    mblk_t		*mp;
    queue_t		*q;
    uchar		*BusMem = tp->t_busmem; 

    if (!(q = tp->t_ctl))
	return(0);

    if(!(tp->t_cread) || !(tp->t_isopen)) 
    {
	SL_printf("IG_RXdt|");
	/* Reset input ring buffer pointer */
	StartPio(slion_pio, ld, return);
	WR_SHORT(&rc_buf->buf_tail, RD_SHORT(&rc_buf->buf_head));
	EndPio();
	return(0);
    }


   /* 
    * This allocation will be done somewhere after input data processing,
    * take the OSF strategy.
    * Allocation of data block  is done at the open time.
    */
    if(!(mp = tp->t_inmsg))
    {
        if(!(mp = slion_allocmsg(tp)))
		return(0);
    }

    tp->t_rxcnt = 0;
    StartPio(slion_pio, ld, return);
    rh = RD_SHORT(&rc_buf->buf_head);
    rt = RD_SHORT(&rc_buf->buf_tail);
    EndPio();

    if (rh != rt) 
    {						/* Buffer is not empty*/
	do 
	{
	    if(ld->block)
	    {
		SL_printf("T_BKon2|");
		return(0);
	    }

	    c_ptr = MK_ADDR(ld->board[rt]) + 2;	  /* Pointer to data area */

	    StartPio(slion_pio, ld, return);
	    numc = BUS_GETSR(c_ptr - 2);	  /* Get data length */
	    EndPio();

#ifdef SLI_TRACE
    if(DBFLG & LVL4)
	SL_printf("Gch(%x)",numc);
#endif

	    /* if we get a bad value for numc, we can crash the machine! */
	    if (numc <= 0 || numc > 32) 
	    {
		SL_printf("bd Rxcnt=%x|",numc);

		slion_err(tp->t_name, ERRID_LION_CHUNKNUMC, numc);
		numc = 1;	
		break;		/* just skip to the end of while */

	    }
	    e_ptr = c_ptr + numc;	/* Point to end of data string */

	    StartPio(slion_pio, ld, return);
	    for (ii = 0; c_ptr+ii < e_ptr ; ii++)
		buf[ii] = BUS_GETC(c_ptr + ii);
	    EndPio();

	    c_ptr = buf;
	    e_ptr = c_ptr + numc;

#ifdef SLI_TRACE
    if(DBFLG & LVL4)
	SL_printf("rcstat=%x|",ld->rc_state);
#endif

	    /* Check char of the previous chunk*/
	    if (cc = ld->rc_state) 		/* track of error chk stuff */
	    {
		ld->rc_state = 0;
		if (cc == 0xff) 
		{
		    if ( (cc = *c_ptr++) == 0xff) 
		    {
			if(err = strttyinput(q, mp, 0xff, good_char, tp))
			    slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
			ld->i_count++;
			if(!(mp = tp->t_inmsg)) return(0); 
		    } 
		    else if (c_ptr == e_ptr)
			ld->rc_state = cc;
		    else 
		    {
			CHECK_ERRORS;
			if(!(mp = tp->t_inmsg)) return(0); 
		    }
		} 
		else 
		{
		    CHECK_ERRORS;
		    if(!(mp = tp->t_inmsg)) return(0); 
		}
	    }

	    ctlx = tp->t_ctlx;		/* Cache up the control X flag */

	    /* Process rx data(max 32 bytes)*/
	    while (c_ptr < e_ptr) 
	    {
		if ( (cc = *c_ptr++) != 0xff) 
		{
#ifdef SLI_TRACE
  if(DBFLG & TRCHA)
	SL_printf("[%x]",cc);
#endif

		    if (tp->t_sak) 
		    {

#ifdef SLI_TRACE
    if(DBFLG & LVL4)
	SL_printf("sak|");
#endif

			/***SAK sequence**/
			if (ctlx && cc == ('r'&0x1f)) 		/*^R check */
			{

#ifdef SLI_TRACE
    if(DBFLG & LVL4)
	SL_printf("^R|");
#endif
				ld->i_count++;
				tp->t_rxcnt++;
				mp->b_datap->db_type = M_PCSIG;
				*(unsigned char *)mp->b_rptr = SIGSAK;
				mp->b_wptr=mp->b_rptr+sizeof(unsigned char);
				putnext(q,mp);
				tp->t_inmsg = 0;
				tp->t_rxcnt = 0;
				if(!(mp = slion_allocmsg(tp)))
				{
					err = -1;
					break;
				}
				ctlx = 0;
				continue;
			}

			/* Feed up old control X if any */
			if (ctlx)				/* ^X*/
			{

#ifdef SLI_TRACE
    if(DBFLG & LVL4)
	SL_printf("^X|");
#endif
			    if(err=strttyinput(q,mp,('x'&0x1f),good_char,tp))
			    {
			        ld->i_count++;
				slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
				break;
			    }
			    if(!(mp = tp->t_inmsg)) break; 
			    ld->i_count++;
			    
			}
			if (ctlx = (cc == ('x'&0x1f)))
			    continue;
			if (err = strttyinput(q, mp, cc, good_char, tp))
			{
			    ld->i_count++;
			    slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
			    break;
			}
			if(!(mp = tp->t_inmsg)) break; 
			ld->i_count++;

		    } /* t_sack */ 
		    else 
		    {
			if (err = strttyinput(q, mp, cc, good_char, tp))
			{
		    	    ld->i_count++;
			    slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
			    break;
			}
			if(!(mp = tp->t_inmsg)) break; 
			ld->i_count++;
		    }
		} 
		else 
		{
		     if (c_ptr == e_ptr)		/* cc==0xff */
		   	 ld->rc_state = 0xff;
		     else if ( (cc = *c_ptr++) == 0xff) 
		     {
			if (err = strttyinput(q, mp, 0xff, good_char, tp))
			{
		    	    ld->i_count++;
			    slion_err(tp->t_name, ERRID_TTY_BADINPUT, err);
			    break;
			}
			if(!(mp = tp->t_inmsg)) break; 
		    	ld->i_count++;
		     } 
		     else if (c_ptr == e_ptr)
		    	ld->rc_state = cc;
		     else 
		     {
		    	CHECK_ERRORS;	
			if(!(mp = tp->t_inmsg)) break; 
		     }
		}
	    } /* while (c_ptr<e_ptr)*/

	    tp->t_ctlx = ctlx;		/* if ^X, ^X. if ^R,0 */

	    if (rt == ld->rc_ba)
		rt = ld->rc_bb;
	    else
		rt += ld->prms->rc_width;

	    StartPio(slion_pio, ld, return);
	    WR_SHORT(&rc_buf->buf_tail, rt); /* write new value */
	    rh = RD_SHORT(&rc_buf->buf_head); /* could have moved */
	    EndPio();

	    /* Error case:no msg blk:
	     * -1=no data blk -2= no msg for M_BREAK */
	    if(err == -1)
		return(0);

	} while ( rt != rh );

	if(tp->t_rxcnt)
	{
		/* Queue up the elaborated message. 
		 * Works for good_chars only. 
		 * Send chars directly up if nothing
		 * else is queued in read queue
		 */
		if(!(q->q_first) && canput(q->q_next) && (q->q_next)->q_ptr)
			putnext(q, mp);
		else
            		putq(q,mp); 

		tp->t_inmsg = 0;
	}
    	/* Allocate an M_DATA to contain characters received later */
	slion_allocmsg(tp);

    } /* rh != rt */

#ifdef SLI_TRACE
    if(DBFLG & LVL4)
	SL_printf("ES_rcv(%x)",tp->t_rxcnt);
#endif

    return(0);

} /* _slion_recv */

/*
 * FUNCTION:	slion_wake_tmr
 *
 * PURPOSE:	Generic timer handling.
 *
 * INPUT:
 *      ld:     Pointer to sliondata structure.
 *
 * RETURN:
 *	None.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	qreply
 *
 * DESCRIPTION:
 *	Continue processing after a fixed amount of time has passed. 
 *	The routine is called by timer and then the suspended message
 *	(M_IOCACK),which is prepared by slioc_slpx, is put in the read queue. 
 *
 * CALLED BY:
 *	Via timeout() in slioc_slpx.
 */
void 
slion_wake_tmr(struct sliondata *ld)
{
	str_lionp_t	 tp = (str_lionp_t)ld->tp;
	queue_t		 *q = WR(tp->t_ctl);
	mblk_t 		 *mp = tp->t_ctloutmsg;

 	/* To put M_IOCACK message*/
	qreply(q, mp);
	tp->t_ctloutmsg = (mblk_t *)0;

} /* slion_wake_tmr */

/*
 * FUNCTION:	slion_set_gcomm
 *
 * PURPOSE:	Set global command mask.
 *
 * INPUT:
 *      ld:	Pointer to sliondata structure.
 *	mask:	Request mask.
 *
 * RETURN:
 *      None.
 *
 * INTERNAL PROCEDURES CALLED:
 *      sgcomm_tmr_f.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	i_disable	i_enable 
 *
 * DESCRIPTION:
 *	The mask is set to perform the global command by sgcomm_tmr_f.
 *
 * CALLED BY:
 *	slionopen	slion_ioctl	slionproc	slion_offlev
 *	sbox_reset
 */
void 
slion_set_gcomm(struct sliondata *ld, int mask)
{
    int		old_intr;

#ifdef SLI_TRACE
    if(DBFLG & LVL3)
	SL_printf("S_s_gcm|");
#endif

    old_intr = i_disable(INT_TTY);

    ld->prms->gcomm_mask |= mask;
    sgcomm_tmr_f(ld);
    i_enable(old_intr);

} /* slion_set_gcomm */

/*
 * FUNCTION:	Sgcomm_tmr_f
 *
 * PURPOSE:	Set global command.
 *
 * INPUT:
 *      ld:     Pointer to sliondata structure.
 *
 * RETURN:
 *      0:	Always.
 *
 * INTERNAL PROCEDURES CALLED:
 *      slion_comm 	slion_do_comms		slion_pio	sbox_reset
 *
 * EXTERNAL PROCEDURES CALLED:
 *	qreply		timeout
 *
 * DESCRIPTION:
 *	Set global command.
 *	Then timer is set to wait as the commands  are completely executed.
 *	When time has elapsed, the timer function routine(sttyofflev)
 *	calls again this routine to 
 *	- put a M_IOCACK message into the read queue, which is set by
 *	  slion_ioctl with LI_SETXP/VT.
 *	otherwise
 *	- do no action.
 *
 * CALLED BY:
 *	Via timeout	slion_set_gcomm
 */
void 
sgcomm_tmr_f(struct sliondata *ld)
{
    DoingPio;
    volatile int	ii, jj;
    volatile uchar	*i_ptr, *ga_ptr, *BusMem;

    /* Put M_IOCACK into the queue if mask is reset: instead of e_wakeup*/
    str_lionp_t		tp = (str_lionp_t)ld->tp;
    queue_t		*q = tp->t_ctl;
    mblk_t 		*mp = tp->t_ctloutmsg;
    struct iocblk	*iocp = (struct iocblk *)mp->b_rptr;


#ifdef SLI_TRACE
    if(DBFLG & LVL3)
	SL_printf("sgc_tmr|");
#endif

    ATT_BD(ld);

    StartPio(slion_pio, ld, DET_BD(); return);
    ga_ptr = MK_ADDR(ld->board[RD_SHORT(&ld->ip->glob_add)]);
    ii = BUS_GETC(ga_ptr);
    EndPio();

    tp->t_gcommtmr = 0;
    if (!ii || ld->prms->has_died) 
    {
	for (ii=1 ; ii<GC_MAX ; ii<<=1)
	    if (ld->prms->gcomm_mask & ii)
		break;

#ifdef SLI_TRACE
    if(DBFLG & LVL3)
	SL_printf("ii=%x|",ii);
#endif
	switch (ii) 
	{
	   case GC_FLSH_MAIN:
	    SL_printf("FLS_MAIN|");
	    StartPio(slion_pio, ld, DET_BD(); return);
	    BUS_PUTC((ga_ptr + 1), 0);
	    BUS_PUTC((ga_ptr + 0), G_DDS_FLSH);
	    EndPio();
	    break;

	   case GC_FLSH_VGER:
	    StartPio(slion_pio, ld, DET_BD(); return);
	    BUS_PUTC((ga_ptr + 1), 1);
	    BUS_PUTC((ga_ptr + 0), G_DDS_FLSH);
	    EndPio();
	    break;

	   case GC_FLSH_XPAR:
	    StartPio(slion_pio, ld, DET_BD(); return);
	    BUS_PUTC((ga_ptr + 1), 2);
	    BUS_PUTC((ga_ptr + 0), G_DDS_FLSH);
	    EndPio();
	    break;

	   case GC_E_FL_MAIN:
	    SL_printf("END_FLS|");
	    slion_comm(ld, E_FL);
	    ld->flags &= ~FLSH_FLAG;
	    slion_do_comms(ld);
	    break;

	   case GC_E_FL_VGER:
	    ld = (struct sliondata *)ld->prms->vtp->t_hptr;
	    slion_comm(ld, E_FL|1);
	    ld->flags &= ~FLSH_FLAG;
	    slion_do_comms(ld);
	    break;

	   case GC_E_FL_XPAR:
	    ld = (struct sliondata *)ld->prms->xtp->t_hptr;
	    slion_comm(ld, E_FL|2);
	    ld->flags &= ~FLSH_FLAG;
	    slion_do_comms(ld);
	    break;

	   case GC_ZAP_INHIB:
	    SL_printf("Z_INHB|");
	    StartPio(slion_pio, ld, DET_BD(); return);
	    BUS_PUTC((ga_ptr + 1), 2);
	    BUS_PUTC((ga_ptr + 0), G_XINHIBIT);
	    EndPio();
	    break;

	   case GC_SET_INHIB:
	    SL_printf("S_INHB|");
	    StartPio(slion_pio, ld, DET_BD(); return);
	    BUS_PUTC((ga_ptr + 1), 0);
	    BUS_PUTC((ga_ptr + 0), G_XINHIBIT);
	    EndPio();
	    break;

#ifdef FEATURES
	   case GC_VGER_DEFS:
	    StartPio(slion_pio, ld, DET_BD(); return);
	    jj = 1;
	    for (i_ptr = ld->prms->goto1 ; jj<5 ; )
		BUS_PUTC((ga_ptr + jj++), *i_ptr++);
	    for (i_ptr = ld->prms->goto2 ; jj<9 ; )
		BUS_PUTC((ga_ptr + jj++), *i_ptr++);
	    for (i_ptr = ld->prms->screen1 ; jj<20 ; )
		BUS_PUTC((ga_ptr + jj++), *i_ptr++);
	    for (i_ptr = ld->prms->screen2 ; jj<31 ; )
		BUS_PUTC((ga_ptr + jj++), *i_ptr++);
	    BUS_PUTC((ga_ptr + 0), G_ALT_DEFS);
	    EndPio();
	    break;

	   case GC_GOTO_XPAR:
	    StartPio(slion_pio, ld, DET_BD(); return);
	    jj = 1;
	    for (i_ptr = ld->prms->in_xpar ; jj<12 ; )
		BUS_PUTC((ga_ptr + jj++), *i_ptr++);
	    BUS_PUTC((ga_ptr + 0), G_IN_XPAR);
	    EndPio();
	    break;

	   case GC_LEAV_XPAR:
	    StartPio(slion_pio, ld, DET_BD(); return);
	    jj = 1;
	    for (i_ptr = ld->prms->lv_xpar ; jj<12 ; )
		BUS_PUTC((ga_ptr + jj++), *i_ptr++);
	    BUS_PUTC((ga_ptr + 0), G_LV_XPAR);
	    EndPio();
	    break;

	   case GC_FRCE_MAIN:
	    StartPio(slion_pio, ld, DET_BD(); return);
	    BUS_PUTC((ga_ptr + 0), G_GOMAIN);
	    EndPio();
	    break;
#endif /*FEATURES */

	   case GC_SET_PARMS:
	    StartPio(slion_pio, ld, DET_BD(); return);
	    for (jj=1 ; jj<32 ; jj++)
		BUS_PUTC((ga_ptr + jj), 0xff);
	    BUS_PUTC((ga_ptr + 4), ld->prms->stop_bits);
	    switch (ld->prms->parity) 
	    {
	       case nopar:
		BUS_PUTC((ga_ptr + 5), 0);
		break;

	       case oddpar:
		BUS_PUTC((ga_ptr + 5), 1);
		break;

	       case evenpar:
		BUS_PUTC((ga_ptr + 5), 2);
		break;

	    default:
		break;
	    }
	    BUS_PUTC((ga_ptr + 6), ld->prms->char_size);
	    BUS_PUTC((ga_ptr + 7), ld->prms->priority);
	    BUS_PUTC((ga_ptr + 0), G_SET_PARM);
	    EndPio();
	    break;

	   case GC_DEF_ERROR:
	    StartPio(slion_pio, ld, DET_BD(); return);
	    BUS_PUTC((ga_ptr + 1), G_RPT_ERRS_PARM);
	    BUS_PUTC((ga_ptr + 0), G_RPT_ERRS);
	    EndPio();
	    break;

	   case GC_CLR_INHIB:
	    SL_printf("C_INHB|");
	    StartPio(slion_pio, ld, DET_BD(); return);
	    BUS_PUTC((ga_ptr + 1), 1);
	    BUS_PUTC((ga_ptr + 0), G_XINHIBIT);
	    EndPio();
	    break;

/* GC_BOX_RST_1&2 are used to delay resetuping the port after a HILINK
** interrupt has occurred.  They will be set together and the reset will
** occur at BOX_RST_2.  The BOX_RST_1 causes the gcomm_tmr to make us
** wait to do the resetupping of the port.
*/
	   case GC_BOX_RST_1:
	    if(!(tp->t_gcommtmr))
		tp->t_gcommtmr = timeout(sgcomm_tmr_f,(caddr_t)ld,hz);
	    ld->prms->gcomm_mask &= ~ii;
	    DET_BD();
	    return;

	   case GC_BOX_RST_2:
	    sbox_reset(ld);
	    break;

	   default:
	    break;
	}
	ld->prms->gcomm_mask &= ~ii;
    }

    if (ld->prms->gcomm_mask) 
    {
	if(!(tp->t_gcommtmr))
	   tp->t_gcommtmr = timeout(sgcomm_tmr_f,(caddr_t)ld,hz/100);
    } 
    else 
    {
	if(tp->t_liflg & FLGLISET)
	{
#ifdef SLI_TRACE
    if(DBFLG & LVL3)
	SL_printf("wrtack|");
#endif
		tp->t_liflg = 0;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = 0;
		mp->b_cont->b_wptr = mp->b_cont->b_rptr;
		tp->t_ctloutmsg = (mblk_t *) 0;
		qreply(q,mp);
	}
    }

    DET_BD();
    return;

} /* gcomm_tmr_f */

/*
 * FUNCTION:	sscan_baud_table
 *
 * PURPOSE:	Get baud rate.
 *
 * INPUT:
 *	arg:	Request speed.
 *
 * RETURN:
 *	ii:	Speed code if success.
 *	-1:	Not found specified speed code.
 *
 * CALLED BY:
 *	slionservice	slinvservice	sbox_reset
 */
static int 
sscan_baud_table(int arg)
{
    int		ii;
    static int baud_table[] = {
	-1,    50,    75,   110,   134,   150,   300,   600,  1200, 
	1800,  2000,  2400,  3600,  4800,  7200,  9600, 19200, 38400,  };


    for (ii=0 ; ii<(sizeof(baud_table)/sizeof(*baud_table)) ; ii++)
	if (arg == baud_table[ii])
	    return ii;
    return -1;
} /* sscan_baud_table */

/*
 * FUNCTION:	slion_err
 *
 * PURPOSE:	Set lion error log record.
 *
 * INPUT:
 *	name:	Routine name.
 *	code:	Type of error.
 *	err: 	Error code.
 *
 * RETURN:
 *      None.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      bcopy		errsave
 *
 * DESCRIPTION:
 *	Record an error log entry in the system error log if an error occurs.
 *
 * CALLED BY:
 *	myttyinput	slionconfig	slionadd	slionopen
 *	slionclose	slionproc	slionservice
 *	slion_offlev	sGotHilink	slion_do_comms
 *	slion_recv
 */
void 
slion_err(char *name, int code, int err)
{
    ERR_REC(sizeof(int)) slion_err;

    SL_printf("S_err|");

    slion_err.error_id = code;
    bcopy(name, slion_err.resource_name, sizeof(slion_err.resource_name));
    *(int *)slion_err.detail_data = err;
    errsave(&slion_err, sizeof(slion_err));

} /* slion_err */

#ifdef DO_PIO

/*
 * FUNCTION:	slion_pio
 *
 * PURPOSE:	Enable/disable board access.
 *
 * INPUT:
 *      ld:	Pointer to sliondata structure.
 *      pud:	Pointer to result value.
 *      logit:	Action to be performed.
 *		- (0): No log. ERROR.
 *		- (1): Log Permanent error.
 *		- (2): LOg temporary error.
 *
 * RETURN:
 *	None.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	sxu_assist	bcopy 		errsave
 *
 * DESCRIPTION:
 *	Start/stop physical operation and logging in error log.
 *
 * CALLED BY:
 *	This routine is called via MACRO StartPio by:
 *	slionconfig  slionadd  sliondel  slioc_slpx  
 *	slioc_dslp  slioc_pres  slioc_dram  sl_dram_wr  slioc_trng 
 *	slionproc  slion_slih  slion_offlev  slion_send  slion_comm 
 *	slion_do_comms  _slion_rec  sgcomm_tmr_f 
 */
void 
slion_pio(struct sliondata *ld, pud_t *pud, int logit)
{
    ERR_REC(sizeof(pud_t))	pio_err;
    int				inxbox;
    char			*name;

#ifdef SLI_TRACE
    if(DBFLG==CNFG)
	SL_printf("PIO(%x %x %x)",ld,pud,logit);
#endif

    if (ld == 0 || ld == (struct sliondata *)1) 
    {
	inxbox = (int)ld;
	name = resource;
    } 
    else 
    {
	name = ld->tp->t_name;
	inxbox = (ld->flags & XBOX_FLAG) ? 1 : 0;
    }

#ifdef SLI_TRACE
    if(DBFLG==CNFG)
	SL_printf("PIO(name=%s)",name);
#endif

    /* First lets clean up the bus, etc */
    if (logit != 2) 
    {			/* not the temp error log call */
	pud->p_except = *(struct pio_except *)csa->except;
	pud->p_check = 0;
	pud->p_mask = 0;
	if (inxbox && sxu_assist)
	    (*sxu_assist)(&pud->p_mask);
    }

    if (logit) 
    {
	pio_err.error_id = (logit == 1) ? ERRID_COM_PERM_PIO :
	    					ERRID_COM_TEMP_PIO;
	bcopy(name, pio_err.resource_name, sizeof(pio_err.resource_name));
	*(pud_t *)pio_err.detail_data = *pud;
	errsave(&pio_err, sizeof(pio_err));
    }

} /* slion_pio */
#endif /*DO_PIO */
