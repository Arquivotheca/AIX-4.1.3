static char sccsid[] = "@(#)95	1.4  src/bos/kernel/io/machdd/lgbus.c, machdd, bos411, 9428A410j 4/26/94 09:06:37";
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: md_timer_pop(), md_delay(),
 *            md_reset_slot(),
 *	      md_slot_bucp(), md_read_vpd(), md_select_slot(),
 *            md_read_cfg(), md_write_cfg()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifdef _RS6K_UP_MCA

#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/rosinfo.h>
#include <sys/timer.h>
#include <sys/sleep.h>
#include <sys/systemcfg.h>
#include <sys/sys_resource.h>
#include <sys/intr.h>
#include <sys/iplcb.h>
#include <sys/vmker.h>
#include <sys/uio.h>
#include <sys/ioacc.h>
#include <sys/nvdd.h>
#include "md_extern.h"

extern struct md_bus_info md_bus[];
int md_event = EVENT_NULL;	/* used by md_delay */

/*
 * NAME: md_timer_pop
 *
 * FUNCTION: Wake up process sleeping on timer (see md_delay)
 *
 * EXECUTION ENVIRONMENT:  Interrupt
 *
 * RETURN VALUE: 0 
 *
 */
void
md_timer_pop(struct trb *tmr)
{
	e_wakeup(&md_event);
}


/*
 * NAME: md_delay
 *
 * FUNCTION: Delay for period specified in nanoseconds.
 *
 * EXECUTION ENVIRONMENT:  Process only.
 *
 * RETURN VALUE: 0 if no error, ENOMEM of talloc fails
 *
 */
int
md_delay(ulong nsecs)
{
	struct trb *tmr;
	int old;
	ulong ns, s;

	if (NULL == (tmr = talloc()))
		return(ENOMEM);
	s = nsecs / NS_PER_SEC;
	ns = nsecs % NS_PER_SEC;
	tmr->timeout.it_value.tv_nsec = ns;
	tmr->timeout.it_value.tv_sec = s;
	tmr->ipri = INTTIMER;
	tmr->flags = 0;
	tmr->func_data = 0;
	tmr->func = md_timer_pop;
	old = i_disable(INTTIMER);
	tstart(tmr);
	e_sleep(&md_event, EVENT_SIGRET);
	i_enable(old);
	tstop(tmr);
	tfree(tmr);
	return(0);
}


/*
 * NAME: md_reset_slot
 *
 * FUNCTION: Reset a bus slot.  Hold reset for a specified number of
 *           nanoseconds, then release.  If delay=0, then clear reset
 *	     bit and return.
 *
 * EXECUTION ENVIRONMENT:  Process only.
 *
 * RETURN VALUE: 0 if no error, EINVAL if slot number is invalid.
 *
 * NOTES:  This routine should only be called on a PowerPC machine.
 *
 */
int
md_reset_slot(uint slot, ulong nsecs)
{
        struct buc_info *md_slot_bucp();
	struct buc_info *buc_ptr;
	volatile uint *busreset;
	int old_int;
	ulong old_arb;
	ulong md_disable_pipe();
	ulong md_arbcntl_rw();

	if (0 == (buc_ptr = md_slot_bucp(slot))) /* scan buc tables for slot */
		return(EINVAL);
	old_int = i_disable(INTMAX);
	old_arb = md_disable_pipe();
	busreset = sys_resource_ptr->sys_regs.bus_slot_reset +
			buc_ptr->bsrr_offset;
	*busreset &= ~buc_ptr->bsrr_mask;	/* clear reset bit */
	__iospace_sync();
	md_arbcntl_rw(old_arb, 1);   /* restore arb cntl reg */
	i_enable(old_int);
	if (nsecs) {
		md_delay(nsecs);		  /* delay */
		old_int = i_disable(INTMAX);
		old_arb = md_disable_pipe();
		*busreset |= buc_ptr->bsrr_mask;  /* set reset bit */
		__iospace_sync();
		md_arbcntl_rw(old_arb, 1);   /* restore arb cntl reg */
		i_enable(old_int);
	}
	return(0);
} /* md_reset_slot */


