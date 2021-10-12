static char sccsid[] = "@(#)02	1.8.1.6  src/bos/usr/bin/localedef/scan.c, cmdnls, bos411, 9428A410j 4/6/94 21:58:22";
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
 * 
 * scan.c,v $ $Revision: 1.3.7.8 $ (OSF) $Date: 1992/09/14 13:53:09 $
 */
#define _ILS_MACROS

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "semstack.h"
#include "symtab.h"
#include "gram.h"
#include "err.h"
/* #define DEBUG */

#define YYTEXT_SIZE  16384	/* line continuation requires big bufs.    */
				/* YYTEXT_SIZE should be at least LINE_MAX */
char yytext[YYTEXT_SIZE+1];
char sym_text[ST_MAX_SYM_LEN+1];
char escape_char = '\\';
int  value = 0;
int  skip_to_EOL=0;		/* flag to ignore characters til EOL      */
int  isa_blank_line=TRUE;	/* is the current line blank?             */
int  wasa_blank_line=TRUE;	/* was the last line blank?               */

extern int no_strings;          /* if TRUE, we should ignore strings, and */
                                /* simply return the quote char as we run */
                                /* across it.  gram.y will handle strings */
extern int no_punct;            /* if TRUE, we should ignore most punctuation */
				/* and return ( ) and , as char values    */

#define MAX_KW_LEN  32
typedef struct {
    char key[MAX_KW_LEN+1];
    int  token_val;
} keyword_t;

/*
 * Keywords for lexical scan.  THIS TABLE MUST BE IN ASCII ALPHABETIC
 * ORDER.  bsearch() is used to look things up in it.
 */
