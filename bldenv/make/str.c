/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: Str_Match
 *		_string_headtail
 *		_string_prefsuff
 *		brk_string
 *		string_archmemb
 *		string_concat
 *		string_create
 *		string_deref
 *		string_finish
 *		string_flatten
 *		string_init
 *		string_ref
 *		string_setup
 *		strndup
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: str.c,v $
 * Revision 1.2.2.8  1992/12/03  19:07:19  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:36:34  damon]
 *
 * Revision 1.2.2.7  1992/11/13  15:19:56  root
 * 	Changed assignment of NULL to '\0'
 * 	[1992/11/13  14:57:58  root]
 * 
 * Revision 1.2.2.6  1992/09/24  19:27:12  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:55:40  gm]
 * 
 * Revision 1.2.2.5  1992/06/24  16:41:54  damon
 * 	CR 60. brk_string now counts args correctly
 * 	[1992/06/24  16:38:41  damon]
 * 
 * Revision 1.2.2.4  1992/06/16  21:24:36  damon
 * 	2.1.1 touch-up
 * 	[1992/06/16  21:18:22  damon]
 * 
 * Revision 1.2.2.3  1992/06/12  00:49:18  damon
 * 	Synched with 2.1.1
 * 	[1992/06/12  00:38:50  damon]
 * 
 * Revision 1.2.4.3  1992/03/25  22:46:03  damon
 * 	Removed comment after endif
 * 	[1992/03/25  21:49:50  damon]
 * 
 * Revision 1.2.4.2  1992/03/09  21:10:41  mhickey
 * 	Closed open comment in brk_string that was preventing
 * 	leading whitespace from being stripped from command lines.
 * 	[1992/03/09  21:01:44  mhickey]
 * 
 * Revision 1.2  1991/12/05  20:45:15  devrcs
 * 	Changes for Reno make
 * 	[91/03/22  16:10:27  mckeen]
 * 
 * $EndLog$
 */
/*-
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)52  1.4  src/bldenv/make/str.c, bldprocess, bos412, GOLDA411a 1/19/94 16:31:37";
#endif /* not lint */

#ifndef lint
static char     rcsid[] = "@(#)str.c	5.8 (Berkeley) 6/1/90";
#endif
				/* not lint */

#include "make.h"
#include "hash.h"

Hash_Table stringHashTable;

/*-
 * strndup --
 *	allocate and return a copy of the string s of length n.
 *
 * returns --
 *	the resulting string in allocated space.
 */
char *
strndup(const char *s, int n)
{
	char *r = (char *)malloc(n+1);
	if (r == NULL)
	    enomem();
	memcpy(r, s, n);
	r[n] = '\0';
	return(r);
}

/*-
 * string_concat --
 *	concatenate the two strings, inserting a space or slash between them,
 *	freeing them if requested.
 *
 * returns --
 *	the resulting string in allocated space.
 */
string_t
string_concat(string_t s1, string_t s2, int flags)
{
	register int len1, len2;
	register char *result;
	string_t rs;

	/* get the length of both strings */
	len1 = s1 ? s1->len : 0;
	len2 = s2 ? s2->len : 0;

 	/* allocate length plus separator plus EOS */
	result = emalloc((u_int)(len1 + len2 + 2));

	/* copy first string into place */
	if (len1)
	    memcpy(result, s1->data, len1);

	if (len1 && len2) {
	    /* add separator character */
	    if (flags & STR_ADDSPACE) {
		    result[len1] = ' ';
		    ++len1;
	    } else if (flags & STR_ADDSLASH) {
		    result[len1] = '/';
		    ++len1;
	    }
	}

	/* copy second string into place */
	if (len2)
	    memcpy(result + len1, s2->data, len2);

	result[len1+len2] = '\0';

	rs = string_create(result);

	/* free original strings */
	free(result);
	if (flags & STR_DOFREE) {
		string_deref(s1);
		string_deref(s2);
	}

	return(rs);
}

/*-
 * brk_string --
 *	Fracture a string into an array of words (as delineated by tabs or
 *	spaces) taking quotation marks into account.  Leading tabs/spaces
 *	are ignored.
 *
 * returns --
 *	Pointer to the array of pointers to the words.  To make life easier,
 *	the first word is always the value of the MAKE variable.
 */
