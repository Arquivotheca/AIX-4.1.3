#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)91	1.25  src/bos/usr/ccs/bin/ld/bind/dump_cmd.c, cmdld, bos41B, 9504A 12/7/94 15:45:10")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS (for binder commands)
 *		dump_cmd
 *
 *   FUNCTIONS:
 *		get_ifile_type_string
 *		dump_csect
 *		dump_rld
 *
 *   STATIC FUNCTIONS:
 *		convert_args
 *		dump_csects
 *		dump_csects_by_name
 *		dump_csect_by_number
 *		dump_csect_with_name
 *		dump_er
 *		dump_er1
 *		dump_ers
 *		dump_ers_by_name
 *		dump_ers_by_number
 *		dump_flags
 *		dump_ifile
 *		dump_ifiles
 *		dump_ifiles1
 *		dump_object_info
 *		dump_object
 *		dump_objects
 *		dump_objects1
 *		dump_rlds
 *		dump_rlds_by_name
 *		dump_rlds_by_number
 *		dump_rlds_for_symbol
 *		dump_rlds_by_STR
 *		dump_numbered_rld
 *		dump_rlds_srcfile
 *		dump_something_by_name
 *		dump_something_by_number
 *		dump_SRCFILE
 *		dump_SRCFILEs
 *		dump_SRCFILEs_by_number
 *		dump_symbol
 *		dump_symbols
 *		dump_symbols_by_name
 *		dump_symbol_with_name
 *		dump_symbol_by_number
 *		dump_symbols_by_STR
 *		dump_TYPECHK
 *		dump_TYPECHKs
 *		dump_TYPECHKs_by_STR
 *		dump_TYPECHKs_by_name
 *		dump_TYPECHKs_by_number
 *		dump_TYPECHKs_with_name
 *		dump_unrefs
 *		dump_unrefs_by_name
 *		dump_xref_by_name
 *		find_numbered_object
 *		get_sf_inpndx
 *		get_o_number
 *		get_object_type
 *		print_STR_name_and_flags
 *		get_sectnum
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "bind.h"
#include "global.h"
#include "strs.h"
#include "error.h"

#include "dump.h"
#include "ifiles.h"
#include "match.h"
#include "save.h"
#include "objects.h"
#include "util.h"

/* Union for arguments to dump functions. */
typedef union {
    char	*c_arg;
    int		n_arg;
} arg_t;

/* Note:  The source code in this file is only used for the dump command
   (except for dump_csect() and dump_rld(), which are only called from
   code_save.c if dbg_opt7 is set).

   Therefore, no translation is provided for anything printed here.
   There should be no NLSMSG macros in this file.  */

/* Static variables */
static int something_printed;

/* Forward declarations */
static RETCODE dump_something_by_name(char *, RETCODE (*)(), char *, char *);
static RETCODE
    dump_ifiles(arg_t []),		dump_ifiles1(void),
    dump_objects(arg_t []),		dump_objects1(void),
    dump_SRCFILEs_by_number(arg_t []),	dump_SRCFILEs(void),
    dump_csect_by_number(arg_t []),
    dump_csects_by_name(arg_t []),	dump_csects(void),
    dump_TYPECHKs_by_name(arg_t []),	dump_TYPECHKs(void),
    dump_TYPECHKs_by_number(arg_t []),
    dump_symbols_by_name(arg_t []),	dump_symbols(void),
    dump_symbol_by_number(arg_t []),
    dump_unrefs_by_name(arg_t []), dump_unrefs(void),
    dump_ers_by_number(arg_t []), dump_ers_by_name(arg_t []), dump_ers(void),
    dump_xref_by_name(arg_t []),
    dump_rlds_by_number(arg_t []),dump_rlds_by_name(arg_t []),dump_rlds(void),
    dump_numbered_rld(arg_t []);

/* Table of dump subcommands:  Each subcommand can be abbreviated by
   its first letter (or two letters, for subcommands beginning with '#').
   The "args" field describes the expected arguments.  Upper case
   letters denote required arguments, while lower case letters denote
   optional arguments.  Even for commands with required arguments, no
   arguments at all can be supplied, so the first or only argument can be
   specified with either an upper case or a lower case letter.  The defined
   letters are:
	n: a number
	d: a decimal number (not converted by convert_args())
	p: a positive number
	a: a symbol name
    If no arguments are supplied, the "fun_no_args" routine is called.
    Otherwise, the "function" is called.
*/
static struct {
    char *name;
    char *args;
    RETCODE (*function)();
    RETCODE (*fun_no_args)(void);
} dump_commands[] = {
    {"ifiles",	"nn",	dump_ifiles,		dump_ifiles1},
    {"files",	"A",	NULL,			dump_SRCFILEs},
    {"#files",	"d",	dump_SRCFILEs_by_number,dump_SRCFILEs},
    {"objects",	"dp",	dump_objects,		dump_objects1},
    {"csects",	"A",	dump_csects_by_name,	dump_csects},
    {"#csects",	"N",	dump_csect_by_number,	dump_csects},
    {"symbols",	"A",	dump_symbols_by_name,	dump_symbols},
    {"#symbols","N",	dump_symbol_by_number,	dump_symbols},
    {"typchks",	"A",	dump_TYPECHKs_by_name,	dump_TYPECHKs},
    {"#typchks","N",	dump_TYPECHKs_by_number,dump_TYPECHKs},
    {"rlds",	"A",	dump_rlds_by_name,	dump_rlds},
    {"#rlds",	"N",	dump_rlds_by_number,	dump_rlds},
    {"#nrlds",	"N",	dump_numbered_rld,	NULL},
    {"ers",	"A",	dump_ers_by_name,	dump_ers},
    {"unrefs",	"A",	dump_unrefs_by_name,	dump_unrefs},
    {"#ers",	"N",	dump_ers_by_number,	dump_ers},
    {"xref",	"A",	dump_xref_by_name,	NULL/*dump_dont*/},
#if 0
    {"#xref",	"N",	dump_xref_by_number,	NULL/*dump_dont*/},
#endif
};

/************************************************************************
 * Name: get_ifile_type_string
 *									*
 * PURPOSE: Converts a file type into printable form.
 *
 ************************************************************************/
char *
get_ifile_type_string(IFILE_TYPE_T ft)
{
    switch(ft) {
      case I_T_UNKNOWN:		return "Unknown";
      case I_T_IGNORE:		return "Invalid-file(being ignored)";
      case I_T_SCRIPT:		return "Script";
      case I_T_EXPORT:		return "Export-file";
      case I_T_OBJECT:		return "Object";
      case I_T_SHARED_OBJECT:	return "Shared-Obj";
      case I_T_ARCHIVE:		return "Archive";
    }
    return "I_T_???";
}
/************************************************************************
 * Name: get_object_type
 *									*
 * Purpose: Converts an object type into printable form.
 *
 ************************************************************************/
static char *
get_object_type(unsigned char k)
{
    switch(k) {
      case O_T_IGNORE:	 	return "Ignored";
      case O_T_UNKNOWN:	 	return "Unknown";
      case O_T_SCRIPT:	 	return "Script";
      case O_T_EXPORT:	 	return "Export-file";
      case O_T_OBJECT:	 	return "Object";
      case O_T_SHARED_OBJECT:	return "Shared-Obj";
      case O_T_ARCHIVE:	 	return "Archive";
      case O_T_ARCHIVE_SYMTAB:	return "Archive-Symtab";
    }
    return "O_T_???";
}
/************************************************************************
 * Name: get_sectnum
 *									*
 * Purpose: Converts a section number into printable form.
 *
 ************************************************************************/
static char *
get_sectnum(int sect)
{
    static char	buf[20];

    switch(sect) {
      case N_IMPORTS:	return "IMPORTS";
      case N_GENERATED: return "GENERATED";
      case N_UNKNOWN:	return "UNKNOWN";
      case N_FIXUP:	return "FIXUP";

      default:
	sprintf(buf, "%d", sect);
	return buf;
    }
}
/************************************************************************
 * Name: get_sf_inpndx
 *									*
 * Purpose: Converts an input_index from a SRCFILE structure into
 *	printable form.
 *									*
 ************************************************************************/
static char *
get_sf_inpndx(int inpndx,
	      char *buf)
{
    if (inpndx == SF_GENERATED_INPNDX)
	return "GENERATED";
    else
	sprintf(buf, "%d", inpndx);
    return buf;
}
/************************************************************************
 * Name: find_numbered_object
 *									*
 * Purpose: Return the OBJECT with number fnum.onum.  If File fnum has
 *	no members, onum is ignored.  If the specified object is not found,
 *	an error is printed and NULL is returned.
 *
 * Returns: The object.
 ************************************************************************/
