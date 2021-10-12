/* @(#)73	1.9  src/bos/kernext/sol/sol_defs.h, sysxsol, bos411, 9428A410j 6/2/92 13:26:34 */
#ifndef _H_SOL_DEFS
#define _H_SOL_DEFS
/*
 * COMPONENT_NAME: (SYSXSOL) - Serial Optical Link Device Handler Include File
 *
 * FUNCTIONS: sol_defs.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define	NUM_SC_Q		16	/* number of anchors for subchannels */
#define CDDQ_TABLE_SIZE		(NUM_SC_Q * 2 + NUM_SET_Q + 1)
#define SLA_OUTQ		(NUM_SC_Q * 2)
#define SLA_INQ			(NUM_SC_Q * 2 + NUM_SET_Q)
#define CDDQ_HASH_MASK		0x0000000f  /* number of bits needed to hash */
#define	IRQ_TBL_SIZE 		32  	/* entries in hash anchor table */
#define	RQ_HASH_MASK 		0x0000001f  /* number of bits needed to hash */
#define	IRQ_RECEIVING		0x80
#define IMCS_STRICT_Q		0x80
#define IMCS_LIBERAL_Q		0x40
#define MAX_IMCS_SQ_ANCHOR	32
#define SQ_HASH_MASK	 	0x0000001f  /* number of bits needed to hash */
#define IMCSHELLO_VERSION	1
#define	HELLO_REQUEST		1
#define	HELLO_REPLY		2
#define HELLO_REJECT 		3
#define NSC_SWITCH		'n'
#define RS_6000			'r'
#define IMCS_PROC_LIMIT		255 /* valid pids are 1-254 only */
#define IMCS_FAKE_PID		0
#define INVALID_PID		IMCS_PROC_LIMIT
#define PID_PRESENT		1
#define PID_EVER_PRESENT	2
#define PID_HOLDING		4
#define SETQ1			0x40
#define SETQ2			0x20
#define IMCS_SETQ_LIMIT		4	
#define NUM_SET_Q		(IMCS_SETQ_LIMIT + 2)
#define LIMBO_QUEUE		(IMCS_SETQ_LIMIT)
#define FAKE_QUEUE		(IMCS_SETQ_LIMIT + 1)
#define INVALID_SETQ		NUM_SET_Q	/* null value for set q */
#define INVALID_SLAID		MAX_NUM_SLA
#define INVALID_SLA_ADDR	0
#define FORBIDDEN_SLA_ADDR	IMCS_PROC_LIMIT + 1 /* 2 byte value */
#define NO_ERROR		FALSE
#define TIME_OUT_FAIL		1
#define NOP_ERROR		2	
#define SIG_ERROR		3
#define OFF_ERROR		4
#define DRV_ERROR		5
#define BSY_ERROR		6
#define CHANNEL_CHECK		7
#define DID_NOT_START		8
#define ALA_RECEIVED		9
#define SLA_STOPPED		10
#define DIAG			11
#define SLADRIVE		12
#define MAX_NUM_SLA		4
#define MAX_SWITCH_PORT		16
#define NUM_TCWS		16
#define MAX_SWITCH_ADDR		17
#define ONE_MILLI		1000000	/* one millisecond=1 million nanosecs */
#define POLL_SEC		0
#define POLL_NSEC		(5 * ONE_MILLI)	
#define SEND_TIME_SEC		0
#define SEND_TIME_NSEC		(3 * ONE_MILLI)
#define BUSY_TIME_SEC		1
#define BUSY_TIME_NSEC		0
#define PBSY_SEC		1
#define PBSY_NSEC		(500 * ONE_MILLI)
#define POLL2_SEC		0
#define POLL2_NSEC		(ONE_MILLI)	
#define MIN_ALA_WAIT		(50 * ONE_MILLI)	/* 50 milliseconds */
#define MAX_ALA_WAIT		(70 * ONE_MILLI)	/* 70 milliseconds */
#define SYNC_TIME_SEC		0
#define SYNC_TIME_NSEC		(15 * ONE_MILLI)	/* 15 milliseconds */
#define MAX_START_TRY		3
#define	MAX_IDLE_COUNT		10
#define SLAS_DONE		0
#define	SLAS_FATAL		1
#define SLAS_SCR_PROGRESS	2
#define SLAS_UNEXPD		3
#define SLAS_DISCARDED		-1
#define SLAS_SCR_DONE		-2
#define NO_ACTION_CODE		0
#define CONTINUE_CODE		1
#define SYNC_CODE		2
#define WAIT_CODE		3
#define LFA_CODE		4	/* LFA = listen for ala */
#define XMT_UD_CODE		6
#define SEND_CCR		INITIATE
#define SCR_CCR			(DOSCR | PRIORITY | ISTP | RTV_4)
#define	XMT_CODE		(SEND_CCR | SXMT)
#define	XMT_INT_CODE		(SEND_CCR | SXMT | ISTP)
#define	XMT_DIS_CODE		(SEND_CCR | SXMT | EOF_DIS)
#define	XMT_DIS_INT_CODE	(SEND_CCR | SXMT | EOF_DIS | ISTP)
#define	XMT_AUTO_N_ACK_CODE	(SEND_CCR | ISTP | AUTO_SEQ | AUTO_ACK)
#define	XMT_AUTO_CODE		(SEND_CCR | ISTP | AUTO_SEQ)
#define XMT_CON_CODE		(SEND_CCR | SOF_CON | ISTP)
#define	LISTEN_CODE		ISTP
#define WRAP_1_CODE		DIAG_B
#define WRAP_10_CODE		DIAG_C
#define WRAP_NEIGHBOR_CODE	DIAG_A
#define LOCK_XTAL_ON_CODE	(DIAG_A | DIAG_B)
#define LOCK_XTAL_OFF_CODE	(DIAG_A | DIAG_B | DIAG_C)
#define	SLA_INTR_OFFSET		28
#define LINK_RESPONSE_TEST      0x01000000 /* LINK REPLY TO REQUEST TEST     */
#define LINK_REQUEST_TEST       0x09000000 /* LINK REQUEST TEST FRAME        */
#define LINK_RESPONSE_RJT       0x11000000 /* LINK RESP. REJECT		     */
#define DYNAMIC_PORT_RJT	0x19000000 /* SWITCH response reject         */
#define LINK_RESPONSE_BSY       0x21000000 /* NODE BUSY                      */
#define DYNAMIC_PORT_BSY	0x29000000 /* SWITCH BUSY                    */
#define LINK_RESPONSE_ACK       0x61000000 /* LINK RESP. ACK                 */
#define LINK_REQUEST_SCN	0x69000000 /* LINK REQ. STATE CHANGE         */
#define LINK_REQUEST_RESET      0x79000000 /* LINK REQ. RESET                */
#define LINK_REQUEST_ALA        0x71000000 /* LINK REQ. ASSIGN LINK ADDRESS  */
#define DEVICE_CTL_DATA		0x04000000 /* device control and data frame  */
#define RJT_LINK_SIGERR		0x01000000 /* link signal error */
#define RJT_CODE_VERR		0x02000000 /* code violation error */
#define RJT_FRAME_SERR		0x03000000 /* frame structure error */
#define RJT_CRC_ERR		0x04000000 /* crc error */
#define RJT_LINK_AERR		0x05000000 /* link address error */
#define RJT_RES_FERR		0x07000000 /* reserved field error */
#define RJT_UNINS_LCF		0x08000000 /* uninstalled link ctrl func */
#define	RJT_NON_SPEC_CODE	0x09000000 /* non-specific */
#define PRJT_ADDR_ERR		0x10000000 /* address invalid error */
#define PRJT_UNDA_ERR		0x11000000 /* undefined address dest err */
#define PRJT_DESTP_MALF		0x12000000 /* dest port malfunction */
#define PRJT_DSINT_REQ		0x13000000 /* dynamic switch intervention required */
#define RJT_SID_ADDR_ERR	0x00010000 /* source address */
#define RJT_CONT_ERR		0x00020000 /* contention */
#define RJT_LCTL_FUN_ERR	0x00030000 /* link ctl function */
#define RJT_RST_DISABLE		0x00040000 /* reception of reset frame */
#define RJT_UNINS_DCF		0x00050000 /* uninstalled data ctl func. */
#define PRJT_DID_ADDR_ERR	0x00000000 /* destination invalid */
#define PRJT_SID_ADDR_ERR	0x00010000 /* source id invalid */
#define PRJT_LINK_FAIL_ERR	0x00000000 /* link failure */
#define PRJT_NO_RESP_ERR	0x00010000 /* no response */
#define PRJT_OFF_ERR		0x00000000 /* offline mode */
#define PRJT_STAT_CON_ERR	0x00010000 /* static connection */
#define PRJT_FENCED_ERR		0x00020000 /* fenced port */
#define PRJT_PCM_VIOL_ERR	0x00030000 /*Partition Control Mask Violation */
#define PRJT_NOT_INST_ERR	0x00040000	/* Optical card not installed */
#define	DEV_REQ_SEND		0x00000000	/* request to send */
#define	DEV_REQ_SEND_IMM	0x10000000	/* request to send immediate */
#define	DEV_REQ_RCV		0x20000000	/* request to receive */
#define	DEV_REQ_INT		0x30000000	/* request interrupt */
#define	DEV_RESP_SEND		0x40000000	/* response to send */
#define	DEV_RESP_ACK		0x50000000	/* response ack */
#define	DEV_RESP_NAK		0x60000000	/* responce nack */
#define	DEV_RESP_ACKT		0x80000000	/* responce ack turn around */
#define	DEV_SEND		0x70000000	/* send */
#define	DEV_FG_DATA_END		0x00800000	/* data end */
#define	DEV_FG_BUF_256		0x00000000	/* 256 bytes */
#define	DEV_FG_BUF_512		0x00080000	/* 512 bytes */
#define	DEV_FG_BUF_1024		0x00100000	/* 1024 bytes */
#define	DEV_FG_NB_2		0x00000000	/* 2 buffers */
#define	DEV_FG_NB_4		0x00010000	/* 4 buffers */
#define	DEV_FG_NB_6		0x00020000	/* 6 buffers */
#define	DEV_FG_NB_8		0x00030000	/* 8 buffers */
#define	CSOF_RCVD		0x80000000	/* sof rcvd:1=conn.,0=passive*/
#define	NOEOF			0x00000000 	/* no eof */
#define	PEOF     		0x20000000	/* passive eof */
#define	DEOF     		0x40000000	/* disconnect eof */
#define	EOF_ABORT		0x60000000	/* abort */
#define	CRC_CHK			0x10000000	/* link crc check */
#define	STRUCT_CHK		0x08000000	/* frame structure check */
#define	CDVL_CHK		0x04000000	/* link code violation check */
#define	SFRS_CHK		0x02000000	/* send frame size check */
#define	IOREG			15 	/* segment for addressing the sla */
#define SLA_CCR_SZ		4	/* sizeof(struct slaregs.ccr) */
#define SLA_TCW_SZ		4	/* sizeof(struct slaregs.tcw[0]) */
#define SLA_TCWS_SZ		(SLA_TCW_SZ * NUM_TCWS)
#define SLA_CHPR_SZ		(SLA_THR_SZ + SLA_CCR_SZ + SLA_TCWS_SZ)
#define NATIVE			0x00000000
#define TRANSPARENT		0x10000000
#define DOSCR			0x20000000
#define ACTIVATE		0x30000000
#define BUFAC			0x40000000	/* buffer access */
#define PRIORITY		0x08000000	/* priority override bit 4 */
#define INITIATE		0x04000000	/* initiate transfer  bit 5 */
#define AUTO_SEQ		0x02000000	/* enable auto sequence bit 6 */
#define SXMT			0x01000000	/* stop after transfer  bit 7 */
#define RETRY			0x00800000	/* auto retry  bit 8 */
#define PTPD			0x00400000	/* point to point discard  9 */
#define DRTO			0x00200000	/* disable response time out */
#define	AUTO_ACK		0x00100000	/* auto ack enable bit 11 */
#define	AUTO_BUSY		0x00080000	/* auto busy (always ON) 12 */
#define	SOF_CON			0x00040000	/* start of frame connect 13 */
#define	SOF_PAS			0x00000000	/* start of frame passive 13 */
#define	EOF_DIS			0x00020000	/* end of frame disconnect 14 */
#define	EOF_PAS			0x00000000	/* end of frame passive 14 */
#define	RSTD			0x00008000	/* reset disable bit 16 */
#define	ISTP			0x00004000	/* interrupt when stopped 17 */
#define	AUTO_SCR		0x00002000	/* automatic response to SCR */
#define	RTV_1			0x00000000	/* response time out 1 sec. */
#define	RTV_4			0x00000800	/* response time out 4 sec. */
#define	DRIVER			0x00000080	/* driver on */		
#define DIAG_A			0x00000004
#define DIAG_B			0x00000002
#define DIAG_C			0x00000001
#define XMT_RUN			0x00000000	/* transmit from run state 13 */
#define XMT_CTRY		0x00040000	/* tx from connect try state*/
#define SB_XMT			0x00000000	/* single buffer trasmit 14 */
#define CS_XMT			0x00020000	/*continuous sequence transmit*/
#define BUF_WRITE		0x04000000	/* buffer write bit 5 */
#define BUF_READ 		0x00000000	/* buffer read */
#define	PROG_CHK		0x80000000	/* programming check */ 
#define	LINK_CHK		0x40000000	/* link check *//* bit 1 */
#define	CHAN_CHK		0x20000000	/* internal channel check */
#define	UNEXPD			0x10000000	/* unexpected frame  bit 3 */
#define SCR_DONE		0x04000000	/* SCR complete  bit 5 */
#define SCR_ACTV		0x02000000	/* SCR active bit 6 */
#define CMD_RJT			0x01000000	/* command reject  bit 7 */
#define CMD_RJT2		0x00800000	/* secondary command reject */
#define RESP_TO			0x00400000	/* response time out */
#define ABT_XMT			0x00100000	/* abort transmitted */	
#define	FRM_DISC		0x00010000	/* frame discarded */
#define	BSY_DISC		0x00008000	/* busy frame discarded*/
#define RJT_DISC		0x00004000	/* reject frame discarded */
#define	OP_DONE			0x00000800	/* operation completed */
#define CMD_PEND		0x00000200	/* command pending bit 22 */
#define PRIMARY			0x00000100	/* primary frame received */ 
#define PUI_NOOP		0x00000000	/* not operational */
#define PUI_STOPPED		0x00000008	/* stopped */
#define	PUI_WORK1		0x00000010	/* working 1 */
#define	PUI_WORK2		0x00000018	/* working 2 (cmd pending) */
#define	PUI_OFFLINE		0x00000020
#define	LI_CWAIT		0x00000000	/* connect wait */
#define	LI_CTRY			0x00000001	/* connect try */
#define	LI_LISTEN		0x00000002	/* listen */
#define LI_RUNNING		0x00000003	/* running */
#define S1_BIT_MASK		0xffffffc0	/* all bits in s1,except link*/
#define RESERVED_S1		0x082e3400	/* reserved bits in s1 */
#define RBUFCHK			0x80000000	/* receive buffer check */
#define XBUFCHK			0x40000000	/* transmit buffer check */
#define CMDCHK			0x20000000	/* command check */
#define TAGPARITY		0x04000000	/* tag parity check */	
#define BUFPARITY		0x02000000	/* buffer parity check */
#define STORCHK			0x01000000	/* storage access check */
#define RST_RCVD		0x00800000	/* link reset received */
#define SNDCNTCHK		0x00400000	/* send count check */	
#define ADDR_MIS		0x00200000	/* link adrress mismatch */
#define SIG_FAIL		0x00080000	/* signal failure */
#define XMT_FAIL		0x00040000	/* xmt driver failure */
#define	OFFSQ_REC		0x00000040	/* offline seq. recognized */
#define	NOPSQ_REC		0x00000020	/* non-op. seq recognized  */
#define UDSEQ_REC		0x00000010	/* UD sequence recognized  */
#define	UDRSEQ_REC		0x00000008	/* UDR sequence recognized  */
#define	XMTE_FAIL		0x00000004	/* xmt driver early failure  */
#define	LINK_SIG_ERR		0x00000002	/* signal error */
#define	NO_OPCARD   		0x00000001	/* optical card absent */
#define RESERVED_S2		0x1813ff00	/* reserved bits in s2 */
#define SLAC_INIT		0x00	/* initial sla 3.0E fifo, dma, 
					   clock bug */
