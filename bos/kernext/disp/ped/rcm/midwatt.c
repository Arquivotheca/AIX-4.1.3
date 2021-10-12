static char sccsid[] = "@(#)66  1.5.1.5  src/bos/kernext/disp/ped/rcm/midwatt.c, peddd, bos41J, 9513A_all 3/24/95 11:00:06";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: mid_create_win_attr
 *              mid_delete_win_attr
 *              mid_update_win_attr
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*****************************************************************************
	                         INCLUDES
 *****************************************************************************/
#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/malloc.h>
#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include <gai/gai.h>

#include <hw_dd_model.h>        /* defines MID_DD -- must be first hw_*.h */
#include <hw_defer.h>
#include <hw_typdefs.h>
#include <hw_regs_k.h>
#include <hw_FIFkern.h>
#include <hw_PCBrms.h>
#include "mid.h"
#include "midhwa.h"
#include "midddf.h"

#include <mid_rcm_mac.h>
#include <mid_dd_trace.h>
#include "midRC.h"


/*****************************************************************************
	                         Externals
 *****************************************************************************/

extern long mid_create_win_attr (
	                          gscDev   *,
	                          rcmWA    *,
	                          create_win_attr *) ;

extern long mid_update_win_attr (
	                          gscDev   *,
	                          rcx      *,
	                          rcmWA    *,
	                          int        ) ;

extern long mid_delete_win_attr (
	                          gscDev   *,
	                          rcmWA    * ) ;

MID_MODULE (midwatt);
#define         dbg_middd       dbg_midwatt

/*****************************************************************************
	                    Internal Functions
 *****************************************************************************/

#define PDEV_PRIORITY (pGD->devHead.display->interrupt_data.intr.priority-1)

#define abs( x ) ( (x) < 0 ? -(x) : (x) )

gBool BoxesEqual ( gBox *,
	           gBox *) ;

gBool RegionsEqual ( gRegion *,
	             gRegion *) ;

ulong mid_write_GPM (
	              midddf_t *,
	              mid_rcx_t *,
	              gRegion * ,
	              rcmWG  *  ,
	              int       )  ;


/******************************************************************************

	                     Start of Prologue

   Function Name:    mid_create_win_attr

   Descriptive Name:  Create Window Attribute Structure

 *--------------------------------------------------------------------------*

   Function:

     This routine creates any necessary internal window attribute data
     structures required by the device driver for the mid-level graphics
     adapter (Pedernales and Lega).

     Currently, there is no device dependant data required, so there is
     no DD window attribute data structure (and very little to do here).

 *--------------------------------------------------------------------------*

    Restrictions:
	  none

    Dependencies:
	  none

 *--------------------------------------------------------------------------*

    Linkage:
	   This function is one of the GAI functions.  Hence it is available
	   as an aixgsc call.

	   The function is initiated by the window manager when a new
	   window is created.  The window manager performs a CreateWindow
	   which is handled by the device indenpendent RCM portion of the
	   RMS layer.  It is the device independent RCM code that actually
	   invokes this routine.

 *--------------------------------------------------------------------------*

    INPUT:  There are 3 input parameters to the general GAI create_win_attr
	     function:
	      . pointer to the virtual terminal structure
	      . pointer to the device independent window attribute structure
	      . pointer to a parameter list

    OUTPUT:  none

    RETURN CODES:  none


	                         End of Prologue
 ******************************************************************************/

long mid_create_win_attr (
	                   gscDev           *pGD,
	                   rcmWA            *pWA,
	                   create_win_attr  *arg )

{
    midddf_t *  ddf =   (midddf_t *) (pGD->devHead.display->free_area) ;

    MID_DD_ENTRY_TRACE (midwatt, 2, CREATE_WA, ddf, pGD, pWA, 0, pGD) ;

    BUGLPR(dbg_midwatt, BUGNFO+1, ("Top of mid_create_win_attr \n") ) ;
    BUGLPR(dbg_midwatt, BUGNFO+2, ("pGD = 0x%8X, pWA = 0x%8X, args = 0x%8X\n",
	                             pGD, pWA, arg)  );



	/*-------------------------------------------------------------------*
	    First validate the parameters.
	*--------------------------------------------------------------------*/
	if (pWA == NULL)
	{
	        BUGLPR(dbg_midwatt, 0,
	         ("No graphics device structure (pGD or pDev) pointer!!\n\n"));
	        return (MID_PARM_ERROR) ;
	}


	pWA->pPriv = NULL ;


    BUGLPR(dbg_midwatt, BUGNFO,
	                ("Bottom of create_win_attr, WA = 0x%8X \n\n", pWA) ) ;

    MID_DD_ENTRY_TRACE (midwatt, 1, CREATE_WA, ddf, pGD,pWA, pWA->pPriv, 0xF0);

    return 0;
}

