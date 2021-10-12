static char sccsid[] = "@(#)34	1.5  src/bos/usr/bin/localedef/init.c, cmdnls, bos411, 9428A410j 4/6/94 21:58:07";
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

#include <stdlib.h>
#include <ctype.h>	
#include "symtab.h"
#include "locdef.h"
#include "err.h"

/*
*  The following are the global containers which are filled with the 
*  locale data from the locale sourcefile.  These structures are populated
*  with the default values to be assumed in the event the sourcefile 
*  does not specify a given locale attribute.
*/

/*
			       CHARMAP
*/
_LC_charmap_t  charmap={
    0, 0, 0, 0,			/* header */
    NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,	/* core */
    "ISO8859-1",
    1, 1,			/* cm_mv_cur_max, cm_mb_cur_min */
    1,				/* cm_max_disp_width */
    0,				/* cm_cstab */
    0, 0, 0			/* loc_rec, __meth_ptr, __data_ptr */
};

/*
			       COLLATE
*/
_LC_collate_t  collate={
    0, 0, 0, 0,			/* header */
    NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL,		/* core */
    1, {1, 0, 0, 0, 0}, 	/* co_nord, co_sort */
    0, 255, 			/* co_wc_min, co_wc_max */
    0, 255, 			/* co_col_min, co_col_max */
    0, 0, 0,			/* co_coltbl, co_nsubs, co_subs */
    0,				/* co_special */
    0, 0, 0			/* loc_rec, __meth_ptr, __data_ptr */
};

_LC_collate_t   *collate_ptr = &collate;

/*
				CTYPE
*/
_LC_ctype_t    ctype;
_LC_ctype_t    *ctype_ptr = &ctype;

/*
			       MONETARY
*/
_LC_monetary_t monetary = {
    0, 0, 0, 0,			/* header */
    NULL, NULL, NULL, NULL,	/* core */
    "", "", "", "", "", "", "-",
    -1, -1, -1, -1, -1, -1, -1, -1,
    "", "", "", "",
    0, 0, 0			/* loc_rec, __meth_ptr, __data_ptr */
    };

_LC_monetary_t  *monetary_ptr = &monetary;

/*
			       NUMERIC
*/
_LC_numeric_t  numeric={ 
    0, 0, 0, 0,			/* header */
    NULL, NULL, NULL,		/* core */
    ".", "", "",
    0, 0, 0			/* loc_rec, __meth_ptr, __data_ptr */
    };

_LC_numeric_t   *numeric_ptr = &numeric;

