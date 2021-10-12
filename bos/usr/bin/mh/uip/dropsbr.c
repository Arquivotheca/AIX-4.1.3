static char sccsid[] = "@(#)52        1.7  src/bos/usr/bin/mh/uip/dropsbr.c, cmdmh, bos411, 9428A410j 3/28/91 14:50:33";
/* 
 * COMPONENT_NAME: CMDMH dropsbr.c
 * 
 * FUNCTIONS: MSGSTR, map_chk, map_name, map_open, map_read, 
 *            map_write, mbx_Xopen, mbx_chk, mbx_close, mbx_copy, 
 *            mbx_create, mbx_mmdf, mbx_open, mbx_read, mbx_size, 
 *            mbx_uucp, mbx_write 
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
/* static char sccsid[] = "dropsbr.c	7.1 87/10/13 17:26:33"; */

/* dropsbr.c - write to a mailbox */

#include <stdio.h>
#ifndef	MMDFONLY
#include "mh.h"
#include "dropsbr.h"
#include "mts.h"
#else	MMDFONLY
#include "dropsbr.h"
#include "strings.h"
#include "mmdfonly.h"
#endif	MMDFONLY
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


#define	MMDF	1
#define	UUCP	2

/*  */

char *index();

static	int	mbx_style = MMDF;
static int  mbx_create (char *, int, int, int);
static int  mbx_chk (int);
static int  map_open (char *, int *, int);
extern int  errno;


long   lseek ();

/*  */

int     mbx_mmdf () {
    int     style = mbx_style;

    mbx_style = MMDF;
    return style;
}


int     mbx_uucp () {
    int     style = mbx_style;

    mbx_style = UUCP;
    return style;
}

/*  */

int     mbx_open (file, uid, gid, mode)
char   *file;
int     uid,
        gid,
	mode;
{
    int     clear,
            fd;

    if ((fd = mbx_Xopen (file, uid, gid, mode, &clear)) == NOTOK)
	return fd;

    if (!clear)
	switch (mbx_style) {
	    case MMDF: 
	    default: 
		if (mbx_chk (fd) == NOTOK) {
		    (void) close (fd);
		    return NOTOK;
		}
		break;

	    case UUCP: 
		if (lseek (fd, 0L, 2) == (long) NOTOK) {
		    (void) close (fd);
		    return NOTOK;
		}
		break;
	}

    return fd;
}

/*  */

int	mbx_Xopen (file, uid, gid, mode, clear)
char   *file;
int	uid,
	gid,
	mode,
       *clear;
{
    register int j;
    int	    count,
            fd;
    struct stat st;

    for (*clear = 0, count = 4, j = 0; count > 0; count--)
	if ((fd = lkopen (file, 6)) == NOTOK)
	    switch (errno) {
		case ENOENT: 
		    if (mbx_create (file, uid, gid, mode) == NOTOK)
			return NOTOK;
		    (*clear)++;
		    break;

#ifdef	BSD42
		case EWOULDBLOCK:
#endif	BSD42
		case ETXTBSY: 
		    j = errno;
		    sleep (5);
		    break;

		default: 
		    return NOTOK;
	    }
	else {
	    *clear = fstat (fd, &st) != NOTOK && st.st_size == 0L;
	    break;
	}

    errno = j;
    return fd;
}

/*  */

static int  mbx_create (char *file, int uid, int gid, int mode)
{
    int     fd;

    if ((fd = creat (file, 0600)) == NOTOK)
	return NOTOK;

    (void) close (fd);
    (void) chown (file, uid, gid);
    (void) chmod (file, mode);

    return OK;
}


static int  mbx_chk (int fd)
{
    int     count;
    char    ldelim[BUFSIZ];

    count = strlen (mmdlm2);

    if (lseek (fd, (long) (-count), 2) == (long) NOTOK
	    || read (fd, ldelim, count) != count)
	return NOTOK;
    ldelim[count] = (char)NULL;

    if (strcmp (ldelim, mmdlm2)
	    && write (fd, "\n", 1) != 1
	    && write (fd, mmdlm2, count) != count)
	return NOTOK;

    return OK;
}

/*  */

