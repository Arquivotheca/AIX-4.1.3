#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)10	1.31  src/bos/usr/ccs/bin/ld/bind/xcoff.c, cmdld, bos41B, 9504A 12/16/94 08:13:30")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: read_xcoff_symbols
 *		read_section_rlds
 *
 *   STATIC FUNCTIONS:
 *		aligned_reloc_ptr
 *		do_del_csect
 *		process_symbol_entry
 *		process_section_headers
 *		put_rel
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

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

#include "ifiles.h"
#include "insert.h"
#include "util.h"
#include "objects.h"
#include "symbols.h"
#include "stats.h"
#include "dump.h"

/* Static variables */
static int	some_duplicates;	/* For delcsect processing */
static caddr_t	symtab_base;
static ulong	*sect_starts;		/* Used to validate symbol addresses */
static OBJECT	*rld_object;		/* For printing error messages */
static int	rld_sect_num;		/* For printing error messages */
static SECTION	*rld_section;		/* For printing error messages */

/* Static variables used by put_rel that must be set by read_section_rlds */
static ulong	high;
static ulong	prev_addr;
static RLD	*prev_rld;

/* Global variable */
int hash_section;			/* First hash section, if any */

/* Forward declarations */
static void do_del_csect(OBJECT *, CSECT *);
static SYMBOL *process_symbol_entry(int, SYMENT *, AUXENT *, STR *,
				    CSECT *, SYMBOL *, OBJECT *, const int);
static int process_section_headers(OBJECT *, off_t, off_t, int);

/************************************************************************
 * Name: read_xcoff_symbols
 *									*
 * Purpose: Read the symbol table of an XCOFF file.
 *									*
 * Arguments:								*
 *	obj:	Object being read.
 *	base_offset: Offset (in file) to beginning of object to read.
 *	glue_name_ptr:  If not null, we're reading the glue prototype file
 *
 * Pre-conditions:1) The object_info structure must be allocated.
 *		  2) The input file must be open and mapped, if appropriate.
 *		  3) The section headers are part of the input file (that is,
 *			f_nscns is not too big).
 *
 * Returns:	Nothing.  If an error occurs, the o_type field of "obj" is
 *		set to O_T_IGNORE.
 *
 * Side-effects:
 *	1.)  The section_info array is allocated.
 *
 ************************************************************************/
void
read_xcoff_symbols(OBJECT *obj,
		   off_t base_offset,
		   STR *glue_name_ptr)	/* If not null, reading glue file */
{
    char	*id = "read_xcoff_symbols";
    char	c;
    char	*object_file;
    const int	s_flags_init = obj->o_contained_in_type == O_T_ARCHIVE
				? S_ARCHIVE_SYMBOL : 0;
    int		l;
    int		i, j, j1, numaux, num_symbols;
    int		save_toc_anchor;
    int		num_syms = 0;
    int		useful_sections;
    int		cur_csect_index = -1;
    int		srcfiles_found = 0, labels_found = 0, csects_found = 0,
		ers_found=0,commons_found=0;
    off_t	sections_offset;
    off_t	symtab_end;
    AUXENT	*auxsym, temp_aux;
    SYMENT	*cursym, temp_sym;
    CSECT	*cur_csect = NULL,	**prev_csect_ptr_addr;
    CSECT	*cs;
    CSECT_HEAP	h;
    ITEM	*lu_syms;
    STR		*name_ptr, *saved_name_ptr;
    SRCFILE	*cur_srcfile = NULL,	*tmp_cfile;
    SYMBOL	*cur_symbol = NULL;
    SYMBOL	*prev_er = NULL, *prev_symbol = NULL;
    SYMBOL	*prev;

#ifdef READ_FILE
    int		read_file = obj->o_ifile->i_access == I_ACCESS_READ;
#else
    #define read_file 0
#endif

    hash_section = 0;			/* No hash section seen for this OBJ */
    some_duplicates = 0;

    sections_offset = obj->oi_sections_offset;
    obj->oi_flags &= ~OBJECT_NOT_READ;
    if (process_section_headers(obj, base_offset, sections_offset,
				read_file) != 0) {
	if (sect_starts)
	    efree(sect_starts);
	obj->o_type = O_T_IGNORE;
	return;
    }

    num_symbols = obj->oi_num_symbols;

    /* Validate the num_symbols and oi_symtab_offset values.  The symbol table
       cannot start before the end of the section headers, nor can it extend
       longer than the length of the file. */
    if (obj->oi_symtab_offset - base_offset
	< sections_offset + SCNHSZ * obj->oi_num_sections
	|| obj->oi_symtab_offset - base_offset
	> obj->o_size - num_symbols * SYMESZ) {
      bad_sym_tab:
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(BAD_SYM_TAB,
"%1$s: 0711-598 SEVERE ERROR: Object %2$s cannot be processed.\n"
"\tThe XCOFF header contains an invalid value for the offset to the\n"
"\tsymbol table or for the number of symbol table entries."),
		 Main_command_name, get_object_file_name(obj));

	obj->o_type = O_T_IGNORE;
	efree(sect_starts);
	return;
    }

    /* symtab_base is assigned, in case read_file == 0 */
    symtab_base = obj->o_ifile->i_map_addr + obj->oi_symtab_offset;
    cursym = &temp_sym;			/* In case read_file == 0 */

    symtab_end = obj->oi_symtab_offset + num_symbols * SYMESZ;
    if (symtab_end - base_offset == obj->o_size) {
	obj->oi_strtab_len = 0;
	goto set_strtab_base;
    }
    else if (symtab_end - base_offset > obj->o_size - 4) {
	if (symtab_end - base_offset > obj->o_size)
	    goto bad_sym_tab;
	obj->oi_strtab_len = 0;
	goto garbage_at_end;		/* Need at least 4 bytes for length */
    }

    /* Now we have guaranteed that we can read (or access) the length of the
       string table. */
#ifdef READ_FILE
    if (read_file) {
	/* Read string table */
	if (fseek_read(obj->o_ifile, symtab_end, &obj->oi_strtab_len,
		       4) != 0) {
	    obj->o_type = O_T_IGNORE;
	    efree(sect_starts);
	    return;
	}
	if (obj->oi_strtab_len <= 4)	/* Only 0 and 4 make sense, but don't
					   print errors for other values. */
	    obj->oi_strtab_len = 0;
	else {
	    /* Length includes length field itself */
	    if (obj->oi_strtab_len != obj->o_size - (symtab_end - base_offset))
		goto strtab_size_error;

	    obj->oi_strtab_base = alloca(obj->oi_strtab_len + 1);
	    if (safe_fread(obj->oi_strtab_base + 4, obj->oi_strtab_len - 4,
			   obj->o_ifile) != 0) {
		obj->o_type = O_T_IGNORE;
		efree(sect_starts);
		return;
	    }
	    /* Make sure last string is null-terminated, to catch
	       corrupted object files */
	    obj->oi_strtab_base[obj->oi_strtab_len] = '\0';
	}
    }
    else
