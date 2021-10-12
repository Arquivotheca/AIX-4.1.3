/* @(#)81	1.14.1.4  src/bos/usr/ccs/lib/libdbx/envdefs.h, libdbx, bos411, 9428A410j 2/11/94 14:30:27 */
#ifndef _h_envdefs
#define _h_envdefs
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) msgend, errend
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
/*
 * Public definitions, for programs using dbx procedural interface.
 */

#include "decode.h"
extern unsigned int action_mask;        /* tracks actions that dbx   */
                                        /* has performed for a given */
                                        /* command.                  */
 
#define	EXECUTION 		0x0001
#define BREAKPOINT 		0x0002
#define	DETACHED		0x0004
#define TRACE_ON		0x0008
#define	CONTEXT_CHANGE		0x0010
#define	LISTING			0x0020
#define	EXECUTION_COMPLETED	0x0040
#define	DBX_TERMINATED		0x0080
#define ASSIGNMENT              0x0100
#define	ELISTING		0x0200	/* extended listing bit		*/
#define	LOADCALL		0x0400
#define	CONFIGURATION		0x0800
#define EXECCALL		0x1000  /* program called exec		*/
#define LOADED			0x2000	/* program is loaded		*/
#define FUNC_CHANGE		0x4000	/* func command			*/
#define THREAD_CHANGE           0x8000  /* thread hold/unhold           */

#define ENVFATAL	-3
#define ENVQUIT		-2
#define	ENVEXIT 	-1
#define	ENVCONT 	 1
    
#define GLOBAL           0
#define LOCAL            1
    
#define msgbegin	\
	msgptr = nil; \
	rpt_save = rpt_output; \
	rpt_output = rpt_message;
#define msgend( string ) \
	string = msgptr; \
	msgptr = nil; \
	rpt_output = rpt_save;
/*
 * the msgptr is at least BUFSIZ bytes.  If you are getting alot of
 * small things that you intend to keep around a long time, using this
 * msgcopyend will make a copy in a smaller buffer for you.
 */
#define msgcopyend( string ) \
	string = malloc(strlen(msgptr)+1); \
	strcpy(string, msgptr);\
	free(msgptr);\
	msgptr = nil;\
	rpt_output = rpt_save;


#define errbegin	\
	msgptr = nil; \
	rpt_save = rpt_error; \
	rpt_error = rpt_message;
#define errend( string ) \
	string = msgptr; \
	msgptr = nil; \
	rpt_error = rpt_save;

extern	char	*msgptr;

typedef enum { VARIABLE = 1, FUNCTION = 2, UNKNOWN = 8 } SymbolType;
typedef enum { BREAKPOINT_SET, BREAKPOINT_DELETED } BkptType;
typedef enum { ARRAY_MODE, PTR_MODE } OffsetType;
typedef enum { NOFORMAT, OCTINTS, HEXINTS,
	       HEXCHARS, HEXSTRINGS, HEX } FormatMode;
typedef enum { V_TYPE, V_RECORD, V_SCAL, V_TAG, V_UNION, V_ARRAY, V_PTR,
               V_FFUNC, V_GROUP, V_RGROUP, V_MOD_ARRAY,
               V_INLINE, V_EXPRESSION, V_CONSTANT, V_UNDEFINED, V_CLASS } 
               VarClass;
typedef enum { C_LANG, FORTRAN_LANG, PASCAL_LANG, COBOL_LANG,
                UNSUPPORTED_LANG, CPLUSPLUS_LANG } VarLanguage;
typedef  enum { PROGRAM_EXTERNALS, FILE_STATICS,
                LOCAL_AND_NESTED, ALL_VISIBLE, UNKNOWN_LIST } VariableListType;
typedef  enum { EVENT_BREAK=0x1, EVENT_TRACE=0x2, EVENT_COND=0x4 } EventType;

typedef struct  variablelistT {
     char     *name;
     Symbol   key;
} VariableList;

typedef struct ExecStruct *ExecStruct;
typedef struct LoadStruct *LoadStruct;
typedef struct BkptStruct *BkptStruct;
typedef struct TrbkStruct *TrbkStruct;
typedef struct TraceStruct *TraceStruct;
typedef struct Range Range;


struct ExecStruct {
    char *file;
    char *function;
    int line;
    unsigned int address;
};

struct LoadStruct {
    char *file;
    unsigned int address;
    unsigned int memory;
};

struct BkptStruct {
    char *file;
    char *variable;
    int line;
    unsigned int address;
    BkptType type;
    int eventnum;
    EventType event_type;
};

struct TrbkStruct {
    char *function;
    int line;
    char *file;
} ;

struct TraceStruct {
    char *file;
    char *function;
    int line;
    unsigned int address;
    int eventnum;
    char *token;
    char *value;
    int reporting_trace_output;	/* set true for the call to	*/
				/* dpi_report_trace() to indi-	*/
				/* cate that data from a trace	*/
				/* event is about to be printed	*/
				/* and set to false for the	*/
				/* call to dpi_report_trace()	*/
				/* after the trace output has	*/
				/* been printed.		*/
} ;

struct VarStruct {
    Symbol key;
    char *name;
    char *basename;
    char *decl;
    int  index;
    VarClass class;
    int  format;
    int  inline;
    struct Range **range;
    struct VarStruct *implode;
    struct VarStruct *parent;
    struct VarStruct **chain;
    Symbol symbol;
    char *scope;
    int  decl_lines;
} ;

struct Range {
    int lower;
    int upper;
    int base;
} ;

struct DisplayLine {
        char    *line;
        int     size;
};

unsigned int genRegs[ NGREG + NSYS ];
double floatRegs[ 32 ];
#endif /* _h_envdefs */