static OBJECT *
find_numbered_object(int fnum, int onum)
{
    IFILE *ifile;
    OBJECT *o;
    int n;

    for (ifile = first_ifile, n = 1; n < fnum && ifile; ifile = ifile->i_next)
	n++;
    if (ifile == NULL || fnum < 1) {
	say(SAY_NO_NLS, "Input file number %d does not exist.", fnum);
	return NULL;
    }

    o = ifile->i_objects;

    if (o == NULL) {
	say(SAY_NO_NLS, "Input file %d has no associated object.", fnum);
	return o;
    }

    /* Ignore onum if object has no subobjects. */
    if (o->o_num_subobjects && onum >= 0) {
	if (onum > o->o_num_subobjects - 1) {
	    say(SAY_NO_NLS, "Object %d.%d is last in file %s.",
		n, o->o_num_subobjects - 1, ifile->i_name->name);
	    return NULL;
	}
	o = o->o_next;
	/* Objects in an archive are allocated consecutively. */
	o = o + onum;
    }
    return o;
}
/************************************************************************
 * Name: convert_args
 *									*
 * Purpose: Copy arguments from oargs to nargs, converting options as
 *	requested under control of the template. The template characters
 *	are described above in the dump subcommand table.
 *
 ************************************************************************/
static RETCODE
convert_args(arg_t nargs[],
	     char *oargs[],
	     char *template)
{
    int		n;

    for ( ; *template; template++) {
	if (isupper(*template) && *oargs == NULL) {
	    say(SAY_NO_NLS, "SYNTAX ERROR: %s: Missing argument",
		Command_name);
	    return RC_NI_WARNING;
	}
	switch(tolower(*template)) {
	  case 'a':			/* Symbol name */
	  case 'd':			/* Decimal number (not converted) */
	    if (*oargs == NULL)
		(nargs++)->c_arg = NULL;
	    else if (*oargs[0] == ',' && *oargs[1] == '\0') {
		(nargs++)->c_arg = NULL;
		oargs++;
	    }
	    else
		(nargs++)->c_arg = *oargs++;
	    break;
	  case 'n':
	    if (*oargs)
		if (*oargs[0] == '*' && *oargs[1] == '\0')
		    (nargs++)->n_arg = 0x7FFFFFFF;
		else
		    (nargs++)->n_arg = atoi(*oargs++);
	    else
		(nargs++)->n_arg = 0x80000000;
	    break;
	  case 'p':
	    if (*oargs) {
		if (*oargs[0] == '*' && *oargs[1] == '\0')
		    n = 0x7FFFFFFF;
		else {
		    n = atoi(*oargs++);
		    if (n < 0) {
			say(SAY_NO_NLS, "Number (%d) must be non-negative", n);
			return RC_NI_WARNING;
		    }
		}
		(nargs++)->n_arg = n;
	    }
	    else
		(nargs++)->n_arg = 0x80000000;
	    break;
	  default:
	    internal_error();
	}
    }
    return RC_OK;
} /* convert_args */
/************************************************************************
 * Name: dump_cmd
 *									*
 * Purpose: Print symbolic representation of insternal structures.
 *
 * Command Format:							*
 *	DUMP dump-type [arg ...]
 *
 * Arguments:
 *	Valid dump_types are found in the dump_commands array defined above.
 *
 ************************************************************************/
int
dump_cmd(char *arg[])			/* argv-style arguments */
{
    char	*id = "DUMP";
    int		i;
    int		n,
		ac = 1,
		rc;

    arg_t *new_args;

    lower(arg[1]);

    if (arg[1][0] == '#' && arg[1][1] == '\0') {
	bind_err(SAY_NO_NLS, RC_NI_WARNING,
	 "Ambiguous dump subcommand: #.  Specify at least two characters");
	return RC_NI_WARNING;
    }
    for (i=0; i < sizeof(dump_commands)/sizeof(dump_commands[0]); i++) {
	if (strncmp(arg[1], dump_commands[i].name, strlen(arg[1])) == 0) {
	    n = strlen(dump_commands[i].args);
	    if (arg[2] == NULL)
		if (dump_commands[i].fun_no_args) {
		    rc = dump_commands[i].fun_no_args();
		    if (rc == RC_ABORT && interrupt_flag) {
			bind_err(SAY_NO_NLS, RC_OK, "%s interrupted.",
				 Command_name);
			rc = RC_OK;
		    }
		    return rc;
		}
		else {
		    bind_err(SAY_NO_NLS, RC_NI_WARNING,
			     "Dump subcommand %s requires arguments.", arg[1]);
		    return RC_NI_WARNING;
		}
	    if (n)
		new_args = emalloc(n * sizeof(*new_args), id);
	    else
		new_args = NULL;
	    rc = convert_args(new_args, &arg[ac+1], dump_commands[i].args);
	    if (rc == RC_OK) {
		if (dump_commands[i].function)
		    rc = dump_commands[i].function(new_args);
		else {
		    bind_err(SAY_NO_NLS, RC_NI_WARNING,
			     "Dump subcommand %s cannot have arguments.",
			     arg[1]);
		    return RC_NI_WARNING;
		}
	    }
	    if (n)
		efree(new_args);
	    if (rc == RC_ABORT && interrupt_flag) {
		bind_err(SAY_NO_NLS, RC_OK, "%s interrupted.", Command_name);
		rc = RC_OK;
	    }
	    return rc;
	}
    }
    bind_err(SAY_NO_NLS, RC_NI_WARNING, "Unknown dump subcommand");
    return RC_NI_WARNING;
} /* dump_cmd */
/************************************************************************
 * Name: print_STR_name_and_flags
 *									*
 * Purpose: Print symbolic representation of a STR name and its flags.
 *
 ************************************************************************/
static void
print_STR_name_and_flags(STR *s)
{
    say(SAY_NO_NLS | SAY_NO_NL, "%s", s->name);
    if (s->flags == 0) {
	if (s->str_value)
	    say(SAY_NO_NLS, "\t[value=0x%x (%d)]",
		s->str_value, s->str_value);
	return;
    }

    say(SAY_NO_NLS | SAY_NO_NL, " :");

    if (s->flags & STR_ISSYMBOL)
	say(SAY_NO_NLS | SAY_NO_NL, " ISSYMBOL");
    if (s->flags & STR_NO_DEF)
	say(SAY_NO_NLS | SAY_NO_NL, " NO_DEF");
    if (s->flags & STR_RESOLVED)
	say(SAY_NO_NLS | SAY_NO_NL, " RESOLVED");
    if (s->flags & STR_TOC_RESOLVED)
	say(SAY_NO_NLS | SAY_NO_NL, " TOC_RESOLVED");

    if (s->flags & STR_EXPORT)
	say(SAY_NO_NLS | SAY_NO_NL, " EXPORT");
    if (s->flags & STR_ENTRY)
	say(SAY_NO_NLS | SAY_NO_NL, " ENTRY");
    if (s->flags & STR_IMPORT)
	say(SAY_NO_NLS | SAY_NO_NL, " IMPORT");
    if (s->flags & STR_DS_EXPORTED)
	say(SAY_NO_NLS | SAY_NO_NL, " DS_EXPORTED");

    if (s->flags & STR_ERROR)
	say(SAY_NO_NLS | SAY_NO_NL, " ERROR");
    if (s->flags & STR_SYSCALL)
	say(SAY_NO_NLS | SAY_NO_NL, " SYSCALL");
    if (s->flags & STR_KEEP)
	say(SAY_NO_NLS | SAY_NO_NL, " KEEP");
    if (s->flags & STR_STRING_USED)
	say(SAY_NO_NLS | SAY_NO_NL, " STRING_USED");

    if (s->flags & LOADER_USED)
	say(SAY_NO_NLS | SAY_NO_NL, " L_USED");
    if (s->flags & LOADER_IMPID_USED)
	say(SAY_NO_NLS | SAY_NO_NL, " L_IMPID_USED");
    if (s->flags & LOADER_RENAME)
	say(SAY_NO_NLS | SAY_NO_NL, " L_RENAME");
    if (s->flags & STR_ER_OUTPUT)
	say(SAY_NO_NLS | SAY_NO_NL, " ER_OUTPUT");
    if (s->str_value)
	say(SAY_NO_NLS, "\t[value=0x%x (%d)]",
	    s->str_value, s->str_value);
    else
	say(SAY_NL_ONLY);
}
/************************************************************************
 * Name: dump_ifile
 *									*
 * Purpose: Print symbolic representation of an IFILE structure.
 *
 ************************************************************************/
