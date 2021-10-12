static char sccsid[] = "@(#)84	1.74.3.24  src/bos/usr/ccs/lib/libdbx/symbols.c, libdbx, bos412, 9445C412a 11/11/94 15:39:57";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: InsertMemFunc
 *		MemFuncTabInit
 *		RetrieveMemFunc
 *		address
 *		assigntypes
 *		binaryop
 *		buildSet
 *		buildaref
 *		buildarray
 *		checkCobolOp
 *		chkboolean
 *		chkflt
 *		chkint
 *		classScopeWhich
 *		compatible
 *		constval
 *		container
 *		cppFunc_symname
 *		cpp_searchClass
 *		cpp_whereis
 *		criteriaStr
 *		deffregname
 *		defregname
 *		delete
 *		delete_unloaded_syms
 *		dot
 *		dotptr
 *		dpi_find_symbol_for_variable
 *		dpi_lookup
 *		dpi_which
 *		dump_externs
 *		dump_symbol_names
 *              dumpSymbolTable
 *		dumpfuncsyms
 *		dumpvars
 *		dynwhich
 *		encode
 *		evalindex
 *		findModuleMark
 *		findThis
 *		findbounds
 *		findfield
 *		findtype
 *		forward
 *		get_all_local
 *		get_externs_list
 *		get_file_static_list
 *		get_functions_list
 *		get_output_nodetype
 *		getbound
 *		getrangesize
 *		hash
 *		insert
 *		insertsym
 *		isambiguous
 *		isbitfield
 *		ischar
 *		ischartype
 *		iscobolglobal
 *		isconst
 *		isentry
 *		isfilestatic
 *		isglobal
 *		isintegral
 *		isinternal
 *		islocal
 *		ismodule
 *		isquad
 *		istypename
 *		isvariable
 *		isvarparam
 *		lookup
 *		lvalRecord
 *		maketype
 *		markEntry
 *		markInternal
 *		meets
 *		memberFunction
 *		mkstring
 *		multiword
 *		newSymbol
 *		nextarg
 *		passaddr
 *		primlang_typematch
 *		process_cbl_grp
 *		psize
 *		qualWhich
 *		regnum
 *		resolveRef
 *		rtype
 *		sameQualifiedName
 *		size
 *		staticMemberFunction
 *		stwhich
 *		subscript
 *		symbol_dump
 *		symbol_free
 *		symbols_init
 *		typematch_indices
 *		typename
 *		which
 *		withinblock
 *
 *   ORIGINS: 26,27,83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1982 Regents of the University of California
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
 * Symbol management.
 */

#include <setjmp.h>
#include "defs.h"
#include "envdefs.h"
#include "symbols.h"
#include "languages.h"
#include "printsym.h"
#include "tree.h"
#include "operators.h"
#include "eval.h"
#include "mappings.h"
#include "events.h"
#include "process.h"
#include "runtime.h"
#include "machine.h"
#include "names.h"
#include "frame.h"
#include "resolve.h"
#include "cplusplus.h"
#include "cma_thread.h"

#define AVG_NO_EXTERN 30      /*
                               * Average number of external variables in a
                               * program that are defined in files with debug
                               * information
                               */
#define AVG_NO_ALL_EXTERN     400     /*
                                       * Average number of external variables
                                       * in a program including those bound in
                                       * by libraries
                                       */
#define AVG_NO_STATIC 10      /*
                               * Average number of file static variables in a
                               * file
                               */
#define AVG_NO_LOCAL  AVG_NO_STATIC   /*
                                       * Average number of local variables in
                                       * a block
                                       */

public Symbol t_boolean;
public Symbol t_char;
public Symbol t_int;
public Symbol t_longlong;
public Symbol t_ulonglong;
public Symbol t_float;
public Symbol t_real;
public Symbol t_quad;
public Symbol t_complex;
public Symbol t_qcomplex;
public Symbol t_nil;
public Symbol t_addr;
public Symbol t_gchar;

/* Pre-defined types */
public Symbol dt_null;
public Symbol dt_arg;
public Symbol dt_char;
public Symbol dt_short;
public Symbol dt_int;
public Symbol dt_long;
public Symbol dt_float;
public Symbol dt_double;
public Symbol dt_struct;
public Symbol dt_union;
public Symbol dt_enum;
public Symbol dt_moe;
public Symbol dt_uchar;
public Symbol dt_ushort;
public Symbol dt_uint;
public Symbol dt_ulong;
public Symbol dt_uchar2;
public Symbol dt_ushort2;
public Symbol dt_uint2;
public Symbol dt_ulong2;
public Symbol dt_NLchar;
public Symbol dt_bool;

public Symbol program;
public Symbol curfunc;
public Sympool sympool = nil;
public Integer nleft = 0;

public boolean showaggrs;
extern boolean noexec;	/* Whether or not program is executable */
public int *envptr;
extern Symbol dbsubn_sym;

private Symbol partial_name(); /* C++ related func's and externs */
private Name tagname();
private binaryop();
private chkflt();
private chkint();
private encode();

extern char *sigdecode();
extern Boolean is_fortran_padded();
extern Language fLang;
extern Language cLang;
extern Language cppLang;

typedef enum { integral, floating, none } CobolType;
CobolType checkCobolOp();
Name dpi_get_ident();
void process_cbl_grp();
Symbol get_output_nodetype ();

/*
 * Symbol table structure currently does not support deletions.
 * Hash table size is a power of two to make hashing faster.
 * Using a non-prime is ok since we aren't doing rehashing.
 */
#define HASHTABLESIZE 8192

private Symbol hashtab[HASHTABLESIZE];

#define hash(name) ((((unsigned) name) >> 2) & (HASHTABLESIZE - 1))

public String cppFunc_symname(s)
Symbol s;
{
    assert(s->language == cppLang);
    if (s->class == FUNC || s->class == CSECTFUNC)
    {
	if (s->isMemberFunc)
	    return s->symvalue.funcv.u.memFuncSym->
		      symvalue.member.attrs.func.dName->fullName;
	else
	    return s->symvalue.funcv.u.dName->shortName;
    }
    else
    {
	assert(s->class == MEMBER);
	assert(s->symvalue.member.type == FUNCM);
	if (!s->symvalue.member.isCompGen)
	    return s->symvalue.member.attrs.func.dName->shortName;
	else
	    return ident(s->name);
    }
}

public symbol_dump (func)
Symbol func;
{
    register Symbol s;
    register integer i;

    (*rpt_output)(stdout, " symbols in %s \n",symname(func));
    for (i = 0; i < HASHTABLESIZE; i++) {
	for (s = hashtab[i]; s != nil; s = s->next_sym) {
	    if (s->block == func) {
		psym(s);
	    }
	}
    }
}

/*
 * Free all the symbols currently allocated.
 */

public symbol_free ()
{
    Sympool s, t;
    register Integer i;

    s = sympool;
    while (s != nil) {
	t = s->prevpool;
	dispose(s);
	s = t;
    }
    for (i = 0; i < HASHTABLESIZE; i++) {
	hashtab[i] = nil;
    }
    sympool = nil;
    nleft = 0;
}

/*
 * Create a new symbol with the given attributes.
 */

public Symbol newSymbol (name, blevel, class, type, chain)
Name name;
Integer blevel;
Symclass class;
Symbol type;
Symbol chain;
{
    register Symbol s;

    symbol_alloc(s);
    s->name = name;
    s->language = primlang;
    s->storage = EXT;
    s->level = blevel;
    s->class = class;
    s->type = type;
    s->chain = chain;
    if ((class == RANGE) || (class == PACKRANGE))
      /*  initialize the size field to zero.  There are some
            cases where it will never be reset, such as pascal
            strings.  For "integer" types, this field will be
            initialized  */
      /*  since this is a union, it is inappropriate to 
            set this field for anything except RANGE and PACKRANGE  */
      s->symvalue.rangev.size = 0;
    return s;
}

/*
 * Create a symbol with the given name and insert it into the hash table.
 */

public Symbol insert (name)
Name name;
{
    Symbol s;
    unsigned int h;

    h = hash(name);
    symbol_alloc(s);
    s->name = name;
    s->next_sym = hashtab[h];
    hashtab[h] = s;
    return s;
}

/*
 * Symbol lookup.
 */

public Symbol lookup(name)
Name name;
{
    register Symbol s;
    register unsigned int h;

    h = hash(name);
    s = hashtab[h];
    while (s != nil and s->name != name) {
	s = s->next_sym;
    }
    return s;
}

/*
 * Delete a symbol from the symbol table.
 */

public delete (s)
Symbol s;
{
    register Symbol t;
    register unsigned int h;

    h = hash(s->name);
    t = hashtab[h];
    if (t == nil) {
	panic( catgets(scmc_catd, MS_symbols, MSG_361,
				     "delete of non-symbol '%s'"), symname(s));
    } else if (t == s) {
	hashtab[h] = s->next_sym;
    } else {
	while (t->next_sym != s) {
	    t = t->next_sym;
	    if (t == nil) {
		panic( catgets(scmc_catd, MS_symbols, MSG_361,
				     "delete of non-symbol '%s'"), symname(s));
	    }
	}
	t->next_sym = s->next_sym;
    }
}

/*
 * Dump out all the variables associated with the given
 * procedure, function, or program associated with the given stack frame.
 *
 * This is quite inefficient.  We traverse the entire symbol table
 * each time we're called.  The assumption is that this routine
 * won't be called frequently enough to merit improved performance.
 */

extern boolean dumpvarsFirstLine; 

public dumpvars (f, frame)
Symbol f;
Frame frame;
{
    register Integer i;
    register Symbol s;

    for (i = 0; i < HASHTABLESIZE; i++) {
	for (s = hashtab[i]; s != nil; s = s->next_sym) {
	    if (container(s) == f) {
		if (should_print(s)) {
		    dumpvarsFirstLine = true;
		    printv(s, frame, true);
		    dumpvarsFirstLine = false;
		    (*rpt_output)(stdout, "\n" );
		} else if (s->class == MODULE) {
		    dumpvars(s, frame);
		}
	    }
	}
    }
}

/* 
 * New for C++, we want to run thru the entire hash table looking for
 * classes which have data and function members whose names match the
 * search name.
 */

private boolean cpp_searchClass(report, f, tag, class, name)
int (*report)();
File f;
Symbol tag, class;
Name name;
{
    boolean foundSymbol = false;
    Symbol member = class->chain;

    assert(class->class == CLASS);
    cpp_touchClass(class);

    while (member != nil) 
    {
	switch (member->class)
	{
	    case MEMBER:
	    case CONST:
		if (member->name == name && 
		    (member->class == CONST ||
		     (member->symvalue.member.type != DATAM ||
		      member->symvalue.member.isStatic)))
		{
		    foundSymbol = true;
		    printwhich(report, f, tag, false);
		    (*report)(f, "::%s\n", symname(member));
		}
		break;

	    case NESTEDCLASS:
		if (member->name == name)
		{
		    foundSymbol = true;
		    printwhich(report, f, tag, false);
		    (*report)(f, "::%s\n", symname(member));
		}
		if (rtype(member)->class == CLASS && 
		    !cpp_tempname(member->name))
		{
                    Symbol s = forward(member->type);
		    if (cpp_searchClass(report, f, s, rtype(s), name))
		        foundSymbol = true;
		}
		break;
	}
	member = member->next_sym;
    }
    return foundSymbol;
}

public boolean cpp_whereis (report, f, name)
int (*report)();
File f;
Name name;
{
    register unsigned h;
    register Symbol class, s;
    boolean foundSymbol = false;

    for (h = 0; h < HASHTABLESIZE; ++h) 
    {
        s = hashtab[h];
	while (s != nil) 
	{
	    if (s->class == TAG && !s->isClassTemplate &&
	        (class = rtype(s))->class == CLASS && !cpp_tempname(s->name))
	    {
		if (cpp_searchClass(report, f, s, class, name))
		    foundSymbol = true;
	    }
	    s = s->next_sym;
	}
    }
    return foundSymbol;
}

/*
 * Delete all of the variables associated with the given
 * module.  This is extremely inefficient, but is necessary
 * in order to remove symbols from unloaded modules.
 * To make this more efficient, one should probably redesign the
 * allocation sheme to be done on a per-module basis rather than
 * in a global hash table.
 */

public delete_unloaded_syms (ldndx, prev_info)
int ldndx;
struct ldinfo *prev_info;
{
    register Integer i;
    register Symbol s, t, basesym;
    Address text_base, text_end, data_base, data_end;
    Address rootaddr;
    Boolean foundblock;

    text_base = prev_info[ldndx].textorg;
    text_end = text_base + prev_info[ldndx].textsize;
    data_base = prev_info[ldndx].dataorg;
    data_end = data_base + prev_info[ldndx].datasize;
    for (i = 0; i < HASHTABLESIZE; i++) {
	for (s = hashtab[i]; s != nil; s = s->next_sym) {
	    basesym = t = s;
	    while ((!(foundblock = (Boolean) isblock(t))) &&
	    	   (!((t->block == nil) || (t->block->class == PROG))))
			t = t->block;
	    if (foundblock) {
		rootaddr = t->symvalue.funcv.beginaddr;
	    } else {
		rootaddr = 0;
	    }
	    if (((rootaddr >= text_base) && (rootaddr <= text_end)) ||
		((rootaddr >= data_base) && (rootaddr <= data_end))) {
		/* Delete the symbol. */
		if (basesym == hashtab[i]) {
		    hashtab[i] = s->next_sym;
    		} else {
		    t = hashtab[i];
		    while (t->next_sym != basesym) {
	    		t = t->next_sym;
			if (t == nil) 
				break;
		    }
		    if (t != nil) {
			t->next_sym = basesym->next_sym;
		    }
	        }
	    }
        }
    }
}


/*
 * NAME: isglobal
 *
 * FUNCTION: Determines if a symbol is an external variable
 *
 * PARAMETERS:
 *      s       - Symbol to check
 *
 * RETURNS: True if symbol is an external variable; False otherwise
 */
public Boolean isglobal(s)
Symbol s;
{
    return (Boolean) ( s->block &&
			( s->block->class == MODULE || s->block->class == PROG )
			&& (s->class == VAR || s->class == TOCVAR) &&
			s->level == program->level );
}

/*
 * NAME: iscobolglobal
 *
 * FUNCTION: Determines if a symbol is an external cobol variable
 *
 * PARAMETERS:
 *      s       - Symbol to check
 *
 * RETURNS: True if symbol is an external variable; False otherwise
 */
public Boolean iscobolglobal(s)
Symbol s;
{
    return (Boolean) ( s->block && s->block->class == MODULE &&
                     ( s->class == VAR || s->class == TOCVAR ||
                       s->class == REFFIELD ));
}

/*
 * NAME: isfilestatic
 *
 * FUNCTION: Determines if a symbol is a static symbol in specified module
 *
 * PARAMETERS:
 *      f       - Module the symbol needs to be in
 *      s       - Symbol to check
 *
 * RETURNS: True if symbol is static symbol in specified module; False otherwise
 */
public Boolean isfilestatic(f, s)
Symbol f;
Symbol s;
{
    return (Boolean) ( s->block && s->block == f &&
		      s->block->class == MODULE &&
		      (s->class == VAR || s->class == TOCVAR) &&
		      s->level != program->level );
}

