/* @(#)52       1.6.1.1  src/bos/kernel/sys/POWER/pscsi720dd.h, sysxscsi, bos41J, 9515A_all 4/6/95 19:07:33 */
#ifndef _H_PSCSI720
#define _H_PSCSI720
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver (NCR 53C720) Header File
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/************************************************************************/
/* General Device Driver Defines                                        */
/************************************************************************/

#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif

#define DEFAULT_MIN_PHASE       0x19 
#define DEFAULT_BYTE_BUF        0x08

#define REG_FAIL                -1
#define MAXREQUEST    0x40000            /* largest data xfer size:      */
                                         /* =  262144 dec. (256KB)       */

#define MAX_SCRIPTS     7                /* max scripts possible (7 ids) */
#define PSC_WORDSIZE	32		 /* # bits in a word  		 */
#define TCE_RESERVED    0x77
#define DMA_INIT        MICRO_CHANNEL_DMA/* DMA init flags               */
#define DMA_TYPE        0                /* DMA master/complete flags    */
#define ST_SIZE         256              /* size of a small transfer     */
#define NUM_STA         (PAGESIZE/ST_SIZE)   /* num STAs available*/
#define STA_TCE		0
#define NUM_4K_TCES     64
#define TCE_TABLE_SIZE  (NUM_4K_TCES)
#define LARGE_TCE_SIZE  8
#define LARGESIZE       (PAGESIZE*LARGE_TCE_SIZE)
#define PSC_MAX_PREEMPTS 80		  /* times a cmd can be preempted */
#define LONGWAIT        5                 /* timeout value for abrt/bdrs  */
#define RESETWAIT       5                 /* timeout value for bus resets */
#define DISC_PENDING    0x02		  /* a disc. int. may be pending  */
#define DMA_ERR         1
#define REG_ERR         2
#define PHASE_ERR       3
#define DEVICE_ERR      4
#define DISC_ERR        5
#define HOST_ERR        6
#define HBUS_ERR        7

                    /* the ioctl SCSI commands      */
#define PSC_NO_ERR      0x00            /* routine call failed          */
#define PSC_FAILED      0x01            /* routine call failed          */
#define PSC_COPY_ERROR  0x02            /* error returned during xmem   */
#define PSC_DMA_ERROR   0x03            /* error returned during cleanup*/
#define PSC_RESET_CHIP  0x01            /* flag used to cleanup register*/
#define PSC_RESET_DMA   0x02            /* flag used to cleanup DMA     */
#define PSC_COMP_RESET  0x000100a0      /* @ of component reset register*/
#define PSC_COMP_OFF    0xfffffffb      /* mask to reset scsi comp off  */
#define PSC_COMP_ON     0xffffffff      /* mask to reset scsi comp on   */

#define PSC_ALLOC_FAILED -1
#define PSC_NO_TAG_FREE  -1
#define PSC_NO_TCE_FREE  -2
#define PSC_NO_STA_FREE  -3


/* the following macros calculate the bus dma address the adapter uses  */
/* when using the reserved TCE range which was passed in the ddi area.  */
#define DMA_ADDR(start_addr, tce_num) \
    ((start_addr) + ((tce_num) * PAGESIZE))
#define DMA_ADDR2(start_addr, tce_num) \
    ((start_addr) + ((tce_num) * LARGESIZE))

#define INT_TYPE        BUS_MICRO_CHANNEL /* bus type for interrupt level */
#define INT_FLAGS       0               /* define as sharable interrupt */

/************************************************************************/
/* MACROS                                                               */
/************************************************************************/
/* this macro reset the n th bit in the long word passed                */
#define ALLOC(lw,n)     lw = lw &~(1<<(31-(n)));

/* this macro set the n th bit in the long word passed                  */
#define FREE(lw,n)      lw = lw | (1<<(31-(n)));

/* this macro checks the n th bit in the long word passed               */
#define IN_USE(lw,n)    (~(lw) & (1<<(31-(n)))) 

/* FUNCTION:  Frees a command tag                                         */
/* EXECUTION ENVIRONMENT:  Interrupts should be disabled.                 */
/* NOTES:  This routine frees a command tag for later use.                */
#define p720_FREE_TAG(com_ptr)                                             \
        {                                                                  \
                TRACE_2 ("freeTAG", (int) (com_ptr)->tag, (int) (com_ptr)) \
                (com_ptr)->in_use = FALSE;                                 \
                FREE(adp_str.TAG_alloc[(com_ptr)->tag / PSC_WORDSIZE],     \
                     (com_ptr)->tag % PSC_WORDSIZE)                        \
        }

/* the following macro is used to quickly generate the device index.    */
/* the macro assumes "a" is SCSI ID, and "b" is LUN                     */
#define INDEX(a,b)    ((((a) & 0x07)<<3) | ((b) & 0x07))

/* this macro returns the scsi id from a previously generated index.    */
#define SID(x)        ((x)>>3)

/* this macro returns the lun id from a previously generated index.     */
#define LUN(x)        ((x) & 0x07)

/* this macro is used to determine the number of times a patch to the   */
/* script needs to be made, based on the number of array elements.      */
#define     S_COUNT(a)      (sizeof(a)/sizeof((a)[0]))

/* remove an element from the bp_save queue */
#define p720_DEQ_BP_SAVE(dev_ptr)    \
            dev_ptr->bp_save_head = \
                (struct sc_buf *) dev_ptr->bp_save_head->bufstruct.av_forw;

