/* @(#)51       1.13.2.3  src/bos/usr/ccs/bin/common/mfile1.h, cmdprog, bos411, 9428A410j 4/14/94 13:17:02 */
/*
 * COMPONENT_NAME: (CMDPROG) mfile1.h
 *
 * FUNCTIONS: BTYPE, CHARCAST, DECREF, DEUNSIGN, ENUNSIGN, FIXARG, FIXDEF    
 *            FIXSTRUCT, FOLD_GENERAL, FOLD_INTEGRAL, HASCONST, HASVOLATILE   
 *            INCREF, ISAGGREGATE, ISARITHM, ISARY, ISBTYPE, ISCHAR, ISCONST  
 *            ISCONSTANT, ISFLOAT, ISFTN, ISINTEGRAL, ISNUMBER, ISPTR         
 *            ISQUALIFIED, ISSCALAR, ISTSIGNED, ISTUNSIGNED, ISUNION          
 *            ISUNSIGNED, ISVOLATILE, MODTYPE, NO_FOLD, QUALIFIERS, SETDCON   
 *            TOOLSTR, TOPQTYPE, TOPTYPE, UNSIGNABLE, checkst, tyalloc        
 *
 * ORIGINS: 27 3 9 32 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */
/*
 * @OSF_COPYRIGHT@
 */
#ifndef HOSTIEEE
# include <sys/fpfp.h>
#endif HOSTIEEE
# define _ILS_MACROS     /* 139729 - use macros for better performance */
# include "macdefs.h"
# include "manifest.h"
# include <nl_types.h>
# include <sys/localedef.h>
# include <ctype.h>

#include "ctools_msg.h"
#define         TOOLSTR(Num, Str) catgets(catd, MS_CTOOLS, Num, Str)
nl_catd catd;

#if     !defined (CXREF) && !defined (LINT) && !defined (CFLOW)
# define IS_COMPILER    1
#endif /* !CXREF && !LINT && !CFLOW */

/*      storage classes  */
# define SNULL          0
# define AUTO           1
# define AUTOREG        2
# define PARAM          3
# define PARAMREG       4
# define PARAMFAKE      5
# define REGISTER       6
# define LABEL          7
# define ULABEL         8
# define STATIC         9
# define USTATIC        10
# define FORTRAN        11
# define UFORTRAN       12
# define EXTDEF         13
# define EXTENT         14
# define EXTERN         15
# define STNAME         16
# define UNAME          17
# define ENAME          18
# define MOS            19
# define MOU            20
# define MOE            21
# define TYPEDEF        22
# define TCSYM          23      /* Variable which is addressd off of TOC */
        /* field size is ORed in */
# define FIELD 0100
# define FLDSIZ 077
# ifndef BUG1
extern char *scnames();
# endif

/*      location counters */
# define PROG 0
# define DATA 1
# define ADATA 2
# define STRNG 3
# define ISTRNG 4
# define STAB 5
#ifdef XCOFF
# define RESET 6
#endif

/* symbol table flags */
# define SNSPACE        03      /* name space mask: */
#       define SREG             0       /* regular name */
#       define SMOS             01      /* structure member */
#       define STAG             02      /* structure tag */
#       define SLABEL           03      /* label name */
# define SNONUNIQ       04      /* non-unique structure member name */
# define SHIDDEN        010     /* currently hidden by nested definition */
# define SHIDES         020     /* definition hides earlier declaration */
# define SEXTRN         040     /* external symbol (extern or static) */
# define SSCOPED        0100    /* scoped out block scoped external symbol */
# define SSET           0200    /* symbol was assigned to */
# define SREF           0400    /* symbol value was used */
# define SLNAME         01000   /* symbol can be lname (auto variable) */
# define SPNAME         02000   /* symbol can be pname (parameter) */
#ifdef XCOFF
# define SFCALLED       04000   /* function was called */
# define SFADDR         010000  /* function address was referenced */
#endif


# ifndef FIXDEF
# define FIXDEF(p)
# endif
# ifndef FIXARG
# define FIXARG(p)
# endif
# ifndef FIXSTRUCT
# define FIXSTRUCT(a,b)
# endif

