static char sccsid[] = "@(#)94	1.7  src/bos/usr/ccs/bin/mkstr/mkstr.c, cmdprog, bos411, 9428A410j 6/15/90 22:53:13";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, copystr, fgetNUL, hashit, inithash, match, octdigit,
	      process
 *
 * ORIGINS: 26; 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <locale.h>

#if defined(NLS) || defined(KJI)
#include <NLchar.h>
#define	ungetchar(c)	ungetwc(c, stdin)
#else
#define	ungetchar(c)	ungetc(c, stdin)
#endif

char	*calloc();
/*
 * mkstr - create a string error message file by massaging C source
 *
 * Bill Joy UCB August 1977
 *
 * Modified March 1978 to hash old messages to be able to recompile
 * without addding messages to the message file (usually)
 *
 * Based on an earlier program conceived by Bill Joy and Chuck Haley
 *
 * Program to create a string error message file
 * from a group of C programs.  Arguments are the name
 * of the file where the strings are to be placed, the
 * prefix of the new files where the processed source text
 * is to be placed, and the files to be processed.
 *
 * The program looks for 'error("' in the source stream.
 * Whenever it finds this, the following characters from the '"'
 * to a '"' are replaced by 'seekpt' where seekpt is a
 * pointer into the error message file.
 * If the '(' is not immediately followed by a '"' no change occurs.
 *
 * The optional '-' causes strings to be added at the end of the
 * existing error message file for recompilation of single routines.
 */


#ifdef MSG
#include "mkstr_msg.h" 
#define MSGSTR(n,s) NLgetamsg(MF_MKSTR, MS_MKSTR, n, s) 
#else
#define MSGSTR(n,s) s
#endif

FILE	*mesgread, *mesgwrite;
char	*progname;
char	usagestr[] =	"usage: %s [ - ] mesgfile prefix file ...\n";
char	name[MAXPATHLEN], *np;

main(argc, argv)
	int argc;
	char *argv[];
{
	char addon = 0;

	setlocale(LC_ALL, "");
	argc--, progname = *argv++;
	if (argc > 1 && argv[0][0] == '-')
		addon++, argc--, argv++;
	if (argc < 3)
		fprintf(stderr, MSGSTR(USAGE, usagestr), progname), exit(1); /*MSG*/ 
	mesgwrite = fopen(argv[0], addon ? "a" : "w");
	if (mesgwrite == NULL)
		perror(argv[0]), exit(1);
#ifdef aiws
	fseek(mesgwrite, (long) 0, 2);
#endif
	mesgread = fopen(argv[0], "r");
	if (mesgread == NULL)
		perror(argv[0]), exit(1);
	inithash();
	argc--, argv++;
	strcpy(name, argv[0]);
	np = name + strlen(name);
	argc--, argv++;
	do {
		strcpy(np, argv[0]);
		if (freopen(name, "w", stdout) == NULL)
			perror(name), exit(1);
		if (freopen(argv[0], "r", stdin) == NULL)
			perror(argv[0]), exit(1);
		process();
		argc--, argv++;
	} while (argc > 0);
	exit(0);
}