static void
dump_ifile(IFILE *s)
{
    char	*iaccess;

    switch(s->i_access) {
      case I_ACCESS_SHMAT:
	iaccess = "shmat";
	break;
      case I_ACCESS_MMAP:
	iaccess = "mmap";
	break;

#ifdef I_ACCESS_MALLOC
      case I_ACCESS_MALLOC:
	iaccess = "malloc";
	break;
#endif

      case I_ACCESS_ANONMMAP:
	iaccess = "anonmmap";
	break;

#ifdef READ_FILE
      case I_ACCESS_READ:
	iaccess = "read";
	break;
#endif

      default:
	iaccess = "??";
    }

    say(SAY_NO_NLS,
	"File %d [%.8X]: (stringhash[%x]) BILK=%d%d%d%d %s\n"
	"\tfd=%d type=%s closed=%d access=%s size=%ld\n"
	"\tmapped-at[%x] auxnxt[%x]",
	s->i_ordinal,
	s,
	s->i_name,
	s->i_rebind, s->i_reinsert, s->i_library, s->i_keepfile,
	s->i_name->name,
	s->i_fd,
	get_ifile_type_string(s->i_type),
	s->i_closed,
	iaccess,
	s->i_filesz,
	s->i_map_addr,
	s->i_auxnext ? s->i_auxnext->i_ordinal : 0);
} /* dump_ifile */
/************************************************************************
 * Name: dump_ifiles1
 *									*
 * Purpose: Print symbolic representation of all IFILE structures.
 *
 ************************************************************************/
static RETCODE
dump_ifiles1(void)
{
    return dump_ifiles(NULL);
}
/************************************************************************
 * Name: dump_ifiles
 *									*
 * Purpose: Print symbolic representation of certain IFILE structures.
 * Subcommand format: DUMP IFILE [first [count]]
 *	Starting with IFILE number 'first', print 'count' structures.
 *
 ************************************************************************/
static RETCODE
dump_ifiles(arg_t limits[])
{
    int		low;
    int		count;
    int		n=1;
    IFILE	*ifile;

    if (limits == NULL) {
	count = 0x7FFFFFFF;
	low = 1;
    }
    else {
	low = limits[0].n_arg;
	count = limits[1].n_arg;

	if (count == 0x80000000) {
	    count = 1;			/* If count not specified */
	    if (low == 0x80000000)
		low = 1;	/* if low not specified */
	}

	if (low < 1) {
	    say(SAY_NO_NLS, "Input file number %d does not exist.", low);
	    return RC_OK;
	}
    }

    ifile = first_ifile;
    if (ifile == NULL) {
	say(SAY_NO_NLS, "No input files exist.");
	return RC_OK;
    }

    for ( ; n < low && ifile; ifile = ifile->i_next)
	n++;

    if (ifile == NULL) {
	say(SAY_NO_NLS, "Input file number %d does not exist.", low);
	return RC_OK;
    }

    do {		/* Always print at least one, even if count is <= 0. */
	if (interrupt_flag)
	    return RC_ABORT;
	dump_ifile(ifile);
	ifile = ifile->i_next;
    } while (ifile && --count > 0);
    return RC_OK;
} /* dump_ifiles */
/************************************************************************
 * Name: dump_object_info
 *									*
 * Purpose: Print symbolic representation of an object_info structure
 *
 ************************************************************************/
static void
dump_object_info(OBJECT_INFO *info)
{
    char buf[15];

    if (info->_flags & OBJECT_NOT_READ)
	say(SAY_NO_NLS | SAY_NO_NL,
	    "\t#sects=%d #syms=%d symtab_offset[0x%x] sect-offset[%x]\n"
	    "\tFlags(0x%x):",
	    info->_num_sections,
	    info->_num_symbols,
	    info->_symtab_offset,
	    info->u._sections_offset,
	    info->_flags);
    else
	say(SAY_NO_NLS | SAY_NO_NL,
	    "\t#sects=%d debug_sect=%d except_sect=%d #syms=%d strtab_len=%d\n"
	    "\tsymtab_offset[0x%x] ext_refs[%08x] strtab_base[0x%x]\n"
	    "\tSECTS[%08x] syms_lookup[%08x] srcfiles[%s]\n"
	    "\tFlags(0x%x):",
	    info->_num_sections,
	    info->_flags & OBJECT_HAS_DEBUG ? info->_debug_sect_i : 0,
	    info->_flags & OBJECT_HAS_EXCEPT ? info->_except_sect_i : 0,
	    info->_num_symbols,
	    info->_strtab_len,
	    info->_symtab_offset,
	    info->_ext_refs,
	    info->_strtab_base,
	    info->u._section_info,
	    info->_syms_lookup,
	    info->_srcfiles
	    	? get_sf_inpndx(info->_srcfiles->sf_inpndx, buf) : "",
	    info->_flags);
    if (info->_flags) {
	if (info->_flags & OBJECT_HAS_DEBUG)
	    say(SAY_NO_NLS | SAY_NO_NL, " HAS_DEBUG");
	if (info->_flags & OBJECT_HAS_TYPECHK)
	    say(SAY_NO_NLS | SAY_NO_NL, " HAS_TYPECHK");
	if (info->_flags & OBJECT_HAS_EXCEPT)
	    say(SAY_NO_NLS | SAY_NO_NL, " HAS_EXCEPT");
	if (info->_flags & OBJECT_HAS_INFO)
	    say(SAY_NO_NLS | SAY_NO_NL, " HAS_INFO");
	if (info->_flags & OBJECT_NOT_READ)
	    say(SAY_NO_NLS | SAY_NO_NL, " NOT_READ");
	if (info->_flags & OBJECT_GLUE)
	    say(SAY_NO_NLS | SAY_NO_NL, " GLUE");
	if (info->_flags & OBJECT_USED)
	    say(SAY_NO_NLS | SAY_NO_NL, " USED");
	if (info->_flags & OBJECT_HAS_BINCL_EINCL)
	    say(SAY_NO_NLS | SAY_NO_NL, " HAS_BINCL_EINCL");
    }
    say(SAY_NL_ONLY);
}
/************************************************************************
 * Name: get_o_number
 *									*
 * Purpose: Return the string for an object number. An object number is just
 *	its file number if it is not an archive member, and a file number,
 *	a period, and a member number for members.
 *
 * Returns: The string.
 *
 ************************************************************************/
static char *
get_o_number(OBJECT *o)
{
    static char buff[30];

    if (o == NULL)
	return "";
    else if (o->o_ifile == NULL)
	return "0.?";
    else if (o->o_member_info)
	sprintf(buff, "%d.%d", o->o_ifile->i_ordinal,
		o->o_member_info
		- o->o_ifile->i_objects->o_next->o_member_info);
    else
	sprintf(buff, "%d", o->o_ifile->i_ordinal);
    return buff;
}
/************************************************************************
 * Name: dump_object
 *									*
 * Purpose: Print symbolic representation of an object file.
 *
 ************************************************************************/
static void
dump_object(OBJECT *o)
{
    char buf[15];

    say(SAY_NO_NLS | SAY_NO_NL, "Object %s [0x%x]: %s", get_o_number(o), o,
	get_object_file_name(o));
    say(SAY_NO_NLS, " Next[%s] size=%d %s",
	get_o_number(o->o_next), o->o_size, get_object_type(o->o_type));

    switch(o->o_type) {
      case O_T_OBJECT:
	dump_object_info(o->o_info);
	break;

      case O_T_SHARED_OBJECT:
	if (is_deferred_ifile(o->o_ifile)) {
	    say (SAY_NO_NLS | SAY_NO_NL, "\tnum_sects[%d], opt_hdr_size[%d]",
		 o->o_num_sections, o->o_opt_hdr_size);
	    break;
	}
	/* else fall through */
      case O_T_SCRIPT:
	say (SAY_NO_NLS | SAY_NO_NL, "\tsrcfiles[%s]",
	     o->o_srcfiles
	     ? get_sf_inpndx(o->o_srcfiles->sf_inpndx, buf) : "");
	break;

      case O_T_EXPORT:
	/* Nothing extra needs to be printed for export files. */
	break;

      case O_T_ARCHIVE:
	say (SAY_NO_NLS | SAY_NO_NL, "\tgst_syms[%s] (%d members)",
	     show_sym(o->o_gst_syms, NULL), o->o_num_subobjects);
	break;
    }
    if (o->o_member_info) {
	say(SAY_NO_NLS, "\tmember of(%s): offset[%ld]",
	    get_object_type(o->o_contained_in_type),
	    o->o_member_info->o_ohdr_off);
    }
    else if (o->o_type != O_T_OBJECT)
	say(SAY_NL_ONLY);
} /* dump_object */
/************************************************************************
 * Name: dump_objects1
 *									*
 * Purpose: Print symbolic representation of all object files.
 *
 ************************************************************************/
