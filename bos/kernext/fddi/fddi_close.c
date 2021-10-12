static char sccsid[] = "@(#)66	1.6  src/bos/kernext/fddi/fddi_close.c, sysxfddi, bos411, 9428A410j 3/23/94 15:01:30";
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: fddi_close
 *		free_addr
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
#include "fddiproto.h"
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/sleep.h>


/* 
 * get access to the global device driver control block
 */
extern fddi_tbl_t	fddi_tbl;

extern struct cdt	*p_fddi_cdt;
extern int		l_fddi_cdt;

/*
 * NAME:     fddi_close()
 *
 * FUNCTION: FDDI close entry point from kernel.
 *
 * EXECUTION ENVIRONMENT:
 *	Process thread.
 *
 * NOTES:
 *	This routine will close the device for the
 *	specified user.  For each close, this routine will:
 *
 *		wait for transmit completes when acknowledgements requested
 *
 *	This routine assumes that when it is called there are no users actively
 * 	using the driver.  Users should have freed the filters and 
 *	status filters prior to the close, but close will clean up all resources
 * 	it has allocated.  A user can break the driver by calling routines
 * 	after this one has begun, but this is considered illegal (they have 
 *	freed the resource and are still using it).  The driver will not be
 *	checking for this case.
 *		
 *	
 * RETURNS:
 *		0	- successful
 *
 */
int
fddi_close( fddi_acs_t *p_acs)
{
	int 			ipri;
	int 			iocc;
	int 			ioa;

	FDDI_TRACE("CfcB",p_acs, p_acs->dev.state,0);

	/* 
	 * Lock to syncronize with limbo/bugout.  This will prevent strange 
	 * state changes.  Once CLOSING_STATE is set limbo and bugout will 
	 * behave correctly.
	 */
	ipri = disable_lock(CFDDI_OPLEVEL, &p_acs->dev.slih_lock);

	if (p_acs->dev.state == LIMBO_STATE || 
		p_acs->dev.state == LIMBO_RECOVERY_STATE)
	{
		fddi_logerr(p_acs, ERRID_CFDDI_RCVRY_TERM, __LINE__, __FILE__,
				0, 0, 0);
	}

	p_acs->dev.state = CLOSING_STATE;

	unlock_enable(ipri, &p_acs->dev.slih_lock);

	/* 
	 * Wait for the transmits to drain.  They will either complete normally
	 * or time out and limbo will clear them.
	 */
	while (p_acs->tx.hdw_in_use > 0)
	{
		delay(5*HZ);
	}
	
	/* remove the ACS from the dump table */
	fddi_cdt_del (p_acs);
	fddi_cdt_del (p_acs->tx.p_sf_cache);
	if (--fddi_tbl.open_cnt == 0)
		fddi_cdt_undo_init ();

	ioa = BUSIO_ATT (p_acs->dds.bus_id, p_acs->dds.bus_io_addr);
	PIO_PUTSRX (ioa + FDDI_HMR_REG, FDDI_HMR_BASE_INTS);
	BUSIO_DET (ioa);

	free_tx(p_acs);
	
	free_rx(p_acs);

	free_services(p_acs);

	free_addr(p_acs);

	p_acs->ndd.ndd_flags = NDD_BROADCAST | NDD_SIMPLEX | 
		(p_acs->ndd.ndd_flags & CFDDI_NDD_DAC);

	p_acs->dev.state = DNLD_STATE;

	FDDI_TRACE("CfcE",p_acs,0,0);

	/* unpin the driver code */
	unpincode(fddi_config);

	return(0);

} /* end fddi_close() */

/*
 * NAME:     free_addr
 *
 * FUNCTION: Cleans up the address list freeing all memory on the extra list
 *		(if any).
 *
 * EXECUTION ENVIRONMENT: Process thread 
 *
 * NOTES:
 *	
 * ROUTINES CALLED: 
 *
 * RETURNS:
 *		0	- successful
 *
 */
void
free_addr(	fddi_acs_t	*p_acs)	/* ACS ptr */
{
	int 	i;
	fddi_addr_blk_t *p_addr_blk;

	FDDI_TRACE("CfaB",p_acs,p_acs->addrs.hdw_addr_cnt,
		p_acs->addrs.sw_addr_cnt);
	FDDI_TRACE("CfaC",p_acs->addrs.sw_addrs,0,0);

	for (i=0; i<p_acs->addrs.hdw_addr_cnt; i++)
	{
		p_acs->addrs.hdw_addrs[i].addr_cnt = 0;
	}
	
	while (p_acs->addrs.sw_addrs != 0)
	{
		p_addr_blk = p_addr_blk->next;

		net_free(p_acs->addrs.sw_addrs,M_DEVBUF);

		p_acs->addrs.sw_addrs = p_addr_blk;
	}

	p_acs->addrs.sw_addrs = 0;
	p_acs->addrs.hdw_addr_cnt = 0;
	p_acs->addrs.sw_addr_cnt = 0;
	FDDI_TRACE("CfaE",p_acs,0,0);
}

