/* @(#)32	1.26.1.3  src/bos/kernext/tokdiag/tokdslo.h, diagddtok, bos411, 9428A410j 6/13/93 12:43:27 */

/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS: tokdslo.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include <stddef.h>

#ifndef _H_TOKDSLO
#define _H_TOKDSLO

/*----------------------------------------------------------------------*/
/* 			fixed storage area				*/
/*----------------------------------------------------------------------*/
#define TOK_MAX_MINOR   16       /* Max. minor number for Token-Ring */

typedef volatile struct {
   int       initialized;          /* device driver initialized?     */
   int       num_dds;              /* number of dds's currently have */
   dds_t *p_dds[TOK_MAX_MINOR];   /* dds pointers for each adapter   */
   int       num_opens;            /* number of open's currently have */
} dd_ctrl_t;

typedef volatile struct {
   ulong nexthole;                 /* index into table for next add  */
   ulong table[TRACE_TABLE_SIZE];  /* ring buffer of trace data      */
} tracetable_t;

/* enough for trace table and, for each adap, the dds, open structs, and ds  */
#define MAX_CDT_ELEMS (1 + (MAX_ADAPTERS * (1 + MAX_OPENS + MAX_CDT)))

typedef struct {
   struct cdt_head  header;
   struct cdt_entry entry[MAX_CDT_ELEMS];
} cdt_t;

/*-----------------------------------------------------------------*/
/*                     ACA control #defines                        */
/*-----------------------------------------------------------------*/
#define NTCW       63              /* # of available TCWs */
#define RCV_TCW    0x10            /* # of TCWs to reserve for RCV bufs */

#define TX_TCW     ((NTCW-2) - RCV_TCW)   /*
                                           * # of TCWs for TX bufs.
                                           * This is used in the calculation
                                           * of the Transmit bus address
                                           * offset.  It is also used in
                                           * the call to reg_init() in
                                           * TX setup.
                                           */

#define RECV_AREA_OFFSET       0        /*
                                         * Offset where the receive
                                         * buffers bus memory addresses
                                         * will start.  This is needed for
                                         * the call to reg_init() during
                                         * receive setup.
                                         */

#define TX_AREA_OFFSET   (RCV_TCW << DMA_L2PSIZE)
                                  /*
                                   *   Offset where the buffers for
                                   *   transmission bus memory addresses
                                   *   will start.  This is needed for the
                                   *   call to reg_init() during TX setup.
                                   */

#define ACA_SIZE           0x40   /*  Adapter Control Area Size
                                   *  It can hold 0x40 entries that
                                   *  are each 0x40 bytes in size.
                                   *  The ACA takes up 1 TCW.
                                   */
                                   /* SCB location in ACA */
#define ACA_SCB_BASE       (0x00 * IOCC_CACHE_SIZE)

                                   /* SSB location in ACA */
#define ACA_SSB_BASE       (0x01 * IOCC_CACHE_SIZE)

                                   /* Product ID location in ACA */
#define ACA_PROD_ID_BASE   (0x02 * IOCC_CACHE_SIZE)

                                   /* Adapter Error Log location in ACA */
#define ACA_ADAP_ERR_LOG_BASE  (0x03 * IOCC_CACHE_SIZE)

                                   /* Ring Information location in ACA */
#define ACA_RING_INFO_BASE     (0x04 * IOCC_CACHE_SIZE)

                                   /* Adapter Open Parameter block */
#define ACA_OPEN_BLOCK_BASE    (0x06 * IOCC_CACHE_SIZE)

                                   /* Receive Chain Start location in ACA */
#define ACA_RCV_CHAIN_BASE     (0x07 * IOCC_CACHE_SIZE)

                                   /* Transmit Chain Start location in ACA */
#define ACA_TX_CHAIN_BASE      (ACA_RCV_CHAIN_BASE +   \
                                    (RCV_CHAIN_SIZE * IOCC_CACHE_SIZE) )


#define P_A_UCODE_LEVEL        0x0202     /*
                                           *   Location of the pointer
                                           *   to the microcode level
                                           */

#define P_A_ADDRS              0x0204     /*
                                           *   Location of the pointer
                                           *   to the adapter addresses
                                           */

#define P_A_PARMS              0x0206     /*
                                           *   Location of the pointer
                                           *   to the adapter parameters
                                           */

#define P_A_MAC_BUF            0x0208     /*
                                           *   Location of the pointer
                                           *   to the MAC Buffer
                                           */



/*----------------------------------------------------------------------*/
/*			 Constant Defines				*/
/*----------------------------------------------------------------------*/
#define TOKEN_L         0xc8            /* low byte of CARDID   */
#define TOKEN_H         0x8f            /* low byte of CARDID   */

/* POS Register bits */

