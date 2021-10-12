#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)00	1.11  src/bos/usr/ccs/bin/ld/bind/print.c, cmdld, bos41B, 9504A 1/6/95 13:33:22")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS (for bind commands):
 *		mapgen
 *
 *   FUNCTIONS:	do_mapgen
 *
 *   STATIC FUNCTIONS:
 *		compare_names
 *		compare_rlds
 *		compare_rld_names
 *		gen_sorted_map
 *		output_symbol
 *		print_map_header
 *		print_rld
 *		print_rlds
 *		print_symbol
 *		print_symbol_name
 *		print_symbols_for_STR
 *		print_syms_from_csect
 *		renumber_symbols
 *		restore_numbers
 *		xref_rld
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <errno.h>

#include "global.h"
#include "bind.h"
#include "error.h"
#include "strs.h"
#include "dump.h"
#include "save.h"
#include "objects.h"
#include "util.h"

/* Static variables */
static jmp_buf	print_routine;
static char	gen_flag;		/* Which map to generate */
#define GEN_XREF 4			/* Do not change this value, because
					   Print_map_header depends on it. */
#define GEN_CALLS 8			/* Do not change this value, because
					   Print_map_header depends on it. */

static int	used_rlds;		/* RLD count for generating
					   cross-references */
static int	sym_number;		/* Used to renumber symbols */
static int	safe_fprintf_rc;	/* Used for fprintf error detection */
static int	gensort;		/* 1 if sorted address map needed */
static char	*single_object_file_name; /* Non-null if a single input object
					     has been read.  */
static char	*import_char, *export_char; /* Single characters used to denote
					       imported or exported symbols. */

/* Routine to sort RLDs for GEN_XREF. It depends on gensort.*/
static int	(*print_rlds_sort_routine)(const void *, const void *);

/* Structure used to save RLD pointers for sorting. */
static struct sort_rld_array_t {
    union {
	int n;
	RLD *rld;
    } u;
    SYMBOL *sym;
    unsigned long addr;
} *sort_rlds_array;

static SYMBOL **symbol_array;		/* For sorting subsets of symbols */

/* Global variable */
struct save_stuff *saved_stuff;		/* Used to save fields modified while
					   generating XREF address maps.
					   This variable is global so that
					   a routine in dump.c can use it when
					   it prints out symbol numbers. */

/* Define a macro that retries an fprintf() call if EINTR is returned,
   but uses longjmp to exit if another error is returned. */
#define safe_fprintf \
    for(safe_fprintf_rc=-1,errno=EINTR;\
	safe_fprintf_rc<0&&(errno==EINTR||(longjmp(print_routine,1),1));\
	)safe_fprintf_rc=fprintf

/* Forward declarations */
static void print_symbol(FILE *, const int, SYMBOL *, CSECT *, OBJECT *);
static void print_rld(FILE *, const int, RLD *, SYMBOL *, unsigned long);
static void print_rlds(FILE *, const int, SYMBOL *);
static void print_syms_from_csect(FILE *, const int, CSECT *, OBJECT *);
static void gen_sorted_map(FILE *, const int, STR **);
static int  print_symbol_name(FILE *, SYMBOL *, char *);
static void print_map_header(FILE *, const int);
static void renumber_symbols(const int);
static void restore_numbers(void);
static void xref_rld(const int, RLD *);
static int compare_rld_names(const void *, const void *);
static int compare_rlds(const void *, const void *);

/************************************************************************
 * Name: mapgen			Binder Formatted MAP & XREF generator	*
 *				Command Processor			*
 *									*
 * Purpose: Binder command processor which generates a formatted	*
 *	MAP or XREF file.  The output produced is a print image of	*
 *	the binder MAP or Cross Reference.
 *
 *	The output depends on whether the SAVE command has been executed.
 *	If it has, the address map is in essence a dump of the output file.
 *
 *	If SAVE has not been run, the output is a dump of the symbols from
 *	all input files.
 *									*
 * Command Format:							*
 *	GEN request fn
 *									*
 * Parms/Returns:							*
 *	Input:	REQUEST - Request for the type of output to generate.	*
 *			The supported requests are:			*
 *			MAP	- generate the bind map only.		*
 *			XREF	- generate a bind map with list of all	*
 *					references to each symbol.	*
 *			CALLS	- generate the bind map with list of all*
 *					symbols to each CSECT references.
 *			SMAP	- generate the bind map sorted by	*
 *					symbol name.			*
 *			SXREF	- generate a bind map with list of all	*
 *					references to each symbol, sorted
 *					by symbol name
 *			SCALLS	- generate the bind map with list of all*
 *					symbols to each CSECT references,
 *					sorted by symbol name
 *		FN	- File Name for the generated output.		*
 *			The file name can be '-' to have the map go to
 *			stderr
 *	Returns: Returns a status completion code.			*
 *		0 - OK no error detected.				*
 *		4 - WARNING Invalid command parameter	OR
 *			File cannot be opened		OR
 *			File writing interrupted
 *		8 - ERROR: File cannot be written to (after successful
 *			open)
 *									*
 ************************************************************************/
