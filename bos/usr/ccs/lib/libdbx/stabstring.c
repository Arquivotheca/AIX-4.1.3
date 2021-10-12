static char sccsid[] = "@(#)82	1.76.6.2  src/bos/usr/ccs/lib/libdbx/stabstring.c, libdbx, bos41J, 9517B_all 4/27/95 17:17:37";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: getRangeBoundType, addpredefines, chkcont, consComplex,
 *	      consCond, consDynarray, consEnum, consFD, consGroup, consImpType,
 *	      consIndex, consMulti, consOpaqType, consParamlist, consPic,
 *	      consReal, consRecord, consString, consStringptr, consSubrange,
 *	      consUindex, consValue, consVarRecord, consWide, constName,
 *	      constype, enterNestedBlock, enterRoutine, entersym, extVar,
 *	      findBlock, getExtRef, getint, initTypeTable,
 *	      makeParameter, makeVariable, newSym, optchar, ownVariable, 
 *	      privateRoutine, publicRoutine, skipchar, tagName, typeName, 
 *	      getPointer, nestedType, stab_init, setupMemberFunc, get_name,
 *	      string_alloc, string_pool_free
 *
 * ORIGINS: 26, 27
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

#ifdef KDBXRT
#include "rtnls.h"            /* MUST BE FIRST */
#endif

/* 			include file for message texts */
#include "dbx_msg.h" 
nl_catd  scmc_catd; /* Cat descriptor for scmc conversion */
#define getmsg(X, Y) catgets(scmc_catd, MS_stabstring, (X), (Y))

/*
 * String information interpretation
 *
 * The string part of a stab entry is broken up into name and type information.
 */

#include "defs.h"
#include "symbols.h"
#include "stabstring.h"
#include "object.h"
#include "main.h"
#include "names.h"
#include "languages.h"
#include "tree.h"
#include "mappings.h"
#include "cplusplus.h"
#include "eval.h"
#include <a.out.h>
#include <dbxstclass.h>
#include "aoutdefs.h"
#include <ctype.h>

/*
 * Special characters in symbol table information.
 */

#define LABELNAME 'L'			/* Pascal label*/
#define CONSTNAME 'c'
#define TYPENAME 't'
#define TAGNAME 'T'
#define MODULEBEGIN 'm'
#define EXTPROCEDURE 'P'
#define PRIVPROCEDURE 'Q'
#define INTPROCEDURE 'I'
#define EXTFUNCTION 'F'
#define PRIVFUNCTION 'f'
#define GENERICFUNC 'g'
#define INTFUNCTION 'J'
#define EXTVAR 'G'
#define MODULEVAR 'S'
#define OWNVAR 'V'
#define REGVAR 'r'
#define FREGVAR 'd'
#define CONSTPARAM 'C'	   		/* Pascal constant parameter */
#define VALUEPARAM 'p'
#define VARIABLEPARAM 'v'
#define REGPARAM 'R'
#define FREGPARAM 'D'
#define REGVARPARAM 'a'
#define FPOINTEE 'Z'
#define FPOINTER 'Y'
#define LOCALVAR 			/* default */

/*
 * Type information special characters.
 */

#define T_SUBRANGE 'r'
#define T_ARRAY 'a'
#define T_DEFERSHAPEARRAY 'A'
#define T_ASSUMESHAPEARRAY 'O'
#define T_SUBARRAY 'E'
#define T_PACKEDARRAY 'P'
#define T_VARNT 'v'	   /* Pascal Variant Record */
#define T_RECORD 's'
#define T_UNION_ 'u'
#define T_ENUM_ 'e'
#define T_PTR '*'
#define T_FUNCVAR 'f'
#define T_PROCVAR 'p'
#define T_IMPORTED 'i'
#define T_SET 'S'
#define T_OPAQUE 'o'
#define T_FILE 'd'
#define T_STRINGPTR 'N'
#define T_STRING 'n'
#define T_COMPLEX 'c'
#define T_REAL 'g'
#define T_WIDECHAR 'w'
#define T_PIC 'C'          /* Picture type specific to Cobol */
#define T_INDEX 'I'        /* Indexed by type specific to Cobol */
#define T_UINDEX 'l'       /* Usage is index specific to Cobol */
#define T_GROUP 'G'        /* Group type specific to Cobol */
#define T_MULTI 'M'        /* Multiple-unit base type, e.g. character * 10 */
#define T_PROCPARAM 'R'	   /* Pascal routine/procedure parameter */
#define T_FUNCPARAM 'F'	   /* Pascal Function parameter */
#define T_FILEDESCR 'K'	   /* Cobol File Descriptor */
#define T_SPACE 'b'        /* Pascal Space date type */
#define T_GSTRING 'z'      /* Pascal gstring type */

#define T_CONST 'k'	   /* C++ const type */
#define T_CLASS 'Y'	   /* C++ class type */
#define T_VOL 'V'	   /* C++ volatile type */
#define T_ELLIPSES 'Z'	   /* C++ ... parameters */
#define T_REF '&'	   /* C++ reference type */
#define T_PTRTOMEM 'm'	   /* C++ pointer to member type */

static char *csect_fcn_type = "F-1";		/* Temporary type string */
static integer group = 0;     /* group level for cobol */
static integer group_level = 1;     /* field level for cobol */

static Language lastlanguage = nil; 		/* Last language initialized */

/* The following chunk of definitions is designed to appease lint. */
private addpredefines();
private constName();
private typeName();
private tagName();
private publicRoutine();
private privateRoutine();
private extVar();
private getExtRef();
private enterNestedBlock();
private ownVariable();
private getint();
private Rangetype getRangeBoundType ();
private Symbol consCond();
private LongLong getlonglong ();

private char * lastchar = NULL;
extern char * debugtab;
/*
 * Table of types indexed by per-file unique identification number.
 */

/* Variables to keep track of nested Pascal functions and their parameters */

struct routinestack {
	Symbol name;
	struct routinestack *next, *back;
	};
public  struct routinestack *curroutine = nil;
extern  Address text_reloc, data_reloc;
extern  Filetab *curfilep;

extern Address csectaddr;

extern Language cLang;
extern Language cppLang;
extern Language fLang;
extern Language asmLang;
extern Language pascalLang;
extern boolean keepdescriptors;

/*
 * Table of types indexed by per-file unique identification number.
 */

#define NTYPES 5000
#define NUMSTANDRDTYPES 21
#define T_NLS		16	/* Type ID for National Language Support */
#define POOLSIZE 5000		/* default size for string pool */

private integer *typeindex;	/* Table of indices into type table */
private integer typendx = 0;
private integer typesinit = 0;
private SYMENT *curnp;
private int dertmask = 0;	/* Used to determine derived type level */
private int dimnum = 0;	/* Index into dimension of array */
private int lastsize = 0;
private int utypendx = 0;
private boolean resolving = false;
private int lastndx = 0;
private Symbol lastVar = nil;	/* for COBOL ref field inheritance */

extern Name csectname;		/* Interpret '^' as previous csectname */
extern int curndx;
extern boolean newfile;
extern unsigned long ln_index;	/* Index into line number table for function */
extern unsigned long linenoptr;	/* Raw offset into line table for function */
extern Linetab *curlinep;	/* Base of current line number table */
public int ntypes = 0;
extern char *stringtab;
extern boolean intoexterns;
extern Symbol findType();
extern char *calloc(), *realloc();
extern struct exec hdr;
extern Sympool sympool;
extern Integer nleft;
extern integer nesting;
extern Address addrstk[];
struct Symbol forward_ref;
public Address lastfuncprol = 0;
public Address lastfuncaddr = 0;
extern boolean isCOBOL;
extern boolean isfcn();
extern Desclist *ElimLinkage();
extern boolean keep_linkage;
extern boolean strip_;		/* Set if stripping '_' (fortran names) */

struct forward {
	int typeindx;
	int frwdindx;
	struct forward *next_frwd;
	}  *frwd_list = nil;

public Symbol *typetable = nil;
public Symbol *last_typetable = nil;	/* ptr to last typetable used */
public int typenummax = -1;

/* struct definition and variables for string pool used in */
/* string_alloc and string_pool_free.			   */

typedef struct Stringpool {
   char pool[POOLSIZE];
   struct Stringpool *prevpool;
} *Stringpool;

private Stringpool stringpool = nil;	
private int string_left = 0;

/* define our own isdigit since the libc one handles multibyte */
/* stuff and is too slow for our needs.			       */
#define isdigit(x) ((x >= (int)'0') && (x <= (int)'9'))

#define entertype(typenum,s) \
{ \
     typetable[typenum] = s; \
     if (typenum > typenummax) \
	 typenummax = typenum; \
}

static char *predefines[] = TP_ARRAY;

public initTypeTable (newprog)
boolean *newprog;
{
    boolean newlang;

    if (curlang != lastlanguage)
    {
	/* The language is new if the new language is different than the  */
	/* previous language, it is not assembler and the two languages   */
	/* are not C and C++ (for which the builtin symbols are the same) */
	newlang = (curlang != asmLang) &&
		  !((curlang == cLang) &&
		     (lastlanguage == cppLang) ||
		     (curlang == cppLang) &&
		     (lastlanguage == cLang));
    }
    else
        newlang = false;

    if (typetable == nil) {			/* Not allocated yet */
	*newprog = true;			/* true also if switched */
	typenummax = -1;			/* to new typetable      */
	if (ntypes == 0)			/* default */
	   ntypes = NTYPES;
	typetable = (Symbol *) malloc((ntypes+TP_NTYPES+1)*sizeof(Symbol));
    }

    /*  this is to handle cross load-module changes  */
    /*  ie : if the last module of one load module is the same */
    /*       language as the first module of the next load module  */
    if (last_typetable != typetable)
      newlang = true;

    if (*newprog || newlang) {
	if (typenummax == -1) {
            bset0(typetable, (ntypes+TP_NTYPES+1) * sizeof(Symbol));
	} else
	    if (!(*newprog) && (stab_compact_level >= 2))
	       /* In case we have unique typeid across the program */
	       /* (compact level >= 2), keep definitions of user   */
	       /* defined types and reset predefined types only.   */
	       /* Definitions of predefined types are different    */
	       /* for each language. 				   */
               bset0(typetable, TP_NTYPES * sizeof(Symbol));
	    else
	       /* reset all used entries in typetable */
               bset0(typetable, (typenummax + 1) * sizeof(Symbol));
    } else {
	/* Reset user defined types only if stabstring compact */
	/* level is 0 or 1 - type ID not shared across files.  */
	if (stab_compact_level >= 2) {
           last_typetable = typetable;
	   return;
	}
        bset0(&typetable[TP_NTYPES], 
			(typenummax + 1 - TP_NTYPES) * sizeof(Symbol));
    }

    addpredefines(newprog, newlang);

    if (newlang)
	lastlanguage = curlang;

    typenummax = TP_NTYPES + 1;
    last_typetable = typetable;
}

private addpredefines(newprog, newlang) 
boolean *newprog, newlang;
{
    int i;
    char *tmppredef;

    if (*newprog || newlang)
        (*language_op(curlang, L_MODINIT))();
    *newprog = false; 
}

/*
 * Each language decides which predefined types to insert.
 */

public addlangdefines(usetypes)
short usetypes[];
{
    char *predefstr;
    int i;

    for (i = TP_NTYPES - 1; i >= 0; i--) {
        if ((usetypes[i]) || (i < -TP_LDOUBLE)
#ifdef TP_LLONG
	/* To compile kdbx on AIX3.2 */
         || (i == (-TP_LLONG - 1)) || (i == (-TP_ULLONG - 1))
#endif
	) {
            predefstr = strdup(predefines[i]);
            entersym(predefstr, nil, false, true);
        }
    }
}

/*
 * Each language decides which predefined types to insert.
 */

public addpredefined(tp_index)
short tp_index;
{
    char *predefstr;

    predefstr = strdup(predefines[-tp_index-1]);
    entersym(predefstr, nil, false, true);
}

/*
 * Put an nlist entry into the symbol table.
 * If it's already there just add the associated information.
 *
 * Type information is encoded in the name following a ":".
 */

private Symbol constype();
private Char *curchar;

#define skipchar(ptr, ch) \
{ \
    if (*ptr != ch) { \
	panic(getmsg(MSG_270, "expected char '%c', found '%s'"), ch, ptr); \
    } \
    ++ptr; \
}

#define optchar(ptr, ch) \
{ \
    if (*ptr == ch) { \
	++ptr; \
    } \
}

#ifdef sun
#    define chkcont(ptr) \
{ \
    if (*ptr == '\\' or *ptr == '?') { \
	ptr = getcont(); \
    } \
}
#else /* if notsun */
#    define chkcont(ptr) \
{ \
    if (*ptr == '\\' or *ptr == '?') { \
	ptr = getcont(); \
    } \
}
#endif

#define newSym(s, n) \
{ \
    s = insert(n); \
    s->level = curblock->level + 1; \
    s->language = curlang; \
    s->block = curblock; \
}

#define makeVariable(s, n, off) \
{ \
    newSym(s, n); \
    s->class = VAR; \
    s->symvalue.offset = off; \
    lastVar = s; \
    s->type = constype(nil); \
    group_level = 1; \
}

#define makeParameter(s, n, st, cl, off) \
{ \
    newSym(s, n); \
    s->storage = st; \
    s->param = true; \
    s->class = cl; \
    s->symvalue.offset = off; \
    curparam->chain = s; \
    curparam = s; \
    s->type = constype(nil); \
}  