#define CARD_ENABLE        0x01
#define SFDBKRTN           0x80        /* 1 = Monitor SFDBKRTN */
#define MC_PARITY_OFF      0xbf        /* 0 = parity not enabled */
#define MC_PARITY_ON       0x40        /* 1 = parity enabled */
#define MC_PREEMPT_TIME    0x20        /* 1 = fr delay 6.5 micro secs */
#define MC_FAIRNESS        0x10        /* 1 = Fairness not employed */
#define STREAM_DATA        0x20        /* Enable Streaming Data */
#define MC_ARBITRATION     0x10        /* 1 = Disable MC Arbitration */
#define DMA_RCVRY          0x4         /* Include in System DMA recovery */
#define RING_SPEED_BIT     0x2         /* 1 = 16M, 0 = 4M */
#define EARLY_TOKEN_REL    0x0         /* Early Token Release
                                       *   0 = ETR
                                       *   1 = No ETR
                                       */

#define DF_ADAP_INIT_OPTIONS   0xf700  /* adapter initialization options */
#define DF_RCV_BURST_SIZE      0x80    /* default rcv burst size */
#define DF_TX_BURST_SIZE       0x80    /* default TX burst size */
#define DF_DMA_ABORT_THRESH    0x0a0a  /* default DMA abort threshold */

#define ADAP_BUF_SIZE_512      0x200   /* 512 byte adapter buffer size */
#define ADAP_4M_TX_BUF_MIN_CNT_512      0x09
#define ADAP_4M_TX_BUF_MAX_CNT_512      0x09
#define ADAP_16M_TX_BUF_MIN_CNT_512     0x24
#define ADAP_16M_TX_BUF_MAX_CNT_512     0x24

#define ADAP_BUF_SIZE_1K       0x400   /* 1024 byte adapter buffer size */
#define ADAP_4M_TX_BUF_MIN_CNT_1K      0x05
#define ADAP_4M_TX_BUF_MAX_CNT_1K      0x05
#define ADAP_16M_TX_BUF_MIN_CNT_1K     0x12
#define ADAP_16M_TX_BUF_MAX_CNT_1K     0x12


/*----------------------------------------------------------------------*/
/* 		Token-Ring Interrupt Code definitions			*/
/*----------------------------------------------------------------------*/
#define TOK_INT_SYSTEM     0x0080      /* Adapter has interrupt for system */
#define MC_ERR             0x0040      /* Micro-Channel Error Interrupt */
#define TOK_IC             0x000e      /* All 3 bits of Interrupt Code set */
#define ADAPTER_CHK        0x0000      /* Adapter Check Interrupt */
#define IMPL_FORCE         0x0002      /* IMPL Force MAC Frame int. code */
#define RING_STATUS        0x0004      /* Ring Status int. code */
#define SCB_CLEAR          0x0006      /* SCB Clear interrupt code */
#define CMD_STATUS         0x0008      /* Command Status int. code */
#define RECEIVE_STATUS     0x000a      /* Receive interrupt code */
#define TRANSMIT_STATUS    0x000c      /* Transmit int. code */
#define ADAPTER_CHK_ADDR   0x05e0      /* Address for getting Adap Chk info */
                                       /* When an adap. chk occurs, the */
                                       /* Adapter  address register is set */
                                       /* this value. After setting the */
                                       /* address register, the dh. will */
                                       /* read 8 bytes for the adapter */
                                       /* check reason. */

#define ACK_INT            0xa000      /* Free SSB and acknowledge int. */
                                       /* This value will interrupt the */
                                       /* adapter & tell it that the SSB is */
                                       /* available for the adapter to post */
                                       /* additional status */

/*---------------------------------------------------------------------*/
/*         ----------- Adapter Command Literals----------              */
/*---------------------------------------------------------------------*/
#define EXECUTE         0x9080    /* Execute SCB Cmd -Byte Swapped */
#define SCBINT          0x8880    /* Interrupt when SCB free */
#define RCV_VALID       0x8280    /* Receive  valid -Byte Swapped */
#define TX_VALID        0x8180    /* Transmit valid  _Byte Swapped */
#define REJECT_CMD      0x0002    /* Reject Command */
#define ADAP_OPEN_CMD   0x0003    /* Open command for SCB */
#define ADAP_XMIT_CMD   0x0004    /* Transmit  command for SCB */
#define ADAP_XMIT_HALT  0x0005     /* Transmit Halt command for SCB */
#define ADAP_RCV_CMD    0x0006    /* Receive  command for SCB */
#define ADAP_CLOSE_CMD  0x0007    /* Close Command For SCB */
#define ADAP_GROUP_CMD  0x0008    /* Set Group Address Cmd For SCB */
#define ADAP_FUNCT_CMD  0x0009    /* Set Functional Addr Cmd For SCB */
#define ADAP_ELOG_CMD   0x000a    /* Read Error Log Cmd For SCB */
#define ADAP_READ_CMD   0x000b    /* Read Adapter Command For SCB */
#define ADAP_IMPL_EN    0x000c    /* IMPL Enable Cmd for SCB */
#define ADAP_TRACE      0x000d    /* Start/Stop Adap. Trace Cmd for SCB */
#define START_TRACE     0x00010000      /* Start Trace subcommand */
#define STOP_TRACE      0x00000000      /* Stop Trace subcommand */