RETCODE
mapgen(char *arg[])			/* argv-style arguments */
{
    char	*arg_temp;
    int		sorting, lgen_flag;
    char	map_fn[PATH_MAX+1], *map_fn_ptr;

    /* Validate the address map type. */
    lower(arg[1]);
    if (arg[1][0] == 's') {
	sorting = 1;
	arg_temp = &arg[1][1];
    }
    else {
	sorting = 0;
	arg_temp = arg[1];
    }

    if (strcmp(arg_temp, "map") == 0)
	lgen_flag = 0;
    else if (strcmp(arg_temp, "xref") == 0) /* set flag to generate xref */
	lgen_flag = GEN_XREF;
    else if (strcmp(arg_temp, "calls") == 0) /* set flag to generate calls */
	lgen_flag = GEN_CALLS;
    else {
	bind_err(SAY_NORMAL, RC_NI_WARNING, NLSMSG(GEN_BAD_TYPE,
		   "%1$s: 0711-834 WARNING: Invalid address map type %2$s.\n"
		   "\tChoices are: %3$s"),
		 Main_command_name, arg[1],
		 "map smap xref sxref calls scalls");
	return RC_NI_WARNING;
    }

    /* Check for a valid pathname */
    if (*arg[2] == '\\') {
	/* The filename contains escapes */
	if (unescape_pathname(map_fn, PATH_MAX, arg[2]) == NULL)
	    goto name_too_long;
	map_fn_ptr = map_fn;
    }
    else {
	if (strlen(arg[2]) > PATH_MAX) {
	  name_too_long:
	    bind_err(SAY_NORMAL, RC_NI_WARNING,
		     NLSMSG(GEN_NAMETOOLONG,
		    "%1$s: 0711-839 WARNING: The pathname is too long.\n"
		    "\tThe maximum length of a pathname is %2$d.\n"
		    "\tThe address map file has not been generated."),
		     Main_command_name, PATH_MAX);
	    return RC_NI_WARNING;
	}
	map_fn_ptr = arg[2];
    }
    return do_mapgen(lgen_flag, sorting, map_fn_ptr, 1);
} /* mapgen */
RETCODE
do_mapgen(int gen_kind,
	  int do_sort,
	  char *map_fn_ptr,
	  int name_len_ok)
{
    int		i;
    int		stderr_map;		/* 1 if map file is "-" (for stderr) */
    int		rc = RC_OK;
    const int	state_save_called
	= (Bind_state.state & STATE_SAVE_CALLED) ? 1 : 0;
    FILE	*fp = NULL;
    OBJECT	*obj;
    SRCFILE	*sf;
    CSECT	*cs;
    SYMBOL	*sym;
    SYMBOL	*er;
    STR		**sorted = NULL;

    if (do_sort) {
	gensort = 1;
	print_rlds_sort_routine = compare_rld_names;
    }
    else {
	gensort = 0;
	print_rlds_sort_routine = compare_rlds;
    }

    gen_flag = gen_kind;

    /* Check for a valid pathname if necessary. */
    if (name_len_ok == 0 && strlen(map_fn_ptr) > PATH_MAX) {
      name_too_long:
	bind_err(SAY_NORMAL, RC_NI_WARNING,
		 NLSMSG(GEN_NAMETOOLONG,
			"%1$s: 0711-839 WARNING: The pathname is too long.\n"
			"\tThe maximum length of a pathname is %2$d.\n"
			"\tThe address map file has not been generated."),
		 Main_command_name, PATH_MAX);
	return RC_NI_WARNING;
    }

    /* Allocate arrays. */
    if (gen_flag == GEN_XREF || gensort && gen_flag == GEN_CALLS)
	sort_rlds_array
	    = emalloc(sizeof(*sort_rlds_array) * total_rlds_allocated(), NULL);

    if (gen_flag == GEN_XREF) {
	/* We have no direct way to find all references to
	   a particular symbol, so we must construct a reverse mapping.
	   We allocate arrays for this. */
	saved_stuff
	    = emalloc(sizeof(*saved_stuff) * total_rlds_allocated(), NULL);
	if (sort_rlds_array == NULL || saved_stuff == NULL)
	    goto no_memory;
	used_rlds = 0;
    }

    if (gensort) {
	/* Allocate an array for sorting symbol names */
	sorted = emalloc(total_STRS() * sizeof(*sorted), NULL);
	if (sorted == NULL
	    || (gen_flag == GEN_CALLS && sort_rlds_array == NULL))
	    goto no_memory;
    }

    /* Allocate an array for sorting subsets of symbols. */
    if ((symbol_array = emalloc(sizeof(*symbol_array)
				* total_symbols_allocated(), NULL)) == NULL)
	goto no_memory;

    single_object_file_name = NULL;
    renumber_symbols(state_save_called);

    /* Look up strings for "import", "export" and "entry" */
    if (import_char == NULL) {
	/* Get these names once */
	import_char = msg_get(NLSMSG(LIT_IMPORT, "I"));
	export_char = msg_get(NLSMSG(LIT_EXPORT, "E"));
    }

    if (map_fn_ptr[0] == '-' && map_fn_ptr[1] == '\0') {
	stderr_map = 1;
	fp = stderr;
    }
    else {
	if ((fp = fopen(map_fn_ptr, "w+")) == NULL) {
	    bind_err(SAY_NORMAL, RC_NI_WARNING,
		     NLSMSG(GEN_NO_OPEN,
	    "%1$s: 0711-836 WARNING: Cannot open the address map file: %2$s\n"
	    "\t%1$s:fopen() %3$s"),
		     Main_command_name, map_fn_ptr, strerror(errno));
	    rc = RC_NI_WARNING;
	    goto free_and_return;
	}
	stderr_map = 0;
    }

    if (setjmp(print_routine) != 0) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(GEN_NO_WRITE,
	"%1$s: 0711-837 ERROR: Cannot write to the address map file: %2$s\n"
	"\t%1$s:fprintf() %3$s\n"
	"\tThe file may be incomplete."),
		 Main_command_name, map_fn_ptr, strerror(errno));
	rc = RC_NI_ERROR;
	if (gen_flag == GEN_XREF)
	    restore_numbers();
	goto free_and_return;
    }

    print_map_header(fp, state_save_called);

    if (gensort)
	gen_sorted_map(fp, state_save_called, sorted);
    else if (state_save_called) {
	/* Print unresolved symbols */
	for (i = 0; i < last_unresolved; ++i) {
	    if (interrupt_flag)
		goto interrupt_return;
	    er = unresolved_queue[i];
	    print_symbol(fp, 1, er, NULL, NULL);
	    /* No CALLS from undefined symbols. */
	    if (er->s_flags & S_NUMBER_USURPED) /* => XREF */
		print_rlds(fp, 1, er);
	}

	/* Print symbols from import files and shared objects. */
	for (i = 0; i < first_text_index; i++) {
	    if (interrupt_flag)
		goto interrupt_return;
	    cs = Queue[i];
	    obj = cs->c_srcfile->sf_object;
	    for (sym = &cs->c_symbol; sym; sym = sym->s_next_in_csect) {
		if (sym->s_flags & S_SAVE) {
		    print_symbol(fp, 1, sym, cs, obj);
		    if (sym->s_flags & S_NUMBER_USURPED) /* => XREF */
			print_rlds(fp, 1, sym);
		}
	    }
	}

	/* Print remaining symbols */
	for ( ; i < Queue_size; i++) {
	    if (interrupt_flag)
		goto interrupt_return;
	    print_syms_from_csect(fp, 1, Queue[i],
				  Queue[i]->c_srcfile->sf_object);
	}
    }
    else {
	for (obj = first_object(); obj; obj = obj->o_next) {
	    switch(obj->o_type) {
	      case O_T_OBJECT:
		/* Print ERs */
		for (sym = obj->oi_ext_refs;
		     sym;
		     sym = sym->er_next_in_object) {
		    if (interrupt_flag)
			goto interrupt_return;
		    print_symbol(fp, 0, sym, NULL, obj);
		    if (sym->s_flags & S_NUMBER_USURPED) /*=> xref */
			print_rlds(fp, 0, sym);
		}
		/* Print other symbols */
		for (sf = obj->oi_srcfiles; sf; sf = sf->sf_next) {
		    if (interrupt_flag)
			goto interrupt_return;
		    for (cs = sf->sf_csect; cs; cs = cs->c_next)
			print_syms_from_csect(fp, 0, cs, obj);
		}
		break;

	      case O_T_SCRIPT:
	      case O_T_SHARED_OBJECT:
		for (sf = obj->o_srcfiles; sf; sf = sf->sf_next)
		    for (cs = sf->sf_csect; cs; cs = cs->c_next) {
			if (interrupt_flag)
			    goto interrupt_return;
			for (sym=&cs->c_symbol; sym; sym=sym->s_next_in_csect)
			    print_symbol(fp, 0, sym, cs, obj);
			/* If state_save_called == 0, references that are
			   printed are only within an object file.  Therefore,
			   there can be no CALLS or references. */
		    }
		break;

	      case O_T_ARCHIVE_SYMTAB:
		for (sym = obj->o_gst_syms; sym; sym = sym->s_next_in_csect) {
		    if (interrupt_flag)
			goto interrupt_return;
		    print_symbol(fp, 0, sym, NULL, obj);
		    /* If state_save_called == 0, references that are
		       printed are only within an object file.  Therefore,
		       there can be no CALLS or references. */
		}
		break;
	    }
	}
    }
    goto free_and_return;

  interrupt_return:
    bind_err(SAY_NORMAL, RC_NI_WARNING,
	     NLSMSG(MAIN_INTERRUPT,
		    "%1$s: 0711-635 WARNING: Binder command %2$s interrupted."),
	     Main_command_name, Command_name);
    goto return_warning;

  no_memory:
    bind_err(SAY_NORMAL, RC_NI_WARNING,
	     NLSMSG(GEN_NO_MEM,
	    "%1$s: 0711-838 WARNING: There is not enough memory available.\n"
	    "\tThe address map file %2$s has not been generated."),
	     Main_command_name, map_fn_ptr);
  return_warning:
    rc = RC_NI_WARNING;

  free_and_return:
    if (saved_stuff) {
	efree(saved_stuff);
	saved_stuff = NULL;
    }
    if (symbol_array) {
	efree(symbol_array);
	symbol_array = NULL;
    }
    if (sort_rlds_array) {
	efree(sort_rlds_array);
	sort_rlds_array = NULL;
    }
    if (sorted) {
	efree(sorted);
	sorted = NULL;
    }

    if (!stderr_map && fp) {
	(void) fclose(fp);
	fp = NULL;
    }
    return rc;
} /* do_mapgen */
/************************************************************************
 * Name: compare_addrs() - comparison routine for qsort() used to sort
 *		symbol addresses.
 *									*
 ************************************************************************/