/*
 * NAME: islocal
 *
 * FUNCTION: Determines if a symbol is a local variable in a module
 *
 * NOTES: Parameters are returned false by this function, since they are not
 *        handled the same way by different languages, they need to be
 *        collected separately.
 *
 * PARAMETERS:
 *      f       - Module the symbol should be in
 *      s       - Symbol to check
 *
 * RETURNS: True if symbol is a local variable in module; False otherwise
 */
public Boolean islocal(f, s)
Symbol f;
Symbol s;
{
    char *name;

    if (s->block && s->block == f && !s->param &&
	    ( s->class == VAR || s->class == TOCVAR || s->class == FVAR
            || s->class == FPTR || s->class == FPTEE
	    || (s->class == CONST &&
		( s->type->class == TYPE || s->type->class == ARRAY ||
		  s->type->class == FPTR || s->type->class == FPTEE )))) {
	name = symname(s);
        /*
         * If the name begins with a '.', then it is a parameter in FORTRAN.
         * s->param is not set for FORTRAN parameters.
         */
	if (name[0] != '.') return true;
    }
    return false;
}


/*
 * NAME: get_functions_list
 *
 * FUNCTION: Gets all functions defined in the program.
 *
 * PARAMETERS:
 *      symarr          - Functions found are returned here
 *      numfuncs        - Number of functions found is returned here
 *      file_lookup     - If set it is the function to use to check if a
 *                        function is defined in a specified list of files.  If
 *                        this is not set then all functions are returned.
 *
 * RETURNS: NONE
 */
public void get_functions_list(symarr, numfuncs, file_lookup)
Symbol **symarr;
int *numfuncs;
#ifdef _NO_PROTO
int (*file_lookup)();
#else
int (*file_lookup)( char *, struct SUFLIST * );
#endif
{
    int i;
    int arrsiz;	/* Amount of allocated space for symarr */
    extern AddrOfFunc *functab;
    extern int nfuncs;

    if( file_lookup ) {
	/*
         * Look through the function table for all functions
         * that are in the specified files; Count them in arrsiz;
         * arrsiz is initialized to 1 to account for the NULL
	 */
        for (i = 0, arrsiz = 1; i < nfuncs; i++) {
            /*
             * Make sure there is a block before adding this to the list;
             * Also make sure this is not an unloaded function, unloaded
             * functions will have an address of -1
             */
            if ( functab[i].func->block && functab[i].addr != (Address) -1 &&
		 file_lookup( functab[i].func->block->name->identifier,
                              functab[i].func->language->suflist ) ) {
                arrsiz++;
            }
        }
    } else {
        /*
         * Look through the function table for all functions; Count them
         * in arrsiz; arrsiz is initialized to 1 to account for the NULL
         */
        for (i = 0, arrsiz = 1; i < nfuncs; i++) {
            /*
             * Make sure there is a block before adding this to the list
             * Also make sure this is not an unloaded function, unloaded
             * functions will have an address of -1
             */
            if ( functab[i].func->block && functab[i].addr != (Address) -1 ) {
                arrsiz++;
            }
        }
    }
    *symarr = (Symbol *) malloc ( arrsiz * sizeof(Symbol) );
    /* Initialize the number of variables to zero */
    *numfuncs = 0;

    if( file_lookup ) {
        /*
         * Look through the function table for all functions
         * that are in the specified files; Put them into the symarr list
         */
        for (i = 0; i < nfuncs; i++) {
            /*
             * Make sure there is a block before adding this to the list
             * Also make sure this is not an unloaded function, unloaded
             * functions will have an address of -1
             */
            if ( functab[i].func->block && functab[i].addr != (Address) -1 &&
                 file_lookup( functab[i].func->block->name->identifier,
                              functab[i].func->language->suflist ) ) {
                (*symarr)[(*numfuncs)++] = functab[i].func;
            }
        }
    } else {
        /*
         * Look through the function table for all functions;
         * Put them into the symarr list
         */
        for (i = 0; i < nfuncs; i++) {
            /*
             * Make sure there is a block before adding this to the list
             * Also make sure this is not an unloaded function, unloaded
             * functions will have an address of -1
	     *
	     * The last check here is for dummy functions with "" as the name.
	     * These are being produced and we are not sure if they are needed
	     * for anything within dbx, but we don't want them showing up in the
	     * function list.  To save the time, we don't check for these when
	     * counting the number of functions in the list.
             */
            if ( functab[i].func->block && functab[i].addr != (Address) -1 &&
		 *functab[i].func->name->identifier != '\0' ) {
                (*symarr)[(*numfuncs)++] = functab[i].func;
            }
        }
    }
    (*symarr)[*numfuncs] = NULL;
}


/*
 * NAME: get_externs_list
 *
 * FUNCTION: Gets all external variables defined in a specified file list, and
 *    get all externals defined in the loaded program.
 *
 * PARAMETERS:
 *    symarr          - External variables found in specified file list are
 *                      returned here
 *    numvars         - Number of external variables found in specified file
 *                      list is returned here
 *    file_lookup     - Function to use to check if a variable is defined in a
 *                      specified list of files.
 *    all_symarr      - All external variables found are returned here
 *    all_list_num    - Number of all external variables found is returned
 *                      here
 *
 * RETURNS: NONE
 */
public void get_externs_list(symarr, numvars, file_lookup, all_symarr,
			     all_list_num)
Symbol **symarr;
int *numvars;
#ifdef _NO_PROTO
int (*file_lookup)();
#else
int (*file_lookup)( char *, struct SUFLIST * );
#endif
Symbol **all_symarr;
int *all_list_num;
{
    Symbol cur_sym;
    int i;
    int arrsiz = AVG_NO_EXTERN;		/* Amount allocated space for symarr */
    int all_arrsiz = AVG_NO_ALL_EXTERN;	/* Amount allocated space for symarr */

    *symarr = (Symbol *) malloc ( arrsiz * sizeof(Symbol) );
    *all_symarr = (Symbol *) malloc ( all_arrsiz * sizeof(Symbol) );
    /* Initialize the number of variables to zero */
    (*numvars) = 0;
    *all_list_num = 0;

    /* Look through the symbol table for all external variables. */
    for (i = 0; i < HASHTABLESIZE; i++) {
        for (cur_sym = hashtab[i]; cur_sym != NULL;
             cur_sym = cur_sym->next_sym) {
            /*
             * Check if the variable is external
             */
             if ( isglobal( cur_sym )) {
                /* This is an external variable - add it to all list */
                if ( *all_list_num == all_arrsiz ) {
                    all_arrsiz += AVG_NO_ALL_EXTERN;
                    *all_symarr = (Symbol *) realloc(*all_symarr,
                                                   all_arrsiz * sizeof(Symbol));
                }
                (*all_symarr)[(*all_list_num)++] = cur_sym;

                /*
                 * For COBOL variables make a separate list of variables whose
                 * type is FILET.  If the variable is of type
                 * ARRAY/GROUP/RGROUP, then create a symbol entry for each
                 * member of the group
                 */
                if( islang_cobol( cur_sym->language ) &&
                    ( cur_sym->type->class == GROUP ||
                      cur_sym->type->class == RGROUP ||
                      cur_sym->type->class == ARRAY )) {
                     process_cbl_grp(all_symarr, cur_sym, all_list_num,
                                     &all_arrsiz);
                 }

                 /*
                  * Now check if the variable is defined in a set of files
                  */
                 if ( islang_cobol( cur_sym->language )) {
                     if ( !file_lookup( cur_sym->block->name->identifier,
                                        cur_sym->language->suflist ))
                          continue;
                 } else if ( !file_lookup( cur_sym->block->name->identifier,
                                          cur_sym->block->language->suflist )) {
                          continue;
                 }

		if( cur_sym->name->identifier &&
		    *cur_sym->name->identifier == '_' ) {
		    /*
		     * This is a compiler generated symbol, don't want in list
		     */
		    continue;
		}

                /* This external variable is in our specified list of files */
                if ( *numvars == arrsiz ) {
                    arrsiz += AVG_NO_EXTERN;
                    *symarr = (Symbol *) realloc(*symarr,
                                                 arrsiz * sizeof(Symbol));
                }
                (*symarr)[(*numvars)++] = cur_sym;

                /*
                 * For COBOL variables make a separate list of variables whose
                 * type is FILET.  If the variable is of type
                 * ARRAY/GROUP/RGROUP, then create a symbol entry for each
                 * member of the group
                 */
                if( islang_cobol( cur_sym->language ) &&
                    ( cur_sym->type->class == GROUP ||
                      cur_sym->type->class == RGROUP ||
                      cur_sym->type->class == ARRAY )) {
                     process_cbl_grp(symarr,cur_sym, numvars,&arrsiz);
                }
            }
        }
    }
    /*
     * Null terminate the lists
     */
    if ( *numvars == arrsiz ) {
      arrsiz++;
      *symarr = (Symbol *) realloc(*symarr, arrsiz * sizeof(Symbol));
    }
    (*symarr)[*numvars] = NULL;

    if( *all_list_num == all_arrsiz ) {
      all_arrsiz++;
      *all_symarr = (Symbol *)realloc(*all_symarr,
                                      all_arrsiz * sizeof(Symbol));
    }
    (*all_symarr)[*all_list_num] = NULL;
  }

/*
 * NAME: dump_externs
 *
 * FUNCTION: Retrieves all external and static variables of a particular file
 *
 * PARAMETERS:
 *      filename        - Not used; Left for historic purposes
 *      modulesym       - Symbol of file to get static variables from
 *      numvars         - Set to the number of variables returned
 *      file_lookup     - Function to use to check if the external variable
 *                        is defined in one of the specified files
 *
 * RETURNS: Array of Symbol's of the external and static variables
 */
Symbol *dump_externs(filename, modulesym, numvars, file_lookup)
char *filename;
Symbol modulesym;
int *numvars;
#ifdef _NO_PROTO
int (*file_lookup)();
#else
int (*file_lookup)( char *, struct SUFLIST * );
#endif
{
    Symbol symbl;
    Symbol *hptr, *hend, *symarr;
    int arrsiz; /* Amount of allocated space for symarr */

    arrsiz = AVG_NO_EXTERN;
    symarr = (Symbol *) malloc( arrsiz * sizeof( Symbol ));

    /*
     * Initialize the number of variables to zero
     */
    *numvars = 0;

    /*
     * Look through the symbol table for all external variables defined in the
     * specified files, and all file static variables defined in this file
     */
    for( hptr = hashtab, hend = hashtab + HASHTABLESIZE; hptr < hend; hptr++ ) {
        for( symbl = *hptr; symbl; symbl = symbl->next_sym ) {
            if( ( isglobal( symbl ) &&
                  file_lookup( symbl->block->name->identifier,
                               symbl->language->suflist ))
                || isfilestatic( modulesym, symbl )) {
                if( *numvars == arrsiz ) {
                    arrsiz += AVG_NO_EXTERN;
                    symarr = (Symbol *) realloc( symarr,
                                                 arrsiz * sizeof( Symbol ));
                }
                symarr[*numvars] = symbl;
                ++*numvars;
            }
        }
    }
    if( *numvars == arrsiz ) {
        arrsiz++;
        symarr = (Symbol *) realloc( symarr, arrsiz * sizeof( Symbol ));
    }
    symarr[*numvars] = NULL;

    return( symarr );
}

/*
 * NAME: process_cbl_grp
 *
 * FUNCTION: Recursively called to get variable children of a COBOL group
 *
 * PARAMETERS:
 *      symarr  - Variables are added into this array
 *      cur_sym - Top level variable whose children are obtained
 *      numvars - When entering and leaving this function, set to current number
 *                of variables in symarr.
 *      size    - When entering and leaving this function, indicates amount of
 *                memory allocated for symarr.
 *
 * RETURNS: NONE
 */
void process_cbl_grp(symarr, cur_sym, numvars, size)
Symbol ** symarr;
Symbol  cur_sym;
int 	*numvars;
int  	*size;
{
	Symbol sym;

	if ( ! cur_sym ) return;
	if ( cur_sym->type->class == ARRAY )
	   sym = cur_sym->type->type->chain;
	else
	   sym = cur_sym->type->chain;
	while ( sym ) {
	   if ( not streq( symname(sym), "FILLER") )  {
	      if ( *numvars == *size ) {
		*size += AVG_NO_EXTERN;
		*symarr = (Symbol *) realloc(*symarr, *size * sizeof(Symbol));
	      }
	      (*symarr)[(*numvars)++] = sym;
	      if ( sym->type->class == GROUP || sym->type->class == RGROUP ||
		   sym->type->class == ARRAY ) {
		process_cbl_grp( symarr, sym, numvars, size );
	      }
	   }
	   sym = sym->chain;
	}
}

/*
 * NAME: get_file_static_list
 *
 * FUNCTION: Gets all the file level static variables for a specified file.
 *
 * PARAMETERS:
 *      file_sym        - File we are interested in
 *      symarr          - Variables are returned here.
 *      numvars         - Number of variables found is returned here.
 *
 * RETURNS: NONE
 */
public void get_file_static_list(file_sym, symarr, numvars)
Symbol file_sym;
Symbol **symarr;
int *numvars;
{
    Symbol cur_sym;
    int i, arrsiz = AVG_NO_STATIC;

    *symarr = (Symbol *) malloc ( arrsiz * sizeof(Symbol) );
    *numvars = 0; /* Initialize the number of variables to zero */

    /* Look through the symbol table for all static variables in this file. */
    for (i = 0; i < HASHTABLESIZE; i++) {
    	for (cur_sym = hashtab[i]; cur_sym != NULL;
    	     cur_sym = cur_sym->next_sym) {
    	    if ( isfilestatic( file_sym, cur_sym ) ) {
#ifdef ADCDEBUG
		printf( "symbol\t%s\n", symname( cur_sym ) );
		printf( "block\t%s\t%d\n", symname( cur_sym->block ),
			cur_sym->block->class );
#endif
		/* Found a static in this file, add it to the list */
		if ( *numvars == arrsiz ) {
		    arrsiz += AVG_NO_STATIC;
		    *symarr = (Symbol *) realloc(*symarr,
						arrsiz * sizeof(Symbol));
		}
		(*symarr)[(*numvars)++] = cur_sym;
    	    }
    	}
    }
    if ( *numvars == arrsiz ) {
        arrsiz++;
        *symarr = (Symbol *) realloc(*symarr, arrsiz * sizeof(Symbol));
    }
    (*symarr)[*numvars] = NULL;
}


/*
 * NAME: dumpfuncsyms
 *
 * FUNCTION: Gets all the local variables for a particular block
 *
 * PARAMETERS:
 *      f       - Name of block to get variables from
 *      symarr  - Variables are returned here.
 *      numvars - Number of variables found is returned here.
 *      arrsiz  - Amount of memory allocated for symarr.
 *
 * RETURNS: NONE
 */
