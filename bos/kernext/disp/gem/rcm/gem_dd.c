static char sccsid[] = "@(#)17  1.17.3.16  src/bos/kernext/disp/gem/rcm/gem_dd.c, sysxdispgem, bos41J, 9525E_all 6/20/95 17:20:04";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *               chk_adap
 *              gem_close
 *              gem_config
 *              gem_define
 *              gem_dev_init
 *              gem_dev_term
 *              gem_dma_service
 *              gem_ioctl
 *              gem_open
 *              get_vpd
 *              set_refresh
 *              uload
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/**********************************************************************/
/*                                                                    */
/* NGK 00  06/29/93  097669  Check correlator field for ISO support.  */
/*                                                                    */
/**********************************************************************/

#include <sys/errno.h>
#include <sys/dma.h>
#include <sys/xmem.h>
#include <sys/systemcfg.h>
#include <sys/sleep.h>
#include "gemincl.h"
#include "gem_diag.h"
extern void gem_timeout_imm(struct watch_imm *);
extern void gem_timeout_trav(struct watch_trav *);
extern void gem_timeout_event(struct watch_event *);
 
/**********************************************************************/
/*                                                                    */
/* IDENTIFICATION:  GEM_CONFIG                                        */
/*                                                                    */
/**********************************************************************/
gem_config(devno,cmd, uiop)
dev_t devno;
long cmd;
struct uio *uiop;
{
	struct gem_dds *gem_ddsp;
	int rc, min_num, i;
	struct phys_displays *pd,*last;
	struct gem_ddf    *ddf;
	struct vpd_data   *vpd;
 
#ifdef GEM_DBUG
	printf("gem_config: entered\n");
#endif GEM_DBUG
 
	switch (cmd)
	{
	case CFG_INIT:
		/****************************************************/
		/* Load the GEMINI Device Driver into the system.   */
		/* This is done in gem_define where the physical    */
		/* display data structure is filled in and then the */
		/* device driver is added to the system by filling  */
		/* in the devsw table                               */
		/****************************************************/
 
		/****************************************************/
		/* Copy the 'dds' from user space to kernel space,  */
		/****************************************************/
		gem_ddsp = (struct gem_dds *)
		    xmalloc(sizeof(struct gem_dds),3,pinned_heap);
		if (gem_ddsp == NULL)
		{
			gemlog(NULL,gem_ddsp->comp_name,"gem_config",
				      "xmalloc",NULL,NULL,UNIQUE_1);
			errno = ENOMEM;
			return(ENOMEM);
		}
		rc = uiomove(gem_ddsp,sizeof(struct gem_dds) ,
		    UIO_WRITE, uiop);
		if (rc != 0)
			return(rc);
 
		/****************************************************/
		/* Call gem_define to load device driver            */
		/****************************************************/
		rc = gem_define(devno,gem_ddsp,sizeof(struct gem_dds));
		return(rc);
		break;
 
	case CFG_TERM:
		/****************************************************/
		/* In this case we would remove all malloc'd data   */
		/* and remove ourselves from the device switch table*/
		/* if we are the last device of this type.          */
		/* Calculate pointer to phys_display                */
		/****************************************************/
		rc = devswqry(devno,NULL,&pd);
		if (rc != SUCCESS)
			return(rc);
		last = NULL;
		min_num = 0;
		while (pd)
		{
			if (pd->devno == devno)
			   break;
			last = pd;
			++min_num;
			pd = pd->next;
		}
		if (!pd)
		   return(ERROR);
		if (pd->next)
		   ++min_num;
		if (pd->open_cnt != 0)
		   return(EBUSY);
 
		/****************************************************/
		/* We now have the pointer to the display structure */
		/* free the ddf structure and the display structure */
		/* set the pointer in the previous structure to null*/
		/****************************************************/
		ddf = (struct gem_ddf *) pd->free_area;
		while(w_clear(&ddf->watch_imm.wi));
		while(w_clear(&ddf->watch_trav.wt));
		while(w_clear(&ddf->watch_event.w_event));
		d_clear(pd->dma_chan_id);
		gem_ddsp = (struct gem_dds *) pd->odmdds;
		xmfree(gem_ddsp->u1.gcpucp,pinned_heap);
		xmfree(gem_ddsp->u2.gcptblp,pinned_heap);
		xmfree(gem_ddsp->u3.c25ucp,pinned_heap);
		xmfree(gem_ddsp->u4.shptblp,pinned_heap);
		xmfree(gem_ddsp->u5.gvp5ap,pinned_heap);
		xmfree(gem_ddsp->u6.c25vpd,pinned_heap);
		xmfree(gem_ddsp, pinned_heap);
		xmfree(pd->interrupt_data.intr_args[1],pinned_heap);
		xmfree(pd->free_area,pinned_heap);
                rc = releasepd(devno);
                if (rc != SUCCESS)
                   return(rc);

		break;
 
	case CFG_QVPD:
		/****************************************************/
		/* This function returns the Vital Product Data from*/
		/* the adapter.                                     */
		/****************************************************/
		rc = devswqry(devno,NULL,&pd);
		if (rc != SUCCESS)
			return(rc);
 
		last = NULL;
		while (pd)
		{
			if (pd->devno == devno)
			   break;
			last = pd;
			pd = pd->next;
		}
		if (!pd)
		   return(ERROR);
 
		/*******************************************************/
		/* Malloc storage for the VPD Data structure           */
		/*******************************************************/
		gem_ddsp = (struct gem_dds *) pd->odmdds;
		vpd = (struct vpd_data *)
		    xmalloc(sizeof(struct vpd_data),3,pinned_heap);
		if (vpd == NULL)
		{
			gemlog(NULL,gem_ddsp->comp_name,"gem_config",
				      "xmalloc",NULL,NULL,UNIQUE_2);
			errno = ENOMEM;
			return(ENOMEM);
		}
 
		/*******************************************************/
		/* Collect VPD Data from adapter                       */
		/*******************************************************/
		get_vpd(pd,vpd);
 
		/*******************************************************/
		/* Move VPD data user buffer                           */
		/*******************************************************/
		rc = uiomove(vpd, sizeof(struct vpd_data), UIO_READ, uiop);
		xmfree(vpd,pinned_heap);
		if (rc != SUCCESS)
		     return(rc);
		break;
 
	}
 
	return(SUCCESS);
}
/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: RELEASEPD                                           */
/*                                                                     */
/* DESCRIPTIVE NAME: RELEASEPD - deallocate the phys_displays struct   */
/*                                                                     */
/* FUNCTION: Procedure de-links and dealloactes the pd from the list   */
/*           of pds. Calls devswdel when the last pd is deallocated    */
/*                                                                     */
/* INPUTS:   Device number                                             */
/*                                                                     */
/* OUTPUTS:  None.                                                     */
/*                                                                     */
/* CALLED BY: gem_config                                               */
/*                                                                     */
/* CALLS:    None.                                                     */
/*                                                                     */
/***********************************************************************/

#define INVALID_DEV_T   (-1)

int releasepd (dev_t devno)
{
        struct phys_displays *  first_pd;
        struct phys_displays *  pd;
        struct phys_displays *  last_pd;
        int                     rc;

        rc = devswqry(devno, NULL, &first_pd);
        if (rc != SUCCESS)
                return (rc);
        for (pd = first_pd ; pd != NULL ; pd = pd->next) {
                if (pd->devno == devno)
                        break;
                last_pd = pd;
        }
        if (pd == NULL)
                return (ENODEV);

        if (pd == first_pd) {
                pd->devno = INVALID_DEV_T;
        } else {
                last_pd->next = pd->next;
                xmfree(pd, pinned_heap);
        }
        if ((first_pd->devno == INVALID_DEV_T) && (first_pd->next == NULL)) {
                xmfree(first_pd, pinned_heap);
                devswdel(devno);
        }

        return (SUCCESS);
}
 
