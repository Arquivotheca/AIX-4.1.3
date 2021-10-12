#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)94	1.24  src/bos/usr/ccs/bin/ld/bind/glue.c, cmdld, bos41J, 9508A 2/13/95 08:45:09")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS FOR BINDER COMMANDS:
 *		addglue
 *		addnamedglue
 *
 *   STATIC FUNCTIONS:
 *		add_glue_for_name
 *		addgl
 *		copy_sym_files
 *		finish_glue
 *		setup_glue
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>

#include "bind.h"
#include "error.h"
#include "global.h"
#include "symbols.h"
#include "match.h"
#include "objects.h"
#include "ifiles.h"
#include "insert.h"
#include "strs.h"
#include "resolve.h"
#include "stats.h"

/* Static variables */
static STR *glue_plain_name, *glue_dot_name; /* Names for symbol for which
						glue code is being added. */
/* The following STR is a dummy used to point to all symbols read from the
   glink prototype file. */
static STR glue_name = {NULL, {NULL, NULL, NULL}, NULL, 0, 0, 0};
static STR *const glue_name_ptr = &glue_name;

static int	glue_found;
static int	cs_count, sym_count, rld_count, er_count;
static HEADERS	*glue_hdr;		/* For glink prototype file */
static IFILE	*glue_ifile;		/* For glink prototype file */
static OBJECT	*glue_object;		/* For glink prototype file */
static SRCFILE	**prev_sf_slot, *glue_sf;
static STR	*pointer_glue_name;	/* STR for _ptrgl */
static SYMBOL	**prev_er_slot, *glue_ext_refs;

/* Bit definitions for the ADDGL command */
#define NORMAL_GLUE 1
#define MILICODE_GLUE 2
#define UNDEFINED_GLUE 4
#define EXPORTS_GLUE 8
#define LOCAL_GLUE 16
#define ANY_GLUE 64

/* Define choices for ADDGL command */
static const struct {
    char *glue_type;
    int glue_flag;
} glue_types[] = {
    {"normal",		NORMAL_GLUE},
    {"local",		LOCAL_GLUE},
    {"milicode",	MILICODE_GLUE},
    {"undefined",	UNDEFINED_GLUE},
    {"exports",		EXPORTS_GLUE}
};
static const int num_glue_types = sizeof(glue_types)/sizeof(glue_types[0]);

/* Global variables */
SYMBOL		*glue_ext_symbol;	/* One and only external symbol
					   defined in the glink file. Each
					   invocation of ADDGL or
					   ADDNAMEDGLUE can use a different
					   glue file. */

/* Forward declarations */
static OBJECT	*setup_glue(IFILE *, HEADERS *);
static RETCODE	addgl(char *, char *, const int);
static SYMBOL	*copy_sym_files(OBJECT *, STR *);

/************************************************************************
 * Name: addglue		Processor for the ADDGLUE command
 *									*
 * Purpose: Insert glue code for certain types of symbols.
 *									*
 * Command Format:							*
 *	ADDGL filename [glue-types]
 *									*
 * Arguments:								*
 *	filename - File that contains linkage code			*
 *	glue-types: normal, exports, local, undefined, milicode
 *	If glue-types is not specified, "normal undefined" is used.
 *
 * Returns:	See addgl()
 ************************************************************************/
RETCODE
addglue(char *args[])			/* argv-style arguments */
{
    return addgl(args[1], NULL, 0);
}
/************************************************************************
 * Name: addnamedglue		Processor for the ADDGLUE command
 *									*
 * Purpose: Insert glue code for certain named symbols
 *									*
 * Command Format:							*
 *	ADDnamedGLUE filename patterns ...
 *									*
 * Arguments:								*
 *	filename - File that contains linkage code			*
 *									*
 * Returns:	See addgl()
 ************************************************************************/
