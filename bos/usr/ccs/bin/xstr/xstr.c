static char sccsid[] = "@(#)35	1.9  src/bos/usr/ccs/bin/xstr/xstr.c, cmdprog, bos411, 9428A410j 4/14/94 12:51:36";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, fgetNUL, flushsh, found, hashit, inithash, istail, lastchr,
	      octdigit, onintr, process, prstr, savestr, xgetc, xsdotc, yankstr
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
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#define _ILS_MACROS      /* 139729 - use macros for better performance */
#include <sys/localedef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <locale.h>
#include <nl_types.h>

#ifdef MSG
#include "xstr_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_XSTR,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#if defined(NLS) || defined(KJI)
#include <NLchar.h>
#include <NLctype.h>
#endif
/*
 * xstr - extract and hash strings in a C program
 *
 * Bill Joy UCB
 * November, 1978
 */

#define	ignore(a)	((void) a)

char	*calloc();
off_t	tellpt;
off_t	hashit();
char	*mktemp();
int	onintr(void);
char	*savestr();
off_t	yankstr();

off_t	mesgpt;
char	*strings =	"strings";

int	cflg;
int	vflg;
int	readstd;

main(argc, argv)
	int argc;
	char *argv[];
{

	setlocale(LC_ALL, "");
#ifdef MSG
	catd = catopen(MF_XSTR, NL_CAT_LOCALE);
#endif
	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		register char *cp = &(*argv++)[1];

		argc--;
		if (*cp == 0) {
			readstd++;
			continue;
		}
		do switch (*cp++) {

		case 'c':
			cflg++;
			continue;

		case 'v':
			vflg++;
			continue;

		default:
			fprintf(stderr, MSGSTR(USAGE, "usage: xstr [ -v ] [ -c ] [ - ] [ name ... ]\n")); /*MSG*/
		} while (*cp);
	}
	if (signal(SIGINT, SIG_IGN) == SIG_DFL)
		ignore(signal(SIGINT, (void (*)(int))onintr));
	if (cflg || argc == 0 && !readstd)
		inithash();
	else
		strings = mktemp(savestr("/tmp/xstrXXXXXX"));
	while (readstd || argc > 0) {
		if (freopen("x.c", "w", stdout) == NULL)
			perror("x.c"), exit(1);
		if (!readstd && freopen(argv[0], "r", stdin) == NULL)
			perror(argv[0]), exit(2);
		process("x.c");
		if (readstd == 0)
			argc--, argv++;
		else
			readstd = 0;
	};
	flushsh();
	if (cflg == 0)
		xsdotc();
	if (strings[0] == '/')
		ignore(unlink(strings));
	exit(0);
}

char linebuf[BUFSIZ];

