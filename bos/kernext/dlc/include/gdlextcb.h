/* @(#)73       1.14.1.5  src/bos/kernext/dlc/include/gdlextcb.h, sysxdlcg, bos411, 9428A410j 5/12/94 13:42:00 */
#ifndef _h_GDLEXTCB
#define _h_GDLEXTCB
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: gdlextcb.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>

/*   All of the data areas and control blocks defined in this chapter
will be included in a single file, called gdlextcb.h. This include file
defines the structure types, not instances of structures.  The
application process or &LLC. process can create the instances of the
structures as needed.

   The <gdlextcb.h> include file is shown below:
*/


#define DLC_ERR -1
#define DLC_OK   0 
#define DLC_NULL 0

/*=========================================================================*/

   /* Read Extension Parameters */
					/*----------------------------*/
					/*        ioctl operations    */
					/*----------------------------*/
#define DLC_ENABLE_SAP		1    
#define DLC_DISABLE_SAP		2
#define DLC_START_LS		3
#define DLC_HALT_LS		4
#define DLC_TRACE		5
#define DLC_CONTACT		6
#define DLC_TEST		7
#define DLC_ALTER		8
#define DLC_QUERY_SAP		9
#define DLC_QUERY_LS		10
#define DLC_ENTER_LBUSY		11
#define DLC_EXIT_LBUSY		12
#define DLC_ENTER_SHOLD		13
#define DLC_EXIT_SHOLD		14
#define DLC_GET_EXCEP		15
#define DLC_ADD_GRP             16
#define DLC_ADD_FUNC_ADDR       17
#define DLC_DEL_FUNC_ADDR       18
#define DLC_DEL_GRP             19
/* #define IOCINFO               0xff01   see /usr/include/sys/ioctl.h */ 


/*=========================================================================*/

   /* Open Extension Parameters */
struct dlc_open_ext
{
	ulong_t maxsaps;      /* 1 (1 to 127) service access points */
	int   (*rcvi_fa)(); /* receive I-frame function address */
	int   (*rcvx_fa)(); /* receive XID function address */
	int   (*rcvd_fa)(); /* receive Datagram function address */
	int   (*rcvn_fa)(); /* receive Network data function address */
	int   (*excp_fa)(); /* exception handler function address */
};


/*=========================================================================*/

   /* Read and Write Extension Parameters */

/*** Read and Write Flags ***/
#define DLC_INFO     0x80000000  /* normal I-frame */
#define DLC_XIDD     0x40000000  /* XID data */
#define DLC_DGRM     0x20000000  /* datagram */
#define DLC_NETD     0x10000000  /* network data */
#define DLC_OFLO     0x00000002  /* receive overflow occurred */
#define DLC_RSPP     0x00000001  /* response pending */

#define DLC_FUNC_OK     0          /* receive I-frame accepted */
#define DLC_FUNC_BUSY  -1          /* receive I-frame failed, user busy */
                                   /*    user will reset local busy later */
#define DLC_FUNC_RETRY -2          /* function call failed, retry later */
   
struct dlc_io_ext
{
	ulong_t sap_corr;     /* Sap correlator */
	ulong_t ls_corr;      /* Link Station correlator */
	ulong_t flags;        /* flags */
	ulong_t dlh_len;      /* data link header length */
};


/*=========================================================================*/

   /* Ioctl Extension Parameters (Enable Sap) */

                                  /*** Common SAP Flags ***/
#define DLC_ESAP_NTWK     0x40000000  /* teleprocessing network type (LEASED) */
#define DLC_ESAP_LINK     0x20000000  /* teleprocessing link type (multi) */
#define DLC_ESAP_PHYC     0x10000000  /* physical network call */
#define DLC_ESAP_ANSW     0x08000000  /* teleprocessing auto call/answer */
#define DLC_ESAP_ADDR     0x04000000  /* local address/name indicator (ADDR) */

#define DLC_MAX_NAME	20	/* maximum size of the addr/name */
#define DLC_MAX_GSAPS	7	/* maximum number of group sap */
#define DLC_MAX_ADDR	8	/* maximum byte length of an addr */


