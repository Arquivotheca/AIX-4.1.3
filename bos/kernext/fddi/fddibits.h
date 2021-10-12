/* @(#)77       1.5  src/bos/kernext/fddi/fddibits.h, sysxfddi, bos411, 9428A410j 3/30/94 15:38:29 */
/*
 *   COMPONENT_NAME: SYSXFDDI
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
#ifndef _H_FDDIBITS
#define _H_FDDIBITS


/*
 * state machine possible values
 */
#define NULL_STATE 		(0x0fdd0000)	/* the initial state */
#define DNLDING_STATE 		(0x0fdd0dac)	/* Downloading
						 * microcode to adapter
						 */
#define DNLD_STATE		(0x0fdd0010)	/* microcode has been
						 * successfully loaded
						 * we will be in this
						 * after the config method
						 * has successfully finished.
						 */
#define INIT_STATE		(0x0fdd0020)	/* The CFG_INIT has successfully
						 * run.
						 */
#define OPENING_STATE		(0x0fdd0030)	/* fddi_open has been called 
						 * but has not
						 * completed yet 
						 */
#define OPEN_STATE		(0x0fdd0fdd)	/* fddi_open has been called 
						 * and completed successfully. 
						 * This is the normal, fully
						 * operational state
						 */

#define LIMBO_STATE		(0x0fdd0404)	/* We are in limbo. we are
						 * trying to recover from a
						 * significant error.
						 */

#define LIMBO_RECOVERY_STATE	(0x0fdd0505)	/* We are recovering from limbo.
						 * The adapter reset has 
						 * completed.
						 */
#define CLOSING_STATE		(0x0fdd0606)	/* shutting down the device */
#define DEAD_STATE		(0x0fdddead)	/* the adapter has 
						 * died, it is no 
						 * longer useable 
						 */

/*
 * DMA values
 */
#define FDDI_MIN_DMA_SPACE	(0x55000) 	/* min TCW space needed */
#define FDDI_MAX_DMA_PARTITION	(65536)		/* HW lim, 16 bit addressing */ 

/* 
 * download tx block value
 */
#define FDDI_MAX_TX_SZ 		(65534)		/* HW lim */


#define CFDDI_OPLEVEL		(PL_IMP)

#define CFDDI_GET_TRACE		(NDD_DEVICE_SPECIFIC+1)


/* -------------------------------------------------------------------- */
/* 			Link Statistics					*/
/* -------------------------------------------------------------------- */
/*
 *
 * SMT Error Word (high and low)
 *	The RSI (Ring Status Interrupt) bit in the HSR
 *	(host status register) may be set when any of the
 *	SMT error word bits are set. This will depend on
 *	the value of the associated bit in the SMT error
 *	mask word.
 */


/*
 * The PVO - parameter value out of range indicates that the parameter
 *	value passed in one of the following commands was out of range:
 *
 *		Write T-Max Greatest Lower Bound
 *		Write T-Max Lower Bound
 *		Write T-Max 
 *		Write T-Min 
 *		Write T-Req
 *		Write PORT A (or PORT S) LER cutoff
 *		Write PORT B LER cutoff
 *		Write PORT A (or PORT S) LER alarm
 *		Write PORT B LER alarm
 */

#define FDDI_SMT_ERR_HI_VMR	(0x8000)	/* Version Mismatch Response */
#define FDDI_SMT_ERR_HI_NSR	(0x0200)	/* No SBA RAF Response */
#define FDDI_SMT_ERR_HI_IFG	(0x0100)	/* Inter Frame Gap */
#define FDDI_SMT_ERR_HI_MPC	(0x0080)	/* MAC Path Change Event */
#define FDDI_SMT_ERR_HI_BPC	(0x0040)	/* Port B Path Change Event */
#define FDDI_SMT_ERR_HI_APC	(0x0020)	/* Port A Path Change Event */
#define FDDI_SMT_ERR_HI_NCE	(0x0010)  	/* MAC neighbor chged state */
#define FDDI_SMT_ERR_HI_UCE	(0x0008)	/* undesirable connect event */
#define FDDI_SMT_ERR_HI_PWR	(0x0004)	/* Peer wrap condition */
#define FDDI_SMT_ERR_HI_DAC	(0x0002)	/* Duplicate addr condition */
#define FDDI_SMT_ERR_HI_BED	(0x0001)	/* Port B EB ovflw condition */
/*
 * SMT Error Low Word 
 */
#define FDDI_SMT_ERR_LO_AEB	(0x8000)	/* Port A EB ovflw condition */
#define FDDI_SMT_ERR_LO_BLC	(0x4000)	/* PORT B LER condition */
#define FDDI_SMT_ERR_LO_ALC	(0x2000)	/* PORT A LER condition */
#define FDDI_SMT_ERR_LO_NCC	(0x1000)	/* MAC Frame not copied */
						/*  condition */
#define FDDI_SMT_ERR_LO_FEC	(0x0800)	/* MAC Frame error condition */
#define FDDI_SMT_ERR_LO_BSN	(0x0400)	/* PORT B stuck in PC4 NEXT */
						/*  STATE */
#define FDDI_SMT_ERR_LO_ASN	(0x0200)	/* PORT A stuck in PC4 NEXT */
						/*  STATE */
#define FDDI_SMT_ERR_LO_BSC	(0x0100)	/* PORT B stuck in PC3 CONNECT*/
						/*  STATE */
