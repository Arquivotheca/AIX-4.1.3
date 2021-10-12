static char sccsid[] = "@(#)78	1.56  src/bos/kernel/ios/POWER/initiocc.c, sysios, bos41J 5/2/95 16:53:43";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: initiocc
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/iocc.h>
#include <sys/iplcb.h>
#include <sys/nio.h>
#include <sys/machine.h>
#include <sys/except.h>
#include <sys/syspest.h>
#include <sys/vmker.h>
#include <sys/sysdma.h>
#include <sys/ppda.h>
#include <sys/systemcfg.h>
#include <sys/sys_resource.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/inline.h>
#include "intr_hw.h"
#include "dma_hw.h"
#include "interrupt.h"
#ifdef _RS6K_SMP_MCA
#include "pegasus.h"
#endif /* _RS6K_SMP_MCA */
#ifdef _RSPC
#include <sys/system_rspc.h>
#include <sys/residual.h>
#include <sys/genadpnp.h>
#include "mpic.h"
#endif
#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif /* _POWER_MP */

#ifdef _POWER
extern int num_ioccs;
#endif /* _POWER */

extern uint nio_buid, nio_slot;
extern uint nio_posdata;
extern struct io_map	nio_map;

#ifndef _POWER_MP
extern	struct ppda	ppda[];	/* Need this for PPDA macro */
#endif /* _POWER_MP */

#ifdef _RS6K_SMP_MCA
#define APR_BIT_INDEX(intr) \
	((struct sys_interrupt_regs *)intr - \
	 &sys_resource_ptr->sys_interrupt_space.sys_intr_regs[0])
#endif /* _RS6K_SMP_MCA */

#ifdef _POWER_RS
/*
 * NAME: init_iocc_pwr
 *
 * FUNCTION: Initialize a POWER IOCC if it is present
 *
 * EXECUTION ENVIORMENT:
 *	This is called after VMSI from ios_init.  This is still early in
 * 	system initialization, but we are able to take IO exceptions
 *
 * RETURNS: None
 */
static void
init_iocc_pwr(
	ulong bid,			/* iocc bid */
	int mlvl)			/* level to map IOCC to */
{
	int num_slave_tcws;		/* tcws for slave channels  */
	int num_master_tcws;		/* tcws for master channels */
	int num_slave_chan;		/* number of slave channels */
	int first_slv_tcw;		/* number of TCW used for slaves */
	volatile struct	iocc *i;	/* IOCC */
	ulong value;			/* interrupt level mask */
	int level;			/* interrupt level */
	int channel;			/* DMA channel number */
	struct slave_regs *d;		/* DMA registers */
	ulong tag_num;			/* tag table index */
	struct tag *t;			/* tag entry */
	uint num_tcws;			/* number of TCWs */
	uint tcw_num;			/* TCW index */
	uint ioccaddr;			/* Effective address of IOCC */
	void init_nio();

	/* Setup addressibility to the IOCC.
	 */
	ioccaddr = (uint) io_att(bid, 0);
	i = (volatile struct iocc *)(ioccaddr + IO_IOCC);

	/*
	 * Initialize the IOCC interrupt control registers. All interrupts
	 * are disabled. Any pending interrupts are cleared. The bus
	 * interrupt levels are mapped to the most significant bits in EIS0.
	 *
	 * The IOCC is now in a state so that interrupt requests
	 * will not be processed. The i_init service must be
	 * called before taking an interrupt.
	 */
	i->int_enable = 0;		/* disable all IOCC interrupts */
	i->int_misc = 0;		/* turn off miscellaneous interrupts */
	i->rfi = 0;			/* clear all pending interrupts */

	/* Initialize the interrupt vector table, unless we are on a RSC
	 * machine, where the stores will except.  This is useless for RS2
	 * but it won't hurt anything.
	 */
	if (!__power_rsc())
	{
		value = (mlvl << 24) | ((mlvl+1) << 16) | ((mlvl+2) << 8) |
								(mlvl+3);
		for ( level = 0; level < 4; level++ )
		{
			i->vector[level] = value;
			value += 0x04040404;
		}
	}

	/*
	 * Initialize the IOCC DMA control registers. All DMA 
	 * channels are disabled. All buffers are marked as invalid.
	 *
	 * The IOCC is now in a state so that DMA requests
	 * will not be processed. The d_init service must be
	 * called before performing DMA operations.
	 */
	for ( channel = 0; channel < 16; channel++ )
	{
		d = SLAVE_EFF_ADDR(channel);
		d->control = 0;
		d->raddr = -1;
		d->buf_ctl = BUF_INVALID;
	}

	num_tcws = ICF_TCWS(i->config);
	num_slave_chan = ICF_NUM_SLVCHN(i->config);


	/*
	 * Check if DMA slave opperations will be 
	 * done with TCWs or TAGs
	 */
	if (i->config & ICF_SLAVE_TCW)
	{

		/*
		 * allocate:
		 *	TCWs 16 -- (first_slave_tcw-1) for bus master
		 *      opperations
		 *
		 *	TCW fist_slave_tcw -- (num_tcws-1) are for 
		 *      slave operations, with TAGS_PER_CHANNEL TCWs 
		 * 	allocated for each channel
		 */
		num_slave_tcws = ICF_SLV_TCWS(i->config);
		num_master_tcws = ICF_NUM_TCWS(i->config);
		first_slv_tcw = num_master_tcws;
	}
	else
	{

		/*
		 * initialize TAG table
		 */
		for ( tag_num = 0; tag_num < (TAGS_PER_CHANNEL * 16);
							 tag_num++ )
		{
			t = TAG_EFF_ADDR(tag_num);
			t->control = 0;
			t->raddr = -1;
		}

		/*
		 * allocate all TCWs for use by DMA master opperations
		 */
		num_slave_tcws = 0;
		num_master_tcws = num_tcws;
		first_slv_tcw = -1;

	}

	/*
	 * Initialize the TCWs allocated for DMA masters.
	 */
	for (tcw_num = 0; tcw_num < num_master_tcws; tcw_num++)
	{
		iocc_rw(IOCC_WRITE, TCW_EFF_ADDR(tcw_num),
				MASTER_TCW(-1, 0xF, 0, TCW_NO_ACCESS));
		tlbi(tcw_num << DMA_L2PSIZE);
	}

	/*
	 * initialize TCWs allocated for DMA slave
	 */
	for (tcw_num = num_master_tcws; tcw_num < num_tcws; tcw_num++)
	{
		iocc_rw(IOCC_WRITE, TCW_EFF_ADDR(tcw_num), 
			SLAVE_TCW(-1));
		tlbi(tcw_num << DMA_L2PSIZE);
	}

	/* If IOCC 0 is being initialized set global vaiables that 
	 * are tested in IOS services
	 */
	if (bid == IOCC0_BID)
	{
		iocc_config = i->config;
		first_slave_tcw = first_slv_tcw;
	}

        /*
         * Call the Native I/O Initialization to setup any NIO that
         * may be on this IOCC
         */
        init_nio(ioccaddr, bid);

	/*
	 * Clean up after IOCC initialization.
	 */
	io_det( ioccaddr );
}
#endif /* _POWER_RS */