/* remove an element from the cmd_save queue */
#define p720_DEQ_CMD_SAVE(dev_ptr)    \
            dev_ptr->cmd_save_head = dev_ptr->cmd_save_head->next_dev_cmd;

#define ISSUE_MAIN_TO_DEVICE(script_dma_addr, eaddr)   \
( p720_write_reg( (uint)DSP, (char) DSP_SIZE, \
        word_reverse(script_dma_addr + Ent_scripts_entry_point), eaddr ) )

#define ISSUE_MAIN_AFTER_NEG(script_dma_addr, eaddr)   \
( p720_write_reg( (uint)DSP, (char) DSP_SIZE, \
        word_reverse(script_dma_addr + Ent_cleanup_phase), eaddr ) )

/************************************************************************/
/* Adapter I/O Defines                                                  */
/************************************************************************/
/* Adapter POS register addressing definitions for real RAINBOW */
#define POS0_SIZE       0x01    /* size in bytes                */
#define POS0            0x00400000  /* RO, card id low        0xBA  */
#define POS1_SIZE       0x01    /* size in bytes                */
#define POS1            0x00400001  /* RO, card id high       0x8F  */
#define POS2_SIZE       0x01    /* size in bytes                */
#define POS2            0x00400002  /* RW, card arb and enable      */
#define POS4_SIZE       0x01    /* size in bytes                */
#define POS4            0x00400004  /* RW, card int and nibble ena  */
#define POS5_SIZE       0x01    /* size in bytes                */
#define POS5            0x00400005  /* RW, card status, unused      */

#define POS0_VAL        0xBA            /* Card ID low value            */
#define POS1_VAL        0x8F            /* Card ID high value           */

#define SXFER_INIT              0x08
#define SXFER_ASYNC_MOVE        0x78050000
#define SXFER_DEF_MOVE          0x78050800
#define SXFER_MOVE_MASK         0x78050000
#define SCNTL1_EOFF_MOVE        0x7C017F00
#define SCNTL1_EON_MOVE         0x7A018000
#define SCNTL3_FAST_MOVE        0x78031300
#define SCNTL3_SLOW_MOVE        0x78033300

#define P720_NOP_PATCH		0x80000000
#define P720_INTR_PATCH		0x98080000

#define SCNTL1_EXC_ON           0x80       /* sets extra clock cycle */
#define SCNTL1_EXC_OFF          0x00       /* does not set extra clock cycle */
#define SCNTL3_INIT_FAST        0x13       /* set for up to 10 Mbyte/sec */
#define SCNTL3_INIT_SLOW        0x33       /* set for 5 Mbyte/sec or less */

