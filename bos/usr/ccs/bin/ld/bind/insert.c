#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)97	1.11  src/bos/usr/ccs/bin/ld/bind/insert.c, cmdld, bos41B, 9504A 12/7/94 15:45:06")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS (for binder subcommands):
 *		flist
 *		keepfile
 *		nokeepfile
 *		insert
 *		library
 *		rebind
 *
 *   FUNCTIONS: handle_insert
 *		insert_deferred_files
 *		is_deferred_ifile
 *		read_object_symbols
 *
 *   STATIC FUNCTIONS:
 *		insert0
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
#include <string.h>
#include <syms.h>
#include <sys/param.h>

#include "bind.h"
#include "global.h"
#include "error.h"
#include "strs.h"
#include "insert.h"
#include "ifiles.h"
#include "objects.h"
#include "util.h"

/* Static variables */
static int	expected_file_count;	/* Per 'filelist' command */
static int	inserts_deferred;	/* Actual count */
static IFILE	*first_nonloaded_file;

/************************************************************************
 * Name: flist		Processor for the FILELIST command		*
 *									*
 * Purpose: A list of "insert" (or equivalent) commands should follow
 *	this command.  Processing of the input files is deferred until
 *	they have all been read, or until a non-insert, non-diagnostic
 *	command is read.
 *
 *	The "filelist" command provides a hint to the "insert" command	*
 *	to allow pre-allocation of some data structures. Its use is not
 *	required.
 *									*
 * Command Format:							*
 *	FILELIST <num_object_files> [<num_additional_files>]
 *									*
 * Arguments:								*
 *	<num_object_files>- Number of ordinary input files to be processed
 *			(*.o *.a files)
 *	<num_additional_files> (optional) --Number of additional files,
 *			such as import, export, and glue files.
 *	If num_object_files is 0 and num_additional_files is not specified,
 *	all deferred files are read.
 *
 * Returns: Returns a standard completion status code.			*
 *	RC_OK	    - no error detected.
 *	RC_NI_ERROR -  arguments < 0
 *									*
 ************************************************************************/
RETCODE
do_flist(int n,
	 int m)
{
    expected_file_count += n;

    /* Cause next allocation of an IFILE or OBJECT structure to
       allocate n+m contiguous structures */
    reserve_new_ifiles(n + m);
    reserve_new_objects(n + m);
    return RC_OK;
}
RETCODE
flist(char *arg[])	/* argv-style arguments */
{
    int	n, m;
    n = atoi(arg[1]);

    if (n == 0 && arg[2] == NULL)
	return insert_deferred_files();

    if (n < 0) {
	bind_err(SAY_NORMAL, RC_NI_ERROR, NLSMSG(CMD_NEG_ARG,
 "%1$s: 0711-154 ERROR: Binder command %2$s requires non-negative arguments."),
		 Main_command_name, Command_name);
	return RC_NI_ERROR;
    }

    if (arg[2]) {
	m = atoi(arg[2]);
	if (m < 0) {
	    bind_err(SAY_NORMAL, RC_NI_ERROR, NLSMSG(CMD_NEG_ARG,
 "%1$s: 0711-154 ERROR: Binder command %2$s requires non-negative arguments."),
		 Main_command_name, Command_name);
	    return RC_NI_ERROR;
	}
    }
    else
	m = 0;

    return do_flist(n, m);
} /* flist */
/************************************************************************
 * Name: NOKEEPFILE 	Processor for the NOKEEPFILE command
 *									*
 * Purpose: Reset a flag so that the symbols in a file will be garbage
 *	collected.
 *
 * Command Format:							*
 *	NOKEEPFILE fn
 *									*
 * Arguments:								*
 *	fn   - This must an exact match for a file read with the
 *	library, insert, or rebind command.
 *
 * Returns: Returns a completion status.				*
 *	RC_OK	- no error detected.					*
 *	RC_NI_WARNING - Named file not already read.
 *									*
 ************************************************************************/
RETCODE
nokeepfile(char *args[])	/* argv-style arguments */
{
    IFILE *f;

    for (f = first_ifile; f; f = f->i_next)
	if (strcmp(f->i_name->name, args[1]) == 0) {
	    if (f->i_keepfile == 1) {
		Bind_state.state |= STATE_RESOLVE_NEEDED;
		Bind_state.files_kept--;
		f->i_keepfile = 0;
	    }
	    return RC_OK;
	}

    bind_err(SAY_NORMAL, RC_NI_WARNING,
	     NLSMSG(KEEP_FILE_NOT_INSERTED,
	    "%1$s: 0711-569 WARNING: File %2$s has not been inserted yet."),
	     Main_command_name, args[1]);
    return RC_NI_WARNING;
}
/************************************************************************
 * Name: KEEPFILE 	Processor for the KEEPFILE command
 *									*
 * Purpose: Set a flag so that the symbols in a file will be garbage
 *	collected.
 *
 * Command Format:							*
 *	KEEPFILE fn
 *									*
 * Arguments:								*
 *	fn   - This must an exact match for a file read with the
 *	library, insert, or rebind command.
 *
 * Returns: Returns a completion status.				*
 *	RC_OK	- no error detected.					*
 *	RC_NI_WARNING	- Named file not already read.
 *									*
 ************************************************************************/