#ifdef _RS6K
/*
 * NAME: init_iocc_ppc
 *
 * FUNCTION: Initialize a PowerPC IOCC if it is present
 *
 * EXECUTION ENVIORMENT:
 *	This is called after VMSI from ios_init.  This is still early in
 * 	system initialization, but we are able to take IO exceptions
 *
 * RETURNS: None
 */
static void
init_iocc_ppc(
	int   i_cnt,			/* iocc_info index */
        ulong bid)                      /* iocc bid */
{
	int num_master_tcws;		/* tcws for master channels */
	int num_slave_chan;		/* number of slave channels */
	volatile struct	iocc_ppc *i;	/* IOCC */
	ulong value;			/* interrupt level mask */
	int level;			/* interrupt level */
	uint tcw_num;			/* TCW index */
	uint ioccaddr;			/* Effective address of IOCC */

	struct iocc_info *dp;	/* Pointer to IOCC DMA info struct */
	uint  cfg_reg;			/* contents of IOCC config register */
	struct ipl_cb *iplcb_ptr;	/* ipl cb pointer   */
	void init_nio();

	/* Setup addressibility to the IOCC.
	 */
	ioccaddr = (uint) io_att(bid, 0);
	i = (volatile struct iocc_ppc *)(ioccaddr + IO_IOCC_PPC);

	/*
	 * Initialize the IOCC interrupt control registers. All interrupts
	 * are disabled. Any pending interrupts are cleared. 
	 *
	 * The IOCC is now in a state so that interrupt requests
	 * will not be processed. The i_init service must be
	 * called before taking an interrupt.
	 */
	i->int_enable = 0;		/* disable all IOCC interrupts */
	i->int_misc = 0;		/* turn off miscellaneous interrupts */

	/*
	 * Initialize the interrupt vector table(XIVR) and
	 * issue an EOI for each level
	 */
	value = 0xffff;			/* 0xffXX = Any processor	*/
					/* 0xXXff = least favored prority */
	for ( level = 0; level < NUM_BUS_SOURCE; level++ )
	{
		i->xivr[level] = value;
		i->eoi[level] = 0;
	}

	/*
	 * 	Read IOCC configuration Register
	 */
	cfg_reg = i->config;

	/*
	 * 	Set up a pointer to this IOCC's info structure
	 */
	dp = (struct iocc_info *)&iocc_info[i_cnt];

	/*
	 * 	Initialize DMA channel allocation bitmask (1 = free)
	 */
	dp->dma_chans = INIT_DMA_CHAN_TAB();

#ifdef _POWER_MP
	lock_alloc( &dp->iocc_lock, LOCK_ALLOC_PIN, DMA_LOCK_CLASS, i_cnt );
	simple_lock_init( &dp->iocc_lock );
#endif

	/*
	 * 	Compute Number of Slave Channels, and initialize
	 *	the control register allocation bitmask
	 */
	dp->num_slave_chans = NUM_SLVCHN_PPC(cfg_reg);
	dp->slave_ctrl_regs = INIT_SLV_CTRL_TAB(dp->num_slave_chans);

	/*
	 * 	Initialize TCE Information
	 */
	dp->num_tces = NUM_TCES_PPC(i->tce_addr_low);
	dp->first_slave_tce = num_master_tcws 
			   = dp->num_tces - SLV_TCES_PPC(cfg_reg);

	/*
	 * 	Initialize the TCEs allocated for DMA masters.
	 */
	for (tcw_num = 0; tcw_num < num_master_tcws ; tcw_num++) {
		/*
		 * 	for each master TCE, set it up for page fault
		 */
		*(uint *)(TCE_EFF_ADDR_PPC(dp->tce_offset, tcw_num)) =
			(uint)MASTER_TCE_PPC(-1, 0, TCE_PAGE_FAULT);
	}

	/*
	 * 	Initialize TCEs allocated for DMA slave
	 */
	for (tcw_num = num_master_tcws; tcw_num < dp->num_tces; 
							tcw_num++) {
		*(uint *)(TCE_EFF_ADDR_PPC(dp->tce_offset, tcw_num)) =
			(uint)SLAVE_TCE_PPC(-1);
	}

	/*
	 * 	Perform SYNC so everything is globally seen
	 */
	__iospace_sync();

        /*
         * Call the Native I/O Initialization to setup any NIO that
         * may be on this IOCC
         */
        init_nio(ioccaddr, bid);

	/*
	 * Clean up after IOCC initialization.
	 */
	io_det( ioccaddr );
}
#endif /* _POWER_PC */