/*
 * NAME: md_select_slot
 *
 * FUNCTION: Select or deselect the bus slot by writing to the bus slot
 *           configuration register.  Also check for valid slot number.
 *
 * EXECUTION ENVIRONMENT:  Process only.
 *
 * RETURN VALUE: Return buc pointer if slot is valid, else 0. 
 *
 * NOTES:  This routine should only be called on a PowerPC machine.
 *
 */
struct buc_info * 
md_select_slot(uint slot)
{
    struct buc_info *md_slot_bucp();
    struct buc_info *bucp;

    if (!slot) { 	 	/* slot == 0; turn off config mode */
        sys_resource_ptr->sys_regs.bus_slot_cfg = (uint)0;
        __iospace_sync();	/* make sure config signal is asserted */
	return(0);
    }
    if (bucp = md_slot_bucp(slot)) {
        sys_resource_ptr->sys_regs.bus_slot_cfg = bucp->bscr_value;
        __iospace_sync();	/* make sure config signal is asserted */
    }
    return(bucp);
} /* md_select_slot() */


/*
 * NAME: md_read_vpd
 *
 * FUNCTION: Read from the VPD/Feature ROM space.  Selects the proper
 *           bus slot, then reads the specified data.  The word
 *	     increment is queried to allow stepping over "holes".
 *	     (A29 is not used on some adapters; therefore, every
 *           other 4 bytes is missing and must be skipped.)
 *
 * EXECUTION ENVIRONMENT:  Process only.
 *
 * RETURN VALUE: Returns 0 if successful, EINVAL if slot or address
 *		 is invalid, ENOMEM if memory cannot be allocated,
 *		 EFAULT if a bad buffer address is passed.
 *
 * NOTES:  This routine should only be called on a PowerPC machine.
 *
 */
int
md_read_vpd(uint slot, ulong offset, ulong size, char *buf, int fkern)
{
    caddr_t addr;
    ulong iosize;
    struct buc_info *buc_ptr;
    struct buc_info *md_select_slot();
    uint cnt;
    int rc = 0;
    int i;
    int old_int;
    ulong old_arb, sz=size;
    ulong md_disable_pipe();
    ulong md_arbcntl_rw();
    char *ptr, *p;
    struct io_map iomap;

    if (NULL == (ptr = xmalloc(size, 2, pinned_heap)))
	return(ENOMEM);
    old_int = i_disable(INTMAX); /* lock config register */
    old_arb = md_disable_pipe(); /* disable pipelining */
    if (0 == (buc_ptr = md_select_slot(slot)) || 
      ((offset+size) > MAX_VPDADDR)) {
	md_arbcntl_rw(old_arb, 1);   /* restore arb cntl reg */
	i_enable(old_int);
	xmfree(ptr, pinned_heap);
        return(EINVAL);		/* illegal slot number or address */
    }
    iosize = buc_ptr->cfg_addr_inc;		/* get word increment */
    if ((iosize == 8) && (offset & 0x100)) {	/* check for illegal address */
	md_select_slot(0);
	md_arbcntl_rw(old_arb, 1);   /* restore arb cntl reg */
	i_enable(old_int);
	xmfree(ptr, pinned_heap);
	return(EINVAL);				/* in a hole */
    }
    p = ptr;
    iomap.busaddr = (VPD_SPACE | offset) & 0xfffffffc; /* start on word boundary */
    iomap.key = IO_MEM_MAP;
    iomap.flags = 0;
    iomap.size = 0x200000;		/* size of vpd/feat rom space */
    iomap.bid = REALMEM_BID;
    addr = (caddr_t) iomem_att(&iomap);
    addr += (offset & 3);		/* add non-word align offset */
    while (sz > 0) {
	cnt = 4 - ((uint)addr & 3);		/* handle non-word align */
        cnt = (cnt > sz) ? sz : cnt;		/* size limit */
	for (i=0; i<cnt; i++)		
		*p++ = (uchar) *addr++;		/* byte access always works */
	sz -= cnt;
	if (iosize == 8)				 /* do we have holes? */
	    addr = (caddr_t) (((ulong)addr & 0xfffffff8) + iosize);
    }
    iomem_det(addr);
    md_select_slot(0);
    md_arbcntl_rw(old_arb, 1);   /* restore arb cntl reg */
    i_enable(old_int);
    if (!fkern)
        rc = copyout(ptr, buf, size); 
    else
        bcopy(ptr, buf, size);
    xmfree(ptr, pinned_heap);
    return(rc);
} /* md_read_vpd() */


