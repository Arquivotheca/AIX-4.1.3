static char sccsid[] = "@(#)43	1.3  src/bos/usr/lib/asw/mpqp/timeutil.c, ucodmpqp, bos411, 9428A410j 11/30/90 11:19:50";

/*
 * COMPONENT_NAME: (UCODMPQ) Multiprotocol Quad Port Adapter Software
 *
 * FUNCTIONS: See description below.
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

#include "mpqp.def"
#include "mpqp.typ"
#include "mpqp.ext"
#include "mpqp.pro"
#include "portcb.def"
#include "portcb.typ"
#include "iface.def"

extern unsigned int	ptimer0[NUM_PORT];	/* Each port's timer count */
extern unsigned char	t_port0;		/* Currently running timer */
extern unsigned int	ptimer1[NUM_PORT];	/* Each port's timer count */
extern unsigned char	t_port1;		/* Currently running timer */
extern unsigned short	tscc_val[NUM_PORT];	/* port specific Duscc timer */
extern unsigned short	tscc_typ[NUM_PORT];	/* port specific timer stuff */

extern unsigned int	StopTimer();
extern void		StartTimer( unsigned short, unsigned int );


/*------------------------------------------------------------------------*/
/*  DUSCC Timers:				                          */

start_duscc_timer ( 
	volatile t_pcb	*p_pcb,
	unsigned short	time, 
	unsigned char 	type )
{
	FOOTPRINT( 't', 's', 'r' );		/* tsr */
	
	/*  First set the preset timer registers to a value that will	*/
	/*  count to zero in 1/10 of a second.  The DUSCC clock is	*/
	/*  at 14.7456 MHz; since CTCR is set up for division of this 	*/
	/*  clock by 256 ( divide by 4, prescale by 64 ), one tenth of	*/
	/*  a second is (14,745,600 / ( 256 * 10 )) = 5760 ticks:	*/

	SET_CTPH( HI_BYTE( 5760 ));
	SET_CTPL( LO_BYTE( 5760 ));

	/*  Select Zero detect interrupt, Zero detect preset, prescale	*/
	/*  by 64, and select X1/CLK divided by 4.			*/

	SET_CTC( CTC_EZDI | CTC_PRE_64 | CTC_CS_XDIV4 );

	/*  Issue command to preset counter/timer registers, then save	*/
	/*  the timer value (in 100's of milliseconds) and the type.	*/

	WRITE_CCR( CC_PRESET_PR_CT );
	tscc_val[ p_pcb->port_no ] = time;
	tscc_typ[ p_pcb->port_no ] = type;
	p_pcb->timer_type = type;

	/*  Issue command to start counter/timer hardware:		*/

	WRITE_CCR( CC_START_CT );
	return;
}

stop_duscc_timer (
	volatile t_pcb	*p_pcb )
{
	FOOTPRINT( 't', 'r', 'r' );		/* trr */

	/*  Issue command to stop counter/timer hardware, then clear	*/
	/*  timer values and types.					*/

	WRITE_CCR( CC_STOP_CT );
	tscc_val[p_pcb->port_no] = 0;
	tscc_typ[p_pcb->port_no] = NULL;
	p_pcb->timer_type        = NULL;
	return;
}

/*------------------------------------------------------------------------*/
/*  CPU Timers:						                  */

/*
   set_timer:  Start a timer for a port.  Only one may run at once for any
		port, as reissuing the set_timer for a port with a running
		timer just changes the timer value, either higher or lower,
		but regardless to the new value.
   entry:      port_no - The port number, taken from the PCB.  Values from
		0 to NUM_PORT - 1 are allowed, but no checking is performed.
	       t_value - The timer duration in 100ms (0.1S) increments.
		Values from 1 to 65534 are allowed.
   exit:       Returns nothing of significance.
*/

void settime( unsigned char		port_no,
		unsigned int		t_value,
		unsigned		timer )
{
register unsigned	*p_t;
unsigned char		*p_v;
register unsigned	port_id;
unsigned int		delta;
unsigned int		max = 0xFFFF;

	if ( timer == 0 )
	{
		p_t = &ptimer0[0];
		p_v = &t_port0;
	}
	else
	{
		p_t = &ptimer1[0];
		p_v = &t_port1;
	}
	t_value *= 10;
	delta = *( p_t + *p_v ) - StopTimer(timer);	/* Set delta */
	*( p_t + port_no ) = t_value + delta;
	for ( port_id = 0; port_id < NUM_PORT; ++p_t, ++port_id )
	{
		if ( *p_t != 0xFFFF )
		{
			if ( !( *p_t -= delta ) )
			{
				*p_t = 0xFFFF;
				continue;
			}
			if ( *p_t < max )
			{
				*p_v = port_id;
				max = *p_t;
			}
		}
	}
	if ( max != 0xFFFF )
		StartTimer( timer, max );
}

void set_timer( unsigned char	port_no,
		unsigned int	t_value )
{
	settime ( port_no, t_value, 0 );
	return;
}

void set_failsafe( unsigned char	port_no,
		   unsigned int		t_value )
{
	settime ( port_no, t_value, 1 );
	return;
}


/*
   The actual CPU Timer 0/1 ISRs call this module to manipulate the timer
   tables and create the necessary response queue element(s).
   WARNING!  This runs at INTERRUPT PRIORITY.  Don't do anything time
   consuming.
   Generate an e_block and schedule ERRSTS.  This gets the RQE created
   in off-level instead of here, at interrupt level.  Schedule it.
   Regular event timers (TIMER_P) and failsafe timers (FAILSAFE) both
   come here to start the applicable offlevels and multiplex the timers.

	060489b : Handle multiple expirations in one interrupt
	080989b : Made max (unsigned), added *p_t = 0xFFFF for expired timer
*/

void timer_exp( unsigned int		delta,
		unsigned short		mask,
		register unsigned	*p_t,
		unsigned char		*p_v )
{
register unsigned	temp;
register unsigned	port_id;
unsigned short		timer;
unsigned unsigned	max = 0xFFFF;

	timer = ( p_t == &ptimer0[0] ) ? 0 : 1;
/*
	Get the next lowest timer value we're handling and start it.
*/
	for ( port_id = 0; port_id < NUM_PORT; ++p_t, port_id++ )
		if ( *p_t != 0xFFFF )
		{
			temp = ( *p_t -= delta );
			if ( !temp )
			{
				*p_t = 0xFFFF;
				estat [port_id].e_type |= mask;
				work_q [ERR_STS] |= portbit( port_id );
			}
			else if ( *p_t < max )
			{
				*p_v = port_id;
				max = *p_t;
			}
		}
	if ( max != 0xFFFF )
		StartTimer( timer, max );
}
