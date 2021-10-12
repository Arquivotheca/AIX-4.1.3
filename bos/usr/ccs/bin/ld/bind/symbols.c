#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)07	1.22  src/bos/usr/ccs/bin/ld/bind/symbols.c, cmdld, bos411, 9428A410j 5/9/94 11:03:28")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
 *
 *   Symbol table access routines (#ifdef ALIGNPTRS only):
 *		aligned_aux_ptr
 *		aligned_sym_ptr
 *   String table access routines:
 *		set_strtab_base
 *		get_sym_name
 *		get_sym_name2
 *		get_aux_name
 *		read_name (static)
 *   TYPECHK routines:
 *		create_TYPECHK
 *   SRCFILE routines:
 *		get_init_SRCFILE
 *   SYMBOL routines:
 *		bad_symbol_offset (static)
 *		get_ER
 *		get_SYMBOL (static)
 *		get_SYMBOLs
 *		create_SYMBOL
 *		create_imported_SYMBOL
 *		create_er_SYMBOL
 *		create_global_archive_SYMBOL
 *		total_symbols_allocated
 *		total_ers_allocated
 *   CSECT routines:
 *		get_CSECTs
 *		get_init_CSECT
 *		create_CSECT
 *		free_csect
 *		total_csects_allocated
 *   RLD routines:
 *		total_rlds_allocated
 *		get_RLDs
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

#include <stdio.h>
#include <string.h>
#include <limits.h>

#ifdef lint
extern void *alloca(size_t);
#else
#ifdef CENTERLINE
extern void *alloca();
#else
#pragma alloca
#endif
#endif

#include "bind.h"
#include "global.h"
#include "error.h"
#include "strs.h"
#include "symbols.h"
#include "objects.h"
#include "stats.h"

/* Static Variables */
static int	num_available_csects;
static int	num_available_symbols;
static int	total_symbols;
static int	total_csects;
static int	total_ers;
static int	total_rlds;
static caddr_t	strtab_base;
static CSECT	*free_csects;
static CSECT	*available_csects;
static SYMBOL	*available_symbols;

/* Parameters for allocation symbols in blocks */
#define MAX_SYMBOL_CHUNK 1000
#define MIN_SYMBOL_CHUNK 100

/************************************************************************
 * Name: set_strtab_base
 *									*
 * Purpose: Set static variable with base address of string table.
 *	Must be called for each file before calling one of the get_*_name
 *	routines.
 *									*
 * Returns: Nothing
 *									*
 ************************************************************************/
void
set_strtab_base(OBJECT *obj)
{
    if (obj->oi_strtab_len == 0)
	strtab_base = (caddr_t)0xFFFFFFFF;
    else
	strtab_base = obj->o_ifile->i_map_addr + (long)obj->oi_strtab_base;
}

/************************************************************************
 * SYMBOL TABLE ACCESS ROUTINES
 ************************************************************************/
#ifdef ALIGNPTRS
/************************************************************************
 * Name: aligned_sym_pointer
 *
 * Purpose: Return a pointer to an aligned symbol table entry.  Copies the
 *	entry, if necessary, to a static variable.  Subsequent calls may
 *	overwrite this buffer.
 *
 * Returns: Pointer to the symbol table entry.
 ************************************************************************/
SYMENT *
aligned_sym_ptr(caddr_t base,
		int n)
{
    static SYMENT	stemp;
    void		*s;

    s = (void *)(base + n * SYMESZ);
    return (s&0x3) ? memcpy(&stemp, s, SYMESZ) : s;
}
/************************************************************************
 * Name: aligned_aux_pointer
 *
 * Purpose: Return a pointer to an aligned symbol table auxiliary entry.
 *	Copies the entry, if necessary, to a static variable.
 *	Subsequent calls may overwrite this buffer.
 *
 * Returns: Pointer to the auxiliary symbol table entry.
 ************************************************************************/
AUXENT *
aligned_aux_ptr(caddr_t base,
		int n)
{
    static AUXENT	atemp;
    void		*s;

    s = (void *)(base + n * AUXESZ);
    return (s&0x3) ? memcpy(&atemp, s, AUXESZ) : s;
}
#endif

/************************************************************************
 * STRING TABLE ACCESS ROUTINES
 ************************************************************************/
/************************************************************************
 * Name: bad_symbol_offset
 *									*
 * Purpose: Print an error for a bad n_offset
 *									*
 * Returns: Pointer to STR for symbol named '?BAD?'
 *									*
 ************************************************************************/
static STR *
bad_symbol_offset(long n_off,
		  int n,
		  OBJECT *obj)
{
    bind_err(SAY_NORMAL, RC_SEVERE,
	     NLSMSG(BAD_SYMBOL_NAME,
    "%1$s: 0711-251 SEVERE ERROR: Symbol table entry %2$d in object %3$s:\n"
    "\tField n_offset contains %4$d. Valid values are between 4 and %5$d."),
	     Main_command_name,
	     n, get_object_file_name(obj),
	     n_off, obj->oi_strtab_len - 1);
    return putstring("?BAD?");
}
/************************************************************************
 * Name: get_sym_name							*
 *									*
 * Purpose: Read a symbol name.  The string table for the symbol must
 *	be addressable by the "strtab_base" variable.
 *									*
 * Returns: Pointer to STR structure for symbol name.
 *									*
 ************************************************************************/
STR *
get_sym_name(OBJECT *obj,		/* object defining symbol */
	     SYMENT *sym_ptr,		/* symbol table entry */
	     int n)			/* Symbol table index */
{
    if (sym_ptr->n_zeroes != 0)		/* Name is in symbol table entry.  */
	return putstring_len(sym_ptr->n_name, SYMNMLEN);
    if (sym_ptr->n_offset == 0)
	return &NULL_STR;		/* Null name */

    /* The symbol name is in the string table. */
    if (sym_ptr->n_offset < 4
	|| sym_ptr->n_offset > obj->oi_strtab_len)
	return bad_symbol_offset(sym_ptr->n_offset, n, obj);

    return putstring_len(strtab_base + sym_ptr->n_offset,
			 obj->oi_strtab_len - sym_ptr->n_offset);
} /* get_sym_name */

#ifdef READ_FILE
/************************************************************************
 * Name: read_name							*
 *									*
 * Purpose: Read a symbol name from the string table.  The entire string
 *	table is not in memory, so it is read a name at a time.
 *									*
 * Returns: Pointer to STR for symbol name.
 *									*
 ************************************************************************/
static STR *
read_name(off_t str_offset,
	  off_t max_offset,
	  OBJECT *obj,			/* Used for error messages. */
	  int n)			/* Symbol table entry number */
{
    /* assert(I_ACCESS == READ_FILE) */
    int		l, i, c;
    char	*str_base;
    off_t	position;

    l = max_offset - str_offset;
    str_base = alloca(l);		/* Allocate a temporary work area */

    position = ftell(obj->o_ifile->i_file); /* Save position */
    i = 0;
    safe_fseek(obj->o_ifile,
	       obj->oi_symtab_offset + obj->oi_num_symbols*SYMESZ + str_offset,
	       SEEK_SET);

    /* Read characters up to end of symbol table or to NULL. */
    while (l-- && (c = getc(obj->o_ifile->i_file)) != EOF && c != '\0')
	str_base[i++] = c;

    safe_fseek(obj->o_ifile, position, SEEK_SET); /* Restore */

    /* If string was not null terminated, report an error. */
    if (c != '\0') {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(BAD_SYMBOL_NAME2,
"%1$s: 0711-255 SEVERE ERROR: Symbol table entry %2$d in object %3$s:\n"
"\tThe symbol name was not null-terminated. Either the string table is\n"
"\tdamaged or field n_offset (which contains %4$d) is invalid."),
	     Main_command_name,
	     n, get_object_file_name(obj),
	     str_offset);
	return putstring("?BAD?");
    }

    return putstring_len(str_base, i);
}
#endif
/************************************************************************
 * Name: get_sym_name2							*
 *									*
 * Purpose: Read a symbol name.  This routine is identical to get_sym_name,
 *	except that if I_ACCESS==I_READ and the symbol is in the
 *	string table, we must read the name explicitly.  (If the file is
 *	mapped, the string is obtained directly.)
 *									*
 * Returns: Pointer to STR for symbol name.
 *									*
 ************************************************************************/
