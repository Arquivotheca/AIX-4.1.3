/* @(#)85	1.26.3.10  src/bos/usr/ccs/lib/libdbx/symbols.h, libdbx, bos411, 9433B411a 8/17/94 08:58:09 */
#ifndef _h_symbols
#define _h_symbols
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) codeloc, find, isblock, isinline, isroutine, nosource,
 * 	      prolloc, symbol_alloc
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
typedef struct Symbol *Symbol;

#include "machine.h"
#include "names.h"
#include "languages.h"
#include "tree.h"

/*
 * Symbol classes
 */

typedef enum {
    BADUSE, CONST, TYPE, VAR, ARRAY, OPENARRAY, DYNARRAY, SUBARRAY, PACKARRAY,
    PTRFILE, RECORD, FIELD, REFFIELD, STRINGPTR, STRING, COMPLEX, REAL,
    WIDECHAR, PROC, FUNC, CSECTFUNC, FVAR, REF, PTR, FILET, SET, RANGE, 
    LABEL, WITHPTR, SCAL, STR, PROG, IMPROPER, VARNT,
    FPROC, FFUNC, MODULE, TAG, COMMON, EXTREF, TYPEREF, CHARSPLAT, FSTRING,
    PIC, RPIC, INDX, INDXU, GROUP, RGROUP, COND, PACKRECORD, PACKSET, PACKRANGE,
    PROCPARAM, FUNCPARAM, UNION, VTAG, VLABEL, SPACE, GSTRING, TOCVAR,
    FPTR, FPTEE, MEMBER, CLASS, BASECLASS, NESTEDCLASS, CPPREF, PTRTOMEM, 
    ELLIPSES, FRIENDFUNC, FRIENDCLASS, CPPSYMLIST, GENERIC
} Symclass;

#define LASTCLASS GENERIC

typedef enum { R_CONST, R_STATIC, R_TEMP, R_ARG, R_ADJUST, R_REGARG, R_REGTMP }
 Rangetype;

#define NOTSTORED 0
#define INREG 1
#define STK 2
#define EXT 3

/* C++ access spec */
#define PRIVATE 1
#define PUBLIC 2
#define PROTECTED 3

/* C++ member types */
#define DATAM 0
#define FUNCM 1
 
/* Function types */
#define CPPFUNC 0
#define CPPCTOR 1
#define CPPDTOR 2

/* Function attributes */
#define CPPREAL 0
#define CPPVIRTUAL 1
#define CPPPUREVIRTUAL 2

typedef struct desclist {
    Address descaddr;
    struct desclist *next_desc;
} Desclist;

typedef struct TemplateClassListEntry {
    Symbol templateClass;
    struct TemplateClassListEntry *next;
} *TemplateClassListEntry;

/* Several of the bit fields are actually types other than unsigned, but
   they are changed here to get rid of pestering warnings. */