/*---------------------------------------------------------------------*/
/*                       Literals used in Microcode Download           */
/*---------------------------------------------------------------------*/
#define MC_OK                  0               /* Download uCode OK    */
#define MC_BAD                 -1              /* Download uCode Bad   */

#define MAX_RETRYS             0x0003
#define DL_LOADER_BEGIN        0x00AA
#define ADAP_RESET             0x00
#define DL_LDR_CMD1            0xE600
#define DL_LOADER_IMAGE        0x0010
#define DL_LDR_CMD2            0x0000
#define DL_INIT_ADDR           0x07F0          /* DL Init. Para. addr */
#define DL_PROC_INIT           0x2000          /* DL Process Init. Para. cmd */
#define DL_IPARA_SIZE          5               /* # of 2-byte elements of */
                                               /* down load initialization */
                                               /* parameters */

#define DL_MICROCODE           0x0004
#define MC_DOWNLOAD            0x0100
#define MC_RECORD_SIZE         80              /* MC record size */
#define MC_WR_PROG_SEG         0x2000          /* Write Program Segment cmd */
#define MC_LOAD_CHK            0x0200          /* Check Microcode Loaded cmd */
#define DL_EXIT                0x000F
#define DL_EXE_LDR_PRG         0xc500          /* Execute Loader Program Cmd */
#define DL_OK_EXE_LDR_PRG      0x0010          /* Good return code from      */
                                               /* loader program execution */

#define DMA_OPT                0x0080          /* Parity chk on MMIO/DMA operations */
#define MC_DMA_OK              0x3000          /* Good MC record DMA */
#define DL_DELAY_REG	       0xE0	       /* hardware delay register */


/*---------------------------------------------------------------------*/
/*                 Literals for Adapter Bringup                        */
/*---------------------------------------------------------------------*/
#define FIVE_HUNDRED_MS        (500)           /* 500 ms */
#define HUNDRED_THOU           (100000)         /* 100,000 constant */

#define RESET_SPIN_TIME        10      /*
                                        * 10xFIVE_HUNDRED_MS =
                                        *       5 second
                                        * total reset spin time
                                        */

#define RESET_SPIN_DELTA       (FIVE_HUNDRED_MS)   /* 500 millisec */

#define ADAP_INIT_SPIN_TIME    10      /*
                                        * 10xFIVE_HUNDRED_MS =
                                        *       5 second
                                        * total Timeout for adap
                                        * initialization
                                        */

#define ADAP_INIT_SPIN_DELTA   (FIVE_HUNDRED_MS)   /*
                                               *   500 millisec delta
                                               *   for adap
                                               *   initialization.
                                               */

#define OPEN_TIMEOUT           (FIVE_HUNDRED_MS * 120)
                                      /*
                                       *  60 sec. Timeout value for
                                       *  opening adap
                                       */
#define CLOSE_TIMEOUT        (FIVE_HUNDRED_MS * 12)
                                       /*
                                        * Timeout value for closing adap
                                        * 6 sec. timeout
                                        */

#define RESET_MAX_RETRY        4       /* Max # of times to retry reset */
#define ADAP_INIT_RETRY        4       /* Max retry for adap init */
#define OPEN_MAX_RETRY         4       /* Max # of times to retry open */


#define RESET_OK               0x0040  /* Reset of adapter was successful */
#define RESET_FAIL             0x0030  /* Reset of adapter failed */

#define OPEN_PHASE_ERROR     0x0200

#define RCV_MODE               1       /* Adapter in receive mode */
#define NOT_RCV_MODE           -1      /* adapter not in receive mode */
#define RCV_PENDING            10      /* Adapter receive command pending */
#define RCV_SPIN_TIME          5       /* Timeout value for rcv command */
#define RECEIVE_SUSPENDED      0x4000  /* Adapter has suspended reception */
                                       /* of data */

#define INIT_ADAP_OK           0x0000  /* Initialization of Adap. succeeded */
#define INIT_ADAP_FAIL         0x0010
#define MC_ERROR               0x0080  /* Micro Channel error during adapter */
                                       /* initialization */

/*---------------------------------------------------------------------*/
/*                        Limbo State Flags                            */
/*---------------------------------------------------------------------*/

#define PARADISE           0xa4400001   /* All is well */

#define CHAOS              0xa4400002 	/* Trying to re-activate adapter */

#define PROBATION          0xa4400003 	/* Adapter activated, but still 
					 * may fail
					 */

#define LIMBO_RECV         0xa4400004 	/* Adapter activated, 
					 * issue Receive CMD 
					 */

#define LIMBO_FUNCTIONAL   0xa4400005 	/* Functional addr. SCB 
					 * command pending 
					 */

#define LIMBO_GROUP        0xa4400006 	/* Group addr. SCB command pending */

