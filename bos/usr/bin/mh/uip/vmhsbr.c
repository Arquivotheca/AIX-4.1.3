static char sccsid[] = "@(#)95	1.9  src/bos/usr/bin/mh/uip/vmhsbr.c, cmdmh, bos411, 9428A410j 3/28/91 16:09:32";
/* 
 * COMPONENT_NAME: CMDMH vmhsbr.c
 * 
 * FUNCTIONS: MSGSTR, err2peer, fmt2peer, peer2rc, rc2peer, rc2rc, 
 *            rcdone, rcinit, rclose, str2peer, str2rc 
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
/* static char sccsid[] = "vmhsbr.c	8.1 88/04/15 15:51:12"; */

/* vmhsbr.c - routines to help vmh along */

/* TODO (for vrsn 2):
	INI: include width of windows
 */

#include "mh.h"
#include "vmhsbr.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static char *types[] = {
    "OK",
    "INI", "ACK", "ERR", "CMD", "QRY", "TTY", "WIN", "DATA", "EOF", "FIN",
    "XXX", NULL
};

static	FILE *fp = NULL;

static	int PEERrfd = NOTOK;
static	int PEERwfd = NOTOK;

char *getenv();

extern int  errno;
extern int  sys_nerr;

/*  */

int	rcinit (rfd, wfd)
int	rfd,
	wfd;
{
    char   *cp,
            buffer[BUFSIZ];

    PEERrfd = rfd;
    PEERwfd = wfd;

    (void) sprintf (buffer, "%s.out", invo_name);
    if ((cp = getenv ("MHVDEBUG"))
	    && *cp
	    && (fp = fopen (buffer, "w"))) {
	(void) fseek (fp, 0L, 2);
	fprintf (fp, "%d: rcinit (%d, %d)\n", getpid (), rfd, wfd);
	(void) fflush (fp);
    }

    return OK;
}


int     rcdone () {
    if (PEERrfd != NOTOK)
	(void) close (PEERrfd);
    if (PEERwfd != NOTOK)
	(void) close (PEERwfd);

    if (fp) {
	(void) fclose (fp);
	fp = NULL;
    }
    return OK;
}

/*  */

int	rc2rc (code, len, data, rc)
char	code;
int	len;
char   *data;
struct record *rc;
{
    if (rc2peer (code, len, data) == NOTOK)
	return NOTOK;

    return peer2rc (rc);
}


int	str2rc (code, str, rc)
char	code;
char   *str;
struct record *rc;
{
    return rc2rc (code, str ? strlen (str) : 0, str, rc);
}

/*  */

int	peer2rc (rc)
register struct	record *rc;
{
    if (rc -> rc_data)
	free (rc -> rc_data);

    if (read (PEERrfd, (char *) rc_head (rc), RHSIZE (rc)) != RHSIZE (rc))
	return rclose (rc, MSGSTR(PLOST, "read from peer lost(1)")); /*MSG*/
    if (rc -> rc_len) {
	if ((rc -> rc_data = (char *)malloc ((unsigned) rc -> rc_len+5)) == NULL)
	    return rclose (rc, MSGSTR(MLOST, "malloc of %d lost"), rc -> rc_len); /*MSG*/
	if (read (PEERrfd, rc -> rc_data, rc -> rc_len) != rc -> rc_len)
	    return rclose (rc, MSGSTR(PLOST2, "read from peer lost(2)")); /*MSG*/
	rc -> rc_data[rc -> rc_len] = (char)NULL;
    }
    else
	rc -> rc_data = NULL;

    if (fp) {
	(void) fseek (fp, 0L, 2);
	fprintf (fp, "%d: <--- %s %d: \"%*.*s\"\n", getpid (),
		types[rc -> rc_type], rc -> rc_len,
		rc -> rc_len, rc -> rc_len, rc -> rc_data);
	(void) fflush (fp);
    }

    return rc -> rc_type;
}

/*  */

int	rc2peer (code, len, data)
char	code;
int	len;
char   *data;
{
    struct record   rcs;
    register struct record *rc = &rcs;

    rc -> rc_type = code;
    rc -> rc_len = len;

    if (fp) {
	(void) fseek (fp, 0L, 2);
	fprintf (fp, "%d: ---> %s %d: \"%*.*s\"\n", getpid (),
		types[rc -> rc_type], rc -> rc_len,
		rc -> rc_len, rc -> rc_len, data);
	(void) fflush (fp);
    }

    if (write (PEERwfd, (char *) rc_head (rc), RHSIZE (rc)) != RHSIZE (rc))
	return rclose (rc, MSGSTR(WPLOST, "write to peer lost(1)")); /*MSG*/

    if (rc -> rc_len)
	if (write (PEERwfd, data, rc -> rc_len) != rc -> rc_len)
	    return rclose (rc, MSGSTR(WPLOST2, "write to peer lost(2)")); /*MSG*/

    return OK;
}

/*  */

int	str2peer (code, str)
char	code;
char   *str;
{
    return rc2peer (code, str ? strlen (str) : 0, str);
}


/* VARARGS2 */

int	fmt2peer (code, fmt, a, b, c, d, e, f)
char	code;
char   *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    return err2peer (code, NULLCP, fmt, a, b, c, d, e, f);
}

/*  */

/* VARARGS3 */

int	err2peer (code, what, fmt, a, b, c, d, e, f)
char	code;
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    int     eindex = errno;
    register char  *bp;
    char    buffer[BUFSIZ * 2];

    (void) sprintf (buffer, fmt, a, b, c, d, e, f);
    bp = buffer + strlen (buffer);
    if (what) {
	if (eindex > 0 && eindex < sys_nerr)
	    (void) strcpy (bp, strerror(eindex));
	else
	    (void) sprintf (bp, MSGSTR(ERRORNO, ": Error %d"), eindex); /*MSG*/
	bp += strlen (bp);
    }

    return rc2peer (code, bp - buffer, buffer);
}

/*  */

/* VARARGS2 */

int	rclose (rc, fmt, a, b, c, d, e, f)
register struct record *rc;
char   *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    static char buffer[BUFSIZ * 2];

    (void) sprintf (buffer, fmt, a, b, c, d, e, f);

    rc -> rc_len = strlen (rc -> rc_data = getcpy (buffer));
    return (rc -> rc_type = RC_XXX);
}
