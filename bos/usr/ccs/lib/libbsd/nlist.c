static char sccsid[] = "@(#)29	1.9  src/bos/usr/ccs/lib/libbsd/nlist.c, libbsd, bos411, 9428A410j 3/22/94 17:06:49";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS:  nlist
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <sys/param.h>
#include <a.out.h>

/*
 * NAME:	nlist
 *
 * FUNCTION:	nlist - get entries from a name list
 *
 * NOTES:	Nlist allows a program to examine the name list in
 *		the executable (or object) file named by the 'filename'
 *		parameter.  It selectively extracts a list of values
 *		and places them in the array of nlist structures pointed
 *		to by the 'nl' parameter.  The 'nl' array consists of
 *		'nlist' structures containing the names of the desired
 *		symbols.  The array is terminated by a structure with
 *		a null 'n_name' member.  Nlist fills in in the remaining
 *		elements of the desired elements.
 *
 *		This is the xcoff version.
 *
 * RETURN VALUE DESCRIPTION:	the number of unfound entries on successful
 *		completion, or -1 if an error occurs
 */

/*
 * Use n_name for struct nlist, _n._n_name for struct syment
 */
#define	sym_name	_n._n_name

#define	STRBUF	BSIZE           /* size of buffer used for string table */

/*
 * object file types we handle...
 */
#define	BADMAG(x)	\
	((x.f_magic) != U800TOCMAGIC && (x.f_magic) != U802TOCMAGIC)

/*
 * macros to help us look around the file...
 */
#define	A_SYMPOS(x)	x.f_symptr
#define	A_NSYMS(x)	x.f_nsyms
#define	A_NAMEPOS(x)	(A_SYMPOS(x) + A_NSYMS(x) * SYMESZ)

/*
 * what we get for each symbol...
 */
#define	NLSET(n,s)	(n)->n_value = (s)->n_value,	\
			(n)->n_type = (s)->n_type,	\
			(n)->n_sclass = (s)->n_sclass,	\
			(n)->n_scnum = (s)->n_scnum,	\
			(n)->n_numaux = (s)->n_numaux

int
nlist(filename, nl)
const char *filename;		/* the file to examine...	*/
struct nlist *nl;		/* the symbols to retrieve...	*/
{
	register FILE *fp;
	register struct nlist   *p;
	register char   *np, *sp;
	register long   nsyms;
	register int    nnls;
		 struct syment cursym;
 	register struct syment *symp = &cursym;
		 long   begstr = 0, endstr = 0;
		 struct filehdr    hdr;
		 char   strbuf[STRBUF];

	/* clear all nlist entries */
	for( p = nl; (np = p->n_name) != NULL && np[0] != '\0'; p++ )
	{
	    static   struct nlist   znlist;

	    *p = znlist;
	    p->n_name = np;
	}

	/* nnls = number of nlist entries */
	nnls = p - nl;

	/*
	 * p->n_name (a pointer) may either be NULL, or point to '\0'.
	 * If the latter, make it NULL so the inner loop below is fast.
	 */
	p->n_name = NULL;

	/* attempt to open filename */
	if( (fp = fopen(filename, "r")) == NULL )
		return(-1);

	/* read file header and check file type */
	(void) fread((void *) &hdr, (size_t)sizeof(hdr), (size_t)1, fp);
	if( BADMAG(hdr) )
	{   fclose(fp);
	    return(-1);
	}

	(void) fseek(fp, A_SYMPOS(hdr), SEEK_SET);  /* seek symbol table */

	/*
	 * scan through the symbols, read only SYMESZ bytes at
	 * a time (even though that may be < sizeof(struct syment),
	 * that's actually all that the link editor left us...)
	 */
	for (nsyms = A_NSYMS(hdr); nsyms > 0 &&
	     fread((void *) symp, (size_t)SYMESZ, (size_t)1, fp) == 1 && nnls > 0; nsyms--) {
		/* skip auxiliary entry */
		fseek(fp, symp->n_numaux * SYMESZ, SEEK_CUR);

		if ((symp->n_sclass != C_EXT))
			continue;

		if( symp->n_zeroes == 0 )
		{   
                    /* long name; use string table */
		    if( symp->n_offset < begstr || symp->n_offset >= endstr )
		    {   /* offset is not within strbuf, need to read */

				 int    len;
				 long   home;

			/* check for absurd offset; offset includes the
			 * (long) string table length which precedes the table
			 */
			if( symp->n_offset < sizeof(long) ) continue;

			/* save our place */
			home = ftell(fp);

			/* get a chunk of string table */
			(void) fseek(fp, A_NAMEPOS(hdr)+symp->n_offset,
					SEEK_SET);
			if ((len = fread((void *) strbuf, (size_t)1, (size_t)STRBUF, fp)) <= 0)
			{   /* file malformed, or I/O error */
			    fclose(fp);
			    return(-1);
			}
			/* back to where we were */
			(void) fseek(fp, home, SEEK_SET);

			/* don't include partial strings in the buffer */
			while( strbuf[len-1] != '\0' )
			    if( --len <= 0 )
			    {   /* strings longer than STRBUF aren't allowed */
				*strbuf = '\0';
				break;
			    }

			/* indicate the limits of the buffer */
			begstr = symp->n_offset;
			endstr = begstr + len;
		    }
		    /* locate string within strbuf */
		    sp = strbuf + (int)(symp->n_offset-begstr);
	            /* see if it is in the list */
	            for (p = nl;
			 (np = p->n_name) != NULL && strcmp(sp, np) != 0;
		         p++)
		        	;
		} else {
		    /* short name - use strncmp since the name may not
		     * be null terminated if it's SYMNMLEN long... */
		    sp = symp->sym_name;
	            /* see if it is in the list */
	            for (p = nl; (np = p->n_name) != NULL &&
			 strncmp(sp, np, SYMNMLEN) != 0; p++)
				;
		}

		/*
		 * find it in the list?  (also be sure we haven't seen
		 * this symbol already)
		 */
		if (np != NULL && p->n_value == 0) {
			/* set the list entry from the symbol table entry */
			NLSET(p, symp);
			/* decrement number of entries left ... */
			--nnls;
			}
	}

	fclose(fp);

	return(nnls);
}