struct Symbol {
    Name name;
    Language language;
    /* Symclass */ unsigned class : 8;
    /* boolean */ unsigned param : 1;          /* Is VAR is a param? */
    unsigned storage : 2;                      /* INREG, STK, or EXT */
    unsigned level : 5;                        /* Stack variables only */
    /* boolean */ unsigned isVolatile : 1;     /* C++ volatile type specifier */
    /* boolean */ unsigned isConst : 1;        /* C++ const type specifier */
    /* boolean */ unsigned isStaticMember : 1; /* C++ static data member */
    /* boolean */ unsigned isAnonMember : 1;   /* C++ promoted anon. member */
    /* boolean */ unsigned isCppFunction : 1;  /* C++ function */
    /* boolean */ unsigned isMemberFunc : 1;   /* FUNC that is C++ member */
    /* boolean */ unsigned isClassTemplate: 1; /* C++ class template */
    /* boolean */ unsigned ispredefined: 1;    /* predefined type */
    /* boolean */ unsigned WasUnnamedType: 1;  /* Was it an unnamed type */
    /* boolean */ unsigned isallocatable: 1;   /* F90 allocatable array? */
    /* boolean */ unsigned ispointer: 1;       /* F90 pointer array? */
    /* boolean */ unsigned isassumed: 1;       /* F90 assumed shape array? */
    unsigned unused : 4;		       /* reserved for expansion */
    Symbol type;
    Symbol chain;
    union {
	Node constval;		/* value of constant symbol */
	int offset;		/* variable address */
	long iconval;		/* integer constant value */
	int ndims;		/* no. of dimensions for dynamic/sub-arrays */
	unsigned int size;	/* number of bytes in a real or complex type */
	struct {		/* field offset and size (both in bits) */
	    int offset;
	    int length;
			        /* for cobol */
	    Symbol parent;	/* Need to look at parent sometimes */
	    short group_id;	/* Group members of a symbol within a group */
	    short group_level;	/* The level of the symbol within a group */
	} field;
	struct { 	 /* C++ class type */
	    int offset;		       /* size of class structure */
	    char key; 		       /* type of class (class/struct/union) */
	    unsigned passedByValue: 1; /* this class always passed by value */
	    unsigned touched: 1;       /* passed over to correct unused mems */
	} class;
	struct { 	 /* C++ class template type */
	    TemplateClassListEntry list;
	} template;
        struct {	 /* C++ base class type */
	    int offset;		    /* base class offset */
            unsigned isVirtual : 1; /* virtual base class? */
	    unsigned access : 2;    /* PRIVATE, PUBLIC, or PROTECTED */
	} baseclass;	    
	struct {  	 /* C++ member type */
	    union {
		struct {
	           int offset;
	           int length;
		   unsigned isVtblPtr : 1;
		   unsigned isVbasePtr : 1;
		   unsigned isVbaseSelfPtr : 1;
		} data;
		struct {
		   Symbol varSym;
		   DemangledName dName;
		} staticData;
		struct {
	    	   Symbol funcSym;
		   int funcIndex;
                   /* struct Location *sourceLines; -removed */
		   unsigned isInline : 1;
		   unsigned isConst : 1;
		   unsigned isVolatile : 1;
            	   unsigned isVirtual : 2; /* REAL, VIRTUAL, PUREVIRTUAL */
		   unsigned kind : 2; /* CPPCTOR, CPPDTOR, CPPFUNC */
		   unsigned isSkeleton: 1;
		   DemangledName dName;
		} func;
	    } attrs;
	    unsigned isStatic : 1;    /* Static member? */  
	    unsigned isCompGen : 1;   /* Compiler generated? */  
	    unsigned type : 1;        /* DATAM, FUNCM */
	    unsigned access : 2;      /* PRIVATE, PUBLIC, or PROTECTED */
	} member;

	struct USAGE {	 /* Usage for Cobol Pic items */
	  char  storetype;	/* Type of Pic item */
	  short decimal_align;	/* Location of decimal if item is numeric */
	  unsigned short
		bytesize;	/* Size of internal storage for item */
	  unsigned short	/* Display Size of item or */
		picsize;	/*   element size of index */
	  char  *edit_descript;	/* Edit string for item if one exists */
	  char  *redefines;	/* Name of redefines symbols */
	} usage;
	struct {		/* common offset and chain; used to relocate */
	    int offset;         /* vars in global BSS */
	    Symbol chain;
	    Address com_addr;	/* Origin of common area */
	} common;
	struct {		/* range bounds */
	    /* lowertype and uppertype are actually of type Rangetype */
            unsigned lowertype : 16; 
            unsigned uppertype : 16;  
            unsigned is_unsigned  : 1;
            long size;
	    long lower;
	    long upper;
	} rangev;
	struct {
	    Address beginaddr;	    /* address of function code */
	    Address proladdr;	    /* address of prolog to function */
            Desclist *fcn_desc;     /* list of assoc. function descriptors */
	    /* offset is an int; src, inline, and intern are Boolean */
	    unsigned offset : 16;   /* offset of function value */
	    unsigned src : 1;	    /* true if there is source line info */
	    unsigned inline : 1;    /* true if no separate act. rec. */
	    unsigned intern : 1;    /* internal calling sequence */
	    unsigned isentry : 1;   /* is a FORTRAN ENTRY point */
	    unsigned islinkage : 1; /* is linkage to a routine */
	    unsigned untouched : 1; /* unread lazy read file */
	    unsigned dontskip : 1;  /* special function that dostep should */
				    /* not skip over			   */
	    unsigned unused : 9;