/*
 * NAME: string_alloc
 *
 * NOTE:     Assign 'len' character long buffer from pre-allocated 
 *	     string pool to 'buf'. 
 *	     Allocate new string pool if current pool is empty or
 *	     used up. Pool size is defined by #define POOLSIZE.
 *
 * PARAMETERS:
 *      buf	- string buffer
 *	len	- length of string buffer requested (including null end)
 *
 * RECOVERY OPERATION: Gives error message and exists if malloc fails.
 *
 */
#define string_alloc(buf, len)                                    \
{                                                                 \
    register Stringpool newpool;                                  \
                                                                  \
    if (string_left <= (len)) {                                   \
        /* Not enough space in pool for string */                 \
        /* get new pool...                     */                 \
        newpool = new(Stringpool);                                \
        if (!newpool) {                                           \
           fatal(getmsg(MSG_641,                                  \
			"1283-246 string_alloc: malloc error"));  \
        }                                                         \
        newpool->prevpool = stringpool;                           \
        stringpool = newpool;                                     \
        string_left = POOLSIZE;                                   \
    }                                                             \
    /* get space from name pool */                                \
    buf = &(stringpool->pool[POOLSIZE-string_left]);              \
    string_left -= (len);                                         \
}

/*
 * NAME: string_pool_free
 *
 * FUNCTION: Free all the string pools currently allocated.
 * 
 * PARAMETERS: None.
 *
 * RETURNS: None.
 */
public void string_pool_free ()
{
    Stringpool s, t;
    register Integer i;

    s = stringpool;
    while (s != nil) {
        t = s->prevpool;
        dispose(s);
        s = t;
    }
    stringpool = nil;
    string_left = 0;
}

/*
 * NAME: get_name
 *
 * FUNCTION: Return character buffer containing string defined
 *	     by parameters 'start' and 'end' pointers.  
 * 
 * NOTE:     Get character buffer from pre-allocated string pool
 *	     using string_alloc().
 * 	     Store name string into buffer and return its pointer. 
 *
 * PARAMETERS:
 *      start	- pointer to start of name string
 *	end	- pointer to end of name string (character after
 *		  last character of name or the null terminator)
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: pointer to character buffer containing name string.
 */
public char* get_name(start, end)
char* start;
char* end;
{
   int len = (int) (end - start) + 1;
   char* buf;

   /* get buffer from allocated string pool */
   string_alloc(buf, len);

   /* copy name into buffer and return */
   (void) strncpy(buf, start, len - 1);
   buf[len - 1] = '\0';
   return buf;
}

public entersym (name, np, instrtab, predefine_type)
String name;
SYMENT *np;
Boolean instrtab;
Boolean predefine_type;	/* Indicates this is being added as a predefined type */
{
    Symbol s = nil;	/* ensure this is nil before passing to publicRoutine */
    char *p;
    register Name n;
    char c;
    Boolean is_csect = false;
    char buf[100];
    extern Boolean lazy, process_vars;
    Symbol t = nil;

    if (isfcn(np)) {
	c = *csect_fcn_type;
	p = &csect_fcn_type[-1];
	is_csect = true;
    } else {
        p = index(name, ':');
	if (p == nil)
	    return;
        name = get_name(name, p);	/* get name buffer from name pool */
        instrtab = true;                /* no need to malloc it again...  */
        c = *(p+1);
    }

    /* If strip_ is set, we are dealing with fortran names */
    /* and need to strip '_' at the end of them */
    if (strip_ and (*(p-1)=='_')) 
        name[strlen(name)-1]='\0';

    if ((name[0] == '.') && ((c == EXTFUNCTION) || (c == EXTPROCEDURE))) {
	name++;	/* Strip possible leading '.' */
    }
    if (streq(name, "^")) {
      n = csectname;
    } else {
      n = identname(name, instrtab);
      if (is_csect)
	  csectname = n;
    }
    if (is_csect && lazy && process_vars) {
        Symbol funcdef;
	DemangledName d;
	if (curlang == cppLang) {
	    d = Demangle(n);
	    n = identname(d->qualName, true);
	}
	find (funcdef, n) where
	    (isroutine(funcdef)) &&

	    /* FOR normal cpp functions (FUNC and CSECTFUNC symbols), */
	    /* the demangled name is storage in both symbols under    */
	    /* symvalue.funcv.u.dName. 				      */
	    /* However, for cpp member function, this demangle name   */
	    /* is only storaged in CSECTFUNC symbols and the          */
	    /* corresponding MEMBER symbol for the member function.   */

	    ((curlang != cppLang) || 
	     ((funcdef->isCppFunction) && (!funcdef->isMemberFunc) &&
	     funcdef->symvalue.funcv.u.dName->mName == d->mName)) &&
	    ((funcdef->isCppFunction && !funcdef->isMemberFunc) || 
	     (funcdef->block == curblock))

	endfind (funcdef)
	if (funcdef != nil)
	    pushBlock(funcdef);
	if (curlang == cppLang)
	    EraseDemangledName(d);
	return;
    }

    if (nesting > 0 && addrstk[nesting] != NOADDR)
	chkUnnamedBlock();
    curchar = p + 2;
    switch (c) {
	case LABELNAME:
            /* add '$$' into label names that are just numbers */
            if (isdigit((int) name[0]))
            {
              sprintf(buf, "$$%.90s", name);
              n = identname(buf, false);
            }
	    newSym(s, n);
	    s->class = LABEL;
            s->symvalue.constval = build(O_LCON, np->n_value + text_reloc);
	    break; 

	case CONSTNAME:
	    newSym(s, n);
	    if (np->n_zeroes) 
               lastchar = &curchar[strlen(curchar) - 1];
	    else {
	       lastchar = name + (*((short *)(&debugtab[np->n_offset-2]))-2);
	    }
	    constName(s);
            lastchar = NULL;
	    break;

	case TYPENAME:
	    newSym(s, n);
	    s->ispredefined = predefine_type;
	    typeName(s);
	    break;

	case TAGNAME:
	    symbol_alloc(s);
	    if (isdigit(ident(n)[0]))
	    {
		/* then we have a C++ template class name, which must be */
		/* demangled.						 */
		DemangledName d;
		size_t length;
		char *name;

		/* add "___" so that the name becomes a member name */
		string_alloc(name, strlen(ident(n)) + 4);
		name[0] = '_'; name[1] = '_'; name[2] = '_'; 
		(void)strcpy(&name[3], ident(n));

		d = Demangle(identname(name, true));
		/* the name comes out in the form class::_ */

		length = strlen(d->qualName);
		string_alloc(name, length - 2);
		(void)strncpy(name, d->qualName, length - 3);
		name[length - 3] = '\0';
		n = identname(name, true);

		EraseDemangledName(d);
	    }
	    s->name = n;
	    s->language = curlang;

	    if (curlang == cppLang)
	    {
		enterblock(s);
		tagName(s);
		exitblock(s);
	    }
	    else
		tagName(s);

	    break;

	case MODULEBEGIN:
	    publicRoutine(&s, n, MODULE, np->n_value + text_reloc, false,
					  (Boolean) (np->n_sclass == C_ENTRY));
	    curmodule = s;
	    break;

	case EXTPROCEDURE:
	    publicRoutine(&s, n, PROC, np->n_value + text_reloc, false,
					  (Boolean) (np->n_sclass == C_ENTRY));
	    break;

	case PRIVPROCEDURE:
	    privateRoutine(&s, n, PROC, np->n_value + text_reloc,
					  (Boolean) (np->n_sclass == C_ENTRY));
	    break;

	case INTPROCEDURE:
	    publicRoutine(&s, n, PROC, np->n_value + text_reloc, true,
					  (Boolean) (np->n_sclass == C_ENTRY));
	    break;

	case EXTFUNCTION:
	    publicRoutine(&s, n, (is_csect) ? CSECTFUNC : FUNC,
			  np->n_value + text_reloc, false,
			  (Boolean) (np->n_sclass == C_ENTRY));
	    break;

	case PRIVFUNCTION:
	    privateRoutine(&s, n, FUNC, np->n_value + text_reloc,
					  (Boolean) (np->n_sclass == C_ENTRY));
	    break;

	case INTFUNCTION:
	    publicRoutine(&s, n, FUNC, np->n_value + text_reloc, true,
					  (Boolean) (np->n_sclass == C_ENTRY));
	    break;

        case GENERICFUNC:
            newSym(s, n);
            s->class = GENERIC;
            s->type = constype(nil);
            break;

	case EXTVAR:
	    extVar(&s, n, np->n_value);
	    break;

	case MODULEVAR:
            /* curstat has data_reloc added in already */
            makeVariable(s, n, np->n_value + curstat);
	    if (curcomm) {	/* Allow for ambiguous Pascal symbol groups */
	        ownVariable(s, np->n_value);
	    } else {
	        s->storage = EXT;
	        s->level = program->level;
	    }
	    s->block = curmodule;
	    getExtRef(s);
	    break;

	case OWNVAR:
	    makeVariable(s, n, np->n_value + data_reloc);
	    ownVariable(s, np->n_value);
	    getExtRef(s);
	    break;

        /* fortran pointee type variable */
        case FPOINTEE:
            /* get data for the pointee first */
            makeVariable(s, n, np->n_value + data_reloc);
            s->class = FPTEE;
            ownVariable(s, np->n_value);
            /* now learn about the pointer */
            getPointer(s, instrtab);
            break;

        /* fortran pointer type variable */
        case FPOINTER:
            find(t, n) where
                t->class == FPTR and t->block == curblock
                and t->block->block == curblock->block
            endfind(t);
            if (t!=nil)
               s = t;
            else {
                newSym(s, n);
                s->class = FPTR;
            }
            s->symvalue.offset = np->n_value + data_reloc;
            lastVar = s;
            group_level = 1;
            ownVariable(s, np->n_value);
            break;

	case REGVAR:
	    makeVariable(s, n, np->n_value);
	    s->storage = INREG;
	    break;

/*
	case FREGVAR:
	    makeVariable(s, n, NREG + np->n_value);
	    s->storage = INREG;
	    break;
*/

	case CONSTPARAM:
	    makeParameter(s, n, STK, CONST, np->n_value);
	    break;

	case VALUEPARAM:
	    makeParameter(s, n, STK, VAR, np->n_value);
#	    ifdef IRIS
		/*
		 * Bug in SGI C compiler -- generates stab offset
		 * for parameters with size added in.
		 */
		if (curlang == findlanguage(".c")) {
		    s->symvalue.offset -= size(s);
		}
#	    endif
	    break;

	case VARIABLEPARAM:
	    makeParameter(s, n, STK, REF, np->n_value);
	    break;

	case REGPARAM:
	    makeParameter(s, n, INREG, VAR, np->n_value);
	    break;

/*
	case FREGPARAM:
	    makeParameter(s, n, INREG, VAR, NREG + np->n_value);
	    break;
*/

	case REGVARPARAM:
	    makeParameter(s, n, INREG, REF, np->n_value);
	    break;

	default:	/* local variable */
	    --curchar;
	    makeVariable(s, n, np->n_value);
	    s->storage = STK;
	    break;
    }
    if (tracesyms && (s != nil)) {
	printdecl(s);
	fflush(stdout);
    }
}



private integer char_to_int( ch)
char ch;
{
  int num;

  if ((ch >= '0')&&(ch <= '9'))
    num = ch - '0';
  else if ((ch >= 'A')&&(ch <= 'F'))
    num = ch - 'A' + 10;
  else (*rpt_output)( stdout, " error in constructing Set Constant\n");
  return num;
}

/*
 * NAME: constName
 *
 * FUNCTION: Enter a named constant.
 *
 * PARAMETERS:
 *        s - Symbol describing constant
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: none   
 *
 * RETURNS: nothing
 *
 */

/*  1.2345678901234567E+000   <== 23 characters  */
#define DOUBLE_STRING_LENGTH 23