#define LIMBO_ATRACE       0xa4400007 	/* Adapter Trace SCB command pending */

#define LIMBO_KILL_PEND    0xa4400008 	/* Kill of Limbo is pending */

#define NO_OP_STATE        0xa440ffff   /* Fatal error has occured */


/*---------------------------------------------------------------------*/
/*                 Adapter State flags for Adapter Bringup             */
/*---------------------------------------------------------------------*/
#define DEAD_STATE              0x0acadead     /*  adapter is dead
                                                *  no ACA is available
                                                *  while in this state
                                                */
#define NULL_STATE              0x0acaffff     /* The ACA has been
                                                * allocated, but
                                                * has not been
                                                * d_mastered()
                                                */

       /*
        *  In the following states the ACA is available.
        */


#define RESET_PHASE0           0x0aca0010	/* adapter reset */
#define RESET_PHASE1           0x0aca0011
#define RESET_PHASE2           0x0aca0012
#define ADAP_INIT_PHASE0       0x0aca0020	/* adapter initialization */
#define ADAP_INIT_PHASE1       0x0aca0021
#define OPEN_PHASE0            0x0aca0030	/* adapter open */
#define OPEN_PENDING           0x0aca0031    /* Adapter open cmd is pending */
#define OPEN_STATE             0x0aca0032    /* Adapter is open */
#define CLOSE_PHASE0           0x0aca0001
#define CLOSE_PENDING          0x0aca0002    /* Adapter close cmd is pending */
#define CLOSED_STATE           0x0aca0000    /* Adapter is closed */
#define ADAP_KILL_PENDING      0x0aca0040    /* De-activation of the
					      * adapter is pending 
					      */
#define DOWNLOADING            0x0aca0dac    /* Download of adapter
                                              * microcode is in progress
                                              */
	/*
	 * Flags for when adapter is killed by the SLIH after
	 * a Micro Channel error.
	 */
#define ALIVE_N_WELL	0x00000000	/* All is well */
#define DEAD_ON_ARRIVAL	0xdeadbeef	/* Adapter was killed by SLIH */

#define OFLV_ADAP_RESET        0x0001
#define OFLV_ADAP_INIT         0x0002
#define OFLV_ADAP_OPEN         0x0003
#define OFLV_ADAP_CLOSE        0x0004

#define IOCTL_FUNC             0x0001
#define IOCTL_GROUP            0x0002
#define IOCTL_ATRACE           0x0003
#define IOCTL_RING_INFO	       0x0004
  /*
   *   Limbo Mode Timeout values and thresholds
   */
#define SOFT_LIMBO_ABORT   0x0fffffff
#define HARD_LIMBO_ABORT   0x0fffffff
#define TOIL_MAX           0x0400
#define PENALTY_BOX_TIME   (FIVE_HUNDRED_MS * 22)           /* 11 sec. timer */
#define RECV_LIMBO_TIMEOUT      (FIVE_HUNDRED_MS * 2)       /* 1 sec. timer */
#define FUNCTIONAL_ADDR_TIMEOUT    (FIVE_HUNDRED_MS * 2)    /* 1 sec. timer */
#define GROUP_ADDR_TIMEOUT (FIVE_HUNDRED_MS * 2)            /* 1 sec. timer */
#define ATRACE_TIMEOUT     (FIVE_HUNDRED_MS * 2)            /* 1 sec. timer */
#define RING_INFO_TIMEOUT 	(FIVE_HUNDRED_MS * 2) 	    /* 1 sec. timer */


  /*
   *   Limbo Mode Timeout safegaurds
   */
#define OFLV_PROBATION         0x0005
#define OFLV_FUNC_TIMEOUT      0x0006
#define OFLV_GROUP_TIMEOUT     0x0007
#define OFLV_ATRACE_TIMEOUT    0x0008
#define OFLV_RECV_LIMBO_TIMEOUT    0x0009

  /*
   *   Rings Status interrupt bit definitions
   */

#define RS_SIGNAL_LOSS     0x8000
#define RS_HARD_ERROR      0x4000
#define RS_SOFT_ERROR      0x2000
#define RS_TX_BEACON       0x1000
#define RS_LOBE_WIRE_FAULT 0x0800
#define RS_AUTO_REMOVE_1   0x0400
#define RS_REMOVE_RCVD     0x0100
#define RS_COUNTER_OVERFLW 0x0080
#define RS_SINGLE_STATION  0x0040
#define RS_RING_RECOVERY   0x0020

#define TOK_RS_BEACON2_THRESH	40
#define TOK_RS_ESERR_THRESH	80

/*
 *   Limbo Mode entry conditions for Ring Status
 *   and Command Reject Status
 *
 *	Ring Status:
 *		Lobe Wire Fault
 *		Auto-Removal Error 1
 *	
 *	Cmd Reject Status:
 *		Illegal Cmd
 *		Address Error
 *		Adapter Open
 *		Adapter Closed
 *		Same Cmd
 */

