static char sccsid[] = "@(#)97	1.8.1.6  src/bos/usr/bin/locale/locale.c, cmdnls, bos411, 9428A410j 4/2/94 15:13:05";
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 85
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.2
 */

#define _ILS_MACROS

#include <sys/localedef.h>
#include <sys/lc_core.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>


extern char *optarg;
extern int optind;

/* 
*  prototypes for static functions
*/
void mark_keyword(char *key);
void usage(void);
void error(int, ...);
void list_locales();
void list_charmaps();
void dump_names();
int check_locale( const char*, const struct stat*);
int check_charmap( const char*, const struct stat*);
int my_ftw( char *, int (*)(char *,struct stat *));


void (*pint)(int *, int, char *);
void (*pstr)(int *, int, char *);
void (*pnum)(int *, int, char *);
void (*pslst)(int *, int, char *, int);
void (*prtcollate)(int *, int, char *, int);
void (*prtctype)(int *, int, char *, int);
void (*pstrnum)(int *, int, char *);

void prt_int(int *, int, char *);
void prt_str(int *, int, char *);
void prt_num(int *, int, char *);
void prt_slst(int *, int, char *, int);
void prt_collate(int *, int, char *, int);
void prt_ctype(int *, int, char *, int);
void prt_strnum(int *, int, char*);

void kprt_int(int *, int, char *);
void kprt_str(int *, int, char *);
void kprt_num(int *, int, char *);
void kprt_slst(int *, int, char *, int);
void kprt_ctype(int *, int, char *, int);
void kprt_strnum(int *, int, char *);


#define SZ(t)    (sizeof(t) / sizeof(t[0]))

#define DEF_LOCPATH     "/usr/lib/nls/loc"
#define DEF_CHARMAP     "/usr/lib/nls/charmap"

/*
*  These are arbitrary contants that are used for building the keyword
*  tables.  They simply name the index into the arrays at which the keyword
*  is located.
*/
#define CHARMAP                 0
#define LCCHARMAP               0
#define CODE_SET_NAME           1
#define MBCURMAX                2
#define MBCURMIN                3

#define LCCOLLATE               1

#define LCCTYPE                 2

#define LCMESSAGES	        3
#define YESEXPR		        0
#define NOEXPR		        1
#define YESSTR			2
#define NOSTR			3

#define LCMONETARY		4
#define INT_CURR_SYMBOL		0
#define CURRENCY_SYMBOL		1
#define MON_DECIMAL_POINT	2
#define MON_GROUPING		3
#define MON_THOUSANDS_SEP	4
#define POSITIVE_SIGN		5
#define NEGATIVE_SIGN		6
#define INT_FRAC_DIGITS		7
#define FRAC_DIGITS		8
#define P_CS_PRECEDES		9
#define P_SEP_BY_SPACE		10
#define N_CS_PRECEDES		11
#define N_SEP_BY_SPACE		12
#define P_SIGN_POSN		13
#define N_SIGN_POSN		14
#define DEBIT_SIGN		15
#define CREDIT_SIGN		16
#define LEFT_PARENTHESIS	17
#define RIGHT_PARENTHESIS	18

#define LCNUMERIC		5
#define DECIMAL_POINT		0
#define THOUSANDS_SEP		1
#define GROUPING		2

#define LCTIME		        6
#define ABDAY		        0
#define DAY		        1
#define ABMON		        2
#define MON		        3
#define D_T_FMT		        4
#define D_FMT			5
#define T_FMT		        6
#define AM_PM		        7
#define ERA		        8
#define ERA_D_FMT		9
#define ERA_YEAR		10
#define T_FMT_AMPM		11
#define ERA_T_FMT		12
#define ERA_D_T_FMT		13
#define ALT_DIGITS		14


/*
*  An item list for each category.  The item list contains 
*    o a pointer to the function which prints the value of the variable, 
*    o the number of values the variable may assume,
*    o a boolean which indicates if the keyword for the item was on the 
*         command line,
*    o a string defining the keyword,
*    o the offset of the value in the category structure.
*/
typedef struct {
    void          (*(*prt))();
    int           n;
    unsigned char refd;
    char          *name;
    void          *off;
} item_t;

/*
*  item list for the CHARMAP category 
*/
item_t chr_item_list[] = {
&pstr, 1, FALSE, "charmap",           &((_LC_charmap_t *)0)->cm_csname,
&pstr, 1, FALSE, "code_set_name",     &((_LC_charmap_t *)0)->cm_csname,
&pint, 1, FALSE, "mb_cur_max",        &((_LC_charmap_t *)0)->cm_mb_cur_max,
&pint, 1, FALSE, "mb_cur_min",        &((_LC_charmap_t *)0)->cm_mb_cur_min,
};

