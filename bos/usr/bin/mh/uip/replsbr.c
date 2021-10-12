static char sccsid[] = "@(#)79	1.12  src/bos/usr/bin/mh/uip/replsbr.c, cmdmh, bos41J, 9515B_all 4/3/95 16:11:30";
/* 
 * COMPONENT_NAME: CMDMH replsbr.c
 * 
 * FUNCTIONS: CHECKMEM, CPY, MSGSTR, formataddr, insert, replfilter, 
 *            replout 
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
/* static char sccsid[] = "replsbr.c	7.1 87/10/13 17:35:47"; */

/* replsbr.c - routines to help repl along... */

#include "mh.h"
#include "addrsbr.h"
#include "formatsbr.h"
#include <ctype.h>
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


extern short    ccto,		/* from repl.c */
                cccc,
                ccme,
                format,
                outputlinelen,
		querysw;
extern char *fcc,
	    *filter,
            *form;

static int   dftype;

static char *badaddrs = NULL;
static char *dfhost;

static struct mailname  mq;


#define SBUFSIZ 256		/* buffer size for content part of header
				 * fields.  We want this to be large
				 * enough so that we don't do a lot of
				 * extra FLDPLUS calls on m_getfld but
				 * small enough so that we don't snarf
				 * the entire message body when we're
				 * not going to use any of it.
				 */

static struct format *fmt;

static int	ncomps = 0;		/* # of interesting components */
static char	**compbuffers = 0; 	/* buffers for component text */
static struct comp **used_buf = 0;	/* stack for comp that use buffers */

static struct comp **first_used_buf = 0; /* beginning of comp stack */

static int dat[4];			/* aux. data for format routine */

char *malloc();
char *realloc();
static replfilter(), insert();

/*  */

/* ARGSUSED */

