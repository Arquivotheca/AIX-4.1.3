static char sccsid[] = "@(#)05	1.9.1.4  src/bos/usr/bin/localedef/sem_ctype.c, cmdnls, bos411, 9428A410j 1/11/94 15:46:11";
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

#include <sys/types.h>
#include <sys/localedef.h>
#include <ctype.h>
#include "semstack.h"
#include "symtab.h"
#include "err.h"

/* 
** GLOBAL variables 
*/
wchar_t max_wchar_enc;	/* the maximum wchar encoding for this codeset */
char *  max_wchar_symb;	/* the symbolic label for the maximum wchar    */
                        /* encoding for this codeset                   */
int next_bit = 0x1000;	/* first available bit for ctype classes       */

#define TERMINATOR	NULL	/* terminating value for char lists    */

/*
*  FUNCTION: add_classname
*
*  DESCRIPTION:
*  Adds a classname and associated mask to the ctype.classnms table.  
*/
void add_classname(_LC_ctype_t *ctype, char *s, int mask) 
{
    int i,j;

    j = -1;
    /* check if classnms NULL and allocate space for 32 classes. */
    if (ctype->classnms == NULL) {
	ctype->classnms = MALLOC(_LC_classnm_t, 32);
	ctype->nclasses = 0;
    }
	
    /* search for class name in names array */
    for (i=0; 
	 i<ctype->nclasses && 
	 (j=strcmp(s, ctype->classnms[i].name))>0;
	 i++);
    
    /* insert new class name unless already present */
    if (j != 0) {
	for (j=ctype->nclasses; j > i; j--)
	    ctype->classnms[j] = ctype->classnms[j-1];
	
	ctype->nclasses++;
	
	ctype->classnms[i].name = MALLOC(char, strlen(s)+1);
	strcpy(ctype->classnms[i].name, s);
	ctype->classnms[i].mask = mask;
    }
}


/*
*  FUNCTION: get_ctype_mask
*
*  DESCRIPTION:
*  Given the symbolic name for a character as used in the charmap file,
*  this function returns the ctype mask for that character.
*
*  This function is used for verifying which categories characters are in
*  for posix/xpg compliance regardless of the user assigned character encoding.
*
*  Only valid symbolic names for portable character set characters as defined
*  in section 4.1, table 4-1 of XPG4, System Interface Definitions, should be
*  passed in, since a fatal error is caused if the symbolic name is not found.
*/
int get_ctype_mask(char * symb_name)
{
        extern symtab_t cm_symtab;
	extern _LC_ctype_t ctype;
        symbol_t *s;

        s = loc_symbol(&cm_symtab,symb_name,0);
        if (s == NULL) {
            error(ERR_NO_SYMBOLIC_NAME,symb_name);
            }

	return(ctype.mask[s->data.chr->wc_enc]);
	
}


/*
*  FUNCTION: set_ctype_mask
*
*  DESCRIPTION:
*  Given the symbolic name for a character as used in the charmap file, and
*  an integer mask value, this function sets ctype mask for that
*  to the specified integer mask.
*
*  This function is used for assigning categories characters in a character
*  encoding independent fashion for posix/xpg compliance.
*
*  Only valid symbolic names for portable character set characters as defined
*  in section 4.1, table 4-1 of XPG4, System Interface Definitions should be
*  passed in, since a fatal error is caused if the symbolic name is not found.
*/
void set_ctype_mask(char * symb_name, int new_mask)
{
        extern symtab_t cm_symtab;
	extern _LC_ctype_t ctype;
        symbol_t *s;

        s = loc_symbol(&cm_symtab,symb_name,0);
        if (s == NULL) {
            error(ERR_NO_SYMBOLIC_NAME,symb_name);
            }

	ctype.mask[s->data.chr->wc_enc] = new_mask;
}


/*
*  FUNCTION: addto_ctype_mask
*
*  DESCRIPTION:
*  Given the symbolic name for a character as used in the charmap file, and
*  an integer mask value, this function sets the specified bits on the ctype
*  mask using the binary OR function.
*
*  This function is used for assigning categories characters in a character
*  encoding independent fashion for posix/xpg compliance.
*
*  Only valid symbolic names for portable character set characters as defined
*  in section 4.1, table 4-1 of XPG4 System Interface Definitions should be
*  passed in, since a fatal error is caused if the symbolic name is not found.
*/
void addto_ctype_mask(char * symb_name, int or_value)
{
        extern symtab_t cm_symtab;
	extern _LC_ctype_t ctype;
        symbol_t *s;

        s = loc_symbol(&cm_symtab,symb_name,0);
        if (s == NULL) {
            error(ERR_NO_SYMBOLIC_NAME,symb_name);
            }

	ctype.mask[s->data.chr->wc_enc] |= or_value;
}


