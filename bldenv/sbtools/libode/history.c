/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: ATOI
 *		addtobody
 *		check_branches
 *		checklogstate
 *		copyrights_ok
 *		create_leaderless_log1684
 *		getline
 *		hst_add_comment
 *		hst_alloc_entry
 *		hst_dump_list
 *		hst_dump_list_fd2282
 *		hst_freelist
 *		hst_get_date
 *		hst_getline
 *		hst_insert_file
 *		hst_lookup_logmsg
 *		hst_mark_common_entries2247
 *		hst_merge_lists
 *		hst_next_revision2317
 *		hst_xtract_file
 *		hstdup
 *		if
 *		is_whist_header
 *		logalloc
 *		logfree
 *		month_num
 *		new_comment
 *		new_hist_elem
 *		okmesg
 *		process_log_messages
 *		reinsert_log_messages1145
 *		remove_ibr_markers
 *		save_log_message1065
 *		save_stamp_match
 *		savelog
 *		search_for
 *		src_ctl_extract_history
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1993, 1992, 1991, 1990 Open Software Foundation, Inc.
 *
 * Permission is hereby granted to use, copy, modify and freely distribute
 * the software in this file and its documentation for any purpose without
 * fee, provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.  Further, provided that the name of Open
 * Software Foundation, Inc. ("OSF") not be used in advertising or
 * publicity pertaining to distribution of the software without prior
 * written permission from OSF.  OSF makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 * 
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 *  
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *  
 * Carnegie Mellon requests users of this software to return to
 * 
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU
 * School of Computer Science
 * Carnegie Mellon University
 * Pittsburgh PA 15213-3890
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * HISTORY
 * $Log: history.c,v $
 * Revision 1.1.8.5  1993/11/24  16:56:20  damon
 * 	CR 865. Not so picky about separation of copyrights and history
 * 	[1993/11/24  16:56:10  damon]
 *
 * Revision 1.1.8.4  1993/11/10  19:17:03  root
 * 	CR 463. Pedantic changes
 * 	[1993/11/10  19:16:13  root]
 * 
 * Revision 1.1.8.3  1993/11/08  17:58:40  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  16:08:29  damon]
 * 
 * Revision 1.1.8.2  1993/11/05  23:19:01  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  23:18:29  damon]
 * 
 * Revision 1.1.8.1  1993/10/21  20:40:55  root
 * 	CR 716. Complain if bad leader between COPYRIGHT and HISTORY
 * 	[1993/10/21  20:40:31  root]
 * 
 * Revision 1.1.6.6  1993/09/24  18:56:36  marty
 * 	CR # 682 - fix bci copyriht checking
 * 	[1993/09/24  18:56:16  marty]
 * 
 * Revision 1.1.6.5  1993/09/24  17:23:13  damon
 * 	CR 687. Fixed COPYRIGHT NOTICE checking
 * 	[1993/09/24  17:15:10  damon]
 * 
 * Revision 1.1.6.4  1993/09/16  17:19:10  damon
 * 	Added COPYRIGHT NOTICE handling
 * 	[1993/09/16  17:17:58  damon]
 * 
 * Revision 1.1.6.3  1993/09/02  20:12:30  marty
 * 	CR # 640 - The comments after the HISTORY marker are now extracted during check in.
 * 	[1993/09/02  20:12:12  marty]
 * 
 * Revision 1.1.6.2  1993/08/31  18:15:50  damon
 * 	CR 636. call okmesg more intelligently
 * 	[1993/08/31  18:14:07  damon]
 * 
 * Revision 1.1.6.1  1993/08/25  16:12:59  damon
 * 	CR 622. added const
 * 	[1993/08/25  16:12:47  damon]
 * 
 * Revision 1.1.4.2  1993/08/19  18:35:14  damon
 * 	CR 622. Added typecast
 * 	[1993/08/19  18:29:37  damon]
 * 
 * Revision 1.1.4.1  1993/08/10  21:42:04  marty
 * 	Check for existence of IGNORE_COPYRIGHT variable before
 * 	checking copyright marker.
 * 	[1993/08/10  21:38:31  marty]
 * 
 * Revision 1.1.2.27  1993/06/02  22:49:26  damon
 * 	CR 564. Fixed check for end of string in hst_getline()
 * 	[1993/06/02  22:49:00  damon]
 * 
 * Revision 1.1.2.26  1993/06/01  16:37:23  marty
 * 	CR # 558 - Get it building on rios_aix
 * 	[1993/06/01  16:37:09  marty]
 * 
 * Revision 1.1.2.25  1993/05/19  18:29:11  marty
 * 	CR # 496 - Change hst_getline() to check for comment leaders that end in " ".
 * 	On a line with no comments (just a comment leader); rcs will strip
 * 	trailing spaces.
 * 	[1993/05/19  18:28:58  marty]
 * 
 * Revision 1.1.2.24  1993/05/04  19:46:04  damon
 * 	CR 486. Added size parameter to oxm_gets
 * 	[1993/05/04  19:45:49  damon]
 * 
 * Revision 1.1.2.23  1993/04/30  17:29:38  damon
 * 	CR 463. Fixed variable shadowing problem
 * 	[1993/04/30  17:29:27  damon]
 * 
 * Revision 1.1.2.22  1993/04/28  20:48:06  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  20:44:20  damon]
 * 
 * Revision 1.1.2.21  1993/04/27  15:39:37  damon
 * 	CR 463. First round of -pedantic changes
 * 	[1993/04/27  15:39:16  damon]
 * 
 * Revision 1.1.2.20  1993/04/26  15:17:58  damon
 * 	CR 436. Added NULL to av list for create_leaderless_log
 * 	[1993/04/26  15:17:50  damon]
 * 
 * Revision 1.1.2.19  1993/04/22  15:23:07  marty
 * 	Cleanup memory leaks
 * 	[1993/04/22  15:22:10  marty]
 * 
 * Revision 1.1.2.18  1993/04/16  21:08:54  damon
 * 	CR 463. Fixed unterminated strings
 * 	[1993/04/16  21:08:33  damon]
 * 
 * Revision 1.1.2.17  1993/04/10  19:19:11  marty
 * 	Add include of stdio.h
 * 	[1993/04/10  19:17:41  marty]
 * 
 * Revision 1.1.2.16  1993/04/09  17:15:48  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:15:03  damon]
 * 
 * Revision 1.1.2.15  1993/04/08  18:42:50  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  18:41:47  damon]
 * 
 * Revision 1.1.2.14  1993/04/06  23:48:44  marty
 * 	Clean up hst_*() functions to free up unneeded allocated space.
 * 	[1993/04/06  23:43:45  marty]
 * 
 * Revision 1.1.2.13  1993/03/17  20:42:14  damon
 * 	CR 446. Fixed include files/ forward decls.
 * 	[1993/03/17  20:41:28  damon]
 * 
 * Revision 1.1.2.12  1993/03/15  18:38:39  damon
 * 	CR 436. Removed unused code
 * 	[1993/03/15  18:38:29  damon]
 * 
 * Revision 1.1.2.11  1993/03/15  15:50:30  marty
 * 	Change hst_* routines to better handle file with non existent
 * 	history sections.
 * 	[1993/03/15  15:50:11  marty]
 * 
 * Revision 1.1.2.10  1993/03/05  16:03:42  marty
 * 	Fix code to handle multy line comments properly.
 * 	[1993/03/05  16:01:41  marty]
 * 
 * Revision 1.1.2.9  1993/03/04  20:32:17  marty
 * 	Added some history manipulation routines that work with
 * 	the existing routines but hopefully (over time) will
 * 	be expanded to replace the old routines.  All processing
 * 	of history is done in memeory instead of log files.
 * 	[1993/03/04  20:30:47  marty]
 * 
 * Revision 1.1.2.8  1993/02/06  21:20:00  damon
 * 	CR 396. Changed == to = in sci_xtract_log
 * 	[1993/02/06  21:19:44  damon]
 * 
 * Revision 1.1.2.7  1993/01/26  16:34:35  damon
 * 	CR 396. Conversion to err_log
 * 	[1993/01/26  16:33:57  damon]
 * 
 * Revision 1.1.2.6  1993/01/25  21:28:15  damon
 * 	CR 396. Converted history.c to err_log
 * 	[1993/01/25  21:26:47  damon]
 * 
 * Revision 1.1.2.5  1993/01/20  22:21:37  damon
 * 	CR 376. Moved more code out from sci.c
 * 	[1993/01/20  22:16:22  damon]
 * 
 * Revision 1.1.2.4  1993/01/18  20:35:07  damon
 * 	CR 197. Removed unecessary check for BIN/NONE
 * 	[1993/01/18  20:34:23  damon]
 * 
 * Revision 1.1.2.3  1993/01/15  16:14:19  damon
 * 	CR 376. Moved code from sci_rcs.c
 * 	[1993/01/15  16:11:42  damon]
 * 
 * Revision 1.1.2.2  1993/01/15  14:39:17  damon
 * 	CR 376. Moved code from sci_rcs.c
 * 	[1993/01/15  14:38:55  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)86  1.1  src/bldenv/sbtools/libode/history.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:13";
#endif /* not lint */

#include <ctype.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ode/copyrights.h>
#include <ode/errno.h>
#include <ode/history.h>
#include <ode/interface.h>
#include <ode/misc.h>
#include <ode/parse_rc_file.h>
#include <ode/odexm.h>
#include <ode/odedefs.h>
#include <ode/sci.h>
#include <ode/public/error.h>
#include <ode/public/odexm_client.h>
#include <ode/src_ctl_rcs.h>
#include <ode/util.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdio.h>

