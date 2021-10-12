#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)23	1.40  src/bos/usr/ccs/bin/ld/ld.c, cmdld, bos411, 9428A410j 4/5/94 14:04:31")
#endif

/*
 * COMPONENT_NAME: (CMDLD) ld - xcoff linkage editor
 *
 *   FUNCTIONS:
 *
 *   MAIN ENTRY POINT:
 *		main
 *
 *   STATIC FUNCTIONS:
 *		add_list
 *		allocate
 *		checkliblist
 *		cleanup
 *		convert_arg
 *		error
 *		escape_name
 *		get_files
 *		getfilename
 *		getlibname
 *		new_filearg_info
 *		out_argnames
 *		out_list
 *		output
 *		parse_bflag
 *		reallocate
 *		set_binder_args
 *		start_binder
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <nl_types.h>
#include <locale.h>
#include <glob.h>

#include <sys/access.h>
#include <sys/stat.h>
#include <sys/wait.h>

extern int getpagesize(void);		/* No prototype in standard hdr file */

#ifdef AIX_REL3
extern int optopt;			/* Declared in 4.1 stdio.h, but not
					 rel 3.  AIX_REL3 is only defined
					 when building the bldenv files. */
#endif

#include "ld_msg.h"

/* Use defines so we can override definitions at compile time. */
/* Default error level for halting bind */
#ifndef DEF_HALT
#define	DEF_HALT	4
#endif
/* Default object file name */
#ifndef DEF_OBJ
#define	DEF_OBJ		"a.out"
#endif
/* Default module type */
#ifndef DEF_MODTYPE
#define	DEF_MODTYPE	"1L"
#endif
/* Default glink (glue) file */
#ifndef DEF_GLINK
#define	DEF_GLINK	"/usr/lib/glink.o"
#endif
/* Default library path */
#ifndef DEF_LIBPATH
#define DEF_LIBPATH	"/usr/lib:/lib"
#endif
/* Default binder */
#ifndef DEF_BINDER
#define DEF_BINDER	"/usr/ccs/bin/bind"
#endif

/* Binder setopt choices */
static char *Str_autoimp	= "autoimp";
static char *Str_row		= "r/o->w";
static char *Str_x		= "execut";

static char *Str_noasis		= "noasis";
static char *Str_noautoimp	= "noautoimp";
static char *Str_nomember	= "nomember";
static char *Str_norow		= "nor/o->w";
static char *Str_nox		= "noexecut";
static char *Str_nov		= "noverbose";

static int Pagesize;			/* PAGESIZE on this machine */
static int convert_valid;		/* Set to 0 if a number couldn't
					   be converted. */
/***********************************************************************/
static nl_catd catd;
/*
 * Declare an array to hold the default messages.  The text of the
 * messages is read from a file generated from the message catalog.
 */

static char *default_messages[] = {
    NULL				/* Placeholder--do not delete
					   --do not add a comma.  */
#    include "ld_msg.c"
    };
static long max_message_number		/* Compute number of messages. */
    = (long)(sizeof(default_messages)/sizeof(char *) - 1);

#define NLSMSG(n,s) n
#define def_message(m) default_messages[(m<1||m>max_message_number)?1:m]
/***********************************************************************/

/* valid flags to this program */
#define	PROG_OPTS ":A:b:B:dD:e:f:H:ij:k:Kl:L:mMnNo:QrR:sS:T:u:vV:xY:zZ:"

/* Structure for saving information about filenames */
struct arg_info {
    dev_t	a_dev;
    ino_t	a_ino;
    int		flags;
#define LIBRARY_FLAG	1
#define REINSERT_FLAG	2
#define REBIND_FLAG	4
#define KEEP_FLAG	8
#define KEEPFILE_FLAG	16
#define NOT_FOUND_FLAG	32
#ifdef STAT_DYNAMIC
#define STATIC_FLAG	64
#define SHARED_FLAG	128
#endif
#define DELETED_FLAG	256
};

/* Structure for saving lists of arguments (-L, -Z, etc.) */
struct dlink {	/* double-element linked list */
    char		*value;
    void		*info;
    struct dlink	*next;
};

/* Parameters with special default values.  (The F_DEFAULT flag is set
   in references to these variables in the option table.)  The second element
   is the default value and the first is the current value. */

/* TP_STRING parameters */
static char	*Mod_type[2]	= {DEF_MODTYPE, DEF_MODTYPE},/* -bM -bmodtype*/
		*Binder[2]	= {DEF_BINDER, DEF_BINDER}, /* -bbinder */
		*Glink_file[2]	= {DEF_GLINK, DEF_GLINK}; /* -bglink */
/* TP_DECIMAL parameters */
static int	Stabcmpct[2]	= {-1, -1}, /* -bstabcmpct
					       Stab compaction level */
		Halt_level[2]	= {DEF_HALT, DEF_HALT};	/* -bhalt */

/* TP_STRING parameters with non-NULL initialization */
static char	*Load_map = "";		/* -bloadmap: */

/* TP_STRING parameters initialized (implicitly) to NULL */
static char	*entry,			/* -e <entrypoint name> */
		*Libpath,		/* User-specified libpath (-blibpath)*/
		*Call_file, *SCall_file, /* -bC -bcalls,  -bscalls */
		*Map_file,  *SMap_file,	/* -bR -bmap, -bsmap */
		*Xref_file, *SXref_file, /* -bX -bxref, -bsxref */
		*Dump_args,		/* -bbindopts */
		*Exec1, *Exec2, *Exec3, *Exec4, *Exec5;	/* -bex[1-5] */
static char	*Debug_out_name;	/* -bbindcmds */

/* TP_NUMBER (longs) parameters initialized (implicitly) to 0 */
static ulong	Max_stack, Max_data,	/* -S -bS -bmaxstack, -bD -bmaxdata */
		text_org[2] = {0L,	/* -T, -bpT
					   origin of RO segment */
				   0},	/* if 1, origin is page origin */
		data_org[2] = {0L,	/* -D, -bpD
					   origin of R/W segment */
				   0};	/* if 1, origin is page origin */

/* TP_VALUE parameters initialized (implicitly) to 0 */
static int	noentry,		/* if -b noentry seen */
		nobind_seen,		/* If -b nobind seen */
		Export_all,		/* Export all defined symbols */
		Garb_collect,		/* files not garbage collected */
		Nop_type;		/* Kind of NOPs: */
/* Do not change these NOP_ defines without corresponding changes to the
   option table for options referring to Nop_type */
#define NOP_ORI 0
#define NOP_CROR15 -1
#define NOP_CROR31 1
#define NOP_MANY 2

#define OPCODE_ORI	60000000
#define OPCODE_CROR15	4def7b82
#define OPCODE_CROR31	4ffffb82

/* Define quoted opcodes for printing or comparing */
#define OP_QUOTE(op) #op
#define PRINT_OPCODE(op) OP_QUOTE(op)
#define PRINT_ORI PRINT_OPCODE(OPCODE_ORI)
#define PRINT_CROR15 PRINT_OPCODE(OPCODE_CROR15)
#define PRINT_CROR31 PRINT_OPCODE(OPCODE_CROR31)

/* Define opcodes as hex numbers for comparisons */
#define OP_VALIFY(op) 0x ## op
#define VAL_OPCODE(op) OP_VALIFY(op)
#define VAL_ORI VAL_OPCODE(OPCODE_ORI)
#define VAL_CROR15 VAL_OPCODE(OPCODE_CROR15)
#define VAL_CROR31 VAL_OPCODE(OPCODE_CROR31)

/* TP_VALUE parameters initialized to 1 */
static int	Comprld = 1,		/* -bcomprld */
		Reorder = 1,		/* -breorder */
		Ext_check = 1,		/* -bf -berok -bernotok */
		Quiet = 1,		/* -bquiet */
		Call_binder = 1;	/* whether binder should be called */

/* error recovery flags */
static int	Exit_flag = 0,		/* Exit flag for syntax/file errors */
		Recover_flag = 1,	/* whether to recover after error */
		f_flag_used = 0;	/* Suppress spurious error if -f used*/

static struct dlink
    *Arg_head,				/* list of input object files */
    *Arg_tail,
    *Exp_head,				/* list of export files */
    *Exp_tail,
    *Imp_head,				/* list of import files */
    *Imp_tail,
    *Libprefix_head,			/* list of default library prefixes */
    *Libprefix_tail,
    *Lib_head,				/* list of library directories */
    *Lib_tail,
    *Opt_head,				/* list of binder options */
    *Opt_tail,
    *Rnm_head,				/* list of rename symbols */
    *Rnm_tail,
    *Keep_head,				/* list of symbols to keep */
    *Keep_tail;

static int	Fd = -1;		/* file descriptor for current file */

#define NOP_LIST_INITIAL_SIZE 10
static int	Nop_count;		/* Count of machine instructions for
					   NOPs. */
static int	Nop_list_size;		/* Max size of Nop_list */
static ulong	*Nop_list;		/* Current Nop_list */

static FILE	*Debug_out;		/* to write copy of binder commands */
static FILE	*Dump_fp;		/* For binder arguments */
static FILE	*Out_file = stdout;	/* destination of binder commands */
static int	Debug_out_ok = 1;	/* Write output() line to Debug_out? */
static char	null_byte = '\0';	/* A unique null byte for initializing
					   the info field of list items.  */
