#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)03	1.13  src/bos/usr/ccs/bin/ld/bind/shared.c, cmdld, bos411, 9428A410j 5/9/94 11:04:04")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: read_shared_object_symbols
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

#include <string.h>
#include <stdio.h>

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

#include "objects.h"
#include "symbols.h"
#include "ifiles.h"

/***********************************************************************
 * Name:	read_shared_object_symbols()
 * Purpose:	Read symbols exported from the loader section
 *			of a shared object
 * Given:	OBJECT *obj:		Object we're reading
 *		off_t base_offset:	Offset (in object's file) to beginning
 *					of object.
 * Returns:	Nothing, but a maximum return code will be set because of
 *		bind_err calls.
 *		If reading of symbols interrupted,	RC_NI_WARNING is set
 *		If other errors occur,			RC_SEVERE
 ***********************************************************************/
void
read_shared_object_symbols(OBJECT *obj,
			   off_t base_offset)
{
    int		i, found = 0;
#ifdef READ_FILE
    int		read_file;
#endif
    const int	s_flags_init = obj->o_contained_in_type == O_T_ARCHIVE
				? S_ARCHIVE_SYMBOL : 0;
    char	*p, *impname, *ldstr;
    STR		*impid = NULL,
		*name_ptr;
    SYMBOL	*sym;
    LDHDR	*ldhdr;
    LDSYM	*ldsym;
    SCNHDR	*scn_ptr;		/* Pointer to section headers */
    CSECT	*cs;
    caddr_t	highest_valid_address;	/* Last byte in loader section */

    impname = get_object_file_name(obj);

#ifdef READ_FILE
    read_file = (obj->o_ifile->i_access == I_ACCESS_READ);

    if (read_file) {
	scn_ptr = alloca(SCNHSZ * obj->o_num_sections);
	/* Read all the section headers.  We don't bother using the
	   o_snloader field in the auxiliary header. */
	if (fseek_read(obj->o_ifile,
		       base_offset + FILHSZ + obj->o_opt_hdr_size,
		       scn_ptr,
		       SCNHSZ * obj->o_num_sections) != 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SHARED_NOLDR5,
    "%1$s: 0711-444 SEVERE ERROR: Shared object %2$s\n"
    "\tThe section headers could not be read. The object is being ignored."),
		     Main_command_name, impname);
	    goto ignore_object;
	}
    }
    else
#endif
	scn_ptr = (SCNHDR *)(obj->o_ifile->i_map_addr + base_offset
			     + FILHSZ + obj->o_opt_hdr_size);

    /* Examine section headers */
    for (i = 0; i < obj->o_num_sections; scn_ptr++, i++) {
	/* Find the section header */
	if ((scn_ptr->s_flags & 0xFFFF) == STYP_LOADER)
	    goto loader_section_found;
    }
    bind_err(SAY_NORMAL, RC_SEVERE,
	     NLSMSG(SHARED_NOLDR1,
    "%1$s: 0711-434 SEVERE ERROR: Shared object %2$s\n"
    "\tThe shared object has no .loader section and is being ignored."),
	     Main_command_name, impname);
    goto ignore_object;

  loader_section_found:
    /* If library command was used to read file, get basename of input name. */
    if (obj->o_ifile->i_library) {
	p = strrchr(impname, '/');
	if (p != NULL)
	    ++p;
	else
	    p = impname;
    }
    else
	p = impname;
    impid = putstring(p);

    /* Make sure entire loader-section header exists */
    if (scn_ptr->s_size < LDHDRSZ)
	goto object_truncated;

    /* Validate values */
    if ((unsigned)scn_ptr->s_scnptr > obj->o_size
	|| (unsigned)scn_ptr->s_size > obj->o_size - scn_ptr->s_scnptr) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(SHARED_NOLDR4,
 "%1$s: 0711-441 SEVERE ERROR: Shared object %2$s\n"
 "\tThe s_scnptr or s_size field is invalid in the .loader section header.\n"
 "\tThe object is being ignored."),
		 Main_command_name, impname);
	goto ignore_object;
    }

    /* Get pointer to loader-section header. */
#ifdef READ_FILE
    if (read_file) {
	ldhdr = alloca(scn_ptr->s_size);
	if (fseek_read(obj->o_ifile, base_offset + scn_ptr->s_scnptr,
		       ldhdr, scn_ptr->s_size) != 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SHARED_NOLDR6,
	    "%1$s: 0711-445 SEVERE ERROR: Shared object %2$s\n"
	    "\tCannot read the .loader section. The object is being ignored."),
		     Main_command_name, impname);
	    goto ignore_object;
	}
    }
    else
