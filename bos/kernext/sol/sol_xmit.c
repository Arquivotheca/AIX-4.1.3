static char sccsid[] = "@(#)51	1.9  src/bos/kernext/sol/sol_xmit.c, sysxsol, bos411, 9428A410j 8/31/92 10:31:32";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  sol_fastwrt, sol_check_mbufs, sol_collapse, sol_com_write,
 *	       sol_xmit_notify, sol_xmit_offlevel
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

struct sol_ddi	sol_ddi = { LOCK_AVAIL };
extern uchar	cck_proc[];

/*
 *			GENERAL DESCRIPTION OF TRANSMIT
 *
 *  The toughest part about transmitting is getting the data in one of the
 *  two acceptable forms for transmission:
 *
 *	1)  A chain of up to 4 small mbufs with no clusters.
 *	2)  A chain of up to 15 clusters with a small mbuf-sized
 *	    cluster_descripter on the front (see receive description for
 *	    an explanation of a cluster_descripter).
 *
 *  Basically if the data did not come to us in one of these two forms,
 *  sol_collapse will be called to build a cluster chain and move the
 *  data into this new chain.
 *
 *  Here is the basic flow:
 *	sol_write - 	for a user-mode caller, the data is copied into mbufs
 *			so we are guaranteed that it is in one of the
 *			acceptable forms for transmission.
 *		  - 	for a kernel-mode caller, sol_check_mbufs is called to
 *			see if the data is ok.  If not, sol_collapse is called
 *			to move the data to an acceptable cluster chain.
 *	sol_fastwrt -	this is exactly the same as the kernel-mode caller
 *			of sol_write, except there is less error checking.
 *	sol_com_write -	Both sol_write and sol_fastwrt call this routine
 *			for a single packet.   This routine maps the
 *			buffers for dma, builds the cluster_descripter for
 *			a cluster chain, and calls imcs_sendmsg to do the
 *			actual transmit.
 *	xmit_notify   -	when imcs receives the ACK for the transmit, this
 *			routine is called and schedules the offlevel routine.
 *	xmit_offlevel -	this offlevel routine is called to free the mbufs
 *			and send the acknowledgement to the user(if requested).
 */

/*
 * NAME: sol_fastwrt
 *
 * FUNCTION: Provides means for transmitting data.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from either the process environment or the
 *	interrupt environment, and it therefore can not page fault.
 *
 * NOTES:
 *
 *	This routine is called from a kernel user directly.  It can
 *	be called from the off_level and therefore can not sleep.
 *	This routine makes some assumptions based on the fact that 
 *	this is only called by a trusted kernel-mode caller, and
 *	some checking is not done.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	ENODEV		- invalid minor number
 *	ENETDOWN	- network is down, cannot transmit
 *	ENOCONNECT	- device has not been started
 *	EAGAIN		- transmit queue is full
 *	EINVAL		- invalid parameter
 *	ENOMEM		- unable to allocate necessary memory
 *	EFAULT		- invalid address
 *	EIO		- an error occured
 *	0		- successful completion                        
 */