#define RS_ENTER_LIMBO     0x0c00
#define CRS_ENTER_LIMBO    0xf800


/*-------------------------------------------------------------*/
/*             Flags used in processing Transmit requests      */
/*-------------------------------------------------------------*/
#define TX_GATHER  0x8000      /*
                               *   Flag to indicate to the adapter
                               *   that a gather operation is to be
                               *   performed.
                               */

#define TX_START_OF_FRAME      0x2000
#define TX_VALID_CHAIN_EL      0x8000
#define TX_END_OF_FRAME        0x1000
#define TX_FRAME_INTERRUPT     0x0800

#define TX_PENDING             0xb800  /* TX List Element is awaiting */
                                       /* Transmission by the adapter */
#define LIST_ERROR             0x2000  /* This TX Element had a list error */
                                       /* when the adapter was */
                                       /* trying to transmit it. When */
                                       /* this error is detected, another */
                                       /* transmit command must be issued */
                                       /* to the adapter via the SCB */

#define TX_CMD_COMPLETE        0x8000  /*
                                       *   The current adapter TX command
                                       *   has completed. When this bit is
                                       *   set in the TX CSTAT, another adapter
                                       *   TX command must be issued for the
                                       *   adapter to continue transmitting
                                       *   data.
                                       */
#define TX_ERROR   0x0400              /*
                                       *   The Transmit CSTAT contains
                                       *   indicates an error on transmission
                                       *   The error could have occured
                                       *   anywhere in transmission.
                                       *   This flag will used to determine
                                       *   if the TX error counter should
                                       *   be incremented.
                                       */
#define TX_CHAIN_FULL          0x000f  /* The transmit list chain if full */
#define NO_TCW_AVAIL           0x000e  /* No TCW/bus address space */
                                       /* available */
#define TX_Q_EMPTY             0x000d  /* The transmit que is empty */

#define TX_ODD_ADDR    0x0020
#define TX_SOF         0x0010


/* -------------------------------------------------------------------- */
/*                      Footprint Failure Codes				*/
/* -------------------------------------------------------------------- */
#define FOOT_TX                 0x01000000      /* Transmit routines */
#define FOOT_RCV                0x02000000      /* Receive routines */
#define FOOT_LMB                0x03000000      /* Limbo routines */
#define FOOT_SLIH               0x04000000      /* Slih */
#define FOOT_OFLV               0x05000000      /* Off-Level routines */
#define FOOT_TIME               0x06000000      /* Timer functions */
#define FOOT_ACT                0x07000000      /* Activation routines */
#define FOOT_IOCTLS             0x08000000      /* Ioclt routines */
#define FOOT_CFG                0x09000000      /* Configuration routines */
#define FOOT_DOWN		0x0a000000	/* Download ucode routines */
#define FOOT_PIO		0x0b000000	/* Programmed I/O routines */

        /*
         * Transmit footprint codes
         */
#define TX_TXOP_0               0x01010000  /* tx_oflv_pro() */
#define TX_TXOP_1               0x01010001  /* tx_oflv_pro() */
#define TX_TXOP_2               0x01010002  /* tx_oflv_pro() */
#define TX_TXOP_3               0x01010003  /* tx_oflv_pro() */
#define TX_TXOP_4               0x01010004  /* tx_oflv_pro() */
#define TX_TXOP_5               0x01010005  /* tx_oflv_pro() */
#define TX_TXOP_6               0x01010006  /* tx_oflv_pro() */
#define TX_MOVE_LIST_0		0x01020000  /* move_tx_list() */
#define TX_CHN_UNDO_0           0x01030000  /* tx_chain_undo() */


        /*
         * Receive footprint codes
         */
#define RCV_TRCV_0              0x02010000  /* tok_receive() */
#define RCV_TRCV_1              0x02010001  /* tok_receive() */
#define RCV_TRF_0               0x02020000  /* tok_recv_frame () */
#define RCV_TRF_1               0x02020001  /* tok_recv_frame () */
#define RCV_TRF_2               0x02020002  /* tok_recv_frame () */
#define RCV_LRC_0               0x02030000  /* load_recv_chain () */
#define RCV_RRL_0               0x02040000  /* read_recv_list () */
#define RCV_CRL_0               0x02050000  /* clear_recv_list () */


        /*
         * Limbo footprint codes
         */