/* 53C720 register addressing definitions */
#define SCNTL0_SIZE     0x01
#define SCNTL0          0x0080
#define SCNTL1_SIZE     0x01
#define SCNTL1          0x0081
#define SCNTL2_SIZE     0x01
#define SCNTL2          0x0082
#define SCNTL3_SIZE     0x01
#define SCNTL3          0x0083
#define SCID_SIZE       0x01
#define SCID            0x0084
#define SXFER_SIZE      0x01
#define SXFER           0x0085
#define SDID_SIZE       0x01
#define SDID            0x0086
#define GPREG_SIZE      0x01
#define GPREG           0x0087
#define SFBR_SIZE       0x01
#define SFBR            0x0088
#define SOCL_SIZE       0x01
#define SOCL            0x0089
#define SSID_SIZE       0x01
#define SSID            0x008A
#define SBCL_SIZE       0x01
#define SBCL            0x008B
#define DSTAT_SIZE      0x01
#define DSTAT           0x008C
#define    IID                0x01    /* DSTAT Illegal Instruction        */
#define    WTD                0x02    /* DSTAT Watchdog Timer             */
#define    SIR                0x04    /* DSTAT Scrpt Intrpt Instr. Recvd. */
#define    SSI                0x08    /* DSTAT Script Single Step         */
#define    DABRT              0x10    /* DSTAT Abort occurred             */
#define    BF                 0x20    /* DSTAT Host Bus Fault occurred    */
#define    HPE                0x40    /* DSTAT Host Parity Error detected */
#define    DFE                0x80    /* DSTAT DMA FIFO Empty             */
#define SSTAT0_SIZE     0x01
#define SSTAT0          0x008D
#define SSTAT1_SIZE     0x01
#define SSTAT1          0x008E
#define SSTAT2_SIZE     0x01
#define SSTAT2          0x008F
#define DSA_SIZE        0x04
#define DSA             0x0090
#define ISTAT_SIZE      0x01
#define ISTAT           0x0094
#define    DIP          0x01    /* ISTAT DMA Interrupt Pending  */
#define    SIP          0x02    /* ISTAT SCSI Interrupt Pending */
#define    CONNECTED    0x08    /* Chip currently on scsi bus   */
#define    SIGP		0x20    /* Signal process               */
#define    RST          0x40    /* Software reset of chip       */
#define    ABRT         0x80    /* ISTAT Abort Operation        */
#define CONN_INT_TEST   0x0B    /*  (CONNECTED | SIP | DIP)     */
#define NO_ABORT	0x10    /* SEM bit in ISTAT is set      */
#define CTEST0_SIZE     0x01
#define CTEST0          0x0098
#define CTEST1_SIZE     0x01
#define CTEST1          0x0099
#define CTEST2_SIZE     0x01
#define CTEST2          0x009A
#define CTEST3_SIZE     0x01
#define CTEST3          0x009B
#define TEMP_SIZE       0x04
#define TEMP            0x009C   /* 32 bit stack register 9c-9f */
#define DFIFO_SIZE      0x01
#define DFIFO           0x00A0
#define CTEST4_SIZE     0x01
#define CTEST4          0x00A1
#define CTEST5_SIZE     0x01
#define CTEST5          0x00A2
#define CTEST6_SIZE     0x01
#define CTEST6          0x00A3
#define DBC_SIZE        0x03
#define DBC             0x00A4   /* 24 bit DMA Byte Counter Register a4-a6 */
#define DCMD_SIZE       0x01
#define DCMD            0x00A7
#define DNAD_SIZE       0x04
#define DNAD            0x00A8   /* 32 bit Next Address Data register a8-ab */
#define DSP_SIZE        0x04
#define DSP             0x00AC   /* 32 bit SCRIPTS Pointer register ac-af */
#define DSPS_SIZE       0x04
#define DSPS            0x00B0   /* 32 bit SCRIPTS Pointer Save reg b0-b3 */
#define SCRATCHA_SIZE   0x04
#define SCRATCHA        0x00B4   /* 32 bit general purpose scratch pad A  */
#define SCRATCHA0_SIZE  0x01
#define SCRATCHA0       0x00B4   /* 32 bit general purpose scratch pad A  */
#define DMODE_SIZE      0x01
#define DMODE           0x00B8
#define DIEN_MASK       0x7F     /* don't mask any DMA interrupts */
#define DIEN_SIZE       0x01
#define DIEN            0x00B9
#define DWT_SIZE        0x01
#define DWT             0x00BA
#define DCNTL_SIZE      0x01
#define DCNTL           0x00BB
#define ADDER_SIZE      0x04
#define ADDER           0x00BC
#define SIEN_MASK       0x078F  /* mask non-fatal SCSI interrupts, 2 bytes */
#define SIEN_SIZE       0x02     
#define SIEN            0x00C0  /* pseudo-register for 2 byte operations */
#define SIEN0_MASK      0x8F    /* mask non-fatal SCSI interrupts */
#define SIEN0_SIZE      0x01
#define SIEN0           0x00C0
#define SIEN1_MASK      0x07    /* mask non-fatal SCSI interrupts */
#define SIEN1_SIZE      0x01
#define SIEN1           0x00C1
#define SIST_SIZE       0x02
#define SIST            0x00C2  /* pseudo-register for 2 byte reads */
#define SIST0_SIZE      0x01
#define SIST0           0x00C2
#define SIST1_SIZE      0x01
#define SIST1           0x00C3
#define SLPAR_SIZE      0x01
#define SLPAR           0x00C4
#define SWIDE_SIZE      0x01
#define SWIDE           0x00C5
#define MACNTL_SIZE     0x01
#define MACNTL          0x00C6
#define GPCNTL_SIZE     0x01
#define GPCNTL          0x00C7
#define STIME0_SIZE     0x01
#define STIME0          0x00C8
#define STIME1_SIZE     0x01
#define STIME1          0x00C9
#define RESPID0_SIZE    0x01
#define RESPID0         0x00CA
#define RESPID1_SIZE    0x01
#define RESPID1         0x00CB
#define STEST0_SIZE     0x01
#define STEST0          0x00CC
#define STEST1_SIZE     0x01
#define STEST1          0x00CD
#define STEST2_SIZE     0x01
#define STEST2          0x00CE
#define STEST3_SIZE     0x01
#define STEST3          0x00CF
#define SSIDL_SIZE      0x02
#define SSIDL           0x00D0
#define SODL_SIZE       0x02
#define SODL            0x00D4
#define SODL0_SIZE      0x01
#define SODL0           0x00D4
#define SODL1_SIZE      0x01
#define SODL1           0x00D5
#define SBDL_SIZE       0x02
#define SBDL            0x00D8
#define SCRATCHB_SIZE   0x04
#define SCRATCHB        0x00DC   /* 32 bit general purpose scratch pad B  */

#define SCSIRS_SIZE     0x04
#ifdef P720_DD1
#define SCSIRS          0x0020   /* temporary fix for Olympus DD1 problem */
#else
#define SCSIRS          0x007C   /* 32 bit SCSI Reset-Status register 7c-7f */
#endif

#define SCNTL0_INIT     0xCC     /* Full arbitration, EPC, EPG */
#define SCID_INIT       0x40     /* Reselection enabled */
#define DMODE_INIT      0x80     /* 8-transfer burst */
#define DCNTL_INIT      0x03     /* Fast arbitration, not 700 compatability mode */
#define CTEST0_INIT     0x80     /* Disable cache-line bursting */
#define STIME0_INIT     0x0C     /* No HTH timing, 204.8 msec sel. time-out */
#define STEST3_INIT     0x90     /* Set EAN and DSI */

/* Miscellaneous defines */
#define MAX_RESTART_RETRIES 3           /* num retries of Restart cmd   */
#define MAX_DUMP_LOOPS      10          /* num of loops for dump intr poll */
#define IPL_MAX_SECS        15

/************************************************************************/
/* SCSI Interrupt Defines                                               */
/************************************************************************/
#define SCSI_PARITY_ERR               0x01
#define SCSI_RST                      0x02
#define SCSI_UNEXP_DISC               0x04
#define SCSI_GROSS_ERR                0x08
#define SCSI_RESELECTED               0x10
#define SCSI_SELECTED                 0x20
#define SCSI_COMP                     0x40
#define PHASE_MIS                     0x80
#define SCSI_HTH_TIMEOUT              0x100
#define SCSI_GEN_TIMEOUT              0x200
#define SCSI_SEL_TIMEOUT              0x400

