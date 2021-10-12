/* static char sccsid[] = "@(#)56	1.5.1.4  src/bos/kernext/disp/ped/inc/hw_FIFkern.h, peddd, bos411, 9428A410j 3/31/94 21:33:42"; */

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS:
 *		MID_RCM_FIFO_FLUSH -- no longer used
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/************************************************************************/
/*                                                                      */
/*      PEDERNALES HW MACRO PROGRAMMING INTERFACE                       */
/*                                                                      */
/************************************************************************/

#ifndef _H_HW_FIFKERN
#define _H_HW_FIFKERN

#include <hw_dd_model.h>   
#include <hw_macros.h>   
#include <hw_regs_u.h>   
#include <hw_se_types.h>   
#include <hw_seops.h>   
#include <hw_FIFrms.h>   

#include "mid_dd_trace.h"   


/************************************************************************ 
 ************************************************************************ 

        KERNEL (Device Driver) FIFO Structure Elements  

       All the FIFO operations are now defined in hw_FIFrms.h. 

 ************************************************************************ 
 ************************************************************************/



/************************************************************************ 
 ************************************************************************ 

   KERNEL FIFO MACROS 

          Here we define a unique defer buffer flush for the device
          driver. 

 ************************************************************************ 
 ************************************************************************/

#if 0
#define MID_RCM_FIFO_FLUSH(ddf)			 			      \
       	mid_flush_rcm_fifo (ddf) ; 




#define MID_RCM_FIFO_FLUSH_code			 			      \
	{ 								      \
  	/*  caller must have invoked HWPDDFSetup;  */			      \
									      \
       	if ( MID_DEFERBUF_LEN != 0 ) 					      \
	{ 								      \
      		PIO_EXC_ON();             	/* enables bus */	      \
									      \
		MID_WR_FIFO (MID_DEFERBUF_HD, MID_DEFERBUF_LEN) ;	      \
		MID_DEFERBUF_PURGE					      \
									      \
  		PIO_EXC_OFF();    		/* disables bus */	      \
	} 								      \
	}

#endif


#endif /* _H_HW_FIFKERN */