RETCODE
addnamedglue(char *args[])			/* argv-style arguments */
{
    return addgl(args[1], args[2], 1);
}
/************************************************************************
 * Name: add_glue_for_name
 *									*
 * Purpose: Insert linkage code for name "s".
 *									*
 * Arguments:								*
 *	s (symbol_name):Glue can only be added for a dot name, so if
 *			a plain name is specified, the dot name is used.
 *			The ISSYMBOL flag must be set for the name.
 *									*
 * Returns:	status completion code.					*
 *	RC_OK	- No errors or a minor error.  When a minor error occurs
 *		(such as the symbol with the given name was not referenced),
 *		RC_OK is returned to allow other symbols to be processed.
 *	RC_WARNING - An error occurred with the glue file, so no symbols
 *		can be processed.
 ************************************************************************/
static RETCODE
add_glue_for_name(STR *dot_name)
{
    int		doing_unresolved;
    SYMBOL	*external_symbol;

    /* We only handle dot-names, that is, names beginning with a single dot. */
    if (!(dot_name->name[0] == '.' && dot_name->name[1] != '.')) {
	/* Symbol isn't a dot-name, so we don't need to add glue for it. */
	return RC_OK;
    }

    if (!(dot_name->flags & STR_RESOLVED))
	return RC_OK;			/* If not resolved, no references */

    if (dot_name == pointer_glue_name) {
	bind_err(SAY_NORMAL, RC_ERROR,
		 NLSMSG(GLUE_PTRGL_NO,
	"%1$s: 0711-362 ERROR: Glink code cannot be added for function: %2$s\n"
	"\tThis function is used by compilers and must be called directly."),
		 Main_command_name, &dot_name->name[1]);
	return RC_OK;			/* Return OK so match() will continue
					   to find other matches. */
    }

    if (dot_name->flags & STR_NO_DEF) {
	/* dot_name is not defined */
	/* Don't add glue if plain_name is defined and is not a descriptor. */
	if ((dot_name->alternate->flags & (STR_RESOLVED | STR_NO_DEF))
	    == STR_RESOLVED
	    && dot_name->alternate->first_ext_sym->s_smclass != XMC_DS)
	    return RC_OK;
	doing_unresolved = 1;
    }
    else {
	if (dot_name->first_ext_sym->s_smclass != XMC_PR)
	    return RC_OK;		/* Don't add glue for non-XMC_PRs--
					   among other things, this keeps us
					   from adding glue twice for the
					   same symbol. */
	doing_unresolved = 0;
    }

    /* First time only for this invocation of ADDNAMEDGLUE
       --read the glue OBJECT */
    if (glue_ext_symbol == NULL)
	if ((glue_object = setup_glue(glue_ifile, glue_hdr)) == NULL) {
	    /* The subcommand error level will have been set by a
	       bind_error() call in setup_glue().  We return any non-RC_OK
	       value to prevent the processing of any additional symbols. */
	    return RC_WARNING;
	}

    if (Switches.verbose)
	say(SAY_NO_NLS, "%s: %s", Command_name, dot_name->name);
    ++glue_found;

    external_symbol = copy_sym_files(glue_object, dot_name);
    if (external_symbol) {
	/* Standard glue code has a reference to a plain_name
	   descriptor, which will may be unresolved if we're adding
	   glue_code for unresolved symbols.  In this case, we check
	   whether plain_name is otherwise unreferenced, and after
	   calling dfs(), reset the ERR_UNRESOLVED bit if so. */
	if (doing_unresolved) {
	    int bit;
	    if (dot_name->alternate->flags & STR_ERROR)
		bit = resolve_names[dot_name->alternate->str_value].flags
		    & ERR_UNRESOLVED;
	    else
		bit = 0;
	    dfs(external_symbol);
	    if (!bit
		&& (dot_name->alternate->flags & STR_ERROR))
		/* Reset ERR_UNRESOLVED */
		resolve_names[dot_name->alternate->str_value].flags
		    &= ~ERR_UNRESOLVED;
	}
	else
	    dfs(external_symbol);
    }
    return RC_OK;
} /* add_glue_for_name */
/************************************************************************
 * Name: finish_glue
 *									*
 * Purpose:	Print any errors resulting from the addition of glue code.
 *									*
 ************************************************************************/