/*
 * NAME: init_nio
 *
 * FUNCTION: This routine is called to initialize a NIO card and 
 *	     component reset register.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service executes in the early part of system initialization,
 *	prior to fully establishing a process or interrupt handler
 *	execution environment.
 *	Called by iocc_init().
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 */
void
init_nio(ulong ioccaddr, 		/* ioccaddr attached by initiocc() */
	 ulong bid)			/* BUSID of current IOCC */
{
	ulong	ioaddr;			/* io_att return value */
	int	posdata;		/* hold pos info */
	int	slot,i;
	struct ipl_cb *iplcb_ptr;		/* ipl cb pointer   */
	struct ipl_directory *dir_ptr;		/* iplcb directeory */
	char	*vpd_ptr;			/* iplcb VPD pointer */
	char	NIO_level=0;			/* level of NIO chip */

	/* If this is a Rainbow box, Look in the iplcb to determine the 
	 * NIO chip level.
	 * NOTE: The VPD fields of the IPLCB are not present on all
	 *       implementations.
	 */
	if (__rs6k_up_mca()) {
		iplcb_ptr = (struct ipl_cb *) vmker.iplcbptr;
		dir_ptr = &(iplcb_ptr->s0);
		vpd_ptr = (char *) ((char *) iplcb_ptr +
						dir_ptr->system_vpd_offset);
		NIO_level = vpd_ptr[dir_ptr->system_vpd_size - 1];
	}

  	/* 
	 * Remove the hard reset to the NIO card via the component
	 * reset register.  Also, remove the slot resets. 
	 */
#if defined(_KDB)
	if (!__kdb()) /* reset done by kdb */
#endif /* _KDB */
	if (__rs6k()) {
		if ( __rs6k_up_mca() && (NIO_level == 0x01)) {
			/* this is a 601 box with DD1 NIO, we can't read the
			 * Component Reset Reg...
			 */
  			*(volatile uint *)(ioccaddr + COMP_RESET_REG_PPC) = 0;
			for (i=0;i<1000000;i++)
  				*(volatile uint *)(ioccaddr + IOCC_DELAY) = 0;
  			*(volatile uint *)(ioccaddr + COMP_RESET_REG_PPC) = 
								0xff000007;
		} else if (__rs6k_smp_mca()) {
		  	/* With IONIAN, we can't read the Component
			 * Reset Register
			 */
		  	*(volatile uint *)(ioccaddr + COMP_RESET_REG_PPC) = 
			  					0xffff0007;
		} else {
			/*
		  	 * This is the right way to remove the reset
			 */

  			*(volatile uint *)(ioccaddr + COMP_RESET_REG_PPC) |= 
								0xff000007;
		}

	} else
  		*(volatile uint *)(ioccaddr + COMP_RESET_REG) |= 0xff000007;

	/*
	 * Determine if NIO is present.
	 */
	for (slot=0;slot < MAX_SLOTS;slot++) {
		posdata = (*(volatile uchar *)
					(ioccaddr + POS_ADDR(slot, 0))) << 8;
		posdata |= *(volatile uchar *)(ioccaddr + POS_ADDR(slot, 1));
		posdata &= 0xffff;

		switch (posdata) {
		    /* RS1 planars
		     */
		    case NIO_POS0_REL1:
		    case NIO_POS0_REL2:

#if defined(_KDB)
#ifdef _POWER_RS
			if (__kdb() && __power_rs()) {
				if (bid == IOCC0_BID) {
					nio_buid = bid;
					nio_posdata = posdata;
					nio_slot = slot;
					break; /* reset already done by kdb */
				}
			}
#endif /* _POWER_RS */
#ifdef _RS6K
			if (__kdb() && __rs6k()) {
				if (bid == iocc_info[0].bid) {
					nio_buid = bid;
					nio_posdata = posdata;
					nio_slot = slot;
					break; /* reset already done by kdb */
				}
			}
#endif /* _RS6K */
#endif /* _KDB */
			/*
			 * Reset the Reset/VPD POS register.    
			 */
		      	*(volatile unsigned char *)
					(ioccaddr + POS_ADDR(slot,2)) = 0xbd;
		      	*(volatile unsigned char *)
					(ioccaddr + POS_ADDR(slot,2)) = 0xbf;

			/*
			 * Setup addressibility to the IO space.
			 */
			ioaddr = (ulong) io_att((bid | IO_SEG_REG),0);

			/*
			 * Initialize the keyboard with interrupts disabled.
			 */
		      	*(volatile unsigned char *)
					(ioaddr + KBD_CONFIG_REG) = 0xc3;
		      	*(volatile unsigned char *)
					(ioaddr + KBD_CONFIG_REG) = 0x08;

			/*
			 * Clean up after IO initialization.
			 */
			io_det( ioaddr );

			/*
			 * Remove reset mode.
			 */
		      	*(volatile unsigned char *)
					(ioccaddr + POS_ADDR(slot,2)) = 0x03;
			
			/*
			 * set up the global NIO Bus Id field
			 */
			nio_buid = bid;
			nio_posdata = posdata;
			nio_slot = slot;
			break;

		/* RSC and 601 planars
		 */
		case NIO_POS0_RSC1:

#if defined(_KDB)
#ifdef _POWER_RS
			if (__kdb() && __power_rs()) {
				if (bid == IOCC0_BID) {
					nio_buid = bid;
					nio_posdata = posdata;
					nio_slot = slot;
					break; /* reset already done by kdb */
				}
			}
#endif /* _POWER_RS */
#ifdef _RS6K
			if (__kdb() && __rs6k()) {
				if (bid == iocc_info[0].bid) {
					nio_buid = bid;
					nio_posdata = posdata;
					nio_slot = slot;
					break; /* reset already done by kdb */
				}
			}
#endif /* _RS6K */
#endif /* _KDB */
			/* Reset the standard IO devices
			 */

			*(volatile unsigned char *)
					(ioccaddr + POS_ADDR(slot,2)) = 0xff;

			if (__rs6k_up_mca() && (NIO_level == 0x01)) {
				/*
				 * if this is a Rainbow box with a DD1 NIO chip
				 * then we must disable data parity checking
				 */
				*(volatile unsigned char *)(ioccaddr + 
						POS_ADDR(slot,2)) = 0x01;
			} else {
				/*
				 * set up the normal value
				 */
				*(volatile unsigned char *)(ioccaddr + 
						POS_ADDR(slot,2)) = 0x03;
			}
			/*
			 * set up the global NIO Bus Id field
			 */
			nio_buid = bid;
			nio_posdata = posdata;
			nio_slot = slot;
			break;

#ifdef _RS6K_SMP_MCA
		case NIO_POS0_PEGASUS:
#if defined(_KDB)
			if (__kdb() && __rs6k_smp_mca()) {
				if (bid == iocc_info[0].bid) {
					nio_buid = bid;
					nio_posdata = posdata;
					nio_slot = slot;
					break; /* reset already done by kdb */
				}
			}
#endif /* _KDB */
			/* Reset the 3rd serial line, enable Native IO
			 */
			*(volatile unsigned char *)
					(ioccaddr + POS_ADDR(slot,2)) = 0x05;
			*(volatile unsigned char *)
					(ioccaddr + POS_ADDR(slot,2)) = 0x01;
			/*
			 * set up the global NIO Bus Id field
			 */
			nio_buid = bid;
			nio_posdata = posdata;
			nio_slot = slot;
			break;
#endif /* _RS6K_SMP_MCA */

		default:
			break;
			
		}
	}

  	return ;
}



