/*
static char sccsid[] = "@(#)78  1.7  src/bos/usr/lib/asw/mpqp/mpqp.typ, ucodmpqp, bos411, 9434B411a 6/16/94 10:05:38";
*/

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: 
 *	mpqp.type		: Type defs for commonly used structures for
 *				  command blocks, receive buffers, queues, etc.
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

/*
   MPQP Master type and structure definition file
*/

/* A Response Queue Element is 32 bits in length.  The "sequence" field
   is provided specifically for command acknowledgement pacing, but may
   contain other data on Rx Data RQEs. */

typedef union
{
	struct {
		unsigned short		port    :  4;
		unsigned short		rtype   :  4;
		unsigned short		status  :  8;
		unsigned short		sequence;
	       } f;
	unsigned long		val;
} t_rqe;

typedef struct CMD_BLK
{
	unsigned char			_type;		/* Command Type */
	unsigned char			_port;		/* Port Identifier */
	unsigned short			_sequence;	/* Sequence ID */
	unsigned short			_rsvd;		/* reserved */
	unsigned char			_lgt;		/* Length */
	unsigned char			_ctl;		/* Control */
	unsigned char			_nusd [8];	/* Not Used */
	union {
		unsigned char		_parm [48];
		struct {
			unsigned long	_pmem;		/* pointer */
			unsigned short	_tlgt;		/* test length */
		       } _mtest;			/* Ram/Checksum */
		struct S_ARESP
		       {
			unsigned char	_thi;		/* time, high */
			unsigned char	_tlo;		/* time, low */
			unsigned char	_addr;		/* Tx/Rx/Addr */
			unsigned char	_txctl;		/* Tx Control */
			unsigned char	_rxctl;		/* Rx Control */
			unsigned char	_sa_fill;	/* Filler */
		       } _aresp;			/* auto response */
		struct {
			unsigned char	_proto;		/* Protocol */
			unsigned char	_s1;
			unsigned char	_s2;
			unsigned char	_mode;
			unsigned char	_clgt;		/* Char length */
			unsigned char	_rate;		/* Baud Rate */
			unsigned char	_conn;		/* Connection */
		       } _ddiag;			/* DUSCC diag. */
		struct {
			unsigned short	_off;
			unsigned char	_loc;
			unsigned char	_pad;
			unsigned char	_pairs[44];
		       } _sreg;
	      } p;
} cmd_blk;


typedef struct BYTE_Q
{
	unsigned char		length;
	unsigned char		end;
	unsigned char		out;
	unsigned char		in;
	unsigned char		q [NBUF+4];
} byte_q;

typedef struct WORD_Q
{
	unsigned char		length;
	unsigned char		end;
	unsigned char		out;
	unsigned char		in;
	unsigned short		q [NBUF+4];
} word_q;

typedef struct RQE_Q
{
	unsigned char		length;
	unsigned char		end;
	unsigned char		out;
	unsigned char		in;
	t_rqe			q [NBUF];
} rqe_q;

typedef struct EBLOCK
{
	unsigned short		e_type;
	unsigned char		e_rsr;
	unsigned char		e_trsr;
	unsigned char		e_ictsr;
	unsigned char		e_ciodata;
	unsigned char		e_x21pal;
	unsigned char		e_spare;
} eblock;

/*
   Receive Data Buffer Header
*/

typedef struct
{
	unsigned short		offset;  /* used for port dma status */
	unsigned short		length;
	unsigned short		type;
} rx_buf_hdr;


/* Template for transmit command blocks: */

typedef struct TX_COMMON
{
unsigned char		_type;		/* Command type */
unsigned char		_port;		/* Encoded Port Number */
unsigned short		_sequence;	/* Sequence Number */
unsigned char		_rsvd0[2];	/* Reserved */
unsigned char		_txs_lgt;	/* For TxShort, length */
unsigned char		_ctl;		/* Transmit Control Bits */
unsigned char far	*_addr;		/* Card Address of data  */
unsigned short		_lgt;		/* Card Buffer length    */
unsigned short		_rsvd1;		/* Reserved */
unsigned char		_data[48];	/* For TxShort, actual data */
} t_tx_common;

/* Yet another template for transmit commands: */

typedef struct TX_L_CMD
{
	unsigned char		_type;		/* Command type */
	unsigned char		_port;		/* Encoded Port Number */
	unsigned short		_sequence;	/* Sequence Number */
	unsigned short		_rsvd;		/* Reserved */
	unsigned char		_empty;		/* Reserved */
	unsigned char		c_comp;		/* Xmit Control Bits (compat) */
	unsigned char far	*p_comp;	/* Card Addr of data (compat) */
	unsigned short		l_comp;		/* Card Buffer len   (compat) */
	unsigned short		_spare;		/* Reserved */
	unsigned long		_addr;		/* Host System address */
	unsigned short		_lgt;		/* Host Buffer length */
	unsigned char		_ctl;		/* Transmit Control bits */
} t_txl_cmd;

