static char sccsid[] = "@(#)47	1.9  src/bos/usr/bin/mh/uip/conflict.c, cmdmh, bos411, 9428A410j 11/9/93 09:40:54";
/* 
 * COMPONENT_NAME: CMDMH conflict.c
 * 
 * FUNCTIONS: MSGSTR, Mconflict, alias_files, check, endglent, 
 *            getglent, getglnam, grp_ids, grp_members, grp_names, 
 *            ldr_names, ldr_ship, maildrops, mdrop, pwd_names, 
 *            setglent, setup 
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
/* static char sccsid[] = "conflict.c	7.1 87/10/13 17:24:54"; */

/* conflict.c - the new conflict */

#include "mh.h"
#include "aliasbr.h"
#include <stdio.h>
#include "mts.h"
#include <grp.h>
#include <pwd.h>
#ifndef	BSD42
#include <sys/types.h>
#ifndef	SYS5
#include <ndir.h>
#else	SYS5
#include <sys/dir.h>
#endif	SYS5
#else	BSD42
#include <sys/param.h>
#include <sys/dir.h>
#endif	BSD42

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


#define	NDIRS	100
#define	NGRPS	100

/*  */

static struct swit switches[] = {
#define	MAILSW	0
    "mail name", 0,

#define	SERCHSW	1
    "search directory", 0,

#define	HELPSW	2
    "help", 4,

    NULL, (int)NULL
};

/*  */

static	char   *mail = NULL;

static	char   *dirs[NDIRS];

static	FILE * out = NULL;


extern struct aka  *akahead;
extern struct home *homehead;



/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int	    akp = 0,
            dp = 0;
    char   *cp,
          **argp = argv + 1,
            buf[80],
           *akv[50];

    invo_name = r1bindex (argv[0], '/');
    m_foil (NULLCP);
    mts_init (invo_name);

/*  */

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(USEHELP, "%s [switches] [aliasfiles ...]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case MAILSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if (mail)
			adios (NULLCP, MSGSTR(ONEADDR, "mail to one address only")); /*MSG*/
		    else
			mail = cp;
		    continue;

		case SERCHSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if (dp >= NDIRS)
			adios (NULLCP, MSGSTR(MANYDIRS, "more than %d directories"), NDIRS); /*MSG*/
		    dirs[dp++] = cp;
		    continue;
	    }
	akv[akp++] = cp;
    }

/*  */

    if (akp == 0)
	akv[akp++] = AliasFile;
    if (!homehead)
	init_pw ();
    if (!mail)
	out = stdout;
    dirs[dp] = NULL;

    alias_files (akp, akv);
    pwd_names ();
    grp_names ();
    grp_members ();
    grp_ids ();
#ifdef	UCI
    ldr_names ();
    ldr_ship ();
#endif	UCI
    maildrops ();

    done (0);
}

/*  */

alias_files (akp, akv)
int     akp;
register char   **akv;
{
    char     hlds[NL_TEXTMAX];
    register int    i;

    for (i = 0; i < akp; i++)
	if ((i = alias (akv[i])) != AK_OK) {
	    setup ();
	    strcpy(hlds, MSGSTR(AERROR, "aliasing error in %s - %s\n")); /*MSG*/
	    fprintf (out, hlds, akv[i], akerror (i));
	}
	else
	    if (out && !mail)
		fprintf (out, MSGSTR(ISOK, "alias file %s is ok\n"), akv[i]); /*MSG*/
}

/*  */

pwd_names () {
    int     hit = 0;
    register struct home   *hm,
                           *lm;

    for (hm = homehead; hm; hm = hm -> h_next)
	for (lm = hm -> h_next; lm; lm = lm -> h_next)
	    if (strcmp (hm -> h_name, lm -> h_name) == 0) {
		setup ();
		fprintf (out, MSGSTR(DUPUSER, "duplicate user %s(uid=%d)\n"), lm -> h_name, lm -> h_uid); /*MSG*/
		hit++;
	    }

    if (!hit && out && !mail)
	fprintf (out, MSGSTR(USERSOK, "no duplicate users\n")); /*MSG*/
}


grp_names () {
    register int    gp,
                    hit = 0;
    char   *grps[NGRPS];
    register struct group  *gr;

    grps[0] = NULL;
    (void) setgrent ();
    while (gr = getgrent ()) {
	for (gp = 0; grps[gp]; gp++)
	    if (strcmp (grps[gp], gr -> gr_name) == 0) {
		setup ();
		fprintf (out, MSGSTR(DUPGRP, "duplicate group %s(gid=%d)\n"), gr -> gr_name, gr -> gr_gid); /*MSG*/
		hit++;
		break;
	    }
	if (grps[gp] == NULL)
	    if (gp < NGRPS) {
		grps[gp++] = getcpy (gr -> gr_name);
		grps[gp] = NULL;
	    }
	    else {
		setup ();
		fprintf (out, MSGSTR(MANYGRPS, "more than %d groups (time to recompile)\n"), NGRPS - 1); /*MSG*/
		hit++;
	    }
    }
    (void) endgrent ();

    for (gp = 0; grps[gp]; gp++)
	free (grps[gp]);

    if (!hit && out && !mail)
	fprintf (out, MSGSTR(GRPSOK, "no duplicate groups\n")); /*MSG*/
}

