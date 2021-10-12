/* @(#)17	1.10.1.6  src/bos/usr/bin/localedef/gram.y, cmdnls, bos410 4/17/94 23:04:27 */
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
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
 * gram.y,v $ $Revision: 1.4.7.7 $ (OSF) $Date: 1992/11/20 02:37:52 $
 */

/* ----------------------------------------------------------------------
** Tokens for keywords
** ----------------------------------------------------------------------*/

/* keywords identifying the beginning and end of a locale category */
%token KW_END
%token KW_CHARMAP
%token KW_CHARSETID
%token KW_LC_COLLATE
%token KW_LC_CTYPE
%token KW_LC_MONETARY
%token KW_LC_NUMERIC
%token KW_LC_MSG
%token KW_LC_TIME
%token KW_METHODS
%token KW_DISPWID
%token KW_COPY

/* keywords support LC_COLLATE category */
%token KW_COLLATING_ELEMENT
%token KW_COLLATING_SYMBOL
%token KW_ORDER_START
%token KW_ORDER_END
%token KW_FORWARD
%token KW_BACKWARD
/* %token KW_NO_SUBSTITUTE */
%token KW_POSITION
/* %token KW_WITH */
%token KW_FROM
/* %token KW_SUBSTITUTE */

/* keywords supporting LC_CTYPE category */
%token KW_ELLIPSIS
%token KW_TOUPPER
%token KW_TOLOWER
%token KW_CHARCLASS

/* keywords supporting the LC_MONETARY category */
%token KW_INT_CURR_SYMBOL
%token KW_CURRENCY_SYMBOL
%token KW_MON_DECIMAL_POINT
%token KW_MON_THOUSANDS_SEP
%token KW_MON_GROUPING
%token KW_POSITIVE_SIGN
%token KW_NEGATIVE_SIGN
%token KW_INT_FRAC_DIGITS
%token KW_FRAC_DIGITS
%token KW_P_CS_PRECEDES
%token KW_P_SEP_BY_SPACE
%token KW_N_CS_PRECEDES
%token KW_N_SEP_BY_SPACE
%token KW_P_SIGN_POSN
%token KW_N_SIGN_POSN
%token KW_DEBIT_SIGN
%token KW_CREDIT_SIGN
%token KW_LEFT_PARENTHESIS
%token KW_RIGHT_PARENTHESIS

/* keywords supporting the LC_NUMERIC category */
%token KW_DECIMAL_POINT
%token KW_THOUSANDS_SEP
%token KW_GROUPING

/* keywords supporting the METHODS category */
%token KW_CSID
%token KW_FNMATCH
%token KW_GET_WCTYPE
%token KW_IS_WCTYPE
%token KW_MBLEN
%token KW_MBSTOPCS
%token KW_MBSTOWCS
%token KW_MBTOPC
%token KW_MBTOWC
%token KW_PCSTOMBS
%token KW_PCTOMB
%token KW_REGCOMP
%token KW_REGERROR
%token KW_REGEXEC
%token KW_REGFREE
%token KW_RPMATCH
%token KW_STRCOLL
%token KW_STRFMON
%token KW_STRFTIME
%token KW_STRPTIME
%token KW_STRXFRM
%token KW_TOWLOWER
%token KW_TOWUPPER
%token KW_WCSCOLL
%token KW_WCSFTIME
%token KW_WCSID
%token KW_WCSTOMBS
%token KW_WCSWIDTH
%token KW_WCSXFRM
%token KW_WCTOMB
%token KW_WCWIDTH

/* keywords supporting the LC_TIME category */
%token KW_ABDAY
%token KW_DAY
%token KW_ABMON
%token KW_MON
%token KW_D_T_FMT
%token KW_D_FMT
%token KW_T_FMT
%token KW_AM_PM
%token KW_ERA
%token KW_ERA_YEAR
%token KW_ERA_D_FMT
%token KW_ERA_T_FMT
%token KW_ERA_D_T_FMT
%token KW_ALT_DIGITS
%token KW_T_FMT_AMPM
%token KW_NLLDATE
%token KW_NLTMISC
%token KW_NLTSTR
%token KW_NLTUNITS
%token KW_NLYEAR

/* keywords for the LC_MSG category */
%token KW_YESEXPR
%token KW_NOEXPR
%token KW_YESSTR
%token KW_NOSTR

/* keywords for the METHODS section global names */
%token KW_CSID_STD
%token KW_FNMATCH_C
%token KW_FNMATCH_STD
%token KW_GET_WCTYPE_STD
%token KW_IS_WCTYPE_SB
%token KW_IS_WCTYPE_STD
%token KW_LOCALECONV_STD
%token KW_MBLEN_932
%token KW_MBLEN_EUCJP
%token KW_MBLEN_SB
%token KW_MBSTOPCS_932
%token KW_MBSTOPCS_EUCJP
%token KW_MBSTOPCS_SB
%token KW_MBSTOWCS_932
%token KW_MBSTOWCS_EUCJP
%token KW_MBSTOWCS_SB
%token KW_MBTOPC_932
%token KW_MBTOPC_EUCJP
%token KW_MBTOPC_SB
%token KW_MBTOWC_932
%token KW_MBTOWC_EUCJP
%token KW_MBTOWC_SB
%token KW_MONETARY_INIT
%token KW_NL_MONINFO
%token KW_NL_NUMINFO
%token KW_NL_RESPINFO
%token KW_NL_TIMINFO
%token KW_PCSTOMBS_932
%token KW_PCSTOMBS_EUCJP
%token KW_PCSTOMBS_SB
%token KW_PCTOMB_932
%token KW_PCTOMB_EUCJP
%token KW_PCTOMB_SB
%token KW_REGCOMP_STD
%token KW_REGERROR_STD
%token KW_REGEXEC_STD
%token KW_REGFREE_STD
%token KW_RPMATCH_C
%token KW_RPMATCH_STD
%token KW_STRCOLL_C
%token KW_STRCOLL_SB
%token KW_STRCOLL_STD
%token KW_STRFMON_STD
%token KW_STRFTIME_STD
%token KW_STRPTIME_STD
%token KW_STRXFRM_C
%token KW_STRXFRM_SB
%token KW_STRXFRM_STD
%token KW_TOWLOWER_STD
%token KW_TOWUPPER_STD
%token KW_WCSCOLL_C
%token KW_WCSCOLL_STD
%token KW_WCSFTIME_STD
%token KW_WCSID_STD
%token KW_WCSTOMBS_932
%token KW_WCSTOMBS_EUCJP
%token KW_WCSTOMBS_SB
%token KW_WCSWIDTH_932
%token KW_WCSWIDTH_EUCJP
%token KW_WCSWIDTH_LATIN
%token KW_WCSXFRM_C
%token KW_WCSXFRM_STD
%token KW_WCTOMB_932
%token KW_WCTOMB_EUCJP
%token KW_WCTOMB_SB
%token KW_WCWIDTH_932
%token KW_WCWIDTH_EUCJP
%token KW_WCWIDTH_LATIN

/* tokens for meta-symbols */
%token KW_CODESET
%token KW_ESC_CHAR
%token KW_MB_CUR_MAX
%token KW_MB_CUR_MIN
%token KW_COMMENT_CHAR

/* tokens for user defined symbols, integer constants, etc... */
%token SYMBOL
%token STRING
%token INT_CONST
%token DEC_CONST
%token OCT_CONST
%token HEX_CONST
%token CHAR_CONST
%token LOC_NAME