/*
*  FUNCTION: add_ctype
*
*  DESCRIPTION:
*  Creates a new character classification from the list of characters on
*  the semantic stack.  The productions using this sem-action implement
*  statements of the form:
*
*      print    <A>;...;<Z>;<a>;<b>;<c>;...;<z>
*
*  The function checks if the class is already defined and if so uses the
*  mask in the table, otherwise a new mask is allocated and used.  This 
*  allows seeding of the table with the POSIX classnames and preassignment
*  of masks to those classes.
*/
void add_ctype(_LC_ctype_t *ctype)
{
    extern symtab_t cm_symtab;
    extern wchar_t  max_wchar_enc;
    
    item_t   *it;
    symbol_t *sym0;
    symbol_t *sym1;
    int      i, j;
    int      n_ranges;
    uint     mask;
    wchar_t  wc;

    char *uppers[]={    /* upper case character symbols as          */
    "<A>",              /* defined in XPG4 System Interface         */
    "<B>",              /* sections 5.3.1 and 4.1, table 4-1        */
    "<C>",
    "<D>",
    "<E>",
    "<F>",
    "<G>",
    "<H>",
    "<I>",
    "<J>",
    "<K>",
    "<L>",
    "<M>",
    "<N>",
    "<O>",
    "<P>",
    "<Q>",
    "<R>",
    "<S>",
    "<T>",
    "<U>",
    "<V>",
    "<W>",
    "<X>",
    "<Y>",
    "<Z>",
    TERMINATOR};        /* terminating value which will never occur */

    char *lowers[]={    /* lower case character symbols as          */
    "<a>",              /* defined in XPG4 System Interface         */
    "<b>",              /* sections 5.3.1 and 4.1, table 4-1        */
    "<c>",
    "<d>",
    "<e>",
    "<f>",
    "<g>",
    "<h>",
    "<i>",
    "<j>",
    "<k>",
    "<l>",
    "<m>",
    "<n>",
    "<o>",
    "<p>",
    "<q>",
    "<r>",
    "<s>",
    "<t>",
    "<u>",
    "<v>",
    "<w>",
    "<x>",
    "<y>",
    "<z>",
    TERMINATOR};        /* terminating value which will never occur */

    char *digits[]={    /* digit character symbols as               */
    "<zero>",           /* defined in XPG4 System Interface         */
    "<one>",            /* sections 5.3.1 and 4.1, table 4-1,       */
    "<two>",
    "<three>",
    "<four>",
    "<five>",
    "<six>",
    "<seven>",
    "<eight>",
    "<nine>",
    TERMINATOR};        /* terminating value which will never occur */

    char *xdigits[]={   /* xdigit character symbols as              */
    "<zero>",           /* defined in XPG4 System Interface         */
    "<one>",            /* sections 5.3.1 and 4.1, table 4-1,       */
    "<two>",
    "<three>",
    "<four>",
    "<five>",
    "<six>",
    "<seven>",
    "<eight>",
    "<nine>",
    "<A>",
    "<B>",
    "<C>",
    "<D>",
    "<E>",
    "<F>",
    "<a>",
    "<b>",
    "<c>",
    "<d>",
    "<e>",
    "<f>",
    TERMINATOR};        /* terminating value which will never occur */



    /* check if mask array has been defined yet, and if not allocate memory */
    if (ctype->mask == NULL) {
	ctype->mask = calloc(sizeof(unsigned int), 
                                 (max_wchar_enc < 256) ? 256 : max_wchar_enc+1);
                                 /* gen.c assumes this to be at least 256 */
	if (ctype->mask == NULL) 
	    INTERNAL_ERROR;

        /* we need this to support XPG4 section 5.3.1:  LC_CTYPE  */

	/* 
	  make sure <space>, <form-feed>, <newline>, <carriage_return>, 
	   <tab>, and <vertical-tab> are spaces 
	*/
	set_ctype_mask("<space>",_ISSPACE);
	set_ctype_mask("<form-feed>",_ISSPACE);
	set_ctype_mask("<newline>",_ISSPACE);
	set_ctype_mask("<carriage-return>",_ISSPACE);
	set_ctype_mask("<tab>",_ISSPACE);
	set_ctype_mask("<vertical-tab>",_ISSPACE);

	/* make sure <space> is printable */
	addto_ctype_mask("<space>",_ISPRINT);


	/* make sure <a>..<z> are lower, alpha, alnum, print and graph */
	for (i=0; lowers[i] != TERMINATOR; i++)
	    set_ctype_mask(lowers[i],
                         _ISALPHA | _ISALNUM | _ISLOWER | _ISPRINT | _ISGRAPH);

	/* make sure <A>..<Z> are upper, alpha, alnum, print and graph */
	for (i=0; uppers[i] != TERMINATOR; i++)
	    set_ctype_mask(uppers[i],
		         _ISALPHA | _ISALNUM | _ISUPPER | _ISPRINT | _ISGRAPH);

	/* make sure <zero>..<nine> are digit, alnum, print and graph */
	for (i=0; digits[i] != TERMINATOR; i++)
	    set_ctype_mask(digits[i],
				    _ISDIGIT | _ISALNUM | _ISPRINT | _ISGRAPH);

	/* make sure <zero>..<nine>, <A>..<F>, <a>..<f> are xdigit */
	for (i=0; xdigits[i] != TERMINATOR; i++)
	    addto_ctype_mask(xdigits[i], _ISXDIGIT);

	/* make sure <tab> and <space> are blanks */
	addto_ctype_mask("<tab>",_ISBLANK);
	addto_ctype_mask("<space>",_ISBLANK);
    }

    /* get class name off symbol stack.*/
    sym0 = sym_pop();
    
    /* check if class name already defined */
    sym1 = loc_symbol(&cm_symtab, sym0->sym_id, 0);
    if (sym1 != NULL && sym1->sym_type == ST_CLS_NM) {
	sym0 = sym1;
    } else {
	/* add new mask and type to symbol */
	diag_error(CHARCLASS_NOT_DECLARED, sym0->sym_id);
	add_predef_classname(sym0->sym_id, next_bit);

	next_bit <<= 1;
	sym0 = loc_symbol(&cm_symtab, sym0->sym_id, 0);
    }
    mask = sym0->data.cls->mask;
    
    /* handle derived properties */
    switch (mask) {
      case _ISUPPER:
      case _ISLOWER:
	mask |= _ISPRINT | _ISALPHA | _ISALNUM | _ISGRAPH;
	break;

      case _ISDIGIT:
	mask |= _ISPRINT | _ISALNUM | _ISGRAPH | _ISXDIGIT;
	break;

      case _ISPUNCT:
      case _ISXDIGIT:
	mask |= _ISGRAPH | _ISPRINT;
	break;

      case _ISBLANK:
	mask |= _ISSPACE;
	break;

      case _ISALPHA:
	mask |= _ISPRINT | _ISGRAPH | _ISALNUM;
	break;

      case _ISGRAPH:
	mask |= _ISPRINT;
	break;
    };
	

    /* for each range on stack - add mask to class mask for character */
    while ((it = sem_pop()) != NULL) {

	if (it->is_bogus == FALSE)
	    for (j=it->value.range->min; j <= it->value.range->max; j++) {
	        int wc;

	        wc = wc_from_fc(j);

	        /* only set masks for characters which are actually defined */
	        if (wc >= 0 && wchar_defined(wc)) 
		    ctype->mask[wc] |= mask;

	    }
	
	destroy_item(it);
    }
}	      