#define LOGPROMPT       "<<<replace with log message"
#define LOGMSGSIZE      4096    /* -m<msg> buffer size */
#define IBR_MARKER      "*** Initial Branch Revision ***"
#define MISSING         1       /* positive return value - string not found */
/* comment syntax for C files */
#define BEGINCMT        "/*"
#define CONTCMT         " * "
#define ENDCMT          "*/"
#define COPYRIGHT_BEGIN	"COPYRIGHT NOTICE"
#define CR              "COPYRIGHT"
#define HIST            "HISTORY"
#define BEGINLOG        "$Log"
#define ENDLOG          "$EndLog$"

#define ATOI(n, p) \
    (n) = 0; \
    if ('0' > *(p) || *(p) > '9') \
        return(1); \
    while ('0' <= *(p) && *(p) <= '9') { \
        (n) = (n) * 10 + (*(p) - '0'); \
        (p)++; \
    }

extern OXM_CNXTN rcs_monitor;
extern char working_file_tail[];
extern char canon_working_file [];
extern char temp_working_file [];
extern char temp1[], temp2[], temp3[];
extern char mesgfile [];
extern char * BCSSET_NAME;
extern char bcslog [];

struct logmsg {
    struct logmsg *next;
    char *revhdr;
    short nlines;
    short maxlines;
    char **body;
    char *stamp;
    int seenlog;
};

/*
 * Prototypes
 */
void
hst_add_comment( HIST_ELEM node, char * text );
void
hst_mark_common_entries ( HIST_ELEM ver_ancestor, HIST_ELEM check );
void
hst_dump_list_fd ( HIST_ELEM history, FILE *fd );
void
hst_get_date ( char * date, char * ltime );
struct comment_elem *
new_comment ( char * line );
char *
hst_getline ( char **text, char *cmt_leader );
ERR_LOG
addtobody( struct logmsg *lm, const char *buffer );

STATIC
char *months[] = {
    (char *)"Jan",    (char *)"Feb",    (char *)"Mar",    (char *)"Apr",    (char *)"May",    (char *)"Jun",
    (char *)"Jul",    (char *)"Aug",    (char *)"Sep",    (char *)"Oct",    (char *)"Nov",    (char *)"Dec",
    NULL
};

void month_num( char *mptr, char *bptr )
{
    char **mp, *p;

    for (mp = months; (p = *mp) != NULL; mp++)
        if (*p == *mptr && *(p+1) == *(mptr+1) && *(p+2) == *(mptr+2))
            break;
    if (p == NULL) {
        *bptr++ = '?';
        *bptr = '?';
    } else {
        *bptr++ = ((mp-months)+1)/10 + '0';
        *bptr = ((mp-months)+1)%10 + '0';
    }
} /* end month_num */

BOOLEAN is_whist_header( char *buf )
{
    char tmpbuf[MAXPATHLEN];
    char *bp, *p, *q;
    int i;

    q = buf + 1;
    while ((q = strchr(q + 1, '-')) != NULL) {
        if (*(q + 4) != '-' || *(q + 7) != ' ' || *(q + 8) != ' ' ||
            (p = strstr(q + 9, ") at ")) == NULL)
            continue;
        p--;
        while (*p != '(' && p > q)
            p--;
        if (*p++ != '(')
            continue;
        q -= 2;
        bp = tmpbuf;
        *bp++ = *(q+7);
        *bp++ = *(q+8);
        *bp++ = '/';
        month_num(q+3, bp);
        bp += 2;
        *bp++ = '/';
        *bp++ = (*q == ' ') ? '0' : *q;
        *bp++ = *(q+1);
        for (i = 0; i < 12; i++)
            *bp++ = ' ';
        while (*p != ')')
            *bp++ = *p++;
        *bp++ = '\0';
        (void) memcpy(buf, tmpbuf, bp - tmpbuf);
        return( TRUE );
    }
    return( FALSE );
}

struct logmsg *
logalloc( struct logmsg *logtemp, ERR_LOG * log )
{
    struct logmsg *newlog;

    newlog = (struct logmsg *)calloc(1, sizeof(struct logmsg));
    if (newlog == NULL) {
      *log = err_log ( OE_ALLOC );
      return ( NULL );
    }
    newlog->revhdr = logtemp->revhdr;
    newlog->nlines = logtemp->nlines;
    newlog->maxlines = logtemp->maxlines;
    newlog->body = logtemp->body;
    newlog->stamp = logtemp->stamp;
    newlog->seenlog = logtemp->seenlog;
    memset((char *)logtemp, 0, sizeof(struct logmsg));
    *log = OE_OK;
    return(newlog);
}

void logfree ( struct logmsg *logptr )
{
    int i;

    if (logptr == NULL) {
      return; 
    }
    logfree(logptr->next);
    if (logptr->revhdr != NULL)
    	free(logptr->revhdr);
    for (i = 0; i < logptr->nlines ; i++) {
    	free(logptr->body[i]); 
    }
    if (logptr->stamp != NULL)
    	free(logptr->stamp);
    free(logptr);
}

ERR_LOG
savelog( struct logmsg **headptr,
         struct logmsg **tailptr,
         struct logmsg *logtemp )
{
  ERR_LOG log;

  if (*headptr == NULL) {
    *headptr = logalloc(logtemp, &log);
    if ( log != OE_OK ) {
      return( log );
    }
    *tailptr = *headptr;
    return( OE_OK );
  }
  (*tailptr)->next = logalloc(logtemp, &log);
  if ( log != OE_OK ) {
    return( log );
  }
  *tailptr = (*tailptr)->next;
  return( OE_OK );
}

ERR_LOG
addtobody( struct logmsg *lm, const char *buffer )
{
    char *p;
    int l;

    if (lm->nlines >= lm->maxlines) {
        if (lm->maxlines == 0) {
            lm->maxlines = 4;
            lm->body = (char **) malloc((size_t)(lm->maxlines * sizeof(char *)));
        } else {
            lm->maxlines += 4;
            lm->body =  (char **) realloc(lm->body,
                                          lm->maxlines * sizeof(char *));
        }
        if (lm->body == NULL) {
            return( err_log ( OE_ALLOC ) );
        }
    }
    if ((p = strchr(buffer, '\n')) == NULL)
        p = (char *)buffer + strlen(buffer);
    while (p > (char *)buffer && (*(p-1) == ' ' || *(p-1) == '\t'))
        p--;
    if (p == (char *)buffer)
        p = NULL;
    else {
        l = p - buffer;
        if ((p = (char *) malloc((size_t) (l + 1))) == NULL) {
            return( err_log ( OE_ALLOC ) );
        }
        memcpy(p, buffer, l);
        p[l] = '\0';
    }
    lm->body[lm->nlines++] = p;
    return( OE_OK );
}

ERR_LOG
remove_ibr_markers( struct logmsg *lm )
{
    char *p, *q;
    char **body = lm->body;
    int nlines = lm->nlines;
    int i, j, k;

    i = 0;
    for (;;) {
        if (i == nlines)
            break;
        if (body[i] == NULL) {
            i++;
            continue;
        }
        p = strstr(body[i], IBR_MARKER);
        if (p == NULL) {
            i++;
            continue;
        }
        q = p + sizeof(IBR_MARKER) - 1;
        while (p > body[i] && (*(p-1) == ' ' || *(p-1) == '\t'))
            p--;
        if (p > body[i]) {
            i++;
            continue;
        }
        while (*q == ' ' || *q == '\t')
            q++;
        if (*q != '\0') {
            i++;
            continue;
        }
        j = i + 1;
        if (j == nlines) {
            break;
        }
        if ((q = body[j]) != NULL && *q == '[') {
            q += strlen(q) - 1;
            if (*q != ']') {
                return( err_log ( OE_HASIBR, IBR_MARKER ) );
            }
            q = NULL;
        }
        while (q == NULL) {
            j++;
            if (j == nlines)
                break;
            q = body[j];
        }
        if (j == nlines) {
            break;
        }
        k = i;
        while (j < nlines) {
            if (body[k] != NULL)
                (void) free(body[k]);
            body[k++] = body[j];
            body[j++] = NULL;
        }
        nlines = k;
    }
    while (i > 0 && body[i-1] == NULL)
        i--;
    lm->nlines = i;
    return( OE_OK );
}

ERR_LOG
save_stamp_match( struct logmsg *lm, char *ptr )
{
    char *tptr;

    while (*ptr == ' ')
        ptr++;
    if (*(ptr+4) != '/' || *(ptr+7) != '/' || *(ptr+10) != ' ')
        return( OE_OK );
    tptr = ptr;
    ptr += 10;
    while (*ptr == ' ')
        ptr++;
    if (*(ptr+2) != ':' || *(ptr+5) != ':' || *(ptr+8) != ' ')
        return( OE_OK );
    lm->stamp = strdup(tptr);
    if (lm->stamp == NULL) {
        ui_print ( VFATAL, "Stamp strdup failed\n");
        return( err_log ( OE_ALLOC ) );
    }
    return( OE_OK );
}

