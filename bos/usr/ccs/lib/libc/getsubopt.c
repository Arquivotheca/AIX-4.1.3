static char sccsid[] = "@(#)85	1.1  src/bos/usr/ccs/lib/libc/getsubopt.c, libcenv, bos411, 9428A410j 3/4/94 10:29:03";
/*
 *   COMPONENT_NAME: libcenv
 *
 *   FUNCTIONS: getsubopt
 *
 *   ORIGINS: 85
 *
 *                    SOURCE MATERIALS
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#include <stdio.h>
#include <string.h>

/*
 *  Routine: getsubopt
 *
 *  Function: parses suboptions within an option string
 *
 *  Arguments:
 *	**optionp	a pointer to a pointer to the option string
 *	*tokens[]	a pointer to an array of pointers to valid token
 *			strings.
 *	**valuep	a pointer to the location to put the pointer to
 *			the string indicating some value for a suboption.
 *
 *  Returns:
 *	The index into the tokens[] array which matches the suboption, or -1
 *	if there was no match.
 */
int
getsubopt(char **optionp, char * const *tokens, char **valuep) 
{
	int i=0;

	char *equal_sgn;		/* Remembers where '=' in string is */
	char *option;			/* Keeps matching option */
	char *comma;			/* Find end of option */

	if ( !optionp || !*optionp || !tokens ) {
	    				/* Null pointer or pointer to NULL */
	    if (valuep)			/* Be careful */
	      *valuep = NULL;		/* Indicate no value assignment */
	    return (-1);		/* ..and no matches either */
	}

	option = *optionp;		/* Remember where we were */
	comma = strchr(option, ',');	/* Look for end of option */
	if ( !comma )			/* Is this the _last_ option? */
	    *optionp = option +
	      	strlen(option);		/* YES - point at NUL */
	else {				/* NO */
	    *comma++ = '\0';		/* Break apart string */
	    *optionp = comma;		/* Update to start of next option */
	}

	equal_sgn = strchr(option,'=');	/* Check for a key=value */
	if ( equal_sgn ) {		/* Got one */

	    *equal_sgn++ = '\0';	/* Make two separate strings */
	}
	if (valuep) *valuep = equal_sgn; /* Either NULL or value string */

	for(i=0; tokens[i]; i++) { 	/* Search supplied table for a match */
	    if ( !strcmp(option, tokens[i]) )
	      break;			/* Got it! */
	}

	if ( !tokens[i] ) {		/* Ran off the end of the table? */
	    return (-1);		/*  return failure */
	}

	return (i);
}

