#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)08	1.28  src/bos/usr/ccs/bin/ld/bind/symtab_save.c, cmdld, bos41J, 9512A_all 3/21/95 10:24:09")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: do_continuation
 *		fixup_symbol_table
 *		generate_symbol_table
 *		init_typchk_info
 *		write_exception_section
 *		write_info_section
 *		write_line_numbers
 *		write_symbol_table
 *
 *   STATIC FUNCTIONS:
 *		DEBUG_loc_check (#ifdef DEBUG only)
 *		add_symbol_name
 *		build_er
 *		check_toc
 *		convert_ln_address
 *		copy_debug_esds
 *		csect_esds
 *		csect_or_label_esd
 *		fcn_lnnos
 *		file_esds
 *		fixup_bincl_eincl
 *		fixup_statics
 *		free_lnranges
 *		generate_ers
 *		get_fixup_entry
 *		init_symtab_info
 *		make_csect_auxtab
 *		new_line_number_info
 *		save_bincl_eincl
 *		save_fixup
 *		save_info
 *		save_range
 *		toc_csect
 *		write_typchk_section
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
#include <stddef.h>
#include <string.h>

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
#include "strs.h"
#include "error.h"
#include "global.h"
#include "save.h"
#include "stab.h"
#include "symbols.h"
#include "objects.h"
#include "util.h"
#include "ifiles.h"

/* Defines for the info section */
#define INFO_LENGTH_FIELD_T int32
#define INFO_LEN_FIELD_LEN 4

/* Sizes for saving blocks of information */
#define FIXUP_BLOCK_BUMP	100
#define INFO_BLOCK_BUMP		50
#define LNNO_BLOCK_BUMP		500
#define LNRANGE_BLOCK_BUMP	50
#define BINCL_BLOCK_BUMP	10

/* Static structures for saving fixup information */
#define NUM_FIXUP_TYPES 1
struct info_info {
    X_OFF_T	len;
    X_OFF_T	in_offset;
    SYMENT	*syment;		/* Symbol in output file (for
					   updating section number). */
};

typedef struct fixup_entry {
    OBJECT	*in_obj;
    union {
	struct info_info ii;
    } u;
} FIXUP_ENTRY;

typedef struct fixup_entries {
    struct fixup_entries	*next;
    int				in_use;
    FIXUP_ENTRY			*a;
} FIXUP_ENTRIES;

struct addr_info {
	OBJECT *in_obj;
	long new_index;
	off_t last;		/* File offset to byte past last lnno in sect.
				   If 0, no line_number info.  If < -1, no
				   exception info. If -1, neither kind of
				   information is provided.  (This should never
				   happen.) */
	X_VADDR_T delta;
};
struct fixup_block {
	struct fixup_block *next;
	long in_use;
	struct {
		unsigned long *fixup_value_addr;
		SYMBOL *ref_sym;
		uint32 input_sym_number;
	} block[FIXUP_BLOCK_BUMP];
};
struct lno_block {
	struct lno_block *next;
	long in_use;
	struct addr_info block[LNNO_BLOCK_BUMP];
};
typedef struct ln_range {
    OBJECT	*obj;
    X_OFF_T	old_range_begin, old_range_end;
    X_OFF_T	delta;
} LN_RANGE;
struct lnrange_block {
    struct lnrange_block *next;
    int		in_use;
    LN_RANGE	block[LNRANGE_BLOCK_BUMP];
};
struct bincl_block {
    struct bincl_block *next;
    int		in_use;
    struct {
	OBJECT	*obj;
	SYMENT	*syment;
    } block[BINCL_BLOCK_BUMP];
};

/* Static variables */
static struct {
    FIXUP_ENTRIES	*fst;
    FIXUP_ENTRIES	*cur;
    int			bump;
} fixup_blocks[NUM_FIXUP_TYPES] = {
#define INFO_INDEX 0
    { 0, 0, INFO_BLOCK_BUMP },
};

static struct fixup_block	*cur_fixup_b,	*fst_fixup_b;
static struct lno_block		*cur_lno_b,	*fst_lno_b;
static struct lnrange_block	*cur_lnrange_b,	*fst_lnrange_b;
static struct bincl_block	*cur_bincl_b,	*fst_bincl_b;

static char	FILE_SYMTAB_NAME[SYMNMLEN] = {'.', 'f', 'i', 'l', 'e', 0,0,0};
static int	ifile_num_symbols;
static uint16	typchk_section;
static uint32	new_symtab_index, new_symtab_end_index, old_symtab_index;
static uint32	saved_old_symtab_index;
static uint32	previous_symtab_index;
static long	saved_num_syms;
static long	num_used_strings;
#define STRTAB_LEN_FIELD_LEN 4
static uint32	new_string_table_size = STRTAB_LEN_FIELD_LEN;
static long	symtab_index_offset;
static X_VADDR_T add_delta;		/* Old to new SD address delta */
static caddr_t	debug_section_base, debug_section_end;
static caddr_t	new_symtab_base, old_symtab_base;
static SYMENT	*previous_srcfile_symbol;
static STR	**saved_strings;
static int	cur_obj_read_file;
static long	save_nsyms;
static X_OFF_T	info_sect_offset;
static int	exceptions_seen;	/* If 1, some exception section
					   entries are being saved. */
static int	line_numbers_seen;	/* If 1, some line number
					   entries are being saved. */
static CSECT	*cur_csect;		/* For handling C_BLOCK and C_FCN. */

/* Global variables */
long	num_used_typchks;		/* Number of parm typecheck strings */
long	*symtab_index;
X_OFF_T	typchk_sect_offset;
struct typchk_values_t *typchk_values;

/* Forward declarations */
static void	save_fixup(unsigned long *, SYMBOL *, uint32);
static void	make_csect_auxtab(AUXENT *, SYMBOL *, int, int);
static void	build_er(SYMBOL *, int);
static void	csect_esds(CSECT *, OBJECT *, int);
static void	toc_csect(CSECT *);
static void	fixup_bincl_eincl(void);
static void	generate_ers(void);
static void	fixup_statics(void);
static void	save_bincl_eincl(OBJECT *, SYMENT *);
static void	save_info(OBJECT *, X_OFF_T, INFO_LENGTH_FIELD_T, SYMENT *);
static X_OFF_T	write_typchk_section(X_OFF_T);
static int	check_toc(OBJECT *, SYMBOL *, int, SRCFILE *);
static RETCODE	file_esds(SRCFILE *, OBJECT *, int);
static RETCODE	copy_debug_esds(OBJECT *, int);
static RETCODE	add_symbol_name(STR *, SYMENT *, int);
static RETCODE	fcn_lnnos(OBJECT *, AUXENT *, int, int);
static X_OFF_T	*convert_ln_address(OBJECT *, X_OFF_T);

#if DEBUG == 0 || READ_FILE == 0
#define DEBUG_loc_check(a,b,l)
#else
/************************************************************************
 * Name: DEBUG_loc_check
 *									*
 * Purpose: Make sure the input file is positioned at the proper symbol.
 *									*
 * Function:
 ************************************************************************/
static void
DEBUG_loc_check(OBJECT *obj,
		int sym_num,
		int line_num)
{
    int ff, ff1;

    ff = ftell(obj->o_ifile->i_file);
    if (obj->oi_symtab_offset + sym_num * SYMESZ != ff) {
	bind_err(SAY_NO_NLS, 0,
		 "Mismatch in position and expected position %d %d at line %d",
		 ff, obj->oi_symtab_offset + sym_num * SYMESZ, line_num);
	internal_error();
    }
}
#endif
/************************************************************************
 * Name: init_typchk_info
 *									*
 * Purpose: Perform any initialization required to save typecheck values
 *									*
 * Function:
 ************************************************************************/
void
init_typchk_info(void)
{
    static char *id = "init_typchk_info";

    if (typchk_values == NULL) {
	/* Allocate save arrays for typchks */
	typchk_values
	    = emalloc(Bind_state.num_typechks * sizeof(*typchk_values), id);
	num_used_typchks = 0;
    }
}
/************************************************************************
 * Name: init_symtab_info
 *									*
 * Purpose: Perform any initialization required to save symbol table entries
 *									*
 * Function:
 ************************************************************************/
static void
init_symtab_info(long nsyms)
{
    static char *id = "init_symtab_info";

    new_symtab_base = emalloc(nsyms * SYMESZ, id);
    new_symtab_index = 0;		/* Next available entry */
    save_nsyms = nsyms;
    new_symtab_end_index = nsyms-1;	/* Last available entry */

    symtab_index = emalloc(nsyms * sizeof(*symtab_index), id);
    symtab_index_offset = 0;

    /* Allocate save arrays for strings and typchks */
    init_typchk_info();

    saved_strings = emalloc((Bind_state.num_long_strings
			     + Bind_state.num_potential_C_FILE_strings)
			    * sizeof(*saved_strings), id);
    num_used_strings = 0;
}
/************************************************************************
 * Name: generate_symbol_table
 *									*
 * Purpose: Generate the symbol table--this involves storing typchk
 *	strings, the string table, and the symbol table itself.
 *									*
 * Function:
 ************************************************************************/
X_OFF_T
generate_symbol_table(long num_syms,
		      X_OFF_T output_offset)
{
    int		i, j;
    int		tok_ok;
#ifdef READ_FILE
    int		read_file;
#else
#define read_file 0
#endif
    long	s;
    OBJECT	*obj;
    SRCFILE	*sf;
    CSECT	*cs;
    SYMBOL	*er, *sym;
    TYPECHK	*t;

    saved_num_syms = num_syms;		/* Save value in static variable */
    /* First pass finds ERs with new instances of type-check strings, so we
       can compute the length of the .typchk section and write stabstrings
       directly to the file (in the .debug section). */
    for (obj = first_object(); obj; obj = obj->o_next) {
	switch(obj->o_type) {
	  case O_T_OBJECT:
	    for (er = obj->oi_ext_refs; er; er = er->er_next_in_object)
		if ((er->er_flags & S_SAVE)
		    && (er->er_name->flags & STR_NO_DEF)
		    && (t = er->s_typechk)
		    && (t->t_value == -1
			|| typchk_values[t->t_value].sect_val == 0)) {
		    if (t->t_value == -1) {
			t->t_value = num_used_typchks;
			typchk_values[num_used_typchks].ldr_val = 0;
			typchk_values[num_used_typchks++].t = t;
		    }

		    /* First time for this typchk */
		    typchk_values[t->t_value].sect_val
			= typchk_sect_offset + 2;
		    typchk_sect_offset += 2 + t->t_len;
		}
	    break;
	  case O_T_SCRIPT:
	  case O_T_SHARED_OBJECT:
	    for (sf = obj->o_srcfiles; sf; sf = sf->sf_next) {
		/* One CSECT per SRCFILE for imports */
		for (sym = &sf->sf_csect->c_symbol;
		     sym;
		     sym = sym->s_next_in_csect)
		    if (sym->s_flags & S_SAVE)
			if (t = sym->s_typechk) {
			    if (t->t_value == -1
				|| typchk_values[t->t_value].sect_val == 0) {
				if (t->t_value == -1) {
				    t->t_value = num_used_typchks;
				    typchk_values[num_used_typchks].ldr_val= 0;
				    typchk_values[num_used_typchks++].t = t;
				}

				/* First time for this typchk */
				typchk_values[t->t_value].sect_val
				    = typchk_sect_offset + 2;
				typchk_sect_offset += 2 + t->t_len;
			    }
			}
	    }
	    break;
	}
    }

    output_offset = write_typchk_section(output_offset);
    init_symtab_info(num_syms);

    /* Loop variable initialization */
    previous_srcfile_symbol = NULL;
    previous_symtab_index = -1;
    init_stabs(output_offset, Shmaddr); /* Init. stab routines. */

    /* This pass writes external references, for unresolved symbols and symbols
       resolved from shared libraries. */

    generate_ers();

    /* Write all TC entries and the TOC anchor */
    for (i = SC_TCOVRFL; i <= SC_TC_EXT; i++) {
	if ((j = sect_info[i].heads) != -1)
	    break;
    }
    if (j != -1) {
	for ( ; j < sect_info[SC_TC_EXT].tails; ++j)
	    if (Queue[j]->c_symbol.s_smclass != XMC_TD)
		toc_csect(Queue[j]);
    }

    if (file_esds(NULL, NULL, TB_FRONT))
	return output_offset;

    /* Pass through each object to build the symbol tables */
    for (obj = first_object(); obj; obj = obj->o_next) {
	if (obj->o_type == O_T_OBJECT && (obj->oi_flags & OBJECT_USED)) {
	    ifile_reopen_remap(obj->o_ifile);
	    ifile_num_symbols = obj->oi_num_symbols;
	    stab_obj = obj;		/* Save object in static variable for
					   error messages.  */
#ifdef READ_FILE
	    read_file = (obj->o_ifile->i_access == I_ACCESS_READ);
	    cur_obj_read_file = read_file;
#endif
	    if (!read_file) {
		old_symtab_base
		    = obj->o_ifile->i_map_addr+obj->oi_symtab_offset;
		set_strtab_base(obj);
	    }
	    old_symtab_index = 0;

	    if (obj->oi_flags & OBJECT_HAS_DEBUG) {
		s = obj->oi_section_info[obj->oi_debug_sect_i].sect_size;
#ifdef READ_FILE
		if (read_file) {
		    debug_section_base = alloca(s);
		    if (fseek_read(obj->o_ifile,
				   obj->oi_section_info[obj->oi_debug_sect_i]
				   .u.sect_offset,
				   debug_section_base, s) != 0)
			return output_offset;
		    debug_section_end = &debug_section_base[s];
		}
		else
#endif
		    {
			debug_section_base = obj->o_ifile->i_map_addr
			    + obj->oi_section_info[obj->oi_debug_sect_i]
				.u.sect_offset;
			debug_section_end = &debug_section_base[s];
		    }
	    }
	    for (sf = obj->oi_srcfiles; sf; sf = sf->sf_next) {
		for (cs = sf->sf_csect; cs; cs = cs->c_next)
		    if (cs->c_save) {
			/* If the C_FILE only contains TOC entries or
			   external references, we don't want to call
			   file_esds, so we must check the esds here.
			   If there is some sort of unexpected problem
			   with the TOC entries, check_toc will write
			   the C_FILE entry. */
			if (cs->c_symbol.s_smclass != XMC_TC
			    && cs->c_symbol.s_smclass != XMC_TC0)
			    goto needed;

			/* Compute address delta and fixup symbol address*/
			add_delta = cs->c_new_addr - cs->c_addr;
			for (tok_ok = 0, sym = &cs->c_symbol;
			     sym;
			     sym = sym->s_next_in_csect) {
			    if (tok_ok == 0)
				tok_ok = check_toc(obj, sym, read_file, sf);
			    else
				(void) check_toc(obj, sym, read_file, NULL);
			}
			if (tok_ok != 0) {
			    cs = cs->c_next;
			    goto needed_next;
			}
		    }
		continue; /* No CSECTS in this SRCFILE */
	      needed:
		/* We'll need new local mappings for
		   stabs (if compacting stabstrings). */
#ifdef DEBUG
		stab_sf = sf;
#endif
		reset_stab_mappings();
		if (file_esds(sf, obj, read_file))
		    return output_offset;
	      needed_next:
		for ( ; cs ; cs = cs->c_next)
		    if (cs->c_save)
			csect_esds(cs, obj, read_file); /* Does LDs too */
		if (Switches.stabcmpct >= STABCMPCT_RENUM)
		    process_deferred_stabs();
	    }
	}
	/* Make sure symbol table limit was not exceeded */
	if (new_symtab_index > new_symtab_end_index + 1) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SYMBOL_TABLE_OVERFLOW,
    "%s: 0711-866 INTERNAL ERROR: Output symbol table size miscalculated."),
		     Main_command_name);
	    return output_offset;
	}
    }

    if (new_symtab_index == 1 && new_symtab_end_index == save_nsyms - 1)
	new_symtab_index = 0;		/* Only symbol was TB_FRONT, so we
					   delete it. */
    else {
#ifdef STABCMPCT_NODUPSYMS
	if (Switches.stabcmpct == STABCMPCT_NODUPSYMS)
	    if (file_esds(NULL, NULL, TB_BACK))
		return output_offset;
#endif

	/*  Make the last C_FILE entry in the symbol table point to the
	 *  gened C_FILE entry or -1 if there are no gened entries. */
	if (previous_srcfile_symbol)
	    previous_srcfile_symbol->n_value = -1;

	output_offset = finish_stab_section(output_offset, Scnhdr, &next_scn);
    }
    return output_offset;
} /* generate_symbol_table */
/************************************************************************
 * Name: generate_ers
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static void
generate_ers(void)
{
    int		i;
    SYMBOL 	*er;
    OBJECT	*obj;
    SYMBOL	*er1, *sym;
    SRCFILE	*sf;

    /* First, write ERs for imported symbols. */
    for (obj = first_object(); obj; obj = obj->o_next) {
	switch(obj->o_type) {
	  case O_T_SCRIPT:
	  case O_T_SHARED_OBJECT:
	    for (sf = obj->o_srcfiles; sf; sf = sf->sf_next) {
		/* One CSECT per SRCFILE for imports */
		for (sym = &sf->sf_csect->c_symbol;
		     sym;
		     sym = sym->s_next_in_csect)
		    if (sym->s_flags & S_SAVE) { /* We'll use this symbol */
			for ( ; sym ; sym = sym->s_next_in_csect)
			    if ((sym->s_flags & S_SAVE)
				&& !(sym->er_name->flags & STR_ER_OUTPUT)) {
				sym->er_name->flags |= STR_ER_OUTPUT;
				sym->s_inpndx = new_symtab_index;
				build_er(sym, 1 /* symbol is imported */);
			    }
			break;
		    }
	    }
	    break;
	}
	/* Make sure symbol table limit was not exceeded */
	if (new_symtab_index > new_symtab_end_index + 1) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SYMBOL_TABLE_OVERFLOW,
    "%s: 0711-866 INTERNAL ERROR: Output symbol table size miscalculated."),
		     Main_command_name);
	}
    }

    /* Now write ERs for real unresolved symbols. */
    for (i = 0; i < last_unresolved; ++i) {
	er = unresolved_queue[i];
	old_symtab_index = er->er_inpndx;
	er->er_inpndx = symtab_index_offset;
	er->er_flags |= S_INPNDX_MOD;
	symtab_index[symtab_index_offset++] = old_symtab_index;
	symtab_index[symtab_index_offset++] = new_symtab_index;
	build_er(er, 0 /* symbol is not imported */);
	/* Make sure symbol table limit was not exceeded */
	if (new_symtab_index > new_symtab_end_index + 1) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SYMBOL_TABLE_OVERFLOW,
    "%s: 0711-866 INTERNAL ERROR: Output symbol table size miscalculated."),
		     Main_command_name);
	}
    }
} /* generate_ers */
/************************************************************************
 * Name: write_typchk_section
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static X_OFF_T
write_typchk_section(X_OFF_T output_offset)
{
    caddr_t	mem_ptr;
    uint	i;
    SCNHDR	*sect_hdr;

    if (num_used_typchks == 0)
	return output_offset;

    sect_hdr = &Scnhdr[next_scn++];
    typchk_section = next_scn;	/* 1-based section number */
    output_offset = ROUND(output_offset, 2);

    /* Point to where type check section should be written */
    mem_ptr = Shmaddr + output_offset;

    for (i = 0; i < num_used_typchks; i++) {
	if (typchk_values[i].sect_val == 0)
	    continue;			/* Must have been in loader sect only*/
	/* Copy length first, then typchk string */
	memcpy(mem_ptr, &typchk_values[i].t->t_len, 2);
	if (typchk_values[i].t->t_len == TYPCHKSZ) {
	    memcpy(mem_ptr + 2, &typchk_values[i].t->t_typechk, TYPCHKSZ);
	}
	else {
	    memcpy(mem_ptr + 2,
		   typchk_values[i].t->t_len <= sizeof(TYPCHK)
		   ? &typchk_values[i].t->t_c_typechk[0]
		   : typchk_values[i].t->t_cp_typechk,
		   typchk_values[i].t->t_len);
	}
	mem_ptr += typchk_values[i].t->t_len + 2;
    }

