static char sccsid[] = "@(#)79	1.5  src/bos/usr/bin/pax/replace.c, cmdarch, bos412, 9446B 11/11/94 21:54:20";
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
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
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
/* $Source: /u/mark/src/pax/RCS/replace.c,v $
 *
 * $Revision: 1.2 $
 *
 * replace.c - regular expression pattern replacement functions
 *
 * DESCRIPTION
 *
 *	These routines provide for regular expression file name replacement
 *	as required by pax.
 *
 * AUTHORS
 *
 *	Mark H. Colburn, NAPS International
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Log:	replace.c,v $
 * Revision 1.2  89/02/12  10:05:59  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:36  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: replace.c,v 1.2 89/02/12 10:05:59 mark Exp $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* not lint */

/* Headers */

#include "pax.h"
#define  MAXBKREF 10

/*
 * sepsrch - search a wide char string for a wide char
 *
 * DESCRIPTION
 *
 *	Search the input string for the specified delimiter
 *	If the char is found then return a pointer into the
 *  input string where the character was found, otherwise
 *  return a NULL string.
 *
 * PARAMETERS
 *
 *	wcstr - the string to be searched
 *	sep	  - the char to search for
 */
static wchar_t *
sepsrch(const wchar_t *wcstr, wchar_t sep)
{
	wchar_t *wc;

	wc = wcstr;
	if (*wc == (wchar_t)NULL)
		return ((wchar_t *)NULL);
	do {
		if ((*wc == sep) && (*(wc-1) != L'\\'))
			return (wc);
	} while (*(++wc));
	return((wchar_t *)NULL);
}


/*
 * chk_back_ref - check the back references in a replacement string
 *
 * DESCRIPTION
 *	Check for valid back references in the replacement string.  An
 *  invalid back reference will cause chk_back_ref to return -1.
 *  No back references will return 0, otherwise the highest back
 *  reference found will be returned.
 *
 * PARAMETERS
 *
 *	wchar_t *ptr - a wide char pointer into the replacement string.
 */
static int 
chk_back_ref(const wchar_t *ptr)
{
	int		bkref;
	int		retval;

	retval = 0;
	while (*ptr != L'\0') {
		if (*ptr++ == L'\\') {
			/* get out quick if an invalid back reference */
			if (*ptr == L'0')
				return -1;
			if ((*ptr >= L'1') && (*ptr <= L'9')) {
				bkref = *ptr - '0';
				retval = (retval < bkref) ? bkref : retval;
			}
			ptr++;
		}
	}
	return retval;
}


/* add_replstr - add a replacement string to the replacement string list
 *
 * DESCRIPTION
 *
 *	Add_replstr adds a replacement string to the replacement string
 *	list which is applied each time a file is about to be processed.
 *
 * PARAMETERS
 *
 *	char	*pattern	- A regular expression which is to be parsed
 */


void add_replstr(char *pattern)