static RETCODE
dump_objects1(void)
{
    return dump_objects(NULL);
}
/************************************************************************
 * Name: dump_objects
 *									*
 * Purpose: Print symbolic representation of certain object files.
 *
 * Subcommand format: DUMP OBJECT [first [count]]
 *	Starting with OBJECT number 'first', print 'count' OBJECTs.
 *	An object number is a file number if it is not an archive
 *	member.  Otherwise, it is f.m, where f is the file number and
 *	m is the member number (0-based).
 *
 ************************************************************************/
static RETCODE
dump_objects(arg_t limits[])
{
    int		low, sublow = -1;
    int		no_count = 0;
    int		wildcard_ok = 0;
    char	*low_arg, *ptr;
    int		count;
    int		rc = RC_OK;
    OBJECT	*o, *first_o;

    if (limits == NULL) {
	count = 0x7FFFFFFF;
	o = first_object();
	if (o == NULL) {
	    say(SAY_NO_NLS, "No objects");
	    return RC_OK;
	}
    }
    else {
	low_arg = limits[0].c_arg;
	count = limits[1].n_arg;

	if (count == 0x80000000) {
	    no_count = 1;
	    count = 1;			/* If count not specified */
	}
	if (low_arg == NULL)
	    low = 1;	/* if low not specified */
	else {
	    low = strtoul(low_arg, &ptr, 10);
	    if (*ptr == '.') {
		if (ptr[1] != '*')
		    sublow = strtoul(&ptr[1], NULL, 10);
		else {
		    if (no_count)
			wildcard_ok = 1;
		}
	    }
	}
	o = find_numbered_object(low, sublow);
	if (o == NULL)
	    return RC_OK;
    }
    first_o = o;

    do {		/* Always print at least one, even if count is <= 0. */
	if (interrupt_flag)
	    return RC_ABORT;
	dump_object(o);
	o = o->o_next;
	if (wildcard_ok
	    && o->o_member_info
	    && first_o == o->o_member_info->o_parent)
	    ++count;			/* Keep loop from exiting as
					   long as a wildcard was specified
					   and we're still printing members
					   of the same archive. */
    } while (o && --count > 0);
    return rc;
} /* dump_objects */
/************************************************************************
 * Name: dump_rld
 *									*
 * Purpose: Print a formatted representation of an RLD entry.
 *
 ************************************************************************/
void
dump_rld(RLD *r)
{
    char temp_buf[30];
    char temp_buf2[30];

    say(SAY_NO_NLS | SAY_NO_NL,
        "RLD(%1$s): csect[%2$s], next[%3$s], addr[%4$08x], bit length[%5$d]\n"
	"\ttype[%6$s], flags=%7$d, SYMBOL ",
	show_rld(r, NULL),
	show_sym(&r->r_csect->c_symbol, NULL),
	show_rld(r->r_next, temp_buf2),
	r->r_addr,
	r->r_length,
	get_reltype_name(r->r_reltype),
	r->r_flags);

    (void) show_inpndx(r->r_sym, "[%s]");

    say(SAY_NO_NLS, "(%1$s):%2$s",
	show_sym(r->r_sym, temp_buf), r->r_sym->s_name->name);
}
/************************************************************************
 * Name: dump_rlds_for_symbol
 *									*
 * Purpose: Print a formatted representation of all RLD entries for
 *	a given SYMBOL.
 *
 ************************************************************************/
static RETCODE
dump_rlds_for_symbol(SYMBOL *sym)
{
    RLD		*r;

    if (sym->s_smtype != XTY_SD
	&& sym->s_smtype != XTY_LD)
	return RC_OK;
    dump_csect_with_name(sym);
    for (r = sym->s_csect->c_first_rld; r; r = r->r_next)
	dump_rld(r);
    return RC_OK;
}
/************************************************************************
 * Name: dump_rlds_by_STR
 *									*
 * Purpose: Print a formatted representation of all RLD entries for
 *	all SYMBOLs with a given name.
 *
 ************************************************************************/
static RETCODE
dump_rlds_by_STR(STR *sh)
{
    SYMBOL	*sym;
    RLD		*r;

    for (sym = sh->first_ext_sym ; sym; sym = sym->s_synonym) {
	if (interrupt_flag)
	    return RC_ABORT;
	if (sym->s_smtype == XTY_LD
	    || sym->s_smtype == XTY_SD) {
	    something_printed = 1;
	    dump_csect_with_name(sym);
	    for (r = sym->s_csect->c_first_rld; r; r = r->r_next)
		dump_rld(r);
	}
    }
    for (sym = sh->first_hid_sym; sym; sym = sym->s_synonym) {
	if (interrupt_flag)
	    return RC_ABORT;
	if (sym->s_smtype == XTY_LD
	    || sym->s_smtype == XTY_SD) {
	    something_printed = 1;
	    dump_csect_with_name(sym);
	    for (r = sym->s_csect->c_first_rld; r; r = r->r_next)
		dump_rld(r);
	}
    }
    return RC_OK;
}
/************************************************************************
 * Name: dump_rlds_by_name
 *									*
 * Purpose: Print a formatted representation of all RLD entries for
 *	all SYMBOLs with a name matching a given pattern.
 *
 ************************************************************************/
static RETCODE
dump_rlds_by_name(arg_t parms[])
{
    return dump_something_by_name(parms[0].c_arg, dump_rlds_by_STR,
				  "No symbols named %s",
				  "No unnamed symbols");
}
/************************************************************************
 * Name: dump_rlds
 *									*
 * Purpose: Print a formatted representation of all RLD entries.
 *
 ************************************************************************/
static RETCODE
dump_rlds(void)
{
    OBJECT	*o;
    SRCFILE	*sf;
    CSECT	*cs;
    RLD		*r;

    for (o = first_object(); o; o = o->o_next) {
	if (o->o_type == O_T_OBJECT) {
	    for (sf = o->oi_srcfiles; sf; sf = sf->sf_next) {
		for (cs = sf->sf_csect; cs; cs = cs->c_next) {
		    if (r = cs->c_first_rld) {
			dump_csect_with_name(&cs->c_symbol);
			for ( ; r; r = r->r_next) {
			    if (interrupt_flag)
				return RC_ABORT;
			    dump_rld(r);
			}
		    }
		}
	    }
	}
    }
    return RC_OK;
}
/************************************************************************
 * Name: dump_numbered_rld
 *									*
 * Purpose: Print a formatted representation of a particular RLD.
 *
 ************************************************************************/
static RETCODE
dump_numbered_rld(arg_t parms[])
{
    int		number;
    OBJECT	*o;
    SRCFILE	*sf;
    CSECT	*cs;
    RLD		*r;

    number = parms[0].n_arg;
    for (o = first_object(); o; o = o->o_next) {
	if (o->o_type == O_T_OBJECT) {
	    for (sf = o->oi_srcfiles; sf; sf = sf->sf_next) {
		for (cs = sf->sf_csect; cs; cs = cs->c_next) {
		    for (r = cs->c_first_rld; r; r = r->r_next) {
			if (r->r_number == number) {
			    dump_csect_with_name(&cs->c_symbol);
			    dump_rld(r);
			    return RC_OK;
			}
		    }
		}
	    }
	}
    }
    say(SAY_NO_NLS, "No RLDs numbered %d", number);
    return RC_OK;
} /* dump_numbered_rld */
/************************************************************************
 * Name: dump_SRCFILE
 *									*
 * Purpose: Print a symbolic representation of a srcfile entry.
 *
 ************************************************************************/
static RETCODE
dump_SRCFILE(SRCFILE *sf)
{
    char buf1[15], buf2[15];

    say(SAY_NO_NLS,
	"Source file :%s: object[%s] inpndx=%s csects[%s] next[%s]",
	sf->sf_name->name,
	get_o_number(sf->sf_object),
	get_sf_inpndx(sf->sf_inpndx, buf1),
	sf->sf_csect ? show_sym(&sf->sf_csect->c_symbol, NULL) : "",
	sf->sf_next ? get_sf_inpndx(sf->sf_next->sf_inpndx, buf2) : "");
    return RC_OK;
}
/************************************************************************
 * Name: dump_SRCFILEs_by_number
 *									*
 * Purpose: Print a symbolic representation of a specific srcfile entry
 *	or entries.
 * Command format:	DUMP #FILE o:f
 *			DUMP #FILE o:*
 *	where o is an object number (see dump_objects() and f is a symbol
 *	table index value, or * to specify all SRCFILEs in a given object.
 *
 ************************************************************************/