STR *
get_sym_name2(OBJECT *obj,		/* object symbol is from */
	      SYMENT *sym_ptr,
	      int read_object,		/* IFILE_ACCESS == I_READ? */
	      int n)			/* Symbol table entry number */
{
    if (sym_ptr->n_zeroes != 0)	/* name in syment */
	return putstring_len(sym_ptr->n_name, SYMNMLEN);
    else if (sym_ptr->n_offset == 0)
	return &NULL_STR;

    /* The symbol name is in the string table */
    else if (sym_ptr->n_offset < 4
	     || sym_ptr->n_offset > obj->oi_strtab_len)
	return bad_symbol_offset(sym_ptr->n_offset, n, obj);
#ifdef READ_FILE
    else if (read_object)
	return read_name(sym_ptr->n_offset, obj->oi_strtab_len, obj, n);
#endif
    else
	return putstring_len(strtab_base + sym_ptr->n_offset,
			     obj->oi_strtab_len - sym_ptr->n_offset);
} /* get_sym_name2 */
/************************************************************************
 * Name: get_aux_name							*
 *
 * Purpose: Read a file name from a C_FILE auxiliary entry.
 *
 * Returns: Pointer to STR for file name.
 ************************************************************************/
STR *
get_aux_name(OBJECT *obj,		/* object that symbol is from */
	     AUXENT *aux_ptr,		/* auxiliary entry */
	     int read_object,		/* If not null, we're reading file and
					   the string table has not been read
					   into memory, so strtab_base is not
					   valid. */
	     int n)
{
    if (aux_ptr->x_file._x.x_zeroes != 0) /* name is in auxent itself */
	return putstring_len(aux_ptr->x_file.x_fname, FILNMLEN);
    else if (aux_ptr->x_file._x.x_offset == 0)
	return &NULL_STR;
    /* The file name is in the string table */
    else if ((unsigned long)aux_ptr->x_file._x.x_offset > obj->oi_strtab_len) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(BAD_AUX_NAME,
 "%1$s: 0711-252 SEVERE ERROR: File auxiliary symbol entry %2$d in object %3$s:\n"
 "\tField x_offset contains %4$d. Valid values are between 4 and %5$d.\n"
 "\tThe object name is being substituted."),
		 Main_command_name,
		 n,
		 get_object_file_name(obj),
		 aux_ptr->x_file._x.x_offset,
		 obj->oi_strtab_len - 1);
	return putstring(get_object_file_name(obj));
    }
