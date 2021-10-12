static char sccsid[] = "@(#)20	1.32  src/bos/kernel/vmm/POWER/v_pdtsubs.c, sysvmm, bos411, 9436C411a 9/6/94 14:01:11";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	v_pdtinsert, v_pdtdelete, v_devpdtx
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include "vmsys.h"
#include "sys/errno.h"
#include <sys/buf.h>
#include "mplock.h"
#ifdef _VMM_MP_EFF
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif /* _VMM_MP_EFF */

/*
 *  input/output  parameters to these routines are as follows:
 *
 *  (1) devtype : devtype  specifies whether the device is a
 *	paging device D_PAGING or file system D_FILESYSTEM
 *	or a remote device D_REMOTE or a log D_LOGDEV , or 
 *	server D_SERVER.
 *
 *  (2) devid : the dev_t (major minor numbers ) of the block
 *	device if devtype is not D_REMOTE or D_SERVER. the address
 *	of strategy routine if type is D_REMOTE or D_SERVER.
 */

/*
 * v_pdtinsert(devtype,devid,bp,fragsize,comptype)
 *
 * inserts an entry in the PDT (paging device table) for
 * the device specified. see vm_pdtinsert also.
 *
 * RETURN VALUES
 *	0      - ok
 *	ENOMEM - pdt table is full
 *	EINVAL - device is already in table.
 */

v_pdtinsert(devtype,devid,bp,fragsize,comptype)
int	devtype;
union {
	dev_t   dev;  	/* dev_t of block device */
	int ( *ptr)();  /* pointer to strategy routine */
	} devid;
struct buf *bp;         /* pointer to first of list of buf structs */
int fragsize;		/* device fragment size in bytes */
ushort comptype;	/* compression type */
{
	int k,first,last,free,cnt;
	struct buf *bufp;

	PDT_LOCKHOLDER();

	/* check if in table already
	 */
	if(v_devpdtx(devtype,devid) >= 0)
		return(EINVAL);

	/* only one server allowed
	 */
	if (devtype == D_SERVER && pf_pdtserver)
		return(EINVAL);

	if (devtype == D_PAGING)
	{
		first = 0;
		last = MAXPGDEV - 1;
	}
	else
	{
		first = MAXPGDEV;
		last = PDTSIZE - 1;
	}

	/* pdt_type of 0 means slot is free. however we prefer
	 * to use free slots with either the same devid as before
	 * or never used slots (devid = 0).
	 */
	free = -1;
	for(k = first; k <= last; k ++)
	{
		if (pdt_type(k) == 0 )
		{
			free = k;
			if(pdt_device(k) == 0 || pdt_device(k) == devid.dev)
				break;
		}
	}

	if (free >=  0)
	{
		pdt_type(free) = devtype;
		pdt_nextio(free) = -1;
		pdt_device(free) = devid.dev;
		pdt_iotail(free) = -1;
		pdt_bufstr(free) = bp;
		pdt_iocnt(free) = 0;
		pdt_avail(free) = 0;
		pdt_fperpage(free) = PSIZE / fragsize;
		pdt_comptype(free) = comptype;

		/* count and record number of buf structs
		 */
		for (cnt=0, bufp=bp; bufp!=NULL; cnt++, bufp=bufp->av_forw)
			;
		pdt_nbufs(free) = cnt;

		switch (devtype) {
		case D_PAGING :
			pf_pdtmaxpg = MAX(free,pf_pdtmaxpg);
			break;
		case D_SERVER :
			pf_pdtserver = free;
			break;
		default :
			break;
		}

		return 0;
	}

	return(ENOMEM);
}

/*
 * v_pdtdelete(devtype,devid,bp)
 *
 * delete an entry from the PDT after all i/o currently
 * scheduled for the device has completed. sets bp to
 * the address of first associated buf-struct.
 *
 * the scb devid with be marked as pending delete and
 * segment pages will be release for all valid segment
 * associated with the PDT entry.
 *
 * this procedure executes at VMM interrupt level on 
 * fixed stack with back-tracking enabled.
 *
 * RETURN VALUES
 *	0	- ok
 *	EINVAL	- device not in PDT table.
 *	VM_WAIT - i/o not finished (this doesn't make it
 *		  back to caller of vcs_pdtdelete).
 *	
 */

v_pdtdelete(devtype,devid,bp)
int  devtype; 
union {
	dev_t   dev;  /* dev_t of block device */
	int ( *ptr)();  /* pointer to strategy routine */
	} devid;
struct buf **bp; /* set to pointer to first buf-struct */
{
	int pdtx;
	uint sidx, sid;

	PDT_MPLOCK();

	/* get pdt index associated with device
	 */
	if ( (pdtx = v_devpdtx(devtype,devid)) < 0)
	{
		return(EINVAL);
	}

	PDT_MPUNLOCK();

	/*
	 * For remote devices mark all client segments associated with
	 * the device as inactive and release their pages.
	 */
	if (devtype == D_REMOTE)
	{
		for (sidx = 0; sidx < pf_hisid; sidx += 1)
		{
			/* in mp lazy test first, recheck under lock
			 */
			if (scb_valid(sidx) && scb_clseg(sidx) &&
				scb_devid(sidx) == pdtx)
			{
				SCB_MPLOCK(sidx);
				if (scb_valid(sidx) && scb_clseg(sidx) &&
					scb_devid(sidx) == pdtx)
				{
					scb_inactive(sidx) = 1;
					sid = ITOS(sidx,0);
					v_release(sid,0,MAXFSIZE/PSIZE,V_NOKEEP);
				}
				SCB_MPUNLOCK(sidx);
			}
		}
	}
	
	PDT_MPLOCK();

	/* any more i/o to do ?
	 */
	if (pdt_iocnt(pdtx))
	{
		v_wait(&pf_devwait);
		PDT_MPUNLOCK();
		return(VM_WAIT);
	}

	/* set up bp */
	*bp = pdt_bufstr(pdtx);

	/* mark it as free and return.
	 */
	pdt_type(pdtx) = 0;
	if (devtype == D_SERVER)
		pf_pdtserver = 0;

#ifdef _VMM_MP_EFF
	/* release the instrumentation data struct if any
	 */
	lock_free(&lv_lock(pdtx));
#endif /* _VMM_MP_EFF */

	PDT_MPUNLOCK();

	return 0;
}

/*
 * v_devpdtx(devtype,devid)
 *
 * returns the index in the pdt table associated with the
 * device specified or -1 if there is none.
 */

v_devpdtx(devtype,devid)
int devtype; 
union {
	dev_t   dev;  /* dev_t of block device */
	int ( *ptr)();  /* pointer to strategy routine */
	} devid;
{
	int k,first,last ;

	/*
	 * We assume that operations involving the PDT array are
	 * serialized at a higher level so we do not need the PDT
	 * lock to search it here.
	 */
	if (devtype == D_PAGING)
	{
		first = 0;
		last = MAXPGDEV - 1;
	}
	else
	{
		first = MAXPGDEV;
		last = PDTSIZE - 1;
	}

	for(k = first; k <= last; k++)
	{
		if (pdt_device(k) == devid.dev  && pdt_type(k) != 0)
			return(k);
	}

	return VM_NOTIN;
}
