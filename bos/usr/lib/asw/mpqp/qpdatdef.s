
	PAGE	60,124
;
;  COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
;
;  FUNCTIONS: None
;
;  ORIGINS: 27
;
;  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
;  combined with the aggregated modules for this product)
;                   SOURCE MATERIALS
;  (C) COPYRIGHT International Business Machines Corp. 1988, 1989
;  All Rights Reserved
;
;  US Government Users Restricted Rights - Use, duplication or
;  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
;
;  FUNCTION: Adapter software data definitions.  No data is allocated
;	     in separate "C" or assembler modules.  This is for two
;	     reasons: first, the "C" compiler does not unserstand the
;	     way the ASW uses segment registers and does not typically
;	     generate addressable memory.  Second, the code region is
;	     protected from write accesses with translate table prot-
;	     ection, so local data from assembler causes protection
;	     violation NMIs.
;
;**********************************************************************
;
	TITLE	qpdatdef
	SUBTTL	IBM Confidential
_TEXT	segment	word public 'code'
_TEXT	ends
dgroup	group	_TEXT
	assume	cs:dgroup, ds:dgroup, es:dgroup, ss:dgroup
_TEXT	segment
	.286C
	.LFCOND
sccsid	db	'@(#)62	1.14  src/bos/usr/lib/asw/mpqp/qpdatdef.s, ucodmpqp, bos411, 9428A410j 5/17/94 08:59:49'
	db	0
	.XLIST
	include	define.inc
	include	cio.inc
	include	duscc.inc
	include	portdma.inc
	.LIST
	PAGE
;
; Do the main storage allocation for things in memory.  Notice!  This
; file is the only one that allocates storage.  All others use external
; references and, therefore, depend on this file (in a Make sort of way).
;
;****************************************************************************
; The scheduler works within an array, ordered in descending priority.
; The entries are redefined so that the assembler routines can reference
; individual fields without messing with tables.  Yecch.
;
__HLT__	db	235Fh DUP(0F4h)

BASE	=	$
	 public	_work_q
_work_q dw	NUM_LEV+1 DUP(0)

	org	_work_q		; redefine storage

	public	_rxdma,_txdma,_rxwork,_txwork,_errsts,_prqwork,_pcqwork
_rxdma dw	?
_rxwork	dw	?
_txdma dw	?
_txwork	dw	?
_errsts	dw	?
_prqwork dw	?
_pcqwork dw	?
	dw	?		; Placeholder for Sleep Wakeup indicators

;****************************************************************************
; The off-level work processor vectors through the "lev_vec" function
; vector table to dispatch work at a given service level.
; The vector points are in the TEXT segment and must either be externed
; outside any segment or within the TEXT segment.
;
	extrn	_recv_dma_work:NEAR,_xmit_dma_work:NEAR
	extrn	_f_rxwork:NEAR,_f_txwork:NEAR,_f_errsts:NEAR
	extrn	_f_prqwork:NEAR,_f_pcqwork:NEAR

	 public	_lev_vec
_lev_vec dw	NUM_LEV+1 DUP(?)

	org	_lev_vec		; redefine storage

_v_rdm	dw	_recv_dma_work		; RECV Bus Master DMA processing
_v_rxw	dw	_f_rxwork		; RX work processor function
_v_xdm	dw	_xmit_dma_work		; XMIT Bus Master DMA processing
_v_txw	dw	_f_txwork		; TX work processor function
_v_ers	dw	_f_errsts		; Error/Status processor function
_v_prq	dw	_f_prqwork		; Port response queue work pending
_v_pcq	dw	_f_pcqwork		; Port command queue work pending
_v_wake	dw	0			; Sleep Wakeups pass 0 to GateUser

	public	_task_mask
_task_mask dw	NUM_LEV+1 DUP(1)	; Round-Robin scheduler level masks

	public	_i_sched		; Currently operating scheduler level
_i_sched   dw   0			; Varies between 0 and NUM_LEV-1

;  The fundamental link between the adapter and driver is the organization
;  and size of the following data structures.  The CIS defines the exact
;  order and spacing of these items which cannot be changed without
;  potential impact on all MPQP drivers.

	 public	_rx_free
