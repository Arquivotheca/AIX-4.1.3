static char sccsid[] = "@(#)66	1.81.2.44  src/bos/usr/ccs/lib/libdbx/object.c, libdbx, bos41J, 9514A_all 4/3/95 14:01:15";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: IS_DBXSTAB, IS_EXT, IS_HIDDEN, IS_STAB, IS_UNRESOLVED,
 *	      PascalLabel, SM_TYPE, adjustlines, allocmaps, changeBlock,
 *	      check_auxent, check_global, check_local, check_var,
 *	      chkUnnamedBlock, curfilename, deffunc, defsym, enterSourceModule,
 *	      enter_nl, enterblock, enterfile, enterline, exitblock,
 *	      expandlntbl, fdata, findsym, foundglobals, getcont, initsyms,
 *	      objfree, objs_differ, openfd, pushBlock, readobj, readsyms,
 *	      setnfiles, updateTextSym, read_loaded_object,
 *	      setup_loaded_object, update_lineaux, update_loaded_object,
 *	      isfcn, enterincl, exitincl, process_linenumbers, ElimLinkage,
 *	      resetUnnamedBlockbnum, cleanup_lineaux, mark_dontskip
 *
 * ORIGINS: 26, 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h"
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * Object code interface, mainly for extraction of symbolic information.
 */

#include "defs.h"
#include "symbols.h"
#include "object.h"
#include "stabstring.h"
#include "main.h"
#include "names.h"
#include "languages.h"
#include "mappings.h"
#include "lists.h"
#include "process.h"
#include "execute.h"
#include "cplusplus.h"
#include <syms.h>
#include <aouthdr.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <linenum.h>
/* #include "aoutdefs.h" */
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
/* ldfcn.h and fcntl.h share defines of FREAD and FWRITE */
#undef FREAD
#undef FWRITE
#include <ldfcn.h>
#include "decode.h"
#include <storclass.h>
#include <dbxstclass.h>

#ifndef N_MOD2
#    define N_MOD2 0x50
#endif
/*
#define IS_STAB(c)	 (((c)->n_type & N_BTMASK) || ((c)->n_numaux) || \
			 ((((c)->n_sclass & N_CLASS) != C_EXT) && \
			  (((c)->n_sclass & N_CLASS) != C_HIDEXT) && \
			  (((c)->n_sclass & N_CLASS) != C_STAT)))
#define DERIVED_TYPE(x)  (((x).n_type >> 4) & 0x03)
*/
#define IS_STAB(c)	 (((c)->n_sclass != C_EXT) && \
			  ((c)->n_sclass != C_STAT) && \
			  ((c)->n_sclass != C_HIDEXT))
#define IS_DBXSTAB(c)	 (((c)->n_sclass >>5) == (DBXMASK >> 5))
#define IS_HIDDEN(c)	 (((c)->n_sclass == C_HIDEXT) || \
				   ((!(c)->n_zeroes) && (!(c)->n_offset)))
#define IS_UNRESOLVED(c) (((c)->n_sclass == C_EXT) && \
			  ((c)->n_scnum == N_UNDEF))
#define IS_EXT(c)	 (((c)->n_sclass == C_EXT) || \
			  ((c)->n_sclass == C_STAT))
#define SM_TYPE(x)	 (((x)->x_csect.x_smtyp & 0x7))
#define objs_differ(a,b) (textobj(a) != textobj(b))

#ifndef C_HIDEXT
#define C_HIDEXT 107
#endif

/* These definitions are used to find "main" routines of various languages */
#define FORT_MAIN ".main"
#define PAS_MAIN ".main"
#define COBOL_MAIN "_mF_main"

extern LDFILE *lddopen();

short stab_compact_level = 0;        /* stabstring compaction level */
public String objname = DEFAULTOBJNM;
public integer objsize;

public Language curlang;
public Symbol curmodule;
public Symbol curparam;
public Symbol curcomm;
public Symbol commchain;
public Address curstat;

public Address text_reloc = 0;
public Address data_reloc = 0;
extern integer loadcnt;		/* Number of loaded entries in process */
private String loadname;	/* Name of loaded portion of code */
public Address minlineaddr = 0xffffffff;
extern boolean noexec;
public boolean nonobject = false;

public cases symcase = mixed;
extern cases casemode;

public Address dbsubn_addr;   /* location of breakpoint following call stub */ 
public Symbol dbsubn_sym;     /* symbol of return address of call */ 
public Address dbargs_addr;   /* location of arg save location */

public int curndx;

public char *stringtab;
public char *debugtab;
private char **stringtab_ary;
extern Symbol *typetable;
public Symbol **typetab_ary;
public int *typenummax_ary;  /* ptr to array of typenummax for each module */
extern int typenummax;	      /* max number of types for current module     */

struct	dmmap	{	/* Keeps track of mapped regions		*/
    char	*start;	/* address of mapped region			*/
    int		count;	/* size of mapped region			*/
    char	*debug;	/* address of debug section			*/
    boolean	mapped;	/* True if the area is mmapped			*/
}   *dmmap_ary;

private boolean warned = false;
extern boolean strip_fortran;
public boolean strip_ = false;
private boolean stripped = false;
private boolean syms_stripped = true;
private boolean defsyms = false;
public boolean unique_fns = false;
public boolean intoexterns = false;
extern Address lastfuncprol, lastfuncaddr;

public boolean isPascal = false;
public boolean isFORTRAN = false;
public boolean isCOBOL = false;
public boolean isASM = false;
public boolean hascobolsrc = false;

#ifdef LAZYTEST
public boolean lazy = true;
#else
public boolean lazy = false;
#endif

public int last_module = -2;		/* last module processed for lazy */
public int bflevel = 0;			/* record nested procedures for lazy */
public boolean process_vars = false;
public boolean filetouched = false;
public boolean fileprocessed = false;
public Symbol touchfile;

private Filetab *filep;
private Linetab *linep;
private Lineaux *lineaxp;
public Filetab *curfilep;
private unsigned int lineaux_length;

extern boolean fullcore;		/* Does corefile contain data? */

private unsigned short lnostart;	/* Adjustment to linenos from .bf */
public unsigned long  ln_index;		/* Index into lineno table for func */

public unsigned long  exceptptr;	/* Except tbl ptr for current func */
public unsigned long linenoptr;

private Boolean foundstab = false;
private Boolean proginit;		/* Program symbol initialized? */
private Boolean newprog;		/* Start of a new program? */

public struct scnhdr  text_scn_hdr;    /* New XCOFF variables */
public struct scnhdr  data_scn_hdr;
public struct scnhdr  bss_scn_hdr;
public struct scnhdr  dbg_scn_hdr;
public struct aouthdr opt_hdr;
public unsigned short  text_scn_num;
public unsigned short  data_scn_num;
public unsigned short  bss_scn_num;
public unsigned short  dbg_scn_num;
public  LDFILE *ld_ptr = NULL;
public  LDFILE **ldentry = NULL;
private FILHDR hdr;
private ARCHDR arhdr;
public int sourcefiles = 0;		/* Number of REAL source files */
public int inclfiles = 0;		/* Number of include files */
private boolean hassource;		/* File compiled -g */
private struct stat file_time;
private int stat_error = 0;
public int nfiles_total = 0;
public int nlines_total = 0;
private int nlaux_total = 0;

private unsigned long linesoffset;
private unsigned long *symtab_position;
Address csectaddr;
Name csectname;				/* Saved csect name entry */
SYMENT *namelist;
Linetab *curlinep;

private int lowersyms = 0;		/* Number of files with folded syms */
private int uppersyms = 0;		/* Num files w/ syms folded upper */
private int continued = 0;

extern Desclist *ElimLinkage();

private boolean check_auxent();
private update_lineaux();
private expandlntbl();
private readsyms();
private initsyms();
private enter_nl();
private check_global();
private check_local();
private check_var();
private fdata();
private adjustlines();
private enterSourceModule();
private allocmaps();
private enterfile();
private enterincl();
private exitincl();
private process_linenumbers();
private setnfiles();
private void mark_dontskip();
private void cleanup_lineaux();
static void mark_entry_sym(String, Boolean);

private boolean islinkage;		/* Global linkage csect? */
private boolean isdefinition;		/* Defined HIDEXT storage */
private boolean isTOCsym;		/* Symbol which is a TOC entry */
private boolean isCMsym;		/* Symbol which is a COMMON entry */
private Boolean isshlib;		/* Processing shared library? */

private Incfile *curincl = nil;		/* Current include file */
private boolean source_file_unprocessed = false;

time_t symtime;

extern Language fLang;
extern Language pascalLang;
extern Language cobLang;
extern Language asmLang;
extern Language cppLang;
extern boolean keep_linkage;

public String curfilename ()
{
    return ((filep-1)->filename);
}

/* Variables to keep track of nested functions and parameters in Pascal */

struct routinestack {
	Symbol name;
	struct routinestack *next, *back;
	};
extern struct routinestack *curroutine;

/* Variables to set up main and program name in Pascal */

public Symbol fake_main = nil;

/*
 * Blocks are figured out on the fly while reading the symbol table.
 */

#define DELETED_SYM 0x00de1e00
#define BAD_SYMBOL_NAME "<<bad name>>"

public Symbol curblock = nil;

public unsigned int nesting_limit = 25;
private Symbol *blkstack;
private integer curlevel;
public integer nesting;
public Address *addrstk;
struct heat_shrink *hs_table;
boolean heat_shrunk = false;

public pushBlock (b)
Symbol b;
{
    if (curlevel >= nesting_limit) {
        fatal( catgets(scmc_catd, MS_object, MSG_117,
		"nesting depth limit (%d) exceeded.\n\
Use the -d option to increase nesting depth limit."), curlevel);
    }
    blkstack[curlevel] = curblock;
    ++curlevel;
    curblock = b;
    if (traceblocks) {
        (*rpt_output)(stdout, "entering block %s\n", symname(b));
    }
}

/*
 * Change the current block with saving the previous one,
 * since it is assumed that the symbol for the current one is to be deleted.
 */

public changeBlock (b)
Symbol b;
{
    curblock = b;
}

public enterblock (b)
Symbol b;
{
    if (curblock == nil) {
	b->level = 1;
    } else {
	b->level = curblock->level + 1;
    }
    b->block = curblock;
    pushBlock(b);
}

public exitblock ()
{
    if (curlevel <= 0) {
        fatal( catgets(scmc_catd, MS_object, MSG_173,
				    "nesting depth underflow (%d)"), curlevel);
    }
    --curlevel;
    if (traceblocks) {
        (*rpt_output)(stdout, "exiting block %s\n", symname(curblock));
    }
    curblock = blkstack[curlevel];
}

/*
 * Enter a source line or file name reference into the appropriate table.
 * Expanded inline to reduce procedure calls.
 *
 * private enterline (linenumber, address)
 * Lineno linenumber;
 * Address address;
 *  ...
 */

#define enterline(linenumber, address) \
{ \
    register Linetab *lp; \
 \
    lp = linep - 1; \
    if (linenumber != lp->line) { \
	if (address != lp->addr) { \
	    ++lp; \
	} \
	lp->line = linenumber; \
	lp->addr = address; \
	linep = lp + 1; \
    } \
}

/* Return a stream pointer from a file descriptor, with LDFILE side-effects. */

public File openfd(fd)
int fd;
{
	struct stat statbuf;	

	if (fstat(fd, &statbuf) == -1)
	    return(nil);
	ld_ptr = lddopen(fd,"r",ld_ptr);
	return (ld_ptr) ? IOPTR(ld_ptr) : nil;
}

/*
 * Read in the namelist from the obj file.
 *
 * Reads and seeks are used instead of fread's and fseek's
 * for efficiency sake; there's a lot of data being read here.
 */

