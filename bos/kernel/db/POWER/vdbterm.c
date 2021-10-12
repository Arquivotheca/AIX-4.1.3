static char sccsid[] = "@(#)84	1.16  src/bos/kernel/db/POWER/vdbterm.c, sysdb, bos411, 9428A410j 7/22/93 09:54:19";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: getterm clrdsp
 *
 * ORIGINS: 27 83
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
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

#include <sys/types.h>
#include "dbdebug.h"			/* dbterm defines */
#include <sys/systemcfg.h>

/*
 * EXTERNAL PROCEDURES CALLED: 
 */

extern int d_ttyget();
extern ulong dbterm;


extern char debabend;

/*
 * NAME: getterm
 *                                                                    
 * FUNCTION: 
 *   Get a line from the terminal (like gets).
 *                                                                    
 * RETURN VALUE DESCRIPTION: A pointer to a string.
 *                                                                   
 */  
#define NL 0x0a

#define BUFSIZE 83
static char buf[BUFSIZE];

#define LEN (((ushort)buf[0])<<8)+((ushort)buf[1])

char *getterm()
{
	int i,i_opened_it=0,rc;
	ulong orig_segval;

	/* At present, only tty is supported. */
	if(!(dbterm & USE_TTY)) return(0);
#ifdef _SNOOPY
	if (__snoopy()){
		/* nothing */
	}
	else
#endif /* SNOOPY */
	if(debabend==OUT ) 
		/* we were called from kernel*/
		if((rc=d_ttyopen(dbterm & TTY_PORT))<=0){/* open port */
			return (char *) rc;
		}

#ifdef _POWER_MP
	d_ttyput('>');
	d_ttyput(cpunb + '0');
#endif /* _POWER_MP */

	d_ttyput('>');
	d_ttyput(' ');
	for(i=0;i<BUFSIZE-1;i++) {
		buf[i] = d_ttyget();
		if(buf[i] == '\n')
			break;
		else if(buf[i] == '\b') {
			buf[i--] = '\0';

			if (i >= 0)
			{
				buf[i--] = '\0';
				d_ttyput(' ');
				d_ttyput('\b');
			}
			else
				d_ttyput(' ');
		}
	}
	buf[i] = '\0';
	if(debabend==OUT) 
#ifdef _SNOOPY
		if (__snoopy()){
			/* nothing */
		}
		else
#endif /* SNOOPY */
			d_ttyclose(dbterm & TTY_PORT); 	/* close port*/

	return buf;
}

/*
 * NAME: clrdsp
 *                                                                    
 * FUNCTION: 
 *   This is currently a no-op.
 *                                                                    
 *                                                                   
 */  
void
clrdsp()
{
}