_rx_free EQU	$			; Receive Free Buffer List (byte)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	db	QLGT+4 DUP(0)
;
; If not on 64 byte boundary, get to one
;
; NOTE: Padding moved into the ORG statement at line 24 in this file
;
	 public	_tx_free
_tx_free EQU	$			; Transmit Free Buffer List (byte)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	db	QLGT+4 DUP(0)
	 public	_acqueue
_acqueue EQU	$			; Adapter Command Queue (byte)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	db	QLGT+4 DUP(0)
	 public	_arqueue
_arqueue EQU	$			; Adapter Response Queue (word)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	dd	Q2LGT DUP(0)
	public	_acmdreg
_acmdreg dw	0			; Adapter Command Register

;  More common data fields which define the sizes and locations of
; shared resources.
;
	public	_num_cmd
	public	_rx_nbuf,_rx_bsiz,_rx_para
	public	_tx_nbuf,_tx_bsiz,_tx_para

_num_cmd dw	NUM_CMD			; Number of command blocks
_rx_nbuf dw	RXNBUF			; Number of Rx Buffers (ignored)
_rx_bsiz dw	RXBSIZE			; Size in bytes, each Rx Buffer
_rx_para dw	RXPARA			; Rx Buffer Base Paragraph Address
_tx_nbuf dw	TXNBUF			; Number of Tx Buffers
_tx_bsiz dw	TXBSIZE			; Size in bytes, each Tx Buffer
_tx_para dw	TXPARA			; Tx Buffer Base Paragraph Address
	
;
;****************************************************************************
; Data structures in this region must retain their order, as they are viewed
; directly by the driver through PIO to adapter shared memory.
;****************************************************************************
;
; If not on 64 byte boundary, get to one
;
;	IF	($-BASE) MOD 64
;	ORG	$+(64-(($-BASE) MOD 64))
;	ENDIF
	db	60 DUP(0)
;
	public	_edrr
_edrr	db	64*NUM_PORT DUP(0)	; External entry for "C" lang. array
;
	ORG	_edrr			; Redefine the individual EDRRs
	public	_edrr_0
_edrr_0 db	64 DUP(?)		; Port 0 (& adapter cmd) EDRR
	public	_edrr_1
_edrr_1 db	64 DUP(?)		; Port 1 EDRR
	public	_edrr_2
_edrr_2 db	64 DUP(?)		; Port 2 EDRR
	public	_edrr_3
_edrr_3 db	64 DUP(?)		; Port 3 EDRR
;
; Trace tables and associated regions
;
	public	_trc_ad
_trc_ad	db	256 DUP(0)		; Adapter Synchronization Trace Data
	public	_trc_p
_trc_p	db	256*NUM_PORT DUP(0)
	ORG	_trc_p			; Redefine storage
	public	_trc_0
_trc_0	db	256 DUP(?)		; Port 0 Trace Data
	public	_trc_1
_trc_1	db	256 DUP(?)		; Port 1 Trace Data
	public	_trc_2
_trc_2	db	256 DUP(?)		; Port 2 Trace Data
	public	_trc_3
_trc_3	db	256 DUP(?)		; Port 3 Trace Data
;
	public	_cmdblk
_cmdblk	db	NUM_CMD*64 DUP(0)

;
; This area contains the Port Command Queues, Port Response Queues,
; Tx DMA Work Queue and global data used between interrupt and offlevel
; Bus master DMA service routines.
;
	public	_port_cq
_port_cq db	(QHDR+QLGT+4)*NUM_PORT DUP(?)
	ORG	_port_cq
	public	_p0cmdq,_p1cmdq,_p2cmdq,_p3cmdq
_p0cmdq	EQU	$			; Port 0 Command Queue (byte)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	db	QLGT+4 DUP(0)
_p1cmdq	EQU	$			; Port 1 Command Queue (byte)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	db	QLGT+4 DUP(0)
_p2cmdq	EQU	$			; Port 2 Command Queue (byte)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	db	QLGT+4 DUP(0)
_p3cmdq	EQU	$			; Port 3 Command Queue (byte)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	db	QLGT+4 DUP(0)

	public	_port_rq
_port_rq db	(QHDR+QLGT)*NUM_PORT DUP(?)
	ORG	_port_rq
	public	_p0rspq,_p1rspq,_p2rspq,_p3rspq
