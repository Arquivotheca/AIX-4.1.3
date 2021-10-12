static char sccsid[] = "@(#)55  1.28.3.11  src/bos/kernext/disp/ped/ksr/midcfgvt.c, peddd, bos411, 9436D411a 9/6/94 18:34:23";
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: clean_DD_WG_structs
 *              clear_WID_GPM_Overlay_planes
 *              clear_box
 *              clear_frame_buffer
 *              mid_delay_with_bus_handling
 *              mid_init
 *              set_CRTC_values
 *              vttact
 *              vttdact
 *              vttsetm
 *              vttstct
 *              vttterm
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



#define KSR

#define Bool unsigned

#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/mdio.h>
#include <sys/devinfo.h>
#include <sys/file.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/uio.h>
#include <sys/iocc.h>
#include <sys/proc.h>
#include <fcntl.h>
#include <sys/aixfont.h>
#include <sys/syspest.h>
#include <sys/except.h>
#include "ddsmid.h"
#include "mid.h"
#include "midddf.h"
#include "mid_pos.h"            /* must precede midhwa.h */
#include "midhwa.h"
#include "bidefs.h"
#include "midksr.h"
#include "mid_ras.h"
#include "hw_dd_model.h"
#include "hw_macros.h"
#include "hw_FIFrms.h"         /* for clear bit planes */
#include "hw_FIF2dm1.h"
#include "hw_typdefs.h"
#include "hw_errno.h"
#include "hw_regs_u.h"
#include "hw_regs_k.h"
#include "hw_ind_mac.h"

#include "hw_se_types.h"  /* needed for ClearControlPlanes argument type   */
#include "hw_PCBkern.h"
#include "midddf.h"       /* needed for MIN and MAX screen values          */
#include "midRC.h"

#include "mid_dd_trace.h"
MID_MODULE ( midcfgvt );        /* defines trace variable                    */

BUGXDEF(dbg_middd);

#define MID_K_HOST_IO_CONTEXT_SWITCH_COMPLETED  MID_K_HOST_IO_DSP_SOFT_INTR0


/* ------------------------------------------------------------- */
/* mid_init:                                                     */
/*                                                               */
/*           ...                                                 */
/*           download micro-code                                 */
/*           create default rendering context (2d-0) (Note       */
/*           window parameters are also set. see dsp_newctx.c)   */
/*           clear frame buffer                                  */
/*           clear control planes (WIDs, overlay, GPM)           */
/*           load color table 0                                  */
/*           associate WID 0 with color table 0                  */
/*           set color mode, 8 bit rgb is used (not 24 bits )    */
/*           define default cursor                               */
/*           set colors, foreground green and background black   */
/*           ...                                                 */
/* ------------------------------------------------------------- */

extern long mid_init(pd)
struct phys_displays *pd;
{
	struct midddf *ddf = (struct midddf *) pd->free_area;

	struct vtmstruc *vp = (struct vtmstruc *)pd->visible_vt;

	struct ddsmid *dds = (struct ddsmid *)pd->odmdds;

	struct middata *ld ;                        /* ptr to local data*/

	gscDev  *pdev;

	int     rc = 0;                 /* return code                  */

	int     delay;                  /* delay waiting for context switch */
	
	gBox            extent_rect,    /* rectangle extents for...     */

	                visible_rect;   /* visible rectangle for...     */


	ushort flags, correlator;

	int i;

	HWPDDFSetup;                    /* gain access to ped H/W       */
	BUGLPR(dbg_midcfgvt, 1 , ("Entering mid_init.\n"));
	BUGLPR(dbg_midcfgvt, 1 , ("ddf ptr 0x%x.\n",ddf));
	BUGLPR(dbg_midcfgvt, 1 , ("vp ptr 0x%x.\n",vp));

	MID_DD_ENTRY_TRACE ( midcfgvt, 1, MID_INIT, ddf,
	        0,
	        ddf,
	        vp,
	        0 );

	PIO_EXC_ON();
	
	/*---------------------------------------------------*
	   initialize interrupts from the dsp commo register
	
	   First disable interrupts, then reset any that might
	   be left over at this point in time.  Finally, init
	   the mask register as we want it.
	 *---------------------------------------------------*/
	MID_WR_HOST_INTR (0);

#       define MID_ALL_BIM_STATUS       0x03FF
	MID_WR_HOST_STAT (MID_ALL_BIM_STATUS) ;

	ddf->host_intr_mask_shadow = MID_K_HOST_IO_WRITE_DSP_COMMO_REG ;
	MID_WR_HOST_INTR (MID_K_HOST_IO_WRITE_DSP_COMMO_REG);

	BUGLPR(dbg_midcfgvt, 3 , ("H/W ptr 0x%x.\n",ddf->HWP));
	BUGLPR(dbg_midcfgvt, 3 , ("macro H/W ptr 0x%x.\n",MID_BASE_PTR));

	/* turn trace off  */
	mid_trace_show_asc_off () ;

	/* download the microcode */
	BUGLPR(dbg_midcfgvt,1,("ucode file %s\n",&(dds->ucode_name)));

	rc = fp_open (dds->ucode_name, O_RDONLY, NULL, NULL,
	                        SYS_ADSPACE, &ddf->mid_ucode_file_ptr);
	if (rc != 0)
	{
	        BUGLPR(dbg_midcfgvt,0, ("microcode open failed.\n"));
	        return(rc);
	}

	BUGLPR(dbg_midcfgvt, 1 , ("ucode ptr 0x%x.\n",ddf->mid_ucode_file_ptr));

	/* download the microcode */
	rc = download(ddf);

	fp_close (ddf->mid_ucode_file_ptr);

	if (rc)
	{
	        BUGLPR(dbg_midcfgvt,0, ("downmc failed to load micro-code\n"));
	        return(rc);
	}

	/* turn trace on  */
	mid_trace_show_asc_on () ;


	/* wait a while for the ucode to get its data structures set */
	mid_delay(200000);



	/*****************************************************************
	   initialize several ddf fields
	 ****************************************************************/
	ddf -> max_color_IDs = 4 ;              /* this is a max value */

	ddf -> current_context_midRCX = NULL ;

	ddf -> mid_last_WID_change_corr = WID_CORR_INIT ;
	ddf -> WID_change_flag = MID_WID_NO_CHANGE_PEND ;



	/*****************************************************************
	   Re/Initialize the WID data structures
	 ****************************************************************/

	if (ddf->max_WIDs != MAX_WIDS)
	{
	        ddf->max_WIDs = MAX_WIDS ;      /* this is a max count */
	        mid_wid_init (&(ddf->mid_wid_data)) ;
	}
	else
	{
	        mid_wid_reset(ddf) ;
	}

	PIO_EXC_OFF();