int
sol_fastwrt(
struct mbuf		*mbufp,	/* pointer to mbuf chain		*/
chan_t			chan)	/* mpx channel number			*/
{

	struct mbuf		*mptr, *new_mptr;
	struct cl_desc 		*cl_desc;
	struct sol_open_struct	*open_ptr;
	struct imcs_header	*imcs_header;
	struct super_header	*super_header;
	uchar			havecluster;
	int			rc;
	uint			msglen;


	SYS_SOL_TRACE(FASTWRT_ENTRY, mbufp, 0, 0);
	cl_desc = NULL;
	new_mptr = NULL;
	ASSERT((uint) chan < (uint) SOL_TOTAL_OPENS);

	open_ptr = sol_ddi.open_ptr[chan];
	ASSERT(open_ptr != NULL);

	ASSERT(open_ptr->num_netids != 0);
	/*
	 *  Get an mbuf for the imcs header.
	 */

	mptr = m_get(M_DONTWAIT, MT_DATA);
	if (mptr == NULL) {
		SYS_SOL_TRACE(FASTWRT_EXIT, ENOMEM, 0, 0);
		return ENOMEM;
	}
	super_header = (struct super_header *) mptr;
	imcs_header = &super_header->imcs_header;

	/*
	 *  sol_check_mbufs checks the length of the packet and
	 *  builds a new chain if necessary so we will be able
	 *  to transmit it.
	 */
	rc = sol_check_mbufs(mbufp,&new_mptr,&havecluster,DNDELAY,&msglen);
	if (rc != 0) {
		/*
		 *  Free the imcs_header.
		 */
		m_free((struct mbuf *)super_header);
		if ((rc == SOL_PKT_TOO_LONG) || /* packet too long */
		    (rc == SOL_BAD_ALLIGNMENT)) { /* not 4K cluster */
			SYS_SOL_TRACE(FASTWRT_EXIT, EINVAL, 0, 0);
			return EINVAL;
		} else { /* couldn't get cluster for collapse */
			SYS_SOL_TRACE(FASTWRT_EXIT, ENOMEM, 0, 0);
			return ENOMEM;
		}
	}

	if (havecluster) {
		/*
		 *  Get an mbuf for the cluster descripter.  The data
		 *  area is cleared to avoid having to clear out the
		 *  cl arrays.
		 */
		cl_desc = (struct cl_desc *)m_getclr(M_DONTWAIT, MT_DATA);
		if (cl_desc == NULL) {
			/*
			 *  Free all mbufs allocated so far.  This
			 *  means we have to walk the m_act pointers
			 *  to make one chain.
			 */
			m_free((struct mbuf *) super_header);

			/*
			 *  Also free the new mbuf chain (if present).
			 */
			if (new_mptr != NULL) {
				m_freem(new_mptr);
			}
			SYS_SOL_TRACE(FASTWRT_EXIT, ENOMEM, 0, 0);
			return ENOMEM;
		}
	}
	/*
	 * Save the message length for updating the statistics in the offlevel.
	 */
	super_header->rcv_tx.tx_info.msglen = msglen;

	/*
	 *  Save the chan and clear the write extension flags.  If we got
	 *  a new chain, free the old one here.  Otherwise it will be
	 *  freed from xmit_offlevel.
	 */
	super_header->rcv_tx.tx_info.chan = chan;
	super_header->rcv_tx.tx_info.flags = (ulong) 0;
	super_header->rcv_tx.tx_info.small_mbuf = NULL;
	super_header->rcv_tx.tx_info.orig_mbuf = NULL;
	if (new_mptr) {
		m_freem(mbufp);
		sol_com_write(new_mptr, super_header, cl_desc, open_ptr);
	} else {
		sol_com_write(mbufp, super_header, cl_desc, open_ptr);
	}

	SYS_SOL_TRACE(FASTWRT_EXIT, 0, 0, 0);
	return 0;
}

/*
 * NAME: sol_check_mbufs
 *
 * FUNCTION: Fixes up an mbuf chain for a transmit.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from either the process environment or the
 *	interrupt environment, and it therefore can not page fault.
 *
 * NOTES:
 *
 *	This routine is either called indirectly by a kernel caller, either
 *	from the normal write or the fast write.  This walks through an
 *	mbuf chain and collapses it as necessary to prepare it for the
 *	transmit.  If collapsing is necessary, a new mbuf chain is used
 *	and this routine returns the new pointer to the caller.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0			- successful completion                        
 *	SOL_PKT_TOO_LONG	- the total length of the packet is too long
 *	SOL_NO_CLUSTER		- could not get the cluster needed for collapse
 *	SOL_BAD_ALLIGNMENT	- cluster is not page alligned / page length
 *
 *	new_mptr		- pointer to a new mbuf chain, if one needed
 *	havecluster		- TRUE if there is a cluster present
 *				- SOL_SMALL_COPY set if a small copy was done
 */