#define FDDI_SMT_ERR_LO_ASC	(0x0080)	/* PORT A stuck in PC3 CONNECT*/
						/*  STATE */
#define FDDI_SMT_ERR_LO_BBS	(0x0040)	/* PORT B break state flag */
#define FDDI_SMT_ERR_LO_ABS	(0x0020)	/* PORT A break state flag */
#define FDDI_SMT_ERR_LO_TSE	(0x0008)  	/* Trace status event */
#define FDDI_SMT_ERR_LO_DS	(0x0004)	/* Detect state - loss of */
						/*  Ring_op */
#define FDDI_SMT_ERR_LO_SBF	(0x0001)	/* fail optical bypass switch */
/*
 * SMT Event Word (High)
 */
#define FDDI_SMT_EVNT_HI_SCS	(0x0800)	/* Sync Allocation */
						/* Change Secondary 	*/
#define FDDI_SMT_EVNT_HI_SCP	(0x0400)	/* Sync Allocation */
						/* Change Primary */
#define FDDI_SMT_EVNT_HI_SBD	(0x0200)	/* Sync Bandwidth Denied*/
#define FDDI_SMT_EVNT_HI_SBS	(0x0100)	/* Sync Bandwidth Successful */
#define FDDI_SMT_EVNT_HI_TNC	(0x0080)	/* T_NEG Changed */
#define FDDI_SMT_EVNT_HI_RFP	(0x0040)	/* Received Frames Purged */
#define FDDI_SMT_EVNT_HI_DBS	(0x0020)	/* Directed beacon sent */
#define FDDI_SMT_EVNT_HI_VNS	(0x0010)	/* RDF frame version not */
						/*  supported */
#define FDDI_SMT_EVNT_HI_CNS	(0x0008)	/* RDF frame class not */
						/*  supported */
#define FDDI_SMT_EVNT_HI_VMR	(0x0004)	/* Version mismatch request */
#define FDDI_SMT_EVNT_HI_VMA	(0x0002)	/* Version mismatch announced */
/*
 * SMT Event Word (Low)
 */
#define FDDI_SMT_EVNT_LO_RTT	(0x4000)	/* Restricted token terminated*/
#define FDDI_SMT_EVNT_LO_CPV	(0x1000)	/* Connection policy violation*/
#define FDDI_SMT_EVNT_LO_PE	(0x0800)	/* Port event via remote port */
						/*  cmd */
#define FDDI_SMT_EVNT_LO_LSS	(0x0400)	/* LLC service status */
#define FDDI_SMT_EVNT_LO_LSC	(0x0200)	/* LLC service change */
#define FDDI_SMT_EVNT_LO_PT_MSK	(0x01C0)	/* Path test cur status mask */
#define FDDI_SMT_EVNT_LO_ROS	(0x0020)	/* Ring Op Status */
#define FDDI_SMT_EVNT_LO_ROC	(0x0010)	/* Ring Op Change */
#define FDDI_SMT_EVNT_LO_TRC	(0x0008)	/* T-Req changed remotely */
#define FDDI_SMT_EVNT_LO_STR	(0x0004) 	/* Self Test Required */
#define FDDI_SMT_EVNT_LO_CW	(0x0002)	/* connection withheld */
#define FDDI_SMT_EVNT_LO_RDF	(0x0001)	/* Remote disconnect received */

#define FDDI_SMT_EVNT_LO_PTF	(0x00c0)	/* Path Tests failed mask */
/* 
 * SMT event and error significant things
 */
#define FDDI_BTI_SMT_EVNT_LO 	(FDDI_SMT_EVNT_LO_RDF |		\
				FDDI_SMT_EVNT_LO_STR)

#define FDDI_SMT_ERR_LO_STUCK  (FDDI_SMT_ERR_LO_BSN |		\
				FDDI_SMT_ERR_LO_ASN |		\
				FDDI_SMT_ERR_LO_BSC |		\
				FDDI_SMT_ERR_LO_ASC |		\
				FDDI_SMT_ERR_LO_BBS |		\
				FDDI_SMT_ERR_LO_ABS)

/*
 * SMT Control Word
 */
#define	FDDI_SMT_CTL_MULTI	(0x0040)	/* set adapter into multicast */
#define	FDDI_SMT_CTL_PROM	(0x0020)	/* set adapter into prom */
#define	FDDI_SMT_CTL_CRC	(0x0010)	/* send bad CRC frames to host*/
#define	FDDI_SMT_CTL_BEA	(0x0004)	/* send Beacon Frames to Host */
#define	FDDI_SMT_CTL_NSA	(0x0002)	/* send NSA frames to host */
#define	FDDI_SMT_CTL_SMT	(0x0001)	/* send SMT frames to host */
/*
 * CONNECTION POLICY VIOLATION WORD
 *	Indicates the specific connection type at a Port that is in 
 *	violation of the Connection Policy.
 */
