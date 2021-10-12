#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)85	1.15  src/bos/usr/ccs/bin/ld/bind/archive.c, cmdld, bos411, 9428A410j 1/28/94 13:06:03")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: allocate_archive_objects
 *		read_archive_member_for_symbol
 *		read_archive_symbols
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
#include <stddef.h>
#include <string.h>
#include <unistd.h>

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
#include "symbols.h"
#include "objects.h"

/* Global variables */
SYMBOL *reading_archive_object_symbol;

/* Length of an ASCII number in an archive */
#define ARCHIVE_NUMBER_LENGTH 12

/* Note:  Nested archives are not allowed, so base_offset is always 0.
   If nested archive are ever allowed, the define for BASE_OFFSET should
   be changed to "#define BASE_OFFSET base_offset" */
#define BASE_OFFSET 0

/* ***********************************
 * NAME:	read_archive_member_for_symbol()
 *
 * PURPOSE:	For a symbol defined in the global symbol table of an archive,
 *		read the member itself to get the true definition.
 *
 *		If possible, the SYMBOL is overwritten with the new
 *		information.  If the symbol is a CSECT, a new CSECT will be
 *		allocated, and the returned symbol will be the one within
 *		the CSECT.
 *
 * GIVEN:	SYMBOL pointer
 * ***********************************/
SYMBOL *
read_archive_member_for_symbol(SYMBOL *sym)
{
    OBJECT	*o = sym->s_object;
    off_t	member_offset;

    DEBUG_MSG(ARCHIVE_DEBUG,
	      (SAY_NO_NLS, "Reading archive member %s to read symbol %s",
	       get_object_file_name(o), sym->s_name->name));

    if (o->o_type != O_T_OBJECT)
	if (o->o_type != O_T_IGNORE)
	    internal_error();
	else {
	    /* This symbol must have been read from the global symbol table of
	       an archive.  While reading the actual member for another symbol,
	       an error must have occurred.  We don't try to read the member
	       again. */
	    goto display_error;
	}

    if (!(o->oi_flags & OBJECT_NOT_READ)) {
	/* A previous attempt to read a member containing this symbol appears
	   to have failed.  We do not try again. */
	goto display_error;
    }

    o->oi_flags &= ~OBJECT_NOT_READ;	/* Mark object as read. */

    if (ifile_reopen_remap(o->o_ifile) != RC_OK)
	return sym;			/* Symbol cannot be read.  Error
					   message has already been printed. */

    member_offset = o->o_member_info->o_ohdr_off
	+ AR_HSZ + ROUND(o->o_member_info->o_member->len, 2);

    /* Set global variable for use by read_object_symbols(). */
    reading_archive_object_symbol = sym;
    (void) read_object_symbols(o, member_offset);

    /* Note:  reading_archive_object_symbol will be modified by
       read_object_symbols() if the symbol is a csect name, because a
       CSECT structure includes the SYMBOL structure.  */
    sym = reading_archive_object_symbol;
    reading_archive_object_symbol = NULL;

    /* If the symbol could not be read, its type will still be XTY_AR,
       and it should be treated as undefined. */
    if (sym->s_smtype == XTY_AR) {
      display_error:
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(ARCHIVE_GST_BAD,
"%1$s: 0711-212 SEVERE ERROR: Symbol %2$s, found in the global symbol table\n"
	"\tof archive %3$s, was not defined in archive member %4$s."),
		 Main_command_name,
		 sym->s_name->name,
		 o->o_ifile->i_name->name,
		 o->o_member_info->o_member->name);
    }
    return sym;
}
/* ***********************************
 * NAME:	allocate_archive_objects()
 * PURPOSE:	Allocate the OBJECTs for an archive
 * GIVEN:	ifile:		Input file (could be obtained from the
 *					'archive_obj' parameter)
 *		archive_obj:	Pointer to archive object
 *		Fl_hdr:	Pointer to archive header, if i_access==I_ACCESS_READ
 *		base_offset:	Offset (withing file) to beginning of object.
 *			This will be 0 until nested archives are supported.
 * RESULTS:	If there is an error, the OBJECT's type is set to O_T_IGNORE.
 *		Otherwise, the OBJECT structures are allocated for each member,
 *		as well as the global symbol table, if present.
 * ***********************************/