public void dumpfuncsyms(f, symarr, numvars, arrsiz)
Symbol f;		
Symbol  **symarr;
int *numvars;
int *arrsiz;
{
    register Integer i;
    register Symbol s;

    /*
     * For unallocated storage, allocate space to accomodate
     * the average number of local variables per block.
     */
    if ( !*symarr ) {
    	*arrsiz = AVG_NO_LOCAL;
    	*symarr = (Symbol *) malloc( *arrsiz * sizeof(Symbol) );
    	*numvars = 0;
    }

    /*
     * For every symbol...
     *    If the symbol is local to the specified block and if the symbol is
     *	  not a compiler produced variable starting with '#'
     *       add the symbol to the allocated array to be returned.
     *       Reallocate dynamically if more storage is required.
     */
    for (i = 0; i < HASHTABLESIZE; i++) {
	for (s = hashtab[i]; s != nil; s = s->next_sym) {
	    if (islocal( f, s ) && *(symname(s)) != '#' ) {
		if ( *numvars == *arrsiz ) {
		    *arrsiz += AVG_NO_LOCAL;
		    *symarr = (Symbol *)
		    	realloc(*symarr, *arrsiz * sizeof(Symbol));
		}
                (*symarr)[(*numvars)++] = s;
	    }
	}
    }

    /*
     * Add the arguments to procedure 'f'
     * to the allocated array to be returned.
     * Reallocate dynamically if more storage is required.
     */
    s = f->chain;
    while ( s ) {
        if ( *numvars == *arrsiz ) {
            *arrsiz += AVG_NO_LOCAL;
            *symarr = (Symbol *) realloc(*symarr, *arrsiz * sizeof(Symbol));
        }
        (*symarr)[(*numvars)++] = s;
        s = s->chain;
    }
    if ( *numvars == *arrsiz ) {
        (*arrsiz)++;
        *symarr = (Symbol *) realloc(*symarr, *arrsiz * sizeof(Symbol));
    }
    (*symarr)[*numvars] = nil;
}


/*
 * NAME: get_all_local
 *
 * FUNCTION: Gets all the variables in the local scope, plus any variables that
 *           are in the local scopes we are nested in.
 *
 * NOTES: Uses dumpfuncsyms() to find the variables in each scope.
 *
 * PARAMETERS:
 *      modulesym       - Block we want the symbols from
 *      symarr          - Variables are returned here
 *      numvars         - Number of variables found is returned here
 *
 * RETURNS: NONE
 */
void get_all_local(modulesym, symarr, numvars)
Symbol modulesym;
Symbol **symarr;
int *numvars;
{
    Symbol *comb_symarr = NULL; /* Combined symbol array */
    Symbol cur_module;
    int arrsiz;

    /* Call dumpfuncsyms to get the local variables from the immediate scope */
    dumpfuncsyms( modulesym, &comb_symarr, numvars, &arrsiz );

    /* Now we will look in the scope we are nested in */
    cur_module = modulesym->block;

    while ( cur_module  && cur_module->class != MODULE &&
            cur_module->class != BADUSE ) {
        /* Get local variables from our previous scope. Use the same
         * memory for each call to dumpfuncsyms(), at the end of this
         * loop we will free the memory allocated by dumpfuncsyms()
         */
        dumpfuncsyms( cur_module, &comb_symarr, numvars, &arrsiz );

        /* Move the scope up one level */
        cur_module = cur_module->block;
    }
    /* Return the combined list */
    *symarr = comb_symarr;
}

/*
 * NAME: dpi_lookup
 *
 * FUNCTION: Returns the Symbol for the specified variable in the specified
 *           scope and with the specified block
 *
 * PARAMETERS:
 *      variable        - Name of variable
 *      block_name      - DBX style whereis information for variable
 *      scope           - Which variable list this variable came from
 *
 * RETURNS: Symbol that matches the variable specified or nil if no
 *	match is found.
 */
void  *dpi_lookup( variable, block_name, scope )
char                *variable;
char                *block_name;
VariableListType    scope;
{
    Name                     var, block;
    register Symbol          s, bsym;
    register unsigned int    h;
    Symbol                   parameter;
    char		     *param_block = NULL;

    /*
     * Find the block (immediate scope) symbol.  Search for name without
     * parameter information if there is any.
     */
    param_block = strchr( block_name, '(' );
    if( param_block ) {
	*param_block = '\0';
    }
    block = identname( block_name, false );
    if( param_block ) {
	*param_block = '(';
    }

    if ( scope == FILE_STATICS ) {
        bsym = lookup( block );
        while( bsym != nil &&
	      !(bsym->name == block && bsym->class == MODULE )) {
            bsym = bsym->next_sym;
        }
	if (bsym == nil)
	    return nil;
    }
    else if ( scope == LOCAL_AND_NESTED ) {
        bsym = lookup( block );
	/*
	 * Find the corresponding function in dbx.  If the function name has
	 * parameter information use that information to do the search otherwise
	 * just match on the name
	 */
	if( param_block ) {
	    while( bsym != nil &&
		  !(isroutine( bsym ) && streq( block_name, symname(bsym)))) {
		bsym = bsym->next_sym;
	    }
	} else {
	    while( bsym != nil &&
		  !(bsym->name == block && isroutine( bsym ) )) {
		bsym = bsym->next_sym;
	    }
	}
	if (bsym == nil)
	    return nil;
    }

    var = identname( variable, false );
    h = hash( var );
    do {
	for ( s = hashtab[ h ]; s; s = s -> next_sym ) {
	    if ( s -> name == var ) {
		if (scope == PROGRAM_EXTERNALS && islang_cobol(s->language))
		{
		    if ( iscobolglobal( s ) && ( s->block->name == block ))
			break;
		} else if ( scope == PROGRAM_EXTERNALS ) {
		    if ( isglobal( s ) && ( s -> block -> name == block ))
			break;
		} else if ( scope == FILE_STATICS ) {
		    if ( isfilestatic( bsym, s ))
			break;
		} else if ( scope == LOCAL_AND_NESTED ) {
		    if ( islocal( bsym, s ))
			break;
		    parameter = bsym->chain;
		    while( parameter ) {
			if( parameter == s )
			    break;
			parameter = parameter->chain;
		    }
		    if( parameter )
			break;
		}
	    }
	}
    } while( s == NULL && scope == LOCAL_AND_NESTED && bsym &&
	    (bsym = bsym->next_sym) );
    return(( void * )s );
}


/*
 * NAME: dpi_which
 *
 * FUNCTION: Returns symbol matching the variable in the specified scope
 *
 * PARAMETERS:
 *      var             - Name structure of variable to find
 *      qualification   - DBX style block information for variable to find
 *
 * RETURNS: Symbol matching the specified qualification
 */
Symbol  dpi_which( var, qualification )
Name    var;
char    *qualification;
{

   char                     *scope;
   int                      length;
   register Symbol          s;
   register unsigned int    h;

   /*----------------------+
   | Hash on the variable. |
   +----------------------*/
   h = hash( var );
   for ( s = hashtab[ h ]; s; s = s -> next_sym ) {
        if ( s -> name == var ) {
            /*----------------------------------------+
            | Determine the full scope of the symbol. |
            +----------------------------------------*/
            length = 0;
            dpi_var_scope( s, &scope, &length );

            /*
             * Since dpi_var_scope() leaves a trailing '.' at the end of the
             * scope, remove it before the comparison
             */
            scope[ length - 1 ] = '\0';

            /*-----------------------------------------------------------------+
            | Check whether the symbol's scope matches the specified           |
            | qualification.                                                   |
            +-----------------------------------------------------------------*/
            /*
             * If qualification does not start with a leading '.' don't compare
             * to first character of full scope name
             */
            if( *qualification != '.' ) {
                if ( streq( scope + 1, qualification ) )
                    break;
            } else {
                if ( streq( scope, qualification ) )
                    break;
            }
        }
   }
   return( s );
}

/*
 * NAME: dpi_find_symbol_for_variable
 *
 * FUNCTION: Performs a specialized which() for ADC
 *
 * PARAMETERS:
 *      variable        - Name of variable to find
 *      qualified       - DBX style scope of variable, should be NULL if none
 *                        is specified
 *      scope           - Set to the variable list the variable comes from
 *      file            - Set to the file the variable is in
 *      function        - Set to the function the variable is in
 *      full_scope      - Set to the full scope information for the variable
 *
 * RETURNS: scoping information as well as the symbol for the specified variable
 */
void  *dpi_find_symbol_for_variable( variable, qualified, scope,
				     file, function, full_scope )
char                *variable;
char                *qualified;
VariableListType    *scope;
char                **file;
char                **function;
char                **full_scope;
{

   Name                     var;
   Symbol	          s, bsym;
   int			    length = 0;
   jmp_buf                  env;
   int                      *svenv;
   Symbol                   parameter;

   /*----------------------------------------------------+
   | Save the environment in case a symbol is not found. |
   | ( dbx's error recovery executes a longjmp(). )      |
   +----------------------------------------------------*/
   svenv = envptr;
   envptr = env;
   switch ( setjmp( env )) {
       case ENVCONT:
       case ENVEXIT:
       case ENVQUIT:
       case ENVFATAL:
            envptr = svenv;
            return (( void * )NULL);
       default:
            break;
   } 

   if (*variable == '')
	sscanf(variable+sizeof(char), "%x", &s);
   else
   {

	/*
	 * Get the identifier folding input to case specified if necessary.
	 */
	var = dpi_get_ident( variable );

	/*-----------------------------------------------------------------+
	| For unqualified variables, utilize which() to find the symbol.   |
	| For qualified variables, utilize dpi_which() to find the symbol. |
	+-----------------------------------------------------------------*/
	if ( !qualified ) 
	{
	   Node n = which( var, WANY );
	   assert(n->op == O_SYM);
	   s = n->value.sym;
	}
	else
	   s = dpi_which( var, qualified );
       
   }
   if ( s ) {
       /*-----------------------------------+
       | Determine the scope of the symbol. |
       +-----------------------------------*/
       if ( islang_cobol( s->language ) && iscobolglobal( s ))
           *scope = PROGRAM_EXTERNALS;
       else if ( isglobal( s ))
           *scope = PROGRAM_EXTERNALS;
       else if ( isfilestatic( s -> block, s ))
           *scope = FILE_STATICS;
       else if ( islocal( s -> block, s ))
           *scope = LOCAL_AND_NESTED;
       else {
           *scope = UNKNOWN_LIST;

           /*
            * Since islocal() does not find parameters, we will check here to
            * see if this variable is a parameter.  If it is not then we don't
            * know what scope this variable belongs to.
            */
           parameter = s->block->chain;
           while( parameter ) {
                if( s == parameter ) {
                    *scope = LOCAL_AND_NESTED;
                    break;
                }
                parameter = parameter->chain;
           }
       }

       /*--------------------------------------------+
       | Determine the module containing the symbol. |
       +--------------------------------------------*/
       for ( bsym = s->block; bsym; bsym = bsym -> block ) {
	    if ( bsym -> class == MODULE )
                break;
       }
       if ( bsym ) {
           /*
            * Allocate enough space to hold the filename, the extension,
            * and NULL.  Since the longest extension used will be ".cbl"
            * add 5 to the length of the filename for the allocation
            */
           *file = malloc( (strlen( symname( bsym )) + 5) * sizeof( char ) );
           strcpy( *file, symname( bsym ));
           if ( bsym->language == cLang)
               strcat( *file, ".c" );
           else if ( bsym->language == cppLang)
               strcat( *file, ".C" );
           else if ( bsym->language == fLang)
               strcat( *file, ".f" );
           else if ( islang_cobol( bsym -> language ) ||
                     islang_cobol( s->language ))
               strcat( *file, ".cbl" );
       }
       else 
	   *file = NULL;

       /*----------------------------------------------------+
       | Determine the function/block containing the symbol. |
       +----------------------------------------------------*/
       for ( bsym = s -> block; bsym; bsym = bsym -> block ) {
	    if (( bsym -> class == FUNC ) || ( bsym -> class == PROC ))
                break;
       }
       if ( bsym ) {
           *function = malloc((strlen( symname( bsym )) + 1 ) * sizeof( char ));
	   strcpy( *function, symname( bsym ));
       }
       else 
	   *function = NULL;

       /*----------------------------------------+
       | Determine the full scope of the symbol. |
       +----------------------------------------*/
       dpi_var_scope( s, full_scope, &length );
       /*
        * Since dpi_var_scope() leaves a trailing '.' at the end of the
        * scope, remove it before the comparison
        */
       (*full_scope)[ length - 1 ] = '\0';
   }
   envptr = svenv;
   return(( void * )s );
}


/*
 * Create a builtin type.
 * Builtin types are circular in that btype->type->type = btype.
 */

private Symbol maketype (name, lower, upper)
String name;
long lower;
long upper;
{
    register Symbol s;
    Name n;

    if (name == nil) {
	n = nil;
    } else {
	n = identname(name, true);
    }
    s = insert(n);
    s->language = primlang;
    s->storage = EXT;
    s->level = 0;
    s->class = TYPE;
    s->type = nil;
    s->chain = nil;
    s->type = newSymbol(nil, 0, RANGE, s, nil);
    s->type->symvalue.rangev.lower = lower;
    s->type->symvalue.rangev.upper = upper;
    if (lower < 0)
      s->type->symvalue.rangev.is_unsigned = 0;
    else
      s->type->symvalue.rangev.is_unsigned = 1;
    s->type->symvalue.rangev.size =
                              getrangesize(s, (LongLong) lower,
                                           (uLongLong) upper);
    s->type->name = n;
    return s;
}

/*
 * Create the builtin symbols.
 */

public symbols_init ()
{
    Symbol s;

    t_boolean = maketype("$boolean", 0L, 1L);
    t_char = maketype("$char", 0L, 255L);
    t_int = maketype("$integer", 0x80000000L, 0x7fffffffL);

    t_longlong = maketype("$longlong", 0L, 0L);
    t_longlong->type->symvalue.rangev.size = sizeofLongLong;
    t_longlong->type->symvalue.rangev.is_unsigned = 0;

    t_ulonglong = maketype("$ulonglong", 0L, 0L);
    t_ulonglong->type->symvalue.rangev.size = sizeofLongLong;
    t_ulonglong->type->symvalue.rangev.is_unsigned = 1;

    t_gchar = maketype("$gchar", 0L, 65535L);
    t_real = insert(identname("$real", true));
    t_real->class = REAL;
    t_real->symvalue.size = 8;
    t_real->language = primlang;
    t_real->storage = EXT;
    t_real->level = 0;
    t_real->class = TYPE;
    t_real->type = newSymbol(nil,0,REAL,t_real,nil);
    t_real->type->symvalue.size = 8;
    t_real->chain = nil;

    t_quad = insert(identname("$quad", true));
    t_quad->class = REAL;
    t_quad->symvalue.size = 16;
    t_quad->language = primlang;
    t_quad->storage = EXT;
    t_quad->level = 0;
    t_quad->class = TYPE;
    t_quad->type = newSymbol(nil,0,REAL,t_real,nil);
    t_quad->type->symvalue.size = 16;
    t_quad->chain = nil;

    t_float = insert(identname("$float", true));
    t_float->class = REAL;
    t_float->symvalue.size = 4;
    t_float->language = primlang;
    t_float->storage = EXT;
    t_float->level = 0;
    t_float->class = TYPE;
    t_float->type = newSymbol(nil,0,REAL,t_float,nil);
    t_float->type->symvalue.size = 4;
    t_float->chain = nil;
    t_nil = maketype("$nil", 0L, 0L);
    t_addr = insert(identname("$address", true));
    t_addr->language = primlang;
    t_addr->level = 0;
    t_addr->class = TYPE;
    t_addr->type = newSymbol(nil, 1, PTR, t_int, nil);
    t_complex = insert(identname("$complex", true));
    t_complex->class = COMPLEX;
    t_complex->symvalue.size = 16;
    t_complex->language = primlang;
    t_complex->storage = EXT;
    t_complex->level = 0;
    t_complex->class = TYPE;
    t_complex->type = newSymbol(nil,0,COMPLEX,t_complex,nil);
    t_complex->type->symvalue.size = 16;
    t_complex->chain = nil;
    t_qcomplex = insert(identname("$qcomplex", true));
    t_qcomplex->class = COMPLEX;
    t_qcomplex->symvalue.size = 32;
    t_qcomplex->language = primlang;
    t_qcomplex->storage = EXT;
    t_qcomplex->level = 0;
    t_qcomplex->class = TYPE;
    t_qcomplex->type = newSymbol(nil,0,COMPLEX,t_qcomplex,nil);
    t_qcomplex->type->symvalue.size = 32;
    t_qcomplex->chain = nil;
    s = insert(identname("true", true));
    s->class = CONST;
    s->type = t_boolean;
    s->symvalue.constval = build(O_LCON, 1L);
    s->symvalue.constval->nodetype = t_boolean;
    s = insert(identname("false", true));
    s->class = CONST;
    s->type = t_boolean;
    s->symvalue.constval = build(O_LCON, 0L);
    s->symvalue.constval->nodetype = t_boolean;
    dt_ushort = maketype("$ushort", 0, 65535);
    dt_uint = maketype("$uinteger", 0, 4294967295);
#ifdef CMA_THREAD
    /* create types for CMA thread related objects */
    createCMAtypes();
#endif /* CMA_THREAD */
#ifdef K_THREADS
    /* create types for libpthreads thread related objects */
    create_pthreads_types();
#endif /* K_THREADS */
}

