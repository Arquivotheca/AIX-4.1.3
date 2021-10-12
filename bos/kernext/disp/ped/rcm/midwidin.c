static char sccsid[] = "@(#)72	1.7.1.4  src/bos/kernext/disp/ped/rcm/midwidin.c, peddd, bos411, 9428A410j 11/3/93 11:52:43";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: BUGLPR
 *		for
 *		if
 *		mid_WID_trace
 *		mid_add_WID
 *		mid_print_WG
 *		mid_print_WG_chain1568
 *		mid_print_region1667
 *		mid_wid_init
 *		mid_wid_reset
 *		midl_print
 *		print1
 *		shared_WID_trace
 *		sizeof
 *		while
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


/****************************************************************************** 
                                 INCLUDES                                       
 ******************************************************************************/
#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include "mid.h"
#include "midhwa.h"
#include "midddf.h"
#include "midrcx.h"

#include "midwidmac.h"
#include "mid_rcm_mac.h"
#include "mid_dd_trace.h"

/*************************************************************************** 
         EXTERNALS                  
 ***************************************************************************/
extern long mid_wid_init (
                           mid_wid_data_t * ) ;
  

extern long mid_add_WID ( midddf_t	*, 
			  mid_transfer_WID_t	 *) ;

extern long mid_wid_reset ( midddf_t	*) ;

extern midl_print (  
                    midddf_t   *,     
                    char       ) ;
  
MID_MODULE(midwid);




/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_wid_init   
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Initialize Window ID List
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Initialize the window ID list(s).  This consists of the following:
        . setting all the WID states to UNUSED,
        . chaining all the WIDs to the UNUSED list,
        . setting the other lists to be empty (top pointing to bottom
           and vice versa). 
  
      It is probably worth noting that the window ID lists are all doubly   
      linked lists, i.e. each entry has both a next and a previous (last)
      pointer.  In addition, each list has two static entries--the top and
      the bottom.  These entries are always on their respective lists,    
      therefore, each list always has at least two entries. 

      To determine if a list is empty, one may check either of the following:
      top.next == bottom or bottom.prev == top.  This is the state to which
      all lists except the UNUSED list are initialized. 

 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none yet
  
  
    Dependencies:
        none yet
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
           This routine is called sometime early in the initialization of
           every mid-level graphics adapter -- possibly from middef.c.     
           Note that there is one window ID list for each adapter.  
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the window ID list.  The structure is defined in 
               midrcx.h, although it is probably part of the larger midddf 
               structure. 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/



/***************************************************************************/
/*   Start of mid_wid_init routine  
/***************************************************************************/

long mid_wid_init (
                    mid_wid_data_t *pwid_list )  