char **
brk_string(register char *str, int *store_argc)
{
	static int argmax, curlen;
	static const char **argv;
	static char *buf;
	register int argc, ch;
	register char inquote, *p, *start, *t;
	int len;

	/* save off pmake variable */
	if (!argv) {
		argv = (const char **)emalloc((argmax = 50) * sizeof(char *));
		argv[0] = Var_Value(sMAKE, VAR_GLOBAL);
	}

	/* skip leading space chars. */
	for (; *str == ' ' || *str == '\t'; ++str);

	/* allocate room for a copy of the string */
	if ((len = strlen(str) + 1) > curlen)
		buf = emalloc(curlen = len);

	/*
	 * copy the string; at the same time, parse backslashes,
	 * quotes and build the argument list.
	 */
	argc = 1;
	inquote = '\0';
	for (p = str, start = t = buf;; ++p) {
		switch(ch = *p) {
		case '"':
		case '\'':
			if (inquote)
				if (inquote == ch)
					inquote = '\0';
				else
					break;
			else
				inquote = ch;
			continue;
		case ' ':
		case '\t':
			if (inquote)
				break;
                        for (; *p == ' ' || *p == '\t'; ++p);
                        ch = *p--;
			if (!start)
				continue;
			/* FALLTHROUGH */
		case '\n':
		case '\0':
			/*
			 * end of a token -- make sure there's enough argv
			 * space and save off a pointer.
			 */
			*t++ = '\0';
			if (argc == argmax) {
				argmax *= 2;		/* ramp up fast */
				if (!(argv = (const char **)realloc(argv,
				    argmax * sizeof(char *))))
				enomem();
			}
			argv[argc++] = start;
			start = (char *)NULL;
			if (ch == '\n' || ch == '\0')
				goto done;
			continue;
		case '\\':
			switch (ch = *++p) {
			case '\0':
			case '\n':
				/* hmmm; fix it up as best we can */
				ch = '\\';
				--p;
				break;
			case 'b':
				ch = '\b';
				break;
			case 'f':
				ch = '\f';
				break;
			case 'n':
				ch = '\n';
				break;
			case 'r':
				ch = '\r';
				break;
			case 't':
				ch = '\t';
				break;
			}
			break;
		}
		if (!start)
			start = t;
		*t++ = ch;
	}
done:	argv[argc] = (char *)NULL;
	*store_argc = argc;
	return((char **)argv);
}

/*
 * Str_Match --
 * 
 * See if a particular string matches a particular pattern.
 * 
 * Results: Non-zero is returned if string matches pattern, 0 otherwise. The
 * matching operation permits the following special characters in the
 * pattern: *?\[] (see the man page for details on what these mean).
 * 
 * Side effects: None.
 */
int
Str_Match(
	register const char *string,		/* String */
	register const char *pattern)		/* Pattern */
{
	int c2;

	for (;;) {
		/*
		 * See if we're at the end of both the pattern and the
		 * string. If, we succeeded.  If we're at the end of the
		 * pattern but not at the end of the string, we failed.
		 */
		if (*pattern == 0)
			return(!*string);
		if (*string == 0 && *pattern != '*')
			return(0);
		/*
		 * Check for a "*" as the next pattern character.  It matches
		 * any substring.  We handle this by calling ourselves
		 * recursively for each postfix of string, until either we
		 * match or we reach the end of the string.
		 */
		if (*pattern == '*') {
			pattern += 1;
			if (*pattern == 0)
				return(1);
			while (*string != 0) {
				if (Str_Match(string, pattern))
					return(1);
				++string;
			}
			return(0);
		}
		/*
		 * Check for a "?" as the next pattern character.  It matches
		 * any single character.
		 */
		if (*pattern == '?')
			goto thisCharOK;
		/*
		 * Check for a "[" as the next pattern character.  It is
		 * followed by a list of characters that are acceptable, or
		 * by a range (two characters separated by "-").
		 */
		if (*pattern == '[') {
			++pattern;
			for (;;) {
				if ((*pattern == ']') || (*pattern == 0))
					return(0);
				if (*pattern == *string)
					break;
				if (pattern[1] == '-') {
					c2 = pattern[2];
					if (c2 == 0)
						return(0);
					if ((*pattern <= *string) &&
					    (c2 >= *string))
						break;
					if ((*pattern >= *string) &&
					    (c2 <= *string))
						break;
					pattern += 2;
				}
				++pattern;
			}
			while ((*pattern != ']') && (*pattern != 0))
				++pattern;
			goto thisCharOK;
		}
		/*
		 * If the next pattern character is '/', just strip off the
		 * '/' so we do exact matching on the character that follows.
		 */
		if (*pattern == '\\') {
			++pattern;
			if (*pattern == 0)
				return(0);
		}
		/*
		 * There's no special character.  Just make sure that the
		 * next characters of each string match.
		 */
		if (*pattern != *string)
			return(0);
thisCharOK:	++pattern;
		++string;
	}
}

/*-
 * string_flatten - Flatten out a pathname
 */
string_t
string_flatten(string_t spath)
{
	register char *p, *q, *r;
	register char **sp, **tp;
	char *stack[64];
	char *path = strdup(spath->data);
	string_t result;

	p = q = path;
	sp = tp = stack;
	if (*q == '/') {
		p++;
		while (*++q == '/')
			;
	}
	for (r = q; *r; r = q) {
		while (*++q && *q != '/')
			;
		if (q != r+1 || *r != '.') {
			if (q != r+2 || *r != '.' || *(r+1) != '.')
				*sp++ = r;
			else if (sp != tp)
				sp--;
			else {
				*p++ = '.';
				*p++ = '.';
				if (*q)
					*p++ = '/';
			}
		}
		while (*q == '/')
			q++;
	}
	while (tp < sp)
		for (q = *tp++; *q; )
			if ((*p++ = *q++) == '/')
				break;
	if (p > path+1 && *(p-1) == '/')
		--p;
	*p = 0;

	result = string_create(path);
	string_deref(spath);

	return(result);
}

