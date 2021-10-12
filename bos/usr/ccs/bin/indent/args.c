static char sccsid[] = "@(#)07  1.10  src/bos/usr/ccs/bin/indent/args.c, cmdprog, bos411, 9428A410j 3/16/94 08:40:12";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: eqin, scan_profile, set_defaults, set_option, set_profile
 *
 * ORIGINS: 26 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */


/*
 * Argument scanning and profile reading code.  Default parameters
 * are set here as well.
 */

#include "indent_msg.h"
#include "ind_globs.h"
#include <sys/types.h>
#include <ctype.h>
#include <nl_types.h>
#include <langinfo.h>
#include <mbstr.h>

char *getenv(), *index();

/* profile types */
#define PRO_SPECIAL     1       /* special case */
#define PRO_BOOL        2       /* boolean */
#define PRO_INT         3       /* integer */

/* profile specials for booleans */
#define ON              1       /* turn it on */
#define OFF             0       /* turn it off */

/* profile specials for specials */
#define IGN             1       /* ignore it */
#define CLI             2       /* case label indent (float) */
#define STDIN           3       /* use stdin */
#define KEY             4       /* type (keyword) */

/*
 * N.B.: because of the way the table here is scanned, options
 * whose names are substrings of other options must occur later;
 * that is, with -lp vs -l, -lp must be first.  Also, while (most)
 * booleans occur more than once, the last default value is the
 * one actually assigned.
 */
struct pro {
    char *p_name;               /* name, eg -bl, -cli */
    int   p_type;               /* type (int, bool, special) */
    int   p_default;            /* the default value (if int) */
    int   p_special;            /* depends on type */
    int  *p_obj;                /* the associated variable */
} pro[] = {
    "fa",       PRO_BOOL,       true,   ON,     &flip_assign_oper,
    "nfa",      PRO_BOOL,       true,   OFF,    &flip_assign_oper,
    "npro",     PRO_SPECIAL,    0,      IGN,    0,
    "lc",       PRO_INT,        0,      0,      &block_comment_max_col,
    "lp",       PRO_BOOL,       true,   ON,     &lineup_to_parens,
    "nlp",      PRO_BOOL,       true,   OFF,    &lineup_to_parens,
    "l",        PRO_INT,        78,     0,      &max_col,
    "psl",      PRO_BOOL,       true,   ON,     &procnames_start_line,
    "npsl",     PRO_BOOL,       true,   OFF,    &procnames_start_line,
    "fc1",      PRO_BOOL,       true,   ON,     &format_col1_comments,
    "nfc1",     PRO_BOOL,       true,   OFF,    &format_col1_comments,
    "pcs",      PRO_BOOL,       false,  ON,     &proc_calls_space,
    "npcs",     PRO_BOOL,       false,  OFF,    &proc_calls_space,
    "ip",       PRO_BOOL,       true,   ON,     &ps.indent_parameters,
    "nip",      PRO_BOOL,       true,   OFF,    &ps.indent_parameters,
 /* see set_defaults for initialization of -cli */
    "cli",      PRO_SPECIAL,    0,      CLI,    0,
    "ci",       PRO_INT,        -1,     0,      &continuation_indent,
    "cdb",      PRO_BOOL,       true,   ON,  &comment_delimiter_on_blankline,
    "ncdb",     PRO_BOOL,       true,   OFF, &comment_delimiter_on_blankline,
    "i",        PRO_INT,        8,      0,      &ps.ind_size,
    "cd",       PRO_INT,        0,      0,      &ps.decl_com_ind,
    "ce",       PRO_BOOL,       true,   ON,     &cuddle_else,
    "nce",      PRO_BOOL,       true,   OFF,    &cuddle_else,
    "c",        PRO_INT,        33,     0,      &ps.com_ind,
    "v",        PRO_BOOL,       false,  ON,     &verbose,
    "nv",       PRO_BOOL,       false,  OFF,    &verbose,
    "dj",       PRO_BOOL,       false,  ON,     &ps.ljust_decl,
    "ndj",      PRO_BOOL,       false,  OFF,    &ps.ljust_decl,
 /* don't ask *me* why -bc/-nbc is backwards.... */
    "bc",       PRO_BOOL,       true,   OFF,    &ps.leave_comma,
    "nbc",      PRO_BOOL,       true,   ON,     &ps.leave_comma,
    "di",       PRO_INT,        16,     0,      &ps.decl_indent,
    "d",        PRO_INT,        0,      0,      &ps.unindent_displace,
    "br",       PRO_BOOL,       true,   ON,     &btype_2,
    "bl",       PRO_BOOL,       true,   OFF,    &btype_2,
    "st",       PRO_SPECIAL,    0,      STDIN,  0,
    "ei",       PRO_BOOL,       true,   ON,     &ps.else_if,
    "nei",      PRO_BOOL,       true,   OFF,    &ps.else_if,
    "sc",       PRO_BOOL,       true,   ON,     &star_comment_cont,
    "nsc",      PRO_BOOL,       true,   OFF,    &star_comment_cont,
    "bap",      PRO_BOOL,       false,  ON,     &blanklines_after_procs,
    "nbap",     PRO_BOOL,       false,  OFF,    &blanklines_after_procs,
    "sob",      PRO_BOOL,       false,  ON,     &swallow_optional_blanklines,
    "nsob",     PRO_BOOL,       false,  OFF,    &swallow_optional_blanklines,
    "bad",      PRO_BOOL,       false,  ON,  &blanklines_after_declarations,
    "nbad",     PRO_BOOL,       false,  OFF, &blanklines_after_declarations,
    "bbb",      PRO_BOOL,       false,  ON,  &blanklines_before_blockcomments,
    "nbbb",     PRO_BOOL,       false,  OFF, &blanklines_before_blockcomments,
    "ps",       PRO_BOOL,       false,  ON,     &pointer_as_binop,
    "nps",      PRO_BOOL,       false,  OFF,    &pointer_as_binop,
    "troff",    PRO_BOOL,       false,  ON,     &troff,
    "T",        PRO_SPECIAL,    0,      KEY,    0,
    "slb",      PRO_BOOL,       false,  ON,     &single_line_bc,
    "nslb",     PRO_BOOL,       false,  OFF,    &single_line_bc,
 /* whew! */
    0,          0,              0,      0,      0
};