{
/*************************************************************************** 
    Local Declarations              
 ***************************************************************************/

mid_wid_status_t  *pcur_ent, *pnext_ent ;
int  i ;

/*---------------------------------------------------------------------------* 
    Echo the input
 *---------------------------------------------------------------------------*/
    BUGLPR (dbg_midwid, 1, ("Top of mid_widin, @ of WID list = 0x%8X \n \n"
				" YO MAMA \n\n", pwid_list) );

    /********************************************************************

       INIT the UNUSED list:


       First chain all the window IDs onto the UNUSED list.           

     ********************************************************************/
	 
        pcur_ent = & (pwid_list -> unused_list.Top) ; /* top of unused list */
        pcur_ent -> state = MID_WID_UNUSED ;          /* mark it unused     */
        pcur_ent -> pPrev = NULL ;                    /* nothing before top */

	for (i = 0 ; i < MID_NUM_DWA_WIDS_INIT ; i++ )
	{
            /*------------------------------------------------------------*
               First get the address of the next entry
                (this is simply the next one in the array).
             *------------------------------------------------------------*/
            pnext_ent = & (pwid_list -> mid_wid_entry[i]) ;

            /*------------------------------------------------------------*
               Now chain the current and next entry.  
             *------------------------------------------------------------*/
            pcur_ent -> pNext = pnext_ent ;      /* chain next to current */
            pnext_ent -> pPrev = pcur_ent ;      /* chain current to next */

            /*------------------------------------------------------------*
               Now initialize the new entry: 
                . set the ID
                . set the state to UNUSED   
                . set the color to the NULL color   
                . set the use count to 0    
                . set the window geometry pointer to NULL
             *------------------------------------------------------------*/
            pnext_ent -> mid_wid = i ;            /* set WID to index     */
            pnext_ent -> state = MID_WID_UNUSED;  /* mark entry as unused */
            pnext_ent -> pi.pieces.color = MID_NULL_COLOR ; /* set NULL color */
            pnext_ent -> use_count = 0 ;          /* set use count to 0   */
            pnext_ent -> pwidWG = NULL ;          /* set WG ptr to NULL   */

            pnext_ent -> WID_write_count = MID_NUM_WIDS - i ;    

            pcur_ent = pnext_ent ;                /* & move to next entry */
	}

    /*-----------------------------------------------------------------------*
       Finally, finish the last entry, by chaining it to the bottom.
     *-----------------------------------------------------------------------*/
        pnext_ent = & (pwid_list -> unused_list.Bottom) ; 
        pcur_ent -> pNext = pnext_ent ;      /* chain last entry to Bottom   */
        pnext_ent -> pPrev = pcur_ent ;      /*  & chain bottom back to last */

    /*-----------------------------------------------------------------------*
       and marking the bottom entry as no next and unused (for no apparent
        reason). 
     *-----------------------------------------------------------------------*/
        pnext_ent -> state = MID_WID_UNUSED;   /* mark bottom entry unused   */
        pnext_ent -> pNext = NULL ;            /* nothing after the bottom   */ 





    /********************************************************************
       Now init the UNSHARED list: 

       This consists of: 
        . chaining the top of the list to the bottom, 
        . chaining the bottom of the list to the top, 
     ********************************************************************/
        pcur_ent = & (pwid_list -> unshared_list.Top) ;     /* @ of top */
        pnext_ent = & (pwid_list -> unshared_list.Bottom) ; /* @ of bot */

        pcur_ent -> pPrev = NULL ;              /* nothing before top   */
        pnext_ent -> pNext = NULL ;             /* nothing after bottom */ 

        pcur_ent -> pNext = pnext_ent ;         /* top's next is bottom */
        pnext_ent -> pPrev = pcur_ent ;         /* bottom's prev is top */ 

    /*------------------------------------------------------------------*
       Finally, mark both entrys as unused.  There does not seem to be a  
        requirement for this yet -- it just seems like a good idea.
     *------------------------------------------------------------------*/
        pcur_ent -> state = MID_WID_UNUSED; /* mark the top as unused   */
        pnext_ent -> state = MID_WID_UNUSED;/* mark bottom entry unused */




	 
    /********************************************************************
       Now init the SHARED list: 

       This consists of: 
        . chaining the top of the list to the bottom, 
        . chaining the bottom of the list to the top, 
     ********************************************************************/
        pcur_ent = & (pwid_list -> shared_list.Top) ;       /* @ of top */
        pnext_ent = & (pwid_list -> shared_list.Bottom) ;   /* @ of bot */

        pcur_ent -> pPrev = NULL ;              /* nothing before top   */
        pnext_ent -> pNext = NULL ;             /* nothing after bottom */ 

        pcur_ent -> pNext = pnext_ent ;         /* top's next is bottom */
        pnext_ent -> pPrev = pcur_ent ;         /* bottom's prev is top */ 


    /*------------------------------------------------------------------*
       Finally, mark both entrys as unused.  There does not seem to be a  
        requirement for this yet -- it just seems like a good idea.
     *------------------------------------------------------------------*/
        pcur_ent -> state = MID_WID_UNUSED; /* mark the top as unused   */
        pnext_ent -> state = MID_WID_UNUSED;/* mark bottom entry unused */
	 



    /********************************************************************
       Now init the HELD list: 

       This consists of: 
        . chaining the top of the list to the bottom, 
        . chaining the bottom of the list to the top, 
     ********************************************************************/
        pcur_ent = & (pwid_list -> held_list.Top) ;      /* @ of top */
        pnext_ent = & (pwid_list -> held_list.Bottom) ;  /* @ of bot */

        pcur_ent -> pPrev = NULL ;           /* nothing before top   */
        pnext_ent -> pNext = NULL ;          /* nothing after bottom */ 

        pcur_ent -> pNext = pnext_ent ;      /* top's next is bottom */
        pnext_ent -> pPrev = pcur_ent ;      /* bottom's prev is top */ 

    /*------------------------------------------------------------------*
       Finally, mark both entrys as unused.  There does not seem to be a  
        requirement for this yet -- it just seems like a good idea.
     *------------------------------------------------------------------*/
        pcur_ent -> state = MID_WID_UNUSED; /* mark the top as unused   */
        pnext_ent -> state = MID_WID_UNUSED;/* mark bottom entry unused */
	 



    /********************************************************************

       Finally init the "extra" WID entry.  

       This consists of: 
        . initializing its STATE to the NULL_STATE.   
        . initializing its color to the NULL_COLOR.   
        . initializing the pointers to NULL  

     ********************************************************************/

        pcur_ent = & (pwid_list -> mid_wid_entry[MID_WID_NULL]) ;
        pcur_ent -> state = MID_WID_NULL_STATE ;    /* prevents a state match */
        pcur_ent -> pi.pieces.color = MID_NULL_COLOR ; /* set NULL color */

        pcur_ent -> pNext = NULL  ;    	/* fails immediately if used  */
        pcur_ent -> pPrev = NULL  ;    	/* fails immediately if used  */


#if 0
    /********************************************************************

       Now init the counts 

        . number of WIDs
        . number of guarded WIDs

     ********************************************************************/

	ddf->num_DWA_WIDS = 0;
	ddf->mid_guarded_wid_count = 0;
#endif 


    BUGLPR (dbg_midwid, 1, ("End of mid_wid_init \n"));
    return 0 ;
}






/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_add_wid
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Add WID to (unused) Window ID List
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      X has realized that we need another WID.  Add it to the WID list.  

 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none yet
  
  
    Dependencies:
        none yet
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the ddf 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/



/***************************************************************************/
/*   Start of mid_add_WID routine  
/***************************************************************************/