void
allocate_archive_objects(IFILE *ifile,
			 OBJECT *archive_obj,
			 FL_HDR *Fl_hdr,
			 off_t base_offset)
{
#ifdef READ_FILE
    char		member_table_count[ARCHIVE_NUMBER_LENGTH+1],
			temp_name[256];
#endif
    char		*fbase;
#ifdef NO_NLS
    char		*e;
#else
    int			e;
#endif
    int			v, v2;		/* Parameters for error printing */
    int			i, nsyms;
    int			gst_exists;
    off_t		member_base_offset;
    long		number_members;
    long		Ar_gstoff, Ar_memoff, Ar_curmoff;
    long		size, member_table_size;
    long		ar_name_len;
    OBJECT		*xo;
    AR_HDR		temp_ar_hdr, *Ar_hdr;
    AR_HDR		*ar_ptr;
    HEADERS		*headers;
    OBJ_MEMBER_INFO	*arinfo;

#ifdef READ_FILE
    int			read_file = ifile->i_access == I_ACCESS_READ;
#endif

#ifdef DEBUG
    if (base_offset != 0)
	internal_error();
#endif

    size = archive_obj->o_size;
    archive_obj->o_type = O_T_IGNORE;	/* Initialize */

    /* Look at the fixed archive header to find the archive member table and
       the global symbol table.

       NOTE:  If ifile->i_access==I_ACCESS_READ, then Fl_hdr already points to
       the archive header, contained in a static variable.  */
#ifdef READ_FILE
    if (!read_file)
#endif
	{
	    Fl_hdr = (FL_HDR *)(ifile->i_map_addr + BASE_OFFSET);
	}

    /* Look for the archive member table */
    Ar_memoff = atol(Fl_hdr->fl_memoff);
    Ar_curmoff = atol(Fl_hdr->fl_fstmoff);	/* Offset of 1st member */

    /*The archive member table must exist, unless the archive has no members*/
    if (Ar_memoff == 0)
	if (Ar_curmoff == 0) {
	    bind_err(SAY_NORMAL, RC_WARNING,
		     NLSMSG(ARCHIVE_NO_MEMS,
		    "%1$s: 0711-236 WARNING: Archive %2$s does not contain\n"
		    "\tany members. The archive is being ignored."),
		     Main_command_name,
		     get_object_file_name(archive_obj));
	    return;
	}
	else {
	    /* Member table header, at least, must exist */
	    e = NLSMSG(ARCHIVE_NO_MTAB,
       "%1$s: 0711-200 SEVERE ERROR: Archive file %2$s cannot be processed.\n"
       "\tThe archive member table does not exist.");
	    goto report_error1;
	}

    /* Validate offset to member table: there must be at least enough room
       between the beginning of the member table and the end of the archive
       for a member header. */
    if (Ar_memoff < 0 || Ar_memoff > size - AR_HSZ) {
	v = Ar_memoff;
	e = NLSMSG(ARCHIVE_MTAB_BADOFF,
 "%1$s: 0711-202 SEVERE ERROR: Archive file %2$s cannot be processed.\n"
 "\tThe offset to the archive member table (%3$d), found in the archive\n"
 "\theader, is negative or is not consistent with the length of the archive.");
	goto report_error1;
    }

    /* Read or find member header for archive member table. */
#ifdef READ_FILE
    if (read_file) {
	if (fseek_read(ifile, BASE_OFFSET + Ar_memoff, &temp_ar_hdr, AR_HSZ)
	    != 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(ARCHIVE_BAD,
  "%1$s: 0711-214 SEVERE ERROR: Archive file %2$s cannot be processed."));
	    return;
	}
	Ar_hdr = &temp_ar_hdr;
    }
    else
#endif
	Ar_hdr = (AR_HDR *)((caddr_t)Fl_hdr + Ar_memoff);

    /* Validate member_table_size.  It must contain at least one ASCII number
       and cannot be any bigger then the space left between the end of the
       member header and the end of the archive. */
    member_table_size = atol(Ar_hdr->ar_size);
    if (member_table_size < ARCHIVE_NUMBER_LENGTH
	|| member_table_size > size - (Ar_memoff + AR_HSZ)) {
	e = NLSMSG(ARCHIVE_BAD_MEMTAB2,
   "%1$s: 0711-203 SEVERE ERROR: Archive file %2$s cannot be processed.\n"
   "\tThe length of the archive member table (%3$d) is less than %4$d\n"
   "\tor is not consistent with the length of the archive.");
	v2 = ARCHIVE_NUMBER_LENGTH;
	v = member_table_size;
	goto report_error1;
    }

    /* The length of the name of the archive member table should be 0, and
       the code assumes that it is 0, thus assuming that the member table
       begins at AR_HSZ bytes from Ar_memoff.  If the length of the name is
       not 0, other errors are likely to occur. */

    /* Read or locate first number in member table, which is the number
       of members.  We validate this number, but we do not check the rest
       of the member table for validity.  In other words, the number
       is only used to allocate the proper number of OBJECTs.  */