private constName (s)
Symbol s;
{
    double d,r;
    char *save_curchar;
    char *p, *q, *v, quote = '\'';
    char *buffer;
    int csize, i, j, front, back;
    Symbol t;
    quadf _qatof();

    s->class = CONST;
    skipchar(curchar, '=');
    p = curchar;
    ++curchar;
    switch (*p) {
	case 'b':
	    s->type = t_boolean;
	    s->symvalue.constval = build(O_LCON, getint());
	    break;

	case 'c':
	    s->type = t_char;
	    s->symvalue.constval = build(O_LCON, getint());
	    break;

	case 'i':
          {
            LongLong longlong_num;

            longlong_num = getlonglong();

            if ((longlong_num >= LONG_MIN) && (longlong_num <= LONG_MAX))
            {
              s->type = t_int;
              s->symvalue.constval = build(O_LCON, (int) longlong_num);
            }
            else
            {
              s->type = t_longlong;
              s->symvalue.constval = build(O_LLCON, longlong_num);
            }
	    break;
          }

	case 'r':
          {
            quadf quad_num;

            save_curchar = curchar;
	    while (*curchar != '\0' and *curchar != ';') {
		++curchar;
	    }
	    --curchar;

            if ((curchar - save_curchar) <= DOUBLE_STRING_LENGTH)
            {
              sscanf(save_curchar, "%lf", &d);
	      s->type = t_real;
	      s->symvalue.constval = build(O_FCON, d);
            }
            else
            {
              quad_num = _qatof(save_curchar);
              s->type = t_quad;
              s->symvalue.constval = build(O_QCON, quad_num);
            }
	    break;
          }

	case 's':
	    if ((*curchar == '\'') || (*curchar == '\"'))
		quote = *curchar;
	    skipchar(curchar, quote);
            p = curchar;
	    v = index(p, '\\');
            q = index(p, quote);
	    if (q and not v) {
                p = get_name(p, q);
                s->symvalue.constval = build(O_SCON, p, 0);
	    } else { 
		while (*curchar != quote) {
		 if ((*curchar == '\\')		/* skip next char if '\' */
		   and ( (*(curchar+1)=='\\') or (*(curchar+1)==quote) )) {
		       ++curchar;
		       if (lastchar and curchar >= lastchar) {
          		  warning(getmsg(MSG_426, 
				  "unexpected end of string for \"%s\""),
				  symname(s));
		          break;
		       }
		 }
		 ++curchar;
		 q = curchar;
	    	}
	    	string_alloc(buffer, (q - p + 1));
	    	for (i=0, j=i; i<=(q-p); i++) {
		   if ( (*(p+i)=='\\')
		      and ((*(p+i+1)=='\\') or (*(p+i+1)==quote)) )
		     i++;
	      	   buffer[j++] = *(p+i); 
		}
                buffer[q-p] = '\0';
	    	s->symvalue.constval = build(O_SCON, buffer, j);
	    }
	    s->type = s->symvalue.constval->nodetype;
	    break;

	case 'e':
	    s->type = constype(nil);
	    skipchar(curchar, ',');
	    s->symvalue.constval = build(O_LCON, getint());
	    break;

	case 'C':
          {
            Boolean quad_complex = false;
            quadf quad_real, quad_imag;

            /*  complex constants  */
     
	    while (isspace((int)(*curchar))) {
		++curchar;
	    }

            save_curchar = curchar;
	    while (*curchar != '\0' and *curchar != ',') {
		++curchar;
	    }

            
            if ((curchar - save_curchar) <= DOUBLE_STRING_LENGTH)
            {
	      sscanf(save_curchar, "%lf", &d);
            }
            else
            {
              quad_real = _qatof(save_curchar);
              quad_complex = true;
            }

	    skipchar(curchar, ',');

            if (!quad_complex)
            {
	      sscanf(curchar, "%lf", &r);
              s->type = t_complex;
	      s->symvalue.constval = build(O_KCON, d, r);
            }
            else
            {
              quad_imag = _qatof(curchar);
              s->type = t_qcomplex;
	      s->symvalue.constval = build(O_QKCON, quad_real, quad_imag);
            }
	    while (*curchar != '\0' and *curchar != ';') {
		++curchar;
	    }
	    --curchar;
	    break;
          }

        case 'S':
	    if (*curchar == '0')
	    {
	      getint();
              s->type = newSymbol(identname("$emptySet",true),
			curblock->level+1, SET, nil, nil);
	    } else 
	        s->type = constype(nil);
            skipchar(curchar, ',');
            getint();           /* number of element */
            skipchar(curchar, ',');
            csize = getint();    /* number of bits in constant */
            skipchar(curchar, ',');
            csize = (csize + BITSPERBYTE - 1) / BITSPERBYTE;
            string_alloc(buffer,csize);
            for (i=0; i<csize; i++)
            {
              front = char_to_int(*curchar);
              ++curchar;
              back = char_to_int(*curchar);;
              ++curchar;
              buffer[i] = (char) (front * 16 + back);
            }
            t = s->type->type;
            t = rtype(t);
            s->symvalue.constval = build(O_SETCON, &buffer[0], t);
            return;

        default:
	    s->type = t_int;
	    s->symvalue.constval = build(O_LCON, 0);
	    (*rpt_output)(stdout, getmsg(MSG_341,
			  "[internal error: unknown constant type '%c']"), *p);
	    break;
    }
    s->symvalue.constval->nodetype = s->type;
}

/*
 * Enter a type name.
 */

private typeName (s)
Symbol s;
{
    register integer i;
    boolean oldentry = false;
    int typenum;
    boolean shared_entry = false;

    s->class = TYPE;
    s->language = curlang;
    s->block = curblock;
    s->level = curblock->level + 1;
    i = getint();
    if (i == 0) {
	panic(getmsg(MSG_345, "bad input on type \"%s\" at \"%s\""), 
			      symname(s), curchar);
    } 
    while (i >= ntypes) {
	/* get around problem of a full typetable by extending the table */
        int temp_ntypes = ntypes + NTYPES;
        if (typetable = (Symbol *) realloc(typetable, 
				(temp_ntypes+TP_NTYPES+1)*sizeof(Symbol))) {
           bset0(&typetable[ntypes+TP_NTYPES+1], 
				(temp_ntypes - ntypes) * sizeof(Symbol));
	   ntypes = temp_ntypes;
	} else
           fatal( NLcatgets(scmc_catd, MS_object, MSG_346,
                 "1283-230 too many types in file \"%s\""), curfilename());
    }
    typenum = i + TP_NTYPES;
    /*
     * Handle C typedefs that don't create new types,
     * e.g. typedef unsigned int Hashvalue;
     *  or  typedef struct blah BLAH;
     */
    if (*curchar != '=') {
	s->type = typetable[typenum];
	if (s->type == nil) {
	    symbol_alloc(s->type);
	    entertype(typenum, s->type);
	}
    } else {
	if (typetable[typenum] != nil) {
	    /* check language pointer to see if it's an empty entry */
	    if (stab_compact_level >= 2 && 
		                       typetable[typenum]->language != nil) {
	       shared_entry = true;
	    } else {
	       typetable[typenum]->language = curlang;
	       typetable[typenum]->class = TYPE;
	       if (nilname(typetable[typenum]))
	          typetable[typenum]->name = s->name;
	       typetable[typenum]->type = s;
	       oldentry = true;
	    }
	} else {
	    entertype(typenum, s);
	}
	skipchar(curchar, '=');
	/* For unnamed symbol, dbx stores the type symbol of this */
	/* unnamed symbol in the typetable. Here we check if the  */
	/* symbol for this typetable entry is the type symbol of  */
	/* an unnamed symbol. If so, use this symbol for s->type, */
	/* else use it's type symbol. (See also comment below...) */
	if (shared_entry)
	   s->type = (typetable[typenum]->WasUnnamedType) ?
				typetable[typenum] : typetable[typenum]->type;
	else
	   s->type = constype(nil);
	if (nilname(s) && !shared_entry) {
	    if (oldentry) {
		memcpy((char *)typetable[typenum], (char *)s->type,
							sizeof(struct Symbol));
	    } else {
	        entertype(typenum, s->type);
	    }
	    /* Record that the type symbol of an unnamed symbol is stored   */
	    /* in the typetable entry instead of the unnamed symbol itself. */
	    typetable[typenum]->WasUnnamedType = true;
	}
    }
}

/*
 * Enter or update a template class definition
 */

void enterTemplateClass(s, argStart)
Symbol s;
char *argStart;
{
    /* To implement the template exception mechanism, for each class */
    /* template (the class from which template classes were created) */
    /* we create a Symbol of type TAG and hang each of the template  */
    /* classes themselves off this Symbol's chain.		     */

    Symbol t;
    char *templateClassName = ident(s->name);
    TemplateClassListEntry tclEntry = new(TemplateClassListEntry);
    unsigned long tcNameLength = argStart - templateClassName;
    Name classTemplateName;
    char *className;

    string_alloc(className,(tcNameLength + 1));
    strncpy(className, templateClassName, tcNameLength);
    className[tcNameLength] = '\0';

    classTemplateName = identname(className, true);

    /* search for an existing class template definition */
    find(t, classTemplateName) where
	t->class == TAG and t->isClassTemplate
    endfind(t);

    if (t == nil)
    {
	/* create a class template. */
	t = newSymbol(classTemplateName, s->level, TAG, nil, nil);
	t->block = s->block;
	t->language = cppLang;
	t->isClassTemplate = true;
	insertsym(t);
    }

    tclEntry->templateClass = s;
    tclEntry->next = t->symvalue.template.list;
    t->symvalue.template.list = tclEntry;
}

/*
 * Enter a tag name.
 */

private tagName (s)
Symbol s;
{
    Boolean nestedClass = false;
    Boolean forwardDeclsSeen = false;
    Boolean shared_entry = false;

    integer i = getint();

    if (i == 0) {
	panic(getmsg(MSG_347, "bad input on tag \"%s\" at \"%s\""), 
			      symname(s), curchar);
    } else if (i >= ntypes) {
        fatal( NLcatgets(scmc_catd, MS_object, MSG_346,
                "1283-230 too many types in file \"%s\""), curfilename());
    }

    s->class = TAG;
    skipchar(curchar, '=');
    if (*curchar == T_CLASS)
    {
	Symbol t;

	t = lookup(s->name);
	while (t != nil)
	{
	    /* Try to find another definition of class s. If one is found, */
	    /* see if its size is zero. If so, it is a forward reference   */
	    /* whose type must set later (if not now).                     */
	    if (rtype(t)->class == CLASS && cpp_equivalent(s, t))
	    {
                /*  there are problems here with different classes
                      of the same name.  Moving the break inside the
                      if takes care of that, but breaks the case where
                      the same class is used in multiple files.  More
                      work required here...  */

		if (rtype(t)->symvalue.class.offset == 0) {
		    forwardDeclsSeen = true;
                }
                break;
	    }
	    t = t->next_sym;
	}

	if (t != nil && !forwardDeclsSeen /* t is real definition */) {
			/* if current definition is equivalent
			   to defining type */
		while (typetable[i + TP_NTYPES] == t->type)
			t = t->type;
    		s->type = t->type;
		
			/* process any continuation decl entries */
		while (curchar = (strchr(curchar,'\0')-1)) {
    			if (*curchar == '\\' or *curchar == '?') { 
				curchar = getcont(); 
    			} 
			else break;
		}
	}
	else
	{
	    char *argStart;

	    s->type = constype(nil);
	    if (s->type->symvalue.class.offset > 0 /* real definition */ &&
	        (argStart = strchr(ident(s->name), '<')) != nil)
	    {
		/* we've got a real definition of a template class. Create */
		/* or update a template class Symbol with "s" hanging off  */
		/* it. Note that this class is in global scope.            */
		enterTemplateClass(s, argStart);
	    }
	}

 	if (forwardDeclsSeen && s->type->symvalue.class.offset > 0)
	{
	    /* set the type of all forward references seen to the type of */
	    /* the non-forward reference s.				  */
	    t = lookup(s->name);
	    while (t != nil)
	    {
	        if (rtype(t)->class == CLASS && cpp_equivalent(s, t) &&
		    rtype(t)->symvalue.class.offset == 0)
		{
		    delete(t);
		    t->type = s->type;
		}
		t = t->next_sym;
	    }
	}
    }
    else if (stab_compact_level >= 2 &&
	     typetable[i + TP_NTYPES] != nil && 
	     typetable[i + TP_NTYPES]->language != nil) {
            s->type = typetable[i + TP_NTYPES]->type;
	    shared_entry = true;
    } else
	s->type = constype(nil);

    if (typetable[i + TP_NTYPES] != nil) {
	if (curlang == cppLang) {
	    /* nested classes are not inserted into the symbol table  */
	    nestedClass = rtype(s->block)->class == CLASS;
	    /* real declarations get the same block as its forward decl. */
	    s->block = typetable[i + TP_NTYPES]->block;
	    typetable[i + TP_NTYPES]->name = nil;
	}
	if (!shared_entry) {
	   typetable[i + TP_NTYPES]->language = curlang;
	   typetable[i + TP_NTYPES]->class = TAG;
	   typetable[i + TP_NTYPES]->type = s;
	}
    } else {
	entertype(i + TP_NTYPES, s);
    }

    if (curlang == cppLang)
    {
	if (!nestedClass)
	    insertsym(s);
    }
    else
    {
	Symbol t;
	if (curlang == pascalLang) 
	{
	    t = insert(s->name);
	    t->class = TYPE;
	} 
	else
	{
	    char *buf;
	    string_alloc(buf, (strlen(ident(s->name)) + 3));
	    strcpy(buf, "$$");
	    t = insert(identname(strcat(buf, ident(s->name)), true));
	    t->class = TAG;
	}
	t->type = s->type;
	t->block = s->block;
	t->language = s->language;
    }
}


/*
 * NAME: setupMemberFunc
 *
 * FUNCTION: Set up double links between MEMBER symbol and its corresponding
 *	     FUNC or CSECTFUNC symbol, plus fill in other data fields.
 *
 * PARAMETERS:
 *      t      - FUNC or CSECTFUNC symbol for the member function
 *	d      - DemangledName structure containing names of the function
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
void setupMemberFunc (t, d)
Symbol t;
DemangledName d;
{
   t->isCppFunction = true;

   if (ident(d->name) != d->qualName) {
      /* The function is a member function. Find it in the list   */
      /* of unattached member function symbols, and doubly link   */
      /* the real function Symbol with the member function Symbol */
      /* See the notes at the end of symbols.c regarding inlines. */

      Symbol p = RetrieveMemFunc(d->mName);

      if (p != nil) {
	 Symbol s;
	 /* p is nil when the function is a compiler generated       */
	 /* function, but it can also be nil sometimes when member   */
	 /* func is not mentioned in class declaration stabstring.   */

	 /* link the two symbols */
	 t->isMemberFunc = true;
	 t->symvalue.funcv.u.memFuncSym = p;
	 p->symvalue.member.attrs.func.funcSym = t;

	 /* As there is only one MEMBER symbol for a member function */
	 /* we must be sure that the FUNC symbol gets the same block */ 
	 /* as the class which it is a member of, unless it is an    */
	 /* inline function, in which case its block needn't change  */
	 /* (in order to distinguish different inlines).		    */

	 if (!p->symvalue.member.attrs.func.isInline)
	    t->block = p->block->block;

	 /* lastly, give the member function Symbol its real name */
	 p->name = d->name;
	 p->symvalue.member.attrs.func.dName = d;
      }
      /* for C++ member func not mentioned in a class declaration stab */
      else
         t->symvalue.funcv.u.dName = d;
    }
    else
      t->symvalue.funcv.u.dName = d;
}
     