long mid_add_WID ( 	midddf_t		*ddf, 
			mid_transfer_WID_t	*transfer) 
{

/*---------------------------------------------------------------------------* 
    Echo the input
 *---------------------------------------------------------------------------*/
    	BUGLPR (dbg_midwid, 1, ("Top of add_WID, WID = %X\n", transfer->WID) );

	MID_DD_ENTRY_TRACE (midwid, 1, ADD_NEW_WID, ddf, ddf, transfer->WID, 
				ddf->num_DWA_WIDS, ddf->num_DWA_contexts);



    /********************************************************************
       Init the new entry 
     ********************************************************************/

	MID_INIT_WID_ENTRY (ddf, transfer->WID) ;



    /********************************************************************
       Now increment the WID count 
     ********************************************************************/

	ddf->num_DWA_WIDS ++ ;

	MID_DD_EXIT_TRACE (midwid, 1,  ADD_NEW_WID, ddf, ddf,
				transfer->WID, ddf->num_DWA_WIDS, 0xF0);

    	BUGLPR (dbg_midwid, 1, ("End of mid_add_WID \n"));
    	return 0 ;
}








/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_wid_reset
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Reset WID table to all unused state
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      

 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none yet
  
  
    Dependencies:
        none yet
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the ddf 
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/



/***************************************************************************/
/*   Start of mid_wid_reset routine  
/***************************************************************************/

long mid_wid_reset ( 	midddf_t	*ddf)
{
mid_wid_status_t  *list_end; 
mid_wid_status_t  *current_entry ;
mid_wid_status_t  *next_entry ;

int  i ;


/*---------------------------------------------------------------------------* 
    Echo the input
 *---------------------------------------------------------------------------*/
    	BUGLPR (dbg_midwid, 1, ("Top of mid_wid_reset\n") );

	MID_DD_ENTRY_TRACE (midwid, 1, WID_INIT, ddf, ddf, 
				ddf->mid_guarded_WID_count,
				ddf->num_DWA_WIDS, ddf->num_DWA_contexts);


    /********************************************************************

       INIT the UNUSED list:

     ********************************************************************/
	 
        list_end = &(ddf->mid_wid_data.unused_list.Bottom) ; 

	for (	current_entry = ddf->mid_wid_data.unused_list.Top.pNext ; 
		current_entry != list_end ; 
		current_entry = current_entry->pNext ) 
	{
            /*------------------------------------------------------------*
               Reset each entry 
             *------------------------------------------------------------*/
             MID_RESET_WID_ENTRY (current_entry) ; 
	}



    /********************************************************************
       INIT the UNSHARED list (if required):
     ********************************************************************/
	 
        list_end = &(ddf->mid_wid_data.unshared_list.Bottom) ; 

	for (	current_entry = ddf->mid_wid_data.unshared_list.Top.pNext ; 
		current_entry != list_end ; 
		current_entry = next_entry ) 
	{
#		if 0
             MID_BREAK (WID_INIT,ddf,current_entry,current_entry->mid_wid,0,0);
#		endif

		next_entry = current_entry->pNext ; 
            /*------------------------------------------------------------*
               Reset each entry and move to the unused list
             *------------------------------------------------------------*/
             MID_INIT_WID_ENTRY (ddf, current_entry->mid_wid) ; 
	}

    	/*--------------------------------------------------------------------*
    	   Now rechain the top and the bottom
    	 *------------------------------------------------------------------*/
        current_entry = &(ddf->mid_wid_data.unshared_list.Top) ; 
        next_entry    = &(ddf->mid_wid_data.unshared_list.Bottom) ;

        current_entry-> pPrev = NULL ;		/* nothing before top   */
        current_entry-> pNext = next_entry  ;	/* top's next is bottom */

        next_entry-> pPrev = current_entry ;	/* bottom's prev is top */ 
        next_entry-> pNext = NULL ;             /* nothing after bottom */ 




    /********************************************************************
       INIT the SHARED list (if required):
     ********************************************************************/
	 
        list_end = &(ddf->mid_wid_data.shared_list.Bottom) ; 

	for (	current_entry = ddf->mid_wid_data.shared_list.Top.pNext ; 
		current_entry != list_end ; 
		current_entry = next_entry ) 
	{
#		if 0
             MID_BREAK (WID_INIT,ddf,current_entry,current_entry->mid_wid,0,0);
#		endif

		next_entry = current_entry->pNext ; 
            /*------------------------------------------------------------*
               Reset each entry and move to the unused list
             *------------------------------------------------------------*/
             MID_INIT_WID_ENTRY (ddf, current_entry->mid_wid) ; 
	}

    	/*--------------------------------------------------------------------*
    	   Now rechain the top and the bottom
     	 *------------------------------------------------------------------*/
        current_entry = &(ddf->mid_wid_data.shared_list.Top) ; 
        next_entry    = &(ddf->mid_wid_data.shared_list.Bottom) ;

        current_entry-> pPrev = NULL ;		/* nothing before top   */
        current_entry-> pNext = next_entry  ;	/* top's next is bottom */

        next_entry-> pPrev = current_entry ;	/* bottom's prev is top */ 
        next_entry-> pNext = NULL ;             /* nothing after bottom */ 






    /********************************************************************

       INIT the GUARDED list (if required):

     ********************************************************************/
	 
        list_end = &(ddf->mid_wid_data.guarded_list.Bottom) ; 

	for (	current_entry = ddf->mid_wid_data.guarded_list.Top.pNext ; 
		current_entry != list_end ; 
		current_entry = next_entry ) 
	{
#		if 0
             MID_BREAK (WID_INIT,ddf,current_entry,current_entry->mid_wid,0,0);
#		endif

		next_entry = current_entry->pNext ; 
            /*------------------------------------------------------------*
               Reset each entry and move to the unused list
             *------------------------------------------------------------*/
             MID_INIT_WID_ENTRY (ddf, current_entry->mid_wid) ; 
	}

    	/*--------------------------------------------------------------------*
    	   Now rechain the top and the bottom
     	*------------------------------------------------------------------*/
        current_entry = &(ddf->mid_wid_data.guarded_list.Top) ; 
        next_entry    = &(ddf->mid_wid_data.guarded_list.Bottom) ;

        current_entry-> pPrev = NULL ;		/* nothing before top   */
        current_entry-> pNext = next_entry  ;	/* top's next is bottom */

        next_entry-> pPrev = current_entry ;	/* bottom's prev is top */ 
        next_entry-> pNext = NULL ;             /* nothing after bottom */ 






    /********************************************************************
       Now reset the guarded WID count 
       DO NOT RESET the number of DWA WIDs !!
     ********************************************************************/

	ddf->mid_guarded_WID_count = 0 ;

	MID_DD_EXIT_TRACE (midwid, 1,  WID_INIT, ddf, ddf,
				ddf->mid_guarded_WID_count,
				ddf->num_DWA_WIDS, ddf->num_DWA_contexts);

    	BUGLPR (dbg_midwid, 1, ("End of mid_add_WID \n"));
    	return 0 ;
}