BOOLEAN check_branches( struct logmsg *lm, char **branches, int nrev,
                        ERR_LOG * log )
{
    int i;
    char *ptr;
    char *bptr;

    *log = OE_OK;
    if (branches) {
        for (i = 0; branches[i] != NULL; i++) {
            ptr = lm->revhdr;
            bptr = branches[i];
            while (*bptr != '\0' && *ptr == *bptr) {
                ptr++;
                bptr++;
            }
            if (*bptr != '\0')
                continue;
            while (*ptr >= '0' && *ptr <= '9')
                ptr++;
            if (*ptr != ' ')

                continue;
            bptr = lm->body[lm->nlines-1];
            if (*bptr != '[')
                bptr = NULL;
            else {
                bptr += strlen(bptr) - 1;
                if (*bptr != ']')
                    bptr = NULL;
            }
            if (bptr == NULL && save_stamp_match(lm, ptr) != OE_OK ) {
              *log = err_log ( OE_INTERNAL );
              return( FALSE ); /* The return value here is superfluous */
            }
            return( FALSE );
        }
    }
    if (nrev == 0) {
        return( TRUE );
    }
    i = 0;
    ptr = lm->revhdr;
    for (;;) {
        while (*ptr >= '0' && *ptr <= '9')
            ptr++;
        i++;
        if (*ptr != '.')

            break;
        ptr++;
    }
    if (*ptr != ' ' || i <= nrev)
        return( TRUE );
    bptr = lm->body[lm->nlines-1];
    if (*bptr != '[')
        bptr = NULL;
    else {
        bptr += strlen(bptr) - 1;
        if (*bptr != ']')
            bptr = NULL;
    }
    if (bptr == NULL && save_stamp_match(lm, ptr) != OE_OK) {
        *log = err_log ( OE_INTERNAL );
        return( FALSE ); /* The return value here is superfluous */
    }
    return( TRUE );
}

ERR_LOG
process_log_messages( struct logmsg *loghead, char *leader, FILE *outf,
                      FILE *mesgf, char **branches, int nrev )
{
    char *p;
    struct logmsg *lm;
    FILE *fp;
    int i;
    int status;
    int donemsg = FALSE;
    ERR_LOG log;

  enter ( "process_log_messages" );
    for (lm = loghead; lm != NULL; lm = lm->next) {
        if (lm->nlines == 0) {
            ui_print ( VDETAIL, "Removed empty RCS log message\n");
            continue;
        }
        if ( ( log = remove_ibr_markers(lm) ) != OE_OK ) {
          leave ( );
          return( log );
        } /* end if */
        if (lm->nlines == 0) {
            ui_print ( VDETAIL, "Removed empty RCS log message\n");

            continue;
        }
        if (lm->seenlog == FALSE) {
            fp = mesgf;
        } else if (lm->revhdr == NULL) {
            fp = outf;
        } else {
            status = check_branches(lm, branches, nrev, &log);
            if ( log != OE_OK ) {
              leave ( );
              return( log );
            }
            if (status == 0) {
                fp = mesgf;
            } else {
                fp = outf;
            }
        }
        if (fp == mesgf) {
            if (lm->stamp == NULL && lm->revhdr != NULL) {
                p = lm->body[lm->nlines-1];
                if (*p != '[')
                    p = NULL;
                else {
                    p += strlen(p) - 1;
                    if (*p != ']')
                        p = NULL;
                }
                if (p == NULL) {
                    p = lm->revhdr;
                    while (*p != ' ')
                        p++;
                    if ( (log = save_stamp_match(lm, p) ) != OE_OK) {
                      leave ( );
                      return( log );
                    }
                }
            }
            if (donemsg)
                fprintf(fp, "\n");
            for (i = 0; i < lm->nlines; i++) {
                p = lm->body[i];
                if (p == NULL)
                    fprintf(fp, "\n");
                else
                    fprintf(fp, "%s\n", p);
            }
            if (lm->stamp != NULL) {
                fprintf(fp, "[%s]\n", lm->stamp);
            }
            donemsg = TRUE;
            continue;
        }
        fprintf(fp, "%sRevision ", leader);
        if (lm->revhdr == NULL) {
            if (lm->stamp == NULL)
                fprintf(fp, "0.0  00/00/00  00:00:00  unknown\n");
            else
                fprintf(fp, "0.0  %s\n", lm->stamp);
        } else
            fprintf(fp, "%s\n", lm->revhdr);
        for (i = 0; i < lm->nlines; i++) {
            p = lm->body[i];
            if (p == NULL)
                fprintf(fp, "%s\n", leader);
            else
                fprintf(fp, "%s\t%s\n", leader, p);
        }
        if (lm->stamp != NULL)
            fprintf(fp, "%s\t[%s]\n", leader, lm->stamp);
        fprintf(fp, "%s\n", leader);
    }
    leave ( );
    return( OE_OK );
}

ERR_LOG
src_ctl_extract_history( FILE *mesgf, char *leader, char **branches, char *rev )
{
    char newtemp[MAXPATHLEN];
    char buffer[MAXPATHLEN];
    int havestamp;
    int owenewline;
    char *ptr;
    char *bufptr;
    int leaderlen=0, wsleaderlen;
    int i;
    int seenhdr;
    int wanthdr;
    int iswhist;
    FILE *inf, *outf;
    int seenmarker;
    struct logmsg *loghead;
    struct logmsg *logtail;
    struct logmsg logtemp;
    int nrev;
    int no_ws_leaderlen;
    ERR_LOG log = OE_OK;

    enter ( "src_ctl_extract_history" );
    if (rev == NULL)
        nrev = 0;
    else {
        nrev = numfields(rev, &log);
        if ( log != OE_OK ) {
            leave ( );
            return( log );
        }
    }
    if (leader == '\0') {
        leave ( );
        return( err_log ( OE_NOLEADER ) );
    }
    if (branches)
            ui_print ( VDETAIL, "Scanning for HISTORY and $Log...$ messages\n");
        else
            ui_print ( VDETAIL, "Scanning for HISTORY\n" );
    for (ptr = leader; *ptr != '\0'; ptr++)
        if (*ptr != ' ' && *ptr != '\t')
            leaderlen = (ptr - leader) + 1;
    wsleaderlen = ptr - leader;
    no_ws_leaderlen = wsleaderlen - 1;
    havestamp = FALSE;
    owenewline = FALSE;
    seenmarker = FALSE;
    loghead = NULL;
    logtail = NULL;
    memset((char *)&logtemp, 0, sizeof(logtemp));
    if ((inf = fopen(temp_working_file, "r")) == NULL) {
        leave ( );
        return( err_log ( OE_OPEN, temp_working_file ) );
    }
    (void) concat(newtemp, sizeof(newtemp), temp_working_file, ".new", NULL);
    (void) unlink(newtemp);
    if ((outf = fopen(newtemp, "w")) == NULL) {
        (void) fclose(inf);
        (void) unlink(newtemp);
        leave ( );
        return( err_log ( OE_OPEN, newtemp) );
    }
    while ((bufptr = fgets(buffer, sizeof(buffer), inf)) != NULL) {
        (void) fputs(bufptr, outf);
        if (strstr(bufptr, "HISTORY") != NULL) {
            seenmarker = TRUE;
            break;
        }
    }
    if (!seenmarker) {
        (void) fclose(inf);
        (void) fclose(outf);
        (void) unlink(newtemp);
        leave ( );
        return( err_log ( OE_HISTORY ) );
    }
    seenmarker = FALSE;
    while ((bufptr = fgets(buffer, sizeof(buffer), inf)) != NULL) {
        if ((ptr = strstr(bufptr, "$Log")) != NULL &&
            (*(ptr+4) == '$' || *(ptr+4) == ':')) {
            (void) fputs(bufptr, outf);
            seenmarker = TRUE;
            break;
        }
        if ( is_whist_header(bufptr)) {
            owenewline = FALSE;
            if (havestamp) {
                if ( (log = savelog(&loghead, &logtail, &logtemp) ) != OE_OK )
                    break;
            }
            logtemp.stamp = strdup(bufptr);
            if (logtemp.stamp == NULL) {
                log = err_log ( OE_ALLOC );
                break;
            }
            havestamp = TRUE;
            continue;
        }
        if (strncmp(bufptr, leader, leaderlen) != 0)
            break;
        bufptr += leaderlen;
        for (i = leaderlen; i < wsleaderlen; i++) {
            if (*bufptr != leader[i])
                break;
            bufptr++;
        }
        for (ptr = bufptr; *ptr != '\0' && *ptr != '\n'; ptr++)
            if (*ptr != ' ' && *ptr != '\t')
                break;
        if (*ptr == '\0' || *ptr == '\n') {
            owenewline = TRUE;
            continue;
        }
        if (*bufptr == '\t')
            bufptr++;
        if (owenewline) {
            if ( ( log = addtobody(&logtemp, "\n") ) != OE_OK )
                break;
            owenewline = FALSE;
        }
        if ( ( log = addtobody(&logtemp, bufptr) ) != OE_OK )
            break;
        havestamp = TRUE;
    }
    if ( log != OE_OK )
      return (log);
    if (!seenmarker) {
        (void) fclose(inf);
        (void) fclose(outf);
        (void) unlink(newtemp);
        leave ( );
        return( err_log ( OE_LOG ) );
    }
    seenmarker = FALSE;
    seenhdr = FALSE;
    wanthdr = TRUE;
    iswhist = FALSE;
    if (!havestamp)
        logtemp.seenlog = TRUE;
    while ((bufptr = fgets(buffer, sizeof(buffer), inf)) != NULL) {
        if (strncmp(bufptr, leader, wsleaderlen) == 0 &&
            *(bufptr + wsleaderlen) == '$' &&
            *(bufptr + wsleaderlen + 7) == '$' &&
            strncmp(bufptr + wsleaderlen + 1, "EndLog", 6) == 0) {
            seenmarker = TRUE;
            break;
        }
        if (wanthdr) {
            if (!iswhist && strncmp(bufptr, leader, wsleaderlen) == 0 &&
                strncmp(bufptr + wsleaderlen, "Revision ", 9) == 0) {
                bufptr += wsleaderlen + 9;
                if (havestamp) {
                    if ( (log = savelog(&loghead, &logtail, &logtemp) )
                          != OE_OK ) {
                        break;
                    }
                    logtemp.seenlog = TRUE;
                }
                ptr = bufptr + strlen(bufptr) - 1;
                if (*ptr == '\n')
                    *ptr = '\0';
                logtemp.revhdr = strdup(bufptr);
                if (logtemp.revhdr == NULL) {
                    log = err_log ( OE_ALLOC );
                    break;
                }
                havestamp = TRUE;
                seenhdr = TRUE;
                wanthdr = FALSE;
                continue;
            }
            if (is_whist_header(bufptr)) {
                if (havestamp) {
                    if ( (log = savelog(&loghead, &logtail, &logtemp) )
                          != OE_OK ) {
                        break;
                    }
                    logtemp.seenlog = TRUE;
                }
                logtemp.stamp = strdup(bufptr);
                if (logtemp.stamp == NULL) {
                    log = err_log ( OE_ALLOC );
                    break;
                }
                havestamp = TRUE;
                seenhdr = TRUE;
                wanthdr = FALSE;
                iswhist = TRUE;
                continue;
            }
        }
        if (!seenhdr) {
            break;
        }
        if (!iswhist) {
            if (strncmp(bufptr, leader, no_ws_leaderlen ) != 0) {
                break;
            }
            bufptr += wsleaderlen;
        } else {
            if (strncmp(bufptr, leader, no_ws_leaderlen) != 0) {
                break;
            }
            bufptr += wsleaderlen;
            if (strchr(" \t\n", *bufptr) == NULL) {
                break;
            }
            for (i = leaderlen; i < wsleaderlen; i++) {
                if (*bufptr != leader[i])
                    break;
                bufptr++;
            }
        }
        if (*bufptr == '\t')
            bufptr++;
        for (ptr = bufptr; *ptr != '\0' && *ptr != '\n'; ptr++)
            if (*ptr != ' ' && *ptr != '\t')
                break;
        if (*ptr == '\0' || *ptr == '\n') {
            if (!wanthdr) {
                wanthdr = TRUE;
            }
            continue;
        }
        if (wanthdr && ( log = addtobody(&logtemp, "\n") ) != OE_OK ) {
            break;
        }
        if ( (log = addtobody(&logtemp, bufptr) ) != OE_OK ) {
            break;
        }
        if (wanthdr)
            wanthdr = FALSE;
    }
    if ( log != OE_OK )
      return (log);
    if (!seenmarker ||
        (havestamp && ( log = savelog(&loghead, &logtail, &logtemp))
        != OE_OK) ||
        ( log = process_log_messages(loghead, leader, outf, mesgf,
        branches, nrev)) != OE_OK )
    {
        (void) fclose(inf);
        (void) fclose(outf);
        (void) unlink(newtemp);
        leave ( );
        logfree(loghead);
        return( log );
    }
            (void) fputs(bufptr, outf);
    (void) ffilecopy(inf, outf);
  if (ferror(inf) || fclose(inf) == EOF) {
        (void) fclose(inf);
        (void) fclose(outf);
        (void) unlink(newtemp);
        leave ( );
        logfree(loghead);
        return( err_log ( OE_READ, temp_working_file ) );
  }
  if (ferror(outf) || fclose(outf) == EOF) {
        (void) fclose(outf);
        (void) unlink(newtemp);
        leave ( );
        logfree(loghead);
        return( err_log ( OE_WRITE, newtemp ) );
  }
  if (rename(newtemp, temp_working_file) < 0) {
    (void) fclose(inf);
    (void) unlink(newtemp);
    leave ( );
    logfree(loghead);
    return( err_log ( OE_RENAME, newtemp, temp_working_file ) );
  }
  logfree(loghead);
  leave ( );
  return( OE_OK );
}

