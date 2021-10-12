/* @(#)97	1.13  src/bos/kernext/tok/tokbits.h, sysxtok, bos411, 9428A410j 5/26/94 16:20:47 */
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TOKBITS
#define _H_TOKBITS

/*
 *	Adapter states (used with WRK.adap_state)
 */
#define DEAD_STATE	0x0acadead	/* adapter is dead */
#define NULL_STATE	0x0acaffff	/* after CFG_INIT */
#define OPEN_PENDING	0x0aca0031	/* started first adap. init. */
#define OPEN_STATE	0x0aca0032	/* Adapter is open */
#define CLOSED_STATE	0x0aca0000	/* Adapter is closed */
#define CLOSE_PENDING	0x0aca0002	/* Adapter close cmd is pending */
#define LIMBO_STATE	0x0aca0040	/* adapter in limbo   */

/*
 *	Adapter bringup/initialization states (used with WRK.bringup)
 */
#define RESET_PHASE0	0x0aca0010	/* adapter reset */
#define RESET_PHASE1	0x0aca0011
#define ADAP_INIT_PHASE0  0x0aca0020	/* adapter initialization */
#define ADAP_INIT_PHASE1  0x0aca0021
#define OPEN_PHASE0	 0x0aca0030	/* adapter open */

/*
 *	Adapter limbo states (used with WRK.limbo)
 */
#define PARADISE	0xa4400001	/* All is well */
#define CHAOS		0xa4400002 	/* Trying to re-activate adapter */
#define PROBATION	0xa4400003 	/* Adapter activated, but still 
					 * may fail
					 */
#define LIMBO_KILL_PEND	0xa4400004 	/* Kill of Limbo is pending */
#define NO_OP_STATE	0xa440ffff	/* Fatal error has occured */

/*
 *	POS register definitions
 */
#define TOKEN_L		0xc8		/* low byte of CARDID   */
#define TOKEN_H		0x8f		/* high byte of CARDID   */

#define CARD_ENABLE	0x01		/* 1 = card enabled */
#define SFDBKRTN	0x80		/* 1 = Monitor SFDBKRTN */
#define MC_PARITY_ON	0x40		/* 1 = parity enabled */
#define MC_PREEMPT_TIME	0x20		/* 1 = fr delay 6.5 micro secs */
#define MC_FAIRNESS	0x10		/* 1 = Fairness not employed */
#define STREAM_DATA	0x20		/* Enable Streaming Data */
#define MC_ARBITRATION	0x10		/* 1 = Disable MC Arbitration */
#define DMA_RCVRY	0x4		/* Include in System DMA recovery */
#define RING_SPEED_16	0x2		/* 1 = 16M, 0 = 4M */
#define DISABLE_TOKEN_REL  0x1		/* 1 = Disable Early Token Release */

#define POS_REG_0      0
#define POS_REG_1      1
#define POS_REG_2      2
#define POS_REG_3      3
#define POS_REG_4      4
#define POS_REG_5      5
#define POS_REG_6      6
#define POS_REG_7      7

#define POS_INT_9      0x09
#define POS_INT_3      0x03
#define POS_INT_4      0x04
#define POS_INT_5      0x05
#define POS_INT_7      0x07
#define POS_INT_10     0x0a
#define POS_INT_11     0x0b
#define POS_INT_12     0x0c

#define PIO_86A0   0x86a0
#define PIO_96A0   0x96a0
#define PIO_A6A0   0xa6a0
#define PIO_B6A0   0xb6a0
#define PIO_C6A0   0xc6a0
#define PIO_D6A0   0xd6a0
#define PIO_E6A0   0xe6a0
#define PIO_F6A0   0xf6a0

/*
 *	constants used for device initialization/open
 */
#define DF_ADAP_OPEN_OPTIONS   0x3100  /* adapter open options */
#define DF_ADAP_INIT_OPTIONS   0xf700  /* adapter initialization options */
#define DF_RCV_BURST_SIZE      0x80    /* default rcv burst size */
#define DF_TX_BURST_SIZE       0x80    /* default TX burst size */
#define DF_DMA_ABORT_THRESH    0x0a0a  /* default DMA abort threshold */

#define ADAP_BUF_SIZE_1K       0x400   /* 1024 byte adapter buffer size */
#define ADAP_4M_TX_BUF_MIN_CNT_1K      0x05
#define ADAP_4M_TX_BUF_MAX_CNT_1K      0x05
#define ADAP_16M_TX_BUF_MIN_CNT_1K     0x12
#define ADAP_16M_TX_BUF_MAX_CNT_1K     0x12