{
    Replstr	*rptr;
    wchar_t	wc_pattern[PATH_MAX+1];
	wchar_t *wcs;
	wchar_t	*ptr;
	wchar_t *wptr;
	wchar_t	sep;
	char    old[PATH_MAX+1];
	char    *new;
	size_t	retval;
	int		highbkref;

    if ((rptr = (Replstr *) calloc(1,sizeof(Replstr))) == (Replstr *)NULL) {
	warn(MSGSTR(RP_NOADD, "Replacement string not added"),
	     MSGSTR(RP_SPACE, "No space"));
	return;
    }

	if ((new = (char *) malloc(strlen(pattern)+1)) == (char *)NULL) {
	warn(MSGSTR(RP_NOADD, "Replacement string not added"),
	     MSGSTR(RP_SPACE, "No space"));
	free(rptr);
	return;
	}
	retval = mbstowcs(wc_pattern,pattern,PATH_MAX+1);
	switch (retval) {
	case -1:
		/*ERROR-Invalid mb char*/
		goto quit;
	case PATH_MAX+1:
		/*ERROR-Name too large no terminating null */
		goto quit;
	default:
		break;
	}

	/* get the delimiter - the first character in wcpattern*/
	/* make wcs now point to the first character in old */
	sep = wc_pattern[0];
	wcs = wc_pattern+1;
/* set wptr to point to old */
	wptr = wcs;
	retval = 0;

	/* find the second seperator */
	if ((ptr = sepsrch(wcs,sep)) == (wchar_t *)NULL) {
		warn(MSGSTR(RP_NOADD, "Replacement string not added"),
	    	MSGSTR(RP_BADDEL,  "Bad delimeters"));
		goto quit;
	}

	/* old will point to the search string and be null terminated */
	wcs = ptr;
	*ptr = (wchar_t)L'\0';
	wcstombs(old,wptr,PATH_MAX+1);

	/* set wptr to point to new */
	wptr = ++wcs;

	/* find the third and last seperator */
	if ((ptr = sepsrch(wcs,sep)) == (wchar_t *)NULL) {
		warn(MSGSTR(RP_NOADD, "Replacement string not added"),
	    	MSGSTR(RP_BADDEL,  "Bad delimeters"));
		goto quit;
	}
	/* new will point to the replacement string and be null terminated */
	wcs = ptr;
	*ptr = (wchar_t)L'\0';
	wcstombs(new,wptr,PATH_MAX+1);

	/* check for trailing g or p options */
	while (*++wcs) {
		if (*wcs == (wchar_t)L'g')
            rptr->global = 1;
		else
			if (*wcs == (wchar_t)L'p')
	    		rptr->print = 1;
			else {
				warn(MSGSTR(RP_NOADD, "Replacement string not added"),
	    			MSGSTR(RP_TRLR, "Invalid trailing RE option"));
				goto quit;
			}
    }

	if ((highbkref = chk_back_ref(wptr)) == -1) {
		/* ERROR-Invalid back references */
		warn(MSGSTR(RP_NOADD, "Replacement string not added"),
	     	MSGSTR(RP_BADREF, "Invalid RE backreference(s)"));
		goto quit;
	}

    /* Now old points to 'old' and new points to 'new' and both
	 * are '\0' terminated */
    if ((retval = regcomp(&rptr->comp, old, REG_NEWLINE)) != 0) {
		regerror(retval, &rptr->comp, old, sizeof(old));
		warn(MSGSTR(RP_NOADD, "Replacement string not added"),old);
		goto quit;
    }

	if (rptr->comp.re_nsub < highbkref) {
	warn(MSGSTR(RP_NOADD, "Replacement string not added"),
	     MSGSTR(RP_BADREF, "Invalid RE backreference(s)"));
	goto quit;
	}

    rptr->replace = new;
    if (rplhead == (Replstr *)NULL) {
	rplhead = rptr;
	rpltail = rptr;
    } else {
	rpltail->next = rptr;
	rpltail = rptr;
    }
	return;
quit:
	free(rptr);
	free(new);
	return;
}


/* rpl_name - possibly replace a name with a regular expression
 *
 * DESCRIPTION
 *
 *	The string name is searched for in the list of regular expression
 *	substituions.  If the string matches any of the regular expressions
 *	then the string is modified as specified by the user.
 *
 * PARAMETERS
 *
 *	char	*name	- name to search for and possibly modify
 */

void rpl_name(char *name)
{
	int				i,len,ret,indx;
    int             found;
	int				eflags;
    char           *b;
    char           *p;
	char		   *r;
    Replstr        *rptr;
	regmatch_t		subexp[MAXBKREF];
	/*
	 * please leave buff1 declared before buff here - this declaration allows
	 * us to get away with a little bit of overflow in buff1 before we check
	 * for such and quit
	 */
    char            buff1[PATH_MAX + 1];/* holds the name with substitutions */
    char         	buff[PATH_MAX + 1]; /* holds the original name */

	found = 0;
	eflags = 0;
    strcpy(buff, name);
	r = buff;
	b = buff1;
    for (rptr = rplhead; !found && rptr != NULL; rptr = rptr->next) {
	do {
	    if ((ret = regexec(&rptr->comp,r,(size_t)MAXBKREF,subexp,eflags)) == 0) {

		p = rptr->replace;
		/* output the characters from the beginning of the input string
		 * until we reach the point where the match occurred and make
		 * sure b is null terminated. */
		len = (int)subexp[0].rm_so;
		strncpy(b,r,(size_t)len);
		b += len;

		/* insert the replacement string into our new string in buff1 */
		while (*p) {

			/* if the character isn't a special character then output
			 * otherwise handle accordingly. */
			if ((*p != '\\') && (*p != '&')) {
				len = mblen(p,MB_CUR_MAX);
				do {
					*b++ = *p++;
				}while (--len > 0);
			} else 
				if (*p++ == '\\') {
				/* if the character following a backslash is a digit
				 * then handle the back reference, otherwise output
				 * the character. */

					if ((*p >= '1') && (*p <= '9')) {
						indx = *p++ - '0';
						for(i=subexp[indx].rm_so;i<subexp[indx].rm_eo;i++)
							*b++ = *(r+i);
					}else {
						len = mblen(p, MB_CUR_MAX);
						do
							*b++ = *p++;
						while (--len > 0);
					}
				} else {
					/* handle the case of & */
					for(i=subexp[0].rm_so;i<subexp[0].rm_eo;i++)
						*b++ = *(r+i);
				}
			if (b > (buff1 + PATH_MAX)) {
				fprintf(stderr, MSGSTR(RP_STRLONG, "new name exceeds path limit\n"));
				fprintf(stderr, MSGSTR(RP_NOSUB, "no substitution was done on %s\n"),name);
				ret = -1;
				break;
			}
		}
		found = 1;
		if (ret == 0) {
			eflags |= REG_NOTBOL;
			/* put the rest of the input string into buff1 */
			r += subexp[0].rm_eo;
			strcpy(b,r);
		}
	    } /* if regexec */
	} while ((ret == 0) && rptr->global);
	if (found) {
	    if (rptr->print) {
			if (ret >= 0)
				fprintf(stderr, "%s >> %s\n", name, buff1);
			else
				fprintf(stderr, "%s >> %s\n", name, name);
	    }
		if (ret >= 0)
	    	strcpy(name, buff1);
	}
    } /* for */
}