/******************************************************************************

	                       Start of Prologue

   Function Name:    mid_update_win_attr

   Descriptive Name:  Update Window Attribute Structure

 *--------------------------------------------------------------------------*

   Function:

     There is no device dependent window attribute structure.  This
     routine is a NOP.


 *--------------------------------------------------------------------------*

    Restrictions:
	           none

    Dependencies:
	           none

 *--------------------------------------------------------------------------*


    Linkage:
	   This function is one of GAI functions.  Hence it is available
	   as an aixgsc call.

	   In addition, the "update window attribute" function is part of
	   the RMS software layer.  When a graphic process needs to update
	   a window attribute, it invokes UpdateWinAttr.  UpdateWinAttr
	   forms the interface to this routine.  UpdateWiAttr is also the
	   routine responsible for updating the device independent window
	   attribute data structure.  Note that this is done AFTER calling
	   this routine.


 *--------------------------------------------------------------------------*

    INPUT:  There are 4 input parameters to the general GAI update_win_attr
	     function:
	      . pointer to the virtual terminal structure
	      . pointer to the current rendering context for this window.
	      . pointer to a DI window attribute structure containing the
	                 changes to the window attribute structure.
	                 Note that this is a "normal" window attribute
	                 structure, however, only the changed fields are valid.
	      . a changemask -- This is a set of flags indicating which fields
	                 in the window attribute structure are to be changed.

	    NOTE:  none of these parameters are used.

    OUTPUT:  none

    RETURN CODES:  none


	                         End of Prologue
 ******************************************************************************/

long mid_update_win_attr (
	                   gscDev   *pGD,
	                   rcx      *pRCX,
	                   rcmWA    *pnewWA,
	                   int       changemask )

{
    midddf_t *  ddf =   (midddf_t *) (pGD->devHead.display->free_area) ;
    int         old_interrupt_priority; /*save old priority for enable*/

    MID_DD_ENTRY_TRACE (midwatt,2, UPDATE_WA,ddf,pGD, pnewWA,pRCX,changemask);

/*---------------------------------------------------------------------------*
   Echo the input parms.
 *---------------------------------------------------------------------------*/
    BUGLPR(dbg_midwatt, BUGNFO+1, ("Top of mid_update_win_attr \n") ) ;
    BUGLPR(dbg_midwatt, BUGNFO+2, ("pGD = 0x%8X, pRCX = 0x%8X\n", pGD, pRCX) );
    BUGLPR(dbg_midwatt, BUGNFO+2,
	             ("newWA = 0x%8X, mask = 0x%8X \n", pnewWA, changemask) );

    /*-----------------------------------------------------------------------*
       Update the client clip (if necessary)
     *-----------------------------------------------------------------------*/
    if (pRCX != NULL)
    {
	MID_GUARD_AND_MAKE_CUR(pGD, pRCX) ;
	old_interrupt_priority = i_disable(PDEV_PRIORITY);
	mid_set_client_clip (pGD, pRCX, pnewWA->wa.pRegion, pRCX->pWG, 0) ;
	i_enable(old_interrupt_priority);
	MID_UNGUARD_DOMAIN(pGD, pRCX) ;
    }


    BUGLPR(dbg_midwatt, BUGNFO,
	  ("EXIT 1:  mid_update_win_attr,  mask = x%4X \n\n", changemask) );

    MID_DD_ENTRY_TRACE (midwatt,1,UPDATE_WA,ddf,pGD, pnewWA, pRCX, changemask);

    return 0;
}


/******************************************************************************

	                       Start of Prologue

   Function Name:    mid_delete_win_attr

   Descriptive Name:  Delete Window Attribute Structure

 *--------------------------------------------------------------------------*

   Function:

     This routine performs all processing necessary to delete the window
     attribute with respect to the mid-level graphics device driver.

     This consists mainly of freeing the memory allocated for the DD window
     attribute structure.  However, there currently is no device dependent
     window attribute structure, therefore, we have rather little to do.

 *--------------------------------------------------------------------------*

    Restrictions:
	          none

    Dependencies:
	          none


 *--------------------------------------------------------------------------*

    Linkage:
	   This function is one of GAI functions.  Hence it is available
	   as an aixgsc call.
	   It is called by the window manager when a window is deleted.


 *--------------------------------------------------------------------------*

    INPUT:  There are 2 input parameters to the general GAI update_win_attr
	     function:
	      . pointer to the virtual terminal structure
	      . pointer to the window attribute structure

    OUTPUT:  none

    RETURN CODES:  none


	                         End of Prologue
 ******************************************************************************/

long mid_delete_win_attr (
	                   gscDev   *pGD,
	                   rcmWA    *pWA )

