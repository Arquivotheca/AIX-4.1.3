/* @(#)86	1.11.1.6  src/bos/kernext/dlc/sdl/sdlc.h, sysxdlcs, bos411, 9428A410j 4/7/94 13:20:44 */
/*
 * COMPONENT_NAME: SYSXDLCS SDLC DATA LINK CONTROL
 *
 * FUNCTIONS: common header file for SDLC Code
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
**      File Name      : 86
**
**      Version Number : 1.11.1.6
**      Date Created   : 94/04/07
**      Time Created   : 13:20:44
*/


/************************************************************************/
/*									*/
/*      MM     MM     AAA     LL       LL        OOOOO    CCCCC		*/
/*      MMM   MMM    AA AA    LL       LL       OO   OO  CC   CC	*/
/*      MM M M MM   AA   AA   LL       LL       OO   OO  CC		*/
/*      MM  M  MM  AAA   AAA  LL       LL       OO   OO  CC		*/
/*      MM     MM  AAAAAAAAA  LL       LL       OO   OO  CC		*/
/*      MM     MM  AA     AA  LL       LL       OO   OO  CC   CC	*/
/*      MM     MM  AA     AA  LLLLLLL  LLLLLLL   OOOOO    CCCCC		*/
/*									*/
/************************************************************************/

#define MULT_PU

#ifndef H_SDLC
#define H_SDLC

/*
**	Additional include files
*/
#include <sys/sdlerr.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/sdlextcb.h>
#include <sys/fp_io.h>
#include "dlcadd.h"
#include <sys/gdlextcb.h>
#include "sdldefs.h"
#include <sys/comio.h>
#include <sys/mpqp.h>
#include <sys/mbuf.h>

/*
*****************************************
**	flags				*
*****************************************
*/

#ifdef MULT_PU
 #define MAX_NUM_STATIONS    255         /* number of secondary stations */
#endif

struct flags
{
	/*
	** timer flags
	*/
	uchar	idle_timer_popped;	/* idle poll time out flag	*/
	uchar	slow_timer_popped;	/* idle poll time out flag	*/
	/*
	** enqueue pending
	*/
	uchar	enque_disc_pending;
	uchar	enque_snrm_pending;
	/*
	** flags
	*/
	uchar	pf_bit;
	uchar	poll_final_sent;
	/*
	** physical link flags
	*/
	uchar	phy_starting;
	uchar	halt_pending;
	uchar	sap_aborted;
	/*
	** others
	*/
	uchar	device_opened;
	uchar	xmit_endframe_expected;
	uchar	no_active;
	uchar	ignore;
	uchar	process_ending;
#ifdef MULT_PU
	uchar   mpu_enabled;            /* TRUE = Multi-PU mode enabled */
#endif
} flags;

/*
*****************************************
** physical link characterisics		*
*****************************************
*/

struct	physical_link
{
	struct	dlc_esap_arg	esap;
	struct	ST_DEV_PARM	mpqp;	/* mpqp device dependent struct	*/
};

/*
*****************************************
** device characteristics		*
*****************************************
*/

struct	logical_link
{
	struct	dlc_sls_arg	sls;	/* generic start link stn flds	*/ 
	struct	sdl_start_psd	sdl;	/* sdlc specific fields		*/
};

/*
*****************************************
**	RAS counters			*
*****************************************
*/

struct counters
{
	ulong	test_cmd_sent;		/* test frame sent counter     	*/
	ulong	test_cmd_fail;		/* pri test command failure ctr	*/
	ulong	test_cmd_rec;		/* sec tests received counter  	*/
	ulong	i_frames_sent;		/* total i-frame xmitted ctr   	*/
	ulong	i_frames_resent;	/* i-frame bursts retransmitted	*/
					/*    counter                  	*/
	ulong	contig_resent;		/* contigous i-frame bursts    	*/
					/*  retransmitted counter      	*/
	ulong	i_frames_rec;		/* i-frame received valid ctr  	*/
	ulong	invalid_i_frame;	/* frames received invalid ctr 	*/
	ulong	adapter_det_rx;		/* adapter detected receive    	*/
					/*    errors counter           	*/
	ulong	adapter_det_tx;		/* adapter detected transmit   	*/
					/*    errors counter           	*/
	ulong	sec_inact_to;		/* sec inactivity timeout ctr  	*/
	ulong	ttl_polls_sent;		/* pri total   polls counter   	*/
	ulong	ttl_repolls_sent;	/* pri total repolls counter   	*/
	ulong	contig_repolls_sent;	/* contiguous repolls ctr  	*/
};


