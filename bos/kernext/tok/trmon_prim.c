static char sccsid[] = "@(#)14	1.8  src/bos/kernext/tok/trmon_prim.c, sysxtok, bos411, 9428A410j 5/26/94 18:12:21";
/*
 *   COMPONENT_NAME: SYSXTOK
 *
 *   FUNCTIONS: get_adap_point
 *		hwreset
 *		issue_scb_command
 *		pio_retry
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

#include "tokpro.h"

/*
 *  FUNCTION:   get_adap_point()
 *
 *          This function will read the Token-Ring adapter's internal
 *          pointers for the following:
 *              - Microcode Level
 *              - Adapter Addresses
 *                  Node
 *                  Group
 *                  Functional
 *              - Adapter Parameters
 *
 *          This function should be successfully issued before any
 *          READ ADAPTER SCB command can be issued to the adapter.
 *
 *  INPUT:      p_dds   - pointer to DDS
 *
 *
 */
int get_adap_point(dds_t *p_dds)
{  /* begin function */
    
    
    int     ioa;
    ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );

    /*
     *   Get the pointer to the microcode level
     */
    TOK_PUTSRX( ioa + ADDRESS_REG, P_A_UCODE_LEVEL );
    TOK_GETSRX( ioa + DATA_REG, &WRK.p_a_ucode_lvl );

    /*
     *   Get the pointer to the adapter addresses
     */
    TOK_PUTSRX( ioa + ADDRESS_REG, P_A_ADDRS );
    TOK_GETSRX( ioa + DATA_REG, &WRK.p_a_addrs );
    
    /*
     *   Get the pointer to the adapter parmeters
     */
    TOK_PUTSRX( ioa + ADDRESS_REG, P_A_PARMS );
    TOK_GETSRX( ioa + DATA_REG, &WRK.p_a_parms );

    BUSIO_DET( ioa );

    return(WRK.pio_rc);
    
    
}  /* end function get_adap_point() */

/*****************************************************************************/
/*
 * NAME:     hwreset
 *
 * FUNCTION: do a hardware reset of the device
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * - disable interrupts from the card
 * - turn off parity checking on the card
 * - disable DMA arbitration (wait 100 usec)
 * - write to the reset adapter SIF register
 * - wait 150 usec (no register to check if reset is done)
 * - then write to the enable adapter SIF register
 * - turn on parity checking on the card
 * - enable DMA arbitration
 *
 * RETURNS:  nothing
 *
 */
/*****************************************************************************/
void
hwreset (
	dds_t		*p_dds)
{
	int	i;		/* loop	control	*/
	int	tmp;		/* value read from delay reg (ignored) */
	int	ioa, iocc, delay_seg;
	uchar	pos4, pos5;
    
	TRACE_BOTH(MON_OTHER, "Hres", p_dds, 0, 0);
	if (!WRK.mask_int) {
		WRK.mask_int = TRUE;
        	ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );
		TOK_PUTCX( ioa + IMASK_DISABLE, 0x00);
		TRACE_BOTH(MON_OTHER, "H001", p_dds, ioa, 0);
		BUSIO_DET( ioa );
	}
	
        iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
			(ulong)(IO_IOCC + (DDI.slot << 16)));
	TOK_GETPOS( iocc + POS_REG_4, &pos4 );
	pos4 = pos4 & ~( MC_PARITY_ON );
	TOK_PUTPOS( iocc + POS_REG_4, pos4 );
	
	TOK_GETPOS( iocc + POS_REG_5, &pos5 );
	pos5 = pos5 | MC_ARBITRATION;
	TOK_PUTPOS( iocc + POS_REG_5, pos5 );
	TRACE_BOTH(MON_OTHER, "H002", iocc, pos4, pos5);
	IOCC_DET( iocc );
	
	delay_seg = (uint)IOCC_ATT( DDI.bus_id, IOCC_DELAY ); 
	TRACE_BOTH(MON_OTHER, "H003", p_dds, delay_seg, 0);
	i = 0;
	while ( ++i < 100) {
		TOK_GETCX(delay_seg, &tmp); /* delay 1 microsecond */
	}

        ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );
	TRACE_BOTH(MON_OTHER, "H004", p_dds, ioa, 0);
	TOK_PUTSRX( ioa + RESET_REG, 0x00); /* reset adapter */
	
	i = 0;
	while ( ++i < 150) {
		TOK_GETCX(delay_seg, &tmp); /* delay 1 microsecond */
	}
	TRACE_BOTH(MON_OTHER, "H005", p_dds, 0, 0);
	IOCC_DET( delay_seg );
	
	TOK_PUTSRX( ioa + ENABLE_REG, 0x00);
	BUSIO_DET( ioa );

        iocc = (int)IOCC_ATT( (ulong)DDI.bus_id,
			(ulong)(IO_IOCC + (DDI.slot << 16)));
	TOK_GETPOS( iocc + POS_REG_4, &pos4 );
	pos4 = pos4 | MC_PARITY_ON;
	TOK_PUTPOS( iocc + POS_REG_4, pos4 );
	
	TOK_GETPOS( iocc + POS_REG_5, &pos5 );
	pos5 = pos5 & ~(MC_ARBITRATION);
	TOK_PUTPOS( iocc + POS_REG_5, pos5 );
	TRACE_BOTH(MON_OTHER, "H006", iocc, pos4, pos5);
	IOCC_DET( iocc );
}