#ifdef DEBUG
    if ((mem_ptr - Shmaddr) - output_offset != typchk_sect_offset)
	internal_error();
#endif

    /* Update section header for typchk section */
    (void) strncpy(sect_hdr->s_name, _TYPCHK, sizeof(sect_hdr->s_name));

    sect_hdr->s_paddr = sect_hdr->s_vaddr = 0;
    sect_hdr->s_size = typchk_sect_offset;
    sect_hdr->s_scnptr = output_offset;
    sect_hdr->s_relptr = sect_hdr->s_lnnoptr = 0;
    sect_hdr->s_nreloc = sect_hdr->s_nlnno = 0;
    sect_hdr->s_flags = STYP_TYPCHK;

    return output_offset + typchk_sect_offset;
} /* write_typchk_section */
/************************************************************************
 * Name: write_symbol_table
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
X_OFF_T
write_symbol_table(X_OFF_T output_offset)
{
    int		i;
    caddr_t	mem_ptr;

    output_offset = ROUND(output_offset, 2);

    Filehdr->f_symptr = output_offset;

    if (new_symtab_base) {		/* New symbols exist */
	/* Copy symbol table to file */
	memcpy(Shmaddr + output_offset,
	       new_symtab_base,
	       new_symtab_index * SYMESZ);
	output_offset += new_symtab_index * SYMESZ;

#ifdef STABCMPCT_NODUPSYMS
	/* This code isn't complete yet.  It is used if all unnamed C_DECL
	   symbols are to be moved to the very end of the symbol table. */
	while (unnamed_type_symbols < end_new_symtab) {
	    /* Copy symbol by symbol */
	    memcpy(Shmaddr + output_offset, (char *)end_new_symtab, SYMESZ);
	    output_offset += SYMESZ;
	    end_new_symtab -= SYMESZ;
	    new_symtab_index++;
	}
#endif
	efree(new_symtab_base);
    }

    Filehdr->f_nsyms = new_symtab_index
	+ (saved_num_syms - new_symtab_end_index - 1);

    /* Write out the String table. */
    mem_ptr = Shmaddr + output_offset;

    if (num_used_strings == 0) {
	if (Filehdr->f_nsyms != 0) {
	    /* Write a word of zeros for string table length */
	    memset(mem_ptr, 0, STRTAB_LEN_FIELD_LEN);
	    output_offset += STRTAB_LEN_FIELD_LEN;
	}
    }
    else {
	/* Copy string table size field to file */
	memcpy(mem_ptr, &new_string_table_size, STRTAB_LEN_FIELD_LEN);
	mem_ptr += STRTAB_LEN_FIELD_LEN;
	for (i = 0; i < num_used_strings; i++) {
	    /* Copy string to file (along with terminating null) */
	    strcpy(mem_ptr, saved_strings[i]->name);
	    /* Increment mem_ptr--be sure to add 1 for NULL. */
	    mem_ptr += saved_strings[i]->len + 1;
	}
	output_offset += new_string_table_size;
    }
    return output_offset;
} /* write_symbol_table */
/************************************************************************
 * Name: file_esds
 *									*
 * Purpose:  Process File esd's.
 *									*
 * Function:
 ************************************************************************/