%{
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include "err.h"
#include "symtab.h"
#include "semstack.h"
#include "locdef.h"
#include "method.h"

symtab_t cm_symtab;

extern _LC_charmap_t  charmap;
extern _LC_collate_t  collate;
extern _LC_ctype_t    ctype;
extern _LC_monetary_t monetary;
extern _LC_numeric_t  numeric; 
extern _LC_time_t     lc_time;
extern _LC_resp_t     resp;
extern _LC_aix31_t    lc_aix31;

extern int next_bit;  /* sem_ctype.c:holds next bitmask for ctype properties */

extern char yytext[];
extern char sym_text[];
extern int private_table;
extern int skip_to_EOL;
extern char * max_wchar_symb;
int mb_cur_max=0;
int method_class=SB_CODESET;
int max_disp_width=0;
int sort_mask = 0;
int cur_order = 0;
int global_table = FALSE;
int g_tbl_index=-1;
char g_method_name[MAX_METHOD_NAME];

int no_strings = FALSE;  /* if this is set to TRUE, then scan.c will not   */
                         /* treat strings specially, it will simply return */
                         /* the quote character				   */
int no_punct = FALSE;    /* if this is set to TRUE, then scan.c will return*/
                         /* all punctuation except double quote and        */
                         /* semicolon as data characters rather than       */
                         /* punctuation.                                   */

#define SET_METHOD(n)	( (private_table) ? (set_method_private(n)) : \
		 (set_method_global(n, g_tbl_index, g_method_name)))


/* Flags for determining if the category was empty when it was defined and
   if it has been defined before */

static int lc_time_flag = 0;
static int lc_monetary_flag = 0;
static int lc_ctype_flag = 0;
static int lc_message_flag = 0;
static int lc_numeric_flag = 0;
static int arblen;
static int user_defined = 0;

#define MAX_WEIGHT_STACK	16	/* actually 4 should be big enough! */

int current_weight=0;
_LC_weight_t weight_array[MAX_WEIGHT_STACK];

_LC_weight_t weights;		/* weights specified by r.h.s.               */
_LC_weight_t weights_1;		/* weights specified by r.h.s. (tmp storage) */
_LC_weight_t weights_2;		/* weights specified by r.h.s. (tmp storage) */

symbol_t * char_sym1;		/* for ellipsis endpoint that has a weight   */
symbol_t * char_sym2;		/* for ellipsis endpoint that has a weight   */

symbol_t *coll_tgt = NULL;

/* prototypes */
_LC_weight_t pop_wgt();
void push_wgt(_LC_weight_t);

%}

%%
/* ----------------------------------------------------------------------
** GRAMMAR for files parsed by the localedef utility.  This grammar 
** supports both the CHARMAP and the LOCSRC definitions.  The 
** implementation will call yyparse() twice.  Once to parse the CHARMAP 
** file, and once to parse the LOCSRC file.
** ----------------------------------------------------------------------*/

file    : charmap
    	| category_list
        | method_def
	;

category_list :
   	category_list category
	| category
	;

/* ----------------------------------------------------------------------
** CHARMAP GRAMMAR 
**
** This grammar parses the charmap file as specified by POSIX 1003.2.
** ----------------------------------------------------------------------*/
charmap : charmap charmap_sect
	| charmap_sect
	;

charmap_sect : 
	metasymbol_assign
	| KW_CHARMAP '\n' charmap_stat_list KW_END KW_CHARMAP '\n'
	{
	    check_digit_values();
	}
        | charsets_def
	;

charmap_stat_list : 
        charmap_stat_list charmap_stat
        | charmap_stat
	;

charmap_stat :
	symbol_def
  	| symbol_range_def
	;

symbol_range_def :
	symbol KW_ELLIPSIS symbol byte_list {skip_to_EOL++;} '\n'
	{
	    sem_symbol_range_def();
	}
	;

symbol_def :
	symbol byte_list {skip_to_EOL++;} '\n'
	{
	    sem_symbol_def();
	}
	;

metasymbol_assign : 
	KW_MB_CUR_MAX const '\n'
  	{
	    item_t *it;
	  
	    it = sem_pop();
	    if (it->type != SK_INT)
		INTERNAL_ERROR;
	    sem_push(it);
	    mb_cur_max = it->value.int_no;
	    if (method_class != USR_DEFINED) {
		switch (mb_cur_max) {
		    case 1: method_class = SB_CODESET;
			      break;
		    case 2: method_class = IBM_932;
			      break;
		    case 3: method_class = IBM_eucJP;
			      break;
		    default : method_class = SB_CODESET;
			      break;
	   	}
	    }
	    sem_set_sym_val("<mb_cur_max>", SK_INT);
	}
	| KW_MB_CUR_MIN const '\n'
  	{
	    item_t *it;

	    it = sem_pop();
	    if (it->type != SK_INT)
	       INTERNAL_ERROR;
	    if (it->value.int_no != 1) {
		diag_error(ERR_INV_MB_CUR_MIN,it->value.int_no);
		destroy_item(it);
	    }
	    else {
	        sem_push(it);
	        sem_set_sym_val("<mb_cur_min>", SK_INT);
	    }
	}
	| KW_CODESET string '\n'
  	{
	    item_t *it;
	    int i;

	    /* The code set name must consist of character in the PCS -
	       which is analagous to doing an isgraph in the C locale */

	    it = sem_pop();
	    if (it->type != SK_STR)
	       INTERNAL_ERROR;
	    for (i=0; yytext[i] != '\0'; i++){
	        if (!isgraph(yytext[i]))
		   error(ERR_INV_CODE_SET_NAME,yytext);
	    }
	    sem_push(it);
	    sem_set_sym_val("<code_set_name>", SK_STR);
	    charmap.cm_csname = MALLOC(char,strlen(yytext)+1);
	    strcpy(charmap.cm_csname, yytext);
	}
	;

/* ----------------------------------------------------------------------
** LOCSRC GRAMMAR 
**
** This grammar parses the LOCSRC file as specified by POSIX 1003.2.
** ----------------------------------------------------------------------*/
category : regular_category 
	{
	    if (user_defined)
		diag_error(ERR_USER_DEF);
	}
	| non_reg_category
	;

regular_category : '\n' 
	| lc_collate
	| lc_ctype
	| lc_monetary
	| lc_numeric
	| lc_msg
	| lc_time
  	;

non_reg_category : unrecognized_cat
	;

/* ----------------------------------------------------------------------
** LC_COLLATE
**
** This section parses the LC_COLLATE category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_collate : 
	coll_sect_hdr coll_stats order_spec KW_END KW_LC_COLLATE '\n'
	{ 
	    no_punct = FALSE;
	    sem_collate(); 
	}
        | coll_sect_hdr order_spec KW_END KW_LC_COLLATE '\n'
	{ 
	    no_punct = FALSE;
	    sem_collate(); 
	}
	| coll_sect_hdr KW_COPY locale_name '\n' KW_END KW_LC_COLLATE '\n'
	{
	    copy_locale(LC_COLLATE);
	}
	;

coll_sect_hdr :
    	KW_LC_COLLATE '\n'
	{
	  sem_init_colltbl();
	}
	;

coll_stats :
  	coll_stats coll_stat
	| coll_stat
	;

coll_stat : '\n'
        | KW_COLLATING_ELEMENT symbol KW_FROM string '\n'
    	{
	    sem_def_collel();
	}
	| KW_COLLATING_SYMBOL symbol '\n'
	{
	    sem_spec_collsym();
	}
/*	| KW_SUBSTITUTE string KW_WITH string '\n'	*/
/*	{						*/
/*	    sem_def_substr(-1, TRUE);*/ /*-1 is order passed to set_coll_wgt()*/
/*	}						*/
	;

order_spec : 
	KW_ORDER_START sort_sect initial_ellipsis coll_spec_list terminal_ellipsis KW_ORDER_END
	| KW_ORDER_START sort_sect initial_ellipsis coll_spec_list terminal_ellipsis KW_ORDER_END white_space
	| KW_ORDER_START sort_sect pathological_ellipsis KW_ORDER_END
	| KW_ORDER_START sort_sect pathological_ellipsis KW_ORDER_END white_space
  	;

white_space :
        white_space '\n'
        | '\n'
        ;

