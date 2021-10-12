static char sccsid[] = "@(#)27	1.2  src/bos/etc/security/migration/user_mrg.c, cfgsauth, bos411, 9428A410j 3/4/94 18:32:34";
/*
 *   COMPONENT_NAME: CFGSAUTH
 *
 *   FUNCTIONS: append, clearpeek, do_comments, get41stanza, get_pwr_stanza,
 *		getline, main, peekline, ungetline
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

/* array sizes */
#define	DEFVALNUM	18
#define	ROOTVALNUM	3
#define	DEFCOPYNUM	6
#define	POSCOPYNUM	1

/* st_type values */
#define	OTHER	0
#define	ROOT	1
#define	DEF	2

/* These are the lines which are new to the 4.1 'defaults' stanza */
char *defvals[DEFVALNUM] = {
	"SYSTEM",		"logintimes",		"pwdwarntime",
	"account_locked",	"loginretries",		"histexpire",
	"histsize",		"minage",		"maxage",
	"maxexpired",		"minalpha",		"minother",
	"minlen",		"mindiff",		"maxrepeats",
	"dictionlist",		"pwdchecks",		"admgroups"
};
char *defstrs[DEFVALNUM];
int defcopyflags[DEFVALNUM];

/* These are the lines which are new to the 4.1 'root' stanza */
char *rootvals[ROOTVALNUM] = {
	"SYSTEM",		"loginretries",		"account_locked"
};
char *rootstrs[ROOTVALNUM];

/* These are indices into the defvals list which indicate which values */
/* should be copied from the 3.2 login.cfg if possible.                */
int defcopy[DEFCOPYNUM] = { 7, 8, 10, 11, 13, 14 };

/* These are indices into the defvals list which indicate values that */
/* MIGHT be in any of the 3.2 default: stanzas.                       */
int poscopy[POSCOPYNUM] = { 17 };

int haveline = 0;
char linebuf[BUFSIZ];
FILE *fp41, *fp32, *fp41new, *fp41pwr;
regex_t comment_re, blank_re, stanza_re, root_re, default_re;
regex_t defval_re[DEFVALNUM], rootval_re[ROOTVALNUM];

char *getline(FILE *);

#define peekline()	( haveline )
#define ungetline()	( haveline = 1 )
#define clearpeek()	( haveline = 0 )


char *
getline(FILE *infp)
{
    if (peekline()) {
	clearpeek();
	return linebuf;
    }
    return fgets(linebuf, BUFSIZ, infp);
} /* getline */


do_comments(FILE *infp, FILE *outfp)
{

    while (getline(infp) != NULL) {
	if (!match(&comment_re, linebuf) && !match(&blank_re, linebuf))
	    break;
	if (outfp)
	    fputs(linebuf, outfp);
    }
    if (!match(&comment_re, linebuf) && !match(&blank_re, linebuf))
	    ungetline();
} /* do_comments */


get_pwr_stanza()
{
    int i;

    while (getline(fp41pwr) != NULL) {
	for(i=0; i<DEFCOPYNUM; i++) {
	    if (match(&(defval_re[defcopy[i]]), linebuf)) {
		strcpy(defstrs[defcopy[i]], linebuf);
		break;
	    }
	}
    }
} /* get_pwr_stanza */


