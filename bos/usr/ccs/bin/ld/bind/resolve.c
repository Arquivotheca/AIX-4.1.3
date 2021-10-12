#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)01	1.44  src/bos/usr/ccs/bin/ld/bind/resolve.c, cmdld, bos411, 9428A410j 5/12/94 13:23:14")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS (for binder subcommands):
 *			comprld
 *			gc
 *			keep
 *			mismatch
 *			resolve
 *			unres
 *   GLOBAL FUNCTIONS:	dfs
 *			display_symbol
 *			display_resolve_errors
 *			free_resolve_names
 *   STATIC FUNCTIONS:	check_ext_refs
 *			compare_hash
 *			comprldx
 *			create_descriptor
 *			create_generated_import_SYMBOL
 *			display_typecheck_errors
 *			dbg_opt4
 *			do_dot_name
 *			do_plain_name
 *			find_or_create_toc_entry
 *			get_toc_anchor
 *			print_refs_for_sym
 *			resolve_hidden_symbol
 *			resolve_name
 *			resolve_symbol
 *			UA_func_ref
 *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bind.h"
#include "global.h"
#include "error.h"
#include "strs.h"

#include "symbols.h"
#include "objects.h"
#include "match.h"
#include "insert.h"
#include "resolve.h"
#include "dump.h"
#include "stats.h"
#include "ifiles.h"
#include "util.h"

/* Static symbols */
static SYMBOL	*toc0_symbol;		/* Unique TOC anchor found during
					   symbol resolution. */
static int	kept_symbols = 0;	/* Count of symbols kept (i.e., not
					   garbage collected. */
static int	KEEP_count;		/* Number of symbols kept by
					   KEEP command. */
static int	delcsect_message_needed; /* Extra heading needed in loadmap
					    for undefined symbols. */

/* Dummy symbol to use in case a TOC-relative reference is made to a symbol
   in a section that does not also contain a TOC anchor. */
static SYMBOL	dummy_TOC_anchor = {
    &NULL_STR,				/* s_name */
    (SYMBOL *)NULL,			/* s_next_in_csect */
    (SYMBOL *)NULL,			/* s_synonym */
    0,					/* s_addr */
    INPNDX_GENERATED,			/* s_inpndx */
    -1,					/* s_number */
    XMC_TC0,				/* s_smclass */
    XTY_SD,				/* s_smtype */
    S_PRIMARY_LABEL | S_HIDEXT,		/* s_flags */
#ifdef DEBUG
    (OBJECT *)NULL,			/* s_object */
    (CSECT *)NULL,			/* s_csect */
    (TYPECHK *)NULL,			/* s_typechk */
    (SYMBOL *)NULL,			/* s_resolved */
    (SYMBOL *)NULL,			/* s_prev_in_gst */
#else
    (OBJECT *)NULL,			/* u.o / u.csect */
    (TYPECHK *)NULL,			/* u1.t / u1.r / u1._prev_in_gst */
#endif
};

/* Array to contain names for which errors must be printed.  The flags
   field identifies the kind of message needed. */
int	resolve_names_flags;		/* Logical OR of all flags in
					   resolve_names[] array. */
static int	resolve_names_count;	/* Count of names in resolve_names[] */
struct resolve_info *resolve_names;

/* Macro to add a name to resolve_names[] */
#define add_to_resolve_names(n,flgs) { \
    if (!(n->flags & STR_ERROR)) { \
	n->flags |= STR_ERROR; \
	resolve_names[(n->str_value=resolve_names_count++)].name = n; \
	resolve_names[n->str_value].flags=(flgs); \
    } else { resolve_names[n->str_value].flags |= (flgs);} \
    resolve_names_flags |= (flgs); \
}

/* Forward declarations */
static SYMBOL	*resolve_symbol(SYMBOL *, int);
static SYMBOL	*resolve_hidden_symbol(SYMBOL *);
static SYMBOL	*create_descriptor(STR *, SYMBOL *,  SYMBOL *, OBJECT *);
static SYMBOL	*create_generated_import_SYMBOL(STR *, SYMBOL *, SYMBOL *);
static SYMBOL	*find_or_create_toc_entry(RLD *, CSECT *, const int);

#ifdef DEBUG
int		dfs_depth;
#endif

/************************************************************************
 * Name: keep			keep command
 *									*
 * Purpose: Designate symbols to be kept in the final output.  Only
 *	external names can be kept, but a kept symbol doesn't have to be
 *	defined before the "keep" command is executed.
 *
 *	NOTE:  Specifying .foo or foo can cause either foo and .foo to
 *		be kept. (See resolve_name().)
 *									*
 * Command Format:							*
 *	KEEP name ...
 *									*
 * Parms/Returns:							*
 *	Returns: RC_OK
 *									*
 ************************************************************************/
RETCODE
keep(char *args[])			/* argv-style arguments */
{
    char	*arg;
    STR		*name;

    arg = args[1];
    do {
	name = putstring(arg);		/* Find or add name */
	if (!(name->flags & STR_KEEP)) {
	    name->flags |= STR_ISSYMBOL | STR_KEEP;
	    KEEP_count++;
	}
	if (Switches.verbose)
	    say(SAY_NORMAL,
		NLSMSG(KEEP_MSG, "%1$s: Symbol %2$s will be kept."),
		Command_name, arg);
    } while (moretokens(&arg, 1) > 0);
    Bind_state.state |= STATE_RESOLVE_NEEDED;
    return RC_OK;
}
/***************************************************************************
 * Name:	compare_hash
 *
 * Purpose:	Compare 2 type-checking hash strings
 *
 * Results:	0 if type-checking hash strings are compatible
 *		1 otherwise
 * *************************************************************************/
static int
compare_hash(TYPECHK *t1,
	     TYPECHK *t2)
{
    if (!Switches.hash_chk)		/* Do not check hash if
					   "setopt notypchk" was used. */
	return 0;			/* Match by default */

    /* If either is a null hash or pointers are identical,
       then there is no mismatch */
    if (t1 == NULL || t2 == NULL || t1 == t2)
	return 0;			/* Match */

    if (t1->t_len != t2->t_len) {	/* don't check mix of old and new */
	if ( (t1->t_len == TYPCHKSZ && t2->t_len == OLDTYPCHKSZ)
	    || (t1->t_len == OLDTYPCHKSZ && t2->t_len == TYPCHKSZ))
	    return 0;			/* Match */
	else
	    return 1;			/* Mismatch */
    }

    if (t1->t_len != TYPCHKSZ)	/* Don't check old hash ? */
	return 0;			/* Match */

/*
  The following compares the Parameter Type check of one external to another.
  The typcheck structure contains three parts,
	t_lang (short ) 	language code (eg. C, FORTRAN,,, ),
        t_ghash[4] (char) 	contains the global hash and
	t_lhash[4] (char)	contains the language specific hash.

*/

    /* Check for both "global hashes" not equal to "universal hash",
       and mismatch between "global hashes" */
    if (   memcmp(t1->t_typechk.t_ghash, universal_hash, 4) != 0
	&& memcmp(t2->t_typechk.t_ghash, universal_hash, 4) != 0
	&& memcmp(t1->t_typechk.t_ghash, t2->t_typechk.t_ghash, 4) != 0)
	return 1;			/* Mismatch */

    /* "Global hashes" match--check for same "language"  */
    if( t1->t_typechk.t_lang != t2->t_typechk.t_lang )
	return 0;			/* Match */

    /* "global hashes" match and languages are the same. */
    /* Check for "language hash"  mismatch */
    if (   memcmp(t1->t_typechk.t_lhash, universal_hash, 4) == 0
	|| memcmp(t2->t_typechk.t_lhash, universal_hash, 4) == 0
	|| memcmp(t1->t_typechk.t_lhash, t2->t_typechk.t_lhash, 4) == 0)
	return 0;			/* Match */

    return 1;				/* Mismatch */
} /* compare_hash */
/***********************************************************************
 * Name:	dbg_opt4()
 *
 * Purpose:	Check the value of two TOC entries to make sure they refer to
 *		the same address so they can be combined.  This check is only
 *		performed when the option dbopt4 is set.  This requires
 *		reading the values from the .data sections of the symbols.
 *
 * Results:	If two combinable TOC entries contain different values, display
 *		an error and return 0, so TOC resolution won't occur for this
 *		symbol.  Also, return 0 if any errors occur trying to read
 *		the files.  Otherwise, return 1.
 * ***********************************/
static int
dbg_opt4(SYMBOL *sym1,			/* TOC symbols to check */
	 SYMBOL *sym2,
	 STR	*name)			/* Name for errors only. */
{
    int		i;
    unsigned long TOC[2];
    SYMBOL	*syms[2];
    OBJECT	*o;

    syms[0] = sym1;
    syms[1] = sym2;
    for (i = 0; i < 2; i++) {
	o = syms[i]->s_csect->c_srcfile->sf_object;

	if (ifile_reopen_remap(o->o_ifile))
	    return 0;			/* Can't check--assume mismatch */

#ifdef READ_FILE
	if (o->o_ifile->i_access == I_ACCESS_READ) {
	    if (fseek_read(o->o_ifile,
			   o->oi_section_info[syms[i]->s_csect->c_secnum-1]
			   	.u.raw_offset + syms[i]->s_addr,
			   &TOC[i],
			   sizeof(TOC[i])) != 0)
		return 0;		/* Can't check--assume mismatch */
	}
	else
#endif
	    memcpy(&TOC[i],
		   o->o_ifile->i_map_addr + syms[i]->s_addr +
		   o->oi_section_info[syms[i]->s_csect->c_secnum-1].
		   	u.raw_offset,
		   4);

	/* Subtract origin address of symbol.  The TOC symbols under
	   consideration have exactly 1 RLD, so we don't have to check for
	   NULLs. */
	TOC[i] -= syms[i]->s_csect->c_first_rld->r_sym->s_addr;
    }

    if (TOC[1] != TOC[2]) {
	bind_err(SAY_NORMAL, RC_WARNING,
		 NLSMSG(RSLV_NOTCMATCH,
 "%1$s: 0711-194 WARNING: TOC entries for symbol %2$s contain different\n"
 "\tvalues. The entries are not being combined."),
		 Main_command_name, name->name);
	for (i = 0; i < 2; i++) {
	    say(SAY_NO_NL, NLSMSG(LIT_VALUE, " Value = 0x%08x%*s"),
		TOC[i], MINIDUMP_NAME_LEN - 18, "");
	    minidump_symbol(syms[i], 0,
			    MINIDUMP_SYMNUM_DBOPT11
			    | MINIDUMP_INPNDX
			    | MINIDUMP_ADDRESS
			    | MINIDUMP_TYPE
			    | MINIDUMP_SMCLASS
			    | MINIDUMP_SOURCE_INFO,
			    NULL);
	    dump_rld(syms[i]->s_csect->c_first_rld);
	}

	return 0;			/* Mismatch--don't combine */
    }
    return 1;				/* Match */
} /* dbg_opt4 */
/***********************************************************************
 * Name:	resolve_hidden_symbol()
 *
 * Purpose:	Resolve a symbol with a hidden (C_HIDEXT) name.  Ordinarily,
 *		hidden symbols resolve to themselves, with two exceptions.  All
 *		TOC anchors (XMC_TC0) resolve to a single arbitrary TOC anchor,
 *		and other TOC symbols (XMC_TC) are combined if
 *		1) They have the same name (or are both unnamed)
 *		2) They are names of CSECTs 4 bytes long
 *		3) They each have exactly 1 RLD item and
 *		4) the RLD items refer to symbols with the same global (C_EXT)
 *			name.
 *
 *		Note that it is possible for the hidden symbols of a given name
 *		to be resolved to several equivalence classes:  1 for the TOC
 *		anchor, 1 each for Toc symbols referring to separate RLD names,
 *		and singular classes for other hidden symbols not meeting the
 *		requirements for TOC resolution.
 *
 * Results:	If the symbol is a toc anchor, the global variable
 *		"toc0_symbol" is checked.  If it is NULL, the symbol is
 *		assigned to "toc0_symbol" to be used as the arbitrary
 *		toc anchor.
 *
 *		The resolved-to symbol is returned, and is also saved in the
 *		input symbol's s_resolved field, and the S_RESOLVED_OK flag is
 *		set, so subsequent calls can use the s_resolved
 *		field directly.  The S_ISTOC flag is set if TOC combining
 *		occurred.
 *
 *		NULL is returned if the referenced symbol was deleted by
 *		delcsect processing.
 *
 ************************************/
static SYMBOL *
resolve_hidden_symbol(SYMBOL *sym)
{
    SYMBOL	*sym2;
    RLD		*r;

    if (sym->s_flags & S_RESOLVED_OK) {
	/* This hidden symbol has already been resolved,
	   so the "s_resolved" field is valid. */
	return sym->s_resolved;
    }

#ifdef DEBUG
    if (bind_debug & RESOLVE_DEBUG) {
	if (bind_debug & DEBUG_LONG) {
	    int i;
	    for (i = dfs_depth; i > 0; i--)
		say(SAY_NO_NLS | SAY_NO_NL, ".");
	}
	show_inpndx(sym, "Calling resolve_hidden_symbol([%s]");
	say(SAY_NO_NLS, "<%s>{0x%x})", sym->s_name->name, sym);
    }
#endif

    sym->s_flags |= S_RESOLVED_OK;	/* Set flag--we fill in s_resolved */

    if (sym->s_flags & S_DUPLICATE) {
	/* Symbol was deleted by delcsect processing.  Therefore, the csect
	   must have multiple labels, at least one of which must be
	   external. These symbols are listed by the 'er' command. */
	add_to_resolve_names(sym->s_name, ERR_HID_UNRESOLVED);
	sym->s_resolved = NULL;
	return NULL;
    }

    if (sym->s_smclass == XMC_TC0) {
	/* All TOC anchors get resolved to the first TOC anchor encountered. */
	if (toc0_symbol == NULL) {
	    toc0_symbol = sym;
	    Bind_state.o_toc_sym = toc0_symbol; /* Unique TOC anchor */
	}
	else if (sym->s_csect->c_align > toc0_symbol->s_csect->c_align)
	    /* Compute maximum alignment */
	    toc0_symbol->s_csect->c_align = sym->s_csect->c_align;
	sym->s_resolved = toc0_symbol;
	return toc0_symbol;
    }

    /* TOC Csect must be 4 bytes long, or it doesn't qualify for resolution */
    if (!(	/* Check for conditions allowing for TOC symbol resolution. */
	  sym->s_smclass == XMC_TC
	  && sym->s_csect->c_len == 4
	  && (r = sym->s_csect->c_first_rld)	/* Symbol has at least 1 RLD */
	  && r->r_next == NULL			/* Symbol has exactly 1 RLD */
	  && (r->r_flags & RLD_RESOLVE_BY_NAME))) {
	sym->s_resolved = sym;
	return sym;			/* Symbol resolves to self */
    }

    /* Symbol is eligible for TOC resolution */
    sym->s_flags |= S_ISTOC;

    /* If this is the first eligible symbol with this name for TOC
       resolution, then it resolves to itself. */
    if (!(sym->s_name->flags & STR_TOC_RESOLVED)) {
	/* No TOC resolution has been done yet for a symbol with this name. */
	sym->s_name->flags |= STR_TOC_RESOLVED;
	sym->s_resolved = sym;
	return sym;			/* Symbol resolves to self */
    }

    /* We've already done some TOC resolution for this symbol name, so we
       must search through all the other hidden symbols with this name to see
       if we find one that the current symbol should be combined with. */
    for (sym2 = sym->s_name->first_hid_sym; sym2; sym2 = sym2->s_synonym) {
	if (sym == sym2 || !(sym2->s_flags & S_ISTOC))
	    continue;

	/* NOTE:  sym2->s_flags & S_ISTOC =>
	   sym2->s_csect->c_first_rld is the one and only RLD for sym2,
	   and this RLD refers to an external name. */
	if (r->r_sym->s_name != sym2->s_csect->c_first_rld->r_sym->s_name)
	    continue;			/* The two TOC symbol refer to
					   symbols with different names. */

	/* We found a second TOC entry that the current should be combined
	   with.  The exception now is if the offsets to the referenced
	   symbol are different. */
	if (Switches.dbg_opt4)
	    if (dbg_opt4(sym, sym2, r->r_sym->s_name) == 0)
		continue; /* Values of referred-to fields are different--
			     don't combine after all. */

	/* We must call resolve_symbol to mark the symbols referenced by
	   "sym" as VISITED, to allow length and alignments to be checked
	   for BSS symbols, and to allow type-checking strings to be
	   inherited for all symbols.  (The returned symbol is ignored.) */
	(void) resolve_symbol(r->r_sym, ERR_UNRESOLVED);

	sym->s_resolved = sym2->s_resolved;

	if (Switches.dbg_opt8 /* Exhaustive resolution output */
	    || Switches.dbg_opt10) { /* Verbose TOC resolve */
	    add_to_resolve_names(sym->s_name, ERR_TOC_RESOLVE);
	}
	return sym->s_resolved;
    }
    /* No matching TOC entry found for symbol, so it's in a
       new equivalence class */
    sym->s_resolved = sym; /* resolve to self */
    return sym;
} /* resolve_hidden_symbol */
/***************************************************************************
 * Name:	display_symbol
 *
 * Purpose:	Print symbolic information for a symbol
 *
 * Results:
 * *************************************************************************/
