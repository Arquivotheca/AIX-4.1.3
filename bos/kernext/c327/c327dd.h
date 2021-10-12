/* @(#)28  1.37 src/bos/kernext/c327/c327dd.h, sysxc327, bos411, 9430C411a 7/27/94 09:31:32 */
#ifndef _H_C327DD
#define _H_C327DD

/*
 * COMPONENT_NAME: (SYSXC327) c327 device driver header file
 *
 * FUNCTIONS:    N/A
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*****************************************************************************/
/* declarations common to c327dd, cut, diag, tca, & dftns                    */
/*****************************************************************************/

#include <sys/uio.h>

#ifdef _POWER_MP
#include <sys/lock_def.h>
#define DISABLE_INTERRUPTS(sil) sil=disable_lock(c327_intr_level, \
						(&c327_intr_lock))
#define RESTORE_INTERRUPTS(sil) unlock_enable(sil, (&c327_intr_lock))
#else
#include <sys/lockl.h>
#define DISABLE_INTERRUPTS(sil) sil=i_disable(c327_intr_level)
#define RESTORE_INTERRUPTS(sil) i_enable(sil)
#endif

#define boolean unsigned char
extern int c327_intr_level;

/*****************************************************************************/
/* define PIO utilities                                                      */
/*****************************************************************************/

typedef enum 
{
   GETCIO,                   /* read a character from a register            */
   GETCMEM,                  /* read a character from adapter memory        */
   GETCMEML,                 /* read several characters from adapter memory */
   PUTCIO,                   /* write a character to a register             */
   PUTCMEM,                  /* write a character to adapter memory         */
   PUTCIOCC,                 /* write a character to a POS register         */
   PUTCMEML,                 /* write several characters to adapter memory  */
   BZEROMEM                  /* zero adapter memory                         */
} c327io_t;

/*
** structure to track the state of each of (up to) four adapters plus dummy
*/
typedef enum {
    DDS_NOT_EXIST,       /* there's no card                               */
    DDS_AVAILABLE,       /* can be opened in any of the following modes   */
    DDS_OPENED_DIAG,     /* currently open in diagnostic mode             */
    DDS_OPENED_CUT,      /* currently open in CUT mode                    */
    DDS_OPENED_CUTFT,    /* currently open in CUT file transfer mode      */
    DDS_OPENED_DFTNS     /* currently open in dft non-sna mode            */
} DDS_STATE;

#define GETC_BUSIO(a1,a2)     c327_getc (GETCIO,   a1, (uint)(a2))
#define GETC_BUSMEM(a1,a2)    c327_getc (GETCMEM,  a1, (uint)(a2))
#define PUTC_BUSIO(a1,a2,a3)  c327_putc (PUTCIO,   a1, (uint)(a2), (uchar)(a3))
#define PUTC_BUSMEM(a1,a2,a3) c327_putc (PUTCMEM,  a1, (uint)(a2), (uchar)(a3))
#define PUTC_IOCC(a1,a2,a3)   c327_putc (PUTCIOCC, a1, (uint)(a2), (uchar)(a3))
#define GETC_BUSMEML(a1,a2,a3,a4) c327_ios  (GETCMEML, a1, a2, a3, a4)
#define PUTC_BUSMEML(a1,a2,a3,a4) c327_ios  (PUTCMEML, a1, a2, a3, a4)
#define C327_BZERO(a1,a2) c327_ios(BZEROMEM, a1, (int)0, (char *)0, (int)(a2))

/*****************************************************************************/
/* handle two independent types of tracing with one set of routines          */
/*****************************************************************************/

#define C327TRACE1(a1)       c327SaveTrace(a1, 0, 0, 0, 0)
#define C327TRACE2(a1,a2)    c327SaveTrace(a1, (int)a2, 0, 0, 0)
#define C327TRACE3(a1,a2,a3) c327SaveTrace(a1, (int)a2, (int)a3, 0, 0)

#define C327TRACE4(a1,a2,a3,a4) \
        c327SaveTrace(a1, (int)a2, (int)a3, (int)a4, 0)

#define C327TRACE5(a1,a2,a3,a4,a5) \
        c327SaveTrace(a1, (int)a2, (int)a3, (int)a4, (int)a5)

