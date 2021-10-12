static char sccsid[] = "@(#)74	1.11  src/bos/usr/ccs/lib/libs/gettcbattr.c, libs, bos411, 9428A410j 6/16/90 02:31:17";
/*
 * COMPONENT_NAME: (LIBS) security library functions
 *
 * FUNCTIONS: gettcbattr, puttcbattr, endtcbattr
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
 
#include <usersec.h>
#include <sysck.h>
#include "libs.h"

/*
 * NAME: gettcbattr
 *                                                                    
 * FUNCTION: get tcb attributes
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This is a library routine. It returns the requested attribute values
 *	in malloc'd memory. A call to endtcbattr() will free all the memory.
 *                                                                   
 * DATA STRUCTURES:
 *	
 * RETURNS: 
 *
 */  
int
gettcbattr(register char *tcb, register char *atnam, register void *val,
		register int	type)
{
	if (chksessions() == 0)
	{
		if (setuserdb (S_READ))
			return (-1);
	}

	if (! tcb)
	{
		if (nextentry (val,SYSCK_TABLE))
			return -1;
		else
			return 0;
	}

	if (getuattr (tcb, atnam, val, type, SYSCK_TABLE))
		return -1;
	else
		return 0;
}

/*
 * NAME: puttcbattr
 *                                                                    
 * FUNCTION: put the specified tcb attribute
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  
int
puttcbattr (register char *tcb, register char *atnam,
	register void *val,register int type)
{
	if (chksessions() == 0)
	{
		if (setuserdb (S_READ | S_WRITE))
			return (-1);
	}

	if (putuattr (tcb, atnam, val, type, SYSCK_TABLE))
		return -1;
	else
		return 0;
}

/*
 * NAME: endtcbattr
 *                                                                    
 * FUNCTION: end the tcb attribute session
 *                                                                    
 * EXECUTION ENVIRONMENT: library
 *                                                                   
 * RETURNS: 
 */  

int
endtcbattr (void)
{
	return enduserdb ();
}