#ifdef READ_FILE
    else if (read_object)
	return read_name(aux_ptr->x_file._x.x_offset, obj->oi_strtab_len,
			 obj, n);
#endif
    else
	return putstring_len(strtab_base + aux_ptr->x_file._x.x_offset,
			     obj->oi_strtab_len - aux_ptr->x_file._x.x_offset);
} /* get_aux_name */

/************************************************************************
 * TYPECHK ROUTINES
 ************************************************************************/
/************************************************************************
 * Name: create_TYPECHK
 *
 * Purpose: Read the typchk-string from the .typchk section for a symbol.
 *	Assume a value for x_snhash if it's 0 but a .typchk section exists.
 *
 * Returns: Pointer to TYPECHK structure.
 ************************************************************************/
TYPECHK *
create_TYPECHK(AUXENT *a,
	       OBJECT *o,
	       int n,
	       STR *name)
{
    int s = a->x_csect.x_snhash;

    if (a->x_csect.x_parmhash == 0) {
	/* No hash string. */
#ifdef DEBUG
	if (s != 0 && Switches.verbose) {
	    bind_err(SAY_NO_NLS, RC_WARNING,
 "%1$s: WARNING: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tThe x_parmhash field is 0 but the x_snhash field is nonzero.",
		     Main_command_name,
		     name->name, n,
		     get_object_file_name(o));
	}
#endif
	return NULL;
    }

    /* Because of some XLC compilers, x_snhash may be 0 even for a valid
       hash string.  Therefore, if any .typchk section exists, we assume
       it's the proper section. */
    if (hash_section > 0 && s == 0)
	s = hash_section;

    if (s >= 1 && s <= o->oi_num_sections
	&& o->oi_section_info[s-1].sect_type == STYP_TYPCHK)
	return
	    put_TYPECHK(o->oi_section_info[s-1].u.sect_base
			+a->x_csect.x_parmhash,
	       o->oi_section_info[s-1].sect_size-a->x_csect.x_parmhash);
    else {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(BAD_SNHASH,
"%1$s: 0711-253 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
"\tx_snhash=%5$d, which is not the section number of a .typchk section"),
		 Main_command_name,
		 name->name, n,
		 get_object_file_name(o),
		 s);

	return NULL;
    }
}