/*------------------  I S S U E _ S C B _ C O M M A N D  ---------------*/
/*                                                                      */
/*  NAME: issue_scb_command                                             */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Fills in the SCB with COMMAND and ADDRESS and issues an         */
/*      execute to the adapter.  For commands that do not have an       */
/*      address, ADDRESS can be set to zero.                            */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the System Control Block (SCB).                        */
/*                                                                      */
/*  RETURNS: 0                                                          */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: d_kmove					*/
/*                                                                      */
/*----------------------------------------------------------------------*/
void
issue_scb_command (dds_t        *p_dds,
                   unsigned int command,
                   unsigned int address)
{
        volatile t_scb	scb;
        int		ioa, rc, i=0;

	TRACE_BOTH(MON_OTHER, "ISCB", (int)p_dds, command, address);

	/*
	 * The adapter zeros the command field when it is ready to
	 * accept another command.  The only commands that might have
	 * been issued (and the field not zero) are:
	 * set group/functional address, read adapter error log, and
	 * read adapter memory.  They all set the event_wait flag.
	 *
	 * If the field is not zero within 100 times, we'll either
	 * issue another transmit command or something will timeout
	 * and we will clean things up.  Note: it is very rare to
	 * issue a group/functional address other than at startup
	 * (which is normally handled with the open command to the
	 * adapter).  The read adapter error and read adapter commands
	 * are also rare.
	 */
	if (WRK.event_wait) {
		do {
			if (WRK.do_dkmove) {
        			d_kmove (&scb,
					WRK.p_d_scb,
					sizeof(scb),
					WRK.dma_chnl_id,
					DDI.bus_id,
					DMA_READ);
			} else {
				scb.adap_cmd = WRK.p_scb->adap_cmd;
			}

		} while ( (scb.adap_cmd) && (++i < 100) );
	}

	if (WRK.do_dkmove) {
		scb.adap_cmd    = command;
		scb.addr_field1 = ADDR_HI( address );
		scb.addr_field2 = ADDR_LO( address );
		/*
		 *  d_kmove the image through the IOCC cache into system memory
		 *  so that both the cache and memory have the same SCB image
		 */
        	d_kmove (&scb,
			WRK.p_d_scb,
			sizeof(scb),
			WRK.dma_chnl_id,
			DDI.bus_id,
			DMA_WRITE_ONLY);
	} else {
		WRK.p_scb->adap_cmd    = command;
		WRK.p_scb->addr_field1 = ADDR_HI( address );
		WRK.p_scb->addr_field2 = ADDR_LO( address );
	}

	/*
	 * tell the adapter to execute the command
	 */
        ioa = BUSIO_ATT( DDI.bus_id, DDI.io_port );
	TOK_PUTSRX( ioa + COMMAND_REG, EXECUTE);
	BUSIO_DET( ioa );

	if (WRK.pio_rc) {
		bug_out( p_dds, ERRID_CTOK_DEVICE_ERR, NDD_PIO_FAIL, 0 );
	}
}

/*****************************************************************************/
/*
 * NAME pio_retry
 *
 * FUNCTION: This routine is called when a pio rotine returns an
 *	exception.  It will retry the the PIO and do error logging
 *
 * EXECUTION ENVIRONMENT:
 *	Called by interrupt and processes level
 *	This routine is invoked by the PIO_xxxX routines
 *
 * RETURNS:
 *	0 - excetion was retried successfully
 *	exception code of last failure if not successful
 */
/*****************************************************************************/

int
pio_retry(
    dds_t		*p_dds,	     /* tells which adapter this came from  */
    enum pio_func 	iofunc,      /* io function to retry                */
    void		*ioaddr,     /* io address of the exception         */
    long		ioparam)     /* parameter to PIO routine            */

{
    int	retry_count = PIO_RETRY_COUNT;
    int			excpt_code;
    ndd_statblk_t	sb;		/* status block */
    
    TRACE_BOTH(MON_OTHER, "pior", (int)p_dds, (ulong)iofunc, (ulong)ioaddr);

    WRK.pio_errors++;
    
    while(TRUE) {
	/*
	 * check if out of retries, and send a status block to user
	 */
	if (retry_count <= 0) {
	    TRACE_BOTH(MON_OTHER, "pio1", (int)p_dds, excpt_code, WRK.piox);
	    WRK.pio_rc |= excpt_code;
	    WRK.pio_addr = ioaddr;
	    if ((WRK.adap_state == OPEN_STATE)    ||
		(WRK.adap_state == OPEN_PENDING)  ||
		(WRK.adap_state == CLOSE_PENDING) ||
		(WRK.adap_state == LIMBO_STATE)) {
		bzero (&sb, sizeof(sb));
		sb.code = NDD_HARD_FAIL;
		sb.option[0] = NDD_PIO_FAIL;
		TRACE_BOTH(MON_OTHER, "stat", (int)p_dds, sb.code, 0);
		TRACE_BOTH(MON_OTHER, "sta2", NDD.ndd_flags, 0, 0);
		NDD.nd_status( &NDD, &sb );
	    }
	    break;
	}
	retry_count--;
	
	/*
	 * retry the pio function
	 */
	switch (iofunc) {
	case PUTC:
	    excpt_code = BUS_PUTCX((char *)ioaddr, (char)ioparam);
	    break;
	case PUTSR:
	    excpt_code = BUS_PUTSRX((short *)ioaddr, (short)ioparam);
	    break;
	case GETC:
	    excpt_code = BUS_GETCX((char *)ioaddr, (char *)ioparam);
	    break;
	case GETSR:
	    excpt_code = BUS_GETSRX((short *)ioaddr, (short *)ioparam);
	    break;
	case GETL:
	    excpt_code = BUS_GETLX((long *)ioaddr, (long *)ioparam);
	    break;
	default:
	    ASSERT(0);
	}
	
	if (excpt_code == 0)
	    break;
    }
    return (excpt_code);
} /* End of pio_retry() */