#endif /* READ_FILE */
    {
	obj->oi_strtab_base = (char *)symtab_end; /* Save offset to strtab */
	obj->oi_strtab_len
	    = *(int *)((long)obj->o_ifile->i_map_addr + obj->oi_strtab_base);
	if (obj->oi_strtab_len <= 4)	/* Only 0 and 4 make sense, but don't
					   print errors for other values. */
	    obj->oi_strtab_len = 0;
	else {
	    if (obj->oi_strtab_len != obj->o_size - (symtab_end-base_offset)) {
	      strtab_size_error:
		if (obj->oi_strtab_len
		    > obj->o_size - (symtab_end - base_offset)) {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_STRING_TAB,
"%1$s: 0711-590 SEVERE ERROR: Object %2$s cannot be processed.\n"
"\tThe length field at the beginning of the string table is invalid."),
			     Main_command_name, get_object_file_name(obj));
		    obj->o_type = O_T_IGNORE;
		    efree(sect_starts);
		    return;
		}
		else {
		  garbage_at_end:
		    bind_err(SAY_NORMAL, RC_WARNING,
			     NLSMSG(GARBAGE_AT_EOF,
"%1$s: 0711-597 WARNING: Object %2$s contains extra bytes at the end."),
			     Main_command_name, get_object_file_name(obj));
		}
	    }
	}
    }
  set_strtab_base:
    set_strtab_base(obj);

    /* Allocate plenty of space for symbol-index->symbol lookup map */
    lu_syms = emalloc((1+(num_symbols+1)/2) * sizeof(*lu_syms), id);
    obj->oi_syms_lookup = lu_syms;

    /* Read symbols */
    for (j = 0; j < num_symbols; j += 1 + numaux) {
#ifdef READ_FILE
	if (read_file) {
	    if (fseek_read(obj->o_ifile, obj->oi_symtab_offset + j * SYMESZ,
			   &temp_sym, SYMESZ) != 0) {
		efree(sect_starts);
		obj->o_type = O_T_IGNORE;
		return;			/* Let's hope nothing is too
					   inconsistent. */
	    }
	    c = temp_sym.n_sclass;
	    numaux = temp_sym.n_numaux;
	}
	else
#endif /* READ_FILE */
	{
	    c = ((SYMENT *)(symtab_base + j*SYMESZ))->n_sclass;
	    numaux = ((SYMENT *)(symtab_base + j*SYMESZ))->n_numaux;
	}

	switch(c) {
	  case C_FILE:
	    if (!read_file)
		cursym = aligned_sym_ptr(symtab_base, j);

	    /* If there are any auxiliary entries, the first one must be the
	       filename. Otherwise, the symbol name is the file name. */
	    if (numaux > 0) {
#ifdef DEBUG
		if (Switches.verbose) {
		    if (strcmp(".file", cursym->n_name) != 0) {
			bind_err(SAY_NO_NLS, RC_WARNING,
 "%1$s: WARNING: Symbol entry %2$d in object %3$s:\n"
 "\tThe name of a C_FILE entry with auxiliary entries is not .file",
				 Main_command_name,
				 j, get_object_file_name(obj));
		    }
		}
#endif
		if (j + numaux >= num_symbols) {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(XCOFF_END_SYMTAB,
    "%1$s: 0711-589 SEVERE ERROR: Symbol table entry %2$d in object %3$s:\n"
    "\tAuxiliary entries for the symbol are missing."),
			     Main_command_name, j, get_object_file_name(obj));
		    break;
		}
#ifdef READ_FILE
		if (read_file) {
		    /* Seek not necessary */
		    if (safe_fread(&temp_aux, AUXESZ, obj->o_ifile) != 0)
			continue;
		    auxsym = &temp_aux;
		}
		else
#endif /* READ_FILE */
		    auxsym = aligned_aux_ptr(symtab_base, j + 1);

		name_ptr = get_aux_name(obj, auxsym, 0, j+1);
		/* When allocating arrays for the output string table, we may
		   have to add additional strings for the remaining
		   aux entries of the C_FILE entry. */
		Bind_state.num_potential_C_FILE_strings += numaux - 1;
	    }
	    else
		name_ptr = get_sym_name(obj, cursym, j);

	    /* Add a new entry to the list of srcfiles in
	       this XCOFF object.
	       The head of the list is: obj->oi_srcfiles.
	       The tail is:		cur_srcfile.
	       */
	    tmp_cfile = get_init_SRCFILE(obj, name_ptr);
#ifdef _CPUTYPE_FEATURE
	    if (cursym->n_type == 0)	/* Old style */
		tmp_cfile->sf_cputype = TCPU_PWR; /* Default is POWER. */
	    else
		tmp_cfile->sf_cputype = cursym->n_cputype;
#endif
	    if (name_ptr == &NULL_STR) {
		/* File name is null. */
		name_ptr = putstring(get_object_file_name(obj));
		tmp_cfile->sf_name = name_ptr;
		if (numaux == 0)
		    bind_err(SAY_NORMAL, RC_WARNING,
			     NLSMSG(VALIDATE_NULL_FILE_NAME,
"%1$s: 0711-571 WARNING: Symbol table entry %2$d in object %3$s is invalid.\n"
	"\tFilename in C_FILE symbol table entry is null."),
			     Main_command_name, j, name_ptr->name);
		else
		    bind_err(SAY_NORMAL, RC_WARNING,
			     NLSMSG(VALIDATE_NULL_AUX_NAME,
"%1$s: 0711-570 WARNING: Symbol table entry %2$d in object %3$s is invalid.\n"
	"\tFilename in C_FILE auxiliary symbol is null."),
			     Main_command_name, j+1, name_ptr->name);
	    }
	    else
		tmp_cfile->sf_inpndx = j;

	    if (cur_srcfile)
		cur_srcfile->sf_next = tmp_cfile;
	    else
		obj->oi_srcfiles = tmp_cfile;
	    cur_srcfile = tmp_cfile;

	    /* Current list of CSECTS in this SRCFILE is null.*/
	    cur_csect = NULL;
	    /* Save address of tail pointer for list of CSECTS */
	    prev_csect_ptr_addr = &cur_srcfile->sf_csect;

	    DEBUG_MSG(XCOFF_DEBUG | DEBUG_LONG,
		      (SAY_NO_NLS,
		       "Creating source-file entry for %s",
		       name_ptr->name));

	    srcfiles_found++;
	    break;
	    /*==================== End of C_FILE processing =============== */

	  case C_EXT:
	  case C_HIDEXT:
	    lu_syms[++num_syms].item_sym_index = j; /* key */

	    /* An auxiliary entry must exist. */
	    if (numaux < 1) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(INSRT_SYMTABCOR,
	"%1$s: 0711-562 SEVERE ERROR: Symbol table entry %2$d in object %3$s:\n"
	"\tAn auxiliary symbol table entry is required for a\n"
	"\tC_EXT or C_HIDEXT entry."),
			 Main_command_name, j, get_object_file_name(obj));
		lu_syms[num_syms].item_symbol = NULL;
		break;
	    }
	    if (j + numaux >= num_symbols) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(XCOFF_END_SYMTAB,
	"%1$s: 0711-589 SEVERE ERROR: Symbol table entry %2$d in object %3$s:\n"
	"\tAuxiliary entries for the symbol are missing."),
			 Main_command_name, j, get_object_file_name(obj));
		lu_syms[num_syms].item_symbol = NULL;
		break;
	    }

	    /* If no symbol table file name found, generate a dummy one. */
	    if (!cur_srcfile) {
		object_file = get_object_file_name(obj);
		l = strlen(object_file);
		/* Change the name of a *.o to *.s to reduce the possibility
		   that dbx will try to read the real *.o as a source file. */
		if (object_file[l-2] == '.'
		    && object_file[l-1] == 'o') {
		    object_file = save_string(object_file, l+1);
		    object_file[l-1] = 's';
		    object_file[l] = '\0';
		}
		cur_srcfile =
		    get_init_SRCFILE(obj, putstring(object_file));
		/* This dummy entry will cover the initial
		   symbol table entries up to the current CSECT */
		obj->oi_srcfiles = cur_srcfile;
		/* Current list of CSECTS is null.*/
		cur_csect = NULL;
		/* Save address of tail pointer for CSECTS */
		prev_csect_ptr_addr = &cur_srcfile->sf_csect;

		DEBUG_MSG(XCOFF_DEBUG | DEBUG_LONG,
			  (SAY_NO_NLS, "Creating dummy srcfile for %s",
			   get_object_file_name(obj)));
	    }

#ifdef READ_FILE
	    if (read_file) {
		if (numaux > 1)
		    if (safe_fseek(obj->o_ifile, (numaux-1) * SYMESZ, SEEK_CUR)
			!= 0) {
			efree(sect_starts);
			obj->o_type = O_T_IGNORE;
			return;
		    }
		if (safe_fread(&temp_aux, AUXESZ, obj->o_ifile) != 0) {
		    efree(sect_starts);
		    obj->o_type = O_T_IGNORE;
		    return;
		}
		auxsym = &temp_aux;
	    }
	    else