/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: GEM_DEFINE                                          */
/*                                                                     */
/* DESCRIPTIVE NAME: GEM_DEFINE - Define Device routine for the        */
/*                   GEMINI adapter.                                   */
/*                                                                     */
/* FUNCTION:                                                           */
/*                                                                     */
/***********************************************************************/
gem_define(devno,gem_ddsp,ddslen)
dev_t devno;
struct gem_dds *gem_ddsp;
long ddslen;
{
	struct phys_displays *pd, *nxtpd;
	long i;
	union  {
		long  full;
		struct {
			char b1;
			char b2;
			char b3;
			char b4;
		} byte;
	} devdat;

	struct gem_dds *prev_ddsp;
	struct devsw gem_devsw;
	int rc;
        ulong fd;
	struct gem_unique *gem_data;
	struct gem_ddf    *ddf;
	struct file *ucd_fp;
	uint  io_addr;
	volatile char *pptr;
	uint count = 0;
	uint level = 0;
	char slot;
	extern nodev();
 
#ifdef GEM_DBUG
	printf("gem_define: entered\n");
	printf("gem_define: gem_ddsp = %08x\n",gem_ddsp);
	printf("gem_define: devno = %08x\n",devno);
#endif GEM_DBUG
 
	/****************************************************************/
	/* Get the Physical Display Structure associated with this      */
	/* device. If the address returned is NULL, then this is the    */
	/* first time this we are being configured. If this is the      */
	/* first time then the microcode must be copied into the kernel */
	/****************************************************************/
	rc = devswqry(devno,NULL,&nxtpd);
	if (rc != SUCCESS)
		return(rc);

	/****************************************************************/
	/* malloc storage for a Physical Display data structure         */
	/****************************************************************/
	pd = (struct phys_displays *) xmalloc(sizeof(struct phys_displays),
	    3,pinned_heap);
 
	/****************************************************************/
	/* Log an error if a malloc failure is reported.                */
	/****************************************************************/
	if (pd == NULL)
	{
		gemlog(NULL,gem_ddsp->comp_name,"gem_define","xmalloc",
						     NULL,NULL,UNIQUE_2);
		errno = ENOMEM;
		return(ENOMEM);
	}
 
	/****************************************************************/
	/* Clear the Physical Display Data Structure.                   */
	/****************************************************************/
	bzero(pd, sizeof(struct phys_displays));
 
	/****************************************************************/
	/* Initialize the Physical Display data structure using data    */
	/* found in the dds.                                            */
	/****************************************************************/
	pd->next = NULL;
	pd->interrupt_data.intr.next     = (struct intr *) NULL;
	pd->interrupt_data.intr.handler  = gem_intr;
	pd->interrupt_data.intr.bus_type = BUS_MICRO_CHANNEL;
	pd->interrupt_data.intr.flags    = 0;
	pd->interrupt_data.intr.level    = gem_ddsp->intr_level;
	pd->interrupt_data.intr.priority = gem_ddsp->intr_priority;
	pd->interrupt_data.intr.bid      = BUS_ID;
 
	pd->same_level = NULL;
	pd->dds_length = ddslen;
	pd->odmdds = (char *) gem_ddsp;
	pd->devno = devno;
	pd->dma_characteristics = DMA_SLAVE_DEV;
 
	/****************************************************************/
	/* Set up pointers to the VDD functions                         */
	/****************************************************************/
	pd->vttact =  vttact;       /*  VDD Activate                    */
	pd->vttcfl =  vttcfl;       /*  Copy Full Lines                 */
	pd->vttclr =  vttclr;       /*  Clear a Rectangle on screen     */
	pd->vttcpl =  vttcpl;       /*  Copy a part of the line         */
	pd->vttdact = vttdact;      /*  VDD Deactivate                  */
	pd->vttddf  = gem_ddf;      /*  VDD Device Dependent Functions  */
	pd->vttdefc = vttdefc;      /*  Change the cursor shape         */
	pd->vttterm = vttterm;      /*  Free any resources used by VDD  */
	pd->vttdma_setup = NULL;    /*  Setup for DMA                   */
	pd->vttdma  = vttdma;       /*  DMA Access routine              */
	pd->vttinit = vttinit;      /*  setup new logical terminal      */
	pd->vttmovc = vttmovc;      /*  Move the cursor                 */
	pd->vttrds  = vttrds;       /*  Read a line segment             */
	pd->vtttext = vtttext;      /*  Write a string of chars         */
	pd->vttscr  = vttscr;       /*  Scroll text on the VT           */
	pd->vttsetm = vttsetm;      /*  Set mode to KSR or MOM          */
	pd->vttstct = vttstct;      /*  Change Color Table              */
 
	/****************************************************************/
	/* Set up pointers to Rendering Context Manager Functions       */
	/****************************************************************/
	pd->make_gp = gem_make_gp;
	pd->unmake_gp = gem_unmake_gp;
	pd->create_rcx = gem_create_rcx;
	pd->delete_rcx = gem_delete_rcx;
	pd->create_win_attr = gem_create_win_attr;
	pd->delete_win_attr = gem_delete_win_attr;
	pd->update_win_attr = gem_update_win_attr;
	pd->start_switch = gem_start_switch;
	pd->end_switch = gem_end_switch;
	pd->check_dev = gem_check_dev;
	pd->create_rcxp = gem_create_rcxp;
	pd->delete_rcxp = gem_delete_rcxp;
	pd->associate_rcxp = gem_associate_rcxp;
	pd->disassociate_rcxp = gem_disassociate_rcxp;
	pd->create_win_geom = gem_create_win_geom;
	pd->delete_win_geom = gem_delete_win_geom;
	pd->update_win_geom = gem_update_win_geom;
	pd->bind_window = gem_bind_window;
	pd->async_mask = gem_async_mask;
	pd->sync_mask = gem_sync_mask;
	pd->enable_event = gem_enable_event;
	pd->dev_init= gem_dev_init;
	pd->dev_term= gem_dev_term;
 
	/****************************************************************/
	/* Initialize the Display info data structure                   */
	/****************************************************************/
	pd->display_info.font_width = 12;
	pd->display_info.font_height = 30;
	pd->display_info.bits_per_pel = 1;
	pd->display_info.adapter_status = 0xC0000000;
	pd->display_info.apa_blink = 0x80000000;
	pd->display_info.screen_width_pel = 1280;
	pd->display_info.screen_height_pel = 1024;
 
	pd->display_info.screen_width_mm = gem_ddsp->screen_width_mm;
	pd->display_info.screen_height_mm = gem_ddsp->screen_height_mm;
	pd->display_info.colors_total = 16777215;
	pd->display_info.colors_active = gem_ddsp->color_count;
	pd->display_info.colors_fg = gem_ddsp->color_count;
	pd->display_info.colors_bg = gem_ddsp->color_count;
	for (i=0; i < 16; i++)
		pd->display_info.color_table[i] = gem_ddsp->color_table[i];
 
	devdat.full = gem_ddsp->display_id;
	pd->disp_devid[0] = devdat.byte.b1;
	pd->disp_devid[1] = devdat.byte.b2;
	pd->disp_devid[2] = devdat.byte.b3;
	pd->disp_devid[3] = devdat.byte.b4;

	/****************************************************************/
	/* GEMINI physical device id                                    */
	/****************************************************************/
	pd->usage = 0;               /* Set to indicate no VTs open     */
	pd->open_cnt = 0;            /* Set to indicate driver is closed*/
 
	/****************************************************************/
	/* Malloc storage for the GEMINI Device Dependent Functions and */
	/* initialize the ddf to zeros                                  */
	/****************************************************************/
	pd->free_area = (ulong *) xmalloc(sizeof(struct gem_ddf),
	    3,pinned_heap);
 
	/****************************************************************/
	/* Log an error if a malloc failure is reported.                */
	/****************************************************************/
	if (pd->free_area == NULL)
	{
		gemlog(NULL,gem_ddsp->comp_name,"gem_define","xmalloc",
						       NULL,NULL,UNIQUE_3);
		errno = ENOMEM;
		return(ENOMEM);
	}
	bzero(pd->free_area,sizeof(struct gem_ddf));
 
	/****************************************************************/
	/* Initialize watch dog timer functions                         */
	/****************************************************************/
	ddf = (struct gem_ddf *) pd->free_area;
 
	ddf->watch_imm.wi.func = gem_timeout_imm;
	ddf->watch_imm.wi.restart = WATCH_RESTART_TIME;
	while(w_init(&ddf->watch_imm.wi));
 
	ddf->watch_trav.wt.func = gem_timeout_trav;
	ddf->watch_trav.wt.restart = WATCH_RESTART_TIME;
	while(w_init(&ddf->watch_trav.wt));
 
	ddf->watch_event.w_event.func = gem_timeout_event;
	ddf->watch_event.w_event.restart = WATCH_RESTART_TIME;
	while(w_init(&ddf->watch_event.w_event));
 
	/****************************************************************/
	/* Find the slot number where the GEMINI Interface Card is      */
	/* installed. Return error if adapter not found!! Get value to  */
	/* set in pos reg which will determine where GEMINI I/O memory  */
	/* is to be mapped.                                             */
	/* NOTE: This value is multiplied by 2 meg                      */
	/****************************************************************/
	if (gem_ddsp->slot_number == -1)
	{
		gem_ddsp->slot_number = getport();
		if (gem_ddsp->slot_number == -1)
			return(ERROR);
	}
	slot = gem_ddsp->slot_number;
	count = gem_ddsp->mem_map_start;
 
	/****************************************************************/
	/* Set up interrupt variable for use when setting POS 2         */
	/****************************************************************/
	switch (gem_ddsp->intr_level)
	{
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		level = ((gem_ddsp->intr_level) - 3) << 1;
		break;
	case 9:
	case 10:
	case 11:
		level = ((gem_ddsp->intr_level) - 4) << 1;
		break;
	default:
		level = 0x0A;
		gem_ddsp->intr_level = level;
		break;
	}
 
	/****************************************************************/
	/* Initialize the POS Registers for the GEMINI adapter. First   */
	/* disable adapter, set up memory address, set up interrupt     */
	/* level and enable the adapter.                                */
	/****************************************************************/
	io_addr = IOCC_ATT(BUS_ID,0);
 
	/****************************************************************/
	/* Select POS Reg 0 and disable adapter                         */
	/****************************************************************/
	pptr  = io_addr + POSREG(2,slot) + IO_IOCC;
	*pptr = PosDisable;               /* Disable the adapter (POS0) */
 
	/****************************************************************/
	/* Select POS Reg 4 and clear it                                */
	/****************************************************************/
	pptr  = io_addr + POSREG(6,slot) + IO_IOCC;
	*pptr = 0x00;                     /* Enable Option Select(POS4) */
 
	/****************************************************************/
	/* Select POS Reg 1 and initialize it as follows:               */
	/*   Memory Select   = B'1' - 4 meg                             */
	/*   Interrupt Level = from DDS                                 */
	/*   Parity Enable   = B'0' - enable parity                     */
	/*   Data Stream     = B'11'                                    */
	/*   Fairness        = B'1'                                     */
	/****************************************************************/
	pptr    = io_addr + POSREG(3,slot) + IO_IOCC;
	*pptr++ = 0xE1 | level;            /* Initialize        (POS1)  */
 
	/****************************************************************/
	/* Initialize Pos Reg 2 and 3                                   */
	/****************************************************************/
	*pptr++ = count;                  /* Set up IO addresses (POS2) */
	*pptr = 0xF0 | gem_ddsp->dma_channel;             /* POS3       */
 
	/****************************************************************/
	/* Select Pos Reg 0 and Enable adapter                          */
	/****************************************************************/
	pptr    = io_addr + POSREG(2,slot) + IO_IOCC;
	*pptr   = PosEnable;              /* Enable adapter      (POS0) */
 
	IOCC_DET(io_addr);
 
	gem_ddsp->io_bus_mem_start = (count << 22);
 
	/****************************************************************/
	/* Malloc storage to hold geographical addresses and initialize */
	/* Physical Display structure to be used by Interrupt Handler   */
	/****************************************************************/
	pd->interrupt_data.intr_args[0] = gem_ddsp->io_bus_mem_start;
	pd->interrupt_data.intr_args[1] = (ulong *)
			 xmalloc(sizeof(struct gmcrdslots),3,pinned_heap);
	if (pd->interrupt_data.intr_args[1] == NULL)
	{
		gemlog(NULL,gem_ddsp->comp_name,"gem_define","xmalloc",
						     NULL,NULL,UNIQUE_4);
		errno = ENOMEM;
		return(ENOMEM);
	}
 
	/****************************************************************/
	/* Initialize a DMA Channel                                     */
	/****************************************************************/
	pd->dma_chan_id = d_init(gem_ddsp->dma_channel,
			    MICRO_CHANNEL_DMA | DMA_SLAVE, BUS_ID);
	if (pd->dma_chan_id == DMA_FAIL)
	     gemlog(NULL,gem_ddsp->comp_name,"gem_define","d_init",
						NULL,NULL,UNIQUE_5);
	if (nxtpd == NULL)
	{
	    /************************************************************/
	    /* Copy Microcode files into malloced areas and update dds  */
	    /************************************************************/
	    fp_getf(gem_ddsp->u1.gcpucfd,&ucd_fp);
            fd = gem_ddsp->u1.gcpucfd;

	    /************************************************************/
	    /* Malloc storage for the GCP Microcode                     */
	    /************************************************************/
	    gem_ddsp->u1.gcpucp = (char *)
		 xmalloc(gem_ddsp->gcpuclen,3,pinned_heap);

	    if (gem_ddsp->u1.gcpucp == NULL)
	    {
		  gemlog(NULL,gem_ddsp->comp_name,"gem_define","xmalloc",
						    NULL,NULL,UNIQUE_6);
		  errno = ENOMEM;
		  return(ENOMEM);
	    }

	    /************************************************************/
	    /* Read GCP microcode from file into kernel storage         */
	    /************************************************************/
	    fp_read(ucd_fp,gem_ddsp->u1.gcpucp,gem_ddsp->gcpuclen,
			 0,UIO_SYSSPACE,&count);

            /************************************************************/
            /* Release the GCP ucode File Descriptor                    */
            /************************************************************/
            ufdrele(fd);
            
	    /************************************************************/
	    /* Malloc storage for the GCP Tables                        */
	    /************************************************************/
	    fp_getf(gem_ddsp->u2.gcptblfd,&ucd_fp);
            fd = gem_ddsp->u2.gcptblfd;
	    gem_ddsp->u2.gcptblp = (char *)
			 xmalloc(gem_ddsp->gcptbllen,3,pinned_heap);

	    if (gem_ddsp->u2.gcptblp == NULL)
	    {
		  gemlog(NULL,gem_ddsp->comp_name,"gem_define","xmalloc",
						     NULL,NULL,UNIQUE_7);
		  errno = ENOMEM;
		  return(ENOMEM);
	    }

	   /*************************************************************/
	   /* Read GCP table information from file into kernel storage  */
	   /*************************************************************/
	   fp_read(ucd_fp,gem_ddsp->u2.gcptblp,gem_ddsp->gcptbllen,
		   0,UIO_SYSSPACE,&count);

            /************************************************************/
            /* Release the GCP Table File Descriptor                    */
            /************************************************************/
            ufdrele(fd);
            
	   /*************************************************************/
	   /* Malloc storage for the C25 Microcode                      */
	   /*************************************************************/
	   fp_getf(gem_ddsp->u3.c25ucfd,&ucd_fp);
           fd = gem_ddsp->u3.c25ucfd; 
	   gem_ddsp->u3.c25ucp = (char *)
			  xmalloc(gem_ddsp->c25uclen,3,pinned_heap);

	   if (gem_ddsp->u3.c25ucp == NULL)
	   {
		   gemlog(NULL,gem_ddsp->comp_name,"gem_define","xmalloc",
						       NULL,NULL,UNIQUE_8);
		   errno = ENOMEM;
		   return(ENOMEM);
	   }

	   /*************************************************************/
	   /* Read Drp/SHP microcode from file into kernel storage      */
	   /*************************************************************/
	   fp_read(ucd_fp,gem_ddsp->u3.c25ucp,gem_ddsp->c25uclen,
		    0,UIO_SYSSPACE,&count);

            /************************************************************/
            /* Release the Drp/SHP File Descriptor                      */
            /************************************************************/
            ufdrele(fd);

	   /*************************************************************/
	   /* Malloc storage for the SHP Tables                         */
	   /*************************************************************/
	   fp_getf(gem_ddsp->u4.shptblfd,&ucd_fp);
           fd = gem_ddsp->u4.shptblfd;
	   gem_ddsp->u4.shptblp = (char *)
			 xmalloc(gem_ddsp->shptbllen,3,pinned_heap);

	   if (gem_ddsp->u4.shptblp == NULL)
	   {
		   gemlog(NULL,gem_ddsp->comp_name,"gem_define","xmalloc",
						       NULL,NULL,UNIQUE_9);
		   errno = ENOMEM;
		   return(ENOMEM);
	   }

	   /*************************************************************/
	   /* Read SHP table information from file into kernel storage  */
	   /*************************************************************/
	   fp_read(ucd_fp,gem_ddsp->u4.shptblp,gem_ddsp->shptbllen,
		0,UIO_SYSSPACE,&count);

            /************************************************************/
            /* Release the SHP Table File Descriptor                    */
            /************************************************************/
            ufdrele(fd);

	   /*************************************************************/
	   /* Malloc storage for the GCP GVP Tables                     */
	   /*************************************************************/
	   fp_getf(gem_ddsp->u5.gvp5afd,&ucd_fp);
           fd = gem_ddsp->u5.gvp5afd;
	   gem_ddsp->u5.gvp5ap = (char *)
			  xmalloc(gem_ddsp->gvp5alen,3,pinned_heap);

	   if (gem_ddsp->u5.gvp5ap == NULL)
	   {
		   gemlog(NULL,gem_ddsp->comp_name,"gem_define","xmalloc",
						     NULL,NULL,UNIQUE_10);
		   errno = ENOMEM;
		   return(ENOMEM);
	   }

	   /*************************************************************/
	   /* Read GCP table information from file into kernel storage  */
	   /*************************************************************/
	   fp_read(ucd_fp,gem_ddsp->u5.gvp5ap,gem_ddsp->gvp5alen,
						  0,UIO_SYSSPACE,&count);

            /************************************************************/
            /* Release the GCP GVP Table File Descriptor                */
            /************************************************************/
            ufdrele(fd);

	   /*************************************************************/
	   /* Malloc storage for the C25 VPD Microcode                  */
	   /*************************************************************/
	   fp_getf(gem_ddsp->u6.c25vpdfd,&ucd_fp);
           fd = gem_ddsp->u6.c25vpdfd;
	   gem_ddsp->u6.c25vpd = (char *)
		      xmalloc(gem_ddsp->c25vpdlen,3,pinned_heap);

	   if (gem_ddsp->u6.c25vpd == NULL)
	   {
		   gemlog(NULL,gem_ddsp->comp_name,"gem_define","xmalloc",
						     NULL,NULL,UNIQUE_11);
		   errno = ENOMEM;
		   return(ENOMEM);
	   }

	   /*************************************************************/
	   /* Read VPD C25 Microcode into kernel storage                */
	   /*************************************************************/
	   fp_read(ucd_fp,gem_ddsp->u6.c25vpd,gem_ddsp->c25vpdlen,
						   0,UIO_SYSSPACE,&count);

            /************************************************************/
            /* Release the VPD C25 Ucode File Descriptor                */
            /************************************************************/
            ufdrele(fd);

	   /*************************************************************/
	   /* The GEMINI adapter is being configured for the first time.*/
	   /* Thus a devswadd must be done to get the device driver into*/
	   /* the devsw table                                           */
	   /*************************************************************/
	   gem_devsw.d_open = gem_open;
	   gem_devsw.d_close = gem_close;
	   gem_devsw.d_read = nodev;
	   gem_devsw.d_write = nodev;
	   gem_devsw.d_ioctl = gem_ioctl;
	   gem_devsw.d_strategy = nodev;
	   gem_devsw.d_select = nodev;
	   gem_devsw.d_config = gem_config;
	   gem_devsw.d_print = nodev;
	   gem_devsw.d_dump = nodev;
	   gem_devsw.d_mpx = nodev;
	   gem_devsw.d_revoke = nodev;
	   gem_devsw.d_dsdptr = (char *) pd;
	   gem_devsw.d_ttys = NULL;
	   gem_devsw.d_selptr = NULL;
	   gem_devsw.d_opts = 0;

	   rc = devswadd(devno, &gem_devsw);
	   if(rc != 0)
		  return(rc);

	}
	else
	   {
		prev_ddsp = (struct gem_dds *) nxtpd->odmdds;
		gem_ddsp->u1.gcpucp = prev_ddsp->u1.gcpucp;
		gem_ddsp->gcpuclen = prev_ddsp->gcpuclen;

		gem_ddsp->u2.gcptblp = prev_ddsp->u2.gcptblp;
		gem_ddsp->gcptbllen = prev_ddsp->gcptbllen;

		gem_ddsp->u3.c25ucp = prev_ddsp->u3.c25ucp;
		gem_ddsp->c25uclen = prev_ddsp->c25uclen;

		gem_ddsp->u4.shptblp = prev_ddsp->u4.shptblp;
		gem_ddsp->shptbllen = prev_ddsp->shptbllen;

		gem_ddsp->u5.gvp5ap = prev_ddsp->u5.gvp5ap;
		gem_ddsp->gvp5alen = prev_ddsp->gvp5alen;

		gem_ddsp->u6.c25vpd = prev_ddsp->u6.c25vpd;
		gem_ddsp->c25vpdlen = prev_ddsp->c25vpdlen;

		/*********************************************************/
		/* At least one other GTO adapter has been defined to the*/
		/* system. Traverse the linked list of Physical Display  */
		/* structures and added this new one to the linked list. */
		/*********************************************************/

		while ( nxtpd->next != NULL )
		{
		nxtpd = nxtpd->next;
		}
		nxtpd->next = pd;

	   }

	/***************************************************************/
	/* POS registers are set and card enabled. Now attempt to      */
	/* access the adapter to determine if it is really out there...*/
	/***************************************************************/
	ddf->adapter_ready = chk_adap(pd);

#ifdef GEM_DBUG
	printf("gem_define: exited\n");
#endif GEM_DBUG

	return(SUCCESS);
}
 