replout (inb, msg, drft)
    register FILE *inb;
    char    *msg;
    char    *drft;
{
    register int  state;
    register int  i;
    register struct comp *cptr;
    register char *tmpbuf;
    register char **nxtbuf;
    register struct comp **savecomp;
    FILE    *out;
    char    name[NAMESZ];
    char    *scanl;
    int	    char_read = 0;
    char    *cp;
    int      format_len;

    (void) umask( ~ m_gmprot() );
    if ((out = fopen (drft, "w")) == NULL)
	adios (drft, MSGSTR(NOCREATE, "unable to create %s"), drft); /*MSG*/

    cp = new_fs (form ? form : replcomps, NULLCP, NULLCP);
    format_len = strlen (cp);
    ncomps = fmt_compile (cp, &fmt) + 1;
    nxtbuf = compbuffers = (char **)calloc((unsigned)ncomps,sizeof(char *));
    if (nxtbuf == NULL)
	adios (NULLCP, MSGSTR(NOACBUF, "unable to allocate component buffers")); /*MSG*/
    used_buf = (struct comp **)calloc((unsigned)(ncomps+1),sizeof(struct comp *));
    first_used_buf = used_buf; /* Save beginning of buffer */
    if (used_buf == NULL)
	adios (NULLCP, MSGSTR(NOACBSTACK, "unable to allocate component buffer stack")); /*MSG*/
    used_buf += ncomps+1; *--used_buf = 0;
    for (i = ncomps; i--; )
	if ((*nxtbuf++ = malloc( SBUFSIZ )) == NULL)
	    adios (NULLCP, MSGSTR(NOACBUF2, "unable to allocate component buffer")); /*MSG*/

    nxtbuf = compbuffers;
    savecomp = used_buf;
    tmpbuf = *nxtbuf++;

    /* ignore any components killed by command line switches */
    if (!ccto) {
	FINDCOMP (cptr, "to");
	if (cptr)
	    cptr->c_name = "";
    }
    if (!cccc) {
	FINDCOMP (cptr, "cc");
	if (cptr)
	    cptr->c_name = "";
    }
    /* set up the "fcc" pseudo-component */
    if (fcc) {
	FINDCOMP (cptr, "fcc");
	if (cptr)
	    cptr->c_text = getcpy (fcc);
    }
    if (!ccme)
	(void) ismymbox ((struct mailname *)0); /* XXX */

    /* pick any interesting stuff out of msg "inb" */
    for (state = FLD;;) {
	state = m_getfld (state, name, tmpbuf, SBUFSIZ, inb);
	switch (state) {
	    case FLD: 
	    case FLDPLUS: 
		/*
		 * if we're interested in this component, save a pointer
		 * to the component text, then start using our next free
		 * buffer as the component temp buffer (buffer switching
		 * saves an extra copy of the component text).
		 */
		if (cptr = wantcomp[CHASH(name)])
		    do {
			if (uleq(name, cptr->c_name)) {
			    char_read += msg_count;
			    if (! cptr->c_text) {
				cptr->c_text = tmpbuf;
				*--savecomp = cptr;
				tmpbuf = *nxtbuf++;
			    } else {
				i = strlen (cp = cptr->c_text) - 1;
				if (cp[i] == '\n')
				    if (cptr->c_type & CT_ADDR) {
					cp[i] = '\0';
					cp = add (",\n\t", cp);
				    } else {
					cp = add ("\t", cp);
				    }
				cptr->c_text = add (tmpbuf, cp);
			    }
			    while (state == FLDPLUS) {
				state = m_getfld (state, name, tmpbuf,
						  SBUFSIZ, inb);
				cptr->c_text = add (tmpbuf, cptr->c_text);
				char_read += msg_count;
			    }
			    break;
			}
		    } while (cptr = cptr->c_next);

		while (state == FLDPLUS)
		    state = m_getfld (state, name, tmpbuf, SBUFSIZ, inb);
		break;

	    case LENERR: 
	    case FMTERR: 
	    case BODY: 
	    case FILEEOF:
		goto finished;

	    default: 
		adios (NULLCP, MSGSTR(RETD, "m_getfld() returned %d"), state); /*MSG*/
	}
    }
    /*
     * format and output the header lines.
     */
finished:
    /* if there's a "subject" component, strip any "re:"s off it */
    FINDCOMP (cptr, "subject")
    if (cptr && (cp = cptr->c_text)) {
	register char *sp = cp;

	for (;;) {
	    while (isspace(i = *cp++))
		;
	    if ((i | 0x20) != 'r' || (*cp++ | 0x20) != 'e' || *cp++ != ':')
		break;
	    sp = cp;
	}
	if (sp != cptr->c_text) {
	    cp = cptr->c_text;
	    cptr->c_text = getcpy (sp);
	    free (cp);
	}
    }
    i = format_len + char_read + 256;
    scanl = malloc ((unsigned)i + 2);
    dat[0] = dat[1] = dat[2] = 0;
    dat[3] = outputlinelen;
    (void) fmtscan (fmt, scanl, i, dat);
    fputs (scanl, out);
    if (badaddrs) {
	fputs (MSGSTR(BADADDRS, "\nrepl: bad addresses:\n"), out); /*MSG*/
	fputs ( badaddrs, out);
    }
    if (filter)
	replfilter (inb, out);

    if (ferror (out))
	adios (drft, MSGSTR(WERR, "error writing %s"), drft); /*MSG*/
    (void) fclose (out);

    /* return dynamically allocated buffers */
    free (scanl);
    while ( cptr = *savecomp++ )
	free (cptr->c_text);
    while ( cp = *nxtbuf++)
	free (cp);
    free (tmpbuf);
    free ((char *) compbuffers);
/* danc - Need to free from the beginning of the stack, not the end */
/*    free ((char *) used_buf); */
    free ((char *) first_used_buf);
}

/*  */

static char *buf;		/* our current working buffer */
static char *bufend;		/* end of working buffer */
static char *last_dst;		/* buf ptr at end of last call */
static unsigned int bufsiz;	/* current size of buf */

#define BUFINCR 512		/* how much to expand buf when if fills */

#define CPY(s) { cp = (s); while (*dst++ = *cp++) ; --dst; }

/* check if there's enough room in buf for str.  add more mem if needed */
#define CHECKMEM(str) \
	    if ((len = strlen (str)) >= bufend - dst) {\
		bufsiz += ((dst + len - bufend) / BUFINCR + 1) * BUFINCR;\
		last_dst = dst - buf;\
		buf = realloc (buf, bufsiz);\
		if (! buf)\
		    adios (NULLCP, MSGSTR(NOBSP, "formataddr: couldn't get buffer space"));	/*MSG*/ \
		dst = buf + (int)last_dst;\
		bufend = buf + bufsiz;\
	    }


