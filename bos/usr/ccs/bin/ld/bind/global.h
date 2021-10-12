/* @(#)16	1.20  src/bos/usr/ccs/bin/ld/bind/global.h, cmdld, bos41B, 9504A 12/7/94 15:44:55 */
#ifndef Binder_GLOBAL
#define Binder_GLOBAL
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
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

#include <sys/types.h>
#include <sys/ltypes.h>

#include "typedefs.h"

/* Default library path--this value used only if libpath is not specified with
   "libpath" command and LIBPATH environment variable is not set. */
#ifndef DEFLIBPATH
#define	DEFLIBPATH	"/usr/lib:/lib"
#endif

#ifndef DEFAULT_ENTRYPOINT
#define DEFAULT_ENTRYPOINT	"__start"
#endif

#define TOC_NAME		"TOC"

/* Miscellaneous defines */
#define	MAXTOKENS	8	/* maximum arguments for any binder command */
#define	MAXARGS		40	/* maximum option arguments */

#define ROUND(X,Y)	(((X)+((Y)-1))&~((Y)-1))
#define	max(a,b)	(((a) > (b)) ? (a) : (b))
#define	min(a,b)	(((a) < (b)) ? (a) : (b))

#define atoul(x)	strtoul(x,NULL,10)

/* command return severity codes */
#define	RC_ABORT	-2	/* return code from abort command */
#define	RC_QUIT		-1	/* return code from quit command */
#define	RC_OK		0	/* no error */
#define	RC_WARNING	4	/* warning message generated */
#define	RC_ERROR	8	/* error message generated */
#define	RC_SEVERE	12	/* severe error message generated */
#define	RC_PGMERR	16	/* bind program error detected */
/* NOTE:  Most RC_PGMERRs are fatal, and bind exits.  Most RC_SEVEREs are
   not fatal, but a few are. */

/* Return values that should be converted to RC_OK if in interactive mode, and
   to the values above (without "_NI") otherwise. */
#define RC_NI_BASE	32
#define RC_NI_WARNING	(RC_NI_BASE+RC_WARNING)
#define RC_NI_ERROR	(RC_NI_BASE+RC_ERROR)
#define RC_NI_SEVERE	(RC_NI_BASE+RC_SEVERE)

/* Initialization and termination routines */
extern void	init_ifiles(void);
extern void	init_objects(void);
extern void	init_signals(void);
extern void	init_save(void);

extern void	reset_signals(void);
extern void	cleanup(RETCODE);

/* Utility routine */
extern int moretokens(char *[], int);

/* Volatile variables for handling interrupts */
extern volatile int	stop_flag;		/* If ^Z pressed */
extern volatile int	interrupt_flag;		/* If ^C pressed */

/* Allocation routines */
extern void	*emalloc(size_t, char *);
extern void	*erealloc(void *, size_t, char *);

/* Define efree() */
#ifdef DEBUG
extern void	efree(void *);
#elif CENTERLINE == 1 || STATS == 0
#define efree free
#else
/* WARNING:  Portability:  This macro assumes that the length of an allocated
   area is stored in the word preceding the area. */
#include "stats.h"
#define efree(m) free((STAT_free(BYTES_ID,((long *)m)[-1]),m))
#endif

/* Define get_memory() */
#ifdef STATS
#define get_memory get_mem
extern void	*get_mem(size_t, unsigned int, int, char *);
#else
#define get_memory(a,b,c,d)	get_mem(a,b,d)
extern void	*get_mem(size_t, unsigned int, char *);
#endif					/* STATS */