/************************************************************************/
/* Miscellaneous Structures                                             */
/************************************************************************/
struct timer    {
    struct watchdog dog;                /* the watchdog struct          */
/* can we eliminate siop tmr ? */
#define PSC_SIOP_TMR    1
#define PSC_RESET_TMR   2
#define PSC_COMMAND_TMR 3
#define PSC_RESTART_TMR 4
    uchar       save_time;              /* used to manage the active q  */

    uchar       timer_id;               /* my internal timer id val     */
                                        /*  1 = adapter cmd timer       */
                                        /*  2 = scsi bus reset timer    */
                                        /*  3 = dev_info cmd timer      */
};

/***
*struct small_xfer_area_str {            &* Small Transfer Area Structure *&
*    char   *sta_ptr;                    &* address of this xfer area    *&
*    uchar   in_use;                     &* TRUE if this area in use     *&
*};
***/

struct scripts_struct   {               /* SCRIPTS used to run the SIOP */
    ulong   *script_ptr;                /* pointer to SCRIPTS work area */
    ulong   *dma_ptr;
    uchar   TCE_index;                  /* index into 4K xfer area      */
    uchar   in_use;                     /* TRUE if this area in use     */
};

struct table_i_struct   {               /* Table Indirect area for SIOP */
    ulong   *system_ptr;                /* pointer to SCRIPTS work area */
    ulong   *dma_ptr;
    uchar   TCE_index;                  /* index into 4K xfer area      */
};

struct move_info {
    int data_byte_count;	        /* bytes to move, last 3 valid	*/
    int data_buffer_addr;		/* address of data buffer	*/
    int cmd_byte_count;	        	/* bytes to move, last 3 valid	*/
    int cmd_buffer_addr;		/* address of command buffer	*/
};

struct error_log_def {                  /* driver error log structure   */
    struct err_rec0    errhead;         /* error log header info        */
    struct rc    data;                  /* driver dependent err data    */
};

struct p720_cdt_table {                  /* component dump table struct  */
    struct    cdt_head   p720_cdt_head;  /* header to the dump table     */
    struct    cdt_entry  p720_entry[1];  /* space for each minor + trace */
};

struct scsi_id_info {
   uchar            scsi_id;          /* the target's scsi id         */
   uchar            negotiate_flag;   /* if SCSI negotiation needed   */
   uchar            async_device;     /* if this is defined as async  */
   uchar            restart_in_prog;  /* after BDRs and SCSI resets   */
   uchar            disconnect_flag;  /* indicates disconnect priv.   */
                                      /* in patched identify message  */
   uchar	    tag_flag;	      /* currently patched tag	      */
   uchar	    tag_msg_flag;     /* currently patched tag msg    */
   uchar	    lun_flag;	      /* currently patched lun	      */
   uint             dma_script_ptr;   /* dma addr of the script       */
   int              script_index;     /* index into SCRIPTS array     */
   struct dev_info *bdring_lun;	      /* ptr to lun associated w/ bdr */
};


/************************************************************************/
/* Structures related to device control                                 */
/************************************************************************/
struct dev_info {
    struct timer    dev_watchdog;   /* watchdog timer for dev struct*/
    uchar           opened;
    uchar           scsi_id;        /* SCSI ID of this device       */
    uchar           lun_id;         /* LUN ID of this device        */
    uchar           device_state;   /* what the device structure is doing*/
#define NOTHING_IN_PROGRESS       0x01
#define CMD_IN_PROGRESS           0x02
#define ABORT_IN_PROGRESS         0x04
#define BDR_IN_PROGRESS           0x08
#define NEGOTIATE_IN_PROGRESS     0x10

    uchar           queue_state;       /* device general queue state       */
				       /* effecting acceptance of new cmds */
#define ACTIVE          0x01
#define WAIT_FLUSH   	0x02
#define HALTED          0x04
#define HALTED_CAC      0x08
#define WAIT_INFO   	0x10
#define STOPPING        0x20
#define STOPPING_MASK   0x38		/* stopping during a CAC */
#define WAIT_INFO_or_HALTED_CAC        0x18
#define STOPPING_or_HALTED_or_FLUSHING 0x26
#define ACTIVE_or_WAIT_FLUSH           0x03
#define STOPPING_or_HALTED_CAC         0x28

    uchar           ioctl_wakeup;   /* wakeup sleeping ioctl call   */

#define RETRY_ERROR         0x0001 /* flag used for retry of abort/bdr */
#define CAUSED_TIMEOUT      0x0002 /* this device caused a timeout */
#define SCSI_ABORT          0x0004 /* a scsi abort is active */
#define SCSI_BDR            0x0008 /* a scsi bdr is active for device */
#define SELECT_TIMEOUT      0x0010 /* a selection timeout occurred */
#define NEG_PHASE_2         0x0020 /* used during negotiation */
#define BLOCKED             0x0040 /* indicates failed allocation */
#define STARVATION          0x0080 /* device is undergoing cmd starvation */
#define SCSI_BDR_or_ABORT   0x000C /* a scsi bdr or abort is active */
#define FLUSH_or_STARVE_or_BLOCKED 0x00CC
    ushort          flags;         /* field used to hold the setting of*/
                                   /* flags for the target             */