private Symbol enterRoutine();
/*
 * Setup a symbol entry for a public procedure or function.
 *
 * If it contains nested procedures, then it may already be defined
 * in the current block as a MODULE.
 */

private publicRoutine (s, n, class, addr, isinternal, isentry)
Symbol *s;
Name n;
Symclass class;
Address addr;
boolean isinternal;
Boolean isentry;
{
    Symbol nt, t, u;
    Symbol funcdecl;
    Desclist *fcndesc;
    DemangledName d = nil;
    boolean cppfunc;
    extern boolean lazy;

    if (!keep_linkage)
      fcndesc = ElimLinkage(n);

    if (cppfunc = (curlang == cppLang))
    {
	d = Demangle(n);
	n = identname(d->qualName, true);
    }
    newSym(nt, n);

    if (isinternal) {
	markInternal(nt);
    }
    else { /* eliminate any previously defined externals of the same name */
        for (t = lookup(n); t != nil;) {
	    u = t;
	    t = t->next_sym;
	    if (u->name == n && u->storage == EXT && u->class != VAR &&
		addrtoobj(addr) == addrtoobj(u->symvalue.offset)) {
		delete(u);
	    }
	}
    }
    if ((funcdecl = enterRoutine(nt, class, isentry, d)) != nil) {
	/* search the member function table after the FUNC entry is */
	/* read. In lazy mode, the reading order is CSECTFUNC, DECL */
	/* (class decl - enter in member func table), and then FUNC */
	if (cppfunc && lazy)
	    setupMemberFunc(funcdecl,d);
	
	if (*s == nil)		/* don't overwrite existing symbol */
	    *s = nt;
	else if (cppfunc)
	    EraseDemangledName(d);
	if (isCOBOL && isentry)
	{
	    funcdecl->symvalue.funcv.beginaddr = 
		csectaddr + (addr - text_reloc);
	}
	return;
    }
    find(t, n) where
	t != nt and t->class == MODULE and t->block == nt->block
    endfind(t);
    if (t == nil) {
	t = nt;
    } else {
	if (!intoexterns) {
	   t->language = nt->language;
	   t->class = nt->class;
	   t->type = nt->type;
	   t->chain = nt->chain;
	   t->symvalue = nt->symvalue;
	   nt->class = EXTREF;
	   nt->symvalue.extref = t;
	   delete(nt);
	   curparam = t;
	   changeBlock(t); 
	} else {
	   nt->block = t;
	   t = nt;
	}
    }
    if (t->block == program) {
	t->level = program->level;
    } else if (t->class == MODULE) {
	t->level = t->block->level;
    } else if (t->block->class == MODULE) {
	t->level = t->block->block->level;
    } else {
	t->level = t->block->level + 1;
    }
    t->symvalue.funcv.src = false;
    t->symvalue.funcv.inline = false;
    t->symvalue.funcv.dontskip = false;
    if (cppfunc)
    {
	/* For normal (non-lazy) startup, we can search the member func */
	/* table to create links after the CSECTFUNC entry. (order is   */
	/* DECL, CSECTFUNC, FUNC. 					*/
        setupMemberFunc(t,d);
    }
     
    if (isentry && !isCOBOL) {
	markEntry(t);
    } else {
        t->symvalue.funcv.isentry = false;
    }
 
    t->symvalue.funcv.fcn_desc = fcndesc;
    if (class != CSECTFUNC)
        prolloc(t) = csectaddr + (addr - text_reloc); /* addr has text_reloc */
    else
        prolloc(t) = addr;
    if (linenoptr) {
        t->symvalue.funcv.beginaddr = curlinep[ln_index].addr + text_reloc;
        t->symvalue.funcv.src = true;
    } else {
        t->symvalue.funcv.beginaddr = prolloc(t);
    }
    newfunc(t, prolloc(t), curfilep);
    lastfuncprol = prolloc(t);
    lastfuncaddr = t->symvalue.funcv.beginaddr;
    *s = t;
}

/*
 * Setup a symbol entry for a private procedure or function.
 */

private privateRoutine (s, n, class, addr, isentry)
Symbol *s;
Name n;
Symclass class;
Address addr;
Boolean isentry;
{
    Symbol t;
    boolean isnew;
    Desclist *fcndesc;

    if (!keep_linkage)
      fcndesc = ElimLinkage(n);

    find(t, n) where
	t->level == curmodule->level and t->class == class
    endfind(t);
    if (t == nil) {
	isnew = true;
	newSym(t, n);
    } else {
	isnew = false;
    }
    t->language = curlang;
    isnew = isnew && (enterRoutine(t, class, isentry, nil) == nil);
    if (isentry && !isCOBOL) {
	markEntry(t);
    }
    if (isnew) {
	t->symvalue.funcv.src = false;
	t->symvalue.funcv.inline = false;
	t->symvalue.funcv.fcn_desc = fcndesc;
        prolloc(t) = addr;
    	if (linenoptr) {
            t->symvalue.funcv.beginaddr = curlinep[ln_index].addr + text_reloc;
            t->symvalue.funcv.src = true;
        } else {
            t->symvalue.funcv.beginaddr = prolloc(t);
        }
        newfunc(t, prolloc(t), curfilep);
        lastfuncprol = prolloc(t);
        lastfuncaddr = t->symvalue.funcv.beginaddr;
	findbeginning(t);
    }
    *s = t;
}

/*
 * new_block - Enter a new block and update the routine list.
 */
private new_block(s)
Symbol s;
{
        struct routinestack *ptr;

	enterblock(s);
        ptr = (struct routinestack*)malloc(sizeof(struct routinestack));
        ptr->name = curparam = s;
        ptr->next = nil;
        ptr->back = curroutine;
	if (curroutine)
	  curroutine->next = ptr;
        curroutine = ptr;
}

/*
 * Set up for beginning a new procedure, function, or module.
 * If it's a function, then read the type.
 *
 * If the next character is a ",", then read the name of the enclosing block.
 * Otherwise assume the previous function, if any, is over, and the current
 * routine is at the same level.
 */

private Symbol enterRoutine (s, class, alt_entry, d)
Symbol s;
Symclass class;
Boolean alt_entry;
DemangledName d;
{
    Symbol funcdef;
    extern boolean lazy, process_vars;
    extern int bflevel;

    s->class = class;
    if ((class == FUNC) || (class == CSECTFUNC))
        s->type = constype(nil);

    /* We have both a csect entry and a C_FUN entry for procs now.  We really
       only want one symbol, so once we get the type information, go with it */
    if (class == FUNC || class == PROC) {
	find (funcdef, s->name) where
	    (funcdef != s) &&
	    (isroutine(funcdef)) &&

	    /* more checking needed for cpp overloaded function, esp.  */
	    /* when doing lazy read where the entries are not in order */
	    (!lazy || !d || 
	     (!funcdef->isMemberFunc && 
	      funcdef->symvalue.funcv.u.dName->mName == d->mName)) &&

	    ((s->block == funcdef) || (s->block == funcdef->block) ||
	     /* cpp member function need not check blocks */
	     (d && (ident(d->name) != d->qualName)) ||
	     (isCOBOL && (s->block->block == funcdef->block)))
	endfind (funcdef)
	if (funcdef != nil) {
	   funcdef->language = s->language;
	   funcdef->class = s->class;
	   funcdef->type = s->type;
	   funcdef->chain = s->chain;
	   s->class = EXTREF;
	   s->symvalue.extref = funcdef;
	   curparam = funcdef;
	   if (alt_entry) {
	       if (isCOBOL) {
		   if (nosource(funcdef))
		       markEntry(funcdef);
	       }
	       else {
	           markEntry(funcdef);
	           if (s->block != funcdef) {
                       Symbol funcdefblock = whatblock(prolloc(funcdef));
                       /* make sure we are not already in that block */
                       if (funcdefblock != curblock) {
                          /* before we push container of entry points */
                          /* as a block and update its block pointer. */
                          pushBlock(funcdefblock);
                          funcdef->block = funcdefblock;
                       }
		   }
	       }
	   }
	   delete(s);
	   s = funcdef;
	   return funcdef;	/* found a funcdef */
	}
	else if (isCOBOL && (curblock->class == PROC || curblock->class == FUNC
			     || curblock->class == CSECTFUNC)
	         && !strcmpi(curblock->name->identifier, s->name->identifier)
	        ) { /* additional COBOL entry points */
	    delete(s);
	    s = curblock;
	    return curblock;		/* curblock is the funcdef */
	}
    }
    if (s->class != MODULE) {
	getExtRef(s);
    } else if (*curchar == ',') {
	++curchar;
    }
    if (*curchar != '\0') {
	exitblock();
	enterNestedBlock(s);
    } else {
	if ( (curblock->class == CSECTFUNC && s->class == CSECTFUNC) &&
	     (!lazy || (bflevel <= 0)) ) {
	    exitblock();
	}
	if (class == MODULE) {
	    exitblock();
	}
	new_block(s);
    }
    curparam = s;
    return nil;			/* no funcdef found */
}

/*
 * Handling an external variable is tricky, since we might already
 * know it but need to define it's type for other type information
 * in the file.  So just in case we read the type information anyway.
 */

Symbol staticMemberList = nil;

private extVar (symp, n, off)
Symbol *symp;
Name n;
integer off;
{
    Symbol s;

    find(s, n) where
	s->level == program->level and 
	((s->class == VAR) or (s->class == TOCVAR))
    endfind(s);
    if (s == nil) {
	makeVariable(s, n, off);
	s->storage = EXT;
	s->level = program->level;
	s->block = curmodule;
	getExtRef(s);
    } else {
	s->language = curlang;
	s->type = constype(nil);
	if (s->language == cppLang) {
	    /* make an optimistic guess at whether the variable is a static */
	    /* member. If it may be, we will search "staticMemberList".     */
	    Name varName = s->name;
	    String name = ident(varName);
	    int length = strlen(name);
	    int i = 1;

	    while (true)
	    {
		while (i < length && name[i] != '_')
		    i++;

		if (    /* failure */ i == length 
		    ||  /* success */ i < length + 3 && name[i + 1] == '_' && 
			(name[i + 2] == 'Q' || isdigit(name[i + 2]))) 
		    break;

		i += 1;
	    }

	    /* Try to find the symbol in staticMemberList. Set its name to  */
	    /* the fully qualified name of the member symbol, and remove it */ 
	    /* from the list.						    */
	    if (i < length)
	    {
		Symbol p = staticMemberList;
		Symbol prev = nil;
		while (p != nil)
		{
		    if (p->name == varName)
		    {
			p->symvalue.member.attrs.staticData.varSym = s;
			p->name = p->symvalue.member.attrs.staticData.
				  dName->name;
			s->name = identname(p->symvalue.member.attrs.staticData.
				            dName->qualName, true);
			s->isStaticMember = true;

			if (prev == nil)
			    staticMemberList = p->chain;
			else
			    prev->chain = p->chain;
			p->chain = nil;

			break;
		    }
		    else
		    {
			prev = p;
			p = p->chain;
		    }
		}
	    }
	}
    }
    *symp = s;
}


/*
 * getPointer() links symbols of POINTER and POINTEE variables 
 * together so that we can get address of the pointee later 
 * from the memory space of the pointer. 
 */ 
private getPointer(s, instrtab)
Symbol s;
Boolean instrtab;
{
    char *p;
    Name n;
    Symbol t;

    if (*curchar == ':' and *(curchar + 1) != '\0') {
        /* find out which is the pointer                    */
	/* Pointer name is stored in stabstring after a ':' */
        p = index(curchar + 1, ',');
        if (p != nil) {
            instrtab = true;                /* no need to malloc it again... */
            n = identname(get_name(curchar + 1, p), instrtab);
            curchar = p + 1;
        } else {
            n = identname(curchar + 1, instrtab);
        }
        /* Then see if the pointer has been read in */
        find(t, n) where
            (t->class == FPTR or t->class == REF)
            and t->block == curblock
            and t->block->block == curblock->block
        endfind(t);
	/* If not, create a dummy one and get the address later */
        if (t == nil) {
            t = insert(n);
            t->language = s->language;
            t->class = FPTR;
            t->block = curblock;
            t->level = curblock->level + 1;
        }
	/* if is parameter, change class to FPTR */
        if (t->class == REF) {
            t->class = FPTR;
        }
	/* links up pointer and pointee */
        s->chain = t;
    }
}


/*
 * Check to see if the stab string contains the name of the external
 * reference.  If so, we create a symbol with that name and class EXTREF, and
 * connect it to the given symbol.  This link is created so that when
 * we see the linker symbol we can resolve it to the given symbol.
 */

private getExtRef (s)
Symbol s;
{
    char *p;
    Name n;
    Symbol t;

    if (*curchar == ',' and *(curchar + 1) != '\0') {
	p = index(curchar + 1, ',');
	if (p != nil) {
            n = identname(get_name(curchar + 1, p), true);
	    curchar = p + 1;
	} else {
	    n = identname(curchar + 1, true);
	}
	t = insert(n);
	t->language = s->language;
	t->class = EXTREF;
	t->block = program;
	t->level = program->level;
	t->symvalue.extref = s;
    }
}

/*
 * Find a block with the given identifier in the given outer block.
 * If not there, then create it.
 */

private Symbol findBlock (id, m)
String id;
Symbol m;
{
    Name n;
    Symbol s;

    n = identname(id, true);
    find(s, n) where s->block == m and isblock(s) endfind(s);
    if (s == nil) {
	s = insert(n);
	s->block = m;
	s->language = curlang;
	s->class = MODULE;
	s->level = m->level + 1;
    }
    return s;
}

/*
 * Enter a nested block.
 * The block within which it is nested is described
 * by "module{:module}[:proc]".
 */

