/* @(#)58    1.2  src/bos/kernext/disp/trace/gs_trace.h, sysxdisp, bos411, 9428A410j  5/13/93  10:43:30 */

#ifndef _H_GS_TRACE
#define _H_GS_TRACE

#include <sys/trcmacros.h>

/*
 * COMPONENT_NAME: (sysxdisp) Graphics Systems Trace 
 *
 * FUNCTIONS (macros):	GS_TRC_GLB, GS_TRC_MODULE, GS_MODULE, GS_FUNCTION
 *		GS_ENTER_TRC0, GS_ENTER_TRC1, GS_ENTER_TRC,
 *		GS_PARM_TRC0,  GS_PARM_TRC1,  GS_PARM_TRC,
 *		GS_EXIT_TRC0,  GS_EXIT_TRC1,  GS_EXIT_TRC,
 *		GS_ENTER_DBG0, GS_ENTER_DBG1, GS_ENTER_DBG,
 *		GS_PARM_DBG0,  GS_PARM_DBG1,  GS_PARM_DBG,
 *		GS_EXIT_DBG0,  GS_EXIT_DBG1,  GS_EXIT_DBG
 *		GS_DATA_DBG 
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/************************************************************************
 ************************************************************************
 ************************************************************************
 									 
  OVERVIEW of GRAPHICS SYSTEM SYSTEM TRACE STRATEGY 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
  
  System traces are used for debug in several environments: every test from
  unit test through CET and ship test, including performance testing as well.
  After release, system trace will still be used for isolation and debug of 
  customer problems.

  To satisfy these broad requirements, Graphics Systems employs two types
  of traces: debug traces and ship traces.  These two types of traces are
  provided by two different (but similar) sets of macros.  The names of 
  these macros should make it obvious whether the trace is debug only trace
  or shipped for the customer.

  Ship traces are only those trace required to provide a bare minumum amount
  of information for isolation and debug.  These must be carefully chosen.
  One good place for ship-level traces are in places where each new piece of
  work for a component is received:  the Xserver dispatch loop, an interrupt
  handler, the RCM's aixgsc interface. Probably, no more than one (maybe two)
  traces are desired for each piece of work.  It is better to err on the side
  of too few ship level traces -- it is important to not flood the trace buffer
  with graphics information.

  Another good place for ship traces is error prone code.  If we find a
  section of code in FVT or PVT test that tend to be error, a ship trace
  may be prudent, (but only if useful information can be obtained).

  Debug trace provide a lot of information -- generally too much.  So each 
  trace invocation is assigned a priority.  A data area is used to turn 
  traces on and off to control the amount of data being traced.  This allows 
  the user to throttle the amount trace data and presumably capture the failure
  with a minumum of noise.  These mechanisms will be described more later.
  
  As a rule of thumb, every function has an entry trace and an exit trace.
  Another general guideline:  all debug traces provide timestamping to 
  facilitate performance evaluation.




  TRACE RELATED COMPILE VARIABLES
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  Several compile variables are used to control the GS system trace function.
  Many of these variables MUST not be left in ship code.  This can be
  accomplished by simply removing them from the Makefile, however, a more
  elegant solution is to replace the -D with a -U.

  -DGS_DEBUG_TRACE -- Enables debug trace hooks to be compiled in.  This 
			compile variable should ONLY be used for development 
			builds only.  It should never be used in a production 
			level build as it causes significant performance 
			degradation.
 		** DO NOT LEAVE IN FOR SHIP LEVEL CODE !!! **		       
  									       
  -DGS_DEBUG -- With respect to system trace, this variable has the same 
                        action and restrictions as GS_DEBUG_TRACE.  It is
			expected that this variable will be used to turn on
			other debug facilities besides trace.  Hence, it acts 
			as a superset of GS_DEBUG_TRACE.
 		** DO NOT LEAVE IN FOR SHIP LEVEL CODE !!! **		       

  -DGS_REMOVE_TRACE -- Removes all system trace hooks, even the ship-level 
			traces.  This may be desirable when traces are 
			suspected of impacting performance or problem symptoms.
			This variable supercedes GS_DEBUG_TRACE (and GS_DEBUG).
 		** DO NOT LEAVE IN FOR SHIP LEVEL CODE !!! **		       
  									       
  -DGS_USE_SYS_TRACE_TIMESTAMP -- Adds the timestamp to all ship-level trace 
			statements.  Since it was unknown at design time
			whether this was a performance concern or not, it
			was decided that timestamping in production code
			should be under compilation variable control.
  									       
  -DGS_REMOVE_TRACE_RANGE_CHECK -- Removes the range check (of the routine ID 
			and the subfunction ID) in debug traces.  Has no action
			for ship-level traces.  Usually has no action for code
			that is compiled optimized either.  The optimizer 
			removes these checks unless the routine or sub-function
			ID is incorrect.  So there is little reason to use this
			flag, but I don't see any reason to remove it, now that
			I've put it in.
  									       



  THE GS SYSTEM SYSTEM TRACE HOOKWORD 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  The following is the format of the trace hookword:			 
 								  	 
  0                      11 12    15 16        21 22                 31  
  +------------------------+--------+------------+--------------------+  
  |    Major Hook ID       | flags  | GS_subfunc |     GS_routine     |  
  +------------------------+--------+------------+--------------------+  
 								  	 
  The first two fields in the hookward (Major hook ID and flags) are global.  
  They have a common definition for all system traces used in AIX.  The low 
  order sixteen bits of the trace hookword have a common definition within
  the Graphics Systems products (eg. AIXWindows).  (These sixteen bits are
  user definable from an AIX system perspective.)

  Within GS, the Major Hook IDs are defined roughly on a component basis.
  That is, each component is assign a unique Major Hook ID.  This makes it
  very easy to turn system traces on for a component (or set of components).
  The Major Hook ID assignments are contained in gshkid.h.  		     */

