/* @(#)46       1.7  src/bos/usr/ccs/bin/common/manifest.h, cmdprog, bos411, 9428A410j 6/3/91 11:59:55 */
/*
 * COMPONENT_NAME: (CMDPROG) manifest.h
 *
 * FUNCTIONS: NOFIT, PKFIELD, SETOFF, UPKFOFF, UPKFSZ, asgop, callop, logop  
 *            optype                                                          
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
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
# include <stdio.h>
/*      manifest constant file for the lex/yacc interface */

# define ERROR 1
# define NAME 2
# define STRING 3
# define WSTRING 600
# define ICON 4
# define FCON 5
# define PLUS 6
# define MINUS 8
# define MUL 11
# define AND 14
# define OR 17
# define ER 19
# define QUEST 21
# define COLON 22
# define ANDAND 23
# define OROR 24

/* the defines for ASOP, RELOP, EQUOP, DIVOP,
   SHIFTOP, ICOP, UNOP, and STROP have been
   moved to mfile1                              */
/*      reserved words, etc */
# define TYPE 33
# define CLASS 34
# define STRUCT 35
# define RETURN 36
# define GOTO 37
# define IF 38
# define ELSE 39
# define SWITCH 40
# define BREAK 41
# define CONTINUE 42
# define WHILE 43
# define DO 44
# define FOR 45
# define DEFAULT 46
# define CASE 47
# define SIZEOF 48
# define ENUM 49


/*      little symbols, etc. */
/*      namely,

        LP      (
        RP      )

        LC      {
        RC      }

        LB      [
        RB      ]

        CM      ,
        SM      ;

        */

/*  These defines are being moved to mfile1
    to alleviate preprocessor problems with
    second pass files.
# define LP 50
# define RP 51
# define LC 52
# define RC 53
*/
# define LB 54
# define RB 55
# define CM 56
# define SM 57
# define ASSIGN 58
# define ASM 59

/*      END OF YACC */

/*      left over tree building operators */
# define COMOP 59
# define DIV 60
# define MOD 62
# define LS 64
# define RS 66
# define DOT 68
# define STREF 69
# define CALL 70
# define FORTCALL 73
# define NOT 76
# define COMPL 77
# define INCR 78
# define DECR 79
# define EQ 80
# define NE 81
# define LE 82
# define LT 83
# define GE 84
# define GT 85
# define ULE 86
# define ULT 87
# define UGE 88
# define UGT 89
# define SETBIT 90
# define TESTBIT 91
# define RESETBIT 92
# define ARS 93
# define REG 94
# define OREG 95
# define CCODES 96
# define FREE 97
# define STASG 98
# define STARG 99
# define STCALL 100

/*      some conversion operators */
# define FLD 103
# define SCONV 104
# define PCONV 105
# define PMCONV 106
# define PVCONV 107

/*      special node operators, used for special contexts */
# define FORCE 108
# define CBRANCH 109
# define INIT 110
# define CAST 111
# define LNAME 112      /* atomic local automatic */
# define PNAME 113      /* atomic local parm */
# define TNAME 114      /* optimizer stack temporary */
# define TREG 115       /* optimizer register temporary */

# define OCONVLEAF 116  /* optimizer conversion node - leaf     */
# define OCONVTREE 117  /* optimizer conversion node - tree     */
# define LADDR 118      /* address of a local           */
# define PADDR 119      /* address of a param           */
# define ADDR 120       /* address of anything else     */

# define LEAFNOP 121    /* leaf no-op - used by optimizer       */
# define UNARYNOP 122   /* unary no-op - used by optimizer      */

# define LTEMP 123      /* Loop invariant temporary */
# define STATNAME 124   /* Atomic local static - used mostly by Fort */
# define STADDR 125     /* Address of a STATNAME        */

/*      special added to indicate T bit in AFWS     sal */
# define CCODET 126     /* old value = 112      */
# define DOLR   127     /* old value = 113      */

#define SFCON   128     /* single precision floating constant */
#define LABCON  129
#define CONV    130     /* rewrite rule for FLOAT REG op= INT */

#define QUAL    131     /* type qualifier */
#define TYDEF   132     /* typedef specifier */
#define PARAMETER       133     /* convert parameter type */
#define ELLIPSIS        134     /* the '...' symbol */
/*      THIS IS THE LAST LABEL  */
# define STLABEL 135

/*      node types */
# define LTYPE 02
# define UTYPE 04
# define BITYPE 010

        /* DSIZE is the size of the dope array */
# define DSIZE STLABEL+1

# define ASG 1+
# define UNARY 2+
# define NOASG (-1)+
# define NOUNARY (-2)+

