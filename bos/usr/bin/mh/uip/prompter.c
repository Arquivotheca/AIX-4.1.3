static char sccsid[] = "@(#)71  1.9  src/bos/usr/bin/mh/uip/prompter.c, cmdmh, bos411, 9428A410j 11/9/93 09:43:25";
/* 
 * COMPONENT_NAME: CMDMH prompter.c
 * 
 * FUNCTIONS: MSGSTR, Mprompter, chrcnv, chrdsp, getln, intrser 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* static char sccsid[] = "prompter.c	7.1 87/10/13 17:32:54"; */

/* prompter.c - prompting editor front-end */

#include "mh.h"
#include <stdio.h>
#include <errno.h>
#ifndef	SYS5
#include <sgtty.h>
#else	SYS5
#include <sys/types.h>
#include <termio.h>
#include <sys/ioctl.h>
#endif	SYS5
#ifdef	BSD42
#include <setjmp.h>
#endif	BSD42
#include <signal.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

#define	QUOTE	'\\'
#ifndef	CKILL
#define CKILL   '@'
#endif	not CKILL
#ifndef	CERASE
#define CERASE  '#'
#endif	not CERASE

/*  */

static struct swit switches[] = {
#define	ERASESW	0
    "erase chr", 0,
#define	KILLSW	1
    "kill chr", 0,

#define	PREPSW	2
    "prepend", 0,	
#define	NPREPSW	3
    "noprepend", 0,	

#define	RAPDSW	4
    "rapid", 0,	
#define	NRAPDSW	5
    "norapid", 0,	

#define	BODYSW	6
    "body", -4,
#define	NBODYSW	7
    "nobody", -6,

#define	HELPSW	8
    "help", 4,		

    NULL, (int)NULL
};

/*  */

extern int  errno;


#ifndef	SYS5
#define	ERASE	sg.sg_erase
#define	KILL	sg.sg_kill
static struct sgttyb    sg;

#define	INTR	tc.t_intrc
static struct tchars    tc;
#else	SYS5
#define	ERASE	sg.c_cc[VERASE]
#define	KILL	sg.c_cc[VKILL]
#define	INTR	sg.c_cc[VINTR]
static struct termio    sg;
#endif	SYS5


static void	intrser (int);

static int  wtuser = 0;
static int  sigint = 0;

#ifdef	BSD42
static jmp_buf sigenv;
#endif	BSD42

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     body = 1,
	    prepend = 1,
	    rapid = 0,
	    fdi,
	    fdo,
            i,
            state;
    char   *cp,
           *drft = NULL,
           *erasep = NULL,
           *killp = NULL,
            name[NAMESZ],
            field[BUFSIZ],
            buffer[BUFSIZ],
            tmpfil[BUFSIZ],
          **ap,
           *arguments[MAXARGS],
          **argp;
    FILE *in, *out;

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

