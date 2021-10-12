/* @(#)60       1.13  src/bos/kernext/disp/ped/inc/mid_pos.h, peddd, bos411, 9428A410j 4/8/94 16:11:01 */

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS:
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

#ifndef _H_MID_POS
#define _H_MID_POS

#include   <sys/mdio.h>
#include   <sys/iocc.h>
#include   "midddf.h"
#include   "hw_trace.h"
#include   "hw_regs_k.h"



/***************************************************************************/
/*									   */ 
/*                              Macro Definitions                          */
/*									   */ 
/*  This set provides an interface to the POS registers.                   */
/*									   */ 
/***************************************************************************/

#define   MID_RD_POS_0(value)     MID_RD_POS(0,value)
#define   MID_RD_POS_1(value)     MID_RD_POS(1,value)
#define   MID_RD_POS_2(value)     MID_RD_POS(2,value)
#define   MID_RD_POS_3(value)     MID_RD_POS(3,value)
#define   MID_RD_POS_4(value)     MID_RD_POS(4,value)
#define   MID_RD_POS_5(value)     MID_RD_POS(5,value)
#define   MID_RD_POS_6(value)     MID_RD_POS(6,value)
#define   MID_RD_POS_7(value)     MID_RD_POS(7,value)

#define   MID_WR_POS_2(value)     MID_WR_POS(2,value)
#define   MID_WR_POS_3(value)     MID_WR_POS(3,value)
#define   MID_WR_POS_4(value)     MID_WR_POS(4,value)
#define   MID_WR_POS_5(value)     MID_WR_POS(5,value)
#define   MID_WR_POS_6(value)     MID_WR_POS(6,value)
#define   MID_WR_POS_7(value)     MID_WR_POS(7,value)



/*************************************************************************** 
  									   
    The following macros are defined here: 
       . MID_POSREG 							   
  					
 ***************************************************************************/

#define   MID_POSREG(reg, card_slot)   POSREG (reg, card_slot) + 0x400000  





/*******************************************************************************
  									   
    The following macros are defined here: 
       . MID_FAST_IO					   
       . MID_FAST_IO_NO_IOCC					   
       . MID_SLOW_IO					   
       . MID_SLOW_IO_NO_IOCC
  					
   *------------------------------------------------------------------------* 

    The following macros set up the access time for Lega.  
    Since this is slower than the Ped access time, it should work for both.
    Code should be added sometime to do the correct setup depending on     
    machine type. 

    The macros that have 'NO_IOCC' tacked on at the end, do not manage the 
    the IOCC access.  (It is assumed the user does it in this case.)

    WAIT_STATE is used to delay (or not to delay) IOCC bus access from host.
    SPEED      is used to do 1 or 2 cycle accesses of the TI bus on the adapter.

 ******************************************************************************/

#define   MID_FAST_IO(ddf)						       \
{									       \
    volatile char   *ptr ; 						       \
    volatile unsigned long HWP;						       \
 									       \
    ddf->bim_slow_access_count-- ;  	 				       \
    if ((ddf->bim_slow_access_count) == 0) 				       \
    {                                      				       \
  	HWP = IOCC_ATT( BUS_ID, 0);					       \
 									       \
	ptr = HWP + MID_POSREG (2, ddf -> slot ) ; 			       \
 									       \
    	if ( ddf->hwconfig & MID_VPD_PPZ )      /* Then, this is a 3D Mid   */ \
    	{                                                                      \
    	    *ptr = (    						       \
    		MID_POS2_ENABLE     |		/* Enable Adapter           */ \
    		MID_POS2_FAIR       |           /* Enable Adapter Fairness  */ \
    		MID_POS2_STREAM     |           /* Enable Data Streaming    */ \
    		MID_POS2_WAIT_STATE |           /* 200 ns slave mode        */ \
    		MID_POS2_PARITY     |           /* Enable Parity Mode       */ \
    		MID_POS2_SPEED_30NS             /* 30 nsec access (LEGA/PED)*/ \
    	       ) ;							       \
    	}          /*  end if  */                                              \
    	else                                    /* ...else, it is a 2D Mid  */ \
    	{                                                                      \
    	    *ptr = (   							       \
    		MID_POS2_ENABLE     |		/* Enable Adapter           */ \
    		MID_POS2_FAIR       |           /* Enable Adapter Fairness  */ \
    		MID_POS2_STREAM     |           /* Enable Data Streaming    */ \
    		MID_POS2_WAIT_STATE |           /* 200 ns slave mode        */ \
    		MID_POS2_PARITY     |           /* Enable Parity Mode       */ \
    		MID_POS2_SPEED_70NS             /* 70 nsec access (LEGA/PED)*/ \
    	       ) ;							       \
    	}          /*  end else */                                             \
 	IOCC_DET( HWP );						       \
    }                                      				       \
}



