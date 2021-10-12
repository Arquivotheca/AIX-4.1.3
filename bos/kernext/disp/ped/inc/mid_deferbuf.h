/* static char sccsid[] = "@(#)87       1.1.1.3  src/bos/kernext/disp/ped/inc/mid_deferbuf.h, peddd, bos411, 9428A410j 5/12/94 09:39:33"; */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: *		MID_DD_CHECK_DEFERBUF
 *		MID_RCM_FIFO_FLUSH
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



#ifndef _H_MID_LOCK_DB
#define _H_MID_LOCK_DB


#include <sys/lockl.h> 

#include <hw_dd_model.h>   
#include <hw_macros.h>   
#include <hw_regs_u.h>   


/*****************************************************************************
 
 *****************************************************************************/

#if 1
#define MID_RCM_FIFO_FLUSH(ddf)			 			   
#define MID_RCM_FIFO_FLUSH_code
#else

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

#if  1
#define MID_DD_CHECK_DEFERBUF(module, ddf)

#else  /* DEBUG */

#define MID_DD_CHECK_DEFERBUF(module, ddf) 				      \
{ 									      \
	/* -------------------------------------------------------*/	      \
	/* Ensure the defer buffer is empty.   	 		  */	      \
	/* -------------------------------------------------------*/	      \
       	if ( MID_DEFERBUF_LEN != 0 ) 					      \
	{ 								      \
		BUGLPR (dbg_ ## module, 0, ("DEFERBUF NOT EMPTY \n")) ;	      \
		brkpoint ( ddf, MID_DEFERBUF_HD, MID_DEFERBUF_LEN) ;          \
	} 								      \
}


#endif  /* DEBUG */

#endif /* _H_MID_LOCK_DB */
