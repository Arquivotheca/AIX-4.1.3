static char sccsid[] = "@(#)04	1.7.1.14  src/bos/kernel/io/machdd/hand.c, machdd, bos41J, bai15 2/14/95 17:08:15";
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: md_move, md_exp_hand, mdget, mdput
 *	      md_mapit, md_unmapit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/systm.h>
#include <sys/adspace.h>
#include <sys/except.h>
#include <sys/errno.h>
#include <sys/systemcfg.h>
#include "md_extern.h"
#ifdef _RS6K_SMP_MCA
#include <sys/param.h>
#include <sys/processor.h>
#include <sys/thread.h>
#include <sys/inline.h>
#endif /* _RS6K_SMP_MCA */



#define CSR    0	/* except struct as saved in io_exception() */
#define DSISR  1
#define SR_VAL 2
#define DAR    3

extern ulong NVRAM_size;
extern ulong NVRAM_base;

#ifdef _RS6K_SMP_MCA
#define DISKETTE_INT_LVL        4
extern int fd_mutex;
extern int pgs_SSGA_lvl;
extern void d_abort_fd();
#endif /* _RS6K_SMP_MCA */


struct md_bus_info md_bus[MD_BUS_TABSZ];	/* Bus info array */

void *md_mapit();
void md_unmapit();


/*
 * NAME: md_move
 *
 * FUNCTION: Copy data using byte or long transfers
 *
 * EXECUTION ENVIRONMENT: Process or interrupt; must be pinned
 *
 * NOTES:
 *
 * RETURNS:   Always 0
 *           
 *
 */
int
md_move(register struct d_st *data)
{
    long incr;
    volatile long *from, *to;
    volatile char *bfrom, *bto;
    volatile short *sfrom, *sto;

    from = data->source;
    to = data->target;
    switch(data->bcopy) {
	default:
	case MV_WORD:	if (data->size % 4 == 0) {
			    for (incr = 0; incr < data->size; incr += 4) 
				*to++ = *from++;
			    break;
			} /* else fall through */
	case MV_SHORT:	if (data->size % 2 == 0) {
			    sto = (short *) to;
			    sfrom = (short *) from;
			    for (incr = 0; incr < data->size; incr += 2) 
				*sto++ = *sfrom++;
			    break;
			} /* else fall through */
	case MV_BYTE:   bto = (char *) to;
			bfrom = (char *) from;
			for (incr = 0; incr < data->size; incr++) 
			    *bto++ = *bfrom++;
			break;
    }
    __iospace_sync();
    return 0;
}


/*
 * NAME: md_exp_hand
 *
 * FUNCTION: I/O exception handler
 *
 * EXECUTION ENVIRONMENT: Process or interrupt; must be pinned
 *
 * NOTES:
 *
 * RETURNS: 0 if no errors, else EIO
 *
 */
int
md_exp_hand(int (*io_func)(), caddr_t io_data, void *ptr)
{
    volatile int e_count = 5;
    int rc, iorc;
    volatile int oldrc;
    label_t jump_buf;
    volatile int handler = FALSE;
    ulong srval;

    srval = ldfsr((ulong)ptr >> 28);
    for (;;) {
        if ((rc = setjmpx(&jump_buf)) == 0) 
	{       /* got here by setting up handler */
		if (handler) /* we've already had some error before */
		{
		    if (srval == (long)csa->except[SR_VAL]) 
		    {
			if ((PIO_EXCP_DPRTY & csa->except[CSR]) && 
			    (--e_count >= 0))  
			{
			    /* having decremented e_count, set handler to 
			     * false and try iofunc() again
			     */
			    handler = FALSE; 
			}
			else 
			{ 
			    /* it wasn't a parity error, or we've already
			       retried 5 times, we're quitting */
			    iorc = EIO;
			    break;
			}
		    } 
		    else 
		    {  /* this was not our I/O error, srval doesn't match */
			iorc = EXCEPT_NOT_HANDLED;
			break;
		    } 
		   
		}

		/* 
		 * do I/O processing - I/O error will return us to setjumpx
		 * with rc != 0
		 */
		iorc = (*io_func)(io_data); 
		break;

	} /* got to setjumpx because of an exception occurring */
        else    if ((rc == EXCEPT_IO) || (rc == EXCEPT_IO_SLA)
                	|| (rc == EXCEPT_IO_SGA)) 
		{
	   		/*  PIO error or invalid BUID */
	    		handler = TRUE;
	    		oldrc = rc;    /* save old setjmpx code */ 
		}
		else {
	    		longjmpx(rc);    /* exception not handled here */
	    		/* NOTREACHED */
		}
    } /* end for */

    clrjmpx(&jump_buf);     /* remove jump_buf from except stack */
    if (iorc == EXCEPT_NOT_HANDLED) {
	longjmpx(oldrc);
	/* NOTREACHED */
    }
    return iorc;
} 


