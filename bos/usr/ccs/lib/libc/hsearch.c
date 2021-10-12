static char sccsid[] = "@(#)04	1.3  src/bos/usr/ccs/lib/libc/hsearch.c, libcsrch, bos411, 9428A410j 10/20/93 14:30:13";
/*
 *   COMPONENT_NAME: LIBCSRCH
 *
 *   FUNCTIONS: COMPARE
 *		FOUND
 *		HASH
 *		HASH2
 *		HCREATE
 *		HDESTROY
 *		HDUMP
 *		HSEARCH
 *		STRCMP
 *		build
 *		compare
 *		crunch
 *		hcreate
 *		hcreate_r
 *		hdestroy
 *		hdestroy_r
 *		hdump
 *		hdump_r
 *		hsearch
 *		hsearch_r
 *		main
 *
 *   ORIGINS: 3,27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

/* hsearch.c,v $ $Revision: 1.5.2.4 $ (OSF) */
/*
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 */


#include <stdio.h>				/* for NULL	*/
#include <stdlib.h>
#include <string.h>
#include <search.h>				/* for ACTION	*/
#include <values.h>				/* for BITSPERBYTE */


/* Compile time switches:

   CHAINED	- use a linked list to resolve collisions.
	SORTUP		- CHAINED list is sorted in increasing order.
	-or-
	SORTDOWN	- CHAINED list is sorted in decreasing order.
	-or-
	START		- CHAINED list with entries appended at front.
	-or-
	(null)		- entries appended at end of chain
   -or-
   OPEN		- use open addressing to resolve collisions.
	BRENT		- Brent's modification to improve the OPEN algorithm.

   MULT		- use a multiplicative hashing function.
   -or-
   DIV		- use the remainder mod table size as a hashing function.

   USCR		- user supplied comparison routine.
   DRIVER	- compile in a main program to drive the tests.
   DEBUG	- compile some debugging printout statements.
*/

#ifdef OPEN
#	undef CHAINED
#else
#	ifndef CHAINED
#		define OPEN
#	endif
#endif	/* OPEN */

#ifdef MULT
#	undef DIV
#else
#	ifndef DIV
#		define MULT
#	endif
#endif	/* MULT */

#ifdef START
#	undef SORTUP
#	undef SORTDOWN
#else
#	ifdef SORTUP
#		undef SORTDOWN
#	endif
#endif	/* START */

#ifdef USCR
#define COMPARE(A, B)	(*hcompar)((A), (B))
extern int (*hcompar)();
#else
#define COMPARE(A, B)	strcmp((A), (B))
#endif	/* USCR */

#ifdef MULT
/*
 * NOTE: The following algorithm only works on machines where
 * the results of multiplying two integers is the least
 * significant part of the double word integer required to hold
 * the result.  It is adapted from Knuth, Volume 3, section 6.4.
 */

#define SHIFT	((BITSPERBYTE * sizeof(int)) - DIM)	/* Shift factor */
#define FACTOR	035761254233		/* Magic multiplication factor */
/*
 * Multiplication hashing scheme
 */
#define	HASH(key) \
	((int) (((unsigned) (crunch(key) * FACTOR)) >> SHIFT))
/*
 * Secondary multiplication hashing scheme
 */
#define	HASH2(key) \
	((int) (((unsigned) ((crunch(key) * FACTOR) << DIM) >> SHIFT) | 1))

#else /* MULT */

/*
 * Division hashing scheme
 */
#define	HASH(key)	(crunch(key) % LENGTH)
/*
 * Secondary division fudge
 */
#define HASH2(key)	1

#endif	/* MULT */


#ifdef OPEN

typedef ENTRY	TABELEM;	/* What the table contains (TABle ELEMents) */

static unsigned int	count;	/* Number of entries in hash table */

#else	/* OPEN */
	/* CHAINED */

typedef struct node {	/* Part of the linked list of entries */
	ENTRY		item;
	struct node	*next;
} NODE;
typedef NODE	*TABELEM;

#endif	/* OPEN */


#ifdef _THREAD_SAFE

#include <errno.h>

#define	FOUND(x)	(*target = x, (*target) ? 0 : (errno=ESRCH, -1))
#define	NOSPACE		(errno=ENOMEM, -1)

#define	TABLE	((TABELEM *)hsearch_data->table)
#define	DIM	(hsearch_data->dim)
#define	LENGTH	(hsearch_data->length)
#define	COUNT	(hsearch_data->count)
#define	PRCNT	(hsearch_data->prcnt)

#else

#define	FOUND(x)	(x)
#define	NOSPACE		(0)

