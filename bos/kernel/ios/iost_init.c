static char sccsid[] = "@(#)26	1.2  src/bos/kernel/ios/iost_init.c, sysios, bos411, 9428A410j 6/16/90 03:27:19";
/*
 * COMPONENT_NAME: (SYSIOS) I/O Subsystem
 *
 * FUNCTIONS: iostat_init
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/types.h>
#include <sys/param.h>
#include <sys/iostat.h>
#include <sys/malloc.h>
#include <sys/pin.h>
#include <sys/syspest.h>
#include <sys/var.h>
#include <sys/sysconfig.h>

extern	struct	iostat	iostat;	
extern	struct	cfgncb	iostat_cfgncb;
extern	iostrun_mon();

/*
 * NAME:  iost_init
 *
 * FUNCTION:  Creates and initializes the iostat table.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called at system initialization time.
 *      It cannot page fault.
 *
 * NOTES:  This routine is used to create and initialize the iostat
 *	structure.
 *
 * DATA STRUCTURES:  iostat
 *
 * RETURN VALUE DESCRIPTION:	none
 *
 * EXTERNAL PROCEDURES CALLED:  pin
 */
void
iost_init()
{
	register int		rc, iost_size;

	/*
	 * Pin iostat structure
	 */
	rc = pin(&iostat, sizeof(struct iostat)); 
	assert (rc == PIN_SUCC);

	iostat.dkstatp = NULL;		/* init dkstat ptr to indicate
					   null chain			*/
	iostat.dk_cnt = 0;		/* set disk count to 0		*/
	iostat_cfgncb.cbnext = NULL;	/* register notification for changes
					   to v.v_iostrun flag		*/
	iostat_cfgncb.cbprev = NULL;	   
	iostat_cfgncb.func = iostrun_mon;
	v.v_iostrun = TRUE;		/* set update statistics control on */
	cfgnadd(&iostat_cfgncb);
	return;

}  /* end iostat_init */
	
