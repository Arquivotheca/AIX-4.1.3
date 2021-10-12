/* @(#)11	1.38  src/bos/kernext/lft/inc/sys/display.h, sysxdisp, bos41J, 9513A_all 3/20/95 19:34:14 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993-1994.
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * KSR - keyboard send/receive mode (character)
 * MOM - monitor mode
 * VDD - virtual display driver
 * VT  - virtual terminal
*/

#ifndef _H_DISPLAY
#define _H_DISPLAY 1

#define MAX_DOMAINS 4
#define MAXDMABUFS 4

#include <sys/xmem.h>
#include <sys/rcm_win.h>
#include <sys/aixgsc.h>
#include <sys/rcm.h>
#include <sys/pwr_mgr.h>


struct dma_bufs {			/*************************************/
	ulong  bus_addr;		/* byte offset of base of dma area   */
	ulong  length;			/* length in bytes of dma area	     */
};					/*************************************/


struct _bmr {				/*************************************/
	int	auth_mask;		/* authorization mask to allow access*/
	caddr_t bus_mem_start_ram;	/* byte offset of base of dma area   */
	caddr_t bus_mem_end_ram;	/* byte offset of base of dma area   */
};					/*************************************/


					/***********************************/
struct	display_info {			/* display info for the LFT & RCM  */
	ulong  font_width;		/* width  of the char in pixels    */
	ulong  font_height;		/* height of the char in pixels    */
	ulong  bits_per_pel;		/* # bits in each pixel value	   */
	ulong  adapter_status;		/* adapter status as follows:	   */
					/* :1=1 if color monitor	   */
					/* :1=1 if the adapter's VLT is    */
					/*	modifiable		   */
					/* :1=1 set to 0 by the device	   */
					/*	driver if the adapter is   */
					/*	is working properly	   */
					/* :29	reserved		   */
	ulong  apa_blink;		/* :1=1 APA device, 0=A/N device   */
					/* :1=1 blink supported 	   */
					/* :30	reserved		   */
	ulong  screen_width_pel;	/* width of the screen in pixels   */
	ulong  screen_height_pel;	/* height of the screen in pixels  */
	ulong  screen_width_mm; 	/* width of screen in mm	   */
	ulong  screen_height_mm;	/* height of screen in mm	   */
	ulong  colors_total;		/* total # colors available	   */
	ulong  colors_active;		/* # colors that can be displayed  */
					/*   simultaneously		   */
	ulong  colors_fg;		/* # foreground colors that can be */
					/*   displayed simultaneously	   */
	ulong  colors_bg;		/* # background colors that can be */
					/*   displayed simultaneously	   */
	ulong  color_table[16]; 	/* ksr default colors		   */
	long   reserved1;		/* reserved			   */
	long   reserved2;		/* reserved			   */
};					/***********************************/


struct phys_displays {			/***********************************/
	struct {			/* data to set up interrupt call   */
	    struct intr intr;		/* at init time (i_init)	   */
	    long intr_args[4];		/*				   */
	} interrupt_data;		/***********************************/
	struct phys_displays *same_level; /* other interrupts on same level*/
	struct phys_displays *next;	/* ptr to next minor number data   */
        struct _gscDev       *pGSC;     /* device struct used by rcm       */
	dev_t devno;			/* Device number of this adapter   */
	struct lft 	*lftanchor;	/* lft subsystem		   */
	long dds_length;		/* length in bytes		   */
	char *odmdds;			/* ptr to define device structure  */
	struct display_info display_info; /* display information	   */
	uchar disp_devid[4];		/* device information		   */
					/* [1] = 04=display device	   */
					/* [2] = 21=reserved 22=reserved   */
					/*	 25=reserved 27=reserved   */
					/*	 29=reserved		   */
					/* [3] = 00=functional		   */
					/* [4] = 01-04=adapter instance    */
	uchar usage;			/* number of VT's using real screen*/
					/*   used to prevent deletion of   */
					/*   real screen from configuration*/
					/*   if any VT is using it.	   */
	uchar open_cnt; 		/* Open flag for display	   */
	uchar display_mode;		/* Actual state of the display,    */
					/* not the virtual terminal:	   */
					/* KSR_MODE or MOM_MODE (see vt.h) */
	uchar dma_characteristics;	/* Attributes related to DMA ops   */
#	define DMA_SLAVE_DEV	1	/* Device is bus slave, ow. master */
	struct font_data *default_font; /* Pointer to the default font for */
					/*   this display		   */
	struct vtmstruc	*visible_vt;	/* Pointer to current vt active or */
					/*   pseudo-active on THIS display */
					/***********************************/
					/* DMA Data Areas		   */
					/***********************************/
	long  dma_chan_id;		/* channel id returned from d_init */
	struct dma_bufs 		/* DMA buffer structure 	   */
		d_dma_area[MAXDMABUFS]; /*				   */
					/***********************************/
					/* Rendering Context Manager Areas */
					/***********************************/
	rcmProcPtr cur_rcm;		/* Pointer to current rcm on this  */
					/* display			   */
	int num_domains;		/* number of domains		   */
	int dwa_device; 		/* supports direct window access   */
	struct _bmr			/* bus memory ranges		   */
		busmemr[MAX_DOMAINS];	/*				   */
	ulong io_range; 		/* low limit in high short	   */
					/* high limit in low short	   */
					/* to match IOCC register	   */
	ulong *free_area;		/* area free for usage in a device */
					/*   dependent manner by the VDD   */
					/*   for this real screen.	   */

