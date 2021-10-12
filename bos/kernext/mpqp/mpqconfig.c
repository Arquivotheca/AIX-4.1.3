static char sccsid[] = "@(#)25	1.24.3.6  src/bos/kernext/mpqp/mpqconfig.c, sysxmpqp, bos411, 9434B411a 8/23/94 09:35:24";
/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqconfig
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <errno.h>
#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/file.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/timer.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

/*******************************************************************
 *    External Function Declarations                               *
 *******************************************************************/

extern int mpqintr();		/* SLIH function */
extern int mpqoffl();           /* Offlevel */
extern int nodev();
extern int mpqopen();
extern int mpqclose();
extern int mpqread();
extern int mpqwrite();
extern int mpqioctl();
extern int mpqmpx();
extern int mpqselect();
 
extern unsigned int	reset_card( t_acb *,
				    unsigned long,
				    unsigned long,
				    unsigned long);

/*******************************************************************
 *    Internal Function Declarations                               *
 *******************************************************************/

int 		mpqconfig(dev_t,
			  int,
			  struct uio	*);
#ifdef _POWER_MPQP
void		init_locks();
void		free_locks();
#endif /* _POWER_MPQP */

/*******************************************************************
 *      Declarations for this translation unit			   *
 *******************************************************************/

static struct devsw mpqp_dsw;	/* the devsw entry for devswadd */

static int      num_acbs = 0;   /* number of active adapters */

/*******************************************************************
 *      External Declarations 					   *
 *******************************************************************/

extern t_acb		*acb_dir[];	/* ACB directory */

extern t_mpqp_dds	*dds_dir[];	/* DDS directory */

#ifdef _POWER_MPQP
/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ global adapter configuration lock word                            ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
static lock_t          mpqp_cfg_lock = LOCK_AVAIL;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ global adapter device driver mp lock word                         ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
Simple_lock  mpqp_mp_lock;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ MP lock word for IOCTLs                                           ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
Simple_lock  mpqp_ioctl_lock;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ MP lock word for interrupt handler                                ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
Simple_lock mpqp_intr_lock;

/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ MP lock word for internal tracing                                 ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
Simple_lock mpqp_trace_lock;
#endif /* _POWER_MPQP */

#define TRACE_TABLE_SIZE   512           /* size (int) of mpqTraceTable */ 
int  MPQTraceTable[TRACE_TABLE_SIZE];    /* static trace data           */
int  MPQnexthole;

/*
 * NAME: mpqconfig
 *
 * FUNCTION: mpqconfig performs operations necessary to the initialization of
 *	     an individual port on an adapter.  mpqconfig will be called for
 *	     each valid port during the bus configuration/device configuration
 *           phase of the boot procedure.  mpqconfig must set up various data
 *           data structures, validate requests and so forth.
 *
 * EXECUTION ENVIRONMENT: 
 *
 *	Preemptable	   : Yes
 *	VMM Critical Region: No
 *	Runs on Fixed Stack: Yes
 *	May Page Fault	   : Yes
 *	May Backtrack	   : Yes
 *
 * NOTES: 
 *
 * DATA STRUCTURES:  
 *
 * RETURN VALUE DESCRIPTION: 0 - EVT mpqconfig return was successful
 */

int mpqconfig ( dev_t		devno,
		int		cmd,
		struct uio	*uiop )

{
    /* mpqconfig local variables */

    int			adap_num;	/* adapter number */
    unsigned long	bus_sr;		/* IO Seg Reg number mask */
    unsigned long	iob;		/* io base address */
    unsigned long	memb;		/* bus memory base */
    int			minor_num;	/* minor device number */
    int			port_num;	/* port number on adapter */
    int			rc;             /* general return code */
    t_acb               *p_acb;		/* pointer to the ACB */
    t_mpqp_dds          *p_dds;		/* pointer to the DDS */
    struct intr         *p_intr;        /* work ptr           */


    /* log a trace hook */
    DDHKWD5 (HKWD_DD_MPQPDD, DD_ENTRY_CONFIG, 0, devno, cmd, 0, 0, 0);

#ifdef _POWER_MPQP
    if (lockl(&mpqp_cfg_lock, LOCK_SIGRET) == LOCK_SIG)
    {
	    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, EINTR, devno, 0,0,0,0);
            return (EINTR);
    }