	/*****************************************************************
	   (Re)Initialize the domain flags
	 ****************************************************************/

	ddf->dom_flags = 0;

	/*****************************************************************
	   set 2D context and CRTC values
	 ****************************************************************/

	if ( !rc )
	{
	   BUGLPR(dbg_midcfgvt, 2 , ("Calling mid_load_default_context\n"));
	   mid_load_default_context (ddf) ;

	   BUGLPR(dbg_midcfgvt, 2 , ("Waiting for context switch to complete\n"));
	   delay = 5000000;         /* Wait for 5 seconds */
	   while((ddf->dom_flags & MID_CONTEXT_SWITCH_IN_PROCESS) && delay--)
	       mid_delay(1);
	   if (! delay)
	       rc = -1;
	   else
	   {

	       PIO_EXC_ON();

	       BUGLPR(dbg_midcfgvt,2,("Clearing frame buffer & control planes\n"));
	       clear_frame_buffer(ddf,vp);
	       clear_WID_GPM_Overlay_planes(ddf);


	       /*--------------
	       Want to use indexed shading mode so that when we set
	       foreground and background colors, we pass the indices.
	       (Note: for 24 bit color mode, we have to pass the color
	            intensities instead)
	       ----------------*/
	
	       MID_Set2DColorMode(3);
	
	       /*--------------
	        Need to output the CRTC values to the adapter
	       ----------------*/
	       set_CRTC_values(ddf, dds);

	       PIO_EXC_OFF();
	   }

	}


	MID_DD_EXIT_TRACE ( midcfgvt, 1, MID_INIT, ddf,
	        0,
	        ddf,
	        vp,
	        rc );

	BUGLPR(dbg_midcfgvt,1, ("Leaving mid_init.\n"));
	return(rc);
}



/* -------------------------------------------------------------

   clean_DD_WG_structs:

   At hot key time, the data in the device dependent window geometry
   structures becomes stale: any previous correlation between a
   geometry and a pixel interpretation or a window ID has been
   lost on the hardware. To reflect this, the device dependent window
   geometry data structures need to be reset to the values they
   had when they were created.


 ------------------------------------------------------------- */

clean_DD_WG_structs(prcmWG, ddf)
	rcmWGPtr        prcmWG;         /* pointer to DI WG struct */
	midddf_t        *ddf ;

{
	midWG_t         *pmidWG;        /* pointer to DD WG struct */



	        /* set device dependent ptr */

	        for ( pmidWG = (midWG_t *)prcmWG->pPriv;
	              prcmWG != NULL;
	              prcmWG = prcmWG->pNext )
	        {
	                if ( pmidWG == NULL || pmidWG -> wid == MID_WID_NULL )
	                {
	                        continue;
	                }

	                mid_delete_WID (MID_NULL_REQUEST, ddf, prcmWG );

	                /* reset the data */
	                pmidWG->wid                = MID_WID_NULL;

	                /* Set all the bits to zero
	                   except the MID_WG_BOUND bit */

	                pmidWG->wgflags            &= MID_WG_BOUND;
	                pmidWG->pNext              = NULL;
	                pmidWG->pPrev              = NULL;
	                pmidWG->pNext_ctx_sw_delay = NULL;

	}

	return (0);
}



/* ------------------------------------------------------------- */
/*                                                               */
/* clear_WID_GPM_Overlay_planes:                                 */
/*                                                               */
/* clear window ID plane, overlay plane, and Global Plane Mask.  */
/* This is used mainly during intialization of the default       */
/* for KSR mode.                                                 */
/*                                                               */
/* ------------------------------------------------------------- */

clear_WID_GPM_Overlay_planes(ddf)
struct midddf *ddf;
{
	MIDDimensionRectangle   dim_rect;    /* rectangle coordinates for */
	                                     /* ClearControlPlanes.       */
	HWPDDFSetup;                         /* gain access to the adapter*/

	BUGLPR(dbg_midcfgvt,1, ("Entering clear_WID_GPM_Overlay_planes\n"));

	/* --------------------
	   Clear all control planes (WID, overlay, and GPM)

	   Args:  (cp_value, cp_mask, rectangle_dimension, wc_flag, wc_id)

	   1. cp_value: 0
	           window fill value : 0
	           overlay file value: 0
	           global plane mask fill value: 0

	   2. cp_mask: 0
	           allows update all planes above

	   3. wc_flag: 0
	           no window comparision so that updates will take place
	           indepentdently of value of window id planes for that pixel

	   4. wc_id: not applicable because of (3).  Pick 0 arbitrarily

	----------------------- */

	dim_rect.rulx = X_MIN;          /* upper-left X-coordinate         */
	dim_rect.ruly = Y_MIN;          /* upper-left Y-coordinate         */
	dim_rect.rwth = X_MAX + 1;      /* screen width                    */
	dim_rect.rht  = Y_MAX + 1;      /* screen height                   */

	/* PCB or FIFO version?  Any different ?  PCB verions is used */
	MID_ClearControlPlanesFIF(0, 0, (&dim_rect) , 0, 0, 0x1234);

	BUGLPR(dbg_midcfgvt,1, ("Leaving clear_WID_GPM_Overlay_planes\n"));
}



/* -------------------------------------------------------------- */
/* clear_frame_buffer:                                            */
/*                                                                */
/* This function is used to clear the entire frame buffer during  */
/* initialization of the default context for KSR mode.            */
/*                                                                */
/* -------------------------------------------------------------- */

clear_frame_buffer(ddf,vp)
struct midddf *ddf;
struct vtmstruc *vp;                   /* virtual terminal struct ptr  */
{
	ushort flags, correlator, uflags, fbflags;
	MIDDimensionRectangle  box;      /* the rectangle region of frame
	                                    buffer A to be filled */

	ulong fbfill;                    /* frame buffer fill value */
	HWPDDFSetup;                     /* gain access to the adapter*/

	BUGLPR(dbg_midcfgvt,1, ("Entering clear_frame_buffer\n"));
	BUGLPR(dbg_midcfgvt, 1 , ("vp ptr 0x%x.\n",vp));



	/*----------------------------------------------------------------
	  Associate the KSR window id 0 with a color table 0,
	  and set draw buffer to buffer "A".

	  "0"    - window ID 0

	  "0x20" - can be interpreted as follows:
	           2 == 0010
	                  ||___means Frame buffer A for "0", B for "1"
	                  |____"1" means check next bit for Frame buffer
	
	----------------------------------------------------------------*/

#if 0   /* Delay was commented out when MID_Assoc... was changed to FIFO ver */
	/* wait a while for the ucode to quiesce before Assoc */
	mid_delay(200000);
#endif

	MID_AssocOneWindowWithColorPaletteFIF( 0, 0x20 );


	fbfill = 0;              /* fill buffer with current background color */