public Name typename (t)
Symbol t;
{
    Symbol type = forward(t->type);
    return (type == nil) ? nil : type->name;
}

/*
 * Reduce type to avoid worrying about type names.
 */

public Symbol rtype (type)
Symbol type;
{
    register Symbol t;

    t = type;
    if (t != nil) 
    {
        if (t->class == VAR or t->class == CONST or t->class == VTAG or
	    t->class == FIELD or t->class == REF or t->class == REFFIELD or
	    t->class == TOCVAR or 
	    t->class == BASECLASS or t->class == NESTEDCLASS or 
	    t->class == FRIENDFUNC or t->class == FRIENDCLASS or 
	    t->class == MEMBER) 
 	{
	    t = t->type;
	}

	if (t->class == TYPEREF) 
	    resolveRef(t);
	while (t->class == TYPE or t->class == TAG) 
	{
	    t = t->type;
	    if (t->class == TYPEREF) 
		resolveRef(t);
	}
    }
    return t;
}

/*
 * Find the end of a module name.  Return nil if there is none
 * in the given string.
 */

private String findModuleMark (s)
String s;
{
    register char *p, *r;
    register boolean done;

    p = s;
    done = false;
    do {
	if (*p == ':') {
	    done = true;
	    r = p;
	} else if (*p == '\0') {
	    done = true;
	    r = nil;
	} else {
	    ++p;
	}
    } while (not done);
    return r;
}

/*
 * Resolve a type reference by modifying to be the appropriate type.
 *
 * If the reference has a name, then it refers to an opaque type and
 * the actual type is directly accessible.  Otherwise, we must use
 * the type reference string, which is of the form "module:{module:}name".
 */

public resolveRef (t)
Symbol t;
{
    register char *p;
    char *start;
    Symbol s, m, outer;
    Name n;

    if (t->name != nil) {
	s = t;
    } else {
	start = t->symvalue.typeref;
	outer = program;
	p = findModuleMark(start);
	while (p != nil) {
	    *p = '\0';
	    n = identname(start, true);
	    find(m, n) where m->block == outer endfind(m);
	    if (m == nil) {
		p = nil;
		outer = nil;
		s = nil;
	    } else {
		outer = m;
		start = p + 1;
		p = findModuleMark(start);
	    }
	}
	if (outer != nil) {
	    n = identname(start, true);
	    find(s, n) where s->block == outer endfind(s);
	}
    }
    if (s != nil and s->type != nil) {
	t->name = s->type->name;
	t->class = s->type->class;
	t->type = s->type->type;
	t->chain = s->type->chain;
	t->symvalue = s->type->symvalue;
	t->block = s->type->block;
    }
}

public int regnum (s)
Symbol s;
{
    int r = -1;

    checkref(s);
    if (s->param) {
	r = preg(s, nil);
	if (r == -1 && s->storage == INREG) {
	    r = s->symvalue.offset;
	}
    } else if (s->storage == INREG) {
	r = s->symvalue.offset;
    }
    return r;
}

public Symbol container (s)
Symbol s;
{
    checkref(s);
    return s->block;
}

public Node constval (s)
Symbol s;
{
    checkref(s);
    /* Everywhere that constval is called, isconst is called first,
       and isconst() merely checks for s->class == CONST, so this can never
       happen.
       if (s->class != CONST) {
	   error( "[internal error: constval(non-CONST)]");
       }
    */
    return s->symvalue.constval;
}

/*
 * Return the object address of the given symbol.
 *
 * There are the following possibilities:
 *
 *	globals		- just take offset
 *	locals		- take offset from locals base
 *	arguments	- take offset from argument base
 *	register	- offset is register number
 */

extern Boolean call_command;

public Address address (s, frame)
Symbol s;
Frame frame;
{
    Frame frp;
    Address addr;
    register Symbol cur;
    int r;
    Boolean isCall = false;
    extern Boolean processed_partial_core;
    extern Boolean coredump;

    checkref(s);
    addr = s->symvalue.offset;
    if (s->storage != EXT) {
        if (not isactive(s->block)) {
	    error( catgets(scmc_catd, MS_symbols, MSG_367,
			       "\"%s\" is not currently defined"), symname(s));
	}
	frp = frame;
	if (frp == nil) {
	    cur = s->block;
	    while (cur != nil and cur->class == MODULE) {
		cur = cur->block;
	    }
	    if (cur == nil) {
		frp = nil;
	    } else {
		/* Needs to turn off call_command before calling findframe */
		isCall = call_command;
		call_command = false;
		frp = findframe(cur);
		call_command = isCall;
		if (frp == nil) {
		    error( catgets(scmc_catd, MS_symbols, MSG_368,
			  "[internal error: unexpected nil frame for \"%s\"]"),
			symname(s)
		    );
		}
	    }
	}
	if (s->param) {
	    r = preg(s, frp);
	    if (r != -1) {
		addr = r;
	    } else if (s->storage == STK) {
	        addr += locals_base(frp);
/*
 *  The compilers will give us the correct address.  No need to adjust.
		if (multiword(s->block)) {
		    addr += sizeof(Word);
		}
*/
	    }
	} else if (s->storage == STK) {
	    addr += locals_base(frp);
	} else if (s->storage != INREG) {
	    panic( catgets(scmc_catd, MS_symbols, MSG_369,
				    "address: bad symbol \"%s\""), symname(s));
	}
        /* Need to dereference fortran pointers even if they are not EXT */
        if ((s->type != nil) && (s->type->class == PTR) &&
                (s->language == fLang))
           dread(&addr, addr, 4);  /* Don't make fortran users dereference */
    }
    else if (s->class == REFFIELD) { /* COBOL REFFIELD */
      if (s->symvalue.field.parent->type->class == PTR)
	/* Linkage items */
	dread(&addr, address(s->symvalue.field.parent, frame),
	      sizeof(Address));
      else
      	addr = address(s->symvalue.field.parent, frame);
      addr += (s->symvalue.field.offset / 8);
    } else if ((s->type != nil) && (s->type->class == PTR) &&
                (s->language == fLang))  {
      dread(&addr, addr, 4);	/* Don't make fortran users dereference */
    } else if ((s->class == TOCVAR) &&
	       (processed_partial_core) && (!coredump)) {
      dread(&s->symvalue.offset,s->symvalue.offset,4);
      addr = s->symvalue.offset;
      s->class = VAR;
    }
    return addr;
}

/*
 * Determine whether a function returns a multi-word value
 * whose address is passed as an invisible first argument.
 */

public boolean multiword (f)
Symbol f;
{
    register boolean r;
    register Symbol t;

    if (f == nil) {
	r = false;
    } else {
	t = rtype(f->type);
	r = (boolean) (t->class == RECORD || t->class == UNION);
    }
    return r;
}

/*
 * Define a symbol used to access register values.
 */

public defregname (n, r)
Name n;
int r;
{
    Symbol s;

    s = insert(n);
    s->language = t_addr->language;
    s->class = VAR;
    s->storage = INREG;
    s->level = 3;
    s->type = t_addr;
    s->symvalue.offset = r;
}

/*
 * Define a symbol used to access floating point register values.
 */

public deffregname (n, r)
Name n;
int r;
{
    Symbol s;

    s = insert(n);
    s->language = t_addr->language;
    s->class = VAR;
    s->storage = INREG;
    s->level = 3;
    s->type = t_real;
    s->symvalue.offset = r;
}

/*
 * Resolve an "abstract" type reference.
 *
 * It is possible in C to define a pointer to a type, but never define
 * the type in a particular source file.  Here we try to resolve
 * the type definition.  This is problematic, it is possible to
 * have multiple, different definitions for the same name type.
 */

public findtype (s)
Symbol s;
{
    register Symbol t, u, prev;

    u = s;
    prev = nil;
    while (u != nil and u->class != BADUSE) {
	if (u->name != nil) {
	    prev = u;
	}
	u = u->type;
    }
    if (prev == nil) {
	error( catgets(scmc_catd, MS_symbols, MSG_370,
				      "couldn't find link to type reference"));
    }
    t = lookup(prev->name);
    while (t != nil and
	not (
	    t != prev and t->name == prev->name and
	    t->block->class == MODULE and t->class == prev->class and
	    t->type != nil and t->type->type != nil and
	    t->type->type->class != BADUSE
	)
    ) {
	t = t->next_sym;
    }
    if (t == nil) {
	error( catgets(scmc_catd, MS_symbols, MSG_371,
						"couldn't resolve reference"));
    } else {
	prev->type = t->type;
    }
}

/*
 * NAME: getrangesize
 *
 * FUNCTION: determines the size of an "integer" type based  
 *           on its range of values.
 *
 * PARAMETERS:
 *        t     - Symbol
 *        lower - the lower bound
 *        upper - the upper bound
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: none   
 *
 * RETURNS: the size of the "integer" type
 *
 */

#define MAXUCHAR 255
#define MAXUSHORT 65535L
#define MINCHAR -128
#define MAXCHAR 127
#define MINSHORT -32768
#define MAXSHORT 32767

long getrangesize (t, lower, upper)
Symbol t;
LongLong lower;
uLongLong upper;
{
  LongLong r;

  if (upper == 0 and lower > 0) {
    /* real */
    r = lower;
  }
  else if ((lower >= MINCHAR && upper <= MAXCHAR) 
       || (lower >= 0 && upper <= MAXUCHAR))
  {
    r = sizeof(char);
  } 
  else if ((lower >= MINSHORT && upper <= MAXSHORT)
       || (lower >= 0 and upper <= MAXUSHORT))
  {
    r = sizeof(short);
  } 
  else if (t->class == PACKRANGE)
  {
    unsigned int range;
    r = 0;
    for (range = upper - lower; range != 0; r++) 
    {
      range >>= 8;
    }
  }
  else
    r = sizeof(long);

  return (r);
}

/*
 * Find the size in bytes of the given type.
 *
 * This is probably the WRONG thing to do.  The size should be kept
 * as an attribute in the symbol information as is done for structures
 * and fields.  I haven't gotten around to cleaning this up yet.
 */

public findbounds (u, lower, upper)
Symbol u;
long *lower, *upper;
{
    Rangetype lbt, ubt;
    long lb, ub;

    if (u->class == RANGE) {
	lbt = (Rangetype) u->symvalue.rangev.lowertype;
	ubt = (Rangetype) u->symvalue.rangev.uppertype;
	lb = u->symvalue.rangev.lower;
	ub = u->symvalue.rangev.upper;
        if (lbt != R_CONST && lbt != R_ADJUST) {
	    if (not getbound(u, lb, lbt, lower)) {
		error( catgets(scmc_catd, MS_symbols, MSG_372,
				    "dynamic bounds not currently available"));
	    }
	} else {
	    *lower = lb;
	}
        if (ubt != R_CONST) {
	    if (not getbound(u, ub, ubt, upper)) {
		error( catgets(scmc_catd, MS_symbols, MSG_372,
				    "dynamic bounds not currently available"));
	    }
	} else {
	    *upper = ub;
	}
    } else if (u->class == SCAL) {
	*lower = 0;
	*upper = u->symvalue.iconval - 1;
    } else {
	error( catgets(scmc_catd, MS_symbols, MSG_374,
			     "[internal error: unexpected array bound type]"));
    }
}



#define SETLEN 32