sort_sect : '\n'
	{
	    item_t *i;

	    i = create_item(SK_INT, _COLL_FORWARD_MASK);
	    sem_push(i);

	    sem_sort_spec();
	
            no_punct = TRUE;
	}
	| sort_modifier_spec '\n'
	{
	    sem_sort_spec();

            no_punct = TRUE;
	}
	;


sort_modifier_spec :
	sort_modifier_spec ';' sort_modifier_list
	{
	    if (collate.co_nord > _COLL_WEIGHTS_MAX) 
		diag_error(ERR_COLL_WEIGHTS);
	    collate.co_nord ++;
	}
	| sort_modifier_list
	{
	    if (collate.co_nord > _COLL_WEIGHTS_MAX) 
		diag_error(ERR_COLL_WEIGHTS);
	    collate.co_nord ++;
	}
	;

sort_modifier_list :
	sort_modifier_list ',' sort_modifier
	{
	    item_t *i;
	
	    /* The forward and backward mask are mutually exclusive */
	    /* Ignore the second mask and continue processing       */

	    i = sem_pop();
	    if (((i->value.int_no & _COLL_FORWARD_MASK) 
	           && (sort_mask == _COLL_BACKWARD_MASK)) 
	       ||
	       ((i->value.int_no & _COLL_BACKWARD_MASK)
	          && (sort_mask == _COLL_FORWARD_MASK))) {
		diag_error(ERR_FORWARD_BACKWARD);
		sem_push(i);
	    }
	    else {
	        i->value.int_no |= sort_mask;
	        sem_push(i);
	    }
	}
	| sort_modifier
	{
	    item_t *i;

	    i = create_item(SK_INT, sort_mask);
	    sem_push(i);
	}
	;

sort_modifier :
	KW_FORWARD             { sort_mask = _COLL_FORWARD_MASK;  }
	| KW_BACKWARD          { sort_mask = _COLL_BACKWARD_MASK; }
/*	| KW_NO_SUBSTITUTE     { sort_mask = _COLL_NOSUBS_MASK;   } */
	| KW_POSITION          { sort_mask = _COLL_POSITION_MASK; }
	;

initial_ellipsis :
     	| /* OPTIONAL */
        KW_ELLIPSIS '\n' coll_symbol_ref '\n'
	{    /*
              * need to pop current symbol, push a null char symbol for 
              * sem_set_dflt_collwgt_range(), then repush the current symbol
              */
             symbol_t *it = sym_pop();
	     /* NULL is in the portable character set so we  */
             /* are assuming that it exists in the charmap   */
             sem_existing_symbol("<NUL>");
             sem_coll_sym_ref();
             sym_push(it);
             sem_set_dflt_collwgt_range();
             no_strings = FALSE;
        }
        | KW_ELLIPSIS '\n' coll_symbol_ref coll_rhs_list '\n'
	{    /*
              * need to pop current symbol, push a null char symbol for 
              * sem_set_dflt_collwgt_range(), then repush the current symbol
              */
             symbol_t *it = sym_pop();
	     (void)pop_wgt();	/* already have value in "weights" */
	     /* NULL is in the portable character set so we  */
             /* are assuming that it exists in the charmap   */
             sem_existing_symbol("<NUL>");
             sem_coll_sym_ref();
             sym_push(it);
             sem_set_dflt_collwgt_range();

	     /* update endpoint */
             sym_push(it);
	     sem_set_collwgt(&weights);
             no_strings = FALSE;
        }
        | ellipsis coll_rhs_list '\n' coll_symbol_ref '\n'
	{    /*
              * need to pop current symbol, push a null char symbol for 
              * sem_set_dflt_collwgt_range(), then repush the current symbol
              */
             symbol_t *it = sym_pop();
	     (void)pop_wgt();	/* already have value in "weights" */
	     /* NULL is in the portable character set so we  */
             /* are assuming that it exists in the charmap   */
             sem_existing_symbol("<NUL>");
             sem_coll_sym_ref();
             sym_push(it);
             sem_set_collwgt_range(&weights);
             no_strings = FALSE;
	}
        | ellipsis coll_rhs_list '\n' coll_symbol_ref coll_rhs_list '\n'
	{    /*
              * need to pop current symbol, push a null char symbol for 
              * sem_set_dflt_collwgt_range(), then repush the current symbol
              */
             symbol_t *it = sym_pop();
	     (void)pop_wgt();	/* already have value in "weights" */
	     weights_1 = pop_wgt();
	     /* NULL is in the portable character set so we  */
             /* are assuming that it exists in the charmap   */
             sem_existing_symbol("<NUL>");
             sem_coll_sym_ref();
             sym_push(it);
             sem_set_collwgt_range(&weights_1);

	     /* update endpoint */
             sym_push(it);
	     sem_set_collwgt(&weights);
             no_strings = FALSE;
        }
        ;

terminal_ellipsis :
        | /* OPTIONAL */
	coll_symbol_ref '\n' KW_ELLIPSIS '\n'
	{    /*
              * need to create and push a symbol for the character with
              * the largest collating val for sem_set_dflt_collwgt_range()
              * May have problems with ambiguous grammar.
              */
             sem_existing_symbol(max_wchar_symb);
             sem_coll_sym_ref();
             sem_set_dflt_collwgt_range();
             no_strings = FALSE;
        }
	| coll_symbol_ref coll_rhs_list '\n' KW_ELLIPSIS '\n'
	{    /*
              * need to create and push a symbol for the character with
              * the largest collating val for sem_set_dflt_collwgt_range()
              * May have problems with ambiguous grammar.
              */
             symbol_t *it = sym_pop();  /* get copy of symbol */
	     (void)pop_wgt();	/* already have value in "weights" */
             sym_push(it);
             sem_existing_symbol(max_wchar_symb);
             sem_coll_sym_ref();
             sem_set_dflt_collwgt_range();

	     /* update endpoint */
             sym_push(it);
	     sem_set_collwgt(&weights);
             no_strings = FALSE;
        }
	| coll_symbol_ref '\n' ellipsis coll_rhs_list '\n'
	{    /*
              * need to create and push a symbol for the character with
              * the largest collating val for sem_set_dflt_collwgt_range()
              * May have problems with ambiguous grammar.
              */
	     (void)pop_wgt();	/* already have value in "weights" */
             sem_existing_symbol(max_wchar_symb);
             sem_coll_sym_ref();
             sem_set_collwgt_range(&weights);
             no_strings = FALSE;
        }
	| coll_symbol_ref coll_rhs_list '\n' ellipsis coll_rhs_list '\n'
	{    /*
              * need to create and push a symbol for the character with
              * the largest collating val for sem_set_dflt_collwgt_range()
              * May have problems with ambiguous grammar.
              */
             symbol_t *it = sym_pop();  /* get copy of symbol */
             sym_push(it);
	     (void)pop_wgt();	/* already have value in "weights" */
	     weights_1 = pop_wgt();
             sem_existing_symbol(max_wchar_symb);
             sem_coll_sym_ref();
             sem_set_collwgt_range(&weights);

	     /* update endpoint */
             sym_push(it);
	     sem_set_collwgt(&weights_1);
             no_strings = FALSE;
        }
        ;

/* handle the case where a user ONLY specifies a single set of ellipsis */
pathological_ellipsis : 
	KW_ELLIPSIS '\n'
	{
             sem_existing_symbol("<NUL>");
             sem_coll_sym_ref();
             sem_existing_symbol(max_wchar_symb);
             sem_coll_sym_ref();
             sem_set_dflt_collwgt_range();
	}
	| ellipsis coll_rhs_list '\n'
	{
	     (void)pop_wgt();	/* already have value in "weights" */
             sem_existing_symbol("<NUL>");
             sem_coll_sym_ref();
             sem_existing_symbol(max_wchar_symb);
             sem_coll_sym_ref();
             sem_set_collwgt_range(&weights);
             no_strings = FALSE;
	}
        ;


