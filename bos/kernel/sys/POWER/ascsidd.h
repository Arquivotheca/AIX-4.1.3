/* @(#)49	1.8  src/bos/kernel/sys/POWER/ascsidd.h, sysxscsi, bos412, 9445C412a 10/28/94 14:16:03 */
#ifndef _H_ASCSIDD
#define _H_ASCSIDD
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Device Driver
 *
 * FUNCTIONS:	NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * COMPONENT:   SYSXSCSI 
 *                                                
 * NAME:        ascsidd.h
 *                                               
 * FUNCTION:    IBM SCSI Adapter Driver Header File
 *                                                                
 *		Header file for the Fast Wide SCSI-2 adapter driver.
 *
 *      	The adapter driver is the interface between the SCSI 
 *		protocol layer and the SCSI adapter. It sends commands 
 *		to the adapter on behalf of the SCSI head device drivers.
 */

#include <sys/watchdog.h>
#include <sys/timer.h>
#include <sys/dump.h>
#include <sys/err_rec.h>
#include <sys/xmem.h>
#include <sys/scsi.h>
#include <sys/scb_user.h>
#include <sys/ndd.h>
#include <sys/cdli.h>

/*
 *  Global storage area used to manage
 *  all adapters.
 */
typedef volatile struct {
#define		MAX_ADAPTERS	0x10 			/* MAX supported adp */
	int	semaphore;				/* semaphore	     */
        int     num_of_opens; 				/* adp's open count  */
        int     num_of_cfgs; 				/* adp's open count  */
        struct  adapter_info  *ap_ptr[MAX_ADAPTERS];	/* ap ptrs for each  */
							/* adapter, indexed  */
							/* by sequence number*/
} adp_ctrl_t;

/*
 *  Surrogate Control Areas.
 *  The surrogate control areas are used to communicate pipe
 *  status between the driver and the adapter.
 */
struct	surr_ctl	{
	ushort	*eq_ssf;			/* outbound SSF		*/
	ushort	*eq_ssf_IO;			/* outbound SSF IO addr	*/
	ushort	*eq_ses;			/* outbound SES		*/
	ushort	*eq_ses_IO;			/* outbound SES IO addr	*/
	ushort	*dq_sse;			/* inbound SSE		*/
	ushort	*dq_sse_IO;			/* inbound SSE IO addr	*/
	ushort	*dq_sds;			/* inbound SDS		*/
	ushort	*dq_sds_IO;			/* inbound SDS IO addr	*/
	ushort	*dq_ssf;			/* inbound SSF		*/
	ushort	*dq_ssf_IO;			/* inbound SSF IO addr	*/
	ushort	*dq_ses;			/* inbound SES		*/
	ushort	*dq_ses_IO;			/* inbound SES IO addr	*/
	ushort	*eq_sse;			/* outbound SSE		*/
	ushort	*eq_sse_IO;			/* outbound SSE IO addr	*/
	ushort	*eq_sds;			/* outbound SDS		*/
	ushort	*eq_sds_IO;			/* outbound SDS IO addr	*/
	ulong	*pusa;				/* peer unit signal area    */
	ulong	*pusa_IO;			/* peer unit signal area    */
	ulong	*ausa;				/* adapter unit signal area */
	ulong	*ausa_IO;			/* adapter unit signal area */
};

/*
 *  Surrogate area status bit definitions.
 */
#define	WRAP_BIT	0x0001
#define	EMPTY_BIT	0x0002
#define	EMPTY_BIT_REV	0x0200
#define	EQ_PIPE_EMPTY	0x0002
#define	EQ_PIPE_FULL	0x0100


/*
 *  The Local Control Areas.
 *  This structure contains fields used by the driver to manage
 *  both the inbound(dequeue) and outbound(enqueue) pipes.
 */
struct	local_ctl {
	uchar	*eq_sf;			/* enqueue start of free */
	uchar	*eq_ef;			/* enqueue end of free */
	uchar	*eq_se;			/* enqueue start of element */
	uchar	*eq_top;		/* eq num of bytes left for ctl elems */
	uchar	*eq_end;		/* eq num of bytes to end of pipe */
	uchar	*dq_ee;			/* dequeue end of elements addr */
	uchar	*dq_se;			/* dequeue start of elements addr */
	uchar	*dq_top;		/* dequeue ptr to end of usable area */
	uchar	*dq_end;		/* dequeue ptr to physical end pipe */
	ushort	eq_wrap;		/* enqueue wrap elem addr */
	ushort	dq_wrap;		/* dequeue wrap elem addr */
	ushort	eq_status;		/* status of the enqueue pipe */
	ushort	dq_status;		/* status of the dequeue pipe */
};

/*
 *  Adapter's pool of ctl_elem_blk's
 */
struct	adp_ctl_elem	{
	struct	ctl_elem_blk	ctl_blk;	/* control element block    */
	uchar	allocated;			/* in-use flag		    */
	uchar	cmd_pending;			/* outstanding command 	    */
	uchar	status;				/* command status 	    */
	uchar	reserved;			/* pad			    */
	int	event;				/* sleep event word	    */
#define	REPLY_SIZE	0x30
#define NUM_CTRL_ELEMS  0x25			/* ctl_elm_blk pool size    */
	uchar	reply[REPLY_SIZE];		/* reply element	    */
};

/*
 *  Structure used to manage the watchdog timer and
 *  the timing of adapter commands.
 */
struct wtimer    {
	struct	watchdog dog;               /* the watchdog struct           */
	struct	adapter_info	*ap;	    /* ptr to adapter struct 	     */
	ushort	reason;			    /* reason timer was set	     */
};
#define	INVAL_TMR		0	    /* phantom timeout	     	     */
#define	RESET_TMR		1	    /* timing a reset adap request   */
#define LOCATE_TMR		3           /* locate mode SCB cmd timeout   */
#define MOVE_TMR		4           /* move mode SCB cmd timeout     */
#define RESET_DURATION		30	    /* resets may take this long     */
#define SCB_CMD_DURATION	30	    /* adapter command duration      */