#endif /* _POWER_MPQP */


    /* get the minor number */
    minor_num = minor(devno);	/* minor num comes from macro */

    /* if minor number is invalid, return error */
    if (minor_num >= (MAX_ADAPTERS*NUM_PORTS))
    {
	    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, EINVAL, devno, 0,0,0,0);
#ifdef _POWER_MPQP
	    unlockl(&mpqp_cfg_lock);
#endif /* _POWER_MPQP */
	    return(EINVAL);
    }

    /* get dds pointer */
    p_dds = dds_dir[minor_num];

    switch (cmd)		/* switch based on command type */
    {

	case CFG_INIT:		/* initialize device driver and internal */
				/* data areas				 */
	{
	    if /* dds already exists, return error */
	       ( p_dds != (t_mpqp_dds *)NULL )
	    {
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, EINVAL, devno, 0,0,0,0);

#ifdef _POWER_MPQP
    		unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		return(EINVAL);
	    }

	    /* first time through CFG_INIT requires special steps */
	    /* no active adapters means this is first time */
	    if ( num_acbs == 0 )
	    {
		/* pin the mpqp device driver in memory */
		if /* attempt to pin the code fails */
		   ((rc = pincode(mpqconfig)) != 0)
		{
		    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, rc, devno, 0,0,0,0);

#ifdef _POWER_MPQP
    		    unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		    return(rc);
		}

		/* now that driver is pinned, can start in-core traces */
		/* init in-core trace table */
		MPQnexthole = 0;
		MPQTraceTable [MPQnexthole] = 0x2A2A2A2A;
#ifdef _POWER_MPQP
                /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
                ³ Initialize MP locks on first adapter to be configured      ³
                ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		init_locks();
#endif /* _POWER_MPQP */

		MPQTRACE2("Cin1", devno);

		/* add our entry points to the devsw table */

		mpqp_dsw.d_open     = mpqopen;
		mpqp_dsw.d_close    = mpqclose;
		mpqp_dsw.d_read     = mpqread;
		mpqp_dsw.d_write    = mpqwrite;
		mpqp_dsw.d_ioctl    = mpqioctl;
		mpqp_dsw.d_strategy = nodev;
		mpqp_dsw.d_ttys     = NULL;
		mpqp_dsw.d_select   = mpqselect;
		mpqp_dsw.d_config   = mpqconfig;
		mpqp_dsw.d_print    = nodev;
		mpqp_dsw.d_dump     = nodev;
		mpqp_dsw.d_mpx      = mpqmpx;
		mpqp_dsw.d_revoke   = nodev;
		mpqp_dsw.d_dsdptr   = NULL;
		mpqp_dsw.d_selptr   = NULL;
#ifdef _POWER_MPQP
		mpqp_dsw.d_opts     = DEV_MPSAFE;
#else
		mpqp_dsw.d_opts     = 0;
#endif /* _POWER_MPQP */

		if /* attempt to add entry points to devsw fails */
		   ((rc = devswadd(devno, &mpqp_dsw)) != 0)
		{
		    MPQTRACE2("Cie1", rc);
		    unpincode (mpqconfig);
		    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, rc, devno, 0,0,0,0);

#ifdef _POWER_MPQP
		    free_locks();
    		    unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		    return(rc);
		}
		timeoutcf(64);

		MPQTRACE2("Cin2", devno);

	    } /* end of first time...pin code and load devsw entry */

	    MPQTRACE4("Cin3", devno, p_dds, uiop);

	    /* allocate a bit of memory for the dds */
	    p_dds = (t_mpqp_dds *)xmalloc ( sizeof(t_mpqp_dds), 2, pinned_heap);

	    if /* xmalloc failed...return error */
	       ( p_dds == (t_mpqp_dds *)NULL )
	    {
		MPQTRACE1("Cie2");
		free_resources(num_acbs, devno, NULL, NULL);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, ENOMEM, devno, 0,0,0,0);