/*
*  item list for the LC_COLLATE category 
*/
item_t col_item_list[] = {
&prtcollate, 1, FALSE, "", 0,
};

/*
*  item list for the LC_CTYPE category 
*/
item_t ctp_item_list[] = {
&prtctype, 1, FALSE, "", 0,
};

/*
*  item list for the LC_MESSAGES category
*/
item_t msg_item_list[] = {
&pstr, 1, FALSE, "yesexpr",           &((_LC_resp_t *)0)->yesexpr,
&pstr, 1, FALSE, "noexpr",            &((_LC_resp_t *)0)->noexpr,
&pstr, 1, FALSE, "yesstr",            &((_LC_resp_t *)0)->yesstr,
&pstr, 1, FALSE, "nostr",             &((_LC_resp_t *)0)->nostr,
};

/*
*  item list for the LC_MONETARY category
*/
item_t mon_item_list[] = {
&pstr, 1,FALSE, "int_curr_symbol",  &((_LC_monetary_t *)0)->int_curr_symbol,
&pstr, 1,FALSE, "currency_symbol",  &((_LC_monetary_t *)0)->currency_symbol,
&pstr, 1,FALSE, "mon_decimal_point",&((_LC_monetary_t *)0)->mon_decimal_point,
&pstrnum, 1,FALSE, "mon_grouping",     &((_LC_monetary_t *)0)->mon_grouping,
&pstr, 1,FALSE, "mon_thousands_sep",&((_LC_monetary_t *)0)->mon_thousands_sep,
&pstr, 1,FALSE, "positive_sign",    &((_LC_monetary_t *)0)->positive_sign,
&pstr, 1,FALSE, "negative_sign",    &((_LC_monetary_t *)0)->negative_sign,
&pnum, 1,FALSE, "int_frac_digits",  &((_LC_monetary_t *)0)->int_frac_digits,
&pnum, 1,FALSE, "frac_digits",	    &((_LC_monetary_t *)0)->frac_digits,
&pnum, 1,FALSE, "p_cs_precedes",    &((_LC_monetary_t *)0)->p_cs_precedes,
&pnum, 1,FALSE, "p_sep_by_space",   &((_LC_monetary_t *)0)->p_sep_by_space,
&pnum, 1,FALSE, "n_cs_precedes",    &((_LC_monetary_t *)0)->n_cs_precedes,
&pnum, 1,FALSE, "n_sep_by_space",   &((_LC_monetary_t *)0)->n_sep_by_space,
&pnum, 1,FALSE, "p_sign_posn",      &((_LC_monetary_t *)0)->p_sign_posn,
&pnum, 1,FALSE, "n_sign_posn",      &((_LC_monetary_t *)0)->n_sign_posn,
&pstr, 1,FALSE, "debit_sign",       &((_LC_monetary_t *)0)->debit_sign,
&pstr, 1,FALSE, "credit_sign",      &((_LC_monetary_t *)0)->credit_sign,
&pstr, 1,FALSE, "left_parenthesis", &((_LC_monetary_t *)0)->left_parenthesis,
&pstr, 1,FALSE, "right_parenthesis",&((_LC_monetary_t *)0)->right_parenthesis,
};

/*
*  item list for the LC_NUMERIC category
*/
item_t num_item_list[] = {
&pstr,     1, FALSE, "decimal_point", &((_LC_numeric_t *)0)->decimal_point,
&pstr,     1, FALSE, "thousands_sep", &((_LC_numeric_t *)0)->thousands_sep,
&pstrnum,  1, FALSE, "grouping",      &((_LC_numeric_t *)0)->grouping,
};

/*
*  item list for the LC_TIME category
*/
item_t tim_item_list[] = {
&pslst, 7,  FALSE, "abday",	  &((_LC_time_t *)0)->abday,
&pslst, 7,  FALSE, "day",	  &((_LC_time_t *)0)->day,
&pslst, 12, FALSE, "abmon",	  &((_LC_time_t *)0)->abmon,
&pslst, 12, FALSE, "mon",	  &((_LC_time_t *)0)->mon,
&pstr,  1,  FALSE, "d_t_fmt",	  &((_LC_time_t *)0)->d_t_fmt,
&pstr,  1,  FALSE, "d_fmt",	  &((_LC_time_t *)0)->d_fmt,
&pstr,  1,  FALSE, "t_fmt",	  &((_LC_time_t *)0)->t_fmt,
&pslst, 2,  FALSE, "am_pm",	  &((_LC_time_t *)0)->am_pm,
&pstr,  1,  FALSE, "era",	  &((_LC_time_t *)0)->era,
&pstr,  1,  FALSE, "era_d_fmt",   &((_LC_time_t *)0)->era_d_fmt,
&pstr,  1,  FALSE, "era_year",    &((_LC_time_t *)0)->era_year,
&pstr,  1,  FALSE, "t_fmt_ampm",  &((_LC_time_t *)0)->t_fmt_ampm,
&pstr,  1,  FALSE, "era_t_fmt",   &((_LC_time_t *)0)->era_t_fmt,
&pstr,  1,  FALSE, "era_d_t_fmt", &((_LC_time_t *)0)->era_d_t_fmt,
&pstr,  1,  FALSE, "alt_digits",  &((_LC_time_t *)0)->alt_digits,

};


