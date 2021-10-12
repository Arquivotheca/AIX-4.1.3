#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)02	1.52  src/bos/usr/ccs/bin/ld/bind/save.c, cmdld, bos41B, 9505A 1/23/95 16:00:19")
#endif

/*
 * COMPONENT_NAME: (CMDLD) bind - xcoff linkage editor
 *
 * FUNCTIONS (for binder subcommands):
 *			savename
 *			save
 *
 * GLOBAL FUNCTIONS:	init_save
 *
 * STATIC FUNCTIONS:	open_output_file
 *			setup_temp_file
 *			finish_output_file
 *			mk_loader
 *			cancel
 *			cancel2
 *			mk_rbd_dbg
 *			add_overflow_section
 *			write_rld_info
 *			compare_sym_names
 *			ro_text
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>

#ifdef XLC_BUG
#undef strncpy
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/errno.h>

#include "bind.h"
#include "strs.h"
#include "error.h"
#include "global.h"

#include "objects.h"
#include "ifiles.h"
#include "symbols.h"
#include "save.h"
#include "dump.h"
#include "util.h"

/* Typedefs */
typedef struct output_file_info {
    mode_t outfile_mode;
    mode_t mask;
    char use_tmp_dir;
    char output_is_new;
    struct stat stats;
    char *dir_name;			/* Temp file dir name */
    char outfile[PATH_MAX+1];		/* Actual output file name */
} output_file_info;

/* Global variables */
uint16	next_scn;			/* Next section in output file */
caddr_t	Shmaddr;			/* Global shmat address for output */
short	sect_mappings[MS_MAX+1];
SCNHDR	pad_section = {{'\0'}, 0, 0, 0, 0, 0, 0, 0, 0, STYP_PAD};
#ifdef DEBUG
int	imp_exp_symbols;		/* # of imported and exported syms */
int	XO_imports;			/* # of imported, saved, XO symbols */
#endif
int	imp_symbols;			/* # of imported symbols */

/* Global Pointers to parts of the output XCOFF file */
SCNHDR	*Scnhdr;
FILHDR	*Filehdr;

/* Static variables */
static int	xcoff_flags = 0;	/* Flags in XCOFF header */
static int	savename_called = 0;
static int	Fd = -1;		/* File descriptor number for output */
static off_t	max_file_size;
static ulong	num_text_rlds, num_data_rlds;
static int	Fd1 = -1;		/* Outfile if temp file needed. */
static output_file_info ofi;
static int	rld_counts[2];		/* Number of rld entries in .loader
					   section for .text and .data
					   sections.  We only really need to
					   know whether there are any .text
					   section RLDs for the -bro option. */

/* Forward declarations */
static void	add_overflow_section(uint16, long, long);
static X_OFF_T	write_rld_info(X_OFF_T);
static int	cancel(RLD *, SYMBOL *);
static RETCODE	open_output_file(char *, mode_t);
static RETCODE	setup_temp_file(char **, mode_t);
static RETCODE	finish_output_file(int, int);
static RETCODE	mk_loader(X_OFF_T *);
static X_OFF_T	mk_rbd_dbg(X_OFF_T);
static RETCODE	ro_text(void);

/************************************************************************
 * Name: init_save	Initialize things needed by the save routines
 *									*
 * Purpose: Perform any initialization required by SAVENAME or SAVE.
 *									*
 * Function:  Initializes the loadtoc instructions and no-op instructions
 *		recognized after an in-module BL instruction.
 ************************************************************************/
void
init_save(void)
{
    static char *id = "init_save";

    Bind_state.loadtocs = emalloc(10 * sizeof(*Bind_state.loadtocs), id);
    Bind_state.max_loadtocs = 10;
    /* First entry in array is default entry.  There are no other valid
       entries now, but if there were, any other valid entry found while
       relocating code would be changed to this first entry when a call is
       out-of-module. The TOCLOAD command is used to modify this array. */
    Bind_state.loadtocs[0] = 0x80410014; /* L R2, 20(R1) ???? */
    Bind_state.num_loadtocs = 1;

    Bind_state.nops = emalloc(10 * sizeof(*Bind_state.nops), id);
    Bind_state.max_nops = 10;
    /* First entry in array is default entry.  Any other valid entry found
       while relocating code is changed to this first entry when a call is
       not out-of-module. The NOP command is used to modify this array. */
    Bind_state.nops[0] = 0x60000000;	/* ori 0,0,0, */;
    Bind_state.nops[1] = 0x4FFFFB82;	/* cror 31,31,31 */
    Bind_state.nops[2] = 0x4DEF7B82;	/* cror 15,15,15 */
    Bind_state.num_nops = 3;
} /* init_save */
/************************************************************************
 * Name: savename		Processor for the SAVENAME command
 *									*
 * Purpose: Designate an output file, so it can be checked without
 * actually writing to it.
 *									*
 * Command Format:							*
 *	SAVENAME fn
 *									*
 * Parms/Returns:							*
 *	Returns: Returns a status completion code.			*
 *		0 - OK if file name is OK
 *		12- SEVERE if file name won't be writable
 *									*
 ************************************************************************/
RETCODE
savename(char *arg[])			/* argv-style arguments */
{
    int rc;

    if (arg[1] == NULL) {
	if (savename_called > 0)
	    say(SAY_NORMAL,
		NLSMSG(SAVENAME_NAME, "%1$s: The output filename is %2$s"),
		Command_name, ofi.outfile);
	else
	    say(SAY_NORMAL,
		NLSMSG(SAVENAME_NONAME,
		       "%s: No output filename has been specified yet."),
		Command_name);
	return RC_OK;
    }

    if (savename_called > 0) {
	bind_err(SAY_NORMAL, RC_NI_WARNING,
		 NLSMSG(COMMAND_ONCE,
"%1$s: 0711-874 WARNING: The binder command %2$s may not be called more than once."),
		 Main_command_name, Command_name);
	return RC_NI_WARNING;
    }
    else if (savename_called == 0) {
	ofi.mask = umask(0);	/* get current umask setting */
	(void) umask(ofi.mask); /* reset back to current setting */
    }

    rc = open_output_file(arg[1], 0777 & ~ofi.mask /* Default mode*/);
    if (rc == RC_OK)
	savename_called = 1;
    else
	savename_called = -1;		/* Called erroneously */

    return rc;
} /* savename */
/************************************************************************
 * Name: save			Processor for the SAVE command		*
 *									*
 * Purpose: Write an XCOFF relocatable module from the results of the	*
 *	previous bind steps.						*
 *									*
 * Command Format:							*
 *	SAVE module_type fn
 *									*
 * Parms/Returns:							*
 *	Input:	MT - Module type (1L - Single Load			*
 *				  SR - Serially Reusable		*
 *				  RE - Reentrant			*
 *				  RC - Recursively Callable		*
 *				  (single copy satisfies all loads	*
 *					from a process))		*
 *		FN - Name of file to process				*
 *									*
 *	Returns: Returns a status completion code.			*
 *		0 - OK no error detected.				*
 *		4 - WARNING class of error detected.			*
 *		8 - ERROR class of error detected.			*
 *		12- SEVERE class of error detected.			*
 *									*
 ************************************************************************/