RETCODE
keepfile(char *args[])	/* argv-style arguments */
{
    IFILE *f;

    /* Argumentis filename */
    for (f = first_ifile; f; f = f->i_next)
	if (strcmp(f->i_name->name, args[1]) == 0) {
	    if (f->i_keepfile == 0) {
		Bind_state.state |= STATE_RESOLVE_NEEDED;
		Bind_state.files_kept++;
		f->i_keepfile = 1;
	    }
	    return RC_OK;
	}

    bind_err(SAY_NORMAL, RC_NI_WARNING,
	     NLSMSG(KEEP_FILE_NOT_INSERTED,
	    "%1$s: 0711-569 WARNING: File %2$s has not been inserted yet."),
	     Main_command_name, args[1]);
    return RC_NI_WARNING;
}
/************************************************************************
 * Name: INSERT 	Processor for the INSERT  command		*
 *									*
 * Purpose: Save the information about an input file for later processing
 * Command Format:							*
 *	I       [keep] fn
 *	INSERT  [keep] fn
 *	LIB     [keep] fn
 *	LIBRARY [keep] fn
 *	REBIND  [keep] fn
 *									*
 * Arguments:								*
 *	FN   - Name of file to process
 *	[keep] optional keyword "keep"  If specified, all the external symbols
 *		in the file will be kept, i.e., not garbage collected.
 *									*
 * Returns: Returns a completion status.				*
 *	RC_OK	- no error detected.					*
 *	RC_ERROR- No input file specified or file not found	*
 *									*
 ************************************************************************/
static RETCODE insert0(char *[], int);	/* Forward declaration */
RETCODE
insert(char *args[])			/* argv-style arguments */
{
    Command_name = "INSERT";
    return insert0(args, 0);
}
RETCODE
library(char *args[])			/* argv-style arguments */
{
    Command_name = "LIBRARY";
    return insert0(args, 1);
}
RETCODE
rebind(char *args[])			/* argv-style arguments */
{
    return insert0(args, 2);
}
static RETCODE
insert0(char *args[],			/* argv-style arguments */
	int flags)			/* 1=library, 2=rebind, 3=reinsert */
{
    int		cmd_keepfile = 0;
    IFILE	*ifile;
    HEADERS	*hdr;

    /* If we have 2 arguments and the first argument is "keep", we set
       the keep flag. */
    if (args[2]) {
	if ((args[1][0] == 'k' || args[1][0] == 'K')
	    && (args[1][1] == 'e' || args[1][1] == 'E')
	    && (args[1][2] == 'e' || args[1][2] == 'E')
	    && (args[1][3] == 'p' || args[1][3] == 'P')
	    && args[1][4] == '\0') {
	    cmd_keepfile = 1;
	    args++;
	}
    }

    ifile = ifile_open_and_map(args[1]);

    if (ifile == NULL)
	return RC_ERROR;

    switch(flags) {
      case 1: ifile->i_library = 1;	break;
      case 2: ifile->i_rebind = 1;	break;
      case 3: ifile->i_reinsert = 1;	break;
    }

    if (cmd_keepfile) {
	ifile->i_keepfile = 1;
	Bind_state.files_kept++;
    }

    switch(set_ifile_type(ifile, &hdr)) {
      case I_T_IGNORE:
	/* This case is only here to handle minimal object files, that
	   is, ones that consist of nothing but a header.  If "setopt verbose"
	   is used, a warning will have been printed, and the command's return
	   code will be picked up from the error level in the message call.
	   Otherwise, nothing will have been printed, and the return code for
	   the command will be RC_OK. */
	return RC_OK;
      case I_T_UNKNOWN:
	return RC_ERROR;
    }

    return handle_insert(ifile, hdr);
} /* insert0 */
/************************************************************************
 * Name: handle_insert
 *
 * Purpose: Allocate OBJECTS for the given IFILE.  If not deferring inserts,
 *		expected_file_count will be 0, and the file will be read.  If
 *		we've read the last file expected by a filelist command, we
 *		read all the deferred files.
 *
 * Returns: RC_OK.  Errors are set by bind_err() calls.
 ************************************************************************/