#endif /* READ_FILE */
	    {
		cursym = aligned_sym_ptr(symtab_base, j);
		auxsym = aligned_aux_ptr(symtab_base, j+numaux);
	    }

	    name_ptr = get_sym_name(obj, cursym, j);

	    if (auxsym->x_csect.x_smclas == XMC_TC0) {
		if (Switches.verbose) {
		    if (strcmp(name_ptr->name, TOC_NAME) != 0)
			bind_err(SAY_NORMAL, RC_WARNING,
				 NLSMSG(VALIDATE_BAD_TOC0,
 "%1$s: 0711-572 WARNING: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tThe TOC anchor is not named %5$s."),
				 Main_command_name, name_ptr->name,
				 j, get_object_file_name(obj), TOC_NAME);

		    if (/* || cursym->n_value == ARBITRARY */
			/* cursym->n_scnum <= 0 CHECKED BELOW */
			/* || cursym->n_type == ?? */

			cursym->n_sclass != C_HIDEXT

			/* || cursym->n_numaux== 0 already checked*/

			|| auxsym->x_csect.x_scnlen != 0

			/*|| auxsym->x_csect.x_parmhash != 0*/
			/* NOTE:  Some libraries (libxlf.a) contain code from
			   the PL8 compiler (or some variant), which generates
			   hash strings for hidden symbols, including the
			   TOC anchor */
			/* || auxsym->x_csect.x_snhash != 0*/

			|| (auxsym->x_csect.x_smtyp & 7) != XTY_SD
			|| auxsym->x_csect.x_stab != 0
			|| auxsym->x_csect.x_snstab != 0)
			bind_err(SAY_NORMAL, RC_WARNING,
				 NLSMSG(VALIDATE_TC0,
 "%1$s: 0711-575 WARNING: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tThe symbol table entry for the TOC anchor has bad values."),
				 Main_command_name, name_ptr->name,
				 j, get_object_file_name(obj));
		}

		if (cursym->n_scnum <= 0
		    || cursym->n_scnum > obj->oi_num_sections) {
		    bind_err(SAY_NORMAL, RC_ERROR,
			     NLSMSG(BAD_SYMBOL_SCNUM,
  "%1$s: 0711-563 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
	"\tThe n_scnum field contains %5$d.\n"
	"\tThe section number must be between 1 and %6$d."),
			     Main_command_name, name_ptr->name,
			     j, get_object_file_name(obj),
			     cursym->n_scnum, obj->oi_num_sections);
		    lu_syms[num_syms].item_symbol = NULL;
		    continue;
		}
		else if (!(obj->oi_section_info[cursym->n_scnum-1].sect_flags
			   & SECT_RAWDATA)) {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_TOC_SECTION,
  "%1$s: 0711-592 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
	"\tThe TOC anchor is not contained in a .text or .data section."),
			     Main_command_name, name_ptr->name,
			     j, get_object_file_name(obj));
		    lu_syms[num_syms].item_symbol = NULL;
		    continue;
		}
		else if (obj->oi_section_info[cursym->n_scnum-1].l_toc_anchor
			 != NULL) {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_TOCS,
    "%1$s: 0711-553 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
    "\tA TOC anchor for section %5$d has already been seen.\n"
    "\tThe symbol is being ignored."),
			     Main_command_name, name_ptr->name,
			     j, get_object_file_name(obj),
			     cursym->n_scnum);
		    lu_syms[num_syms].item_symbol = NULL; /* default value */
		    continue;
		}

		/* Toc anchors are always named TOC--actual name is ignored */
		name_ptr = saved_name_ptr = putstring(TOC_NAME);
		name_ptr->flags |= STR_ISSYMBOL;
		save_toc_anchor = 1;

		DEBUG_MSG(SYMBOLS_DEBUG | DEBUG_LONG,
			  (SAY_NO_NLS | SAY_NO_NL, "Found TOC anchor"));
	    }
	    else {
		save_toc_anchor = 0;
		if ((auxsym->x_csect.x_smtyp & 7) != XTY_ER
		    && (cursym->n_scnum <= 0
			|| cursym->n_scnum > obj->oi_num_sections)) {
		    bind_err(SAY_NORMAL, RC_ERROR,
			     NLSMSG(BAD_SYMBOL_SCNUM,
	    "%1$s: 0711-563 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
	"\tThe n_scnum field contains %5$d.\n"
	"\tThe section number must be between 1 and %6$d."),
			     Main_command_name, name_ptr->name,
			     j,
			     get_object_file_name(obj), cursym->n_scnum,
			     obj->oi_num_sections);
		    lu_syms[num_syms].item_symbol = NULL;
		    continue;
		}
		saved_name_ptr = name_ptr;
		saved_name_ptr->flags |= STR_ISSYMBOL;
	    }
	    if (glue_name_ptr)
		name_ptr = glue_name_ptr;
	    else
		name_ptr = saved_name_ptr;
	    DEBUG_MSG(SYMBOLS_DEBUG | DEBUG_LONG,
		      (SAY_NO_NLS | SAY_NO_NL,
		       "Found '%s'", saved_name_ptr->name));

	    cur_symbol = NULL;		/* Initialize in case of error */
	    switch(auxsym->x_csect.x_smtyp & 7) {
/* Define shorthand notation */
#define SI obj->oi_section_info[cursym->n_scnum-1]
	      case XTY_CM:
		if (!((SI.sect_type == STYP_BSS
		       && (auxsym->x_csect.x_smclas == XMC_BS
			   || auxsym->x_csect.x_smclas == XMC_UC
			   || auxsym->x_csect.x_smclas == XMC_RW))
		      || (SI.sect_type == STYP_DATA
			  && auxsym->x_csect.x_smclas == XMC_TD))) {
		    bind_err(SAY_NORMAL, RC_ERROR,
			     NLSMSG(VALIDATE_BAD_CM,
    "%1$s: 0711-573 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
    "\tA symbol with type XTY_CM must be in a .bss section and have a\n"
    "\tstorage-mapping class of XMC_BS, XMC_UC, or XMC_RW; or it must\n"
    "\tbe in a .data section and have a storage-mapping class of XMC_TD."),
			     Main_command_name, saved_name_ptr->name,
			     j, get_object_file_name(obj));
		    break;
		}

		commons_found++;
		goto add_csect;

	      case XTY_SD:
		/* Make sure symbols of this type are in a .text or .data
		   section. */
		if (!(SI.sect_flags & SECT_RAWDATA)) {
		    bind_err(SAY_NORMAL, RC_ERROR,
			     NLSMSG(BAD_SECTION2,
    "%1$s: 0711-591 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
    "\tA symbol with type XTY_SD must be in a .text or .data section."),
			     Main_command_name, saved_name_ptr->name,
			     j, get_object_file_name(obj));
		    break;
		}

	      add_csect:
		cur_csect = create_CSECT(cursym, auxsym);
		/* Make sure the first and last bytes of the csect are within
		   the section.  Using the unsigned cast saves a comparison and
		   also allows negative lengths to be detected.

		   If the halt level is set high enough, we could continue,
		   so we must fix up the symbol addresses to avoid possible
		   references outside a mapped file. */
		if ((unsigned)(cur_csect->c_addr
			       - sect_starts[cur_csect->c_secnum-1])
		    > SI.sect_size
		    || (unsigned)(cur_csect->c_addr + auxsym->x_csect.x_scnlen
				  - sect_starts[cur_csect->c_secnum-1])
		    > SI.sect_size) {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_SYM_ADDR,
    "%1$s: 0711-583 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
    "\tThe symbol is not entirely contained within its section."),
			     Main_command_name, saved_name_ptr->name,
			     j, get_object_file_name(obj));

		    /* Fix up csect fields. */
		    if ((unsigned)(cur_csect->c_addr
				   - sect_starts[cur_csect->c_secnum-1])
			> SI.sect_size)
			cur_csect->c_addr = sect_starts[cur_csect->c_secnum-1];
		    if ((unsigned)(cur_csect->c_addr + auxsym->x_csect.x_scnlen
				   - sect_starts[cur_csect->c_secnum-1])
			> SI.sect_size)
			cur_csect->c_len
			    = sect_starts[cur_csect->c_secnum-1] + SI.sect_size
				- cur_csect->c_addr;
		}
		SI.csect_count++;
		cur_csect->c_srcfile = cur_srcfile;

		/* Update list of csects in SRCFILE */
		*prev_csect_ptr_addr = cur_csect;
		prev_csect_ptr_addr = &cur_csect->c_next;
		cur_csect_index = j;

#ifdef DEBUG
		if ((bind_debug & (SYMBOLS_DEBUG | DEBUG_LONG))
		    == (SYMBOLS_DEBUG | DEBUG_LONG)) {
		    say(SAY_NO_NLS | SAY_NO_NL, "\tCreated csect");
		    if (saved_name_ptr->name[0] == '\0')
			if (c == C_EXT) {
			    /* Shouldn't happen */
			    say(SAY_NO_NLS, " without a label");
			}
			else
			    say(SAY_NO_NLS, " with an empty label.");
		    else
			say(SAY_NO_NLS, " with a label.");
		}
#endif
		cur_symbol = process_symbol_entry(j, cursym, auxsym, name_ptr,
						  cur_csect, NULL, obj,
						  s_flags_init);
		prev_symbol = cur_symbol;
		if (save_toc_anchor)
		    SI.l_toc_anchor = cur_symbol;