_p0rspq	EQU	$			; Port 0 Response Queue (rqe)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	dd	QLGT DUP(0)
_p1rspq	EQU	$			; Port 1 Response Queue (rqe)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	dd	QLGT DUP(0)
_p2rspq	EQU	$			; Port 2 Response Queue (rqe)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	dd	QLGT DUP(0)
_p3rspq	EQU	$			; Port 3 Response Queue (rqe)
	db	QHDR DUP(0)		; Header fields, Lgt, End, Outp, Inp
	dd	QLGT DUP(0)

	public	_tx_dma_q
_tx_dma_q EQU	$			; Transmit DMA Work Queue (word)
	db	0,0,0,0			; Header fields, Lgt, End, Outp, Inp
	dw	QLGT+4 DUP(?)

	even
	public	_Dma_Oper,_Dma_Type,_Dma_Count,_Dma_Retries,_Dma_BM_Index
	public	_Dma_Port,_dma_err,_dma_tmp,_Recv_Enable
_Dma_Oper      dd  0			; Bus Master Operation In Progress
					; Must contain an entire RQE on Rx
_Dma_Count     dw  0
_Dma_Retries   dw  0			; Number of retries to start DMA
_Dma_Type      db  0			; Bus Master Operation Type
_Dma_BM_Index  db  0			; Index for Bus Master DMA addresses
_Dma_Port      dw  0			; Used for BM DMA fairness.
_Recv_Enable   dw  0			; Enable/Disable Frame reception.

	  even
_dma_err  dw	0			; Global for restarting Bus Master
					; operations that abort/NMI
_dma_tmp  dw	0			;

	even
	public	_pcq_tab
_pcq_tab dw	_p0cmdq,_p1cmdq,_p2cmdq,_p3cmdq
	public	_prq_tab
_prq_tab dw	_p0rspq,_p1rspq,_p2rspq,_p3rspq

	public	_estat
_estat	db	NUM_PORT*8 DUP(?)		; 4 ports, 8 bytes each
	public	_estat0,_estat1,_estat2,_estat3
	org	_estat
_estat0	db	8 DUP(0)
_estat1	db	8 DUP(0)
_estat2	db	8 DUP(0)
_estat3	db	8 DUP(0)

;
; These next variables are used to implement the port context switching
; required to have sleeping retain the calling stack.  Only four registers
; must be saved to restore a context.  When a port stack is being used,
; these fields contain the context of the scheduler stack environment
; which existed when the port function was dispatched by the scheduler.
; Registers SP, SI, DI and BP are saved in that order.
	even
	public	_StackPtr
_StackPtr dw	NUM_PORT*4 DUP(?)
	org	_StackPtr
					; Sched  : 0AC02 - 0FFFE (5400)
Stk0	dw	0AC00h,0,0,0AC00h	; Port 0 : 08202 - 0AC00 (2A00)
Stk1	dw	08200h,0,0,08200h	; Port 1 : 05802 - 08200 (2A00)
Stk2	dw	05800h,0,0,05800h	; Port 2 : 02E02 - 05800 (2A00)
Stk3	dw	02E00h,0,0,02E00h	; Port 3 : 00402 - 02E00 (2A00)
					;  IVT   : 00000 - 00400 ( 400)

	public	_SleepEn
_SleepEn db	NUM_PORT DUP(0)		; Sleeping disabled
	public	_ReRoute
_ReRoute db	NUM_PORT DUP(0)		; RQE Rerouting disabled
	public	_TraceOn
_TraceOn db	NUM_PORT DUP(0)		; Tracing disabled

; This queue holds the data from the driver Receive Buffer Indicate commands.
; Used only in intzero.c

	public	_rx_dma_q
_rx_dma_q equ	$
	db	17,16,0,0		;				051091
	dd	17 DUP(0)

;
; This allocates the actual storage for the port control blocks.  The size
; will certainly have to be changed periodically.  Note that enough space
; for the blocks is allocated, only the "C" code knows where in this area
; a given PCB starts.
;
	public	_pcb
_pcb	db	NUM_PORT*256 DUP(0)

; When Echo interrupts indicating the end of a channel operation, two bytes
; of status are saved for the offlevels to look at.  Both _txstat and _rxstat
; fields contain the Echo channel DMA status in the high byte and the DUSCC
; Tx/Rx Status register in the low byte.
;
;  ?xstat [15-8] = Echo Status     ?xstat [7-0] = SCC Tx/Rx Status

	public	_rxstat,_txstat