#ifdef READ_FILE
    if (read_file) {
	if (safe_fread(member_table_count, ARCHIVE_NUMBER_LENGTH, ifile)
	    != 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(ARCHIVE_BAD,
    "%1$s: 0711-214 SEVERE ERROR: Archive file %2$s cannot be processed."));
	    return;
	}
	member_table_count[ARCHIVE_NUMBER_LENGTH] = '\0';
	number_members = atol(member_table_count);
    }
    else
#endif
	{
	    number_members = atol((char *)(&Ar_hdr[1]));
	}

    /* We validate the number of members to avoid allocating a ridiculous
       number of OBJECTs.  For each member in the archive, the member table
       must contain the member's name (at least 2 bytes) and the ASCII offset
       to the member. In addition, the member table contains the count of
       members in the archive. We could also validate the count with the
       size of the entire archive. */
    if (number_members <= 0
	|| ARCHIVE_NUMBER_LENGTH	/*  For member count */
		/* For offset to each member and minimum name of each member */
		+ number_members * (ARCHIVE_NUMBER_LENGTH + 2)
	> member_table_size) {
	e = NLSMSG(ARCHIVE_BAD_MEMTAB3,
"%1$s: 0711-204 SEVERE ERROR: Archive file %2$s cannot be processed.\n"
"\tThe number of archive members (%3$d), specified in the archive member\n"
"\ttable, is less than or equal to 0 or is not consistent with the length\n"
"\tof the member table.");
	v = number_members;
	goto report_error1;
    }

    /* Convert offsets and validate. */
    Ar_gstoff = atol(Fl_hdr->fl_gstoff);

    /* Validate offset to global symbol table: there must be at least enough
       room between the beginning of the global symbol table and the end of
       the archive for a member header. */
    if (Ar_gstoff < 0 || Ar_gstoff > size - AR_HSZ) {
	e = NLSMSG(ARCHIVE_BAD_GST_OFF,
"%1$s: 0711-208 SEVERE ERROR: Archive file %2$s cannot be processed.\n"
"\tThe offset to the global symbol table (%3$d), found in the archive\n"
"\theader, is negative or is not consistent with the length of the archive.");
	v = Ar_gstoff;
	goto report_error1;
    }

    if (Ar_curmoff < 0 || Ar_curmoff > size - AR_HSZ) {
	e = NLSMSG(ARCHIVE_BADMEMOFF1,
   "%1$s: 0711-215 SEVERE ERROR: Archive file %2$s cannot be processed.\n"
   "\tThe offset to the first archive member header (%3$d),\n"
   "\tfound in the archive header, is negative or is not consistent\n"
   "\twith the length of the archive.");
	v = Ar_curmoff;
	goto report_error1;
    }

    /* Use already-allocated object for archive itself */
    archive_obj->o_num_subobjects = number_members + (Ar_gstoff != 0);
    archive_obj->o_type = O_T_ARCHIVE;

    /* Allocate contiguous objects for members, along with contiguous
       member_info structures.  */
    xo = new_init_member_objects(ifile,
				 archive_obj,
				 archive_obj->o_num_subobjects);

    /* If a global symbol table exists in the archive, it
       will be saved as the first OBJECT following the ARCHIVE itself.   We
       don't examine the global symbol table or its header now. */
    if (Ar_gstoff > 0) {
	gst_exists = 1;
	xo->o_type = O_T_ARCHIVE_SYMTAB;
	xo->o_member_info->o_ohdr_off = Ar_gstoff + BASE_OFFSET;
	xo->o_member_info->o_member = &NULL_STR;
	++xo;			/* Skip over the initial entry */
    }

    arinfo = xo->o_member_info;	/* Member_info structs were allocated
				   contiguously, so we can use a pointer to
				   address consecutive elements. */

