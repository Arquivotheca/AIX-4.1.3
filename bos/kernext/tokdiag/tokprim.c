static char sccsid[] = "@(#)40	1.17  src/bos/kernext/tokdiag/tokprim.c, diagddtok, bos411, 9428A410j 10/26/93 14:58:28";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  pio_read(), pio_write(), attach_bus(), detach_bus(),
 *             attach_iocc(), detach_iocc(), issue_scb_command()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/except.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/comio.h>
#include <sys/trchkid.h>
#include <sys/tokuser.h>
#include <sys/adspace.h>
#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

/*-------------------------  P I O _ R E A D  --------------------------*/
/*                                                                      */
/*  NAME: pio_read                                                      */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Returns the contents of REG after reading it from the bus or    */
/*      iocc attachment established by attach_bus or attach_iocc.  If   */
/*      a PIO exception occurs, the read is retried several times; if   */
/*      the retries fail to complete the read, all future pio accesses  */
/*      are blocked and a -1 is returned.                               */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Uses the pio_attachment in the DDS; modifies pio_block if       */
/*      a fatal error occurs.                                           */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed successfully.                         */
/*      -1              Unrecoverable PIO error or block.               */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: i_disable, i_enable, setjumpx,          */
/*                              longjumpx, clrjumpx, BUS_GETSR,         */
/*                              BUS_GETC                                */
/*                                                                      */
/*----------------------------------------------------------------------*/

pio_read (
        dds_t           *p_dds,
        unsigned int    reg)
{
        int             x, rc;
        label_t         jump_buf;               /* jump buffer */
        volatile int    retry = PIO_RETRY_COUNT;

	/*
	 * FUTURE FIX:
	 * 	Add support for handling an error when the
	 *	error -1 is returned to all routines that call
	 *	the pio_read() routine.  When that is done, the
	 *	following lines can then be uncommented.
	 *
	 *	if (p_dds->wrk.pio_block)               
	 *		return(-1); 		 if blocked,  return an error 
	 */

        x = i_disable(INTCLASS2);
        while (TRUE) {                          /* retry loop */
            /*                                                            */
            /*  Set up for context save by doing setjumpx.  If it returns */
            /*  zero, all is well; otherwise, an exception occurred.      */
            /*                                                            */
            if ((rc = setjmpx(&jump_buf)) == 0) {
                if (retry--) {                  /* retry? */
                    /*                                                   */
                    /*  If retries have not been used up, do the read.   */
                    /*  If the register is two bytes, use BUSIO_GETSR    */
                    /*  to read and byte-swap the value from the offset  */
                    /*  added to the current pio attachment; otherwise,  */
                    /*  use BUSIO_GETC to read a single byte.            */
                    /*                                                   */
                    if (REG_SIZE(reg) == sizeof(short))
                        rc = BUSIO_GETSR( p_dds->wrk.pio_attachment
                                        + REG_OFFSET( reg ));
                    else
                        rc = BUSIO_GETC ( p_dds->wrk.pio_attachment
                                        + REG_OFFSET( reg ));
                    break;                      /* exit retry loop */
                } else {
                    /*                                                   */
		    /*  FUTURE FIX:					 */
		    /*		Add support for the pio_block		 */
		    /*		flag. uncomment the following line	 */
                    /*  Out of retries, so block future pio accesses     */
                    /*  and return an error.                             */
		    /*							 */
                    /*  p_dds->wrk.pio_block = TRUE;			 */
                    /*                                                   */
			TRACE5("FooT", (ulong)PIO_READ_1, 
				(ulong)rc, (ulong)reg, (ulong)retry );

			p_dds->wrk.footprint = PIO_READ_1;
			p_dds->wrk.piox = rc;
			logerr( p_dds, ERRID_TOK_PIO_ERR);
                    rc = -1;
                    break;
                }
            } else {
                /*                                                       */
                /*  An exception has occurred or reoccurred -- if it is  */
                /*  a PIO error, simply retry; else, it is an exception  */
                /*  not handled here so longjmpx to the next handler on  */
                /*  the stack.                                           */
                /*                                                       */
                if (rc != EXCEPT_IO)
                    longjmpx(rc);
                p_dds->wrk.pio_errors++;
		p_dds->wrk.footprint = PIO_READ_0;
		p_dds->wrk.piox = rc;
		TRACE5("FooT", (ulong)PIO_READ_0, 
			(ulong)rc, (ulong)reg, (ulong)retry );
            }
        }                                       /* end retry loop */
        /*  Out of retry loop -- remove jump buffer from exception       */
        /*  stack and return.                                            */
        /*                                                               */
        clrjmpx(&jump_buf);
        i_enable(x);
        return (rc);
}