#undef SI
		cur_symbol->s_flags |= S_PRIMARY_LABEL;

		if (glue_name_ptr)
		    goto set_glue_ptr;

		break;

	      case XTY_LD:
		labels_found++;
		if (auxsym->x_csect.x_scnlen != cur_csect_index) {
		    /* Must search for current csect by cur_csect_index.
		       The csect must be within the current SRCFILE. */
		    for (cur_csect = cur_srcfile->sf_csect;
			 cur_csect;
			 cur_csect = cur_csect->c_next)
			if (cur_csect->c_symbol.s_inpndx
			    == auxsym->x_csect.x_scnlen)
			    break;

		    if (cur_csect == NULL) {
			cur_csect_index = -1;
			bind_err(SAY_NORMAL, RC_SEVERE,
				 NLSMSG(BAD_LABEL,
 "%1$s: 0711-593 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tThe symbol refers to a csect with symbol number %5$d, which was not\n"
 "\tfound. The new symbol cannot be associated with a csect and\n"
 "\tis being ignored."),
				 Main_command_name, saved_name_ptr->name,
				 j, get_object_file_name(obj),
				 auxsym->x_csect.x_scnlen);
			break;
		    }

		    cur_csect_index = cur_csect->c_symbol.s_inpndx;
		    /* Find last symbol in the csect we found */
		    for (prev_symbol = &cur_csect->c_symbol;
			 prev_symbol->s_next_in_csect;
			 prev_symbol = prev_symbol->s_next_in_csect)
			/* skip */;
		}
		if (cur_csect->c_symbol.s_smtype != XTY_SD
		    || cur_csect->c_secnum != cursym->n_scnum
		    || cur_csect->c_symbol.s_smclass != auxsym->x_csect.x_smclas
		    || cursym->n_value < cur_csect->c_addr
		    || cursym->n_value > cur_csect->c_addr + cur_csect->c_len) {
		    bind_err(SAY_NORMAL, RC_ERROR,
			     NLSMSG(VALIDATE_BAD_LD,
	    "%1$s: 0711-574 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
	    "\tXTY_LD symbol contains invalid values."),
			     Main_command_name, saved_name_ptr->name,
			     j, get_object_file_name(obj));
		    break;
		}

		cur_symbol = process_symbol_entry(j, cursym, auxsym, name_ptr,
						  cur_csect, prev_symbol, obj,
						  s_flags_init);
		prev_symbol = cur_symbol;

		if (glue_name_ptr) {
		  set_glue_ptr:
		    cur_symbol->s_name = saved_name_ptr;
		    if (c == C_EXT) {
			if (glue_ext_symbol) {
			    bind_err(SAY_NORMAL, RC_ERROR,
				     NLSMSG(GLUE_TOO_MANY_NAMES,
   "%1$s: 0711-595 ERROR: Only one external name is allowed in a glink file.\n"
   "\tThe glink file %2$s is invalid."),
				     Main_command_name,
				     get_object_file_name(obj));
			    glue_ext_symbol = NULL;
			    efree(sect_starts);
			    return;
			}
			glue_ext_symbol = cur_symbol;
		    }
		}
		break;

	      case XTY_ER:
		ers_found++;
		if (Switches.verbose) {
		    if (saved_name_ptr->name[0] == '\0'
			|| cursym->n_sclass == C_HIDEXT
			|| (cursym->n_value != 0
			    && auxsym->x_csect.x_smclas != XMC_XO
			    && auxsym->x_csect.x_smclas != XMC_TD)
			|| cursym->n_scnum != N_UNDEF
			|| auxsym->x_csect.x_scnlen != 0
			|| (auxsym->x_csect.x_smclas != XMC_PR &&
			    auxsym->x_csect.x_smclas != XMC_XO &&
			    auxsym->x_csect.x_smclas != XMC_DS &&
			    auxsym->x_csect.x_smclas != XMC_RW &&
			    auxsym->x_csect.x_smclas != XMC_SV &&
			    auxsym->x_csect.x_smclas != XMC_TD &&
			    auxsym->x_csect.x_smclas != XMC_UA)) {
			/* The message should really list n_sclass and
			   x_smclas separately, but to avoid changing the
			   message, the symbolic names are concatenated and
			   printed together. */
			char buf[30];
			sprintf(buf, "%s/%s", get_sclass(cursym->n_sclass),
				get_smclass(auxsym->x_csect.x_smclas));
			bind_err(SAY_NORMAL, RC_ERROR,
				 NLSMSG(VALIDATE_BAD_SYMBOL,
	"%1$s: 0711-577 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
	"\tExternal reference has invalid fields:\n"
	"\t\tn_sclass=%5$s; n_smtype=%6$s; n_value=%7$d; n_scnum=%8$d"),
				 Main_command_name,
				 saved_name_ptr->name,
				 j,
				 get_object_file_name(obj),
				 buf,
				 get_smtype(XTY_ER),
				 cursym->n_value,
				 cursym->n_scnum);
		    }
		}

		cur_symbol = create_er_SYMBOL(name_ptr, j, obj, auxsym);
		/* Symbol address should be 0, except for XMC_XO symbols,
		   but the address is used in any case. In particular, this
		   allows rebinding of objects where an imported XO-symbol
		   was referenced as a data-in-TOC symbol. */
		cur_symbol->s_addr = cursym->n_value;
		cur_symbol->er_next_in_object = NULL;
		if (prev_er == NULL)
		    obj->oi_ext_refs = cur_symbol;
		else
		    prev_er->er_next_in_object = cur_symbol;
		prev_er = cur_symbol;

		if (glue_name_ptr) {
		    if (cur_symbol->er_typechk)
			bind_err(SAY_NORMAL, RC_WARNING,
				 NLSMSG(GLUE_TYPECHK,
		"%1$s: 0711-587 WARNING: Object %2$s:\n"
		"\tType-checking information from a glink file is ignored."),
				 Main_command_name,
				 get_object_file_name(obj));
		    cur_symbol->er_typechk = NULL;

		    /* Save real name */
		    cur_symbol->s_name = saved_name_ptr;
		}
		break;
	    } /* switch x_xmtyp */
	    lu_syms[num_syms].item_symbol = cur_symbol; /* value */
	    break;
	  default:		/* Other symbol classes not important */
	    break;
	} /* switch(c = n_sclass) */
    } /* for all symbols in OBJECT file */

    efree(sect_starts);

    lu_syms[0].item_key = num_syms;

    useful_sections = 0;
    csects_found = 0;
    /* Count all the csects we found. */
    for (i = 0; i < obj->oi_num_sections; i++) {
	if ((j1 = obj->oi_section_info[i].csect_count) > 0) {
	    DEBUG_MSG(XCOFF_DEBUG | DEBUG_LONG,
		      (SAY_NO_NLS,"Found %d symbols in section %d", j1, i+1));
	    if (obj->oi_section_info[i].sect_type != STYP_BSS) {
		useful_sections++;
		csects_found += j1;
	    }
	}
    }

    h = emalloc(sizeof(*h) * (csects_found + useful_sections), id);
    for (i = 0; i < obj->oi_num_sections; i++) {
	if ((j1 = obj->oi_section_info[i].csect_count) > 0
	    && obj->oi_section_info[i].sect_type != STYP_BSS) {
	    h[0].heap_index = 0;
	    obj->oi_section_info[i].l_csect_heap = h;
	    h += 1 + j1;
	}
    }

#define Heap obj->oi_section_info[cs->c_secnum-1].l_csect_heap
    for (cur_srcfile = obj->oi_srcfiles;
	 cur_srcfile;
	 cur_srcfile = cur_srcfile->sf_next)
	for (cs = cur_srcfile->sf_csect; cs; cs = cs->c_next) {
	    if (obj->oi_section_info[cs->c_secnum-1].sect_type != STYP_BSS)
		Heap[++Heap[0].heap_index].csect = cs;
	}
#undef Heap

    /* We call makeheap if there are any RLDs for the section.  Otherwise, we
       won't need the heap, unless we need to save with reorder==BY_ADDRESS. */
    for (i = 0; i < obj->oi_num_sections; i++)
	if (obj->oi_section_info[i].l_reloc_count > 0)
	    makeheap(obj->oi_section_info[i].l_csect_heap);

    if (Switches.dbg_opt0) {
	say(SAY_NORMAL, NLSMSG(FOUND_FILE, "%1$s: Found %2$d C_FILEs."),
	    Command_name, srcfiles_found);
	say(SAY_NORMAL, NLSMSG(FOUND_SD, "%1$s: Found %2$d XTY_SDs."),
	    Command_name, csects_found);
	say(SAY_NORMAL, NLSMSG(FOUND_CM, "%1$s: Found %2$d XTY_CMs."),
	    Command_name, commons_found);
	say(SAY_NORMAL, NLSMSG(FOUND_LD, "%1$s: Found %2$d XTY_LDs."),
	    Command_name, labels_found);
	say(SAY_NORMAL, NLSMSG(FOUND_ER, "%1$s: Found %2$d XTY_ERs."),
	    Command_name, ers_found);
    }

    if (Switches.del_csect && some_duplicates) {
	/* Now check for any duplicate symbols */
	/* Search all symbols in CSECTS for duplicates. */
	for (tmp_cfile = obj->oi_srcfiles;
	     tmp_cfile;
	     tmp_cfile = tmp_cfile->sf_next) {
	    for (cs = tmp_cfile->sf_csect; cs; cs = cs->c_next)
		do_del_csect(obj, cs);
	}
    }
} /* read_xcoff_symbols */
/************************************************************************
 * Name: process_section_headers
 *									*
 * Purpose:	Read the section headers from an XCOFF object
 *									*
 * Arguments:	obj:		The OBJECT to read
 *		base_offset:	Offset (within the IFILE to the beginning
 *				of the object.)
 *		saved_sections_offset:
 *		read_file	if 1, read the file.  Otherwise, it is mapped.
 *
 * Returns: 0 for no errors, non-zero otherwise
 *
 ************************************************************************/