/*
*****************************************
**	control byte masks              *  
*****************************************
*/

#define	U_MASK      0x03		/* 00000011  binary 		*/
#define	S_F_MASK    0x01		/* 00000001  binary 		*/
#define	POLL_MASK   0x10		/* 00010000  binary 		*/
#define	NR_MASK     0xE0		/* 11100000  binary 		*/
#define	NS_MASK     0x0E		/* 00001110  binary 		*/
#define	RR_MASK     0x0C		/* 00001100  binary 		*/
#define	DATA_MASK   0x01		/* 00000001  binary 		*/
 
 
/*
*****************************************
**	timer struct			*
*****************************************
*/
struct	timer
{
	int	enabled;		/* true if timer is being used	*/
	int	ticks;			/* ticks remaining 		*/
};
typedef	struct timer		TIMER;

/*
*****************************************
**	SDLC frame information offsets	*
*****************************************
*/


struct	frame_ct_info
{
	union
	{
		uchar	nr_ns_byte;
		uchar	type;
		uchar	control;
		struct	          
		{
			unsigned	nr	: 3;
			unsigned	pf_bit	: 1;
			unsigned	ns	: 3;
			unsigned		: 1;
		} count;
	} frame;
};

/*
*****************************************
** SDLC monitor trace 			*
*****************************************
*/
#define	MAX_MON_DATA	4
#define	TYPE_BYTES	4		/* number of bytes in type fild	*/
#define	TRACE_TBL_SIZE	2000

struct	monitor_rec
{
	char	type[TYPE_BYTES];	/* ASCII operation label	*/
	ulong	corr;			/* address of current station	*/
	ulong	results;		/* operation return code	*/
	char	data[MAX_MON_DATA];
};

/*
*****************************************
**	Transmit queue information	*
*****************************************
*/

struct	tx_qe				/* transmit queue queue-element	*/
{
	struct	mbuf	*m;
	ulong	held;			/* true if buffer has been sent	*/
					/* to the device handler	*/
};

typedef	struct link_station	LINK_STATION;

/*
*****************************************
*	SDLC poll list			*
*****************************************
*/
struct	poll_list
{
	LINK_STATION	*head;
	LINK_STATION	*tail;
	int		count;
};

/*
*****************************************
**	SDLC Link Station Struct	*
*****************************************
*/

struct link_station
{
	LINK_STATION	*back;		/* previous link station	*/

	struct	logical_link	ll;	/* device characteristics	*/
	struct	counters	ct;	/* counters			*/

#ifdef MULT_PU
    struct dlc_chan *cid;      /* Value of cid for multiple opens */

    /*
    ** retry user information
    */
    struct  mbuf    *i_m;       /* saved i frame            */
    struct  dlc_io_ext i_block; /* saved i/o block info     */
    struct  mbuf    *x_m;       /* saved xid data           */
    struct  dlc_io_ext x_block; /* saved i/o block info     */
	LINK_STATION *retry_next;   /* next link station with retry information */
	LINK_STATION *retry_back;   /* previous link station with retry info */
#endif

	/*
	** auto response information
	*/
	uchar	addr;
	uchar	tx_control;
	uchar	rx_control;

	uchar	in_list;		/* TRUE when station is in	*/
					/* active or quiesce list	*/

 	TIMER	abort_timer;
#ifdef MULT_PU
	TIMER   inact_timer;            /* mpu inactivity timer         */
#endif

	struct	tx_qe	tx_que[TX_Q_LEN];	
	int	in;			/* points to next available 	*/
					/* slot in transmit queue	*/

	/*
	** station status
	*/
	uchar	ll_status;		/* 0 -> CLOSED			*/
					/* 1 -> OPENED  		*/
					/* 2 -> CLOSING  		*/
					/* 3 -> OPENING  		*/

	uchar	mode;			/* 0 -> QUIESCE			*/
					/* 1 -> NDM			*/
					/* 2 -> NRM			*/

	uchar	poll_only;		/* only allow polls to be sent	*/
	uchar	poll_mode;		/* active, slow or idle		*/

	ulong	sub_state;
	ulong	repoll_count;
	ulong	total_poll_count;
	ulong	burst_rexmitted;
	ulong	total_burst_xmitted;

	/*
	** pending flags
	*/
	ushort	unnum_cmd;		/* pending commands		*/
	ushort	unnum_rsp;		/* pending responses		*/

	uchar	frmr_response_pending;
	uchar	inact_pending;
	uchar	ack_pending;
	uchar	retransmission_flag;
	uchar	xmit_que_empty;