#define C327UNPTRACE1(a1)       c327SaveTrace(a1, 0, 0, 0, 0)
#define C327UNPTRACE2(a1,a2)    c327SaveTrace(a1, (int)a2, 0, 0, 0)
#define C327UNPTRACE3(a1,a2,a3) c327SaveTrace(a1, (int)a2, (int)a3, 0, 0)

#define C327UNPTRACE4(a1,a2,a3,a4) \
        c327SaveTrace(a1, (int)a2, (int)a3, (int)a4, 0)

#define C327UNPTRACE5(a1,a2,a3,a4,a5) \
        c327SaveTrace(a1, (int)a2, (int)a3, (int)a4, (int)a5)

#ifdef DEBUG
#define C327PERF(a1) \
        DDHKWD1 ( HKWD_DD_C327DD, DD_PERF_HOOK, 0, (int)a1 )
#else
#define C327PERF(a1) \
     /* DDHKWD1 ( HKWD_DD_C327DD, DD_PERF_HOOK, 0, (int)a1 ) */
#endif

#define MAX_SESSIONS       8      /* maximum netid table entries for 1 card */
#define MAX_ADAPTERS       4      /* Minor # can range from 0 - 7 but only  */
                                  /* 4 adapters can be configured at any    */
                                  /* given time                             */

#define BAD_ADAPTER        TRUE
#define GOOD_ADAPTER       FALSE
#define MAX_MINORS         8      /* Maximum value for minor #              */
#define TIMR_QUE_SIZE MAX_ADAPTERS   /* how many open timeouts can be queued */
#define INTR_QUE_SIZE 10*MAX_ADAPTERS /* how many interrupts can be queued  */

#define C327_TIMEOUT_INTERVAL HZ/8   /* interval between calls to dftnsTimer in
                                      timer ticks (used in calls to timeout) */
#define C327_TIMER_INTERVAL 125   /* interval in milliseconds */

/*****************************************************************************/
/* return codes passed from dftnsXxxxx commands */
/*****************************************************************************/
#define RC_OK_INTR_PEND   0x0000 /* completed successfully -- int pending */
#define RC_OK_NONE_PEND   0x0100 /* completed successfully -- no int pend */
#define RC_OPEN_TIMEOUT   0x8200 /* open time out -- control unit down    */
#define RC_ADAPTER_FAIL   0x8300 /* adapter not functioning or not presnt */
#define RC_NON_SUPP_CU    0x8306 /* non-supported control unit            */
#define RC_HOST_CONTEN    0x8307 /* cmd rejected due to host contention   */
#define RC_NOT_OPEN       0x8308 /* adapter port not opened               */
#define RC_OPEN_BUSY      0x830B /* open in progress -- try later         */
#define RC_NO_SESS_AVAIL  0x8601 /* available session not found           */
#define RC_INVAL_PARMS    0x8800 /* invalid parameters passed             */
#define RC_INVAL_NETID    0x8802 /* invalid network id parameter passed   */
#define RC_CMD_NOT_VALID  0x8804 /* command is not valid at this time     */

/* status codes passed to device head interrupt handler */
#define OR_NO_ERROR       0x0000      /* no error               */
#define OR_REC_DATA_AVAIL 0x8100      /* receive data available */
#define OR_LOCK           0x0900      /* CU/Device locked       */
#define OR_UNLOCK         0x0A00      /* CU/Device unlocked     */
#define OR_CHK_STAT_MSK   0x2000      /* Check Status Sent Mask */
#define OR_CU_DOWN        0x8905      /* control unit down      */
#define OR_RESTART_DONE   0x8906      /* wcus redy received after restart */
#define OR_WRITE_DISCARD  0x8907      /* write command was thrown away */
#define OR_SNA_INFO       0x10000     /* SNA WCUS 40 or 41      */
#define OR_SOFT_DSC       0x20000     /* Non-fatal disconnect   */
#define OR_SNA_RUSIZE     0x40000     /* broadcast inbound size to app.*/

/* or'ed with cu_error to indicate error class */
#define BROADCAST_MSK     0x8000      /* Broadcast Bit Mask                  */
#define MC_ERROR_MSK      0x1000      /* Machine Check Mask (Broadcast)      */
#define PC_ERROR_MSK      0x2000      /* Program Check Mask (one session)    */
#define CC_ERROR_MSK      0x4000      /* Communication Check Msk (Broadcast) */