static RETCODE
file_esds(SRCFILE *sf,
	  OBJECT *obj,
	  int read_file)
{
    STR		*name_ptr;
    long	num_aux;
    SYMENT	*new_symtab, *old_symtab;
    static SYMENT *TB_FRONT_syment;
    AUXENT	*new_auxtab, *old_auxtab;
    int		read_obj;

    new_symtab = (SYMENT *) (new_symtab_base + new_symtab_index * SYMESZ);

    /* Patch link in previous source-file symbol table entry */
    if (previous_srcfile_symbol)
	previous_srcfile_symbol->n_value = new_symtab_index;
    previous_srcfile_symbol = new_symtab;

    new_symtab_index++;

    /* Check for generated SRCFILE entry */
    if (sf == NULL || sf->sf_inpndx == SF_GENERATED_INPNDX) {
	new_symtab->n_value = -1;
	new_symtab->n_scnum = N_DEBUG;
	new_symtab->n_sclass = C_FILE;
	new_symtab->n_numaux = 0;
	if (sf == NULL) {
	    /* Generated C_FILE for TB_FRONT or TB_BACK */
	    new_symtab->n_lang = read_file;
	    new_symtab->n_cputype = TCPU_COM;
	    (void) strncpy(new_symtab->n_name, " ", sizeof(new_symtab->n_name));
	    if (read_file == TB_FRONT) {
		new_symtab->n_name[3] = Switches.stabcmpct;
		TB_FRONT_syment = new_symtab;
	    }
	    else			/* Patch prologue C_FILE entry */
		TB_FRONT_syment->n_offset = new_symtab_index - 1;
	}
	else {
	    /* Generated C_FILE for object with missing C_FILE */
	    new_symtab->n_lang = TB_OBJECT;
	    new_symtab->n_cputype = TCPU_PWR;
	    add_symbol_name(sf->sf_name, new_symtab, SYMNMLEN);
	}
	return RC_OK;
    }

    /* Copy the old symbol to the new symbol */
#ifdef READ_FILE
    if (read_file) {
	read_obj = 1;
	if (fseek_read(obj->o_ifile,
		       obj->oi_symtab_offset + sf->sf_inpndx * SYMESZ,
		       new_symtab,
		       SYMESZ) != 0)
	    return RC_SEVERE;
    }
    else
#endif
    {
	read_obj = 0;
	old_symtab = aligned_sym_ptr(old_symtab_base, sf->sf_inpndx);
	memcpy(new_symtab, old_symtab, SYMESZ);
    }

    /* Build C_FILE entry */
    new_symtab->n_value = -1;
    new_symtab->n_scnum = N_DEBUG;	/* Can we just leave this as copied? */
    num_aux = new_symtab->n_numaux;

    old_auxtab = (AUXENT *)(old_symtab_base + (sf->sf_inpndx + 1) * SYMESZ),
    old_symtab_index = sf->sf_inpndx + 1 + num_aux;

    if (num_aux <= 1) {
	/* We have at most one auxiliary entry for the file name--it can be
	   saved in the C_FILE entry itself. */
	new_symtab->n_numaux = 0;
	add_symbol_name(sf->sf_name, new_symtab, SYMNMLEN);
#ifdef READ_FILE
	if (num_aux == 1 && read_file)
	    /* Skip over auxiliary entry */
	    if (safe_fseek(obj->o_ifile, SYMESZ, SEEK_CUR) != 0)
		return RC_SEVERE;
#endif
    }
    else if (num_aux != 0) {
	/* Copy ".file" to symbol name */
	memcpy(new_symtab->n_name, FILE_SYMTAB_NAME, SYMNMLEN);

	/* Copy all auxiliary entries */
	new_auxtab = (AUXENT *)(new_symtab_base + new_symtab_index * SYMESZ);
	new_symtab_index += num_aux;
#ifdef READ_FILE
	if (read_file) {
	    DEBUG_loc_check(obj, sf->sf_inpndx + 1, __LINE__);
	    if (safe_fread(new_auxtab, AUXESZ * num_aux, obj->o_ifile) != 0)
		return RC_SEVERE;
	}
	else
#endif
	    memcpy(new_auxtab, old_auxtab, AUXESZ * num_aux);

	/* First auxiliary entry is file name */
	add_symbol_name(sf->sf_name, (SYMENT *)new_auxtab, FILNMLEN);

	/* Fix up name for remaining auxiliary entries */
	while (--num_aux) {
	    new_auxtab = (AUXENT *)((char *)new_auxtab + AUXESZ);

	    if (new_auxtab->x_file._x.x_zeroes == 0) {
		name_ptr = get_aux_name(obj, new_auxtab, read_obj,
					sf->sf_inpndx + 1);
		add_symbol_name(name_ptr, (SYMENT *)new_auxtab, FILNMLEN);
	    }
	}
    }
    /* Copy debug related esd entries */
    if (copy_debug_esds(obj, read_file))
	return RC_SEVERE;

    return RC_OK;
} /* file_esds */
/************************************************************************
 * Name: make_csect_auxtab
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static void
make_csect_auxtab(AUXENT *new_auxtab,
		  SYMBOL *sym,
		  int len,
		  int align)
{
    TYPECHK *t;

    new_auxtab->x_csect.x_scnlen = len;

    if (!(sym->s_flags & S_HIDEXT) && (t = sym->s_typechk)) {
#ifdef DEBUG
	/* All used typchk values should have already been written. */
	if (t->t_value == -1 || typchk_values[t->t_value].sect_val == 0)
	    internal_error();
#endif

	new_auxtab->x_csect.x_parmhash = typchk_values[t->t_value].sect_val;
	new_auxtab->x_csect.x_snhash = typchk_section;
    }
    else {
	new_auxtab->x_csect.x_parmhash = 0;
	new_auxtab->x_csect.x_snhash = 0;
    }

    new_auxtab->x_csect.x_smtyp = (unsigned short) align << 3 | sym->s_smtype;
    new_auxtab->x_csect.x_smclas = sym->s_smclass;

    new_auxtab->x_csect.x_stab = 0; /*??*/
    new_auxtab->x_csect.x_snstab = 0; /*??*/
}
/************************************************************************
 * Name: csect_or_label_esd
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static RETCODE
csect_or_label_esd(OBJECT *obj,
		   SYMBOL *sym,
		   SYMENT *new_symtab,
		   int read_file,
		   int len_or_index,
		   int align,
		   STR *sym_name)
{
    int		num_aux;
    SYMENT	temp_symtab, *old_symtab;
    AUXENT	*new_auxtab;

    if (sym->s_inpndx == INPNDX_GENERATED) {
	/* Symbol was generated.  We just fill in the information */

	/* Fill in fields in new symbol */
	add_symbol_name(sym_name, new_symtab, SYMNMLEN); /* Insert name */
	new_symtab->n_value = sym->s_addr + add_delta;
	new_symtab->n_scnum = sect_mappings[sym->s_csect->c_major_sect];
	new_symtab->n_type = DT_NON;
	new_symtab->n_sclass = (sym->s_flags & S_HIDEXT) ? C_HIDEXT : C_EXT;
	new_symtab->n_numaux = 1;

	/* Even though symbol is generated, we save its index in case there
	   is a later check for S_INPNDX_MOD on this symbol. */
	sym->s_inpndx = symtab_index_offset;
	sym->s_flags |= S_INPNDX_MOD;
	symtab_index[symtab_index_offset++] = INPNDX_GENERATED;
	symtab_index[symtab_index_offset++] = new_symtab_index;

	new_auxtab = (AUXENT *)((char *)new_symtab + SYMESZ);
	++new_symtab_index;		/* Advance to first auxiliary entry */
	/* Build the CSECT auxiliary entry */
	make_csect_auxtab(new_auxtab, sym, len_or_index, align);
	++new_symtab_index;		/* Advance past auxiliary entries */
	return RC_OK;
    }

    /* Read (or copy) the old symbol */
#ifdef READ_FILE
    if (read_file) {
	if (fseek_read(obj->o_ifile,
		       obj->oi_symtab_offset + sym->s_inpndx * SYMESZ,
		       &temp_symtab, SYMESZ) != 0)
	    return RC_SEVERE;
	old_symtab = &temp_symtab;
    }
    else
#endif
	old_symtab = aligned_sym_ptr(old_symtab_base, sym->s_inpndx);

    /* Get num_aux from old symbol */
    num_aux = old_symtab->n_numaux;

    /* Fill in fields in new symbol */
    add_symbol_name(sym_name, new_symtab, SYMNMLEN); /* Insert name */
    new_symtab->n_value = old_symtab->n_value + add_delta;
    new_symtab->n_scnum = sect_mappings[sym->s_csect->c_major_sect];
    new_symtab->n_type = old_symtab->n_type;
    new_symtab->n_sclass = (sym->s_flags & S_HIDEXT) ? C_HIDEXT : C_EXT;
    new_symtab->n_numaux = num_aux;

    old_symtab_index = sym->s_inpndx;
    sym->s_inpndx = symtab_index_offset;
    sym->s_flags |= S_INPNDX_MOD;
    symtab_index[symtab_index_offset++] = old_symtab_index;
    symtab_index[symtab_index_offset++] = new_symtab_index;

    new_auxtab = (AUXENT *)((char *)new_symtab + SYMESZ);

    old_symtab_index++;			/* Advance to first auxiliary entry */
    new_symtab_index++;			/* Advance to first auxiliary entry */

    if (num_aux > 1) {
	if (ISFCN(new_symtab->n_type)) {
	    if (fcn_lnnos(obj, new_auxtab, old_symtab->n_scnum, read_file))
		return RC_SEVERE;
	    /* Bump pointer and decrement count */
	    new_auxtab = (AUXENT *)((char *)new_auxtab + AUXESZ);
	    num_aux--;
	    old_symtab_index++;		/* Advance past function aux. entry */
	    new_symtab_index++;		/* Advance past function aux. entry */
	}
	if (num_aux > 1) {
	    /* Copy auxiliary entries (except for CSECT entry) */
#ifdef READ_FILE
	    if (read_file) {
		DEBUG_loc_check(obj, old_symtab_index, __LINE__);
		if (safe_fread(new_auxtab, AUXESZ * (num_aux - 1),
			       obj->o_ifile) != 0)
		    return RC_SEVERE;
	    }
	    else
#endif
		memcpy(new_auxtab,
		       (AUXENT *)(old_symtab_base + old_symtab_index * SYMESZ),
		       (num_aux-1) * SYMESZ);
	}
	new_auxtab = (AUXENT *)((char *)new_auxtab + (num_aux - 1) * AUXESZ);
    }

    /* Build the CSECT auxiliary entry */
    make_csect_auxtab(new_auxtab, sym, len_or_index, align);

#ifdef READ_FILE
    /* Advance past CSECT auxiliary entry */
    if (read_file)
	if (safe_fseek(obj->o_ifile, SYMESZ, SEEK_CUR) != 0)
	    return RC_SEVERE;
#endif

    old_symtab_index += num_aux;
    new_symtab_index += num_aux;	/* Advance past auxiliary entries */

    if (copy_debug_esds(obj, read_file))
	return RC_SEVERE;

    return RC_OK;
} /* csect_or_label_esd */
/************************************************************************
 * Name: toc_csect
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static void
toc_csect(CSECT *cs)
{
    SYMBOL	*sym, *next_sym;
    SYMENT	*new_symtab;
    AUXENT	*new_auxtab;
    int		csect_symtab_index = new_symtab_index;
    STR		*csect_name;

    new_symtab = (SYMENT *) (new_symtab_base + new_symtab_index * SYMESZ);
    sym = &cs->c_symbol;

    csect_name = sym->s_name;
    next_sym = sym->s_next_in_csect; /* Continue output with next symbol */

    /* Compute address delta and fixup symbol address*/
    add_delta = cs->c_new_addr - cs->c_addr;

    /* Fill in fields in new symbol */
    add_symbol_name(csect_name, new_symtab, SYMNMLEN); /* Insert name */
    new_symtab->n_value = sym->s_addr + add_delta;
    new_symtab->n_scnum = temp_aout_hdr.o_sndata;
    new_symtab->n_type = 0;
    new_symtab->n_sclass = (sym->s_flags & S_HIDEXT) ? C_HIDEXT : C_EXT;
    new_symtab->n_numaux = 1;

    old_symtab_index = sym->s_inpndx;
    sym->s_inpndx = symtab_index_offset;
    sym->s_flags |= S_INPNDX_MOD;
    symtab_index[symtab_index_offset++] = old_symtab_index;
    symtab_index[symtab_index_offset++] = new_symtab_index;

    new_symtab_index++;		/* Advance to first auxiliary entry */
    new_auxtab = (AUXENT *)(new_symtab_base + new_symtab_index * SYMESZ);

    /* Build the CSECT auxiliary entry */
    make_csect_auxtab(new_auxtab, sym, cs->c_len, cs->c_align);
    new_symtab_index++;	/* Advance past auxiliary entries */

    for (sym = next_sym ; sym; sym = sym->s_next_in_csect) {
	new_symtab = (SYMENT *)(new_symtab_base + new_symtab_index * SYMESZ);
	/* Fill in fields in new symbol */
	add_symbol_name(sym->s_name, new_symtab, SYMNMLEN);/* Insert name */
	new_symtab->n_value = sym->s_addr + add_delta;
	new_symtab->n_scnum = sect_mappings[sym->s_csect->c_major_sect];
	new_symtab->n_type = 0;
	new_symtab->n_sclass = (sym->s_flags & S_HIDEXT) ? C_HIDEXT : C_EXT;
	new_symtab->n_numaux = 1;

	old_symtab_index = sym->s_inpndx;
	sym->s_inpndx = symtab_index_offset;
	sym->s_flags |= S_INPNDX_MOD;
	symtab_index[symtab_index_offset++] = old_symtab_index;
	symtab_index[symtab_index_offset++] = new_symtab_index;

	new_symtab_index++;		/* Advance to first auxiliary entry */
	new_auxtab = (AUXENT *)(new_symtab_base + new_symtab_index * SYMESZ);

	/* Build the CSECT auxiliary entry */
	make_csect_auxtab(new_auxtab, sym, csect_symtab_index, 0);
	new_symtab_index++; /* Advance past auxiliary entry */
    }
} /* toc_csect */
/************************************************************************
 * Name: check_toc
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static int
check_toc(OBJECT *obj,
	  SYMBOL *sym,
	  int read_file,
	  SRCFILE *sf)
{
    SYMENT	temp_symtab;
    int		c, n_aux;

    if (symtab_index[sym->s_inpndx] == INPNDX_GENERATED)
	return 0;			/* TOC was generated */

    if (symtab_index[sym->s_inpndx] == INPNDX_ARCHIVE) {
	/* The TOC was read from an archive.  This should never happen, since
	   archive symbols are external and TOC symbols must be internal.
	   Nevertheless, to avoid memory errors, we silently return 0. */
	return 0;
    }

    if (sym->s_flags & S_INPNDX_MOD)
	old_symtab_index = symtab_index[sym->s_inpndx];
    else {
	bind_err(SAY_NORMAL, RC_PGMERR,
		 NLSMSG(SAVE_NO_TOC,
"%1$s: 0711-868 INTERNAL ERROR: The TOC symbol should have already been written."),
		 Main_command_name);
	old_symtab_index = sym->s_inpndx;
    }

    /* Read (or copy) the old symbol */
