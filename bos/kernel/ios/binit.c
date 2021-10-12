static char sccsid[] =
"@(#)04 1.28 src/bos/kernel/ios/binit.c, sysios, bos411, 9428A410j 9/15/93 11:57:09";

/*
 * COMPONENT_NAME: (SYSIOS) Block I/O initialization
 *
 * FUNCTIONS: binit
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Level 1, 5 years, Bull confidental information
 */                                                                   
/*
 * @BULL_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: binit.c,v $
 * $EndLog$
 */


#include <sys/param.h>
#include <sys/buf.h>
#include <sys/sysconfig.h>
#include <sys/syspest.h>
#include <sys/var.h>
#include <sys/sleep.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include "bio.h"
#include "bio_locks.h"

extern struct buf	bfreelist;	/* head of available buffer pool*/
extern struct hbuf	*hbuf;		/* hash array			*/

#include <sys/ppda.h>			/* Each CPU has its own private */
					/* 'iodonelist', see in ppda.h */

#if defined(_POWER_MP) && defined(_FUNNELED)
extern IODONELIST_LOCK_DECLARE;		/* Declare the IODONE-list lock */
extern mpc_msg_t bio_mpc_msg;		/* Inter-CPU off-level IT */
extern STRATLIST_LOCK_DECLARE;		/* Declare the strategy-list lock */
extern mpc_msg_t strat_mpc_msg;		/* Inter-CPU off-level IT */
#endif

#if defined(_POWER_MP)
extern BIO_MUTEX_LOCK_DECLARE;		/* Declare the BIO module MUTEX lock */
#endif

#define	INIT_BUFHW	20		/* init buffer cache high-water mark*/

/*
 * NAME:	binit
 *
 * FUNCTION:	Initialize the buffer I/O system by freeing all buffers
 *	and setting all device hash buffer lists to empty.
 *
 * EXECUTION ENVIRONMENT:  This routine is called at system initialization
 *	   by an entry in a pointer-to-function array defined in <sys/init.h>
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  none
 *
 * EXTERNAL PROCEDURES CALLED: cfgnadd, xmalloc, palloc, i_init, locking stuff
 */
void
binit()
{
	register struct buf	*bp;
	register struct buf	*dp;
	register unsigned	i;
	register char		*buf_pool;
	register int		bufsiz;	/* buffer pool size		*/
	register int		hdsiz;	/* buffer header pool size	*/
	register int		hbufsiz;/* hash buffer array size	*/
	register int		rc;	/* i_init return code		*/
	struct var		new_var;/* var struct w/ buffer cache size*/
	extern struct cfgncb	buf_cfgcb;/* config notification ctl blk*/
	extern struct buf	*buf_hdr;/* ptr to buffer header cache	*/
	extern caddr_t		buffer;	/* ptr to buffer cache		*/
	extern void		iodone_offl();/* I/O done off-level interrupt*/
#if defined(_POWER_MP) && defined(_FUNNELED)
	extern void		strat_offl();/* I/O done off-level interrupt*/
#endif
	extern int		bufhw_xtnd();/* config notification routine*/

	BIO_MUTEX_LOCK_ALLOC();		/* Allocate the BIO module MUTEX lock */
	BIO_MUTEX_LOCK_INIT();		/* Initialize the lock */

#if defined(_POWER_MP) && defined(_FUNNELED)
	IODONELIST_LOCK_ALLOC();	/* Allocate the IODONE-list lock */
	IODONELIST_INIT();		/* Initialize the IODONE-list lock */
					/* Register the X-CPU off-level IT */
	bio_mpc_msg = mpc_register(INTIODONE, iodone_offl);

	STRATLIST_LOCK_ALLOC();		/* Allocate the strategy-list lock */
	STRATLIST_INIT();		/* Initialize the strategy-list lock */
					/* Register the X-CPU off-level IT */
	strat_mpc_msg = mpc_register(INTIODONE, strat_offl);
#endif
	/*
	 * Off-level IT cannot migrate to otherprocessors --
	 * Each CPU has its own private 'iodonelist', see in ppda.h
	 */
	for (i=0; i<MAXCPU; i++)
		ppda[i] . iodonelist = NULL;

	dp = &bfreelist;
	dp->b_forw = dp->b_back = dp->av_forw = dp->av_back = dp;
	dp->b_event = EVENT_NULL;

	/*
	 * Allocate buffer headers from kernel heap.
	 */
	hdsiz = sizeof(struct buf) * NBUF;
	bp = (struct buf *)(xmalloc(hdsiz, 2, kernel_heap));
	assert(bp != NULL);
	buf_hdr = bp;

	/*
	 * Allocate buffers from kernel heap & make them page-boundary aligned.
	 */
	bufsiz = BSIZE * NBUF;
	buf_pool = (char *)palloc(bufsiz, PGSHIFT);
	assert(buf_pool != NULL);
	buffer = buf_pool;

	/*
	 * Get the current values of the configurable system parameters
	 * (the "v" var struct).  Set the buffer pool high-water mark to
	 * INIT_BUFHW.  This will cause the bufhw_xtnd() routine to be
	 * called; it will make available INIT_BUFHW #buf structs.
	 */
	new_var = v;
	new_var.v_bufhw = INIT_BUFHW;
	rc = bufhw_xtnd(CFGV_COMMIT, &v, &new_var);
	assert(rc == 0);
	v.v_bufhw = new_var.v_bufhw;

	/*
	 * Allocate hash buffer array and initialize pointers
	 */
	hbufsiz = sizeof(struct hbuf) * NHBUF;
	hbuf = (struct hbuf *)(xmalloc(hbufsiz, PGSHIFT, pinned_heap));
	assert(hbuf != NULL);

	for (i = 0; i < NHBUF; i++)
	{
		(hbuf + i)->b_forw =
			(hbuf + i)->b_back =
				(struct buf *)(hbuf + i);
	}

	/*
	 * Register block I/O's config notification routine on the
	 * cfgncb list.  This will cause the cfgncb.func routine to
	 * be called when the sysconfig SYS_SETPARMS system call is
	 * invoked, and will allow block I/O's buffer cache to be
	 * increased up to NBUF buffers/buf headers.
	 */
	buf_cfgcb.cbnext = NULL;
	buf_cfgcb.cbprev = NULL;
	buf_cfgcb.func = bufhw_xtnd;
	cfgnadd(&buf_cfgcb);

}  /* end binit */
