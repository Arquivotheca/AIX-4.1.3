static char sccsid[] = "@(#)04	1.13.1.9  src/bos/usr/bin/localedef/sem_coll.c, cmdnls, bos411, 9428A410j 4/27/94 10:53:41";
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
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <ctype.h>
#include "symtab.h"
#include "semstack.h"
#include "fwdrefstack.h"
#include "method.h"
#include "locdef.h"
#include "err.h"

/* necessary prototype definition */
void sem_set_dflt_collwgt(void);
wchar_t get_coll_wgt(_LC_weight_t *weights, int ord);

void show_hex(char *s)
{
while (*s) printf("%02x",*s++);
printf(" ");
}

/*
*  FUNCTION: nxt_coll_wgt
*
*  DESCRIPTION:
*  Collation weights cannot have a zero in either the first or second
*  byte (assuming two byte wchar_t).
*/
wchar_t nxt_coll_wgt(void)
{
    static wchar_t nxt_coll_val = 0x0101;
    extern _LC_collate_t collate;
    wchar_t collval;
    
    collval = nxt_coll_val;
    nxt_coll_val++;
    if ((nxt_coll_val & 0xff00) == 0)
	INTERNAL_ERROR;
    
    if ((nxt_coll_val & 0x00ff) == 0)
	nxt_coll_val |= 0x0001;

    if (collval > collate.co_col_max)
	collate.co_col_max = collval;
    
    return collval;
}


/* MACRO: has_substitution
*
*  DESCRIPTION:
*  If the weight of the first order is SUB_STRING, then this character
*  does 1-to-many mapping for some order. 
*
*  This is a macro just to make things a bit more efficient...
*
*/

#define has_substitution(weights) ((weights)->n[0]==SUB_STRING)


/* FUNCTION: get_index 
*
*  DESCRIPTION:
*  Once we know that this character does 1-to-many mapping, we need to
*  get the index into the co_subs[] array.  This is always
*  stored as the weight for the second order, when the weight of the
*  first order is SUB_STRING.
*/

int get_index(_LC_weight_t *weights)
{

    if (!has_substitution(weights))
	INTERNAL_ERROR;

    return ((int)weights->n[1]);
}


/* FUNCTION: assign_index_and_flag
*
*  DESCRIPTION:
*  Once a character has been identified as having a 1-to-many
*  mapping it becomes necessary to initialize the the first
*  two weights to SUB_STRING, and the index into the co_subs[]
*  array.  Occasionally, this routine is used to change existing 
*  index values.  When this happens, set_coll_wgt() can not be
*  used since it will modify the value in the co_subs[] array
*  not the actual index.
*
*/

void assign_index_and_flag(_LC_weight_t *weights,int index)
{

    weights->n[0] = SUB_STRING;
    weights->n[1] = (unsigned short) index;
}

/* FUNCTION: reset_weights
*
*  DESCRIPTION:
*  Prevent set_coll_wgt() from writing information into
*  co_subs[] array, by initalizing weights.
*
*/

void reset_weights(_LC_weight_t *weights)
{
    weights->n[0] = 0;
    weights->n[1] = 0;
}



/*
*  FUNCTION: set_coll_wgt
*
*  DESCRIPTION:
*  Sets the collation weight for order 'ord' in weight structure 'weights'
*  to 'weight'.  If 'ord' is -1, all weights in 'weights' are set to 
*  'weight'.
*
*  The absolute weight is implicitly set here when ord < 0
*
*  When has_substitution(weights) is true, then update weight in
*  collate.co_subs[] array.
*
*/
void set_coll_wgt(_LC_weight_t *weights, wchar_t weight, int ord)
{
    extern _LC_collate_t collate;
    int i;

    if (has_substitution(weights)) {
	int index,start,stop;
	index=get_index(weights);
	if ((collate.co_subs == NULL) || (index>collate.co_nsubs))
		INTERNAL_ERROR;
	if (ord < 0) {
		start=0;stop=_COLL_WEIGHTS_MAX;
	} 
	else {
		start=ord;stop=ord;
	}
	for (i=start;i<=stop;i++) {
	    collate.co_subs[index].tgt_wgt_str[i]=MALLOC(char,sizeof(short)+1);
	    collate.co_subs[index].tgt_wgt_str[i][0]=((char *)&weight)[0];
	    collate.co_subs[index].tgt_wgt_str[i][1]=((char *)&weight)[1];
	    collate.co_subs[index].tgt_wgt_str[i][2]='\0';
	    if (i==_UCW_ORDER)      /* set ucw in coll table too */
	        weights->n[i]=weight;
	    }
	}
    else {
	if (ord < 0) {
	    for (i=0; i <= _COLL_WEIGHTS_MAX; i++)
		weights->n[i] = weight;
	} else
	    if (ord >= 0 && ord <= _COLL_WEIGHTS_MAX)
		weights->n[ord] = weight;
	    else
		INTERNAL_ERROR;
    }
}


/*
*  FUNCTION: get_coll_wgt
*
*  DESCRIPTION:
*  Gets the collation weight for order 'ord' in weight structure 'weights'.
*
*  When has_substitution(weights) is true, then get weight in
*  collate.co_subs[] array.
*
*/
wchar_t get_coll_wgt(_LC_weight_t *weights, int ord)
{
    extern _LC_collate_t collate;

	if (ord >= 0 && ord <= _COLL_WEIGHTS_MAX) {
	    if (has_substitution(weights)) {
		int index = get_index(weights);
		unsigned short ret_val;
		if ((collate.co_subs == NULL) || (index>collate.co_nsubs))
			INTERNAL_ERROR;
		ret_val =
		     *(unsigned short *)collate.co_subs[index].tgt_wgt_str[ord];
		return ret_val;
		}
	    else
	        return weights->n[ord];
	    }
	else
	    error(ERR_TOO_MANY_ORDERS);

}


/*
*  FUNCTION: set_abs_wgt
*
*  DESCRIPTION:
*  sets the absolute collation weight (the _UCW_ORDER'th (or 
*  _COLL_WEIGHTS_MAX+1'th) order) for weight structure 'weights' to weight
* 
*  This could be a macro if performance were a serious issue.
*/
void set_abs_wgt(_LC_weight_t * weights, wchar_t weight)
{
    extern _LC_collate_t collate;

    set_coll_wgt(weights,weight,_UCW_ORDER);
    set_coll_wgt(weights,weight,collate.co_nord);  /* temp compat */
    weights->n[_UCW_ORDER] = weight;
}


/*
*  FUNCTION: get_abs_wgt
*
*  DESCRIPTION:
*  gets the absolute collation weight (the collate.co_nord'th order) for
*  weight structure 'weights'
*
*  This could be a macro if performance were a serious issue.
*/
wchar_t get_abs_wgt(_LC_weight_t * weights)
{
    return (get_coll_wgt(weights,_UCW_ORDER));
}


/* 
*  FUNCTION: sem_init_colltbl
*
*  DESCRIPTION:
*  Initialize the collation table.  This amounts to setting all collation
*  values to IGNORE, assigning the default collation order (which is 1), 
*  allocating memory to contain the table.
*/
void sem_init_colltbl(void)
{
    extern symtab_t cm_symtab;
    extern wchar_t max_wchar_enc;
    extern _LC_collate_t collate;
    short    low;
    symbol_t *s;
    int      i;
    
    /* initialize collation attributes to defaults */
    collate.co_nord   = 1;		/* potentially modified by 'order' */
    collate.co_wc_min = 0;		/* always 0                        */
    collate.co_wc_max = max_wchar_enc;	/* always max_wchar_enc            */
    
    /* allocate and zero fill memory to contain collation table */
    collate.co_coltbl = calloc(max_wchar_enc+1, sizeof(_LC_coltbl_t));
    if (collate.co_coltbl == NULL)
	INTERNAL_ERROR;

    /* set default min and max collation weights */
    collate.co_col_min = collate.co_col_max = 0;

    /* initialize substitution strings */
    collate.co_nsubs = 0;
    collate.co_subs  = NULL;
}


/* 
*  FUNCTION: sem_push_collel();
*  DESCRIPTION:
*  Copies a symbol from the symbol stack to the semantic stack.
*/
void sem_push_collel()
{
    symbol_t *s;
    item_t   *i;

    s = sym_pop();
    i = create_item(SK_SYM, s);

    if (s->is_bogus == TRUE)
        i->is_bogus = TRUE;

    sem_push(i);
}


