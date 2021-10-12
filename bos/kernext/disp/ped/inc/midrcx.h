/* @(#)38  	1.13.1.5  src/bos/kernext/disp/ped/inc/midrcx.h, peddd, bos411, 9428A410j 6/18/93 12:38:16 */
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
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_MIDRCX
#define _H_MIDRCX


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

/*---------------------------------------------------------------------------*
  Additional INCLUDES:
 *---------------------------------------------------------------------------*/
#include <sys/time.h>
#include <sys/rcm.h>
#include <hw_se_types.h>     
#include "mid.h"     


/*---------------------------------------------------------------------------*
  The following variables define the size(s) of a context on the adapter.
  Note that the size and content of a context is model dependent. 

  The indivdual components of a context are never transferred to or
  from the adapter.  The CPU merely swaps an entire context onto or off of 
  the adapter to alleviate adapter storage constraints.
 *---------------------------------------------------------------------------*/

#define    MAX_CTX_LEN_2D        40*1024
#define    MAX_CTX_LEN_3DM1     220*1024 
#define    MAX_CTX_LEN_3DM2      40*1024 
#define    MID_MAX_CTX_SIZE     MAX_CTX_LEN_3DM1


/*----------------------------------------------------------------------------*
   Structure for the headers for the hardware save chain
 *----------------------------------------------------------------------------*/

typedef struct mid_ctx_save_chain  {

        struct mid_ctx_save_chain  *pNext ;
        struct mid_ctx_save_chain  *pPrev ;

} mid_ctx_save_chain_t ;



/*----------------------------------------------------------------------------*
   Structure for mid_rcx_t.flags
 *----------------------------------------------------------------------------*/

typedef volatile struct mid_rcx_flags  {

	uint  adapter_knows_context : 1 ;    	/* context has been sent to the
                                                    adapter at least once */
	uint  context_on_adapter : 1 ; 		/* Ctx currently on adapter */

	uint  default_context  : 1 ;    	/* default context */ 
					           
	uint  terminate_context : 1 ;    	/* context termination in proc*/


	uint  context_space_allocated  : 1 ;    /* system allocated for this ctx
					           (as HW ctx save area) */
	uint  remove_context : 1 ;	   	/* ctx is to be removed */

	uint  geometry_changed : 1 ;	   	/* geometry changed since last
						   Set Window Parms */  
	uint  waiting_for_WID : 1 ;   	        /* ctx waiting for WID*/


	uint  waiting_for_FIFO_low_water_mark : 1 ;  /* app asleep, FIFO full*/

	uint  low_water_condition_pending : 1 ;  /* LW occurred during switch*/

	uint  sleeping_for_FIFO : 1 ;	   	/* HW occured w/DD using FIFO*/

	uint  domain_guarded_and_current : 1 ; 	/* domain logically guarded */



} mid_rcx_flags_t;




/******************************************************************************
   PED client clip structure --

        This is a part of the device dependent context structure.  It is
        defined as a separate structure to keep these fields grouped
        together.
 ******************************************************************************/

typedef struct _mid_CC_t {

        ulong           flags ;
#                       define    MID_CC_GPM_IN_USE             (1L<<0)
#                       define    MID_CC_CCR_IN_USE             (1L<<1)

        gBox            pClip1 ;        /* Reg of last single-box clipping */

        gPoint          pClipOrigin ;   /* Origin last complex client clip */
        gRegion         pClipComplex ;  /* Region last complex client clip */

} mid_CC_t ;




/******************************************************************************
   PED RCX:  (Here is the Ped device specific context data structure) 
 ******************************************************************************/