/*
 * This function currently only handles one revision in xlog as opposed
 * to the multiple ones in the previous version ( from if_rcs.c ).
 */
ERR_LOG
hst_lookup_logmsg ( char *rev, char *leader,
                    BOOLEAN condense, char *xlog, int nxlog )
{
    char buf[MAXPATHLEN];
    char brbuf[MAXPATHLEN];
    char **branches;
    int nbr;
    char xlogrev[MAXPATHLEN];
    FILE *mesgf;
    char *ptr, *bptr;
    int i;
    ERR_LOG log;

  enter ( "hst_lookup_logmsg" );
      rev = NULL;
    if ( !condense )
      branches = NULL;
    else {
      nbr = 0;
      bptr = brbuf;
      branches = (char **)malloc((size_t)((nxlog + 1) * sizeof(char *)));
      if (branches == NULL) {
        leave ( );
        log_error ();
        return ( err_log ( OE_ALLOC ) );
      }
      branches[nxlog] = NULL;
      ptr = xlogrev;
      for (i = 0; i < nxlog; i++) {

           /*
            * Strip the last number from xlog, giving a three number
            * branch designator.
            */
            strcpy ( buf, xlog );
            if (*buf != NUL) {
                ptr = buf + strlen(buf) - 1;
                while (*ptr >= '0' && *ptr <= '9')
                    ptr--;
                if (*ptr++ == '.')
                    *ptr = NUL;
                else
                    *buf = NUL;
            } /* if */

            ptr = concat(bptr, brbuf+sizeof(brbuf)-bptr,
                         buf, NULL);
            if (ptr == NULL) {
                ui_print ( VWARN, "Too many branches - '%s' ignored.\n", xlog);
                bptr = ptr;
            } else {
                branches[nbr++] = bptr;
                bptr = ++ptr;
            } /* if */
      } /* for */
  } /* if */
  if ((mesgf = fopen(mesgfile, "w")) == NULL) {
      (void) unlink(mesgfile);
      leave ( );
      log_error ();
      free(branches);
      return ( err_log ( OE_OPEN, mesgfile ) );
  }
  log = src_ctl_extract_history(mesgf, leader, branches, rev);
  if (ferror(mesgf) || fclose(mesgf) == EOF) {
      (void) fclose(mesgf);
      (void) unlink(mesgfile);
      leave ( );
      log_error ();
      free(branches);
      return ( err_log ( OE_WRITE, mesgfile ) );
  }
  leave ( );
      free(branches);
  return( log );
}

ERR_LOG
save_log_message( char *mfile )
{
  FILE *inf;
  FILE *outf;

  ui_print ( VDEBUG, "Entering save_log_message\n" );
  ui_print ( VDETAIL, "Updating ./.BCSlog-%s", BCSSET_NAME);
  if ((inf = fopen(mfile, "r")) == NULL) {
    return( err_log ( OE_OPEN, mfile ) );
  }
  if ((outf = fopen(bcslog, "a")) == NULL) {
    (void) fclose(inf);
    return( err_log ( OE_OPEN, bcslog ) );
  }
  fprintf(outf, "[ %s ]\n", canon_working_file);
  (void) ffilecopy(inf, outf);
  if (ferror(inf) || fclose(inf) == EOF) {
    (void) fclose(inf);
    (void) fclose(outf);

    return( err_log ( OE_READ, mfile ) );
  }
  if (ferror(outf) || fclose(outf) == EOF) {
    (void) fclose(outf);
    return( err_log ( OE_WRITE, bcslog ) );
  }
  ui_print ( VDEBUG, "Leaving save_log_message\n" );
  return( OE_OK );
} /* end save_log_message */



/*
 * check contents of log message file, must not contain end-of-comment,
 * must differ from default prompt and can't exceed length limit
 * returns: OK - valid log message
 *          ERROR - on error
 */
ERR_LOG
okmesg( char *leader, char *logbuf)
{
    char buf[MAXPATHLEN];
    FILE *inf;
    int cfile;
    char *ptr;

    enter ( "okmesg" );
    ptr = logbuf;
    if ((inf = fopen(mesgfile, "r")) == NULL) {
        leave ( );
        return ( err_log ( OE_OPEN, mesgfile ) );
    }
    cfile = (strcmp(leader, CONTCMT) == 0);
    while (fgets(buf, sizeof(buf), inf) != NULL) {
        if (strstr(buf, LOGPROMPT) != NULL) {
            (void) fclose(inf);
            leave ( );
            return ( err_log ( OE_EMPTYLOG ) );
        }
        if (cfile && strstr(buf, ENDCMT) != NULL) {
            (void) fclose(inf);
            leave ( );
            return ( err_log ( OE_CMTINLOG ) );
        }
        if (*buf != NUL && *buf != '\n')
            *ptr++ = '\t';
        ptr = concat(ptr, logbuf+LOGMSGSIZE-ptr-1, buf, NULL);
        if (ptr == NULL)
            break;
    }
    if (ferror(inf) || fclose(inf) == EOF) {
        (void) fclose(inf);
        leave ( );
        return ( err_log ( OE_READ, mesgfile ) );
    }
    leave ( );
    return ( OE_OK );
}