static int
compare_addrs(const void *a,
	      const void *b)
{
    SYMBOL	*sym1 = *(SYMBOL **)a;
    SYMBOL	*sym2 = *(SYMBOL **)b;
    int		n1, n2;

    if (sym1->s_addr == sym2->s_addr) {
	/* If addresses are equal, compare symbol input indexes, preserving
	   the order the symbols were listed in the input file.  Since a
	   CSECT must appear before any labels contained in the CSECT,
	   this secondary sort makes sure that labels appear after
	   CSECTs in the address map file.  We cannot use the s_number
	   field, because a label found in the global symbol table of
	   an archive will be created (and have a smaller s_number)
	   than its containing CSECT.  */
	n1 = (sym1->s_flags & S_INPNDX_MOD)
	    ? symtab_index[sym1->s_inpndx] : sym1->s_inpndx;
	n2 = (sym2->s_flags & S_INPNDX_MOD)
	    ? symtab_index[sym2->s_inpndx] : sym2->s_inpndx;
	return n1 - n2;
    }
    else
	return sym1->s_addr - sym2->s_addr;
}
/************************************************************************
 * Name: renumber_symbols
 *
 * Purpose: Give each symbol a unique number so that they are printed in
 *	numberical order
 *
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
renumber_symbols(const int state_save_called)
{
    int		i;
    int		count, k;
    OBJECT	*obj;
    RLD		*rld;
    SRCFILE	*sf;
    CSECT	*cs;
    SYMBOL	*sym;

#ifdef DEBUG
    long	unused_symbols = -1;	/* Number unused symbols to detect
					   errors in case an unused symbol is
					   printed when it shouldn't be. */
#endif

#define number_symbol(sym) \
(*((sym->s_flags & S_NUMBER_USURPED) \
   ? &saved_stuff[sym->s_number].number \
   : &sym->s_number) = sym_number++)

    sym_number = 1;			/* Start numbering from 1 */

    /* We print two different symbol maps depending on whether the SAVE
       command has been called or not.  If it has been called, we use
       the csect array to print out symbols in address order, or to decide
       which symbols to print if a sorted map is needed.  Otherwise, we go
       through all objects to find the symbol to print. Only the first case
       can be generated by standard 'ld' options. */

    /* Number all the used symbols */
    if (state_save_called) {
	/* Number undefined symbols. */
	for (i = 0; i < last_unresolved; ++i)
	    number_symbol(unresolved_queue[i]);

	/* Number imported symbols */
	for (i = 0; i < first_text_index; i++) {
	    /* For an import file, the CSECT is a dummy.  Each symbol
	       is marked with S_SAVE individually. */
	    for (sym = &Queue[i]->c_symbol; sym; sym = sym->s_next_in_csect)
		if (sym->s_flags & S_SAVE)
		    number_symbol(sym);
#ifdef DEBUG
		else
		    sym->s_number = unused_symbols--;
#endif
	}

	/* Number symbols from XCOFF files. */
	for ( ; i < Queue_size; i++) {
	    cs = Queue[i];
	    /* All symbols in a saved CSECT are saved.  The LDs aren't
	     necessarily in addess order, so we must sort them. */
	    count = 0;
	    for (sym = &cs->c_symbol; sym; sym = sym->s_next_in_csect)
		symbol_array[count++] = sym;
	    qsort(symbol_array, count, sizeof(symbol_array[0]), compare_addrs);
	    for (k = 0; k < count ; ++k)
		number_symbol(symbol_array[k]);
	    if (gen_flag == GEN_XREF)
		for (rld = cs->c_first_rld; rld; rld = rld->r_next)
		    xref_rld(1, rld);
	}
    }
    else {
	obj = first_object();
	if (obj->o_next == NULL)
	    single_object_file_name = get_object_file_name(obj);
	for ( ; obj; obj = obj->o_next) {
	    switch(obj->o_type) {
	      case O_T_OBJECT:
		/* Number ERs in object. */
		for (sym = obj->oi_ext_refs; sym; sym = sym->er_next_in_object)
		    number_symbol(sym);
		/* Number other symbols in object */
		for (sf = obj->oi_srcfiles; sf; sf = sf->sf_next)
		    for (cs = sf->sf_csect; cs; cs = cs->c_next) {
			count = 0;
			for (sym = &cs->c_symbol;
			     sym;
			     sym = sym->s_next_in_csect)
			    symbol_array[count++] = sym;
			qsort(symbol_array, count, sizeof(symbol_array[0]),
			      compare_addrs);
			for (k = 0; k < count ; ++k)
			    number_symbol(symbol_array[k]);

			if (gen_flag == GEN_XREF)
			    for (rld = cs->c_first_rld; rld; rld = rld->r_next)
				xref_rld(0, rld);
		    }
		break;

	      case O_T_SCRIPT:
	      case O_T_SHARED_OBJECT:
		for (sf = obj->o_srcfiles; sf; sf = sf->sf_next)
		    for (cs = sf->sf_csect; cs; cs = cs->c_next) {
			for (sym = &cs->c_symbol;
			     sym;
			     sym = sym->s_next_in_csect)
			    number_symbol(sym);
			/* No RLDs for imported or shared object symbols. */
		    }
		break;

	      case O_T_ARCHIVE_SYMTAB:
		for (sym = obj->o_gst_syms; sym; sym = sym->s_next_in_csect)
		    number_symbol(sym);
		/* No RLDs for symbols from global symbol table */
		break;
	    }
	}
    }
}
/************************************************************************
 * Name: xref_rld
 *									*
 * PURPOSE:  This routine takes an RLD, finds the symbol it references,
 *	and adds it to a chain of RLDs referencing that symbol.  The
 *	chain is stored in the saved_stuff array, and the head of
 *	the chain is found in the s_number field of the symbol.  The
 *	original contents of the s_number field is saved in
 *	saved_stuff[].number.
 *
 *	Because new elements are added to
 *	the head of the list, saved_stuff[].number must be copied
 *	as each new RLD is added to the list.
 *
 *	Returns: none							*
 *									*
 ************************************************************************/
