/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS:
 *      u_iomem_att, u_iomem_det
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/machine.h>
#include <sys/ioacc.h>
#include <sys/seg.h>
#include <sys/user.h>
#include <sys/systemcfg.h>
#include <sys/dispauth.h>
#include <sys/syspest.h>
#include <sys/inline.h>

extern struct businfo bid_table[];

/*
 * Table to convet mapping size to BAT BL bits.  Since size is
 * a power of two a count leading zero can be used to generate
 * an index into this table.
 */
static short bl_tab[] = {
        BT_256M, BT_128M, BT_64M, BT_32M, BT_16M, BT_8M, BT_4M,
        BT_2M, BT_1M, BT_512K, BT_256K, BT_128K };

#define WIMG_IMG	0x7

/*
 * NAME: u_iomem_att
 *
 * FUNCTION:
 *      u_iomem_att() kernel service is provided to map T = 0 non-switchable
 *      displays into a graphics thread's user mode context.  
 *      The mapped display is accessible by the thread in the user mode 
 *      until u_iomem_det() is called. 
 *
 * NOTES:
 *      This is an undocumented kernel serivce
 *
 * EXECUTION ENVIRONEMT:
 *      This may execute in the process environment
 *
 * RETURNS:
 *      success: 32 bit effective IO address
 *      fail:    NULL
 */

void *u_iomem_att(struct io_map *iop)
{
        uint bid;                       /* BID of IO 			   */
        uint index;                     /* table index of BID   	   */
        uint region;                    /* region in bus                   */
        uint size;                      /* size to map 			   */
        uint batu;                      /* upper bat contents              */
        uint batl;                      /* lower bat contents              */
	uint braddr;			/* real address of display region  */ 
	uint eaddr;                     /* eff address of display region   */
	uint eaddr1;                    /* starting eff address of display */ 
	struct busprt *busprtp;         /* busprt structure pointer        */
	adspace_t *adsp;                /* address space pointer           */
	
	if (!__power_pc())  return(NULL);
	bid = iop->bid;
	index = BID_INDEX(bid);
	region = BID_REGION(bid);
	size = iop->size;
	
        ASSERT(iop->key == IO_MEM_MAP);
        ASSERT(index < MAX_BID_INDEX);
	/* verify bid is a registered bus id 		*/
        ASSERT(BID_TYPE(bid_table[index].bid) == BID_TYPE(bid));
        ASSERT(BID_NUM(bid_table[index].bid) == BID_NUM(bid));
	/* verify region is a valid region   		*/
        ASSERT(bid_table[index].num_regions > region);
	/* verify that 4K <= size <= 256M    		*/
        assert(size >= PAGESIZE && size <= SEGSIZE);
	/* verify that size is power of 2    		*/
	ASSERT(size == (0x1 << (31 - clz32(size))));

	braddr = (uint)bid_table[index].ioaddr[region] + iop->busaddr; 
	/* verify that braddr is on size boundary	*/
	ASSERT((braddr & ~(size -1)) == braddr);

	adsp = getadsp();
	/* For 601, map the region by T = 1, buid 7f */
	if (__power_601())
	{
	    eaddr =
		as_att(adsp, BUID_7F_SRVAL(braddr), braddr & (SEGSIZE-1));
	    return(eaddr);
	}

	/* For non-601 ppc machine, map the region by bats */

	eaddr = eaddr1 = as_att(adsp,INV_PB_SEGREG_PPC,braddr);
	if (eaddr == NULL) return(NULL);
	
	if (size < MIN_BAT_SIZE)
	{
	    size = MIN_BAT_SIZE;
	    eaddr = eaddr & ~(MIN_BAT_SIZE - 1);
	    braddr = braddr & ~(MIN_BAT_SIZE - 1);
        }
	
	/* compute (batu, batl) based on eaddr, braddr, flags, and size */
	batu = DBATU(eaddr, bl_tab[clz32(size) - 3], BT_VP);

	batl = DBATL(braddr, WIMG_IMG,
                     (iop->flags & IOM_RDONLY) ? BT_RDONLY : BT_WRITE);
	
	/* allocate busprt structure and initialize it */
	busprtp =
		(struct busprt *)xmalloc(sizeof(struct busprt), 4, pinned_heap);
	if (busprtp == NULL)
	{
		as_det(adsp, eaddr1);
		return(NULL);
	}
					 
        BUSPRT_TYPE(busprtp) = _BUSPRT_BAT;
        BUSPRT_EADDR(busprtp) = eaddr;
	BUSPRT_BAT_BATU(busprtp) = batu;
	BUSPRT_BAT_BATL(busprtp) = batl;

        /* register the busprt structure for the calling thread */
	if (reg_display_acc(busprtp) == -1)
	{
	    as_det(adsp, eaddr1);
	    xmfree(busprtp,pinned_heap);
	    return(NULL);
        }

	/* grant the display to the calling thread */			 
	grant_display_owner(busprtp);
	return(eaddr1);
}


/*
 * NAME: u_iomem_det
 *
 * FUNCTION:
 *      u_iomem_det() kernel service releases user mode access to a T = 0
 *      non-switchable display.
 *
 * NOTES:
 *      This is an undocumented kernel serivce
 *
 * EXECUTION ENVIRONEMT:
 *
 *      This may execute in the processes environment
 *
 * RETURNS:
 *      None
 */

void u_iomem_det(void *ioaddr)
{
	uint  eaddr;
	uint  segaddr;
	struct busprt *bp;
	struct gruprt *gp;
	
	ASSERT(__power_pc());

	if (!__power_601())
	{
	    eaddr = (uint)ioaddr & ~(MIN_BAT_SIZE - 1);
	    segaddr = (uint)ioaddr & ~(SEGSIZE - 1);

	    /* find the busprt structure, unreg it, and free the memory	*/
	    for (gp = curthread->t_graphics; gp; gp = gp->gp_next)
	    {
		 if (gp->gp_type ==  _BUSPRT_BAT && gp->gp_eaddr ==  segaddr)
		 {
		     bp = gp->gp_busprt; 
		     if (bp->bp_eaddr == eaddr)
		     {
			 unreg_display_acc(bp);
                         xmfree(bp,pinned_heap);
			 break;
		     }
		     /* each segment may only be mapped by one T = 0
		      *	non-switchable display
		      */	
		     ASSERT(0);   
                 }
	    }
	    ASSERT(gp);
	}

	/* free the segment in the address space */
	as_det(&u.u_adspace, ioaddr);
}
		
