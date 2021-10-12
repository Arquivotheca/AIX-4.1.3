/* @(#)96       1.11.1.2  src/bos/kernext/dlc/lan/lanstacb.h, sysxdlcg, bos411, 9428A410j 1/20/94 17:52:18 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: lanstacb.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#ifndef _h_LANSTACB
#define _h_LANSTACB
#if defined(TRL) || defined(FDL)
#define TRLORFDDI
#endif
/**********************************************************************/
/* station control block declaration  */
/**********************************************************************/

#define  DISC_ENA	0x80
#define  XID_ENA	0x40
#define  TEST_ENA	0x20
#define	 NONE_ENA	0x00
#define  CLOSE_PEND_NON_BUF_MASK	0x3800
#define  LS_CLOSE_STATE 0x00 		/* link station closed        */
#define  LS_CALL_PEND   0x01		/* call pending completion    */
#define  LS_LISTEN_PEND 0x02		/* listen pending completion  */
#define  LS_CLOSE_PEND  0x03		/* UA or DM to DISC sent      */
#define  LS_ADM         0xFF		/* logical link opened        */
#define  LS_CONTACTING  0xFE		/* SABME sent waiting for UA  */
#define  LS_ABME_PEND   0xFD		/* waiting for RR after SABME */
#define  LS_ABME        0xFC		/* contacted in norm data mode*/
#define  LS_DISCONTACTING  0xFB		/* DISC sent                  */
#define  T3_INACT       0		/* normal session inactivity  */
#define  T3_ABORT       1		/* discontact abort timeout   */
#define  XID_RESET      0		/* reset                      */
#define  TEST_RESET     0		/* reset                      */
#define  TXQ_SIZE 128              /* transmit queue size per station */

struct station_cb {
	struct 	dlc_sls_arg    ls_profile;   /* from externals        */
#ifdef TRL
	struct  trl_start_psd  trl_start_psd; /* token ring externals */
#endif
#ifdef FDL
	struct  fdl_start_psd  fdl_start_psd; /* FDDI externals */
#endif
	/*--- parms from configuration -----*/
/* fddi deleted migration lraddr_8 and raddr_8[8] */

/* LEHb defect 43788 */
	struct mbuf *retry_rcvx_buf;      /* retry xid buffer pointer */
	struct mbuf *retry_rcvd_buf;      /* retry datagram buffer ptr*/
	struct mbuf *retry_rcvi_buf;      /* retry iframe buffer ptr  */
	struct dlc_io_ext retry_rcvx_ext; /* retry xid extension      */
	struct dlc_io_ext retry_rcvd_ext; /* retry dgrm extension     */
	struct dlc_io_ext retry_rcvi_ext; /* retry i-frame extension  */
/* delete 2 lines */
/* LEHe */

	/*--- parms built at open link -----*/
     	ulong_t   max_trace_size; /* maximum trace entry size   */
     	ulong_t   resp_to_val;   	/* t1 timeout value           */
     	ulong_t   ack_to_val;    	/* t2 timeout value           */
     	ulong_t   inact_to_val;  	/* t3 timeout value (inactivi)*/
     	ulong_t   force_to_val;  	/* t3 timeout value (abort)   */

	/*--- add prebuilt transmit I-frame header area -----*/
#ifdef EDL
       	ushort_t 	dpad;		/* offset from top of data =2 */
       	char 	raddr[6];     	/* destination address(remote)*/
       	char 	laddr[6];	/* source address(local)      */
       	ushort_t 	llc_type;	/* llc type field             */
       	ushort_t 	lpdu_length;    /* length of the lpdu         */
       	u_char 	lpad;     	/* leading pad                */
#endif
#ifdef E3L
       	ushort_t 	dpad;		/* offset from top of data =2 */
       	char 	raddr[6];     	/* destination address(remote)*/
       	char 	laddr[6];	/* source address(local)      */
       	ushort_t 	lpdu_length;    /* length of the lpdu         */
#endif
#ifdef TRLORFDDI
#ifdef FDL
	u_char   reserved[3];
#endif
	u_char   phy_ctl1;      /* physical control byte 1    */
#ifdef TRL
        u_char   phy_ctl2;    	/* physical control byte 2    */
#endif
	u_char   raddr[6];      /* source address(local) */
	u_char   laddr[6];      /*destination address(remote)     */
#define RI_PRESENT 0x80     	/* routing info present       */
/* LEHb defect 44499 */
	struct {                            /* routing control fields */
		unsigned all_route     :1;    /* all rings broadcast    */
		unsigned single_route  :1;    /* limited route broadcast*/
		unsigned unknown       :1;    /* unused                 */
		unsigned ri_lth        :5;    /* length of ri field     */
		unsigned direction     :1;    /* scan direction         */
		unsigned largest_field :3;    /* largest field          */
		unsigned fill          :4;    /* reserved               */
#ifdef TRL
		u_char   routes[16];     /* fills out the 18 bytes of RI */
#elif FDL
		u_char   routes[FDL_ROUTING_LEN -2 ];
#endif
	} ri_field;
/* LEHe */
#endif /* TRLORFDDI */
       	u_char   rsap;           /* dsap(remote)               */
       	u_char   lsap;           /* ssap(local)                */
       	u_char   vs;             /* send state var(next ns sent*/
	u_char   vr;             /* rcv state var(next nr sent */
	/*--- add end of prebuilt transmit I-frame header area -----*/

