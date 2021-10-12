/* @(#)63	1.7  src/bos/kernext/sol/soldd.h, sysxsol, bos411, 9428A410j 10/26/93 10:25:21 */
#ifndef _H_SOLDD
#define _H_SOLDD
/*
 * COMPONENT_NAME: (SYSXSOL) - Serial Optical Link Device Handler Include File
 *
 * FUNCTIONS: soldd.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/mbuf.h>
#include <sys/comio.h>
#include <sys/devinfo.h>
#include <sys/utsname.h>
#include <sys/lockl.h>
#include <sys/intr.h>

/*
 *  Misc. type definitions (needed for other include files)
 */
typedef ushort	imcspid_t;
typedef uchar	slaid_t;
typedef	uchar	slalinka_t;

#include "sol_defs.h"
#include "sol_vars.h"
#include "sol_macros.h"
#include "sol_errids.h"
#include <sys/soluser.h>

/*
 *  Miscellaneous constants
 */

#define SOL_TOTAL_OPENS		SOL_MAX_OPENS+SOL_MAX_PORTS
					/* max normal + max diag opens  */
#define SOL_TOTAL_NETIDS	(256)	/* one byte netid 0..255	*/
#define SOL_SER_SC		(0x83)	/* start of serialized sc range */
#define SOL_SC_MAX		(SOL_SER_SC+SOL_MAX_OPENS*2-1)
#define SOL_MIN_SC		(0x81)	/* start of soldd subch. (mbuf) */
#define SOL_MAX_PORTS		(4)	/* max number of ports		*/
#define SOL_MAX_PID		(254)	/* max valid processor id	*/
#define SOL_MAX_MMBUFS		(3)	/* number of small mbuf chains	*/
#define SOL_MAX_CMBUFS		(3)	/* number of cluster chains	*/
#define SOL_MMBUF_LEN		(4)	/* number in small mbuf chain	*/
#define SOL_CMBUF_LEN		(15)	/* number in cluster chain	*/
#define SOL_LOW_MMBUFS		(1)	/* low water mark - mbuf chains	*/
#define SOL_LOW_CMBUFS		(1)	/* low water mark - clusters	*/
#define SOL_OPS_MINOR		(4)	/* minor number for /dev/ops	*/
#define SOL_SLA_SRVAL		(0x80000000)	/* seg reg value	*/
#define SOL_CFG_OFFSET		(0x4)	/* offset to config register	*/
#define SOL_SLIH_LEVEL		INTCLASS2	/* slih level		*/
#define SOL_PROC_OFFSET		(0)	/* offset to proc_id in data	*/
#define SOL_NETID_OFFSET	(0)	/* offset to netid in rcv frame	*/
#define SOL_CRC_CHECK		(0x10000000)	/* check for CRC bit	*/
#define SOL_VPDSIZE		(4)	/* size of VPD data		*/
#define SOL_KPROC_EVENT		(0x80000000)	/* mask for kproc	*/
#define SOL_IMCS_QID		(0x80)	/* queue id for IMCS		*/
#define	SOL_PKT_TOO_LONG	(1)	/* return value from check_mbuf	*/
#define	SOL_NO_CLUSTER		(2)	/* return value from check_mbuf */
#define	SOL_BAD_ALLIGNMENT	(4)	/* return value from check_mbuf */
#define SOL_TRACE_SIZE          (500*4) /* max number of trace table entries */
#define SOL_SMALL_COPY		(2)	/* flag for sol_check_mbufs	*/

struct sol_trace_tag
{
        int     next;   /* next index hole to put trace data in the table */
        int     res1;
        int     res2;
        int     res3;
        ulong   table[SOL_TRACE_SIZE];
};
typedef struct sol_trace_tag sol_trace_t;


/*
 *  Receive queue definition - the queue is basically a list of mbuf
 *  chains linked together with the m_nextpkt field.  Therefore there is
 *  actually no limit (other than running out of mbufs) to the size
 *  of the receive queue, but it is limited to the value specified in
 *  the dds to prevent hogging all the mbufs.
 */
struct sol_recv_que {
	struct mbuf	*head,		/* ptr to head of mbuf chain	*/
			*tail;		/* ptr to tail of mbuf chain	*/
};

/*
 *  Status queue definition - the queue is a ring queue
 *  so that if head=NULL, it is empty, if head=tail, there is one element,
 *  and if tail->next=head, it is full.
 */

