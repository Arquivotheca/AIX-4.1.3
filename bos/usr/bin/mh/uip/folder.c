static char sccsid[] = "@(#)55  1.11  src/bos/usr/bin/mh/uip/folder.c, cmdmh, bos411, 9428A410j 11/9/93 09:41:23";
/* 
 * COMPONENT_NAME: CMDMH folder.c
 * 
 * FUNCTIONS: MSGSTR, Mfolder, addfold, addir, compare, dodir, 
 *            dother, pfold, sfold, tfold 
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
/* static char sccsid[] = "folder.c	8.1 88/04/15 15:50:17"; */

/* folder(s).c - report on folders */

#include "mh.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <locale.h>
#ifndef	BSD42
#ifndef SYS5
#include <ndir.h>
#else   SYS5
#include <sys/dir.h>
#endif  SYS5
#else	BSD42
#include <sys/dir.h>
#endif	BSD42
#include <sys/stat.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 
/*  */

static struct swit switches[] = {
#define	ALLSW	0
    "all", 0,

#define	FASTSW	1
    "fast", 0,
#define	NFASTSW	2
    "nofast", 0,

#define	HDRSW	3
    "header", 0,
#define	NHDRSW	4
    "noheader", 0,

#define	PACKSW	5
    "pack", 0,
#define	NPACKSW	6
    "nopack", 0,

#define	RECURSW	7
    "recurse", 0,
#define	NRECRSW	8
    "norecurse", 0,

#define	TOTALSW	9
    "total", 0,
#define	NTOTLSW	10
    "nototal", 0,

#define	PRNTSW	11
    "print", 0,
#define	NPRNTSW	12
    "noprint", 0,
#define	LISTSW	13
    "list", 0,
#define	NLISTSW	14
    "nolist", 0,
#define	PUSHSW	15
    "push", 0,
#define	POPSW	16
    "pop", 0,

#define	HELPSW	17
    "help", 4,

    NULL, (int)NULL
};

/*  */

extern int errno;

static int  fshort = 0;
static int  fpack = 0;
static int  fheader = 0;
static int  frecurse = 0;
static int  ftotonly = 0;
static int  msgtot = 0;
static int  foldtot = 0;
static int  start = 0;
static int  foldp = 0;

static char *mhdir;
static char *stack = "Folder-Stack";
static char folder[BUFSIZ];
static char *folds[NFOLDERS + 1];

struct msgs *tfold ();
char *strcpy();

static	addir (register char *),
	dodir (register char *),
	addfold (register char *);

static int  pfold (register char *, register char *),
            compare (register char *, register char *),
            sfold (register struct msgs *, char *);

static dother();
/*  */

/* ARGSUSED */