{
/*****************************************************************************
   Local variables:
 *****************************************************************************/

    midddf_t *  ddf =   (midddf_t *) (pGD->devHead.display->free_area) ;


	MID_DD_ENTRY_TRACE (midwatt, 2, DELETE_WA, ddf, pGD, pWA, 0, 0) ;

/*---------------------------------------------------------------------------*
   Echo the input parms.
 *---------------------------------------------------------------------------*/
	BUGLPR(dbg_midwatt, BUGNFO, ("Top of mid_delete_win_attr \n") );
	BUGLPR(dbg_midwatt, BUGNFO, ("pGD = 0x%8X, pWA = 0x%8X\n", pGD, pWA));




	BUGLPR(dbg_midwatt, BUGNFO, ("EXIT 1: mid_delete_win_attr \n\n") );

	MID_DD_ENTRY_TRACE (midwatt, 1, DELETE_WA, ddf, pGD, pWA, 0, 0xF0) ;

	return (MID_RC_OK) ;
}



#define GS_UL_EQUAL(gBox1ptr, gBox2ptr) \
	( (*((ulong *)(gBox1ptr))) == (*((ulong *)(gBox2ptr))) )

#define GS_LR_EQUAL(gBox1ptr, gBox2ptr) \
	( (*((ulong *)(((ulong)(gBox1ptr))+4)) ) == \
	  (*((ulong *)(((ulong)(gBox2ptr))+4)) ) )

#define GS_BOX_EQUAL(gBox1ptr, gBox2ptr)        \
	((GS_UL_EQUAL(gBox1ptr,gBox2ptr)) && (GS_LR_EQUAL(gBox1ptr,gBox2ptr)))


#define MID_DEBUG_RECTANGLE(module, priority, string, rect)     \
	BUGLPR(dbg_ ## module, priority, (string " 0x%8X: "             \
	"UL (%4X,%4X)  LR (%4X,%4X)\n", rect,   \
	 *(ushort *)(rect), *((ushort *)(((ulong)(rect))+2)),   \
	 *((ushort *)(((ulong)(rect))+4)), *((ushort *)(((ulong)(rect))+6)))) ;


/******************************************************************************

	                     Start of Prologue

   Function Name:    mid_set_client_clip

   Descriptive Name:  Routine to (re)compute the current client clip

 *--------------------------------------------------------------------------*

   Function:

     This routine recomputes the current client clip requirements from the
     passed input:
	. the client clip region (passed as a subset of the dev dependent
	        context structure)
	. the window geometry (passed as the dev dependent WG),

     This routine also performs the I/O necessary to establish this client
     clip region.  The gscDev structure pointer is also passed as an
     input parm, as it is required to perform the I/O.

     This routine also records (updates) the current state of client clip
     (for this context) in a sub-structure of the context.  This state
     information is used to help us perform the minimal I/O for subsequent
     client clip updates.

 *--------------------------------------------------------------------------*

    Restrictions:
	  none

    Dependencies:
	  none

 *--------------------------------------------------------------------------*

    Linkage:
	   This function is called wherever the client clip information must
	   be recomputed:
	        . when the context is bound
	        . when the window attributes (client clip area) is updated or
	        . during the first context switch (context state update time)
	           AFTER the window geometry is updated.  Note that this path
	           is different from the other two, in that we get control
	           in the interrupt environment, with no active context.
	           All I/O during this path must be through the PCB.

	           Note further that it would not help to handle this path
	           during the update window geometry call.  This request
	           is not synchronized with the 3D app, and hence we are
	           not guaranteed we can use the app's FIFO.

 *--------------------------------------------------------------------------*

    INPUT:  These are the input parameters:
	      . pointer to the graphics device (gscDev) structure
	      . pointer to the device independent context structure
	      . pointer to the window geometry region structure
	      . pointer to the client clip region structure
	      . A flag to indicate whether we are in the context switch path.

    OUTPUT:  none

    RETURN CODES:  none


	                         End of Prologue
 *****************************************************************************/

#define midddBool       uchar