public readobj(file)
String file;
{
    Symbol sym;
    int ldcnt;
    struct stat obj_data;

    nfiles_total = 0;
    nlines_total = 0;
    proginit = false;
    newprog = true;
    foundstab = false;

    ldentry = (LDFILE **) calloc(loadcnt,sizeof(LDFILE *));
    nlhdr = (struct obj_stat *) malloc(loadcnt * sizeof(struct obj_stat));
    filetab = (Filetab **) calloc(loadcnt, sizeof(Filetab *));
    linetab = (Linetab **) calloc(loadcnt, sizeof(Linetab *));
    stringtab_ary = (char **) calloc(loadcnt, sizeof(char *));
    typetab_ary = (Symbol **) calloc(loadcnt, sizeof(Symbol *));
    typenummax_ary = (int *) malloc(loadcnt * sizeof(int));
    dmmap_ary = (struct dmmap *)calloc(loadcnt, sizeof(struct dmmap));
    symtab_position = (unsigned long *) calloc(loadcnt, sizeof(unsigned long));
    hs_table = (struct heat_shrink *)
                 calloc (loadcnt, sizeof(struct heat_shrink));

#ifdef LAZIEST
    if (lazy)
	loadcnt = 1;
#endif
    for (ldcnt = 0; ldcnt < loadcnt; ldcnt++) {
	typenummax_ary[ldcnt] = -1;
	setup_loaded_object(ldcnt, (ldcnt > 0) ? nil : file);
        allocmaps(ldcnt, nlhdr[ldcnt].nfiles, nlhdr[ldcnt].nlines);
    }

    lineaux = newarr(Lineaux, nlines_total);
    nlaux_total = nlines_total;
    lineaxp = lineaux;

    if (nonobject) {
   	noexec_file_map(0,0,0,0,0,0);
	filetab[0][0].filename = file;
	setsource(file);
        defvar(identname("$file",true),nil);
	return;	/* Nothing left to do here. */
    }

    /* Set up the seek addresses for text & data */
    if (coredump) {
	   coredump_set_maps();
    }

    /* Read in the object file information */
    for (ldcnt = 0; ldcnt < loadcnt; ldcnt++) {
	filep = filetab[ldcnt];
	linep = linetab[ldcnt];

        /* Get the time that the object file was last modified */
        fstat(loader_info[ldcnt].ldinfo_fd, &obj_data);
        symtime = obj_data.st_mtime;
        loadname = (ldcnt) ? fd_info[ldcnt].pathname : objname;

	read_loaded_object(ldcnt);
	nlhdr[ldcnt].nfiles = filep - filetab[ldcnt];
	if ((!noexec) && (!coredump) && (!lazy))
	    ldclose(ld_ptr);
    }

    if (not foundstab) 
    {
          warning( catgets(scmc_catd, MS_object, MSG_228,
						"no source compiled with -g"));
    }
    if (!syms_stripped) {
       if (nfiles_total == 0) {
           casemode = mixed;
           symcase = mixed;
       } else if ((lowersyms != 0) && ((sourcefiles / lowersyms) < 2)) 
           symcase = lower;
       else if ((uppersyms != 0) && ((sourcefiles / uppersyms) < 2)) 
           symcase = upper;
    } else {
           casemode = mixed;
           if (!proginit)
		initsyms();
    }
    if (noexec) {
	   noexec_file_map(hdr.f_magic, opt_hdr.text_start, opt_hdr.tsize,
			  opt_hdr.data_start, opt_hdr.dsize, 0);
    }
    ordfunctab();
    ordlineaux(lineaxp-lineaux);
    setnfiles();
    lineaux_length = lineaxp - lineaux;
    find(sym, identname("__dbsubn",true)) where isroutine(sym) endfind(sym);
    if (sym != nil)
    {
        dbsubn_sym = sym;
        dbsubn_addr = prolloc(sym);
    }
    sym = lookup(identname("__dbargs",true));
    if (sym != nil)
    	dbargs_addr = sym->symvalue.offset;
    if ((!noexec) && (!coredump) && (!lazy))
        ldclose(ld_ptr);
#ifdef KDBX
    kdbx_readobj();
#endif
#if defined (CMA_THREAD) || defined (K_THREADS)
    update_lib_type();
#endif /* CMA_THREAD || K_THREADS */
}

/*
 * Gather up information from newly loaded portions of the executable.
 *
 * Also, discard information from unloaded portions of the executable.
 * 
 */

public update_loaded_object(prev_ldcnt, new_ldcnt, old_map, new_map, prev_info)
int prev_ldcnt;		/* Number of previous loaded portions */
int new_ldcnt;		/* Number of currently loaded portions */
int old_map[];		/* Map from old indices to new indices */
int new_map[];		/* Map from new indices to old indices */
struct ldinfo prev_info[]; /* Previous loader information */
{
	int oldndx, ldndx, newndx;

	struct obj_stat *new_nlhdr;
    	struct stat obj_data;
	Filetab **new_filetab;
	Linetab **new_linetab;
	LDFILE **new_ldtab;
	char **new_strtab;
        Symbol **new_typetab;
        int *new_typenummax;
	struct	dmmap	*new_dmmap;
	unsigned long *new_sympostab;
	unsigned int prev_linetotal;

	if ((lazy) && (last_module) && (last_module != -2)) {
	    if (!old_map[last_module]) {
	        last_module = -2;
	    } else {
	        last_module = old_map[last_module];
	    }
	}

	/* Have to discard information from unloaded programs. */
	for (oldndx = 1; oldndx < prev_ldcnt; oldndx++) {
	    if (!old_map[oldndx]) {
		if (nlhdr[oldndx].nlines) {
		    cleanup_lineaux(prev_info[oldndx].textorg,
				    prev_info[oldndx].textsize);
		    dispose(linetab[oldndx]);
		}
		if (nlhdr[oldndx].nfiles) {
		    dispose(filetab[oldndx]);
		}
		/* Still have to get rid of unloaded symbols */
		delete_unloaded_syms(oldndx, prev_info);
		delete_unloaded_funcs(oldndx, prev_info);
	    }
	}

	new_ldtab = (LDFILE **) calloc(loadcnt,sizeof(LDFILE *));
	new_nlhdr = (struct obj_stat *)
				calloc(loadcnt,sizeof(struct obj_stat));
	new_filetab = (Filetab **) calloc(loadcnt,sizeof(Filetab *));
        new_linetab = (Linetab **) calloc(loadcnt,sizeof(Linetab *));
        new_strtab = (char **) calloc(loadcnt, sizeof(char *));
        new_typetab = (Symbol **) calloc(loadcnt, sizeof(Symbol *));
        new_typenummax = (int *) malloc(loadcnt * sizeof(int));
        new_dmmap = (struct dmmap *) calloc(loadcnt, sizeof(struct dmmap));
	new_sympostab = (unsigned long *)
				malloc(loadcnt * sizeof(unsigned long));

	prev_linetotal = nlines_total;
	nlines_total = 0;
	nfiles_total = 0;

	/* Load information on retained programs with new load indices */
	for (oldndx = 0; oldndx < prev_ldcnt; oldndx++) {
	     newndx = old_map[oldndx];     
	     if (newndx || (!oldndx)) {
	         new_nlhdr[newndx] = nlhdr[oldndx];
	         new_filetab[newndx] = filetab[oldndx];
	         new_linetab[newndx] = linetab[oldndx];
	         new_ldtab[newndx] = ldentry[oldndx];
	         new_strtab[newndx] = stringtab_ary[oldndx];
                 new_typetab[newndx] = typetab_ary[oldndx];
                 new_typenummax[newndx] = typenummax_ary[oldndx];
		 new_dmmap[newndx] = dmmap_ary[oldndx];
	         new_sympostab[newndx] = symtab_position[oldndx];

	         nfiles_total += new_nlhdr[newndx].nfiles;
	         nlines_total += new_nlhdr[newndx].nlines;
	     }
	}

	dispose(ldentry);
	dispose(linetab);
	dispose(filetab);
	dispose(nlhdr);
	dispose(stringtab_ary);
        dispose(typetab_ary);
        dispose(typenummax_ary);
	dispose(dmmap_ary);
	dispose(symtab_position);

	nlhdr = new_nlhdr;
	filetab = new_filetab;
	linetab = new_linetab;
	ldentry = new_ldtab;
	stringtab_ary = new_strtab;
        typetab_ary = new_typetab;
        typenummax_ary = new_typenummax;
	dmmap_ary = new_dmmap;
	symtab_position = new_sympostab;

	/* Setup tables to load information from newly loaded portions. */
	for (ldndx = 1; ldndx < new_ldcnt; ldndx++) {
	    if (!new_map[ldndx]) {	/* Unmatched new portions */
	        typenummax_ary[ldndx] = -1;
		setup_loaded_object(ldndx,nil);
        	allocmaps(ldndx, nlhdr[ldndx].nfiles, nlhdr[ldndx].nlines);
	    }
	}

        update_lineaux(lineaux_length);

	/* Read in the object file information for newly loaded portions. */
	for (ldndx = 1; ldndx < new_ldcnt; ldndx++) {
	    if (!new_map[ldndx]) {	/* Unmatched new portions */
	        filep = filetab[ldndx];
	        linep = linetab[ldndx];
	        stringtab = stringtab_ary[ldndx];
                typetable = typetab_ary[ldndx];
                typenummax = typenummax_ary[ldndx];
		debugtab = dmmap_ary[ldndx].debug;

        	/* Get the time that the object file was last modified */
        	fstat(loader_info[ldndx].ldinfo_fd, &obj_data);
        	symtime = obj_data.st_mtime;
        	loadname = fd_info[ldndx].pathname;

	        read_loaded_object(ldndx);
	        nlhdr[ldndx].nfiles = filep - filetab[ldndx];
		if (!lazy)
		    ldclose(ld_ptr);
	    }
	}

	ordfunctab();
        ordlineaux(lineaxp-lineaux);
        lineaux_length = lineaxp-lineaux;
}

/*
 * Accumulate information about a loaded object.
 */
setup_loaded_object(obj_index, file) 
unsigned int obj_index; /* Index of object within loader table. */
String file;
{
    int i = obj_index;
    struct stat filestat;
    SCNHDR tmp_scn_hdr;

    nlhdr[obj_index].stringsize = 0;
    /* if we are processing local vars for lazy read */
    /* all we need is the size of the string table   */
    if (!(lazy && process_vars)) {
       /* Determine size of line number & file sections */
       if ((i != 0) || (file == nil))
           file = fd_info[i].pathname;
       /* Create common object file pointers */
       if ((noexec) || (coredump)) {
            if (!(ldentry[i] = ldopen(file,ldentry[i]))) {
	           fatal(catgets(scmc_catd, MS_object, MSG_219,
						       "cannot open %s"), file);
	    }
	    /* Associate a file descriptor with the file for ease
	       of use later. */
	    loader_info[i].ldinfo_fd = fileno(IOPTR(ldentry[i]));
       } else {
	    if ((fstat(loader_info[i].ldinfo_fd,&filestat) == -1) ||
	        (i != 0) ||
                (!(ldentry[i] =
			  lddopen(loader_info[i].ldinfo_fd,"r",ldentry[i])))) {
	    /* If cannot open by file descriptor, try by name */
                   if (!(ldentry[i] = ldopen(file,ldentry[i]))) {
                         fatal(catgets(scmc_catd, MS_object, MSG_219,
						       "cannot open %s"), file);
		   }
            }
       }
    }
    ld_ptr = ldentry[i];
    if ((fd_info[i].membername) && (fd_info[i].membername[0] != '\0')) { 
       do {
	    if (ldahread(ld_ptr,&arhdr) == FAILURE)
	        fatal(catgets(scmc_catd, MS_object, MSG_167,
		      "cannot open member %s in file %s"),
						  fd_info[i].membername, file);
	    if (streq(arhdr.ar_name,fd_info[i].membername)) {
	          nlhdr[obj_index].stringsize =
                        (unsigned) arhdr.ar_size + ldentry[i]->offset; 
		  break;
	    }
	    if (ldclose(ld_ptr) == SUCCESS)
	          fatal(catgets(scmc_catd, MS_object, MSG_167,
		      "cannot open member %s in file %s"),
						  fd_info[i].membername, file);
       } while (1);
    }
    if (!(lazy && process_vars)) {
       if (ldfhread(ld_ptr, &hdr)) {
	   if (!exec_object(hdr.f_magic)) {
		nonobject = true;
		nlhdr[obj_index].nfiles = 1;
		nlhdr[obj_index].nlines = 0;
	   } else if (ldnshread(ld_ptr, _TEXT, &text_scn_hdr)) {
	        objsize = text_scn_hdr.s_size;             /* size of text */
	        nlhdr[obj_index].nsyms = hdr.f_nsyms;  /* # of symtab entries */
	        nlhdr[obj_index].nfiles = hdr.f_nsyms;	 /* # of files */
	        nlhdr[obj_index].nlines = text_scn_hdr.s_nlnno;/* # of lines */
		if (text_scn_hdr.s_nlnno == 0xffff) { /* Overflow header */
		    int i;
		    /* Have to determine number of text section */
		    for (i=1; ldshread(ld_ptr,i,&tmp_scn_hdr)==SUCCESS; i++) {
			if (streq(tmp_scn_hdr.s_name,_TEXT)) {
			    text_scn_num = i;
			    break;
			}
		    }
		    /* Find the text overflow section, nlines is in s_vaddr */
		    for (i=1; ldshread(ld_ptr,i,&tmp_scn_hdr)==SUCCESS; i++) {
			if ((tmp_scn_hdr.s_flags & 0xffff) == STYP_OVRFLO &&
				(tmp_scn_hdr.s_nlnno == text_scn_num)) {
			    nlhdr[obj_index].nlines = tmp_scn_hdr.s_vaddr;
			    text_scn_hdr.s_nlnno = tmp_scn_hdr.s_vaddr;
			    break;
			}
		    }
		}
	   }
       }
    }
}

