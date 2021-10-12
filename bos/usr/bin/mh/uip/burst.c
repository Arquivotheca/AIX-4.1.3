static char sccsid[] = "@(#)44  1.8  src/bos/usr/bin/mh/uip/burst.c, cmdmh, bos411, 9428A410j 11/9/93 09:40:36";
/* 
 * COMPONENT_NAME: CMDMH burst.c
 * 
 * FUNCTIONS: MSGSTR, Mburst, burst, cpybrst 
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
/* static char sccsid[] = "burst.c	7.1 87/10/13 17:23:56"; */

/* burst.c - explode digests into individual messages */

#include "mh.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 
/*  */

static struct swit switches[] = {
#define	INPLSW	0
    "inplace", 0,
#define	NINPLSW	1
    "noinplace", 0,

#define	QIETSW	2
    "quiet", 0,
#define	NQIETSW	3
    "noquiet", 0,

#define	VERBSW	4
    "verbose", 0,
#define	NVERBSW	5
    "noverbose", 0,

#define	HELPSW	6
    "help", 4,

    NULL, (int)NULL
};

/*  */

static char delim3[] = "-------";


static struct msgs *mp;

struct smsg {
    long    s_start;
    long    s_stop;
};

static  burst (register struct smsg *,
	       int, int, int, int);
static cpybrst (register FILE *, register FILE *,
		register char *, register char *,register int);

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int     inplace = 0,
            quietsw = 0,
            verbosw = 0,
            msgp = 0,
            hi,
            msgnum;
    char   *cp,
           *maildir,
           *folder = NULL,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *msgs[MAXARGS];
    struct smsg *smsgs;

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
		    adios (NULLCP, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(MHELPSW, "%s [+folder] [msgs] [switches]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case INPLSW: 
		    inplace++;
		    continue;
		case NINPLSW: 
		    inplace = 0;
		    continue;

		case QIETSW: 
		    quietsw++;
		    continue;
		case NQIETSW: 
		    quietsw = 0;
		    continue;

		case VERBSW: 
		    verbosw++;
		    continue;
		case NVERBSW: 
		    verbosw = 0;
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    if (folder)
		adios (NULLCP, MSGSTR(ONEFOLDER, "only one folder at a time!")); /*MSG*/
	    else
		folder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	}
	else
	    msgs[msgp++] = cp;
    }

/*  */

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (!msgp)
	msgs[msgp++] = "cur";
    if (!folder)
	folder = m_getfolder ();
    maildir = m_maildir (folder);

    if (chdir (maildir) == NOTOK)
	adios (maildir, MSGSTR(NOCHANGE, "unable to change directory to %s"), maildir); /*MSG*/
    if (!(mp = m_gmsg (folder)))
	adios (NULLCP, MSGSTR(NOREADF, "unable to read folder %s"), folder); /*MSG*/
    if (mp -> hghmsg == 0)
	adios (NULLCP, MSGSTR(NOMESF, "no messages in %s"), folder); /*MSG*/

    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    done (1);
    m_setseq (mp);

    smsgs = (struct smsg   *)
		calloc ((unsigned) (MAXFOLDER + 2), sizeof *smsgs);
    if (smsgs == NULL)
	adios (NULLCP, MSGSTR(NOABSTOR, "unable to allocate burst storage")); /*MSG*/

    hi = mp -> hghmsg + 1;
    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED)
	    burst (smsgs, msgnum, inplace, quietsw, verbosw);

    free ((char *) smsgs);

    m_replace (pfolder, folder);
    if (inplace) {
	if (mp -> lowsel != mp -> curmsg)
	    m_setcur (mp, mp -> lowsel);
    }
    else
	if (hi <= mp -> hghmsg)
	    m_setcur (mp, hi);
    m_sync (mp);
    m_update ();

    done (0);
}

/*  */