static void
finish_glue(int found)
{
    if (resolve_names_flags & (ERR_DUPLICATE | ERR_TOC_RESOLVE
			       | ERR_CM_BUMP | ERR_ALIGN_BUMP)) {
	DEBUG_MSG(RESOLVE_DEBUG,
		  (SAY_NO_NLS,
		   "Calling display_resolve_errors from addgl"));
	display_resolve_errors(1);
    }

    if (found > 0)
	Bind_state.num_glue_symbols += found * glue_object->oi_num_symbols;

    say(SAY_NORMAL,
	NLSMSG(GLUE_COUNT, "%1$s: Glink code added for %2$d symbols."),
	Command_name, found);

    /* Clear out the pointers in the dummy name anchor 'glue_name_ptr'
       in case glue is added again. */
    glue_name.str_syms[0]
	= glue_name.str_syms[1]
	    = glue_name.str_syms[2] = NULL;

} /* finish_glue */
/************************************************************************
 * Name: addgl
 *									*
 * Purpose:	Add glue for symbols with a given name or for a set
 *		of names.
 *									*
 * Arguments:	fn:	Prototype glue code object file.
 *		first_pattern:	First pattern (for ADDNAMEDGLUE)
 *
 * Side Effects:	The external symbol from the glink code is searched
 *			as if resolve had been called again.
 *
 * Returns:	Subcommand return code
 ************************************************************************/