void
display_symbol(SYMBOL *s,
	       int replacement_type)	/* 1 from TOC combination
					   2 from glue code. */
{
    int flags = MINIDUMP_SOURCE_INFO;

    if (Switches.verbose) {
	flags |= ((imported_symbol(s) ? MINIDUMP_LONG_INPNDX : MINIDUMP_INPNDX)
		  | MINIDUMP_SYMNUM_DBOPT11
		  | MINIDUMP_ADDRESS
		  | MINIDUMP_TYPE
		  | MINIDUMP_SMCLASS
		  | MINIDUMP_LEN_ALIGN);

	if (replacement_type == 1)
	    flags &= ~(MINIDUMP_SMCLASS | MINIDUMP_LEN_ALIGN);
    }
    minidump_symbol(s, 0 /* Suppress name */, flags, NULL);
}
/***********************************************************************
 * Name:	resolve_symbol()
 *
 * Purpose:	Resolve a symbol by finding a unique representative for
 *		"similar" symbols.  Hidden symbols (C_HIDEXT) are resolved
 *		with the routine resolve_hidden_symbol() (see above).  Global
 *		symbols (C_EXT) are resolved by name.  A unique symbol with the
 *		given name is chosen as the representative for all symbols
 *		with that name.  Ordinarily, the first global definition is
 *		used.
 *
 * Results:	The designated symbol if moved to the head of the list of
 *		global symbols with the given name, and the STR_RESOLVED
 *		flag is set in the STR structure.
 * ***********************************/
static SYMBOL *
resolve_symbol(SYMBOL *sym,
	       int ref_type_flags)	/* Kind of reference:  ERR_KEEP,
					   ERR_EXPORT, or ERR_UNRESOLVED */
{
    int		num_replacements;
    int		silent_replacements, not_silent;
    int		symbol_from_rebind = 0;
    int		CM_max_len, CM_max_align;
    STR		*name;
    SYMBOL	*cur_sym, *next_sym, *keep_sym, *temp_sym;

    name = sym->s_name;

    switch(name->flags & (STR_RESOLVED | STR_NO_DEF)) {
      case 0:
	break;

      case STR_RESOLVED:
	if (sym->s_flags & S_VISITED)
	    return name->first_ext_sym;

	/* We have our first reference to a symbol that was not chosen as
	   the unique representative for "name"-named symbols.
	   Nevertheless, the s_typechk field might be needed, and we might
	   need to check sizes and alignments for CM symbols, so mark the
	   symbol as visited, and go check some things. */
	sym->s_flags |= S_VISITED;
	keep_sym = name->first_ext_sym;
	goto check_stuff;

      case STR_RESOLVED | STR_NO_DEF:
	if ((ref_type_flags & ERR_UNRESOLVED) && name->first_ext_sym)
	    delcsect_message_needed = 1;
	add_to_resolve_names(name, ref_type_flags);
	/* The symbol must be an XTY_ER or a deleted symbol. */
	sym->s_flags |= S_VISITED;
	return NULL;

      case STR_NO_DEF:
	internal_error();
    }

    name->flags |= STR_RESOLVED;

    /* Check all global symbols with name "name" */
   for (cur_sym = name->first_ext_sym; cur_sym; cur_sym = cur_sym->s_synonym){
	/* If first external symbol is from a global symbol table,
	   we read the member to continue resolution */
	if (cur_sym->s_smtype == XTY_AR) {
	    temp_sym = read_archive_member_for_symbol(cur_sym);
	    if (cur_sym == sym)
		sym = temp_sym;
	    cur_sym = temp_sym;
	    /* Make sure the symbol was overwritten with its real definition.
	       If it wasn't, an error message will have already been printed,
	       so we go on with the next like-named symbol. */
	    if (cur_sym->s_smtype == XTY_AR)
		continue;
	}
	/* See if symbol was not deleted by "delcsect" processing,
	   we've found the first good symbol. */
	if (!(cur_sym->s_flags & S_DUPLICATE))
	    break;
    }

    /* We found 1st "real" instance of a "name" symbol */
    if (cur_sym == NULL) {
	sym->s_flags |= S_VISITED;
	name->flags |= STR_NO_DEF; /* Symbol was never defined or all
				      definitions are duplicates or bad symbols
				      from the global symbol table of an
				      archive. */
	if ((ref_type_flags & ERR_UNRESOLVED) && name->first_ext_sym)
	    delcsect_message_needed = 1;
	add_to_resolve_names(name, ref_type_flags);
	return NULL;
    }

    num_replacements = 0;
    silent_replacements = 0;
    keep_sym = cur_sym;
    if (keep_sym->s_smtype == XTY_CM) {
	CM_max_align = keep_sym->s_csect->c_align;
	CM_max_len = keep_sym->s_csect->c_len;
    }
    else {
	CM_max_align = 0;
	CM_max_len = 0;
    }

    /* Compare first symbol in list against all remaining symbols.  By default,
       we keep the first non-deleted symbol in the list. */
    for (next_sym = cur_sym->s_synonym;
	 next_sym;
	 next_sym = next_sym->s_synonym) {
	if (next_sym->s_smtype == XTY_AR) {
	    temp_sym = read_archive_member_for_symbol(next_sym);
	    if (next_sym == sym)
		sym = temp_sym;
	    next_sym = temp_sym;
	    if (next_sym->s_smtype == XTY_AR)
		continue;
	}
	if (next_sym->s_flags & S_DUPLICATE) /* Symbol was deleted */
	    continue;

	num_replacements++;		/* Count number of additional instances
					   of viable symbols with this name. */

	/* Check to see if replacement symbol is from rebind. */
	if ((Bind_state.state & STATE_REBIND_USED)
	    && next_sym->s_csect->c_srcfile->sf_object->o_ifile->i_rebind)
	    symbol_from_rebind = 1;
	else
	    symbol_from_rebind = 0;

	if (compare_hash(next_sym->s_typechk, keep_sym->s_typechk)) {
	    /* Mismatch; don't coalesce. */
	    /* We always report a type-checking error if Switches.dbg_opt8
	       is set.

	       Otherwise, we do not report an error if:
	       1) The duplicate symbol is from the rebound file; OR
	       2) The duplicate symbol is from an archive member; OR
	       3) Either symbol is glue code.  If we have mismatched glue code,
		  we'll have mismatched descriptors as well.

	       Note:  Even if an error is suppressed here, it will be
	       reported if the duplicate symbol is referenced.
	    */
	    if ((next_sym->s_csect->c_symbol.s_smclass == XMC_GL
		 || keep_sym->s_csect->c_symbol.s_smclass == XMC_GL
		 || symbol_from_rebind == 1
		 || next_sym->s_flags & S_ARCHIVE_SYMBOL)
		&& !Switches.dbg_opt8) {
		silent_replacements++;
		next_sym->s_number = -next_sym->s_number;
	    }
	    else
		add_to_resolve_names(sym->s_name, ERR_TYPECHK);

	    /* A typecheck error keeps us from doing any special case
	       such as combining CM symbols. */
	}
	else {
	    /************************************************
	      Determine which SYMBOL is to be kept:
	      Usually, the first entry (keep_sym) to kept.
	      Exceptions:
	      	  o SD/RW is preferred to CM.
	          o GL code is preferred to generated import symbols (to avoid
		    having to add the glue code for the symbol later.  The
		    duplicate symbol message is suppressed in this case.
		  o Other non-GL's are preferred to GL's.
	      Note:  We have 5 choices here for each symbol--SD, LD, CM, IF,
	      and IS--or 25 total combinations.
	     ************************************************/
	    not_silent = 0;
#define pair(x,y) (256*x+y)
	    switch(pair(keep_sym->s_smtype, next_sym->s_smtype)) {
	      case pair(XTY_SD, XTY_LD):
	      case pair(XTY_SD, XTY_SD):
	      case pair(XTY_LD, XTY_LD):
	      case pair(XTY_LD, XTY_SD):
		if (next_sym->s_smclass == XMC_GL &&
		    keep_sym->s_smclass == XMC_PR) {
		    /* Regular code replaces glue code silently. */
		    if (next_sym->s_number > 0) {
			silent_replacements++;
			next_sym->s_number = -next_sym->s_number;
		    }
		}
		else if (keep_sym->s_smclass == XMC_GL &&
			 next_sym->s_smclass == XMC_PR) {
		    /* Regular code replaces glue code silently. */
		    if (keep_sym->s_number > 0) {
			silent_replacements++;
			keep_sym->s_number = -keep_sym->s_number;
		    }
		    keep_sym = next_sym;
		}
		/* Second symbol is replaced. */
		break;

	      case pair(XTY_IS, XTY_LD):
	      case pair(XTY_IS, XTY_SD):
		if (next_sym->s_smclass == XMC_GL
		    && keep_sym->s_smclass == XMC_PR
		    && keep_sym->s_inpndx == INPNDX_GENERATED) {
		    /* Keep glue code instead of generated import symbol. */
		    if (keep_sym->s_number > 0) {
			silent_replacements++;
			keep_sym->s_number = -keep_sym->s_number;
		    }
		    keep_sym = next_sym;
		}
		break;

	      case pair(XTY_SD, XTY_IS):
	      case pair(XTY_LD, XTY_IS):
		if (keep_sym->s_smclass == XMC_GL
		    && next_sym->s_smclass == XMC_PR
		    && next_sym->s_inpndx == INPNDX_GENERATED) {
		    /* Keep glue code instead of generated import symbol. */
		    if (next_sym->s_number > 0) {
			silent_replacements++;
			next_sym->s_number = -next_sym->s_number;
		    }
		}
		break;

	      case pair(XTY_IF, XTY_LD):
	      case pair(XTY_IF, XTY_SD):
		if (next_sym->s_smclass == XMC_GL &&
		    (keep_sym->s_smclass == XMC_PR
		     || keep_sym->s_smclass == XMC_UA)
		    && keep_sym->s_inpndx == INPNDX_GENERATED) {
		    /* Keep glue instead of generated import symbol. */
		    if (keep_sym->s_number > 0) {
			silent_replacements++;
			keep_sym->s_number = -keep_sym->s_number;
		    }
		    keep_sym = next_sym;
		}
		break;

	      case pair(XTY_LD, XTY_IF):
	      case pair(XTY_SD, XTY_IF):
		if (keep_sym->s_smclass == XMC_GL &&
		    (next_sym->s_smclass == XMC_PR
		     || next_sym->s_smclass == XMC_UA)
		    && next_sym->s_inpndx == INPNDX_GENERATED) {
		    /* Keep glue code instead of generated import symbol. */
		    if (next_sym->s_number > 0) {
			silent_replacements++;
			next_sym->s_number = -next_sym->s_number;
		    }
		}
		break;

	      case pair(XTY_CM, XTY_IS):
		if (next_sym->s_smclass == XMC_DS)
		    not_silent = 1;	/* Report replacement even if
					   next_sym is from an archive. */
		break;

	      case pair(XTY_IS, XTY_CM):
		if (keep_sym->s_smclass == XMC_DS)
		    not_silent = 1;	/* Report replacement even if
					   next_sym is from an archive. */
		break;

	      case pair(XTY_CM, XTY_IF):
	      case pair(XTY_IF, XTY_CM):
		/* We can't check length and alignment.  Allow replacement
		   to take place with a message. */
		break;

	      case pair(XTY_CM, XTY_LD):
		not_silent = 1;
		/* This case is not silent, because it is probably an error.
		   Therefore, we don't have to do any length or alignment
		   checks.  They will be done when messages are printed. */
		/* Don't allow descriptors to replace CM symbols. */
		if (next_sym->s_smclass == XMC_DS)
		    break;

		/* Keep the LD. */
		keep_sym = next_sym;
		break;

	      case pair(XTY_LD, XTY_CM):
		/* This case is not silent--this may be an error.  Therefore,
		   we don't have to do any length or alignment checks.  They
		   will be done when messages are printed. */
		not_silent = 1;
		break;

	      case pair(XTY_CM, XTY_SD):
		/* Do not allow descriptors to replace CM symbols. */
		if (next_sym->s_smclass == XMC_DS) {
		    not_silent = 1;
		    break;
		}

		/* CM(s) replaced by SD.  This replacement is silent unless
		   the CM is longer or more strictly alignment than the SD. */
		if (CM_max_len <= next_sym->s_csect->c_len
		    && CM_max_align <= next_sym->s_csect->c_align) {
		    if (keep_sym->s_number > 0) {
			silent_replacements++;
			keep_sym->s_number = -keep_sym->s_number;
		    }
		}
		else
		    not_silent = 1;
		keep_sym = next_sym;
		break;

	      case pair(XTY_SD, XTY_CM):
		/* This replacement is silent unless the SD is a descriptor or
		   the CM is longer or more strictly aligned than the SD. */
		if (keep_sym->s_smclass != XMC_DS
		    && next_sym->s_csect->c_len <= keep_sym->s_csect->c_len
		    && next_sym->s_csect->c_align <= keep_sym->s_csect->c_align
		    ) {
		    if (next_sym->s_number > 0) {
			silent_replacements++;
			next_sym->s_number = -next_sym->s_number;
		    }
		}
		else
		    not_silent = 1;
		break;

	      case pair(XTY_CM, XTY_CM):
		/* Keep the first CM, but keep track of the maximum length
		   and alignment of all CM symbols.  These maximum values
		   will only be used if a CM symbol is the one that is kept. */
		if (next_sym->s_number > 0) {
		    silent_replacements++;
		    next_sym->s_number = -next_sym->s_number;
		}
		CM_max_len = max(CM_max_len, next_sym->s_csect->c_len);
		CM_max_align = max(CM_max_align, next_sym->s_csect->c_align);
		/* Symbols will be coalesced */
		break;

	      case pair(XTY_IS, XTY_IS):
	      case pair(XTY_IS, XTY_IF):
	      case pair(XTY_IF, XTY_IS):
	      case pair(XTY_IF, XTY_IF):
		break;
	    } /* switch */
	    if (keep_sym != next_sym) {
		if (next_sym->s_number > 0) {
		    if (!not_silent && (next_sym->s_flags & S_ARCHIVE_SYMBOL
					|| symbol_from_rebind)) {
			silent_replacements++;
			next_sym->s_number = -next_sym->s_number;
		    }
		}
	    }
	}
    }

    /* Move the winning symbol to the head of the list */
    if (keep_sym != name->first_ext_sym) {
	DEBUG_MSG(RESOLVE_DEBUG,
		  (SAY_NO_NLS, "First_ext_sym not kept for %s", name->name));
	next_sym = keep_sym->s_synonym;
	keep_sym->s_synonym = name->first_ext_sym;
	for (cur_sym = name->first_ext_sym;
	     cur_sym->s_synonym != keep_sym;
	     cur_sym = cur_sym->s_synonym)
	    /* skip */;
	cur_sym->s_synonym = next_sym;
	name->first_ext_sym = keep_sym;
    }
    if (imported_symbol(keep_sym))
	name->flags |= STR_IMPORT;

    if (num_replacements > 0)
	if (Switches.verbose
	    || Switches.dbg_opt8
	    || num_replacements - silent_replacements > 0) {
	    add_to_resolve_names(sym->s_name, ERR_DUPLICATE);
	}
	else {
	    if (silent_replacements > 0) {
		/* reset s_number fields */
		for (cur_sym = keep_sym->s_synonym;
		     cur_sym;
		     cur_sym = cur_sym->s_synonym)
		    if (cur_sym->s_number < 0)
			cur_sym->s_number = -cur_sym->s_number;
	    }
	}

    if (keep_sym->s_smtype == XTY_CM) {
	/* BSS symbol is being kept.  Set its length and alignment to the
	   maxima of all BSS symbols. */
	if (CM_max_len > keep_sym->s_csect->c_len) {
	    if (Switches.verbose) {
		/* display_resolve_errors() will update length */
		add_to_resolve_names(sym->s_name, ERR_CM_BUMP);
	    }
	    else
		keep_sym->s_csect->c_len = CM_max_len;
	}
	if (CM_max_align > keep_sym->s_csect->c_align) {
	    if (Switches.verbose) {
		/* display_resolve_errors() will update length */
		add_to_resolve_names(sym->s_name, ERR_ALIGN_BUMP);
	    }
	    else
		keep_sym->s_csect->c_align = CM_max_align;
	}
    }

  check_stuff:
    if (sym != keep_sym) {
	sym->s_flags |= S_VISITED;

	/* Check any referenced symbol against the kept symbol.  All the
	   typechecks must match or we must issue a message. */
	if (keep_sym->s_typechk == NULL) {
	    if (sym->s_typechk != NULL) {
		keep_sym->s_typechk = sym->s_typechk;
		keep_sym->s_flags |= S_TYPECHK_IMPLIED;
	    }
	}
	else if (keep_sym->s_flags & S_TYPECHK_IMPLIED) {
	    if (compare_hash(sym->s_typechk, keep_sym->s_typechk) != 0)
		add_to_resolve_names(sym->s_name, ERR_TYPECHK);
	}
    }
    return keep_sym;
} /* resolve_symbol */
/***************************************************************************
 * Name:	UA_func_ref
 *
 * Purpose:	If 'plain_name' is referenced as a DS or '.plain_name' is
 *		referenced as a PR, return 1.  Otherwise, return 0.
***************************************************************************/
static int
UA_func_ref(STR *plain_name)
{
    SYMBOL *sym;

    for (sym = plain_name->first_ext_sym; sym; sym = sym->s_synonym)
	if (!(sym->s_flags & S_DUPLICATE)
	    && sym->s_smclass == XMC_DS)
	    return 1;
    for (sym = plain_name->refs; sym; sym = sym->s_synonym)
	if (sym->s_smclass == XMC_DS)
	    return 1;
    if (plain_name->alternate == NULL)
	return 0;
    for (sym = plain_name->alternate->first_ext_sym; sym; sym = sym->s_synonym)
	if (!(sym->s_flags & S_DUPLICATE)
	    && sym->s_smclass == XMC_PR)
	    return 1;
    for (sym = plain_name->alternate->refs; sym; sym = sym->s_synonym)
	if (sym->s_smclass == XMC_PR)
	    return 1;
    return 0;
}
/***************************************************************************
 * Name:	do_plain_name
 *
 * Purpose:	For the given plain_sym, check whether a corresponding
 *		symbol (with a .name) should be generated.
 *		We generate .name if
 *		a) name is imported from an import file (its smclass will
 *			be XMC_UA or XMC_XO)
 *		b) name is imported from a shared object and its storage-
 *			mapping class is XMC_DS or XMC_UA or XMC_XO.
 *
 *		We do not have to generate .name if name is a descriptor
 *			from an XCOFF file, because the code corresponding to
 *			the descriptor must be from the same object file.
 *
 *		NOTE: If name is from an archive member, we don't know its
 *			storage-mapping class.  If it turns out to be XMC_XO,
 *			the corresponding .name will never be generated.
 *
 * Results:	The generated symbol, if one if generated.
 *		prev_dot_sym, otherwise.
 * *************************************************************************/