/* Forward declarations */
static void *allocate(size_t);
static void *reallocate(void *, size_t);
typedef enum error_flag {FILE_NOT_FOUND_FLAG,
			       TERMINAL_FLAG,
			       SYNTAX_FLAG,
			       IGNORE_FLAG} error_flag_t;
static void error(error_flag_t, char *, int, ...);

static ulong convert_arg(char *, int, char *);
static void cleanup(int);
static void get_files(char *);
static void out_list(char *, struct dlink *, int);
static void out_argnames(char *, char *, char *, char *);
static void output(const char *, ...);
static void parse_bflag(char *);
static void set_binder_args(char *[]);
static void start_binder(char *[]);
static void add_list(struct dlink **, struct dlink **, char *, void *);
static char *getfilename(char *, struct arg_info *);
static char *escape_name(char *);
static char *getlibname(char *);
static struct arg_info *new_filearg_info(void);

/************************************************************************
 * Name: main
 *									*
 * Purpose: Main entry point for ld.
 *									*
 * Does not return.  Calls cleanup() if no errors occur earlier.
 ************************************************************************/

main(int argc,
     char *argv[])
{
    char		*p, *ptr;
    char		*av[4];		/* argv for call to binder */
    int			c;
    int			i;
    int			skip;
    long		val;
    struct dlink	*q;
    struct dlink	*qprev;
    struct dlink	*z;
    struct arg_info	*info, *prev_info;
    struct stat		sbuf;

    char		*outfile = DEF_OBJ; /* output object filename */
    dev_t		outfile_dev;	/* Device of output file */
    ino_t		outfile_ino;	/* Inode of output file */
    int			num_files = 0,	/* input object files */
			num_exports = 0,
			num_imports = 0;
    long		rwalign = 0L;	/* alignment of R/W segment */

    (void) setlocale(LC_ALL, "");
#ifdef AIX_REL3
    catd = 0;				/* Don't use message catalog. */
#else
    catd = catopen("ld.cat", NL_CAT_LOCALE);
#endif
    Pagesize = getpagesize();

    opterr = 0;				/* turn off getopt error messages
					   (we do our own) */

    while (optind < argc) {
	switch(c = getopt(argc, argv, PROG_OPTS)) {
	  case 'b':			/* binder options flags */
	    parse_bflag(optarg);
	    break;

	  case 'T':			/* Origin of .text segment */
	    text_org[0] = convert_arg(optarg, 0, NULL);
	    text_org[1] = 0;	     /* Origin is actual, not page */
	    break;

	  case 'D':			/* Origin of .data segment */
	    data_org[0] = convert_arg(optarg, 0, NULL);
	    data_org[1] = 0;	     /* Origin is actual, not page */
	    break;

	  case 'H':			/* Alignment for file sections */
	    rwalign = convert_arg(optarg, 0, NULL);
	    break;

	  case 'S':			/* Max allowed stack size */
	    Max_stack = convert_arg(optarg, 0, NULL);
	    break;

	  case 'e':			/* entry point label */
	    entry = optarg;
	    noentry = 0;
	    break;

	  case 'f':			/* file with list of input objects */
	    f_flag_used = 1;
	    get_files(optarg);
	    break;

	  case 'K':			/* pad file sections to page boundary*/
	  case 'z':			/* pad file sections to page boundary*/
	    rwalign = Pagesize;
	    break;

	  case 'l':	/* include libkey.a in compilation */
	    info = new_filearg_info();
	    add_list(&Arg_head, &Arg_tail, optarg, info);
	    info->flags |= LIBRARY_FLAG;
	    break;

	  case 'L':	/* search dir for libraries also */
	    add_list(&Lib_head, &Lib_tail, optarg, NULL);
	    break;

	  case 'm':	/* list all files and archive members used */
	  case 'M':
	    add_list(&Opt_head, &Opt_tail, "member", NULL);
	    break;

	  case 'o':			/* output object file name */
	    outfile = optarg;
	    break;

	  case 'r':
	    /* -r is equivalent to -bnogc, -berok, -bnoglink, -bnox  (or no
	       garbage collect, no external check, no glue code, no execute) */
	    Garb_collect = -1;		/* parse_bflag("nogc"); */
	    Ext_check = 0;		/* parse_bflag("erok"); */
	    Glink_file[0] = NULL;	/* parse_bflag("noglink"); */
	    add_list(&Opt_head, &Opt_tail, Str_nox, NULL);
					/* parse_bflag("nox"); */
	    break;

	  case 's':	/* strip symbol table */
	    add_list(&Opt_head, &Opt_tail, "strip", NULL);
	    break;

	  case 'u':	/* enter undefined symbol */
	    add_list(&Keep_head, &Keep_tail, optarg, NULL);
	    break;

	  case 'v':	/* setopt verbose */
	    add_list(&Opt_head, &Opt_tail, "verbose", NULL);
	    break;

	  case 'Z':	/* lib name prefix */
	    add_list(&Libprefix_head, &Libprefix_tail, optarg, NULL);
	    break;

	  case -1:	/* filename argument or end of arguments */
	    if (optind < argc)
		add_list(&Arg_head, &Arg_tail,
			 argv[optind++], new_filearg_info());
	    break;

	  case 'A':	/* ignored (a_misc value) */
	  case 'B':	/* ignored (BSS origin) */
	  case 'j':
	  case 'k':
	  case 'R':	/* ignored */
	  case 'V':	/* ignored */
	  case 'Y':
	    error(IGNORE_FLAG, NULL,
		  NLSMSG(LD_FIGNORED1,
			 "ld: 0706-027 The -%c%s%s flag is ignored."),
		  optopt, " ", optarg);
	    break;

	  case 'd':	/* ignored (defines common) */
	  case 'i':	/* ignored */
	  case 'n':
	  case 'N':	/* ignored (text not read only) */
	  case 'Q':	/* local symbols to symbol table */
	  case 'x':	/* no local symbols */
	    error(IGNORE_FLAG, NULL,
		  NLSMSG(LD_FIGNORED1,
			 "ld: 0706-027 The -%c%s%s flag is ignored."),
		  optopt, "", "");
	    break;

	  case '?':
	    error(SYNTAX_FLAG, NULL,
		  NLSMSG(LD_BADOPT,
			 "ld: 0706-012 The -%c flag is not recognized."),
		  optopt);
	    break;
	  case ':':
	    error(SYNTAX_FLAG, NULL,
		  NLSMSG(LD_ARGREQ,
			 "ld: 0706-011 The -%c flag needs a parameter."),
		  optopt);
	    break;
	  default:
	    error(TERMINAL_FLAG, NULL,
		  NLSMSG(LD_INTERNAL_ERR,
			 "ld: 0706-002 Internal error at line %1$d (v=%2$d).\n"
"\tDepending on where this product was acquired, contact your service\n"
			 "\trepresentative or the approved supplier."),
		  __LINE__, c);
    	}
    }

    /* If no -Z option was specified, make sure we search default library */
    if (Libprefix_head == NULL)
	add_list(&Libprefix_head, &Libprefix_tail, "", NULL);

    /* Check output file--if it exists and is an input file,
       we'll use "rebind" for that file. */
    if (stat(outfile, &sbuf) == 0) {
	outfile_ino = sbuf.st_ino;
	outfile_dev = sbuf.st_dev;
    }
    else {
	outfile_dev = -1;
	outfile_ino = -1;
    }

    /* check for object file existence before calling binder */
    /* and also eliminate duplicate specification of a filename */
    for (q = Arg_head; q != NULL; qprev = q, q = q->next) {
	if ((char *)q->info == &null_byte) {
	    /* File came from -bkeepfile:option */
	    info = new_filearg_info();
	    q->info = info;
	    info->flags |= KEEPFILE_FLAG | KEEP_FLAG;
	}
	else
	    info = (struct arg_info *)(q->info);

#ifdef STAT_DYNAMIC
	/* Ignore dummies for -bstatic and -bshared */
	if (info->flags & (STATIC_FLAG | SHARED_FLAG))
	    continue;
#endif

	q->value = getfilename(q->value, info);

	/* See if filename has already occurred */
	for (z = Arg_head; z != q; z = z->next) {
	    prev_info = (struct arg_info *)z->info;
	    if (prev_info->flags & (DELETED_FLAG | NOT_FOUND_FLAG))
		continue;
#ifdef STAT_DYNAMIC
	    if (prev_info->flags & (STATIC_FLAG | SHARED_FLAG))
		continue;
#endif
	    /* Check for duplicate file */
	    if (info->a_ino == prev_info->a_ino
		&& info->a_dev == prev_info->a_dev) {
		/* We have a duplicate.  See if the first one was from
		   -bkeepfile, and the second was not. */
		if ((prev_info->flags & KEEPFILE_FLAG) &&
		    !(info->flags & KEEPFILE_FLAG)) {
		    /* Delete the filename from the -bkeepfile option */
		    prev_info->flags |= DELETED_FLAG;
		    info->flags |= KEEP_FLAG;
		}
		else {
		    /* If current filename is from -bkeepfile option,
		       set flag on previous instance */
		    if (info->flags & KEEPFILE_FLAG)
			prev_info->flags |= KEEP_FLAG;
		    /* delete this duplicate file entry from list */
		    qprev->next = q->next;
		    q = qprev;
		}
		break;
	    }
	}

	if (!(info->flags & NOT_FOUND_FLAG)
	    && info->a_ino == outfile_ino
	    && info->a_dev == outfile_dev) {
	    /* This input file is the output file as well. */
	    info->flags |= REBIND_FLAG;
	}
    }

    for (q = Arg_head; q != NULL; q = q->next) {
	info = (struct arg_info *)(q->info);
#ifdef STAT_DYNAMIC
	if (info->flags & (STATIC_FLAG | SHARED_FLAG))
	    continue;
#endif
	if ((info->flags & (DELETED_FLAG | KEEPFILE_FLAG))
	    == (DELETED_FLAG | KEEPFILE_FLAG))
	    continue;
	++num_files;
	if (Garb_collect > 0) {
	    info->flags |= KEEP_FLAG;
	    Garb_collect--;
	}
    }

    /* check for import file existence--no check for duplicates */
    for (q = Imp_head; q != NULL; q = q->next) {
	num_imports++;
	if (accessx(q->value, R_OK, ACC_SELF) < 0) {
	    error(FILE_NOT_FOUND_FLAG, "ld:accessx()",
		  NLSMSG(LD_NOTFOUND1,
			 "ld: 0706-003 Cannot find or read import file: %s"),
		  q->value);
	}
    }

    /* check for export file existence--no check for duplicates */
    for (q = Exp_head; q != NULL; q = q->next) {
	num_exports++;
	if (accessx(q->value, R_OK, ACC_SELF) < 0) {
	    error(FILE_NOT_FOUND_FLAG, "ld:accessx()",
		  NLSMSG(LD_NOTFOUND2,
			 "ld: 0706-004 Cannot find or read export file: %s"),
		  q->value);
	}
    }

    if (num_files == 0
	&& Imp_head == NULL
	&& Exec1 == NULL
	&& Exec2 == NULL
	&& Exec3 == NULL
	&& Exec4 == NULL
	&& Exec5 == NULL
	&& f_flag_used == 0) {
	error(TERMINAL_FLAG, NULL,
	      NLSMSG(LD_NO_INPUT,
     "ld: 0706-030 No processing done.  Specify at least one\n"
	"\tinput or import file or use at least one -b ex<n> option."));
    }

    /* If errors occurred, we exit now. */
    if (Recover_flag == 0)
	cleanup(0);

    /* generate an argument vector for the binder */
    set_binder_args(av);		/* construct argv for binder */

    /* Call binder, unless -bnobind flag seen. */
    if (Call_binder && !nobind_seen)
	start_binder(av);	/* make output() the input to binder */

    /* if the binder is called then output will be piped to the binder.
       If not, then the output is to stdout if no syntax errors. */
    if (Call_binder || nobind_seen) {

	if (Debug_out_name) {
	    if ((Debug_out = fopen(Debug_out_name, "w")) == NULL) {
		error(IGNORE_FLAG, "ld:fopen()",
		      NLSMSG(LD_NOBINDCMDS,
 "ld: 0706-023 Cannot write the binder commands to file: %s"),
		      Debug_out_name);
	    }
	}

	output("halt %d\n", Halt_level[0]);

	/* set options */
	out_list("setopt ", Opt_head, 0);

	switch(Nop_type) {
	  case NOP_ORI:			/* ori: binder default */
	    break;
	  case NOP_CROR15:
	    output("nop primary " PRINT_CROR15 "\n"
		   "nop no "      PRINT_CROR31 "\n"
		   "nop no "      PRINT_ORI    "\n");
	    break;
	  case NOP_CROR31:
	    output("nop primary " PRINT_CROR31 "\n"
		   "nop no "      PRINT_ORI   "\n");
	    break;
	  case NOP_MANY:
	    skip = 0;
	    for (i = 0; ; i++) {
		switch(Nop_list[i]) {
		  case VAL_CROR15:
		    skip |= 1;
		    break;
		  case VAL_CROR31:
		    skip |= 2;
		    break;
		  case VAL_ORI:
		    skip |= 4;
		    break;
		}
		if (i == Nop_count-1) {
		    output("nop primary %x\n", Nop_list[i]);
		    break;
		}
		output("nop %x\n", Nop_list[i]);
	    }
	    if (!(skip & 1))
		output("nop no "    PRINT_CROR15 "\n");
	    if (!(skip & 2))
		   output("nop no " PRINT_CROR31 "\n");
	    if (!(skip & 4))
		   output("nop no " PRINT_ORI    "\n");
	    break;
	}

	if (Stabcmpct[0] != -1)
	    output("setopt stabcmpct:%d\n", Stabcmpct[0]);

	switch(Reorder) {
	  case 1:
	    break;			/* Default */
	  case 0:			/* -b noreorder */
	    output("setopt mapping:dataintext\n");
	    output("setopt fold\n");
	    /* Fall through */
	  case -1:			/* -b noobjreorder */
	    output("setopt reorder:by_address\n");
	    break;
	}

	output("savename %s\n", escape_name(outfile));

	if (Garb_collect == -1)
	    output("setopt keepall\n");

	output("filelist %d %d\n",
	       num_files,
	       num_imports + num_exports + (Glink_file[0] ? 1 : 0));

	if (Exec1)
	    output("exec %s\n", escape_name(Exec1));

	/* insert object files */
	out_argnames("i ", "lib ", "rebind ", "keep ");

	out_list("imports ", Imp_head, 1 /* Escape file name */);
	out_list("rename ", Rnm_head, 0);
	out_list("exports ", Exp_head, 1 /* Escape file name */);

	if (Export_all)
	    output("expall\n");

	if (noentry)
	    output("noentry\n");
	else if (entry)	/* entry point */
	    output("entry %s\n", entry);

	/* Miscellaneous */

	out_list("keep ", Keep_head, 0);

	if (Exec2)
	    output("exec %s\n", escape_name(Exec2));
	output("resolve\n");
	if (Exec3)
	    output("exec %s\n", escape_name(Exec3));

	if (Glink_file[0])
	    output("addgl %s\n", escape_name(Glink_file[0]));

	if (Ext_check)
	    output("er full\n");

	output("mismatch\n");

	if (Comprld)
	    output("comprld\n");

	/* text & data origin */
	if (text_org[1] == data_org[1]) {
	    if (text_org[1] == 0)
		if (data_org[0] == 0xFFFFFFFFL)
		    output("origin 0x%x -1\n", text_org[0]);
		else
		    output("origin 0x%x 0x%x\n", text_org[0], data_org[0]);
	    else
		output("origin page 0x%x 0x%x\n", text_org[0], data_org[0]);
	}
	else if (text_org[1] == 0) {
	    output("origin 0x%x\n", text_org[0]);
	    output("origin page . 0x%x\n", data_org[0]);
	}
	else {
	    output("origin page 0x%x\n", text_org[0]);
	    output("origin . 0x%x\n", data_org[0]);
	}

	if (rwalign > 0L)
	    output("pad %ld %ld %ld\n", rwalign, rwalign, rwalign);

	if (Max_stack != 0L)
	    output("maxstack 0x%x\n", Max_stack);

	if (Max_data != 0L)
	    output("maxdata 0x%x\n", Max_data);

	if (Libpath != (char *)-1) {
	    if (Libpath)
		output("libpath %s\n", escape_name(Libpath));
	    else if (Lib_head != NULL) {
		output("libpath ");
		for (q = Lib_head; q; q = q->next)
		    output("%s:", escape_name(q->value));

		p = DEF_LIBPATH;
		output("%s\n", p);
	    }
	}

	if (Exec4)
	    output("exec %s\n", escape_name(Exec4));

	output("save %s .\n", Mod_type[0]);

	if (Exec5)
	    output("exec %s\n", escape_name(Exec5));

	if (Map_file)
	    output("gen map %s\n", escape_name(Map_file));

	if (Xref_file)
	    output("gen xref %s\n", escape_name(Xref_file));

	if (Call_file)
	    output("gen calls %s\n", escape_name(Call_file));

	if (SMap_file)
	    output("gen smap %s\n", escape_name(SMap_file));

	if (SXref_file)
	    output("gen sxref %s\n", escape_name(SXref_file));

	if (SCall_file)
	    output("gen scalls %s\n", escape_name(SCall_file));

	output("rc\n");
    }

    cleanup(0);
    /*NOTREACHED*/
}