    int             ioctl_event;
    int             ioctl_errno;
    int             stop_event;

    struct scsi_id_info *sid_ptr;   /* pointer to parent scsi id info */

    struct dev_info *DEVICE_ACTIVE_fwd;
    struct dev_info *DEVICE_ACTIVE_bkwd;
    struct dev_info *ABORT_BDR_fwd;
    struct dev_info *ABORT_BDR_bkwd;

   struct  cmd_info *active_head;      /* ptr to active cmd queue     */
   struct  cmd_info *active_tail;      /* ptr to active cmd queue     */

   struct  cmd_info *wait_head;	      /* ptr to waiting cmd queue    */
   struct  cmd_info *wait_tail;	      /* ptr to waiting cmd queue    */

   struct  cmd_info *cmd_save_head;    /* ptr to frozen command queue */
   struct  cmd_info *cmd_save_tail;    /* ptr to frozen command queue */

   struct sc_buf *bp_save_head;       /* ptr to frozen sc_buf queue  */
   struct sc_buf *bp_save_tail;       /* ptr to frozen sc_buf queue  */

#define PSC_RETRY_COUNT              20
    int             retry_count;    /* times allowed to retry issue */
                                    /* of an abort/bdr command.     */
};

#define P720_TRACE_SIZE  0x800
struct p720_trace_entry {
    union {
        struct {
            char            header1[12];
            int             data;
        } one_val;
        struct {
            char            header2[8];
            int             data1;
            int             data2;
        } two_vals;
        struct {
            char            header3[4];
            int             val1;
            int             val2;
            int             val3;
        } three_vals;
        char                header[16];
    }un;
};

struct p720_trace_struct {
    struct p720_trace_entry  trace_buffer[P720_TRACE_SIZE];
};

/************************************************************************/
/* Structures related to commands                                       */
/************************************************************************/
struct cmd_info {
    uchar        tag;                  	/* Index of this struct in the array */
    uchar        preempt_counter;      	/* to prevent command starvation */

    uchar        resource_state;    	/* Type of resources used */
#define LARGE_TCE_RESOURCES_USED      1
#define SMALL_TCE_RESOURCES_USED      2
#define STA_RESOURCES_USED            3
#define NO_RESOURCES_USED             4

    uchar        flags;
#define PREP_MAIN_COMPLETE  0x0001      /* flag used for prep main check */
#define RESID_SUBTRACT      0x0002 	/* signals we have to update resid */

    uchar	 in_use;		/* used to help validate tag */

    struct cmd_info    *active_fwd;     /* next command in queue */
                                        /* (either waitq or activeq) */
    struct cmd_info    *active_bkwd;    /* previous command in queue */
                                        /* (activeq only) */
    struct cmd_info    *next_dev_cmd;   /* next command for a device */
                                        /* (used in the waitq)       */
    struct dev_info   *device;          /* target device of the command */
    struct sc_buf     *bp;              /* struct sc_buf involved */

    uint         STA_addr;              /* pointer to save area for STA */
    uint         resource_index;        /* save area for STA or TCE index */
    uint         TCE_count;             /* save area for TCE count */
    uint         dma_addr;              /* save area for dma address */
    uint         max_disconnect;        /* max value xferred before disc */
    uint         bytes_moved;           /* # of bytes moved in last xfer */
};


struct adapter_def {
    struct intr     intr_struct;      /* interrupt handler structure   */
    struct adap_ddi ddi;
    struct timer    reset_watchdog;   /* watchdog timer for bus_resets */
    struct timer    restart_watchdog; /* watchdog timer for cmd delays */
    uchar           opened;
    uchar           epow_state;             /* power failure flag   */
                                            /*  0 = normal state    */
#define EPOW_PENDING    4                   /* adap EPOW pending    */

    uchar           errlog_enable;          /* set if errors are to */
    uchar           open_mode;              /* mode opened in:      */
#define NORMAL_MODE     0                   /*  normal operation    */
#define DIAG_MODE       1                   /*  diagnostic mode     */
    uchar           iowait_inited;          /* iowait script inited */

#define TCE_UNUSED  0xFFFFFFFF
    uint            *large_TCE_alloc;       /* pointer to rsvd tces */
                                            /*   management table   */
    ulong           large_tce_start_addr;   /* starting tce, big xfr*/
    ushort          large_req_end;          /* where large reqs end */

#define MAX_DEVICES 64
#define MAX_LUNS 8
    struct dev_info    *device_queue_hash[MAX_DEVICES];
                                /* pointers to the device               */
                                /* queue structures                     */
                                /* access the array via scsi id/lun:    */
                                /*       msb  7 6 5 4 3 2 1 0 lsb       */
                                /*       ----+-+-+-+-+-+-+-+-+---       */
                                /*            x x i i i l l l           */
                                /*                d d d u u u           */
                                /*                2 1 0 n n n           */
                                /*                      2 1 0           */
                                /*                                      */

    struct scsi_id_info sid_info[MAX_SCRIPTS];

#define NB_TAG         256
#define NUM_TAG        256
    struct cmd_info     command[NUM_TAG]; /* Array of struct cmd_info  */

#define START_Q_TAGS   (MAX_DEVICES / PSC_WORDSIZE)     /* alloc word for  */
                                                        /* the queued tags */
#define TAG_TABLE_SIZE   (((NUM_TAG-1) / PSC_WORDSIZE) + 1)
#define TAG_UNUSED     0xFFFFFFFF
    uint               TAG_alloc[TAG_TABLE_SIZE];