int	mbx_read (fp, pos, drops, noisy)
register FILE  *fp;
register long	pos;
struct drop **drops;
int	noisy;
{
    register int    len,
                    size;
    long    ld1,
            ld2;
    register char  *bp;
    char    buffer[BUFSIZ];
    register struct drop   *cp,
                           *dp,
                           *ep,
                           *pp;

    pp = (struct drop  *) calloc ((unsigned) (len = MAXFOLDER), sizeof *dp);
    if (pp == NULL) {
	if (noisy)
	    admonish (NULLCP, MSGSTR(NOADSTOR, "unable to allocate drop storage")); /*MSG*/
	return NOTOK;
    }

    ld1 = (long) strlen (mmdlm1);
    ld2 = (long) strlen (mmdlm2);

    (void) fseek (fp, pos, 0);
    for (ep = (dp = pp) + len - 1; fgets (buffer, sizeof buffer, fp);) {
	size = 0;
	if (strcmp (buffer, mmdlm1) == 0)
	    pos += ld1, dp -> d_start = pos;
	else {
	    dp -> d_start = pos, pos += (long) strlen (buffer);
	    for (bp = buffer; *bp; bp++, size++)
		if (*bp == '\n')
		    size++;
	}

	while (fgets (buffer, sizeof buffer, fp) != NULL)
	    if (strcmp (buffer, mmdlm2) == 0)
		break;
	    else {
		pos += (long) strlen (buffer);
		for (bp = buffer; *bp; bp++, size++)
		    if (*bp == '\n')
			size++;
	    }

	if (dp -> d_start != pos) {
	    dp -> d_id = 0;
	    dp -> d_size = size;
	    dp -> d_stop = pos;
	    dp++;
	}
	pos += ld2;

	if (dp >= ep) {
	    register int    curlen = dp - pp;

	    cp = (struct drop  *) realloc ((char *) pp,
		                    (unsigned) (len += MAXFOLDER) * sizeof *pp);
	    if (cp == NULL) {
		if (noisy)
		    admonish (NULLCP, MSGSTR(NOADSTOR, "unable to allocate drop storage")); /*MSG*/
		free ((char *) pp);
		return 0;
	    }
	    dp = cp + curlen, ep = (pp = cp) + len - 1;
	}
    }

    if (dp == pp)
	free ((char *) pp);
    else
	*drops = pp;
    return (dp - pp);
}

/*  */

int	mbx_write (mailbox, md, fp, id, pos, stop, mapping, noisy)
char   *mailbox;
register FILE *fp;
int     md,
	id,
	mapping,
	noisy;
register long	pos,
		stop;
{
    register int    i,
                    j,
                    size;
    register long   start,
                    off;
    register char  *cp;
    char    buffer[BUFSIZ];

    off = lseek (md, 0L, 1);
    j = strlen (mmdlm1);
    if (write (md, mmdlm1, j) != j)
	return NOTOK;
    start = lseek (md, 0L, 1);
    size = 0;

    (void) fseek (fp, pos, 0);
    while (fgets (buffer, sizeof buffer, fp) != NULL && pos < stop) {
	i = strlen (buffer);
	for (j = 0; (j = stringdex (mmdlm1, buffer)) >= 0; buffer[j]++)
	    continue;
	for (j = 0; (j = stringdex (mmdlm2, buffer)) >= 0; buffer[j]++)
	    continue;
	if (write (md, buffer, i) != i)
	    return NOTOK;
	pos += (long) i;
	if (mapping)
	    for (cp = buffer; i-- > 0; size++)
		if (*cp++ == '\n')
		    size++;
    }

    stop = lseek (md, 0L, 1);
    j = strlen (mmdlm2);
    if (write (md, mmdlm2, j) != j)
	return NOTOK;
    if (mapping)
	(void) map_write (mailbox, md, id, start, stop, off, size, noisy);

    return OK;
}

/*  */

int     mbx_copy (mailbox, md, fd, mapping, text, noisy)
char   *mailbox;
int     md,
        fd,
	mapping,
	noisy;