/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: GEM_OPEN                                            */
/*                                                                     */
/* DESCRIPTIVE NAME: GEM_OPEN - GEMINI Open routine                    */
/*        This routine will eventually only set the device open field. */
/*        For hft bring up it will invoke the gem_config routine which */
/*        eventually be invoked by the adapters command. After config  */
/*        completes installing the driver into the devsw and allocates */
/*        the phys_display table, open will call gem_init to initialize*/
/*        the interrupt code and run diagnostic code to reset the      */
/*        adapter.                                                     */
/***********************************************************************/
/***********************************************************************/
/* Domain Offsets                                                      */
/***********************************************************************/
#define IMM_CREG_START 0x000000
#define IMM_CREG_LEN   0x001000
#define TRV_FIFO_START 0x001000    /* 0x01000 - 0x014ff Traversal regs */
#define TRV_FIFO_LEN   0x1de000    /* 0x01500 - 0x10000 ucode scratch  */
				   /* 0x10000 - 0x1ffff Trav SE fifo   */
				   /* 0x20000 - 0x2ffff Trav Blt fifo  */
				   /* rest is for contexts & Imm Fifo  */
/***********************************************************************/
/* TRV_GBL and TRV_FIFO are now one domain                             */
/***********************************************************************/
#define TRV_GBL_START  TRV_FIFO_START
#define TRV_GBL_LEN    TRV_FIFO_LEN
#define IMM_FIFO_ADDR  0x1df000
#define IMM_FIFO_LEN   0x021000
gem_open(devno,flag,chan,ext)
dev_t devno;                 /* Major and minor number of the device   */
long flag;                   /* flags for type of open.                */
long chan;                   /* Channel number ignored by this routine */
struct diag_open *ext;       /* Used by adapter diagnostics            */
{
        struct gem_dds *gem_ddsp;
        ulong position_ofst, memory_pad;
	ulong auth;
	short	i;
	long   rc;
	struct phys_displays *pd;
	struct gem_ddf    *ddf;
 
#ifdef GEM_DBUG
	printf("gem_open: entered\n");
	if (ext != NULL)
	   printf("Diagnostic open id = %08x\n",ext->diag_id);
#endif GEM_DBUG
 