private enterNestedBlock (b)
Symbol b;
{
    register char *p, *q;
    Symbol m;

    q = curchar;
    p = index(q, ':');
    m = program;
    while (p != nil) {
        q = get_name(q, p);          /* findBlock expect name be allocated */
	m = findBlock(q, m);
	q = p + 1;
	p = index(q, ':');
    }
    if (*q != '\0') {
	m = findBlock(q, m);
    }
    b->level = m->level + 1;
    b->block = m;
    pushBlock(b);
}

/*
 * Enter a statically-allocated variable defined within a routine.
 *
 * Global BSS variables are chained together so we can resolve them
 * when the start of common is determined.  The list is kept in order
 * so that f77 can display all vars in a COMMON.
 *
 * C++ static member variables must be linked from their class members
 * entries to "s".
 */

private ownVariable (s, addr)
Symbol s;
Address addr;
{
    s->storage = EXT;
    s->level = curblock->level + 1;
    if (curcomm) {
	if (commchain != nil) {
	    commchain->symvalue.common.chain = s;
	} else {
	    curcomm->symvalue.common.offset = (int) s;
	}			  
	commchain = s;
	s->symvalue.common.offset = addr + curcomm->symvalue.common.com_addr;
	s->symvalue.common.chain = nil;
    } else if (curstat) {
	s->symvalue.common.offset = addr + curstat;
	s->symvalue.common.chain = nil;
    }
}

/*
 * Produce a clone symbol with a true size info for Pascal array of
 * records or strings. This is needed for Pascal array with paddings
 * between elements. This routine puts the padded sizes into the type
 * symbol structure of the element (.symvalue.offset) and store the
 * previous unpadded size in ".symvalue.field.length", second integer
 * in the symvalue struct.
 */
private Symbol makeClone(t, size)
Symbol t;
integer size;
{
  Symbol s;
  integer tsize;

  if (t == nil) return t;
  if (t->class == RECORD or t->class == PACKRECORD or
      t->class == VARNT or t->class == STRING)
  {
    s = newSymbol(t->name, t->level, t->class, t->type, t->chain);
    tsize = (size + BITSPERBYTE - 1) / BITSPERBYTE;
    s->symvalue.field.length = t->symvalue.offset;
    s->symvalue.offset = tsize;
  }
  else s = newSymbol(t->name, t->level, t->class,
                     makeClone(t->type, size), makeClone(t->chain, size));
  s->language = curlang;
  return s;
}

private Symbol cloneSym (t, size)
Symbol t;
integer size;
{
  if (rtype(t)->class != RECORD and rtype(t)->class != PACKRECORD
      and rtype(t)->class != VARNT and rtype(t)->class != STRING)
    return t;
  else return makeClone(t, size);
}

/*
 * Construct a subrange type.
 */

private consSubrange (t, class)
Symbol t;
Symclass class;
{
    LongLong lower; 
    uLongLong upper;

    t->class = class;
    t->type = constype(nil);
    skipchar(curchar, ';');
    chkcont(curchar);
    t->symvalue.rangev.lowertype = getRangeBoundType();
    lower = getlonglong();
    if (lower >= LONG_MIN)
    {
      t->symvalue.rangev.lower = (long) lower;
    }
    else
    {
      t->symvalue.rangev.lower = 0;
    }

    if (lower < 0)
      t->symvalue.rangev.is_unsigned = 0;
    else
      t->symvalue.rangev.is_unsigned = 1;

    if (t->symvalue.rangev.lowertype == R_STATIC)
        t->symvalue.rangev.lower = t->symvalue.rangev.lower + curstat;
    skipchar(curchar, ';');
    chkcont(curchar);
    t->symvalue.rangev.uppertype = getRangeBoundType();
    upper = getlonglong();
    if (upper <= ULONG_MAX)
    {
      t->symvalue.rangev.upper = (long) upper;
      t->symvalue.rangev.size = getrangesize(t, lower, upper);
    }
    else
    {
      t->symvalue.rangev.upper = 0;
      t->symvalue.rangev.size = sizeofLongLong;
    }

    if (t->symvalue.rangev.uppertype == R_STATIC)
        t->symvalue.rangev.upper = t->symvalue.rangev.upper + curstat;
}

/*
 * Construct a floating point type.
 */

private consReal (t)
Symbol t;
{
    t->class = REAL;
    t->type = constype(nil);
    skipchar(curchar, ';');
    chkcont(curchar);
    t->symvalue.size = getint();
}


/*
 * Construct a wide character point type.
 */

private consWide (t)
Symbol t;
{
    t->class = WIDECHAR;
    t->type = constype(nil);
    t->symvalue.size = 2;
}


/*
 * Construct a FORTRAN complex type.
 */

private consComplex (t)
Symbol t;
{
    t->class = COMPLEX;
    t->type = constype(nil);
    skipchar(curchar, ';');
    chkcont(curchar);
    t->symvalue.size = getint();
}

/*
 * Construct a Pascal stringptr type.
 */

private consStringptr (t)
Symbol t;
{
    t->class = STRINGPTR;
    t->type = t;
    /*
    t->type = typetable[TP_STRNGPTR + TP_NTYPES];
    */
}

/*
 * Construct a Pascal string type.
 */

private consString (t, class)
Symbol t;
Symclass class;
{
    int d, n;
  
    d = curblock->level + 1;
    t->class = class;
    t->type = constype(nil);
    if (class == STRING)
      t->type = newSymbol( nil, d, TYPE, t_char, nil);
    else t->type = newSymbol( nil, d, TYPE, t_gchar, nil);
    skipchar(curchar, ';');
    chkcont(curchar);
    n = getint();
    t->chain = newSymbol(nil, d, RANGE, t_int, nil);;
    t->symvalue.size = n + 2;		   /* Account for length field */
    t->chain->symvalue.rangev.lower = -1;  /* index -1 and 0 for length bits */
    t->chain->symvalue.rangev.upper = n;
}

/*
 * Handle a type which has a multiple-unit base type, e.g. character*N
 */

private consMulti (t)
Symbol t;
{
    char *savecp;

    t->class = CHARSPLAT;
    t->type = constype(nil);
    skipchar(curchar, ';');
    chkcont(curchar);
    savecp = curchar;
    t->symvalue.multi.sizeloc = getRangeBoundType(); /* Dynamically bounded? */
    t->symvalue.multi.size = getint();        /* Length or offset field */
    if (t->symvalue.multi.sizeloc == R_STATIC)
        t->symvalue.multi.size = t->symvalue.multi.size + curstat;
    if (t->symvalue.multi.sizeloc != R_CONST)	/* character*(*) variable */
        t->class = FSTRING;
}



/*
 * Figure out the bound type of a range.
 *
 * Some letters indicate a dynamic bound, ie what follows
 * is the offset from the fp which contains the bound; this will
 * need a different encoding when pc a['A'..'Z'] is
 * added; J is a special flag to handle fortran a(*) bounds
 */

private Rangetype getRangeBoundType ()
{
    Rangetype r;

    switch (*curchar) {
	case 'A':
	    r = R_ARG;
	    curchar++;
	    break;

        case 'a':
            r = R_REGARG;
            curchar++;
            break;

        case 'S':
            r = R_STATIC;
            curchar++;
            break;

	case 'T':
	    r = R_TEMP;
	    curchar++;
	    break;

        case 't':
            r = R_REGTMP;
            curchar++;
            break;

	case 'J': 
	    r = R_ADJUST;
	    curchar++;
	    break;

	default:
	    r = R_CONST;
	    break;
    }
    return r;
}

/*
 * Construct a dynamic array descriptor.
 */

private consDynarray (t, c)
register Symbol t;
Symclass c;
{
    t->class = c;
    t->symvalue.ndims = getint();
    skipchar(curchar, ',');
    t->type = constype(nil);
    t->chain = t_int;
}

private boolean OptMultiBaseSpec()
{
    boolean multiBaseSpec;

    if (*curchar == 'm') {
	multiBaseSpec = true;
	curchar += 1;
    }
    else
	multiBaseSpec = false;
    return multiBaseSpec;
}

private boolean OptVBaseSpec()
{
    boolean vBaseSpec;

    if (*curchar == 'v') {
	vBaseSpec = true;
	curchar += 1;
    }
    else
	vBaseSpec = false;
    return vBaseSpec;
}

private void AnonMember(anon)
boolean *anon;
{
    if (*curchar == 'a') {
	curchar += 1;
	*anon = true;
    }
    else
	*anon = false;
}

private void CompilerGenerated(isCompGen)
int *isCompGen;
{
    if (*curchar == 'c') {
	*isCompGen = true;
	curchar += 1;
    }
    else
	*isCompGen = false;

}

/* Process (virtual) access type for member functions */
private VirtualSpec(attrs)
unsigned *attrs;
{
    if (*curchar == 'v') {
	curchar += 1;
        if (*curchar == 'p') {
	    *attrs = CPPPUREVIRTUAL;
	    curchar += 1;
	}
	else
	    *attrs = CPPVIRTUAL;
    }
    else
	*attrs = CPPREAL;
}

/* Process access type for member functions */
private AccessSpec(access)
unsigned *access;
{
    switch (*curchar) 
    {
        case 'i':
	    *access = PRIVATE;
	    break;
        case 'o':
	    *access = PROTECTED;
	    break;
        case 'u':
	    *access = PUBLIC;
	    break;
        default:
	    panic(getmsg(MSG_631,
		  "expected access type 'i', 'o', or 'u', saw '%1$s'"),
		  curchar);
   }
   curchar += 1;
   chkcont(curchar);
}
	
/* Process (virtual) access type for base classes */
private VirtualAccessSpec(isVirtual, access)
unsigned *isVirtual, *access;
{
    if (*curchar == 'v') {
	*isVirtual = true;
	curchar += 1;
    }
    else
	*isVirtual = false;

    switch (*curchar) 
    {
        case 'i':
	    *access = PRIVATE;
	    break;
        case 'o':
	    *access = PROTECTED;
	    break;
        case 'u':
	    *access = PUBLIC;
	    break;
        default:
	    panic(getmsg(MSG_631,
		  "expected access type 'i', 'o', or 'u', saw '%1$s'"), 
                  curchar);
   }
   curchar += 1;
   chkcont(curchar);
}
	
/* Construct base class to a class */
private BaseClass (t)
register Symbol t;
{
   unsigned isVirtual, access;

   t->language = curlang;
   VirtualAccessSpec(&isVirtual, &access);
   t->symvalue.baseclass.isVirtual = isVirtual;
   t->symvalue.baseclass.access = access;
   t->symvalue.baseclass.offset = getint();
   skipchar(curchar, ':');
   t->type = constype(nil);
   t->name = forward(t->type)->name;
   if (*curchar == ',')
      ++curchar;
}

/* Determine member function type */
private FuncType(u)
Symbol u;
{
    switch (*curchar) {
	case 'f':
	    u->symvalue.member.attrs.func.kind = CPPFUNC;
	    break;
	case 'c':
	    u->symvalue.member.attrs.func.kind = CPPCTOR; /* constructor */
	    break;
	case 'd':
	    u->symvalue.member.attrs.func.kind = CPPDTOR; /* destructor */
	    break;
	default:
	    panic(getmsg(MSG_632, 
	          "expected member function types, found '%1$s'"), curchar);
    }
    ++curchar;
}

/* Gather member function attrs */
private MemberFuncAttrs(u)
Symbol u;
{
      int isInline = false, isConst = false, isVolatile = false;

      while (*curchar != ':' and *curchar != ';') 
      {
	 switch (*curchar) 
	 {		/* process member attrs */
	    case 's':
		u->symvalue.member.isStatic = true;
		break;
	    case 'i':
		isInline = true;
		break;
	    case 'k':
		isConst = true;
		break;
	    case 'V':
		isVolatile = true;
		break;
	    default:
	        panic(getmsg(MSG_633, 
	              "expected member function attribute, found '%1$s'"),
		      curchar);
         }
	 ++curchar;
      }

      u->symvalue.member.attrs.func.isInline = isInline;
      if (!u->symvalue.member.isStatic)
      {
         u->symvalue.member.attrs.func.isConst = isConst;
         u->symvalue.member.attrs.func.isVolatile = isVolatile;
      }
}

/* Gather attrs about data member */
private DataMemberAttrs(u)
Symbol u;
{
      int isVtblPtr = false, isVbasePtr = false, isVbaseSelfPtr = false;

      while (*curchar != ':' and *curchar != ';')
      {
 	 if (isdigit(*curchar))
	 {
            char *p = index(curchar, ':');
            if (p == nil) {
               panic(getmsg(MSG_354, "index(\"%s\", ':') failed"), curchar);
               return;
            }
	    curchar = p;
	    break;
	 }
	    
	 switch (*curchar) 
	 {		/* process member attrs */
	    case 's':
		u->symvalue.member.isStatic = true;
		break;
	    case 'p':
		isVtblPtr = true;
		break;
	    case 'r':
		isVbaseSelfPtr = true;
	    case 'b':
		isVbasePtr = true;
		break;
	    default:
	        panic(getmsg(MSG_634,
	         "expected member attribute 's', 'p', 'r' or 'b', found '%s'"),
		      curchar);
         }
	 ++curchar;
      }
      if (!u->symvalue.member.isStatic)
      {
         u->symvalue.member.attrs.data.isVtblPtr = isVtblPtr;
         u->symvalue.member.attrs.data.isVbasePtr = isVbasePtr;
         u->symvalue.member.attrs.data.isVbaseSelfPtr = isVbaseSelfPtr;
      }
}

/* process nested class */

Symbol *currMemList = nil;