/*-----------------------  P I O _ W R I T E  --------------------------*/
/*                                                                      */
/*  NAME: pio_write                                                     */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Writes VALUE to the register REG according to the bus or        */
/*      iocc attachment established by attach_bus or attach_iocc.  If   */
/*      a PIO exception occurs, the write is retried several times; if  */
/*      the retries fail to complete the write, all future pio          */
/*      accesses are blocked and a -1 is returned.                      */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Uses the pio_attachment in the DDS; modifies pio_block if       */
/*      a fatal error occurs.                                           */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0               Completed successfully.                         */
/*      -1              Unrecoverable PIO error or block.               */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: i_disable, i_enable, setjumpx,          */
/*                              longjumpx, clrjumpx, BUS_PUTSR,         */
/*                              BUS_PUTC                                */
/*                                                                      */
/*----------------------------------------------------------------------*/

pio_write (
        dds_t           *p_dds,
        unsigned int    reg,
        unsigned short  value)
{
        int             x, rc;
        label_t         jump_buf;               /* jump buffer */
        volatile int    retry = PIO_RETRY_COUNT;


	/*
	 * FUTURE FIX:
	 * 	Add support for handling an error when the
	 *	error -1 is returned to all routines that call
	 *	the pio_write() routine.  When that is done, the
	 *	following lines can then be uncommented.
	 *
	 *	if (p_dds->wrk.pio_block)               
	 *		return(-1); 		 if blocked,  return an error 
	 */

        x = i_disable(INTCLASS2);
        while (TRUE) {                          /* retry loop */
            /*                                                            */
            /*  Set up for context save by doing setjumpx.  If it returns */
            /*  zero, all is well; otherwise, an exception occurred.      */
            /*                                                            */
            if ((rc = setjmpx(&jump_buf)) == 0) {
                if (retry--) {                  /* retry? */
                    /*                                                   */
                    /*  If retries have not been used up, do the write.  */
                    /*  If the register is two bytes, use BUSIO_PUTSR    */
                    /*  to byteswap and write VALUE to the offset added  */
                    /*  to the current pio_attachment; otherwise, use    */
                    /*  BUSIO_PUTC to write a single byte.               */
                    /*                                                   */
                    if (REG_SIZE(reg) == sizeof(short))
                        BUSIO_PUTSR( p_dds->wrk.pio_attachment
                                + REG_OFFSET( reg ), value);
                    else
                        BUSIO_PUTC(  p_dds->wrk.pio_attachment
                                + REG_OFFSET( reg ), value);
                    rc = 0;
                    break;                      /* exit retry loop */
                } else {
                    /*                                                   */
		    /*  FUTURE FIX:					 */
		    /*		Add support for the pio_block		 */
		    /*		flag. uncomment the following line	 */
		    /*							 */
                    /*  p_dds->wrk.pio_block = TRUE;			 */
                    /*                                                   */
                    /*  Out of retries, so block future pio accesses     */
                    /*  and return an error.                             */
                    /*                                                   */
			TRACE5("FooT", (ulong)PIO_WRITE_1, 
				(ulong)rc, (ulong)reg, (ulong)retry );

			p_dds->wrk.footprint = PIO_READ_1;
			p_dds->wrk.piox = rc;
			logerr( p_dds, ERRID_TOK_PIO_ERR);
                    rc = -1;
                    break;
                }
            } else {
                /*                                                       */
                /*  An exception has occurred or reoccurred -- if it is  */
                /*  a PIO error, simply retry; else, it is an exception  */
                /*  not handled here so longjmpx to the next handler on  */
                /*  the stack.                                           */
                /*                                                       */
                if (rc != EXCEPT_IO)
                    longjmpx(rc);
                p_dds->wrk.pio_errors++;
		p_dds->wrk.footprint = PIO_WRITE_0;
		p_dds->wrk.piox = rc;
		TRACE5("FooT", (ulong)PIO_WRITE_0, 
			(ulong)rc, (ulong)reg, (ulong)retry );
            }
        }                                       /* end retry loop */
        /*  Out of retry loop -- remove jump buffer from exception       */
        /*  stack and return.                                            */
        /*                                                               */
        clrjmpx(&jump_buf);
        i_enable(x);
        return (rc);
}