static SYMBOL *
do_plain_name(STR **dot_name,		/* Pointer to dot_name, for generated
					   symbol, if needed. */
	      SYMBOL *prev_dot_sym,	/* Predecessor of new symbol */
	      SYMBOL *plain_sym)	/* Existing plain symbol */
{
    if (imported_symbol(plain_sym)) {
	switch(plain_sym->s_smclass) {
	  case XMC_UA:
	    if (!UA_func_ref(plain_sym->s_name))
		break;
	    /* else fall through */
	  case XMC_XO:
	  case XMC_DS:
	    if (*dot_name == NULL)
		*dot_name = putstring(&plain_sym->s_name->name[-1]);
	    return create_generated_import_SYMBOL((*dot_name), prev_dot_sym,
						  plain_sym);
	}
    }
    return prev_dot_sym;
}
/***************************************************************************
 * Name:	do_dot_name
 *
 * Purpose:	For the given dot_sym, check whether a corresponding
 *		symbol (with a plain name) should be generated.
 *
 *		A symbol is generated if the dot symbol has smclass:
 *		XMC_PR:	(the new symbol will be a descriptor)
 *		XMC_XO: (the new symbol will be a generated import)
 *		XMC_UA and the symbol is imported: (the new symbol will be
 *						a generated import)
 *
 * Results:	The generated symbol, if one is generated.
 *		prev_plain_sym, otherwise.
 * *************************************************************************/
static SYMBOL *
do_dot_name(STR *plain_name,		/* Plain_name, for generated symbol,
					   if needed. */
	    SYMBOL *prev_plain_sym,	/* Predecessor of new symbol */
	    SYMBOL *dot_sym,		/* Existing dot symbol */
	    OBJECT *dot_obj)		/* dot_sym's  object */
{
    switch(dot_sym->s_smclass) {
      case XMC_PR:
	return create_descriptor(plain_name, prev_plain_sym, dot_sym, dot_obj);
      case XMC_XO:
	return create_generated_import_SYMBOL(plain_name, prev_plain_sym,
					      dot_sym);
      case XMC_UA:
	if (imported_symbol(dot_sym)
	    && UA_func_ref(plain_name))
	    return create_generated_import_SYMBOL(plain_name,
						  prev_plain_sym, dot_sym);
	break;
    }
    return prev_plain_sym;
}
/***************************************************************************
 * Name:	resolve_name
 *
 * Purpose:	Resolve a symbol by name.
 *
 *		Because of the correspondence between (XMC_PR, .foo) and
 *		(XMC_DS, foo), we generate symbols the first time resolve_name
 *		is called for either .foo or foo.
 *
 *		Afterwards, we simply call resolve_symbol().
 *
 * Inputs:	STR *s: The name of the symbol
 *		default_symbol:  The referenced symbol if the symbol is
 *			defined in the same file as the reference.
 * Results:
 * *************************************************************************/
static SYMBOL *
resolve_name(STR *s,			/* Name of symbol to resolve */
	     SYMBOL *default_symbol,	/* Symbol being visited, if RLD
					   is not to an ER. */
	     int ref_type_flags)	/* Kind of reference:  ERR_KEEP,
					   ERR_EXPORT, or ERR_UNRESOLVED */
{
    STR		*plain_name, *dot_name;
    SYMBOL	*plain_sym, *dot_sym;
    SYMBOL	*prev_plain_sym, *prev_dot_sym;
    OBJECT	*plain_obj, *dot_obj;

#define advance_sym(s,o) (o=((s=s->s_synonym)==NULL)?NULL\
    :(s->s_smtype==XTY_AR)?s->s_object:s->s_csect->c_srcfile->sf_object,s)

#ifdef DEBUG
    if (bind_debug & RESOLVE_DEBUG) {
	if (bind_debug & DEBUG_LONG) {
	    int i;
	    for (i = dfs_depth; i > 0; i--)
		say(SAY_NO_NLS | SAY_NO_NL, ".");
	}
	say(SAY_NO_NLS, "Calling resolve_name(%s)", s->name);
    }
#endif

    if (s->flags & STR_RESOLVED)
	goto simple_resolve;		/* We only resolve a name once. */

    if (s->name[0] == '.' && s->name[1] != '.') {
	dot_name = s;
	plain_name = s->alternate;	/* s->alternate must exist */
	if (plain_name->flags & STR_RESOLVED)
	    goto simple_resolve;
    }
    else {
	plain_name = s;
	if (s->alternate == NULL)
	    dot_name = NULL;
	else {
	    dot_name = s->alternate;
	    if (dot_name->flags & STR_RESOLVED)
		goto simple_resolve;
	}
    }

    /* For each imported symbol foo that is a descriptor or has an unknown
       storage-mapping class, add an implicit symbol .foo as a placeholder
       for possible glue code.

       If .foo comes from an archive and foo is not also from the
       same archive member, we must read the archive member to see whether
       .foo has storage-mapping class XMC_PR.  We don't need to do this
       for foo, because if foo is a descriptor, .foo must be in the
       same object or the naming convention hasn't been followed.

       Note:  If foo and .foo are both imported at fixed addresses, we won't
       need to generate a symbol.

       For each .foo with storage-mapping class XMC_PR, generate a
       descriptor foo is a symbol from the same object does not already
       exist.
     */

    /* Walk both symbol chains */

    /* Initialize plain_name variables */
    prev_plain_sym = NULL;
    plain_sym = plain_name->first_ext_sym;
    if (plain_sym == NULL)
	plain_obj = NULL;
    else
	plain_obj = plain_sym->s_smtype == XTY_AR
	    ? plain_sym->s_object : plain_sym->s_csect->c_srcfile->sf_object;

    /* Initialize dot_name variables */
    prev_dot_sym = NULL;
    if (dot_name == NULL || (dot_sym = dot_name->first_ext_sym) == NULL) {
	dot_sym = NULL;
	dot_obj = NULL;
    }
    else
	dot_obj = dot_sym->s_smtype == XTY_AR
	    ? dot_sym->s_object : dot_sym->s_csect->c_srcfile->sf_object;

    while (dot_obj && plain_obj) {
	if (dot_obj == plain_obj) {
	    /* Duplicates symbols from the same object.  If both symbols are
	       imports and they are not both XO items, it is not clear what
	       to do. */
	    prev_plain_sym = plain_sym;
	    plain_sym = advance_sym(plain_sym, plain_obj);
	    prev_dot_sym = dot_sym;
	    dot_sym = advance_sym(dot_sym, dot_obj);
	}
	else {
	    /* Now we read the archive member if .name is from an archive. */
	    if (dot_sym->s_smtype == XTY_AR) {
		dot_sym = read_archive_member_for_symbol(dot_sym);
		if (dot_sym->s_smtype != XTY_AR)
		    dot_obj = dot_sym->s_csect->c_srcfile->sf_object;
	    }

	    if (dot_obj->o_ifile->i_ordinal == plain_obj->o_ifile->i_ordinal) {
		if (dot_obj < plain_obj)
		    goto dot_name_first;
		else
		    goto plain_name_first;
	    }
	    else if (dot_obj->o_ifile->i_ordinal
		     < plain_obj->o_ifile->i_ordinal) {
	      dot_name_first:
		/* dot_name has appeared first in list */
		if (!(dot_sym->s_flags & S_DUPLICATE))
		    prev_plain_sym = do_dot_name(plain_name, prev_plain_sym,
						 dot_sym, dot_obj);
		prev_dot_sym = dot_sym;
		dot_sym = advance_sym(dot_sym, dot_obj);
	    }
	    else {
	      plain_name_first:
		/* Plain name has appeared first in list */
		if (!(plain_sym->s_flags & S_DUPLICATE))
		    do_plain_name(&dot_name, prev_dot_sym, plain_sym);
		prev_plain_sym = plain_sym;
		plain_sym = advance_sym(plain_sym, plain_obj);
	    }
	}
    }
    while (dot_obj) {
	/* Only dot_names left */
	/* Now we read the archive member if .name is from an archive. */
	if (dot_sym->s_smtype == XTY_AR) {
	    dot_sym = read_archive_member_for_symbol(dot_sym);
	    if (dot_sym->s_smtype != XTY_AR)
		dot_obj = dot_sym->s_csect->c_srcfile->sf_object;
	}

	if (!(dot_sym->s_flags & S_DUPLICATE))
	    prev_plain_sym = do_dot_name(plain_name, prev_plain_sym,
					 dot_sym, dot_obj);
	dot_sym = advance_sym(dot_sym, dot_obj);
    }

    while (plain_obj) {
	/* Only plain names left */
	if (!(plain_sym->s_flags & S_DUPLICATE)) {
	    prev_dot_sym = do_plain_name(&dot_name, prev_dot_sym, plain_sym);
	}
	plain_sym = advance_sym(plain_sym, plain_obj);
    }

    /* Now we have generated all possible symbols, so we can call
       resolve_symbol(). */
  simple_resolve:
    if (default_symbol)
	return resolve_symbol(default_symbol, ref_type_flags);
    else if (s->first_ext_sym)
	return resolve_symbol(s->first_ext_sym, ref_type_flags);
    else {
	/* Set resolved flag and flag to indicate no definition */
	s->flags |= (STR_RESOLVED | STR_NO_DEF);
	add_to_resolve_names(s, ref_type_flags);
	return NULL;
    }
} /* resolve_name */
/***************************************************************************
 * Name:	get_toc_anchor
 *
 * Purpose:	Return any TOC anchor in the object if no TOC anchors have been
 *		referenced.  Return NULL if some anchor has been referenced.
 *
 * Results:	If the object file contains no TOC anchors, an error message
 *		is issued and NULL is returned.
 * *************************************************************************/
static SYMBOL *
get_toc_anchor(OBJECT *obj,
	       SYMBOL *sym)		/* For error messages */
{
    int		sn, i;
    SYMBOL	*temp_sym = NULL;

    for (i = 0; i < obj->oi_num_sections; i++) {
	if (obj->oi_section_info[i].sect_flags & SECT_TOC_REFERENCED)
	    return NULL;		/* One TOC already referenced */
	if (temp_sym == NULL && obj->oi_section_info[i].l_toc_anchor != NULL) {
	    sn = i;
	    temp_sym = obj->oi_section_info[i].l_toc_anchor; /* Save 1st */
	}
    }
    if (temp_sym == NULL) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(NO_TOC_ANCHOR2,
 "%1$s: 0711-337 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tThere is a TOC-relative reference to the symbol,\n"
 "\tbut the object contains no TOC anchor."),
		 Main_command_name,
		 sym->s_name->name,
		 sym->s_inpndx,
		 get_object_file_name(obj));
    }
    else
	obj->oi_section_info[sn].sect_flags |= SECT_TOC_REFERENCED;
    return temp_sym;
}
/***************************************************************************
 * Name:	dfs
 *
 * Purpose:	Recursively visit all symbols reachable from 'sym', marking
 *		them so that they will be saved.
 *
 *		The RLDs of sym's csect are used to determine which symbols
 *		are reachable from sym.  In addition, if 'sym' makes a TOC-
 *		relative reference, an implicit reference to a TOC anchor
 *		is made.
 *
 * Results:	Visited SYMBOLs have their S_MARK and S_VISITED flags set.
 *		Visited CSECTs have c_mark set to 1.
 * *************************************************************************/
