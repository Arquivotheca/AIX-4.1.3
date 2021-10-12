/* @(#)94  	1.2  src/bos/kernext/disp/ped/inc/midddftrc.h, peddd, bos411, 9428A410j 3/19/93 18:54:49 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: *		
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



#ifndef _H_MIDDDFTRC
#define _H_MIDDDFTRC




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

    These structures define the data associated with the internal device     
    driver traces. 

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */



/*----------------------------------------------------------------------------
			Trace Entry Data Structure
 *---------------------------------------------------------------------------*/

typedef struct dd_trace_entry {

        ulong	id ;
        ulong	parm1 ;
        ulong	parm2 ;
        ulong	parm3 ;

} dd_trace_entry_t ;




/*----------------------------------------------------------------------------* 
	   		Trace Header Data Structure
*----------------------------------------------------------------------------*/

typedef struct dd_trace_header {

	char			title[16] ;

	dd_trace_entry_t 	*top  ; 
	dd_trace_entry_t 	*next ; 
	dd_trace_entry_t 	*last ; 
	int		 	*DI_RCM_trace ; 

} dd_trace_header_t ;



#endif /* _H_MIDDDFTRC */
