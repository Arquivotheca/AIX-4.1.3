static char sccsid[] = "@(#)94	1.26.1.12  src/bos/kernel/vmm/POWER/vmcreate.c, sysvmm, bos41B, 9505B 1/25/95 15:25:51";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:	vms_create
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "vmsys.h"
#include <sys/types.h>
#include <sys/vnode.h>
#include <sys/errno.h>
#include <sys/inline.h>
#include <sys/trchkid.h>
#ifdef _VMM_MP_EFF
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif /* _VMM_MP_EFF */

/*
 * vms_create(sid,type,device,size,uplim,downlim)
 *
 * create a virtual memory segment of the type and size and
 * limits specified.
 *
 * INPUT PARAMETERS:
 *
 * (1) type : type is specified as an OR of bits (see vmuser.h)
 *      (a) basic type of segment
 *          V_WORKING - working storage segment
 *          V_CLIENT - client segment
 *          V_PERSISTENT  - persistent storage segment
 *          V_MAPPING - mapping segment (mmap)
 *
 *      (b) options for working storage segments
 *          V_UREAD   - only reads are allowed in the user region
 *                    - with access key 1.
 *          V_PTASEG  - the segment is a page table area segment.
 *          V_SYSTEM  - the segment is a system segment.
 *          V_PRIVSEG - the segment is a process private segment.
 *          V_SHRLIB - Shared Library segment
 *          V_SPARSE -  the segment has pages sparsely populated.
 *
 *      (c) options for persistent segments
 *          V_JOURNAL - the segment is also journalled.
 *          V_DEFER   - the segment is deferred update
 *          V_LOGSEG  - the segment is used for a log
 *          V_SYSTEM  - the segment is a system segment.
 *
 *      (d) options for client segments.
 *          V_INTRSEG  - the segment is a interruptable segment.
 *
 *      (e) options for mapping segments.
 *          none - all other parameters (device,size,uplim,downlim)
 *		   should be 0
 *
 * (2) device: device is the dev_t of the filesystem or block
 *     device for persistent segments and the address of the
 *     gnode for client segments. the device must have be 
 *     mounted prior to calling this procedure (see vm_mount).
 *     the device parameter is ignored for working segments.
 *
 * (3) size : size is expressed in units of bytes and is rounded
 *     up to a multiple of pagesize. its interpretation depends
 *     on the segment type as follows. size may have any value
 *     between [0,SEGSIZE], inclusive.
 *
 *     (a) working storage segments: the interval in the segment
 *         [0,size - 1] is the "user" region of the segment and
 *         the interval [size,SEGSIZE-1] is the "system" region.
 *         the default storage protect key for the user region
 *         is read-write with access key 1. this can be overridden
 *         by specifying V_UREAD in which case only reads are
 *         permitted with key 1. the storage protect key for the
 *         system region is no-access with key 1 and read-write
 *         with key 0.
 *
 *      (b) persistent storage segments. size specifies the current
 *	    size of the file.
 *
 *      (c) client segments: size specifies the current size of the 
 *	    file.
 *
 *  (4) uplim and downlim : these are expressed in units of bytes
 *      and specify the sizes of the legal low and high address
 *      intervals in the segment. both uplim and downlim are rounded
 *      up to a multiple of PAGESIZE.
 *
 *      (a) working storage segments: the intervals [0,uplim-1]
 *          and [SEGSIZE - downlim, SEGSIZE -1] are legal addresses.
 *          the interval in the middle [uplim, SEGSIZE - downlim - 1]
 *          is not. attempts to reference data in the middle result
 *          in the addressing exception EFAULT.
 *
 *      (b) persistent storage segments: ignored. the limit on
 *          file size is growth is in the u-block.
 *
 *      (c) client segments - same as persistent storage.
 *
 *  OUTPUTS
 *
 *       (1) sid is set to the segment id of the allocated segment.
 *
 *  RETURN VALUES
 *
 *      0 - successful
 *
 *      ENOMEM - out of sids or xpt space.
 *
 *      ENODEV - device not mounted. (client or persistent segments).
 *
 *      EINVAL - incompatible or bad parameters.
 *
 */

int vms_create(sidp,type,device,size,uplim,downlim)
int    * sidp;    /* set to sid of created segment */
int     type;    /* type of segment to create + options */
union {
	dev_t  dev;	/* dev_t of block device */
	struct gnode *ptr;  /* pointer to gnode of file */
      } device;