keyword_t kw_tbl[]={
"<code_set_name>",   KW_CODESET,
"<comment_char>",    KW_COMMENT_CHAR,
"<escape_char>",     KW_ESC_CHAR,
"<mb_cur_max>",      KW_MB_CUR_MAX,
"<mb_cur_min>",      KW_MB_CUR_MIN,
"CHARMAP",           KW_CHARMAP,
"CHARSETID",         KW_CHARSETID,
"CSID_STD",	     KW_CSID_STD,
"DISPWID",           KW_DISPWID,
"END",               KW_END,
"FNMATCH_C",	     KW_FNMATCH_C,
"FNMATCH_STD",       KW_FNMATCH_STD,
"GET_WCTYPE_STD",    KW_GET_WCTYPE_STD,
"IS_WCTYPE_SB",      KW_IS_WCTYPE_SB,
"IS_WCTYPE_STD",     KW_IS_WCTYPE_STD,
"LC_COLLATE",        KW_LC_COLLATE,
"LC_CTYPE",          KW_LC_CTYPE,
"LC_MESSAGES",       KW_LC_MSG,
"LC_MONETARY",       KW_LC_MONETARY,
"LC_NUMERIC",        KW_LC_NUMERIC,
"LC_TIME",           KW_LC_TIME,
"LOCALECONV_STD",    KW_LOCALECONV_STD,
"MBLEN_932",	     KW_MBLEN_932,
"MBLEN_EUCJP",	     KW_MBLEN_EUCJP,
"MBLEN_SB",	     KW_MBLEN_SB,
"MBSTOWCS_932",	     KW_MBSTOWCS_932,
"MBSTOWCS_EUCJP",    KW_MBSTOWCS_EUCJP,
"MBSTOWCS_SB",	     KW_MBSTOWCS_SB,
"MBTOWC_932",        KW_MBTOWC_932,
"MBTOWC_EUCJP",      KW_MBTOWC_EUCJP,
"MBTOWC_SB",         KW_MBTOWC_SB,
"METHODS",           KW_METHODS,
"REGCOMP_STD",	     KW_REGCOMP_STD,
"REGERROR_STD",	     KW_REGERROR_STD,
"REGEXEC_STD",	     KW_REGEXEC_STD,
"REGFREE_STD",	     KW_REGFREE_STD,
"RPMATCH_C",	     KW_RPMATCH_C,
"RPMATCH_STD",	     KW_RPMATCH_STD,
"STRCOLL_C",	     KW_STRCOLL_C,
"STRCOLL_SB",	     KW_STRCOLL_SB,
"STRCOLL_STD",	     KW_STRCOLL_STD,
"STRFMON_STD",	     KW_STRFMON_STD,
"STRFTIME_STD",	     KW_STRFTIME_STD,
"STRPTIME_STD",	     KW_STRPTIME_STD,
"STRXFRM_C",	     KW_STRXFRM_C,
"STRXFRM_SB",	     KW_STRXFRM_SB,
"STRXFRM_STD",	     KW_STRXFRM_STD,
"TOWLOWER_STD",	     KW_TOWLOWER_STD,
"TOWUPPER_STD",      KW_TOWUPPER_STD,
"WCSCOLL_C",	     KW_WCSCOLL_C,
"WCSCOLL_STD",	     KW_WCSCOLL_STD,
"WCSFTIME_STD",	     KW_WCSFTIME_STD,
"WCSID_STD",         KW_WCSID_STD,
"WCSTOMBS_932",	     KW_WCSTOMBS_932,
"WCSTOMBS_EUCJP",    KW_WCSTOMBS_EUCJP,
"WCSTOMBS_SB",       KW_WCSTOMBS_SB,
"WCSWIDTH_932",      KW_WCSWIDTH_932,
"WCSWIDTH_EUCJP",    KW_WCSWIDTH_EUCJP,
"WCSWIDTH_LATIN",    KW_WCSWIDTH_LATIN,
"WCSXFRM_C",	     KW_WCSXFRM_C,
"WCSXFRM_STD",	     KW_WCSXFRM_STD,
"WCTOMB_932",	     KW_WCTOMB_932,
"WCTOMB_EUCJP",      KW_WCTOMB_EUCJP,
"WCTOMB_SB",	     KW_WCTOMB_SB,
"WCWIDTH_932",	     KW_WCWIDTH_932,
"WCWIDTH_EUCJP",     KW_WCWIDTH_EUCJP,
"WCWIDTH_LATIN",     KW_WCWIDTH_LATIN,
"__MBSTOPCS_932",    KW_MBSTOPCS_932,
"__MBSTOPCS_EUCJP",  KW_MBSTOPCS_EUCJP,
"__MBSTOPCS_SB",     KW_MBSTOPCS_SB,
"__MBTOPC_932",      KW_MBTOPC_932,
"__MBTOPC_EUCJP",    KW_MBTOPC_EUCJP,
"__MBTOPC_SB",	     KW_MBTOPC_SB,
"__PCSTOMBS_932",    KW_PCSTOMBS_932,
"__PCSTOMBS_EUCJP",  KW_PCSTOMBS_EUCJP,
"__PCSTOMBS_SB",     KW_PCSTOMBS_SB,
"__PCTOMB_932",      KW_PCTOMB_932,
"__PCTOMB_EUCJP",    KW_PCTOMB_EUCJP,
"__PCTOMB_SB",	     KW_PCTOMB_SB,
"__mbstopcs",	     KW_MBSTOPCS,
"__mbtopc",	     KW_MBTOPC,
"__pcstombs",	     KW_PCSTOMBS,
"__pctomb",	     KW_PCTOMB,
"abday",             KW_ABDAY,
"abmon",             KW_ABMON,
"alt_digits",        KW_ALT_DIGITS,
"am_pm",             KW_AM_PM,
"backward",          KW_BACKWARD,
"charclass",         KW_CHARCLASS,
"collating-element", KW_COLLATING_ELEMENT,
"collating-symbol",  KW_COLLATING_SYMBOL,
"copy",              KW_COPY,
"credit_sign",       KW_CREDIT_SIGN,
"csid",		     KW_CSID,
"currency_symbol",   KW_CURRENCY_SYMBOL,
"d_fmt",             KW_D_FMT,
"d_t_fmt",           KW_D_T_FMT,
"day",               KW_DAY,
"debit_sign",        KW_DEBIT_SIGN,
"decimal_point",     KW_DECIMAL_POINT,
"era",               KW_ERA,
"era_d_fmt",         KW_ERA_D_FMT,
"era_d_t_fmt",       KW_ERA_D_T_FMT,
"era_t_fmt",         KW_ERA_T_FMT,
"era_year",          KW_ERA_YEAR,
"fnmatch",	     KW_FNMATCH,
"forward",           KW_FORWARD,
"frac_digits",       KW_FRAC_DIGITS,
"from",              KW_FROM,
"get_wctype",	     KW_GET_WCTYPE,
"grouping",          KW_GROUPING,
"int_curr_symbol",   KW_INT_CURR_SYMBOL,
"int_frac_digits",   KW_INT_FRAC_DIGITS,
"is_wctype",	     KW_IS_WCTYPE,
"left_parenthesis",  KW_LEFT_PARENTHESIS,
"mblen",	     KW_MBLEN,
"mbstowcs",	     KW_MBSTOWCS,
"mbtowc",	     KW_MBTOWC,
"mon",               KW_MON,
"mon_decimal_point", KW_MON_DECIMAL_POINT,
"mon_grouping",      KW_MON_GROUPING,
"mon_thousands_sep", KW_MON_THOUSANDS_SEP,
"n_cs_precedes",     KW_N_CS_PRECEDES,
"n_sep_by_space",    KW_N_SEP_BY_SPACE,
"n_sign_posn",       KW_N_SIGN_POSN,
"negative_sign",     KW_NEGATIVE_SIGN,
"nlldate",           KW_NLLDATE,
"nltmisc",           KW_NLTMISC,
"nltstr",            KW_NLTSTR,
"nltunits",          KW_NLTUNITS,
"nlyear",            KW_NLYEAR,
/* "no-substitute",     KW_NO_SUBSTITUTE, */
"noexpr",	     KW_NOEXPR,
"nostr",             KW_NOSTR,
"order_end",         KW_ORDER_END,
"order_start",       KW_ORDER_START,
"p_cs_precedes",     KW_P_CS_PRECEDES,
"p_sep_by_space",    KW_P_SEP_BY_SPACE,
"p_sign_posn",       KW_P_SIGN_POSN,
"position",          KW_POSITION,
"positive_sign",     KW_POSITIVE_SIGN,
"regcomp",	     KW_REGCOMP,
"regerror",	     KW_REGERROR,
"regexec",	     KW_REGEXEC,
"regfree",	     KW_REGFREE,
"right_parenthesis", KW_RIGHT_PARENTHESIS,
"rpmatch",	     KW_RPMATCH,
"strcoll",	     KW_STRCOLL,
"strfmon",	     KW_STRFMON,
"strftime",	     KW_STRFTIME,
"strptime",	     KW_STRPTIME,
"strxfrm",	     KW_STRXFRM,
/* "substitute",        KW_SUBSTITUTE, */
"t_fmt",             KW_T_FMT,
"t_fmt_ampm",        KW_T_FMT_AMPM,
"thousands_sep",     KW_THOUSANDS_SEP,
"tolower",           KW_TOLOWER,
"toupper",           KW_TOUPPER,
"towlower",	     KW_TOWLOWER,
"towupper",	     KW_TOWUPPER,
"wcscoll",	     KW_WCSCOLL,
"wcsftime",	     KW_WCSFTIME,
"wcsid",	     KW_WCSID,
"wcstombs",	     KW_WCSTOMBS,
"wcswidth",	     KW_WCSWIDTH,
"wcsxfrm",	     KW_WCSXFRM,
"wctomb",	     KW_WCTOMB,
"wcwidth",	     KW_WCWIDTH,
/* "with",              KW_WITH, */
"yesexpr",           KW_YESEXPR,
"yesstr",            KW_YESSTR,
};