#define FDDI_CPV_MM	(0x8000)	/* Master to Master - not supported */
#define FDDI_CPV_MS	(0x4000)	/* Master to Slave - not supported */
#define FDDI_CPV_MB	(0x2000)	/* Master to Port B - not supported */
#define FDDI_CPV_MA	(0x1000)	/* Master to Port A - not supported */
#define FDDI_CPV_SM	(0x0800)	/* Slave to Master conn attempted */
#define FDDI_CPV_SS	(0x0400)	/* Slave to Slave conn attempted */
#define FDDI_CPV_SB	(0x0200)	/* Slave to Port B conn attempted */
#define FDDI_CPV_SA	(0x0100)	/* Slave to Port A conn attempted */
#define FDDI_CPV_BM	(0x0080)	/* Port B to Master conn attempted */
#define FDDI_CPV_BS	(0x0040)	/* Port B to Slave conn attempted */
#define FDDI_CPV_BB	(0x0020)	/* Port B to Port B conn attempted */
#define FDDI_CPV_BA	(0x0010)	/* Port B to Port A conn attempted */
#define FDDI_CPV_AM	(0x0008)	/* Port A to Master conn attempted */
#define FDDI_CPV_AS	(0x0004)	/* Port A to Slave conn attempted */
#define FDDI_CPV_AB	(0x0002)	/* Port A to Port B conn attempted */
#define FDDI_CPV_AA	(0x0001)	/* Port A to Port A conn attempted */
/*
 * 	PORT EVENT WORD
 */
#define FDDI_PORTB_EVNT_BSP	(0x1000)	/* Port B OFF state */
#define FDDI_PORTB_EVNT_BST	(0x0800)	/* Port B BREAK state */
#define FDDI_PORTB_EVNT_BDS	(0x0400)	/* Port B MAINT state */
#define FDDI_PORTB_EVNT_BEN	(0x0200)	/* Port B OFF state from MAINT*/
						/*  state */
#define FDDI_PORTB_EVNT_BMT	(0x0100)	/* Port B MAINT state from OFF*/
						/*  state */
#define FDDI_PORTA_EVNT_ASP	(0x0010)	/* Port A OFF state due to */
						/*  remote cmd */
#define FDDI_PORTA_EVNT_AST	(0x0008)	/* Port A BREAK state */
#define FDDI_PORTA_EVNT_ADS	(0x0004)	/* Port A MAINT state */
#define FDDI_PORTA_EVNT_AEN	(0x0002)	/* Port A OFF state from MAINT*/
						/*  state */
#define FDDI_PORTA_EVNT_BMT	(0x0001)	/* Port A MAINT state from OFF*/
						/*  state */
/*
 *	ADAPTER CHECK INTERRUPT (ACI) ID-CODE
 */
#define FDDI_ACI_SDRAPAR	(0x1106) /* A-frame parity err during DPC read*/
#define FDDI_ACI_SDRSPAR	(0x1105) /* S-frame parity err during DPC read*/
#define FDDI_ACI_SDWRPAR	(0x1104) /* parity err during DPC write */
#define FDDI_ACI_SNLSLD		(0x1103) /* NP and RBC simultaneous load */
#define FDDI_ACI_SPENWR		(0x1211) /* Parity error for NP write */
#define FDDI_ACI_SPENRD		(0x1210) /* Parity error for NP read */
#define FDDI_ACI_SNPPND		(0x1207) /* NP Request pending to FRB for */
					 /* >100 cycles */
#define FDDI_ACI_SCEPDA		(0x2115) /* Coding err in A-frame ptr or desc */
#define FDDI_ACI_SCEPDS		(0x2114) /* Coding err in S-frame ptr or desc */
#define FDDI_ACI_SPEPDA		(0x2113) /* Parity err in A-frame ptr or desc */
#define FDDI_ACI_SPEPDS		(0x2112) /* Parity err in S-frame ptr or desc */
#define FDDI_ACI_SOVFUND	(0x2111) /* DPC FIFO underflow or overflow */
#define FDDI_ACI_SNDSLP 	(0x2110) /* No response to DPC caused Q ovflo*/
#define FDDI_ACI_SS		(0x6100) /* Protocol violation oc cmd reject */
#define FDDI_ACI_PER		(0x6102) /* Parity err on Y-bus or Skyline */
#define FDDI_ACI_CRTN		(0x7101) /* QUEUE storage is full */
#define FDDI_ACI_LISA_INT	(0x7401) /* ALISA retry limit exceeded */
#define FDDI_ACI_CMD_HDLR	(0x7701) /* Station Address (LAID) is */
					 /*  invalid*/

/* ---------------------------------------------------------------------------
 * 	System Interface (SIF) REGISTER OFFSETS
 * ---------------------------------------------------------------------------
 */
#define FDDI_HSR_REG        (0x00)   /* Base Offset of 0 */
#define FDDI_HMR_REG        (0x08)   /* Base Offset of 8 */
#define FDDI_NM1_REG        (0x0A)   /* Base Offset of 10 */
#define FDDI_NM2_REG        (0x0C)   /* Base Offset of 12 */
#define FDDI_ACL_REG        (0x0E)   /* Base Offset of 14 */

/*
 * Command Parameter Block (CPB) values
 */
#define FDDI_CPB_SIZE		(20)
#define FDDI_CPB_ERR_IDX	(FDDI_CPB_SIZE - 1)	/* index to rc */

				/* Addr of the hcr cmd's result field */
#define FDDI_CPB_RES       	(FDDI_CPB_SHARED_RAM + (FDDI_CPB_ERR_IDX * 2)) 
									
/*
 * HCR return codes for all HCR commands
 */