ERR_LOG
reinsert_log_messages( char *mfile )
{
  FILE *inf, *outf, *mesgf;
  int status;
  char buffer[MAXPATHLEN];
  char *ptr;

  if ((inf = fopen(temp_working_file, "r")) == NULL) {
    return( err_log ( OE_OPEN, temp_working_file ) );
  }
  if ((mesgf = fopen(mfile, "r")) == NULL) {
    (void) fclose(inf);
    return( err_log ( OE_OPEN, mfile ) );
  }
  (void) unlink(temp1);
  if ((outf = fopen(temp1, "w")) == NULL) {
    (void) fclose(inf);
    (void) fclose(mesgf);
    (void) unlink(temp1);
    return( err_log ( OE_OPEN, temp1 ) );
  }
  status = ERROR;
  while ((fgets(buffer, sizeof(buffer), inf)) != NULL) {
      (void) fputs(buffer, outf);
      if ((ptr = strstr(buffer, "$Log")) != NULL &&
          (*(ptr+4) == '$' || *(ptr+4) == ':')) {
          (void) ffilecopy(mesgf, outf);
          (void) ffilecopy(inf, outf);
          status = OK;
          break;
      }
  }
  if (ferror(inf) || fclose(inf) == EOF) {
      (void) fclose(inf);
      (void) fclose(outf);
      (void) fclose(mesgf);
      (void) unlink(temp1);
      return( err_log ( OE_READ, temp_working_file ) );
  }
  if (ferror(mesgf) || fclose(mesgf) == EOF) {
      (void) fclose(outf);
      (void) fclose(mesgf);
      (void) unlink(temp1);
      return( err_log ( OE_READ, mfile ) );
  }
  if (ferror(outf) || fclose(outf) == EOF) {
    (void) fclose(outf);
    (void) unlink(temp1);
    return( err_log ( OE_WRITE, temp1 ) );
  }
  if (status == ERROR) {
    (void) unlink(temp1);
    return( err_log ( OE_HISTORY ) );
  }
  if (rename(temp1, temp_working_file) < 0) {
    (void) unlink(temp1);
    return( err_log ( OE_RENAME, temp1, temp_working_file ) );
  }
  (void) unlink(mfile);
  return( OE_OK );
}

int buf_lines;

/*
 * get a line from the file pointed to by fp
 * (or reuse the line previously read)
 * side-effect: increments global buf_lines
 */
STATIC
char *
getline( char *buf, int bufsiz, FILE *fp, int sameline, ERR_LOG *log )
{
  if (!sameline) {
    if (fgets(buf, bufsiz, fp) == NULL) {
      ui_print(VDEBUG, "reached EOF reading input file\n");
      *log = err_log ( OE_EOF );
      return NULL;        /* EOF or error reading */
    }
    ++buf_lines;
  }
  *log = OE_OK;
  return buf;
} /* getline */


/*
 * Search for a particular string and optionally check for good leaders on
 * each line as you go.
 *
 * modes: 1 - require good leader
 *        2 - good leader not required
 *        3 - only check this line
 *
 * returns: OK - if string is found
 *          MISSING - if not found
 *          EOF - on end of file
 */
STATIC
int search_for(
  char  * buf,          /* buffer for getline */
  int     size,         /* length of buffer */
  FILE  * inf,          /* file ptr from which to read */
  const char  * search, /* string to look for */
  int     mode,                   /* flag: good leader required on each line */
  const char  * leader, /* comment leader */
  int     lead_len,     /* length of comment leader */
  int   * good_leader,                 /* does the string have a good leader */

  char ** ptr,          /* pointer returned by strstr() */
  int     reuse,        /* use existing line already in buffer */
  ERR_LOG *log )
{
  char * res;

  enter ( "search for" );
  *good_leader = FALSE;
  res = NULL;
  switch(mode) {
  case 1:
   /*
    *  Good leader required
    */
    while ((res = getline(buf, size, inf, reuse, log)) != NULL) {
      reuse = FALSE;
      *good_leader = (strncmp(buf, leader, lead_len) == 0);
      if (*good_leader) {
        if ((*ptr = strstr(buf, search)) != NULL) {
          leave ( );
          return (OK);  /* found match */
        } /* endif */
      } else {
        leave ( );
        return (MISSING);
      } /* endif */
    } /* endwhile */
    break;

   /*
    *  Good leader not required
    */
  case 2:
    while ((res = getline(buf, size, inf, reuse, log)) != NULL) {
      reuse = FALSE;
      if ((*ptr = strstr(buf, search)) != NULL) {
        *good_leader = (strncmp(buf, leader, lead_len) == 0);
        leave ( );
        return (OK);    /* strings matched */
      } /* endif */
    }/* endwhile */
    break;
   /*
    * Look on immediate line only
    */
  case 3:
    if ((res = getline(buf, size, inf, reuse, log)) != NULL) {
      reuse = FALSE;
      *good_leader = (strncmp(buf, leader, lead_len) == 0);
      if ((*ptr = strstr(buf, search)) != NULL) {
        leave ( );
        return (OK);    /* found match */
      } else {
        leave ( );
        return (MISSING);
      } /* if */
    } else {
      leave ( );
      return (EOF);
    } /* endif */
  } /* endswitch */

  if (res == NULL) {
    leave ( );
    return (EOF);
  } else {
    leave ( );
    return (MISSING);
  } /* endif */
} /* end search_for */

extern BOOLEAN check_copyrights;

/*
 * look for copyright marker, HISTORY, $Log and $EndLog$ in file
 * returns: OK - if everything is found as expected (or no log)
 *          ERROR - on error
 */
