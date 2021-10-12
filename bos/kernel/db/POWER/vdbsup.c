static char sccsid[] = "@(#)83	1.7  src/bos/kernel/db/POWER/vdbsup.c, sysdb, bos411, 9428A410j 6/16/90 03:06:11";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: setup_debaddr, debug_dec
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/seg.h>			/* virt. mem. segment info	*/
#include "debaddr.h"			/* Address descriptor		*/
#include "debvars.h"			/* debugger variables		*/

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */

extern int strlen();

/*
 * NAME: setup_debaddr
 *                                                                    
 * FUNCTION:
 *   Setup the debaddr structure.
 *                                                                    
 * RETURN VALUE: none
 */  

extern struct debaddr debaddr;		/* area pointed to on output	*/

struct debaddr *setup_debaddr(ad,vt)
int	ad;				/* effective address		*/
int	vt;				/* virt or real			*/
{
	int i;

	debaddr.addr = (caddr_t) ad;    /* Address (32-bit)           	 */ 
	debaddr.segid = debvars[IDSEGS+((ad)>>SEGSHIFT)].hv;  
	debaddr.virt = vt;              /* TRUE is virtual          	 */
	return(&debaddr);
}

/*
 * PL.8 Convert to decimal routine.
 */
void
debug_dec(in,out,l)
int in;
char *out;
int l;
{
	char s[12];
	int i,j;

	sprintf(s,"%d",in);
	for (i=strlen(s)-1, j=l-1; j>=0; i--, j--) {
		if (i>=0) out[j] = s[i];
		else out[j] = ' ';
	}
}
