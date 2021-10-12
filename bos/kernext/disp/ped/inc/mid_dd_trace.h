/* @(#)59  	1.13.1.6  src/bos/kernext/disp/ped/inc/mid_dd_trace.h, peddd, bos411, 9428A410j 12/2/93 09:46:04 */
#ifndef _H_MID_DD_TRACE
#define _H_MID_DD_TRACE

/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: 
 *		MID_DDT_PI
 *		MID_DDT_WG_PI
 *		MID_DDT_WID
 *		MID_DDT_WID_PI
 *		MID_DD_ENTRY_TRACE
 *		MID_DD_EXIT_TRACE
 *		MID_DD_INT_TRACE
 *		MID_DD_TRACE_DATA
 *		MID_DD_TRACE_DATA_WITH_LEVEL_CHECK
 *		MID_DD_TRACE_PARMS
 *		MID_DEBUG_REGION
 *		MID_DEBUG_WG
 *		MID_DEBUG_WID_LIST
 *		MID_DW0
 *		MID_MODULE
 *		MINOR_HKWD
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

#define  _DISPLAYTRACEFLAG 
#include <sys/disptrc.h>

#include <sys/syspest.h>
#include <sys/types.h>



/* *****************************************************************************

                              MACRO DEFINITION

   MACRO NAME:     MID_MODULE 


   Function: 

    This macro defines several variables for other macros.  Many or most 
    of these variables are related to the name of the module, therefore, 
    this macro could be thought of as registering this module to the 
    macro processor.

  *-------------------------------------------------------------------------*

   Macro specification:
     MID_MODULE (module_name) ;


   Parameter definition:

     module_name - name of the module.  (This is by convention, only, but is 
          probably a good one.  The only real requirement, this that this    
          string be unique from any other used in a MID_MODULE invocation.      


  *-------------------------------------------------------------------------*

   Sample Invocation:
     MID_MODULE (midgeom) ; 


 ******************************************************************************/