/* 
 * Read in the information associated with a loaded object.
 * An object may be an executable, loadable, or a shared library.
 */

read_loaded_object(ldcnt) 
unsigned int ldcnt; 	/* Index of object within loader table. */
{
    int i;
    SCNHDR temp_scn_hdr;
    unsigned int file_end;
    unsigned int currentpos;

    /* if doing lazy read, we want to cut down our memory usage */
    /* So dispose string table and debug table of last module   */
    if (lazy) {
	dispose(stringtab);
	for (i=0; i<loadcnt; i++)
	{
	    if (dmmap_ary[i].mapped == false && dmmap_ary[i].debug != NULL)
		dispose(dmmap_ary[i].debug);
	}
    }
    text_scn_num = data_scn_num = dbg_scn_num = bss_scn_num = 0;
    ld_ptr = ldentry[ldcnt];
    /*
     * Determine section numbers for text, data and bss sections.
     */
    ldfhread(ld_ptr, &hdr);
    ldnshread(ld_ptr, _TEXT, &text_scn_hdr);
    for (i = 1; i <= hdr.f_nscns; i++)
    {
	ldshread(ld_ptr, (unsigned short)i, &temp_scn_hdr);
	if streq(temp_scn_hdr.s_name, _TEXT)
	    text_scn_num = i;
	else if streq(temp_scn_hdr.s_name, _DATA) {
	    data_scn_num = i;
	    memcpy(&data_scn_hdr, &temp_scn_hdr, SCNHSZ);
	} else if streq(temp_scn_hdr.s_name, ".debug") {
	    dbg_scn_num = i;
	    memcpy(&dbg_scn_hdr, &temp_scn_hdr, SCNHSZ);
	    if (dmmap_ary[ldcnt].debug == nil)
	    {
		long	pagesize;	/* Page size for the system	*/
		long	pageoffset;	/* next lowest page boundary	*/
		long	fudge;		/* (start of .debug) - page	*/

		/*
		 * Find the start of the .debug section.  The .s_scnptr
		 * address is not necessarily what we want, so we need to
		 * use the ldr lib stuff to find it.
		 */
		ldsseek(ld_ptr, dbg_scn_num);
		currentpos = FTELL(ld_ptr);

		pagesize = sysconf(_SC_PAGE_SIZE);

		/*
		 * The next line looks pointless, but really does truncation.
		 * Mmap only works on page boundaries, this finds the
		 * page boundary for us.
		 */
		pageoffset = pagesize * (currentpos / pagesize);
		fudge = currentpos - pageoffset;

		/*
		 * Here we attempt to map the file.
		 * Note that we could do this with lots of the sections,
		 * including the string table and line number table.
		 */
		dmmap_ary[ldcnt].start = mmap(NULL,dbg_scn_hdr.s_size+fudge,
			PROT_READ|PROT_WRITE,
			MAP_FILE|MAP_VARIABLE|MAP_PRIVATE,
			fileno(IOPTR(ld_ptr)), pageoffset);
		if (dmmap_ary[ldcnt].start == -1)
		{
		    /*
		     * If the mmap call fails, then try to allocate
		     * space to read in the buffer.
		     */
		    dmmap_ary[ldcnt].start = nil;
		    dmmap_ary[ldcnt].count = 0;
		    ldsseek(ld_ptr, dbg_scn_num);
		    dmmap_ary[ldcnt].debug = newarr(char, dbg_scn_hdr.s_size);
		    FREAD((void *)dmmap_ary[ldcnt].debug, (size_t) 1,
			  (size_t) dbg_scn_hdr.s_size,ld_ptr);
		    dmmap_ary[ldcnt].mapped = false;
		}
		else
		{
		    /*
		     * This is the address in our memory space for the newly
		     * mapped .debug section.
		     */
		    dmmap_ary[ldcnt].debug = dmmap_ary[ldcnt].start + fudge;
		    /*
		     * This is needed for the call to munmap.
		     */
		    dmmap_ary[ldcnt].count = dbg_scn_hdr.s_size+fudge;
		    dmmap_ary[ldcnt].mapped = true;
		}
	    }
	    debugtab = dmmap_ary[ldcnt].debug;
	} else if streq(temp_scn_hdr.s_name, _BSS)
	    bss_scn_num = i;
        else if streq(temp_scn_hdr.s_name, "mjsxref")
        {
          
          /************************************************************

          Example of part of a heat-shrink xref section :

          Section header #8: name: mjsxref
           Physical addr:      00000000
           Virtual addr:       00000000
           Section size:       00005410
           FilePtr2Raw:        00064E4A  ---+
           FilePtr2Reloc:      00000000     |
           FilePtr2LineNo:     00000000     |
           NumRelEntries:          0000     |
           NumLineNoEntries:       0000     |
           Flags:              00000000     |
                                            V

          00064E4A: 00 00 0A 81  FF FF FF FF  00 00 02 00  00 04 BC 88
          00064E5A: 00 00 02 78  00 04 BC B8  00 00 02 BC  00 04 BC FC
          00064E6A: 00 00 02 F0  00 04 BD 30  00 00 02 FC  00 04 BD 3C

          The first word, 0xA81, is the number of XREF entries
          (including the first 8 byte entry) followed by (int)-1.
          Next are the xref pairs: 0x200-0x4BC88, 0x278-0x4BCB8, ...; 
          0x200 is the beginning of the basic block in the original
          text section and 0x4BC88 is the "new" text area where that
          block has been moved.  The size of the new text basic block
          is the range to the next text address.

          ************************************************************/

          int rc;

          heat_shrunk = true;

          hs_table[ldcnt].map
             = (struct reloc_table *) newarr(char,temp_scn_hdr.s_size);

          rc = fseek(IOPTR(ld_ptr), temp_scn_hdr.s_scnptr, 0);
          FREAD((void *)hs_table[ldcnt].map, (size_t) 1,
                (size_t) temp_scn_hdr.s_size,ld_ptr);
          hs_table[ldcnt].num_elements
             = hs_table[ldcnt].map[0].old_addr - 1;
        }
    }

    /* Get the aouthdr information from the object file. */
    if (ldohseek(ld_ptr) != SUCCESS) {
	opt_hdr.magic = 0;
	opt_hdr.data_start = 0;
	opt_hdr.text_start = text_scn_hdr.s_scnptr;
    } else {
   	if (!(FREAD((void *)(&opt_hdr), (size_t) sizeof(opt_hdr),
							(size_t) 1, ld_ptr))) {
   	    opt_hdr.data_start = 0;
   	    opt_hdr.text_start = text_scn_hdr.s_scnptr;
         }
    }
#ifdef KDBX
    if (ldcnt == 0) {
	text_reloc = 0;
	data_reloc = 0;
    } else
#endif
    {
	text_reloc = loader_info[ldcnt].textorg -
				    opt_hdr.text_start + text_scn_hdr.s_scnptr;
	data_reloc = loader_info[ldcnt].dataorg - opt_hdr.data_start;
    }
    /* If there is no symbols during process  */
    /* locals of lazy read, we are done here. */ 
    if ((lazy && process_vars) && (hdr.f_nsyms <= 0))   
	return;
    if (coredump && !(lazy && process_vars)) {
	coredump_reloc(ldcnt, text_scn_hdr.s_scnptr);
    }
    if (hdr.f_nsyms > 0) {
	/*
	 * Read string table information.
	 */
	 if (syms_stripped)
	     syms_stripped = false;
	 FSEEK(ld_ptr, hdr.f_symptr + (hdr.f_nsyms * SYMESZ), 0);
	 file_end = (nlhdr[ldcnt].stringsize) ? nlhdr[ldcnt].stringsize : 
						(unsigned) -1;
	 currentpos = FTELL(ld_ptr);
	 if (currentpos >= file_end) {	/* No string table */
	    nlhdr[ldcnt].stringsize = 0;
	 } else {
	     FREAD((void *)(&nlhdr[ldcnt].stringsize),
		(size_t) sizeof(nlhdr[ldcnt].stringsize), (size_t) 1, ld_ptr);
	 }
	 if (nlhdr[ldcnt].stringsize != 0) {
	     nlhdr[ldcnt].stringsize -= 4;
	     stringtab_ary[ldcnt] = newarr(char, nlhdr[ldcnt].stringsize);
	     stringtab = stringtab_ary[ldcnt];

	     /* stringtab can still be nil if the size requested is */
	     /* too much and we don't have enough memory...         */
	     if (stringtab)
	        FREAD((void *)stringtab, (size_t) 1,
				     (size_t) nlhdr[ldcnt].stringsize, ld_ptr);
	 } else {
	     stringtab = nil;
	 }
	 if (nlhdr[ldcnt].nlines != 0) {
	     ldnlseek(ld_ptr, _TEXT);
	     linesoffset = FTELL(ld_ptr) - OFFSET(ld_ptr);
	     if (!(lazy && process_vars)) {
	        FREAD((void *)linetab[ldcnt], (size_t) LINESZ,
					(size_t) nlhdr[ldcnt].nlines, ld_ptr);
	        expandlntbl(curlinep = linetab[ldcnt], nlhdr[ldcnt].nlines);
	     }
	 } else {
	     linesoffset = 0;
	 }
	 ldtbseek(ld_ptr);
	 symtab_position[ldcnt] = FTELL(ld_ptr);
    	 isshlib = (boolean) ((fd_info[ldcnt].membername) &&
				       (fd_info[ldcnt].membername[0] != '\0'));
         /* set up typetable to use before reading symbol/type info */
         typetable = typetab_ary[ldcnt];
         typenummax = typenummax_ary[ldcnt];
         if (lazy && process_vars)
             return;
	 readsyms(ldcnt);
         /* save allocated typetable and new typenummax for reuse */
         typetab_ary[ldcnt] = typetable;
         typenummax_ary[ldcnt] = typenummax;
    }

    for (i = 0; i < nlhdr[ldcnt].nlines; i++) /* Remove fake line number */
         if (linetab[ldcnt][i].line == 0)
             linetab[ldcnt][i].line = linetab[ldcnt][i+1].line;

    /* Set the number of actual files in the object. */
    nlhdr[ldcnt].nfiles = filep - filetab[ldcnt];
    nfiles_total += nlhdr[ldcnt].nfiles;
}

/*
 * Read the local information from a file under the
 * lazy reading mode of operation.
 */
public touch_file (s)
Symbol s;
{
    int current_module;
    Symbol file_sym;
    SYMENT np;

    /* Determine the file associated with this symbol. */
    if (s->class != MODULE) {
	for (file_sym = s; (file_sym != nil) && (file_sym->class != MODULE);
		file_sym = file_sym->block);
	if (file_sym == nil)
		return;
    } else {
	file_sym = s;
    }
    touchfile = file_sym;
    /* If the symbols have already been read in for this file,
       do not do it again. */
    if (!file_sym->symvalue.funcv.untouched)
	return;
    /* Determine which module this file belongs in */
    current_module = addrtoobj(file_sym->symvalue.funcv.beginaddr);
    if (current_module == -1)	/* Not in any we know about. */
	return;
    process_vars = true;	/* we are reading locals now */
    ld_ptr = ldentry[current_module];
    if (last_module != current_module) {
	last_module = current_module;
	/* if we are dealing with a different module, go */
	/* reread the string table and debug table.      */
	setup_loaded_object(current_module, nil);
	read_loaded_object(current_module);
    }
    FSEEK(ld_ptr, (long int) file_sym->symvalue.funcv.fcn_desc, 0);
    file_sym->symvalue.funcv.untouched = (unsigned) false;
    /* Reset the stage for a clean start */
    if (curblock->class != PROG) {
	exitblock();
	while (curblock->class != PROG) {
	    exitblock();
	}
    }
    curmodule = program;
    /* Read 'em in, Dano... */
    filetouched = false;
    readsyms(current_module);
    /* sort functab again in case local entries got turned into */
    /* functions by check_global (updateTextSym).	   	*/
    ordfunctab();
    /* save allocated typetable and new typenummax for reuse */
    typetab_ary[current_module] = typetable;
    typenummax_ary[current_module] = typenummax;
    process_vars = false;
}