#ifdef _POWER_RS
/*
 * NAME: init_ios_pwr
 *
 * FUNCTION: This routine is called by ios_init() to perform POWER platform
 *	specific initialization. It initializes the hardware for the 
 *	I/O Subsystem.
 *	This includes the following:
 *
 *		Initialize the decrementer's EIS level.
 *		Initialize the EPOW's EIS level.
 *		Initialize the PC AT to Micro Channel converter.
 *		Initialize the IOCC(s).
 *		Initialize the Native I/O.
 *		Initialize the EIMs and EISs - if RS1/RSC.
 *		Initialize the PEISs and CIL - if RS2.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service executes in the early part of system initialization,
 *	prior to fully establishing a process or interrupt handler
 *	execution environment.
 *
 * NOTES:
 *	Check the IPL contol block before touching buid 21.  This is
 *	because some Lamp. machines will check stop instead of DSI on
 *	a buid 21 access.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      io_att, io_det
 *
 */
void
ios_init_pwr()
{
	volatile ulong	*dec;			/* EICR */
	volatile ulong	*exch;			/* EICR */
	volatile ulong	*epow;			/* EICR */
#if defined( _POWER_RS1 ) || defined( _POWER_RSC )
	volatile ulong	*eim;			/* EIM */
	volatile ulong	*eis;			/* EIS */
	ulong trash;				/* return data ignored */
#endif /* _POWER_RS1 || _POWER_RSC */

	int	i;				/* Loop counter	*/
	ulong sreset_ptr;			/* system reset count ptr */
	ulong ioccaddr;				/* io_att return value */
	struct ipl_cb *iplcb_ptr;		/* ipl cb pointer   */
	struct ipl_info	*info_ptr;		/* ipl info pointer */
	struct iocc_post_results *post_ptr;	/* ipl iocc posts  */
	struct ipl_directory *dir_ptr;		/* iplcb directeory */
        int i_cnt;                              /* count of IOCCs */

	struct ppda	*ppda_ptr;		/* Processor info */

	/*
	 * Tell the decrementer to use INT_TIMLVL. The decrementer can
	 * not be turned off and defaults to using the most significant
	 * EIS bit (bit 0 of EIS0). This must be done before the EIS
	 * and EIM are initialized so that an interrupt will not be
	 * left pending on interrupt level 0.
	 *
	 * PowerPC decrementer has its own low memory vector and is
	 * enabled/disabled by the MSR(EE) bit.
	 */
	ioccaddr = (ulong) io_att( EICRBID, 0 );
	dec = (volatile ulong *)(ioccaddr + DEC_EISBID);
	*dec = INT_TIMLVL;
	io_det( ioccaddr );

	/*
	 * Set the decrementer to max time and hope this is enough time
	 * to get the interrupt subsystem initialized.  If the debugger
	 * is invoked then this won't happen but only side effect of
	 * this is phantom interrupts detected in the flih.
	 *
	 * Any pending bogus decrementer interrupts will be discarded
	 * when the EIS/PEIS is initialized(cleared) later.
	 */

	mtdec( -1 );

	/*
	 * Tell the EPOW handler to use INT_EPOW 
	 */
	ioccaddr = (ulong) io_att( EICRBID, 0 );
	epow = (volatile ulong *)(ioccaddr + EPOW_EISBID);

#if defined( _POWER_RS1 ) || defined( _POWER_RSC )
	if( __power_set( POWER_RS1 | POWER_RSC )) {
		*epow = INT_EPOW_RS1;
	}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
	if( __power_rs2() ) {
		*epow = INT_EPOW_RS2;
	}
#endif /* _POWER_RS2 */

	io_det( ioccaddr );


        /*
         * Tell the external check handler to use INT_SCUB
         */
        ioccaddr = (ulong) io_att( EICRBID, 0 );
        exch = (volatile ulong *)(ioccaddr + MEEB_EISBID);

#if defined( _POWER_RS1 ) || defined( _POWER_RSC )
	if( __power_set( POWER_RS1 | POWER_RSC )) {
        	*exch = INT_SCUB_RS1;
	}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
	if( __power_rs2() ) {
        	*exch = INT_SCUB_RS2;
	}
#endif /* _POWER_RS2 */

        io_det( ioccaddr );

	/*
	 * Initialize IOCCs on the POWER platforms
	 */

	/* Look in the iplcb to determine the number of IOCCs present.
	 * 
	 * When the dust settles down around the iplcb then the method
	 * of locating and identifying an IOCC should differ between
	 * the POWER and PPC boxes.  PPC boxes should pick up the 
	 * assigned BUIDs from there.  For the time being PPC IOCCs
	 * will use BUID 20-23 just like POWER.
	 */
	iplcb_ptr = (struct ipl_cb *) vmker.iplcbptr;
	dir_ptr = &(iplcb_ptr->s0);
	post_ptr = (struct iocc_post_results *) ((char *) iplcb_ptr +
				dir_ptr->iocc_post_results_offset);

	init_iocc_pwr(IOCC0_BID, 0);

	/* Initialize IOCCs */
	if (post_ptr->length == 2) {
		init_iocc_pwr(IOCC1_BID, LVL_PER_WORD);
		num_ioccs = 2;
	}
	else {
		num_ioccs = 1;
	}

        /*
         * Initialize the system reset counter to enable warm IPL
         * For POWER platforms
         */
	sreset_ptr = io_att(IOCC0_BID, 0x004000E0);
	if (__power_rsc()) {
		*(volatile ulong *)sreset_ptr = 0;
	} else {
		*(volatile uchar *)sreset_ptr = 0;
	}
	io_det(sreset_ptr);

#if defined( _POWER_RS1 ) || defined( _POWER_RSC )
	/*
	 *  Clear the EIM and EIS. The i_init service will initialize
	 *  an array of EIM values with one entry for each priority.
	 *  This array is defined in interrupt.h. Code in low.s uses
	 *  this array to set the EIM as per the desired priority.
	 */
	if( __power_set( POWER_RS1 | POWER_RSC ))
	{
	    ioccaddr = (ulong) io_att( EICRBID, 0 );
	    eim = (volatile ulong *)(ioccaddr + EIM0);
	    *eim = 0;
	    eim = (volatile ulong *)(ioccaddr + EIM1);
	    *eim = 0;
	    eis = (volatile ulong *)(ioccaddr + EIS0);
	    trash = *eis;
	    eis = (volatile ulong *)(ioccaddr + EIS1);
	    trash = *eis;
	    io_det( ioccaddr );
	}
#endif /* _POWER_RS1 || _POWER_RSC */

#ifdef _POWER_RS2
	/*
 	 * Clear any bogus pending interrupts and set CIL to most
	 * favored interrupt level.
	 */

	if( __power_rs2() )
	{
	    ulong peis0, peis1;
	    int plvl;

	    mfpeis( &peis0, &peis1 );
	    while( peis0 ) {
		plvl = bitindex( &peis0 );
		peis0 &= ~((ulong)(0x80000000) >> plvl);
		i_peis_reset( plvl );
	    }
	    while( peis1 ) {
		plvl = bitindex( &peis1 );
		peis1 &= ~((ulong)(0x80000000) >> plvl);
		plvl += LVL_PER_WORD;
		i_peis_reset( plvl );
	    }
	    /*
	     * Now make current interrupt level 0
	     */
	    i_cil_set( 0 );
	}
#endif /* _POWER_RS2 */

}