#ifdef READ_FILE
    if (read_file) {
	if (fseek_read(obj->o_ifile,
		       obj->oi_symtab_offset + old_symtab_index * SYMESZ,
		       &temp_symtab, SYMESZ) != 0)
	    return -1;
	n_aux = temp_symtab.n_numaux;
    }
    else
#endif
	n_aux = ((SYMENT*)(old_symtab_base
			   + old_symtab_index*SYMESZ))->n_numaux;

    if (n_aux != 1) {
	bind_err(SAY_NORMAL, RC_ERROR,
		 NLSMSG(BAD_TOC2,
 "%1$s: 0711-865 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tExtra auxiliary entries for the TOC anchor are not being copied to\n"
 "\tthe output file."),
		 Main_command_name, sym->s_name->name, old_symtab_index,
		 get_object_file_name(obj));
    }

    old_symtab_index += 1 + n_aux; /* Advance past aux entries */

#ifdef READ_FILE
    /* Advance past CSECT auxiliary entry */
    if (read_file)
	if (safe_fseek(obj->o_ifile, n_aux * SYMESZ, SEEK_CUR) != 0)
	    return -1;
#endif

    /* Any subsequent symbols */
    if (old_symtab_index >= ifile_num_symbols)
	return 0;

    /* Read the next symbol--it must be C_FILE, C_EXT, or C_HIDEXT */
#ifdef READ_FILE
    if (read_file) {
	if (fseek_read(obj->o_ifile,
		       obj->oi_symtab_offset + old_symtab_index * SYMESZ,
		       &temp_symtab, SYMESZ) != 0)
	    return -1;
	c = temp_symtab.n_sclass;
    }
    else
#endif
	c = ((SYMENT*)(old_symtab_base+ old_symtab_index*SYMESZ))->n_sclass;

    if (c == C_EXT || c == C_HIDEXT || c == C_FILE)
	return 0;

    if (sf) {
	stab_obj = obj;
#ifdef DEBUG
	stab_sf = sf;
#endif
	reset_stab_mappings();
	if (file_esds(sf, obj, read_file))
	    return -1;
    }
    if (copy_debug_esds(obj, read_file))
	return -1;

    return 1;
} /* check_toc */
/************************************************************************
 * Name: csect_esds
 *									*
 * Purpose:  Process csect esd's.
 *									*
 * Function:
 ************************************************************************/
static void
csect_esds(CSECT *cs,
	   OBJECT *obj,
	   int read_file)
{
    SYMBOL	*sym, *next_sym;
    SYMENT	*new_symtab;
    int		csect_symtab_index = new_symtab_index;
    STR		*csect_name;

    sym = &cs->c_symbol;

    if (cs->c_symbol.s_smclass == XMC_TC
	|| cs->c_symbol.s_smclass == XMC_TC0) {
	/* Compute address delta and fixup symbol address*/
	add_delta = cs->c_new_addr - cs->c_addr;
        for ( ; sym; sym = sym->s_next_in_csect)
	    check_toc(obj, sym, read_file, NULL);
	return;
    }

    new_symtab = (SYMENT *) (new_symtab_base + new_symtab_index * SYMESZ);

#if 0
    /* Insert name in new symbol table entry and set n_sclass */
    /*
      We keep the name on the csect (in the first sym) and
      output the symbols separately if :
      1. The Csect is C_EXT.
      2. The symbol is referenced (S_SAVE bit is set in s_flags).
      3. There are no LDs

      We use a null name for the csect, and output a separate LD for the
      first symbol if:

      1. there is more than one label

      We use the name on the first LD for the name of the symbol,
      effectively tossing the original name on the CSECT if:
      1) The CSECT is not referenced.
      2) The CSECT is C_HIDEXT.
      3) There are no extra aux. entries for the CSECT and no
      debugging entries between the csect and the first label.
      */

    if ((cs->c_symbol.s_flags & S_HIDEXT) /* Symbol (CSECT) is C_EXT */
	|| sym->s_next_in_csect == NULL /* No LDs */
	|| (sym->s_flags & S_SAVE) != 0 /* CSECT was referenced */
	|| (sym->s_next_in_csect	/* CSECT has >1 aux entries or
					   debugging entries exist between
					   CSECT and first LD.  */
	    && sym->s_next_in_csect->s_inpndx != csect_symtab_index+2)) {
	/* Keep the original name on the CSECT */
	add_symbol_name(sym->s_name, new_symtab, SYMNMLEN);
	new_symtab->n_sclass
	    = (cs->c_symbol.s_flags & S_HIDEXT) ? C_HIDEXT : C_EXT;
	next_sym = sym->s_next_in_csect; /* Continue output with next
					    symbol */
    }
    else if (sym->s_name != &NULL_STR
	     && sym->s_next_in_csect
	     && sym->s_next_in_csect->s_name == sym->s_name) {
	/* CSECT is named (and hidden) and has same name as first LD */
	memset(new_symtab->n_name, 0, SYMNMLEN); /* Use null name. */
	new_symtab->n_sclass = C_HIDEXT;
	next_sym = sym->s_next_in_csect; /* We'll write the name in its
					    own LD entry. */
    }
    else if (sym->s_next_in_csect	/* Exactly 1 LD */
	     && sym->s_next_in_csect->s_next_in_csect
	     && sym->s_name == &NULL_STR) {	/* No name on csect */
	/* Use name on LD */

	add_symbol_name(sym->s_next_in_csect->s_name, new_symtab, SYMNMLEN);
	new_symtab->n_sclass
	    = (sym->s_next_in_csect->s_flags & S_HIDEXT) ? C_HIDEXT : C_EXT;

	next_sym = NULL;		/* We'll write the name in its
					   own LD entry. */
    }
#endif

    csect_name = sym->s_name;

    next_sym = sym->s_next_in_csect; /* Continue output with next symbol */

    /* Compute address delta and fixup symbol address*/
    add_delta = cs->c_new_addr - cs->c_addr;

    cur_csect = cs;
    csect_or_label_esd(obj, sym, new_symtab, read_file,
		       cs->c_len, cs->c_align, csect_name);

    for (sym = next_sym ; sym; sym = sym->s_next_in_csect) {
	new_symtab = (SYMENT *)(new_symtab_base + new_symtab_index * SYMESZ);
	csect_or_label_esd(obj, sym, new_symtab, read_file,
			   csect_symtab_index, 0, sym->s_name);
    }
    cur_csect = NULL;
} /* csect_esds */
/************************************************************************
 * Name: build_er
 *									*
 * Purpose: Fill in the symbol table entry (and auxiliary entry)
 *	for an external reference.  External references are needed for
 *	imported symbols, which need to redefined for subsequent binds,
 *	and for truly undefined symbols.
 *
 *	Note that for a symbol imported at a fixed address, the symbols's
 *	address and storage-mapping class (XMC_XO) are written to the
 *	symbol table.  If the output file being generated is rebound
 *	without redefining the milicode symbols, the storage-mapping
 *	class of the reference must be changed from XMC_XO to XMC_UA, and
 *	the symbol's previous address is ignored.
 *									*
 ************************************************************************/
static void
build_er(SYMBOL *sym,
	 int imported)
{
    SYMENT *new_symtab;
    AUXENT *new_auxtab;
    TYPECHK *t;

    new_symtab = (SYMENT *)(new_symtab_base + new_symtab_index * SYMESZ);
    new_auxtab = (AUXENT *)((char *)new_symtab + AUXESZ);

    /* Build the ER entry */
    add_symbol_name(sym->s_name, new_symtab, SYMNMLEN);
    new_symtab->n_sclass = C_EXT;
    if (imported) {
	new_symtab->n_value = sym->s_addr; /* If sym->s_smclass is XMC_XO, this
					      value may be non-zero.  */
	new_auxtab->x_csect.x_smclas = sym->s_smclass;
    }
    else {
	new_symtab->n_value = 0;
	if (sym->s_smclass != XMC_XO)
	    new_auxtab->x_csect.x_smclas = sym->s_smclass;
	else
	    new_auxtab->x_csect.x_smclas = XMC_UA;
    }
    new_symtab->n_scnum = N_UNDEF;
    new_symtab->n_type = DT_NON;
    new_symtab->n_numaux = 1;

    new_auxtab->x_csect.x_scnlen = 0;

    if (!(sym->s_flags & S_HIDEXT) && (t = sym->s_typechk)) {
#ifdef DEBUG
	/* All used typchk values should have already been written. */
	if (t->t_value == -1 || typchk_values[t->t_value].sect_val == 0)
	    internal_error();
#endif
	new_auxtab->x_csect.x_parmhash = typchk_values[t->t_value].sect_val;
	new_auxtab->x_csect.x_snhash = typchk_section;
    }
    else {
	new_auxtab->x_csect.x_parmhash = 0;
	new_auxtab->x_csect.x_snhash = 0;
    }

    new_auxtab->x_csect.x_smtyp = XTY_ER;

    new_auxtab->x_csect.x_stab = 0; /*??*/
    new_auxtab->x_csect.x_snstab = 0; /*??*/

    new_symtab_index += 2;
} /* build_er */
/************************************************************************
 * Name: get_debug_name
 *									*
 * Purpose:  Get a debug name from the .debug section
 *									*
 * Returns:	NULL if offset value is bad or name is not null-terminated.
 *		Pointer to stabstring otherwise.
 ************************************************************************/
static caddr_t
get_debug_name(X_OFF_T stab_offset,
	       uint16 *stab_len_out)
{
    uint16 stab_len;
    caddr_t stab_string;

    if (stab_offset < 2 ||
	stab_offset > debug_section_end - debug_section_base)
	goto bad_offset;
    stab_string = &debug_section_base[stab_offset],
    /* Get length (without null) */
    memcpy(&stab_len, &stab_string[-2], 2);
    --stab_len;		/* Don't count terminating '\0' */
    if (&stab_string[stab_len+1] > debug_section_end
	|| stab_string[stab_len] != '\0') {
      bad_offset:
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(STAB_BAD_LEN2,
"%1$s: 0711-380 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
"\tLength of stabstring in .debug section is invalid.\n"
"\tThe stabstring is being deleted."),
		 Main_command_name,
		 saved_old_symtab_index,
		 get_object_file_name(stab_obj));
	return NULL;
    }
    else {
	*stab_len_out = stab_len;
	return stab_string;
    }
}
/************************************************************************
 * Name: do_continuation
 *									*
 * Purpose: Process a continued stabstring.  Fix up global variables as
 *	appropriate.  This is only needed when stabstrings are being parsed.
 *
 * Function:  Read the next stabstring.  If an error occurs, return
 *	++input, which skips over the continuation character and causes
 *	subsequent parsing to fail.  Otherwise, the address of the new
 *	part of the stabstring is returned.
 *
 ************************************************************************/
char *
do_continuation(char *input,
		uint32 *old_symtab_index_ptr,
		char **max_input)
{
    static char	stab_buf[SYMNMLEN+1];
    int		c, num_aux;
    SYMENT	*new_symtab, *old_symtab;
#ifdef READ_FILE
    SYMENT	temp_sym;
#endif

    if (old_symtab_index >= ifile_num_symbols) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(STAB_BAD_CONTINUATION,
"%1$s: 0711-378 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
"\tA continued stabstring is at the end of the symbol table."),
		 Main_command_name,
		 saved_old_symtab_index,
		 get_object_file_name(stab_obj));
	return ++input;			/* Skip over continuation char */
    }

#ifdef READ_FILE
    if (cur_obj_read_file) {
	if (safe_fread(&temp_sym, SYMESZ, stab_obj->o_ifile) != 0)
	    return ++input;		/* Skip over continuation char */
	c = temp_sym.n_sclass;
	num_aux = temp_sym.n_numaux;
    }
    else {
#endif
	c = ((SYMENT*)(old_symtab_base + old_symtab_index * SYMESZ))->n_sclass;
	num_aux = ((SYMENT *)(old_symtab_base
			      + old_symtab_index * SYMESZ))->n_numaux;
#ifdef READ_FILE
    }
#endif

#ifdef READ_FILE
    if (!cur_obj_read_file)