#include <graphics/gshkid.h>

/* As can be seen above, the low order sixteen bits are split into two 
   fields (for GS system traces).  The combination of these two fields is 
   referred to as the minor hook ID.  
  
    - GS_routine (bits 22 through 31) are a routine ID.  This allows 1024
            routines within a component.  In the unlikely event that a 
            component grows larger than 1024 routines, routine ID 1023 
            could be used as an escape mechanism or an additional Major Hook 
            could be defined (we'll worry about when it happens).
 								   
            The routine ID is placed in the low order bits of the hookword, 
            to facilitate reading the hook words (in hex) in a dump or 
            memory trace. 

    - GS_subfunc (bits 16 through 21) are a subfunction identifier for the
            routine.  Generally a routine has multiple trace invocations.  
            This field identifies the traces within a routine or function. 
            Most of the 64 values can be used, however the following
            values are reserved for the following definitions: 		     */
  									 
#define	hkwd_GS_ENTER	0
  	 		/*  0 is defined as the ID of the function entry */
#define	hkwd_GS_EXIT	0x20
  			/* 32 is defined as the ID of the function exit  */
#define	hkwd_GS_ESCAPE	0x3F
  			/* 63 is defined as an escape, indicating the 
			   subfunction IDs (for this routine) have been 
			   exhausted and the first dataword contains the real 
			   subfunction ID. */
#define	hkwd_GS_dummy	0
			/* dummy define for use with data traces */