/*
 * NAME: cleanup_lineaux
 *
 * FUNCTION: Remove any references to unloaded functions from the
 *	lineaux table.
 *
 * EXECUTION ENVIRONMENT:
 *	Normal user process
 *
 * NOTES: This should be called after a routine calls unload, or
 *	reruns.  This routine assumes that there can be no valid
 *	line entries for address 0x00000000.
 *
 * RECOVERY OPERATION: none
 *
 * DATA STRUCTURES:
 *	lineaux		The auxilary line number table
 *	lineaux_length	The number of entries in the lineaux table
 *	lineaxp		Pointer to the next free entry in the lineaux table
 *
 * PARAMETERS:
 *	startaddr	Starting address of the unloaded segment
 *	size		length of the unloaded segment
 *
 * TESTING: The line number table should be correct and not contain
 *	any extraneous entries after a segment is unloaded.
 *
 * RETURNS: void
 *
 */
private void cleanup_lineaux(startaddr, size)
Address		startaddr;
unsigned	int	size;
{
    Address	end = startaddr + size;
    int		i, j, new_laux_used = lineaux_length;
   
    /*
     * Mark all deleted line aux entries nil
     */
    for (i=0; i<lineaux_length; i++)
    {
	if (lineaux[i].lp->addr >= startaddr && lineaux[i].lp->addr <= end)
	{
	    lineaux[i].addr = 0;
	    new_laux_used--;
	}
    }
    
    /*
     * Get rid of the nil entries
     */
    i=0; 
    while (i<new_laux_used)
    {
	if (lineaux[i].addr == 0)
	{
	    j=i;
	    for (j=i; lineaux[j].addr == 0 && j<lineaux_length; j++)
		;
	    if (!j<lineaux_length)
		break;
	    lineaux[i] = lineaux[i+j];
	    lineaux[i+j].addr = 0;
	}
	i++;
    }
    lineaux_length = new_laux_used;
    lineaxp = &lineaux[lineaux_length];
}

/*
 * Update the auxiliary line number table after a load/unload.
 */

private update_lineaux(laux_used, old_length)
unsigned int laux_used;	/* Line chunks actually used. */
{
    if (nlaux_total > laux_used + nlines_total) {
	return;
    } else {
	lineaux = (Lineaux *) realloc(lineaux,nlines_total * sizeof(Lineaux));
	lineaxp = &lineaux[laux_used];
	nlaux_total = nlines_total;
    }
}

/*
 * Found the beginning of the externals in the object file
 * (signified by the first external symbol); close the
 * block for the last procedure.
 */

private foundglobals ()
{
    if (curblock->class != PROG) {
	exitblock();
	while (curblock->class != PROG) {
	    exitblock();
	}
    }
    intoexterns = true;
    curmodule = program;
}

/*
 * Expand the line number table so that we have long line numbers.
 */

private expandlntbl (linetable, nl)
Linetab *linetable;
int nl;
{
   struct oldlnno {
	unsigned short addr1;
	unsigned short addr2;
	unsigned short ln;
   } *oldntry;
   Linetab *newntry;
   int i;

   newntry = linetable;
   oldntry = (struct oldlnno *) linetable;

   for (i = nl-1; i >= 0; i--) {
	newntry[i].line = oldntry[i].ln;
	if (!newntry[i].line)
		newntry[i].addr = newntry[i+1].addr;
	else
		newntry[i].addr = (oldntry[i].addr1 << 16) | oldntry[i].addr2;
   }
   newntry[nl].line = newntry[nl].addr = 0;
}

/*
 * Read in symbols from object file.
 */

private readsyms (ldcnt)
int ldcnt;
{
    SYMENT *np;
    AUXENT *axp;
    unsigned int extstart = 0xffffffff;
    Boolean instrtab = false;
    char sym_name[50];
    register String name;
    char *tmpname;
    register int i;
    char *lastchar;
    unsigned long ndx = 1;
    boolean readhidden;
    boolean isfcn();

    if (!proginit)
	initsyms();
    name = sym_name;
    intoexterns = false;
    np = (SYMENT *) malloc(SYMESZ);
    axp = (AUXENT *) malloc(sizeof(AUXENT));
    if (lazy && process_vars) {
	fileprocessed = false;
	ndx = ((FTELL(ld_ptr) - symtab_position[ldcnt]) / SYMESZ) + 1;
    } else {
        ldtbseek(ld_ptr);
    }
    while ((ndx <= nlhdr[ldcnt].nsyms) && 
           (!(lazy && process_vars && fileprocessed))) {
	stripped = false;
	instrtab = false;
	readhidden = false;
	isdefinition = false;
	isTOCsym = false;
	isCMsym = false;
	continued = 0;
	FREAD((void *)np, (size_t) SYMESZ, (size_t) 1, ld_ptr);
	if (IS_UNRESOLVED(np)) {
	    if (np->n_numaux) {
	        ndx += np->n_numaux;
	        FSEEK(ld_ptr, (long int) (np->n_numaux * SYMESZ), 1);
	    }
	    ndx++;
#ifndef KDBX
	    continue;
#else
	    /* (1)
	     * Falltrough, for /unix to get the imported
	     * kernel adddresses right (u, proc, errno, etc.)
	     * This fixes what the ifdef below (**) brakes,
	     * see also ifdef at end (3).
	     */
	    if (ldcnt != 0)
		continue;
#endif
	} else if (IS_HIDDEN(np)) {
	    if (np->n_numaux) {
	        ndx += np->n_numaux;
		if (np->n_numaux > 1)
	            FSEEK(ld_ptr, (long int) ((np->n_numaux - 1) * SYMESZ), 1);
	        FREAD((void *) axp, (size_t) SYMESZ, (size_t) 1, ld_ptr);
		isdefinition = (boolean) ((SM_TYPE(axp) == XTY_SD)
						 || (SM_TYPE(axp) == XTY_CM));
		isTOCsym = (boolean) ((!isshlib) && isdefinition &&
					(axp->x_csect.x_smclas == XMC_TC));
#ifdef KDBX
		/* (2)
		 * This prevents lots of ptraces to get the TOC value
		 * from the kernel, (only for /unix).
		 * With this code in "print &u" does not work ...
		 * (see ifdef (1) above).
		 */
		if (ldcnt == 0)
			isTOCsym = false;
#endif
		if (isdefinition) {
		    csectaddr = np->n_value;
		    if (np->n_scnum == text_scn_num)
			csectaddr += text_reloc;
		    else if ((np->n_scnum == data_scn_num) ||
		    			(np->n_scnum == bss_scn_num))
			csectaddr += data_reloc;
		}
		
		readhidden = (boolean) ((SM_TYPE(axp) == XTY_LD) ||
			  		(axp->x_csect.x_smclas == XMC_RO) ||
			  		(axp->x_csect.x_smclas == XMC_RW) ||
			  		(axp->x_csect.x_smclas == XMC_BS) ||
		       			(isTOCsym));
	    }
	    if (readhidden) {
		ndx -= np->n_numaux;
	        FSEEK(ld_ptr,(long int)(-(np->n_numaux * SYMESZ)), 1);
	    } else {
	        ndx++;
	        continue;
	    }
	}    
	i = np->n_zeroes;
	if (i != 0) {
	    name = (String) np;
	    if (strlen(name) >= 8) {
	 	tmpname = (String) malloc(9);
		tmpname[8] = '\0';
	 	strncpy(tmpname,name,8);
	 	name = tmpname;
	    }
	    if (IS_DBXSTAB(np)) {
	        if (!foundstab)
	           foundstab = true;
	    }
	} else {
	    if (IS_DBXSTAB(np)) {
	       name = (String) &debugtab[np->n_offset];
	       /* we disposed string table for lazy read, so */
	       /* cannot use them from the string table now. */
	       if (!lazy)
	           instrtab = true;
	       if (!foundstab)
	           foundstab = true;
	    } else {
		if ((isdefinition) && (np->n_offset == 0)) {
		    isdefinition = false;
		}
                if (!stringtab)
                  name = nil;
                else
	          name = (String) &stringtab[np->n_offset - 4];
	        if (!lazy)
	            instrtab = true;
	    } 
	}
#ifdef KDBX
	/*
	 * There is a "ld" command bug when the "noreorder" flag is used.
	 * Some text symbols end up having data section numbers instead
	 * of text section numbers, because of that they are not put in
	 * the function table.
	 * This only happens when there is no debugging information for
	 * those symbols. "/unix" is the only object binded with noreorder.
	 * This code tries to fix this until the binder is fixed.
	 *	ldcnt == 0	==>> /unix
	 *	*name == '.'	==>> function
	 */
        if (ldcnt == 0 && np->n_scnum == data_scn_num && *name == '.')
            np->n_scnum = text_scn_num;
	/*
         * There is an other effect of the "noreorder" flag :
         * Some data symbols are changed from data to text by ld.
	 * We change them back to data in case of RW symbol class.
         *      ldcnt == 0      ==>> /unix
         *      .text && RW     ==>> .data
         */

        if (ldcnt == 0 && np->n_scnum == text_scn_num && *name != '.'){
	    if (!IS_HIDDEN(np)) {
		if (np->n_numaux) {
		    ndx += np->n_numaux;
		    if (np->n_numaux > 1) {
			FSEEK(ld_ptr, (long)((np->n_numaux - 1) * SYMESZ), 1);
		    }
		    FREAD((void *) axp, (size_t) SYMESZ, (size_t) 1, ld_ptr);
		    ndx -= np->n_numaux;
		    FSEEK(ld_ptr,(long )(-(np->n_numaux * SYMESZ)), 1);
		}
	    }
	    if (axp->x_csect.x_smclas == XMC_RW)
		np->n_scnum = data_scn_num;
	}
#endif
	/*
         *  If the program contains any .f files a trailing _ is stripped
         *  from the name on the assumption it was added by the compiler.
         *  This only affects names that follow the sdb N_SO entry with
         */
        if (strip_ and name[0] != '\0' ) {
	    lastchar = &name[strlen(name) - 1];
	    if (*lastchar == '_') {
	        *lastchar = '\0';
	        stripped = true;
	   }
         }

#ifdef KDBX
	/* (3) this completes the fix started at (1) above because of (2). */
	if (IS_UNRESOLVED(np)) {
	    check_global(name, np, instrtab);
	    continue;
	}
#endif
	/* If this is debug information, record it as such. */
	if (IS_STAB(np) || (isfcn(np))) {
	  enter_nl(name, ndx, np, axp, &extstart, instrtab, ldcnt);
	} else if ((intoexterns) || (np->n_sclass == C_HIDEXT) ||
		      (np->n_sclass == C_EXT) || (np->n_sclass == C_STAT)) {
		  if ((!intoexterns) && (ndx >= extstart))
			foundglobals();
		  if ((!(check_auxent(np, &ndx, axp)))
			&&(!(lazy&&(process_vars&&np->n_sclass!=C_HIDEXT))))
		         check_global(name, np, instrtab);
		   ndx += (continued + 1);
		   continue;
	} else {
		if ((!(check_auxent(np, &ndx, axp))) && (!(lazy&&process_vars)))
		      check_local(name, np, instrtab);
	        ndx += (continued + 1);   /* Skip continued stabs */
		continue;
	}
	check_auxent(np, &ndx, axp);
	ndx += (continued + 1);   /* Skip continued stabs */
    }
    if (source_file_unprocessed) {  /* Possibly one file left unprocessed */
	process_linenumbers();
    }
    mark_dontskip();
    free( np );
    free( axp );
}

/*
 * Get a continuation entry from the name list.
 * Return the beginning of the name.
 */

public String getcont ()
{
    register integer nameindex;
    register String name, tmpname;
    SYMENT *contsym;

    
    contsym = (SYMENT *) malloc(SYMESZ);
    continued++;
    FREAD((void *)contsym, (size_t) SYMESZ, (size_t) 1, ld_ptr);
    nameindex = contsym->n_zeroes;
    if (nameindex == 0) {
	name = (String) &debugtab[contsym->n_offset];
    } else {
	name = (String) contsym;
	if (strlen(name) > 8) {
		tmpname = (String) malloc(9);
		tmpname[8] = '\0';
		strncpy(tmpname,name,8);
		name = tmpname;
	}
    }
    return name;
}

/*
 * Initialize symbol information.
 */

private initsyms ()
{
    curblock = nil;
    curlevel = 0;
    nesting = 0;
    blkstack = (Symbol *) calloc(nesting_limit, sizeof(Symbol));
    addrstk = (Address *) calloc(nesting_limit, sizeof(Address));
    program = insert(identname("", true));
    program->class = PROG;
    program->type = t_int;
    program->language = primlang;
    if ((coredump) || (attach) || (runfirst))
        program->symvalue.funcv.beginaddr = CODESTART;
    else
        program->symvalue.funcv.beginaddr = reg(SYSREGNO(PROGCTR));
    program->symvalue.funcv.inline = false;
    program->symvalue.funcv.isentry = false;
    program->symvalue.funcv.islinkage = false;
    program->symvalue.funcv.fcn_desc = nil;
    program->symvalue.funcv.dontskip = false;
    program->symvalue.funcv.proladdr = program->symvalue.funcv.beginaddr;
    newfunc(program, codeloc(program), 0);
    findbeginning(program);
    enterblock(program);
    curmodule = program;
    proginit = true;
}