/************************************************************************
 * SRCFILE ROUTINES
 ************************************************************************/

#ifdef DEBUG
/* Define a static initialized variable to try to catch changes to the
   structure without corresponding code changes */
static SRCFILE initialized_srcfile = {
    &initialized_srcfile,		/* sf_next */
    &NULL_STR,				/* sf_name */
    INT_MAX-1,				/* sf_inpndx */
    (CSECT *)NULL,			/* sf_csect */
    (OBJECT *)NULL			/* sf_object */
#ifdef _CPUTYPE_FEATURE
    , SF_USED				/* sf_flags */
    , TCPU_PWR				/* sf_cputype */
#endif
};
#endif

/************************************************************************
 * Name: get_init_SRCFILE
 * Purpose: Allocate a new, initialized SRCFILE structure.
 * Returns: The new SRCFILE structure.
 ************************************************************************/
SRCFILE *
get_init_SRCFILE(OBJECT *obj,
		 STR *name)
{
    char *id = "get_init_SRCFILE";

    SRCFILE *sf = get_memory(sizeof(SRCFILE), 1, SRCFILES_ID, id);
    STAT_use(SRCFILES_ID, 1);

    sf->sf_next = NULL;
    sf->sf_name = name;
    sf->sf_inpndx = SF_GENERATED_INPNDX;
    sf->sf_csect = NULL;
    sf->sf_object = obj;
#ifdef _CPUTYPE_FEATURE
    sf->sf_cputype = 0;
    sf->sf_flags = 0;
#endif
    return sf;
}

/************************************************************************
 * SYMBOL routines
 ************************************************************************/
#ifdef DEBUG
/* Initialize a dummy symbol to try to catch errors if the
   structure of SYMBOL changes. */
static SYMBOL initialized_s = {
    &NULL_STR,				/* s_name */
    &initialized_s,			/* s_next_in_csect */
    &initialized_s,			/* s_synonym */
    0xdeadbead,				/* s_addr */
    INPNDX_GENERATED,			/* s_inpndx */
    INT_MAX-1,				/* s_number */
    XMC_PR,				/* s_smclass */
    XTY_SD,				/* s_smtype */
    S_DUPLICATE,			/* s_flags */
    (OBJECT *)NULL,			/* s_object */
    (CSECT *)NULL,			/* s_csect */
    (TYPECHK *)NULL,			/* s_typechk */
    &initialized_s,			/* s_resolved */
};
#endif
/************************************************************************
 * Name: get_symbol
 * Purpose: Return an uninitialized SYMBOL structure.  Number the symbol.
 *	Symbol numbers are never reused.
 *
 * Returns: Pointer to uninitialized (except for s_number) SYMBOL structure
 ************************************************************************/
static SYMBOL *
get_SYMBOL(void)
{
    char *id = "get_SYMBOL";

    if (num_available_symbols)		/* Symbols already allocated? */
	num_available_symbols--;
    else {
	if (Size_estimates.num_symbols) {
	    /* Allocate a block of symbols */
	    num_available_symbols = Size_estimates.num_symbols - 1;
	    Size_estimates.num_symbols = 0;
	}
	available_symbols = get_memory(sizeof(SYMBOL),
				       num_available_symbols + 1,
				       SYMBOLS_ID, id);
    }
    STAT_use(SYMBOLS_ID, 1);
    return available_symbols++;
}
/************************************************************************
 * Name: get_SYMBOLs
 *
 * Purpose: Allocate "n" consecutive, uninitialized SYMBOL structures,
 *	except for the s_number fields.
 *
 * Returns: Pointer to array of symbols.
 ************************************************************************/