/*
           Note that the above symbols are also reserved, and should not be
	   redefined by a component's trace file.

           Often no subfunction IDs are required for a function (other than 
	   the predefined entry and exit IDs).
 									 
   Note:  The GS_routine and GS_subfunc are range checked (at runtime) when   
          either GS_DEBUG or GS_DEBUG_TRACE are specified.  The range check   
          is not present in the ship-level traces.  If the ranges have been
          tested, the range check can be removed with the 
          GS_REMOVE_TRACE_RANGE_CHECK compile flag.



  INDIVIDUAL COMPONENT TRACE FILES 
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  It is anticipated that each component will have its own component trace 
  definition file, which will define:
    . the routine IDs,
    . the subfunction IDs,
    . a set of trace macros for that component.  Generally this is done
        to eliminate the specification of the same Major Hook ID for every
        trace invocation.  If there are other common parameters, these can
        can customized in the component trace macros as well.
    . macro layers (and any other component that is included and compiled 
	as part of another component) will provide a header file to define
 	the group of (external) trace control variables (discussed below).
 	This same header file is included in a .c file to define the 
	static versions of these variables.  (discussed a little more below)

  The routine IDs must start with "hkwd_GS_", followed by a short identifier
  (2 to 4 characters) for the component, followed by a string to identify
  the routine (function).  Note that when the routine ID is specified in 
  a trace macro, the "hkwd_GS_" prefix IS NOT included.  The macros defined in
  this module automatically concatenate that prefix.  Note, that this forces
  the symbol to be defined with that prefix.

  NOTA BENE:  For performance reasons, no checking is done to ensure that the 
              user defined fields (GS_routine and GS_subfunc) stay within the 
              defined ranges in ship-level traces.  Please code safely.

  For components that are included and compiled with other components
  (ie. macro layers), the group of (external) trace control variables
  (discussed below)


  MINOR HOOK DEFINITION EXAMPLE
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  For example, in component gc1bbl, we might have a routine called 
  write_something, with a function called foobar, then we might have
  the following definitions: 

   enum gs_gb1_hkwd {
                           .
                           .
                           .
 
                    hkwd_GS_gb1_FOOBAR =            0x145, 
                    hkwd_GS_gb1_FOOBAR_OPEN_ERR =          0x01, 
                    hkwd_GS_gb1_FOOBAR_RECALIBRATION =     0x02,
                    hkwd_GS_gb1_FOOBAR_CLOSE_ERR =         0x03,
                           .
                           .
                           .
 									 
  This defines foobar()'s routine id as 0x145.  It also implies that
  there are five (5) traces within foobar:
   . subfunction ID 0 (entry), 
   . subfunction ID 1 (open error of some sort), 
   . subfunction ID 2 (recalibration tracking), 
   . subfunction ID 3 (close error of some sort) and 
   . subfunction ID 32 (exit).  

  Subfunction ID 63 could also be used (for escape), but there are plenty of 
  spare IDs, so this seems unlikely in this example.


 									 
  CONTROLLING THE AMOUNT OF DATA TRACED
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  There are several factors involved in the determination of whether a
  GS system trace invocation actually produces a trace call or not.

  First, the trace statement must be compiled in (ie. must produce actual 
  code).  This is controlled with compile variables and described above in
  the TRACE RELATED COMPILE VARIABLES section.
 									 
  Assuming a particular debug trace invocation is compiled in, there are three
  variables that determine whether the trace call is made or not.  An if
  statement (involving these variables) precedes each debug trace.  Note 
  that ship traces (in ship mode) are not subject to the if-test (to keep the 
  path length as short as possible). 

  Each GS trace invocation includes a parameter which defines the priority
  of the trace.  It is this parameter that is tested against the three
  trace variables in the if-test mentioned above.  This priority is an 
  unsigned integer, and is really an inverse priority -- with 0 being the 
  highest priority, 1 the next highest priority and so on.  

  Traditionally, 0 is not used.
    1 is used for either function entry or exit -- whichever yields the best
       information,
    2 is used for either function entry or exit (whichever was not specified
       as priority 1),
    3 would be the next level of detail,
        . 
        . 
        . 

  Generally, about 5 to 10 levels of trace are defined for a component,
  however, the developer should feel free to define more or less as 
  appropriate.

  The three trace variables are:
    . a global (per executable) trace level - This variable defines a value,
       such that only traces with a priority number less than or equal are 
       traced.

    . a global (per executable) trace mask - This variable defines a value,
       such that only traces with a priority number greater than or equal 
       are traced.  This trace mask can only be used when the global trace 
       level is 0.

    . a module or function trace level - This variable defines a value
       much like the global trace level, except that it only applies to
       the particular function or module. Only traces with a priority number 
       less than or equal to the function trace level are traced.  Again,
       the function trace level can only be used when the global trace 
       level is 0.

  A .c file is required per exectuable to define all these variables.
  Let's call this the component's trace variable definition file. 
  The GS_TRC_GLB and GS_TRC_MODULE macros are provided to define the data
  space associated with these external variables (that comprise this file). 

  In addition, the GS_MODULE macro is required in each module utilizing the 
  GS trace macros to define the local function trace level variable. 
  Note that this is used only once in a .c file.  If additional functions
  are present the GS_FUNCTION macro should be used.
    
  When a macro layer is included, the macro layer's trace header file defines 
  all the trace variables specific to the macro layer.  This same header file 
  is also included in the components's trace variable definition file to 
  define the space for the macro layer's trace control variables.
  This requires the specification of the GS_DEFINE_MACRO_EXTERNS in the
  trace variable definition (.c) file to notify the macros when to make
  the declarations that reserve space.

******************************************************************************/

