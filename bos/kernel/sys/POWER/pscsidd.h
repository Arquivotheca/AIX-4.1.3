/* @(#)88	1.14.3.6  src/bos/kernel/sys/POWER/pscsidd.h, sysxscsi, bos411, 9428A410j 3/17/94 18:17:03 */
#ifndef _H_PSCSIDD
#define _H_PSCSIDD
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver Header File
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
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

#define DEFAULT_MIN_PHASE       0x32
#define DEFAULT_BYTE_BUF        0x08
#define SYNC_VAL                0x08
#define REG_FAIL                -1
#define MAXREQUEST    0x40000            /* largest data xfer size:      */
                                         /* =  262144 dec. (256KB)       */
#define MAX_DEVICES     64               /* This stands for a possible of*/
                                         /* 8 SCSI devices, each with 8  */
                                         /* possible LUNs                */
#define MAX_SCRIPTS     56               /* max scripts possible         */
#define TCW_RESERVED    0x77
#define DMA_INIT        MICRO_CHANNEL_DMA/* DMA init flags               */
#define DMA_TYPE        0                /* DMA master/complete flags    */
#define ST_SIZE         256              /* size of a small transfer     */
#define NUM_STA         (PAGESIZE/ST_SIZE)   /* num STAs available*/
#define NUM_4K_TCWS     64
#define TCW_TABLE_SIZE  (NUM_4K_TCWS+1)
#define LARGE_TCW_SIZE  8
#define LARGESIZE       (PAGESIZE*LARGE_TCW_SIZE)
#define LONGWAIT        5                 /* timeout value for abrt/bdrs  */
#define SIOPWAIT        15                /* timeout value for SIOP ABRT  */
#define RESETWAIT       5                 /* timeout value for bus resets */
#define DMA_ERR         1
#define REG_ERR         2
#define PHASE_ERR       3
#define DEVICE_ERR      4
#define DISC_ERR        5
#define HOST_ERR        6

                    /* the ioctl SCSI commands      */
#define PSC_NO_ERR      0x00            /* routine call failed          */
#define PSC_FAILED      0x01            /* routine call failed          */
#define PSC_COPY_ERROR  0x02            /* error returned during xmem   */
#define PSC_DMA_ERROR   0x03            /* error returned during cleanup*/
#define PSC_RESET_CHIP  0x01            /* flag used to cleanup register*/
#define PSC_RESET_DMA   0x02            /* flag used to cleanup DMA     */
#define PSC_COMP_RESET  0x0040002c      /* @ of component reset register*/
#define PSC_COMP_OFF    0xfffffffb      /* mask to reset scsi comp off  */
#define PSC_COMP_ON     0xffffffff      /* mask to reset scsi comp on   */

/* the following macros calculate the bus dma address the adapter uses  */
/* when using the reserved TCW range which was passed in the ddi area.  */
#define DMA_ADDR(start_addr, tcw_num) \
    ((start_addr) + ((tcw_num) * PAGESIZE))
#define DMA_ADDR2(start_addr, tcw_num) \
    ((start_addr) + ((tcw_num) * LARGESIZE))

#define INT_TYPE        BUS_MICRO_CHANNEL /* bus type for interrupt level */
#define INT_FLAGS       0               /* define as sharable interrupt */

/************************************************************************/
/* MACROS                                                               */
/************************************************************************/
/* the following macro is used to quickly generate the device index.    */
/* the macro assumes "a" is SCSI ID, and "b" is LUN                     */
#define INDEX(a,b)    ((((a) & 0x07)<<3) | ((b) & 0x07))

/* this macro returns the scsi id from a previously generated index.    */
#define SID(x)        ((x)>>3)

/* this macro returns the lun id from a previously generated index.     */
#define LUN(x)        ((x) & 0x07)


#define ISSUE_MAIN_TO_DEVICE(script_dma_addr)   \
( psc_write_reg( (uint)DSP, (char) DSP_SIZE, \
                word_reverse(script_dma_addr + Ent_scripts_entry_point) ) )

#define ISSUE_MAIN_AFTER_NEG(script_dma_addr)   \
( psc_write_reg( (uint)DSP, (char) DSP_SIZE, \
                word_reverse(script_dma_addr + Ent_cleanup_phase) ) )

