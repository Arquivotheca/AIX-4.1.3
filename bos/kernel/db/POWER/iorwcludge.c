static char sccsid[] = "src/bos/kernel/db/POWER/iorwcludge.c, sysdb, bos411, 9428A410j 6/15/90 18:06:38";

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:  Sior_h, Sior_w
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

# include <sys/types.h>

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */


/* Read and write just one byte. */
extern	int read_align();		/* Read a byte.		*/
#define READBYTE(a) (read_align((a),1) & 0xff)

/*
 * FUNCTION: 
 *   These procedures simply read and write half words and words to
 *   get around a hardware bug in the family 1 adapters.
 *                                                                    
 */

/* Read half word */
int
Sior_h(addr)
caddr_t addr;
{
	return((READBYTE(addr)<<8) | READBYTE(addr+1));
}

/* Read full word */
int
Sior_w(addr)
caddr_t addr;
{
	return(	(READBYTE(addr)<<24) | (READBYTE(addr+1)<<16) |
		(READBYTE(addr+2)<<8) | READBYTE(addr+3) );
}
