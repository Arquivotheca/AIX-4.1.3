static char sccsid[] = "@(#)50	1.4  src/bos/kernext/sol/sol_write.c, sysxsol, bos411, 9428A410j 6/11/91 07:32:36";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  sol_write
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
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include <sys/soluser.h>
#include "soldd.h"

extern struct sol_ddi	sol_ddi;

/*
 * NAME: sol_write
 *
 * FUNCTION: Provides means for transmitting data.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from the process environment, and it can
 *	page fault.                   
 *
 * NOTES:
 *	For a user-mode caller the data is in a uio structure and must be
 *	copied into mbufs.  For a kernel caller, the data is already in
 *	mbufs.  Both this routine and the sol_fastwrt routine call
 *	sol_com_write.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	ENODEV		- invalid minor number
 *	ENETDOWN	- network is down, can't transmit
 *	ENOCONNECT	- device has not been started
 *	EAGAIN		- transmit queue is full
 *	EINVAL		- invalid parameter specified
 *	ENOMEM		- couldn't malloc memory required
 *	EINTR		- system call was interrupted
 *	EFAULT		- invalid address specified
 *	EIO		- error occured, see status field for more info
 *	0		- successful completion                        
 */

int
sol_write(
dev_t			devno,	/* major/minor device number		*/
struct uio		*uiop,	/* pointer to uio struct 		*/
chan_t			chan,	/* mpx channel number			*/
struct write_extension	*ext)	/* NULL, or pointer to write_extension	*/
{

	cio_write_ext_t		write_ext;
	struct sol_open_struct	*open_ptr;
	struct super_header	*super_header;
	struct mbuf		*m, *mbufp, *hdrp, *tail, *new_mptr;
	struct imcs_header	*imcs_hdr;
	struct cl_desc		*cl_desc;
	uchar			havecluster;
	uint			total_bytes, imcs_addr;
	int			rc;

	SYS_SOL_TRACE(WRITE_ENTRY, uiop, uiop->uio_iov->iov_base, ext);
	new_mptr = NULL;
	if ((uint) chan >= (uint) SOL_TOTAL_OPENS) {
		SYS_SOL_TRACE(WRITE_EXIT, ENODEV, chan, 0);
		return ENODEV;
	}
	open_ptr = sol_ddi.open_ptr[chan];
	if (open_ptr == NULL) {
		SYS_SOL_TRACE(WRITE_EXIT, ENODEV, chan, 0);
		return ENODEV;
	}
	if (minor(devno) != SOL_OPS_MINOR) {
		SYS_SOL_TRACE(WRITE_EXIT, ENODEV, chan, 0);
		return ENODEV;
	}
	if (ext != NULL) {
		if (rc = MOVEIN(open_ptr->devflag, ext, &write_ext,
		    sizeof(struct write_extension))) {
			SYS_SOL_TRACE(WRITE_EXIT, rc, chan, 0);
			return rc;
		}
	}