	/****************************************************************/
	/* Get pointer to the phys_displays data structure              */
	/****************************************************************/
	rc = devswqry(devno,NULL,&pd);
	if (rc != SUCCESS)
		return(rc);
	while ( pd != NULL )
	{
	  if (pd->devno == devno)
	     break;
	  else
	    pd = pd->next;
	}

	ddf = (struct gem_ddf *) pd->free_area;
	if (ddf->adapter_ready != 0x00)
	{
	   errno = ddf->adapter_ready;
	   return(ddf->adapter_ready);
	}

	if (pd->open_cnt == 0)
	{
 
		/********************************************************/
		/* Load Microcode and initialize the adapter            */
		/********************************************************/
		if (ext == NULL)
		{
		      rc = uload(pd);
		      if (rc != SUCCESS)
			    return(rc);
		}
		else
		    if (ext->flags == 0x01)
			  rc = uload(pd);
 
		/********************************************************/
		/* Initialize for interrupts                            */
		/********************************************************/
		if (i_init(&(pd->interrupt_data)) != INTR_SUCC)
			return(ERROR);
 
		/********************************************************/
		/* Increment open count in the Physical Display Data    */
		/* structure, pin the interrupt handler code and Enable */
		/* the interrupt level                                  */
		/********************************************************/
		pd->open_cnt++;
		pincode((void *)gem_intr);
		i_unmask(&(pd->interrupt_data));
 
		/*********************************************************/
		/* Initialize number of domains and bus mem addr ranges  */
		/*********************************************************/
		gem_ddsp = (struct gem_dds *) pd->odmdds;
		position_ofst = gem_ddsp->io_bus_mem_start;
		pd->num_domains = GM_MAX_DOMAINS;
		pd->dwa_device = 1;
		pd->io_range = 0xffff0000;
		
		/* Set var to shift IMM_FIFO_DOMAIN to end on 4 meg adpt*/
		if(gem_ddsp->features & MEMORY_4_MEG)
		  memory_pad = 0x00200000;
		else
		  memory_pad = 0x00000000;
		
		pd->busmemr[IMMEDIATE_CREG_DOMAIN].bus_mem_start_ram =
		  (caddr_t) (position_ofst + IMM_CREG_START);
		pd->busmemr[IMMEDIATE_CREG_DOMAIN].bus_mem_end_ram =
		  (caddr_t) (position_ofst + IMM_CREG_START +
			     IMM_CREG_LEN - 1);
		
		pd->busmemr[TRAVERSAL_FIFO_DOMAIN].bus_mem_start_ram =
		  (caddr_t) (position_ofst + TRV_FIFO_START);
		pd->busmemr[TRAVERSAL_FIFO_DOMAIN].bus_mem_end_ram =
		  (caddr_t) (position_ofst + TRV_FIFO_START +
			     memory_pad + TRV_FIFO_LEN - 1);
		
		pd->busmemr[IMMEDIATE_FIFO_DOMAIN].bus_mem_start_ram =
		  (caddr_t) (position_ofst + memory_pad + IMM_FIFO_ADDR);
		pd->busmemr[IMMEDIATE_FIFO_DOMAIN].bus_mem_end_ram =
		  (caddr_t) (position_ofst + IMM_FIFO_ADDR +
			     IMM_FIFO_LEN - 1);

		/*********************************************************/
		/* Check the features flags in the DDS. If the DrP       */
		/* hardware supports 77Hz refresh rates, then set the    */
		/* default refresh rate as specified in the DDS by       */
		/* configuration.                                        */
		/*********************************************************/
		if (gem_ddsp->features & ISO_SUPPORTED)
		   if (gem_ddsp->features & SEVENTY7_HZ_REQUEST)
		      set_refresh(pd,SEVENTY7_HZ);
		   else
		      set_refresh(pd,SIXTY_HZ);

#ifdef GEM_DBUG
		printf("gem_open: exited\n");
#endif GEM_DBUG
		return(SUCCESS);
 
	}
	else
	   /************************************************************/
	   /* Allow another open if it is for diagnostics              */
	   /************************************************************/
	   if (ext != NULL)
	   {
		if (ext->flags == 0x01)
		    rc = uload(pd);
		return(SUCCESS);
	   }
	   else
		return(SUCCESS);
}
 
/*************************************************************************/
/*                                                                       */
/* IDENTIFICATION: GEM_IOCTL                                             */
/*                                                                       */
/* DESCRIPTIVE NAME: GEM_IOCTL- GEMINI DD ioctl routine                  */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
#define DWA_SEG_REG_BP (unsigned) (0xC20C0000 | 0x00000020)
#define DWA_SEG_REG_PPC (unsigned) (0x82000000)
gem_ioctl(devno,cmd,arg,devflag,chan,ext)
dev_t devno;                   /* The device number of the adapter       */
long cmd;                      /* The command for the ioctl              */
long arg;                      /* The address of paramter block          */
ulong devflag;                 /* Flag indicating type of operation      */
long chan;                     /* Channel number ignored by this routine */
long ext;                      /* Extended system parameter - not used   */
{
	struct gem_dma_diag dma_diag;
	uint i;
	struct phys_displays *pd;
	struct gem_dds *gem_ddsp;
	struct gem_ddf *ddf;
	uint  io_addr, rc;
 
#ifdef GEM_DBUG
	printf("gem_ioctl: entered\n");
#endif GEM_DBUG
 
	/***************************************************************/
	/* Get the phys_display pointer                                */
	/***************************************************************/
	rc = devswqry(devno,NULL,&pd);
	if (rc != SUCCESS)
		return(rc);
	while ( pd != NULL )
	{
	  if (pd->devno == devno)
	     break;
	  else
	    pd = pd->next;
	}

	gem_ddsp = (struct gem_dds *) pd->odmdds;
	ddf = (struct gem_ddf *) pd->free_area;
 
	switch (cmd)
	{
	   case GEM_START_DIAG:
		/*******************************************************/
		/* Malloc storage for an interrupt diag data struct    */
		/*******************************************************/
		ddf->diaginfo = (ulong *)
				xmalloc(sizeof(struct gem_intr_count),
						       3,pinned_heap);
		if (ddf->diaginfo == NULL)
		{
		       gemlog(NULL,gem_ddsp->comp_name,
					 "gem_ioctl",xmalloc,
					  NULL,NULL,UNIQUE_1);
		       return(ENOMEM);
		}
 
		bzero(ddf->diaginfo,sizeof(struct gem_intr_count));
		ddf->cmd = GEM_DIAG_MODE;
		break;
 
	   case GEM_QUERY_DIAG:
		/*******************************************************/
		/* Return interrupt data to user                       */
		/*******************************************************/
		rc = copyout(ddf->diaginfo, arg,
				       sizeof(struct gem_intr_count));
		if (rc != SUCCESS)
			return(rc);
 
		bzero(ddf->diaginfo,sizeof(struct gem_intr_count));
		break;
 
	   case GEM_STOP_DIAG:
		/*******************************************************/
		/* Free interrupt diag structure and exit diag mode    */
		/*******************************************************/
		xmfree((char *)ddf->diaginfo, pinned_heap);
		ddf->cmd = NULL;
		break;
 
	   case GEM_LOAD_UCODE:
		/*******************************************************/
		/* Load ucode requested                                */
		/*******************************************************/
		uload(pd);
		break;
 
	case GEM_GET_STARTADDR:
		/*******************************************************/
		/* Access to adapter memory requested                  */
		/*******************************************************/
                if (__power_pc ())
                   io_addr = as_att(getadsp(),DWA_SEG_REG_PPC,(caddr_t) 0);
                else
                   io_addr = as_att(getadsp(),DWA_SEG_REG_BP,(caddr_t) 0);
		io_addr |= gem_ddsp->io_bus_mem_start;
		rc = copyout(&io_addr, arg, sizeof(io_addr));
		if (rc != SUCCESS)
			return(rc);
		break;
 
	case GEM_RET_STARTADDR:
		/*******************************************************/
		/* Return segment register                             */
		/*******************************************************/
		rc = copyin(arg, &io_addr, sizeof(io_addr));
		if (rc != SUCCESS)
			return(rc);
		io_addr &= 0xF0000000;
		as_det(getadsp(),io_addr);
		break;
 
	case GEM_DMA_DIAG:
		/*******************************************************/
		/* DMA Diagnostics requested                           */
		/*******************************************************/
		rc = copyin(arg, &dma_diag,sizeof(struct gem_dma_diag));
		if (rc != SUCCESS)
			return(rc);
 
		gem_dma_service(pd,&dma_diag,ddf,gem_ddsp->io_bus_mem_start);
		break;
 
	case SET_DISPLAY_HZ:
		/*******************************************************/
		/* Change refresh rate of monitor                      */
		/*******************************************************/
		rc = copyin(arg, &i, sizeof(i));
		if (rc != SUCCESS)
			return(rc);
		if ((i == SIXTY_HZ) | (i == SEVENTY7_HZ))
		{
			set_refresh(pd,i);
			if (i == SEVENTY7_HZ)
			   gem_ddsp->features |= SEVENTY7_HZ_REQUEST;
			else
			   gem_ddsp->features &= ~SEVENTY7_HZ_REQUEST;

		}
		else
			return(EINVAL);
		break;

	case HOOKUP:
		break;
 
	default:
		return(ERROR);
	}
 
	return(SUCCESS);
}
 