#endif
	old_symtab = aligned_sym_ptr(old_symtab_base, old_symtab_index);
    new_symtab = old_symtab;

    if (new_symtab->n_zeroes != 0) {
	int len;
	memcpy(stab_buf, new_symtab->n_name, SYMNMLEN);
	stab_buf[SYMNMLEN] = '\0';	/* Ensure null-termination */
	/* Compute length of stabstring taking into account
	   trailing nulls.  The 'for' loop must terminate, since
	   new_symtab->n_zeroes != 0 implies that one of the
	   first 4 bytes is non-zero. */
	for (len = SYMNMLEN; stab_buf[len-1] == '\0'; )
	    --len;
	input = stab_buf;
	*max_input = &stab_buf[len];
    }
    else if (new_symtab->n_offset) {
	uint16 stab_len;
	caddr_t stab_string;

	stab_string = get_debug_name(new_symtab->n_offset, &stab_len);
	if (stab_string == NULL) {
	    input = stab_buf;
	    *max_input = &stab_buf[0];
	}
	else {
	    input = stab_string;
	    *max_input = &stab_string[stab_len]; /* Guard against overrun */
	}
    }
    else {
	/* n_zeroes == n_offset = 0:  Continuation is a null string. */
	input = stab_buf;
	*max_input = &stab_buf[0];
    }
    *old_symtab_index_ptr = old_symtab_index++;
    return input;
} /* do_continuation */
/************************************************************************
 * Name: copy_debug_esds
 *									*
 * Purpose:  Copy all symbols from the .file entry to the
 *		first csect entry, or between csect entries.
 *									*
 * Function:
 ************************************************************************/
static RETCODE
copy_debug_esds(OBJECT *obj,
		int read_file)
{
    char	c;
    int		read_obj;
    int		num_aux;
    int		len, syms_kept;
    int		s;
#ifdef STABCMPCT_NODUPSYMS
    int		at_end;
#endif
    int		skip_to_estat = 0;
    char	stab_buf[SYMNMLEN+1];
    char	name_buf[SYMNMLEN+1];
    uint16	stab_len;
    char	*stab_string;
    X_OFF_T	first, info_len, in_offset;
    INFO_LENGTH_FIELD_T info_len_work;
    STR		*sym_name;
    SYMBOL	*sym;
    ITEM 	*i;
    SYMENT	temp_sym, *new_symtab, *old_symtab;
    AUXENT	*new_auxtab;

#define long_or_short_name(sym,n) \
	(((sym)->n_zeroes==0)\
	 ?(char *)(name_buf[SYMNMLEN]='\0',memcpy(name_buf,n->name,SYMNMLEN)) \
	 :sym->n_name)

    if (read_file) {
	read_obj = 1;
	old_symtab = &temp_sym;
    }
    else
	read_obj = 0;

    while (old_symtab_index < ifile_num_symbols) {
#ifdef READ_FILE
	if (read_file) {
	    DEBUG_loc_check(obj, old_symtab_index, __LINE__);
	    if (safe_fread(&temp_sym, SYMESZ, obj->o_ifile) != 0)
		return RC_SEVERE;
	    c = temp_sym.n_sclass;
	    num_aux = temp_sym.n_numaux;
	}
	else {
#endif
	    c = ((SYMENT*)(old_symtab_base
			   + old_symtab_index*SYMESZ))->n_sclass;
	    num_aux = ((SYMENT *)(old_symtab_base
				  + old_symtab_index * SYMESZ))->n_numaux;
#ifdef READ_FILE
	}
#endif

	/* Quit at first C_EXT, C_HIDEXT or C_FILE */
	if (c == C_EXT || c == C_HIDEXT || c == C_FILE) {
	    if (skip_to_estat)
		return RC_SEVERE;
	    break;
	}

	saved_old_symtab_index = old_symtab_index; /* Save for messages */
	old_symtab_index += 1 + num_aux; /* Update for next symbol */

	if (skip_to_estat) {
	    if (c == C_ESTAT)
		skip_to_estat = 0;
	    /* Skip entry */
#ifdef READ_FILE
	    if (read_file && num_aux > 0)
		/* Seek past auxiliary entries */
		if (safe_fseek(obj->o_ifile, num_aux * SYMESZ, SEEK_CUR) != 0)
		    return RC_SEVERE;
#endif
	    continue;
	}

#ifdef READ_FILE
	if (!read_file)
#endif
	    old_symtab = aligned_sym_ptr(old_symtab_base,
					 saved_old_symtab_index);

	/* Copy symbol to output file */
	new_symtab = (SYMENT *)(new_symtab_base + new_symtab_index * SYMESZ);
	new_symtab_index++;
	memcpy(new_symtab, old_symtab, SYMESZ);
	syms_kept = 1;			/* Default is to keep symbol */
#ifdef STABCMPCT_NODUPSYMS
	at_end = 0;
#endif
	switch(c) {
	  case C_ESTAT:
	    break;

	  case C_STAT:
	    /* Don't copy C_STAT entries for TEXT, DATA, and BSS */
	    if (strcmp(new_symtab->n_name, ".text") == 0
		|| strcmp(new_symtab->n_name, ".data") == 0
		|| strcmp(new_symtab->n_name, ".bss") == 0) {
		syms_kept = 0;		/* Delete symbol. */
		break;
	    }
	    /* Fall through */

	  case C_HIDDEN:
	  case C_LABEL:
	    /* If needed, get name from string table */
	    if (new_symtab->n_zeroes == 0) {
		sym_name = get_sym_name2(obj, new_symtab, read_obj,
					 saved_old_symtab_index);
		/* Update Sym and write string table */
		add_symbol_name(sym_name, new_symtab, SYMNMLEN);
	    }
	    /* Fall through */

	  case C_BLOCK:
	  case C_FCN:
	    /* Adjust value and section number */
	    new_symtab->n_value += add_delta;
#if 1
	    /* This is wrong, but fixing it now could affect existing
	       object files.  The following code can be used in AIX 4.2 */
	    new_symtab->n_scnum = temp_aout_hdr.o_sntext;
#else
	    /* We expect the scnum field of the symbol to match the
	       scnum field of the preceding CSECT, but exceptions may
	       exist. */
	    if (new_symtab->n_scnum > N_UNDEF) {
#ifdef DEBUG
		if (cur_csect == NULL
		    || (new_symtab->n_value - add_delta < cur_csect->c_addr
			|| new_symtab->n_value - add_delta
			> cur_csect->c_addr + cur_csect->c_len))
		    internal_error();
#endif
		if (cur_csect != NULL
		    && cur_csect->c_secnum == new_symtab->n_scnum)
		    new_symtab->n_scnum
			= sect_mappings[cur_csect->c_major_sect];
		else {
		    /* Section of preceding doesn't match section
		       number of C_BLOCK or C_FCN symbol.  As a
		       default, use the section number of the .text
		       section. */
		    new_symtab->n_scnum = temp_aout_hdr.o_sntext;
		}
	    }
#endif
	    break;

	  case C_BSTAT:
	    /* Look up the referenced symbol.  It should be a csect or
	       an ER. */
	    i = find_item(obj->oi_syms_lookup, new_symtab->n_value);
	    if (i == NULL || i->item_symbol == NULL) {
		/* Either the referenced symbol does not exist,
		   or there was an error reading the symbol.  If the
		   symbol does not exist, the object file is bad. */
		if (i == NULL) {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_LABEL,
    "%1$s: 0711-593 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
    "\tThe symbol refers to a csect with symbol number %5$d, which was not\n"
    "\tfound. The new symbol cannot be associated with a csect and\n"
    "\tis being ignored."),
			     Main_command_name,
			     "C_BSTAT", saved_old_symtab_index,
			     get_object_file_name(obj),
			     new_symtab->n_value);
		}
		sym = NULL;
	    }
	    else {
		sym = i->item_symbol;
		/* The symbol referenced by the C_BSTAT symbol may have been
		   replaced by another symbol.  If the C_BSTAT was used
		   correctly, the referenced symbol will have been visited.
		   If not, we silently skip over the .bs/.es symbols. */
		if (sym->s_flags & S_HIDEXT) {
		    if (sym->s_flags & S_RESOLVED_OK)
			sym = sym->s_resolved;
		    else
			sym = NULL;	/* Referenced symbol not visited. */
		}
		else {
		    if (sym->s_name->flags & STR_RESOLVED) {
			if (sym->s_name->flags & STR_NO_DEF) {
			    /* Referenced symbol is undefined.  Since the
			       C_BSTAT reference is by symbol table index
			       and not by name, this case can only occur
			       if delcsect processing deleted the symbol. */
			    if (sym->s_name->refs)
				sym = sym->s_name->refs;
			    else
				sym = sym->s_name->first_ext_sym;
			}
			else
			    sym = sym->s_name->first_ext_sym;
		    }
		    else
			sym = NULL;	/* Referenced symbol not visited. */
		}
	    }

	    if (sym == NULL) {
		skip_to_estat = 1;
		syms_kept = 0;		/* Delete symbol. */
	    }
	    else if (sym->s_flags & S_INPNDX_MOD) {
		/* If we have already written the referenced symbol to the
		   output symbol table, we can write its new index. */
		new_symtab->n_value = symtab_index[sym->s_inpndx+1];
	    }
	    else
		save_fixup(&new_symtab->n_value, sym, saved_old_symtab_index);
	    break;

	  case C_INFO:
	    /* If needed, get name from string table.  A short name has
	       already been copied. */
	    if (new_symtab->n_zeroes == 0) {
		sym_name = get_sym_name2(obj, new_symtab, read_obj,
					 saved_old_symtab_index);
		/* Update symbol and write name in string table */
		add_symbol_name(sym_name, new_symtab, SYMNMLEN);
	    }

	    s = new_symtab->n_scnum - 1; /* Get 0-based section number */
	    if (s < 0 || s >= obj->oi_num_sections) {
		bind_err(SAY_NORMAL, RC_ERROR,
			 NLSMSG(BAD_SYMBOL_SCNUM,
	"%1$s: 0711-563 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
	"\tThe n_scnum field contains %5$d.\n"
	"\tThe section number must be between 1 and %6$d."),
			 Main_command_name,
			 long_or_short_name(new_symtab, sym_name),
			 saved_old_symtab_index,
			 get_object_file_name(obj),
			 s+1, obj->oi_num_sections);
		syms_kept = 0;		/* Delete symbol */
	    }
	    else if (obj->oi_section_info[s].sect_type != STYP_INFO) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(BAD_SNINFO,
 "%1$s: 0711-601 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tThe n_scnum field of the symbol contains %5$d, which is not\n"
 "\tthe section number of a .info section. The symbol is being deleted."),
			 Main_command_name,
			 long_or_short_name(new_symtab, sym_name),
			 saved_old_symtab_index,
			 get_object_file_name(obj),
			 s+1);
		syms_kept = 0;		/* Delete symbol */
	    }
	    else {
		first = obj->oi_section_info[s].u.sect_offset;
		info_len = obj->oi_section_info[s].sect_size;
		if (new_symtab->n_value < INFO_LEN_FIELD_LEN
		    || new_symtab->n_value > first + info_len) {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_INFO_VAL,
 "%1$s: 0711-602 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tThe value of the symbol is not a valid offset into .info section %5$d.\n"
 "\tThe symbol is being deleted."),
			     Main_command_name,
			     long_or_short_name(new_symtab, sym_name),
			     saved_old_symtab_index,
			     get_object_file_name(obj),
			     s+1);
		    syms_kept = 0;	/* Delete symbol */
		}
		else {
		    /* Read length of string */
		    in_offset
			= first + new_symtab->n_value - INFO_LEN_FIELD_LEN;
#ifdef READ_FILE
		    if (obj->o_ifile->i_access == I_ACCESS_READ) {
			if (fseek_read(obj->o_ifile, in_offset,
				       &info_len_work, INFO_LEN_FIELD_LEN)) {
			    syms_kept = 0; /* Delete symbol */
			    break;
			}
		    }
		    else
#endif
			memcpy(&info_len_work,
			       obj->o_ifile->i_map_addr + in_offset,
			       INFO_LEN_FIELD_LEN);

		    if (info_len_work < 0
			|| info_len_work + new_symtab->n_value > info_len) {
			bind_err(SAY_NORMAL, RC_SEVERE,
				 NLSMSG(BAD_INFO_LEN,
 "%1$s: 0711-603 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tThe length of the .info string referenced by the symbol is invalid.\n"
 "\tThe string appears to extend beyond the end of section %5$d.\n"
 "\tThe symbol is being deleted."),
				 Main_command_name,
				 long_or_short_name(new_symtab, sym_name),
				 saved_old_symtab_index,
				 get_object_file_name(obj),
				 s+1);
			syms_kept = 0;	/* Delete symbol */
		    }
		    else {
			new_symtab->n_value
			    = info_sect_offset + INFO_LEN_FIELD_LEN;
			info_sect_offset += INFO_LEN_FIELD_LEN + info_len_work;
			save_info(obj, in_offset, info_len_work, new_symtab);
		    }
		}
	    }
	    break;

	  case C_BINCL:
	  case C_EINCL:
	    save_bincl_eincl(obj, new_symtab);

	    /* If needed, get name from string table */
	    if (new_symtab->n_zeroes == 0) {
		sym_name = get_sym_name2(obj, new_symtab, read_file,
					 saved_old_symtab_index);
		/* Update Sym and write string table */
		add_symbol_name(sym_name, new_symtab, SYMNMLEN);
	    }
	    DEBUG_MSG(LINENUMS_DEBUG,
		      (SAY_NO_NLS, "%s: symbol %d %s from %s",
		       c == C_BINCL ? "BINCL" : "EINCL",
		       new_symtab_index - 1,
		       long_or_short_name(new_symtab, sym_name),
		       get_object_file_name(obj)));
	    break;

	  case C_BCOMM:
	  case C_ECOMM:
	    /* Short names are copied when the symbol is copied. Long names
	       come from .debug section, but they aren't parsed. */
	    if (new_symtab->n_zeroes == 0) {
		/* Get name from .debug section */
		if (new_symtab->n_offset != 0) {
		    uint16 stab_len;
		    stab_string = get_debug_name(new_symtab->n_offset,
						&stab_len);
		    if (stab_string == NULL) {
			syms_kept = 0;	/* Delete symbol */
			break;
		    }
		    (void) save_unique_stabstring(new_symtab, stab_string,
						  stab_len, SUS_TO_DEBUG);
		}
		else
		    goto null_stabstring;
	    }
	    break;

	  case C_DECL:
	    /* WARNING: Do not move this case label.  The code falls through
	       if STABCMPCT_NODUPSYMS is not defined, and may fall
	       through otherwise. */
