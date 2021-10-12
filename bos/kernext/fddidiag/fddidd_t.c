static char sccsid[] = "@(#)78	1.3  src/bos/kernext/fddidiag/fddidd_t.c, diagddfddi, bos411, 9428A410j 3/7/94 10:26:37";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: cfg_pos_regs
 *		fddi_cdt_add
 *		fddi_cdt_del
 *		fddi_cdt_init
 *		fddi_cdt_undo_init1505
 *		fddidiag_config
 *		fddi_config_init
 *		fddi_config_qvpd
 *		fddi_config_term
 *		fddi_gen_crc
 *		get_fr_xcard_vpd1307
 *		get_sc_xcard_vpd1222
 *		get_vpd
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "fddiproto.h"
#include <sys/ioacc.h>
#include <sys/adspace.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include "fddi_comio_errids.h"
#include <sys/malloc.h>
#include <sys/iocc.h>

/* 
 * get access to the global device driver control block
 */
extern fddi_ctl_t	fddi_ctl;
extern fddi_trace_t	fdditrace;

/*
 * NAME: fddidiag_config()
 *                                                                    
 * FUNCTION: Handles the CFG_INIT, CFG_TERM, CFG_QVPD sysconfig cmds
 *                                                                    
 * EXECUTION ENVIRONMENT:
 * 	This routine executes on the process thread only. 
 *                                                                   
 * NOTES:
 *	
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES:
 *	Modifies the global fddi_ctl structure.
 *
 * ROUTINES CALLED:
 *	minor(), devswadd(), devswdel(), lockl(),
 *	fddi_ctl_init(), config_init(), config_term(), config_qvpd()
 *
 * RETURNS: 
 *	CFG_INIT:
 *		0	- successful
 *		ENOMEM	- unable to allocate the internal space needed
 *		EBUSY	- Indicates that the device was already initialized
 *		EINVAL	- Indicates that DDS provided is invalid
 *		EINTR	- Indicates that the getting of the lock was interrupted
 *		EFAULT	- Indicates that an invalid address was specified
 *		ENODEV 	- indicates that there was no device to initialize
 *
 *	CFG_TERM:
 *		0	- successful
 *		EBUSY	- Indicates that there are outstanding opens, unable 
 *			  to terminate
 *		ENODEV	- Indicates that there was no device to terminate
 *		ENXIO	- Indicates that the device was not configured
 *		EACCESS	- Unable to remove the device from the device
 *			  switch table
 *		EINTR	- Indicates that the getting of the lock was interrupted
 *
 *	CFG_QVPD:
 *		0	- successful
 *		ENODEV	- Indicates that there was to device to query the VPD
 *		EFAULT	- Indicates that an invalid address was specified
 *		ENXIO	- Indicates that the FDDI device Driver is 
 *			  not initialized
 *		EINTR	- Indicates that the getting of the lock was interrupted
 */  

int
fddidiag_config(dev_t devno,	/* major and minor number */
	    int cmd,		/* operation (CFG_INIT, CFG_TERM, CFG_QVPD */
	    struct uio *p_uio)	/* pointer to uio structure */
{
	int adap=0, rc=0,i;
	struct devsw 	fddidevsw;

	extern int nodev();

	FDDI_TRACE("cfgB", devno, cmd, p_uio);

	/* grab driver lock */
	rc = lockl(&fddi_ctl.fddilock, LOCK_SIGRET);

	if( rc != LOCK_SUCC )
		return(EINTR);

	/* sanity check the minor number */
	if ( ((adap = minor(devno)) < 0) || (adap >= FDDI_MAX_MINOR) )
	{
		FDDI_TRACE("cfg1", adap, ENODEV, 0);
		unlockl(&fddi_ctl.fddilock);
		return(ENODEV);
	}

	switch(cmd)
	{
		case CFG_INIT:
		{

			FDDI_TRACE("cfg2", cmd, adap,  0);
			/* 
			 * first CFG_INIT must init driver
			 */
			if ( !fddi_ctl.initialized )
			{
				/* define entry points */
				fddidevsw.d_open     = (int(*)())fddi_open;
				fddidevsw.d_close    = (int(*)())fddi_close;
				fddidevsw.d_read     = (int(*)())fddi_read;
				fddidevsw.d_write    = (int(*)())fddi_write;
				fddidevsw.d_ioctl    = (int(*)())fddi_ioctl;
				fddidevsw.d_strategy = nodev;
				fddidevsw.d_ttys     = NULL;
				fddidevsw.d_select   = (int(*)())fddi_select;
				fddidevsw.d_config   = (int(*)())fddidiag_config;
				fddidevsw.d_print    = nodev;
				fddidevsw.d_dump     = nodev;
				fddidevsw.d_mpx      = (int(*)())fddi_mpx;
				fddidevsw.d_revoke   = nodev;
				fddidevsw.d_dsdptr   = NULL;
				fddidevsw.d_selptr   = NULL;
				fddidevsw.d_opts     = 0;

				/* add dev to switch table */
				rc = devswadd(devno, &fddidevsw);
				if (rc)
				{
					FDDI_TRACE("cfg3", cmd, adap,  rc);
					unlockl(&fddi_ctl.fddilock);
					return(ENOMEM);
				}

				/* init control structure */
				fddi_ctl.acs_cnt = 0;
				fddi_ctl.open_cnt = 0;
				fddi_ctl.channels = 0;
				fddi_ctl.first_open = TRUE;

				/* reset ptrs to NULL */
				for ( i=0; i < FDDI_MAX_MINOR; ++i) 	
				{
					fddi_ctl.p_acs[i] = NULL;
				}
			}

			/* try to add a DDS */
			if ((rc = fddi_config_init(devno, p_uio, adap) != 0) &&
				(!fddi_ctl.initialized))
			{
				/* undo 1st time initialization stuff */

				FDDI_TRACE("cfg4", cmd, adap,  rc);

				/* remove from switch table */
				devswdel(devno);
				break;	/* break out of CFG_INIT case */
			}

			/* if this is the first time thru the
			 * initialization, set the initialized global
			 * flag to indicate success initialization.
			 */
			if ( !fddi_ctl.initialized )
				fddi_ctl.initialized = TRUE;

		} /* end CFG_INIT */	
		break;

		case CFG_TERM:
		{
			/* we can't terminate if we were never
			 * successfully initialized
			 */
			if ( !fddi_ctl.initialized )
			{
				rc = ENXIO;
				FDDI_TRACE("cfg5", cmd, adap,  ENXIO);
				break;		/* exit case */
			}

			/* there is at least one ACS, remove the one
			 * requested.
			 */
			if ( (rc=fddi_config_term(adap)) != 0 )
			{
				FDDI_TRACE("cfg6", cmd, adap,  rc);
				break;	/* exit case CFG_TERM */
			}

			/* If this is the last minor number for the
			 * whole FDDI device driver, we need to
			 * remove ourselves from the device switch table
			 */
			if ( fddi_ctl.acs_cnt == 0)
			{
				/* there are now no more ACSs, unconfigure
				 * the device.
				 */

				/* remove from switch table */
				devswdel(devno);

				fddi_ctl.initialized = FALSE;
			}

			FDDI_TRACE("cfg7", cmd, adap,  0);
		}  /* end case CFG_TERM */
		break;

		case CFG_QVPD:
		{

			/* we can't Querry VPD if we were never
			 * successfully initialized.  The DDS tells
			 * us where the adapter and Xtender card are.
			 */
			if ( !fddi_ctl.initialized )
			{
				rc = ENXIO;
				FDDI_TRACE("cfg8", cmd, adap,  ENXIO);
				break;		
			}

			/* get the VPD for this adapter */
			rc = fddi_config_qvpd( adap, p_uio );

			FDDI_TRACE("cfg9", cmd, adap,  rc);

		} /* end CFG_QVPD */
		break;

		default:
			rc = EINVAL;

	} /* end switch (cmd) */