void
dfs(SYMBOL *sym)
{
    int		toc_anchor_reference, tocdata_reference;
    CSECT	*cs;			/* Csect containing symbol */
    RLD		*r;
    SYMBOL	*toc_sym;
#ifdef DEBUG
    SYMBOL	*save_sym = sym;	/* Save original parameter for dbx */
#endif

    if (sym->s_flags & S_MARK)
	return;
    sym->s_flags |= (S_MARK | S_VISITED); /* Mark symbol--this is the only
					     place a symbol gets marked. */
    kept_symbols++;

#ifdef DEBUG
    if (bind_debug & RESOLVE_DEBUG) {
	if (bind_debug & DEBUG_LONG) {
	    int i;
	    for (i = dfs_depth; i > 0; i--)
		say(SAY_NO_NLS | SAY_NO_NL, ".");
	}
	else
	    say(SAY_NO_NLS | SAY_NO_NL, "Calling ");
	show_inpndx(sym, "dfs([%s]");
	say(SAY_NO_NLS, "%s%s%s{0x%x})",
	    (sym->s_flags & S_HIDEXT) ? "<" : "",
	    sym->s_name->name,
	    (sym->s_flags & S_HIDEXT) ? ">" : "",
	    sym);
    }
#endif

    switch(sym->s_smtype) {
      case XTY_AR:		/* Should never happen */
      case XTY_ER:		/* Should never happen */
	internal_error();
	return;
      case XTY_IS:
      case XTY_IF:
	sym->s_csect->c_mark = 1; /* Mark csect containing symbol, so that
				     the shared object for the csect will
				     be included in list of import files. The
				     csect is actually a dummy, used to point
				     to all symbols in the shared object. */
	if (sym->s_inpndx == INPNDX_GENERATED) {
#ifdef DEBUG
	    ++dfs_depth;
#endif
	    sym = resolve_name(sym->s_name->alternate, NULL, 0);
	    if (sym)
		dfs(sym);
#ifdef DEBUG
	    --dfs_depth;
#endif
	}
	return;
    }

    cs = sym->s_csect;
    if (cs->c_mark)		/* Csect already visited? */
	return;
    cs->c_mark = 1;		/* Mark csect as visited. This is the only
				   place a csect is marked (except for the
				   dummy csect for an imported symbol, which
				   is marked above). */
#ifdef DEBUG
    dfs_depth++;
#endif

    /* Visit all symbols referenced by the RLDs in the csect. */
    for (r = cs->c_first_rld; r; r = r->r_next) {
	/* Multiple RLDs to the same symbol only have to be handled once,
	   except when the reference is TOC-relative. A TOC-relative reference
	   implies a reference to a TOC anchor, and there is no way to tell
	   whether the RLD that previously visited the referenced symbol was
	   a TOC-relative reference.  In the case of data-in-TOC, we also may
	   need to worry about fixup code or mapping the resolved symbol into
	   the TOC. */

	const int visited_bit = r->r_sym->s_flags & S_VISITED; /* Save bit */

	if (r->r_reltype == R_TOC
	    || r->r_reltype == R_TRL
	    || r->r_reltype == R_TRLA) {
	    /* Set flag to speed up later testing for TOC relocation types. */
	    r->r_flags |= RLD_TOC_RELATIVE | RLD_OVERFLOW_FIXUP_OK;
	    toc_anchor_reference = 1;

	    switch(r->r_sym->s_smclass) {
	      case XMC_TD:
		if (r->r_flags & RLD_RESOLVE_BY_NAME)
		    tocdata_reference = 1;
		break;
	      case XMC_TC:
		tocdata_reference = 0;
		break;
	      default:
		tocdata_reference = 0;
		/* We won't generate fixup code for this reference. */
		r->r_flags &= ~RLD_OVERFLOW_FIXUP_OK;
		if (r->r_flags & RLD_RESOLVE_BY_NAME)
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_EXT_TOCREF,
 "%1$s: 0711-338 SEVERE ERROR: Symbol %2$s in object %3$s:\n"
 "\tThere is a TOC-relative reference to this global symbol\n"
 "\tbut its storage-mapping class is not XMC_TD or XMC_TC."),
			     Main_command_name,
			     r->r_sym->s_name->name,
			     (r->r_flags & RLD_EXT_REF)
			     ? get_object_file_name(r->r_sym->er_object)
			     : get_object_file_name(r->r_sym->s_csect
						    ->c_srcfile->sf_object));
		else
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_HIDEXT_TOC_REF,
 "%1$s: 0711-339 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tThere is a TOC-relative reference to this internal symbol,\n"
 "\tbut its storage-mapping class is not XMC_TD or XMC_TC."),
			     Main_command_name,
			     r->r_sym->s_name->name,
			     r->r_sym->s_inpndx,
			     get_object_file_name(r->r_sym->er_object));
	    }
	}
	else {
	    tocdata_reference = 0;
	    toc_anchor_reference = 0;
	}

	if (!(r->r_flags & RLD_RESOLVE_BY_NAME)) {
	    /* Reference to C_HIDEXT symbol.
	       This can never be a data-in-TOC reference, but we might need
	       to make an implicit reference to a TOC anchor. */
	    if (!visited_bit)
		sym = resolve_hidden_symbol(r->r_sym);
	    else if (toc_anchor_reference)
		sym = NULL;		/* No need to resolve symbol again,
					   but we fall through to handle the
					   implicit TOC anchor reference. */
	    else
		continue;
	}
	else {
	    /* NOTE:	tocdata_reference => toc_anchor_reference, so
	       		~toc_anchor_reference => ~tocdata_reference.  We can
			only have toc_anchor_reference & ~tocdata_reference if
			the referenced symbol is a global XMC_TC or the input
			file was bad. */
	    if (visited_bit && !toc_anchor_reference)
		continue;

	    if (r->r_flags & RLD_EXT_REF)
		sym = resolve_name(r->r_sym->s_name, NULL, ERR_UNRESOLVED);
	    else
		sym = resolve_name(r->r_sym->s_name, r->r_sym, ERR_UNRESOLVED);

	    if (tocdata_reference) {
		if (sym) {
		    if (imported_symbol(sym)) {
			DEBUG_MSG(RESOLVE_DEBUG,
				  (SAY_NO_NLS,
		   "TD reference to imported symbol [%d]%s; Fixup code needed",
				   sym->s_number, sym->s_name->name));
			/* A single CSECT structure exists for all symbols in an
			   import file.  Therefore, we must mark the symbol so
			   that references to it can be fixed up. The symbol is
			   marked by modifying its s_inpndx field. */
#ifdef DEBUG
			if (!(sym->s_inpndx == INPNDX_IMPORT
			      || (sym->s_inpndx == INPNDX_IMPORT_TD
				  && visited_bit)))
			    internal_error();
#endif
			sym->s_csect->c_TD_ref = 1; /* Mark referenced csect
						       so it will be mapped
						       into the TOC. */
			sym->s_inpndx = INPNDX_IMPORT_TD;
			r->r_flags |= RLD_TOCDATA_FIXUP;
			++Bind_state.num_data_in_toc_fixups;
			sym = find_or_create_toc_entry(r, cs, visited_bit);
			if (sym == NULL)	/* Previous toc symbol found. */
			    continue;
		    }
		    else if (!(sym->s_flags & S_PRIMARY_LABEL)
			     || sym->s_next_in_csect != NULL) {
			bind_err(SAY_NORMAL, RC_SEVERE,
				 NLSMSG(LABELED_TD,
	 "%1$s: 0711-340 SEVERE ERROR Symbol %2$s in object %3$s\n"
	 "\tA csect with multiple labels cannot be moved into the TOC."),
				 Main_command_name,
				 sym->s_name->name,
				 get_object_file_name(sym->s_csect
						      ->c_srcfile->sf_object));
			r->r_flags &= ~RLD_OVERFLOW_FIXUP_OK;
			toc_anchor_reference = 0; /* Don't look for TOC
						     in this case. */
		    }
		    else {
			DEBUG_MSG(RESOLVE_DEBUG,
				  (SAY_NO_NLS,
				   "TD reference to%s symbol [%d]%s",
				   (sym->s_smclass == XMC_TD) ? "":" (non-TD)",
				   sym->s_number, sym->s_name->name));
			sym->s_csect->c_TD_ref = 1; /* Mark referenced csect
						       so it will be mapped
						       into the TOC. */
		    }
		}
		else {
		    DEBUG_MSG(RESOLVE_DEBUG, (SAY_NO_NLS,
		      "TD reference to undefined symbol %s; Fixup code needed",
					      r->r_sym->s_name->name));
		    r->r_flags |= RLD_TOCDATA_FIXUP;
		    ++Bind_state.num_data_in_toc_fixups;
		    sym = find_or_create_toc_entry(r, cs, visited_bit);
		    /* The RLD has been updated to point to a TOC entry, which
		       itself points to the undefined symbol. */
		    r->r_sym->s_resolved->s_csect->c_first_rld->r_flags
			|= RLD_TOCDATA_REF;
		    if (sym == NULL)	/* Previous toc symbol found. */
			continue;
		}
	    }
	}

	if (sym) {
	    /* sym is NULL if it is unresolved or already resolved and we're
	       making an implicit reference to the TOC anchor. */
	    dfs(sym);
	}

	if (toc_anchor_reference) {
	    /* We have a toc-relative reference.  Therefore, we have
	       an implicit reference to the TOC anchor in the same section
	       as the referenced TOC symbol.  Make sure it's marked
	       properly. This only needs to be done once per section. */

	    if (r->r_sym->s_inpndx == INPNDX_GENERATED) {
		/* We must have a reference to a data-in-toc variable,
		   and this is the generated TOC entry.  Aside from error
		   checking, we only need to be sure at least one TOC
		   anchor exists. The XTY_ER/XMC_TD symbol is passed in
		   case an error needs to be printed. */
		toc_sym
		    = get_toc_anchor(r->r_sym->s_csect->c_srcfile->sf_object,
				     r->r_sym->s_csect->c_first_rld->r_sym);
	    }
	    else if (r->r_sym->s_smtype == XTY_ER) {
		/* The reference is to an ER, so we have no direct way to
		   find the TOC anchor.  If the code was fixed up previously,
		   there will be a TC symbol pointing to r->r_sym that we
		   could search for, but there could be other TC symbols in
		   the object file as well. If the input file is from the
		   compiler, there is no TC symbol, but there must be a TOC
		   anchor associated with the code.

		   Even for a rebind, we don't search for the TC symbol,
		   because of the difficulty in finding the proper one.
		   In addition, even for rebinding, we can restore the
		   original instruction without knowing which TC entry was
		   involved, as long as we restore the instruction by
		   examining the fixup code and not by finding a copy of
		   the original instruction before the fixup code.  See
		   code_save.c for details. */
		toc_sym = get_toc_anchor(r->r_sym->s_object, r->r_sym);
	    }
/* Define shorthand notation */
#define SI r->r_sym->s_csect->c_srcfile->sf_object-> \
		oi_section_info[r->r_sym->s_csect->c_secnum-1]

	    else if (!(SI.sect_flags & SECT_TOC_REFERENCED)) {
		SI.sect_flags |= SECT_TOC_REFERENCED;
		toc_sym = SI.l_toc_anchor;
		if (toc_sym == NULL) {
		    SI.l_toc_anchor = &dummy_TOC_anchor;
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(NO_TOC_ANCHOR,
 "%1$s: 0711-325 SEVERE ERROR: Symbol %2$s (entry %3$d) in section %4$d\n"
 "\tin object %5$s: There is a TOC-relative\n"
 "\treference to the symbol, but the section contains no TOC anchor."),
			     Main_command_name,
			     r->r_sym->s_name->name,
			     r->r_sym->s_inpndx,
			     r->r_sym->s_csect->c_secnum,
			     get_object_file_name(r->r_sym->s_csect
						  ->c_srcfile->sf_object));
		}
	    }
	    else
		toc_sym = NULL;		/* Already done. */
	    /* We've found the implicitly referenced TOC anchor.  If we haven't
	       already found a TOC anchor, we need to get it marked now. */
	    if (toc0_symbol == NULL && toc_sym != NULL) {
		DEBUG_MSG(RESOLVE_DEBUG,
			  (SAY_NO_NLS, "Making implicit reference to TOC"));
		if (toc_sym->s_flags & S_HIDEXT)
		    sym = resolve_hidden_symbol(toc_sym);
		else
		    sym = resolve_name(toc_sym->s_name,
				       toc_sym,
				       ERR_UNRESOLVED);
		if (sym)
		    dfs(sym);
		toc_sym->s_flags |= S_VISITED;
	    }
	}

	/* Mark referenced symbol after calling dfs(), in
	   case sym != r->r_sym.  This will prevent extra dfs() calls if
	   this symbol is referenced again. */
	r->r_sym->s_flags |= S_VISITED;
    } /* for all RLDs in CSECT */

#ifdef DEBUG
    dfs_depth--;
#endif
} /* dfs */
/***************************************************************************
 * Name:	display_resolve_errors
 *
 * Purpose:	Print messages for duplicate symbols and other messages arising
 *		from symbol resolution.
 *
 * Results:	Referenced symbols are 'marked'
 * *************************************************************************/
void
display_resolve_errors(int from_glue)	/* 1 if called from addgl(). */
{
    int		header_shown = 0;
    int		dumped_first;
    int		i;
    int		max_align, max_len;
    const char *head1 = Switches.dbg_opt11 ? " Sym# " : "";
    const char *head2 = Switches.dbg_opt11 ? " -----" : "";
    const int	verbose_messages = Switches.verbose | Switches.dbg_opt8;
    long	tmp_addr;
    STR		*name;
    SYMBOL	*sym, *other_sym;

    if (resolve_names_flags & (ERR_DUPLICATE | ERR_CM_BUMP | ERR_ALIGN_BUMP))
	for (i = 0; i < resolve_names_count; i++) {
	    if (!(resolve_names[i].flags
		  & (ERR_DUPLICATE | ERR_CM_BUMP | ERR_ALIGN_BUMP)))
		continue;
	    if (resolve_names[i].flags & ERR_DUPLICATE) {
		name = resolve_names[i].name;
		bind_err(SAY_STDERR_ONLY, RC_WARNING,
			 NLSMSG(CMPCT_RCW_DUP1,
			"%1$s: 0711-224 WARNING: Duplicate symbol: %2$s"),
			 Main_command_name, name->name);
		if (header_shown == 0) {
		    say(SAY_NORMAL, NLSMSG(DUPLICATE_MSG,
"%1$s: 0711-228 WARNING: Duplicate symbols were found while resolving symbols.\n"
"\tThe following duplicates were found:"),
			Main_command_name);

		    if (Switches.verbose)
			say(SAY_NORMAL,
			    NLSMSG(DUPLICATE_HEAD1,
" Symbol                   %s Inpndx  Address  TY CL Length Align Source-File(Object-File) OR Import-File{Shared-object}\n"
" -------------------------%s ------- -------- -- -- ------ ----- ------------------------------------------------------"
				   ),
			    head1, head2);
		    else
			say(SAY_NORMAL,
			    NLSMSG(DUPLICATE_HEAD2,
" Symbol                    Source-File(Object) OR Import-File{Shared-object}\n"
" ------------------------- -------------------------------------------------"
				   ));
		    header_shown = 1;
		}
		/* Print information about the winning symbol */
		sym = name->first_ext_sym;
		if (sym->s_smtype == XTY_CM) {
		    max_len = sym->s_csect->c_len;
		    max_align = sym->s_csect->c_align;
		}
#ifdef DEBUG
		else {
		    max_len = 0;
		    max_align = 0;
		}
#endif
		say(SAY_NO_NLS | SAY_NO_NL, " %-25s", name->name);
		display_symbol(sym, 0);

		/* Print information about the losing symbols */
		for (other_sym = sym->s_synonym;
		     other_sym;
		     other_sym = other_sym->s_synonym) {
		    if (other_sym->s_smtype == XTY_AR
			|| (other_sym->s_flags & S_DUPLICATE))
			/* Symbol is not relevant */
			continue;

		    /* Check for silent replacement case: */
		    if (other_sym->s_number < 0) {
			other_sym->s_number = -other_sym->s_number;
			/* If a XTY_CM symbol is replaced, the replacment is
			   usually silent.  However, if other duplicates exist
			   for the symbol, we should print the CM symbol
			   as well. */
			if (!verbose_messages && other_sym->s_smtype != XTY_CM)
			    continue;
		    }
		    /* If both symbols are XTY_CM, compute the maximum
		       length and alignment. */
		    if (sym->s_smtype == XTY_CM
			&& other_sym->s_smtype == XTY_CM) {
			if (other_sym->s_csect->c_len > max_len)
			    max_len = other_sym->s_csect->c_len;
			if (other_sym->s_csect->c_align > max_align)
			    max_align = other_sym->s_csect->c_align;
		    }
		    say(SAY_NO_NLS | SAY_NO_NL, "    %-22s",
			msg_get(NLSMSG(LIT_DUPLICATE, "** Duplicate **")));
		    display_symbol(other_sym, 0);

		    if (other_sym->s_smtype == XTY_CM) {
			switch(sym->s_smtype) {
			  case XTY_LD:
			    if (other_sym->s_csect->c_len >
				sym->s_csect->c_len
				- (sym->s_addr - sym->s_csect->c_addr))
				bind_err(SAY_NORMAL, RC_ERROR,
					 NLSMSG(NOROOM,
	"%1$s: 0711-346 ERROR: Replaced XTY_CM symbol %2$s is larger\n"
	"\tthan the space remaining after the label that replaces it."),
					 Main_command_name,
					 sym->s_name->name);

			    /* Compute an address aligned as little as
			       possible for the csect. */
			    tmp_addr = 1 << sym->s_csect->c_align;
			    /* Compute relative address of label in csect. */
			    tmp_addr += sym->s_addr
				- sym->s_csect->c_addr;
			    if (tmp_addr
				!= ROUND(tmp_addr,
					 (1 << other_sym->s_csect->c_align)))
				bind_err(SAY_NORMAL, RC_WARNING,
					 NLSMSG(BAD_ALIGN1,
 "%1$s: 0711-341 WARNING: Replaced XTY_CM symbol %2$s requires an\n"
 "\talignment more strict than the alignment of the symbol that replaces it."),
					 Main_command_name,
					 sym->s_name->name);
			    break;
			  case XTY_SD:
			    if (other_sym->s_csect->c_len
				> sym->s_csect->c_len)
				bind_err(SAY_NORMAL, RC_ERROR,
					 NLSMSG(SD_SMALL,
	"%1$s: 0711-326 ERROR: Replaced XTY_CM symbol %2$s is larger\n"
	"\tthan the symbol that replaces it."),
					 Main_command_name,
					 sym->s_name->name);

			    if (other_sym->s_csect->c_align
				> sym->s_csect->c_align)
				bind_err(SAY_NORMAL, RC_WARNING,
					 NLSMSG(BAD_ALIGN1,
 "%1$s: 0711-341 WARNING: Replaced XTY_CM symbol %2$s requires an\n"
 "\talignment more strict than the alignment of the symbol that replaces it."),
					 Main_command_name,
					 sym->s_name->name);
			    break;
			}
		    }
		}
	    }

	    /* Print messages that might have been saved up for display. */
	    if (resolve_names[i].flags & ERR_CM_BUMP) {
#ifdef DEBUG
		if (max_len <= sym->s_csect->c_len)
		    internal_error();
#endif
		sym->s_csect->c_len = max_len;
		bind_err(SAY_NORMAL, RC_WARNING,
			 from_glue
			 ? NLSMSG(CM_BUMPED_GLUE,
  "%1$s: 0711-359 WARNING: Length of XTY_CM symbol %2$s increased to %3$d.")
			 : NLSMSG(CM_BUMPED,
  "%1$s: 0711-349 WARNING: Length of XTY_CM symbol %2$s set to %3$d."),
			 Main_command_name, sym->s_name->name, max_len);
	    }
#ifdef DEBUG
	    else {
		if (max_len > sym->s_csect->c_len)
		    internal_error();
	    }
#endif

	    if (resolve_names[i].flags & ERR_ALIGN_BUMP) {
#ifdef DEBUG
		if (max_align <= sym->s_csect->c_align)
		    internal_error();
#endif
		sym->s_csect->c_align = max_align;
		bind_err(SAY_NORMAL, RC_WARNING,
			 from_glue
			 ? NLSMSG(CM_ALIGNED_GLUE,
"%1$s: 0711-358 WARNING: Alignment of XTY_CM symbol %2$s increased to %3$d.")
			 : NLSMSG(CM_ALIGNED,
"%1$s: 0711-348 WARNING: Alignment of XTY_CM symbol %2$s set to %3$d."),
			 Main_command_name,  sym->s_name->name, max_align);
	    }
#ifdef DEBUG
	    else {
		if (max_align > sym->s_csect->c_align)
		    internal_error();
	    }
#endif
	    /* Reset flags */
	    resolve_names[i].flags
		&= ~(ERR_DUPLICATE | ERR_CM_BUMP | ERR_ALIGN_BUMP);
	}

    if (resolve_names_flags & ERR_TOC_RESOLVE) {
	for (i = 0; i < resolve_names_count; i++) {
	    if (resolve_names[i].flags & ERR_TOC_RESOLVE) {
		if (header_shown == 1) {
		    say(SAY_NL_ONLY);	/* Separate the TOC messages */
		    header_shown = 0;	/* These messages have new header */
		}
		say(SAY_NORMAL,
		    NLSMSG(PRINT_TOC,
			   "%1$s: The following TOC Symbols were combined:"),
		    Command_name);
		for (sym = resolve_names[i].name->first_hid_sym;
		     sym;
		     sym = sym->s_synonym) {
		    if ((sym->s_flags & (S_TOC_PRINTED | S_RESOLVED_OK))
			!= S_RESOLVED_OK)
			continue;
		    dumped_first = 0;
		    sym->s_flags |= S_TOC_PRINTED;
		    for (other_sym = sym->s_synonym;
			 other_sym;
			 other_sym = other_sym->s_synonym) {
			if (((other_sym->s_flags & (S_TOC_PRINTED
						   | S_RESOLVED_OK))
			     == S_RESOLVED_OK)
			    && other_sym->s_resolved == sym->s_resolved) {
			    if (dumped_first == 0) {
				if (header_shown == 0) {
				    if (Switches.verbose)
					say(SAY_NORMAL,
					    NLSMSG(DUPLICATE_TOC_HEAD1,
" Referenced Symbol         TOC Symbol               %s Inpndx  Address  TY Source-File(Object-File) OR Import-File{Shared-object}\n"
" ------------------------- -------------------------%s ------- -------- -- ------------------------------------------------------"
						   ),
					    head1, head2);
				    else
					say(SAY_NORMAL,
					    NLSMSG(DUPLICATE_TOC_HEAD2,
" Referenced Symbol         TOC Symbol                Source-File(Object) OR Import-File{Shared-object}\n"
" ------------------------- ------------------------- -------------------------------------------------"
				   ));
				    header_shown = 1;
				}
				say(SAY_NO_NLS | SAY_NO_NL, " %-25s %-25s",
				    sym->s_csect->c_first_rld->r_sym
				    	->s_name->name,
				    sym->s_name->name);
				display_symbol(sym, 1);
				dumped_first = 1;
			    }
			    say(SAY_NO_NLS | SAY_NO_NL, " %-25s    %-22s",
				"",
				msg_get(NLSMSG(LIT_DUPLICATE,
					       "** Duplicate **")));
			    display_symbol(other_sym, 1);
			    other_sym->s_flags |= S_TOC_PRINTED;
			}
		    }
		}
		/* Reset flags */
		resolve_names[i].flags &= ~ERR_TOC_RESOLVE;
	    }
	}
    }

    if (header_shown)
	show_loadmap_message(RC_WARNING);

    /* Reset flags */
    resolve_names_flags
	&= ~(ERR_DUPLICATE | ERR_CM_BUMP | ERR_ALIGN_BUMP | ERR_TOC_RESOLVE);
} /* display_resolve_errors */
/***************************************************************************
 * Name:	resolve
 *
 * Purpose:	Resolve all referenced symbols
 *
 * Results:	Referenced symbols are 'marked'
 * *************************************************************************/