/*---------------------   A T T A C H _ B U S   ------------------------*/
/*                                                                      */
/*  NAME: attach_bus                                                    */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Attaches the driver to the bus for I/O accesses.  Returns the   */
/*      value of the previous attachment; this is passed to "detach_    */
/*      bus" to restore the attachment to its previous value.  Note     */
/*      that attachments can be nested, but only one attachment can     */
/*      be in effect at any given time.                                 */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the pio_attachment in the DDS for use by pio_write     */
/*      and pio_read.                                                   */
/*                                                                      */
/*  RETURNS: Previous attachment value.                                 */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: i_disable, i_enable, BUSIO_ATT          */
/*                                                                      */
/*----------------------------------------------------------------------*/

attach_bus (
        dds_t           *p_dds)
{
        int     x, save_att;

        /*  Block interrupts to ensure an atomic operation; save the    */
        /*  old attachment value for return and get a new bus attach-   */
        /*  ment for this access.                                       */
        /*                                                              */
        x = i_disable(INTCLASS2);
        save_att = p_dds->wrk.pio_attachment;
        p_dds->wrk.pio_attachment =
                BUSIO_ATT( p_dds->ddi.bus_id,
                        p_dds->ddi.bus_io_addr );
        i_enable(x);
        return(save_att);
}

/*---------------------   D E T A C H _ B U S   ------------------------*/
/*                                                                      */
/*  NAME: detach_bus                                                    */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Detaches the driver from the bus; takes the value of the        */
/*      previous attachment and restores it.                            */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the pio_attachment in the DDS for use by pio_write     */
/*      and pio_read.                                                   */
/*                                                                      */
/*  RETURNS: Nothing.                                                   */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: i_disable, i_enable, BUSIO_DET          */
/*                                                                      */
/*----------------------------------------------------------------------*/

detach_bus (
        dds_t           *p_dds,
        unsigned int    prev_att)
{
        int     x;

        /*  Block interrupts to insure an atomic operation; restore the */
        /*  old attachment value after removing the current attachment  */
        /*                                                              */
        x = i_disable(INTCLASS2);
        BUSIO_DET(p_dds->wrk.pio_attachment);
        p_dds->wrk.pio_attachment = prev_att;
        i_enable(x);
}

/*---------------------  A T T A C H _ I O C C  ------------------------*/
/*                                                                      */
/*  NAME: attach_iocc                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Attaches the driver to the iocc for I/O accesses.  Returns the  */
/*      value of the previous attachment; this is passed to "detach_    */
/*      iocc" to restore the attachment to its previous value.  Note    */
/*      that attachments can be nested, but only one attachment can     */
/*      be in effect at any given time.                                 */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the pio_attachment in the DDS for use by pio_write     */
/*      and pio_read.                                                   */
/*                                                                      */
/*  RETURNS: Previous attachment value.                                 */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: i_disable, i_enable, IOCC_ATT           */
/*                                                                      */
/*----------------------------------------------------------------------*/

attach_iocc (
        dds_t   *p_dds)
{
        int     x, save_att;

        /*  Block interrupts to ensure an atomic operation; save the    */
        /*  old attachment value for return and get a new iocc attach-  */
        /*  ment for this access.                                       */
        /*                                                              */
        x = i_disable(INTCLASS2);
        save_att = p_dds->wrk.pio_attachment;
        p_dds->wrk.pio_attachment =
                IOCC_ATT( p_dds->ddi.bus_id,
                                IO_IOCC + (p_dds->ddi.slot << 16));
        i_enable(x);
        return(save_att);
}

/*--------------------   D E T A C H _ I O C C  ------------------------*/
/*                                                                      */
/*  NAME: detach_iocc                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Detaches the driver from the iocc; takes the value of the       */
/*      previous attachment and restores it.                            */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the pio_attachment in the DDS for use by pio_write     */
/*      and pio_read.                                                   */
/*                                                                      */
/*  RETURNS: Nothing.                                                   */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: i_disable, i_enable, IOCC_DET           */
/*                                                                      */
/*----------------------------------------------------------------------*/

detach_iocc (
        dds_t           *p_dds,
        unsigned int    prev_att)
{
        int     x;

        /*  Block interrupts to insure an atomic operation; restore the */
        /*  old attachment value after removing the current attachment  */
        /*                                                              */
        x = i_disable(INTCLASS2);
        IOCC_DET( p_dds->wrk.pio_attachment );
        p_dds->wrk.pio_attachment = prev_att;
        i_enable(x);
}

