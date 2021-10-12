/* @(#)07	1.10  10/12/93 10:26:45 */
/*
 *   COMPONENT_NAME: SYSXPSLA
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/* PURPOSE:     Define the control blocks for the device driver.        */
/*                                                                      */
/*;bb 020790    Removed psbptr in g struct.                             */
/*;MJ 021690    gsw_dds structure pointer removed from gswcb structure  */
/*;MJ 021990    Removed structure 'intr' from gswcb structure.		*/
/*;bb 030590    Removed 'open' from gswcb structure.                    */
/*;MJ 032090    Removed 'diag_mode' from 'gswcb' structure.		*/
/*;bb 051290    Added 'parbuf' to 'gswcb' for parity buffer.            */
/*                                                                      */
/************************************************************************/

/* INCLUDE FILES:                                                       */
#include <sys/types.h>
#include "gswdef.h"
#include "gswdds.h"

#ifdef TRYWITHOUT
#define TESTEXCLUDE  1  /* To bypass gswio.h opnparms and ctlparms use */
			/* so FPGI fields not visible to vendors yet.  */
#endif TRYWITHOUT

#include <sys/gswio.h>


/*----------------------------------------------------------------------*/
/* Terminology:                                                         */
/*                                                                      */
/*  mgcb_ptr      -  the major anchor block pointer to 'struct gswcb'.  */
/*                   Each minor device has one. Space for these is      */
/*                   allocated at the first time through INIT for       */
/*                   all minor devices (for SOLO, there are 3 minors).  */
/*  ccw           -  channel status word, analogous to IBM/370 CCWs.    */
/*                   Contains fields indicating the type of IO command  */
/*                   and control flags.                                 */
/*  ccb           -  command control block. Contains command elements   */
/*                   (ce's) describing a collection of io operations    */
/*                   to be performed.                                   */
/*  ce            -  command element. Contains pointer to buffer area   */
/*                   and length of data transfer for a single io        */
/*                   command.                                           */
/*  sma           -  SelectMemoryArea. Whenever a read/write is done,   */
/*                   The 508x expects the operation to consist of       */
/*                   two parts: the first part is a sma control block   */
/*                   describing the io.                                 */
/*  g             -  local variable pointer to the struct gswcb for the */
/*                   current minor device being serviced.               */
/*  IQT           -  Interrupt Qualifier Table. See description below.  */
/*  qel           -  interrupt queue element. Consists of a header and  */
/*                   associated data for an interrupt.                  */
/*  rmiq          -  queue of dev's for pending RMIs.                   */
/*                   The queue is large enough to hold one RMI          */
/*                   per minor device plus one empty slot (e.g. 4 for   */
/*                   SOLO).                                             */
/*                   This queue is used to hold the minor number of the */
/*                   device that wants to issue an RMI but cannot at    */
/*                   this time because a SIO is in progress and must    */
/*                   complete (i.e., no more cc) before another         */
/*                   operation can be started.                          */
/*  pndq          -  queue of minor numbers for pending user SIOs.      */
/*                   The queue is large enough to hold one SIO          */
/*                   per minor device plus one empty slot (e.g. 4 for   */
/*                   SOLO). It is used when a SIO for a device (e.g.    */
/*                   port a) is issued while another SIO for another    */
/*                   device (graphics) is in process.                   */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*  cap           -  ptr to 'adapter to R2' communication area.         */
/*  rap           -  ptr to 'R2 to adapter' communication area.         */
/*  ccw_offset    - ptr to currently processing CCW                     */
/*  ce_offset     - ptr to currently processing CE                      */
/*  hdr_offset    - ptr to currently processing CCB                     */
/*  pa            - array of ptrs to pinned areas.                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*  FIELDS in 'g' STRUCT:                                               */
/*                                                                      */
/*  io_pend_cnt - incremented for a solicited SIO from the user.        */
/*                It is done in 'gswio' after a successful 'setup_sio'  */
/*                It should always be either 0 or 1, since the 2nd      */
/*                SIO is never started by the driver until the 1st one  */
/*                completes. It is NOT dependent upon the number of     */
/*                CEs needed for the complete operation.                */
/*  rmip        - holds the RMI data for a RMI.                         */
/*  rmidp       - the ptr to struct xmem associated with the RMI        */
/*  intp        - holds the SENSE data for a GEOP or the RMI data for   */
/*                a SMI until the associated i/o operation for these    */
/*                types of interrupts complete.                         */
/*  intdp       - the ptr to struct xmem associated with the GEOP/SMI   */
/*                i/o                                                   */
/*  xm          - struct usrxmem, which contains a ptr to the           */
/*                struct xmem's allocated by the driver for the i/o     */
/*                operations. 'gswio' sets up access and pinning of     */
/*                the user areas for chained operations.                */
/* struct oflag - flags associated with Opening the device              */
/* struct b     - flags associated with ccws (e.g., AE, DE)             */
/* struct c     - flags associated with i/o Completion                  */
/* struct f     - flags associated with lda and attaches (may be        */
/*                eliminated at a future time. Carryover from VRM).     */
/* dma_....     - all these fields are the parms used on the current    */
/*                dma 'dmaster' operation. Used for the 'dcomplete'.    */
/*                                                                      */
/*----------------------------------------------------------------------*/