#define	TABLE	table
#define	DIM	dim
#define	LENGTH	length
#define	COUNT	count
#define	PRCNT	prcnt

static TABELEM		*table;	/* The address of the hash table */
static unsigned int	length;	/* Size of the hash table */
static unsigned int	dim;	/* Log base 2 of length */
static unsigned int	prcnt;	/* Number of probes this item */

#endif /* _THREAD_SAFE */


/*
 * Convert multicharacter key to unsigned int
 */
static unsigned int	
crunch(char *key)
{
	unsigned int	sum = 0;	/* Results */
	int		s;		/* Length of the key */

	for (s = 0; *key; s++)	/* Simply add up the bytes */
		sum += *key++;

	return (sum + s);
}


/*
 * NAME:	hcreate
 *
 * FUNCTION:	create the hash table
 *
 * NOTES:	Hcreate creates the hash table.  The parameter indicates
 *		the largest number of entries this hash table will contain.
 *
 * DATA:	'table' is allocated
 *
 * RETURN:	1 is returned if the allocation succeeds,
 *		else 0 is returned
 */
#ifdef _THREAD_SAFE
int
hcreate_r(	size_t size,	/* max size for hash table */
		struct hsearch_data *hsearch_data)
#else
int
hcreate(size_t size)	/* max size for hash table */
#endif	/* _THREAD_SAFE */
{
	unsigned int	unsize;	/* Holds the shifted size */

	if (size == 0)
		return (0);

	unsize = size;	/* +1 for empty table slot; -1 for ceiling */
	LENGTH = 1;	/* Maximum entries in table */
	DIM = 0;		/* Log2 length */
	while (unsize) {
		unsize >>= 1;
		LENGTH <<= 1;
		DIM++;
	}
#ifdef OPEN
	COUNT = 0;
#endif	/* OPEN */

#ifdef _THREAD_SAFE
	hsearch_data->table = calloc((size_t)LENGTH, sizeof(TABELEM));
#else
	table = (TABELEM *)calloc((size_t)LENGTH, sizeof(TABELEM));
#endif
	return (TABLE != NULL);
}


/*
 * NAME:	hdestroy
 *                                                                    
 * FUNCTION:	destroy the hash table
 *                                                                    
 * NOTES:	hdestroy frees up the hash table.  This allows the
 *		program to then hcreate another, as only one hash
 *		table can exist at a time.
 *
 * DATA:	'table' is freed
 *
 * RETURN:	none
 */
#ifdef _THREAD_SAFE
void
hdestroy_r(struct hsearch_data *hsearch_data)
#else
void
hdestroy(void)
#endif /* _THREAD_SAFE */
{
	free((void *)TABLE);
}


#ifdef OPEN

/*
 * Hash search of a fixed-capacity table.  Open addressing used to
 * resolve collisions.  Algorithm modified from Knuth, Volume 3,
 * section 6.4, algorithm D.  Labels flag corresponding actions.
 */

/*
 * NAME:	hsearch
 *                                                                    
 * FUNCTION:	search and/or add to a hash table
 *                                                                    
 * NOTES:	hsearch searches a hash table for a matching entry.  If
 *		no matching entry is found and 'action' is ENTER, then
 *		the entry is added, if possible.  If no matching entry
 *		is found, and 'action' is FIND, then the entry is not
 *		added.
 *
 * DATA:	'table' may be added to
 *
 * RETURN:	NULL if 'action' is FIND and the entry
 *		was not found, NULL if 'action' is ENTER, the entry was
 *		not found, and the table was full, else a pointer to
 *		the matching entry.
 */
#ifdef _THREAD_SAFE
int
hsearch_r(	ENTRY	item,		/* Item to be inserted or found */
		ACTION	action,		/* FIND or ENTER */
		ENTRY	**target,
		struct hsearch_data *hsearch_data)
#else
ENTRY *
hsearch(	ENTRY	item,		/* Item to be inserted or found */
		ACTION	action)		/* FIND or ENTER */
