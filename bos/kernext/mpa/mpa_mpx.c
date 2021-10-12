static char sccsid[] = "@(#)86        1.1  src/bos/kernext/mpa/mpa_mpx.c, sysxmpa, bos411, 9428A410j 4/30/93 12:52:02";
/*
 *   COMPONENT_NAME: (SYSXMPA) MP/A SDLC DEVICE DRIVER
 *
 *   FUNCTIONS: mpa_mpx
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

#include <sys/mpadd.h>

extern int     acbglobal_lock;                  /* Global Lock Variable */

/*****************************************************************************
** NAME:        mpa_mpx
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
mpa_mpx(
	dev_t dev,      /* major and minor number */
	int *chanp,     /* address for returning allocated channel number */
	char *channame) /* pointer to special file name suffix */
{
	struct acb *acb;
 	int rc=0;


    /*����������������Ŀ
    � log a trace hook �
    ������������������*/
    DDHKWD1 (HKWD_DD_MPADD, DD_ENTRY_MPX, 0, dev);

    /*���������������������������������������Ŀ
    � Get the acb structure for this adapter. �
    �����������������������������������������*/
    if ((acb = get_acb(minor(dev))) == NULL) 
    {
	    DDHKWD4(HKWD_DD_MPADD,DD_EXIT_MPX, ENXIO, dev,
			    ((channame != NULL) ? *channame : 0), 0, 0);
	    return ENXIO;
    }

    /*����������������������������������������Ŀ
    � Either Deallocate or Allocate a channel. �
    ������������������������������������������*/
    if (channame == NULL) 
    {
	    /*��������������������Ŀ
	    � Deallocate a channel �
	    ����������������������*/
	    if( !(OPENP.op_flags & OP_OPENED) ) 
    	    {
		    DDHKWD4(HKWD_DD_MPADD,DD_EXIT_MPX,ENODEV,dev,0,0,0);
		    return ENODEV;
	    }
	    OPENP.op_flags &= ~OP_OPENED;

    } 
    else 
    {
	    /*���������������������������������������������������������������Ŀ
	    � Acquire Adapter Lock, waits for lock or returns early on signal �
	    �����������������������������������������������������������������*/
	    if (lockl(&acb->adap_lock, LOCK_SIGRET) != LOCK_SUCC) 
	    {
		    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EINTR, dev,
				    ((channame!=NULL)?*channame:0), 0, 0);
		    return EINTR;
	    }

	    /*������������������Ŀ
	    � Allocate a channel �
	    ��������������������*/
	    if ( *channame != 0) 
	    {
		    /*��������������������Ŀ
		    � Invalid extension... �
		    ����������������������*/
		    unlockl(&acb->adap_lock);
		    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EINVAL,
			    dev, *channame, 0, 0);
		    return EINVAL;
	    } 
	    else 
	    {
		    /*�����������������������������������Ŀ
		    � Allocate the next available channel �
		    �������������������������������������*/

		    if (acb->num_opens >= MPA_MAX_OPENS) 
		    {
			    /*���������������������������������������������Ŀ
			    � Fail if already have maximum number of opens. �
			    �����������������������������������������������*/
			    unlockl(&acb->adap_lock);
			    DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EBUSY,
				    dev, *channame, 0, 0);
			    return EBUSY;
		    }

		    /*���������������������������������Ŀ
		    � check to see if device has already�
		    � been opened (only allow 1 open)   �
		    �����������������������������������*/

                    if (OPENP.op_flags&OP_OPENED) 
		    {
                            unlockl(&acb->adap_lock);
                            DDHKWD4(HKWD_DD_MPADD, DD_EXIT_MPX, EBUSY,
                                    dev, *channame, 0, 0);
                            return EBUSY;
                    }
                    else 
		    {
                            bzero(OPENP,sizeof(open_t));
                            OPENP.op_flags |= OP_OPENED;
                            OPENP.op_pid = getpid();
                    }
		    
		    /*�������������������������������������������Ŀ
		    � always return zero as the channel allocated �
		    � since this is not a multiplex device        �
		    ���������������������������������������������*/
		    *chanp = 0;


	    }

	    unlockl(&acb->adap_lock);
    }

    DDHKWD5 (HKWD_DD_MPADD, DD_EXIT_MPX, 0, dev, 0, 0, 0, 0);
    return(0);
}  /*  mpa_mpx() */