/* get_disposition - get a file disposition
 *
 * DESCRIPTION
 *
 *	Get a file disposition from the user.  If the user enters 
 *	the locales equivalent of yes the the file is processed, 
 * 	anything else and the file is ignored.
 * 
 * 	The functionality (-y option) is not in 1003.2 as of DRAFT 11 
 * 	but we leave it in just in case.
 * 
 *	If the user enters EOF, then the PAX exits with a non-zero return
 *	status.
 *
 * PARAMETERS
 *
 *	char	*mode	- string signifying the action to be taken on file
 *	char	*name	- the name of the file
 *
 * RETURNS
 *
 *	Returns 1 if the file should be processed, 0 if it should not.
 */


int get_disposition(int mode, char *name)

{
    char	ans[10];
    char	buf[PATH_MAX + 10];
	int		ret=-1;
	char	*ptr;	/* pointer to the prompt in catalog */

    if (f_disposition) {
		ptr = MSGSTR(RP_ADD, "");
		if (*ptr == '\0') {
			if (mode == ADD)
	    		sprintf(buf, "add %s? yes or no : ", name);
			else if (mode == EXTRACT)
	    		sprintf(buf, "extract %s? yes or no : ",name);
			else	/* pass */
	    		sprintf(buf, "pass %s? yes or no : ",name);

			if (nextask(buf, ans, sizeof(ans)) == -1)
	    		exit(1);

			if ((strcmp(ans,"yes") == 0) || (strcmp(ans,"y") == 0))
				return(0);
			else
				return(1);
		} else {
			if (mode == ADD)
	    		sprintf(buf, MSGSTR(RP_ADD, "add %s? "), name);
			else if (mode == EXTRACT)
	    		sprintf(buf, MSGSTR(RP_EXTRACT, "extract %s? "), name);
			else	/* pass */
	    		sprintf(buf, MSGSTR(RP_PASS, "pass %s? "), name);

			if (nextask(buf, ans, sizeof(ans)) == -1)
	    		exit(1);

			if ( (strlen(ans) == 0) || (rpmatch(ans) != 1) )
	    		return(1);
			else
				return(0);
		}
    }
    return(0);
}


/* get_newname - prompt the user for a new filename
 *
 * DESCRIPTION
 *
 *	The user is prompted with the name of the file which is currently
 *	being processed.  The user may choose to rename the file by
 *	entering the new file name after the prompt; the user may press
 *	carriage-return/newline, which will skip the file or the user may
 *	type an 'EOF' character, which will cause the program to stop.
 * 	The user can enter a single period ('.') and the name of the
 * 	file will remain unchanged.
 *
 * PARAMETERS
 *
 *	char	*name		- filename, possibly modified by user
 *	int	size		- size of allowable new filename
 *
 * RETURNS
 *
 *	Returns 0 if successfull, or -1 if an error occurred.
 *
 */


int get_newname(char *name, int size)

{
    char	buf[PATH_MAX + 10];
    char	orig_name[PATH_MAX + 1];

    if (f_interactive) {
	sprintf(buf, MSGSTR(RP_RENAME, "rename %s? "), name);
	strcpy(orig_name, name);
	if (nextask(buf, name, size) == -1) {
	    exit(1);	/* EOF */
	}
	if (strlen(name) == 0) {
	    return(1);
	}
	if ((name[0] == '.') && (name[1] == '\0'))
	    strcpy(name, orig_name);	/* leave the name the same */
    }
    return(0);
}
