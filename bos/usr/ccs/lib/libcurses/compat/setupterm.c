static char sccsid[] = "@(#)94  1.3  src/bos/usr/ccs/lib/libcurses/compat/setupterm.c, libcurses, bos411, 9428A410j 4/18/94 18:10:44";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:  getsh, setupterm
 *
 * ORIGINS: 3, 10, 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef MSG
/*              include file for message texts          */
#include "setupterm_msg.h"
#ifndef DONOTUSELOCALE
#include <locale.h>
#endif
				/* Cat descriptor for scmc conversion */
static nl_catd  setupterm_catd = CATD_ERR;
#endif

#include "cursesext.h"
#include <IN/uparm.h>

extern	struct	term _first_term;
extern	struct	term *cur_term;

static char firststrtab[2048];
static int called_before = 0;	/* To check for first time. */
char *getenv();
char *malloc();
char ttytype[128];
#ifndef termpath
#define termpath(file) "/usr/share/lib/terminfo/file"
#endif
#define MAGNUM 0432

#define getshi()	getsh(ip) ; ip += 2

#ifndef getsh

/*
 * NAME:        getsh
 *
 * FUNCTION:
 *
 *      Get a short from a pointer.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The short is in a standard
 *      format: two bytes, the first is the low order byte, the second is
 *      the high order byte (base 256).  The only negative number allowed is
 *      -1, which is represented as 255, 255.  This format happens to be the
 *      same as the hardware on the pdp-11 and vax, making it fast and
 *      convenient and small to do this on a pdp-11.
 *
 */

getsh(p)
register unsigned char *p;
{
	register int rv;
	rv = *p++;
	if (rv == 0377 && *p == 0377)
		return -1;
	rv += *p * 256;
	return rv;
}
#endif

/*
 * NAME:        setupterm
 *
 * FUNCTION:
 *
 *      Low level routine to dig up terminfo from database
 *      and read it in.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Parms are terminal type (0 means use getenv("TERM"),
 *      file descriptor all output will go to (for ioctls), and a pointer
 *      to an int into which the error return code goes (0 means to bomb
 *      out with an error message if there's an error).  Thus, setupterm(0,
 *      1, 0) is a reasonable way for a simple program to set up.
 */