/* The command block template to end all (other) command block templates */

typedef struct {
    unsigned char		type;		/* Command Type */
    unsigned char		port;		/* Port Number */
    unsigned short		sequence;	/* Sequence Number */
    unsigned short		length;		/* Length of data */
    unsigned short		flags;		/* Control flags */
    unsigned long		host_addr;	/* Host address of data */
    unsigned char far		*card_addr;	/* Card address of data */

    union {					/* COMMAND SPECIFIC: */

        unsigned char		data[48];	/*  TX Short Data */

	struct {				/*  Set Parm Fields */
	    unsigned char	field_sel;	/*    field select */
	    unsigned char	modem_int;	/*    modem interupt */
	    unsigned char	phys_link;	/*    physical link */
	    unsigned char	poll_addr;	/*    poll address */
	    unsigned char	select_addr;	/*    select address */
	    unsigned char	dial_proto;	/*    autodial protocol */
	    unsigned char	dial_flags;	/*    dial flags */
	    unsigned char	data_proto;	/*    data protocol */
	    unsigned short	data_flags;	/*    data flags */
	    unsigned short	recv_timer;	/*    receive timer */
	    unsigned short	baud_rate;	/*    baud rate */
	} set_parm;

	struct autoresp {			/* Auto Response Fields */
	    unsigned char	time_hi;	/*    Time, High */
	    unsigned char	time_lo;	/*    Time, Low */
	    unsigned char	address;	/*    TX/RX Address */
	    unsigned char	xmit_ctrl;	/*    transmit control */
	    unsigned char	recv_ctrl;	/*    receive control */
	    unsigned char	unused;
	} auto_resp;

	struct autodial {			/* Auto Dial */
	    unsigned char	modem_flags;	/* modem flags */
	    unsigned char	unused;
	    unsigned short	rdto;		/* Rcvd Data Transfer Offset */
	    unsigned short	connect_timer;	/* time to wait for DSR */
	    unsigned short	v25b_tx_timer;	/* wait time to xfer data */
            unsigned short      max_rx_bsiz;    /* Maximum receive buffer size*/
	} auto_dial;

	struct {				/* Register Setup */
	    unsigned short	offset;		/*    offset to reg pairs */
	    unsigned char	in_buffer;	/*    TRUE if in buffer */
	    unsigned char	unused;
	    unsigned char	pairs[44];	/*    reg/value pairs */
	} reg_setup;

	struct {
	    unsigned char       parm;   	/* Command parameter */
	    unsigned char       filler; 	/* Not used */
	    unsigned short      RDTO;   	/* Receive data offset */
            unsigned short      filler1;        /* time to wait for DSR */
            unsigned short      filler2;        /* wait time to xfer data */
            unsigned short      max_rx_bsiz;    /* Maximum receive buffer size*/
	} start_port;

   } cs;
} cmd_t;

/*
   Bus Master DMA Transmit object type
*/

typedef union
{
	t_rqe	rx;				/* Rx RQE */
	struct {
		unsigned char	offset;		/* Port CQ Offset */
		unsigned char	cmdblk;		/* Command Block # */
	       } tx;
	unsigned short	op;
} t_dma_op;

typedef struct
{
	unsigned short		_old_tc;
	unsigned short		_status;
	unsigned short		_llc_ind;
	unsigned short		_new_tc;
	unsigned short		_ta;
	unsigned short		_tae;
	unsigned short		_ccw;
	unsigned short		_lae;
	unsigned short		_la;
} t_pdma_rx_llc;

typedef struct
{
	unsigned short		_llc_ind;
	unsigned short		_new_tc;
	unsigned short		_ta;
	unsigned short		_tae;
	unsigned short		_ccw;
	unsigned short		_lae;
	unsigned short		_la;
} t_pdma_tx_llc;

typedef struct
{
	unsigned char far	*_ca;	/* Card address */
	unsigned short		_tc;	/* Transfer count */
	unsigned short		_ioa;	/* Fifo address */
	unsigned short		_cmb;	/* Character Match bytes (Rx only) */
	unsigned short		_ccw;	/* Channel Control word (-ENABLE) */
	union
	{   t_pdma_rx_llc far   *_rxla; /* list pointer, Receive */
	    t_pdma_tx_llc far	*_txla; /* list pointer, Transmit */
        } p;
} t_pdma_setup;

/*
   The x.21 call progress retry and network error logging data structure
   is in global memory, as there is only one port with x.21.
*/

typedef struct
{
	unsigned short		sel_lgt;
	unsigned char		sel_sig [256];
	unsigned char		retry_count;
	unsigned char		notused;
	unsigned short		retry_delay;
	unsigned short		retry[10];
	unsigned short		netlog [10];
	unsigned char		log_cnt[10];
} t_x21_ctl;
