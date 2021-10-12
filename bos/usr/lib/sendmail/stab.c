static char sccsid[] = "@(#)29	1.3  src/bos/usr/lib/sendmail/stab.c, cmdsend, bos411, 9428A410j 6/15/90 23:26:16";
/* 
 * COMPONENT_NAME: CMDSEND stab.c
 * 
 * FUNCTIONS: stab 
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

# include <stdio.h>
# include <ctype.h>
# include <string.h>
# include <memory.h>

# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <netinet/in.h>

# include "sendmail.h"

char  *xalloc ();

# define STABSIZE	400
static STAB	*SymTab[STABSIZE];

#ifdef DEBUG
/*
 *  dumpstab - dump to stdout all entries of the specified type
 */
void dumpstab(type)
int type;
{
int i, j, k, c;
register STAB *s;

    /* step through table and look for entries of given type */
    for (i = 0; i < STABSIZE; ++i) {
	s = SymTab[i];
	while (s) {  /* trace links for this hash table entry */
	    if (s->s_type == type) {  /* found one */
		printf(
		    "  name='%s', hash=%d, value=%#08x %08x %08x %08x\n",
		    s->s_name, i, s->s_class[0], s->s_class[1], s->s_class[2],
		    s->s_class[3]);
		if (type == ST_CLASS) {  /* print class character(s) */
		    fputs("    class(es) =", stdout);
		    /* check each bit of bitmask */
		    for (k = 0; k < BITMAPINTS; ++k)
			for (j = 0; j <= BITININTMASK; ++j)
			    if (s->s_class[k] & (1 << j))  /* bit is set */
				/* print value of bit as a char or int */
				if (isprint(c = (k * (BITININTMASK + 1)) + j))
				    printf(" '%c'", c);
				else
				    printf(" %#02x", c);
		    putchar('\n');
		}
	    }
	    s = s->s_next;
	}
    }
}
#endif DEBUG

/*
 *  freestab - free and clear all entries of the specified type
 */
void freestab(type)
int type;
{
int i;
register STAB *s, *prev, *next;

    /* step through table and look for entries of given type */
    for (i = 0; i < STABSIZE; ++i) {
	s = SymTab[i];
	prev = NULL;  /* previous "good" entry */
	while (s) {  /* trace links for this hash table entry */
	    if (s->s_type == type) {  /* found one */
# ifdef DEBUG
		if (tTd(0, 25))
		    printf("freestab: freeing entry '%s'\n", s->s_name);
# endif DEBUG
		free(s->s_name);
		next = s->s_next;
		free(s);
		s = next;  /* point to next entry */
		/* remove this entry from list */
		if (prev)
		    prev->s_next = s;
		else  /* first in list */
		    SymTab[i] = s;
	    } else {  /* good one: keep it */
		prev = s;
		s = s->s_next;
	    }
	}
    }
}

/*
**  STAB -- manage the symbol table
**
**	Parameters:
**		name -- the name to be looked up or inserted.
**		type -- the type of symbol.
**		op -- what to do:
**			ST_ENTER -- enter the name if not
**				already present.
**			ST_FIND -- find it only.
**
**	Returns:
**		pointer to a STAB entry for this name.
**		NULL if not found and not entered.
**
**	Side Effects:
**		can update the symbol table.
*/

STAB *
stab(name, type, op)
	char *name;
	int type;
	int op;
{
	register STAB *s;
	register STAB **ps;
	register int hfunc;
	register char *p;
	char  lowname[MAXNAME];

	(void) uncapstr (lowname, name, MAXNAME); /* just use max */
	(void) unquotestr (lowname, lowname, MAXNAME);

# ifdef DEBUG
	if (tTd(36, 5))
		(void) printf("STAB: name %s type %d lowname %s ", 
						name, type, lowname);
# endif DEBUG

	/*
	**  Compute the hashing function
	**
	**	We could probably do better....
	*/

	hfunc = type;

	for (p = lowname; *p != '\0'; p++)
		hfunc = (((hfunc << 7) | *p) & 077777) % STABSIZE;

# ifdef DEBUG
	if (tTd(36, 9))
	{
	    (void) printf("hfunc %d ", hfunc);
	    s = SymTab[hfunc];
	    while (s)
	    {
		(void) printf ("(%s,%d) ", s->s_name, s->s_type);
		s = s->s_next;
	    }
	}
# endif DEBUG

	/*
	 *  Beginning at hash location, search string of structures till
	 *  a match is found, or the end of the string.
	 */
	ps = &SymTab[hfunc];
	while ((s = *ps) != NULL && (!sameword(lowname, s->s_name) || s->s_type != type))
		ps = &s->s_next;

	/*
	 *  If entry found (or if op == ST_FIND) just return pointer or NULL.
	 */
	if (s != NULL || op == ST_FIND)
	{
# ifdef DEBUG
		if (tTd(36, 5))
		{
			if (s == NULL)
				(void) printf("not found\n");
			else
			{
				long *lp = (long *) s->s_class;

				(void) printf("type %d val %lx %lx %lx %lx\n",
					s->s_type, lp[0], lp[1], lp[2], lp[3]);
			}
		}
# endif DEBUG
		return (s);
	}

	/*
	 *  Make the new entry
	 */
	s = (STAB *) xalloc(sizeof (*s));
	(void) memset((char *) s, 0, sizeof (*s));
	s->s_name = newstr(lowname);
	s->s_type = type;
	*ps = s;			/* link it in */

# ifdef DEBUG
	if (tTd(36, 5))
		(void) printf("entered\n");
# endif DEBUG

	return (s);
}