process()
{
	register char *cp;

#if defined(NLS) || defined(KJI)
	register NLchar c;
#else
	register c;
#endif

	for (;;) {

#if defined(NLS) || defined(KJI)
		c = getwchar();
		if (c == (NLchar)EOF)
#else
		c=getchar();
		if (c == EOF)
#endif
			return;

#if defined(NLS) || defined(KJI)
		if (c != (NLchar)'e') {
			putwchar(c);
#else
		if (c != 'e') {
			putchar(c);
#endif
			continue;
		}
		if (match("error(")) {
			printf("error(");

#if defined(NLS) || defined(KJI)
			c = getwchar();
			if (c != (NLchar)'"')
				putwchar(c);
#else
			c = getchar();
			if (c != '"')
				putchar(c);
#endif
			else
				copystr();
		}
	}
}

match(ocp)
	char *ocp;
{
#if defined(NLS) || defined(KJI)
	NLchar c;
#else
	register c;
#endif
	register char *cp;

	for (cp = ocp + 1; *cp; cp++) {
#if defined(NLS) || defined(KJI)
		c = getwchar();
		if (c != (NLchar)*cp) {
#else
		c = getchar();
		if (c != *cp) {
#endif
			while (ocp < cp)
				putchar(*ocp++);
			ungetchar(c);
			return (0);
		}
	}
	return (1);
}

copystr()
{
	register c, ch;
	char buf[512];
	register char *cp = buf;
	long hashit();

	for (;;) {
		c = getchar();
		if (c == EOF)
			break;
		switch (c) {

		case '"':
			*cp++ = 0;
			goto out;
		case '\\':
			c = getchar();
			switch (c) {

			case 'b':
				c = '\b';
				break;
			case 't':
				c = '\t';
				break;
			case 'r':
				c = '\r';
				break;
			case 'n':
				c = '\n';
				break;
			case '\n':
				continue;
			case 'f':
				c = '\f';
				break;
			case '0':
				c = 0;
				break;
			case '\\':
				break;
			default:
				if (!octdigit(c))
					break;
				c -= '0';
				ch = getchar();
				if (!octdigit(ch))
					break;
				c <<= 7, c += ch - '0';
				ch = getchar();
				if (!octdigit(ch))
					break;
				c <<= 3, c+= ch - '0', ch = -1;
				break;
			}
		}
		*cp++ = c;
	}
out:
	*cp = 0;
	printf("%ld", hashit(buf, 1, NULL));
}

octdigit(c)
	char c;
{

	return (c >= '0' && c <= '7');
}

inithash()
{
	char buf[512];
	int mesgpt = 0;
	long hashit();

	rewind(mesgread);
	while (fgetNUL(buf, sizeof buf, mesgread) != 0) {
		hashit(buf, 0, mesgpt);
		mesgpt += strlen(buf) + 2;
	}
}

#define	NBUCKETS	511

struct	hash {
	long	hval;
	unsigned hpt;
	struct	hash *hnext;
} *bucket[NBUCKETS];

long hashit(str, really, fakept)
	char *str;
	char really;
	unsigned fakept;
{
	int i;
	register struct hash *hp;
	char buf[512];
	long hashval = 0;
	register char *cp;

	if (really)
		fflush(mesgwrite);
	for (cp = str; *cp;)
		hashval = (hashval << 1) + *cp++;
	i = hashval % NBUCKETS;
	if (i < 0)
		i += NBUCKETS;
	if (really != 0)
		for (hp = bucket[i]; hp != 0; hp = hp->hnext)
		if (hp->hval == hashval) {
			fseek(mesgread, (long) hp->hpt, 0);
			fgetNUL(buf, sizeof buf, mesgread);
#ifdef _DEBUG
			fprintf(stderr, "Got (from %d) %s\n", hp->hpt, buf);
#endif
			if (strcmp(buf, str) == 0)
				break;
		}
	if (!really || hp == 0) {
		hp = (struct hash *) calloc(1, sizeof *hp);
		hp->hnext = bucket[i];
		hp->hval = hashval;
		hp->hpt = really ? ftell(mesgwrite) : fakept;
		if (really) {
			fwrite((void *)str, (size_t)sizeof(char), strlen(str) + 1, mesgwrite);
			fwrite((void *)"\n", (size_t)sizeof(char), (size_t)1, mesgwrite);
		}
		bucket[i] = hp;
	}
#ifdef _DEBUG
	fprintf(stderr, "%s hashed to %ld at %d\n", str, hp->hval, hp->hpt);
#endif
	return (hp->hpt);
}

#include <sys/types.h>
#include <sys/stat.h>

fgetNUL(obuf, rmdr, file)
	char *obuf;
	register int rmdr;
	FILE *file;
{
	register c;
	register char *buf = obuf;

	while (--rmdr > 0 && (c = getc(file)) != 0 && c != EOF)
		*buf++ = c;
	*buf++ = 0;
	getc(file);
	return ((feof(file) || ferror(file)) ? 0 : 1);
}