public integer size(sym)
Symbol sym;
{
    register Symbol t, u;
    register integer nel, elsize;
    long lower, upper;
    integer r, off, len;
    unsigned int range;
    extern Language cLang;
    Boolean is_active = true;

    t = sym;
    checkref(t);
    if (t->class == TYPEREF) {
	resolveRef(t);
    }

    if (is_f90_sym(t))
      t = convert_f90_sym(t, &is_active);

    switch (t->class) {
        case SPACE:
        case GSTRING:
	case STRING:
            r = t->symvalue.size;
            if (r == 0)
            {
              Address a;
              char *sizebuf;

              a = pop(Address);
              sizebuf = (char *) &r;
              dread(sizebuf+2, a, sizeof(short));         /* read len bytes */
              r = r+2;
            }
	    break;

	case PACKRANGE:
	case RANGE:
	    if ((t->class != PACKRANGE) &&
	      ((t->type->type->type != t)&&(t->type->type != t)))
	      r = size(t->type);
	    else
	    {
              if (t->symvalue.rangev.size)
              {
                r = t->symvalue.rangev.size;
              }
              else
                r = getrangesize (t, (LongLong) t->symvalue.rangev.lower,
                                     (uLongLong) t->symvalue.rangev.upper); 
            }
	    break;

	case FSTRING:
	    getbound(t, t->symvalue.multi.size, t->symvalue.multi.sizeloc, &r);
	    break;

	case CHARSPLAT:
	    r = t->symvalue.multi.size;
	    break;

	case REAL:
	case COMPLEX:
	    r = t->symvalue.size;
	    break;

	case PACKARRAY:
	case ARRAY:
	    elsize = size(t->type);
	    nel = 1;
	    for (t = t->chain; t != nil; t = t->chain) {
		u = rtype(t);
		findbounds(u, &lower, &upper);
		nel *= (upper-lower+1);
	    }
	    r = nel*elsize;
	    break;

	case SUBARRAY:
	    r = (2 * t->symvalue.ndims + 1) * sizeof(Word);
	    break;

	case VAR:
	case TOCVAR:
        case FPTEE:
	    r = size(t->type);
	    break;

        case ELLIPSES:
            r = 0;
            break;

	case FVAR:
	case CONST:
        case REF:
            if (t->param and rtype(t)->class == STRING 
                and (rtype(t)->symvalue.size == 0) )
            {
               Address str;
               str = address(t, nil);
               rpush(str, sizeof(Address));
            }
	case TAG:
	    r = size(t->type);
	    break;

	case TYPE:
	    /*
	     * This causes problems on the IRIS because of the compiler bug
	     * with stab offsets for parameters.  Not sure it's really
	     * necessary anyway.
	     */
#	    ifndef IRIS
	    if (t->type->class == PTR and t->type->type->class == BADUSE) {
		findtype(t);
	    }
#	    endif
	    /* Is this a padded type in fortran when AUTODBL?? */
	    if (is_fortran_padded(t))
		r = t->symvalue.size;
	    else
	        r = size(t->type);
	    break;

        case VTAG:
        case VLABEL:
	case REFFIELD:
	case FIELD:
            if (t->type && t->type->class == ARRAY) { /* cobol "usage is" */
              r = size(t->type);
            }
            else {
              off = t->symvalue.field.offset;
              len = t->symvalue.field.length;
              r = (off + len + 7) / 8 - (off / 8);
            }
            break;

	case CLASS:
	    r = t->symvalue.class.offset;
	    if (r == 0 and t->chain != nil) {
		panic(catgets(scmc_catd, MS_symbols, MSG_635,
         	      "missing size information for class"));
	    }
	    break;

        case BASECLASS:
            r = size(t->type);
            break;

        case MEMBER:
            if (t->symvalue.member.type == DATAM)
            {
                        off = t->symvalue.member.attrs.data.offset;
                        len = t->symvalue.member.attrs.data.length;
                        r = (off + len + 7) / 8 - (off / 8);
            }
            else
                r = 0;
            break;

        case NESTEDCLASS:
        case FRIENDFUNC:
        case FRIENDCLASS:
            r = 0;
            break;

	case GROUP:
	case RGROUP:
	    r = t->symvalue.usage.bytesize;
	    if (r == 0 and t->chain != nil) {
		panic( catgets(scmc_catd, MS_symbols, MSG_375,
				       "missing size information for record"));
	    }
	    break;
	case RECORD:
	case PACKRECORD:
	case UNION:
	case VARNT:
	    r = t->symvalue.offset;
	    if (r == 0 and t->chain != nil) {
		panic( catgets(scmc_catd, MS_symbols, MSG_375,
				       "missing size information for record"));
	    }
	    break;

        case PIC:
        case RPIC:
	    r = t->symvalue.usage.bytesize;
	    break;

	case STRINGPTR:
	case PTR:
	    /* Pascal pointers and stringptrs are two words long */
            if (t->language == pascalLang)
            {
              r = 2*sizeof(Word);
              break;
            }
            /* Skip thru all regular pointers (PTR) in fortran */
            if ( t->language == fLang)
            {
              r = size(t->type);
              break;
            }
	case FPTR:
	case TYPEREF:
	case FILET:
	    r = sizeof(Word);
	    break;

        case CPPREF:
	    r = sizeof(Word);
	    break;

        case PTRTOMEM: 
            /* the ptrType filed contains a pointer to the pointer to */
	    /* member structure                        		      */
            r = size(t->symvalue.ptrtomem.ptrType);

            break;

	case SCAL:
            if (t->language == pascalLang)
            {
              if (t->symvalue.iconval > 255) {
                  r = sizeof(short);
              } else {
                  r = sizeof(char);
              }
	    }
            else
              /* C & C++ enums vary in type length */
              if (t->language == cppLang || t->language == cLang)
                 r = size(t->type);
              else
                 r = sizeof(Word);
            break;

	case FPROC:
	case FFUNC:
	    r = sizeof(Word);
	    break;

        case LABEL:
	case PROC:
	case FUNC:
	case CSECTFUNC:
        case PROCPARAM:
        case FUNCPARAM:
	case MODULE:
	case PROG:
	    r = sizeof(Symbol);
	    break;

	case PACKSET:
	case SET:
            if (streq(symname(t), "$emptySet"))
              return 0;
	    /* Adjust for true size of packed set of integer */
            if (t->symvalue.size != 0)
              return (t->symvalue.size + BITSPERBYTE - 1) / BITSPERBYTE;
	    u = rtype(t->type);
	    switch (u->class) {
		case RANGE:
		    if (t->class == SET)
		      r = SETLEN;
		    else 
		    {
		      r = u->symvalue.rangev.upper + 1;
 		      r = (r + BITSPERBYTE - 1) / BITSPERBYTE; 
		    }
		    break;

		case SCAL:
		    r = u->symvalue.iconval;
		    r = (r + BITSPERBYTE - 1) / BITSPERBYTE;
		    break;

		default:
		    error( catgets(scmc_catd, MS_symbols, MSG_376,
					  "expected range for set base type"));
		    break;
	    }
	    break;

	case COMMON:
	case COND:
	    r = 0;
	    break;

	/*
	 * These can happen in C (unfortunately) for unresolved type references
	 * Assume they are pointers.
	 */
	case BADUSE:
	    r = sizeof(Address);
	    break;

	default:
	    if (ord(t->class) > ord(LASTCLASS)) {
		panic( catgets(scmc_catd, MS_symbols, MSG_377,
				       "size: bad class (%d)"), ord(t->class));
	    } else {
		(*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_378,
				"cannot compute size of a %s\n"), classname(t));
	    }
	    r = 0;
	    break;
    }
    return r;
}


/*
 * Return the size associated with a symbol that takes into account
 * reference parameters.  This might be better as the normal size function, but
 * too many places already depend on it working the way it does.
 */

public integer psize (s)
Symbol s;
{
    integer r;
    Symbol t;

    if (s->class == REF) {
	t = rtype(s->type);
	if (t->class == SUBARRAY) {
	    r = (2 * t->symvalue.ndims + 1) * sizeof(Word);
        } else if (t->class == STRING) {
            r = 2*sizeof(Word); /* First word is the address, second is len */
	} else {
	    r = sizeof(Word);
	}
    } else if (s->class == CONST and rtype(s)->class == STRING) {
        r = 2*sizeof(Word);     /* First word is the address, second is len */
    } else {
	r = size(s);
    }
    return r;
}

/*
 * Test if a symbol is a var parameter, i.e. has class REF.
 */

public Boolean isvarparam(s)
Symbol s;
{
    return (Boolean) ( ( (s->class == REF) &&
                         (s->type->class != FUNCPARAM) &&
                         (s->type->class != PROCPARAM) ) ||
                       ( (s->class == CONST || s->class == FPTR) &&
                         (s->param == true) )
                     );
}

/*
 * Test if a symbol is a variable (actually any addressible quantity
 * will do).
 */

public Boolean isvariable(s)
Symbol s;
{
    return (Boolean) ((s != nil) and 
		      (s->class == VAR or s->class == FVAR
		       or s->class == TOCVAR
                       or s->class == FPTR or s->class == FPTEE
		       or s->class == REF or s->class == REFFIELD
		       or (s->class == CONST and s->param == true)) );
}


public Boolean withinblock( f, s )
Symbol f;
Symbol s;
{
    return (Boolean) (f ? f == s->block : true );
}
/*
 * Test if a symbol is a constant.
 */

public Boolean isconst(s)
Symbol s;
{
    return (Boolean) (s->class == CONST and s->param == false);
}

/*
 * Test if a symbol is a module.
 */

public Boolean ismodule(s)
register Symbol s;
{
    return (Boolean) (s->class == MODULE);
}

/*
 * Mark a procedure or function as internal, meaning that it is called
 * with a different calling sequence.
 */

public markInternal (s)
Symbol s;
{
    s->symvalue.funcv.intern = true;
}

public boolean isinternal (s)
Symbol s;
{
    return s->symvalue.funcv.intern;
}

/*
 * Mark a procedure or function as a FORTRAN ENTRY point.
 */

public markEntry (s)
Symbol s;
{
    s->symvalue.funcv.isentry = true;
    s->symvalue.funcv.src = true;
}

public boolean isentry (s)
Symbol s;
{
    return s->symvalue.funcv.isentry;
}

/*
 * Decide if a field begins or ends on a bit rather than byte boundary.
 */

public Boolean isbitfield(s)
register Symbol s;
{
    boolean b;
    register integer off, len;
    register Symbol t;

    if (s->class == FIELD)
    {
	off = s->symvalue.field.offset;
	len = s->symvalue.field.length;
	if ((off mod BITSPERBYTE) != 0 or (len mod BITSPERBYTE) != 0) {
	    b = true;
	} else {
	    t = rtype(s->type);
	    b = (Boolean) (
                /* C and C++ enum can have varying sizes... */
                (t->class == SCAL and
                 (len != (sizeof(int)*BITSPERBYTE) and
                  len != (sizeof(short)*BITSPERBYTE) and
                  len != (sizeof(char)*BITSPERBYTE)))
                or len != (size(t)*BITSPERBYTE)
	    );
	}
    }
    else if (s->class == MEMBER && s->symvalue.member.type == DATAM &&
             !s->symvalue.member.isStatic)
    {
	off = s->symvalue.member.attrs.data.offset;
	len = s->symvalue.member.attrs.data.length;
	if ((off mod BITSPERBYTE) != 0 or (len mod BITSPERBYTE) != 0) {
	    b = true;
	} else {
	    t = rtype(s->type);
	    b = (len != (size(t) * BITSPERBYTE));
        }
    }
    else
	return false;
    return b;
}

public Boolean isquad(s)
Symbol s;
{
  Symbol t = rtype(s);
  
  return ( t != nil && t->class != PIC && t->class != RPIC &&
           (size(t) > size(t_real)) );
}

private boolean primlang_typematch (t1, t2)
Symbol t1, t2;
{

 if ((!t1) || (!t2) || (!t1->type) || (!t2->type))
     return false;
 return (boolean) ((t1 == t2) 
        or ( !strcmp(t1->type->name->identifier,
                     t2->type->name->identifier)
             && t1->type->name)			/* same but not nil */
        or ( !strcmp(t1->type->name->identifier,"char") &&
             !strcmp(t2->type->name->identifier,"$char")) 
        or (isintegral(t1->type->name) &&
            isintegral(t2->type->name))
        or ( !strcmp(t1->type->name->identifier,"float") &&
             !strcmp(t2->type->name->identifier,"$real")) 
        or ( t1->class == RANGE and t2->class == RANGE and
	     t1->symvalue.rangev.lower == t2->symvalue.rangev.lower and
	     t1->symvalue.rangev.upper == t2->symvalue.rangev.upper)
        or ( t1->class == PTR and t2->class == RANGE and
	     t2->symvalue.rangev.upper >= t2->symvalue.rangev.lower) 
        or ( t2->class == PTR and t1->class == RANGE and
	     t1->symvalue.rangev.upper >= t1->symvalue.rangev.lower));
}

/*
 * Test if two types match.
 * Equivalent names implies a match in any language.
 *
 * Special symbols must be handled with care.
 */

public Boolean compatible (t1, t2)
register Symbol t1, t2;
{
    Boolean b;

    if (t1 == t2) {
	b = true;
    } else if (t1 == nil or t2 == nil) {
	b = false;
    } else if (t1 == procsym) {
	b = isblock(t2);
    } else if (t2 == procsym) {
	b = isblock(t1);
    } else if (t1->language == nil) {
	if (t2->language == nil) {
	    b = false;
	} else if (t2->language == primlang) {
	    b = (boolean) primlang_typematch(rtype(t1), rtype(t2));
	} else {
	    b = (boolean) (*language_op(t2->language, L_TYPEMATCH))(t1, t2);
	}
    } else if (t1->language == primlang) {
	if (t2->language == primlang or t2->language == nil) {
	    b = primlang_typematch(rtype(t1), rtype(t2));
	} else {
	    b = (boolean) (*language_op(t2->language, L_TYPEMATCH))(t1, t2);
	}
    } else {
	b = (boolean) (*language_op(t1->language, L_TYPEMATCH))(t1, t2);
    }
    return b;
}

/*
 * Check for a type of the given name.
 */

public Boolean istypename (type, name)
Symbol type;
String name;
{
    register Symbol t;
    Boolean b;

    t = type;
    if (t == nil) {
	b = false;
    } else {
	b = (Boolean) (
	    /* fortran derived type can be named anything, so */
	    /* we need to return false if we have one.        */
            t->class == TYPE and streq(ident(t->name), name) and
            ( (t->class != RECORD and t->class != PACKRECORD) or
              (t->language != fLang))
	);
    }
    return b;
}

/*
 * Check for the various forms of char
 */

public Boolean ischartype (s)
Symbol s;
{
    Symbol t;

    t = s->type;
    return( (Boolean) (istypename(t,"char") || istypename(t,"unsigned char") ||
    	     istypename(t,"signed char") || istypename(t,"character") ||
	     istypename(t,"$char")) );
}

/*
 * Determine if a (value) parameter should actually be passed by address.
 */

public boolean passaddr (p, exprtype)
Symbol p, exprtype;
{
    boolean b;
    Language def;

    if (p == nil) {
	def = findlanguage(".c");
	b = (boolean) (*language_op(def, L_PASSADDR))(p, exprtype);
    } else if (p->language == nil or p->language == primlang) {
	b = false;
    } else {
	b = (boolean) (*language_op(p->language, L_PASSADDR))(p, exprtype);
    }
    return b;
}


/*
 * NAME: sameQualifiedName
 *
 * FUNCTION: Determine if two symbols have the same qualified name.
 *
 * PARAMETERS:
 *	s	- first symbol
 *	f	- second symbol
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: True if the two symbols have the same qualified name.
 *	    Else return false.
 */
private Boolean sameQualifiedName(s, f)
Symbol s, f;
{
   Symbol sb = s;
   Symbol fb = f;

   while ((sb && fb) && (sb->name == fb->name)) {
        sb = sb->block;
        fb = fb->block;
        if (sb == fb) return true;      /* sb == fb or both nil */
   }
   return false;
}


/*
 * Test if the name of a symbol is uniquely defined or not.
 * Treat as not ambiguous the case where s is defined inside
 * a module of the same name.
 */

public Boolean isambiguous (s, criteria)
register Symbol s;
int criteria;
{
    Symbol t;

    for (t = lookup(s->name); t != nil; t = t->next_sym)
        if (t != s && t->name == s->name) 
        {
            /* consider exceptions first */
            if (t->class == MODULE && s->block == t ||
		!meets(t, criteria) || cpp_equivalent(s, t) ||
	        (criteria == WFUNC && !sameQualifiedName(s,t))) 
		continue;
            return true;
        }
    return false;
}

typedef char *Arglist;

#define nextarg(type)  ((type *) (ap += sizeof(type)))[-1]

private Symbol mkstring();

/*
 * Determine the type of a parse tree.
 *
 * Also make some symbol-dependent changes to the tree such as
 * removing indirection for constant or register symbols.
 */