/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_WID_trace    
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Trace Window ID List (for debug)
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Trace the current state of the window ID list.  This includes 
        . a length and a string of 16 bytes for the UNSHARED list,    
        . a length and a string of 16 bytes for the GUARDED list,    
        . a length and a string of 16 bytes for the SHARED list and 
        . a length and a string of 16 bytes for the UNUSED list.    
 

 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none yet
  
  
    Dependencies:
        none yet
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the device dependent data structure (ddf),
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/


#include "mid_dd_trace.h"


/*************************************************************************** 
     Start of mid_WID_trace routine  
 ***************************************************************************/

mid_WID_trace ( midddf_t *ddf ) 
{

/*************************************************************************** 
    Local Declarations              
 ***************************************************************************/

mid_wid_data_t    *table ;
mid_wid_status_t  *entry ;

/*---------------------------------------------------------------------------* 
    Important trace buffer structures:
 *---------------------------------------------------------------------------*/
typedef struct _one_list
{ 
	ulong 	length ;
	char 	WID[16] ;

} one_list ; 


typedef struct _whole_list
{ 
	one_list	unshared_list ;
	one_list	guarded_list ;
	one_list	shared_list ;
	one_list	unused_list ;

} whole_list ; 

whole_list 	trace_list ;

int  i ;




/*---------------------------------------------------------------------------* 
    Echo the input
 *---------------------------------------------------------------------------*/
    BUGLPR (dbg_midwid, BUGNFO, ("Top of mid_WID_trace, ddf @ = 0x%X\n", ddf) );

    /********************************************************************
       get the WID list address 
     ********************************************************************/

    table = &(ddf -> mid_wid_data) ; 


    /********************************************************************
       Move the Unshared WIDs into the trace buffer:
     ********************************************************************/

    entry = table -> unshared_list.Top.pNext ;
    i = 0 ;  

    while ( entry != &( table -> unshared_list.Bottom) )  
    { 
    	trace_list.unshared_list.WID[i] = entry -> mid_wid ;
    	i += 1 ; 
    	entry = entry -> pNext ; 
    } 
    trace_list.unshared_list.length = i ;



#ifdef WID_LIST_DEBUG
    printf ("\nUNSHARED LIST: (length = %X)  ",trace_list.unshared_list.length);

    for ( i = 0 ; i < trace_list.unshared_list.length ; i ++ )
    { 
    	printf ("%X, ", trace_list.unshared_list.WID[i]) ; 
    } 
#endif




    /********************************************************************
       Move the GUARDED WIDs into the trace buffer:
     ********************************************************************/

    entry = table -> guarded_list.Top.pNext ;
    i = 0 ;  

    while ( entry != &( table -> guarded_list.Bottom) )  
    { 
    	trace_list.guarded_list.WID[i] = entry -> mid_wid ;
    	i += 1 ; 
    	entry = entry -> pNext ; 
    } 
    trace_list.guarded_list.length = i ;



#ifdef WID_LIST_DEBUG
    printf ("\nGUARDED LIST : (length = %X)  ",trace_list.guarded_list.length);

    for ( i = 0 ; i < trace_list.guarded_list.length ; i ++ )
    { 
    	printf ("%X, ", trace_list.guarded_list.WID[i]) ; 
    } 
#endif




    /********************************************************************
       Move the SHARED WIDs into the trace buffer:
     ********************************************************************/

    entry = table -> shared_list.Top.pNext ;
    i = 0 ;  

    while ( entry != &( table -> shared_list.Bottom) )  
    { 
    	trace_list.shared_list.WID[i] = entry -> mid_wid ;
    	i += 1 ; 
    	entry = entry -> pNext ; 
    } 
    trace_list.shared_list.length = i ;



#ifdef WID_LIST_DEBUG
    printf ("\nSHARED LIST  : (length = %X)  ",trace_list.shared_list.length);

    for ( i = 0 ; i < trace_list.shared_list.length ; i ++ )
    { 
    	printf ("%X, ", trace_list.shared_list.WID[i]) ; 
    } 
#endif




    /********************************************************************
       Move the UNUSED WIDs into the trace buffer:
     ********************************************************************/

    entry = table -> unused_list.Top.pNext ;
    i = 0 ;  

    while ( entry != &( table -> unused_list.Bottom) )  
    { 
    	trace_list.unused_list.WID[i] = entry -> mid_wid ;
    	i += 1 ; 
    	entry = entry -> pNext ; 
    } 
    trace_list.unused_list.length = i ;



#ifdef WID_LIST_DEBUG
    printf ("\nUNUSED LIST: (length = %X)  ",trace_list.unused_list.length);

    for ( i = 0 ; i < trace_list.unused_list.length ; i ++ )
    { 
    	printf ("%X, ", trace_list.unused_list.WID[i]) ; 
    } 
    printf ("\n ") ; 

#endif

    MID_DD_TRACE_DATA ( 2, WID_LIST, ddf, &(trace_list), sizeof(trace_list) );


    if (trace_list.shared_list.length > 0) 
    {
    	shared_WID_trace (ddf) ;
    }

} 












