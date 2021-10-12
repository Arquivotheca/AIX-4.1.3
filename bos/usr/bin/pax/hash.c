static char sccsid[] = "@(#)68	1.2  src/bos/usr/bin/pax/hash.c, cmdarch, bos411, 9428A410j 6/26/91 13:08:40";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: pax
 *
 * ORIGINS: 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log$
 *
 */
/* static char rcsid[] = "RCSfile Revision (OSF) Date"; */
/*
 * hash.c - hash table functions
 *
 * DESCRIPTION
 *
 *	These routines are provided to manipulate the hashed list
 *	of filenames in an achive we are updating.
 *
 * AUTHOR
 *
 *     Tom Jordahl - The Open Software Foundation
 *
 *
 */

/* Headers */

#include "pax.h"


static	Hashentry	*hash_table[HTABLESIZE]; 

/* hash_name - enter a name into the table
 *
 * DESCRIPTION
 *
 *	hash_name places an entry into the hash table.  The name
 *	is hashed in to a 16K table of pointers by adding all the
 *	byte values in the pathname modulo 16K.  This gives us a
 *	moderatly size table without too much memory usage.
 *	
 *	Will update the mtime of an entry which already exists.
 *
 * PARAMETERS
 *
 *	char 	*name 	- Name to be hashed
 *	Stat	*sb	- stat buffer to get the mtime out of
 *
 */


void hash_name(char *name, Stat *sb)

{
    Hashentry	*entry;
    Hashentry	*hptr;
    char  	*p;
    uint	 total=0;


    p = name;
    while (*p != '\0') {
	total += *p; 
	p++;
    }

    total = total % HTABLESIZE;

    if ((entry = (Hashentry *)mem_get(sizeof(Hashentry))) == NULL) {
	fatal(MSGSTR(NOMEM, "Out of memory"));
    }

    if ((hptr = hash_table[total]) != NULL) {
	while (hptr->next != NULL) {
	    if (!strcmp(name, hptr->name)) {
		hptr->mtime =  sb->sb_mtime;
		free(entry);
		return;
	    }
	    hptr = hptr->next;
	}
	hptr->next = entry;
    } else {
	hash_table[total] = entry;
    }

    entry->name = mem_str(name);
    entry->mtime = sb->sb_mtime;
    entry->next = NULL;

}


/* hash_lookup - lookup the modification time of a file in hash table
 *
 * DESCRIPTION
 *
 *	Check the hash table for the given filename and returns the
 *	modification time stored in the table, -1 otherwise.
 *
 * PARAMETERS
 *
 *	char *name 	- name of file to lookup
 *
 * RETURNS
 *
 *	modification time found in the hash table.
 *	-1 if name isn't found.
 *
 */


time_t hash_lookup(char *name)

{
    char	*p;
    uint	 total=0;
    Hashentry	*hptr;

    p = name;
    while (*p != '\0') {
	total += *p; 
	p++;
    }

    total = total % HTABLESIZE;
   
    if ((hptr = hash_table[total]) == NULL) 
	return((time_t) -1);

    while (hptr != NULL) {
	if (!strcmp(name, hptr->name)) 
	    return(hptr->mtime);	/* found it */
	hptr = hptr->next;
    }

    return((time_t) -1);		/* not found */
}
