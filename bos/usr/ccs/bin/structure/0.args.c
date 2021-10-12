static char sccsid[] = "@(#)49	1.4  src/bos/usr/ccs/bin/structure/0.args.c, cmdprog, bos411, 9428A410j 3/9/94 13:18:03";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: getargs, setsw
 *
 * ORIGINS: 26 27
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

#include <stdio.h>
#
#include "def.h"
int errflag;
FILE *infd;


int intcase=1, arbcase=0;
int exitsize=0;			/* max number of nodes to be left in loop without iterating */
int maxnode=400;	/* max number of nodes */
int maxhash=347;	/* prime number = size of hash table */
int progress=0;		/* if not 0, print line number every n lines, n = progress */
int labinit=10;			/* labels generated starting with labinit */
int labinc=10;			/* labels increase by labinc */
int inputform=0;		/* = 0 if freeform input, 1 if standard form input */
int debug=0;
int levbrk=1;	/* true implies multilevel breaks; false implies single-level breaks only */
int levnxt=1;	/* true implies multilevel nexts; false implies single-level nexts only */


int maxprogsw=12;		/* number of program switches which can be set */
char *progsw[]		= {"i", "a",
			"e", "m",
			"h", "p",
			"t", "c",
			"s", "d",
			"b", "n"
			};


int *swval[]		= {&intcase, &arbcase,
			&exitsize, &maxnode,
			&maxhash, &progress,
			&labinit, &labinc,
			&inputform, &debug,
			&levbrk, &levnxt
			};


char *getargs(argc, argv)
int argc; char *argv[];
	{
	int n, infile;
	infile = 0;

	for (n = 1; n < argc; ++n)
		{
		if (argv[n][0] == '-')
			setsw(&argv[n][1]);
		else
			{
			if (infile != 0) {
				error("", "", "");
				fprintf(stderr,
				 MSGSTR(MULTIN, "error multiple input files - using first one:  %s\n"),
					argv[infile]);
			}
			else
				infile = n;
			}
		}
	if (errflag)
		exit(1);
	if (!infile) faterr(MSGSTR(NOINFILE, "no input file"),"",""); /*MSG*/
	infd = fopen(argv[infile],"r");
	if (infd == NULL) {
		error("", "", "");
		fprintf(stderr, MSGSTR(CANTOPENINFILE, "error can't open input file: %s\n"),
			argv[infile]);
		exit(1);
	}
	return;
	}

setsw(str)
char *str;
	{
	int i, val, swnum;
#define maxtemp 15
	char temp[maxtemp];
	for (i = 0; 'a' <= str[i] && str[i] <= 'z'; ++i)
		{
		if (i >= maxtemp)
			{
			error("", "", "");
			fprintf(stderr, MSGSTR(INVSW, "error invalid switch: %s\n"),str);
			errflag = 1;
			}
		temp[i] = str[i];
		}
	temp[i] = '\0';

	swnum = find(temp,progsw,maxprogsw);
	if (swnum == -1)
		{
		error("", "", "");
		fprintf(stderr, MSGSTR(INVSW, "error invalid switch: %s\n"), str);
		errflag = 1;
		return;
		}
	if (str[i] == '\0')
		*(swval[swnum]) = !*(swval[swnum]);
	else
		{
		sscanf(&str[i],"%d",&val);
		*(swval[swnum]) = val;
		}
	}