	/* unlockl global driver lock */
	unlockl(&fddi_ctl.fddilock);


	FDDI_TRACE("cfgE", adap, rc, 0);

	return(rc);

} /* end fddi_config() */


/*
 * NAME: fddi_config_init()
 *                                                                    
 * FUNCTION: 	Initialize a new minor number.  Check the DDS that
 *		was passed in the uio structure.  Allocates and
 *		initializes the ACS.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine executes on the process thread.
 *                                                                   
 * NOTES: 
 *	The adapter POS registers will be initialized, the VPD for the
 *	primary (and possibly extender card) will be read and santity
 *	checked.
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *	Modifies the global fddi_ctl control structure.
 *
 * ROUTINES CALLED:
 *
 * RETURNS: 
 *		0	- successful
 *		EBUSY	- device already exists
 *		EINVAL	- invalid DDS
 *		ENOMEM	- unable to malloc required memory
 *		return code from uiomove()
 */  

int
fddi_config_init( dev_t devno,	/* the device major and minor number */
	     struct uio *p_uio,	/* uio struct containing DDS */
	     int	adap)	/* adapter we are working with */
{
	fddi_dds_t	tmp_dds;
	fddi_acs_t	*p_acs;
	int		iocc;
	int		bus;
	int		i;
	int		error;
	int		badrc;

	FDDI_TRACE("cciB", devno, p_uio, adap);

	/* make sure dds not already supplied */
	if (fddi_ctl.p_acs[adap] != NULL)
	{
		FDDI_TRACE("cci1", devno, adap, fddi_ctl.p_acs[adap]);
		FDDI_TRACE("****", 0, 1, fddi_ctl.p_acs[adap]);
   		return (EBUSY);
	}
	else if (p_uio->uio_resid != sizeof(fddi_dds_t)) 
	{
		FDDI_TRACE("cci2", devno, p_uio, EINVAL);
		FDDI_TRACE("cciC", p_uio->uio_resid, sizeof(fddi_dds_t),0);
   		return (EINVAL);	/* DDS size is out of bounds */
	}
             /* get temporary copy of dds ) */
	else if ( uiomove (&tmp_dds, (int)sizeof(fddi_dds_t), 
			UIO_WRITE, p_uio) != 0 )
	{
		FDDI_TRACE("cci3", devno, p_uio, 0);
   		return (EFAULT);
	}

	/* 
	 * get ACS memory from the pinned heap, page aligned
	 */
	p_acs = xmalloc( sizeof(fddi_acs_t), FDDI_WORDSHIFT, 
		pinned_heap );	
	if ( p_acs == NULL )
	{
		FDDI_TRACE("cci4", devno, p_uio, ENOMEM);
		return(ENOMEM);
	}
	bzero(p_acs, sizeof(fddi_acs_t) );
	/*
	 * copy the tmp DDS into the ACS DDS section.
	 */
	bcopy(&tmp_dds, &p_acs->dds, sizeof(fddi_dds_t) );

	/* 
	 * set state and other initial values for this adapter structure
	 */
	p_acs->dev.state = FDDI_NULL;
	p_acs->ctl.acslock = LOCK_AVAIL;
	p_acs->ctl.first_open = TRUE;
	p_acs->ctl.devno = devno;

	/*
	 * Set SMT control word
	 */
	FDDI_TRACE("cci5", p_acs, p_acs->dev.smt_control,0);
	if (p_acs->dds.pass_bcon_frames)
		p_acs->dev.smt_control |= FDDI_SMT_CTL_BEA;
	if (p_acs->dds.pass_nsa_frames)
		p_acs->dev.smt_control |= FDDI_SMT_CTL_NSA;
	if (p_acs->dds.pass_smt_frames)
		p_acs->dev.smt_control |= FDDI_SMT_CTL_SMT;
	FDDI_TRACE("cci6", p_acs, p_acs->dev.smt_control,0);
				  
	/* 
	 * set in POS what was just given to us
	 * in the DDS.
	 */
	if (cfg_pos_regs(p_acs))	/* init POS regs */
	{
		FDDI_TRACE("cci7", devno, p_uio, EINVAL);
		xmfree( p_acs, pinned_heap );	/* free ACS struct */
		return(EINVAL);
	}

	get_vpd(p_acs);	/* get the VPD from adap */

	if (p_acs->vpd.status != FDDI_VPD_VALID)
	{
		FDDI_TRACE("cci8", devno, p_uio, p_acs->vpd.status);
		xmfree( p_acs, pinned_heap );	/* free ACS struct */
		return(EINVAL);
	}

	fddi_get_stest(p_acs);

	error=FALSE;
	for (i=0;i<8;++i)
	{
		if (p_acs->dev.stestrc[i] !=0)
		{
			error=TRUE;
			badrc = p_acs->dev.stestrc[i];
		}
	}
	if ( (p_acs->dev.stestrc[8] != 0x0000) && 
		(p_acs->dev.stestrc[8] != 0x80ea) )
	{
		error=TRUE;
		badrc = p_acs->dev.stestrc[8];
	}
	
	if ( (p_acs->dev.stestrc[9] != 0x0000) &&
		 (p_acs->dev.stestrc[9] != 0x9001) )
	{
		error=TRUE;
		badrc = p_acs->dev.stestrc[9];
	}

	if ((p_acs->ctl.card_type == FDDI_SC) && 
		(p_acs->dev.stestrc[10] != 0x0000)  &&
		(p_acs->dev.stestrc[10] != 0xA0EA))
	{
		error=TRUE;
		badrc = p_acs->dev.stestrc[10];
	} /* self tests failed */

	if (error)
	{
		FDDI_TRACE("cci9", badrc, 0, 0);
		p_acs->dev.stest_cover = TRUE;
		fddi_logerr(p_acs, ERRID_FDDI_SELFT_ERR,__LINE__,__FILE__);
		xmfree( p_acs, pinned_heap );	/* free ACS struct */
		return(EFAULT);
	} /* self tests failed */

	p_acs->dev.stest_cover = FALSE;
	p_acs->vpd.xc_status = FDDI_VPD_NOT_READ;

	if (p_acs->dev.stestrc[8] == 0x0000)
	{
		if (p_acs->ctl.card_type == FDDI_FR)
			get_fr_xcard_vpd(p_acs);
		else
			if (p_acs->ctl.card_type == FDDI_SC)
				get_sc_xcard_vpd(p_acs);
			else 
				return(EBUSY);

		p_acs->vpd.xc_status = FDDI_VPD_VALID;
		p_acs->dev.attach_class = FDDI_ACLASS_DAS;
	}
	else 
	{
		p_acs->vpd.xc_status = FDDI_VPD_NOT_READ;
		p_acs->dev.attach_class = FDDI_ACLASS_SAS;
	}

	/* 
	 * set the MAC SMT addr
	 */
	if (p_acs->dds.use_alt_mac_smt_addr)
	{
		bcopy (p_acs->dds.alt_mac_smt_addr, 
			p_acs->ctl.long_src_addr, FDDI_NADR_LENGTH);
	}
	else
	{
		bcopy (p_acs->ctl.vpd_addr, 
			p_acs->ctl.long_src_addr, FDDI_NADR_LENGTH);
	}

	fddi_ctl.acs_cnt++;
	fddi_ctl.p_acs[adap] = p_acs;

	FDDI_TRACE("cciE", p_acs, 0, 0);
	return(0);

} /* end fddi_config_init() */