static RETCODE
addgl(char *fn,
      char *first_pattern,
      const int names)			/* If 1, addnamedglue was called */
{
    static char *id = "addgl";
    int		j, n, found;
    int		doing_unresolved = 0;
    IFILE	*ifile;
    HASH_STR	*sroot, *sh;
    STR		*dot_name;
    SYMBOL	*dot_sym, *external_symbol, *er;
    HEADERS	*hdr;
    char	*args[2];
    int		glue_mask;

    if (Bind_state.state & STATE_RESOLVE_NEEDED) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(RESOLVE_NEEDED,
 "%1$s: 0711-300 ERROR: RESOLVE must be called before calling binder command %2$s."),
		 Main_command_name, Command_name);
	return RC_NI_ERROR;
    }

    /* Now we insert the glue file.  This is almost identical to insert0()
       in insert.c.  The file is not actually read until glue code is needed
       for the first time. */
    if ((ifile = ifile_open_and_map(fn)) == NULL) {
	/* Error message already printed, so return code is already set. */
	return RC_OK;
    }
    if (set_ifile_type(ifile, &hdr) != I_T_OBJECT) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(GLUE_BAD_FILE,
			"%1$s: 0711-364 ERROR: Invalid glink code file: %2$s\n"
			"\tAn XCOFF file is needed. No glink code added."),
		 Main_command_name, fn);
	return RC_NI_ERROR;
    }

    /* Add ._ptrgl name to make sure we never add glue for it. */
    if (pointer_glue_name == NULL)
	pointer_glue_name = putstring("._ptrgl");

    glue_ext_symbol = NULL;		/* External symbol from glue code */
    found = 0;

    if (names == 1) {
	/* For ADDNAMEDGLUE, call the routine for each name specified */
	glue_ifile = ifile;
	glue_hdr = hdr;
	for (n = 1, args[0] = first_pattern; n > 0; n = moretokens(args, 1)) {
	    char *temp_arg;
	    int rc;
	    if (args[0][0] != '.' && args[0][0] != '*') {
		/* Add a . to beginning of pattern. */
		temp_arg = emalloc(strlen(args[0]) + 1 + 1, id);
		temp_arg[0] = '.';
		strcpy(&temp_arg[1], args[0]);
	    }
	    else
		temp_arg = args[0];
	    glue_found = 0;
	    rc = match(temp_arg, MATCH_NO_NEWNAME, MATCH_EXT | MATCH_ER,
		       add_glue_for_name);
	    found += glue_found;
	    if (temp_arg != args[0])
		efree(temp_arg);
	    if (rc != RC_OK)
		break;
	    if (glue_found == 0) {
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(ADDNAMEDGLUE_NO_MATCH,
	"%1$s: 0711-363 WARNING: %2$s: No function names match pattern %3$s"),
			 Main_command_name,
			 Command_name,
			 args[0]);
	    }
	}
	finish_glue(found);
	return RC_OK;
    }

    /* ADDGL command processing only after this. */

    /* Check for optional glue-type parameters. */
    glue_mask = 0;
    while (moretokens(args, 1) > 0) {
	for (j = 0; j < num_glue_types; j++)
	    if (strcmp(args[0], glue_types[j].glue_type) == 0) {
		glue_mask |= ANY_GLUE | glue_types[j].glue_flag;
		goto next_glue_type;
	    }

	bind_err(SAY_NO_NL, RC_NI_ERROR,
		 NLSMSG(GLUE_BAD_TYPE,
			"%1$s: 0711-365 ERROR: Unknown glink type: %2$s.\n"
			"\tChoices are:"),
		 Command_name, args[0]);
	for (j = 0; j < num_glue_types - 1; j++)
	    bind_err(SAY_NO_NLS | SAY_NO_NL, RC_NI_ERROR,
		     " %s", glue_types[j].glue_type);
	bind_err(SAY_NO_NLS, RC_NI_ERROR, " %s", glue_types[j].glue_type);
	return RC_NI_ERROR;
      next_glue_type:
	;
    }

    if (!(glue_mask & ANY_GLUE))
	glue_mask = NORMAL_GLUE | UNDEFINED_GLUE; /* Default */

    /* Now check all dot-names */
    for (sroot = &HASH_STR_root; sroot; sroot = sroot->next)
	for (j = 0, sh = sroot+1; j < sroot->s.len; sh++, j++) {
	    /* We need to look at all dot-names. */
	    if ((dot_name = sh->s.alternate) == NULL
		|| !(dot_name->flags & STR_RESOLVED))
		continue;		/* If dot-name symbol isn't resolved. */

	    if (dot_name->flags & STR_NO_DEF) {
		if ((glue_mask & UNDEFINED_GLUE)
		    && dot_name != pointer_glue_name) {
		    /* We expect all external references to a dot-name to have
		       storage-mapping class XMC_PR, but we add glue if at
		       least one external reference is appropriate. */
		    for (er = dot_name->refs; er; er = er->er_synonym)
			if (er->er_smclass == XMC_PR) {
			    if (Switches.verbose)
				say(SAY_NO_NLS, "%s: %s",
				    Command_name, dot_name->name);
			    doing_unresolved = 1;
			    goto doit;
			}
		    /* No glue if no XMC_PR ERs */
		}
		continue;
	    }

	    dot_sym = dot_name->first_ext_sym;
	    switch(dot_sym->s_smclass) {
	      case XMC_UA:		/* Check for symbol imported from file
					   as dot-name, not as plain name. */
		if (dot_sym->s_smtype != XTY_IF)
		    break;
	      case XMC_PR:
		/* Normal case--dot_sym is code */
		if (glue_mask & NORMAL_GLUE) {
		    if (imported_symbol(dot_sym)) {
			/* The pointer-glue code should never be imported,
			   but we check anyway. */
			if (dot_name == pointer_glue_name) {
			    bind_err(SAY_NORMAL, RC_ERROR,
				     NLSMSG(GLUE_PTRGL_NO,
    "%1$s: 0711-362 ERROR: Glink code cannot be added for function: %2$s\n"
    "\tThis function is used by compilers and must be called directly."),
				     Main_command_name,
				     &dot_name->name[1]);
			    continue;
			}
			if (Switches.verbose)
			    say(SAY_NO_NLS, "%s: %s",
				Command_name, dot_name->name);
			goto doit;
		    }
		}

		/* Special cases follow, but we never do the special cases
		   for the pointer glue name. */
		if (dot_name == pointer_glue_name)
		    continue;

		/* There are no special cases that add glue for
		   ordinary (non-XMC_XO) imported symbols. */
		if (imported_symbol(dot_sym))
		    continue;

		if (glue_mask & (EXPORTS_GLUE | LOCAL_GLUE)) {
		    /* LOCAL_GLUE implies not EXPORTS_GLUE, allowing
		       different kinds of glue for the two cases. */
		    if (dot_name->alternate->flags & STR_EXPORT) {
			if (glue_mask & EXPORTS_GLUE) {
			    if (Switches.verbose) {
				say(SAY_NO_NLS, "%s: %s",
				    Command_name, dot_name->name);
#ifdef DEBUG
				say(SAY_NO_NLS, "\t(exported symbol)");
#endif
			    }
			    goto doit;
			}
		    }
		    else {
			if (glue_mask & LOCAL_GLUE) {
			    if (Switches.verbose) {
				say(SAY_NO_NLS, "%s: %s",
				    Command_name, dot_name->name);
#ifdef DEBUG
				say(SAY_NO_NLS, "\t(added by local glue)");
#endif
			    }
			    goto doit;
			}
		    }
		}
		break;

	      case XMC_XO:
		if (glue_mask & MILICODE_GLUE) {
		    if (dot_name == pointer_glue_name) {
			bind_err(SAY_NORMAL, RC_ERROR,
				 NLSMSG(GLUE_PTRGL_NO,
	"%1$s: 0711-362 ERROR: Glink code cannot be added for function: %2$s\n"
	"\tThis function is used by compilers and must be called directly."),
				 Main_command_name,
				 &dot_name->name[1]);
			continue;
		    }
		    if (Switches.verbose) {
			say(SAY_NO_NLS, "%s: %s",
			    Command_name, dot_name->name);
#ifdef DEBUG
			say(SAY_NO_NLS, "\t(milicode symbol)");
#endif
		    }
		    goto doit;
		}
		break;
	    } /* switch */
	    continue;

	  doit:
	    /* First time only for this invocation of ADDGL
	       --read the glue OBJECT */
	    if (glue_ext_symbol == NULL)
		if ((glue_object = setup_glue(ifile, hdr)) == NULL)
		    return RC_OK;	/* Error level set by setup_glue() */

	    ++found;
	    external_symbol = copy_sym_files(glue_object, dot_name);
	    if (external_symbol) {
		/* Standard glue code has a reference to a plain_name
		   descriptor, which will may be unresolved if we're adding
		   glue_code for unresolved symbols.  In this case, we check
		   whether plain_name is otherwise unreferenced, and after
		   calling dfs(), reset the ERR_UNRESOLVED bit if so. */
		if (doing_unresolved) {
		    int bit;
		    if (dot_name->alternate->flags & STR_ERROR)
			bit = resolve_names[dot_name->alternate->str_value
					    ].flags & ERR_UNRESOLVED;
		    else
			bit = 0;
		    dfs(external_symbol);
		    if (!bit
			&& (dot_name->alternate->flags & STR_ERROR))
			/* Reset ERR_UNRESOLVED */
			resolve_names[dot_name->alternate->str_value].flags
			    &= ~ERR_UNRESOLVED;
		    doing_unresolved = 0;
		}
		else
		    dfs(external_symbol);
	    }
	} /* for All dot-names */

    finish_glue(found);
    return RC_OK;
} /* addgl */
/************************************************************************
 * Name: setup_glue
 *									*
 * Purpose: Read the glue code file and initialize data structures.
 *									*
 * Arguments: ifile:	The IFILE structure for the glue code
 *	      hdr:	The XCOFF header for the glue file
 *									*
 * Returns:	The OBJECT for the glue code if not errors.
 *		NULL otherwise.
 ************************************************************************/