ERR_LOG
checklogstate( const char *file_name,
               const char *leader,   /* comment leader (if any) */
               BOOLEAN * improper ) /* bad copyright, leader line, etc. */
{
  char ldr[MAXPATHLEN];       /* comment leader without trailing white space */

  char buf[MAXPATHLEN];
  char *ptr, *ptr2;
  FILE *inf;
  int lead_len;                 /* length of leader (for small optimization) */
  int good_leader;              /* flag indicating good leader */
  int result = 0;
  int reuse;                    /* read new line into input buffer */
  register int i;
  ERR_LOG log;
  BOOLEAN first_comment_section = TRUE;

  buf_lines = 0;
  reuse = FALSE;
  *improper = FALSE;

  strcpy(ldr, leader);
  for (i = strlen(leader)-1; i >= 0; i--)
      if (isspace(ldr[i]))
          ldr[i] = '\0';        /* eliminate any trailing whitespace */
      else
          break;
  lead_len = strlen(ldr);

 /*
  *  Open a temporary copy of the file to check for correct log info.
  */
  if ((inf = fopen(file_name, "r")) == NULL) {
    return ( err_log ( OE_READ, file_name ) );
  } /* endif */

  if (check_copyrights == FALSE) {
    if (( getenv ( "IGNORE_COPYRIGHT" )) == NULL) {

       if (strcmp(leader, CONTCMT) == 0)     /* is this a C source file? */
        do {
         result = search_for(buf, sizeof(buf), inf, BEGINCMT, 2, ldr,
                          lead_len, &good_leader, &ptr, reuse, &log);
         if ( log != OE_OK )
           return ( log );
         reuse = FALSE;
         if (result == EOF)
           break;
         else if (result == MISSING)
           continue;
         /* if */
         /* otherwise found start of comment */
         result = search_for(buf, sizeof(buf), inf, CR, 3, ldr,
                          lead_len, &good_leader, &ptr, 0, &log);
         if ( log != OE_OK )
           return ( log );
         if (result == OK && good_leader ) {
           if ( ! legal_copyright(buf) ) {
             continue;
           }/* endif */
         } /* if */
         if (result == EOF)
           break;
         else if (result == MISSING || ! good_leader ) {
           reuse = TRUE;      /* start scanning again from this line */
        continue;
      } /* if */
      result = search_for(buf, sizeof(buf), inf, ENDCMT, 2, ldr,
                          lead_len, &good_leader, &ptr, 0, &log);
        if ( log != OE_OK )
           return ( log );
         if (result != MISSING)
           break;
         /* if */
         reuse = TRUE;        /* repeat scan starting with this line */
       } while (result != EOF);    /* end do */
      else {
       do {
         result = search_for(buf, sizeof(buf), inf, CR, 1, ldr,
                          lead_len, &good_leader, &ptr, 0, &log);
         if ( log != OE_OK )
           return ( log );
         if ( result == OK )
           if ( legal_copyright ( buf ) )
             break;
           /* if */
         /* if */
       } while (result != EOF);    /* end do */
      } /* if */
      if ( result != OK ) {
       ui_print ( VWARN, "Missing valid copyright marker in %s\n",
            working_file_tail);
       *improper = TRUE;

      /*
       * Check for file errors.
       */
       if (ferror(inf) || fclose(inf) == EOF) {
         (void) fclose(inf);
         return ( err_log ( OE_READ, file_name ) );
       } /* if */

       return ( OE_OK );
      } /* endif */
     } /* IGNORE_COPYRIGHT */
  } /* if check_copyrights */
else {

  if (( getenv ( "IGNORE_COPYRIGHT" )) == NULL) {
  reuse = FALSE;
  if (strcmp(leader, CONTCMT) == 0)     /* is this a C source file? */
    do {
      result = search_for(buf, sizeof(buf), inf, BEGINCMT, 2, ldr,
                          lead_len, &good_leader, &ptr, reuse, &log);
      if ( log != OE_OK )
        return ( log );
      reuse = FALSE;
      if (result == EOF)
        break;
      else if (result == MISSING)
        continue;
      /* endif */
      result = search_for(buf, sizeof(buf), inf, COPYRIGHT_BEGIN, 1, ldr,
                          lead_len, &good_leader, &ptr, 0, &log);
      if ( log != OE_OK ) {
        if ( err_ode_errno ( log ) == OE_EOF ) {
          ui_print ( VALWAYS, "Missing '%s' marker in %s\n", COPYRIGHT_BEGIN,
                              working_file_tail);
        } /* if */
        return ( log );
      } /* if */
      if (result != MISSING)
        break;
      reuse = TRUE;    /* couldn't find HISTORY, repeat scan from this line */
      /* endif */
    } while (result != EOF);
  else {
    result = search_for(buf, sizeof(buf), inf, COPYRIGHT_BEGIN, 2, ldr,
                        lead_len, &good_leader, &ptr, 0, &log);
    if ( log != OE_OK ) {
      if ( err_ode_errno ( log ) == OE_EOF ) {
        ui_print ( VALWAYS, "Missing '%s' marker in %s\n", COPYRIGHT_BEGIN,
                            working_file_tail);
      } /* if */
      return ( log );
    } /* if */
  } /* if */
  if (result != OK || ! good_leader ) {
    if (!good_leader) {
      ui_print ( VWARN, "Improper leader for '%s' marker in %s\n",
                        COPYRIGHT_BEGIN, working_file_tail);
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
    } else
      ui_print ( VALWAYS, "Missing '%s' marker in %s\n", COPYRIGHT_BEGIN,
                          working_file_tail);
    /* if */
    *improper = TRUE;

   /*
    * Check for file errors.
    */
    if (ferror(inf) || fclose(inf) == EOF) {
      (void) fclose(inf);
      return ( err_log ( OE_READ, file_name ) );
    } /* if */

    return ( OE_OK );
  } /* endif */
 } /* IGNORE_COPYRIGHT */
} /* check_copyrights */


  reuse = FALSE;
  if (strcmp(leader, CONTCMT) == 0)     /* is this a C source file? */
    do {
      result = search_for(buf, sizeof(buf), inf, BEGINCMT, 3, ldr,
                          lead_len, &good_leader, &ptr, reuse, &log);
      rm_newline ( buf );
      add_to_copy_section ( leader, buf );
      if ( log != OE_OK ) {
        free_copy_section();
        return ( log );
      } /* if */
      reuse = FALSE;
      if (result == EOF)
        break;
      else if (result == MISSING)
        continue;
      /* endif */
      result = search_for(buf, sizeof(buf), inf, HIST, 3, ldr, lead_len,
                          &good_leader, &ptr, 0, &log);
      if ( log != OE_OK ) {
        free_copy_section();
        return ( log );
      } /* if */
      if (result != MISSING)
        break;
      rm_newline ( buf );
      reuse = TRUE;    /* couldn't find HISTORY, repeat scan from this line */
      /* endif */
    } while (result != EOF);
  else {
    first_comment_section = TRUE;
    for (;;) {
      result = search_for(buf, sizeof(buf), inf, HIST, 3, ldr, lead_len,
                          &good_leader, &ptr, 0, &log);
      if ( log != OE_OK ) {
        free_copy_section();
        return ( log );
      } /* if */
      if ( result == OK || result == EOF ) {
        break;
      } /* if */
      if ( first_comment_section && good_leader ) {
        rm_newline ( buf );
        add_to_copy_section ( leader, buf );
      } else {
        first_comment_section = FALSE;
      } /* if */
    } /* for */
  } /* if */
  if (result != OK || ! good_leader ) {
    if (!good_leader) {
      ui_print ( VWARN, "Improper leader line between copyright marker and %s marker in %s\n", HIST, working_file_tail);
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
    } else
      ui_print ( VALWAYS, "Missing %s marker in %s\n", HIST,
                          working_file_tail);
    /* if */
    *improper = TRUE;

   /*
    * Check for file errors.
    */
    if (ferror(inf) || fclose(inf) == EOF) {
      (void) fclose(inf);
      return ( err_log ( OE_READ, file_name ) );
    } /* if */

    free_copy_section();
    return ( OE_OK );
  } /* endif */
  if ( !copyrights_ok () ) {
    *improper = TRUE;
    return ( OE_OK );
  } /* if */
  result = search_for(buf, sizeof(buf), inf, BEGINLOG, 1, ldr, lead_len,
                      &good_leader, &ptr, 0, &log);
  if ( log != OE_OK )
    return ( log );
  if (result != OK) {
    if (result == EOF)
      ui_print ( VALWAYS, "Missing %s...$ marker in %s\n", BEGINLOG,
                          working_file_tail);
    else {
      ui_print ( VWARN, "Improper leader line between %s marker and %s marker.\n" );
      ui_print ( VCONT, "File: '%s'\n", HIST, BEGINLOG, working_file_tail );
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
    } /* endif */
    *improper = TRUE;

   /*
    * Check for file errors.
    */
    if (ferror(inf) || fclose(inf) == EOF) {
      (void) fclose(inf);
      return ( err_log ( OE_READ, file_name ) );
    } /* if */

    return ( OE_OK );
  } else {
    ptr2 = ptr+4;
    if (*ptr2 != '$' && (*ptr2 != ':' || strstr(ptr2, "$") < ptr2)) {
      ui_print ( VALWAYS, "Bad %s...$ marker in %s", BEGINLOG,
                          working_file_tail);
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
      *improper = TRUE;

     /*
      * Check for file errors.
      */
      if (ferror(inf) || fclose(inf) == EOF) {
        (void) fclose(inf);
        return ( err_log ( OE_READ, file_name ) );
      } /* if */

      return ( OE_OK );
    } /* endif */
  } /* endif */

  result = search_for(buf, sizeof(buf), inf, ENDLOG, 1, ldr, lead_len,
                      &good_leader, &ptr, 0, &log);
  if ( log != OE_OK )
    return ( log );
  if (result != OK) {
    if (result == EOF) {
      ui_print ( VALWAYS, "Missing %s marker in %s\n", ENDLOG,
                          working_file_tail);
    } else if (!good_leader) {
      ui_print ( VWARN, "Improper leader line between %s marker and %s marker in %s\n", BEGINLOG, ENDLOG, working_file_tail);
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
    } /* if */
    *improper = TRUE;

   /*
    * Check for file errors.
    */
    if (ferror(inf) || fclose(inf) == EOF) {
      (void) fclose(inf);
      return ( err_log ( OE_READ, file_name ) );
    } /* if */

    return ( OE_OK );
  } /* if */
  if (strcmp(leader, CONTCMT) == 0) {
    result = search_for(buf, sizeof(buf), inf, ENDCMT, 3, ldr, lead_len,
                        &good_leader, &ptr, 0, &log);
    if ( log != OE_OK )
      return ( log );
    if (result != OK) {
      ui_print ( VALWAYS, "Missing comment end after %s marker.\n", ENDLOG,
            working_file_tail);
      ui_print ( VCONT, "Line: %d .\n", buf_lines );
      *improper = TRUE;

     /*
      * Check for file errors.
      */
      if (ferror(inf) || fclose(inf) == EOF) {
        (void) fclose(inf);
        return ( err_log ( OE_READ, file_name ) );
      } /* if */

      return ( OE_OK );
    } /* endif */
  } /* endif */

 /*
  * Check for file errors.
  */
  if (ferror(inf) || fclose(inf) == EOF) {
    (void) fclose(inf);
    return ( err_log ( OE_READ, file_name ) );
  } /* if */
  return ( OE_OK );
} /* end checklogstate */

/*
 * use rlog to obtain log messages
 * returns OK or ERROR
 */