/*
 *	Token-Ring Interrupt Code definitions
 */
#define TOK_INT_SYSTEM     0x0080      /* Adapter has interrupt for system */
#define MC_ERR             0x0040      /* Micro-Channel Error Interrupt */
#define TOK_IC             0x000e      /* All 3 bits of Interrupt Code set */
#define ADAPTER_CHK        0x0000      /* Adapter Check Interrupt */
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

/*
 *	adapter command literals
 */
#define EXECUTE         0x9080    /* Execute SCB Cmd -Byte Swapped */
#define SCBINT          0x8880    /* Interrupt when SCB free */
#define RCV_VALID       0x8280    /* Receive  valid -Byte Swapped */
#define TX_VALID        0x8180    /* Transmit valid -Byte Swapped */
#define REJECT_CMD      0x0002    /* Reject Command */
#define ADAP_OPEN_CMD   0x0003    /* Open command for SCB */
#define ADAP_XMIT_CMD   0x0004    /* Transmit  command for SCB */
#define ADAP_XMIT_HALT  0x0005    /* Transmit Halt command for SCB */
#define ADAP_RCV_CMD    0x0006    /* Receive  command for SCB */
#define ADAP_CLOSE_CMD  0x0007    /* Close Command For SCB */
#define ADAP_GROUP_CMD  0x0008    /* Set Group Address Cmd For SCB */
#define ADAP_FUNCT_CMD  0x0009    /* Set Functional Addr Cmd For SCB */
#define ADAP_ELOG_CMD   0x000a    /* Read Error Log Cmd For SCB */
#define ADAP_READ_CMD   0x000b    /* Read Adapter Command For SCB */

/*
 *	Literals used in Microcode Download
 */
#define MAX_RETRYS             0x0003
#define DL_LOADER_BEGIN        0x00AA
#define DL_LDR_CMD1            0xE600
#define DL_LOADER_IMAGE        0x0010
#define DL_INIT_ADDR           0x07F0          /* DL Init. Para. addr */
#define DL_PROC_INIT           0x2000          /* DL Process Init. Para. cmd */
#define DL_EXE_LDR_PRG         0xc500          /* Execute Loader Program Cmd */


/*---------------------------------------------------------------------*/
/*                 Literals for Adapter Bringup                        */
/*---------------------------------------------------------------------*/
#define RESET_TIME		2	/* 2 second timer for reset */
#define RESET_SPIN_COUNT	5	/* retry reset 5 times */

#define INIT_TIME		2	/* 2 second timer for init status */
#define ADAP_INIT_SPIN_COUNT    5	/* check init status 5 times */

#define OPEN_TIMEOUT		60	/* 60 second timer for open */

#define CLOSE_TIMEOUT		6	/* 6 second timer for close */

#define RESET_OK               0x0040  /* Reset of adapter was successful */
#define RESET_FAIL             0x0030  /* Reset of adapter failed */

#define RECEIVE_SUSPENDED      0x4000  /* Adapter has suspended reception */
                                       /* of data */

#define INIT_ADAP_OK           0x0000  /* Initialization of Adap. succeeded */
#define INIT_ADAP_FAIL         0x0010

   /*
    * used with bu_wdt_cmd
    */
#define INTR_ADAP_RESET        	1
#define INTR_ADAP_INIT         	2
#define INTR_ADAP_OPEN         	3
#define INTR_ADAP_CLOSE        	4
#define INTR_PROBATION         	5
#define INTR_FUNC_TIMEOUT      	6
#define INTR_GROUP_TIMEOUT     	7
#define INTR_ELOG_TIMEOUT     	8
#define INTR_READ_ADAP_TIMEOUT 	9
#define INTR_CYCLE_LIMBO 	10

  /*
   *   Timeout values
   */
#define PENALTY_BOX_TIME	12		/* 12 sec. timer */
#define FUNC_ADDR_TIMEOUT	1		/* 1 sec. timer */
#define GROUP_ADDR_TIMEOUT	1		/* 1 sec. timer */
#define ELOG_CMD_TIMEOUT	1		/* 1 sec. timer */
#define READ_ADAP_TIMEOUT	1		/* 1 sec. timer */
#define TRANSMIT_TIMEOUT	10		/* 10 sec. timer */
#define OPEN_ERROR_TIME		30		/* 30 sec. timer */
#define RING_SPEED_TIME		120		/* 2 minute timer */
#define RMV_RECEIVED_TIME	360		/* 6 minute timer */


  /*
   *   Ring Status interrupt bit definitions
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

/*
 *   Limbo Mode entry conditions for Ring Status
 *   and Command Reject Status
 *
 *	Ring Status:
 *		Lobe Wire Fault
 *		Auto-Removal Error 1
 *		Remove received	
 *	
 *	Cmd Reject Status:
 *		Illegal Cmd
 *		Address Error
 *		Adapter Open
 *		Adapter Closed
 *		Same Cmd
 */