#ifdef READ_FILE
    if (read_file)
	ar_ptr = &temp_ar_hdr;
#endif

    for (i = 0;
	 i < number_members && Ar_curmoff != Ar_memoff;
	 ++i, ++xo,
	 	v = Ar_curmoff, Ar_curmoff = atol(ar_ptr->ar_nxtmem)) {

	/* This test has already been done for the first member */
	if (Ar_curmoff < 0 || Ar_curmoff > size - AR_HSZ) {
	    v2 = Ar_curmoff;
	    e = NLSMSG(ARCHIVE_BADMEMOFF2,
"%1$s: 0711-216 SEVERE ERROR: Archive file %2$s cannot be\n"
"\tprocessed completely. The offset to the next member header (%4$d),\n"
"\tfound in the member header beginning at offset %3$u,\n"
"\tis negative or is not consistent with the length of the archive.");
	    goto report_error2;
	}

	/* Save offset with IFILE to beginning of member header */
	arinfo[i].o_ohdr_off = Ar_curmoff + BASE_OFFSET;
	/* Member[i] can't extend past end of the archive */
#ifdef READ_FILE
	if (read_file) {
	    /* Read member header except for name. */
	    if (fseek_read(ifile, BASE_OFFSET + Ar_curmoff, &temp_ar_hdr,
			   AR_HSZ - sizeof(temp_ar_hdr._ar_name)) != 0) {
		e = NLSMSG(ARCHIVE_BAD3,
  "%1$s: 0711-217 SEVERE ERROR: Archive file %2$s was not completely read.");
		goto report_error2;	/* Cannot continue reading. */
	    }

	    ar_name_len = atol(ar_ptr->ar_namlen);
	    if (ar_name_len <= 0 || ar_name_len > 255) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(ARCHIVE_BAD_NAMELN1,
	"%1$s: 0711-213 SEVERE ERROR: Archive file %2$s is corrupt.\n"
	"\tThe member header at offset %3$u is invalid. The length of the\n"
	"\tmember name (%4$d) is greater than 255, less than or\n"
	"\tequal to 0, or is not consistent with the length of the archive.\n"
	"\tThe member is being ignored."),
			 Main_command_name, get_object_file_name(archive_obj),
			 Ar_curmoff, ar_name_len);
		xo->o_type = O_T_IGNORE; /* Set object type */
		continue;
	    }

	    if (safe_fread(temp_name, ar_name_len, ifile) != 0) {
		e = NLSMSG(ARCHIVE_BAD3,
  "%1$s: 0711-217 SEVERE ERROR: Archive file %2$s was not completely read.");
		goto report_error2;	/* Don't try reading more members. */
	    }
	    temp_name[ar_name_len] = '\0';
	    arinfo[i].o_member = putstring(temp_name);
	}
	else