/*
 * NAME: mdget
 *
 * FUNCTION: Read from specified address using i/o exception handler
 *
 * NOTES:
 *    busnum: index into md_bus table
 *    region: region to be accessed - index into md_bus.bid[] array
 *    addr  : machine register address
 *    size  : size of transfer
 *    bcpy  : access type (byte, word, short)
 *    buf   : address of data buffer; if 0, a buffer is allocated
 *    err   : pointer for return code; if NULL, no return code
 *
 * RETURNS:
 *
 *    pointer to data area containing requested data
 *    NULL : if data area couldn't be allocated
 *
 */
char *
mdget(int busnum, int region, ulong addr, long size, char *buf, int bcpy, int *err)
{
    char *data;
    struct d_st io_data;
    int rc=0;
    void *ptr;
#ifdef _RS6K_SMP_MCA
    int intpri = INTMAX;
    cpu_t save_cpu;
#endif /* _RS6K_SMP_MCA */


    if (md_bus[busnum].map != NO_MAP)
        addr = addr & ~SEGREGMASK;	/* mask off upper nibble */
    if (buf)
	data = buf;
    else
	if (NULL == (data = (char *)MD_PG_ALLOC(size)))
            return NULL;
    if (INVALID == (ptr = md_mapit(busnum, region, addr, size))) {
	rc = EINVAL;
    } else {
    	io_data.iodir = IO_SOURCE;
	io_data.source = (long) ptr + addr;
	io_data.target = (long) data;
	io_data.size = size;
	io_data.bcopy = bcpy;

#ifdef _RS6K_SMP_MCA
/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 */
        if (pgs_SSGA_lvl == 2 && busnum == MD_NVRAM_IDX) {
                if (CSA->intpri == INTBASE) {
                        save_cpu = CURTHREAD->t_cpuid;
                        switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
			pin ((uint)data,size);
                        intpri = i_disable (DISKETTE_INT_LVL);
                }
                if ( fd_mutex)
                        d_abort_fd();
        }
#endif /* _RS6K_SMP_MCA */

	switch(md_bus[busnum].io_excpt) {
	    case TRUE:
	        rc = md_exp_hand(md_move, (caddr_t)&io_data, ptr);
	        break;
	    case FALSE:
	        rc = md_move((caddr_t)&io_data);
	        break;
	    default:
	        rc = EINVAL;
	        break;
        }
        md_unmapit(busnum, ptr);
    }
    if (err)
	*err = rc;

#ifdef _RS6K_SMP_MCA
	if (pgs_SSGA_lvl == 2 && busnum == MD_NVRAM_IDX && intpri == INTBASE) {
		i_enable (intpri);
		unpin ((uint)data,size);
		switch_cpu (save_cpu,RESET_PROCESSOR_ID);
	}
#endif /* _RS6K_SMP_MCA */

    return data;
} /* mdget() */


/*
 * NAME: mdput
 *
 * FUNCTION: Write to specified address using i/o exception handler
 *
 * NOTES:
 *    busnum: index into md_bus table
 *    region: region to be accessed - index into md_bus.bid[] array
 *    addr  : machine register address
 *    size  : size of transfer
 *    udata : pointer to data buffer
 *    bcpy  : access type (byte, word, short)
 *    err   : pointer for return code; if NULL, no return code
 *
 * RETURNS:    NONE
 *
 */
