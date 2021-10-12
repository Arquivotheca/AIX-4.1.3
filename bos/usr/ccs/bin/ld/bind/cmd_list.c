#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)87	1.18  src/bos/usr/ccs/bin/ld/bind/cmd_list.c, cmdld, bos411, 9428A410j 5/11/94 16:51:31")
#endif

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

#include <stddef.h>

#include "bind.h"
#include "global.h"
#include "commands.h"
#include "error.h"

#if DEBUG || XLC_TYPCHK_BUG
#include "strs.h"
#endif

/* Redefine commands not yet implemented */
#define discard unsupported
#define expall unsupported
#define lrename unsupported
#define hsh unsupported
#define tags unsupported

#ifndef STATS
#define stats unsupported
#endif

#ifndef DEBUG
#define gc ignored
#endif

/* Declarations for "unsupported" and "ignored" commands. */
extern	bind_command_t unsupported;
extern	bind_command_t ignored;

/* external declarations for command functions */
extern bind_command_t
    abort_bind,				/* Terminate and exit binder */
    addglue,				/* Insert "glue" code */
    addnamedglue,			/* Insert "glue" code for symbols */
    align,				/* Set/display csect alignment(s) */
    bindopt,				/* Set/reset binder options */
    comprld,				/* Combine RLDs at same address */
    discard,				/* Discard a csect */
    dump_cmd,				/* Display info about some symbols */
    entry,				/* Set/display object entry point */
    execute,				/* Read and execute cmds from file */
    export_symbols,			/* Export an external symbol */
    expall,				/* Export all external symbols */
    export_symbols_in_file,		/* Export list of external symbols */
    flist,				/* Read a list of object files */
    gc,					/* Garbage collect unused csects */
    halt,				/* Set/display max. error code */
    help,				/* Display summary of binder cmds */
    hsh,				/* Set/display function hash values */
    import_symbols,			/* Import symbol */
    import_symbols_in_file,		/* Import a list of symbols */
    insert, rebind, library,		/* Read and process an object file */
    keep,				/* Keep a symbol and its descendants */
    keepfile,				/* Command prefix for insert, rebind,
					   or library commands--no gc */
    nokeepfile,				/* GC file after all */
    libpath, nolibpath,			/* Set/display LIBPATH value; Delete */
    noop, tocload,			/* Instructions for NOP, TOC reload */
    lrename,				/* Rename loader section symbols */
    mapgen,				/* Print load map information */
    maxdat,				/* Set/display max data seg size */
    maxstk,				/* Set/display max stack size */
    mismatch,				/* Check for parameter mismatches */
    origin,				/* Set text/data origin addresses */
    noentry,				/* Remove entry point */
    pad,				/* Generate pad sections */
    prompt,				/* Change binder prompt */
    quit,				/* Quit current subcommand level */
    max_retcode,			/* Display max returned error code */
    rename_cmd,				/* Rename a symbol */
    resolve,				/* Resolve symbol references */
    save,				/* Save the output file */
    savename,				/* Name output file, for early check */
    shell,				/* Create a sub-shell */
    stats,				/* Display memory usage statistics */
    stgcls,				/* Set storage class for symbol */
    tags,				/* Display tags info from file */
    unres;				/* Check for unresolved symbols */

/* Here is a command table for all the binder commands.  It is searched
   linearly, so often-used commands should appear first. */
struct command		Commands[] = {
    /* Name,    function, process_deferred_inserts? minargs, maxargs,usage # */
    {"I",	insert,		0,	1, 2,
	 NLSMSG(USAGE_INSERT, "%1$s: 0711-532 USAGE: {I|INSERT} [KEEP] fn")},
    {"LIB",	library,	0,	1, 2},
    {"IMPORTS",	import_symbols_in_file,0,1,1},
    {"EXPORTS",	export_symbols_in_file,0,1, 1},
    {"RENAME",	rename_cmd,	CMD_READ_FILES_FIRST,2, 2},
    {"SETOPT",	bindopt,	0,	0, -1},
    {"ADDGL",	addglue,	CMD_READ_FILES_FIRST,1, -1,
	 NLSMSG(USAGE_GLUE, "%1$s: 0711-372 USAGE: ADDGL fn")},
    {"FILELIST",flist,		0,	1, 2},
    {"HALT",	halt,		0,	0, 1},
    {"MISMATCH",mismatch,	CMD_READ_FILES_FIRST,0, 1},
    {"RC",	max_retcode,	0,	0, 0},
    {"RESOLVE",	resolve,	CMD_READ_FILES_FIRST,0, 0},
    {"SAVE",	save,		CMD_READ_FILES_FIRST,2, 2},
    {"SAVENAME",savename,	0,	0, 1},
    {"COMPRLD",	comprld,	CMD_READ_FILES_FIRST,0, 0},
    {"ER",	unres,		CMD_READ_FILES_FIRST,0, 1},
    {"GEN",	mapgen,		CMD_READ_FILES_FIRST,2, 2},
    {"LIBPATH",	libpath,	0,	0, 1},
    {"MAXDATA",	maxdat,		0,	0, 1},
    {"MAXSTACK",maxstk,		0,	0, 1},
    {"ORIGIN",	origin,		0,	0, 3},
    {"PAD",	pad,		0,	0, 3},
    {"ENTRY",	entry,		0,	0, 1},
    {"EXEC",	execute,	0,	1, 1},
    {"KEEP",	keep,		0,	1, -1},
    {"NOENTRY",	noentry,	0,	0, 0},
    {"NOP",	noop,		0,	0, -1},
    {"REBIND",	rebind,		0,	1, 2},