struct ccb {
	ushort result;
	ushort dev_opt;
	ushort reserved;
	ushort rsvd2;
	uint   rsvd[4];
 };

#define CCB_ELM         (0x1000)    /* One or more command elements follow */
#define CCB_SYNCH       (0x2000)
#define CCB_INTERR      (0x4000)
#define CCB_INTCOMP     (0x8000)

struct com_elm {
	ushort  res1;
	ushort  linkwd;
	unsigned tranlen;
	caddr_t memaddr;
	long res3;
 };


typedef double WDALIGN;
 typedef  struct sma {
     ushort memid;                      /* memory area id               */
     ushort flags;                      /* control flags                */
     ulong  memofs;                     /* memory area offset           */
     ushort memcnt;                     /* memory area count            */
     ushort extra;                      /* align to 12 bytes            */
 } SMA;

 /*---------------------------------------------------------------------*/
 /* ccb to be used for both read and write.                             */
 /*---------------------------------------------------------------------*/

struct com_rw {
   struct ccb ccb;
   struct com_elm ce[MAXCE];
};

struct com_rwfp {                          /*FOR FPGI */
   struct ccb ccb;
   struct com_elm ce[3];
};
 
/* FOR FPGI CONNECTION REQUEST */
struct conn_req {
   short len;
   short cmd;
   int func;
   int req_size;
   int max_size;
   int target;
};
typedef struct com_rw RWCCB;
typedef struct com_rw WSCCB;
typedef struct com_rwfp RWCCBFP;          /* FOR FPGI */
typedef struct com_rwfp WSCCBFP;          /* FOR FPGI */

/*--------------------------------------------------------------*/
/* macro to fill in a ccw                                       */
/*--------------------------------------------------------------*/

#define FCCW(ccwstruct,command,flgs)           \
{       (ccwstruct).cmdcode = (command);       \
    (ccwstruct).flags   = (flgs);  }

/*--------------------------------------------------------------*/
/* macro to fill in a sma (SelectMemoryArea)                    */
/*--------------------------------------------------------------*/

#define FSMA(smastruct,id, ofs,cnt)           \
{   (smastruct).memid   = (ushort)id;         \
    (smastruct).memofs  = (ulong)(ofs);       \
    (smastruct).memcnt  = (ushort)cnt;      }

/*--------------------------------------------------------------*/
/* macro to fill in a ccb header                                */
/*--------------------------------------------------------------*/

#define FCCB(ccbstruct)                         \
	(ccbstruct).ccb.dev_opt = CCB_ELM +     \
                                  CCB_INTERR +  \
                                  CCB_INTCOMP + \
				  CCWLST


/*--------------------------------------------------------------*/
/* Macro to fill in a ce (CommandElement)                       */
/* Using 'res3' field for DMA location indicator.               */
/*--------------------------------------------------------------*/

#define FCEX(ccbstruct,i, res , len, addr, location)  \
{       (ccbstruct).ce[i].res1    = (ushort)(res );   \
	(ccbstruct).ce[i].tranlen = (unsigned)(len);  \
	(ccbstruct).ce[i].res3    = (long)location;   \
        (ccbstruct).ce[i].memaddr = (caddr_t)addr; }

/*--------------------------------------------------------------*/
/* Structure for user xmem values.                              */
/* Used when chaining io operations and to free pinned dp's     */
/* when operation is completed.                                 */
/*--------------------------------------------------------------*/
struct usrxmem  {
	int   cnt;              /* # entries                    */
	int   indx;             /* currnt one to process        */
	struct xmem *dp;        /* ptr to malloced array        */
};

/*--------------------------------------------------------------*/
/* Structure for unsolicited sense area.                        */
/* When an IO operation failed, sense data is retured to        */
/* indicate the error.                                          */
/* As sense area data is received, 'head' is the index to       */
/* the available location to put the data. 'tail' is the        */
/* index to the location from which to remove sense data.       */
/* Basically, enteries are put in 'head' and removed from       */
/* 'tail', using 'first-in-first-out' method.                   */
/*--------------------------------------------------------------*/
struct u_sen_hdr {
    ushort tail;                /* remove oldest sense from here*/
    ushort head;                /* put new sense at 'head'      */
};

union u_sen_data {              /* sense area field descriptions*/
    char data[24];
    struct {
	    unsigned cmd_rej       : 1;
	    unsigned int_req       : 1;
	    unsigned bus_out_chk   : 1;
	    unsigned equip_chk     : 1;
	    unsigned data_chk      : 1;
	    unsigned over_run      : 1;
	    unsigned disp_prog_run : 1;
	    unsigned rsvd1         : 1;
	    unsigned pick_det      : 1;
	    unsigned geop          : 1;
	    unsigned char_mode     : 1;
	    unsigned rsvd2         : 1;
	    unsigned hdw_err       : 1;
	    unsigned seg_pick      : 1;
	    unsigned gsdevi        : 1;
	    unsigned prog_err      : 1;
	    char sens[22];
    } sd;
};
union  rwccw  {
     ccw_t ccw;
     WDALIGN xx;                /* forces 'double' alignment    */
 };