/*
 * set_profile reads $HOME/.indent.pro and ./.indent.pro and
 * handles arguments given in these files.
 */
set_profile()
{
    register FILE *f;
    char fname[BUFSIZ];
    static char pro[] = ".indent.pro";

    sprintf(fname, "%s/%s", getenv("HOME"), pro);
    if ((f = fopen(fname, "r")) != NULL) {
        scan_profile(f);
        (void) fclose(f);
    }
    if ((f = fopen(pro, "r")) != NULL) {
        scan_profile(f);
        (void) fclose(f);
    }
}

scan_profile(f)
    register FILE *f;
{
    register char *p, *arg;
    char buf[BUFSIZ];

    while (fgets(buf, sizeof buf, f)) {
        if ((p = mbschr (buf, '\n')) != NULL)
            *p = 0;
#if PROCOM
        if ((p = mbschr (buf, '#')) != NULL)    /* skip comments */
            *p = 0;
#endif /*PROCOM*/
        if (verbose)
            printf("profile: %s\n", buf);
        p = buf;
        for (;;) {
            while (isspace(*p))         /* space is always 1 byte ... */
                p++;                    /* ... so no MBS worries */

            if (*p == 0)
                break;
            arg = p;
            while (*p) {
                if (isspace(*p)) {      /* space is always 1 byte ... */
                    *p++ = 0;           /* ... so no MBs worries */
                    break;
                }
                p = nextmbc (p);
            }
            set_option(arg);
        }
    }
}

char       *param_start;

eqin(s1, s2)
    register char *s1;
    register char *s2;
{
    while (*s1) {
        if (*s1++ != *s2++)
            return (false);
    }
    param_start = s2;
    return (true);
}

/*
 * Set the defaults.
 */
set_defaults()
{
    register struct pro *p;

    /*
     * Because ps.case_indent is a float, we can't initialize it
     * from the table:
     */
    ps.case_indent = 0.0;       /* -cli0.0 */
    for (p = pro; p->p_name; p++)
        if (p->p_type != PRO_SPECIAL)
            *p->p_obj = p->p_default;
}

char num_string[11];    /* hold [.,]0123456789 for nl_langinfo work */

set_option(arg)
    register char *arg;
{
    register struct pro *p;
    size_t len;
    extern double atof();
    int n;

    arg++;                      /* ignore leading "-" */
    for (p = pro; p->p_name; p++)
        if (*p->p_name == *arg && eqin(p->p_name, arg))
            goto found;
    fprintf(stderr, MSGSTR(BADPARAM, "indent: unknown parameter \"%s\"\n"), arg - 1); /*MSG*/
    exit(1);
found:
    switch (p->p_type) {

        case PRO_SPECIAL:
            switch (p->p_special) {

                case IGN:
                    break;

                case CLI:
                    if (*param_start == 0)
                        goto need_param;
                    /* May 15/90 : -cli flag now accepts nl RADIXCHAR */
                    strcpy(num_string,nl_langinfo(RADIXCHAR)); /* ie . or , */
                    strcat(num_string,"0123456789");
                    len = strspn(param_start, num_string);
                    if (!len || len != strlen(param_start)) {
                          fprintf(stderr, MSGSTR(NUMERROR,
                                  "Option %s must be followed by a number.\n"),
                                  p->p_name);
                          exit(1);
                    }

                    ps.case_indent = atof(param_start);
                    break;

                case STDIN:
                    if (input == 0)
                        input = stdin;
                    if (output == 0)
                        output = stdout;
                    break;

                case KEY:
                    if (*param_start == 0)
                        goto need_param;
#ifdef MBCDEBUG
                    printf ("set_option: type <%s>\n", param_start);
#endif /*MBCDEBUG*/
                    addkey(param_start, 4);
                    break;

                default:
                    fprintf(stderr, "\
indent: set_option: internal error: p_special %d\n", p->p_special);
                    exit(1);
            }
            break;

        case PRO_BOOL:
            if (p->p_special == OFF)
                *p->p_obj = false;
            else
                *p->p_obj = true;
            break;

        case PRO_INT:
            if (*param_start == 0) {
need_param:
                fprintf(stderr, MSGSTR(NEEDPARAM, "indent: ``%s'' requires a parameter\n"), /*MSG*/
                        arg - 1);
                exit(1);
            }
            len = strspn(param_start, "0123456789");
            if (!len || len != strlen(param_start)) {
                fprintf(stderr, MSGSTR(INTERROR,
                        "Option %s must be followed by an integer.\n"),
                        p->p_name);
                exit(1);
            }
            *p->p_obj = atoi(param_start);
            break;

        default:
            fprintf(stderr, MSGSTR(PARSE_ERR, "indent: set_option: internal error: p_type %d\n"), /*MSG*/
                    p->p_type);
            exit(1);
    }
}