RETCODE
save(char *arg[])			/* argv-style arguments */
{
    int		shared;
    int		rc;
    int		change_mod;
    X_OFF_T	output_offset;		/* Next offset to raw data in output */
    OBJECT	*obj;
#ifdef _CPUTYPE_FEATURE
    SRCFILE	*sf;
#endif

    if (Bind_state.state & STATE_RESOLVE_NEEDED) {
	bind_err(SAY_NORMAL, RC_NI_ERROR,
		 NLSMSG(RESOLVE_NEEDED,
 "%1$s: 0711-300 ERROR: RESOLVE must be called before calling binder command %2$s."),
		 Main_command_name, Command_name);
	return RC_NI_ERROR;
    }

    if (Bind_state.state & STATE_SAVE_CALLED) {
	bind_err(SAY_NORMAL, RC_WARNING,
		 NLSMSG(COMMAND_ONCE,
 "%1$s: 0711-874 WARNING: The binder command %2$s may not be called more than once."),
		 Main_command_name, Command_name);
	return RC_WARNING;
    }

    if (savename_called == 0) {
	ofi.mask = umask(0);	/* get current umask setting */
	(void) umask(ofi.mask); /* reset back to current setting */
    }

    if (Switches.execute && Bind_state.retcode < RC_ERROR) {
	ofi.outfile_mode = 0777 & ~ofi.mask;
	change_mod = 1;			/* Execute bits should be set
					   in output file. */
    }
    else if (savename_called != 1) {
	ofi.outfile_mode = 0666 & ~ofi.mask;
	change_mod = 0;
    }
    else if ((0666 & ~ofi.mask) != ofi.outfile_mode) {
	/* The output file is already open and if it was a new file,
	   it was created with the wrong mode bits. */
	change_mod = -1;		/* Execute bits should not be set. */
	ofi.outfile_mode = 0666 & ~ofi.mask;
    }

    if (savename_called == 0
	|| (savename_called == -1
	    && (arg[2][0] != '.' || arg[2][1] != '\0'))) {
	if ((rc = open_output_file(arg[2], ofi.outfile_mode)) != RC_OK)
	    return rc;
    }
    else if (arg[2][0] == '.' && arg[2][1] == '\0') {
	if (savename_called == -1) {
	    bind_err(SAY_NORMAL, RC_ERROR,
		     NLSMSG(SAVE_BADNAME1,
		    "%1$s: 0711-876 ERROR: Filename %2$s,\n"
		    "\tspecified by the SAVENAME command, was invalid."),
		     Main_command_name, ofi.outfile);
	    return RC_ERROR;
	}
    }
    else if (strcmp(arg[2], ofi.outfile) != 0) {
	bind_err(SAY_NORMAL, RC_ERROR,
		 NLSMSG(SAVE_BADNAME2,
	"%1$s: 0711-877 ERROR: Filename must be %2$s\n"
	"\tUse the same name as specified with the SAVENAME binder command."),
		 Main_command_name, ofi.outfile);
	return RC_ERROR;
    }
    if ((rc = setup_temp_file(&Shmaddr, ofi.outfile_mode)) != RC_OK)
	return rc;

    /* Assign address to Filehdr in the mapped output file */
    Filehdr = (FILHDR *) Shmaddr;
    Bind_state.out_writing = 1;	/* So signal catcher knows we're writing */
    Filehdr->f_magic = U802TOCMAGIC;	/* Set file type. */

    /* Set size of aux header */
    if (Switches.execute)
	Filehdr->f_opthdr = sizeof(AOUTHDR);
    else
	Filehdr->f_opthdr = offsetof(AOUTHDR, o_toc); /* Limited header */

    Scnhdr = (SCNHDR *)(Shmaddr + FILHSZ + Filehdr->f_opthdr);

    /* save module type field for aouthdr */
    upper(arg[1]);
    shared = (strlen(arg[1]) == 3 && arg[1][0] == 'S');
    temp_aout_hdr.o_modtype[0] = arg[1][shared];
    temp_aout_hdr.o_modtype[1] = arg[1][shared+1];

    Bind_state.state |= STATE_SAVE_CALLED;

    /* At this point, collect_and_save_csects is called.  It is
       responsible for writing out the .text, .data and .bss sections.
       It will also update the in-memory section headers.  It returns the
       offset to the next available byte in the output file.  If it returns 0,
       TOC overflow occurred, and the output file was not completely written.
       The temporary file is deleted. */
    output_offset = collect_and_save_csects();
    if (output_offset == 0) {
	(void) shmdt(Shmaddr);
	(void) close(Fd);
	if (Fd1 > -1)
	    (void) close(Fd1);
	if (Bind_state.out_tmp_name)
	    (void) unlink(Bind_state.out_tmp_name);
	Bind_state.out_writing = 0;
	return RC_SEVERE;
    }

#ifdef _CPUTYPE_FEATURE
    /* Compute output CPUTYPE and print included object files if -m flag
       used. */
    if (Switches.member)
	say((Switches.quiet && !Bind_state.ever_interactive)
		? SAY_NORMAL : SAY_STDOUT,
	    NLSMSG(MEMBERS,
		   "%s: These object files and archive members were\n"
		   "\tused to create the output file:"),
	    Command_name);

    for (obj = first_object(); obj; obj = obj->o_next) {
	if (obj->o_type == O_T_OBJECT
	    && ((obj->oi_flags & (OBJECT_GLUE | OBJECT_USED))
		== OBJECT_USED)) {
	    if (Switches.member)
		say(SAY_NO_NLS | (Bind_state.ever_interactive ? 0 : SAY_STDOUT),
		    " %s", get_object_file_name(obj));
	    for (sf = obj->o_srcfiles; sf; sf = sf->sf_next) {
		if (sf->sf_flags & SF_USED) {
		    switch(sf->sf_cputype) {
		      case TCPU_ANY:
			temp_aout_hdr.o_cpuflag |= TOBJ_COMPAT;
			break;
		      default:
			/* Here's where the cputype calculation takes place. */
		    }
		}
	    }
	}
    }

    if (temp_aout_hdr.o_cpuflag & TOBJ_SAME) {
	if (temp_aout_hdr.o_cputype == 0)
	    temp_aout_hdr.o_cputype = TCPU_PWR;
    }
    else {
	temp_aout_hdr.o_cputype = TCPU_COM; /* Arbitrary? */
	bind_err(SAY_NORMAL, RC_WARNING,
		 NLS-MSG(?, "0711-... WARNING Mix of CPU types"));
    }
#else
    if (Switches.member) {
	say((Switches.quiet && !Bind_state.ever_interactive)
		? SAY_NORMAL : SAY_STDOUT,
	    NLSMSG(MEMBERS,
		   "%s: These object files and archive members were\n"
		   "\tused to create the output file:"),
	    Command_name);
	for (obj = first_object(); obj; obj = obj->o_next) {
	    if (obj->o_type == O_T_OBJECT
		&& ((obj->oi_flags & (OBJECT_GLUE | OBJECT_USED))
		    == OBJECT_USED))
		say(SAY_NO_NLS | (Bind_state.ever_interactive ? 0 : SAY_STDOUT),
		    " %s", get_object_file_name(obj));
	}
    }
#endif

    /* Set up values for sect_mappings[] */
    sect_mappings[MS_TEXT] = temp_aout_hdr.o_sntext;
    sect_mappings[MS_DATA] = temp_aout_hdr.o_sndata;
    sect_mappings[MS_BSS] = temp_aout_hdr.o_snbss;
    sect_mappings[MS_EXTERN] = N_UNDEF;

    /* If desired, create the loader section */
    if (Switches.execute) {
	/* attempt to create the loader section */
	if (mk_loader(&output_offset) == RC_OK) {
	    if (Switches.ro_text)	/* -bro option used */
		ro_text();		/* Print messages if necessary */

	    temp_aout_hdr.o_snloader = next_scn; /* 1-based */
	    xcoff_flags |= F_EXEC | F_DYNLOAD;
	}
    }

    output_offset =  ROUND(output_offset, 2); /* Pad to half-word boundary */

    if (Switches.strip)
	xcoff_flags |= F_RELFLG | F_LNNO;
    else {
	/* Create the rebind and debug info. */
	Bind_state.state |= STATE_RBD_DBG_CALLED; /* Set flag to cause
						     mk_rbd_dbg to use a
						     different algorithm for
						     ifile_open_remap(). */
	output_offset = mk_rbd_dbg(output_offset);
    }

    max_file_size = output_offset;
    Bind_state.out_writing = 1;

    /* See if first header section needs to be adjusted */
    if (next_scn > 0 && Scnhdr[0].s_flags == STYP_PAD) {
	output_offset = FILHSZ + Filehdr->f_opthdr + next_scn * SCNHSZ;
	if (Scnhdr[0].s_scnptr > output_offset) {
	    DEBUG_MSG(OUTPUT_DEBUG,
		      (SAY_NO_NLS,
		       "Resetting length of first pad section from %d to %d",
		       Scnhdr[0].s_size,
		       Scnhdr[0].s_size + Scnhdr[0].s_scnptr - output_offset));
	    Scnhdr[0].s_size += Scnhdr[0].s_scnptr - output_offset;
	    Scnhdr[0].s_scnptr -= Scnhdr[0].s_scnptr - output_offset;
	}
    }

    /* Complete the filehdr */
    Filehdr->f_nscns = next_scn;
    Filehdr->f_flags = xcoff_flags | (shared ? F_SHROBJ : 0);
    if (Switches.dbg_opt9)
	Filehdr->f_timdat = 0;		/* Use 0 to make comparisons easier */
    else
	Filehdr->f_timdat = time(NULL);

    /* Complete the optional header */
    if (Bind_state.entrypoint_sym) {
	temp_aout_hdr.o_entry = Bind_state.entrypoint_sym->s_addr
	    + (Bind_state.entrypoint_sym->s_csect->c_new_addr
	      - Bind_state.entrypoint_sym->s_csect->c_addr);
	temp_aout_hdr.o_snentry
	    = sect_mappings[Bind_state.entrypoint_sym->s_csect->c_major_sect];
    }

    if (Bind_state.o_toc_sym) {
	temp_aout_hdr.o_toc = Bind_state.o_toc_sym->s_addr
	    + (Bind_state.o_toc_sym->s_csect->c_new_addr
	       - Bind_state.o_toc_sym->s_csect->c_addr);
	temp_aout_hdr.o_sntoc
	    = sect_mappings[Bind_state.o_toc_sym->s_csect->c_major_sect];
    }

    memcpy(Shmaddr + FILHSZ, &temp_aout_hdr, Filehdr->f_opthdr);

    Bind_state.out_writing = 0;

    /* Compute a return code.  Since we are in the middle of the save command,
       we must examine cmd_err_lev to determine whether the original output
       file should be overwritten or whether the output file should be made
       executable. */
    rc = Bind_state.cmd_err_lev;
    if (rc > RC_NI_BASE)
	rc -= RC_NI_BASE;
    if (rc < Bind_state.retcode)
	    rc = Bind_state.retcode;
    if (rc >= RC_ERROR) {
	if ((0666 & ~ofi.mask) != ofi.outfile_mode) {
	    change_mod = -1;		/* Execute should not be set. */
	    ofi.outfile_mode = 0666 & ~ofi.mask;
	}
    }
    /* Do not free Queue, since the printing routines may use it. */

    return finish_output_file(change_mod, rc);
} /* save */
/************************************************************************
 * Name: open_output_file	Open the output file to see if it is
 *				writable.
 *									*
 * Purpose: Open (or create) and check the output file.
 *	NOTE:  If there are any errors, we want to make sure to return
 *		RC_NI_SEVERE and not RC_NI_ERROR so the the binder will
 *		exit under normal halt values.  Otherwise, it doesn't do
 *		us any good to do this early checking.
 *									*
 * Returns:  RC_OK if file will be acceptable.
 *	     RC_NI_SEVERE otherwise
 ************************************************************************/
static RETCODE
open_output_file(char *arg,		/* Filename */
		 mode_t mode)		/* Mode (for creation only) */
{
    char	*msg;
    struct stat	temp_stats;

    /* Save the output filename */
    if (*arg == '\\') {
	/* The filename contains escapes:  we have to process the name */
	if (unescape_pathname(ofi.outfile, PATH_MAX, arg) == NULL)
	    goto name_too_long;
    }
    else {
	if (strlen(arg) > PATH_MAX) {
	  name_too_long:
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(FILE_NAMETOOLONG,
		    "%1$s: 0711-878 SEVERE ERROR: The pathname is too long.\n"
		    "\tThe maximum length of a pathname is %2$d.\n"
		    "\tBinder command %3$s cannot be executed."),
		     Main_command_name, PATH_MAX, Command_name);
	    return RC_NI_SEVERE;
	}
	(void) strcpy(ofi.outfile, arg);
    }

    ofi.outfile_mode = mode;
    Bind_state.out_name = &ofi.outfile[0];

    ofi.use_tmp_dir = 0;
    if (statx(ofi.outfile, &ofi.stats, 0, STX_NORMAL) == -1) {
	if (errno == ENOENT) {
	    DEBUG_MSG(OUTPUT_DEBUG,
		      (SAY_NO_NLS,
		       "statx failed for %s because it doesn't exist.",
		       ofi.outfile));
	    Bind_state.flags |= FLAGS_UNLINK_OUT;
	  retry1:
	    /* Note:  5/24/93: O_TRUNC flag is required if file is in AFS. */
	    Bind_state.out_Fd = open(ofi.outfile,
				     /* Note:  O_EXCL is not used in case
					the file name is a symbolic link
					naming a non-existant file. */
				     O_RDWR | O_CREAT | O_TRUNC,
				     mode);
	    Fd = Bind_state.out_Fd;
	    if (Fd == -1) {
		if (errno == EINTR)
		    goto retry1;
		Bind_state.out_Fd = 0;
		Bind_state.flags &= ~FLAGS_UNLINK_OUT;
		bind_err(SAY_NORMAL, RC_NI_SEVERE,
			 NLSMSG(SAVE_BAD_OUTPUT,
       "%1$s: 0711-934 SEVERE ERROR: Cannot create the output file: %2$s\n"
       "\t%1$s:open() %3$s"),
			 Main_command_name, ofi.outfile, strerror(errno));
		Bind_state.out_name = NULL;
		return RC_NI_SEVERE;
	    }
	    if (fstatx(Fd, &ofi.stats, 0, STX_NORMAL) == -1) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
				"\t%1$s:%2$s() %3$s"),
			 Main_command_name, "fstatx", strerror(errno));
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(WHILE_PROCESSING,
		"%1$s: 0711-996 Error occurred while processing file: %2$s"),
			 Main_command_name, ofi.outfile);

		(void) close(Fd);
		Bind_state.out_Fd = 0;
		(void) unlink(ofi.outfile);
		Bind_state.flags &= ~FLAGS_UNLINK_OUT;
		Bind_state.out_name = NULL;
		return RC_SEVERE;
	    }
	    if (ofi.stats.st_flag & FS_REMOTE) {
		DEBUG_MSG(OUTPUT_DEBUG,
			  (SAY_NO_NLS, "New File %s is remote", ofi.outfile));
		Fd1 = Fd;
		Fd = -1;
		ofi.use_tmp_dir = 1;
	    }
	    ofi.output_is_new = 1;
	}
	else {
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(SAVE_BADNAME,
    "%1$s: 0711-933 SEVERE ERROR: Pathname %2$s is invalid or inaccessible.\n"
			    "\t%1$s:%3$s() %4$s"),
		     Main_command_name, ofi.outfile, "statx", strerror(errno));
	    Bind_state.out_name = NULL;
	    return RC_NI_SEVERE;
	}
    }
    else {
	ofi.output_is_new = 0;
	if (!(S_ISREG(ofi.stats.st_mode))) {
	    bind_err(SAY_NORMAL, RC_NI_SEVERE,
		     NLSMSG(SAVE_BADTYPE,
			    "%1$s: 0711-875 SEVERE ERROR: Output file: %2$s\n"
			    "\tThe output file must be a regular file."),
		     Main_command_name, ofi.outfile);
	    Bind_state.out_name = NULL;
	    return RC_NI_SEVERE;
	}
	DEBUG_MSG(OUTPUT_DEBUG,
		  (SAY_NO_NLS, "Output file %s already exists", ofi.outfile));

	/* Open the output file for write access */
      retry2:
	Bind_state.out_Fd = open(ofi.outfile, O_WRONLY, mode);
	Fd1 = Bind_state.out_Fd;
	if (Fd1 == -1) {
	    Bind_state.out_Fd = 0;
	    switch(errno) {
	      case EINTR:
		goto retry2;
	      case EROFS:
		msg = msg_get(NLSMSG(SAVE_READONLY,
		     "%1$s: 0711-936 SEVERE ERROR: Output file: %2$s\n"
		     "\tThe file is on a read-only file system."));
		break;
	      case ETXTBSY:
		msg = msg_get(NLSMSG(SAVE_RENMTMPTOOUT1,
		     "%1$s: 0711-851 SEVERE ERROR: Output file: %2$s\n"
		     "\tThe file is in use and cannot be overwritten."));
		break;
	      case EACCES:
		msg = msg_get(NLSMSG(FILE_WR_PROTECT,
		     "%1$s: 0711-931 SEVERE ERROR: Output file: %2$s\n"
		     "\tThe file is write-protected."));
		break;
	      default:
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(BAD_SYSCALL,
		"%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
		"\t%1$s:%2$s() %3$s"),
			 Main_command_name, "open", strerror(errno));
		msg = msg_get(NLSMSG(WHILE_PROCESSING,
	     "%1$s: 0711-996 Error occurred while processing file: %2$s"));
		break;
	    }
	    bind_err(SAY_NO_NLS, RC_NI_SEVERE, msg,
		     Main_command_name, ofi.outfile);
	    Bind_state.out_name = NULL;
	    return RC_NI_SEVERE;
	}

	/* The output file exists--we'll need to open a temporary file.  If the
	   output file is remote, we'll use /tmp for the temp file.
	   Otherwise, we'll use the same directory as the output file. */
	if (ofi.stats.st_flag & FS_REMOTE) {
	    DEBUG_MSG(OUTPUT_DEBUG,
		      (SAY_NO_NLS, "Existing file %s is remote", ofi.outfile));
	    ofi.use_tmp_dir = 1;
	}
    }

    return RC_OK;
} /* open_output_file */
/************************************************************************
 * Name: setup_temp_file
 *									*
 * Purpose:  Open an output file.  It will be a temporary file unless the
 *	output file does not already exist.
 *									*
 * Function:
 ************************************************************************/