string_t
string_ref(string_t s)
{
    if (s->_refCount == 0)
	printf("string %s had zero refs\n", s->data);
    else if (s->_refCount < 0) {
	printf("string %s had negative refs\n", s->data);
	s->_refCount = 0;
    }
    s->_refCount += 1;
    return s;
}

void
string_deref(string_t s)
{
    register Hash_Entry *he;

    s->_refCount -= 1;
    if (s->_refCount == 0) {
	if (s->_flags & STR_HEADTAIL) {
	    string_deref(s->_head);
	    string_deref(s->_tail);
	}
	if (s->_flags & STR_PREFSUFF) {
	    string_deref(s->_pref);
	    string_deref(s->_suff);
	}
	if (s->_arch)
	    string_deref(s->_arch);
	if (s->_memb)
	    string_deref(s->_memb);
	he = Hash_FindEntry(&stringHashTable, s);
	Hash_DeleteEntry(&stringHashTable, he);
	free((void *)s->data);
	free(s);
    } else if (s->_refCount < 0)
	printf("string %s negative refs\n", s->data);
}

static void
string_setup(string_t s)
{
    register unsigned h;
    register const char *p;

    h = 0;
    for (p = s->data; *p; p++)
	h = (h << 5) - h + *p;
    s->hashval = h;
    s->len = p - s->data;
}

static void
string_finish(string_t s)
{
    register const char *cp, *p, *tail, *suff;
    register int f;

    f = 0;
    cp = s->data;
    tail = NULL;
    suff = NULL;
    p = cp + s->len;
    while (p > cp) {
	p--;
	if (*p == '/') {
	    tail = p + 1;
	    break;
	}
	if (suff == NULL && *p == '.') {
	    suff = p;
	    continue;
	}
	if (*p == '$')
	    f |= STR_HASVAR;
    }
    if (f == 0 && *cp == '$')
	f |= STR_HASVAR;
    else
	while (f == 0 && p > cp)
	    if (*--p == '$')
		f |= STR_HASVAR;
    s->_flags = f;
    s->_tail = (string_t) tail;
    s->_pref = (string_t) (tail ? tail : cp);
    s->_suff = (string_t) suff;
}

string_t
string_create(const char *cp)
{
    struct string strtmpl;
    static struct string strzero;
    string_t s;

    strtmpl = strzero;
    strtmpl.data = cp;
    string_setup(&strtmpl);
    s = Hash_FindString(&stringHashTable, &strtmpl);
    if (s != (string_t) NULL)
	return(string_ref(s));
    s = (string_t) malloc(sizeof(struct string));
    if (s == NULL)
	enomem();
    *s = strtmpl;
    s->data = (const char *)strndup(cp, s->len);
    string_finish(s);
    s->_refCount = 1;
    Hash_CreateString(&stringHashTable, s);
    return(s);
}

void
string_init(void)
{
    Hash_InitTable(&stringHashTable, 1024);
}

void
_string_headtail(string_t s)
{
    const char *cp = s->data;
    const char *p;

    p = (const char *) s->_tail;
    if (p == NULL) {
	s->_head = (string_t) NULL;
	s->_tail = string_ref(s);
    } else {
	s->_tail = string_create(p--);
	if (p == cp && *(p + 1) == '\0')
	    s->_head = string_ref(s);
	else if (p == cp)
	    s->_head = string_create("/");
	else {
	    p = (const char *)strndup(cp, p - cp);
	    s->_head = string_create(p);
	    free((char *)p);
	}
    }
    s->_flags |= STR_HEADTAIL;
}

void
_string_prefsuff(string_t s)
{
    const char *cp, *p;

    cp = (const char *) s->_pref;
    p = (const char *) s->_suff;
    if (p == NULL ||
	(*p == '\0' &&
	 (p == cp + 1 ||
	  (p == cp + 2 && *(p-2) == '.')))) {
	s->_pref = string_create((const char *) s->_pref);
	s->_suff = (string_t) NULL;
    } else {
	if (p == cp) {
	    s->_suff = string_ref(s);
	    s->_pref = string_ref(sNULL);
	} else {
	    s->_suff = string_create(p);
	    p = (const char *)strndup(cp, p - cp);
	    s->_pref = string_create(p);
	    free((char *)p);
	}
    }
    s->_flags |= STR_PREFSUFF;
}

void
string_archmemb(string_t s, string_t arch, string_t memb)
{

    s->_arch = string_ref(arch);
    s->_memb = string_ref(memb);
}