int
sol_check_mbufs(
struct mbuf	*mptr,		/* pointer to original mbuf chain	*/
struct mbuf	**new_mptr,	/* pointer to new mbuf chain (if needed)*/
uchar		*havecluster,	/* returned flag indicates cluster	*/
uint		nodelay,	/* indicates if we can delay		*/
uint		*msglen)	/* total bytes in packet		*/
{
	struct mbuf	*m;
	int		maxbufs, numbufs;
	uchar		mustcollapse, small_copy;

	/*
	 *  NOTE:  it seems that most of the packets from IP come with a
	 *  small mbuf on the front, which means, we end up copying the
	 *  the whole packet.  To prevent this, we have the concept of
	 *  a "small copy".  In this case, all we get is one additional
	 *  cluster, and copy the mbuf into the cluster.  The new_mptr
	 *  will point to the new cluster, and mptr will still point to
	 *  the old mbuf.  The SOL_SMALL_COPY flag will be set in havecluster
	 *  to indicate that this was done.  This requires special handling
	 *  in sol_xmit_offlevel, if the caller requested to NOT free the
	 *  mbufs.
	 */
	*msglen = mptr->m_len;
	numbufs = 1;
	*havecluster = M_HASCL(mptr);
	small_copy = !(*havecluster);
	mustcollapse = FALSE;
	m = mptr->m_next;
	if (M_HASCL(mptr) && ((uint)SOL_MTOCL(mptr) & (PAGESIZE-1))) {
		return SOL_BAD_ALLIGNMENT;
	}
	while (m != NULL) {
		*msglen += m->m_len;
		numbufs++;
		if (M_HASCL(m)) {
			SYS_SOL_TRACE(PASS_MBUF, m,
			    m->M_dat.MH.MH_dat.MH_ext.ext_buf,0);
			/*
			 *  Check to make sure the cluster is page alligned.
			 *  If there is ever anything other than a 4k mbuf
			 *  size, we need this check here.
			 */
			if ((uint)SOL_MTOCL(m) & (PAGESIZE-1)) {
				return SOL_BAD_ALLIGNMENT;
			}
			if (!(*havecluster) && !small_copy) {
				mustcollapse = TRUE;
			}
			*havecluster = TRUE;
		} else {
			SYS_SOL_TRACE(PASS_MBUF, m, 0, 0);
			small_copy = FALSE;
			if (*havecluster) {
				mustcollapse = TRUE;
			}
		}
		m = m->m_next;
	}
	if (*msglen > SOL_MAX_XMIT) {
		return SOL_PKT_TOO_LONG;
	}
	if ((*havecluster && (numbufs > SOL_CMBUF_LEN)) ||
	    (!(*havecluster) && (numbufs > SOL_MMBUF_LEN))) {
		mustcollapse = TRUE;
	}
	if (mustcollapse) {
		*new_mptr = sol_collapse(mptr, *msglen, nodelay);
		if (*new_mptr == NULL) {
			return SOL_NO_CLUSTER;
		}
		/*
		 *  If we called collapse, we must have a cluster now, so
		 *  set the flag (and make sure SMALL_COPY is not set).
		 */
		*havecluster = TRUE;
	} else {
		if (small_copy &&  numbufs > 1) {
			/*
			 *  Get a cluster and copy the first small mbuf
			 *  into it.
			 */
			if (nodelay) {
				*new_mptr = m_getclust(M_DONTWAIT, MT_DATA);
			} else {
				*new_mptr = m_getclust(M_WAIT, MT_DATA);
			}
			if (*new_mptr == NULL) {
				return SOL_NO_CLUSTER;
			}
			bcopy(mptr->m_data, (*new_mptr)->m_data, mptr->m_len);
			(*new_mptr)->m_len = mptr->m_len;
			(*new_mptr)->m_next = mptr->m_next;
			mptr->m_next = NULL;
			*havecluster |= SOL_SMALL_COPY;
		} else {
			*new_mptr = NULL;
			*havecluster &= ~SOL_SMALL_COPY;
		}
	}
	return 0;
}
/*
 * NAME: sol_collapse
 *
 * FUNCTION: Copies the data from an unacceptable mbuf chain into a normal
 *	chain of clusters.  The pointer of the new chain is returned.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from either the process environment or the
 *	interrupt environment, and it therefore can not page fault.
 *
 * NOTES:
 *
 *	This routine is called from sol_check_mbufs when a chain is
 *	detected that contains an unallowed combintaion of small mbufs
 *	and clusters.  The only solution is to make the chain all clusters,
 *	which is what is done here.  Since the user may need the mbufs
 *	back "as is", the routine simply gets a completely new chain and
 *	leaves the original alone.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	struct mbuf *	- successful completion (new mbuf chain is returned)
 *	NULL		- could not get a cluster
 */