private nestedType(t)
Symbol t;
{
   Symbol s, u;

   ++curchar;					/* skip over 'N'    */
   s = forward(t->type = constype(nil));
   assert(s != nil && (s->class == TAG || s->class == TYPE));
   t->name = s->name;

   /* set the scope of the nested object. */
   s->block = curblock;

   /* delete the nested type from the symbol table, should it have actually */
   /* been inserted. Certain times, nested types are not inserted.          */
   find (u, s->name) where s == u endfind(u);
   if (u != nil)
       delete(s);

   if (s->type->class == SCAL)
   {
      /* change the scope of enum constants as well */
      Symbol e = s->type->chain;
      String name;

      while (e != nil)
      {
         assert(e->class == CONST);

	 delete(e);
	 e->next_sym = *currMemList;
	 *currMemList = e;

	 e = e->chain;
      }
   }

   skipchar(curchar, ';');
   chkcont(curchar);
}

/* process friend class */
private friendClass(t)
Symbol t;
{
   ++curchar;					/* skip over '('    */
   t->type = constype(nil);
   skipchar(curchar, ';');
   chkcont(curchar);
}

/* process friend function */
private friendFunc(t)
Symbol t;
{
   char *cur, *p, *rest;
   Name name;
   DemangledName dName;
   
   ++curchar;					/* skip over ']'    */
   p = index(curchar, ':');
   if (p == nil) {
       panic(getmsg(MSG_354, "index(\"%s\", ':') failed"), curchar);
       return;
   }
   curchar = get_name(curchar, p);
   dName = Demangle(identname(curchar, true), &rest);
   curchar = p + 1;

   t->name = dName->name;
   t->symvalue.member.attrs.func.dName = dName;
   t->language = curlang;
   t->type = constype(nil);

   skipchar(curchar, ';');
   chkcont(curchar);
}

/* Construct the parameters of a member function.		    */
/* Currently, the compiler does not output this information, taking */
/* the empty grammar alternative.				    */
private void params()
{
}

/* Construct the chain of members to a class */
Symbol ClassMember (d)
integer d;
{
   Symbol u;
   register char *p;
   Name name;
   unsigned vattrs, access, isCompilerGenerated, isPure;
   boolean anonymous;
   int funcIndex = 0;

   AnonMember(&anonymous);
   if (*curchar == '(') 
   {
        u = newSymbol(nil, d, FRIENDCLASS, nil, nil);
        u->isAnonMember = anonymous;
        friendClass(u);
        return u;
   }
   else if (*curchar == ']') 
   {
        u = newSymbol(nil, d, FRIENDFUNC, nil, nil);
        u->isAnonMember = anonymous;
        friendFunc(u);
	return u;
   }

   CompilerGenerated(&isCompilerGenerated);
   VirtualSpec(&vattrs);
   AccessSpec(&access);
   AnonMember(&anonymous);
   
   if (*curchar == 'N')
   {
    	symbol_alloc(u);
    	u->level = d;
    	u->class = NESTEDCLASS;

        assert(vattrs == CPPREAL);
        assert(!isCompilerGenerated);

        u->isAnonMember = anonymous;
        u->symvalue.member.access = access;
        nestedType(u);
	return u;
   }

   u = newSymbol(nil, d, MEMBER, nil, nil);
   u->block = curblock;
   u->symvalue.member.access = access;
   u->symvalue.member.isCompGen = isCompilerGenerated;
   if (isdigit((int)*curchar))
   {
        /* get optional member function index */
	funcIndex = getint();
   }
   if (*curchar == '[')
   {
        ++curchar;

        /* We have a member function */
	u->symvalue.member.type = FUNCM;
        FuncType(u);
        MemberFuncAttrs(u);
	if (!u->symvalue.member.isStatic)
	    u->symvalue.member.attrs.func.funcIndex = funcIndex;
	u->symvalue.member.attrs.func.isVirtual = vattrs;
	u->isCppFunction = true;
   } 
   else
   {
        /* else, we have a data member */

        u->symvalue.member.type = DATAM;
        u->isAnonMember = anonymous;

        DataMemberAttrs(u);
   }
   skipchar(curchar, ':');
   p = index(curchar, ':');
   if (p == nil)
   {
        panic(getmsg(MSG_354, "index(\"%s\", ':') failed"), curchar);
        return;
   }
   curchar = get_name(curchar, p);
   name = identname(curchar, true);
   if (u->symvalue.member.type == DATAM) 
   {
       if (u->symvalue.member.isStatic)
       {
	   /* demangle the name, but do not set "name" to the base of the   */
	   /* demangled name as yet. This is done so that when the actual   */
	   /* variable symbol is later encountered in the symbol table, the */
	   /* mangled name of this symbol can be compared to the global     */
	   /* variable name. After the match is made, it will be changed.   */
           u->symvalue.member.attrs.staticData.dName = Demangle(name);
           u->symvalue.member.attrs.staticData.varSym = nil;

	   /* add the symbol to the list of static members yet to be tied */
	   /* to their variable entries.			          */
	   u->chain = staticMemberList;
	   staticMemberList = u;
       }
       u->name = name;
   }
   else
   {
       u->name = name;
       u->symvalue.member.attrs.func.funcSym = nil;

       /* add the symbol to the list of member functions yet to be tied to */
       /* their function entries.					   */
       InsertMemFunc(u);
   }
   curchar = p + 1;
   u->language = curlang;
   u->type = constype(nil);
   
   /* process regular data member field */
   if (u->symvalue.member.type == DATAM)
   {
       int offset, length;

       skipchar(curchar, ',');
       offset = getint();
       skipchar(curchar, ',');
       length = getint();
       skipchar(curchar, ';');
       chkcont(curchar);

       if (!u->symvalue.member.isStatic)
       {
	   u->symvalue.member.attrs.data.offset = offset;
	   u->symvalue.member.attrs.data.length = length;
       }
   } 
   else 
   {
       params();
       skipchar(curchar, ';');
       chkcont(curchar);
   }
   return u;
}

private NameResolution (t)
Symbol t;
{
  /* What info are we expecting, where to store them??? */
}

/*
 * Construct a C++ Class type.
 * Interpret stabstring and gather info about the class (base classes &
 * members)
 */

private consClass (t)
Symbol t;
{
  integer d;
  register Symbol u = t;
  register char *cur, *p;
  Name name;

  t->class = CLASS;
  t->symvalue.class.touched = false;
  t->symvalue.class.offset = getint();	/* NumByte - size of class */
  d = curblock->level + 1;
  
  chkcont(curchar);
  t->symvalue.class.key = *curchar;	/* Struct, Class, or Union?	*/
  curchar++;
  if (*curchar == 'V') 
  {					/* Pass-by-value class */
     t->symvalue.class.passedByValue = true;
     curchar++;			
  }
  else
     t->symvalue.class.passedByValue = false;
  if (*curchar != '(') 
  {					/* Process first class */
     BaseClass(u->type = newSymbol(nil, d, BASECLASS, nil, nil));
     u = u->type;			
     while (*curchar != '(')
     {					/* Process remaining classes	*/
	BaseClass(u->chain = newSymbol(nil, d, BASECLASS, nil, nil));
	u = u->chain;			
     }
  }

  u = t;
  curchar++;				/* skip over '(' */

  /* we set "currMemList" to the symbol name of the C++ class/   */
  /* struct/union so we can change the names of nested types in  */
  /* the class if and when they are encountered parsing the type */
  currMemList = &u->chain;

  /* Process member list */
  if (*curchar != ';' and *curchar != ')')
  {
     Symbol firstMember = ClassMember(d);
     if (u->chain != nil /* first member was an enum */)
     {
	 for (u = u->chain; u->next_sym != nil; u = u->next_sym);
	 u = u->next_sym = firstMember;
     }
     else
	 u = u->chain = firstMember;
     while (*curchar != ';' and *curchar != ')')
        u = u->next_sym = ClassMember(d);
  }

  currMemList = nil;

  if (*curchar == ')')
      NameResolution();			/* Process Name resolution list */
}


/*
 * Construct a variant record type.
 */

private consVarRecord (t)
Symbol t;
{
  register Symbol u;
  register char *cur, *p, *tagname, *q;
  Name name;
  boolean isField = false;
  Symclass class;
  integer d;
 
  t->class = VARNT;
  t->symvalue.offset = getint();
  d = curblock->level + 1;
  u = t;
 
  chkcont(curchar);
  cur = curchar;
  while (*cur != ';' and *cur != '\0')
  {
    class = FIELD;
    if (*cur == '[')            /* '[' means it is a Variant Tag */
    {
      cur = cur + 2;            /* take off the '[(' */
      class = VTAG;
    }
    else
    if ((*cur == '(') or isField)       /* '(' is a Variant Label */
    {
      if (!isField) ++cur;
      class = VLABEL;
      isField = false;
    }

    /* Process Variant labels: labels can either be constants (call    */
    /* constName) or range (call consSubrange)                         */
    if (class == VLABEL)
    {
      name = identname("$vlabel", true);
      u->chain = newSymbol(name, d, VLABEL, nil, nil);
      u = u->chain;
      if (*cur == 'r')                  /* label of range type */
      {
        curchar = curchar + 2;          /* take off '(r' */
        u->type = newSymbol(name, d, RANGE, nil, nil);
        consSubrange(u->type, RANGE);
      }
      else
      {                                 /* label of constant type */
        *curchar = '=';                 /* change '(' to '=' for constName */
        u->type = newSymbol(name, d, CONST, nil, nil);
        constName(u->type);
      }
      if (*curchar == ',')              /* Multiple Vlabel of the same */
        *curchar = '(';                 /* vfield are divided by ','   */
      else                              /* Change it to '(' and start  */
        skipchar(curchar, ':');         /* from the top again.         */
      cur = curchar;
    }
    else
    {                                   /* Process names of regular fields or */
       p = index(cur, ':');             /* tags                               */
       q = index(cur, ',');
       if ((q != nil)&&(q < p))
       {
          p = q;
          isField = true;
       }
       if (p == nil) {
          panic(getmsg(MSG_354, "index(\"%s\", ':') failed"), curchar);
	  return;
       }
       cur = get_name(cur, p);
       name = identname(cur, true);
       u->chain = newSymbol(name, d, class, nil, nil);
       u = u->chain;
       cur = p + 1;
    }

    /* Process type, offset and length of fields and tags */
    if ((*cur != ';') and ((isdigit((int)(*cur)))||(*cur == '-')))
    {
      u->language = curlang;
      curchar = cur;
      u->type = constype(nil);
      if (*curchar != ';')
      {
        skipchar(curchar, ',');
        u->symvalue.field.offset = getint();
        skipchar(curchar, ',');
        u->symvalue.field.length = getint();
      }
      skipchar(curchar, ';');
      chkcont(curchar);
      cur = curchar;
    }
    if ( (*cur == ';') or (*cur == ']') )
    {
      do {
        /* add empty label tag and end of Variant tag */
        u->chain = newSymbol(nil, d, TAG, nil, nil);
        u = u->chain;
        ++cur;
      } while (*cur == ']');
      curchar = cur;
    }
  }
  curchar = cur;
}



/*
 * Construct a record or union type.
 */

private consRecord (t, class)
     Symbol t;
     Symclass class;
{
  register Symbol u;
  register char *cur, *p, *tagname;
  Name name;
  integer d;

  t->class = class;
  t->symvalue.offset = getint();
  d = curblock->level + 1;
  u = t;
  
  chkcont(curchar);
  cur = curchar;
  while (*cur != ';' and *cur != '\0') 
  {
    p = index(cur, ':');
    if (p == nil) {
      panic(getmsg(MSG_354, "index(\"%s\", ':') failed"), curchar);
      return;
    }
    cur = get_name(cur, p);

    name = identname(cur, true);
    u->chain = newSymbol(name, d, FIELD, nil, nil);
    cur = p + 1;
    u = u->chain;
    u->language = curlang;
    curchar = cur;
    u->type = constype(nil);
    skipchar(curchar, ',');
    u->symvalue.field.offset = getint();
    skipchar(curchar, ',');
    u->symvalue.field.length = getint();
    skipchar(curchar, ';');
    chkcont(curchar);
    cur = curchar;
  }
  if (*cur == ';') 
  {
    ++cur;
  }
  curchar = cur;
}

/*
 * Construct a COBOL group type.
 */

private consGroup (t)
     Symbol t;
{
  register Symbol u;
  register char *cur, *p, *tagname;
  Name name;
  integer d;
  boolean getCondition = false;
  Symbol lastGroup;
  
  ++group;
  ++group_level;

  if (*curchar == 'r') {
    ++curchar;
    getRedefines(t);
    t->class = RGROUP;
  } else {
    t->class = GROUP;
  }
  if (*curchar == 'c') {
    getCondition = true;
  }
  ++curchar;

  t->symvalue.usage.bytesize = getint();
  d = curblock->level + 1;
  u = t;
  lastGroup = lastVar;
  
  chkcont(curchar);
  if (getCondition) {
    u->chain = consCond(u);
    skipchar(curchar, ',');
    while (u->chain)
      u = u->chain;
  }
  chkcont(curchar);
  cur = curchar;

  while (*cur != ';' and *cur != '\0') {
    p = index(cur, ':');
    if (p == nil) {
      panic(getmsg(MSG_354, "index(\"%s\", ':') failed"), curchar);
      return;
    }
    cur = get_name(cur, p);
    name = identname(cur, true);
    u->chain = insert (name);
    u->chain->language = primlang;
    u->chain->symvalue.field.group_id = group; 
    u->chain->symvalue.field.group_level = group_level; 
    u->chain->symvalue.field.parent = lastGroup;
    u->chain->storage = EXT;
    u->chain->level = d;
    u->chain->class = REFFIELD;
    u->chain->chain = nil;
    u->chain->block = curblock;
    cur = p + 1;
    u = u->chain;
    lastVar = u;
    u->language = curlang;
    curchar = cur;
    u->type = constype(nil);
    skipchar(curchar, ',');
    u->symvalue.field.offset = getint();
    skipchar(curchar, ',');
    u->symvalue.field.length = getint();
    skipchar(curchar, ';');
    chkcont(curchar);
    cur = curchar;
  }

  --group_level;
  if (*cur == ';') {
    ++cur;
  }
  curchar = cur;
}