struct dlc_esap_arg
{
	ulong_t gdlc_sap_corr;              /* GDLC Sap correlator - RETURNED */
	ulong_t user_sap_corr;              /* User's sap correlator */
	ulong_t len_func_addr_mask;         /* length of the field below it */
	uchar_t func_addr_mask[DLC_MAX_ADDR];/* Mask of the valid func addr */
	ulong_t len_grp_addr;               /* length of the field below it */
	uchar_t grp_addr[DLC_MAX_ADDR];     /* Addr of grp packets to be rcvd */
	ulong_t max_ls;                     /* Max number of ls per sap */
	ulong_t flags;                      /* Enable Sap flags */
	ulong_t len_laddr_name;             /* Len of the local name/addr */
	uchar_t laddr_name[DLC_MAX_NAME];   /* The local addr/name */
	uchar_t num_grp_saps;               /* Number of group saps */
	uchar_t grp_sap[DLC_MAX_GSAPS];     /* Group saps the sap will rsp to */
	uchar_t res1[3];                    /* reserved */
	uchar_t local_sap;                  /* Id of local sap */
};

/*=========================================================================*/

   /* Ioctl Extension Parameters (Start Link Station) */


#define DLC_TRCO      0x80000000  /* Trace Control On */
#define DLC_TRCL      0x40000000  /* Trace Control Long (full packet) */
#define DLC_SLS_STAT  0x20000000  /* Station type for SDLC (primary) */
#define DLC_SLS_NEGO  0x10000000  /* Negotiate Station Type for SDLC */
#define DLC_SLS_HOLD  0x08000000  /* Hold link on inactivity */
#define DLC_SLS_LSVC  0x04000000  /* Link Station Virtual Call */
#define DLC_SLS_ADDR  0x02000000  /* Address Indicator  (not discovery)*/

#define DLC_MAX_DIAG  16          /* the max string of chars in the diag name*/

struct dlc_sls_arg 
{
	ulong_t gdlc_ls_corr;           /* GDLC User link station correlator */
	uchar_t ls_diag[DLC_MAX_DIAG];  /* the char name of the ls */
	ulong_t gdlc_sap_corr;          /* GDLC sap correlator */ 
	ulong_t user_ls_corr;           /* User's sap correlator */ 
	ulong_t flags;                  /* Start Link Station flags */
	ulong_t trace_chan;             /* Trace Channel (rc of trcstart) */
	ulong_t len_raddr_name;         /* Len of the remote name/addr */
	uchar_t raddr_name[DLC_MAX_NAME];/* The Remote addr/name */
	ulong_t maxif;                  /* Maximum number of bytes in a frame */
	ulong_t rcv_wind;               /* Maximum size of the rcv window */
	ulong_t xmit_wind;              /* Maximum size of the xmit window */
	uchar_t rsap;                   /* Remote SAP value */
	uchar_t rsap_low;               /* Remote SAP low range value */
	uchar_t rsap_high;              /* Remote SAP high range value */
	uchar_t res1;                   /* Reserved */
	ulong_t max_repoll;             /* Maximum Repoll count */
	ulong_t repoll_time;            /* Repoll timeout value */
	ulong_t ack_time;               /* Time to delay trans of an ack */
	ulong_t inact_time;             /* Time before inactivity times out */
	ulong_t force_time;             /* Time before a forced disconnect */
};

/*=========================================================================*/

   /* Ioctl Extension Parameters (Disable Sap Parameters) */
   /* Ioctl Extension Parameters (Halt Link Station Parameters) */
   /* Ioctl Extension Parameters (Enter Local Busy) */
   /* Ioctl Extension Parameters (Exit Local Busy) */
   /* Ioctl Extension Parameters (Enter Short Hold) */
   /* Ioctl Extension Parameters (Exit Short Hold) */
   /* Ioctl Extension Parameters (Contact Remote Parameters) */
   /* Ioctl Extension Parameters (Test Link Parameters) */

struct dlc_corr_arg
{
	ulong_t gdlc_sap_corr;          /* GDLC sap correlator */
	ulong_t gdlc_ls_corr;           /* GDLC ls correlator */
};

/*=========================================================================*/

   /* Ioctl Extension Parameters (Alter Link Parameters) */