	    /* C++: backpointer to member function Symbol */
	    /*      or demangled name of non-member func. */
	    union {
	        Symbol memFuncSym;
	        DemangledName dName;
	    } u;
	} funcv;
	struct {		/* variant record info */
	    int size;
	    Symbol vtorec;
	    Symbol vtag;
	} varnt;
	struct {		/* variable length multiples (FORTRAN char*) */
	    int size;
	    Rangetype sizeloc;
	} multi;
	String typeref;		/* type defined by "<module>:<type>" */
	Symbol extref;		/* indirect symbol for external reference */
	struct {
	    Symbol ptrType;   	/* specific type of C++ ptr. to mem. type */
	    Symbol memType;   	/* type of member of C++ ptr. to mem. type */
	    unsigned hasMultiBases : 1;
	    unsigned hasVBases : 1;
	} ptrtomem;
	struct cppSymList *sList; /* List of C++ "overloaded" symbols */
    } symvalue;
    Symbol block;		/* symbol containing this symbol */
    Symbol next_sym;		/* hash chain */
};

typedef struct sym_list *SLIST;		/* list of symbols */
struct sym_list {
	SLIST s_next;
	Symbol s_sym;
};

struct array_descriptor 
{
  unsigned char byte[4];
  int           len;
  int           rank;
  int           unknown;
};

struct array_bounds
{
  int lower_bound;
  int extent;
  int multiplier;
};

/*  macro to determine if a symbol is a fortran 90 symbol  */
#define is_f90_sym(sym)         \
  ((sym)->type                  \
&& ((sym)->type->isallocatable  \
 || (sym)->type->isassumed      \
 || (sym)->type->ispointer))

/*
 * Basic types.
 */


#define codeloc(f) ((f)->symvalue.funcv.beginaddr)
#define prolloc(f) ((f)->symvalue.funcv.proladdr)
#define isblock(s) (Boolean) ( \
    s->class == FUNC or s->class == PROC or \
    s->class == MODULE or s->class == PROG or \
    s->class == CSECTFUNC \
)
#define isroutine(s) (Boolean) ( \
    s->class == FUNC or s->class == PROC or s->class == CSECTFUNC or \
    s->class == FFUNC \
)

#define nosource(f) (not (f)->symvalue.funcv.src)
#define isinline(f) ((f)->symvalue.funcv.inline)

/*
 * Some macros to make finding a symbol with certain attributes.
 */