#endif
	{
	    fbase = archive_obj->o_ifile->i_map_addr + Ar_curmoff+BASE_OFFSET;
	    ar_ptr = (AR_HDR *)fbase;

	    ar_name_len = atol(ar_ptr->ar_namlen);
	    if (ar_name_len <= 0 || ar_name_len > 255
		|| ar_name_len > size - (Ar_curmoff + AR_HSZ)) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(ARCHIVE_BAD_NAMELN1,
	"%1$s: 0711-213 SEVERE ERROR: Archive file %2$s is corrupt.\n"
	"\tThe member header at offset %3$u is invalid. The length of the\n"
	"\tmember name (%4$d) is greater than 255, less than or\n"
	"\tequal to 0, or is not consistent with the length of the archive.\n"
	"\tThe member is being ignored."),
			 Main_command_name,
			 get_object_file_name(archive_obj),
			 Ar_curmoff,
			 ar_name_len);
		xo->o_type = O_T_IGNORE; /* Set object type */
		continue;
	    }

	    /* putstring_len() is guaranteed not to access memory past
	       ar_name_len characters of the name */
	    arinfo[i].o_member = putstring_len(ar_ptr->_ar_name.ar_name,
					       ar_name_len);
	}

	xo->o_size = atol(ar_ptr->ar_size);

	if (xo->o_size > size - (Ar_curmoff + AR_HSZ)) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(ARCHIVE_BAD_NAMELEN,
    "%1$s: 0711-210 SEVERE ERROR: The length of archive member %2$s (%3$d)\n"
    "\tis negative or is not consistent with the length of the archive.\n"
    "\tThe member is being ignored."),
		     Main_command_name,
		     get_object_file_name(xo));
	    xo->o_type = O_T_IGNORE;	/* Set object type */
	    continue;
	}

	/* Get base_offset of member itself */
	member_base_offset = Ar_curmoff + BASE_OFFSET
	    + AR_HSZ + ROUND(xo->o_member_info->o_member->len, 2);

	xo->o_type = read_magic_number(xo->o_ifile, &headers, xo,
				       member_base_offset, xo->o_size);
	switch(xo->o_type) {
	  case O_T_UNKNOWN:
	    bind_err(SAY_NORMAL, RC_WARNING,
		     NLSMSG(ARCHIVE_BAD_MEMBER,
    "%1$s: 0711-235 WARNING: Archive member %2$s is not\n"
    "\tan object file or an import list. The member is being ignored."),
		     Main_command_name,
		     get_object_file_name(xo));
	    xo->o_type = O_T_IGNORE;
	    break;
	  case O_T_TRUNCATED:
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(ARCHIVE_PREMEND,
  "%1$s: 0711-211 SEVERE ERROR: Archive member %2$s was found to be\n"
  "\ttruncated. The member is being ignored."),
		     Main_command_name,
		     get_object_file_name(xo));
	    xo->o_type = O_T_IGNORE;
	    break;
	  case O_T_IGNORE:
	  case O_T_SCRIPT:
	    break;
	  case O_T_SHARED_OBJECT:
	    if (headers->xcoff_hdr.f_flags & F_LOADONLY) {
		if (Switches.verbose) {
		    bind_err(SAY_NORMAL, RC_WARNING,
			     NLSMSG(ARCHIVE_LOADONLY,
    "%1$s: 0711-219 WARNING: Archive member %2$s\n"
    "\tThe member is being ignored because the F_LOADONLY flag is set."),
			     Main_command_name, get_object_file_name(xo));
		}
		xo->o_type = O_T_IGNORE;
	    }
	    else {
		/* Save information from the XCOFF header */
		xo->o_num_sections = headers->xcoff_hdr.f_nscns;
		xo->o_opt_hdr_size = headers->xcoff_hdr.f_opthdr;

		/* Update estimates--One SRCFILE for shared obj */
		Size_estimates.num_srcfiles++;
	    }
	    break;

	  case O_T_OBJECT:
	    if (headers->xcoff_hdr.f_flags & F_LOADONLY) {
		if (Switches.verbose) {
		    bind_err(SAY_NORMAL, RC_WARNING,
			     NLSMSG(ARCHIVE_LOADONLY,
    "%1$s: 0711-219 WARNING: Archive member %2$s\n"
    "\tThe member is being ignored because the F_LOADONLY flag is set."),
			     Main_command_name, get_object_file_name(xo));
		}
		xo->o_type = O_T_IGNORE;
	    }
	    else {
		allocate_object_info(xo, &headers->xcoff_hdr,
				     member_base_offset);
		Size_estimates.num_srcfiles++; /* Usually 1 SRCFILE/OBJECT */

		if (gst_exists && Switches.keepall == 0) {
		    xo->oi_flags |= OBJECT_NOT_READ;
		}
		else {
		    nsyms = headers->xcoff_hdr.f_nsyms/2; /* Upper limit */
		    Size_estimates.num_csects += nsyms;
		    Size_estimates.num_symbols += nsyms;
		}
	    }
	    break;
	  case O_T_ARCHIVE:
	    bind_err(SAY_NORMAL, RC_WARNING,
		     NLSMSG(ARCHIVE_NESTED,
	    "%1$s: 0711-230 WARNING: Nested archives are not supported.\n"
	    "\tArchive member %2$s is being ignored."),
		     Main_command_name,
		     get_object_file_name(xo));
	    xo->o_type = O_T_IGNORE;
	    break;
	  default:
	    internal_error();
	    break;
	}
    }
    if (i >= number_members && Ar_curmoff == Ar_memoff)
	return;

    e = NLSMSG(ARCHIVE_BAD4,
	"%1$s: 0711-218 SEVERE ERROR: Archive file %2$s cannot be\n"
	"\tprocessed completely. The number of members (%3$d), as specified\n"
	"\tby the archive member table, is not consistent with the number\n"
	"\tof member headers found (%4$d).");
    v = number_members;
    v2 = i;

  report_error2:
    /* We can't read the rest of the archive.  We ignore the rest of the
       members */
    while (i++ < number_members)
	(xo++)->o_type = O_T_IGNORE;

  report_error1:
    bind_err(SAY_NORMAL, RC_SEVERE, e,
	     Main_command_name, get_object_file_name(archive_obj), v, v2);
    return;			/* No objects here */
} /* allocate_archive_objects */
/* ***********************************
 * NAME:	read_archive_symbols()
 *
 * PURPOSE:	Read the SYMBOLs found in an archive
 *		First see if an object is a shared object or script file,
 *		or if "setopt keepall" is set, process normally.  If not
 *		all members are processed, read names from the global symbol
 *		table (skipping over names read from shared objects).
 *
 * GIVEN:	Object pointer of archive
 *		(If nested archives are supported, we'll also want the
 *		 base_offset to the archive.)
 * ***********************************/