	short pwr_mgr_phase; 		/* not used - will remove in 4.2   */ 

	int pwr_mgr_time[3]; 		/* not used - will remove in 4.2   */ 

	pwr_mgr_t pwr_mgr_data;         /* not used - will remove in 4.2   */ 
	
	int pwr_mgr_init_wd;            /* not used - will remove in 4.2   */ 

	int kb_active;                  /* not used - will remove in 4.2   */ 
	int pwr_mgr_flag;               /* not used - will remove in 4.2   */ 


	long current_dpm_phase;         /* current phase of DPM this display is in */
                                        /* full-on=1, standby=2, suspend=3, off=4  */
#define	DPMS_ON		0x1
#define	DPMS_STANDBY	0x2
#define	DPMS_SUSPEND	0x3
#define	DPMS_OFF	0x4

	ulong reserved2; 
	long reserved3;
	long reserved4;

	long (*lft_pwrmgr)();		/* not used - will remove in 4.2   */ 

					/***********************************/
					/* VDD Function Pointers	   */
					/***********************************/


	long (*vttpwrphase)();		/* power management phase change    */
                                        /* function.  It's device dependent */

	long (*vttact)();		/* Activate the display 	   */
	long (*vttcfl)();		/* Move lines around		   */
	long (*vttclr)();		/* Clear a box on screen	   */
	long (*vttcpl)();		/* Copy a part of the line	   */
	long (*vttdact)();		/* Mark the terminal as being	   */
					/*   deactivated		   */
	long (*vttddf)();		/* Device dependent functions	   */
					/*   i.e. Pacing, context support  */
	long (*vttdefc)();		/* Change the cursor shape	   */
	long (*vttdma)();		/* Issue dma operation		   */
	long (*vttdma_setup)(); 	/*  Setup dma			   */
	long (*vttterm)();		/* Free any resources used	   */
					/*   by this VT 		   */
	long (*vttinit)();		/* setup new logical terminal	   */
	long (*vttmovc)();		/* Move the cursor to the	   */
					/*   position indicated 	   */
	long (*vttrds)();		/* Read a line segment		   */
	long (*vtttext)();		/* Write a string of chars	   */
	long (*vttscr)();		/* Scroll text on the VT	   */
	long (*vttsetm)();		/* Set mode to KSR or MOM	   */
	long (*vttstct)();		/* Change color mappings	   */

	long (*reserved5)();
	long (*reserved6)();	
					/***********************************/
					/* RCM Function Pointers	   */
					/***********************************/
	long (*make_gp)();		/*  Make a graphics process	   */
	long (*unmake_gp)();		/*  Unmake a graphics process	   */
	long (*state_change)();		/*  State change handler invoked   */
	long (*reserved8)();		/*  Reserved for future use	   */
	long (*create_rcx)();		/*  Create a hardware context	   */
	long (*delete_rcx)();		/*  Delete a hardware context	   */
	long (*create_rcxp)();		/*  Create a context part	   */
	long (*delete_rcxp)();		/*  Delete a context part	   */
	long (*associate_rcxp)();	/*  Link a part to a context	   */
	long (*disassociate_rcxp)();	/*  Unlink a part from a context   */
	long (*create_win_geom)();	/*  Create a window on the screen  */
	long (*delete_win_geom)();	/*  Delete a window on the screen  */
	long (*update_win_geom)();	/*  Update a window on the screen  */
	long (*create_win_attr)();	/*  Create a window on the screen  */
	long (*delete_win_attr)();	/*  Delete a window on the screen  */
	long (*update_win_attr)();	/*  Update a window on the screen  */
	long (*bind_window)();		/*  Update a window bound to rcx   */
	long (*start_switch)(); 	/*  Start a context switch	   */
					/*  Note: This routine runs on	   */
					/*  the interrupt level 	   */
	long (*end_switch)();		/*  Finish the context switch	   */
					/*  started by start_switch()	   */
	long (*check_dev)();		/*  Check if this address belongs  */
					/*  to this device.		   */
					/*  Note: this is run on interrupt */
					/*  level.			   */
	long (*async_mask)();		/* Set async events reporting	   */
	long (*sync_mask)();		/* Set sync events reporting	   */
	long (*enable_event)(); 	/* Turns adapter function on	   */
					/* without reports to application  */
	long reserved9; 		/* reserved			   */
	long reserved10; 		/* reserved			   */
	void (*give_up_time_slice)();	/* Relinquish remaining time	   */
	long (*diag_svc)();		/* Diagnostics Services (DMA)	   */
	long (*dev_init)();		/* Device dep. initialization      */
	long (*dev_term)();		/* Device dep. cleanup  	   */
					/***********************************/