/* Debugging variable and flags */
#ifdef DEBUG
#define DEBUG_LONG	1		/* Even more information */
#define SYMBOLS_DEBUG	2
#define STAB_DEBUG	4
#define ALLOC_DEBUG	8
#define RELOC_DEBUG	16
#define XCOFF_DEBUG	32
#define IFILES_DEBUG	64
#define RESOLVE_DEBUG	128
#define OUTPUT_DEBUG	256
#define ARCHIVE_DEBUG	512
#define SAVE_DEBUG	1024
#define FIXUP_DEBUG	2048
#define DO_GC_DEBUG	0x1000 /* = 4096 */
#define LINENUMS_DEBUG	0x2000 /* = 8192 */
extern int	bind_debug;		/* Flag that can be set to provide for
					   diagnostic output. */
#endif

/* Estimates to make allocations more efficient.  This numbers are not
   always very useful. */
struct size_estimates {
    int num_srcfiles;
    int num_csects;
    int num_symbols;
    int num_sections;
} Size_estimates;

/************************************************************************
 *	bind_state - Contains global processing and control variables
 *		used during the bind process.
 *	Note:	This structure is initialized in "global.c"
 *		Changes here must also be reflected in the initialization.
 ************************************************************************/
#ifdef STATS
#define IFILES_ID	0
#define OBJECTS_ID	1
#define SRCFILES_ID	2
#define CSECTS_ID	3
#define SYMBOLS_ID	4
#define RLDS_ID		5
#define TYPECHKS_ID	6
#define STRS_ID		7
#define CHARS_ID	8
#define STAB_STRS_ID	9
#define STABS_ID	10
#define BYTES_ID	11
#define OBJECT_INFO_ID	12
#define NUM_MEM_TYPES	13

#ifndef MEM_NAMES
extern char *mem_name[];
#else
/* Initialize mem_name array */
char *mem_name[] = {
    "IFILEs", "OBJECTs", "SRCFILES", "CSECTs", "SYMBOLs", "RLDs",
    "TYPECHECKS", "STRs", "TYPECHK/STR chars", "STAB_STRS", "STABS",
    "BYTES", "OBJECT_INFOs"};
#endif /* MEM_NAMES */
#endif /* STATS */

struct bind_state {
#ifdef STATS
    struct {
	unsigned int allocations;	/* Number of calls to alloc routine*/
	unsigned int alloc;		/* Currently allocated structures */
	unsigned int used;		/* Structures in use */
	unsigned int maxused;		/* Max used */
    } memory[NUM_MEM_TYPES];

    /* *File	File related control variables */
    int		open_cur;		/* current # of input files open */
    int		open_max;		/* max # of input files to open */
    int		open_total;		/* Totals calls to openx */
    int		close_total;		/* max # of input files to open */
    int		shmat_cur;		/* current # of shmat-ed files */
    int		shmat_max;		/* max # of shmat-ed files allowed */
    int		shmat_total;		/* total # of shmat-ed files */
    int		shmdt_total;		/* total shmdt calls */
    int		mmap_cur;		/* current # of mem-mapped files */
    int		mmap_max;		/* max # of mem-mapped files */
    int		mmap_total;		/* max # of mem-mapped files */
    int		munmap_total;		/* max # of mem-mapped files */
#endif /* STATS */

    /* Binder flags and state */
    char	flags;
#define	FLAGS_TEXT_PAGE_ADDR	1	/* text_start refers to page origin */
#define	FLAGS_DATA_PAGE_ADDR	2	/* data_start refers to page origin */
#define FLAGS_WANT_LOADER	4	/* Want to create .loader section */
#define	FLAGS_WANT_DEBUG	8	/* Want to create .debug section */
#define FLAGS_UNLINK_OUT	16	/* Unlink output file on error */
#define FLAGS_UNLINK_TMP	32	/* Unlink temp. output file on error */
    char	loadmap_err;		/* 1 if last attempted write to the
					   loadmap file resulted in an error.*/
    short	state;
#define STATE_REBIND_USED	1	/* rebind was used to insert a file */
#define STATE_RESOLVE_NEEDED	2
#define STATE_RESOLVE_CALLED	4
#define STATE_SAVE_CALLED	8	/* SAVE has been called and has
					   made irrevocable changes to
					   data structures. */
#define STATE_SAVE_COMPLETE	16	/* The output file is completely
					   written. */
#define STATE_RBD_DBG_CALLED	32
#define STATE_PROCESSING_IMPEXP	64	/* Imp/exp file being processed. */