RETCODE
read_archive_symbols(OBJECT *o)
{
    char	*gst_name_ptr;
    char	*last_gst_byte;
    int		i, l, halves;
    long	gst_size, *gst_ptr, num_gst_syms, number_members;
    off_t	member_offset, gst_header_offset;
    AR_HDR	*gst_hdr;
    AR_HDR	temp_ar_hdr;
    OBJECT	*obj, *gst_object, *member_object, *archive_object;
    OBJ_MEMBER_INFO *mem_info_base;
    SYMBOL	dummy_symbol, *prev_sym = &dummy_symbol;
    SYMBOL	*sym;
    STR		*name_ptr;
    int		archive_symbols_found = 0;

    int		lm_num = -1;
    int		use_global_symtab = 0;
    int		save_sym_count = total_symbols_allocated();
    long	last_member_offset = -1;
    IFILE	*ifile = o->o_ifile;

#ifdef DEBUG
    int		miss_count = 0;
#endif

#ifdef READ_FILE
    int		read_file = ifile->i_access == I_ACCESS_READ;
#endif

    number_members = o->o_num_subobjects;
    archive_object = o;			/* Save archive OBJECT */
    archive_object->o_gst_syms = NULL;
    o = o->o_next;			/* Skip over archive OBJECT */

    /* Check first member for global symbol table */
    if (o->o_type == O_T_ARCHIVE_SYMTAB) {
	gst_object = o;
	o = o->o_next;		/* Skip over global symtab */
    }
    else
	gst_object = NULL;		/* No global symtab */

    /* First, we process script files or shared objects, or if the global
       symbol table (GST) does not exist, all files. */
    for (obj = o; obj && obj->o_ifile == o->o_ifile; obj = obj->o_next) {
	switch(obj->o_type) {
	  case O_T_IGNORE:
	    break;
	  case O_T_SCRIPT:	/* read script file() */
	    member_offset = obj->o_member_info->o_ohdr_off
		+ AR_HSZ + ROUND(obj->o_member_info->o_member->len, 2);
	    read_impexp_object(obj, 1 /*import*/, member_offset);
	    break;
	  case O_T_SHARED_OBJECT:
	    member_offset = obj->o_member_info->o_ohdr_off
		+ AR_HSZ + ROUND(obj->o_member_info->o_member->len, 2);
	    read_shared_object_symbols(obj, member_offset);
	    break;
	  case O_T_OBJECT:
	    if (gst_object && Switches.keepall == 0)
		use_global_symtab = 1;	/* read names below */
	    else {
		member_offset = obj->o_member_info->o_ohdr_off
		    + AR_HSZ + ROUND(obj->o_member_info->o_member->len, 2);
		read_object_symbols(obj, member_offset);
	    }
	    break;
	  default:
	    internal_error();
	    break;
	}
	if (interrupt_flag) {
	    bind_err(SAY_NORMAL, RC_NI_WARNING,
		     NLSMSG(MAIN_INTERRUPT,
	    "%1$s: 0711-635 WARNING: Binder command %2$s interrupted."),
		     Main_command_name, Command_name);
	    bind_err(SAY_NORMAL, RC_WARNING,
		     NLSMSG(ARCHIVE_INTERRUPT1,
    "%1$s: 0711-231 WARNING: Reading of archive file %2$s interrupted.\n"
    "\tSome members were not read. Additional errors may occur."),
		     Main_command_name,
		     get_object_file_name(archive_object));
	    return RC_WARNING;
	}
    }

    /* No non-shared objects were found--no need to read the GST. */
    if (use_global_symtab == 0)
	return RC_OK;

#if BASE_OFFSET != 0
    /* Get base offset of archive in case archive is a member */
    base_offset = archive_obj->o_member_info->o_ohdr_off
	+ AR_HSZ + ROUND(archive_obj->o_member_info->o_member->len, 2);
#endif

    /* Use the ARCHIVE global symbol table to read names defined in the
       archive. */
    gst_header_offset = gst_object->o_member_info->o_ohdr_off;

#ifdef READ_FILE
    if (read_file) {
	/* We have already ensured that enough bytes exist in the file to
	   read the global symbol table header. */
	if (fseek_read(ifile, gst_header_offset, &temp_ar_hdr, AR_HSZ) != 0)
	    goto read_error;

	/* The length of the name of the archive global symbol table should
	   be 0, and the code assumes that the symbol table starts
	   immediately after the fixed-size portion of the member header.
	   If the name length is not 0, other errors are likely to occur. */

	gst_size = atol(temp_ar_hdr.ar_size);

	if (gst_size < 4
	    || gst_size > archive_object->o_size
		- ((gst_header_offset - BASE_OFFSET) + AR_HSZ))
	    goto gst_size_error;

	/* Now read in the global symbol table */
	gst_ptr = alloca(gst_size);

	if (safe_fread(gst_ptr, gst_size, ifile) != 0) {
	  read_error:
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(ARCHIVE_BAD_GST3,
	    "%1$s: 0711-207 SEVERE ERROR: Archive file %2$s cannot be\n"
	    "\tprocessed completely. The global symbol table cannot be read."),
		     Main_command_name, get_object_file_name(archive_object));
	    return RC_SEVERE;
	}
    }
    else