SYMBOL *
get_SYMBOLs(int n)
{
    char	*id = "get_SYMBOLs";
    SYMBOL	*syms;

    if (num_available_symbols >= n) {
	/* A block of symbols is already allocated */
	num_available_symbols -= n;
	syms = available_symbols;
	available_symbols += n;
    }
    else {
	Size_estimates.num_symbols -= n;
	if (Size_estimates.num_symbols < 0)
	    Size_estimates.num_symbols = 0;
	syms = get_memory(sizeof(SYMBOL), n, SYMBOLS_ID, id);
    }
    STAT_use(SYMBOLS_ID, n);
    while (n-- > 0)			/* Number the symbols */
	syms[n].s_number = ++total_symbols;
    return syms;
}
/************************************************************************
 * Name: create_SYMBOL
 *
 * Purpose: Return a pointer to an initialized SYMBOL structure.  If the
 *	SYMBOL is part of the CSECT structure, its address is passed.
 *	Otherwise, a new SYMBOL is allocated.
 *
 *
 * Returns: Pointer to SYMBOL
 ************************************************************************/
SYMBOL *
create_SYMBOL(OBJECT *obj,		/* Object containing symbol */
	      CSECT *c,			/* Csect containing symbol */
	      SYMBOL *s,		/* Symbol to be filled in (optional) */
	      int n,			/* Symbol table index */
	      STR *name,		/* Name of symbol */
	      SYMENT *sym,		/* Symbol table entry */
	      AUXENT *auxsym,		/* CSECT auxiliary entry for symbol */
	      const int s_flags_init)	/* Equal to S_ARCHIVE_SYMBOL if
					   an archive member is being read. */
{
    if (s == NULL) {
	s = get_SYMBOL();
	s->s_number = ++total_symbols;
    }

    s->s_name = name;
    s->s_next_in_csect = NULL;
    s->s_synonym = NULL;
    s->s_addr = sym->n_value;
    s->s_inpndx = n;
    /* s->s_number = Value filled in above. */
    s->s_smclass = auxsym->x_csect.x_smclas;
    s->s_smtype = auxsym->x_csect.x_smtyp & 3;

    s->s_csect = c;
#if DEBUG != 0 && CENTERLINE == 0
    /* ifndef DEBUG, s_object and s_csect are members of a union. */
    s->s_object = (OBJECT *)0xdeadfeed;
#endif

    if (sym->n_sclass == C_HIDEXT) {
	s->s_flags = S_HIDEXT;
	s->s_resolved = NULL;
#if DEBUG != 0 && CENTERLINE == 0
	/* Ifndef DEBUG, s_resolved, s_typechk, and s_prev_in_gst
	   are members of a union. */
	s->s_typechk = (TYPECHK *)0xdeadfeed;
	s->s_prev_in_gst = (SYMBOL *)0xdeadfeed;
#endif

	if (Switches.verbose
	    && create_TYPECHK(auxsym, obj, n, name)
	    /* Some old library members have typecheck strings for TOC entries
	       and the TOC anchor, the current binder inserts the hash for a
	       glue LD, and some versions of the XLC compiler have hash values
	       for hidden symbols, so this validation check may produce
	       spurious messages.  We just make sure that x_snhash was really
	       non-zero (and not implied by the existence of a .typchk section
	       before printing the error. */
	    && auxsym->x_csect.x_snhash != 0) {
	    bind_err(SAY_NORMAL, RC_WARNING,
		     NLSMSG(VALIDATE_EXTRA_TYPECHK,
 "%1$s: 0711-254 WARNING: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tType-checking strings for C_HIDEXT symbols are ignored."),
		     Main_command_name,
		     name->name, n,
		     get_object_file_name(obj));
	}
    }
    else {
	s->s_flags = s_flags_init;
	s->s_typechk = create_TYPECHK(auxsym, obj, n, name);
#if DEBUG != 0 && CENTERLINE == 0
	/* Ifndef DEBUG, s_resolved, s_typechk, and s_prev_in_gst
	   are members of a union. */
	s->s_resolved = (SYMBOL *)0xdeadfeed;
	s->s_prev_in_gst = (SYMBOL *)0xdeadfeed;
#endif
    }

    return s;
}
/************************************************************************
 * Name: create_imported_SYMBOL
 *
 * Purpose: Return an initialized SYMBOL for an imported name.
 *	The symbol is linked into the first_ext_sym list for its name.
 *
 *	This routine should only be used for imported symbols, because
 *	no delcsect processing is done.
 *
 * Returns: Pointer to SYMBOL
 ************************************************************************/