/*
 * Free all the object file information that's being stored.
 */

public objfree ()
{
    symbol_free();
    /* keywords_free(); */
    /* names_free(); */
    /* dispose(stringtab); */
    clrfunctab();
    syms_stripped = true;
}

/*
 * Enter a namelist entry.
 */

private enter_nl (name, ndx, np, axp, extstart, instrtab, ldcnt)
String name;
register int ndx;
SYMENT *np;
AUXENT *axp;
unsigned int *extstart;
boolean instrtab;
int ldcnt;
{
    register Name n;
    int class;
    unsigned long currentnp;
    SYMENT tmpsym;
    char *tmpname;

    class = np->n_sclass;
    if ((!intoexterns) && (ndx >= *extstart) &&
				((class == C_EXT) || (class == C_STAT))) {
	  foundglobals();
    }
    /* Read in the auxiliary entry here, but don't move the file pointer.
       This is done this way just for now...probably. */
    if (np->n_numaux) {
	FREAD((void *) axp, (size_t) SYMESZ, (size_t) 1, ld_ptr);
	FSEEK(ld_ptr, (long int) -SYMESZ, 1);
    }

    switch (class) {
	/*
	 * Build a symbol for the FORTRAN common area.  All GSYMS that follow
	 * will be chained in a list with the head kept in common.offset, and
	 * the tail in common.chain.
	 */
	case C_BCOMM:
	    {
	    Symbol t;
	    if (lazy && !process_vars) 
		break;
 	    if (curcomm) {
		curcomm->symvalue.common.chain = commchain;
	    }
	    if (stripped)
		name[strlen(name)] = '_';
	    n = identname(name, instrtab);
	    find(t, n) where
		(t->storage == EXT) && (t->class == COMMON)
	    endfind(t);
	    curcomm = t;
	    if (curcomm == nil) {
		curcomm = insert(n);
		curcomm->class = COMMON;
		curcomm->block = curblock;
		curcomm->level = program->level;
		curcomm->storage = EXT;
		curcomm->symvalue.common.chain = nil;
		    if(!(strip_fortran)) {
			name[strlen(name)] = '_';
		    	n = identname(name, instrtab);
		    }
		    find(t, n) where
			(t->storage == EXT) && (t->class == VAR)
	    	    endfind(t);
		if (t == nil)
			curcomm->symvalue.common.com_addr = 0;
		else
			curcomm->symvalue.common.com_addr = 
			          t->symvalue.offset;
	    /* 
	    To handle cases where unamex & extern apppear before bcomm 
	    and ecomm entries. xlf once produced in this order...
	    Removed because the unamex entry can actually belong to
	    a variable of the same name as the COMMON and not the
	    unamex of the COMMON, and there's no way to tell the difference
	    } else if ((curcomm->class == VAR) || (curcomm->class == TOCVAR)) {
		curcomm->class = COMMON;
		curcomm->block = curblock;
		curcomm->level = program->level;
		curcomm->storage = EXT;
		curcomm->symvalue.common.com_addr = curcomm->symvalue.offset;
		curcomm->symvalue.common.chain = nil;
	    */
	    }
	    commchain = curcomm->symvalue.common.chain;
	    break;
	    }

	case C_ECOMM:
	    if (lazy && !process_vars) 
		break;
	    if (curcomm) {
		curcomm->symvalue.common.chain = commchain;
		curcomm = nil;
	    }
	    break;

	case C_BSTAT:
	    if (lazy && !process_vars)
	       break;
	    currentnp = FTELL(ld_ptr) - OFFSET(ld_ptr);
	    ldtbread(ld_ptr, np->n_value, &tmpsym);
	    curstat = tmpsym.n_value;
	    /* Need to add in relocation from section tmpsym belongs in. */
	    if (tmpsym.n_scnum == text_scn_num)
		curstat += text_reloc;
	    else if ((tmpsym.n_scnum == data_scn_num) ||
	    			(tmpsym.n_scnum == bss_scn_num))
		curstat += data_reloc;
	    FSEEK(ld_ptr, (long int) currentnp, 0);
	    break;

	case C_ESTAT:
	    if (lazy && !process_vars)
	       break;
	    curstat = 0;
	    break;

	case C_BLOCK:
	    {
    	    Address blockaddr = np->n_value + text_reloc;
	    if (streq(name,".bb")) {
		/* Check if we have a .bb following right after */
		/* a .bb, which happens if we have only static  */
		/* variables within an unamed block.	        */
    	        if (nesting > 0 && addrstk[nesting] != NOADDR)
                  chkUnnamedBlock();
	    	if (defsyms) {
			++nesting;
			addrstk[nesting] = blockaddr;
	    	}
	    }
	    else /* Has to be a .eb if not a .bb ...
	    if (streq(name,".eb"))
		*/
	    	if (nesting > 0) {
			if (lazy && !process_vars) {
			    chkUnnamedBlock();  /* Have to force a $b* */
			}
			if (addrstk[nesting--] == NOADDR) {
				curblock->symvalue.funcv.src = true;
		    		exitblock();
				if (!(lazy && process_vars))
		    		    newfunc(curblock, blockaddr, curfilep);
			}
	    	}
	    }
	    break;

	/*
	 * Source files.
	 */
	case C_FILE:
	    if (lazy && !process_vars) {
		FSEEK(ld_ptr, (long int) -SYMESZ, 1);
	        np->n_value = FTELL(ld_ptr) - OFFSET(ld_ptr);
		FSEEK(ld_ptr, (long int) SYMESZ, 1);
	    }
	    if (np->n_numaux) {	/* Name is flexnamed into aux entry */
	        if (axp->x_file._x.x_zeroes != 0) {
	            name = (String) axp->x_file.x_fname;
	    	    if (strlen(name) > FILNMLEN) {
	 	        tmpname = (String) malloc(FILNMLEN+1);
			tmpname[FILNMLEN] = '\0';
	 		strncpy(tmpname,name,FILNMLEN);
	 		name = tmpname;
	    	    }
		} else {
                    if (!stringtab)
                      name = nil;
                    else
	              name = (String) &stringtab[axp->x_file._x.x_offset - 4];
		}
	    }
	    if (lazy && process_vars && filetouched) {
		String mname, p;
		mname = strdup(name);
		p = index(mname, '.');
		if (p) 
		    *p = '\0';
		/* fortran compiler produces more than one file entries     */
		/* for each file, so make sure we are into a different file */
	        if (strcmp(mname,symname(touchfile))) {
		    /* done if the filename are not the same */
		    fileprocessed = true;
		    return;
		} else {
		    /* else are their languages the same also? */
		    Language filelang;
		    if (p)
			*p = '.';
        	    filelang = findlanguage(p);
		    if (filelang != touchfile->language) {
		        fileprocessed = true;
			return;
		    }
		}	
	    }
	    if (intoexterns)
	       intoexterns = false;
	    *extstart = np->n_value;
	    n = identname(name, false);
	    if (lazy && process_vars) {
                enterSourceModule(n, (Address) 0, np->n_type, 
				                  np->n_name, ldcnt);
		filetouched = true;
		fileprocessed = false;
	    } else {
                enterSourceModule(n, (Address) np->n_value,
                                               np->n_type, np->n_name, ldcnt);
	    }
	    break;

	/*
	 * Textually included files.
	 */
/*
	case N_SOL:
	    enterfile(name, (Address) np->n_value);
	    break;
*/

	/*
	 * These symbols are assumed to have non-nil names.
	 */
	case C_LABEL:
	case C_EXT:
	case C_HIDEXT:
	case C_FUN:
	case C_DECL:
	case C_ECOML:
	case C_GSYM:
	case C_LSYM:
	case C_PSYM:
	case C_RSYM:
	case C_RPSYM:
	case C_STSYM:
	    defsyms = true;
		
	    if (isfcn(np)) {
		linenoptr = axp->x_sym.x_fcnary.x_fcn.x_lnnoptr;
		exceptptr = axp->x_sym.x_tagndx;
		if (linenoptr == 0) {
		    ln_index = 0;
		} else
	            ln_index = (linenoptr - linesoffset) / LINESZ;
	    	    entersym(name, np, instrtab, false);
            } else if (!lazy || process_vars)
                entersym(name, np, instrtab, false);
	    break;

	case C_ENTRY:
            if (!lazy || process_vars)
                entersym(name, np, instrtab, false);
              else
                mark_entry_sym(name, instrtab);

	    break;

	case C_FCN:
		defsyms = false;
		if (!strcmp(name,".bf")) {
		    if (curlinep) 
			defsyms = true;
		    /* bflevel records number of nested level for lazy */
		    /* read because .bf doesn't produce any action for */
		    /* the read pass of lazy read.                     */
		    if (lazy) bflevel++; 			
		    if (!(lazy && process_vars)) {
			lnostart = axp->x_sym.x_misc.x_lnsz.x_lnno - 1;
			if (curlinep && linenoptr) {
		   	 adjustlines(&curlinep[ln_index], lnostart, text_reloc);
			}
		    }
		}
		else
                {	
		  struct routinestack	*tmp_routine;

                  curblock->symvalue.funcv.src = true;
                  exitblock();  /* Exit block on .ef */
                  if (lazy)
                    bflevel--; 			
                  if (curroutine) {
		    tmp_routine = curroutine;
                    curroutine = curroutine->back;
		    free( tmp_routine );
		  }
                  if (curroutine)
                  {
                    curroutine->next = nil;
                    curparam = curroutine->name;
                  }
                }
		break;

	case C_AUTO:
	case C_REG:
	case C_EXTDEF:
	case C_ULABEL:
	case C_MOS:
	case C_ARG:
	case C_MOU:
	case C_TPDEF:
	case C_USTATIC:
	case C_MOE:
	case C_REGPARM:
	case C_FIELD:
	case C_EOS:
	    break;

	case C_BINCL:
	    if (!(lazy && process_vars))
	        enterincl(name, np->n_value);
	    break;

	case C_EINCL:
	    if (!(lazy && process_vars))
	        exitincl(np->n_value);
	    break;
	case C_NULL:
		/* 
		 * if it's a valid deleted symtab entry, ignore 
		 *  Ref. defect 69438 
		 */
	    if ( np->n_value == DELETED_SYM )
		break;

	default:
	    if (!(lazy && process_vars)) {
            (*rpt_output)(stdout,  catgets(scmc_catd, MS_object, MSG_260,
				       "warning:  stab entry unrecognized: "));
	    if (name != nil) {
		(*rpt_output)(stdout, "name %s,", name);
	    }
	    (*rpt_output)(stdout, "type %2x, class %x, value %x'\n",
		np->n_type, class, np->n_value);
	    }
	    break;
    }
}

/*
 * NAME: mark_entry_sym
 *
 * FUNCTION: Find the symbol for the function corresponding to the
 *             entry name and mark the symbol as being an entry.
 *
 * PARAMETERS:
 *        none
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: none   
 *
 * RETURNS: nothing
 *
 * NOTES: This is only called for the first pass of lazy read.  The
 *        entry stabstrings are actually processed during the second
 *        pass of lazy read.  It is important to mark the symbols
 *        appropriately now because the whatblock routine skips over
 *        entries.  If they are not marked at that time, the scoping
 *        is incorrect, and some function symbols never get marked as
 *        being entries, and having source line information.
 *
 */

static void mark_entry_sym(String name, Boolean instrtab)
{
  Name n;
  Symbol funcdef;
  char *p;

  p = index(name, ':');
  name = get_name(name, p);
  n = identname(name, instrtab);

  find (funcdef, n) where
    (isroutine(funcdef) && (funcdef->block == curblock))
  endfind (funcdef)

  if (funcdef != NULL)
    markEntry(funcdef);
}

/*
 * Try to find the symbol that is referred to by the given name.  Since it's
 * an external, we need to follow a level or two of indirection.
 */