/*--------------------------------------------------------------*/
/* Structure for the 4 io ccws.                                 */
/* For some io operations done the IOCTL, the user can          */
/* optionally stop the dlb (DisplayListBuffer) before doing the */
/* i/o and/or restart the dlb after the i/o is performed.       */
/* To avoid constantly malloc/free space for the ccws, the      */
/* space is allocated at open time. Pointers are used to        */
/* include or exclude the stop and/or start ccws.               */
/*--------------------------------------------------------------*/

 struct ccw4 {
     ccw_t stop;
     ccw_t rw_sma;
     ccw_t rw;
     ccw_t start;
 } ;
 
 struct ccw2 {                         /* FOR FPGI */
     ccw_t rw_sma;
     ccw_t rw;
 };

typedef union rwccw  RWCCW;
typedef struct gswcb GSWCB;

/*----------------------------------------------------------------------*/
/* The story of passing unsolicited interrupt information to the user:  */
/*                                                                      */
/* When an unsolicited interrupt occurs, information regarding the      */
/* interrupt plus associated data needs to be passed back to the        */
/* user, if the user has indicated to the driver that (s)he wants to    */
/* be notified. The K_ENABLE IOCTL call is the mechanism whereby the    */
/* user requests the driver to maintain a queue of interrupt queue      */
/* elements (qel's). The user receives these qels via K_WAIT or         */
/* K_POLL IOCTL calls. The user can use a signal handler to receive     */
/* control when the interrupt queue changes state from empty to         */
/* non-empty.                                                           */
/* Each of the various interrupts require different operations to be    */
/* done in order to get the associated data. For example, the data      */
/* associated with a lighted PF key (LPFK) is contained in the RMI      */
/* data from the 508x. A GEOP interrupt, however, may require the       */
/* driver to issue a i/o operation to read an area in 508x memory.      */
/* The qel contains a header describing the type of interrupt and       */
/* the data returned.                                                   */
/* Because of the variety of actions and data for each interrupt type,  */
/* multiple structures are needed to maintain information about         */
/* processing the interrupts. What follows is a description of those    */
/* structures and there inter-relationship.                             */
/*                                                                      */
/*                                                                      */
/* 'struct intr_table' (aka IQT) Interrupt Qualifier Table              */
/*                                                                      */
/* An IQT represents each interrupt that can appear on the interrupt    */
/* queue, and which will ultimately be passed to the user.              */
/* For some types of interrupts, only one IQT is necessary. For example,*/
/* there is only one type of LPFK interrupt. For other interrupt types, */
/* several IQTs may be necessary to represent each unique interrupt     */
/* of that type. For example, there may be several GEOP interrupts      */
/* ENABLEd, each one representing a different address in the DLB.       */
/* In our terminology, each is a separate qualifier.                    */
/* In FPGI mode, the iqt will not be necessary. Any information         */
/* regarding the interrupt (like reading data) will be done by the      */
/* driver and put directly in the user's area.                          */
/* That info is saved at OPEN time (by call to gopn_x, which calls      */
/* fpgibufin).                                                          */
/* At the current time, only two interrupt types, GEOP and GSMI,        */
/* require multiple IQTs.                                               */
/*                                                                      */
/*                                                                      */
/* 'struct qhdr' (aka q_hdr)                                            */
/*                                                                      */
/*  There is one q_hdr per interrupt type in 'enum input_type'.         */
/*  It contains a pointer to its IQT list and flags to indicate         */
/*  mode (ENABLEd) and status of signal notification.                   */
/*                                                                      */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*  g->hdr_ptr                                                          */
/*      |                                                               */
/*      |                                                               */
/*      |                                                               */
/*      |   q_hdr (one per type)          IQTs                          */
/*      |                                                               */
/*      |->  ----------------                                           */
/*          |  Gpick         |          -------------                   */
/*          |         intrp--|----->   |             |                  */
/*          |----------------|          -------------                   */
/*          |  Gank          |                                          */
/*          |                |                                          */
/*          |----------------|                                          */
/*          |  Gpfk          |                                          */
/*          |                |                                          */
/*          |----------------|                                          */
/*          |   .            |                                          */
/*          |   .            |                                          */
/*          |   .            |                                          */
/*          |----------------|                                          */
/*          |  Ggeop         |          -------------                   */
/*          |         intrp--|----->   |             |                  */
/*          |----------------|          -------------                   */
/*          |   .            |         |             |                  */
/*          |   .            |          -------------                   */
/*          |   .            |               .                          */
/*          |   .            |               .                          */
/*           ----------------                .                          */
/*                                      -------------                   */
/*                                     |             |                  */
/*                                      -------------                   */
/*                                                                      */
/*                                                                      */
/*----------------------------------------------------------------------*/

struct intr_table {
	struct {
		unsigned dirty_slot : 1;        /* slot in use          */
		unsigned last_slot  : 1;        /* last slot in chain   */
		unsigned mode       : 2;        /* EVENT, REQUEST       */
		unsigned reserved   : 4;
		char rsvd[3];
	} flags;
	int   type;
	int   qualifier;                /* used with smi and geop       */
	char *buf_adr;                  /* 5080 bufr adr to read        */
	int   data_len;                 /* len of data .                */
	int   slot_len;                 /* qel hdr+data size            */
	char *w_adr;                    /* 5080 bfr adr to write short 0*/
} ;
typedef struct intr_table IQT;