/*
 *  Structure to manage the Small Transfer Area.
 */
struct sta_str  { 
	uint    in_use;				/* TRUE if this area in use  */
	char    *stap;				/* pointer to this xfer area */
};

/*
 *  Define Device Structure (DDS) of the SCSI Adapter Driver.
 */
struct ascsi_ddi {
	ulong		bus_id;		/* adapter I/O bus id value     */
	ushort		bus_type;	/* adapter I/O bus type         */
	uchar		slot;		/* I/O slot number              */
	uint		base_addr;	/* adapter base address         */
	char		resource_name[16];  /* resource name for err log*/
	int		battery_backed;	/* TRUE if this adapter and its */
					/* SCSI devices are batt-backed */
	int		dma_lvl;	/* dma level                    */
	int		int_lvl;	/* interrupt level              */
	int		int_prior;	/* interrupt priority           */
	int		ext_bus_data_rate;/* ext bus data rate		*/
	ulong		tcw_start_addr;	/* start addr of bus tcw space  */
	int		tcw_length;	/* length (in bytes) of tcw area*/
	int		tm_tcw_length;	/* target mode tcw length       */
	ulong		tm_tcw_start_addr; /* TCW start addr for target mode */
	uchar		i_card_scsi_id;	/* the adapter's int SCSI id     */
	uchar		e_card_scsi_id;	/* the adapter's SCSI id        */
	uchar		int_wide_ena;	/* wide neg. enabled on int bus	*/
	uchar		ext_wide_ena;	/* wide neg. enabled on ext bus	*/
};


/*
 *  Adapter structure, one per adapter.
 */
struct  adapter_info {
        struct  intr     intr;                  /* interrupt handler struct  */
        ndd_t   ndd;				/* pointer to ndd element    */
	dev_t	seq_number;			/* adapter's device number   */
	struct  adapter_info	*next;          /* next ap struct	     */
	struct	local_ctl	local;		/* struct to manage pipes    */
	struct	ascsi_ddi	ddi;		/* struct passed in at config*/
	struct	ctl_elem_blk	*active_head;	/* first elem in active que  */
	struct	ctl_elem_blk	*active_tail;	/* last elem in active que   */
	struct	ctl_elem_blk	*wait_head;	/* first elem in wait queue  */
	struct	ctl_elem_blk	*wait_tail;	/* last elem in wait queue   */
	ushort	num_cmds_queued;		/* # cmds queued counter     */
	ushort	num_cmds_active;		/* # cmds active counter     */
	struct	adp_ctl_elem	*adp_pool; 	/* adapter's ctrl_elem pool  */
	struct	surr_ctl	surr_ctl;	/* control area		     */
#define STA_SIZE         	0x100
#define STA_ALLOC_SIZE  	PAGESIZE
#define NUM_STA         	(STA_ALLOC_SIZE/STA_SIZE)
        struct	sta_str		sta[NUM_STA];	/* STA mgmt table	     */
	struct	timestruc_t	time_s;		/* curtime structure         */
	uint	*tcw_table;			/* to manage tcw allocation  */
        uchar   opened;				/* opened flag		     */
        uchar   adapter_mode;			/* adapter operating mode    */
#define	LOCATE  0
#define	MOVE    1
	ushort	adp_uid;			/* adapter unit id	     */
	ushort	peer_uid;			/* peer unit id	     	     */
	uchar	*sysmem;			/* system memory base addr   */
	uchar	*sysmem_end;			/* system memory pointer     */
	uchar	*busmem;			/* bus memory pointer        */
	uchar	*busmem_end;			/* bus memory pointer        */
	uchar	*tm_tcw_table;			/* to mng TM tcw alloc table */
	uchar	*eq_raddr;			/* enqueue pipe io address   */
	uchar	*dq_raddr;			/* dequeue pipe io address   */
	uchar	*eq_vaddr;			/* enqueue pipe virtual addr */
	uchar	*dq_vaddr;			/* dequeue pipe virtual addr */
	uchar	*sta_raddr;			/* STA IO address   	     */
	uchar	*sta_vaddr;			/* STA virtual addr 	     */
	uchar	*bufs;				/* io address of the buffers */
	uchar	*tm_sysmem;			/* system memory base addr TM*/
	struct	wtimer	wdog;			/* watchdog timer struct     */
	struct	wtimer	tm;			/* watchdog timer struct  TM */
	struct  trb	delay_trb;    		/* timer block for cmd delay */
	struct	xmem	xmem;			/* local xmem descriptor     */
	uint	dma_channel;			/* channel id from d_init    */
	uint	mtu;				/* max transfer unit         */
	uint	num_tcw_words;			/* number of TCWs in pool    */
	uchar	shift;				/* where to begin TCW search */
	uchar	tcw_word;			/* next tbl word to look at  */
	ushort	resvd1;				/* reserved		     */
	ushort	resvd2;				/* reserved		     */
	uchar	vpd_close;			/* indicates if close needed */
	uchar	locate_state;			/* state of the driver       */
#define	RESET_PENDING	1
#define	RESET_COMPLETE	2
#define	SCB_PENDING	3
#define	SCB_COMPLETE	4
	int	locate_event;			/* locate state event word   */
	int	rir_event;			/* read immediate event word */
	int	vpd_event;			/* get vpd event word        */
	int	eid_event;			/* entity id event word      */
	int	ebp_event;			/* establish buf pool event  */
	lock_t	eid_lock; 			/* lock wrd to serialize eid */
	int	(*recv_fn)();			/* proto initiator rcv func  */
	int	(*tm_recv_fn)();		/* proto target rcv function */
	struct	buf_pool_elem	*tm_buf_info;	/* ptr to buf_info structs   */
	struct	buf_pool_elem	*tm_head;	/* wait q of buf_info structs*/
	struct	buf_pool_elem	*tm_tail;	/* wait q of buf_info structs*/
	int	*tm_recv_buf;			/* recv buf passed to proto  */
	ulong	tm_bufs_tot;			/* tm bufs counter.	     */
	ulong	tm_bufs_at_adp;			/* tm bufs counter.	     */
	ulong   tm_bufs_to_enable;              /* num bufs to enable        */
	uchar	*tm_buf;			/* ptr to TM buffers 	     */
	uchar	*tm_raddr;			/* IO address of TM buffers  */
	uchar	*proto_tag_e;			/* tag for async status exter*/
	uchar	*proto_tag_i;			/* tag for async status inter*/
	ulong	adapter_check;			/* adp in fatal error state  */
#define	IDS		0x10
#define	LUNS		0x20
#define	BUSSES		0x2
#define	WORDSIZE	0x20
#define	EID_TABLE_SIZE	(( IDS * LUNS * BUSSES ) / WORDSIZE )
	uint	eid[EID_TABLE_SIZE];		/* entity id bit table 	     */
	uchar	dev_eid[0x400];			/* entity id table 	     */
	uchar	tm_dev_eid[0x400];		/* entity id table - TM	     */
	uint	pipe_full_cnt;			/* # of pipe full conditions */
	uint	dump_state;			/* state of dump driver	     */
	uchar	pad;				/* reserved		     */
	uchar	adp_cmd_pending;		/* move mode adp cmd pending */
	uchar	reset_pending;			/* SCSI bus reset pending    */
	uchar	epow_state;			/* early power off warning   */
	uchar	mm_reset_in_prog;		/* Move mode reset in progres*/
	uchar	sleep_pending;			/* e_sleep is needed flag    */
	uchar	bus_reset_in_prog;		/* SCSI bus rst in progress  */
	uchar	first_try;			/* reset flag		     */
	ushort	devs_in_use_I;			/* count of active devs - INT*/
	ushort	devs_in_use_E;			/* count of active devs - EXT*/
	ushort	num_buf_cmds;			/* # of establish buffer pool*/
						/* commands for tm add_filter*/
	ushort	next_id;			/* next free entity id       */
	ushort	next_id_tm;			/* next free entity id - TM  */
	ushort	resvd4;				/* reserved		     */
	uchar	ebp_flag;			/* estab buf pool flag	     */
	uchar	tm_bufs_blocked;		/* TM buffers waiting	     */
	uchar	tm_enable_threshold;		/* buffer enable threshold   */
	
#define	MAX_TM_BUFS	320
#define	TM_BUF_SIZE	0x30
#define	MAX_EST		0x20
#define	RIR		0x1
#define	EID		0x2
#define	ADP_INFO	0x3
#define	BUS_RESET	0x4
#define	EPOW_OFF	0x0
#define	EPOW_PENDING	0x1
};