static RETCODE
setup_temp_file(char **retvalue,
		mode_t mode)
{
    static char	dirbuf[BUFSIZ];
    char	*uniq_name = NULL;	/* Temp file path name */

    if (ofi.output_is_new == 0) {
	if (Bind_state.retcode > RC_ERROR) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SAVE_NOSAVE1,
    "%1$s: 0711-937 SEVERE ERROR: Severe errors have occurred and the output\n"
    "\tfile (%2$s) already exists. The file will not be overwritten."),
		     Main_command_name, ofi.outfile);
	    return RC_SEVERE;
	}
	else if (Bind_state.retcode > RC_WARNING &&
		 (Bind_state.state & STATE_REBIND_USED)) {
	    bind_err(SAY_NORMAL, RC_ERROR,
		     NLSMSG(SAVE_NOSAVE2,
    "%1$s: 0711-938 ERROR: Errors have occurred and the output file %2$s,\n"
    "\twhich already exists, was used as an input file.\n"
    "\tThe file will not be overwritten."),
		     Main_command_name, ofi.outfile);
	    return RC_ERROR;
	}
    }

    /* See if we need a temporary file */
    if (Fd == -1) {
	if (ofi.use_tmp_dir) {
	  use_tmp_dir:
	    ofi.dir_name = NULL;
	    uniq_name = tempnam(NULL, "b."); /* Use /tmp (or TMPDIR) */
	}
	else {
	    if ((ofi.dir_name = strrchr (ofi.outfile, '/')) != NULL) {
		/* Find where to write terminating null before modifying
		   ofi.dir_name.  */
		dirbuf[ofi.dir_name - &ofi.outfile[0]] = '\0';
		ofi.dir_name = strncpy(dirbuf, ofi.outfile,
				       ofi.dir_name - &ofi.outfile[0]);
		if (ofi.dir_name[0] == '\0') {
		    if (ofi.outfile[0] == '/')
			ofi.dir_name = "/../";
		}
	    }
	    else
		ofi.dir_name = "./";

	    uniq_name = tempnam(ofi.dir_name, "b.");
	}

      retry3:
	/* Set up to delete output object file on exception */
	Bind_state.out_tmp_name = uniq_name;
	Bind_state.flags |= FLAGS_UNLINK_TMP;
	/* Note:  5/24/93: O_TRUNC flag is required if file is in AFS. */
	Bind_state.temp_out_Fd = open(uniq_name,
				      O_RDWR | O_CREAT | O_EXCL | O_TRUNC,
				      mode);
	Fd = Bind_state.temp_out_Fd;

	if (Fd < 0) {
	    if (errno == EINTR
		|| ((errno == ENFILE || errno == EMFILE)
		    && ifile_close_one() == 1))
		goto retry3;
	    Bind_state.flags &= ~FLAGS_UNLINK_TMP;
	    Bind_state.out_tmp_name = NULL;
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SAVE_TMP_CREAT,
    "%1$s: 0711-925 SEVERE ERROR: Cannot create temporary output file: %2$s\n"
    "\t%1$s:open() %3$s"),
		     Main_command_name, uniq_name, strerror(errno));
	    efree(uniq_name);
	    return RC_SEVERE;
	}

	DEBUG_MSG(OUTPUT_DEBUG,
		  (SAY_NO_NLS, "Opened temporary file %s for output",
		   uniq_name));
    }

#ifndef CENTERLINE
    /* Memory map the file */
    if ((*retvalue = shmat(Fd, 0, SHM_MAP)) == (char *)-1 ) {
	if (errno == EMFILE) {
	    free_segments(1);
	    /* Try again */
	    *retvalue = shmat(Fd, 0, SHM_MAP);
	}
    }
    if (*retvalue == (char *)-1 ) {
	DEBUG_MSG(OUTPUT_DEBUG, (SAY_NO_NLS, "Couldn't shmat %s",
				 uniq_name ? uniq_name : ofi.outfile));

	if (ofi.use_tmp_dir == 1) {	/* We've already tried using /tmp */
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SAVE_NOSHMATMP,
    "%1$s: 0711-929 SEVERE ERROR: Cannot map the temporary output file: %2$s\n"
    "\t%1$s:%3$s() %4$s"),
		     Main_command_name, uniq_name, "shmat", strerror(errno));
	    (void) unlink(uniq_name);
	    Bind_state.flags &= ~FLAGS_UNLINK_TMP;
	    Bind_state.out_tmp_name = NULL;
	    efree(uniq_name);
	    return RC_SEVERE;
	}
	else if (uniq_name) {
	    (void) unlink(uniq_name);
	    Bind_state.flags &= ~FLAGS_UNLINK_TMP;
	    Bind_state.out_tmp_name = NULL;
	    efree(uniq_name);
	    close(Fd);
	    Bind_state.temp_out_Fd = 0;
	}
	else
	    Fd1 = Fd;
	ofi.use_tmp_dir = 1;
	goto use_tmp_dir;
    }
#else
    *retvalue = emalloc(2000000, "dummy");
#endif /* CENTERLINE */

    return RC_OK;
} /* setup_temp_file */
/************************************************************************
 * Name: finish_output_file
 *									*
 * Purpose:  Close the temporary file, if any, and copy it to the
 *	real output file.
 *									*
 * Function:
 ************************************************************************/
static RETCODE
finish_output_file(int change_mode,
		   int rc)
{
    char	new_file_name[PATH_MAX];
    int		s;

    if (ofi.output_is_new == 0) {
	int close_and_delete = 0;

	if (rc > RC_ERROR) {
	    bind_err(SAY_NORMAL, RC_ERROR,
		     NLSMSG(SAVE_NOSAVE1,
    "%1$s: 0711-937 SEVERE ERROR: Severe errors have occurred and the output\n"
    "\tfile (%2$s) already exists. The file will not be overwritten."),
		     Main_command_name, ofi.outfile);
	    close_and_delete = 1;
	}
	else if (rc > RC_WARNING &&
		 (Bind_state.state & STATE_REBIND_USED)) {
	    bind_err(SAY_NORMAL, RC_ERROR,
		     NLSMSG(SAVE_NOSAVE2,
    "%1$s: 0711-938 ERROR: Errors have occurred and the output file %2$s,\n"
    "\twhich already exists, was used as an input file.\n"
    "\tThe file will not be overwritten."),
		     Main_command_name, ofi.outfile);
	    close_and_delete = 1;
	}
	if (close_and_delete) {
	    (void) close(Fd1);		/* Close the output file--
					   it was not touched. */
	    (void) close(Fd);		/* Close the temporary file. */
	    (void) unlink(Bind_state.out_tmp_name); /* Delete the temp. file. */
	    Bind_state.flags &= ~FLAGS_UNLINK_TMP;
	    efree(Bind_state.out_tmp_name);
	    Bind_state.out_tmp_name = NULL;
	    return RC_ERROR;
	}
    }

    if (Bind_state.out_tmp_name == NULL) {
	DEBUG_MSG(OUTPUT_DEBUG,
		  (SAY_NO_NLS, "File %s written to directly--closing",
		   ofi.outfile));

	/* The output file is new.  If it was created before we knew that
	   the return code would be 8 or greater, it was created with
	   execute bits set.  We reset them now.  We don't worry about
	   errors, because the exit status will be non-zero anyway. */
	if (change_mode == -1)
	    (void) fchmod(Fd, ofi.outfile_mode);

#ifdef CENTERLINE
	write(Fd, Shmaddr, max_file_size);
	close(Fd);
#else
	(void) shmdt(Shmaddr);
	if (ftruncate(Fd, max_file_size) != 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
		     Main_command_name, "ftruncate", strerror(errno));
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(WHILE_PROCESSING,
	    "%1$s: 0711-996 Error occurred while processing file: %2$s"),
		     Main_command_name, ofi.outfile);
	    /* Don't delete output file.  It may contain useful information. */
	    (void) close(Fd);		/* If this fails, we don't want
					   another error message. */
	}
	else if (close(Fd) != 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(BAD_SYSCALL,
	    "%1$s: 0711-999 SEVERE ERROR: Unexpected system call error.\n"
	    "\t%1$s:%2$s() %3$s"),
		     Main_command_name, "close", strerror(errno));
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(WHILE_CLOSING,
	  "%1$s: 0711-997 Error occurred while closing file: %2$s"),
		     Main_command_name, ofi.outfile);
	    /* Don't delete output file.  It may contain useful information. */
	}