static void
xref_rld(const int	state_save_called,
	 RLD		*rld)
{
    SYMBOL *sym1;

    if (state_save_called) {
	if (rld->r_flags & RLD_RESOLVE_BY_NAME) {
	    if (rld->r_sym->s_name->flags & STR_NO_DEF)
		sym1 = NULL;
	    else
		sym1 = rld->r_sym->s_name->first_ext_sym;
	}
	else
	    sym1 = rld->r_sym->s_resolved;

	if (sym1 == NULL) {
	    /* We'll use first ER as symbol for cross-reference information */
	    sym1 = rld->r_sym->s_name->refs;
	}
    }
    else {
	/* We may need to worry about delcsect processing here. */
	sym1 = rld->r_sym;
    }

    if (sym1->s_flags & S_NUMBER_USURPED) {
	saved_stuff[used_rlds].link = sym1->s_number;
	saved_stuff[used_rlds].number = saved_stuff[sym1->s_number].number;
    }
    else {
	saved_stuff[used_rlds].link = -1;
	saved_stuff[used_rlds].number = sym1->s_number;
	sym1->s_flags |= S_NUMBER_USURPED;
    }
    saved_stuff[used_rlds].rld = rld;

    /* Symbol uses s_number field to point to head of list */
    sym1->s_number = used_rlds++;
} /* xref_rld */
/************************************************************************
 * Name: print_syms_from_csect
 *									*
 * Purpose: Prints all symbols from the given CSECT.  This function is
 *		only used for unsorted address maps.
 *									*
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
print_syms_from_csect(FILE *fp,
		      const int state_save_called,
		      CSECT *cs,
		      OBJECT *obj)
{
    SYMBOL	*sym;
    RLD		*rld;
    int		count, k;

    /* The XTY_LDs for a csect are not necessarily in
       address order, so we must sort the symbols in a csect. */
    count = 0;
    for (sym = &cs->c_symbol; sym; sym = sym->s_next_in_csect)
	symbol_array[count++] = sym;

    qsort(symbol_array, count, sizeof(symbol_array[0]), compare_addrs);

    switch(gen_flag) {
      case GEN_CALLS:
	/* For "CALLS", we step through symbols and rlds in tandem,
	   since both are in address order. */
	for (k = 0, rld = cs->c_first_rld; k < count ; ++k) {
	    print_symbol(fp, state_save_called, symbol_array[k], cs, obj);
	    for ( ; rld; rld = rld->r_next) {
		if (k + 1 < count && rld->r_addr >= symbol_array[k+1]->s_addr)
		    break;
		print_rld(fp, state_save_called, rld, NULL, 0);
	    }
	}
	break;
      case GEN_XREF:
	for (k = 0; k < count ; ++k) {
	    sym = symbol_array[k];
	    print_symbol(fp, state_save_called, sym, cs, obj);
	    if (sym->s_flags & S_NUMBER_USURPED) /* => XREF */
		print_rlds(fp, state_save_called, sym);
	}
	break;
      default:
	for (k = 0; k < count ; ++k)
	    print_symbol(fp, state_save_called, symbol_array[k], cs, obj);
    }
} /* print_syms_from_csect */
/************************************************************************
 * Name: compare_rld_names() - comparison routine for qsort() called by
 *		print_rlds() and output_symbol()
 *
 *	Returns: 							*
 *									*
 ************************************************************************/
static int
compare_rld_names(const void *a,
		  const void *b)
{
    int n;
    SYMBOL *sym1 = ((struct sort_rld_array_t *)a)->sym;
    SYMBOL *sym2 = ((struct sort_rld_array_t *)b)->sym;

    n = strcoll(sym1->s_name->name, sym2->s_name->name);
    if (n == 0) {
	int n1 = (sym1->s_flags & S_NUMBER_USURPED)
	    ? saved_stuff[sym1->s_number].number : sym1->s_number;
	int n2 = (sym2->s_flags & S_NUMBER_USURPED)
	    ? saved_stuff[sym2->s_number].number : sym2->s_number;
	n = n1 - n2;
	if (n == 0)
	    if (gen_flag == GEN_XREF)
		n = ((struct sort_rld_array_t *)a)->addr
		    - ((struct sort_rld_array_t *)b)->addr;
	    else
		n = ((struct sort_rld_array_t *)a)->u.rld->r_addr
		    - ((struct sort_rld_array_t *)b)->u.rld->r_addr;

    }
    return n;
}
/************************************************************************
 * Name: output_symbol
 *									*
 * Purpose: Prints information about a given SYMBOL.
 *		This function is only used for sorted address maps.
 *									*
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
output_symbol(FILE *fp,
	      const int state_save_called,
	      SYMBOL *sym)
{
    RLD *rld;
    SYMBOL *ref_sym;
    int i, n;
    unsigned long addr;
    CSECT *cs;
    OBJECT *obj;

#ifdef DEBUG
    if (!gensort)
	internal_error();
#endif

    switch(sym->s_smtype) {
      case XTY_AR:
	obj = sym->s_object;
	cs = NULL;
	break;

      case XTY_ER:
	if (state_save_called)
	    obj = NULL;
	else
	    obj = sym->er_object;
	cs = NULL;
	break;

      default:
	obj = sym->s_csect->c_srcfile->sf_object;
	cs = sym->s_csect;
	break;
    }
    print_symbol(fp, state_save_called, sym, cs, obj);
    if (gen_flag == GEN_CALLS && cs) {
	n = 0;
	for (rld = cs->c_first_rld; rld; rld = rld->r_next) {
	    if (rld->r_addr < sym->s_addr)
		continue;
	    if (sym->s_next_in_csect
		&& rld->r_addr > sym->s_next_in_csect->s_addr)
		break;

	    /* Find symbol referenced by RLD and its address */
	    if (!state_save_called) {
		ref_sym = rld->r_sym;
		addr = ref_sym->s_addr;	/* Get address from ER.  It will be
					   non-zero if it is an XO symbol. */
	    }
	    else if (rld->r_flags & RLD_RESOLVE_BY_NAME) {
		if (rld->r_sym->s_name->flags & STR_NO_DEF) {
		    ref_sym = rld->r_sym->s_name->refs;
		    addr = 0;
		}
		else {
		    ref_sym = rld->r_sym->s_name->first_ext_sym;
		    if ((ref_sym->s_flags & S_LOCAL_GLUE)
			&& rld->r_csect->c_symbol.s_smclass == XMC_DS)
			ref_sym = ref_sym->s_synonym;
		    addr = ref_sym->s_addr +
			ref_sym->s_csect->c_new_addr - ref_sym->s_csect->c_addr;
		}
	    }
	    else {
		ref_sym = rld->r_sym->s_resolved;
		if (ref_sym == NULL)	/* Delcsect processing must have
					   deleted symbol. */
		    continue;
		addr = ref_sym->s_addr +
		    ref_sym->s_csect->c_new_addr - ref_sym->s_csect->c_addr;
	    }

	    sort_rlds_array[n].u.rld = rld;
	    sort_rlds_array[n].addr = addr;
	    sort_rlds_array[n++].sym = ref_sym;
	}
	if (n > 0) {
	    qsort(sort_rlds_array, n, sizeof(sort_rlds_array[0]),
		  compare_rld_names);
	    for (i = 0; i < n; i++)
		print_rld(fp, state_save_called, sort_rlds_array[i].u.rld,
			  sort_rlds_array[i].sym, sort_rlds_array[i].addr);
	}
    }
    if (sym->s_flags & S_NUMBER_USURPED) /* => XREF */
	print_rlds(fp, state_save_called, sym);
} /* output_symbol */
/************************************************************************
 * Name: compare_numbers() - comparison routine for qsort() used to sort
 *		symbol numbers.
 *
 *									*
 ************************************************************************/