/*
*  This is the category table.  There is one entry for each category which
*  contains 
*    o the category name, 
*    o two booleans which indicate if the category keyword was referenced
*         and if any item in the category was referenced, 
*    o a pointer to the category pointer for the locale,
*    o the number of locale items in the category
*    o a pointer to the item list for the category.
*/
struct category {
    char          *name;
    unsigned char key_refd;
    unsigned char cat_refd;
    void          *cat_ptr;
    unsigned char n_items;
    item_t        *items;
} cat_list[] = {
"CHARMAP",     FALSE, FALSE, &__lc_charmap, SZ(chr_item_list), chr_item_list,
"LC_COLLATE",  FALSE, FALSE, &__lc_charmap, SZ(col_item_list), col_item_list, 
"LC_CTYPE",    FALSE, FALSE, &__lc_charmap, SZ(ctp_item_list), ctp_item_list,
"LC_MESSAGES", FALSE, FALSE, &__lc_resp, SZ(msg_item_list), msg_item_list,
"LC_MONETARY", FALSE, FALSE, &__lc_monetary, SZ(mon_item_list), mon_item_list,
"LC_NUMERIC",  FALSE, FALSE, &__lc_numeric, SZ(num_item_list), num_item_list,
"LC_TIME",     FALSE, FALSE, &__lc_time, SZ(tim_item_list), tim_item_list,
};