/*
 *  Offsets of IO registers.
 */
#define	CIR0	0			/* Command Interface Register 0 */
#define	CIR1	1			/* Command Interface Register 1 */
#define	CIR2	2			/* Command Interface Register 2 */
#define	CIR3	3			/* Command Interface Register 3 */
#define	ATN	4			/* Attention Register 		*/
#define	BCR	5			/* Basic Control Register 	*/
#define	ISR	6			/* Interrupt Status Register 	*/
#define	BSR	7			/* Basic Status Register 	*/

/*
 *  Offsets to POS registers.
 */
#define	POS0	0
#define	POS1	1
#define	POS2	2
#define	POS3	3
#define	POS4	4
#define	POS5	5
#define	POS6	6
#define	POS7	7

/*
 *  Bit definitions of the POS registers.
 */
#define	ENABLE_ADAPTER	0x01

/*
 *  Miscellaneous defines.
 */
#define MAXREQUEST      0x40000         /* largest data xfer size       */
#define PIPESIZE	PAGESIZE	/* size of each delivery pipe   */
#define PIO_FAILURE	(-1)		/* PIO failure after 3 retries  */
#define ACTIVATE_RIR	(0)		/* activate read immediate cmd  */
#define DEACTIVATE_RIR	(1)		/* deactivate read immediate cmd*/
#define HW_FAILURE	(-1)		/* Hardware failure		*/
#define QTAG_MASK     	0x000F0000      /* QTAG field mask 	        */
#define ASC_SIMPLE_Q    0x00000000      /* simple q_msg mask value      */
#define ASC_ORDERED_Q   0x00020000      /* ordered q_msg mask value     */
#define ASC_HEAD_OF_Q   0x00010000      /* head of q_msg mask value     */
#define ASC_NO_Q        0x00030000      /* no q_msg mask value          */
#define ASC_NO_DISC     0x00000100      /* do not allow disconnect      */
#define ASC_UNTAG_CMD   0x00000004      /* do not send tagged cmd messg */
#define SUPRESS_SHORT   0x00000020      /* supress short exceptions     */
#define TAGMASK		0x00000001 	/* mask for bus is isrdata      */
#define DUMP_TIMEOUT	30		/* timeout for dump writes      */
#define ALL_ONES	0xFFFFFFFF	/* all ones for TCW calculations*/
#define	SHARED_MEMORY	0x4000		/* adapter configuration option */
#define	SYSTEM		0x0000		/* adapter configuration option */
#define	ADAPTER		0x0800		/* adapter configuration option */
#define	BUSY		0x01		/* adp busy processing a command*/
#define	CONTROL_SIZE	0x104		/* size of the signalling area  */
#define	ALLOC		0x1		/* allocation request   	*/
#define	FREE		0x2		/* free request   		*/
#define	FREE_ALL	0x3		/* free all entity ids		*/
#define	WRAP		0x803F0000	/* wrap opcode			*/
#define	WRAP_REV	0x3F80		/* wrap opcode - byte reversed	*/
#define	WRAP_SIZE	0x8		/* size of a wrap element	*/
#define PIPE_FULL	2		/* pipe full condition		*/
#define BDR_CODE	0x00002000	/* BDR requested		*/
#define RESET_CODE	0x0000C000	/* Bus reset requested.		*/
#define ABORT_CODE	0x00004000	/* Abort reset requested.	*/
#define MORE_BUFFERS	0x00008000	/* More TM buffers follow.	*/
#define BUS_MASK	0x00100000	/* mask for bus bit.		*/
#define	BYTES_PER_WORD	0x4		/* number of bytes in a word    */
#define	PD_SIZE		0x4		/* parameter descriptor size	*/
#define	BUS_SHIFT	0xE		/* location of bus field	*/
#define	ID_SHIFT	0x10		/* location of SCSI id field	*/
#define	IDLUN_WORD	0x2		/* word that contains id & lun  */
#define	STATUS_WORD	0x4		/* word that contains status	*/
#define	FLAGS_WORD	0x3		/* word that contains flags	*/
#define	EID_WORD	0x1		/* location of the element id   */
#define	OPCODE		0x3		/* location of opcode in elem 	*/
#define	CORRELATION	0x3		/* location of correlation fld  */
#define	SURR_SIZE	0x8		/* sz of each surrogate section */
#define	LEN_FIELD	0x0		/* location of length field	*/
#define	LUN_MASK	0x0000001F	/* location of lun field	*/
#define	ID_MASK		0x000001E0	/* location of id field		*/
#define	PAGE_MASK	0xFFFFF000	/* mask off page offset		*/
#define	PAGE_OFFSET	0x00000FFF	/* mask to get page offset 	*/
#define	L2_PAGE		0xC		/* log2 of page size		*/
#define	ATN_SIGNAL	0x90		/* exec a management request 	*/
#define	ABORT_CODE_MASK	0x0000E000	/* abort code field mask 	*/
#define	ABORT_NODEV_MSK		0x00080000	/* don't send msg to device*/
#define SCSI_LENGTH_MASK	0x0F000000	/* mask off len of request */
#define SCSI_DIRECTION_MASK	0x00001000	/* direction of xfer	   */
#define SCSI_REACTIVATE_MASK	0x00000400	/* reactivate q indication */
#define SEND_SCSI_LENGTH	0x34		/* len of Send SCSI elem   */
#define MGNT_LENGTH		0x58		/* len of mangnt rqst elem */
#define RIR_LENGTH		0x10		/* len of RIR request	   */
#define RELEASE_LENGTH		0x10		/* len of RELEASE request  */
#define REACT_LENGTH		0x10		/* len of REACTIVATE rqst  */
#define GETVPD_LENGTH		0x20		/* len of Exec Locate elem */
#define EID_LENGTH		0x1C		/* len of EID request	   */
#define ABORT_LENGTH		0x18		/* len of Abort request	   */
#define INIT_LENGTH		0x14		/* len of Init SCSI request*/
#define EST_LENGTH		0x3C		/* len of Est Buf pool rqst*/
#define	EMPTY_TO_NOTEMPTY	0x0008		/* adp cfg option	   */