SYMBOL *
create_imported_SYMBOL(STR *name,	/* Symbol name */
		       CSECT *cs,	/* Csect containing symbol */
		       SYMBOL *prev_sym, /* Previous symbol from same import
					    file or shared object (if any) */
		       unsigned char smclass, /* Storage-mapping class of sym */
		       unsigned int import_addr) /* Address if XMC_XO */
{
    SYMBOL *s;

    if (prev_sym == NULL)
	s = &cs->c_symbol;
    else {
	s = get_SYMBOL();
	s->s_number = ++total_symbols;
	prev_sym->s_next_in_csect = s;
    }

    s->s_name = name;
    s->s_next_in_csect = NULL;
    s->s_synonym = NULL;
    s->s_inpndx = INPNDX_IMPORT;
    s->s_csect = cs;

#if DEBUG != 0 && CENTERLINE == 0
    /* ifndef DEBUG, s_object and s_csect are members of a union. */
    s->s_object = (OBJECT *)0xfeedbeef;
    /* Ifndef DEBUG, s_resolved, s_typechk, and s_prev_in_gst
       are members of a union. */
    s->s_resolved = (SYMBOL *)0xfeedbeef;
    s->s_prev_in_gst = (SYMBOL *)0xdeadfeed;
#endif

    /* The following fields must be set by the caller:
       s->s_smtype
       s->s_typechk

       The s->s_flags field is initialized to 0, and S_XMC_XO is set if
       appropriate.  Other flags must be set by the caller.
    */

    /* Assign storage-mapping class and address */
    if (smclass == XMC_XO) {
	s->s_flags = S_XMC_XO;
	s->s_addr = import_addr;
	s->s_smclass = XMC_XO;
	if (Switches.verbose)
	    say(SAY_NORMAL,
		NLSMSG(SHARED_IMPORT_ADDRESS,
		       "%1$s: Symbol imported (at address 0x%2$08X): %3$s"),
		Command_name, import_addr, name->name);

    }
    else {
	s->s_addr = 0;
	s->s_flags = 0;
	s->s_smclass = smclass;
	if (Switches.verbose)
	    say(SAY_NORMAL,
		NLSMSG(SHARED_IMPORT, "%1$s: Symbol imported: %2$s"),
		Command_name, name->name);
    }

    /* Add symbol to end of chain of external symbols. */
    if (prev_sym = name->first_ext_sym) {
	DEBUG_MSG(SYMBOLS_DEBUG,
		  (SAY_NO_NLS,
		   "Duplicate symbol (in create_imported_SYMBOL) %s",
		   name->name));
	while (prev_sym->s_synonym)	/* Find end of chain. */
	    prev_sym = prev_sym->s_synonym;
	prev_sym->s_synonym = s;
    }
    else
	name->first_ext_sym = s;	/* First external symbol */

    return s;
} /* create_imported_SYMBOL */
/************************************************************************
 * Name: create_global_archive_SYMBOL
 * Purpose: Get a file name from an auxiliary entry.
 * Returns: Pointer to OBJECT and AUXENT for name
 ************************************************************************/
SYMBOL *
create_global_archive_SYMBOL(STR *name,
			     OBJECT *obj)
{
    OBJECT *prev_obj;
    SYMBOL *s, *prev_sym, *back_sym;

    s = get_SYMBOL();
    s->s_number = ++total_symbols;
    s->s_name = name;
    s->s_synonym = NULL;
    s->s_object = obj;

#if DEBUG != 0 && CENTERLINE == 0
    /* ifndef DEBUG, s_object and s_csect are members of a union. */
    s->s_csect = (CSECT *)0xdeadfee0;
#endif

    prev_sym = name->first_ext_sym;
    if (prev_sym == NULL)
	name->first_ext_sym = s;	/* First external symbol */
    else {
	/* We are entering a duplicate symbol read from the global symbol
	   table of an archive.  Script (import) files and shared objects
	   have already been read, so the current symbol will not
	   necessarily be entered at the end of the chain, but might have
	   to be entered before an existing symbol. */
	DEBUG_MSG(SYMBOLS_DEBUG,
		  (SAY_NO_NLS,
		   "Duplicate symbol (in create_global_archive_SYMBOL) %s",
		   name->name));
	back_sym = NULL;
	do {
	    if (prev_sym->s_smtype == XTY_AR)
		prev_obj = prev_sym->s_object;
	    else
		prev_obj = prev_sym->s_csect->c_srcfile->sf_object;

	    if (prev_obj->o_ifile == obj->o_ifile) {
		/* Symbols are from same file, so their objects
		   were allocated contiguously.  */
		if (obj < prev_obj) {
		    /* New symbol is first */
		    s->s_synonym = prev_sym;
		    break;
		}
	    }
	    back_sym = prev_sym;
	    prev_sym = prev_sym->s_synonym;
	} while (prev_sym);

	if (back_sym)
	    back_sym->s_synonym = s;
	else
	    name->first_ext_sym = s;	/* Symbol goes at head of list */
    }
    return s;
}
/************************************************************************
 * Name: get_ER
 *
 * Purpose:
 *
 * Returns:
 ************************************************************************/