#define RS_ENTER_LIMBO     0x0d00
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

#define MULTI_BIT_MASK	0x80		/* bit on == not-individual address */
#define GROUP_ADR_MASK	0x80		/* bit on == group address */


        /*
         * Transmit footprint codes
         */
#define TX_TXOP_0               0x01010000  /* tx_done() */
#define TX_TXOP_1               0x01010001  /* tx_done() */
#define TX_TXOP_3               0x01010003  /* tx_done() */
#define TX_TXOP_4               0x01010004  /* tx_done() */
#define TX_TXOP_5               0x01010005  /* tx_done() */
#define TX_MOVE_LIST_0		0x01020000  /* move_tx_list() */


        /*
         * Receive footprint codes
         */
#define RCV_TRCV_0              0x02010000  /* tok_receive() */
#define RCV_TRCV_1              0x02010001  /* tok_receive() */
#define RCV_TRCV_2              0x02010002  /* tok_receive() */
#define RCV_LRC_0               0x02020000  /* load_recv_chain () */
#define RCV_RRL_0               0x02030000  /* read_recv_list () */
#define RCV_CRC_0               0x02040000  /* clear_recv_chain () */


        /*
         * Limbo footprint codes
         */
#define LMB_KILL_0              0x03030000  /* kill_limbo() */
#define LMB_KILL_1              0x03030001  /* kill_limbo() */
#define LMB_KILL_2              0x03030002  /* kill_limbo() */
#define LMB_KILL_3              0x03030003  /* kill_limbo() */
#define LMB_EGRESS_0            0x03040000  /* egress_limbo() */

        /*
         * SLIH Footprint codes
         */
#define SLIH_0          0x04010000
#define SLIH_1          0x04010001
#define SLIH_2          0x04010002

        /*
         * Off-Level footprint codes
         */
#define INTR_CMD_0      0x05010000        /* cmd_pro() */
#define INTR_CMD_1      0x05010001        /* cmd_pro() */
#define INTR_ADAP_CHK   0x05020000        /* adap_chk() */
#define INTR_IOCTL_0    0x05030000        /* INTR_ioctl_to */
#define INTR_IOCTL_1    0x05030001        /* INTR_ioctl_to */
#define INTR_IOCTL_2    0x05030002        /* INTR_ioctl_to */
#define INTR_RSP_0      0x05050000        /* ring_stat_pro() */
#define INTR_RSP_1      0x05050001        /* ring_stat_pro() */
#define INTR_RSP_2      0x05050002        /* ring_stat_pro() */

        /*
         * Activation Footprint codes
         */
#define ACT_RESET_PHS2_0        0x07020000      /* reset_phase2 */
#define ACT_RESET_PHS2_2        0x07020002
#define ACT_IPHS0_0             0x07040000      /* init_adap_phase0 */
#define ACT_IPHS0_1             0x07040001      /* init_adap_phase0 */
#define ACT_IPHS1_0             0x07050000      /* init_adap_phase1 */
#define ACT_IPHS1_1             0x07050001      /* init_adap_phase1 */
#define ACT_IPHS1_3             0x07050003      /* init_adap_phase1 */
#define ACT_IPHS1_4             0x07050004      /* init_adap_phase1 */
#define ACT_OA_PEND_0           0x07080000      /* open_adap_pend */
#define ACT_OA_PEND_1           0x07080001      /* open_adap_pend */
#define ACT_OA_TO_0             0x07090000      /* open_adap_timeout */
#define ACT_OA_TO_1             0x07090001      /* open_adap_timeout */
#define ACT_INTR_BUP_0          0x070a0000
#define ACT_INTR_BUP_1          0x070a0001
#define ACT_INTR_BUP_2          0x070a0002
#define ACT_INTR_BUP_3          0x070a0003
#define ACT_INTR_BUP_4          0x070a0004
#define ACT_INTR_BUP_5          0x070a0005
#define ACT_INTR_BUP_6          0x070a0006
#define TOK_XMALLOC_FAIL	0x070b0001	/* xmalloc failed */
#define TOK_XMATTACH_FAIL	0x070b0002	/* xmattach failed */
#define D_INIT_FAIL		0x070b0003	/* d_init failed */
#define D_CLEAR_FAIL		0x070b0005	/* d_clear failed */
#define D_UNMASK_FAIL		0x070b0008	/* d_umask failed */

        /*
         * Micorcode Download footprint codes
         */