	if (open_ptr->num_netids == 0) {
		if (ext != NULL) {
			write_ext.status = CIO_NOT_STARTED;
			rc = MOVEOUT(open_ptr->devflag, &write_ext, ext,
			    sizeof(struct write_extension));
			SYS_SOL_TRACE(WRITE_EXIT, EIO, chan, 0);
			return EIO;
		} else {
			SYS_SOL_TRACE(WRITE_EXIT, ENOCONNECT, chan, 0);
			return ENOCONNECT;
		}
	}
	havecluster = 0;
	if (open_ptr->devflag & DKERNEL) { /* kernel caller */
		/*
		 *  For a kernel-mode caller, the data comes in a single
		 *  mbuf chain, but it may not be in a shape that we
		 *  we can transmit it.  sol_check_mbufs will check
		 *  this and return a new chain if necessary.
		 */
		mbufp = (struct mbuf *)(uiop->uio_iov->iov_base);

		rc = sol_check_mbufs(mbufp, &new_mptr, &havecluster,
		    open_ptr->devflag & DNDELAY, &total_bytes);
		if ((rc == SOL_PKT_TOO_LONG) ||		/* packet too long */
		    (rc == SOL_BAD_ALLIGNMENT)) {	/* not 4K cluster  */
			if (ext != NULL) {
				write_ext.status = CIO_BAD_RANGE;
				rc = MOVEOUT(open_ptr->devflag, &write_ext,
				    ext, sizeof(struct write_extension));
				SYS_SOL_TRACE(WRITE_EXIT, EIO, chan, 0);
				return EIO;
			} else {
				SYS_SOL_TRACE(WRITE_EXIT, EINVAL, chan, 0);
				return EINVAL;
			}
		} else if (rc == SOL_NO_CLUSTER) {
			/* couldn't get cluster for collapse */
			if (ext != NULL) {
				write_ext.status = CIO_NOMBUF;
				rc = MOVEOUT(open_ptr->devflag, &write_ext,
				    ext, sizeof(struct write_extension));
				SYS_SOL_TRACE(WRITE_EXIT, EIO, chan, 0);
				return EIO;
			} else {
				SYS_SOL_TRACE(WRITE_EXIT, ENOMEM, chan, 0);
				return ENOMEM;
			}
		}
	} else { /* user-mode caller */
		/*
		 *  For a user-mode caller, the data comes in the uio
		 *  structure.  So the data is moved to mbufs here, and
		 *  we make sure the mbufs are in the right shape for
		 *  a transmit.  Basically, if the data won't fit in
		 *  a single small mbuf, it is put in a cluster chain.
		 */
		total_bytes = uiop->uio_resid;
		if (total_bytes > SOL_MAX_XMIT) {
			if (ext != NULL) {
				write_ext.status = CIO_BAD_RANGE;
				rc = MOVEOUT(open_ptr->devflag, &write_ext,
				    ext, sizeof(struct write_extension));
				SYS_SOL_TRACE(WRITE_EXIT, EIO, chan, 0);
				return EIO;
			} else {
				SYS_SOL_TRACE(WRITE_EXIT, EINVAL, chan, 0);
				return EINVAL;
			}
		}
		if (total_bytes <= MLEN) {
			/*
			 *  The data will all fit in a single small mbuf.
			 */
			if (open_ptr->devflag & DNDELAY) {
				mbufp = m_get(M_DONTWAIT, MT_DATA);
				if (mbufp == NULL) {
					if (ext != NULL) {
						write_ext.status = CIO_NOMBUF;
						rc = MOVEOUT(open_ptr->devflag,
						    &write_ext, ext, sizeof(
						    struct write_extension));
						SYS_SOL_TRACE(WRITE_EXIT,
						    EIO, chan, 0);
						return EIO;
					} else {
						SYS_SOL_TRACE(WRITE_EXIT,
						    ENOMEM, chan, 0);
						return ENOMEM;
					}
				}
			} else {
				mbufp = m_get(M_WAIT, MT_DATA);
			}
			mbufp->m_len = total_bytes;
			if (rc = uiomove(MTOD(mbufp,uchar *), mbufp->m_len,
			    UIO_WRITE, uiop)) {
				m_free(mbufp);
				if (ext != NULL) {
					write_ext.status = CIO_BAD_RANGE;
					rc = MOVEOUT(open_ptr->devflag,
					    &write_ext, ext,
					    sizeof(struct write_extension));
					SYS_SOL_TRACE(WRITE_EXIT, EIO, chan, 0);
					return EIO;
				} else {
					SYS_SOL_TRACE(WRITE_EXIT, EFAULT,
					    chan, 0);
					return EFAULT;
				}
			}
		} else {
			/*
			 *  The data won't fit in a small mbuf, so send
			 *  all clusters instead.
			 */
			mbufp = NULL;
			tail = NULL;
			while (uiop->uio_resid) {
				if (open_ptr->devflag & DNDELAY) {
					m = m_getclust(M_DONTWAIT,MT_DATA);
					if (m == NULL) {
						if (mbufp != NULL) {
							m_freem(mbufp);
						}
						if (ext != NULL) {
							write_ext.status =
							    CIO_NOMBUF;
							rc = MOVEOUT(open_ptr->
							    devflag, &write_ext,
							    ext, sizeof(struct
							    write_extension));
							SYS_SOL_TRACE(
							    WRITE_EXIT, EIO,
							    chan, 0);
							return EIO;
						} else {
							SYS_SOL_TRACE(
							    WRITE_EXIT, ENOMEM,
							    chan, 0);
							return ENOMEM;
						}
					}
				} else {
					m = m_getclust(M_WAIT, MT_DATA);
				}
				if (tail != NULL) {
					tail->m_next = m;
					tail = m;
				} else {
					mbufp = m;
					tail = m;
				}
				havecluster = 1;
				tail->m_len = MIN(CLBYTES, uiop->uio_resid);
				if (rc = uiomove (MTOD(tail, uchar *),
				    tail->m_len, UIO_WRITE, uiop)) {
					m_freem(mbufp);
					if (ext != NULL) {
						write_ext.status=CIO_BAD_RANGE;
						rc = MOVEOUT(open_ptr->devflag,
						    &write_ext, ext,
						    sizeof(struct
						    write_extension));
						SYS_SOL_TRACE(WRITE_EXIT, EIO,
						    chan, 0);
						return EIO;
					} else { 
						SYS_SOL_TRACE(WRITE_EXIT,
						    EFAULT, chan, 0);
						return EFAULT;
					}
				}
			}
		}
	}
	/*
	 *  Check the processor id from the packet header.
	 */
	imcs_addr =  *(uint *)(MTOD(mbufp, uint) + SOL_PROC_OFFSET);
	if ((imcs_addr < 1) || (imcs_addr >= IMCS_PROC_LIMIT)) {
		if (!(open_ptr->devflag & DKERNEL)) {
			m_freem(mbufp);
		}
		if (ext != NULL) {
			write_ext.status = CIO_BAD_RANGE;
			rc = MOVEOUT(open_ptr->devflag, &write_ext,
			    ext, sizeof(struct write_extension));
			SYS_SOL_TRACE(WRITE_EXIT, EIO, chan, 0);
			return EIO;
		} else {
			SYS_SOL_TRACE(WRITE_EXIT, EINVAL, chan, 0);
			return EINVAL;
		}
	}