SYMBOL *
get_ER(void)
{
    SYMBOL *er;

    er = get_SYMBOL();

    er->er_flags = 0;
    er->er_number = ++total_ers;
    er->s_smtype = XTY_ER;

#if DEBUG != 0 && CENTERLINE == 0
    /* ifndef DEBUG, s_csect is unioned to er_object (a.k.a. s_object)
       and s_resolved and s_prev_in_gst are unioned to er_typechk
       (a.k.a. s_typechk) */
    er->s_csect = (CSECT *)0xdeadfeed;
    er->s_resolved = (SYMBOL *)0xdeadfeed;
    er->s_prev_in_gst = (SYMBOL *)0xdeadfeed;
#endif

    return er;
}
/************************************************************************
 * Name:	create_er_SYMBOL
 *
 * Purpose:	Create and return an external reference symbol, initialized
 *		with information from a symbol table entry.
 *
 *		The er_next_in_object field is not initialized.
 *
 * Returns:	The new SYMBOL.
 *
 ************************************************************************/
SYMBOL *
create_er_SYMBOL(STR *name,
		 int n,
		 OBJECT *obj,
		 AUXENT *auxsym)
{
    SYMBOL *er;

    er = get_ER();

    er->er_name = name;
    /*er->er_next_in_object = NULL;*/	/* Assigned by caller */

    /* References are linked in reverse order, because order doesn't matter. */
    er->er_synonym = name->refs;
    name->refs = er;

    /* er->s_addr = 0; */		/* Assigned by caller. */
    er->er_inpndx = n;
    er->er_smclass = auxsym->x_csect.x_smclas;
    er->er_object = obj;
    er->er_typechk = create_TYPECHK(auxsym, obj, n, name);

    return er;
}
/************************************************************************
 * Name: total_symbols_allocated
 * Purpose:
 * Returns:
 ************************************************************************/
int
total_symbols_allocated(void)
{
    return total_symbols;
}
/************************************************************************
 * Name: total_ers_allocated
 * Purpose:
 * Returns:
 ************************************************************************/
int
total_ers_allocated(void)
{
    return total_ers;
}
/************************************************************************
 * CSECT routines
 ************************************************************************/
#ifdef DEBUG
/* Initialize a dummy csect to try to catch errors if the
   structure of CSECT changes. */
static CSECT tmp_CSECT = {
    999,				/* c_len */
    1,					/* c_TD_ref */
    0,					/* c_mark */
    1,					/* c_save */
    31,					/* c_align */
    255,				/* c_major_sect */
    N_UNKNOWN,				/* c_secnum */
    (RLD *)NULL,			/* c_first_rld */
    0xdeadbeef,				/* c_addr */
    0xdeadfeed,				/* c_new_addr */
    (SRCFILE *)NULL,			/* c_srcfile */
    &tmp_CSECT,				/* c_next */
    {					/* c_symbol */
	&NULL_STR			/* Remainder of SYMBOL can be caught
					   by tmp_SYMBOL initialization. */
	}
};
#endif
/************************************************************************
 * Name: total_csects_allocated
 * Purpose:
 * Returns:
 ************************************************************************/
int
total_csects_allocated(void)
{
    return total_csects;
}
/************************************************************************
 * Name: get_CSECTs
 * Purpose: Allocate an uninitialized CSECT
 *	The SYMBOLs in the CSECTs have their s_number fields initialized.
 *
 * Returns: Pointer to CSECT
 ************************************************************************/