	/* clear the entire frame buffer */

	box.rulx = X_MIN;
	box.ruly = Y_MIN;

	box.rwth = X_MAX + 1;
	box.rht  = Y_MAX + 1;
	clear_box(ddf,vp,&box,fbfill);

	BUGLPR(dbg_midcfgvt,1, ("Leaving clear_frame_buffer\n"));
}



/* -------------------------------------------------------------- */
/* set_CRTC_values:                                               */
/*                                                                */
/* This function sends the CRTC values to the adapter via the PCB.*/
/* It uses either 60 Hz or 77 Hz values dependent upon the state  */
/* of the ODM attribute "refresh_rate".
/* -------------------------------------------------------------- */



set_CRTC_values(ddf, dds)
struct midddf *ddf;
struct ddsmid *dds;
{
	HWPDDFSetup;                    /* gain access to ped H/W       */

	if( dds->refresh_rate == 77 )
	{
	        if( ddf->hwconfig & MID_VPD_PPZ )
	        {
	                MID_CRTCControl(LCIR_PARAM,
	                        HBET_PARAM_77, HBST_PARAM_77,
	                        P_HSET_PARAM_77, P_HSST_PARAM_77,
	                        VBET_PARAM_77, VBST_PARAM_77,
	                        VSET_PARAM_77, VSST_PARAM_77,
	                        CRTC_FLAGS_77);
	        }
	        else
	        {
	                MID_CRTCControl(LCIR_PARAM,
	                        HBET_PARAM_77, HBST_PARAM_77,
	                        L_HSET_PARAM_77, L_HSST_PARAM_77,
	                        VBET_PARAM_77, VBST_PARAM_77,
	                        VSET_PARAM_77, VSST_PARAM_77,
	                        CRTC_FLAGS_77);
	        }
	}
	else
	{
	        if( ddf->hwconfig & MID_VPD_PPZ )
	        {
	                MID_CRTCControl(LCIR_PARAM,
	                        HBET_PARAM_60, HBST_PARAM_60,
	                        P_HSET_PARAM_60, P_HSST_PARAM_60,
	                        VBET_PARAM_60, VBST_PARAM_60,
	                        VSET_PARAM_60, VSST_PARAM_60,
	                        CRTC_FLAGS_60);
	        }
	        else
	        {
	                MID_CRTCControl(LCIR_PARAM,
	                        HBET_PARAM_60, HBST_PARAM_60,
	                        L_HSET_PARAM_60, L_HSST_PARAM_60,
	                        VBET_PARAM_60, VBST_PARAM_60,
	                        VSET_PARAM_60, VSST_PARAM_60,
	                        CRTC_FLAGS_60);
	        }
	}
}



/* -------------------------------------------------------------- */
/* clear_box:                                                     */
/*                                                                */
/* This function fills a  rectangular region of the current       */
/* drawing buffer (A) with the specified fill value.  When       */
/* backgound color is used, the effect is that we are clearing    */
/* a rectangular box on screen.                                   */
/*                                                                */
/* -------------------------------------------------------------- */

clear_box(ddf,vp,box,fill_value)
struct midddf *ddf;
struct vtmstruc *vp;                   /* virtual terminal struct ptr  */
MIDDimensionRectangle * box;            /* the rectangle region to be filled */
ulong fill_value;
{
	ushort flags;
	struct middata  *ld;            /* ptr to local data area       */
	
	HWPDDFSetup;                    /* gain access to ped H/W       */


	/* Note:
	        clear_box can be called from mid_init first time.
	        If so, vp is zero.
	*/

	BUGLPR(dbg_midcfgvt, 1 , ("vp ptr 0x%x.\n",vp));
	if (vp != 0)
	        ld = (struct middata *)vp->vttld;

	
	BUGLPR(dbg_midcfgvt,1, ("Entering clear_box\n"));
	BUGLPR(dbg_midcfgvt,1, ("ddf = 0x%x.\n",ddf));
	BUGLPR(dbg_midcfgvt,1, ("box = 0x%x.\n",box));
	BUGLPR(dbg_midcfgvt,1,
	         ("box's contents  = ulx %d uly %d\n",box->rulx,box->ruly));
	BUGLPR(dbg_midcfgvt,1,
	         ("wth %d ht %d.\n",box->rwth,box->rht));
	BUGLPR(dbg_midcfgvt,1, ("fill value = %d.\n",fill_value));



	/* --------------------
	   Clear frame buffer:

	   Arg: flags, x coordinate of upper left hand corner of rectangle,
	        y coordinate of upper left hand corner of rectangle,
	        width in pixels of the rectangle to be filled, height in pixels
	        of the rectangle to be filled, frame buffer fill color,
	        overlay planes fill value, z buffer fill value

	   - either 8 or 24 bit fill, depending on the hardware
	   - no overlay planes fill
	   - no global planes mask fill
	   - no Z buffer fill

	   - frame buffer fill value will be the same as the background
	     color which is 0

	   - overlay planes and zbuffer fill values are not applicable.  0
	     is used arbitrarily ??

	----------------------- */

	flags = EIGHT_BIT_FILL;
	BUGLPR(dbg_midcfgvt,3, ("8 bit fill\n"));

	if (ddf->hwconfig & MID_VPD_POP)
	{
	        BUGLPR(dbg_midcfgvt,1, ("24 bit fill\n"));
	        /* 24-bit frame buffer */
	        flags = TWENTY4_BIT_FILL;
	        if (vp != 0)
	        {
	                BUGLPR(dbg_midcfgvt,1,("index=%d.\n",fill_value));
	                fill_value = ld->color_table.rgbval[fill_value];
	                BUGLPR(dbg_midcfgvt,1,("fill value=%d.\n",fill_value));
	        }

	}


	/* fill the rectangle of frame buffer with fill value.  Since we
	   not clearing overlay planes and z buffer so 0's are used
	   arbitrarily
	*/

	MID_ClearBitPlanes(flags,box->rulx,box->ruly,
	                         box->rwth,box->rht,fill_value,0,0)

	BUGLPR(dbg_midcfgvt,1, ("Leaving clear_box\n"));
}