#ifdef _POWER_MPQP
    		unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		return(ENOMEM);
	    }

	    /* zero out the dds */
	    bzero((char *)p_dds, sizeof(t_mpqp_dds));

	    /* copy input structure into dds */
	    rc = uiomove(p_dds, sizeof(t_mpqp_dds), UIO_WRITE, uiop);

	    MPQTRACE4("Cin4", sizeof(t_mpqp_dds), uiop->uio_resid, rc);

	    if /* uiomove returns bad status */
	       ( rc )
	    {
		MPQTRACE2("Cie3", rc);
		free_resources(num_acbs, devno, p_dds, NULL);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, rc, devno, 0,0,0,0);

#ifdef _POWER_MPQP
    		unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		return(rc);
	    }

	    /* set port number from ddi information */
	    port_num = p_dds->dds_dvc.port_num;

	    /* adapter number is slot number */
	    adap_num = p_dds->dds_hdw.slot_num;

	    MPQTRACE3("Cin5", port_num, adap_num);

	    /* adapter num must be in range of 0 to 15, inclusive */
	    /* and port num must be in range of 0 to 3, inclusive */
	    if ((adap_num > ((MAX_ADAPTERS * MAX_BUSES) - 1)) || (port_num > (NUM_PORTS -1))) 
	    {
		MPQTRACE1("Cie4");
		free_resources(num_acbs, devno, p_dds, NULL);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, EINVAL, devno, 0,0,0,0);

#ifdef _POWER_MPQP
    		unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		return(EINVAL);
	    }

	    p_acb = acb_dir[adap_num];

	    if /* no Adapter Control Block for this device */
	       ( p_acb == (t_acb *)NULL)
	    {
		MPQTRACE3("Cin6", adap_num, p_acb);

		/* allocate the memory for the acb */
		p_acb = (t_acb *)xmalloc(sizeof(t_acb), 2, pinned_heap);

		if /* allocation failed... */
		   ( p_acb == (t_acb *)NULL)
		{
		    MPQTRACE1("Cie5");
		    free_resources(num_acbs, devno, p_dds, NULL);
		    DDHKWD5(HKWD_DD_MPQPDD,DD_EXIT_CONFIG,ENOMEM,devno,0,0,0,0);

#ifdef _POWER_MPQP
    		    unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		    return(ENOMEM);
		}

		/* zero out the adapter control block */
		bzero((char *)p_acb, sizeof(t_acb));

		p_acb->p_port_dds[port_num] = p_dds;

		/* now we set up for the POS register settings */
		p_acb->int_lvl = p_dds->dds_hdw.bus_intr_lvl;
		p_acb->int_pri = p_dds->dds_hdw.intr_priority;
		p_acb->slot_num = (unsigned char)(p_dds->dds_hdw.slot_num);
		p_acb->arb_lvl = p_dds->dds_hdw.dma_lvl;
		p_acb->io_base = p_dds->dds_hdw.bus_io_addr;
		p_acb->mem_base = p_dds->dds_hdw.bus_mem_addr;
		p_acb->dma_base = p_dds->dds_hdw.tcw_bus_mem_addr;
                                                    
                /* setup segment register value based on bus id */
                p_acb->io_segreg_val = p_dds->dds_hdw.bus_id | IO_SEG_REG; 
                p_acb->iocc_segreg_val = p_dds->dds_hdw.bus_id;

		p_acb->adapter_state = 0;
		p_acb->cpu_page = 0xFF;

		/* allocate the local transmit free buffer queue */
		p_acb->p_lcl_txfree_buf_q =
			xmalloc((4 + N_TXFREE), 2, pinned_heap);

		/* if allocation failed, free dds and acb */
		if (p_acb->p_lcl_txfree_buf_q == NULL)
		{
		    MPQTRACE1("Cie6");
		    free_resources(num_acbs, devno, p_dds, p_acb);
		    DDHKWD5(HKWD_DD_MPQPDD,DD_EXIT_CONFIG,ENOMEM,devno,0,0,0,0);

#ifdef _POWER_MPQP
    		    unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		    return(ENOMEM);
		}
		bzero(( char *)p_acb->p_lcl_txfree_buf_q, (4 + N_TXFREE));

		/* invoke set_pos to set POS registers */
		set_pos( p_acb );

		/* set up segment register for next phase */
		bus_sr = BUSIO_ATT(p_acb->io_segreg_val, 0);

		/* set up the busio and bus memory base address for the card */
		iob = p_acb->io_base + bus_sr;
		memb = p_acb->mem_base + bus_sr;
		rc = reset_card ( p_acb, bus_sr, iob, memb);

		/* free up segment register */
		BUSIO_DET(bus_sr);

		if /* reset failed... */
		   ( rc )
		{
		    MPQTRACE2("Cie7", rc);
		    free_resources(num_acbs, devno, p_dds, p_acb);
		    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, EIO, devno,0,0,0,0);

#ifdef _POWER_MPQP
    		    unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		    return(EIO);
		}

		p_acb->c_intr_rcvd = 0;     /* zero interrupt count  */

		/* now we set up our DMA channel by calling d_init */
		p_acb->dma_channel_id =
			d_init((int)p_acb->arb_lvl, MICRO_CHANNEL_DMA,
				p_acb->io_segreg_val);

		/* free up resources if d_init failed */
		if (p_acb->dma_channel_id == DMA_FAIL)
		{
		    MPQTRACE1("Cie8");
		    free_resources(num_acbs, devno, p_dds, p_acb);
		    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, EIO, devno,0,0,0,0);

