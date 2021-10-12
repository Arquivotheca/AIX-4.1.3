static char sccsid[] = "@(#)98	1.1  src/bos/usr/bin/errlg/libras/stracpy.c, cmderrlg, bos411, 9428A410j 3/2/93 09:03:18";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: stracpy
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern char *malloc();

char *stracpy(str)
char *str;
{
	char *cp;
	int n;

	n = strlen(str);
	if((cp = malloc(n+1)) == 0) {
		perror("malloc on stracpy");
		genexit(2);
	}
	strcpy(cp,str);
	return(cp);
}

