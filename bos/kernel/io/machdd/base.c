static char sccsid[] = "@(#)51	1.2  src/bos/kernel/io/machdd/base.c, machdd, bos411, 9428A410j 11/20/92 09:58:31";
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS: baseload
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/types.h>
#include <sys/vmker.h>
#include <sys/malloc.h>

extern caddr_t base_conf_start;
extern caddr_t base_conf_end;

/*
 * NAME: baseload
 *                                                                    
 * FUNCTION: load base customize info
 *                                                                    
 * EXECUTION ENVIRONMENT: Initialization only
 *                                                                   
 * NOTES: mallocs a buffer and copies the base customize info
 *	  into it.
 *
 * RETURNS: pointer to allocated buffer containing base config info
 */  

caddr_t
baseload(long *bsize)
{
    register long tsize;
    register caddr_t ibase, obase;
    register int vmid;

    tsize = base_conf_end - base_conf_start;
    if (tsize == 0)
       return 0;

    obase = (caddr_t) malloc(tsize);
    if (!obase)
       return 0;
    
    ibase = vm_att(vmker.bconfsrval, 0);
    if (!ibase)
       return 0;
    
    bcopy(ibase, obase, tsize);

    vm_det(ibase);
    vmid = vm_vmid(vmker.bconfsrval);
    vms_delete(vmid);

    *bsize = tsize;
    return obase;
}