/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    shared_WID_trace    
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Trace WG chain on shared WID (for debug)
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      For each shared WID, trace the following: 
        . the WID itself, 
        . the first four (4) windows (midWGs) sharing that WID,      
        . and the last four (4) windows sharing that WID.  

      If less than four windows are sharing a WID, the other entries are
      zeroed out. 
 

 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none yet
  
  
    Dependencies:
        none yet
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the device dependent data structure (ddf),
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/


#include "mid_dd_trace.h"


/*************************************************************************** 
     Start of mid_WID_trace routine  
 ***************************************************************************/

shared_WID_trace ( midddf_t *ddf ) 
{

/*************************************************************************** 
    Local Declarations              
 ***************************************************************************/

mid_wid_data_t    *table ;
mid_wid_status_t  *entry ;

midWG_t   *midWG ;
midWG_t   *lastWG ;

/*---------------------------------------------------------------------------* 
    Important trace buffer structures:
 *---------------------------------------------------------------------------*/
typedef struct _one_list
{ 
	midWG_t *midWG[4] ;

} one_list ; 


typedef struct _whole_list
{ 
	ulong 		WID ; 
	one_list	first_four ; 
	one_list	last_four ;

} whole_list ; 

whole_list 	trace_list ;

int  i ;




/*---------------------------------------------------------------------------* 
    Echo the input
 *---------------------------------------------------------------------------*/
    BUGLPR (dbg_midwid, 3, ("Top of shared_WID_trace, ddf @ = 0x%X\n", ddf) );

    /********************************************************************
       get the WID list address 
     ********************************************************************/

    table = &(ddf -> mid_wid_data) ; 



    /********************************************************************
       Loop through all the shared WIDs 
     ********************************************************************/

    entry = table -> shared_list.Top.pNext ;

    while ( entry != &( table -> shared_list.Bottom) )  
    { 
    	trace_list.WID = (ulong) entry -> mid_wid ;
    	midWG = entry -> pwidWG ;

    	for ( i = 0 ; i < 4 ; i ++ )
    	{ 
    		trace_list.first_four.midWG[i] = midWG ;
    		if ( midWG != NULL )   	
    		{ 
			lastWG = midWG ;
			midWG = midWG -> pNext ;
    		} 
    	} 



    	while ( lastWG -> pNext != NULL ) 
    	{ 
    		lastWG = lastWG -> pNext ;
    	} 


    	for ( i = 0 ; i < 4 ; i ++ )
    	{ 
    		trace_list.last_four.midWG[3-i] = lastWG ;
    		if ( lastWG != NULL ) 
		 	lastWG = lastWG -> pPrev ;
    	} 


#ifdef  DEBUG_WG_CHAIN
        printf ("\n WG chain for WID %X ", trace_list.WID );
        printf ("\n first four:   ");

    	for ( i = 0 ; i < 4 ; i ++ )
    	{ 
    	 	printf ("%8X   ", trace_list.first_four.midWG[i] ) ; 
        } 

        printf ("\n last four:   ");

    	for ( i = 0 ; i < 4 ; i ++ )
    	{ 
    	 	printf ("%8X   ", trace_list.last_four.midWG[i] ) ; 
        } 
        printf ("\n \n ");
#endif


    	MID_DD_TRACE_DATA (2, WG_CHAIN,ddf, &(trace_list),sizeof(trace_list) );

    	entry = entry -> pNext ; 
    } 





} 

















/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    midl_print    
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Print Window ID List (for debug)
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Print the current state of the window ID list.  This includes 
        . headers for the lists and the list headers,
        . 16 lines for the list itself, 
        . 8 more lines for the header data, 
           totalling 30 lines.
 
      Each line is approximately 80 characters wide.

      Two forms are available: 
        . REQUEST code = 'LIST_ONLY' print just the list
                       (plus two lines of heading) -- 18 lines total,
        . REQUEST code = anything else will print the list (as above) 
                        plus the list header entries -- 30 lines total. 


 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none yet
  
  
    Dependencies:
        none yet
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the device dependenet data structure (ddf),
            . a request code
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/