#define FDDI_HCR_SUCCESS	(0x0)	/* HCR cmd success code */
#define FDDI_HCR_UNDEFINED	(0x1)	/* HCR cmd undefined command code */
#define FDDI_HCR_INV_SETCOUNT	(0x2)	/* HCR cmd invalid setcount code */
#define FDDI_HCR_PVO_RANGE	(0x3)	/* HCR cmd param value out of range */
#define FDDI_HCR_NO_ADDR	(0x4)	/* HCR cmd no address available */
#define FDDI_HCR_INV_ADDRTYPE	(0x6)	/* HCR cmd invalid address type */
#define FDDI_HCR_DIS_REQ	(0x7)	/* HCR cmd disconnect required */

/*
 * HCR return codes for specific commands
 */
#define FDDI_HCR_WR_SKY_UNKN	(0x0901) /* write SKYLINE address failed */
#define FDDI_HCR_WR_SKY_IND_ADDR (0x0902) /* write SKYLINE address failed */
#define FDDI_HCR_WRITE_FLG	(0x0911) /* write FORMAC long group fail */
#define FDDI_HCR_CLEAR_SKY	(0x0B00) /* Clear SKYLINE failed */
#define FDDI_HCR_PTF		(0x0E00) /* Path Test failed */
#define FDDI_HCR_INV_PATH	(0x0E02) /* Insertion policy violation */
#define FDDI_HCR_CONN_REQ	(0x0F00) /* CONNECT required */
#define FDDI_HCR_RO_REQ		(0x2C00) /* Ring-Op required */
					/* No addresses avail for a 
					 * a Measure Ring Latency
					 * test
					 */
#define FDDI_HCR_NO_ADDR_RING_LAT	(0x2C01) 

/*
 * Other HCR values
 */
#define FDDI_HCR_ALD_SKY	(0x2)	/* addr len designator SKYLINE long */
#define FDDI_HCR_ATD_ALL	(0xff)  /* all address registers (clear cmd) */
#define FDDI_HCR_ATD_FOR	(0x1)	/* addr type designator FORMAC long */
#define FDDI_HCR_ATD_SKY	(0x0)	/* addr type designator SKYLINE long */

#define FDDI_PRI_CMD_ULS	(0x0001)
#define FDDI_PRI_CMD_RSI	(0x0002)
#define FDDI_HCR_QADR		(0xffff)

#define FDDI_PRI_PATH		(0x0001) /* Path identifiers for HCR commands */
#define FDDI_SEC_PATH		(0x0002)
#define FDDI_LOC_PATH		(0x0003)

/*
 * Shared Ram
 */
#define FDDI_RX_SHARED_RAM		(0xf0)
#define FDDI_CPB_SHARED_RAM       	(0x190) 	/* CPB address */
#define FDDI_LS_SHARED_RAM		(0x01b8)	/* addr of links stat */


/* -------------------------------------------------------------------- */
/* 				FDDI HCR Commands			*/
/* -------------------------------------------------------------------- */
#define FDDI_HCR_REG        		(0x02)   /* Base Offset of 2 */
#define FDDI_HCR_START_MCODE		(0xFF) 	 /* Start the microcode */
#define FDDI_HCR_ABORT_TX		(0x0200) /* Abort Transmit */
#define FDDI_HCR_ABORT_RX		(0x0400) /* Abort Receive */
#define FDDI_HCR_WRITE_ADDR		(0x0900) /* set short address */
#define FDDI_HCR_READ_ADDR		(0x0A00) /* set Long Address */
#define FDDI_HCR_CLEAR_ADDR		(0x0B00) /* Clear Address */
#define FDDI_HCR_ULS			(0x0C00) /* Update Link Statistics */
#define FDDI_HCR_DIAG_MODE		(0x0D00) /* Enter Diagnostic Mode */
#define FDDI_HCR_CONNECT		(0x0E00) /* Connect cmd */
#define FDDI_HCR_DISCONNECT		(0x0F00) /* Disconnect cmd */
#define FDDI_HCR_READ_USER_DATA		(0x1000) /* Read user data */
#define FDDI_HCR_READ_CONN_POL		(0x1100) /* Read Connectivity policy */
#define FDDI_HCR_READ_RING_LAT		(0x1200) /* Read Ring Latency */
#define	FDDI_HCR_READ_STATE		(0x1800) /* Read the adapter state */
#define FDDI_HCR_RD_REQ_PATH		(0x1900) /* Read Requested Path */
#define FDDI_HCR_WR_USR_DATA		(0x2200) /* Write User Data */
#define FDDI_HCR_WR_ATT_CLASS		(0x2300) /* Write Attachment Class */
#define FDDI_HCR_WR_PASSWORD		(0x2400) /* Write PMF password */
#define FDDI_HCR_WR_TMAX_LOW		(0x2600) /* Write T-Max Lower bound */
#define FDDI_HCR_WR_MAX_TREQ		(0x2900) /* Write Max T-Req */
#define FDDI_HCR_WR_ERR_THRHLD		(0x2A00) /* Wr frame error threshold */
#define FDDI_HCR_WR_NCP_THRHLD		(0x2B00) /* Wr not copied threshold */
#define FDDI_HCR_WR_RING_LAT		(0x2C00) /* Wr measure ring latency */
#define FDDI_HCR_WR_FOT_CLASS		(0x2D00) /* Wr Fiber Optic Tx Class */
#define FDDI_HCR_WR_A_LER_CUTOFF	(0x2E00) /* Wr port A LER Cutoff */
#define FDDI_HCR_WR_B_LER_CUTOFF	(0x2F00) /* Wr port B LER Cutoff */
#define FDDI_HCR_WR_A_LER_ALARM		(0x3000) /* Wr port A LER Alarm */
#define FDDI_HCR_WR_B_LER_ALARM		(0x3100) /* Wr port B LER Alarm */
#define FDDI_HCR_WR_LLC_ENABLE		(0x3500) /* Wr LCC service enable */
#define FDDI_HCR_WR_W			(0x3600) /* Wr weighing factor */
#define FDDI_HCR_WR_SAM_TIME		(0x3700) /* Wr Sample time */
#define FDDI_HCR_WR_TVX_LOW_BND		(0x3800) /* Wr TVX Lower Bound */
#define FDDI_HCR_WR_SMT_CTL		(0x3D00) /* Wr SMT Control Word */

