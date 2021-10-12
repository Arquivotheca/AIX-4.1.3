static char sccsid[] = "@(#)96	1.1  src/bos/usr/ccs/lib/libmi/usage.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:41";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** usage.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>
#include <stdio.h>

extern	void	exit(   int status   );
extern	int	strncmp(   char * s1, char * s2, int n   );

void
usage (str)
	char	* str;
{
	if (!str  ||  !*str)
		str = "\n";
	else if (strncmp("usage:", str, 6) == 0)
		(void)fprintf(stderr, "%s", str);
	else
		(void)fprintf(stderr, "usage: %s %s", get_program_name(), str);
	while (*str)
		str++;
	if (str[-1] != '\n')
		(void)fprintf(stderr, "\n");
	exit(BAD_EXIT_STATUS);
	/*NOTREACHED*/
}
