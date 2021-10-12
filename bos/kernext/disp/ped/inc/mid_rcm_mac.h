/* @(#)77	1.3  src/bos/kernext/disp/ped/inc/mid_rcm_mac.h, peddd, bos411, 9428A410j 6/18/93 11:02:51 */

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS:
 *		MID_ASSERT
 *		MID_BREAK
 *		MID_DD_INTR_PRIORITY
 *		MID_DD_I_DISABLE
 *		MID_DD_I_ENABLE
 *		MID_GUARD_AND_MAKE_CUR
 *		MID_UNGUARD_DOMAIN
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


#ifndef _H_MID_RCM_MAC
#define _H_MID_RCM_MAC

/*---------------------------------------------------------------------------*
  This file contains various macros to perform Ped device specific versions
  of RCM functions. 
 *---------------------------------------------------------------------------*/

#define MID_DD_INTR_PRIORITY(ddf)				\
	(ddf->pdev->devHead.display->interrupt_data.intr.priority -1) 


#define MID_DD_I_DISABLE(ddf)	i_disable (MID_DD_INTR_PRIORITY(ddf)) ; 

#define MID_DD_I_ENABLE(x)	i_enable (x) ; 




/*---------------------------------------------------------------------------*
  Miscellaneous definitions: 
    DI = device independent  
    DD = device dependent  
    RCM = rendering context manager
    RCX = rendering context 
    WG = window geometry     
    WA = window attributes     
    mid = MID = mid-level graphics adapter (Pedernales, Lega)
 *---------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
   MID_GUARD_AND_MAKE_CUR(pGD, pRCX) 

   Macro to guard the domain and make the context current.  This macro is used
   to ensure the correct context is on the adapter.  It is primarily useful
   for those functions which must write to the FIFO on behalf of a graphics 
   process. 
 *----------------------------------------------------------------------------*/

#define 	MID_GUARD_AND_MAKE_CUR(pGD, pRCX) 	\
{							\
       (*pGD-> devHead.pCom-> rcm_callback-> make_cur_and_guard_dom)(pRCX) ;\
       							\
       ((mid_rcx_t *)(pRCX->pData))->flags.domain_guarded_and_current = 1 ;   \
}



/*----------------------------------------------------------------------------*
   MID_UNGUARD_DOMAIN(pGD, pRCX) 

   Inverse macro for the above.  This macro undoes everything the above macro 
   does. 
 *----------------------------------------------------------------------------*/

#define 	MID_UNGUARD_DOMAIN(pGD, pRCX) 		\
{							\
       ((mid_rcx_t *)(pRCX->pData))->flags.domain_guarded_and_current = 0 ; \
   							\
     	(*pGD->devHead.pCom->rcm_callback->unguard_domain) (pRCX-> pDomain);\
}





/*----------------------------------------------------------------------------*
   MID_BREAK (module_ID, parm1, parm2, parm3, parm4, parm5) 

   Breakpoint when MID_BREAK is comiled ON.  Else a NOP.
   Right now this is at least when DEBUG or MID_ASSERT_SWITCH are ON.

 *----------------------------------------------------------------------------*/
#ifdef 	DEBUG
#define 	MID_ASSERT_SWITCH 	ON
#endif 	

#ifdef 	MID_ASSERT_SWITCH
#define 	MID_BREAK_SWITCH 	ON
#endif 	


#ifdef 	MID_BREAK_SWITCH
#define 	MID_BREAK(module, p1, p2, p3, p4, p5)  		\
{								\
       brkpoint (0xEEEE0000 | hkwd_DISPLAY_MIDDDF_ ## module, p1,p2,p3,p4,p5); \
	   							\
}

#else  
#define 	MID_BREAK(module, p1, p2, p3, p4, p5)  		
#endif



/*----------------------------------------------------------------------------*
   MID_ASSERT (condition, module_ID, parm1, parm2, parm3, parm4, parm5) 

   Tests the condition, if FALSE, we break. 
   Breakpoint when MID_ASSERT_SWITCH is comiled ON.  Else a NOP.
   Right now this is at least when DEBUG is ON. 

 *----------------------------------------------------------------------------*/

#ifdef 	MID_ASSERT_SWITCH
#define 	MID_ASSERT(condition, module, p1, p2, p3, p4, p5) 	\
{						\
	if (!(condition))  			\
	{					\
		MID_BREAK(module, p1, p2, p3, p4, p5) 	\
	}			\
}

#else 
#define 	MID_ASSERT(condition, module, p1, p2, p3, p4, p5)  		
#endif


/*----------------------------------------------------------------------------*
   MID_SHIP_ASSERT (condition, module_ID, parm1, parm2, parm3, parm4, parm5) 

   Tests the condition, if FALSE, we break. 

 *----------------------------------------------------------------------------*/

#define 	MID_SHIP_ASSERT(condition, module, p1, p2, p3, p4, p5) 	\
{						\
    if (!(condition))  			\
    {					\
       brkpoint (0xEEEE0000 | hkwd_DISPLAY_MIDDDF_ ## module, p1,p2,p3,p4,p5);\
    }					\
}


#endif /* _H_MID_RCM_MAC */
