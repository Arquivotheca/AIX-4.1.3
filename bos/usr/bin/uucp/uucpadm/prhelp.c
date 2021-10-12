static char sccsid[] = "@(#)76	1.3  src/bos/usr/bin/uucp/uucpadm/prhelp.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:26";
/* 
 * COMPONENT_NAME: CMDUUCP prhelp.c
 * 
 * FUNCTIONS: npage, prhelp 
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  uucpadm
 *
 *  Copyright (c) 1988 IBM Corporation.  All rights reserved.
 *
 *
 */

#include <curses.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include "defs.h"

extern  int  Badfile;
extern  WINDOW *helpwin;

int dhelp();
int derror();

struct index_struct { char  *ptr; int  len; };

static char  help[HELPSIZE];
static struct index_struct indexs[INDEXSIZE];

prhelp (s)
char  *s;
{
    static int first = 1;
    int  size, prflag;
    int npage();
    register int  i;
    register char  *p;
    register struct index_struct  *ip;

    if (first)
    {
	int  fd;
	first = 0;
/* Initialize the help window. */

	werase(helpwin);
	fd = open (HELPFILE, O_RDONLY);
	if (fd < 0)
	{
		wattron(helpwin,A_REVERSE);
	        if ((wprintw (helpwin," uucpadm: error opening help file %s ", HELPFILE)) == ERR)
			derror(EX_SC,"Error on write to help screen!");
		wattroff(helpwin,A_REVERSE);
		dhelp();
		(void) wgetch(helpwin);
		Badfile = 1;
		return;
	}

	size = read (fd, help, HELPSIZE);
	(void) close (fd);
	if (size < 0)
	{
		wattron(helpwin,A_REVERSE);
	        if ((wprintw (helpwin," uucpadm: error reading help file %s ", HELPFILE)) == ERR)
			derror(EX_SC,"Error on write to help screen!");
		wattroff(helpwin,A_REVERSE);
		dhelp();
		(void) wgetch(helpwin);
		Badfile = 1;
		return;
	}

	if (size > HELPSIZE - 2)	/* lv room for possbly two more chars */
	{
		wattron(helpwin,A_REVERSE);
	        if ((wprintw (helpwin, " uucpadm: help file %s too big", HELPFILE)) == ERR)
			derror(EX_SC,"Error on write to help screen!");
		wattroff(helpwin,A_REVERSE);
		dhelp();
		(void) wgetch(helpwin);
		Badfile = 1;
		return;
	}

	/*
	 *  Change help buffer to convenient null terminated strings, followed
	 *  by 0xff.
	 */
	if (size == 0 || help[size - 1] != '\n') /* add \n if necessary */
	    help[size++] = '\n';		/* add missing newline */

	help[size++] = 0xff;		/* add superterminator */

	p = help;			/* cvt newlines to nulls */
	i = size - 1;			/* don't look at final 0xff */
	prflag = 0;
	while (i-- > 0)
	{
	    register  int c;

	    c = *p;
	    if (c == '\0' || c == 0xff)		/* ck for illegal chars */
	    {
		if (!prflag) {
			wattron(helpwin,A_REVERSE);
	    		if ((wprintw (helpwin, " uucpadm: help file %s invalid format", HELPFILE)) == ERR)
			derror(EX_SC,"Error on write to help screen!");
			wattroff(helpwin,A_REVERSE);
			dhelp();
			(void) wgetch(helpwin);
		}
		prflag = 1;

		*p = ' ';			/* chg to blank */
	    }

	    if (c == '\n')			/* newlines to nulls */
		*p = '\0';

	    p++;
	}

	/*
	 *  Build the index.  Keys start in col 1 and terminate with |.
	 *  Only the first line of a keyed section has the key.
	 */
/**/	/* OPTIMIZE */
	for (ip = indexs, i = 0; i < INDEXSIZE; ip++, i++) /* null out ptr */
	    ip->ptr = NULL;

	ip = indexs;			/* init index pointer	*/
	p  =  help;			/* init help scanner	*/

	prflag = 0;			/* msg control flag	*/

	while (1)
	{
	    register char  *psave;

	    if (*p == 0xff)		/* superterminator?	*/
		break;

	    psave = strchr (p, '|');	/* locate key delimiter */

	    if (psave == NULL)		/* no key? */
	    {
	        p += strlen (p) + 1;	/* move to next string */
	        ip->len++;		/* count the one passed over */
		continue;		/* loop */
	    }

	    *psave = '\0';		/* isolate the key */

	    if (ip >= &indexs[INDEXSIZE - 2])	/* leave last one for stopper */
	    {
		if (!prflag) {
		wattron(helpwin,A_REVERSE);
	    	if ((wprintw (helpwin," uucpadm: prhelp index overflow")) == ERR)
			derror(EX_SC,"Error on write to help screen!");
		wattroff(helpwin,A_REVERSE);
		dhelp();
		(void) wgetch(helpwin);
		}
		prflag = 1;
	    }
	    else				/* successfully */
	    {
		if (ip->ptr != NULL)		/* might be first time */
	            ip++;
	        ip->ptr = p;			/* point to key string */
	        ip->len = 1;			/* initialize length */
		p += strlen (p) + 1;		/* skip the key */
		p += strlen (p) + 1;		/* to next string */
	    }
	}

    }

    if (Badfile)
	return;

    /*
     *  Search index for key
     */
	werase(helpwin);
    ip = indexs;
    while (ip->ptr != NULL && strcmp (ip->ptr, s))
	ip++;

    /*
     *  Print out the string packet
     */
    if (ip->ptr != NULL)
    {
        p = ip->ptr;				/* start of key */
        p += strlen (p) + 1;			/* skip to start of text */


        for (i = 0; i < ip->len; i++)		/* output the text */
        {
/* we will allow the help file to spill over into multiple screens. */
	    if (helpwin->_cury > LINES - 3)
		(void) npage();
	    if ((wprintw (helpwin,"%s\n",p)) == ERR)
		derror(EX_SC,"Error on write to help screen!");
	    p += strlen (p) + 1;
        }
	wattron(helpwin,A_REVERSE);
	    if ((mvwprintw (helpwin,helpwin -> _cury, COLS / 3,"Hit any key to continue ")) == ERR)
		derror(EX_SC,"Error on write to help screen!");
	wattroff(helpwin,A_REVERSE);
    }
}
int npage()
{
	wattron(helpwin,A_REVERSE);
	    if ((mvwprintw (helpwin,helpwin -> _cury, COLS / 3,"Hit any key to continue ")) == ERR)
		derror(EX_SC,"Error on write to help screen!");
	wattroff(helpwin,A_REVERSE);
	clear();
	refresh();
	wrefresh(helpwin);
	(void) wgetch(helpwin);
	werase(helpwin);
	return(EX_OK);
}
