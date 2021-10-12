/* @(#)41	1.12.1.4  src/bos/kernext/disp/ped/inc/midwidmac.h, peddd, bos411, 9428A410j 3/31/94 21:34:04 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: *		FIX_COLOR_PALETTE
 *		MID_ADD_WID_AFTER
 *		MID_ADD_WID_BEFORE
 *		MID_DELETE_WID_ENTRY
 *		MID_INIT_WID_ENTRY
 *		MID_RESET_WID_ENTRY
 *		MIN
 *		WRITE_WID_PLANES
 *		WRITE_WINDOW_PARMS
 *		sync_WID_requests
 *		sync_WID_requests_and_handle_bus
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




#ifndef _MIDWIDMAC_H
#define _MIDWIDMAC_H


#define WRITE_WINDOW_PARMS(ddf, midRCX, WID, reg, org, width, height)	\
{								\
	if (!(ddf->dom_flags & MID_CONTEXT_SWITCH_IN_PROCESS)) 	\
	{   							\
	    write_window_parms_PCB(ddf, midRCX, WID, reg, org, width, height);\
	}   							\
}


#define WRITE_WID_PLANES(pd, pWG, WID, reg, org)		\
{								\
	write_WID_planes_PCB_queue (ddf, pWG, WID, reg, org) ;	\
}

#define FIX_COLOR_PALETTE(ddf, WID, color, frame_buffer)	\
{								\
	fix_color_palette (ddf, WID, color, frame_buffer) ;	\
}



#ifndef MIN
#define MIN(x,y)        ((x) < (y)) ? (x) : (y)
#endif


        /*********************************************************************

          These macros manipulate the Window ID list.

          There is one macro for deleting an entry from the WID list and
          two for adding entries (one before a given entry and one after).

          The next two macros involve initializing (or re-initializing) a 
          WID entry. 

          NOTE:  These macros do not manipulate the window geometry chain!

         *********************************************************************/

#define MID_DELETE_WID_ENTRY(entry)                                            \
        entry -> pNext -> pPrev = entry -> pPrev ;                             \
        entry -> pPrev -> pNext = entry -> pNext

#define MID_ADD_WID_AFTER(top, entry)                                          \
        entry -> pNext = top -> pNext ;                                        \
        top -> pNext = entry ;                                                 \
                                                                               \
        entry -> pPrev = top ;                                                 \
        entry -> pNext -> pPrev = entry ;

#define MID_ADD_WID_BEFORE(entry,bottom)                                       \
        entry -> pPrev = bottom -> pPrev ;                                     \
        bottom -> pPrev = entry ;                                              \
                                                                               \
        entry -> pNext = bottom ;                                              \
        entry -> pPrev -> pNext = entry ;



#define MID_RESET_WID_ENTRY(entry)				    \
{ 								\
	entry-> state = MID_WID_UNUSED;  			\
	entry-> pi.pieces.color = MID_NULL_COLOR ; 		\
	entry-> pi.pieces.flags = 0 ; 				\
	entry-> use_count = 0 ;          			\
	entry-> pwidWG = NULL ;         			\
								\
} 



#define MID_INIT_WID_ENTRY(ddf,WID)					    \
    /********************************************************************   \
       Init the new UNUSED entry:					    \
     ********************************************************************/  \
									\
{ 									\
	mid_wid_status_t  	*new_WID_entry ; 			\
	mid_wid_status_t  	*last_unused ;	 			\
									\
	new_WID_entry = &(ddf->mid_wid_data.mid_wid_entry[WID]) ;	\
									\
	new_WID_entry-> mid_wid = WID ;      				\
	new_WID_entry-> state = MID_WID_UNUSED;  			\
	new_WID_entry-> pi.pieces.color = MID_NULL_COLOR ; 		\
	new_WID_entry-> pi.pieces.flags = 0 ; 				\
	new_WID_entry-> use_count = 0 ;          			\
	new_WID_entry-> pwidWG = NULL ;         			\
									\
	new_WID_entry-> WID_write_count = MID_NUM_WIDS - WID ;		\
									\
									\
    /********************************************************************   \
       Add to the UNUSED list:						    \
     ********************************************************************/  \
									\
        last_unused =  ddf->mid_wid_data.unused_list.Bottom.pPrev ;	\
									\
	MID_ADD_WID_AFTER (last_unused, new_WID_entry) ;		\
  									\
} 