/*****************************************************************************/
/* parameters passed between dftns package and device head */
/*****************************************************************************/
#define SESS_TYPE_TERMINAL       0 /* terminal session                       */
#define SESS_TYPE_PRINTER        1 /* printer session                        */
#define SEND_STAT_ACK            0 /* prev rec'd data/cmd ok, make response  */
#define SEND_STAT_OKDATA         1 /* valid read rec'd, write will follow    */
#define SEND_STAT_KBD_RESET      2 /* 3270 reset key pressed                 */
#define SEND_STAT_PRINT_COMPLETE 3 /* print complete                         */
#define SEND_STAT_DEV_NOT_SEL  107 /* cmd rec'd for device not selected      */
#define SEND_STAT_BUFFER_ERR   109 /* bad buf order or invalid buf adr rec'd */
#define SEND_STAT_BAD_CMD      110 /* non-3270 cmd rec'd                     */
#define SEND_STAT_UNSUPP_CMD   111 /* unsupported 3270 cmd rec'd             */

/*
** Everything you ever wanted to know about the 
** dd read, write and auto ack data buffer
*/

/*
** This number choosen to handle screen size up thru model 5.
** Whip also defaults to this. 
*/

#define LO_BUFSIZE    3456

struct dbhead {
	int             buff_size;
	boolean         buf_ovrflw;
	int     	data_len;	/* number of bytes in data area */
	int     	data_offset;	/* number of bytes currently */
};

typedef struct Data_Buffer {
	struct dbhead  dbhead;
	char	       buf_start;	/* start of the buffer */
} Data_Buffer;	

typedef struct {
                      /* network id parameter */
  char  adapter_code; /* minor # of adapter card  */
                      /* for this command (1 to MAX_MINORS) */
  char  session_code; /* session for this command (1 to MAX_SESSIONS) */
} NETWORK_ID;

/*****************************************************************************/
/* the interrupt type passed to AckCmd, InterruptDH, and dh intr handler     */
/*****************************************************************************/
typedef enum 
{
   INTR_UNSOL,                 /* Unsolicited interrupt                   */
   INTR_SOL_START,             /* Start Device Solicited                  */
   INTR_SOL_WRITE              /* Write Device Solicited                  */
} INTR_TYPE;

/*****************************************************************************/
/* special values of cu_type used to detect special cases */
/*****************************************************************************/
#  define BSC_3274    0x42        /* 3274    type BSC CU                     */
#  define CH_3274     0xC2        /* 3274    type Channel CU                 */
#  define CH_4361     0xC3        /* 4361    type Channel CU                 */

/*****************************************************************************/
/* special values of the TCA data header flag char for read/write */
/*****************************************************************************/
#  define F_O_M        0x80       /* first of message */
#  define L_O_M        0x40       /* last  of message */
#  define BSC_TRANSPAR 0x10       /* BSC transparency */
#  define BSC_TEST_REQ 0x04       /* BSC test request */

/*****************************************************************************/
/* the TCA, that is, the directly addressable adapter memory */
/*****************************************************************************/
#define MAX_CULTA 8   /* this is a control unit definition - don't change it */