#endif
    {
	gst_hdr = (AR_HDR *)(o->o_ifile->i_map_addr
			     + gst_object->o_member_info->o_ohdr_off);

	/* The length of the name of the archive global symbol table should
	   be 0, and the code assumes that the symbol table starts
	   immediately after the fixed-size portion of the member header.
	   If the name length is not 0, other errors are likely to occur. */

	gst_size = atol(gst_hdr->ar_size);
	if (gst_size < 4		/* Count of symbols must exist */
	    || gst_size > archive_object->o_size
	    - ((gst_header_offset - BASE_OFFSET) + AR_HSZ)) {
	  gst_size_error:
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(ARCHIVE_BAD_GST2,
  "%1$s: 0711-206 SEVERE ERROR: Archive file %2$s cannot be\n"
  "\tprocessed completely. The length of the global symbol table (%3$d)\n"
  "\tis less than 4 or is not consistent with the length of the archive."),
		     Main_command_name, get_object_file_name(archive_object),
		     gst_size);
	    return RC_SEVERE;
	}

	gst_ptr = (long *)(&gst_hdr[1]);
    }

    last_gst_byte = (char *)gst_ptr + gst_size;
    num_gst_syms = *gst_ptr++;
    gst_name_ptr = (char *)(&gst_ptr[num_gst_syms]);

    save_sym_count = num_gst_syms - (total_symbols_allocated()-save_sym_count);
    if (save_sym_count > 0)
	Size_estimates.num_symbols += save_sym_count;

    /* When the archive objects were allocated, they were allocated
       contiguously.  So too were the member_info structures.  Therefore,
       they can be treated as arrays. */
    obj = gst_object->o_next;		/* First member */
    mem_info_base = obj->o_member_info;
    for (i = 0; i < num_gst_syms; i++, gst_ptr++) {
	if (interrupt_flag) {
	    bind_err(SAY_NORMAL, RC_NI_WARNING,
		     NLSMSG(MAIN_INTERRUPT,
	    "%1$s: 0711-635 WARNING: Binder command %2$s interrupted."),
		     Main_command_name, Command_name);
	    bind_err(SAY_NORMAL, RC_WARNING,
		     NLSMSG(ARCHIVE_INTERRUPT2,
    "%1$s: 0711-232 WARNING: Reading of archive file %2$s interrupted.\n"
    "\tSome symbols from the global symbol table were not read.\n"
    "\tAdditional errors may occur."),
		     Main_command_name,
		     get_object_file_name(archive_object));
	    return RC_WARNING;
	}

	if (gst_name_ptr >= last_gst_byte) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(ARCHIVE_BAD_SYM_COUNT,
	  "%1$s: 0711-209 SEVERE ERROR: Archive file %2$s cannot be\n"
	  "\tprocessed completely. The global symbol table should\n"
	  "\tcontain %3$d names. Only %4$d names were found."),
		     Main_command_name, get_object_file_name(archive_object),
		     num_gst_syms, archive_symbols_found);
	    return RC_SEVERE;
	}

	/* Find object with this member offset */
	/* NOTE:  We expect most archives to have symbols in member order,
	   so a linear search of names in the member table is most efficient
	   if the offset doesn't match the offset of the previous symbol.  */
	if (last_member_offset != *gst_ptr) {
	    l = lm_num;
	    halves = 2;
	    /* Search from last member to end of list */
	    if (last_member_offset < *gst_ptr) {
	      high_half:
		for (lm_num = l + 1; lm_num < number_members; lm_num++) {
		    if (mem_info_base[lm_num].o_ohdr_off == *gst_ptr)
			goto found;
#ifdef DEBUG
		    miss_count++;
#endif
		}
		if (--halves == 0)
		    goto not_found;
	    }
	    /* Search from last member to beginning of list */
	    for (lm_num = l - 1; lm_num >= 0; lm_num--) {
		if (mem_info_base[lm_num].o_ohdr_off == *gst_ptr)
		    goto found;
#ifdef DEBUG
		miss_count++;
#endif
	    }
	    if (--halves != 0)
		goto high_half;
	  not_found:
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(ARCHIVE_BAD_GST,
    "%1$s: 0711-205 SEVERE ERROR: Archive file %2$s cannot be\n"
    "\tprocessed completely. Offset %3$d, found in the global symbol\n"
    "\ttable for symbol %4$s, does not match the offset of any member."),
		     Main_command_name,
		     get_object_file_name(archive_object),
		     *gst_ptr,
		     gst_name_ptr);
	    lm_num = l;			/* restore value */
	    gst_name_ptr += strlen(gst_name_ptr) + 1;
	    continue;			/* Ignore symbol */

	  found:
	    last_member_offset = *gst_ptr;
	    member_object = &obj[lm_num];
	}
	if (member_object->o_type == O_T_SHARED_OBJECT) {
	    gst_name_ptr += strlen(gst_name_ptr) + 1;
	    /* Don't use symbols from shared object */
	    continue;
	}

	name_ptr = putstring_len(gst_name_ptr, last_gst_byte - gst_name_ptr);
	name_ptr->flags |= STR_ISSYMBOL;

	gst_name_ptr += name_ptr->len + 1;
