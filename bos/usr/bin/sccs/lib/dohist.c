static char sccsid[] = "@(#)88 1.11 src/bos/usr/bin/sccs/lib/dohist.c, cmdsccs, bos412, 9444b412 10/25/94 15:39:40";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: dohist, getresp, mrfixup, savecmt, stalloc, valmrs
 *
 * ORIGINS: 3, 10, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */

# include       "defines.h"
# include 	"delta_msg.h"
# include       <stdlib.h>

extern char *Comments;
extern char *Mrs;

static char nospace[] = "out of space (ut9)";

/*
extern char *malloc();
extern char *realloc();
*/
extern char *strcpy();
char *stalloc();

dohist()
{
	extern char *getresp();
	extern char *Sflags[];
	static mrsdone;

	FSAVE(FTLEXIT | FTLMSG | FTLCLN);
	if (Sflags[VALFLAG - 'a'] && !mrsdone) {
		if (!Mrs)
			Mrs = getresp("MRs", " ");
		mrfixup();
		mrsdone = 1;
	}
	if (!Comments) {
		static char sep[] = {'\n',CTLCHAR,COMMENTS,' ','\0'};

		Comments = getresp(catgets(catd,MS_DELTA,MCOMMENTS,"comments"), sep);
	}
	FRSTR();
}


char *
getresp(type, sep)
char *type, *sep;
{
	register char   *p, *buffer, *ebuffer, *ep;
	register int    i, slen, tty;
	extern char	had_standinp;

	if (had_standinp)
		fatal(catgets(catd,MS_DELTA,STDINSPCD, "\nUse the -y or the -m flag when you specify standard input. (de16)\n"));  /* MSG */
	if (tty = isatty(0)) {
		char type_copy[NL_TEXTMAX];

		/* copy in case type points to NLS static space */
		strcpy(type_copy, type);
		printf(MSGCO(TYPTERM, "%s? "), type_copy);
	}
	slen = strlen(sep);
	buffer = ep = p = stalloc(BUFSIZ);
	ebuffer = p + BUFSIZ - slen;
	strcpy(p, sep+1);
	p += slen - 1;
	for (;;) {
		while (fgets(p, ebuffer-p, stdin)) {
			i = strlen(p) - 1;
			p += i;
			if (*p == '\n')
				break;
			++p;
			/* not able to read whole line; get more space */
			ebuffer = buffer;
			buffer = realloc(buffer, p-buffer + BUFSIZ);
			if (!buffer)
				fatal(MSGCO(OTOFSPC, "\nThere is not enough memory available now.\n\
\tTry again later or\n\
\tuse local problem reporting procedures. (ut9)\n"));  /* MSG */
			p = buffer + (p-ebuffer);
			ep = buffer + (ep-ebuffer);
			ebuffer = p + BUFSIZ - slen;
		}
		/* terminate on EOF or blank line from terminal */
		if (feof(stdin) || i == 0 && tty)
			break;
		/* strip trailing backslash if any */
		if ((i = p[-1]) == '\\' 
				&& mblen(&p[-1], MB_CUR_MAX) == 1)
				--p;
		else
			i = 0;

		strcpy(ep = p, sep);
		p += slen;
		if (*type == 'M') {
			/* MRs from file terminate with line with no backslash */
			if (!tty && i != '\\')
				break;
		}
		else
			/* include trailing separator */
			ep++;
	}
	clearerr(stdin);        /* allow multiple EOF's from terminal */
	*ep++ = '\0';
	/* give back unused space */
	return(realloc(buffer, ep-buffer));
}


char	*Qarg[NVARGS];
char	**Varg = Qarg;

valmrs(pkt,pgm)
struct packet *pkt;
char *pgm;
{
	extern char *Sflags[];
	register int i;
	int st;
	register char *p;
	char *auxf();

	Varg[0] = pgm;
	Varg[1] = auxf(pkt->p_file,'g');
	if (p = Sflags[TYPEFLAG - 'a'])
		Varg[2] = p;
	else
		Varg[2] = Null;
	if ((i = fork()) < 0) {
		fatal(MSGCO(CANTFORK, "\nCannot create another process at this time.\n\
\tTry again later or\n\
\tuse local problem reporting procedures. (co20)\n"));  /* MSG */
	}
	else if (i == 0) {
		for (i = 4; i < 15; i++)
			close(i);
		execvp(pgm,Varg);
		exit(1);
	}
	wait(&st);
	return(st);
}


mrfixup()
{
	register char **argv, *p, *ap, c;

	for (p = Mrs, argv = &Varg[VSTART];; argv++) {
		NONBLANK(p);
		if (!*p)
			break;
		if (argv >= &Varg[NVARGS - 1])
			fatal(MSGCO(TOOMANY, "\nThe maximum number of MRs allowed is 64. (co21)\n"));  /*MSG*/
		for (ap = p; (c = *p++) && c != ' ' && c != '\t';)
			;
		p[-1] = '\0';
		*argv = strcpy(stalloc(p-ap), ap);
		*--p = c;
	}
	*argv = 0;
}


char *
stalloc(n)
{
	register char *p;

	if (!(p = malloc(n)))
		fatal(MSGCO(OTOFSPC, "\nThere is not enough memory available now.\n\
\tTry again later or\n\
\tuse local problem reporting procedures. (ut9)\n"));  /* MSG */
	return(p);
}


savecmt(p)
register char *p;
{
	register char   *p1;
	register int    nlcnt;

	nlcnt = 1;
	for (p1 = p; *p1; p1++)
		if (*p1 == '\n')
			nlcnt++;
	Comments = p1 = stalloc(strlen(p) + nlcnt * 3 + 2);
	while (*p) {
		*p1++ = CTLCHAR;
		*p1++ = COMMENTS;
		*p1++ = ' ';
		do {
			if(*p)
				*p1 = *p++;
			else
				*p1 = '\n';
		} while(*p1++ != '\n');
	}
	*p1 = '\0';
}