/*
				 TIME
*/
_LC_time_t lc_time={
    0, 0, 0, 0,				      /* header */
    NULL, NULL, NULL, NULL, NULL, NULL,	      /* core */
    "%m/%d/%y",
    "%H:%M:%S",
    "%a %b %d %H:%M:%S %Z %Y",
    "",
    { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", },
    { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
      "Saturday", },
    { "Jan", "Feb", "Mar", "Apr",
      "May", "Jun", "Jul", "Aug", 
      "Sep", "Oct", "Nov", "Dec", },
    { "January", "February", "March", "April",
      "May", "June", "July", "August", 
      "September",  "October", "November", "December", },
    { "AM", "PM", },
    "", "", "", "", "", "",
    0, 0, 0			/* loc_rec, __meth_ptr, __data_ptr */
};

_LC_time_t      *lc_time_ptr = &lc_time;

/*
			       RESPONSE
*/
_LC_resp_t resp={ 
    0, 0, 0, 0,				      /* header */
    NULL, NULL, NULL, NULL,		      /* core */
    "[yY][[:alpha:]]*", "[nN][[:alpha:]]*", 
    "y:Y:YES", "n:N:NO",
    0, 0, 0				/* loc_rec, __meth_ptr, __data_ptr */
    };

_LC_resp_t      *resp_ptr = &resp;

/* Special IBM-only localedef data structure */
_LC_aix31_t lc_aix31;


/*
*  FUNCTION: add_predef_classname
*
*  DESCRIPTION:
*  Build a character class with name 'name' and mask 'mask' and add
*  the 'name' to the symbol table.
*/
void add_predef_classname(char *name, int mask)
{
    extern symtab_t    cm_symtab;
    extern _LC_ctype_t ctype;
    symbol_t *s;
	
    s = create_symbol(name, 0);
    s->sym_type = ST_CLS_NM;
    s->data.cls = MALLOC(cls_sym_t, 1);
    s->data.cls->mask = mask;
    add_symbol(&cm_symtab, s);
    add_classname(&ctype, s->sym_id, mask);
}

struct seed_sym {
    char *sym;
    int  enc;
} seed_tbl[] = {
"<NUL>",			0x00,
"<SOH>",			0x01,
"<STX>",			0x02,
"<ETX>",			0x03,
"<EOT>",			0x04,
"<ENQ>",			0x05,
"<ACK>",			0x06,
"<BEL>",			0x07,
"<alert>",			0x07,
"<backspace>",			0x08,
"<tab>",			0x09,
"<newline>",			0x0a,
"<vertical-tab>",		0x0b,
"<form-feed>",			0x0c,
"<carriage-return>",		0x0d,
"<SO>",				0x0e,
"<SI>",				0x0f,
"<DLE>",			0x10,
"<DC1>",			0x11,
"<DC2>",			0x12,
"<DC3>",			0x13,
"<DC4>",			0x14,
"<NAK>",			0x15,
"<SYN>",			0x16,
"<ETB>",			0x17,
"<CAN>",			0x18,
"<EM>",				0x19,
"<SUB>",			0x1a,
"<ESC>",			0x1b,
"<IS4>",			0x1c,
"<IS3>",			0x1d,
"<IS2>",			0x1e,
"<IS1>",			0x1f,
"<SP>",				0x20,
"<space>",			0x20,
"<exclamation-mark>",		0x21,
"<quotation-mark>",		0x22,
"<number-sign>",		0x23,
"<dollar-sign>",		0x24,
"<percent-sign>",		0x25,
"<ampersand>",			0x26,
"<apostrophe>",			0x27,
"<left-parenthesis>",		0x28,
"<right-parenthesis>",		0x29,
"<asterisk>",			0x2a,
"<plus-sign>",			0x2b,
"<comma>",			0x2c,
"<hyphen>",			0x2d,
"<hyphen-minus>",		0x2d,
"<period>",			0x2e,
"<full-stop>",			0x2e,
"<slash>",			0x2f,
"<solidus>",			0x2f,
"<zero>",			0x30,
"<one>",			0x31,
"<two>",			0x32,
"<three>",			0x33,
"<four>",			0x34,
"<five>",			0x35,
"<six>",			0x36,
"<seven>",			0x37,
"<eight>",			0x38,
"<nine>",			0x39,
"<colon>",			0x3a,
"<semicolon>",			0x3b,
"<less-than-sign>",		0x3c,
"<equals-sign>",		0x3d,
"<greater-than-sign>",		0x3e,
"<question-mark>",		0x3f,
"<commercial-at>",		0x40,
"<A>",				0x41,
"<B>",				0x42,
"<C>",				0x43,
"<D>",				0x44,
"<E>",				0x45,
"<F>",				0x46,
"<G>",				0x47,
"<H>",				0x48,
"<I>",				0x49,
"<J>",				0x4a,
"<K>",				0x4b,
"<L>",				0x4c,
"<M>",				0x4d,
"<N>",				0x4e,
"<O>",				0x4f,
"<P>",				0x50,
"<Q>",				0x51,
"<R>",				0x52,
"<S>",				0x53,
"<T>",				0x54,
"<U>",				0x55,
"<V>",				0x56,
"<W>",				0x57,
"<X>",				0x58,
"<Y>",				0x59,
"<Z>",				0x5a,
"<left-square-bracket>",	0x5b,
"<backslash>",			0x5c,
"<reverse-solidus>",		0x5c,
"<right-square-bracket>",	0x5d,
"<circumflex>",			0x5e,
"<circumflex-accent>",		0x5e,
"<underscore>",			0x5f,
"<low-line>",			0x5f,
"<grave-accent>",		0x60,
"<a>",				0x61,
"<b>",				0x62,
"<c>",				0x63,
"<d>",				0x64,
"<e>",				0x65,
"<f>",				0x66,
"<g>",				0x67,
"<h>",				0x68,
"<i>",				0x69,
"<j>",				0x6a,
"<k>",				0x6b,
"<l>",				0x6c,
"<m>",				0x6d,
"<n>",				0x6e,
"<o>",				0x6f,
"<p>",				0x70,
"<q>",				0x71,
"<r>",				0x72,
"<s>",				0x73,
"<t>",				0x74,
"<u>",				0x75,
"<v>",				0x76,
"<w>",				0x77,
"<x>",				0x78,
"<y>",				0x79,
"<z>",				0x7a,
"<left-brace>",			0x7b,
"<left-curly-bracket>",		0x7b,
"<vertical-line>",		0x7c,
"<right-brace>",		0x7d,
"<right-curly-bracket>",	0x7d,
"<tilde>",			0x7e,
"<DEL>",			0x7f,	/* if changes, update max_wchar_symb */
};


/*
*  FUNCTION: init_symbol_tbl
*
*  DESCRIPTION:
*  Builds an empty symbol table and optionally 'seed's the table with the
*  POSIX Portable Character Set names.
*/
void init_symbol_tbl(int seed)
{
    extern symtab_t    cm_symtab;
    extern wchar_t     max_wchar_enc;
    extern char *      max_wchar_symb;
    extern int         mb_cur_max;
    extern _LC_ctype_t ctype;
    symbol_t *s;
    int      i;
    
    /* clear symbol table */
    cm_symtab.n_symbols = 0;
    for (i=0; i< HASH_TBL_SIZE; i++) {
	cm_symtab.symbols[i].next = NULL;
    }
    
    /* add predefined symbols
     */
    s = create_symbol("<code_set_name>", 0);     /* <codeset> symbol */
    s->data.str = NULL;
    add_symbol(&cm_symtab, s);
    
    s = create_symbol("<mb_cur_max>", 0);	 /* <mb_cur_max> symbol */
    s->data.ival = 0x1;
    mb_cur_max = 1;
    add_symbol(&cm_symtab, s);
    
    s = create_symbol("<mb_cur_min>", 0);	 /* <mb_cur_min> symbol */
    s->data.ival = 0;
    add_symbol(&cm_symtab, s);
    
    /* add predefined class names to ctype table */
    add_predef_classname("alpha", _ISALPHA);
    add_predef_classname("alnum", _ISALNUM);
    add_predef_classname("blank", _ISBLANK);
    add_predef_classname("cntrl", _ISCNTRL);
    add_predef_classname("digit", _ISDIGIT);
    add_predef_classname("graph", _ISGRAPH);
    add_predef_classname("lower", _ISLOWER);
    add_predef_classname("print", _ISPRINT);
    add_predef_classname("punct", _ISPUNCT);
    add_predef_classname("space", _ISSPACE);
    add_predef_classname("upper", _ISUPPER);
    add_predef_classname("xdigit", _ISXDIGIT);

    if (seed) {
	/* Seed symbol table with POSIX Portable Character Set. */
	int i;

	max_wchar_enc = 127;

	max_wchar_symb = "<DEL>";    /* symbolic name of max symbol */

	for (i = 0; i<sizeof(seed_tbl)/sizeof(seed_tbl[0]); i++) {
	    s = create_symbol(seed_tbl[i].sym, 0);
	    s->sym_type = ST_CHR_SYM;
	    s->data.chr = MALLOC(chr_sym_t, 1);
	    s->data.chr->str_enc[0] = seed_tbl[i].enc;
	    s->data.chr->fc_enc = seed_tbl[i].enc;
	    s->data.chr->wc_enc = seed_tbl[i].enc;
	    s->data.chr->width = 1;
	    s->data.chr->len = 1;
	    add_symbol(&cm_symtab, s);
	}
    } else 
	max_wchar_enc = 0;
}


/*
*
*  Dummy symbols to allow this code to bind and run non-shared
*  on a 3.1 Kernel.
*
*/
int _fp_fpscrx()
{
    return 0;
}

int fp_cpusync()
{
    return 0;
}

void debug(void)
{
printf("in debug\n");
}
