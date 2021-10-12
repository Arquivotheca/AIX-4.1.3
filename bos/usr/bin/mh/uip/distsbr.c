static char sccsid[] = "@(#)50	1.8  src/bos/usr/bin/mh/uip/distsbr.c, cmdmh, bos411, 9428A410j 3/28/91 14:50:26";
/* 
 * COMPONENT_NAME: CMDMH distsbr.c
 * 
 * FUNCTIONS: MSGSTR, distout, ready_msg 
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
/*static char sccsid[] = "distsbr.c	7.1 87/10/13 17:25:53"; */

/* distsbr.c - routines to do additional "dist-style" processing */

#include "mh.h"
#include <ctype.h>
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


static int  hdrfd = NOTOK;
static int  txtfd = NOTOK;
static        ready_msg (register char   *);

long	lseek ();

char *strcpy();

/*  */

#define	BADHDR	"please re-edit %s to remove the ``%s'' header!"
#define	BADTXT	"please re-edit %s to consist of headers only!"
#define	BADMSG	"please re-edit %s to include a ``Resent-To:''!"
#define	BADRFT	"please re-edit %s and fix that header!"

int	distout (drft, msgnam, backup)
register char   *drft,
       *msgnam,
       *backup;
{
    int     state;
    register char  *dp,
                   *resent;
    char    name[NAMESZ],
            buffer[BUFSIZ];
    char    hlds[NL_TEXTMAX];
    register    FILE *ifp,
		     *ofp;

    if (rename (drft, strcpy (backup, m_backup (drft))) == NOTOK)
	adios (backup, MSGSTR(NORENAME, "unable to rename %s to %s"),drft, backup); /*MSG*/
    if ((ifp = fopen (backup, "r")) == NULL)
	adios (backup, MSGSTR(NOREAD, "unable to read %s"), backup); /*MSG*/

    if ((ofp = fopen (drft, "w")) == NULL)
	adios (drft, MSGSTR(NOTMPF2, "unable to create temporary file %s"), drft); /*MSG*/
    (void) chmod (drft, m_gmprot ());

    ready_msg (msgnam);
    (void) lseek (hdrfd, 0L, 0);	/* msgnam not accurate */
    cpydata (hdrfd, fileno (ofp), msgnam, drft);

/*  */

    for (state = FLD, resent = NULL;;)
	switch (state =
		m_getfld (state, name, buffer, sizeof buffer, ifp)) {
	    case FLD: 
	    case FLDPLUS: 
	    case FLDEOF: 
		if (uprf (name, "distribute-"))
		    (void) sprintf (name, "%s%s", "Resent", &name[10]);
		if (uprf (name, "distribution-"))
		    (void) sprintf (name, "%s%s", "Resent", &name[12]);
		if (!uprf (name, "resent")) {
		    strcpy (hlds, MSGSTR(BADHDR1, BADHDR)); /*MSG*/ 
		    advise (NULLCP, hlds, MSGSTR(DRFT, "draft"), name); /*MSG*/
		    goto leave_bad;
		}
		if (state == FLD)
		    resent = add (":", add (name, resent));
		resent = add (buffer, resent);
		fprintf (ofp, "%s: %s", name, buffer);
		while (state == FLDPLUS) {
		    state = m_getfld (state, name,
			    buffer, sizeof buffer, ifp);
		    resent = add (buffer, resent);
		    fputs (buffer, ofp);
		}
		if (state == FLDEOF)
		    goto process;
		break;

	    case BODY: 
	    case BODYEOF: 
		for (dp = buffer; *dp; dp++)
		    if (!isspace (*dp)) {
		        strcpy (hlds, MSGSTR(BADTXT1, BADTXT)); /*MSG*/ 
			advise (NULLCP, hlds, MSGSTR(DRFT, "draft")); /*MSG*/
			goto leave_bad;
		    }

	    case FILEEOF: 
		goto process;

	    case LENERR: 
	    case FMTERR: 
		strcpy (hlds, MSGSTR(BADRFT1, BADRFT)); /*MSG*/ 
		advise (NULLCP, hlds, MSGSTR(DRFT, "draft")); /*MSG*/
	leave_bad: ;
		(void) fclose (ifp);
		(void) fclose (ofp);
		(void) unlink (drft);
		if (rename (backup, drft) == NOTOK)
		    adios (drft, MSGSTR(NORENAME, "unable to rename %s to %s"), backup, drft); /*MSG*/
		return NOTOK;

	    default: 
		adios (NULLCP, MSGSTR(RET, "getfld() returned %d"), state); /*MSG*/
	}
process: ;
    (void) fclose (ifp);
    (void) fflush (ofp);

/*  */

    if (!resent) {
	strcpy (hlds, MSGSTR(BADMSG1, BADMSG)); /*MSG*/ 
	advise (NULLCP, hlds, MSGSTR(DRFT, "draft")); /*MSG*/
	(void) fclose (ofp);
	(void) unlink (drft);
	if (rename (backup, drft) == NOTOK)
	    adios (drft, MSGSTR(NORENAME, "unable to rename %s to %s"), backup, drft); /*MSG*/
	return NOTOK;
    }
    free (resent);

    if (txtfd != NOTOK) {
	(void) lseek (txtfd, 0L, 0);	/* msgnam not accurate */
	cpydata (txtfd, fileno (ofp), msgnam, drft);
    }

    (void) fclose (ofp);

    return OK;
}

