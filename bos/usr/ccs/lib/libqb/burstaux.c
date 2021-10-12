static char sccsid[] = "@(#)83  1.10  src/bos/usr/ccs/lib/libqb/burstaux.c, libqb, bos411, 9428A410j 12/1/93 09:44:38";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: put_header, put_trailer, asterisks, blankline
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * queueing system routines to print headers and trailers.
 * documented in "How to Write Your Own Backend"
 */

#include <stdio.h>
#include <IN/standard.h>
#include <IN/backend.h>
#include <IN/stfile.h>
#include <ctype.h>
#include <nl_types.h>
#include <stdlib.h>
#include "libqb_msg.h"
#include "notify.h"
#define MAXSTR          10



#define pchr(x) ( cfunc? (*cfunc)(x): putchar(x) )

extern char *getmesg();

struct tm       *localtime();
extern struct stfile __stfile;
static int (*cfunc)();
static twocolumn();
static int fw;

static char blank[] = " ";

put_header(fn, width)
	int (*fn)(); {
	char now[40];
	time_t timet;
	struct tm *tm_time;
	int date_size;

	cfunc = fn;
	fw = width;

	timet = time((long *) 0);
	tm_time = localtime(&timet);
	date_size = strftime(now,40,"%c",tm_time);
	if ( date_size > 24 )
	{
		if ( MB_CUR_MAX == 1 )
			now[24] = '\0';
		else trunc_str ( now, 24 );
	}

	asterisks();
	asterisks();
	blankline();
	twocolumn(MSGSTR(MSGTITL,"Title:"), __stfile.s_title);
	blankline();
	twocolumn(MSGSTR(MSGTIME,"Time printed:"), now);
	blankline();
	twocolumn(MSGSTR(MSGQUEU,"Time queued:"), __stfile.s_qdate);
	blankline();
	twocolumn(MSGSTR(MSGPRNT,"Printed by:"), __stfile.s_from);
	blankline();
	twocolumn(MSGSTR(MSGDELI,"Deliver to:"), __stfile.s_to);
	blankline();
	asterisks();
	asterisks();

	return (17);      /* # of lines written */
}

put_trailer(user, fn, width)
	char *user;
	int (*fn)(); {

	cfunc = fn;
	fw = width;

	asterisks();
	asterisks();
	blankline();
	twocolumn(MSGSTR(MSGTWOC,"End of output for:"), user);
	blankline();
	asterisks();
	asterisks();

	return (7);       /* # of lines written */
}

/*
 * twocolumn
 *      put the specified strings in two columns
 */

/*
 * checks that the form width is not exceeded both when putting
 * out the heading column and when putting out the text column.
 */

#define TEXTCOL  22     /* start of text field    */

/* VARARGS1 */
static twocolumn( heading, text )
 char *heading, *text;
{       register int count;
	register char *s;
	register int len;

	/* 4 spaces before left column */
	for ( count = 0; count < 4; count++ ) pchr(' ');

	/* output the heading           */
	for( s = heading, count = 4; *s && count < TEXTCOL-1 &&
	     count < fw; count++ )
		pchr( *s++ );

	/* output spaces to get us to where the text should start */
	while( count < TEXTCOL )
	{       pchr(' ');
		count++;
	}

	/* truncate the text (on the left) as necessary to fit field    */
	len = strlen( text ) - (fw - TEXTCOL);
	if (len > 0)
		text += len;

	/* output the text                                              */
	for( s = text; *s && count < fw; count++)
		pchr( *s++ );

	pchr('\r');
	pchr('\n');
}


asterisks()     /* print a line of asterisks indented 4 spaces */
{       int i;

	for ( i = 0; i < 4; i++ ) pchr(' ');
	for ( ; i < fw; i++ ) pchr('*');
	pchr('\r');
	pchr('\n');
}


blankline()
{
	pchr('\r');
        pchr('\n');
}

/*
 * NAME: trunc_str
 * FUNCTION:  This function will truncate a string to a specified
 *    number of characters.  It checks for multibyte characters and
 *    truncates to the last multibyte character in the string such that
 *    the total number of bytes is <= length.  Assumes that the buffer
 *    size of str is > length, though the string length itself may
 *    be < length.
 */
static trunc_str ( str, length)
char *str;
int length;
{
	int count=0, newcount=0, i=0, wclen;
	wchar_t wc;

	do {
		if ( (wclen =  mbtowc(&wc, &str[count], MB_CUR_MAX)) == -1 ) {
			fprintf(stderr,MSGSTR(MSGMBER, "Error in multibyte character conversion.\n"));
			exit(-1);
		}
		else {
			newcount = count + wclen;
			if ( newcount > length ) {
				str[count] = '\0';
				break;
			}
			else
				count = newcount;
		}
	}
	while ( (count <= length) && (wclen != 0) );
}


/* function used to retrieve message from the message catalog */
char *getmesg(int num,char *str)
{
	char *p, *dest;
	nl_catd catd;

	catd = catopen(MF_LIBQB, NL_CAT_LOCALE);
	p = catgets(catd,MS_LIBQB,num,str);
	dest = malloc(strlen(p) + 1);
	strcpy(dest,p);
        catclose(catd);
        return(dest);

}   