#define   MID_FAST_IO_NO_IOCC(ddf)					      \
{									      \
    volatile char   *ptr ; 						      \
 									      \
    ddf->bim_slow_access_count-- ;  	 				      \
    if ((ddf->bim_slow_access_count) == 0) 				      \
    {                                      				      \
	ptr = ddf->HWP + MID_POSREG (2, ddf -> slot ); 		      	      \
 									      \
    	if ( ddf->hwconfig & MID_VPD_PPZ )      /* Then, this is a 3D Mid   */\
    	{                                                                     \
    	*ptr = (    							      \
    		MID_POS2_ENABLE     |		/* Enable Adapter           */\
    		MID_POS2_FAIR       |           /* Enable Adapter Fairness  */\
    		MID_POS2_STREAM     |           /* Enable Data Streaming    */\
    		MID_POS2_WAIT_STATE |           /* 200 ns slave mode        */\
    		MID_POS2_PARITY     |           /* Enable Parity Mode       */\
    		MID_POS2_SPEED_30NS             /* 30 nsec access (LEGA/PED)*/\
    	       ) ;							      \
    	}          /*  end if  */                                             \
    	else                                    /* ...else, it is a 2D Mid  */\
    	{                                                                     \
    	*ptr = (    							      \
    		MID_POS2_ENABLE     |		/* Enable Adapter           */\
    		MID_POS2_FAIR       |           /* Enable Adapter Fairness  */\
    		MID_POS2_STREAM     |           /* Enable Data Streaming    */\
    		MID_POS2_WAIT_STATE |           /* 200 ns slave mode        */\
    		MID_POS2_PARITY     |           /* Enable Parity Mode       */\
    		MID_POS2_SPEED_70NS             /* 70 nsec access (LEGA/PED)*/\
    	       ) ;						       	      \
    	}          /*  end else */                                            \
    }                            		       			      \
}


/* ---------------------------------------------------------------------------
      MID_SLOW_IO  
 * --------------------------------------------------------------------------*/

#define   MID_SLOW_IO(ddf) 						      \
{									      \
    volatile char   *ptr ; 						      \
    volatile unsigned long HWP;						      \
 									      \
 									      \
    ddf->bim_slow_access_count++ ;  	 				      \
    if ((ddf->bim_slow_access_count) == 1) 				      \
    {                                      				      \
  	HWP = IOCC_ATT( BUS_ID, 0);					      \
 									      \
	ptr = HWP + MID_POSREG (2, ddf -> slot ) ; 			      \
 									      \
    	*ptr = (    							      \
    		MID_POS2_ENABLE     |		/* Enable Adapter           */\
    		MID_POS2_FAIR       |           /* Enable Adapter Fairness  */\
    		MID_POS2_STREAM     |           /* Enable Data Streaming    */\
    		                                /* 300 ns slave mode        */\
    		MID_POS2_PARITY     |           /* Enable Parity Mode       */\
    		MID_POS2_SPEED                  /* 70 nsec access (LEGA/PED)*/\
    	       ) ;							      \
 									      \
 	IOCC_DET( HWP );	 					      \
    }                                      				      \
}



#define   MID_SLOW_IO_NO_IOCC(ddf) 					      \
{									      \
    volatile char   *ptr ; 						      \
 									      \
 									      \
    ddf->bim_slow_access_count++ ;  	 				      \
    if ((ddf->bim_slow_access_count) == 1) 				      \
    {                                      				      \
	ptr = ddf->HWP + MID_POSREG (2, ddf -> slot ); 		      	      \
 									      \
    	*ptr = (    							      \
    		MID_POS2_ENABLE     |		/* Enable Adapter           */\
    		MID_POS2_FAIR       |           /* Enable Adapter Fairness  */\
    		MID_POS2_STREAM     |           /* Enable Data Streaming    */\
    		                                /* 300 ns slave mode        */\
    		MID_POS2_PARITY     |           /* Enable Parity Mode       */\
    		MID_POS2_SPEED                  /* 70 nsec access (LEGA/PED)*/\
    	   	) ;							      \
    }                                      				      \
}


#endif /* _H_MID_POS */
