/* @(#)74       1.1.1.2  src/bos/kernext/rcm/inc/rcm_trc.h, rcm, bos411, 9428A410j 10/27/93 16:16:38 */

/*
 * COMPONENT_NAME: (rcm) AIX Rendering Context Manager structure definitions
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989-1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*================================================================
		    INCLUDES
  ==============================================================*/

#ifndef _H_RCM_TRC
#define _H_RCM_TRC

#define  _DISPLAYTRACEFLAG 
#include <sys/disptrc.h>

#include <sys/syspest.h>
#include <sys/types.h>



/*================================================================
		    MACROS
  ==============================================================*/


/* -------------------------------------------------------------

    RCM_FIND_WG_TRACE --  Produces a system trace record of a FIND_WG search
    RCM_WG_HASH_TRACE --  Produces a system trace record of a WG hash chain

 * ------------------------------------------------------------- */

#ifndef WG_DEBUG

#define RCM_FIND_WG_TRACE(p0, p1, p2, p3, p4)
#define RCM_WG_HASH_TRACE(pdev,pwg,type)

#else

#define RCM_FIND_WG_TRACE(p0, p1, p2, p3, p4)				\
	 								\
	TRCHKGT (HKWD (HKWD_DISPLAY_RCM_D, 				\
			      	hkwd_DISPLAY_RCM_ ## FIND_WG, 0, 3),	\
		        	p0, p1, p2, p3, p4) ;

#define RCM_WG_HASH_TRACE(pdev,pwg,type)			\
    { 								\
    	rcmWG *pWG[4] = {0, 0, 0, 0} ;				\
    	rcmWG *temp ; 						\
    	ulong index, old_int;					\
    	ulong n = 0 ;   					\
      								\
        old_int = i_disable(INTMAX);				\
    	index = RCM_WG_HASH(pwg) ;			 	\
    	temp = (rcmWG *)(pdev->devHead.wg_hash_table->entry[index].pWG) ;\
      								\
    	while (temp != NULL)      				\
    	{  							\
    		pWG[n] = temp ;					\
    		n++ ;						\
    		temp = temp->pNext ;				\
    	}  							\
      								\
    	TRCHKGT (HKWD_USER1+0x690+type, 			\
			index, pdev->devHead.window_count,	\
			pWG[0], pWG[1], pWG[2] ) ;		\
        i_enable( old_int );                          	        \
    }

#endif 


/* -------------------------------------------------------------

    DEPTH_INIT  and  DEPTH_INC  --  Track hash depth for FIND_WG

    CHECK_FOR_LARGE_DEPTH -- macro to ascertain whether the hashing
 	"depth" for a particular WG is unnaturally large.  This may
	indicate a pathological condition where WG pointers are hashing
	to the same index.  The "if" check is somewhat arbitrary, ie,
	we attempt to determine if the hash-depth is greater than 
	twice the average.

 * ------------------------------------------------------------- */

#ifdef  WG_DEBUG
#	define DEPTH_INIT(depth)					\
		depth = 0;

#	define DEPTH_INC(depth)					\
		depth++;

#	define CHECK_FOR_LARGE_DEPTH(pdev, depth)			\
	{								\
		/* If depth GT twice average, print message.      */    \
									\
		if (depth > 						\
		(pdev->devHead.window_count*4 / RCM_WG_HASH_SIZE +2))   \
		{							\
		  printf("\n Hash depth for WG is %d",depth);           \
		  printf("\n This is > than 4 times average depth"      \
		  "  window_count=%d",pdev->devHead.window_count);      \
		}							\
	}

#else
#	define DEPTH_INIT(depth)

#	define DEPTH_INC(depth)

#	define CHECK_FOR_LARGE_DEPTH(pdev, depth)
#endif
		



 /***************************************************************************
  ***************************************************************************
    	  	gsctrace trace macros.
  ***************************************************************************
  ***************************************************************************/

#ifdef DEBUG
#	define GSCTRACE_SWITCH
#endif

#ifdef GSCTRACE_SWITCH

#	define gsctrace( component, trace_id )				     \
	{				   				     \
	  TRCHKT(HKWD_DISPLAY_RCM_P | ((component & 0xff) << 4) | trace_id); \
	}

#else   /* stub it out */

#	define gsctrace( component, trace_id )	

#endif









/***************************************************************************** 

                              MACRO DEFINITION

   MACRO NAME:     RCM_DEBUG_ENTRY_TRACE
                   RCM_DEBUG_EXIT_TRACE
                   RCM_DEBUG_TRACE


   Function: 

    These macros provide an interface to the system trace facility for 
    debug tracing.  

  *-------------------------------------------------------------------------*

   Macro specification:

   Parameter definition:

  *-------------------------------------------------------------------------*

   Sample Invocation:

*****************************************************************************/


#ifdef WG_DEBUG
#define  RCM_DEBUG_TRACE_SWITCH
#endif


#ifdef DEBUG
#define  RCM_DEBUG_TRACE_SWITCH
#endif


#ifndef  RCM_DEBUG_TRACE_SWITCH

 /***************************************************************************
	Non-system trace versions (compile option OFF)
  ***************************************************************************/

#define RCM_DEBUG_ENTRY_TRACE(minor_hook, p0, p1, p2, p3, p4)		\
	RCM_TRACE (hkwd_DISPLAY_RCM_ ## minor_hook, p2, p3, p4) ;	
	

#define RCM_DEBUG_EXIT_TRACE(minor_hook, p0, p1, p2, p3, p4)		\
	RCM_TRACE (hkwd_DISPLAY_RCM_ ## minor_hook, p2, p3, p4) ;	
	
#define RCM_DEBUG_TRACE(minor_hook, p0, p1, p2, p3, p4)		\
	RCM_TRACE (hkwd_DISPLAY_RCM_ ## minor_hook, p2, p3, p4) ;	
	

#else 

 /***************************************************************************
	SYSTEM TRACE version (compile option ON)
  ***************************************************************************/

#define RCM_DEBUG_ENTRY_TRACE(minor_hook, p0, p1, p2, p3, p4)		\
	 								\
	RCM_TRACE (hkwd_DISPLAY_RCM_ ## minor_hook, p2, p3, p4) ;	\
	 								\
	TRCHKGT (HKWD (HKWD_DISPLAY_RCM_D, 				\
			      	hkwd_DISPLAY_RCM_ ## minor_hook, 0, 0),	\
		        	p0, p1, p2, p3, p4) ;		       \


#define RCM_DEBUG_EXIT_TRACE(minor_hook, p0, p1, p2, p3, p4)		\
	 								\
	RCM_TRACE (hkwd_DISPLAY_RCM_ ## minor_hook, p2, p3, p4) ;	\
	 								\
	TRCHKGT (HKWD (HKWD_DISPLAY_RCM_D, 				\
			      	hkwd_DISPLAY_RCM_ ## minor_hook, 0, 1),	\
		        	p0, p1, p2, p3, p4) ;		       \


#define RCM_DEBUG_TRACE(minor_hook, p0, p1, p2, p3, p4)		\
	 								\
	RCM_TRACE (hkwd_DISPLAY_RCM_ ## minor_hook, p2, p3, p4) ;	\
	 								\
	TRCHKGT (HKWD (HKWD_DISPLAY_RCM_D, 				\
			      	hkwd_DISPLAY_RCM_ ## minor_hook, 0, 2),	\
		        	p0, p1, p2, p3, p4) ;		       \

#endif






  /***************************************************************************
   ***************************************************************************

                                 TRACE IDs: 

   ***************************************************************************
  ***************************************************************************/

  /***************************************************************************
		    OLD TRACE IDs
  ***************************************************************************/

#define RCM_AIXGSC_ENTRY		0x000
#define RCM_AIXGSC_EXIT			0x001

#define RCM_CMD_LIST_ENTRY		0x002
#define RCM_CMD_LIST_EXIT		0x003

#define GP_FAULT	240	/* graphics process fault */
#define GP_DISPATCH	241	/* graphics process dispatch */
#define RCX_SWITCH_DONE 242	/* rendering context switch done */
#define GP_GIVEUP_TS	243	/* relinquish remaining time in slice */
#define AIXGSC		244
#define RCX_FAULT_LIST	245
#define RCX_SWITCH	246
#define HSC_END_SWITCH	247


  /***************************************************************************
	NEW TRACE IDs: (for RCM_DEBUG traces)
  ***************************************************************************/

#define hkwd_DISPLAY_RCM_FIND_WG   			( 0x069 << 4 )
#define hkwd_DISPLAY_RCM_FAULT_LIST			( 0x151 << 4 )



#endif /* _H_RCM_TRC */