static int
compare_numbers(const void *a,
		const void *b)
{
    SYMBOL *sym1 = *(SYMBOL **)a;
    SYMBOL *sym2 = *(SYMBOL **)b;
    int n1 = (sym1->s_flags & S_NUMBER_USURPED)
	? saved_stuff[sym1->s_number].number : sym1->s_number;
    int n2 = (sym2->s_flags & S_NUMBER_USURPED)
	? saved_stuff[sym2->s_number].number : sym2->s_number;
    return n1 - n2;
}
/************************************************************************
 * Name: print_symbols_for_STR
 *									*
 * Purpose: Prints information about a given STR.
 *		This function is only used for sorted address maps.
 *									*
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
print_symbols_for_STR(FILE *fp,
		      const int state_save_called,
		      STR *str)
{
    int		i, n;
    SYMBOL	*sym;

    if (str == NULL)
	return;

    /* External references */
    sym = str->refs;
    if (sym) {
	if (state_save_called) {
	    /* Only 1 undefined instance possible. */
	    if (sym->s_flags & S_SAVE)
		output_symbol(fp, 1, sym);
	}
	else {
	    /* Multiple instances could occur. */
	    n = 0;
	    for ( ; sym; sym = sym->s_synonym)
		symbol_array[n++] = sym;
	    qsort(symbol_array, n, sizeof(symbol_array[0]), compare_numbers);
	    for (i = 0; i < n; i++)
		output_symbol(fp, 0, symbol_array[i]);
	}
    }

    /* External symbols */
    sym = str->first_ext_sym;
    if (sym) {
	if (state_save_called) {
	    /* Only 1 instance can possibly be saved,
	       unless we added local glue. */
	    if ((sym->s_flags & S_SAVE) || (!(imported_symbol(sym))
					    && sym->s_smtype != XTY_AR
					    && sym->s_csect->c_save)) {
		output_symbol(fp, 1, sym);
		if (sym->s_flags & S_LOCAL_GLUE)
		    output_symbol(fp, 1, sym->s_synonym);
	    }
	}
	else {
	    /* Multiple instances could occur. */
	    n = 0;
	    for ( ; sym; sym = sym->s_synonym) {
		if (!(sym->s_flags & S_DUPLICATE))
		    symbol_array[n++] = sym;
	    }
	    qsort(symbol_array, n, sizeof(symbol_array[0]), compare_numbers);
	    for (i = 0; i < n; i++)
		output_symbol(fp, 0, symbol_array[i]);
	}
    }
    /* Hidden symbols */
    n = 0;
    for (sym = str->first_hid_sym; sym; sym = sym->s_synonym) {
	if ((state_save_called
	     && ((sym->s_flags & S_SAVE) || (!(imported_symbol(sym))
					     && sym->s_smtype != XTY_AR
					     && sym->s_csect->c_save)))
	    || (!state_save_called && !(sym->s_flags & S_DUPLICATE)))
	    symbol_array[n++] = sym;
    }
    qsort(symbol_array, n, sizeof(symbol_array[0]), compare_numbers);
    for (i = 0; i < n; i++)
	output_symbol(fp, state_save_called, symbol_array[i]);

} /* print_symbols_for_STR */
/************************************************************************
 * Name: compare_names() - comparison routine for qsort() used to sort
 *		symbol names.
 *
 * Purpose:  Compare names in the current locale.  No error checking
 *	is performed.  No attempt is made to use strxform
 *									*
 ************************************************************************/
static int
compare_names(const void *a,
	      const void *b)
{
    return strcoll((*((STR **)a))->name, ((*(STR **)b))->name);
}
/************************************************************************
 * Name: gen_sorted_map
 *									*
 * Purpose: Prints the sorted maps
 *									*
 ************************************************************************/