get41stanza(char *st_name, char *vals[], char *strs[], regex_t val_re[], int valnum)
{
    int i;
    char *str;
    regex_t st_re;

    sprintf(linebuf, "^%s[[:space:]]*:[[:space:]]*$", st_name);
    prep_match(&st_re, linebuf);
    for(i=0; i<valnum; i++) {
	sprintf(linebuf, "^[[:space:]]*%s[[:space:]]*=", vals[i]);
	prep_match(&(val_re[i]), linebuf);
    }
    rewind(fp41);
    while (getline(fp41) != NULL) {
	if (match(&st_re, linebuf))
	    break;
    }

    if (!match(&st_re, linebuf)) {
	fprintf(stderr, "ERROR: '%s' stanza not found.\n", st_name);
	exit(1);
    }

    for(i=0; i<valnum; i++) {
	strs[i] = (char *) malloc(LINE_MAX);
	if (strs[i] == NULL) {
	    fprintf(stderr, "ERROR: out of memory.\n");
	    exit(1);
	}
	strs[i][0] = '\0';
    }

    while (getline(fp41) != NULL) {
	if (match(&blank_re, linebuf))
	    break;
	for(i=0; i<valnum; i++) {
	    if (match(&(val_re[i]), linebuf)) {
		strcpy(strs[i], linebuf);
		break;
	    }
	}
    }

    for(i=0; i<valnum; i++) {
	if (strs[i][0] == '\0') {
	    fprintf(stderr, "ERROR: '%s' stanza line '%s' not found.\n",
		st_name, vals[i]);
	    exit(1);
	}
    }
} /* get41stanza */


append(FILE *outfp, char *strs[], int valnum, int *copyflags)
{
    int i;

    for(i=0; i<valnum; i++) {
	if (copyflags) {
	    if (copyflags[i])
		fputs(strs[i], outfp);
	} else
	    fputs(strs[i], outfp);
    }
} /* append */


main()
{
    int i, firststanza, st_type;

/* Initialize & open files */
    check_mig();
    set_mig_dir();

    unlink(NEWUSER41);
    fp41new = getfile(NEWUSER41, "w");
    fp32 = getfile(USER32, "r");
    fp41 = getfile(USER41, "r");
    fp41pwr = getfile(PWRESTSAVE, "r");

    prep_match(&comment_re, "^\\*");
    prep_match(&blank_re, "^[[:space:]]*$");
    prep_match(&stanza_re, "^[^*[:space:]].*:[[:space:]]*$");
    prep_match(&default_re, "^default[[:space:]]*:[[:space:]]*$");
    prep_match(&root_re, "^root[[:space:]]*:[[:space:]]*$");

/* Read/copy 4.1 comments */
    do_comments(fp41, fp41new);
    clearpeek();

/* Read 'default' stanza */
    get41stanza("default", defvals, defstrs, defval_re, DEFVALNUM);

/* Read 'pw_restrictions' stanza */
    get_pwr_stanza();

/* Read 'root' stanza */
    get41stanza("root", rootvals, rootstrs, rootval_re, ROOTVALNUM);

/* Read 3.2 comments */
    do_comments(fp32, NULL);

/* Process 3.2 stanzas */
    st_type = OTHER;
    firststanza = 1;
    while (getline(fp32) != NULL) {

	/* STANZA */
	if (match(&stanza_re, linebuf)) {
	    if (firststanza) {
		firststanza = 0;
		if (!match(&default_re, linebuf)) {
		    fprintf(fp41new, "default:\n");
		    append(fp41new, defstrs, DEFVALNUM, NULL);
		    fprintf(fp41new, "\n");
		}
	    }

	    if (match(&default_re, linebuf)) {
		st_type = DEF;
		for(i=0; i<DEFVALNUM; i++)
		    defcopyflags[i] = 1;
	    } else if (match(&root_re, linebuf))
		st_type = ROOT;
	    else
		st_type = OTHER;
	} else {
	    if (st_type == DEF) {
		for(i=0; i<POSCOPYNUM; i++) {
		    if (match(&(defval_re[poscopy[i]]), linebuf))
			defcopyflags[poscopy[i]] = 0;
		}
	    }
	}

	/* BLANK (end of stanza) */
	if (match(&blank_re, linebuf)) {
	    if (st_type == ROOT)
		append(fp41new, rootstrs, ROOTVALNUM, NULL);
	    if (st_type == DEF)
		append(fp41new, defstrs, DEFVALNUM, defcopyflags);
	    st_type = OTHER;
	}

	/* PRINT LINE */
	fputs(linebuf, fp41new);
    }
    unlink(PWRESTSAVE);
    unlink(USER32);
    rename(NEWUSER41, USER32);
    exit(0);
}
