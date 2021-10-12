static char sccsid[] = "@(#)72  1.9  src/bos/usr/bin/mail/strings.c, cmdmailx, bos411, 9428A410j 11/17/93 10:41:11";
/* 
 * COMPONENT_NAME: CMDMAILX strings.c
 * 
 * FUNCTIONS: MSGSTR, salloc, sdel_item, sreset 
 *
 * ORIGINS: 10  26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * #ifndef lint
 * static char *sccsid = "strings.c    5.2 (Berkeley) 6/21/85";
 * #endif not lint
 */

/*
 * Mail -- a mail program
 *
 * String allocation routines.
 * Strings handed out here are reclaimed at the top of the command
 * loop each time, so they need not be freed.
 */


#include "rcv.h"

#include "mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

static sdel_item();
/*
 *	These string allocation routines have been recoded from the 
 *	Berkeley release.  They now use malloc() to get the space, 
 *	rather than managing it themselves.  In order to retain the
 *	bulk-disposal capability of sreset(), each malloc()'ed pointer
 *	is saved in a list, so that they can all be free()'ed at once.
 *	This is a much saner (and more portable) way to do it (if I do
 *	say so myself).  The original Berkeley code is ifdef'ed out
 *	below, for those with a morbid curiousity.
 *	Ron Hitchens, IBM Austin RT Group, July 87
 */


/*
 *	The structure for squirrling away the malloc()'ed pointers, so
 *	that we can nuke them all later
 */

struct str_itm {
	struct str_itm	*next;		/* pointer to next string item */
	char		*str;		/* pointer originally malloc()'ed */
};
typedef struct str_itm	str_item;	/* a more convenient name for it */

#define	NULL_ITEM	(str_item *)0	/* a null pointer to one */


/*	The anchors of the list of memory blocks that have been malloc'ed */

static	str_item	*first = (void *)0, *last = (void *)0;
/*static	str_item	*first = NULL_ITEM, *last = NULL_ITEM; */


/*
 *	allocate a memory block of the requested size.  Since I'm not
 *	sure if the caller is leaving room for the trailing null, I'll
 *	give him an extra byte just in case
 */

char *
salloc (size)
	int	size;
{
	char		*p = (char *)malloc (size + 1);
	str_item	*item = (str_item *)malloc (sizeof (str_item));

	if (p == NULL || item == NULL_ITEM) {
		fprintf(stderr, MSGSTR(NOSPACE, "no space to allocate string\n")); /*MSG*/
		panic (MSGSTR(INTERROR, "internal error")); /*MSG*/
	}
	item->next = NULL_ITEM;
	item->str = p;

	if (first == NULL_ITEM) {
		first = last = item;	/* list was empty */
	} else {
		last->next = item;	/* add to end of list */
		last = item;
	}
	return (p);
}

/*
 *	Dispose of all the memory blocks allocated by salloc(), since
 *	startup or the last sreset().  
 */

sreset ()
{
	if (noreset)
		return;			/* never mind */
	minit ();
	sdel_item (first);		/* recursively delete the list */
	first = last = NULL_ITEM;	/* clear the anchor pointers */
}

static
sdel_item (p)
	str_item	*p;
{
	if (p == NULL_ITEM)		/* that's the end of the list */
		return;
	sdel_item (p->next);		/* delete the guy to my right first */
	free (p->str);			/* free the string */
	free (p);			/* free the string item */
}


#ifdef notdef
/* --------------------------------------------------------------------- */

/*
 *	Below are the the original Berkeley string allocation routines.
 *	I'm not sure of the original reason for doing all this space
 *	management here, but it loses for portability.  Returning and
 *	address on a 16bit boundary doesn't get it for all machines.
 *	The replacement routines above accomplish the same thing the
 *	routines below did, using the local malloc() to get and manage
 *	the space.  Which is the right thing to do in the first place.
 *	Ron Hitchens,	IBM Austin RT Group, July 87
 */

/*
 * Allocate size more bytes of space and return the address of the
 * first byte to the caller.  An even number of bytes are always
 * allocated so that the space will always be on a word boundary.
 * The string spaces are of exponentially increasing size, to satisfy
 * the occasional user with enormous string size requests.
 */

char *
salloc(size)
{
	register char *t;
	register int s;
	register struct strings *sp;
	int index;

	s = size;
	s++;
	s &= ~01;
	index = 0;
	for (sp = &stringdope[0]; sp < &stringdope[NSPACE]; sp++) {
		if (sp->s_topFree == NOSTR && (STRINGSIZE << index) >= s)
			break;
		if (sp->s_nleft >= s)
			break;
		index++;
	}
	if (sp >= &stringdope[NSPACE])
		panic(MSGSTR(BIGSTR, "String too large")); /*MSG*/
	if (sp->s_topFree == NOSTR) {
		index = sp - &stringdope[0];
		sp->s_topFree = (char *) calloc(STRINGSIZE << index,
		    (unsigned) 1);
		if (sp->s_topFree == NOSTR) {
			fprintf(stderr, MSGSTR(NOROOM, "No room for space %d\n"), index); /*MSG*/
			panic(MSGSTR(INTERNAL, "Internal error")); /*MSG*/
		}
		sp->s_nextFree = sp->s_topFree;
		sp->s_nleft = STRINGSIZE << index;
	}
	sp->s_nleft -= s;
	t = sp->s_nextFree;
	sp->s_nextFree += s;
	return(t);
}

/*
 * Reset the string area to be empty.
 * Called to free all strings allocated
 * since last reset.
 */

sreset()
{
	register struct strings *sp;
	register int index;

	if (noreset)
		return;
	minit();
	index = 0;
	for (sp = &stringdope[0]; sp < &stringdope[NSPACE]; sp++) {
		if (sp->s_topFree == NOSTR)
			continue;
		sp->s_nextFree = sp->s_topFree;
		sp->s_nleft = STRINGSIZE << index;
		index++;
	}
}
#endif

