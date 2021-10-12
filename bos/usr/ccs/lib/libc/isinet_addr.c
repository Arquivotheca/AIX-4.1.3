static char sccsid[] = "@(#)77  1.4  src/bos/usr/ccs/lib/libc/isinet_addr.c, libcinet, bos411, 9428A410j 5/27/93 15:17:33";
/*
 * COMPONENT_NAME: LIBCINET isinet_addr.c
 *
 * FUNCTIONS: isinet_addr
 *
 * ORIGINS: 26  27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#include <ctype.h>
#include <sys/types.h>

/*
 * This routine determines whether a string is a four-part dotted decimal
 * Internet address.  Each part must be < 255.  RFC 1123 states that
 * "The host SHOULD check the string syntactically for a dotted-decimal
 * number before looking it up in the Domain Name System."
 */
isinet_addr(name)
char *name;
{
        register u_long val;
	register int base, parts;
        register char *cp;

	cp = name;

	/* see if it's a name or an address */
	parts = 0;
	while (*cp != '\0') {
        	val = 0; base = 10;
		if (*cp == '0') {
			if (*++cp == 'x' || *cp == 'X')
				base = 16, cp++;
			else
				base = 8;
		}
		while (*cp != '\0') {
			if (isdigit(*cp)) {
				val = (val * base) + (*cp - '0');
				cp++;
				continue;
			}
			if (base == 16 && isxdigit(*cp)) {
				val = (val << 4) + (*cp + 10 - (islower(*cp) ? 
					'a' : 'A'));
				cp++;
				continue;
			}

			/* must be a name... */
			break;	
		}
		if (*cp == '.') {
			if ((cp == name) || (*(cp-1) == '.') ||
			    (*(cp+1) == '\0') || val > 255 || (parts + 1) >= 4)
				return(0);
			++parts;
			val = 0;
			cp++;
			continue;
		}
		break;
	}
	++parts;

	/* if cp == 0, then we've parsed valid octal, decimal, or
	 * hex digits, our total number of parts has been <= 4,
	 * and each part has a value less than 255.  Now check that 
	 * the final part has a valid value.
	 */
	if (*cp == '\0') {
		switch (parts) {

		case 1:		/* a -- 32 bits */
			break;

		case 2:		/* a.b -- 8.24 bits */
			if (val > 0xffffff)
				return (0);
			break;

		case 3:		/* a.b.c -- 8.8.16 bits */
			if (val > 0xffff)
				return (0);
			break;

		case 4:		/* a.b.c.d -- 8.8.8.8 bits */
			if (val > 0xff)
				return (0);
			break;
		}
		return(1);
	}
	return(0);
}
