static char sccsid[] = "@(#)44	1.3  src/bos/usr/lib/asw/mpqp/tracefn.c, ucodmpqp, bos411, 9428A410j 11/30/90 11:20:37";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: See description below
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
/   NAME: traceadd
/
/   FUNCTION:     Traces command and response queue elements to and from the
/                 adapter.  Ioctl's are used to retrieve and format the data
/                 kept out on the adapter.
/
/   PARAMETERS:   port_num - port being traced
/                 type     - type of trace entry
/                 q_entry  - data to be added to trace
/                 bufno    - bufno number associated with this trace element
/
*/

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"

/* constant declarations */
#define		NUM_PORT	4	/* number of ports/adapter  */
#define		MAX_P_ELE	63	/* max num of port trace elemts */
#define		MAX_A_ELE	254	/* max num of adapt trace elemnts */

/* type defs of queues structures and elements */

#define	t_trace_ele	unsigned long

typedef struct			/* struct of port queues */
{
	unsigned char	last;
	unsigned char	lgt;
	unsigned short	rsvd1;
	t_trace_ele 	trc_ele[MAX_P_ELE];
} t_port_trace;

typedef struct			/* struct of adapter trace */
{
	unsigned char	last;
	unsigned char	lgt;
	unsigned char	pt_num[MAX_A_ELE]; 
} t_adapt_trace;

/* external declarations */

extern volatile unsigned char	TraceOn[NUM_PORT];

extern volatile t_adapt_trace	trc_ad;
extern volatile t_port_trace	trc_p[NUM_PORT];
extern volatile t_port_trace	trc_0,trc_1,trc_2,trc_3;

/* start of trace code */

/*
   Enable tracing for a port.  This "rewinds" the port trace by setting the
   length to zero.  Function 051789b
*/

void    start_trace( cmd_blk               *p_cmd,
		     unsigned char far     *p_buf )
{
register int		port = p_cmd->_port;
register t_port_trace	*p_trc;

	if ( !( TraceOn[0] || TraceOn[1] || TraceOn[2] || TraceOn[3] ) )
		trc_ad.last = trc_ad.lgt = 0;

	TraceOn[port] = 0xFF;
	p_trc = &trc_p [port];
	p_trc->last = 0xFF;
	p_trc->lgt  = 0;

	return;
}

/*
   Disable tracing for a port.  This has no effect on the pointers in the
   trace queue.  Function 051789b
*/

void    stop_trace( cmd_blk               *p_cmd,
		    unsigned char far     *p_buf )
{
register int		port = p_cmd->_port;

	TraceOn[port] = 0x00;
	return;
}

/*----------------------------------------------------------------------*/
/*   Character trace -- starts at 0x1F800 in adapter memory; last 	*/
/*   entry ends with asterisk '---':					*/

# define	TRCSTART	0xF800
# define	TRCSIZE		1800

char 	*trcptr;

char_trace (
	char	c1,
	char	c2,
	char	c3 )
{
	disable();
	if ((trcptr < (char *)TRCSTART) || 
	    (trcptr >= (char *)(TRCSTART + TRCSIZE)))
	{
		trcptr = (char *)TRCSTART;
	}
	*trcptr++ = c1;
	*trcptr++ = c2;
	*trcptr++ = c3;
	*trcptr++ = ' ';
	*trcptr++ = '-';
	*trcptr++ = '-';
	*trcptr++ = '-';
	*trcptr   = ' ';
	trcptr--; trcptr--; trcptr--;
	if (trcptr >= (char *)(TRCSTART + TRCSIZE))
	{
	    *trcptr++ = ' ';
	    *trcptr++ = ' ';
	    *trcptr++ = ' ';
	    *trcptr++ = ' ';
	    trcptr = (char *)TRCSTART;
	} 
	enable();
}