#ifdef _POWER_MPQP
    		    unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		    return(EIO);
		}

		/* enable DMA channel */
		d_unmask( p_acb->dma_channel_id);

	        /* here we will register the SLIH and fill */
		/* in off level interrupt structures.      */

                MPQTRACE2("Cin7", p_acb);
	
                /* first we initialize the offlevel intr structures */
                p_acb->arq_sched = FALSE;
                p_acb->offl.p_acb_intr = (struct t_acb *)p_acb;
                p_intr = &(p_acb->offl.offl_intr);
                INIT_OFFL3(p_intr, mpqoffl, p_acb->io_segreg_val);
#ifdef _POWER_MPQP
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Initialize OFFLEVEL handler as being "MP-Safe" ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
		p_intr->flags |= INTR_MPSAFE;
#endif /* _POWER_MPQP */

                p_acb->slih_intr.next = NULL;
                p_acb->slih_intr.handler = mpqintr;
                p_acb->slih_intr.bus_type = BUS_MICRO_CHANNEL;
#ifdef _POWER_MPQP
		/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
		³ Initialize SLIH as being "MP-Safe"             ³
		ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
                p_acb->slih_intr.flags |= INTR_MPSAFE;
#else
                p_acb->slih_intr.flags = 0;
#endif /* _POWER_MPQP */
                p_acb->slih_intr.level = p_acb->int_lvl;
                p_acb->slih_intr.priority = p_acb->int_pri;
                p_acb->slih_intr.bid = p_acb->io_segreg_val;

                if /* registration of interrupt handler fails */
                ((rc = i_init(&p_acb->slih_intr)) != 0)
                {
                        MPQTRACE4("Cie9", devno, p_acb, rc);

			/* clear the DMA channel */
			d_mask(p_acb->dma_channel_id);
                    	d_clear(p_acb->dma_channel_id);

			/* free up resources */
			free_resources(num_acbs, devno, p_dds, p_acb);
                        DDHKWD5( HKWD_DD_MPQPDD, DD_EXIT_CONFIG, ENXIO,
                                devno, 0, 0,0,0);

#ifdef _POWER_MPQP
    		        unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

                        return(ENXIO);
                }


		num_acbs++;                     /* adapter is now active */
		acb_dir[adap_num] = p_acb;
		MPQTRACE3("Cin8",p_acb->dma_channel_id,p_acb->arb_lvl);

	    } /* end of no existing acb if */

	    p_acb->n_cfg_ports++;
	    p_acb->p_port_dds[port_num] = p_dds;
	    dds_dir[minor_num] = p_dds;
	    MPQTRACE1("Cin8");
	    break;

	} /* end of case CFG_INIT */


	case CFG_TERM:		/* terminate the device associated with */
				/* devno parameter			*/
	{
	    MPQTRACE3("Ctm1", devno, p_dds);

	    if (p_dds == NULL)
	    {
#ifdef _POWER_MPQP
    		    unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		    return(EACCES);
	    }

	    MPQTRACE2("Ctm2", p_dds->dds_dvc.port_state);
	    if (p_dds->dds_dvc.port_state != CLOSED)
	    {
#ifdef _POWER_MPQP
    		    unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

		    return(EBUSY);
	    }
	    port_num = p_dds->dds_dvc.port_num;
	    adap_num = p_dds->dds_hdw.slot_num;
	    p_acb = acb_dir[p_dds->dds_hdw.slot_num];

	    MPQTRACE4("Ctm3", p_dds, port_num, adap_num);
	    MPQTRACE4("Ctm4", p_acb, p_acb->n_cfg_ports, num_acbs);

	    /* decrement number of configured ports on this adapter */
	    p_acb->n_cfg_ports--;

	    /* if last configured port on adapter, free adapter resources */
	    if (p_acb->n_cfg_ports == 0)
	    {
		    /* Release the dma_channel */
		    MPQTRACE3("Ctm5", p_acb->dma_channel_id,
			    p_acb->p_lcl_txfree_buf_q);
		    d_mask(p_acb->dma_channel_id);
		    d_clear(p_acb->dma_channel_id);

		    /* Unregister the interrupt handler */
		    i_clear(&p_acb->slih_intr);

		    MPQTRACE2("Ctm6", num_acbs);

		    /* decrement number of active adapters */
		    num_acbs--;

		    free_resources(num_acbs, devno, p_dds, p_acb);
		    acb_dir[adap_num] = (t_acb *)NULL;
	    }
	    else
	    {
		    MPQTRACE1("Ctm7");
		    free_resources(num_acbs, devno, p_dds, NULL);
		    p_acb->p_port_dds[port_num] = NULL;
	    }

	    dds_dir[minor_num] = NULL;
	    break;

	} /* end of case CFG_TERM */


	case CFG_QVPD:		/* query vital product data		*/
	    break;

	default:
	{

#ifdef _POWER_MPQP
	    unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

	    return(EINVAL);
	} /* end of case default */

    } /* end of switch statement */

    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_CONFIG, 0, devno, 0, 0, 0, 0);