/*
 * NAME: fddi_config_term()
 *                                                                    
 * FUNCTION: Terminate a minor number.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *	This routine will free the ACS for the device if there are
 *	no outstanding opens for the device.
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *	Modifies the global fddi_ctl control structure.
 *
 * ROUTINES CALLED:
 *	lockl(), xmfree(), unlockl()
 *
 * RETURNS: 
 *		0	- successful
 *		ENOENT	- no device to terminate
 *		EBUSY 	- device busy, unable to terminate
 *		
 */  

int
fddi_config_term( int adap) 	/* the device minor number */
{
	fddi_acs_t	*p_acs;
	int	iocc;

	FDDI_TRACE("cctB", adap, 0, 0);
	p_acs = fddi_ctl.p_acs[adap];

	if ( p_acs == NULL )
	{
		FDDI_TRACE("cct1", adap, ENODEV, 0);
		return(ENODEV);
	}

	if ( p_acs->ctl.open_cnt != 0)
	{
		FDDI_TRACE("cct2", p_acs, p_acs->ctl.open_cnt, EBUSY);
		return(EBUSY);
	}

	fddi_ctl.p_acs[adap] = NULL;	/* reset ACS ptr */
	fddi_ctl.acs_cnt--;

	/*
	 * Write out pos 2 with the Adapter Reset (AR) and 
	 * Card Enable (CEN) bits set.
	 */

	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC+(p_acs->dds.slot<<16)) );
	BUS_PUTCX( (iocc+FDDI_POS_REG2), (p_acs->dev.pos2 | FDDI_POS2_AR  
					| FDDI_POS2_CEN) );
	IOCC_DET(iocc);
	xmfree( p_acs, pinned_heap );	/* free ACS struct */

	delay(10*HZ);

	FDDI_TRACE("cctE", adap, fddi_ctl.acs_cnt, 0);
	return(0);
} /* end fddi_config_term() */

/*
 * NAME: fddi_config_qvpd()
 *                                                                    
 * FUNCTION: Query the VPD for a minor number.  
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread.
 *                                                                   
 * NOTES: 
 *	This routine will copy the VPD for the device into the
 *	provided uio buffer.
 *	
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *
 * ROUTINES CALLED:
 *
 * RETURNS: 
 *		0	- successful
 *		ENOENT	- no device to query VPD
 *		rc from copyout()
 */  

int
fddi_config_qvpd(	int adap,		/* the device minor number */
			struct uio *p_uio) 	/* uio struct for VPD */
{
	fddi_acs_t	*p_acs;
	int rc;

	/* 
	 * The sanity check of the adap parameter was
	 * done at the beginning of the fddi_config() routine.
	 */
	FDDI_TRACE("cqvB", adap, p_uio, 0);

	p_acs = fddi_ctl.p_acs[adap];

	if(p_acs == NULL)
	{
		FDDI_TRACE("cqv1", adap, p_uio, ENODEV);
		return(ENODEV);
	}
	/* move out the VPD section  */
	if ( (rc = uiomove( &(p_acs->vpd), (int)MIN(p_uio->uio_resid, 
			sizeof(fddi_vpd_t)),
			UIO_READ, p_uio)) != 0 ) 
	{
		FDDI_TRACE("cqv2", p_acs, p_uio, rc);
		return(EFAULT);
	}

	FDDI_TRACE("cqvE", adap, p_acs, p_uio);
	return(0);
} /* end fddi_config_qvpd() */


/*
 * NAME: cfg_pos_regs()
 *                                                                    
 * FUNCTION: Configure the POS registers
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *	This routine decodes the configuration values in the DDS
 *	and sets the POS registers accordingly.
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *	Modifies the ACS pos register settings.
 *
 * ROUTINES CALLED:
 *
 *
 * RETURNS: 
 *		0	- successful
 *		
 */  