static RETCODE
dump_SRCFILEs_by_number(arg_t parms[])
{
    int		number;
    char	*num_ptr = parms[0].c_arg;
    char	*ptr;
    int		seen = 0, one_o = 0;
    int		onum, onum2;
    OBJECT	*o;
    SRCFILE	*srcfiles;

    if (num_ptr == NULL)
	return dump_SRCFILEs();

    number = strtoul(num_ptr, &ptr, 10);
    onum = -1;
    onum2 = 0;
    switch(*ptr) {
      case ':':
	onum = number;
	goto get_number;
      case '.':
	onum = number;
	num_ptr = &ptr[1];
	if (*num_ptr == '*')
	    number = -2;		/* All SRCFILES in object */
	else {
	    number = strtoul(num_ptr, &ptr, 10);
	    if (*ptr == ':') {
		onum2 = number;
	      get_number:
		if (ptr[1] == '*')
		    number = -2;
		else
		    number = strtoul(&ptr[1], NULL, 10);
	    }
	    else
		onum2 = 0;
	}
	o = find_numbered_object(onum, onum2);
	if (o == NULL)
	    return RC_OK;
	if (number == -1 )
	    number = SF_GENERATED_INPNDX;
	one_o = 1;
	goto do_o;
      case '\0':
	break;
      default:
	say(SAY_NO_NLS, "Invalid SRCFILE number");
	return RC_OK;
    }

    if (number == -1 )
	number = SF_GENERATED_INPNDX;

    for (o = first_object(); o; o = o->o_next) {
      do_o:
	switch(o->o_type) {
	  case O_T_SHARED_OBJECT:
	    if (is_deferred_ifile(o->o_ifile)) {
		srcfiles = NULL;
		break;
	    }
	    /* else fall through */
	  case O_T_SCRIPT:
	    srcfiles = o->o_srcfiles;
	    break;

	  case O_T_OBJECT:
	    srcfiles = o->oi_srcfiles;
	    break;

	  case O_T_EXPORT:
	  default:
	    srcfiles = NULL;
	    break;
	}
	for ( ; srcfiles; srcfiles = srcfiles->sf_next) {
	    if (number == -2 || srcfiles->sf_inpndx == number) {
		seen = 1;
		dump_SRCFILE(srcfiles);
	    }
	    if (interrupt_flag)
		return RC_ABORT;
	}
	if (one_o)
	    break;
    }
    if (seen == 0)
	say(SAY_NO_NLS, "No SRCFILES numbered %d", number);
    return RC_OK;
}
/************************************************************************
 * Name: dump_SRCFILEs
 *									*
 * Purpose: Print symbolic representation of all SRCFILE structures.
 *
 ************************************************************************/
static RETCODE
dump_SRCFILEs(void)
{
    OBJECT	*o;
    SRCFILE	*srcfiles;

    for (o = first_object(); o; o = o->o_next) {
	switch(o->o_type) {
	  case O_T_SHARED_OBJECT:
	    if (is_deferred_ifile(o->o_ifile)) {
		srcfiles = NULL;
		break;
	    }
	    /* else fall through */
	  case O_T_SCRIPT:
	    srcfiles = o->o_srcfiles;
	    break;
	  case O_T_OBJECT:
	    srcfiles = o->oi_srcfiles;
	    break;
	  case O_T_EXPORT:
	  default:
	    srcfiles = NULL;
	    break;
	}
	for ( ; srcfiles; srcfiles = srcfiles->sf_next) {
	    dump_SRCFILE(srcfiles);
	    if (interrupt_flag)
		return RC_ABORT;
	}
    }
    return RC_OK;
}
/************************************************************************
 * Name: dump_TYPECHK
 *									*
 * Purpose: Print symbolic representation of a TYPECHK for a given symbol.
 *
 ************************************************************************/
static RETCODE
dump_TYPECHK(SYMBOL *s,
	     int noname)
{
    if (noname)
	say(SAY_NO_NLS | SAY_NO_NL, "\tSymbol[%s]:", show_sym(s, NULL));
    else
	say(SAY_NO_NLS | SAY_NO_NL, "Symbol(%s)[%s]:",
	    s->s_name->name, show_sym(s, NULL));

    if (s->s_typechk == NULL)
	say(SAY_NO_NLS | SAY_NO_NL, "No typecheck");
    else
	say(SAY_NO_NLS | SAY_NO_NL, "%4s %08X %08X ",
	    language_name(s->s_typechk->t_typechk.t_lang),
	    *(ulong *)&s->s_typechk->t_typechk.t_ghash[0],
	    *(ulong *)&s->s_typechk->t_typechk.t_lhash[0]);
    say(SAY_NO_NLS, "\tfrom:%s",
	get_object_file_name(s->s_smtype == XTY_ER
			     ? s->er_object
			     : s->s_smtype == XTY_AR
			     ? s->s_object
			     : s->s_csect->c_srcfile->sf_object));
}
/************************************************************************
 * Name: dump_flags
 *									*
 * Purpose: Print symbolic value of flags halfword for a SYMBOL
 *
 ************************************************************************/
static void
dump_flags(uint16 flags)
{
    if (flags == 0)
	say(SAY_NO_NLS | SAY_NO_NL, "none");
    else {
	if (flags & S_XMC_XO)
	    say(SAY_NO_NLS | SAY_NO_NL, " XMC_XO");
	if (flags & S_PRIMARY_LABEL)
	    say(SAY_NO_NLS | SAY_NO_NL, " PRIMARY_LABEL");
	if (flags & S_DUPLICATE)
	    say(SAY_NO_NLS | SAY_NO_NL, " DUPLICATE");
	if (flags & S_DUPLICATE2)
	    say(SAY_NO_NLS | SAY_NO_NL, " DUP2");
	if (flags & S_ARCHIVE_SYMBOL)
	    say(SAY_NO_NLS | SAY_NO_NL, " IN_ARCHIVE");
	if (flags & S_MARK)
	    say(SAY_NO_NLS | SAY_NO_NL, " MARKED");
	if (flags & S_ISTOC)
	    say(SAY_NO_NLS | SAY_NO_NL, " ISTOC");
	if (flags & S_VISITED)
	    say(SAY_NO_NLS | SAY_NO_NL, " VISITED");
	if (flags & S_RESOLVED_OK)
	    say(SAY_NO_NLS | SAY_NO_NL, " RESOLVED_OK");
	if (flags & S_TYPECHK_IMPLIED)
	    say(SAY_NO_NLS | SAY_NO_NL, " TYPCHK_IMPLIED");
	if (flags & S_TYPECHK_USED)
	    say(SAY_NO_NLS | SAY_NO_NL, " TYPCHK_USED");
	if (flags & S_LOCAL_GLUE)
	    say(SAY_NO_NLS | SAY_NO_NL, " LOCAL_GLUE");
	if (flags & S_SAVE)
	    say(SAY_NO_NLS | SAY_NO_NL, " SAVED");
	if (flags & S_NUMBER_USURPED)
	    say(SAY_NO_NLS | SAY_NO_NL, " NUMBER_USURPED");
    }
}
/************************************************************************
 * Name: dump_symbol
 *									*
 * Purpose: Print symbolic representation of a SYMBOL.
 *
 ************************************************************************/