/************************************************************************/
/* Adapter I/O Defines                                                  */
/************************************************************************/
/* Adapter POS register addressing definitions for real SALMON */
#define POS0_SIZE       0x01    /* size in bytes                */
#define POS0            0x00400000  /* RO, card id low        0xF4  */
#define POS1_SIZE       0x01    /* size in bytes                */
#define POS1            0x00400001  /* RO, card id high       0x8F  */
#define POS2_SIZE       0x01    /* size in bytes                */
#define POS2            0x00400002  /* RW, card arb and enable      */
#define POS3_SIZE       0x01    /* size in bytes                */
#define POS3            0x00400003  /* RW, data when 6&7 non-zero   */
#define POS4_SIZE       0x01    /* size in bytes                */
#define POS4            0x00400004  /* RW, card int and nibble ena  */
#define POS5_SIZE       0x01    /* size in bytes                */
#define POS5            0x00400005  /* RW, card status, unused      */
#define POS6_SIZE       0x01    /* size in bytes                */
#define POS6            0x00400006  /* RW, card addr ext low        */

#define POS0_VAL        0xF4            /* Card ID low value            */
#define POS1_VAL        0x8F            /* Card ID high value           */
/* 53C700 register addressing definitions */
#define SCNTL0_SIZE     0x01
#define SCNTL0          0x0080
#define SCNTL1_SIZE     0x01
#define SCNTL1          0x0081
#define SDID_SIZE       0x01
#define SDID            0x0082
#define SIEN_MASK       0xAF    /* mask SCSI interrupts */
#define SIEN_SIZE       0x01
#define SIEN            0x0083
#define SCID_SIZE       0x01
#define SCID            0x0084
#define SXFER_SIZE      0x01
#define SXFER           0x0085

#define SODL_SIZE       0x01
#define SODL            0x0086
#define SOCL_SIZE       0x01
#define SOCL            0x0087
#define SFBR_SIZE       0x01
#define SFBR            0x0088
#define SSIDL_SIZE      0x01
#define SSIDL           0x0089
#define SBDL_SIZE       0x01
#define SBDL            0x008A
#define SBCL_SIZE       0x01
#define SBCL            0x008B
#define DSTAT_SIZE      0x01
#define DSTAT           0x008C
#define    OPC                0x01    /* DSTAT Illegal Instruction        */
#define    WTD                0x02    /* DSTAT Watchdog Timer             */
#define    SIR                0x04    /* DSTAT Scrpt Intrpt Instr. Recvd. */
#define    SSI                0x08    /* DSTAT Script Single Step         */
#define    DABRT              0x10    /* DSTAT Abort occurred             */
#define    DFE                0x80    /* DSTAT DMA FIFO Empty             */
#define SSTAT0_SIZE     0x01
#define SSTAT0          0x008D
#define SSTAT1_SIZE     0x01
#define SSTAT1          0x008E
#define SSTAT2_SIZE     0x01
#define SSTAT2          0x008F
#define CTEST0_SIZE     0x01
#define CTEST0          0x0094
#define CTEST1_SIZE     0x01
#define CTEST1          0x0095
#define CTEST2_SIZE     0x01
#define CTEST2          0x0096
#define CTEST3_SIZE     0x01
#define CTEST3          0x0097
#define CTEST4_SIZE     0x01
#define CTEST4          0x0098
#define CTEST5_SIZE     0x01
#define CTEST5          0x0099
#define CTEST6_SIZE     0x01
#define CTEST6          0x009A
#define CTEST7_SIZE     0x01
#define CTEST7          0x009B
#define TEMP_SIZE       0x04
#define TEMP            0x009C   /* 32 bit stack register 9c-9f */
#define DFIFO_SIZE      0x01
#define DFIFO           0x00A0
#define ISTAT_SIZE      0x01
#define ISTAT           0x00A1
#define    DIP          0x01    /* ISTAT DMA Interrupt Pending  */
#define    SIP          0x02    /* ISTAT SCSI Interrupt Pending */
#define    CONNECTED    0x08    /* Chip currently on scsi bus   */
#define    ABRT         0x80    /* ISTAT Abort Operation        */
#define CONN_INT_TEST   0x0B    /*  (CONNECTED | SIP | DIP)     */
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
#define DMODE_SIZE      0x01
#define DMODE           0x00B4
#define DIEN_MASK       0x15    /* mask DMA interrupts */
#define DIEN_CHK_MASK   0x1F    /* interrupt check     */
#define DIEN_SIZE       0x01
#define DIEN            0x00B9
#define DWT_SIZE        0x01
#define DWT             0x00BA
#define DCNTL_SIZE      0x01
#define DCNTL           0x00BB
#define SCSIRS_SIZE     0x04
#define SCSIRS          0x00BC   /* 32 bit SCSI Reset-Status register bc-bf */
#define SCNTL0_INIT     0xCC	 /* Full arbitration, EPC, EPG */
#define SCNTL1_INIT     0xA0
#define DMODE_INIT      0x80
#define DCNTL_INIT      0x00

