/* @(#)39       1.7.1.7  src/bos/usr/ccs/lib/libdbx/defs.h, libdbx, bos411, 9434B411a 8/20/94 16:07:07 */
#ifndef _h_defs
#define _h_defs
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) assert, badcaseval, checkref, dispose, get, max
 * 	       min, new, newarr, ord, put, strdup, streq
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

/*
 * Public definitions, common to all.
 */

#include <stdio.h>
#include <string.h>

#ifdef sgi
#   define double long float
#   define atof _latof
#   define IRIS
#   define mc68000
#endif

#define ibm032

#define new(type)           ((type) malloc(sizeof(struct type)))
#define newarr(type, n)     ((type *) malloc((unsigned) (n) * sizeof(type)))
#define dispose(ptr) { \
	if (ptr) { \
		free((void *) ptr); \
		ptr = 0; \
	} \
}
#define public
#define private static

#define ord(enumcon) ((unsigned int) enumcon)
#define nil 0
#define and &&
#define or ||
#define not !
#define mod %
#define max(a, b)    ((a) > (b) ? (a) : (b))
#define min(a, b)    ((a) < (b) ? (a) : (b))

#define assert(b) { \
    if (not(b)) { \
	panic("assertion failed at line %d in file %s", __LINE__, __FILE__); \
    } \
}

#define badcaseval(v) { \
    panic("unexpected value %d at line %d in file %s", v, __LINE__, __FILE__); \
}

#define checkref(p) { \
    if (p == nil) { \
	panic("reference through nil pointer at line %d in file %s", \
	    __LINE__, __FILE__); \
    } \
}

typedef int Integer;
typedef int integer;
typedef char Char;
typedef double Real;
typedef double real;
typedef struct { double val[2]; } quadf;

#ifdef LONGLONGTYPE
typedef long long LongLong;
typedef unsigned long long uLongLong;
#define sizeofLongLong sizeof(long long)
#else
typedef long LongLong;
typedef unsigned long uLongLong;
#define sizeofLongLong 2*sizeof(long)
#define strtoll strtol
#define strtoull strtoul
#endif

typedef enum { false, true } Boolean;
typedef enum { filedep, lower, upper, mixed } cases;
typedef enum { off = -1, on = 1, parent, child } fork_type;
typedef Boolean boolean;
typedef char *String;

#define strdup(s)       strcpy(malloc((unsigned) strlen(s) + 1), s)
#define streq(s1, s2)   (strcmp(s1, s2) == 0)

typedef FILE *File;
typedef int Fileid;
typedef String Filename;

#define get(f, var) fread((char *) &(var), sizeof(var), 1, f)
#define put(f, var) fwrite((char *) &(var), sizeof(var), 1, f)

#undef FILE

extern long atol();
extern double atof();

extern String cmdname;
extern String prompt;
extern String errfilename;
extern short errlineno;
extern int debug_flag[];

#ifndef _NO_PROTO
extern	int		(*rpt_output)( FILE *, const char *, ...);
extern	int		(*rpt_error)( FILE *, const char *, ...);
extern	int		(*rpt_message)( FILE *, const char *, ...);
extern	int		(*rpt_save)( FILE *, const char *, ...);
extern	int		(*rpt_executing)( );
extern	int		(*rpt_shell)( );
extern	int		(*rpt_trace)( );
extern	int		(*rpt_multiprocess)( int );
extern	int		(*rpt_ctx_level)( int );
extern	int		(*rpt_open)( );
extern  int		dpi_report_executing();
extern  int		dpi_report_shell();
extern  int		dpi_report_trace( );
extern  int		dpi_report_multiprocess( int );
extern  int		dpi_ctx_level( int );
extern  int		xde_open_windows();
#else
extern	int		(*rpt_output)( );
extern	int		(*rpt_error)( );
extern	int		(*rpt_executing)( );
extern	int		(*rpt_message)( );
extern	int		(*rpt_shell)( );
extern	int		(*rpt_trace)( );
extern	int		(*rpt_multiprocess)( );
extern	int		(*rpt_ctx_level)( );
extern	int		(*rpt_open)( );
extern	int		(*rpt_save)( );
extern  int		dpi_report_executing();
extern  int		dpi_report_shell();
extern  int		dpi_report_trace();
extern  int		dpi_report_multiprocess();
extern  int		dpi_ctx_level();
extern  int		xde_open_windows();
#endif
#endif /* _h_defs */
