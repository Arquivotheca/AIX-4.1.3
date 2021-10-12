static char sccsid[] = "@(#)42	1.3  src/bos/kernel/lib/libsysp/listmgr.c, libsysp, bos411, 9428A410j 6/16/90 02:40:04";
/*
 * COMPONENT_NAME:  (LIBSYS) - Region List Manager routines
 *
 * FUNCTIONS: 	reg_init(), reg_alloc(), reg_free(), reg_clear(),
 *		reg_avail()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   



#include <sys/cblock.h>
#include <sys/device.h>
#include <sys/dma.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/listmgr.h>

/*
 * NAME: reg_init()
 *                                                                    
 * FUNCTION: 
 * 	Create a region management structure.  How this service is
 *	used is entirely up to the caller.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This must be called on the process thread.
 *                                                                   
 * NOTES:
 *	Inputs:
 *		Base Address - starting base address of region
 *
 *		Number of regions 
 *
 *		log of size of each member in region
 *
 *	Note that these parameters may not actually contain the
 *	init routine passed values if the driver is breaking its
 *	address space across several region management lists.
 *
 *	External References:
 *		xmalloc() - kernel memory allocation service
 *		pinned_heap - kernel pinned memory pool
 *
 * RECOVERY OPERATION:	None
 *
 *
 * DATA STRUCTURES:
 *	 
 *
 * RETURNS:
 * 	Pointer to a region control structure, (t_reg_list *) if
 * 	successful, NULL on failure.  Call will fail if either
 * 	memory cannot be obtained or the region length is zero.
 */  


t_reg_list   *reg_init( unsigned char      *p_region,
                        unsigned            num_region,
                        unsigned            l2_regsize )
{
register t_reg_list             *p_reg;
register unsigned char          *p_avail;

        if ( num_region == 0 )
                return ( (t_reg_list *)NULL );
        p_reg = (t_reg_list *)xmalloc((uint)(sizeof(t_reg_list) + 
				num_region - 1), (uint)2, pinned_heap);
        if ( p_reg == (t_reg_list *)NULL )
                return ( p_reg );                       /* Is NULL */
        p_reg->_p_region = p_region;
        p_reg->_n_region = num_region;
        p_reg->_l2rsize = l2_regsize;
        p_reg->_rsize = 1 << l2_regsize;
        for ( p_avail = p_reg->_free; num_region; --num_region )
                *p_avail++ = REG_FREE;
        return ( p_reg );
}


/*
 * NAME: reg_alloc()
 *                                                                    
 * FUNCTION: 
 * 	Locate contiguous regions for mapping a region of the
 *	specified size.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This may be called on the process thread or on the OFLV.
 *                                                                   
 * NOTES:
 *	Inputs:
 *		Region List Pointer - pointer to a region list management
 *			structure created by the reg_init service.
 *
 *		Block Size - Size of the region which must occupy
 *			a contiguous address space.
 *
 * RECOVERY OPERATION:	None
 *
 *
 * DATA STRUCTURES:
 *	 Modifies the callers t_reg_list structure passed in.
 *
 * RETURNS:
 *	Address - Base adress of the allocated contiguous region.
 *		Function returns a NULL pointer if a contiguous
 *		region of the requested size cannot be allocated with
 *		the regions presently free.
 */  

unsigned char *reg_alloc( t_reg_list  *p_reg,
                          unsigned     block_size )
{
register unsigned char          *p_avail = p_reg->_free;
register int                    length;
int                             reg_req;

        reg_req = ( block_size + ( p_reg->_rsize - 1 ) ) >> p_reg->_l2rsize;
        {
        register int            current;

                for ( length = p_reg->_n_region; length; length-- )
                {
                        if ( length < ( current = reg_req ) )
                                return ( (unsigned char *)NULL );
                        while ( *p_avail && current )
                        {
                                ++p_avail; --current;
                        }
                        if ( !current )
                                break;
                        ++p_avail;
                }
        if ( !length )
                return ( (unsigned char *)NULL );
        }

        {
        register unsigned char  *p_temp = ( p_avail -= reg_req );

        for ( length = reg_req; length; length-- )
                *p_temp++ = REG_USED;
        }

        {
        register unsigned char  *p_region = p_reg->_p_region;

        return ( p_region + ( ( p_avail - p_reg->_free ) << p_reg->_l2rsize ) );
        }
}