#endif	/* _THREAD_SAFE */
{
	unsigned int	i;	/* Insertion index */
	unsigned int	c;	/* Secondary probe displacement */

	PRCNT = 1;

	/* D1: */
	i = HASH(item.key);	/* Primary hash on key */

	/* D2: */
	if (TABLE[i].key == NULL)	/* Empty slot? */
		goto D6;
	else if (COMPARE(TABLE[i].key, item.key) == 0)	/* Match? */
		return (FOUND(&TABLE[i]));

	/* D3: */
	c = HASH2(item.key);	/* No match => compute secondary hash */

D4:
	i = (i + c) % LENGTH;	/* Advance to next slot */
	PRCNT++;

	/* D5: */
	if (TABLE[i].key == NULL)	/* Empty slot? */
		goto D6;
	else if (COMPARE(TABLE[i].key, item.key) == 0)	/* Match? */
		return (FOUND(&TABLE[i]));
	else
		goto D4;

D6: 
	if (action == FIND)		/* Insert if requested */
		return (FOUND(NULL));
	if (COUNT == (LENGTH - 1))	/* Table full? */
		return (NOSPACE);

#ifdef BRENT
	/* Brent's variation of the open addressing algorithm.  Do extra
	 * work during insertion to speed retrieval.  May require switching
	 * of previously placed items.  Adapted from Knuth, Volume 3, section
	 * 4.6 and Brent's article in CACM, volume 10, #2, February 1973.
	 */

	 {   
		unsigned int	p0 = HASH(item.key);   /* First probe index */
		unsigned int	c0 = HASH2(item.key);  /* Main branch incr */
		unsigned int	r = PRCNT - 1; /* Current minimum distance */
		unsigned int	j;         /* Counts along main branch */
		unsigned int	k;         /* Counts along secondary branch */
		unsigned int	curj;      /* Current best main branch site */
		unsigned int	curpos;    /* Current best table index */
		unsigned int	pj;        /* Main branch indices */
		unsigned int	cj;        /* Secondary branch incr distance */
		unsigned int	pjk;       /* Secondary branch probe indices */

		if (PRCNT >= 3) {
			/* Count along main branch */
			for (j = 0; j < PRCNT; j++) {
				/* New main branch index */
				pj = (p0 + j * c0) % LENGTH;
				/* Secondary branch incr. */
				cj = HASH2(TABLE[pj].key);
				/* Count on secondary branch*/
				for (k = 1; j + k <= r; k++) {
					/* Secondary probe */
					pjk = (pj + k * cj) % LENGTH;
					/* Improvement found */
					if (TABLE[pjk].key == NULL) {
						/* Decrement upper bound */
						r = j + k;
						/* Save main probe index */
						curj = pj;
						/* Save secondeary index */
						curpos = pjk;
					}
				}
			}
			/* If an improvement occurred */
			if (r != PRCNT - 1) {
				/* Old key to new site */
				TABLE[curpos] = TABLE[curj];
				i = curj;
			}
		}
	}
#endif	/* BRENT */

	COUNT++;			/* Increment table occupancy count */
	TABLE[i] = item;		/* Save item */
	return (FOUND(&TABLE[i]));	/* Address of item is returned */
}


#else	/* OPEN */
	/* CHAINED */

#ifdef SORTUP
#        define STRCMP(A, B) (COMPARE((A), (B)) > 0)
#else
#	ifdef SORTDOWN
#		define STRCMP(A, B) (COMPARE((A), (B)) < 0)
#	else
#		define STRCMP(A, B) (COMPARE((A), (B)) != 0)
#	endif	/* SORTDOWN */
#endif	/* SORTUP */


static ENTRY *
build(	NODE	**last, 	/* Where to store in last list item */
	NODE	*next, 	/* Link to next list item */
	ENTRY	item)	/* Item to be kept in node */
{
	NODE	*p = (NODE * )malloc(sizeof(NODE));

	if (p != NULL) {
		p->item = item;
		*last = p;
		p->next = next;
		return (&(p->item));
	}
	return (NULL);
}


/*
 * Chained search with sorted lists
 */
#ifdef _THREAD_SAFE
int
hsearch_r(	ENTRY	item,		/* Item to be inserted or found */
		ACTION	action,		/* FIND or ENTER */
		ENTRY	**target,
		struct hsearch_data *hsearch_data)
#else
ENTRY *
hsearch(	ENTRY	item,		/* Item to be inserted or found */
		ACTION	action)		/* FIND or ENTER */
#endif	/* _THREAD_SAFE */
{
	NODE		*p;	/* Searches through the linked list */
	NODE		**q;	/* Where to store the pointer to a new NODE */
	unsigned int	i;	/* Result of hash */
	int		res;	/* Result of string comparison */

	PRCNT = 1;

	i = HASH(item.key);	/* Table[i] contains list head */

	if (TABLE[i] == (NODE * )NULL) {
		/* List has not yet been begun */
		if (action == FIND)
			return (FOUND(NULL));
		return (FOUND(build(&TABLE[i], (NODE *)NULL, item)));
	} else {
		/* List is not empty */
		q = &TABLE[i];
		p = TABLE[i];
		while (p != NULL && (res = STRCMP(item.key, p->item.key))) {
			PRCNT++;
			q = &(p->next);
			p = p->next;
		}

		if (p != NULL && res == 0)	/* Item has been found */
			return (FOUND(&(p->item)));
		/* Item is not yet on list */
		if (action == FIND)
			return (FOUND(NULL));
#ifdef START
		return (FOUND(build(&TABLE[i], TABLE[i], item)));
#else
		return (FOUND(build(q, p, item)));
#endif	/* START */
	}
}