#define TOKDWNLD_0		0x0a010000  /* tokdnld() */
#define TOKDWNLD_1		0x0a010001  /* tokdnld() */
#define DOWN_LDR_IMAGE_0	0x0a020000  /* dl_ldr_image() */
#define DOWN_INIT_MC_0		0x0a030000  /* dl_init_mc() */
#define DOWN_INIT_MC_1		0x0a030001  /* dl_init_mc() */
#define DOWN_INIT_MC_2		0x0a030002  /* dl_init_mc() */
#define DOWN_INIT_MC_3		0x0a030003  /* dl_init_mc() */
#define DOWN_INIT_MC_4		0x0a030004  /* dl_init_mc() */
#define DOWN_INIT_MC_5		0x0a030005  /* dl_init_mc() */
#define DOWN_INIT_MC_6		0x0a030006  /* dl_init_mc() */
#define DOWN_INIT_MC_7		0x0a030007  /* dl_init_mc() */

/*-------------------------------------------------------------------------*/
/*                          Receive Defines                                */
/*-------------------------------------------------------------------------*/

#define TOK_NETID_RECV_OFFSET  32
#define TOK_FC_OFFSET          1       /* byte offset to frame ctrl field */

/* Receive Requests */

#define VALID                  0x8000
#define FRAME_COMPLETE         0x4000
#define START_OF_FRAME         0x2000
#define END_OF_FRAME           0x1000
#define FRAME_INTERRUPT        0x0800
#define INTERFRAME_WAIT        0x0400

/* Receive Status */

#define FRAME_RECEIVED         0x8000
#define RECV_SUSPENDED         0x4000

/*
 * used with open_status
 */
#define OPEN_STATUS0	0
#define OPEN_STATUS1	1
#define OPEN_STATUS2	2
#define OPEN_STATUS3	3
#define OPEN_STATUS4	4
#define OPEN_COMPLETE	6

#define DD_NAME_STR        "trmon"         /* define name for dumping */
#define TX_DUMP_TIMEOUT  1000   /* Dump write timeout value */

#define IOCC_CACHE_SIZE    0x40		/* IOCC Cache size in bytes */
#define RCV_CHAIN_SIZE     0x10		/* Receive Chain Size */
#define TX_CHAIN_SIZE      (14)		/* Transmit Chain Size */
#define TX_MAX_BUFFERS     (38)		/* Max transmit buffers (2K each) */
					/*   must be max of next 4 counts */
#define TX_4_MEG_MIN_BUF   (8)		/* Minimum xmit buffers for 4 meg */
					/*   enough to hold 2 packets of max */
					/*   size + 2 packets < 2k in size */
#define TX_4_MEG_MAX_BUF   (14)		/* Maximum xmit buffers for 4 meg */
					/*   enough to hold 4 packets of max */
					/*   size + 2 packets < 2k in size */
#define TX_16_MEG_MIN_BUF  (20)		/* Minimum xmit buffers for 16 meg */
					/*   enough to hold 2 packets of max */
					/*   size + 2 packets < 2k in size */
#define TX_16_MEG_MAX_BUF  (38)		/* Maximum xmit buffers for 16 meg */
					/*   enough to hold 4 packets of max */
					/*   size + 2 packets < 2k in size */
#define TX_BUF_SIZE	   0x800	/* size of each transmit buffer */
#define ROS_LEVEL_SIZE 4
#define MICROCODE_LEVEL_SIZE    3

/*
 * define enough entries for DD control structure
 * and for each adapter, the DDs and ACA memory block
 */
#define TOK_MAX_ADAPTERS	16
#define MAX_CDT_ELEMS (1 + (TOK_MAX_ADAPTERS * 2))

/*
 * define the freeze dump commands and amount to dump
 */
#define TOK_DO_FREEZE_DUMP	(NDD_DEVICE_SPECIFIC)
#define TOK_READ_FREEZE_DUMP	(NDD_DEVICE_SPECIFIC +1)
#define TOK_FREEZE_DUMP_SIZE	0x10000

#endif     /* ! _H_TOKBITS */