/*
 * Defines used in the status field of the ctl_elem_blk.
 */
#define	ADAPTER_INITIATED	0x2		/* adapter initiated cmd   */
#define	CMD_TIMED_OUT		0x5		/* cmd timed out	   */
#define STA_USED        	0x8 		/* STA in use		   */
#define	TARGET_OP		0x10		/* target mode operation   */
#define ASC_INTERNAL    	0x400     	/* internal SCSI bus 	   */
#define ASC_EXTERNAL    	0x800     	/* external SCSI bus 	   */
#define ASC_BOTH_BUSSES 	0x100     	/* intern & extern SCSI busses*/
#define DUMP_ELEMENT		0x1000     	/* this element used for dump */

/*
 * SCSI Dump States
 */
#define	DUMP_IDLE	0x01
#define	DUMP_INIT	0x02
#define	DUMP_START	0x03

#ifdef	ASC_TRACE
#define TRACE_ENTRIES   32

struct trace_element {
	uint  type;
	uint  *ap;
	struct ctl_elem_blk ctl_elem;
};
#endif /* ASC_TRACE */

/*
 *  Status field values found in the Read Immediate
 *  event element.
 */
#define	RESET_INT		0x01000000	/* SCSI bus reset - internal */
#define	RESET_EXT		0x02000000	/* SCSI bus reset - external */
#define	DEV_RESELECT_INT	0x01010000	/* unexpected dev reselect-int*/
#define	DEV_RESELECT_EXT	0x02010000	/* unexpected dev reselect-ext*/
#define	TERM_POWER_LOSS_INT	0x01020000	/* term power loss - internal */
#define	TERM_POWER_LOSS_EXT	0x02020000	/* term power loss - external */
#define	DIFF_SENSE_INT		0x01030000	/* differential sense error-i */
#define	DIFF_SENSE_EXT		0x02030000	/* differential sense error-e */
#define STATUS_MASK		0x0FFF0000	/* read immediate status mask */

/*
 *  Reasons for asynchronous notification.
 */
#define	ASC_BUS_RESET		0x1	/* SCSI bus has been reset 	*/
#define	ASC_DEV_RESELECT	0x2	/* Unexpected device reselect 	*/
#define	ASC_TERM_POWER_LOSS	0x3	/* Power loss to terminator 	*/
#define	ASC_DIFF_SENSE		0x4	/* Differential sense error 	*/

