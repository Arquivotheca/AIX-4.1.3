static char sccsid[] = "@(#)74        1.1  src/bos/kernext/dmpa/dmpa_mpx.c, sysxdmpa, bos411, 9428A410j 4/30/93 12:49:09";
/*
 *   COMPONENT_NAME: (SYSXDMPA) MP/A DIAGNOSTICS DEVICE DRIVER
 *
 *   FUNCTIONS: mpampx
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

#include <errno.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/sysmacros.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/id.h>

#include <sys/dmpauser.h>
#include <sys/dmpadd.h>

extern int     acbglobal_lock;                  /* Global Lock Variable */

/*****************************************************************************
** NAME:        mpampx
**
** FUNCTION:    Either allocates a channel with the adapter in exclusive
**              (diagnostic) mode, allocates the next free channel, or
**              deallocates a given channel.
**
**              Get the channel structure for this adapter.
**              Check for a dead adapter.
**              Acquire the adapter lock.
**              Deallocate a multiplexed channel:
**                      free the memory if the channel exists.
**              or ... Allocate the diagnostic mode channel:
**                      Verify access priviledge (superuser).
**                      Verify exlusive access (no other open()s).
**                      Allocate memory for an open structure.
**                      Set adapter mode to DIAGNOSTIC
**                      Return the channel ID.
**              or ... Allocate the next free channel:
**                      Set adapter mode to NORMAL.
**                      Find next free channel ID.
**                      Allocate memory for open structure.
**                      Return the channel ID.
**
** EXECUTION ENVIRONMENT: process thread only.
**
** NOTES:
**    Input:
**              device major/minor numbers
**              mode: delete a channel, open next free channel, or open in
**                      diagnostic mode
**    Output:
**              channel number or error code
**    Called From:
**              kernel system call handler
**    Calls:
**              lockl() unlockl() get_acb() xmfree() xmalloc()
**
** RETURNS:     0 - Success (and allocated channel number)
**              EINTR - received signal while trying to acquire lock
**              ENXIO - device doesn't exist
**              EBUSY - already open... exclusive open (diagnostic mode) failed
**              EBUSY - maximum open()s exceeded
**              ENODEV - couldn't deallocate a non-existent mpx-channel
**              EFAULT - KFREE() (xmemfree()) couldn't free allocated memory
**              EINVAL - invalid device extension (NOT 'D')
**              ESHUTDOWN - the adapter has been shutdown
**
*****************************************************************************/
int
mpampx(
	dev_t dev,              /* major and minor number */
	int *chanp,     /* address for returning allocated channel number */
	char *channame) /* pointer to special file name suffix */
{
	struct acb *acb;
	int i;


    /* log a trace hook */
    DDHKWD1 (HKWD_DD_MPADD, DD_ENTRY_MPX, 0, dev);

    /*
    ** Get the acb structure for this adapter.
    */
    if ((acb = get_acb(minor(dev))) == NULL) {
	    DDHKWD4(HKWD_DD_MPADD,DD_EXIT_MPX, ENXIO, dev,
			    ((channame != NULL) ? *channame : 0), 0, 0);
	    return ENXIO;
    }
    /*
    ** Either Deallocate or Allocate a channel.
    */
    if (channame == NULL) {
	    /*
	    ** Deallocate a channel
	    */
	    if( !(OPENP.op_flags & OP_OPENED) ) {
		    DDHKWD4(HKWD_DD_MPADD,DD_EXIT_MPX,ENODEV,dev,0,0,0);
		    return ENODEV;
	    }
	    OPENP.op_flags &= ~OP_OPENED;
	    acb->flags &= ~OPEN_DIAG;

	    /*
	    ** Free the channel.
	    */
	    free_open_struct(acb);


    } else {
	    /*
	    ** Acquire Adapter Lock, waits for lock or returns early on signal
	    */
	    if (lockl(&acb->adap_lock, LOCK_SIGRET) != LOCK_SUCC) {
		    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EINTR, dev,
				    ((channame!=NULL)?*channame:0), 0, 0);
		    return EINTR;
	    }

	    /*
	    ** Allocate a channel in either diagnostic or normal mode.
	    */
	    if (*channame == 'D') {
		    /*
		    ** Check the callers access priviledge
		    ** (Must be superuser)
		    */
		    if (getuidx(ID_EFFECTIVE)) {
			    unlockl(&acb->adap_lock);
			    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EACCES,
				    dev, *channame, 0, 0);
			    return EACCES;
		    }
		    /*
		    ** Open the adapter in diagnostic mode
		    */
		    if (acb->num_opens) {
			    unlockl(&acb->adap_lock);
			    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EBUSY,
				    dev, *channame, 0, 0);
			    return EBUSY;
		    }
		    acb->flags |= OPEN_DIAG;
		    bzero(OPENP, sizeof(open_t));
		    OPENP.op_flags |= OP_OPENED;
		    *chanp = 0;
	    } else if (*channame != 'C' && *channame != 0) {
		    /*
		    ** Invalid extension --- only 'D' and 'C' are allowed...
		    */
		    unlockl(&acb->adap_lock);
		    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EINVAL,
			    dev, *channame, 0, 0);
		    return EINVAL;
	    } else {
		    /*
		    ** Allocate the next available channel in normal mode.
		    */
		    if (acb->flags & OPEN_DIAG) {
			    /*
			    ** Fail if already open in diagnostic mode.
			    */
			    unlockl(&acb->adap_lock);
			    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EBUSY,
				    dev, *channame, 0, 0);
			    return EBUSY;
		    }

		    if (acb->num_opens >= MPA_MAX_OPENS) {
			    /*
			    ** Fail if already have maximum number of opens.
			    */
			    unlockl(&acb->adap_lock);
			    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EBUSY,
				    dev, *channame, 0, 0);
			    return EBUSY;
		    }

		    /*
		    ** Fail if already in exclusive-use/
		    ** Customer Engineer mode.
		    */
		    if (acb->flags & MPA_CE_OPEN) {
			    unlockl(&acb->adap_lock);
			    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EBUSY,
				    dev, *channame, 0, 0);
			    return EBUSY;
		    }

		    if (*channame == 'C') {
			    /*
			    ** Check the callers access priviledge
			    ** (Must be superuser)
			    */
			    if (getuidx(ID_EFFECTIVE)) {
				    unlockl(&acb->adap_lock);
				    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX,
					    EACCES, dev, *channame, 0, 0);
				    return EACCES;
			    }
			    acb->flags |= MPA_CE_OPEN;
		    }

		    /*
		    ** Find the next free channel
		    */
		    for (i=0; i < MPA_MAX_OPENS && (OPENP.op_flags&OP_OPENED); i++);
		    if (i < MPA_MAX_OPENS) {
			    bzero(OPENP,sizeof(open_t));
			    OPENP.op_flags |= OP_OPENED;
			    if (*channame == 'C')
				    OPENP.op_flags |= OP_CE_OPEN;
			    OPENP.op_pid = getpid();
		    } else {
			    unlockl(&acb->adap_lock);
			    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EBUSY,
				    dev, *channame, 0, 0);
			    return EBUSY;
		    }

		    *chanp = i;
	    }

	    /*
	    ** If this is the first open for this adapter
	    ** allocate and initialize the necessary data
	    ** structures and make sure the driver is pinned.
	    */
	    if (acb->num_opens == 0) {
		    if ( mpa_init_dev(acb) ) {
			    OPENP.op_flags &= ~(OP_OPENED | OP_CE_OPEN);
			    unlockl(&acb->adap_lock);
			    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EIO, dev,
				    *channame, 0, 0);
			    return EIO;
		    }
	    }
	    acb->num_opens++;

	    unlockl(&acb->adap_lock);
    }

    DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_MPX, 0, dev, 0, 0, 0, 0);
    return(0);
}  /*  mpampx() */