setupterm(term, filenum, errret)
char *term;
int filenum;	/* This is a UNIX file descriptor, not a stdio ptr. */
int *errret;
{
	/* MSH:Change tiebuf to static so this compiles on a 16-bit machine.*/
	/* MSH:It appears that setupterm() is not recursive. */
	static char tiebuf[4096];   /* MSH:  not static on System V.2 tape */
	char fname[128];
	register char *ip;
	register char *cp;
	int  rc, n, tfd;
	char *lcp, *ccp;
	int snames, nbools, nints, nstrs, sstrtab;
	char *strtab, *termdef() ;

#ifdef MSG
	/* open the message catalog file descriptor */

#ifndef DONOTUSELOCALE
	setlocale(LC_ALL, "");
#endif

	if (setupterm_catd == CATD_ERR)
		setupterm_catd = catopen(MF_SETUPTERM, NL_CAT_LOCALE);
#endif

	if (term == NULL)
		term = getenv("TERM");
	if (term == NULL || *term == '\0')
		term = "unknown";
	tfd = -1;
	if (cp=getenv("TERMINFO")) {
		strcpy(fname, cp);
		cp = fname + strlen(fname);
		*cp++ = '/';
		*cp++ = *term;
		*cp++ = '/';
		strcpy(cp, term);
		tfd = open(fname, 0);
	}
	if (tfd < 0) {
		strcpy(fname, termpath(a/));
		cp = fname + strlen(fname);
		cp[-2] = *term;
		strcpy(cp, term);
		tfd = open(fname, 0);
	}

	if( tfd < 0 )
	{
		if( access( termpath( . ), 0 ) )
		{
			if( errret == 0 )
				perror( termpath( . ) );
			else
				*errret = -1;
		}
		else
		{
			if( errret == 0 )
			{
#ifdef MSG
				write(2,  NLcatgets(setupterm_catd,
				MS_setupterm, M_MSG_2,
				"No such terminal: "), 18);
#else
				write(2, "No such terminal: ", 18);
#endif
				write(2, term, strlen(term));
				write(2, "\r\n", 2);
			}
			else
			{
				*errret = 0;
			}
		}
		if( errret == 0 )
			exit( -2 );
		else {
			rc = -1;
			goto out;
		     }
	}

	if( called_before && cur_term ) /* 2nd or more times through */
	{
		cur_term = (struct term *) malloc(sizeof (struct term));
		bzero(cur_term, sizeof(struct term));
		strtab = NULL;
	}
	else					/* First time through */
	{
		cur_term = &_first_term;
		called_before = TRUE;
		strtab = firststrtab;
	}

	if( filenum == 1 && !isatty(filenum) )	/* Allow output redirect */
	{
		filenum = 2;
	}
	cur_term -> Filedes = filenum;
	def_shell_mode();

	if (errret)
		*errret = 1;
	n = read(tfd, tiebuf, sizeof tiebuf);
	close(tfd);
	if (n <= 0) {
corrupt:
#ifdef MSG
		write(2, NLcatgets(setupterm_catd, MS_setupterm, M_MSG_4,
		"corrupted term entry\r\n") , 22);
#else
		write(2, "corrupted term entry\r\n", 22);
#endif
		if (errret == 0)
			exit(-3);
		else {
			rc = -1;
			goto out;
		     }
	}
	if (n == sizeof tiebuf) {
#ifdef MSG
		write(2,  NLcatgets(setupterm_catd, MS_setupterm, M_MSG_5,
		"term entry too long\r\n") , 21);
#else
		write(2, "term entry too long\r\n", 21);
#endif
		if (errret == 0)
			exit(-4);
		else {
			rc = -1;
			goto out;
		     }
	}
	cp = ttytype;
	ip = tiebuf;

	/* Pick up header */
	snames = getshi();
	if (snames != MAGNUM) {
		goto corrupt;
	}
	snames = getshi();
	nbools = getshi();
	nints = getshi();
	nstrs = getshi();
	sstrtab = getshi();
	if (strtab == NULL) {
		strtab = (char *) malloc(sstrtab);
		bzero(strtab, sstrtab);
	}

	while (snames--)
		*cp++ = *ip++;	/* Skip names of terminals */

	/*
	 * Inner blocks to share this register among two variables.
	 */
	{
		char *fp = (char *)&cur_term->Columns;
		register char s;
		for (cp= &cur_term->Auto_left_margin; nbools--; ) {
			s = *ip++;
			if (cp < fp)
				*cp++ = s;
		}
	}

	/* Force proper alignment */
	if (((unsigned int) ip) & 1)
		ip++;

	{
		register short *sp;
		short *fp = (short *)&cur_term->strs;
		register int s;

		for (sp= &cur_term->Columns; nints--; ) {
			s = getshi();
			if (sp < fp)
				*sp++ = s;
		}
	}

#ifdef JWINSIZE
	/*
	 * ioctls for Blit - you may need to #include <jioctl.h>
	 * This ioctl defines the window size and overrides what
	 * it says in terminfo.
	 */
	{
		struct winsize w;

		if (ioctl(2, JWINSIZE, &w) != -1) {
			lines = w.bytesy;
			columns = w.bytesx;
		}
	}
#endif
	lcp = termdef(filenum, 'l');    /* use termdef                  */
	ccp = termdef(filenum, 'c');    /* instead of accessing environ.*/
	if (*lcp != '\0')
		lines = atoi(lcp);
	if (*ccp != '\0')
		columns = atoi(ccp);

	{
		register char **pp;
		char **fp = (char **)&cur_term->Filedes;

		for (pp= &cur_term->strs.Back_tab; nstrs--; ) {
			n = getshi();
			if (pp < fp) {
				if (n == -1)
					*pp++ = NULL;
				else
					*pp++ = strtab+n;
			}
		}
	}

	for (cp=strtab; sstrtab--; ) {
		*cp++ = *ip++;
	}

	/*
	 * If tabs are being expanded in software, turn this off
	 * so output won't get messed up.  Also, don't use tab
	 * or backtab, even if the terminal has them, since the
	 * user might not have hardware tabs set right.
	 */
#ifdef USG
	if ((cur_term -> Nttyb.c_oflag & TABDLY) == TAB3) {
		cur_term->Nttyb.c_oflag &= ~TABDLY;
		tab = NULL;
		back_tab = NULL;
		reset_prog_mode();
		rc = 0;
		goto out;
	}
#else
	if ((cur_term -> Nttyb.sg_flags & XTABS) == XTABS) {
		cur_term->Nttyb.sg_flags &= ~XTABS;
		tab = NULL;
		back_tab = NULL;
		reset_prog_mode();
		rc = 0;
		goto out;
	}
#endif
#ifdef DIOCSETT
	reset_prog_mode();
#endif 
#ifdef LTILDE
	ioctl(cur_term -> Filedes, TIOCLGET, &n);
	if (n & LTILDE);
		reset_prog_mode();
#endif

out:

#ifdef MSG
	if (setupterm_catd != CATD_ERR) {
		(void) catclose (setupterm_catd);
		setupterm_catd = CATD_ERR;
	}
#endif /* MSG */

	/*free(strtab); Added 9/10/90 A11213*/
	/*free(cur_term); Added 9/10/90 A11213*/
	return (rc);
}