long mid_set_client_clip (
	                   gscDev *             pGD,
	                   rcx *                pRCX,
	                   gRegion *            CC_Region,
	                   rcmWG *              pWG,
	                   uint                 use_PCB_IO)
{
	midddf_t *      ddf = (midddf_t *) (pGD->devHead.display->free_area) ;
	mid_rcx_t *     midRCX = (mid_rcx_t *) (pRCX->pData) ;

	midddBool       updates_required = FALSE ;

	midddBool       update_GPM_control = FALSE ;
	midddBool       update_GPM_required = FALSE ;
	midddBool       update_CCR = FALSE ;

	ulong   new_GPM_control_value = NULL;
	ulong   new_CCR_switch_value = NULL;

	MID3DRectangle  rect ;


	HWPDDFSetup;              /* declares variables and get bus access */


  /*------------------------------------------------------------------------*
	Local Macros
   *------------------------------------------------------------------------*/

#define RCX_CLIP_FLAGS          (midRCX->client_clip.flags)

#define GPM_CURRENTLY_IN_USE    \
	((RCX_CLIP_FLAGS & MID_CC_GPM_IN_USE) == MID_CC_GPM_IN_USE)

#define CCR_CURRENTLY_IN_USE    \
	((RCX_CLIP_FLAGS & MID_CC_CCR_IN_USE) == MID_CC_CCR_IN_USE)



  /*------------------------------------------------------------------------*
	Start of code
   *------------------------------------------------------------------------*/

	MID_DD_ENTRY_TRACE (midwatt, 1, SET_CLIENT_CLIP, ddf, pWG, pRCX,
	                        RCX_CLIP_FLAGS, CC_Region) ;

	BUGLPR(dbg_midwatt, 1, ("Top of mid_set_client_clip \n") ) ;
	BUGLPR(dbg_midwatt, 4, ("gscDev = 0x%8X, pRCX = 0x%8X, ddf = 0x%8X\n",
	                         pGD, pRCX, ddf) );
	BUGLPR(dbg_midwatt, 2, ("CC Reg = 0x%8X,  WG = 0x%8X\n\n",
	                         CC_Region, pWG));

	MID_DEBUG_REGION (midwatt, 4, CC_Region) ;
	MID_DEBUG_REGION (midwatt, 5, pWG->wg.pClip) ;


	/*-------------------------------------------------------------------*
	    First validate the parameters.
	*--------------------------------------------------------------------*/
	MID_ASSERT((pGD != NULL),  SET_CLIENT_CLIP, pGD,pRCX,CC_Region,pWG,1);
	MID_ASSERT((pRCX != NULL), SET_CLIENT_CLIP, pGD,pRCX,CC_Region,pWG,2);
	MID_ASSERT((pWG != NULL),  SET_CLIENT_CLIP, pGD,pRCX,CC_Region,pWG,3);


	/*-------------------------------------------------------------------*
	    If this is not a mod 1 context, exit.
	*--------------------------------------------------------------------*/
	if (midRCX->type != RCX_3DM1)
	{
	        return (0) ;
	}


	/*-------------------------------------------------------------------*
	    Check the new client clip region.  We will disable client clip if,
	        . the Region is NULL, or
	        . numBoxes is 0 (zero).
	*--------------------------------------------------------------------*/
	if ( (CC_Region == NULL) || (gREGION_NUM_RECTS(CC_Region) == 0) )
	{
	        if (CCR_CURRENTLY_IN_USE)
	        {
	                RCX_CLIP_FLAGS &= ~(MID_CC_CCR_IN_USE) ;
	                updates_required = TRUE ;
	                update_CCR = TRUE ;
	                new_CCR_switch_value = MID_CLIENT_CLIP_REGION_DISABLE ;
	        }

	        if (GPM_CURRENTLY_IN_USE)
	        {
	                RCX_CLIP_FLAGS &= ~(MID_CC_GPM_IN_USE) ;
	                updates_required = TRUE ;
	                update_GPM_control = TRUE ;
	                new_GPM_control_value = MID_DISABLE_GPM_CHECKING ;
	        }
	}
	else /* client clip IS required */

	/*-------------------------------------------------------------------*
	  OK, client clip is required.  Check on type (single box or complex).
	*--------------------------------------------------------------------*/
	{
	        if (gREGION_NUM_RECTS(CC_Region) == 1)
	        {
	                if (CCR_CURRENTLY_IN_USE)
	                {
	                        if (! (GS_BOX_EQUAL(&(CC_Region->pBox[0]),
	                              (&(midRCX->client_clip.pClip1)) )))
	                        {
	                                BUGLPR(dbg_midwatt,4,
	                                        ("Boxes not equal\n"));
	                                updates_required = TRUE ;
	                                update_CCR = TRUE ;
	                                new_CCR_switch_value =
	                                        MID_CLIENT_CLIP_REGION_ENABLE ;
	                        }
	                }
	                else /* CCR was not in use */
	                {
	                        RCX_CLIP_FLAGS |= MID_CC_CCR_IN_USE ;
	                        updates_required = TRUE ;
	                        update_CCR = TRUE ;
	                        new_CCR_switch_value =
	                                        MID_CLIENT_CLIP_REGION_ENABLE;

	                        if (GPM_CURRENTLY_IN_USE)
	                        {
	                                RCX_CLIP_FLAGS &= ~MID_CC_GPM_IN_USE;
	                                update_GPM_control = TRUE ;
	                                new_GPM_control_value =
	                                        MID_DISABLE_GPM_CHECKING;
	                        }
	                }
	        }
	        else
	        /*------------------------------------------------------------*
	            OK, COMPLEX client clip is required.
	        *-------------------------------------------------------------*/
	        {
	                if (CCR_CURRENTLY_IN_USE)
	                {
	                        RCX_CLIP_FLAGS &= ~(MID_CC_CCR_IN_USE) ;
	                        updates_required = TRUE ;
	                        update_CCR = TRUE ;
	                        new_CCR_switch_value =
	                                        MID_CLIENT_CLIP_REGION_DISABLE;
	                }

	                if (! GPM_CURRENTLY_IN_USE)
	                {
	                        RCX_CLIP_FLAGS |= (MID_CC_GPM_IN_USE) ;
	                        updates_required = TRUE ;
	                        update_GPM_control = TRUE ;
	                        new_GPM_control_value =
	                                        MID_ENABLE_GPM_CHECKING | 1 ;
	                }

	                if (! (RegionsEqual(CC_Region,
	                                &(midRCX->client_clip.pClipComplex) )))
	                {
	                        updates_required = TRUE ;
	                        update_GPM_required = TRUE ;
	                }
	        }
	}



	/*-------------------------------------------------------------------*
	    Record what we decided above
	*--------------------------------------------------------------------*/
	BUGLPR(dbg_midwatt, 2, ("      updates required = %d \n"
	"            update_CCR = %d,  new_CCR_switch_value = 0x%8X \n"
	"            update_GPM_control = %d,  new_GPM_control_value = 0x%8X\n"
	"            update_GPM_required = %d \n",
	updates_required, update_CCR, new_CCR_switch_value,
	update_GPM_control,new_GPM_control_value, update_GPM_required) );

	MID_DD_TRACE_PARMS (midwatt, 1, SET_CLIENT_CLIP, ddf, ddf,
	        ( (updates_required) << 24 | (update_CCR) << 16 |
	          (update_GPM_control) << 8 | (update_GPM_required) ),
	          new_CCR_switch_value, new_GPM_control_value) ;

	/*-------------------------------------------------------------------*
	    Check if any I/O is required
	*--------------------------------------------------------------------*/
	if (updates_required)
	{
	        /*------------------------------------------*
	           Prepare for I/O operations
	         *------------------------------------------*/
	        PIO_EXC_ON();             /* enables bus */


	        if (update_CCR)
	        {
	                BUGLPR(dbg_midwatt, 5, ("Write CCR control to 0x%8X\n",
	                                         new_CCR_switch_value) );

	                if (CCR_CURRENTLY_IN_USE)       /* save the new box */
	                {
	                    midRCX->client_clip.pClip1 = CC_Region->pBox[0] ;
	                }

	                MID_SHIP_ASSERT ( ((use_PCB_IO) == 0),
	                        SET_CLIENT_CLIP, ddf, pRCX, CC_Region,
	                        midRCX->client_clip.pClip1,
	                        new_CCR_switch_value) ;

	                MID_SetM1ClientClipRegion
	                        (midRCX->client_clip.pClip1,
	                           new_CCR_switch_value) ;
	        }


	        /*---------------------------------------------------------*
	           Write the GPM
	             This can be done independently of the GPM control,
	             since we can specify on the GPM write whether to
	             clip to the GPM or not.
	         *---------------------------------------------------------*/
	        if (update_GPM_required)
	        {
	                mid_write_GPM(ddf, midRCX, CC_Region, pWG,use_PCB_IO);
	        }


	        if (update_GPM_control)
	        {
	                BUGLPR(dbg_midwatt, 5, ("Write GPM control to 0x%8X\n",
	                                         new_GPM_control_value) );

	                MID_SHIP_ASSERT ( ((use_PCB_IO) == 0),
	                        SET_CLIENT_CLIP, ddf, pRCX, CC_Region,
	                        midRCX->client_clip.pClipComplex,
	                        new_GPM_control_value) ;

	                 MID_SetGPMCompareValue(new_GPM_control_value) ;
	        }


	        /*------------------------------------------*
	           Release the bus
	         *------------------------------------------*/

	        PIO_EXC_OFF();    /* disables bus */

	}

	BUGLPR(dbg_midwatt, 1, ("Bottom of mid_set_client_clip, \n\n") ) ;

	MID_DD_ENTRY_TRACE (midwatt, 2, SET_CLIENT_CLIP, ddf, pWG, pRCX,
	                                RCX_CLIP_FLAGS, 0xF0) ;
    return 0;
}



