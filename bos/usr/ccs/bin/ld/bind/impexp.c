#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)96	1.20  src/bos/usr/ccs/bin/ld/bind/impexp.c, cmdld, bos41B, 9504A 1/10/95 14:12:30")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS (for binder subcommands):
 *		import_symbols_in_file
 *		import_symbols
 *		export_symbols_in_file
 *		export_symbols
 *
 *   FUNCTIONS: read_impexp_object
 *
 *   STATIC FUNCTIONS:
 *		import_name
 *		export_name
 *		parse_impname
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
#include <errno.h>

#include <sys/param.h>

#include "bind.h"
#include "error.h"
#include "global.h"
#include "strs.h"

#include "ifiles.h"
#include "insert.h"
#include "match.h"
#include "objects.h"
#include "symbols.h"
#include "util.h"

/* Static variables */
static int export_syscall;
static int export_found;

/************************************************************************
 * Name: import_symbols_in_file       Processor for the IMPORTS command *
 *									*
 * Purpose: Identify the symbols that are external to
 *	the module being bound.  These symbols will be listed in	*
 *	the loader section of the module that is built.			*
 *									*
 *	This routine reads the specified file, which contains the	*
 *	list of symbols to import.  The file contains one symbol	*
 *	per line.  Any line may also contain the hexadecimal		*
 *	address to be used by the binder for address resolution.	*
 *									*
 * Command Format:							*
 *	IMPORTS fn
 *									*
 * Arguments:								*
 *	fn - Name of file to process.					*
 *									*
 * Returns: Return value from handle_insert(); or
 *	RC_ERROR - Cannot find or open input file.
 *									*
 ************************************************************************/
RETCODE
import_symbols_in_file(char *args[])	/* argv-style arguments */
{
    IFILE	*ifile;

    /* open file with names & addresses of symbols to be imported */
    ifile = ifile_open_and_map(args[1]);

    if (ifile == NULL)
	return RC_ERROR;

    ifile->i_type = I_T_SCRIPT;	/* Set file type */
    return handle_insert(ifile, NULL);
}
/************************************************************************
 * Name: export_symbols_in_file	      Processor for the EXPORTS command *
 *									*
 * Purpose: Identify symbols defined in this module
 *	that are to be made known to other modules.  These symbols will	*
 *	be listed in the loader section of the module that is being	*
 *	built.								*
 *									*
 *	This routine reads the specified file, which contains the	*
 *	list of symbols to export.  The file contains one symbol per	*
 *	line. A symbol may be followed by the keyword "svc" or "syscall"
 *	to cause it to be exported as a system call (with storage-mapping
 *	class XMC_SV.
 *									*
 * Command Format:							*
 *	EXPORTS fn
 *									*
 * Arguments:								*
 *	fn - Name of file to process.					*
 *									*
 * Returns: Return value from handle_insert(); or
 *	RC_ERROR - Cannot find or open input file.
 *									*
 ***********************************************************************/
RETCODE
export_symbols_in_file(char *args[])	/* argv-style arguments */
{
    IFILE	*ifile;

    /* open file with names & addresses of symbols to be imported */
    ifile = ifile_open_and_map(args[1]);

    if (ifile == NULL)
	return RC_ERROR;

    ifile->i_type = I_T_EXPORT;
    return handle_insert(ifile, NULL);
}
/************************************************************************
 * Name: export_name
 *
 * Purpose: Export an external symbol with a given name.
 *
 * Returns:
 *	RC_OK
 *
 ************************************************************************/