# ifndef SETDCON
# define SETDCON(p) 0
# endif

        /* alignment of initialized quantities */
# ifndef AL_INIT
#       define  AL_INIT ALINT
# endif

/*      type names, used in symbol table building */
/*
** If you change this list, there are several other places that have to
** be changed:  m_dep/local.c, m_ind/tytable.h, m_ind/manifest.h
*/
# define TNULL          000
# define TELLIPSIS      001
# define FARG           002
# define MOETY          003
# define SIGNED         004
# define UNDEF          005

/* "real" basic types start here */

# define TVOID          006
# define CHAR           007
# define SCHAR          010
# define SHORT          011
# define INT            012
# define LONG           013
# define LNGLNG         014
# define FLOAT          015
# define DOUBLE         016
# define LDOUBLE        017
# define UCHAR          020
# define USHORT         021
# define UNSIGNED       022
# define ULONG          023
# define ULNGLNG        024
# define ENUMTY         025
# define STRTY          026
# define UNIONTY        027
# define CPPCLASS       030

#define WCHAR USHORT

#define BIGINT  LNGLNG
#define BIGUINT ULNGLNG

# define NBTYPES        (CPPCLASS + 1)
# define NRBTYPES       (NBTYPES - TVOID)

/* type modifiers */
# define REF            033
# define MEMPTR         034
# define PTR            035
# define FTN            036
# define ARY            037

/* type flag/qualifier constants (must fit into a short due to dope[]) */
# define TSIGNED        040000  /* explicitly signed (for bitfields) */
# define HAVECONST      020000  /* struct/union has const member */
# define HAVEVOLATILE   010000  /* struct/union has volatile member */
# define CONST          000200
# define VOLATILE       000100
# define NOQUAL     0
# define INCQUAL    0100
# define ALLQUAL   (CONST | VOLATILE)

/* type packing constants */
# define TMASK          0377    /* mask for basic type & qualifiers */
# define BTMASK         037     /* mask for basic type */

/* type encoding constants */
# define PTROUT         01
# define FTNOUT         02
# define ARYOUT         03
# define TSHIFT         2       /* number of bits for type modifiers */
# define BTSHIFT        4       /* number of bits for basic type */

/*      macros  */
#define TOPTYPE(t)      ((t)->tword&BTMASK)
#define TOPQTYPE(t)     ((t)->tword&TMASK)
#define MODTYPE(t,bt)   (t=modtype(t,bt))
#define BTYPE(t)        (TOPTYPE(btype(t)))
#define ISBTYPE(t)      ((t)->next==TNIL)
#define ISUNSIGNED(t)   ((TOPTYPE(t) <= BIGUINT && TOPTYPE(t) >= UCHAR) || \
                         TOPTYPE(t) == CHAR)
#define ISTCHAR(bt)     ((bt) == UCHAR || (bt) == SCHAR || (bt) == CHAR)
#define ISCHAR(t)       ISTCHAR(TOPTYPE(t))

/* RW: 91 */
#define ISWCHAR(t)      (TOPTYPE(t) == WCHAR)

#define ISREF(t)        (TOPTYPE(t) == REF)
#define ISMEMPTR(t)     (TOPTYPE(t) == MEMPTR)
#define ISFLOAT(t)      (TOPTYPE(t) <= LDOUBLE && TOPTYPE(t) >= FLOAT)
#define ISARITHM(t)     (ISINTEGRAL(t) || ISFLOAT(t))
#define ISSCALAR(t)     (ISPTR(t) || ISARITHM(t))
#define ISTUNSIGNED(bt) (((bt) <= BIGUINT && (bt) >= UCHAR) || (bt) == CHAR)
#define UNSIGNABLE(bt)  ((bt) <= BIGINT && (bt) >= SCHAR)
#define ISINTEGRAL(t)   ((TOPTYPE(t) >= CHAR && TOPTYPE(t) <= BIGINT) || \
                         (TOPTYPE(t) >= UCHAR && TOPTYPE(t) <= ENUMTY))
#define ISBINTEGRAL(t)  ((TOPTYPE(t) >= CHAR && TOPTYPE(t) <= LONG) || \
                         (TOPTYPE(t) >= UCHAR && TOPTYPE(t) <= ULONG))