#endif /* _POWER_RS */

#ifdef _RS6K
/*
 * NAME: init_ios_ppc
 *
 * FUNCTION: This routine is called by ios_init() to perform PowerPC platform
 *	specific initialization. It initializes the hardware for the I/O 
 *	Subsystem.
 *	This includes the following:
 *
 *		Initialize the decrementer's EIS level.
 *		Initialize the IOCC(s).
 *		Initialize the Native I/O.
 *		Initialize the CPPR - if PPC.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service executes in the early part of system initialization,
 *	prior to fully establishing a process or interrupt handler
 *	execution environment.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      io_att, io_det
 *
 */
void
ios_init_ppc()
{

        int i_cnt;                              /* count of IOCCs */
	struct ppda	*ppda_ptr;		/* Processor info */
	volatile struct ppcint *intr;		/* PPC interrupt registers */
#ifdef _POWER_MP
	extern 	Simple_lock	int_hw_lock;	/* XIVR/IER/IRR global lock */
	extern	uint	cppr_random;
#endif
	/*
	 * Set the decrementer to max time and hope this is enough time
	 * to get the interrupt subsystem initialized.  If the debugger
	 * is invoked then this won't happen but only side effect of
	 * this is phantom interrupts detected in the flih.
	 *
	 * Any pending bogus decrementer interrupts will be discarded
	 * when the EIS/PEIS is initialized(cleared) later.
	 */

	mtdec( -1 );

	/*
	 * NOTE: No setup necessory for EPOW, as 
	 * _POWER_PC machines are hardwired to use BUID 0 src 1.
	 */

	/*
	 * Initialize all IOCCs that were defined by hardinit()
	 * for the PowerPC platform
	 */
#ifdef _POWER_MP
	lock_alloc(&int_hw_lock, LOCK_ALLOC_PIN, IOS_LOCK_CLASS, 0);
	simple_lock_init(&int_hw_lock);
#endif
	for(i_cnt=0;i_cnt < MAX_NUM_IOCCS;i_cnt++) {
		if (iocc_info[i_cnt].bid != 0xFFFFFFFF) {
			/*
			 * if a valid IOCC
			 */
			init_iocc_ppc(i_cnt, iocc_info[i_cnt].bid);
		}
	}

        /*
         * Initialize the system reset counter to enable warm IPL
         * For PowerPC platforms
         */
#ifdef _RS6K_SMP_MCA
	*rsr_addr = 0;  /* works on all PowerPC platforms */
#else
	sys_resource_ptr->sys_regs.reset_status = 0;
#endif

	ppda_ptr = PPDA;

	/*
	 * Set the MFRR to no interrupt
	 * Set the CPPR to most favored interrupt priority
	 */
	intr = (volatile struct ppcint *)(ppda_ptr->intr);
	intr->mfrr = 0xff;
	intr->i_cppr = INTMAX;
	__iospace_eieio();	/* 601/Ithaca workaround */
#ifdef _POWER_MP
	ppda_ptr->mpc_pend = 0;
	ppda_ptr->mfrr_pend = 0;
	ppda_ptr->mfrr_lock = 0;
#endif /* _POWER_MP */
#ifdef _RS6K_SMP_MCA
	if (__rs6k_smp_mca()) {
		/*
		 * Set the corresponding APR bit to allow interrupts, 
		 * reset other bits (safety).
		 */
		((struct pgs_apr*)(sys_resource_ptr->reserved3))->apr 
			= (1 << (7 - APR_BIT_INDEX(intr)));
		/*
		 * Even balancing for interrupts within the processor complex
		 */
		cppr_random = 1;
	}
#endif /* _RS6K_SMP_MCA */
}
#endif /* _RS6K */

#ifdef _POWER_MP
/*
 * NAME: ios_bs_proc
 *
 * INPUT:
 *	_rs6k:
 *		ppda address (in SPRG0)
 *		ppda->intr
 *	_rspc:
 *		MPIC I/O Map
 *		Physical CPU ID
 *
 * FUNCTION: This routine is called by main_bs_proc() to perform PowerPC
 *	platform specific initialization for boot slave processors:
 *	    _rs6k:
 *		Initialize the CPPR MFRR - if PPC.
 *		Initialize the APR - if PEGASUS.
 *	    _rspc:
 *		Enable interrupts to the local processor by writing down the
 *		  Current Task Priority Register from 0xF (disabled).
 *
 * RETURNS:  None.
 */