                                        /***********************************/
                                        /* Font Support Function Pointers  */
                                        /***********************************/

        long (* pinned_font_ready)();
	long (* vttddf_fast)();		/* fast ddf functions              */

        ushort bus_type;                /* indicates what type of bus      */
#       define  DISP_BUS_MCA    0x8000  /* Microchannel                    */
#       define  DISP_BUS_SGA    0x4000  /* currently not used              */
#       define  DISP_BUS_PPC    0x2000  /* processor bus                   */

        ushort flags;                   /* physical display flags          */

#       define GS_DD_DOES_AS_ATT        (1L << 0) /* no as_att() by RCM    */
						  /* not currently used    */

#	define GS_BUS_AUTH_CONTROL	(1L << 1) /* Request bus access ctrl */

#       define GS_HAS_INTERRUPT_HANDLER (1L << 2) /* 1 after i_init()      */
                                                  /* 0 after i_clear()     */
						  /* not currently used    */

	ulong reserved_seg_ext;		/* currently not used		   */
	vmhandle_t segment;		/* currently not used		   */
	ulong reserved_addr_ext;	/* currently not used		   */
	caddr_t bus_addr;		/* currently not used		   */

	struct io_map  *io_map;		/* in case driver needs u_iomem_att */
	int		ear;		/* image for EAR reg (xferdata) if !0 */
	ulong	spares[18];		/* not used - for future development */
};


/*------------
  Virtual Device Driver (vdd) interface:

    struct vtmstruc *vp;	   vt structure pointer - see vt.h
    ulong cursor_show;		   true=move cursor to cursor position in vp

--- Move display data from the presentation space to the display --
  vttact(vp);

--- Move lines around ---
  vttcfl(vp,src_row,dest_row,num_rows,cursor_show);
    long src_row, dest_row, num_rows;

--- Clear a box on screen ---
  vttclr(vp, sp, attr, cursor_show);
    struct vtt_box_rc_parms *sp;	   source ptr (row/col upper left
					      & lower right corners of box)
    ulong attr; 			   see #defines in vt.h (blink, etc)

--- Copy a part of the line ---
  vttcpl(vp, rc, cursor_show);
    struct vtt_rc_parms *rc;		   row/column parameters

--- Mark the terminal as being deactivated ---
  vttdact(vp);

--- Change the cursor shape ---
  vttdefc(vp, selector, cursor_show);
    uchar  selector;		   cursor shape - see #defines in lft.h

--- Free any resources used by this VT ---
  vttterm(vp);

--- Setup new logical terminal ---
  vttinit(vp, font_ids, ps_s);
    struct fontpal *font_ids;	   font pallet in vt.h
    struct ps_s *ps_s;		   presentation space height/width in vt.h

---  Move the cursor to the position indicated ---
  vttmovc(vp);

--- Read a line segment ---
  vttrds(vp, ds, ds_size, attr, attr_size, rc);
    ushort *ds; 		   array of display symbols vttrds returns
    long ds_size;		   size of ds array
    ushort *attr;		   array of attributes vttrds returns
    long attr_size;		   size of attr array
    struct vtt_rc_parms *rc;	   string position and length

--- Write a string of chars ---
  vtttext(vp, string, rc, cp, cursor_show);
    char *string;		   array of ascii chars
    struct vtt_rc_parms *rc;	   string position and length
    struct vtt_cp_parms *cp;	   code pt base/mask & new cursor position

--- Scroll text on the VT ---
  vttscr(vp, lines, attr, cursor_show);
    long lines; 		   number of lines to scroll
    ulong attr; 		   attr associated w/all chars of new lines

---  Set mode to KSR or MOM ---
  vttsetm(vp,newmode);
    long newmode;		   KSR_MODE or MOM_MODE in vt.h

--- Change color mappings ---
  vttstct(vp, color_table);
    struct colorpal *color_table;	   color pallet in vt.h
  ------------*/

#endif /* _h_DISPLAY */