static OBJECT *
setup_glue(IFILE *ifile,
	   HEADERS *hdr)
{
    CSECT	*cs;
    OBJECT	*o;
    RLD		*r;
    SRCFILE	*sf;
    SYMBOL	*er;
    SYMBOL	*sym;
    int		i;

    allocate_ifile_objects(ifile, hdr);
    o = ifile->i_objects;

    if (o == NULL) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(GLUE_BAD_FILE,
		"%1$s: 0711-364 ERROR: Invalid glink code file: %2$s\n"
		"\tAn XCOFF file is needed. No glink code added."),
		 Main_command_name, ifile->i_name->name);
	return NULL;
    }
    o->oi_flags |= OBJECT_GLUE;

    read_xcoff_symbols(o, 0, glue_name_ptr);

    /* Make sure:
       -A single external name is defined (glue_ext_symbol will be NULL if not)
       -The name starts with a single '.'
       -The storage-mapping class is XMC_PR or XMC_GL */
    if (glue_ext_symbol == NULL
	|| !(glue_ext_symbol->s_name->name[0] == '.'
	     && glue_ext_symbol->s_name->name[1] != '.')
	|| (glue_ext_symbol->s_smclass != XMC_PR
	    && glue_ext_symbol->s_smclass != XMC_GL)) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(GLUE_BAD_FILE,
			"%1$s: 0711-364 ERROR: Invalid glink code file: %2$s\n"
			"\tAn XCOFF file is needed. No glink code added."),
		 Main_command_name, ifile->i_name->name);
	return NULL;
    }
    glue_ext_symbol->s_smclass = XMC_GL; /* Glue code must be XMC_GL */

    glue_dot_name = glue_ext_symbol->s_name;
    glue_plain_name = glue_dot_name->alternate;
    for (i = 1; i <= o->oi_num_sections; i++)
	if (o->oi_section_info[i-1].l_reloc_count > 0)
	    read_section_rlds(o, i);

    /* Initialize variables that will be used by copy_sym_files(); */
    glue_sf = o->oi_srcfiles;
    glue_ext_refs = o->oi_ext_refs;
    prev_sf_slot = &o->oi_srcfiles;
    prev_er_slot = &o->oi_ext_refs;

    /* Count number of instances of SRCFILES, CSECTS, SYMBOLS,
       ERs, and RLDs, so that copy_sym_files can allocate
       the proper number. */
    cs_count = sym_count = rld_count = er_count = 0;
    for (sf = o->oi_srcfiles ; sf; sf = sf->sf_next) {
	for (cs = sf->sf_csect; cs; cs = cs->c_next) {
	    cs_count++;
	    for (sym=cs->c_symbol.s_next_in_csect;sym;sym=sym->s_next_in_csect)
		sym_count++;
	    for (r = cs->c_first_rld; r; r = r->r_next)
		rld_count++;
	}
    }
    for (er = o->oi_ext_refs; er; er = er->er_next_in_object)
	er_count++;

    return o;
} /* setup glue */
/************************************************************************
 * Name: copy_sym_files
 *									*
 * Purpose:	Replicate the symbols from the glue code prototype file
 *		for the specified symbol.  All symbols are on the STR
 *		chains for the dummy glue_name STR, but the symbols' real
 *		names are in the s_name fields, so they can be put in
 *		the proper place.
 *
 *		Because we don't use the STR chains, the s_synonym field in
 *		each prototype SYMBOL is used to save the address of the copied
 *		SYMBOL, so the RLD references can be replicated correctly.
 *									*
 * Arguments:
 *									*
 * Returns:
 ************************************************************************/