void
ios_bs_proc()
{
#ifdef _RS6K
	if (__rs6k())
	{
		struct ppda		*ppda_ptr;	/* Processor info */
		volatile struct ppcint	*intr;		/* PPC interrupt regs */

		ppda_ptr = PPDA;
		/*
		 * Set the MFRR to no interrupt
		 * Set the CPPR to most favored interrupt priority
		 */
		intr = (volatile struct ppcint *)(ppda_ptr->intr);
		intr->mfrr = 0xFF;
		intr->i_cppr = INTBASE;
		ppda_ptr->mpc_pend = 0;
		ppda_ptr->mfrr_pend = 0;
		__iospace_sync();
#ifdef _RS6K_SMP_MCA
		/*
		 * Set the corresponding APR bit to allow interrupts.
		 */
		if (__rs6k_smp_mca())
		{
			((struct pgs_apr*)(sys_resource_ptr->reserved3))->apr 
				|= (1 << (7 - APR_BIT_INDEX(intr)));
		}
	}
#endif /* _RS6K_SMP_MCA */
#endif /* _RS6K */

#if defined( _RSPC ) && defined( _RSPC_MP_PCI )
	if (__rspc() && __rspc_mp_pci())
	{
		int				 phys_cpu;
		volatile struct s_mpic		*Mpic;
		union { struct taskpri		 s;
			unsigned int		 i; } tp;
		extern struct io_map		 mpic_map;

		Mpic = (volatile struct s_mpic *)iomem_att( &mpic_map );

		/*
		 * Enable routing of interrupts to the current processor
		 */
		phys_cpu = my_phys_id();
		tp.i = MpicRead( &Mpic->procreg[phys_cpu].taskpri );
		tp.s.priority = MPIC_PRI(INTBASE);
		MpicWrite( &Mpic->procreg[phys_cpu].taskpri, tp.i );

		/*
		 * Set platform-specific APR.
		 *   Victory: N/A
		 */
		sync();

		iomem_det( (void *)Mpic );
	}
#endif /* _RSPC && _RSPC_MP_PCI */
}
#endif /* _POWER_MP */

#ifdef _RSPC
/*
 * NAME: ios_init_rspc
 *
 * FUNCTION: This routine is called by ios_init() to perform platform
 *	specific initialization. It initializes the hardware for the I/O 
 *	Subsystem. The 8259s are part of the E/ISA Bridge chip.
 *	This includes the following:
 *
 *		Reinitializing the 8259s and/or MPIC
 *		When applicable:
 *		    Setting up the 8259 ELCR registers for edge trigger mode
 *			except IRQ 15 which is level sensitive.
 *		    Setting up the MPIC Interrupt Source Configuration and
 *			Destination registers.
 *		
 *		Initialize NMI
 *
 * NOTE: The 8259s should have been initialized before the kernel gets
 *	 control, to the point of being quiet.  They are reinitialized
 *	 here so AIX can set them up differently if needed.
 *
 * EXECUTION ENVIRONMENT:
 *	This service executes in the early part of system initialization,
 *	prior to fully establishing a process or interrupt handler
 *	execution environment.  You need not acquire any locks because only
 *	one processor is running the operating system yet.
 *
 * EXTERNAL PROCEDURES CALLED:
 */