private Symbol findsym (n, isfunc, var_isextref)
Name n;
boolean isfunc;
boolean *var_isextref;
{
    register Symbol r, s;

    *var_isextref = false;
				/* First look for external vars or funcs */
    find(s, n) where
	(
	    /* we exclude C++ functions from this lookup because their names
	       have been demangled, and the match is based on the mangled
	       name */
	    s->level == program->level and (
		s->class == EXTREF or
		s->class == VAR or s->class == TOCVAR or
		s->class == PROC or (s->class == FUNC && !s->isCppFunction)
	    )
	)
    endfind(s);
    if (s == nil) {		/* Then look for modules second */
      find(s, n) where
	(
	    s->block == program and s->class == MODULE
	)
      endfind(s);
    }
    if (s == nil) {
	r = nil;
    } else if (s->class == EXTREF) {
	*var_isextref = true;
	r = s->symvalue.extref;
	delete(s);

	/*
	 * Now check for another level of indirection that could come from
	 * a forward reference in procedure nesting information.  In this case
	 * the symbol has already been deleted.
	 */
	if (r != nil and r->class == EXTREF) {
	    r = r->symvalue.extref;
	}
    } else {
	r = s;
    }
    /* This probably should not be done.  This is going to be done anyhow.
    if (isfunc && r != nil && r->class != FUNC && r->class != MODULE) {
	r->class = FUNC;
	r->symvalue.funcv.src = false;
	r->symvalue.funcv.inline = false;
	r->symvalue.funcv.dontskip = false;
	r->symvalue.funcv.isentry = false;
	r->symvalue.funcv.islinkage = false;
        r->symvalue.funcv.fcn_desc = nil;
	fdata(r->name, r->symvalue.offset);
    }
    */
    return r;
}

/*
 * Create a symbol for a text symbol with no source information.
 * We treat it as an assembly language function.
 */

private Symbol deffunc (n, block, level)
Name n;
Symbol block;
int level;
{
    Symbol f;

    f = insert(n);
    f->language = findlanguage(".s");
    f->class = FUNC;
    f->type = t_int;
    f->block = block;
    f->level = level;
    f->symvalue.funcv.src = false;
    f->symvalue.funcv.inline = false;
    f->symvalue.funcv.isentry = false;
    f->symvalue.funcv.islinkage = islinkage;
    f->symvalue.funcv.dontskip = false;
    f->symvalue.funcv.fcn_desc = nil;
    if (f->chain != nil) {
        panic( catgets(scmc_catd, MS_object, MSG_263,
						  "chain not nil in deffunc"));
    }
    return f;
}

/*
 * NAME: mark_dontskip
 *
 * FUNCTION: Mark some routines as needing special attention when
 *	stepping.
 *
 * EXECUTION ENVIRONMENT:
 *	Normal user process
 *
 * NOTES: There are some routines that we want to single step through even if
 *	the $stepignore variable is set.  These are routines that make non
 *	local gotos or just don't return to the point they were called from.
 *	For these routines, we set a bit in the symbol that tells dostep()
 *	that it cannot ignore them.
 *
 *	This routine needs to be called after symbols are loaded at any time.
 *
 * RECOVERY OPERATION: none
 *
 * DATA STRUCTURES: none
 *
 * PARAMETERS:none
 *
 * TESTING: Tested with the step command
 *
 * RETURNS: void
 *
 */
private void mark_dontskip()
{
    static char *names[] = {
	"$PTRGL",
	"longjmp",
	"__DoThrow",
	"__Throw",
	"__RestoreRegsAndJump",
	"__ReThrow",
	"__BadThrow",
	"__InvokeDestructor",
	"__CleanupCatch",
	"__Invoke__Destructor",
	"__ResetSPAndReturn",
	"__unexpected__Fv",
	"terminate__Fv",
	"__start",
	"exit",
	"_exit",
    };
    int i;
    Symbol	s;
    Name	n;

    for (i=0; i<sizeof(names)/sizeof(char *); i++)
    {
	n=identname(names[i], true);
	find(s, n) where s->class == FUNC endfind(s);
	if (s)
	    s->symvalue.funcv.dontskip = true;
    }
}    

/*
 * Create a symbol for a data or bss symbol with no source information.
 * We treat it as an assembly language variable.
 */

private Symbol defsym (n, block, level)
Name n;
Symbol block;
int level;
{
    Symbol v;

    v = insert(n);
    v->language = findlanguage(".s");
    v->storage = EXT;
    v->class = (isTOCsym) ? TOCVAR : VAR;
    v->type = t_int;
    v->block = block;
    v->level = level;
    return v;
}

/*
 * Update a symbol entry with a text address.
 */

private updateTextSym (s, name, addr)
Symbol s;
char *name;
Address addr;
{
    if ((s->class == VAR) || (s->class == TOCVAR)) {
	s->symvalue.offset = addr + text_reloc;
    } else {
        s->symvalue.funcv.beginaddr = addr;
	s->symvalue.funcv.proladdr = addr;
	newfunc(s, prolloc(s), 0);
	findbeginning(s);
    }
}

/*
 * Avoid seeing Pascal labels as text symbols.
 */

private boolean PascalLabel (n)
Name n;
{
    boolean b;
    register char *p;

    b = false;
    if (curlang == pascalLang) {
	p = ident(n);
	while (*p != '\0') {
	    if (*p == '_' and *(p+1) == '$') {
		b = true;
		break;
	    }
	    ++p;
	}
    }
    return b;
}

/*
 * Check to see if a global _name is already in the symbol table,
 * if not then insert it.
 */

private check_global (name, np, instrtab)
String name;
register SYMENT *np;
Boolean instrtab;
{
    register Name n, original_n;
    register Symbol u = nil, t = nil;
    char buf[4096];
    boolean isfunc, isextref;
    integer count;
    char *nmptr;
    Desclist *fcndesc = nil;
    boolean has_main_name;
    Address curaddr;

    if (not streq(name, "_end"))
    {
	isfunc = false;
	curaddr = np->n_value +
		    ((np->n_scnum == text_scn_num) ? text_reloc : data_reloc);
	if (isTOCsym && (!noexec) && (fullcore || (!coredump)))
	    dread(&curaddr, curaddr, 4);
	if (name[0] == '.')
        {
	    isfunc = true;
	    n = identname(&name[1], false);
	}
	else 
        {
	    original_n = n = identname(nmptr = name, false);
	    find(t, original_n) where
		(t->storage == EXT) && (t->symvalue.offset != 0) &&
		(t->level == program->level) &&
		(!objs_differ(t->symvalue.offset,curaddr))
	    endfind(t);
	    if (t != nil) 
            {
		 if ((curaddr == t->symvalue.offset) || 
		     ((!isTOCsym) && (t->class == TOCVAR))) {
		     if (!isTOCsym) {
		         if (isdefinition) {
			     t->block = curmodule;
			 }
			 if (t->class == TOCVAR) {
			     t->block = curmodule;
			     if (isCMsym && 
			         (curlang == fLang) )
			     {
			        /* unamex entry for fortran common */
		     	        t->class = COMMON;
			        t->symvalue.common.com_addr = curaddr;
			        t->symvalue.common.offset = nil;
		 	     } else {
			        t->symvalue.offset = curaddr;
			        t->class = VAR;
			     }
			 }
		     }
		     return;
		 } else {
		     buf[0] = '_';
		     strcpy(&buf[1], nmptr);
		     n = identname(buf, false);
		 }
	    }
	}
	if (np->n_scnum == text_scn_num)
		/* || ((np->n_sclass&N_SECT) == N_ABS)) */
        {
	   /* Eliminate previous linkage entries as unnecessary. */
	    if (!islinkage && !keep_linkage) {
	      fcndesc = ElimLinkage(n);
	    }
	    count = 0;
	    t = findsym(n, isfunc, &isextref);
	    while (isextref) 
            {
		++count;
		updateTextSym(t, name, curaddr);
		t = findsym(n, isfunc, &isextref);
	    }
	    if (count == 0) 
            {
		has_main_name = (boolean)
			    	   ((isFORTRAN && streq(name, FORT_MAIN)) ||
			     	    (isPascal && streq(name, PAS_MAIN)));
		if (t == nil) 
                {
/*
		    After examining Profile: removed this because
		    we don't ever have any Pascal Labels.
		    if (not PascalLabel(n))    
*/
                    {
		        if (isCOBOL && isfunc &&
			    !strcmpi(ident(n), ident(curblock->name)) &&
			    curblock->block->class == MODULE)
			    t = deffunc(n, curblock->block, program->level);
			else
			    t = deffunc(n, curblock, program->level);
			if (!islinkage) {
			     t->symvalue.funcv.fcn_desc = fcndesc;
			}
			if (has_main_name) {
			     fake_main = t;
			}
			updateTextSym(t, name, curaddr);
			if (tracesyms)
			    printdecl(t);
		    }
		} 
                else if (t->class == MODULE) 
                {
		    u = t;
		    t = deffunc(n, curblock, program->level);
		    t->symvalue.funcv.fcn_desc = fcndesc;
		    if (has_main_name) {
			     fake_main = t;
		    }
		    t->block = u;
		    updateTextSym(t, name, curaddr);
		    if (tracesyms)
			printdecl(t);
		} 
                else if ((t->class == FUNC) || (t->class == PROC))
                {
		    if (isfunc) {
			if (t->symvalue.funcv.islinkage) {
			    if (!islinkage) {
				delete(t);
				t = deffunc(n, curblock, program->level);
			        t->symvalue.funcv.fcn_desc = fcndesc;
				updateTextSym(t,name,curaddr);
			    }
			    /* keep linkage routine of different modules */
			    else if (addrtoobj(prolloc(t)) != 
				     addrtoobj(prolloc(curblock))) {
                                t = deffunc(n, curblock, program->level);
			        t->symvalue.funcv.fcn_desc = fcndesc;
				updateTextSym(t,name,curaddr);
			    }

			} else if (!islinkage) {
			    if objs_differ(prolloc(t),curaddr)
			    {
				if (fcndesc == nil)
				    fcndesc = t->symvalue.funcv.fcn_desc;
				t = deffunc(n, curblock, program->level);
			        t->symvalue.funcv.fcn_desc = fcndesc;
				updateTextSym(t,name,curaddr);
			    } else {
			        t->symvalue.funcv.fcn_desc = fcndesc;
				updateTextSym(t,name,curaddr);
			    }
			}
                        else if (islinkage && keep_linkage)
                        {
                          t = deffunc(n, curblock, program->level);
                          t->symvalue.funcv.fcn_desc = fcndesc;
                          updateTextSym(t,name,curaddr);
                        }
		    	if (has_main_name) {
		     	    fake_main = t;
			}
		    }
		}
                else if (isfunc)
                {
		        if (t->class == TOCVAR) {
			    delete(t);
			    sprintf(buf, "_%s", &name[1]);
			    t->name = identname(buf, false);
			    insertsym(t);
			}
			t = deffunc(n, curblock, program->level);
			t->symvalue.funcv.fcn_desc = fcndesc;
			updateTextSym(t, name, curaddr);
		    	if (has_main_name) {
		     	    fake_main = t;
			}
			if (tracesyms)
			    printdecl(t);
		}
		else
		    updateTextSym(t, name, curaddr);
	    }
	} 
        else if ((np->n_scnum == bss_scn_num)||(np->n_scnum == data_scn_num))
        {
	    Address com_org;

	    find(t, original_n) where
		t->class == COMMON
	    endfind(t);
	    if (t != nil) 
            {
		com_org = curaddr;
		t->symvalue.common.com_addr = com_org;
		u = (Symbol) t->symvalue.common.offset;
		/* One relocation per common area, please. */
		if ((u != nil) && (u->symvalue.offset < 0x10000000)) {
		    while (u != nil) 
                    {
		        u->symvalue.offset = u->symvalue.common.offset+com_org;
		        u = u->symvalue.common.chain;
		    }
		}
            } 
            else
		check_var(np, n);
        } 
        else 
	    check_var(np, n);
    }
    csectname = n;
}

/*
 * Check to see if a namelist entry refers to a variable.
 * If not, create a variable for the entry.  In any case,
 * set the offset of the variable according to the value field
 * in the entry.
 *
 * If the external name has been referred to by several other symbols,
 * we must update each of them.
 */