/*
 * NAME: md_slot_bucp
 *
 * FUNCTION: Scan the buc tables for the matching slot number.
 *
 * EXECUTION ENVIRONMENT:  Process only.
 *
 * RETURN VALUE: Returns a pointer to the buc table, or 0 if not found
 *	or if device is not present..
 *
 * NOTES:  This routine should only be called on a PowerPC machine.
 *
 */
struct buc_info * 
md_slot_bucp(uint slot)
{
	struct ipl_cb *iplcb;
	struct ipl_directory *idir_ptr;
	struct buc_info *buc_ptr;
	ulong num_bucs, buc;

	iplcb = (struct ipl_cb *)vmker.iplcbptr;
	idir_ptr = (struct ipl_directory *)(&iplcb->s0);
	buc_ptr = (struct buc_info *)((char *)iplcb + 
		idir_ptr->buc_info_offset);
	num_bucs = buc_ptr->num_of_structs;
	for (buc = 0; buc < num_bucs; buc++)
	{	/* point to next buc structure */
		buc_ptr = (struct buc_info *)(((char *)iplcb + 
			idir_ptr->buc_info_offset) + 
			(buc * buc_ptr->struct_size));
		if ((buc_ptr->bscr_value & CFG_SLOT_MASK) 
		    == (slot & CFG_SLOT_MASK)) {
		    if (buc_ptr->device_type == 0xF) /* device present? */
			return(0);
		    else
			return(buc_ptr);
		}
	}
	return(0);
} /* md_slot_bucp */


/*
 * NAME: md_arbcntl_rw
 *
 * FUNCTION: Read or write the model 250 arbiter control register
 *
 * EXECUTION ENVIRONMENT:  Process only.
 *
 * Inputs: data = data to write
 *	   wflag = 0 to read, 1 to write
 *
 * RETURN VALUE: contents of arbiter register
 *
 * Note: This is model dependent code.
 *
 */
ulong
md_arbcntl_rw(ulong data, int wflag)
{
	ulong *real_addr;
	ulong rc=data;
	struct io_map iomap;

	iomap.busaddr = 0xFF00100C;
	iomap.key = IO_MEM_MAP;
	iomap.flags = 0;
	iomap.size = PAGESIZE;		/* minimum */
	iomap.bid = REALMEM_BID;
	real_addr = (ulong *) iomem_att(&iomap);
	if (wflag)
		*real_addr = data;	/* write */
	else
		rc = *real_addr;	/* read */
	iomem_det(real_addr);
	__iospace_sync();	/* make sure it's seen */
	return(rc);
}


/*
 * NAME: md_disable_pipe
 *
 * FUNCTION: Disable pipelining
 *
 * EXECUTION ENVIRONMENT:  Process only.
 *
 * Inputs: None
 *
 * RETURN VALUE: Previous value of arbiter control reg
 */
ulong
md_disable_pipe()
{
	ulong old;

	old = md_arbcntl_rw(0, 0);
	md_arbcntl_rw(old & ~1, 1);
	return(old);
}


/*
 * NAME: md_read_cfg
 *
 * FUNCTION: Read PowerPC config register
 *
 * EXECUTION ENVIRONMENT:  Process only.
 *
 * Inputs: None
 *
 * RETURN VALUE: Previous value of arbiter control reg
 *
 * Note: An iospace_sync is not necessary in this routine since it is performed
 *       by md_select_slot() and md_arbcntl_rw().
 */