#ifdef STABCMPCT_NODUPSYMS
	    if ((Switches.stabcmpct == STABCMPCT_NODUPSYMS ||
		 Switches.stabcmpct == STABCMPCT_FULLNODUPSYMS)
		&& num_aux == 0) {
		/* Get name now, so we can check first character */
		if (new_symtab->n_zeroes != 0)
		    c = new_symtab->n_name[0];
		else if (new_symtab->n_offset != 0) {
		    stab_string = get_debug_name(new_symtab->n_offset,
						 &stab_len);
		    if (stab_string == NULL)
			goto null_stabstring;
		    else
			c = stab_string[0];
		}
		else
		    goto null_stabstring;

		if (c == ':') {
		    /* Move symbol to end of symbol table. */
		    SYMENT *temp_symtab = new_symtab;
		    new_symtab = (SYMENT *)(new_symtab_base
					    + new_symtab_end_index * SYMESZ);
		    memcpy(new_symtab, temp_symtab, SYMESZ);
		    --new_symtab_index;	/* Restore value */
		    --new_symtab_end_index;
		    at_end = 1;
		}
		if (new_symtab->n_zeroes != 0)
		    goto handle_short_stabstring;
		goto handle_long_name;
	    }
#endif

	  case C_ECOML:
	  case C_FUN:   case C_ENTRY:
	  case C_GSYM:  case C_LSYM:
	  case C_PSYM:  case C_RSYM:
	  case C_RPSYM: case C_STSYM:
	  case C_TCSYM:
	    if (new_symtab->n_zeroes != 0) {
	      handle_short_stabstring:
		/* A short name is copied when the symbol table entry is
		   copied.  Nevertheless, if we're parsing the stab strings,
		   we must still call put_debug_name() to do the parse. */
		if (Switches.stabcmpct >= STABCMPCT_RENUM) {
		    memcpy(stab_buf, new_symtab->n_name, SYMNMLEN);
		    stab_buf[SYMNMLEN] = '\0'; /* Ensure null-termination */
		    /* Compute length of stabstring taking into account
		       trailing nulls.  The 'for' loop must terminate, since
		       new_symtab->n_zeroes != 0 implies that one of the
		       first 4 bytes is non-zero. */
		    for (len = SYMNMLEN; stab_buf[len-1] == '\0'; )
			--len;
		    syms_kept = put_debug_name(new_symtab, stab_buf, len,
					       &old_symtab_index,
					       saved_old_symtab_index
#ifdef STABCMPCT_NODUPSYMS
					       , at_end
#endif
					       );
		}
	    }
	    else if (new_symtab->n_offset) {
		stab_string = get_debug_name(new_symtab->n_offset, &stab_len);
	      handle_long_name:
		if (stab_string == NULL)
		    syms_kept = 0;	/* Delete invalid symbol */
		else
		    syms_kept = put_debug_name(new_symtab,
					       stab_string, stab_len,
					       &old_symtab_index,
					       saved_old_symtab_index
#ifdef STABCMPCT_NODUPSYMS
					       , at_end
#endif
					       );
	    }
	    else {
	      null_stabstring:
		if (Switches.verbose)
		    bind_err(SAY_NORMAL, RC_WARNING,
			     NLSMSG(VALIDATE_NULL_STAB,
    "%1$s: 0711-256 WARNING: Symbol entry %2$d in object %3$s:\n"
    "\tStabstring symbol has n_zeroes = n_offset = 0.\n"
    "\tThe symbol is being copied to the output file."),
			     Main_command_name, saved_old_symtab_index,
			     get_object_file_name(stab_obj));
	    }
	    if (syms_kept == 0) {
#ifdef STABCMPCT_NODUPSYMS
		if (at_end == 1) {
		    new_symtab_end_index++;
		    ++new_symtab_index;	/* Increment, because the variable
					   will be decremented below. */
		}
#endif
		/* Don't decrement new_symtab_index here.  It will be
		   decremented below. */
	    }
	    else if (syms_kept > 0) {
		new_symtab_index += syms_kept - 1; /* Advance by number of
						      stabstrings written. */
	    }
	    else {
		/* syms_kept < 0: Symbols written at end. */
		new_symtab_end_index -= (syms_kept - 1);
	    }
	    break;

	  case C_NULL:
	    if (new_symtab->n_value == C_NULL_VALUE) {
		syms_kept = 0;		/* Delete symbol. */
		break;
	    }
	    /* Fall through to error message */

	  default:
	    bind_err(SAY_NORMAL, RC_WARNING,
		     NLSMSG(SAVE_RCW_BADSC,
 "%1$s: 0711-870 WARNING: Symbol table entry %2$d in object %3$s:\n"
 "\tUnknown symbol type %4$d. The symbol is being copied to the output file."),
		     Main_command_name, saved_old_symtab_index,
		     get_object_file_name(obj), c);
	}
	if (num_aux) {
	    if (syms_kept == 0) {	/* Symbol was deleted. */
		--new_symtab_index;	/* Adjust index */
#ifdef READ_FILE
		if (read_file)
		    /* Seek past auxiliary entries */
		    if (safe_fseek(obj->o_ifile, num_aux * SYMESZ, SEEK_CUR)
			!= 0)
			return RC_SEVERE;
#endif
	    }
	    else {
		new_auxtab
		    = (AUXENT*)(new_symtab_base + new_symtab_index * SYMESZ);
		/* Copy auxiliary entries */
#ifdef READ_FILE
		if (read_file) {
		    DEBUG_loc_check(obj, saved_old_symtab_index + 1, __LINE__);
		    if (safe_fread(new_auxtab, AUXESZ * num_aux, obj->o_ifile)
			!= 0)
			return RC_SEVERE;
		}
		else
#endif
		    memcpy(new_auxtab,
			   (AUXENT *)(old_symtab_base
				      + (saved_old_symtab_index + 1) * SYMESZ),
			   num_aux * SYMESZ);

		new_symtab_index += num_aux;
	    }
	}
	else if (syms_kept == 0)
	    --new_symtab_index;	/* Adjust index */
    }

    return RC_OK;
#undef long_or_short_name
} /* copy_debug_esds */
/************************************************************************
 * Name: fixup_symbol_table
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
void
fixup_symbol_table(void)
{
    fixup_bincl_eincl();
    fixup_statics();
}
/************************************************************************
 * Name: add_symbol_name
 *									*
 * Purpose:	Write a symbol name into a symbol table entry.  If the
 *		name is less than or equal to 'len', the name can be
 *		written directly to the symbol.  Otherwise, information
 *		about the name is saved that it can be written to the string
 *		table later, and the eventual offset into the string table
 *		is written into the symbol.
 *									*
 * Function:
 ************************************************************************/
static RETCODE
add_symbol_name(STR *name,
		SYMENT *symtab,
		int len)
{
    if (name->len <= len) {
	/* Name will fit in symbol */
	/* This function will pad the name with nulls if necessary. */
	strncpy(symtab->n_name, name->name, len);
    }
    else {
	/* name goes into string table) */
	symtab->n_zeroes = 0;
	if (name->flags & STR_STRING_USED)
	    /* String already in table */
	    symtab->n_offset = name->str_value;
	else if (name->name[0] != '.'
		 && name->alternate
		 && (name->alternate->flags & STR_STRING_USED)) {
	    /* Dot name has been saved. We can share plain name part */
	    name->str_value = name->alternate->str_value + 1;
	    symtab->n_offset = name->str_value;
	    name->flags |= STR_STRING_USED;
	}
	else {
	    /* New instance  */
	    name->flags |= STR_STRING_USED;
	    saved_strings[num_used_strings++] = name;
	    symtab->n_offset= new_string_table_size;
	    name->str_value = new_string_table_size;
	    new_string_table_size += name->len + 1;
	}
    }
}
/************************************************************************
 * Name: fixup_statics
 *									*
 * Purpose: Fixup C_BSTAT symbols--those symbols containing forward
 *	references to static csect symbols.
 *									*
 * Function:
 ************************************************************************/
static void
fixup_statics(void)
{
    struct fixup_block	*fixup_b;	/* Temp struct for line number entry */
    int			i;
    SYMBOL		*sym;

    for (fixup_b = fst_fixup_b; fixup_b; fixup_b = fixup_b->next)
	for (i = 0; i < fixup_b->in_use; i++) {
	    sym = fixup_b->block[i].ref_sym;
	    if (sym->s_flags & S_INPNDX_MOD)
		*fixup_b->block[i].fixup_value_addr
		    = symtab_index[sym->s_inpndx+1];
	    else {
		*fixup_b->block[i].fixup_value_addr = -1;
#ifdef DEBUG
		internal_error();
#endif
	    }
	}
}
/************************************************************************
 * Name: get_fixup_entry
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static FIXUP_ENTRY *
get_fixup_entry(int i)
{
    char		*id = "get_fixup_entry";

    if (fixup_blocks[i].cur == NULL) {
	fixup_blocks[i].fst = emalloc(sizeof(*fixup_blocks[i].fst), id);
	fixup_blocks[i].cur = fixup_blocks[i].fst;
    }
    else if (fixup_blocks[i].cur->in_use != fixup_blocks[i].bump)
	return &fixup_blocks[i].cur->a[fixup_blocks[i].cur->in_use++];
    else {
	fixup_blocks[i].cur->next
	    = emalloc(sizeof(*fixup_blocks[i].cur->next), id);
	fixup_blocks[i].cur = fixup_blocks[i].cur->next;
    }

    fixup_blocks[i].cur->a 
	= emalloc(fixup_blocks[i].bump * sizeof(*fixup_blocks[i].cur->a), id);
    fixup_blocks[i].cur->in_use = 1;
    fixup_blocks[i].cur->next = NULL;

    return fixup_blocks[i].cur->a;
}
/************************************************************************
 * Name: save_info
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static void
save_info(OBJECT *obj,
	  X_OFF_T off,
	  INFO_LENGTH_FIELD_T len,
	  SYMENT *syment)
{
    char	*id = "save_info_info";
    FIXUP_ENTRY	*incl_b = get_fixup_entry(INFO_INDEX);

    incl_b->in_obj = obj;
    incl_b->u.ii.len = (X_OFF_T)len + INFO_LEN_FIELD_LEN;
    incl_b->u.ii.in_offset = off;
    incl_b->u.ii.syment = syment;
}
/************************************************************************
 * Name: save_bincl_eincl
 *									*
 * Purpose: Save address of a C_BINCL or C_EINCL symbol so that it can
 *		be fixed up when line number entries are written to the
 *		output file.
 *									*
 ************************************************************************/
static void
save_bincl_eincl(OBJECT *obj,
		 SYMENT *syment)
{
    char		*id = "save_bincl_eincl";
    struct bincl_block	*temp_bincl_b;

    if (cur_bincl_b == NULL) {
	fst_bincl_b = emalloc(sizeof(*fst_bincl_b), id);
	fst_bincl_b->next = NULL;
	fst_bincl_b->in_use = 0;
	cur_bincl_b = fst_bincl_b;
    }
    else if (cur_bincl_b->in_use == BINCL_BLOCK_BUMP) {
	temp_bincl_b = emalloc(sizeof(*temp_bincl_b), id);
	temp_bincl_b->next = NULL;
	temp_bincl_b->in_use = 0;
	cur_bincl_b->next = temp_bincl_b;
	cur_bincl_b = temp_bincl_b;
    }
    cur_bincl_b->block[cur_bincl_b->in_use].obj = obj;
    cur_bincl_b->block[cur_bincl_b->in_use].syment = syment;
    cur_bincl_b->in_use++;
}
/************************************************************************
 * Name: free_lnranges
 *									*
 * Purpose: Free data structure that is no longer needed.
 *									*
 ************************************************************************/