    struct sc_buf      *REQUEST_WFR_head;
    struct sc_buf      *REQUEST_WFR_tail;
    struct sc_buf      *blocked_bp;

    struct dev_info        *DEVICE_ACTIVE_head;
    struct dev_info        *DEVICE_ACTIVE_tail;

    struct cmd_info        *DEVICE_WAITING_head;
    struct cmd_info        *DEVICE_WAITING_tail;

    struct dev_info        *ABORT_BDR_head;
    struct dev_info        *ABORT_BDR_tail;

                        /* small transfer area */
    uint                STA_alloc[((NUM_STA-1) / PSC_WORDSIZE) + 1];
    char                *STA[NUM_STA];/* table of the 16 small*/

    struct table_i_struct IND_TABLE;
    struct scripts_struct SCRIPTS[MAX_SCRIPTS];
#define SCR_IN_USE      0x01
#define SCR_UNUSED      0xFF
                        /* table of pointers to    */
                        /* scripts areas created   */
                        /* for each device hung    */
                        /* on the SCSI bus         */
    dev_t               devno;                   /* adapter major/minor  */
    uchar               dump_inited;             /* dump init completed  */
    uchar               dump_started;            /* dump start completed */
    int                 max_request;             /* max xfer allowed     */
    int                 channel_id;              /* dma channel id       */
    int                 dump_pri;                /* saved dump int prior.*/
    struct xmem         xmem_STA;                /* local xmem descrip.  */
    struct xmem         xmem_SCR;                /* local xmem descrip.  */
    struct xmem         xmem_MOV;                /* local xmem descrip.  */

    uint                TCE_alloc[((TCE_TABLE_SIZE - 1) / PSC_WORDSIZE) + 1];

#define NUM_720_SYNC_RATES 12
    uchar               xfer_max[NUM_720_SYNC_RATES];

#ifdef P720_TRACE
    struct p720_trace_struct  *trace_ptr;
    int                 current_trace_line;
#endif
};

/**************************************************************************/
/*    BEGIN SCRIPTS ORIGINAL CONTROL CODE FOR COPYING                     */
/**************************************************************************/

#define A_bdr_select_failed     0x0000001B
#define A_bdr_io_complete       0x0000001C
#define A_modify_data_ptr       0x00000021
#define A_target_sync_sent      0x00000022

#define ISSUE_NEGOTIATE_TO_DEVICE(script_dma_addr, eaddr)      \
( p720_write_reg( (uint)DSP, (char) DSP_SIZE, word_reverse(script_dma_addr + Ent_sync_negotiation), eaddr ) )
/*
 * THIS IS A SIMPLE MACRO TO WRITE THE SCRIPT ADDRESS OF THE NEGOTIATE
 * SCRIPT INTO THE DSP REGISTER.  ALL THE VALUES NEEDED TO DO THE NEGOTIATE
 * WERE ALREADY LOADED BY p720_script_init.
 */

#define GET_CMD_STATUS_BYTE(script_dma_addr)    \
( script_dma_addr[Ent_status_buf/4] >> 24)
/*
 * THIS GOES TO THE BUFFER AREA OF THE MAIN SCRIPT AND READS BACK THE VALUE
 * BEING HELD IN THE STATUS BUFFER.
 */

#include "pscsi720hss.h"

#ifdef DEBUG
#include <stdio.h>
#define DEBUG_0(A)                      {printf(A);}
#define DEBUG_1(A,B)                    {printf(A,B);}
#define DEBUG_2(A,B,C)                  {printf(A,B,C);}
#define DEBUG_3(A,B,C,D)                {printf(A,B,C,D);}
#define DEBUG_4(A,B,C,D,E)              {printf(A,B,C,D,E);}
#define DEBUG_5(A,B,C,D,E,F)            {printf(A,B,C,D,E,F);}
#define DEBUG_6(A,B,C,D,E,F,G)          {printf(A,B,C,D,E,F,G);}
#define DEBUGELSE                       else
#else
#define DEBUG_0(A)
#define DEBUG_1(A,B)
#define DEBUG_2(A,B,C)
#define DEBUG_3(A,B,C,D)
#define DEBUG_4(A,B,C,D,E)
#define DEBUG_5(A,B,C,D,E,F)
#define DEBUG_6(A,B,C,D,E,F,G)
#define DEBUGELSE
#endif
#ifdef P720_TRACE
#define TRACE_1(A,B)                      {p720_trace_1(A,B);}
#define TRACE_2(A,B,C)                    {p720_trace_2(A,B,C);}
#define TRACE_3(A,B,C,D)                  {p720_trace_3(A,B,C,D);}
#else
#define TRACE_1(A,B)
#define TRACE_2(A,B,C)
#define TRACE_3(A,B,C,D)
#endif

#ifndef    _NO_PROTO
/*****************************************************************************/
/*     functions in pscsi720ddt.c                                            */
/*****************************************************************************/

int     p720_config(dev_t devno, int op, struct uio *uiop);
int     p720_open(dev_t devno, ulong devflag, int chan, int ext);
int     p720_close(dev_t devno, int offchan);
void    p720_fail_open(int undo_level, int ret_code, dev_t devno);
int     p720_inquiry(dev_t devno, int arg, ulong devflag);
int     p720_start_unit(dev_t devno, int arg, ulong devflag);
int     p720_test_unit_rdy(dev_t devno, int arg, ulong devflag);
int     p720_readblk(dev_t devno, int arg, ulong devflag);
void    p720_adp_str_init();
int     p720_ioctl(dev_t devno, int cmd, int arg, ulong devflag, int chan,
                  int ext);