/* fmtscan will call this routine if the user includes the function
 * "(formataddr {component})" in a format string.  "orig" is the
 * original contents of the string register.  "str" is the address
 * string to be formatted and concatenated onto orig.  This routine
 * returns a pointer to the concatenated address string.
 *
 * We try to not do a lot of malloc/copy/free's (which is why we
 * don't call "getcpy") but still place no upper limit on the
 * length of the result string.
 */
char *formataddr (orig, str)
    char *orig;
    char *str;
{
    register int  len;
    char    baddr[BUFSIZ];
    register int  isgroup;
    register char  *dst;
    register char  *cp;
    register char  *sp;
    register struct mailname *mp = NULL;

    /* if we don't have a buffer yet, get one */
    if (bufsiz == 0) {
	buf = malloc (BUFINCR);
	if (! buf)
	    adios (NULLCP, MSGSTR(NOABSP, "formataddr: couldn't allocate buffer space")); /*MSG*/
	bufsiz = BUFINCR - 6;  /* leave some slop */
	bufend = buf + bufsiz;
    }
    /*
     * If "orig" points to our buffer we can just pick up where we
     * left off.  Otherwise we have to copy orig into our buffer.
     */
    if (orig == buf)
	dst = last_dst;
    else if (!orig || !*orig) {
	dst = buf;
	*dst = '\0';
    } else {
	CHECKMEM (orig);
	CPY (orig);
    }

    /* concatenate all the new addresses onto 'buf' */
    for (isgroup = 0; cp = getname (str); ) {
	if ((mp = getm (cp, dfhost, dftype, AD_NAME, NULLCP)) == NULL) {
	    (void) sprintf (baddr, MSGSTR(BADADDR2, "\tBAD ADDRRESS: %s\n"), cp); /*MSG*/
	    badaddrs = add (baddr, badaddrs);
	    continue;
	}
	if (isgroup && (mp->m_gname || !mp->m_ingrp)) {
	    *dst++ = ';';
	    isgroup = 0;
	}
	if (insert (mp)) {
	    /* if we get here we're going to add an address */
	    if (dst != buf) {
		*dst++ = ',';
		*dst++ = ' ';
	    }
	    if (mp->m_gname) {
		CHECKMEM (mp->m_gname);
		CPY (mp->m_gname);
		isgroup++;
	    }
	    sp = adrformat (mp);
	    CHECKMEM (sp);
	    CPY (sp);
	}
    }

    if (isgroup)
	*dst++ = ';';

    *dst = '\0';
    last_dst = dst;
    return (buf);
}
/*  */

static	insert (np)
register struct mailname *np;
{
    char    buffer[BUFSIZ];
    register struct mailname *mp;

    if (np -> m_mbox == NULL)
	return 0;

    for (mp = &mq; mp -> m_next; mp = mp -> m_next) {
#ifdef BERK
	if (uleq (np -> m_mbox, mp -> m_next -> m_mbox))
	    return 0;
#else not BERK
	if (uleq (np -> m_host, mp -> m_next -> m_host)
		&& uleq (np -> m_mbox, mp -> m_next -> m_mbox))
	    return 0;
#endif BERK
    }
    if (!ccme && ismymbox (np))
	return 0;

    if (querysw) {
	(void) sprintf (buffer, MSGSTR(REPLY, "Reply to %s <yes or no>? "), adrformat (np)); /*MSG*/
	if (!getanswer (buffer))
	return 0;
    }
    mp -> m_next = np;
#ifdef	ISI
    if (ismymbox (np))
	ccme = 0;
#endif	ISI
    return 1;
}

/*  */

static	replfilter (in, out)
register FILE *in,
	      *out;
{
    int	    pid;
    char   *mhl;

    if (filter == NULL)
	return;

    if (access (filter, 04) == NOTOK)
	adios (filter, MSGSTR(NOREAD, "unable to read %s"), filter); /*MSG*/

    mhl = r1bindex (mhlproc, '/');

    rewind (in);
    (void) fflush (out);

    switch (pid = vfork ()) {
	case NOTOK: 
	    adios ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/

	case OK: 
	    (void) dup2 (fileno (in), fileno (stdin));
	    (void) dup2 (fileno (out), fileno (stdout));
	    closefds (3);

	    execlp (mhlproc, mhl, "-form", filter, "-noclear", NULLCP);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (mhlproc);
	    _exit (-1);

	default: 
	    if (pidXwait (pid, mhl))
		done (1);
	    (void) fseek (out, 0L, 2);
	    break;
    }
}