#define LMB_ENTER_0             0x03010000  /* enter_limbo() */
#define LMB_CYCLE_0             0x03020000  /* cycle_limbo() */
#define LMB_CYCLE_1             0x03020001  /* cycle_limbo() */
#define LMB_CYCLE_2             0x03020002  /* cycle_limbo() */
#define LMB_CYCLE_3             0x03020003  /* cycle_limbo() */
#define LMB_CYCLE_4             0x03020004  /* cycle_limbo() */
#define LMB_CYCLE_5             0x03020005  /* cycle_limbo() */
#define LMB_CYCLE_6		0x03020006  /* cycle_limbo() */
#define LMB_KILL_0              0x03030000  /* kill_limbo() */
#define LMB_KILL_1              0x03030001  /* kill_limbo() */
#define LMB_KILL_2              0x03030002  /* kill_limbo() */
#define LMB_KILL_3              0x03030003  /* kill_limbo() */
#define LMB_EGRESS_0            0x03040000  /* egress_limbo() */
#define LMB_BUG_0               0x03050000  /* bug_out() */
#define LMB_BUG_1               0x03050001  /* bug_out() */
#define LMB_BUG_2               0x03050002  /* bug_out() */
#define LMB_BUG_3               0x03050003  /* bug_out() */
#define LMB_BUG_4               0x03050004  /* bug_out() */
#define LMB_BUG_5               0x03050005  /* bug_out() */
#define LMB_BUG_6               0x03050006  /* bug_out() */
#define LMB_BUG_7               0x03050007  /* bug_out() */
#define LMB_BUG_8               0x03050008  /* bug_out() */

        /*
         * SLIH Footprint codes
         */
#define SLIH_0          0x04010000
#define SLIH_1          0x04010001
#define SLIH_2          0x04010002
#define SLIH_3          0x04010003
#define SLIH_4          0x04010004
#define SLIH_5          0x04010005
#define SLIH_6          0x04010006
#define SLIH_7          0x04010007
#define SLIH_8          0x04010008
#define SLIH_9          0x04010009

        /*
         * Off-Level footprint codes
         */
#define OFLV_CMD_0      0x05010000        /* cmd_pro() */
#define OFLV_CMD_1      0x05010001        /* cmd_pro() */
#define OFLV_ADAP_CHK   0x05020000        /* adap_chk() */
#define OFLV_IOCTL_0    0x05030000        /* oflv_ioctl_to */
#define OFLV_IOCTL_1    0x05030001        /* oflv_ioctl_to */
#define OFLV_IOCTL_2    0x05030002        /* oflv_ioctl_to */
#define OFLV_IOCTL_3    0x05030003        /* oflv_ioctl_to */
#define OFLV_0          0x05040000        /* tokoflv() */
#define OFLV_1          0x05040001        /* tokoflv() */
#define OFLV_RSP_0      0x05050000        /* ring_stat_pro() */
#define OFLV_RSP_1      0x05050001        /* ring_stat_pro() */
#define OFLV_RSP_2      0x05050002        /* ring_stat_pro() */
#define OFLV_RSP_3      0x05050003        /* ring_stat_pro() */
#define OFLV_RSP_4      0x05050004        /* ring_stat_pro() */


        /*
         * Timer footprint codes
         */
#define TIME_0             0x06010000  /* recv_mbuf_timer() */

        /*
         * Activation Footprint codes
         */
#define ACT_RESET_ADAP          0x07010000      /* reset_adap() */
#define ACT_RESET_PHS2_0        0x07020000      /* reset_phase2 */
#define ACT_RESET_PHS2_1        0x07020001
#define ACT_RESET_PHS2_2        0x07020002
#define ACT_RESET_PHS2_3        0x07020003
#define ACT_INIT_0              0x07030000      /* init_adap() */
#define ACT_IPHS0_0             0x07040000      /* init_adap_phase0 */
#define ACT_IPHS0_1             0x07040001      /* init_adap_phase0 */
#define ACT_IPHS0_2             0x07040002      /* init_adap_phase0 */
#define ACT_IPHS1_0             0x07050000      /* init_adap_phase1 */
#define ACT_IPHS1_1             0x07050001      /* init_adap_phase1 */
#define ACT_IPHS1_2             0x07050002      /* init_adap_phase1 */
#define ACT_IPHS1_3             0x07050003      /* init_adap_phase1 */
#define ACT_IPHS1_4             0x07050004      /* init_adap_phase1 */
#define ACT_IPHS1_5             0x07050005      /* init_adap_phase1 */
#define ACT_OPEN_0              0x07060000      /* open_adap() */
#define ACT_OA_PHS0_0           0x07070000      /* open_adap_phase0 */
#define ACT_OA_PEND_0           0x07080000      /* open_adap_pend */
#define ACT_OA_PEND_1           0x07080001      /* open_adap_pend */
#define ACT_OA_PEND_2           0x07080002      /* open_adap_pend */
#define ACT_OA_PEND_3           0x07080003      /* open_adap_pend */
#define ACT_OA_TO_0             0x07090000      /* open_adap_timeout */
#define ACT_OA_TO_1             0x07090001      /* open_adap_timeout */
#define ACT_OA_TO_2             0x07090002      /* open_adap_timeout */
#define ACT_OFLV_BUP_0          0x070a0000      /* oflv_bringup() */
#define ACT_OFLV_BUP_1          0x070a0001      /* oflv_bringup() */
#define ACT_OFLV_BUP_2          0x070a0002      /* oflv_bringup() */
#define ACT_OFLV_BUP_3          0x070a0003      /* oflv_bringup() */
#define ACT_OFLV_BUP_4          0x070a0004      /* oflv_bringup() */
#define ACT_OFLV_BUP_5          0x070a0005      /* oflv_bringup() */
#define ACT_OFLV_BUP_6          0x070a0006      /* oflv_bringup() */
#define ACT_OFLV_BUP_7          0x070a0007      /* oflv_bringup() */
#define ACT_OFLV_BUP_8          0x070a0008      /* oflv_bringup() */
#define TOK_XMALL_FAIL 		0x070b0001	/* x_malloc failed */
#define TOK_XMATT_FAIL		0x070b0002	/* x_memattach failed */
#define TOK_XMDET_FAIL		0x070b0003	/* x_memdetattch failed */
#define D_MASTER_FAIL		0x070b0004 	/* d_master failed */
#define D_CLEAR_FAIL		0x070b0005	/* d_clear failed */
#define D_COMPLETE_FAIL		0x070b0006	/* d_complete failed */
#define D_INIT_FAIL		0x070b0007	/* d_init failed */  
#define D_UNMASK_FAIL		0x070b0008	/* d_umask failed */
#define ACT_DEACT_0		0x070c0000	/* ds_deact() */
#define ACT_DEACT_1		0x070c0001	/* ds_deact() */
#define ACT_DEACT_2		0x070c0002	/* ds_deact() */
#define ACT_DEACT_3		0x070c0003	/* ds_deact() */


        /*
         * Micorcode Download footprint codes
         */
