/*
static char sccsid[] = "@(#)80  1.7  src/bos/usr/lib/asw/mpqp/portcb.typ, ucodmpqp, bos411, 9428A410j 8/23/93 13:22:51";
*/

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	portcb.typ		: Typedef file for port control blocks
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include "cio.h"		/* CIO Structures and internals */
#include "duscc.h"		/* SCC Structures and internals */

/*
   The Port Control Block master type definition.
   Each MPQP adapter has exactly four of these in the adapter software.
*/

#define	bufno	unsigned char		/* buffer number = unsigned char */

#define	ioptr	unsigned short		/* io address = unsigned short */

typedef struct S_PCB
{
	unsigned char	port_no;   
	unsigned char	start_parm; 

	sch_lev		mask_on;	/* Scheduler mask, On (OR) */
	sch_lev		mask_off;	/* Scheduler mask, Off (~AND) */
/*
   The RX Control section of the PCB handles the interrupt level toggling
   of RX DMA channels and the signalling protocol to the work scheduler.
   The RX complete area, rx_work, is only one item deep!  In all cases,
   a Buffer Number of 0xFF means a channel is not setup.
*/
	unsigned char	rx_next;	/* Next allocated Rx Buffer # */
	unsigned char	rx_curr;	/* Current Rx Buffer # */
	unsigned char	rx_last;	/* Just completed Rx Buffer # */
	unsigned char	rxchan;		/* Even/Odd channel indicator */
	unsigned short	rx_ccw;		/* Channel default Rx CCW */
	unsigned short	rx_cmb;		/* Channel default Rx Character
					   Match bytes */
	unsigned short	rx_off;		/* Data Offset in each frame */
	unsigned short	rx_flgt;	/* Frame length in bytes */
/*
   The TX Control section of the PCB works to keep the transmitter busy
   and allows, via the tx_work byte, for the off-level tx packet processor
   to get one packet ahead of the actual physical link.  The only comm.
   method back to the scheduler is via SCC, CIO or DMA interruption.
*/
	unsigned char	txbuf0;		/* Even TX Buffer Number */
	unsigned char	txbuf1;		/* Odd TX Buffer Number */
	unsigned char	txchan;		/* Even/Odd channel indicator */
	unsigned char	tx_pad;		/* Pad byte, Internal clocking */
/*
   Point to the Port Command and Response Queues
*/
	byte_q		*cmd_q;		/* Port Command Queue Address */
	rqe_q		*rsp_q;		/* Port Response Queue Address */
/*
   Each channel has certain hardware resources at different I/O addresses.
   This portion of the PCB identifies significant hardware locations for
   both off-level and ISR routines.
*/
	ioptr		rx_dma_0;	/* Port RX DMA channel 0 */
	ioptr		rx_dma_1;	/* Port RX DMA channel 1 */
	ioptr		tx_dma_0;	/* Port TX DMA channel 0 */
	ioptr		tx_dma_1;	/* Port TX DMA channel 1 */

	ioptr		scc_base;	/* Port DUSCC base address */
	ioptr		scc_data;	/* Port DUSCC data reg. I/O addr */
	ioptr		cio_base;	/* Port CIO base I/O address */
	ioptr		cio_data;	/* Port CIO data register I/O addr */
	ioptr		alt_cio;	/* Alternate CIO base */
	unsigned char	cio_port;	/* Which port, A = 0, B = 1 */
	unsigned char	cio_last_data;	/* Last CIO data value read */

/* depending on the operating protocol, the receive and transmit processing
   functions change.  Rather than move some level of intelligence about the
   protocol up to the scheduler vector functions (i.e. f_rxwork), control
   is routed (by setting the following two function pointers) when para-
   meters are set.  For notes on the required return codes, see the applic-
   able scheduler service functions.  These should be NULL when the port
   is not in a mode capable of moving data. */

	unsigned int	( *f_txserve )();	/* Tx data preprocessor  */
	unsigned int	( *f_txframe )();	/* Tx frame routine */
	unsigned int	( *f_rxserve )();	/* Rx data postprocessor */

	t_rqe			sleep_rqe;	/* RQE removed which re-  */
						/* scheduled a sleeper.   */
	t_rqe			error_rqe;	/* Storage during err/sts */
						/* offlevel processing.   */

	t_rqe			start_rqe;	/* Storage during start   */
						/* processing. 		  */

	/* Flags and Port configuration: */

	unsigned short		baud_rate;
	unsigned char		modem_int;
	unsigned char		modem_state;
	unsigned char		port_state;
	unsigned char		port_status;
	unsigned char		x21_state;
	unsigned char		proto;
	unsigned char		flags;
	unsigned char		field_select;
	unsigned char		modem_mask;
	unsigned char		phys_link;
	unsigned char		poll_addr;
	unsigned char		select_addr;
	unsigned char		dial_proto;
	unsigned char		dial_flags; 
	unsigned char		data_proto;
	unsigned char		data_flags; 

	unsigned char		timer_type;
	unsigned char		fs_timer_type;
	unsigned short		rx_timeout;

	/* State Information:	*/

	unsigned short		rx_state;	/* rx state machine ctl */
	unsigned short		tx_state;	/* tx state machine ctl */

	unsigned short		recv_type;	/* receive message type */
	unsigned short		bsc_prev_rx;	/* Previous bisync frame, Rx */
	unsigned short		bsc_prev_tx;	/* Previous bisync frame, Tx */

	/* Dial Information for Modems: */

	bufno  far		*dial_data;

	/* Statistics: */

	unsigned long		tx_bytes;
	unsigned long		rx_bytes;

	unsigned long		tx_frames;
	unsigned long		rx_frames;

	/* Bisync character stream variables: */

	unsigned char far	*destptr;	/* ptr to dest buffer */
	unsigned char far	*destlimit;	/* end of dest buffer */
	unsigned int		destlen;	/* length of dest buff */
	unsigned int		write_chars;	/* # of chars written */

	unsigned char far	*srcptr;	/* ptr to src buffer */
	unsigned char far	*srclimit;	/* end of src buffer */
	unsigned int		srclen;		/* length of src buff */
	unsigned int		read_chars;	/* # of chars read */

	unsigned short		retry_in_prog;	/* x21 retry flag */

	t_pdma_rx_llc		pdma_rx_llc[2];	/* one for each channel */

	t_scc_reg		scc_reg;
       	t_cio_reg		cio_reg;    

/* When offlevel takes control of the e_block, it's moved from the global
   eblock array "estat [NUM_PORT]" here so the ISRs have a fresh one to use */

	eblock			estat;

	struct S_ARESP		auto_resp;

	t_pdma_tx_llc		pdma_tx_llc[2];	/* one for each channel */
} t_pcb;