_rxstat	dw	NUM_PORT DUP(0)
_txstat	dw	NUM_PORT DUP(0)

; If the adapter is essentially Full Duplex, receive channels must not be
; explicitly enabled after and disabled before transmitting.  The RxAuto
; flags, one per port, tell the ISRs how the port is operating, FDX | HDX.
; The next Rx Channel will be started automatically regardless of the
; state of the transmitter if RxAuto is non-zero.

	public	_RxAuto
_RxAuto	db	NUM_PORT DUP(0)

;
; When a BiSync frame comes along which requires transparency but the frame
; has the STX buried inside, a special sequence of events occurs so that the
; DUSCC can be given the TDLE command with the STX when it finally occurrs.
; The Echo channel is stopped and causes an interrupt, at which time the ISR
; sends the DUSCC command and restarts the channel with List Chaining (to the
; address already in the I/O resident CDT).

	even
	public	_txlgt,_txflgt
_txlgt	dw	NUM_PORT DUP(0)
_txflgt	dw	NUM_PORT DUP(0)

; BiSync requires that SYN characters be provided no further apart than
; one second on transmit, three seconds on receive.  To facilitate efficient
; and relatively automatic SYN insertion, the maximum number of characters
; that can be sent without reSYNcing is determined, by line speed, and
; stored in the _txinc port specific global variables.

	public	_txinc
_txinc	dw	NUM_PORT DUP(0FFFFh)

; Echo destroys the channel control word when a transmit channel reaches
; terminal count even if list chaining is not specified.  This makes it
; impossible for the Tx ISR to continue a Transparent Bisync frame with
; imbedded STX without knowing the original CCW value.

	public	_txccw
_txccw	dw	NUM_PORT DUP(0)

; When the SCC ISR receives the TSOM Ack interrupt, it starts the Port DMA
; channel which contains the frame data.  The txioa field contains that
; DMA channel I/O base address, which is started.

	public	_txioa
_txioa	dw	NUM_PORT DUP(0)

; In certain circumstances, assembler needs to know the data protocol selected
; for the port.  Most notably, transmitting transparent frames requires many
; additional steps compared with non-transparent data.  This field is prone to
; changing on a frame-to-frame basis.

	even
	public	_txftype
_txftype db	NUM_PORT DUP(0)

; Before a frame is transmitted, at least one command to the DUSCC is
; required.  If a BiSync frame is going out Transparent and starts with
; the STX, another command is also sent (TDLE).  Since the command(s)
; are decided in the transmit frame preprocessor, which may run one frame
; ahead of the wire, global storage is required to save the "next" frame's
; DUSCC commands until the frame is started.  That is what txcmd does.

	public	_txcmd
_txcmd	db	NUM_PORT DUP(0)

	public	_trsMask,_rsMask,_e_rxMask,_e_txMask
_trsMask db	NUM_PORT DUP(0)
_rsMask  db	NUM_PORT DUP(0)
_e_rxMask  db	NUM_PORT DUP(0)
_e_txMask  db	NUM_PORT DUP(0)

	even
	public	_tscc_val,_tscc_typ,_tscc_sval,_tscc_styp
_tscc_val  dw	NUM_PORT DUP(0)		; Time interval, tenths of a second
_tscc_typ  dw	NUM_PORT DUP(0)		; Timer type, Rx, Tx Failsafe?
_tscc_sval  dw	NUM_PORT DUP(0)		; Save Time interval, tenths of a second
_tscc_styp  dw	NUM_PORT DUP(0)		; Save Timer type, Rx, Tx Failsafe?

	even
	public	_tx_int_en
_tx_int_en dw	0

; This table contains the Port DMA Channel base I/O addresses in order, from
; zero to fifteen.  Next, the SCC bases and CIO bases.

	even
	public	_PDMAbase
_PDMAbase dw	PD0BASE,PD1BASE,PD2BASE,PD3BASE
	  dw	PD4BASE,PD5BASE,PD6BASE,PD7BASE
	  dw	PD8BASE,PD9BASE,PDABASE,PDBBASE
	  dw	PDCBASE,PDDBASE,PDEBASE,PDFBASE

	public	_SCCbase
_SCCbase  dw	SCC_0A,SCC_0B,SCC_1A,SCC_1B

	public	_CIObase