#define DOWN_IOCTL_0             0x0a010000  /* tokdnld() */
#define DOWN_IOCTL_1             0x0a010001  /* tokdnld() */
#define DOWN_IOCTL_2             0x0a010002  /* tokdnld() */
#define DOWN_IOCTL_3             0x0a010003  /* tokdnld() */
#define DOWN_LDR_IMAGE_0         0x0a020000  /* dl_ldr_image() */
#define DOWN_INIT_MC_0           0x0a030000  /* dl_init_mc() */
#define DOWN_INIT_MC_1           0x0a030001  /* dl_init_mc() */
#define DOWN_INIT_MC_2           0x0a030002  /* dl_init_mc() */
#define DOWN_INIT_MC_3           0x0a030003  /* dl_init_mc() */
#define DOWN_INIT_MC_4           0x0a030004  /* dl_init_mc() */
#define DOWN_INIT_MC_5           0x0a030005  /* dl_init_mc() */
#define DOWN_INIT_MC_6           0x0a030006  /* dl_init_mc() */
#define DOWN_INIT_MC_7           0x0a030007  /* dl_init_mc() */
#define DOWN_INIT_MC_8           0x0a030008  /* dl_init_mc() */
#define DOWN_INIT_MC_9           0x0a030009  /* dl_init_mc() */
#define DOWN_INIT_MC_A           0x0a03000a  /* dl_init_mc() */
#define DOWN_INIT_MC_B           0x0a03000b  /* dl_init_mc() */


        /*
         * PIO footprint codes
         */
#define PIO_READ_0             0x0b010000  /*  pio_read() */
#define PIO_READ_1             0x0b010001  /*  pio_read() */
#define PIO_WRITE_0            0x0b020000  /*  pio_write() */
#define PIO_WRITE_1            0x0b020001  /*  pio_write() */


#define GET_MEM_DINIT           0xf001
#define CLOSE_TIMER_POP         0xf002
#define GET_MEM_BLOCK           0xf003
#define RESET_TIMEOUT           0xf004
#define IPARMS_VERIFY           0xf005
#define IADAP_FAIL              0xf006
#define IADAP_TIMEOUT           0xf007
#define OPEN_FAILED             0xf008
#define OPEN_TIMER_POP          0xf009
#define SB_DMA_SETUP            0xf00a
#define BRINGUP_DIAG_FAIL       0xf00b
#define TX_TCW_SETUP            0xf00c


#define DL_LOADER_PROG         0xa
#define RESET_ADAPTER          0xb
#define RCV_TCW_SETUP          0xc
#define GET_RCV_HEAD_MBUF      0xe
#define GET_MBUF_BUS_ADDR      0xf
#define GET_MBUF               0x10
#define GET_MBUF_EXT           0x11
#define OPEN_ADAP_FAILED       0x12
#define OPEN_NO_DMA_SPACE      0x14
#define OPEN_BAD_D_COMPLETE    0x15

#define OPEN_RETRYS_FAILED     0x17
#define ADAP_INIT_RETRY_FAIL   0x18


/*-------------------------------------------------------------------------*/
/*                          Receive Defines                                */
/*-------------------------------------------------------------------------*/

# define TOK_NETID_RECV_OFFSET  32
# define TOK_FC_OFFSET          1       /* byte offset to frame ctrl field */

/* Receive Requests */