/* Miscellaneous defines */
#define MAX_POS_RETRIES     3               /* num retries of POS op        */
#define MAX_RESTART_RETRIES 3           /* num retries of Restart cmd   */
#define MAX_DUMP_LOOPS      10          /* num of loops for dump intr poll */
#define IPL_MAX_SECS        15
#define VPD_SIZE            255                 /* size of adap VPD area (bytes)*/

/************************************************************************/
/* Interrupt Defines                                                    */
/************************************************************************/
#define SCSI_PARITY_ERR               0x01
#define SCSI_RST                      0x02
#define SCSI_UNEXP_DISC               0x04
#define SCSI_GROSS_ERR                0x08
#define SCSI_SEL                      0x10
#define SCSI_SEL_FAIL                 0x20
#define SCSI_COMP                     0x40
#define PHASE_MIS                     0x80

/************************************************************************/
/* Miscellaneous Structures                                             */
/************************************************************************/
struct timer    {
    struct watchdog dog;                /* the watchdog struct          */
#define PSC_SIOP_TMR    1
#define PSC_RESET_TMR   2
#define PSC_COMMAND_TMR 3
#define PSC_RESTART_TMR 4
    uint        timer_id;               /* my internal timer id val     */
                                        /*  1 = adapter cmd timer       */
                                        /*  2 = scsi bus reset timer    */
                                        /*  3 = dev_info cmd timer      */
};

struct small_xfer_area_str {            /* Small Transfer Area Structure */
    char   *sta_ptr;                    /* address of this xfer area    */
    uchar   in_use;                     /* TRUE if this area in use     */
};

struct scripts_struct   {               /* SCRIPTS used to run the SIOP */
    ulong   *script_ptr;                /* pointer to SCRIPTS work area */
    ulong   *dma_ptr;
    uchar   TCW_index;                  /* index into 4K xfer area      */
    uchar   in_use;                     /* TRUE if this area in use     */
};

struct error_log_def {                  /* driver error log structure   */
    struct err_rec0    errhead;         /* error log header info        */
    struct rc    data;                  /* driver dependent err data    */
};

struct psc_cdt_table {                  /* component dump table struct  */
    struct    cdt_head   psc_cdt_head;  /* header to the dump table     */
    struct    cdt_entry  psc_entry[1];  /* space for each minor + trace */
};


/************************************************************************/
/* Structures related to device control                                 */
/************************************************************************/
struct dev_info {
    struct timer    dev_watchdog;   /* watchdog timer for dev struct*/
    uchar           opened;
    uchar           scsi_id;        /* SCSI ID of this device       */
    uchar           lun_id;         /* LUN ID of this device        */
    uchar           negotiate_flag; /* SYNC/ASYNC negotiations      */
    uchar           async_device;   /* if this is defined as async  */
    uchar           restart_in_prog;/* a restart of this device     */
    uchar           disconnect_flag;/* disconnect or no disconnect  */
    uchar           special_xfer_val;
    uchar           agreed_xfer;
    uchar           agreed_req_ack;
    uchar           ioctl_wakeup;   /* wakeup sleeping ioctl call   */
    int             ioctl_event;
    int             ioctl_errno;
    int             stop_event;

    struct dev_info *DEVICE_ACTIVE_fwd;
    struct dev_info *DEVICE_ACTIVE_bkwd;
    struct dev_info *DEVICE_WAITING_fwd;
    struct dev_info *DEVICE_WAITING_FOR_RESOURCES_fwd;
    struct dev_info *ABORT_BDR_fwd;
    struct dev_info *ABORT_BDR_bkwd;