#define KW_TBL_SZ  (sizeof(kw_tbl) / sizeof(kw_tbl[0]))
#define NIL(t)     ((t *)0)

void scan_error(int);

/*
SYM [0-9A-Za-z_-]
LTR [a-zA-Z_-]
HEX [0-9a-fA-F]
*/

/* character position tracking for error messages */
int yycharno=1;
int yylineno=1;
static char comment_char = '#';
static int  seenfirst=0;	/* Once LC_<category> seen, the escape_char and
                                   comment_char cmds are no longer permitted */
static int eol_pos=0;
static int peekc = 0;
static char locale_name[PATH_MAX+1];	/* for copy directive */
static int copyflag = 0;		/* for copy directive */

void initlex(void) {
    comment_char = '#';
    escape_char = '\\';

    yycharno = 1;
    yylineno = 1;
}


static int getspecialchar(int errcode) {
    int c;

    /* Eat white-space */
    c=input();
    while (isspace(c) && (c != '\n'))
	c=input();

    if (c != '\n') {
        if (c >127)	/* VERY ASCII dependent: won't work for EBCDIC, etc */
            diag_error(ERR_CHAR_NOT_PCS,c);
    } else
        diag_error(errcode);

    while (input() !='\n');
    wasa_blank_line = isa_blank_line;
    isa_blank_line = TRUE;  	/* since we are skipping to next line */
    return(c);
}