typedef struct mid_rcx {

	rcx     *pRcx ;           	/* pointer back to DI rcx */

	mid_rcx_type_t  type ;    	/* variable passed from GP indicating */
	                                /*  type of context (2D, 3DM1, 3DM2) */
	mid_rcx_flags_t	flags;          /* flags, see defs above */

	ushort		low_water_switch_count ;	/* see note below */
#	define	LOW_WATER_SWITCH_MAX	100  

	char 		*hw_rcx ;	/* pointer to saved graphics context */
	ulong		size ;		/* max size of context save area */
	struct		xmem xs;	/* cross memory desc. of hw_rcx */

	int     context_sleep_event_word ;	/* event wd for waiting for: */
						/*  guarded WIDs (double buf)*/
						/*  FIFO (after HI Water Int)*/
	struct  mid_rcx    *pNext_sleep  ;  /* next context on sleep list */

        mid_CC_t        client_clip  ;  /* Client Clip structure */

} mid_rcx_t;



/******************************************************************************
   Note on low_water_switch_count:  (and low water watchdog in general)
 ******************************************************************************/
/*
	The low_water_switch_count field is part of the watchdog for the 
	high water / low water function.  There is only one watchdog timer 
	which covers all the contexts.   It is restarted every context switch.  
	Therefore, it really provides a watchdog function only when the 
	domain is in a steady state (not switching for a long time).  
	When the domain is switching we use the following counter to 
	determine if we've waited for the adapter to post the low water 
	interrupt. */


/******************************************************************************
   Low Water Interrupt Watchdog structure
 ******************************************************************************/

typedef	struct _mid_watch_lowwater_int {
	struct watchdog 	dog;		/* watchdog timer structure  */
	char			*ddf; 		/* ddf pointer        */
} mid_watch_lowwater_int_t;



/******************************************************************************
   PI 

   The next structure defines the Ped pixel interpretation.  For our 
   mid-level graphics adapter, pixel interpretation consists of the 
   colormap and the swap buffer being used.   

   A typedef is defined here, which will be included as part of a couple      
   other structures. 
 ******************************************************************************/

/*----------------------------------------------------------------------------*
   The Pixel Interpretation structure (midPI):
   
   A outer union is used here to allow us to compare all (both) parts of the
   pixel interpretation in a single compare.  
   The inner union is to allow to initialize all the flags to zero and print
   them as a unit in midl_print. 
 *----------------------------------------------------------------------------*/
typedef union  _midPI_t 
{
	ulong       PI ;      /* this variable allows us to work with the 
	                             entire pixel int as a single quantity. */

	struct		      /* this portion defines the individual pieces */ 
	{
		ushort color ;      /* color map being used */
		ushort flags;      /* same as bit flags, below */
# 		      define   MID_USING_BACK_BUFFER   (1<<0)

	}  pieces ;
} midPI_t ;
 
#define   MID_NULL_COLOR     0xFFFF

/******************************************************************************

 	Usage of Flags                                      

  . To set the swapbuffer flag to "back buffer" without changing other flags: 
          midPI_t.pieces.flags.swapbuffer = MID_USING_BACK_BUFFER;   

  . To toggle the swapbuffer flag 
          midPI_t.pieces.flags.swapbuffer ^= 1;                      

 ******************************************************************************/





/******************************************************************************
   WA

   This structure would define the Ped device dependent extension to the RCM
   window attribute structure.  These structures would be allocated every   
   time a device independent window attribute (structure) is created.  
   This structure is linked to the device independent window attribute by 
   the pPriv pointer.

   Currently this structure is empty. 
 ******************************************************************************/

 



/******************************************************************************
   WG structures (actual WG is further below)

   The following structure defines the device dependent extension to
   the RCM window geometry structure.  One of these structures is allocated    
   every time a window geoemtry is created.  This structure is linked to 
   the device independent window geometry by the pPriv pointer.

   Several defines pertaining to window IDs are defined first. 
 ******************************************************************************/