int 
cfg_pos_regs( fddi_acs_t *p_acs )
{
	int	acc;		/* for access to IOCC to modify POS regs */

	FDDI_TRACE("cprB", p_acs, 0, 0);
	/*
	 * POS Register 2
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * |   Interrupt (INTR)    | DD  | not | AR  | CEN |
	 * |              Level    |     | used |    |     |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *   INTR = Interrupt Level.  These bits contain the binary 
	 *	representation of the adapter card interrupt level.  
	 *
	 *   DD = Download/Diagnostic bit.  When set (1), this bit places the
	 *	adapter in to Download/Diagnostic state.  in this state, the 
	 *	ALISA chip is the master of the Node Processor bus and the 
	 *	Host has access to the ALISA Instruction Cmd registers.  The 
	 *	Node Processor is placed in a bus hold condition.
	 *
	 *   AR = Adapter Reset.   When this bit is written to a one (1), an
	 *	adapter reset pulse is will be generated.  All ALISA regs will
	 *	return to their default settings.  This bit will will always
	 *	read as a one (1).  A write of zero (0) has no effect.
	 *
	 *   CEN = Card Enable Bit.  When this bit is cleared (0), all ALISA
	 *	MC drivers will be tri-stated during normal MC cycles.  The
	 *	When programmed high (1), the adapter will support all MC
	 *	activity.  This bit will default to to a cleared (0) condition
	 *	after the occurance of a channel reset.
	 */


	p_acs->dev.pos2 = 0;
	/*
	 * These are NOOPs but here to reflect intentions
	 */
	p_acs->dev.pos2 &= ~(FDDI_POS2_CEN);	/* disable the card */
	p_acs->dev.pos2 &= ~(FDDI_POS2_DD);	/* put adapter in normal op */
	p_acs->dev.pos2 &= ~(FDDI_POS2_AR);	/* do not reset adapter */

	/*
	 * decode and set the interrupt level 
	 */
	switch (p_acs->dds.bus_intr_lvl)
	{
		case (FDDI_INTR_LVL_9):		
			p_acs->dev.pos2 |=0xe0;	
			break;
		case (FDDI_INTR_LVL_10):
			p_acs->dev.pos2 |=0xd0;	
			break;
		case (FDDI_INTR_LVL_14):
			p_acs->dev.pos2 |=0xb0;	
			break;
		case (FDDI_INTR_LVL_15):
			p_acs->dev.pos2 |=0x70;	
			break;
		default :
			FDDI_TRACE("cpr1", p_acs->dds.bus_intr_lvl, 0, 0);
			return(EINVAL);

	} /* end bus_intr_lvl switch */

	/*
	 * POS Register 3
	 *
	 *	Non-extension Mode
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * | SDE | MSE | not |  F  | Arbitration  (ARB)    |
	 * |     |     | used|     |              Level    |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *   SDE = Streaming Data Enable.  If set (1), then
	 *	the MC Streaming Data mode is disabled.  If reset (0),
	 *	then the Streaming Data mode is enabled using 10Mhz
	 *	(100nSec).  The default is set.
	 *
	 *   MSE = Streaming Data Enable.  If set (1), then 64bit
	 *	MC Streaming Data will not be attempted regardless of the 
	 *	MSDR indication.  If reset (0), then the 64 bit Streaming Data
	 *	mode is enabled.  The default value is set.
	 *
	 *   F = Fairness Bit.  If set (1), then the MC fairness feature
	 *	is enabled for Burst mode and Nibble mode transfers.  If 
	 *	reset (0), then fairness is not supported.  The default 
	 *	condition is set (1).
	 *
	 *   ARB = Arbitration Level.  These bits define the adapter's
	 *	MC arbitration level.
	 *
	 *
	 * 	Extension Mode
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * |  d  |  d  |  d  |  d  |  d  |  d  |  d  |  d  |
	 * |     |     |     |     |     |     |     |     |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *	d = Data interface with the POS extension register or mem.
	 *
	 */
	p_acs->dev.pos3 = 0;
	p_acs->dev.pos3 &= ~(FDDI_POS3_SDE);
	p_acs->dev.pos3 &= ~(FDDI_POS3_MSE);
	p_acs->dev.pos3 |= FDDI_POS3_FAIR;
	p_acs->dev.pos3 |= (p_acs->dds.dma_lvl & FDDI_POS3_ARB_MASK);

	/*
	 * POS Register 4
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * | not | not | NSE | AIM |    ABM    |   P SEL   |
	 * | used| used|     |     |           |           |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *   NSE = No Selected Feedback interrupt enable.  If set (1),
	 *	then ALISA will automatically set the NSF bit in SIF 
	 *	registers HSR and NS2 when a bus master operation has been
	 *	attempted with no SFDBK RTN or CHCK returned by the selected
	 *	slave.  If cleared (0), then the HSR and NS2 bits will not
	 *	be set by ALISA (default setting).
	 *
	 *   AIM = Auto Increment Mode for POS extensions.  If set (1),
	 *	then the index values in POS register 6 will be auto 
	 *	incremented after each POS extension access.  The default
	 *	is reset (0).
	 *
	 *   ABM = Address Burst Managment field.  This field will be used
	 *	to minimize the number of bus arbitration cycles.  Enabling 
	 *	ABM will cause the adapter to terminate a bus transfer when 
	 *	an address boundary crossing occurs.  ABM shall be controlled
	 *	by two bits in POS 4 as follows:
	 *
	 *		00 - ABM disabled (terminate transfer on bus
	 *			interface FIFO empty or preempt)
	 *		01 - 16 byte address boundary crossing
	 *		10 - 64 byte address boundary crossing
	 *		11 - 256 byte address boundary crossing
	 *
	 *   P SEL = Parity Select Field.  These bits enable and disable
	 *	the parity checking capability of ALISA.  The decode is
	 *	as follows:
	 *
	 *		00 - Parity is enabled on the MC data bus and the
	 *			MC address bus.
	 *		x1 - Parity is disabled for the MC data bus (default).
	 *		1x - Parity is disabled for the MC addr bus (default).
	 *
	 */

	p_acs->dev.pos4 = 0;
	p_acs->dev.pos4 |= FDDI_POS4_NSE;
	p_acs->dev.pos4 |= FDDI_POS4_AIM;
	p_acs->dev.pos4 |= FDDI_POS4_ABM;
	p_acs->dev.pos4 |= FDDI_POS4_PSEL;

	/*
	 * POS Register 5
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * | CHK | STA |       IAF       |       MAF       |
	 * |     |     | I/O adap addr   | Mem adap addr   |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *   CHK = Channel check indicator.  When set (1), the Micro
	 *	channel check signal (CHCK) has not been driven by the
	 *	adapter.  When reset (0), the CHCK signal has been drivn by
	 *	the adapter and a MC address bus parity error 
	 *
	 *   STA = Status Field.  When set high (1), indicates that no 
	 *	status is reported for the encountered channel check 
	 *	exception condition.  When set low (0), this bit indicates 
	 *	that channel check status is reported via bytes XXX6 and XXX7.
	 *
	 *   IAF - I/O adapter address field.  This field will alter the 
	 *	MC I/O memory address for the ALISA system interface registers.
	 *	The default value is 0.  Possible I/O address are:
	 *
	 *		000 = 0x7140 
	 *		001 = 0x7150 
	 *		010 = 0x7540
	 *		011 = 0x7550
	 *		100 = 0x7940
	 *		101 = 0x7950
	 *		110 = 0x7d40
	 *		111 = 0x7d50
	 *		
	 *
	 *   MAF = Memory adapter address field.  This field will alter the
	 *	MC memory address for the the ALISA shared RAM buffer.
	 *	The default value is 0.  Possible RAM address are:
	 *
	 *		000 = 0x00100000
	 *		001 = 0x00300000
	 *		010 = 0x00500000
	 *		011 = 0x00700000
	 *		100 = 0x00900000
	 *		101 = 0x00b00000
	 *		110 = 0x00d00000
	 *		111 = 0x00f00000
	 *
	 */


	p_acs->dev.pos5 = FDDI_POS5_CHK | FDDI_POS5_STAT;

	switch ( (int)p_acs->dds.bus_mem_addr )
	{
		case FDDI_POS5_MEM_1:
			p_acs->dev.pos5 |= FDDI_POS5_MAF_000;
			break;
		case FDDI_POS5_MEM_3:
			p_acs->dev.pos5 |= FDDI_POS5_MAF_001;
			break;
		case FDDI_POS5_MEM_5:
			p_acs->dev.pos5 |= FDDI_POS5_MAF_010;
			break;
		case FDDI_POS5_MEM_7:
			p_acs->dev.pos5 |= FDDI_POS5_MAF_011;
			break;
		case FDDI_POS5_MEM_9:
			p_acs->dev.pos5 |= FDDI_POS5_MAF_100;
			break;
		case FDDI_POS5_MEM_b:
			p_acs->dev.pos5 |= FDDI_POS5_MAF_101;
			break;
		case FDDI_POS5_MEM_d:
			p_acs->dev.pos5 |= FDDI_POS5_MAF_110;
			break;
		case FDDI_POS5_MEM_f:
			p_acs->dev.pos5 |= FDDI_POS5_MAF_111;
			break;
		default :
			FDDI_TRACE("cpr2", p_acs->dds.bus_mem_addr, 0, 0);
			return(EINVAL);
	} /* end switch on bus_mem_addr */

	switch ( (int)p_acs->dds.bus_io_addr )
	{
		case FDDI_POS5_IO_7140:
			p_acs->dev.pos5 |= FDDI_POS5_IAF_000;
			break;
		case FDDI_POS5_IO_7150:
			p_acs->dev.pos5 |= FDDI_POS5_IAF_001;
			break;
		case FDDI_POS5_IO_7540:
			p_acs->dev.pos5 |= FDDI_POS5_IAF_010;
			break;
		case FDDI_POS5_IO_7550:
			p_acs->dev.pos5 |= FDDI_POS5_IAF_011;
			break;
		case FDDI_POS5_IO_7940:
			p_acs->dev.pos5 |= FDDI_POS5_IAF_100;
			break;
		case FDDI_POS5_IO_7950:
			p_acs->dev.pos5 |= FDDI_POS5_IAF_101;
			break;
		case FDDI_POS5_IO_7d40:
			p_acs->dev.pos5 |= FDDI_POS5_IAF_110;
			break;
		case FDDI_POS5_IO_7d50:
			p_acs->dev.pos5 |= FDDI_POS5_IAF_111;
			break;
		default :
			FDDI_TRACE("cpr3", p_acs->dds.bus_io_addr, 0, 0);
			return(EINVAL);
	} /* end switch on bus_io_addr */

	/*
	 * POS Register 6
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * |  a  |  a  |  a  |  a  |  a  |  a  |  a  |  a  |
	 * |     |     |     |     |     |     |     |     |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *    a = Address field.  The address field will be the index into
	 *	the VPD ROM, depending on the decode of POS register 7 
	 *	D SEL field.  The default value of POS register 7 is 0.
	 *
	 */
	/*
	 * save the POS register 6 configuration value
	 */
	p_acs->dev.pos6 = 0;

	/*
	 * POS Register 7
	 *
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 * | MDP | not | not | not | VPS |     D SEL       |
	 * |     | used| used| used|     |                 |
	 * +-----+-----+-----+-----+-----+-----+-----+-----+
	 *    7     6     5     4     3     2     1     0
	 *
	 *   MDP = Micro Channel Data Bus Parity Error.  This bit is
	 *	set (1) when a parity error is detected on the MC data bus.
	 *	This bit can be cleared by a write to this register by the
	 *	Host processor.  The Node processor does not have write access.
	 *
	 *   VPS = Vital Products ROM Select. When this bit is set (1), then
	 *	the D SEL field can be used to select the VPD ROM on the
	 *	Class A extender card.  When this bit is cleared (0), then the
	 *	D SEL field can be used to select the VPD ROM on the base
	 *	adapter card.  A value of 000 in the D SEL field will select
	 *	a VPD ROM.
	 *
	 *   D SEL = Device Select bit.
	 *		000 - VPD ROM
	 *		001 - I/O device address field low (bits 1-8)
	 *		010 - I/O device address field high (bits 0)
	 *		011 - Memory device address field 1 (bits 9-16)
	 *		100 - Memory device address field 2 (bits 1-8)
	 *		101 - Memory device address field 3 (bits 0)
	 *		110 - reserved
	 *		111 - reserved
	 */
	/*
	 * save the POS register 7 configuration value
	 */
	p_acs->dev.pos7 = 0;

	/*
	 * write out the POS register settings.
	 */
	acc = IOCC_ATT( p_acs->dds.bus_id, (IO_IOCC + (p_acs->dds.slot<<16)));
	BUS_PUTCX( (acc + FDDI_POS_REG6), p_acs->dev.pos6 );
	BUS_PUTCX( (acc + FDDI_POS_REG7), p_acs->dev.pos7 );
	BUS_PUTCX( (acc + FDDI_POS_REG5), p_acs->dev.pos5 );
	BUS_PUTCX( (acc + FDDI_POS_REG4), p_acs->dev.pos4 );
	BUS_PUTCX( (acc + FDDI_POS_REG3), p_acs->dev.pos3 );
	BUS_PUTCX( (acc + FDDI_POS_REG2), p_acs->dev.pos2 );
	IOCC_DET(acc); 

	FDDI_TRACE("cprE", p_acs, 0, 0);
	return(0);

} /* end cfg_pos_regs */