#define GS_TRC_GLB(trace_level_mask, trace_level) \
	int	trc_gsm = (trace_level_mask); 	\
	int	trc_gsl = (trace_level)


#define GS_TRC_MODULE(module, trace_level) 	\
	int	trc_ ## module = (trace_level)

#define GS_FUNCTION(module) \
	extern trc_ ## module; 

#define GS_MODULE(module) 		\
	GS_FUNCTION(module) 		\
	extern int	trc_gsm, trc_gsl


/************************************************************************

  Macro layers present special problems.  A header file is used to define a 
  set of trace variables for the macro layer.  This same header file 
  is also included in the "higher-level" components's trace variable 
  definition file to define the space for the macro layer's trace control 
  variables.  When included in this type of file, the trace variable 
  declarations must actually define storage.    

  The net is that the declarations must be made two different ways.  This
  is controlled with the GS_DEFINE_MACRO_EXTERNS compile variable.

  The following declaration macros are used for defining trace symbols 
  in macro layers. 

******************************************************************************/

#ifdef  GS_DEFINE_MACRO_EXTERNS

#define GS_MACRO_FUNCTION(module, trace_level)		\
	GS_TRC_MODULE(module, trace_level) 

#else   /* GS_DEFINE_MACRO_EXTERNS */

#define GS_MACRO_FUNCTION(module, trace_level)		\
        GS_FUNCTION(module) 

#endif  /* GS_DEFINE_MACRO_EXTERNS */





/************************************************************************
 									 
  The following macro is one of the building blocks for the real GS system 
  trace interface macros. 
    
    . GS_TRC_HKWD - This macro forms a hook word, shifting the fields into
		     	the appropriate location within the word.  No masking
		 	is done for performance reasons.

******************************************************************************/
  
#define GS_TRACE_MAJOR_NUM_SHIFT 	20
#define GS_TRACE_SUBFUNC_SHIFT 		10