/*
 * HOST INITIATED DIAGNOSTIC tests
 */
#define FDDI_HCR_TEST0 			(0x0000)
#define FDDI_HCR_TEST1 			(0x0001)
#define FDDI_HCR_TEST2 			(0x0002)
#define FDDI_HCR_TEST3 			(0x0003)
#define FDDI_HCR_TEST4 			(0x0004)
#define FDDI_HCR_TEST5 			(0x0005)
#define FDDI_HCR_TEST6 			(0x0006)
#define FDDI_HCR_TEST7 			(0x0007)
#define FDDI_HCR_TEST8 			(0x0008)
#define FDDI_HCR_TEST9 			(0x0009)

/* -------------------------------------------------------------------- */
/* 			HSR - Host Status Register 			*/
/* -------------------------------------------------------------------- */
#define FDDI_HSR_ADDR     	(0x00)   /* Base Offset of 0 */
#define FDDI_HSR_DDC        	(0x8000) /* Download/Diagnostic Complete */
#define FDDI_HSR_NSF        	(0x4000) /* Error - No Cd SFDBK */
#define FDDI_HSR_CCR        	(0x2000) /* Error - Channel Check On Read */
#define FDDI_HSR_CCW        	(0x1000) /* Error - Channel Check on Write */
#define FDDI_HSR_DPR        	(0x0800) /* Error - Data Parity On Read */
#define FDDI_HSR_ACI        	(0x0400) /* Error - Adapter Check Interrupt */
#define FDDI_HSR_AWS        	(0x0200) /* Error - Adapter Warm Start */
#define FDDI_HSR_RSI        	(0x0100) /* Ring Status Interrupt  */
#define FDDI_HSR_CCI        	(0x0080) /* Command Completion Interrupt */
#define FDDI_HSR_CIA        	(0x0040) /* Tx Command Comp Interrupt Async*/
#define FDDI_HSR_RCI        	(0x0020) /* Rcv Command Completion Interrupt */
#define FDDI_HSR_CIS        	(0x0010) /* Tx Command Comp Interrupt sync*/
#define FDDI_HSR_TIS        	(0x0008) /* Sync Tx buffer has been queued */
#define FDDI_HSR_TIA        	(0x0004) /* Async Tx buffer has been queued */
#define FDDI_HSR_RIR        	(0x0002) /* Interrupt Frame Rcv Complete */
#define FDDI_HSR_DDA        	(0x0001) /* Download/diagnostics Aborted */

/* 
 * Create an error mask that will detect all errors that 
 * can cause a state change.
 *
 * 	NSF - No CD SFDBK (MC error)
 *	CCR - Channel Check on read (MC error)
 *	CCW - Channel Check on write (MC error)
 *	DPR - Data parity on read (MC error)
 *	ACI - Adapter Check interrupt
 */
#define FDDI_HSR_ERRORS	(FDDI_HSR_NSF	|\
			FDDI_HSR_CCR	|\
			FDDI_HSR_CCW	|\
			FDDI_HSR_DPR	|\
			FDDI_HSR_AWS	|\
			FDDI_HSR_ACI	)

/*
 * Only interrupt on ERRORS, and CCI, RSI
 */
#define FDDI_HMR_BASE_INTS	(0x807F)
#define FDDI_HMR_RX_TX		(0x8059)

/*
 * HMR masks:
 * 	Turn off the bits that you want interrupts 
 *	(a '1' prevents the setting of the corresponding bit in the reg)
 */
#define FDDI_HMR_DWNLD	(0x007e)	/* mask for DOWNLOAD */

/* ------------------------------------------------------------------------- */
/* NS1 - Node Processor Status Register 1               (I/O Base + 4)       */
/* ------------------------------------------------------------------------- */
#define FDDI_NS1_REG    (0x04)   /* Base Offset of 4 */
#define FDDI_NS1_LIC    (0x8000) /* Instruction Complete From Cmd Reg. */
#define FDDI_NS1_LCA    (0x4000) /* Instruction Complete Due To Abort */
#define FDDI_NS1_NCL    (0x2000) /* New Command List */
#define FDDI_NS1_XMA    (0x1000) /* Transmit Command Asynchronous */
#define FDDI_NS1_RX    (0x0800) /* Receive Command */
#define FDDI_NS1_HCC    (0x0400) /* Host Channel Check (user defined) */
#define FDDI_NS1_XMS    (0x0200) /* Transmit Command Synchronous */

/* ------------------------------------------------------------------------- */
/* NS2 - Node Processor Status Register 2               (I/O Base + 6)       */
/* ------------------------------------------------------------------------- */
#define FDDI_NS2_REG    (0x06)   /* Base Offset of 6 */
#define FDDI_NS2_NSF    (0x8000) /* No CD SFDBK */
#define FDDI_NS2_CCR    (0x4000) /* Channel Check On Read */
#define FDDI_NS2_CCW    (0x2000) /* Channel Check On Write */
#define FDDI_NS2_DPR    (0x1000) /* Data Parity On Read */
#define FDDI_NS2_LPR    (0x0800) /* Local Bus Parity Error on Rcv Frame */