struct mbuf *
sol_collapse(
struct mbuf	*mptr,		/* pointer to mbuf chain 		*/
uint		msglen,		/* total length of message		*/
uint		nodelay)	/* indicates if we can wait for mbufs	*/
{
	struct mbuf	*m, *new_mptr, *m_dest, *m_src;
	caddr_t		dest_ptr, src_ptr;
	uint		wait_flag, num_clus, i, dest_resid, src_resid;

	SYS_SOL_TRACE(COLLAPSE_ENTRY, mptr, 0, 0);

	/*
	 *  Update statistics
	 */
	if (ULONG_MAX == sol_ddi.stats.ds.collapsed_frame_lcnt) {
		/* record overflow in msh of counter */
		sol_ddi.stats.ds.collapsed_frame_mcnt++;
	}
	sol_ddi.stats.ds.collapsed_frame_lcnt++;

	/*
	 *  First compute how many clusters we will need for the data.
	 */
	num_clus = (msglen + CLBYTES -1) / CLBYTES;

	/*
	 *  Convert the DNDELAY flag to M_DONTWAIT
	 */
	if (nodelay) {
		wait_flag = M_DONTWAIT;
	} else {
		wait_flag = M_WAIT;
	}

	/*
	 *  Get all the clusters we will need (and link them together).
	 */
	if ((new_mptr = m_getclust(wait_flag, MT_DATA)) == NULL) {
		SYS_SOL_TRACE(COLLAPSE_EXIT, NULL, 0, 0);
		return NULL;
	}
	m = new_mptr;
	for (i=1 ; i<num_clus ; i++) {
		if ((m->m_next = m_getclust(wait_flag, MT_DATA)) == NULL) {
			m_freem(new_mptr);
			SYS_SOL_TRACE(COLLAPSE_EXIT, NULL, 0, 0);
			return NULL;
		}
		m = m->m_next;
	}

	/*
	 *  At this point we have all the buffers we need, so copy the data.
	 */
	m_dest = new_mptr;
	m_src = mptr;
	dest_ptr = MTOD(m_dest, caddr_t);
	src_ptr = MTOD(m_src, caddr_t);
	dest_resid = CLBYTES;
	src_resid = m_src->m_len;
	m_dest->m_len = 0;
	while (TRUE) {
		if (dest_resid >= src_resid) {
			bcopy(src_ptr, dest_ptr, src_resid);
			dest_ptr = (caddr_t) ((uint)dest_ptr + src_resid);
			dest_resid -= src_resid;
			m_dest->m_len += src_resid;
			m_src = m_src->m_next;
			if (m_src == NULL) {
				break;
			} else {
				src_resid = m_src->m_len;
				src_ptr = MTOD(m_src, caddr_t);
			}
		} else {
			bcopy(src_ptr, dest_ptr, dest_resid);
			src_ptr = (caddr_t) ((uint)src_ptr + dest_resid);
			src_resid -= dest_resid;
			m_dest->m_len = CLBYTES;
			m_dest = m_dest->m_next;
			m_dest->m_len = 0;
			dest_resid = CLBYTES;
			dest_ptr = MTOD(m_dest, caddr_t);
		}
	}
	SYS_SOL_TRACE(COLLAPSE_EXIT, new_mptr, 0, 0);
	return new_mptr;
}

/*
 * NAME: sol_com_write
 *
 * FUNCTION: Common write routine for both sol_write and sol_fastwrt
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from either the process environment or the
 *	interrupt environment, and it therefore can not page fault.
 *
 * NOTES:
 *
 *	This will transmit one packet only (m_act is ignored).
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	Nothing.
 */