int input(void)
{
    int c;

    if (peekc != 0) {
	c = peekc;
	peekc = 0;
	return c;
    }
	
    while ((c=getchar()) != EOF) {

	/* if we see an escape char check if it's 
	 * for line continuation char 
	 */
	if (c==escape_char) {
	    c = getchar();

	    if (c!='\n') {
		/* oops - not end-of-line, put it back. */
		peekc = c;
		return escape_char;
	    } else {
		yylineno++;
		eol_pos = yycharno;
		yycharno = 1;
		continue;
	    }
	}

        /* eat comments */
	/* to support in-line comments, remove   */
	/* "&& (yycharno==1)" from the next line */
        if ((c==comment_char) && (yycharno==1))
	    while ((c=getchar()) != '\n') {
                if(c==EOF) return c;
            }

	if (c=='\n') {
	    yylineno++;
	    eol_pos = yycharno;

	    /* 
	      Don't return '\n' if empty line 
	    */
	    if ((yycharno==1) || (isa_blank_line == TRUE)) {
		wasa_blank_line = TRUE;
		yycharno=1;
		continue;
	    }
	    wasa_blank_line = isa_blank_line;
	    isa_blank_line = TRUE;
	    yycharno = 1;
	} else 
	    yycharno ++;

	return c;
    }
    
    return c;
}


void uninput(int c)
{
    int  t;

    if (peekc != 0) {		/* already one char pushed back */
	ungetc(peekc, stdin);
	t = peekc;
	peekc = c;
	c = t;
    } else
	ungetc(c, stdin);

    if (c!='\n')
	yycharno--;
    else {
	yylineno--;
	yycharno = eol_pos;
	isa_blank_line=wasa_blank_line;
    }
}
 
	
#ifdef DEBUG
#define RETURN(t) do {  \
    fprintf(stderr,"isa_blk_ln=%d ",isa_blank_line); \
    switch(t) {         \
      case INT_CONST:   fprintf(stderr,"INT %s\n",yytext);      break;  \
      case CHAR_CONST:  fprintf(stderr,"CHAR '%c' (X%x)\n",value,value);break;\
      case SYMBOL:      fprintf(stderr,"SYMB %s\n",sym_text);   break;  \
      case STRING:      fprintf(stderr,"STR %s\n",yytext);      break;  \
      case LOC_NAME:    fprintf(stderr,"LOC_NAME %s\n",yytext); break;  \
      case EOF:         fprintf(stderr,"EOF\n"); break;                 \
      case KW_COPY:     fprintf(stderr,"KW_COPY\n"); break;             \
      default:  {                                                       \
                        if (isascii(t)) {                               \
                            fprintf(stderr,"'%c'\n",t);                 \
                        } else {                                        \
                            fprintf(stderr,"KEYW %s\n", yytext);        \
                        }                                               \
                }                                                       \
    }                                                                   \
    return (t);                                                         \
    } while (0)