_CIObase  dw	CIO_0,CIO_0,CIO_1,CIO_1

; The storage with which the transmit data C services communicate with the
; assembly interface to Echo

	even
	public	_pdma_cdt
_pdma_cdt dw	8 DUP(0)

;
; The f_pwork (port work) scheduler level dispatches to one of 16 possible
; command service functions listed in the function table below.  Unused
; entries must, at least, return because no check is performed before
; vectoring.
	even
	extrn	_startp:NEAR,_set_parm:NEAR
	extrn	_stop_p:NEAR,_flush_p:NEAR,not_used:NEAR
	extrn	_start_ar:NEAR,_stop_ar:NEAR,_chg_parm:NEAR
	extrn	_get_modem_stats:NEAR
	public	_f_portwork
_f_portwork equ	$
	dw	not_used,_set_parm,_startp,_stop_p
	dw	not_used,_flush_p,not_used,not_used
	dw	not_used,not_used,_get_modem_stats,_start_ar
	dw	_stop_ar,_chg_parm,not_used,not_used
;
; These are the diagnostic command vector tables, which C/2 toasts and,
; thus, must be put here.
;
	even

; Command Types 0xF0 to 0xFF

	extrn	_ramtst:NEAR,_chktst:NEAR,_cputst:NEAR,_ciotst:NEAR
	extrn	_scctst:NEAR,_ssttst:NEAR,_adreset:NEAR

	public	_ad_cntl
_ad_cntl dw	_ramtst, _chktst, _cputst,       0
	dw	_ciotst, _scctst, _ssttst,       0
	dw	      0,       0,       0,       0
	dw	      0,       0,       0, _adreset

; Command Types 0xE0 to 0xEF

	extrn	_memtest:NEAR,_dmatest:NEAR

	public	_ad_diag
_ad_diag dw	_memtest,       0,       0,       0
	dw	       0,       0,       0,       0
	dw	       0,       0,       0,       0
	dw	_dmatest,       0,       0,       0

; Command Types 0xD0 to 0xDF

	extrn	_gmsize:NEAR,_getiid:NEAR,_geteid:NEAR,_setcio:NEAR
	extrn	_setscc:NEAR,_setdma:NEAR,_settmr:NEAR,_intwdt:NEAR
	extrn	_priswc:NEAR,_getvpd:NEAR,_io_read:NEAR,_io_write:NEAR

	public	_ad_sest
_ad_sest dw	_gmsize, _getiid, _geteid, _setcio
	dw	_setscc, _setdma, _settmr, _intwdt
	dw	_priswc, _getvpd,       0,       0
	dw	      0,       0, _io_read, _io_write

; Command Types 0xC0 to 0xCF

	extrn	_FE_sccregs:NEAR,_FE_cioregs:NEAR,_FE_dmaregs:NEAR
	public	_pt_diag

_pt_diag dw	_FE_sccregs,_FE_cioregs,_FE_dmaregs,0
	dw	0,0,0,0
	dw	0,0,0,0
	dw	0,0,0,0

; Command Types 0xB0 to 0xBF

	extrn	_FE_cpuregs:NEAR,_FE_cputmr:NEAR,_FE_cpudma:NEAR
	extrn	_FE_segreg:NEAR
	extrn	_start_trace:NEAR,_stop_trace:NEAR
	extrn	_data_blaster:NEAR,_set_scc_diag:NEAR,_c_serial_setup:NEAR
	public	_ad_qury

_ad_qury dw	_FE_cpuregs,_FE_cputmr,_FE_cpudma,0
	dw	0,0,0,0
	dw	_FE_segreg,0,0,_data_blaster
	dw	_set_scc_diag,_c_serial_setup,_start_trace,_stop_trace

; Command Types 0xA0 to 0xAF
	public	_us_diag
_us_diag dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

	public	_fT
_fT	dw	_ad_cntl,_ad_diag,_ad_sest,_pt_diag,_ad_qury,_us_diag
	dw	0,0,0,0,0,0,0,0,0,0

	even
	public	_Bm_Dma,_a_blc_list,__llc_list
_Bm_Dma     dw	10 DUP(0)	; Most Operational Bus Master DMA uses this
_a_blc_list dw	7*6 DUP(0)	; Buffer list chaining link blocks
__llc_list  dw	8*9 DUP(0)	; Gather and test commands use LLC

	even
	public	_p_io_tab
