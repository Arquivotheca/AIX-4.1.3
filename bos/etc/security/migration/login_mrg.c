static char sccsid[] = "@(#)25	1.3  src/bos/etc/security/migration/login_mrg.c, cfgsauth, bos411, 9428A410j 6/6/94 13:50:06";
/*
 *   COMPONENT_NAME: CFGSAUTH
 *
 *   FUNCTIONS: main
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

FILE* getfile(char *, char *);

#define USW_RE_STR	"^usw[[:space:]]*:[[:space:]]*$"
#define PWREST_RE_STR	"^pw_restrictions[[:space:]]*:[[:space:]]*$"

main()
{
    int len, hflag;
    char linebuf[BUFSIZ], shells[BUFSIZ], logtime[BUFSIZ];
    FILE *fp41, *fp32, *fp41new, *fptmp;
    regex_t logtime_re, shell_re, stanza_re, herald_re, usw_re;

/* Initialize & open files */
    check_mig();
    set_mig_dir();

    unlink(NEWLOGIN41);
    unlink(TMPFILE);
    fp41new = getfile(NEWLOGIN41, "w");
    fp32 = getfile(LOGIN32, "r");
    fp41 = getfile(LOGIN41, "r");
    prep_match(&logtime_re, "^[[:space:]]*logintimeout[[:space:]]*=");
    prep_match(&herald_re, "^[[:space:]]*herald[[:space:]]*=");
    prep_match(&shell_re, "^[[:space:]]*shells[[:space:]]*=");
    prep_match(&usw_re, USW_RE_STR);
    prep_match(&stanza_re, "^[^*[:space:]].*:[[:space:]]*$");

/* Read up to "usw:" */
    while (fgets(linebuf, BUFSIZ, fp41) != NULL) {
	if (match(&usw_re, linebuf))
	    break;
	fputs(linebuf, fp41new);
    }
    if (!match(&usw_re, linebuf)) {
	fprintf(stderr, "ERROR: 'usw:' line not found.\n");
	exit(1);
    }

/* Grab 3.2 "usw:" and "pw_restrictions:" stanzas */
    sprintf(linebuf, "/usr/bin/grep -p \"%s\" %s > %s",
	USW_RE_STR, LOGIN32, TMPFILE);
    system(linebuf);
    sprintf(linebuf, "/usr/bin/grep -p \"%s\" %s > %s",
	PWREST_RE_STR, LOGIN32, PWRESTSAVE);
    system(linebuf);

/* Get 4.1 usw fields */
    while (fgets(linebuf, BUFSIZ, fp41) != NULL) {
	if (match(&logtime_re, linebuf))
	    strcpy(logtime, linebuf);
	if (match(&shell_re, linebuf))
	    strcpy(shells, linebuf);
    }
    fclose(fp41);

/* Process 3.2 usw fields */
    fptmp = getfile(TMPFILE, "r");
    while (fgets(linebuf, BUFSIZ, fptmp) != NULL) {
	if (match(&usw_re, linebuf)) {
	    fputs(linebuf, fp41new);
	    fputs(shells, fp41new);
	    fputs(logtime, fp41new);
	} else if (match(&logtime_re, linebuf))
	    ; /* do nothing, replaced w/4.1 version */
	else if (match(&shell_re, linebuf)) {
	    fprintf(fp41new, "* The following line was the 3.2 shell list.\n");
	    fprintf(fp41new, "* %s", linebuf);
	} else
	    fputs(linebuf, fp41new);
    }
    fclose(fptmp);

/* Remove 3.2 "usw:" and "pw_restrictions:" stanzas */
    unlink(TMPFILE);
    sprintf(linebuf, "/usr/bin/egrep -vp \"%s|%s\" %s > %s",
	USW_RE_STR, PWREST_RE_STR, LOGIN32, TMPFILE);
    system(linebuf);

/* Append Remainder of 3.2 stanzas */
    fptmp = getfile(TMPFILE, "r");
    hflag = 0;
    while (fgets(linebuf, BUFSIZ, fptmp) != NULL) {
	if (match(&stanza_re, linebuf)) {
	    if (strstr(linebuf, "defport") || strstr(linebuf, "/dev/console"))
		hflag = 1;
	    else
		hflag = 0;
	}
	if (hflag && match(&herald_re, linebuf))
	    fprintf(fp41new, "* %s", linebuf);
	else
	    fputs(linebuf, fp41new);
    }
    fclose(fptmp);
    unlink(TMPFILE);
    unlink(LOGIN32);
    rename(NEWLOGIN41, LOGIN32);
    exit(0);
}

