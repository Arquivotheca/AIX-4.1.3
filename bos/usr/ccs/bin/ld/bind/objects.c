#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)99	1.11  src/bos/usr/ccs/bin/ld/bind/objects.c, cmdld, bos411, 9428A410j 7/20/94 22:16:52")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: allocate_ifile_objects
 *		allocate_object_info
 *		first_object
 *		get_object_file_name
 *		init_objects
 *		last_object
 *		new_init_member_objects
 *		new_init_object
 *		read_magic_number
 *		reinit_objects
 *		reserve_new_objects
 *		set_ifile_type
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bind.h"
#include "error.h"
#include "global.h"
#include "ifiles.h"
#include "strs.h"

#include "objects.h"
#include "insert.h"
#include "stats.h"

/* Static variables */
static OBJECT	*objects_root;
static OBJECT	*objects_last;
static OBJECT	*free_objects;
static int	next_allocation_objects;
static int	num_free_objects;

static OBJECT_INFO *free_object_info;
static int	next_allocation_object_info;
static int	num_free_object_info;
static char	*str_NOFILE;
static char	*str_UNKNOWN_COMP;
static int	len_UNKNOWN_COMP;

/* ***********************************
 * Name:	init_objects()
 * Purpose:	Initialize OBJECT routines
 * ***********************************/
void
init_objects(void)
{
    str_NOFILE = msg_get(NLSMSG(LIT_NO_FILE, "<no file>"));
    str_UNKNOWN_COMP = msg_get(NLSMSG(LIT_UNKNOWN_COMPOSITE,
				      "UNKNOWN_COMPOSITE(%s[%s])"));
    len_UNKNOWN_COMP = strlen(str_UNKNOWN_COMP);
}
/* ***********************************
 * Name:	first_object()
 * Purpose:	Return pointer to first OBJECT
 * ***********************************/
OBJECT *
first_object(void)
{
    return objects_root;
}
/* ***********************************
 * Name:	last_object()
 * Purpose:	Return pointer to last OBJECT
 * ***********************************/
OBJECT *
last_object(void)
{
    return objects_last;
}
/* ***********************************
 * Name:	reserve_new_objects()
 * Purpose:	Make sure next allocation of an OBJECT allocates "num"
 *		consecutive OBJECTS.
 * Given:	Count of objects to reserve
 * ***********************************/
void
reserve_new_objects(int num)
{
    next_allocation_objects += num;
    next_allocation_object_info += num;
}
/* ***********************************
 * Name:	new_init_member_objects()
 * Purpose:	Return "count" consecutive OBJECTs and initialize them, along
 *		with associated OBJ_MEMBER_INFO structures.  These objects will
 *		be linked after "obj" in chain of objects, and "obj" should
 *		be a composite member.
 *
 * Given:	IFILE pointer, count, OBJECT
 * ***********************************/
OBJECT *
new_init_member_objects(IFILE *f,
			OBJECT *obj,
			int count)
{
    static char		*id = "new_init_member_objects";
    OBJECT		*o;
    OBJ_MEMBER_INFO	*info;
    int			n;

#ifdef DEBUG
    if (!objects_last)
	internal_error();
#endif

    o = get_memory(sizeof(OBJECT), count, OBJECTS_ID, id);
    info = emalloc(sizeof(OBJ_MEMBER_INFO) * count, id);
    STAT_use(OBJECTS_ID, count);

    /* Initialize requested OBJECTs */
    for (n = 0; n < count; n++) {
	o[n].o_next = &o[n+1];	/* Link OBJECTs */
	o[n].o_ifile = f;
	o[n].o_size = 0;
	o[n].o_member_info = &info[n];
	info[n].o_parent = obj;
	o[n].o_num_subobjects = 0;
	o[n].o_type = O_T_IGNORE;
	o[n].o_contained_in_type = O_T_ARCHIVE;
    }

    o[count-1].o_next = obj->o_next;
    obj->o_next = o;

    if (objects_last == obj)
	objects_last = &o[count-1];

    return o;
}
/* ***********************************
 * Name:	new_init_object()
 * Purpose:	Return a pointer to an initialized OBJECT.
 * Given:	IFILE pointer for file containing object
 * ***********************************/