    uint16	sections_seen;		/* Logical OR of low-order 16 bits
					   of s_flags field for all sections
					   seen in any input file. */

    /* COMMAND loop execution control variables */
    char	ever_interactive;	/* Binder is or was run interactively.
					   (This flag is never reset.) */
    char	interactive;		/* Binder being run interactively,
					   whether at the current command
					   level or a previous (less
					   recursive) level.  */
    char	stdin_is_tty;		/* Standard input is a tty. */
    char	stdout_is_tty;		/* Standard output is a tty. */
    char	cmd_err_lev;		/* Return code for this command (can
					   also be set by bind_err calls) */
    char	err_exit_lev;		/* Max CMD_RET to terminate bind */
    char	err_show_lev;		/* Level for showing error messages */
    int		retcode;		/* maximum return value encountered */

    char	prompt[33];		/* binder command prompt */
    FILE	*tty_file;		/* FILE for /dev/tty, if open */

    /* entry command */
    STR		*entrypoint;		/* Entry point name--if NULL, there is
					   no entry point. */
    SYMBOL	*entrypoint_sym;	/* Entry point Symbol pointer--
					   should be descriptor */

    /* libpath command */
    char	*libpath;		/* library path to search */
    int 	libpath_len;		/* Len of allocated area for libpath */

    /* save command */
    long	num_data_in_toc_fixups;	/* Number of instances of fixup code
					   needed in output file because of
					   imported symbols that were expected
					   to be in the TOC. */
    long	num_long_strings;	/* Number of saved strings with
					   length > 8 characters. */
    long	num_typechks;		/* Number of typechk strings saved */
    long	num_potential_C_FILE_strings; /* Count of aux C_FILE entries
						 (for proper allocation when
						 writing output file). */
    long	num_glue_symbols;	/* For computing symbol table size. */
    long	tpad_align;		/* .text alignment (pad) value */
    long	dpad_align;		/* .data alignment (pad) value */
    long	lpad_align;		/* .loader alignment (pad) value */
    SYMBOL	*o_toc_sym;		/* SYMBOL for TOC anchor */
    long        o_tocsz;		/* Size of toc (in bytes) */

    char	**ldrinfo;		/* Strings to be placed in loader
					   section string table. */
    char	*order_file;		/* File containing list of names
					   controlling csect reordering. */

    /* Miscellaneous */
    int		generated_symbols;	/* Number of generated symbols */
    int		num_exports;		/* Number of exported symbols */
    int		files_kept;		/* # of files not garbage collected */

    /* File open and name information */
    int		out_Fd;			/* If not 0, must be closed on error */
    char	*out_name;		/* Output file name */
    int		temp_out_Fd;		/* If not 0, must be closed on error */
    char	*out_tmp_name;		/* Name of temporary object file */
    char	out_writing;		/* Output file is being written
					   (using mapped file) */

    char	*loadmap_fn;		/* file name of loadmap */

    /* Machine instructions */
    int		num_nops;		/* Current number of NOPs */
    int		max_nops;		/* Size of nops[] array. */
    long	*nops;			/* Array of NOPs */
    int		num_loadtocs;		/* Current number of LOADTOCs */
    int		max_loadtocs;		/* Size of loadtocs[] array */
    long	*loadtocs;		/* Array of LOADTOCs */

#ifdef DEBUG
    char	init_flag;		/* Field initialized in global.c--used
					   to check for change to structure
					   without change to initial values. */
#endif
};

/************************************************************************
 *	switches - Contains processing control switches and options used
 *		to control the bind process
 *		Most are set either from main arg string or the
 *			"setopt" command.
 *	Note:	This structure is initialized in "global.c"
 *		Changes here must also be reflected in the initialization.
 ************************************************************************/