/*
 *  Function codes of the Management Request control element.
 */
#define	MNGT_PIPE_CFG		0x80100000	/* configure pipes	*/
#define	MNGT_PIPE_UNCFG		0x80110000	/* unconfigure pipes	*/
#define	MNGT_ASSIGN_ID		0xA0000000	/* assign eid - initiat */
#define	MNGT_RELEASE_ID		0xA0010000	/* release eid - initiat*/
#define	MNGT_RELEASE_ALL_IDS	0xA0020000	/* release all eids 	*/
#define	MNGT_SUSPEND_ID		0xA0030000	/* suspend an eid	*/
#define	MNGT_RESUME_ID		0xA0040000	/* resume an eid	*/
#define	MNGT_QUERY_ID		0xA0100000	/* query an eid		*/
#define	INITIATOR		0x0		/* initiator request	*/
#define	TARGET			0x08000000	/* target request	*/

/*
 *  Status codes for reply elements
 */
#define	REPLY_SUCCESS		0x0000		/* Unqualified Success */
#define	REPLY_SUCCESS_Q		0x0100		/* Qualified Success   */

/*
 *  Bit values of the Basic Status Register(BSR).
 */
#define BSR_INTERRUPT		0x2	/* System Interrupt Request bit */
#define BSR_EXCEPTION		0x10	/* System Interrupt Request bit */
#define BSR_EXCP_CC		0x00	/* Synchronous Channel Check    */
#define BSR_EXCP_HW_FAIL	0x20	/* Hardware Failure		*/
#define BSR_EXCP_INV_CMD	0x40	/* Invalid cmd or parameter 	*/
#define BSR_EXCP_ILLOGICAL	0x60	/* Adapter in illogical state   */
#define BSR_EXCP_PIPE_CORRUPTED	0x80	/* Corrupted pipe		*/
#define BSR_EXCP_PIPE_ERROR	0xA0	/* Pipe	control error		*/
#define BSR_EXCP_MASK		0xE0	/* Pipe	control error		*/

/*
 *  BCR bit definitions.
 */
#define	BCR_RESET		0x80	/* Reset the adapter		*/
#define	BCR_CLR_ON_READ		0x40	/* Clear intr on BSR read	*/
#define	BCR_ENABLE_DMA		0x02	/* Enable DMA			*/
#define	BCR_ENABLE_INTR		0x01	/* Enable interrupts		*/

/*
 *  ISR bit definitions.
 */
#define	ISR_RESET_COMPLETE	0x0F	/* HW reset complete		*/
#define	ISR_SCB_COMPLETE	0x10	/* SCB cmd completed		*/
#define ISR_SCB_COMPLETE_RT	0x50	/* SCB_cmd completes w/retries  */
#define ISR_HDW_FAILURE		0x70	/* Hardware failure		*/
#define ISR_CMD_FAILURE		0xC0	/* Cmd completed with error	*/
#define ISR_CMD_ERROR		0xE0	/* Cmd error: invalid param or cmd */


/* 
 *  element IDs 
 */
#define REQUEST_EID     0X00000000	/* request element		*/
#define REPLY_EID       0X40000000	/* reply element		*/
#define EVENT_EID       0X80000000	/* event element		*/
#define ERROR_EID       0XC0000000	/* error element		*/

/*  
 *  byte-reversed eids and mask.
 */
#define	EID_MASK	0x000000FF	/* mask for incoming elements	*/
#define	OP_MASK		0x0000FF00	/* mask for incoming opcodes	*/
#define REQUEST_REV     0X0000		/* request element eid 		*/
#define REPLY_REV       0X0040		/* reply element		*/
#define EVENT_REV       0X0080		/* event element		*/
#define ERROR_REV       0X00C0		/* error element		*/

/* 
 *  option bit definitions. 
 */
#define SUPRESS		0X20000000	/* suppress short errors	*/
#define EXPEDITED    	0X00800000	/* expedite - head of queue	*/
#define REACTIVATE     	0X00000002	/* reactivate halted queue	*/
#define BDR             0X00000100	/* BDR 				*/
#define ABORT_NODEV     0X00004000	/* abort but don't send to dev  */
#define INT_BUS    	0X00000400	/* internal bus			*/
#define EXT_BUS    	0X00000800	/* external bus			*/

/* 
 *  chaining bits - used for CTQ 
 */
#define NOCHAIN         0X0000		/* simple queue			*/
#define STARTCHAIN      0X0800		/* ordered queue		*/
#define MIDDLECHAIN     0X1800		/* ordered queue		*/
#define ENDCHAIN        0X1000		/* ordered queue		*/

/* Function codes - opcodes for SCB request elements */
#define SEND_SCSI      	0x00410000	/* send scsi			*/
#define ABORT          	0x00400000	/* abort			*/
#define CANCEL_RIR	0x000C0000	/* cancel a read immediate	*/
#define MANAGEMENT	0x00100000	/* management 			*/
#define READ_IMMEDIATE	0x00060000	/* read immediate		*/
#define EXECUTE_LOCATE	0x00700000	/* execute a locate mode cmd	*/
#define INITIALIZE     	0x00520000	/* scsi bus resets		*/
#define REACT      	0x007E0000	/* reactivate			*/
#define PERFORM        	0x00700000	/* perform scsi			*/
#define ESTABLISH      	0x00490000	/* establish buffer pool	*/
#define RELEASE      	0x004C0000	/* release buffer pool		*/

