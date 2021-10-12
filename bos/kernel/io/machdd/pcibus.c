static char sccsid[] = "@(#)96	1.8  src/bos/kernel/io/machdd/pcibus.c, machdd, bos41J, 9516a 4/17/95 17:20:47";
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: pci_cfgrw
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifdef _RSPC_UP_PCI

#include <sys/errno.h>
#include <sys/lock_def.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/adspace.h>
#include <sys/seg.h>
#include <sys/inline.h>
#include "md_extern.h"

Simple_lock md_pci_lock;
extern struct md_bus_info md_bus[];

#define	CFG_ACCESS_SIZE	4

/*
 * NAME: pci_cfgrw
 *
 * FUNCTION: Read/Write PCI bus config space
 *
 * Notes: Gets the lock, then reads or writes the PCI config space for
 *	  the given bus and slot.  Only pinned (stack) buffers are used in the
 *	  inner locked loop since interrupts are disabled.
 * 
 * Inputs:
 *
 *  bid = BID for bus, region is not used
 *  md = pointer to mdio struct
 *    md->md_sla = device/function combination ((devnum * 8) + function)
 *    md->md_addr = address of config register to access
 *    md->md_size = number of bytes to read/write
 *    md->md_incr = access type: MV_BYTE, MV_WORD, MV_SHORT
 *    md->md_data = pointer to data buffer
 *    md->md_length not used
 *  read_flag = 1 for read, 0 for write 
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt (pinned routine)
 *
 * RETURN VALUE: 0 if successful
 *               EINVAL if map fails or slot/size invalid
 *		 EFAULT if copyin/copyout fails
 *
 */