/*
 * NAME: reg_free()
 *                                                                    
 * FUNCTION: 
 *	Return regions to the free list for reallocation.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This may be called on the process thread or on the OFLV.
 *                                                                   
 * NOTES:
 *	Inputs:
 *		Region List Pointer - Pointer to a previously allocated
 *			region management structure.
 *
 *		Block Size - Size of the contiguous address region.
 *
 *		Address - Address returned by a reg_alloc
 *			call.  Any number of separate reg_free calls can
 *			be made to release the address space allocated
 *			by a single reg_alloc call (if desired...).
 *
 *
 * RECOVERY OPERATION:	None
 *
 *
 * DATA STRUCTURES:
 *	 
 *	 Modifies the callers t_reg_list structure passed in.
 *
 * RETURNS:
 *	A zero is returned on normal completion.  If the
 *	address provided is NOT CONTROLLED by the regions
 *	in the passed reg_list structure, a -1 is returned.
 */  

/*
*/

int   reg_free( t_reg_list       *p_reg,
                unsigned          block_size,
                unsigned char    *p_region )
{
register unsigned char          *p_avail;
register int                    base,reg_req;

        reg_req = ( block_size + ( p_reg->_rsize - 1 ) ) >> p_reg->_l2rsize;
        base = ( p_region - p_reg->_p_region ) >> p_reg->_l2rsize;
        if ( ( base > p_reg->_n_region ) || ( base < 0 ) )
                return ( -1 );                  /* !owned by this reg_list */
        p_avail = &p_reg->_free [base];
        while ( reg_req-- )
                *p_avail++ = REG_FREE;
        return ( 0 );
}

/*
 * NAME: reg_clear()
 *                                                                    
 * FUNCTION: 
 *	Set all regions on the controlled list to available status.
 *	Useful for drivers which threshold their region lists.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES:
 *	Inputs:
 *		Region List Pointer - Pointer to a previously allocated
 *			region management structure.
 *
 * RECOVERY OPERATION:	None
 *
 *
 * DATA STRUCTURES:
 *	 
 *	 Modifies the callers t_reg_list structure passed in.
 *
 * RETURNS: None
 */  


void reg_clear( t_reg_list       *p_reg )
{
register unsigned char          *p_avail = p_reg->_free;
register int                    length = p_reg->_n_region;

        for ( ; length; length-- )
                *p_avail++ = REG_FREE;
        return;
}


/*
 * NAME: reg_avail()
 *                                                                    
 * FUNCTION: 
 *	Return the size of available address space on the list.
 *	This is useful for drivers supporting hardware with
 *	scatter/gather capabilities, in which case being contiguous
 *	is entirely superfluous.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES:
 *	Inputs:
 *		Region List Pointer - Pointer to a previously allocated
 *			region management structure.
 *
 * RECOVERY OPERATION:	None
 *
 *
 * DATA STRUCTURES:
 *	 
 *	 Modifies the callers t_reg_list structure passed in.
 *
 * RETURNS:
 *	The number of free, unallocated bytes of address space.
 *	This varies between 0 and p_reg->_n_region times p_reg->_rsize.
 *	No error return is possible.
 */

int  reg_avail( t_reg_list       *p_reg )
{
register unsigned char          *p_avail = p_reg->_free;
register int                    length = p_reg->_n_region;
register int                    free_cnt = 0;

        while ( length-- )
                if ( *p_avail++ == REG_FREE )
                        ++free_cnt;
        return ( free_cnt << p_reg->_l2rsize );
}