#define ENUNSIGN(bt)    ((bt) + (UNSIGNED-INT))
#define DEUNSIGN(bt)    ((bt) + (INT-UNSIGNED))
#define ISPTR(t)        (TOPTYPE(t) == PTR)
#define ISFTN(t)        (TOPTYPE(t) == FTN)
#define ISARY(t)        (TOPTYPE(t) == ARY)
#define DECREF(t)       ((t)->next != TNIL ? (t)->next : (t))
#define INCREF(t,bt)    incref(t,bt)
#define tyalloc(bt)     (&btypes[bt])
#define ISQUALIFIED(t,qt)       ((t)->tword&(qt))
#define ISCONST(t)      ((t)->tword & CONST)
#define ISVOLATILE(t)   ((t)->tword & VOLATILE)
#define QUALIFIERS(t)   ((t)->tword & (CONST|VOLATILE))
#define ISTSIGNED(t)    ((t)->tword & TSIGNED)
#define HASCONST(t)     ((t)->tword & HAVECONST)
#define HASVOLATILE(t)  ((t)->tword & HAVEVOLATILE)
#define ISCONSTANT(t)   ((t)->in.op == ICON || (t)->in.op == FCON)
#define ISNUMBER(t)     (((t)->in.op == ICON && (t)->tn.rval == NONAME) || \
                                (t)->in.op == FCON)
#define ISAGGREGATE(type) (TOPTYPE(type) == ARY || TOPTYPE(type) == STRTY)
#define ISUNION(type) (TOPTYPE(type) == UNIONTY)

#define ISTLNGLNG(bt) (bt == LNGLNG || bt == ULNGLNG)
#define ISTLONG(bt) (bt == LONG || bt == ULONG)

#if (SZINT != SZLONG)
 #define CONVtoINT(x) (x = (int)x)
 #define CONVtoUINT(x) (x = (unsigned)x)
#else
 #define CONVtoINT(x)
 #define CONVtoUINT(x)
#endif

#define CONVtoLONG(x) (x = (long)x)
#define CONVtoULONG(x) (x = (unsigned long)x)
#define CONVtoLNGLNG(x) (x = (long long)x)

#define USUFFIX(x) (x & 1)

#if 0

#define isFULLUNS(x) (x >= UNSIGNED && x <= BIGUINT)
#define LENGTHEN(x) (++x)
#define SHORTEN(x) (--x)

#endif

/* is an automatic of type t OK for LNAME or PNAME?  */
#define CanBeLNAME(t) (ISPTR(t) || ISARITHM(t))

typedef struct tyinfo *TPTR;
typedef struct parminfo *PPTR;

struct parminfo {
        TPTR    type;
        PPTR    next;
};

struct tyinfo {
        TWORD   tword;
        TPTR    next;
        union {
                unsigned size;
                PPTR     parms;
        }       info;
};

#define typ_size        info.size
#define ary_size        info.size
#define ftn_parm        info.parms

#define TNIL (0)        /* (TPTR) */
#define PNIL (0)        /* (PPTR) */
extern struct tyinfo btypes[];  /* array containing some predefined types */

struct symtab {
#if     defined (LINT) || defined (CFLOW)
        char *ifname;   /* included filename */
        short line;     /* line number symbol occurance */
#endif
        char *psname;   /* symbol name */
        TPTR stype;     /* type word */
        char sclass;    /* storage class */
        char slevel;    /* scope level */
        short sflags;   /* flags for set, use, hidden, mos, etc. */
        int offset;     /* offset or value */
        short suse;     /* line number of last use of the symbol */
        short uniqid;   /* unique identifier number in this function */
};

# ifdef ONEPASS
/* NOPREF must be defined for use in first pass tree machine */
# define NOPREF 020000  /* no preference for register assignment */
# else
#ifndef OPTIMIZER
union ndu {

        struct {
                int op;
                int rall;
                TPTR type;
                short su, fpsu;
                char filler[8];
                char *pname;
                short svsu; /* saved reg sethi ulman counter */
                short flags; /* extra flags field (lec) */
                NODE *left;
                NODE *right;
                }in; /* interior node */