_p_io_tab dw	p0io,p1io,p2io,p3io

p0io	dw	PD0BASE,PD8BASE,PD1BASE,PD9BASE
	dw	SCC_0A,SCC_0A+ICTSTAT
	dw	CIO_0,CIO_0+P_A_DATA,CIO_0A
p1io	dw	PD2BASE,PDABASE,PD3BASE,PDBBASE
	dw	SCC_0B,SCC_0B+ICTSTAT
	dw	CIO_0,CIO_0+P_B_DATA,CIO_0B
p2io	dw	PD4BASE,PDCBASE,PD5BASE,PDDBASE
	dw	SCC_1A,SCC_1A+ICTSTAT
	dw	CIO_1,CIO_1+P_A_DATA,CIO_1A
p3io	dw	PD6BASE,PDEBASE,PD7BASE,PDFBASE
	dw	SCC_1B,SCC_1B+ICTSTAT
	dw	CIO_1,CIO_1+P_B_DATA,CIO_1B


	even
	public	_patterns
_patterns dw	 0001H, 0002H, 0004H, 0008H, 0010H, 0020H, 0040H, 0080H
	  dw	 0100H, 0200H, 0400H, 0800H, 1000H, 2000H, 4000H, 8000H
	  dw	0FFFEH,0FFFDH,0FFFBH,0FFF7H,0FFEFH,0FFDFH,0FFBFH,0FF7FH
	  dw	0FEFFH,0FDFFH,0FBFFH,0F7FFH,0EFFFH,0DFFFH,0BFFFH, 7FFFH
	  dw	 5A5AH,0A5A5H,0FFFFH, 0000H, 3639H

	even
	public	_scc_io_tab,_cio_io_tab
_scc_io_tab EQU	$
	dw	 0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, -1, -1, 28, 30
	dw	32, 32, 32, 32, 40, 40, 40, 40, 48, 50, 52, 54, 56, -1, 60,124
	dw	62,126
	dw	128,130,132,134,136,138,140,142,144,146,148,150, -1,-1,156,158
	dw	160,160,160,160,168,168,168,168,176,178,180,182,184,-1,188,124
	dw	190,126
_cio_io_tab EQU	$
	dw	0, 2,-1,16,20,26,32,34,44,46,56,64,68,70,72,74,76,78
	dw	0, 2,-1,18,22,28,36,38,48,50,58,80,84,86,88,90,92,94

; table of duscc command registers for each port
	even
	public	_scc_cmdr_tab
_scc_cmdr_tab dw SCC_0A+CH_CMD,SCC_0B+CH_CMD,SCC_1A+CH_CMD,SCC_1B+CH_CMD

;****************************************************************************
; Bus Master Channel dump area, used during certain NMI conditions
; Notice addition of space for the PCCSTAT register at the end.
	even
	public	_bm_area
_bm_area dw	9 DUP(?)	; [ 0- F]: Bus Master Channel Register Map
				; [10-11]: PCCSTAT

;****************************************************************************
; Parity Error dump area, used during certain NMI conditions
	even
	public	_pe_area
_pe_area dw	3 DUP(?)	; [0,1]:PESTAT,
				; [2]:RICPAR0, [3]:RICPAR1, [4]:RICPAR2

;****************************************************************************
; Variables used providing the offlevel two timers per port, Timer & Failsafe

	public	_ptimer0,_t_port0
	public	_ptimer1,_t_port1
;
	 even
_ptimer0 dw	NUM_PORT DUP(0FFFFh)	; Port wait values in T0 timer ticks
_t_port0 db	0			; Which port (0->?) is running
	 even
_ptimer1 dw	NUM_PORT DUP(0FFFFh)	; Port wait values in T0 timer ticks
_t_port1 db	0			; Which port (0->?) is running

;
; Adapter function call trace queue pointer
;
	even
	public	_tq_fnc,_tq_ret
_tq_fnc	dw	0			; Entry point trace queue
_tq_ret	dw	0			; Return address trace queue

	even
	public	_x21_Skip
_x21_Skip dw	01h
	public	_x21_Stat
_x21_Stat db	0

; The x.21 Call Progress Signal retry and netlog data structure
;
	even
	public	_x21_ctl