	/*
	** timer
	*/
#ifdef MULT_PU
	ulong   abort_ticks;            /* abort timeout x10 ticks/sec  */
	ulong   inact_ticks;            /* inact timeout x10 ticks/sec  */
#endif
	uchar	abort_cancel;		/* abort term timer cancel	*/
	uchar	abort_running;		/* abort timer			*/
	uchar	rnr;			/* receiver not ready flag	*/

	/*
	** transmission queue
	*/
	uchar	nr;			/* number of frames received	*/
	uchar	ns;			/* number of frames sent	*/
	uchar	ack_nr;			/* last nr received     	*/
					/* also tx queue out pointer	*/
	uchar	tx_ns;			/* transmitted ns count		*/
	ulong	long_a;

	/*
	** link trace
	*/
	ulong	max_trace_size;
	/*
	** misc
	*/
	struct	tx_qe	xid;
	struct	tx_qe	test;
	short	disc_reason;		/* disconnect reason       	*/
	uchar	failed_control_byte;	/* used by frame reject		*/
	uchar	frmr_reason;		/* reason for failure		*/
	uchar	last_sent;		/* last control byte sent  	*/
	uchar	last_rcvd;		/* last control byte rcvd  	*/
	uchar	saved_ns;		/*                         	*/
	uchar	saved_repoll;		/* saved repoll value      	*/
	uchar	conn_pending;		/* connection pending flag 	*/
	uchar	s_frame_ct;		/* count  of consecutive   	*/
                                        /* times both pri and sec  	*/
                                        /* sends rr or rnr         	*/
	uchar	rec_first_pending;	/* lease line only : start 	*/
                                        /* inactivity only after   	*/
                                        /* received first frame    	*/
	uchar	auto_resp;		/* state of the auto resp mode	*/

	LINK_STATION	*next;		/* pointer to next LS in list	*/
};



/*
*****************************************
**	SDLC Control Block		*
*****************************************
*/

typedef	int	SP_FILE;		/* temporary name (to be det.)	*/

struct port_cb 
{
	/*
	** generic control block information
	*/
	struct	dlc_port	dlc;

#ifdef MULT_PU
	/*
	** removed mpx channel pointer
	** removed function pointers
	** removed retry user information
	*/
#endif
/* defect 141966 */
        struct  port_cb *port_cb_addr;  /* port control block address   */
/* end defect 141966 */
	struct	mbuf	*n_m;		/* saved network data		*/
	struct	dlc_io_ext n_block;	/* saved i/o block info		*/

	SP_FILE	*fp;				/* used for devcalls	*/

	ulong	rc;
	uchar	sdllc_trace;			/* internal trace (yes)	*/
	uchar	poll_seq_sw;
	uchar	station_type;			/* 0=sec  1=primary     */
	uchar	pl_status;			/* physical link status	*/
						/* 0=closed, 1=opening  */
						/* 2=opened, 4=closing  */

	struct	mbuf	*m;			/* most recently rcvd	*/
						/* data buffer		*/
	uchar	*frame_ptr;			/* pointer to the data 	*/
						/* area in the rcv buf	*/

	struct	tx_qe	write;			/* current outgoing 	*/
						/* data buffer		*/
	uchar		*write_frm;		/* pointer to data area	*/
						/* of outgoing buffer	*/

	ulong	trace_index;			/* next available slot	*/
	struct	monitor_rec	trace_tbl[TRACE_TBL_SIZE];

	uchar	*errptr;
	uchar	sdl_error_rec[300];
	int	err_vec_size;
	ulong	sense;				/* sdlc sense data	*/


	/*
	**	comio session information
	*/
	struct	session_blk	session;

	/*
	**	physical link configuration data
	*/
	struct	physical_link	pl;

	/*
	**	poll lists
	*/
	struct	poll_list	active_list;
	struct	poll_list	quiesce_list;

	uchar	poll_list_cmd;			/* poll list command    */
	uchar	poll_list_flag;			/* 1=repeat list        */
	ushort	poll_list_time;			/* receive timeout      */

	/*
	**	timers and timer information
	*/
	TIMER	slow_timer;
	TIMER	idle_timer;
 	TIMER	repoll_timer;

	ulong	abort_ticks;
	ulong	idle_ticks;
	ulong	slow_ticks;

	LINK_STATION	*slow_marker;	/* first slow station polled	*/
	LINK_STATION	*idle_marker;	/* first idle station polled	*/

	ulong	slow_count;		/* num stations in slow mode	*/
	ulong	idle_count;		/* num stations in idle mode	*/

