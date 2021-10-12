static char sccsid[] = "@(#)50  1.11.1.13  src/bos/kernext/disp/ped/config/cfg_init.c, peddd, bos411, 9434B411a 8/3/94 10:17:03";
/*
 * COMPONENT_NAME: PEDDD
 *
 * FUNCTIONS: cfg_init, mid_timeout, bad_func, good_func
 *	
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/types.h>
#include <sys/iocc.h>
#include <sys/mdio.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/syspest.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/sleep.h>

#include "ddsmid.h"
#include "midddf.h"
#include "midhwa.h"

BUGXDEF(dbg_middd);

#ifndef BUGVPD				/* Set to 0 in Makefile if debugging */
#define BUGVPD	BUGACT
#endif

extern long mid_open (), mid_close (), mid_ioctl (), mid_config ();

extern long mid_init();
extern long mid_intr();
extern ulong ucode_version(void);

extern long vttact(struct vtmstruc *);
extern long vttcfl(struct vtmstruc *, long, long, long, ulong);
extern long vttclr(struct vtmstruc *, struct vtt_box_rc_parms *, ulong, ulong);
extern long vttcpl( struct vtmstruc *, struct vtt_rc_parms *, ulong);
extern long vttdact(struct vtmstruc *);
extern long vttdefc( struct vtmstruc *, uchar, ulong);
extern long vttinit( struct vtmstruc *, struct fontpal *, struct ps_s *);
extern long vttmovc( struct vtmstruc *);
extern long vttrds( struct vtmstruc *, ushort *, long, ushort *, long,
	struct vtt_rc_parms *);
extern long vttscr( struct vtmstruc *, long , ulong, ulong);
extern long vttsetm( struct vtmstruc *, long);
extern long vttstct( struct vtmstruc *, struct colorpal *);
extern long vttterm(struct vtmstruc *);
extern long vtttext( struct vtmstruc *, char *, struct vtt_rc_parms *,
	struct vtt_cp_parms *, ulong);

extern long vttdpm();

extern long mid_ddf();
extern long vttdma_setup(struct vtmstruc *, struct _gscdma *);
extern long vttdma(gscDev *, struct _gscdma *, int (*)(), caddr_t);
extern long diag_svc(gscDev *, struct _gscdma *, int (*)(), caddr_t);

extern long mid_make_gp();
extern long mid_unmake_gp();
extern long mid_check_dev();
extern long mid_create_rcx();
extern long mid_delete_rcx();
extern long mid_create_win_attr();
extern long mid_delete_win_attr();
extern long mid_update_win_attr();
extern long mid_start_switch();
extern long mid_end_switch();
extern long mid_create_win_geom();
extern long mid_delete_win_geom();
extern long mid_update_win_geom();
extern long mid_bind_window();
extern long mid_async_mask();
extern long mid_enable_event();
extern long mid_sync_mask();
extern long pinned_font_ready();
extern void mid_timeout_getcpos();
extern void mid_timeout_getcolor();
extern void mid_timeout_getcondition();
extern void mid_timeout_gettextfontindex();
extern void mid_timeout_swap();
extern void mid_timeout_lowwater();
extern void mid_timeout_endrender();
extern void mid_WID_wakeup();

extern void mid_dev_init();
extern void mid_dev_term();

static int bad_func();
static int good_func();
void mid_timeout();






int bad_func()
{
	return -1;
}

int good_func()
{
	return 0;
}


/***********************************************************************/
/*								       */
/* DESCRIPTIVE NAME: CFG_INIT - Config init routine for MIDDD3D        */
/*								       */
/* FUNCTION:							       */
/*								       */
/*								       */
/*								       */
/* INPUTS:							       */
/*								       */
/* OUTPUTS:							       */
/***********************************************************************/

long cfg_init(devno,uiop)
dev_t devno;
struct uio *uiop;