coll_spec_list :
        coll_spec_list coll_symbol_ref '\n'
  	{
	    sem_set_dflt_collwgt();
            no_strings = FALSE;
	}
	| coll_symbol_ref '\n'
  	{
	    sem_set_dflt_collwgt();
            no_strings = FALSE; 
	}
	| coll_spec_list char_coll_range
	| char_coll_range
        | coll_spec_list coll_ell_spec
        | coll_ell_spec
        | coll_spec_list '\n'
        | '\n'
        ;

char_coll_range :
	coll_symbol_ref '\n' KW_ELLIPSIS '\n' coll_symbol_ref '\n'
	{
	    sem_set_dflt_collwgt_range();
            no_strings = FALSE; 
	}
	| coll_symbol_ref coll_rhs_list '\n' KW_ELLIPSIS '\n' coll_symbol_ref '\n'
	{
	    (void)pop_wgt();	/* already have value in "weights" */
	    char_sym2 = sym_pop();
	    char_sym1 = sym_pop();
	    sym_push(char_sym1);
	    sym_push(char_sym2);

	    sem_set_dflt_collwgt_range();

	    /* update endpoint */
            sym_push(char_sym1);
	    sem_set_collwgt(&weights);

            no_strings = FALSE; 
	}
	| coll_symbol_ref '\n' KW_ELLIPSIS '\n' coll_symbol_ref coll_rhs_list '\n'
	{ 
	    (void)pop_wgt();	/* already have value in "weights" */
	    char_sym1 = sym_pop();
	    sym_push(char_sym1);

	    sem_set_dflt_collwgt_range();

	    /* update endpoint */
            sym_push(char_sym1);
	    sem_set_collwgt(&weights);

            no_strings = FALSE; 
	}
	| coll_symbol_ref coll_rhs_list '\n' KW_ELLIPSIS '\n' coll_symbol_ref coll_rhs_list '\n'
	{
	    char_sym2 = sym_pop();
	    char_sym1 = sym_pop();
	    sym_push(char_sym1);
	    sym_push(char_sym2);
	    
	    (void)pop_wgt();	/* already have value in "weights" */
	    weights_1 = pop_wgt();

	    sem_set_dflt_collwgt_range();

	    /* update endpoints */
            sym_push(char_sym1);
	    sem_set_collwgt(&weights_1);
            sym_push(char_sym2);
	    sem_set_collwgt(&weights);

            no_strings = FALSE; 
	}
	| coll_symbol_ref '\n' ellipsis coll_rhs_list '\n' coll_symbol_ref '\n'
	{
	    (void)pop_wgt();	/* already have value in "weights" */
	    sem_set_collwgt_range(&weights);
            no_strings = FALSE; 
	}
	| coll_symbol_ref coll_rhs_list '\n' ellipsis coll_rhs_list '\n' coll_symbol_ref '\n'
	{
	    char_sym2 = sym_pop();
	    char_sym1 = sym_pop();
	    sym_push(char_sym1);
	    sym_push(char_sym2);

	    (void)pop_wgt();	/* already have value in "weights" */
	    weights_1 = pop_wgt();

	    sem_set_collwgt_range(&weights);

	    /* update endpoint */
            sym_push(char_sym1);
	    sem_set_collwgt(&weights_1);

            no_strings = FALSE; 
	}
	| coll_symbol_ref '\n' ellipsis coll_rhs_list '\n' coll_symbol_ref coll_rhs_list '\n'
	{
	    char_sym1 = sym_pop();
	    sym_push(char_sym1);

	    (void)pop_wgt();	/* already have value in "weights" */
	    weights_1 = pop_wgt();

	    sem_set_collwgt_range(&weights_1);

	    /* update endpoint */
            sym_push(char_sym1);
	    sem_set_collwgt(&weights);

            no_strings = FALSE; 
	}
	| coll_symbol_ref coll_rhs_list '\n' ellipsis coll_rhs_list '\n' coll_symbol_ref coll_rhs_list '\n'
	{
	    char_sym2 = sym_pop();
	    char_sym1 = sym_pop();
	    sym_push(char_sym1);
	    sym_push(char_sym2);

	    (void)pop_wgt();	/* already have value in "weights" */
	    weights_2 = pop_wgt();
	    weights_1 = pop_wgt();

	    sem_set_collwgt_range(&weights_2);

	    /* update endpoints */
            sym_push(char_sym1);
	    sem_set_collwgt(&weights_1);
            sym_push(char_sym2);
	    sem_set_collwgt(&weights);

            no_strings = FALSE; 
	}
        ;

coll_ell_spec :
       	coll_symbol_ref coll_rhs_list '\n'
    	{
	    (void)pop_wgt();	/* already have value in "weights" */
	    sem_set_collwgt(&weights);
            no_strings = FALSE; 
	}
	;

coll_rhs_list :
     	coll_rhs_list ';' coll_ell_list_quote
	{
	    sem_collel_list(&weights, coll_tgt, ++cur_order);
	    push_wgt_no_inc(weights);
	}
	| coll_ell_list_quote
	{
	    sem_collel_list(&weights, coll_tgt, cur_order=0);
	    push_wgt_inc(weights);
	}
	;

coll_ell_list_quote :
        '"' coll_ell_list '"'
        | coll_ell_list
        ;

coll_ell_list :
	coll_ell_list coll_symbol_ref_new
	{
	    item_t *i;

	    i = sem_pop();
	    if (i==NULL || i->type != SK_INT)
	  	INTERNAL_ERROR;
	    i->value.int_no++;
	    sem_push_collel();
	    sem_push(i);
	}
	| coll_symbol_ref_new
	{
	    item_t *i;

	    i = create_item(SK_INT, 1);
	    sem_push_collel();
	    sem_push(i);
	}
	;

coll_symbol_ref : 
        char_symbol_ref_SKIP
        {   
	    sem_coll_sym_ref();
            no_strings=TRUE;   
       	    coll_tgt = sem_get_coll_tgt();
        }
	| byte_list
	{
	    sem_coll_literal_ref();
	    sem_coll_sym_ref();
            no_strings=TRUE; 
       	    coll_tgt = sem_get_coll_tgt();
	}
        ;

coll_symbol_ref_new :   /* same as coll_symbol_ref PLUS ellipsis */
        char_symbol_ref_SKIP
        {   
	    sem_coll_sym_ref();
            no_strings=TRUE;   
        }
	| byte_list
	{
	    sem_coll_literal_ref();
	    sem_coll_sym_ref();
            no_strings=TRUE; 
	}
	| KW_ELLIPSIS
          {
            symbol_t *s;
	    s = sem_make_ellipsis();
            sym_push(s);
        }
        ;

/* -----------------------------------------------------------------------
** LC_CTYPE
**
** This section parses the LC_CTYPE category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_ctype :
	KW_LC_CTYPE '\n' 
	{ 
	    /* The LC_CTYPE category can only be defined once in a file */

	    if (lc_ctype_flag)
	        diag_error(ERR_DUP_CATEGORY,"LC_CTYPE");
        } 
        lc_ctype_spec_list KW_END KW_LC_CTYPE '\n'
	{
	    /* A category with no text is an error (POSIX) */

	    if (!lc_ctype_flag) {
	        diag_error(ERR_EMPTY_CAT,"LC_CTYPE");
                /* be sure we initialize the character types: XPG4 sec 5.3.1 */
                sem_symbol("upper");   /* dummy value */
	        add_ctype(&ctype);
                }
	    else {
		check_upper();
		check_lower();
		check_alpha();
		check_space();
		check_cntl();
		check_punct();
		check_graph();
		check_print();
		check_digits();
		check_xdigit();
	    }
	}
	| KW_LC_CTYPE '\n' KW_COPY locale_name '\n' KW_END KW_LC_CTYPE '\n'
	{
	    copy_locale(LC_CTYPE);
	}
	| KW_LC_CTYPE '\n' KW_END KW_LC_CTYPE '\n'
	{
	    lc_ctype_flag++;

	    /* A category with no text is an error (POSIX) */

	    diag_error(ERR_EMPTY_CAT,"LC_CTYPE");

            /* be sure we initialize the character types: XPG4 sec 5.3.1 */
            sem_symbol("upper");  /* dummy value */
	    add_ctype(&ctype);

	}
	;