/*
*  FUNCTION: loc_collel
*
*  DESCRIPTION: 
*  Locates a collation element in an array of collation elements.  This
*  function returns the first collation element which matches 'sym'.
*/
_LC_collel_t *loc_collel(char *sym, wchar_t pc)
{
    extern _LC_collate_t collate;
    _LC_collel_t *ce;

    for (ce = collate.co_coltbl[pc].ct_collel; ce->ce_sym != NULL; ce++) {
	if (strcmp(sym, ce->ce_sym)==0)
	    return ce;
    }
    
    INTERNAL_ERROR;
}


/*
*  FUNCTION: sem_coll_sym_ref
*
*  DESCRIPTION:
*  checks that the symbol referenced has a collation weights structure
*  attached.  If one is not yet present, one is allocated as necessary 
*  for the symbol type.
*/
void sem_coll_sym_ref()
{
    extern _LC_collate_t collate;
    _LC_collel_t *ce;
    symbol_t     *s;

    /* Pop symbol specified off of symbol stack
    */
    s = sym_pop();

    /* 
      Check that this element has a weights array with
      the correct number of orders.  If the element does not, create
      one and initialize the contents to UNDEFINED.
      */
    switch (s->sym_type) {
      case ST_CHR_SYM:
	if (s->data.chr->wgt == NULL)
	    s->data.chr->wgt = 
		&(collate.co_coltbl[s->data.chr->wc_enc].ct_wgt);
	break;

      case ST_COLL_ELL:
	ce = loc_collel(s->data.collel->sym, s->data.collel->pc);
	break;

      case ST_COLL_SYM:
	/* no work, but leave a place holder */
	break;
    }

    sym_push(s);
}


/*
*  FUNCTION: sem_coll_literal_ref
*  
*  DESCRIPTION:
*  A character literal is specified as a collation element.  Take this
*  element and create a dummy symbol for it.  The dummy symbol is pushed
*  onto the symbol stack, but is not added to the symbol table.
*/
void sem_coll_literal_ref()
{
    extern int max_disp_width;
    symbol_t *dummy;
    item_t   *it;
    wchar_t  pc;
    int      rc;
    int      fc;
    void *   old__lc_ctype_obj;
    _LC_ctype_t bogus_ctype_obj;
    _LC_charmap_objhdl_t null_object ={0,0};
    unsigned int  bogus_ctype = _ISPRINT;
    unsigned char bogus_index = 0;
    extern symtab_t cm_symtab;
    symbol_t *sym;
    char s[20];  /* # should be greater than max possbile bytes per char + 3 */


    /* Pop the file code to use as character off the semantic stack. */
    it = sem_pop();
    fc = it->value.int_no;


    /* If specified symbol exists, we need to treat it like a valid symbol */
    /* this satisfies XPG4 page 56 and 44 re: valid expressions of weights */
   
    sprintf(s,"<%C>",fc);

    /* look for symbol in symbol table */
    sym = loc_symbol(&cm_symtab, s, 0);

    if (sym != NULL)
       {
         sym_push(sym);
         return;
       }


    /* Create a dummy symbol with this byte list as its encoding. */
    dummy = MALLOC(symbol_t, 1);
    dummy->sym_type = ST_CHR_SYM;
    dummy->sym_scope = 0;
    dummy->data.chr = MALLOC(chr_sym_t, 1);

    /* save file code for character */
    dummy->data.chr->fc_enc = fc;
    
    /* use hex translation of file code for symbol id (for errors) */
    sprintf(dummy->sym_id, "0x%x", fc);

    /* save length of character */
    dummy->data.chr->len = mbs_from_fc(dummy->data.chr->str_enc, fc);

    /* check if characters this long are valid */
    if (dummy->data.chr->len > mb_cur_max)
	error(ERR_CHAR_TOO_LONG, dummy->sym_id);

    /* define process code for character literal */
    rc = CALL_METH(METH_OFFS(CHARMAP_MBTOWC))(null_object, &pc,
					      dummy->data.chr->str_enc,
					      MB_LEN_MAX);
    if (rc < 0)
	error(ERR_UNSUP_ENC, dummy->sym_id);

    dummy->data.chr->wc_enc = pc;
    
    /* wcwidth() methods now call iswprint().  iswprint() requires       */
    /* a ctypes classification table, which does not exist yet.          */
    /* Unless we provide some kind of a table for isprint() to use       */
    /* iswprint() will either core dump or return the wrong value.       */
    /* We will bunt, and create an object for each character which will  */
    /* return true for iswprint().  If iswprint(), wcwidth(), or the     */
    /* locale data structures change much, this may need to be modified. */
	
    /* Commentary on this modification:  After saving the global pointer */
    /* __lc_ctype.obj, create a bogus_ctype_obj that is solely intended  */
    /* to cause iswprint() or isprint() to return true on character pc.  */
    /* The obvious solution is to create a large ctype table, but since  */
    /* this is wasteful of memory, simply calculate a pointer the        */
    /* specific character classification value, bogus_ctype.             */
    /* This code will ONLY support an isprint() operation for "pc".      */

    old__lc_ctype_obj = __OBJ_DATA(__lc_ctype);
    bogus_ctype_obj.core=__OBJ_DATA(__lc_ctype)->core;
    bogus_ctype_obj.min_wc=0;
    bogus_ctype_obj.max_wc=pc;
    if (pc<=255)
	bogus_ctype_obj.mask  = &bogus_ctype-pc;
    else {
        bogus_ctype_obj.qmask = &bogus_ctype;
	bogus_ctype_obj.qidx  = &bogus_index-(pc-256);
	}

    __OBJ_DATA(__lc_ctype) = &bogus_ctype_obj;

    /* define width for character */
    rc = CALL_METH(METH_OFFS(CHARMAP_WCWIDTH))(null_object, pc);

   __OBJ_DATA(__lc_ctype) = old__lc_ctype_obj;

    if (rc > max_disp_width)
	error(ERR_UNSUP_ENC, dummy->sym_id);
    else
	dummy->data.chr->width = rc;

    /* clear out wgt and subs_str pointers */
    dummy->data.chr->wgt = NULL;
    dummy->data.chr->subs_str = NULL;

    /* mark character as defined */
    define_wchar(pc);

    destroy_item(it);
    sym_push(dummy);
}

/*
*  FUNCTION:  remove_subst_entry
*
*  DESCRIPTION:
*
*  Remove specified entry from substitution table and update existing
*  index values in the collation table...
*
*/

void remove_subst_entry(int index)
{
    extern _LC_collate_t collate;
    extern wchar_t max_wchar_enc;
    _LC_subs_t *subs;
    int i,j;
    int current_index;

    subs = MALLOC(_LC_subs_t, collate.co_nsubs-1);

   /* copy old substitute table to new substitute table */
   for (i=0,current_index=0; i<collate.co_nsubs; i++)
	if (i!=index) {
            for (j=0; j<=_COLL_WEIGHTS_MAX; j++)
	        subs[current_index].tgt_wgt_str[j]=
					collate.co_subs[i].tgt_wgt_str[j];
	    current_index++;
	}

    for (i=0; i<=max_wchar_enc; i++) {

	if (wchar_defined(i))
	    if (has_substitution(&(collate.co_coltbl[i].ct_wgt))) {
		current_index = get_index(&(collate.co_coltbl[i].ct_wgt));
		if (current_index==index)
		    INTERNAL_ERROR; /* indices for removal can't be used! */
		if (current_index>index)
		    assign_index_and_flag(&(collate.co_coltbl[i].ct_wgt),
							     current_index-1);
		    }
	}

    collate.co_nsubs--;
    free(collate.co_subs);
    collate.co_subs = subs;

}