typedef struct qhdr {
    unsigned rvd     : 5;
    unsigned mode    : 2;
    unsigned en_sig  : 1;
    char rsvd[3];
    IQT *intrp;
    pid_t pid;
} q_hdr;

/*----------------------------------------------------------------------*/
/*; Explanation of QELs and the interrupt queue:                        */
/*;                                                                     */
/*;The use of queue elements (qel's) is somewhat complicated because    */
/*;queue elements are varying sizes, based on the interrupt.            */
/*;For this reason, one cannot allocate a fixed number of them, each    */
/*;having the same size. There would be too much wasted space, since    */
/*;the largest one has 512 bytes of data and the smallest has 0 bytes   */
/*;of data.                                                             */
/*;                                                                     */
/*;Throughout this discussion, note that the qels are in the AIX        */
/*;kernel data space and COPYOUT is used copy that info into the        */
/*;user's data space. All this qel manipulation is done within the      */
/*;kernel.                                                              */
/*;                                                                     */
/*;The technique used to control the allocation of qels is as follows:  */
/*;The HEAD pointer points to the next available location in the queue  */
/*;into which the interrupt information is put. When the user wants     */
/*;the data, TAIL points to the next qel to be removed from the queue.  */
/*;Basically, put the data into the HEAD and out the TAIL.              */
/*;                                                                     */
/*;The complexity begins with two factors:                              */
/*;     . what to do if there is insufficient space at the              */
/*;       bottom of the queue to create the current queue element       */
/*;     . how to wrap to top of queue                                   */
/*;                                                                     */
/*; When insufficient space is available at bottom of queue to create   */
/*; the current queue element, HEAD is set to the top of the queue      */
/*; and the leftover space on the bottom of the queue is unused.        */
/*; This unused space is marked by putting type=GALL into its type      */
/*; field. The name should have been more descriptive (like GUNUSED)    */
/*; but it wasnt. Anyway, in GFND_QEL there is a check for  sufficient  */
/*; space. If too small, type is set to GALL and HEAD is set to the top.*/
/*; And, when GGET_QEL searches for a qel to give to the user, it       */
/*; resets (sets type=GNULL) the qel whose type is GALL and sets TAIL   */
/*; to the top. This is necessary, since on the next cycle through      */
/*; the queue may have all different qel sizes.                         */
/*;                                                                     */
/*; The final item to remember is the relationship of HEAD and TAIL.    */
/*; Note that TAIL is not always less that HEAD. As pointers, TAIL      */
/*; may be pointing to the last qel and HEAD may be pointing to         */
/*; the next available slot, which is at the top of the queue.          */
/*; In this situation, HEAD < TAIL. At other times, HEAD can be         */
/*; greater than TAIL. For example, after the first qel is filled,      */
/*; HEAD points to the next slot. therefore, HEAD > TAIL.               */
/*; The only relationship that is always true is that when HEAD = TAIL, */
/*; the queue is empty.                                                 */
/*; GFND_QEL checks that the queue does not overlap itself with the     */
/*; use of END_PTR to measure the distance between HEAD and space       */
/*; space available from TAIL or bottom of queue.                       */
/*;                                                                     */
/*;                                                                     */
/*; 'struct intr_q'   Interrupt Queue Pointers                          */
/*;                                                                     */
/*; The fields in intr_q are pointers to QELs within the interrupt      */
/*; queue. These pointers indicate the next available space ('head'),   */
/*; the next qel to remove from the queue ('tail'), the beginning       */
/*; of the interrupt queue ('top') and the end of the interrupt queue   */
/*; ('bottom').                                                         */
/*; Each element consists of a header following by any data associated  */
/*; with that interrupt.                                                */
/*;                                                                     */
/*;---------------------------------------------------------------------*/

struct intr_q {
	q_qel *head;                   /* items put in at the head     */
	q_qel *tail;                   /* items removed at the tail    */
	q_qel *top;                    /* top of space                 */
	q_qel *bottom;                 /* bottom of space              */
};

/*---------------------------------------------------------------------*/
/*  The major anchor control block, 'struct gswcb'.                    */
/*                                                                     */
/*  'struct gswcb' contains all the fields and/or pointers             */
/*  necessary to determine the state of the device it represents.      */
/*                                                                     */
/* Note:                                                               */
/*      The 'struct intr' must be the first element of the structure   */
/*      so that when its address is passed to the interrupt handler,   */
/*      the interrupt handler has access to all the fields.            */
/*---------------------------------------------------------------------*/