/*----------------------------------------------------------------------------*
   WID defines

   This first typedef defines a window ID.  Window IDs use integers zero to
   MID_WID_MAX.  This is a constant which is the max across all adapters. 
   There is a field in midddf.h which contains the max for this adapter.  

   WIDs are shared between the device driver and X.  See com/inc/mid/mid.h
   for more WID defines (especially pertaining to the WID interface(s) between
   X and the device driver. 

   MID_NUM_DWA_WIDS is a constant defining the number of WIDs initially
   assigned to DWA windows. 

   WID MID_WID_MAX+1 is assigned as the null ID.  It is used for
   error handling.   The extra window ID allows us to make one extra   
   entry in the WID which corresponds to structures that do not have a WID.
   This extra entry will be used in the unlikely event that some code        
   incorrectly assumes a WID does exist.    
 *----------------------------------------------------------------------------*/
typedef int  mid_wid_t ; 

#define MID_WID_0           0              /* window ID 0 is, in fact 0 */
#define MID_WID_INCR        1              /* window IDs are 0, 1, ... 15 */
#define MID_NUM_WIDS        16             /* the highest WID for Ped */ 
#define MID_NUM_DWA_WIDS_INIT    MID_NUM_WIDS_START_DWA 

#define MAX_WIDS            MID_NUM_WIDS   /* another name for the same */
#define MID_WID_MAX         MID_NUM_WIDS-1 /* maximum WID is one less because
                                                      of 0-origin */ 
#define MID_WID_NULL        MID_WID_MAX+1  /* this WID means no WID exists for 
                                                   this WG, context, etc. */ 
#define MID_WID_ARRAY_SIZE  MID_NUM_WIDS+1 /* the WID array includes an entry
                                                   for the null WID, too  */ 



/*----------------------------------------------------------------------------*
   WG

   Here is the Ped DD window geometry. 
 *----------------------------------------------------------------------------*/
#define pNextWG  pNext
#define pPrevWG  pPrev

typedef struct  _midWG 
{
	rcmWG       *pWG ;		/* pointer back to dev ind WG */
	gRegion     *pRegion ;		/* pointer to current geometry region */

	struct _midWG   *pNext ;        /* List of WGs sharing WID */ 
	struct _midWG   *pPrev ;        /* same list, back pointer */ 

	mid_wid_t    wid ;		/* WID for this WG */

	midPI_t      pi ;		/* pixel interpretation for this WG */

				/*--------------------------------------------
				 During a context switch, WID writes must be
				  postponed (until the adapter can handle them). 
				 The following pointer forms a list of WGs for
				  which WID writes are delayed during a switch.
				 *--------------------------------------------*/
	struct _midWG   *pNext_ctx_sw_delay ;    

	ulong   wgflags ;		/* misc flags for window geoms */
#               define MID_WG_BOUND  (1L<<0)     	/* WG has been bound */
#               define MID_CM_UPDATED  (1L<<1)     	/* color changed */
#               define MID_WID_WRITE_DELAYED  (1L<<2)     /* see note above */

	/*----------------------------------------------------------------
	    getImage functionality:

	    Memory for what was last written to the user buffer indicating
	    which physical buffer is currently being displayed.
	  ----------------------------------------------------------------*/

	gscCurrDispBuff_t	CurrDispBuff;	/* current user value */
	rcm_uaddr_t		CDBaddr ;	/* 60-bit addr of user area */

}  midWG_t  ;


#define MID_LOWER_LEFT_ORIGIN  1 
#define MID_UPPER_LEFT_ORIGIN  2 
 

/******************************************************************************
   The following structures define the window ID lists.  They are grouped
   into one structure (mid_wid_data) here.  This structure is included 
   as part of the midddf structure.  The result is that the window ID data 
   gets allocated with the midddf data.
 ******************************************************************************/

/*----------------------------------------------------------------------------*
   WID LIST ENTRY

   This is the basic structure of the window ID lists.  There is one of these
   structures for each window ID.  One of the most important items in the 
   structure is the window ID state.  This field indicates how the ID is    
   being used and, hence, what list it is on. 

   The lists are the unused list, the shared list, the unshared list and the 
   the held list.  The unused list should be evident.  The unshared list is  
   the list of IDs being used by a single window (and not being held).  The  
   held list is the list of IDs that must be reserved due to double buffering.
   It is expected that the held list will always be short.    
   The shared list is the list of window IDs in use by multiple windows.
   The window geometry pointer on this list is actually the top of 
   a linked list of window geometries. 
 *----------------------------------------------------------------------------*/