public assigntypes (p)
register Node p;
{
    register Node p1;
    register Symbol s;
    CobolType cobtype;

    switch (p->op) {
	case O_SYM:
	    p->nodetype = p->value.sym;
	    break;

/*
 * Moved inside build()
	case O_LCON:
	    p->nodetype = t_int;
	    break;

	case O_CCON:
	    p->nodetype = t_char;
	    break;

	case O_FCON:
	    p->nodetype = t_real;
	    break;
*/

	case O_SCON:
	    p->nodetype = mkstring(p->value.scon);
	    if (p->value.fscon.strsize != 0)
    	      p->nodetype->chain->symvalue.rangev.upper = 
						p->value.fscon.strsize;
	    break;

	case O_INDIR:
	case O_INDIRA:
	case O_CPPREF:
	    p1 = p->value.arg[0];
	    s = rtype(p1->nodetype);
	    /* Should allow indirection for regular pointer */
	    /* types and PASCAL stringptr and file types.   */			
            if (!(s->class == PTR or s->class == STRINGPTR or
                  s->language == pascalLang and s->class == FILET or
		  s->class == CPPREF))
	    {
		beginerrmsg();
		(*rpt_error)(stderr, "\"");
		prtree( rpt_error, stderr, p1);
		(*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_380,
						       "\" is not a pointer"));
		enderrmsg();
	    }

            /*  if we are dereferencing a dbx-created address  */ 
            /*  Example : print *(&x)                          */
            /*            should yield x                       */

            if ((p1->op == O_SYM) && (p1->nodetype == t_addr))
            {
              /*  set the nodetype to the type of the symbol  */

              /*  NOTE : this is necessary because there is code
                           under the O_INDIR case in eval for 
                           handling "debugger" variables  */
              p->nodetype = rtype (p1->value.sym);
            }
            else
              p->nodetype = rtype(p1->nodetype)->type;

	    break;

	case O_DOT:
	    p->nodetype = p->value.arg[1]->value.sym;
	    break;

        case O_DOTSTAR:
        {
            Symbol tp;

            tp = rtype(p->value.arg[1]->nodetype);
            if (tp->class == CPPREF)
                p->nodetype = rtype(tp->type)->type;
            else
                p->nodetype = tp->type;

            break;
	}

	case O_RVAL:
	    p1 = p->value.arg[0];
	    p->nodetype = p1->nodetype;
	    if (p1->op == O_SYM) 
	    {
		unsigned int class = p->nodetype->class;
		if (class == PROC or class == CSECTFUNC or class == FUNC) 
		{
	            p->op = p1->op;
		    p->value.sym = p1->value.sym;
		    tfree( p1 );
		}
		else if (isconst(p1->value.sym))
		{
	            p->op = p1->op;
		    p->value = p1->value;
		    tfree( p1 );
		}
	    }
	    break;

	case O_COMMA:
	    p->nodetype = p->value.arg[0]->nodetype;
	    break;

	case O_CALLPROC:
	case O_CALL:
	    p1 = p->value.arg[0];
	    p->nodetype = rtype(p1->nodetype)->type;
	    break;

	case O_TYPERENAME:
	    p->nodetype = p->value.arg[1]->nodetype;
	    break;

	case O_ITOF:
	    p->nodetype = t_real;
	    break;

	case O_ITOQ:
	case O_FTOQ:
	    p->nodetype = t_quad;
	    break;

	case O_NEG:
	    s = p->value.arg[0]->nodetype;
	    cobtype = checkCobolOp(p->value.arg[0]);
	    if (not compatible(s, t_int) || (cobtype == floating)) {
		if (not compatible(s, t_real)) {
		    beginerrmsg();
		    (*rpt_error)(stderr, "\"");
		    prtree( rpt_error, stderr, p->value.arg[0]);
		    (*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols,
					      MSG_382, "\" is improper type"));
		    enderrmsg();
		} else {
		    if (isquad(s)) {
		       p->op = O_NEGQ;
		       p->nodetype = t_quad;
		    } else {
		       p->op = O_NEGF;
		       p->nodetype = t_real;
		    }
		}
	    } else {
              if (size(s) == sizeofLongLong)
                p->nodetype = t_longlong;
              else
                p->nodetype = t_int;
	    }
	    break;

	case O_COMP:
	    s = p->value.arg[0]->nodetype;
	    cobtype = checkCobolOp(p->value.arg[0]);
	    if (not compatible(s, t_int) || (cobtype == floating)) {
		    beginerrmsg();
		    (*rpt_error)(stderr, "\"");
		    prtree( rpt_error, stderr, p->value.arg[0]);
		    (*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols,
					      MSG_382, "\" is improper type"));
		    enderrmsg();
		}
              if (size(s) == sizeofLongLong)
                p->nodetype = t_longlong;
              else
                p->nodetype = t_int;
	    break;

	case O_ADD:
	case O_SUB:
	case O_MUL:
	    binaryop(p, nil);
	    break;

	case O_LT:
	case O_LE:
	case O_GT:
	case O_GE:
	case O_EQ:
	case O_NE:
	    binaryop(p, t_boolean);
	    break;

	case O_DIVF:
	case O_EXP:
	    if (isquad(p->value.arg[0]->nodetype) ||
	        isquad(p->value.arg[1]->nodetype)) {
	      binaryop(p, nil); 
	      break;
	    }
	    chkflt(&p->value.arg[0]);
	    chkflt(&p->value.arg[1]);
	    p->nodetype = t_real;
	    break;

	case O_DIV:
	case O_MOD:
	case O_BAND:
	case O_BOR:
	case O_BXOR:
	case O_SL:
	case O_SR:
	    chkint(p->value.arg[0]);
	    chkint(p->value.arg[1]);
            p->nodetype =
              get_output_nodetype(rtype(p->value.arg[0]->nodetype),
                                  rtype(p->value.arg[1]->nodetype));
	    break;

	case O_NOT:
	    chkboolean(p->value.arg[0]);
	    p->nodetype = t_boolean;
	    break;

	case O_AND:
	case O_OR:
	    chkboolean(p->value.arg[0]);
	    chkboolean(p->value.arg[1]);
	    p->nodetype = t_boolean;
	    break;

	case O_QLINE:
	    p->nodetype = t_int;
	    break;

	case O_SIZEOF:
	    p1 = p->value.arg[0];
	    s = rtype(p1->nodetype);
	    p->nodetype = t_int;
	    break;

	default:
	    p->nodetype = nil;
	    break;
    }
}

/*
 * NAME: get_output_nodetype
 *
 * FUNCTION: determines the type of the "result" of a binary 
 *           operation, based on the types of the input.
 *
 * PARAMETERS:
 *        op1   - Symbol containing the "left" operand
 *        op2   - Symbol containing the "right" operand
 *
 * NOTES: this follows the rules for integer promotion.
 *
 * 1) For binary operators that expect operands of arithmetic
 *    type, if either operand has type unsigned long long int,
 *    the other operand is converted to unsigned long long int.
 * 2) Otherwise, if either operand has type long long int, the
 *    other is converted to long long int.
 * 3) Otherwise, if either operand has type unsigned int, the
 *    other is converted to unsigned int.
 * 4) Otherwise, both operands are converted to int.
 * 
 * This translates to the following diagram where you convert
 * each operand to the lowest type at or above both operand types:
 *
 *       unsigned long long
 *               |
 *           long long
 *               |
 *          unsigned long
 *             /   \
 *          long   unsigned int
 *             \   /
 *              int
 *
 *   
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: none   
 *
 * RETURNS: the Symbol descriping the "output" type
 *
 */

Symbol get_output_nodetype (op1, op2)
Symbol op1;   
Symbol op2;
{
  unsigned char op1_type = ISLONG, op2_type = ISLONG;
  
  if (op1->symvalue.rangev.size == sizeofLongLong) 
    op1_type |= ISLONGLONG;
  if (op1->symvalue.rangev.is_unsigned)
    op1_type |= ISUNSIGNED;
  
  if (op2->symvalue.rangev.size == sizeofLongLong) 
    op2_type |= ISLONGLONG;
  if (op2->symvalue.rangev.is_unsigned)
    op2_type |= ISUNSIGNED;
  
  if ((op1_type == ISUNSIGNEDLONGLONG) 
   || (op2_type == ISUNSIGNEDLONGLONG))
  {
    return t_ulonglong;
  }
  else if ((op1_type == ISLONGLONG) || (op2_type == ISLONGLONG))
  {
    return t_longlong;
  }
  else if ((op1_type == ISUNSIGNEDLONG) || (op2_type == ISUNSIGNEDLONG))
  {
    return dt_uint;
  }
  else
  {
    return t_int;
  }
}


/*
 * NAME: binaryop
 *
 * FUNCTION: Process a binary arithmetic or relational operator.
 *           Convert from integer to real, if necessary.
 *
 * PARAMETERS:
 *        p   - Input Node
 *        t   - Input Symbol
 *
 *   
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: none   
 *
 * RETURNS: nothing
 *
 */

private binaryop (p, t)
Node p;
Symbol t;
{
    Node p1, p2;
    Boolean t1real, t2real, t1float, t2float;
    Boolean t1quad, t2quad;
    CobolType t1cobtype, t2cobtype;
    Symbol t1, t2;

    p1 = p->value.arg[0];
    p2 = p->value.arg[1];
    /* Don't convert in comparing against the proc compare symbol */
    if ((p1->value.sym  == procsym) || (p2->value.sym == procsym)) {
        if (t != nil) {
	    p->nodetype = t;
        }
	return;
    }
    t1 = rtype(p1->nodetype);
    t2 = rtype(p2->nodetype);
    t1cobtype = checkCobolOp(p1);
    t2cobtype = checkCobolOp(p2);
    t1real = compatible(t1, t_real);
    t2real = compatible(t2, t_real);
    t1float = compatible(t1, t_float);
    t2float = compatible(t2, t_float);
    t1quad = (t1real && isquad(t1));
    t2quad = (t2real && isquad(t2));
    if (t1quad or t2quad) {
	p->op = (Operator) (ord(p->op) + 2);
	if (!t1quad) {
	   if (t1real || t1float)
	     p->value.arg[0] = build(O_FTOQ, p1);
	   else 
	     p->value.arg[0] = build(O_ITOQ, p1);
	} else if (!t2quad) {
	   if (t2real || t2float)
	     p->value.arg[1] = build(O_FTOQ, p2);
	   else 
	     p->value.arg[1] = build(O_ITOQ, p2);
	}
	p->nodetype = t_quad;
    } else if (t1real or t2real) {
	p->op = (Operator) (ord(p->op) + 1);
	if (not t1real and not t1float) {
	    p->value.arg[0] = build(O_ITOF, p1);
	} else if (not t2real and not t2float) {
	    p->value.arg[1] = build(O_ITOF, p2);
	}
	p->nodetype = t_real;
    }
    else {
	if (t1float or t2float) {
	    p->op = (Operator) (ord(p->op) + 1);
	    if (!t1float)
	       p->value.arg[0] = build(O_ITOF, p1);
	    if (!t2float)
	       p->value.arg[1] = build(O_ITOF, p2);
	    p->nodetype = (t == nil) ? t_real : t;
	    return;
	}
	else if ((size(p1->nodetype) > sizeofLongLong) &&
		 (t1cobtype != integral)) {
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_385,
					       "operation not defined on \""));
	    prtree( rpt_error, stderr, p1);
	    (*rpt_error)(stderr, "\"");
	    enderrmsg();
	} else if ((size(p2->nodetype) > sizeofLongLong) &&
		   (t2cobtype != integral)) {
	    beginerrmsg();
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_385,
					       "operation not defined on \""));
	    prtree( rpt_error, stderr, p2);
	    (*rpt_error)(stderr, "\"");
	    enderrmsg();
	}
        else
        {
          p->nodetype = get_output_nodetype (t1, t2);
        }
    }
    if (t != nil) {
	p->nodetype = t;
    }
}

/*
 * See if operation is defined on COBOL data type
 */
CobolType checkCobolOp(n)
     Node n;
{
  Symbol s = rtype(n->nodetype);
  
  if (s->class == PIC || s->class == RPIC) {
    if ((s->symvalue.usage.storetype >= 'e' && /* numeric */
	 s->symvalue.usage.storetype <= 'o') ||
	s->symvalue.usage.storetype == 'q' || /* pic 9 */
	s->symvalue.usage.storetype == 's') { /* index */
      if (s->symvalue.usage.decimal_align > 0)
	return floating;
      else
	return integral;
    }
    else {
      beginerrmsg();
      (*rpt_error)(stderr, catgets(scmc_catd, MS_symbols, MSG_419,
	"Operation not defined on edited or alpha data types."));
      enderrmsg();
    }
  }
  return none;
}

private chkflt (pp)
Node *pp;
{
    register Node p;
    CobolType cobtype;

    p = *pp;
    cobtype = checkCobolOp(p);
    if (compatible(p->nodetype, t_int) && (cobtype != floating)) {
	*pp = build(O_ITOF, p);
    } else if (!compatible(p->nodetype, t_real)) {
	error( catgets(scmc_catd, MS_symbols, MSG_389,
			"non-numeric operand for division or exponentiation"));
    }
}

private chkint (operand)
register Node operand;
{
    checkCobolOp(operand);
    if (!compatible(operand->nodetype, t_int)) {
	error( catgets(scmc_catd, MS_symbols, MSG_390,
					"non-integer operand for div or mod"));
    }
}


/*
 * construct a node for the .* operator
 */


public Node dotptr (record, pmember)
Node record;
Node pmember;
{
    return build(O_RVAL, build(O_DOTSTAR, record, pmember));
}

private String criteriaStr(criteria)
int criteria;
{
    static char buffer[100];
    int length = 0;

    if (((criteria & WSEARCH) == WANY) || (criteria & WOTHER))
	return "";
    else if ((criteria & WSEARCH) == WTYPE)
	return "type ";
    else
    {
	buffer[0] = '\0';
	if (criteria & WCLASS)
	{
	    (void)strcat(buffer, "class");
	    length += 5;
	}
	if (criteria & WUNION)
	{
	    if (length > 0)
		buffer[length++] = '/';
	    (void)strcpy(&buffer[length], "union");
	    length += 5;
	}
	if (criteria & WSTRUCT)
	{
	    if (length > 0)
		buffer[length++] = '/';
	    (void)strcpy(&buffer[length], "struct");
	    length += 6;
	}
	if (criteria & WENUM)
	{
	    if (length > 0)
		buffer[length++] = '/';
	    (void)strcpy(&buffer[length], "enum");
	    length += 4;
	}
	if (criteria & WTYPEDEF)
	{
	    if (length > 0)
		buffer[length++] = '/';
	    (void)strcpy(&buffer[length], "typedef");
	    length += 7;
	}
	buffer[length++] = ' ';
	buffer[length] = '\0';

	return buffer;
    }
}

private Node findfield(/* Name, Node, int */);
private Symbol findThis(/* Symbol */);

/*
 * Construct a node for the dot operator.
 *
 * If the left operand is not a record, but rather a procedure
 * or function, then we interpret the "." as referencing an
 * "invisible" variable; i.e. a variable within a dynamically
 * active block but not within the static scope of the current procedure.
 */