#endif
    }
    else {
	/* DEFECT 166969: We never use rename() to give the output
	   file the proper name, because the owner and group of an
	   existing output file may not be the same as the owner and
	   group of the temporary file.  (While we could check to see
	   if the existing output file already has the proper owner
	   and group, it doesn't seem worth the effort.) */
	DEBUG_MSG(OUTPUT_DEBUG,
		  (SAY_NO_NLS, "Copying temporary file to output file %s",
		   ofi.outfile));
	/* Truncate the existing output file */
	if (ftruncate(Fd1, 0) != 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(FILE_UNEXPECTED_ERROR,
		    "%1$s: 0711-753 SEVERE ERROR: Unexpected I/O error\n"
		    "\t%1$s:%2$s() %3$s"),
		     Main_command_name, "ftruncate", strerror(errno));
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(WHILE_PROCESSING,
	    "%1$s: 0711-996 Error occurred while processing file: %2$s"),
		     Main_command_name, ofi.outfile);

	    /* Close and delete the temporary file. */
	    (void) close(Fd);
	    (void) unlink(Bind_state.out_tmp_name);
	    Bind_state.flags &= ~FLAGS_UNLINK_TMP;
	    efree(Bind_state.out_tmp_name);
	    Bind_state.out_tmp_name = NULL;
	    Bind_state.temp_out_Fd = 0;
	    Bind_state.out_Fd = 0;
	    return RC_SEVERE;
	}

	/* Copy from the mapped temporary file to the output file. */
	for (rc = 0, s = max_file_size; s > 0 && rc != -1; s -= rc)
	    rc = write(Fd1, Shmaddr, s);

	if (rc == -1) {
	    switch(errno) {
	      case ENOSPC:
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(FILE_SYSTEM_FULL,
			"%s: 0711-750 SEVERE ERROR: The file system is full."),
			 Main_command_name);
		break;
	      case EDQUOT:
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(FILE_QUOTA,
	"%s: 0711-751 SEVERE ERROR: You have exceeded your disk quota."),
			 Main_command_name);
		break;
	      case EFBIG:
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(FILE_ULIMIT,
	"%s: 0711-752 SEVERE ERROR: You have exceeded your filesize ulimit."),
			 Main_command_name);
		break;
	      default:
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(FILE_UNEXPECTED_ERROR,
			"%1$s: 0711-753 SEVERE ERROR: Unexpected I/O error\n"
			"\t%1$s:%2$s() %3$s"),
			 Main_command_name, "write", strerror(errno));
	    }
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(WHILE_WRITING_OUTPUT,
    "%1$s: 0711-993 Error occurred while writing to the output file: %2$s"),
		     Main_command_name, Bind_state.out_name);
	    /* Don't leave a partially written output file around. */
	    close(Fd1);
	    (void) unlink(ofi.outfile);
	    Bind_state.flags &= ~FLAGS_UNLINK_OUT;
	}

#ifdef CENTERLINE
	efree(Shmaddr);
#else
	(void) shmdt(Shmaddr);
#endif

	close(Fd);			/* Close the temporary file */
	(void) unlink(Bind_state.out_tmp_name);	/* And delete it */
	Bind_state.flags &= ~FLAGS_UNLINK_TMP;
	efree(Bind_state.out_tmp_name);
	Bind_state.out_tmp_name = NULL;
	if ((ofi.stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO))
	    != ofi.outfile_mode) {
	    /* The existing output file does not have the proper mode bits,
	       so we set them here. */
	    if (fchmod(Fd1, ofi.outfile_mode) != 0
		&& change_mode == 1) {
		bind_err(SAY_NORMAL, RC_ERROR,
			 NLSMSG(SAVE_NOFCHMOD,
	"%1$s: 0711-856 ERROR: Cannot set the mode of the output file: %2$s\n"
	    "\t%1$s:fchmod() %3$s"),
		     Main_command_name, ofi.outfile, strerror(errno));
		/* Don't return yet.  Try to close the output file. */
	    }
	}

	/* If close fails, we don't want an empty or truncated target file,
	   so print msg and unlink the target file. */
	if (close(Fd1) < 0) {		/* Close the output file */
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SAVE_NOCLOSE2,
	    "%1$s: 0711-855 SEVERE ERROR: Cannot close the output file: %2$s\n"
	    "\t%1$s:close() %3$s"),
		     Main_command_name, ofi.outfile, strerror(errno));
	    (void) unlink(ofi.outfile);
	}
    }

    Bind_state.temp_out_Fd = 0;
    Bind_state.out_Fd = 0;
    Bind_state.state |= STATE_SAVE_COMPLETE;
    Bind_state.flags &= ~FLAGS_UNLINK_OUT;
    return RC_OK;
} /* finish_output_file */
/****************************************************************************
 *	mk_loader - make the loader section if desired
 *
 *		This routine is responsible for building the loader section
 *	of an XCOFF object.  It will be called only if a loader section is
 *	desired.
 ***************************************************************************/