/*
*  This is the keyword table which is searched for each keyword on the 
*  command line.  The keyword table contains the keyword itself, and a 
*  pointer to the 'referenced' fields of the category and item lists.
*
*  This list is sorted!
*/
struct {
    char *key;
    unsigned char *key_refd;
    unsigned char *cat_refd;
} keylist[]={
"CHARMAP",          
&cat_list[CHARMAP].key_refd,           &cat_list[CHARMAP].cat_refd,
"LC_COLLATE",          
&cat_list[LCCOLLATE].key_refd,         &cat_list[LCCOLLATE].cat_refd,
"LC_CTYPE",          
&cat_list[LCCTYPE].key_refd,           &cat_list[LCCTYPE].cat_refd,
"LC_MESSAGES",          
&cat_list[LCMESSAGES].key_refd,        &cat_list[LCMESSAGES].cat_refd,
"LC_MONETARY",	        
&cat_list[LCMONETARY].key_refd,        &cat_list[LCMONETARY].cat_refd,
"LC_NUMERIC",		
&cat_list[LCNUMERIC].key_refd,	       &cat_list[LCNUMERIC].cat_refd,
"LC_TIME",		
&cat_list[LCTIME].key_refd,            &cat_list[LCTIME].cat_refd,

"abday",		
&tim_item_list[ABDAY].refd,             &cat_list[LCTIME].cat_refd,
"abmon",		
&tim_item_list[ABMON].refd,             &cat_list[LCTIME].cat_refd,
"alt_digits",
&tim_item_list[ALT_DIGITS].refd,        &cat_list[LCTIME].cat_refd,
"am_pm",		
&tim_item_list[AM_PM].refd,             &cat_list[LCTIME].cat_refd,
"charmap",
&chr_item_list[LCCHARMAP].refd,         &cat_list[CHARMAP].cat_refd,
"code_set_name",
&chr_item_list[CODE_SET_NAME].refd,     &cat_list[CHARMAP].cat_refd,
"credit_sign",		
&mon_item_list[CREDIT_SIGN].refd,       &cat_list[LCMONETARY].cat_refd,
"currency_symbol",	
&mon_item_list[CURRENCY_SYMBOL].refd,   &cat_list[LCMONETARY].cat_refd,
"d_fmt",
&tim_item_list[D_FMT].refd,		&cat_list[LCTIME].cat_refd,
"d_t_fmt",		
&tim_item_list[D_T_FMT].refd,           &cat_list[LCTIME].cat_refd,
"day",		 	
&tim_item_list[DAY].refd,               &cat_list[LCTIME].cat_refd,
"debit_sign",		
&mon_item_list[DEBIT_SIGN].refd,        &cat_list[LCMONETARY].cat_refd,
"decimal_point",	
&num_item_list[DECIMAL_POINT].refd,     &cat_list[LCNUMERIC].cat_refd,
"era",		 	
&tim_item_list[ERA].refd,               &cat_list[LCTIME].cat_refd,
"era_d_fmt",
&tim_item_list[ERA_D_FMT].refd,         &cat_list[LCTIME].cat_refd,
"era_d_t_fmt",
&tim_item_list[ERA_D_T_FMT].refd,       &cat_list[LCTIME].cat_refd,
"era_t_fmt",
&tim_item_list[ERA_T_FMT].refd,         &cat_list[LCTIME].cat_refd,
"era_year",
&tim_item_list[ERA_YEAR].refd,          &cat_list[LCTIME].cat_refd,
"frac_digits",
&mon_item_list[FRAC_DIGITS].refd,	&cat_list[LCMONETARY].cat_refd,
"grouping",		
&num_item_list[GROUPING].refd,          &cat_list[LCNUMERIC].cat_refd,
"int_curr_symbol",	
&mon_item_list[INT_CURR_SYMBOL].refd,   &cat_list[LCMONETARY].cat_refd,
"int_frac_digits",	
&mon_item_list[INT_FRAC_DIGITS].refd,   &cat_list[LCMONETARY].cat_refd,
"left_parenthesis",	
&mon_item_list[LEFT_PARENTHESIS].refd,  &cat_list[LCMONETARY].cat_refd,
"mb_cur_max",
&chr_item_list[MBCURMAX].refd,          &cat_list[CHARMAP].cat_refd,
"mb_cur_min",
&chr_item_list[MBCURMIN].refd,          &cat_list[CHARMAP].cat_refd,
"mon",	
&tim_item_list[MON].refd,               &cat_list[LCTIME].cat_refd,
"mon_decimal_point",	
&mon_item_list[MON_DECIMAL_POINT].refd, &cat_list[LCMONETARY].cat_refd,
"mon_grouping",		
&mon_item_list[MON_GROUPING].refd,      &cat_list[LCMONETARY].cat_refd,
"mon_thousands_sep",	
&mon_item_list[MON_THOUSANDS_SEP].refd, &cat_list[LCMONETARY].cat_refd,
"n_cs_precedes",	
&mon_item_list[N_CS_PRECEDES].refd,     &cat_list[LCMONETARY].cat_refd,
"n_sep_by_space",	
&mon_item_list[N_SEP_BY_SPACE].refd,    &cat_list[LCMONETARY].cat_refd,
"n_sign_posn",		
&mon_item_list[N_SIGN_POSN].refd,       &cat_list[LCMONETARY].cat_refd,
"negative_sign",	
&mon_item_list[NEGATIVE_SIGN].refd,     &cat_list[LCMONETARY].cat_refd,
"noexpr",		
&msg_item_list[NOEXPR].refd,            &cat_list[LCMESSAGES].cat_refd,
"nostr",		
&msg_item_list[NOSTR].refd,             &cat_list[LCMESSAGES].cat_refd,
"p_cs_precedes",	
&mon_item_list[P_CS_PRECEDES].refd,     &cat_list[LCMONETARY].cat_refd,
"p_sep_by_space",	
&mon_item_list[P_SEP_BY_SPACE].refd,    &cat_list[LCMONETARY].cat_refd,
"p_sign_posn",		
&mon_item_list[P_SIGN_POSN].refd,       &cat_list[LCMONETARY].cat_refd,
"positive_sign",	
&mon_item_list[POSITIVE_SIGN].refd,     &cat_list[LCMONETARY].cat_refd,
"right_parenthesis",	
&mon_item_list[RIGHT_PARENTHESIS].refd, &cat_list[LCMONETARY].cat_refd,
"t_fmt",		
&tim_item_list[T_FMT].refd,             &cat_list[LCTIME].cat_refd,
"t_fmt_ampm",
&tim_item_list[T_FMT_AMPM].refd,        &cat_list[LCTIME].cat_refd,
"thousands_sep",	
&num_item_list[THOUSANDS_SEP].refd,     &cat_list[LCNUMERIC].cat_refd,
"yesexpr",	 	
&msg_item_list[YESEXPR].refd,           &cat_list[LCMESSAGES].cat_refd,
"yesstr",	 	
&msg_item_list[YESSTR].refd,            &cat_list[LCMESSAGES].cat_refd,
};

static int err_count = 0;