/********************************************************************/
/*                                                                  */
/* IDENTIFICATION: VTTACT                                           */
/*                                                                  */
/* DESCRIPTIVE NAME:  Activate an inactive virtual terminal         */
/*                                                                  */
/* FUNCTION:    Set the Virtual Terminal state to active            */
/*                                                                  */
/* PSEUDO-CODE:                                                     */
/*                                                                  */
/*           Reset Adapter                                          */
/*             IF we are activating a Monitored (GRAPHICS) mode:    */
/*               - Do all things necessary to the adapter to bring  */
/*                 it to a "base state" i.e. clear out frame buffer,*/
/*                 stop any blinking, and set the screen origin to  */
/*                 the upper left corner.                           */
/*               - Turn off cursor                                  */
/*               - Take context off adapter                         */
/*                                                                  */
/*           IF we are activating a Character (KSR) mode terminal:  */
/*              - IF virtual terminal is currently in Monitor Mode: */
/*                  - insure good microcode                         */
/*              - Reset pointer to active VT for interrupt handler  */
/*              - Set character mode in real screen table           */
/*              - IF possible load fonts onto adapter.              */
/*              - Send the color table to the adapter.              */
/*              - Select cursor                                     */
/*              - Set up attribute fields                           */
/*              - IF necessary set screen origin                    */
/*              - Copy the contents of the presentation space into  */
/*                the adapter frame buffer.                         */
/*                                                                  */
/*           IF user has requested something other than KSR or      */
/*           GRAPHICS they are in error:                            */
/*              - Log an error and return a Zero                    */
/*                                                                  */
/*                                                                  */
/* INPUTS:    Virtual terminal pointer                              */
/*                                                                  */
/* OUTPUTS:                                                         */
/*                                                                  */
/* CALLED BY: Mode Processor                                        */
/*            vttsetm - VDD set mode routine                        */
/*                                                                  */
/* CALLS:     copy_ps                                               */
/*                                                                  */
/********************************************************************/
long vttact(vp)
struct vtmstruc *vp;                    /* ptr to VT data str           */
{

	struct middata  *ld;            /* ptr to local data area       */

	midddf_t   *ddf = (midddf_t *)vp->display->free_area;
	int     init_rc,rc,seq;
	ulong   fontid;
	gscDev     *pGSC = vp->display->pGSC;
	int     izz;                    /* for printing color palette   */


	HWPDDFSetup;                    /* gain access to ped H/W       */

	ddf-> pdev = pGSC;

	MID_DD_ENTRY_TRACE ( midcfgvt, 1, VTTACT, ddf,
	        0,
	        ddf,
	        vp,
	        0 );


	VDD_TRACE(ACT, ENTRY, vp);

	BUGLPR(dbg_midcfgvt, 1, ("Entering vttact\n"));

	PIO_EXC_ON();


	/*---------------------------------*/
	/* set the local data area pointer */
	/*---------------------------------*/

	ld = (struct middata *)vp->vttld;


	/*---------------------------------------------*/
	/* set the Virtual Terminal state to activated */
	/*---------------------------------------------*/

	ld->vtt_active = VTT_ACTIVE;


	switch (ld->vtt_mode)
	/*----------------------------------------------*/
	/* on the current state of the Virtual Terminal */
	/*----------------------------------------------*/

	{
	case
	/*------------*/
	/* monitored  */
	/*------------*/

	GRAPHICS_MODE:

	/*---------------------------------------------------*/
	/* Clear the entire frame buffer to background color */
	/* and restore the rendering context.                */
	/*---------------------------------------------------*/

	        if (vp->display->display_mode == KSR_MODE)
	        /*--------------------------------------*/
	        /* going from KSR to GRAPHICS mode.     */
	        /*--------------------------------------*/
	        {
	                BUGLPR(dbg_midcfgvt, 2, ("from KSR to MON mode\n"));
	                mid_init_rcx(vp->display);
	        }
	        else if (vp->display->display_mode == GRAPHICS_MODE)
	        {
	                BUGLPR(dbg_midcfgvt, 2, ("from MON to MON mode\n"));
	        }

	        vp->display->display_mode = GRAPHICS_MODE;

	        /*--------------------------------------*/
	        /* clear the screen.                    */
	        /*--------------------------------------*/

	        clear_frame_buffer(ddf,vp);

	        PIO_EXC_OFF();


	        /*------------------------------*/
	        /* Turn off the cursor          */
	        /*------------------------------*/

	        vttdefc(vp,0,FALSE);

	        /*-------------------------------------------------------*/
	        /* Reset pointer to active vt for interrupt handler      */
	        /*-------------------------------------------------------*/

	        vp->display->visible_vt = vp;

	        /* Roll last active context back on */
	        BUGLPR(dbg_midcfgvt, 1,
	                ("In act dom flags = %x \n", ddf->dom_flags));

	        /*--------------------------------------------------------
	          There are occasions where we have a null device structure
	          pointer as we enter vttact.  This can occur when we enter
	          GRAPHICS from KSR because there is no longer an active
	          GRAPHICS process (ie we just killed X, etc...
	        --------------------------------------------------------*/

	        if ( pGSC )     /* if we have a valid GSC pointer... */
	        {
	                if (!(ld->prcx_tosave[0] == NULL))
	                {
	                        mid_do_switch(pGSC,ddf->default_context_RCX,
	                                        ld->prcx_tosave[0]);
	                        ld->prcx_tosave[0] = NULL ;
	                }
	        }

	        PIO_EXC_ON();
	        break;                  /* end of monitored mode case   */


	case
	/*----------------*/
	/* character mode */
	/*----------------*/
	KSR_MODE:

	/*-----------------------------------------------------------*/
	/* copy the contents of the presentation space into the      */
	/* frame buffer establish the correct                        */
	/* position and shape of the hardware cursor.                */
	/* NOTE: A newly selected VDD has the following attributes:  */
	/* - the presentation space is initialized with all spaces   */
	/* - the shape of the hardware cursor is a double underscore */
	/* - the hw cursor is in the upper-left corner of the screen */
	/*-----------------------------------------------------------*/

	BUGLPR(dbg_midcfgvt, 1, ("Going to KSR mode\n"));

	/*-----------------------------------------------------------*/
	/* Reset pointer to active vt for interrupt handler          */
	/*-----------------------------------------------------------*/

	        vp->display->visible_vt = vp;

	        /*---------------------------------------------------*/
	        /* If the virtual terminal is currently in monitor   */
	        /* mode and we want to go into KSR mode, then we     */
	        /* must verify that the microcode is OK.             */
	        /* NOTE: currently we always reload the microcode.   */
	        /*---------------------------------------------------*/

	        if (vp->display->display_mode == GRAPHICS_MODE)
	        {
#if 0
	                /* ------
	                   If going from GRAPHICS to KSR mode: re-initialize
	                   the display adapter
	                -----  */

	                BUGLPR(dbg_midcfgvt, 1, ("from GRAPHICS to KSR mode\n"));
	                init_rc = mid_init(vp->display);
	                if (init_rc)
	                {
	                        init_rc = mid_init(vp->display);
	                        if (init_rc)
	                        {
	                        ddf->hwconfig |= MID_BAD_UCODE;
	                        BUGLPR(dbg_middd, 1,
	                                ("Cannot initialize adapter.\n"));
	                        }
	                        else ddf->hwconfig &= ~MID_BAD_UCODE;
	                }
	                else ddf->hwconfig &= ~MID_BAD_UCODE;
#endif
	        }
	        else
	        {

	                /*---------------------------------------------------
	                Want to use indexed shading mode so that when we set
	                foreground and background colors, we pass the indices.
	                (Note: for 24 bit color mode, we have to pass the color
	                intensities instead).

	                Display active cursor when going to KSR mode.
	                -----------------------------------------------------*/

	                MID_Set2DColorMode(3);
	                MID_HideShowActiveCursor(MID_SHOW_ACTIVE_CURSOR)

	        }


	        vp->display->display_mode = KSR_MODE;

	        /*--------------------------------------*/
	        /* clear the screen.                    */
	        /*--------------------------------------*/

	        clear_frame_buffer(ddf,vp);

	        /*--------------------------------------*/
	        /* Reset the current font.              */
	        /*--------------------------------------*/

	        fontid = 0xf0000000 |
	                (ulong) (ld->AIXfontptr[(ATTRIFONT(ld->current_attr))]);

	        BUGLPR(dbg_midcfgvt,1, ("SetActiveFont id=%x\n", fontid));

	        MID_SetActiveFont(fontid,fontid);


	        /*--------------------------------------*/
	        /* Reset the character colors.          */
	        /*--------------------------------------*/

	        BUGLPR(dbg_midcfgvt,3, ("ld->current_attr=0x%x\n",
	                ld->current_attr ));
	        BUGLPR(dbg_midcfgvt,3, ("ld->color_table.numcolors=0x%x\n",
	                ld->color_table.numcolors ));

#ifdef  DEBUG
	        for ( izz=0; izz<COLORPAL; izz++ )
	        {
	          BUGLPR(dbg_midcfgvt,3, (" ld->color_table.rgbval[%d]=0x%x\n",
	                izz,ld->color_table.rgbval[izz] ));
	        }
#endif


	        MID_SetForegroundPixel(ATTRIFORECOL(ld->current_attr));
	        MID_SetBackgroundPixel(ATTRIBAKCOL(ld->current_attr));


	        PIO_EXC_OFF();


	        /*--------------------------------------*/
	        /* send color table to adapter          */
	        /*--------------------------------------*/
	        vttstct(vp, &ld->color_table);

	        /*--------------------------------------*/
	        /* copy contents of presentation space  */
	        /* to adapter                           */
	        /*--------------------------------------*/
	        copy_ps(vp);
	        vttdefc(vp, ld->cursor_select, ld->cursor_show);


	        PIO_EXC_ON();
	        break;                  /* end of character mode        */



	default:
	        /*---------------------------------*/
	        /* invalid mode specified          */
	        /*---------------------------------*/
	        /*---------------------------------*/
	        /* log an error                    */
	        /*---------------------------------*/

	        BUGLPR(dbg_midcfgvt, 1,
	                ("Invalid mode (not KSR or GRAPHICS).\n"));
	        miderr(ddf,NULL,"mid-level","VTTACT",NULL,NULL,MID_INVALID_DISPLAY_MODE,
	                RAS_UNIQUE_1);

	        PIO_EXC_OFF();
	        return(-1);
	        break;                  /* end of invalid mode          */
	}                               /* end of switch                */