#endif /* CHAINED */



#ifdef DRIVER

#ifdef _THREAD_SAFE

#define	RES			(&res)

#define	HCREATE(s)		hcreate_r(s, hsearch_data)
#define	HSEARCH(e, a, x)	(!hsearch_r(e, a, &x, hsearch_data))
#define	HDESTROY()		hdestroy_r(hsearch_data)
#define	HDUMP()			hdump_r(hsearch_data)

#else

#define	RES			res

#define	HCREATE(s)		hcreate(s)
#define	HSEARCH(e, a, x)	((x = hsearch(e, a)) != NULL)
#define	HDESTROY()		hdestroy()
#define	HDUMP()			hdump()

#endif	/* _THREAD_SAFE */


#ifdef USCR
static int	
compare(char *a, char *b)
{
	return (strcmp(a, b));
}


int	(*hcompar)() = compare;
#endif /* USCR */


/* Dumps loc, data, probe count, key */
static void	
#ifdef _THREAD_SAFE
hdump_r(struct hsearch_data *hsearch_data)
#else
hdump(void)
#endif /* _THREAD_SAFE */
{
	ENTRY		*tgt;
	unsigned int	i;	/* Counts table slots */
#ifdef OPEN
	unsigned int	sum = 0;	/* Counts probes */
#else	/* OPEN */
	/* CHAINED */
	NODE * a;		/* Current Node on list */
#endif	/* OPEN */

	for (i = 0; i < LENGTH; i++)
#ifdef OPEN
		if (TABLE[i].key == NULL)
			printf("%o.\t-,\t-,\t(NULL)\n", i);
		else {
			unsigned int	oldpr = PRCNT; /* Save current probe count */
			HSEARCH(TABLE[i], FIND, tgt);
			sum += PRCNT;
			printf("%o.\t%d,\t%d,\t%s\n", i,
			    *TABLE[i].data, PRCNT, TABLE[i].key);
			PRCNT = oldpr;
		}
	printf("Total probes = %d\n", sum);

#else	/* OPEN */
	/* CHAINED */

	if (TABLE[i] == NULL)
		printf("%o.\t-,\t-,\t(NULL)\n", i);
	else {
		printf("%o.", i);
		for (a = TABLE[i]; a != NULL; a = a->next)
			printf("\t%d,\t%#0.4x,\t%s\n",
			    *a->item.data, a, a->item.key);
	}
#endif	/* OPEN */
}


main()
{
#ifdef _THREAD_SAFE
	struct hsearch_data	data;
	struct hsearch_data	*hsearch_data = &data;
#endif	/* _THREAD_SAFE */
	ENTRY	*res;		/* Result of hsearch */
	char	line[80];	/* Room for the input line */
	int	i = 0;		/* Data generator */
	ENTRY	*new;		/* Test entry */

	if (HCREATE(5))
		printf("Length = %u, m = %u\n", LENGTH, DIM);
	else {
		fprintf(stderr, "Out of core\n");
		exit(1);
	}

	for (;;) {
		HDUMP();

		printf("Enter a probe: ");
		fflush(stdout);
		if (EOF == scanf("%s", line))
			break;

		new = (ENTRY *)malloc(sizeof(ENTRY));
		if (new == NULL) {
			fprintf(stderr, "Out of core \n");
			exit(1);
		}

		new->key = malloc((unsigned) strlen(line) + 1);
		if (new->key == NULL) {
			fprintf(stderr, "Out of core \n");
			exit(1);
		}

		strcpy(new->key, line);
		new->data = malloc(sizeof(int));
		if (new->data == NULL) {
			fprintf(stderr, "Out of core \n");
			exit(1);
		}

		*new->data = i++;
		if (!HSEARCH(*new, ENTER, res))
			printf("Table is full\n");
		else {
			printf("Success: ");
			printf("Key = %s, Value = %d\n", res->key, *res->data);
		}
		printf("The number of probes required was %d\n", PRCNT);
	}
	exit(0);
}

#endif	/* DRIVER */
