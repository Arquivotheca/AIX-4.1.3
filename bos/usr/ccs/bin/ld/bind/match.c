#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)98	1.7  src/bos/usr/ccs/bin/ld/bind/match.c, cmdld, bos411, 9428A410j 1/21/94 09:00:39")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: match
 *
 *   STATIC FUNCTIONS:
 *		make_call
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "global.h"
#include "bind.h"
#include "error.h"
#include "strs.h"
#include "match.h"

/* Forward declaration */
static int make_call(STR *, int);

/***************************************************************************
 * Name:	match
 *
 * Purpose:	Given a pattern, call a given function for each STR matching
 *		the pattern. The '*' is the only special character, and
 *		matches 0 or more characters in a name.  A matched name is
 *		expected to be a real symbol, so STR_ISSYMBOL is checked.
 *
 *		If pattern is NULL, the buffer "buf" is freed.  In this case,
 *		add_flag and f are ignored.
 *
 * INPUTS:	add_flag: (see match.h)
 *		f: The function to call for names that match.
 *
 * Results:	Returns any non-RC_OK error code from the function that is
 *		called.
 *		RC_NI_WARNING: if regcomp() fails.
 *		RC_OK: Otherwise.
 * *************************************************************************/
RETCODE
match(char *pattern,
      enum match_flag add_flag,
      int match_scope,
      int (*f)(STR *))			/* Function to call */
{
    static char	*id = "match";
    static char *buf = NULL;
    static int	buflen = 0;
    char	*bufp;
    char	*save_pattern;
    char	errbuf[512];
    int		dot_name;
    int		dot_star;
    int		i, len, rc;
    int		match_found;
    int		regexec_called;
    HASH_STR	*sroot, *sh;
    STR		*s, *name;
    regex_t	preg;

    if (pattern == NULL) {
	/* Free buffer */
	if (buf) {
	    efree(buf);
	    buf = NULL;
	    buflen = 0;
	}
	return RC_OK;
    }

#ifdef DEBUG
    if (*pattern == '\0')
	internal_error();
#endif

    /* Check for *, indicating that the pattern is a regular expression. */
    if (strchr(pattern, '*') == NULL) {	/* none */
	switch(add_flag) {
	  case MATCH_WARN_ADD_NEWNAME:
	    /* If the name is not a wild card and we're going to add the name,
	       we do not print a warning if the name doesn't already exist. */
	  case MATCH_ADD_NEWNAME:
	    name = putstring(pattern);	/* Find or add name */
	    name->flags |= STR_ISSYMBOL; /* Added name is a symbol */
	    return f(name);		/* Function always called. */
	    /*NOTREACHED*/
	    break;
	  case MATCH_WARN_NO_NEWNAME:
	    name = lookup_stringhash(pattern); /* Look for name */
	    if ((name == NULL || !(name->flags & STR_ISSYMBOL))) {
		bind_err(SAY_NORMAL, RC_NI_WARNING,
			 NLSMSG(MATCH_NONE1,
		"%1$s: 0711-678 WARNING: No symbols match pattern: %2$s"),
			 Main_command_name, pattern);
		return RC_OK;		/* No match */
	    }
	    break;
	  case MATCH_NO_NEWNAME:
	    name = lookup_stringhash(pattern); /* Look for name */
	    break;
	}

	if (name != NULL && (name->flags & STR_ISSYMBOL)
	    && make_call(name, match_scope))
	    return f(name);
	/* No matches */
	return RC_OK;
    }

    save_pattern = pattern;

    /* We don't actually search for names beginning with a single '.', since
       each STR represents both dotted and undotted names.  Therefore, we
       massage the pattern if it begins with a dot.  */
    dot_star = 0;
    switch (*pattern) {
      case '*':
	dot_name = 0;			/* Dotted and undotted names are OK */
	break;
      case '.':
	if (pattern[1] == '*') {
	    /* Pattern begins with ".*", which matches any . name or any
	       plain name beginning with "..". */
	    dot_star = 1;
	    pattern++;			/* Remove leading dot */
	    break;
	}
	else if (pattern[1] != '.') {
	    pattern++;
	    dot_name = 1;		/* Name must begin with dot. */
	    break;
	}
	/* Else fall through.  Name begins with '..' */
      default:
	dot_name = -1;			/* Name cannot begin with dot */
    }

    len = strlen(pattern);

    /* Allocate and save a buffer that can be used for subsequent match()
       calls.  Note that a single character in "pattern" can result in two
       characters in "buf", and we also add 2 extra characters plus a
       terminating null. */
    if (buflen < 2*len + 3 || buflen > 16 * (2*len + 3)) {
	/* Reallocate buffer if too small or much too big. */
	if (buf)
	    efree(buf);
	/* Allocate buffer twice as big as necessary to provide some slop
	   for future calls.  */
	buf = emalloc(2 * (2*len + 3), id);
	buflen = 2 * (2*len + 3);
    }

    /* Modify the pattern:
       Add ^ to beginning and $ to end.
       Hide the special characters [ and . with backslashes.
       Convert '*' to '.*'.
       Backslashes are stripped, allowing arbitrary patterns.
       A trailing backslash is ignored.
       */

    /* Is NLS processing needed here? */

    for (buf[0] = '^', bufp = &buf[1]; *pattern; ++pattern) {
	switch (*pattern) {
	  case '[':
	  case '.':			/* Escape character for regcomp() */
	    *bufp++ = '\\';
	    break;
	  case '*':			/* add more meaning to '*' */
	    *bufp++ = '.';
	    break;
	  case '\\':			/* don't interpret next character */
	    if (*++pattern == '\0')
		goto add_dollar;	/* Ignore trailing \. */
	    break;
	}
	*bufp++ = *pattern;
    }
  add_dollar:
    *bufp++ = '$';
    *bufp = '\0';

    /* compile pattern */
    rc = regcomp(&preg, buf, REG_NOSUB);
    if (rc != 0) {
	/* Get error--don't worry if it's truncated. */
	(void) regerror(rc, &preg, errbuf, sizeof(errbuf));
	bind_err(SAY_NORMAL,
		 RC_NI_WARNING,
		 NLSMSG(MATCH_BAD_PAT,
		"%1$s: 0711-677 WARNING: regcomp() failed for pattern: %2$s\n"
		 "\t%3$s"),
		 Main_command_name, buf, errbuf);
	return RC_NI_WARNING;
    }

    rc = RC_OK;
    match_found = 0;

    /* Now look at all names for a match */
    if (dot_star == 1) {
	for (sroot = &HASH_STR_root; sroot; sroot = sroot->next)
	    for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
		s = &sh->s;

		/* Check for match with plain name */
		regexec_called = 0;
		if (s->flags & STR_ISSYMBOL) {
		    /* A matching plain name must begin with a '.' (implying
		       that it begins with 2 dots.  The leading '*' in the
		       pattern matches any number of dots. */
		    if (s->name[0] == '.') {
			if (regexec(&preg, &s->name[1], 0, NULL, 0) != 0)
			    continue;		/* Not a match */
			regexec_called = 1;

			if (make_call(s, match_scope)) {
			    if ((rc = f(s)) != RC_OK)
				goto finish_up;
			    else
				match_found = 1;
			}
		    }
		}

		/* Check for match with name beginning with dot. */
		if (s->alternate != NULL
		    && (s->alternate->flags & STR_ISSYMBOL)) {
		    if (regexec_called == 0) /* regexec already called */
			if (regexec(&preg, s->name, 0, NULL, 0) != 0)
			    continue;
		    if (make_call(s->alternate, match_scope)) {
			if ((rc = f(s->alternate)) != RC_OK)
			    goto finish_up;
			else
			    match_found = 1;
		    }
		}   
	    }
    }
    else {
	for (sroot = &HASH_STR_root; sroot; sroot = sroot->next)
	    for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
		s = &sh->s;

		/* Check for match with plain name */
		if (dot_name <= 0	/* Undotted match is possible. */
		    && (s->flags & STR_ISSYMBOL)) {
		    if (regexec(&preg, s->name, 0, NULL, 0) != 0)
			continue;		/* Not a match */
		    regexec_called = 1;
		    if (make_call(s, match_scope)) {
			if ((rc = f(s)) != RC_OK)
			    goto finish_up;
			else
			    match_found = 1;
		    }
		}
		else
		    regexec_called = 0;

		/* Check for match with name beginning with dot. */
		if (dot_name >= 0	/* Dotted match is possible. */
		    && s->alternate
		    && (s->alternate->flags & STR_ISSYMBOL)) {
		    if (regexec_called == 0)	/* regexec already called */
			if (regexec(&preg, s->name, 0, NULL, 0) != 0)
			    continue;
		    if (make_call(s->alternate, match_scope)) {
			if ((rc = f(s->alternate)) != RC_OK)
			    goto finish_up;
			else
			    match_found = 1;
		    }
		}
	    }
    }

    if (match_found == 0)
	switch(add_flag) {
	  case MATCH_WARN_ADD_NEWNAME:
	    bind_err(SAY_NORMAL, RC_NI_WARNING,
		     NLSMSG(MATCH_NONE2,
		    "%1$s: 0711-679 WARNING: Symbol %2$s does not exist."),
		     Main_command_name, save_pattern);
	    break;
	  case MATCH_WARN_NO_NEWNAME:
	    bind_err(SAY_NORMAL, RC_NI_WARNING,
		     NLSMSG(MATCH_NONE1,
		    "%1$s: 0711-678 WARNING: No symbols match pattern: %2$s"),
		     Main_command_name, save_pattern);
	    break;
	}
  finish_up:
    regfree(&preg);			/* Free regcomp() areas. */
    return rc;
} /* match */
/***************************************************************************
 * Name:	make_call
 *
 * Purpose:	Given a name and the scope flags, determine whether the
 *		function parameter should be called for the name.  The name
 *		has already matched the pattern and the ISSYMBOL is set for
 *		the name.
 *
 * Results:	1 if function should be called.
 *		0 otherwise.
 *
 * *************************************************************************/
static int
make_call(STR *s,
	  int scope)
{
    SYMBOL *sym;

    if (scope & MATCH_ANY)
	return 1;
    if (scope & MATCH_EXT) {
	for (sym = s->first_ext_sym; sym; sym = sym->s_synonym) {
	    if (!(sym->s_flags & S_DUPLICATE)
		|| ((sym->s_flags & S_DUPLICATE) && (scope & MATCH_DUP)))
		return 1;
	}
    }
    if (scope & MATCH_HID) {
	for (sym = s->first_hid_sym; sym; sym = sym->s_synonym) {
	    if (!(sym->s_flags & S_DUPLICATE)
		|| ((sym->s_flags & S_DUPLICATE) && (scope & MATCH_DUP)))
		return 1;
	}
    }
    /* ERs are never deleted. */
    if ((scope & MATCH_ER) && s->refs != NULL)
	return 1;
    return 0;
}