static RETCODE
export_name(STR *name)
{
    export_found++;

    if (name->flags & STR_EXPORT) {
	bind_err(SAY_NORMAL, RC_WARNING,
		 NLSMSG(EXPORT_DUP,
		"%1$s: 0711-415 WARNING: Symbol %2$s is already exported."),
		 Main_command_name, name->name);
	if ((export_syscall == 0) != (!(name->flags & STR_SYSCALL)))
	    if (export_syscall)
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(EXPORT_SYSCALL1,
				"\tAdding syscall attribute for symbol."));
	    else
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(EXPORT_SYSCALL2,
				"\tRemoving syscall attribute for symbol."));
    }
    else {
	name->flags |= STR_EXPORT;
	++Bind_state.num_exports;
    }

    if (export_syscall)
	name->flags |= STR_SYSCALL;
    else
	name->flags &= ~STR_SYSCALL;

    if (Switches.verbose)
	say(SAY_NO_NLS, "%s: %s", Command_name, name->name);
    return RC_OK;
}
/************************************************************************
 * Name: export_symbols		Processor for the EXPORT command	*
 *									*
 * Purpose: Identify symbols that are defined within this module	*
 *	and are to be made known to other modules.  These symbols will	*
 *	be listed in the loader section of the module that is being	*
 *	built.								*
 *									*
 * Command Format:							*
 *	EXPORT pattern ['syscall'|'svc']
 *									*
 * Arguments:								*
 *	pattern - Pattern for a symbol name.				*
 *	'svc' and 'syscall' are keywords, to be specified without quotes
 *
 * Returns:
 *	RC_OK
 *									*
 *									*
 ************************************************************************/
RETCODE
export_symbols(char *arg[])	/* argv-style arguments */
{
    export_found = 0;
    export_syscall = 0;

    if (arg[2]) {
	lower(arg[2]);
	if (strcmp(arg[2], "svc") == 0 || strcmp(arg[2], "syscall") == 0)
	    export_syscall = 1;
    }

    match(arg[1], MATCH_ADD_NEWNAME, MATCH_EXT | MATCH_DUP, export_name);
    say(SAY_NORMAL, NLSMSG(EXPORTS_COUNT, "%1$s: Symbols exported: %2$d"),
	Command_name, export_found);

    if (export_found)
	Bind_state.state |= STATE_RESOLVE_NEEDED;

    return RC_OK;
}
/************************************************************************
 * Name: import_name
 *
 * Purpose: Allocate and initialize an imported symbol.
 *
 * Returns: New symbol
 *
 ************************************************************************/
static SYMBOL *
import_name(STR *name,			/* Name of symbol */
	    CSECT *cs,			/* Csect containing symbol */
	    SYMBOL *prev_sym,		/* Previous symbol, or NULL if none */
	    int import_smc,		/* If XMC_XO, symbol is prebound  */
	    unsigned int import_addr,	/* Address if import_smc==XMC_XO */
	    int in_archive)		/* S_ARCHIVE_SYMBOL if symbol is from
					   archive member */
{
    SYMBOL	*sym;

    sym = create_imported_SYMBOL(name, cs, prev_sym, import_smc, import_addr);

    sym->s_smtype = XTY_IF;		/* Imported symbol from ascii file */
    sym->s_flags |= in_archive;
    sym->s_typechk = NULL;

    return sym;
} /* import_name */
/************************************************************************
 * Name: import_symbols		Processor for the IMPORT command	*
 *									*
 * Purpose: Identify a symbol that is defined in another module.
 *	The symbol will be listed in the loader section of the
 *	output file.  The filename for such symbols will be null,
 *	indicating a "deferred resolve."
 *									*
 * Command Format:							*
 *	IMPORT name [<addr>]
 *									*
 * Arguments:								*
 *	NAME - A symbol name.
 *									*
 * Returns: Returns a status completion code.				*
 *	OK	- no error detected					*
 *	ERROR	- Argument error
 *									*
 * Side Effects:							*
 *	None								*
 *									*
 ************************************************************************/