/* -------------------------------------------------------------------- */
/* 			Address registers				*/
/* -------------------------------------------------------------------- */
/* 
 * 	REG0 is reserved for station address
 */
#define FDDI_LONG_ADDR_REG0	(0x0001)
#define FDDI_ADDR_REG1		(0x0004)
#define FDDI_ADDR_REG2		(0x0008)
#define FDDI_ADDR_REG3		(0x0010)
#define FDDI_ADDR_REG4		(0x0020)
#define FDDI_ADDR_REG5		(0x0040)
#define FDDI_ADDR_REG6		(0x0080)
#define FDDI_ADDR_REG7		(0x0100)
#define FDDI_ADDR_REG8		(0x0200)
#define FDDI_ADDR_REG9		(0x0400)
#define FDDI_ADDR_REG10		(0x0800)
#define FDDI_ADDR_REG11		(0x1000)
#define FDDI_ADDR_REG12		(0x2000)

/* Bits used in the Transmit Control field (fddi_tx.ctl) Set by HOST */
#define FDDI_TX_CTL_BDV		(0x8000) /* buffer descriptor valid */
#define FDDI_TX_CTL_SOF		(0x2000) /* Start of Frame */
#define FDDI_TX_CTL_EOF		(0x1000) /* End of Frame */
#define FDDI_TX_CTL_IFX		(0x0800) /* Interrupt on Frame tx cmplt*/
#define FDDI_TRF_MSK		(0x0600) /* Mask for the trf bits in a short */

/* Bits used in the Transmit Status field (fddi_tx.stat) Set by NPOSW */
#define FDDI_TX_STAT_SV		(0x8000) 	/* Status valid */
#define FDDI_TX_STAT_AS		(0x2000)	/* AS: if set frame is sync */
#define FDDI_TX_STAT_SAM	(0x1000) 	/* Sync/Async mismatch */
#define FDDI_TX_STAT_AB		(0x0100) 	/* Aborted tx command */
#define FDDI_TX_STAT_NSF	(0x0010) 	/* No Start of Frame */
#define FDDI_TX_STAT_NEF	(0x0008) 	/* No End of Frame */
#define FDDI_TX_STAT_CPT	(0x0004) 	/* Card parity error on tx */
#define FDDI_TX_STAT_ILL	(0x0001) 	/* Illegal tx frame */
/*
 * Error bits in the status field
 *	FDDI_TX_STAT_ERR (indicate frame errors)
 *	FDDI_TX_STAT_BTI (big ticket items - are indicate a bad adapter)
 */
#define FDDI_TX_STAT_ERR	(FDDI_TX_STAT_AB | 	\
				FDDI_TX_STAT_ILL) 
#define FDDI_TX_STAT_BTI	(FDDI_TX_STAT_NSF | 	\
				FDDI_TX_STAT_NEF |	\
				FDDI_TX_STAT_SAM |	\
				FDDI_TX_STAT_CPT)

#define FDDI_SPURIOUS_THRESHOLD (50) 	/* threshold for spurious interrupts */
					/*  from the card */
#define FDDI_SMT_THRESH  (60)    /* The driver will report 1/60 smt err*/


/* RECEIVE bits for control: adap.ctl  (Set by HOST) */
#define FDDI_RX_CTL_BDV	(0x8000)	/* Buffer descriptor valid */
#define FDDI_RX_CTL_IFR	(0x4000)	/* Interrupt Frame Receive */

/* RECEIVE bits for statue: adap.stat  (Set by NPOSW) */
#define FDDI_RX_STAT_SV	(0x8000)	/* Status Valid */
#define FDDI_RX_STAT_SOF	(0x4000)	/* Start of Frame */
#define FDDI_RX_STAT_EOF	(0x2000)	/* End of Frame */
#define FDDI_RX_STAT_BTS	(0x1000)	/* Buffer to Small */
#define FDDI_RX_STAT_AB 	(0x0800)	/* Frame aborted */
#define FDDI_RX_STAT_LF 	(0x0400)	/* Local Frame (trf loopback)*/
#define FDDI_RX_STAT_AF 	(0x0200)	/* Adapter Generated Frame */
#define FDDI_RX_STAT_FCS	(0x0100) 	/* Bad Frame */

#define FDDI_CLBYTES		(4096)		/* HW Lim (pg size */
#define FDDI_RI_SA		(0x80)		/* Routing Info bit is source */
						/* 	address */
#define FDDI_GRP_ADDR		(0x80)		/* Group address bit */

/*
 * This is the layout of the TCW space for FDDI:
 *
 * Start of TCW's --> ________________
 *		     |  	      |
 *		     | 16*(4500 bytes |
 *		     | rounded up to  |
 *		     | a cache        |
 *		     | boundary) for  |
 *		     | rcv            |
 *		     |________________|
 *		     |  	      |
 *		     | 12 pages for   |
 *		     | tx smallframes |
 *       	     |________________|
 *		     |  	      |
 *		     |  24 pages for  |
 *		     |  tx 	      |
 *		     |________________|
 *		     |                |
 *		     |  24 pages for  |
 *		     |  download      |
 *		     |________________|
 */