/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: GEM_CLOSE                                           */
/*                                                                     */
/* DESCRIPTIVE NAME: GEM_CLOSE - GEMINI adapter close routine          */
/*                   This routine will check the usage field in the    */
/*                   Phys_display  struct and see if it is non-zero.   */
/*                   If so we can close the device if not then a -1    */
/*                   is return to the caller                           */
/*                                                                     */
/***********************************************************************/
gem_close(devno,chan,ext)
dev_t devno;
long chan;                  /* Channel number ignored by this routine */
long ext;                   /* Extended system parameter - not used   */
{
	int rc;
	struct phys_displays *pd;
 
#ifdef GEM_DBUG
	printf("gem_close: entered\n");
#endif GEM_DBUG
 
	/**********************************************************/
	/* Here will be the devsw traversal for devices which can */
	/* multiple instances                                     */
	/**********************************************************/
	rc = devswqry(devno,NULL,&pd);
	if (rc != SUCCESS)
		return(rc);
	while ( pd != NULL )
	{
	  if (pd->devno == devno)
	     break;
	  else
	    pd = pd->next;
	}

	/**********************************************************/
	/* test to see if the device is in use. Return error      */
	/**********************************************************/
	if (pd->usage > 0)
		return(ERROR);
 
	/**********************************************************/
	/* If this is the last process to close the driver, then  */
	/* unpin the interrupt handler and reset interrupt level  */
	/**********************************************************/
	if (pd->open_cnt == 1)
	{
		unpincode((void *)gem_intr);
		i_clear(&(pd->interrupt_data));
	}
	if (pd->open_cnt > 0)
	       pd->open_cnt--;
 
#ifdef GEM_DBUG
	printf("gem_close: exited\n");
#endif GEM_DBUG
 
	return(SUCCESS);
}
 
 
/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: GEM_DMA_DIAG                                        */
/*                                                                     */
/***********************************************************************/
gem_dma_service(pd,dma_diag,ddf,start_addr)
struct phys_displays *pd;
struct gem_dma_diag *dma_diag;
struct gem_ddf *ddf;
uint start_addr;
{
	gscDev *pdev;              /* pointer to gsc device structure */
	int  dma_flags;            /* parameter to d_slave service    */
	int rc, i;
	ulong old_int;             /* old interrupt level             */
	struct xmem dp;
	uint seg_reg;
	volatile unsigned int *dma_intr_ena;
	volatile unsigned int *dma_dest_addr;
	label_t jmpbuf;
 
#ifdef GEM_DBUG
      printf("entering gem_dma_service dma_diag = 0x%08x\n",dma_diag);
#endif GEM_DBUG
 
	/************************************************************/
	/* Set up dma flags parameter to pass to d_slave            */
	/************************************************************/
	if (dma_diag->dma_ctls.dma_diag_flags == GEM_DMA_READ)
	{
		dma_flags = DMA_READ;
#ifdef GEM_DBUG
		printf("DMA_READ requested\n");
#endif GEM_DBUG
	}
	else
	{
		dma_flags = DMA_WRITE_ONLY;
#ifdef GEM_DBUG
		printf("DMA_WRITE_ONLY requested\n");
#endif GEM_DBUG
	}
 
	/************************************************************/
	/* Call xmattach to attach the user buffer for a cross      */
	/* memory operation.                                        */
	/************************************************************/
	dp.aspace_id = XMEM_INVAL;
	rc = xmattach(dma_diag->dma_buff_ptr,
			    dma_diag->dma_buff_len,&dp, USER_ADSPACE);
	if (rc != XMEM_SUCC)
	{
		dma_diag->dma_ctls.dma_diag_error = XMATTACH_ERROR;
		return(rc);
	}
 
	/************************************************************/
	/* Call pinu to pin the user buffer                         */
	/************************************************************/
	rc = pinu(dma_diag->dma_buff_ptr,
			dma_diag->dma_buff_len,(short)UIO_USERSPACE);
	if (rc != SUCCESS)
	{
		dma_diag->dma_ctls.dma_diag_error = PINU_ERROR;
		return(rc);
	}
 
	/**********************************************************/
	/* Call d_slave function to map a DMA transfer into the   */
	/* TCW's (transfer control words), flush the system cache,*/
	/* and make the buffer inaccessible to the processor to   */
	/* prevent corruption of the data being transferred.      */
	/**********************************************************/
#ifdef GEM_DBUG
	printf("About to call d_slave\n");
	printf("  D_SLAVE PARMS  channel id = %d dma_flags = %08x\n",
				 pd->dma_chan_id, dma_flags);
 
	printf("  D_SLAVE PARMS  buff_ptr = %08x buf_len = %08x dp addr\n",
		       dma_diag->dma_buff_ptr,dma_diag->dma_buff_len, &dp);
#endif GEM_DBUG
 
	d_slave(pd->dma_chan_id, dma_flags,dma_diag->dma_buff_ptr,
				       dma_diag->dma_buff_len, &dp);
 
#ifdef GEM_DBUG
	printf("d_slave returned\n");
#endif GEM_DBUG
 
	/*******************************************************/
	/* The request is to start the DMA operation and 'wait'*/
	/* for it to complete. To do this we will mask off     */
	/* system interrupts, enable DMA Complete interrupt on */
	/* the adapter and start the operation by setting the  */
	/* DMA Destination Address register and then sleeping. */
	/*******************************************************/
	ddf->cmd = DMA_IN_PROGRESS | SYNC_WAIT_CMD;
	old_int  = i_disable(INTMAX);
	ddf->sleep_addr = EVENT_NULL;
 
	if (rc = (setjmpx(&jmpbuf)))
	{
	    if (rc == EXCEPT_IO)
	    {
		dma_diag->dma_ctls.dma_diag_error = BUS_PARITY_ERROR;
		return(rc);
	    }
	    else
	      longjmpx(rc);
	}
 
	/***********************************************************/
	/* Get access to the adapter and enable DMA interrupts     */
	/***********************************************************/
	seg_reg = BUSMEM_ATT(BUS_ID,0);
	dma_intr_ena = (unsigned int *)((start_addr) +
					     ENA_DMA_COMP | seg_reg);
	*dma_intr_ena = ENA_DMA_INTR;
 
	/***********************************************************/
	/* Start the DMA operation by writing to the DMA           */
	/* Destination Address register on the adapter             */
	/***********************************************************/
	dma_dest_addr = (unsigned int *)((start_addr) +
					  DMA_DEST_ADDR | seg_reg);
	*dma_dest_addr = dma_diag->dma_dest_ptr;
	clrjmpx(&jmpbuf);
	BUSMEM_DET(seg_reg);
 
	/*******************************************************/
	/* Sleep here to wait for DMA to complete              */
	/*******************************************************/
	ddf->watch_event.sleep_addr = (caddr_t) &ddf->sleep_addr;
	ddf->watch_event.ddf = ddf;
	w_start(&ddf->watch_event.w_event);
	rc = e_sleep(&(ddf->sleep_addr), EVENT_SIGRET);
	w_stop(&ddf->watch_event.w_event);
	i_enable(old_int);
	if (rc == EVENT_SUCC)
	{
		/***********************************************/
		/* The DMA Operation has completed             */
		/***********************************************/
		ddf->cmd = NULL_CMD;
		ddf->sleep_addr = EVENT_NULL;
	}
	else
	   {
		dma_diag->dma_ctls.dma_diag_error = WAIT_EVENT_ERROR;
		return(rc);
	   }
 
	/************************************************************/
	/* Call d_complete                                          */
	/************************************************************/
	rc = d_complete(pd->dma_chan_id, dma_flags,
			     dma_diag->dma_buff_ptr,
				 dma_diag->dma_buff_len, &dp, NULL);
	if (rc != DMA_SUCC)
	{
		dma_diag->dma_ctls.dma_diag_error = DMA_ERROR;
		return(rc);
	}
 
	/************************************************************/
	/* Detach the cross memory pointers.                        */
	/************************************************************/
	rc = xmdetach(&dp);
	if (rc != XMEM_SUCC)
	{
		dma_diag->dma_ctls.dma_diag_error = XMDETACH_ERROR;
		return(rc);
	}
 
	/************************************************************/
	/* Call unpinu to unpin the user buffer                     */
	/************************************************************/
	rc = unpinu(dma_diag->dma_buff_ptr,
		    dma_diag->dma_buff_len,(short)UIO_USERSPACE);
	if (rc != SUCCESS)
	{
		dma_diag->dma_ctls.dma_diag_error = UNPINU_ERROR;
		return(rc);
	}
 
	return(0);
}
 
/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: uload                                               */
/*                                                                     */
/***********************************************************************/
uload(pd)
struct phys_displays *pd;
{
 
	uint  io_addr, rc, i;
	int parityrc;
	ulong *geo_addrs;
	struct gemini_data *ldtemp;
	struct gem_dds *gem_ddsp;
	label_t jmpbuf;
	uint retry = 0;
 
	/****************************************************************/
	/* Initialize local variables and get access to the adapter     */
	/****************************************************************/
	gem_ddsp = (struct gem_dds *) pd->odmdds;
	if (parityrc = (setjmpx(&jmpbuf)))
	{
	    if (parityrc == EXCEPT_IO)
	    {
		 gemlog(NULL,gem_ddsp->comp_name,"gem_ioctl","setjmpx",
					    parityrc,NULL,UNIQUE_2);
		 return(EIO);
	    }
	    else
		longjmpx(parityrc);
	}
	io_addr = BUSMEM_ATT(BUS_ID,0x00);
 
	/****************************************************************/
	/* Malloc a temporary local data area if non exists otherwise   */
	/* use existing one                                             */
	/****************************************************************/
	if (pd->interrupt_data.intr_args[2] == NULL)
	{
	      ldtemp = (struct gemini_data *)
		     xmalloc(sizeof(struct gemini_data),3,pinned_heap);
	     if (ldtemp == NULL)
	     {
		 gemlog(NULL,gem_ddsp->comp_name,"uload","xmalloc",
					     NULL,NULL,UNIQUE_1);
		 clrjmpx(&jmpbuf);
		 BUSMEM_DET(io_addr);
		 return(ENOMEM);
	     }
	     ldtemp->gcardslots = &(ldtemp->gm_crdslots);
	     ldtemp->ipl_shp_fp = &(ldtemp->iplshp_flgs);
	     ldtemp->component = (char *) gem_ddsp;
	}
	else
	   ldtemp = pd->interrupt_data.intr_args[2];
 
	/****************************************************************/
	/* Load Microcode                                               */
	/****************************************************************/
	while (retry < 3)
	{
		     rc = load_ucode(pd,ldtemp,(io_addr +
					    gem_ddsp->io_bus_mem_start));
		     if (rc != SUCCESS)
			 retry++;
		     else
			 break;
	}
	clrjmpx(&jmpbuf);
	BUSMEM_DET(io_addr);
 
#ifdef GEM_DBUG
	if (!(gem_ddsp->features & 0x02))
 
		printf("   TWO MEG MEMORY INSTALLED\n");
 
	else
		printf("   FOUR MEG MEMORY INSTALLED\n");
 
	if (!(gem_ddsp->features & 0x01))
		printf("   8 Bits/Pixel FBB INSTALLED\n");
	else
		printf("   24 Bits/Pixel FBB INSTALLED\n");
 
	if (gem_ddsp->features & NO_SHP)
		printf("   SHP NOT INSTALLED\n");
	else
		printf("   SHP INSTALLED\n");
#endif GEM_DBUG
 
	/****************************************************************/
	/* Save geographical addresses of cards on adapter              */
	/****************************************************************/
	geo_addrs = (ulong) pd->interrupt_data.intr_args[1];
	*geo_addrs++ = ldtemp->gcardslots->magic;
	*geo_addrs++ = ldtemp->gcardslots->drp;
	*geo_addrs++ = ldtemp->gcardslots->gcp;
	*geo_addrs++ = ldtemp->gcardslots->shp;
	*geo_addrs++ = ldtemp->gcardslots->imp;
	*geo_addrs++ = ldtemp->gcardslots->gcp_start;
 