/************************************************************************
 * Name: add_list
 *									*
 * Purpose: Add a node to a linked list.
 *									*
 ************************************************************************/
static void
add_list(struct dlink **head,
	 struct dlink **tail,
	 char *value,
	 void *info)
{
    struct dlink *z;

    /* allocate memory for the node */
    z = allocate(sizeof(struct dlink));

    /* assign values */
    z->value = value;
    z->info = (info == NULL) ? (void *)&null_byte : info;
    z->next = NULL;

    if (*tail == NULL)			/* no items in the list */
	*head = z;
    else				/* Add the item to the list. */
	(*tail)->next = z;
    *tail = z;
}
/************************************************************************
 * Name: handle_sigpipe
 *									*
 * Purpose: Call cleanup with an indication that SIGPIPE was received.
 *									*
 * Does not return
 ************************************************************************/
static void
handle_sigpipe(int sig) {
    cleanup(1);				/* Specify that SIGPIPE seen */
}
/************************************************************************
 * Name: cleanup
 *									*
 * Purpose: Wait for the binder to exit (if called) and get the final
 *	return code.
 *									*
 * Does not return
 ************************************************************************/
static void
cleanup(int sigpipe_seen)
{
    int retval;

    if (!sigpipe_seen) {
	/* make sure the data buffers are written */
	(void) fclose(Out_file);
    }

    if (Debug_out)
	if (fclose(Debug_out) < 0) {
	    error(IGNORE_FLAG, "ld:fclose()",
		  NLSMSG(LD_NOBINDCMDS,
 "ld: 0706-023 Cannot write the binder commands to file: %s"),
		  Debug_out_name);
	}

    if (Call_binder && !nobind_seen) {
	/* wait for binder to finish */
	if (wait(&retval) < 0)
	    error(TERMINAL_FLAG, "ld:wait()",
		  NLSMSG(LD_SYSCALLFAIL,
			 "ld: 0706-021 The %s() system call failed."),
		  "wait");

	if (WIFEXITED(retval)) {	/* child called exit() */
	    retval = WEXITSTATUS(retval);
	    if (retval >= 8) {
		if (retval != 255)	/* Assume that the binder never
					   returns 255, so the child must
					   have exited with an error
					   without calling bind, and we
					   preserve the return code of 255. */
		    retval &= ~1;	/* Clear low order bit */
	    }
	    else if (retval & 1)
		retval = 1;
	    else
		retval = 0;		/* Return 0 for binder warnings */
	}
	else if (WIFSIGNALED(retval)) {	/* killed by signal */
	    /* psignal() is a non-standard function.  It prints its second
	       argument, a colon, and the signal number description.
	       error() is not used to print LD_BINDKILLED, because it
	       automatically adds a newline. */
	    extern int psignal(unsigned int, char *);
	    int msg = NLSMSG(LD_BINDKILLED,
		     "\nld: 0706-010 The binder was killed by a signal");
	    char *fmt = catgets(catd, LD_ERR_MSG, msg, def_message(msg));

	    psignal(WTERMSIG(retval), fmt); /* Message goes to stderr */
	    error(IGNORE_FLAG, NULL,
	      NLSMSG(LD_BINDKILLED2,
     "\tCheck for binder messages or use local problem reporting procedures."));
	    retval = 254;
	}
	else {	/* Shouldn't happen */
	    error(IGNORE_FLAG, NULL,
		  NLSMSG(LD_WAITFAILED,
"ld: 0706-024 The wait() system call returned %d.\n"
"\tCheck for binder messages or use local problem reporting procedures."),
		  retval);
	    retval = 254;
	}
    }
    else if (Exit_flag)
	retval = 255;
    else
	retval = 0;

    catclose(catd);
    exit(retval);
}