/*
 * Construct an enumeration type.
 */

private consEnum (t)
Symbol t;
{
    register Symbol u;
    register char *p;
    register integer count;

    t->class = SCAL;
    if (isdigit(*curchar) || *curchar == '-')
    {
	t->type = constype(nil);
	skipchar(curchar, ':');
    }
    else
	t->type = typetable[TP_INT + TP_NTYPES];
    count = 0;
    u = t;
    while (*curchar != ';' and *curchar != '\0' and *curchar != ',') 
    {
	p = index(curchar, ':');
        if (p == nil) {
           panic(getmsg(MSG_354, "index(\"%s\", ':') failed"), curchar);
           return;
        }
	curchar = get_name(curchar, p);
	u->chain = insert(identname(curchar, true));
	curchar = p + 1;
	u = u->chain;
	u->language = curlang;
	u->class = CONST;
	u->level = curblock->level + 1;
	u->block = curblock;
	u->type = t;
	u->symvalue.constval = cons(O_LCON, (long) getint());
	u->symvalue.constval->nodetype = t_int;
	++count;
	skipchar(curchar, ',');
	chkcont(curchar);
    }
    if (*curchar == ';') {
	++curchar;
    }
    t->symvalue.iconval = count;
}

/*
 * Construct a parameter list for a function or procedure variable.
 */

private consParamlist (t, named)
Symbol t;
boolean named;
{
    Symbol p;
    integer i, d, n, paramclass;
    char *q;
    Name para;

    n = getint();
    skipchar(curchar, ';');
    p = t;
    d = curblock->level + 1;
    for (i = 0; i < n; i++) {
	if (named)
	{ 
  	  q = index(curchar, ':');
          if (q == nil) {
             panic(getmsg(MSG_354, "index(\"%s\", ':') failed"), curchar);
             return;
          }
	  curchar = get_name(curchar, q);
          para = identname(curchar, true);	   
          curchar = q + 1;
          p->chain = newSymbol(para, d, VAR, nil, nil);
	}
	else	
	  p->chain = newSymbol(nil, d, VAR, nil, nil);
	p = p->chain;
	p->type = constype(nil);
	skipchar(curchar, ',');
	paramclass = getint();
	if (paramclass == 0) {
	    p->class = REF;
	}
	skipchar(curchar, ';');
	chkcont(curchar);
    }
}

/*
 * Construct an imported type.
 * Add it to a list of symbols to get fixed up.
 */

private consImpType (t)
Symbol t;
{
    register char *p;

    p = curchar;
    while (*p != ',' and *p != ';' and *p != '\0') {
	++p;
    }
    if (*p == '\0') {
	panic(getmsg(MSG_363, "bad import symbol entry '%s'"), curchar);
	return;
    }
    t->class = TYPEREF;
    t->symvalue.typeref = curchar;
    if (*p == ',') {
	curchar = p + 1;
	constype(nil);
    } else {
	curchar = p;
    }
    skipchar(curchar, ';');
}

/*
 * Construct an opaque type entry.
 */

private consOpaqType (t)
Symbol t;
{
    register char *p;
    register Symbol s;
    register Name n;
    boolean def;

    p = curchar;
    while (*p != ';' and *p != ',') {
	if (*p == '\0') {
	    panic(getmsg(MSG_364, "bad opaque symbol entry '%s'"), curchar);
	    return;
	}
	++p;
    }
    def = (Boolean) (*p == ',');
    curchar = get_name(curchar, p);
    n = identname(curchar, true);
    find(s, n) where s->class == TYPEREF endfind(s);
    if (s == nil) {
	s = insert(n);
	s->class = TYPEREF;
	s->type = nil;
    }
    curchar = p + 1;
    if (def) {
	s->type = constype(nil);
	skipchar(curchar, ';');
    }
    t->class = TYPE;
    t->type = s;
}

/*
 * NAME: getlonglong
 *
 * FUNCTION: read an integer from the current position in the
 *           type string.
 *
 * PARAMETERS:
 *        none
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: none   
 *
 * RETURNS: a long long number
 *
 */

private LongLong getlonglong ()
{
    uLongLong n = 0;
    register char *p = curchar;
    register Boolean isneg;

    if (*p == '-') {
	isneg = true;
	++p;
    } else {
	isneg = false;
    }
    while (isdigit((int)(*p))) {
        n = 10 * n + (*p -'0');
        ++p;
    }

    curchar = p;
    return isneg ? (-n) : n;
}

/*
 * Read an integer from the current position in the type string.
 */

private integer getint ()
{
    register integer n;
    register char *p;
    register Boolean isneg;

    n = 0;
    p = curchar;
    if (*p == '-') {
	isneg = true;
	++p;
    } else {
	isneg = false;
    }
    while (isdigit((int)(*p))) {
	n = 10*n + (*p - '0');
	++p;
    }
    curchar = p;
    return isneg ? (-n) : n;
}

/*---------------------------------+
| Construct a cobol picture type.  |
+---------------------------------*/
private consPic (t)
Symbol t;
{
    char *p, c;
    int  desc_size, i, pic_size;

    t->chain = nil;
    t->type = t;
    c = *curchar;
    p = ++curchar;
    desc_size = 0;

    if (c == 'r') {
	getRedefines(t);
        c = *curchar;
        p = ++curchar;
        t->class = RPIC;
    } else {
        t->class = PIC;
    }

    if ((c < 'a') || (c > 't'))
       /*  error */
         panic(getmsg(MSG_528, "consPic: unknown cobol storage type '%c'"), *p);
    else {
        t->symvalue.usage.storetype = c;
        curchar = p;
        pic_size = getint();
        t->symvalue.usage.bytesize = pic_size;
        p = curchar; 
        if (!pic_size) {
            panic(getmsg(MSG_529,
		  "consPic: cobol picture definition had no internal size"));
	}

        /* increment p past bytesize part of stabstring */
        while ((*p != ',') && (*p != ';')) { 
            ++p;
	}
        if (*p == ';')  { /* end of typedef for pic */
            *p = ',';
            curchar = p;
            return;
        }
        else
            curchar = ++p; /* continue with the pic part of typedef */ 
         
        switch (c) {
	     case 'b':   /* the following 3 types must have an edit desc. */
             case 'd':
             case 'p':
                 if ((*p) != '\'') 
                   panic(getmsg(MSG_530, "consPic: expected edit description"));
                 else {
                     char ed_str [BUFSIZ];
                     p++; /* move to first character in edit description */
                     desc_size = 0;
                     while (((*p) != '\'') && (desc_size < BUFSIZ-1)) {  
                        ed_str[desc_size] = *p;
                        desc_size++;
                        p++; 
                     } 
                     ed_str [desc_size] = '\0';
                     ++p;
                     skipchar (p,',');
                     curchar = p;
                     pic_size = getint();
                     p = curchar;
                     if ((desc_size > BUFSIZ-2) || (desc_size != pic_size) ||
			 (desc_size == 0)) 
                          panic(getmsg(MSG_531,
				       "consPic: wrong size edit description"));
                     else 
                         if (t->symvalue.usage.edit_descript = 
                          (char *) malloc(pic_size + 1)) {
                          strcpy (t->symvalue.usage.edit_descript,ed_str); 
		         }
		         else
                             fatal(getmsg(MSG_532, "consPic: malloc error!"));
                 t->symvalue.usage.decimal_align = 0;
                 t->symvalue.usage.picsize = pic_size;
		 }
                 break;
	      default:
                  if (!isdigit((int)(*p)))
                      panic (getmsg(MSG_533,
				  "consPic: expected cobol decimal alignment"));
                  else {
                      t->symvalue.usage.edit_descript = nil;
                      curchar = p; 
                      t->symvalue.usage.decimal_align = getint();
                      skipchar (curchar,',');
                      t->symvalue.usage.picsize = getint();
                      p = curchar;
		  }
	} /* end switch */
        curchar = p;
	if (*curchar == ',') {
	  skipchar(curchar, ',');
	  chkcont(curchar);
	  t->chain = consCond(t); /* Cobol Condition clause */
	}
	skipchar(curchar, ';');
  }  /* end else there was a valid storage type */
	
 }  /* end procedure */

/*
 * consValue - Construct a COBOL Conditional Value List
 */
private Symbol consValue(primtype)
     int primtype;
{
  int len;
  Symbol t = nil, top = nil;
  char *p = nil;
  
  while (*curchar != ';' && *curchar != '\0') {
    if (t) {
      symbol_alloc(t->chain);
      t = t->chain;
    }
    else {
      symbol_alloc(t);
      top = t;			/* t is nil only the first time thru */
    }
    len = getint();
    skipchar(curchar, ':');
    string_alloc(p, len+1);
    if (p) {
      strncpy(p, curchar, len);
      p[len] = '\0';
    }
    t->symvalue.usage.storetype = (char)primtype; /* overload storetype */
    t->symvalue.usage.edit_descript = p; /* overload edit_descript */
    t->chain = nil;
    curchar += len;
    chkcont(curchar);
  }
  return t;
}

/*
 * Construct a COBOL Conditional type
 */
private Symbol consCond(parent)
     Symbol parent;
{
  Symbol t = nil, top = nil;
  char *p;
  int type_id,
  primtype,
  bytesize,
  sign,
  decimalsite,
  kanjichar;
  
  while (*curchar != ';' && *curchar != ',' && *curchar != '\0') {
    p = index(curchar, ':');
    if (p) {
      curchar = get_name(curchar, p);
      if (t) {
	t->chain = insert(identname(curchar, true));
	t = t->chain;
      }
      else {			/* first time thru only */
	t = top = insert(identname(curchar, true));
      }
      curchar = p + 1;
      t->chain = nil;
      t->symvalue.field.parent = parent;
      t->class = COND;
      t->language = curlang;
      type_id = getint();
      skipchar(curchar, '=');
      skipchar(curchar, 'q');
      primtype = *curchar++;
      if (primtype == 'n') {
	sign = *curchar++;
	decimalsite = getint();
      }
      skipchar(curchar, ',');
      kanjichar = getint();
      skipchar(curchar, ',');
      t->type = consValue(primtype);
      skipchar(curchar, ';');
      chkcont(curchar);
    }
  }
  return top;
}

/*-------------------------------+
| Construct a cobol index type.  |
+-------------------------------*/
private consIndex (t)
Symbol t;
{
    t->class = INDX;
    t->type = t;
    t->type->symvalue.usage.bytesize = getint();
    skipchar(curchar, ';');
    chkcont(curchar);
    t->type->symvalue.usage.picsize = getint();

  }

/*----------------------------------------+
| Construct a cobol usage is index type.  |
+----------------------------------------*/

private consUindex (t)
Symbol t;
{
    t->class = INDXU;
    t->type = t;
    skipchar(curchar, ';');
    chkcont(curchar);
}

/*
 * consFD(t) - Construct a COBOL File Descriptor type
 */
private consFD(t)
     Symbol t;
{
  t->class = FILET;
  string_alloc(t->symvalue.usage.edit_descript, 2);
  t->symvalue.usage.edit_descript[0] = *curchar++; /* Organization */
  t->symvalue.usage.edit_descript[1] = *curchar++; /* Access */
  t->symvalue.usage.bytesize = getint(); 	   /* Bytesize */
  skipchar(curchar, ';');
}

/*
 * Construct a type out of a string encoding.
 */