/* 
*  FUNCTION: sem_def_substr
*
*  DESCRIPTION:
*  Defines a substitution string for order specified by order.
*
*	order	The current collating order.  Must be called in order.
*		(ie First 0, then 1, then 2, etc.)
*
*	w	The weight structure for the current character.
*		This will either contains weights defined to date, or
*		an identifieable index to the substitution table for this
*		character.
*
*	wgt_str	A string of byte values that contain the weights to
*		be used for this character at this order.
*
*  Updates weight structure for the target character as needed:
*	1.  If a substitution string for this source character has
*	    already been specified (order 0 weight == SUB_STRING, and
*	    order 1 weight == index into subs table) update the
*	    entry specified by order and index.
*	2.  If a substitution string for this source character has
*	    not yet been specified, then make room for it (allocate
*	    sufficient space for new substitution table, copy old info over,
*	    then add it to the end of the table), if order>=1 then copy
*	    the weights from all orders <order into the new entry in the
*           substitution table for the appropriate orders.  
*
*/
void sem_def_substr(int order, _LC_weight_t *w, char *wgt_str)
{
    extern _LC_collate_t collate;
    char       *ss;
    _LC_subs_t *subs;
    int        i, j;
    int        index;
    unsigned   short weight;

    ss = MALLOC(char, strlen(wgt_str)+1);
    strcpy(ss, wgt_str);

    if (!has_substitution(w)) {
    	/* allocate space for new substitution string */
    	subs = MALLOC(_LC_subs_t, collate.co_nsubs+1);

    	/* copy old substitute table to new substitute table */
    	for (i=0; i<collate.co_nsubs; i++)
	    for (j=0; j<=_COLL_WEIGHTS_MAX; j++)
		subs[i].tgt_wgt_str[j]=collate.co_subs[i].tgt_wgt_str[j];

	index = collate.co_nsubs;

	/* copy existing weights for this character */
	for (i=0;i<=_COLL_WEIGHTS_MAX;i++) {
	    weight = (unsigned short)get_coll_wgt(w,i);
	    subs[index].tgt_wgt_str[i]    = MALLOC(char,sizeof(short)+1);
	    subs[index].tgt_wgt_str[i][0] = ((char *)&weight)[0];
	    subs[index].tgt_wgt_str[i][1] = ((char *)&weight)[1];
	    subs[index].tgt_wgt_str[i][2] = '\0';
	    }

        assign_index_and_flag(w,index);

        /* increment substitute string count */
        collate.co_nsubs++;

        /* free space occupied by old list */
        free(collate.co_subs);
    
        /* attach new list to coll table */
        collate.co_subs = subs;

	}
    else
	index = get_index(w);	/* this is the index */

    collate.co_subs[index].tgt_wgt_str[order] = ss;

}