#ifdef _POWER_MPQP
    unlockl( &mpqp_cfg_lock );
#endif /* _POWER_MPQP */

    return(0);
}

/*
 *  free_resources - frees up allocated resources
 *                   if number of active adapters is now zero it
 *                   also deletes switch table entry and unpins driver
 */

free_resources( int             num_adapters,
		dev_t           devno,
		t_mpqp_dds      *p_dds,
		t_acb           *p_acb)
{
	int     i;              /* loop variable */

	/* if valid acb pointer then free the acb memory */
	if (p_acb)
	{
		/*  first release memory for tx buf queue */
		if (p_acb->p_lcl_txfree_buf_q) {
			xmfree((char *)p_acb->p_lcl_txfree_buf_q,
				   pinned_heap);
		}
		/* free the acb itself */
		xmfree((char *)p_acb, pinned_heap);
	}

	/* if valid dds pointer then free the dds memory */
	if (p_dds)
	{
		/* first release memory associated with this dds */
		for (i=0; i < MAX_CHAN; i++)
		{
		    if (p_dds->dds_wrk.p_chan_info[i] != NULL)
		    {
			xmfree((char *)p_dds->dds_wrk.p_chan_info[i],
				pinned_heap);
		    }
		}

		/* now free the dds itself */
		xmfree((char *)p_dds, pinned_heap);
	}

	/* if no more active adapters then .... */
	if (num_adapters == 0)
	{
		timeoutcf(-64);
		devswdel(devno);

#ifdef _POWER_MPQP
		free_locks();
#endif /* _POWER_MPQP */

		unpincode(mpqconfig);
	}

	return;
}


/*
 *  set_pos - set Programmable Option Select (POS) registers for the
 *  		 adapter 
 */