struct sc_buf * p720_build_command();
void    p720_script_init(uint *iowait_vir_addr, uint *script_vir_addr,
                        int dev_info_hash, uint iowait_dma_addr,
                        uint script_dma_addr);
int     p720_diagnostic(int arg, ulong devflag);
int     p720_run_diagnostics(struct sc_card_diag *diag_ptr);
int     p720_loop_test_diagnostics(struct sc_card_diag *diag_ptr);
int     p720_diag_arb_select(struct sc_card_diag *diag_ptr,
                        caddr_t iocc_addr, uchar initiator);
int     p720_diag_move_byte_out(struct sc_card_diag *diag_ptr,
                        caddr_t iocc_addr, uchar data, uchar clear_atn);
int     p720_diag_move_byte_in(struct sc_card_diag *diag_ptr,
                        caddr_t iocc_addr, uchar data);
int     p720_register_test(struct sc_card_diag *diag_ptr);
int     p720_pos_register_test();
int     p720_diag_reset_scsi_bus();
int     p720_start_dev(int dev_index);
int     p720_stop_dev(int dev_index);
int     p720_issue_abort(int dev_index);
int     p720_issue_BDR(int dev_index);

/*****************************************************************************/
/*     functions in pscsi720ddb.c                                            */
/*****************************************************************************/

int     p720_chip_register_init(caddr_t eaddr);
void    p720_chip_register_reset(int reset_flag, caddr_t eaddr);
int     p720_config_adapter(caddr_t eaddr);
void    p720_logerr(int errid, int add_halfword_status, int errnum,
           int data1, struct cmd_info *com_ptr, uchar read_regs);
int     p720_read_reg(uint offset, char reg_size, caddr_t eaddr);
int     p720_write_reg(uint offset, char reg_size, int data, caddr_t eaddr);
int     p720_write_reg_disable(uint offset, char reg_size, int data);
int     p720_read_POS(uint offset);
int     p720_write_POS(uint offset, int data);
int     p720_strategy(struct sc_buf *bp);
void    p720_start(struct sc_buf *bp, struct dev_info *dev_ptr);
int     p720_unfreeze_qs(struct dev_info *dev_ptr, struct sc_buf *bp,
			 uchar check_chip, caddr_t eaddr);
void    p720_process_q_clr(struct dev_info *dev_ptr, struct sc_buf *bp);
void    p720_q_full(struct dev_info *dev_ptr, struct cmd_info *com_ptr);
void    p720_freeze_qs(struct dev_info * dev_ptr);
struct sc_buf *p720_process_bp_save(struct dev_info *dev_ptr);
int     p720_alloc_TAG(struct sc_buf *bp);
void    p720_free_TAG(struct cmd_info *com_ptr);
int     p720_alloc_STA(struct cmd_info *com_ptr);
void    p720_free_STA(struct cmd_info *com_ptr);
int     p720_alloc_TCE(struct sc_buf *bp, struct cmd_info *com_ptr);
void    p720_free_TCE(struct cmd_info *com_ptr);
int     p720_alloc_resources(struct sc_buf *bp, struct dev_info *dev_ptr);
int     p720_free_resources(struct cmd_info *com_ptr, uchar copy_required);
int     p720_iodone(struct sc_buf *bp);
void    p720_enq_active(struct dev_info *dev_ptr, struct cmd_info *com);
void    p720_enq_WFR(struct sc_buf *bp);
void    p720_deq_WFR(struct sc_buf *bp, struct sc_buf *prev_bp);
void    p720_enq_wait(struct cmd_info *com);
void    p720_enq_abort_bdr(struct dev_info *dev_ptr);
void    p720_deq_active(struct dev_info *dev_ptr, struct cmd_info *com_ptr);
void    p720_deq_wait(struct cmd_info *com_ptr);
void    p720_deq_abort_bdr(struct dev_info *dev_ptr);
int     p720_dump(dev_t devno, struct uio *uiop, int cmd, int arg,
                 int chan, int ext);
int     p720_dump_intr(struct dev_info *dev_ptr, char abort_chip);
int     p720_dumpstrt();
int     p720_dumpwrt(struct sc_buf *bp);
int     p720_dump_dev(struct sc_buf *bp, struct dev_info *dev_ptr);
struct  cdt *p720_cdt_func(int arg);
void    p720_sel_glitch(struct cmd_info *com_ptr, int save_isr, caddr_t eaddr);
void    p720_fail_cmd(struct dev_info *dev_ptr);
void    p720_delay(int delay);
int     p720_intr(struct intr *handler);
int     p720_issue_cmd(caddr_t eaddr);
void    p720_check_wfr_queue(uchar command_issued_flag, caddr_t eaddr);
void    p720_cleanup_reset(int err_type, caddr_t eaddr);
void    p720_scsi_reset_received(caddr_t eaddr);
void    p720_check_qs(struct dev_info * tmp_dev_ptr);
int     p720_epow(struct intr *handler);
void    p720_watchdog(struct watchdog *w);
void    p720_command_reset_scsi_bus(caddr_t eaddr);
int     p720_verify_tag(struct cmd_info * com_ptr, caddr_t eaddr);
int     p720_fail_free_resources(struct dev_info *dev_ptr, struct sc_buf *bp,
                uchar connected, int cmd_issued, caddr_t eaddr);