public Node dot(record, field, criteria)
Node record;
Node field;
int criteria;
{
    Node p;
    Symbol s, t, nodetype;
    boolean inreg;
    Name name;
    extern int lazy;

    name = field->value.name;
    nodetype = record->nodetype;
    if (lazy)
        touch_sym(nodetype);
    if (isblock(nodetype)) {
	find(s, name) where
	    s->block == nodetype and
	    s->class != FIELD and meets(s, criteria)
	endfind(s);

	/* If the lookup failed and this is a C++ member function, try to */
	/* find the symbol as a member of function's class.		  */
	if (s == nil && nodetype->language == cppLang &&
	    nodetype->class == FUNC && nodetype->isMemberFunc)
	{
            Node classScopeWhich(/* Node, Node, Name, int */);

	    if ((p = classScopeWhich(nodetype, nil, name, criteria)) != nil)
	    {
		if (rtype(p->nodetype)->class == CPPREF)
		    p = build(O_CPPREF, p);
		return build(O_RVAL, p);
	    }
	    else
		s = nil;
	}
	if (s == nil) {
	    beginerrmsg();
	    (*rpt_error)(stderr, catgets(scmc_catd, MS_symbols, MSG_620,
	                         "%1$s\"%2$s\" is not defined in "), 
			         criteriaStr(criteria), ident(name));
	    printname(rpt_error, stderr, nodetype, false);
	    enderrmsg();
	}
	p = new(Node);
	p->op = O_SYM;
	p->value.sym = s;
	p->nodetype = s;

	if (rtype(s)->class == CPPREF)
	    p = build(O_CPPREF, p);

	if (nodetype->class == PROG)
	    return p;
	else
	    return build(O_RVAL, p);
    } else {
	int nonDataMemberSeen;

	p = findfield(name, record, criteria, &nonDataMemberSeen);

	if (p != nil && rtype(p->nodetype)->class == CPPREF)
	    p = build(O_CPPREF, p);

	if (p == nil) {
	    beginerrmsg();
	    (*rpt_error)(stderr, catgets(scmc_catd, MS_symbols, MSG_627,
	                         "\"%1$s\" is not %2$sfield in "), 
				 ident(name), criteriaStr(criteria));
	    prtree(rpt_error, stderr, record);
	    enderrmsg();
	}
	else if (nonDataMemberSeen && record->op == O_SYM)
	    return p;
	else
	    return build(O_RVAL, p);
    }
}

/*
 * buildSet:
 */

#define MAXINT 2147483647
#define MININT -2147483648
#define SETLEN 32

public Node buildSet(slist)
Node slist;
{
  Symbol t;
  Node p;
  int code, i, len = 0;
  char *buffer;
  long low, up;

  if (slist == nil)  				/* Empty Set */
  {
    t = maketype("$integer", 1, 0);
    t = t->type;
    buffer = (char *) malloc(SETLEN);
    for (i=0; i<SETLEN; i++)
      buffer[i] = 0x00; 
  }
  else
  {
    t = slist->nodetype->type;
    if (t->class == RANGE)
    {
      len = SETLEN;
      low = MAXINT; up = MININT;
    }
    else if (t->class == SCAL)
         { len = t->symvalue.iconval;
           len = (len + BITSPERBYTE -1) / BITSPERBYTE;
         } 
    buffer = (char *) malloc(len);
    for (i=0; i<len; i++)
       buffer[i] = 0x00;
  
    p = slist;
    while (p)
    {
      if (p->nodetype->type != t)
      {
        beginerrmsg();
        (*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_416,
			   "elements of a set must be of the same base type"));
        enderrmsg();
        break;
      }
      if (p->value.arg[0]->op == O_CCON or p->value.arg[0]->op == O_LCON)
      {
         code = p->value.arg[0]->value.lcon;
         if (code < low) low = code;
         if (code > up) up = code;
      }
      else if ((p->nodetype->type->class == SCAL)  	/* scalar constant */
                and (p->nodetype->class == CONST))	/* only */
         code = p->nodetype->symvalue.constval->value.lcon;
      else {
	     beginerrmsg();
             (*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_417,
	   "elements of a set must be of type char, int, or scalar constant"));
             enderrmsg();
             break;
           }
      encode(buffer, code, len);
      p = p->value.arg[1];
    }
    if (t->class == RANGE)
    {
      if (istypename(t->type, "$integer"))
         { t = maketype("$integer", low, up); t=t->type; } 
      else if (istypename(t->type, "$char"))
             { t = maketype("$char", low, up); t=t->type; }
      else {
         t->symvalue.rangev.lower = low;
         t->symvalue.rangev.upper = up;
      }
    }
  }
  return build(O_SETCON, buffer, t);
}



private encode(char *buffer, int code, int len)
{
  int index, shift;

  index = code / BITSPERBYTE;

  if (index < len)
  {
    shift = code mod BITSPERBYTE;
    buffer[index] |= (1<<(7-shift));
  }
}



/*
 * Return a tree corresponding to an array reference and do the
 * error checking.
 */

public Node subscript (a, slist)
Node a, slist;
{
    Symbol t;
    Node p;

    t = rtype(a->nodetype);
    if (t->language == nil or t->language == primlang) {
	p = (Node) (*language_op(findlanguage(".s"), L_BUILDAREF))(a, slist);
    } else {
	p = (Node) (*language_op(t->language, L_BUILDAREF))(a, slist);
    }
    return build(O_RVAL, p);
}

/*
 * Build an Array reference
 */

private Node buildarray(t, a, slist, range)
Symbol t;
Node a, slist;
boolean range;
{
    register Node esub;
    Node r = a, p = slist;

	for (; p != nil && ((t->class == ARRAY or t->class == PACKARRAY) or
              t->class == STRING or t->class == SPACE or 
              (range and t->class == PTR)); p = p->value.arg[1])
	{
	   esub = p->value.arg[0];
	   if (!range)
	      assert(t->chain->class == RANGE);
	   if (esub->op == O_DOTDOT)
	   {
	      typematch_indices(esub->value.arg[0], t);
	      typematch_indices(esub->value.arg[1], t);
	   }
	   else
	      typematch_indices(esub, t);
	   r = build(O_INDEX, r, esub);
	   r->nodetype = t->type;
	   t = t->type;
	}
        if (p != nil or
           (((t->class == SPACE or t->class == PACKARRAY))
            && (not istypename(t->type,"char"))))
        {
	   beginerrmsg();
	   if (p != nil)
	      (*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_397,
						"Too many subscripts for \""));
	   else
	      (*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_398,
					      "Not enough subscripts for \""));
	   prtree( rpt_error, stderr, a);
	   (*rpt_error)(stderr, "\"");
	   enderrmsg();
        }
  return r;
}

/*
 * Build an Array reference
 */

public Node buildaref(a, slist)
Node a, slist;
{
    register Symbol t, eltype;
    register Node old_rval, ptr_node;
    Node r = a, p = slist, q;

    if (is_f90_sym(a->nodetype))
      a->nodetype = convert_f90_sym(a->nodetype, NULL);

    t = rtype(a->nodetype);
    eltype = t->type;
    if (t->class == PTR) 
    {
      if (streq(language_name(t->language), "c"))
      {
			/* if any index contains a .. process as array */
         for (q=slist; q != nil && ((t->class == PTR or t->class == ARRAY or 
               t->class == PACKARRAY) or
               t->class == STRING or t->class == SPACE); q = q->value.arg[1])
	 {
	    if (q->value.arg[0]->op == O_DOTDOT) {
		r = buildarray(t, a, slist, true);
		return r;
		}
	 }
      }
      while (p)
      {
	if (p->value.arg[0]->op == O_DOTDOT)
	{
	   r = build(O_INDEX, a, p->value.arg[0]);
	   r->nodetype = eltype;
	   return(r);
	}
	if (r->op == O_DOTSTAR)
	    old_rval = r;
	else
	    old_rval = build(O_RVAL, r);
        ptr_node = p->value.arg[0];
	typematch_indices(p, t);
	r = build(O_MUL, ptr_node, build(O_LCON, (long) size(eltype)));
	r = build(O_ADD, old_rval, r);
	r->nodetype = eltype;
	p = p->value.arg[1];
        if (p)
	{
           t = rtype(eltype);
	   if (t->class != PTR)
	      break;
           eltype = t->type;
	   old_rval = build(O_RVAL, r);
	   ptr_node = p->value.arg[0];
	}
      }
      if (!p) return r;
    } 
    if (((t->class != ARRAY)&&(t->class != PACKARRAY)) 
        and (t->class != STRING) and (t->class != SPACE))
    {
	beginerrmsg();
	(*rpt_error)(stderr, "\"");
	prtree( rpt_error, stderr, a);
	(*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_396,
							"\" is not an array"));
	enderrmsg();
    }
    else 
	r = buildarray(t, r, p, false);
  return r;
}

/*
 * Evaluate a subscript index.
 */

public int evalindex (s, base, i)
Symbol s;
Address base;
long i;
{
    Symbol t;
    int r;

    t = rtype(s);
    if (t->language == nil or t->language == primlang) {
	r = ((*language_op(findlanguage(".s"), L_EVALAREF)) (s, base, i));
    } else {
	r = ((*language_op(t->language, L_EVALAREF)) (s, base, i));
    }
    return r;
}

/*
 * Check to see if a tree is boolean-valued, if not it's an error.
 */

public chkboolean (p)
register Node p;
{
    if ((p->nodetype != t_boolean) && 
		((p->nodetype != nil) && (p->nodetype->type != t_boolean))) {
	beginerrmsg();
	(*rpt_error)(stderr, "found ");
	prtree( rpt_error, stderr, p);
	(*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_401,
					     ", expected boolean expression"));
	enderrmsg();
    }
}

/*
 * Construct a node for the type of a string.
 */

private Symbol mkstring (str)
String str;
{
    register Symbol s;

    s = newSymbol(nil, 0, ARRAY, t_char, nil);
    s->chain = newSymbol(nil, 0, RANGE, t_int, nil);
    s->chain->language = s->language;
    s->chain->symvalue.rangev.lower = 1;
    s->chain->symvalue.rangev.upper = strlen(str) + 1;
    s->chain->symvalue.rangev.size = 0;
    return s;
}


/*
 * Figure out the "current" variable or function being referred to
 * by the name n.
 */

private Node stwhich(), dynwhich();

public boolean meets(s, criteria)
Symbol s;
int criteria;
{
    int match = 0x0;

    /* for searching function of any languages */
    if (criteria == WFUNC) {
       return isroutine(s) ? true : false;
    }

    if ((criteria & WSEARCH) == WANY || s->language != cppLang)
	return true;

    else if (s->class == TAG)
    {
	Symbol t;

	if (s->isClassTemplate)
	    t = rtype(s->symvalue.template.list->templateClass);
	else
	    t = rtype(s->type);

	if (t->class == CLASS)
	{
	    if (t->symvalue.class.key == 's') 
		match |= WSTRUCT;
	    else if (t->symvalue.class.key == 'u') 
		match |= WUNION;
	    else if (t->symvalue.class.key == 'c') 
		match |= WCLASS;
	}
	else if (t->class == SCAL) 
	    match |= WENUM;
    }
    else if (s->class == TYPE) 
	match |= WTYPEDEF;

    if (match == 0) 
	match |= WOTHER;

    return criteria & match;
}

private boolean staticMemberFunction(func)
Symbol func;
{
    return (func != nil && func->isMemberFunc &&
	    func->symvalue.funcv.u.memFuncSym->symvalue.member.isStatic) ?
        true : false;
}

private boolean memberFunction(func)
Symbol func;
{
    return (func != nil && func->isMemberFunc &&
	    !func->symvalue.funcv.u.memFuncSym->symvalue.member.isStatic) ?
        true : false;
}

private Symbol findThis(f)
Symbol f;
{
    /* find the this symbol of the given function */
    Symbol s = lookup(this);

    assert(memberFunction(f));
    while (s != nil && (s->name != this || s->block != f))
	s = s->next_sym;
    assert(s != nil); /* we know this because f is a member function */
    return s;
}

private Node classScopeWhich(func, qual, name, criteria)
/*
 * if the current function is in fact a member function of a class, check 
 *     1. If the function is non-static, "name" is a member of the class; and
 *     2. If the function is static, "name" is a static member of the class.
 */
Symbol func;
Node qual;
Name name;
int criteria;
{
    Boolean isMemFuncSym = memberFunction(func);
    Boolean isStatFuncSym = staticMemberFunction(func);

    Symbol tagSym;
    Symbol classSym;
    Node thisQual;
    Symbol memFuncSym;

    memFuncSym = func->symvalue.funcv.u.memFuncSym;
    if ((!isMemFuncSym || memFuncSym->symvalue.member.attrs.func.isSkeleton) && 
        !isStatFuncSym)
	return nil;

    tagSym = func->symvalue.funcv.u.memFuncSym->block;
    classSym = rtype(tagSym);

    if (isMemFuncSym)
	thisQual = build(O_RVAL, build(O_SYM, findThis(func)));
    else if (isStatFuncSym)
	thisQual = build(O_SYM, tagSym);

    if (qual != nil)
	thisQual = resolveQual(qual, thisQual, true);

    /* If "name" is a member and, should qual not be nil and the */
    /* qualifier be a base class of this function, resolve the   */
    /* name that way.						 */
    if (thisQual != nil && isMember(classSym, name, criteria))
    {
	Node etree = dot(thisQual, build(O_NAME, name), criteria);

	/* we must remove the O_RVAL supplied by O_DOT because */
	/* the unqualified symbol name already has one.	   */
	if (etree->op == O_RVAL)
	    return etree->value.arg[0];
	else
	    /* non-member (nested type or static) */
	    return etree;
    }
 
    /* Otherwise, the "qualifier" is the name of a nested class. Look */
    /* up the symbol in that class. 				      */
    if (qual != nil)
    {
	Node classSymNode = findQual(qual, build(O_SYM, classSym), true);
	if (classSymNode != nil)
	{
	    classSym = classSymNode->value.sym;
	    return dot(classSym, build(O_NAME, name), criteria);
	}
    }
    return nil;
}

public Node which (n, criteria)
Name n;
int criteria;
{
    Node symNode;

    Symbol s = lookup(n);
    while (s != nil && (s->name != n || !meets(s, criteria)))
	s = s->next_sym;

    if ((symNode = stwhich(s, n, nil, criteria)) == nil && 
	(symNode = dynwhich(s, n, nil, criteria)) == nil &&
	s != nil)
    {
	if (s->ispredefined == false && isambiguous(s, criteria))
	{
	    (*rpt_output)(stdout, "[using %s", criteriaStr(criteria));
	    printname(rpt_output, stdout, s, true);
	    (*rpt_output)(stdout, "]\n");
	}
	symNode = build(O_SYM, s);
    }

    if (symNode == nil && !(criteria & WNIL))
    {
	error(catgets(scmc_catd, MS_symbols, MSG_621,
	      "%1$s\"%2$s\" is not defined"), 
	      criteriaStr(criteria), ident(n));
	/*NOTREACHED*/
    }
    return symNode;
}

public Node qualWhich (n, q, criteria)
Name n;
Node q;
int criteria;
{
    Node symNode;

    if ((symNode = stwhich(nil, n, q, criteria)) == nil && 
	(symNode = dynwhich(nil, n, q, criteria)) == nil &&
	(symNode = dot(findQual(q, nil, false), build(O_NAME, n), criteria)) 
	 == nil)
    {
	error(catgets(scmc_catd, MS_symbols, MSG_621,
	      "%1$s\"%2$s\" is not defined"), 
	      criteriaStr(criteria), ident(n));
	/*NOTREACHED*/
    }
    return symNode;
}

/*
 * Static search.
 */

extern boolean isstopped;	/* Used to find symbol if prog finished */
extern boolean just_started;    /* Used to find symbol if prog beginning */