{
	struct ddsmid *ddsptr;
	struct phys_displays *pd;
	struct phys_displays *nxtpd;
	long i;
	union
	{
		long  full;
		struct
		{
			char b1;
			char b2;
			char b3;
			char b4;
		} byte;
	} devdat;

	char		slot;
	char            pos;
	long		b;
	volatile char	*pptr;
	midddf_t	*ddf;
	struct devsw	MIDDD3D_devsw;
	int		rc;
	unsigned long	level;
	int		status;
	extern		nodev();
	label_t jmpbuf;

	struct file	*MIDDD3D_ucode_file_ptr;
	volatile unsigned long HWP;
	unsigned 	int save_bus_acc = 0;

	long    ddslen = sizeof(struct ddsmid);

	int	init_rc;

	/*-------------------------------------------------------------------*/
	/* This routine will be evoked by the configuration method which will*/
	/* pass in the complete dds for the adapter. The routine will malloc */
	/* space for a physical display structure and fill in the function   */
	/* pointer fields as well as the interrupt data, the display type and*/
	/* the pointer to the dds structure. It then will look into the devsw*/
	/* table using the iodn in the dds and determine if any other like   */
	/* adapters have been put in the system. If none, it will invoke the */
	/* os config method to get the driver in the system.		     */
	/* If the adapter is already in the devsw the code will walk the     */
	/* chain of struct and attach the struct to the end of the list.     */
	/*-------------------------------------------------------------------*/

	BUGLPR(dbg_middd, BUGVPD, ("Entering cfg_init\n"));

	ddsptr = (struct ddsmid *) xmalloc(sizeof(struct ddsmid),
		3,pinned_heap);

	if (ddsptr == NULL)
	{
		return(-1);
	}

	rc = uiomove(ddsptr,sizeof(struct ddsmid), UIO_WRITE, uiop);

	if (rc != 0)
	{
		xmfree( ddsptr, pinned_heap );
		return(rc);
	}

	pd = (struct phys_displays *) xmalloc(sizeof(struct phys_displays),
		3,pinned_heap);

	if (pd == NULL)
	{
		xmfree( ddsptr, pinned_heap );
		return(-1);
	}

	bzero(pd, sizeof(struct phys_displays));


	/*----------------------------------------------------*/
	/* Initialize physical display structure	      */
	/*----------------------------------------------------*/

	pd -> next = NULL;

	pd->interrupt_data.intr.next	 = (struct intr *) NULL;
	pd->interrupt_data.intr.handler  = mid_intr;
	pd->interrupt_data.intr.bus_type = BUS_MICRO_CHANNEL;
	pd->interrupt_data.intr.flags	 = 0;
	pd->interrupt_data.intr.level	 = ddsptr->int_level;
	pd->interrupt_data.intr.priority = ddsptr->int_priority;
	pd->interrupt_data.intr.bid	 = BUS_ID;

	pd->same_level = NULL;
	pd->dds_length = ddslen;
	pd->odmdds = (char *) ddsptr;

	/*-------------------------------------------------------------*/
	/* Set up function pointers				       */
	/*-------------------------------------------------------------*/