static SYMBOL *
copy_sym_files(OBJECT *obj,		/* Glue prototype object */
	       STR *dot_name)		/* Name of symbol needing glue code. */
{
    STR		*plain_name;
    SRCFILE	*sf,  *new_sf,  *first_new_sf;
    CSECT	*cs,  *new_cs,  **prev_cs_slot;
    SYMBOL	*sym, *new_sym, *prev_sym, *glue_replace_sym;
    SYMBOL	*er,  *new_er;
    RLD		*r,   *new_r,   **prev_r_slot;
    SYMBOL	*saved_ext_symbol = NULL;
    long	symbol_number;

    /* Allocate these structures all at once. */
    CSECT	*new_csects	= get_CSECTs(cs_count);
    SYMBOL	*new_symbols	= get_SYMBOLs(sym_count + er_count);
    RLD		*new_rlds	= get_RLDs(rld_count);

    plain_name = dot_name->alternate;	/* Must exist */

    /* Copy all SRCFILES in prototype glink object */
    for (sf = glue_sf, new_sf = get_init_SRCFILE(obj, sf->sf_name),
	 	first_new_sf = new_sf;
	 sf;
	 (sf = sf->sf_next)
	 	? new_sf = get_init_SRCFILE(obj, sf->sf_name)
	 	: NULL) {
	new_sf->sf_inpndx = sf->sf_inpndx;

	/* All the new SRCFILES are added to the list of SRCFILES for the glue
	   object.  The original SRCFILES are only reachable through glue_sf
	   now. */
	*prev_sf_slot = new_sf;
	prev_sf_slot = &new_sf->sf_next;
	new_sf->sf_next = NULL;
	prev_cs_slot = &new_sf->sf_csect;

	/* Copy all CSECTs in SRCFILE */
	for (cs = sf->sf_csect; cs; cs = cs->c_next) {
	    /* Copy csect, preserving the s_number from the contained SYMBOL */
	    symbol_number = new_csects->c_symbol.s_number;
	    *(new_cs = new_csects++) = *cs;
	    new_cs->c_symbol.s_number = symbol_number;

	    new_cs->c_srcfile = new_sf;
	    *prev_cs_slot = new_cs;
	    prev_cs_slot = &new_cs->c_next;
	    new_cs->c_next = NULL;
	    new_sym = &new_cs->c_symbol;

	    /* Copy all symbols in CSECT */
	    for (sym = &cs->c_symbol; sym; sym = sym->s_next_in_csect) {
		if (sym != &cs->c_symbol) {
		    /* Copy symbol, preserving the s_number field */
		    symbol_number = new_symbols->s_number;
		    *(new_sym = new_symbols++) = *sym;
		    new_sym->s_number = symbol_number;

		    /* Add symbol to csect chain */
		    prev_sym->s_next_in_csect = new_sym;
		}
		prev_sym = new_sym;
		new_sym->s_next_in_csect = NULL;
		new_sym->s_csect = new_cs;

		/* Save address of new instance of symbol, in case an RLD
		   entry refers to "sym". */
		sym->s_synonym = new_sym;

		if (sym == glue_ext_symbol) {
		    /* Here's the entry for the glue code--we use the same
		       name as the symbol we're adding glue code for, and add
		       the new symbol at the head of the chain of external
		       symbols. The previous head should be:
		       a) The generated label ".foo" if "foo" was imported;
		       b) The "real" label, if we're adding non-standard
		       glue code (such as export or local glue).
		       c) Undefined, if we're adding glue for
		       unresolved symbols */
		    new_sym->s_name = dot_name;

		    new_sym->s_synonym = dot_name->first_ext_sym;
		    dot_name->first_ext_sym = new_sym;

		    if (dot_name->flags & STR_NO_DEF) {
			/* Clear STR_NO_DEF flag for this name */
			dot_name->flags &= ~STR_NO_DEF;
		    }
		    else {
			glue_replace_sym = new_sym->s_synonym;
			new_sym->s_typechk = glue_replace_sym->s_typechk;
			if (glue_replace_sym->s_flags & S_TYPECHK_IMPLIED) {
			    glue_replace_sym->s_flags &= ~S_TYPECHK_IMPLIED;
			    glue_replace_sym->s_typechk = NULL;
			    new_sym->s_flags |= S_TYPECHK_IMPLIED;
			}
			if (glue_replace_sym->s_smtype == XTY_LD
			    || glue_replace_sym->s_smtype == XTY_SD)
			    new_sym->s_flags |= S_LOCAL_GLUE;
			else {
			    /* The symbol must be imported.  Will it be written
			       to the symbol table?? */
 			    glue_replace_sym->s_flags &= ~S_MARK;
			}

			/* Print replacement message, if appropriate */
			if (Switches.dbg_opt8) {
			    say(SAY_NORMAL | SAY_NO_NL,
				NLSMSG(GLUE_REPLACEMENT,
				       "%1$s: Glink symbol %2$s replaces"),
				Command_name, dot_name->name);
			    display_symbol(glue_replace_sym, 2);
			}
		    }

		    saved_ext_symbol = new_sym;
		}
		else {
		    /* The symbol must be hidden, because glue code can only
		       contain a single external symbol.  Hidden names must be
		       converted if they match the external name to allow
		       TOC folding. */
		    if (sym->s_name == glue_plain_name)
			new_sym->s_name = plain_name;
		    else if (sym->s_name == glue_dot_name)
			new_sym->s_name = dot_name;
		    /* Now make sure symbol is on proper chain. */
		    new_sym->s_synonym = new_sym->s_name->first_hid_sym;
		    new_sym->s_name->first_hid_sym = new_sym;
		}
	    }
	    /* We can't do RLDs now, because there might be references to
	       symbols that haven't been copied yet.  Just save a pointer to
	       the first RLD in the prototype code */
	    new_cs->c_first_rld = cs->c_first_rld;
	}
    }
    for (er = glue_ext_refs; er; er = er->er_next_in_object) {
	/* Copy the existing ER, preserving er_number. */
	symbol_number = new_symbols->er_number;
	*(new_er = new_symbols++) = *er;
	new_er->er_number = symbol_number;

	*prev_er_slot = new_er;
	prev_er_slot = &new_er->er_next_in_object;
	new_er->er_next_in_object = NULL;

	/* Names of external symbols need to be converted */
	if (new_er->er_name == glue_plain_name)
	    new_er->er_name = plain_name;
	else if (new_er->er_name == glue_dot_name)
	    new_er->er_name = dot_name;

	new_er->er_synonym = new_er->er_name->refs;
	new_er->er_name->refs = new_er;

	er->er_synonym = new_er; /* Save address of copied symbol for RLDs */
    }
    /* Now go through all CSECTS, copying RLDs */
    for (new_sf = first_new_sf; new_sf; new_sf = new_sf->sf_next) {
	for (new_cs = new_sf->sf_csect; new_cs; new_cs = new_cs->c_next) {
	    prev_r_slot = &new_cs->c_first_rld;
	    for (r = new_cs->c_first_rld; r; r = r->r_next) {
		/* Copy RLD, preserving r_number */
		symbol_number = new_rlds->r_number;
		*(new_r = new_rlds++) = *r;
		new_r->r_number = symbol_number;

		*prev_r_slot = new_r;
		prev_r_slot = &new_r->r_next;
		new_r->r_next = NULL;

		new_r->r_csect = new_cs;
		new_r->r_sym = r->r_sym->s_synonym;
	    }
	}
    }
    STAT_use(RLDS_ID, rld_count);

    return saved_ext_symbol;
} /* copy_sym_files */