/*  */

grp_members () {
    register int    hit = 0;
    register char **cp,
                  **dp;
    register struct group  *gr;
    register struct home   *hm;

    (void) setgrent ();
    while (gr = getgrent ())
	for (cp = gr -> gr_mem; *cp; cp++) {
	    for (hm = homehead; hm; hm = hm -> h_next)
		if (!strcmp (*cp, hm -> h_name))
		    break;
	    if (hm == NULL) {
		setup ();
		fprintf (out, MSGSTR(UNKMEM, "group %s(gid=%d) has unknown member %s\n"), gr -> gr_name, gr -> gr_gid, *cp); /*MSG*/
		hit++;
	    }
#ifdef	BSD42
	    else
		hm -> h_ngrps++;
#endif	BSD42

	    for (dp = cp + 1; *dp; dp++)
		if (strcmp (*cp, *dp) == 0) {
		    setup ();
		    fprintf (out, MSGSTR(DUPMEM, "group %s(gid=%d) has duplicate member %s\n"), gr -> gr_name, gr -> gr_gid, *cp); /*MSG*/
		    hit++;
		}
	}
    (void) endgrent ();

#ifdef	BSD42
    for (hm = homehead; hm; hm = hm -> h_next)
	if (hm -> h_ngrps > NGROUPS) {
	    setup ();
	    fprintf (out, MSGSTR(MULTIGRP, "user %s is a member of %d groups (max %d)"), hm -> h_name, hm -> h_ngrps, NGROUPS); /*MSG*/
	    hit++;
	}
#endif	BSD42

    if (!hit && out && !mail)
	fprintf (out, MSGSTR(GRPMEMOK, "all group members accounted for\n")); /*MSG*/
}


grp_ids () {		/* -DRAND not implemented at most places */
    register int    hit = 0;
    register struct home   *hm;

    for (hm = homehead; hm; hm = hm -> h_next)
	if (getgrgid (hm -> h_gid) == NULL) {
	    setup ();
	    fprintf (out, MSGSTR(UNKGRPID, "user %s(uid=%d) has unknown group-id %d\n"), hm -> h_name, hm -> h_uid, hm -> h_gid); /*MSG*/
	    hit++;
	}

    if (!hit && out && !mail)
	fprintf (out, MSGSTR(GRPUOK, "all group-id users accounted for\n")); /*MSG*/
}

/*  */

maildrops () 
{
    register int    i;

    if (mmdfldir && *mmdfldir)
	mdrop (mmdfldir);
    if (uucpldir && *uucpldir)
	mdrop (uucpldir);
    for (i = 0; dirs[i]; i++)
	mdrop (dirs[i]);
}


mdrop(drop)
register char *drop;
{
    register int    hit = 0;
    register struct direct *dp;
    register DIR *dd = opendir (drop);

    if (!dd) {
	setup ();
	fprintf (out, MSGSTR(NOOPENMA, "unable to open maildrop area %s\n"), drop); /*MSG*/
	return;
    }

    while (dp = readdir (dd))
	if (dp -> d_name[0] != '.' && !check (dp ->d_name)) {
	    setup ();
	    fprintf (out, MSGSTR(MAILDROP, "there is a maildrop for the unknown user %s in %s\n"), dp -> d_name, drop); /*MSG*/
	    hit++;
	}

    closedir (dd);
    if (!hit && out && !mail)
	fprintf (out, MSGSTR(MDROPSOK, "all maildrops accounted for in %s\n"), drop); /*MSG*/
}


/*  */

int     check (s)
register char   *s;
{
    register struct home *hm;

    for (hm = homehead; hm; hm = hm -> h_next)
	if (!strcmp (s, hm -> h_name))
	    return 1;
    return 0;
}

/*  */

setup () {
    int     fd,
            pd[2];

    if (out)
	return;

    if (mail) {
	if (pipe (pd) == NOTOK)
	    adios ("pipe", MSGSTR(NOPIPE, "unable to pipe")); /*MSG*/

	switch (fork ()) {
	    case NOTOK: 
	        adios ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/

	    case OK: 
		(void) close (pd[1]);
		if (pd[0] != 0) {
		    (void) dup2 (pd[0], 0);
		    (void) close (pd[0]);
		}
		if ((fd = open ("/dev/null", 1)) != NOTOK)
		    if (fd != 1) {
			(void) dup2 (fd, 1);
			(void) close (fd);
		    }
		execlp (mailproc, r1bindex (mailproc, '/'),
			mail, "-subject", invo_name, NULLCP);
		adios (mailproc, MSGSTR(UNEXEC, "unable to exec %s"), mailproc); /*MSG*/

	    default: 
		(void) close (pd[0]);
		out = fdopen (pd[1], "w");
		fprintf (out, MSGSTR(SUSP, "%s: the following is suspicious\n\n"), invo_name); /*MSG*/
	}
    }
}