/* WARNING!  Multi-char quantities in this structure must be ms char to ls */
typedef volatile struct
{
      char      dpastat;                 /*        000   |        0000 */
      char      dpsstat;                 /*        001   |        0001 */
      char      dssv;                    /*        002   |        0002 */
      char      dssp1;                   /*        003   |        0003 */
      char      dssp2;                   /*        004   |        0004 */
      char      dssp3;                   /*        005   |        0005 */
      char      daltad;                  /*        006   |        0006 */
      char      daev;                    /*        007   |        0007 */
      char      daep1;                   /*        008   |        0008 */
      char      daep2;                   /*        009   |        0009 */
      char      daep3;                   /*        010   |        000A */
      char      daep4;                   /*        011   |        000B */
      char      dtid1;                   /*        012   |        000C */
      char      dtid2;                   /*        013   |        000D */
      char      dtid3;                   /*        014   |        000E */
      char      dtid4;                   /*        015   |        000F */
      char      dbuf_msb;                /*        016   |        0010 */
      char      dbuf_lsb;                /*        017   |        0011 */
      char      reserved1[2];            /*  018 - 019   | 0012 - 0013 */
      char      reserved2[4];            /*  020 - 023   | 0014 - 0017 */
      char      reserved3[4];            /*  024 - 027   | 0018 - 001B */
      char      reserved4[4];            /*  028 - 031   | 001C - 001F */
      char      exflt;                   /*        032   |        0020 */
      char      exfrq;                   /*        033   |        0021 */
      char      exfp1;                   /*        034   |        0022 */
      char      exfp2;                   /*        035   |        0023 */
      char      exfp3;                   /*        036   |        0024 */
      char      exfp4;                   /*        037   |        0025 */
      char      exfak;                   /*        038   |        0026 */
      char      reserved5;               /*        039   |        0027 */
      char      reserved6[4];            /*  040 - 043   | 0028 - 002B */
      char      reserved7[4];            /*  044 - 047   | 002C - 002F */
      char      reserved8[4];            /*  048 - 051   | 0030 - 0033 */
      char      reserved9[4];            /*  052 - 047   | 0034 - 0037 */
      char      reserved10[4];           /*  056 - 059   | 0038 - 003B */
      char      reserved11[4];           /*  060 - 063   | 003C - 003F */

/*****************************************************************************/
/* second half of the TCA */
/*****************************************************************************/

      char      cudp_msb;                /*        064   |        0040 */
      char      cudp_lsb;                /*        065   |        0041 */
      char      cultad;                  /*        066   |        0042 */
      char      reserved12;              /*        067   |        0043 */
      char      cufrv;                   /*        068   |        0044 */
      char      cusyn;                   /*        069   |        0045 */
      char      cufrp1;                  /*        070   |        0046 */
      char      cufrp2;                  /*        071   |        0047 */
      char      cufrp3;                  /*        072   |        0048 */
      char      cufrp4;                  /*        073   |        0049 */
      char      reserved13;              /*        074   |        004A */
      char      reserved14;              /*        075   |        004B */
      char      reserved15[4];           /*  076 - 079   | 004C - 004F */
      char      cudport;                 /*        080   |        0050 */
      char      cuat;                    /*        081   |        0051 */
      char      cudser_msb;              /*        082   |        0052 */
      char      cudser_lsb;              /*        083   |        0053 */
      char      culta[MAX_CULTA];        /*  084 - 091   | 0054 - 005B */
      char      exfd1;                   /*        092   |        005C */
      char      exfd2;                   /*        093   |        005D */
      char      exfd3;                   /*        094   |        005E */
      char      exfd4;                   /*        095   |        005F */
      char      extime;                  /*        096   |        0060 */
      char      cudsl;                   /*        097   |        0061 */
      char      reserved16[2];           /*  098 - 099   | 0062 - 0063 */
      char      reserved17[4];           /*  100 - 103   | 0064 - 0067 */
      char      reserved18[4];           /*  104 - 107   | 0068 - 006B */
      char      reserved19[4];           /*  108 - 111   | 006C - 006F */
      char      reserved20[4];           /*  112 - 115   | 0070 - 0073 */
      char      reserved21[4];           /*  116 - 119   | 0074 - 0077 */
      char      reserved22[4];           /*  120 - 123   | 0078 - 007B */
      char      reserved23[2];           /*  124 - 125   | 007C - 007D */
      char      cuslvl_1;                /*        126   |        007E */
      char      cuslvl_2;                /*        127   |        007F */
} tca_t;

/*****************************************************************************/
/* sub-structure DDS_HDW_SECTION of DDS_DATA */
/*****************************************************************************/
#define MAX_NUM_BUS 2               /* number of i/o adapters                */
typedef volatile struct
{
      int     bus_mem_size;         /* size of adapter bus memory            */
      int     bus_mem_beg;          /* begin of adapter bus memory           */
      int     io_port;              /* port base (2D0,6D0,AD0,or ED0)        */
      int     slot_number;          /* slot number for this card             */
      int     last_disconnect;      /* time of last disconnect (init to 0)   */
      boolean PIO_error;            /* permanent PIO error occurred          */
      ulong   c327_busid;           /* ID of microchanel bus being used      */
} DDS_HDW_SECTION;