void
ios_init_rspc()
{
	char			 reg;
	int			 i;
	int			 numIRQ;
	unsigned int		 cpumask;
	unsigned int		 master;	/* Master proc. Phys ID */
	volatile unsigned char	*ioseg;		/* Pointer to IO segment */
	extern struct io_map	 mpic_map;
	extern void		 setup_intctl();
#ifdef _POWER_MP
	extern Simple_lock	 int_hw_lock;	/* MPIC global lock */
#endif

#ifdef _POWER_MP
	lock_alloc(&int_hw_lock, LOCK_ALLOC_PIN, IOS_LOCK_CLASS, 0);
	simple_lock_init(&int_hw_lock);
#endif /* _POWER_MP */

	/* get addressability to IO space */
	ioseg = (char *)iomem_att( &nio_map );

	/* Determine to interrupt architecture.  The global int (machine.h)
	 * mach_model says what platform we're on.  That gets set in
	 * hardinit.c, before this code runs.
	 */

	setup_intctl();				/* Query resid data */

	if ( ( intctl_pri == INT_CTL_8259 ) || ( intctl_sec == INT_CTL_8259 ) )
	{
        	/* Initialize the master 8259 */
        	*(ioseg + INTA00) = ICW1;
		eieio();
        	*(ioseg + INTA01) = ICW2A;
        	eieio();
        	*(ioseg + INTA01) = ICW3A;
        	eieio();
        	*(ioseg + INTA01) = ICW4;
        	eieio();
        	*(ioseg + INTA01) = 0xFF;	/* Mask all interrupts lines */
        	eieio();

        	/* Initialize the slave 8259 */
        	*(ioseg + INTB00) = ICW1;
        	eieio();
        	*(ioseg + INTB01) = ICW2B;
        	eieio();
        	*(ioseg + INTB01) = ICW3B;
        	eieio();
        	*(ioseg + INTB01) = ICW4;
        	eieio();
        	*(ioseg + INTB01) = 0xFF;	/* Mask all interrupts lines */
        	eieio();

        	*(ioseg + ELCR0) = 0;		/* everybody on edge	*/
        	*(ioseg + ELCR1) = 0;
		__iospace_sync();
	}

	if ( intctl_pri == INT_CTL_MPIC )
	{
		union { struct feature0		 s;
			unsigned int		 i; } feat0;
		union { struct globalcfg0	 s;
			unsigned int		 i; } gcfg0;
		union { struct ipivecpri	 s;
			unsigned int		 i; } ivp;
		union { struct timer_vecpri	 s;
			unsigned int		 i; } tvp;
		union { struct vecpri		 s;
			unsigned int		 i; } vp;
		union { struct taskpri		 s;
			unsigned int		 i; } tp;
		volatile struct s_mpic		*Mpic;

		/* Generate the MPIC destination bitmask for the master CPU.
		 */
#ifdef _POWER_MP
		master = my_phys_id();
#else /* _POWER_MP */
		master = 0;
#endif /* _POWER_MP */

		master = 1 << master;

		mpic_map.key     = IO_MEM_MAP;
		mpic_map.flags   = 0;
		mpic_map.size    = 0x40000;
		mpic_map.bid     = REALMEM_BID;
		mpic_map.busaddr = mpic_base;

		/* get addressability to the MPIC
		 */
		Mpic = (volatile struct s_mpic *)iomem_att( &mpic_map );

		/* Initialize the MPIC controller */

		feat0.i = MpicRead( &Mpic->global.feature0 );
		numIRQ = feat0.s.numIRQ;
		if ( numIRQ >= 0xF0 )
			numIRQ = 0xEF;

		/* Generate the MPIC destination bitmask for all possible CPUs
		 * in the system.  This works because every CPU's Current Task
		 * Priority register is currently 0xF, i.e. disabled.  Only
		 * CPUs that start will lower that and enable interrupts to
		 * themselves.  MPIC numIRQ reports 3 for a 4 CPU system.
		 */
		i = feat0.s.numCPU;
		cpumask = ( 1 << ( i + 1 ) ) - 1;

		/* Set the global operating mode to indicate cascaded 8259s
		 */
		gcfg0.i = MpicRead( &Mpic->global.globalcfg0 );
		gcfg0.s.mode = MPIC_GMODE_MIXED;
		MpicWrite( &Mpic->global.globalcfg0, gcfg0.i );

		/* Initialize Per-Processor Registers
		 */
		while( i >= 0 )
		{
			/* There are currently no items of this type */
			i -= 1;
		}

		/* Initialize Global Per-Timer Registers
		 */
		for( i = 0; i <= 3; i += 1 )
		{
			tvp.i = MpicRead( &Mpic->timer[i].timer_vecpri );
			tvp.s.mask = MPIC_MASK_DISABLED;
			MpicWrite( &Mpic->timer[i].timer_vecpri, tvp.i );
		}

		/* Initialize Global Per-IPI Registers
		 */
		for( i = 0; i <= 3; i += 1 )
		{
			ivp.i = MpicRead( &Mpic->global.ivp[i].ipivecpri );
			ivp.s.mask = MPIC_MASK_DISABLED;
			MpicWrite( &Mpic->global.ivp[i].ipivecpri, ivp.i );
		}

		/* Initialize Per-Interrupt Source Registers
		 */
		for( i = 0; i < numIRQ; i += 1 )
		{
			vp.i = MpicRead( &Mpic->intsource[i].vecpri );

			/* Disable the interrupt source */
			vp.s.mask   = MPIC_MASK_DISABLED;
			MpicWrite( &Mpic->intsource[i].vecpri, vp.i );
			eieio();
			/* Its permanent vector value */
			vp.s.vector = MPIC_VECT(i);
			/* Make it level sensitive */
			vp.s.sense  = MPIC_SENSE_LEVEL;
			/* Make it active low, the PCI Default */
			vp.s.polarity = MPIC_POLARITY_LOW;
			MpicWrite( &Mpic->intsource[i].vecpri, vp.i );
			/* Steer to any processor */
			MpicWrite( &Mpic->intsource[i].destination, cpumask );
		}

		if ( intctl_sec == INT_CTL_8259 )
		{
			/* The 8259s are physically wired to MPIC interrupt
			 * zero (0).  The 8259 input to MPIC must be
			 * programmed Active High.  Set the cascaded 8259
			 * source (ISA) to priority INTMAX.  Steer ISA
			 * interrupts to the master processor (Funnelled).
			 * Turn on the MPIC source for the cascaded 8259s (ISA)
			 */
			vp.i = MpicRead( &Mpic->intsource[0].vecpri );
			vp.s.polarity = MPIC_POLARITY_HIGH;
			vp.s.priority = MPIC_PRI(INTMAX);
			MpicWrite( &Mpic->intsource[0].vecpri, vp.i );
			MpicWrite( &Mpic->intsource[0].destination, master );
			eieio();
			vp.s.mask = MPIC_MASK_ENABLED;
			MpicWrite( &Mpic->intsource[0].vecpri, vp.i );
		}
		__iospace_sync();

		/* Enable routing interrupts to this (the master) processor
		 */
		tp.i = MpicRead( &Mpic->procreg[0].taskpri );
		tp.s.priority = MPIC_PRI(INTBASE);
		MpicWrite( &Mpic->procreg[0].taskpri, tp.i );
		eieio();

		/* get addressability back to IO space
		 */
		iomem_det( (void *)Mpic );
		ioseg = (char *)iomem_att( &nio_map );
	}

        /* Enable NMI to permit machine check notifications */
	reg = *(ioseg + NMISC);
	reg &= ~NMISC_INIT;
	*(ioseg + NMISC) = reg;
	eieio();

	*(ioseg + NMIREG) = NMIREG_INIT;
	__iospace_sync();

	iomem_det( (void *)ioseg );

	/* set decrementer to max */
	mtdec( -1 );
}

/*
 * NAME: setup_intctl
 *
 * FUNCTION: This routine is called during system initialization on all
 *	RSPC systems whose machine model byte reads 0x4c.  Such systems
 *	indicate their interrupt control architecture through residual
 *	data.  Only one processor on an MP system will ever run this code.
 *	Once the two system global variables intctl_pri and intctl_sec
 *	are written to indicate the appropriate controllers, the system
 *	should be capable of servicing calls to i_init() and physical
 *	interrupts.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service executes in the early part of system initialization,
 *	prior to fully establishing a process or interrupt handler
 *	execution environment.
 *	This is initialization code, so it has many assertions.
 *
 * EXTERNAL PROCEDURES CALLED:
 *	query_parms_mpic() - later in this file
 *	query_parms_8259() - later in this file
 */