int
pci_cfgrw(int bid, struct mdio *md, int write_flag)
{
    int rc=0;
    int i;
    ulong cfg_addr, cfg_addr_LE;
    int busnum;
    int iosize;
    int oldi;
    void *data;
    volatile void *c_ptr;
    volatile void *data_reg;
    volatile uint *addr_reg;
    int devnum, function;
    uint align, incr;
    struct io_map iomap;

/* Variables for workaround */
#define	IDAHO	0x14101500
    int workaround = FALSE;
    uint  id, revision, orig;

    if (INVALID == (busnum = md_bid2bus(bid)))
	return(EINVAL);

    /*
     * convert the md_sla to device number and function number
     */
    devnum = (md->md_sla >> 3) & 0x1F;
    function = md->md_sla & 0x7;

    switch(md->md_incr) {
        case MV_BYTE:  iosize = md->md_size;
		       align = 0x00;
		       incr = 1;
	   break;
        case MV_SHORT: iosize = md->md_size * sizeof(short);
		       align = 0x01;
		       incr = 2;
	   break;
        default:
        case MV_WORD:  iosize = md->md_size * sizeof(int);
		       align = 0x03;
		       incr = 4;
	   break;
    }

    if (md->md_addr & align)
	/*
	 * If this operation's starting register address does not
	 * match the alignment required by the increment
	 */
	return(EINVAL);
    if ((iosize + (md->md_addr & PCI_CFG_MASK)) > PCI_CFG_MASK+1)
	/*
	 * if this operation will cross into another devices configuration
	 * space
	 */
	return(EINVAL);

    data = md->md_data;		/* get pointer to data buffer */
    if (md_bus[busnum].bid[CFG_RGN] != INVALID) {
	/*
	 * This is a PCI 1.0 type access, so verify validity of device
	 * number
	 */
	if ((devnum < 11) || (devnum > 22)) {
		/*
		 * for type 1.0, we can only look at IDSEL #s 11 to 22
		 * for anything other than that, we must fake 0xFF as data
		 * read.
		 */
		if (!write_flag) {
			/* 
			 * for reads only 
			 */
	        	for (i=0 ; i < iosize; i++) 
				*(uchar *)data++ = 0xFF;
			return 0;
		} else {
			return EINVAL;
		}
	}

	/*
	 * The address = (1 << devnum) | (function << 8) | register
	 */
    	cfg_addr = (md->md_addr & PCI_CFG_MASK) | ((ulong)1 << devnum) |
		   (function << 8);
	/* 
	 * Attach to PCI Configuration Space
	 */
	iomap.key = IO_MEM_MAP;
	iomap.flags = 0;
	iomap.size = (cfg_addr + iosize + PAGESIZE) & ~(PAGESIZE-1);
	iomap.bid = md_bus[busnum].bid[CFG_RGN];
	iomap.busaddr = 0;
	c_ptr = (volatile void *) iomem_att(&iomap) + cfg_addr;
	
        oldi = disable_lock(INTMAX, &md_pci_lock);
        for (i=0 ; i < iosize; i+=incr) {
        	if (write_flag) {
		    switch (md->md_incr) {
			case MV_BYTE:
		    		*((volatile uchar *)c_ptr) = 
						*((uchar *)data);
				break;
			case MV_SHORT:
		    		*((volatile ushort *)c_ptr) = 
						*((ushort *)data);
				break;
			case MV_WORD:
			default:
		    		*((volatile ulong *)c_ptr) = 
						*((ulong *)data);
				break;
		    }
        	} else {
		    switch (md->md_incr) {
			case MV_BYTE:
		    		*((uchar *)data) = 
						*((volatile uchar *)c_ptr);
				break;
			case MV_SHORT:
		    		*((ushort *)data) = 
						*((volatile ushort *)c_ptr);
				break;
			case MV_WORD:
			default:
		    		*((ulong *)data) = 
						*((volatile ulong *)c_ptr);
				break;
		    }
		}
		c_ptr += incr;
		data  += incr;
    	}
        unlock_enable(oldi, &md_pci_lock);
	iomem_det((void *)c_ptr);

    } else {
	/*
	 * This is a PCI 2.0 type access
	 *	address = 0x80000000 | (hw_busnumber << 16) | (devnum << 11) |
	 *			(function << 8) | register
	 */
	cfg_addr = PCI_CFG_VALID | (md_bus[busnum].hw_busnum << 16) |
		(devnum << 11) | (function << 8) | (md->md_addr & PCI_CFG_MASK);

	/* 
	 * Attach to PCI I/O Space
	 */
	iomap.key = IO_MEM_MAP;
	iomap.flags = 0;
	iomap.size = SEGSIZE;
	iomap.bid = md_bus[busnum].bid[IO_RGN];
	iomap.busaddr = 0;
	c_ptr = (volatile void *) iomem_att(&iomap);
	addr_reg = (volatile uint *) (c_ptr + (ulong)md_bus[busnum].pci_base_addr);
	data_reg = (volatile void *) (c_ptr + (ulong)md_bus[busnum].pci_data_addr);

        oldi = disable_lock(INTMAX, &md_pci_lock);	/* serialize access */
	/*
	 * BEGIN IDAHO WORKAROUND
	 *	- following is a workaround for Pass 2 and below Idaho
	 *	  memory controllers.  This code should eventually be
	 *	  removed.
	 */
	/*
	 * Read the ID to see if this is an Idaho
	 */
	*addr_reg = (uint)0x00000080;
	__iospace_eieio();
	id = *((volatile uint *)data_reg);
	if (id == IDAHO) {
		/*
		 * Read the Revision ID to see if it is < pass 3
		 */
		*addr_reg = (uint)0x08000080;
		__iospace_eieio();
		revision = *((uchar *)data_reg);
		if (revision < 3) {
			/* 
			 * do the workaround
			 */
			workaround = TRUE;
			/*
			 * Disable Machine Check in register 0xBA
			 */
			*addr_reg = (uint)0xBA000080;
			__iospace_eieio();
			orig = *((volatile uchar *)(data_reg + 0x2));
			*((volatile uchar *)(data_reg + 0x2)) = (orig & 0xFE);
		}
	}
	/*
	 * END IDAHO WORKAROUND
	 */
	/*
	 * Write the starting config address to the address register
	 * (this handles the case if we start on a non-word aligned register
	 *  address)
	 */
	cfg_addr_LE = lbrx(&cfg_addr);	/* put in Little-Endian */
	*addr_reg = (uint)cfg_addr_LE;
	__iospace_eieio();

	for (i=0; i < iosize; i+=incr) {
		if ( !(cfg_addr & 0x3) ) {
			/* 
			 * the address has bumped to the next word-aligned 
			 * boundary, then we must write the address port
			 * again.
			 */
			cfg_addr_LE = lbrx(&cfg_addr);	/* Little-Endian */
			*addr_reg = (uint)cfg_addr_LE;
			__iospace_eieio();
		}
        	if (write_flag) {
		    switch (md->md_incr) {
			case MV_BYTE:
			*((volatile uchar *)(data_reg + (cfg_addr & 0x3))) = 
							*((uchar *)data);
			break;
			case MV_SHORT:
			*((volatile ushort *)(data_reg + (cfg_addr & 0x3))) = 
							*((ushort *)data);
			break;
			case MV_WORD:
			default:
			*((volatile ulong *)(data_reg + (cfg_addr & 0x3))) = 
							*((ulong *)data);
			break;
		    }
		} else {
		    switch (md->md_incr) {
			case MV_BYTE:
			*((uchar *)data) = *((volatile uchar *)(data_reg + 
							(cfg_addr & 0x3)));
			break;
			case MV_SHORT:
			*((ushort *)data) = *((volatile ushort *)(data_reg + 
							(cfg_addr & 0x3)));
			break;
			case MV_WORD:
			default:
			*((ulong *)data) = *((volatile ulong *)(data_reg + 
							(cfg_addr & 0x3)));
			break;
		    }
		}	
		cfg_addr+=incr;
		data+=incr;
	}

	/*
	 * BEGIN WORKAROUND....again
	 */
	if (workaround) {
		/*
		 * If we previously started a workaround, finish it off
		 */
		/*
		 * Clear Error Status Registers
		 */
		*addr_reg = (uint)0x07000080;
		__iospace_eieio();
		*((volatile char *)(data_reg + 3)) = 0xFF; /* Device Status */
		*addr_reg = (uint)0xC1000080;
		__iospace_eieio();
		*((volatile char *)(data_reg + 1)) = 0xFF; /* Error Detect 1*/
		*addr_reg = (uint)0xC5000080;
		__iospace_eieio();
		*((volatile char *)(data_reg + 1)) = 0xFF; /* Error Detect 2*/
		/*
		 * Now re-enable the Machine Check Pin
		 */
		*addr_reg = (uint)0xBA000080;
		__iospace_eieio();
		*((volatile uchar *)(data_reg + 0x2)) = orig;
	}
	/*
	 * END WORKAROUND
	 */
	unlock_enable(oldi, &md_pci_lock);
	iomem_det((void *)c_ptr);
    }


    return rc;
}


/*
 * NAME: md_bid2bus
 *
 * FUNCTION: Return bus number for given BID
 *
 * Inputs:  bid = BID for bus, region is not used
 *
 * EXECUTION ENVIRONMENT:  Process or interrupt (pinned routine)
 *
 * RETURN VALUE: bus number if successful
 *		 INVALID if bid not found
 *
 */
int
md_bid2bus(int bid)
{
    int busnum;

    bid = BID_ALTREG(bid, 0);			/* Mask off region field */
    for (busnum = 0; busnum < MD_MAX_BUS; busnum++)
	if (bid == BID_ALTREG(md_bus[busnum].bid[IO_RGN],0))
	    return(busnum);
    return(INVALID);
}

#endif	/* _RSPC_UP_PCI */