static int
process_section_headers(OBJECT *obj,
			off_t base_offset,
			off_t saved_sections_offset,
			int read_file)
{
    char	*id = "process_section_headers";
    char	*sect_name;
    uint16	object_sect_flag, old_sections_seen;
    uint16	*object_sect_no_ptr;
    short	p_index;
    int		size_invalid;
    int		i, primary_sect_no;
    int		overflow_needed = 0;
    int		obj_ok = 0;
    int		rc = 0;
    SCNHDR	*scn_ptr, *primary_scn_ptr;

#ifdef READ_FILE
    if (read_file) {
	scn_ptr = alloca(SCNHSZ * obj->oi_num_sections);
	if (fseek_read(obj->o_ifile,
		       base_offset + obj->oi_sections_offset, scn_ptr,
		       SCNHSZ * obj->oi_num_sections) != 0) {
	    obj->o_type = O_T_IGNORE;
	    sect_starts = NULL;		/* Non-null value could still exist
					   from previous call.  Make sure the
					   caller doesn't free the same block
					   of memory twice. */
	    return -1;
	}
    }
    else
#endif /* READ_FILE */
	scn_ptr = (SCNHDR *)(obj->o_ifile->i_map_addr
			     + base_offset + obj->oi_sections_offset);

    /* Allocate space for section headers information */
    obj->oi_section_info = emalloc(obj->oi_num_sections
				   * sizeof(*obj->oi_section_info), id);

    /* Allocate temporary space to save the beginning addresses of sections. */
    sect_starts = emalloc(obj->oi_num_sections * sizeof(*sect_starts), id);

/* Define shorthand notation */
#define SI obj->oi_section_info[i]

    /* Initialize--SECT_OVERFLOW_NEEDED in sect_flags will be used to detect
       sections that should have an overflow section but for which an overflow
       section is not found. */
    for (i = 0; i < obj->oi_num_sections; i++) {
	SI.sect_flags = 0;
	SI.l_reloc_count = 0;
	SI.csect_count = 0;
    }

    /* Examine section headers */
    for (i = 0; i < obj->oi_num_sections; i++, scn_ptr++) {
	SI.sect_type = scn_ptr->s_flags & 0xFFFF;

	/* Save old value of Bind_state.sections_seen */
	old_sections_seen = Bind_state.sections_seen;
	Bind_state.sections_seen |= SI.sect_type;

	/* Validate s_scnptr and s_size fields.
	   This is needed for most sections. */
	size_invalid
	    = scn_ptr->s_scnptr < (saved_sections_offset
				   + SCNHSZ * obj->oi_num_sections)
		|| scn_ptr->s_scnptr > (long)(obj->o_size - scn_ptr->s_size);

	/* Look for useful sections. */
	switch(SI.sect_type) {
	  case STYP_REG:
	  case STYP_PAD:
	  case STYP_LOADER:
	    break;

	  case STYP_TYPCHK:
	    /* Some compilers and linkers put out 0-length sections */
	    if (scn_ptr->s_size == 0) {
#ifdef DEBUG
		if (Switches.verbose)
		    bind_err(SAY_NO_NLS, RC_WARNING,
			     "%1$s: WARNING: Object %2$s contains\n"
			     "\ta section header for an empty %3$s section.",
			     Main_command_name,
			     get_object_file_name(obj),
			     ".typchk");
#endif
		break;
	    }

	    if (size_invalid) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(SECTION_MISPLACED,
 "%1$s: 0711-599 SEVERE ERROR: Section header %2$d (%3$s) in object %4$s:\n"
 "\tFields in the header are invalid: s_scnptr = %5$d; s_size = %6$d"),
			 Main_command_name, i+1, ".typchk",
			 get_object_file_name(obj),
			 scn_ptr->s_scnptr,
			 scn_ptr->s_size);
		obj->o_type = O_T_IGNORE;
		rc = -1;
		break;
	    }

#ifdef READ_FILE
	    if (read_file) {
		SI.u.sect_base = emalloc(scn_ptr->s_size, id);
		if (fseek_read(obj->o_ifile, base_offset + scn_ptr->s_scnptr,
			       SI.u.sect_base,
			       scn_ptr->s_size) != 0) {
		    efree(SI.u.sect_base);
		    SI.u.sect_base = NULL;
		    break;
		}
	    }
	    else
#endif /* READ_FILE */
	    {
		/* Save memory address */
		SI.u.sect_base = obj->o_ifile->i_map_addr
		    + scn_ptr->s_scnptr + base_offset;
	    }
	    SI.sect_size = scn_ptr->s_size;
	    obj->oi_flags |= OBJECT_HAS_TYPECHK;

	    /* Save the section number of the first TYPCHK section.  Some
	       versions of the xlc compiler didn't assign the x_snhash field
	       of a symbol table entry, so if we see x_parmhash != 0, and
	       x_snhash == 0, we use "hash_section" as the proper TYPCHK
	       section. */
	    if (hash_section == 0)
		hash_section = i + 1;	/* Save 1-based index. */
	    break;

	  case STYP_INFO:
	    /* Some compilers and linkers put out 0-length sections */
	    if (scn_ptr->s_size == 0) {
#ifdef DEBUG
		if (Switches.verbose)
		    bind_err(SAY_NO_NLS, RC_WARNING,
			     "%1$s: WARNING: Object %2$s contains\n"
			     "\ta section header for an empty %3$s section.",
			     Main_command_name,
			     get_object_file_name(obj),
			     _INFO);
#endif
	    }
	    else if (size_invalid) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(SECTION_MISPLACED,
 "%1$s: 0711-599 SEVERE ERROR: Section header %2$d (%3$s) in object %4$s:\n"
 "\tFields in the header are invalid: s_scnptr = %5$d; s_size = %6$d"),
			 Main_command_name, i+1,
			 sect_name,
			 get_object_file_name(obj),
			 scn_ptr->s_scnptr,
			 scn_ptr->s_size);
		    obj->o_type = O_T_IGNORE;
		    rc = -1;
	    }
	    else {
		obj->oi_flags |= OBJECT_HAS_INFO;
		SI.sect_size = scn_ptr->s_size;
		SI.u.sect_offset = base_offset + scn_ptr->s_scnptr;
	    }
	    break;

	  case STYP_EXCEPT:
	    sect_name = _EXCEPT;
	    object_sect_flag = OBJECT_HAS_EXCEPT;
	    object_sect_no_ptr = &obj->oi_except_sect_i;
	    goto check_section;

	  case STYP_DEBUG:
	    sect_name = _DEBUG;
	    object_sect_flag = OBJECT_HAS_DEBUG;
	    object_sect_no_ptr = &obj->oi_debug_sect_i;

	  check_section:
	    /* Some compilers and linkers put out 0-length sections */
	    if (scn_ptr->s_size == 0) {
#ifdef DEBUG
		if (Switches.verbose)
		    bind_err(SAY_NO_NLS, RC_WARNING,
			     "%1$s: WARNING: Object %2$s contains\n"
			     "\ta section header for an empty %3$s section.",
			     Main_command_name,
			     get_object_file_name(obj),
			     sect_name);
#endif /*DEBUG*/
	    }
	    else if (obj->oi_flags & object_sect_flag)
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(MULTIPLE_SECTIONS,
 "%1$s: 0711-586 WARNING: Object %2$s contains multiple %3$s sections.\n"
 "\tThe additional sections are being ignored."),
			 Main_command_name,
			 get_object_file_name(obj),
			 sect_name);
	    else if (size_invalid) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(SECTION_MISPLACED,
 "%1$s: 0711-599 SEVERE ERROR: Section header %2$d (%3$s) in object %4$s:\n"
 "\tFields in the header are invalid: s_scnptr = %5$d; s_size = %6$d"),
			 Main_command_name, i+1,
			 sect_name,
			 get_object_file_name(obj),
			 scn_ptr->s_scnptr,
			 scn_ptr->s_size);
		    obj->o_type = O_T_IGNORE;
		    rc = -1;
	    }
	    else {
		obj->oi_flags |= object_sect_flag;
		*object_sect_no_ptr = i; /* Save 0-based index */
		SI.sect_size = scn_ptr->s_size;
		SI.u.sect_offset = base_offset + scn_ptr->s_scnptr;
	    }
	    break;

	  case STYP_BSS:
	    obj_ok = 1;
	    SI.sect_size = scn_ptr->s_size;
	    SI.sect_flags |= SECT_CODE;
	    sect_starts[i] = scn_ptr->s_paddr;

	    if (scn_ptr->s_nreloc != 0 || scn_ptr->s_nlnno != 0)
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(VALIDATE_BAD_BSS_RELOC,
	"%1$s: 0711-576 WARNING: Section %2$d in object %3$s:\n"
	"\tA .bss section may not have relocation or line number entries.\n"
	"\tThe entries are being ignored."),
			 Main_command_name, i + 1,
			 get_object_file_name(obj));
	    break;

	  case STYP_TEXT:		/* text section */
	    sect_name = _TEXT;
	    goto handle_text_or_data;

	  case STYP_DATA:		/* data section */
	    sect_name = _DATA;

	  handle_text_or_data:
	    if (scn_ptr->s_size != 0 && size_invalid) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(SECTION_MISPLACED,
 "%1$s: 0711-599 SEVERE ERROR: Section header %2$d (%3$s) in object %4$s:\n"
 "\tFields in the header are invalid: s_scnptr = %5$d; s_size = %6$d"),
			 Main_command_name, i+1,
			 sect_name,
			 get_object_file_name(obj),
			 scn_ptr->s_scnptr,
			 scn_ptr->s_size);
		obj->o_type = O_T_IGNORE;
		rc = -1;
		break;
	    }

	    if (scn_ptr->s_nreloc == 65535 && scn_ptr->s_nlnno == 65535) {
		/* If we have already seen an overflow section header for
		   the current section header, we don't have to do anything
		   else. */
		if (!(SI.sect_flags & SECT_OVERFLOW_FOUND)) {
		    ++overflow_needed;
		    SI.sect_flags |= SECT_OVERFLOW_NEEDED;
		}
		break;
	    }
	    obj_ok = 1;

	    SI.l_reloc_count = scn_ptr->s_nreloc;
	    SI.l_linenum_last = scn_ptr->s_lnnoptr + scn_ptr->s_nlnno * LINESZ;
	    primary_scn_ptr = scn_ptr;
	    primary_sect_no = i;
	    goto lsect_common;
	    /*break;*/