#define DLC_ALT_RTO     0x80000000   /* Alter Repoll Timeout */
#define DLC_ALT_AKT     0x40000000   /* Alter Acknowledge Timeout */
#define DLC_ALT_ITO     0x20000000   /* Alter Inactivity Timeout */
#define DLC_ALT_FHT     0x10000000   /* Alter Force Halt Timeout */
#define DLC_ALT_MIF     0x08000000   /* Alter Maximum I-Frame Size */
#define DLC_ALT_XWIN    0x04000000   /* Alter Tranxmit Window Size */
#define DLC_ALT_MXR	0x02000000   /* Alter Maximum Repoll Count */
#define DLC_ALT_RTE     0x01000000   /* Alter Routing */
#define DLC_ALT_SM1     0x00800000   /* Alter Mode (SDLC) bit 1 (Primary) */
#define DLC_ALT_SM2     0x00400000   /* Alter Mode (SDLC) bit 2 (Secondary) */
#define DLC_ALT_IT1     0x00200000   /* Alter Inactivity bit 1 (Notify) */
#define DLC_ALT_IT2     0x00100000   /* Alter Inactivity bit 2 (Halt) */

#define DLC_MAX_ROUT    20           /* Maximum Size of Routing Info */

#define DLC_MSS_RES     0x00040000   /* Mode Set Secondary */
#define DLC_MSSF_RES    0x00020000   /* Mode Set Secondary Failed */
#define DLC_MSP_RES     0x00010000   /* Mode Set Primary */
#define DLC_MSPF_RES    0x00008000   /* Mode Set Primary Failed */

struct dlc_alter_arg
{
	ulong_t gdlc_sap_corr;          /* GDLC sap correlator */
	ulong_t gdlc_ls_corr;           /* GDLC ls correlator */
	ulong_t flags;                  /* Alter Flags */
	ulong_t repoll_time;            /* New Repoll Timeout */
	ulong_t ack_time;               /* New Acknowledge Timeout */
	ulong_t inact_time;             /* New Inactivity Timeout */
	ulong_t force_time;             /* New Force Timeout */
	ulong_t maxif;                  /* New Maximum I Frame Size */
	ulong_t xmit_wind;              /* New Transmit Value */
	ulong_t max_repoll;		/* New Max Repoll Value */
	ulong_t routing_len;            /* Routing Length */
	uchar_t routing[DLC_MAX_ROUT];  /* New Routing Data */
	ulong_t result_flags;           /* Returned flags */
};

struct dlc_add_grp
{
	ulong_t gdlc_sap_corr;          /* GDLC sap correlator */
	ulong_t grp_addr_len;           /* Group address length */
	uchar_t grp_addr[DLC_MAX_ADDR]; /* Group address to be added */
};

struct dlc_func_addr
{
	ulong_t gdlc_sap_corr;          /* GDLC sap correlator */
	ulong_t len_func_addr_mask;     /* length of functional address mask */
	uchar_t func_addr_mask[DLC_MAX_ADDR]; /* functional address mask     */
};

/*=========================================================================*/

   /* Generic Trace entry definitions */

                            /* generic trace data link protocols */
#define DLC_DL_SDLC    0      /* SDLC */
#define DLC_DL_HDLC    1      /* HDLC */
#define DLC_DL_BSC     2      /* Bisync */
#define DLC_DL_ASC     3      /* Async */
#define DLC_DL_PCNET   4      /* PC Network */
#define DLC_DL_ETHER   5      /* Ethernet */
#define DLC_DL_802_3   6      /* IEEE 802.3 */
#define DLC_DL_TOKEN   7      /* Token Ring */
#define DLC_DL_QLLC    8      /* X.25 Qualified DLC */
#define DLC_DL_FDDI    9      /* Fiber Distributed Data Interface */

#define DLC_DL_DATA_ONLY 15   /* extended data link trace data */

                            /* generic trace physical link protocols */
