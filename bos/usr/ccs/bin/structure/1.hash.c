static char sccsid[] = "@(#)60	1.5  src/bos/usr/ccs/bin/structure/1.hash.c, cmdprog, bos411, 9428A410j 3/9/94 13:19:37";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: addref, clear, connect, fixvalue, hash, hash_check, hash_free,
	      hash_init, parse
 *
 * ORIGINS: 26 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include "1.incl.h"
#include "1.defs.h"
#include"def.h"

extern int match[], symclass[],  action[], newstate[];
extern char symbol[];
long *hashtab;
int *value, *chain;

extern FILE *infd;


parse()
	{int i,j,found,current, someread;
	char c;

	hash_init();
	routinit();
	line_init();

	someread = 0;			/* indicates haven't read part of a routine */

	empseek(0);
	endbuf = getline(&endline, &endchar, &endcom, & comchar);
	if (progress && endbuf != -1) fprintf(stderr,MSGSTR(PARSING, "parsing\n")); /*MSG*/
	while(endbuf != -1)			/* getline returns -1 when no more input */
		{
		someread = 1;
		if (progress > 0)
			{
			for (i = begline; i <= endline; i++)
				if (!(i % progress)) fprintf(stderr,MSGSTR(PARSINGLINE, "parsing line %d\n"),i); /*MSG*/
			}
		current = 0;
		for (i = 0; i < endbuf; i++)
			{

			c = buffer[i];
			if(c != '~') 
				{
				found = 0;
				if ( (current < 0 || current >= snum) && current != ABORT)
					{
					error("", "", "");
					fprintf(stderr, MSGSTR(PARSINVST,
					  "struct error in parsing:\nline %d of file, parser in invalid state"),
					  begline,current);
					fprintf(stderr,MSGSTR(STRGTCODE, "treating it as straight line code\n")); /*MSG*/
					current = ABORT;
					}
				else
					for (j = match[current];  j < match[current + 1]; j++)
						{
						if ((symclass[j] == 0 && c == symbol[j]) ||
						    (symclass[j] != 0 && classmatch(c,symclass[j]) ))
							{found = 1;  break;
							}
						}
				if (!found)
					{
					error("", "", "");
					fprintf(stderr, MSGSTR(BTWNLINES,
						"error in syntax:\nbetween lines %d and %d of file\n"),
						begline, endline);
					if (debug)
					fprintf(stderr,MSGSTR(SYMBOLNOTMATCH, "symbol '%c' does not match entries for state %d\n"),c,current); /*MSG*/
					fprintf(stderr,MSGSTR(STRGTCODE, "treating it as straight line code\n")); /*MSG*/
					current = ABORT;
					}
				else if (!action[j])  
					current = newstate[j];
				else
					{
					current = act(action[j],c,i);
					if (current == nulls)  current = newstate[j];
					}
				if (current == ABORT)  break;
				if (current == endrt)
					{
					return(1);
					}
				}
			}
		line_init();
		endbuf = getline(&endline, &endchar, &endcom,&comchar);
		}
	if (someread) return(1);
	else return(0);
	}


hash_init()
	{
	int i;
	hashtab = (long *) challoc(sizeof(*hashtab) * maxhash);
	chain = (int *) challoc(sizeof(*chain) * maxhash);
	value = (int *) challoc(sizeof(*value) * maxhash);
	for (i = 0; i < maxhash; i++)
		{
		hashtab[i] = -1L;
		value[i] = -2;
		chain[i] = 0;
		}
	}


hash_check()
	{
	int i;
	for (i = 0; i < maxhash; ++i)
		if (value[i] == -2 && hashtab[i] != -1L)
			{
			error(MSGSTR(LABUSED, "in syntax; label used but does not appear as statement label:"),"",""); /*MSG*/
			fprintf(stderr,"%ld\n",hashtab[i]);
			routerr = 1;
			}
	}

hash_free()
	{
	chfree(hashtab,sizeof(*hashtab) * maxhash);
	hashtab = 0;
	chfree(chain,sizeof(*chain) * maxhash);
	chain = 0;
	chfree(value,sizeof(*value) * maxhash);
	value = 0;
	}
hash(x)
long x;
	{
	int quo, rem, hcount, temp;

	ASSERT(x >= 0L, hash);
	quo = x/maxhash;
	rem = x - (quo * maxhash);
	if (quo == 0)  quo = 1;

	temp = rem;
	for (hcount=0; (hashtab[temp] != -1L) && (hashtab[temp] != x) && (hcount<maxhash); hcount++)
		temp = (temp + quo)%maxhash;
	if(hcount>=maxhash) faterr(MSGSTR(HASHOVERFLW, "hash table overflow - too many labels"),"",""); /*MSG*/
	hashtab[temp] = x;
	return(temp);
	}

addref(x,ptr)				/* put ptr in chain for x or assign value of x to *ptr */
long x;
int *ptr;
	{
	int index;
	index = hash(x);

	if (value[index]  == -1)
		{			/* x already assigned value */
		*ptr = chain[index];
		return;
		}
	
	/* add ptr to chain */
	
	if (chain[index] == 0)
		*ptr = 0;
	else
		*ptr = chain[index];
	chain[index] = (int) ptr;
	}

fixvalue (x,ptr)
long x;
int ptr;
	{
	int *temp1, *temp2, index, temp0;
	index = hash(x);

	while (index != -2)
		{			/* trace chain of linked labels */

		if (value[index]  == -1)
			{
			error("", "", "");
			fprintf(stderr, MSGSTR(REDEFLAB,
		     		"error in syntax:\nattempt to redefine value of label %ld between lines %d and %d\n"),
				x, begline,endline);
			routerr = 1;
			return;
			}
	
		temp1 = &chain[index];		/* trace chain for each label */
		while (temp1 != 0)
			{
			temp2 = (int *) *temp1;
			*temp1 = ptr;
			temp1 = temp2;
			}
		temp0 = index;
		index = value[index];
		value[temp0] = -1;
		}
	}

connect(x,y)
long x,y;
	{
	int *temp, index, temp2;
	index = hash(x);

	if (value[index] == -1)
		fixvalue(y, chain[index]);
	else
		{
		if (y == implicit)
		{		/* attach implicit chain to x chain */
		temp = &chain[index];
	
		while (*temp != 0)
			temp = (int *) *temp;
	
		*temp = chain[hash(y)];
		}
		temp2 = index;		/* attach y linked labels to x linked labels */
		while (value[temp2] >= 0)
			temp2 = value[temp2];
		if (y == implicit)
			value[temp2] = value[hash(y)];
		else
			value[temp2] = hash(y);
		}
	if (y == implicit)  clear(y);
	}
	
	
clear(x)
long x;
	{
	int index;
	index = hash(x);
	value[index] = -2;
	chain[index] = 0;
	hashtab[index] = -1L;
	}


