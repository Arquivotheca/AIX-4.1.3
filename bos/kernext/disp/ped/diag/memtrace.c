static char sccsid[] = "@(#)92  1.3  src/bos/kernext/disp/ped/diag/memtrace.c, peddd, bos411, 9428A410j 3/19/93 18:48:49";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: memtrace
 *		memtrace_init
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/***************************************************************************** 
                                  INCLUDES
 *****************************************************************************/

#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/rcm_win.h>
#include <sys/malloc.h>
#include <sys/intr.h>

#include "midddf.h"
#include "midrcx.h"
#include "midRC.h"

#include "mid_dd_trace.h"


extern long memtrace_init(midddf_t *);

/***************************************************************************** 
                                 Externals
 *****************************************************************************/
MID_MODULE (memtrace);




/****************************************************************************** 
                              
                             Start of Prologue  
  
   Function Name:    memtrace_init 
  
   Descriptive Name:  Initializes the memory area for the internal trace buffer.
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
  
     
     This routine allocates a trace buffer area and sets up the pointers to
     the trace buffer in the ddf area. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
  
  
    Dependencies:
  
  
 *--------------------------------------------------------------------------* 
  
    Linkage:
           Called internally from the Ped device driver initialization path.


 *--------------------------------------------------------------------------* 
  
    INPUT:  ddf address 
          
    OUTPUT:  none 
          
    RETURN CODES:  none         
          
                                 End of Prologue                                
 ******************************************************************************/

long memtrace_init  ( 
			midddf_t   *ddf  
		    ) 
{

/***************************************************************************** 
   Local variables: 
 *****************************************************************************/

#define  DD_MEM_TRACE_NAME_SIZE   (16)
#define  DD_MEM_TRACE_SIZE   (1024)

	dd_trace_entry_t 	*trace_buffer ;		/* ptr to trace buf */
 
	char  	trace_name[DD_MEM_TRACE_NAME_SIZE] = " rdh_ecart_dddim" ;  

	int  	i ; 
 



  	/***************************************************************** 
 	    First check if a buffer has laready been allocated.   
   	*****************************************************************/

  	if ( (ddf -> int_trace.top) != NULL )
  	{ 
  		return (1) ;
  	} 



  	/***************************************************************** 
 	    Next allocate space for the internal trace buffer.   
   	*****************************************************************/

  	trace_buffer = (dd_trace_entry_t *) 
			xmalloc ( (DD_MEM_TRACE_SIZE*sizeof(dd_trace_entry_t)),
				   4, pinned_heap);

  	if ( trace_buffer == NULL )
  	{
		BUGLPR(dbg_memtrace, 0,
			("** ERROR ** PED DD internal trace alloc failed\n\n"));
        	return (MID_ERROR) ;
  	}

	BUGLPR(dbg_memtrace, 1, (" ddf = 0x%8X , trace buffer = 0x%8X \n\n", 
					ddf, trace_buffer));



  	/***************************************************************** 
 	    Now set up the trace header (in ddf). 
   	*****************************************************************/

    	for ( i = 0 ; i < DD_MEM_TRACE_NAME_SIZE ; i++ )
	{
    	    ddf->int_trace.title[i] = trace_name[DD_MEM_TRACE_NAME_SIZE-1 -i] ; 
	}

    	ddf -> int_trace.top  = trace_buffer ; 
    	ddf -> int_trace.next = trace_buffer ; 
    	ddf -> int_trace.last = trace_buffer + (DD_MEM_TRACE_SIZE - 1) ;


	BUGLPR(dbg_memtrace, 2, (" top = %8X, next = %8X, last = %8X \n\n", 
    	ddf -> int_trace.top, ddf -> int_trace.next, ddf -> int_trace.last));
}








/****************************************************************************** 
                              
                             Start of Prologue  
  
   Function Name:    memtrace 
  
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

long memtrace  ( 
		midddf_t   *ddf ,
		ulong	hook,   
		ulong	parm1,   
		ulong	parm2,   
		ulong	parm3   
		) 

{

/***************************************************************************** 
   Local variables: 
 *****************************************************************************/

dd_trace_entry_t 	*trace_buffer ;		/* pointer to trace buffer */
 

    /*-------------------------------------------------------------------* 
           Make the trace entry 
    *--------------------------------------------------------------------*/

    trace_buffer = ddf -> int_trace.next ; 

    trace_buffer -> id  = hook ; 
    trace_buffer -> parm1  = parm1 ; 
    trace_buffer -> parm2  = parm2 ; 
    trace_buffer -> parm3  = parm3 ; 


    BUGLPR(dbg_memtrace, 2, (" hook = %8X, p1 = %8X, p2 = %8X, p3 = %8X \n", 
    	trace_buffer -> id, 
	trace_buffer -> parm1, trace_buffer -> parm2, trace_buffer -> parm3 ));


    /*-------------------------------------------------------------------* 
           Increment the trace entry
    *--------------------------------------------------------------------*/

    /* trace_buffer = trace_buffer + SIZEOF_TRACE_ENTRY ; */

    trace_buffer ++ ;

    if ( trace_buffer < ddf -> int_trace.last )
    {
    	ddf -> int_trace.next = trace_buffer ; 
    }
    else /* wrap */ 
    {
    	ddf -> int_trace.next = ddf -> int_trace.top ;
    }


    BUGLPR(dbg_memtrace, 1, (" top = %8X, next = %8X, last = %8X \n\n", 
    	ddf -> int_trace.top, ddf -> int_trace.next, ddf -> int_trace.last));
}