#define DLC_PL_RS232   0      /* EIA RS232C Telecommunications */
#define DLC_PL_RS336   1      /* EIA RS336 Auto Dial */
#define DLC_PL_X21     2      /* CCITT X.21 Data Network */
#define DLC_PL_PCNET   3      /* PC Network Broadband */
#define DLC_PL_ETHER   4      /* Standard Baseband Ethernet */
#define DLC_PL_SMART   5      /* Smart Modem Auto Dial */
#define DLC_PL_802_3   6      /* IEEE 802.3 Baseband Ethernet */
#define DLC_PL_TBUS    7      /* IEEE 802.4 Token Bus */
#define DLC_PL_TRING   8      /* IEEE 802.5 Token Ring */
#define DLC_PL_X25     9      /* X.25 Packet Network   */
#define DLC_PL_EIA422 10      /* EIA-422 Telecommunications */
#define DLC_PL_V35    11      /* CCITT V 35 Telecommunications */
#define DLC_PL_V25BIS 12      /* V 25 Autodial                 */
#define DLC_PL_FDDI   13      /* FDDI                          */

                            /* generic trace timeout types */
#define DLC_TO_SLOW_POLL  1     /* Slow Station Poll */
#define DLC_TO_IDLE_POLL  2     /* Idle Station Poll */
#define DLC_TO_ABORT      3     /* Link Station Aborted */
#define DLC_TO_INACT      4     /* Link Station receive Inactivity */
#define DLC_TO_FAILSAFE   5     /* Command Failsafe */
#define DLC_TO_REPOLL_T1  6     /* Command Repoll */
#define DLC_TO_ACK_T2     7     /* I-Frame Acknowledgment */

/*=========================================================================*/

   /* Ioctl Extension Parameters (Trace Link Parameters) */


/* trace flag values are defined in start_ls flags (DLC_TRCO, DLC_TRCL) */

struct dlc_trace_arg 
{
	ulong_t gdlc_sap_corr;          /* GDLC sap correlator */
	ulong_t gdlc_ls_corr;           /* GDLC ls correlator */
	ulong_t trace_chan;             /* Trace Channel (rc of trcstart) */
	ulong_t flags;                  /* Trace Flags */
};

/*=========================================================================*/

   /* Ioctl Extension Parameters (Query Sap Parameters) */


#define DLC_OPENING    1       /* the sap or ls is in the process of opening*/
#define DLC_OPENED     2       /* the sap or ls has been opened */
#define DLC_CLOSING    3       /* the sap or ls is in the process of closing */
#define DLC_INACTIVE   4       /* the ls is in an inactive state at present */

struct dlc_qsap_arg
{
	ulong_t gdlc_sap_corr;	/* GDLC sap correlator */
	ulong_t user_sap_corr;	/* user sap correlator - RETURNED */
	ulong_t sap_state;	/* state of the sap, - RETURNED */
	uchar_t dev[DLC_MAX_DIAG]; /* device handler's dev name - RETURNED */
	ulong_t devdd_len;	/* device driver dependent data byte length */
};

/*=========================================================================*/

   /* Ioctl Extension Parameters (Query Link Parameters) */


#define DLC_CALLING        0x80000000    /* the ls is calling */
#define DLC_LISTENING      0x40000000    /* the ls is listening */
#define DLC_CONTACTED      0x20000000    /* the connection is up and running */
#define DLC_LOCAL_BUSY     0x10000000    /* the local ls is busy right now */
#define DLC_REMOTE_BUSY    0x08000000    /* the remote ls is busy right now */

struct dlc_ls_counters 
{
	ulong_t test_cmds_sent;    /* number of test commands sent */
	ulong_t test_cmds_fail;    /* number of test commands failed */
	ulong_t test_cmds_rec;     /* number of test commands received */
	ulong_t data_pkt_sent;     /* number of sequenced data packets sent */
	ulong_t data_pkt_resent;   /* number of sequenced data packets resent */
	ulong_t max_cont_resent;   /* maximum number of contiguous resendings */
	ulong_t data_pkt_rec;      /* data packets received */
	ulong_t inv_pkt_rec;       /* number of invalid packets received */
	ulong_t adp_rec_err;       /* number of data detected receive errors */
	ulong_t adp_send_err;      /* number of data_detected transmit errors */
	ulong_t rec_inact_to;      /* number of received inactivity timeouts */
	ulong_t cmd_polls_sent;    /* number of command polls sent */
	ulong_t cmd_repolls_sent;  /* number of command repolls sent */
	ulong_t cmd_cont_repolls;  /* max number of continuous repolls sent */
};