	/* 
	**	misc
	*/
/* defect 115819 removed proc_term */
	ulong	wait_end_ar;		/* event word - used when	*/
					/* waiting for auto response	*/
					/* mode to end			*/
	struct	mbuf	*stored_m;
	ushort	header_len;		/* buf length exclude data 	*/
	ushort	max_i_frame;		/* max i frame size		*/
	ushort	entire_buf_size;	/* length of the whole buf 	*/


	/*
	**	temporary receive storage
	*/
	uchar	control_byte;
	uchar	stored_control_byte;
	uchar	saved_inact_to;
						/* poll list		*/
	/*
	**	link information
	*/
	uchar	num_opened;			/* number of instances	*/
						/* opened		*/

	LINK_STATION	*active_ls;		/* active link station	*/

	struct	flags	flags;			/* timer flags		*/

#ifdef MULT_PU
	/* Link station array. This is an array of pointers into the
	   link_station structure. This logic was added as part of the
	   multiple secondary station logic */
	struct link_station *link_station_array[MAX_NUM_STATIONS+1];

	/* cid of process that did first successful ENABLE_SAP*/
	struct dlc_chan *sap_cid;

	/* Pointer to a link station that has retry information */
	LINK_STATION *retry_list;

	struct  poll_list mpu_sta_list; /* head of multi-pu station list */
					/* used for fast timer scans     */
#endif

};
typedef	struct port_cb	PORT_CB;

/*
*****************************************
*	queue element templates		*
*****************************************
*/
struct	rx_qe			/* used for received data	*/
{
	ulong	type;		/* receive or exception		*/
	ulong	status;	 	/* receive status		*/
	ulong	m_addr;		/* address of mbuf		*/
	ulong	options[3];	/* unused			*/
};

struct	status_qe		/* used for status block	*/
{
	ulong	type;		/* type of status block		*/
	ulong	op[5];
};

/*
*****************************************
*	Misc macros 			*
*****************************************
*/
#define	ENABLE(_p)	((struct timer *)(&_p))->enabled = TRUE
#define	DISABLE(_p)	((struct timer *)(&_p))->enabled = FALSE
#define SETTIMER(_p, _q)((struct timer *)(&_p))->ticks = _q 

/* increments counter _c as long as it is not equal to threshold _t */
#define	INC_COUNTER(_c, _t)	((_c != (ulong) _t) ? (++_c) : (_c=_t))

/*
** increments the counter using modulus MOD
** i.e. IMOD(7) would return 0
*/
#define	IMOD(_c)	(_c = (_c + 1) % MOD)

/*
*****************************************
*	Misc definitions		*
*****************************************
*/

#define FIRST_3_CLEAR	0x1F			/* clear first_3_bits	*/
#define LAST_5_CLEAR	0xE0			/* clear last 5 bits	*/

 
#define	SLOT_MASK	0x70			/* 01110000 binary	*/
#define	ERROR_MASK	0x0F			/* 00001111 binary	*/
#define	OVF_MASK	0x04			/* 00000100 binary	*/

#define DATA_REC       		0x80000000	/* normal data received	*/
#define XID_REC        		0x40000000	/* xid data received   	*/
#define TEST_COMPLETE  		0x20000000	/* test complete       	*/
#define PHY_LINK_OPENED		0x10000000	/* physical link opened	*/
#define PHY_LINK_CLOSED		0x08000000	/* physical link closed	*/
#define LOG_LINK_OPENED		0x04000000	/* logical link opened 	*/
#define LOG_LINK_CLOSED		0x02000000	/* logical link closed 	*/
#define DIAL           		0x01000000	/* dial the call       	*/
#define INACT_WO_TERM   	0x00800000	/* inactivity w/o term 	*/
#define INACT_ENDED     	0x00400000	/* inactivity ended    	*/
#define CONTACTED       	0x00200000	/* contacted           	*/
#define MODE_SET_SEC    	0x00100000	/* set secondary mode  	*/
#define MODE_SEC_FAIL   	0x00080000	/* mode set sec failed 	*/
#define MODE_SET_PRI    	0x00040000	/* mode set primary    	*/
#define MODE_PRI_FAIL   	0x00020000	/* mode set pri failed 	*/
#define NET_DATA        	0x00010000	/* x21/hayes data     	*/
#define SAPS            	0x00004000	/* sap statistics     	*/
#define STAS            	0x00002000	/* link  statistics   	*/
#define RMTA            	0x00001000	/* remote address     	*/
#define BUF_OVERFLOW    	0x00000002	/* buffer_overflow     	*/
#define RESP_PENDING    	0x00000001	/* response pending    	*/


#endif	/* H_SDLC */
