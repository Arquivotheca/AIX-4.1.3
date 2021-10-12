static char sccsid[] = "@(#)42	1.6  src/bos/kernext/sol/sol_ioctl.c, sysxsol, bos411, 9428A410j 9/13/91 11:43:19";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  sol_ioctl
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/dma.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include <sys/errids.h>
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/limits.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pri.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/ioacc.h>
#include <sys/m_except.h>
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include "soldd.h"

extern struct sol_ddi		sol_ddi;
extern uchar			cck_proc[];

/* 64 byte array coded with link control ala sequence with a */
/* disrupted crc character.  Will be sent in Transparent mode to */
/* test crc error status.*/
int ala_mode_data[16] = {
	0xC14FAC14, 0x3E707538, 0x62D8B62C, 0x8CF19154,
	0x3DB0A3E8, 0xC14FAC14, 0x3EB053E8, 0xC14FAC14,
	0x3EB053E8, 0xC14FAC14, 0x3EB053E8, 0xC14FAC14,
	0x3EB053E8, 0xC14FAC14, 0x3EB053E8, 0xC14FAC14,
};

int offline_seq_data[16] = {
	0x3E8C53E8, 0x314FA314, 0x3E8C53E8, 0x314FA314,
	0x3E8C53E8, 0x314FA314, 0x3E8C53E8, 0x314FA314,
	0x3E8C53E8, 0x314FA314, 0x3E8C53E8, 0x314FA314,
	0x3E8C53E8, 0x314FA314, 0x3E8C53E8, 0x314FA314,
};

/*
 * NAME: sol_ioctl
 *
 * FUNCTION: Provides various functions for both diagnostic and normal mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from the process environment, and it can
 *	page fault.
 *
 * NOTES: 
 *
 *	Possible ioctl operations:
 *	IOCINFO		- get device handler information
 *	CIO_START	- start the device
 *	CIO_HALT	- halt the device
 *	CIO_QUERY	- query device statistics
 *	CIO_GET_STAT	- get device status
 *	CIO_GET_FASTWRT	- fast write function address
 *	SOL_GET_PRIDS	- get list of connected processor id's
 *	SOL_CHECK_PRID	- check to see if a processor id is accessable
 *
 *	Diagnostic ioctls:
 *	SOL_BUFFER_ACCESS		SOL_SYNC_OTP
 *	SOL_ACTIVATE			SOL_SCR
 *	SOL_OLS				SOL_RHR
 *	SOL_CRC				SOL_LOCK_TO_XTAL
 *	SOL_CARD_PRESENT
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	ENODEV	- devno is invalid
 *	EACCES	- attempted diagnostic ioctl when not in diagnostic mode
 *	EINVAL	- invalid ioctl operation
 *	others	- see return values from individual ioctl routines
 */

int
sol_ioctl(
dev_t	devno,			/* major/minor device number		*/
int	cmd,			/* specifies operation to perform	*/
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan,			/* channel allocated by sol_mpx		*/
int	ext)			/* extension parameter not used		*/
{
	int	rc;
	int	sla_num;

	SYS_SOL_TRACE("PioB", devno, chan, cmd);
	if ((uint)(minor(devno) > (uint)SOL_OPS_MINOR) ||
	    ((uint) chan >= (uint) SOL_TOTAL_OPENS) ||
	    (sol_ddi.open_ptr[chan] == NULL)) {
		SYS_SOL_TRACE("PioE", ENODEV, chan, cmd);
		return ENODEV;
	}
	(void) lockl(&(sol_ddi.global_lock), LOCK_SHORT);
	sla_num = minor(devno);
	switch (cmd) {
	case IOCINFO:
		rc = sol_iocinfo(arg, devflag, chan);
		break;
	case CIO_START:
		rc = sol_cio_start(arg, devflag, chan);
		break;
	case CIO_HALT:
		rc = sol_cio_halt(arg, devflag, chan);
		break;
	case CIO_QUERY:
		rc = sol_cio_query(arg, devflag, chan);
		break;
	case CIO_GET_STAT:
		/*  NOTE:  get_stat is a bottom-half routine (sol_misc.c) */
		rc = sol_cio_get_stat(arg, devflag, chan);
		break;
	case CIO_GET_FASTWRT:
		rc = sol_cio_get_fastwrt(devno,
		    (struct cio_get_fastwrt *) arg, devflag, chan);
		break;
	case SOL_GET_PRIDS:
		rc = sol_get_prids(arg, devflag, chan);
		break;
	case SOL_CHECK_PRID:
		rc =sol_check_prid(arg, devflag, chan);
		break;
	case SOL_BUFFER_ACCESS:
		rc = sol_buffer_access(sla_num, arg, devflag, chan);
		break;
	case SOL_SYNC_OTP:
		rc = sol_sync_otp(sla_num, arg, devflag, chan);
		break;
	case SOL_LOCK_TO_XTAL:
		rc = sol_lock_to_xtal(sla_num, arg, devflag, chan);
		break;
	case SOL_ACTIVATE:
		rc = sol_activate(sla_num, arg, devflag, chan);
		break;
	case SOL_SCR:
		rc = sol_scr(sla_num, arg, devflag, chan);
		break;
	case SOL_OLS:
		rc = sol_ols(sla_num, arg, devflag, chan);
		break;
	case SOL_RHR:
		rc = sol_rhr(sla_num, arg, devflag, chan);
		break;
	case SOL_CRC:
		rc = sol_crc(sla_num, arg, devflag, chan);
		break;
	case SOL_CARD_PRESENT:
		rc = sol_card_present(sla_num, arg, devflag, chan);
		break;
	case SOL_LOOPBACK_TEST:
		rc = sol_loopback();
		break;
	default:
		rc = EINVAL;
		break;
	}
	unlockl(&(sol_ddi.global_lock));
	SYS_SOL_TRACE("PioE", rc, chan, cmd);
	return rc;
}

/*
 * NAME: sol_iocinfo
 *
 * FUNCTION: Provides information from the device handler.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EFAULT	- invalid address
 */