struct gswcb {
    dev_t       devno;                  /*     minor number             */
    struct  {                           /*     flags                    */
	unsigned slp_pending    : 1;    /*       sleep in progress      */
	unsigned sig_pending    : 1;    /*       signal to be sent      */
	unsigned gsw_open       : 1;    /*       5080 is open           */
	unsigned q_chng_state   : 1;    /*       empty to non-empty     */
	unsigned wokeup         : 1;    /*       intr hdlr issued wakeup*/
	unsigned req_intr       : 1;    /*       rcvd a REQUEST intr    */
	unsigned link_sw        : 1;    /*       openx: collect linksw  */
	unsigned rsvd0          : 1;    /*       reserved               */
					/*                              */
	unsigned io_waiting     : 1;    /*       async_io used- wait    */
	unsigned wokehdr        : 1;    /*       intr hdlr issued wakeup*/
	unsigned rsvd           : 6;    /*                              */
	char rvd[2];
    } oflag;                            /*       flags used at open time*/
    struct  {                           /*     flags                    */
	unsigned not_to_ready   : 1;    /*                              */
	unsigned io_allowed     : 1;    /*                              */
	unsigned not_configured : 1;    /*       define not configured  */
	unsigned gsw_switched   : 1;    /*                              */
	unsigned rsvdi          : 4;    /*       reserved               */
	char rvd[3];
    } iflag;                            /*      flags used at ipl time  */
    char   *wsbufp;                     /*     static wsf area          */
    RWCCB *rw_ccbp;                     /*     static ccb area          */
    ccw_t *rw_ccwp;                     /*     static ccb area          */
    SMA   *rw_smap;                     /*     static sma area          */
    RWCCB *ws_ccbp;                     /*     static ccb area          */
    ccw_t *ws_ccwp;                     /*     static ccw area          */
    RWCCB *in_ccbp;                     /*     for read in intr hdlr    */
    ccw_t *in_ccwp;                     /*              "               */
    SMA   *in_srmap;                    /*              "               */
    SMA   *in_swmap;                    /*              "               */
    q_hdr *hdr_ptr;                     /*                              */
    char  *pathid;                      /*                              */
    char  *u_err_area;                  /*     uarea for sense of bad io*/
    struct intr_q q;                    /*     four ptrs for intr queue */
    pid_t  pid;                         /*     used with pidsig         */
    struct u_sen_hdr *sen_hdrp;         /*     ptr to sense area header */
    union u_sen_data *sen_datap;        /*     ptr to sense area        */
    struct {                            /*                              */
	    unsigned bad_data :1;       /*                              */
	    unsigned reserved :7;       /*                              */
	    char rsvd[3];               /*                              */
    } req_flags;                        /*                              */
    int    req_qualifier;               /*      REQUEST mode            */
    int    req_type;                    /*      REQUEST mode            */
    int    req_len;                     /*      REQUEST mode            */
    int    io_pend_cnt;                 /*      counts io's pending     */
    int    signal;                      /*      holds users signal value*/
    ushort op_res;                      /*      holds op_res from sio   */
    ushort uns_res;                     /*      holds uns_res from uns  */
    char   req_data[MAX_DATA+48];       /*                              */
    RWCCB *lcwccb_adr[2];               /*      lcw ccb adrs            */
    ccw_t *lcwccw_adr[2];               /*      lcw ccw adrs (user space*/
    int    lcwccw_cnt[2];               /*      # of user ccw's (K_LCW) */
    RWCCB *last_ccp;                    /*      last async ccb          */
    int    rw_io;                       /*      flip-flop for dual rw io*/
    int    ws_io;                       /*      flip-flop for dual ws io*/
    int    lc_io;                       /*      flip-flop for dual lcwio*/
    int    fp_io;                       /*      flip-flop for dual f io */
    RWCCBFP *fp_ccbp;                   /*      static ccb area         */
    ccw_t *fp_ccwp;                     /*      static ccb area         */
    SMA   *fp_smap;
    int    devmode;
    int    send_done;
    struct fpgi_send *fp_sendptr;
    ushort sid;                         /* sid                          */
    char   lda;                         /* lda                          */
    char   pad0;                        /* pad for alignment            */
    struct {                            /* bits ..CCW flag bits         */
	    unsigned    dc        :  1; /* data chaining                */
	    unsigned    cc        :  1; /* cmd  chaining                */
	    unsigned    sli       :  1; /* suppress len ind             */
	    unsigned    skip      :  1; /* skip                         */
	    unsigned    pci       :  1; /* pgm cntl intrpt              */
	    unsigned    ida       :  1; /* indirect data address        */
	    unsigned    suspend   :  1; /* suspend channel pgm          */
	    unsigned    rsvd1     :  1; /* reserved                     */
    } b;
    struct {                            /* completion control...        */
	    unsigned    ae_exp    :  1; /* adapter end expected         */
	    unsigned    de_exp    :  1; /* device  end expected         */
	    unsigned    rsvd1     :  1; /* reserved                     */
	    unsigned    rsvd2     :  1; /* reserved                     */
	    unsigned    rsvd3     :  1; /* reserved                     */
	    unsigned    rsvd4     :  1; /* reserved                     */
	    unsigned    rsvd5     :  1; /* reserved                     */
	    unsigned    rsvd6     :  1; /* reserved                     */
    } c;
    struct fpgi_in *fpgiinp;            /* ptr to user's inbnd struct   */
    int             num_pins;           /* number of pinned structures  */
    int             num_buf;            /* number of user inbound bfrs  */
    int             buf_len;            /* length of each user data bfr */
    int             fp_inbufs;          /* TRUE if user bfr available   */
    int             inbuf_indx;         /* indx to user's bfr           */
    struct in_buf_hdr **pbp;            /* ptr to list of user bfr ptrs */
    struct fpgiincb *fpcbp;             /* ptr to array of ccb,ce,ccws  */
    char           *rmidata;            /* ptr to rmi data              */
    struct in_buf_hdr *crnt_inbufp;     /* current inbnd bfr hdr in use */
    ushort          openmode;           /* OPEN mode (5080,FPGI,system) */
    int             noinbuf;            /* an FPGI field                */
    int             lock;               /* lock flag for LOCKL          */
    ulong           buid;               /* bus unit id (for intrpt parm)*/
    char           *name;               /* name array ptr               */
    int             mode;               /* OPEN mode parm               */
    int             open_channel;       /* OPEN channel parm            */
    int             dma_channel;        /* DMA channel                  */
    char           *dma_bus_addr;       /* DMA base bus addr            */
					/* VARS FOR D_COMPLETE CALL     */
    int             dma_location;       /* DMA location (user or kernel)*/
    int             dma_chanid;         /* DMA channel id               */
    int             dma_flags;          /* DMA flags                    */
    char           *dma_baddr;          /* DMA base addr                */
    int             dma_count;          /* DMA count                    */
    struct xmem     *dma_dp;            /* DMA xmem struct adr          */
    char            *dma_daddr;         /* DMA adapter addr             */
    int             *sleep_sio;         /* event list for sleep on SIO  */
    int             *sleep_eventq;      /* event list for sleep on evntq*/
    int             *sleep_open;        /* event list for sleep in OPEN */
    struct basic_ws *sfp;               /* struct field ptr for IOCTL   */
    struct sflpat   *sflp;              /* struct field ptr for IOCTL   */
    RWCCB           *cbp;               /* ccb ptr          for IOCTL   */
    ccw_t           *ccp;               /* ccw ptr          for IOCTL   */
    struct sma      *smp;               /* struct sma ptr   for IOCTL   */
    struct usrxmem   xm;                /* struct uf user xmem values   */
    char            *rmip;              /* ptr to area for rmi intrpts  */
    struct xmem     *rmidp;             /* xmem struct for rmi intrpts  */
    char            *intp;              /* ptr to area for geop/smi intr*/
    struct xmem     *intdp;             /* xmem struct for geop/smi intr*/
    ushort          *zerop;             /* ptr to 2 bytes of 0 for Gsmi */
    int              intio_type;        /* enum input_type for GEOP/SMI */
    struct ccb      *pndccbp;           /* ptr to ccb for pending io    */
    struct xmem     *pnddp;             /* ptr xmem struct - pending io */
    struct basic_ws *basesfp;           /* ptr to StructField area      */
    caddr_t          usrccwp;           /* ptr to copyin area-usr ccws  */
    int              usrccwlen;         /* len of usr ccws (byte cnt)   */
    label_t          parbuf;            /* parity buffer for setjmpx    */
};


     struct base    {                   /* 6 bytes of structured field  */
      ushort sflen;                     /*  common to all wsf's         */
      ushort t_c;                       /*    type and code             */
      ushort rsvd;                      /*                              */
     };                                 /*                              */
					/*                              */
					/*                              */
     struct sflpat {                    /* load line patterns           */
       struct base b;                   /*                              */
       char   rsvd2;                    /*                              */
       char   frst_lp;                  /*                              */
       short  lp[192];                  /*    up to 12-32 byte data     */
     };                                 /*                              */
     struct basic_ws {                  /* write structure for all      */
       struct base b;                   /* ws except line patterns      */
       ushort half1;
       ushort half2;
       ushort half3;
       ushort half4;
       ushort half5;
       ushort half6;
       ushort half7;
     };