/*  */

    while (cp = *argp++)
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buffer, MSGSTR(USEFILE, "%s [switches] file"), invo_name); /*MSG*/
		    help (buffer, switches);
		    done (1);

		case ERASESW: 
		    if (!(erasep = *argp++) || *erasep == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
		case KILLSW: 
		    if (!(killp = *argp++) || *killp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case PREPSW: 
		    prepend++;
		    continue;
		case NPREPSW: 
		    prepend = 0;
		    continue;

		case RAPDSW: 
		    rapid++;
		    continue;
		case NRAPDSW: 
		    rapid = 0;
		    continue;

		case BODYSW: 
		    body++;
		    continue;
		case NBODYSW: 
		    body = 0;
		    continue;
	    }
	else
	    if (!drft)
		drft = cp;

/*  */

    if (!drft)
	adios (NULLCP, MSGSTR(USAGE3, "usage: %s [switches] file"), invo_name); /*MSG*/
    if ((in = fopen (drft, "r")) == NULL)
	adios (drft, MSGSTR(NOOPEN, "unable to open %s"), drft); /*MSG*/

    (void) strcpy (tmpfil, m_tmpfil (invo_name));
    if ((out = fopen (tmpfil, "w")) == NULL)
	adios (tmpfil, MSGSTR(NOCREATE, "unable to create %s"), tmpfil); /*MSG*/
    (void) chmod (tmpfil, 0600);

    if (killp || erasep) {
#ifndef	SYS5
	int    serase,
	       skill;
#else	SYS5
	char   serase,
	       skill;
#endif	SYS5

#ifndef	SYS5
	(void) ioctl (0, TIOCGETP, (char *) &sg);
	(void) ioctl (0, TIOCGETC, (char *) &tc);
#else	SYS5
	(void) ioctl(0, TCGETA, &sg);
#endif	SYS5
	skill = KILL;
	serase = ERASE;
	KILL = killp ? chrcnv (killp) : skill;
	ERASE = erasep ? chrcnv (erasep) : serase;
#ifndef	SYS5
	(void) ioctl (0, TIOCSETN, (char *) &sg);
#else	SYS5
	(void) ioctl(0, TCSETAW, &sg);
#endif	SYS5

	chrdsp ("erase", ERASE);
	chrdsp (", kill", KILL);
	chrdsp (", intr", INTR);
	(void) putchar ('\n');
	(void) fflush (stdout);

	KILL = skill;
	ERASE = serase;
    }

/*  */

    sigint = 0;
    setsig (SIGINT, (void(*)(int))intrser);

    for (state = FLD;;) {
	switch (state = m_getfld (state, name, field, sizeof field, in)) {
	    case FLD: 
	    case FLDEOF: 
	    case FLDPLUS: 
		for (cp = field; *cp; cp++)
		    if (*cp != ' ' && *cp != '\t')
			break;
		if (*cp++ != '\n' || *cp != (char)NULL) {
		    printf ("%s:%s", name, field);
		    fprintf (out, "%s:%s", name, field);
		    while (state == FLDPLUS) {
			state =
			    m_getfld (state, name, field, sizeof field, in);
			printf ("%s", field);
			fprintf (out, "%s", field);
		    }
		}
		else {
		    printf ("%s: ", name);
		    (void) fflush (stdout);
		    i = getln (field, sizeof field);
		    if (i == -1) {
abort: ;
			if (killp || erasep)
#ifndef	SYS5
			    (void) ioctl (0, TIOCSETN, (char *) &sg);
#else	SYS5
			    (void) ioctl (0, TCSETA, &sg);
#endif	SYS5
			(void) unlink (tmpfil);
			done (1);
		    }
		    if (i != 0 || (field[0] != '\n' && field[0] != (char)NULL)) {
			fprintf (out, "%s:", name);
			do {
			    if (field[0] != ' ' && field[0] != '\t')
				(void) putc (' ', out);
			    fprintf (out, "%s", field);
			} while (i == 1
				    && (i = getln (field, sizeof field)) >= 0);
			if (i == -1)
			    goto abort;
		    }
		}
		if (state == FLDEOF) {/* moby hack */
		    fprintf (out, "--------\n");
		    printf ("--------\n");
		    if (!body)
			break;
		    goto no_body;
		}
		continue;

	    case BODY: 
	    case BODYEOF:
	    case FILEEOF: 
		fprintf (out, "--------\n");
		if (field[0] == (char)NULL || !prepend)
		    printf ("--------\n");
		if (field[0]) {
		    if (prepend && body) {
			printf (MSGSTR(ENTERTXT, "\n--------Enter initial text\n\n")); /*MSG*/
			(void) fflush (stdout);
			for (;;) {
			    (void) getln (buffer, sizeof buffer);
			    if (buffer[0] == (char)NULL)
				break;
			    fprintf (out, "%s", buffer);
			}
		    }

		    do {
			fprintf (out, "%s", field);
			if (!rapid && !sigint)
			    printf ("%s", field);
		    } while (state == BODY &&
			    (state = m_getfld (state, name, field, sizeof field, in)));
		    if (prepend || !body)
			break;
		    else
			printf (MSGSTR(ENTERTXT2, "\n--------Enter additional text\n\n")); /*MSG*/
		}
no_body: ;
		(void) fflush (stdout);
		for (;;) {
		    (void) getln (field, sizeof field);
		    if (field[0] == (char)NULL)
			break;
 		    fprintf (out, "%s", field);
		}
		break;

	    default: 
		adios (NULLCP, MSGSTR(POORFMT, "skeleton is poorly formatted")); /*MSG*/
	}
	break;
    }

    if (body)
	printf ("--------\n");
    (void) fflush (stdout);

    (void) fclose (in);
    (void) fclose (out);

    (void) signal (SIGINT, SIG_IGN);

/*  */

    if (killp || erasep)
#ifndef	SYS5
	(void) ioctl (0, TIOCSETN, (char *) &sg);
#else	SYS5
	(void) ioctl (0, TCSETAW, &sg);
#endif	SYS5

    if ((fdi = open (tmpfil, 0)) == NOTOK)
	adios (tmpfil, MSGSTR(NOROPEN, "unable to re-open %s"), tmpfil); /*MSG*/
    if ((fdo = creat (drft, m_gmprot ())) == NOTOK)
	adios (drft, MSGSTR(NOWRITE, "unable to write %s"), drft); /*MSG*/
    cpydata (fdi, fdo, tmpfil, drft);
    (void) close (fdi);
    (void) close (fdo);
    (void) unlink (tmpfil);

    m_update ();

    done (0);
}

/*  */

getln (buffer, n)
register char   *buffer;
register int     n;
{
    int     c;
    char   *cp;

    cp = buffer;
    *cp = (char)NULL;

#ifndef	BSD42
    wtuser = 1;
#else	BSD42
    switch (setjmp (sigenv)) {
	case OK: 
	    wtuser = 1;
	    break;

	case DONE: 
	    wtuser = 0;
	    return 0;

	default: 
	    wtuser = 0;
	    return NOTOK;
    }
#endif	BSD42

    for (;;)
	switch (c = getchar ()) {
	    case EOF: 
#ifndef BSD42
		wtuser = 0;
		return (errno != EINTR ? 0 : NOTOK);
#else	BSD42
		clearerr (stdin);
		longjmp (sigenv, DONE);
#endif	BSD42

	    case '\n': 
		if (cp[-1] == QUOTE) {
		    cp[-1] = c;
		    wtuser = 0;
		    return 1;
		}
		*cp++ = c;
		*cp = (char)NULL;
		wtuser = 0;
		return 0;

	    default: 
		if (cp < buffer + n)
		    *cp++ = c;
		*cp = (char)NULL;
	}
}

/*  */

/* ARGSUSED */

static	void intrser (int i)
{
#ifndef	BSD42
    (void) signal (SIGINT, (void(*)(int))intrser);
    if (!wtuser)
	sigint++;
#else	BSD42
    if (wtuser)
	longjmp (sigenv, NOTOK);
    sigint++;
#endif	BSD42
}


chrcnv (cp)
register char   *cp;
{
    return (*cp != QUOTE ? *cp : m_atoi (++cp));
}


chrdsp (s, c)
char   *s,
	c;
{
    printf ("%s ", s);
    if (c < ' ' || c == 0177)
	printf ("^%c", c ^ 0100);
    else
	printf ("%c", c);
}