static RETCODE
mk_loader(X_OFF_T *next_offset)
{
    char	*id = "mk_loader";
    char	*string_base;
    char	*p, *q;
    char	**strs;
    int16	tmp_size;
    int		i, j;
    int		sym_flags;
    int		limit, limits[2];
    int		*saved_er_inpndx;
    int		inpndx_names = 0;
    struct {
	SYMBOL	*symbol;
	int	inpndx;
    } *saved_inpndx;
    int		num_lstrs = 0;		/* Number of lesd name strings */
    caddr_t	file_offset;		/* offset for ldr section pieces */
    long	next_off;
    unsigned int pad;
    CSECT	*cs;
    RLD		*rld;
    STR		*file_name, **limps, *orig_name;
    STR		*loader_name, *alt_name;
    SYMBOL	*sym;
    TYPECHK	*t;

    LDHDR	*ldhdr;	/* Pointer for loader section header */
    LDSYM	*lesd;	/* Pointer for current loader symbol table entry */
    LDREL	*ldrel;	/* Pointer for current loader relocation entry */
    struct strs {
	union {
	    STR *s;
	    TYPECHK *t;
	} u;
	int kind;
    } *lstrs;			/* Pointers to used names and typchks */
    struct sm {
	int impid_v;
	int name_v;
    } *string_mappings;

    int		num_STRs;
    int		sm_count = 0;
    int		num_limps = 1;		/* Number of import ID strings
					   (1st is implicit for LIBPATH) */
    long	num_lesds = 3,		/* Number of loader esds
					   (1st 3 are implicit) */
		num_lrels = 0,		/* Number of loader relocs */
		ldr_size,		/* Size of loader section */
		str_size = 0,		/* Loader string table size
					   (& typechecks) */
		imp_size;		/* Loader import ID size */

    if (
#ifdef GENERATE_PAD_HEADERS
	next_scn + 1 >= Filehdr->f_nscns
#else
	next_scn >= Filehdr->f_nscns
#endif
	) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(SAVE_MISCOUNT1,
		"%s: 0711-885 INTERNAL ERROR: Number of sections miscounted.\n"
		"\tThe .loader section cannot be written."),
		 Main_command_name);
	internal_error();
    }

    next_off = *next_offset;		/* Next offset to write */

    if (Bind_state.lpad_align < 4)
	pad = 4;			/* Silently align to word boundary */
    else
	pad = Bind_state.lpad_align;

    if (next_off % pad > 0) {
	pad = pad - (next_off % pad);
#ifdef GENERATE_PAD_HEADERS
	if (pad > 0 && Bind_state.lpad_align >= 4) {
	    Bind_state.out_writing = 1;
	    Scnhdr[next_scn] = pad_section;
	    Scnhdr[next_scn].s_scnptr = next_off;
	    Scnhdr[next_scn].s_size = pad;
	    Bind_state.out_writing = 0;
	    ++next_scn;
	}
#endif
	next_off += pad;
    }

    /* If we have loader information strings, calculate their length */
    if (Bind_state.ldrinfo != NULL) {
	for (strs = Bind_state.ldrinfo; *strs; ++strs)
	    str_size += strlen(*strs) + 1 + 2; /* 1 for \0; 2 for len field */
    }

    /* Calculate mapped address of beginning of loader section. */
    file_offset = Shmaddr + next_off;
    ldhdr = (LDHDR *)file_offset;

    file_offset += LDHDRSZ;
    lesd = (LDSYM *)file_offset;

    /* Allocate memory to hold the following:
     * lstrs	Needed symbol name loader section offset.
     * limps	Needed loader import file id loader section offset.
     */

    num_STRs = total_STRS();

    /* These arrays can only be used once per STR */
    limps = emalloc(num_STRs * sizeof(STR *), id);
    string_mappings = emalloc(num_STRs * sizeof(struct sm), id);

    /* This array holds information about strings and type-checking values */
    lstrs = emalloc((num_STRs + Bind_state.num_typechks) * sizeof(struct strs),
		    id);
    saved_inpndx
	= emalloc((1 /* entry point */ + imp_symbols + Bind_state.num_exports)
		  * sizeof(saved_inpndx[0]), id);

    Bind_state.out_writing = 1;
    for (i = 0; i < Queue_size; ++i) {
	cs = Queue[i];
	for (sym = &cs->c_symbol; sym; sym = sym->s_next_in_csect) {
	    /* Only consider marked, global symbols */
	    if ((sym->s_flags & (S_HIDEXT | S_MARK)) != S_MARK)
		continue;

	    orig_name = sym->s_name;

	    if ((orig_name->flags & (STR_ENTRY|STR_EXPORT|STR_IMPORT)) == 0)
		continue;

	    /* Don't export undefined symbols */
	    if (orig_name->flags & STR_NO_DEF)
		continue;

	    /* Don't ever export GLUE code */
	    if (sym->s_smclass == XMC_GL)
		continue;

	    /* If '.foo' and 'foo' both exist and are marked, but only one
	       was imported, we only add the name as it appeared in the import
	       file to the loader section.  Exception:  If '.foo' was imported
	       but 'foo' is a generated descriptor, then 'foo' is added to
	       the loader section and '.foo' is not. */
	    if ((orig_name->flags & STR_IMPORT)
		&& (alt_name = orig_name->alternate) != NULL
		&& !(alt_name->flags & STR_NO_DEF)
		&& (alt_name->first_ext_sym->s_flags & S_MARK)
		&& (sym->s_smclass != XMC_DS
		    || sym->s_inpndx != INPNDX_GENERATED)) {
		if (alt_name->first_ext_sym->s_inpndx == INPNDX_GENERATED
		    && alt_name->first_ext_sym->s_smclass == XMC_DS) {
		    DEBUG_MSG(SAVE_DEBUG,
			      (SAY_NO_NLS,
	       "Skipping imported symbol because descriptor was generated: %s",
			       orig_name->name));
		    continue;
		}
		if (sym->s_inpndx == INPNDX_GENERATED) {
		    DEBUG_MSG(SAVE_DEBUG,
			      (SAY_NO_NLS,
			       "Skipping generated, imported symbol: %s",
			       orig_name->name));
		    continue;
		}
	    }

	    DEBUG_MSG(SAVE_DEBUG,
		      (SAY_NO_NLS,
		       "Generating loader symbol table entry%s%s%s for %s",
		       (orig_name->flags & STR_IMPORT) ? " IMPORT" : "",
		       (orig_name->flags & STR_EXPORT) ? " EXPORT" : "",
		       (orig_name->flags & STR_ENTRY) ? " ENTRY" : "",
		       orig_name->name));

	    /* If LOADER rename command was used,
	       we should use a different name here. */
	    loader_name = orig_name;

#if 0
	    /* This code is broken.  It needs changes for lexport to work
	       properly. */
	    if (loader_name->flags & LOADER_RENAME)
		loader_name = (STR *)loader_name->str_value;

	    if (loader_name == NULL) {
		if (Switches.verbose)
		    bind_err(SAY_NORMAL, RC_WARNING,
			     NLSMSG(EXPORT_IN_USE,
    "%1$s: 0711-873 WARNING: Symbol %2$s cannot be exported as symbol %3$s.\n"
    "\tAnother instance of symbol %3$s is already exported."),
			     Main_command_name, s->name);
		continue;
	    }
#endif
	    /* Set up name */
	    if (loader_name->len == 0)
		internal_error();
	    else if (loader_name->len <= SYMNMLEN) {
		/* Name will fit in ldsym */
		/* Copy name, padding with nulls if necessary. */
		strncpy(lesd->l_name, loader_name->name, loader_name->len);
	    }
	    else {
		/* name goes into string table) */
		lesd->l_zeroes = 0;
#ifdef DEBUG
		if (loader_name->flags & LOADER_USED) {
		    /* This should never happen, because a name should only
		       appear once in the loader symbol table. */
		    internal_error();
		}
#endif
		switch(loader_name->flags & (LOADER_IMPID_USED | LOADER_USED)) {
		  case 0:
		    /* New instance  */
		    loader_name->flags |= LOADER_USED;
		    lstrs[num_lstrs].u.s = loader_name;
		    lstrs[num_lstrs++].kind = 1;
		    loader_name->str_value = str_size + 2;
		    str_size += loader_name->len + 3;
		    /* Fall through */
		  case LOADER_USED:
		    lesd->l_offset = loader_name->str_value;
		    break;

		  case LOADER_IMPID_USED:
		    /* New instance  */
		    loader_name->flags |= LOADER_USED;
		    lstrs[num_lstrs].u.s = loader_name;
		    lstrs[num_lstrs++].kind = 1;

		    /* The str_value field of loader_name is in use.  This can
		       only occur if the import file name is the same as the
		       variable name. */
		    string_mappings[sm_count].impid_v = loader_name->str_value;
		    loader_name->str_value = sm_count;
		    string_mappings[sm_count++].name_v = str_size + 2;
		    str_size += loader_name->len + 3;
		    /* Fall through */

		  case (LOADER_IMPID_USED | LOADER_USED):
		    lesd->l_offset
			= string_mappings[loader_name->str_value].name_v;
		    break;
		}
	    }

	    /* Never write XMC_TD as a storage-mapping class for a symbol
	       in the loader section. */
	    if (sym->s_smclass == XMC_TD)
		lesd->l_smclas = XMC_RW;
	    else
		lesd->l_smclas = sym->s_smclass;

	    /* establish type and section */
	    if (imported_symbol(sym)) {
		lesd->l_smtype = XTY_ER |
		    (orig_name->flags & (L_ENTRY | L_EXPORT | L_IMPORT));
		lesd->l_scnum = 0;
		if (sym->s_flags & S_XMC_XO) {
		    lesd->l_value = sym->s_addr;
		    lesd->l_smclas = XMC_XO;
		}
		else
		    lesd->l_value = 0;
	    }
	    else {
		lesd->l_smtype = sym->s_smtype
		    | (orig_name->flags & (L_ENTRY | L_EXPORT | L_IMPORT));
		lesd->l_scnum = sect_mappings[cs->c_major_sect];
		lesd->l_value = sym->s_addr + (cs->c_new_addr - cs->c_addr);
	    }

	    /* Check for exported system call */
	    if ((orig_name->flags & STR_SYSCALL)
		&& (sym->s_smclass == XMC_DS || sym->s_smtype == XTY_IF))
		lesd->l_smclas = XMC_SV;

	    if (orig_name->flags & STR_IMPORT) { /* Import file name */
		file_name = cs->c_srcfile->sf_name;

		if (file_name == &NULL_STR)
		    lesd->l_ifile = 0;	/* none */
		else {
		    switch(file_name->flags & (LOADER_IMPID_USED|LOADER_USED)) {
		      case 0:
			limps[num_limps] = file_name;
			file_name->str_value = num_limps++;
			file_name->flags |= LOADER_IMPID_USED;
			/* Fall through */
		      case LOADER_IMPID_USED:
			lesd->l_ifile = file_name->str_value;
			break;
		      case LOADER_USED:
			limps[num_limps] = file_name;
			string_mappings[sm_count].name_v
			    = file_name->str_value;
			file_name->str_value = sm_count;
			string_mappings[sm_count++].impid_v = num_limps++;
			file_name->flags |= LOADER_IMPID_USED;
			/* Fall through */
		      case LOADER_IMPID_USED | LOADER_USED:
			lesd->l_ifile
			    = string_mappings[file_name->str_value].impid_v;
			break;
		    }
		}
	    }
	    else
		lesd->l_ifile = 0;	/* none */

	    if (sym->s_typechk) {	/* Parameter type check */
		t = sym->s_typechk;
		if (t->t_value == -1
		    || typchk_values[t->t_value].ldr_val == 0) {
		    if (t->t_value == -1) {
			t->t_value = num_used_typchks;
			typchk_values[num_used_typchks].sect_val = 0;
			typchk_values[num_used_typchks++].t = t;
		    }

		    /* First time for this typchk */
		    typchk_values[t->t_value].ldr_val = str_size + 2;
		    str_size += 2 + t->t_len;
		    lstrs[num_lstrs].u.t = t;
		    lstrs[num_lstrs++].kind = 0;
		}
		lesd->l_parm = typchk_values[t->t_value].ldr_val;
	    }
	    else
		lesd->l_parm = 0;	/* none */

#ifdef DEBUG
	    if (imported_symbol(sym)) {
		if (sym->s_inpndx != INPNDX_IMPORT
		    && sym->s_inpndx != INPNDX_IMPORT_TD
		    && sym->s_inpndx != INPNDX_GENERATED)
		    internal_error();
	    }
#endif
	    /* Save old s_inpndx and assign new one. */
	    saved_inpndx[inpndx_names].symbol = sym;
	    saved_inpndx[inpndx_names++].inpndx = sym->s_inpndx;
	    sym->s_inpndx = num_lesds++; /* Count number of loader symbols */
	    lesd = (LDSYM *)((caddr_t)lesd + LDSYMSZ); /* Point to next sym. */
	}
    }

    saved_er_inpndx = emalloc(last_unresolved * sizeof(int), id);
    /* Put unresolved symbols in loader symbol table */
    for (i = 0; i < last_unresolved; ++i) {
	sym = unresolved_queue[i];
	orig_name = sym->s_name;
#ifdef DEBUG
	if (!(orig_name->flags & STR_ER_QUEUED))
	    internal_error();
#endif
	DEBUG_MSG(SAVE_DEBUG,
		  (SAY_NO_NLS,
	   "Generating loader symbol table entry for undefined symbol %s",
		   orig_name->name));

	loader_name = orig_name;

#if 0
	/* If LOADER rename command was used,
	   we should use a different name here. */

	/* Broken code */
	if (loader_name->flags & LOADER_RENAME)
	    loader_name = (STR *)loader_name->str_value;

	if (loader_name == NULL) {
	    if (Switches.verbose)
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(EXPORT_IN_USE,
 "%1$s: 0711-873 WARNING: Symbol %2$s cannot be exported as symbol %3$s.\n"
 "\tAnother instance of symbol %3$s is already exported."),
			 Main_command_name, orig_name->name, "?");
	    continue;
	}
#endif
	/* Set up name */
	if (loader_name->len == 0)
	    internal_error();
	else if (loader_name->len <= SYMNMLEN) {
	    /* Name will fit in ldsym */
	    /* Copy name, padding with nulls if necessary. */
	    strncpy(lesd->l_name, loader_name->name, loader_name->len);
	}
	else {
	    /* name goes into string table) */
	    lesd->l_zeroes = 0;

#ifdef DEBUG
	    if (loader_name->flags & LOADER_USED) {
		/* This should never happen, because a name should only
		   appear once in the loader symbol table. */
		bind_err(SAY_NO_NLS, RC_SEVERE,
		 "Symbol being entered into loader symbol table twice: %s",
			 loader_name->name);
		internal_error();
	    }
#endif
	    switch(loader_name->flags & (LOADER_IMPID_USED | LOADER_USED)) {
	      case 0:
		/* New instance  */
		loader_name->flags |= LOADER_USED;
		lstrs[num_lstrs].u.s = loader_name;
		lstrs[num_lstrs++].kind = 1;
		loader_name->str_value = str_size + 2;
		str_size += loader_name->len + 3;
		/* Fall through */
	      case LOADER_USED:
		lesd->l_offset = loader_name->str_value;
		break;

	      case LOADER_IMPID_USED:
		/* New instance  */
		loader_name->flags |= LOADER_USED;
		lstrs[num_lstrs].u.s = loader_name;
		lstrs[num_lstrs++].kind = 1;

		/* The str_value field of loader_name is in use.  This can
		   only occur if the import file name is the same as the
		   variable name. */
		string_mappings[sm_count].impid_v = loader_name->str_value;
		loader_name->str_value = sm_count;
		string_mappings[sm_count++].name_v = str_size + 2;
		str_size += loader_name->len + 3;
		/* Fall through */

	      case (LOADER_IMPID_USED | LOADER_USED):
		lesd->l_offset = string_mappings[loader_name->str_value].name_v;
		break;
	    }
	}

	/* establish type and section */
	/*Defect 125284: Undo fix for defect 75447.  Treat all undefined
	  symbols in the loader section as if they were deferred imports.
	  This is the behavior of the 3.2.5 binder. */
#if 0
	lesd->l_smtype = XTY_ER | (orig_name->flags & (L_ENTRY | L_EXPORT));
#else
	lesd->l_smtype
	    = XTY_ER | L_IMPORT | (orig_name->flags & (L_ENTRY | L_EXPORT));
