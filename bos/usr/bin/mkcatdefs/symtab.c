static char sccsid[] = "@(#)83	1.9  src/bos/usr/bin/mkcatdefs/symtab.c, cmdmsg, bos411, 9428A410j 7/8/93 11:58:22";
/*
 * COMPONENT_NAME: (CMDMSG) Message Catalogue Facilities
 *
 * FUNCTIONS: insert, nsearch, hash
 *
 * ORIGINS: 9, 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 Release 1.0
 */

#define HASHSIZE 256			/* must be a power of 2 */
#define HASHMAX HASHSIZE - 1


/*
 * EXTERNAL PROCEDURES CALLED: None
 */

struct name {
  char *regname;
  int   regnr;
  struct name *left;
  struct name *right;
  };

struct name *symtab[HASHSIZE];		/* hashed pointers to binary trees */

char *calloc();
char *malloc();

/*
 * NAME: insert
 *
 * FUNCTION: Insert symbol
 *
 * EXECUTION ENVIRONMENT: 
 *  	User mode.
 * 
 * NOTES: These routines manipulate a symbol table for the mkcatdefs program.
 *  	  The symbol table is organized as a hashed set of binary trees.  If the
 *  	  symbol being passed in is found then a -1 is returned, otherwise the
 *  	  symbol is placed in the symbol table and a 0 is returned.  The purpose
 *  	  of the symbol table is to keep mkcatdefs from assigning two different
 *  	  message set / message numbers to the same symbol.
 *        Read the next line from the open message catalog descriptor file.
 *
 * RETURNS: 0 - symbol inserted.
 *         -1 - symbol exists.
 */

insert(char *tname, int seqno)

	/*
	  tname - pointer to symbol
	  seqno - integer value of symbol
	*/

{
	register struct name *ptr,*optr;
	int rslt = -1,i,hashval;

	hashval = hash(tname);
	ptr = symtab[hashval];

	/* search the binary tree for specified symbol */
	while (ptr && (rslt = strcmp(tname,ptr->regname))) {
		optr=ptr;  
		if (rslt<0)
			ptr = ptr->left;
		else
			ptr = ptr->right;
	}

	if (rslt == 0)		/* found the symbol already defined */
		return (-1);

	/* symbol not defined yet so put it into symbol table */
	else {
		ptr = (struct name *)calloc(sizeof(struct name), 1);
		ptr->regname = malloc(strlen(tname) + 1);
		strcpy (ptr->regname, tname);
		ptr->regnr = seqno;

		/* not first entry in tree so update branch pointer */
		if (symtab[hashval]) {
			if (rslt < 0)
				optr->left = ptr;
			else
				optr->right = ptr;

		/* first entry in tree so set the root pointer */
		} else
			symtab[hashval] = ptr;

		return (0);
	}
}

/*
 * NAME: insert
 *
 * FUNCTION: Insert symbol
 *
 * EXECUTION ENVIRONMENT: 
 *  	User mode.
 * 
 * NOTES: Searches for specific identifier. If found, return allocated number.
 * 	  If not found, return -1.
 *
 * RETURNS: Symbol sequence number if symbol is found.
 *          -1 if symbol is not found.
 */

nsearch (char *tname)

	/*
	  tname - pointer to symbol
	*/

{
	register struct name *ptr,*optr;
	int rslt = -1,i,hashval;

	hashval = hash(tname);
	ptr = symtab[hashval];

	/* search the binary tree for specified symbol */
	while (ptr && (rslt = strcmp(tname,ptr->regname))) {
		optr=ptr;  
		if (rslt<0)
			ptr = ptr->left;
		else
			ptr = ptr->right;
	}

	if (rslt == 0)		/* found the symbol already defined */
		return(ptr->regnr);
	else
		return (-1);
}


/*
 * NAME: hash
 *
 * FUNCTION: Create hash value from symbol name.
 *
 * EXECUTION ENVIRONMENT: 
 *  	User mode.
 * 
 * NOTES: Hash the symbol name using simple addition algorithm.
 * 	  Make sure that the hash value is in range when it is returned.
 *
 * RETURNS: A hash value.
 */

hash (name)
register char *name;

	/*
	  name - pointer to symbol
	*/

{
	register int hashval = 0;

	while (*name)
		hashval += *name++;

	return (hashval & HASHMAX);
}