/******************************************************************************

	                     Start of Prologue

   Function Name:    BoxesEqual

   Descriptive Name:  Routine to compare two rectangles

 *--------------------------------------------------------------------------*

   Function:

     This routine compares two rectangles and returns TRUE if they compare and
     FALSE if they do not.

 *--------------------------------------------------------------------------*

    Restrictions:
	  none

    Dependencies:
	  none

 *--------------------------------------------------------------------------*

    Linkage:
	   This function is called

 *--------------------------------------------------------------------------*

    INPUT:  There are 2 input parameters:
	      . pointer to the first box
	      . pointer to the second box

    OUTPUT:  none

    RETURN CODES:  none


	                         End of Prologue
 ******************************************************************************/

#define GS_UL_EQUAL(gBox1, gBox2)       \
	(((ulong)(gBox1.ul)) == ((ulong)(gBox2.ul)))

#define GS_LR_EQUAL(gBox1, gBox2)       \
	(((ulong)(gBox1.lr)) == ((ulong)(gBox2.lr)))

gBool BoxesEqual ( gBox *       box1,
	           gBox *       box2)
{


  /*------------------------------------------------------------------------*
	Start of code
   *------------------------------------------------------------------------*/

	BUGLPR(dbg_midwatt, 2, ("Top of BoxesEqual\n") ) ;
	BUGLPR(dbg_midwatt, 3, ("box 1 = 0x%8X, box 2 = 0x%8X \n", box1,box2));
	BUGLPR(dbg_midwatt, 4, ("box 1: ll = (0x%4X, 0x%4X), "
	                              " ur = (0x%4X, 0x%4X) \n\n",
	    *((ushort *)((ulong)box1)),     *(((ushort *)((ulong)box1)+2)),
	    *(((ushort *)((ulong)box1)+4)), *(((ushort *)((ulong)box1)+6)) )) ;

	BUGLPR(dbg_midwatt, 4, ("box 2: ll = (0x%4X, 0x%4X), "
	                              " ur = (0x%4X, 0x%4X) \n\n",
	    *((ushort *)((ulong)box2)),     *(((ushort *)((ulong)box2)+2)),
	    *(((ushort *)((ulong)box2)+4)), *(((ushort *)((ulong)box2)+6)) )) ;


	/*-------------------------------------------------------------------*
	    If boxes equal, return TRUE
	*--------------------------------------------------------------------*/
	if ( (*((ulong *)(box1)) ==  *((ulong *)(box2)) ) &&
	     (*((ulong *)((ulong)box1+4))) ==  (*((ulong *)((ulong)box2+4))) )
	{
	        BUGLPR(dbg_midwatt, 2, ("Boxes EQUAL !!!\n") ) ;
	        return (TRUE) ;
	}
	else
	{
	        BUGLPR(dbg_midwatt, 2, ("Boxes not Equal\n") ) ;
	        return (FALSE) ;
	}
}