int
sol_iocinfo(
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/
{
	struct devinfo	info;
	int		rc;

	info.devtype = DD_NET_DH;
	info.flags = 0x00;
	info.devsubtype = DD_SOL;
	info.un.sol.broad_wrap = FALSE;
	info.un.sol.rdto = 0;
	info.un.sol.processor_id = sol_ddi.ops_info.processor_id;
	rc = MOVEOUT(devflag, &info, arg, sizeof(info));
	return rc;
}

/*
 * NAME: sol_cio_start
 *
 * FUNCTION: Starts a specified netid.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0		- successful completion                        
 *	EINVAL		- invalid parameter
 *	ENETDOWN	- unrecoverable hardware failure
 *	EIO		- error occured, see the status field for more info
 *	ENOSPC		- the netid table is full
 *	EADDRINUSE	- the netid is already in use
 *	EFAULT		- invalid address supplied
 *	ENODEV		- invalid minor number
 */

int
sol_cio_start(
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/
{
	struct sol_open_struct	*open_ptr;
	cio_sess_blk_t		sess_blk;
	cio_stat_blk_t		stat_blk;
	int			rc,i;

	SYS_SOL_TRACE("PstB", arg, devflag, chan);
	if (rc = MOVEIN(devflag, arg, &sess_blk, sizeof(cio_sess_blk_t))) {
		SYS_SOL_TRACE("PstE", rc, 0, chan);
		return rc;
	}
	/*
	 *  Check if we have exceeded the max number of netids
	 */
	if (sol_ddi.num_netids == SOL_MAX_NETIDS) {
		sess_blk.status = CIO_NETID_FULL;
		rc = MOVEOUT(devflag, &sess_blk, arg, sizeof(cio_sess_blk_t));
		SYS_SOL_TRACE("PstE", EIO, sess_blk.status, chan);
		return EIO;
	}
	/*
	 *  Check for a group netid (odd) - not supported
	 */
	if (sess_blk.netid % 2) {
		sess_blk.status = CIO_NETID_INV;
		rc = MOVEOUT(devflag, &sess_blk, arg, sizeof(cio_sess_blk_t));
		SYS_SOL_TRACE("PstE", EIO, sess_blk.status, chan);
		return EIO;
	}
	/*
	 * Check for duplicate netid.
	 */
	open_ptr = sol_ddi.netid_table[sess_blk.netid >> 1];
	if (open_ptr != NULL) { /* duplicate netid found */
		sess_blk.status = CIO_NETID_DUP;
		rc = MOVEOUT(devflag, &sess_blk, arg, sizeof(cio_sess_blk_t));
		SYS_SOL_TRACE("PstE", EIO, sess_blk.status, chan);
		return EIO;
	}

	open_ptr = sol_ddi.open_ptr[chan];
	SYS_SOL_TRACE("Pst1", chan, open_ptr, sess_blk.netid);

	/*
	 *  Add this netid to the netid table
	 */
	sol_ddi.netid_table[sess_blk.netid >> 1] = open_ptr;
	sol_ddi.num_netids++;
	open_ptr->num_netids++;

	/*
	 * Update statistics if necessary.
	 */
	if (sol_ddi.num_netids > (uchar)sol_ddi.stats.cc.nid_tbl_high) {
		sol_ddi.stats.cc.nid_tbl_high++;
	}

	if (sol_ddi.start_state == SOL_HALTED) { /* first start */
		/*
		 *  This is the first start.  Tell IMCS to startup the
		 *  hardware, and start sending "hello" messages.
		 */
		sol_ddi.start_state = SOL_STARTING;
		(void) imcs_start(sol_ddi.ops_info.processor_id);
	} else if (sol_ddi.start_state == SOL_STARTED) {
		/*
		 *  The device is already started, so just build the status
		 *  block and return.
		 */
		stat_blk.code = CIO_START_DONE;
		stat_blk.option[0] = CIO_OK;
		stat_blk.option[1] = sess_blk.netid;
		sol_report_status(open_ptr, &stat_blk);
	}
	/*
	 *  If the state is SOL_STARTING, the async status will be sent
	 *  from the off_level so nothing has to be done here.
	 *  The state will never be SOL_HALTING because the halt completes
	 *  synchronously, and holds the global lock.
	 */
	sess_blk.status = CIO_OK;
	rc = MOVEOUT(devflag, &sess_blk, arg, sizeof(cio_sess_blk_t));
	SYS_SOL_TRACE("PstE", 0, sess_blk.status, chan);
	return 0;
}
	

/*
 * NAME: sol_cio_halt
 *
 * FUNCTION: Halts a specified netid.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	ENODEV	- invalid minor number specified
 */

int
sol_cio_halt(
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/
{
	struct sol_open_struct	*open_ptr;
	struct mbuf		*m;
	struct pm		*pm;
	cio_sess_blk_t		sess_blk;
	cio_stat_blk_t		stat_blk;
	int			rc,i;

	SYS_SOL_TRACE("PhtB", arg, devflag, chan);
	if (rc = MOVEIN(devflag, arg, &sess_blk, sizeof(cio_sess_blk_t))) {
		SYS_SOL_TRACE("PhtE", rc, 0, chan);
		return rc;
	}
	/*
	 *  Check for a group netid (odd)
	 */
	if (sess_blk.netid % 2) {
		sess_blk.status = CIO_NETID_INV;
		rc = MOVEOUT(devflag, &sess_blk, arg,sizeof(cio_sess_blk_t));
		SYS_SOL_TRACE("PhtE", EIO, sess_blk.status, chan);
		return EIO;
	}
	/*
	 *  Find the netid.
	 */
	open_ptr = sol_ddi.netid_table[sess_blk.netid >> 1];
	if (open_ptr == NULL) { /* netid not found */
		sess_blk.status = CIO_NETID_INV;
		rc = MOVEOUT(devflag, &sess_blk, arg,sizeof(cio_sess_blk_t));
		SYS_SOL_TRACE("PhtE", EIO, sess_blk.status, chan);
		return EIO;
	}
	SYS_SOL_TRACE("Pht1", chan, open_ptr, sess_blk.netid);
	/*
	 *  Remove the netid from the table.
	 */
	sol_ddi.num_netids--;
	open_ptr->num_netids--;
	sol_ddi.netid_table[sess_blk.netid >> 1] = NULL;

	/*
	 * Build async CIO_HALT_DONE status with CIO_OK
	 */
	stat_blk.code = CIO_HALT_DONE;
	stat_blk.option[0] = CIO_OK;
	stat_blk.option[1] = sess_blk.netid;
	sol_report_status(open_ptr, &stat_blk);
	if (sol_ddi.num_netids == 0) {
		/*
		 *  If this is the last halt, shut down imcs.
		 */
		sol_shutdown();
	}
	sess_blk.status = CIO_OK;
	rc = MOVEOUT(devflag, &sess_blk, arg, sizeof(cio_sess_blk_t));
	SYS_SOL_TRACE("PhtE", 0, sess_blk.status, chan);
	return 0;
}

/*
 * NAME: sol_shutdown
 *
 * FUNCTION: Performs the final halt of the device
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from sol_cio_halt and sol_close, in the
 *	process environment, and it can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  Nothing.
 */

void
sol_shutdown()

{
	struct mbuf		*m;
	struct super_header	*super_header;
	struct pm		*pm;
	int			i;

	imcs_undeclare(SOL_IMCS_QID);
	sol_ddi.start_state = SOL_HALTED;
	imcs_stop();

	/*
	 *  Free all the mbufs we have, starting with free
	 *  small mbufs that have been mapped.
	 */

	m = sol_ddi.freembuf;
	while (m != NULL) {
		m->m_type = MT_DATA;
		m->m_data = m->m_dat;
		m->m_flags = 0;
		m->m_nextpkt = NULL;
		m = m->m_next;
	}
	m_freem(sol_ddi.freembuf);
	sol_ddi.freembuf = NULL;

	/*
	 *  Next free all the cluster mbufs that have been
	 *  mapped.
	 */

	m_freem(sol_ddi.freeclus);
	sol_ddi.freeclus = NULL;

	/*
	 *  Free up all the mbufs that are in the small
	 *  mbuf receive chains.
	 */

	while (sol_ddi.hdrmlist != NULL) {
		super_header = sol_ddi.hdrmlist;
		sol_ddi.hdrmlist = super_header->imcs_header.imcs_chain_word;
		sol_ddi.num_mbuf--;
		for (i=0 ; i<SOL_MMBUF_LEN ; i++) {
			m = super_header->rcv_tx.rcv_info.msave[i];
			m->m_type = MT_DATA;
			m->m_data = m->m_dat;
			m->m_next = NULL;
			m->m_flags = 0;
			m->m_nextpkt = NULL;
			m_free(m);
		}

		/*  Free the header */

		m_free((struct mbuf *) super_header);
	}

	/*
	 *  Free up all the mbufs and clusters that are in
	 *  the cluster receive chains.
	 */

	while (sol_ddi.hdrclist != NULL) {
		super_header = sol_ddi.hdrclist;
		sol_ddi.hdrclist = super_header->imcs_header.imcs_chain_word;
		sol_ddi.num_clus--;

		/* First fix and free the cluster descripter */

		m = (struct mbuf *) super_header->rcv_tx.rcv_info.msave[0];
		m->m_type = MT_DATA;
		m->m_data = m->m_dat;
		m->m_next = NULL;
		m->m_flags = 0;
		m->m_nextpkt = NULL;
		m_free(m);

		/*  Next fix and free the mbufs and clusters */

		for (i=1 ; i<SOL_CMBUF_LEN + 1 ; i++) {
			m = super_header->rcv_tx.rcv_info.msave[i];
			m->m_next = NULL;
			m_free(m);
		}

		/*  Free the header */

		m_free((struct mbuf *) super_header);
	}
	return;
}

/*
 * NAME: sol_cio_query
 *
 * FUNCTION: Provides counter values accumulated by the device handler.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	ENODEV	- invalid minor number specified
 */

int
sol_cio_query(
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/
{
	cio_query_blk_t		qparms;
	int			rc;

	if (rc = MOVEIN(devflag, arg, &qparms, sizeof(cio_query_blk_t))) {
		return rc;
	}
	/*
	 *  Copy the stats to the user's buffer.
	 */
	if (rc = MOVEOUT(devflag, &sol_ddi.stats, qparms.bufptr,
	    MIN(sizeof(sol_query_stats_t), qparms.buflen))) {
		return rc;
	}
	/*
	 *  Clear the stats if requested.
	 */
	if (qparms.clearall == CIO_QUERY_CLEAR) {
		bzero((caddr_t) &sol_ddi.stats, sizeof(sol_query_stats_t));
	}
	qparms.status = CIO_OK;
	rc = MOVEOUT(devflag, &qparms, arg, sizeof(cio_query_blk_t));
	return rc;
}

/*
 * NAME: sol_cio_get_fastwrt
 *
 * FUNCTION: Provides address of fast write entry point.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * NOTES:
 *	This routine can only be called by a kernel-mode caller.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EACCES	- caller is not in kernel-mode
 *	ENODEV	- invalid minor number specified
 */

int
sol_cio_get_fastwrt(
dev_t	devno,			/* major/minor number 			*/
struct cio_get_fastwrt	*arg,	/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/
{
	if (!(devflag & DKERNEL))
		return EACCES;
	arg->fastwrt_fn = sol_fastwrt;
	arg->devno = devno;
	arg->chan = chan;
	return 0;
}

/*
 * NAME: sol_get_prids
 *
 * FUNCTION: Returns list of all processor IDs connected to SOL subsystem
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * NOTES:
 *	In case there are more ID's than the user supplies buffer space
 *	for, as many ID's as possible will be returned, and the num_ids
 *	field will contain the actual number detected.  Therefore the
 *	caller can determine how many additional IDs there were by
 *	subtracting the buffer size from the num_ids field (there is
 *	one byte per processor id).  If num_ids is 0 and there is no
 *	error returned, there were no processors detected.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	ENOMEM	- couldn't allocate memory
 *	ENODEV	- invalid minor number specified
 */

int
sol_get_prids(
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/
{
	struct sol_get_prids	prids_struct;
	caddr_t			buf;
	uint			prids;
	int			rc, rc2;
	uchar			i;

	if (rc = MOVEIN(devflag, arg, &prids_struct,
		sizeof(struct sol_get_prids))) {
		return rc;
	}
	/*
	 *  Malloc a buffer big enough to hold the max processor ids.
	 */
	buf = xmalloc(IMCS_PROC_LIMIT, 2, kernel_heap);
	if (buf == NULL) {
		return ENOMEM;
	}
	bzero(buf, prids_struct.buflen);
	/*
	 *  Move the reachable processor ids into the buffer (one byte each),
	 *  and count them.
	 */
	prids_struct.num_ids = 0;
	for (i=0 ; i<IMCS_PROC_LIMIT ; i++) {
		if (cck_proc[i] & PID_PRESENT) {
			*(caddr_t) ((uint)buf+(uint)prids_struct.num_ids++) = i;
		}
	}
	/*
	 *  Copy as much of the data as possible to the users buffer.
	 */
	rc = MOVEOUT(devflag, buf, (caddr_t) prids_struct.bufptr,
	    prids_struct.buflen);
	xmfree(buf, kernel_heap);
	if (rc == 0) {
		rc = MOVEOUT(devflag, &prids_struct, arg,
		    sizeof(struct sol_get_prids));
	} else {
		rc2 = MOVEOUT(devflag, &prids_struct, arg,
		    sizeof(struct sol_get_prids));
	}
	return rc;
}

/*
 * NAME: sol_check_prid
 *
 * FUNCTION: Returns successfully if the processor id is accessable.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * NOTES:
 *	The requested processor id should be in the low order byte of the
 *	arg parameter.  If the ioctl returns with no error, the processor
 *	is accessable.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0		- successful completion                        
 *	EINVAL		- invalid parameter
 *	ENOCONNECT	- the processor id is not accessable
 *	ENODEV		- invalid minor number specified
 */

int
sol_check_prid(
int	arg,			/* processor id to check		*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/
{
	uchar	pid;
	int	rc;

	pid = arg & 0x000000FF;
	if (pid > IMCS_PROC_LIMIT) {
		rc = EINVAL;
	} else if (cck_proc[pid] & PID_PRESENT) {
		rc = 0;
	} else {
		rc = ENOCONNECT;
	}
	return rc;
}

/*
 * NAME: sol_buffer_access
 *
 * FUNCTION:
 *	Provides access to the two SLA data buffers (2 X 256 byte buffers)
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	EPERM	- called after a normal open (not allowed)
 *	ENODEV	- invalid minor number specified
 *					 in 'result' field of passed structure)
 *	return code from copyin or copyout
 */

int
sol_buffer_access(
int	sla_num,		/* number of currently accessed sla */
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/

{
	int				rc;
	unsigned int			page_raddr;
	struct sol_buffer_access	access_struct;
	struct slaregs volatile		*sla_ptr;
	caddr_t				page_ptr;

	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) {
		return EPERM;
	}
	if (rc = MOVEIN(devflag, arg, &access_struct, sizeof(access_struct))) {
		return rc;
	}
	access_struct.result = SOL_SUCCESS;
	if (access_struct.buflen > 512) {	/* buffer length too long */
		return EINVAL;
	}

	ADDR_SLA(sla_num, sla_ptr);

	if (PUISTATE(sla_ptr->sample) != PUI_STOPPED) {
		rc = stop_sla(sla_ptr, &access_struct.status);
		if (rc) {
			read_registers(sla_ptr, &access_struct.status,
			    (uchar) TRUE);
			access_struct.result = SOL_CANT_STOP_SLA;
			rc = MOVEOUT(devflag, &access_struct, arg,
				sizeof(access_struct));
			UNADDR_SLA(sla_ptr);
			return EIO;
		}
	}

	/* xmalloc buffer to hold info to be written */
	page_ptr = xmalloc(512, 12, kernel_heap);
	if (page_ptr == NULL) {
		return ENOMEM;
	}

	page_raddr = imcslra(page_ptr);
	page_raddr |= 8;	/* set cache line to 8 for 512 bytes */

	/* clear status registers */
	access_struct.status.status_2 = sla_ptr->status2;
	access_struct.status.status_1 = sla_ptr->status1;

	/* start dma un-hang process   */
	rc = sla_dma_unhang(sla_ptr, page_raddr);
	if (rc != 0) {
		xmfree(page_ptr, kernel_heap);
		read_registers(sla_ptr, &access_struct.status, (uchar) TRUE);
		access_struct.result = SOL_DMA_UNHANG_FAILED;
		rc = MOVEOUT(devflag, &access_struct, arg,
		    sizeof(access_struct));
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	if (access_struct.flag == SOL_WRITE) {
		access_struct.status.ccr = BUFAC | PRIORITY | INITIATE |
			DRIVER | SOL_10_BIT_WRAP;
		if (rc = MOVEIN(devflag, access_struct.bufptr, page_ptr,
		    access_struct.buflen)) {
			xmfree(page_ptr, kernel_heap);
			UNADDR_SLA(sla_ptr);
			return rc;
		}
		vm_cflush(page_ptr, 512);
	} else {
		bzero(page_ptr, access_struct.buflen);
		vm_cflush(page_ptr, 512);
		access_struct.status.ccr = BUFAC | PRIORITY | DRIVER |
			SOL_10_BIT_WRAP;
	}

	rc = startup_chip(sla_ptr, &access_struct.status, page_raddr, sla_num);
	if (rc) {
		access_struct.result = rc;
		xmfree(page_ptr, kernel_heap);
		read_registers(sla_ptr, &access_struct.status, (uchar) FALSE);
		rc = MOVEOUT(devflag, &access_struct, arg,
		    sizeof(access_struct));
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	if (access_struct.flag == SOL_READ) {
		rc = MOVEOUT(devflag, page_ptr, access_struct.bufptr,
			access_struct.buflen);
		if (rc) {
			xmfree(page_ptr, kernel_heap);
			read_registers(sla_ptr, &access_struct.status,
			    (uchar) FALSE);
			rc = MOVEOUT(devflag, &access_struct, arg,
				sizeof(access_struct));
			UNADDR_SLA(sla_ptr);
			return rc;
		}
	}
	xmfree(page_ptr, kernel_heap);
	read_registers(sla_ptr, &access_struct.status, (uchar) FALSE);
	UNADDR_SLA(sla_ptr);
	rc = MOVEOUT(devflag, &access_struct, arg, sizeof(access_struct));
	return rc;
}

/*
 * NAME: sla_dma_unhang
 *
 * FUNCTION:
 *	Executes a DMA unhang and starts the channel
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_buffer_access (process environment),
 *	and it can page fault.
 *
 * NOTES:
 *	This routine can only be called by a user-mode caller.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion
 *	EIO	- SLA I/O exception interrupt encountered
 *	return code from sol_spin() call
 */

int
sla_dma_unhang(
struct slaregs volatile		*sla_ptr,
uint				page_raddr
)

{
	uint	rc;

	rc = 0;
	rc |= BUS_PUTLX((long *) &sla_ptr->thr.fr.link_ctl, DEVICE_CTL_DATA);
	rc |= BUS_PUTLX((long *) &sla_ptr->thr.fr.w3.dev_ctl, DEV_SEND | 31);
	rc |= BUS_PUTLX((long *) &sla_ptr->thr.fr.w4.B, 63);
	rc |= BUS_PUTLX((long *) &sla_ptr->ccr, PRIORITY | INITIATE | SXMT |
		AUTO_BUSY | DRIVER | DIAG_C);
	rc |= BUS_PUTLX((long *) &sla_ptr->tcw[0], page_raddr);
	rc |= BUS_PUTLX((long *) &sla_ptr->tcw[1], LAST_TAG);
	if (rc) {
		ASSERT(rc == EXCEPT_IO_SLA);
		return EIO;
	}

	/* start the channel */
	BUS_GETLX((long *) &sla_ptr->ch_start, &rc);
	rc = sol_spin(sla_ptr);
	return rc;
}

/*
 * NAME: sol_spin
 *
 * FUNCTION:
 *	Waits until the SLA goes to STOPPED or NO-OP state
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sla_dma_unhang (process environment), and
 *	it can page fault.
 *
 * NOTES:
 *	This routine can only be called by a user-mode caller.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EIO	- SLA never went to STOPPED or NO-OP state
 */

uint
sol_spin(
struct slaregs volatile		*sla_ptr
)

{
	int	num_spins;
	uint	rc;

	rc = sla_ptr->sample;

	num_spins = 0;
	while (!(rc & OP_DONE) && PUISTATE(rc) != PUI_STOPPED &&
		PUISTATE(rc) != PUI_NOOP && num_spins < 1000) {
		rc = sla_ptr->sample;
		num_spins++;
	}

	if (num_spins == 1000) {
		return EIO;
	}

  	/* get register value, else hardware will reject successive starts */
	rc = sla_ptr->status1;

	return 0;
}

/*
 * NAME: cancel_sla
 *
 * FUNCTION:
 *	Issues the channel cancel trigger command to the SLA
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_ols (process environment), and it
 *	can page fault.
 *
 * NOTES:
 *	This routine can only be called by a user-mode caller.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	-1	- cancel command failed
 */

int
cancel_sla(
struct slaregs volatile		*sla_ptr,
struct sol_sla_status		*status_struct
)

{
	uchar	stop_flag, counter;

	/* clear status2 register */
	status_struct->status_2 = sla_ptr->status2;

	counter = 0;
	stop_flag = FALSE;
	while ((counter < 10) && (!stop_flag)) {
		/* issue the channel cancel trigger command */
		status_struct->status_1 = sla_ptr->sample;
		switch (PUISTATE(status_struct->status_1)) {
			case PUI_NOOP:
				BUS_GETLX((long *) &sla_ptr->ch_op,
				    &status_struct->status_1);
				break;
			case PUI_STOPPED:
				stop_flag = TRUE;
				break;
			case PUI_WORK1:		/* normal case */
				BUS_GETLX((long *) &sla_ptr->ch_cancel,
				    &status_struct->status_1);
				break;
			case PUI_WORK2:
				BUS_GETLX((long *) &sla_ptr->ch_cancel,
				    &status_struct->status_1);
				break;
			default:
				break;
		}
		counter++;
	}
	if (counter == 10)
		return -1;
	else return 0;
}

/*
 * NAME: read_registers
 *
 * FUNCTION:
 *	Reads the status, config, CCR, receive and transmit registers from the
 *	SLA.  Clears the CCR, receive and transmit registers if an SLA I/O
 *	Exception Interrupt was encountered.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from various diagnostic ioctl functions (process
 *	environment), and it can page fault.
 *
 * NOTES:
 *	This routine can only be called by a user-mode caller.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	NONE
 */

void
read_registers( 
struct slaregs volatile		*sla_ptr,
struct sol_sla_status		*status_struct,
uchar				read_status_flag
)

{
	uint	rc;

	rc = 0;
	if (read_status_flag) {
		status_struct->status_2 = sla_ptr->status2;
		status_struct->status_1 = sla_ptr->status1;
	}
	status_struct->cfg_reg = sla_ptr->config;

	rc |= BUS_GETLX((long *) &sla_ptr->ccr, (long *) &status_struct->ccr);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[0],
	   (long *) &status_struct->rhr_word0);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[1],
	    (long *) &status_struct->rhr_word1);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[2],
	    (long *) &status_struct->rhr_word2);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[3],
	    (long *) &status_struct->rhr_word3);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[4],
	    (long *) &status_struct->rhr_word4);
	rc |= BUS_GETLX((long *) &sla_ptr->thr.thr[0],
	    (long *) &status_struct->thr_word0);
	rc |= BUS_GETLX((long *) &sla_ptr->thr.thr[1],
	    (long *) &status_struct->thr_word1);
	rc |= BUS_GETLX((long *) &sla_ptr->thr.thr[2],
	    (long *) &status_struct->thr_word2);
	rc |= BUS_GETLX((long *) &sla_ptr->thr.thr[3],
	    (long *) &status_struct->thr_word3);
	rc |= BUS_GETLX((long *) &sla_ptr->thr.thr[4],
	    (long *) &status_struct->thr_word4);
	if (rc) {	/* zero out restricted registers */
		ASSERT(rc == EXCEPT_IO_SLA);
		status_struct->ccr = 0;
		status_struct->rhr_word0 = 0;
		status_struct->rhr_word1 = 0;
		status_struct->rhr_word2 = 0;
		status_struct->rhr_word3 = 0;
		status_struct->rhr_word4 = 0;
		status_struct->thr_word0 = 0;
		status_struct->thr_word1 = 0;
		status_struct->thr_word2 = 0;
		status_struct->thr_word3 = 0;
		status_struct->thr_word4 = 0;
	}

	return;
}

/*
 * NAME: sol_sync_otp
 *
 * FUNCTION:
 *	Determines if the receive logic of an optics port can lock
 * 	on the transmit bit pattern.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	EPERM	- called after a normal open (not allowed)
 *	ENODEV	- invalid minor number specified
 */

int
sol_sync_otp(
int	sla_num,		/* number of currently accessed sla */
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/

{
	int				rc;
	uchar				counter;
	struct sol_diag_test		diag_struct;
	struct slaregs volatile		*sla_ptr;

	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) {
		return EPERM;
	}
	if (rc = MOVEIN(devflag, arg, &diag_struct, sizeof(diag_struct))) {
		return rc;
	}
	diag_struct.result = SOL_SUCCESS;

	ADDR_SLA(sla_num, sla_ptr);

	/* clear status registers */
	diag_struct.status.status_2 = sla_ptr->status2;
	diag_struct.status.status_1 = sla_ptr->status1;

	counter = 0;
	rc = 0;
	while ((diag_struct.status.status_2 & LINK_SIG_ERR) && (counter < 10)) {
		/* no light so pulse lock_to_xtal */
		rc |= pulse_lock_to_xtal(sla_ptr, diag_struct.diag_mode);

		/* clear status registers */
		diag_struct.status.status_2 = sla_ptr->status2;
		diag_struct.status.status_1 = sla_ptr->status1;

		/* status2 is read again to handle SLA hardware bug */
		diag_struct.status.status_2 = sla_ptr->status2;
		counter++;
	}
	if (rc && counter == 10) {
		ASSERT(rc == EXCEPT_IO_SLA);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		diag_struct.result = SOL_SLA_IO_EXCEPTION;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	if (counter == 10) {
		read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
		UNADDR_SLA(sla_ptr);
		diag_struct.result = SOL_SYNC_OTP_FAILED;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		return EIO;
	}
	read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
	UNADDR_SLA(sla_ptr);
	rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
	return rc;
}

/*
 * NAME: sol_lock_to_xtal
 *
 * FUNCTION:
 *	Executes a "lock to crystal" function to sync up the receive and
 *	transmit sections of the optics port
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * NOTES:
 *	This routine can only be called by a user-mode caller.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	EPERM	- called after a normal open (not allowed)
 *	ENODEV	- invalid minor number specified
 */

int
sol_lock_to_xtal(
int	sla_num,		/* number of currently accessed sla */
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/

{
	int				rc;
	struct sol_diag_test		diag_struct;
	struct slaregs volatile		*sla_ptr;
	uchar				counter;

	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) {
		return EPERM;
	}
	if (rc = MOVEIN(devflag, arg, &diag_struct, sizeof(diag_struct))) {
		return rc;
	}
	diag_struct.result = SOL_SUCCESS;

	ADDR_SLA(sla_num, sla_ptr);

	/* clear status registers */
	diag_struct.status.status_2 = sla_ptr->status2;
	diag_struct.status.status_1 = sla_ptr->status1;

	rc = 0;
	counter = 0;
	do {
		counter++;
		rc = pulse_lock_to_xtal(sla_ptr, diag_struct.diag_mode);
	} while (rc && (counter < 10));
	if ((counter == 10) && rc) {
		ASSERT(rc == EXCEPT_IO_SLA);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		diag_struct.result = SOL_SLA_IO_EXCEPTION;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
	UNADDR_SLA(sla_ptr);
	rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
	return rc;
}

/*
 * NAME: pulse_lock_to_xtal
 *
 * FUNCTION:
 *	Executes a lock_to_xtal pulse on SLA
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_lock_to_xtal and sol_sync_otp (process
 *	environment), and it can page fault.
 *
 * NOTES:
 *	This routine can only be called by a user-mode caller.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0		- successful completion 
 *	EXCEPT_IO_SLA	- SLA I/O Exception Interrupt encountered
 */

uint
pulse_lock_to_xtal(
struct slaregs volatile		*sla_ptr,
uchar				wrap_mode
)

{
	uint	temp;

	temp = 0;
	temp |= BUS_PUTLX((long *) &sla_ptr->ccr, DRIVER | LOCK_XTAL_ON_CODE);
	delay(HZ/50);	/* delay 1/50 of second */
	temp |= BUS_PUTLX((long *) &sla_ptr->ccr, DRIVER | LOCK_XTAL_OFF_CODE);
	temp |= BUS_PUTLX((long *) &sla_ptr->ccr, DRIVER | wrap_mode);
	delay(HZ/100);	/* delay 1/100 of second */
	return temp;
}

/*
 * NAME: sol_activate
 *
 * FUNCTION:
 *	Executes an "activate" command on the SLA which determines
 *	whether or not the SLA is sending and receiving the IDLE sequence
 *	correctly.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	EPERM	- called after a normal open (not allowed)
 *	ENODEV	- invalid minor number specified
 */

int
sol_activate(
int	sla_num,		/* number of currently accessed sla */
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/

{
	int				rc;
	unsigned int			page_address;
	struct sol_diag_test		diag_struct;
	struct slaregs volatile		*sla_ptr;
	struct sol_slabuf		*rw_buf;

	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) {
		return EPERM;
	}
	if (rc = MOVEIN(devflag, arg, &diag_struct, sizeof(diag_struct))) {
		return rc;
	}
	diag_struct.result = SOL_SUCCESS;
	
	ADDR_SLA(sla_num, sla_ptr);

	/* clear status registers */
	diag_struct.status.status_2 = sla_ptr->status2;
	diag_struct.status.status_1 = sla_ptr->status1;

	diag_struct.status.ccr = ACTIVATE | PRIORITY | INITIATE | RSTD | ISTP
				| DRIVER | RTV_1 | diag_struct.diag_mode;

	rc = startup_chip(sla_ptr, &diag_struct.status, (uint) NULL, sla_num);

	if (rc || ((PUISTATE(diag_struct.status.status_1) != PUI_STOPPED) ||
			(diag_struct.status.status_1 & LINK_CHK))) {
		read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
		UNADDR_SLA(sla_ptr);
		if (rc) {
			diag_struct.result = rc;
		} else {
			if (diag_struct.status.status_1 & LINK_CHK) {
				diag_struct.result =
					SOL_LINK_CHK_BIT_SET;
			} else {
				diag_struct.result = SOL_CANT_STOP_SLA;
			}
		}
		rc = MOVEOUT(devflag,&diag_struct,arg,sizeof(diag_struct));
		return EIO;
	}

	read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
	UNADDR_SLA(sla_ptr);

	rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
	return rc;
}

/*
 * NAME: sol_scr
 *
 * FUNCTION:
 *	Executes a "switch connection recovery" command on the SLA which
 *	sends a special 10-bit sequence.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	EPERM	- called after a normal open (not allowed)
 *	ENODEV	- invalid minor number specified
 */

int
sol_scr(
int	sla_num,		/* number of currently accessed sla */
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/

{
	int				rc;
	struct sol_diag_test		diag_struct;
	struct slaregs volatile		*sla_ptr;

	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) {
		return EPERM;
	}
	if (rc = MOVEIN(devflag, arg, &diag_struct, sizeof(diag_struct))) {
		return rc;
	}
	diag_struct.result = SOL_SUCCESS;
	
	ADDR_SLA(sla_num, sla_ptr);

	/* clear status registers */
	diag_struct.status.status_2 = sla_ptr->status2;
	diag_struct.status.status_1 = sla_ptr->status1;

	diag_struct.status.ccr = DOSCR | PRIORITY | INITIATE | RSTD | ISTP
				| DRIVER | AUTO_SCR | diag_struct.diag_mode;

	rc = startup_chip(sla_ptr, &diag_struct.status, (uint) NULL, sla_num);

	if (rc || !(diag_struct.status.status_1 & SCR_DONE)) {
		read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
		UNADDR_SLA(sla_ptr);
		if (rc) {
			diag_struct.result = rc;
		} else {
			diag_struct.result = SOL_SCR_NOT_COMPLETE;
		}
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		return EIO;
	}

	read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
	UNADDR_SLA(sla_ptr);
	rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
	return rc;
}

/*
 * NAME: sol_ols
 *
 * FUNCTION:
 *	Executes an "off-line sequence" command to the SLA which sends a
 *	special 10-bit sequence.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 * 
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	EPERM	- called after a normal open (not allowed)
 *	ENODEV	- invalid minor number specified
 *	ENOMEM	- malloc failed
 */

int
sol_ols(
int	sla_num,		/* number of currently accessed sla */
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/

{
	int				rc, counter;
	uint				real_addr;
	struct sol_diag_test		diag_struct;
	struct slaregs volatile		*sla_ptr;
	int				*offline_data_ptr;

	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) {
		return EPERM;
	}
	if (rc = MOVEIN(devflag, arg, &diag_struct, sizeof(diag_struct))) {
		return rc;
	}
	diag_struct.result = SOL_SUCCESS;
	
	ADDR_SLA(sla_num, sla_ptr);

	rc = stop_sla(sla_ptr, &diag_struct.status);

	if (PUISTATE(sla_ptr->status1) != PUI_STOPPED) {
		cancel_sla(sla_ptr, &diag_struct.status);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		UNADDR_SLA(sla_ptr);
		diag_struct.result = SOL_SLA_NOT_STOPPED;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		return EIO;
	}

	offline_data_ptr = xmalloc(sizeof(offline_seq_data), 12, kernel_heap);
	if (offline_data_ptr == NULL) {
		cancel_sla(sla_ptr, &diag_struct.status);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		UNADDR_SLA(sla_ptr);
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		return ENOMEM;
	}

	bcopy(offline_seq_data, offline_data_ptr, sizeof(offline_seq_data));
	vm_cflush(offline_data_ptr, sizeof(offline_seq_data));

	/* clear status registers */
	diag_struct.status.status_2 = sla_ptr->status2;
	diag_struct.status.status_1 = sla_ptr->status1;

	real_addr = imcslra(offline_data_ptr);
	real_addr |= 1; /* length = 1 cache line */
	rc = 0;
	for (counter = 0; counter < NUM_TCWS - 1; counter++) {
		rc |= BUS_PUTLX((long *) &sla_ptr->tcw[counter], real_addr);
	}
	rc |= BUS_PUTLX((long *) &sla_ptr->tcw[NUM_TCWS - 1], LAST_TAG);
	rc |= BUS_PUTLX((long *) &sla_ptr->ccr, TRANSPARENT | PRIORITY | DRTO |
		CS_XMT | AUTO_BUSY | RSTD | DRIVER | diag_struct.diag_mode);
	if (rc) {
		ASSERT(rc == EXCEPT_IO_SLA);
		cancel_sla(sla_ptr, &diag_struct.status);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		UNADDR_SLA(sla_ptr);
		diag_struct.result = SOL_SLA_IO_EXCEPTION;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		xmfree(offline_data_ptr, kernel_heap);
		return EIO;
	}

	if (PUISTATE(sla_ptr->sample) == PUI_STOPPED) {
		BUS_GETLX((long *) &sla_ptr->ch_start,
		    &diag_struct.status.status_1);
	}

	counter = 0;
	do {
		diag_struct.status.status_2 = sla_ptr->status2;
		counter++;
	} while (!(diag_struct.status.status_2 & OFFSQ_REC)
		&& (counter < 1000));
	if (counter == 1000) {
		cancel_sla(sla_ptr, &diag_struct.status);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		UNADDR_SLA(sla_ptr);
		diag_struct.result = SOL_OFFL_SEQ_NOT_REC;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		xmfree(offline_data_ptr, kernel_heap);
		return EIO;
	}

	diag_struct.status.status_1 = sla_ptr->status1;
	if (!(diag_struct.status.status_1 & LINK_CHK)) {
		cancel_sla(sla_ptr, &diag_struct.status);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		UNADDR_SLA(sla_ptr);
		diag_struct.result = SOL_NO_LINK_CHK_BIT;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		xmfree(offline_data_ptr, kernel_heap);
		return EIO;
	}

	rc = cancel_sla(sla_ptr, &diag_struct.status);
	read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
	xmfree(offline_data_ptr, kernel_heap);
	UNADDR_SLA(sla_ptr);
	if (rc) {
		diag_struct.result = SOL_CANT_STOP_SLA;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		return EIO;
	}

	rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
	return rc;
}

/*
 * NAME: sol_rhr
 *
 * FUNCTION:
 *	Fills all receive header register bits with 1's and sends a Link
 *	Control Frame with data in the information field.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	EPERM	- called after a normal open (not allowed)
 *	ENODEV	- invalid minor number specified
 */

int
sol_rhr(
int	sla_num,		/* number of currently accessed sla */
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/

{
	int				rc,i;
	struct sol_diag_test		diag_struct;
	struct slaregs volatile		*sla_ptr;

	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) {
		return EPERM;
	}
	if (rc = MOVEIN(devflag, arg, &diag_struct, sizeof(diag_struct))) {
		return rc;
	}
	diag_struct.result = SOL_SUCCESS;
	
	ADDR_SLA(sla_num, sla_ptr);

	/* clear status registers */
	diag_struct.status.status_2 = sla_ptr->status2;
	diag_struct.status.status_1 = sla_ptr->status1;

	/* set RHR bits to all ones */
	for (i = 0; i < 5; i++) {
		rc = BUS_PUTLX((long *) &sla_ptr->prhr.prhr[i], 0xffffffff);
		if (rc) {
			ASSERT(rc == EXCEPT_IO_SLA);
			read_registers(sla_ptr, &diag_struct.status,
			    (uchar) TRUE);
			for (i = 0; i < 5; i++) {
				sla_ptr->prhr.prhr[i] = sla_ptr->thr.thr[i] = 0;
			}
			diag_struct.result = SOL_SLA_IO_EXCEPTION;
			rc = MOVEOUT(devflag, &diag_struct, arg,
			    sizeof(diag_struct));
			UNADDR_SLA(sla_ptr);
			return EIO;
		}
	}

	/* set up transmit registers and ccr to send link control frame
	in wrap mode with arbitrary test data */
	rc = 0;
	rc |= BUS_PUTLX((long *) &diag_struct.status.ccr, NATIVE | PRIORITY |
	    INITIATE|SOF_CON|EOF_PAS|ISTP|DRIVER|diag_struct.diag_mode);
	rc |= BUS_PUTLX((long *) &diag_struct.status.thr_word0, 0xC000C100);
	rc |= BUS_PUTLX((long *) &diag_struct.status.thr_word1,
	    LINK_REQUEST_TEST);
	rc |= BUS_PUTLX((long *) &diag_struct.status.thr_word4, 8);
	rc |= BUS_PUTLX((long *) &diag_struct.status.thr_word2, 0x01234567);
	rc |= BUS_PUTLX((long *) &diag_struct.status.thr_word3, 0x89ABCDEF);
	if (rc) {
		ASSERT(rc == EXCEPT_IO_SLA);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		for (i = 0; i < 5; i++) {
			sla_ptr->prhr.prhr[i] = sla_ptr->thr.thr[i] = 0;
		}
		diag_struct.result = SOL_SLA_IO_EXCEPTION;
		rc = MOVEOUT(devflag, &diag_struct, arg,
			sizeof(diag_struct));
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	rc = startup_chip(sla_ptr, &diag_struct.status, (uint) NULL, sla_num);
	if (rc) {
		read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
		for (i = 0; i < 5; i++) {
			sla_ptr->prhr.prhr[i] = sla_ptr->thr.thr[i] = 0;
		}
		diag_struct.result = rc;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	/* read RHR from hardware */
	rc = 0;
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[0],
	    (long *) &diag_struct.status.rhr_word0);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[1],
	    (long *) &diag_struct.status.rhr_word1);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[2],
	    (long *) &diag_struct.status.rhr_word2);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[3],
	    (long *) &diag_struct.status.rhr_word3);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[4],
	    (long *) &diag_struct.status.rhr_word4);
	if (rc) {
		ASSERT(rc == EXCEPT_IO_SLA);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		for (i = 0; i < 5; i++) {
			sla_ptr->prhr.prhr[i] = sla_ptr->thr.thr[i] = 0;
		}
		diag_struct.result = SOL_SLA_IO_EXCEPTION;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		UNADDR_SLA(sla_ptr);
		return EIO;
	}


	if ((bcmp(&diag_struct.status.rhr_word0,
		&diag_struct.status.thr_word0,16)) ||
		((diag_struct.status.rhr_word4 & 0x00ffffff) != 0x8)) {
		diag_struct.result = SOL_RHR_COMPARE_FAILED;
		read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
		for (i = 0; i < 5; i++) {
			sla_ptr->prhr.prhr[i] = sla_ptr->thr.thr[i] = 0;
		}
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	if (!(diag_struct.status.status_1 & UNEXPD) ||
	    !(diag_struct.status.status_2 & ADDR_MIS)) {
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		for (i = 0; i < 5; i++) {
			sla_ptr->prhr.prhr[i] = sla_ptr->thr.thr[i] = 0;
		}
		if (!(diag_struct.status.status_1 & UNEXPD)) {
			diag_struct.result = SOL_UNEXPD_FRAME_NOT_SET;
		} else {
			diag_struct.result = SOL_ADDR_MIS_NOT_SET;
		}
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
	for (i = 0; i < 5; i++) {
		sla_ptr->prhr.prhr[i] = sla_ptr->thr.thr[i] = 0;
	}
	UNADDR_SLA(sla_ptr);

	rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
	return rc;
}

/*
 * NAME: sol_crc
 *
 * FUNCTION:
 *	Sends a Link Control Frame in SLA Transparent Mode with an
 *	intentionally invalid CRC word.  The CRC is then verified.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EINVAL	- invalid parameter
 *	EFAULT	- invalid address specified
 *	EIO	- error occured, see the status field for more info
 *	EPERM	- called after a normal open (not allowed)
 *	ENODEV	- invalid minor number specified
 */

int
sol_crc(
int	sla_num,		/* number of currently accessed sla */
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/

{
	int				rc;
	struct sol_diag_test		diag_struct;
	struct slaregs volatile		*sla_ptr;
	struct xmem			ala_d;
	int				*ala_mode_buffer;
	unsigned int			page_address;

	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) {
		return EPERM;
	}
	if (rc = MOVEIN(devflag, arg, &diag_struct, sizeof(diag_struct))) {
		return rc;
	}
	diag_struct.result = SOL_SUCCESS;
	
	ADDR_SLA(sla_num, sla_ptr);

	/* clear status registers */
	diag_struct.status.status_2 = sla_ptr->status2;
	diag_struct.status.status_1 = sla_ptr->status1;

	ala_mode_buffer = (int *) xmalloc(sizeof(ala_mode_data),12,pinned_heap);
	if (ala_mode_buffer == NULL) {
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		UNADDR_SLA(sla_ptr);
		return ENOMEM;
	}

	bcopy(ala_mode_data, ala_mode_buffer, sizeof(ala_mode_data));
	vm_cflush(ala_mode_buffer, sizeof(ala_mode_data));

	rc = 0;
	page_address = imcslra(ala_mode_buffer);
	page_address |= 1;	/* set cache line to 1 for 64 bytes */
	rc |= BUS_PUTLX((long *) &sla_ptr->tcw[0], page_address);
	rc |= BUS_PUTLX((long *) &sla_ptr->tcw[1], LAST_TAG);
	/* set up the ccr for transparent mode to send ala frame. */
	/* SOF_PAS = xmt from run state, EOF_PAS = sft */
	rc |= BUS_PUTLX((long *) &diag_struct.status.ccr, TRANSPARENT|PRIORITY|
		INITIATE | SXMT | DRTO | SOF_PAS | EOF_PAS | RSTD | ISTP |
		DRIVER | diag_struct.diag_mode);
	/* set THR S_ID to FF00 to match D_ID of RHR so we won't cause a link */
	/* address mismatch. */
	rc |= BUS_PUTLX((long *) &diag_struct.status.thr_word0, 0x0000FF00);
	if (rc) {
		ASSERT(rc == EXCEPT_IO_SLA);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		diag_struct.result = SOL_SLA_IO_EXCEPTION;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		xmfree(ala_mode_buffer, pinned_heap);
		UNADDR_SLA(sla_ptr);
		return EIO;
	}


	rc = startup_chip(sla_ptr, &diag_struct.status, page_address, sla_num);
	if (rc) {
		read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
		diag_struct.result = rc;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		xmfree(ala_mode_buffer, pinned_heap);
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	/* read RHR from hardware */
	rc = 0;
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[0],
	    (long *) &diag_struct.status.rhr_word0);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[1],
	    (long *) &diag_struct.status.rhr_word1);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[2],
	    (long *) &diag_struct.status.rhr_word2);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[3],
	    (long *) &diag_struct.status.rhr_word3);
	rc |= BUS_GETLX((long *) &sla_ptr->prhr.prhr[4],
	    (long *) &diag_struct.status.rhr_word4);
	if (rc) {
		ASSERT(rc == EXCEPT_IO_SLA);
		read_registers(sla_ptr, &diag_struct.status, (uchar) TRUE);
		diag_struct.result = SOL_SLA_IO_EXCEPTION;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		xmfree(ala_mode_buffer, pinned_heap);
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	if (!(diag_struct.status.status_1 & UNEXPD) ||
	    !(diag_struct.status.status_1 & LINK_CHK)) {
		read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
		if (!(diag_struct.status.status_1 & UNEXPD)) {
			diag_struct.result = SOL_UNEXPD_FRAME_NOT_SET;
		} else {
			diag_struct.result = SOL_NO_LINK_CHK_BIT;
		}
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		xmfree(ala_mode_buffer, pinned_heap);
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	/* check crc_check bit */
	if (!(diag_struct.status.rhr_word4 & SOL_CRC_CHECK)) {
		read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
		diag_struct.result = SOL_CRC_CHK_BIT_NOT_SET;
		rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
		xmfree(ala_mode_buffer, pinned_heap);
		UNADDR_SLA(sla_ptr);
		return EIO;
	}

	xmfree(ala_mode_buffer, pinned_heap);
	read_registers(sla_ptr, &diag_struct.status, (uchar) FALSE);
	UNADDR_SLA(sla_ptr);
	rc = MOVEOUT(devflag, &diag_struct, arg, sizeof(diag_struct));
	return rc;
}

/*
 * NAME: sol_card_present
 *
 * FUNCTION:
 *	Determines whether or not an optic two-port card is connected to
 *	the SLA.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from sol_ioctl (process environment), and it
 *	can page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0		- successful completion                        
 *	EINVAL		- invalid parameter
 *	EFAULT		- invalid address specified
 *	ENOCONNECT	- optic two-port card is not connected
 *	EIO		- error occured, see the status field for more info
 *	EPERM		- called after a normal open (not allowed)
 *	ENODEV		- invalid minor number specified
 */

int
sol_card_present(
int	sla_num,		/* number of currently accessed sla */
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/

{
	int				rc;
	uint				temp;
	struct sol_diag_test		diag_struct;
	struct slaregs volatile		*sla_ptr;

	if ((sol_ddi.chan_state[chan] == SOL_CH_NORM) ||
	    (sol_ddi.chan_state[chan] == SOL_CH_SNORM)) {
		return EPERM;
	}
	
	ADDR_SLA(sla_num, sla_ptr);

	temp = sla_ptr->status2;

	UNADDR_SLA(sla_ptr);

	if (temp & NO_OPCARD) {
		return ENOCONNECT;
	} else {
		return 0;
	}

}