/*----------------------------------------------------------------------*/
/* Trace id.                                                            */
/*----------------------------------------------------------------------*/

struct tr_id {
	unsigned chan_id : 5;           /* channel id                   */
	unsigned hook_id : 11;          /* hook    id                   */
};

/*----------------------------------------------------------------------*/
/* Struct stat_flags - global status flags                              */
/*----------------------------------------------------------------------*/

  struct msla_flags {
	unsigned        sol_in_prog     :1;     /* solicited cmd in prog*/
	unsigned        rmi_in_prog     :1;     /* unsol rmi in progress*/
	unsigned        retry_in_prog   :1;     /* cmd retry in progress*/
	unsigned        ipl_req         :1;     /* msla ipl request     */
	unsigned        ipl_in_prog     :1;     /* msla ipl in progress */
	unsigned        dma_enabled     :1;     /* dma channel enabled  */
	unsigned        start_in_prog   :1;     /* msla start in progres*/
	unsigned        stop_in_prog    :1;     /* msla stop  in progres*/
	unsigned        need_start      :1;     /* msla start required  */
	unsigned        need_stop       :1;     /* msla stop  required  */
	unsigned        fpsol_in_prog   :1;     /* FPGI in prog         */
	unsigned        intio_in_prog   :1;     /* intrpt hndlr io      */
	unsigned        rsvd0           :4;     /* reserved             */
  };