/******************************************************************************

	                     Start of Prologue

   Function Name:    RegionsEqual

   Descriptive Name:  Routine to compare two regions

 *--------------------------------------------------------------------------*

   Function:

     This routine compares two regions and returns TRUE if they compare and
     FALSE if they do not.

     The following is checked:
       If both regions do not exist, the compare is TRUE.
       If only one region exists, the compare is FALSE.
       If the number of boxes does not match, the compare is FALSE.
       If any of the boxes does not match, the compare is FALSE.

     Note that the extents are not checked.  (The extents are not saved
     in the internal Ped DD client clip region.)

 *--------------------------------------------------------------------------*

    Restrictions:
	  none


    Dependencies:
	        The comparisons of the points are done as ulongs for
	        performance reasons.  If gPoints are changed so they do
	        not fit into a ulong, this code must change.

 *--------------------------------------------------------------------------*

    Linkage:
	   This function is called

 *--------------------------------------------------------------------------*

    INPUT:  There are 2 input parameters:
	      . pointer to the first region
	      . pointer to the second region

    OUTPUT:  none

    RETURN CODES:  none


	                         End of Prologue
 ******************************************************************************/

gBool RegionsEqual ( gRegion *          reg1,
	             gRegion *          reg2)
{
    int         i;
    ulong *     box1;
    ulong *     box2;


	BUGLPR(dbg_midwatt, 1, ("Top of RegionsEqual \n") ) ;
	BUGLPR(dbg_midwatt, 21, ("Parms:   reg 1 = 0x%8X,  reg 2 = 0x%8X \n",
	        reg1, reg2) ) ;

	if ((!reg1 && reg2) || (reg1 && !reg2))
	        return FALSE;

	if (!reg1 && !reg2)
	{
	        BUGLPR(dbg_midwatt, 1, ("Regions ARE Equal !! \n\n") ) ;
	        return TRUE;
	}

	if (gREGION_NUM_RECTS(reg1) != gREGION_NUM_RECTS(reg2))
	{
	        BUGLPR(dbg_midwatt, 1, ("Regions NOT Equal (numBoxes)!\n\n"));
	        return FALSE;
	}

	box1 = (ulong *) (gREGION_RECTS(reg1)) ;
	box2 = (ulong *) (gREGION_RECTS(reg2)) ;

	for (i = gREGION_NUM_RECTS(reg1) * 2 ; i > 0 ; i--)
	{
	        BUGLPR(dbg_midwatt, 4, ("compare point %d: "
	                "point 1 = (%8X), point2 = (%8X)\n", i, *box1, *box2));

	        if ( (*(box1++)) != (*(box2++)) )
	        return FALSE;
	}

	BUGLPR(dbg_midwatt, 1, ("Regions ARE Equal !! \n\n") ) ;
	return TRUE;
}