	/*
	 *  Since the fast write path can not wait for mbufs, we will do
	 *  the mget's for the IMCS header and cluster descripter here,
	 *  in case DNDELAY is not set, we will be able to wait here.
	 */
	if (open_ptr->devflag & DNDELAY) {
		super_header = (struct super_header *)
		    m_get(M_DONTWAIT, MT_DATA);
		if (super_header == NULL) {
			if (!(open_ptr->devflag & DKERNEL)) {
				m_freem(mbufp);
			}
			if (ext != NULL) {
				write_ext.status = CIO_NOMBUF;
				rc = MOVEOUT(open_ptr->devflag, &write_ext,
				    ext, sizeof(struct write_extension));
				SYS_SOL_TRACE(WRITE_EXIT, EIO, chan, 0);
				return EIO;
			} else {
				SYS_SOL_TRACE(WRITE_EXIT, ENOMEM, chan, 0);
				return ENOMEM;
			}
		}
	} else {
		super_header = (struct super_header *) m_get(M_WAIT, MT_DATA);
	}
	imcs_hdr = &super_header->imcs_header;
	if (havecluster) {
		if (open_ptr->devflag & DNDELAY) {
			cl_desc = (struct cl_desc *) m_getclr(M_DONTWAIT,
			    MT_DATA);
			if (cl_desc == NULL) {
				m_free((struct mbuf *) super_header);
				if (!(open_ptr->devflag & DKERNEL)) {
					m_freem(mbufp);
					if (new_mptr) {
						m_freem(new_mptr);
					}
				}
				if (ext != NULL) {
					write_ext.status = CIO_NOMBUF;
					rc = MOVEOUT(open_ptr->devflag,
					    &write_ext, ext,
					    sizeof(struct write_extension));
					SYS_SOL_TRACE(WRITE_EXIT, EIO, chan, 0);
					return EIO;
				} else {
					SYS_SOL_TRACE(WRITE_EXIT, ENOMEM,
					    chan, 0);
					return ENOMEM;
				}
			}
		} else {
			cl_desc = (struct cl_desc *) m_getclr(M_WAIT, MT_DATA);
		}
	} else {
		cl_desc = NULL;
	}
	/*  
	 *  If there is a new mbuf chain for this xmit, we will
	 *  free the original chain here (unless NOFREE_MBUF is set),
	 *  and set the flag so that sol_xmit_done will free the new
	 *  chain.  If there is NOT a new chain, we will simply set
	 *  the flag to match what is in the extension.
	 */
	super_header->rcv_tx.tx_info.small_mbuf=NULL; /* assume no small copy*/
	super_header->rcv_tx.tx_info.orig_mbuf=NULL;  /* assume no collapse */
	if (new_mptr) {
		if (ext) {
			/*
			 *  We need to save the address of the original mbuf
			 *  chain for the TX_DONE notification, even if we
			 *  free the mbuf.
			 */
			if (havecluster & SOL_SMALL_COPY) {
				super_header->rcv_tx.tx_info.small_mbuf=mbufp;
			} else {
				super_header->rcv_tx.tx_info.orig_mbuf=mbufp;
			}
			if (!(ext->flag & CIO_NOFREE_MBUF)) {
				m_freem(mbufp);
			}
			super_header->rcv_tx.tx_info.chan = chan;
			super_header->rcv_tx.tx_info.flags = ext->flag &
			    ~CIO_NOFREE_MBUF;
			super_header->rcv_tx.tx_info.write_id = ext->write_id;
		} else {
			m_freem(mbufp);
			super_header->rcv_tx.tx_info.chan = chan;
			super_header->rcv_tx.tx_info.flags = (ulong) 0;
			super_header->rcv_tx.tx_info.write_id = (ulong) 0;
		}
		mbufp = new_mptr;
	} else {
		if (ext) {
			super_header->rcv_tx.tx_info.chan = chan;
			super_header->rcv_tx.tx_info.flags = ext->flag;
			super_header->rcv_tx.tx_info.write_id = ext->write_id;
		} else {
			super_header->rcv_tx.tx_info.chan = chan;
			super_header->rcv_tx.tx_info.flags = (ulong) 0;
			super_header->rcv_tx.tx_info.write_id = (ulong) 0;
		}
	}

	/*
	 *  Save the message length to update the statistics in xmit_offlevel.
	 */
	super_header->rcv_tx.tx_info.msglen = total_bytes;

	sol_com_write(mbufp, super_header, cl_desc, open_ptr);
	if (ext != NULL) {
		write_ext.status = CIO_OK;
		rc = MOVEOUT(open_ptr->devflag, &write_ext, ext,
		    sizeof(struct write_extension));
	}
	/*
	 *  Before returning, clear out the uio_offset field to prevent
	 *  over-running the counter.
	 */
	uiop->uio_offset = 0;
	SYS_SOL_TRACE(WRITE_EXIT, 0, 0, 0);
	return 0;
}