/* The following provides linkage to the port control blocks */

extern volatile t_pcb           pcb [NUM_PORT];

/* The scc default areas below, provide initialization values for scc 
registers in bisync, sdlc, async, x21 mode */

/* Defaults for EIA 232, 422 and V.35 physical links */
extern t_scc_reg  scc_def_bi_e;	/* SCC defaults, BiSync EBCDIC */ 
extern t_scc_reg  scc_def_bi_a;	/* SCC defaults, BiSync ASCII/Odd Parity */ 
extern t_scc_reg  scc_def_cds_e;/* SCC defaults, COP DUAL SYN EBCDIC */ 
extern t_scc_reg  scc_def_cds_a;/* SCC defaults, COP DUAL SYN ASCII/Odd Par. */ 
extern t_scc_reg  scc_def_sdlc;	/* SCC defaults, sdlc */ 
extern t_scc_reg  scc_def_asy;	/* SCC defaults, async */ 
/* Defaults for X.21 physical links */
extern t_scc_reg  scc_def_x21;	/* SCC defaults, x21 during call estab */ 
extern t_scc_reg  scc_x21_bi_e;	/* SCC defaults, BiSync EBCDIC on x.21 */ 
extern t_scc_reg  scc_x21_bi_a;	/* SCC defaults, BiSync ASCII/Odd on x.21 */ 
extern t_scc_reg  scc_x21_cds_e;/* SCC defaults, COP DUAL SYN EBCDIC on x.21 */ 
extern t_scc_reg  scc_x21_cds_a;/* SCC defaults, COP DUAL SYN ASCII/Odd x.21 */ 
extern t_scc_reg  scc_x21_sdlc;	/* SCC defaults, sdlc on x.21 */ 

/* External calls by STARTP and SETPARM modules  */
extern int	init_scc   ( t_pcb   *p_pcb );
extern int	set_sdlc   ( t_pcb   *p_pcb );
extern int	set_async  ( t_pcb   *p_pcb );
extern int	set_bisync ( t_pcb   *p_pcb );
extern int	set_x21    ( t_pcb   *p_pcb );

/*
   Sleep services.  Added 05/18/89 b
*/

extern t_pcb 			*sleep( volatile t_pcb    * );
extern void			sleep_enable( volatile t_pcb    * );
extern void			sleep_disable( volatile t_pcb    * );

extern volatile t_pcb		*ProcSleep ();

extern void			add_rqe( volatile t_pcb *, unsigned long );