/*
 * NAME: get_vpd()
 *                                                                    
 * FUNCTION: Configure the POS registers
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *	This routine reads in the VPD for the primary adapter 
 *
 *	The VPD is crc checked. 
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *	Modifies the VPD section of the ACS
 *
 * ROUTINES CALLED:
 *
 *
 * RETURNS: 
 *	0		- successful
 *	ENETDOWN	- unsuccessful
 *		
 */  

int
get_vpd( fddi_acs_t	*p_acs )
{
	uint 		iocc;
	int		i,j;
	fddi_vpd_t	*p_vpd;
	int 		vpd_found = FALSE;
	int 		na_found = FALSE;
	int 		crc_valid = FALSE;
	int 		fn_found = FALSE;
	ushort 		cal_crc;
	ushort 		vpd_crc;
	uchar		tmp1, tmp2;

	/*
	 * get access to the IOCC.  
	 */



	FDDI_TRACE("cgvB", p_acs, 0, 0);
	
	iocc = IOCC_ATT( p_acs->dds.bus_id, (IO_IOCC + (p_acs->dds.slot<<16)));

	BUS_GETCX( (iocc + FDDI_POS_REG0) ,&tmp1 );
	BUS_GETCX( (iocc + FDDI_POS_REG1) ,&tmp2 );

	if ( (tmp1 != FDDI_POS_REG0_VAL) ||
		(tmp2 != FDDI_POS_REG1_VAL) )
	{
		/* 
		 * not a valid card 
		 * we don't read the VPD.
		 */
		p_acs->vpd.status=FDDI_VPD_NOT_READ;
	}
	else
	{
		p_vpd = &(p_acs->vpd);

		/* Initialize POS reg 6 and 7 */
		BUS_PUTCX( (iocc + FDDI_POS_REG7), p_acs->dev.pos7 );
		BUS_PUTCX( (iocc + FDDI_POS_REG6), p_acs->dev.pos6 );

		for ( i = 0; i < FDDI_VPD_LENGTH; ++i )
		{
			/*
			 * set up the correct addr for the VPD read byte 
			 */
			tmp1=(uchar)i+1;
			BUS_PUTCX( (iocc + FDDI_POS_REG6), tmp1 );

			BUS_GETCX( (iocc + FDDI_POS_REG3), &p_vpd->vpd[i] );
			for (j=0; j<7; ++j)
			{
				BUS_GETCX( (iocc + FDDI_POS_REG3), &tmp2 );
				if ( p_vpd->vpd[i] == tmp2 )
					break;
				else
					p_vpd->vpd[i] = tmp2;
			}

		} 

		p_vpd->vpd[FDDI_VPD_LENGTH - 1] = 0x00;

		/* Initialize POS reg 6 and 7 */
		BUS_PUTCX( (iocc + FDDI_POS_REG7), p_acs->dev.pos7 );
		BUS_PUTCX( (iocc + FDDI_POS_REG6), p_acs->dev.pos6 );

		/* 
		 * Test the VPD fields.
		 */

		if ( (p_vpd->vpd[0] == 'V') &&
		     (p_vpd->vpd[1] == 'P') &&
		     (p_vpd->vpd[2] == 'D') &&
		     (p_vpd->vpd[7] == '*'))
		{
			vpd_found = TRUE;
			/* 
			 * calculate the VPD length
			 */
			p_vpd->l_vpd = ( (2 * ((p_vpd->vpd[3] << 8) |
						   p_vpd->vpd[4])) + 7);


			if (p_vpd->l_vpd > FDDI_VPD_LENGTH)
			{
				p_vpd->l_vpd = FDDI_VPD_LENGTH;
				/*
				 * mismatch on the length - can not test crc
				 * assume good crc
				 */
				crc_valid = TRUE;

			}
			else
			{
				vpd_crc = ( (p_vpd->vpd[5] << 8) |
						p_vpd->vpd[6]);

				/*
				 * verify that the crc is valid
				 */

				cal_crc = fddi_gen_crc( &(p_vpd->vpd[7]), 
						((ushort)p_vpd->l_vpd -7) );

				if ( vpd_crc == cal_crc )
					crc_valid = TRUE;
			} 

			/*
			 * get the network address 
			 */
			for ( i=0; i < p_vpd->l_vpd; ++ i)
			{
				/* test for the Network Address header */
				if ( (p_vpd->vpd[i+0] == '*' ) && 
				     (p_vpd->vpd[i+1] == 'N' ) &&
				     (p_vpd->vpd[i+2] == 'A' ) &&
				     (p_vpd->vpd[i+3] == 5) )
				{
					na_found = TRUE;
					/* save the net address */
					for (j=0; j < FDDI_NADR_LENGTH; ++j)
						p_acs->ctl.vpd_addr[j]=
							p_vpd->vpd[i+4+j];

					i += j-1;
				} /* end test for net addr */
				else 
				{
					if ((p_vpd->vpd[i] == 'F') &&
						(p_vpd->vpd[i+1] == 'N'))
					{
						if (!strncmp(&p_vpd->vpd[i+4],
							FDDI_FR_FRU1, 
							FDDI_FN_LEN))
						{
							p_acs->ctl.card_type =	
								FDDI_FR;
							fn_found = TRUE;
						}
						else
						if (!strncmp(&p_vpd->vpd[i+4],
							FDDI_FR_FRU2, 
							FDDI_FN_LEN))
						{
							p_acs->ctl.card_type =	
								FDDI_FR;
							fn_found = TRUE;
						}
						else 
						if ((!strncmp(&p_vpd->vpd[i+4],
							FDDI_SC_FRU, 
							FDDI_FN_LEN)) ||
						(!strncmp(&p_vpd->vpd[i+4],
							FDDI_FC_FRU, 
							FDDI_FN_LEN)))
						{
							p_acs->ctl.card_type =	
								FDDI_SC;
							fn_found = TRUE;
						}
					}

				}
			} /* end for loop search for net addr */
		}

		if ( vpd_found && na_found && crc_valid && fn_found)
			p_vpd->status = FDDI_VPD_VALID;
		else
			p_vpd->status = FDDI_VPD_INVALID;
	} /* end else VPD read */

	IOCC_DET(iocc);

	/*
	 * NB:
	 *	At this point we need to check if there is a valid
	 *	long network address for us to set in the FORMAC for
	 *	adapter activation.  If there is no valid address for
	 *	us to use, we need to set the control flag telling us
	 *	to fail all CIO_STARTs due to a configuration problem.
	 *	We will not attempt to activate the adapter on the
	 *	1st CIO_START.
	 *
	 *	We want the device to be configured because we want
	 *	to be able to run diagnostics on the adapter if needed.
	 */
	if ( (na_found != TRUE)  &&
		(p_acs->dds.use_alt_mac_smt_addr != TRUE) )
	{
		/* !!! set the control flag telling us to fail starts */	
	}

	FDDI_TRACE("cgvE", vpd_found, na_found, crc_valid);
	return(0);
} /* end get_vpd() */

