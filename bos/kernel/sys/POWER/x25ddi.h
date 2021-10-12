/* @(#)89  1.6  src/bos/kernel/sys/POWER/x25ddi.h, sysxx25, bos411, 9428A410j 4/16/92 16:54:15 */
#ifndef _H_X25DDI
#define _H_X25DDI

/* COMPONENT_NAME: (SYSXX25) X.25 Device Handler Interface
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/watchdog.h>
#include <sys/intr.h>

#define PVC_SIZE                  64
#define MAX_SEARCH_LENGTH         1024
#define MAX_PATH_LENGTH	          128 
#define MAXSLOTS                  16
#define RUN_MODE                  0
#define DIAGNOSTIC_MODE           1
#define MAX_NON_DF_ATT_SIZE       30
#define MIN_MAX_PKT_SIZE          256
#define MAX_ALLOWABLE_PKT_SIZE_80 1024

struct DOWNLOAD_CMD
{
	char	 *p_mcode_x25;
	char	 *p_mcode_rcm;
	char	 *p_mcode_diag;
	int	 l_mcode_x25;
	int	 l_mcode_rcm;
	int	 l_mcode_diag;
};

struct ras_struct
{
	int	 dummy;
};

struct	vpd_struct
{
	int	 dummy;
};

typedef struct x25_ddi_hdw_struct
{
	int	 bus_type;		/* for use with i_int		     */
	int	 bus_id;		/* for use with BUSACC, i_init	     */
	int	 bus_intr_lvl;		/* for use with i_init		     */
	int	 intr_priority;		/* for use with i_init		     */
	int	 offl_level;		/* for use with i_sched		     */
	int	 offl_priority;		/* for use with i_sched		     */
} x25_ddi_cc_section_t;