	/****************************************************************/
	/* If ucode loaded successfully then initialize the adapter with*/
	/* defaults                                                     */
	/****************************************************************/
	if (rc == SUCCESS)
	{
	     ldtemp->ctbl.num_entries = 16;
	     for (i=0; i < 16; i++)
		 ldtemp->ctbl.colors[i] = gem_ddsp->color_table[i];
 
	     gem_init(ldtemp);
	     /************************************************************/
	     /* Initialize Device Dependent RCX                          */
	     /************************************************************/
	     if (pd->interrupt_data.intr_args[2] != NULL)
		     gem_init_rcx(pd, ldtemp);
	}
 
	if (pd->interrupt_data.intr_args[2] == NULL)
	    xmfree(ldtemp,pinned_heap);
	return(rc);
 
}
 
/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: get_vdp                                             */
/*                                                                     */
/***********************************************************************/
#define BIFLEN      512
#define BIF_PATTERN 0x00000080
#define NGSLOTS     11
#define CARD_MASK   0x00FF0000
#define VPD_DATA    0x0000A000
#define VPD_ERROR   0x80000000
#define RD_DRP_FBB  0x00000001
#define RD_SHP      0x00000003
#define RD_GCP      0x00070000
get_vpd(pd,vpd)
   struct phys_displays *pd;
   struct vpd_data *vpd;
{
 
	static slot;
	caddr_t  iocc_addr;
	int *m_stat_rp;                   /* MBC Master Status reg ptr    */
	int *m_ctrl_rp;                   /* MBC Master Contro reg ptr    */
	int *mem_s_adr_rp;                /* MBC Memory Start Addr reg ptr*/
	int *mem_e_adr_rp;                /* MBC Memory End Addr reg ptr  */
	int *loc_s_adr_rp;                /* MBC Local Start Addr reg ptr */
	int *loc_e_adr_rp;                /* MBC Local End Addr reg ptr   */
	int *glb_s_adr_rp;                /* MBC Global Start Addr reg ptr*/
	int *glb_e_adr_rp;                /* MBC Global end Addr reg ptr  */
	volatile uint *sc_r_p;            /* System Control Reg addr      */
	volatile uint *gadr_r_p;          /* Geographic Addr Reg ptr      */
	volatile uint *gc_r_p;            /* Gemini Control Reg ptr       */
	volatile char *pptr,*poscmd,*posdata;
	volatile int i;
	int drp_uc_aoff;                  /* DrP uc adapter offset in glb */
	int shp_uc_aoff;                  /* ShP uc adapter offset in glb */
	uint *uc_strtp;                   /* ptr to start of ucode in glb */
	int *clrfifo_p;                   /* ptr to Clear Fifo rtn        */
	int *memtst_p;                    /* ptr to Memory test rtn       */
	uint *bif_p;                      /* ptr to BIF                   */
	int *bstrp_p;                     /* ptr to Boot Strap rtn        */
	int *glb_44_adr;                  /* offset 0x44 in globa mem ptr */
	int *glb_48_adr;                  /* offset 0x4A in globa mem ptr */
	int *glb_4C_adr;                  /* offset 0x4C in globa mem ptr */
	int *glb_50_adr;                  /* offset 0x50 in globa mem ptr */
	volatile uint *temp, *intptr;
	uint *offsetptr;
	unsigned char *delay_ptr;               /* ptr to bud addr with
						  certain length of delay*/
	int volatile *runerrflgp;         /* BIF Run Flag ptr             */
	short volatile *runflg_p;         /* BIF Run Flag ptr             */
	struct gem_dds *gem_ddsp;
	volatile unsigned char val;
	uint start_addr;
	label_t jmpbuf;
	int parityrc;
	uint seg_reg;
 
	volatile uint status;
	ulong   id;
	ulong   card;
	struct gem_ddf    *ddf;
 
	/***************************************************************/
	/* Set the value to access the IOCC and get access to the bus. */
	/* Initialize a pointer to POS reg 0                           */
	/***************************************************************/
#ifdef VPD_TST
printf("get_vdp: entered   vpd addr = %08x\n",vpd);
#endif VPD_TST

	ddf = (struct gem_ddf *) pd->free_area;
	gem_ddsp = (struct gem_dds *) pd->odmdds;
	iocc_addr = IOCC_ATT(BUS_ID,0);
	slot = gem_ddsp->slot_number;
	pptr = iocc_addr + POSREG(0,slot) + IO_IOCC;
	poscmd = pptr + 6;
	posdata = pptr + 3;
 
	/***************************************************************/
	/* Read VPD data from Interface card into data structure       */
	/***************************************************************/
	for (i=1; i < 256; i++)
	{
		 *poscmd = i;
		 vpd->uch_vpd[i] = *posdata;
	}
	IOCC_DET(iocc_addr);
 
	/****************************************************************/
	/* Return here if we have failed to verify that we can access   */
	/* adapter                                                      */
	/****************************************************************/
	if (ddf->adapter_ready != 0x00)
	   return(ddf->adapter_ready);

	/****************************************************************/
	/* Get access to the adapter                                    */
	/****************************************************************/
	if (parityrc = (setjmpx(&jmpbuf)))
	{
	     if (parityrc == EXCEPT_IO)
	     {
		 gemlog(NULL,gem_ddsp->comp_name,"get_vpd","setjmpx",
					    parityrc,NULL,UNIQUE_1);
		 errno = EIO;
		 return(EIO);
	     }
	     else
		longjmpx(parityrc);
	}
	seg_reg = BUSMEM_ATT(BUS_ID,0x00);
 
	/****************************************************************/
	/* Initialize local variables                                   */
	/****************************************************************/
	start_addr = gem_ddsp->io_bus_mem_start + seg_reg;
	gc_r_p   = (uint *) (start_addr + GEM_CONTROL);
	sc_r_p   = (uint *) (start_addr + SYS_CONTROL);
	gadr_r_p = (uint *) (start_addr + GEO_ADDR);
	delay_ptr = (unsigned char *) ((start_addr & 0xf0000000) + DELAY_ADDR);
	uc_strtp = (uint *)(start_addr + GLOBAL_OFF);
	bif_p = (uint *)start_addr;
	m_stat_rp =    (int *)(start_addr + MASTER_STAT);
	m_ctrl_rp =    (int *)(start_addr + MASTER_CTRL);
	mem_s_adr_rp = (int *)(start_addr + MEMORY_START);
	mem_e_adr_rp = (int *)(start_addr + MEMORY_END);
	loc_s_adr_rp = (int *)(start_addr + LOCAL_START);
	loc_e_adr_rp = (int *)(start_addr + LOCAL_END);
	glb_s_adr_rp = (int *)(start_addr + GLOBAL_START);
	glb_e_adr_rp = (int *)(start_addr + GLOBAL_END);
	runerrflgp = (int *)(start_addr + BIF_RUNFLG_A);
	runflg_p = (short *)runerrflgp;
 
	/****************************************************************/
	/* +++++++++   RESET ADAPTER +++++++++++++++                    */
	/* Reset GEMINI and CVME                                        */
	/****************************************************************/
	*gc_r_p = RESET_GEMINI;
	*gc_r_p = CLEAR_REG;
	*gc_r_p = RESET_CVME;
 
	/****************************************************************/
	/* DO mode load to initialize Gemini memory controller.         */
	/****************************************************************/
	*gc_r_p  = SET_MODELOAD;
	for (i=0; i < DELAY_MODE_LOAD; i++)
	    val = *((uchar volatile *) delay_ptr);
	temp = (uint *)(start_addr + MLOAD_ADDR);
	*temp = 0x00000000;
	for (i=0; i < DELAY_MODE_LOAD; i++)
	    val = *((uchar volatile *) delay_ptr);
 
	*gc_r_p = CLEAR_REG;
 
	/****************************************************************/
	/* Collect VPD Data from Magic card                             */
	/****************************************************************/
	*gadr_r_p = (*gc_r_p) >> 28;
	offsetptr = (uint *)(start_addr + VPD_START);
	for (i=0; i < 256; i++)
	       vpd->mgc_vpd[i] = (unsigned char *)*offsetptr++;
 
	/****************************************************************/
	/* Initialize Master Bus Control(MBC) registers. Set the Master */
	/* Control Register, Local and Global address decode (enable)   */
	/****************************************************************/
	*gadr_r_p = (*gc_r_p) >> 28;
	*m_ctrl_rp    = 0xC0000000;
	*m_ctrl_rp    = 0xE0000000;
	*mem_s_adr_rp = CLEAR_REG;
	*mem_e_adr_rp = CLEAR_REG;
	*loc_s_adr_rp = CLEAR_REG;
	*loc_e_adr_rp = CLEAR_REG;
	*glb_s_adr_rp = GBL_LO_LIMIT;
	*glb_e_adr_rp = GBL_HI_LIMIT_2M;
 
	/****************************************************************/
	/* Get slot numbers for Gemini cards.                           */
	/****************************************************************/
	vpd->uch_slot = gem_ddsp->slot_number;
	vpd->mgc_slot = (*gc_r_p) >> 28;
	vpd->gcp_slot = -1;
	vpd->drp_slot = -1;
	vpd->fbb_slot = 0;
	vpd->shp_slot = -1;
	vpd->imp_slot = -1;
 
	for (slot=0; slot < NGSLOTS; slot++)
	{
	    *gadr_r_p = slot;
	    id = (*sc_r_p) & CARD_MASK;
 
	    *gadr_r_p = (*gc_r_p) >> 28;
	    status = (*m_stat_rp) & M_STAT_MASK;
	    if (status)
	    {
	       *m_stat_rp = 0x00000000;
	       id = 0;
	    }
 
	    card = slot;
	    switch ( id )
	    {
		case SHP_ID:
			vpd->shp_slot = card ;
			break;
 
		case GCP_ID:
			vpd->gcp_slot = card;
			break;
 
		case DRP_ID:
			vpd->drp_slot = card;
			break;
 
		case IMP_ID:
			vpd->imp_slot = card;
			break;
 
		default:
			break;
	    }
	}
	if (vpd->drp_slot != -1)
	      vpd->fbb_slot = vpd->drp_slot + 1;
 
#ifdef VPD_TST
	printf(" magic = %08X\n",vpd->mgc_slot);
	printf(" drp   = %08X\n",vpd->drp_slot);
	printf(" gcp   = %08X\n",vpd->gcp_slot);
	printf(" fbb   = %08X\n",vpd->fbb_slot);
	printf(" shp   = %08X\n",vpd->shp_slot);
	printf(" imp   = %08X\n",vpd->imp_slot);
#endif VPD_TST
 