        struct {
                int op;
                int rall;
                TPTR type;
                short su, fpsu;
                char filler[8];
                char *pname;
                short svsu;
                short flags;
                char filler2[8]; /* do not clobber left and right */
                CONSZ lval;
                int rval;
                }tn; /* terminal node */

        struct {
                int op, rall;
                TPTR type;
                short su, fpsu;
                int label;
                int notapplied;
                }bn; /* branch node */

        struct {
                int op, rall;
                TPTR type;
                short su, fpsu;
                int stsize;
                int stalign;
                }stn; /* structure node */

        struct {
                int op;
                int dummy1;
                TPTR type;
                int dummy2;
                }fn; /* front node */

        struct {
                int op;
                int dummy1;
                TPTR type;
                int dummy2;
#ifdef HOSTIEEE
                double dval;
#else
                FP_DOUBLE dval;
#endif
                }fpn; /* floating point node */

        };

#define NOTLVAL 2       /* to mark trees where we can't tell anymore */
#define SIDEFF 1 /* for floating register nodes, the result is also in r2,r3 */

#endif
# endif

/* These definitions are for lint(1) style warnings. */
# define VAL 0
# define EFF 1

# define LNAMES 250
struct lnm {
        short lid, flgs;
};
extern struct lnm lnames[LNAMES], *lnp;


extern struct sw swtab[];
extern struct sw *swp;
extern int swx;

extern struct tyinfo labltype;

extern int nGlobTyEnts;
extern int nLoclTyEnts;
extern TPTR globTyTab;
extern TPTR loclTyTab;

extern unsigned int Initializer;
extern int foldadd;
extern unsigned int foldMask;
extern int ftnno;
extern int blevel;
extern int tempBlevel;
extern int instruct, stwart, funcstyle, volEmitted, funcConflict, paramFlg;

extern int lineno, nerrors;
typedef union {
        int intval;
        long long llval;
        NODE * nodep;
        } YYSTYPE;
extern YYSTYPE yylval;

extern CONSZ lastcon;
#ifdef HOSTIEEE
extern double dcon;
#else
extern FP_DOUBLE dcon;
#endif
extern char ftitle[];
extern char ititle[];
extern int nstabents;
extern int ndiments;
extern struct symtab *stab;
#ifdef XCOFF
extern int *saved_str;
extern int max_chars;
extern int saved_chars;
extern int *saved_lab;
extern int max_strings;
extern int saved_strings;
#endif
extern int curftn;
extern int curLevel;
extern int curclass;
extern int curdim;
extern int *dimtab;
struct dnode {
        int index;
        short cextern;
        };
extern struct dnode *dimptr, dimrec[];
extern int *paramstk;
extern int *protostk;
extern int paramno;
extern int paramchk;
#ifdef LINT
extern int vflag;
#endif
#if defined(LINT) || defined(CFLOW) || defined(CXREF)
extern int func_ptr;
#endif
extern int paramlevel;
extern int autooff, argoff, strucoff;
extern int regvar;
extern int fpregvar;
extern char yytext[];

extern int strflg;
extern int strsize;

extern OFFSZ inoff;

extern int reached;

/* LINT flags. */
extern int lintargu;    /* for the ARGSUSED directive */
extern int lintvarg;    /* for the VARARGS directive */
extern int lintlib;     /* for the LINTLIBRARY directive */
extern int lintnrch;    /* for the NOTREACHED directive */
extern int lintused;    /* for the NOTUSED directive */
extern int lintdefd;    /* for the NOTDEFINED directive */
extern int lintrsvd;    /* for the LINTSTDLIB directive */

#if     defined(LINT) || defined(LINTP2) || defined(CFLOW) || defined(CFLOW2)

/* These three variables not needed by LINTP2, better to be general. */
extern char *pfname;            /* physical filename (stripped) */
extern char *ifname;            /* included filename (stripped) */
extern FILE *tmplint;           /* temporary output file */

/* Definitions for iocode (char). */
#define LINTBOF         1       /* beginning of lint file */
#define LINTEOF         2       /* end of lint file */
#define LINTSYM         3       /* new symbol */
#define LINTADD         4       /* add to previous symbol */
#define CFLOWBOF        5       /* beginning of cflow file */
#define CFLOWEOF        6       /* end of cflow file */