#define SLAC_CMB2E		0x01	/* 2E combo 3.1E dma and clock bug */
#define SLAC_CMB2S		0x02	/* 2S combo 3.2S clock bug */
#define SLAC_CMB3E		0x03	/* 2E combo 3.2E no known bugs */
#define SLAC_4S			0x11	/* 4 sla chip clock bug */
#define	LAST_TAG		0xffffffc0
#define THR_DONT_CARE		0	/* a value for unused field */ 
#define SWITCH_LA		((uchar) 0xfe)	/* address of the switch */
#define DX_SWITCH_LA		((uchar) 0xfd)	/* address of the dx switch */
#define SWITCH_ALA_ADDRESS 	0xff000000	/* adrs in 1st ala sequence */
#define RJT_DID_ADDRESS		0xff000000	/* address send reject frames */
#define BSY_DID_ADDRESS		0xff000000	/* address send busy frames */
#define RST_ACK_ADDRESS		0xff000000	/* address to link ack frame */ 
#define NUM_CONT_TCWS		4
#define OFF_SEQ_W1		0xc1735c14	/* k 28.5+, d 24.2-, k 28.5+ */
#define OFF_SEQ_W2		0xcd705cd4	/* d 24.2-, k 28.5+, d 24.2- */
#define UD_SEQ_W1		0xc1575c14	/* k 28.5+, d 15.2-, k 28.5+ */
#define UD_SEQ_W2		0x5d7055d4	/* d 15.2-, k 28.5+, d 15.2- */
#define UDR_SEQ_W1		0xc15b5c14	/* k 28.5+, d 16.2-, k 28.5+ */
#define UDR_SEQ_W2		0x6d7056d4	/* d 16.2-, k 28.5+, d 16.2- */
#define IDLE_SEQ_W1		0xc14fac14	/* k 28.5+, k 28.5-, k 28.5+ */
#define IDLE_SEQ_W2		0x3eb053e8	/* k 28.5-, k 28.5+, k 28.5- */
#define SOF_WORD		0x3eb053e0	/* k 28.5-, k 28.5+, k 28.7- */
#define SLA_ADDR_RANGE 		256
#define RCV			TRUE
#define DO_NOT_RCV		FALSE
#define DCL_DONE		0
#define DCL_TOO_SMALL		101
#define DCL_TOO_LARGE		102
#define DCL_NO_RANGE		103
#define DCL_IN_USE		104
#define DCL_OUT_BLOCK		105
#define DCL_TYPE		106
#define DCL_STRICT_Q		0x80
#define DCL_LIBERAL_Q		0x40
#define IMCS_INT_PRIORITY	INTCLASS2
#define RESET			1
#define WRAP_N			2
#define	SCH_SDMA		0x001f	/* send dma unhang */
#define IMCS_SDMA_QUEUE		0x001e	/* dma unhang */
#define IMCS_SCHR_IP		0x0080
#define SCH_IP			0x0081
#define SCH_IPCL		0x0082  /* ip clusters */
#define IMCS_AUDITOR_QUEUE	0x0181	/* the imcs auditor queue. */ 
#define IMCS_DEBLOCK_QUEUE	0x0182	/* one of imcs own queues*/
#define IMCS_PAGER_QUEUE	0x0183	/* queue for the distributed */
#define NA			(void (*)()) NULL
#define IMCS_DONE		0
#define QUEUE_NOT_DCL		-201
#define STACK_FULL		-202
#define SIZE_OF_SLAS		(16*4)	/* size of sla structure in bytes */
#define SLA_INITIAL_CODE	0x00
#define SLA_RECEIVING_CODE	0x01
#define SLA_SENDING_CODE	0x02
#define SLA_WORKING_CODE	0x03	/* or of receiving and sending */
#define SLA_FAILED_CODE		0x04
#define SLA_BROKEN_CODE		0x08
#define SLA_BLOCKED_CODE	0x80
#define SLA_IDENT_CODE		0x00
#define SLA_NOTIDENT_CODE	0x01
#define SLA_NOTSYNC_CODE	0x02
#define SLA_POLLMODE_CODE	0x04
#define SLA_DIAGNOSTIC_CODE	0x08
#define SWITCH_PRESENT		0x80
#define POINT_TO_POINT		0x40
#define DX_SWITCH_PRESENT	0x20
#define	OFF_SEQ_ADDR		0
#define	UD_SEQ_ADDR		256
#define	UDR_SEQ_ADDR		512
#define SOF_SEQ_ADDR		768
#define ALA_SENT		1
#define LISTENED_FOR_ALA	2
#define ALA_SENT_TO_SWITCH	4
#define ALA_NULL		0
#define ACK_SENT		1
#define ACK_TURN		2
#define IMCS_VERSION_CLONE	'D'
#define IMCS_VERSION_N		2
#define	NUM_HDR_TCWS		16
#define	IMCS_RTSI_SND_CODE	0xe000	
#define	IMCS_RTSI_RCV_CODE	0x2000
#define IMCS_RTS_SND_CODE	0xc000
#define	IMCS_RTS_RCV_CODE	0x0000
#define	IMCS_RTR_SND_CODE	0x8000
#define	IMCS_RTR_RCV_CODE	0x4000
#define IMCS_SEND_CODE		0x1
#define IMCS_RECEIVE_CODE	0x0
#define IMCS_INITIATE		0x1
#define IMCS_DELAY		0x0
#define IMCS_ACK		SOL_ACK	/* imcs ack */
#define IMCS_NACK_NQ		1	/* nack - non-existent queue */
#define IMCS_TAG_CHK		2	/* tag words check */
#define IMCS_NACK_NB		SOL_NACK_NB
					/* nack - not enough header or buffer */
