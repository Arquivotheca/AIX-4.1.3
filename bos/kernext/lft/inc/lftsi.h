/* @(#)91       1.2  src/bos/kernext/lft/inc/lftsi.h, lftdd, bos411, 9428A410j 7/6/94 10:49:41 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _L_LFTSI
#define _L_LFTSI

/* ------------------------------------------------------------------------
   if this is the first LFTSI defined; then set up the structures 
   shared across all the LFTSI's installed. The lft_streams_init module should
   define INITLFT to be 1 before including this header. All other modules that
   include this header should not define INITLFT.
   This will give them the declaration of these variables, but not cause a 
   redefiniton problem.
   ------------------------------------------------------------------------- */


#define LFT_RD_HI	512	/* Read side HI water mark (bytes) */
#define LFT_RD_LO	256	/* Read side LO water mark (bytes) */
#define LFT_WR_HI	512	/* write side HI water mark (bytes) */
#define LFT_WR_LO	256	/* write side LO water mark (bytes) */

#define	LFTMSGSIZE	64	/* input buffer size (bytes) */


#ifdef INITLFT
int lftopen(), lftclose(), lftwput(), lftrsrv(), lftwsrv();
struct module_info rminfo={
	0,		/* module ID number */
	"lft",		/* module name */
	0,		/* minimum packet size accepted */
	INFPSZ,		/* max packet size accepted */
	LFT_RD_HI,	/* hi-water mark for flow control */
	LFT_RD_LO	/* lo-water mark for flow control */
	};

struct module_info wminfo={
	0,		/* module ID number */
	"lft",		/* module name */
	0,		/* minimum packet size accepted */
	INFPSZ,		/* max packet size accepted */
	LFT_WR_HI,	/* hi-water mark for flow control */
	LFT_WR_LO	/* lo-water mark for flow control */
	};

struct qinit rinit={	/* read QUEUE */
	NULL,		/* there is no read side put procedure */
	lftrsrv,	/* read side service procedure */
	lftopen,	/* called on each open or a push */
	lftclose,	/* called on each close or a pop */
	NULL,		/* future use */
	&rminfo,	/* information structure */
	NULL		/* statistics structure - optional */
	};

struct qinit winit={	/* write QUEUE */
	lftwput,	/* write side put procedure */
	lftwsrv,	/* write side service procedure */
	lftopen,	/* called on each open or a push */
	lftclose,	/* called on each close or a pop */
	NULL,		/* future use */
	&wminfo,	/* information structure */
	NULL		/* statistics structure - optional */
	};

struct streamtab lftinfo={
	&rinit,		/* read QUEUE */
	&winit,		/* write QUEUE */
	NULL,		/* for multiplexing driver only */
	NULL		/* for multiplexing driver only */
	};
#endif

/* LFT Driver Data Structures */
/* --------------------------------------------------------------------
   Data local to the LFT driver is stored in the strlft of the structure
   ttylft. The ttylft has the following format;
   ----------------------------------------------------------------- */
/*-------------------------------------------------------------------
  incoming keystroke (from the keyboard) and its mapping
  -------------------------------------------------------------------*/
#define MAX_KEY_SEQ_LENGTH        32
struct key_struc {
    lft_keystroke_t keystroke;                 /* defined in lft_swkbd.h */
    ushort key_seq_length;                      /* length in bytes */
    uchar key_buffer[MAX_KEY_SEQ_LENGTH];       /* string key maps to */
    ushort key_buff_cnt;                        /* number of bytes in ascii_buffer*/
    uchar ascii_buffer[LFTMSGSIZE];		/* final output buffer */
};

/* ************
 * STRUCTURE UP
 * ************
 */

/*
  special keyboard map
  */
#define GREEK_MAP	"el_GR"

/* input_flag must be protected by rising the priority */
#define INT_LFT INTOFFL2

#define STOP_INPUT 	0x0001		/* stop input received */
#define OFFL_INUSE	0x0002		/* lftKiOffl in use */
#define TO_BE_FLUSH	0x0004		/* Input ring must be flushed */
#define UPQ_OVFL	0x0008		/* upper queue overflow */

/* ds_state_flag */
#define INIT_STATE          0x00        /* initial values. */
#define KBD_DEAD_STATE      0x08        /* got diacritic, looking for 2nd */

/* state_flag */
#define LATIN_LEVEL         0x0008      /* for switching between latin and */
					/* greek layers on greek kbd       */
#define ALPHANUM_STATE      0x0004
#define KATAKANA_STATE      0x0002
#define HIRAGANA_STATE      0x0001