static RETCODE
dump_symbol(SYMBOL *s,
	    int noname)
{
    if (noname)
	say(SAY_NO_NLS | SAY_NO_NL, "\tSymbol[%s]:", show_sym(s, NULL));
    else
	say(SAY_NO_NLS | SAY_NO_NL, "Symbol[%s]: name=%s\n\t",
	    show_sym(s, NULL), s->s_name->name);

    dump_controls |= DUMP_GET_SMTYPE;	/* Allow get_smtype to return
					   internal-only smtype names. */
    say(SAY_NO_NLS | SAY_NO_NL, "type=%s,class=%s, addr[%08x] syn[%s]",
	get_smtype(s->s_smtype), get_smclass(s->s_smclass),
	s->s_addr, show_sym(s->s_synonym, NULL));
    dump_controls &= ~DUMP_GET_SMTYPE;	/* Reset. */

    say(SAY_NO_NLS | SAY_NO_NL, " nxt[%s]",
	show_sym(s->s_next_in_csect, NULL));
    if (s->s_smtype == XTY_AR)
	say(SAY_NO_NLS | SAY_NO_NL, " prv[%s]",
	    show_sym(s->s_prev_in_gst, NULL));

    say(SAY_NO_NLS | SAY_NO_NL, "\n\t inpndx=");
    (void) show_inpndx(s, "%s");
    if (s->s_inpndx == INPNDX_IMPORT_TD)
	say(SAY_NO_NLS | SAY_NO_NL, "_TD");

    if (s->s_flags & S_INPNDX_MOD)
	say(SAY_NO_NLS | SAY_NO_NL, "/%d", symtab_index[s->s_inpndx+1]);

    /* Check for overload of s_csect */
    if (s->s_smtype == XTY_AR)
	say(SAY_NO_NLS | SAY_NO_NL, "\tobject[%s]", get_o_number(s->s_object));
    else
	say(SAY_NO_NLS | SAY_NO_NL, "\tcsect[%s]",
	    show_sym(&s->s_csect->c_symbol, NULL));

    if (s->s_flags & S_HIDEXT) {
	if (s->s_flags & S_RESOLVED_OK)
	    say(SAY_NO_NLS | SAY_NO_NL,
		" Resolved=[%s]", show_sym(s->s_resolved, NULL));
    }
    else if (s->s_smtype != XTY_AR)
	say(SAY_NO_NLS | SAY_NO_NL, " typechk=%x", s->s_typechk);

    say(SAY_NO_NLS | SAY_NO_NL, " from:%s\n\t flags:",
	get_object_file_name(s->s_smtype == XTY_AR
			     ? s->s_object
			     : s->s_csect->c_srcfile->sf_object));

    dump_flags(s->s_flags);
    say(SAY_NL_ONLY);

    return RC_OK;
} /* dump_symbol */
/************************************************************************
 * Name: dump_er
 *									*
 * Purpose: Print symbolic representation of an ER symbol.
 *
 ************************************************************************/
static RETCODE
dump_er(SYMBOL *er)
{
    say(SAY_NO_NLS | SAY_NO_NL, "\tER[%s]:", show_er(er, NULL));
    say(SAY_NO_NLS | SAY_NO_NL, "class=%s, syn[%s]",
	get_smclass(er->er_smclass), show_sym(er->er_synonym, NULL));
    say(SAY_NO_NLS, " nxt[%s] typechk=%x from:%s",
	show_sym(er->er_next_in_object, NULL),
	er->er_typechk,
	get_object_file_name(er->er_object));
    say(SAY_NO_NLS | SAY_NO_NL, "\t flags:");
    dump_flags(er->s_flags);

    say(SAY_NO_NLS | SAY_NO_NL, ", inpndx=");
    (void) show_inpndx(er, "%s");
    if (er->s_inpndx == INPNDX_IMPORT_TD)
	say(SAY_NO_NLS | SAY_NO_NL, "_TD");

    if (er->er_flags & S_INPNDX_MOD)
	say(SAY_NO_NLS | SAY_NO_NL, "%d", symtab_index[er->er_inpndx+1]);

    if (er->s_flags & S_XMC_XO || er->s_addr != 0)
	say(SAY_NO_NLS | SAY_NO_NL, " address = 0x%X", er->s_addr);

    say(SAY_NO_NLS | SAY_NL_ONLY);

    return RC_OK;
}
/************************************************************************
 * Name: dump_xref_by_name
 *									*
 * Purpose: Print symbolic representation of all RLDs referring to symbols
 *	matching a given pattern.
 * Command format:	DUMP XREF pattern
 *
 ************************************************************************/
static RETCODE
dump_xref_by_name(arg_t parms[])
{
    char	*name = parms[0].c_arg;
    CSECT	*cs;
    OBJECT	*o;
    RLD		*r;
    SRCFILE	*sf;
    STR		*sh;

    sh = lookup_stringhash(name);
    if (sh == NULL) {
	say(SAY_NO_NLS, "No symbols named %s", name);
	return RC_OK;
    }
    else {
	print_STR_name_and_flags(sh);
	for (o = first_object(); o; o = o->o_next) {
	    switch(o->o_type) {
	      case O_T_SHARED_OBJECT:
		if (is_deferred_ifile(o->o_ifile)) {
		    sf = NULL;
		    break;
		}
		/* else fall through */
	      case O_T_SCRIPT:
		sf = o->o_srcfiles;
		break;
	      case O_T_OBJECT:
		sf = o->oi_srcfiles;
		break;
	      case O_T_EXPORT:
	      default:
		sf = NULL;
		break;
	    }
	    for ( ; sf; sf = sf->sf_next) {
		for (cs = sf->sf_csect; cs; cs = cs->c_next) {
		    if (interrupt_flag)
			return RC_ABORT;
		    for (r = cs->c_first_rld; r; r = r->r_next)
			if (r->r_sym->s_name == sh)
			    dump_rld(r);
		}
	    }
	    if (interrupt_flag)
		return RC_ABORT;
	}
    }
    return RC_OK;
} /* dump_xref_by_name */
/************************************************************************
 * Name: dump_csects_by_name
 *									*
 * Purpose: Print symbolic representation of all CSECTS containing a symbol
 *	whose name matches a given pattern.
 *
 ************************************************************************/
static RETCODE
dump_csects_by_name(arg_t parms[])
{
    char	*name = parms[0].c_arg;
    int		name_printed = 0;
    STR		*sh;
    SYMBOL	*sym;

    sh = lookup_stringhash(name);
    if (sh == NULL) {
	say(SAY_NO_NLS, "No symbols named %s", name);
	return RC_OK;
    }
    else {/*if (sh && (sh->first_ext_sym || sh->first_hid_sym)) {*/
	for (sym = sh->first_ext_sym ; sym; sym = sym->s_synonym) {
	    if (interrupt_flag)
		return RC_ABORT;
	    if (sym->s_smtype == XTY_AR)
		continue;		/* Archive symbols don't have csects */
	    if (name_printed == 0) {
		print_STR_name_and_flags(sh);
		name_printed = 1;
	    }
	    dump_csect(sym->s_csect, 1);
	}
	for (sym = sh->first_hid_sym; sym; sym = sym->s_synonym) {
	    if (interrupt_flag)
		return RC_ABORT;
	    if (name_printed != 2) {
		if (name_printed == 0)
		    print_STR_name_and_flags(sh);
		say(SAY_NO_NLS, "    Hidden csects");
		name_printed = 2;
	    }
	    dump_csect(sym->s_csect, 1);
	}
    }
    return RC_OK;
} /* dump_csects_by_name */
/************************************************************************
 * Name: dump_TYPECHKs_by_STR
 *									*
 * Purpose: Print typchks for all external references that
 *	match the pattern given by *parms.
 *
 ************************************************************************/
static RETCODE
dump_TYPECHKs_by_STR(STR *sh)
{
    SYMBOL	*sym;
    SYMBOL	*er;

    if (sh->first_ext_sym || sh->refs || (sh->flags & STR_EXPORT)) {
	something_printed = 1;
	print_STR_name_and_flags(sh);
	for (sym = sh->first_ext_sym ; sym; sym = sym->s_synonym) {
	    if (interrupt_flag)
		return RC_ABORT;
	    dump_TYPECHK(sym, 1);
	}
	if (er = sh->refs) {
	    say(SAY_NO_NLS, "    External References");

	    for (; er; er = er->er_synonym) {
		if (interrupt_flag)
		    return RC_ABORT;
		dump_TYPECHK(er, 1);
	    }
	}
    }
    return RC_OK;
}
/************************************************************************
 * Name: dump_symbols_by_STR
 *									*
 * Purpose: Print symbolic representation of SYMBOLs (and ERs) for
 *	a given STR.
 *
 ************************************************************************/
static RETCODE
dump_symbols_by_STR(STR *sh)
{
    SYMBOL	*sym;
    SYMBOL	*er;

    if (sh->flags & STR_ISSYMBOL) {
	something_printed = 1;
	print_STR_name_and_flags(sh);
	for (sym = sh->first_ext_sym ; sym; sym = sym->s_synonym) {
	    if (interrupt_flag)
		return RC_ABORT;
	    dump_symbol(sym, 1);
	}
	if (er = sh->refs) {
	    say(SAY_NO_NLS, "    External References");

	    for (; er; er = er->er_synonym) {
		if (interrupt_flag)
		    return RC_ABORT;
		dump_er(er);
	    }
	}
	if (sym = sh->first_hid_sym) {
	    say(SAY_NO_NLS, "    Hidden symbols");
	    for (; sym; sym = sym->s_synonym) {
		if (interrupt_flag)
		    return RC_ABORT;
		dump_symbol(sym, 1);
	    }
	}
    }
    return RC_OK;
}
/************************************************************************
 * Name: dump_something_by_name
 *									*
 * Purpose: Print symbolic representation of something for all names that
 *	match the pattern given by *parms.
 *
 ************************************************************************/