/*****************************************************************************/
/* sub-structure DDS_DEV_SECTION of DDS_DATA */
/*****************************************************************************/
typedef volatile struct
{
   long  buffer_size;           /* how big to make the data buffers     @*/
   int   netid_table_entries;   /* determines amount of memory allocated@*/

   union
   {
      char dev_array[36];
      struct
      {
         char   machine_type_number[4];    /* machine type number        @*/
         char   customer_id;               /* customer id                @*/
         char   model_number[3];           /* model number               @*/
         char   plant_manufactured[2];     /* plant manufactured         @*/
         char   serial_no[7];              /* serial number              @*/
         char   software_release_level[3]; /* software release level     @*/
         char   EC_level[16];              /* eng change (EC) level      @*/
      } dev;
   } dev_info;

   /* these items are initialized in the DDS based on user information */
   char       culta_prt[MAX_CULTA];  /* printer session addresses         */

   /* these items are set from adapter memory after a connection is made  */
   char       cuslvl_1_dds;         /* cu TCA support level bit map part 1*/
   char       cuslvl_2_dds;         /* cu TCA support level bit map part 2*/
   char       cuat_dds;             /* cu host attachment protocol bit map*/
   char       culta_dds[MAX_CULTA]; /* logical terminal session addr in cu*/

   union
   {
      char cu_array[40];
      struct
      {
         char   data_length;               /* amt data cu sent to device  */
         char   data_format;               /* info gened by cu or dev     */
         char   cont_id_1;                 /* EBCDIC chars for to cu type */
         char   cont_id_2;                 /* EBCDIC chars for to cu type */
         char   machine_type_number[4];    /* machine type number         */
         char   customer_id;               /* customer id                 */
         char   model_number[3];           /* model number                */
         char   plant_manufactured[2];     /* plant manufactured          */
         char   serial_no[7];              /* serial number               */
         char   software_release_level[3]; /* software release level      */
         char   EC_level[16];              /* eng change (EC) level       */
      } cu;
   } cu_info;

} DDS_DEV_SECTION;

/*****************************************************************************/
/* an async que element for dft mode */
/*****************************************************************************/
typedef volatile struct
{
   char    as_parm[4];            /* Async Request parms  */
   char    as_event;              /* Event ID code value */
   char    as_lta;                /* Logical terminal address */
} ASYNC_REQ;

/*****************************************************************************/
/* timer types for dft mode */
/*****************************************************************************/
typedef enum
{
   TIMER_TYPE_NONE = 0,        /* timer is not in use                     */
   TIMER_TYPE_RESTART,        /* we are waiting for 5 sec for restart    */
   TIMER_TYPE_CU_INACTIVE,     /* normally 30-second timeout for timer 1  */
   TIMER_TYPE_CU_DEAD,         /* normally 10-second timeout for timer 1  */
   TIMER_TYPE_HOST_TIMING,     /* normally .5-second timeout for timer 2  */
   TIMER_TYPE_DEVICE_TIMING    /* normally .5-second timeout for timer 2  */
} TIMER_TYPE;

/*****************************************************************************/
/* states of interface for dft mode */
/*****************************************************************************/
typedef enum
{
   NOT_CONNECTED = 0,          /* COAX disabled                           */
   CONNECTED_A_IDLE,           /* COAX on & Start Op done                 */
   CONNECTED_A_ACTIVE          /* COAX on & Start Op going                */
} IN_STATE;

/*****************************************************************************/
/* states of device for dft mode */
/*****************************************************************************/
typedef enum
{
   OFF_LINE_TO_CU = 0,         /* Interface Disconnected                  */
   INIT_IN_PROCESS,            /* from POR to WCUS start op               */
   ON_LINE_TO_CU,              /* WCUS = 3274 Ready                       */
   PENDING_ON_LINE,            /* AEDV on & async stat pnd                */
   PENDING_OFF_LINE,           /* AEDV off & async stat pnd               */
   ON_LINE_TO_HOST             /* AEDV on & async stat ack                */
} DV_STATE;

/*****************************************************************************/
/* states of device driver for dft mode */
/*****************************************************************************/
typedef enum
{
   NOT_OPENED = 0,             /* DD not opened                           */
   OPEN_PENDING,               /* Open in progress                        */
   OPEN_TIMEOUT,               /* Open timed out                          */
   DD_OPENED                   /* DD is currently opened                  */
} OP_STATE;