_x21_ctl db	312 DUP(0)

	public _disables
_disables db 	0 		; the number of times ints have been disabled
;
;
; Bisync protocol control character translate tables
;

	public	_asc_tbl
_asc_tbl 	EQU THIS BYTE
;		SOH STX ITB ETB ETX DLE ENQ EOT NAK SYN RVI AK0 AK1 WACK
	db	01h,02h,1Fh,17h,03h,10h,05h,04h,15h,16h,3Ch,30h,31h,3Bh
				
	public	_ebc_tbl
_ebc_tbl	EQU THIS BYTE
;		SOH STX ITB ETB ETX DLE ENQ EOT NAK SYN RVI AK0 AK1 WACK
	db	01h,02h,1Fh,26h,03h,10h,2Dh,37h,3Dh,32h,7Ch,70h,61h,6Bh

;-------------------------------------------------------------------------
;
;	Global table for bytewise calculation of CRC-16.  This table 
;	maps all possible values of the input byte exor'ed with the 		
;	CRC register (256 possible values) to a 16-bit quantity; 	
;	this result is then exor'ed with the shifted CRC register 	
;	to complete a single bytewise CRC computation.  This table 	
;	is generated as follows (lowest bit is 0):			
;									
;	Let I = input byte I0, I1, I2, . . ., I7.			
;	    C = current CRC register contents C0, C1, C2, . . . C15.	
;	    X = EXOR of I and C = C0 ^ I0, C1 ^ I1, . . ., C7 ^ I7.	
;	    R = value derived from X = R0, R1, R2, . . ., R15.		
;									
;	R can be described as a function of X, so X is used as an 	
;	index to lookup the value of R.  The function R(X) can be	
;	derived by algebraically performing eight shift/xors of a	
;	bitwise CRC-16 register:					
;									
;	    R0  = X0 ^ X1 ^ X2 ^ X3 ^ X4 ^ X5 ^ X6 ^ X7			
;	    R1  = 0							
;	    R2  = 0							
;	    R3  = 0							
;	    R4  = 0							
;	    R5  = 0							
;	    R6  = X0							
;	    R7  = X0 ^ X1						
;	    R8  = X1 ^ X2						
;	    R9  = X2 ^ X3						
;	    R10 = X3 ^ X4						
;	    R11 = X4 ^ X5						
;	    R12 = X5 ^ X6						
;	    R13 = X6 ^ X7						
;	    R14 = X0 ^ X1 ^ X2 ^ X3 ^ X4 ^ X5 ^ X6     			
;	    R15 = X0 ^ X1 ^ X2 ^ X3 ^ X4 ^ X5 ^ X6 ^ X7			
;									
;	This calculation is performed for all possible values of X	
;	to generate the following table:					

	public	_Crctbl