	vp->display->display_mode = vp->vtm_mode;

	PIO_EXC_OFF();

	VDD_TRACE(ACT, EXIT, vp);

	MID_DD_EXIT_TRACE ( midcfgvt, 1, VTTACT, ddf,
	        0,
	        ld->current_attr,
	        ld->cursor_show,
	        vp->vtm_mode );


	BUGLPR(dbg_midcfgvt, 1 , ("Leaving vttact with rc = 0.\n"));

	return(0);

}                                       /* end  of  vttact              */



/*--------------------------------------------------------------------*/
/*                                                                    */
/*  IDENTIFICATION: VTTDACT                                           */
/*                                                                    */
/*  DESCRIPTIVE NAME:  Deactivate Virtual Terminal                    */
/*                                                                    */
/*  FUNCTION:   Save and reset any necessary DDF data structures      */
/*                                                                    */
/*  PSEUDO-CODE:                                                      */
/*                                                                    */
/*              IF VT is currently in monitored mode:                 */
/*                 - reset any opflags and context data.              */
/*                 - set state of VT to inactive                      */
/*                 - reset WG wids                                    */
/*                 - reset WID table                                  */
/*                                                                    */
/*              Set the Virtual Terminal mode to inactive.  All       */
/*              subsequent draw, copy, cursor, etc. commands operate  */
/*              on the presentation space rather than on the adapter  */
/*              hardware. This prevents writes to the hardware.       */
/*                                                                    */
/*                                                                    */
/*  INPUTS:     Virtual terminal pointer                              */
/*                                                                    */
/*  OUTPUTS:    None                                                  */
/*                                                                    */
/*  CALLED BY:  Mode Processor                                        */
/*                                                                    */
/*  CALLS:      None                                                  */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/
long vttdact(vp)
struct vtmstruc    *vp;                 /* virtual terminal struct ptr  */
{
	struct middata  *ld;            /* ptr to local data area       */
	midddf_t   *ddf = (midddf_t *)vp->display->free_area;
	long            buf_offset;
	gscDev     *pGSC;
	rcmProcPtr pProc;
	rcxPtr     prcx;
	rcmWG      *pWG;
	int        rc,seq = 0;
	int        savecnt,i;
	mid_rcx_t  *midrcx;
	int        saved_intr_priority,our_intr_priority;
	ulong      int_mask;
	rcm_wg_hash_t   *pWG_hash_index;  /* use to traverse WG hash table */
	int              WG_table_index;  /* use to traverse WG hash table */

	HWPDDFSetup ;

	MID_DD_ENTRY_TRACE ( midcfgvt, 1, VTTDACT, ddf,
	        0,
	        ddf,
	        vp,
	        0 );

	VDD_TRACE(DACT, ENTRY, vp);

	BUGLPR(dbg_midcfgvt, 1 , ("Entering vttdact \n"));

	/*-----------------------------------------------------*/
	/* set the local data area pointer                     */
	/*-----------------------------------------------------*/

	ld = (struct middata *)vp->vttld;