void
sol_com_write(
struct mbuf		*mbufp,		/* pointer to mbuf chain	*/
struct super_header	*super_header,	/* pointer to imcs header	*/
struct cl_desc		*cl_desc,	/* pointer to cluster desc.	*/
struct sol_open_struct	*open_ptr)	/* pointer to open structure	*/
{

	struct mbuf		*m, *tmp_mptr;
	struct imcs_header	*imcs_hdr;
	cio_stat_blk_t		stat_blk;
	short			imcs_addr;
	int  			tag, mbufcount, rc, oldpri;

	SYS_SOL_TRACE(COM_WRITE_ENTRY, mbufp, super_header, cl_desc);
	imcs_hdr = &super_header->imcs_header;
	/*
	 *  Get the processor id from the packet header.
	 */
	imcs_addr = (short) *(uint *)(MTOD(mbufp, uint) + SOL_PROC_OFFSET);
	ASSERT((imcs_addr > 0) && (imcs_addr < 255));
	if (cck_proc[imcs_addr] == 0) {	/* unreachable processor_id */
		/*
		 *  The purpose of this code is to avoid calling imcs_sendmsg
		 *  when it is known that the destination is not reachable.
		 *  So just build the status block here and return.
		 */
		if (super_header->rcv_tx.tx_info.small_mbuf != NULL) {
			tmp_mptr = m_free(mbufp);
			mbufp = super_header->rcv_tx.tx_info.small_mbuf;
			mbufp->m_next = tmp_mptr;
		} else if (super_header->rcv_tx.tx_info.orig_mbuf != NULL) {
			m_freem(mbufp);
			mbufp = super_header->rcv_tx.tx_info.orig_mbuf;
		}
		if (super_header->rcv_tx.tx_info.flags & CIO_ACK_TX_DONE) {
			/*
			 *  Notify the user that the xmit is done.
			 */
			stat_blk.code = (ulong)CIO_TX_DONE;
			stat_blk.option[0] = CIO_HARD_FAIL;
			stat_blk.option[1] =
			    super_header->rcv_tx.tx_info.write_id;
			stat_blk.option[2] = (ulong) mbufp;
			stat_blk.option[3] = SOL_NEVER_CONN;
			sol_report_status(open_ptr, &stat_blk);
		}
		if (!(super_header->rcv_tx.tx_info.flags & CIO_NOFREE_MBUF)) {
			/*
			 * Unless the sender requested not to,
			 * free the mbuf chain.
			 */
			m_freem(mbufp);
		}
		/*
		 *  Free the cluster descripter (if present) and the header.
		 */
		if (cl_desc) {
			m_free((struct mbuf *) cl_desc);
		}
		m_free((struct mbuf *) super_header);
		SYS_SOL_TRACE(COM_WRITE_EXIT, mbufp, 1, 0);
		return;
	}

	/*
	 *  On transmit, we have to point the offset pointer beyond the
	 *  processor id field in the data (and decrement the length).
	 */
	mbufp->m_len -= 4;
	mbufp->m_data = (caddr_t) ((uint) mbufp->m_data + 4);

	/*
	 *  Link the cluster descripter to the front of the chain,
	 *  and initialize the descripter.
	 */
	if (cl_desc != NULL) {
		((struct mbuf *)cl_desc)->m_next = mbufp;
		/*
		 *  The m_len field has to be filled in or else it will
		 *  be treated as a 0 length mbuf and never sent.
		 */
		((struct mbuf *)cl_desc)->m_len =
		    (SOL_CMBUF_LEN*sizeof(uint))*2;
		mbufp = (struct mbuf *)cl_desc;
	}

	/* build tag words for mbufs or clusters (if any) and flush*/

	tag = 0;
	mbufcount = 0;
	for (m = mbufp ; m ; m = m->m_next) {
		if (m->m_len != 0) {
			if (!(M_HASCL(m))) { /* small mbuf */
				/*
				 *  The data pointer has to be converted
				 *  to an offset to be meaningful to the
				 *  receiver.
				 */
				m->m_data = (caddr_t) ((uint)m->m_data-(uint)m);
				rc = tagwords(m, MSIZE, imcs_hdr, &tag);
				ASSERT(rc == 0);
				vm_cflush(m, MSIZE);
			}
			else { /* cluster */
				rc = tagwords(SOL_MTOCL(m), CLBYTES, imcs_hdr,
				    &tag);
				ASSERT(rc == 0);
				vm_cflush(SOL_MTOCL(m), CLBYTES);
				/*
				 *  Save the data offset and length of
				 *  the cluster.
				 */
				cl_desc->offsets[mbufcount] =
				    MTOD(m,int) & CLOFSET;
				cl_desc->lengths[mbufcount] = m->m_len;
				mbufcount++;
			}
		}
	}