static void
free_lnranges(void)
{
    struct lnrange_block	*lnr_b, *next_b;

    for (lnr_b = fst_lnrange_b; lnr_b; lnr_b = next_b) {
	next_b = lnr_b->next;
	efree(lnr_b);
    }
    fst_lnrange_b = cur_lnrange_b = NULL;
}
/************************************************************************
 * Name: fixup_bincl_eincl
 *									*
 * Purpose: Relocate the values of C_BINCL and C_EINCL symbols.
 *	As each C_BINCL or C_EINCL symbol is processed, its address is
 *	saved.  This routine adds the proper relocation amount to the
 *	symbols' values if the range of line numbers referenced by the
 *	symbol was saved.  Otherwise, the storage class of the symbol
 *	is changed to C_NULL and its value is set to C_NULL_VALUE.
 *									*
 ************************************************************************/
static void
fixup_bincl_eincl(void)
{
    struct bincl_block	*bi_b, *bi_next;
    int			i;
    SYMENT		*new_symtab;
    X_OFF_T		*delta_ptr;

    for (bi_b = fst_bincl_b; bi_b; bi_b = bi_next) {
	bi_next = bi_b->next;
	for (i = 0; i < bi_b->in_use; i++) {
	    new_symtab = bi_b->block[i].syment;
	    delta_ptr = convert_ln_address(bi_b->block[i].obj,
					   (X_OFF_T)new_symtab->n_value);
	    if (delta_ptr == NULL) {
		new_symtab->n_sclass = C_NULL;
		new_symtab->n_value = C_NULL_VALUE;
		DEBUG_MSG(LINENUMS_DEBUG,
			  (SAY_NO_NLS,
		   "BINCL/EINCL deleted: range not saved"));
	    }
	    else {
		DEBUG_MSG(LINENUMS_DEBUG,
			  (SAY_NO_NLS,
			   "BINCL: Symtab %d %08x -> %08x (delta=%d) %s",
			   ((caddr_t)bi_b->block[i].syment
			    - (caddr_t)new_symtab_base)/SYMESZ,
			   new_symtab->n_value,
			   new_symtab->n_value + *delta_ptr,
			   *delta_ptr,
			   get_object_file_name(bi_b->block[i].obj)));
		new_symtab->n_value += *delta_ptr;
	    }
	}
	efree(bi_b);
    }
    if (fst_lnrange_b)
	free_lnranges();
} /* fixup_bincl_eincl */
/************************************************************************
 * Name: convert_ln_address
 *									*
 * Purpose: Find relocation amounts for C_BINCL and C_EINCL symbols.  For
 *	the given file offset (the 'addr' parameter) in a given object,
 *	find a range of line number entries saved from the object file.
 *	Associated with the range is the relocation amount for the block.
 *
 * Returns:
 *	Address of the relocation amount if an appropriate block is found.
 *	NULL if a block is not found.  This can occur when part of a
 *	source file is kept (the C_FILE symbol is kept) but some csects
 *	from the file are not kept.
 *									*
 ************************************************************************/
static X_OFF_T *
convert_ln_address(OBJECT *obj,
		   X_OFF_T addr)
{
    struct lnrange_block	*lnr_b;
    int				i;

    /* Brute force */
    for (lnr_b = fst_lnrange_b; lnr_b; lnr_b = lnr_b->next)
	for (i = 0; i < lnr_b->in_use; i++)
	    if (lnr_b->block[i].obj == obj
		&& addr >= lnr_b->block[i].old_range_begin
		&& addr < lnr_b->block[i].old_range_end)
		return &lnr_b->block[i].delta;

    return NULL;			/* No appropriate range saved. */
}
/************************************************************************
 * Name: save_range
 *									*
 * Purpose: Save information about each range of line numbers copied from an
 *	input file.  This is needed to relocate the file offsets found in
 *	C_BINCL and C_EINCL symbols.
 *
 *	Multiple, contiguous ranges occur frequently, so they are coalesced.
 *									*
 ************************************************************************/
static void
save_range(OBJECT *obj,			/* Current input object file. */
	   X_OFF_T r_begin,		/* File offset in input file to
					   a block of line number entries. */
	   X_OFF_T r_end,		/* File offset in input file of byte
					   past block of line number entries. */
	   X_OFF_T r_delta)		/* Difference between file offsets of
					   line number blocks in output and
					   input files. */
{
    char			*id = "save_range";
    struct lnrange_block	*temp_lnrange_b;
    static LN_RANGE		*prev_entry = NULL;

    if (prev_entry &&
	prev_entry->obj == obj
	&& prev_entry->old_range_end == r_begin
	&& prev_entry->delta == r_delta) {
	/* Coalesce contiguous ranges from same object. */
	prev_entry->old_range_end = r_end;
	DEBUG_MSG(LINENUMS_DEBUG,
		  (SAY_NO_NLS,
		   "RANGE: Coalesce: end = 0x%08x count=%d",
		   r_end, (r_end - prev_entry->old_range_begin) / LINESZ));
	return;
    }

    DEBUG_MSG(LINENUMS_DEBUG,
	      (SAY_NO_NLS,
	       "RANGE: old=0x%08X new=0x%08X (delta %d) count=%d obj=%s",
	       r_begin, r_begin + r_delta, r_delta,
	       (r_end - r_begin) / LINESZ,
	       get_object_file_name(obj)));

    if (cur_lnrange_b == NULL) {
	/* Allocate first block of LN_RANGEs. */
	fst_lnrange_b = emalloc(sizeof(*fst_lnrange_b), id);
	fst_lnrange_b->next = NULL;
	fst_lnrange_b->in_use = 0;
	cur_lnrange_b = fst_lnrange_b;
    }
    else if (cur_lnrange_b->in_use == LNRANGE_BLOCK_BUMP) {
	/* Allocate additional block of LN_RANGEs. */
	temp_lnrange_b = emalloc(sizeof(*temp_lnrange_b), id);
	temp_lnrange_b->next = NULL;
	temp_lnrange_b->in_use = 0;
	cur_lnrange_b->next = temp_lnrange_b;
	cur_lnrange_b = temp_lnrange_b;
    }
    prev_entry = &cur_lnrange_b->block[cur_lnrange_b->in_use];
    prev_entry->obj = obj;
    prev_entry->old_range_begin = r_begin;
    prev_entry->old_range_end = r_end;
    prev_entry->delta = r_delta;
    cur_lnrange_b->in_use++;
} /* save_range */
/************************************************************************
 * Name: save_fixup
 *									*
 * Purpose: A C_BSTAT contains a reference to the symbol table index of
 *	the csect containing static symbols.  While processing the output
 *	symbol table, we can write the new n_value if the referenced csect
 *	has already been written.  Otherwise, we must save the address of
 *	the symbol table entry so that it can be fixed up later.
 *									*
 * Function:
 ************************************************************************/
static void
save_fixup(unsigned long *fixup_value_addr, /* Address of n_value in symbol
					       table entry that needs to
					       be fixed up. */
	   SYMBOL *ref_sym,		/* Symbol being referenced. */
	   uint32 old_sym_index)	/* Input symbol table index (for
					   error messages). */
{
    char		*id = "save_fixup";
    struct fixup_block	*temp_fixup_b;

    if (cur_fixup_b == NULL) {
	fst_fixup_b = emalloc(sizeof(*fst_fixup_b), id);
	fst_fixup_b->next = NULL;
	fst_fixup_b->in_use = 0;
	cur_fixup_b = fst_fixup_b;
    }
    else if (cur_fixup_b->in_use == FIXUP_BLOCK_BUMP) {
	temp_fixup_b = emalloc(sizeof(*temp_fixup_b), id);
	temp_fixup_b->next = NULL;
	temp_fixup_b->in_use = 0;
	cur_fixup_b->next = temp_fixup_b;
	cur_fixup_b = temp_fixup_b;
    }
    cur_fixup_b->block[cur_fixup_b->in_use].fixup_value_addr = fixup_value_addr;
    cur_fixup_b->block[cur_fixup_b->in_use].ref_sym = ref_sym;
    cur_fixup_b->block[cur_fixup_b->in_use].input_sym_number = old_sym_index;
    cur_fixup_b->in_use++;
}
/************************************************************************
 * Name: new_line_number_info
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static struct addr_info *
new_line_number_info(void)
{
    char		*id = "new_line_number_info";
    struct lno_block	*temp_lno_b;

    if (cur_lno_b == NULL) {
	fst_lno_b = emalloc(sizeof(*fst_lno_b), id);
	fst_lno_b->next = NULL;
	fst_lno_b->in_use = 0;
	cur_lno_b = fst_lno_b;
    }
    else if (cur_lno_b->in_use == LNNO_BLOCK_BUMP) {
	temp_lno_b = emalloc(sizeof(*temp_lno_b), id);
	temp_lno_b->next = NULL;
	temp_lno_b->in_use = 0;
	cur_lno_b->next = temp_lno_b;
	cur_lno_b = temp_lno_b;
    }
    return cur_lno_b->block + cur_lno_b->in_use++;
}
/************************************************************************
 * Name: fcn_lnnos
 *									*
 * Purpose:  Process function auxiliary entry and line number,
 *	and exception section information.
 *									*
 * Function:
 ************************************************************************/
static RETCODE
fcn_lnnos(OBJECT *obj,			/* Current object file */
	  AUXENT *new_auxtab,		/* Pointer for writing new
					   function auxiliary entry */
	  int cursect,			/* Input file section number */
	  int read_file)		/* 1 if we're reading input file */
{
    int		d1, d2;
    struct addr_info	*lni;
    X_OFF_T		base_offset;

    /* Copy function auxiliary entry from input file to output file. */
#ifdef READ_FILE
    if (read_file) {
	DEBUG_loc_check(obj, old_symtab_index, __LINE__);
	if (safe_fread(new_auxtab, AUXESZ, obj->o_ifile) != 0)
	    return RC_SEVERE;
    }
    else
#endif
	memcpy(new_auxtab,
	       (AUXENT *)(old_symtab_base + old_symtab_index * SYMESZ),
	       AUXESZ);

    if (obj->o_member_info == NULL)
	base_offset = 0;
    else
	base_offset = obj->o_member_info->o_ohdr_off
	    + AR_HSZ + ROUND(obj->o_member_info->o_member->len, 2);

    /* Fix up the function auxiliary entry.  Set x_lnnoptr, x_exptr, and
       x_endndx for entry.  The x_lnnoptr and x_exptr will require fixup later
       when we know the final position of the output line number table and
       exception section. */

    /* QUESTION:  Can x_endndx be non-zero, but x_lnnoptr and x_exptr == 0? */

    /* Set x_endndx to ending symbol index in output symbol table.  This is
       a guess, since we may not have the same number of intervening
       symbols in the old and new symbol tables, espcially if stab string
       entries are deleted. */
    if (new_auxtab->x_sym.x_fcnary.x_fcn.x_lnnoptr != 0) {
	new_auxtab->x_sym.x_fcnary.x_fcn.x_endndx
	    += new_symtab_index - old_symtab_index;
	d1 = 2;
    }
    else
	d1 = 0;
    d2 = (new_auxtab->x_sym.x_exptr != 0);

    if (d1 | d2) {
	/* Fixup will be needed. */
	lni = new_line_number_info();	/* Get structure for saving info */
	lni->in_obj = obj;
	lni->new_index = new_symtab_index;
	lni->delta = add_delta;		/* Relocation delta for function */

	/* We want to keep the address of the last possible line number entry
	   in the input file, to guard against erroneous errors.  We also
	   use the lni->last field to indicate whether the x_exptr field is 0
	   or not, so there are 4 possibilities:
	   lni->last < -1: x_lnnoptr only (-lni->last-1 is the address value)
	   lni->last == -1: Neither x_lnnoptr nor x_exptr (shouldn't happen?)
	   lni->last == 0: x_exptr only
	   lni->last > 0: Both x_lnnoptr and x_exptr
	   */

	switch(d1 + d2) {
	  case 1:			/* x_exptr only */
	    lni->last = 0;
	    exceptions_seen = 1;
	    break;
	  case 2:			/* x_lnnoptr only */
	    lni->last = -(base_offset
			  + obj->oi_section_info[cursect-1].l_linenum_last) - 1;
	    line_numbers_seen = 1;
	    break;
	  case 3:			/* Both x_exprt and x_lnnoptr */
	    lni->last = (base_offset
			 + obj->oi_section_info[cursect-1].l_linenum_last);
	    exceptions_seen = 1;
	    line_numbers_seen = 1;
	    break;
	  default:
	    internal_error();
	}
    }

    return RC_OK;
} /* fcn_lnnos */
/************************************************************************
 * Name: write_line_numbers
 *									*
 * Purpose: Write the line number entries (for the .text section) containing
 *	all line number entries referenced by saved symbol table entries.
 *	Information about referenced line number entries is saved (along with
 *	exception entry information) in a structure rooted by fst_lno_b.
 *									*
 * Function:  Write the line number entries directly to the output file
 *	beginning at file offset 'line_offset'  If any entries are written
 *	the section should be aligned on a halfword boundary.
 *
 * Returns:	The offset to the next available byte in the output file.
 *		Assigns the number of entries written to
 *		'*num_linenum_entries'.
 *
 ************************************************************************/