#define sync_WID_requests(ddf)                                                 \
{                                                                              \
{ BUGXDEF(dbg_syncwid) ;                                                       \
{ BUGXDEF(trc_syncwid) ;                                                       \
{ BUGXDEF(perf_syncwid) ;                                                      \
                                                                               \
        /*---------------------------------------------------------------------\
           Check if there are any Set Window Parameters (WID assignements)     \
           still outstanding.                                                  \
         --------------------------------------------------------------------*/\
                                                                               \
        BUGLPR (dbg_syncwid, BUGNTA, ("sync WID request \n")) ;                \
        if ( ddf -> WID_change_flag == MID_WID_CHANGE_PEND )                   \
        {                                                                      \
                BUGLPR (dbg_syncwid,BUGNTA, ("change pending, read corr\n"));  \
                BUGLPR (dbg_syncwid,BUGNTA,("Corr = %4X, WID_change_flag = %X, \
                        \n", ddf -> mid_last_WID_change_corr,                  \
                        ddf -> WID_change_flag));                              \
                                                                              \
                                                                              \
                /* brkpoint ( ddf, ddf-> current_context_midRCX,              \
                                 ddf-> mid_last_WID_change_corr ) ; */        \
                                                                              \
                MID_DD_ENTRY_TRACE (syncwid, 2, DRAIN_FIFO, ddf,              \
                                ddf, ddf-> current_context_midRCX-> pRcx,     \
                                ddf-> current_context_midRCX-> type,          \
                                ddf-> mid_last_WID_change_corr ) ;            \
                                                                              \
                ddf -> WID_change_flag = MID_WID_NO_CHANGE_PEND ;             \
                                                                              \
                MID_POLL_CSB_EQ_MASK_TO (MID_CSB_SET_WINDOW_PARAMETERS_CORR,  \
                                 ((ddf -> mid_last_WID_change_corr) << 16),   \
                                  0xFFFF0000, 1000000 ) ;                     \
                                                                              \
                MID_DD_EXIT_TRACE (syncwid, 2, DRAIN_FIFO, ddf,               \
                                ddf, ddf-> current_context_midRCX-> pRcx,     \
                                ddf-> current_context_midRCX-> type,          \
                                ddf-> mid_last_WID_change_corr ) ;            \
        }                                                                     \
}}}}


#define sync_WID_requests_and_handle_bus(ddf)                                  \
        /*---------------------------------------------------------------------\
           same function as above, plus enable and disable the bus             \
         --------------------------------------------------------------------*/\
                                                                               \
        BUGLPR (dbg_midpixi, BUGNTA, ("sync WID request \n")) ;                \
        if ( ddf -> WID_change_flag == MID_WID_CHANGE_PEND )                   \
        {                                                                      \
                PIO_EXC_ON () ;                                                \
                                                                               \
                BUGLPR (dbg_midpixi,BUGNTA, ("change pending, read corr\n"));  \
                ddf -> WID_change_flag = MID_WID_NO_CHANGE_PEND ;              \
                                                                               \
                MID_POLL_CSB_EQ_MASK_TO (MID_CSB_SET_WINDOW_PARAMETERS_CORR,   \
                                 ((ddf -> mid_last_WID_change_corr) << 16),    \
                                  0xFFFF0000, 1000000 ) ;                      \
                PIO_EXC_OFF () ;                                               \
        }                                                                      \





/**********************************************************************
        MID_MARK_BUFFERS_SWAPPED

        This macro is now defunct.  The function is done inline in
        the swap buffers interrupt handler -- intrswap.c

 **********************************************************************/


#endif /* _MIDWIDMAC_H */
