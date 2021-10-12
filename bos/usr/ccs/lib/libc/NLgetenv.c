static char sccsid[] = "@(#)54	1.2  src/bos/usr/ccs/lib/libc/NLgetenv.c, libcenv, bos411, 9428A410j 4/22/91 11:45:13";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: NLgetenv
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <sys/localedef.h>
#include <nl_types.h>
#include <locale.h>
#include <langinfo.h>

/*
**  Get an NLS parameter from the locale information
**  setup by a call to setlocale().
**  This parameter should belong in one of the following categories:
**  LC_MONETARY, LC_NUMERIC, LC_TIME, LC_MESSAGES.
**  If the information solicited is not found in the tables setup
**  by setlocale(), a NULL pointer is returned.
*/

char * fmt_ampmstr();
char * fmt_lconv_str();
char * fmt_lconv_chr();
char * fmt_langinfo();
char * fmt_days();
char * fmt_months();
char * fmt_str();

#define MAX_KEYWORD   12
typedef struct keyword_t {
    char key[MAX_KEYWORD+1];
    char *(*fmt)();
    void *parm;
} keyword_t;

static keyword_t keys[]={
    "AMPMSTR",      fmt_ampmstr,  0, 
    "CUR_SYM",      fmt_lconv_str, &((lconv *)0)->currency_symbol,
    "DEC_PNT",      fmt_lconv_str, &((lconv *)0)->decimal_point, 
    "FRAC_DIG",     fmt_lconv_chr, &((lconv *)0)->frac_digits,
    "GROUPING",     fmt_lconv_str, &((lconv *)0)->grouping,
    "INT_CUR_SYM",  fmt_lconv_str, &((lconv *)0)->int_curr_symbol,
    "INT_FRAC",     fmt_lconv_chr, &((lconv *)0)->int_frac_digits,
    "MESSAGES",     fmt_str,       "",
    "MON_DEC_PNT",  fmt_lconv_str, &((lconv *)0)->mon_decimal_point,
    "MON_GRP",      fmt_lconv_str, &((lconv *)0)->mon_grouping,
    "MON_THOUS",    fmt_lconv_str, &((lconv *)0)->mon_thousands_sep,
    "NEG_SGN",      fmt_lconv_str, &((lconv *)0)->negative_sign,
    "NLDATE",       fmt_langinfo,  D_FMT,
    "NLDATIM",      fmt_langinfo,  D_T_FMT,
    "NLLDATE",      fmt_langinfo,  NLLDATE,
    "NLLDAY",       fmt_days,      DAY_1,
    "NLLMONTH",     fmt_months,    MON_1,
    "NLSDAY",       fmt_days,      ABDAY_1,
    "NLSMONTH",     fmt_months,    ABMON_1,
    "NLTIME",       fmt_langinfo,  T_FMT,
    "NLTMISC",      fmt_langinfo,  NLTMISC,
    "NLTSTRS",      fmt_langinfo,  NLTSTRS,
    "NLTUNITS",     fmt_langinfo,  NLTUNITS,
    "NLYEAR",       fmt_langinfo,  NLYEAR,
    "NOSTR",        fmt_langinfo,  NOSTR,
    "N_CS_PRE",     fmt_lconv_chr, &((lconv *)0)->n_cs_precedes,
    "N_SEP_SP",     fmt_lconv_chr, &((lconv *)0)->n_sep_by_space,
    "N_SGN_POS",    fmt_lconv_chr, &((lconv *)0)->n_sign_posn,
    "POS_SGN",      fmt_lconv_str, &((lconv *)0)->positive_sign,
    "P_CS_PRE",     fmt_lconv_chr, &((lconv *)0)->p_cs_precedes,
    "P_SEP_SP",     fmt_lconv_chr, &((lconv *)0)->p_sep_by_space,
    "P_SGN_POS",    fmt_lconv_chr, &((lconv *)0)->p_sign_posn,
    "THOUS_SEP",    fmt_lconv_str, &((lconv *)0)->thousands_sep,
    "YESSTR",       fmt_langinfo,  YESSTR,
};


/*
* Yes, this number is completely arbitrary.
*/
#define BUFSIZE  512
static buf[BUFSIZE+1];

/*
*  FUNCTION: NLgetenv
*
*  DESCRIPTION: 
*
*/
struct lconv *loc;
char *NLgetenv(char *name)
{
    keyword_t *key;


    loc = localeconv();
    key = bsearch(name, keys, 
		  sizeof(keys)/sizeof(key[0]), sizeof(keyword_t), 
		  strcmp);

    if (key == NULL)
	return NULL;
    else
	return (*(key->fmt))(key->parm);
}


/*
*  FUNCTION: fmt_ampmstr
*
*  DESCRIPTION: 
*
*/
char * fmt_ampmstr(void)
{
    strncpy(buf, nl_langinfo(AM_STR), BUFSIZE);
    strncat(buf, ":", BUFSIZE);
    strncat(buf, nl_langinfo(PM_STR), BUFSIZE);

    return buf;
}


/*
*  FUNCTION: fmt_lconv_str
*
*  DESCRIPTION: 
*
*/
char * fmt_lconv_str(unsigned int offs)
{
    return *(char **)((char *)loc + offs);
}


/*
*  FUNCTION: fmt_str
*
*  DESCRIPTION: 
*
*/
char * fmt_str(char * s)
{
    return s;
}


/*
*  FUNCTION: fmt_langinfo
*
*  DESCRIPTION: 
*
*/
char * fmt_langinfo(nl_item item)
{
    return nl_langinfo(item);
}


/*
*  FUNCTION: fmt_lconv_chr
*
*  DESCRIPTION: 
*
*/
char * fmt_lconv_chr(int offs)
{
    buf[0] = *(char *)((char *)loc + offs);
    buf[1] = '\0';

    return buf;
}


/*
*  FUNCTION: fmt_months
*
*  DESCRIPTION: 
*
*/
char *fmt_months(nl_item first_month)
{
    int i;

    strncpy(buf, nl_langinfo(first_month), BUFSIZE);
    for (i=first_month+1; i < first_month+12; i++) {
	strncat(buf, ":", BUFSIZE);
	strncat(buf, nl_langinfo(i), BUFSIZE);
    }

    return buf;
}


/*
*  FUNCTION: fmt_days
*
*  DESCRIPTION: 
*
*/
char *fmt_days(nl_item first_day)
{
    int i;

    strncpy(buf, nl_langinfo(first_day), BUFSIZE);
    for (i=first_day+1; i < first_day+7; i++) {
	strncat(buf, ":", BUFSIZE);
	strncat(buf, nl_langinfo(i), BUFSIZE);
    }

    return buf;
}