struct dlc_qls_arg
{
	ulong_t gdlc_sap_corr;         /* GDLC sap correlator */
	ulong_t gdlc_ls_corr;          /* GDLC ls correlator */
	ulong_t user_sap_corr;         /* user's sap corr - RETURNED */
	ulong_t user_ls_corr;          /* user's ls corr - RETURNED */
	uchar_t ls_diag[DLC_MAX_DIAG]; /* the char name of the ls */
	ulong_t ls_state;              /* current ls state */
	ulong_t ls_sub_state;          /* further clarification of state */
	struct dlc_ls_counters counters;
	ulong_t protodd_len;	       /* protocol dependent data byte length */
};

/*=========================================================================*/

   /* Ioctl Extension Parameters (Get Exception Parameters) */


/* result indicators */

#define DLC_TEST_RES      0x08000000   /* a test cmd completion */
#define DLC_SAPE_RES      0x04000000   /* an enable sap completion */
#define DLC_SAPD_RES      0x02000000   /* a disable sap completion */
#define DLC_STAS_RES      0x01000000   /* a start link station completion */
#define DLC_STAH_RES      0x00800000   /* a halt link station completion */ 
#define DLC_DIAL_RES      0x00400000   /* manually dial the phone now */
#define DLC_IWOT_RES      0x00200000   /* inactivity without termination */
#define DLC_IEND_RES      0x00100000   /* the inactivity has ended */
#define DLC_CONT_RES      0x00080000   /* the station is now contacted */
#define DLC_RADD_RES      0x00004000   /* the remote addr has changed */

/* result codes */

#define DLC_SUCCESS        0         /* the result indicated was successful */
#define DLC_PROT_ERR    -906         /* protocol error */
#define DLC_BAD_DATA    -908         /* a bad data compare on a TEST */
#define DLC_NO_RBUF     -910         /* no remote buffering on test */
#define DLC_RDISC       -912         /* remote initiated discontact */
#define DLC_DISC_TO     -914         /* discontact abort timeout */
#define DLC_INACT_TO    -916         /* inactivity timeout */
#define DLC_MSESS_RE    -918         /* mid session reset */
#define DLC_NO_FIND     -920         /* cannot find the remote name */
#define DLC_INV_RNAME   -922         /* invalid remote name */
#define DLC_SESS_LIM    -924         /* session limit exceeded */
#define DLC_LST_IN_PRGS -926         /* listen already in progress */
#define DLC_LS_NT_COND  -928         /* ls unusual network condition */
#define DLC_LS_ROUT     -930         /* link station resource outage */
#define DLC_RBUSY       -932         /* remote station found, but busy */
#define DLC_REMOTE_CONN -936         /* specified remote is already connected */
#define DLC_NAME_IN_USE -901         /* local name already in use */
#define DLC_INV_LNAME   -903         /* invalid local name */
#define DLC_SAP_NT_COND -905         /* sap network unusual network condition */
#define DLC_SAP_ROUT    -907         /* sap resource outage */
#define DLC_USR_INTRF   -909         /* user interface error */
#define DLC_ERR_CODE    -911         /* error in the code has been detected */
#define DLC_SYS_ERR     -913         /* system error */

#define DLC_MAX_EXT       48         /* max size of the result extension field*/

struct dlc_getx_arg 
{
	ulong_t user_sap_corr;         /* user sap correlator - RETURNED */
	ulong_t user_ls_corr;          /* user ls correlator - RETURNED */
	ulong_t result_ind;            /* the type of excep */
	int     result_code;           /* the manner the excep */
	uchar_t result_ext[DLC_MAX_EXT]; /* exception specific extension */
};

/*=========================================================================*/

   /* Ioctl Extension Parameters (Get Exception Result Extensions) */
   /* Ioctl Extension Parameters (SAPE) */


struct dlc_sape_res
{
	ulong_t max_net_send;            /* maximum write network data length */
	ulong_t lport_addr_len;          /* local port network address length */
	uchar_t lport_addr[DLC_MAX_ADDR];/* the local port address */
};

/*=========================================================================*/

   /* Ioctl Extension Parameters (STAS) */

