static char sccsid[] = "@(#)58	1.1.1.7  src/bos/kernext/rcm/rcmbusacc.c, rcm, bos411, 9437A411a 9/8/94 17:52:20";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Bus Access Keys
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991-1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>                    /* includes for all lft related data */
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/user.h>			/* user structure */
#include <sys/adspace.h>		/* address space stuff	*/
#include <sys/lockl.h>			/* lock stuff  */
#include <sys/sleep.h>
#include <sys/ioacc.h>
#include <sys/signal.h>
#include <sys/syspest.h>
#include "gscsubr.h"			/* functions */
#include "rcm_mac.h"
#include "rcmhsc.h"
#include "xmalloc_trace.h"



BUGVDEF(dbg_domain_init,1);	
BUGVDEF(dbg_get_key,1) ;
BUGVDEF(dbg_put_key,1) ;
BUGVDEF(dbg_keys_init,1) ;

void  put_busmem_keys (gscComProcPtr  pcproc, gscDevPtr  pdev) ;
static void  init_busmem_keys (struct busmem_key  *pbus);


/* ------------------------------------------------------------------- 
   Initialize bus access authority for each domain.  This is done only 
   for gp leaders.

   Invoked by: dev_init()  ONLY

   NOTE:  This has to be done under the COMMON Anchor lock.

   ------------------------------------------------------------------- */

int domain_acc_authority_init (pcproc,pd)
    gscComProcPtr   pcproc;      /* each gp process has a com proc structure */
    struct phys_displays *pd;

{
    gscDevPtr	pdev;            /* pointer to device */
    int 	i, key;

    BUGLPR(dbg_domain_init,BUGNFO, ("Enter domain_access_authority_init\n"));

    SET_PDEV(pd,pdev);

    BUGLPR(dbg_domain_init,BUGNFO,
      ("domain_access_authority_init pdev 0x%x pdev->devHead.num_domains %d\n",
	pdev, pdev->devHead.num_domains));

    /* initialize the bus memory access keys in the domain array */
    for (i=0; i < pdev->devHead.num_domains; i++) 
    {
	/*
	 *  Allocate a key for us.  That is get next available unused key 
	 */
	key = get_busmem_key (pcproc, pdev);
	
	/* trace pid of gp leader, key allocated, and rcmProc pointer of gp 
           leader.  We mostly are interested in the key value.
        */ 
	RCM_TRACE(0x600,getpid(),key,pcproc);

	if (key < 0)		/* did one exist? */
	    break;

	/*
	 *  Store key assignment (and i/o range) in the proper places.
	 */

	pd->busmemr[i].auth_mask = key;

	/* 
	   Bus access is granted to a process if the bit that corresponds 
           to the key in the TCW is set in bits 16-23 of CSR 15.  We use the 
           assigned key to initialize a mask in pdom->auth.  This mask gets 
           used in fix_mstsave to grant or remove bus access.
	*/
	pdev->domain[i].auth = 1 << (15 - key);

	pdev->domain[i].range.range = pd->io_range;
    }

    if (i < pdev->devHead.num_domains)		/* failed? */
    {
	put_busmem_keys (pcproc, pdev);		/* free any ones we got */
	return (EINVAL);
    }

    /*
     *  Call the DD level so it can set up device related info.  A good,
     *  example is the d_protect calls.
     *
     */
    if (pdev->devHead.display->dev_init)
    {
	int  rc;

	BUGLPR(dbg_domain_init,BUGNFO,
		("domain_access_authority_init call DDdev_init\n"));

	if (rc = (pdev->devHead.display->dev_init) (pdev))
	{
	    BUGLPR(dbg_domain_init,BUGNFO,
		("domain_access_authority_init DDdev_init error\n"));

	    put_busmem_keys (pcproc, pdev);	/* free any keys we got */
	    return (EIO);
	}
    }

    BUGLPR(dbg_domain_init,BUGNFO, ("Exit domain_access_authority_init\n"));

    return (0);
}


/*
 *  get_busmem_key - Get a unique key for bus memory access control.
 *
 *  This function will allocate the database if it does not exist.
 *  This function is to be called from domain_acc_authority_init ().
 *
 *  This function will return < 0 if no key is available.
 *
 *  NOTE:  This has to be done under the COMMON Anchor lock.
 */