/*ARGSUSED*/
RETCODE
resolve(char *arg[])				/* argv-style arguments */
{
    char	*id = "resolve";
    int		i;
    int		keepall = Switches.keepall;
    CSECT	*cs;
    HASH_STR	*sroot, *shash;
    IFILE	*ifile;
    OBJECT	*obj;
    SRCFILE	*sf;
    STR		*s;
    SYMBOL	*entry_sym, *sym;
    SYMBOL	*alt_sym, *plain_sym, *dot_sym;
    RLD		*rld, *rld2;

    if (!(Bind_state.state & STATE_RESOLVE_NEEDED)) {
	bind_err(SAY_NORMAL, RC_OK,
		 NLSMSG(RESOLVE_AGAIN,
	"%1$s: 0711-347 WARNING: RESOLVE already called and no new symbols\n"
	"\thave been inserted, exported, or kept."),
		 Main_command_name);
	return RC_OK;
    }
    Bind_state.state &= ~STATE_RESOLVE_NEEDED;

    if (Bind_state.state & STATE_RESOLVE_CALLED) {
	/* Resolve is being called again--we have to clear the bits */
	bind_err(SAY_NORMAL, RC_WARNING,
		 NLSMSG(RESOLVE_WARNING,
 "%1$s: 0711-329 WARNING: Binder command RESOLVE called more than once.\n"
 "\tMessages may differ from previous call."),
		 Main_command_name);
	/* Now reinitialize all the bits. */
	for (obj = first_object(); obj; obj=obj->o_next) {
	    switch(obj->o_type) {
	      case O_T_OBJECT:
		for (sf = obj->oi_srcfiles; sf; sf = sf->sf_next)
		    for (cs = sf->sf_csect; cs; cs = cs->c_next) {
			cs->c_mark = 0;
			if (Bind_state.num_data_in_toc_fixups > 0) {
			    for (rld = cs->c_first_rld; rld; rld = rld->r_next)
				if (rld->r_flags & RLD_TOCDATA_FIXUP) {
				    rld2 = rld->r_sym->s_csect->c_first_rld;
				    rld->r_flags &= ~(RLD_TOCDATA_FIXUP
						      | RLD_EXT_REF
						      | RLD_RESOLVE_BY_NAME);
				    rld->r_flags
					|= rld2->r_flags & (RLD_RESOLVE_BY_NAME
							   | RLD_EXT_REF);
				    --Bind_state.num_data_in_toc_fixups;
				    rld->r_sym = rld2->r_sym;
				}
			}
		    }
	    }
	}
	for_all_STRs(sroot, i, shash, s) {
	    s->flags &= ~(STR_RESOLVED | STR_NO_DEF | STR_ERROR | STR_IMPORT
			  | STR_DS_EXPORTED | STR_TOC_RESOLVED);
	    for (sym = s->first_ext_sym; sym; sym = sym->s_synonym) {
		if (sym->s_flags & S_TYPECHK_IMPLIED)
		    sym->s_typechk = NULL;
		sym->s_flags &= ~(S_MARK | S_VISITED | S_TYPECHK_USED
				  | S_TYPECHK_IMPLIED);
	    }
	    for (sym = s->first_hid_sym; sym; sym = sym->s_synonym)
		sym->s_flags &= ~(S_MARK | S_VISITED | S_RESOLVED_OK | S_ISTOC
				  | S_TOC_PRINTED);	/* For dbopt8 */
	    for (sym = s->refs; sym; sym = sym->s_synonym)
		sym->s_flags &= ~(S_MARK | S_VISITED | S_TYPECHK_USED);
	}
	if (resolve_names != NULL)
	    efree(resolve_names);
	/* Re-allocate the array to keep track of symbol names that might need
	   error messages printed about them. */
	resolve_names = emalloc(sizeof(struct resolve_info) * 2 * total_STRS(),
				id);
	Bind_state.o_toc_sym = NULL;
	toc0_symbol = NULL;
    }
    else
	Bind_state.state |= STATE_RESOLVE_CALLED;

    kept_symbols = 0;
    delcsect_message_needed = 0;
    loadmap_message_seen = 0;		/* Reset for show_loadmap_message(). */

    /* Allocate an array to keep track of symbol names that might need
       error messages printed about them.  We only need to save the names,
       but to allow for generated symbols, we allocate space for twice the
       number of current names.  */
    resolve_names = emalloc(sizeof(struct resolve_info) * 2 * total_STRS(),id);
    resolve_names_count = 0;

    /* Resolve entry point and its successors. */
    if (Bind_state.entrypoint == NULL)
	Bind_state.entrypoint_sym = NULL; /* No entrypoint */
    else {
	entry_sym = resolve_name(Bind_state.entrypoint, NULL,
				 0 /* Keep additional errors from being
				      generated. */);
	Bind_state.entrypoint_sym = entry_sym;
	if (entry_sym == NULL) {
	    if (Switches.execute) {
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(NO_ENTRY,
			"%1$s: 0711-327 WARNING: Entry point not found: %2$s"),
			 Main_command_name, Bind_state.entrypoint->name);
	    }
	}
	else {
	    if (entry_sym->s_smclass != XMC_DS && Switches.verbose) {
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(CMPCT_RCW_NDESC,
	"%1$s: 0711-220 WARNING: The entry point is not a descriptor: %2$s"),
			 Main_command_name, Bind_state.entrypoint->name);
	    }

	    DEBUG_MSG(RESOLVE_DEBUG,
		      (SAY_NO_NLS, "Resolving entrypoint symbol %s",
		       entry_sym->s_name->name));
	    dfs(entry_sym);
	}
    }

    /* If we're keeping all symbols in some files, but not all symbols
       in all files, go through each file to resolve reachable symbols */
    if (Bind_state.files_kept > 0 && !keepall) {
	for (ifile = first_ifile; ifile; ifile = ifile->i_next) {
	    if (ifile->i_keepfile == 0)
		continue;
	    for (obj=ifile->i_objects; obj->o_ifile == ifile; obj=obj->o_next){
		switch(obj->o_type) {
		  case O_T_OBJECT:
		    DEBUG_MSG(RESOLVE_DEBUG,
			      (SAY_NO_NLS, "Keeping symbols in object %s",
			       get_object_file_name(obj)));
		    for (sf = obj->oi_srcfiles; sf; sf = sf->sf_next)
			for (cs = sf->sf_csect; cs; cs = cs->c_next)
			    for (sym = &cs->c_symbol;
				 sym;
				 sym = sym->s_next_in_csect)
				if (!(sym->s_flags & S_HIDEXT)) {
				    s = sym->s_name;
				    sym = resolve_name(s, NULL,
						       s->flags & (STR_EXPORT
								  | STR_KEEP));

				    if (sym)
					dfs(sym);
				}
		    break;
		  case O_T_SCRIPT:
		  case O_T_SHARED_OBJECT:
		    /* Imported symbols must be redefined when rebinding, so
		       these symbols can't be kept. */
		    break;
		}
	    }
	}
    }

    if (!keepall && Bind_state.num_exports == 0 && KEEP_count == 0) {
	DEBUG_MSG(RESOLVE_DEBUG,
		  (SAY_NO_NLS,
	   "Skipping for_all_STRs because no symbols kept or exported."));
	goto finish_up;
    }

    /* Process KEPT or EXPORTed symbols, or all symbols if 'setopt keepall'
       was specified. For KEPT and EXPORTed symbols, we process both the
       specified name and its alternate. */
    for_all_STRs(sroot, i, shash, s) {
	if (!(s->flags & STR_ISSYMBOL))
	    continue;
	if (keepall || (s->flags & (STR_KEEP | STR_EXPORT))) {
	    sym = resolve_name(s, NULL,
			       s->flags & (STR_KEEP | STR_EXPORT));

	    if (s->flags & STR_EXPORT) {
		/* We have an exported symbol.  If we say 'export .foo' and
		   'foo' is a descriptor, we export 'foo' instead.  Otherwise,
		   we just export the name as specified. */
		if (s->name[0] == '.' && s->name[1] != '.') {
		    /* s is a dot name */
		    dot_sym = sym;
		    /* Look for plain_sym, but there's no error if it's not
		       found. */
		    plain_sym = resolve_name(s->alternate, NULL, 0);
		    if (plain_sym && plain_sym->s_smclass == XMC_DS) {
			bind_err(SAY_NORMAL, RC_WARNING,
				 NLSMSG(EXPORT_MOD,
	"%1$s: 0711-324 WARNING: Export of %2$s changed to export of %3$s"),
				 Main_command_name,
				 dot_sym->s_name->name,
				 plain_sym->s_name->name);
			sym = plain_sym;
			if (dot_sym->s_name->flags & STR_SYSCALL)
			    plain_sym->s_name->flags |= STR_SYSCALL;
			/* Clear EXPORT flag for .name; Set flag for 'name'. */
			dot_sym->s_name->flags &= ~STR_EXPORT;
			plain_sym->s_name->flags |= STR_EXPORT;
		    }
		}

		if (sym->s_name->flags & STR_SYSCALL) {
		    /* We allow imported symbols to be exported as
		       syscalls, so that a common export file can be used.
		       Note that the loader treats XMC_SV identically
		       to XMC_DS except for the kernel and kernel
		       extensions. */
		    if (!imported_symbol(sym) && sym->s_smclass != XMC_DS)
			bind_err(SAY_NORMAL, RC_WARNING,
				 NLSMSG(BAD_EXPORT,
		"%1$s: 0711-320 WARNING: Symbol %2$s does not appear to be a\n"
		"\tprocedure and should not be exported as a system call."),
				 Main_command_name, sym->s_name->name);
		}

		if (sym)
		    dfs(sym);
	    }
	    else {
		/* The symbol is KEPT, or we have 'setopt keepall'.
		   There is never a need to keep an imported symbol here,
		   because imported symbols must be redefined during a rebind.
		   We don't save XO symbols, but they will be kept if
		   they are referenced. */
		if (sym == NULL
		    || imported_symbol(sym)
		    || sym->s_smclass == XMC_XO)
		    continue;

		if (s->flags & STR_KEEP) {
		    if (sym->s_smtype == XMC_DS
			&& sym->s_inpndx == INPNDX_GENERATED) {
			/* We said 'keep foo', but 'foo' is a generated
			   descriptor (which must refer to the label '.foo').
			   We don't really need the descriptor in this case,
			   so we just save the code itself. */
#ifdef DEBUG
			if (s->alternate == NULL)
			    internal_error();
#endif
			alt_sym = resolve_name(s->alternate, NULL, STR_KEEP);
#ifdef DEBUG
			if (alt_sym == NULL)
			    internal_error();
#endif
			dfs(alt_sym);
			continue;
		    }
		    dfs(sym);
		}

		/* Other generated symbols don't have to be kept. */
		if (sym && sym->s_inpndx != INPNDX_GENERATED)
		    dfs(sym);
	    }
	}
    }

  finish_up:
    /* Print errors about duplicate symbols. */
    if (resolve_names_flags & (ERR_DUPLICATE | ERR_TOC_RESOLVE
			       | ERR_CM_BUMP | ERR_ALIGN_BUMP))
	display_resolve_errors(0);

#ifdef DEBUG
    if (dfs_depth != 0)
	internal_error();
#endif

    say(SAY_NORMAL,
	NLSMSG(CMPCTS_KEPT, "%1$s: %2$d of %3$d symbols were kept."),
	Command_name,
	kept_symbols,
	total_symbols_allocated());

    DEBUG_MSG(RESOLVE_DEBUG,
	      (SAY_NO_NLS, "resolve_names_count = %d", resolve_names_count));

    return RC_OK;
} /* resolve */
/***************************************************************************
 * Name:	print_refs_for_sym
 *
 * Purpose:	Print all RLDs referring to a particular symbol.  The symbol
 *		is usually an XTY_ER, but it could be a deleted symbol if
 *		the delcsect option was used.
 *
 * Results:
 * *************************************************************************/