lc_ctype_spec_list :
	lc_ctype_spec_list lc_ctype_spec
	| lc_ctype_spec
	;

lc_ctype_spec : '\n'
	| symbol '\n'
	{
	    lc_ctype_flag++;
	    add_ctype(&ctype);
	}
	| symbol  {no_punct = TRUE; no_strings = TRUE;} char_range_list '\n'
	{
	    no_punct = FALSE;
            no_strings = FALSE;
	    lc_ctype_flag++;
	    add_ctype(&ctype);
	}
	| STRING '\n'
	{
	    sem_symbol(yytext);
	    lc_ctype_flag++;
	    add_ctype(&ctype);
	}
	| STRING {sem_symbol(yytext); no_punct = TRUE; no_strings = TRUE;}  char_range_list '\n'
	{	/* need to handle '"ctype" character range', */
                /* as well as 'ctype character range'.       */
	    no_punct = FALSE;
            no_strings = FALSE;
	    lc_ctype_flag++;
	    add_ctype(&ctype);
	}
	| KW_CHARCLASS ctype_proplist '\n'
	| KW_TOUPPER char_pair_list '\n'
	{
	    lc_ctype_flag++;
	    add_upper(&ctype);
	}
	| KW_TOLOWER {no_strings = TRUE;} char_pair_list '\n'
	{
	    no_strings = FALSE;
	    lc_ctype_flag++;
	    add_lower(&ctype);
	}  
	;

ctype_proplist : ctype_proplist ';' ctype_prop
	| ctype_prop
	;

ctype_prop : STRING
	{
	    add_predef_classname(yytext, next_bit);
	    next_bit <<= 1;
	}
	| SYMBOL
	{
	    /* need to handle unquoted ctypes as well as quoted ctypes */
	    add_predef_classname(sym_text, next_bit);
	    next_bit <<= 1;
	}
	;

char_pair_list : char_pair_list ';' char_pair
	| char_pair
	;

char_pair : '(' char_ref_SKIP ',' char_ref_SKIP ')'
	{
	    sem_push_xlat();
	}
	;

/* ----------------------------------------------------------------------
** LC_MONETARY
**
** This section parses the LC_MONETARY category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_monetary :
	KW_LC_MONETARY '\n' 
	{
	    /*  The LC_MONETARY category can only be defined once in a */
	    /*  locale						       */

	    if (lc_monetary_flag)
		diag_error(ERR_DUP_CATEGORY,"LC_MONETARY");
	 
	}
        lc_monetary_spec_list KW_END KW_LC_MONETARY '\n'
	{
	    /* A category must have at least one line of text (POSIX) */

	    if (!lc_monetary_flag)
	        diag_error(ERR_EMPTY_CAT,"LC_MONETARY");
	}
	| KW_LC_MONETARY '\n' KW_COPY locale_name '\n' KW_END KW_LC_MONETARY '\n'
	{
	    copy_locale(LC_MONETARY);
	}
	| KW_LC_MONETARY '\n' KW_END KW_LC_MONETARY '\n'
	{
	    lc_monetary_flag++;

	    /* A category must have at least one line of text (POSIX  */

	    diag_error(ERR_EMPTY_CAT,"LC_MONETARY");

	}
	;

lc_monetary_spec_list :
	lc_monetary_spec_list lc_monetary_spec
        | lc_monetary_spec_list '\n'
	{
	    lc_monetary_flag++;
	}
	| lc_monetary_spec
	{
	    lc_monetary_flag++;
	}
        | '\n'
	;

lc_monetary_spec :
  	KW_INT_CURR_SYMBOL string '\n'
	{
	    sem_set_str(&monetary.int_curr_symbol);
	}
	| KW_CURRENCY_SYMBOL string '\n'
	{
	    sem_set_str(&monetary.currency_symbol);
        }
	| KW_MON_DECIMAL_POINT string '\n'
	{ 
	    sem_set_str(&monetary.mon_decimal_point); 
	}
	| KW_MON_DECIMAL_POINT char_ref '\n'
	{ 
	    sem_set_chr(&monetary.mon_decimal_point); 
	}
	| KW_MON_THOUSANDS_SEP string '\n'  
	{
	    sem_set_str(&monetary.mon_thousands_sep);
	}
	| KW_CREDIT_SIGN string '\n'
	{
	    sem_set_str(&monetary.credit_sign);
	}
	| KW_DEBIT_SIGN string '\n' 
        {
	    sem_set_str(&monetary.debit_sign);
	}
	| KW_POSITIVE_SIGN string '\n'
	{
	    sem_set_str(&monetary.positive_sign);
	}
	| KW_NEGATIVE_SIGN string '\n'
	{
	    sem_set_str(&monetary.negative_sign);
	}
	| KW_LEFT_PARENTHESIS string '\n'
	{
	    sem_set_str(&monetary.left_parenthesis);
	}
	| KW_RIGHT_PARENTHESIS string '\n'
	{
	    sem_set_str(&monetary.right_parenthesis);
	}
	| KW_MON_GROUPING digit_list '\n'
	{
	    sem_set_diglist(&monetary.mon_grouping);
	}
	| KW_INT_FRAC_DIGITS const '\n'
	{
	    sem_set_int(&monetary.int_frac_digits);
	}
	| KW_FRAC_DIGITS const '\n'
	{
	    sem_set_int(&monetary.frac_digits);
	}
	| KW_P_CS_PRECEDES const '\n'
	{
	    sem_set_int(&monetary.p_cs_precedes);
	}
	| KW_P_SEP_BY_SPACE const '\n'
	{
	    sem_set_int(&monetary.p_sep_by_space);
	}
	| KW_N_CS_PRECEDES const '\n'
	{
	    sem_set_int(&monetary.n_cs_precedes);
	}
	| KW_N_SEP_BY_SPACE const '\n'
	{
	    sem_set_int(&monetary.n_sep_by_space);
	}
	| KW_P_SIGN_POSN const '\n'
	{
	    sem_set_int(&monetary.p_sign_posn);
	}
	| KW_N_SIGN_POSN const '\n'
	{
	    sem_set_int(&monetary.n_sign_posn);
	}
	;