/*
*  FUNCTION: sem_collel_list
*
*  DESCRIPTION:
*  Process the set of symbols now on the semantic stack for the character 
*  for this particular order.
*/
void sem_collel_list(_LC_weight_t *w, symbol_t *tgt, int order)
{
    extern _LC_collate_t collate;
    item_t       *i;		/* count item - # of items which follow */
    item_t       *si;		/* symbol item - item containing symbol */
    _LC_collel_t *ce;
    wchar_t      weight;
    wchar_t      next_weight = 0;  /* keep track weight for initialization */

    i = sem_pop();		/* pop count item */

    if (i == NULL || i->type != SK_INT)
	INTERNAL_ERROR;

    /* collating symbols can not have weights (XPG4) */
    if ((tgt->sym_type==ST_COLL_SYM) && (strcmp(tgt->sym_id,"UNDEFINED")!=0)) {
        int n;
        diag_error(ERR_BAD_WEIGHT, tgt->sym_id);

        /* pop off and ignore info from the stack */
	for (n=0; n < i->value.int_no; n++)
	    (void) sem_pop();

        return;
        }

    if (order >= _COLL_WEIGHTS_MAX) {
        int n;
	/* can not handle more orders than _COLL_WEIGHTS_MAX  */
	/* gram.y already output an error message             */

        /* pop off and ignore info from the stack */
	for (n=0; n < i->value.int_no; n++)
	    (void) sem_pop();

        return;
        }

    if (order >= collate.co_nord) {
        int n;
        diag_error(TOO_MANY_WEIGHTS,tgt->sym_id);

        /* pop off and ignore info from the stack */
	for (n=0; n < i->value.int_no; n++)
	    (void) sem_pop();

        return;
        }

    if (order==0)
	reset_weights(w);

    if ((tgt->sym_type==ST_ELLIPSIS) && (order==0))
	set_coll_wgt(w,ELLIPSIS_WEIGHT,-1);

    if (i->value.int_no==1) {
	/* character gets collation value of symbol */
	si = sem_pop();
	if (si == NULL || si->type != SK_SYM)
	    INTERNAL_ERROR;

	switch (si->value.sym->sym_type) {

	  case ST_CHR_SYM:		/* character */
	    /* only need to check for bogus chars if ST_CHR_SYM */
	    if (si->is_bogus == TRUE) {
		if (tgt->sym_type == ST_ELLIPSIS)
		    return;
		if (order==0) {
		    next_weight = nxt_coll_wgt();
		    set_coll_wgt(w, next_weight, -1);  
		    if (tgt->sym_type != ST_COLL_ELL)
		        set_coll_wgt(tgt->data.chr->wgt, next_weight, -1);
		    else {
		        ce=loc_collel(tgt->data.collel->sym,tgt->data.collel->pc);
		        set_coll_wgt(&(ce->ce_wgt), next_weight, -1);
		    }
		}
		else {
	           next_weight=get_abs_wgt(w);
		   set_coll_wgt(w, next_weight, order);
		   if (tgt->sym_type != ST_COLL_ELL)
		       set_coll_wgt(tgt->data.chr->wgt, next_weight, order);
		   else {
		       ce=loc_collel(tgt->data.collel->sym,tgt->data.collel->pc);
		       set_coll_wgt(&(ce->ce_wgt), next_weight, order);
		   }
		}
		return;
	    }
	    weight=get_abs_wgt(si->value.sym->data.chr->wgt);
	    if (weight == UNDEFINED) {
		/*
		  a symbol with UNDEFINED collation can only appear on the
		  RHS if it is also the target on the LHS
		*/
		if (si->value.sym == tgt) {
		    /* assign collation weight for self reference */
		    if (order==0)
		        set_coll_wgt(si->value.sym->data.chr->wgt, 
				 next_weight = nxt_coll_wgt(), -1);
		    else
		        set_coll_wgt(si->value.sym->data.chr->wgt, 
				 get_abs_wgt(w), -1);

		} else {
			/* push forward reference element for later */
			fwdref_t *fwdref = create_fwdref();
			fwdref->type = ST_CHR_SYM;
			fwdref->symbol_text = 
				MALLOC(char, strlen(si->value.sym->sym_id)+1);
			strcpy(fwdref->symbol_text,si->value.sym->sym_id);
			fwdref->order = order;
			fwdref->wc = si->value.sym->data.chr->wc_enc;
			fwdref->target_wc = tgt->data.chr->wc_enc;
			if (tgt->sym_type == ST_COLL_ELL) {
				fwdref->target_wc = tgt->data.collel->pc;
				fwdref->target_coll_ell_sym=
							tgt->data.collel->sym;
			}
			if (strcmp(tgt->sym_id,"UNDEFINED")==0)
				fwdref->is_UNDEFINED=TRUE;
			if (tgt->sym_type==ST_ELLIPSIS)
				Efwdref_push(fwdref);
			else
				fwdref_push(fwdref);
		        if ((order==0) && (tgt->sym_type != ST_ELLIPSIS)) {
			    if (next_weight == 0)
			        next_weight = nxt_coll_wgt();
			    set_coll_wgt(w,next_weight,-1);  
		        }
			set_coll_wgt(w,FWD_REF,order);
			return;
		    }
	    }

	    /* initialize all weights per POSIX 2.5.2.2.4 line 1973 */
	    if ((order==0) && (tgt->sym_type != ST_ELLIPSIS)) {
	        if (next_weight == 0)
		    next_weight = nxt_coll_wgt();
		set_coll_wgt(w,next_weight,-1);  
	    }

	    set_coll_wgt(w, get_abs_wgt(si->value.sym->data.chr->wgt), order);
	    break;
	    
	  case ST_COLL_ELL:	/* collation element */
	    ce = loc_collel(si->value.sym->data.collel->sym,
			    si->value.sym->data.collel->pc);
	    
	    weight=get_abs_wgt(&(ce->ce_wgt));
	    if (weight == UNDEFINED) {
		/*
		  a symbol with UNDEFINED collation can only appear on the
		  RHS if it is also the target on the LHS
		*/
		if (si->value.sym == tgt) {
		    if (order == 0)
		       set_coll_wgt(&(ce->ce_wgt),next_weight=nxt_coll_wgt(),-1);
		    else
		       set_coll_wgt(&(ce->ce_wgt),get_abs_wgt(w),-1);
				
                    }
		else {
			/* push forward reference element for later */
			fwdref_t *fwdref = create_fwdref();
			fwdref->type = ST_COLL_ELL;
			fwdref->symbol_text = 
				MALLOC(char, strlen(si->value.sym->sym_id)+1);
			strcpy(fwdref->symbol_text,si->value.sym->sym_id);
			fwdref->order = order;
			fwdref->wc = si->value.sym->data.collel->pc;
			fwdref->target_wc = tgt->data.chr->wc_enc;
			fwdref->coll_ell_sym = si->value.sym->data.collel->sym;
			if (tgt->sym_type == ST_COLL_ELL) {
				fwdref->target_wc = tgt->data.collel->pc;
				fwdref->target_coll_ell_sym=
							tgt->data.collel->sym;
			}
			if (strcmp(tgt->sym_id,"UNDEFINED")==0)
				fwdref->is_UNDEFINED=TRUE;
			if (tgt->sym_type==ST_ELLIPSIS)
				Efwdref_push(fwdref);
			else
				fwdref_push(fwdref);
		        if ((order==0) && (tgt->sym_type != ST_ELLIPSIS)) {
			    if (next_weight == 0)
			        next_weight = nxt_coll_wgt();
			    set_coll_wgt(w,next_weight,-1);  
		        }
			set_coll_wgt(w,FWD_REF,order);
			return;
		}
	    } 

            /* initialize all weights per POSIX 2.5.2.2.4 line 1973 */
	    if ((order==0) && (tgt->sym_type != ST_ELLIPSIS)) {
                if (next_weight == 0)
                    next_weight = nxt_coll_wgt();
                set_coll_wgt(w,next_weight,-1);
            }

	    set_coll_wgt(w, get_abs_wgt(&(ce->ce_wgt)), order);
	    break;

	  case ST_COLL_SYM:	/* collation symbol */
	    weight=get_abs_wgt(si->value.sym->data.collsym);
	    if (weight == UNDEFINED) {
		/*
		  a symbol with UNDEFINED collation can only appear on the
		  RHS if it is also the target on the LHS
	        */
		if (si->value.sym == tgt) {
		    if (order==0)
		        set_coll_wgt(tgt->data.collsym,
						next_weight=nxt_coll_wgt(),-1);
		    else
		        set_coll_wgt(tgt->data.collsym, get_abs_wgt(w),-1);
		} else {
			/* push forward reference element for later */
			fwdref_t *fwdref = create_fwdref();
			fwdref->type = ST_COLL_SYM;
			fwdref->symbol_text = 
				MALLOC(char, strlen(si->value.sym->sym_id)+1);
			strcpy(fwdref->symbol_text,si->value.sym->sym_id);
			fwdref->order = order;
			fwdref->coll_sym_wgt = si->value.sym->data.collsym;
			fwdref->target_wc = tgt->data.chr->wc_enc;
			if (tgt->sym_type == ST_COLL_ELL) {
				fwdref->target_wc = tgt->data.collel->pc;
				fwdref->target_coll_ell_sym =
							tgt->data.collel->sym;
			}
			if (strcmp(tgt->sym_id,"UNDEFINED")==0)
				fwdref->is_UNDEFINED=TRUE;
			if (tgt->sym_type==ST_ELLIPSIS)
				Efwdref_push(fwdref);
			else
				fwdref_push(fwdref);
		        if ((order==0) && (tgt->sym_type != ST_ELLIPSIS)) {
			    if (next_weight == 0)
			        next_weight = nxt_coll_wgt();
			    set_coll_wgt(w,next_weight,-1);  
		        }
			set_coll_wgt(w,FWD_REF,order);
			return;
		}
	    }

            /* initialize all weights per POSIX 2.5.2.2.4 line 1973 */
	    if ((order==0) && (tgt->sym_type != ST_ELLIPSIS)) {
                if (next_weight == 0)
                    next_weight = nxt_coll_wgt();
                set_coll_wgt(w,next_weight,-1);
            }

	    set_coll_wgt(w, get_abs_wgt(si->value.sym->data.collsym), order);
	    break;
 
	  case ST_ELLIPSIS:	/* ellipsis on the r.h.s. */

            if (strcmp(tgt->sym_id,"UNDEFINED") == 0)
	        break;       /* use default behavior in sem_collate */

            if (tgt->sym_type != ST_ELLIPSIS) {
                diag_error(ERR_BAD_ELLIPSIS1);
	        return;
            }

            /* initialize all weights per POSIX 2.5.2.2.4 line 1973 */
            if (order==0)
                set_coll_wgt(w,ELLIPSIS_WEIGHT,-1);
	    else
                set_coll_wgt(w,ELLIPSIS_WEIGHT,order);

            break;
        } /* switch */

    } else {
	/* 
	  collation substitution, i.e. <eszet>   <s><s>; <eszet>
	*/
	item_t **buf;
	int    n, count=0;
	char   *subs_weights;
	int    had_a_bogus_char = FALSE; /* any undefined chars/syms? */
	char   tmp_wgt_str[3];	/* temporary weight string            */

	/* 
	  pop all of the collation elements on the semantic stack and
	  create a string from each of their encodings.
	*/
	subs_weights = MALLOC(char, (i->value.int_no * MB_LEN_MAX) + 1);
	buf = MALLOC(item_t *, i->value.int_no);
	for (n=0; n < i->value.int_no; n++)
	    buf[n] = sem_pop();

        /* initialize all weights per POSIX 2.5.2.2.4 line 1973 */
	if ((order==0) && (tgt->sym_type != ST_ELLIPSIS)) {
            set_coll_wgt(w,next_weight=nxt_coll_wgt(),-1); 
	    if (tgt->sym_type != ST_COLL_ELL)
	        set_coll_wgt(tgt->data.chr->wgt, next_weight, -1);
	    else {
		ce = loc_collel(tgt->data.collel->sym,tgt->data.collel->pc);
	        set_coll_wgt(&(ce->ce_wgt), next_weight, -1);
	    }
	}

	for (n=i->value.int_no-1; n >= 0; n--) {
	    if (buf[n]->type == SK_SYM) {
		if (buf[n]->is_bogus == TRUE)
		    had_a_bogus_char = TRUE;

                switch (buf[n]->value.sym->sym_type) {
		    case ST_CHR_SYM:
	                weight=get_abs_wgt(buf[n]->value.sym->data.chr->wgt);
	                if (weight == UNDEFINED) {
			    if (buf[n]->value.sym == tgt) {
				/* assign collation weight for self reference */
				set_coll_wgt(buf[n]->value.sym->data.chr->wgt,
                                       weight=next_weight=get_abs_wgt(w), -1);
                                }
			    else {
				/* push forward reference element for later */
				fwdref_t *fwdref = create_fwdref();
				fwdref->symbol_text = MALLOC(char, 
					 strlen(buf[n]->value.sym->sym_id)+1);
				strcpy(fwdref->symbol_text,
						buf[n]->value.sym->sym_id);
				fwdref->type = ST_CHR_SYM;
				fwdref->order = order;
				fwdref->one_many_offset = count;
				fwdref->target_wc = tgt->data.chr->wc_enc;
				fwdref->wc=buf[n]->value.sym->data.chr->wc_enc;
				if (tgt->sym_type == ST_COLL_ELL) {
					fwdref->target_wc=tgt->data.collel->pc;
					fwdref->target_coll_ell_sym
							 =tgt->data.collel->sym;
				}
				if (strcmp(tgt->sym_id,"UNDEFINED")==0)
					fwdref->is_UNDEFINED=TRUE;
				if (tgt->sym_type==ST_ELLIPSIS)
					Efwdref_push(fwdref);
				else
					fwdref_push(fwdref);
				weight = FWD_REF;
			        }
                        }
			tmp_wgt_str[0] = ((char *)&weight)[0];
			tmp_wgt_str[1] = ((char *)&weight)[1];
			tmp_wgt_str[2] = '\0';
			break;
		    case ST_COLL_SYM:
		        weight=get_abs_wgt(buf[n]->value.sym->data.collsym);
		        if (weight == UNDEFINED) {
                            if (buf[n]->value.sym == tgt) {
			        /* assign collation weight for self reference */
				set_coll_wgt(buf[n]->value.sym->data.collsym,
					weight=next_weight=get_abs_wgt(w),-1);
				}
			    else {
				/* push forward reference element for later */
				fwdref_t *fwdref = create_fwdref();
				fwdref->type = ST_COLL_SYM;
				fwdref->symbol_text = MALLOC(char, 
					strlen(buf[n]->value.sym->sym_id)+1);
				strcpy(fwdref->symbol_text,
						buf[n]->value.sym->sym_id);
				fwdref->one_many_offset = count;
				fwdref->target_wc = tgt->data.chr->wc_enc;
				fwdref->order = order;
				fwdref->coll_sym_wgt = 
						buf[n]->value.sym->data.collsym;
				if (tgt->sym_type == ST_COLL_ELL) {
					fwdref->target_wc=tgt->data.collel->pc;
					fwdref->target_coll_ell_sym =
							 tgt->data.collel->sym;
				}
				if (strcmp(tgt->sym_id,"UNDEFINED")==0)
					fwdref->is_UNDEFINED=TRUE;
				if (tgt->sym_type==ST_ELLIPSIS)
					Efwdref_push(fwdref);
				else
					fwdref_push(fwdref);
				weight = FWD_REF;
				}
			    }
			tmp_wgt_str[0] = ((char *)&weight)[0];
			tmp_wgt_str[1] = ((char *)&weight)[1];
			tmp_wgt_str[2] = '\0';
			break;
		    case ST_COLL_ELL:
	                ce = loc_collel(buf[n]->value.sym->data.collel->sym,
			    buf[n]->value.sym->data.collel->pc);
	    
	                weight=get_abs_wgt(&(ce->ce_wgt));
		        if (weight == UNDEFINED) {
			    if (buf[n]->value.sym == tgt) {
				/* assign collation weight for self reference */
				set_coll_wgt(&(ce->ce_wgt),
				      weight=next_weight=get_abs_wgt(w), -1);
				}
			    else {
				/* push forward reference element for later */
				fwdref_t *fwdref = create_fwdref();
				fwdref->type = ST_COLL_ELL;
				fwdref->symbol_text = MALLOC(char, 
					 strlen(buf[n]->value.sym->sym_id)+1);
				strcpy(fwdref->symbol_text,
						buf[n]->value.sym->sym_id);
				fwdref->one_many_offset = count;
				fwdref->target_wc = tgt->data.chr->wc_enc;
				fwdref->order = order;
				fwdref->wc = buf[n]->value.sym->data.collel->pc;
				fwdref->coll_ell_sym = 
					    buf[n]->value.sym->data.collel->sym;
				if (tgt->sym_type == ST_COLL_ELL) {
					fwdref->target_wc =tgt->data.collel->pc;
					fwdref->target_coll_ell_sym = 
							tgt->data.collel->sym;
				}
				if (strcmp(tgt->sym_id,"UNDEFINED")==0)
					fwdref->is_UNDEFINED=TRUE;
				if (tgt->sym_type==ST_ELLIPSIS)
					Efwdref_push(fwdref);
				else
					fwdref_push(fwdref);
				weight = FWD_REF;
				}
			    }
			tmp_wgt_str[0] = ((char *)&weight)[0];
			tmp_wgt_str[1] = ((char *)&weight)[1];
			tmp_wgt_str[2] = '\0';
			break;
		    default:
			INTERNAL_ERROR;
		  } /* switch */
		strcat(subs_weights, tmp_wgt_str);
		}
	    else {
                diag_error(ERR_BAD_ELLIPSIS2);
		had_a_bogus_char = TRUE;
                }

	    destroy_item(buf[n]);
	count++;
	}
	free(buf);

	if (!had_a_bogus_char) {
	    /* add this weight string to substitution table as required */
	    sem_def_substr(order, w, subs_weights);
	    }
		
	free(subs_weights);
    }
}