static  burst (register struct smsg *smsgs,
	       int     msgnum, int     inplace,
	       int     quietsw, int     verbosw)
{
    int     i,
            j,
            ld3,
	    wasdlm,
            mode,
            msgp;
    register long   pos;
    register char   c,
                   *msgnam;
    char    buffer[BUFSIZ],
            f1[BUFSIZ],
            f2[BUFSIZ],
            f3[BUFSIZ];
    struct stat st;
    register    FILE *in,
		     *out;

    ld3 = strlen (delim3);

    if ((in = fopen (msgnam = m_name (msgnum), "r")) == NULL)
	adios (msgnam, MSGSTR(NORMSG, "unable to read message %s"), msgnam); /*MSG*/

    mode = fstat (fileno (in), &st) != NOTOK ? (st.st_mode & 0777)
	: m_gmprot ();
    for (msgp = 1, pos = 0L; msgp <= MAXFOLDER;) {
	while (fgets (buffer, sizeof buffer, in) != NULL
		&& buffer[0] == '\n')
	    pos += (long) strlen (buffer);
	if (feof (in))
	    break;
	(void) fseek (in, pos, 0);
	smsgs[msgp].s_start = pos;

	for (c = (char)NULL;
		fgets (buffer, sizeof buffer, in) != NULL;
		c = buffer[0])
	    if (strncmp (buffer, delim3, ld3) == 0
		    && peekc (in) == '\n'
		    && (msgp == 1 || c == '\n'))
		break;
	    else
		pos += (long) strlen (buffer);

	wasdlm = strncmp (buffer, delim3, ld3) == 0;
	if (smsgs[msgp].s_start != pos)
	    smsgs[msgp++].s_stop = c == '\n' && wasdlm ? pos - 1 : pos;
	if (feof (in)) {
	    if (wasdlm) {
		smsgs[msgp - 1].s_stop -= ((long) strlen (buffer) + 1);
		msgp++;		/* fake "End of XXX Digest" */
	    }
	    break;
	}
	pos += (long) strlen (buffer);
    }

/*  */

    switch (--msgp) {		/* toss "End of XXX Digest" */
	case 0: 
	    adios (NULLCP, MSGSTR(BIGLOSE3, "burst() botch -- you lose big")); /*MSG*/

	case 1: 
	    if (!quietsw)
		admonish (NULLCP, MSGSTR(NOTDF, "message %d not in digest format"), msgnum); /*MSG*/
	    (void) fclose (in);
	    return;

	default: 
	    if (verbosw) {
		if (msgp -1 != 1)
		    printf (MSGSTR(EXP, "%d messages exploded from digest %d\n"), msgp - 1, msgnum); /*MSG*/
		else
		    printf (MSGSTR(EXP2, "1 message exploded from digest %d\n"), msgnum); /*MSG*/
	    }
	    if (msgp == 2)	/* XXX */
		msgp++;
	    break;
    }

    if ((mp = m_remsg (mp, 0, mp -> hghmsg + msgp)) == NULL)
	adios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/

/*  */

    msgp--;
    j = mp -> hghmsg;
    mp -> hghmsg += msgp - 1;
    mp -> nummsg += msgp - 1;
    if (mp -> hghsel > msgnum)
	mp -> hghsel += msgp - 1;

    if (inplace && msgp > 1)
	for (i = mp -> hghmsg; j > msgnum; i--, j--) {
	    (void) strcpy (f1, m_name (i));
	    (void) strcpy (f2, m_name (j));
	    if (mp -> msgstats[j] & EXISTS) {
		if (verbosw)
		    printf (MSGSTR(MTM, "message %d becomes message %d\n"), j, i); /*MSG*/

		if (rename (f2, f1) == NOTOK)
		    admonish (f1, MSGSTR(NORENAME, "unable to rename %s to %s"), f2, f1); /*MSG*/
		mp -> msgstats[i] = mp -> msgstats[j];
		mp -> msgstats[j] = (char)NULL;
		mp -> msgflags |= SEQMOD;
	    }
	}
    
    mp -> msgstats[msgnum] &= ~SELECTED;
    i = inplace ? msgnum + msgp - 1 : mp -> hghmsg;
    for (j = msgp; j >= (inplace ? 1 : 2); i--, j--) {
	(void) strcpy (f1, m_name (i));
	(void) strcpy (f2, m_scratch ("", invo_name));
	if (verbosw && i != msgnum)
	    printf (MSGSTR(TRANS, "message %d of digest %d becomes message %d\n"), j, msgnum, i); /*MSG*/

	if ((out = fopen (f2, "w")) == NULL)
	    adios (f2, MSGSTR(NOWRTM, "unable to write message %s"), f2); /*MSG*/
	(void) chmod (f2, mode);
	(void) fseek (in, pos = smsgs[j].s_start, 0);
	cpybrst (in, out, msgnam, f2,
		(int) (smsgs[j].s_stop - smsgs[j].s_start));
	(void) fclose (out);

	if (i == msgnum) {
	    (void) strcpy (f3, m_backup (f1));
	    if (rename (f1, f3) == NOTOK)
		admonish (f3, MSGSTR(NORENAME, "unable to rename %s to %s"), f1, f3); /*MSG*/
	}
	if (rename (f2, f1) == NOTOK)
	    admonish (f1, MSGSTR(NORENAME, "unable to rename %s to %s"), f2, f1); /*MSG*/
	mp -> msgstats[i] = mp -> msgstats[msgnum];
	mp -> msgflags |= SEQMOD;
    }

    (void) fclose (in);
}


/*  */

#define	S1	0
#define	S2	1
#define	S3	2

static cpybrst (register FILE   *in,
		register FILE  *out,
		register char   *ifile,
		register char   *ofile,
		register int	len    )
{
    register int    c,
                    state;

    for (state = S1; (c = fgetc (in)) != EOF && len > 0; len--) {
	if (c == (char)NULL)
	    continue;
	switch (state) {
	    case S1: 
		switch (c) {
		    case '-': 
			state = S3;
			break;

		    default: 
			state = S2;
		    case '\n': 
			(void) fputc (c, out);
			break;
		}
		break;

	    case S2: 
		switch (c) {
		    case '\n': 
			state = S1;
		    default: 
			(void) fputc (c, out);
			break;
		}
		break;

	    case S3: 
		switch (c) {
		    case ' ': 
			state = S2;
			break;

		    default: 
			state = c == '\n' ? S1 : S2;
			(void) fputc ('-', out);
			(void) fputc (c, out);
			break;
		}
		break;
	}
    }

    if (ferror (in))
	adios (ifile, MSGSTR(RERR, "error reading %s"), ifile); /*MSG*/
    if (ferror (out))
	adios (ofile, MSGSTR(WERR, "error writing %s"), ofile); /*MSG*/
}