static int  get_busmem_key (gscComProcPtr  pcproc, gscDevPtr  pdev)
{
    int  i, alloc;
    struct  busmem_key  *pbus;

    /*
     *  Make sure the common process key allocation database
     *  is allocated.  Note only gp leaders will have this data base
     */

    BUGLPR(dbg_get_key,BUGNFO, ("Enter get_busmem_key\n"));

    if (alloc = (pcproc->pBusKey == NULL))   /* if not allocated */ 
    {
	pcproc->pBusKey =
	    xmalloc (MAX_BUSMEM_KEYS * sizeof (struct busmem_key),
							3, pinned_heap);
	if (pcproc->pBusKey == NULL)
	    return (ENOMEM);

	/*  Initialize the data in the key database */
	init_busmem_keys (pcproc->pBusKey);
    }

    for (i=0,pbus=pcproc->pBusKey; i<MAX_BUSMEM_KEYS; i++,pbus++)
    {
	if (pbus->owner == NULL)
	    break;
    }
    if (i >= MAX_BUSMEM_KEYS)			/* no key? */
    {
	if (alloc)
	    xmfree ((caddr_t) pcproc->pBusKey, pinned_heap);
	return (-1);				/* no key rtn code */
    }

    pbus->owner = pdev;				/* assign ownership */

    BUGLPR(dbg_get_key,BUGNFO, ("Exit get_busmem_key key=%d\n",pbus->key));

    return (pbus->key);				/* return the key value */
}

/*
 *  put_busmem_keys - Return all keys for bus memory access control for this
 *		     device.
 *
 *  This function will deallocate the database if all keys become unowned.
 *  This function is to be called from dev_term.
 *
 *  This function returns nothing.
 *
 *  NOTE:  This has to be done under the COMMON Anchor lock.
 */
void  put_busmem_keys (gscComProcPtr  pcproc, gscDevPtr  pdev)
{
    int  i, all_free = 1;
    struct  busmem_key  *pbus;

    BUGLPR(dbg_put_key,BUGNFO, ("Enter put_busmem_keys owner=0x%x\n",pdev));


    for (i=0,pbus=pcproc->pBusKey; i<MAX_BUSMEM_KEYS; i++,pbus++)
    {
	if (pbus->owner == pdev)	/* is this one to be released */
        {                               /* release ownership */
	    pbus->owner = NULL;	

            /* trace pid of gp leader, returned key, device structure.
                We are only intersted in which key is freed. 
            */
	    BUGLPR(dbg_put_key,BUGNFO,
			("put_busmem_key 0x%x owner=0x%x\n",pbus->key, pdev));
            RCM_TRACE(0x601,getpid(), pbus->key,pdev);
	}

	if (pbus->owner != NULL)	/* is any key owned? */
	    all_free = 0;		/* yes */
    }


    if (all_free)			/* should we release database? */
    {
	/* We want to know if we free the memory for keys */
        RCM_TRACE(0x602,getpid(),all_free,pdev);

	xmfree ((caddr_t) pcproc->pBusKey, pinned_heap);
	pcproc->pBusKey = NULL;
    }

    BUGLPR(dbg_put_key,BUGNFO, ("Exit put_busmem_keys\n"));
}

/*
 *  init_busmem_keys - Initialize the busmem structure to the correct
 *		       set of unowned and available key values.
 *
 *  NOTE:  This can accommodate noncontiguous available key values by
 *  munging the definition loop in here.  Currently, keys 1-7 are used.
 *
 *  NOTE:  This is all supposed to be done under the COMMON lock.
 */
void  init_busmem_keys (struct busmem_key  *pbus)
{
    int  i, key_val;

    BUGLPR(dbg_keys_init,BUGNFO, ("Enter init_busmem_keys\n"));

    key_val = FIRST_BUSMEM_KEY_VAL;          /* value of first key */

    for (i=0; i<MAX_BUSMEM_KEYS; i++,pbus++)
    {
	pbus->key = key_val;		/* key definitions! */
	pbus->spare = 0;		/* not used at present */
	pbus->owner = NULL;		/* no owner */

	key_val++;                      /* next key's value */
    }

    BUGLPR(dbg_keys_init,BUGNFO, ("Exit init_busmem_keys\n"));
}