process(name)
	char *name;
{
	char *cp;
#if defined(NLS) || defined(KJI)
	NLchar c;
#else
	int c;
#endif
	register int incomm = 0;
	int ret;

	printf(MSGSTR(EXTERN, "extern char\txstr[];\n")); /*MSG*/
	for (;;) {
		if (fgets((char *)linebuf, (int)sizeof linebuf, stdin) == NULL) {
			if (ferror(stdin)) {
				perror(name);
				exit(3);
			}
			break;
		}
		if (linebuf[0] == '#') {
			if (linebuf[1] == ' ' && isdigit((int)linebuf[2]))
				printf(MSGSTR(LINE, "#line%s"), &linebuf[1]); /*MSG*/
			else
				printf("%s", linebuf);
			continue;
		}

#if defined(NLS) || defined(KJI)

		for (cp = linebuf; cp += NCdec(cp,&c),c; )	{
		switch (c) {
#else
		for (cp = linebuf; c = *cp++;) { switch (c) {
#endif
			
		case '"':
			if (incomm)
				goto def;
			if ((ret = (int) yankstr(&cp)) == -1)
				goto out;
			printf("(&xstr[%d])", ret);
			break;

		case '\'':
			if (incomm)
				goto def;
			putchar(c);
			if (*cp)
				putchar(*cp++);
			break;

		case '/':
			if (incomm || *cp != '*')
				goto def;
			incomm = 1;
			cp++;
			printf("/*");
			continue;

		case '*':
			if (incomm && *cp == '/') {
				incomm = 0;
				cp++;
				printf("*/");
				continue;
			}
			goto def;
		
def:
		default:

#if defined(NLS) || defined(KJI)
			putwchar(c);
#else
			putchar(c);
#endif
			break;
		} /* end switch */
		} /* end inner for loop */
	} /* end outer for loop */
out:
	if (ferror(stdout))
		perror("x.c"), onintr();
}

off_t
yankstr(cpp)
	register char **cpp;
{
	register char *cp = *cpp;

#if defined(NLS) || defined(KJI)
	NLchar c;
#else
	int c;
#endif
	register int ch;
	char dbuf[BUFSIZ];
	register char *dp = dbuf;
	register char *tp;


#if defined(NLS) || defined(KJI)

	while (cp += NCdec(cp,&c), c) {
#else
	while (c = *cp++) {
#endif
		switch (c) {

		case '"':
			cp++;
			goto out;

		case '\\':

#if defined(NLS) || defined(KJI)
			cp += NCdec(cp,&c);
#else
			c = *cp++;
#endif
			if (c == 0)
				break;
			if (c == '\n') {
				if (fgets((char *)linebuf, (int)sizeof linebuf, stdin) 
				    == NULL) {
					if (ferror(stdin)) {
						perror("x.c");
						exit(3);
					}
					return(-1);
				}
				cp = linebuf;
				continue;
			}
			for (tp = "b\bt\tr\rn\nf\f\\\\\"\""; ch = *tp++; tp++)

#if defined(NLS) || defined(KJI)
				if (c == (NLchar)ch) {
#else
				if (c == ch) {
#endif
					c = *tp;
					goto gotc;
				}
			if (!octdigit(c)) {
				*dp++ = '\\';
				break;
			}
			c -= '0';
			if (!octdigit(*cp))
				break;
			c <<= 3, c += *cp++ - '0';
			if (!octdigit(*cp))
				break;
			c <<= 3, c += *cp++ - '0';
			break;
		}
gotc:
#if defined(NLS) || defined(KJI)
		dp += NCenc(&c,dp);
#else
		*dp++ = c;
#endif
	}
out:
	*cpp = --cp;
	*dp = 0;
	return (hashit(dbuf, 1));
}

octdigit(c)
	char c;
{

#if defined(NLS) || defined(KJI)
	return (isascii((int)c) && isdigit((int)c) && c != '8' && c != '9');
#else
	return (isdigit(c) && c != '8' && c != '9');
#endif
}

inithash()
{
	char buf[BUFSIZ];
	register FILE *mesgread = fopen(strings, "r");

	if (mesgread == NULL)
		return;
	for (;;) {
		mesgpt = tellpt;
		if (fgetNUL(buf, sizeof buf, mesgread) == 0)
			break;
		ignore(hashit(buf, 0));
	}
	ignore(fclose(mesgread));
}

fgetNUL(obuf, rmdr, file)
	char *obuf;
	register int rmdr;
	FILE *file;
{
	register c;
	register char *buf = obuf;

	while (--rmdr > 0 && (c = xgetc(file)) != 0 && c != EOF)
		*buf++ = c;
	*buf++ = 0;
	return ((feof(file) || ferror(file)) ? 0 : 1);
}

xgetc(file)
	FILE *file;
{

	tellpt++;
	return (getc(file));
}

#define	BUCKETS	128

struct	hash {
	off_t	hpt;
	char	*hstr;
	struct	hash *hnext;
	short	hnew;
} bucket[BUCKETS];

off_t
hashit(str, new)
	char *str;
	int new;
{
	int i;
	register struct hash *hp, *hp0;

	hp = hp0 = &bucket[lastchr(str) & 0177];
	while (hp->hnext) {
		hp = hp->hnext;
		i = istail(str, hp->hstr);
		if (i >= 0)
			return (hp->hpt + i);
	}
	if ((hp = (struct hash *) calloc(1, sizeof (*hp))) == NULL) {
		perror("xstr");
		exit(8);
	}
	hp->hpt = mesgpt;
	hp->hstr = savestr(str);
	mesgpt += strlen(hp->hstr) + 1;
	hp->hnext = hp0->hnext;
	hp->hnew = new;
	hp0->hnext = hp;
	return (hp->hpt);
}

flushsh()
{
	register int i;
	register struct hash *hp;
	register FILE *mesgwrit;
	register int old = 0, new = 0;

	for (i = 0; i < BUCKETS; i++)
		for (hp = bucket[i].hnext; hp != NULL; hp = hp->hnext)
			if (hp->hnew)
				new++;
			else
				old++;
	if (new == 0 && old != 0)
		return;
	mesgwrit = fopen(strings, old ? "r+" : "w");
	if (mesgwrit == NULL)
		perror(strings), exit(4);
	for (i = 0; i < BUCKETS; i++)
		for (hp = bucket[i].hnext; hp != NULL; hp = hp->hnext) {
			found(hp->hnew, hp->hpt, hp->hstr);
			if (hp->hnew) {
				fseek(mesgwrit, hp->hpt, 0);
				ignore(fwrite((void *)hp->hstr, strlen(hp->hstr) + 1, (size_t)1, mesgwrit));
				if (ferror(mesgwrit))
					perror(strings), exit(4);
			}
		}
	if (fclose(mesgwrit) == EOF)
		perror(strings), exit(4);
}

found(new, off, str)
	int new;
	off_t off;
	char *str;
{
	if (vflg == 0)
		return;
	if (!new)
		fprintf(stderr, MSGSTR(FOUND, "found at %d:"), (int) off); /*MSG*/
	else
		fprintf(stderr, MSGSTR(NEW, "new at %d:"), (int) off); /*MSG*/
	prstr(str);
	fprintf(stderr, "\n");
}

#if defined(NLS) || defined(KJI)
prstr(cp)
	register char *cp;
{
	while (*cp)	{
		if (*cp < ' ')
			fprintf(stderr, "^%c", *cp + '`');
		else if (*cp == 0177)
			fprintf(stderr, "^?");
		else if (NCisprint(*cp))
			NLfprintf(stderr,"%C",*cp);
		else if (NCisshift(*cp))
			fprintf(stderr,"\\03o\n\\03o",*cp,*(cp+1));
		else
			fprintf(stderr,"\\03o",*cp);

		cp += NLchrlen(cp);
	}
}
#else
prstr(cp)
	register char *cp;
{
	register int c;

	while (c = (*cp++ & 0377))
		if (c < ' ')
			fprintf(stderr, "^%c", c + '`');
		else if (c == 0177)
			fprintf(stderr, "^?");
		else if (c > 0200)
			fprintf(stderr, "\\%03o", c);
		else
			fprintf(stderr, "%c", c);
}
#endif

xsdotc()
{
	register FILE *strf = fopen(strings, "r");
	register FILE *xdotcf;

	if (strf == NULL)
		perror(strings), exit(5);
	xdotcf = fopen("xs.c", "w");
	if (xdotcf == NULL)
		perror("xs.c"), exit(6);
	fprintf(xdotcf, MSGSTR(XDOTCF, "char\txstr[] = {\n")); /*MSG*/
	for (;;) {
		register int i, c;

		for (i = 0; i < 8; i++) {
			c = getc(strf);
			if (ferror(strf)) {
				perror(strings);
				onintr();
			}
			if (feof(strf)) {
				fprintf(xdotcf, "\n");
				goto out;
			}
			fprintf(xdotcf, "0x%02x,", c);
		}
		fprintf(xdotcf, "\n");
	}
out:
	fprintf(xdotcf, "};\n");
	ignore(fclose(xdotcf));
	ignore(fclose(strf));
}

char *
savestr(cp)
	register char *cp;
{
	register char *dp;

	if ((dp = (char *) calloc(1, strlen(cp) + 1)) == NULL) {
		perror("xstr");
		exit(8);
	}
	return (strcpy(dp, cp));
}

lastchr(cp)
	register char *cp;
{

	while (cp[0] && cp[1])
		cp++;
	return (*cp);
}

istail(str, of)
	register char *str, *of;
{
	register int d = strlen(of) - strlen(str);

	if (d < 0 || strcmp(&of[d], str) != 0)
		return (-1);
	return (d);
}

onintr(void)
{

	ignore(signal(SIGINT, SIG_IGN));
	if (strings[0] == '/')
		ignore(unlink(strings));
	ignore(unlink("x.c"));
	ignore(unlink("xs.c"));
	exit(7);
}
