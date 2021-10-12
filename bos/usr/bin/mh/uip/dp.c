static char sccsid[] = "@(#)51	1.8  src/bos/usr/bin/mh/uip/dp.c, cmdmh, bos411, 9428A410j 11/9/93 09:41:15";
/* 
 * COMPONENT_NAME: CMDMH dp.c
 * 
 * FUNCTIONS: MSGSTR, Mdp, process 
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
/* static char sccsid[] = "dp.c	7.1 87/10/13 17:26:12"; */

/* dp.c  - parse dates 822-style */

#include "mh.h"
#include "formatsbr.h"
#include "tws.h"
#include <stdio.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


#define	NDATES	100

#define	WIDTH	78
#define	WBUFSIZ	BUFSIZ

#define	FORMAT	"%<(nodate{text})error: %{text}%|%(putstr(pretty{text}))%>"

/*  */

static struct swit switches[] = {
#define	FORMSW	0
    "form formatfile", 0,
#define	FMTSW	1
    "format string", 5,

#define	WIDSW	2
    "width columns", 0,

#define	HELPSW	3
    "help", 4,

    NULL, (int)NULL
};

/*  */

static struct format *fmt;
static        int process (register char *, int);
static int dat[4];

/*  */

/* ARGSUSED */

main (argc, argv)
int argc;
char **argv;
{
    int     datep = 0,
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
           *dates[NDATES];

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

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(HELPOPTS, "%s [switches] dates ..."), invo_name); /*MSG*/
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
	    }
	if (datep > NDATES)
	    adios (NULLCP, MSGSTR(MANYDTS, "more than %d dates"), NDATES); /*MSG*/
	else
	    dates[datep++] = cp;
    }
    dates[datep] = NULL;

/*  */

    if (datep == 0)
	adios (NULLCP, MSGSTR(USEMSG, "usage: %s [switches] dates ..."), invo_name); /*MSG*/

    nfs = new_fs (form, format, FORMAT);
    if (width == 0) {
	if ((width = sc_width ()) < WIDTH / 2)
	    width = WIDTH / 2;
	width -= 2;
    }
    if (width > WBUFSIZ)
	width = WBUFSIZ;
    (void) fmt_compile (nfs, &fmt);
    dat[0] = dat[1] = dat[2] = 0;
    dat[3] = width;

    for (datep = 0; dates[datep]; datep++)
	status += process (dates[datep], width);

    m_update ();

    done (status);
}

/*  */

static	int process (register char   *date,
	 	     int	length)
{
    int     status = 0;
    char    buffer[WBUFSIZ + 1];
    register struct comp   *cptr;

    FINDCOMP (cptr, "text");
    if (cptr)
	cptr -> c_text = date;
    (void) fmtscan (fmt, buffer, length, dat);
    (void) fputs (buffer, stdout);

    return status;
}