set_pos ( t_acb         *p_acb )
{
    unsigned long               iocc_sr;
    volatile unsigned long      pos_ptr;
    unsigned long		slot;
    unsigned long		bus_num;
    unsigned char		t_pos;


	/* get the slot number */

	bus_num = (((p_acb->iocc_segreg_val >> 20) & 0xff) - 0x20);
	slot = p_acb->slot_num - (bus_num * NUM_SLOTS );

	/* get access to bus I/O space */

	iocc_sr = IOCC_ATT( p_acb->iocc_segreg_val, 0 );
	pos_ptr = iocc_sr + ( slot << 16 ) + IO_IOCC;

	/* set up POS registers */

	t_pos = (unsigned char)0;

	/* POS2 contains CARDID + interrupt level + sleep enable */
	/* currently manufacturing support interrupt level = 7	*/
	/* Added 0x80 bit for Pulsed I/O Channel Check +266+	*/
	/*   7     6     5     4     3     2     1     0
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	  | CKP | Card Base Addr. | Interrupt Level | SE  |
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	 */

	 /* first we squeeze in io base address info in temp var */

	t_pos = (p_acb->io_base - 0x2a0) >> 6;

	/* next we add in interrupt level info */

	switch ( p_acb->int_lvl )
	{
	    case 3:
		t_pos |= P2_INT3;
		break;
	    case 4:
		t_pos |= P2_INT4;
		break;
	    case 7:
		t_pos |= P2_INT7;
		break;
	    case 9:
		t_pos |= P2_INT9;
		break;
	    case 10:
		t_pos |= P2_INT10;
		break;
	    case 11:
		t_pos |= P2_INT11;
		break;
	    case 12:
		t_pos |= P2_INT12;
		break;
	    default:
		break;                  /* should never happen */
	}

	/* now we set the sleep enable and sync check bits */

	t_pos |= ( P2_ENABLE | P2_SYNC_CHCK );

	PIO_PUTC( pos_ptr + POS2, t_pos );

	/* POS3 contains addressing information */
	/*   7     6     5     4     3     2     1     0
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	  | A20 | A19 | A18 | A17 | A16 | A15 | A14 | A13 |
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	 */

	/* Note: this assumes a 64K window and must change for different */
	/*       window sizes */

	t_pos = ( p_acb->mem_base & 0x001f0000 ) >> 13;

	PIO_PUTC( pos_ptr + POS3, t_pos );

	/* POS4 contains addressing info and window size */
	/*   7     6     5     4     3     2     1     0
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	  | Window Size Inf | A31 | A30 | A23 | A22 | A21 |
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	 */

	t_pos = ( p_acb->mem_base & 0xc0000000 ) >> 27;
	t_pos |= ( p_acb->mem_base & 0x00e00000 ) >> 21;
	t_pos |= P4_WSIZ_64K;

	PIO_PUTC( pos_ptr + POS4, t_pos );

	/* POS5 contains Bus Arbitration Level, Fairness Enabled Bit */
	/* Data Parity Enabled Bit, and two other undescribed bits */
	/*   7     6     5     4     3     2     1     0
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	  | CKI | CKS | DPE | Bus Arbitration Level | FE  |
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	 */

	t_pos = p_acb->arb_lvl << 1;
	t_pos |= P5_FAIRNESS;

	PIO_PUTC( pos_ptr + POS5, t_pos );

	/* now set up acb variables for addresses and POS values */

	p_acb->pos0 = PIO_GETC ( pos_ptr + POS0 );	/* get POS0 value */
	p_acb->pos1 = PIO_GETC ( pos_ptr + POS1 );	/* get POS1 value */
	p_acb->pos2 = PIO_GETC ( pos_ptr + POS2 );	/* get POS2 value */
	p_acb->pos3 = PIO_GETC ( pos_ptr + POS3 );	/* get POS3 value */
	p_acb->pos4 = PIO_GETC ( pos_ptr + POS4 );	/* get POS4 value */
	p_acb->pos5 = PIO_GETC ( pos_ptr + POS5 );	/* get POS5 value */
	p_acb->pos6 = PIO_GETC ( pos_ptr + POS6 );	/* get POS6 value */
	p_acb->pos7 = PIO_GETC ( pos_ptr + POS7 );	/* get POS7 value */

	IOCC_DET( iocc_sr );

	return;
}