	pd->vttact =  vttact;		/*  Move the data from this    */
					/*  terminal to the display.   */
	pd->vttcfl =  vttcfl;		/*  Move lines around.	       */
	pd->vttclr =  vttclr;		/*  Clear a box on screen.     */
	pd->vttcpl =  vttcpl;		/*  Copy a part of the line.   */
	pd->vttdact = vttdact;		/*  Mark the terminal as       */
					/*  being deactivated.	       */
	pd->vttddf  = mid_ddf;		/*  Device Dependent stuff.    */
	pd->vttdefc = vttdefc;		/*  Change the cursor shape.   */
	pd->vttdma_setup = vttdma_setup;/*  Setup the DMA.	       */
	pd->vttdma  = vttdma;           /*  DMA Access routine.        */
	pd->diag_svc = diag_svc;	/*  Diagnostics DMA service    */
	pd->vttterm = vttterm;		/*  Free any resources used    */
					/*  by this VT. 	       */
	pd->vttinit = vttinit;		/*  setup new VT.	       */
	pd->vttmovc = vttmovc;		/*  Move the cursor to the     */
					/*  position indicated.        */
	pd->vttrds  = vttrds;		/*  Read a line segment.       */
	pd->vtttext = vtttext;		/*  Write a string of chars.   */
	pd->vttscr  = vttscr;		/*  Scroll text on the VT.     */
	pd->vttsetm = vttsetm;		/*  Set mode to KSR or MOM.    */
	pd->vttstct = vttstct;		/*  Change color mappings.     */
	pd->vttpwrphase = vttdpm;	/*  Display power managemetn   */
	pd->make_gp = mid_make_gp;
	pd->unmake_gp = mid_unmake_gp;
	pd->check_dev = mid_check_dev;
	pd->create_rcx = mid_create_rcx;
	pd->delete_rcx = mid_delete_rcx;
	pd->create_win_attr = mid_create_win_attr;
	pd->delete_win_attr = mid_delete_win_attr;
	pd->update_win_attr = mid_update_win_attr;
	pd->start_switch = mid_start_switch;
	pd->end_switch = mid_end_switch;
	pd->create_rcxp = good_func;
	pd->delete_rcxp = good_func;
	pd->associate_rcxp = good_func;
	pd->disassociate_rcxp = good_func;
	pd->create_win_geom = mid_create_win_geom;
	pd->delete_win_geom = mid_delete_win_geom;
	pd->update_win_geom = mid_update_win_geom;
	pd->bind_window = mid_bind_window;
	pd->async_mask = mid_async_mask;
	pd->enable_event = mid_enable_event;
	pd->sync_mask = mid_sync_mask;
	pd->pinned_font_ready = pinned_font_ready;
	pd->dev_init = mid_dev_init;
	pd->dev_term = mid_dev_term;


	pd->display_info.colors_total = 16777215;
	pd->display_info.colors_active = 16;
	pd->display_info.colors_fg = 16;
	pd->display_info.colors_bg = 16;
	for (i=0; i < 16; i++)
		pd->display_info.color_table[i] = ddsptr->ksr_color_table[i];
	pd->display_info.font_width = 12;
	pd->display_info.font_height = 30;
	pd->display_info.bits_per_pel =  1;
	pd->display_info.adapter_status = 0xC0000000;
	pd->display_info.apa_blink = 0x80000000;
	pd->display_info.screen_width_pel = 1280;
	pd->display_info.screen_height_pel = 1024;
	pd->display_info.screen_width_mm = ddsptr->screen_width_mm;
	pd->display_info.screen_height_mm = ddsptr->screen_height_mm;


	/*-----------------------------------------------------------------*/
	/* MIDDD3D physical device id					   */
	/*-----------------------------------------------------------------*/

	devdat.full = ddsptr->display_id;
	pd->disp_devid[0] = devdat.byte.b1;
	pd->disp_devid[1] = devdat.byte.b2;
	pd->disp_devid[2] = devdat.byte.b3;
	pd->disp_devid[3] = devdat.byte.b4;

	pd->usage = 0;		       /* Set to indicate no VTs open	   */
	pd->open_cnt = 0;	       /* Set to indicate driver is closed */
	pd->devno = devno;


	/*-----------------------------------------------------------------*/
	/* allocate area for device dependent functions.		   */
	/*-----------------------------------------------------------------*/

	pd->free_area = (ulong *)xmalloc(sizeof(midddf_t),3,pinned_heap);