/* Function codes - byte reversed */
#define SEND_SCSI_REV  	0x4100		/* send scsi			*/
#define ABORT_REV      	0x4000		/* abort 			*/
#define CANCEL_REV     	0x0C00		/* cancel			*/
#define MANAGEMENT_REV	0x1000		/* management			*/
#define INITIALIZE_REV 	0x5200		/* scsi bus reset		*/
#define PERFORM_REV    	0x7000		/* perform scsi			*/
#define SCSI_INFO_REV  	0x4700		/* scsi info event		*/
#define ESTABLISH_POOL_REV     	0x4900  /* establish buffer pool	*/
#define RELEASE_POOL_REV     	0x4C00  /* release buffer pool		*/
#define EXECUTE_LOCATE_REV     	0x7000	/* execute locate mode cmd	*/
#define READ_IMMEDIATE_REV	0x0600	/* read immediate		*/

/*
 *  The scb header structure is common to all SCB move mode elements
 *  NOTE: This structure is byte reversed to compensate for little
 *  endian chip.
 */
struct  scb_header {
        ulong	length;                 /* length of control element in bytes*/
        ulong	indicators;             /* common indicators                 */
        ushort  dst;                    /* destination field                 */
        ushort  src;                    /* source field                      */
        int     correlation;            /* correlation field                 */
};


/*
 *  Generic send scsi scb request element
 */
struct  send_scsi   {
	struct  scb_header	header;	/* generic SCB header 		*/
        ulong	options;		/* option controls 		*/
        ulong	auto_rs;		/* auto request sense dst addr 	*/
        ulong	auto_rs_len;		/* auto request sense buf length*/
        uchar	scsi_cmd[12];		/* SCSI cmd block		*/
        ulong	total_len;		/* total transfer length	*/
        ulong   buf_addr;		/* buffer address		*/
        ulong   buf_size;               /* buffer size			*/
};

/*
 *  Configure Delivery Pipes Management Request Element.
 *  The structure is laid out to make the Little Endian
 *  a bit more bearable.
 */
struct  cfg_mre {
	struct  scb_header	header;		/* generic SCB header */
	ushort	id;				/* id */
	ushort	function;			/* function */
	ushort	uids;				/* unit ids */
	ushort	cfg_status;			/* config status */
	ulong	pusa;				/* peer unit signalling addr */
	ulong	ausa;				/* adp unit signalling addr */
	ushort	aioa;				/* adapter io address */
	ushort	pioa;				/* peer io address */
	ulong	timer_ctrl;			/* timer control */
	ushort	adp_cfg_options;		/* adapter config options */
	ushort	sys_cfg_options;		/* system config options */
	ushort	eq_pipe_size;			/* outbound pipe size */
	ushort	dq_pipe_size;			/* inbound pipe size */
	ulong	dq_pipe_addr;			/* inbound pipe address */
	ulong	dq_sds_addr;			/* inbound surrogate DQ stat */
	ulong	dq_sse_addr;			/* inbound surr. SOE address */
	ulong	dq_ses_addr;			/* inbound surr. EQ stat addr */
	ulong	dq_ssf_addr;			/* inbound surr. SOF addr */
	ulong	eq_pipe_addr;			/* outbound pipe address */
	ulong	eq_sds_addr;			/* outbound surrogate DQ stat */
	ulong	eq_sse_addr;			/* outbound surr. SOE address */
	ulong	eq_ses_addr;			/* outbound sur. EQ stat addr */
	ulong	eq_ssf_addr;			/* outbound surr. SOF addr */
};

/*
 *  PIO macros
 */

/*
 *  Attention Register
 */
#define ATTENTION(location, value)					\
{									\
	if(BUS_PUTCX((char *)(location) + ioaddr + ATN, value) != 0) {	\
		if(asc_retry_put(ap,location + ioaddr + ATN, 1,value) != 0) \
			pio_error = TRUE;				\
		else							\
			pio_error = FALSE;				\
	}								\
};

/*
 *  Basic Control Register read/write macros
 */
#define WRITE_BCR(location, value)                                      \
{                                                                       \
	if(BUS_PUTCX((char *)(location) + ioaddr + BCR, value) != 0 ) {	\
		if(asc_retry_put(ap,location + ioaddr + BCR, 1,value) != 0) \
			pio_error = TRUE;				\
		else							\
			pio_error = FALSE;				\
	}								\
};


/*
 *  Interrupt Status Register
 */
#define READ_ISR(location, value)                                      \
{                                                                       \
	if(BUS_GETCX((char *)(location) + ioaddr + ISR, value) != 0 ) { \
		if(asc_retry_get(ap,location + ioaddr + ISR, 1,value) != 0) \
			pio_error = TRUE;				\
		else							\
			pio_error = FALSE;				\
	}								\
};


/*
 *  Basic Status Register
 */
#define READ_BSR(location, value)                                          \
{                                                                          \
	if(BUS_GETCX((char *)(location) + ioaddr + BSR, value) != 0) {   \
		if(asc_retry_get(ap,location + ioaddr + BSR, 1,value) != 0) \
			pio_error = TRUE;				\
		else							\
			pio_error = FALSE;				\
	}								\
};

/*
 *  Command Interface Registers macro.
 */
#define WRITE_CIR(location, value)                                      \
{                                                                       \
	if (BUS_PUTCX((char *)(location) + ioaddr, value) != 0) {     	\
		if(asc_retry_put(ap,location + ioaddr, 1,value) != 0) 	\
			pio_error = TRUE;				\
		else							\
			pio_error = FALSE;				\
	}								\
};


/*
 *  Macro to extract entity ID from table given
 *   bits 14 - 15 contain the bus
 *   bits 5 - 8 contain the SCSI id
 *   bits 0 - 4 contain the LUN
 *   equation: ((bus * 512) + (SCSI id * 32) + LUN )
 */
#define CALC_ID( punlunbus )	                                         \
	 (((punlunbus >> 14) * 512) + (((punlunbus >> 5) & 0xF) * 0x20) + (punlunbus & 0x1F));
