static char sccsid[] = "@(#)00	1.13  src/bos/usr/bin/bsh/hash.c, cmdbsh, bos411, 9428A410j 9/1/93 17:32:15";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: hcreate hscan hfind henter crunch
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.7  com/cmd/sh/sh/hash.c, cmdsh, bos324
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

#include	"defs.h"
#include	"hash.h"

#define STRCMP(A, B)	(cf(A, B) != 0)
#define FACTOR 	 		035761254233	/* Magic multiplication factor */
#define TABLENGTH		64				/* must be multiple of 2 */
#define LOG2LEN			6				/* log2 of TABLENGTH */

/*
    NOTE: The following algorithm only works on machines where
    the results of multiplying two integers is the least
    significant part of the double word integer required to hold
    the result.  It is adapted from Knuth, Volume 3, section 6.4.
*/

#define hash(str)		(int) (((unsigned) (crunch(str) * FACTOR)) >> shift)

struct node
{
	ENTRY item;
	struct node *next;
};

static struct node	**last;
static struct node	*next;
static struct node 	**table;

static unsigned int 	bitsper;		/* Bits per byte */
static unsigned int	shift;

static unsigned int crunch();

hcreate()
{
	uchar_t c = ~0;			/* A byte full of 1's */
	int j;

	table = (struct node **)malloc(TABLENGTH * sizeof(struct node *));

	for (j=0; j < TABLENGTH; ++j)  
	{
		table[j] = 0;
	}

	bitsper = 0;

	while (c)		
	{
		c >>= 1;
		bitsper++;
	}

	shift = (bitsper * sizeof(int)) - LOG2LEN;
}


void hscan(uscan)	
	void	(*uscan)();
{
	struct node		*p, *nxt;
	int				j;

	for (j=0; j < TABLENGTH; ++j)
	{
		p = table[j];
		while (p)
		{
			nxt = p->next;
			(*uscan)(&p->item);
			p = nxt;
		}
	}
}



ENTRY *
hfind(str)
	uchar_t	*str;
{
	struct node 	*p;
	struct node 	**q;
	unsigned int 	i;
	int 			res;		

	i = hash(str);

	if(table[i] == 0)
	{			
		last = &table[i];
		next = 0;
		return(0);
	}
	else 
	{
		q = &table[i];
		p = table[i];
		while (p != 0 && (res = STRCMP(str, p->item.key))) 
		{
			q = &(p->next);
			p = p->next;
		}

		if (p != 0 && res == 0)	
			return(&(p->item));
		else
		{
			last = q;
			next = p;
			return(0);
		}
	}
}

ENTRY *
henter(item)
	ENTRY item;
{
	struct node	*p = (struct node *)malloc(sizeof(struct node));

	p->item = item;
	*last = p;
	p->next = next;
	return(&(p->item));
}


static unsigned int 
crunch(key)	
	uchar_t	*key;
{
	unsigned int 	sum = 0;	
	int s;

	for (s = 0; *key; s++)				/* Simply add up the bytes */
		sum += *key++;

	return(sum + s);
}

