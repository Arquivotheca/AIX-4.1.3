/* @(#)10	1.8  src/bos/usr/bin/localedef/symtab.h, cmdnls, bos411, 9428A410j 4/6/94 21:59:12 */
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
 */

#ifndef _H_SYMTAB
#define _H_SYMTAB
#include <sys/localedef.h>
#include <sys/limits.h>
#include <sys/types.h>

/* symbol table/stack limits */
#define ST_MAX_SYM_LEN  32
#define ST_MAX_DEPTH    65536

typedef struct {
    wchar_t      pc;
    char         *sym;
    char  	 *str;
} coll_ell_t;

typedef struct {
  char *mth_sym;		 /* method symbol name */
  char *lib_name;		 /* name of library */
  void *(*mth_ptr);		 /* pointer to method */
} mth_sym_t;

#ifndef _COLL_WEIGHTS_MAX
#define _COLL_WEIGHTS_MAX  4
#endif

typedef struct {
    wchar_t      wc_enc;	/* the actual encoding for the character */
    unsigned int fc_enc;	/* character as integral file code */
    char         str_enc[MB_LEN_MAX+1];  /* character as string */
    _LC_weight_t *wgt;		/* collation weights */
    _LC_subs_t   *subs_str;	/* substitution string */
    short        width;		/* the display width of the character */
    short        len;		/* the length of the character in bytes */
    wchar_t	 abs_wgt;       /* absolute weight per defect 41409.  This  */
                                /* is a relative position for each defined  */
                                /* character, not the character value */
} chr_sym_t;

typedef struct {
  uint mask;
} cls_sym_t;

typedef struct _SYMBOL_T symbol_t;

struct _SYMBOL_T {
  char   sym_id[ST_MAX_SYM_LEN+1]; /* the symbol id text */
  int    sym_type;		   /* message set, text, ... symbol */
  int    sym_scope;                /* message set in which this symbol */
				   /* is defined */
  char   is_bogus;                 /* ignore if symbol is undefined */

  union  {
    chr_sym_t   *chr;              /* the character encoding this */
				   /* symbol refers to.  */

    cls_sym_t   *cls;		   /* symbol refers to a character class. */

    mth_sym_t   *mth;		   /* symbol refers to a method */

    coll_ell_t  *collel;	   /* symbol refers to a collation element */

    _LC_weight_t *collsym;	   /* symbol refers to a collation symbol */

    char        *str;

    int         ival;

  } data;

  symbol_t *next;
};

/* Valid values for sym_type above.
*/
#define ST_UNKNOWN      0	   /* symbol type is unknown */
#define ST_MTH_SYM      1	   /* symbol refers to a method */
#define ST_CHR_SYM      2	   /* symbol refers to character enc. */
#define ST_COLL_SYM     3	   /* symbol is a collation symbol */
#define ST_COLL_ELL     4          /* symbol is a collation element */
#define ST_CLS_NM       5	   /* symbol refers to char class */
#define ST_STR          6	   /* symbol refers to text string  */
#define ST_INT          7          /* symbol is an integer constant */
#define ST_ELLIPSIS	8	   /* symbol refers to an ellipsis */

#define HASH_TBL_SIZE   89
typedef struct {

  int      n_symbols;		   /* number of symbols in table */

  symbol_t symbols[HASH_TBL_SIZE]; /* sllist of symbol_t entries sorted */
				   /* alphabetically. */

} symtab_t;

/*
  symbol table utility error values
*/
#define ST_OK          0
#define ST_DUP_SYMBOL  1
#define ST_OVERFLOW    2

int        cmp_symbol(symbol_t *, char *, int);
symbol_t * create_symbol(char *, int);
int        add_symbol(symtab_t *, symbol_t *);
symbol_t * loc_symbol(symtab_t *, char *, int);

int        sym_push(symbol_t *);
symbol_t * sym_pop(void);

#endif