#define	FDDI_SF_BUFSIZ		(2048)		/* SW Lim */
#define FDDI_SF_CACHE 		(FDDI_SF_BUFSIZ * FDDI_MAX_TX_DESC)


/*
 * Frame Control bits 
 */
#define FDDI_FC_SYNC    	(0x80)  /* FC class bit = 1 for sync data */
#define FDDI_FC_ADDR		(0x40)	/* Addr:  1 = 6 byte addr,0 = 2 byte */

#define FDDI_FC_MSK		(0xf0)	/* Mask of valid llc bits in fc */
#define FDDI_FC_LLC		(0x50)	/* LLC frames (mask) */
#define FDDI_FC_SMT		(0x40)	/* SMT frames (mask) */
#define FDDI_FC_NSA		(0x47)	/* NSA SMT Frames (mask) */
#define FDDI_FC_BEA		(0xC2)	/* MAC Beacon frames (mask) */

#define FDDI_ACLASS_SAS 	(0x0)	/* Attachment class = Single */
#define FDDI_ACLASS_DAS 	(0x1)	/* Attachment class = Dual */
#define FDDI_WORDSHIFT		(2)	/* log 2 for word aligned xmalloc */
#define FDDI_ROS_SIZE		(4)	/* ROS level size in the vpd */

/* -------------------------------------------------------------------- */
/* 			POS registers					*/
/* -------------------------------------------------------------------- */
#define FDDI_POS_REG0_VAL 	(0xf4)	/* POS reg 0 adapter ID code */
#define FDDI_POS_REG1_VAL 	(0x8e)	/* POS reg 1 adapter ID code */

#define FDDI_POS_REG0		(0x00)	/* Pos reg 0 base offset */
#define FDDI_POS_REG1		(0x01)	/* Pos reg 1 base offset */
#define FDDI_POS_REG2		(0x02)	/* Pos reg 2 base offset */
#define FDDI_POS_REG3		(0x03)	/* Pos reg 3 base offset */
#define FDDI_POS_REG4		(0x04)	/* Pos reg 4 base offset */
#define FDDI_POS_REG5		(0x05)	/* Pos reg 5 base offset */
#define FDDI_POS_REG6		(0x06)	/* Pos reg 6 base offset */
#define FDDI_POS_REG7		(0x07)	/* Pos reg 7 base offset */
/*
 * POS Register 2
 */
#define FDDI_POS2_DD		(0x08)	/* download/diagnostic bit */
#define FDDI_POS2_AR		(0x02)	/* card reset bit */
#define FDDI_POS2_CEN		(0x01)	/* card enable bit */

#define FDDI_INTR_LVL_9		(0x09)	/* interrupt level 9 */
#define FDDI_INTR_LVL_10	(0x0a)	/* interrupt level 10 */
#define FDDI_INTR_LVL_14	(0x0e)	/* interrupt level 14 */
#define FDDI_INTR_LVL_15	(0x0f)	/* interrupt level 15 */


/*
 * POS Register 3
 */
#define FDDI_POS3_SDE		(0x80)	/* Steaming Data enable */
#define FDDI_POS3_MSE		(0x40)	/* 64bit Streaming data enable */
#define FDDI_POS3_FAIR		(0x10)	/* enable fairness */
#define FDDI_POS3_ARB_MASK	(0x0f)	/* DMA arbitration mask */

/*
 * POS Register 4
 */
#define FDDI_POS4_NSE	 	(0x20)	/* No selected feedback enable */
#define FDDI_POS4_AIM		(0x10)	/* Auto increment mode for POS ext */ 
#define FDDI_POS4_ABM		(0x0c)	/* address burst management =
					 * 256 byte addr boundary crossing
					 */ 
#define FDDI_POS4_PSEL		(0x00)	/* parity select = enabale parity
					 * for MC data bus and MC addr bus
					 */

/*
 * POS Register 5
 */
#define FDDI_POS5_CHK		0x80	/* channel check bit */
#define FDDI_POS5_STAT		0x40	/* Status Field bit */
/*
 * Possible I/O adapter addresses
 */
#define FDDI_POS5_IO_7140	(0x7140)
#define FDDI_POS5_IO_7150	(0x7150)
#define FDDI_POS5_IO_7540	(0x7540)
#define FDDI_POS5_IO_7550	(0x7550)
#define FDDI_POS5_IO_7940	(0x7940)
#define FDDI_POS5_IO_7950	(0x7950)
#define FDDI_POS5_IO_7d40	(0x7d40)
#define FDDI_POS5_IO_7d50	(0x7d50)

/*
 * I/O adapter address mapping for the IAF setting
 * in POS register 5.
 */
#define FDDI_POS5_IAF_000	(0x00)	/* FDDI_POS5_IO_7140 */
#define FDDI_POS5_IAF_001	(0x08)	/* FDDI_POS5_IO_7150 */
#define FDDI_POS5_IAF_010	(0x10)	/* FDDI_POS5_IO_7540 */
#define FDDI_POS5_IAF_011	(0x18)	/* FDDI_POS5_IO_7550 */
#define FDDI_POS5_IAF_100	(0x20)	/* FDDI_POS5_IO_7940 */
#define FDDI_POS5_IAF_101	(0x28)	/* FDDI_POS5_IO_7950 */
#define FDDI_POS5_IAF_110	(0x30)	/* FDDI_POS5_IO_7d40 */
#define FDDI_POS5_IAF_111	(0x38)	/* FDDI_POS5_IO_7d50 */
/*
 * Possible Memory adapter addresses
 */