/* ----------------------------------------------------------------------
** LC_MSG
**
** This section parses the LC_MSG category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_msg :
	KW_LC_MSG '\n' 
	{
	    if (lc_message_flag)
	        diag_error(ERR_DUP_CATEGORY,"LC_MESSAGE");

	}
	lc_msg_spec_list KW_END KW_LC_MSG '\n'
	{
	    if (!lc_message_flag)
		diag_error(ERR_EMPTY_CAT,"LC_MESSAGE");
	}
	| KW_LC_MSG '\n' KW_COPY locale_name '\n' KW_END KW_LC_MSG '\n'
	{
	    copy_locale(LC_MESSAGES);
	}
	| KW_LC_MSG '\n' KW_END KW_LC_MSG '\n'
	{
	    lc_message_flag++;

	    diag_error(ERR_EMPTY_CAT,"LC_MESSAGE");

	}
	;

lc_msg_spec_list :
	lc_msg_spec_list lc_msg_spec
	| lc_msg_spec_list '\n'
	{
	    lc_message_flag++;
	}
	| lc_msg_spec
	{
	    lc_message_flag++;
	}
        | '\n'
	;

lc_msg_spec :
	KW_YESEXPR string '\n'
	{
	    sem_set_str(&resp.yesexpr);
	}
	| KW_NOEXPR string '\n'
        {
	    sem_set_str(&resp.noexpr);
	}
        | KW_YESSTR string '\n'
        {
	    sem_set_str(&resp.yesstr);
	}
	| KW_NOSTR string '\n'
	{
	    sem_set_str(&resp.nostr);
	}
	;

/* ----------------------------------------------------------------------
** LC_NUMERIC
**
** This section parses the LC_NUMERIC category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_numeric :
	KW_LC_NUMERIC '\n' 
	{
	    if (lc_numeric_flag)
		diag_error(ERR_DUP_CATEGORY,"LC_NUMERIC");

	}
	lc_numeric_spec_list KW_END KW_LC_NUMERIC '\n'
	{
	    if (!lc_numeric_flag)
		diag_error(ERR_EMPTY_CAT,"LC_NUMERIC");
	}
	| KW_LC_NUMERIC '\n' KW_COPY locale_name '\n' KW_END KW_LC_NUMERIC '\n'
	{
	    copy_locale(LC_NUMERIC);
	}
	| KW_LC_NUMERIC '\n' KW_END KW_LC_NUMERIC '\n'
	{
	    lc_numeric_flag++;

	    diag_error(ERR_EMPTY_CAT,"LC_NUMERIC");

	}
	;

lc_numeric_spec_list :
	lc_numeric_spec_list lc_numeric_spec
	| lc_numeric_spec
	{
	    lc_numeric_flag++;
	}
        | lc_numeric_spec_list '\n'
	{
	    lc_numeric_flag++;
	}
        | '\n'
	;

lc_numeric_spec :
	KW_DECIMAL_POINT string '\n'
	{
	    sem_set_str(&numeric.decimal_point);
            if (numeric.decimal_point == NULL) {
              numeric.decimal_point = "";
              diag_error(ERR_ILL_DEC_CONST, "");  /* decimal pt is mandatory */
            }
	}
	| KW_DECIMAL_POINT char_ref '\n'
	{
	    sem_set_chr(&numeric.decimal_point);
            if (numeric.decimal_point == NULL) {
              numeric.decimal_point = "";
              diag_error(ERR_ILL_DEC_CONST, "");  /* decimal pt is mandatory */
            }
	}
	| KW_THOUSANDS_SEP string '\n'
        {
	    sem_set_str(&numeric.thousands_sep);
	}
	| KW_GROUPING digit_list '\n'
        {
	    sem_set_diglist(&numeric.grouping);
	}
	;

/* ----------------------------------------------------------------------
** LC_TIME
**
** This section parses the LC_TIME category section of the LOCSRC
** file.
** ----------------------------------------------------------------------*/

lc_time :
	KW_LC_TIME '\n' 
	{
	    if (lc_time_flag)
	 	diag_error(ERR_DUP_CATEGORY,"LC_TIME");

	}
	lc_time_spec_list KW_END KW_LC_TIME '\n'
	{
	    if (!lc_time_flag)
		diag_error(ERR_EMPTY_CAT,"LC_TIME");
	}
	| KW_LC_TIME '\n' KW_COPY locale_name '\n' KW_END KW_LC_TIME '\n'
	{
	    copy_locale(LC_TIME);
	}
	| KW_LC_TIME '\n' KW_END KW_LC_TIME '\n'
	{
	    lc_time_flag++;

	    diag_error(ERR_EMPTY_CAT,"LC_TIME");

	}
	;

lc_time_spec_list :
	lc_time_spec_list lc_time_spec
	| lc_time_spec
	{
	    lc_time_flag++;
	}
        | lc_time_spec_list '\n'
	{
	    lc_time_flag++;
	}
        | '\n'
	;

lc_time_spec :
	KW_ABDAY string_list '\n'
        {
	    sem_set_str_lst(lc_time.abday,7);
	}
	| KW_DAY string_list '\n'
	{
	    sem_set_str_lst(lc_time.day,7);
	}
	| KW_ABMON string_list '\n'
	{
	    sem_set_str_lst(lc_time.abmon,12);
	}
	| KW_MON string_list '\n'
	{
	    sem_set_str_lst(lc_time.mon,12);
	}
	| KW_D_T_FMT string '\n'
	{
	    sem_set_str(&lc_time.d_t_fmt);
	}
	| KW_D_FMT string '\n'
	{
	    sem_set_str(&lc_time.d_fmt);
	}
	| KW_T_FMT string '\n'
	{
	    sem_set_str(&lc_time.t_fmt);
	}
	| KW_AM_PM string_list '\n'
	{
	    sem_set_str_lst(lc_time.am_pm,2);
	}
	| KW_T_FMT_AMPM string '\n'
	{
	    sem_set_str(&lc_time.t_fmt_ampm);
	}
	| KW_ERA {arblen=0;} arblist '\n'
        {
            char **arbp = MALLOC(char*, arblen+1);
	    int total_len = 0;
            int i;

            sem_set_str_lst(arbp,arblen);

            for(i=0;i<arblen;i++)
		total_len += strlen(arbp[i]);

            lc_time.era = (char *) MALLOC(char *, total_len + 1);
	    lc_time.era[0] = '\0';

            for(i=0;i<arblen;i++) {
		strcat(lc_time.era,arbp[i]);
		if (i != arblen-1)
		    strcat(lc_time.era,";");
	    }
        }
	| KW_ERA_YEAR string '\n'
	{
	    sem_set_str(&lc_time.era_year);
	}
	| KW_ERA_D_FMT string '\n'
	{
	    sem_set_str(&lc_time.era_d_fmt);
	}
        | KW_ERA_D_T_FMT string '\n'
        {
            sem_set_str(&lc_time.era_d_t_fmt);
        }
        | KW_ERA_T_FMT string '\n'
        {
            sem_set_str(&lc_time.era_t_fmt);
        }
	| KW_ALT_DIGITS  {arblen=0;} arblist '\n'
      	{
            char **arbp = MALLOC(char*, arblen+1);
            int total_len = 0;
            int i;

            sem_set_str_lst(arbp,arblen);

            for(i=0;i<arblen;i++)       
                total_len += strlen(arbp[i]);

            lc_time.alt_digits = (char *)MALLOC(char *, total_len + 1);
            lc_time.alt_digits[0] = '\0';

            for(i=0;i<arblen;i++) {
                strcat(lc_time.alt_digits,arbp[i]);
                if (i != arblen-1)
                    strcat(lc_time.alt_digits,";");
            }



	}
    	| KW_NLLDATE string '\n'
	{
	    sem_set_str(&lc_aix31.nlldate);
	}
    	| KW_NLTMISC string '\n'
	{
	    sem_set_str(&lc_aix31.nltmisc);
	}
    	| KW_NLTSTR string '\n'
	{
	    sem_set_str(&lc_aix31.nltstr);
	}
    	| KW_NLTUNITS string '\n'
	{
	    sem_set_str(&lc_aix31.nltunits);
	}
        | KW_NLYEAR string '\n'
	{
	    sem_set_str(&lc_aix31.nlyear);
	}
	;

arblist :
        arblist ';' string
        {
            arblen++;
        }
        | string
        {
            arblen++;
        }
        ;

unrecognized_cat :
	SYMBOL '\n'
	{
	    int token;

	    user_defined++;
	    while ((token = yylex()) != KW_END);	    
	}
	SYMBOL '\n'
	{
	    diag_error(ERR_UNDEF_CAT,yytext);
	}
	;

/* ----------------------------------------------------------------------
** METHODS
**
** This section defines the grammar which parses the methods file.
** ----------------------------------------------------------------------*/

method_def : 
  	KW_METHODS '\n' 
	{
	    method_class = USR_DEFINED;
	    init_loc_std_methods();
	}
	method_assign_list KW_END KW_METHODS '\n'
	{
	    /* check replacements */
	    /* check_methods */
	    build_method_file();
	    check_methods();
	    /* load methods */
	    cc_ld_method_file();
	    load_method();
	}
	;

method_assign_list : 
	method_assign_list method_assign
	| method_assign_list '\n'
	| method_assign
        | '\n'
	;