static void
gen_sorted_map(FILE *fp,
	       const int state_save_called,
	       STR **sorted)		/* Array for sorting names */
{
    int		i;
    int		j = 0;
    SYMBOL	*sym;
    STR		*s;
    HASH_STR	*sroot, *sh;

    /* Find names to be sorted */
    for (sroot = &HASH_STR_root; sroot; sroot = sroot->next) {
	if (interrupt_flag)
	    goto interrupt_return;
	for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
	    if (!state_save_called) {
		sorted[j++] = &sh->s;
		continue;
	    }
	    /* We save a name if an external symbol instance has been saved, or
	       it is contained in a csect that has been saved.  We only have to
	       check the first one, because the saved symbol is moved to the
	       front of the list during resolve.

	       We save a name if any internal instance was saved.

	       We save a name if there were any undefined references.  In
	       this case, the first ER will have S_SAVE set.

	       If we save a name because of a plain name, there's no need
	       to check for a .name. */
	    s = &sh->s;
	    if ((sym = s->first_ext_sym)
		&& ((sym->s_flags & S_SAVE) || (!(imported_symbol(sym))
						&& sym->s_smtype != XTY_AR
						&& sym->s_csect->c_save))) {
		sorted[j++] = &sh->s;
		continue;
	    }

	    for (sym = s->first_hid_sym; sym; sym = sym->s_synonym) {
		if ((sym->s_flags & S_SAVE) || (!(imported_symbol(sym))
						&& sym->s_smtype != XTY_AR
						&& sym->s_csect->c_save)) {
		    sorted[j++] = &sh->s;
		    goto next_i;
		}
	    }
	    if (s->refs && s->refs->s_flags & S_SAVE) {
		sorted[j++] = &sh->s;
		continue;
	    }

	    if ((s = s->alternate) == NULL)
		continue;

	    /* Repeat above code for the dotted name */
	    if ((sym = s->first_ext_sym)
		&& ((sym->s_flags & S_SAVE) || (!(imported_symbol(sym))
						&& sym->s_smtype != XTY_AR
						&& sym->s_csect->c_save))) {
		sorted[j++] = &sh->s;
		continue;
	    }

	    for (sym = s->first_hid_sym; sym; sym = sym->s_synonym) {
		if ((sym->s_flags & S_SAVE) || (!(imported_symbol(sym))
						&& sym->s_smtype != XTY_AR
						&& sym->s_csect->c_save)) {
		    sorted[j++] = &sh->s;
		    goto next_i;
		}
	    }
	    if (s->refs && s->refs->s_flags & S_SAVE) {
		sorted[j++] = &sh->s;
		continue;
	    }
	  next_i:
	    ;
	}
    }

    qsort(sorted, j, sizeof(*sorted), compare_names);

    /* Generate the output */
    /* Unnamed symbols */
    print_symbols_for_STR(fp, state_save_called, &NULL_STR);
    /* Symbols named '.' */
    print_symbols_for_STR(fp, state_save_called, NULL_STR.alternate);

    /* Do all dot names. */
    for (i = 0; i < j && !interrupt_flag; ++i)
	print_symbols_for_STR(fp, state_save_called, sorted[i]->alternate);

    /* Do all plain names */
    for (i = 0; i < j && !interrupt_flag; ++i)
	print_symbols_for_STR(fp, state_save_called, sorted[i]);

  interrupt_return:
    if (interrupt_flag)
	bind_err(SAY_NORMAL, RC_NI_WARNING,
		 NLSMSG(MAIN_INTERRUPT,
		"%1$s: 0711-635 WARNING: Binder command %2$s interrupted."),
		 Main_command_name, Command_name);
    return;
} /* gen_sorted_map */
/************************************************************************
 * Name: print_map_header
 *									*
 * Purpose: Print the title and column headings for the address map
 *									*
 * Parms
 *	Input:	FP - File pointer
 *									*
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
print_map_header(FILE *fp,
		 const int state_save_called)
{
    char *title, *s_hdr, *hdr2;
    char *f;

    switch (gen_flag) {
      case 0:
	hdr2 = "";
	break;
      case GEN_CALLS:
	hdr2 = msg_get(NLSMSG(GEN_CALLS_HDR, "\
    ADDRESS                         References to:  ADDRESS  RLD-type(bit-length) Sym#  NAME\n"));
/*       1         2         3         4         5         6         7         8         9
12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012 */
	break;
      case GEN_XREF:
	hdr2 = msg_get(NLSMSG(GEN_XREF_HDR, "\
                                  References from:  ADDRESS  RLD-type(bit-length) Sym#  NAME\n"));
/*       1         2         3         4         5         6         7         8         9
12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012 */
	break;
    }

    if (state_save_called)
	f = Bind_state.out_name;
    else if (single_object_file_name != NULL)
	f = single_object_file_name;
    else
	f = NULL;

    switch (gen_flag + 2 * gensort + (f != NULL)) {
      case 0:
	title = msg_get(NLSMSG(GEN_TITLE_DEFAULT2,
			       "ADDRESS MAP FOR INPUT FILES\n"));
	break;
      case 1:				/* state_save_called */
	title = msg_get(NLSMSG(GEN_MAP3, "ADDRESS MAP FOR %s\n"));
	break;
      case 2:				/* gensort */
	title = msg_get(NLSMSG(GEN_TITLE_DEFAULT2_S,
			       "SORTED ADDRESS MAP FOR INPUT FILES\n"));
	break;
      case 3:				/* gensort + state_save_called */
	title = msg_get(NLSMSG(GEN_TITLE_DEFAULT_S,
			       "SORTED ADDRESS MAP FOR %s\n"));
	break;

      case GEN_CALLS:
	title = msg_get(NLSMSG(GEN_TITLE_CALLS2,
		       "ADDRESS MAP FOR INPUT FILES WITH REFERENCES\n"));
	break;
      case GEN_CALLS + 1:		/* state_save_called */
	title = msg_get(NLSMSG(GEN_MAP2,
			       "ADDRESS MAP FOR %s WITH REFERENCES\n"));
	break;
      case GEN_CALLS + 2:		/* gensort */
	    title = msg_get(NLSMSG(GEN_TITLE_CALLS2_S,
		   "SORTED ADDRESS MAP FOR INPUT FILES WITH REFERENCES\n"));
	break;
      case GEN_CALLS + 3:		/* gensort + state_save_called */
	title = msg_get(NLSMSG(GEN_TITLE_CALLS_S,
			       "SORTED ADDRESS MAP FOR %s WITH REFERENCES\n"));
	break;

      case GEN_XREF:
	title = msg_get(NLSMSG(GEN_TITLE_XREF2,
		       "ADDRESS MAP FOR INPUT FILES WITH CROSS REFERENCES\n"));
	break;
      case GEN_XREF + 1:		/* state_save_called */
	title = msg_get(NLSMSG(GEN_MAP1,
			       "ADDRESS MAP FOR %s WITH CROSS REFERENCES\n"));
	break;
      case GEN_XREF + 2:		/* gensort */
	title = msg_get(NLSMSG(GEN_TITLE_XREF2_S,
	       "SORTED ADDRESS MAP FOR INPUT FILES WITH CROSS REFERENCES\n"));
	break;
      case GEN_XREF + 3:		/* gensort + state_save_called */
	title = msg_get(NLSMSG(GEN_TITLE_XREF_S,
		       "SORTED ADDRESS MAP FOR %s WITH CROSS REFERENCES\n"));
	break;
    }
#define MAP_NAME_POS 30
#define MAP_NAME_LEN 31

#define MAP_XREF_BEGIN 53
#define MAP_RLDTYPE_LEN 21

 s_hdr = msg_get(NLSMSG(GEN_HDR, "*%s%s\
 ADDRESS  LENGTH AL CL TY Sym#  NAME                      SOURCE-FILE(OBJECT) or IMPORT-FILE{SHARED-OBJECT}\n"
"%s---\
 -------- ------ -- -- -- ----- ------------------------- -------------------------------------------------\n"));