RETCODE
handle_insert(IFILE *ifile,
	      HEADERS *hdr)
{
    allocate_ifile_objects(ifile, hdr);

    /* If any errors were encountered, the file's state will now be CLOSED */
    if (ifile->i_closed)
	return RC_ERROR;

    if (first_nonloaded_file == NULL)
	first_nonloaded_file = ifile;

    if (expected_file_count > 0)
	expected_file_count--;

    if (expected_file_count == 0)
	return insert_deferred_files();

    inserts_deferred++;
    return RC_OK;
}
/************************************************************************
 * Name: is_deferred_ifile
 *									*
 * Purpose: See if reading of IFILE 'f' has been deferred.
 *
 * Return Codes: Returns 1 if reading is deferred; 0 otherwise.
 *
 ************************************************************************/
int
is_deferred_ifile(IFILE *f)
{
    IFILE *ifile;

    for (ifile = first_nonloaded_file; ifile
#ifdef LD_BIND
	 && ifile->i_type != I_T_UNKNOWN
#endif
	 ; ifile = ifile->i_next) {
	if (ifile == f)
	    return 1;
    }
    return 0;
}
/************************************************************************
 * Name: insert_deferred_files
 *
 * Purpose: Read the symbols from all previously unread files.
 *
 * Returns: RC_OK.  Error level is set by bind_err() calls.
 ************************************************************************/
RETCODE
insert_deferred_files(void)
{
    int		count = 0;
    IFILE	*ifile;

    if (first_nonloaded_file == NULL) {
	expected_file_count = 0;
	return RC_OK;
    }

    Bind_state.state |= STATE_RESOLVE_NEEDED;

    /* Read symbols from files */
    for (ifile = first_nonloaded_file; ifile
#ifdef LD_BIND
	 && ifile->i_type != I_T_UNKNOWN
#endif
	 ; ifile = ifile->i_next) {

	/* See whether an error was already detected with the file. */
	if (ifile->i_closed)
	    continue;

	/* Make sure file is open and mapped */
	if (ifile_reopen_remap(ifile) == RC_OK) {
	    OBJECT *o = ifile->i_objects;

	    if (o->o_num_subobjects != 0) {	/* Composite object */
		switch(o->o_type) {
		  case O_T_ARCHIVE:
		    read_archive_symbols(o);
		    break;
		  case O_T_IGNORE:	/* If composite object was corrupted */
		    break;
		  default:		/* No other composite objects */
		    internal_error();
		}
	    }
	    else
		read_object_symbols(o, 0 /* base offset */);

	    if (ifile->i_rebind)
		Bind_state.state |= STATE_REBIND_USED;
	}
	count++;
    }
    if (inserts_deferred)
	say(SAY_NORMAL, NLSMSG(FILELIST_COUNT,
	      "%1$s: Number of previously inserted files processed: %2$d"),
	    "FILELIST", count);

    first_nonloaded_file = NULL;
    inserts_deferred = 0;
    expected_file_count = 0;

    return RC_OK;
} /* insert_deferred_files */
/************************************************************************
 * Name: read_object_symbols
 *
 * Purpose: Read the symbols from a single object.  The object cannot be
 *	a composite object but it can be a member of a composite object.
 *
 *	Call the proper routine based on the type of the object.
 *
 * Side effects:  If the object is a shared object or a script file and
 *	it is not a member of a composite object, the file containing
 *	the object is closed, because it won't be needed any more.
 *
 * Returns: Status code
 ************************************************************************/
RETCODE
read_object_symbols(OBJECT *o,		/* Object to read */
		    off_t base_offset)	/* Offset (within IFILE) to object */
{
    int i;
    int rc = RC_OK;
    int close_ok = 0;

    switch(o->o_type) {
      case O_T_SHARED_OBJECT:
	read_shared_object_symbols(o, base_offset);
	close_ok = 1;			/* No further use for input object */
	break;

      case O_T_OBJECT:
	/* If an error occurs, o->o_type will be set to O_T_IGNORE.  */
	read_xcoff_symbols(o, base_offset, NULL);
	for (i = 1; o->o_type != O_T_IGNORE && i <= o->oi_num_sections; i++)
	    if (o->oi_section_info[i-1].l_reloc_count > 0)
		read_section_rlds(o, i);
	break;

      case O_T_SCRIPT:
	rc = read_impexp_object(o, 1 /*import*/, base_offset);
	close_ok = 1;			/* No further use for input object */
	break;

      case O_T_EXPORT:
	rc = read_impexp_object(o, 0 /*export*/, base_offset);
	close_ok = 1;			/* No further use for input object */
	break;

      case O_T_IGNORE:
	break;				/* Error already detected */

      case O_T_UNKNOWN:
      default:
	internal_error();
	rc = RC_ERROR;
	o->o_type = O_T_IGNORE;
    }
    if (close_ok || o->o_type == O_T_IGNORE)
	if (o->o_member_info == NULL)
	    ifile_close_for_good(o->o_ifile);

    return rc;
} /* read_object_symbols */