static void
print_refs_for_sym(SYMBOL *bad_sym,
		   int rld_indent,	/* If non-zero, print RLD information
					   indented by rld_indent blanks. */
		   int print_name,	/* -2: Leave blanks for name, but not
					       first time line is printed.
					   -1: leave blanks for name
					    1: Print name.
					    0: Don't print name. */
		   int print_hash)	/* -1: leave blanks for hash
					    1: Print hash
					    0: Don't print hash. */
{
    char 	*sect_name;
    int		sf_shown;
    int		n;
    int		name_len;
    CSECT	*cs;
    OBJECT	*obj;
    RLD		*rld;
    SRCFILE	*sf;
    SYMBOL	*sym;

    if (print_name == -2)
	name_len = 0;
    else
	name_len = MINIDUMP_NAME_LEN * print_name;

    if (bad_sym->s_smtype == XTY_ER)
	obj = bad_sym->er_object;
    else {
	obj = bad_sym->s_csect->c_srcfile->sf_object;
	if (name_len > 0) {
	    say(SAY_NO_NLS | SAY_NO_NL, "%%"); /* Print single %. */
	    name_len -= 1;		/* Decrease width of field to account
					   for % marker.*/
	}
    }

    sf_shown = 0;
    for (sf = obj->oi_srcfiles; sf; sf = sf->sf_next) {
	/* For ERs, we print a new instance every time there are references
	   from within the scope of a new C_FILE symbol.  For deleted symbols,
	   only a single instance is printed. */
	if (bad_sym->s_smtype == XTY_ER)
	    sf_shown = 0;
	for (cs = sf->sf_csect; cs; cs = cs->c_next) {
	    for (rld = cs->c_first_rld; rld; rld = rld->r_next)
		if (rld->r_sym == bad_sym) {
		    if (sf_shown == 0) {
			sf_shown = 1;

			minidump_symbol(bad_sym, name_len,
					(print_hash == 0 ? 0
					: print_hash > 0 ? MINIDUMP_HASH
							: MINIDUMP_HASH_PAD)
					| MINIDUMP_SYMNUM_DBOPT11
					| MINIDUMP_INPNDX
					| MINIDUMP_TYPE
					| MINIDUMP_SMCLASS
					| MINIDUMP_SOURCE_INFO,
					(bad_sym->s_smtype == XTY_ER
					 && rld_indent != 0)
						? &cs->c_symbol : NULL);
			if (rld_indent == 0)
			    return;
			if (print_name == -2)
			    name_len = -MINIDUMP_NAME_LEN;
		    }
		    for (sym = &cs->c_symbol;
			 sym;
			 sym = sym->s_next_in_csect) {
			if (sym->s_addr <= rld->r_addr
			    && (sym->s_next_in_csect == NULL
				||  sym->s_next_in_csect->s_addr > rld->r_addr))
			    break;
		    }
		    if (sym == NULL)	/* Should never happen */
			sym = &cs->c_symbol;
		    switch(obj->oi_section_info[sym->s_csect->c_secnum-1]
			   .sect_type) {
		      case STYP_TEXT:	sect_name = ".text";	break;
		      case STYP_DATA:	sect_name = ".data";	break;
		      default:		sect_name = "";		break;
		    }
		    if (Switches.dbg_opt11)
			say(SAY_NO_NLS | SAY_NO_NL,
			    "%*s%-6s%08x %-8s %-8s %-6s",
			    rld_indent, "", show_rld(rld, NULL),
			    rld->r_addr, sect_name,
			    get_reltype_name(rld->r_reltype),
			    show_sym(sym, NULL));
		    else
			say(SAY_NO_NLS | SAY_NO_NL, "%*s%08x %-8s %-8s ",
			    rld_indent, "", rld->r_addr, sect_name,
			    get_reltype_name(rld->r_reltype));
		    n = 5 - show_inpndx(sym, "[%s]");
		    say(SAY_NO_NLS,
			(sym->s_flags & S_HIDEXT) ? "%*s<%s>" : "%*s%s",
			n+1, " ", sym->s_name->name);
		}
	}
    }
} /* print_refs_for_sym */
/************************************************************************
 * Name: unres			Processor for the ER command		*
 *									*
 * Purpose: Command processor for the ER binder command.  Depending	*
 *	on the option, checks to see if there are any unresolved	*
 *	references or any undefined symbols.				*
 *									*
 * Command Format:							*
 *	ER	-- display all undefined symbols.			*
 *    or								*
 *	ER FULL	-- display all unresolved references.			*
 *									*
 * Parms/Returns:							*
 *	Input: FULL - flag requesting the ER FULL processing option	*
 *									*
 *	Returns: Returns a status completion code.			*
 *		0 - OK    No unresolved symbols found			*
 *		8 - ERROR Unresolved symbols were found			*
 *									*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
unres(char *arg[])			/* argv-style arguments */
{
    static char *id = "unres";
    int		full = 0;
    int		i;
    int		header_printed = 0;
    const char *head1 = Switches.dbg_opt11 ? " SYM# " : "";
    const char *head2 = Switches.dbg_opt11 ? " -----" : "";
    const char *head3 = Switches.dbg_opt11 ? " RLD# " : "";
    HASH_STR	*sroot, *sh;
    RETCODE	rc = RC_OK;
    int		need_loadmap_message = 0;
    STR		*s;
    SYMBOL	*er, *sym, *dup_sym;

    if (Bind_state.state & STATE_RESOLVE_NEEDED) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(RESOLVE_NEEDED,
		"%1$s: 0711-300 ERROR: RESOLVE must be called before calling binder command %2$s."),
		 Main_command_name, Command_name);
	return RC_NI_ERROR;
    }
    if (resolve_names == NULL) {
	/* Our array was deleted. Allocate and regenerate the information */
	resolve_names = emalloc(sizeof(struct resolve_info) * total_STRS(),id);
	/* NOTE: These errors could differ slightly from the original ones. */
	for_all_STRs(sroot, i, sh, s) {
	    if ((s->flags & (STR_RESOLVED | STR_NO_DEF))
		== (STR_RESOLVED | STR_NO_DEF)
		&& s != Bind_state.entrypoint) {
		/* Symbol needed but not defined. */
		rc = RC_WARNING;
		if (s->refs && s->first_ext_sym)
		    delcsect_message_needed = 1;
		add_to_resolve_names(s,
				     (s->flags & (STR_EXPORT | STR_KEEP))
				     | (s->refs ? ERR_UNRESOLVED : 0));
	    }
	}
    }

    if (arg[1]) {
	lower(arg[1]);
	if (strcmp(arg[1], "full") == 0) {
	    /* Indentation for RLDs */
	    full = 1 + MINIDUMP_NAME_LEN + 1 + 8;
	}
    }

    /* Resolve processing has already updated the resolve_names[] array for
       all unresolved symbols.  In this function, we just print the names.
       Note that there are separate errors for exported symbols, kept symbols,
       and other symbols. */

    if (resolve_names_flags & ERR_EXPORT) {
	rc = RC_WARNING;
	for (i = 0; i < resolve_names_count; i++)
	    if (resolve_names[i].flags & ERR_EXPORT)
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(UNRES_EXPORT,
		"%1$s: 0711-319 WARNING: Exported symbol not defined: %2$s"),
			 Main_command_name, resolve_names[i].name->name);
    }
    if (resolve_names_flags & ERR_KEEP) {
	rc = RC_WARNING;
	for (i = 0; i < resolve_names_count; i++)
	    if (resolve_names[i].flags & ERR_KEEP)
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(UNRES_KEEP,
 "%1$s: 0711-301 WARNING: Symbol specified with the -u flag not defined: %2$s"),
			 Main_command_name, resolve_names[i].name->name);
    }

    if (resolve_names_flags & ERR_UNRESOLVED)
	for (i = 0; i < resolve_names_count; i++)
	    if (resolve_names[i].flags & ERR_UNRESOLVED) {
		s = resolve_names[i].name;
		sym = s->first_ext_sym;
		/* If symbol is no longer undefined, it must have been defined
		   by adding glue code.  For displaying errors, we use the
		   subsequent symbol, which will only be non-NULL if it is
		   a deleted symbol. */
		if (!(s->flags & STR_NO_DEF))
		    sym = sym->s_synonym;
		/* Find first visited reference */
		for (er = s->refs; er; er = er->er_synonym)
		    if (er->er_flags & S_VISITED)
			break;

		if (header_printed == 0) {
		    if (delcsect_message_needed)
			say(SAY_NORMAL,
			    NLSMSG(UNRES_MSG2,
   "%1$s: 0711-305 ERROR: Undefined symbols were found. Symbols marked\n"
   "\twith %% were deleted because the -bdelcsect option was specified.\n"
   "\tThe following symbols are in error:"),
			    Main_command_name);
		    else
			say(SAY_NORMAL,
			    NLSMSG(UNRES_MSG,
		   "%1$s: 0711-318 ERROR: Undefined symbols were found.\n"
		   "\tThe following symbols are in error:"),
			    Main_command_name);

		    if (full)
			say(SAY_NORMAL,
			    NLSMSG(UNRES_HEAD1,
" Symbol                   %s Inpndx  TY CL Source-File(Object-File) OR Import-File{Shared-object}\n"
  "                              RLD:%s Address  Section  Rld-type%s Referencing Symbol\n"
" -------------------------%s---------------------------------------------------------------------"
				   ),
			    head1, head3, head1, head2);
		    else
			say(SAY_NORMAL,
			    NLSMSG(UNRES_HEAD2,
   " Symbol                   %s Inpndx  TY CL Source-File(Object-File) OR Import-File{Shared-object}\n"
   " -------------------------%s ------- -- -- ------------------------------------------------------"
				   ),
			    head1, head2);

		    header_printed = 1;
		}

		if (sym == NULL) {
		    if (s == Bind_state.entrypoint && er == NULL) {
			/* Don't print additional error about entrypoint
			   unless it has explicit references. */
			continue;
		    }
		    bind_err(SAY_STDERR_ONLY, RC_ERROR,
			     NLSMSG(UNRESOLVE_HEADING,
			    "%1$s: 0711-317 ERROR: Undefined symbol: %2$s"),
			     Main_command_name, s->name);
		}
		else {
		    /* Symbol is undefined because of delcsect processing */
#ifdef DEBUG
		    if ((sym->s_flags & (S_DUPLICATE | S_DUPLICATE2))
			!= S_DUPLICATE)
			internal_error();
#endif

		    /* This call prints to stderr only. */
		    bind_err(SAY_STDERR_ONLY, RC_ERROR,
			     NLSMSG(UNRESOLVE_MSG,
		    "%1$s: 0711-304 ERROR: Undefined symbol: %2$s\n"
		    "\t(deleted because the -bdelcsect option was specified)"),
			     Main_command_name, s->name);

		    /* Print information about the symbol (and if references,
		       if full == 1) to the loadmap. */
		    do {
			print_refs_for_sym(sym, full, 1, /* Print name. */
					   0); /* Don't print hash. */
			sym = sym->s_synonym;
		    } while(sym);
		}

		rc = RC_ERROR;
		need_loadmap_message = 1;
		for (; er; er = er->er_synonym) {
		    if (er->er_flags & S_VISITED) {
			print_refs_for_sym(er, full, 1, /* Print name. */
					   0); /* Don't print hash. */
		    }
		}
	    }

    if (resolve_names_flags & ERR_HID_UNRESOLVED)
	for (i = 0; i < resolve_names_count; i++)
	    if (resolve_names[i].flags & ERR_HID_UNRESOLVED) {
		s = resolve_names[i].name;
		rc = RC_ERROR;

		/* An internal symbol can have multiple undefined instances. */
		for (sym = s->first_hid_sym; sym; sym = sym->s_synonym) {
		    if (sym->s_flags & S_DUPLICATE) {
			bind_err(SAY_NORMAL, RC_ERROR,
				 NLSMSG(INTERNAL_UNRESOLVED,
		"%1$s: 0711-342 ERROR: Undefined internal symbol: %2$s"),
				 Main_command_name, sym->s_name->name);
			need_loadmap_message = 1;
			for (dup_sym = &sym->s_csect->c_symbol;
			     dup_sym;
			     dup_sym = dup_sym->s_next_in_csect) {
			    if (dup_sym->s_flags & S_DUPLICATE2) {
				bind_err(SAY_NOSTDOUT, RC_ERROR,
					 NLSMSG(DATAS_CSDUP,
"%1$s: 0711-295 ERROR: The -bdelcsect option was used and the undefined symbol\n"
"\tis in a csect containing duplicate symbol: %2$s"),
					 Main_command_name,
					 dup_sym->s_name->name);
				break;
			    }
#ifdef DEBUG
			    if (dup_sym == NULL)
				internal_error();
#endif
			}
		    }
		}
	    }

    if (need_loadmap_message)
	show_loadmap_message(RC_ERROR);

    if (rc != RC_OK)
	return rc;

    say(SAY_NORMAL,
	NLSMSG(DATAS_NOUNRES, "%s: There are no unresolved symbols."),
	Command_name);
    return RC_OK;
} /* unres */
/***************************************************************************
 * Name:	display_typecheck_errors
 *
 * Purpose:	Display information about type-check mismatches.
 *
 * Results:
 * *************************************************************************/
static void
display_typecheck_errors(STR *name)
{
    static int	first_mismatch = 1;
    int		need_name, name_len;
    const char *head1 = Switches.dbg_opt11 ? " Sym# " : "";
    const char *head2 = Switches.dbg_opt11 ? " -----" : "";
    const char *head3 = Switches.dbg_opt11 ? " RLD# " : "";
    SYMBOL	*next_sym, *sym1;
    SYMBOL	*er, *er1;
    TYPECHK	*t1, *t2;
    int		references_printed;

    /* A type mismatch has been detected */
    bind_err(SAY_STDERR_ONLY, RC_ERROR,
	     NLSMSG(MISMATCH_SYM,
		    "%1$s: 0711-197 ERROR: Type mismatches for symbol: %2$s"),
	     Main_command_name, name->name);
    show_loadmap_message(RC_ERROR);

#if 0
    if (next_sym->s_csect->c_symbol.s_smclass == XMC_RO
	&& keep_sym->s_csect->c_symbol.s_smclass == XMC_RO
	&& next_sym->s_csect->c_len == cur_sym->s_csect->c_len) {
	bind_err(SAY_NORMAL, RC_WARNING,
		 NLSMSG(CMPCT_RCW_CONST,
		"%1$s: 0711-227 WARNING: Symbol %2$s may be a CONSTANT."),
		 Main_command_name, next_sym->s_name->name);
    }
#endif

    /* Information about the mismatch goes to stderr only
       (or stdout if -bnoquiet is used), so we use say() to display
       the remaining information. */
    if (first_mismatch == 1) {
	first_mismatch = 0;
	say(SAY_NORMAL,
	    NLSMSG(MISMATCH_HEAD,
		   "%1$s: 0711-189 ERROR: Type mismatches were detected.\n"
		   "\tThe following symbols are in error:"),
	    Main_command_name);
	if (Switches.verbose)
	    say(SAY_NORMAL,
		NLSMSG(MISMATCH_HEAD1,
" Symbol                    Hash                  %s Inpndx  TY CL Source-File(Object-File) OR Import-File{Shared-object}\n"
"                                            RLD:%s Address  Section  Rld-type%s Referencing Symbol\n"
" ------------------------- ----------------------%s --------------------------------------------------------------------"
		       ),
		head1, head3, head1, head2);
	else
	    say(SAY_NORMAL,
		NLSMSG(MISMATCH_HEAD2,
" Symbol                    Hash                  %s Inpndx  TY CL Source-File(Object-File) OR Import-File{Shared-object}\n"
" ------------------------- ----------------------%s ------- -- -- ------------------------------------------------------"
		       ),
		head1, head2);
    }
    else
	say(SAY_NL_ONLY);		/* Leave blank before other syms */

    if (name->flags & STR_NO_DEF) {
	say(SAY_NO_NLS, " %-25s %s",
	    name->name,
	    msg_get(NLSMSG(LIT_UNDEFINED, "** Undefined Symbol **")));
	goto do_all_ERs;
    }
    else
	next_sym = name->first_ext_sym;

    /* Reset any implied typecheck. */
    if (next_sym->s_flags & S_TYPECHK_IMPLIED) {
	next_sym->s_flags &= ~S_TYPECHK_IMPLIED;
	next_sym->s_typechk = NULL;
    }

    name_len = MINIDUMP_NAME_LEN;
    /* Print equivalence classes for symbols. */
    for ( ; next_sym; next_sym = next_sym->s_synonym) {
	if (next_sym->s_flags & S_TYPECHK_USED)
	    continue;		/* Already printed */
	if (next_sym->s_smclass == XMC_GL
	    && next_sym->s_synonym
	    && next_sym->s_synonym->s_typechk == next_sym->s_typechk
	    && ((next_sym->s_flags & S_LOCAL_GLUE)
		|| imported_symbol(next_sym->s_synonym)))
	    continue;			/* Don't print message
					   for glue code since we will
					   just print it for the imported
					   (or duplicate, local) symbol. */
	/* New hash value */
	if (name_len != MINIDUMP_NAME_LEN) {
	    say(SAY_NO_NLS | SAY_NO_NL, "    %-22s",
		msg_get(NLSMSG(LIT_DUPLICATE, "** Duplicate **")));
	}

	dump_controls |= DUMP_SHOW_INPNDX; /* Cause generated, imported symbols
					      to be displayed as IMPORT. */
	minidump_symbol(next_sym, name_len,
			MINIDUMP_HASH
			| MINIDUMP_SYMNUM_DBOPT11
			| MINIDUMP_INPNDX
			| MINIDUMP_TYPE
			| MINIDUMP_SMCLASS
			| MINIDUMP_SOURCE_INFO,
			NULL);
	dump_controls &= ~DUMP_SHOW_INPNDX; /* Reset */

	next_sym->s_flags |= S_TYPECHK_USED;
	name_len = 0;

	/* Print other matching definitions. */
	for (sym1 = next_sym->s_synonym;
	     sym1;
	     sym1 = sym1->er_synonym) {
	    if (sym1->s_flags & S_TYPECHK_IMPLIED)
		t1 = NULL;
	    else
		t1 = sym1->s_typechk;
	    if (next_sym->s_flags & S_TYPECHK_IMPLIED)
		t2 = NULL;
	    else
		t2 = next_sym->s_typechk;
	    if (t1 == t2) {
		sym1->s_flags |= S_TYPECHK_USED;
		say(SAY_NO_NLS | SAY_NO_NL, "    %-22s",
		    msg_get(NLSMSG(LIT_DUPLICATE, "** Duplicate **")));
		minidump_symbol(sym1, 0,
				MINIDUMP_HASH_PAD
				| MINIDUMP_SYMNUM_DBOPT11
				| MINIDUMP_INPNDX
				| MINIDUMP_TYPE
				| MINIDUMP_SMCLASS
				| MINIDUMP_SOURCE_INFO, NULL);
	    }
	}

	/* Print matching references */
	references_printed = 0;
	for (er = name->refs; er; er = er->er_synonym) {
	    /* Skip if ER not visited or already used for printing */
	    if ((er->er_flags & (S_TYPECHK_USED | S_VISITED)) != S_VISITED)
		continue;
	    if (er->er_typechk == next_sym->s_typechk) {
		if (references_printed == 0) {
		    references_printed = 1;
		    need_name = -2;
		    say(SAY_NO_NL | SAY_NO_NLS, "       %-19s",
			msg_get(NLSMSG(LIT_REFERENCES, "** References **")));
		}
		er->er_flags |= S_TYPECHK_USED;
		print_refs_for_sym(er,
				   Switches.verbose == 0 ? 0
				   : 1 + MINIDUMP_NAME_LEN
				   	+ HASH_FIELD_WIDTH + 1,
				   need_name, /* Print name? */
				   -1);	/* Leave blanks for hash */
		need_name = -1;
	    }
	}
    }

  do_all_ERs:
    references_printed = 0;
    /* Print remaining references that don't match any definition */
    for (er = name->refs; er; er = er->er_synonym) {
	/* Skip if ER not visited or already used for printing */
	if ((er->er_flags & (S_TYPECHK_USED | S_VISITED)) != S_VISITED)
	    continue;
	if (references_printed == 0) {
	    references_printed = 1;
	    say(SAY_NO_NLS, "       %s",
		 msg_get(NLSMSG(LIT_OTHER_REFS,
			"** References Without Matching Definitions **"))
		);
	}
	print_refs_for_sym(er,
			   Switches.verbose == 0 ? 0
			   : 1 + MINIDUMP_NAME_LEN + HASH_FIELD_WIDTH + 1,
			   -1,		/* Leave blanks for name */
			   1);		/* Print hash */

	/* Now we print equivalence classes of symbols.
	   If sym_typchk_set is 1, the 1st class contains references
	   without typchk strings. */

	/* Check all references for equality between "sym_typchk" and
	   the typchks of the references. */
	for (er1 = er->er_synonym; er1; er1 = er1->er_synonym) {
	    if (er1->er_typechk == er->er_typechk
		&& (er1->er_flags & S_VISITED)) {
		/* No check needed for S_TYPECHK_USED, because this is the
		   first time we've printed this symbol with this hash value */
		er1->er_flags |= S_TYPECHK_USED;
		print_refs_for_sym(er1,
				   Switches.verbose == 0 ? 0
				   : 1 + MINIDUMP_NAME_LEN
				   	+ HASH_FIELD_WIDTH + 1,
				   -1,	/* Leave blanks for name */
				   -1);	/* Leave blanks for hash */
	    }
	}
    }
} /* display_typecheck_errors */
/***************************************************************************
 * Name:	check_ext_refs
 *
 * Purpose:	Check all visited references to symbol 'sym_name'.  If they do
 *		not all match, print an error message.
 *
 * Results:
 * *************************************************************************/