int
md_read_cfg(uint slot, ulong offset, ulong incr, char *buf, int fkern)
{
	int rc = 0;
	int old_int;
	ulong old_arb;
	char *ptr;
	int cnt;
	caddr_t addr;
	union u {
	    uchar b;
    	    ushort s;
	    uint w;
	} u;
	struct io_map iomap;

	old_int = i_disable(INTMAX);
	old_arb = md_disable_pipe(); /* disable pipelining */
	if ((0 == md_select_slot(slot)) ||
	  (offset > MAX_CFGADDR)) {
	    md_arbcntl_rw(old_arb, 1);   /* restore arb cntl reg */
	    i_enable(old_int);
	    return(EINVAL);	/* illegal slot number or address */
	}
        iomap.busaddr = CFG_REGS | offset;
	iomap.key = IO_MEM_MAP;
	iomap.flags = 0;
	iomap.size = 0x2000;		/* size of config space */
	iomap.bid = REALMEM_BID;
	addr = (caddr_t) iomem_att(&iomap);
	switch(incr) {
	    case MV_BYTE:  u.b = *(uchar *)addr;
			   cnt = sizeof(u.b);
			   break;
	    case MV_SHORT: u.s = *(ushort *)addr;
			   cnt = sizeof(u.s);
			   break;
	    default:
	    case MV_WORD:  u.w = *(uint *)addr;
		 	   cnt = sizeof(u.w);
			   break;
	}
	iomem_det(addr);
	md_select_slot(0);
	md_arbcntl_rw(old_arb, 1);   /* restore arb cntl reg */
	i_enable(old_int);
	if (!fkern)
	    rc = copyout(&u, buf, cnt);
	else
	    bcopy(&u, buf, cnt);
	return(rc);
}


/*
 * NAME: md_write_cfg
 *
 * FUNCTION: Write PowerPC config register
 *
 * EXECUTION ENVIRONMENT:  Process only.
 *
 * Inputs: None
 *
 * RETURN VALUE: Previous value of arbiter control reg
 *
 * Note: An iospace_sync is not necessary in this routine since it is performed
 *       by md_select_slot() and md_arbcntl_rw().
 */
int
md_write_cfg(uint slot, ulong offset, ulong incr, char *buf, int fkern)
{
	int old_int;
	ulong old_arb;
	caddr_t addr;
	int cnt, rc;
	union u {
	    uchar b;
    	    ushort s;
	    uint w;
	} u;
	struct io_map iomap;

	switch (incr) {
	    case MV_BYTE:   cnt = sizeof(char);
			    break;
	    case MV_SHORT:  cnt = sizeof(short);
			    break;
	    default:
	    case MV_WORD:   cnt = sizeof(int);
			    break;
	}
	if (!fkern) {
	    if (rc = copyin(buf, &u, cnt))
	        return(rc);
	} else
	    bcopy(buf, &u, cnt);
	old_int = i_disable(INTMAX);
	old_arb = md_disable_pipe(); /* disable pipelining */
	if ((0 == md_select_slot(slot))
	  || (offset > MAX_CFGADDR)) {
	    md_arbcntl_rw(old_arb, 1);   /* restore arb cntl reg */
	    i_enable(old_int);
	    return(EINVAL);	/* illegal slot number or address */
	}
        iomap.busaddr = CFG_REGS | offset;
	iomap.key = IO_MEM_MAP;
	iomap.flags = 0;
	iomap.size = 0x2000;		/* size of config space */
	iomap.bid = REALMEM_BID;
	addr = (caddr_t) iomem_att(&iomap);
	switch(incr) {
	    case MV_BYTE:   *(uchar *)addr = u.b;
			    break;
	    case MV_SHORT:  *(ushort *)addr = u.s;
			    break;
	    default:
	    case MV_WORD:   *(uint *)addr = u.w;
			    break;
	}
	iomem_det(addr);
	md_select_slot(0);
	md_arbcntl_rw(old_arb, 1);   /* restore arb cntl reg */
	i_enable(old_int);
	return(0);
}

#endif	/* _RS6K_UP_MCA */