#define MID_MODULE(mod_name)                     			       \
	BUGXDEF (dbg_ ## mod_name) ;					       \
	BUGXDEF (perf_ ## mod_name) ;					       \
	BUGXDEF (trc_ ## mod_name) ;










#define MID_DW0(ddf) ddf        

#define MINOR_HKWD(procid, funcid, ex )	 				       \
                ( ( procid | (funcid << 1) | ex ) << 16 )
 

#define MID_DDT_WID(WID)		 	\
	( ((WID)<<16) | (ddf->mid_wid_data.mid_wid_entry[WID].state))

#define MID_DDT_WID_PI(WID)	 	\
    	( ((ddf->mid_wid_data.mid_wid_entry[WID].pi.pieces.color) << 8) | \
       	   (ddf->mid_wid_data.mid_wid_entry[WID].pi.pieces.flags) )

#define MID_DDT_WG_PI(midWG)	 		\
    	( ((midWG->pi.pieces.color) << 8) | 	\
    	   (midWG->pi.pieces.flags) )

#define MID_DDT_PI(midWG)	 	\
    	( MID_DDT_WG_PI(midWG) ) << 16 | ( MID_DDT_WID_PI(midWG->wid)) 

 

#if     0
#define MID_DD_INT_TRACE(ddf, hook, p1, p2 ,p3)  			       \
	memtrace (ddf, hook, p1, p2 ,p3) ;

#else  

/****************************************************************************** 
                              
                             Start of Prologue  
  
   Macro Name:    MID_DD_INT_TRACE  (or memtrace routine)
  
   Descriptive Name:  Produce an internal memory trace of the Ped DD functions
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
  
     
     This routine writes a 16-byte trace entry in the next trace entry and 
     increments the trace pointer (with wrap). 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
  
  
    Dependencies:
  
  
 *--------------------------------------------------------------------------* 
  
    Linkage:
           Called internally from the WID handling code of the Ped device
           driver.  


 *--------------------------------------------------------------------------* 
  
    INPUT:  ddf addresss 
          
    OUTPUT:  none 
          
    RETURN CODES:  none         
          
                                 End of Prologue                                
 ******************************************************************************/



#define MID_DD_INT_TRACE(ddf, hook, p1, p2 ,p3)  			       \
									       \
{									       \
									       \
/***************************************************************************** \
   Local variables: 							       \
 *****************************************************************************/\
									       \
dd_trace_entry_t 	*trace_buffer ;		/* pointer to trace buffer */  \
									       \
BUGXDEF (dbg_memtrace) ;						       \
									       \
									       \
    /*-------------------------------------------------------------------*     \
           Make the trace entry 					       \
    *--------------------------------------------------------------------*/    \
									       \
    trace_buffer = (ddf) -> int_trace.next ; 				       \
									       \
    trace_buffer -> id  = (hook) ; 					       \
    trace_buffer -> parm1  = (ulong) (p1) ; 				       \
    trace_buffer -> parm2  = (ulong) (p2) ; 				       \
    trace_buffer -> parm3  = (ulong) (p3) ; 				       \
									       \
									       \
									       \
    /*-------------------------------------------------------------------*     \
           Increment the trace entry					       \
    *--------------------------------------------------------------------*/    \
									       \
    trace_buffer ++ ;							       \
									       \
									       \
    if ( trace_buffer < (ddf) -> int_trace.last )			       \
    {									       \
    	(ddf) -> int_trace.next = trace_buffer ;			       \
    }									       \
    else /* wrap */ 							       \
    {									       \
    	(ddf) -> int_trace.next = (ddf) -> int_trace.top ;		       \
    }									       \
									       \
}


#endif  




/* *****************************************************************************

                              MACRO DEFINITION

   MACRO NAME:     MID_DD_TRACE_PARMS


   Function: 

    This macro provides an interface to the system trace facility for the      
    Mid-level graphics adapter device driver.  There are actually several
    macros which provide the full set of system trace functions.
    These are: 
       MID_DD_TRACE_PARMS  -  Debug trace for 1 to 4 discrete parameters
                               with capability for prioritizing traces.  
       MID_DD_TRACE_DATA   -  Debug trace for a string of contiguous data
                               with capability for prioritizing traces.  
       MID_DD_TRACE_PERF   -  Performance trace

  *-------------------------------------------------------------------------*

   Macro specification:
     MID_DD_TRACE_PARMS (module, priority, minor_hook, 
				parm0, parm1, parm2, parm3, parm4);


   Parameter definition:

     module - module/function invoking the trace.  This parameter MUST match 
          the string used to define the module/function in the MID_MODULE macro
          and must match a similar string defined in middiag.c (which is used
          to turn these traces on). 

     priority - This parameter specifies a numeric priority for the trace
          entry.  This priority is compared with the external variable defined
          in middiag.c.  When the priority equals or exceeds that specified in
	  middiag.c, the trace entry is made.  The numeric is an unsigned 
	  integer.  (Hence, traces with a priority of 0 are always made.)

     minor_hook - Minor hook assigned to the module/function.  These are of 
          the form:  hkwd_DISPLAY_MIDDDF_UPDATE_WIN_GEOM.  Currently, these
          are defined in mid_dd_trace.h  (lower part of this include). 

     parm0 - should be passed with the device driver common (ddf) structure 
          pointer.  This is intended to be used to distinguish which adapter
          the trace is from.  Currently, this function (adapter distinction) 
	  is unimplemented.  

     parm1 - parm4 - 32 bit parameters.  This is the data which is traced.
          If less than four parameters are required, the unused    
          parameters should be set to 0. 

  *-------------------------------------------------------------------------*


   Sample Invocation:
   MID_DD_TRACE_PARMS (midgeom, 1, UPDATE_WG, ddf, pGD, pWGold, pWGnew, wid);


 ******************************************************************************/

#ifndef  DEBUG

#define MID_DD_TRACE_PARMS(module, priority, minor_hook, ddf, p1, p2, p3, p4)\
	 								       \
	MID_DD_INT_TRACE (ddf,						       \
	        	  HKWD (HKWD_DISPLAY_DDF_D, 			       \
			        hkwd_DISPLAY_MIDDDF_ ## minor_hook, 1, 0),     \
		          p2, p3, p4) ;	

#else  /* DEBUG */


#define MID_DD_TRACE_PARMS(module, priority, minor_hook, ddf, p1, p2, p3, p4)\
	 								       \
	MID_DD_INT_TRACE (ddf,						       \
	        	  HKWD (HKWD_DISPLAY_DDF_D, 			       \
			        hkwd_DISPLAY_MIDDDF_ ## minor_hook, 1, 0),     \
		          p2, p3, p4) ;					       \
	 								       \
	if (priority <=  trc_ ## module)				       \
	{								       \
		trchkgt(                                                       \
		        HKWD (HKWD_DISPLAY_DDF_D, 			       \
			      hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 0),       \
		        	MID_DW0(ddf), p1, p2, p3, p4) ;		       \
	}								       

#endif  /* DEBUG */











/* *****************************************************************************

                              MACRO DEFINITION

   MACRO NAME:     MID_DD_TRACE_DATA 


   Function: 

    This macro provides an interface to the system trace facility for the      
    Mid-level graphics adapter device driver.  There are actually several
    macros which provide the full set of system trace functions.

    These are: 
       MID_DD_TRACE_PARMS  -  Debug trace for 1 to 4 discrete parameters
       MID_DD_TRACE_DATA   -  Debug trace for a string of contiguous data
       MID_DD_TRACE_PERF   -  Performance trace

    In addition, the trace data interface, is generally provided by a 
    yet-higher-layer macro.  Amongst the data trace macros are: 
       MID_DEBUG_WG  -  provides both printf and traces for window geometries
       MID_DEBUG_REGION  -  provides both printf and traces for regions
       MID_DEBUG_WID_LIST  -  provides both printf and traces for the WID list
       
  *-------------------------------------------------------------------------*

   Macro specification:
     MID_DD_TRACE_DATA (ID, minor_hook, ddf, length, address) 		       


   Parameter definition:

     module - module/function invoking the trace.  This parameter was intended
          to be used for the middiag/external variable control for turning
          traces on and off.  Generally, this function has been moved to the
          higher layer macros, so this parameter is currently unused. 

     minor_hook - Minor hook assigned to the module/function.  These are of 
          the form:  hkwd_DISPLAY_MIDDDF_UPDATE_WIN_GEOM.  Currently, these
          are defined in mid_dd_trace.h  (lower part of this include). 

     ddf - This is intended to provide a mechanism to distiguish multiple
     	  multiple adapters from each other.  The intent is to use the   )   
          single parameter in the trcgen interface to hold some distinguishing
          value, however, the upper 16 bits of this paraemeter had to be stolen
	  as a place to put the minor hook.  So if we do try to distinguish 
     	  adapters, it must be done in 16 bits. 

     length - Number of bytes of data pointed to by address. 

     address - Address of data to be traced. 


  *-------------------------------------------------------------------------*

   Sample Invocation:
     MID_DD_TRACE_DATA (midgeom, Region, ddf, 
			numBoxes*sizeof(gBox), pRegion) ; 

 ******************************************************************************/


#ifndef  DEBUG

#define MID_DD_TRACE_DATA(module, minor_hook, ddf, address, length)

#define MID_DEBUG_WG(function, priority, pWG)
#define MID_DEBUG_REGION(function, priority, pClip) 
#define MID_DEBUG_WID_LIST(function, priority, ddf)

#else  /* DEBUG */


#define MID_DD_TRACE_DATA(module, minor_hook, ddf, address, length)	       \
		                                                               \
	trcgenkt(0,                    		                               \
	         HKWD (HKWD_DISPLAY_DDF_D, 				       \
			      hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 0),       \
	         MINOR_HKWD ( hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 0),       \
	         length, address) ;					       \



 /***************************************************************************** 
   The following macro is probably unused. 
 ******************************************************************************/
#define MID_DD_TRACE_DATA_WITH_LEVEL_CHECK(module, priority, minor_hook, ddf,  \
						address, length)	       \
	if (trc_ ## module >= priority)					       \
	{								       \
		trcgenkt(0,                                                    \
		         HKWD (HKWD_DISPLAY_DDF_D, 			       \
			      hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 0),       \
		         DATA_HKWD (HKWD_DISPLAY_DDF_D,			       \
			      hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 0),       \
		         length, address) ;				       \
	}								       



/***************************************************************************** 
 ***************************************************************************** 

     HIGHER LAYER DEBUG interfaces: 

         The following macros form the higher-layer interfaces to debug 
	 output; i.e. both print (printf) and system trace.

 ***************************************************************************** 
*****************************************************************************/


/***************************************************************************** 
     Window Geometry
*****************************************************************************/

#define MID_DEBUG_WG(function, priority, pWG)				       \
                                                                               \
{									       \
BUGXDEF(trc_ ## function ## _WG) ;					       \
                                                                               \
    if (trc_ ## function ## _WG >= priority)				       \
    {                                                                          \
        if ((pWG) != NULL) 						       \
        {								       \
	    MID_DD_TRACE_DATA (WG, WG, ddf, (pWG), sizeof(rcmWG)) ;	       \
   	}                                                                      \
     }                                                                         \
}									       \
                                                                               \
                                                                               \
{									       \
BUGXDEF(dbg_ ## function ## _WG) ;					       \
                                                                               \
    if (dbg_ ## function ## _WG >= priority)				       \
    {                                                                          \
   	mid_print_WG (pWG) ;					       	       \
    }                                                                          \
}									       


/***************************************************************************** 
     Region 
*****************************************************************************/

#define MID_DEBUG_REGION(function, priority, pClip)  			       \
                                                                               \
{									       \
BUGXDEF(trc_ ## function ## _Region) ;					       \
                                                                               \
    if (trc_ ## function ## _Region >= priority)			       \
    {									       \
        if ((pClip) != NULL) 						       \
        {								       \
	    MID_DD_TRACE_DATA (Region, REGION, ddf,			       \
		                  (pClip), sizeof(gRegion)) ;		       \
                                                                               \
    	    if ( (pClip)-> numBoxes != 0) 				       \
    	    {								       \
    	  	MID_DD_TRACE_DATA (Region, VISIBLE_RECTS, ddf,		       \
	       		           ((pClip)->pBox),		      	       \
    	    			   (((pClip)->numBoxes) * sizeof(gBox)) );     \
    	    }								       \
        }								       \
    }									       \
}									       \
                                                                               \
                                                                               \
{									       \
BUGXDEF(dbg_ ## function ## _Region) ;					       \
                                                                               \
    if (dbg_ ## function ## _Region >= priority)				       \
    {	                                                                       \
   	mid_print_region (pClip) ;					       \
    }	                                                                       \
}									       




/***************************************************************************** 
     Window ID (WID) List 
*****************************************************************************/

#define MID_DEBUG_WID_LIST(function, priority, ddf)			       \
                                                                               \
{									       \
BUGXDEF(trc_ ## function ## _WID_LIST) ;				       \
                                                                               \
    if (trc_ ## function ## _WID_LIST >= priority)			       \
    {                                                                          \
   	mid_WID_trace (ddf) ;					       	       \
    }                                                                          \
}									       \
                                                                               \
                                                                               \
{									       \
BUGXDEF(dbg_ ## function ## _WID_LIST) ;				       \
                                                                               \
    if (dbg_ ## function ## _WID_LIST >= priority)			       \
    {                                                                          \
   	midl_print (ddf, "list") ;				       	       \
    }                                                                          \
}									       


#endif  /* DEBUG */





/***************************************************************************** 

                              MACRO DEFINITION

   MACRO NAME:     MID_DD_ENTRY_TRACE
                   MID_DD_EXIT_TRACE


   Function: 

    These two macros provide an interface to the system trace facility for 
    performance measurements.  Generally, performance measurements are made
    at module entry and exit.  If other measurements become necessary, addi-
    macros may be written.

  *-------------------------------------------------------------------------*

   Macro specification:
     MID_DD_tttt_PERF_TRACE (module, TRACE_ID) 


   Parameter definition:

     module - module/function invoking the trace.  This parameter MUST match the
          string used to define the module/function in the MID_MODULE macro
          AND must match a similar string defined in middiag.c (which is used
          to turn these traces on). 

     TRACE_ID - trace ID of function invoking the trace.  This parameter MUST 
	  match the (end of) a defined TRACE_ID.  These trace IDs are 
          currently defined in mid_dd_trace.h.

  *-------------------------------------------------------------------------*

    Priority of trace entrys: 
     
     	Currently, there is no concept of priority in the performance traces.
	A module's (or function's) performance traces are either off or on as 
        determined by the flag defined in middiag.c. 

  *-------------------------------------------------------------------------*

   Sample Invocation:
     MID_DD_ENTRY_TRACE (midgeom, UPDATE_WG) ;

*****************************************************************************/


#ifndef  DEBUG

#define MID_DD_ENTRY_TRACE(module, priority, minor_hook, ddf, p1, p2, p3, p4)  \
	 								       \
	MID_DD_INT_TRACE (ddf,						       \
       	    		  HKWD (HKWD_DISPLAY_DDF_D, 			       \
	          		hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 0),     \
            		  p2, p3, p4) ;	
	
	
#else /* DEBUG */


#ifdef  COMPFIX

#define MID_DD_ENTRY_TRACE(module, priority, minor_hook, ddf, p1, p2, p3, p4)  \
	 								       \
	if(perf_ ## module >= 1)	 				       \
	{								       \
		trchkt (						       \
		         HKWD (HKWD_DISPLAY_DDF_P, 			       \
			      hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 0) ) ;    \
	}								       \
	 								       \
	else 		 		 				       \
	{								       \
		MID_DD_TRACE_PARMS (module, priority, minor_hook, 	       \
					ddf, p1, p2, p3, p4) ; 		       \
	}

#else


#define MID_DD_ENTRY_TRACE(module, priority, minor_hook, ddf, p1, p2, p3, p4)  \
	 								       \
	if(perf_ ## module >= 1)	 				       \
	{								       \
		trchkt (   			 	 		       \
		         HKWD (HKWD_DISPLAY_DDF_P, 			       \
			      hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 0) ) ;    \
	}								       \
	else 		 		 				       \
	{								       \
		MID_DD_INT_TRACE (ddf,					       \
	            		HKWD (HKWD_DISPLAY_DDF_D, 		       \
		          	     hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 0),\
		          	p2, p3, p4) ;				       \
	 								       \
		if(trc_ ## module >= priority)				       \
		{							       \
			trchkgt(                                               \
		        	HKWD (HKWD_DISPLAY_DDF_D, 		       \
			      	hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 0),     \
		        	MID_DW0(ddf), p1, p2, p3, p4) ;		       \
		}							       \
	}

#endif
#endif /* DEBUG */



#ifndef  DEBUG

#define MID_DD_EXIT_TRACE(module, priority, minor_hook, ddf, p1, p2, p3, p4)   \
	 								       \
	MID_DD_INT_TRACE (ddf,						       \
	        	  HKWD (HKWD_DISPLAY_DDF_D, 			       \
			        hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 1),     \
		          p2, p3, p4) ;
	
	
#else /* DEBUG */



#ifdef  COMPFIX

#define MID_DD_EXIT_TRACE(module, priority, minor_hook, ddf, p1, p2, p3, p4)   \
	 								       \
	if(perf_ ## module >= 1)	 				       \
	{								       \
		trchkt (   			 	 		       \
		         HKWD (HKWD_DISPLAY_DDF_P, 			       \
			      hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 1) ) ;    \
	}								       \
	else 		 		 				       \
	{								       \
		MID_DD_TRACE_PARMS (module, priority, minor_hook,  	       \
					ddf, p1, p2, p3, p4) ; 		       \
	}


#else

#define MID_DD_EXIT_TRACE(module, priority, minor_hook, ddf, p1, p2, p3, p4)   \
	 								       \
	if(perf_ ## module >= 1)	 				       \
	{								       \
		trchkt (   			 	 		       \
		         HKWD (HKWD_DISPLAY_DDF_P, 			       \
			      hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 1) ) ;    \
	}								       \
	 								       \
	else 		 		 				       \
	{								       \
		MID_DD_INT_TRACE (ddf,					       \
	        	  HKWD (HKWD_DISPLAY_DDF_D, 			       \
			        hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 1),     \
		          p2, p3, p4) ;					       \
	 								       \
		if(trc_ ## module >= priority)				       \
		{							       \
			trchkgt(                                               \
		        	HKWD (HKWD_DISPLAY_DDF_D, 		       \
			      	hkwd_DISPLAY_MIDDDF_ ## minor_hook, 0, 1),     \
		        	MID_DW0(ddf), p1, p2, p3, p4) ;		       \
		}							       \
	}

#endif
#endif /* DEBUG */






  /***************************************************************************
   ***************************************************************************

                                 TRACE IDs: 

   ***************************************************************************
  ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_MID_MAKE_GP    ( 0x001 << 4 )
#define hkwd_DISPLAY_MIDDDF_MID_UNMAKE_GP  ( 0x002 << 4 )

#define hkwd_DISPLAY_MIDDDF_SET_PRIORITY   ( 0x003 << 4 )

#define hkwd_DISPLAY_MIDDDF_MID_CREATE_RCX ( 0x004 << 4 )
#define hkwd_DISPLAY_MIDDDF_MID_DELETE_RCX ( 0x005 << 4 )

#define hkwd_DISPLAY_MIDDDF_CREATE_WG      ( 0x006 << 4 )
#define hkwd_DISPLAY_MIDDDF_DELETE_WG      ( 0x007 << 4 )
#define hkwd_DISPLAY_MIDDDF_UPDATE_WG      ( 0x008 << 4 )

#define hkwd_DISPLAY_MIDDDF_CREATE_WA      ( 0x009 << 4 )
#define hkwd_DISPLAY_MIDDDF_DELETE_WA      ( 0x00A << 4 )
#define hkwd_DISPLAY_MIDDDF_UPDATE_WA      ( 0x00B << 4 )

#define hkwd_DISPLAY_MIDDDF_BIND           ( 0x00C << 4 )
#define hkwd_DISPLAY_MIDDDF_SWITCH_RCX     ( 0x00D << 4 )

#define hkwd_DISPLAY_MIDDDF_LOCK_HW        ( 0x00E << 4 )
#define hkwd_DISPLAY_MIDDDF_UNLOCK_HW      ( 0x00F << 4 )
#define hkwd_DISPLAY_MIDDDF_LOCK_DOMAIN    ( 0x010 << 4 )
#define hkwd_DISPLAY_MIDDDF_UNLOCK_DOMAIN  ( 0x011 << 4 )

#define hkwd_DISPLAY_MIDDDF_CREATE_RCXP    ( 0x012 << 4 )
#define hkwd_DISPLAY_MIDDDF_DELETE_RCXP    ( 0x013 << 4 )
#define hkwd_DISPLAY_MIDDDF_ASSOCIATE_RCXP    ( 0x014 << 4 )
#define hkwd_DISPLAY_MIDDDF_DISASSOCIATE_RCXP ( 0x015 << 4 )

  /***************************************************************************
        Leave some spares here (from 0x16 to 0x2F)
   ***************************************************************************/

  /***************************************************************************
        0x30 - 0x3E reserved for DDF functions 
        0x58 - 0x5F below are also reserved for DDF functions
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_DMA      		( 0x030 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_COLOR      	( 0x031 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_CPOS    	( 0x032 << 4 )
#define hkwd_DISPLAY_MIDDDF_SWAP_BUFFERS 	( 0x033 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_CONDITION	( 0x034 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_MODEL_MATRIX	( 0x035 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_PROJ_MATRIX	( 0x036 << 4 )
#define hkwd_DISPLAY_MIDDDF_TEXT_FONT_INDEX 	( 0x037 << 4 )

#define hkwd_DISPLAY_MIDDDF_GET_COLOR_PARMS    	( 0x038 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_CPOS_PARMS    	( 0x039 << 4 )
#define hkwd_DISPLAY_MIDDDF_SWAP_BUFFERS_PARMS 	( 0x03A << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_CONDITION_PARMS	( 0x03B << 4 )
#define hkwd_DISPLAY_MIDDDF_DMA_SLEEP		( 0x03C << 4 )

#define hkwd_DISPLAY_MIDDDF_TEXT_FONT_INDEX_PARMS ( 0x03E << 4 )



  /***************************************************************************
        Check dev is part of the RCM - device driver interface
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_CHECK_DEV 		( 0x03F << 4 )


  /***************************************************************************
   			     WID handling code 
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_WID_INIT       ( 0x040 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_WID_RENDER ( 0x041 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_WID_SWAP   ( 0x042 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_WID_PI     ( 0x043 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_UNUSED_WID ( 0x044 << 4 )
#define hkwd_DISPLAY_MIDDDF_STEAL_WID      ( 0x045 << 4 )
#define hkwd_DISPLAY_MIDDDF_DELETE_WID     ( 0x046 << 4 )
#define hkwd_DISPLAY_MIDDDF_RELEASE_WID_SWAP ( 0x047 << 4 )
#define hkwd_DISPLAY_MIDDDF_STEAL_WID_2    ( 0x048 << 4 )
#define hkwd_DISPLAY_MIDDDF_REALLY_STEAL_WID (0x049 << 4 )
#define hkwd_DISPLAY_MIDDDF_WID_SLEEP	   ( 0x04A << 4 )
#define hkwd_DISPLAY_MIDDDF_WID_WAKEUP	   ( 0x04B << 4 )
#define hkwd_DISPLAY_MIDDDF_WID_CTX_SLEEP  ( 0x04C << 4 )
#define hkwd_DISPLAY_MIDDDF_WID_CTX_WAKEUP ( 0x04D << 4 )
#define hkwd_DISPLAY_MIDDDF_ADD_NEW_WID    ( 0x04E << 4 )
#define hkwd_DISPLAY_MIDDDF_WID_LIST       ( 0x04F << 4 )

#define hkwd_DISPLAY_MIDDDF_GENERIC_PARM   ( 0x050 << 4 )
#define hkwd_DISPLAY_MIDDDF_GENERIC_PARM1  ( 0x051 << 4 )
#define hkwd_DISPLAY_MIDDDF_GENERIC_PARM2  ( 0x052 << 4 )
#define hkwd_DISPLAY_MIDDDF_GENERIC_PARM3  ( 0x053 << 4 )
#define hkwd_DISPLAY_MIDDDF_GENERIC_PARM4  ( 0x054 << 4 )
#define hkwd_DISPLAY_MIDDDF_GENERIC_PARM5  ( 0x055 << 4 )

  /***************************************************************************
        0x58 - 0x5F reserved for DDF functions 
        0x30 - 0x3E above are also reserved for DDF functions
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_END_RENDER			( 0x058 << 4 )
#define hkwd_DISPLAY_MIDDDF_GET_FREE_PENDING_REQ_STRUCT	( 0x059 << 4 )

  /***************************************************************************
   		       Interrupt Handling  - BIM status interrupts
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_INTERRUPT                     ( 0x060 << 4 )
#define hkwd_DISPLAY_MIDDDF_INTERRUPT_2                   ( 0x061 << 4 )
#define hkwd_DISPLAY_MIDDDF_INTR_FIFO_AVAILABLE 	  ( 0x062 << 4 )
#define hkwd_DISPLAY_MIDDDF_INTR_FIFO_FULL  		  ( 0x063 << 4 )
#define hkwd_DISPLAY_MIDDDF_INTR_FIFO_EMPTY 		  ( 0x064 << 4 )
#define hkwd_DISPLAY_MIDDDF_INTR_SWITCH_DONE		  ( 0x065 << 4 )

#define hkwd_DISPLAY_MIDDDF_UNEXPECTED_LWM_INT            ( 0x06D << 4 )
#define hkwd_DISPLAY_MIDDDF_UNEXPECTED_HWM_INT            ( 0x06E << 4 )
#define hkwd_DISPLAY_MIDDDF_LOW_WATER_WATCHDOG		  ( 0x06F << 4 )

  /***************************************************************************
		       Interrupt Handling  - DSP status interrupts
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_INTR_DSP 			  ( 0x070 << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_GET_COLOR		  ( 0x071 << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_GET_CPOS		  ( 0x072 << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_GET_CONDITION 		  ( 0x073 << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_PICK 			  ( 0x075 << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_CHKWATER	  	  ( 0x077 << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_NEWCTX			  ( 0x078 << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_PINCTX_DATA		  ( 0x079 << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_PINCTX			  ( 0x07B << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_GET_FONT_INDEX 		  ( 0x07C << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_FIFO_STALLED   		  ( 0x07D << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_SWAP			  ( 0x07E << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_END_RENDER 		  ( 0x07F << 4 )

#define hkwd_DISPLAY_MIDDDF_DSP_GET_COLOR_PARMS		  ( 0x091 << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_GET_CPOS_PARMS		  ( 0x092 << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_GET_CONDITION_PARMS	  ( 0x093 << 4 )
#define hkwd_DISPLAY_MIDDDF_HIGH_WATER_COMMON             ( 0x094 << 4 )
#define hkwd_DISPLAY_MIDDDF_LOW_WATER_COMMON		  ( 0x095 << 4 )

#define hkwd_DISPLAY_MIDDDF_DSP_GET_FONT_INDEX_PARMS	  ( 0x09C << 4 )
#define hkwd_DISPLAY_MIDDDF_DSP_SWAP_PARMS		  ( 0x09E << 4 )



  /***************************************************************************
                                KSR traces
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_MID_INIT                      ( 0x0A0 << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTACT                        ( 0x0A1 << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTDACT                       ( 0x0A2 << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTSETM                       ( 0x0A3 << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTSTCT                       ( 0x0A4 << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTTERM                       ( 0x0A5 << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTINIT                       ( 0x0A6 << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTCLR                        ( 0x0A7 << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTCPL                        ( 0x0A8 << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTRDS                        ( 0x0A9 << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTTEXT                       ( 0x0AA << 4 )
#define hkwd_DISPLAY_MIDDDF_COPY_PS                       ( 0x0AB << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTCFL                        ( 0x0AC << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTSCR                        ( 0x0AD << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTDEFC                       ( 0x0AE << 4 )
#define hkwd_DISPLAY_MIDDDF_VTTMOVC                       ( 0x0AF << 4 )



  /***************************************************************************
                                Font traces
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_FONT_REQUEST                  ( 0x0B0 << 4 )
#define hkwd_DISPLAY_MIDDDF_REQUEST                       ( 0x0B0 << 4 )
#define hkwd_DISPLAY_MIDDDF_PINFONT                       ( 0x0B1 << 4 )
#define hkwd_DISPLAY_MIDDDF_UNPINFONT                     ( 0x0B2 << 4 )
#define hkwd_DISPLAY_MIDDDF_FONT_READY                    ( 0x0B3 << 4 )
#define hkwd_DISPLAY_MIDDDF_FONT_HOTKEY                   ( 0x0B4 << 4 )

  /***************************************************************************
                                Pick event traces
   ***************************************************************************/
#define hkwd_DISPLAY_MIDDDF_BEGINPICK_EVENT               ( 0x0C0 << 4 )
#define hkwd_DISPLAY_MIDDDF_ENDPICK_EVENT                 ( 0x0C1 << 4 )
#define hkwd_DISPLAY_MIDDDF_SERIALIZE_BEGINPICK           ( 0x0C2 << 4 )
#define hkwd_DISPLAY_MIDDDF_UNBLOCK_BEGINPICK             ( 0x0C3 << 4 )
#define hkwd_DISPLAY_MIDDDF_SYNC_MASK      		  ( 0x0C4 << 4 )

#define hkwd_DISPLAY_MIDDDF_SET_CLIENT_CLIP    		  ( 0x0CC << 4 )


  /***************************************************************************
		       Context Switching Traces 
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_START_SWITCH_NO_RENQ       	  ( 0x0D0 << 4 )
#define hkwd_DISPLAY_MIDDDF_START_SWITCH_RENQ        	  ( 0x0D1 << 4 )
#define hkwd_DISPLAY_MIDDDF_WAIT_SWITCH_END            	  ( 0x0D2 << 4 )
#define hkwd_DISPLAY_MIDDDF_START_SWITCH_FATES       	  ( 0x0D3 << 4 )
#define hkwd_DISPLAY_MIDDDF_WAIT_FIFO 	           	  ( 0x0D4 << 4 )
#define hkwd_DISPLAY_MIDDDF_START_SWITCH_INT_MASK    	  ( 0x0D5 << 4 )


#define hkwd_DISPLAY_MIDDDF_END_SWITCH               	  ( 0x0D8 << 4 )
#define hkwd_DISPLAY_MIDDDF_END_SWITCH_2             	  ( 0x0D9 << 4 )
#define hkwd_DISPLAY_MIDDDF_CTX_DMASTER              	  ( 0x0DA << 4 )
#define hkwd_DISPLAY_MIDDDF_CTX_DCOMPLETE            	  ( 0x0DB << 4 )

#define hkwd_DISPLAY_MIDDDF_GET_CONTEXT_SAVE_AREA      	  ( 0x0DC << 4 )
#define hkwd_DISPLAY_MIDDDF_DEALLOCATE_CONTEXT         	  ( 0x0DD << 4 )
#define hkwd_DISPLAY_MIDDDF_DEF_CTX_ALLOC            	  ( 0x0DE << 4 )
#define hkwd_DISPLAY_MIDDDF_DEFAULT_CONTEXT          	  ( 0x0DF << 4 )



  /***************************************************************************
   			       Special Data Traces
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_GENERIC_DATA                  ( 0x0E0 << 4 )
#define hkwd_DISPLAY_MIDDDF_ASSOCIATE_COLOR_SE            ( 0x0E1 << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_WID_PLANES_FIFO_SE      ( 0x0E2 << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_WID_PLANES_PCB_SE       ( 0x0E3 << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_WINDOW_PARMS_FIFO_SE    ( 0x0E4 << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_WINDOW_PARMS_PCB_SE     ( 0x0E5 << 4 )
#define hkwd_DISPLAY_MIDDDF_SET_CONTEXT_IO_CE             ( 0x0E6 << 4 )

#define hkwd_DISPLAY_MIDDDF_WG_CHAIN                      ( 0x0E7 << 4 )
#define hkwd_DISPLAY_MIDDDF_RCX                           ( 0x0E8 << 4 )
#define hkwd_DISPLAY_MIDDDF_MIDRCX                        ( 0x0E9 << 4 )
#define hkwd_DISPLAY_MIDDDF_WG                            ( 0x0EA << 4 )
#define hkwd_DISPLAY_MIDDDF_MIDWG                         ( 0x0EB << 4 )
#define hkwd_DISPLAY_MIDDDF_WA                            ( 0x0EC << 4 )
#define hkwd_DISPLAY_MIDDDF_MIDWA                         ( 0x0ED << 4 )

#define hkwd_DISPLAY_MIDDDF_REGION                        ( 0x0EE << 4 )
#define hkwd_DISPLAY_MIDDDF_VISIBLE_RECTS                 ( 0x0EF << 4 )

  /***************************************************************************
   			          I/O routines 
   ***************************************************************************/

#define hkwd_DISPLAY_MIDDDF_ASSOCIATE_COLOR_PCB      ( 0x0F0 << 4 )
#define hkwd_DISPLAY_MIDDDF_ASSOCIATE_COLOR          ( 0x0F1 << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_WID_PLANES_FIFO    ( 0x0F2 << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_WID_PLANES_PCB     ( 0x0F3 << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_WINDOW_PARMS_FIFO  ( 0x0F4 << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_WINDOW_PARMS_PCB   ( 0x0F5 << 4 )
#define hkwd_DISPLAY_MIDDDF_DRAIN_FIFO               ( 0x0F6 << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_WINDOW_PARMS_SUB   ( 0x0F7 << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_DELAY_WID_PLANE_FIFO  ( 0x0f8 << 4 )
#define hkwd_DISPLAY_MIDDDF_FLUSH_WIDS_COLORMP_UPDATE  ( 0x0f9 << 4 )

#define hkwd_DISPLAY_MIDDDF_WRITE_TO_HCR             ( 0x0FA << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_TO_PCB             ( 0x0FB << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_TO_PCB_TIMEOUT     ( 0x0FC << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_TO_FIFO            ( 0x0FD << 4 )
#define hkwd_DISPLAY_MIDDDF_WRITE_TO_FIFO_SLEEP      ( 0x0FE << 4 )



#endif