static int
check_ext_refs(STR *sym_name,
	       int dot_name)
{
    int		rc = 0;
    int		sym_typchk_set = 0;
    uint8	smc;
    SYMBOL	*sym, *sym2;
    SYMBOL	dummy_symbol;
    SYMBOL	*er;

    if (sym_name->flags & STR_NO_DEF) {
	sym = &dummy_symbol;
	sym->s_typechk = NULL;
	sym->s_flags = 0;
	sym->s_smclass = XMC_PR;	/* Anything except XMC_GL */
    }
    else {
	sym = sym_name->first_ext_sym;
	/* Update storage-mapping class first */
	if (sym->s_smclass == XMC_UA) {
	    /* Try to get a better storage-mapping from an ER.  We never
	       use XMC_TD, because it will be picked up during save processing
	       if fixup code is needed. */
	    for (er = sym_name->refs; er; er = er->er_synonym)
		if ((er->er_flags & S_VISITED) && er->er_smclass != XMC_UA)
		    break;
	    if (er != NULL && er->er_smclass != XMC_TD) {
		smc = er->er_smclass;	/* First non-XMC_UA, non-XMC_TD
					   storage-mapping class for
					   visited ER */
		for (; er; er = er->er_synonym)
		    if ((er->er_flags & S_VISITED) && er->er_smclass != smc)
			goto no_update;

		if (smc == XMC_GL)
		    smc = XMC_PR;
		sym->s_smclass = smc;
		if (Switches.verbose)
		    add_to_resolve_names(sym->s_name, ERR_SMCLASS_CHANGED);
	    }
	}

      no_update:
	/* Now print type-check errors for symbol */
   	if (sym_name->flags & STR_ERROR
	    && (resolve_names[sym_name->str_value].flags & ERR_TYPECHK)) {
	    display_typecheck_errors(sym_name);
	    rc = 1;
	    goto skip_ers;
	}
    }

    /* Now do additional type-checking */
    for (er = sym_name->refs; er; er = er->er_synonym) {
	if (!(er->er_flags & S_VISITED))
	    continue;

	/* Pick up typchk from ER if symbol doesn't have one, or if we
	   picked one up from a duplicate symbol during resolve().  */
	if (sym->s_typechk == NULL ||
	    (sym_typchk_set == 0 && (sym->s_flags & S_TYPECHK_IMPLIED))) {
	    sym_typchk_set = 1;
	    if (er->er_typechk != NULL) {
		sym->s_typechk = er->er_typechk;
		sym->s_flags |= S_TYPECHK_IMPLIED;
		/* If '.name' is from glue and 'name' (without a dot) is a
		   descriptor without a typecheck value, then 'name' inherits
		   the typecheck value from '.name'. */
		if (dot_name
		    && sym->s_smclass == XMC_GL
		    && (sym->s_csect->c_srcfile->sf_object
			->oi_flags & OBJECT_GLUE)
		    && (sym2 = sym_name->alternate->first_ext_sym)
		    && sym2->s_typechk == NULL
		    && sym2->s_smclass == XMC_DS
		    && (sym2->s_flags & S_MARK)) {
		    sym2->s_typechk = sym->s_typechk;
		    sym2->s_flags |= S_TYPECHK_IMPLIED;
		}
	    }
	}
	else if (compare_hash(sym->s_typechk, er->er_typechk) != 0) {
	    display_typecheck_errors(sym_name);
	    rc = 1;
	    break;
	}
    }
  skip_ers:
    if (sym_typchk_set == 0 && (sym->s_flags & S_TYPECHK_IMPLIED))
	sym->s_typechk = NULL;		/* Reset */

    return rc;
} /* check_ext_refs */
/************************************************************************
 * Name: mismatch
 *
 * Purpose: Check all external references for hash type
 *	mismatches.  Print information about mismatches.
 *									*
 * Command Format:							*
 *	MISMATCH [free | nofree | freeonly]
 *									*
 * Arguments: none							*
 *									*
 * Returns: Returns a status completion code.				*
 *	OK	- no hash mismatches found.
 *	ERROR	- "resolve" not called yet or hash type mismatches found.
 *									*
 * Side Effects:							*
 *	Prints messages or writes them to the LOADMAP file.
 *									*
 ************************************************************************/
RETCODE
mismatch(char *arg[])			/* argv-style arguments */
{
    int		i;
    int		freeit;
    int		rc = 0;
    HASH_STR	*sroot, *sh;
    SYMBOL	*sym;
    STR		*s;

    if (Bind_state.interactive)
	freeit = 0;
    else
	freeit = 1;

    if (arg[1]) {
	lower(arg[1]);
	if (strcmp(arg[1], "free") == 0)
	    freeit = 1;
	else if (strcmp(arg[1], "nofree") == 0)
	    freeit = 0;
	else if (strcmp(arg[1], "freeonly") == 0)
	    goto free_resolve_names;
	/* Else ignore argument */
    }

    if (Bind_state.state & STATE_RESOLVE_NEEDED) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(RESOLVE_NEEDED,
		"%1$s: 0711-300 ERROR: RESOLVE must be called before calling binder command %2$s."),
		 Main_command_name, Command_name);
	return RC_NI_ERROR;
    }

    /* Do storage-mapping class and type-string checking. */
    for (sroot = &HASH_STR_root; sroot; sroot = sroot->next)
	for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
	    if (sh->s.flags & STR_RESOLVED)
		rc |= check_ext_refs(&sh->s, 0); /* Plain name */
	    if (sh->s.alternate) {
		if (sh->s.alternate->flags & STR_RESOLVED)
		    rc |= check_ext_refs(sh->s.alternate, 1); /* Dot name */
	    }
	}

    if (resolve_names_flags & ERR_SMCLASS_CHANGED)
	for (i = 0; i < resolve_names_count; i++)
	    if (resolve_names[i].flags & ERR_SMCLASS_CHANGED) {
		s = resolve_names[i].name;
		sym = s->first_ext_sym;
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(SMC_CHANGE2,
	"%1$s: 0711-323 WARNING: Storage-mapping class of symbol %2$s\n"
	"\tchanged from XMC_UA to XMC_%3$s because of references."),
			 Main_command_name,
			 sym->s_name->name,
			 get_smclass(sym->s_smclass));
	    }

    if (rc == 0)
	say(SAY_NORMAL, NLSMSG(MISMATCH_NONE, "%s: No type mismatches exist."),
	    Command_name);

    if (freeit == 1) {
      free_resolve_names:
	free_resolve_names();
    }

    return rc == 0 ? RC_OK : RC_ERROR;
} /* mismatch */
/***************************************************************************
 * Name:	free_resolve_names
 *
 * Purpose: Free resolve_names array.  If required, this routine could also
 *	clear the STR_ERROR bit in all STRs and set str_value to 0.
 *
 * Returns:	Nothing
 *
 * *************************************************************************/
void
free_resolve_names(void)
{
    int i;

    if (resolve_names != NULL) {
#if 0
	for (i = 0; i < resolve_names_count; i++) {
	    resolve_names[i].name->flags &= ~STR_ERROR;
	    resolve_names[i].name->str_value = 0;
	}
#endif
	efree(resolve_names);
	resolve_names = NULL;
	resolve_names_count = 0;
    }
}
/***************************************************************************
 * Name:	comprldx
 *
 * Purpose: Combines r1 and r2, RLDs at the same address, into a single RLD, if
 *		 possible.
 *	RLDs can be combined if one if R_POS and the other is R_NEG and:
 *	a) The rlds reference the same symbol
 *		(the result is R_REF);
 *	b) The R_NEG RLD refers to the CSECT containing both RLDs
 *		(the result is R_REL);
 *	c) The R_NEG RLD refers to the TOC anchor and R_POS is a symbol in the
 *		same section as the TOC anchor
 *		(the result is R_TOC);
 *
 *	NOTE:  In case (b), we could also combine the RLDs if the R_NEG RLD
 *		refers to a symbol not at the beginning of its CSECT, but this
 *		would require changing the code to adjust for the offset of
 *		the SYMBOL within its CSECT.
 *
 * Returns:	NULL, if RLDs cannot be combined
 *		r1's current successor, otherwise.  Since r1's original
 *			successor can be deleted, its new successor is returned
 *			here to indicate that RLDs were combined.
 * *************************************************************************/
static RLD *
comprldx(CSECT *cs,			/* Csect containing the RLDs */
	 RLD *r1,			/* RLDs to combine, if possible. */
	 RLD *r2)
{
    RLD		*r1_next, *rx;
    SYMBOL	*sym_NEG;
    CSECT	*cs_NEG;

    /* NOTE: r1 must precede r2 in chain of RLD items for csect cs */
    if (r1->r_reltype == R_POS && r2->r_reltype == R_NEG) {
	sym_NEG = r2->r_sym;

	if (r1->r_sym == sym_NEG) {
	    r1->r_reltype = R_REF;
	    goto delete_r2;
	}
	else {
	    cs_NEG = sym_NEG->s_csect;
	    if (cs_NEG == r1->r_csect && cs_NEG->c_addr == sym_NEG->s_addr) {
		r1->r_reltype = R_REL;
		goto delete_r2;
	    }
	    else if (sym_NEG->s_smclass == XMC_TC0
		     && r1->r_sym->s_csect->c_secnum == cs_NEG->c_secnum) {
		r1->r_reltype = R_TOC;

	      delete_r2:
		for (rx = r1; rx; rx = rx->r_next) {
		    if (rx->r_next == r2) {
			rx->r_next = r2->r_next;
			/*free_rld(r2);*/
			return r1->r_next;
		    }
		}
		internal_error();
	    }
	}
    }
    if (r2->r_reltype == R_POS && r1->r_reltype == R_NEG) {
	sym_NEG = r1->r_sym;

	if (sym_NEG == r2->r_sym) {
	    r1->r_reltype = R_REF;
	    goto delete_r2;
	}
	else {
	    cs_NEG = sym_NEG->s_csect;
	    if (cs_NEG == r2->r_csect && cs_NEG->c_addr == sym_NEG->s_addr) {
		r2->r_reltype = R_REL;
		goto delete_r1;
	    }
	    else if (sym_NEG->s_smclass == XMC_TC0
		     && r2->r_sym->s_csect->c_secnum == cs_NEG->c_secnum) {
		r2->r_reltype = R_TOC;

	      delete_r1:
		r1_next = r1->r_next;
		if (r1 == cs->c_first_rld) {
		    cs->c_first_rld = r1->r_next;
		    /*free_rld(r1);*/
		}
		else {
		    for (rx = cs->c_first_rld; rx; rx = rx->r_next) {
			if (rx->r_next == r1) {
			    rx->r_next = r1->r_next;
			    /*free_rld(r1);*/
			    break;
			}
		    }
		}
		return r1_next;
	    }
	}
    }
    return NULL;
} /* comprldx */
/************************************************************************
 * Name: comprld		Processor for the COMPRLD command	*
 *									*
 * Purpose: Command processor for the COMPRLD command.  Combines	*
 *	multiple RLDs at the same address where appropriate.  See
 *	comprldx() for details.
 *									*
 * Command Format:							*
 *	COMPRLD								*
 *									*
 * Arguments:	none							*
 *									*
 * Returns: Returns a status completion code.				*
 *	OK	- no error detected.					*
 *	RC_IN_ERROR - "resolve" not called yet.
 *									*
 * Side Effects:							*
 *	Causes messages to be displayed or placed in the LOADMAP file	*
 *									*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
comprld(char *arg[])			/* argv-style arguments */
{
    OBJECT 		*obj;
    SRCFILE	*sf;
    CSECT		*cs;
    RLD		*r1, *r2, *temp_r, *r1_next;
    int		rld_deleted = 0;
    int		rld_checked = 0;

    if (Bind_state.state & STATE_RESOLVE_NEEDED) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(RESOLVE_NEEDED,
		"%1$s: 0711-300 ERROR: RESOLVE must be called before calling binder command %2$s."),
		 Main_command_name, Command_name);
	return RC_NI_ERROR;
    }

    for (obj = first_object(); obj; obj = obj->o_next) {
	if (obj->o_type != O_T_OBJECT)
	    continue;
	for (sf = obj->oi_srcfiles; sf; sf = sf->sf_next)
	    for (cs = sf->sf_csect; cs; cs = cs->c_next)
		if (cs->c_mark)
		    for (r1 = cs->c_first_rld; r1; r1 = r1_next) {
			rld_checked++;
			r1_next = r1->r_next;
			for (r2 = r1->r_next;
			     r2 && r1->r_addr == r2->r_addr;
			     r2 = r2->r_next) {
			    if (r1->r_length == r2->r_length) {
				temp_r = comprldx(cs, r1, r2);
				if (temp_r) {
				    rld_deleted++;
				    /* Previous r1_next could have just been
				       deleted. */
				    r1_next = temp_r;
				    break;
				}
			    }
			}
		    }
    }

    say(SAY_NORMAL,
	NLSMSG(CMPCTS_REL, "%1$s: Kept %2$d of %3$d relocation entries."),
	Command_name, rld_checked - rld_deleted, rld_checked);
    return RC_OK;
} /* comprld */
#ifdef DEBUG
/************************************************************************
 * Name: gc
 *									*
 * Purpose: Command processor for the GC command.
 *									*
 * Command Format:							*
 *	gc								*
 *									*
 * Arguments:	none							*
 *									*
 * Returns: Returns a status completion code.				*
 *	OK	- no error detected.					*
 *	WARNING - "resolve" not called yet.
 *	ERR	- Multiple entries for $TOC				*
 *									*
 ************************************************************************/