/*****************************************************************************/
/* interrupt/status register bits for cut mode                               */
/*****************************************************************************/
#define CUT_KEYACCPT  0x01  /* keystroke accepted */
#define CUT_RESETOP   0x02  /* reset command decoded */
#define CUT_VISOUND   0x04  /* VSR register updated */
#define CUT_BBMODCOM  0x10  /* buffer modified complete */
#define CUT_LDIOADR   0x20  /* load I/O address command decoded */
#define CUT_BBMOD     0x40  /* buffer being modified */
#define CUT_INTGEN    0x80  /* interrupt generated */

/*****************************************************************************/
/* sub-structure DDS_WORK_SECTION of DDS_DATA */
/*****************************************************************************/
#define ASYNC_QUE_SIZE 2*MAX_SESSIONS
#define CUT_QUE_SIZE 100

typedef volatile struct
{
      int        CUT_que_nextin;        /* input index */
      int        CUT_que_nextout;       /* output index */
      char       CUT_que[CUT_QUE_SIZE]; /* que for cut mode keystrokes */
      short      CUT_intr_mask;         /* msb=vis_snd_reg lsb=intr_stat_reg */
      boolean    CUT_read_without_wait; /* don't wait for change during read */
      int        CUT_read_sleeping;     /* read sleeping until change */
      int        CUT_read_sleep_event;  /* event read is sleeping on */
      boolean    CUT_buffer_changed;    /* buffer changed */
      boolean    CUT_cursor_changed;    /* cursor changed */
      boolean    CUT_reset_command;     /* reset command decoded */
      boolean    CUT_key_out_busy;      /* keystroke sent, not yet ack'ed */
      boolean    CUT_timer_started;     /* 60 second timer started */
      boolean    CUT_had_interrupt;     /* interrupt since timer started */
      char       CUT_vis_snd;           /* or'ed visual/sound status for CUT */
      char       CUT_intr_stat;         /* or'ed intr stat for CUT */
      char       diag_intr_stat;        /* or'ed intr stat for diagnostics */
      char       diag_intr_stat2;       /* or'ed intr stat for diagnostics */
      TIMER_TYPE timer_type_1;          /* type of timer 1 in use */
      TIMER_TYPE timer_type_2;          /* type of timer 2 in use */
      int        timer_count_1;         /* current timer 1 count if active */
      int        timer_count_2;         /* current timer 2 count if active */
      char       num_opens;             /* num opens completed "link count" */
      char       num_sess;              /* min(netid_tbl_entries,#cu suports)*/
      char       intr_reg_save;         /* Interrupt/Status Reg Saved */
      char       conn_ctrl_save;        /* IL Control Register Saved */
      OP_STATE   open_state;            /* Device Driver Open State value */
      IN_STATE   interface_state;       /* Interface State Variable */
      DV_STATE   device_state;          /* Device State Variable */
      boolean    rm_cmd_write_pending;  /* DH has asked dft to do rm Write  */
      boolean    rm_write_in_progress;  /* rm write in prog for this session */
      boolean    WCUS_30_pending;       /* CU sent a comm check reminder     */
      boolean    reset_in_process;      /* ignore first Read ID from cu */
      boolean    WCUS_10_received;      /* CU reported a state of readiness */
      boolean    restart_device;         /* COAX down - Restarting Device */
      boolean    coax_reset_received;    /* Reset command decoded */
      boolean    restart_in_progress;    /* In the middle of a restart       */
      boolean    waiting_for_restart;    /* Waiting for a restart            */
      int        cu_error_o;             /* WCUS Control Unit Error value    */
      char       cusyn_save;             /* CU Syncronization value          */
      char       dpastat_save;           /* AS acknowledgement save char     */
      char       cu_type;                /* combo of cuat and cu_array[7]    */
      boolean    local_attach;           /* control unit is channel type     */
      boolean    non_sna;                /* bi-sync bit from cuat_dds on?    */
      char       el_cnt;                 /* No. of Send_AS requests queued   */
      char       in_ptr;                 /* Input pointer to Send_AS Q       */
      char       out_ptr;                /* Output pointer to Send_AS Q      */
      ASYNC_REQ old_async_req;           /* old async request                */
      ASYNC_REQ async_req_que[ASYNC_QUE_SIZE]; /* queued async requests      */
      int        seek_displacement;      /*  for use by CUT                  */
      boolean    enh_buffer_mgmt;        /* SNA only. Indicates that the CU  
                                           supports enhanced buffer management*/
      boolean    aeep_outstanding;       /* There is an AEEP req. outstanding */
} DDS_WRK_SECTION;

