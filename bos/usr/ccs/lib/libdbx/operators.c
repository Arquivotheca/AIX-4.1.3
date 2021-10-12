static char sccsid[] = "@(#)68    1.11.2.7  src/bos/usr/ccs/lib/libdbx/operators.c, libdbx, bos411, 9428A410j 11/9/93 15:38:36";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS:
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

/*
 * Tree node classes.
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include "defs.h"
#include "operators.h"

/*
 * Operator information structure.
 */

public Opinfo opinfo[] ={
/* O_NOP */		0,	null,		0,
/* O_NAME */		-1,	LEAF,		0,
/* O_SYM */		-1,	LEAF,		0,
/* O_LCON */		-1,	LEAF,		0,
/* O_ULCON */           -1,     LEAF,           0,
/* O_LLCON */           -1,     LEAF,           0,
/* O_ULLCON */          -1,     LEAF,           0,
/* O_CCON */		-1,	LEAF,		0,
/* O_FCON */		-1,	LEAF,		0,
/* O_QCON */		-1,	LEAF,		0,
/* O_KCON */		-1,	LEAF,		0,
/* O_QKCON */		-1,	LEAF,		0,
/* O_SCON */		-1,	LEAF,		0,
/* O_SETCON */		-1,	LEAF,		0,
/* O_RVAL */		1,	UNARY,		0,
/* O_UNRVAL */		1,	UNARY,		0,
/* O_INDEX */		2,	null,		0,
/* O_INDIR */		1,	UNARY,		"^",
/* O_INDIRA */		1,	UNARY,		"^",
/* O_DOT */		2,	null,		".",
/* O_FREE */		1,	null,		0,
/* O_ADDR */		1,	null,		"&",
/* O_BSET */		1,	null,		0,
/* O_COMMA */		2,	null,		",",
/* O_DOTDOT */		2,	null,		"..",
/* O_DOTSTAR */         2,      null,           ".*",
/* O_UNRES */		2,	null,		"::",
/* O_SCOPE */		2,	null,		"::",
/* O_INDEX_OR_CALL */   2,      null,           "?",
/* O_NAMELIST */        1,      null,           "name list",
/* O_CPPREF */          1,      UNARY,          "&",
/* O_ITOF */		1,	UNARY|INTOP,	0,
/* O_ITOQ */		1,	UNARY|INTOP,	0,
/* O_FTOQ */		1,	UNARY|REALOP,	0,
/* O_ADD */		2,	BINARY|INTOP,	"+",
/* O_ADDF */		2,	BINARY|REALOP,	"+",
/* O_ADDQ */		2,	BINARY|REALOP,	"+",
/* O_PADD */		2,	BINARY|INTOP,	"*+*",
/* O_SUB */		2,	BINARY|INTOP,	"-",
/* O_SUBF */		2,	BINARY|REALOP,	"-",
/* O_SUBQ */		2,	BINARY|REALOP,	"-",
/* O_PSUB */		2,	BINARY|INTOP,	"*-*",
/* O_NEG */		1,	UNARY|INTOP,	"-",
/* O_NEGF */		1,	UNARY|REALOP,	"-",
/* O_NEGQ */		1,	UNARY|REALOP,	"-",
/* O_MUL */		2,	BINARY|INTOP,	"*",
/* O_MULF */		2,	BINARY|REALOP,	"*",
/* O_MULQ */		2,	BINARY|REALOP,	"*",
/* O_DIVF */		2,	BINARY|REALOP,	"/",
/* O_DIV */		2,	BINARY|INTOP,	" div ",
/* O_DIVQ */		2,	BINARY|REALOP,	"/",
/* O_MOD */		2,	BINARY|INTOP,	" mod ",
/* O_NOT */		1,	UNARY|INTOP,	" not ",
/* O_AND */		2,	BINARY|INTOP,	" and ",
/* O_OR */		2,	BINARY|INTOP,	" or ",
/* O_BAND */		2,	BINARY|INTOP,	" bitand ",
/* O_BOR */		2,	BINARY|INTOP,	" | ",
/* O_BXOR */		2,	BINARY|INTOP,	" xor ",
/* O_COMP */		1,	UNARY|INTOP,	"~",
/* O_EXP */		2,	BINARY|REALOP,	"**",
/* O_SIZEOF */		1,	null,		"sizeof ",
/* O_EXPQ */		2,	BINARY|REALOP,	"**",
/* O_LT */		2,	BINARY|INTOP,	" < ",
/* O_LTF */		2,	BINARY|REALOP,	" < ",
/* O_LTQ */		2,	BINARY|REALOP,	" < ",
/* O_LE */		2,	BINARY|INTOP,	" <= ",
/* O_LEF */		2,	BINARY|REALOP,	" <= ",
/* O_LEQ */		2,	BINARY|REALOP,	" <= ",
/* O_GT */		2,	BINARY|INTOP,	" > ",
/* O_GTF */		2,	BINARY|REALOP,	" > ",
/* O_GTQ */		2,	BINARY|REALOP,	" > ",
/* O_GE */		2,	BINARY|INTOP,	" >= ",
/* O_GEF */		2,	BINARY|REALOP,	" >= ",
/* O_GEQ */		2,	BINARY|REALOP,	" >= ",
/* O_EQ */		2,	BINARY|INTOP,	" = ",
/* O_EQF */		2,	BINARY|REALOP,	" = ",
/* O_EQQ */		2,	BINARY|REALOP,	" = ",
/* O_NE */		2,	BINARY|INTOP,	" <> ",
/* O_NEF */		2,	BINARY|REALOP,	" <> ",
/* O_NEQ */		2,	BINARY|REALOP,	" <> ",
/* O_SL */		2,	BINARY|INTOP,	"<<",
/* O_SR */		2,	BINARY|INTOP,	">>",

/* O_ALIAS */		2,	null,		"alias",
/* O_ASSIGN */		2,	null,		" := ",
/* O_CALL */		2,	null,		"call",
/* O_CASE */        	0,      null,           "case",
/* O_CATCH */		1,	null,		"catch",
/* O_CHFILE */		0,	null,		"file",
/* O_CLEAR */		2,	null,		"clear",
/* O_CLEARI */		1,	null,		"cleari",
/* O_CONT */		1,	null,		"cont",
/* O_CPU */             1,      null,          "cpu",
/* O_DEBUG */		0,	null,		"debug",
/* O_DELALL */		0,	null,		"delete all",
/* O_DELETE */		1,	null,		"delete",
/* O_DUMP */		1,	null,		"dump",
/* O_EDIT */		0,	null,		"edit",
/* O_FUNC */		1,	null,		"func",
/* O_GOTO */		1,	null,		"goto",
/* O_HELP */		1,	null,		"help",
/* O_KLOAD */		0,	null,		"kload",
/* O_IGNORE */		1,	null,		"ignore",
/* O_LISTI */		2,	null,		"listi",
/* O_LIST */		2,	null,		"list",
/* O_LLDB */		1,	null,		"lldb",
/* O_MAP */		0,	null,		"map",
/* O_MOVE */		2,	null,		"move",
/* O_OBJECT */          1,      null,           "object",
/* O_PRINT */		1,	null,		"print",
/* O_PROMPT */		1,	null,		"prompt",
/* O_PSYM */		1,	null,		"psym",
/* O_REGS */		0,	null,		"registers",
/* O_RUN */		0,	null,		"run",
/* O_SKIP */		1,	null,		"skip",
/* O_SOURCE */		0,	null,		"source",
/* O_STATUS */		0,	null,		"status",
/* O_STEP */		0,	null,		"step",
/* O_STOP */		4,	null,		"stop",
/* O_STOPI */		4,	null,		"stopi",
/* O_SYMTYPE */         1,	null,		"symtype",
/* O_TRACE */		3,	null,		"trace",
/* O_TRACEI */		3,	null,		"tracei",
/* O_WATCH */		3,	null,		"watch",
/* O_WHATIS */		2,	null,		"whatis",
/* O_WHERE */		0,	null,		"where",
/* O_WHEREIS */		1,	null,		"whereis",
/* O_WHICH */		1,	null,		"which",
/* O_EXAMINE */		0,	null,		"examine",

/* O_THREAD */          2,      null,           "thread",
/* O_ATTRIBUTE */       2,      null,           "attribute",
/* O_CONDITION */       2,      null,           "condition",
/* O_MUTEX */           2,      null,           "mutex",

/* O_ADDEVENT */	0,	null,		"when",
/* O_ENDX */		0,	null,		nil,
/* O_IF */		0,	null,		"if",
/* O_ONCE */		0,	null,		"once",
/* O_PRINTCALL */	1,	null,		"printcall",
/* O_PRINTIFCHANGED */	2,	null,		"printifchanged",
/* O_PRINTRTN */	1,	null,		"printrtn",
/* O_PRINTSRCPOS */	2,	null,		"printsrcpos",
/* O_PROCRTN */		1,	null,		"procrtn",
/* O_QLINE */		2,	null,		nil,
/* O_STOPIFCHANGED */	2,	null,		"stopifchanged",
/* O_STOPX */		0,	null,		"stop",
/* O_TRACEON */		1,	null,		"traceon",
/* O_TRACEOFF */	1,	null,		"traceoff",
/* O_TYPERENAME */	2,	UNARY,		"type rename",
/* O_PTRRENAME */	2,	UNARY,		"pointer rename",
/* O_REFRENAME */	2,	UNARY,		"reference rename",
/* O_RERUN */		0,	null,		"rerun",
/* O_RETURN */		1,	null,		"return",
/* O_UP */		1,	UNARY,		"up",
/* O_DOWN */		1,	UNARY,		"down",
/* O_CALLPROC */	2,	null,		"call",
/* O_SEARCH */		2,	null,		"search",
/* O_SWITCH */          1,      null,          "switch",
/* O_SET */		2,	null,		"set",
/* O_UNSET */		1,	null,		"unset",
/* O_UNALIAS */		1,	null,		"unalias",
/* O_WINS */		1,	UNARY,		"@wins",
/* O_WINI */		1,	UNARY,		"@wini",
/* O_SCREEN */          0,      null,           "screen",
/* O_MULTPROC */        1,      null,           "multproc",
/* O_XCALL */           2,      null,           "xcall",
/* O_DETACH */          1,      null,           "detach"
};