static RETCODE
dump_something_by_name(char *pattern,
		       int (*printer)(), /* Function to call for each
					    matching name */
		       char *no_match_message,
		       char *no_match_null_message)
{
    int		rc;

    something_printed = 0;
    if (strcmp(pattern, "''") == 0) {	/* Null pattern */
	printer(&NULL_STR);
	if (something_printed == 0)
	    say(SAY_NO_NLS, no_match_null_message);
    }
    else {
	rc = match(pattern, MATCH_NO_NEWNAME, MATCH_ANY, printer);
	if (rc)
	    return rc;
	if (something_printed == 0)
	    say(SAY_NO_NLS, no_match_message, pattern);
    }
    return RC_OK;
}
/************************************************************************
 * Name: dump_symbols_by_name
 *									*
 * Purpose: Print symbolic representation of all SYMBOLs (and ERs) that
 *	match the pattern given by *parms.
 *
 ************************************************************************/
static RETCODE
dump_symbols_by_name(arg_t parms[])
{
    return dump_something_by_name(parms[0].c_arg, dump_symbols_by_STR,
				  "No symbols named %s",
				  "No unnamed symbols");
}
/************************************************************************
 * Name: dump_TYPECHKs_by_name
 *									*
 * Purpose: Print symbolic representation of all TYPECHKs for SYMBOLs
 *	(and ERs) whose name matches the pattern given by *parms.
 *
 ************************************************************************/
static RETCODE
dump_TYPECHKs_by_name(arg_t parms[])
{
    return dump_something_by_name(parms[0].c_arg, dump_TYPECHKs_by_STR,
				  "No symbols named %s",
				  "No unnamed symbols");
}
/************************************************************************
 * Name: dump_csect
 *									*
 * Purpose: Print symbolic representation of a CSECT.
 *	This function is called from code_save.c, but only if dbgopt7
 *	is set.
 *
 ************************************************************************/
RETCODE
dump_csect(CSECT *cs,			/* csect to print about */
	   int debug)			/* Show mark/save bits */
{
    char temp_buf[30];

    say(SAY_NO_NLS | SAY_NO_NL,
	"CSECT(%s):len=%d align=%d smcl=%s hid=%c TD_ref=%c,",
	show_sym(&cs->c_symbol, NULL),
	cs->c_len,
	cs->c_align,
	get_smclass(cs->c_symbol.s_smclass),
	(cs->c_symbol.s_flags & S_HIDEXT) ? 'Y' : 'N',
	cs->c_TD_ref ? 'Y' : 'N');
    if (debug)
	say(SAY_NO_NLS | SAY_NO_NL, "M/S=%c/%c,",
	    cs->c_mark ? 'Y' : 'N', cs->c_save ? 'Y' : 'N');

    if (debug && cs->c_save)
	say(SAY_NO_NLS, "sect = %s/%d", get_sectnum(cs->c_secnum),
	    cs->c_major_sect);
    else
	say(SAY_NO_NLS, "sect = %s", get_sectnum(cs->c_secnum));

    say(SAY_NO_NLS, "\tLDs[%s] RLDs[%s] addrs[%08x,%08x] nxt[%s]",
	show_sym(&cs->c_symbol, NULL),
	show_rld(cs->c_first_rld, NULL),
	cs->c_addr,
	cs->c_new_addr,
	cs->c_next ? show_sym(&cs->c_next->c_symbol, temp_buf) : "");
    if (cs->c_srcfile->sf_inpndx == SF_GENERATED_INPNDX)
	say(SAY_NO_NLS, "\tFrom:%s",
	    get_object_file_name(cs->c_srcfile->sf_object));
    else
	say(SAY_NO_NLS, "\tFrom:%s/[F%d]%s",
	    get_object_file_name(cs->c_srcfile->sf_object),
	    cs->c_srcfile->sf_inpndx,
	    cs->c_srcfile->sf_name->name);

    return RC_OK;
}
/************************************************************************
 * Name: dump_csects
 *									*
 * Purpose: Print symbolic representation of all CSECTs.
 *
 ************************************************************************/
static RETCODE
dump_csects(void)
{
    OBJECT	*o;
    SRCFILE	*sf;
    CSECT	*cs;

    for (o = first_object(); o; o = o->o_next) {
	switch(o->o_type) {
	  case O_T_OBJECT:
	    for (sf = o->oi_srcfiles; sf; sf = sf->sf_next)
		for (cs = sf->sf_csect; cs; cs = cs->c_next) {
		    if (interrupt_flag)
			return RC_ABORT;
		    dump_csect(cs, 1);
		}
	    break;
	}
    }
    return RC_OK;
}
/************************************************************************
 * Name: dump_something_by_number
 *									*
 * Purpose: Print something for a particular symbol.  Only one symbol
 *	is dumped.
 *
 ************************************************************************/
static RETCODE
dump_something_by_number(arg_t parms[],
			 int (*dump_routine)())
{
    int		i;
    int		number;
    HASH_STR	*sroot, *sh;
    STR		*s;
    SYMBOL	*sym;

    number = parms[0].n_arg;
    for (sroot = &HASH_STR_root; sroot; sroot = sroot->next) {
	for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
	    s = &sh->s;
	    for (sym = s->first_ext_sym; sym; sym = sym->s_synonym)
		if (sym->s_number == number)
		    goto show_it;
	    for (sym = s->first_hid_sym ; sym; sym = sym->s_synonym)
		if (sym->s_number == number)
		    goto show_it;
	    if (s->alternate) {
		for (sym= s->alternate->first_ext_sym; sym; sym=sym->s_synonym)
		    if (sym->s_number == number)
			goto show_it;
		for (sym= s->alternate->first_hid_sym; sym; sym=sym->s_synonym)
		    if (sym->s_number == number)
			goto show_it;
	    }
	}
    }
    for (sym = NULL_STR.first_ext_sym; sym; sym = sym->s_synonym)
	if (sym->s_number == number)
	    goto show_it;
    for (sym = NULL_STR.first_hid_sym; sym; sym = sym->s_synonym)
	if (sym->s_number == number)
	    goto show_it;

    if (NULL_STR.alternate != NULL) {
	for (sym = NULL_STR.alternate->first_ext_sym; sym; sym= sym->s_synonym)
	    if (sym->s_number == number)
		goto show_it;
	for (sym = NULL_STR.alternate->first_hid_sym; sym; sym= sym->s_synonym)
	    if (sym->s_number == number)
		goto show_it;
    }
    say(SAY_NO_NLS, "No symbols numbered %d", number);
    return RC_OK;

  show_it:
    if (interrupt_flag)
	return RC_ABORT;

    dump_routine(sym);
    return RC_OK;
} /* dump_something_by_number */
/************************************************************************
 * Name: dump_TYPECHK_with_name
 *									*
 * Purpose: Print symbolic representation of a TYPECHK for a given symbol,
 *	including the symbol's name.
 *
 ************************************************************************/
static RETCODE
dump_TYPECHK_with_name(SYMBOL *s)
{
    return dump_TYPECHK(s, 0);
}
/************************************************************************
 * Name: dump_symbol_with_name
 *									*
 * Purpose: Print symbolic representation of a SYMBOL, including its name.
 *
 ************************************************************************/
static RETCODE
dump_symbol_with_name(SYMBOL *s)
{
    return dump_symbol(s, 0);
}
/************************************************************************
 * Name: dump_csect_with_name
 *									*
 * Purpose: Print symbolic representation of a CSECT, including its name.
 *
 ************************************************************************/
static RETCODE
dump_csect_with_name(SYMBOL *s)
{
    if (s->s_smtype == XTY_AR)
	say(SAY_NO_NLS,
	    "Symbol %d, from an archive symbol table, has no CSECT.");
    else {
	say(SAY_NO_NLS | SAY_NO_NL, "Symbol[%s]: name=%s\n\t",
	    show_sym(s, NULL), s->s_name->name);

	return dump_csect(s->s_csect, 1);
    }
}
/************************************************************************
 * Name: dump_TYPECHKs_by_number
 *									*
 * Purpose: Print symbolic representation of a TYPECHK for a symbol with
 *	the given number.
 *
 ************************************************************************/