struct dlc_stas_res
{
	ulong_t maxif;               /* max size of the data sent on a write */
	ulong_t rport_addr_len;      /* remote port network address length */
	uchar_t rport_addr[DLC_MAX_ADDR]; /* remote port addr */
	ulong_t rname_len;           /* remote network name length */
	uchar_t rname[DLC_MAX_NAME]; /* remote network name */
	uchar_t res[3];              /* reserved  */
	uchar_t rsap;                /* remote sap */
	ulong_t max_data_off;        /* the maximum data offsets for sends */
};

/*=========================================================================*/

   /* Ioctl Extension Parameters (STAH) */

struct dlc_stah_res
{
	ulong_t conf_ls_corr;          /* conflicting link station corr */
};

/*=========================================================================*/

   /* Ioctl Extension Parameters (RADD, LSST, SAPS) */

struct dlc_radd_res
{
	ulong_t rname_len;             /* remote network name/addr length */
	uchar_t rname[DLC_MAX_NAME];   /* remote network name/addr */
};
  
/*
 *     DLC Trace Performance Hook definitions
*/
#define DLC_TRACE_WAITB         0x0100    /* Begin Wait Call               */
#define DLC_TRACE_WAITE         0x0200    /* End Wait Call                 */
#define DLC_TRACE_RCVBB         0x0300    /* Get Receive Buffer Begin      */
#define DLC_TRACE_RCVBE         0x0400    /* Get Receive Buffer End        */
#define DLC_TRACE_HASHB         0x0500    /* Start HASH Function           */
#define DLC_TRACE_HASHE         0x0600    /* End   HASH Function           */
#define DLC_TRACE_GTXBB         0x0700    /* Get Xmit Buffer from Ring Q   */
#define DLC_TRACE_GTXBE         0x0800    /* End get of Buff from Ring Q   */
#define DLC_TRACE_RDIDA         0x0900    /* Receive Network Data          */
#define DLC_TRACE_SNDIF         0x0A00    /* Send I-Frame Data             */
#define DLC_TRACE_XMITD         0x0B00    /* Put Data in Transmit Ring Q   */
#define DLC_TRACE_XMITX         0x0C00    /* Put XID  in Transmit Ring Q   */
#define DLC_TRACE_T1TO          0x0D00    /* Repoll (T1) Timeout           */
#define DLC_TRACE_T2TO          0x0E00    /* Acknowledge (T2) Timeout      */
#define DLC_TRACE_T3TO          0x0F00    /* Inactivity (T3)  Timeout      */
#define DLC_TRACE_STDH          0x1000    /* Start Device Handler          */
#define DLC_TRACE_RCVF          0x1100    /* Rcv Discovery Find Command    */
#define DLC_TRACE_RSLV          0x1200    /* Rcv Resolve Command           */
#define DLC_TRACE_OPLK          0x1300    /* Open Physical Link           */
#define DLC_TRACE_DHST          0x1400    /* Device Handler Started        */
#define DLC_TRACE_SNOIF         0x1500    /* Send Non I-Frame Data         */
#define DLC_TRACE_SNDDG         0x1600    /* Send Datagram Data            */
#define DLC_TRACE_SNDND         0x1700    /* Send Network Data             */
#define DLC_TRACE_T3ABORT       0x1800    /* T3 Abort Timer                */
/*
 *     DLC Trace DLC Types Definitions
*/
#define DLC_SDLC                0x0000    /* SDLC                   */
#define DLC_ETHERNET            0x0005    /* Standard Ethernet      */
#define DLC_IEEE_802_3          0x0006    /* IEEE 802.3             */
#define DLC_TOKEN_RING          0x0007    /* Token Ring             */
#define DLC_QLLC                0x0008    /* X.25 QLLC              */
#define DLC_FDDI                0x0009    /* FDDI                   */
/*
 *     DLC Trace Monitor Definitions
*/
#define DLC_TRACE_WRTC          0x01      /* Write Command          */
#define DLC_TRACE_RNONI         0x02      /* Receive Non I-Frame Cmd */
#define DLC_TRACE_RCVI          0x03      /* Receive I-Frame         */
#define DLC_TRACE_ISND          0x04      /* Input Send Command      */
#define DLC_TRACE_SNDC          0x05      /* Send Command            */
#define DLC_TRACE_TIMER         0x06      /* Timer                   */
#define DLC_TRACE_RNETD         0x07      /* Receive Network Data    */
/*=========================================================================*/
#endif /* _h_GDLEXTCB */
