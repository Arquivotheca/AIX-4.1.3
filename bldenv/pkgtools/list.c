static char sccsid[] = "@(#)26  1.2  src/bldenv/pkgtools/list.c, pkgtools, bos412, GOLDA411a 1/29/93 14:28:01";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: linkNew
 *		listAppend
 *		listCompare
 *		listEnd
 *		listGet
 *		listInit
 *		listInsert
 *		listNew
 *		listRewind
 *		space
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "list.h"

List *
listNew()
{
	return (List *)	space(sizeof(struct _List));
}

static Link *
linkNew()
{
	return (Link *)	space(sizeof(struct _Link));
}

void
listInit(List *l)
{
	l->count = 0;
	l->last	= l->current = (Link *)0;
}

int
listEnd(List *l)
{
	return (l->current == (Link *) 0);
}

void
listRewind(List *l)
{
	l->current = l->last->next;
}

char *
listGet(List *l)
{
	char	*d;

	d = l->current->data;
	l->current = (l->current == l->last) ? (Link *)	0 : l->current->next;
	return(d);
}

void
listInsert(List *l, char *data)
{
	Link	*newLink = linkNew();

	if (l->last) {
		newLink->next =	l->last->next;
		l->last->next =	newLink;
	}
	else {
		newLink->next =	newLink;
		l->last	= newLink;
	}
	newLink->data =	data;
	l->count++;
}

void
listAppend(List *l, char *data)
{
  char *d;
  
  while(!listEnd(l))
    d = listGet(l);

  listInsert(l, data);
  l->last = l->last->next;
}

char *
space(int size)
{
	char            *new;

	new = calloc(size, 1);
	return(new);
}

int
listCompare(List *l, char *data)
{
	char *c;
	
	listRewind(l);

	while ( !listEnd(l) )
	{
		c = listGet(l);

		if ( !strcmp (c, data) )
			return (1);
	}
	return (0);
}