/***************************************************************************/
/*   Start of midl_print routine  
/***************************************************************************/

midl_print (  
                    midddf_t *ddf, 
                    char      request ) 

{
/*************************************************************************** 
    Local Declarations              
 ***************************************************************************/

mid_wid_data_t    *table ;
mid_wid_status_t  *entry ;

int  i ;

/*---------------------------------------------------------------------------* 
    Echo the input
 *---------------------------------------------------------------------------*/
    BUGLPR (dbg_midwid, BUGNFO, ("Top of midl_print, ddf @ = 0x%x\n", ddf) );

    /********************************************************************
       Print the heading for the list: 
     ********************************************************************/

    table = &(ddf -> mid_wid_data) ; 


    /********************************************************************
       Print the UNUSED list 
     ********************************************************************/

    entry = table -> unused_list.Top.pNext ;
    printf ("UNUSED LIST:  ");

    if ( entry == &( table -> unused_list.Bottom) )  
    { 
    	printf ("EMPTY \n");
    } 
    else 
    { 
    	while ( entry != &( table -> unused_list.Bottom) )  
    	{ 
    		printf ("%X, ",entry -> mid_wid) ;
    		entry = entry -> pNext ; 
    	} 
    	printf ("\n");
    } 





    /********************************************************************
       Print the UNSHARED list 
     ********************************************************************/

    entry = table -> unshared_list.Top.pNext ;
    printf ("UNSHARED LIST:  ");

    if ( entry == &( table -> unshared_list.Bottom) )  
    { 
    	printf ("EMPTY \n");
    } 
    else 
    { 
    	while ( entry != &( table -> unshared_list.Bottom) )  
    	{ 
    		printf ("%X, ",entry -> mid_wid) ;
    		entry = entry -> pNext ; 
    	} 
    	printf ("\n");
    } 




    /********************************************************************
       Print the SHARED list 
     ********************************************************************/

    entry = table -> shared_list.Top.pNext ;
    printf ("SHARED LIST:  ");

    if ( entry == &( table -> shared_list.Bottom) )  
    { 
    	printf ("EMPTY \n");
    } 
    else 
    { 
    	while ( entry != &( table -> shared_list.Bottom) )  
    	{ 
    		printf ("%X, ",entry -> mid_wid) ;
    		entry = entry -> pNext ; 
    	} 
    	printf ("\n");
    } 





    /********************************************************************
       Print the GUARDED list 
     ********************************************************************/

    entry = table -> guarded_list.Top.pNext ;
    printf ("GUARDED LIST:  ");

    if ( entry == &( table -> guarded_list.Bottom) )  
    { 
    	printf ("EMPTY \n");
    } 
    else 
    { 
    	while ( entry != &( table -> guarded_list.Bottom) )  
    	{ 
    		printf ("%X, ",entry -> mid_wid) ;
    		entry = entry -> pNext ; 
    	} 
    	printf ("\n");
    } 












#if 0

    /********************************************************************
       Print the heading for the list: 
     ********************************************************************/

    printf ("---------------  WID LIST     ") ;
    printf ("starting address = 0x%8X   --------------\n", entry) ; 


    printf ("address  ID   state  PI   F/B    previous    next       ") ;
    printf ("WG chain   use\n") ; 


    /*-----------------------------------------------------------------------* 
       print the list
     *-----------------------------------------------------------------------*/
    entry = &(table -> mid_wid_entry [0]) ;

    for (i=0; i<MID_NUM_WIDS ; i++)
    {
        print1 ( &(table -> mid_wid_entry[i]) ) ;
    }




    /*-----------------------------------------------------------------------* 
       print the list headers if requested
     *-----------------------------------------------------------------------*/
    if (request != "LIST_ONLY")
    {
        /*-------------------------------------------------------------------* 
          UNUSED LIST HEADERs 
         *------------------------------------------------------------------*/
        entry = &(table -> unused_list.Top) ;
        printf (" UNUSED LIST HEADER (top, bottom),   address = %x \n",entry) ;

        print1 ( entry ) ;                      /* print data for top */
        entry = &(table -> unused_list.Bottom) ;
        print1 ( entry ) ;                      /* print data for bottom */
 


        /*-------------------------------------------------------------------* 
          UNSHARED LIST HEADERs 
         *------------------------------------------------------------------*/
        entry = &(table -> unshared_list.Top) ;
        printf (" UNSHARED LIST HEADER (top, bottom),   address = %x \n",entry);

        print1 ( entry ) ;                      /* print data for top */
        entry = &(table -> unshared_list.Bottom) ;
        print1 ( entry ) ;                      /* print data for bottom */
 


        /*-------------------------------------------------------------------* 
          SHARED LIST HEADERs 
         *------------------------------------------------------------------*/
        entry = &(table -> shared_list.Top) ;
        printf (" SHARED LIST HEADER (top, bottom),   address = %x \n",entry) ;

        print1 ( entry ) ;                      /* print data for top */
        entry = &(table -> shared_list.Bottom) ;
        print1 ( entry ) ;                      /* print data for bottom */
 


        /*-------------------------------------------------------------------* 
          HELD LIST HEADERs 
         *------------------------------------------------------------------*/
        entry = &(table -> held_list.Top) ;
        printf (" HELD LIST HEADER (top, bottom),   address = %x \n",entry) ;

        print1 ( entry ) ;                      /* print data for top */
        entry = &(table -> held_list.Bottom) ;
        print1 ( entry ) ;                      /* print data for bottom */
 
    }

#endif


    BUGLPR (dbg_midwid, BUGNFO, ("End of midl_print\n") );
/* return 0 ;*/
}