char   *text;
{
    register int    i,
                    j,
                    size;
    register long   start,
                    stop,
                    pos;
    register char  *cp;
    char    buffer[BUFSIZ];
    register FILE  *fp;

    pos = lseek (md, 0L, 1);
    size = 0;

    switch (mbx_style) {
	case MMDF: 
	default: 
	    j = strlen (mmdlm1);
	    if (write (md, mmdlm1, j) != j)
		return NOTOK;
	    start = lseek (md, 0L, 1);

	    if (text) {
		i = strlen (text);
		if (write (md, text, i) != i)
		    return NOTOK;
		for (cp = buffer; *cp++; size++)
		    if (*cp == '\n')
			size++;
	    }
		    
	    while ((i = read (fd, buffer, sizeof buffer)) > 0) {
		for (j = 0;
			(j = stringdex (mmdlm1, buffer)) >= 0;
			buffer[j]++)
		    continue;
		for (j = 0;
			(j = stringdex (mmdlm2, buffer)) >= 0;
			buffer[j]++)
		    continue;
		if (write (md, buffer, i) != i)
		    return NOTOK;
		if (mapping)
		    for (cp = buffer; i-- > 0; size++)
			if (*cp++ == '\n')
			    size++;
	    }

	    stop = lseek (md, 0L, 1);
	    j = strlen (mmdlm2);
	    if (write (md, mmdlm2, j) != j)
		return NOTOK;
	    if (mapping)
		(void) map_write (mailbox, md, 0, start, stop, pos, size, noisy);

	    return (i != NOTOK ? OK : NOTOK);

	case UUCP: 		/* I hate this... */
	    if ((j = dup (fd)) == NOTOK)
		return NOTOK;
	    if ((fp = fdopen (j, "r")) == NULL) {
		(void) close (j);
		return NOTOK;
	    }
	    start = lseek (md, 0L, 1);

	    if (text) {
		i = strlen (text);
		if (write (md, text, i) != i)
		    return NOTOK;
		for (cp = buffer; *cp++; size++)
		    if (*cp == '\n')
			size++;
	    }
		    
	    for (j = 0; fgets (buffer, sizeof buffer, fp) != NULL; j++) {
		if (j != 0 && strncmp (buffer, "From ", 5) == 0) {
		    (void) write (fd, ">", 1);
		    size++;
		}
		i = strlen (buffer);
		if (write (md, buffer, i) != i) {
		    (void) fclose (fp);
		    return NOTOK;
		}
		if (mapping)
		    for (cp = buffer; i-- > 0; size++)
			if (*cp++ == '\n')
			    size++;
	    }

	    (void) fclose (fp);
	    (void) lseek (fd, 0L, 2);
	    stop = lseek (md, 0L, 1);
	    if (mapping)
		(void) map_write (mailbox, md, 0, start, stop, pos, size,
			    noisy);

	    return OK;
    }
}

/*  */

int	mbx_size (md, start, stop)
int	md;
long	start,
	stop;
{
    register int    i,
                    fd;
    register long   pos;
    register FILE  *fp;

    if ((fd = dup (md)) == NOTOK || (fp = fdopen (fd, "r")) == NULL) {
	if (fd != NOTOK)
	    (void) close (fd);
	return NOTOK;
    }

    (void) fseek (fp, start, 0);
    for (i = 0, pos = stop - start; pos-- > 0; i++)
	if (fgetc (fp) == '\n')
	    i++;

    (void) fclose (fp);

    return i;
}

/*  */

int     mbx_close (mailbox, md)
char   *mailbox;
int     md;
{
    (void) lkclose (md, mailbox);

    return OK;
}

/*  */

/* This function is performed implicitly by getbbent.c:

		bb -> bb_map = map_name (bb -> bb_file);
*/

char    *map_name (file)
register char	*file;
{
    register char  *cp,
                   *dp;
    static char buffer[BUFSIZ];

    if ((dp = index (cp = r1bindex (file, '/'), '.')) == NULL)
	dp = cp + strlen (cp);
    if (cp == file)
	(void) sprintf (buffer, ".%.*s%s", dp - cp, cp, ".map");
    else
	(void) sprintf (buffer, "%.*s.%.*s%s", cp - file, file, dp - cp,
		cp, ".map");

    return buffer;
}

/*  */

int	map_read (file, pos, drops, noisy)
char   *file;
long	pos;
struct drop **drops;
int	noisy;
{
    register int    i,
                    md,
                    msgp;
    register char  *cp;
    struct drop d;
    register struct drop   *mp,
                           *dp;

    if ((md = open (cp = map_name (file), 0)) == NOTOK
	    || map_chk (cp, md, mp = &d, pos, noisy)) {
	if (md != NOTOK)
	    (void) close (md);
	return 0;
    }

    msgp = mp -> d_id;
    dp = (struct drop  *) calloc ((unsigned) msgp, sizeof *dp);
    if (dp == NULL) {
	(void) close (md);
	return 0;
    }

    (void) lseek (md, (long) sizeof *mp, 0);
    if ((i = read (md, (char *) dp, msgp * sizeof *dp)) < sizeof *dp) {
	i = 0;
	free ((char *) dp);
    }
    else
	*drops = dp;

    (void) close (md);

    return (i / sizeof *dp);
}

/*  */

int     map_write (mailbox, md, id, start, stop, pos, size, noisy)
register char   *mailbox;
int     md,
        id,
	size,
	noisy;