/************************************************************************
 * Name: error
 *									*
 * Purpose: Print an error message to standard error.  Depending on the
 *	error level, status flags are set or cleanup() is called.
 *									*
 * Arguments:
 *									*
 ************************************************************************/
static void
error(error_flag_t flag,		/* Message severity */
      char *perror_string,		/* If not null, call perror() */
      int message,			/* Message to print */
      ...)
{
    va_list	ap;
    char	*fmt;

    va_start(ap, message);

    /* Get message format from message catalog */
    fmt = catgets(catd, LD_ERR_MSG, message, def_message(message));

    (void) vfprintf(stderr, fmt, ap);
    (void) putc('\n', stderr);
    if (perror_string) {
	(void) putc('\t', stderr);
	perror(perror_string);
    }

    va_end(ap);			/* clean up */

    /* if syntax error in one flag, go ahead and recover to parse
       other flags on the command line; if a flag is to be ignored
       then just skip it; if an error is severe, exit the program. */
    switch(flag) {
      case SYNTAX_FLAG:
	Recover_flag = 0;		/* Don't generate list of commands */
	/* Fall through */

      case FILE_NOT_FOUND_FLAG:
	Exit_flag = 1; /* let cleanup() know of error */
	Call_binder = 0;		/* do not call the binder */
	break;
      case TERMINAL_FLAG:		/* Unrecoverable error */
	exit(255);
	break;
      case IGNORE_FLAG:			/* Informational message only */
	break;
      default:
	/* Print internal error message.  Avoid recursive call to error(). */
	message = NLSMSG(LD_INTERNAL_ERR,
			 "ld: 0706-002 Internal error at line %1$d (v=%2$d).\n"
"\tDepending on where this product was acquired, contact your service\n"
			 "\trepresentative or the approved supplier.");
	fmt = catgets(catd, LD_ERR_MSG, message, def_message(message));
	fprintf(stderr, fmt, __LINE__, flag);
	(void) putc('\n', stderr);
	exit(255);
    }
}
/************************************************************************
 * Name: getfilename
 *									*
 * Purpose: Get the real name for 'file' and make sure it's valid.
 *	Files are opened because that's how it has always been done.
 *									*
 * Arguments: Filename, info structure.  The filename can by an -l flag.
 *									*
 * Returns the real file name
 ************************************************************************/
static char *
getfilename(char *file,
	    struct arg_info *info)
{
    char *p = file;
    struct stat buf;

    Fd = -1;				/* initialize */

    /* if it's an -l argument, generate a pathname for it */
    if (info->flags & LIBRARY_FLAG)
	p = getlibname(p);	/* also opens Fd */
    else {
	Fd = open(p, O_RDONLY);
	if (Fd == -1) {
	    error(FILE_NOT_FOUND_FLAG, "ld:open()",
		  NLSMSG(LD_NOTFOUND3,
			 "ld: 0706-005 Cannot find or open file: %s"),
		  p);
	}
    }

    if (Fd != -1) {
	fstat(Fd, &buf);
	info->a_dev = buf.st_dev;
	info->a_ino = buf.st_ino;
	close(Fd);
    }
    else
	info->flags |= NOT_FOUND_FLAG;

    return p;	/* name of file to 'insert' */
}

/************************************************************************
 * Name: get_files
 *									*
 * Purpose: Read a list of input object filenames from a file. Add the
 *	names to the Arg_head list.
 *									*
 * Arguments: Filename
 *									*
 ************************************************************************/
static void
get_files(char *f)
{
    char *p;
    int i;
    char buffer[LINE_MAX+1];
    char *b;
    FILE *fp;

#ifdef CENTERLINE
    glob_t matched_names;
    matched_names.gl_pathc = 0;
    matched_names.gl_pathv = NULL;
    matched_names.gl_offs = 0;
    matched_names.gl_padr = NULL;
    matched_names.gl_ptx = NULL;
#else
    glob_t matched_names = {0, NULL, 0, NULL, NULL};
#endif

    if ((fp = fopen(f, "r")) == NULL) {
	error(SYNTAX_FLAG, "ld:fopen()",
	      NLSMSG(LD_NOTFOUND3,
		     "ld: 0706-005 Cannot find or open file: %s"),
	      f);
    }
    else {
	while (fgets(buffer, LINE_MAX, fp) != NULL) {
	    /* Skip leading whitespace */
	    for (b = buffer; *b == ' ' || *b == '\t'; b++)
		; /* Skip */
	    if ((p = strrchr(b, '\n')) == NULL) {
		/* Line too long */
		error(SYNTAX_FLAG, NULL,
		      NLSMSG(LD_LINE_TOOLONG,
		     "ld: 0706-008 A line is too long in file: -f %1$s\n"
	"\tThe maximum line length is %2$d bytes."),
		      f, LINE_MAX);
		while ((i = getc(fp)) != EOF)
		    if (i == '\n')
			break;
		continue;
	    }
	    /* Strip trailing whitespace */
	    while (p > b && (p[-1] == ' ' || p[-1] == '\t'))
		p--;
	    *p = '\0';			/* Terminate string */

	    if (b[0] == '\0')
		continue;		/* Empty line */
	    if (glob(b,
#ifdef AIX_REL3
		     GLOB_QUOTE |
#endif
		     GLOB_NOCHECK, NULL, &matched_names)
		== GLOB_NOSPACE) {
		error(TERMINAL_FLAG, NULL,
		      NLSMSG(LD_OUTOFMEM1,
     "ld: 0706-007 There is not enough memory available.  Check your ulimit\n"
     "\tor paging space or use local problem reporting procedures."));
	    }

	    for (i = 0; i < matched_names.gl_pathc; i++) {
		add_list(&Arg_head, &Arg_tail,
			 matched_names.gl_pathv[i],
			 new_filearg_info());
	    }
	    /* Don't free glob() areas, because they contain
	       the path names of the matched files. */
	}

	(void) fclose(fp);
    }
} /* get_files */

/************************************************************************
 * Name: checkliblist
 *									*
 * Purpose: Return a file descriptor of a library, if it can be found in
 *	one of a list of directories.
 *									*
 * Arguments:
 *									*
 * Returns: File descriptor, if library found.
 *	-1, otherwise
 ************************************************************************/
