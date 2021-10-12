static char sccsid[] = "@(#)73	1.9  src/bos/usr/bin/mh/uip/rcvpack.c, cmdmh, bos411, 9428A410j 11/9/93 09:43:40";
/* 
 * COMPONENT_NAME: CMDMH rcvpack.c
 * 
 * FUNCTIONS: MSGSTR, Mrcvpack 
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
/* static char sccsid[] = "rcvpack.c	7.1 87/10/13 17:33:37"; */

/* rcvpack.c - a rcvmail program to keep a copy */

#include "mh.h"
#include "dropsbr.h"
#include "rcvmail.h"
#include "tws.h"
#include "mts.h"

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	HELPSW	0
    "help", 4,

    NULL, (int)NULL
};

/*  */

long	lseek ();

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int     md;
    char   *cp,
           *file = NULL,
            buf[100],
	    ddate[BUFSIZ],
          **ap,
          **argp,
           *arguments[MAXARGS];

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
		    (void) sprintf (buf, MSGSTR(USEFILE, "%s [switches] file"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);
	    }
	if (file)
	    adios (NULLCP, MSGSTR(ONEFILE, "only one file at a time!")); /*MSG*/
	else
	    file = cp;
    }

/*  */

    if (!file)
	adios (NULLCP, MSGSTR(USEFILE, "%s [switches] file"), invo_name); /*MSG*/

    (void) sprintf (ddate, "Delivery-Date: %s\n", dtimenow ());
    rewind (stdin);
    if ((md = mbx_open (file, getuid (), getgid (), m_gmprot ())) == NOTOK
	    || mbx_copy (file, md, fileno (stdin), 1, ddate, 0) == NOTOK
	    || mbx_close (file, md) == NOTOK) {
	if (md != NOTOK)
	    (void) mbx_close (file, md);
	done (RCV_MBX);
    }

    done (RCV_MOK);
}