#endif
	lesd->l_scnum = N_UNDEF;
	lesd->l_value = 0;
	lesd->l_smclas = sym->s_smclass;

	lesd->l_ifile = 0;	/* none */

	if (sym->s_typechk) {	/* Parameter type check */
	    t = sym->s_typechk;
	    if (t->t_value == -1
		|| typchk_values[t->t_value].ldr_val == 0) {
		if (t->t_value == -1) {
		    t->t_value = num_used_typchks;
		    typchk_values[num_used_typchks].sect_val = 0;
		    typchk_values[num_used_typchks++].t = t;
		}

		/* First time for this typchk */
		typchk_values[t->t_value].ldr_val = str_size + 2;
		str_size += 2 + t->t_len;
		lstrs[num_lstrs].u.t = t;
		lstrs[num_lstrs++].kind = 0;
	    }
	    lesd->l_parm = typchk_values[t->t_value].ldr_val;
	}
	else
	    lesd->l_parm = 0;	/* none */

	saved_er_inpndx[i] = sym->s_inpndx;
	sym->s_inpndx = num_lesds++;	/* Count number of loader symbols */
	lesd = (LDSYM *)((caddr_t)lesd + LDSYMSZ); /* Point to next sym. */
    }

    file_offset += ((num_lesds-3) * LDSYMSZ);

    /* Write RLDs */
    limits[0] = first_data_index - 1;
    limits[1] = last_data_index;
    i = first_text_index;
    for (j = 0; j < 2; ++j) {
	limit = limits[j];
	for ( ; i <= limit; ++i) {
	    cs = Queue[i];
	    for (rld = cs->c_first_rld; rld; rld = rld->r_next) {
		if (rld->r_flags & RLD_CANCELED)
		    continue;

		if (rld->r_flags & RLD_RESOLVE_BY_NAME)
		    if (rld->r_sym->s_name->flags & STR_NO_DEF) {
			/* sym_flags not needed to compute ldrel->l_symndx
			   for undefined symbols. */
			sym = NULL;
		    }
		    else {
			sym = rld->r_sym->s_name->first_ext_sym;
			sym_flags = sym->s_name->flags;
		    }
		else {
		    sym_flags = 0;
#ifdef DEBUG
		    if (!(rld->r_sym->s_flags & S_RESOLVED_OK))
			internal_error();
#endif
		    sym = rld->r_sym->s_resolved;
		    if (sym == NULL) {
			/* Symbol must have been deleted by delcsect.
			   No RLD is written. */
			rld->r_flags |= RLD_CANCELED;
			continue;
		    }
		}

		if (sym && cancel(rld, sym))
		    continue;

		ldrel = (LDREL *)file_offset;

		/* set address and type */
		ldrel->l_vaddr = rld->r_addr + cs->c_new_addr - cs->c_addr;
		ldrel->l_rtype = ((rld->r_flags & RLD_SIGNED) ? 1 << 15 : 0)
		    | ((unsigned short)(rld->r_length-1)<<8)
			| rld->r_reltype;

		if (sym == NULL) {
		    /* The symbol-table index for an undefined symbol is saved
		       in the symbol's first reference. */
		    ldrel->l_symndx = rld->r_sym->s_name->refs->s_inpndx;
		}
		else if ((sym_flags & (STR_IMPORT | STR_EXPORT)) == 0)
		    ldrel->l_symndx = sym->s_csect->c_major_sect;
		else if (sym_flags & STR_IMPORT) {
		    if (sym->s_inpndx < 0) {
#ifdef DEBUG
			if (sym->s_name->alternate == NULL
			    || (sym->s_name->alternate->flags & STR_NO_DEF)
			    || (sym->s_name->alternate->first_ext_sym->s_inpndx
				< 0))
			    internal_error();
#endif
			ldrel->l_symndx
			    = sym->s_name->alternate->first_ext_sym->s_inpndx;
		    }
		    else
			ldrel->l_symndx = sym->s_inpndx;
		}
		else {			/* Must be exported */
		    /* If the loader rename command was used, we must get
		       the inpndx of the symbol with the new name.
		       Furthermore, the new name might have its own inpndx
		       (as an import), so we may need to save it in a new
		       place. */
		    ldrel->l_symndx = sym->s_inpndx;
		}

		/* Determine section number */
		ldrel->l_rsecnm = sect_mappings[rld->r_csect->c_major_sect];

		file_offset += LDRELSZ;
		num_lrels++;
	    } /* for rlds */
	} /* for text and data section RLDs */
	rld_counts[j] = num_lrels;
    }

    /* Now, restore the modified s_inpndx values */
    for (i = 0; i < last_unresolved; ++i)
	unresolved_queue[i]->s_inpndx = saved_er_inpndx[i];
    efree(saved_er_inpndx);
    for (i = 0; i < inpndx_names; ++i)
	saved_inpndx[i].symbol->s_inpndx = saved_inpndx[i].inpndx;
    efree(saved_inpndx);

    /* Don't delete unresolved_queue.
       It might be needed for creating symbol maps. */

    /* Now we need to write out the Import file ID strings */
    imp_size = 0;

    /* Write impid 0 (the LIBPATH string) */
    if ((p = Bind_state.libpath) == NULL && (p = getenv("LIBPATH")) == NULL)
	p = DEFLIBPATH;

    string_base = (char *)file_offset;
    imp_size = 0;
    strcpy(&string_base[imp_size], p);
    imp_size += strlen(p) + 1;
    string_base[imp_size++] = '\0';	/* No BASE or MEMBER for LIBPATH */
    string_base[imp_size++] = '\0';

    for (i = 1; i < num_limps; i++) {
	STR *name;

	name = limps[i];

	if ((p = strrchr(name->name, '/')) == NULL)
	    string_base[imp_size++] = '\0';	/* no path */
	else if (p == name->name) {
	    /* Absolute path name */
	    string_base[imp_size++] = '/';
	}
	strcpy(&string_base[imp_size], name->name);
	if (p) {
	    /* Overwrite last / in pathname */
	    p = &string_base[imp_size + (p-name->name) + 1];
	    p[-1] = '\0';
	}
	else
	    p = &string_base[imp_size];
	imp_size += name->len + 1;

	if ((q = strchr(p, '[')) == NULL) {
	    /* No member name */
	    string_base[imp_size++] = '\0';
	}
	else {
	    *q = '\0';	/* Terminate base name */
	    imp_size--;
	    string_base[imp_size-1] = '\0';	/* Overwrite ']' */
	}

    } /* for imps */

    file_offset += imp_size;

    /* Now write out string table */

    /* If we have loader information strings, put them in the string table
       first. */
    if (Bind_state.ldrinfo != NULL) {
	for (strs = Bind_state.ldrinfo; *strs; ++strs) {
	    tmp_size = strlen(*strs) + 1; /* Add 1 for '\0' */
	    memcpy((char *)file_offset, (char *)&tmp_size, 2);
	    file_offset += 2;
	    /* Copy string to file (along with terminating null) */
	    strcpy((char *)file_offset, *strs);
	    file_offset += tmp_size;
	    if (Switches.verbose) {
		say(SAY_NORMAL, NLSMSG(SAVE_LDRINFO,
		       "%1$s: LDRINFO string added to .loader section: %2$s"),
		    Command_name, *strs);
	    }
	}
    }

    for (i = 0; i < num_lstrs; i++) {
	if (lstrs[i].kind == 1) { /* A STR */
	    tmp_size = lstrs[i].u.s->len + 1; /* Add 1 for NULL */

	    /* Copy string length to file */
	    memcpy((char *)file_offset, (char *)&tmp_size, 2);
	    file_offset += 2;

	    /* Copy string to file (along with terminating null) */
	    strcpy((char *)file_offset, lstrs[i].u.s->name);
	    file_offset += tmp_size;
	}
	else {			/* A TYPECHK string */
	    /* get length of string */
	    tmp_size = lstrs[i].u.t->t_len;

	    /* Copy length first, then typchk string */
	    memcpy((char *)file_offset, (char *)&lstrs[i].u.t->t_len, 2);
	    if (tmp_size == TYPCHKSZ) {
		/* Typchk string/len can be copied directly */
		memcpy((char *)file_offset + 2,
		       (char *)&lstrs[i].u.t->t_typechk,
		       TYPCHKSZ);
	    }
	    else {
		memcpy((char *)file_offset + 2,
		       tmp_size <= sizeof(TYPCHK)
		       ? lstrs[i].u.t->t_c_typechk
		       : lstrs[i].u.t->t_cp_typechk,
		       tmp_size);
	    }
	    file_offset += tmp_size + 2;
	}
    } /* for strs */

    *next_offset =  (char *)file_offset - Shmaddr;

    /* Now fill in the loader header ldhdr */
    ldhdr->l_version = LOADER_VERSION;
    ldhdr->l_nsyms = num_lesds-3;
    ldhdr->l_nreloc = num_lrels;

    ldr_size = LDHDRSZ + (num_lesds-3) * LDSYMSZ + num_lrels *  LDRELSZ;

    ldhdr->l_istlen = imp_size;
    ldhdr->l_nimpid = num_limps;
    ldhdr->l_impoff = imp_size ? ldr_size : 0;
    ldr_size += imp_size;

    ldhdr->l_stlen = str_size;
    ldhdr->l_stoff = ldhdr->l_stlen ? ldr_size : 0;
    ldr_size += ldhdr->l_stlen;

    /* Finally complete the section header for the loader section */
    /* Copy section name, padding with nulls if necessary. */
    (void) strncpy(Scnhdr[next_scn].s_name, _LOADER, sizeof(Scnhdr->s_name));
    Scnhdr[next_scn].s_paddr = 0;
    Scnhdr[next_scn].s_vaddr = 0;
    Scnhdr[next_scn].s_size = ldr_size;
    Scnhdr[next_scn].s_scnptr = next_off;
    Scnhdr[next_scn].s_relptr = 0;
    Scnhdr[next_scn].s_lnnoptr = 0;
    Scnhdr[next_scn].s_nreloc = 0;
    Scnhdr[next_scn].s_nlnno = 0;
    Scnhdr[next_scn].s_flags = STYP_LOADER;
    next_scn++;				/* Next available section */

    Bind_state.out_writing = 0;

    /* free up memory used */
    efree(lstrs);
    efree(limps);

    return RC_OK;
} /* mk_loader */
/************************************************************************
 * Name: cancel2
 *									*
 * Purpose: See if a symbol referenced by one RLD can be canceled by
 *	a symbol referenced by another RLD.  The first symbol is passed
 *	as an argument.  The second symbol is determined by the RLD passed
 *	as an argument.
 *									*
 * Returns:	1, if RLDs should be canceled (shouldn't go in .loader section)
 *		0, otherwise
 *
 ************************************************************************/
static int
cancel2(SYMBOL *sym,
	RLD *rld2)
{
    int		major_sect1, major_sect2;
    SYMBOL	*sym2;

    if (rld2->r_flags & RLD_RESOLVE_BY_NAME)
	if (rld2->r_sym->s_name->flags & STR_NO_DEF)
	    return 0;			/* A RLD for an undefined symbol will
					   always be written to the loader
					   section */
	else
	    sym2 = rld2->r_sym->s_name->first_ext_sym;
    else {
#ifdef DEBUG
	if (!(rld2->r_sym->s_flags & S_RESOLVED_OK))
	    internal_error();
#endif
	sym2 = rld2->r_sym->s_resolved;
	if (sym2 == NULL) {
	    /* Symbol must have been deleted by delcsect.
	       No RLD is written.  We don't have to do this
	       here, but it saves executing the code again
	       in mk_loader(). */
	    rld2->r_flags |= RLD_CANCELED;
	    return 0;
	}
    }

    /* Get a symbol's major section.  Since an imported symbol has a
       c_major_csect of MS_EXTERN, we use the actual loader-section symbol
       number for the symbol.  For ordinary symbols, we subtract 1 from
       c_major_sect to avoid a false match between an export symbol and
       imported symbol 3 (the current value of MS_EXTERN). */
    if (sym->s_name->flags & (STR_IMPORT | STR_EXPORT))
	major_sect1 = sym->s_inpndx;
    else
	major_sect1 = sym->s_csect->c_major_sect - 1;

    if (sym2->s_name->flags & (STR_IMPORT | STR_EXPORT))
	major_sect2 = sym2->s_inpndx;
    else
	major_sect2 = sym2->s_csect->c_major_sect - 1;

    return (major_sect1 == major_sect2);
} /* cancel2 */
/************************************************************************
 * Name: cancel
 *									*
 * Purpose: Set the RLD_CANCELED bit in RLDs that should not be written
 *		into the .loader section.  This routine can set the
 *		RLD_CANCELED bit in more than 1 RLD.  The RLD passed as a
 *		parameter should not already have the RLD_CANCELED bit set.
 *		Furthermore, sym must be non-null, so an RLD for a reference
 *		to an undefined symbol is always written to the .loader section.
 *									*
 * Returns:	1, if RLD is canceled (shouldn't go in .loader section)
 *		0, otherwise
 *
 ************************************************************************/