	if (ld->vtt_mode == GRAPHICS_MODE && ddf->num_graphics_processes > 0)
	{
	   /* get device head and loop through all the process structs */
	   /* to find the contexts on this virtual terminal */
	   /* This code ASSUMES two things:
	      1: The device structure is created per virtual terminal. An
	         inspection was done of the rcm and lft code this seems to
	         be the case.
	      2: Need to be able to count on knowing the current rcx.
	                                                                 */
	    /* Interrupts are not being diabled since we are now under the
	       control of the screen manager process. Therefore all the
	       graphics processes have ben put to sleep for hot keying  */

	    /*
	       There are occasions where we have a null device structure
	       pointer as we enter vttact.  This can occur when we enter
	       GRAPHICS from KSR because there is no longer an active GRAPHICS
	       process (ie we just killed X, etc...
	    */

	    pGSC = vp->display->pGSC;
	    if ( pGSC )    /* let's make sure that our GSC pointer is good */
	                   /* before we start passing it around.           */
	    {

	        /* Save current context in first element */
	        /* Note that vttact needs the current context to be in element
	           0 for a successful hotkey */

	        ld->prcx_tosave[0] = (ddf->current_context_midRCX)->pRcx;
	        savecnt = 1;


	        /* Loop thru the list of all process on this Virtual terminal */

	        /* Now get the first proc structure */
	        pProc = pGSC->devHead.pProc;

	        /* Loop through all the processes looking for non null */
	        /* contexts to remove                                  */

	        while (pProc != NULL)
	        {
	          /* for all the contexts under this process */
	          BUGLPR(dbg_midcfgvt, 1 ,
	                ("looking at process %x pid = %d\n",
	                pProc,pProc->procHead.pid));

	          prcx = pProc->procHead.pRcx;


	          while (prcx != NULL)
	          {
	         /* if context is non null and not on the adapter*/

	         midrcx = (mid_rcx_t *) prcx->pData;

	         if (!(prcx->flags & RCX_NULL) &&
	             ( midrcx->flags.context_on_adapter ) )
	         {

	              /* check to see id this is the current rcx */
	              /* if so no need to save pointer */
	              if (!(prcx == ld->prcx_tosave[0]))
	              {
	                 ld->prcx_tosave[savecnt++] = prcx;
	                 BUGLPR(dbg_midcfgvt, 1,
	                   ("Going to save %x cnt is %d\n", prcx,savecnt));
	              }
#ifdef DEBUG
	              else
	                 BUGLPR(dbg_midcfgvt, 1,("Found current again \n"));
#endif

	         }
#ifdef DEBUG
	         else
	            BUGLPR(dbg_midcfgvt, 1,
	                ("Context is null or not on adapter \n"));
#endif
	         prcx = prcx->pNext;
	          }

	          pProc = pProc->procHead.pNext;
	        }

	        /* At this point we should have all the contexts we need to */
	        /* get off the adapter and our save context struct has the  */
	        /* current in entry 0. So loop through the structure saving */
	        /* the old to system memory and making the next entry active */
	        /* WE must be careful at the end to make the default active */
	        /* So we must loop from 0 to savecnt-1 then make the default */
	        /* active.                                                  */

	        ddf->dom_flags |= HOT_KEY_IN_PROGRESS;
	        BUGLPR(dbg_midcfgvt, 1, ("We have %d contexts to save\n",
	                                  savecnt));

	        for ( i = 0; i < savecnt-1; i++)
	        {

	                /* wait a while for ucode to quiesce before switch */
	                mid_delay_with_bus_handling (ddf, 100000);

	        BUGLPR(dbg_midcfgvt, 1, ("Now rolling ctx off %x\n",
	                                  prcx));
	        mid_do_switch(pGSC, ld->prcx_tosave[i],ld->prcx_tosave[i+1]);

	        }

	        /* wait a while for ucode to quiesce before switch */
	        mid_delay_with_bus_handling (ddf, 100000);

	        BUGLPR(dbg_midcfgvt, 1, ("Now making default active \n"));

	        mid_do_switch(pGSC,ld->prcx_tosave[i],ddf->default_context_RCX);

	        /* Now we need to ensure we are synced up and the last switch is
	           truly complete */

	        our_intr_priority = pGSC -> devHead.display ->
	                        interrupt_data.intr.priority - 1 ;
	        saved_intr_priority = i_disable (our_intr_priority) ;

	        PIO_EXC_ON();

	        MID_SLOW_IO(ddf) ;

	        /*-----------------------------------------------------------*
	        Read the adapter (BIM) interrupt mask
	         *-----------------------------------------------------------*/


	        int_mask = ddf->host_intr_mask_shadow ;
	        BUGLPR(dbg_midcfgvt, 2, ("mask before = %4X \n", int_mask ) );

	        int_mask = int_mask | MID_K_HOST_IO_CONTEXT_SWITCH_COMPLETED ;

	        ddf->host_intr_mask_shadow = int_mask ;
	        MID_WR_HOST_INTR (int_mask) ;
	        BUGLPR(dbg_midcfgvt, 2, ("mask written = %4X \n", int_mask ) );

	        MID_FAST_IO(ddf) ;

	        PIO_EXC_OFF () ;

	        /*-----------------------------------------------------------*
	        Set the domain flag indicating that we are waiting for
	         the previous switch to complete.
	         *-----------------------------------------------------------*/

	        ddf->dom_flags |= HOT_KEY_SLEEPING ;
	        ddf -> dom_switch_sleep_word = EVENT_NULL;
	        ddf -> dom_switch_sleep_flag = NULL;
#if 0
	        BUGLPR(dbg_midcfgvt, 1, ("Going to sleep \n" ) );

	        e_sleep ( &(ddf -> dom_switch_sleep_word), EVENT_SHORT) ;
#endif
	        i_enable (saved_intr_priority) ;

	        while ( !ddf -> dom_switch_sleep_flag )
	        {
	                /* Waiting for Context Switch Complete interrupt.*/
	        }

	        ddf->dom_flags = PREVIOUS_SWITCH_FINALLY_COMPLETED;

	        BUGLPR(dbg_midcfgvt, 1,
	        ("Woke up dom flags = %x \n", ddf->dom_flags));


	        /* Now clean up the device dependent window geometry structures.

	           Traverse WG hash table.  Each hash table entry is a linked
	           list of window geometries, so loop through all hash table
	           entries and then loop through the linked list which makes up
	           each of those entries.
	        */

	        for ( WG_table_index = 0;
	              WG_table_index < RCM_WG_HASH_SIZE;
	              WG_table_index++ )
	        {
	            pWG_hash_index =
	                &(pGSC->devHead.wg_hash_table->entry[WG_table_index]);

	            pWG = pWG_hash_index->pWG;

	            while ( pWG != NULL )
	            {
	                clean_DD_WG_structs(pWG, ddf);
	                pWG = pWG->pNext;
	            }

	        }
	    }

	
	   /* here remove any active X fonts for this particular vt */

	   MID_DD_TRACE_PARMS ( midcfgvt, 1, FONT_HOTKEY, ddf, ddf,
	                        ddf->font_DMA[0].font_ID,
	                        ddf->font_DMA[1].font_ID,
	                        ddf->pin_count ) ;

	   our_intr_priority = pGSC -> devHead.display ->
	                        interrupt_data.intr.priority - 1 ;

	   saved_intr_priority = i_disable (our_intr_priority) ;

	   for (i=0; i < MAX_PIN_FONTS_ALLOW ; i++)
	   {
	       if (ddf->font_DMA[i].font_ID != NO_FONT_ID)
	       {
	          unpin_font(vp->display, &(ddf->font_DMA[i]) ) ;
	          ddf->font_DMA[i].font_ID = NO_FONT_ID;
	       }
	   }
	   i_enable (saved_intr_priority) ;

	   MID_DD_TRACE_PARMS ( midcfgvt, 1, FONT_HOTKEY, ddf, ddf,
	                        ddf->font_DMA[0].font_ID,
	                        ddf->font_DMA[1].font_ID,
	                        ddf->pin_count ) ;

	}