/*****************************************************************************/
/* states of current cu session */
/*****************************************************************************/
typedef enum
{
      LT_NO_SESSION = 0,          /* No Session Available                    */
      LT_OFFLINE,                 /* Logical Terminal Off Line               */
      LT_ONLINE_PEND,             /* LT Pending Online                       */
      LT_ONLINE,                  /* Logical Terminal On Line                */
      LT_WRT_CMD_REDO,            /* Resend AEEP                             */
      LT_WRT_CMD_PEND,            /* AEEP Sent                               */
      LT_WRT_CMD_ACK,             /* PDAT Received - data avail              */
      LT_WRT_LOCK_DA,             /* Wrt lock - data available               */
      LT_WRT_LOCK_NA,             /* Wrt lock - TCA empty                    */
      LT_WRT_RDAT_NA,             /* Wrt lock - TCA empty                    */
      LT_LOCK,                    /* LT Locked (Read etc)                    */
      LT_READ_LOCK,               /* Host out bound data lock                */
      LT_READ_LOCK_PRT,           /* Printer data sent up                    */
      LT_READ_LOCK_DEF,           /* Host bound status deffered              */
      LT_RM_LOCK,                 /* Unsol Read Modify Lock                  */
      LT_RM_LOCKED,               /* Unsol Read Modify Locked                */
      LT_RM_LOCKED_1,             /* RDAT Received                           */
      LT_RM_LOCKED_2,             /* Write Command Received                  */
      LT_RM_LOCKED_3              /* Both RDAT & WRT Received                */
} LT_STATE;

/*****************************************************************************/
/* sub-structure NETID_TBL_ENTRY repeated MAX_SESSIONS times in DDS_DATA */
/*****************************************************************************/
typedef volatile struct
{
   /* pre-set session configuration information */
   boolean       sess_slow_dev;       /* allow slow device feature          */
   char          lt_bit_map;          /* used when responding to some cu req*/
   char          prt_bit_map;         /* bit map for prt session 0==!printer*/

   /* saved parameters from device head */
   Data_Buffer  *read_buff_addr;      /* read buffer header                 */
   char         *read_data_addr;      /* address for next char read         */
   Data_Buffer  *write_buff_addr;     /* write buffer header                */
   char         *write_orig_data_addr;/* original write cmd buffer addr     */
   int          write_orig_data_len; /* original write cmd buffer length   */
   char         *write_data_addr;     /* address of next data char to write */
   int          write_data_len;      /* number of chars left to write      */

   /* parameters used for SNA only      */
   char         trans_header[6];     /* 6-byte SNA transmission header     */
   boolean      bind_recvd;          /* SNA only. Indicates that last data
                                        seg. recieved was a bind/unbind. 
                                        Only useful if non-EBM support CU */
   boolean      first_inbound;        /* SNA only. Indicates that the first
                                         inbound data should broadcast size*/
   int          MaxRuSize;            /* Maximum RU size that will fit in
                                         one buffer */


   /* state/status used by device handler */
   NETWORK_ID    network_id;          /* dft creates this during Start      */
   LT_STATE      lt_state;            /* current state of cu session        */
   boolean       cmd_write_pending;   /* DH has asked dft to do Write       */
   boolean       cmd_start_pending;   /* DH has asked dft to do Start       */
   boolean       cmd_halt_pending;    /* DH has asked dft to do Halt??SEND??*/
   boolean       kbd_reset_pending;   /* kbd reset pressed but not processed*/
   boolean       program_check;       /* cu issued program check            */
   boolean       ack_pending;         /* dft waiting on emul to ack data    */
   boolean       halt_pending;        /* dft waiting on cu to ack term sess */
   boolean       apnd_to_buffer;      /* recv data should append, not ovrlay*/
   boolean       cmd_chaining;        /* cu is chaining 3270 cmds together  */
   boolean       write_in_progress;   /* write in progress for this session */
   boolean       BSC_transparency;    /* write using BSC transparency mode  */

   /* saved info from control unit */
   uchar         lt_id;               /* cu's logical terminal ID           */
   char          cmd_3270;            /* 3270 host cmd type recvd on WLCC   */
   boolean       start_print;         /* 3270 host cmd stat for prnt sess   */

} NETID_TBL_ENTRY;