typedef struct x25_static_ddi
{
	int 	slot;			/* slot the card is in		     */
	int 	bus_io_addr;		/* for use with i_sched		     */
	int 	bus_mem_addr; 		/* address of shared memory	     */
	int 	window_size;		/* size of memory window	     */
	char	resource_name[6];	/* logical name of the device	     */

	/*********************************************************************/
	/*                       Network parameters                          */
	/*********************************************************************/
 	ushort	network_id;
	char	local_nua[16];
	ushort	in_svc;			/* an lcn after the PVCs	     */
	ushort	num_in_svcs;
	ushort	in_out_svc;		/* an lcn after the ic SVCs	     */
	ushort	num_in_out_svc;
	ushort	out_svc;		/* an lcn after the 2-way SVCs	     */
	ushort	num_out_svcs;
	ushort	pvc_channel;		/* lowest logical channel number     */
					/* for a PVC number of PVCs	     */
	ushort	num_of_pvcs;		/* each PVC has an entry in the	     */
					/* PVC table. max 64 per line 	     */
	uchar	acu_mode;

	/*********************************************************************/
	/*                      Packet parameters                            */
	/*********************************************************************/
	uchar	ccitt_support;   	/* TRUE use 1984 extentions	     */
					/* typically true		     */
	uchar	pkt_modulo;  		/* 0-modulo8, 1-modulo128 	     */
					/* typically : 0		     */
	ushort	line_type;		/* 0 DTE, 1 DCE typically 0 	     */

	/*********************************************************************/
	/* Default parameters for SVCs                                       */
	/*********************************************************************/
	ushort	def_rx_pkt_size;	/* SVCs only. 16-4096 bytes	     */
					/* typical value: 2		     */
	ushort	def_tx_pkt_size;	/* SVCs only. 16-4096 bytes	     */
					/* typically: 128		     */
	ushort	def_rx_pkt_win;	        /* 1-7 Modulo8,1-127 Modulo 128      */
					/* typical value: 2		     */
	ushort	def_tx_pkt_win;		/* same as for rx_window	     */
	uchar	def_rx_through;		/* as CCITT 3-12.		     */
					/* typically 10	(9600)		     */
	uchar	def_tx_through;		/* as CCITT 3-12.		     */
					/* typically: 10 (9600)	 	     */

	/*********************************************************************/
	/* Maximum negotiable parameters for SVCs                            */
	/*********************************************************************/
	ushort	max_rx_pkt_size;	/* SVCs only. Max pkt size that      */
					/* can be negotiated. 128-4096       */
					/* bytes. typical value: 128	     */
	ushort	max_tx_pkt_size;	/* same for transmission	     */
	ushort	max_rx_pkt_win;		/* max window size that can be       */
					/* negotiated 2-7 Modulo 128	     */
					/* typical value: 3		     */
	ushort	max_tx_pkt_win;		/* same for rx_window		     */
	uchar	max_rx_through;		/* maximum negotiable throughput     */
					/* as CCITT 3-12.		     */
					/* typically: 12 (4800)		     */
	uchar	max_tx_through;		/* same as max_rx_throughput	     */
	ushort	max_rx_packet;		/* Maximum size for incoming 	     */
					/* packets on the link		     */
	
	/*********************************************************************/
	/* Timers - ISO8208 defined timers                                   */
	/*********************************************************************/
	uchar	t21_timer;		/* call req timeout in seconds       */
					/* 1-255. typical value : 200        */
	uchar	t22_timer;		/* reset req timeout in seconds      */
					/* 1-255. typical value : 180	     */
	uchar	t23_timer;		/* clear req timeout in seconds      */
					/* 1-255 . typical value : 180       */
	uchar	t24_timer;		/* set when pkt carrying an ack      */
					/* if timer expires an RR is sent    */
					/* 1-255 secs, 0 disables timer      */
					/* typical value : 180		     */
	uchar	t25_timer;		/* pkt level T1, set when pkt        */
					/* sent reset when ack received      */
	uchar	t26_timer;		/* intrrupt req timeout              */
					/* 1-255 secs, 0 disables timer      */
					/* typical value : 180               */

	/*********************************************************************/
	/* Packet level features                                             */
	/*********************************************************************/
	uchar	neg_through;		/* Throughput negotiation	     */
					/* throughput request	 	     */
					/* negotiated by the card.	     */
					/* typical value: TRUE		     */
	uchar	neg_pkt_size;		/* Packet Size negotiation	     */
					/* TRUE card substitutes possible    */
					/* pkt sizes and window sizes if     */
					/* facilities requested cannot       */
					/* be met. typical value: TRUE       */
	uchar	calls_in;		/* TRUE all incoming calls are       */
					/* cleared. typical value: TRUE      */
	uchar	calls_out;		/* TRUE no outgoing calls are        */
	uchar	fast_select;		/* TRUE allow fast select 	     */
					/* typical value: TRUE		     */
	uchar	d_bit;			/* TRUE Dbit allowed on SVCs         */
	uchar	max_reset;		/* max reset pkts to send before     */
					/* VC fail. 0-127, typically 5       */
	uchar	max_clear;		/* max clear pkts to send before     */
					/* VC fail, 0-127, typically 5       */
	uchar	cug;			/* cug subscription parameter        */
					/* 0x00:no CUG			     */
					/* 0x01:i/c & o/g without	     */
					/* preferential 		     */
					/* 0x02:o/g without preferential     */
					/* 0x04:i/c without preferential     */
					/* 0x08:i/c & o/g with		     */
					/* preferential 		     */
					/* 0x10:o/g with preferential        */
					/* 0x20:i/c with preferential        */
					/* 0x40:with preferential	     */
	uchar	bcug;			/* TRUE facilities indicating	     */
					/* membership of a BCUG are	     */
					/* allowed 			     */
					/* otherwise the call will be 	     */
					/* cleared			     */
					/* typical value: FALSE		     */
	uchar	rev_charging;		/* TRUE reverse charge calls are     */
					/* accepted. typical value: FALSE    */
	uchar	charge_prev;
					/* TRUE all outgoing calls must      */
					/* be reverse charging.		     */
					/* typical value: FALSE		     */
	uchar	restart_on_line;	/* TRUE a restart packet is sent     */
					/* on level2 connection. 	     */
					/* typical value: TRUE 	    	     */

	/*********************************************************************/
	/*                        Frame parameters                           */
	/*********************************************************************/
	ushort	frame_window;		/* frame window size 		     */
	int	clock;			/* clocking source for card	     */
	uchar	tx_speed;		/* used with internal clocking	     */
					/* 1-50, 2-75, 3-100, 4-110 	     */
					/* 5-134.5, 6-150, 7-200, 8-300	     */
					/* 9-600, 10-1200, 11-1800	     */
					/* 12-2400, 13-4800, 14-9600	     */
					/* 15-19200, 16-4800, 17-56000	     */
					/* 18-64000, 19-72000		     */
	ushort	t1_timer;		/* standard CCITT 50ms units	     */
					/* typical value : 60		     */
	uchar	t4_timer;		/* ISO timer RR on this much	     */
					/* inactivity			     */
					/* typical value : 10 seconds	     */
	uchar	n2_counter;		/* CCITT N2 counter 		     */
	uchar	connection_mode;	/* how frame level is to start	     */
					/* 0x00 wait SABM send UA	     */
					/* 0x01 send SABM wait UA	     */
	uchar	dcd_retry_cntr;		/* num of times the physical 	     */
					/* layer is checked after  	     */
					/* start_physical before it is	     */
					/* considered to have failed	     */
					/* typical value : 11		     */
	uchar	dcd_polling_tmr;	/* DCD and/or DSR are checked	     */
					/* to see the physical layer	     */
					/* is still up			     */
					/* typical value:10 (50ms unots)     */
	uchar	dcd_polling_ctr;	/* 0-DCD is tested each time and     */
					/* no retry is attempted if DCD      */
					/* found down 1-255 DSR and DCD      */
					/* are tested and retried this 	     */
					/* many time if found to be down     */
	uchar	frame_modulo;		/* 8 or 128. typically 8	     */

	/*********************************************************************/
	/*                      PVC parameters                               */
	/*********************************************************************/
	ushort	pvc_rx_size;
	ushort	pvc_tx_size;
	uchar	pvc_rx_win;
	uchar	pvc_tx_win;
	uchar	pvc_d_bit;
	uchar	pvc_reset;

	/*********************************************************************/
	/* General parameters                                                */
	/*********************************************************************/
	uchar	rdto;			/* receive data transfer offset      */
	ushort  lms_max_bytes;          /* The number of bytes of data       */
					/* from within packets that will     */
					/* be displayed. 		     */
					/* typical value: max packet size    */
	uchar   lms_flow; 		/* Should the lms start		     */
					/* controlling the data flow on      */
					/* the line if it is short of	     */
					/* buffers. typical value: False     */
	uchar	zero_address;
	ushort	acu_timeout;		/* only on ACU links. time	     */
					/* between last call being   	     */
					/* cleared and the link being	     */
					/* disconnected.		     */
					/* 1-65536 seconds 0 for no 	     */
					/* timeout			     */
        uchar  link_restart;            /* Should the link be restarted      */
                                        /* automatically on non-requested    */
                                        /* disconnections                    */
        ushort  restart_timer;          /* period between restart attempts   */


	/*********************************************************************/
	/*                NON DISPLAYING ATTRIBUTES                          */
	/*********************************************************************/
	long	facs_map;		/* Facilities Control		     */
					/* Allow/Disable		     */
					/* Packet Size, Window Size          */
					/* Throughput class		     */
					/* CUG basic format		     */
					/* CUG extended format		     */
					/* CUG with OA selection basic	     */
					/* format			     */
					/* CUG with OA selection 	     */
					/* extended format		     */
					/* BCUG				     */
					/* Reverse charging or fast 	     */
					/* select			     */
					/* NUI				     */
					/* Charging requesting service	     */
					/* Receiving information about	     */
					/* monetary unit 		     */
					/* Receiving information about	     */
					/* segment count		     */
					/* Receiving information about	     */
					/* call duration		     */
					/* RPOA basic format		     */
					/* RPOA extended format		     */
					/* Called line address modified	     */
					/* notification			     */
					/* Call redirection notification     */
					/* Transit delay selection and	     */
					/* notification			     */
					/* Marker code, 0x00 calling	     */
					/* network facs			     */
					/* Marker code, 0xFF called	     */
					/* network facs			     */
					/* Marker code, 0x0F CCITT-DTE	     */
					/* facilites			     */
	uchar	pkt_feat1;		/* Network specific feature 	     */
					/* control			     */
 					/* call_accept_user_data	     */
					/* disable_cconf_dbit		     */
					/* ddn_diag_codes		     */
					/* enforce_clear_length		     */
					/* unassigned_lcn		     */
					/* send_reset_on_pvc		     */
					/* collision_notification	     */
	uchar	pkt_feat2;		/* Enforce clear confirm length      */
					/* Ban duplicate facilites	     */
	uchar	pkt_feat3;		/* Allow short restart packets       */
					/* Allow short clear packets 	     */
					/* Allow short reset packets 	     */
					/* Allow short connect packets 	     */
	uchar	frame_ft;		/* Network specific feature	     */
					/* control			     */
					/* force_frmr			     */
					/* force_frmr_rr_only		     */
					/* clear_pf 			     */
					/* disc_answer			     */
					/* disc_action			     */
					/* info_count			     */
					/* n2_action			     */
					/* disable_dm			     */
	uchar	min_through;
	uchar	max_through;  
	ushort	min_pkt;
	ushort	max_pkt;
	uchar	t20_timer;		/* restart req timeout in seconds    */
					/* 1-255. typical value : 180        */
	uchar	max_restart_pkt;	/* max restart pkts to send	     */
					/* before VC fail, 0-127.	     */
					/* typically: 5		 	     */

	ushort  total_luns;             /* the total number of luns that will*/
	                                /* will be opened                    */

	uchar	line_protocol;
	ushort	non_dflt_pvcs;
	uchar	num_of_pools;		/* The number of buffer pools	     */
					/* typical value: 4		     */
	ushort 	num_of_cells;		/* Number of event cells used 	     */
					/* for interprocess communication    */
	uchar	num_short_time;		/* Used to manage T1	             */
					/* typical value: 5 per line	     */
	ushort	num_long_time;		/* Used by to control pkt            */
					/* reply response timing	     */
					/* typical value: 5 per line + 7     */
					/* per VC			     */
	uchar	max_ic_calls;		/* The maximum number of incoming    */
					/* calls that can be waiting to      */
					/* be read. Probably not a           */
					/* problem for us as we read the     */
					/* calls quickly.		     */
					/* typical value: num of i/c SVCs    */
					/*	  + num of 2-way SVCs 	     */
	ushort	size_lms_buffer;
	ushort	num_lms_buffers; 
	ushort	size_buffer_1;
	ushort	num_in_pool_1;
	ushort	size_buffer_2;
	ushort	num_in_pool_2;
	ushort	size_buffer_3;
	ushort	num_in_pool_3;
	ushort	size_buffer_4;
	ushort	num_in_pool_4;
	ushort	size_buffer_5;
	ushort	num_in_pool_5;
	ushort	size_buffer_6;
	ushort	num_in_pool_6;
	ushort	size_buffer_7;
	ushort	num_in_pool_7;
	ushort	size_buffer_8;
	ushort	num_in_pool_8;
	uchar	max_rx_buffers;		/* num of rx buffers pre	     */
					/* allocated. typical value : 16     */
	uchar	min_rx_buffers;		/* minimum number with which the     */
					/* system will run	  	     */
					/* typical value : 4                 */
	uchar	min_free_buffer;	/* When free buffers fall below      */
					/* this, RNR will be generated       */
					/* typical value: 10	             */
	uchar	free_buff_timer;	/* if the number of free buffers     */
					/* falls below min_free, it is       */
					/* checked again at this interval    */
					/* (50ms units).typical value:10     */
	uchar	highest_lcn;		/* highest lcn(+1) that will be      */
					/* used, keep as low as possible     */
					/* typically 9	                     */
	uchar	negotiation;
} x25_ddi_ds_section_t;