	/*---------------------------------------------------*/
	/* set the state of the virtual terminal to inactive */
	/*---------------------------------------------------*/

	ld->vtt_active = VTT_INACTIVE;

	VDD_TRACE(DACT, EXIT, vp);

	MID_DD_EXIT_TRACE ( midcfgvt, 1, VTTDACT, ddf,
	        0,
	        ld->vtt_mode,
	        ddf->dom_flags,
	        (ddf->current_context_midRCX)->pRcx );


	BUGLPR(dbg_midcfgvt, 1 , ("Leaving vttdact with rc = 0.\n"));

	return(0);
}                                       /* end  of  vttdact             */




/*---------------------------------------------------------------------*/
/*                                                                     */
/* IDENTIFICATION: mid_delay_with_bus_handling                          */
/*                                                                      */
/* DESCRIPTIVE NAME: Delay including PIO exception on/off handling      */
/*                                                                     */
/* PSEUDO CODE:                                                        */
/*                                                                     */
/*            PIO_EXC_ON                                                */
/*                                                                     */
/*            mid_delay ()                                              */
/*                                                                     */
/*            PIO_EXC_OFF                                               */
/*                                                                     */
/*---------------------------------------------------------------------*/
long mid_delay_with_bus_handling (ddf, usecs)
	midddf_t        *ddf ;
	ulong            usecs ;
{

	HWPDDFSetup;                    /* gain access to ped H/W       */

	PIO_EXC_ON();

	mid_delay(usecs);

	PIO_EXC_OFF();

	return(0);
}



/*---------------------------------------------------------------------*/
/*                                                                     */
/* IDENTIFICATION: VTTSETM                                             */
/*                                                                     */
/* DESCRIPTIVE NAME: Set Mode for the Virtual Display Driver           */
/*                                                                     */
/* FUNCTION:  This command sets the mode of the VDD to the             */
/*            value of the mode passed.                                */
/*                                                                     */
/* PSEUDO CODE:                                                        */
/*                                                                     */
/*            IF the mode passed in is valid                           */
/*               - Update the mode                                     */
/*                                                                     */
/*            IF the virtual terminal is active                        */
/*               - Call the activate routine ( vttact() ), to          */
/*                 propagate the mode change                           */
/*                                                                     */
/*                                                                     */
/* INPUTS:    Virtual terminal pointer                                 */
/*            The value to set vtt_mode to.                            */
/*                                                                     */
/* OUTPUTS:   None.                                                    */
/*                                                                     */
/* CALLED BY: This routine is called by the Virtual Terminal Mode      */
/*            Processor.  It is an entry point.                        */
/*                                                                     */
/*                                                                     */
/* CALLS:     This routine calls vttact(), if this virtual terminal    */
/*            is active, to propagate the mode change.                 */
/*                                                                     */
/*                                                                     */
/*---------------------------------------------------------------------*/
long vttsetm(vp, mode)
struct vtmstruc     *vp;                /* virtual terminal struct ptr */
long                mode;               /* adapter mode: 0=> monitored,*/
	                                /* 1=> character               */
{
long                    i;              /* loop ctr for ucode check    */
struct middata          *ld;            /* ptr to local data area      */
midddf_t   *ddf =  (midddf_t *)vp->display->free_area;


	MID_DD_ENTRY_TRACE ( midcfgvt, 1, VTTSETM, ddf,
	        0,
	        ddf,
	        vp,
	        mode );

	VDD_TRACE(SETM, ENTRY, vp);

	BUGLPR(dbg_midcfgvt, 1 , ("Entering vttsetm\n"));


	/*----------------------------------------------*/
	/* set the local data area pointer              */
	/*----------------------------------------------*/

	ld = (struct middata *)vp->vttld;


	if  ((mode == KSR_MODE) || (mode == GRAPHICS_MODE))
	/*----------------------------------------------*/
	/* the requested mode is character or monitored */
	/*----------------------------------------------*/
	{

	        /*----------------------------------------*/
	        /* set the VDD state to the specifed mode */
	        /*----------------------------------------*/

	        ld->vtt_mode = mode;

	        if (ld->vtt_active)
	        /*----------------------------------------*/
	        /* the virtual terminal is active         */
	        /*----------------------------------------*/
	        {
	                /*-----------------------*/
	                /* Call activate routine */
	                /*-----------------------*/

	                BUGLPR(dbg_midcfgvt, 2, ("Activating now.\n"));
	                vttact(vp);
	        }
	}

	VDD_TRACE(SETM, EXIT, vp);

	MID_DD_EXIT_TRACE ( midcfgvt, 1, VTTSETM, ddf,
	        0,
	        ddf,
	        vp,
	        mode );


	BUGLPR(dbg_midcfgvt, 1, ("Leaving vttsetm with rc = 0.\n"));
	return(0);
}                                /* end  of  set mode                  */



/*---------------------------------------------------------------------*/
/*                                                                     */
/* IDENTIFICATION: VTTSTCT                                             */
/*                                                                     */
/* DESCRIPTIVE NAME: Set color table routine                           */
/*                                                                     */
/* FUNCTION:                                                           */
/*                                                                     */
/* PSEUDO-CODE:                                                        */
/*                                                                     */
/*            Copy color table passed in to the local data color table */
/*                                                                     */
/*            Enable bus access                                        */
/*                                                                     */
/*            Send fixed and variable length portions of               */
/*            colormap to adapter                                      */
/*                                                                     */
/*            Disable bus access                                       */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/* INPUTS:    Virtual terminal pointer                                 */
/*            Color table in the structure of colorpal                 */
/*                                                                     */
/* OUTPUTS:                                                            */
/*                                                                     */
/* CALLED BY: Mode processor                                           */
/*            vttinit()                                                */
/*                                                                     */
/* CALLS:                                                              */
/*                                                                     */
/*---------------------------------------------------------------------*/
long vttstct(vp, color_table)
struct vtmstruc         *vp;            /* virtual terminal struct ptr  */
struct colorpal *color_table;
{
ushort                  i;              /* loop variable                */
midddf_t   *ddf =  (midddf_t *)vp->display->free_area;
struct middata          *ld;            /* ptr to local data area       */
union   {
	ulong full;