#define IMCS_NACK_NR		SOL_NACK_NR
					/* nack - queue is not receiving */
#define IMCS_NACK_NS		SOL_NACK_NS
				 	/* nack - non-existent subchennel */
#define IMCS_NACK_RTSI		6	/* rtsi is not ready */
#define IMCS_NEVER_CONN		SOL_NEVER_CONN
					/* message could not be delivered */
#define IMCS_NO_CONN		SOL_NO_CONN
					/* message could not be delivered */
#define IMCS_DOWN_CONN		SOL_DOWN_CONN
					/* message could not be delivered */
#define IMCS_DYING		10	/* imcs is stopping */
#define IMCS_LAST_ACK		11  
#define IMCS_INIT_GCK		0x0100	/* waiting for sla */
#define IMCS_DEBLOCKED		0x0200	/* queue is deblocked */
#define IMCS_SLA_ENQ		0x0300	/* enqueued to go out */
#define IMCS_CDD_ENQ		0x0400	/* enqeueued at the cdd level */
#define NULL_HDR		(struct imcs_header *) NULL
#define LAST_TCW		0xffffffff   /* marks the last tcw in a set */
#define IMCS_LINE_SIZE		64	/* imcs line size in bytes. */ 
#define IMCS_LOG2_LINE_SIZE	6	/* log base 2 of IMCS_LINE_SIZE.  */
#define IMCS_HDR_SIZE		128	/*  bytes must equal sizeof(imcshdr) */
#define IMCS_RTSI_UHDR_SIZE	128	/* size of user portion of the header */
#define IMCS_RTSI_HDR_SIZE	(IMCS_RTSI_UHDR_SIZE + IMCS_HDR_SIZE) 
#define IMCS_RTSI_SHDR_SIZE	128	/* portion of header actually sent */
#define L2I_RTSI_HDR_SIZE	8	/* log base 2 of IMCS_RTSI_HDR_SIZE */
#define SLA_ERROR		0	/* error detections */
#define	PCC_RCV_BUF_CHK		1	/* received buffer checks */
#define	PCC_XMT_BUF_CHK		2	/* transmit buffer checks */
#define	PCC_CMD_CHK		3	/* command checks */
#define	CCC_TAG_PAR		4	/* tag parity */
#define	CCC_BUF_PAR		5	/* buffer parity */
#define	CCC_STOR_CHK		6	/* storage check */
#define	LCC_FRM_DISC		7	/* frames discarded */
#define	LCC_BSY_DISC		8	/* busy discarded */
#define	LCC_RJT_DISC		8	/* rejects discarded */
#define	LCC_ABT_XMT		10 	/* abort transmitted */
#define LCC_RST_RCVD		11	/* reset received */
#define	LCC_SND_CNT_CHK		12	/* send count checks */
#define	LCC_ADDR_MIS		13	/* address mismatch */
#define	LCC_SIG_FAIL		14	/* signal failure */
#define	LCC_DRV_FAIL		15	/* transmit driver failure */
#define	LCC_OFFSQ_RC		16	/* offline sequence recognized */
#define	LCC_NOPSQ_RC		17 	/* not-operational seq. recognized */
#define	RCC_ABORT		18	/* abort eof */
#define	RCC_CRC_CHK		19	/* crc check */
#define	RCC_STRUCT_CHK		20	/* frame structure check */
#define	RCC_CDVL_CHK		21	/* code violation check */
#define	RCC_SFRS_CHK		22	/* send frame size check */
#define BUG_MISS_INT		23	/* missed interrupts */
#define BUG_ABT_XMT		24	/* abort trasmitted by itself */
#define BUG_LINK_CHK		25	/* link check was returned to start */
#define BUG_INT_RUN		26	/* interrupt with channel running */
#define ALA_ADDR_MIS		30	/* address mismatch during ala */
#define ALA_SENDS		31	/* number of ala frame sent */
#define ALA_TIME_OUT		32	/* time out during ala */
#define ALA_BSY			33	/* frames busied during ala */
#define ALA_ACK_SENDS		34
#define ALA_RJT_SENDS		35
#define ALA_RCVD		36
#define ALA_RJT_RCVD		37
#define	CON_TRY			40	/* calls to sla_send */
#define	CON_NOSTOP		41 	/* channel did not stop in sla_send */
#define	CON_IN			42 /* stop in sla_send rets. interruptible s1 */
#define	CON_SEND		43	/* calls to start_send_auto */
#define	CON_FRCOLL		44 	/* frame collision */
#define	CON_PFRCOLL		45 	/* potential frame collision */
#define CON_BUSY		46	/* sla is busy receiving */
#define	OTH_INTS		50 	/* interrupts */
#define	OTH_TIME_OUT   		51	/* time outs */
#define OTH_UNKN_INT		52	/* interrupts due to no reason */
#define OTH_UNKN_DEV		53	/* unknown device control frame */
#define OTH_UNKN_LINK		54	/* unknown link control frame */
#define OTH_NO_START		55	/* channel would not start */
#define OTH_UNEXPD		56	/* channel start returns unexpected */
#define	OTH_SPINS		57 	/* max number of spins */
#define OTH_SCR_START		58	/* SCR started */
#define OTH_SCR_DONE		59	/* (no UD no UDR) seen at end of SCR */
#define	OTH_LBSY		61 	/* link busy frames */
#define	OTH_PBSY		62 	/* port busy frames */
#define OTH_RDMA		63	/* calls to rdma */
#define OTH_SDMA		64	/* calls to sdma */
#define OTH_DONE_FR		65	/* op done with wrong frame */
#define OTH_BAD_FR		66	/* bad frame */
#define OTH_UNIMP_FR		67	/* unimplemented frame */
#define OTH_UNX_FR		68	/* unexpected frame */
#define OTH_DIAG		69	/* diagnostics */
#define OTH_STOP		69	/* calls to close */
#define OTH_REAL_BSY		69	/* link busy for over a second */
#define MAX_NUMBER_CNT		70
#define MAX_NUM_SLA   		4
#define BUFF 			256
#define LINE 			64
#define OFF 			0
#define ON 			1
#define DONE			2
#define TRUE 			1
#define FALSE	 		0
#define SLEEP   		TRUE
#define DO_NOT_SLEEP    	FALSE
#define SLATICKSPERSEC   	1000   /* Ticks per Second */
#define RETRY_COUNT      	10 /* max number of retries when NAK   */
#define SLA_STATUS               0
#define SLA_GET_READ_LOG         1
#define SLA_SET_DEBUG            2
#define SLA_RESET_DEBUG          3
#define SLA_SET_DIAG_MODE        4
#define SLA_RESET_DIAG_MODE      5
#define SLA_DIAG_01              6
#define SLA_DIAG_02              7
#define SYNC_OTP                 8
#define CANCEL_SLA               9
#define CARD_PRES                10
#define SLA_RESET_READLOG        11
#define SLA_RESET_WRAP           12
#define SLA_EOF         	0
#define SLA_RESTART     	1
#define PAGE_SIZE        	4096
#define PING             	0
#define PONG             	1
#define SLAEB_L2SIZE		5
#define SLAEB_SIZE		(1 << SLAEB_L2SIZE)
#define SLAEB_WRAP_VALUE	(SLAEB_SIZE - 1)
#define INTERRUPT_PENDING       \
        (PROG_CHK | LINK_CHK | CHAN_CHK | UNEXPD | SCR_DONE | RESP_TO | OP_DONE)