void main(int argc, char *argv[])
{
    int show_cat = FALSE;
    int show_kwd = FALSE;
    int show_locales  = FALSE;
    int show_charmaps = FALSE;
    int c;
    int i, j;

    char *locname;

    setlocale(LC_ALL,"");	/* needed so that NLS messages are output */
				/* by mark_keyword() and usage()          */
    locname = "";
    if (argc==1) {
	dump_names();
	exit(0);
    }

    while((c=getopt(argc, argv, "amck")) != EOF) {
	switch (c) {
	  case 'a':
            show_locales = TRUE;
	    break;
	  case 'm':
            show_charmaps = TRUE;
	    break;
	  case 'c':
	    show_cat = TRUE;
	    break;
	  case 'k':
	    show_kwd = TRUE;
	    break;
	  default:
	    usage();
	}
    }

    if (((show_kwd || show_cat) && !argv[optind])
        || (show_locales && (show_charmaps || show_kwd || show_cat))
        || (show_charmaps && (show_locales || show_kwd || show_cat))
        || ((show_locales || show_charmaps) && argv[optind]))
             usage();

    if (show_locales) {
        list_locales();
        exit(0);
    }

    if (show_charmaps) {
        list_charmaps();
        exit(0);
    }

    if ((argc==optind) && !(show_locales||show_charmaps||show_kwd||show_cat)) {
	dump_names();			/* if the user typed "locale --" */
	exit(0);
    }

    for(i=optind; i < argc; i++)
	mark_keyword(argv[i]);

    setlocale(LC_ALL, locname);

    if (show_kwd) {
	pint = kprt_int;
	pstr = kprt_str;
	pnum = kprt_num;
	pslst = kprt_slst;
	prtcollate = prt_collate;
	prtctype = kprt_ctype;
	pstrnum = kprt_strnum;
    } else {
	pint = prt_int;
	pstr = prt_str;
	pnum = prt_num;
	pslst = prt_slst;
	prtcollate = prt_collate;
	prtctype = prt_ctype;
	pstrnum = prt_strnum;
    }

    for (i=0; i<SZ(cat_list); i++) {
	if (cat_list[i].cat_refd) {
	    if (show_cat) 
		printf("%s\n", cat_list[i].name);
	    
	    /* category name was used as keyword - print entire category 
	     */
	    if (cat_list[i].key_refd) {
		for (j=0; j<cat_list[i].n_items; j++)
		    (*(*cat_list[i].items[j].prt))
			(cat_list[i].cat_ptr, 
			 cat_list[i].items[j].off, 
			 cat_list[i].items[j].name, 
			 cat_list[i].items[j].n);
	    } else if (cat_list[i].cat_refd) {
		/* category name was not used print only items specified 
		 */
		for (j=0; j<cat_list[i].n_items; j++)
		    if (cat_list[i].items[j].refd) 
			(*(*cat_list[i].items[j].prt))
			    (cat_list[i].cat_ptr, 
			     cat_list[i].items[j].off, 
			     cat_list[i].items[j].name,
			     cat_list[i].items[j].n);
	    }		
	}
    }

    if (err_count) 
	exit(1);
    else
	exit(0);
}	


/*
*  FUNCTION: dump_names
*
*  DESCRIPTION:
*  Print out effective locale names for the various categories.
*/
void dump_names(void)
{
    static const char *category_name[]={
	"LC_COLLATE", "LC_CTYPE", "LC_MONETARY", "LC_NUMERIC", 
	"LC_TIME",  "LC_MESSAGES"
	};
    
    int  i;
    char *s;
    char *lc_all;
    char *lang;

    lc_all = getenv("LC_ALL");
    if (lc_all == NULL) lc_all="";

    lang = getenv("LANG");
    if (lang == NULL) lang = "";

    s = setlocale(LC_ALL, "");

    if (s == NULL)
	s = setlocale(LC_ALL,NULL);

    /*
      LANG environment variable is always written out "as-is"
    */
    printf("LANG=%s\n", lang);

    for (i=0; i<=LC_MESSAGES; i++) {
	char *envvar;
	char *effval;

	envvar = getenv(category_name[i]);
	effval = strtok(s, " ");
	if (effval == NULL) 
	    effval = "";

	if (envvar == NULL)
	    printf("%s=\"%s\"\n", category_name[i], effval);
	else {

	    if (strcmp(envvar, effval) != 0)
		printf("%s=\"%s\"\n", category_name[i], effval);
	    else
		printf("%s=%s\n", category_name[i], effval);
	}

	s = NULL;
    }

    printf("LC_ALL=%s\n", lc_all);
}