/*
 * NAME: get_sc_xcard_vpd()
 *                                                                    
 * FUNCTION: get the extender card's vpd for the scarborough type of card
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *	Modifies the VPD section of the ACS
 *
 * ROUTINES CALLED:
 *
 *
 * RETURNS: 
 *	0		- successful
 *	ENETDOWN	- unsuccessful
 *		
 */  


int
get_sc_xcard_vpd( fddi_acs_t	*p_acs )
{
	uint 		iocc;
	int		i,j;
	fddi_vpd_t	*p_vpd;
	ushort 		cal_crc;
	ushort 		vpd_crc;
	uchar		tmp1, tmp2;

	/*
	 * get access to the IOCC.  
	 */
	FDDI_TRACE("csxB", p_acs, 0, 0);
	
	iocc = IOCC_ATT( p_acs->dds.bus_id, (IO_IOCC + (p_acs->dds.slot<<16)));

	p_vpd = &(p_acs->vpd);

	/* Initialize POS reg 6 and 7 */
	BUS_PUTCX( (iocc + FDDI_POS_REG7), p_acs->dev.pos7 | FDDI_POS7_VPS );
	BUS_PUTCX( (iocc + FDDI_POS_REG6), p_acs->dev.pos6 );

	for ( i = 0; i < FDDI_VPD_LENGTH; ++i )
	{
		/*
		 * set up the correct addr for the VPD read byte 
		 */
		tmp1=(uchar)i+1;
		BUS_PUTCX( (iocc + FDDI_POS_REG6), tmp1 );

		BUS_GETCX( (iocc + FDDI_POS_REG3), &p_vpd->xcvpd[i] );
		for (j=0; j<7; ++j)
		{
			BUS_GETCX( (iocc + FDDI_POS_REG3), &tmp2 );
			if ( p_vpd->xcvpd[i] == tmp2 )
				break;
			else
				p_vpd->xcvpd[i] = tmp2;
		}

	} 

	p_vpd->xcvpd[FDDI_VPD_LENGTH - 1] = 0x00;
	p_vpd->l_xcvpd = FDDI_XCVPD_LENGTH;

	/* Initialize POS reg 6 and 7 */
	BUS_PUTCX( (iocc + FDDI_POS_REG7), p_acs->dev.pos7 );
	BUS_PUTCX( (iocc + FDDI_POS_REG6), p_acs->dev.pos6 );

	IOCC_DET(iocc);

	FDDI_TRACE("csxE", 0,0,0);
	return(0);
} /* end get_sc_xcard_vpd() */