main (argc, argv)
char   *argv[];
{
    int     all = 0,
            printsw = 0,
            listsw = 0,
            pushsw = 0,
            popsw = 0;
    char   *cp,
           *dp,
           *msg = NULL,
           *argfolder = NULL,
          **ap,
          **argp,
            buf[100],
           *arguments[MAXARGS];
    char hlds[NL_TEXTMAX];
    struct stat st;

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    if (argv[0][strlen (argv[0]) - 1] == 's')
	all++;
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
		    (void) sprintf (buf, MSGSTR(UMSG, "%s [+folder] [msg] [switches]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case ALLSW: 
		    all++;
		    continue;

		case FASTSW: 
		    fshort++;
		    continue;
		case NFASTSW: 
		    fshort = 0;
		    continue;

		case HDRSW: 
		    fheader = -1;
		    continue;
		case NHDRSW: 
		    fheader++;
		    continue;

		case PACKSW: 
		    fpack++;
		    continue;
		case NPACKSW: 
		    fpack = 0;
		    continue;

		case RECURSW: 
		    frecurse++;
		    continue;
		case NRECRSW: 
		    frecurse = 0;
		    continue;

		case TOTALSW: 
		    all++;
		    ftotonly++;
		    continue;
		case NTOTLSW: 
		    if (ftotonly)
			all = 0;
		    ftotonly = 0;
		    continue;

		case PRNTSW: 
		    printsw++;
		    continue;
		case NPRNTSW: 
		    printsw = 0;
		    continue;

		case LISTSW: 
		    listsw++;
		    continue;
		case NLISTSW: 
		    listsw = 0;
		    continue;

		case PUSHSW: 
		    pushsw++;
		    popsw = 0;
		    continue;
		case POPSW: 
		    popsw++;
		    pushsw = 0;
		    continue;
	    }
	if (*cp == '+' || *cp == '@')
	    if (argfolder)
		adios (NULLCP, MSGSTR(ONEFOLDER, "only one folder at a time!")); /*MSG*/
	    else
		argfolder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	else
	    if (msg)
		adios (NULLCP, MSGSTR(CURMSG, "only one (current) message at a time!")); /*MSG*/
	    else
		msg = cp;
    }

/*  */

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    mhdir = concat (m_maildir (""), "/", NULLCP);

    if (pushsw == 0 && popsw == 0 && listsw == 0)
	printsw++;
    if (pushsw) {
	if (!argfolder) {
	    if ((cp = m_find (stack)) == NULL
		    || (ap = brkstring (dp = getcpy (cp), " ", "\n")) == NULL
		    || (argfolder = *ap++) == NULL)
		adios (NULLCP, MSGSTR(NOOTHER, "no other folder")); /*MSG*/
	    for (cp = getcpy (m_getfolder ()); *ap; ap++)
		cp = add (*ap, add (" ", cp));
	    free (dp);
	    m_replace (stack, cp);
	}
	else
	    m_replace (stack,
		    (cp = m_find (stack))
		    ? concat (m_getfolder (), " ", cp, NULLCP)
		    : getcpy (m_getfolder ()));
    }
    if (popsw) {
	if (argfolder)
	    adios (NULLCP, MSGSTR(NOPOP, "sorry, no folders allowed with -pop")); /*MSG*/
	if ((cp = m_find (stack)) == NULL
		|| (ap = brkstring (dp = getcpy (cp), " ", "\n")) == NULL
		|| (argfolder = *ap++) == NULL)
	    adios (NULLCP, MSGSTR(EMPSTK, "folder stack empty")); /*MSG*/
	for (cp = NULL; *ap; ap++)
	    cp = cp ? add (*ap, add (" ", cp)) : getcpy (*ap);
	free (dp);
	if (cp)
	    m_replace (stack, cp);
	else
	    (void) m_delete (stack);
    }
    if (pushsw || popsw) {
	if (access (cp = m_maildir (argfolder), 0) == NOTOK)
	    adios (cp, MSGSTR(NOFINDF, "unable to find folder %s"), cp); /*MSG*/
	m_replace (pfolder, argfolder);
	m_update ();
	argfolder = NULL;
    }
    if (pushsw || popsw || listsw) {
	printf ("%s", argfolder ? argfolder : m_getfolder ());
	if (cp = m_find (stack)) {
	    for (ap = brkstring (dp = getcpy (cp), " ", "\n"); *ap; ap++)
		printf (" %s", *ap);
	    free (dp);
	}
	printf ("\n");

	if (!printsw)
	    done (0);
    }

/*  */

    if (all) {
	fheader = 0;
	if (argfolder || msg) {
	    (void) strcpy (folder, argfolder ? argfolder : m_getfolder ());

	    if (pfold (argfolder, msg) && argfolder) {
		m_replace (pfolder, argfolder);
		m_update ();
	    }
	    if (!frecurse)	/* counter-intuitive */
		dodir (folder);
	}
	else {
	    dother ();
/* The following statement was causing the pl8 complier to hiccup. 
*  Solution was to break it up.
*	    (void) strcpy (folder, (cp = m_find (pfolder)) ? cp : "");
*/
		if ((cp = m_find (pfolder)) != NULL)
	    		(void) strcpy (folder,cp);
		else
			folder[0] = '\0';

	    dodir (".");
	}

	if (!fshort) {
	    if (!ftotonly)
		printf ("\n\t\t     ");
	    if (msgtot != 1) {
		if (foldtot != 1)
	            printf (MSGSTR(TOTAL, "TOTAL= %*d messages in %d folders.\n"), DMAXFOLDER, msgtot, foldtot); /*MSG*/
		else
	            printf (MSGSTR(TOTAL2, "TOTAL= %*d messages in 1 folder.\n"), DMAXFOLDER, msgtot); /*MSG*/
	    }
	    else {
		if (foldtot != 1)
	            printf (MSGSTR(TOTAL3, "TOTAL= %*d message in %d folders.\n"), DMAXFOLDER, msgtot, foldtot); /*MSG*/
		else
	            printf (MSGSTR(TOTAL4, "TOTAL= %*d message in 1 folder.\n"), DMAXFOLDER, msgtot); /*MSG*/
	    }
	}
    }
    else {
	fheader++;

	(void) strcpy (folder, argfolder ? argfolder : m_getfolder ());
	if (stat (strcpy (buf, m_maildir (folder)), &st) == NOTOK) {
	    if (errno != ENOENT)
		adios (buf, MSGSTR(ERRF, "error on folder %s"), buf); /*MSG*/
	    sprintf(hlds, MSGSTR(CREFOL, "Create folder \"%s\"? "), buf); /*MSG*/
	    if (!getanswer (hlds))
		done (1);
	    if (!makedir (buf))
		adios (NULLCP, MSGSTR(NOCREAT, "unable to create folder %s"), buf); /*MSG*/
	}

	if (pfold (folder, msg) && argfolder)
	    m_replace (pfolder, argfolder);
    }

    m_update ();

    done (0);
}