static int
checkliblist(struct dlink *list,
	     char *pattern1,
	     char *pattern2,
	     char *file,
	     char *buffer)
{
    int fd = -1;
    int len, len1, len2;

    len = strlen(file) - 4;		/* For 2 %s strings in pattern[12] */
    len1 = len + strlen(pattern1);
    if (pattern2)
	len2 = len + strlen(pattern2);

    while (list != NULL) {
	len = strlen(list->value);
	if (len1 + len <= PATH_MAX) {
	    (void) sprintf(buffer, pattern1, list->value, file);
	    fd = open(buffer, O_RDONLY);
	    if (fd != -1)
		break;
	}
	else {
	    errno = ENAMETOOLONG;
	    buffer[0] = '\0';		/* Buffer not used */
	}
	if (pattern2 && len2 + len <= PATH_MAX) {
	    (void) sprintf(buffer, pattern2, list->value, file);
	    fd = open(buffer, O_RDONLY);
	    if (fd != -1)
		break;
	}
	else {
	    errno = ENAMETOOLONG;
	    buffer[0] = '\0';		/* Buffer not used */
	}
	list = list->next;
    }
    return fd;
}
/************************************************************************
 * Name: getlibname
 *									*
 * Purpose: Return the full pathname for a library specified with an
 *	-l flag.
 *									*
 * Arguments: Library name
 *									*
 * Returns: File name, if found.
 *	"Almost" last file name tried, it not found
 *	and !Call_binder || nobind_seen NULL, otherwise.  Two buffers are
 *	used so files that are not found display in error messages with the
 *	first path in the default liblist.
 ************************************************************************/
static char *
getlibname(char *file)
{
    char	buffer[PATH_MAX+1];
    char	*buf;

    Fd = -1;			/* Initialize error value */

    /* Check list of libraries supplied with -L */
    Fd = checkliblist(Lib_head, "%s/lib%s.a", NULL, file, buffer);


    if (Fd == -1) {		/*See if it's in default library(ies) */
	Fd = checkliblist(Libprefix_head,
			  "%s/usr/lib/lib%s.a", "%s/lib/lib%s.a",
			  file, buffer);
    }

    if (Fd != -1) {
	buf = allocate(strlen(buffer) + 1);
	(void) strcpy(buf, buffer);
    }
    else {
	error(FILE_NOT_FOUND_FLAG, "ld:open()",
	      NLSMSG(LD_LIBNOTFOUND,
		     "ld: 0706-006 Cannot find or open library file: -l %s"),
	      file);

	if (Call_binder && !nobind_seen)
	    return NULL;

	/* If we are printing commands,
	   make sure the last name tried is returned. */
	if (buffer[0] != '\0') {
	    buf = allocate(strlen(buffer) + 1);
	    (void) strcpy(buf, buffer);
	}
	else {
	    buf = allocate(strlen(file) + 1);
	    (void) strcpy(buf, file);
	}
    }
    return buf;
}

/************************************************************************
 * Name: out_list
 *									*
 * Purpose: Print (using output()) a linked list with the given command
 *	for each element.
 *									*
 * Arguments:
 *									*
 ************************************************************************/
static void
out_list(char *command,
	 struct dlink *p,
	 int use_escape_name)
{
    char *temp_v;

    while (p != NULL) {
	if (use_escape_name)
	    temp_v = escape_name(p->value);
	else
	    temp_v = p->value;

	if (p->info)
	    output("%s%s %s\n", command, temp_v, p->info);
	else
	    output("%s%s\n", command, temp_v);
	p = p->next;
    }
}
/************************************************************************
 * Name: new_filearg_info
 *									*
 * Purpose: Allocate a new arg_info structure and return it.
 *									*
 * Arguments: None
 *									*
 ************************************************************************/
static struct arg_info *
new_filearg_info(void)
{
    struct arg_info *r = allocate(sizeof(struct arg_info));

    r->flags = 0;
    return r;
}

/************************************************************************
 * Name: escape_name
 *									*
 * Purpose: Convert a name with escapes if white space exists
 *		in the name, or if the name begins with \.
 *									*
 * Arguments: The name to be printed.
 *									*
 * Returns: The converted name.
 ************************************************************************/
static char *
escape_name(char *name)
{
    char *result, *r;

    if (strpbrk(name, " \t\n") == NULL && name[0] != '\\')
	return name;
    result = allocate(2 * strlen(name) + 2);
    r = result;
    *r++ = '\\';
    while (*name) {
	switch(*name) {
	  case ' ':
	    *r++ = '\\';
	    *r++ = 's';
	    break;
	  case '\n':
	    *r++ = '\\';
	    *r++ = 'n';
	    break;
	  case '\t':
	    *r++ = '\\';
	    *r++ = 't';
	    break;
	  case '\\':
	    *r++ = '\\';
	    *r++ = '\\';
	    break;
	  default:
	    *r++ = *name;
	}
	++name;
    }
    *r = '\0';				/* Terminate */
    return result;
}

/************************************************************************
 * Name: out_argnames
 *									*
 * Purpose: Print the list of input files (headed by Arg_head),
 *	using the appropriate binder commands, as specified by the flags
 *	field of the file name structures.
 *									*
 * Arguments:
 *									*
 ************************************************************************/
static void
out_argnames(char *command,
	     char *lib_command,
	     char *rebind_command,
	     char *keepfile_attribute)
{
    struct arg_info	*info;
    struct dlink *p;
    for (p = Arg_head; p != NULL; p = p->next) {
	info = (struct arg_info *)(p->info);
#ifdef STAT_DYNAMIC
	if (info->flags & (SHARED_FLAG | STATIC_FLAG)) {
	    if (info->flags & SHARED_FLAG)
		output("setopt %s\n", Str_autoimp);
	    else
		output("setopt %s\n", Str_noautoimp);
	    continue;
	}
#endif
	if (info->flags & DELETED_FLAG)
	    continue;

	/* Note that if an input file is specified with "-l" and is also
	   the output file, the "library" command won't be generated. Over-
	   writing the library is probably an error in the first place. */
	if (info->flags & REBIND_FLAG)
	    output("%s", rebind_command);
	else if (info->flags & LIBRARY_FLAG)
	    output("%s", lib_command);
	else
	    output("%s", command);

	if (info->flags & KEEP_FLAG)
	    output("%s", keepfile_attribute);

	output("%s", escape_name(p->value));

	output("\n");
    }
}

/************************************************************************
 * Name: output
 *									*
 * Purpose: Print binder commands to Out_file (the pipe to the binder)
 *	and to Debug_out, the file specified with the -bbindcmds option.
 *									*
 * Arguments: Same as printf()
 *									*
 ************************************************************************/
static void
output(const char *fmt,
       ...)
{
    va_list	ap;

    va_start(ap, fmt);		/* initialize argument list */

    /* write command */
    (void) vfprintf(Out_file, fmt, ap);

    if (Debug_out && Debug_out_ok == 1)
	if (vfprintf(Debug_out, fmt, ap) < 0) {
	    error(IGNORE_FLAG, "ld:vfprintf()",
		  NLSMSG(LD_NOBINDCMDS,
 "ld: 0706-023 Cannot write the binder commands to file: %s"),
		  Debug_out_name);
	    (void) fclose(Debug_out);	/* Keep from generating more errors */
	    Debug_out = NULL;
	}
    va_end(ap);			/* clean up */
}

/*
 * this structure is for use in the options table
 * so that both the head and tail of a list can
 * be accessed.
 */
static struct lists {
    struct dlink	**head;		/* address of head of linked list */
    struct dlink	**tail;		/* address of tail of linked list */
} Lists[] = {
#define	LST_IMP	0
    { &Imp_head,	&Imp_tail	},	/* import files */
#define	LST_EXP	1
    { &Exp_head,	&Exp_tail	},	/* export files */
#define	LST_RNM	2
    { &Rnm_head,	&Rnm_tail	},	/* rename symbols */
#define	LST_OPT	3
    { &Opt_head,	&Opt_tail	},	/* setopt options */
#define	ARG_OPT	4
    { &Arg_head,	&Arg_tail	},	/* Files to insert */
};

static struct opts {
    char	*arg;	/* name of -b option */
    char	num_args;
    char	flags;
/* Define bit masks for most cases */
#define F_NULLABLE 1			/* Null argument means null value */
#define F_DEFAULT 3			/* Null argument restores default.
					   F_DEFAULT implies F_NULLABLE. */
#define F_VAL_MASK 12
#define F_ON 4				/* Set value to 1 */
#define F_OFF 8				/* Set value to 0 */
#define F_MINUS1 12			/* Set value to -1 */

/* Define values for TP_SPECIAL options */
#define SP_NOP 1
#define SP_STATIC 2
#define SP_SHARED 3