RETCODE
import_symbols(char *arg[])	/* argv-style arguments */
{
    STR		*imported_name;
    int		import_smc = XMC_UA;
    unsigned int import_addr;
    SRCFILE	*import_srcfile;

    static OBJECT	*import_object;
    static SYMBOL	*import_sym = NULL;
    static CSECT	*import_cs;

    if (arg[2]) {
	char *ptr;
	errno = 0;
	import_addr = strtoul(arg[2], &ptr, 0);
	if (errno != 0 || *ptr != '\0') {
	    bind_err(SAY_NORMAL, RC_NI_ERROR,
		     NLSMSG(IMPORT_SYNTAX,
 "%1$s: 0711-419 The symbol address %2$s is not a valid number."),
		     Main_command_name, arg[2]);
	    return RC_NI_ERROR;
	}
	import_smc = XMC_XO;
    }

    if (!Switches.asis)
	upper(arg[1]);

    imported_name = putstring(arg[1]);
    imported_name->flags |= STR_ISSYMBOL;
    Bind_state.state |= STATE_RESOLVE_NEEDED;

    if (import_object == NULL || import_object != last_object()) {
	import_object = new_init_object(NULL);
	import_srcfile = get_init_SRCFILE(import_object, &NULL_STR);
	import_cs = get_init_CSECT();

	import_object->o_srcfiles = import_srcfile;
	import_object->o_type = O_T_SCRIPT;

	import_srcfile->sf_csect = import_cs;

	import_cs->c_secnum = N_IMPORTS; /* Imported symbols */
	import_cs->c_srcfile = import_srcfile;

	import_sym = import_name(imported_name, import_cs, NULL,
				 import_smc, import_addr, 0);
    }
    else
	import_sym = import_name(imported_name, import_cs, import_sym,
				 import_smc, import_addr, 0);

    return RC_OK;
} /* import_symbols */
/* *******************************************
 * NAME: parse_impname
 *
 * FUNCTION: Parse the string on a #! imports file
 *	     extracting the filename and membername information
 *
 *	Let ipath/ifile(imember) be the input file name.  If the file was read
 *	using the "library" command, ipath is null.  If the file is not
 *	an archive member, imember is null.  For symbols before the first #!
 *	line, the loader-section path string is ipath, ifile, imember, unless
 *	ipath is absolute, in which case the path part is null.  For lines
 *	beginning with #!, the following loader-section path strings result:
 *
 *	Input line		Result
 *	----------		------
 *	#!			Deferred resolution--return &NULL_STR
 *	#! ()			Use ipath, ifile, imember
 *			EXCEPTION: If ipath is absolute, use a null path
 *	#! (member)		Use ipath, ifile, member
 *	#! file			Use NULL,  file,  NULL
 *	#! file()		Use NULL,  file,  imember
 *	#! file(member)		Use NULL,  file,  member
 *	#! path/file		Use path,  file,  NULL
 *	#! path/file()		Use path,  file,  imember
 *	#! path/file(member)	Use path,  file,  member
 *
 *	Blanks are allowed before the '('.
 *	Extra characters are allowed after ')' or after the blank terminating
 *	the file name if no '(' is present.  No syntax errors are defined or
 *	detected for this line.
 *
 *	Note:  In the last 6 cases, it is not really necessary to distinguish
 *	between "file" and "path/file" because it is only when the loader
 *	section is written that "path" and "file" must be separated.  In fact,
 *	in the return values described below, "filename" is path/file, or just
 *	file if path is null.
 *
 * Return: STR with the name
 *	"filename[membername]"	if membername is not null
 *	"filename"		if membername is null
 *	&NULL_STR		for deferred resolution
 *	&NULL_STR		if a pathname or file name is too long
 *
 * ********************************************/