/*  */

static	dodir (register char   *dir)
{
    int     i;
    int     os = start;
    int     of = foldp;
    char    buffer[BUFSIZ];

    start = foldp;
    if (chdir (mhdir) == NOTOK)
	adios (mhdir, MSGSTR(NOCHANGE, "unable to change directory to %s"), mhdir); /*MSG*/

    addir (strcpy (buffer, dir));
    for (i = start; i < foldp; i++)
	(void) pfold (folds[i], NULLCP), (void) fflush (stdout);

    start = os;
    foldp = of;
}

/*  */

static int  pfold (register char *fold,
		   register char *msg  )
{
    int	    hack,
	    others,
            retval = 1;
    register char *mailfile;
    register struct msgs   *mp = NULL;

    mailfile = m_maildir (fold);
    if (chdir (mailfile) == NOTOK) {
	if (errno != EACCES)
	    admonish (mailfile, MSGSTR(NOCHANGE, "unable to change directory to %s"), mailfile); /*MSG*/
	else
	    printf (MSGSTR(UNRDBL, "%22s%c unreadable\n"), fold, strcmp (folder, fold) ? ' ' : '+'); /*MSG*/
	return 0;
    }

    if (fshort) {
	printf ("%s\n", fold);

	if (!msg && !fpack) {
	    if (frecurse)
		dodir (fold);
	    return retval;
	}
    }

    if (!(mp = m_gmsg (fold))) {
	admonish (NULLCP, MSGSTR(NOREADF, "unable to read folder %s"), fold); /*MSG*/
	return 0;
    }

    if (msg && !sfold (mp, msg))
	retval = 0;
    if (fpack)
	mp = tfold (mp);

    if (fshort)
	goto out;
    foldtot++;
    msgtot += mp -> nummsg;

    if (ftotonly)
	goto out;

    if (!fheader++)
	printf (MSGSTR(HEADER, "\t\tFolder  %*s# of messages (%*srange%*s); cur%*smsg  (other files)\n"), DMAXFOLDER, "", DMAXFOLDER - 2, "", DMAXFOLDER - 2, "", DMAXFOLDER - 2, ""); /*MSG*/

    printf ("%22s%c ", fold, strcmp (folder, fold) ? ' ' : '+');

    hack = 0;
    if (mp -> hghmsg == 0)
	printf (MSGSTR(NOTANY, "has   no messages%*s"), mp -> msgflags & OTHERS ? DMAXFOLDER * 2 + 4 : 0, ""); /*MSG*/
    else {
	if (mp -> nummsg == 1)
	    printf (MSGSTR(ONEMSG2, "has %*d message  (%*d-%*d)"), 
		DMAXFOLDER, mp -> nummsg,
		DMAXFOLDER, mp -> lowmsg, DMAXFOLDER, mp -> hghmsg); /*MSG*/
	else
	    printf (MSGSTR(MULTIPLE, "has %*d messages (%*d-%*d)"), 
		DMAXFOLDER, mp -> nummsg, 
		DMAXFOLDER, mp -> lowmsg, DMAXFOLDER, mp -> hghmsg); /*MSG*/
	if (mp -> curmsg >= mp -> lowmsg && mp -> curmsg <= mp -> hghmsg)
	    printf ("; cur=%*d", DMAXFOLDER, hack = mp -> curmsg);
    }

    if (mp -> msgflags & OTHERS)
	printf (MSGSTR(OTHERS1, ";%*s (others)"), hack ? 0 : DMAXFOLDER + 6, ""); /*MSG*/
    printf (".\n");

out: ;
    others = mp -> msgflags & OTHERS;
    m_fmsg (mp);

    if (frecurse && others)
	dodir (fold);

    return retval;
}