/*----------------------------------------------------------------------*/
/* Struct uns_sense  - unsolicited sense area ( 24 bytes)               */
/*----------------------------------------------------------------------*/

  struct uns_sense {
	unsigned        cmd_rej         :1;     /* command reject       */
	unsigned        int_req         :1;     /* intervention required*/
	unsigned        bo_chk          :1;     /* bus out check        */
	unsigned        equip_chk       :1;     /* equipment check      */
	unsigned        data_chk        :1;     /* data check           */
	unsigned        over_run        :1;     /* overrun              */
	unsigned        dp_run          :1;     /* display pgm running  */
	unsigned        rsvd07          :1;     /* reserved             */

	unsigned        pick_det        :1;     /* pick detect          */
	unsigned        eop             :1;     /* end order processing */
	unsigned        char_mode       :1;     /* character mode       */
	unsigned        rsvd13          :1;     /* reserved             */
	unsigned        hwd_err         :1;     /* hardware error       */
	unsigned        seg_pick        :1;     /* segment pick         */
	unsigned        gsdev1          :1;     /* see 5080 manual      */
	unsigned        pgm_err         :1;     /* program error        */

	ushort          hword3;                 /* used with errors     */

	unsigned        sf_err          :1;     /* struct field error   */
	unsigned        dev_timeout     :1;     /* device work timeout  */
	unsigned        inval_pg        :1;     /* invalid page         */
	unsigned        dp_loop         :1;     /* display gpm loop     */
	unsigned        TCF_overflow    :1;     /* TCF matrix overflow  */
	unsigned        stack_err       :1;     /* stack error          */
	unsigned        inv_mem_adr     :1;     /* invalid memory adr   */
	unsigned        rsvd47          :1;     /* reserved             */

	unsigned        mem_err         :1;     /* memory error         */
	unsigned        rsvd51          :6;     /* reserved             */
	unsigned        unit_specify    :1;     /* error in controller  */

	char            pn_MSB;                 /* pg # MSB             */
	char            pn_LSB;                 /* pg # LSB             */
	char            sn_MSB;                 /* seg name MSB         */
	char            sn_LSB;                 /* seg name LSB         */

	ushort          hword10;                /* see 5080 POP         */
	int             word12;                 /* see 5080 POP         */
	int             word16;                 /* see 5080 POP         */
	int             word20;                 /* see 5080 POP         */
  };


/*----------------------------------------------------------------------*/
/* Struct comm_area - common area in msla ram space                     */
/*----------------------------------------------------------------------*/

  struct mr_comm_area {                         /* msla to R2 com area  */
	unsigned        ipf             :8;     /* intrpt pending flag  */

			/* these next 2 bytes are like 370 status in CSW*/

	unsigned        attn            :1;     /* attention            */
	unsigned        stat_mod        :1;     /* status modifier      */
	unsigned        adapt_ready     :1;     /* adapter ready        */
	unsigned        busy            :1;     /* busy                 */
	unsigned        ae              :1;     /* adapter end          */
	unsigned        de              :1;     /* device end           */
	unsigned        uc              :1;     /* unit check           */
	unsigned        ue              :1;     /* unit exception       */

	unsigned        pci             :1;     /* pgm controlled intrpt*/
	unsigned        incorrect_len   :1;     /* incorrect length     */
	unsigned        pgm_chk         :1;     /* program check        */
	unsigned        prot_chk        :1;     /* protection check     */
	unsigned        cd_chk          :1;     /* channel data check   */
	unsigned        cc_chk          :1;     /* channel control check*/
	unsigned        ic_chk          :1;     /* interface control chk*/
	unsigned        chain_chk       :1;     /* chaining check       */

	char            icc;                    /* intrpt category code */
	char            ccc;                    /* cmd control code     */
	char            lda;                    /* logical dev adr      */
	ushort          rct;                    /* residual cnt frm msla*/

	uint            rsvd0;                  /* reserved             */

	union {
		char    ch[32];
		struct {
			ushort  vdahw1;         /* data                 */
			ushort  vdahw2;         /* data                 */
			ushort  vdahw3;         /* data                 */
		} vdahw;
		char    conf_lda[NumDevSupp];   /* config data          */
		struct {
#ifndef HYDRA
			char pad;               /* REMOVE FOR HYDRA ....*/
#endif  /* HYDRA */
			char nrtr_lda[NumDevSupp];
		} nrtr_data;

		/*char    nrtr_lda[NumDevSupp];    not-rdy-to-rdy data  */
		struct  uns_sense uns_sen;      /* unsolicited sense    */
	} vda;
  };


  struct rm_comm_area {                         /* R2 to msla com area  */
	char            scf;                    /* status clear flag    */
	char            smf;                    /* sense mode   flag    */
	char            fsb;                    /* status byte          */
	char            icc;                    /* intrpt category code */
	char            ccc;                    /* cmd control code     */
	char            lda;                    /* logical dev adr      */
	ushort          cnt;                    /* count field          */
	uint            adf;                    /* address field        */
	char            ch[NumDevSupp];         /* vda data             */
  };