X_OFF_T
write_line_numbers(X_OFF_T output_offset,
		   long *num_linenum_entries)
{
    int		i;
#ifdef DEBUG
    long	num_lnnos = 0;
#endif
    caddr_t	mem_ptr;
    X_OFF_T	base_offset;
    X_OFF_T	range_begin, range_delta;
    X_OFF_T	input_offset;
    X_OFF_T	last_offset;
    AUXENT	*new_auxtab;
    OBJECT	*obj;
    struct lno_block	*f_b;		/* Ptr to exception/line num entries. */
    struct addr_info	*f;

    if (line_numbers_seen == 0) {
	*num_linenum_entries = 0;
	return output_offset;
    }

    /* Line number entries are always halfword aligned. */
    output_offset = ROUND(output_offset, 2);
    Scnhdr[temp_aout_hdr.o_sntext - 1].s_lnnoptr = output_offset;

    /* Point to where line number info should be written */
    mem_ptr = Shmaddr + output_offset;

    for (f_b = fst_lno_b; f_b; f_b = f_b->next)
	for (i = 0; i < f_b->in_use; i++) {
	    f = &f_b->block[i];

	    /* Decode f->last (see comment in fcn_lnnos) */
	    if (f->last == 0 || f->last == -1)
		continue;		/* No line numbers for this symbol */
	    if (f->last < 0)
		last_offset = -(f->last + 1);
	    else
		last_offset = f->last;

	    obj = f->in_obj;
	    ifile_reopen_remap(obj->o_ifile);

	    if (obj->o_member_info == NULL)
		base_offset = 0;
	    else
		base_offset = obj->o_member_info->o_ohdr_off
		    + AR_HSZ + ROUND(obj->o_member_info->o_member->len, 2);

	    /* Get address of new function auxiliary entry */
	    new_auxtab = (AUXENT *)(new_symtab_base + f->new_index * SYMESZ);

	    /* Get address of line number entries for fcn (in input file) */
	    range_begin = new_auxtab->x_sym.x_fcnary.x_fcn.x_lnnoptr;
	    range_delta = (mem_ptr - Shmaddr) - range_begin;
	    input_offset = base_offset + range_begin;

	    /* Copy the first line number entry, which should be
	       the function declaration */
#ifdef READ_FILE
	    if (obj->o_ifile->i_access == I_ACCESS_READ) {
		if (fseek_read(obj->o_ifile, input_offset, mem_ptr, LINESZ))
		    continue;		/* A bad error, but we keep trying */
	    }
	    else
#endif
		memcpy(mem_ptr, obj->o_ifile->i_map_addr + input_offset,
		       LINESZ);

	    new_auxtab->x_sym.x_fcnary.x_fcn.x_lnnoptr = mem_ptr - Shmaddr;

	    ((LINENO *)mem_ptr)->l_addr.l_symndx = f->new_index - 1;
	    mem_ptr += LINESZ;
	    input_offset += LINESZ;
#ifdef DEBUG
	    num_lnnos++;
#endif

	    /* Copy other lnnos, adjust addresses by add_delta */
	    while(input_offset < last_offset) {
		/* Copy the lnno entry */
#ifdef READ_FILE
		if  (obj->o_ifile->i_access == I_ACCESS_READ) {
		    if (safe_fread(mem_ptr, LINESZ, obj->o_ifile) != 0)
			break;		/* Stop copying for current function */
		    if (((LINENO *)mem_ptr)->l_lnno == 0)
			break; /* Quit at next function */
		}
		else
#endif
		{
		    if (((LINENO *)(obj->o_ifile->i_map_addr
				    + input_offset))->l_lnno == 0)
			break; /* Quit at next function */
		    memcpy(mem_ptr, obj->o_ifile->i_map_addr + input_offset,
			   LINESZ);
		}

		/* Adjust address */
		((LINENO *)mem_ptr)->l_addr.l_paddr += f->delta;
		mem_ptr += LINESZ;
		input_offset += LINESZ;
#ifdef DEBUG
		num_lnnos++;
#endif
	    }
	    /* In case any BINCL-EINCLs exist, save the range of
	       line number entries saved. */
	    save_range(obj, range_begin,
		       input_offset - base_offset, range_delta);
	}

#ifdef DEBUG
    if (num_lnnos == 0
	|| num_lnnos != (mem_ptr - Shmaddr - output_offset) / LINESZ)
	internal_error();
#endif

    *num_linenum_entries = ((mem_ptr - Shmaddr) - output_offset) / LINESZ;
    return mem_ptr - Shmaddr;
} /* write_line_numbers */
/************************************************************************
 * Name: write_exception_section
 *									*
 * Purpose: Write the exception section containing all exception entries
 *	referenced by saved symbol table entries.  Information about
 *	referenced exception entries is saved (along with line number
 *	information) in a structure rooted by fst_lno_b.
 *									*
 * Function:  Write the exception entries directly to the output file
 *	beginning at file offset 'except_offset'  If any entries are written
 *	the section should be aligned on a halfword boundary.
 *
 * Returns:	The offset to the next available byte in the output file.
 *
 ************************************************************************/
X_OFF_T
write_exception_section(X_OFF_T output_offset)
{
    int		i;
    caddr_t	mem_ptr;
    X_OFF_T	input_offset;
    X_OFF_T	last_offset;
    AUXENT	*new_auxtab;
    OBJECT	*obj;
    struct lno_block	*f_b;		/* Ptr to exception/line num entries. */
    struct addr_info	*f;
#ifdef DEBUG
    long	num_except_entries = 0;	/* Count of exception sect. entries. */
#endif

    if (exceptions_seen == 0)
	return output_offset;

    /* Exception table entries are always halfword aligned. */
    output_offset = ROUND(output_offset, 2);
    Scnhdr[next_scn].s_scnptr = output_offset;

    /* Point to where exception section should be written */
    mem_ptr = Shmaddr + output_offset;

    for (f_b = fst_lno_b; f_b; f_b = f_b->next)
	for (i = 0; i < f_b->in_use; i++) {
	    f = &f_b->block[i];
	    if (f->last < 0)
		continue;		/* No exception list for this symbol */

	    obj = f->in_obj;
	    ifile_reopen_remap(obj->o_ifile);

	    /* Get address of new function auxiliary entry */
	    new_auxtab = (AUXENT *)(new_symtab_base + f->new_index * SYMESZ);

	    /* Get address of exception entries for fcn (in input file) */
	    input_offset = new_auxtab->x_sym.x_exptr;

	    if (obj->o_member_info != NULL)
		input_offset += obj->o_member_info->o_ohdr_off
		    + AR_HSZ + ROUND(obj->o_member_info->o_member->len, 2);

	    /* Copy the first exception entry, which should refer to a function
	       symbol. */
#ifdef READ_FILE
	    if (obj->o_ifile->i_access == I_ACCESS_READ) {
		if (fseek_read(obj->o_ifile, input_offset, mem_ptr, EXCEPTSZ))
		    continue;		/* A bad error, but we keep trying */
	    }
	    else
#endif
		memcpy(mem_ptr, obj->o_ifile->i_map_addr + input_offset,
		       EXCEPTSZ);

	    new_auxtab->x_sym.x_exptr = mem_ptr - Shmaddr;

	    ((EXCEPTAB *)mem_ptr)->e_addr.e_symndx = f->new_index - 1;
	    mem_ptr += EXCEPTSZ;
	    input_offset += EXCEPTSZ;
#ifdef DEBUG
	    num_except_entries++;
#endif

	    /* Copy other exception entries, adjust addresses by add_delta */
	    last_offset
		= obj->oi_section_info[obj->oi_except_sect_i].u.sect_offset
		    + obj->oi_section_info[obj->oi_except_sect_i].sect_size;
	    while(input_offset < last_offset) {
		/* Copy the exception entry */
#ifdef READ_FILE
		if  (obj->o_ifile->i_access == I_ACCESS_READ) {
		    if (safe_fread(mem_ptr, EXCEPTSZ, obj->o_ifile) != 0)
			break;		/* Stop copying for current function */
		    if (((EXCEPTAB *)mem_ptr)->e_reason == 0)
			break; /* Quit at next function */
		}
		else
#endif
		{
		    if (((EXCEPTAB *)(obj->o_ifile->i_map_addr
				      + input_offset))->e_reason == 0)
			break; /* Quit at next function */

		    memcpy(mem_ptr,
			   obj->o_ifile->i_map_addr + input_offset,
			   EXCEPTSZ);
		}

		/* Adjust address */
		((EXCEPTAB *)mem_ptr)->e_addr.e_paddr += f->delta;
		mem_ptr += EXCEPTSZ;
		input_offset += EXCEPTSZ;
#ifdef DEBUG
		num_except_entries++;
#endif
	    }
	}

#ifdef DEBUG
    if (num_except_entries == 0)
	internal_error();
#endif

    output_offset = mem_ptr - Shmaddr;

    (void) strncpy(Scnhdr[next_scn].s_name, _EXCEPT, sizeof(Scnhdr->s_name));
    Scnhdr[next_scn].s_paddr = Scnhdr[next_scn].s_vaddr = 0;
    Scnhdr[next_scn].s_size = output_offset - Scnhdr[next_scn].s_scnptr;
    Scnhdr[next_scn].s_relptr = Scnhdr[next_scn].s_lnnoptr = 0;
    Scnhdr[next_scn].s_nreloc = Scnhdr[next_scn].s_nlnno = 0;
    Scnhdr[next_scn].s_flags = STYP_EXCEPT;
    next_scn++;				/* Increment for next section */

    return output_offset;
} /* write_exception_section */
/************************************************************************
 * Name: write_info_section
 *									*
 * Purpose: Write the info section containing all info strings
 *	referenced by saved symbol table entries.  Information about
 *	referenced info entries is saved the array fixup_blocks.
 *									*
 * Function:  Write the info strings directly to the output file
 *	beginning at file offset 'output_offset'  If any entries are written,
 *	the section should be aligned on a halfword boundary.
 *
 * Returns:	The offset to the next available byte in the output file.
 *
 ************************************************************************/
X_OFF_T
write_info_section(X_OFF_T output_offset)
{
    int			i;
    caddr_t		mem_ptr;
    uint16		info_section;
    SCNHDR		*sect_hdr;
    X_OFF_T		sect_offset;
    OBJECT		*obj;
    FIXUP_ENTRIES	*f_b, *f_next;
    FIXUP_ENTRY		*f;

    if (fixup_blocks[INFO_INDEX].fst == NULL)
	return output_offset;

    sect_hdr = &Scnhdr[next_scn++];
    info_section = next_scn;		/* 1-based section number */

    /* Align all sections on halfword boundary. */
    output_offset = ROUND(output_offset, 2); /* Offset in output file to
						info section. */
    sect_hdr->s_scnptr = output_offset;

    /* Point to where info section should be written */
    mem_ptr = Shmaddr + output_offset;
    sect_offset = 0;
    for (f_b = fixup_blocks[INFO_INDEX].fst; f_b; f_b = f_next) {
	f = f_b->a;
	f_next = f_b->next;
	for (i = 0; i < f_b->in_use; i++, f++) {
	    obj = f->in_obj;
	    ifile_reopen_remap(obj->o_ifile);

	    f->u.ii.syment->n_scnum = info_section;
	    /* Copy the length field and string. */
#ifdef READ_FILE
	    if (obj->o_ifile->i_access == I_ACCESS_READ) {
		if (fseek_read(obj->o_ifile, f->u.ii.in_offset, mem_ptr,
			       f->u.ii.len))
		    continue;		/* A bad error, but we keep trying */
	    }
	    else
#endif
		memcpy(mem_ptr, obj->o_ifile->i_map_addr + f->u.ii.in_offset,
		       f->u.ii.len);

	    mem_ptr += f->u.ii.len;
	    sect_offset += f->u.ii.len;
	}
	efree(f_b->a);
	efree(f_b);
    }

    output_offset = mem_ptr - Shmaddr;

    (void) strncpy(sect_hdr->s_name, _INFO, sizeof(sect_hdr->s_name));
    sect_hdr->s_paddr = sect_hdr->s_vaddr = 0;
    sect_hdr->s_size = output_offset - sect_hdr->s_scnptr;
    sect_hdr->s_relptr = sect_hdr->s_lnnoptr = 0;
    sect_hdr->s_nreloc = sect_hdr->s_nlnno = 0;
    sect_hdr->s_flags = STYP_INFO;

    return output_offset;
} /* write_info_section */