int create_leaderless_log ( const char * name, const char * user_set,
                            char * logmsg, const char * mfile )
{
  char * rev;
  const char * av [6];
  char buffer[MAXPATHLEN];
  char tmp_logmsg [LOGMSGSIZE];
  FILE *mesgf;
  BOOLEAN found_desc_line;
  int status = OK;
  ERR_LOG log;

  enter ( "create_leaderless_log" );
  if ( ( rev = alloc_switch('r', user_set ) ) == NULL ) {
    ui_print ( VFATAL, "Revision alloc switch failed ");
    leave ( );
    return ( ERROR );
  } /* end if */
  av[0] = "rlog";
  av[1] = rev;
  av[2] = name;
  av[3] = NULL;
  log = oxm_runcmd ( rcs_monitor, 3, av, NULL );
  if ((mesgf = fopen(mfile, "w")) == NULL) {
    ui_print ( VFATAL, "fopen %s\n", mfile);
    (void) unlink(mfile);
    leave ( );
    return( ERROR );
  }
  found_desc_line = FALSE;
  while ( oxm_gets( rcs_monitor, buffer, sizeof(buffer), &log) != NULL) {
    if ( strncmp ( "description", buffer, 11 ) == 0 ) {
      found_desc_line = TRUE;
      break;
    } /* if */
  } /* while */
  if ( found_desc_line ) {
    /*
       '-----' line
    */
    if ( oxm_gets( rcs_monitor, buffer, sizeof(buffer), &log ) == NULL) {
      ui_print ( VFATAL, "Unexpected EOF 1\n" );
      status = ERROR;
    } else {
      for(;;) {
        /*
           revision line
        */
        if ( oxm_gets( rcs_monitor, buffer, sizeof(buffer), &log ) != NULL) {
          if ( strncmp ( "revision", buffer, 8 ) != 0 )
            continue;
          /* if */
          if ( buffer [ strlen ( buffer ) -2 ] == '1' )
            break;
          /* if */
        } else {
          ui_print ( VFATAL, "Unexpected EOF 2\n" );
          status = ERROR;
          break;
        }
        /*
           info line
        */
        if ( oxm_gets( rcs_monitor, buffer, sizeof(buffer), &log ) != NULL) {
        } else {
          ui_print ( VFATAL, "Unexpected EOF 3\n" );
          status = ERROR;
          break;
        } /* if */
        /*
           Actual log information
        */
        for(;;) {
          if ( oxm_gets( rcs_monitor, buffer, sizeof(buffer), &log ) != NULL) {
            /*
             * Hit end of log entry, '----etc' or '====etc'
             */
            if ( *buffer == '-' || *buffer == '-' ) {
              break;
            } /* if */
          } else {
            ui_print ( VFATAL, "Unexpected EOF 4\n" );
            status = ERROR;
            break;
          } /* if */
          fprintf ( mesgf, "%s", buffer );
          concat ( tmp_logmsg, sizeof ( tmp_logmsg ), logmsg, buffer, NULL );
          strcpy ( logmsg, tmp_logmsg );
        } /* for */
      } /* for */
    } /* if */
  } else {
    ui_print ( VFATAL, "Unable to find log information !?\n" );
    status = ERROR;
  } /* if */
  (void) fclose(mesgf);
  log = oxm_endcmd ( rcs_monitor, &status );
  leave ( );
  return ( status );
} /* end create_leaderless_log */


/*
 * 
 * New history manipulation routines that work with
 * some of the above routines and hopefully (over time)
 * will be expanded to replace above routines. 
 */

/* The routines provided are: 

history = hst_xtract_file(pathname);
			Given a pathname to a source file, this routine
			removes the history section and returns it
			as a list of HIST_ELEM structures.

hst_insert_file (pathname, history);
			Given a pathname to a source file and a history 
			structure; the history is inserted into the 
			source file.

history = hst_alloc_entry (comment_leader, logmsg, revision);
			Allocate and initialize a history node.

history = hst_merge_lists (version_to merge_to, users_version, 
			ancestor_version);
			Given the revision we wish to merge to,
			the revision on the users branch, and the 
			common ancestor of the above two; construct
			a new history list that contains the history
			the history comments unique to the user's
			branch followed by the history comments unique
			to the revision to merge to followed by
			the history comments of the ancestor rivision.

** Other functions local to this file. ** 

hst_add_comments(history_node, text)
			Given a log message, create a comment node and
			add it to the list of comments in a history node.

history_node = new_hist_elem (line, comment_leader);
			Given the revision line from an existing
			history message, create a new history node.

comment = new_comment(text)
			Given some text return a new comment node.

hst_mark_common_entries (ancestor, history_to_compare)
			Given the ancestor history and the history
			of some other revision; match all history
			nodes that are in common.

hstdup (history_node)
			return a duplicate of a history node.

hst_dump_list_fd (history, fd)
			dump the contents of a history list into a file.
		
hst_dump_list (history)
			dump the contexts of a history to stdout.

*/

HIST_ELEM
hst_xtract_file ( char * fpath, char * comment_leader )
{
   char tmp_path[PATH_LEN];
   char rev_mark[PATH_LEN];
   char Log_mark[PATH_LEN];
   char EndLog_mark[PATH_LEN];
   FILE *fd, *tmp_fd;
   int nbytes, Loglen, EndLoglen;
   char line[PATH_LEN];
   int into_code, history_done;
   struct hist_elem * head,
                    * tail,
                    * node;
   int len;

   head = NULL;
   tail = NULL;
   concat (rev_mark, sizeof(rev_mark), comment_leader, "Revision", NULL);
   concat (Log_mark, sizeof(Log_mark), comment_leader, "$Log:", NULL);
   concat (EndLog_mark, sizeof(EndLog_mark), comment_leader, "$EndLog$", NULL);
   len = strlen (rev_mark);
   Loglen = strlen (Log_mark);
   EndLoglen = strlen (EndLog_mark);

   if (access (fpath, F_OK) < 0) {
	ui_print (VFATAL, "Could not extract history from file %s; it does not exist.\n", fpath);
	return(NULL);
   }
   concat (tmp_path, sizeof(tmp_path), fpath, ".tmp", NULL);

   /* create temporary code file */
   if ((tmp_fd = fopen(tmp_path, "w+")) == NULL)
	ui_print (VFATAL, "Could not open file %s\n",
		tmp_path); 

   /* open the file */
   if ((fd = fopen(fpath, "r")) == NULL)
	ui_print (VFATAL, "Could not open file %s\n",
		fpath); 

   into_code = TRUE;
   history_done = FALSE;
   nbytes=PATH_LEN;
   node = NULL;

   /* Separate the sections. */
   while  (fgets (line, nbytes, fd) != NULL)  {

	if ((history_done == FALSE) && 
			(strncmp (line, Log_mark, Loglen) == 0)) {
		into_code = FALSE;
		/* Write marker to code file */
		fputs (line, tmp_fd);
	} else if ((history_done == FALSE) && 
			(strncmp (line, EndLog_mark, EndLoglen) == 0)) {
		into_code = TRUE;
		history_done = TRUE;
		/* Write marker to code file */
		fputs (line, tmp_fd);
	} else {

		if (into_code == TRUE) {
			/* Write code out to tmp file. */
			fputs (line, tmp_fd);
		} else {
			/* Build up history list */
       			if (strncmp (rev_mark, line, len) == 0) {
               			/* allocate a new entry. */
               			node = new_hist_elem (line, comment_leader);
               			if (head == NULL) {
                       			head = node;
                       			tail = node;
               			} else {
                       			tail->next = node;
                       			tail = node;
               			}
				hst_add_comment (node, line);
       			} else {
               			if (node != NULL) {
                       			/* Add to comment list of current node. */
					hst_add_comment (node, line);
               			} 
       			}

		}
	}
   }

   fclose (fd);
   fclose (tmp_fd);

   if (head != NULL)
   	/* Rename tmp file into "path" only if a history was extracted
	   (otherwise the tmp_path and path would be identical). */
   	rename (tmp_path, fpath);

   return (head);
}

void
hst_add_comment( HIST_ELEM node, char * text )
{
   	struct comment_elem * comment;
	char *line, *ptr;
	char buffer[MAXPATHLEN];

   if (text == NULL)
	return;

   strcpy (buffer, text);

   ptr = text;
   while ((line = hst_getline(&ptr, node->comment_leader)) != NULL) {
       	comment = new_comment(line);
       	if (node->comments == NULL) {
               	node->comments = comment;
               	node->comment_tail = comment;
       	} else {
               	node->comment_tail->next = comment;
               	node->comment_tail = comment;
       	}
	free(line);
   }

}

char *
hst_getline ( char **text, char *cmt_leader )
{

   char  buffer[MAXPATHLEN];
   char *ptr;
   int bufptr;
  
   if (*text == NULL || **text == '\0' )
	return(NULL);

   /*  Trailing spaces on comment leaders sometimes disappear. */
   if (strncmp (cmt_leader, *text, strlen(cmt_leader)) != 0) { 
	if ((strlen(*text) > 1) &&
		(strncmp (cmt_leader, *text, strlen(*text)-1) != 0)) {
		strcpy (buffer, cmt_leader);
		bufptr = strlen (buffer);
	} else
		buffer[0] = '\0';
   } else {
	buffer[0] = '\0';
   }
   bufptr = strlen (buffer);
	
   ptr = *text;

   /* Find next new-line or end-of-line. */
   for (ptr = *text; (*ptr != '\0') && (*ptr != '\n'); ptr++)
	{
		buffer[bufptr] = *ptr;
		bufptr++;
	};

   buffer[bufptr] = '\n'; 
   bufptr++;
   buffer[bufptr] = '\0';

   if (*ptr == '\n')
	*text = ptr + 1;
   else
	*text = ptr;
	
   return (strdup(buffer));
}