	/*
	 *  Fill in the header and send the message.
	 */

	super_header->rcv_tx.tx_info.mptr = mbufp;

	for ( ; tag < NUM_HDR_TCWS ; tag++) {
		imcs_hdr->IMCS_TAG(tag) = LAST_TCW;
	}
	imcs_hdr->IMCS_PROTOCOL = IMCS_RTS_SND_CODE;
	imcs_hdr->IMCS_SERIALIZE = open_ptr->serialize;
	imcs_hdr->dest_proc_token = imcs_addr;         
	/*
	 *  mbuf chains are sent on odd subchannels, clusters are sent
	 *  on even subchannels.
	 */
	if (cl_desc != NULL) {
		imcs_hdr->IMCS_SUBCHANNEL = open_ptr->subchannel + 1;
		vm_cflush(cl_desc, MSIZE);
	} else {
		imcs_hdr->IMCS_SUBCHANNEL = open_ptr->subchannel;
	}
	imcs_hdr->notify_address = (void (*) ()) sol_xmit_notify;
	rc = imcs_sendmsg(imcs_hdr);
	ASSERT(rc == 0);
	oldpri = i_disable(SOL_OFF_LEVEL);
	open_ptr->xmit_count++;
	i_enable(oldpri);
	SYS_SOL_TRACE(COM_WRITE_EXIT, mbufp, 0, 0);

	return;
}
/*
 * NAME: sol_xmit_notify
 *
 * FUNCTION: Schedules transmit done off-level routine.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from the interrupt environment, and it can
 *	not page fault.
 *
 * (NOTES:)
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	Nothing.
 */
void
sol_xmit_notify(
struct imcs_header	*hdr)	/* IMCS header that just completed	*/
{
	struct super_header *super_header;
	
	/*
	 *  This routine is called with the address of an imcs_header.  The
	 *  header is at an offset of sizeof(m_hdr) in the super_header.
	 *  So we first compute the address of the super_header.
	 */
	super_header = (struct super_header *)((uint)hdr-sizeof(struct m_hdr));
	SYS_SOL_TRACE(XMIT_NOTIFY_ENTRY, super_header, 0, 0);

	/* 
	 *  The interrupt structure used to schedule the off-level is part
	 *  of the super_header.  We set up a field in the interrupt struct
	 *  to point back to the start of the super_header, for when the
	 *  off-level is called, and then schedule the off-level.
	 */
	super_header->ourintr.super_header = super_header;
	INIT_OFFL1(&super_header->ourintr.intr,sol_xmit_offlevel,0);
	i_sched(&super_header->ourintr.intr);
	SYS_SOL_TRACE(XMIT_NOTIFY_EXIT, super_header, 0, 0);
}

/*
 * NAME: sol_xmit_offlevel
 *
 * FUNCTION: Off-level routine to handle transmit completion.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from the interrupt environment, and it can
 *	not page fault.
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
 */
int
sol_xmit_offlevel(
struct intr	*iptr)		/* intr structure filled in by xmit_notify */
{

cio_stat_blk_t		stat_blk;
struct mbuf		*mptr, *tmp_mptr;
struct sol_open_struct	*open_ptr;
struct imcs_header	*hdr;
struct super_header	*super_header;

	super_header = ((struct ourintr *)iptr)->super_header;
	hdr = &super_header->imcs_header;

	SYS_SOL_TRACE(XMIT_OFFLEVEL_ENTRY, super_header, hdr->outcome, 0);
	mptr = super_header->rcv_tx.tx_info.mptr;
	open_ptr = sol_ddi.open_ptr[super_header->rcv_tx.tx_info.chan];