private check_var (np, n)
SYMENT *np;
register Name n;
{
    register Symbol t, u, next;
    Symbol conflict;
    Address data_addr;
    boolean has_main_name = false;
    String name;

    find (t, n) where
	(t->class == MODULE && t->block == program) ||
						(t->level == program->level)
    endfind(t);
    name = ident(n);
    if (t == nil) {
	t = defsym(n, curblock, program->level);
	t->symvalue.offset = np->n_value + data_reloc;
	if ((isTOCsym) && (!noexec) && (fullcore || (!coredump))) {
	     dread(&t->symvalue.offset, t->symvalue.offset,4);
	}
	if (tracesyms) {
	    printdecl(t);
	}
	has_main_name = (boolean) (((isASM || isCOBOL) &&
				    streq(name, COBOL_MAIN)));
    } else {
	conflict = nil;
	if ((isTOCsym) && (!noexec)) {
	    dread(&data_addr, np->n_value+data_reloc, 4);
	} else {
	    data_addr = np->n_value+data_reloc;
	}
	do {
	    next = t->next_sym;
	    if (t->name == n) {
		if (t->class == MODULE and t->block == program) {
		    conflict = t;
		} else if (t->level == program->level) {
		    switch (t->class) {
			case EXTREF:
			    u = t->symvalue.extref;
			    while (u != nil and u->class == EXTREF) {
				u = u->symvalue.extref;
			    }
			    u->symvalue.offset = data_addr;
			    delete(t);
			    break;
			case VAR:
			    conflict = nil;
			    if (!isTOCsym && t->storage == EXT) {
			        t->symvalue.offset = data_addr;
			    } else {
				Symbol z;
    				find (z, n) where
				  (z->class == FUNC && z->language == asmLang
				   && z->symvalue.funcv.beginaddr == data_addr)
    				endfind(z);
				if (z != nil) {
				  t->symvalue.offset = data_addr;
				  delete(z);
				}
			    }
			    break;
			case TOCVAR:
			    conflict = nil;
			    t->symvalue.offset = data_addr;
			    t->block = curblock;
			    t->class = VAR;
			    break;
			case PROC:
			case FUNC:
			case CSECTFUNC:
			    if (!isTOCsym) {
			        fdata(n, data_addr);
			    }
			    return;
			default:
                            warning( catgets(scmc_catd, MS_object, MSG_266,
					       "global %s ignored"), ident(n));
		    }
		}
	    }
	    t = next;
	} while (t != nil);
	has_main_name = (boolean) (((isASM || isCOBOL) &&
				    streq(name, COBOL_MAIN)));
	if (conflict != nil) {
	    u = defsym(n, curblock, program->level);
	    u->block = conflict;
	    u->symvalue.offset = np->n_value + data_reloc;
	    if ((isTOCsym) && (!noexec) && (fullcore || (!coredump))) {
		 dread(&u->symvalue.offset, u->symvalue.offset,4);
	    }
	}
    }
    if (has_main_name) {
	fake_main = t;
    }
}

/*
 * Handle a data symbol with the same name as a known function.
 * These contain the address of function data areas.
 */

private fdata (n, addr)
Name n;
Address addr;
{
    char *str, *newstr;
    Symbol v;

    str = ident(n);
    newstr = malloc(strlen(str) + 2);
    sprintf(newstr, ".%s", str);
    v = defsym(identname(newstr, true), program, program->level);
    v->symvalue.offset = addr;
}

/*
 * Check to see if a local _name is known in the current scope.
 * If not then enter it.
 */

private check_local (name, np)
String name;
register SYMENT *np;
{
    register Name n;
    register Symbol s, t;

    if (name[0] == '.') {
	n = identname(&name[1], false);
    } else {
	n = identname(name, false);
    }
    s = lookup(n);

    for (t = s; t != nil; t = t->next_sym) {
      if (t->name == n && t->block == curblock) {
        t->symvalue.offset = np->n_value + text_reloc;
        return;
      }
    }
    for (t = s; t != nil; t = t->next_sym) {
      if (t->name == n && t->block == curmodule && t->class == FUNC) {
        fdata(n, np->n_value + text_reloc);
        return;
      }
    }
    t = defsym(n, curblock, curblock->level);
    t->symvalue.offset = np->n_value;
}

/*
 * Check to see whether we desire to read in this symbol or not.
 */
private boolean check_auxent (np, ndxp, axp)
SYMENT *np;
unsigned long *ndxp;
AUXENT *axp; 
{
    boolean ignoresym = false;
    int smclass;

    if (np->n_numaux) {
       *ndxp += np->n_numaux;
       if (IS_EXT(np) || (isdefinition && isTOCsym)) {
	   if (np->n_numaux > 1)
               FSEEK(ld_ptr, (long int)((np->n_numaux - 1) * SYMESZ), 1);
           FREAD((void *)axp, (size_t) SYMESZ, (size_t) 1, ld_ptr);
	   smclass = axp->x_csect.x_smclas;
	   islinkage = (boolean) (smclass == XMC_GL); /* Global linkage */
	   ignoresym = (boolean) ((smclass == XMC_DB)  || /* Debug Dict. */
	       			  (smclass == XMC_XO)  || /* Extended Op */
				  (smclass == XMC_TI)  || /* Traceback Index */
	       			  (smclass == XMC_TB)  || /* Traceback CSECT */
#ifdef KDBX
				  (smclass == XMC_SV)  || /* Supervisor */
#endif
	       			  (smclass == XMC_TC0) || /* TOC Anchor */
				  (smclass == XMC_DS)  || /* Descriptor */
				  ((smclass == XMC_TC) && (!isTOCsym)));
	   if ((SM_TYPE(axp) == XTY_SD) || (SM_TYPE(axp) == XTY_CM)) {
	       if (SM_TYPE(axp) == XTY_CM)
		  isCMsym = true;
	       csectaddr = np->n_value;
	       if (np->n_scnum == text_scn_num)
		   csectaddr += text_reloc;
	       else if ((np->n_scnum == data_scn_num) ||
						  (np->n_scnum == bss_scn_num))
		   csectaddr += data_reloc;
	   }
       } else {
	   if (isTOCsym)
		ignoresym = true;
           FSEEK(ld_ptr, (long int) (np->n_numaux * SYMESZ), 1);
       }
    }
    return ignoresym;
}

/*
 * Check to see if a symbol is about to be defined within an unnamed block.
 * If this happens, we create a procedure for the unnamed block, make it
 * "inline" so that tracebacks don't associate an activation record with it,
 * and enter it into the function table so that it will be detected
 * by "whatblock".
 */
static int bnum = 0;

public chkUnnamedBlock ()
{
    register Symbol s;
    char buf[100];

    if (nesting > 0 and addrstk[nesting] != NOADDR) {
	if (!(lazy && process_vars)) {
	   ++bnum;
	   sprintf(buf, "$b%d", bnum);
	   s = insert(identname(buf, false));
	   s->language = curlang;
	   s->class = PROC;
	   s->symvalue.funcv.src = false;
	   s->symvalue.funcv.inline = true;
	   s->symvalue.funcv.dontskip = false;
	   s->symvalue.funcv.isentry = false;
	   s->symvalue.funcv.islinkage = false;
	   s->symvalue.funcv.fcn_desc = nil;
	   s->symvalue.funcv.beginaddr = addrstk[nesting];
	   enterblock(s);
	   newfunc(s, addrstk[nesting], curfilep);
	   addrstk[nesting] = NOADDR;
        } else {
	/* Find the unnamed block at this address.  What if there are
	 * two unnamed blocks starting at the same address? */

	   Symbol bb;
	   bb = whatblock(addrstk[nesting]);
	   if (bb != nil) 
	   	pushBlock(bb);
	   addrstk[nesting] = NOADDR;
	}
    }
}


/* Used by dbx graphic interface to reset */
/* unnamed block number after a reload.   */
public resetUnnamedBlockbnum()
{
  bnum = 0;
}


/*
 * Adjust the line number table so that the line numbers reflect the source.
 */

private adjustlines (ltab, offset, reloc)
Linetab *ltab;
unsigned short offset;
Address reloc;
{
   Linetab *lp = ltab;		/* Originally points to function entry. */
   Linechunk *lc, *lcback, *lcp;
   cases folding;

   if (!hassource) {	/* Count this as a -g source file */
	hassource = true;
	sourcefiles++;
        folding = (*language_op(curlang, L_FOLDNAMES))();
	if (folding == upper)
	  ++uppersyms;
	else if (folding == lower)
	  ++lowersyms;
        if (curlang == cobLang)
	  hascobolsrc = true;
        stat_error = stat(findsource(curfilep->filename, NULL),
                          &file_time);
        if ((file_time.st_mtime > symtime) && !stat_error)
             warning(catgets(scmc_catd, MS_object, MSG_169,
			   "%s is newer than %s"),
             basefile(curfilep->filename), loadname);
   }
   lineaxp->lp = ++lp;
   lineaxp->addr = ltab->addr = lastfuncaddr;  /* Get the function address */

   if (minlineaddr < lastfuncaddr)
       minlineaddr = lastfuncaddr;

   /* Insert line number chunk into file list */
   lc = (Linechunk *) malloc(sizeof(Linechunk));
   lc->lp = lineaxp->lp;
   for (lcp = curfilep->lineptr;
	     (lcp != nil) && (lcp->lp->addr < lastfuncaddr); lcp = lcp->next) {
       lcback = lcp;
   }
   if (!curfilep->lineptr) {
       lc->next = (Linechunk *) nil;
       curfilep->lineptr = lc;
   } else {
       lc->next = lcp;
       if (lcp == curfilep->lineptr) {
	   curfilep->lineptr = lc;
       } else {
           lcback->next = lc;
       }
   }

   /*
    *  We used to get rid of the fake line number entry
    *  by setting ltab->line = lp->line + offset but this messed
    *  up dbx when it was debugging pascal programs so we now
    *  leave it at zero and then adjust it later.
    *  ltab->line = lp->line + offset;
    */

   for (; lp->line; lp++) {	/* Continue until next function entry.  */
   /* Do this once for an entire file's worth in process_linenumbers()
    *   lp->line += offset;
    */
	lp->addr += reloc;
   } 
   lc->lend = lineaxp->lend = (lp - 1);
   lc->line_reloc = offset;
   if (lc->lend >= lc->lp)
      lineaxp++;
   source_file_unprocessed = true;	/* Still must adjust line numbers */
}

/*
 * Compilation unit.  C associates scope with filenames
 * so we treat them as "modules".  The filename without
 * the suffix is used for the module name.
 *
 * Because there is no explicit "end-of-block" mark in
 * the object file, we must exit blocks for the current
 * procedure and module.
 */

private enterSourceModule (n, addr, stamps, name_field, ldcnt)
Name n;
Address addr;
unsigned short stamps;                  /* cpu and lang stamps */
char *name_field;                       /* field with stab compact level */
int ldcnt;				/* load module count */
{
    register Symbol s;
    Name nn;
    String mname, mname_save, suffix;

    /* 
     * Check to see if any source files are newer than the executable 
     */
    if (unique_fns) {  /* Have to save room for the prepended '@' */
	mname_save = mname = (char *) malloc(strlen(ident(n))+2);
	strcpy(&mname[1],ident(n));
	mname++;
    } else {
    	mname_save = mname = strdup(ident(n));
    }
    if (rindex(mname, '/') != nil) {
	mname = rindex(mname, '/') + 1;
    }
    if (unique_fns) {
        *--mname = '@';
    }
    suffix = rindex(mname, '.');
    if (suffix > mname && *(suffix-1) == '.') {
	/* handle C++ */
	--suffix;
    }
    if (stamps) {
       /* if we have stamps info, use it to determine langauge */
       curlang = stampfindlang(stamps);

       /* Check and see if we have a special language field... */
       /* If so, get the 'ld' stabstring compaction level.     */
       /* Compaction level is stored at the fourth byte of     */
       /* np->n_name field if lang equals tb_front or tb_back  */
       if (curlang == tb_front || curlang == tb_back) {
          stab_compact_level = (int) name_field[3];
          /* Check and make sure the compaction level is supported */
          if (stab_compact_level < 0 || stab_compact_level > 2) {
             warning( catgets(scmc_catd, MS_object, MSG_640,
          "unsupported binder stabstring compaction level %d, resetting to 1"),
                                                        stab_compact_level);
             stab_compact_level = 1;
          }
          if (traceexec)
             (*rpt_output)(stdout,
                              "***compact level = %d\n", stab_compact_level);
          /* reset curlang for special Lang ID */
          curlang = (Language) nil;
          return;
       }
       if (!curlang)
          /* if we couldn't determine lang from stamps, use suffix */
          curlang = findlanguage(suffix);
    } else {
       /* no stamp, use filename suffix to determine language */
       curlang = findlanguage(suffix);
    }
    hassource = false;
    isPascal = isFORTRAN = isCOBOL = isASM = strip_ = false;
    if (curlang == fLang) {
	if (strip_fortran) strip_ = true;
	isFORTRAN = true;
    } else if (curlang == pascalLang) {
	isPascal = true;
    } else if (curlang == cobLang) {
	isCOBOL = true;
    } else if (curlang == asmLang) {
	isASM = true;
    } else if (curlang == cppLang) {
	cppModuleSeen = true;
    }
    if ((suffix != nil) && (!(lazy && process_vars))) {
	*suffix = '\0';
    }
    if (not (*language_op(curlang, L_HASMODULES))()) {
	if (curblock->class != PROG) {
	    exitblock();
	    while (curblock->class != PROG) {
		exitblock();
	    }
	}
	nn = identname(mname, true);
	if( mname != nn->identifier ) {
	    /*
	     * identname() didn't use the allocated memory for mname
	     */
	    free( mname_save );
	}
	if (!(lazy && process_vars)) {
	   if (curmodule == nil or curmodule->name != nn) {
	       s = insert(nn);
	       s->class = MODULE;
	       /* The following two lines merely establish which
	          load_module the file came from. */
	       s->symvalue.funcv.beginaddr = loader_info[ldcnt].textorg;
	       s->symvalue.funcv.proladdr = loader_info[ldcnt].textorg;
	       s->symvalue.funcv.untouched = (unsigned) (lazy);
	       if (lazy && !process_vars) {
	   	   /* Overload func_desc with offset into symbol table. */
	   	   s->symvalue.funcv.fcn_desc = (Desclist *) addr;
	       }
	       findbeginning(s);
	   } else {
	       s = curmodule;
	   }
	   s->language = curlang;
	   enterblock(s);
	} else {
	   /* Find file symbol, and set s */
	   s = touchfile;
	   pushBlock(s);
	}
	curmodule = s;
    }
    if (program->language == nil) {
	program->language = curlang;
    }
    if (!(lazy && process_vars)) {
        warned = false;
        enterfile(ident(n));
    }
    initTypeTable(&newprog);
}