/* 
 *  Acquire an unused entity id.
 *  Valid entity ids range from 1 - 254(0xFE).
 */
#define GET_ID( ap )	                                         	\
{                                                                       \
 if(ap->eid[ap->next_id / WORDSIZE] & (0x80000000 >> (ap->next_id % WORDSIZE)))\
    {									\
        for( j = 1; j < 0xFF; j++ )					\
        {								\
	    if(ap->eid[j / WORDSIZE] & (0x80000000 >> (j % WORDSIZE)))  \
	        continue;						\
	    else {							\
	        ap->eid[(j / WORDSIZE)] |= (0x80000000 >> (j % WORDSIZE)); \
	        eid_val = j;						\
	        break;							\
	    }								\
        }								\
	if( j == 0xFF )						\
	    eid_val = 0;						\
    }									\
    else {								\
	ap->eid[(ap->next_id / 32)] |= (0x80000000 >> (ap->next_id % 32)); \
        eid_val = ap->next_id;						\
    }									\
    if( ap->next_id == (0xFF - 1))					\
	ap->next_id = 1;						\
    else								\
	ap->next_id++;							\
};

/*
 *  Given an EID, the following macros set or clear the corresponding
 *  bit in the EID table.
 */
#define SET_ENTITY_ID( ap )                                            \
        ap->eid[eid_val / WORDSIZE] |= ( 0x80000000 >> (eid_val % WORDSIZE) );

#define CLEAR_ENTITY_ID( ap )                                         \
        ap->eid[eid_val / WORDSIZE] &= ~(0x80000000 >> (eid_val % WORDSIZE));

/*
 *	Hash on the device's sequence number to 
 *	find the proper adapter structure.
 *
 *	ap    	   -  adapter structure
 *      seq_number -  device number
 */
#define HASH( ap, sequence )                          			\
{                                                                       \
	ap = adp_ctrl.ap_ptr[sequence];					\
	while( ap != NULL ) {						\
		if ( ap->seq_number == sequence )			\
			break;						\
        	ap = ap->next;						\
	}								\
};

/*
 *  Macro to load the POS registers. Since POS accesses do NOT
 *  generate exceptions, a write/read/compare sequence is
 *  necessary to verify successful completion.
 */
#define WRITE_POS( addr, val )					\
{								\
	BUS_PUTC( addr, val);					\
	BUS_GETC( addr );					\
	if( (int)x != (int)val )  				\
		rc = EIO;					\
	else							\
		rc = 0;						\
};

/*
 *  Macros to calculate offsets for surrogate areas.
 */
#define OFFSET( base, location )                                         \
        (uint) ( (location) - (base) )

#define ADD_BASE( base, location )                                         \
        ( base + location );

#define REV_SHORT( addr )                                         	\
        (ushort)(( (ushort)addr >> 8 ) | ( (ushort)addr << 8 ))

#define REV_LONG( addr )                                         	\
        ( ((ulong)addr >> 24) | (((ulong)addr >> 8 ) & 0x0000FF00) |	\
	  (((ulong)addr << 8 ) & 0x00FF0000) | ( (ulong)addr << 24 ) )

#define REV_LONG_SHORT( addr )                                         	\
        ( (((ulong)addr << 8 ) & 0xFF00FF00) |	\
	  (((ulong)addr >> 8 ) & 0x00FF00FF) )


/*
 *  Macros to convert real(IO) address to the
 *  corresponding system memory equivalent and
 *  vis-a-vis. These are necessary because the surrogate
 *  areas are in system memory and the adapter accesses these
 *  areas with different addresses than the system does.
 *  NOTE: addr is a 16-bit offset in R_TO_V and a 32-bit 
 *  virtual address in V_TO_R.
 */
#define	R_TO_V(io_base, v_base, addr )				\
	((((io_base + addr) - io_base) + v_base ));

#define	V_TO_R(io_base, v_base, addr )				\
	((addr - v_base ) + io_base )

/*
 *  Error logging template.
 */
struct error_log_def {                  /* driver error log structure   */
	struct	err_rec0 errhead;       /* error log header info        */
	struct	rc	 data;          /* driver dependent err data    */
};

/* 
 *  Component dump table struct.
 */
struct asc_cdt_tab {
        struct  cdt_head   asc_cdt_head;
        struct  cdt_entry  asc_entry[MAX_ADAPTERS*2];
};


#ifndef _NO_PROTO
/*****************************************************************************/
/*     internal functions                                                    */
/*****************************************************************************/

int	asc_config(int op, struct uio *uiop);
int	asc_open(ndd_t *ndd);
int     asc_close(ndd_t *ndd);
int     asc_output(ndd_t   *ndd, struct ctl_elem_blk *ctrl_elem );
int     asc_ioctl(ndd_t *ndd, int cmd, int arg);
void    asc_stub();
int	asc_add_filter();
int	asc_del_filter();
int 	asc_add_status();
int 	asc_del_status();
int	asc_intr(struct intr * intr);
int	asc_epow(struct intr * intr);
void	asc_wdt_intr();
void	asc_cleanup(struct adapter_info *ap, ulong cleanup_depth);
int 	asc_get_adp_info(struct adapter_info *ap, struct ndd_config *cfg);
int	asc_init_adapter(struct adapter_info  *ap);
int	asc_retry_get(struct adapter_info *ap, ulong ioaddr, ulong size, 
								void *value);
int	asc_retry_put(struct adapter_info *ap,ulong ioaddr,ulong size,
								ulong value);