typedef struct  _mid_wid_status     
{
	mid_wid_t         mid_wid ;
 
	int	state ;                     /* state of window ID */
#		define MID_WID_UNUSED    0  /* ID not in used, available */
#		define MID_WID_UNSHARED  1  /* ID in use by single window */
#		define MID_WID_SHARED    2  /* ID used by multiple windows */
#		define MID_WID_HELD      3  /* ID used for double buffering */
#		define MID_WID_GUARDED   3  /* ID used for double buffering */
#		define MID_WID_NULL_STATE   255  /* state for "extra WID" */
						/* This prevents a state match 
						 if a window has no real WID. */
 
	midPI_t pi ;              /* pixel interpretation is kept here, too */

	struct _mid_wid_status  *pPrev ;   /* ptr to previous wid entry */
	struct _mid_wid_status  *pNext ;   /* ptr to next wid entry */

	midWG_t  *pwidWG;     /* ptr to win geom using this WID or start of the
                              /* chain of WGs sharing this WID if > 1.  */
	int   	use_count ;          /* count of WGs using this WID */

	ushort  WID_flags ;		/* Miscellaneous flags:  */ 
#		define WID_DELAYED_ASSOC_UPDATE  1L<<0 

	ushort   WID_write_count ;	/* DEBUG USE:  count of times this WID
					   is written (determines which overlay
					   is written)  */ 
#if  0
		/* The following flag(s) used for DEBUG only (so far) */ 
        uint 	origin_defined:1 ;  

	gPoint   wid_origin ;		/* saved origin for DEBUG */
#endif 

}  mid_wid_status_t  ;




/*----------------------------------------------------------------------------*
   WID HEADER    

   This structure defines a list header, which consists of two WID list 
   entries -- one for top of the list and one for the bottom.  These list    
   entries are empty except for the chaining pointers.  This allows simpler  
   list management code to be written.  There is one of these headers for
   each of the WID lists.  
 *----------------------------------------------------------------------------*/
typedef struct  _mid_wid_list_header
{
	mid_wid_status_t    Top ; 
	mid_wid_status_t    Bottom ;

}  mid_wid_list_header_t  ;




/*----------------------------------------------------------------------------*
   ENTIRE WID STRUCTURE  

   And this structure puts all the WID pieces together into one big typedef. 
 *----------------------------------------------------------------------------*/
typedef struct _mid_wid_data  
{
	mid_wid_status_t       mid_wid_entry[MID_WID_ARRAY_SIZE] ;
	mid_wid_list_header_t  unused_list ; 
	mid_wid_list_header_t  unshared_list ; 
	mid_wid_list_header_t  shared_list ; 
	mid_wid_list_header_t  held_list ; 

}  mid_wid_data_t  ;


#define guarded_list held_list


/*----------------------------------------------------------------------------*
   WID DATA ADDRESSABILITY MACROS

   Finally, we define a couple of macros to establish addressability to
    the Ped window ID list:
 *----------------------------------------------------------------------------*/
#define MID_addr_WID_data(pDev)  (&(MID_addr_DDF(pDev)->mid_wid_data))

#define MID_addr_WID_list(pDev)  (&(MID_addr_WID_data(pDev)->mid_wid_entry[0]))
 


/*----------------------------------------------------------------------------*
   MISCELLANEOUS MACRO(S)
 *----------------------------------------------------------------------------*/

#define MID_PARM_CHK (parm, parm_name, dbgvar)                                \
#if MID_RCX_DBG                                                               \
    if (parm == NULL)                                                         \
    {                                                                         \
        BUGLPR (dbgvar, 0, ("%n was passed NULL!!\n",parm_name) );            \
        return (MID_PARM_ERROR) ;                                             \
    }                                                                         \
#endif 




#endif /* _H_MIDRCX */
