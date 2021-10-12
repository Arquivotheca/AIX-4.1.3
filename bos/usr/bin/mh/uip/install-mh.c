static char sccsid[] = "@(#)58	1.10  src/bos/usr/bin/mh/uip/install-mh.c, cmdmh, bos411, 9428A410j 11/9/93 09:41:52";
/* 
 * COMPONENT_NAME: CMDMH install-mh.c
 * 
 * FUNCTIONS: MSGSTR, Minstall-mh, geta 
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
/* static char sccsid[] = "install-mh.c	7.1 87/10/13 17:27:55"; */

/* install-mh.c - initialize the MH environment */

#include "mh.h"
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static char message[] = 
    "\nPrior to using MH, it is necessary to have a file in your login\n\
directory (%s) named %s which contains information\n\
to direct certain MH operations.  The only item which is required\n\
is the path to use for all MH folder operations.  The suggested MH\n\
path for you is %s/Mail...\n\n";


static char   *geta ();


/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int     autof,
	    i;
    char   *cp,
           *path;
    char   hlds[NL_TEXTMAX];
    struct node *np;
    struct passwd *pw;
    struct stat st;
    FILE   *in,
	   *out;

        setlocale(LC_ALL,"");
	catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');

#ifdef	COMPAT
    if (argc == 2 && strcmp (argv[1], "-compat") == 0) {
	context = "/dev/null";	/* hack past m_getdefs() */
	
	m_getdefs ();
	for (np = m_defs; np; np = np -> n_next)
	    if (uleq (pfolder, np -> n_name)
		    || ssequal ("atr-", np -> n_name)
		    || ssequal ("cur-", np -> n_name))
		np -> n_context = 1;

	ctxpath = getcpy (m_maildir (context = "context"));
	ctxflags |= CTXMOD;
	m_update ();

	if ((out = fopen (defpath, "w")) == NULL)
	    adios (defpath, MSGSTR(NOWRITE, "unable to write %s"), defpath); /*MSG*/
	for (np = m_defs; np; np = np -> n_next)
	    if (!np -> n_context)
		fprintf (out, "%s: %s\n", np -> n_name, np -> n_field);
	(void) fclose (out);

	done (0);
    }
#endif	COMPAT

    autof = (argc == 2 && strcmp (argv[1], "-auto") == 0);
    if (mypath == NULL) {	/* straight from m_getdefs... */
	if (mypath = (char *)getenv ("HOME"))
	    mypath = getcpy (mypath);
	else
	    if ((pw = getpwuid (getuid ())) == NULL
		    || pw -> pw_dir == NULL
		    || *pw -> pw_dir == (char)NULL)
		adios (NULLCP, MSGSTR(NOHOME, "no HOME envariable")); /*MSG*/
	    else
		mypath = getcpy (pw -> pw_dir);
	if ((cp = mypath + strlen (mypath) - 1) > mypath && *cp == '/')
	    *cp = (char)NULL;
    }
    defpath = concat (mypath, "/", mh_profile, NULLCP);

    if (stat (defpath, &st) != NOTOK)
	if (autof)
	    adios (NULLCP, MSGSTR(INVERR, "invocation error")); /*MSG*/
	else
	    adios (NULLCP,
		    MSGSTR(ALRDY, "You already have an MH profile, use an editor to modify it")); /*MSG*/

    if (!autof && getanswer (MSGSTR(WANT, "Do you want help <yes or no>? "))) /*MSG*/
	    printf (MSGSTR(MESSAGE, message), mypath, mh_profile, mypath); /*MSG*/

/*  */

    cp = concat (mypath, "/", "Mail", NULLCP);
    if (stat (cp, &st) != NOTOK) {
	if ((st.st_mode & S_IFMT) == S_IFDIR) {
	    sprintf(hlds, MSGSTR(STANDIR, "You already have the standard MH directory \"%s\".\nDo you want to use it for MH <yes or no>? "), cp); /*MSG*/
	    if (getanswer (hlds))
		path = "Mail";
	    else
		goto query;
	}
	else
	    goto query;
    }
    else {
	if (autof)
	    printf (MSGSTR(THANKS, "I'm going to create the standard MH path for you.\n")); /*MSG*/
	else
	    sprintf(hlds, MSGSTR(STANPTH, "Do you want the standard MH path \"%s/Mail\" <yes or no>? "), mypath); /*MSG*/
	if (autof || getanswer (hlds))
	    path = "Mail";
	else {
    query:  ;
	    if (getanswer (MSGSTR(PATH, "Do you want a path below your login directory <yes or no>? "))) /*MSG*/ {
		printf (MSGSTR(WPATH, "What is the path?  %s/"), mypath); /*MSG*/
		path = geta ();
	    }
	    else {
		printf (MSGSTR(WHOLEPATH, "What is the whole path?  /")); /*MSG*/
		path = concat ("/", geta (), NULLCP);
	    }
	}
    }

    (void) chdir (mypath);
    if (chdir (path) == NOTOK) {
	sprintf(hlds, MSGSTR(CREPTH, "\"%s\" doesn't exist; Create it <yes or no>? "), path); /*MSG*/
	if (autof || getanswer (hlds))
	    if (makedir (path) == 0)
		adios (NULLCP, MSGSTR(NOCREATE, "unable to create %s"), path); /*MSG*/
    }
    else
	printf (MSGSTR(USEEXST, "[Using existing directory]\n")); /*MSG*/

/*  */

    np = m_defs = (struct node *) malloc (sizeof *np);
    if (np == NULL)
	adios (NULLCP, MSGSTR(NOAPSTOR, "unable to allocate profile storage")); /*MSG*/
    np -> n_name = getcpy ("Path");
    np -> n_field = getcpy (path);
    np -> n_context = 0;
    np -> n_next = NULL;

    if (in = fopen (mh_defaults, "r")) {
	m_readefs (&np -> n_next, in, mh_defaults, 0);
	(void) fclose (in);
    }

    ctxpath = getcpy (m_maildir (context = "context"));
    m_replace (pfolder, defalt);
    m_update ();

    if ((out = fopen (defpath, "w")) == NULL)
	adios (defpath, MSGSTR(NOWRITE, "unable to write %s"), defpath); /*MSG*/
    for (np = m_defs; np; np = np -> n_next)
	if (!np -> n_context)
	    fprintf (out, "%s: %s\n", np -> n_name, np -> n_field);
    (void) fclose (out);

    done (0);
}

/*  */

static char *geta () {
    register char  *cp;
    static char line[BUFSIZ];

    (void) fflush (stdout);
    if (fgets (line, sizeof line, stdin) == NULL)
	done (1);
    if (cp = (char *)index (line, '\n'))
	*cp = (char)NULL;
    return line;
}