    struct  sc_buf  *head_pend;   /* ptr to pending cmd queue     */
    struct  sc_buf  *tail_pend;   /* ptr to pending cmd queue     */
    struct  sc_buf  *active;      /* ptr to active cmd queue      */

#define ABORT_IN_PROGRESS         3
#define BDR_IN_PROGRESS           4
#define CMD_IN_PROGRESS           5
#define NEGOTIATE_IN_PROGRESS     6
    uchar            cmd_activity_state;
                                 /* what the device structure is doing*/
#define LARGE_TCW_RESOURCES_USED      1
#define SMALL_TCW_RESOURCES_USED      2
#define STA_RESOURCES_USED            3
#define NO_RESOURCES_USED             4
    uchar            resource_state; /* pointer to save area for STA */
#define ACTIVE        0
#define STOPPING      1
#define HALTED        2
    uchar            queue_state;  /* device general queue state   */
                                   /* ACTIVE, STOPPING, or HALTED    */
                                   /* this only represents the     */
                                   /* state of the cmd_active queue*/
#define RETRY_ERROR         0x0001 /* flag used for retry of abort/bdr */
#define PREP_MAIN_COMPLETE  0x0002 /* flag used for prep main check */
#define CAUSED_TIMEOUT      0x0004 /* this device caused a timeout */
#define RESID_SUBTRACT      0x0008 /* signals we have to update resid */
#define SCSI_ABORT          0x0010 /* a scsi abort is active */
#define SCSI_BDR            0x0020 /* a scsi bdr is active for device */
#define SELECT_TIMEOUT      0x0040 /* a selection timeout occurred */
#define NEG_PHASE_2         0x0080 /* used during negotiation */

#define CHK_SCSI_ABDR       0x0030 /* value used to check active cmd */
    ushort          flags;         /* field used to hold the setting of*/
                                   /* flags for the drive.             */
#define PSC_RETRY_COUNT              10
    int             retry_count;    /* times allowed to retry issue */
                                    /* of an abort/bdr command.     */

    uint            script_dma_addr;/* pointer to dma addr of script*/
    int             cmd_script_ptr; /* script table entry pointer   */
    uint            STA_addr;       /* pointer to save area for STA */
    int             STA_index;      /* STA index save area          */
    uint            dma_addr;       /* save area for dma address    */
    int             TCW_index;      /* save area for TCW index      */
    int             TCW_count;      /* TCW count save area          */
    uint            max_disconnect;  /* max value xferred befor disconnect */
    uint            bytes_moved;/* num of bytes moved in last transfer */
};

#define PSC_TRACE_SIZE  0x1000
struct psc_trace_entry {
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

struct psc_trace_struct {
    struct psc_trace_entry  trace_buffer[PSC_TRACE_SIZE];
};

struct adapter_def {
    struct adap_ddi ddi;
    struct timer    adap_watchdog;    /* watchdog timer for adap struct*/
    struct timer    reset_watchdog;   /* watchdog timer for bus_resets */
    struct timer    restart_watchdog; /* watchdog timer for cmd delays */
    uchar           defined;
    uchar           opened;
    uchar           epow_state;             /* power failure flag   */
                                            /*  0 = normal state    */
                                            /*  4 = EPOW pending    */
#define EPOW_PENDING    4                   /* adap EPOW pending    */
    uchar           errlog_enable;          /* set if errors are to */
    uchar           open_mode;              /* mode opened in:      */
#define NORMAL_MODE     0                   /*  normal operation    */
#define DIAG_MODE       1                   /*  diagnostic mode     */
    uchar           iowait_inited;          /* allow 1 iowait init  */
    char            *large_TCW_table;       /* pointer to rsvd tcws */
                                            /*   management table   */
    ulong           large_tcw_start_addr;   /* starting tcw, big xfr*/
    ushort          large_req_begin;        /* where large reqs strt*/
    ushort          large_req_end;          /* where large reqs end */
    ushort          next_large_req;


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

    struct dev_info        *DEVICE_ACTIVE_head;
    struct dev_info        *DEVICE_ACTIVE_tail;
    struct dev_info        *DEVICE_WAITING_head;
    struct dev_info        *DEVICE_WAITING_tail;
    struct dev_info        *DEVICE_WAITING_FOR_RESOURCES_head;
    struct dev_info        *DEVICE_WAITING_FOR_RESOURCES_tail;
    struct dev_info        *ABORT_BDR_head;
    struct dev_info        *ABORT_BDR_tail;