/*-------------  I N I T I A T E _ S C B _ C O M M A N D  --------------*/
/*                                                                      */
/*  NAME: initiate_scb_command                                          */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Contends for access to the SCB, then fills in COMMAND and       */
/*      ADDRESS before issuing an execute command to the adapter.       */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can only be called from a process.                              */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Modifies the SCB lock, event, and free control words.           */
/*                                                                      */
/*  RETURNS: Nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: i_disable, i_enable, lockl, unlockl,    */
/*                              issue_scb_command.                      */
/*                                                                      */
/*----------------------------------------------------------------------*/

initiate_scb_command (dds_t        *p_dds,
                   unsigned int command,
                   unsigned int address)
{
        /*  Access the lock to the scb; this causes all process scb     */
        /*  accesses to be serialized.                                  */
        /*                                                              */
        lockl( &p_dds->wrk.scb_lock, LOCK_SHORT );
        /*                                                              */
        /*  We own the lock!  Issue the scb command to the adapter and  */
        /*  release the lock for another access.                        */
        /*                                                              */
        issue_scb_command( p_dds, command, address );
        unlockl( &p_dds->wrk.scb_lock );
        return;
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
/*  EXTERNAL PROCEDURES CALLED: i_disable, i_enable, xmattach, d_kmove, */
/*                              xmdetach, attach_bus, detach_bus,       */
/*                              pio_write.                              */
/*                                                                      */
/*----------------------------------------------------------------------*/

issue_scb_command (dds_t        *p_dds,
                   unsigned int command,
                   unsigned int address)
{
        volatile t_scb          scb;            /* system command block */
        unsigned int            x;
        int                     pio_attachment, rc;

        x = i_disable(INTCLASS2);
        /*                                                           */
        /*  Fill in a local image of the SCB with the address of the */
        /*  command; the command field with the command.             */
        /*                                                           */
        scb.adap_cmd    = command;
        scb.addr_field1 = ADDR_HI( address );
        scb.addr_field2 = ADDR_LO( address );
	TRACE4("ISCB", command, ADDR_HI( address), ADDR_LO( address));
        /*                                                           */
        /*  Attach the local scb image using a local cross memory    */
        /*  descriptor.                                              */
        /*                                                           */

        /*                                                           */
        /*  D_move the image through the IOCC cache into system      */
        /*  memory so that both the cache and memory have the same   */
        /*  SCB image.                                               */
        /*                                                           */

        rc = d_kmove (&scb,
                 p_dds->wrk.p_d_scb,
                 sizeof(scb),
                 p_dds->wrk.dma_chnl_id,
                 p_dds->ddi.bus_id,
                 DMA_WRITE_ONLY);

	if (rc == EINVAL) 	/* IOCC is NOT buffered */
           bcopy (&scb,
              p_dds->wrk.p_scb,
              sizeof(scb));

        /*                                                           */
        /*  Issue the pio EXECUTE command to the adapter; this       */
        /*  causes it to read the scb and process the command.       */
        /*                                                           */
        pio_attachment = attach_bus(p_dds);
        pio_write(p_dds, COMMAND_REG, EXECUTE);
        detach_bus( p_dds, pio_attachment );
        i_enable(x);
        return(0);
}

/*------------------  C H E C K _ S C B _ C O M M A N D  ---------------*/
/*                                                                      */
/*  NAME: check_scb_command                                             */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Checks if the command field of the SCB is zero (adapter has     */
/*      obtained the command from the SCB).                             */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Reads the System Control Block (SCB).                           */
/*                                                                      */
/*  RETURNS: nothing                                                    */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: bcopy, d_kmove, delay                   */
/*                                                                      */
/*----------------------------------------------------------------------*/

void
check_scb_command (dds_t *p_dds)
{
        volatile t_scb          scb;            /* system command block */
        int                     i=0, rc;

        /* 
         *  Loop reading the value of the SCB until the command field
	 *  is zero or one tenth second has passed.
	 *
	 *  Since the adapter is using DMA to update the SCB, it is necessary
         *  to d_kmove the image through the IOCC cache into system memory
         *  so that both the cache and memory have the same SCB image.
         */

	do {
        	rc = d_kmove (&scb,
			WRK.p_d_scb,
			sizeof(scb),
			WRK.dma_chnl_id,
			DDI.bus_id,
			DMA_READ);

		if (rc == EINVAL) 	/* IOCC is NOT buffered */
			bcopy (WRK.p_scb,
				&scb,
				sizeof(scb));

		if (scb.adap_cmd)
			delay (HZ/100);

	} while ( (scb.adap_cmd) && (++i < HZ/10) );

	return;
}