#define FDDI_POS5_MEM_1		(0x00100000)
#define FDDI_POS5_MEM_3		(0x00300000)
#define FDDI_POS5_MEM_5		(0x00500000)
#define FDDI_POS5_MEM_7		(0x00700000)
#define FDDI_POS5_MEM_9		(0x00900000)
#define FDDI_POS5_MEM_b		(0x00b00000)
#define FDDI_POS5_MEM_d		(0x00d00000)
#define FDDI_POS5_MEM_f		(0x00f00000)

/*
 * memory adapter address mapping for the MAF
 * setting in POS register 5.
 */
#define FDDI_POS5_MAF_000	(0x00)	/* FDDI_POS5_MEM_1 */
#define FDDI_POS5_MAF_001	(0x01)	/* FDDI_POS5_MEM_3 */
#define FDDI_POS5_MAF_010	(0x02)	/* FDDI_POS5_MEM_5 */
#define FDDI_POS5_MAF_011	(0x03)	/* FDDI_POS5_MEM_7 */
#define FDDI_POS5_MAF_100	(0x04)	/* FDDI_POS5_MEM_9 */
#define FDDI_POS5_MAF_101	(0x05)	/* FDDI_POS5_MEM_b */
#define FDDI_POS5_MAF_110	(0x06)	/* FDDI_POS5_MEM_d */
#define FDDI_POS5_MAF_111	(0x07)	/* FDDI_POS5_MEM_f */

/*
 * POS 7 Register values
 */
#define FDDI_POS7_DSEL_VPD	(0x00)	/* VPD rom */
#define FDDI_POS7_DSEL_IOL	(0x01)	/* I/O device addr lo */
#define FDDI_POS7_DSEL_IOH	(0x02)	/* I/O device addr hi */
#define FDDI_POS7_DSEL_MDA1	(0x03)	/* Memory device addr field 1 */
#define FDDI_POS7_DSEL_MDA2	(0x04)	/* Memory device addr field 2 */
#define FDDI_POS7_DSEL_MDA3	(0x05)	/* Memory device addr field 3 */
#define FDDI_POS7_VPS		(0x08)	/* Vital Products ROM Select */
#define FDDI_POS7_MDP		(0x80)	/* MC Data Bus Parity Error */

/*
 *  Misc: 
 */
#define FDDI_MAX_TX_DESC	(24)	/* # of desc in shared mem for tx*/
#define FDDI_MAX_RX_DESC	(16)	/* # of desc in shared mem for rcv*/
#define	FDDI_SMT_STATUS		(1)	/* for DDT */
#define FDDI_MAX_HDW_ADDRS	(12)	/* # of skyline address registers */
#define FDDI_MIN_NETMALLOC	(256)   /* Minimum size of the a netmalloc */

#define FDDI_SMT_EVNT_MSK	(0x0f180000)	/* unmask all the events */
#define FDDI_SMT_ERR_MSK	(0x02000000)	/* unmask all the errors */
#define FDDI_CMD_RETRY		(3)	/* SW Lim: num of times to retry cmd */
/*
 * Watchdog timer values
 *	The CMD_WDT_RESTART timer value must be at least as much as the
 *	Download value because the CMD_WDT_RESTART value is used to start
 *	the microcode after a successful download. If CMD_WDT_RESTART is
 *	set to 3 (or so) then the code must be changed so that the 
 *	START microcode command uses the Download value!!!
 */
#define FDDI_CMD_WDT_RESTART	(10)	/* cmd timeout in this many secs */
#define FDDI_DNLD_WDT_RESTART	(10)	/* download timeout in this many secs */
#define FDDI_RESET_WDT_RESTART	(15)	/* reset timeout in this many secs */
#define FDDI_CLOSE_WDT_RESTART	(15)	/* close timeout in this many secs */
#define FDDI_TX_WDT_RESTART	(90)	/* 90 sec watchdog timer */

/*
 * Conversion for dump timers
 */
#define	MSEC_PER_SEC	(NS_PER_SEC / NS_PER_MSEC)

/*
 * Component dump table stuff
 */
#define FDDI_CDT_ENTRIES	(32)	/* initial size of dump entries */
#define FDDI_DD_STR		"fddi_dd"

#define FDDI_STEST_CNT		(11)	/* Number of adapter self tests */
#define PIO_RETRY_COUNT       	(3)	/* PIO retry number */

/*
 * Get Trace #defines
 */
#define FDDI_AMDMEM_SIZE        (2*0x7f)  	/* sizeof AMD regs to dump */
#define FDDI_SKYMEM_SIZE        (2*0x1f)  	/* sizeof SKYLINE regs to dmp*/
#define FDDI_DATAMEM_SIZE       (0xf100)  	/* size of DATA store to dump */
#define FDDI_SRAM_SIZE          (2*0x00ff)      /* size of SHARED RAM to dump */


/*
 * #defines to define the min length acceptable in a call to mib_addr and the 
 * sizeof the mib_addr_elem structure for a FDDI address.  The structures themselves
 * are openended (generic not setup for an actual lan)
 */
#define FDDI_MIB_ADDR_MIN_LEN 	(16)
#define FDDI_MIB_ADDR_ELEM_LEN	(12)

#endif /* end if ! _H_FDDIBITS */