OBJECT *
new_init_object(IFILE *f)
{
    OBJECT *o;

    if (num_free_objects == 0) {
	if (next_allocation_objects > 0) {
	    num_free_objects = next_allocation_objects;
	    next_allocation_objects = 0;
	}
	else
	    num_free_objects = 1;
	free_objects = get_memory(sizeof(OBJECT), num_free_objects,
				  OBJECTS_ID, "new_init_object");
    }

    o = free_objects++;
    num_free_objects--;

    STAT_use(OBJECTS_ID, 1);

    o->o_next = NULL;
    o->o_ifile = f;
    o->o_member_info = NULL;
    o->o_num_subobjects = 0;
    o->o_type = o->o_contained_in_type = O_T_UNKNOWN;

    if (objects_last)
	objects_last->o_next = o;
    else
	objects_root = o;
    objects_last = o;

    return o;
}
/* ***********************************
 * Name:	get_object_file_name()
 *
 * Purpose:	Return printable name of an OBJECT in a static buffer.  This
 *		buffer may be freed by subsequent calls to get_object_file_name
 *
 *	NOTE:  Nested composite files are not handled by this routine
 *		(because they are not allowed).
 *
 * Given:	OBJECT pointer
 * ***********************************/
char *
get_object_file_name(OBJECT *object)
{
    static char		*id = "get_object_file_name";
    static char		*buf = NULL;
    static char 	*last_returned_value;
    static int		buf_len = 0;
    static OBJECT	*prev_obj = NULL;
    int			new_len;
    char		*s;
    STR 		*filename_str;

    if (object == prev_obj)
	return last_returned_value;
    else
	prev_obj = object;

    if (object->o_ifile == NULL)
	return last_returned_value = str_NOFILE;

    filename_str = object->o_ifile->i_name;

    if (object->o_member_info == NULL)
	return last_returned_value = filename_str->name;

    switch(object->o_contained_in_type) {
      case O_T_ARCHIVE:
	s = "%s[%s]";
	new_len = strlen("%s[%s]");
	break;
      default:
	s = str_UNKNOWN_COMP;
	new_len = len_UNKNOWN_COMP;
    }
    new_len += - 4			/* Subtract 4 for two %s occurrences */
	+ filename_str->len + object->o_member_info->o_member->len
	    + 1 /* for terminating '\0' */;

    if (new_len > buf_len || buf_len > new_len * 64) {
	/* Reallocate buffer if it is too small or larger than needed. */
	if (buf)
	    efree(buf);
	buf = emalloc(new_len * 8, id); /* Allocate extra */
	buf_len = new_len * 8;
    }

    (void) sprintf(buf, s,
		   filename_str->name, object->o_member_info->o_member->name);

    return last_returned_value = buf;
}
/* ***********************************
 * Name:	allocate_object_info()
 * Purpose:	Allocate structures for "real" objects.
 * Given:	OBJECT pointer, FILHDR, base_offset
 * ***********************************/
void
allocate_object_info(OBJECT *obj,
		     FILHDR *hdr,
		     off_t base_offset)
{
    if (num_free_object_info == 0) {
	if (next_allocation_object_info > 0) {
	    num_free_object_info = next_allocation_object_info;
	    next_allocation_object_info = 0;
	}
	else
	    num_free_object_info = 1;
	free_object_info = get_memory(sizeof(OBJECT_INFO),
				      num_free_object_info,
				      OBJECT_INFO_ID, "new_object_info");
    }

    obj->o_info = free_object_info++;
    num_free_object_info--;
    STAT_use(OBJECT_INFO_ID, 1);

    obj->oi_srcfiles = NULL;
    obj->oi_ext_refs = NULL;
    obj->oi_syms_lookup = NULL;
    obj->oi_flags = 0;

    /* Save information from the XCOFF header */
    obj->oi_num_sections = hdr->f_nscns;
    obj->oi_num_symbols = hdr->f_nsyms;
    obj->oi_symtab_offset = base_offset + (uint32)(hdr->f_symptr);
    obj->oi_sections_offset = FILHSZ + hdr->f_opthdr;

    Size_estimates.num_sections += hdr->f_nscns;
}
/* ***********************************
 * Name:	allocate_ifile_objects()
 * Purpose:	Allocate OBJECTs contained in an IFILE.
 * Given:	IFILE pointer
 * ***********************************/