/*
 * NAME: get_fr_xcard_vpd()
 *                                                                    
 * FUNCTION: if there is an extender card then read the vpd off of it (it is 
 *  not interpreted only read.  If there is not an extender card mark the xcard_
 *  vpd as NOT_READ.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *	This routine reads in the VPD for the extender adapter 
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *	Modifies the VPD section of the ACS
 *
 * ROUTINES CALLED:
 *
 *
 * RETURNS: 
 *	0		- successful
 *	ENETDOWN	- unsuccessful
 *		
 */  

int
get_fr_xcard_vpd( fddi_acs_t	*p_acs )
{
	uint 		iocc;
	int		i,j;
	fddi_vpd_t	*p_vpd;
	uchar		tmp1, tmp2;

	/*
	 * get ioccess to the IOCC.  
	 */

	FDDI_TRACE("cfxB", p_acs, 0, 0);
	
	iocc = IOCC_ATT( p_acs->dds.bus_id,(IO_IOCC+((p_acs->dds.slot-1)<<16)));

	BUS_GETCX( (iocc + FDDI_POS_REG0) ,&tmp1 );
	BUS_GETCX( (iocc + FDDI_POS_REG1) ,&tmp2 );

	p_vpd = &(p_acs->vpd);

	/* Initialize POS reg 6 and 7 */
	BUS_PUTCX( (iocc + FDDI_POS_REG7), p_acs->dev.pos7 );
	BUS_PUTCX( (iocc + FDDI_POS_REG6), p_acs->dev.pos6 );

	for ( i = 0; i < FDDI_XCVPD_LENGTH; ++i )
	{
		/*
		 * set up the correct addr for the VPD read byte 
		 */
		tmp1=(uchar)i+1;
		BUS_PUTCX( (iocc + FDDI_POS_REG6), tmp1 );

		BUS_GETCX( (iocc + FDDI_POS_REG3), &p_vpd->xcvpd[i] );
		for (j=0; j<7; ++j)
		{
			BUS_GETCX( (iocc + FDDI_POS_REG3), &tmp2 );
			if ( p_vpd->xcvpd[i] == tmp2 )
				break;
			else
				p_vpd->xcvpd[i] = tmp2;
		}
	} 

	p_vpd->xcvpd[FDDI_XCVPD_LENGTH - 1] = 0x00;
	p_vpd->l_xcvpd = FDDI_XCVPD_LENGTH;

	/* Initialize POS reg 6 and 7 */
	BUS_PUTCX( (iocc + FDDI_POS_REG7), p_acs->dev.pos7 );
	BUS_PUTCX( (iocc + FDDI_POS_REG6), p_acs->dev.pos6 );

	IOCC_DET(iocc);
	FDDI_TRACE("cfxE", 0, 0, 0);
	return(0);
} /* end get_fr_xcard_vpd() */

/*
 * NAME: fddi_gen_crc()
 *                                                                    
 * FUNCTION: Generate a 16-bit CRC value
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine executes on the process thread. 
 *                                                                   
 * NOTES: 
 *	This routine calculates a 16-bit crc value used to check
 *	the validity of VPD.
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *
 * ROUTINES CALLED:
 *
 *
 * RETURNS: 
 *		A 16-bit crc value per specs for MCA bus VPD area CRC.
 *		
 */  

ushort
fddi_gen_crc(	register uchar 	*p_buf,
		register int	l_buf)
{
	register uchar	work_msb;
	register uchar	work_lsb;
	register uchar	value_msb;
	register uchar	value_lsb;
	ushort rc;

	FDDI_TRACE("cgcB", p_buf, l_buf, 0);

	for (value_msb = 0xff, value_lsb = 0xff; l_buf > 0; l_buf-- )
	{
		value_lsb ^= *p_buf++;
		value_lsb ^= (value_lsb << 4);

		work_msb = value_lsb >> 1;
		work_lsb = (value_lsb << 7) ^ value_lsb;

		work_lsb = ( work_msb << 4 ) | (work_lsb >> 4);
		work_msb = ((work_msb >> 4) & 0x07) ^ value_lsb;

		value_lsb = work_lsb ^ value_msb;
		value_msb = work_msb;
	} 

	rc = ((ushort)value_msb << 8) | value_lsb;
	FDDI_TRACE("cgcE", p_buf, l_buf, rc);
	return(rc);

} /* end fddi_gen_crc() */

