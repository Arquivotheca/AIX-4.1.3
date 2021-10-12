/* @(#)79  	1.2  src/bos/kernext/disp/ped/pedmacro/hw_dd_io_trace.h, pedmacro, bos411, 9428A410j 3/17/93 19:26:49 */
/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS: MID_DD_IO_TRACE
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

#ifndef	_H_DD_IO_TRACE
#define	_H_DD_IO_TRACE




/* *****************************************************************************

                              MACRO DEFINITION

   MACRO NAME:     MID_DD_IO_TRACE


   Function: 

    This macro provides an interface to the system trace facility for the      
    Mid-level graphics adapter device driver specifically for the low level
    I/O interface to trace what actually is sent to the device. 

  *-------------------------------------------------------------------------*

   Macro specification:
     MID_DD_IO_TRACE (type, address, length) ;


   Parameter definition:

     type - FIFO, PCB or HCR  
		(only types currently supported -- IND not yet done)

     address - Address of the data to be traced 

     length -  Length of data to be traced

  *-------------------------------------------------------------------------*

   Sample Invocation:

   MID_DD_IO_TRACE (FIFO, &(SE), sizeof(SE) ) ; 


 ******************************************************************************/

#ifdef  DEBUG
#define	MID_DD_IO_TRACE_SWITCH 
#endif 

#ifdef  MID_DD_IO_TRACE_SWITCH

#define  _DISPLAYTRACEFLAG 
#include <sys/disptrc.h>

#include <sys/syspest.h>
#include <sys/types.h>


#define  MID_DD_IO_TRACE_FIFO 		1
#define  MID_DD_IO_TRACE_PCB 		2
#define  MID_DD_IO_TRACE_HCR 		3


#define	MID_DD_IO_TRACE(type, address, length)				\
{									\
	trcgenkt(0, 							\
	    HKWD(HKWD_DISPLAY_RESERVED_519, MID_DD_IO_TRACE_ ## type, 0, 0),\
	    (MID_DD_IO_TRACE_ ## type)<<16, length, address) ;	\
}


#else  

#define  MID_DD_IO_TRACE(type, address, length)	
#endif  


#endif