void
hst_insert_file ( char *fpath, HIST_ELEM history, char *comment_leader )
{
   char tmp_path[PATH_LEN];
   char Log_mark[PATH_LEN];
   char History_mark[PATH_LEN];
   int Loglen;
   int Histlen;
   FILE *fd, *tmp_fd;
   int history_done, nbytes;
   char line[PATH_LEN];

   if (history == NULL)
	return;

   if (access (fpath, F_OK) < 0) {
	ui_print (VFATAL, "Could not insert history into file %s; it does not exist.\n", path);
	return;
   }
   concat (tmp_path, sizeof(tmp_path), fpath, ".tmp", NULL);

   /* create temporary code file */
   (void) unlink (tmp_path);
   if ((tmp_fd = fopen(tmp_path, "w+")) == NULL)
	ui_print (VFATAL, "Could not open file %s\n",
		tmp_path); 

   /* open the file */
   if ((fd = fopen(fpath, "r")) == NULL)
	ui_print (VFATAL, "Could not open file %s\n",
		path); 

   nbytes=PATH_LEN;
   history_done = FALSE;
   concat (Log_mark, sizeof(Log_mark), comment_leader, "$Log:", NULL);
   concat (History_mark, sizeof(History_mark), comment_leader, "HISTORY", NULL);
   Loglen = strlen(Log_mark);
   Histlen = strlen(History_mark);

   while  (fgets (line, nbytes, fd) != NULL)  {
	if (strncmp (line, History_mark, Histlen) == 0) {
		fputs (line, tmp_fd);

   		/* Weed out comments after HISTORY line and */
   		/* insert the history section after "Log" marker. */
   		while  (fgets (line, nbytes, fd) != NULL)  {

			if (history_done == FALSE) {
			   if (strncmp (line, Log_mark, Loglen) == 0) {
				/* Write marker to code file */
				fputs (line, tmp_fd);
				/* Write the history. */
				hst_dump_list_fd (history, tmp_fd);
				history_done = TRUE;
			   }
			} else 
				fputs (line, tmp_fd);
		}



		break;
	} else 
		fputs (line, tmp_fd);
   }

   fclose (tmp_fd);
   fclose (fd);

   if (history_done == TRUE)
	/* Rename the file only if the history section has been added. */
   	rename (tmp_path, fpath);
}



HIST_ELEM
new_hist_elem ( char * line, char * comment_leader)
{
   struct hist_elem * node;
   char revision[16];
   char date[100];
   char htime[100];
   char user[32];
   char cmt_leader[100];

   node = (struct hist_elem *) malloc (sizeof(struct hist_elem));
   sscanf (line, "%s Revision %s %s %s %s", cmt_leader, revision, date, 
	htime, user);
   node->header.revision = strdup(revision); 
   node->header.date = strdup(date); 
   node->header.time = strdup(htime); 
   node->header.user = strdup(user); 
   node->next = NULL;
   node->comments = NULL;
   node->comment_tail = NULL;
   node->common_with_ancestor = FALSE;
   node->comment_leader = strdup(comment_leader);

   return (node);
}

HIST_ELEM
hst_alloc_entry ( char * comment_leader, char * logmsg, char * revision )
{

   HIST_ELEM node;
   struct passwd *pw;
   uid_t  uid;
   char line[PATH_LEN];
   char comment[PATH_LEN];
   char date[100];
   char htime[100];

   hst_get_date (date, htime);
   node = (struct hist_elem *) malloc (sizeof(struct hist_elem));
   node->comment_leader = strdup(comment_leader);
   node->header.revision = strdup(revision); 
   node->header.date = strdup(date); 
   node->header.time = strdup(htime); 
   uid = getuid();
   pw = getpwuid(uid);
   node->header.user = strdup(pw->pw_name);
   node->comments = NULL;
   node->comment_tail = NULL;
   node->common_with_ancestor = FALSE;
   node->next = NULL;

   /* build up comment list */
   /* Get revisionline. */
   concat (line, sizeof(line), comment_leader, "Revision ", node->header.revision,
	"  ", node->header.date,
	"  ", node->header.time, "  ", node->header.user, "\n", NULL);
   hst_add_comment (node, line);
   concat (comment, sizeof(comment), comment_leader, logmsg, NULL);
   hst_add_comment (node, comment);
   
   /* log message */
   concat (comment, sizeof(comment), comment_leader, "\n", NULL);
   hst_add_comment (node, comment);

   node->common_with_ancestor = FALSE;

   return (node);
}


struct comment_elem *
new_comment ( char * line )
{
   struct comment_elem * comm;

   comm = (struct comment_elem *) malloc (sizeof(struct comment_elem));
   comm->comment = (char *)strdup(line);
   comm->next = NULL;

   return (comm);
}

HIST_ELEM
hst_merge_lists ( HIST_ELEM ver_merge,  /*  The version to merge to. */
                  HIST_ELEM ver_user,   /*  The user's current version. */
                  HIST_ELEM ver_ancestor )
                                    /* The common ancestor of the above two. */
{

   struct hist_elem * hist_head,
		    * hist_tail,
		    * ptr;
   int done;
   hist_head = NULL;
   hist_tail = NULL;

   hst_mark_common_entries (ver_ancestor, ver_merge);
   hst_mark_common_entries (ver_ancestor, ver_user);

   /* Build up a history list with user comments. */
   for (ptr = ver_user, done = FALSE; (done != TRUE) && (ptr != NULL); 
	ptr = ptr->next) {

	if ( ptr -> common_with_ancestor == TRUE) {
		done = TRUE;	
	} else {
		if (hist_head == NULL) {
			hist_head = hstdup(ptr);
			hist_tail = hist_head;
		} else {
			hist_tail->next = hstdup(ptr);
			hist_tail = hist_tail->next;
		}
	}
   }

   /* Append comments from the version we are merging too. */
   for (ptr = ver_merge, done = FALSE; (done != TRUE) && (ptr != NULL); 
	ptr = ptr->next) {
	if ( ptr -> common_with_ancestor == TRUE) {
		done = TRUE;	
	} else {
		if (hist_head == NULL) {
			hist_head = hstdup(ptr);
			hist_tail = hist_head;
		} else {
			hist_tail->next = hstdup(ptr);
			hist_tail = hist_tail->next;
		}
	}
   }

   /* Tack on the common ancestor comments. */
   for (ptr = ver_ancestor, done = FALSE; (done != TRUE) && (ptr != NULL); 
	ptr = ptr->next) {
	if (hist_head == NULL) {
		hist_head = hstdup(ptr);
		hist_tail = hist_head;
	} else {
		hist_tail->next = hstdup(ptr);
		hist_tail = hist_tail->next;
	}
   }

   return (hist_head);

}


void
hst_mark_common_entries ( HIST_ELEM ver_ancestor, HIST_ELEM check )
{
   HIST_ELEM ptr;

   for (ptr = check; ptr != NULL; ptr = ptr->next) {

	if (strcmp(ptr->header.revision, ver_ancestor->header.revision) == 0) {
		ptr->common_with_ancestor = TRUE;
	} else {
		ptr->common_with_ancestor = FALSE;
	}
   }
}

HIST_ELEM
hstdup( HIST_ELEM node)
{

   struct hist_elem *new_node;

   new_node = (struct hist_elem *) malloc (sizeof(struct hist_elem));
   new_node->header.revision = strdup(node->header.revision); 
   new_node->header.date = strdup(node->header.date); 
   new_node->header.time = strdup(node->header.time); 
   new_node->header.user = strdup(node->header.user); 
   new_node->comments = node->comments;
   new_node->comment_tail = node->comment_tail;
   new_node->common_with_ancestor = FALSE;
   new_node->next = NULL;

   return (new_node);
}


void
hst_dump_list_fd ( HIST_ELEM history, FILE *fd )
{
   struct hist_elem * ptr;
   struct comment_elem * cmts;


   for (ptr = history ; ptr != NULL; ptr = ptr->next){
	for (cmts = ptr->comments;  cmts != NULL;
		cmts = cmts->next) {
		fputs (cmts->comment, fd);
	}
   }

}

void
hst_dump_list ( HIST_ELEM history )
{
   struct hist_elem * ptr;
   struct comment_elem * cmts;

   for (ptr = history ; ptr != NULL; ptr = ptr->next){
        ui_print (VALWAYS, "header -> %sRevision %s  %s  %s  %s\n",
                ptr->comment_leader, ptr->header.revision,
                ptr->header.date, ptr->header.time,
                ptr->header.user);
	for (cmts = ptr->comments;  cmts != NULL;
		cmts = cmts->next) {
                ui_print  (VALWAYS, "comment -> %s", cmts->comment);
	}
   }
}


char *
hst_next_revision ( char * revision )
{
  char result[30];
  int a, b, c, d;
  if (revision == NULL)
	return(NULL);

  sscanf (revision, "%d.%d.%d.%d", &a, &b, &c, &d);
  d++;
  sprintf (result, "%d.%d.%d.%d", a, b, c, d);

  return(strdup(result));
}

void
hst_get_date ( char * date,                              /* date in mm/dd/yy */
               char * ltime )                               /* time in hh:mm */

        /* This function get the current date and time in hours
           and minutes and returns the values. */

{
    struct tm * time_info;                        /* structure to hold ctime */
    long        t_clock;                                        /* misc long */

  t_clock = time (( long * ) 0 );
  time_info = localtime ( &t_clock );
/* FIX ME ... Hard coded century shouldn't be here. */
  sprintf ( date, "%d/%02d/%02d", time_info->tm_year, 
		time_info->tm_mon + 1, time_info->tm_mday);

  sprintf ( ltime, "%d:%02d:%02d", time_info->tm_hour, time_info->tm_min,
		time_info->tm_sec);
}                             

void
hst_freelist( HIST_ELEM node ) 
{
     HIST_ELEM ptr, next_ptr;
     struct comment_elem * cmt, *next_cmt;

     for (ptr = node; ptr != NULL;) {
          next_ptr = ptr->next;
          for (cmt = ptr->comments ; cmt != NULL;) {

                 next_cmt = cmt->next;
                 free (cmt->comment);
                 free (cmt);
                 cmt = next_cmt;

          }
          free(ptr->comment_leader);
          free(ptr->header.revision);
          free(ptr->header.date);
          free(ptr->header.time);
          free(ptr->header.user);
          free(ptr);
          ptr = next_ptr;
     }
}