    char	type;			/* type of entry for this option */
#define	TP_SETOPT	0		/* SETOPT "value" */
#define	TP_SETOPT_COPY	1		/* Use flag as SETOPT "value" */
#define	TP_ADDLIST	2		/* add 'opt1' to linked list */
#define	TP_VALUE	3		/* assign specified value to var. */
#define	TP_STRING	4		/* 'opt1' is a character string */
#define	TP_DECIMAL	5		/* 'opt1' is a decimal number */
#define	TP_NUMBER	6		/* 'opt1' is a number */
#define TP_SILENT	7		/* Ignore option with no message */
#define TP_IGNORE	8		/* Ignore this option (obsolete) */
#define TP_ORIGIN	9		/* Special case of NUMBER for orgs */
#define TP_SPECIAL	10		/* Other special cases.  Flags field
					   contains type of special case. */
    void	*var;			/* Address of variable to modify */
    /* Pointers should be to
	char *		for TP_STRING and TP_SETOPT
	struct lists	for TP_ADDLIST
	int		for TP_DECIMAL and TP_VALUE
	long		for TP_NUMBER and TP_ORIGIN
	NULL		for TP_IGNORE
	?		for TP_SPECIAL (depends on flags)
	*/
} Opts[] = {	/* NOTE: Option names must be in alphabetical order.
		   Options beginning with "no" are listed in the Opts_no[]
		   array below. */
    { "C",		1, F_NULLABLE,	TP_STRING,	&Call_file	},
    { "D",		1, F_NULLABLE,	TP_NUMBER,	&Max_data	},
    { "E",		1, 0,		TP_ADDLIST,	&Lists[LST_EXP]},
    { "I",		1, 0,		TP_ADDLIST,	&Lists[LST_IMP]},
    { "M",		1, F_DEFAULT,	TP_STRING,	&Mod_type},
    { "R",		1, F_NULLABLE,	TP_STRING,	&Map_file},
    { "S",		1, F_NULLABLE,	TP_NUMBER,	&Max_stack},
    { "X",		1, F_NULLABLE,	TP_STRING,	&Xref_file},
    { "asis",		0, 0,		TP_SETOPT_COPY,	NULL},
    { "autoimp",	0, 0,		TP_SETOPT_COPY,	NULL},
    { "bigtoc",		0, 0,		TP_SETOPT_COPY,	NULL},
    { "bindcmds",	1, F_NULLABLE,	TP_STRING,	&Debug_out_name},
    { "binder",		1, F_DEFAULT,	TP_STRING,	&Binder	},
    { "bindopts",	1, F_NULLABLE,	TP_STRING,	&Dump_args},
    { "calls",		1, F_NULLABLE,	TP_STRING,	&Call_file},
    { "caps",		0, 0,		TP_SETOPT,	&Str_noasis},
    { "comprld",	0, F_ON,	TP_VALUE,	&Comprld},
    { "crld",		0, F_ON,	TP_VALUE,	&Comprld},
    { "cror15",		0, F_MINUS1,	TP_VALUE,	&Nop_type},
    { "cror31",		0, F_ON,	TP_VALUE,	&Nop_type},
    { "dbg",		1, 0,		TP_ADDLIST,	&Lists[LST_OPT]},
    { "debugopt",	1, 0,		TP_ADDLIST,	&Lists[LST_OPT]},
    { "delcsect",	0, 0,		TP_SETOPT_COPY,	NULL},
#ifdef STAT_DYNAMIC
    { "dynamic",	0, SP_SHARED,	TP_SPECIAL,	NULL},
#endif
    { "ernotok",	0, F_ON,	TP_VALUE,	&Ext_check},
    { "erok",		0, F_OFF,	TP_VALUE,	&Ext_check},
    { "errmsg",		0, 0,		TP_SETOPT_COPY,	NULL},
    { "ex1",		1, F_NULLABLE,	TP_STRING,	&Exec1	},
    { "ex2",		1, F_NULLABLE,	TP_STRING,	&Exec2	},
    { "ex3",		1, F_NULLABLE,	TP_STRING,	&Exec3	},
    { "ex4",		1, F_NULLABLE,	TP_STRING,	&Exec4	},
    { "ex5",		1, F_NULLABLE,	TP_STRING,	&Exec5	},
    { "expall",		0, F_ON,	TP_VALUE,	&Export_all},
    { "export",		1, 0,		TP_ADDLIST,	&Lists[LST_EXP]},
    { "f",		0, F_ON,	TP_VALUE,	&Ext_check},
    { "filelist",	0, 0,		TP_IGNORE,	NULL	},
    { "fl",		0, 0,		TP_IGNORE,	NULL	},
    { "forceimp",	0, 0,		TP_SILENT,	NULL},
    { "gc",		0, F_OFF,	TP_VALUE,	&Garb_collect},
    { "gcbypass",	1, F_NULLABLE,	TP_DECIMAL,	&Garb_collect},
    { "glink",		1, F_DEFAULT,	TP_STRING,	&Glink_file},
    { "h",		1, F_DEFAULT,	TP_DECIMAL,	&Halt_level},
    { "halt",		1, F_DEFAULT,	TP_DECIMAL,	&Halt_level},
    { "i",		0, 0,		TP_IGNORE,	NULL	},
    { "import",		1, 0,		TP_ADDLIST,	&Lists[LST_IMP]},
    { "insert",		0, 0,		TP_IGNORE,	NULL	},
    { "keepfile",	1, 0,		TP_ADDLIST,	&Lists[ARG_OPT]},
    { "l",		1, F_NULLABLE,	TP_STRING,	&Load_map},
    { "libpath",	1, F_NULLABLE,	TP_STRING,	&Libpath},
    { "loadmap",	1, F_NULLABLE,	TP_STRING,	&Load_map},
    { "map",		1, F_NULLABLE,	TP_STRING,	&Map_file},
    { "maxdata",	1, F_NULLABLE,	TP_NUMBER,	&Max_data},
    { "maxstack",	1, F_NULLABLE,	TP_NUMBER,	&Max_stack},
    { "modtype",	1, F_DEFAULT,	TP_STRING,	&Mod_type},
    { "n",		0, F_OFF,	TP_VALUE,	&Reorder},
    { "nl",		0, 0,		TP_STRING,	&Load_map},
    { "nro",		0, 0,		TP_SETOPT,	&Str_norow},
    { "nso",		0, 0,		TP_SETOPT,	&Str_noautoimp},
    { "objreorder",	0, F_ON,	TP_VALUE,	&Reorder},
    { "output",		1, F_NULLABLE,	TP_STRING,	&Debug_out_name},
    { "pD",		1, 0,		TP_ORIGIN,	&data_org	},
    { "pT",		1, 0,		TP_ORIGIN,	&text_org	},
    { "quiet",		0, F_ON,	TP_VALUE,	&Quiet	},
    { "r",		0, F_ON,	TP_VALUE,	&Reorder},
    { "rename",		2, 0,		TP_ADDLIST,	&Lists[LST_RNM]},
    { "reorder",	0, F_ON,	TP_VALUE,	&Reorder},
    { "ro",		0, 0,		TP_SETOPT,	&Str_row},
    { "scalls",		1, F_NULLABLE,	TP_STRING,	&SCall_file},
#ifdef STAT_DYNAMIC
    { "shared",		0, SP_SHARED,	TP_SPECIAL,	NULL},
#endif
    { "smap",		1, F_NULLABLE,	TP_STRING,	&SMap_file},
    { "so",		0, 0,		TP_SETOPT,	&Str_autoimp},
    { "stabcmpct",	1, F_DEFAULT,	TP_DECIMAL,	&Stabcmpct},
#ifdef STAT_DYNAMIC
    { "static",		0, SP_STATIC,	TP_SPECIAL,	NULL},
#endif
    { "strcmpct",	0, 0,		TP_IGNORE,	NULL},
    { "sxref",		1, F_NULLABLE,	TP_STRING,	&SXref_file},
    { "textro",		0, 0,		TP_SETOPT,	&Str_row},
    { "typchk",		0, 0,		TP_SETOPT_COPY,	NULL},
    { "x",		0, 0,		TP_SETOPT,	&Str_x	},
    { "xref",		1, F_NULLABLE,	TP_STRING,	&Xref_file},
    { NULL,		0, 0,		0,		NULL}
},

    Opts_no[] = {	/* NOTE: Option names must be in alphabetical order.
			   The leading "no" is implied.  */
    { "autoimp",	0, 0,		TP_SETOPT_COPY,	NULL},
    { "bigtoc",		0, 0,		TP_SETOPT_COPY,	NULL},
    { "bind",		0, F_ON,	TP_VALUE,	&nobind_seen},
    { "comprld",	0, F_OFF,	TP_VALUE,	&Comprld},
    { "crld",		0, F_OFF,	TP_VALUE,	&Comprld},
    { "delcsect",	0, 0,		TP_SETOPT_COPY,	NULL},
    { "entry",		0, F_ON,	TP_VALUE,	&noentry},
    { "errmsg",		0, 0,		TP_SETOPT_COPY,	NULL},
    { "expall",		0, F_OFF,	TP_VALUE,	&Export_all},
    { "forceimp",	0, 0,		TP_SILENT,	NULL},
    { "gc",		0, F_MINUS1,	TP_VALUE,	&Garb_collect},
    { "glink",		0, 0,		TP_STRING,	&Glink_file},
    { "libpath",	0, F_MINUS1,	TP_VALUE,	&Libpath},
    { "loadmap",	0, 0,		TP_STRING,	&Load_map},
    { "m",		0, 0,		TP_SETOPT,	&Str_nomember},
    { "objreorder",	0, F_MINUS1,	TP_VALUE,	&Reorder},
    /* The -b nop option is not a "no" option,
       but just happens to begin with "no". */
    { "p",		1, SP_NOP,	TP_SPECIAL,	NULL},
    { "quiet",		0, F_OFF,	TP_VALUE,	&Quiet},
    { "reorder",	0, F_OFF,	TP_VALUE,	&Reorder},
    { "ro",		0, 0,		TP_SETOPT,	&Str_norow},
    { "so",		0, 0,		TP_SETOPT,	&Str_noautoimp},
    { "strcmpct",	0, 0,		TP_IGNORE,	NULL},
    { "strip",		0, 0,		TP_SETOPT_COPY,	NULL},
    { "textro",		0, 0,		TP_SETOPT,	&Str_norow},
    { "typchk",		0, 0,		TP_SETOPT_COPY,	NULL},
    { "v",		0, 0,		TP_SETOPT,	&Str_nov},
    { "x",		0, 0,		TP_SETOPT,	&Str_nox},
    { NULL,		0, 0,		0,		NULL}
};

