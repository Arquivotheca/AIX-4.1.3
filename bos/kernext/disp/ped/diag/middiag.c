static char sccsid[] = "@(#)25  1.12.1.5  src/bos/kernext/disp/ped/diag/middiag.c, peddd, bos411, 9428A410j 12/2/93 08:45:09";
/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: MID_DEBUG_AND_TRACE
 *	
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/syspest.h>
#include <sys/types.h>
#include "mid_dd_trace.h"



#define MID_DEBUG_AND_TRACE(module, debug, trace_parms, perf_trc)  	       \
	BUGVDEF(dbg_ ## module, debug) 					       \
	BUGVDEF(trc_ ## module, trace_parms)				       \
	BUGVDEF(perf_ ## module, perf_trc)				        





/*
 *  Define debug control variables for the functions.
 */

#define BUGVPD BUGNFO


/* ------------ ------------ ------------ ------------ ------------ ------------

    A few helpful hints:

	This routine defines external variables which control debug output 
	(both printfs and system traces).  The MID_DEBUG_AND_TRACE macro
	(defined above) invokes BUGVDEF to perform the actual external
	declarations.  This macro is invoked for each module/function 
	which provides debug output. 


	The parameters for MID_DEBUG_AND_TRACE are: 

	MID_DEBUG_AND_TRACE (module, printf, trace, performance_trace) ;
	  where:  

	    . module = module name

	    . printf = value assigned to dbg_"module" which controls the
	                number of printfs generated 

	    . trace  = value assigned to trc_"module" which controls the
	               number of system traces generated.  Generally, a 
	                a value of 2 or more is required to turn on the
	                device drivers traces because of the implementation
	                of the trace interface macro (MID_DD_TRACE_PARMS).  

	    . performance_trace = value assigned to perf_"module" which controls
	                whether performance traces are turned on or not. 
	                Any non-zero value turns on the performance traces
	                for that module/function.  Note that most of the 
	                special data traces do not have equivalent performance 
	                traces.


 * ------------ ------------ ------------ ------------ ------------ ----------*/


MID_DEBUG_AND_TRACE (midddf,        0, 0, 0) ;
MID_DEBUG_AND_TRACE (midswap,       0, 1, 0) ;
MID_DEBUG_AND_TRACE (midgetcolor,   0, 2, 0) ;
MID_DEBUG_AND_TRACE (midgetcpos,    0, 2, 0) ;
MID_DEBUG_AND_TRACE (midgetcondition, 0, 2, 0) ;
MID_DEBUG_AND_TRACE (midgetfontindex, 0, 2, 0) ;
MID_DEBUG_AND_TRACE (midendrender,  0, 2, 0) ;
MID_DEBUG_AND_TRACE (midgetfreestr, 0, 2, 0) ;
MID_DEBUG_AND_TRACE (middma,        0, 2, 0) ;
MID_DEBUG_AND_TRACE (midevt,        0, 2, 0) ;
MID_DEBUG_AND_TRACE (middd,         0, 0, 0) ;

/*------------------------------------------------------------------------------
	Interrupt handling modules
 -----------------------------------------------------------------------------*/
MID_DEBUG_AND_TRACE (midddi,        0, 0, 0) ;		/* remove someday? */
MID_DEBUG_AND_TRACE (midintr,		   0, 2, 0) ;
MID_DEBUG_AND_TRACE (intr_dsp,		   0, 2, 0) ;
MID_DEBUG_AND_TRACE (intr_fifo,		   0, 2, 0) ;
MID_DEBUG_AND_TRACE (water_level,	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (hi_water_com,	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (lo_water_com,	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (intr_switch_done,	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (dsp_pick,		   0, 2, 0) ;
MID_DEBUG_AND_TRACE (dsp_newctx,	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (dsp_pinctx,	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (dsp_swap,  	   0, 1, 0) ;
MID_DEBUG_AND_TRACE (dsp_stall,  	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (dsp_getcolor,  	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (dsp_getcpos,  	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (dsp_getcondition, 	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (dsp_getfontindex, 	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (dsp_endrender, 	   0, 2, 0) ;


/*------------------------------------------------------------------------------
	KSR modules
 -----------------------------------------------------------------------------*/

MID_DEBUG_AND_TRACE (midcfgvt, 	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (midcrsr, 	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (midcfl, 	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (midtext, 	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (midinit, 	   0, 2, 0) ;
MID_DEBUG_AND_TRACE (midscr, 	   0, 2, 0) ;


/*------------------------------------------------------------------------------
	RCM modules
 -----------------------------------------------------------------------------*/
MID_DEBUG_AND_TRACE (midgp,         0, 2, 0) ;
MID_DEBUG_AND_TRACE (midbind,       0, 2, 0) ;
MID_DEBUG_AND_TRACE (middev,        0, 2, 0) ;
MID_DEBUG_AND_TRACE (midcontext,    0, 2, 0) ;
MID_DEBUG_AND_TRACE (midgeom,       0, 2, 0) ;
MID_DEBUG_AND_TRACE (midwatt,       0, 2, 0) ;
MID_DEBUG_AND_TRACE (midswitch,     0, 4, 0) ;
MID_DEBUG_AND_TRACE (midparts,      0, 0, 0) ;
MID_DEBUG_AND_TRACE (midwid,   	    0, 0, 0) ;  
MID_DEBUG_AND_TRACE (midpixi,      00,02, 0) ;
MID_DEBUG_AND_TRACE (midexch,       0, 0, 0) ;
MID_DEBUG_AND_TRACE (mid_ctxdma,    0, 0, 0) ;

/*------------------------------------------------------------------------------
	RCM Window ID modules
 -----------------------------------------------------------------------------*/
MID_DEBUG_AND_TRACE (get_WID,  	   	   0, 2, 0);
MID_DEBUG_AND_TRACE (get_WID_for_PI,	   0, 2, 0);
MID_DEBUG_AND_TRACE (get_unused_WID,	   0, 0, 0);
MID_DEBUG_AND_TRACE (delete_WID,	   0, 0, 0);
MID_DEBUG_AND_TRACE (steal_WID,		   0, 0, 0);
MID_DEBUG_AND_TRACE (really_steal_WID,	   0, 0, 0);
MID_DEBUG_AND_TRACE(release_guarded_WID,   0, 2, 0);




/*------------------------------------------------------------------------------
	Miscellaneous modules
 -----------------------------------------------------------------------------*/

MID_DEBUG_AND_TRACE (miducod,       0, 0, 0) ;
MID_DEBUG_AND_TRACE (trace_clip,    0, 0, 0) ;  /* is this used ??? */
MID_DEBUG_AND_TRACE (dd_font,       0, 0, 0) ;
MID_DEBUG_AND_TRACE (memtrace,     0, 0, 0) ;


/*******************************************************************************
 *******************************************************************************
	Special data area debug controls 
 *******************************************************************************
 ******************************************************************************/

/*------------------------------------------------------------------------------
	WID list Debug Controls 
 -----------------------------------------------------------------------------*/
MID_DEBUG_AND_TRACE (get_WID_WID_LIST, 		0,  0, 0);
MID_DEBUG_AND_TRACE (get_WID_for_PI_WID_LIST, 	0,  0, 0);
MID_DEBUG_AND_TRACE (get_unused_WID_WID_LIST, 	0,  0, 0);
MID_DEBUG_AND_TRACE (steal_WID_WID_LIST, 	0,  0, 0);
MID_DEBUG_AND_TRACE (really_steal_WID_WID_LIST,	0,  0, 0);
MID_DEBUG_AND_TRACE (delete_WID_WID_LIST, 	0,  0, 0);
MID_DEBUG_AND_TRACE(release_guarded_WID_WID_LIST,0,  0, 0);

MID_DEBUG_AND_TRACE (WID_list, 	       		0,  0, 0);  	/* generic */ 


/*------------------------------------------------------------------------------
	Structure Element and Command Element Debug (mainly from midpixi.c)
 -----------------------------------------------------------------------------*/
MID_DEBUG_AND_TRACE (SWP_FIFO, 	       0, 2, 0);
MID_DEBUG_AND_TRACE (SWP_PCB , 	       0, 2, 0);
MID_DEBUG_AND_TRACE (WID_planes_FIFO,  0, 2, 0);
MID_DEBUG_AND_TRACE (WID_planes_PCB ,  0, 2, 0);
MID_DEBUG_AND_TRACE (Associate_Color,  0, 2, 0);
MID_DEBUG_AND_TRACE (Set_Context,      0, 2, 0); 	/* midswitch.c */
MID_DEBUG_AND_TRACE (syncwid,          0, 2, 0); 	/* midswitch.c */

/*------------------------------------------------------------------------------
	Window Region Debug Control 
 -----------------------------------------------------------------------------*/
MID_DEBUG_AND_TRACE (Update_WG_Region, 0, 0, 0);
MID_DEBUG_AND_TRACE (midwatt_Region,   0, 0, 0);
MID_DEBUG_AND_TRACE (Bind_Region,      0, 0, 0);
MID_DEBUG_AND_TRACE (SWP_Region,       0, 0, 0);
MID_DEBUG_AND_TRACE (WID_planes_Region,0, 0, 0);
MID_DEBUG_AND_TRACE (drain_FIFO_Region,0, 0, 0);

/*------------------------------------------------------------------------------
	Window Geometry Debug Control 
 -----------------------------------------------------------------------------*/
MID_DEBUG_AND_TRACE (Update_WG_WG,     0, 0, 0);
MID_DEBUG_AND_TRACE (Bind_WG,          0, 0, 0);
MID_DEBUG_AND_TRACE (WID_planes_WG,    0, 0, 0);
MID_DEBUG_AND_TRACE (drain_FIFO_WG,    0, 0, 0);

MID_DEBUG_AND_TRACE (WG, 	       0, 0, 0);	/* generic */
MID_DEBUG_AND_TRACE (midWG, 	       0, 0, 0);	/* generic */