/*****************************************************************************/
/* Non default PVC structure                                                 */
/*****************************************************************************/
struct x25_pvc
{
	ushort	pvc_lcn;		/* if special_pvc_mode TRUE then     */
					/* lcn given explicitly,otherwise    */
					/* ordered from lowest pvc	     */
					/* (0-4096). typical value: 0        */
	ushort	pvc_rx_size;    	/* 16-4096, typically 128            */
	ushort	pvc_tx_size;		/* 16-4096, typically 128	     */
	uchar	pvc_rx_win;		/* 1-7 modulo8, 1-127 modulo128      */
	uchar	pvc_tx_win;		/* 1-7 modulo8, 1-127 modulo128      */
	uchar	pvc_d_bit;		/* TRUE D bits allowed, otherwise    */
					/* circuit is reset		     */
	uchar	pvc_reset; 	 	/* 0x00 to 0xff - def 0x00	     */
					/* default value doesn't send 	     */
					/* reset			     */
}; 
       
struct x25_ddi_struct
{
	x25_ddi_cc_section_t	cc;
	x25_ddi_ds_section_t	ds;
};

struct x25_dds_struct
{ 
	struct	intr		ihs_section;	/* Interrupt Handler control */
	struct	watchdog	wdt_section;	/* Watchdog timer ctrl struct*/
	struct	ras_struct	ras_section;	/* Statistics - as in comio.h*/
	struct	vpd_struct	vdp_section;	/* Vital Data - as in comio.h*/
	struct	x25_ddi_struct	ddi;
};
#endif