	struct {
	        uchar  char0,
	        char1,
	        char2,
	        char3;
	        } byte;
	} workword;

	HWPDDFSetup;                    /* gain access to ped H/W       */

	MID_DD_ENTRY_TRACE ( midcfgvt, 1, VTTSTCT, ddf,
	        0,
	        ddf,
	        vp,
	        color_table );

	VDD_TRACE(STCT, ENTRY, vp);

	BUGLPR(dbg_midcfgvt, 1, ("Entering vttstct\n"));


	/*--------------------------------------------*/
	/* set the local data area pointer            */
	/*--------------------------------------------*/

	ld = (struct middata *)vp->vttld;


	/*--------------------------------------------*/
	/* set the device dependent data area pointer */
	/*--------------------------------------------*/

	ddf = (midddf_t *) vp->display->free_area;


	/*--------------------------------------------*/
	/* copy color table passed in to the local    */
	/* data color table                           */
	/*--------------------------------------------*/

	for (i = 0; i < ld->color_table.numcolors; i++)
	{
	        ld->color_table.rgbval[i] = color_table->rgbval[i];
	        BUGLPR(dbg_midcfgvt,3, ("vttstct: color%d = 0x%x\n",i,
	                                      color_table->rgbval[i]));
	}

	PIO_EXC_ON();

	/*--------------------------------------------*/
	/* load color table - Send fixed and variable */
	/* portions of colormap to FIFO               */
	/*--------------------------------------------*/
      MID_LoadFrameBufferColorTable(
      0,                                    /* color table 0 */
      0,                                    /* start index 0 */
      ld->color_table.numcolors,            /* # of entries  */
      ((MIDColor24 *)color_table->rgbval)); /* ary of colors */

	PIO_EXC_OFF();

	VDD_TRACE(STCT, EXIT, vp);

	MID_DD_EXIT_TRACE ( midcfgvt, 1, VTTSTCT, ddf,
	        0,
	        ddf,
	        vp,
	        ld->color_table.numcolors );


	BUGLPR(dbg_midcfgvt, 1, ("Leaving vttstct with rc = 0.\n"));
	return(0);
}                                       /*  end  of  vttstct            */

/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: VTTTERM                                             */
/*                                                                     */
/* DESCRIPTIVE NAME: Terminate virtual terminal                        */
/*                                                                     */
/* FUNCTION:  This procedure deallocates the resources associated with */
/*            a given virtual terminal.                                */
/*                                                                     */
/*                                                                     */
/* PSEUDO-CODE:                                                        */
/*            Free the space used for the presentation space           */
/*                                                                     */
/*            Decrement the usage count                                */
/*                                                                     */
/*            Free the local data area                                 */
/*                                                                     */
/*                                                                     */
/* INPUTS:    Virtual terminal pointer                                 */
/*                                                                     */
/* OUTPUTS:   None                                                     */
/*                                                                     */
/* CALLED BY: Mode processor                                           */
/*                                                                     */
/* CALLS:     None.                                                    */
/*                                                                     */
/***********************************************************************/
long vttterm(vp)
struct vtmstruc *vp;                   /* virtual terminal struct ptr  */
{
struct middata  *ld;                   /* ptr to local data area       */
midddf_t   *ddf =   (midddf_t *)vp->display->free_area;

	VDD_TRACE(TERM, ENTRY, vp);

	MID_DD_ENTRY_TRACE ( midcfgvt, 1, VTTTERM, ddf,
	        0,
	        ddf,
	        vp,
	        0 );

	BUGLPR(dbg_midcfgvt, 1, ("Entering vttterm\n"));


	/*---------------------------------*/
	/* set the local data area pointer */
	/*---------------------------------*/

	ld = (struct middata *)vp->vttld;


	/*---------------------------------*/
	/* free the presentation space     */
	/*---------------------------------*/

	if (ld->pse != NULL)
	        xmfree((char *)ld->pse, pinned_heap);
	ld->pse = NULL;
	ld->ps_bytes = 0;


	/*---------------------------------*/
	/* free the local data area        */
	/*---------------------------------*/

	if (ld != NULL)
	        xmfree((char *)vp->vttld, pinned_heap);
	vp->vttld = NULL;


	/*---------------------------------*/
	/* decrement the usage count       */
	/*---------------------------------*/

	if (vp->display->usage >= 0)
	        vp->display->usage -= 1;

	/*-----------------------------------*/
	/* reset for Display Power Management*/
	/*-----------------------------------*/
        vp->display->current_dpm_phase = 0;

	VDD_TRACE(TERM, EXIT, vp);

	MID_DD_EXIT_TRACE ( midcfgvt, 1, VTTTERM, ddf,
	        0,
	        ddf,
	        vp,
	        vp->display->usage );

	BUGLPR(dbg_midcfgvt, 1, ("Leaving vttterm with rc = 0.\n"));

	return(0);
}                                /* end  of  vttterm             */



/*-----------------------------------------------------------
 |
 | This function support display power management.  The
 | driver tells the micro-code when to power down the
 | display.  Due to some limitation (see defect 148497)  
 | the microcode can only support on and off states.  Also,
 | before turning off the H and V sync pulse, the microcode
 | set all color map entries to zeroes to blacken the screen.
 | This is needed for displays which don't support DPMS because
 | when the sync pulses are turned off, the monitor won't work 
 | correctly any more.  As the result, user will notice weird
 | stuff on the screen.
 |
 |-----------------------------------------------------------*/

long vttdpm(pd,phase)
struct phys_displays *pd;
int phase;                  /* 1=full-on, 2=standby, 3=suspend, 4=off */
{
	midddf_t   *ddf =  (midddf_t *)pd->free_area;

	HWPDDFSetup;                    /* gain access to ped H/W       */

	PIO_EXC_ON();

        /*
         * 1. if caller wants to turn on the display when it's off, we turn it on,
         *    and then update current DPM state
         * 2. if caller wants to turn off the display when it's on, we turn it off.
         *    and then update current DPM state
         * 3. Other than that just update current DPM state
         *
         *    Note initially current_dpm_phase is zero because the whole pd is bzeroed out
         *    Also, every lft is unconfigured, it calls vttterm.  In there current_dpm_phase
         *    is set to zero again.
         */

	if ((phase == 1) && (phase != pd->current_dpm_phase))      /* case 1 */
	{
		MID_DisplayPowerUpPCB()
	}
	else if ( (phase != 1) && (pd->current_dpm_phase < 2) )    /* case 2 */
	{
		MID_DisplayPowerDownPCB()
	}		

	pd->current_dpm_phase = phase;
 
	PIO_EXC_OFF();
	
 	return(0);
}
