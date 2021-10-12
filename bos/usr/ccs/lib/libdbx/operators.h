/* @(#)69    1.10.2.5  src/bos/usr/ccs/lib/libdbx/operators.h, libdbx, bos411, 9428A410j 9/15/93 10:10:18 */
#ifndef _h_operators
#define _h_operators
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) degree, isbinary, isbitset, isboolean, isint, isleaf,
 * 	      isreal, isunary, nargs
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

typedef struct {
    signed char numargs;
    signed char opflags;
    String opstring;
} Opinfo;

typedef enum {
    O_NOP, O_NAME, O_SYM, O_LCON, O_ULCON, O_LLCON, O_ULLCON,
    O_CCON, O_FCON, O_QCON, O_KCON, O_QKCON, O_SCON, 
    O_SETCON, O_RVAL, O_UNRVAL, O_INDEX, O_INDIR, O_INDIRA, O_DOT, O_FREE, 
    O_ADDR, O_BSET, O_COMMA, O_DOTDOT, O_DOTSTAR, O_UNRES, O_SCOPE, 
    O_INDEX_OR_CALL, O_NAMELIST, O_CPPREF,

    O_ITOF, O_ITOQ, O_FTOQ,
    O_ADD, O_ADDF, O_ADDQ, O_PADD,
    O_SUB, O_SUBF, O_SUBQ, O_PSUB,
    O_NEG, O_NEGF, O_NEGQ,
    O_MUL, O_MULF, O_MULQ,
    O_DIVF, O_DIV, O_DIVQ, O_MOD,

    O_NOT, O_AND, O_OR,

    O_BAND, O_BOR, O_BXOR, O_COMP, O_EXP, O_SIZEOF, O_EXPQ,

    O_LT, O_LTF, O_LTQ, O_LE, O_LEF, O_LEQ, 
    O_GT, O_GTF, O_GTQ, O_GE, O_GEF, O_GEQ,
    O_EQ, O_EQF, O_EQQ, O_NE, O_NEF, O_NEQ,
    O_SL, O_SR,

    O_ALIAS,		/* rename a command */
    O_ASSIGN,		/* assign a value to a program variable */
    O_CALL,		/* call a procedure in the program */
    O_CASE,		/* control folding symbols to a case */
    O_CATCH,		/* catch a signal before program does */
    O_CHFILE,		/* change (or print) the current source file */
    O_CLEAR,		/* clear all breakpoints on a line */
    O_CLEARI,		/* clear all breakpoints at an address */
    O_CONT,		/* continue execution */
    O_CPU,              /* switch cpu on target machine (kdbx) */
    O_DEBUG,		/* invoke a dbx internal debugging routine */
    O_DELALL,		/* remove all trace/stops */
    O_DELETE,		/* remove a trace/stop */
    O_DUMP,		/* dump out variables */
    O_EDIT,		/* edit a file (or function) */
    O_FUNC,		/* set the current function */
    O_GOTO,		/* change point of execution */
    O_HELP,		/* print a synopsis of debugger commands */
    O_KLOAD,		/* update load map */
    O_IGNORE,		/* let program catch signal */
    O_LISTI,		/* list assembly code */
    O_LIST,		/* list source lines */
    O_LLDB,		/* low level debugger */
    O_MAP,		/* dump loader information */
    O_MOVE,		/* move next line to be displayed */
    O_OBJECT,		/* Change object file being used */
    O_PRINT,		/* print the values of a list of expressions */
    O_PROMPT,		/* change or display the prompt */
    O_PSYM,		/* print symbol information */
    O_REGS,		/* print register information */
    O_RUN,		/* start up program */
    O_SKIP,		/* skip x number of breakpoints */
    O_SOURCE,		/* read commands from a file */
    O_STATUS,		/* display currently active trace/stop's */
    O_STEP,		/* execute a single line */
    O_STOP,		/* stop on an event */
    O_STOPI,		/* stop on an event at an instruction boundary */
    O_SYMTYPE,		/* get symbol type */
    O_TRACE,		/* trace something on an event */
    O_TRACEI,		/* trace at the instruction level */
    O_WATCH,		/* display a variable in a trace window */
    O_WHATIS,		/* print the declaration of a variable */
    O_WHERE,		/* print a stack trace */
    O_WHEREIS,		/* print all the symbols with the given name */
    O_WHICH,		/* print out full qualification of a symbol */
    O_EXAMINE,		/* examine program instructions/data */

    O_THREAD,           /* list existing CMA threads */
    O_ATTRIBUTE,        /* list existing CMA attributes */
    O_CONDITION,        /* list existing CMA condition variables */
    O_MUTEX,            /* list existing CMA mutexes */

    O_ADDEVENT,		/* add an event */
    O_ENDX,		/* end of program reached */
    O_IF,		/* if first arg is true, do commands in second arg */
    O_ONCE,		/* add a "one-time" event, delete when first reached */
    O_PRINTCALL,	/* print out the current procedure and its arguments */
    O_PRINTIFCHANGED,	/* print the value of the argument if it has changed */
    O_PRINTRTN,		/* print out the routine and value that just returned */
    O_PRINTSRCPOS,	/* print out the current source position */
    O_PROCRTN,		/* call completed */
    O_QLINE,		/* filename, line number */
    O_STOPIFCHANGED,	/* stop if the value of the argument has changed */
    O_STOPX,		/* stop execution */
    O_TRACEON,		/* begin tracing source line, variable, or all lines */
    O_TRACEOFF,		/* end tracing source line, variable, or all lines */

    O_TYPERENAME,	/* cast the type of an expression */
    O_PTRRENAME,	/* cast the type of the address of an expression */
    O_REFRENAME,	/* cast the type of an expression to a reference */
    O_RERUN,		/* re-run program with the same arguments as before */
    O_RETURN,		/* continue execution until procedure returns */
    O_UP,		/* move current function up the call stack */
    O_DOWN,		/* move current function down the call stack */
    O_CALLPROC,		/* call command */
    O_SEARCH,		/* regular expression pattern search through source */
    O_SWITCH,           /* switch context to another thread (kdbx) */
    O_SET,		/* set a debugger variable */
    O_UNSET,		/* unset a debugger variable */
    O_UNALIAS,		/* remove an alias */
    O_WINS,		/* change size of source window under X */
    O_WINI,		/* change size of instruction window under X */

    O_SCREEN,           /* switch to new virtual terminal         */
    O_MULTPROC,         /* turn on/off multi-process debugging    */
    O_XCALL,            /* print using a user defined function (kdbx) */
    O_DETACH,           /* detach dbx from program being debugged   */
    O_LASTOP
} Operator;

/*
 * Operator flags and predicates.
 */

#define null 0
#define LEAF 01
#define UNARY 02
#define BINARY 04
#define BOOL 010
#define REALOP 020
#define INTOP 040

#define isbitset(a, m)	((a&m) == m)
#define isleaf(o)	isbitset(opinfo[ord(o)].opflags, LEAF)
#define isunary(o)	isbitset(opinfo[ord(o)].opflags, UNARY)
#define isbinary(o)	isbitset(opinfo[ord(o)].opflags, BINARY)
#define isreal(o)	isbitset(opinfo[ord(o)].opflags, REALOP)
#define isint(o)	isbitset(opinfo[ord(o)].opflags, INTOP)
#define isboolean(o)	isbitset(opinfo[ord(o)].opflags, BOOL)

#define degree(o)	(opinfo[ord(o)].opflags&(LEAF|UNARY|BINARY))
#define nargs(o)	(opinfo[ord(o)].numargs)

extern Opinfo opinfo[] ;
#endif /* _h_operators */