	if (pd->free_area == NULL)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		return(-1);
	}

	bzero(pd->free_area, sizeof(midddf_t));

	/*-----------------------------------------------------------------*/
	/* Initialize device dependent data				   */
	/*-----------------------------------------------------------------*/

	ddf = (midddf_t *) pd->free_area;
	ddf->phys_disp = pd;

	ddf->dma_sleep_addr = EVENT_NULL;

	ddf->num_DWA_WIDS = MID_NUM_DWA_WIDS_INIT ;

	ddf->mid_watch.dog.func = mid_timeout;
	ddf->mid_watch.dog.restart = 2;

	ddf->bus_offset = ddsptr->io_bus_mem_start;

	slot = ddf->slot = ddsptr->slot_number;
	ddf->hwconfig = ddsptr->hwconfig;

	for (i = 0;i < 16;i ++) {
		ddf->component[i] = ddsptr->component[i];
	}


#ifdef  ENABLE_WATCHDOGS  /* Disable the watchdog timers.  They can cause   */
			  /* problems when the adapter is legitimately      */
			  /* taking more time than the watchdog is set for. */

	/*--------------------------------------------------------------*/
	/* Initialize watchdog timers.                                  */
	/*--------------------------------------------------------------*/

	if ((rc = pincode((void *) mid_timeout)) != 0)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );

		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}

	w_init(&ddf->mid_watch.dog);

	if ((rc = pincode((void *) mid_timeout_getcpos)) != 0)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );

		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}

	if ((rc = pincode((void *) mid_timeout_getcolor)) != 0)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );

		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}

	if ((rc = pincode((void *) mid_timeout_getcondition)) != 0)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );

		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}

	if ((rc = pincode((void *) mid_timeout_gettextfontindex)) != 0)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );

		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}

	if ((rc = pincode((void *) mid_timeout_lowwater)) != 0)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );

		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}

	if ((rc = pincode((void *) mid_timeout_endrender)) != 0)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );

		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}
