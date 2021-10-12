static char sccsid[] = "@(#)30	1.6  src/bos/usr/lib/sendmail/stats.c, cmdsend, bos411, 9428A410j 4/21/91 17:11:26";
/* 
 * COMPONENT_NAME: CMDSEND stats.c
 * 
 * FUNCTIONS: MSGSTR, emsg, markstats 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
*/


#include <nl_types.h>
#include "sendmail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

# include <stdio.h>
# include <ctype.h>
# include <errno.h>
# include <string.h>
# include <fcntl.h>

# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <netinet/in.h>
# include <sys/lockf.h>

# include "sendmail.h"
# include "mailstats.h"

char *xalloc();
off_t lseek();
long  time ();
#define cur_time	time ((long *) 0)

static emsg (char *, int);

/*
 *  MARKSTATS -- take statistics info to the stats file.
 *	
 *	Update the existing stats file if it exists and has the
 *	correct format.  Else, don't do it.  That is, don't wipe
 *	out older format versions of the stats file.  Let the system
 *	manager save the data first.  The only error notification he 
 *	gets is via console messages and log messages.
 */
markstats (toflag, mno, count)
int   toflag;			/* set if "to" instead of "from" count */
short mno;			/* mailer number affected */
long  count;			/* bytes */
{
    int fd;
    register int  i;
    struct statistics stat;
    int  size;

    /*
     *  Sendmail doesn't become highly aware of any exceptions in statistics 
     *  processing.  You don't want the sendmail user to be bothered.
     *  We just emit a message to the current user and to the console, plus
     *  a sendmail log message.  The current user only sees his message
     *  in verbose mode.
     */

    /*
     *  Make sure the mailer number is in range.
     */
    if (mno < 0 || mno > MAXMAILERS)
    {
	emsg (MSGSTR(ST_ESTAT, "statistics call error"), 0); /*MSG*/
	return;
    }

    /*
     *  Open the statistics file, if possible.
     */
    fd = open (StatFile, O_RDWR);
    if (fd < 0)				/* not there or inaccessible for I/O */
    {
	if (errno != ENOENT)		/* no such file means don't gather */
	    emsg (MSGSTR(ST_EOPEN, "statistics file open error"), 1); /*MSG*/
	errno = 0;			/* always clean up the mess */
	return;
    }

    /*
     *  Serialize multiple invocations of sendmail at this point.
     *  (Wait for exclusive use of this file).
     *  (Close frees the lock).
     */
    if (lockf (fd, F_LOCK, 0) < 0)
    {
	emsg (MSGSTR(ST_ELOCK, "statistics file lock error"), 1); /*MSG*/
	(void) close (fd);
	errno = 0;
	return;
    }

    /*
     *  Read the data structure from the disk.
     */
    size = read (fd, (char *) &stat, sizeof stat);
    if (size < 0)
    {
	(void) close (fd);
	emsg (MSGSTR(ST_EREAD, "statistics file read error"), 1); /*MSG*/
	errno = 0;
	return;
    }

    /*
     *  Check to see if we get a properly sized data structure from
     *  the disk.  Zero size is OK.  We will create the structure in that case.
     *  The first test with sizeof ensures that enough data was read for
     *  the second test with sizeof to be valid.
     */
    if (size != 0 && (size != sizeof stat || stat.stat_size != sizeof stat))
    {
	(void) close (fd);
	emsg (MSGSTR(ST_EFORMAT, "statistics not current format"), 1); /*MSG*/
	emsg (MSGSTR(ST_ETRUNC, "save and/or truncate with \"mailstats -z\""), 0); /*MSG*/
	errno = 0;
	return;
    }

    /*
     *  If structure was read, make sure the mailer names are properly in place.
     */
    if (size != 0)
    {
	/*
	 *  Search for name mismatch or end of mailer list.
	 */
	for (i=0; i<MAXMAILERS && Mailer[i] != NULL; i++)
	{
	    if (strcmp (Mailer[i]->m_name, stat.stat_mname[i]))
	    {
		(void) close (fd);
		emsg (MSGSTR(ST_EFORMAT, "statistics not current format"), 1); /*MSG*/
		emsg (MSGSTR(ST_ETRUNC, "save and/or truncate with \"mailstats -z\""), 0); /*MSG*/
		errno = 0;
		return;
	    }
	}

	/*
	 *  If at end of mailer list, search for more mailers in file.
	 */
	for ( ; i<MAXMAILERS; i++)
	{
	    if (*stat.stat_mname[i] != '\0')
	    {
		(void) close (fd);
		emsg (MSGSTR(ST_EFORMAT, "statistics not current format"), 1); /*MSG*/
		emsg (MSGSTR(ST_ETRUNC, "save and/or truncate with \"mailstats -z\""), 0); /*MSG*/
		errno = 0;
		return;
	    }
	}
    }

    /*
     *  If the data structure is to be newly created, do it now.
     */
    if (size == 0)
    {
	/*
	 *  The data structure from the file is invalid.
	 *
	 *  Prime the file data structure.
	 */
	stat.stat_itime = cur_time;
	stat.stat_size  = sizeof stat;

	for (i=0; i<MAXMAILERS; i++)
	{
	    /*
	     *  Fill in all names.  Use null strings where mailers aren't
	     *  defined.
	     */
	    if (Mailer[i] != NULL)
		    (void) strcpy (stat.stat_mname[i], Mailer[i]->m_name);
	    else
		    (void) strcpy (stat.stat_mname[i], "");

	    /*
	     *  Zero all counts.
	     */
	    stat.msgsfrom[i]  = 0;
	    stat.bytesfrom[i] = 0;
	    stat.msgsto[i]    = 0;
	    stat.bytesto[i]   = 0;
	}
    }

    /*
     *  Update the file data structure.
     */
    if (toflag)
    {
        stat.msgsto[mno]    += 1;
        stat.bytesto[mno]   += count;
    }
    else
    {
        stat.msgsfrom[mno]  += 1;
        stat.bytesfrom[mno] += count;
    }

    /*
     *  Write back the file data structure.
     */
    if (lseek (fd, (off_t) 0, 0) < 0                      || 
	write (fd, (char *) &stat, sizeof stat) != sizeof stat)
	emsg (MSGSTR(ST_EWRITE, "statistics file write error"), 1); /*MSG*/

    (void) close (fd);
    errno = 0;
}

static
emsg (char *s, int flag)
{
    FILE  *fp;
    char *hlds;

    hlds = newstr(s);			/* save it */
    if (flag)
        nmessage (0, MSGSTR(ST_MS, "000 sendmail: %s, file \"%s\""), hlds, StatFile); /*MSG*/
    else
	nmessage (0, MSGSTR(ST_SM2, "000 sendmail: %s"), hlds); /*MSG*/

    /*
     *  Write msg to console, if possible.
     */
    fp = fopen ("/dev/console", "a");
    if (fp != NULL)
    {
	if (flag)
	    (void) fprintf (fp, MSGSTR(ST_SM3, "sendmail: %s, file \"%s\"\r\n"), hlds, StatFile); /*MSG*/
	else
	    (void) fprintf (fp, MSGSTR(ST_SM4, "sendmail: %s\r\n"), hlds); /*MSG*/

	(void) fclose (fp);
    }
    errno = 0;

# ifdef LOG
    /*
     *  Create a message in the sendmail log file.
     */
    if (flag)
        syslog (LOG_ERR, MSGSTR(ST_LOG, "%s, file \"%s\""), hlds, StatFile); /*MSG*/
    else
	syslog (LOG_ERR, "%s", hlds);
    free(hlds);
# endif LOG
}