#define L2PAGESIZE      	PGSHIFT
#define L2HDRS_IN_PAGE  	(L2PAGESIZE - L2I_RTSI_HDR_SIZE)
#define HDRS_IN_PAGE   	 	(1 << L2HDRS_IN_PAGE) /*no. of headers a page */
#define MIN_HDR_PAGES   	1     /* minimum pages allocated for headers */

#define L2PAGESIZE 		PGSHIFT
/* time to retry hello message if header is being used */
#define HLMSG_RS        0
#define HLMSG_RNS       (ONE_MILLI)

/* time to retry hello message if the other side is not set up for RTSI */
#define HLMSG_RTSI_RS   0
#define HLMSG_RTSI_RNS  (25 * ONE_MILLI)

/* time between successive hello messages on the same sla */
#define HLMSG_NEXT_RS   0
#define HLMSG_NEXT_RNS  (2 * ONE_MILLI)

/* after a failure, wait this much before recovering */
#define RC_DEL_S        0
#define RC_DEL_NS       0

#define	RESTART_ALA 		1
#define RESTART_SYNC		2 
#define RESTART_CONT		3
#define RESTART_RECOVERY	4
#define RESTART_RECPOLL		5
#define RESTART_CONTSCR		6
#define SLA_ADDR_UNH		7
#define RESTART_INT		8
#define START_ALA		9
#define RESTART_SEND 		10
#define TIMER_RECOVER 		11
#define RESTART_HELLO		12
#define TIMER_HELLO		13 
#define DIAG_TIMER		14 
#define LOCK_XTAL_ON		15
#define LOCK_XTAL_OFF		16
#define LOCK_XTAL_DONE		17
#define	SEND_NEXT		18
#define QUEUE_FLUSH		19
#define RESTART			20

#define HANDFULL 		6

#define MAX_ERRORS		8 /* currently 2 types of frame errors */

#endif _H_SOL_DEFS