	/****************************************************************/
	/* Initialize frequently used offsets in adapter global memory  */
	/****************************************************************/
	glb_44_adr = (int *)(start_addr + 0x44);
	glb_48_adr = (int *)(start_addr + 0x48);
	glb_4C_adr = (int *)(start_addr + 0x4C);
	glb_50_adr = (int *)(start_addr + 0x50);
 
	/***************************************************************/
	/* Load C25 VPD microcode into Adapter global memory.          */
	/***************************************************************/
	memcpy(uc_strtp, gem_ddsp->u6.c25vpd, gem_ddsp->c25vpdlen);
 
	/***************************************************************/
	/* Select the DRP and hold the C25 processor. Initialize ptrs  */
	/* to sizes of different routines in the C25 ucode load file   */
	/***************************************************************/
	*gadr_r_p = vpd->drp_slot;
	*sc_r_p  &= HOLD_ADAP;
 
	clrfifo_p = (int *) ((char *) uc_strtp + 0x0020);
	memtst_p  = (int *) ((char *) uc_strtp + *(uc_strtp));
	bstrp_p   = (int *) ((char *) uc_strtp + *(uc_strtp + 1));
	drp_uc_aoff = VME_BUS_MEM | *(uc_strtp+2) + GLOBAL_OFF;
	shp_uc_aoff = VME_BUS_MEM | *(uc_strtp+3) + GLOBAL_OFF;
 
	/***************************************************************/
	/* Load the Clear FIFO routine into the DRPs BIF               */
	/***************************************************************/
	memcpy(bif_p, clrfifo_p, BIFLEN);
 
	/****************************************************************/
	/* Set up the interface for Clear Fifo routine in DRPs BIF      */
	/****************************************************************/
	*(int *)(start_addr + 0x14) = 0x003C003D;
	*(int *)(start_addr + 0x18) = 0x16A45500;
	*glb_44_adr = 0x607FD001;
 
	/****************************************************************/
	/* Release the DRPs C25 processor and wait for the Clear FIFO   */
	/* routine to complete.                                         */
	/****************************************************************/
	*sc_r_p |= REL_ADAP;
	*sc_r_p |= RELS_ADAP;
	for (i=0; i < 100000; i++){
	    val = *((uchar volatile *) (delay_ptr));
	    if (!*runflg_p)
	      break;
	}
 
	/****************************************************************/
	/* Hold the DRPs C25 processor and load memory tests into the   */
	/* DRP's BIF                                                    */
	/****************************************************************/
	*sc_r_p &= HOLD_ADAP;
	memcpy(bif_p, memtst_p, BIFLEN);
 
	/****************************************************************/
	/* Set up the interface for Mem Test routine for exec in DRP BIF*/
	/****************************************************************/
	*glb_44_adr = 0x00000000;
	*glb_48_adr = 0x40004000;
	*glb_4C_adr = 0x00004000;
 
	/****************************************************************/
	/* Wait for Run flag to be turned OFF, indicating that the      */
	/* Memory test has completed.                                   */
	/****************************************************************/
	*sc_r_p |= REL_ADAP;
	*sc_r_p |= RELS_ADAP;
	for (i=0; i < 100000; i++){
	    val = *((uchar volatile *) (delay_ptr));
	    if (!*runflg_p)
	      break;
	}
 
	/****************************************************************/
	/* Hold the DRPs C25 processor and load the Bootsrtap Loader    */
	/* into DRP's BIF                                               */
	/****************************************************************/
	*sc_r_p &= HOLD_ADAP;
	memcpy(bif_p, bstrp_p, BIFLEN);
 
	/****************************************************************/
	/* Adapter Global mem start address of DRP microcode            */
	/****************************************************************/
	*glb_44_adr = drp_uc_aoff;
	*glb_48_adr = 0x40004000;
	*glb_4C_adr = 0x00004000;
	*glb_50_adr = 0x00420002;
 
	/****************************************************************/
	/* Release the DRPs C25 processor and wait for the run flag in  */
	/* the DRPs BIF to be reset by the DRP microcode when it has    */
	/* completed initialization.                                    */
	/****************************************************************/
	*sc_r_p |= REL_ADAP;
	*sc_r_p |= RELS_ADAP;
	for (i=0; i < 500000; i++){
	    val = *((uchar volatile *) (delay_ptr));
	    if (!*runflg_p)
	      break;
	}
 
	/****************************************************************/
	/* If an error was detected, then clear it.                     */
	/****************************************************************/
	status = (*m_stat_rp) & M_STAT_MASK;
	if (status)
	    *m_stat_rp = 0x00000000;
 
	/****************************************************************/
	/* Clear BIF communications to zero                             */
	/****************************************************************/
	intptr = start_addr;
	for (i=0; i < 128; i++)
	     *intptr++ = 0x00;
 
	/****************************************************************/
	/* Set Read VPD request and global address in BIF comm area     */
	/****************************************************************/
	*glb_44_adr = (uint) RD_DRP_FBB;
	*glb_48_adr = (uint) VME_BUS_MEM | VPD_DATA;
	*runerrflgp = (uint) RUN_FLAG;
	for (i=0; i < 500000; i++){
	    val = *((uchar volatile *) (delay_ptr));
	    if (!*runflg_p)
	      break;
	}
 
	/****************************************************************/
	/* If error then set error flag in slot number for DRP/FBB else */
	/* copy VPD data from global memory to vpd data structure       */
	/****************************************************************/
	if (*runerrflgp)
	{
	   vpd->drp_slot |= VPD_ERROR;
	   vpd->fbb_slot |= VPD_ERROR;
	}
	else
	{
	     /***********************************************************/
	     /* Copy DRP vpd to data structure                          */
	     /***********************************************************/
	     temp = (uint *) ((char *) start_addr + VPD_DATA);
	     intptr = (uint *) (vpd->drp_vpd);
	     pptr = ((char *) intptr) + 0x1c;
	     for (i=0; i < 8; i++)
		 *intptr++ = (uint) *temp++;

	     /***********************************************************/
	     /* Determine if the DrP supports HW Anti-Alias.            */
	     /***********************************************************/
	     val = *pptr;
	     if (val >= ANTI_ALIAS)
		 gem_ddsp->features |= ANTI_ALIAS_INSTALLED;

	     /***********************************************************/
	     /* Copy FBB vpd to data structure                          */
	     /***********************************************************/
	     intptr = (uint *) (vpd->fbb_vpd);
	     pptr = ((char *) intptr) + 0x1c;
	     for (i=0; i < 8; i++)
		 *intptr++ = (uint) *temp++;

	     /***********************************************************/
	     /* Determine if FBB support 77 Hz (ISO)                    */
	     /***********************************************************/
             val = *pptr;    /* check the minimum device driver  NGK 00 */
             if (val >= ISO) /* level to be >= 1 ...             NGK 00 */
                 gem_ddsp->features |= ISO_SUPPORTED;
             intptr = (uint *) (vpd->fbb_vpd);                /* NGK 00 */
             pptr = ((char *) intptr) + 0x01;                 /* NGK 00 */
             val = *pptr;    /* also check the correlator field  NGK 00 */
             if (val > ISO)  /* to be > 1 (see defect 97669)     NGK 00 */
                 gem_ddsp->features |= ISO_SUPPORTED;         /* NGK 00 */
          /* printf("features = 0x%02x\n",gem_ddsp->features);** NGK 00 */
	}
 
	/****************************************************************/
	/* Stop the DRP C25 processor                                   */
	/****************************************************************/
	*sc_r_p  &= HOLD_ADAP;
 
	/****************************************************************/
	/* If shading processor exists get its VPD data                 */
	/****************************************************************/
	if (vpd->shp_slot != -1)
	{
		/********************************************************/
		/* Select the SHP and hold the C25 processor            */
		/********************************************************/
		*gadr_r_p = vpd->shp_slot;
		*sc_r_p  &= HOLD_ADAP;
 
		/********************************************************/
		/* Load the Clear FIFO  into the SHP BIF, starting from */
		/* begining of the BIF.                                 */
		/********************************************************/
		bif_p = (uint *) start_addr;
		memcpy(bif_p, clrfifo_p, BIFLEN);
 
		/********************************************************/
		/* Set up the interface for Clear Fifo rtn for exec in  */
		/* SHP BIF                                              */
		/********************************************************/
		*(int *) (start_addr + 0x18) = 0x06A45500;
		*(int *) (start_addr + 0x44) = 0x00000000;
 
		/********************************************************/
		/* Release the SHPs C25 processor and wait for the      */
		/* Clear FIFO routine to complete.                      */
		/********************************************************/
		*sc_r_p |= REL_ADAP;
		*sc_r_p |= RELS_ADAP;
		for (i=0; i < 100000; i++){
		       val = *((uchar volatile *) (delay_ptr));
		       if (!*runflg_p)
		       break;
		    }
 
		/********************************************************/
		/* Load Memory tests code into SHP's BIF from its start */
		/********************************************************/
		*sc_r_p &= HOLD_ADAP;
		memcpy(bif_p, memtst_p,BIFLEN);
 
		/*******************************************************/
		/* Set up the interface for Mem Test routine for exec  */
		/* in SHP BIF                                          */
		/*******************************************************/
		*(int *)(start_addr + 0x18) = 0x16A45500;
		*glb_44_adr = 0x00000000;
		*glb_48_adr = 0x40008000;
		*glb_4C_adr = 0x00008000;
 
		/********************************************************/
		/* Release the SHPs C25 processor and wait for run flag */
		/* to be cleared indicating memory test complete.       */
		/********************************************************/
		*sc_r_p |= REL_ADAP;
		*sc_r_p |= RELS_ADAP;
		for (i=0; i < 100000; i++){
		       val = *((uchar volatile *) (delay_ptr));
		       if (!*runflg_p)
		       break;
		    }
 
		/********************************************************/
		/* Hold the SHPs C25 processor and load the Bootstrap   */
		/* Loader into SHP's BIF                                */
		/********************************************************/
		*sc_r_p &= HOLD_ADAP;
		memcpy(bif_p, bstrp_p, BIFLEN);
 
		/********************************************************/
		/* Set up the interface for Ucode Loader rtn for exec   */
		/* in SHP BIF                                           */
		/********************************************************/
		*glb_44_adr = shp_uc_aoff;
		*glb_48_adr = 0x40008000;
		*glb_4C_adr = 0x00008000;
		*glb_50_adr = 0x00410001;
 
		/********************************************************/
		/* Release the SHPs C25 processor and wait for the      */
		/* run flag to be cleared by the SHP microcode          */
		/* indicating that it has completed initialization.     */
		/********************************************************/
		*sc_r_p |= REL_ADAP;
		*sc_r_p |= RELS_ADAP;
		for (i=0; i < 500000; i++){
		      val = *((uchar volatile *) (delay_ptr));
		      if (!*runflg_p)
			 break;
		}
 
		/********************************************************/
		/* If an was detected, then clear it.                   */
		/********************************************************/
		status = (*m_stat_rp) & M_STAT_MASK;
		if (status)
		    *m_stat_rp = 0x00000000;
 
		/********************************************************/
		/* Clear BIF communications                             */
		/********************************************************/
		intptr = start_addr;
		for (i=0; i < 128; i++)
		    *intptr++ = 0x00;
 
		/********************************************************/
		/* Set Read SHP VPD request and global memory address   */
		/* into BIF SHP VPD request and global memory address   */
		/********************************************************/
		*glb_44_adr = (uint) RD_SHP;
		*glb_48_adr = (uint) VME_BUS_MEM | VPD_DATA;
		*runerrflgp = (uint) RUN_FLAG;
		for (i=0; i < 500000; i++){
		      val = *((uchar volatile *) (delay_ptr));
		      if (!*runflg_p)
			  break;
		}
 
		/*******************************************************/
		/* If error then set error flag in slot number for SHP */
		/* else copy VPD data from global memory to vpd struct */
		/*******************************************************/
		if (*runerrflgp)
		     vpd->shp_slot |= VPD_ERROR;
		else
		{
		     temp = (uint *) ((char *) start_addr + VPD_DATA);
		     intptr = (uint *) (vpd->shp_vpd);
		      for (i=0; i < 32; i++)
			   *intptr++ = (uint) *temp++;
		}
 
		/*******************************************************/
		/* Stop the SHP C25 processor                          */
		/*******************************************************/
		*sc_r_p  &= HOLD_ADAP;
	}
 