#else
#define RETURN(t)       return(t)
#endif /* DEBUG */


int yylex(void)
{
    int yypos;
    int c;
    int escaped;		/* state variable: following char is escaped */
    int in_symbol = FALSE;	/* is a symbol is being parsed in a string?  */

    /*
     * If we have just processed a 'copy' keyword:
     *   locale_name[0] == '"'  - let the quoted string routine handle it
     *   locale_name[0] == '\0' - We hit and EOF while getting the locale
     *   else
     *    we have an unquoted locale name which we already read
     */
    if (copyflag && locale_name[0] != '"') {
        copyflag = 0;
        if (locale_name[0] == '\0')
            RETURN(EOF);

        for (yypos = 0; locale_name[yypos] != '\0'; yypos++)
            yytext[yypos] = locale_name[yypos];
        yytext[yypos] = '\0';

        RETURN(LOC_NAME);
    }

    if (skip_to_EOL) {
        /*
         * Ignore all text til we reach a new line.
         * (for example, in char maps, after the <encoding>
         */
        skip_to_EOL = 0;
        
        while ((c=input()) != EOF) {
            if (c == '\n')
                RETURN(c);
        }
        RETURN(EOF);
    }

    while ((c = input()) != EOF) {

	if (!isspace(c))
		isa_blank_line = FALSE;

	/* 
	        [0-9]+
	        | [-+][0-9]+ 
	*/
	if (isdigit(c) || c=='-' || c=='+') {
	    yytext[yypos=0] = c;
	    c=input();
	    while (isdigit(c)) {
		yytext[++yypos] = c;
		c=input();
	    }

	    if ((c != EOF && !isspace(c)) || c == '\n')
		uninput(c);

	    if (isdigit(yytext[0]) || yypos >0) {
		/* multiple chars in string or single digit */
		yytext[++yypos] = '\0';
		RETURN(INT_CONST);
	    } else {
		/* single char - should be digit. */
		yytext[++yypos] = '\0';

		/* if single char and not digit return char */
		if (!isdigit(yytext[0]))
		    RETURN(yytext[0]);
	    }
	}

	
	/* '\\'
	        \\d[0-9]{1,3}
		\\0[0-8]{1,3}
		\\[xX][0-9a-fA-F]{2}
	        \\[^xX0d]
	*/
	if (c==escape_char) {
	    char *junk;
	    char number[80];
	    char *s;

	    value = 0;
	    yypos = -1;

	    do {
		yytext[++yypos] = c;

		c = input();
		yytext[++yypos] = c;

		value <<= 8;

		switch (c) {
		  case 'd':  /* decimal constant */
		    for (c=input(),s=number; isdigit(c); c=input())
			*s++ = yytext[++yypos] = c;

		    *s++ = '\0';

		    if ((c != EOF && !isspace(c)) || c == '\n')
			uninput(c);

		    if ((s - number) > 4) {
			/* too many digits in decimal constant */
		        yytext[yypos+1] = '\0';
			diag_error(ERR_ILL_DEC_CONST,yytext);
		    }

		    value += strtol(number, &junk, 10);
		    continue;
		  
		  case 'x':  /* hex constant */
		  case 'X':
		    for (c=input(),s=number; isxdigit(c); c=input())
			*s++ = yytext[++yypos] = c;
		    *s++ = '\0';

		    if ((c != EOF && !isspace(c)) || c == '\n')
			uninput(c);

		    if ((s - number) > 3) {
			/*  wrong number of digits in hex const */
			yytext[yypos+1] = '\0';
			diag_error(ERR_ILL_HEX_CONST,yytext);
		    }

		    value += strtol(number, &junk, 16);
		    continue;

		  default:   
		    if (isdigit(c)) {		/* octal constant */	
		        for (s=number; c >= '0' && c <= '7' && c != EOF; 
			     c=input())
			    *s++ = yytext[++yypos] = c;
		        *s++ = '\0';
			if (c == '8' || c == '9'){
			    while (isdigit(c)){	/*invalid oct */
			       yytext[++yypos] = c;
			       c = input();
			    }
			    yytext[yypos+1] = '\0';
			    diag_error(ERR_ILL_OCT_CONST,yytext);
			}
		
		        if ((c != EOF && !isspace(c)) || c == '\n')
			    uninput(c);

		        if ((s - number) > 4) {
			    /* too many digits in octal constant */
			    yytext[yypos+1] = '\0';
			    diag_error(ERR_ILL_OCT_CONST,yytext);
			}

		        value += strtol(number, &junk, 8);
		        continue;
		    }
		    else {             /* escaped character, e.g. \\ */
		        yytext[++yypos] = '\0';
		        RETURN(c);
		    }
		}
	    } while ((c=input()) == escape_char);

	    yytext[++yypos] = '\0';

	    if ((c != EOF && !isspace(c)) || c == '\n')
		uninput(c);

	    RETURN(CHAR_CONST);
	}

        if ((no_strings == TRUE) && (c == '"'))
            RETURN('"');

	/* 
	      symbol for character names - or keyword:

	      < [:isgraph:]+ >
	*/
	if (c=='<') {
	    keyword_t *kw;
	    
	    yytext[yypos=0] = c;
	    do {
		c = input();
		if (c==escape_char) {
		    c = input();
                    escaped = 1;
                    if (c == '>' || c==escape_char) /*escaped chrs for symbols*/
                        yytext[++yypos] = escape_char;
                    }
                else
                    escaped = 0;
		yytext[++yypos] = c;
	    } while (((c != '>') || escaped) && isgraph(c));
	    yytext[++yypos] = '\0';
	    if (c != '>') {
		uninput(c);
	 	yytext[yypos-1] = '\0';
		diag_error(ERR_ILL_CHAR_SYM, yytext);
		yytext[yypos-1] = '>';
		yytext[yypos] = '\0';
	    }

	    /* look for one of the special 'meta-symbols' */
	    kw = bsearch(yytext, kw_tbl, 
			 KW_TBL_SZ, sizeof(keyword_t), strcmp);
	    
	    if (kw != NULL) {
		/* check for escape character replacement. */
		
		if (kw->token_val == KW_ESC_CHAR) {
                    escape_char = getspecialchar(ERR_ESC_CHAR_MISSING);
		    continue;
		} 
	        else
		    if (kw->token_val == KW_COMMENT_CHAR) {
                        comment_char = getspecialchar(ERR_COM_CHAR_MISSING);
			continue;
		    }
		    else
		        RETURN(kw->token_val);
	    } 
	    else {
		strncpy(sym_text, yytext, sizeof(sym_text));
		RETURN(SYMBOL);
	    }
	}
	
	
	/* 
	    symbol for character class names - or keyword.
	  
	    [:alpha:_]+[:digit::alpha:_-]
	*/
	if (isalpha(c) || c == '_') {
	    keyword_t *kw;
	    
	    yytext[yypos=0] = c;
	    c=input();
	    while (isalnum(c) || c=='_' || c == '-')  {
		yytext[++yypos] = c;
		c=input();
	    }
	    yytext[++yypos] = '\0';

	    if ((c != EOF && !isspace(c)) || c == '\n')
		uninput(c);

	    kw = bsearch(yytext, kw_tbl, 
			 KW_TBL_SZ, sizeof(keyword_t), strcmp);
	    
            
	    if (kw != NULL) {
                switch (kw->token_val) {
                  case KW_LC_COLLATE:
                  case KW_LC_CTYPE:
                  case KW_LC_MSG:
                  case KW_LC_MONETARY:
                  case KW_LC_NUMERIC:
                  case KW_LC_TIME:
                    seenfirst++;
                    RETURN(kw->token_val);

                  case KW_COPY:
                    {
                        char *ptr, *endp;

                        copyflag = 1;
			c=input();
                        while (isspace(c) && c != '\n')
			    c=input();

                        if (c == EOF) {
                            locale_name[0] = '\0' ;  /*indicate EOF for later*/
                            RETURN(KW_COPY);
                        }

                        if (c == '"' || c == '\n') {
                            locale_name[0] = c; /* indicate quotes for later */
                            uninput(c);
                            RETURN(KW_COPY);
                        }
                                                /* save locale name for later */
                        ptr = locale_name;
                        endp = &locale_name[PATH_MAX];
                        do {
                            *ptr++ = c;
                            if (ptr > endp)
                                error(NAME_TOO_LONG, PATH_MAX);
                        } while (((c=input()) != EOF) && !isspace(c));
                        *ptr++ = '\0';
                        uninput(c);
                        RETURN(KW_COPY);
                    }

                  default:
                    RETURN(kw->token_val);
                } /* switch */

            } else if ( !seenfirst && !strcmp(yytext,"comment_char") && yycharno == 14) {
                comment_char = getspecialchar(ERR_COM_CHAR_MISSING);
                continue;

            } else if ( !seenfirst && !strcmp(yytext,"escape_char") && yycharno == 13) {
                escape_char = getspecialchar(ERR_ESC_CHAR_MISSING);
                continue;

	    } else {
	 	strncpy(sym_text, yytext, sizeof(sym_text));
	 	RETURN(SYMBOL);	
	    }
	}
	
	
	/* 
	    strings! replacement of escaped constants and <char> references
	    are handled in copy_string() in sem_chr.c
	    
	    Replace all escape_char with \ so that strings will have a
	    common escape_char for locale and processing in other locations.
	  
	    "[^\n"]*"
        */

	if (c == '"') {

            if (no_strings == TRUE)
                RETURN('"');

	    yypos = 0;
            in_symbol = FALSE;
            do {
	        while ((c=input()) != '"' && c != '\n' && c != EOF) {
	 	    if (c == escape_char){
		        yytext[yypos++] = escape_char;
		        if ((c = input()) != EOF)
		            yytext[yypos++] = c;
		        else {
			    yytext[yypos] = '\0';
			    break;
		        }
		    }
		    else {
	   	        yytext[yypos++] = c;

                        if (c == '>')
                            in_symbol = FALSE;      /* not in a symbol */
                        else
                            if (c == '<')
                                in_symbol = TRUE;   /* in a symbol */
                           
                    }
	        }

                if (c == '\n' || c == EOF) {
                    yytext[yypos] = '\0';
                    error(ERR_BAD_SYMBOL_STRING,yytext);
                }
 
                if (c == '"' && in_symbol == TRUE)
	   	        yytext[yypos++] = c;	/* add quotes in symbols */

            } while (in_symbol == TRUE);/* don't terminate string mid-symbol */

	    yytext[yypos] = '\0';

	    if (c=='\n' || c==EOF)
		diag_error(ERR_MISSING_QUOTE,yytext);
	    
            if (copyflag) {
                copyflag = 0;
                RETURN(LOC_NAME);
            }

	    RETURN(STRING);
	}

	if (c=='.') {
	    yytext[yypos=0] = c;
	    while (yypos < 2 && (c=input()) == '.') 
		yytext[++yypos] = c;
	    yytext[++yypos] = '\0';
	    if (strcmp(yytext, "...")==0)
		RETURN(KW_ELLIPSIS);
	    else {
		diag_error(ERR_UNKNOWN_KWD, yytext);
		continue;
	    }
	}

	/* 
	  The newline is this grammar statement terminator - yuk!
	*/
	if (c=='\n')
	    RETURN(c);

	if (isspace(c))
	    continue;

	if (c==';' || c=='(' || c==')' || c==',') {
	    if ((c == ';') || (no_punct == FALSE)) {
                /* return punctuation if it is a semicolon, or if */
                /* no_punct FALSE, else treat like character data */
	        yytext[0] = c;
	        RETURN(c);
            }
	}

	value = c;
        RETURN(CHAR_CONST);
	
	/* XPG4 specified that characters can be defined as themselves     */
	/* If character is invalid in the grammar, yacc will barf for us   */

	/* diag_error(ERR_ILL_CHAR, c); */

    } /* while c != EOF */

    RETURN(EOF);
}