/*
** Usage definitions (short).
** Leave first two bits open for sflags&SNSPACE.
*/
#define LINTDEF         04      /* symbol definition */
#define LINTREF         010     /* symbol reference */
#define LINTDCL         020     /* symbol declaration (extern) */
#define LINTMBR         040     /* symbol has member(s) */
#define LINTTAG         0100    /* symbol member has tagname */
#define LINTRET         0200    /* function has return value */
#define LINTUSE         0400    /* function return value used */
#define LINTIGN         01000   /* function return value ignored */
#define LINTLIB         02000   /* LINTLIBRARY flag */
#define LINTVRG         04000   /* VARARGSn flag */
#define LINTNOT         010000  /* NOTUSED flag */
#define LINTNDF         020000  /* NOTDEFINED flag */
#define LINTADDR        040000  /* function address referenced */

#endif

extern int ininit;

/*      tunnel to buildtree for name id's */

extern int idname;

extern NODE *node;
extern NODE *lastfree;

/* various labels */
extern int brklab;
extern int contlab;
extern int flostat;
extern int retlab;
extern int retstat;
extern int loop_level;
extern int asavbc[], *psavbc;

/*      flags used for name space determination */

# define SEENAME 01
# define INSTRUCT 02
# define INUNION 04
# define FUNNYNAME 010
# define TAGNAME 020
# define LABNAME 040

/*      flags used in the (elementary) flow analysis ... */

# define FBRK 02
# define FCONT 04
# define FDEF 010
# define FLOOP 020

/*      flags used for return status */

# define RETVAL 1
# define NRETVAL 2

/*      used to mark a constant with no name field */

# define NONAME 040000

/*      flags used for function prototypes */

# define OLD_STYLE 1
# define NEW_STYLE 2

/*  used to check for misuse of void and ellipsis in parameter lists */

# define SAW_VOID       1
# define SAW_ELLIP      2

        /* mark an offset which is undefined */

# define NOOFFSET (-10201)

/*      declarations of various functions */

extern NODE
        *docast(),
        *foldexpr(),
        *oldfoldexpr(),
        *buildtree(),
        *bdty(int, NODE *, CONSZ),
        *mkty(),
        *rstruct(),
        *dclstruct(),
        *getstr(),
        *tymerge(),
        *stref(),
        *offcon(),
        *bcon(CONSZ),
        *bpsize(),
        *convert(),
        *pconvert(),
        *oconvert(),
        *ptmatch(),
        *tymatch(),
        *makety(),
        *block(),
        *doszof(),
        *talloc(),
        *optim(),
        *strargs(),
        *fixargs(),
        *cvtarg(),
        *clocal();

CONSZ   icons(NODE *);

OFFSZ   tsize(),
        psize();

TWORD   ctype();

TPTR    incref(),
        btype(),
        modtype(),
        qualtype(),
        unqualtype(),
        signedtype(),
        qualmember(),
        tynalloc(),
        copytype(),
        CheckType(),
        CheckTypedef(),
        ResultType(),
        FindBType();

PPTR    parmalloc();

# define checkst(x)

# define CHARCAST(x) (unsigned char)(x) /* how AIWS does it! */
# ifndef CHARCAST
/* to make character constants into character connstants */
/* this is a macro to defend against cross-compilers, etc. */
# define CHARCAST(x) (char)(x)
# endif

#       define BCSZ 125         /* table size to save break, continue labels */
#       define SYMTSZ 1500      /* size of the symbol table */
#       define DIMTABSZ 2000    /* size of the dimension/size table */
#       define BNEST 30         /* Block Nesting Depth */
#       define GLOBTYSZ 2000    /* global type table size */
#       define LOCLTYSZ 8000    /* local type table size */
#       define MAXBUNCH 20      /* parameter node allocation chunk */
        extern int paramsz;     /* size of the parameter stack */
        extern int protosz;     /* size of the prototype stack */
/*      special interfaces for yacc alone */
/*      These serve as abbreviations of 2 or more ops:
        ASOP    =, = ops
        RELOP   LE,LT,GE,GT
        EQUOP   EQ,NE
        DIVOP   DIV,MOD
        SHIFTOP LS,RS
        ICOP    ICR,DECR
        UNOP    NOT,COMPL
        STROP   DOT,STREF

        */