struct sol_sta_que_elem {
	cio_stat_blk_t		stat_blk;	/* status block		*/
	struct sol_sta_que_elem	*next;		/* next stat elem	*/
};

struct sol_stat_que {
	struct sol_sta_que_elem	*head,		/* ptr to head of list	*/
				*tail;		/* ptr to tail of list	*/
};
/*
 *  Open structure definition
 *  -------------------------
 *
 *  The array of open structures is indexed by the mpx channel number.
 *  The netid table can be used to get the open pointer given the netid.
 */

struct sol_open_struct {
	chan_t		chan;		/* mpx channel number		*/
	ulong		devflag;	/* open mode (kernel or user)	*/
	void		(*recv_fn)();	/* kernel mode receive func.	*/
	void		(*xmit_fn)();	/* kernel mode transmit func.	*/
	void		(*stat_fn)();	/* kernel mode status func.	*/
	struct sol_recv_que recv_que;	/* user mode receive queue	*/
	struct sol_stat_que stat_que;	/* user mode status queue	*/
	int		close_event;	/* event to wait for xmits	*/
	int		xmit_count;	/* # xmits outstanding		*/
	int		recv_event;	/* event for blocked read	*/
	int		recv_count;	/* number of receive packets	*/
	int		stat_count;	/* number of status elements 	*/
	ulong		open_id;	/* from open ext. - kernel mode	*/
	int		select_req;	/* flags requesting selnotify	*/
	uchar		subchannel;	/* first of a pair of sc's	*/
	uchar		stat_full;	/* status queue is full		*/
	uchar		num_netids;	/* # netids started w/ this open*/
	uchar		serialize;	/* serialize (T or F)		*/
};
	
/*
 *  DDS structure definition
 */

struct sol_dds {
	uchar	dds_type;
#define SOL_PORT_DDS	1			/* DDS for a specific port */
#define	SOL_OPS_DDS	2			/* DDS for the subsystem   */
	uchar	rsvd0;				/* Reserved, should be 0   */
	union {
		struct sol_ops {		/* Subsystem type DDS	   */
			uchar	processor_id;	/* Processor ID		   */
			uchar	rsvd1;		/* Reserved, should be 0   */
			int	rec_que_size;	/* Max receive queue size  */
			int	sta_que_size;	/* Max status queue size   */
			int	machine_id;	/* Reserved, should be 0   */
		} sol_ops;
		struct sol_port {		/* Port type DDS	   */
			uchar	buid;		/* BUID of this port	   */
			uchar	rsvd1;		/* Reserved, should be 0   */
			uint	rsvd2;		/* Reserved, should be 0   */
		} sol_port;
	} un;
	uint	rsvd3;				/* Reserved, should be 0   */
	uint	rsvd4;				/* Reserved, should be 0   */
};

/*
 *  DDI structure
 *  -------------
 *
 *  The DDI structure is a single structure for the entire driver.
 *  Anything that has to do with the operation of the driver as a
 *  whole rather than individual opens is kept here.
 */