void
allocate_ifile_objects(IFILE *file,
		       HEADERS *hdr)
{
    OBJECT	*o;
    int		nsyms;

    o = new_init_object(file);
    file->i_objects = o;
    o->o_size = file->i_filesz;
    switch (file->i_type) {
      case I_T_OBJECT:
	o->o_type = O_T_OBJECT;
	allocate_object_info(o, &hdr->xcoff_hdr, 0);

	Size_estimates.num_srcfiles++; /* One SRCFILE per OBJECT */
	if (hdr->xcoff_hdr.f_nsyms > 4)
	    /* Assume a debug section */
	    nsyms = hdr->xcoff_hdr.f_nsyms/8;	/* Wild guess */
	else
	    nsyms = hdr->xcoff_hdr.f_nsyms/2;
	Size_estimates.num_csects += nsyms;
	Size_estimates.num_symbols += nsyms;
	break;

      case I_T_SHARED_OBJECT:
	o->o_type = O_T_SHARED_OBJECT;

	/* Update estimates */
	Size_estimates.num_srcfiles++; /* One SRCFILE for shared obj */

	/* Save information from the XCOFF header */
	o->o_num_sections = hdr->xcoff_hdr.f_nscns;
	o->o_opt_hdr_size = hdr->xcoff_hdr.f_opthdr;
	break;

      case I_T_ARCHIVE:
	allocate_archive_objects(file, o, &hdr->archive_hdr, 0);
	break;

      case I_T_SCRIPT:
	o->o_type = O_T_SCRIPT;
	o->o_srcfiles = NULL;
	Size_estimates.num_csects++;
	Size_estimates.num_symbols += o->o_size/32;
	break;
      case I_T_EXPORT:
	o->o_type = O_T_EXPORT;
	o->o_srcfiles = NULL;
	Size_estimates.num_csects++;
	Size_estimates.num_symbols += o->o_size/32;
	break;

      case I_T_UNKNOWN:
      case I_T_IGNORE:
      default:
	internal_error();
    }
} /* allocate_ifile_objects */
/***********************************************************************
 * Name:	read_magic_number()
 * Purpose:	Determine the type of an object file
 * Given:	IFILE *ifile:	Pointer to source file structure.  File must
 *				be open and mapped.
 *		FILHDR **hdr:	OUT: Pointer to header of file or member being
 *				read, if it is an XCOFF file.
 *
 *				If we are reading the	If we are reading a
 *				file for the first	member of a composite
 *				time:			structure:
 *                              ======================= ======================
 *	OBJECT *obj:		NULL			Pointer to structure
 *							for member being read
 *	off_t base_offset	0			Offset (in file) to the
 *							beginning of the member
 *	long size		Length of the file	Length of the member
 * Returns:	O_T_UNKNOWN: File or member type unknown.  No message printed.
 *		O_T_IGNORE:  Innocuous error--no sections, stripped, etc., OR
 *				a read error occurred.
 *				A message is printed.
 *		O_T_TRUNCATED: The file type can be determined from the
 *				magic number, but the file is too short to
 *				be valid.  No message is printed.
 *		Otherwise:	The proper type is returned.
 ***********************************************************************/
