static char sccsid[] = "@(#)36	1.1  src/bos/usr/ccs/lib/libc/des_crnull.c, libcdes, bos410, 9415B410a 4/26/90 22:23:25";
/* 
 * COMPONENT_NAME: (LIBCDES) Data Encryption Standards Library
 * 
 * FUNCTIONS:
 *
 * ORIGINS: 24 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                                  
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */
/*
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "(#)des_soft.c	1.4 88/05/20 4.0NFSSRC SMI;     from 1.3 88/02/08 (C) 1987 Sun Micro";
#endif
*/

/* to make this routine compile comment this dummy bad routine 
{
x
}
*/

cbc_crypt(key, buf, len, mode, ivec)
	char *key;
	char *buf;
	unsigned len;
	unsigned mode;
	char *ivec;	
{
return(2);
}

ecb_crypt(key, buf, len, mode)
	char *key;
	char *buf;
	unsigned len;
	unsigned mode;
{
return(2);
}

void
des_setparity(p)
	char *p;
{
;
}