struct sol_ddi {
	lock_t			global_lock;
					/* global lock word		*/
					/* NOTE: must be first in struct*/
	dev_t			devno;
					/* ops major/minor number	*/
	uchar			sla_buid[SOL_MAX_PORTS];
					/* PIO address for each port	*/
	struct sol_ops		ops_info;
					/* processor id, rcv_que_size...*/
	uchar			port_state[SOL_MAX_PORTS];
					/* open status of each port:	*/
#define SOL_NO_PORT	0		/* port is not configured	*/
#define SOL_NORM_MODE	1		/* port not in diag mode	*/
#define	SOL_DIAG_MODE	2		/* port is in diag mode		*/
	uchar			ser_sc[SOL_MAX_OPENS];
					/* table of serialized sc's	*/
#define SOL_SC_AVAIL	0		/* this subchannel(pair) avail.	*/
#define SOL_SC_IN_USE	1		/* this subchannel(pair) in use	*/
	uchar			chan_state[SOL_TOTAL_OPENS];
					/* table of channel states	*/
#define SOL_CH_AVAIL	0		/* this channel available	*/
#define SOL_CH_NORM	1		/* normal open on this channel	*/
#define SOL_CH_SNORM	2		/* serialized open on this chan	*/
#define SOL_CH_DIAG	3		/* diag open on this channel	*/
#define SOL_CH_FDIAG	4		/* forced diag open on this chan*/
#define SOL_CH_CLOSED	5		/* close in-progress		*/
	uchar			start_state;
					/* start status of all devices	*/
#define	SOL_HALTED	0		/* device is halted		*/
#define SOL_STARTING	1		/* start is in progress		*/
#define SOL_STARTED	2		/* device is started		*/
#define SOL_HALTING	3		/* halt is in progress		*/
	uchar			num_diag_opens;
					/* number of diagnostic opens	*/
	uchar			num_norm_opens;
					/* number of normal opens	*/
	uchar			num_netids;
					/* number of netids started	*/
	uchar			num_ports;
					/* number of ports configured	*/
	uchar			ops_config;
					/* ops configured (T/F)		*/
	struct mbuf		*freembuf;
					/* small mbuf free list		*/
	struct mbuf		*freeclus;
					/* cluster mbuf free list	*/
	struct super_header	*hdrmlist;
					/* list of available mbuf hdrs	*/
	struct super_header	*hdrclist;
					/* list of available clus. hdrs	*/
	int			num_mbuf;
					/* current num of mbuf chains	*/
	int			num_clus;
					/* current num of clus. chains	*/
	struct sol_open_struct	*open_ptr[SOL_TOTAL_OPENS];
					/* array of open structure ptrs	*/
	/*
	 *  The netid table is an array of direct pointers to open
	 *  structures.  Odd netids are group ids and are not valid,
	 *  so all we need is the even ones.  Therefore the netid is
	 *  divided by two to get the correct element in the array.
	 */
	struct sol_open_struct	*netid_table[SOL_TOTAL_NETIDS/2];
					/* maps netid to open pointer	*/
	sol_query_stats_t	stats;
					/* stats for CIO_QUERY		*/
	struct mbreq		mbreq;
					/* structure for mreg call	*/
	struct cdt		*cdt;
					/* struct for component dump	*/
#ifdef DEBUG
	sol_trace_t		soltrace; /* internal trace structure */
#endif
};

/*
 *  The following structure contains data needed for receive.  The save
 *  array is used to keep all the pointers to the mbufs for receive data.
 *  The tag_save variable is used to save the tagword when this mbuf (or
 *  cluster) is put on the free list to be re-used.  Note that the tag_save
 *  field is the only field in the entire super-header that is not actually
 *  used in an imcs_header.  Instead it is used in a normal mbuf, but this
 *  is a convenient way to store this information someplace where it won't
 *  mess up the important info in the mbuf.
 */
struct rcv_info {
	struct mbuf	*msave[NUM_HDR_TCWS];	/* save array for rcv */
	uint		tag_save;		/* saves tag word for re-use */
};

/*
 *  The following info is needed when a xmit completes.  The msglen is needed
 *  for keeping statistics.  The mptr is needed so the mbuf chain can be
 *  freed.  The chan is needed to access the open structure.  The write_id
 *  is needed for the status block to the user.  And the flags are used
 *  to indicate if the mbufs are to be freed, and if a transmit acknowledgement
 *  is expected.
 */
struct tx_info {
	uint		msglen;			/* packet size, for stats */
	struct mbuf	*mptr;			/* ptr to first mbuf sent */
	chan_t		chan;			/* mpx chan */
	ulong		write_id;		/* from write extension */
	ulong		flags;			/* from write extension */
	struct mbuf	*small_mbuf;		/* for small-copy case */
	struct mbuf	*orig_mbuf;		/* for collapse case */
};

/*
 *  The following structure is needed in the off_level interrupt handlers
 *  to be able to access the super_header, given the address of the interrupt
 *  stucture.
 */
struct ourintr {
	struct intr		intr;		/* standard intr structure */
	struct super_header	*super_header;	/* ptr to start of header */
};
/*
 *  A 256 mbuf is used for an imcs header.  By casting the mbuf as a 
 *  super_header, all the additional fields needed for transmit and receive
 *  can also be stored in the mbuf.  Also, by leaving the m_hdr in the
 *  structure, there should be less "fixing-up" required (none).  The intr
 *  structure is needed to schedule either the receive or transmit off-level
 *  routines.
 */
struct super_header {
	struct m_hdr		m_hdr;		/* for spacing	*/
	struct imcs_header	imcs_header;	/* IMCS header	*/
	union {
		struct rcv_info	rcv_info;	/* all info for a rcv mbuf */
		struct tx_info	tx_info;	/* all info for a tx mbuf  */
	} rcv_tx;
	struct ourintr		ourintr;	/* for offlevel scheduling */
};