private Node stwhich (s, n, q, criteria)
Symbol s;
Name n;
Node q;
int criteria;
{
    Symbol f;		/* iteration variable for blocks containing s */
    Symbol i;		/* iteration variable for s */
    Symbol save_sym = NULL;
    integer curobj;	/* Which program object we are currently using */
    Symbol class;	/* class of which current function is a member */
    Node symNode;	/* Node of the symbol returned */

    f = curfunc;
    while (f != nil)
    {
	/* 
	 * check first to see if the symbol is a local variable of the 
	 * current block.
	 */
	if (q == nil)
	{
	    i = s;
	    while (i != nil)
	    {
	        if (i->name == n && i->block == f && i->class != FIELD && 
		    (i->class != TAG || i->language == cppLang) &&
		    meets(i, criteria))
	        {
                    /*  if this symbol is a module, see if we can find
                          another one before using this  */
                    if (i->class == MODULE)
                    {
                      save_sym = i;
                    }
                    else
                    {
                      s = i;
                      goto found;
                    }
	        }
	        i = i->next_sym;
	    }
            if (save_sym != NULL)
            {
              s = save_sym;
              goto found;
            }
	}

        if (f->language == cppLang)
	{
	    Node t = classScopeWhich(f, q, n, criteria);
	    if (t != nil)
		return t;
	}
	f = f->block;
    }

    return nil;

found:
    /* This section of code deals with yet another ambiguity in dbx.
     * It is possible that there will be several modules (files) 
     * which have the same name, but which are really entirely different
     * files.  If these are combined in the same load module, then there
     * isn't anything which we can do about it.  However, if the
     * ambiguous files are in different load modules, then we should be 
     * able to select the file which is in the same context as the 
     * currently active load module.  That is, if the pc is in a shared
     * library, then the ambiguous file from the shared library should
     * be selected, but if in the execed program, then the one from the
     * execed program should be selected.
     */
    if ((s->class == MODULE || s->class == FUNC) && (s->next_sym != nil)) 
    {
	Symbol m;
	boolean ambiguous = false;
	unsigned int module_level = 0xffffffff;
	unsigned int file_level;

	m = s->next_sym;
	do 
	{
	    if ((m->name == n) && (m->class == s->class)) 
	    {
		if (!ambiguous) 
		{
		    ambiguous = true;

		    if (just_started || (!isstopped))
			curobj = 0;
		    else
			curobj = addrtoobj(pc);

		    module_level = addrtoobj(prolloc(s));
		    if (module_level == curobj)
			break;
		}
		file_level = addrtoobj(prolloc(m));
		if (file_level == curobj) 
		{
		    s = m;
		    break;
		} 
		/* To really do it right, dbx will need to read the */
		/* ".loader" section and figure out what module the */
		/* symbol is imported from. But for a simple hack,  */
		/* we will take the symbol from the later module,   */
		/* which may or may not be the one user really want.*/
		else if (file_level > module_level)
		{
		    s = m;
		    module_level = file_level;
		}
	    }
	    m = m->next_sym;
	} while (m != nil);
    }
    return build(O_SYM, s);
}

/*
 * Dynamic search.
 */

private Node dynwhich (s, n, q, criteria)
Symbol s;
Name n;
Node q;
int criteria;
{
    struct Frame frame;

    Symbol f;		/* iteration variable for active functions */
    Frame frp;		/* pointer to the current frame */
    Symbol class;       /* The most likely symbol found to date */
    Symbol i;           /* Iteration symbol */
    Node symNode;	/* Node returned that contains the symbol */

    if (noexec || curfunc == nil)
	return nil;
    
    f = curfunc;
    frp = curfuncframe(&frame);

    i = nil;
    if (frp != nil) 
    {
	frp = nextfunc(frp, &f);
	while (frp != nil) 
	{
	    if (q == nil)
	    {
	        i = s;
	        while (i != nil and 
		       (i->name != n or i->block != f or i->class == FIELD or 
		        (i->class == TAG and i->language != cppLang) or
		        !meets(i, criteria)))
		    i = i->next_sym;

	        if (i != nil)
		    break;
	    }

            if (f == program || f == dbsubn_sym)
		break;
	    /*
	     * if the current block is in fact a member function, check to 
	     * see if the symbol is a member of the function's class
	     */
	    if (f->language == cppLang)
	    {
		Node t = classScopeWhich(f, q, n, criteria);
		if (t != nil)
		    return t;
	    }

	    frp = nextfunc(frp, &f);
	}
    }

    if (i == nil)
    {
	/* If the program is at the beginning or the end, then pick the global
	   which has the lowest load module index; this will favor the user's
	   exec module and disfavor the shared libraries, since the exec module
	   is always first. */

        integer curobj = addrtoobj(pc);
	if (curfuncframe(nil) != nil || just_started || !isstopped)
	{
	    unsigned int module_level;
	    unsigned int sym_module;

	    while (s != nil)
	    {
		if (s->name == n && s->level == 1 &&
		    (s->storage == EXT || isroutine(s)) &&
		    s->class != FIELD && s->class != TAG && meets(s, criteria))
		{
		    sym_module = addrtoobj(s->storage == EXT ?
					   s->symvalue.offset : prolloc(s));
		    if (i == nil) 
		    {
			i = s;
			module_level = sym_module;
		    } 
		    else 
		    {
			if (curobj == 0 || (sym_module != curobj)) 
			{
			    if (sym_module > module_level) 
			    {
				module_level = sym_module;
				i = s;
			    }
			} 
			else 
			    i = s;
		    }
		}
	        s = s->next_sym;
	    }
	}
    }

    if (i == nil) 
	return nil;
    else
        return build(O_SYM, i);
}

public Node lvalRecord (record)
Node record;
{
    Node p = record;
    if (record->nodetype->storage == INREG) 
    {
	p = unrval(p);
	pushregvalue(p->nodetype, p->nodetype->symvalue.offset, nil,
		     sizeof(Address));
	p = build(O_LCON, pop(Address));
    }
    else if (rtype(record->nodetype)->class != PTR ||
             record->nodetype->language == fLang)
	    /* For fortran PTR type, we do want to unrval it... */
	p = unrval(p);
    p->nodetype = t_addr;

    return p;
}

/*
 * Find the symbol that has the same name and scope as the
 * given symbol but is of the given field.  Return nil if there is none.
 */

private Node findfield (fieldname, record, criteria, nonDataMemberSeen)
Name fieldname;
Node record;
int criteria;
int *nonDataMemberSeen;
{
    Symbol m, t;

    t = rtype(record->nodetype);
    if (t->class == CPPREF)
	t = rtype(t->type);
    if (t->class == PTR)
	t = rtype(t->type);

    *nonDataMemberSeen = false;
    if (t->class == CLASS)
    {
	AccessList path;
	Node p;

	m = findMember(t, fieldname, &path, criteria);
	if (m != nil)
	{
	    if (m->class == MEMBER && m->symvalue.member.type == DATAM)
	    {
		if (m->symvalue.member.isStatic)
		{
		    Symbol varSym = m->symvalue.member.attrs.staticData.varSym;
		    assert(varSym != nil);
		    p = build(O_SYM, varSym);
		    *nonDataMemberSeen = true;
		}
		else
		{
		    /* Build the node that corresponds to a lookup of the */
		    /* thing 						  */
		    p = build(O_DOT, buildAccess(path, t, lvalRecord(record)),
			      build(O_SYM, m));
		}
	    }
	    else
	    {
		*nonDataMemberSeen = true;
		if (m->class == MEMBER && m->symvalue.member.type == FUNCM)
		{
		    /* We ignore static, virtual, etc., here as this is */
		    /* taken care of by resolveOverload. We instead en- */
		    /* sure that we don't lose any information.		*/

		    /* then the function call was qualified with a */
		    /* class, rather than an instance.	           */
		    Node this;
                    if (record->op == O_SYM && record->value.sym->class == TAG)
			this = record;
		    else
			this = lvalRecord(record);

		    /* Build the node that corresponds to a lookup */
		    /* of the function's this pointer. 		   */ 
		    p = build(O_DOT, buildAccess(path, t, this),
				     build(O_SYM, m));
		}
		else
		    p = build(O_SYM, m);
	    }
	}
	else
	    p = nil;
	freeAccessList(path);
	return p;
    }
    else
    {
	m = t->chain;
        while (m != nil and m->name != fieldname) 
	    m = m->chain;

	if (m != nil)
	    return build(O_DOT, lvalRecord(record), build(O_SYM, m));
	else
	    return nil;
    }
}

public Boolean getbound (s, off, type, valp)
Symbol s;
int off;
Rangetype type;
int *valp;
{
    Frame frp;
    Address addr;
    Symbol cur;
    Node vardim_node;

    if (type == R_ADJUST) {
       vardim_node = (Node) findvar(identname("$vardim",true));
       if (vardim_node == nil)
           *valp = 10;
       else {
           *valp = vardim_node->value.lcon;
       }
       return true;
    }
    if (not isactive(s->block)) {
	return false;
    }
    cur = s->block;
    while (cur != nil and cur->class == MODULE) {
	cur = cur->block;
    }
    if(cur == nil) {
	cur = whatblock(pc);
    }
    frp = findframe(cur);
    if (frp == nil) {
	return false;
    }
    if (type == R_TEMP) {
        addr = args_base(frp) + off;
    } else if (type == R_STATIC) {
        addr = off;
    } else if (type == R_ARG) {
        addr = args_base(frp) + off;       /* Calculate indirect address */
        dread(&addr, addr, sizeof(long));  /* Now get the real address.  */
    } else if (type == R_REGARG) {         /* Check for non-saved register */
        addr = frp->save_reg[off];         /* Calculate indirect address */
        dread(&addr, addr, sizeof(long));  /* Now get the real address.  */
    } else if (type == R_REGTMP)    {      /* Check for non-saved register */
        *valp = frp->save_reg[off];        /* Calculate indirect address */
        return true;
    } else {
	return false;
    }
    dread(valp, addr, sizeof(long));
    return true;
}

insertsym(s)
Symbol s;
{
   unsigned int h;
   
   h = hash(s->name);
   s->next_sym = hashtab[h];
   hashtab[h] = s;
}

typematch_indices( s, t)
Node s;
Symbol t;
{
   Symbol etype, indexType;

   etype = rtype(s->nodetype);
   indexType = t->chain->type;
   if (not compatible( t_int, etype) and not compatible(indexType, etype))
   {
      beginerrmsg();
      (*rpt_error)(stderr, "subscript ");
      prtree( rpt_error, stderr, s);
      (*rpt_error)(stderr, " is type %s ",symname(etype->type));
      enderrmsg();
   }
}

/*
 * isintegral(name) - tests a name to see if it is an integral type
 * These names came from dbxstclass.h - update this function if
 * the predefined types in dbxstclass.h change.
 */
boolean isintegral(name)
Name name;
{
  return (boolean) (streq(ident(name), "$integer") ||
      streq(ident(name), "$uinteger") ||
      streq(ident(name), "$longlong") ||
      streq(ident(name), "$ulonglong") ||
      streq(ident(name), "int") ||
      streq(ident(name), "short") ||
      streq(ident(name), "long") ||
      streq(ident(name), "long long") ||
      streq(ident(name), "unsigned short") ||
      streq(ident(name), "unsigned int") ||
      streq(ident(name), "unsigned") ||
      streq(ident(name), "unsigned long") ||
      streq(ident(name), "unsigned long long") ||
      streq(ident(name), "integer") ||
      streq(ident(name), "integer*1") ||
      streq(ident(name), "integer*2") ||
      streq(ident(name), "integer*4") ||
      streq(ident(name), "integer*8"));
}
  
/*
 * ischar(name) - tests a name to see if it is a character type
 * These names came from dbxstclass.h - update this function if
 * the predefined types in dbxstclass.h change.
 */
boolean ischar(name)
Name name;
{
  return (boolean)
     (streq(ident(name), "$char") ||
      streq(ident(name), "char") ||
      streq(ident(name), "unsigned char") ||
      streq(ident(name), "signed char") ||
      streq(ident(name), "character"));
}
  
/*
 * throw-away function, just wanted to look at what symbols are in hash table
 */
dump_symbol_names()
{
  register unsigned h;
  register Symbol s;

  for (h = 0; h < HASHTABLESIZE; ++h) {
    s = hashtab[h];
    if (s) {
      (*rpt_output)(stdout, "%d:\t",h);
      while (s != nil) {
	(*rpt_output)(stdout, "%s, ",ident(s->name));
	s = s->next_sym;
      }
      (*rpt_output)(stdout, "\n");
    }
  }
  return 0;
}

/*
 * NAME: dumpSymbolTable
 *
 * FUNCTION: Print declarations (whatis) of all symbols in Symbol table.
 *
 * NOTE: For debug use only. See function debug().
 *
 * PARAMETERS: NONE
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
public void dumpSymbolTable()
{
    register Integer i;
    register Symbol s;
    register count = 0;

    for (i = 0; i < HASHTABLESIZE; i++) {
        for (s = hashtab[i]; s != nil; s = s->next_sym) {
            printdecl(s);
            fflush(stdout);
            count++;
        }
    }
    (*rpt_output)(stdout, "\n\nTotal number of symbols = %d\n",count);
}

public Symbol forward(t)
Symbol t;
{
    Symbol s = t;
    if (s->class == TAG)
        while (s->name == nil && s->type->class == TAG)
            s = s->type;
    else if (s->class == TYPE)
        while (s->name == nil && s->type->class == TYPE)
            s = s->type;
    return s;
}

/*
 * The C++ member function Symbol table. This table is filled as the C++ class
 * stabstrings are parsed. The Symbol of each member function is placed in 
 * this table. Then, later, as CSECTFUNC symbol table entries are read, each
 * that is a member function has its corresponding MEMBER symbol looked up in 
 * this table. The two Symbols are then linked to one another, and the MEMBER
 * Symbol is removed from this table. It is possible that after loading this
 * table is not empty because certain member functions may not have been used
 * and the linker threw them away, so there is no CSECTFUNC symbol that will
 * retrieve them. Additionally, inline functions are a problem. Firstly, there
 * will be one copy of an inline in each module that includes the class defin-
 * ition, so there will be one CSECTFUNC for each inline. Thus, we don't want
 * to pull them out of the table during retreive, as they will likely be
 * referenced again. Also, the name of the member function symbol is set to
 * its real name (as opposed to its mangled name) after it is retreived, so
 * the next time we look up an inline function, we must be careful to check
 * that we're comparing the rigth names. Note that the mangled name space and 
 * the demangled name space do not overlap.
 */

Symbol memFuncTab[HASHTABLESIZE];

void MemFuncTabInit()
{
    unsigned int i;
    for (i = 0; i < HASHTABLESIZE; i++)
	memFuncTab[i] = nil;
}

void InsertMemFunc(s)
Symbol s;
{
    unsigned int h = hash(s->name);

    s->chain = memFuncTab[h];
    memFuncTab[h] = s;
}

Symbol RetrieveMemFunc(mName)
Name mName;
{
    unsigned int h = hash(mName);
    Symbol s = memFuncTab[h];
    Symbol t = nil;

    while (s != nil) 
    {
	if (!s->symvalue.member.attrs.func.isInline)
	{
	    if (s->name == mName)
	    {
		if (t == nil)
		    memFuncTab[h] = s->chain;
		else
		    t->chain = s->chain;
		return s;
	    }
	}
	else 
	{
	    if (s->name == mName /* true the first time through only */ 
		|| s->symvalue.member.attrs.func.dName->mName == mName)
	    {
		return s;
	    }
	}
	t = s;
	s = s->chain;
    }

    return s;
}