#undef SI

	  case STYP_OVRFLO:		/* Overflow section */
	    obj_ok = 1;

	    /* Get index of primary section */
	    p_index = scn_ptr->s_nreloc;
	    primary_sect_no = p_index - 1; /* Get 0-based index */

	    /* Validate p_index */
	    if (p_index < 1 || p_index > obj->oi_num_sections)
		goto bad_pindex;
	    primary_scn_ptr = &scn_ptr[primary_sect_no - i];

	    if ((primary_scn_ptr->s_flags & 0xFFFF) != STYP_DATA
		&& (primary_scn_ptr->s_flags & 0xFFFF) != STYP_TEXT) {
	      bad_pindex:
		bind_err(SAY_NORMAL, RC_ERROR,
			 NLSMSG(BAD_OVERFLOW,
 "%1$s: 0711-594 SEVERE ERROR: Overflow section header %2$d in object %3$s:\n"
 "\tThe section header does not refer to a .text or .data section header."),
			 Main_command_name, i+1,
			 get_object_file_name(obj));
		obj->o_type = O_T_IGNORE;
		rc = -1;
		break;
	    }

	    if (obj->oi_section_info[primary_sect_no].sect_flags
		& SECT_OVERFLOW_NEEDED)
		--overflow_needed;

	    if (obj->oi_section_info[primary_sect_no].sect_flags
		& SECT_OVERFLOW_FOUND) {
		bind_err(SAY_NORMAL, RC_ERROR,
			 NLSMSG(EXTRA_OVERFLOW,
 "%1$s: 0711-584 WARNING: Section header %2$d in object %3$s:\n"
 "\tAn overflow section header has already been found for section %4$d.\n"
 "\tThe extra overflow header is being ignored."),
			 Main_command_name, p_index,
			 get_object_file_name(obj), i + 1);
		break;
	    }
	    else
		obj->oi_section_info[primary_sect_no].sect_flags
		    |= SECT_OVERFLOW_FOUND;

	    if (Switches.verbose)
		if (p_index != scn_ptr->s_nlnno
		    || scn_ptr->s_relptr != primary_scn_ptr->s_relptr
		    || scn_ptr->s_lnnoptr != primary_scn_ptr->s_lnnoptr) {
		    bind_err(SAY_NORMAL, RC_ERROR,
			     NLSMSG(BAD_OVERFLOW2,
    "%1$s: 0711-588 WARNING: Section header %2$d in object %3$s:\n"
    "\tFields in the section header are inconsistent with fields\n"
    "\tin the corresponding overflow section header %4$d."),
			     Main_command_name, p_index,
			     get_object_file_name(obj), i + 1);
		}

	    obj->oi_section_info[primary_sect_no].l_reloc_count
		= scn_ptr->s_paddr;
	    obj->oi_section_info[primary_sect_no].l_linenum_last
		= scn_ptr->s_lnnoptr + LINESZ * scn_ptr->s_vaddr;

	  lsect_common:
	    obj->oi_section_info[primary_sect_no].u.raw_offset = base_offset
		+ primary_scn_ptr->s_scnptr - primary_scn_ptr->s_paddr;
	    sect_starts[primary_sect_no] = primary_scn_ptr->s_paddr;
	    obj->oi_section_info[primary_sect_no].sect_size
		= primary_scn_ptr->s_size;
	    obj->oi_section_info[primary_sect_no].sect_flags
		|= (SECT_RAWDATA | SECT_CODE);
	    obj->oi_section_info[primary_sect_no].l_toc_anchor = NULL;
	    obj->oi_section_info[primary_sect_no].l_csect_heap = NULL;
	    obj->oi_section_info[primary_sect_no].l_reloc_base
		= base_offset + scn_ptr->s_relptr;
	    break;

	  default:
	    /* Restore old value of Bind_state.sections_seen. */
	    Bind_state.sections_seen = old_sections_seen;
	    bind_err(SAY_NORMAL, RC_WARNING,
		     NLSMSG(INSRT_UNKSECT,
    "%1$s: 0711-523 WARNING: Section %2$d (%3$.8s) in object %4$s:\n"
    "\tSection type %5$d is not recognized. The section is being ignored."),
		     Main_command_name, i+1,
		     scn_ptr->s_name,
		     get_object_file_name(obj),
		     obj->oi_section_info[i].sect_type);
	} /* switch */
    } /* for each section */

    /* Don't check for missing overflow headers if we had any errors. */
    if (rc == -1)
	return -1;

    /* If overflow_needed > 0, we search through section headers looking
       for SECT_OVERFLOW_NEEDED flag. */
    for (i = 0; overflow_needed > 0 && i < obj->oi_num_sections; i++) {
	rc = -1;
	if (obj->oi_section_info[i].sect_flags & SECT_OVERFLOW_NEEDED) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(OBJ_BADOVERFLOW,
	    "%1$s: 0711-719 SEVERE ERROR: Section %2$d in object %3$s"
	    "\tThe overflow section header is missing for this section."),
		     Main_command_name,
		     i + 1, get_object_file_name(obj));
	    --overflow_needed;
	}
    }

    if (rc == -1)
	return -1;
    if (obj_ok == 0) {
	bind_err(SAY_NORMAL, RC_ERROR,
		 NLSMSG(OBJ_BADFILE,
			"%1$s: 0711-725 ERROR: Object %2$s\n"
	"\thas no .text, .data, or .bss sections."),
		 Main_command_name, get_object_file_name(obj));
	obj->o_type = O_T_IGNORE;
	return -1;
    }
    return 0;
} /* process_section_headers */
/************************************************************************
 * Name: process_symbol_entry
 *									*
 * Purpose:	Take a symbol from an XCOFF symbol table and add it to
 *		the internal data structures.
 *									*
 * Arguments:								*
 *									*
 * Returns:	SYMBOL entered
 *
 ************************************************************************/