void
mdput(int busnum, int region, ulong addr, long  size, char *udata, int bcpy, int *err)
{

    struct d_st io_data;
    int rc=0;
    void *ptr;
#ifdef _RS6K_SMP_MCA
    int intpri = INTMAX;
    cpu_t save_cpu;
#endif /* _RS6K_SMP_MCA */


    if (md_bus[busnum].map != NO_MAP)
        addr = addr & ~SEGREGMASK;	/* mask off upper nibble */
    if (INVALID == (ptr = md_mapit(busnum, region, addr, size))) {
	rc = EINVAL;
    } else {
	io_data.iodir = IO_TARGET;
	io_data.source = (ulong)udata;
	io_data.target = (ulong)ptr + addr;
	io_data.size = size;
	io_data.bcopy = bcpy;

#ifdef _RS6K_SMP_MCA
/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 */
        if (pgs_SSGA_lvl == 2 && busnum == MD_NVRAM_IDX) {
                if (CSA->intpri == INTBASE) {
                        save_cpu = CURTHREAD->t_cpuid;
                        switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
			pin ((uint) udata,size);
                        intpri = i_disable (DISKETTE_INT_LVL);
                }
                if (fd_mutex)
                        d_abort_fd();
        }

#endif /* _RS6K_SMP_MCA */

	switch(md_bus[busnum].io_excpt) {
	    case TRUE:
		rc = md_exp_hand(md_move, (caddr_t)&io_data, ptr);
		break;
	    case FALSE:
		rc = md_move((caddr_t)&io_data);
		break;
	    default:
		rc = EINVAL;
		break;
	}
	md_unmapit(busnum, ptr);
    }

#ifdef _RS6K_SMP_MCA
    if (pgs_SSGA_lvl == 2 && busnum == MD_NVRAM_IDX && intpri == INTBASE) {
            i_enable (intpri);
	    unpin ((uint) udata,size);
            switch_cpu (save_cpu,RESET_PROCESSOR_ID);
    }
#endif /* _RS6K_SMP_MCA */

    if (err)
	*err = rc;


} /* mdput() */


/*
 * NAME: md_mapit
 *
 * FUNCTION: Map the region given md_bus and region index
 *
 * NOTES:
 *    busnum: index into md_bus table
 *    region: region to be accessed - index into md_bus.bid[] array
 *    addr:   offset into region - only used to determine size to map
 *    size:   size of access - only used to determine size to map
 * 
 * To adhere to iomem_att() size requirements, this routine 
 * calculates the requested size and rounds up a page.  The returned 
 * pointer must be added to the desired address.
 *
 * RETURNS:    INVALID if invalid region, else mapped pointer
 *
 */
void *
md_mapit(int busnum, int region, ulong addr, ulong size)
{
    void *ptr;
    struct io_map iomap;

    switch(md_bus[busnum].map) {
	case MAP_T1:
	    if (md_bus[busnum].bid[region] == INVALID)
		ptr = INVALID;
	    else
    	        ptr = (void *) io_att(md_bus[busnum].bid[region], 0);
	    break;
	case MAP_T0:
	    if (md_bus[busnum].bid[region] == INVALID)
		ptr = INVALID;
	    else {
	        iomap.key = IO_MEM_MAP;
	        iomap.flags = 0;
	        iomap.size = (addr + size + PAGESIZE) & ~(PAGESIZE-1);
	        iomap.bid = md_bus[busnum].bid[region];
	        iomap.busaddr = 0;
	        ptr = (void *) iomem_att(&iomap);
	    }
	    break;
	case NO_MAP:
	    ptr = (void *) 0;
	    break;
	default:
	    ptr = INVALID;
	    break;
    }
    return(ptr);
}


/*
 * NAME: md_unmapit
 *
 * FUNCTION: UnMap the region given md_bus index and mapped pointer
 *
 * NOTES:
 *    busnum: index into md_bus table
 *    ptr: mapped address
 *
 * RETURNS:    None
 *
 */
void
md_unmapit(int busnum, void *ptr)
{
    switch(md_bus[busnum].map) {
	case MAP_T1:
	    io_det(ptr);
	    break;
	case MAP_T0:
	    iomem_det(ptr);
	    break;
    }
}