	/* Update statistics */
	if (hdr->outcome) {
		sol_ddi.stats.cc.tx_err_cnt++;
	} else {
		if (ULONG_MAX == sol_ddi.stats.cc.tx_frame_lcnt) {
			/* record overflow in msh of counter */
			sol_ddi.stats.cc.tx_frame_mcnt++;
		}
		sol_ddi.stats.cc.tx_frame_lcnt++;

		if ((ULONG_MAX - super_header->rcv_tx.tx_info.msglen) <
		    sol_ddi.stats.cc.tx_byte_lcnt) {
			/* record overflow in msh of counter */
			sol_ddi.stats.cc.tx_byte_mcnt++;
		}
		sol_ddi.stats.cc.tx_byte_lcnt +=
		    super_header->rcv_tx.tx_info.msglen;
	}

	if (!(hdr->IMCS_SUBCHANNEL & 1)) {
		/*
		 * An even subchannel indicates clusters were sent.
		 * Therefore, the cluster descripter must be freed.
		 */
		mptr = m_free(mptr);
	}

	if ((open_ptr == NULL) || !(super_header->rcv_tx.tx_info.flags &
	    CIO_NOFREE_MBUF) || super_header->rcv_tx.tx_info.orig_mbuf) {
		/*
		 * Unless the sender requested not to, free the mbuf chain.
		 */
		m_freem(mptr);
	} else {
		/*
		 *  We need to restore the pointer back to the imcs pid,
		 *  and change the offset pointers back to data pointers.
		 *  In addition, if a small-copy was done, we need to link
		 *  the small mbuf back onto the chain, and free the
		 *  first cluster.
		 */
		if (super_header->rcv_tx.tx_info.small_mbuf == NULL) {
			mptr->m_len += 4;
			mptr->m_data = (caddr_t) ((uint)mptr->m_data - 4);
			tmp_mptr = mptr;
		} else {
			tmp_mptr = m_free(mptr);
			mptr = super_header->rcv_tx.tx_info.small_mbuf;
			mptr->m_next = tmp_mptr;
			/*
			 *  No need to fix up the first mbuf, because we
			 *  never changed it.
			 */
		}
		for ( ; tmp_mptr != NULL ; tmp_mptr = tmp_mptr->m_next) {
			tmp_mptr->m_data = (caddr_t) ((uint)tmp_mptr +
			    (uint)tmp_mptr->m_data);
		}
	}
		
	/*
	 *  Notify the user that the xmit is done.
	 */
	if (open_ptr != NULL) {
		if (super_header->rcv_tx.tx_info.flags & CIO_ACK_TX_DONE) {
			stat_blk.code = (ulong)CIO_TX_DONE;
			stat_blk.option[1] =
			    super_header->rcv_tx.tx_info.write_id;
			if (super_header->rcv_tx.tx_info.orig_mbuf) {
				stat_blk.option[2] = (ulong) super_header->
				    rcv_tx.tx_info.orig_mbuf;
			} else if (super_header->rcv_tx.tx_info.small_mbuf) {
				stat_blk.option[2] = (ulong) super_header->
				    rcv_tx.tx_info.small_mbuf;
			} else {
				stat_blk.option[2] = (ulong) mptr;
			}
			stat_blk.option[3] = hdr->outcome;
			if (hdr->outcome) {  /* some sort of error occured */
				stat_blk.option[0] = CIO_HARD_FAIL;
			} else {
				stat_blk.option[0] = CIO_OK;
			}
			sol_report_status(open_ptr, &stat_blk);
		}
		/*
		 *  If this is the last xmit to complete, and the close is
		 *  waiting, wake him up.
		 */
		open_ptr->xmit_count--;
		if ((open_ptr->xmit_count == 0) &&
		    (open_ptr->close_event != EVENT_NULL)) {
			e_wakeup(&open_ptr->close_event);
		}
	}

	/*
	 *  Free the mbuf used as the IMCS header.
	 */
	mptr = (struct mbuf *) super_header;
	m_free(mptr);

	SYS_SOL_TRACE(XMIT_OFFLEVEL_EXIT, super_header, 0, 0);
	return 0;
}