/*
*  FUNCTION: check_locale
*
*  DESCRIPTION:
*  Checks to see if the file is executable and if it is, prints the
*  name.  The assumption here is that any executable in LOCPATH is 
*  a locale.
*/
int check_locale(const char *name, const struct stat *s)
{
    if (s->st_mode & S_IEXEC){
	const char *p = strrchr(name,'/');
	if (strstr(p,".im") == NULL){
	    if (*p == '/')
	        p++; 			/*get rid of / */
	    printf("%s\n", p);
	}
    }

    return 0;
}


/*
*  FUNCTION: list_locales
* 
*  DESCRIPTION:
*  Locate and display all of the locally available locales on the system.
*  This is accomplished by searching the LOCPATH for executable objects
*  which do not contain the string ".im".  While as it is possible to
*  look for magic numbers, etc the current method is an easy to maintain,
*  and is an efficient  way to check for locale objects.
*/
void list_locales(void)
{
    char *path;
    char *epath;
    char *locpath;

    locpath = getenv("LOCPATH");
    if (locpath == NULL || locpath[0] == '\0')
	locpath = DEF_LOCPATH;

    path = malloc(strlen(locpath)+1);
    if (path==NULL) exit(-1);
    strcpy(path, locpath);

    printf("C\n");			/* Present by default */
    printf("POSIX\n");			/* Present by default */

    for (; path != NULL; path = epath) {
	epath = strchr(path, ':');
	if (epath != NULL) {
	    *epath = '\0';
	    epath++;
	}
	my_ftw(path, check_locale);
    }
}


/*
*  FUNCTION: check_charmap
*
*  DESCRIPTION:
*  Checks to see if the file is a regular file or not.
*/
int check_charmap(const char *name, const struct stat *s)
{
    const char *p = strrchr(name,'/');
    if (strstr(p,".im") == NULL){
	if (*p == '/')
	    p++; 			/*get rid of / */
	printf("%s\n", p);
    }

    return 0;
}


/*
*  FUNCTION: list_charmaps
*
*  DESCRIPTION:
*  Any regular file in the directory DEF_CHARMAP is considered a
*  charmap.
*/
void list_charmaps()
{
    char *path = DEF_CHARMAP;

    my_ftw(path, check_charmap);
}


/* this is a simplified version of libc's ftw().			*/
/* the libc ftw() routine was used in AIX 3.2.x, however since ftw()	*/
/* descends directories, we are also getting iconv objects in version   */
/* 4.1.  This function was created to avoid this problem.               */
/* my_ftw() will return all regular files in a directory pointed to by  */
/* path via the function fn.						*/


int
my_ftw( char *path, int (*fn)(char *,struct stat *))

	/* path the directory we are going to scan			*/
	/* fn is the user function to call for each non directory file	*/
{
	int n;
	char *subpath, *component;
	struct stat sb;
	DIR *dirp;
	struct dirent *entry;

	/*
	 * Try to get file status.
	 * if we can't read it, we don't care so abort
	 */
	if(stat(path, &sb) < 0)
		return(-1);

	/*
	 *	The stat succeeded, so we know the object exists.
	 *	if a regular file, call the user function and quit,
	 *	since there is nothing else...
	 */
	if(!S_ISDIR(sb.st_mode)) {

		if(S_ISREG(sb.st_mode))
			(*fn)(path, &sb);
		return(0);
		}

	/*
	 *	The object was a directory.
	 *
	 *	Open a file to read the directory
	 */
	dirp = opendir(path);

	/*
	 * if we can't read the directory for any reason then abort
	 */
	if(dirp == NULL)
		return(-1);

	/* We could read the directory.   But don't return this	*/
	/* information since we only care about normal files	*/

	/* Allocate a buffer to hold generated pathnames. */
	n = strlen(path);
	subpath = malloc((unsigned)(n+MAXNAMLEN+2));
	if(subpath == NULL) {
		closedir(dirp);
		return(-1);
	}
	
	/* Create a prefix to which we will append component names */
	(void)strcpy(subpath, path);
	if(subpath[0] != '\0' && subpath[n-1] != '/')
		subpath[n++] = '/';
	component = &subpath[n];

	/*
	 *      Read the directory one component at a time.
	 *      We must ignore "." and "..", but other than that,
	 *      just create a path name pass it to user subroutine
	 *	if it is a normal file
	 */
	while((entry = readdir(dirp)) != NULL) {
		if(strcmp(entry->d_name, ".") != 0 &&
		   strcmp(entry->d_name, "..") != 0) {

			/* Append component name to the working path */
			strcpy(component, entry->d_name);

			if(stat(subpath, &sb) < 0)
				continue;  /* skip unreadable entries */

			if(S_ISREG(sb.st_mode))
				(*fn)(subpath, &sb);
		}
	}

	/*
	 *      We got out of the subdirectory loop.
	 *      Clean up and return.
	 */
	free(subpath);
	closedir(dirp);

	return(0);
}

