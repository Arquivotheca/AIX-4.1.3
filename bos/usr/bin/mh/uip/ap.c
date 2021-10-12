static char sccsid[] = "@(#)43	1.8  src/bos/usr/bin/mh/uip/ap.c, cmdmh, bos411, 9428A410j 11/9/93 09:40:24";
/* 
 * COMPONENT_NAME: CMDMH ap.c
 * 
 * FUNCTIONS: MSGSTR, Map, process 
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
/*static char sccsid[] = "ap.c	7.1 87/10/13 17:23:36"; */

/* ap.c - parse addresses 822-style */

#include "mh.h"
#include "addrsbr.h"
#include "formatsbr.h"
#include <stdio.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

#define	NADDRS	100

#define	WIDTH	78
#define	WBUFSIZ	BUFSIZ

#define	FORMAT	"%<{error}%{error}: %{text}%|%(putstr(proper{text}))%>"

/*  */

static struct swit switches[] = {
#define	FORMSW	0
    "form formatfile", 0,
#define	FMTSW	1
    "format string", 5,

#define	NORMSW	2
    "normalize", 0,
#define	NNORMSW	3
    "nonormalize", 0,

#define	WIDSW	4
    "width columns", 0,

#define	HELPSW	5
    "help", 4,

    NULL, (int)NULL
};

/*  */

static struct format *fmt;
static int  process (register char *, int, int );
static int dat[4];

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   **argv;
{
    int     addrp = 0,
            normalize = AD_HOST,
            width = 0,
            status = 0;
    char   *cp,
	   *form = NULL,
           *format = NULL,
	   *nfs,
            buf[80],
	  **ap,
          **argp,
	   *arguments[MAXARGS],
           *addrs[NADDRS];

	setlocale(LC_ALL,"");
	catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    mts_init (invo_name);
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

/*  */

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);

		case UNKWNSW: 
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/

		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(SWADDRS, "%s [switches] addrs ..."), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case FORMSW: 
		    if (!(form = *argp++) || *form == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    format = NULL;
		    continue;
		case FMTSW: 
		    if (!(format = *argp++) || *format == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    form = NULL;
		    continue;

		case WIDSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    width = atoi (cp);
		    continue;

		case NORMSW: 
		    normalize = AD_HOST;
		    continue;
		case NNORMSW: 
		    normalize = AD_NHST;
		    continue;
	    }
	if (addrp > NADDRS)
	    adios (NULLCP, MSGSTR(MANYADDR, "more than %d addresses"), NADDRS); /*MSG*/
	else
	    addrs[addrp++] = cp;
    }
    addrs[addrp] = NULL;

/*  */

    if (addrp == 0)
	adios (NULLCP, MSGSTR(USWADDR, "usage: %s [switches] addrs ..."), invo_name); /*MSG*/

    nfs = new_fs (form, format, FORMAT);
    if (width == 0) {
	if ((width = sc_width ()) < WIDTH / 2)
	    width = WIDTH / 2;
	width -= 2;
    }
    if (width > WBUFSIZ)
	width = WBUFSIZ;
    fmt_norm = normalize;
    (void) fmt_compile (nfs, &fmt);
    dat[0] = dat[1] = dat[2] = 0;
    dat[3] = width;

    for (addrp = 0; addrs[addrp]; addrp++)
	status += process (addrs[addrp], width, normalize);

    done (status);
}

/*  */

struct pqpair {
    char    *pq_text;
    char    *pq_error;
    struct pqpair *pq_next;
};


static int  process (register char  *arg,
	             int     length,
		     int     norm	)
{
    int     addrp,
            status = 0;
    register char  *cp;
    char    buffer[WBUFSIZ + 1],
            error[BUFSIZ];
    register struct comp   *cptr;
    register struct pqpair *p,
                           *q;
    struct pqpair   pq;
    register struct mailname   *mp;

    (q = &pq) -> pq_next = NULL;
    while (cp = getname (arg)) {
	if ((p = (struct pqpair *) calloc ((unsigned) 1, sizeof *p)) == NULL)
	    adios (NULLCP, MSGSTR(NOAPMEM, "unable to allocate pqpair memory")); /*MSG*/
	if ((mp = getm (cp, NULLCP, 0, norm, error)) == NULL) {
	    p -> pq_text = getcpy (cp);
	    p -> pq_error = getcpy (error);
	    status++;
	}
	else {
	    p -> pq_text = getcpy (mp -> m_text);
	    mnfree (mp);
	}
	q = (q -> pq_next = p);
    }

    for (p = pq.pq_next; p; p = q) {
	FINDCOMP (cptr, "text");
	if (cptr)
	    cptr -> c_text = p -> pq_text;
	FINDCOMP (cptr, "error");
	if (cptr)
	    cptr -> c_text = p -> pq_error;

	(void) fmtscan (fmt, buffer, length, dat);
	(void) fputs (buffer, stdout);

	free (p -> pq_text);
	if (p -> pq_error)
	    free (p -> pq_error);
	q = p -> pq_next;
	free ((char *) p);
    }

    return status;
}