long    start,
        stop,
        pos;
{
    register int    i;
    int     clear,
            fd,
            td;
    char   *file;
    register struct drop   *dp;
    struct drop    d1,
		   d2,
                  *rp;
    register FILE *fp;

    if ((fd = map_open (file = map_name (mailbox), &clear, md)) == NOTOK)
	return NOTOK;

    if (!clear && map_chk (file, fd, &d1, pos, noisy)) {
	(void) unlink (file);
	(void) mbx_close (file, fd);
	if ((fd = map_open (file, &clear, md)) == NOTOK)
	    return NOTOK;
	clear++;
    }

    if (clear) {
	if ((td = dup (md)) == NOTOK || (fp = fdopen (td, "r")) == NULL) {
	    if (noisy)
		if (td != NOTOK)
		    admonish (file, MSGSTR(NOFDOPEN2, "unable to fdopen %s"), file); /*MSG*/
		else
		    admonish (file, MSGSTR(NODUP2, "unable to dup %s"), file); /*MSG*/
	    if (td != NOTOK)
		(void) close (td);
	    (void) mbx_close (file, fd);
	    return NOTOK;
	}

	switch (i = mbx_read (fp, 0L, &rp, noisy)) {
	    case NOTOK:
		(void) fclose (fp);
		(void) mbx_close (file, fd);
		return NOTOK;

	    case OK:
		break;

	    default:
		d1.d_id = 0;
		for (dp = rp; i-- >0; dp++) {
		    if (dp -> d_start == start)
			dp -> d_id = id;
		    (void) lseek (fd, (long) (++d1.d_id * sizeof *dp), 0);
		    if (write (fd, (char *) dp, sizeof *dp) != sizeof *dp) {
			if (noisy)
			    admonish (file, MSGSTR(WERROR, "write error %s"), file); /*MSG*/
			(void) mbx_close (file, fd);
			(void) fclose (fp);
			return NOTOK;
		    }
		}
		free ((char *) rp);
		break;
	}
    }
    else {
	dp = &d2;
	dp -> d_id = id;
	dp -> d_size = size ? size : mbx_size (fd, start, stop);
	dp -> d_start = start;
	dp -> d_stop = stop;
	(void) lseek (fd, (long) (++d1.d_id * sizeof *dp), 0);
	if (write (fd, (char *) dp, sizeof *dp) != sizeof *dp) {
	    if (noisy)
		admonish (file, MSGSTR(WERROR, "write error %s"), file); /*MSG*/
	    (void) mbx_close (file, fd);
	    return NOTOK;
	}
    }

    dp = &d1;
    dp -> d_size = DRVRSN;
    dp -> d_start = DRMAGIC;
    dp -> d_stop = lseek (md, 0L, 1);
    (void) lseek (fd, 0L, 0);
    if (write (fd, (char *) dp, sizeof *dp) != sizeof *dp) {
	if (noisy)
	    admonish (file, MSGSTR(WERROR, "write error %s"), file); /*MSG*/
	(void) mbx_close (file, fd);
	return NOTOK;
    }

    (void) mbx_close (file, fd);

    return OK;
}

/*  */

static int  map_open (char *file, int *clear, int md)
{
    int	    mode;
    struct  stat st;

    mode = fstat (md, &st) != NOTOK ? (int) (st.st_mode & 0777) : m_gmprot ();
    return mbx_Xopen (file, st.st_uid, st.st_gid, mode, clear);
}

/*  */

int  map_chk (file, fd, dp, pos, noisy)
char   *file;
int     fd,
	noisy;
register struct drop *dp;
long	pos;
{
    long    count;
    struct drop d;
    register struct drop    *dl;

    if (read (fd, (char *) dp, sizeof *dp) != sizeof *dp) {
#ifdef	notdef
	admonish (NULLCP, MSGSTR(BADINDEX, "%s: missing or partial index"), file); /*MSG*/
#endif	notdef
	return NOTOK;
    }
    if (dp -> d_size != DRVRSN) {
	if (noisy)
	    admonish (NULLCP, MSGSTR(BADMATCH, "%s: version mismatch"), file); /*MSG*/
	return NOTOK;
    }
    if (dp -> d_start != DRMAGIC) {
	if (noisy)
	    admonish (NULLCP, MSGSTR(BADMNUM, "%s: bad magic number"), file); /*MSG*/
	return NOTOK;
    }
    if (dp -> d_stop != pos) {
	if (noisy && pos != 0L)
	    admonish (NULLCP,
		    MSGSTR(BADMI, "%s: pointer mismatch or incomplete index (%ld!=%ld)"),  file, dp -> d_stop, pos); /*MSG*/
	return NOTOK;
    }

    if ((long) ((dp -> d_id + 1) * sizeof *dp) != lseek (fd, 0L, 2)) {
	if (noisy)
	    admonish (NULLCP, MSGSTR(CPTINDX1, "%s: corrupt index(1)"), file); /*MSG*/
	return NOTOK;
    }

    dl = &d;
    count = (long) strlen (mmdlm2);
    (void) lseek (fd, (long) (dp -> d_id * sizeof *dp), 0);
    if (read (fd, (char *) dl, sizeof *dl) != sizeof *dl
	    || (dl -> d_stop != dp -> d_stop
		&& dl -> d_stop + count != dp -> d_stop)) {
	if (noisy)
	    admonish (NULLCP, MSGSTR(CPTINDX2, "%s: corrupt index(2)"), file); /*MSG*/
	return NOTOK;
    }

    return OK;
}