/*
 * Allocate file and line tables and initialize indices.
 */

private allocmaps (ldndx,nf,nl)
int ldndx;
unsigned int nf;
unsigned int nl;
{
    if (filetab[ldndx] != nil) {
	dispose(filetab);
    }
    if (linetab[ldndx] != nil) {
	dispose(linetab);
    }
    if( nf > 0 ) {
	filetab[ldndx] = newarr(Filetab, nf);
    } else {
	filetab[ldndx] = NULL;
    }
    linetab[ldndx] = newarr(Linetab, nl+1);
    nlines_total += nl;
}

/*
 * Add a file to the file table.
 *
 * If the new address is the same as the previous file address
 * this routine used to not enter the file, but this caused some
 * problems so it has been removed.  It's not clear that this in
 * turn may not also cause a problem.
 */

private enterfile (filename)
String filename;
{
    if (source_file_unprocessed) /* Cleanup line numbers from last file */
	process_linenumbers();

    curfilep = filep;
/*
 *      filep->addr and filep->lineindex are only determinable as the 
 *      procedures within the file are processed.
 *
 */
    filep->lineptr = (Linechunk *) 0;
    filep->incl_chain = (Incfile *) 0;
    filep->filename = filename;
    ++filep;
}

/*
 * Process an include file.
 */

private enterincl (filename, lp)
String filename;
unsigned int lp;
{
    Incfile *inkp, *backinkp;
    unsigned long line_index;
	
    if (curlinep == nil) /* Line number information stripped */
	return;
			 /* invalid lp value */
    if (lp < linesoffset) 
	fatal( catgets(scmc_catd, MS_object, MSG_424,
              "invalid value for include file %s"), filename);

    inkp = (Incfile *) calloc(1,sizeof(Incfile));
    line_index = (lp - linesoffset) / LINESZ;
    lp = (unsigned int) &curlinep[line_index];
    inclfiles++;
    if (inkp) {		/* Add information for a new include file. */
	if (curincl) {
            inkp->incl_parent = curincl;
	    if (curincl->incl_child) {
		if (lp < (unsigned int) curincl->incl_child->lp) {
		    inkp->incl_chain = curincl->incl_child;
		    curincl->incl_child = inkp;
		} else {	/* Insert in order of lp information */
		    for (backinkp = curincl->incl_child;
		         ((backinkp->incl_chain != nil) &&
					((unsigned int) backinkp->incl_chain->lp < lp));
		         backinkp = backinkp->incl_chain);
		    inkp->incl_chain = backinkp->incl_chain;
		    backinkp->incl_chain = inkp;
		}
	    } else {
	        curincl->incl_child = inkp;
	    }
	} else {
	    if (curfilep->incl_chain) {
		if (lp < (unsigned int) curfilep->incl_chain->lp) {
		    inkp->incl_chain = curfilep->incl_chain;
		    curfilep->incl_chain = inkp;
		} else {	/* Insert in order of lp information */
		    for (backinkp = curfilep->incl_chain;
		         ((backinkp->incl_chain != nil) &&
			  ((unsigned int) backinkp->incl_chain->lp < lp));
		         backinkp = backinkp->incl_chain);
		    inkp->incl_chain = backinkp->incl_chain;
		    backinkp->incl_chain = inkp;
		}
	    } else {
	        curfilep->incl_chain = inkp;
	    }
	}
	inkp->filename = (String)malloc(strlen(filename)+1);
	strcpy(inkp->filename, filename);
        inkp->src_file = curfilep;
        inkp->lp = (Linetab *) lp;
	curincl = inkp;
    }
}

/*
 * Process the end of an include file.
 */

private exitincl (lp)
unsigned int lp;
{
    unsigned long line_index;
	
    if (curlinep == nil) /* Line number information stripped */
	return;
    /* check for invalid lp value */
    if (lp < linesoffset)
        fatal( catgets(scmc_catd, MS_object, MSG_424,
                                "invalid value for include file %s"));
    line_index = (lp - linesoffset) / LINESZ;
    lp = (unsigned int) &curlinep[line_index];

    if (curincl) {
        curincl->lend = (Linetab *) lp;
	curincl = curincl->incl_parent;
    }
}

/*
 * Adjust the line number tables to reflect correct line number
 * from source file, taking include files into account.
 */

private process_linenumbers ()
{
/*
 *  In order to preserve the COFF-like mechanism which allows for
 *  line number values up to 128K, we must use the relative line 
 *  numbering scheme of adding the .bf lineno value to the line number
 *  in the line number table.  This will not work for cases in which
 *  a line number within a function comes from within an include file,
 *  since there is nothing to be relative to.  In those cases, we must
 *  use absolute line numbering, which does limit line number representation
 *  up to 64K.  The first line will always have a relative line number.
 *  Any line number within that function which is not on the same level
 *  as the first line will be absolute.
 */
    Linechunk *flp;	/* Function line number sections from within file.  */
    Linetab   *lp;	/* Used to traverse line number entries */
    unsigned int relative_line;
    Incfile *inktop, *curink, *nextink; /* Include file traversers */

    if (curfilep->incl_chain) {	/* Process procs with include files. */
	/* Grab the initial include file. */
        for (nextink = curfilep->incl_chain;
	     (nextink->incl_child) && 
				    (nextink->incl_child->lp <= nextink->lp);
	     nextink = nextink->incl_child);
	/* See if we start off in it. */
	if (curfilep->lineptr->lp  >= nextink->lp) {
	    inktop = curink = nextink;
	    getnextincl(&nextink,curfilep->lineptr->lp);
	} else {
	    inktop = curink = nil;
	}
        for (flp = curfilep->lineptr; flp != nil; flp = flp->next) {
	     relative_line = flp->line_reloc;
	     if (flp->lp > curink->lend) {
		  if ((nextink) && (flp->lp >= nextink->lp)) {
		      curink = nextink;
		      getnextincl(&nextink,flp->lp);
		  } else {
		      curink = nil;
		  }
	     }
	     inktop = curink;
	     for (lp = flp->lp; lp <= flp->lend; lp++) {
		  if (curink) {
		      if (lp > curink->lend) {
		          if ((nextink) && (lp >= nextink->lp)) {
		     	      curink = nextink;
		     	      getnextincl(&nextink,lp);
		          } else {
		     	      curink = nil;
		          }
		      } else {
		          /* Check to see if we've entered a nested include */
		          if ((nextink) && (nextink->lend <= curink->lend) &&
							(lp >= nextink->lp)) {
		     	      curink = nextink;
		     	      getnextincl(&nextink,lp);
			  }
		      }
		  } else if (nextink) {
		      if (lp >= nextink->lp) {
		     	  curink = nextink;
		     	  getnextincl(&nextink,lp);
		      }
		  }
		  if ((curink == inktop) ||
                      ((curink != nil) && (inktop != nil) &&
                       streq(curink->filename, inktop->filename)))
                  {
		      lp->line += relative_line;
		  }
	     }
	 }
    } else {	/* No include files, process normally */
        for (flp = curfilep->lineptr; flp != nil; flp = flp->next) {
	     relative_line = flp->line_reloc;
	     for (lp = flp->lp; lp <= flp->lend; lp++) {
		  lp->line += relative_line;
	     }
	}
    }
    source_file_unprocessed = false;
}

/*
 * Adjust so that we have the right number of files for binary search.
 */

private setnfiles ()
{
    Symbol s;
    String mainfile;
    Address main_addr;
    int ldndx;

    if (fake_main) {
	 if ((hascobolsrc) && (streq(ident(fake_main->name),COBOL_MAIN))) {
	     dread(&main_addr,fake_main->symvalue.offset,4);
	 } else {
	     main_addr = prolloc(fake_main);
	 }
	 s = whatblock(main_addr);
    } else {
	find(s, identname("main", false)) where
	    (s->class == FUNC ||
	     s->class == PROC ||
	     s->class == CSECTFUNC)
	endfind(s)
	/* s = lookup(identname("main",false)); */
    }

    if ((s != nil) && (isroutine(s) || (s->class == CSECTFUNC)))
    {
	mainfile = srcfilename(codeloc(s));
	if (mainfile != nil) {
 	    setsource(mainfile);
	} else {
	    /*
	    s2 = lookup(identname("MAIN",false));
	    if ((s2 != nil) && (isroutine(s2) || (s->class == CSECTFUNC))) {
		mainfile = srcfilename(codeloc(s2));
		s = s2;
		if (mainfile != nil) {
		    setsource(mainfile);
		} else {
			for (ldndx = 0; ldndx < loadcnt; ldndx++) {
			   if (filetab[ldndx] != nil) {
			      setsource(mainfile = filetab[ldndx][0].filename);
			      break;
			   }
			}
			return;
		}
	    }
	    */
	    for (ldndx = 0; ldndx < loadcnt; ldndx++) {
	        if (filetab[ldndx] != nil) {
		    int i;
		    boolean found = false;
		    for (i = 0; i < nlhdr[ldndx].nfiles; ++i) {
			if (filetab[ldndx][i].lineptr != nil) {
	                    mainfile = filetab[ldndx][i].filename;
	                    setsource(mainfile);
			    found = true;
	            	    break;
			}
		    }
		    if (found)
			break;
		}
	    }
	}
	setcurfunc(s);
    } else {
	for (ldndx = 0; ldndx < loadcnt; ldndx++) {
	   if (filetab[ldndx] != nil) {
	      setsource(mainfile = filetab[ldndx][0].filename);
	      break;
	   }
	}
    }
}

public boolean isfcn(s)
SYMENT *s;
{
  return ((((s)->n_sclass == C_EXT) ||
	   ((s)->n_sclass == C_HIDEXT)) &&
	  ((s)->n_numaux > 1) &&
	  (!ISARY((s)->n_type)) &&
	  (!ISPTR((s)->n_type)));
}

/*
 * Eliminate previous linkage entries as unnecessary.
 */
public Desclist *ElimLinkage(n)
     Name n;
{
  Symbol t, nextsym;
  Desclist *fcndesc = nil, *newdesc = nil;

  for (t = lookup(n); t != nil;) {
    nextsym = t->next_sym;
    if ((t->name == n) && (t->level == program->level) && 
	((t->class == PROC) || (t->class == FUNC)) &&
	(t->symvalue.funcv.islinkage)) {
      newdesc = (Desclist *) malloc(sizeof(Desclist));
      newdesc->descaddr = prolloc(t);
      newdesc->next_desc = fcndesc;
      fcndesc = newdesc;
      delete(t);
    }
    t = nextsym;
  }

  return fcndesc;
}


void string_debug_tab_free()
{
    int	i;

    /*
     * Walk through and free up all the mmapped regions.
     */
    if (dmmap_ary != nil)
    {
	for (i=0; i<loadcnt; i++)
	{
	    if (dmmap_ary[i].mapped)
	    {
		if (dmmap_ary[i].start != nil)
		{
		    munmap(dmmap_ary[i].start, dmmap_ary[i].count);
		}
	    }
	    else
		dispose(dmmap_ary[i].debug);
	}
	dispose(dmmap_ary);
    }

    dispose( stringtab );
    dispose( stringtab_ary );

    loadname = NULL;
    warned = false;
    stripped = false;
    defsyms = false;
    filep = NULL;
    linep = NULL;
    lineaxp = NULL;
    lineaux_length = 0;
    lnostart = 0;
    foundstab = false;
    proginit = false;
    newprog = false;
    hassource = false;
    stat_error = 0;
    nlaux_total = 0;
    linesoffset = 0;
    dispose(symtab_position);
    lowersyms = 0;
    uppersyms = 0;
    continued = 0;
    islinkage = false;
    isdefinition = false;
    isTOCsym = false;
    isCMsym = false;
    isshlib = false;
    curincl = nil;
    source_file_unprocessed = false;
    dispose(blkstack);
    curlevel = 0;
}