/*
 *  The following is the cluster descripter as defined by NSC.  A regular
 *  small mbuf is cast to one of these to store the offsets and lengths
 *  for a cluster chain.
 */
struct cl_desc {
	struct m_hdr	m_hdr;			/* for spacing */
	uint		offsets[SOL_CMBUF_LEN]; /* offset to cluster data */
	uint		lengths[SOL_CMBUF_LEN]; /* length of cluster data */
};

/*
 *  Macro definitions
 */

#define MOVEIN(dvf,usa,dda,siz)                               \
( ((char *)usa == (char *)NULL) ? (EFAULT) :                  \
      ( (dvf & DKERNEL) ? (bcopy(usa,dda,siz), 0) :           \
            ( copyin(usa,dda,siz) != 0) ) )

#define MOVEOUT(dvf,dda,usa,siz)                              \
( ((char *)usa == (char *)NULL) ? (EFAULT) :                  \
      ( (dvf & DKERNEL) ? (bcopy(dda,usa,siz), 0) :           \
            ( copyout(dda,usa,siz) != 0)  ) )

/*
 *  ADDR_SLA macro
 */
#define ADDR_SLA(sla_num, p) p = (struct slaregs volatile *) \
			io_att(SOL_SLA_SRVAL | \
			(ulong) sol_ddi.sla_buid[sla_num] << 20, 0)
#define UNADDR_SLA(p)	io_det(p)

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif /* MIN */

/*
 *  Prototype definitions
 */
#ifndef _NO_PROTO
int	sol_config(dev_t devno, int cmd, struct uio *uiop);
int	sol_open(dev_t devno, ulong rwflag, chan_t chan, struct kopen_ext
		*ext);
int	sol_close(dev_t devno, chan_t chan);
int	sol_read(dev_t devno, struct uio *uiop, int chan, cio_read_ext_t *arg);
int	sol_write(dev_t devno, struct uio *uiop, chan_t chan,
	    struct write_extension *ext);
int	sol_ioctl(dev_t devno, int op, int arg,ulong flag,chan_t chan,int ext);
int	sol_select(dev_t devno, ushort events, ushort *reventp, chan_t chan);
int	sol_mpx(dev_t devno, int *chanp, char *channame);
int	sol_fastwrt(struct mbuf *m, chan_t chan);
void	sol_xmit_wait(struct sol_open_struct *open_ptr);
void	sol_report_status(struct sol_open_struct *open_ptr, cio_stat_blk_t
	    *stat_blk_ptr);
void	sol_com_write(struct mbuf *mbufp, struct super_header *super_header,
	    struct cl_desc *cl_desc, struct sol_open_struct *open_ptr);
int	sol_check_mbufs(struct mbuf *mptr, struct mbuf **new_mptr,
	    uchar *havecluster, uint delay, uint *msglen);
struct mbuf *sol_collapse(struct mbuf *m, uint msglen, uint nodelay);
void	sol_xmit_notify(struct imcs_header *imcs_hdr);
int	sol_xmit_offlevel(struct intr *iptr);
void	sol_start_done(int status);
int	sol_get_rcv(struct sol_open_struct *open_ptr, struct sol_recv_que
	    *recv_que, struct mbuf **mptr);
void	sol_rcv_notify(struct imcs_header *h);
int	sol_rcv_offlevel(struct intr *iptr);
struct super_header *sol_buildchain(short wantclus);
caddr_t	sol_get_header(uint subch);
void	read_registers(struct slaregs volatile *sla_ptr,
		struct sol_sla_status *status_struct, uchar read_status_flag);
uint	sol_spin(struct slaregs volatile *sla_ptr);
int	cancel_sla(struct slaregs volatile *sla_ptr,
		struct sol_sla_status *status_struct);
int	move_sla_words(int *source_addr, int *dest_addr, uint length);
int	sol_lock_to_xtal(int sla_num, int arg, ulong devflag, chan_t chan);
uint	pulse_lock_to_xtal(struct slaregs volatile *sla_ptr, uchar wrap_mode);
int	sol_iocinfo(int arg, ulong devflag, chan_t chan);
int	sol_cio_start(int arg, ulong devflag, chan_t chan);
int	sol_cio_halt(int arg, ulong devflag, chan_t chan);
void	sol_shutdown();
int	sol_cio_query(int arg, ulong devflag, chan_t chan);
int	sol_cio_get_stat(int arg, ulong devflag, chan_t chan);
int	sol_cio_get_fastwrt(dev_t devno, struct cio_get_fastwrt *arg,
		ulong devflag, chan_t chan);