/*ARGSUSED*/
RETCODE
gc(char *arg[])
{
    int		i, j;
    int		syms_deleted = 0;
    int		syms_kept = 0;
    CSECT	*cs, *prev_cs, *cs_next;
    HASH_STR	*sroot, *sh;
    OBJECT	*obj;
    SRCFILE	*sf;
    STR		*s;
    SYMBOL	*sym, *prev_sym, *sym_next;

#ifdef DEBUG
    if (!(bind_debug & DO_GC_DEBUG))
	return ignored(arg);
#endif

    if (Bind_state.state & STATE_RESOLVE_NEEDED) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(RESOLVE_NEEDED,
		"%1$s: 0711-300 ERROR: RESOLVE must be called before calling binder command %2$s."),
		 Main_command_name, Command_name);
	return RC_NI_ERROR;
    }

    for (obj = first_object(); obj; obj=obj->o_next) {
	switch(obj->o_type) {
	  case O_T_OBJECT:
	    sf = obj->oi_srcfiles;
	    break;
	  case O_T_SHARED_OBJECT:
	  case O_T_SCRIPT:
	    sf = obj->o_srcfiles;
	    break;
	}
	for ( ; sf; sf = sf->sf_next) {
	    prev_cs = NULL;
	    for (cs = sf->sf_csect; cs; cs = cs_next) {
		cs_next = cs->c_next;
		if (cs->c_mark) {
		    if (prev_cs == NULL)
			sf->sf_csect = cs;
		    else
			prev_cs->c_next = cs;
		    prev_cs = cs;
		    cs->c_major_sect = 0; /* Initialize */
		}
		else {
		    for (sym = cs->c_symbol.s_next_in_csect;
			 sym;
			 sym = sym->s_next_in_csect) {
			if (obj->o_type == O_T_OBJECT
			    || !(sym->s_flags & S_MARK)) {
#ifdef DEBUG
			    say(SAY_NO_NLS, "Deleting %s", sym->s_name->name);
#endif
			    sym->s_name = NULL;	/* Mark for deletion */
			}
		    }
		    cs->c_major_sect = 255; /* Mark for deletion */
		}
	    }
	    if (prev_cs == NULL)
		sf->sf_csect = NULL;
	}
    }

    /* Now go through all strings and free the deleted symbols */
    for_all_STRs(sroot, i, sh, s) {
	for (j = 0; j < 3; j++) {
	    prev_sym = NULL;
	    for (sym = s->str_syms[j]; sym; sym = sym_next) {
		sym_next = sym->s_synonym;
		if (sym->s_name != NULL) {
		    ++syms_kept;
		    /* Not deleted */
		    if (prev_sym)
			prev_sym->s_synonym = sym;
		    else
			s->first_ext_sym = sym;
		    prev_sym = sym;
		}
		else if (!(sym->s_flags & S_PRIMARY_LABEL))
		    /* free_sym(sym) */
		    ++syms_deleted;
	    }
	    if (prev_sym == NULL)
		s->str_syms[j] = NULL;
	}
	if (s->first_ext_sym == NULL && s->first_hid_sym == NULL
	    && s->refs == NULL
	    && !(s->flags & (STR_KEEP | STR_EXPORT)))
	    s->flags &= ~STR_ISSYMBOL;
    }
    /* Now go through the symbols with a null name */
    for (j = 0; j < 3; j++) {
	prev_sym = NULL;
	for (sym = NULL_STR.str_syms[j]; sym; sym = sym_next) {
	    sym_next = sym->s_synonym;
	    if (sym->s_name != NULL) {
		++syms_kept;
		/* Not deleted */
		if (prev_sym)
		    prev_sym->s_synonym = sym;
		else
		    s->first_ext_sym = sym;
		prev_sym = sym;
	    }
	    else if (!(sym->s_flags & S_PRIMARY_LABEL)) {
		/* free_sym(sym) */
		++syms_deleted;
	    }
	}
	if (prev_sym == NULL)
	    NULL_STR.str_syms[j] = NULL;
    }
    if (s->first_ext_sym == NULL && s->first_hid_sym == NULL && s->refs == NULL
	&& !(s->flags & (STR_KEEP | STR_EXPORT)))
	s->flags &= ~STR_ISSYMBOL;

    say(SAY_NORMAL,
	NLSMSG(CMPCTS_SYMS_GC,
	       "%1$s: Number of symbols garbage collected: %2$d of %3$d"),
	Command_name, syms_deleted, syms_deleted + syms_kept);

    return RC_OK;
} /* gc */
#endif
/***************************************************************************
 * Name:	create_descriptor
 *
 * Purpose:	Create a descriptor with name 'plain_name' corresponding to
 *		an existing label .plain_name.  The descriptor will have
 *		references to the .plain_name (as an ER) and to the TOC anchor.
 *
 *		Three symbols are generated:  the descriptor, a TOC anchor,
 *			and an ER to .foo
 *
 * Results:	The descriptor symbol is returned.
 *
 * *************************************************************************/
static SYMBOL *
create_descriptor(STR *plain_name,	/* Name of descriptor to generate. */
		  SYMBOL *prev_plain_sym, /* Previous symbol with the same
					     name as descriptor. */
		  SYMBOL *dot_sym,	/* Symbol with name = .plain_name. It
					   will have s_smclass = XMC_PR. */
		  OBJECT *obj)		/* Object file containing dot_sym. */
{
    CSECT	*dot_cs = dot_sym->s_csect;
    CSECT	*cs;
    RLD		*rld;
    STR		*toc_name, *dot_name;
    SYMBOL	*sym0, *sym1, *er;

    if (Switches.verbose)
	say(SAY_NORMAL,
	    NLSMSG(GENERATE_DESC, "%1$s: Generating descriptor: %2$s"),
	    Command_name, plain_name->name);

    /* Add a descriptor CSECT of type XMC_DS referring to a CSECT of type
       XMC_PR. We must generate a new TOC anchor and an ER referring to the
       .foo symbol, to make sure relocation works properly.

       3 symbols generated:  A descriptor, a TOC anchor, and an ER to .foo;
       2 RLDs generated. */
    cs = get_CSECTs(2);
    sym0 = &cs[0].c_symbol;
    sym1 = &cs[1].c_symbol;
    er = get_ER();
    Bind_state.generated_symbols += 3;
    rld = get_RLDs(2);
    STAT_use(RLDS_ID, 2);
    plain_name->flags |= STR_ISSYMBOL;

    toc_name = putstring(TOC_NAME);
    toc_name->flags |= STR_ISSYMBOL;
    dot_name = dot_sym->s_name;

    cs[0].c_next = &cs[1];
    cs[1].c_next = dot_cs->c_next;
    dot_cs->c_next = cs;

    /* Fill in remaining fields of first CSECT (and its SYMBOL). */
    cs[0].c_len = 12;
    cs[0].c_TD_ref = 0;
    cs[0].c_save = 0;
    cs[0].c_mark = 0;
    cs[0].c_align = 2;
    cs[0].c_secnum = N_GENERATED;
    cs[0].c_first_rld = rld;
    cs[0].c_addr = 0;
    cs[0].c_srcfile = dot_cs->c_srcfile;

    sym0->s_name = plain_name;
    sym0->s_next_in_csect = NULL;
    sym0->s_addr = 0;
    sym0->s_inpndx = INPNDX_GENERATED;
    sym0->s_smclass = XMC_DS;
    sym0->s_smtype = XTY_SD;
    /* Preserve S_ARCHIVE_SYMBOL flag */
    sym0->s_flags = S_PRIMARY_LABEL
	| (dot_sym->s_flags & S_ARCHIVE_SYMBOL);

    sym0->s_csect = &cs[0];
    sym0->s_typechk = dot_sym->s_typechk; /* Copy typechk */

    /* Insert new symbol in proper position in its name chain. */
    if (prev_plain_sym == NULL) {
	/* New symbol is first in chain. */
#ifdef DEBUG
	if (bind_debug & SYMBOLS_DEBUG)
	    if (plain_name->first_ext_sym != NULL)
		say(SAY_NO_NLS, "Duplicate symbol (in create_descriptor) %s",
		    plain_name->name);
#endif
	sym0->s_synonym = plain_name->first_ext_sym;
	plain_name->first_ext_sym = sym0;
    }
    else {
	DEBUG_MSG(SYMBOLS_DEBUG,
	    (SAY_NO_NLS, "Duplicate symbol (in create_descriptor) %s",
	     plain_name->name));
	sym0->s_synonym = prev_plain_sym->s_synonym;
	prev_plain_sym->s_synonym = sym0;
    }

    /* Fill in remaining fields of second CSECT (and its SYMBOL). */
    cs[1].c_len = 0;
    cs[1].c_TD_ref = 0;
    cs[1].c_save = 0;
    cs[1].c_mark = 0;
    cs[1].c_align = 2;
    cs[1].c_secnum  = N_GENERATED;
    cs[1].c_first_rld = NULL;
    cs[1].c_addr = 0;
    cs[1].c_srcfile = dot_cs->c_srcfile;

    sym1->s_name = toc_name;
    sym1->s_next_in_csect = NULL;
    sym1->s_addr = 0;
    sym1->s_inpndx = INPNDX_GENERATED;
    sym1->s_smclass = XMC_TC0;
    sym1->s_smtype = XTY_SD;
    sym1->s_flags = S_PRIMARY_LABEL | S_HIDEXT;
    sym1->s_csect = &cs[1];
    sym1->s_resolved = NULL;

    /* Insert new TOC anchor in chain for hidden symbols named 'TOC' */
    sym1->s_synonym = toc_name->first_hid_sym;
    toc_name->first_hid_sym = sym1;

    /* File in remaining fields of ER */
    er->er_name = dot_name;
    er->s_addr = 0;
    er->er_inpndx = INPNDX_GENERATED;
    er->er_smclass = XMC_PR;
    er->er_object = obj;
    er->er_typechk = dot_sym->s_typechk;

    /* Link ER into external references in object */
    er->er_next_in_object = obj->oi_ext_refs;
    obj->oi_ext_refs = er;

    /* Link ER into list of references to dot_name symbols. */
    er->er_synonym = dot_name->refs;
    dot_name->refs = er;

    /* Fill in fields of RLDs. */
    rld[0].r_sym = er;			/* ER to code */
    rld[0].r_csect = &cs[0];
    rld[0].r_next = &rld[1];
    rld[0].r_addr = 0;
    rld[0].r_length = 32;
    rld[0].r_reltype = R_POS;
    rld[0].r_flags = RLD_RESOLVE_BY_NAME | RLD_EXT_REF;

    rld[1].r_sym = sym1;		/* TOC */
    rld[1].r_csect = &cs[0];
    rld[1].r_next = NULL;
    rld[1].r_addr = 4;
    rld[1].r_length = 32;
    rld[1].r_reltype= R_POS;
    rld[1].r_flags = 0;

    return sym0;			/* Return descriptor */
} /* create_descriptor */
/************************************************************************
 * Name: create_generated_import_SYMBOL
 *
 * Purpose: Generate an imported symbol associated with another imported
 *		symbol of the same name, except for the dot.  The new symbol
 *		will be in the same csect as existing_sym.
 *
 *	If existing_sym is XMC_XO, the alternate_name can be either
 *		a plain_name or a dot_name.  The generated symbol will also
 *		be XMC_XO and will have the same address.
 *	If existing_sym is not XMC_XO, the alternate_name can be either
 *		a plain_name or a dot_name.
 *		A generated dot_name gets storage-mapping class XMC_PR.
 *		A generated plain_name gets storage-mapping class XMC_DS.
 *
 * Returns: The generated symbol.
 *
 ************************************************************************/
static SYMBOL *
create_generated_import_SYMBOL(STR *alternate_name, /* Name for new symbol */
			       SYMBOL *prev_alternate_sym,
					/* Previous instance of a symbol named
					   alternate_name (or NULL if none). */
			       SYMBOL *existing_sym) /* Existing symbol */
{
    SYMBOL *sym;

#ifdef DEBUG
    if (!imported_symbol(existing_sym))
	internal_error();
#endif

    if (Switches.verbose)
	say(SAY_NORMAL,
	    NLSMSG(IMPSS_IMP,
		   "%1$s: Generating implicitly imported symbol: %2$s"),
	    Command_name, alternate_name->name);

    sym = get_SYMBOLs(1);
    ++Bind_state.generated_symbols;

    sym->s_name = alternate_name;
    alternate_name->flags |= STR_ISSYMBOL;

    /* Insert into chain of symbols in the current csect. */
    sym->s_next_in_csect = existing_sym->s_next_in_csect;
    existing_sym->s_next_in_csect = sym;

    /* Insert new symbol in proper position in its name chain. */
    if (prev_alternate_sym == NULL) {
	/* New symbol is first in chain. */
#ifdef DEBUG
	if (bind_debug & SYMBOLS_DEBUG)
	    if (alternate_name->first_ext_sym != NULL)
		say(SAY_NO_NLS,
		    "Duplicate symbol (in create_generated_import_SYMBOL) %s",
		    alternate_name->name);
#endif
	sym->s_synonym = alternate_name->first_ext_sym;
	alternate_name->first_ext_sym = sym;
    }
    else {
	DEBUG_MSG(SYMBOLS_DEBUG,
		  (SAY_NO_NLS,
		   "Duplicate symbol (in create_generated_import_SYMBOL) %s",
		   alternate_name->name));
	sym->s_synonym = prev_alternate_sym->s_synonym;
	prev_alternate_sym->s_synonym = sym;
    }

    if (existing_sym->s_smclass == XMC_XO) {
	sym->s_addr = existing_sym->s_addr;
	sym->s_smclass = XMC_XO;
    }
    else {
	sym->s_addr = 0;
	if (existing_sym->s_name->name[0] == '.')
	    sym->s_smclass = XMC_DS;
	else
	    sym->s_smclass = XMC_PR;
    }
    sym->s_inpndx = INPNDX_GENERATED;	/* Generated symbol */
    sym->s_smtype = existing_sym->s_smtype; /* Retain symbol type */

    /* Preserve S_ARCHIVE_SYMBOL and S_XMC_XO flags, but no others */
    sym->s_flags = existing_sym->s_flags & (S_ARCHIVE_SYMBOL | S_XMC_XO);
    sym->s_csect = existing_sym->s_csect;
    sym->s_typechk = existing_sym->s_typechk; /* Retain typchk */

    return sym;
} /* create_generated_import_SYMBOL */
/***************************************************************************
 * Name:	find_or_create_toc_entry
 *
 * Purpose:	Create a toc entry for symbol with external name 'sym', if
 *		one was not already generated.  If necessary, one symbol
 *		is generated:  the TOC symbol with a reference to the ER.
 *
 * Results:	If no symbol is created, NULL is returned.  Otherwise, the
 *		generated symbol is passed as a parameter to
 *		resolve_hidden_symbol(), and the result is returned.
 *
 * *************************************************************************/
static SYMBOL *
find_or_create_toc_entry(RLD *old_rld,
			 CSECT *prev_cs, /* CSECT containing original ref. */
			 const int visited_bit)	/* If 0, we must create sym */
{
    SYMBOL	*old_sym = old_rld->r_sym; /* Original referenced symbol.  This
					      is usually an ER, but could be a
					      regular symbol if an imported
					      symbol is replacing an existing
					      definition. */
    CSECT	*cs;
    SYMBOL	*sym;
    RLD		*rld;

    if (visited_bit) {
	/* The referenced symbol has already been visited, and if an earlier
	   visit was for a TOC-relative RLD, the TC symbol will have already
	   been generated. Look for it. */
	for (sym = old_sym->s_name->first_hid_sym; sym; sym = sym->s_synonym) {
	    if (sym->s_inpndx == INPNDX_GENERATED
		  && sym->s_smclass == XMC_TC
		  && sym->s_csect->c_first_rld
		  && sym->s_csect->c_first_rld->r_sym == old_sym) {
		/* Found it */
		old_rld->r_sym = sym;	/* Fudge the RLD to point to the
					   generated TOC entry. */
#ifdef DEBUG
		if ((old_rld->r_flags & (RLD_RESOLVE_BY_NAME | RLD_EXT_REF))
		    != (RLD_RESOLVE_BY_NAME | RLD_EXT_REF))
		    internal_error();
#endif
		old_rld->r_flags &= ~(RLD_RESOLVE_BY_NAME | RLD_EXT_REF);
		return NULL;		/* No symbol generated */	}
	}
    }

    if (Switches.verbose)
	say(SAY_NORMAL,
	    NLSMSG(GENERATE_TOC_ENTRY, "%1$s: Generating TOC entry for %2$s"),
	    Command_name, old_sym->s_name->name);

    cs = get_CSECTs(1);
    sym = &cs->c_symbol;
    old_rld->r_sym = sym;
    ++Bind_state.generated_symbols;
    rld = get_RLDs(1);
    STAT_use(RLDS_ID, 1);

    cs->c_next = prev_cs->c_next;
    prev_cs->c_next = cs;

    /* Fill in remaining fields of first CSECT (and its SYMBOL). */
    cs->c_len = 4;
    cs->c_TD_ref = 0;
    cs->c_save = 0;
    cs->c_mark = 0;
    cs->c_align = 2;
    cs->c_secnum = N_GENERATED;
    cs->c_first_rld = rld;
    cs->c_addr = 0;
    cs->c_srcfile = prev_cs->c_srcfile;

    sym->s_name = old_sym->s_name;
    sym->s_next_in_csect = NULL;
    sym->s_addr = 0;
    sym->s_inpndx = INPNDX_GENERATED;
    sym->s_smclass = XMC_TC;
    sym->s_smtype = XTY_SD;
    sym->s_flags = S_PRIMARY_LABEL | S_HIDEXT;
    sym->s_csect = cs;
    sym->s_resolved = NULL;

    /* Insert new TOC symbol in chain for hidden symbols with its name. */
    sym->s_synonym = old_sym->s_name->first_hid_sym;
    old_sym->s_name->first_hid_sym = sym;

    /* Fill in fields of RLDs. */
    rld->r_sym = old_sym;
    rld->r_csect = cs;
    rld->r_next = NULL;
    rld->r_addr = 0;
    rld->r_length = 32;
    rld->r_reltype = R_POS;

#ifdef DEBUG
    if ((old_rld->r_flags & (RLD_RESOLVE_BY_NAME | RLD_EXT_REF))
	!= (RLD_RESOLVE_BY_NAME | RLD_EXT_REF))
	internal_error();
#endif
    rld->r_flags = RLD_RESOLVE_BY_NAME | RLD_EXT_REF;
    old_rld->r_flags &= ~(RLD_RESOLVE_BY_NAME | RLD_EXT_REF);
    return resolve_hidden_symbol(sym);
} /* find_or_create_toc_entry */