struct switches {

    /* These switches are appropriate to be modified
       in an import/export file */
    char	quiet;		/* suppress standard output */
    char	loadmap;	/* write standard output to file */
    char	asis;		/* process commands in asis case */
    char	verbose;	/* print tons of information */
    char	errmsg;		/* write error messages to standard output */
    char	skip_if_shared;	/* Skip rest of import file if autoimp is 1. */

    /* These switches affect input processing */
    char	input_opt;	/* How should input files be read?
				   (See bind.h for possible values) */
    char	autoimp;	/* Auto import shared objects */
    char	del_csect;	/* delete entire CSECT if any symbol deleted */

    /* Miscellaneous switches */
    char	keepall;	/* Keep all C_EXT symbols */
    char	hash_chk;	/* check hash codes of external symbols */

    /* The following switches control the "save" command only */
    char	loadabs;	/* Output is being linked at its load address.
				   This means
				   1) Absolute branches with B_RBA RLD types
				   are not converted to relative branches, even
				   if their targets are relative.
				   2) Relative branches with B_RBR RLD types
				   are not converted to absolute branches, even
				   if their targets are absolute. */
    char	member;		/* List objects used to create output */
    char	execute;	/* access mode for save output file  */
    char	strip;		/* strip output file */
    char	local;		/* include local syms in symbol table */
    char	ro_text;	/* Check for relocation in text section? */
    char	bigtoc;		/* Add fixup code for toc overflow? */
    char	reorder;	/* Reorder csects? */
#define REORDER_DFS 2
#define REORDER_BY_ADDRESS 3
#define REORDER_BY_FILE 4
    char	fold;		/* Fold storage mapping classes? */
    char	mapping;
#define MAPPING_DATAINTEXT 1
#define MAPPING_TEXTINDATA 2
#define MAPPING_NORMAL 3
    char	stabcmpct;	/* Level of compression of stab section */
#define STABCMPCT_NONE 0
#define STABCMPCT_NODUPS 1
#define STABCMPCT_RENUM 2

#if 0
/* Define these for future compaction possiblities.  Do not define them
   otherwise, because some is ifdef-ed based on STABCMPCT_NODUPSYMS */
#define STABCMPCT_NODUPSYMS 3
#define STABCMPCT_FULLRENUM 4
#define STABCMPCT_FULLNODUPSYMS 5
#endif

    /* Targets */
    char	target;
    /* More targets than the following may be needed. */
#define POWER_TARGET 1
#define POWERPC_TARGET 2
#define S601_TARGET 3

    /* The following switches are for debugging purposes */
    char	dbg_opt0;	/* Show counts of symbols read from inputs */
    char	dbg_opt1;	/* Print csect ordering */
    char	dbg_opt2;	/* List labels as well as csects with opt1 */
    char	dbg_opt3;	/* List symbol address, not numbers in dump()*/

    char	dbg_opt4;	/* Check coalesced TOC entries for same value*/
    char	dbg_opt5;	/* Not used. */
    char	dbg_opt6;	/* Show lines read from import/export file */
    char	dbg_opt7;	/* Relocation diagnostic information */

    char	dbg_opt8;	/* Exhaustive resolution output */
    char	dbg_opt9;	/* Write 0s in a.out timestamp field */
    char	dbg_opt10;	/* Show TOC symbol resolution */
    char	dbg_opt11;	/* Show Symbol # when printing messages */

#ifdef DEBUG
    STR		*init_flag;		/* Field initialized in global.c--used
					   to check for change to structure
					   without change to initial values. */
#endif
};

extern struct bind_state	Bind_state;
extern struct switches		Switches;
extern char			*Command_name; /* Name of current command */
extern char			*Main_command_name; /* "ld" or "bind" */
extern int			Pagesize; /* Pagesize where bind is running. */
#endif /* Binder_GLOBAL */