int     p720_issue_abort_bdr(struct dev_info *dev_ptr, struct sc_buf *bp,
			uint iodone_flag, uchar connected, caddr_t eaddr);
int     p720_scsi_interrupt(struct cmd_info * com_ptr,
		struct dev_info *dev_ptr,int save_interrupt, 
		int issue_abrt_bdr, caddr_t eaddr);
void    p720_prep_main_script(struct cmd_info *com_ptr, uint *script_vir_addr,
                       uint script_dma_addr);
void    p720_set_disconnect(struct dev_info *dev_ptr, uchar chk_disconnect);
void    p720_patch_tag_changes(struct cmd_info * com_ptr, uchar q_tag_msg);
int     p720_verify_neg_answer(uint *script_vir_addr, struct dev_info *dev_ptr);
void    p720_reset_iowait_jump(uint *iowait_vir_addr,int dev_info_hash,
				uchar all_luns);
void    p720_restore_iowait_jump(uint *iowait_vir_addr,
                            struct dev_info *dev_ptr, uint script_dma_addr);
void    p720_update_dataptr(ulong *script_vir_addr,struct dev_info *dev_ptr,
			    ulong tag, caddr_t eaddr);
int     p720_handle_extended_messages(uint *script_vir_addr,
                        uint *script_dma_addr,
                        struct cmd_info *com_ptr, int interrupt_flag,
		 	caddr_t eaddr);
void    p720_patch_nondefault_sync(uint * target_script, uchar xfer_pd, 
			uchar req_ack);
int     p720_issue_abort_script(uint *script_vir_addr, uint *script_dma_addr,
                        struct cmd_info *com_ptr, int dev_info_hash, 
		 	uchar connected, caddr_t eaddr);
int     p720_issue_bdr_script(uint *script_vir_addr, uint *script_dma_addr,
                             int dev_info_hash, struct cmd_info * com_ptr,
			uchar connected, caddr_t eaddr);
void    p720_trace_1(char *string, int data);
void    p720_trace_2(char *string, int val1, int val2);
void    p720_trace_3(char *string, int data1, int data2, int data3);
#else
/*****************************************************************************/
/*     functions in pscsi720ddt.c                                            */
/*****************************************************************************/

int     p720_config();
int     p720_open();
int     p720_close();
void    p720_fail_open();
int     p720_inquiry();
int     p720_start_unit();
int     p720_test_unit_rdy();
int     p720_readblk();
void    p720_adp_str_init();
int     p720_ioctl();
struct  sc_buf * p720_build_command();
void    p720_script_init();
int     p720_diagnostic();
int     p720_run_diagnostics();
int     p720_loop_test_diagnostics();
int     p720_diag_arb_select();
int     p720_diag_move_byte_out();
int     p720_diag_move_byte_in();
int     p720_register_test();
int     p720_pos_register_test();
int     p720_diag_reset_scsi_bus();
int     p720_start_dev();
int     p720_stop_dev();
int     p720_issue_abort();
int     p720_issue_BDR();
/*****************************************************************************/
/*     functions in pscsi720ddb.c                                            */
/*****************************************************************************/

int     p720_chip_register_init();
void    p720_chip_register_reset();
int     p720_config_adapter();
void    p720_logerr();
int     p720_read_reg();
int     p720_write_reg();
int     p720_write_reg_disable();
int     p720_read_POS();
int     p720_write_POS();
int     p720_strategy();
void    p720_start();
int     p720_unfreeze_qs();
void    p720_process_q_clr();
void    p720_q_full();
void    p720_freeze_qs();
int     p720_alloc_TAG();
void    p720_free_TAG();
int     p720_alloc_STA();
void    p720_free_STA();
int     p720_alloc_TCE();
void    p720_free_TCE();
int     p720_alloc_resources();
int     p720_free_resources();
int     p720_iodone();
void    p720_enq_active();
void    p720_enq_wait();
void    p720_enq_abort_bdr();
void    p720_deq_active();
void    p720_enq_WFR();
void    p720_deq_WFR();
void    p720_deq_wait();
void    p720_deq_abort_bdr();
int     p720_dump();
int     p720_dump_intr();
int     p720_dumpstrt();
int     p720_dumpwrt ();
int     p720_dump_dev();
struct  cdt *p720_cdt_func();
void    p720_sel_glitch();
void    p720_fail_cmd();
void    p720_delay();
int     p720_intr();
int     p720_issue_cmd();
void    p720_check_wfr_queue();
void    p720_cleanup_reset();
void    p720_scsi_reset_received();
void    p720_check_qs();
int     p720_epow();
void    p720_watchdog();
void    p720_command_reset_scsi_bus();
int     p720_verify_tag();
int     p720_fail_free_resources();
void    p720_issue_abort_bdr();
void    p720_scsi_interrupt();
void    p720_prep_main_script();
void    p720_set_disconnect();
void    p720_patch_tag_changes();
int     p720_verify_neg_answer();
void    p720_reset_iowait_jump();
void    p720_restore_iowait_jump();
void    p720_update_dataptr();
int     p720_handle_extended_messages();
void    p720_patch_nondefault_sync();
int     p720_issue_abort_script();
int     p720_issue_bdr_script();
void    p720_trace_1();
void    p720_trace_2();
void    p720_trace_3();
#endif /* not _NO_PROTO */

#endif /* _H_PSCSI720 */