#define IR_NB_EVTS	40		/* number of events in the input ring */
#define IR_WM_EVTS	IR_NB_EVTS/2	/* water mark in the input ring */
	
struct up_stream
{
	int input_flag;			/* input state */
	struct intr offl_hdl;		/* structure to schedule lftKiOffl */
	int msg_alloc;			/* msg is getting allocated (bufcall id)*/
	/* reset by lft_buf_alloc */
	int msg_sak;			/* the same as prvious for SIGSAK */
	mblk_t *next_buf;		/* next allocated buffer  */
	mblk_t *in_process;		/* buffer of current input processed by lftst */
	
	uchar ds_state_flag;           	/* data stream state flag */
	ushort state_flag;
	caddr_t 	last_evt;	/* address of the las evt of the ir */
	caddr_t 	first_evt;	/* address of the las evt of the ir */
	struct inputring *l_ip;		/* KBD driver input ring info. */
	struct key_struc key_struc;	/* incoming keystroke from keyboard */
	lft_keystroke_t saved_dead_key;	/* store diacritic.  Copy back into
                                      	   key_struc if composition fails */
	
	char *ichrstr;			/* The actual string key assignments.*/
	ulong ichrstr_len;              /* number of bytes in ichrstr[] */
	
};

#define REMAINING_EVTS	(LFT_UP.l_ip->ir_tail!=LFT_UP.l_ip->ir_head)

/* **************
 * STRUCTURE DOWN
 * **************
 */

/* ds_mode_flag */
#define _IRM    0x4000   /* insert/replace  mode           default = 0 */
#define _TSM    0x2000   /* tabulation stop mode           default = 0 */
#define _LNM    0x1000   /* line feed/new line mode        default = 0 */
#define _CNM    0x0800   /* carriage return/new line mode  default = 0 */
#define _AUTONL 0x0400   /* wrap to next line when ovflk   default = 1 */
#define SCS_DETECTED 0x0008   /* scs escape sequence in effect default = 0 */
#define DEFAULT_DS_MODES        _AUTONL
	
/* Parser States:  reflected in current value of down->parser_state. */
#define NORML            0
#define ESCAPED          1
#define ESCLBR           2
#define ESCLBRX          3
#define ESCLBRXLEN       4
#define ESCLPAREN        5

#define MAXPARM         10

/* default bit maps for tab stops */
#define DEF_TAB_RACK1           0x80808080      /* 1st char, every 8th */
#define DEF_TAB_RACK2           0x80808080      /* every 8th */
#define DEF_TAB_RACK3           0x80810000      /* every 8th, last char */

struct down_stream
{
	int stop_output;		/* 1 stop the output 0 start */
					/* M_STOP M_START messages */
	int ds_mode_flag;
	lft_disp_query_t sv_dsp_info;	/*used to save info between M_IOCTL &M_IOCDATA */
	ulong parser_state;		/* parser state vtmout() */
	unsigned long numparm;          /* # of parameters in esc/ctl seq */
	long parm[MAXPARM];             /* parameters */
	ulong collected_bytes;          /* seen so far */
	char parse_buf[MAXPARM];        /* collecting parameter numbers */
	struct line_data {              /* per line data */
		ulong tab_settings[3];  /* scr_width=80 => 3 long can contain */
					/* bit map of where tabs are */
		ushort line_length;     /* # chars on line so far */
	}line_data[SCR_HEIGHT+1];
};

/* lft_mode_flag */
#define LFJKANA         0x01            	/* enable kana input */
#define OPENEXCL        0x02            	/* for exclusive open */
#define LFWRAP          0x40            	/* wrap cursor  initial value 1 */
#define INIT_MOD_FLAG   0x40            	/* initial value wrap */
 

struct strlft
{
	queue_t *lft_rdqp;			/* pointer to the lft read queue */
	void(**lft_ksvtbl)();			/* service vector table */
	struct termios lft_tios;		/* For compatibility with tty*/

   	ushort lft_mode_flag;			/* used to store katakana state VTD */
	struct up_stream lft_up;		/* structure for all scancode info */
	struct down_stream lft_down;		/* structure for all terminal emulator info */

};

extern lft_ptr_t lft_ptr;

#define LFT_UP	lft_ptr->strlft->lft_up
#define LFT_DOWN	lft_ptr->strlft->lft_down

#define DEFLT_DISP_IDX	lft_ptr->dds_ptr->default_disp_index

#define NO_DEFLT_DISP	\
(lft_ptr->dds_ptr->displays[DEFLT_DISP_IDX].fp_valid == FALSE)

#define SUCCES	0

#endif _L_LFTSI
