static char sccsid[] = "@(#)11	1.4  src/bos/kernel/io/errdd_si.c, syserrlg, bos411, 9428A410j 5/13/94 16:50:37";

/*
 * COMPONENT_NAME: SYSERRLG   /dev/error pseudo-device driver
 *
 * FUNCTIONS: errinit, errnv_read
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * This file contains error logging device driver functions that
 * are required to be paged in only during system initialization.
 * 
 */

#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/dump.h>
#include <sys/erec.h>
#include "errdd.h"

int errinit();
static int errnv_read(char *ptr);

#ifdef _POWER_MP
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
extern Simple_lock errdd_lock;
#endif /* _POWER_MP */

extern struct cdt_head *errcdtf();
extern int dmp_add();
extern struct errc errc; 


/*
 * NAME:     errinit()
 * FUNCTION: Initialize /dev/error by allocating memory for the buffer,
 *           setting its read, write, start, and end pointers, initializing
 *           count variables, and if it exists, entering the nvram error 
 *           in the buffer.
 * RETURNS:  0 Success/Failure
 */

errinit()
{
	char *e_start;
	struct erec *ep;
	union errnv_buf errnv_buf;
	extern caddr_t xmalloc();
	extern caddr_t pinned_heap;

#ifdef _POWER_MP
	lock_alloc(&errdd_lock, LOCK_ALLOC_PIN, ERRDD_LOCK_CLASS, -1);
	simple_lock_init(&errdd_lock);
#endif /* _POWER_MP */
	if(ERRBUFSIZE == 0)
		return;
	if(errc.e_start)
		return;
	errc.e_size = ERRBUFSIZE;
	if((e_start = (char *)xmalloc(errc.e_size,2,pinned_heap)) == 0) {
		return;
	}
	errc.e_start  = e_start;
	errc.e_end    = e_start + ERRBUFSIZE;
	errc.e_inptr  = e_start;
	errc.e_outptr = e_start;
	errc.e_err_count = 0;
	errc.e_over_write_count = 0;
	errc.e_stale_data_ptr = errc.e_inptr;
	/*
	 * errnv_read will read the saved err_rec0 structure from nvram
	 * If the log buffer was emptied by a errread(),
	 *  the erec.erec_len value will be zeroed-out.
	 * Otherwise, this is an unrecorded (from a previous boot) errlog entry
	 *  and is put in the errlog this time.
	 */
	ep = &errnv_buf.erec;
	if(errnv_read((char*)ep) > 0 && ep->erec_len > 0) {
		if(ep->erec_len > sizeof(errnv_buf))
			ep->erec_len = sizeof(errnv_buf);
		bcopy(ep,errc.e_inptr,ep->erec_len);
                if (errc.e_inptr == errc.e_stale_data_ptr)
                        errc.e_stale_data_ptr += ep->erec_len;
		errc.e_inptr += ep->erec_len;
		errc.e_err_count++;
	}
	dmp_add(errcdtf);
}

/*
 * NAME:     errnv_read()
 * FUNCTION: Read the saved err_rec0 structure from nvram.
 * RETURNS:   1 Success
 *           -1 Failure
 */

static int
errnv_read(char *ptr)
{
	int rv;
	union errnv_buf errnv_buf;

	rv = nvread(3,(char*)&errnv_buf,0,sizeof(errnv_buf));
	if(rv != sizeof(errnv_buf)) 
		return(-1);
	bcopy(&errnv_buf,ptr,sizeof(errnv_buf));
	return(1);
}


