static char sccsid[] = "@(#)53	1.9  src/bos/usr/ccs/bin/makekey/makekey.c, cmdmisc, bos411, 9428A410j 5/24/91 18:45:10";
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
char	*crypt();

#include <stdio.h>

main()
{
	char key[8];
	char salt[2];
	int rckey;
	char *cp;

        /* Reads the first eight characters.  These can be any ASCII value. */
	rckey = read(0, key, (unsigned)8);

	/* If no characters were input, then don't produce the encrypted key. */
        if (rckey <= 0) 
		return(1);
	else {
		/* Reads the final two chacters. These	*
		 * will be placed at the beginning of the result. */
		read(0, salt, (unsigned)2);

		/* Use the salt and key(10 digits) to produce 13 char output. */
		if((cp = crypt(key, salt)) == NULL) {
			return(1);
		}
		write(1, cp, (unsigned)13);
		return(0);
	}
}