/*
*  FUNCTION: sem_set_collwgt
*
*  DESCRIPTION:
*  Assigns the collation weights in the argument 'weights' to the character
*  popped off the symbol stack.
*/
void sem_set_collwgt(_LC_weight_t *weights)
{
    extern _LC_collate_t collate;
    symbol_t     *s;
    int          i;
    _LC_weight_t *tgt;
    _LC_collel_t *ce;

    s = sym_pop();

    if (s->is_bogus == TRUE)
	return;

    switch (s->sym_type) {

      case ST_CHR_SYM:
	tgt = s->data.chr->wgt;
	if (tgt == NULL)
	    s->data.chr->wgt = 
		&(collate.co_coltbl[s->data.chr->wc_enc].ct_wgt);
	break;

      case ST_COLL_ELL:
	ce = loc_collel(s->data.collel->sym, s->data.collel->pc);
	tgt = &(ce->ce_wgt);
	break;

      case ST_COLL_SYM:
	/* user specified weights are not valid for collating symbols    */
	/* use default weights for collating symbols:  see XPG4 5.3.2    */
        /* however "UNDEFINED" is treated as a valid collating symbol... */

        if ((strcmp(s->sym_id,"UNDEFINED")==0)) {
	    tgt = s->data.collsym;
        }
        else {
	    sym_push(s);
	    sem_set_dflt_collwgt();
	    return;
        }
        break;
         
    }

    if (has_substitution(weights)) {
	set_abs_wgt(tgt,get_abs_wgt(weights));
	assign_index_and_flag(tgt,get_index(weights));
	}
    else
        for (i=0; i<=_COLL_WEIGHTS_MAX; i++)
	    set_coll_wgt(tgt, get_coll_wgt(weights,i), i);
}


/*
*  FUNCTION: sem_get_coll_tgt
*
*  DESCRIPTION:
*  Returns a pointer to the symbol on top of the symbol stack.
*/
symbol_t *sem_get_coll_tgt()
{
    symbol_t *s;

    s = sym_pop();

    sym_push(s);
    return s;
}

/*
*  FUNCTION:  sem_make_ellipsis
*
*  DESCRIPTION:
*  Creates an ellipsis symbol (ST_ELLIPSIS) pushes it on the top of the
*  symbol stack and returns a pointer to it.
*
*  This is done so that coll_rhs_list grammar logic can be used for ellipsis.
*  See sem_existing_symbol for pointers on creating a symbol.....
*/

symbol_t *sem_make_ellipsis(void)
{
    symbol_t *s;

    s = create_symbol("__ELLPISIS__",0);
    s->sym_type = ST_ELLIPSIS;
    /* sym_push(s); */
    return s;
}


/* 
*  FUNCTION: sem_set_dflt_collwgt
*
*  DESCRIPTION:
*  Assign collation weight to character - set weight in symbol table
*  entry and in coltbl weight array.
*
*  The collation weight assigned is the next one available, i.e. the
*  default collation weight.
*/
void sem_set_dflt_collwgt(void)
{
    extern _LC_collate_t collate;
    wchar_t      weight;
    symbol_t     *sym;
    wchar_t      pc;
    _LC_collel_t *ce;
    
    
    sym = sym_pop();

    if (sym->is_bogus == TRUE)
	return;

    if (sym->sym_type != ST_CHR_SYM 
	&& sym->sym_type != ST_COLL_SYM && sym->sym_type != ST_COLL_ELL)
	INTERNAL_ERROR;

    pc = sym->data.chr->wc_enc;

    switch (sym->sym_type) {	/* handle character */
      case ST_CHR_SYM:
	/* check if character already specified elswhere */
	if ((get_coll_wgt(&(collate.co_coltbl[pc].ct_wgt),0) != UNDEFINED) &&
            (strcmp(sym->sym_id,"UNDEFINED") != 0)) {
	    diag_error(ERR_DUP_COLL_SPEC, sym->sym_id);
	    return;
	}

        if (strcmp(sym->sym_id,"UNDEFINED") == 0) {  /* case for UNDEFINED */
            if (get_coll_wgt(sym->data.chr->wgt, 0) != UNDEFINED) {
	        diag_error(ERR_DUP_COLL_SPEC, sym->sym_id);
	        return;
    	    }
 
	    /* get next available collation weight */
	    weight = nxt_coll_wgt();

            
	    /* put weight in symbol table entry for character. */
            set_coll_wgt(sym->data.chr->wgt, weight, -1);

         }
         else {  /* normal case */

	    /* get next available collation weight */
	    weight = nxt_coll_wgt();

	    /* place weight in colltbl */
	    set_coll_wgt(&(collate.co_coltbl[pc].ct_wgt), 
		         weight, -1);

	    /* put weight in symbol table entry for character. */
	    sym->data.chr->wgt = 
	        &(collate.co_coltbl[pc].ct_wgt);
        } /* if/then/else */

	break;

      case ST_COLL_ELL:
	ce = loc_collel(sym->data.collel->sym, sym->data.collel->pc);
	
	/* check if character already specified elswhere */
	if (get_coll_wgt(&(ce->ce_wgt), 0) != UNDEFINED) {
	    diag_error(ERR_DUP_COLL_SPEC, sym->sym_id);
	    return;
	}
	
	/* get next available collation weight */
	weight = nxt_coll_wgt();
	
	/* put weights in symbol table entry for character. */
	set_coll_wgt(&(ce->ce_wgt), weight, -1);
	
	break;

      case ST_COLL_SYM:
	/* check if character already specified elswhere */
	if (get_coll_wgt(sym->data.collsym, 0) != UNDEFINED) {
	    diag_error(ERR_DUP_COLL_SPEC, sym->sym_id);
	    return;
	}

	weight = nxt_coll_wgt();
	set_coll_wgt(sym->data.collsym, weight, -1);
	break;
    }
}