#define GS_TRC_HKWD(major_hook, routine, subfunc)  			\
	( ((major_hook) << GS_TRACE_MAJOR_NUM_SHIFT) 	|		\
	((hkwd_GS_ ## subfunc) << GS_TRACE_SUBFUNC_SHIFT) |		\
	 (hkwd_GS_ ## routine) )



/************************************************************************
 									 
  Here we translate compile variables into a set of switches.  This is 
  necessary for the functions that are turned on by multiple compile variables: 
    
    . GS_DEBUG_TRACE_SWITCH - turned on by: 	GS_DEBUG_TRACE 
    						GS_DEBUG

******************************************************************************/

#ifndef	GS_REMOVE_TRACE
#ifndef	GS_DEBUG_TRACE_SWITCH

#ifdef	GS_DEBUG_TRACE
#define	GS_DEBUG_TRACE_SWITCH
#endif	/* GS_DEBUG_TRACE */

#ifdef	GS_DEBUG
#define	GS_DEBUG_TRACE_SWITCH
#endif	/* GS_DEBUG */

#endif	/* GS_DEBUG_TRACE_SWITCH */
#endif	/* GS_REMOVE_TRACE */



/************************************************************************
 ************************************************************************
 ************************************************************************
 									 
  Now define the generic GS system trace macros.  These are the building
  blocks for a component's individualized trace macros.
    
  SHIP-LEVEL TRACES:
  ~~~~~~~~~~~~~~~~~~
    . GS_ENTER_TRC  - Ship-level trace, for function entry, with capability to 
			trace up to 5 parameters (one word each).
    . GS_ENTER_TRC1 - Ship-level trace, for function entry, traces 1 parameter.
    . GS_ENTER_TRC0 - Ship-level trace, for function entry, with no data. 


    . GS_EXIT_TRC  -  Ship-level trace, for function exit, with capability to 
			trace up to 5 parameters (one word each).
    . GS_EXIT_TRC1 - Ship-level trace, for function exit, traces 1 parameter.
    . GS_EXIT_TRC0 - Ship-level trace, for function exit, with no data. 


    . GS_PARM_TRC  - Ship-level trace, for use inside function (not entry or 
			exit), with capability to trace up to 5 parameters. 
    . GS_PARM_TRC1 - Ship-level trace, inside function, traces 1 parameter. 
    . GS_PARM_TRC0 - Ship-level trace, inside function, with no data. 


  DEBUG TRACES:
  ~~~~~~~~~~~~
    . GS_ENTER_DBG  - Debug trace, for function entry, with capability to 
			trace up to 5 parameters (one word each).
    . GS_ENTER_DBG1 - Debug trace, for function entry, traces 1 parameter.
    . GS_ENTER_DBG0 - Debug trace, for function entry, with no data. 


    . GS_EXIT_DBG  -  Debug trace, for function exit, with capability to 
			trace up to 5 parameters (one word each).
    . GS_EXIT_DBG1 - Debug trace, for function exit, traces 1 parameter.
    . GS_EXIT_DBG0 - Debug trace, for function exit, with no data. 


    . GS_PARM_DBG  - Debug trace, for use inside function (not entry or 
			exit), with capability to trace up to 5 parameters. 
    . GS_PARM_DBG1 - Debug trace, inside function, traces 1 parameter. 
    . GS_PARM_DBG0 - Debug trace, inside function, with no data. 

    . GS_DATA_DBG - Debug trace, traces a data area for a specified length

 ************************************************************************

  Each of these macros must be defined one of several ways based on how
  the compile variables are set.  The following logic should summarize the 
  various cases (note that this is not the actual logic used, but should be
  equivalent -- proof thereof, left as an exercise for the reader):

    if ( GS_REMOVE_TRACE ) 
      	Define null versions of all trace macros

    elseif (! (GS_DEBUG | GS_DEBUG_TRACE) )
      	Define null versions of the debug trace macros
      	
  	if (GS_USE_SYS_TRACE_TIMESTAMP)
      	    Define timestamp versions of the ship trace macros
  	else 
      	    Define non-timestamp versions of the ship trace macros

    else  (full debug mode)
      	Define ship macros to be equivalent to the debug macros 
        Define timestamp versions of the debug trace macros


 ************************************************************************
 ************************************************************************
******************************************************************************/
  
	


#ifndef GS_DEBUG_TRACE_SWITCH

#define GS_ENTER_DBG0(major_hook, module, priority, routine)
#define GS_ENTER_DBG1(major_hook, module, priority, routine, p1)
#define GS_ENTER_DBG( major_hook, module, priority, routine, p1,p2,p3,p4,p5)

#define GS_EXIT_DBG0(major_hook, module, priority, routine)
#define GS_EXIT_DBG1(major_hook, module, priority, routine, p1)
#define GS_EXIT_DBG( major_hook, module, priority, routine, p1,p2,p3,p4,p5)

#define GS_PARM_DBG0(major_hook, module, priority, routine, subfunc)
#define GS_PARM_DBG1(major_hook, module, priority, routine, subfunc, p1)
#define GS_PARM_DBG( major_hook, module, priority, routine, subfunc, p1, \
							         p2,p3,p4,p5)

#define GS_DATA_DBG(major_hook, module, priority, routine, subfunc, addr, len)

#ifdef GS_REMOVE_TRACE

#define GS_ENTER_TRC0(major_hook, module, priority, routine)
#define GS_ENTER_TRC1(major_hook, module, priority, routine, p1)
#define GS_ENTER_TRC( major_hook, module, priority, routine, p1,p2,p3,p4,p5)

#define GS_EXIT_TRC0(major_hook, module, priority, routine)
#define GS_EXIT_TRC1(major_hook, module, priority, routine, p1)
#define GS_EXIT_TRC( major_hook, module, priority, routine, p1,p2,p3,p4,p5)

#define GS_PARM_TRC0(major_hook, module, priority, routine, subfunc)
#define GS_PARM_TRC1(major_hook, module, priority, routine, subfunc, p1)
#define GS_PARM_TRC( major_hook, module, priority, routine, subfunc, p1, \
							         p2,p3,p4,p5)

#else /* ! GS_REMOVE_TRACE */

#ifdef 	GS_USE_SYS_TRACE_TIMESTAMP

#define GS_TRCHK  	TRCHKT 
#define GS_TRCHKL  	TRCHKLT
#define GS_TRCHKG  	TRCHKGT

#else 	/* USE_SYS_TRACE_TIMESTAMP */

#define GS_TRCHK  	TRCHK  
#define GS_TRCHKL  	TRCHKL
#define GS_TRCHKG  	TRCHKG

#endif 	/* USE_SYS_TRACE_TIMESTAMP */

#define GS_ENTER_TRC0(major, module, priority, routine)			\
	GS_TRCHK  (GS_TRC_HKWD(major, routine, ENTER) )

#define GS_ENTER_TRC1(major, module, priority, routine, p1)		\
	GS_TRCHKL (GS_TRC_HKWD(major, routine, ENTER), p1) 

#define GS_ENTER_TRC( major, module, priority, routine, p1, p2, p3, p4, p5)  \
	GS_TRCHKG (GS_TRC_HKWD(major, routine, ENTER),   	     \
						      p1, p2, p3, p4, p5)



#define GS_PARM_TRC0(major, module, priority, routine, subfunc)		\
	GS_TRCHK (GS_TRC_HKWD(major, routine, subfunc))

#define GS_PARM_TRC1(major, module, priority, routine, subfunc, p1)	\
	GS_TRCHKL (GS_TRC_HKWD(major, routine, subfunc), p1) 

#define GS_PARM_TRC( major, module, priority, routine,subfunc,p1,p2,p3,p4,p5) \
	GS_TRCHKG (GS_TRC_HKWD(major, routine, subfunc),          \
						      p1, p2, p3, p4, p5)



#define GS_EXIT_TRC0(major, module, priority, routine) 			\
	GS_TRCHK (GS_TRC_HKWD(major, routine, EXIT) )

#define GS_EXIT_TRC1(major, module, priority, routine, p1)		\
	GS_TRCHKL (GS_TRC_HKWD(major, routine, EXIT), p1) 

#define GS_EXIT_TRC( major, module, priority, routine, p1, p2, p3, p4, p5)  \
	GS_TRCHKG (GS_TRC_HKWD(major, routine, EXIT),           \
						      p1, p2, p3, p4, p5)

#endif /* ! NODEBUG */


#else /* full DEBUG versions */

#define GS_ENTER_TRC0(major, module, priority, routine) \
	GS_ENTER_DBG0(major, module, priority, routine)
#define GS_ENTER_TRC1(major, module, priority, routine, p1) \
	GS_ENTER_DBG1(major, module, priority, routine, p1)
#define GS_ENTER_TRC( major, module, priority, routine, p1, p2, p3, p4, p5)  \
	GS_ENTER_DBG( major, module, priority, routine, p1, p2, p3, p4, p5)

#define GS_PARM_TRC0(major, module, priority, routine, subfunc) \
	GS_PARM_DBG0(major, module, priority, routine, subfunc)
#define GS_PARM_TRC1(major, module, priority, routine, subfunc, p1) \
	GS_PARM_DBG1(major, module, priority, routine, subfunc, p1)
#define GS_PARM_TRC( major, module, priority, routine, subfunc, p1,p2,p3,p4,p5) \
	GS_PARM_DBG( major, module, priority, routine, subfunc, p1,p2,p3,p4,p5)

#define GS_EXIT_TRC0(major, module, priority, routine) \
	GS_EXIT_DBG0(major, module, priority, routine)
#define GS_EXIT_TRC1(major, module, priority, routine, p1) \
	GS_EXIT_DBG1(major, module, priority, routine, p1)
#define GS_EXIT_TRC( major, module, priority, routine, p1, p2, p3, p4, p5) \
	GS_EXIT_DBG( major, module, priority, routine, p1, p2, p3, p4, p5)


/*****************************************************************************
 									 
  GS_DEATH - macro which causes an abort for user code and a panic for kernel
    
******************************************************************************/
#ifdef 	_KERNEL
#define GS_DEATH(string) 	panic(string)
#else	/* _KERNEL */
#define GS_DEATH(string) 	abort()
#endif	/* _KERNEL */


/*****************************************************************************
 									 
  GS_TRACE_MINOR_CHECK - macro which checks the range of the routine ID and
  		the subfunction ID.  This range checking is present only in 
		the debug (non-ship) version of the traces and can be turned 
		off with the GS_REMOVE_TRACE_RANGE_CHECK compile flag.

		There is no to use this flag when compiling with the optimizer
		turned on -- as the optimizer removes these checks unless the
		IDs are bad!
    
******************************************************************************/

#ifdef	GS_REMOVE_TRACE_RANGE_CHECK 
#define GS_TRACE_MINOR_CHECK(routine, subfunc)
#else	/* GS_REMOVE_TRACE_RANGE_CHECK */

#define GS_TRACE_MINOR_CHECK(routine, subfunc)				\
	if (  ((hkwd_GS_ ## routine) & 0xFFFFFC00) || 			\
	      ((hkwd_GS_ ## subfunc) & 0xFFFFFFC0) ) 			\
        {								\
              GS_DEATH ("routine ID or sub-function ID out of range") ; \
        } 
#endif	/* GS_REMOVE_TRACE_RANGE_CHECK */

/*****************************************************************************
 									 
  GS_TRACE_PRIORITY_TEST - macro which implements the determination of when
  		to make the trace invocation (based on the priority and the
  		3 trace variables). 
    
******************************************************************************/

#define GS_TRACE_PRIORITY_TEST(module, priority)			     \
	if( (trc_gsl >= (priority)) || ((!trc_gsl) && 			     \
	      ((priority) <= trc_gsm) && (trc_ ## module >= (priority))) )


#define GS_ENTER_DBG0(major, module, priority, routine)			     \
{									     \
        GS_TRACE_MINOR_CHECK(routine, ENTER)			     	     \
	GS_TRACE_PRIORITY_TEST(module, priority)     			     \
	{								     \
	    TRCHKT (GS_TRC_HKWD(major, routine, ENTER)) ;  	     	     \
	}								     \
}

#define	GS_ENTER_DBG1(major, module, priority, routine, p1)	             \
{									     \
        GS_TRACE_MINOR_CHECK(routine, ENTER)			     	     \
	GS_TRACE_PRIORITY_TEST(module, priority)     			     \
	{								     \
	    TRCHKLT(GS_TRC_HKWD(major, routine, ENTER),p1);	     	     \
	}								     \
}


#define	GS_ENTER_DBG(major, module, priority, routine, p1, p2, p3, p4, p5)   \
{									     \
        GS_TRACE_MINOR_CHECK(routine, ENTER)			     	     \
	GS_TRACE_PRIORITY_TEST(module, priority)     			     \
	{								     \
	    TRCHKGT(GS_TRC_HKWD(major, routine, ENTER), p1, p2, p3, p4, p5) ;\
	}								     \
}


#define GS_PARM_DBG0(major, module, priority, routine, subfunc)	 	     \
{									     \
        GS_TRACE_MINOR_CHECK(routine, subfunc)			     	     \
	GS_TRACE_PRIORITY_TEST(module, priority)     			     \
	{								     \
	    TRCHKT (GS_TRC_HKWD(major, routine, subfunc)) ;      	     \
	}								     \
}

#define	GS_PARM_DBG1(major, module, priority, routine, subfunc, p1)	     \
{									     \
        GS_TRACE_MINOR_CHECK(routine, subfunc)			     	     \
	GS_TRACE_PRIORITY_TEST(module, priority)     			     \
	{								     \
	    TRCHKLT (GS_TRC_HKWD(major, routine, subfunc), p1) ; 	     \
	}								     \
}

#define	GS_PARM_DBG(major, module, priority, routine,subfunc,p1,p2,p3,p4,p5) \
{									     \
        GS_TRACE_MINOR_CHECK(routine, subfunc)			     	     \
	GS_TRACE_PRIORITY_TEST(module, priority)     			     \
	{								     \
	    TRCHKGT (GS_TRC_HKWD(major, routine, subfunc), p1,p2,p3,p4,p5) ; \
	}								     \
}

#define GS_EXIT_DBG0(major, module, priority, routine)			     \
{									     \
        GS_TRACE_MINOR_CHECK(routine, EXIT)			     	     \
	GS_TRACE_PRIORITY_TEST(module, priority)     			     \
	{								     \
	    TRCHKT (GS_TRC_HKWD(major, routine, EXIT)) ;   	     	     \
	}								     \
}

#define	GS_EXIT_DBG1(major, module, priority, routine, p1)	     	     \
{									     \
        GS_TRACE_MINOR_CHECK(routine, EXIT)			     	     \
	GS_TRACE_PRIORITY_TEST(module, priority)     			     \
	{								     \
	    TRCHKLT (GS_TRC_HKWD(major, routine, EXIT), p1) ;    	     \
	}								     \
}

#define	GS_EXIT_DBG(major, module, priority, routine, p1, p2, p3, p4, p5)    \
{									     \
        GS_TRACE_MINOR_CHECK(routine, EXIT)			             \
	GS_TRACE_PRIORITY_TEST(module, priority)     			     \
	{								     \
	    TRCHKGT (GS_TRC_HKWD(major, routine, EXIT), p1, p2, p3, p4, p5) ;\
	}								     \
}

#define GS_DATA_DBG(major, module, priority, data_area, subfunc, addr,length)\
{									     \
        GS_TRACE_MINOR_CHECK(data_area, subfunc)		     	     \
	GS_TRACE_PRIORITY_TEST(module, priority)     			     \
	{								     \
	    TRCGENT (0, GS_TRC_HKWD(major, dummy, dummy), 	    	     \
	     	 	GS_TRC_HKWD(major, data_area, subfunc),		     \
			length, addr) ;	  			  	     \
	}								     \
}




#endif /* DEBUG */

#endif /* _H_GS_TRACE */