#endif
    {
	ldhdr = (LDHDR *)(obj->o_ifile->i_map_addr + base_offset
			  + scn_ptr->s_scnptr);
    }

    highest_valid_address = (caddr_t)ldhdr + scn_ptr->s_size;

    ldsym = (LDSYM *)((char *)ldhdr + LDHDRSZ);
    if ((caddr_t)&ldsym[ldhdr->l_nsyms] > highest_valid_address)
	goto object_truncated;

    /* Get pointer to loader-section string table, if it exists */
    if (ldhdr->l_stlen > 0) {
	ldstr = (char *)ldhdr + ldhdr->l_stoff;
	if (&ldstr[ldhdr->l_stlen] > highest_valid_address)
	    goto object_truncated;
    }

    /* Allocate a single SRCFILE and CSECT for the shared object */
    obj->o_srcfiles = get_init_SRCFILE(obj, impid);
    cs = get_init_CSECT();
    obj->o_srcfiles->sf_csect = cs;
    cs->c_srcfile = obj->o_srcfiles;
    cs->c_secnum = N_IMPORTS;

    sym = NULL;

    Size_estimates.num_symbols += ldhdr->l_nsyms;

    /* Look for exported symbols in loader section symbol table. */
    for (i = 0; i < ldhdr->l_nsyms; ldsym++, ++i) {
	if (interrupt_flag) {
	    bind_err(SAY_NORMAL, RC_NI_WARNING,
		     NLSMSG(MAIN_INTERRUPT,
	    "%1$s: 0711-635 WARNING: Binder command %2$s interrupted."),
		     Main_command_name, Command_name);
	    bind_err(SAY_NORMAL, RC_NI_WARNING,
		     NLSMSG(SHARED_INTERRUPTED,
 "%1$s: 0711-448 WARNING: Some symbols were not read from shared object %2$s.\n"
	"\tAdditional errors may occur."),
		     Main_command_name, impname);
	    break;
	}

	if (!LDR_EXPORT(*ldsym))
	    continue;

	if (ldsym->l_zeroes == 0) {
	    if ((unsigned long)ldsym->l_offset > ldhdr->l_stlen) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(SHARED_BAD_LDR_OFFSET,
 "%1$s: 0711-449 SEVERE ERROR: Shared object %2$s\n"
 "\tSymbol table entry %3$d  in the .loader section is invalid. The value of\n"
 "\tthe l_offset field is %4$d, but should be between 0 and %5$d.\n"
 "\tThe symbol is being ignored."),
			 Main_command_name, impname,
			 i, ldsym->l_offset, ldhdr->l_stlen);
		continue; /* Ignore garbled symbol */
	    }
	    name_ptr = putstring_len(&ldstr[ldsym->l_offset],
				     ldhdr->l_stlen - ldsym->l_offset);
	}
	else
	    name_ptr = putstring_len(ldsym->l_name, SYMNMLEN);
	name_ptr->flags |= STR_ISSYMBOL;

	sym = create_imported_SYMBOL(name_ptr, cs, sym,
				     ldsym->l_smclas, ldsym->l_value);
	found++;

	/* Initialize remaining fields of symbol */
	if (ldsym->l_parm == 0)
	    sym->s_typechk = NULL;
	else if ((unsigned long)ldsym->l_parm > ldhdr->l_stlen) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SHARED_BAD_LOADER,
 "%1$s: 0711-446 SEVERE ERROR: Shared object %2$s\n"
 "\tSymbol table entry %3$d (symbol %4$s) in the .loader section is invalid.\n"
 "\tThe value of the l_parm field is %5$d, but should be be between\n"
 "\t0 and %6$d. No type-checking string will be read for the symbol."),
		     Main_command_name, impname,
		     i, name_ptr->name, ldsym->l_parm, ldhdr->l_stlen);
	    sym->s_typechk = NULL;
	}
	else
	    sym->s_typechk = put_TYPECHK(&ldstr[ldsym->l_parm],
					 ldhdr->l_stlen - ldsym->l_parm);
	sym->s_smtype = XTY_IS;	/* Imported symbol (from shared lib).
				   Note:  The actual type in the
				   l_smtype field is ignored.  */
	if (sym->s_smclass == XMC_SV)
	    sym->s_smclass = XMC_DS;
	sym->s_flags |= s_flags_init;
    }

    say(SAY_NORMAL,
	NLSMSG(SHARED_COUNT,
	       "%1$s: Shared object %2$s: %3$d symbols imported."),
	Command_name, impid->name, found);
    return;

  object_truncated:
    bind_err(SAY_NORMAL, RC_SEVERE,
	     NLSMSG(SHARED_TRUNCATED,
    "%1$s: 0711-447 SEVERE ERROR: Shared object %2$s\n"
    "\tThe .loader section has been truncated. The object is being ignored."),
	     Main_command_name, impname);

  ignore_object:
    obj->o_srcfiles = NULL;
    obj->o_type = O_T_IGNORE;
}