/* 
*  FUNCTION: sem_set_dflt_collwgt_range
*
*  DESCRIPTION:
*  Assign collation weights to a range of characters.  The functions
*  expects to find two character symbols on the semantic stack.
*
*  The collation weight assigned is the next one available, i.e. the
*  default collation weight.
*/
void sem_set_dflt_collwgt_range(void)
{
    extern symtab_t cm_symtab;
    extern _LC_collate_t collate;
    symbol_t *s1, *s0;
    int      start, end;
    wchar_t  weight;
    int      wc;
    int      i;
    
    /* 
      Issue warning message that using KW_ELLIPSIS results in the use of
      codeset encoding assumptions by localedef. 

      - required by POSIX.
    */
    diag_error(ERR_CODESET_DEP);

    /* pop symbols of symbol stack */
    s1 = sym_pop();
    s0 = sym_pop();
    
    if ((s1->is_bogus == TRUE) || (s0->is_bogus == TRUE))
	return;

    /* 
      ensure that both of these symbols are characters and not collation
      symbols or elements 
    */
    if (s1->sym_type != ST_CHR_SYM || s0->sym_type != ST_CHR_SYM) {
	diag_error(ERR_INVAL_COLL_RANGE, s0->sym_id, s1->sym_id);
	return;
    }

    /* get starting and ending points in file code */
    start = s0->data.chr->fc_enc;
    end = s1->data.chr->fc_enc;

    /* invalid symbols in range ?*/
    if (start > end)
	error(ERR_INVAL_COLL_RANGE, s0->sym_id, s1->sym_id);
	
    for (i=start; i <= end; i++) {

	if ((wc = wc_from_fc(i)) >= 0) {
	    /* check if already defined elsewhere in map */
	    if (get_coll_wgt(&(collate.co_coltbl[wc].ct_wgt), 
			     0) != UNDEFINED) {
		char bad_char_sym_id[80];   /* 80 should be big enough */
		/* use hex translation of file code for symbol id for error */
		sprintf(bad_char_sym_id, "0x%x", i);

		diag_error(ERR_DUP_COLL_RNG_SPEC,bad_char_sym_id,s0->sym_id, 
								   s1->sym_id);
		return;
	    }
	    /* get next available collation weight */
	    weight = nxt_coll_wgt();

	    /* collation weights for symbols assigned weights in a range
	       are not accessible from the symbol , i.e.

	       s->data.chr->wgt[x] = weight;

	       cannot be assigned here since we don't have the symbol
	       which refers to the file code.
	    */

	    /* put weight in coll table at spot for wchar encoding */
	    set_coll_wgt(&(collate.co_coltbl[wc].ct_wgt), weight, -1);
	    
	}
    }
}


/* 
*  FUNCTION: sem_set_collwgt_range
*
*  DESCRIPTION:
*  Assign collation weights to a range of characters.  The functions
*  expects to find two character symbols on the semantic stack.
*
*  The collation weight assigned is specified by the weights parameter:
*
*  Also update the substitution string table as needed
*
*  And, duplicate any items on the ellipsis forward reference stack for
*  each character in the range.  New items added to the normal forward
*  reference stack for processing in sem_collate().
*
*/
void sem_set_collwgt_range(_LC_weight_t *weights)
{
    extern symtab_t cm_symtab;
    extern _LC_collate_t collate;
    symbol_t *s1, *s0;
    int      start, end;
    wchar_t  weight;
    int      wc;
    int      i, j;
    _LC_collel_t *ce;
    int      has_ellipsis = FALSE;
    fwdref_t *frp;
    
    /* 
      Issue warning message that using KW_ELLIPSIS results in the use of
      codeset encoding assumptions by localedef. 

      - required by POSIX.
    */
    diag_error(ERR_CODESET_DEP);

    /* pop symbols of symbol stack */
    s1 = sym_pop();
    s0 = sym_pop();
    
    if ((s1->is_bogus == TRUE) || (s0->is_bogus == TRUE))
	return;

    /* 
      ensure that both of these symbols are characters and not collation
      symbols or elements 
    */
    if (s1->sym_type != ST_CHR_SYM || s0->sym_type != ST_CHR_SYM) {
	diag_error(ERR_INVAL_COLL_RANGE, s0->sym_id, s1->sym_id);
	return;
    }

    /* get starting and ending points in file code */
    start = s0->data.chr->fc_enc;
    end = s1->data.chr->fc_enc;

    /* invalid symbols in range ?*/
    if (start > end)
	error(ERR_INVAL_COLL_RANGE, s0->sym_id, s1->sym_id);

    has_ellipsis = FALSE;
    /* do we have ellipsis on right hand side? */
    for (j=0;j<collate.co_nord;j++)
    if (get_coll_wgt(weights,j) == ELLIPSIS_WEIGHT)
        has_ellipsis = TRUE;
	
    for (i=start; i <= end; i++) {

	if ((wc = wc_from_fc(i)) >= 0) {
	    /* check if already defined elsewhere in map */
	    if (get_coll_wgt(&(collate.co_coltbl[wc].ct_wgt), 
			     0) != UNDEFINED) {
		char bad_char_sym_id[80];   /* 80 should be big enough */
		/* use hex translation of file code for symbol id for error */
		sprintf(bad_char_sym_id, "0x%x", i);

		diag_error(ERR_DUP_COLL_RNG_SPEC,bad_char_sym_id,s0->sym_id, 
								s1->sym_id);
		return;
	    }

	    /* collation weights for symbols assigned weights in a range
	       are not accessible from the symbol , i.e.

	       s->data.chr->wgt[x] = weight;

	       cannot be assigned here since we don't have the symbol
	       which refers to the file code.
	    */

	    /* put weight in coll table at spot for wchar encoding */

            for (j=0; j<collate.co_nord; j++) {
                if (j==0) 
                   set_abs_wgt(&(collate.co_coltbl[wc].ct_wgt),nxt_coll_wgt());

                weight = get_coll_wgt(weights,j);

                /* should we insert weight of current position?? */
                if (weight == ELLIPSIS_WEIGHT) {    
                    weight = get_abs_wgt(&(collate.co_coltbl[wc].ct_wgt));
                }

                if ((has_substitution(weights)) && (!has_ellipsis)) {
	            set_abs_wgt(&(collate.co_coltbl[i].ct_wgt),
					get_abs_wgt(weights));
		    assign_index_and_flag(&(collate.co_coltbl[i].ct_wgt),
					get_index(weights));
		    }
		else if ((has_substitution(weights)) && (has_ellipsis)) {
		    if (get_coll_wgt(weights,j) == ELLIPSIS_WEIGHT) {
			char wgtstr[3];
			wgtstr[0] = ((char *)&weight)[0];
			wgtstr[1] = ((char *)&weight)[1];
			wgtstr[2] = '\0';
			sem_def_substr(j,&(collate.co_coltbl[i].ct_wgt),wgtstr);
			}
		    else {
			sem_def_substr(j,&(collate.co_coltbl[i].ct_wgt),
			    collate.co_subs[get_index(weights)].tgt_wgt_str[j]);
			}
		    }
		else
                    set_coll_wgt(&(collate.co_coltbl[wc].ct_wgt), weight, j);
            }

	/* process any elements that may be on the Ellipsis Forward Reference */
	/* stack.							      */

	frp = Efwdref_getfirst();
	while (frp != NULL) {
	    frp=fwdref_dup(frp);
	    frp->target_wc = wc;
	    fwdref_push(frp);
	    frp = Efwdref_getnext();
	    }
	}
    }

    if ((has_ellipsis) && (has_substitution(weights)))
	/* the get_index(weights)'th entry in the substitution table is    */
	/* no longer being used.  Clean up by removing this entry from the */
	/* substitution table.  This is not strictly necessary, but a good */
	/* idea.							   */
	remove_subst_entry(get_index(weights));

    /* clean up ellipsis forward reference stack */
    Efwdref_clearall();
}