/*    1         2         3         4         5         6         7         8         9        10        11
45678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890 */

    if (f)
	safe_fprintf(fp, title, f);
    else
	safe_fprintf(fp, title);

    safe_fprintf(fp, s_hdr, import_char, export_char, hdr2);
} /* print_map_header */
/************************************************************************
 * Name: print_symbol		Print formatted output of a SYMBOL
 *									*
 * Purpose: Print information about a symbol
 *									*
 * Side Effects:							*
 *	Information printed to address map file.
 *
 ************************************************************************/
static void
print_symbol(FILE *fp,
	     const int state_save_called,
	     SYMBOL *sym,		/* Symbol to be printed */
	     CSECT *cs,			/* Csect of symbol (or NULL) */
	     OBJECT *obj)		/* Object of symbol (or NULL) */
{
    char	entry,
		*stg,
		*type;
    char	*s1;
    int		need_addr, need_len;
    unsigned long	addr;
    char	*es, *is;
    int		name_len;
    int		pad;

    type = get_smtype(sym->s_smtype);
    stg = get_smclass(sym->s_smclass);

    /* Setup entry and export characters. If SAVE hasn't been called, these
       characters will only be printed for the first instance of an external
       name. */
    if (sym == sym->s_name->first_ext_sym) {
	if (sym->s_name->flags & STR_ENTRY)
	    entry = '*';
	else
	    entry = ' ';
	if (sym->s_name->flags & STR_EXPORT)
	    es = export_char;
	else
	    es = " ";
    }
    else {
	entry = ' ';
	es = " ";
    }
    /* Setup import character. Each imported instance is marked.  If SAVE has
       been called, a single instance is all that will be printed, because
       print_symbol will only be called once for a given global name. */
    if (imported_symbol(sym))
	is = import_char;
    else
	is = " ";

    need_len = 0;
    need_addr = 1;

    switch(sym->s_smtype) {
      case XTY_CM:
      case XTY_SD:
	need_len = 1;
	break;

      case XTY_IF:
	if (XMC_XO != sym->s_smclass)
	    stg = "  ";
	/* Fall through */
      case XTY_IS:
	type = "ER";
	if (XMC_XO != sym->s_smclass)
	    need_addr = 0;
	break;

      case XTY_LD:
	break;

      case XTY_AR:
	type = "  ";
	stg = "  ";
	need_addr = 0;
	break;

      case XTY_ER:
	if (XMC_XO == sym->s_smclass)
	    need_addr = -1;
	else
	    need_addr = 0;
	break;
    }

    if (need_addr) {
	addr = sym->s_addr;
	if (need_addr > 0) {
	    /* After executing the save command, we'll use the new addresses
	       of the symbols */
	    if (state_save_called)
		addr += (cs->c_new_addr - cs->c_addr);
	}

	if (need_len)
	    safe_fprintf(fp, "%c%1s%1s %08X %06X %2d %s %s ", entry, is, es,
			 addr, cs->c_len, cs->c_align, stg, type);
	else
	    safe_fprintf(fp, "%c%1s%1s %08X %s %s %s ", entry, is, es,
			 addr, "         ",	/* 9 blanks */
			 stg, type);
    }
    else
	safe_fprintf(fp, "%c%1s%1s %s %s %s %s ", entry, is, es,
		     "        ",	/* 8 blanks for address column*/
		     "         ",	/* 9 blanks for length/blank/align */
		     stg, type);

    name_len = print_symbol_name(fp, sym, "" /* No newline */);

    if (MAP_NAME_LEN - name_len <= 0)
	pad = 1;
    else
	pad = MAP_NAME_LEN - name_len + 1;

    switch(sym->s_smtype) {
      case XTY_AR:
	safe_fprintf(fp, "%*s<%s>\n",  pad, "", obj->o_ifile->i_name->name);
	return;

      case XTY_ER:
	if (obj)
	    safe_fprintf(fp, "%*s(%s)\n", pad, "", get_object_file_name(obj));
	else
	    safe_fprintf(fp, "\n");
	return;

      case XTY_LD:
	if (gensort == 0) {
	    safe_fprintf(fp, "\n");
	    return;
	}
	break;

      default:
	break;
    }

    if ((sym->s_flags & S_HIDEXT)
	&& (sym->s_smclass == XMC_TC || sym->s_smclass == XMC_TC0))
	safe_fprintf(fp, "\n");
    else if (sym->s_inpndx == INPNDX_FIXUP) {
	safe_fprintf(fp, "%*s**%s**\n", pad, "",
		     msg_get(NLSMSG(DUMPLIT_FIXUP, "FIXUP")));
    }
    else {
	/* Print SOURCE-FILE/OBJECT-FILE column */
	switch(sym->s_smtype) {
	  case XTY_IF:
	    safe_fprintf(fp, "%*s%s{%s}\n", pad, "", get_object_file_name(obj),
			 cs->c_srcfile->sf_name->name);
	    break;
	  case XTY_IS:
	    safe_fprintf(fp, "%*s{%s}\n", pad, "", get_object_file_name(obj));
	    break;
	  default:
#ifdef DEBUG
	    if (cs == NULL)
		internal_error();
#endif
	    if (cs->c_srcfile->sf_inpndx != SF_GENERATED_INPNDX)
		s1 = cs->c_srcfile->sf_name->name;
	    else
		s1 = "";
	    safe_fprintf(fp, "%*s%s(%s)\n", pad, "", s1,
			 get_object_file_name(obj));
	    break;
	}
    }
} /* print_symbol */
/************************************************************************
 * Name: print_symbol_name
 *									*
 * Purpose: Print a symbol name and number.  If the symbol is internal,
 *		put < > around the name.
 *
 * Returns: Number of columns printed.
 *									*
 ************************************************************************/
static int
print_symbol_name(FILE *fp,
		  SYMBOL *sym,		/* Symbol whose name to print */
		  char *newline)	/* Extra string to print (usually
					   "\n" or ""). */
{
    int		n, len;

    if (sym->s_flags & S_NUMBER_USURPED)
	n = saved_stuff[sym->s_number].number;
    else
	n = sym->s_number;

    len = sym->s_name->len + 1 /* for 'S' */ + 5 /* for number */;

    if (sym->s_flags & S_HIDEXT) {
	safe_fprintf(fp, "S%-5d<%s>%s", n, sym->s_name->name, newline);
	len += 2;			/* Add 2 for '<' and '>' */
    }
    else
	safe_fprintf(fp, "S%-5d%s%s", n, sym->s_name->name, newline);

    return len;
}
/************************************************************************
 * Name: compare_rlds() - comparison routine for qsort() called by
 *		print_rlds()
 *
 *	Returns: 							*
 *									*
 ************************************************************************/