/* extra defines for op-assign tokens, they take up the job of ASG */
#define ASG_PLUS 7
#define ASG_MINUS 9
#define ASG_MUL 12
#define ASG_AND 15
#define ASG_DIV 61
#define ASG_MOD 63
#define ASG_LS 65
#define ASG_RS 67
#define ASG_OR 18
#define ASG_ER 20
/*      various flags */
# define NOLAB (-1)

# define SETOFF(x,y)   if( (x)%(y) != 0 ) (x) = ( ((x)/(y) + 1) * (y))
                /* advance x to a multiple of y */
# define NOFIT(x,y,z)   ( ( (x)%(z) + (y) ) > (z) )
                /* can y bits be added to x without overflowing z */
        /* pack and unpack field descriptors (size and offset) */
# define PKFIELD(s,o) (( (o)<<6)| (s) )
# define UPKFSZ(v)  ( (v) &077)
# define UPKFOFF(v) ( (v) >>6)

/*      operator information */

# define TYFLG 016
# define ASGFLG 01
# define LOGFLG 020

# define SIMPFLG 040
# define COMMFLG 0100
# define DIVFLG 0200
# define FLOFLG 0400
# define LTYFLG 01000
# define CALLFLG 02000
# define MULFLG 04000
# define SHFFLG 010000
# define ASGOPFLG 020000
# define BITFLG 040000

# define SPFLG 0100000

#define optype(o) (dope[o]&TYFLG)
#define asgop(o) (dope[o]&ASGFLG)
#define logop(o) (dope[o]&LOGFLG)
#define callop(o) (dope[o]&CALLFLG)

/*      table sizes     */

# define TREESZ 1000 /* space for building parse tree (was 350 and 500) */
extern int ntrnodes; /* number of tree nodes */

/*      common defined variables */

extern int nerrors;  /* number of errors seen so far */

typedef union ndu NODE;
typedef unsigned int TWORD;

# define NIL (0)        /* (NODE *) */
extern int dope[];  /* a vector containing operator information */
extern char *opst[];  /* a vector containing names for ops */
extern OFFSZ offsz, caloff();

# ifdef BUG2
# define BUG1
# endif
# ifdef BUG3
# define BUG2
# define BUG1
# endif
# ifdef BUG4
# define BUG1
# define BUG2
# define BUG3
# endif
# ifndef ONEPASS

# ifndef EXPR
# define EXPR '$'
# endif
# ifndef BBEG
# define BBEG '['
# endif
# ifndef BEND
# define BEND ']'
# endif
# ifndef FBEGIN
# define FBEGIN '{'
# endif
# ifndef FEND
# define FEND '}'
# endif
# ifndef IRETURN
# define IRETURN 'R'
# endif
# ifndef IBRANCH
# define IBRANCH '>'
# endif
# ifndef ILABEL
# define ILABEL '<'
# endif
# ifndef ITEXT
# define ITEXT 'T'
# endif
# ifndef IDATA
# define IDATA 'D'
# endif
# ifndef ESWITCH
# define ESWITCH 'S'
# endif
# ifndef PFHERE
# define PFHERE 'H'
# endif
# ifndef PFETCH
# define PFETCH 'P'
# endif
# ifndef PSAVE
# define PSAVE 'V'
# endif
# ifndef IASM
# define IASM 'A'
# endif
# ifndef STABINFO
# define STABINFO '@'
# endif

# endif

/* Definitions for switches - needed by both passes in split compiler */

# ifdef pdp11
#       define SWITSZ   250             /* size of switch table */
# else
#       define SWITSZ   2000            /* size of switch table */
# endif

struct sw {
        CONSZ sval;
        int slab;
        };

extern struct sw swtab[];


#define   MXDBGFLG  128

extern int devdebug[MXDBGFLG+1];

/* keep the defines sorted by the macro replacment strings
   for easier access.
*/

#define ANSI_MODE       'A'     /* use all ANSI rules, NOTE: if any
                                 * ANSI flags are changed, mainp1() must
                                 * be updated to reflect the change.
                                 */
#define ANSI_PARSE      'a'     /* follow ANSI parsing rules */
#define STRUCTBUG       'b'     /* follow ANSI const array struct member bug */
#define COMPATIBLE      'c'     /* follow ANSI type compatibility rules */
#define PROMOTION       'p'     /* follow ANSI type promotion rules */
#define REFDEF          'r'     /* follow ANSI strict ref/def rules */
#define SCOPING         's'     /* follow ANSI scoping rules for externs */
#define TYPING          't'     /* follow ANSI typing rules */
#define BITFIELDS       'B'     /* allow arbitrary types in :0 bitfields */

#include <limits.h>
#ifdef COMPAT
#define KLUDGE          'K'
#endif