/*
*  FUNCTION: push_char_sym
*
*  DESCRIPTION:
*  Create character range from character symbol.  Routine expects 
*  character symbol on semantic stack.  This symbol is used to create a
*  simple character range which is then pushed back on semantic stack.
*  This production 
*
*  Treatment of single characters as ranges allows a uniform handling of 
*  characters and character ranges in class definition statements.
*/
void push_char_sym(void)
{
    item_t   *it0, *it1;
    
    it0 = sem_pop();
    
    if (it0->type == SK_CHR)
	it1 = create_item(SK_RNG, 
			  it0->value.chr->fc_enc,
			  it0->value.chr->fc_enc);
    else if (it0->type == SK_INT)
	it1 = create_item(SK_RNG, 
			  it0->value.int_no,
			  it0->value.int_no);
    else
	error(ERR_ILL_RANGE_SPEC);

    if (it0->is_bogus == TRUE)
        it1->is_bogus = TRUE;

    destroy_item(it0);

    sem_push(it1);
}


/*
*  FUNCTION: push_char_range
*
*  DESCRIPTION:
*  Modifies end point of range with character on top of stack. This rule is
*  used by productions implementing expressions of the form:
*
*       <A>;...;<Z>
*/
void push_char_range(void)
{
    item_t   *it0, *it1;
    
    it1 = sem_pop();		/* from character at end of range   */
    it0 = sem_pop();		/* from character at start of range */
    if (it1->type == SK_CHR && it0->type == SK_RNG) {
        /* make sure min is less than max */
        if (it1->value.chr->fc_enc > it0->value.range->max)
	    it0->value.range->max = it1->value.chr->fc_enc;
        else
	    it0->value.range->min = it1->value.chr->fc_enc;
    }
    else if (it1->type == SK_INT && it0->type == SK_RNG) {
	if (it1->value.int_no > it0->value.range->max) 
	    it0->value.range->max = it1->value.int_no;
	else
	    it0->value.range->min = it1->value.int_no;
    }
    else
	INTERNAL_ERROR;
    
    if ((it0->is_bogus == TRUE) || (it1->is_bogus == TRUE))
	it0->is_bogus = TRUE;

    destroy_item(it1);
    
    sem_push(it0);
}