/*----------------------------------------------------------------------*/
/* Struct stat1 - 1st status byte in comm area                          */
/*----------------------------------------------------------------------*/

  struct stat1 {                                /*                      */
	unsigned        attn            :1;     /* attention            */
	unsigned        stat_mod        :1;     /* status modifier      */
	unsigned        adapt_ready     :1;     /* adapter ready        */
	unsigned        busy            :1;     /* busy                 */
	unsigned        ae              :1;     /* adapter end          */
	unsigned        de              :1;     /* device end           */
	unsigned        uc              :1;     /* unit check           */
	unsigned        ue              :1;     /* unit exception       */
  };


/*----------------------------------------------------------------------*/
/* Struct stat2 - 2nd status byte in comm area                          */
/*----------------------------------------------------------------------*/

  struct stat2 {                                /*                      */
	unsigned        pci             :1;     /* pgm controlled intrpt*/
	unsigned        incorrect_len   :1;     /* incorrect length     */
	unsigned        pgm_chk         :1;     /* program check        */
	unsigned        prot_chk        :1;     /* protection check     */
	unsigned        cd_chk          :1;     /* channel data check   */
	unsigned        cc_chk          :1;     /* channel control check*/
	unsigned        ic_chk          :1;     /* interface control chk*/
	unsigned        chain_chk       :1;     /* chaining check       */
  };

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

  struct com_rfp {                      /* read (inbound) fpgi cbs      */
	struct ccb ccb;
	struct com_elm ce[4];
 };

/*----------------------------------------------------------------------*/
/* Struct ccw - s/370 ccw                                               */
/*----------------------------------------------------------------------*/
  struct ccw {
	unsigned cmdcode :8;            /* command code                 */
	char     flags;                 /* flag field                   */
	ushort   count;                 /* byte count                   */
	unsigned dataptr :32;           /* data address                 */
  };

  struct ccw3 {
	 struct ccw rmi;
	 struct ccw srma;
	 struct ccw rma;
 };

  struct fpgiincb                       /* ccb,ces,ccws for inbound i/o */
  {
	struct com_rfp rfp;             /* read ( inbound) fpgi cbs     */
	struct ccw3    ccw3;            /* ccws rmi, srma, rma          */
  };

/*----------------------------------------------------------------------*/
/* Structs for recording malloced areas and lengths at INIT time        */
/*         to be freed at TERM time.                                    */
/*         These areas are globally allocated and kept for life of dvr  */
/*----------------------------------------------------------------------*/
  struct malloc_areas {
	char *adr;
	int   len;
  };
  struct pinned_areas {
	int cnt;                        /* number of used 'ma' structs  */
	struct malloc_areas ma[5];
  };


struct fpgi_send {
    struct out_buf_hdr *bufp;
    int buf_len;
};

struct out_buf_hdr {
    unsigned rsvd0   :32;
    unsigned rsvd1   :32;
    unsigned busy    :1 ;
    unsigned rsvd2   : 31;
    int rsvd3;
};

struct fpgi_connection {
	int fn_code;
	int req_send_size;
	int max_recv_size;
	int target_name;
};

struct in_buf_hdr {
    unsigned rsvd0   :32;
    unsigned rsvd1   :32;
    unsigned busy    : 1;
    unsigned rsvd2   :31;
    int len;
};

struct fpgi_in {
    int numbuf;
    int buflen;
    struct in_buf_hdr **balp;
};



#ifdef TRYWITHOUT
/*----------------------------------------------------------------------*/
/* Re-declare 'union ctlparms' here (original in gswio.h) so that       */
/* compile of driver will succeed. The version in gswio.h does not      */
/* have the 'fpgi' structs. We do not want to reveal that info to       */
/* the user at this point in time.                                      */
/* Do same for 'opnparms'.                                              */
/*----------------------------------------------------------------------*/
union ctlparms {                        /* structs for IOCTL calls      */
	struct k_enable k_ena;
	struct k_disable k_dis;
	struct k_request k_req;
	struct g_writesf g_wsf;
	struct g_ld_blnk g_blnk;
	struct g_ld_line g_line;
	struct k_lcw k_lcw;
	struct fpgi_send f_send;
	struct fpgi_connection f_connect;
	char data[sizeof(struct devinfo)];
};

struct opnparms {                       /* struct for 'ext' on OPENX    */
    unsigned rsvd        : 2;           /* reserved. Set to 0.          */
    unsigned diag_mode   : 1;           /* used with diagnostics only   */
    unsigned mode        : 1;           /* reserved. Set to 0.          */
    unsigned change_adrs : 1;           /* reserved. Set to 0.          */
    unsigned start_msla  : 1;           /* used with 'start_msla' only  */
    unsigned stop_msla   : 1;           /* used with 'stop_msla'  only  */
    unsigned link_sw     : 1;           /* capture link switch intrpts  */
    char devmode;                       /* device mode.                 */
    char rvd[2];                        /* reserved. Set to 0.          */
    int signal;                         /* signal for intrpt q non-empty*/
    char *u_err_area;                   /* usr bfr adr for error data   */
    struct fpgi_in *fpgiinp;            /* reserved. Set to 0.          */
    struct fpgi_connection f_connect;   /* reserved. Set to 0.          */
    int sendmax;                        /* reserved. Set to 0.          */
};
#endif TRYWITHOUT