/******************************************************************************

	                     Start of Prologue

   Function Name:    mid_write_GPM

   Descriptive Name:  Routine to write the GPM to a specific region

 *--------------------------------------------------------------------------*

   Function:

     This routine writes the GPM to a region corresponding to the region
     passed in the third parm.

     The GPM must be first be disabled before it is written, to prevent
     the GPM writes from being clipped by the old GPM.

     Then the entire window's GPM is cleared (to 0's).  Finally, we write
     the GPM (to 1's) for the clip client region (rectanlges).

     The GPM control is rewritten by the calling routine.

 *--------------------------------------------------------------------------*

    Restrictions:
	  none

    Dependencies:
	  We are relying on the microcode to clip the GPM to the current
	  window (geometry).  This is the only check we have for ensuring
	  that we do not write the GPM outside the window.

 *--------------------------------------------------------------------------*

    Linkage:
	   This function is called


    INPUT:  There are 3 input parameters:
	      . pointer to the device dependent (ddf) structure
	      . pointer to the device dependent context structure
	      . pointer to the client clip region structure

    OUTPUT:  none

    RETURN CODES:  none


	                         End of Prologue
 ******************************************************************************/

ulong mid_write_GPM (
	           midddf_t *           ddf,
	           mid_rcx_t *          midRCX,
	           gRegion *            pCCregion,
	           rcmWG *              pWG,
	           int                  use_PCB_IO )
{
#if 0
	rcmWG *                 pWG   =  (rcmWG *)(midRCX->pRcx->pWG);
#endif
	midWG_t *               midWG =  (midWG_t *)(pWG->pPriv);

	MIDDimensionRectangle   rect ;

	gPoint                  winOrgLL ;
	gBox *                  box ;

	gBox *                  boxfrom ;
	gBox *                  boxto ;
	int                     num_rects_new ;

	int                     i ;
	HWPDDFSetup;                    /* gain access to the adapter        */



	BUGLPR(dbg_midwatt, 1, ("Top of mid_write_GPM \n") ) ;
	BUGLPR(dbg_midwatt, 2, ("Input parms:   ddf = 0x%8X, midRCX = 0x%8X\n"
	   "                               pCCregion = 0x%8X,  pWG = 0x%8X \n",
	                         ddf, midRCX, pCCregion,pWG));


#if 0
	if (GPM_CURRENTLY_IN_USE)
	{
	        BUGLPR(dbg_midwatt, 2, ("Disable GPM \n") ) ;

	        MID_SetGPMCompareValue (MID_DISABLE_GPM_CHECKING) ;
	}
#endif


	winOrgLL.x  =
	rect.rulx = pWG->wg.winOrg.x ;
	winOrgLL.y  = pWG->wg.winOrg.y + pWG->wg.height - 1 ;
	rect.ruly = pWG->wg.winOrg.y ;
	rect.rwth = pWG->wg.width + 1;
	rect.rht  = pWG->wg.height + 1;

	MID_DEBUG_RECTANGLE (midwatt, 21, "window to clear  ", &rect) ;

	if (use_PCB_IO)
	{
	        BUGLPR(dbg_midwatt, 25,("Clear window via PCB, flag = %8X \n",
	                                 use_PCB_IO) );

	        MID_ClearControlPlanesPCB
	                ( (0<<MID_CCP_GPM_SHIFT), MID_CCP_GPM_MASK, (&(rect)),
	                   MID_CCP_WID_COMPARE_ON | MID_CCP_GPM_COMPARE_OFF,
	                        midWG->wid);
	}
	else
	{
	        BUGLPR(dbg_midwatt, 25,("Clear window via FIFO, flag = %8X \n",
	                                 use_PCB_IO) );

	        MID_ClearControlPlanesFIF
	                ( (0<<MID_CCP_GPM_SHIFT), MID_CCP_GPM_MASK, (&(rect)),
	                   MID_CCP_WID_COMPARE_ON | MID_CCP_GPM_COMPARE_OFF,
	                        midWG->wid, 0xBABE);
	}


	box = gREGION_RECTS(pCCregion) ;
	BUGLPR(dbg_midwatt, 4, ("numBoxes = %d,  addr = %8X \n",
	                         gREGION_NUM_RECTS(pCCregion), box));

	for (i = gREGION_NUM_RECTS(pCCregion); i > 0 ;  i-- )
	{
	        MID_ASSERT ( (box->lr.x >= box->ul.x), SET_CLIENT_CLIP, ddf,
	                        pCCregion, box->ul.x, box->lr.x, i) ;
	        MID_ASSERT ( (box->lr.y >= box->ul.y), SET_CLIENT_CLIP, ddf,
	                        pCCregion, box->ul.y, box->lr.y, i) ;

	        BUGLPR(dbg_midwatt, 25, ("Box %d,   addr = 0x%8X \n", i, box));
	        MID_DEBUG_RECTANGLE (midwatt, 21, "writing GPM rect ", box) ;

	        rect.rulx = winOrgLL.x + box->ul.x ;
	        rect.ruly = winOrgLL.y - ( (box->lr.y > box->ul.y) ? box->lr.y : box->ul.y ) ;

	        rect.rwth = abs(box->lr.x - box->ul.x) + 1 ;
	        rect.rht  = abs(box->lr.y - box->ul.y) + 1 ;

	        MID_DEBUG_RECTANGLE (midwatt, 21,"GPM rect written ",&(rect));



	        if (use_PCB_IO)
	        {
	        BUGLPR(dbg_midwatt, 25, ("Write rect via PCB, flag = %8X \n",
	                                 use_PCB_IO) );

	            MID_ClearControlPlanesPCB
	                ( (1<<MID_CCP_GPM_SHIFT), MID_CCP_GPM_MASK, (&(rect)),
	                    MID_CCP_WID_COMPARE_ON | MID_CCP_GPM_COMPARE_OFF,
	                        midWG->wid);
	        }
	        else
	        {
	        BUGLPR(dbg_midwatt, 25, ("Write rect via FIFO, flag = %8X \n",
	                                 use_PCB_IO) );

	            MID_ClearControlPlanesFIF
	                ( (1<<MID_CCP_GPM_SHIFT), MID_CCP_GPM_MASK, (&(rect)),
	                   MID_CCP_WID_COMPARE_ON | MID_CCP_GPM_COMPARE_OFF,
	                        midWG->wid, 0xBABE);
	        }


#if 0
	        MID_ClearBitPlanes ((MID_GPM_FILL | MID_CBP_CLIP_TO_WINDOW),
	                box->ul.x, box->lr.x,
	                (box->lr.x - box->ul.x)+1, (box->lr.y - box->ul.y)+1,
	                0, 1, 0 ) ;
#endif

	        box++ ;
	}




#define RCX_COMPLEX_CLIP                (&(midRCX->client_clip.pClipComplex))
	/*-------------------------------------------------------------------*
	    Now save the GPM current clip list
	      First check if the existing clip list is large enough
	      First deallocate any existing clip list,
	      Fnally copy the clip list
	 *-------------------------------------------------------------------*/
	MID_DEBUG_REGION (midwatt, 13, pCCregion) ;
	MID_DEBUG_REGION (midwatt, 14, RCX_COMPLEX_CLIP) ;


	num_rects_new = gREGION_NUM_RECTS(pCCregion) ;

	if (gREGION_SIZE(RCX_COMPLEX_CLIP) < num_rects_new)
	{
	        BUGLPR(dbg_midwatt, 5, ("Allocate clip list, numBoxes = %d\n",
	                                        num_rects_new));

	        /*-----------------------------------------------------------*
	           Save clip list too small
	                deallocate any existing clip list,
	                Then allocate a new clip list,
	         *-----------------------------------------------------------*/

	        xmfree ( (gREGION_BOXPTR (RCX_COMPLEX_CLIP)), pinned_heap) ;

	        RCX_COMPLEX_CLIP->pBox =   (gBox *)
	                (xmalloc(gREGION_SZOF(gREGION_NUM_RECTS(pCCregion)),
	                                4, pinned_heap) );

	        MID_SHIP_ASSERT ( ((RCX_COMPLEX_CLIP->pBox) != NULL),
	                  SET_CLIENT_CLIP, ddf, midRCX->pRcx, pCCregion, 0,0) ;

	        gREGION_SIZE(RCX_COMPLEX_CLIP) = num_rects_new ;
	}

	boxfrom = gREGION_RECTS(pCCregion) ;
	boxto = gREGION_RECTS(RCX_COMPLEX_CLIP) ;
	/* brkpoint (pCCregion, RCX_COMPLEX_CLIP) ;*/

	for (i = 0 ; i < num_rects_new ; i++)
	{
	        MID_DEBUG_RECTANGLE (midwatt, 21, "Box to (before): ", boxto);
	        MID_DEBUG_RECTANGLE (midwatt, 21, "Box from         ", boxfrom);

	        *boxto = *boxfrom ;
	        MID_DEBUG_RECTANGLE (midwatt, 21, "Box to (after):  ", boxto);

	        boxto++ ;
	        boxfrom++ ;
	}

	gREGION_NUM_RECTS(RCX_COMPLEX_CLIP) = num_rects_new ;
	/* The extents are not used */

	MID_DEBUG_REGION (midwatt, 12, RCX_COMPLEX_CLIP) ;

	BUGLPR(dbg_midwatt, 1, ("Bottom of mid_write_GPM \n\n") ) ;
}