# define VALID                  0x8000 >> 0
# define FRAME_COMPLETE         0x8000 >> 1
# define START_OF_FRAME         0x8000 >> 2
# define END_OF_FRAME           0x8000 >> 3
# define FRAME_INTERRUPT        0x8000 >> 4
# define INTERFRAME_WAIT        0x8000 >> 5

/* Receive Status */

# define FRAME_RECEIVED         0x8000 >> 0
# define RECV_SUSPENDED         0x8000 >> 1

# define MBUF_MS_WAIT   8000    /* eight seconds between retries */


/*-------------------------------------------------------------------------*/
/*                            PIO ACCESSORS                                */
/*-------------------------------------------------------------------------*/

/* POS register definitions */

# define POS_REG_0      0
# define POS_REG_1      1
# define POS_REG_2      2
# define POS_REG_3      3
# define POS_REG_4      4
# define POS_REG_5      5
# define POS_REG_6      6
# define POS_REG_7      7


#define PR0_VALUE 0xc8
#define PR1_VALUE 0x8f

#define POS_INT_9      0x09
#define POS_INT_3      0x03
#define POS_INT_4      0x04
#define POS_INT_5      0x05
#define POS_INT_7      0x07
#define POS_INT_10     0x0a
#define POS_INT_11     0x0b
#define POS_INT_12     0x0c


/*------------------------------------------------------------------------*/
/*  These BUS accessors are PIO-recovery versions of the original BUS     */
/*  accessor macros.  The essential difference is that retries are        */
/*  performed if pio errors occur; if the retry limit is exceeded, a -1   */
/*  is returned (hence all return an int value).  In the cases of         */
/*  PIO_GETL and PIO_GETLR, the -1 is indistinguishable from all FF's so  */
/*  some heuristic must be used to determine if it is an error (i.e., is  */
/*  all FF's a legitimate read value?).                                   */
/*------------------------------------------------------------------------*/

enum pio_func{
	GETC, GETS, GETSR, GETL, GETLR,
	PUTC, PUTS, PUTSR, PUTL, PUTLR
};

#define PIO_GETCX(addr, c)						\
{									\
	int rc;								\
	if (rc = BUS_GETCX(addr, c))					\
		pio_retry(p_dds, rc, GETC, addr, (ulong)(c));		\
}
#define PIO_GETSX(addr, s)						\
{									\
	int rc;								\
	if (rc = BUS_GETSX(addr, s))					\
		pio_retry(p_dds, rc, GETS, addr, (ulong)(s));		\
}
#define PIO_GETSRX(addr, sr)						\
{									\
	int rc;								\
	if (rc = BUS_GETSRX(addr, sr))					\
		pio_retry(p_dds, rc, GETSR, addr, (ulong)(sr));	\
}
#define PIO_GETLX(addr, l)						\
{									\
	int rc;								\
	if (rc = BUS_GETLX(addr, l))					\
		pio_retry(p_dds, rc, GETL, addr, (ulong)(l));		\
}
#define PIO_GETLRX(addr, lr)						\
{									\
	int rc;								\
	if (rc = BUS_GETLRX(addr, lr))					\
		pio_retry(p_dds, rc, GETLR, addr, (ulong)(lr));	\
}
#define PIO_PUTCX(addr, c)						\
{									\
	int rc;								\
	if (rc = BUS_PUTCX(addr, c))					\
		pio_retry(p_dds, rc, PUTC, addr, (ulong)(c));		\
}
#define PIO_PUTSX(addr, s)						\
{									\
	int rc;								\
	if (rc = BUS_PUTSX(addr, s))					\
		pio_retry(p_dds, rc, PUTS, addr, (ulong)(s));		\
}
#define PIO_PUTSRX(addr, sr)						\
{									\
	int rc;								\
	if (rc = BUS_PUTSRX(addr, sr))					\
		pio_retry(p_dds, rc, PUTSR, addr, (ulong)(sr));	\
}
#define PIO_PUTLX(addr, l)						\
{									\
	int rc;								\
	if (rc = BUS_PUTLX(addr, l))					\
		pio_retry(p_dds, rc, PUTL, addr, (ulong)(l));		\
}
#define PIO_PUTLRX(addr, lr)						\
{									\
	int rc;								\
	if (rc = BUS_PUTLRX(addr, lr))					\
		pio_retry(p_dds, rc, PUTLR, addr, (ulong)(lr));	\
}

/*
 * For pio's that are not in performance critical paths use these macros
 * that will resove to a routine.
 */
#define PIO_PUTC(a, c) pio_putc(p_dds, (char *)(a), (char)(c))
#define PIO_GETC(a) pio_getc(p_dds, (char *)(a))
#define PIO_PUTSR(a, s) pio_putsr(p_dds, (short *)(a), (short)(s))
#define PIO_GETSR(a) pio_getsr(p_dds, (short *)(a))

# define PIO_RETRY_COUNT        3


#endif /* ! _H_TOKDSLO */