static STR *
parse_impname(char *p,			/* Line from input file (without #!) */
	      OBJECT *o,		/* Input object */
	      int *flag,		/* *flag is set to 1 if member name
					   is not null. */
	      char *def_path,		/* Full pathname to import file, if not
					   read with "library" command. */
	      char *def_basename,	/* Basename of import file */
	      char *def_member,		/* Member name of import file, if any */
	      int line_num)		/* Line number of #! line */
{

#define MEMBER_MAX 255		/* Maximum length of archive member name */

    char	fname[PATH_MAX+1], mname[MEMBER_MAX+1];
    char	*fname_ptr, *mname_ptr;
    char	*target;
    char	buf[PATH_MAX + MEMBER_MAX + 2 + 1];
    int		name_len;

    while (isspace(*p))			/* skip leading whitespace */
	++p;

    target = fname;
    name_len = PATH_MAX;

    while (*p && *p != '(' && *p != ' ') { /* Get pathname */
	if (name_len-- == 0)
	    goto empty_impname;
	if (*p == '\\')
	    ++p;

	*target++ = *p++;
    }
    *target = '\0';			/* Null terminate the pathname. */

    while (isspace(*p))			/* skip whitespace */
	++p;

    if (fname[0] == '\0') {		/* No pathname */
	if (*p == '\0')			/* Deferred - no name */
	    return &NULL_STR;
	else if (*p == '(')
	    fname_ptr = def_path;
    }
    else
	fname_ptr = fname;

    mname[0] = '\0';
    mname_ptr = mname;
    target = mname;

    if (*p == '(') {
	*flag = S_ARCHIVE_SYMBOL;
	++p;				/* Skip '(' introducing member name. */

	name_len = MEMBER_MAX;
	while (*p && *p != ')') {	/* form member name */
	    if (name_len-- == 0)
		goto empty_impname;
	    if (*p == '\\')
		++p;

	    *target++ = *p++;
	}
	*target = '\0';			/* Terminate the name. */
	if (mname[0] == '\0') {
	    mname_ptr = def_member;
	    if (fname_ptr == def_path && *fname_ptr == '/')
		fname_ptr = def_basename;
	}
    }

    if (*mname_ptr == '\0')
	if (fname_ptr == o->o_ifile->i_name->name)
	    return o->o_ifile->i_name;
	else
	    return putstring(fname_ptr);
    else {
	(void) sprintf(buf, "%s[%s]", fname_ptr, mname_ptr);
	return putstring(buf);
    }

  empty_impname:
    bind_err(SAY_NORMAL, RC_ERROR,
	     NLSMSG(IMPNAME_BAD,
    "%1$s: 0711-417 ERROR: Import object %2$s at line %3$d:\n"
    "\tThe pathname or member name specified after #! is too long.\n"
    "\tThe line is being ignored."),
	     Main_command_name, get_object_file_name(o),
	     line_num);
    return &NULL_STR;
} /* parse_impname */
/* *******************************************
 * NAME:	read_impexp_object
 * FUNCTION: Read an import/export file, importing or exporting names as
 *		appropriate.
 *
 * Return: Status code
 * ********************************************/