static SYMBOL *
process_symbol_entry(int j,		/* Input index of symbol */
		     SYMENT *cursym,	/* Symbol table entry for symbol */
		     AUXENT *auxsym,	/* CSECT auxiliary entry for symbol */
		     STR *name_ptr,	/* Name of symbol */
		     CSECT *cur_csect,	/* CSECT containing symbol */
		     SYMBOL *cur_symbol, /* Previous SYMBOL in CSECT (or NULL
					    if symbol is the CSECT) */
		     OBJECT *cur_object, /* OBJECT containing symbol */
		     const int s_flags_init) /* S_ARCHIVE_SYMBOL if cur_object
						is an archive member. */
{
    SYMBOL *tmp_symbol;
    SYMBOL *prev, **prev_symbol_slot;
    SYMBOL *prev_sym, *next_sym;
    SYMENT syment_copy;

    if (cursym->n_sclass == C_HIDEXT) {
      handle_HIDEXT:
	tmp_symbol = create_SYMBOL(cur_object, cur_csect,
				   cur_symbol ? NULL : &cur_csect->c_symbol,
				   j, name_ptr, cursym, auxsym,
				   s_flags_init);

	/* The order of hidden symbols doesn't matter, so just
	   insert symbol at beginning of list of hidden symbols.
	   QUESTION:  The order might affect the way diagnostic information
	   is printed.  Is this important?  */
	tmp_symbol->s_synonym = name_ptr->first_hid_sym;
	name_ptr->first_hid_sym = tmp_symbol;
    }
    else if (reading_archive_object_symbol != NULL) {
	/* The current name must have been in the global symbol table, so
	   it is already in our symbol table.  If it is the label on a
	   CSECT, we must use the symbol fields in the csect.  Otherwise,
	   we simple replace the fields in the existing symbol. */

	for (prev_symbol_slot = &name_ptr->first_ext_sym,
	     tmp_symbol = name_ptr->first_ext_sym;
	     tmp_symbol;
	     prev_symbol_slot = &tmp_symbol->s_synonym,
	     tmp_symbol = tmp_symbol->s_synonym) {
	    if (tmp_symbol->s_smtype == XTY_AR
		&& tmp_symbol->s_object == cur_object) {

		/* Remove tmp_symbol from GST chain */
		next_sym = tmp_symbol->s_next_in_csect;
		prev_sym = tmp_symbol->s_prev_in_gst;
		if (next_sym)
		    next_sym->s_prev_in_gst = prev_sym;
		if (prev_sym)
		    prev_sym->s_next_in_csect = next_sym;
		else
		    tmp_symbol->s_object->o_member_info->o_parent->o_gst_syms
			= next_sym;
		tmp_symbol->s_next_in_csect = NULL;

		if (cur_symbol)
		    cur_symbol->s_next_in_csect = tmp_symbol;
		else {
		    /* New symbol is part of new csect--we must set
		       up this symbol. */
		    cur_symbol = &cur_csect->c_symbol;
		    if (reading_archive_object_symbol == tmp_symbol)
			reading_archive_object_symbol = cur_symbol;
		    cur_symbol->s_name = name_ptr;
		    cur_symbol->s_synonym = tmp_symbol->s_synonym;
		    *prev_symbol_slot = cur_symbol;
		    cur_symbol->s_flags = tmp_symbol->s_flags;
		    cur_symbol->s_number = tmp_symbol->s_number;
		    cur_symbol->s_next_in_csect = NULL;
		    /*free_symbol(tmp_symbol);*/
		    tmp_symbol = cur_symbol;
		}
		tmp_symbol->s_smtype = auxsym->x_csect.x_smtyp & 7;
		tmp_symbol->s_smclass = auxsym->x_csect.x_smclas;
		tmp_symbol->s_addr = cursym->n_value;
		tmp_symbol->s_inpndx = j;
		tmp_symbol->s_typechk = create_TYPECHK(auxsym, cur_object,
						       j, name_ptr);
		tmp_symbol->s_csect = cur_csect;

		return tmp_symbol;
	    }
	}

	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(ARCHIVE_MISSING_SYM,
 "%1$s: 0711-201 SEVERE ERROR: External symbol %2$s, defined in member %3$s\n"
 "\tof archive %4$s, was not found in the archive's global symbol table."),
		 Main_command_name,
		 name_ptr->name,
		 cur_object->o_member_info->o_member->name,
		 cur_object->o_ifile->i_name->name);
	/* Change the symbol into an internal symbol, because we may have
	   already resolved a symbol with this external name.  There is
	   no right thing to do, but this may be the least surprising. */
	syment_copy = *cursym;
	syment_copy.n_sclass = C_HIDEXT;
	cursym = &syment_copy;
	goto handle_HIDEXT;
    }
    else {
	/* Add the symbol */
	tmp_symbol = create_SYMBOL(cur_object, cur_csect,
				   cur_symbol ? NULL : &cur_csect->c_symbol,
				   j, name_ptr, cursym, auxsym,
				   s_flags_init);
	if (prev = name_ptr->first_ext_sym) {
	    /* We have found a duplicate symbol.  We add the new symbol at
	       the end of the chain of external symbols with the same name. */

	    DEBUG_MSG(XCOFF_DEBUG | DEBUG_LONG,
		  (SAY_NO_NLS,
		   "\nSD, CM, or LD found after existing symbol %s",
		   prev->s_name->name));

	    /* Here is a shortcut for delcsect processing.  If any other
	       external instance of this symbol has not been deleted, then
	       delcsect processing will have to be done for this object. The
	       global flag some_duplicates indicates that do_del_csect will
	       have to be called. */
	    if (!(prev->s_flags & S_DUPLICATE))
		some_duplicates = 1;
	    while (prev->s_synonym) {
		prev = prev->s_synonym;
		if (!(prev->s_flags & S_DUPLICATE))
		    some_duplicates = 1;
	    }
	    prev->s_synonym = tmp_symbol;
	}
	else				/* Insert symbol at head of list */
	    name_ptr->first_ext_sym = tmp_symbol;
    }

    if (cur_symbol)
	cur_symbol->s_next_in_csect = tmp_symbol;
    return tmp_symbol;
} /* process_symbol_entry */
/************************************************************************
 * Name: do_del_csect
 *									*
 * Purpose: Check all the symbols in a given csect.  If a duplicate
 *	is found, delete it and all other symbols in the csect.
 *									*
 * Arguments:								*
 *									*
 * Returns:
 *
 ************************************************************************/
static void
do_del_csect(OBJECT *obj,
	     CSECT *cs)
{
    int		heading1_printed = 0;
    int		heading2_printed = 0;
    int		symbols_deleted = 0;
    SYMBOL	*sym, *sym1;

    for (sym = &cs->c_symbol; sym; sym = sym->s_next_in_csect) {
	if (sym->s_smtype == XTY_CM || sym->s_flags & S_HIDEXT)
	    continue;	/* Commons and internal symbols can be duplicates */
	/* Find first instance of symbols with sym's name that does not
	   have S_DUPLICATE set.  Since 'sym' itself is on the list, the
	   loop must terminate. */
	for (sym1 = sym->s_name->first_ext_sym;
	     sym1->s_flags & S_DUPLICATE;
	     sym1 = sym1->s_synonym)
	    /* skip */ ;

	if (sym1 == sym)
	    continue;			/* All predecessors checked. */
	if (sym1->s_smtype == XTY_AR) {
	    sym1 = read_archive_member_for_symbol(sym1);
	    if (sym1->s_smtype == XTY_AR) {
		/* Mark as duplicate--symbol in GST not found in member */
		sym1->s_flags |= S_DUPLICATE;
		continue;
	    }
	}
	if (sym1->s_smtype == XTY_CM || sym1->s_flags & S_HIDEXT)
	    continue;

	/* Now we know "sym" is a duplicate.  Delete it and
	   all the other symbol in this csect. */
	if (heading1_printed == 0) {
	    /* Use say() to prevent message from going to stderr. */
	    say(SAY_NORMAL,
		NLSMSG(CMPCT_DUP,
		       "%1$s: 0711-578 WARNING: Duplicate symbols deleted:"),
		Main_command_name);
	    heading1_printed = 1;
	}

	/* This call prints to stderr only. */
	bind_err(SAY_STDERR_ONLY, RC_WARNING,
		 NLSMSG(CMPCT_DUP2,
	"%1$s: 0711-585 WARNING: Object %2$s: Duplicate symbol deleted: %3$s"),
		 Main_command_name,
		 get_object_file_name(obj),
		 sym->s_name->name);
	show_loadmap_message(RC_WARNING);

	/* This call prints to the loadmap or stdout only. */
	minidump_symbol(sym, MINIDUMP_NAME_LEN,
			MINIDUMP_SYMNUM_DBOPT11
			| MINIDUMP_INPNDX
			| MINIDUMP_TYPE
			| MINIDUMP_SMCLASS
			| MINIDUMP_SOURCE_INFO,
			NULL);
	sym->s_flags |= S_DUPLICATE | S_DUPLICATE2;
	symbols_deleted = 1;
    }
    if (symbols_deleted == 0)
	return;

    /* Now go though all symbols in csect and delete them. */
    for (sym = &cs->c_symbol; sym; sym = sym->s_next_in_csect) {
	if (!(sym->s_flags & S_DUPLICATE)) {
	    sym->s_flags |= S_DUPLICATE;
	    if (sym->s_flags & S_HIDEXT)
		continue;
	    if (heading2_printed == 0) {
		/* Use say() to prevent message from going to stderr. */
		say(SAY_NORMAL,
		    NLSMSG(IMPSS_SYMBDEL,
		   "%1$s: 0711-411 WARNING: Additional symbols deleted:"),
		    Main_command_name);
		heading2_printed = 1;
	    }

	    /* This call prints to stderr only. */
	    bind_err(SAY_STDERR_ONLY, RC_WARNING,
		     NLSMSG(IMPSS_SYMBDEL2,
	    "%1$s: 0711-579 WARNING: Additional symbol deleted: %2$s"),
		     Main_command_name,
		     sym->s_name->name);

	    /* This call prints to the loadmap or stdout only. */
	    minidump_symbol(sym, MINIDUMP_NAME_LEN,
			    MINIDUMP_SYMNUM_DBOPT11
			    | MINIDUMP_INPNDX
			    | MINIDUMP_TYPE
			    | MINIDUMP_SMCLASS
			    | MINIDUMP_SOURCE_INFO,
			    NULL);
	}
    }
} /* do_del_csect */
/************************************************************************
 * Name: put_rel							*
 *									*
 * Purpose: Copy appropriate information from a RELOC entry in the
 *	XCOFF file into an RLD entry.
 *
 * NOTE:	When put_rel is called the first time for a given section,
 *		static variables 'high' and 'prev_addr' must be set to 0.
 *		Therefore, the first test fails and the while loop is
 *		execute at least once, assuring that 'low' will be set.
 *
 ************************************************************************/