CSECT *
get_CSECTs(int n)
{
    char	*id = "get_CSECTs";
    CSECT	*csects;

    DEBUG_MSG(SYMBOLS_DEBUG,
	      (SAY_NO_NLS, "Need %d csects; have %d; estimate %d",
	       n, num_available_csects, Size_estimates.num_csects));

    if (free_csects && n == 1) {
	csects = free_csects;
    	free_csects = csects->c_next;
    }
    else {
	if (num_available_csects < n) {
	    if (num_available_csects != 0) {
		Size_estimates.num_csects -= n;
		csects = get_memory(sizeof(CSECT), n, CSECTS_ID, id);
		goto return_it;
	    }
	    num_available_csects = max(n,
				       min(MAX_SYMBOL_CHUNK,
					   max(MIN_SYMBOL_CHUNK,
					       Size_estimates.num_csects)));
	    Size_estimates.num_csects -= num_available_csects;

	    available_csects = get_memory(sizeof(CSECT), num_available_csects,
					  CSECTS_ID, id);
	}

	num_available_csects -= n;
	csects = available_csects;
	available_csects += n;
    }
  return_it:
    STAT_use(CSECTS_ID, n);
    total_csects += n;

    /* Initialize the s_number fields in the SYMBOLs in the CSECTs */
    while (n-- > 0)
	csects[n].c_symbol.s_number = ++total_symbols;
    return csects;
}
/************************************************************************
 * Name: free_csect
 * Purpose: Save an unneeded csect on a free chain.
 *
 * Returns: Nothing
 ************************************************************************/
void
free_csect(CSECT *cs)
{
    cs->c_next = free_csects;
    free_csects = cs;
    --total_csects;
    STAT_free(CSECTS_ID, 1);
}
/************************************************************************
 * Name: get_init_CSECT
 * Purpose: Create a new CSECT and initialize all its fields except for
 *		c_srcfile, c_symbol, and c_major_sect.
 * Returns: Pointer to the new CSECT
 ************************************************************************/
CSECT *
get_init_CSECT(void)
{
    CSECT *cs = get_CSECTs(1);

    cs->c_len = 0;
    cs->c_TD_ref =
	cs->c_mark =
	    cs->c_save = 0;
    cs->c_align = 0;
    /* cs->c_major_sect = 0; Set during SAVE processing. */
    cs->c_secnum = N_UNKNOWN;
    cs->c_first_rld = NULL;
    cs->c_addr =
	cs->c_new_addr = 0;
    /*cs->c_srcfile = NULL;*/		/* leave uninitialized. */
    cs->c_next = NULL;
    /* cs->c_symbol is not initialized */
    return cs;
}
/************************************************************************
 * Name: create_CSECT
 * Purpose: Create a new CSECT and initialize it with information from
 *	a symbol table entry and its associated auxiliary entry.
 *	Do not initialize the SYMBOL portion of the CSECT.
 *
 * Returns: Pointer to the new CSECT
 ************************************************************************/
CSECT *
create_CSECT(SYMENT *s,
	     AUXENT *auxsym)
{
    CSECT *cs = get_CSECTs(1);

    cs->c_len = auxsym->x_csect.x_scnlen;
    cs->c_TD_ref = 0;
    cs->c_mark = 0;
    cs->c_save = 0;
    cs->c_align = (auxsym->x_csect.x_smtyp >> 3) & 0x1F;
    /* cs->c_major_sect = 0; Set during SAVE processing. */
    cs->c_secnum = s->n_scnum;
    cs->c_first_rld = NULL;
    cs->c_addr = s->n_value;
    cs->c_new_addr = 0;
    cs->c_srcfile = NULL;
    cs->c_next = NULL;
    /* cs->c_symbol structure is not initialized */
    return cs;
}

/************************************************************************
 * RLD routines
 ************************************************************************/
/************************************************************************
 * Name: total_rlds_allocated
 * Purpose:
 * Returns:
 ************************************************************************/
int
total_rlds_allocated(void)
{
    return total_rlds;
}
/************************************************************************
 * Name: get_RLDs							*
 *									*
 * Purpose: Allocate an array of n new, uninitialized RLD entries.
 *	Number the fld entries.
 *
 * Returns: Pointer to first element of array
 ************************************************************************/
RLD *
get_RLDs(int n) {			/* Assert n > 0 */
    char	*id = "get_RLDs";
    RLD		*r, *r_temp;

    r = r_temp = get_memory(sizeof(RLD), n, RLDS_ID, id);
    while(n-- > 0)
	(r_temp++)->r_number = ++total_rlds;
    return r;
}