_Crctbl   dw  00000h, 0C0C1h, 0C181h, 00140h, 0C301h, 003C0h, 00280h, 0C241h
	  dw  0C601h, 006C0h, 00780h, 0C741h, 00500h, 0C5C1h, 0C481h, 00440h
	  dw  0CC01h, 00CC0h, 00D80h, 0CD41h, 00F00h, 0CFC1h, 0CE81h, 00E40h
	  dw  00A00h, 0CAC1h, 0CB81h, 00B40h, 0C901h, 009C0h, 00880h, 0C841h
	  dw  0D801h, 018C0h, 01980h, 0D941h, 01B00h, 0DBC1h, 0DA81h, 01A40h
	  dw  01E00h, 0DEC1h, 0DF81h, 01F40h, 0DD01h, 01DC0h, 01C80h, 0DC41h
	  dw  01400h, 0D4C1h, 0D581h, 01540h, 0D701h, 017C0h, 01680h, 0D641h
	  dw  0D201h, 012C0h, 01380h, 0D341h, 01100h, 0D1C1h, 0D081h, 01040h
	  dw  0F001h, 030C0h, 03180h, 0F141h, 03300h, 0F3C1h, 0F281h, 03240h
	  dw  03600h, 0F6C1h, 0F781h, 03740h, 0F501h, 035C0h, 03480h, 0F441h
	  dw  03C00h, 0FCC1h, 0FD81h, 03D40h, 0FF01h, 03FC0h, 03E80h, 0FE41h
	  dw  0FA01h, 03AC0h, 03B80h, 0FB41h, 03900h, 0F9C1h, 0F881h, 03840h
	  dw  02800h, 0E8C1h, 0E981h, 02940h, 0EB01h, 02BC0h, 02A80h, 0EA41h
	  dw  0EE01h, 02EC0h, 02F80h, 0EF41h, 02D00h, 0EDC1h, 0EC81h, 02C40h
	  dw  0E401h, 024C0h, 02580h, 0E541h, 02700h, 0E7C1h, 0E681h, 02640h
	  dw  02200h, 0E2C1h, 0E381h, 02340h, 0E101h, 021C0h, 02080h, 0E041h
	  dw  0A001h, 060C0h, 06180h, 0A141h, 06300h, 0A3C1h, 0A281h, 06240h
	  dw  06600h, 0A6C1h, 0A781h, 06740h, 0A501h, 065C0h, 06480h, 0A441h
	  dw  06C00h, 0ACC1h, 0AD81h, 06D40h, 0AF01h, 06FC0h, 06E80h, 0AE41h
	  dw  0AA01h, 06AC0h, 06B80h, 0AB41h, 06900h, 0A9C1h, 0A881h, 06840h
	  dw  07800h, 0B8C1h, 0B981h, 07940h, 0BB01h, 07BC0h, 07A80h, 0BA41h
	  dw  0BE01h, 07EC0h, 07F80h, 0BF41h, 07D00h, 0BDC1h, 0BC81h, 07C40h
	  dw  0B401h, 074C0h, 07580h, 0B541h, 07700h, 0B7C1h, 0B681h, 07640h
	  dw  07200h, 0B2C1h, 0B381h, 07340h, 0B101h, 071C0h, 07080h, 0B041h
	  dw  05000h, 090C1h, 09181h, 05140h, 09301h, 053C0h, 05280h, 09241h
	  dw  09601h, 056C0h, 05780h, 09741h, 05500h, 095C1h, 09481h, 05440h
	  dw  09C01h, 05CC0h, 05D80h, 09D41h, 05F00h, 09FC1h, 09E81h, 05E40h
	  dw  05A00h, 09AC1h, 09B81h, 05B40h, 09901h, 059C0h, 05880h, 09841h
	  dw  08801h, 048C0h, 04980h, 08941h, 04B00h, 08BC1h, 08A81h, 04A40h
	  dw  04E00h, 08EC1h, 08F81h, 04F40h, 08D01h, 04DC0h, 04C80h, 08C41h
	  dw  04400h, 084C1h, 08581h, 04540h, 08701h, 047C0h, 04680h, 08641h
	  dw  08201h, 042C0h, 04380h, 08341h, 04100h, 081C1h, 08081h, 04040h

; The virtual -1 entries at rate 0 are those found if external clocking has
; actually been selected.  This is important when txinc is set in BiSync, as
; the number of characters between SYN characters is unlimited.

	public	_bitrate,_byterate
_bitrate  dw	65535,75,110,134,150,200,300,600,1050,1200,2000
	  dw	2400,4800,9600,19200,38400
_byterate dw	65535,9,13,16,18,25,37,75,131,150,250
	  dw	300,600,1200,2400,4800

;
; Global (Adapter) statistics are kept in the following global regions
;
	public	_tx_dma,_tx_dma_bytes
	public	_tx_short,_tx_short_bytes
	public	_rx_dma,_rx_dma_bytes
	even
_tx_dma		DD	0	; Tx DMA transactions (Long, Gather add 1)
_tx_dma_bytes	DD	0	; Tx Long and Gather cuumulative byte counts
_tx_short	DD	0	; Tx Short commands invoked
_tx_short_bytes	DD	0	; Tx Short cuumulative byte count
_rx_dma		DD	0	; Rx DMA transactions
_rx_dma_bytes	DD	0	; Rx DMA cuumulative byte count, includes
				; frame header of at least six bytes/frame
	public	_lost_intr
_lost_intr	dw	0	; Interrupts lost.  If host_intr is called
				; a second time before the first is handled.

;
; Unprocessed Channel counter
;
	public	_rx_count
_rx_count	dw	NUM_PORT DUP(0)

;
; Frame size
;
        public  _rx_size
_rx_size        dw      NUM_PORT DUP(0)

_TEXT	ends
	end