method_assign :
	KW_CSID meth_name meth_lib_path        {SET_METHOD(CHARMAP_CSID);}
	| KW_FNMATCH meth_name meth_lib_path   {SET_METHOD(COLLATE_FNMATCH);}
	| KW_GET_WCTYPE meth_name meth_lib_path {SET_METHOD(CTYPE_GET_WCTYPE);}
	| KW_IS_WCTYPE meth_name meth_lib_path {SET_METHOD(CTYPE_IS_WCTYPE);}
	| KW_MBLEN meth_name meth_lib_path     {SET_METHOD(CHARMAP_MBLEN);}
	| KW_MBSTOPCS meth_name meth_lib_path  {SET_METHOD(CHARMAP___MBSTOPCS);}
	| KW_MBSTOWCS meth_name meth_lib_path  {SET_METHOD(CHARMAP_MBSTOWCS);}
	| KW_MBTOPC meth_name meth_lib_path    {SET_METHOD(CHARMAP___MBTOPC);}
	| KW_MBTOWC meth_name meth_lib_path    {SET_METHOD(CHARMAP_MBTOWC);}
	| KW_PCSTOMBS meth_name meth_lib_path  {SET_METHOD(CHARMAP___PCSTOMBS);}
	| KW_PCTOMB meth_name meth_lib_path    {SET_METHOD(CHARMAP___PCTOMB);}
	| KW_REGCOMP meth_name meth_lib_path   {SET_METHOD(COLLATE_REGCOMP);}
	| KW_REGERROR meth_name meth_lib_path  {SET_METHOD(COLLATE_REGERROR);}
	| KW_REGEXEC meth_name meth_lib_path   {SET_METHOD(COLLATE_REGEXEC);}
	| KW_REGFREE meth_name meth_lib_path   {SET_METHOD(COLLATE_REGFREE);}
	| KW_RPMATCH meth_name meth_lib_path   {SET_METHOD(RESP_RPMATCH);}
	| KW_STRCOLL meth_name meth_lib_path   {SET_METHOD(COLLATE_STRCOLL);}
	| KW_STRFMON meth_name meth_lib_path   {SET_METHOD(MONETARY_STRFMON);}
	| KW_STRFTIME meth_name meth_lib_path  {SET_METHOD(TIME_STRFTIME);}
	| KW_STRPTIME meth_name meth_lib_path  {SET_METHOD(TIME_STRPTIME);}
	| KW_STRXFRM meth_name meth_lib_path   {SET_METHOD(COLLATE_STRXFRM);}
	| KW_TOWLOWER meth_name meth_lib_path  {SET_METHOD(CTYPE_TOWLOWER);}
	| KW_TOWUPPER meth_name meth_lib_path  {SET_METHOD(CTYPE_TOWUPPER);}
	| KW_WCSCOLL meth_name meth_lib_path   {SET_METHOD(COLLATE_WCSCOLL);}
	| KW_WCSFTIME meth_name meth_lib_path  {SET_METHOD(TIME_WCSFTIME);}
	| KW_WCSID meth_name meth_lib_path     {SET_METHOD(CHARMAP_WCSID);}
	| KW_WCSTOMBS meth_name meth_lib_path  {SET_METHOD(CHARMAP_WCSTOMBS);}
	| KW_WCSWIDTH meth_name meth_lib_path  {SET_METHOD(CHARMAP_WCSWIDTH);}
	| KW_WCSXFRM meth_name meth_lib_path   {SET_METHOD(COLLATE_WCSXFRM);}
	| KW_WCTOMB meth_name meth_lib_path    {SET_METHOD(CHARMAP_WCTOMB);}
	| KW_WCWIDTH meth_name meth_lib_path   {SET_METHOD(CHARMAP_WCWIDTH);}
        ;   

meth_name: global_name
	{
	   if (private_table)
	      error(ERR_PRIVATE_TABLE);
	   else
	      global_table = TRUE;
	}
	| cfunc_name 
	{
	   if (global_table)
	       error(ERR_PRIVATE_TABLE);
	   else
	       private_table = TRUE;
	}
	;

global_name: KW_CSID_STD         {set_index(CSID_STD,CSID_STD_NAME);}
	| KW_FNMATCH_C        {set_index(FNMATCH_C,FNMATCH_C_NAME);}
	| KW_FNMATCH_STD      {set_index(FNMATCH_STD,FNMATCH_STD_NAME);}
	| KW_GET_WCTYPE_STD   {set_index(GET_WCTYPE_STD,GET_WCTYPE_STD_NAME);}
	| KW_IS_WCTYPE_SB     {set_index(IS_WCTYPE_SB,IS_WCTYPE_SB_NAME);}
	| KW_IS_WCTYPE_STD    {set_index(IS_WCTYPE_STD,IS_WCTYPE_STD_NAME);}
	| KW_LOCALECONV_STD   {set_index(LOCALECONV_STD,LOCALECONV_STD_NAME);}
	| KW_MBLEN_932        {set_index(MBLEN_932,MBLEN_932_NAME);}
	| KW_MBLEN_EUCJP      {set_index(MBLEN_EUCJP,MBLEN_EUCJP_NAME);}
	| KW_MBLEN_SB         {set_index(MBLEN_SB,MBLEN_SB_NAME);}
	| KW_MBSTOPCS_932     {set_index(__MBSTOPCS_932,__MBSTOPCS_932_NAME);}
	| KW_MBSTOPCS_EUCJP {set_index(__MBSTOPCS_EUCJP,__MBSTOPCS_EUCJP_NAME);}
	| KW_MBSTOPCS_SB      {set_index(__MBSTOPCS_SB,__MBSTOPCS_SB_NAME);}
	| KW_MBSTOWCS_932     {set_index(MBSTOWCS_932,MBSTOWCS_932_NAME);}
	| KW_MBSTOWCS_EUCJP   {set_index(MBSTOWCS_EUCJP,MBSTOWCS_EUCJP_NAME);}
	| KW_MBSTOWCS_SB      {set_index(MBSTOWCS_SB,MBSTOWCS_SB_NAME);}
	| KW_MBTOPC_932       {set_index(__MBTOPC_932,__MBTOPC_932_NAME);}
	| KW_MBTOPC_EUCJP     {set_index(__MBTOPC_EUCJP,__MBTOPC_EUCJP_NAME);}
	| KW_MBTOPC_SB        {set_index(__MBTOPC_SB,__MBTOPC_SB_NAME);}
	| KW_MBTOWC_932       {set_index(MBTOWC_932,MBTOWC_932_NAME);}
	| KW_MBTOWC_EUCJP     {set_index(MBTOWC_EUCJP,MBTOWC_EUCJP_NAME);}
	| KW_MBTOWC_SB        {set_index(MBTOWC_SB,MBTOWC_SB_NAME);}
	| KW_NL_MONINFO       {set_index(NL_MONINFO,NL_MONINFO_NAME);}
	| KW_NL_NUMINFO       {set_index(NL_NUMINFO,NL_NUMINFO_NAME);}
	| KW_NL_RESPINFO      {set_index(NL_RESPINFO,NL_RESPINFO_NAME);}
	| KW_NL_TIMINFO       {set_index(NL_TIMINFO,NL_TIMINFO_NAME);}
	| KW_PCSTOMBS_932     {set_index(__PCSTOMBS_932,__PCSTOMBS_932_NAME);}
	| KW_PCSTOMBS_EUCJP {set_index(__PCSTOMBS_EUCJP,__PCSTOMBS_EUCJP_NAME);}
	| KW_PCSTOMBS_SB      {set_index(__PCSTOMBS_SB,__PCSTOMBS_SB_NAME);}
	| KW_PCTOMB_932       {set_index(__PCTOMB_932,__PCTOMB_932_NAME);}
	| KW_PCTOMB_EUCJP     {set_index(__PCTOMB_EUCJP,__PCTOMB_EUCJP_NAME);}
	| KW_PCTOMB_SB        {set_index(__PCTOMB_SB,__PCTOMB_SB_NAME);}
	| KW_REGCOMP_STD      {set_index(REGCOMP_STD,REGCOMP_STD_NAME);}
	| KW_REGERROR_STD     {set_index(REGERROR_STD,REGERROR_STD_NAME);}
	| KW_REGEXEC_STD      {set_index(REGEXEC_STD,REGEXEC_STD_NAME);}
	| KW_REGFREE_STD      {set_index(REGFREE_STD,REGFREE_STD_NAME);}
	| KW_RPMATCH_C        {set_index(RPMATCH_C,RPMATCH_C_NAME);}
	| KW_RPMATCH_STD      {set_index(RPMATCH_STD,RPMATCH_STD_NAME);}
	| KW_STRCOLL_C        {set_index(STRCOLL_C,STRCOLL_C_NAME);}
	| KW_STRCOLL_SB       {set_index(STRCOLL_SB,STRCOLL_SB_NAME);}
	| KW_STRCOLL_STD      {set_index(STRCOLL_STD,STRCOLL_STD_NAME);}
	| KW_STRFMON_STD      {set_index(STRFMON_STD,STRFMON_STD_NAME);}
	| KW_STRFTIME_STD     {set_index(STRFTIME_STD,STRFTIME_STD_NAME);}
	| KW_STRPTIME_STD     {set_index(STRPTIME_STD,STRPTIME_STD_NAME);}
	| KW_STRXFRM_C        {set_index(STRXFRM_C,STRXFRM_C_NAME);}
	| KW_STRXFRM_SB       {set_index(STRXFRM_SB,STRXFRM_SB_NAME);}
	| KW_STRXFRM_STD      {set_index(STRXFRM_STD,STRXFRM_STD_NAME);}
	| KW_TOWLOWER_STD     {set_index(TOWLOWER_STD,TOWLOWER_STD_NAME);}
	| KW_TOWUPPER_STD     {set_index(TOWUPPER_STD,TOWUPPER_STD_NAME);}
	| KW_WCSCOLL_C        {set_index(WCSCOLL_C,WCSCOLL_C_NAME);}
	| KW_WCSCOLL_STD      {set_index(WCSCOLL_STD,WCSCOLL_STD_NAME);}
	| KW_WCSFTIME_STD     {set_index(WCSFTIME_STD,WCSFTIME_STD_NAME);}
	| KW_WCSID_STD        {set_index(WCSID_STD,WCSID_STD_NAME);}
	| KW_WCSTOMBS_932     {set_index(WCSTOMBS_932,WCSTOMBS_932_NAME);}
	| KW_WCSTOMBS_EUCJP   {set_index(WCSTOMBS_EUCJP,WCSTOMBS_EUCJP_NAME);}
	| KW_WCSTOMBS_SB      {set_index(WCSTOMBS_SB,WCSTOMBS_SB_NAME);}
	| KW_WCSWIDTH_932     {set_index(WCSWIDTH_932,WCSWIDTH_932_NAME);}
	| KW_WCSWIDTH_EUCJP   {set_index(WCSWIDTH_EUCJP,WCSWIDTH_EUCJP_NAME);}
	| KW_WCSWIDTH_LATIN   {set_index(WCSWIDTH_LATIN,WCSWIDTH_LATIN_NAME);}
	| KW_WCSXFRM_C        {set_index(WCSXFRM_C,WCSXFRM_C_NAME);}
	| KW_WCSXFRM_STD      {set_index(WCSXFRM_STD,WCSXFRM_STD_NAME);}
	| KW_WCTOMB_932       {set_index(WCTOMB_932,WCTOMB_932_NAME);}
	| KW_WCTOMB_EUCJP     {set_index(WCTOMB_EUCJP,WCTOMB_EUCJP_NAME);}
	| KW_WCTOMB_SB        {set_index(WCTOMB_SB,WCTOMB_SB_NAME);}
	| KW_WCWIDTH_932      {set_index(WCWIDTH_932,WCWIDTH_932_NAME);}
	| KW_WCWIDTH_EUCJP    {set_index(WCWIDTH_EUCJP,WCWIDTH_EUCJP_NAME);}
	| KW_WCWIDTH_LATIN    {set_index(WCWIDTH_LATIN,WCWIDTH_LATIN_NAME);}
	;