#endif

	if ((rc = pincode((void *) mid_timeout_swap)) != 0)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );

		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}


	if ((rc = pincode((void *) mid_WID_wakeup)) != 0)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );
		unpincode((void *) mid_timeout_swap);

		BUGLPR(dbg_middd, 0,
			("Did not pin watchdog timer successfully\n"));
		return(rc);
	}



	/*-----------------------------------------------------------------*/
	/* Set up the pos regs for this adapter 			   */
	/*-----------------------------------------------------------------*/

	BUGLPR(dbg_middd, BUGACT, ("Just before POS Register Setup.\n\n\n"));
	BUGLPR(dbg_middd, BUGACT, ("ddsptr->io_bus_mem_start = 0x%x\n",
						ddsptr->io_bus_mem_start));

	BUGLPR(dbg_middd, BUGGID, ("\nSUB_ADDRESS 1: 0x%x\n",
		(0x8000 & ddsptr->io_bus_mem_start)>>8));
	BUGLPR(dbg_middd, BUGGID, ("\nSUB_ADDRESS 2: 0x%x\n",
		(0xFF0000 & ddsptr->io_bus_mem_start)>>16));
	BUGLPR(dbg_middd, BUGGID, ("\nSUB_ADDRESS 3: 0x%x\n",
		(0xFF000000 & ddsptr->io_bus_mem_start)>>24));


	mid_POS_test ( ddf, &pos);

	if ( !( pos & MID_POS_2_OK ) ||
	     !( pos & MID_POS_4_OK ) ||
	     !( pos & MID_POS_5_OK ) ||
	     !( pos & MID_POS_7_OK ))
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );
		unpincode((void *) mid_timeout_swap);
		unpincode((void *) mid_WID_wakeup);

	     	return (-1);
	}

	BUGLPR(dbg_middd, BUGNTX, ("pos test = 0x%x\n", pos));

	HWP = IOCC_ATT( BUS_ID, 0 );

	BUGLPR(dbg_middd, BUGACT, ("Adapter is in slot #%d\n", slot));


	/*-----------------------------------------------------*/
	/* set POS Register 5				       */
	/*-----------------------------------------------------*/

	pptr = HWP + POSREG(5,slot) + IO_IOCC;

	/*-----------------------------------------------------*/
	/* Set register 5 to auto increment, disable channel   */
	/* status, and disable channel checks		       */
	/*-----------------------------------------------------*/

	*pptr = (POS5_AUTOINC | POS5_CHKST_DI | POS5_CHKCH_DI);

	/*-----------------------------------------------------*/
	/* set POS Register 7 for sub-address 0                */
	/*-----------------------------------------------------*/

	pptr = HWP + POSREG(7,slot) + IO_IOCC;
	*pptr = 0;


	/*-----------------------------------------------------*/
	/* set POS Register 4, sub-addresses 0 through 3       */
	/*-----------------------------------------------------*/

	pptr = HWP + POSREG(4,slot) + IO_IOCC;
	*pptr = ((ddsptr->dma_channel << 4) | ddsptr->int_level);

	pptr = HWP + POSREG(4,slot) + IO_IOCC;
	*pptr = ((ddsptr->io_bus_mem_start >> 8) & BYTE_MASK);

	pptr = HWP + POSREG(4,slot) + IO_IOCC;
	*pptr = ((ddsptr->io_bus_mem_start >> 16) & BYTE_MASK);

	pptr = HWP + POSREG(4,slot) + IO_IOCC;
	*pptr = ((ddsptr->io_bus_mem_start >> 24) & BYTE_MASK);


	/*-----------------------------------------------------*/
	/* set POS register 2.                                 */
	/* set card enable and fairness.                       */
	/*-----------------------------------------------------*/

	pptr = HWP + POSREG(2,slot) + IO_IOCC;

	*pptr = (POS2_ENABLE | POS2_FAIR | POS2_RESERVE1 | POS2_TIMER |
		POS2_STREAM | POS2_PARITY );

	IOCC_DET( HWP );
	BUGLPR(dbg_middd, BUGGID, ("slot = %d   level = %d\n", slot, level));


	/*--------------------------------------------------------------*/
        /* MID_INIT removed.  (This set up the defer buffer which is    */
        /*    no longer used in the device driver.)                     */
	/*--------------------------------------------------------------*/

	ddf->HWP = ddsptr->io_bus_mem_start;

	/*--------------------------------------------------------------*/
        /* Allocate and initialize trace buffer.                        */
	/*--------------------------------------------------------------*/

	memtrace_init (ddf) ;

	/*--------------------------------------------------------------*/
        /* Create default context data structures.                      */
	/*--------------------------------------------------------------*/

        mid_create_default_context (ddf) ;

	/*--------------------------------------------------------------*/
	/* Initialize interrupt.                                        */
	/*--------------------------------------------------------------*/

	if ((rc = pincode((void *) mid_intr)) != 0)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );
		unpincode((void *) mid_timeout_swap);
		unpincode((void *) mid_WID_wakeup);

		BUGLPR(dbg_middd, 0,
			("Did not pin interrupt code successfully\n"));
		return(rc);
	}

	rc = i_init(&(pd->interrupt_data));


	if (rc != INTR_SUCC)
	{
		xmfree( ddsptr, pinned_heap );
		xmfree( pd, pinned_heap );
		xmfree( pd->free_area, pinned_heap );
		unpincode((void *) mid_timeout_swap);
		unpincode((void *) mid_WID_wakeup);

		BUGLPR(dbg_middd, 0,
			("Did not init interrupts successfully\n"));
		BUGLPR(dbg_middd, 0,
			("i_init returned %d, errno=%d.\n", rc,errno));

		return(-1);
	}


	/*--------------------------------------------------------------*/
	/* Enable the interrupt level by calling i_unmask               */
	/*--------------------------------------------------------------*/

	i_unmask(&(pd->interrupt_data));