/*
*  FUNCTION: sem_sort_spec
*
*  DESCRIPTION:
*  This function decrements the global order by one to compensate for the 
*  extra increment done by the grammar, and then copies the sort modifier
*  list to each of the substrings defined thus far.
*/
void sem_sort_spec()
{
    extern _LC_collate_t collate;
    extern symtab_t cm_symtab;
    extern wchar_t max_wchar_enc;
    extern int copying_collate;
    short    low;
    symbol_t *s;
    item_t   *it;
    int      i, j;
    unsigned short    *buf;

    if ((collate.co_nord > 1) && (!copying_collate))
	collate.co_nord --;

    /*
      Get sort values from top of stack and assign to collate.co_sort
    */
    for (i = (collate.co_nord-1) ; i >= 0; i--) {
	it = sem_pop();
	collate.co_sort.n[i] = it->value.int_no;
	destroy_item(it);
    }

    /* 
      seed the symbol table with IGNORE,  <LOW> and <HIGH> 
    */

    /* 
      IGNORE gets a special collation value .  The xfrm and coll
      logic must recognize zero and skip a character possesing this collation
      value.
    */
    s = create_symbol("IGNORE", 0);
    s->sym_type = ST_CHR_SYM;
    s->data.chr = MALLOC(chr_sym_t, 1);
    s->data.chr->wc_enc = 0;
    s->data.chr->width = 0;
    s->data.chr->len = 0;
    s->data.chr->wgt = MALLOC(_LC_weight_t, 1);
    for (i=0;i<=_COLL_WEIGHTS_MAX;i++)
        s->data.chr->wgt->n[i] = 0;
    set_coll_wgt(s->data.chr->wgt, IGNORE, -1);
    add_symbol(&cm_symtab, s);
    
    /*
      LOW gets the first available collation value.  All subsequent characters
      will get values higher than this unless they collate with <LOW>
    */
    s = create_symbol("<LOW>", 0);	/* <LOW> */
    s->sym_type = ST_CHR_SYM;
    s->data.chr = MALLOC(chr_sym_t, 1);
    s->data.chr->wc_enc = 0;
    s->data.chr->width = 0;
    s->data.chr->len = 0;
    s->data.chr->wgt = MALLOC(_LC_weight_t, 1);
    for (i=0;i<=_COLL_WEIGHTS_MAX;i++)
        s->data.chr->wgt->n[i] = 0;
    set_coll_wgt(s->data.chr->wgt, low = nxt_coll_wgt(), -1);
    collate.co_col_min = low;
    add_symbol(&cm_symtab, s);
    /* 
      HIGH gets the maximum collation value which can be contained in a 
      short.  This ensures that all values will collate < then <HIGH>

      This is just a temporary until the last collation weight has actually
      been assigned.  We will reset every collation weight which is <HIGH>
      to the current max plus one at gen() time.
    */
    s = create_symbol("<HIGH>", 0);
    s->sym_type = ST_CHR_SYM;
    s->data.chr = MALLOC(chr_sym_t, 1);
    s->data.chr->wc_enc = 0;
    s->data.chr->width = 0;
    s->data.chr->len = 0;
    s->data.chr->wgt = MALLOC(_LC_weight_t, 1);
    for (i=0;i<=_COLL_WEIGHTS_MAX;i++)
        s->data.chr->wgt->n[i] = 0;
    set_coll_wgt(s->data.chr->wgt, USHRT_MAX, -1);
    collate.co_col_max = 0;
    add_symbol(&cm_symtab, s);
    
    /*
      UNDEFINED may collate <HIGH> or <LOW>.  By default, characters not
      specified in the collation order collate <HIGH>.
    */

/* actually:  UNDEFINED should be able to define where ever it is specified */

    s = create_symbol("UNDEFINED", 0);
    s->sym_type = ST_CHR_SYM;
    s->data.chr = MALLOC(chr_sym_t, 1);
    s->data.chr->wc_enc = 0;
    s->data.chr->width = 0;
    s->data.chr->len = 0;

    s->data.chr->wgt = MALLOC(_LC_weight_t, 1);
    for (i=0;i<=_COLL_WEIGHTS_MAX;i++)
        s->data.chr->wgt->n[i] = 0;
    set_coll_wgt(s->data.chr->wgt, UNDEFINED, -1);

    add_symbol(&cm_symtab, s);
}


/* 
*  FUNCTION: sem_def_collel
*
*  DESCRIPTION:
*  Defines a collation ellement. Creates a symbol for the collation element
*  in the symbol table, creates a collation element data structure for
*  the element and populates the element from the string on the semantic 
*  stack.
*/
void sem_def_collel()
{
    extern _LC_collate_t collate;
    extern symtab_t cm_symtab;
    symbol_t     *sym_name;	/* symbol to be defined                 */
    item_t       *it;		/* string which is the collation symbol */
    char         *s;		/* translated copy of string            */
    wchar_t      pc;		/* process code for collation symbol    */
    _LC_collel_t *coll_sym;	/* collation symbol pointer             */
    char         *sym;		/* translated collation symbol          */
    int          n_syms;	/* no. of coll syms beginning with char */
    int          rc;
    int          i, j, skip;
    char	 *temp_ptr;

    _LC_charmap_objhdl_t null_object ={0,0};

    sym_name = sym_pop();	/* get coll symbol name off symbol stack */
    it = sem_pop();		/* get coll symbol string off of stack */

    if (it->type != SK_STR)
	INTERNAL_ERROR;

    /* Create symbol in symbol table for coll symbol name */
    sym_name->sym_type = ST_COLL_ELL;
    sym_name->data.collel = MALLOC(coll_ell_t,1);
    add_symbol(&cm_symtab, sym_name);
    
    temp_ptr = MALLOC(char,strlen(it->value.str) +1);
    copy(temp_ptr,it->value.str);
    /* Translate collation symbol to file code */
    copy_string(&s, it->value.str);

    /* Allocate space for source string */
    sym = MALLOC(char, strlen(s)+1);
    strcpy(sym, s);
    
    /* 
      Determine process code for collation symbol.  The process code for
      a collation symbol is that of the first character in the symbol.
    */
    rc = CALL_METH(METH_OFFS(CHARMAP_MBTOWC))(null_object, &pc, temp_ptr, 
								MB_LEN_MAX);

    if (rc < 0) {
	diag_error(ERR_ILL_CHAR, it->value.str);
	return;
    }
    skip = 0;
    for (i = 0; i < rc; i++) {
        if (temp_ptr[i] < 128)
	    skip++;
	else
	    skip +=6;
    }

    /* Now finished with the temp array, free it */
    free(temp_ptr);

    /* save process code and matching source str in symbol */
    /* do not put the first character in the src str */
    sym_name->data.collel->pc = pc;
    sym_name->data.collel->str = MALLOC(char, strlen(sym)+1);
    strcpy(sym_name->data.collel->str, sym);	/* need whole string here */

    /* skip first character in string */
    sym = &sym[skip];

    sym_name->data.collel->sym = MALLOC(char, strlen(sym)+1);
    strcpy(sym_name->data.collel->sym, sym);

    if (collate.co_coltbl[pc].ct_collel != NULL) {
	/* 
	  At least one collation symbol exists already --
	  Count number of collation symbols with the process code 
	*/
	for (i=0;
	     collate.co_coltbl[pc].ct_collel[i].ce_sym != NULL;
	     i++);
    
	/* 
	  Allocate memory for 
	     current number + new symbol + terminating null symbol
	*/
	coll_sym = calloc(i+2,sizeof(_LC_collel_t));
	n_syms = i;
    } else {
	/* 
	  This is the first collation symbol, allocate for 

	  new symbol + terminating null symbol
	*/
	coll_sym = calloc(2,sizeof(_LC_collel_t));
	n_syms = 0;
    }
    
    if (coll_sym == NULL)
	INTERNAL_ERROR;
    
    /* Add collation symbols to list in sorted order */
    for (i=j=0; i < n_syms; i++,j++) {
	int   c;

	c = strcmp(sym, collate.co_coltbl[pc].ct_collel[i].ce_sym);
	if (c < 0 && i == j) {
	    coll_sym[j].ce_sym = sym;
	    set_coll_wgt(&(coll_sym[j].ce_wgt), UNDEFINED, -1);
	    j++;
	} 
	coll_sym[j].ce_sym = collate.co_coltbl[pc].ct_collel[i].ce_sym;
	set_coll_wgt(&(coll_sym[j].ce_wgt), UNDEFINED, -1);
    }
    if (i==j) {
	/* 
	  either subs was empty or new substring is greater than any other
	  to date 
	*/
	coll_sym[j].ce_sym = sym;
	set_coll_wgt(&(coll_sym[j].ce_wgt), UNDEFINED, -1);
	j++;
    }
    /* Add terminating NULL symbol */
    coll_sym[j].ce_sym = NULL;

    /* free space occupied by old list */
    if (n_syms>0)
	free(collate.co_coltbl[pc].ct_collel);
    
    /* attach new list to coll table */
    collate.co_coltbl[pc].ct_collel = coll_sym;

    destroy_item(it);

    collate.co_special |= _LOC_HAS_MCCE;  /*can't use fast path collation*/
}


