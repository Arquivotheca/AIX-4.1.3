static char sccsid[] = "@(#)60	1.1  src/bos/etc/security/migration/mrg_util.c, cfgsauth, bos411, 9428A410j 2/18/94 13:47:43";
/*
 *   COMPONENT_NAME: CFGSAUTH
 *
 *   FUNCTIONS: getfile
 *		match
 *		prep_match
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

#include <stdio.h>
#include <regex.h>
#include "mrg_util.h"

set_mig_dir()
{
    if (chdir(MIGDIR) < 0) {
	perror("ERROR: Cannot change to migration directory");
	exit(1);
    }
} /* set_mig_dir */


check_mig()
{
    char *level;

    level = getenv("BOSLEVEL");
    if (level == NULL) {
	fprintf(stderr, "ERROR: 'BOSLEVEL' variable not defined.\n");
	exit(1);
    }

    /* If not a 3.2 migration, then we are done. */
    if (strstr(level, "3.2") == NULL)
	exit(0);
} /* check_mig */


FILE*
getfile(char *fname, char *mode)
{
    FILE *fp;
    fp = fopen(fname, mode);
    if (fp == NULL) {
	fprintf(stderr, "ERROR: Could not open '%s'.\n", fname);
	exit(1);
    }
    return fp;
}/* getfile */


prep_match(regex_t *re, char *pattern)
{
    int rc;
    char errbuf[BUFSIZ];

    if (rc = regcomp(re, pattern, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)) {
	regerror(rc, re, errbuf, BUFSIZ);
	fprintf(stderr, "regcomp: %s\n", errbuf);
	exit(1);
    }
} /* prep_match */


match(regex_t *re, char *string)
{
    int rc, len, repl;
    char errbuf[BUFSIZ];

    repl = 0;
    len = strlen(string);
    if ((len > 0) && (string[len-1] == '\n')) {
	string[len-1] = '\0';
	repl = 1;
    }
    if (rc = regexec(re, string, 0, NULL, 0)) {
	if (rc == REG_NOMATCH) {
	    if (repl) string[len-1] = '\n';
	    return 0;
	}
	regerror(rc, re, errbuf, BUFSIZ);
	fprintf(stderr, "regexec: %s\n", errbuf);
	exit(1);
    }
    if (repl) string[len-1] = '\n';
    return 1;
} /* match */