# define ASOP 25
# define RELOP 26
# define EQUOP 27
# define DIVOP 28
# define SHIFTOP 29
# define INCOP 30
# define UNOP 31
# define STROP 32

# define LP 50
# define RP 51
# define LC 52
# define RC 53

/*      These defines control "for" and "while" loop generation.
 *      Each of wloop_level and floop_level must be set to one
 *      of these values.
 */
# define LL_TOP  0      /* test at top of loop          */
# define LL_BOT  1      /* test at bottom of loop       */
# define LL_DUP  2      /* duplicate test at top and bottom of loop  */


extern int warnlevel[MXDBGFLG+1];

/* These define warning error classes for WARNING(). */
# define WALWAYS        1                       /* always give warning */
# define WCONSTANT      warnlevel['C']          /* warnings for constant usages */
# define WANSI          warnlevel['a']          /* non-ansi features */
# define WUCOMPAR       warnlevel['c']          /* unsigned comparisons */
# define WUDECLAR       warnlevel['D']          /* unused declarations */
# define WDECLAR        warnlevel['d']          /* missing or re-declarations */
# define WHEURISTIC     warnlevel['h']          /* heuristic complaints */
# define WKNR           warnlevel['k']          /* K+R type code expected */
# define WLONGASSGN     warnlevel['l']          /* assignments of long values to not long variables */
# define WNULLEFF       warnlevel['n']          /* null effect usage */
# define WOBSOLETE      warnlevel['O']          /* obsolescent features */
# define WEORDER        warnlevel['o']          /* evaluation order check */
# define WPROTO         warnlevel['P']          /* prototype checks */
# define WPORTABLE      warnlevel['p']          /* portability checks */
# define WREACHED       warnlevel['R']          /* reachable code check */
# define WRETURN        warnlevel['r']          /* function return checks */
# define WSTORAGE       warnlevel['s']          /* storage packing check */
# define WUSAGE         warnlevel['u']          /* extern vars or ftn param not used */


/*
 * constant expression masks:
 * 1.   If bit 0 (from the right) is set then the expression is a
 *      constant expression.
 * 2.   If bit 1 (from the right) is set then the expression
 *      is an integral constant expression.
 *
 * EXPRESSION           -- a mask to indicate a full-fledged expression
 * GENERAL_CONSTANT     -- a mask to indicate a constant expression
 * INTEGRAL_CONSTANT    -- a mask to further restrict a constant expression
 *                         to be integral.
 * FOLD_INTEGRAL        -- are we folding an integral constant expression
 * FOLD_GENERAL         -- or are folding a general constant expression
 * NO_FOLD              -- run-time evalutation of expression only
 */
#define EXPRESSION              (0x0)
#define GENERAL_CONSTANT        (0x1)
#define INTEGRAL_CONSTANT       (0x2)
#define FOLDABLE                (INTEGRAL_CONSTANT | GENERAL_CONSTANT)
#define FOLD_INTEGRAL()         (foldMask & INTEGRAL_CONSTANT)
#define FOLD_GENERAL()          (foldMask & GENERAL_CONSTANT)
#define NO_FOLD()               (foldMask == EXPRESSION)

/*
 * initializers masks:
 *      SINGLE  : single expression initializer;
 *      LIST    : initializer list (constant expression...see above)
 *      HAVEINIT: a mask to indicate that we have an initialization
 *                (for use with defid()).
 *
 *      HAVEINIT implicitly implies LIST. thus, a single expression
 *      initializer is a further restricticion on initialization.
 *
 *      if no initialization is seen, then we have a bit pattern: 00
 */
#define NOINIT          0x0     /* bit pattern: 00 */
#define HAVEINIT        0x2     /* bit pattern: 10 */
#define SINGLE          0x3     /* bit pattern: 11 */
#define LIST            0x2     /* bit pattern: 10 */
#define ISHAVEINIT      (Initializer & HAVEINIT)
#define ISSINGLE        (Initializer == SINGLE)
#define ISLIST          (Initializer == LIST)