	/****************************************************************/
	/* Select and Hold the GCP processor                            */
	/****************************************************************/
	*gadr_r_p = vpd->gcp_slot;
	*sc_r_p   = CLEAR_REG;
 
	/****************************************************************/
	/* Setup to run BIF tests and basic diagnostics. We have to     */
	/* initialize BIF memory first.                                 */
	/****************************************************************/
	bif_p = (uint *)start_addr;
	for (i=0; i < 128; i++)
	    *bif_p++ = (BIF_PATTERN - i) << 16;
 
	/****************************************************************/
	/* Release the GCP and wait for run flag to be reset            */
	/****************************************************************/
	*sc_r_p = REL_ADAP;
	*sc_r_p = RELS_ADAP;
	for (i=0; i < DELAY_TIME; i++){
	    val = *((uchar volatile *) (delay_ptr));
	    if (!*runflg_p)
	      break;
	}
 
	/****************************************************************/
	/* Initialize pointer to global memory for VPD and set command  */
	/* in BIF to get GCP VPD data.                                  */
	/****************************************************************/
	bif_p = (uint *)((char *) start_addr + 0x1f8);
	*bif_p++ = (uint) VME_BUS_MEM | VPD_DATA;
	*bif_p   = RD_GCP;
 
	for (i=0; i < DELAY_TIME; i++){
	    val = *((uchar volatile *) (delay_ptr));
	    if (!*runflg_p)
	      break;
	}
 
	/****************************************************************/
	/* If no errors then collect GCP VPD data.                      */
	/****************************************************************/
	if (*runerrflgp)
	     vpd->gcp_slot |= VPD_ERROR;
	else
	{
	      temp = (uint *) ((char *) start_addr + VPD_DATA);
	      intptr = (uint *) (vpd->gcp_vpd);
	      for (i=0; i < 32; i++)
		     *intptr++ = (uint) *temp++;
	}
 
	*sc_r_p   = CLEAR_REG;

	clrjmpx(&jmpbuf);
	BUSMEM_DET(seg_reg);
	return(SUCCESS);
}
 
 
/****************************************************************************
 
	Function Name: gem_dev_init
 
	Descriptive Name: Device dependent initialization
 
 * ------------------------------------------------------------------------*
 
	Function:
 
	This routine does the set up of the TCW's via the d_protect call.
	
	The bus authorization mask in the display structure is set up in
	the device independent dev_init function.
 
 
***************************************************************************/
 
gem_dev_init(pdev)
gscDevPtr	pdev;
 
{
	struct phys_displays *pd = pdev->devHead.display;
	short	i;
		
	/************************************************************/
	/* Initialize authorization mask for each domain and protect*/
	/* each domain on the adapter                               */
	/************************************************************/
	for (i=0; i < GM_MAX_DOMAINS; i++)
	  {
	    d_protect(NULL, pd->busmemr[i].bus_mem_start_ram,
		      ((pd->busmemr[i].bus_mem_end_ram + 1) -
		       pd->busmemr[i].bus_mem_start_ram),
		       pd->busmemr[i].auth_mask, BUS_ID);
	  }
 
        return 0;
 
}
 
/****************************************************************************
 
	Function Name: gem_dev_term
 
	Descriptive Name: Device dependent cleanup
 
 * ------------------------------------------------------------------------*
 
	Function:
 
	This function is currently a NULL function.
 
***************************************************************************/
 
gem_dev_term(pdev)
gscDevPtr	pdev;
{
}

/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: chk_adap                                            */
/*                                                                     */
/***********************************************************************/
chk_adap(pd)
   struct phys_displays *pd;
{

	volatile uint *mem_ptr;
	volatile uint *gc_r_p;            /* Gemini Control Reg ptr       */
	volatile char *pptr;
	static slot;
	caddr_t  io_addr;
	volatile uint *temp;
	struct gem_dds *gem_ddsp;
	uint start_addr;
	uint seg_reg, i;
	ulong   data;
	label_t jmpbuf;
	int parityrc;
	unsigned char *delay_ptr;
	volatile unsigned char val;

	/****************************************************************/
	/* Get access to the adapter                                    */
	/****************************************************************/
	if (parityrc = (setjmpx(&jmpbuf)))
	{
	     if (parityrc == EXCEPT_IO)
	     {
		 errno = ENOTREADY;
		 return(ENOTREADY);
	     }
	     else
		longjmpx(parityrc);
	}
	seg_reg = BUSMEM_ATT(BUS_ID,0x00);

	/****************************************************************/
	/* Initialize local variables                                   */
	/****************************************************************/
	gem_ddsp = (struct gem_dds *) pd->odmdds;
	start_addr = gem_ddsp->io_bus_mem_start + seg_reg;
	gc_r_p   = (uint *) (start_addr + GEM_CONTROL);
	mem_ptr = (uint *) (start_addr + GLOBAL_OFF);
	delay_ptr = (unsigned char *) ((start_addr & 0xf0000000) + DELAY_ADDR);

	/****************************************************************/
	/* +++++++++   RESET ADAPTER +++++++++++++++                    */
	/* Reset GEMINI and CVME                                        */
	/****************************************************************/
	*gc_r_p = RESET_GEMINI;
	*gc_r_p = CLEAR_REG;
	*gc_r_p = RESET_CVME;

	/****************************************************************/
	/* DO mode load to initialize Gemini memory controller.         */
	/****************************************************************/
	*gc_r_p  = SET_MODELOAD;
	for (i=0; i < DELAY_MODE_LOAD; i++)
	    val = *((uchar volatile *) delay_ptr);
	temp = (uint *)(start_addr + MLOAD_ADDR);
	*temp = 0x00000000;
	for (i=0; i < DELAY_MODE_LOAD; i++)
	    val = *((uchar volatile *) delay_ptr);
	*gc_r_p = CLEAR_REG;

	/****************************************************************/
	/* Write/read adapter memory to determine if the adapter is     */
	/* there...                                                     */
	/****************************************************************/
	for (i=0; i < 5; i++)
	{
	   *mem_ptr = 0x00;
	   *mem_ptr = 0xFEFEFEFE;
	   data = *mem_ptr;
	   if (data == 0xFEFEFEFE)
	     break;
	}
	clrjmpx(&jmpbuf);
	BUSMEM_DET(seg_reg);

	if (i == 5)
	{
	      io_addr = IOCC_ATT(BUS_ID,0);
	      /*******************************************************/
	      /* Select POS Reg 0 and disable adapter                */
	      /*******************************************************/
	      slot = gem_ddsp->slot_number;
	      pptr  = io_addr + POSREG(2,slot) + IO_IOCC;
	      *pptr = PosDisable;     /* Disable the adapter (POS0) */
	      IOCC_DET(io_addr);
	      return(ENOTREADY);
	}
      return(SUCCESS);
}

/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: set_refresh                                         */
/*                                                                     */
/***********************************************************************/
set_refresh(struct phys_displays *pd, uint rate)
{

	volatile uint *sc_r_p;            /* System Control Reg addr      */
	volatile uint *gadr_r_p;          /* Geographic Addr Reg ptr      */
	uint saved_geoadr;
	uint *bifp;                       /* ptr to BIF locations         */
	struct drp_comm *drp_com_p;
	struct gem_dds *gem_ddsp;

	uint start_addr;
	label_t jmpbuf;
	int parityrc;
	uint seg_reg;
/*
 if (rate == 2)
   printf(" set refresh to 60 Hz rate = %x\n",rate);
 else
   printf(" set refresh to 77 Hz rate = %x\n",rate);
  */
	/****************************************************************/
	/* Get access to the adapter                                    */
	/****************************************************************/
	gem_ddsp = (struct gem_dds *) pd->odmdds;
	if (parityrc = (setjmpx(&jmpbuf)))
	{
	     if (parityrc == EXCEPT_IO)
	     {
		 gemlog(NULL,gem_ddsp->comp_name,"set_refresh","setjmpx",
					    parityrc,NULL,UNIQUE_1);
		 errno = EIO;
		 return(EIO);
	     }
	     else
		longjmpx(parityrc);
	}
	seg_reg = BUSMEM_ATT(BUS_ID,0x00);

	/****************************************************************/
	/* Initialize local variables                                   */
	/****************************************************************/
	start_addr = gem_ddsp->io_bus_mem_start + seg_reg;
	sc_r_p     = (uint *) (start_addr + SYS_CONTROL);
	gadr_r_p   = (uint *) (start_addr + GEO_ADDR);
	drp_com_p  = (struct drp_comm *) start_addr;

	/***************************************************************/
	/* Select the DRP and clear the host to DrP Interrupt Request  */
	/* block.                                                      */
	/***************************************************************/
	saved_geoadr = *gadr_r_p;
	*gadr_r_p = 0x09;

	/****************************************************************/
	/* Set request code into DrP Interrupt Request Block using the  */
	/* value passed into this routine.                              */
	/****************************************************************/
	bifp = (uint *)drp_com_p->r_d_intblk;
	*bifp++ = rate;
	*bifp = 0x00;

	/****************************************************************/
	/* Interrupt the DrP and restore geographical address           */
	/****************************************************************/
	*sc_r_p = (0x04000000 | RELS_ADAP);
	*gadr_r_p = saved_geoadr;

	clrjmpx(&jmpbuf);
	BUSMEM_DET(seg_reg);
	return(SUCCESS);
}