/*  */

static int  sfold (register struct msgs   *mp,
		   char   *msg  )
{
    if (!m_convert (mp, msg))
	return 0;

    if (mp -> numsel > 1) {
	admonish (NULLCP, MSGSTR(ONEMSG, "only one message at a time!")); /*MSG*/
	return 0;
    }
    m_setseq (mp);
    m_setcur (mp, mp -> lowsel);
    m_sync (mp);
    m_update ();

    return 1;
}


struct msgs *tfold (mp)
register struct msgs   *mp;
{
    register int    hole,
                    msgnum;
    char    newmsg[BUFSIZ],
            oldmsg[BUFSIZ];

    if (mp -> lowmsg > 1 && (mp = m_remsg (mp, 1, mp -> hghmsg)) == NULL)
	adios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/

    for (msgnum = mp -> lowmsg, hole = 1; msgnum <= mp -> hghmsg; msgnum++)
	if (mp -> msgstats[msgnum] & EXISTS) {
	    if (msgnum != hole) {
		(void) strcpy (newmsg, m_name (hole));
		(void) strcpy (oldmsg, m_name (msgnum));
		if (rename (oldmsg, newmsg) == NOTOK)
		    adios (newmsg, MSGSTR(NORENAME, "unable to rename %s to %s"), oldmsg, newmsg); /*MSG*/
		if (msgnum == mp -> curmsg)
		    m_setcur (mp, mp -> curmsg = hole);
		mp -> msgstats[hole] = mp -> msgstats[msgnum];
		mp -> msgflags |= SEQMOD;
		if (msgnum == mp -> lowsel)
		    mp -> lowsel = hole;
		if (msgnum == mp -> hghsel)
		    mp -> hghsel = hole;
	    }
	    hole++;
	}
    if (mp -> nummsg > 0) {
	mp -> lowmsg = 1;
	mp -> hghmsg = hole - 1;
    }
    m_sync (mp);
    m_update ();

    return mp;
}

/*  */

static	addir (register char   *name)
{
    register char  *base,
                   *cp;
    struct stat st;
    register struct direct *dp;
    register    DIR * dd;

    cp = name + strlen (name);
    *cp++ = '/';
    *cp = (char)NULL;

    base = strcmp (name, "./") ? name : name + 2;/* hack */

    if ((dd = opendir (name)) == NULL) {
	admonish (name, MSGSTR(NORDIR, "unable to read directory %s "), name); /*MSG*/
	return;
    }
    while (dp = readdir (dd))
	if (strcmp (dp -> d_name, ".") && strcmp (dp -> d_name, "..")) {
	    if (cp + dp -> d_namlen + 2 >= name + BUFSIZ)	/* AIX hack */
		continue;
	    (void) strcpy (cp, dp -> d_name);
	    if (stat (name, &st) != NOTOK && (st.st_mode & S_IFMT) == S_IFDIR)
		addfold (base);
	}
    closedir (dd);

    *--cp = (char)NULL;
}

/*  */

static	addfold (register char   *fold)
{
    register int    i,
                    j;
    register char  *cp;

    if (foldp > NFOLDERS)
	adios (NULLCP, MSGSTR(MANYFS, "more than %d folders to report on"), NFOLDERS); /*MSG*/

    cp = getcpy (fold);
    for (i = start; i < foldp; i++)
	if (compare (cp, folds[i]) < 0) {
	    for (j = foldp - 1; j >= i; j--)
		folds[j + 1] = folds[j];
	    foldp++;
	    folds[i] = cp;
	    return;
	}

    folds[foldp++] = cp;
}

/*  */

static int  compare (register char   *s1,
		     register char   *s2 )
{
    register int    i;

    while (*s1 || *s2)
	if (i = *s1++ - *s2++)
	    return i;

    return 0;
}

/*  */

static	dother () {
    int	    atrlen;
    char    atrcur[BUFSIZ];
    register struct node   *np;

    (void) sprintf (atrcur, "atr-%s-", current);
    atrlen = strlen (atrcur);

    m_getdefs ();
    for (np = m_defs; np; np = np -> n_next)
	if (ssequal (atrcur, np -> n_name)
		&& !ssequal (mhdir, np -> n_name + atrlen))
	    (void) pfold (np -> n_name + atrlen, NULLCP);
}