    /* These commands are not generated by 'ld' */
    {"INSERT",	insert,		0,	1, 2,
	 NLSMSG(USAGE_INSERT, "%1$s: 0711-532 USAGE: {I|INSERT} [KEEP] fn")},
    {"LIBRARY",	library,	0,	1, 2},
    {"ADDNAMEDGLUE",addnamedglue,CMD_READ_FILES_FIRST,2, -1,
	 NLSMSG(USAGE_ADDNAMEDGLUE,
		"%1$s: 0711-375 USAGE: ADDNAMEDGLUE fn pattern ...")},
    {"ORG",	origin,		0,	0, 3},
    {"KEEPFILE",keepfile,	0,	1, 1},
    {"NOKEEPFILE",nokeepfile,	0,	1, 1},
    {"Q",	quit,		0,	0, 0},
    {"QUIT",	quit,		0,	0, 0},
    {"EXIT",	quit,		0,	0, 0},
    {"STOP",	quit,		0,	0, 0},
    {"ABORT",	abort_bind,	0,	0, 0},
    {"EXPORT",	export_symbols,	0,	1, 2},
    {"IMPORT",	import_symbols,	CMD_READ_FILES_FIRST,1, 2},
    {"TOCLOAD",	tocload,	0,	0, -1},
    {"NOLIBPATH",nolibpath,	0,	0, 0},

    /* These commands debugging or diagnostic commands and
       are not generated by 'ld' */
    {"DUMP",	dump_cmd,	0,	1, 3},
    {"STATS",	stats,		0,	0, 0},
    {"STG",	stgcls,		0,	0, 2},
    {"PROMPT",	prompt,		0,	1, 1},
    {"SH",	shell,		0,	0, 0},
    {"ALIGN",	align,		0,	1, 2, NLSMSG(USAGE_ALIGN,
		 "%1$s: 0711-610 USAGE: ALIGN [pattern] {0-31}")},

    /* These are ignored commands. */
    {"GC",	gc,		CMD_READ_FILES_FIRST,0, 0},
    {"HELP",	help,		0,	0, 1},
    {"H",	help,		0,	0, 1},
    {"?",	help,		0,	0, 1},

    /* These are the unsupported commands.  Minargs and maxargs are set so
       messages about the wrong number of arguments can never be seen. */
    {"DISCARD",	discard,	0,	0, -1},
    {"EXCLUDE",	discard,	0,	0, -1},
    {"EXPALL",	expall,		0,	0, -1},
    {"EXPNAME",	lrename,	0,	0, -1},
    {"HASH",	hsh,		0,	0, -1},
    {"IMPNAME",	lrename,	0,	0, -1},
    {"TAGS",	tags,		0,	0, -1},
    {NULL}
};

/* Arguments for SETOPT input:<input_method> */
opts input_opts[] = {{"shmat", I_ACCESS_SHMAT},
			 {"mmap", I_ACCESS_MMAP},
#ifdef READ_FILE
			 {"read", I_ACCESS_READ},
			 {"shmat/read", I_ACCESS_SHMAT_READ},
			 {"mmap/read", I_ACCESS_MMAP_READ},
#endif

#ifdef I_ACCESS_MALLOC
			 {"malloc", I_ACCESS_MALLOC},
#endif
			 {NULL, 0}};

#ifdef _CPUTYPE_FEATURE
/* Arguments for SETOPT target:<target> */
opts target_opts[] = {{"power", POWER_TARGET},
			  {"powerpc", POWERPC_TARGET},
			  {"s601", S601_TARGET},
			  {NULL, 0}};