#if 0
	/*--------------------------------------------------------------*/
	/* Initialize the adapter.                              	*/
	/*--------------------------------------------------------------*/

	init_rc = mid_init(pd);

	if (init_rc)
	{
		ddf->hwconfig |= MID_BAD_UCODE;
		BUGLPR(dbg_middd, 0, ("Cannot initialize adapter.\n"));
	}
	else ddf->hwconfig &= ~MID_BAD_UCODE;

	/*--------------------------------------------------------------*/
	/* Initialize the rendering context.                    	*/
	/*--------------------------------------------------------------*/

	if (!init_rc) mid_init_rcx(pd);
#endif

	/*-----------------------------------------------------------------*/
	/* Set dma channel into dds					   */
	/*-----------------------------------------------------------------*/

	pd->d_dma_area[0].bus_addr = ddsptr->dma_range1_start;


	/*-----------------------------------------------------------------*/
	/* Now decide where to put the structure			   */
	/*-----------------------------------------------------------------*/

	devswqry(devno,&status,&nxtpd);


	if (nxtpd != NULL)
	{
		/*---------------------------------------------------------*/
		/* When we reach here at least one other MIDDD3D	   */
		/* has been defined in the system so we need to 	   */
		/* calculate where to hook the display			   */
		/* structure into the chain of displays 		   */
		/*---------------------------------------------------------*/

		while ( nxtpd->next != NULL )
		nxtpd = nxtpd->next;


		/*---------------------------------------------------------*/
		/*  When we fall out of the loop nxtpd will point	   */
		/*  to the structure that the new display should	   */
		/*  be attached to.					   */
		/*---------------------------------------------------------*/

		nxtpd->next = pd;

	}
	else
	{


		/*---------------------------------------------------------*/
		/*  if this pointer is null then the card is		   */
		/*  being configged for the first time. In		   */
		/*  this case we must do a devswadd to get the		   */
		/*  driver into the devswitch.				   */
		/*---------------------------------------------------------*/

		MIDDD3D_devsw.d_open = mid_open;
		MIDDD3D_devsw.d_close = mid_close;
		MIDDD3D_devsw.d_read = nodev;
		MIDDD3D_devsw.d_write = nodev;
		MIDDD3D_devsw.d_ioctl = mid_ioctl;
		MIDDD3D_devsw.d_strategy = nodev;
		MIDDD3D_devsw.d_select = nodev;
		MIDDD3D_devsw.d_config = mid_config;
		MIDDD3D_devsw.d_print = nodev;
		MIDDD3D_devsw.d_dump = nodev;
		MIDDD3D_devsw.d_mpx = nodev;
		MIDDD3D_devsw.d_revoke = nodev;
		MIDDD3D_devsw.d_dsdptr = (char *) pd;
		MIDDD3D_devsw.d_ttys = NULL;
		MIDDD3D_devsw.d_selptr = NULL;
		MIDDD3D_devsw.d_opts = 0;


		/*---------------------------------------------------------*/
		/* call devswadd to get us into the switch table	   */
		/*---------------------------------------------------------*/

		rc = devswadd(devno, &MIDDD3D_devsw);

		if (rc != 0)
		{
			xmfree( ddsptr, pinned_heap );
			xmfree( pd, pinned_heap );
			xmfree( pd->free_area, pinned_heap );
			unpincode((void *) mid_timeout_swap);
			unpincode((void *) mid_WID_wakeup);

			/*-------------------------------------------------*/
			/* Log an error 				   */
			/*-------------------------------------------------*/

			BUGLPR(dbg_middd, 0,
			("Error while adding MIDDD3D to the devswitch\n"));
			return rc;
		}
	}

	BUGLPR(dbg_middd, BUGACT, ("Leaving cfg_init\n"));
	return 0;

}

/*-------------------------------------------------*/
/* Service the timeout (software) interrupt.	 */
/*-------------------------------------------------*/
void mid_timeout(w)
struct mid_watch *w;
{
      BUGLPR(dbg_midddi, BUGNTA, ("entering mid_timeout. \n"));
      e_wakeup( w->sleep_addr );
      BUGLPR(dbg_midddi, BUGNTA, ("leaving  mid_timeout. \n"));
}