cfunc_name: string 
	;

meth_lib_path : '\n'
	| string '\n'
	{
	    if (global_table) {
		error(ERR_PRIVATE_TABLE);
	    }
	    set_method_lib_path();
	}
	;

/* ----------------------------------------------------------------------
** CHARSETID
**
** This section defines the grammar which parses the character set id
** classification of characters.
** ----------------------------------------------------------------------*/

charsets_def : 
  	KW_CHARSETID '\n' charset_assign_list KW_END KW_CHARSETID '\n'
	;

charset_assign_list : 
	charset_assign_list charset_assign
	| charset_assign_list '\n'
	| charset_assign
        | '\n'
	;


charset_assign : 
	charset_range_assign 
	{
	    sem_charset_range_def(&charmap);
	}
	| charset_simple_assign
	{
	    sem_charset_def(&charmap);
	}
	;

charset_range_assign :
	char_symbol_ref KW_ELLIPSIS char_symbol_ref const '\n'
	;

charset_simple_assign :
	char_symbol_ref const '\n'
	;

/* ----------------------------------------------------------------------
** GENERAL
**
** This section parses the syntatic elements shared by one or more of
** the above.
** ----------------------------------------------------------------------*/

digit_list : digit_list ';' const
    	{
	    /* add the new const to the digit list */
	    sem_digit_list();
	}
	| const
	{
	    item_t *i;

	    /* create count and push on top of stack */
	    i = create_item(SK_INT, 1);
	    sem_push(i);
	}
        ;

char_range_list : char_range_list ';' ctype_symbol
        | char_range_list ';' KW_ELLIPSIS ';' char_ref_SKIP
        {
	    push_char_range();
	}  
        | ctype_symbol
	;

ctype_symbol : char_ref_SKIP
        {
	    push_char_sym();
	}
        ;

char_ref : char_symbol_ref
	{
	    sem_char_ref();
	}
	| const
	| byte_list
	;

char_ref_SKIP : char_symbol_ref_SKIP
	{
	    sem_char_ref();
	}
	| const
	| byte_list
	;

char_symbol_ref : SYMBOL
    	{
	    sem_existing_symbol(sym_text,FALSE,FATAL);
	}
	;

char_symbol_ref_SKIP : SYMBOL
    	{
	    sem_existing_symbol(sym_text,TRUE,SKIP);
	}
	;

symbol  : SYMBOL
        {
	    sem_symbol(sym_text);
	}
	;

const   : int_const
	;

string_list : string_list ';' string
	| string
	;

string	: STRING
	{
	    item_t *i;
	    
	    i = create_item(SK_STR, yytext);
	    sem_push(i);
	}
	;

ellipsis: KW_ELLIPSIS
        {
            no_strings = TRUE;
            coll_tgt = sem_make_ellipsis();
        }

int_const : INT_CONST
        {
	    item_t *i;
	    char *junk;
	    
	    i = create_item(SK_INT, strtol(yytext, &junk, 10));
	    sem_push(i);
	}
        ;
	
byte_list : CHAR_CONST
        {
	    extern int value;
	    item_t *it;
	    
	    it = create_item(SK_INT, value);
	    sem_push(it);
        } 
        ;

locale_name : LOC_NAME
	{
	    item_t *i;
	    i = create_item(SK_STR, yytext);
	    sem_push(i);
	}
	;
%%

_LC_weight_t pop_wgt()
{
    if (current_weight!=0)
	return(weight_array[--current_weight]);
    else
       INTERNAL_ERROR;
}

void push_wgt_no_inc(_LC_weight_t wgts)
{
    if (current_weight!=MAX_WEIGHT_STACK-1)
	weight_array[current_weight]=wgts;
    else
        INTERNAL_ERROR;

}

void push_wgt_inc(_LC_weight_t wgts)
{
    if (current_weight!=MAX_WEIGHT_STACK-1)
	weight_array[current_weight++]=wgts;
    else
        INTERNAL_ERROR;

}

void    initgram(void) {
}