OBJECT_TYPE_T
read_magic_number(IFILE *ifile,
		  HEADERS **hdr,
		  OBJECT *obj,
		  off_t base_offset,
		  long size)
{
    FILHDR	*Filhdr;
    size_t	read_size;
    static HEADERS	headers;

    if (size == 0) {
	if (obj)
	    bind_err(SAY_NORMAL, RC_ERROR,
		     NLSMSG(MEMBER_EMPTY,
		    "%1$s: 0711-716 ERROR: Archive member %2$s is empty.\n"
		    "\tThe member is being ignored."),
		     Main_command_name, get_object_file_name(obj));
	else
	    bind_err(SAY_NORMAL, RC_ERROR,
		     NLSMSG(IFILE_EMPTY,
			    "%1$s: 0711-711 ERROR: Input file %2$s is empty.\n"
			    "\tThe file is being ignored."),
		     Main_command_name, ifile->i_name->name);
	return O_T_IGNORE;
    }
    else if (size < sizeof(Filhdr->f_magic)) {
	/* File must be as long a magic number (2 bytes) */
	return O_T_TRUNCATED;
    }

#ifdef READ_FILE
    if (ifile->i_access == I_ACCESS_READ) {
	/* Read the magic number--then the header */
	read_size = min(size, sizeof(headers));
	if (fseek_read(ifile, base_offset, &headers, read_size) != 0)
	    return O_T_IGNORE;
	*hdr = &headers;
    }
    else
#endif
	*hdr = (HEADERS *)(ifile->i_map_addr + base_offset);

    Filhdr = &(*hdr)->xcoff_hdr;

    switch (Filhdr->f_magic) {
      case U802WRMAGIC:
      case U802ROMAGIC:
      case U802TOCMAGIC:
      case U800WRMAGIC:
      case U800ROMAGIC:
      case U800TOCMAGIC:
	/* Check for existence of primary, auxiliary, and
	   section headers. */
	if (size < FILHSZ + Filhdr->f_opthdr + SCNHSZ * Filhdr->f_nscns)
	    return O_T_TRUNCATED;

	if (Filhdr->f_nscns == 0) {
	    if (Switches.verbose)
		if (obj)
		    bind_err(SAY_NORMAL, RC_WARNING, 
			     NLSMSG(MEMBER_NO_SECTIONS,
	    "%1$s: 0711-718 WARNING: Archive member %2$s\n"
	    "\tdoes not have any sections. The member is being ignored."),
			     Main_command_name, get_object_file_name(obj));
		else
		    bind_err(SAY_NORMAL, RC_WARNING, 
			     NLSMSG(OBJECT_NO_SECTIONS,
	    "%1$s: 0711-717 WARNING: Object file %2$s\n"
	    "\tdoes not have any sections. The file is being ignored."),
			     Main_command_name, ifile->i_name->name);
	    return O_T_IGNORE;
	}

	if (Switches.autoimp && (Filhdr->f_flags & F_SHROBJ))
	    return O_T_SHARED_OBJECT;

	if ((Filhdr->f_flags & F_RELFLG) || Filhdr->f_nsyms == 0) {
	    if (obj) {
		/* For a loadonly object, return O_T_OBJECT.  The member
		   will be ignored in the archive reading code. */
		if (Filhdr->f_flags & F_LOADONLY)
		    return O_T_OBJECT;
		bind_err(SAY_NORMAL, RC_ERROR,
			 NLSMSG(MEMBER_STRIPPED,
		"%1$s: 0711-712 ERROR: Archive member %2$s\n"
		"\tis stripped. The member is being ignored."),
			 Main_command_name, get_object_file_name(obj));
	    }
	    else
		bind_err(SAY_NORMAL, RC_ERROR,
			 NLSMSG(OBJECT_STRIPPED,
			"%1$s: 0711-710 ERROR: Input file %2$s\n"
			"\tis stripped. The file is being ignored."),
			 Main_command_name, ifile->i_name->name);
	    return O_T_IGNORE;
	}

	return O_T_OBJECT;

      case 0x2321:	/* script "#! ..." */
	return O_T_SCRIPT;

      case 0x3C61:	/* archive "<aiaff>\n" */
	if (size < FL_HSZ)
	    return O_T_TRUNCATED;
	if (strncmp(((FL_HDR *)Filhdr)->fl_magic, AIAMAG, SAIAMAG) == 0)
	    return O_T_ARCHIVE;
	break;
    }
    return O_T_UNKNOWN;
}
/* ***********************************
 * Name:	set_ifile_type()
 * Purpose:
 * Given:	File structure pointer
 * Returns:	I_T_UNKNOWN for erroneous files
 *		I_T_IGNORE for other errors or warnings
 *		A valid file type, otherwise.
 * ***********************************/
IFILE_TYPE_T
set_ifile_type(IFILE *ifile,
	       HEADERS **hdr)
{
    switch(read_magic_number(ifile, hdr,
			     NULL /* no containing object */,
			     0, /* base_offset */
			     ifile->i_filesz)) {
      case O_T_UNKNOWN:
	bind_err(SAY_NORMAL, RC_ERROR,
		 NLSMSG(IFILE_UNKNOWN_TYPE,
	"%1$s: 0711-715 ERROR: File %2$s cannot be processed.\n"
	"\tThe file must be an object file, an import file, or an archive."),
		 Main_command_name, ifile->i_name->name);
	ifile_close_for_good(ifile);
	return I_T_UNKNOWN;

      case O_T_TRUNCATED:
	bind_err(SAY_NORMAL, RC_ERROR,
		 NLSMSG(FILE_TRUNCATED,
		"%1$s: 0711-161 ERROR: File %2$s cannot be processed.\n"
		"\tThe file was found to be truncated and is being ignored."),
		 Main_command_name,
		 ifile->i_name->name);
	ifile_close_for_good(ifile);
	return I_T_UNKNOWN;

      case O_T_IGNORE:
	/* Returned if read_magic_number encountered a problem.
	   Any required message will have already been printed. */
	ifile_close_for_good(ifile);
	return I_T_IGNORE;

      case O_T_OBJECT:
	ifile->i_type = I_T_OBJECT;
	break;
      case O_T_SHARED_OBJECT:
	ifile->i_type = I_T_SHARED_OBJECT;
	break;
      case O_T_ARCHIVE:
	ifile->i_type = I_T_ARCHIVE;
	break;
      case O_T_SCRIPT:
	ifile->i_type = I_T_SCRIPT;
	break;

	/* Shouldn't be returned by read_magic_number() */
      case O_T_EXPORT:
      case O_T_ARCHIVE_SYMTAB:
      default:
	internal_error();
    }
#ifdef DEBUG
    if (bind_debug & IFILES_DEBUG) {
	extern char *get_ifile_type_string(IFILE_TYPE_T);
	say(SAY_NO_NLS, "File type is:%s",
	    get_ifile_type_string(ifile->i_type));
    }
#endif
    return ifile->i_type;
}