#ifdef DEBUG
	if (bind_debug & ARCHIVE_DEBUG)
	    if (bind_debug & DEBUG_LONG)
		say(SAY_NO_NLS,
		    "Object %s:  Symbol: %s",
		    member_object->o_member_info->o_member->name,
		    name_ptr->name);
	    else
		say(SAY_NO_NLS | SAY_NO_NL, ".");
#endif

	++archive_symbols_found;

	sym = create_global_archive_SYMBOL(name_ptr, member_object);
	sym->s_typechk = NULL;
	sym->s_smtype = XTY_AR;	/* Symbol from global symbol table */
	sym->s_smclass = XMC_UA;
	sym->s_addr = 0;
	sym->s_flags = S_ARCHIVE_SYMBOL;
	sym->s_next_in_csect = NULL;

	sym->s_prev_in_gst = prev_sym;
	prev_sym->s_next_in_csect = sym;
	prev_sym = sym;

	sym->s_inpndx = INPNDX_ARCHIVE;
    }

    dummy_symbol.s_next_in_csect->s_prev_in_gst = NULL;
    archive_object->o_gst_syms = dummy_symbol.s_next_in_csect;

    DEBUG_MSG(ARCHIVE_DEBUG | DEBUG_LONG,
	      (SAY_NO_NLS, "%d misses in looking up offsets", miss_count));

    DEBUG_MSG(ARCHIVE_DEBUG,
	      (SAY_NO_NLS, "Found %d archive symbols", archive_symbols_found));

    return RC_OK;
}
