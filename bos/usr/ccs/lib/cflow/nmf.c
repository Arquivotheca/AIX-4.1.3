static char sccsid[] = "@(#)41	1.5  src/bos/usr/ccs/lib/cflow/nmf.c, cmdprog, bos411, 9428A410j 6/12/94 13:20:01";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 3 10 27 32
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

#include "stdio.h"
#include <locale.h>

#define SYMSTART 11
#define SYMCLASS 9

extern void exit();
extern char *strcpy(), *strncpy();

main(argc, argv)
char	*argv[];
{
	char name[BUFSIZ], buf[BUFSIZ], *fname = NULL, *pty;
	register char *p;
	int nsize, tysize, lineno;
        char type;

	setlocale(LC_ALL, "");
	strcpy(name, argc > 1? argv[1] : "???");
	if (argc > 2)
		fname = argv[2];
	else
		fname = "???";
	while (gets(buf))
	{
		p = buf;
		while (*p != ' ' && *p != '\t')
			++p;
		nsize = p - buf;
		while (*p == ' ' || *p == '\t')
			++p;
                type = *p;
		if (type == 'd' || type == 'D' || type == 't' || type == 'T')
		{					/* it's defined */
			strncpy(name, buf, nsize);
			name[nsize] = '\0';
			printf("%s = ", name);
                        if (type == 'd' || type == 'D')
			        printf("<%s %s>\n", fname, ".data");
                        else
			        printf("<%s %s>\n", fname, ".text");
		}
		else
			printf("%s : %.*s\n", name, nsize, buf);
	}
	exit(0);
	/*NOTREACHED*/
}