static int
cancel(RLD *rld,
       SYMBOL *sym)			/* Symbol referenced by RLD */
{
    RLD	*rld2;
    int rc;

    /* Prebound items are not relocated by system loader. */
    if (sym->s_flags & S_XMC_XO) {
	rld->r_flags |= RLD_CANCELED;
	return 1;
    }

    switch (rld->r_reltype) {
	/*	A(sym-*)
		if relative branch & branch target are in same module section,
		they will move as a unit and no relocation is necessary. */
      case R_REL:
      case R_BR:
      case R_RBR:
	rc = (sym->s_csect->c_major_sect == rld->r_csect->c_major_sect);
	break;

	/* A(sym-$TOC): TOC is always in data section, not code section */
      case R_TOC:
      case R_TRL:
      case R_TRLA:
	rc = (sym->s_csect->c_major_sect == MS_DATA);
	break;

	/* RLDs for absolute branches--these always need relocation if their
	   targets are not fixed (with storage-mapping class XO). */
      case R_BA:
      case R_RBA:
	return 0;

	/* RLDs for address constants--these always need relocation */
      case R_GL:
      case R_TCL:
      case R_RL:
      case R_RLA:
	return 0;

	/* RLDs for address constants--these always need relocation, except
	 for R_POS/R_NEG pairs of RLDs. */
      case R_POS:
	for (rld2 = rld->r_next;
	     rld2 && rld->r_addr == rld2->r_addr;
	     rld2 = rld2->r_next) {
	    if (rld2->r_reltype == R_NEG
		&& rld->r_length == rld2->r_length
		&& cancel2(sym, rld2)) {
		/* We cancel both RLDs */
		rld->r_flags |= RLD_CANCELED;
		rld2->r_flags |= RLD_CANCELED;
		return 1;
	    }
	}
	return 0;
	break;

      case R_NEG:
	for (rld2 = rld->r_next;
	     rld2 && rld->r_addr == rld2->r_addr;
	     rld2 = rld2->r_next) {
	    if (rld2->r_reltype == R_POS
		&& rld->r_length == rld2->r_length
		&& cancel2(sym, rld2)) {
		/* We cancel both RLDs */
		rld->r_flags |= RLD_CANCELED;
		rld2->r_flags |= RLD_CANCELED;
		return 1;
	    }
	}
	return 0;
	break;

	/* RLD types that should never be in .loader section */
      case R_REF:

	/* Other RLD types.  An error message should have already been printed
	   about these when writing the code to the output file.  We just
	   delete these from the .loader section. */
      default:
	rld->r_flags |= RLD_CANCELED;
	return 1;
	break;
    }

    if (rc == 1) {
	rld->r_flags |= RLD_CANCELED;
	return 1;
    }

    return 0;
} /* cancel */
/************************************************************************
 * Name: compare_sym_names() - comparison routine for qsort() used to sort
 *		symbol names in ro_text().
 *
 ************************************************************************/
static int
compare_sym_names(const void *a,
		  const void *b)
{
    int n;
    SYMBOL *sym1 = *(SYMBOL **)a;
    SYMBOL *sym2 = *(SYMBOL **)b;

    n = strcoll(sym1->s_name->name, sym2->s_name->name);
    if (n == 0)
	n = sym1->s_number - sym2->s_number;
    return n;
}
/************************************************************************
 * Name: ro_text			Processor for the R/O->W option
 *									*
 * Purpose: Processor for the R/O->W option.  This routine prints	*
 *	an error message for relocatable values in the .text section
 *									*
 * Returns:
 *	RC_OK: No load-time relocation entries found for .text section.
 *	RC_ERROR: Load-time relocation entries found for .text section.
 *									*
 * Side Effects:							*
 *	Causes messages to be displayed or placed on the LOAD MAP file	*
 *									*
 ************************************************************************/
static RETCODE
ro_text(void)
{
    static char	id[] = "ro_text";
    int		i, j, n, n1, n2, bad_syms;
    char	*c1, *c2;
    CSECT	*cs;
    int		header_printed, csect_printed;
    RLD		*rld;
    SYMBOL	*sym, **symbols;
    char	*head1, *head2, *head3, *head4;
#define CSECT_NAME_LEN (MINIDUMP_NAME_LEN+3)

    if (rld_counts[0] == 0) {
	say(SAY_NORMAL,
	    NLSMSG(DATAS_NORDONLY,
   "%s: No .loader section relocation entries exist for the .text section."),
	    Command_name);
	return RC_OK;
    }

    if (Switches.dbg_opt11) {
	head1 = " Sym# ";
	head2 = "------";
	head3 = "      ";
	head4 = " Rld# ";
	n1 = 6;
    }
    else {
	head1 = head2 = head3 = head4 = "";
	n1 = 0;
    }
    if (!Switches.verbose)
	symbols = emalloc(rld_counts[0] * sizeof(symbols[0]), id);

    header_printed = 0;
    for (i = first_text_index; i < first_data_index; ++i) {
	cs = Queue[i];
	csect_printed = 0;
	bad_syms = 0;
	for (rld = cs->c_first_rld; rld; rld = rld->r_next) {
	    if (rld->r_flags & RLD_CANCELED)
		continue;

	    if (header_printed == 0) {
		say(SAY_NORMAL,
		    NLSMSG(DATA_RDONLYREF,
   "%s: 0711-310 ERROR: Relocation entries from the .text section have been\n"
   "\twritten to the .loader section. The following csects are in error:"),
		    Main_command_name);
		if (Switches.verbose)
		    say(SAY_NORMAL, NLSMSG(ROTEXT_HEAD1, "\
 CSECT or (Symbol in CSECT)%s   Inpndx  Address  TY CL Source-File(Object-File)\n\
                .loader section RLDs:%s Address  TY CL Rld-type%s Inpndx  Name\n\
 ----------------------------%s-------- -------- -- -- ------------------------%s"),
			head1, head4, head1, head2, head2);
		else
		    say(SAY_NORMAL, NLSMSG(ROTEXT_HEAD2, "\
 CSECT or (Symbol in CSECT)%s   Inpndx  Address  TY CL Source-File(Object-File)\n\
%s Symbols referenced with .loader section RLDs: TY CL %sInpndx  Name\n\
 ----------------------------%s----------------- -- -- ------------------------"),
			head1, head3, head1, head2);
		header_printed = 1;
	    }
	    if (csect_printed == 0) {
		/* For an unnamed csect, find first non-null label name
		   in csect, if any. */
		sym = &cs->c_symbol;
		while (sym && sym->s_name == &NULL_STR)
		    sym = sym->s_next_in_csect;
		if (sym == NULL)
		    sym = &cs->c_symbol;
		if (sym != &cs->c_symbol) {
		    bind_err(SAY_STDERR_ONLY, RC_WARNING,
			     NLSMSG(ROTEXT_WARN2,
    "%1$s: 0711-303 ERROR: Object %2$s, csect with label (%3$s)\n"
    "\tThe csect is part of the .text section, and relocation entries\n"
    "\tfrom the csect have been written to the .loader section."),
			     Main_command_name,
			     get_object_file_name(cs->c_srcfile->sf_object),
			     sym->s_name->name);
		    say(SAY_NO_NLS | SAY_NO_NL, " (%s)", sym->s_name->name);
		    n2 = 2;
		}
		else {
		    if (sym->s_flags & S_HIDEXT) {
			c1 = "<";
			c2 = ">";
			n2 = 2;
		    }
		    else {
			c1 = "";
			c2 = "";
			n2 = 0;
		    }
		    bind_err(SAY_STDERR_ONLY, RC_WARNING,
			     NLSMSG(ROTEXT_WARN1,
    "%1$s: 0711-302 ERROR: Object %2$s, csect %3$s%4$s%5$s\n"
    "\tThe csect is part of the .text section, and relocation entries\n"
    "\tfrom the csect have been written to the .loader section."),
			     Main_command_name,
			     get_object_file_name(cs->c_srcfile->sf_object),
			     c1, sym->s_name->name, c2);
		    say(SAY_NO_NLS | SAY_NO_NL, " %s%s%s",
			c1, sym->s_name->name, c2);
		}
		if (n2 + sym->s_name->len < CSECT_NAME_LEN)
		    n2 = CSECT_NAME_LEN - 1 - (sym->s_name->len + n2);
		else
		    n2 = 0;
		minidump_symbol(&cs->c_symbol, -n2,
				MINIDUMP_INPNDX | MINIDUMP_SYMNUM_DBOPT11
				| MINIDUMP_ADDRESS | MINIDUMP_TYPE
				| MINIDUMP_SMCLASS | MINIDUMP_SOURCE_INFO,
				NULL);
		csect_printed = 1;
	    }
	    sym = rld->r_sym;
	    if (Switches.verbose) {
		say(SAY_NO_NLS | SAY_NO_NL, "%*s%08x %-2s %-2s %-8s ",
		    1 + CSECT_NAME_LEN + 9 + n1, "",
		    rld->r_addr,
		    get_smtype(sym->s_smtype), get_smclass(sym->s_smclass),
		    get_reltype_name(rld->r_reltype));
		if (Switches.dbg_opt11)
		    say(SAY_NO_NLS | SAY_NO_NL, "%-6s", show_sym(sym, NULL));

		n = 5 - show_inpndx(sym, "[%s]");
		say(SAY_NO_NLS,
		    (sym->s_flags & S_HIDEXT) ? "%*s<%s>" : "%*s%s",
		    n+1, " ", sym->s_name->name);
	    }
	    else {
		if (!(sym->s_flags & S_NUMBER_USURPED)) {
		    symbols[bad_syms++] = sym;
		    sym->s_flags |= S_NUMBER_USURPED;
		}
		continue;
	    }
	}
	if (!Switches.verbose) {
	    qsort(symbols, bad_syms, sizeof(symbols[0]), compare_sym_names);
	    for (j = 0; j < bad_syms; j++) {
		sym = symbols[j];
		sym->s_flags &= ~S_NUMBER_USURPED;
		say(SAY_NO_NLS | SAY_NO_NL, "%*s %-2s %-2s ",
		    CSECT_NAME_LEN + 18 + n1, "",
		    get_smtype(sym->s_smtype),
		    get_smclass(sym->s_smclass));
		if (Switches.dbg_opt11)
		    say(SAY_NO_NLS | SAY_NO_NL, "%-6s", show_sym(sym, NULL));

		n = 5 - show_inpndx(sym, "[%s]");
		say(SAY_NO_NLS,
		    (sym->s_flags & S_HIDEXT) ? "%*s<%s>" : "%*s%s",
		    n+1, " ", sym->s_name->name);
	    }
	}
    }

    if (!Switches.verbose)
	efree(symbols);

    if (Switches.quiet)
	if (Switches.loadmap)
	    bind_err(SAY_NOLDMAP, RC_ERROR,
		     NLSMSG(CMPCT_LOADMAP1,
	"%1$s: 0711-344 See the loadmap file %2$s for more information."),
		     Main_command_name, Bind_state.loadmap_fn);
	else
	    bind_err(SAY_NOLDMAP, RC_ERROR,
		     NLSMSG(CMPCT_LOADMAP2,
"%1$s: 0711-345 Use the -bloadmap or -bnoquiet option to obtain more information."),
		     Main_command_name, Bind_state.loadmap_fn);
    return RC_ERROR;
} /* ro_text */
/****************************************************************************
 *	mk_rbd_dbg - Output necessary information for rebind and debug
 *
 *		This routine is responsible for creating the following sections
 *	of the output file:
 *		1) Type check section		Necessary for rebind
 *		2) Debug section		Necessary for debug
 *		3) Exception table		Necessary for debug
 *		4) Info section			Necessary for fdpr
 *		5) Relocation information	Necessary for rebind
 *		6) Line number table		Necessary for debug
 *		7) Symbol table			Necessary for rebind and debug
 *		8) String table			Necessary for rebind and debug
 *
 ***************************************************************************/