RETCODE
read_impexp_object(OBJECT *obj,
		   int importing,
		   off_t base_offset)
{
    static char	*id = "READ_IMPEXP_FILE";

    char	*Token[MAXARGS];
    char	*new_buf = NULL;
    char	*input_ptr, *old_input_ptr, *max_f;
    char	*f_base, *f, *f_start, *f_temp, *buffer;
    char	impname[PATH_MAX + MEMBER_MAX + 2 + 1];
    char	*p;
    char	*ptr;
    char	import_smc;
    char	fixed_buffer[LINE_MAX];
    int		archive_flag;
    int		line_num;
    int		f_len, f_remaining;
    int		clen;
    int		new_buf_len = 0;
    int		copy_file;
    int		found = 0;
    int		len;
    int		rc = RC_OK;
    int		syscall;
    long	import_addr;
    struct switches	tmp_switch;
    CSECT	*cs = NULL;
    FILE	*fp;
    SRCFILE	*cur_srcfile = NULL, *tmp_srcfile;
    STR		*impid, *name_ptr;
    char	*imppathname, *impbasename, *impmembername;
    SYMBOL	*import_sym;

    if (importing) {
	/* Save name of object and member--they're used for the
	   shared object name (unless overridden
	   by a line beginning with #!...). */
	imppathname = obj->o_ifile->i_name->name;
	impmembername = obj->o_member_info == NULL
	    ? NULL : obj->o_member_info->o_member->name;

	/* Get the basename of the file */
	if ((p = strrchr(imppathname, '/')) != NULL)
	    impbasename = ++p;
	else
	    impbasename = imppathname;

	/* If the "library" command was used, we just use the basename. */
	if (obj->o_ifile->i_library)
	    imppathname = impbasename;
    }

    archive_flag = (obj->o_contained_in_type == O_T_ARCHIVE)
	? S_ARCHIVE_SYMBOL : 0;

    /* save initial settings */
    tmp_switch = Switches;
    Bind_state.state |= STATE_PROCESSING_IMPEXP;

#ifdef READ_FILE
    if (obj->o_ifile->i_access == I_ACCESS_READ) {
	copy_file = 1;
	fp = obj->o_ifile->i_file;
	if (safe_fseek(obj->o_ifile, base_offset, SEEK_SET) != 0) {
	    obj->o_type = O_T_IGNORE;
	    rc = RC_ERROR;
	    goto all_done;
	}
	/* Ensure input_ptr < max_f  so while loop below never terminates
	   without explicit break; */
	input_ptr = fixed_buffer;
	max_f = input_ptr + 1;
    }
    else
#endif
    {
	copy_file = 0;
	input_ptr = (char *)obj->o_ifile->i_map_addr + base_offset;
	max_f = &input_ptr[obj->o_size];
    }

    import_sym = NULL;
    line_num = 0;

    /* loop through file of symbols to be imported */
    while(input_ptr < max_f) {
	if (interrupt_flag)
	    goto reading_interrupted;

	switch(copy_file) {
	  case 0:			/* Memory-mapped (or shmat() ) file */
	    old_input_ptr = input_ptr;
	    input_ptr = strchr(old_input_ptr, '\n'); /* Find end of line */
	    if (input_ptr == NULL) {	/* No newline--the last line
					   is the rest of the file. */
		clen = max_f - old_input_ptr;
		input_ptr = max_f;
	    }
	    else
		clen = input_ptr++ - old_input_ptr; /*Len doesn't count \n*/
	    f = old_input_ptr;
	    break;

#ifdef READ_FILE
	  case 1:			/* ACCESS==I_READ_FILE */
	    /* Read in the next line */
	    f_base = fixed_buffer;
	    f_len = LINE_MAX;
	    f_remaining = LINE_MAX;
	    f_start = f_base;

	  redo_fgets:
	    f_base[f_remaining - 2] = '\0'; /* Sentinel (of a sort) */
	  retry_fgets:
	    f_temp = fgets(f_start, f_remaining, fp);
	    if (f_temp == NULL) {
		if (feof(fp))
		    goto all_done;
		else if (interrupt_flag)
		    goto reading_interrupted;
		else if (errno == EINTR)
		    goto retry_fgets;
		else {
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
			     Main_command_name, "fgets", strerror(errno));
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(WHILE_READING,
		    "%1$s: 0711-987 Error occurred while reading file: %2$s"),
			     Main_command_name, obj->o_ifile->i_name->name);
		    obj->o_type = O_T_IGNORE;
		    rc = RC_SEVERE;
		    goto all_done;
		}
	    }

	    if (f_start[f_remaining - 2] != '\0') {
		/* Exactly f_remaining - 1 bytes were read. */
		if (f_start[f_remaining - 2] == '\n') {
		    f_start[f_remaining - 2] = '\0';
		    clen = f_len - 2;
		    f = f_base;
		    goto process_line;
		}

		/* We need a bigger buffer. */
		if (f_base == fixed_buffer) {
		    if (new_buf == NULL) {
			new_buf = emalloc(2 * LINE_MAX, id);
			new_buf_len = 2 * LINE_MAX;
		    }
		    memcpy(new_buf, fixed_buffer, LINE_MAX - 1);
		}
		else {
		    new_buf = realloc(new_buf, 2 * new_buf_len);
		    new_buf_len *= 2;
		}
		f_start = &f_base[f_len];
		f_base = new_buf;
		f_remaining = new_buf_len - f_len;
		f_len = new_buf_len;
		goto redo_fgets;
	    }
	    /* A full line was read.  Either the line ends in a newline or
	       the last line in the file has no newline.  Change the final
	       newline to \0 if it exists. */
	    clen = strlen(f_start);
	    if (f_start[clen-1] == '\n') {
		f_start[clen-1] = '\0';	/* Overwrite final '\n'. */
		--clen;
	    }
	    f = f_base;
	    break;
#endif
	}				/* End switch */

      process_line:

	line_num++;
	if (Switches.dbg_opt6)
	    say(SAY_NO_NLS, "%.*s", clen, f); /* Echo the line. */

	if (clen == 0 || f[0] == '*')
	    continue;	/* Skip blank lines and comments */

	if (copy_file <= 0) {
	    if (clen < sizeof(fixed_buffer)) {
		memcpy(fixed_buffer, f, clen);
		buffer = fixed_buffer;
	    }
	    else {
		if (new_buf_len < clen) {
		    if (new_buf)
			efree(new_buf);
		    new_buf = emalloc(clen, id);
		    new_buf_len = clen;
		}
		memcpy(new_buf, f, clen);
		buffer = new_buf;
	    }
	    buffer[clen] = '\0';
	}
	else
	    buffer = f;

	if (buffer[0] == '#') {
	    if (buffer[1] == '!') {
		if (!importing)
		    continue;
		archive_flag = 0;
		tmp_srcfile = NULL;
		impid = parse_impname(&buffer[2], obj, &archive_flag,
				      imppathname, impbasename, impmembername,
				      line_num);
		/* See if we can reuse the current srcfile */
		if (cur_srcfile == NULL
		    || cur_srcfile->sf_name->name != impid->name) {
		    tmp_srcfile = get_init_SRCFILE(obj, impid);
		    if (cur_srcfile)
			cur_srcfile->sf_next = tmp_srcfile;
		    else
			obj->o_srcfiles = tmp_srcfile;
		    cur_srcfile = tmp_srcfile;
		    /* Make sure we allocate a new CSECT for this SRCFILE.
		       We don't allocate it here because there may be no
		       symbols after this #! line. */
		    cs = NULL;		/* We'll need a new CSECT
					   for this SRCFILE. */
		}
		continue;
	    }

	    if (buffer[1] == ' ') {
		len = tokens(buffer, Token, 1);	/* Initialize strtok call */
		/* Pass the file name as Token[0] for error messages.
		   Token[0] must have a non-null value or bindopt() will
		   assume that it is processing the command line. */
		Token[0] = obj->o_ifile->i_name->name;
		if ((rc = bindopt(Token)) != RC_OK)
		    goto all_done;
		continue;
	    }
	}

	/* address given? */
	import_smc = XMC_UA;
	import_addr = 0;
	syscall = 0;

	len = tokens(buffer, Token, MAXTOKENS);
	switch(len) {
	  case 0:
	    continue;			/* Line is just whitespace */
	  case 1:
	    break;			/* Symbol name only */
	  case 2:
	    lower(Token[1]);
	    if (strcmp(Token[1], "svc") == 0
		|| strcmp(Token[1], "syscall") == 0)
		syscall = 1;
	    else {
		errno = 0;
		import_addr = strtoul(Token[1], &ptr, 0);
		if (errno != 0 || *ptr != '\0') {
		    bind_err(SAY_NORMAL, RC_ERROR,
			     NLSMSG(IMPORT_BAD_ADDRESS,
    "%1$s: 0711-418 ERROR: Import or export file %2$s at line %3$d:\n"
    "\tA symbol name may only be followed by an address or by the keyword\n"
    "\tsvc or syscall. The line is being ignored."),
			     Main_command_name,
			     get_object_file_name(obj), line_num);
		    continue;
		}
		syscall = 0;
		if (importing)
		    import_smc = XMC_XO;
	    }
	    break;
	  default:
	    bind_err(SAY_NORMAL, RC_ERROR,
		     NLSMSG(IMPORT_MULTIPLE,
    "%1$s: 0711-413 ERROR: Import or export file %2$s at line %3$d:\n"
    "\tOnly one symbol name per line is allowed. The symbol name may be\n"
    "\tfollowed by svc, syscall, or an address. The line is being ignored."),
		     Main_command_name, get_object_file_name(obj), line_num);
	    continue;
	}

	if (!Switches.asis)
	    upper(Token[0]);

	name_ptr = putstring(Token[0]);
	name_ptr->flags |= STR_ISSYMBOL;

	Bind_state.state |= STATE_RESOLVE_NEEDED;

	if (importing) {
	    if (cur_srcfile == NULL) {	/* No #! seen yet, so this must be 1st
					   symbol--create SRCFILE and csect. */
		if (impmembername == NULL)
		    impid = putstring(*imppathname == '/'
				      ? impbasename : imppathname);
		else {
		    sprintf(impname, "%s[%s]",
			    *imppathname == '/' ? impbasename : imppathname,
			    impmembername);
		    impid = putstring(impname);
		}
		cur_srcfile = get_init_SRCFILE(obj, impid);

		obj->o_srcfiles = cur_srcfile;
		cs = get_init_CSECT();
		import_sym = NULL;
		cur_srcfile->sf_csect = cs;
		cs->c_secnum = N_IMPORTS;
		cs->c_srcfile = cur_srcfile;
	    }
	    else if (cs == NULL) {
		/* First SYMBOL (and hence CSECT) in this SRCFILE */
		cs = get_init_CSECT();
		import_sym = NULL;
		cur_srcfile->sf_csect = cs;
		cs->c_secnum = N_IMPORTS; /* Imported symbols */
		cs->c_srcfile = cur_srcfile;
	    }
	    import_sym = import_name(name_ptr, cs, import_sym,
				     import_smc, import_addr, archive_flag);
	}
	else {
	    export_syscall = syscall;
	    export_name(name_ptr);
	}
	found++;
    }
  all_done:

    if (importing)
	say(SAY_NORMAL,
	    NLSMSG(IMPORTS_COUNT,
		   "%1$s: Symbols imported from import file %2$s: %3$d"),
	    Command_name, get_object_file_name(obj), found);
    else
	say(SAY_NORMAL, NLSMSG(EXPORTS_COUNT, "%1$s: Symbols exported: %2$d"),
	    Command_name, found);

    /* restore initial settings */
    Switches = tmp_switch;

    Bind_state.state &= ~STATE_PROCESSING_IMPEXP;
    return rc;

  reading_interrupted:
    bind_err(SAY_NORMAL, RC_NI_WARNING,
	     NLSMSG(MAIN_INTERRUPT,
		    "%1$s: 0711-635 WARNING: Binder command %2$s interrupted."),
	     Main_command_name, Command_name);
    bind_err(SAY_NORMAL, RC_NI_WARNING,
	     NLSMSG(SYMBOLS_NOT_READ,
		    "%s: 0711-416 Some symbols were not read."),
	     Main_command_name);
    bind_err(SAY_NORMAL, RC_NI_WARNING,
	     NLSMSG(WHILE_READING,
		    "%1$s: 0711-987 Error occurred while reading file: %2$s"),
	     Main_command_name,
	     get_object_file_name(obj));
    Switches = tmp_switch;
    obj->o_type = O_T_IGNORE;
    Bind_state.state &= ~STATE_PROCESSING_IMPEXP;
    return RC_NI_WARNING;
} /* read_impexp_object */