	u_char   xmit_vs;        /* vs already transmitted */
	u_char   ww;             /* working window value       */
	u_char   nw;             /* working window seq ack ct  */
	u_char   ia_ct;		/* working window ack count   */
	u_char	fill;		/* reserved for alignment     */

	/*----- parms cleared at open link ---*/

       	struct 	dlc_qls_arg ras_counters;   /* from externals  */
	struct	transmit_queue {
		char *buf;
	} transmit_queue[TXQ_SIZE];
#ifdef TRLORFDDI
	ulong_t    ri_length;     /* length of routing info     */
/* LEHb defect 44499 */
	ulong_t    sri_length;    /* length of single route broadcast */
/* LEHe */
#endif
 	ulong_t   que_flag;	        /* set when que is full       */
       	ulong_t	xid_cmd_addr;		/* xid command area address   */
       	ulong_t	test_cmd_addr;          /* test command area address  */
	int       closing_reason;         /* llc closing reason code    */
	char loopback;
       	/* pending_indicators; */
	u_char   inact_without_pend;    /* INACT WITHOUT TERMINATION  */
	u_char   iframes_ena;           /* I-FRAMES ENABLED FOR XMIT  */
	u_char   ignore;                /* GENERAL PURPOSE IGNORE IND.*/
/* LEHb defect 43788 */
/* DEL  u_char   pc_local_busy;            PATH CONTROL LOCAL BUSY    */
/* DEL  u_char   rq_local_busy;            RCV RING QUEUE LOCAL BUSY  */
/* DEL  u_char   dh_local_busy;            DEVICE HANDLER LOCAL BUSY  */
/* LEHe */
	u_char   us_local_busy;         /* USER INITIATED LOCAL BUSY  */
/* <<< feature CDLI >>> */
/* <<< removed close_pend_non_buf >>> */
/* <<< end feature CDLI >>> */
	u_char   last_cmd_1;            /* last cmd ctl byte#1 sent   */
        u_char   rcv_ctl1;              /* last control byte rcvd     */
        u_char   rcv_ctl2;              /* last control byte rcvd     */

#define POLL_FINAL_1 	0x10            /* poll-final indicator       */
#define I_TYPE   	1               /* i-frame bit check          */
       	u_char 	p_ct;                   /* poll retry count           */
       	u_char 	is_ct;                  /* i-lpdu retry count         */
       	u_char	txw_ct;                 /* current transmit window ctr*/
       	u_char	ir_ct;                  /* rcvd i-lpdu count          */
       	u_char	txq_input;              /* transmit queue input ptr   */
       	u_char	txq_output;             /* transmit queue output ptr  */
       	u_char	va;                     /* ack state var (last nr rcvd*/
       	u_char	px;                     /* last xidc poll mask rcvd   */
       	u_char	rcvd_nr;                /* received nr count          */
       	u_char	rcvd_ns;                /* received ns count          */
       	u_char   ls;                     /* valid rcv state indicator  */
#define  LS_RCV_VALID 0x80          /* valid rcv state indicator  */
     	u_char	vc;                     /* ns command stacked in abme */
        u_char 	local_busy  ;           /* local station out of bufs  */
        u_char 	remote_busy ;           /* remote station out of bufs */
        u_char 	rejection   ;           /* rejecting out of sequence i*/
        u_char 	checkpointing ;         /* command pending a response */
        u_char 	clearing    ;           /* clearing of local busy is  */
        /* pending (substate of chkpt.*/
	u_char	t3_state;               /* t3 timer state             */
	u_char	xs;                     /* XID state                  */
       	u_char 	xid_ir_pend ;           /* incomming response pending */
       	u_char 	xid_or_pend ;           /* outgoing response pending  */
	u_char   ts;			/* test state                 */
       	u_char 	test_keep_alive ;       /* adm test keep alive mode   */
       	u_char 	test_ir_pend    ;       /* incomming response pending */
	u_char  sta_limbo;              /* sta limbo mode - can't wrt */
        u_char  sta_cache;              /* name cache status flag     */
#define CACHE_NO_NAME     0x00          /* no name in name cache      */
#define CACHE_WRONG_NAME  0x01          /* wrong address in name cache */
#define CACHE_NAME        0x02          /* cache name used in find    */
        u_char  cache_pindex;           /* index to cache name        */
        u_char   inact_pend;            /*  inactivity pending status */
#define INACT_LIMITED     0x01          /* limited braodcast pending  */
#define INACT_ALL_ROUTE   0x02          /* all routes broadcast pend  */
#define INACT_COMPLETE    0x03          /* inactivity pending complete */
#ifdef TRLORFDDI
	u_char  alter_route;            /* token ring alter route status */
/* LEHb defect 44499 */
#ifdef TRL
	uchar   sri_field[18];      /* single route broadcast ri field  */
#elif  FDL
	uchar sri_field[FDL_ROUTING_LEN]; /* single route brdcst ri field */
#endif
#endif
	u_char  iors_rcvd;              /* i-frame or s-frame rcvd    */
/* LEHe */

};
#endif /* _h_LANSTACB */