/* array of default messages and err ids */
#include "locale_msg.h"
char *err_fmt[]={
    "",
    "Usage: locale [-a | -m] | [[-ck] name ... ]\n",
    "locale: unrecognized keyword, '%s', in argument list.\n"
};


/*
*  FUNCTION: error
*
*  DESCRIPTION:
*  This function prints error messages given an error number 'err'.  
*  The routine accepts a variable number of arguments and all errors
*  are non-fatal, i.e. the routine returns regardless of the error
*  number.
*/
void error(int err, ...)
{
    nl_catd catd;
    va_list ap;

    catd = catopen(MF_LOCALE, NL_CAT_LOCALE);

    va_start(ap, err);

    vfprintf(stderr, 
	     catgets(catd, LOCALE, err, err_fmt[err]), 
	     ap);
    err_count++;
}


/*
*  FUNCTION: mark_keyword
*
*  DESCRIPTION: 
*  This routine searches the keyword table for 'key'.  If the keyword
*  is found, it marks the keyword as 'referenced', and the category in
*  which the keyword is located as 'referenced'.  
*
*  A binary search algorithm is used to locate the keyword
*/
void mark_keyword(char *key)
{
    int c;
    int middle;
    int top;
    int bottom;
    
    bottom = 0;
    top    = SZ(keylist);

    do {
	middle = ((top - bottom) / 2) + bottom;
	c=strcmp(key, keylist[middle].key);

	if (c == 0) {
	    *(keylist[middle].key_refd) = TRUE;
	    *(keylist[middle].cat_refd) = TRUE;
	    return;
	} else if (c < 0)
	    top = middle -1;
	else
	    bottom = middle +1;

    } while (top >= bottom);

    error(ERR_KEYWORD, key);
}


/*
*  FUNCTION: usage
*
*  DESCRIPTION:
*  Prints a usage statement for the command.
*/
void usage(void)
{
    nl_catd catd;

    catd = catopen(MF_LOCALE,NL_CAT_LOCALE);
    printf(catgets(catd, LOCALE, ERR_USAGE, err_fmt[ERR_USAGE]));
    exit(1);
}


/*
*  FUNCTION: print_string
*
*  DESCRIPTION:
*  Prints out a string as per the POSIX "escaped character" requirements
*/
void print_string(char *s)
{
    wchar_t pc;
    int     rc;

    while (*s != '\0') {
	rc = mbtowc(&pc, s, MB_CUR_MAX);
	if (rc < 0)
	    s++;
	else if (rc == 1) {
	    /* check for escape characters */
	    switch (*s) {
	      case '\\':
	      case ';':
	      case '"':
		putchar('\\');
		putchar(*s++);
		break;
	      default:
		if (iscntrl(*s)) 
		    printf("\\x%02x", *s++);
		else
		    putchar(*s++);
	    }

	} else 
	    for (; rc > 0; rc--)
		putchar(*s++);
    }
}


/*
*  FUNCTION: kprt_str
* 
*  DESCRIPTION: 
*  Prints the value of a string locale variable in the format specified
*  by POSIX when the -k flag is specified.  The address of this string 
*  is at offset p2 from *p1.
*
*  The format is "%s=\"%s\"\n", <keyword>, <value>
*/
void kprt_str(int *p1, int p2, char *name)
{
    char *s = *(char **)(*p1+p2);

    printf("%s=\"", name);
    
    print_string(s);
    putchar('"');
    putchar('\n');
}


/*
*  FUNCTION: kprt_num
* 
*  DESCRIPTION: 
*  Prints the value of a numeric locale variable in the format specified
*  by POSIX when the -k flag is specified.  The address of this number is
*  at offset p2 from *p1.  The data being printed is a signed char.
*
*  The format is "%s=%d\n", <keyword>, <value>
*/
void kprt_num(int *p1, int p2, char *name)
{
    printf("%s=%d\n", name, *(signed char *)(*p1+p2));
}


/*
*  FUNCTION: kprt_int
* 
*  DESCRIPTION: 
*  Prints the value of a numeric locale variable in the format specified
*  by POSIX when the -k flag is specified.  The address of this number is
*  at offset p2 from *p1.  The data type being printed is an integer.
*
*  The format is "%s=%d\n", <keyword>, <value>
*/
void kprt_int(int *p1, int p2, char *name)
{
    printf("%s=%d\n", name, *(int *)(*p1+p2));
}