/*  */

static	ready_msg (register char   *msgnam)
{
    int     state,
            out;
    char    name[NAMESZ],
            buffer[BUFSIZ],
            tmpfil[BUFSIZ];
    register    FILE *ifp,
		     *ofp;

    if (hdrfd != NOTOK)
	(void) close (hdrfd), hdrfd = NOTOK;
    if (txtfd != NOTOK)
	(void) close (txtfd), txtfd = NOTOK;

    if ((ifp = fopen (msgnam, "r")) == NULL)
	adios (msgnam, MSGSTR(NOOPENM, "unable to open message %s"), msgnam); /*MSG*/

    (void) strcpy (tmpfil, m_tmpfil ("dist"));
    if ((hdrfd = creat (tmpfil, 0600)) == NOTOK)
	adios (tmpfil, MSGSTR(NOTMPF2, "unable to create temporary file %s"), tmpfil); /*MSG*/
    (void) close (hdrfd);
    if ((hdrfd = open (tmpfil, 2)) == NOTOK)
	adios (tmpfil, MSGSTR(NOOPENTF, "unable to re-open temporary file %s"), tmpfil); /*MSG*/
    if ((out = dup (hdrfd)) == NOTOK
	    || (ofp = fdopen (out, "w")) == NULL)
	adios (NULLCP, MSGSTR(NOFDESC, "no file descriptors -- you lose big")); /*MSG*/
    (void) unlink (tmpfil);

/*  */

    for (state = FLD;;)
	switch (state =
		m_getfld (state, name, buffer, sizeof buffer, ifp)) {
	    case FLD: 
	    case FLDPLUS: 
	    case FLDEOF: 
		if (uprf (name, "resent"))
		    fprintf (ofp, "Prev-");
		fprintf (ofp, "%s: %s", name, buffer);
		while (state == FLDPLUS) {
		    state = m_getfld (state, name,
			    buffer, sizeof buffer, ifp);
		    fputs (buffer, ofp);
		}
		if (state == FLDEOF)
		    goto process;
		break;

	    case BODY: 
	    case BODYEOF: 
		(void) fclose (ofp);

		(void) strcpy (tmpfil, m_tmpfil ("dist"));
		if ((txtfd = creat (tmpfil, 0600)) == NOTOK)
		    adios (tmpfil, MSGSTR(NOTMPF2, "unable to create temporary file %s"), tmpfil); /*MSG*/
		(void) close (txtfd);
		if ((txtfd = open (tmpfil, 2)) == NOTOK)
		    adios (tmpfil, MSGSTR(NOOPENTF, "unable to re-open temporary file %s"), tmpfil); /*MSG*/
		if ((out = dup (txtfd)) == NOTOK
			|| (ofp = fdopen (out, "w")) == NULL)
		    adios (NULLCP, MSGSTR(NOFDESC, "no file descriptors -- you lose big")); /*MSG*/
		(void) unlink (tmpfil);
		fprintf (ofp, "\n%s", buffer);
		while (state == BODY) {
		    state = m_getfld (state, name,
			    buffer, sizeof buffer, ifp);
		    fputs (buffer, ofp);
		}
	    case FILEEOF: 
		goto process;

	    case LENERR: 
	    case FMTERR: 
		adios (NULLCP, MSGSTR(MSGFERROR, "format error in message %s"), msgnam); /*MSG*/

	    default: 
		adios (NULLCP, MSGSTR(RET, "getfld() returned %d"), state); /*MSG*/
	}
process: ;
    (void) fclose (ifp);
    (void) fclose (ofp);
}