static X_OFF_T
mk_rbd_dbg(X_OFF_T output_offset)
{
    OBJECT	*obj;
    long	num_syms;	/* Number of symbol table entries */
    long	lnno_index;	/* For # of entries in line number section */

    /*
      Determine the maximum size of the output symbol table.  For
      ordinary object files, we add up the sizes of the symbol tables,
      if any symbol came from the object.

      Most imported symbols do not need be counted, because they are
      only kept if referenced, usually by an XTY_ER symbol in an
      object file, which has already been counted.

      Nevertheless, if an imported symbol is only saved by being
      "keep" or "exported," it must be counted separately.  (Actually,
      we probably don't need these symbols in the symbol table at all,
      even there are no other references.)

      We use imp_symbols as an upper bound on the number of symbol
      table entries needed on account of imported symbols.  We could
      probably replace this with KEEP_count (from resolve.c) plus
      Bind_state.num_exports, but it's probably not worth the
      trouble.

      Exported symbols are already counted if they are defined in the
      output object file.  Passed-through exports are included in the
      count of imports.
    */

    DEBUG_MSG(OUTPUT_DEBUG,
	      (SAY_NO_NLS,
       "imp_symbols=%d, num_exports=%d, imp_exp_symbols=%d, XO_imports=%d",
	       imp_symbols, Bind_state.num_exports,
	       imp_exp_symbols, XO_imports));

    num_syms = 2		/* For possible generated C_FILE entries. */
	+ 2 * (imp_symbols + Bind_state.generated_symbols)
	    + Bind_state.num_glue_symbols;

    /* Look at each used object to determine symbol table size */
    for (obj = first_object(); obj; obj = obj->o_next) {
	if (obj->o_type == O_T_OBJECT && (obj->oi_flags & OBJECT_USED))
	    num_syms += obj->oi_num_symbols;
    }
    output_offset = ROUND(output_offset, 2); /* Align next section */

    output_offset = generate_symbol_table(num_syms, output_offset);
    /* NOTE:  If necessary, generate_symbol_table will write the typchk and
       debug sections to the output file, beginning at "output_offset"
       and writing header information in "Scnhdr[next_scn]" and
       "Scnhdr[next_scn+1]", respectively.  */

    Bind_state.out_writing = 1;

    output_offset = write_exception_section(output_offset);
    output_offset = write_info_section(output_offset);
    output_offset = write_rld_info(output_offset);
    output_offset = write_line_numbers(output_offset, &lnno_index);

    /* Update line-number and rld counts--add overflow section if necessary */
    if (num_text_rlds < 65535 && lnno_index < 65535) {
	/* Just update the text header with the counts */
	Scnhdr[temp_aout_hdr.o_sntext - 1].s_nreloc = num_text_rlds;
	Scnhdr[temp_aout_hdr.o_sntext - 1].s_nlnno = lnno_index;
    }
    else
	add_overflow_section(temp_aout_hdr.o_sntext-1,
			     num_text_rlds,
			     lnno_index);

    if (num_data_rlds < 65535)
	Scnhdr[temp_aout_hdr.o_sndata - 1].s_nreloc = num_data_rlds;
    else
	add_overflow_section(temp_aout_hdr.o_sndata - 1, num_data_rlds, 0);
    Bind_state.out_writing = 0;

    fixup_symbol_table();

    /* Next write out the symbol table and and string table. */
    Bind_state.out_writing = 1;
    output_offset = write_symbol_table(output_offset);
    Bind_state.out_writing = 0;

    return output_offset;
} /* mk_rbd_dbg */
/************************************************************************
 * Name: add_overflow_section
 *									*
 * Purpose: Add an overflow header
 *		(which will be for a .text or .data section)
 *									*
 * Returns: Nothing
 *
 ************************************************************************/
static void
add_overflow_section(uint16 sect_index,	/* Section index of primary header */
		     long num_rlds,
		     long num_lnnos)
{
    /* setup .text header for use with an ovrflo sec*/
    Scnhdr[sect_index].s_nreloc = 65535;
    Scnhdr[sect_index].s_nlnno = 65535;

    /* setup ovrflo section */
    /* Copy section name, padding with nulls if necessary. */
    (void) strncpy(Scnhdr[next_scn].s_name, _OVRFLO, sizeof(Scnhdr->s_name));
    Scnhdr[next_scn].s_paddr = num_rlds;
    Scnhdr[next_scn].s_vaddr = num_lnnos;
    Scnhdr[next_scn].s_size = 0;
    Scnhdr[next_scn].s_scnptr = 0;	/* unused */
    Scnhdr[next_scn].s_relptr = Scnhdr[sect_index].s_relptr;
    Scnhdr[next_scn].s_lnnoptr = Scnhdr[sect_index].s_lnnoptr;
    Scnhdr[next_scn].s_nreloc = sect_index + 1; /* 1-based */
    Scnhdr[next_scn].s_nlnno = sect_index + 1; /* 1-based */
    Scnhdr[next_scn].s_flags = STYP_OVRFLO;
    ++next_scn;
} /* add_overflow_section */
/************************************************************************
 * Name: write_rld_info
 *									*
 * Purpose: Write the RLDs for the text and data sections
 *									*
 * Returns: Offset to the next available byte in the output file.
 *
 ************************************************************************/
static X_OFF_T
write_rld_info(X_OFF_T output_offset)
{
    caddr_t	mem_ptr;
#ifdef CENTERLINE
    int		rld_counts[2];
#else
    int		rld_counts[2] = {0,0};
#endif
    int		i, j, num_rlds;
    int		limit, limits[2];
    RELOC	*cur_reloc;
    CSECT	*cs;
    RLD		*r, *r2;
    SYMBOL	*sym2;

#ifdef CENTERLINE
    rld_counts[0] = rld_counts[1] = 0;
#endif

    output_offset = ROUND(output_offset, 2); /* Align relocation entries. */

    /* Point to where relocation data should be written */
    mem_ptr = Shmaddr + output_offset;

    /* Pass through the RLDS and write the relocation tables for the
     * .text and .data sections.
     */
    limits[0] = first_data_index - 1;
    limits[1] = last_data_index;
    i = first_text_index;

    for (j = 0; j < 2; ++j) {
	limit = limits[j];
	num_rlds = 0;
	for ( ; i <= limit; ++i) {
	    cs = Queue[i];
	    for (r = cs->c_first_rld; r; r = r->r_next) {
		/* Copy information to reloc structure */
		cur_reloc = (RELOC *)mem_ptr;
		cur_reloc->r_vaddr = r->r_addr + cs->c_new_addr - cs->c_addr;
		if (r->r_flags & RLD_TOCDATA_FIXUP) {
		    /* If we needed fixup code, the rld points to a
		       generated TOC symbol that points to the
		       imported or undefined symbol.  We want the RLD
		       in the output file to refer to the imported or
		       undefined symbol.

		       We just duplicate the STR_NO_DEF code below,
		       but we must not modify the variable 'r'. */
		    r2 = r->r_sym->s_csect->c_first_rld;
#ifdef DEBUG
		    if (!(r2->r_flags & RLD_RESOLVE_BY_NAME))
			internal_error();
#endif
		    if (r2->r_sym->s_name->flags & STR_NO_DEF) {
			/* The output symbol-table index should have been
			   saved in the first external reference */
			sym2 = r2->r_sym->s_name->refs;
#ifdef DEBUG
			if (!(r2->r_sym->s_name->flags & STR_ER_OUTPUT)
			    || sym2 == NULL
			    || !(sym2->s_flags & S_INPNDX_MOD))
			    internal_error();
#endif
			cur_reloc->r_symndx = symtab_index[sym2->s_inpndx+1];
		    }
		    else {
			sym2 = r2->r_sym->s_name->first_ext_sym;
#ifdef DEBUG
			if (!(r2->r_sym->s_name->flags & STR_ER_OUTPUT)
			    || sym2 == NULL)
			    internal_error();
#endif
			if (sym2->s_flags & S_INPNDX_MOD)
			    cur_reloc->r_symndx
				= symtab_index[sym2->s_inpndx+1];
			else if (imported_symbol(sym2))
			    cur_reloc->r_symndx = sym2->s_inpndx;
			else
			    internal_error();
		    }
		    cur_reloc->r_rsize = (r->r_length - 1)
			| (r->r_flags & RLD_SIGNED) | RLD_WAS_FIXED_UP;
		}
		else {
		    if (r->r_flags & RLD_RESOLVE_BY_NAME) {
			if (r->r_sym->s_name->flags & STR_NO_DEF) {
			    /* The output symbol table index should have been
			       saved in the first external reference */
			    sym2 = r->r_sym->s_name->refs;
#ifdef DEBUG
			    if (!(r->r_sym->s_name->flags & STR_ER_OUTPUT)
				|| sym2 == NULL
				|| !(sym2->s_flags & S_INPNDX_MOD))
				internal_error();
#endif
			    cur_reloc->r_symndx
				= symtab_index[sym2->s_inpndx+1];
			}
			else {
			    sym2 = r->r_sym->s_name->first_ext_sym;
			    if (sym2->s_flags & S_INPNDX_MOD)
				cur_reloc->r_symndx
				    = symtab_index[sym2->s_inpndx+1];
			    else if (imported_symbol(sym2))
				cur_reloc->r_symndx = sym2->s_inpndx;
			    else
				internal_error();
			}
		    }
		    else {
			sym2 = r->r_sym;
			if (sym2->s_flags & S_RESOLVED_OK)
			    sym2 = sym2->s_resolved;
			cur_reloc->r_symndx = symtab_index[sym2->s_inpndx+1];
		    }

		    /* Note: Shifting the RLD_FIXUP_USED bit yields
		       the RLD_WAS_FIXED_UP bit. */
		    cur_reloc->r_rsize = (r->r_length - 1)
			| (r->r_flags & RLD_SIGNED)
			    | ((r->r_flags & RLD_FIXUP_USED) << 1);
		}
		cur_reloc->r_rtype = r->r_reltype;

		/* Count for .text and .data */
		++num_rlds;
		mem_ptr += RELSZ;
	    }
	}
	rld_counts[j] = num_rlds;
    }

    /* Update .text and .data section headers.  If an overflow section
       is needed, it will be generated later. */
    num_text_rlds = rld_counts[0];
    if (num_text_rlds) {
	Scnhdr[temp_aout_hdr.o_sntext - 1].s_relptr = output_offset;
	output_offset += num_text_rlds * RELSZ;
    }

    num_data_rlds = rld_counts[1];
    if (num_data_rlds) {
	Scnhdr[temp_aout_hdr.o_sndata - 1].s_relptr = output_offset;
	output_offset += num_data_rlds * RELSZ;
    }
    return output_offset;
} /* write_rld_info */