/* 
*  FUNCTION: sem_spec_collsym
*
*  DESCRIPTION:
*  Defines a placeholder collation symbol name.  These symbols are typically
*  used to assign collation values to a set of characters.
*/
void sem_spec_collsym()
{
    extern symtab_t cm_symtab;
    symbol_t *sym,*t;

    sym = sym_pop();		/* get coll symbol name off symbol stack */

    t = loc_symbol(&cm_symtab,sym->sym_id,0);
    if (t != NULL)
	diag_error(ERR_DUP_COLL_SYM,sym->sym_id);
    else {
        /* Create symbol in symbol table for coll symbol name */
        sym->sym_type = ST_COLL_SYM;
        sym->data.collsym = calloc(1, sizeof(_LC_weight_t));
        set_coll_wgt(sym->data.collsym, UNDEFINED, -1);
        add_symbol(&cm_symtab, sym);
    }
}


/*
*  FUNCTION: sem_collate
*
*  DESCRIPTION:
*  Post processing for collation table which consists of the location
*  and assignment of specific value for <HIGH> and UNDEFINED collation
*  weights.
*/
void sem_collate()
{
    extern _LC_collate_t collate;
    extern wchar_t max_wchar_enc;
    extern symtab_t cm_symtab;
    wchar_t high_weight;
    _LC_weight_t *undefined;
    _LC_weight_t *high;
    _LC_collel_t *ce;
    _LC_weight_t *wgt_ptr;
    symbol_t     *s;
    int          i, j, k;
    int          warn=FALSE;
    int		 index;
    fwdref_t	 *fr;
    wchar_t	 weight;
    char         *ss;
    

    s = loc_symbol(&cm_symtab, "UNDEFINED", 0);
    if (s==NULL)
	INTERNAL_ERROR;
    undefined = s->data.chr->wgt;

    high_weight = nxt_coll_wgt();
    if (undefined == NULL) {
        warn = TRUE;
        set_coll_wgt(undefined, high_weight, -1);
        }

    if (get_coll_wgt(undefined, 0)==UNDEFINED) {
	warn = TRUE;
        set_coll_wgt(undefined, high_weight, -1);
        }

                       	/* we may have forward defined symbols in the  */
			/* definition of UNDEFINED.  scan through list */
			/* of forward symbols.  If we find any for     */
			/* UNDEFINED, then do replacement, and remove  */
			/* that particular element from the stack      */
	    
    fwdref_start_scan();
    while ((fr = fwdref_getnext()) != NULL) {
	if (fr->is_UNDEFINED == TRUE) {
	    switch (fr->type) {
		case ST_CHR_SYM:
		    weight=get_abs_wgt(&(collate.co_coltbl[fr->wc].ct_wgt));
		    break;
		case ST_COLL_ELL:
		    ce = loc_collel(fr->coll_ell_sym, fr->wc);
		    weight=get_abs_wgt(&(ce->ce_wgt));
		    break;
		case ST_COLL_SYM:
		    weight=get_abs_wgt(fr->coll_sym_wgt);
		    break;
		default:
		    INTERNAL_ERROR;
		} /* switch */

	    if (weight==UNDEFINED) {
	        weight = 0;  
		diag_error(ERR_SYM_UNDEF,fr->symbol_text);
	    }

	    /* UNDEFINED is not a collating element, which simplifies this*/
	    if (fr->one_many_offset == -1)  /* simply set weight */
	        set_coll_wgt(undefined,weight,fr->order);
	    else {
		if (!has_substitution(undefined))
		    INTERNAL_ERROR;
		index = get_index(undefined);

	        collate.co_subs[index].tgt_wgt_str[fr->order]
                               [2*fr->one_many_offset]=((char *)&weight)[0];
	        collate.co_subs[index].tgt_wgt_str[fr->order]
                             [2*fr->one_many_offset+1]=((char *)&weight)[1];
	        }
		
	    fwdref_remove_element(fr);  /* only if for UNDEFINED... */
	    }
	}

    s = loc_symbol(&cm_symtab, "<HIGH>", 0);
    if (s==NULL)
	INTERNAL_ERROR;
    high = s->data.chr->wgt;
    
    /* assign a collation weight to <HIGH> */
    set_coll_wgt(high, high_weight, -1);

    /* 
      Substitute symbols with UNDEFINED, and <HIGH> weights
      for the weights ultimately determined for UNDEFINED and <HIGH>.
    */
    for (i=0; i<=max_wchar_enc; i++) {

	if (wchar_defined(i)){
	    for (j=0; j <= _COLL_WEIGHTS_MAX; j++) {
	        if (get_coll_wgt(&(collate.co_coltbl[i].ct_wgt), j) ==
                                                        (wchar_t) UNDEFINED) {
                    if (has_substitution(undefined))
		        assign_index_and_flag(&(collate.co_coltbl[i].ct_wgt),
					get_index(undefined));
		    else
		        set_coll_wgt(&(collate.co_coltbl[i].ct_wgt),
                                                 get_coll_wgt(undefined, j), j);
		    if (warn)
		        diag_error(ERR_NO_UNDEFINED);
			 warn = FALSE;
		 }

	     /* IGNORE's must show up by themselves (per XPG4 grammar) */

	    if (get_coll_wgt(&(collate.co_coltbl[i].ct_wgt),j)==(wchar_t)IGNORE)
	        set_coll_wgt(&(collate.co_coltbl[i].ct_wgt), 0, j);
	    }

	    if (collate.co_coltbl[i].ct_collel != NULL) {

		for (j=0, ce=&(collate.co_coltbl[i].ct_collel[j]); 
		     ce->ce_sym != NULL; 
		     ce=&(collate.co_coltbl[i].ct_collel[j++])) {
		
		    for (k=0; k<=_COLL_WEIGHTS_MAX; k++) {
		        if (get_coll_wgt(&(ce->ce_wgt), k)==(wchar_t) UNDEFINED) {
                            if (has_substitution(undefined))
		                assign_index_and_flag(&(ce->ce_wgt),
							get_index(undefined));
			    else
		                set_coll_wgt(&(ce->ce_wgt),
                                                 get_coll_wgt(undefined, k), k);
                        }
			if (get_coll_wgt(&(ce->ce_wgt), k) == (wchar_t) IGNORE)
			    set_coll_wgt(&(ce->ce_wgt), 0, k);
		    }
		}
	    }
	}
    }

    /* now update the FWD_REF weights */
    while ((fr=fwdref_pop())!=NULL) {
	switch (fr->type) {
	    case ST_CHR_SYM:
		weight=get_abs_wgt(&(collate.co_coltbl[fr->wc].ct_wgt));
		break;
	    case ST_COLL_ELL:
		ce = loc_collel(fr->coll_ell_sym, fr->wc);
		weight=get_abs_wgt(&(ce->ce_wgt));
		break;
	    case ST_COLL_SYM:
		weight=get_abs_wgt(fr->coll_sym_wgt);
		break;
	    default:
		INTERNAL_ERROR;
	    } /* switch */

	    if (weight==UNDEFINED) {
	        weight = 0;  
		diag_error(ERR_SYM_UNDEF,fr->symbol_text);
	    }

	    if (fr->one_many_offset == -1) { /* simply set weight */
		if (fr->target_coll_ell_sym != NULL) {
			ce = loc_collel(fr->target_coll_ell_sym, fr->target_wc);
			if (get_coll_wgt(&(ce->ce_wgt),fr->order) != FWD_REF)
				INTERNAL_ERROR;
			set_coll_wgt(&(ce->ce_wgt),weight,fr->order);
			}
		else {
	        	set_coll_wgt(&(collate.co_coltbl[fr->target_wc].ct_wgt),
						 	      weight,fr->order);
			}
	        }
	    else {
		if (fr->target_coll_ell_sym != NULL) {
			ce = loc_collel(fr->target_coll_ell_sym, fr->target_wc);
			wgt_ptr = &(ce->ce_wgt);
			}
		else 
			wgt_ptr = &(collate.co_coltbl[fr->target_wc].ct_wgt);

		if (!has_substitution(wgt_ptr))
		    INTERNAL_ERROR;
		index = get_index(wgt_ptr);

	        collate.co_subs[index].tgt_wgt_str[fr->order]
                                 [2*fr->one_many_offset]=((char *)&weight)[0];
	        collate.co_subs[index].tgt_wgt_str[fr->order]
                                 [2*fr->one_many_offset+1]=((char *)&weight)[1];
	    }
	    free(fr);
	}
}