void
setup_intctl()
{
int			 i;
int			 actual;
int			 i8259 = -1;
int			 iMPIC = -1;
int			 npics = 0;
extern void		 query_parms_mpic( struct _GenericAddr * );
extern void		 query_parms_8259( struct _S8_Pack * );
struct ipl_cb		*iplcb;
struct ipl_directory	*idir_ptr;
RESIDUAL		*rp;

	/* Get pointer to RESIDUAL structure at residual_offset in the ipl_cb.
	 */
	iplcb = (struct ipl_cb *)vmker.iplcbptr;
	idir_ptr = (struct ipl_directory *)(&iplcb->s0);
	rp = (RESIDUAL *)((char *)iplcb + idir_ptr->residual_offset);

	for ( i = 0, actual = rp->ActualNumDevices; i < actual; i++ )
	{
	    if ((rp->Devices[i].DeviceId.BaseType == SystemPeripheral) &&
		(rp->Devices[i].DeviceId.SubType == ProgrammableInterruptController))
	    {
		switch ( rp->Devices[i].DeviceId.Interface )
		{
			case ISA_PIC:
			case EISA_PIC:
				assert( i8259 == -1 );	/* Only 1 allowed */
				i8259 = i;
				npics += 1;
				break;

			case MPIC:
				assert( iMPIC == -1 );	/* Only 1 allowed */
				iMPIC = i;
				npics += 1;
				break;

			default:
				assert(0);	/* Unknown int. ctlr */
		}
	    }
	}

	ASSERT(npics > 0 && npics <= 2);

	intctl_pri = INT_CTL_NONE;
	intctl_sec = INT_CTL_NONE;

	if ( iMPIC != -1 )
	{
		intctl_pri = INT_CTL_MPIC;

		query_parms_mpic( (struct _GenericAddr *)
		    &rp->DevicePnPHeap[rp->Devices[iMPIC].AllocatedOffset] );
	}

	if ( i8259 != -1 )
	{
		if ( intctl_pri == INT_CTL_NONE )
			intctl_pri = INT_CTL_8259;
		else
			intctl_sec = INT_CTL_8259;

		query_parms_8259( (struct _S8_Pack *)
		    &rp->DevicePnPHeap[rp->Devices[i8259].AllocatedOffset] );
	}

	return;
}

/*
 * NAME: query_parms_mpic
 *
 * FUNCTION: This routine queries the residual data heap for the MPIC
 *	interrupt controller and extracts its base address.  It also sets
 *	the global variable mpic_base, which is used in the global io_map
 *	structure passed to iomem_att when doing I/O to the MPIC.
 *	Only one processor on an MP system will ever run this code.
 *
 * NOTES:
 *	Passed a pointer to the first object in the MPIC's residual data heap,
 *	the routine scans for the GenericAddr packet which defines the MPIC
 *	itself.  That detection is made because the length is 0x40000 bytes.
 *	All other packets are ignored up to the end packet.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service executes in the early part of system initialization,
 *	prior to fully establishing a process or interrupt handler
 *	execution environment.
 *
 * EXTERNAL PROCEDURES CALLED:
 */
void
query_parms_mpic( struct _GenericAddr	*gen )
{
unsigned long		 i;
struct _GenericAddr	*mpic = (struct _GenericAddr *)0;

	while ( ( gen->Tag & 0xF8 ) != 0x78 )			/* End */
	{
		if ( gen->Tag == 0x84 )				/* GenAddr */
		{
			i = (gen->Length[3] << 24) + (gen->Length[2] << 16)
			  + (gen->Length[1] <<  8) + (gen->Length[0]);

			if ( i == 0x40000 )
				mpic = gen;
		}

		if ( gen->Tag & 0x80 )				/* Large */
			i = ( gen->Count1 << 8 ) + gen->Count0 + 3;

		else						/* Small */
			i = ( gen->Tag & 0x07 ) + 1;

		gen = (struct _GenericAddr *)( (void *)gen + i );
	}

	assert( mpic != (struct _GenericAddr *)0 );

	i = (mpic->Address[3] << 24) + (mpic->Address[2] << 16)
	  + (mpic->Address[1] <<  8) + (mpic->Address[0]);

	switch( mpic->AddrType )
	{
	    case ISAIOAddr:	i += 0x80000000;	/* PCI I/O base */
				break;
	    case IOMemAddr:	i += 0xC0000000;	/* PCI Memory base */
				break;
	    case SysMemAddr:	break;			/* Address OK */
	    default:		assert(0);
	}

	mpic_base = i;				/* Global mpic_base address */
	return;
}

/*
 * NAME: query_parms_8259
 *
 * FUNCTION: This routine is a stub.  Eventually, it will scan the residual
 *	data heap for the 8259 interrupt controller and extract pertinent
 *	information.
 *	Only one processor on an MP system will ever run this code.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service executes in the early part of system initialization,
 *	prior to fully establishing a process or interrupt handler
 *	execution environment.
 *
 * EXTERNAL PROCEDURES CALLED:
 */
void
query_parms_8259( struct _S8_Pack	*s8 )
{
	return;
}
#endif /* _RSPC */

/*
 * NAME: init_ios
 *
 * FUNCTION: This routine is called early in system initialization by
 *	main.c. It initializes the hardware for the I/O Subsystem.
 * 	This routine determines the platform/machine type and calls the
 *	appropriate hardware specific routine.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This service executes in the early part of system initialization,
 *	prior to fully establishing a process or interrupt handler
 *	execution environment.
 *
 * EXTERNAL PROCEDURES CALLED:
 */
void
ios_init()
{
#ifdef _POWER_PC
	/*
	 * Register REALMEM_BID.  This is used by base kernel
	 * to access memory mapped system resources on Power
	 * PC platforms.
	 */
	if (__power_pc()) {

		struct businfo bi;
		int rc;

		bi.bid = REALMEM_BID;
		bi.d_map_init = NULL;
		bi.num_regions = 1;
		bi.ioaddr[0] = 0;

		rc = bus_register(&bi);
		assert(rc == 0);
	}
#endif /* _POWER_PC */


#ifdef _RS6K
	if (__rs6k()) {
		/*
		 * This is a PowerPC machine, so call PPC version
		 */
		ios_init_ppc();
	} 
#endif /* _RS6K */

#ifdef _POWER_RS
	if (__power_rs()) {
		/*
		 * This is a POWER machine, so call POWER version
		 */
		ios_init_pwr();
	}
#endif /* _POWER_RS */

#ifdef _RSPC
	if (__rspc()) {
		/*
		 * This is a RSPC base Power PC
		 */
		ios_init_rspc();
	}
#endif /* _RSPC */

}