static RETCODE
put_rel(RELOC		*rel_ptr,	/* Reloc entry to convert */
	RLD		*rld,		/* RLD entry to store info in */
	CSECT_HEAP	h,		/* Heap for looking up addresses */
	ITEM		*s)		/* List of symbols read from symbol
					   table (in symbol table index
					   order) */
{
    static CSECT	*cs;
    ITEM		*item;
    static ulong	low;

    item = find_item(s, rel_ptr->r_symndx);
    if (item == NULL || item->item_symbol == NULL) {
	/* If item is not NULL, but item->item_symbol is NULL,
	   Referenced SYMBOL was read, but had an error. */
	if (item == NULL)
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(INSRT_RLDBADREF2,
	    "%1$s: 0711-596 SEVERE ERROR: Object %2$s\n"
	    "\tAn RLD for section %3$d (%4$s) refers to symbol %5$d,\n"
	    "\tbut the storage class of the symbol is not C_EXT or C_HIDEXT."),
		     Main_command_name, get_object_file_name(rld_object),
		     rld_sect_num,
		     rld_section->sect_type == STYP_TEXT ? ".text" : ".data",
		     rel_ptr->r_symndx);
	prev_rld->r_next = &rld[1];
	return RC_OK;			/* Allow processing to continue */
    }

    if (prev_addr > rel_ptr->r_vaddr) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(INSRT_RLDORDER,
"%1$s: 0711-548 SEVERE ERROR: Object %2$s cannot be processed.\n"
"\tRLD address 0x%3$x for section %4$d (%5$s) is\n"
"\tout of order. RLDs must be in ascending address order."),
		 Main_command_name, get_object_file_name(rld_object),
		 rel_ptr->r_vaddr,
		 rld_sect_num,
		 rld_section->sect_type == STYP_TEXT ? ".text" : ".data");
	return RC_SEVERE;
    }

    if (rel_ptr->r_vaddr >= high) {
	/* Terminate chain of RLDs for previous csect */
	prev_rld->r_next = NULL;

	do {
	    /* Get next csect (ordered by address) */
	    cs = deletemin(h);
	    if (cs == NULL) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(INSRT_RLDSECT,
"%1$s: 0711-547 SEVERE ERROR: Object %2$s cannot be processed.\n"
"\tRLD address 0x%3$x for section %4$d (%5$s) is\n"
"\tnot contained in the section."),
			 Main_command_name, get_object_file_name(rld_object),
			 rel_ptr->r_vaddr,
			 rld_sect_num,
			 rld_section->sect_type == STYP_TEXT
				? ".text" : ".data");
		return RC_SEVERE;
	    }
	    low = cs->c_addr;
	    high = low + cs->c_len;
	    DEBUG_MSG(XCOFF_DEBUG,
		      (SAY_NO_NLS, "Next csect found at %x, addr %x to %x",
		       cs, low, high));
	} while (rel_ptr->r_vaddr >= high);
	cs->c_first_rld = rld;
    }

    if (rel_ptr->r_vaddr < low) {
	/* This case can happen if there are holes between csects, such
	   as when padding is inserted for alignment. */
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(INSRT_RLDSECT,
"%1$s: 0711-547 SEVERE ERROR: Object %2$s cannot be processed.\n"
"\tRLD address 0x%3$x for section %4$d (%5$s) is\n"
"\tnot contained in the section."),
		 Main_command_name, get_object_file_name(rld_object),
		 rel_ptr->r_vaddr,
		 rld_sect_num,
		 rld_section->sect_type == STYP_TEXT ? ".text" : ".data");
	return RC_SEVERE;
    }

    rld->r_flags = 0;

    rld->r_sym = item->item_symbol;
    rld->r_csect = cs;
    rld->r_next = &rld[1];		/* RLDs are allocated consecutively. */
    rld->r_addr = rel_ptr->r_vaddr;
    rld->r_length = RELOC_RLEN(*rel_ptr) + 1;
    rld->r_reltype = RELOC_RTYPE(*rel_ptr);

    /* Copy SIGN and FIXUP bits to initialize flags. */
    rld->r_flags = rel_ptr->_r._r_r._r_rsize & (R_SIGN | R_FIXUP);

    if (item->item_symbol->s_smtype == XTY_ER)
	rld->r_flags |= (RLD_EXT_REF | RLD_RESOLVE_BY_NAME);
    else if (!(item->item_symbol->s_flags & S_HIDEXT))
	rld->r_flags |= RLD_RESOLVE_BY_NAME;

    prev_rld = rld;
    prev_addr = rel_ptr->r_vaddr;
    return RC_OK;
} /* put_rel */
/************************************************************************
 * Name: aligned_reloc_ptr
 *									*
 * Purpose: Returns the next RLD item in an aligned buffer.  This routine
 *	is only required on architectures that require alignment, so
 *	for POWER, a #define is used instead of the function.
 *									*
 * Arguments:								*
 *									*
 * Returns:
 *
 ************************************************************************/
#ifdef ALIGNPTRS
static RELOC *
aligned_reloc_ptr(caddr_t base,
		  int n)
{
    static RELOC rtemp;
    int s;

    s = (int)base + n * RELSZ;
    if (s & 0x3) {
	memcpy(&rtemp, (void *)s, RELSZ);
	return &rtemp;
    }
    else
	return (RELOC *)s;
} /* aligned_reloc_ptr */
#else
#define aligned_reloc_ptr(b,n) ((RELOC *)((int)b+(n)*RELSZ))
#endif /* ALIGNPTRS */
/************************************************************************
 * Name: read_section_rlds
 *									*
 * Purpose:	Read the RLDS for section.
 *									*
 * Arguments:	o:		OBJECT being read.
 *		sect_num:	Section number--must be the number of a
 *				.text or .data section.
 *									*
 * Returns:	RC_OK		if the section has no RLDs
 *		RC_SEVERE	if an error occurs seeking within or
 *				reading the object file
 *		non-RC_OK value returned from put_rel()	otherwise
 ************************************************************************/
RETCODE
read_section_rlds(OBJECT *o,
		  int sect_num)
{
    int		j;
    long	num_relocs;
    CSECT_HEAP	h;
    ITEM	*syms = o->oi_syms_lookup;
    RELOC	rtemp;
    RETCODE	rc = RC_OK;
    RLD		*rlds;
    RLD		rld_root;		/* Dummy to anchor RLD chain */
    SECTION	*section;
    caddr_t	reloc_base;

    section = &o->oi_section_info[sect_num-1];

    if ((num_relocs = section->l_reloc_count) == 0)
	return RC_OK;

    h = section->l_csect_heap;
    rlds = get_RLDs(num_relocs);	/* get memory for RLD entries */

    /* save arguments in static variables for error messages */
    rld_object = o;
    rld_sect_num = sect_num;
    rld_section = section;

    /* Initialize static variables for put_rel */
    prev_rld = &rld_root;
    high = 0;
    prev_addr = 0;

#ifdef QUICK
    {
	extern int	last_mid;
	last_mid = -1;
    }
#endif

#ifdef READ_FILE
    if (o->o_ifile->i_access == I_ACCESS_READ) {
	if (safe_fseek(o->o_ifile, section->l_reloc_base, SEEK_SET) != 0) {
	    o->o_type = O_T_IGNORE;
	    return RC_SEVERE;
	}

	/* Get relevant information from relocation entries */
	for (j = 0; j < num_relocs && rc == RC_OK; j++) {
	    if (safe_fread(&rtemp, RELSZ, o->o_ifile) != 0) {
		o->o_type = O_T_IGNORE;
		rc = RC_SEVERE;
	    }
	    else
		rc = put_rel(&rtemp, &rlds[j], h, syms);
	}
    }
    else
#endif /* READ_FILE */
    {
	reloc_base = o->o_ifile->i_map_addr + section->l_reloc_base;

	/* Get relevant information from relocation entries */
	for (j = 0; j < num_relocs && rc == RC_OK; j++) {
	    rc = put_rel(aligned_reloc_ptr(reloc_base, j), &rlds[j], h, syms);
	}
    }
    prev_rld->r_next = NULL;		/* Terminate last chain */
    STAT_use(RLDS_ID, j);
    return rc;
} /* read_section_rlds */
