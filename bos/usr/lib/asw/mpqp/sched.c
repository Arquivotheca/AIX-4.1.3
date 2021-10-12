static char sccsid[] = "@(#)37	1.4  src/bos/usr/lib/asw/mpqp/sched.c, ucodmpqp, bos411, 9428A410j 5/3/93 16:30:39";

/*--------------------------------------------------------------------------
*
*				  SCHED.C
*
*  COMPONENT_NAME:  (UCODEMPQ) Multiprotocol Quad Port Adapter Software.
*
*  FUNCTIONS: scheduler, init_sched	
*
*  ORIGINS: 27
*
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1989
*  All Rights Reserved
*
*  US Government Users Restricted Rights - Use, duplication or
*  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*
*--------------------------------------------------------------------------
*/

# include "mpqp.def"		/* Definitions */
# include "mpqp.typ"		/* Structure and Type definitions */
# include "mpqp.pro"		/* Function prototype declarations */
# include "mpqp.ext"		/* External Data definitions */
# include "intzero.h"		/* External Data definitions */

extern int	i_sched;	/* scheduler's current task level */


/*-------------------------  S C H E D U L E R  ------------------------*/
/*									*/
/*  FUNCTION:								*/
/*	Non-preemptable scheduler.  Scans each task level from 		*/
/*	highest priority to lowest priority and executes each task	*/
/*	level in round-robin fashion.  The lowest priority is set by 	*/
/*	the limit argument.						*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	This IS the execution environment.				*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	References scheduler data structures.				*/
/*  									*/
/*  RETURNS:    							*/
/*	Nothing								*/
/*									*/
/*----------------------------------------------------------------------*/

scheduler ( 
int	limit )
{
	register int	work;
	register int	backoff=0;

	/* run through the levels looking for work */
	for ( i_sched = 0; i_sched <= limit; ++i_sched)
	{
		work = (work_q[ i_sched ] & 0x0F); 
		while(work)
		{
			/* If work is scheduled for this port, call 	*/
			/* the routine assigned to this task level and	*/
			/* pass it the port number:			*/

			/* does this port have work to do? */
			if ( work & task_mask[ i_sched ] )
			{
				/* call work function */
				GateUser( lev_vec[ i_sched ], 
				    port( task_mask[ i_sched ] ));

				/* what's the next new top level
				 *
				 * if there is no dma running try
				 * the top level (dma level) otherwise
				 * try the level after dma
				 */
				work=FALSE;
		        	task_mask[ i_sched ] 
					= NEXT_PORT( task_mask[ i_sched ] );
				if (++backoff < 100 )
					i_sched = 0;
				else
				{
					i_sched = -1;
					backoff=0;
				}

			} else
		        	task_mask[ i_sched ] 
					= NEXT_PORT( task_mask[ i_sched ] );
		}
	}
}

/*-----------------------  I N I T _ S C H E D  ------------------------*/
/*									*/
/*  FUNCTION:								*/
/*	Initializes scheduler data structures, then drives the		*/
/*	scheduler in an infinite loop.					*/
/*									*/
/*  EXECUTION ENVIRONMENT:						*/
/*	This IS the execution environment.				*/
/*									*/
/*  DATA STRUCTURES:							*/
/*	References scheduler data structures.				*/
/*  									*/
/*  RETURNS: Never returns  						*/
/*									*/
/*----------------------------------------------------------------------*/

init_sched ()
{
	/* clear work bits for all levels and start port work with zero */
	for ( i_sched = 0; i_sched < NUM_LEVEL; i_sched++ )
	{				
		work_q[ i_sched ]    = 0;
		task_mask[ i_sched ] = 1;
	}					

	/* init to highest level */
	i_sched = 0;
	
	/* work, work, work ... for all levels */
	while(TRUE)
		scheduler( NUM_LEVEL - 1 );
}