static int
compare_rlds(const void *a,
	     const void *b)
{
    return ((struct sort_rld_array_t *)a)->addr
	- ((struct sort_rld_array_t *)b)->addr;
}
/************************************************************************
 * Name: print_rlds
 *									*
 * Purpose: Print information about all RLDs referencing a symbol
 *	This function is only used for XREF (or SXREF).
 *
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
print_rlds(FILE *fp,
	   const int state_save_called,
	   SYMBOL *ref_sym)		/* Symbol being referenced */
{
    const int	n = ref_sym->s_number;
    int		d, n1;
    RLD		*rld;
    SYMBOL	*sym, *winning_symbol;
    int		diff;
    CSECT	*cs;
    size_t	i, i2;
    char	*rld_name;

    /* Sort the RLDs by address or by name. The sort_rlds_array is big
       enough for all RLDs, so it can never overflow. */
    d = 0;				/* Needed if !state_save_called */
    for (i = 0, n1 = n; n1 != -1; ++i, n1 = saved_stuff[n1].link) {
	sort_rlds_array[i].u.n = n1;
	rld = saved_stuff[n1].rld;
	cs = rld->r_csect;
	if (state_save_called)
	    d = cs->c_new_addr - cs->c_addr; /* Relocation amount */
	sort_rlds_array[i].addr = rld->r_addr + d;

	/* For the name of the symbol making the reference, we find the label
	   before and closest to the address of the RLD.  Because labels
	   aren't necessarily in address order, we have to check all labels.
	   If the first label is hidden and at the same
	   address as the csect, we'll never use the csect name.  If multiple
	   labels are at the same address, the first label (in symbol table
	   order) will be used. */
	sym = &rld->r_csect->c_symbol;
	/* Ignore hidden name on csect if first label is at same address. */
	if (sym->s_next_in_csect
	    && (sym->s_flags & S_HIDEXT)
	    && sym->s_next_in_csect->s_addr == sym->s_addr)
	    sym = sym->s_next_in_csect;
	winning_symbol = sym;		/* Default in case of corrupted
					   labels. */
	diff = cs->c_len;
	for ( ; sym; sym = sym->s_next_in_csect) {
	    if (rld->r_addr - sym->s_addr > 0
		&& rld->r_addr - sym->s_addr < diff) {
		winning_symbol = sym;
		diff = rld->r_addr - sym->s_addr;
	    }
	}
	sort_rlds_array[i].sym = winning_symbol;
    }
    qsort(sort_rlds_array, i, sizeof(sort_rlds_array[0]),
	  print_rlds_sort_routine);

    for (i2 = 0; i2 < i; i2++) {
	rld = saved_stuff[sort_rlds_array[i2].u.n].rld;
	rld_name = get_reltype_name(rld->r_reltype);

	safe_fprintf(fp, "%*s%08X %s(%d)%*s",
		     MAP_XREF_BEGIN - 1, "",
		     sort_rlds_array[i2].addr, rld_name, rld->r_length,
		     MAP_RLDTYPE_LEN - (strlen(rld_name) + 4
					- (rld->r_length < 10)), " ");

	(void) print_symbol_name(fp, sort_rlds_array[i2].sym,
				 "\n" /* Add newline */);
    }

    /* Restore s_number field */
    ref_sym->s_number = saved_stuff[ref_sym->s_number].number;
    ref_sym->s_flags &= ~S_NUMBER_USURPED;
} /* print_rlds */
/************************************************************************
 * Name: print_rld
 *									*
 * Purpose: Print information from an RLD entry.  This function is used
 *		if the map type is CALLS (or SCALLS).
 *
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
print_rld(FILE *fp,
	  const int state_save_called,
	  RLD *rld,			/* RLD to print */
	  SYMBOL *sym,			/* Name of referenced LD for SCALLS.
					   NULL for CALLS. */
	  unsigned long addr)		/* Address of referenced symbol,
					   if sym is not NULL. */
{
    char	*rld_name;
    unsigned long rld_addr;
    CSECT	*cs;

    cs = rld->r_csect;
    rld_name = get_reltype_name(rld->r_reltype);

    /* Compute new address of RLD */
    if (state_save_called)
	rld_addr = rld->r_addr + cs->c_new_addr - cs->c_addr;
    else
	rld_addr = rld->r_addr;

    /* For SCALLS, we will have already computed sym and addr.
       Otherwise, we do it here. */
    if (sym == NULL) {
	/* Find symbol referenced by RLD and its address */
	if (!state_save_called) {
	    sym = rld->r_sym;
	    addr = sym->s_addr;		/* Get address from ER.  It will be
					   non-zero if it is an XO symbol. */
	}
	else if (rld->r_flags & RLD_RESOLVE_BY_NAME) {
	    if (rld->r_sym->s_name->flags & STR_NO_DEF) {
		sym = rld->r_sym->s_name->refs;
		addr = 0;
	    }
	    else {
		sym = rld->r_sym->s_name->first_ext_sym;
		if ((sym->s_flags & S_LOCAL_GLUE)
		    && cs->c_symbol.s_smclass == XMC_DS)
		    sym = sym->s_synonym;
		addr = sym->s_addr +
		    sym->s_csect->c_new_addr - sym->s_csect->c_addr;
	    }
	}
	else {
	    sym = rld->r_sym->s_resolved;
	    if (sym == NULL)	/* Delcsect processing must have
				   deleted symbol. */
		return;
	    addr = sym->s_addr +
		sym->s_csect->c_new_addr - sym->s_csect->c_addr;
	}
    }

    safe_fprintf(fp, "    %08X%*s%08X %s(%d)%*s",
		 rld_addr,
		 MAP_XREF_BEGIN - 1 - (4 /* leading blanks */
				       + 8 /* addr */), "",
		 addr, rld_name, rld->r_length,
		 MAP_RLDTYPE_LEN - (strlen(rld_name) + 4
				    - (rld->r_length < 10)), " ");
    (void) print_symbol_name(fp, sym, "\n" /* Add newline */);
} /* print_rld */
/************************************************************************
 * Name: restore_numbers
 *
 * Purpose: Restore s_number fields and reset S_NUMBER_USURPED field.  This
 *	is needed if an XREF map fails prematurely.
 *
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
restore_numbers2(STR *name)
{
    SYMBOL *sym;

    for (sym = name->first_ext_sym; sym; sym = sym->s_synonym)
	if (sym->s_flags & S_NUMBER_USURPED) {
	    sym->s_number = saved_stuff[sym->s_number].number;
	    sym->s_flags &= ~S_NUMBER_USURPED;
	}
    for (sym = name->first_hid_sym; sym; sym = sym->s_synonym)
	if (sym->s_flags & S_NUMBER_USURPED) {
	    sym->s_number = saved_stuff[sym->s_number].number;
	    sym->s_flags &= ~S_NUMBER_USURPED;
	}
    for (sym = name->refs; sym; sym = sym->s_synonym)
	if (sym->s_flags & S_NUMBER_USURPED) {
	    sym->s_number = saved_stuff[sym->s_number].number;
	    sym->s_flags &= ~S_NUMBER_USURPED;
	}
}
static void
restore_numbers(void)
{
    int		k;
    HASH_STR	*root, *h;
    STR		*name;

    restore_numbers2(&NULL_STR);
    if (NULL_STR.alternate)
	restore_numbers2(NULL_STR.alternate);
    for_all_STRs(root, k, h, name)
	restore_numbers2(name);
} /* restore_numbers */