#define find(s, withname) \
{ \
    s = lookup(withname); \
    while (s != nil and not (s->name == (withname) and

#define where /* qualification */

#define endfind(s) )) { \
	s = s->next_sym; \
    } \
}

/*
 * Allocate a new symbol.
 */

#define SYMBLOCKSIZE 1000

typedef struct Sympool {
    struct Symbol sym[SYMBLOCKSIZE];
    struct Sympool *prevpool;
} *Sympool;

#define symbol_alloc(symbol)                	\
{ 						\
    register Sympool newpool; 			\
						\
    if (nleft <= 0) { 				\
	newpool = new(Sympool); 		\
	bset0(newpool, sizeof(*newpool));	\
	newpool->prevpool = sympool;		\
	sympool = newpool;			\
	nleft = SYMBLOCKSIZE;			\
    }						\
    --nleft;					\
    symbol =  &(sympool->sym[nleft]);		\
}

/* Constants for FORTRAN adjustable arrays */
#define MINUS1 0xffff
#define MINUS2 0xfffe

/* 
 * Builtin types 
 */
extern Symbol t_boolean;
extern Symbol t_char;
extern Symbol t_int;
extern Symbol t_longlong;
extern Symbol t_ulonglong;
extern Symbol t_real;
extern Symbol t_quad;
extern Symbol t_complex;
extern Symbol t_qcomplex;
extern Symbol t_nil;
extern Symbol t_addr;
extern Symbol t_gchar;

/* 
 * Pre-defined types 
 */
extern Symbol dt_null;
extern Symbol dt_arg;
extern Symbol dt_char;
extern Symbol dt_short;
extern Symbol dt_int;
extern Symbol dt_long;
extern Symbol dt_float;
extern Symbol dt_double;
extern Symbol dt_struct;
extern Symbol dt_union;
extern Symbol dt_enum;
extern Symbol dt_moe;
extern Symbol dt_uchar;
extern Symbol dt_ushort;
extern Symbol dt_uint;
extern Symbol dt_ulong;
extern Symbol dt_uchar2;
extern Symbol dt_ushort2;
extern Symbol dt_uint2;
extern Symbol dt_ulong2;
extern Symbol dt_NLchar;
extern Symbol dt_bool;

/*
 * Globals
 */
extern Symbol program;
extern Symbol curfunc;
extern boolean showaggrs;

/* which lookup criteria */
#define WFUNC 		0x00000200    /* looking for a function */

#define WCLASS 		0x00000400    /* looking for a class */
#define WSTRUCT 	0x00000800    /* looking for a struct */
#define WUNION 		0x00001000    /* looking for a union */
#define WTYPEDEF 	0x00002000    /* looking for a typedef */
#define WENUM 		0x00004000    /* looking for an enum */
#define WTYPE 		0x00007C00    /* looking for any type */
#define WOTHER 		0x00008000    /* looking for an object */
#define WANY 		0x0000FC00    /* looking for anything */
#define WSEARCH 	0x0000FC00    /* mask to extract search criteria */

#define WQUAL 		0x00010000    /* the name has been :: qualified */
#define WNIL		0x00020000    /* which() can return nil */

#define WMULTI 		0x00000080    /* multiple overloads selection */
#define WTMPLALL  	0x00000040    /* all templates selected */
#define WALL   		0x00000020    /* all overloads selected */
#define WPARAMCOUNT     0x00000010    /* parameters available */
#define WPARAMSLOT      0x0000000F    /* index in params array (don't move) */

/* 
 * Prototypes
 */
#define symname(s) \
    (((s) && (s)->isCppFunction) ? cppFunc_symname(s) : ident((s)->name))
extern String cppFunc_symname(/* Symbol */);
extern symbol_dump (/* func */);
extern symbol_free (/*  */);
extern Symbol newSymbol (/* name, blevel, class, type, chain */);
extern Symbol insert (/* name */);
extern Symbol lookup(/* name */);
extern delete (/* s */);
extern dumpvars (/* f, frame */);
extern Symbol dpi_which (/*  */);
extern symbols_init (/*  */);
extern Symbol rtype (/* type */);
extern resolveRef (/* t */);
extern integer regnum (/* s */);
extern Symbol container (/* s */);
extern Node constval (/* s */);
extern Address address (/* s, frame */);
extern boolean multiword (/* f */);
extern defregname (/* n, r */);
extern findtype (/* s */);
extern findbounds (/* u, lower, upper */);
extern integer size(/* sym */);
extern integer psize (/* s */);
extern Boolean isparam(/* s */);
extern boolean isopenarray (/* type */);
extern Boolean isvarparam(/* s */);
extern Boolean isvariable(/* s */);
extern Boolean isconst(/* s */);
extern Boolean ismodule(/* s */);
extern markInternal (/* s */);
extern boolean isinternal (/* s */);
extern markEntry (/* s */);
extern boolean isentry (/* s */);
extern Boolean isbitfield(/* s */);
extern Boolean compatible (/* t1, t2 */);
extern Boolean istypename (/* type, name */);
extern Boolean ischartype (/* type */);
extern boolean passaddr (/* p, exprtype */);
extern Boolean isambiguous (/* s */);
extern assigntypes (/* p */);
extern Node dot (/* record, fieldname */);
extern Node lvalRecord (/* record */);
extern Node subscript (/* a, slist */);
extern int evalindex (/* s, base, i */);
extern chkboolean (/* p */);
extern unmkstring (/* s */);
extern Node which (/* n, criteria */);
extern Boolean getbound (/* s, off, type, valp */);
extern Node buildSet (/* slist */);
extern boolean isintegral();
extern boolean ischar();
extern boolean meets(/* type Symbol, criteria */);
extern Name typename(/* type Symbol */);
extern Symbol forward(/* Symbol */);

void MemFuncTabInit();
void InsertMemFunc(/* Symbol */);
Symbol RetrieveMemFunc(/* Symbol */);
long getrangesize (/* lower, upper */);
Node buildaref (Node, Node);

#endif /* _h_symbols */