/************************************************************************
 * Name: convert_arg
 *									*
 * Purpose: Provide a single place for converting ascii to ulongs.
 *									*
 * Arguments: The option (which includes its arguments)
 *									*
 * Side Effects:
 *	convert_valid is set to 0 if an error occurs. (The caller must
 *	initialize convert_valid to a non-zero value if it wishes to
 *	check the result.
 *
 * Returns:
 *	The return value from strtoul().
 ************************************************************************/
ulong
convert_arg(char *number,
	    int base,
	    char *option_name)
{
    ulong v;
    char *ptr;

    errno = 0;
    v = strtoul(number, &ptr, base);
    if (errno || *ptr != '\0') {
	convert_valid = 0;
	if (errno)
	    if (option_name == NULL)
		error(SYNTAX_FLAG, NULL,
		      NLSMSG(LD_BADNUM,
     "ld: 0706-019 The number %1$s is too large for the -%2$c flag.\n"
	"\tUse a value that is %3$u (hex %3$X) or less."),
		      number, optopt, ULONG_MAX);
	    else
		error(SYNTAX_FLAG, NULL,
		      NLSMSG(LD_BADNUMOPT,
     "ld: 0706-020 The number %1$s is too large for the -b %2$s option.\n"
	"\tUse a value that is %3$u (hex %3$X) or less."),
		      number, option_name, ULONG_MAX);
	else if (option_name == NULL)
	    error(SYNTAX_FLAG, NULL,
		  NLSMSG(LD_BADSYN2,
		 "ld: 0706-029 Use a number with the -%c flag."),
		  optopt);
	else
	    error(SYNTAX_FLAG, NULL,
		  NLSMSG(LD_BADSYN1,
	 "ld: 0706-028 Use a number with the -b %s option."),
		  option_name);
    }
    return v;
}
/************************************************************************
 * Name: parse_bflag
 *									*
 * Purpose: Process all -b <opt> options.
 *									*
 * Arguments: The option (which includes its arguments)
 *									*
 * Side Effects:
 *	Error status variables are set as appropriate.
 *
 ************************************************************************/
static void
parse_bflag(char *arg)
{
    char		*p, *p1;
    struct opts		*o;
    struct arg_info	*info;
    char		*q,
			*flag_arg,
			*flag_arg2 = NULL,
			*ptr;		/* argument to strtol() */
    int			found, n;
    long		v;
    int			*int_var;
    long		*long_var;
    char		**charptr_var;
    FILE		**FILEptr_var;

    for (p = arg; p != NULL; p = q, flag_arg2 = NULL) {
	if ((q = strchr(p, ' ')) != NULL)
	    *q++ = '\0';		/* q points to next option */

	if ((flag_arg = strchr(p, ':')) != NULL)
	    *flag_arg++ = '\0';

	if (flag_arg != NULL && (flag_arg2 = strchr(flag_arg, ',')) != NULL)
	    *flag_arg2++ = '\0';

	if (p[0] == 'n' && p[1] == 'o') {
	    p1 = p+2;
	    o = Opts_no;
	}
	else {
	    p1 = p;
	    o = Opts;
	}

	for (found = 0; o->arg != NULL; ++o) {
	    if ((n = strcmp(p1, o->arg)) < 0)
		break;
	    else if (n == 0) {
		found = 1;

		/* check for bad arguments */
		switch(o->num_args) {
		  case 0:
		    if (flag_arg) {
			error(SYNTAX_FLAG, NULL, NLSMSG(LD_BNOARG,
	     "ld: 0706-016 The -b %s option does not take a parameter."),
			      p);
			continue;
		    }
		    break;
		  case 1:
		    if (flag_arg == NULL
			|| (*flag_arg == '\0'
			    && o->type != TP_SPECIAL
			    && !(o->flags & F_NULLABLE))) {
			error(SYNTAX_FLAG, NULL,
			      NLSMSG(LD_ARGBREQ,
		     "ld: 0706-015 The -b %1$s option needs a parameter.\n"
	"\tOption syntax: -b %1$s:PARM"),
			      p);
			continue;
		    }
		    else if (flag_arg2) {
			error(SYNTAX_FLAG, NULL,
			      NLSMSG(LD_BNO2,
	     "ld: 0706-018 The -b %1$s option does not take two parameters.\n"
	"\tOption syntax: -b %1$s:PARM"),
			      p);
			continue;
		    }
		    break;
		  case 2:
		    /* The only option with two arguments is "rename" and it
		       isn't F_NULLABLE, so we don't check for that here. */
		    if (flag_arg == NULL || flag_arg2 == NULL
			|| *flag_arg == '\0' || *flag_arg2 == '\0') {
			error(SYNTAX_FLAG, NULL,
			      NLSMSG(LD_B2ARG,
		  "ld: 0706-017 The -b %1$s option needs two parameters.\n"
				     "\tOption syntax: -b %1$s:PARM1,PARM2"),
			      p);
			continue;
		    }
		    break;
		  default:
		    error(TERMINAL_FLAG, NULL,
			  NLSMSG(LD_INTERNAL_ERR,
			 "ld: 0706-002 Internal error at line %1$d (v=%2$d).\n"
"\tDepending on where this product was acquired, contact your service\n"
				 "\trepresentative or the approved supplier."),
			  __LINE__, o->num_args);
		}

		switch(o->type) {
		  case TP_SILENT:
		    break;		/* Ignore with no message */

		  case TP_IGNORE:
		    error(IGNORE_FLAG, NULL,
			  NLSMSG(LD_OIGNORED1,
				 "ld: 0706-026 The -b %s option is ignored."),
			  p);
		    break;

		  case TP_SETOPT_COPY:
		    add_list(&Opt_head, &Opt_tail, p, NULL);
		    break;

		  case TP_SETOPT:
		    add_list(&Opt_head, &Opt_tail, *(char **)o->var, NULL);
		    break;

		  case TP_ADDLIST:
		    add_list(((struct lists *)o->var)->head,
			     ((struct lists *)o->var)->tail,
			     flag_arg, flag_arg2);
		    break;

		  case TP_VALUE:
		    switch(o->flags & F_VAL_MASK) {
		      case F_ON:
			*(int *)o->var = 1;
			break;
		      case F_OFF:
			*(int *)o->var = 0;
			break;
		      case F_MINUS1:
			*(int *)o->var = -1;
			break;
		    }
		    break;

		  case TP_STRING:
		    charptr_var = (char **)o->var;
		    if (flag_arg && *flag_arg == '\0') {
			if (o->flags & F_DEFAULT)
			    charptr_var[0] = charptr_var[1];
			else /* Must be F_NULLABLE */
			    charptr_var[0] = NULL;
		    }
		    else
			charptr_var[0] = flag_arg;
		    break;

		  case TP_DECIMAL:
		    int_var = (int *)o->var;

		    if (flag_arg && *flag_arg == '\0') {
			if (o->flags & F_DEFAULT)
			    int_var[0] = int_var[1];
			else /* Must be F_NULLABLE */
			    int_var[0] = 0;
		    }
		    else
			int_var[0] = convert_arg(flag_arg, 10, p);
		    break;

		  case TP_ORIGIN:
		    convert_valid = 1;
		    long_var = (long *)o->var;
		    long_var[0] = convert_arg(flag_arg, 0, p);
		    if (convert_valid == 1) {
			if ((long_var[0] & (Pagesize-1)) != 0)
			    error(SYNTAX_FLAG, NULL,
				  NLSMSG(LD_BADVAL,
	 "ld: 0706-009 The number %1$s is not valid for the -b %2$s option.\n"
	 "\tUse a value that is a multiple of %3$d."),
				  flag_arg, p, Pagesize);
			long_var[1] = 1;
		    }
		    break;

		  case TP_NUMBER:
		    long_var = (long *)o->var;
		    if (flag_arg && *flag_arg == '\0') {
			if (o->flags & F_DEFAULT)
			    long_var[0] = long_var[1];
			else /* Must be F_NULLABLE */
			    long_var[0] = 0;
		    }
		    else
			long_var[0] = convert_arg(flag_arg, 0, p);
		    break;

		  case TP_SPECIAL:
		    switch(o->flags) {
#ifdef STAT_DYNAMIC
		      case SP_SHARED:
			info = new_filearg_info();
			add_list(&Arg_head, &Arg_tail, NULL, info);
			info->flags |= SHARED_FLAG;
			break;

		      case SP_STATIC:
			info = new_filearg_info();
			add_list(&Arg_head, &Arg_tail, NULL, info);
			info->flags |= STATIC_FLAG;
			break;
#endif
		      case SP_NOP:
			if (*flag_arg == '\0')
			    Nop_type = NOP_ORI;
			else if (strcmp(flag_arg, "cror15") == 0)
			    Nop_type = NOP_CROR15;
			else if (strcmp(flag_arg, "cror31") == 0)
			    Nop_type = NOP_CROR31;
			else if (strcmp(flag_arg, "ori") == 0)
			    Nop_type = NOP_ORI;
			else {
			    /* List of machine instructions. */
			    n = strlen(flag_arg);
			    if (n < 8)
				goto nop_invalid;
			    if (flag_arg[0] == '-')
				goto nop_invalid; /* Sign not allowed */
			    else if (flag_arg[0] == '0'
				&& (flag_arg[1] == 'x'
				    || flag_arg[1] == 'X')) {
				n -= 2;
			    }

			    if (n != 8)
				goto nop_invalid;

			    convert_valid = 1;
			    v = convert_arg(flag_arg, 16, "nop");
			    if (convert_valid == 0)
				goto nop_invalid;

			    if (Nop_type != NOP_MANY) {
				/* Any previous list was wiped out. */
				Nop_count = 0;
				if (Nop_list_size > 0) {
				    free(Nop_list);
				    Nop_list_size = 0;
				}
				Nop_type = NOP_MANY;
			    }
			    if (Nop_list_size == 0) {
				Nop_list_size = NOP_LIST_INITIAL_SIZE;
				Nop_list = allocate(Nop_list_size
						    * sizeof(Nop_list[0]));
			    }
			    else if (Nop_count + 1 > Nop_list_size) {
				/* We must enlarge Nop_list */
				Nop_list = reallocate(Nop_list,
						      2 * Nop_list_size
						      * sizeof(Nop_list[0]));
				Nop_list_size *= 2;
			    }
			    Nop_list[Nop_count++] = v;
			}
			break;
		      nop_invalid:
			error(SYNTAX_FLAG, NULL,
			      NLSMSG(LD_BADNOP,
     "ld: 0706-013 The string %s is not valid.  The parameter for\n"
	"\tthe -b nop option must be cror15, cror31, ori, or an unsigned,\n"
	"\t8-digit hexadecimal number, with or without a leading 0x or 0X."),
			      flag_arg);

			break;
		      default:
			error(TERMINAL_FLAG, NULL,
			      NLSMSG(LD_INTERNAL_ERR,
		     "ld: 0706-002 Internal error at line %1$d (v=%2$d).\n"
"\tDepending on where this product was acquired, contact your service\n"
		     "\trepresentative or the approved supplier."),
			      __LINE__, o->flags);
			break;
		    }
		    break;

		  default:
		    error(TERMINAL_FLAG, NULL,
			  NLSMSG(LD_INTERNAL_ERR,
		 "ld: 0706-002 Internal error at line %1$d (v=%2$d).\n"
"\tDepending on where this product was acquired, contact your service\n"
		 "\trepresentative or the approved supplier."),
			  __LINE__, o->type);
		    break;
		}
	    }
	}

	if (!found) {
	    error(SYNTAX_FLAG, NULL,
		  NLSMSG(LD_UNKBOPT,
			 "ld: 0706-014 The -b %s option is not recognized."),
		  p);
	}
    }
} /* parse_bflag */
/************************************************************************
 * Name: set_binder_args
 *									*
 * Purpose: Construct an argument vector used to call the binder.
 *									*
 * Arguments: An empty argument vector.
 *									*
 ************************************************************************/