/*
 * NAME: MPQSaveTrace()
 *
 * FUNCTION: enter trace data in the trace table
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

void MPQSaveTrace (int num_args, char *arg1,
	int arg2, int arg3, int arg4, int arg5)
{
	int     trcdata[5];
	int     ndx;
	char    *s;
	int     saved_intr_level;

#ifdef _MPQP_DEBUG
	printf("MPQP Trace ->: %s, %d, %d, %d, %d\n", arg1, arg2, arg3, arg4, arg5);
#else
	/* if there's a chance driver isn't pinned, don't disable interrupts */
	if (num_acbs != 0)
	{
#ifdef _POWER_MPQP
		MPQP_LOCK_DISABLE(saved_intr_level,&mpqp_trace_lock);
#else
		saved_intr_level = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
	}


	for ( ndx = 1; ndx<5; ndx++)
		trcdata[ndx] = 0;

	for (ndx=0, s=(char*)trcdata; ((ndx<4) && (arg1[ndx] != 0)); ndx++, s++)
		*s = arg1[ndx];         /* copy id chars from first argument */

	MPQTraceTable[MPQnexthole] = trcdata[0];
	MPQnexthole = (MPQnexthole +1) & (TRACE_TABLE_SIZE -1);

	if (num_args >= 2)
	{
		trcdata[1] = arg2;
		MPQTraceTable[MPQnexthole] = arg2;
		MPQnexthole = (MPQnexthole +1) & (TRACE_TABLE_SIZE -1);
	}

	if (num_args >= 3)
	{
		trcdata[2] = arg3;
		MPQTraceTable[MPQnexthole] = arg3;
		MPQnexthole = (MPQnexthole +1) & (TRACE_TABLE_SIZE -1);
	}

	if (num_args >= 4)
	{
		trcdata[3] = arg4;
		MPQTraceTable[MPQnexthole] = arg4;
		MPQnexthole = (MPQnexthole +1) & (TRACE_TABLE_SIZE -1);
	}

	if (num_args >= 5)
	{
		trcdata[4] = arg5;
		MPQTraceTable[MPQnexthole] = arg5;
		MPQnexthole = (MPQnexthole +1) & (TRACE_TABLE_SIZE -1);
	}

	TRCHKGT((HKWD_DD_MPQPDD | (DD_MPQ_HOOK<<8)),
		trcdata[0], trcdata[1], trcdata[2], trcdata[3], trcdata[4]);

	MPQTraceTable[MPQnexthole] = 0x2A2A2A2A; /* mark end of the good data */
	if (num_acbs != 0)
	{
#ifdef _POWER_MPQP
	        MPQP_UNLOCK_ENABLE(saved_intr_level,&mpqp_trace_lock);
#else
		i_enable(saved_intr_level);
#endif /* _POWER_MPQP */
	}
#endif /* _MPQP_DEBUG */
	return;
} /*                 end MPQSaveTrace                                      */
#ifdef _POWER_MPQP
/*
 * NAME: init_locks()
 *
 * FUNCTION: enter trace data in the trace table
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

void init_locks()
{
	
	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Allocate and initialize the driver lock ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	lock_alloc(&mpqp_mp_lock,LOCK_ALLOC_PIN,MPQP_ADAP_LOCK, -1);
	simple_lock_init(&mpqp_mp_lock);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Allocate and initialize the trace lock ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	lock_alloc(&mpqp_trace_lock,LOCK_ALLOC_PIN,MPQP_TRACE_LOCK, -1);
	simple_lock_init(&mpqp_trace_lock);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Allocate and initialize the IOCTL lock ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	lock_alloc(&mpqp_ioctl_lock,LOCK_ALLOC_PIN,MPQP_IOCTL_LOCK,-1);
	simple_lock_init(&mpqp_ioctl_lock);

	/*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
	³ Allocate and initialize the interrupt lock ³
	ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
	lock_alloc(&mpqp_intr_lock,LOCK_ALLOC_PIN,MPQP_INTR_LOCK,-1);
	simple_lock_init(&mpqp_intr_lock);

	return;
}

/*
 * NAME: free_locks()
 *
 * FUNCTION: enter trace data in the trace table
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

void free_locks()
{
        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ Free up the device driver lock ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        (void)lock_free((void *)&mpqp_mp_lock);

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ Free up the internal trace table lock ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        (void)lock_free((void *)&mpqp_trace_lock);

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ Free up the IOCTL lock ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        (void)lock_free((void *)&mpqp_ioctl_lock);

        /*ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
        ³ Free up the SLIH and OFFL lock ³
        ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ*/
        (void)lock_free((void *)&mpqp_intr_lock);
        return;
}
#endif /* _POWER_MPQP */