#endif

/* Arguments for SETOPT reorder:<method> */
opts reorder_opts[] = {{"dfs", REORDER_DFS},
			   {"by_address", REORDER_BY_ADDRESS},
			   {"file", REORDER_BY_FILE},
			   {NULL, 0}};

/* Arguments for SETOPT mapping:<method> */
opts mapping_opts[] = {{"textindata", MAPPING_TEXTINDATA},
			   {"dataintext", MAPPING_DATAINTEXT},
			   {"normal", MAPPING_NORMAL},
			   {NULL, 0}};

/* Arguments for SETOPT stabcmpct:<level> */
opts stabcmpct_opts[] = {{"0", STABCMPCT_NONE},
			     {"1", STABCMPCT_NODUPS},
			     {"2", STABCMPCT_RENUM},
#ifdef STABCMPCT_NODUPSYMS
			     {"3", STABCMPCT_NODUPSYMS},
			     {"4", STABCMPCT_FULLRENUM},
			     {"5", STABCMPCT_FULLNODUPSYMS}
#endif
			 {NULL, 0}};

/* Options (arguments to SETOPT command) */
option_t option[] = {
  {"LOADMAP",(char *)&Bind_state.loadmap_fn,LOADMAP_OPT,OPT_OK_IN_IMPFILE,NULL},
  {"ASIS",	&Switches.asis,		NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"AUTOIMP",	&Switches.autoimp,	NOARG_OPT,	0,	NULL},
#ifdef DEBUG
  {"BIND_DEBUG",(char *)&bind_debug,	INT_OPT,	OPT_OK_IN_IMPFILE,NULL},
#endif
  {"DELCSECT",	&Switches.del_csect,	NOARG_OPT,	0,	NULL},
  {"R/O->W",	&Switches.ro_text,	NOARG_OPT,	0,	NULL},
  {"ERRMSG",	&Switches.errmsg,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"EXECUT",	&Switches.execute,	NOARG_OPT,	0,	NULL},
  {"FOLD",	&Switches.fold,		NOARG_OPT,	0,	NULL},
  {"KEEPALL",  	&Switches.keepall,	NOARG_OPT,	0,	NULL},
  {"MEMBER",	&Switches.member,	NOARG_OPT,	0,	NULL},
  {"QUIET",	&Switches.quiet,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"STRIP",	&Switches.strip,	NOARG_OPT,	0,	NULL},
  {"TYPCHK",	&Switches.hash_chk,	NOARG_OPT,	0,	NULL},
  {"VERBOSE",	&Switches.verbose,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"BIGTOC",	&Switches.bigtoc,	NOARG_OPT,	0,	NULL},
  {"INPUT",	&Switches.input_opt,	LIST_OPT,	0,	input_opts},
  {"MAPPING",	&Switches.mapping,	LIST_OPT,	0,	mapping_opts},
  {"REORDER",	&Switches.reorder,	LIST_OPT,	0, 	reorder_opts},
  {"STABCMPCT",	&Switches.stabcmpct,	LIST_OPT,	0, 	stabcmpct_opts},
#ifdef _CPUTYPE_FEATURE
  {"TARGET",	&Switches.target,	LIST_OPT,	0,	target_opts},
#endif
  {"LDRINFO",(char *)&Bind_state.ldrinfo,STRINGLIST_OPT,OPT_OK_IN_IMPFILE,NULL},
  {"LOADABS",	&Switches.loadabs,	NOARG_OPT,	0,	NULL},
  {"ORDER_FILE",(char *)&Bind_state.order_file,	STRING_OPT,	0,	NULL},

  {"DBOPT0",	&Switches.dbg_opt0,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"DBOPT1",	&Switches.dbg_opt1,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"DBOPT2",	&Switches.dbg_opt2,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"DBOPT3",	&Switches.dbg_opt3,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},

  {"DBOPT4",	&Switches.dbg_opt4,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"DBOPT5",	&Switches.dbg_opt5,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"DBOPT6",	&Switches.dbg_opt6,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"DBOPT7",	&Switches.dbg_opt7,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},

  {"DBOPT8",	&Switches.dbg_opt8,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"DBOPT9",	&Switches.dbg_opt9,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"DBOPT10",  	&Switches.dbg_opt10,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  {"DBOPT11",  	&Switches.dbg_opt11,	NOARG_OPT,	OPT_OK_IN_IMPFILE,NULL},
  { NULL}
};