uint     size;    /* size of user area or file  */
uint     uplim;   /* upper limit on growth */
uint     downlim; /* downward limit on growth */
{
        int rc,sid,xptsize,sidx,savevmm,savepta,basetype;
	uint maxsize;
	union {
		dev_t  dev;  /* dev_t block device */
		int (*ptr) ();  /* pointer to strategy routine */
	     	} devparm;	

        /* Some parm checking code;
	 * Note that for Non-process private working segments,
	 * the size is set to maximum as it is ignored 
         */
	maxsize = (type & V_WORKING) ? SEGSIZE : MAXFSIZE;
	if (uplim > maxsize || downlim > maxsize || 
	   (((type & V_PRIVSEG || !(type & V_WORKING)) && (size > maxsize ))))
		return(EINVAL);
        /* load sregs to address vmmdseg and ptaseg
         */
        savevmm = chgsr(VMMSR,vmker.vmmsrval);
        savepta = chgsr(PTASR,vmker.ptasrval);

	/* determine devparm on basis of type.
	 */
	if (type & V_CLIENT)
		devparm.ptr = device.ptr->gn_ops->vn_strategy;
	else
		devparm.dev = device.dev;

        loop:
        if (rc = vcs_create(&sid,type,devparm,size))
                goto closeout;

        /* do we have to delete the sid ?
         * if a segment needs to be deleted it is done here
         * because this code runs under a process whereas
         * sometimes the final detach which makes a segment
         * attach count goto zero occurs in a device interrupt
         * handler.
         */

        sidx = STOI(sid);   /* index in scb table */
        if(scb_delpend(sidx))
        {
		scb_delpend(sidx) = 0;
                vms_delete(sid);
                goto loop;
        }


#ifdef _VMM_MP_EFF
	/*
	 * Allocate lock instrumentation here since it cannot be
	 * done in critical sections.  SCB lock instrumentation
	 * structures are never freed because SCB locks may be
	 * acquired even when the SCB is deleted so we cannot
	 * change the state of the lock word.  We don't know if
	 * the SCB we allocated is brand new or was pulled off the
	 * free list so we don't know whether a lock instrumentation
	 * structure must be allocated.  We pass the REINIT flag
	 * to lock_alloc() so it will figure out whether or not a
	 * lock instrumentation structure needs to be allocated.  If
	 * this is a re-use it will take care of resetting previous
	 * instrumentation data and will not modify the lock word.
	 */
	lock_alloc(&scb_lock(sidx), LOCK_ALLOC_PIN | LOCK_ALLOC_REINIT,
		   VMM_LOCK_SCB, sidx);
#endif	/* _VMM_MP_EFF */

        /* vcs_create allocated an scb entry and an xpt for
         * non-client segments. the scb is cleared except for
         * the scb_vxpto field which points to the xpt and the
         * devid field. the xpt itself is not initialized.
         */

        scb_sidlist(sidx) = -1;   /* null list of page frames */
        scb_maxvpn(sidx) = -1;

        basetype = type & ( V_WORKING | V_PERSISTENT | V_CLIENT | V_MAPPING );
        switch(basetype)
        {
        case V_WORKING:
                scb_defkey(sidx) = (type & V_UREAD) ? UTXTKEY : UDATAKEY;
                scb_wseg(sidx) = 1;
                scb_compseg(sidx) = 1;
                scb_system(sidx) = (type & V_SYSTEM) != 0;
                scb_ptaseg(sidx) = (type & V_PTASEG) != 0;
                scb_privseg(sidx) = (type & V_PRIVSEG) != 0;
                scb_shrlib(sidx) = (type & V_SHRLIB) != 0;
                scb_sparse(sidx) = (type & V_SPARSE) != 0;
                scb_uplim(sidx) = BTOPG(uplim) - 1;
		scb_downlim(sidx) = MAXVPN - BTOPG(downlim) + 1;
                /* make uplim and downlim non-overlapping
                 * uplim and downlim are valid page numbers.
                 */
                if (scb_downlim(sidx) <=  scb_uplim(sidx))
                        scb_downlim(sidx) = scb_uplim(sidx) + 1;

                /* sysbr is last page of user region
		 * for non-system, non-private segments (shared mem segments)
		 * it is set to max since this limit does not apply.
                 */
		if ((type & V_SYSTEM) || (type & V_PRIVSEG))
			scb_sysbr(sidx) = BTOPG(size) - 1;
		else
			scb_sysbr(sidx) = MAXVPN + 1;
		
                scb_minvpn(sidx) = SEGSIZE/PSIZE;

		/* init xpt to zeros
		 *
		 * NOTE: block zero requires starting address and range
		 * be cache aligned.
		 */
		block_zero(scb_vxpto(sidx), 1024);
		scb_npsblks(sidx) = 0;

		/* Allocate paging space for segment
		 */
		if ((type & V_PSEARLYALLOC) && (rc = vcs_psearlyalloc(sidx)))
                {
                        vcs_freeseg(sidx,0);
                        goto closeout;
		}
                break;
        case V_PERSISTENT:
                scb_pseg(sidx)   = 1;
                scb_jseg(sidx)   = (type & V_JOURNAL) != 0;
                scb_logseg(sidx) = (type & V_LOGSEG) != 0;
                scb_defseg(sidx) = (type & V_DEFER) != 0;
                scb_system(sidx) = (type & V_SYSTEM) != 0;
                scb_maxvpn(sidx) = BTOPG(size) - 1;
		scb_npgahead(sidx) = 0;
		scb_lstpagex(sidx) = 0;
                break; 
        case V_CLIENT:
                scb_clseg(sidx) = 1;
		scb_gnptr(sidx) = device.ptr;
                scb_intrseg(sidx)   = (type & V_INTRSEG) != 0;
                scb_maxvpn(sidx) = BTOPG(size) - 1;
		break;
        case V_MAPPING:
                scb_mseg(sidx) = 1;
		break;
        }

	/* Now that the scb initialization is complete
	 * we can set the valid bit but the initialization
	 * must be globally performed.
	 */
	/* Now that the scb initialization is complete,
	 * it must be globally performed before the valid bit
	 * is set.  The scb must always be in a consistent
	 * state when the valid bit is set, for code that
	 * examines scb's in unknown states.
	 *
	 * We must also globally perform the setting of the
	 * valid bit before the scb is made available to
	 * the caller, so that all processors will find the
	 * scb to be valid.
	 */
#ifdef _POWER_MP
	SYNC();
#endif /* _POWER_MP */
	scb_valid(sidx) = 1;
#ifdef _POWER_MP
	SYNC();
#endif /* _POWER_MP */

	/* Now that everything is globally visible, update
	 * sid return value.
	 */
	*sidp = sid;

	/* performance trace hook.
	 */
	TRCHKL2T(HKWD_VMM_SEGCREATE,sid,scb_sibits(sidx));

        closeout:
        (void)chgsr(VMMSR,savevmm);
        (void)chgsr(PTASR,savepta);
        return(rc);
}