    struct small_xfer_area_str STA[NUM_STA];/* table of the 16 small*/
                        /* transfer areas    */
#define STA_IN_USE      0x01
#define STA_UNUSED      0xFF
    struct scripts_struct SCRIPTS[MAX_SCRIPTS];
#define SCR_IN_USE      0x01
#define SCR_UNUSED      0xFF
                        /* table of pointers to    */
                        /* scripts areas created   */
                        /* for each device hung    */
                        /* on the SCSI bus         */
    int                 num_scripts_created;     /* number of scrs created */
    int                 num_4K_tcws_in_use;      /* number of 4K's used  */
    struct intr         intr_struct;             /* int handler struct   */
    dev_t               devno;                   /* adapter major/minor  */
    uchar               dump_inited;             /* dump init completed  */
    uchar               dump_started;            /* dump start completed */
    int                 max_request;             /* max xfer allowed     */
    int                 channel_id;              /* dma channel id       */
    int                 dump_pri;                /* saved dump int prior.*/
    struct xmem         xmem_STA;                /* local xmem descrip.  */
    struct xmem         xmem_SCR;                /* local xmem descrip.  */

    uchar               TCW_alloc_table[TCW_TABLE_SIZE];
#define TCW_IN_USE      0x01
#define TCW_UNUSED      0xFF
    uchar               STA_tcw;
    uchar               next_STA_req;
    uchar               begin_4K_TCW;
    uchar               ending_4K_TCW;
    uchar               next_4K_req;
    uchar               sync_val;
#define NUM_700_SYNC_RATES 8
    uchar               xfer_max[NUM_700_SYNC_RATES];
#ifdef PSC_TRACE
    struct psc_trace_struct  *trace_ptr;
    int                 current_trace_line;
#endif
};

/**************************************************************************/
/*    BEGIN SCRIPTS ORIGINAL CONTROL CODE FOR COPYING                     */
/**************************************************************************/

#define A_set_special_sync      0x00000002
#define A_change_to_async       0x00000003
#define A_io_wait               0x00000007
#define A_bdr_select_failed     0x0000001B
#define A_bdr_io_complete       0x0000001C
#define A_bdr_msg_error         0x0000001F
#define A_modify_data_ptr       0x00000021
#define A_target_sync_sent      0x00000022

#define ISSUE_NEGOTIATE_TO_DEVICE(script_dma_addr)      \
( psc_write_reg( (uint)DSP, (char) DSP_SIZE, word_reverse(script_dma_addr + Ent_sync_negotiation) ) )
/*
 * THIS COULD BE A SIMPLE MACRO TO WRITE THE SCRIPT ADDRESS OF THE NEGOTIATE
 * SCRIPT INTO THE DSP REGISTER.  ALL THE VALUES NEEDED TO DO THE NEGOTIATE
 * WERE ALREADY LOADED BY psc_script_init.
 */

#define GET_CMD_STATUS_BYTE(script_dma_addr)    \
( script_dma_addr[Ent_status_buf/4] >> 24)
/*
 * THIS GOES TO THE BUFFER AREA OF THE MAIN SCRIPT AND READS BACK THE VALUE
 * BEING HELD IN THE STATUS BUFFER.
 */

#define SET_SXFER(val) (psc_write_reg(SXFER, SXFER_SIZE, val))
/*
 * This macro is used to set the chip for data transfers
 */

typedef unsigned long ULONG;

#define	A_phase_error	0x00000001
#define	A_io_done_after_data	0x00000005
#define	A_io_done	0x00000006
#define	A_io_wait	0x00000007
#define	A_unknown_msg	0x00000009
#define	A_ext_msg	0x0000000A
#define	A_check_next_io	0x0000000B
#define	A_check_next_io_data	0x0000000C
#define	A_cmd_select_atn_failed	0x00000012
#define	A_err_not_ext_msg	0x00000013
#define	A_sync_neg_done	0x00000014
#define	A_unexpected_status	0x00000015
#define	A_sync_msg_reject	0x00000016
#define	A_abort_msg_error	0x00000017
#define	A_neg_select_failed	0x00000018
#define	A_abort_select_failed	0x00000019
#define	A_abort_io_complete	0x0000001A
#define	A_unknown_reselect_id	0x0000001D
#define	A_uninitialized_reselect	0x0000001E
#define	R_target_id	0x00000001
#define	R_cmd_bytes_out_count	0x00000000
#define	R_data_byte_count	0x00000000
#define	R_dummy_int	0x00000000
#define	R_ext_msg_size	0x00000000
#define	Ent_cmd_msg_in_buf	0x00000868
#define	Ent_cmd_buf	0x00000870
#define	Ent_status_buf	0x00000880
#define	Ent_identify_msg_buf	0x00000888
#define	Ent_reject_msg_buf	0x000008C0
#define	Ent_regular_phase_hdlr	0x00000320
#define	Ent_sync_msg_out_buf	0x00000890
#define	Ent_extended_msg_buf	0x000008A0
#define	Ent_sync_msg_out_buf2	0x00000898
#define	Ent_abort_bdr_msg_out_buf	0x000008A8
#define	Ent_abort_bdr_msg_in_buf	0x000008B0
#define	Ent_iowait_entry_point	0x00000000
#define	Ent_reselect_router	0x00000008
#define	Ent_lun_msg_buf	0x000008B8
#define	Ent_scsi_id_0	0x00000058
#define	Ent_scsi_id_1	0x000000A8
#define	Ent_scsi_id_2	0x000000F8
#define	Ent_scsi_id_3	0x00000148
#define	Ent_scsi_id_4	0x00000198
#define	Ent_scsi_id_5	0x000001E8
#define	Ent_scsi_id_6	0x00000238
#define	Ent_scsi_id_7	0x00000288
#define	Ent_scripts_entry_point	0x000002D8
#define	Ent_message_loop	0x000002F0
#define	Ent_status_complete	0x00000370
#define	Ent_send_data	0x000003D0
#define	Ent_after_data_move_check	0x000003D8
#define	Ent_receive_data	0x000003F0
#define	Ent_msg_hdlr	0x00000400
#define	Ent_ext_msg_handler	0x00000488
#define	Ent_disconnect_point	0x00000598
#define	Ent_reject_target_sync	0x000004E8
#define	Ent_status_complete_data	0x000003A0
#define	Ent_disconnect_point_1	0x000005B0
#define	Ent_msg_hdlr_1	0x00000440
#define	Ent_unknown_msg_hdlr	0x00000480
#define	Ent_msg_done	0x00000430
#define	Ent_msg_done_1	0x00000470
#define	Ent_failed_selection_hdlr	0x000005C8
#define	Ent_script_reconnect_point	0x000005D0
#define	Ent_reconnect_continuation	0x000005E0
#define	Ent_renegotiate_sync	0x00000550
#define	Ent_sync_msg_in_rejected	0x00000690
#define	Ent_abort_sequence	0x00000728
#define	Ent_bdr_sequence	0x00000700
#define	Ent_start_abort_msg_out_phase	0x000007A8
#define	Ent_abort2_sequence	0x00000750
#define	Ent_start_abort2_msg_out_phase	0x00000778
#define	Ent_start_bdr_msg_out_phase	0x000007D8
#define	Ent_start_abort_bdr_msg_in_phase	0x00000808
#define	Ent_failed_abort_bdr_selection_hdlr	0x00000830
#define	Ent_abort_bdr_unexpected_status_in	0x00000838
#define	Ent_failed_sync_selection_hdlr	0x000006F8
#define	Ent_sync_negotiation	0x00000608
#define	Ent_start_sync_msg_out	0x00000630
#define	Ent_start_sync_msg_in_phase	0x00000658
#define	Ent_sync_unexpected_status_in	0x000006A0
#define	Ent_sync_phase_hdlr	0x000006D0
#define	Ent_reject_cleanup	0x00000530
#define	Ent_complete_ext_msg	0x000004A0
#define	Ent_cleanup_phase	0x000002F8
#define	Ent_goto_cleanup	0x00000540
#define	Ent_ext_msg_patch	0x000004B0
#define	Ent_send_command	0x00000310

/* DEBUGGING AIDS: */
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
#ifdef PSC_TRACE
#define TRACE_1(A,B)                      {psc_trace_1(A,B);}
#define TRACE_2(A,B,C)                    {psc_trace_2(A,B,C);}
#define TRACE_3(A,B,C,D)                  {psc_trace_3(A,B,C,D);}
#else
#define TRACE_1(A,B)
#define TRACE_2(A,B,C)
#define TRACE_3(A,B,C,D)
#endif

#ifndef    _NO_PROTO
/*****************************************************************************/
/*     functions in pscsiddt.c                                               */
/*****************************************************************************/

int     psc_config(dev_t devno, int op, struct uio *uiop);
int     psc_open(dev_t devno, ulong devflag, int chan, int ext);
int     psc_close(dev_t devno, int offchan);
void    psc_fail_open(int undo_level, int ret_code, dev_t devno);
int     psc_inquiry(dev_t devno, int arg, ulong devflag);
int     psc_start_unit(dev_t devno, int arg, ulong devflag);
int     psc_test_unit_rdy(dev_t devno, int arg, ulong devflag);
int     psc_readblk(dev_t devno, int arg, ulong devflag);
void    psc_adp_str_init();
int     psc_ioctl(dev_t devno, int cmd, int arg, ulong devflag, int chan,
                  int ext);
struct sc_buf * psc_build_command();
void    psc_script_init(uint *iowait_vir_addr, uint *script_vir_addr,
                        int dev_info_hash, uint iowait_dma_addr,
                        uint script_dma_addr);
int     psc_diagnostic(int arg, ulong devflag);
int     psc_run_diagnostics( struct sc_card_diag *diag_ptr);
int     psc_loop_test_diagnostics( struct sc_card_diag *diag_ptr);
int     psc_register_test(struct sc_card_diag *diag_ptr);
int     psc_pos_register_test();
int     psc_diag_reset_scsi_bus();
int     psc_start_dev(int dev_index);
int     psc_stop_dev(int dev_index);
int     psc_issue_abort(int dev_index);
int     psc_issue_BDR(int dev_index);

/*****************************************************************************/
/*     functions in pscsiddb.c                                               */
/*****************************************************************************/

int     psc_chip_register_init();
void    psc_chip_register_reset(int reset_flag);
int     psc_config_adapter();
void    psc_logerr(int errid, int add_halfword_status, int errnum,
           int data1, struct dev_info *dev_ptr, uchar read_regs);
int     psc_read_reg(uint offset, char reg_size);
int     psc_write_reg(uint offset, char reg_size, int data);
int     psc_read_POS(uint offset);
int     psc_write_POS(uint offset, int data);
int     psc_strategy(struct sc_buf *bp);
void    psc_start(struct dev_info *dev_ptr);
int     psc_alloc_STA(struct dev_info *dev_ptr);
void    psc_free_STA(struct dev_info *dev_ptr);
int     psc_alloc_TCW(struct sc_buf *bp, struct dev_info *dev_ptr);
void    psc_free_TCW(struct dev_info *dev_ptr);
int     psc_alloc_resources(struct dev_info *dev_ptr);
int     psc_free_resources(struct dev_info *dev_ptr, uchar copy_required);
int     psc_iodone(struct sc_buf *bp);
void    psc_enq_active(struct dev_info *dev_ptr);
void    psc_enq_wait(struct dev_info *dev_ptr);
void    psc_enq_wait_resources(struct dev_info *dev_ptr);
void    psc_enq_abort_bdr(struct dev_info *dev_ptr);
void    psc_deq_active(struct dev_info *dev_ptr);
void    psc_deq_wait(struct dev_info *dev_ptr);
void    psc_deq_wait_resources(struct dev_info *dev_ptr);
void    psc_deq_abort_bdr(struct dev_info *dev_ptr);
int     psc_dump(dev_t devno, struct uio *uiop, int cmd, int arg,
                 int chan, int ext);
int     psc_dump_intr(struct dev_info *dev_ptr, char abort_chip);
int     psc_dumpstrt(struct sc_buf *bp);
int     psc_dumpwrt(struct sc_buf *bp);
int     psc_dump_dev(struct dev_info *dev_ptr);
struct  cdt *psc_cdt_func(int arg);
void    psc_fail_cmd(struct dev_info *bp, int queue_type);
void    psc_delay(int delay);
int     psc_intr(struct intr *handler);
int     psc_issue_cmd();
void    psc_check_wait_queue(struct dev_info *dev_ptr,
                             uchar issue_command_flag);
void    psc_cleanup_reset(int err_type);
void    psc_cleanup_reset_error();
int     psc_epow(struct intr *handler);
void    psc_command_watchdog(struct watchdog *w);
void    psc_command_reset_scsi_bus();
struct  dev_info *psc_find_devinfo(int predefined_dsp);
int     psc_fail_free_resources(struct dev_info *dev_ptr, struct sc_buf *bp,
                                int err_code);
int     psc_issue_abort_bdr(struct dev_info *dev_ptr, uint iodone_flag);
int     psc_scsi_parity_error(struct dev_info *dev_ptr,int save_interrupt,
                             int issue_abrt_bdr);
void    psc_prep_main_script(uint *iowait_vir_addr, uint *script_vir_addr,
                       struct dev_info *dev_ptr, uint script_dma_addr);
void    psc_patch_async_switch_int(uint *script_vir_addr,
                                   int dev_info_hash);
void    psc_set_disconnect(struct dev_info *dev_ptr, uchar chk_disconnect);
void    psc_patch_iowait_int_on();
void    psc_patch_iowait_int_off();
void    psc_reselect_router();
int     psc_verify_neg_answer(uint *script_vir_addr, struct dev_info *dev_ptr,
                              int dev_info_hash);
void    psc_reset_iowait_jump(uint *iowait_vir_addr,int dev_info_hash);
void    psc_restore_iowait_jump(uint *iowait_vir_addr,
                            struct dev_info *dev_ptr, uint script_dma_addr);
int     psc_update_dataptr(ulong *script_vir_addr,struct dev_info *dev_ptr);
int     psc_handle_extended_messages(uint *script_vir_addr,
                        uint *script_dma_addr, int dev_info_hash,
                        struct dev_info *dev_ptr, int interrupt_flag);
int     psc_issue_abort_script(uint *script_vir_addr, uint *script_dma_addr,
                               struct dev_info *dev_ptr, int dev_info_hash,
                               uchar connected);
int     psc_issue_bdr_script(uint *script_vir_addr, uint *script_dma_addr,
                             int dev_info_hash);
void    psc_trace_1(char *string, int data);
void    psc_trace_2(char *string, int val1, int val2);
void    psc_trace_3(char *string, int data1, int data2, int data3);
#else
/*****************************************************************************/
/*     functions in pscsiddt.c                                               */
/*****************************************************************************/

int     psc_config();
int     psc_open();
int     psc_close();
void    psc_fail_open();
int     psc_inquiry();
int     psc_start_unit();
int     psc_test_unit_rdy();
int     psc_readblk();
void    psc_adp_str_init();
int     psc_ioctl();
struct  sc_buf * psc_build_command();
void    psc_script_init();
int     psc_diagnostic();
int     psc_run_diagnostics();
int     psc_loop_test_diagnostics();
int     psc_register_test();
int     psc_pos_register_test();
int     psc_diag_reset_scsi_bus();
int     psc_start_dev();
int     psc_stop_dev();
int     psc_issue_abort();
int     psc_issue_BDR();
/*****************************************************************************/
/*     functions in pscsiddb.c                                               */
/*****************************************************************************/

int     psc_chip_register_init();
void    psc_chip_register_reset();
int     psc_config_adapter();
void    psc_logerr();
int     psc_read_reg();
int     psc_write_reg();
int     psc_read_POS();
int     psc_write_POS();
int     psc_strategy();
void    psc_start();
int     psc_alloc_STA();
void    psc_free_STA();
int     psc_alloc_TCW();
void    psc_free_TCW();
int     psc_alloc_resources();
int     psc_free_resources();
int     psc_iodone();
void    psc_enq_active();
void    psc_enq_wait();
void    psc_enq_wait_resources();
void    psc_enq_abort_bdr();
void    psc_deq_active();
void    psc_deq_wait();
void    psc_deq_wait_resources();
void    psc_deq_abort_bdr();
int     psc_dump();
int     psc_dump_intr();
int     psc_dumpstrt();
int     psc_dumpwrt ();
int     psc_dump_dev();
struct  cdt *psc_cdt_func();
void    psc_fail_cmd();
void    psc_delay();
int     psc_intr();
int     psc_issue_cmd();
void    psc_check_wait_queue();
void    psc_cleanup_reset();
void    psc_cleanup_reset_error();
int     psc_epow();
void    psc_command_watchdog();
void    psc_command_reset_scsi_bus();
struct  dev_info *psc_find_devinfo();
int     psc_fail_free_resources();
void    psc_issue_abort_bdr();
void    psc_scsi_parity_error();
void    psc_prep_main_script();
void    psc_patch_async_switch_int();
void    psc_set_disconnect();
void    psc_patch_iowait_int_on();
void    psc_patch_iowait_int_off();
void    psc_reselect_router();
int     psc_verify_neg_answer();
void    psc_reset_iowait_jump();
void    psc_restore_iowait_jump();
int     psc_update_dataptr();
int     psc_handle_extended_messages();
int     psc_issue_abort_script();
int     psc_issue_bdr_script();
void    psc_trace_1();
void    psc_trace_2();
void    psc_trace_3();
#endif /* not _NO_PROTO */

#endif /* _H_PSCSIDD */