int	asc_init_pipes(struct adapter_info *ap, ulong ioaddr);
int	asc_rir(struct adapter_info *ap, int cmd);
int	asc_enable_bufs(struct buf_pool_elem *bp_elem);
int	asc_create_bufs(struct adapter_info *ap, int num_bufs);
int	asc_disable_bufs(struct adapter_info *ap);
int	asc_entity_id( struct  adapter_info *ap, int cmd, int eid );
int	asc_init_pos(struct  adapter_info *ap);
void	asc_clear_pos(struct  adapter_info *ap );
int	asc_fatal_error( struct adapter_info *ap );
void	asc_dequeue( struct  adapter_info    *ap );
int	asc_get_start_free(struct adapter_info *ap,int size,uint **free_ptr);
void	asc_start( struct  adapter_info *ap );
int	asc_translate( struct adapter_info *ap, struct ctl_elem_blk *ctl_blk );
void	asc_log_error( struct adapter_info * ap, int errid, uchar *reply, 
					int ahs, int errnum, int data1);
int	asc_clear_pipes( struct adapter_info *ap );
int	asc_update_eq( struct adapter_info *ap, uchar *free_ptr, int size);
int	asc_abort( struct adapter_info *ap, struct ctl_elem_blk *ctl_elem );
void	asc_process_elem( struct adapter_info *ap, ulong *reply);
void	asc_process_adp_elem( struct adapter_info *ap, 
				struct ctl_elem_blk *ctl_elem );
void	asc_rtov(struct adapter_info *ap, struct ctl_elem_blk *ctl_elem );
int	asc_get_adp_resources(struct adapter_info *ap,uint **free_ptr,int size);
int     asc_dump (ndd_t *p_ndd, int cmd, caddr_t arg);
void	asc_clear_queues(struct adapter_info *ap,uchar bus );
void	asc_clear_q_adp(struct adapter_info *ap);
void	asc_reassign_eids(struct adapter_info *ap);
void	asc_sendlist_dq(struct adapter_info *ap,struct ctl_elem_blk *ctl_blk);
void	asc_async_status(struct adapter_info *ap,int op,int bus );
int	asc_reset_adapter(struct adapter_info *ap);
void	asc_process_locate(struct adapter_info  *ap,ulong ioaddr);
int	asc_vtor(struct adapter_info *ap,struct ctl_elem_blk *ctl_elem);
int	asc_alloc_tcws(struct adapter_info *ap,struct ctl_elem_blk *ctl_elem);
int	asc_activate_cmd(struct adapter_info *ap,uint *free_ptr,int size,int i);
int	asc_get_ctl_blk(struct adapter_info *ap,int cmd,int index);
int	asc_parse_reply(struct adapter_info *ap,struct ctl_elem_blk *ctl_blk);
int	asc_rir_int(struct adapter_info *ap);
int	asc_init_pipes_int(struct adapter_info *ap,ulong ioaddr);
void	asc_sleep(struct adapter_info *ap,ulong sleep_event);
void    asc_tm_rtov(struct adapter_info	*ap,ulong *reply);
struct  cdt * asc_cdt_func( int    arg);
#ifdef	ASC_TRACE
void	asc_trace(struct adapter_info *ap,struct ctl_elem_blk *cmd, int option);
#endif  /* ASC_TRACE */

void    asc_inval(uchar *eaddr, uint nbytes);
void    qrev_writel(uint src, uint *dest);
void    qrev_writes(ushort src, ushort *dest);
uint    asc_cntlz(uint word);
int     d_align();
int     d_cflush (int channel_id, caddr_t baddr, size_t count, caddr_t daddr);
int     copyout (char *kaddr, char *uaddr, int count);
int     copyin (char *uaddr, char *kaddr, int count);
void    errsave (char *buf, unsigned int cnt);
int     dmp_add (struct cdt *((*cdt_func)()));
int     dmp_del (struct cdt *((*cdt_func)()));
void    curtime (struct timestruc_t *timestruct);
void    vm_cflush (caddr_t eaddr, int nbytes);

#ifdef DEBUG
#include <stdio.h>
void panic (char *s);
#endif /* DEBUG */



#else
/*****************************************************************************/
/*     internal functions                                                    */
/*****************************************************************************/

int     asc_config();
int     asc_open();
int     asc_close();
int     asc_output();
int     asc_ioctl();
void    asc_stub();
int	asc_add_filter();
int	asc_del_filter();
int 	asc_add_status();
int 	asc_del_status();
void	asc_wdt_intr();
int	asc_intr();
int	asc_epow();
void	asc_cleanup();
int 	asc_get_adp_info();
int	asc_init_adapter();
int	asc_retry_get();
int	asc_retry_put();
int	asc_init_pipes();
int	asc_rir();
int	asc_enable_bufs();
int	asc_create_bufs();
int	asc_disable_bufs();
int	asc_entity_id();
int	asc_init_pos();
void	asc_clear_pos();
void	asc_dequeue();
int	asc_get_start_free();
void	asc_start();
int	asc_translate();
void	asc_log_error();
int	asc_clear_pipes();
int	asc_update_eq();
int	asc_abort();
void 	asc_process_elem();
void	asc_process_adp_elem();
void	asc_rtov( );
int	asc_get_adp_resources();
int	asc_dump_write();
void	asc_clear_queues();
void	asc_clear_q_adp();
void	asc_reassign_eids();
void	asc_sendlist_dq();
void	asc_async_status();
int	asc_reset_adapter();
void	asc_process_locate();
int	asc_vtor();
int	asc_alloc_tcws();
int	asc_activate_cmd();
int	asc_get_ctl_blk();
int	asc_parse_reply();
int	asc_rir_int();
int	asc_init_pipes_int();
void	asc_sleep();
void    asc_tm_rtov();
struct  cdt * asc_cdt_func();
#ifdef	ASC_TRACE
void	asc_trace();
#endif  /* ASC_TRACE */

#endif
#endif
