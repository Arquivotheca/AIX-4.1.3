static char sccsid[] = "@(#)51	1.3  src/bos/usr/bin/pax/options.c, cmdarch, bos412, 9446B 11/11/94 21:54:12";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: pax
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * DESCRIPTION
 *
 *	These routines provide for option parsing as required by the
 *	[-o options] option of pax.
 *
 */

/* Headers */

#include "pax.h"

/*
 * parse_opts - Parse options string.
 *
 * DESCRIPTION
 *
 *	Parse "options" string of the form:
 * 
 *	keyword[=value][,keyword[=value], ...]
 *
 * PARAMETERS
 *
 *	opts - the string to be parsed
 */

void parse_opts(char *opts)
{
    char *value, *keyword;

    while(get_next_opt(&opts, &keyword, &value)) {
	if (!strcmp(keyword, "datastream")) {		/*** DATASTREAM ***/
	    if (!value)
		fatal(MSGSTR(OPT_NOVAL, "Keyword specified requires a value"));
	    f_datastream = 1;
	    strcpy(datastr_name, value);
	} else if (!strcmp(keyword, "datastr_size")) {	/*** DATASTR_SIZE ***/
	    if (!value)
		fatal(MSGSTR(OPT_NOVAL, "Keyword specified requires a value"));
	    if ((datastr_size = pax_optsize(value)) <= 0)
		fatal(MSGSTR(OPT_SIZE, "Bad size value specified"));
	} else if (!strcmp(keyword, "file_count")) {	/*** FILE_COUNT ***/
	    f_file_count=1;
	    f_file_count_incr=0;
	    if (value)
		f_file_count_incr=atoi(value);
	    if(f_file_count_incr<1)
		f_file_count_incr=100;
	} else {					/*** BAD OPTION ***/
	    warn(keyword, MSGSTR(OPT_NOSUCH, "No such option"));
	}
    }
}

/*
 * get_next_opt - Get next keyword and optional value.
 *
 * DESCRIPTION
 *
 * Returns the next keyword and optional value from "options" string.
 * Also updates the opts variable to point to the next keyword (or the
 * end of the string).
 * 
 * PARAMETERS
 *
 *	opts	- the string to be parsed.
 *	keyword	- pointer to keyword string if found, NULL otherwise.
 *	value	- pointer to value string if found, NULL otherwise.
 *
 *	NOTE: The strings for 'keyword' and 'value' are static, so they
 *            may be overwritten at the next call to get_next_opt().
 *
 * RETURNS
 *
 *	1 if a keyword was found.
 *	0 if no more keywords are left.
 */

static int 
get_next_opt(char **opts, char **keyword, char **value)
{
    char *obuf, *idx;
    static char keybuf[FILENAME_MAX+1], valbuf[PATH_MAX+1];

    obuf = *opts;
    *keyword = NULL;					/* return NULL if */
    *value = NULL;					/* nothing found  */

    while (*obuf == ',')				/* skip extra commas */
	obuf++;
    if (*obuf == '\0') {
	*opts = obuf;
	return 0;
    }

    *keyword = idx = keybuf;				/* keyword exists */
    while(*obuf && (*obuf != ',') && (*obuf != '=')) {
	if ((*obuf == '\\') && (*(obuf+1) == ','))	/* escape comma */
	    obuf++;
	*idx++ = *obuf++;
    }
    *idx = '\0';					/* terminate string */
    *opts = obuf;
    if (*obuf != '=')					/* value specified? */
	return 1;

    obuf++;						/* skip '=' */
    *value = idx = valbuf;				/* value exists */
    while(*obuf && (*obuf != ',')) {
	if ((*obuf == '\\') && (*(obuf+1) == ','))	/* escape comma */
	    obuf++;
	*idx++ = *obuf++;
    }
    *idx = '\0';					/* terminate string */
    *opts = obuf;
    return 1;
}