private Symbol constype (type)
Symbol type;
{
    register Symbol t, new_t;
    register Symbol typeentry;
    register integer n;
    char class;
    char *p;
    boolean packed = false; 
    boolean Named = true;
    integer trueSize = 0;
    Boolean allocatable = false;
    Boolean pointer = false;

    while (*curchar == '@') 
    {
        if (*(curchar + 1) == 'P') 
	    packed = true;
        else if (*(curchar + 1) == 's') 
			/* get true size for array of records in Pascal */
        { 
	    curchar = curchar + 2; 
            trueSize = getint(); 
	}
        else if (*(curchar + 1) == 'A') 
        { 
          allocatable = true;
        }
        else if (*(curchar + 1) == 'F') 
        { 
          pointer = true;
        }
	p = index(curchar, ';');
	if (p == nil) 
	{
	    fflush(stdout);
	    (*rpt_error)(stderr, getmsg(MSG_350,
					"missing ';' after type attributes"));
	    return;
	} else {
	    curchar = p + 1;
	}
    }
    if (isdigit((int)*curchar) || *curchar == '-') 
    {
	n = getint();
	if (n >= ntypes) {
        fatal( NLcatgets(scmc_catd, MS_object, MSG_346,
                "1283-230 too many types in file \"%s\""), curfilename());
	}
		/* This code necessary for fortran bug which produces
		   types of -99 */
	if (n == -99 && curlang == fLang)
	    n = -1;
	typeentry = typetable[n+TP_NTYPES];
	if (*curchar == '=') {
	    if (typeentry != nil) {
		t = typeentry;
	    } else {
		symbol_alloc(t);
	        entertype(n+TP_NTYPES, t);
	    }
	    ++curchar;
	    constype(t);
	} else {
	    t = typeentry;
	    if (t == nil) {
		symbol_alloc(t);
	        entertype(n+TP_NTYPES, t);
	    }
	    /* fortran padded types when AUTODBL */
	    if (trueSize && (curlang == fLang)) {
                /* create new symbol with new size info */
                Symbol ct = newSymbol(t->name, t->level, t->class, t->type,
                                      t->chain);
                ct->storage = t->storage;
                ct->language = curlang;
                t = ct;
		/* storage the unpadded size and put in the padded one */
    		t->symvalue.field.length = size(t);
    		t->symvalue.size = trueSize;
	    }
	}
        if (pointer)
        {
          symbol_alloc(new_t)
          memcpy (new_t, t, sizeof(struct Symbol));
          new_t->ispointer = true;
          t = new_t;
        }
    } else {
	if (type == nil) {
	    symbol_alloc(t);
	} else {
	    t = type;
	}
	t->language = curlang;
	t->level = curblock->level + 1;
	t->block = curblock;
        t->ispointer = pointer;
	class = *curchar++;
	switch (class) {
	    case T_SUBRANGE:
		if (packed)
                   consSubrange(t, PACKRANGE);
		else
		   consSubrange(t, RANGE);
		break;
            case T_SPACE:
                t->class = SPACE;
                t->type = constype(nil);
                skipchar(curchar, ';');
                chkcont(curchar);
                t->chain = newSymbol(nil, curblock->level+1, RANGE, t_int, nil);
                t->symvalue.size = getint();
                break;
	    case T_ARRAY:
		t->class = ARRAY;
		t->chain = constype(nil);
		skipchar(curchar, ';');
		chkcont(curchar);
                /* Fix the true size of array of record (pascal) */
                if (trueSize != 0 and isdigit((int)(*curchar)))
                  t->type = cloneSym(typetable[getint()+TP_NTYPES],trueSize);
		else
                  t->type = constype(nil);
		break;
	    case T_PACKEDARRAY:
                t->class = PACKARRAY;
                t->chain = constype(nil);
                skipchar(curchar, ';');
                chkcont(curchar);
		t->type = constype(nil);
                break;
	    case T_DEFERSHAPEARRAY:
                if (allocatable)
                  t->isallocatable = true;

                t->class = ARRAY;
                t->type = constype(NULL);
                t->chain = newSymbol(nil, curblock->level+1,
                                     RANGE, t_int, nil);
                break;

	    case T_ASSUMESHAPEARRAY:
                t->class = ARRAY;
                /*  read descriptor offset  */
                t->symvalue.offset = getint();
                skipchar(curchar, ',');
                t->type = constype(NULL);
                t->isassumed = true;
                t->chain = newSymbol(nil, curblock->level+1,
                                     RANGE, t_int, nil);
                t->storage = STK;
		break;

	    case T_SUBARRAY:
		t->class = SUBARRAY;
		t->symvalue.ndims = getint();
		skipchar(curchar, ',');
		t->type = constype(nil);
		t->chain = t_int;
		break;

	    case T_VARNT:
		consVarRecord(t);
		break;

	    case T_RECORD:
                if (packed) 
                   consRecord(t, PACKRECORD);
		else
		   consRecord(t, RECORD);
		break;

	    case T_GROUP:	
		consGroup(t);
		break;

	    case T_UNION_:
		consRecord(t, UNION);
		break;

	    case T_ENUM_:
		consEnum(t);
		break;

	    case T_PTR:
	    /* case T_REF: -- use when implementing refs. as pointers. */
		t->class = PTR;
		t->type = constype(nil);
		break;

	    /*
	     * C function variables are different from Modula-2's.
	     */
	    case T_FUNCVAR:
		t->class = FFUNC;
		t->type = constype(nil);
		break;

	    case T_PROCVAR:
		t->class = FPROC;
		consParamlist(t,!Named);
		break;

	    /* Pascal procedure/function parameter */
	    case T_FUNCPARAM:
	 	t->class = FUNCPARAM;
		t->type = constype(nil);
		skipchar(curchar, ',');
		consParamlist(t,Named);
		break;

	    case T_PROCPARAM:
		t->class = PROCPARAM;
		consParamlist(t,Named);
		break;

	    case T_IMPORTED:
		consImpType(t);
		break;

	    case T_SET:
		if (packed)
		  t->class = PACKSET;
		else
		  t->class = SET;
		t->type = constype(nil);
                if (trueSize != 0)
                  t->symvalue.size = trueSize;
		break;

	    case T_OPAQUE:
		consOpaqType(t);
		break;

	    case T_FILE:
		t->class = FILET;
		t->type = constype(nil);
		break;

	    case T_REAL:
		consReal(t);
	        if (trueSize && (curlang == fLang)) {
		    /* storage the unpadded size and put in the padded one */
    		    t->symvalue.field.length = size(t);
    		    t->symvalue.size = trueSize;
	        }
		break;

            case T_WIDECHAR:
                consWide(t);
                break;

	    case T_COMPLEX:
		consComplex(t);
	        if (trueSize && (curlang == fLang)) {
		    /* storage the unpadded size and put in the padded one */
    		    t->symvalue.field.length = size(t);
    		    t->symvalue.size = trueSize;
	        }
		break;

	    case T_STRINGPTR:
		consStringptr(t);
		break;
 	    case T_PIC:
 		consPic(t);
 		break;
 
 	    case T_INDEX:
 		consIndex(t);
 		break;
 
 	    case T_UINDEX:
 		consUindex(t);
 		break;
 
            case T_STRING:
                consString(t, STRING);
                break;

            case T_GSTRING:
                consString(t, GSTRING);
                break;

            case T_MULTI:
                consMulti(t);
                break;

	    case T_FILEDESCR:
		consFD(t);
		break;

	    /* C++ stabstring information */
	    case T_CLASS:
		consClass(t, CLASS);
		break;

	    case T_CONST:
	    {
		Symbol type = constype(nil);
		*t = *type;
		t->isConst = true;
		break;
	    }
		
	    case T_VOL:
	    {
		Symbol type = constype(nil);
		*t = *type;
		t->isVolatile = true;
		break;
	    }
		
	    case T_REF:
		t->class = CPPREF;
		t->type = constype(nil);
		break;
		
	    case T_PTRTOMEM:
		t->class = PTRTOMEM;
		t->symvalue.ptrtomem.hasVBases = OptVBaseSpec();
		t->symvalue.ptrtomem.hasMultiBases = OptMultiBaseSpec();
		t->type = constype(nil);
		skipchar(curchar, ':');
		t->symvalue.ptrtomem.memType = constype(nil);
		skipchar(curchar, ':');
		t->symvalue.ptrtomem.ptrType = constype(nil);
		skipchar(curchar, ';');
		break;
		
	    case T_ELLIPSES:
		t->class = ELLIPSES;
		curchar += 1;
		break;
		
	    default:
		badcaseval(class);
	}
    }
    return t;
}

/*
 * NAME: convert_f90_sym
 *
 * FUNCTION: take a symbol for a fortran array or pointer, and
 *             create a symbol that dbx understands.
 *
 * PARAMETERS: fortran_sym - the symbol for the allocatable array.
 *             is_active - tell the caller if variable is active.
 *                         if is_active is NULL, call error if 
 *                         variable is not active.
 *
 * NOTES: 
 *
 *    assumed-shape arrays
 *    ____________________
 *
 *   Subroutine Buffle( A )        :t<n>=O<S>,-29
 *         integer(4) A(:)
 *    where <S> is the offset into the stack where the address of the
 *      array descriptor can be found.  The descriptor does not contain
 *      the address of the array.
 *
 *    deferred-shape arrays
 *    _____________________
 *
 *   integer(4),allocatable::A(:)  :t<n>=@A;A-29
 *   integer(4),pointer::A(:)      :t<n>=@F;A-29
 *
 *    The object represents a descriptor which describes the array.  
 *    The descriptor does contain the address of the array.
 *
 *    Array descriptors
 *    _________________
 *
 *   byte   bits    field name and meaning
 *   ----   -----   ----------------------
 *     2     0-0    is_allocated:
 *                    This is meaningful for objects whose type has the
 *                    allocatable stabstring attribute (@A).  If
 *                    is_allocated=1, the object is currently allocated.
 *                    Otherwise, the object is not allocated.
 *     4     0-31   length
 *                    the number of bytes in each array element.
 *     8     0-31   rank
 *                    this is the number of diminsions of the array.
 *    16     0-31   L(rank): The lower bound for the rank'th dimension.
 *    20     0-31   E(rank): The extent for the rank'th dimension.
 *    24     0-31   M(rank): The multiplier for the rank'th dimension.
 *  ... There are a total of "rank" triples of L, E, and M fields.
 *    ??     0-31   L(1): The lower bound for the 1st dimension.
 *    ??     0-31   E(1): The extent for the 1st dimension.
 *    ??     0-31   M(1): The multiplier for the 1st dimension.
 *
 *    Pointer descriptors
 *    ___________________
 *
 *   byte   bits    field name and meaning
 *   ----   -----   ----------------------
 *     0     0-31   address:
 *                    This is the address of the "real" object.
 *     6     1-1    is_associated:
 *                    This is meaningful for objects whose type has the F90
 *                    pointer stabstring attribute(@F).  If 
 *                    is_associated=1, the object is currently associated.
 *                    Otherwise, the object is not associated.
 *
 *
 * 
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: none   
 *
 * RETURNS: a new symbol or the original symbol unchanged
 *
 */

#define ARRAY_SIZE 26
#define MAXDIM 20

Symbol convert_f90_sym(Symbol fortran_sym, Boolean *is_active)
{
   Address addr;
   int real_loc, i;
   struct array_descriptor descriptor;
   struct array_bounds *bounds;
   char *save_curchar = curchar;
   Symbol real_sym, temp_sym;
   char *stabstring;
   char number[11];
   int ndim = 0;

   /*  if this is not a symbol that needs to be converted  */
   if (!fortran_sym->type->isallocatable
    && !fortran_sym->type->ispointer
    && !fortran_sym->type->isassumed)
     /*  return it unchanged  */
     return fortran_sym;

   if (keepdescriptors)
     return fortran_sym;

   if (!isactive(container(fortran_sym)))
   {
     if (is_active)
       *is_active = false;
     else
       error( catgets(scmc_catd, MS_eval, MSG_82,
             "\"%s\" is not active"), symname(fortran_sym));

     return fortran_sym;
   }

   /*  if this is a fortran allocatable array or a pointer  */
   if (fortran_sym->type->isallocatable 
    || fortran_sym->type->ispointer)
   {
     addr = fortran_sym->symvalue.offset;
     dread (&real_loc, addr, sizeof(int));
     addr += sizeof (int);
   }
   else if (fortran_sym->type->isassumed)
   {
     dread (&addr, fortran_sym->type->symvalue.offset + reg(1), 
            sizeof(int));
     real_loc = fortran_sym->symvalue.offset;
   }
    
   /*  read the descriptor - for non-array pointers, just 3 bytes  */
   dread (&descriptor, addr, (fortran_sym->type->class == ARRAY)
                               ? sizeof(descriptor) : 3);
   addr += sizeof (descriptor);

   /*  if not allocated or associated  */
   if ((real_loc == NULL)
    || (fortran_sym->type->isallocatable
      && ((descriptor.byte[2] & 0x80) == NULL))
    || (fortran_sym->type->ispointer
      && ((descriptor.byte[2] & 0x40) == NULL)))
   {
     if (is_active)
       *is_active = false;
     else
       error( catgets(scmc_catd, MS_eval, MSG_82,
             "\"%s\" is not active"), symname(fortran_sym));
     return fortran_sym;
   }

   symbol_alloc(real_sym);
   memcpy (real_sym, fortran_sym, sizeof(struct Symbol));

   if (fortran_sym->type->class == ARRAY)
   {
     ndim = descriptor.rank;

     bounds = malloc (sizeof (struct array_bounds) * ndim);
     dread (bounds, addr, (sizeof (struct array_bounds) * ndim));

     /*  create a "dummy" stabstring to create new type  */
     stabstring = malloc (ARRAY_SIZE * ndim + 1);

     stabstring[0] = '\0';
     curchar = stabstring;

     for (i = 0; i < ndim; i++)
     {
       strcat (stabstring, "ar0;");
       sprintf (number, "%d", bounds[ndim - i -1].lower_bound);
       strcat (stabstring, number);
       strcat (stabstring, ";");
       sprintf (number, "%d",
                bounds[ndim - i -1].lower_bound
              + bounds[ndim - i -1].extent - 1);
       strcat (stabstring, number);
       strcat (stabstring, ";");
     }

     /*  Put a type at the end of the "stabstring" to avoid
           getting an internal error.  We will reset the type
           to the "real" type later  */
     strcat (stabstring, "-29");

     real_sym->type = constype(nil);

     for (temp_sym = real_sym, i = 0; i < ndim; i++)
     {
       temp_sym = temp_sym->type;
     }

     temp_sym->type = fortran_sym->type->type;

     curchar = save_curchar;
   }
   else
   {
     /*  non-array pointer  */
     symbol_alloc(temp_sym);
     memcpy (temp_sym, fortran_sym->type, sizeof(struct Symbol));
     real_sym->type = temp_sym;
     temp_sym->ispointer = false;
   }

   real_sym->symvalue.offset = real_loc;

   return (real_sym);
}

/*
 * read in the redefines name and trailing comma
 */
getRedefines(t)
Symbol t;
{
    char *s = curchar;

    while(isdigit((int)(*s)) || isalpha((int)(*s))
	  || ((*s) == '-'))
	++s;
    if (*s != ',') {
	panic(getmsg(MSG_270, "expected char '%c', found '%s'"), ',', s);
	return;
    }
    t->symvalue.usage.redefines = get_name(curchar, s);
    curchar = s + 1;
}

/*
 * Reinitialize variables
 */
void stab_init()
{
	group = 0;
	group_level = 1;
	lastlanguage = nil;
	staticMemberList = nil;
}