int	sol_get_prids(int arg, ulong devflag, chan_t chan);
int	sol_check_prid(int arg, ulong devflag, chan_t chan);
int	sol_buffer_access(int sla_num, int arg, ulong devflag, chan_t chan);
int	sla_dma_unhang(struct slaregs volatile *sla_ptr, uint page_raddr);
int	startup_chip(struct slaregs volatile *sla_ptr,
		struct sol_sla_status *status_struct, unsigned int page_raddr,
		int sla_num);
int	stop_sla(struct slaregs volatile *sla_ptr,
		struct sol_sla_status *status_struct);
int	sol_sync_otp(int sla_num, int arg, ulong devflag, chan_t chan);
int	sol_activate(int sla_num, int arg, ulong devflag, chan_t chan);
int	sol_scr(int sla_num, int arg, ulong devflag, chan_t chan);
int	sol_ols(int sla_num, int arg, ulong devflag, chan_t chan);
int	sol_rhr(int sla_num, int arg, ulong devflag, chan_t chan);
int	sol_crc(int sla_num, int arg, ulong devflag, chan_t chan);
struct cdt *sol_cdt_func(int arg);
#ifdef DEBUG
void	sol_trace(char *tag, uint w1, uint w2, uint w3,int sysflg);
#endif

#else
int	sol_config();
int	sol_open();
int	sol_close();
int	sol_read();
int	sol_write();
int	sol_ioctl();
int	sol_select();
int	sol_mpx();
int	sol_fastwrt();
void	sol_xmit_wait();
void	sol_report_status();
void	sol_com_write();
int	sol_check_mbufs();
struct mbuf *sol_collapse();
void	sol_xmit_notify();
int	sol_xmit_offlevel();
void	sol_start_done();
int	sol_get_rcv();
void	sol_rcv_notify();
int	sol_rcv_offlevel();
struct super_header *sol_buildchain();
caddr_t	sol_get_header();
void	read_registers();
uint	sol_spin();
int	cancel_sla();
int	move_sla_words();
int	sol_lock_to_xtal();
uint	pulse_lock_to_xtal();
int	sol_iocinfo();
int	sol_cio_start();
int	sol_cio_halt();
void	sol_shutdown();
int	sol_cio_query();
int	sol_cio_get_stat();
int	sol_cio_get_fastwrt();
int	sol_get_prids();
int	sol_check_prid();
int	sol_buffer_access();
int	sla_dma_unhang();
int	startup_chip();
int	stop_sla();
int	sol_sync_otp();
int	sol_activate();
int	sol_scr();
int	sol_ols();
int	sol_rhr();
int	sol_crc();
struct cdt *sol_cdt_func();
#ifdef DEBUG
void	sol_trace();
#endif
#endif /*_NO_PROTO */

/*
 *  Trace table stuff
 */
#define WRITE_ENTRY		"TwrB"		
#define WRITE_EXIT		"TwrE"
#define FASTWRT_ENTRY		"TfwB"
#define FASTWRT_EXIT		"TfwE"
#define COLLAPSE_ENTRY		"PcoB"
#define COLLAPSE_EXIT		"PcoE"
#define COM_WRITE_ENTRY		"TcwB"
#define COM_WRITE_EXIT		"TcwE"
#define XMIT_NOTIFY_ENTRY	"TnoB"
#define XMIT_NOTIFY_EXIT	"TnoE"
#define XMIT_OFFLEVEL_ENTRY	"TolB"
#define XMIT_OFFLEVEL_EXIT	"TolE"
#define RCV_NOTIFY_ENTRY	"RnoB"
#define RCV_NOTIFY_EXIT		"RnoE"	
#define RCV_OFFLEVEL_ENTRY	"RolB"	
#define RCV_OFFLEVEL_EXIT	"RolE"
#define GET_HEADER_ENTRY	"PghB"
#define GET_HEADER_EXIT		"PghE"
#define BUILDCHAIN_ENTRY	"PbcB"
#define BUILDCHAIN_EXIT		"PbcE"
#define GET_MBUF		"PgmB"
#define FREE_MBUF		"PfmB"
#define PASS_MBUF		"PpmB"
	
#endif /* _H_SOLDD */