/*  */

#ifdef	UCI
/* UCI specific stuff for conflict */

/* taken from <grpldr.h> */

#define	GLDRS	"/admin/etc/GroupLeaders"

struct grpldr {
    char *gl_name;
    char **gl_ldr;
};

int	setglent (), endglent ();
struct grpldr *getglent (), *getglnam ();


/* taken from the getglent() routines */

#include <ctype.h>

#define	MAXGLS	100


static FILE *glp = NULL;
static char line[BUFSIZ+1];
static struct grpldr grpldr;
static char *gl_ldr[MAXGLS + 1];

/*  */

setglent() {
    if (glp == NULL)
	glp = fopen (GLDRS, "r");
    else
	rewind (glp);

    return (glp != NULL);
}


endglent() {
    if (glp != NULL) {
	(void) fclose (glp);
	glp = NULL;
    }

    return 1;
}

/*  */

struct grpldr  *getglent () {
    register char  *cp,
                  **q;

    if (glp == NULL && !setglent ())
	return NULL;
    if ((cp = fgets (line, BUFSIZ, glp)) == NULL)
	return NULL;

    grpldr.gl_name = cp;
    grpldr.gl_ldr = q = gl_ldr;

    while (*cp) {
	while (*cp && !isspace (*cp))
	    cp++;
	while (*cp && isspace (*cp))
	    *cp++ = NULL;
	if (*cp == NULL)
	    break;
	if (q < gl_ldr + MAXGLS)
	    *q++ = cp;
	else
	    break;
    }
    *q = NULL;

    return (&grpldr);
}

/*  */

struct grpldr  *getglnam (name)
char   *name;
{
    register struct grpldr  *gl = NULL;

    (void) setglent ();
    while (gl = getglent ())
	if (strcmp (name, gl -> gl_name) == 0)
	    break;
    (void) endglent ();

    return gl;
}

/*  */

ldr_names () {
    register int     gp,
		     hit = 0;
    char   *gldrs[NGRPS];
    register struct grpldr  *gl;

    gldrs[0] = NULL;
    (void) setglent ();
    while (gl = getglent ()) {
	if (getgrnam (gl -> gl_name) == NULL) {
	    setup ();
	    fprintf (out, MSGSTR(UNKGRP, "unknown group %s in group leaders file\n"), gl -> gl_name); /*MSG*/
	    hit++;
	}
	for (gp = 0; gldrs[gp]; gp++)
	    if (strcmp (gldrs[gp], gl -> gl_name) == 0) {
		setup ();
		fprintf (out, MSGSTR(DUPGROUP, "duplicate group %s in group leaders file\n"), gl -> gl_name); /*MSG*/
		hit++;
		break;
	    }
	if (gldrs[gp] == NULL)
	    if (gp < NGRPS) {
		gldrs[gp++] = getcpy (gl -> gl_name);
		gldrs[gp] = NULL;
	    }
	    else {
		setup ();
		fprintf (out, MSGSTR(MANYGPS, "more than %d groups in group leaders file (time to recompile)\n"), NGRPS - 1);  /*MSG*/
                hit++;
	    }
    }
    (void) endglent ();

    for (gp = 0; gldrs[gp]; gp++)
	free (gldrs[gp]);

    if (!hit && out && !mail)
	fprintf (out, MSGSTR(ALLOK, "all groups in group leaders file accounted for\n")); /*MSG*/
}


ldr_ship () {
    register int     hit = 0;
    register char  **cp,
		   **dp;
    register struct grpldr  *gl;

    (void) setglent ();
    while (gl = getglent ())
	for (cp = gl -> gl_ldr; *cp; cp++) {
	    if (!check (*cp)) {
		setup ();
		fprintf (out, MSGSTR(UNKLDR, "group %s has unknown leader %s\n"), gl -> gl_name, *cp); /*MSG*/
		hit++;
	    }

	    for (dp = cp + 1; *dp; dp++)
		if (strcmp (*cp, *dp) == 0) {
		    setup ();
		    fprintf (out, MSGSTR(DUPLDR, "group %s had duplicate leader %s\n"), gl -> gl_name, *cp); /*MSG*/
		    hit++;
		}
	}
    (void) endglent ();

    if (!hit && out && !mail)
	fprintf (out, MSGSTR(ALLLDROK, "all group leaders accounted for\n")); /*MSG*/
}
#endif	UCI