static RETCODE
dump_TYPECHKs_by_number(arg_t parms[])
{
    return dump_something_by_number(parms, dump_TYPECHK_with_name);
}
/************************************************************************
 * Name: dump_symbol_by_number
 *									*
 * Purpose: Print symbolic representation of a SYMBOL with a given
 *	number, specified by parms[0].
 *
 ************************************************************************/
static RETCODE
dump_symbol_by_number(arg_t parms[])
{
    return dump_something_by_number(parms, dump_symbol_with_name);
}
/************************************************************************
 * Name: dump_csect_by_number
 *									*
 * Purpose: Print symbolic representation of a CSECT with a given symbol
 *	number, specified by parms[0].
 *
 ************************************************************************/
static RETCODE
dump_csect_by_number(arg_t parms[])
{
    return dump_something_by_number(parms, dump_csect_with_name);
}
/************************************************************************
 * Name: dump_rlds_by_number
 *									*
 * Purpose: Print symbolic representation of all RLDs for a symbol with
 *	a given symbol number, specified by parms[0].
 *
 ************************************************************************/
static RETCODE
dump_rlds_by_number(arg_t parms[])
{
    return dump_something_by_number(parms, dump_rlds_for_symbol);
}
/************************************************************************
 * Name: dump_ers_by_number
 *									*
 * Purpose: Print symbolic representation of an ER with
 *	a given symbol number, specified by parms[0].
 *
 ************************************************************************/
static RETCODE
dump_ers_by_number(arg_t parms[])
{
    int		i;
    int		number;
    HASH_STR	*sroot, *sh;
    STR		*s;
    SYMBOL	*sym;

    number = parms[0].n_arg;
    for (sroot = &HASH_STR_root; sroot; sroot = sroot->next) {
	for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
	    s = &sh->s;
	    for (sym = s->refs; sym; sym = sym->s_synonym)
		if (sym->s_number == number)
		    goto show_it;
	    if (s->alternate)
		for (sym = s->alternate->refs; sym; sym = sym->s_synonym)
		    if (sym->s_number == number)
			goto show_it;
	}
    }
    for (sym=NULL_STR.refs; sym; sym=sym->s_synonym)
	if (sym->s_number == number)
	    goto show_it;
    if (NULL_STR.alternate)
	for (sym = NULL_STR.alternate->refs; sym; sym=sym->s_synonym)
	    if (sym->s_number == number)
		goto show_it;

    say(SAY_NO_NLS, "No external references numbered %d", number);
    return RC_OK;

  show_it:
    if (interrupt_flag)
	return RC_ABORT;

    print_STR_name_and_flags(s);
    dump_er(sym);
    return RC_OK;
}
/************************************************************************
 * Name: dump_TYPECHKs
 *									*
 * Purpose: Print symbolic representation of all TYPECHKs
 *
 ************************************************************************/
static RETCODE
dump_TYPECHKs(void)
{
    int		i;
    HASH_STR	*sroot, *sh;
    SYMBOL	*sym;

    for (sroot = &HASH_STR_root; sroot; sroot = sroot->next) {
	for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
	    if (interrupt_flag)
		return RC_ABORT;
	    dump_TYPECHKs_by_STR(&sh->s);
	    if (sh->s.alternate)
		dump_TYPECHKs_by_STR(sh->s.alternate);
	}
    }
    if (NULL_STR.alternate)
	dump_TYPECHKs_by_STR(NULL_STR.alternate);

    if (NULL_STR.first_ext_sym) {
	say(SAY_NO_NLS, "Unnamed (HIDEXT) symbols");
	for (sym = NULL_STR.first_ext_sym; sym; sym = sym->s_synonym) {
	    if (interrupt_flag)
		return RC_ABORT;
	    dump_TYPECHK(sym, 1);
	}
    }
    return RC_OK;
}
/************************************************************************
 * Name: dump_symbols
 *									*
 * Purpose: Print symbolic representation of all SYMBOLs.
 *
 ************************************************************************/
static RETCODE
dump_symbols(void)
{
    int		i;
    HASH_STR	*sroot, *sh;
    SYMBOL	*sym;

    for (sroot = &HASH_STR_root; sroot; sroot = sroot->next) {
	for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
	    if (interrupt_flag)
		return RC_ABORT;
	    dump_symbols_by_STR(&sh->s);
	    if (sh->s.alternate)
		dump_symbols_by_STR(sh->s.alternate);
	}
    }
    if (NULL_STR.alternate)
	dump_symbols_by_STR(NULL_STR.alternate);

    if (NULL_STR.first_ext_sym || NULL_STR.first_hid_sym) {
	say(SAY_NO_NLS, "Unnamed (HIDEXT) symbols");
	for (sym = NULL_STR.first_ext_sym; sym; sym = sym->s_synonym) {
	    if (interrupt_flag)
		return RC_ABORT;
	    dump_symbol(sym, 1);
	}
	for (sym = NULL_STR.first_hid_sym; sym; sym = sym->s_synonym) {
	    if (interrupt_flag)
		return RC_ABORT;
	    dump_symbol(sym, 1);
	}
    }
    return RC_OK;
}
/************************************************************************
 * Name: dump_ur1
 *									*
 * Purpose: Print symbolic representation of all symbols with references
 *	but no name.
 *
 ************************************************************************/
static RETCODE
dump_ur1(STR *s)
{
    SYMBOL	*er;

    if ((er = s->refs) && s->first_ext_sym == NULL) {
	something_printed = 1;
	print_STR_name_and_flags(s);
	while (er) {
	    if (interrupt_flag)
		return RC_ABORT;
	    dump_er(er);
	    er = er->er_synonym;
	}
    }
    else if (s->first_ext_sym == NULL && (s->flags & STR_EXPORT))
	say(SAY_NO_NLS, "Symbol %s exported but not defined", s->name);
    return RC_OK;
}
/************************************************************************
 * Name: dump_unrefs
 *									*
 * Purpose: Print symbolic representation of all ERs for undefined
 *	SYMBOLs.
 *
 ************************************************************************/
static RETCODE
dump_unrefs(void)
{
    int		i;
    HASH_STR	*sroot, *sh;

    for (sroot = &HASH_STR_root; sroot; sroot = sroot->next) {
	for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
	    if (interrupt_flag)
		return RC_ABORT;
	    dump_ur1(&sh->s);
	    if (sh->s.alternate)
		dump_ur1(sh->s.alternate);
	}
    }
    dump_ur1(&NULL_STR);
    if (NULL_STR.alternate)
	dump_ur1(NULL_STR.alternate);

    return RC_OK;
}
/************************************************************************
 * Name: dump_unrefs_by_name
 *									*
 * Purpose: Print symbolic representation of all ERs for undefined
 *	symbols whose name matches the pattern given by *parms.
 *
 ************************************************************************/
static RETCODE
dump_unrefs_by_name(arg_t parms[])
{
    return dump_something_by_name(parms[0].c_arg, dump_ur1,
				  "No undefined symbols %s",
				  "No undefined symbols");
}
/************************************************************************
 * Name: dump_er1
 *									*
 * Purpose: Print symbolic representation of all ERs for a given STR.
 *
 ************************************************************************/
static RETCODE
dump_er1(STR *s)
{
    SYMBOL	*er;

    if (er = s->refs) {
	something_printed = 1;
	print_STR_name_and_flags(s);
	while (er) {
	    if (interrupt_flag)
		return RC_ABORT;
	    dump_er(er);
	    er = er->er_synonym;
	}
    }
    return RC_OK;
}
/************************************************************************
 * Name: dump_ers
 *									*
 * Purpose: Print symbolic representation of all ERs.
 *
 ************************************************************************/
static RETCODE
dump_ers(void)
{
    int		i;
    HASH_STR	*sroot, *sh;

    for (sroot = &HASH_STR_root; sroot; sroot = sroot->next) {
	for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
	    if (interrupt_flag)
		return RC_ABORT;
	    (void) dump_er1(&sh->s);
	    if (sh->s.alternate)
		(void) dump_er1(sh->s.alternate);
	}
    }

    /* There shouldn't be any references to unnamed symbols or to symbols
       names '.', but the code's here anyway. */
    (void) dump_er1(&NULL_STR);
    if (NULL_STR.alternate)
	(void) dump_er1(NULL_STR.alternate);

    return RC_OK;
}
/************************************************************************
 * Name: dump_ers_by_name
 *									*
 * Purpose: Print symbolic representation of all ERs that
 *	match the pattern given by *parms.
 *
 ************************************************************************/
static RETCODE
dump_ers_by_name(arg_t parms[])
{
    return dump_something_by_name(parms[0].c_arg, dump_er1,
			  "No external references for name or pattern %s",
				  "No references to unnamed symbols");
}