static void
set_binder_args(char *av[])
{
    char	*e;			/* Error string for perror() */
    char	**p;

    if (Dump_args) {	/* list arguments to binder */
	if ((Dump_fp = fopen(Dump_args, "w")) == NULL) {
	    error(IGNORE_FLAG, "ld:fopen()",
		  NLSMSG(LD_NOBINDARGS,
 "ld: 0706-022 Cannot write the binder command string to file: %s"),
		  Dump_args);
	}
    }

    /* store arguments */
    av[0] = Binder[0];
    av[1] = Quiet ? "quiet" : "noquiet";

    if (Load_map == NULL) {
	/* -bnoloadmap specified */
	av[2] = "noloadmap";
	av[3] = NULL;
    }
    else if (Load_map[0] == '\0') {
	/* Neither -bloadmap:<fn> nor -bnoloadmap specified */
	av[2] = NULL;
    }
    else {
	/* allocate memory */
	av[2] = allocate(strlen("loadmap:") + strlen(Load_map));
	sprintf(av[2], "loadmap:%s", Load_map);
	av[3] = NULL;
    }

    if (Dump_fp) {	/* list arguments to binder */
	p = av;
	if (fprintf(Dump_fp, "%s", *p) < 0) {
	    e = "ld:fprintf()";
	    goto show_error;
	}
	while(*++p) {
	    if (fprintf(Dump_fp, " %s", *p) < 0) {
		e = "ld:fprintf()";
		goto show_error;
	    }
	}

	if (putc('\n', Dump_fp) == EOF) {
	    e = "ld:putc()";
	    goto show_error;
	}
	if (fclose(Dump_fp) < 0) {
	    e = "ld:fclose()";
	  show_error:
	    error(IGNORE_FLAG, e,
		  NLSMSG(LD_NOBINDARGS,
	 "ld: 0706-022 Cannot write the binder command string to file: %s"),
		  Dump_args);
	}
    }
}
/************************************************************************
 * Name: start_binder
 *									*
 * Purpose: Fork the binder and create a pipe to the
 *	binder's standard input.
 *									*
 * Arguments: The argument vector.
 *									*
 * Side Effects:
 *	Out_file is a file pointer for the pipe to the binder's
 *	standard input.
 *
 * Returns:
 *	If any errors occurs, this function does not return.
 ************************************************************************/
static void
start_binder(char *av[])
{
    int			fd[2];
    pid_t		child_pid;
    struct sigaction	act;

    /* open a pipe */
    if (pipe(fd) < 0)
	error(TERMINAL_FLAG, "ld:pipe()",
	      NLSMSG(LD_SYSCALLFAIL,
		     "ld: 0706-021 The %s() system call failed."), "pipe");


    /* to ignore interrupts (^C)--
       binder handles them, so we cannot take default action.  */
    sigaction(SIGINT, NULL, &act);
    act.sa_handler = SIG_IGN;
    act.sa_flags &= ~SA_OLDSTYLE;
    sigaction(SIGINT, &act, NULL);

    /* To catch SIGPIPE, in case binder exits early */
    sigaction(SIGPIPE, NULL, &act);
    act.sa_handler = handle_sigpipe;
    act.sa_flags &= ~SA_OLDSTYLE;
    sigaction(SIGPIPE, &act, NULL);

    if ((child_pid = fork()) == 0) {
	/* child process */
	(void) close(fd[1]);	/* close input to pipe */

	if (fd[0] != 0) {
	    (void) close(0);	/* close stdin */

	    /* make output of pipe the input of this process */
	    if (dup(fd[0]) < 0)
		/* should never happen */
		error(TERMINAL_FLAG, "ld:dup()",
		      NLSMSG(LD_SYSCALLFAIL,
			     "ld: 0706-021 The %s() system call failed."),
		      "dup");

	    (void) close(fd[0]);     /* close "extra" file descriptor */
	}

	(void) execvp(Binder[0], av);	/* run binder */
	error(TERMINAL_FLAG, "ld:execvp()",
	      NLSMSG(LD_NOEXEC,
		     "ld: 0706-025 Cannot call the binder program: %s"),
	      Binder[0]);

	/*NOTREACHED*/
    }
    else if (child_pid == -1) {
	error(TERMINAL_FLAG, "ld:fork()",
	      NLSMSG(LD_SYSCALLFAIL,
		     "ld: 0706-021 The %s() system call failed."),
	      "fork");
    }

    /* Parent */
    (void) close(fd[0]);	/* close output of pipe */

    /* open Out_file so output() goes to the right place */
    if ((Out_file = fdopen(fd[1], "w")) == NULL)
	error(TERMINAL_FLAG, "ld:fdopen()",
	      NLSMSG(LD_SYSCALLFAIL,
		     "ld: 0706-021 The %s() system call failed."),
	      "fdopen");
}
/************************************************************************
 * Name: allocate
 *									*
 * Purpose: Call malloc from a single place, displaying a message
 *	and exiting if memory cannot be obtained.
 *									*
 * Arguments: See malloc()
 *									*
 * Returns:
 *	Pointer to allocated memory.
 ************************************************************************/
static void *
allocate(size_t len)
{
    void *r;

    if ((r = malloc(len)) == NULL)
	error(TERMINAL_FLAG, NULL,
	      NLSMSG(LD_OUTOFMEM1,
  "ld: 0706-007 There is not enough memory available.  Check your ulimit\n"
"\tor paging space or use local problem reporting procedures."));
    return r;
}
/************************************************************************
 * Name: reallocate
 *									*
 * Purpose: Call realloc from a single place, displaying a message
 *	and exiting if memory cannot be obtained.
 *									*
 * Arguments: See realloc()
 *									*
 * Returns:
 *	Pointer to reallocated memory.
 ************************************************************************/
static void *
reallocate(void *ptr,
	   size_t len)
{
    void *r;

    if ((r = realloc(ptr, len)) == NULL)
	error(TERMINAL_FLAG, NULL,
	      NLSMSG(LD_OUTOFMEM1,
  "ld: 0706-007 There is not enough memory available.  Check your ulimit\n"
"\tor paging space or use local problem reporting procedures."));
    return r;
}