/*
*  FUNCTION: kprt_slst
* 
*  DESCRIPTION: 
*  Prints the list of strings for locale variables which take mulitple
*  string values, such as abday.  This format has not been specified by
*  POSIX, so this format is just a guess (probably wrong).
*
*  The format is "%s=\"%s\";...;\"%s\"\n", <keyword>, <value>, ..., <value>
*/
void kprt_slst(int *p1, int p2, char *name, int n)
{
    char **s;
    int  i;

    s = (char **)(*p1+p2);
    printf("%s=", name);
    for (i=0; i<n; i++) {
	if (i != 0) 
	    putchar(';');

	putchar('"');
	print_string(s[i]);
	putchar('"');
    }
    putchar('\n');
}


/*
*  FUNCTION: prt_str
* 
*  DESCRIPTION: 
*  This is the no-keyword version of the kprt_str function.
* 
*  The format is "%s\n", <value>
*/
void prt_str(int *p1, int p2, char *name)
{
    char *s = *(char **)(*p1+p2);

    print_string(s);
    putchar('\n');
}


/*
*  FUNCTION: prt_int
* 
*  DESCRIPTION: 
*  This is the no-keyword version of the kprt_int function.
*
*  The format is "%d\n", <value>
*/
void prt_int(int *p1, int p2, char *name)
{
    printf("%d\n", *(int *)(*p1+p2));
}


/*
*  FUNCTION: prt_num
* 
*  DESCRIPTION: 
*  This is the no-keyword version of the kprt_num function.
* 
*  The format is "%d\n", <value>
*/
void prt_num(int *p1, int p2, char *name)
{
    
    printf("%d\n", *(signed char *)(*p1+p2));
}


/*
*  FUNCTION: prt_slst
* 
*  DESCRIPTION: 
*  This is the no-keyword version of the kprt_slst function.
* 
*  The format is "\"%s\";...;\"%s\"\n", <value>, ..., <value>
*/
void prt_slst(int *p1, int p2, char *name, int n)
{
    char **s;
    int  i;

    s = (char **)(*p1+p2);
    for (i=0; i<n; i++) {
	if (i != 0) 
	    putchar(';');

	putchar('"');
	print_string(s[i]);
	putchar('"');
    }
    putchar('\n');
}

/*
*  FUNCTION: prt_collate
* 
*  DESCRIPTION:
*  Stub for the collate class which does nothing. 
*/
void prt_collate(int *p1, int p2, char *name, int n)
{
}


/*
*  FUNCTION: prt_ctype
* 
*  DESCRIPTION:
*  Prints out a list of the character classes defined for this locale.
*/
void prt_ctype(int *p1, int p2, char *name, int n)
{
    int i;   
    
    for (i=0; i < __OBJ_DATA(__lc_ctype)->nclasses; i++)
	printf("%s\n", __OBJ_DATA(__lc_ctype)->classnms[i].name);
}


/*
*  FUNCTION: kprt_ctype
* 
*  DESCRIPTION:
*  Prints out a list of the character classes defined for this locale.
*/
void kprt_ctype(int *p1, int p2, char *name, int n)
{
    int i;   
    
    for (i=0; i < __OBJ_DATA(__lc_ctype)->nclasses; i++)
	printf("%s=0x%04X\n", 
	       __OBJ_DATA(__lc_ctype)->classnms[i].name,
	       __OBJ_DATA(__lc_ctype)->classnms[i].mask);
}

/*
*  FUNCTION: prt_strnum
* 
*  DESCRIPTION:
*  Prints out a semi-colon separated list of numbers for the grouping
*  keyword (LC_NUMERIC and LC_MONETARY). These numbers are stored as
*  a string. This string is NULL terminated. 
*/
void prt_strnum(int *p1,int p2,char *name)
{
    char *s = *(char **)(*p1 + p2);
    int     i;

    i = 0;
    while (*s != '\0') {
        if (i == 0)
	    i++;
	else
	    printf(";");
	printf("%d", *s++);
    }
    printf("\n");
}

/*
*  FUNCTION: kprt_strnum
* 
*  DESCRIPTION:
*  Prints out a semi-colon separated list of numbers for the grouping
*  keyword (LC_NUMERIC and LC_MONETARY). These numbers are stored as
*  a string. This string is NULL terminated. 
*/
void kprt_strnum(int *p1,int p2,char *name)
{
    char *s = *(char **)(*p1 + p2);
    int     i;

    i = 0;
    printf("%s=",name);
    printf("\"");
    while (*s != '\0') {
	    if (i == 0) 
		i++;
	    else
		printf(";");
	    printf("%d", *s++);
    }
    printf("\"\n");
}