#if 0
/*************************************************************************** 
       Internal routine that prints one entry: 
 ***************************************************************************/
print1 ( mid_wid_status_t *entry) 
{
/*---------------------------------------------------------------------------* 
    Local Declarations              
 *---------------------------------------------------------------------------*/

ushort        wid ;        /* temp window ID */
ushort        state ;      /* temp state */
ushort        color ;      /* temp color */
ushort        fb ;         /* temp double buffering flag */
mid_wid_status_t *pN ;      /* temp pointer to next */
mid_wid_status_t *pP ;      /* temp pointer to previous */
rcmWG         *pWG ;        /* temp pointer to window geometry chain */
ushort        count ;      /* temp use_count */

/*---------------------------------------------------------------------------* 
    Now do something.
 *---------------------------------------------------------------------------*/

wid   = (ushort) entry -> mid_wid ;   /* get the window ID for this entry */
state = (ushort) entry -> state ;     /* get the state */
color = entry -> pi.pieces.color;     /* get the color for this entry */
fb    = entry -> pi.pieces.flags;     /* get flags for this entry */
pP    = entry -> pPrev ;              /* get the prev wid */
pN    = entry -> pNext ;              /* get the next wid */
pWG   = entry -> pwidWG ;             /* get the start of the WG chain */
count = (ushort) entry -> use_count ; /* get the window ID for this entry */



#if 0 
 /* printf ("%x  "  "%x "  "%x  "" %x %x   %x   %x   %x   %x\n", */
 /* printf ("%10.8x %4.4x %4.5x %4.4x %4.5x %9.8x %9.8x %9.8x %4.4x \n",*/
#endif
    printf ("%8X  %X  %4X  %4X %4X   %8X   %8X   %8X   %4X \n",
             entry, wid,  state, color, fb,  pP,   pN,  pWG,  count) ; 


/* return 0;*/

}

#endif




/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_print_WG 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Print Window Geometry (for debug)
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Print the current DI and DD window Geometry structures.  
 

 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none yet
  
  
    Dependencies:
        none yet
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to the device independenet WG data structure,
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/



/***************************************************************************/
/*   Start of midl_print routine  
/***************************************************************************/

mid_print_WG (  
                    rcmWG 	*pWG )
{
/*************************************************************************** 
    Local Declarations              
 ***************************************************************************/

ulong      	*temp_ptr ;
midWG_t      	*midWG ;


/*---------------------------------------------------------------------------* 
    Echo the input
 *---------------------------------------------------------------------------*/
    BUGLPR (dbg_midwid, BUGNFO, ("Top of mid_print_WG, WG @ = 0x%8X\n", pWG) );

    /********************************************************************
       Print the heading for the list: 
     ********************************************************************/

    printf ("\n-------------------------  Window Geometry  "
            "-------------------------\n" ) ; 

    /********************************************************************
       Init the pointers 
     ********************************************************************/

    temp_ptr = (ulong *)pWG ; 
    midWG = (midWG_t *)(pWG -> pPriv) ; 

    printf ("Device Independent Struc (0x%8X)       " 
	    "Device Dependent Struc (0x%8X)\n", temp_ptr, midWG ) ;

    printf ("Next WG:     0x%8X                     "
            "DI WG:       0x%8X \n", pWG->pNext, midWG ->pWG) ; 
    printf ("RCX list:    0x%8X                     "
            "Region:      0x%8X \n", pWG->pHead, midWG ->pRegion) ; 
    printf ("Process:     0x%8X                     "
            "Next ddWG:   0x%8X \n", pWG->pProc, midWG ->pNext) ; 
    printf ("gHandle:     0x%8X                     "
            "Prev ddWG:   0x%8X \n", pWG->wg.wg_handle, midWG ->pPrev) ; 

    printf ("Origin:       %4X %4X                     "
            "WID                   %X \n", 
		pWG->wg.winOrg.x, pWG->wg.winOrg.y, midWG -> wid) ; 
    printf ("W, heigth     %4X %4X                     "
            "PI            %4X %4X \n", 
		pWG->wg.width, pWG->wg.height, midWG -> pi.PI ) ; 

    printf ("wg flags not printed                        "
            "flags:         %8X \n", midWG -> wgflags) ; 


    printf ("Region:      0x%8X                   ""\n" , pWG->wg.pClip ) ;
    printf ("color hand:  0x%8X                   ""\n", pWG->wg.cm_handle ) ;
    printf ("dd WG @:     0x%8X                            ""\n", pWG->pPriv ) ;
    printf ("flags:         %8X                   ""\n\n", pWG->flags ) ;


/* return 0;*/


}



/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_print_WG_chain 
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Print Window Geometry Chain (for debug)
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Print a WG chain, given a WID entry 
 

 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none yet
  
  
    Dependencies:
        none yet
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to a WID entry
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/