/*
 * externs for FDDI component dump table objects
 */
extern struct cdt	*p_fddi_cdt;
extern int		l_fddi_cdt;

/*
 * NAME: fddi_cdt_init
 *                                                                    
 * FUNCTION: initialize component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	0 	if successful
 *	ENOMEN 	if no memory available
 */  

int
fddi_cdt_init ()
{
	/* 
	 * allocate dump table with initial size
	 */
	p_fddi_cdt = (struct cdt *) xmalloc(sizeof(struct cdt_head) + 
		(FDDI_CDT_ENTRIES * sizeof(struct cdt_entry)), 
		FDDI_WORDSHIFT, pinned_heap);
	if (p_fddi_cdt == NULL)
	{
		/* fail */
		return (ENOMEM);
	}
	/* 
	 * initialize component dump table 
	 * l_fddi_cdt is the total size of the allocated dump table
	 * cdt_len is the current size of the actual entries in the sump table
	 */
	l_fddi_cdt = FDDI_CDT_ENTRIES;
	p_fddi_cdt->cdt_magic = DMP_MAGIC;
	bcopy (FDDI_DD_STR, p_fddi_cdt->cdt_name, sizeof(FDDI_DD_STR));
	p_fddi_cdt->cdt_len = sizeof (struct cdt_head);

	/* 
	 * add static data structures: trace table and fddi control struct
	 *	also, add the allocated open table at this time.
	 */
	fddi_cdt_add ("fdditrac", (char *) &fdditrace, sizeof (fdditrace));
	fddi_cdt_add ("fddi_ctl", (char *) &fddi_ctl,  sizeof (fddi_ctl));
	fddi_cdt_add ("Opentab", 
			(char *) fddi_ctl.p_open_tab,  
			fddi_ctl.open_tab_size * sizeof (fddi_open_t *));

	/* 
	 * register for dump 
	 */
	dmp_add (((void(*)())fddi_cdt_func));

	/* ok */
	return (0);
}

/*
 * NAME: fddi_cdt_undo_init
 *                                                                    
 * FUNCTION: remove the component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: none
 */  

int
fddi_cdt_undo_init ()
{
	/* Delete global structures */
	fddi_cdt_del ((char *) &fdditrace);
	fddi_cdt_del ((char *) &fddi_ctl);
	fddi_cdt_del ((char *) fddi_ctl.p_open_tab);

	/* 
	 * If our number of entries is not zero we have an internal 
	 * programming error 
	 */
	ASSERT(NUM_ENTRIES(p_fddi_cdt) == 0);

	dmp_del (((void(*)())fddi_cdt_func));
	xmfree(p_fddi_cdt, pinned_heap);
	p_fddi_cdt = NULL;
	l_fddi_cdt = 0;
	
	return(0);
}

/*
 * NAME: fddi_cdt_add
 *                                                                    
 * FUNCTION: add an entry to the component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: none
 */  

void
fddi_cdt_add (
	char	*p_name,
	char	*ptr,
	int	len)
{
	int		rc;
	int		num_entries;
	int		new_size;
	struct cdt	*p_old;
	struct cdt	*p_new;

	FDDI_DBTRACE ("dadB", p_fddi_cdt, p_fddi_cdt->cdt_len, ptr);
	 
	/* get number of entries */
	num_entries = NUM_ENTRIES (p_fddi_cdt);

	if (num_entries == l_fddi_cdt)
	{
		/* 
		 * enlarge the entries by FDDI_CDT_ENTRIES
		 */
		new_size = sizeof (struct cdt_head) + 
			((l_fddi_cdt + FDDI_CDT_ENTRIES) * 
			sizeof (struct cdt_entry));

		/* grow dump table */
		p_new = (struct cdt *) xmalloc (new_size, FDDI_WORDSHIFT, 
						pinned_heap);

		if (p_new == NULL)
		{
			/* 
			 * can't expand dump table drop entry
			 */
			FDDI_DBTRACE ("dad1", p_fddi_cdt, 0, ptr);
			return;
		}
		/* 
		 * initialize new dump table with old values
		 * (cdt_len is copied as well)
		 */
		bcopy (p_fddi_cdt, p_new, p_fddi_cdt->cdt_len);

		/* 
		 * swap curent (old) dump table with new 
		 * 	Notice p_fddi_cdt will always have a valid 
 		 *	pointer to avoid errors if the dump function
		 *	were called during the swap
		 */
		p_old = p_fddi_cdt;
		p_fddi_cdt = p_new;
		l_fddi_cdt += FDDI_CDT_ENTRIES;
		xmfree (p_old, pinned_heap);

	}
	/*
	 * Add entry 
	 */
	bcopy (p_name, p_fddi_cdt->cdt_entry[num_entries].d_name, 
			sizeof (((struct cdt_entry *) 0)->d_name));
	p_fddi_cdt->cdt_entry[num_entries].d_len = len;
	p_fddi_cdt->cdt_entry[num_entries].d_ptr = ptr;
	p_fddi_cdt->cdt_entry[num_entries].d_segval = 0;
	p_fddi_cdt->cdt_len += sizeof (struct cdt_entry);

	FDDI_DBTRACE ("dadE", p_fddi_cdt, p_fddi_cdt->cdt_len, ptr);

	/* ok */
	return ;
}
/*
 * NAME: fddi_cdt_del
 *                                                                    
 * FUNCTION: deletes an entry to the component dump table
 *                                                                    
 * EXECUTION ENVIRONMENT: process environment only
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *
 * RETURNS: none
 */
void
fddi_cdt_del (
	char	*ptr)
{
	int	i;
	int	rc;
	int	num_entries;

	FDDI_DBTRACE ("ddeB", p_fddi_cdt, p_fddi_cdt->cdt_len, ptr);

	/*
	 * find entry that matches this ptr 
	 */
	num_entries = NUM_ENTRIES(p_fddi_cdt);
	for (i = 0; i < num_entries; i++)
	{
		if (ptr == p_fddi_cdt->cdt_entry[i].d_ptr)
		{
			/* found it */
			break;
		}
	}
	if (i < num_entries)
	{
		/* 
		 * re-pack entries 
		 */
		for ( ; i < num_entries; i++)
		{
			p_fddi_cdt->cdt_entry[i] = p_fddi_cdt->cdt_entry[i+1];
		}
		p_fddi_cdt->cdt_len -= sizeof (struct cdt_entry);
	}

	FDDI_DBTRACE ("ddeE", p_fddi_cdt, p_fddi_cdt->cdt_len, ptr);

	/* ok */
	return ;
}