/*****************************************************************************/
/* the header portion of the DDS */
/*****************************************************************************/
typedef volatile struct
{
   char            dev_name[16];     /* device name                       @*/
   int             minor_number;     /* minor device number               @*/
} DDS_HDR_SECTION;

/*****************************************************************************/
/* this is it -- the main DDS_DATA structure */
/*****************************************************************************/
typedef volatile struct              /* one DDS_DATA for each adapter card */
{
    DDS_HDR_SECTION  dds_hdr_section;  /* structure defined above            */
    DDS_HDW_SECTION  dds_hdw_section;  /* structure defined above            */
    DDS_DEV_SECTION  dds_dev_section;  /* structure defined above            */
    DDS_WRK_SECTION  dds_wrk_section;  /* structure defined above            */
    NETID_TBL_ENTRY  netid_tbl_entry[MAX_SESSIONS]; /* may have fewer entries*/
} DDS_DATA;

#define HDR     dds_ptr->dds_hdr_section
#define HDW     dds_ptr->dds_hdw_section
#define DEV     dds_ptr->dds_dev_section
#define WRK     dds_ptr->dds_wrk_section
#define TBL(x)  dds_ptr->netid_tbl_entry[x]

/*****************************************************************************/
/* the queues used filled by interrupt & timer routines, emptied by OffLevel */
/*****************************************************************************/
typedef struct
{
   int       ndx_in;
   int       ndx_out;
   DDS_DATA *dds_ptr[TIMR_QUE_SIZE];
} TIMR_QUE;

typedef struct
{
   int       ndx_in;
   int       ndx_out;
   DDS_DATA *dds_ptr[INTR_QUE_SIZE];
} INTR_QUE;
/*
** Structure for adapter control 
*/
typedef struct {
    DDS_STATE  dds_state;
    DDS_DATA  *dds_ptr;
} DDS_CONTROL;

/*
** function prototypes
*/

/* c327cutpn.c */
void c327cut_connect(DDS_DATA *, int);
void c327cut_timeout(DDS_DATA *);
void c327cutProcessInterrupt(DDS_DATA *);

/* c327pn.c */
void pio_error (DDS_DATA *);
uchar c327_getc(c327io_t, DDS_DATA *, uint);
void c327_putc(c327io_t, DDS_DATA *, uint, uchar);
void c327_ios(c327io_t, DDS_DATA *, int , char *, int);
int gets_busmem(DDS_DATA *, int, struct uio *);
int puts_busmem(DDS_DATA *, int, struct uio *);
void c327SaveTrace(char *, int, int, int, int);
int c327_off_level(void);
int c327_cfg_init(dev_t, struct uio *);
int c327_cfg_term(dev_t);
void c327ConnectWait(DDS_DATA *, int);
void c327SimulateReset(DDS_DATA *);
void c327Disconnect(DDS_DATA *);
int nodev(void);

/* c327cutnpn.c */
int c327cutread(DDS_DATA *, struct uio *);
int c327cutwrite(DDS_DATA *, struct uio *);
int c327cutioctl(DDS_DATA *, int, caddr_t);

/* c327diagdd.c */
void c327_rdwrtcmd(DDS_DATA *, boolean, uchar);
char c327_rcvcmd(DDS_DATA *, int);
int c327_diag_conn_test(DDS_DATA *, C327DIAG_DATA *);
int c327diagread(DDS_DATA *, struct uio *);
int c327diagwrite(DDS_DATA *, struct uio *);
int c327diagioctl(DDS_DATA *, int, caddr_t);

/* c327npn.c */
void c327InitDDS(DDS_DATA *);
void c327_add_dds(c327_dds *, int);
int c327config(dev_t, int, struct uio *);
int c327mpx(dev_t, int *, char *);
int c327open(dev_t, uint, int, int);
int c327close(dev_t, int);
int c327read(dev_t, struct uio *, int, int);
int c327write(dev_t, struct uio *, int, int);
int c327select(dev_t, ushort, ushort *, int);
int c327ioctl(dev_t, int, caddr_t, int, int);

/* dftnsCmd.c */
int dftnsStart(DDS_DATA *, char, char, Data_Buffer *,boolean *);
int dftnsClose(DDS_DATA *dds_ptr);
int dftnsWrite (NETWORK_ID,Data_Buffer *, uchar);
int dftnsHalt (NETWORK_ID);

#endif /* _H_C327DD */