/***************************************************************************/
/*   Start of midl_print routine  
/***************************************************************************/

mid_print_WG_chain (  
                    mid_wid_status_t 	*WID_entry )
{
/*************************************************************************** 
    Local Declarations              
 ***************************************************************************/

midWG_t      	*midWG ;


/*---------------------------------------------------------------------------* 
    Echo the input
 *---------------------------------------------------------------------------*/
    BUGLPR (dbg_midwid, BUGNFO,
	 ("Top of mid_print_WG_chain, WID = 0x%8X\n", WID_entry ) );

    /********************************************************************
       Print the heading for the list: 
     ********************************************************************/

    printf ("\n-------------------  Window Geometry Chain for WID %X "
            "-------------------\n", WID_entry -> mid_wid ) ; 

    printf ("  (mid) WG       Next       Previous \n") ; 
    printf ("  TOP            %8X\n", WID_entry -> pwidWG ) ; 



    /********************************************************************
       Print the WG chain
     ********************************************************************/

    for ( 
      		midWG = WID_entry -> pwidWG ;
      		midWG != NULL ;
      		midWG = midWG -> pNext 
        ) 
    { 
    printf ("  %8X       %8X   %8X \n",
		 midWG, midWG -> pNext, midWG -> pPrev ) ; 
    } 
    printf ("\n") ; 

/* return 0;*/


}



/****************************************************************************** 
                              
                               Start of Prologue  
  
   Function Name:    mid_print_region   
  
   Descriptive Name:  Mid-level Graphics Adapter, Device Dependent
                        Print Window Region 
      
 *--------------------------------------------------------------------------* 
  
   Function:                                      
      Print a window region, given the pointer to the region
 

 *--------------------------------------------------------------------------* 
  
    Restrictions:
        none yet
  
  
    Dependencies:
        none yet
  
  
 *--------------------------------------------------------------------------* 
  
   Linkage:
  
 *--------------------------------------------------------------------------* 
  
    INPUT:
            . a pointer to a window region
          
    OUTPUT:  none 
          
    RETURN CODES:  none        
          
          
          
                                 End of Prologue                                
 ******************************************************************************/



/***************************************************************************/
/*   Start of mid_print_region routine  
/***************************************************************************/

mid_print_region   (  
                    gRegion	*pRegion )
{
/*************************************************************************** 
    Local Declarations              
 ***************************************************************************/

long      	n,i ;
gBox      	(*pBox)[] ;


/*---------------------------------------------------------------------------* 
    Echo the input
 *---------------------------------------------------------------------------*/
    BUGLPR (dbg_midwid, BUGNFO,
	 ("Top of mid_print_region, region @ = 0x%8X\n", pRegion ) );

    /********************************************************************
       Print the heading for the list: 
     ********************************************************************/

    printf ("\n------------------------  Region %X "
            "------------------------\n", pRegion ) ; 

    if (pRegion == NULL)  
    { 
    	printf ("    CLIPPING REGION is NULL \n\n\n" ) ; 
    	return ;
    } 


    printf ("  + 00  size              %8X \n",pRegion->size     ) ; 
    printf ("  + 04  numBoxes          %8X ",pRegion->numBoxes ) ; 
    n = pRegion->numBoxes ; 
    i = 0 ; 

    if (n == 0)  
    { 
    	printf ("            NO VISIBLE REGIONS\n" ) ; 
    } 
    else 
    { 
    	printf ("            VISIBLE REGIONS:  \n" ) ; 
    } 


    printf ("  + 08  visible region @  %8X ",pRegion->pBox     ) ; 
    pBox = pRegion -> pBox ; 

    if (n == 0)  
    { 
    	printf ("\n" ) ; 
    } 
   else 
    { 
    	printf ("  -------->  Box 0:  %4X %4X\n", 
		(*pBox)[i].ul.x,  (*pBox)[i].ul.y);
    } 



    printf ("  + 0C  extents (ul)     %4X %4X ",
		pRegion->extents.ul.x, pRegion->extents.ul.y     ) ; 
    if (n == 0)  
    { 
    	printf ("\n" ) ; 
    } 
    else 
    { 
    	printf ("                     %4X %4X\n", 
		(*pBox)[i].lr.x,(*pBox)[i].lr.y);
    } 
    i += 1 ;




    printf ("  + 10          (lr)     %4X %4X ",
		pRegion->extents.lr.x, pRegion->extents.lr.y     ) ; 

    if (i >= pRegion->numBoxes)  
    { 
    	printf ("\n" ) ; 
    } 
   else 
    { 
    	printf ("             Box 1:  %4X %4X\n", 
		(*pBox)[i].ul.x,  (*pBox)[i].ul.y);
    	printf ("                                   " 
    	        "                     %4X %4X\n",
		 (*pBox)[i].lr.x,  (*pBox)[i].lr.y);
    	i += 1 ;


    	while (i < pRegion->numBoxes)  
    	{ 
    		printf ("                                             " 
    	       	"   Box %d:  %4X %4X\n", i, (*pBox)[i].ul.x, (*pBox)[i].ul.y);
    		printf ("                                             " 
    	        "           %4X %4X\n", (*pBox)[i].lr.x,  (*pBox)[i].lr.y);
    		i += 1 ;
    	} 

    } 


    printf ("\n") ; 

/* return 0;*/


}

