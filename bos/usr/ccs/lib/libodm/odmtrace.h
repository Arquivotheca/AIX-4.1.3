/* @(#)95	1.6  src/bos/usr/ccs/lib/libodm/odmtrace.h, libodm, bos411, 9428A410j 6/16/90 02:19:01 */
/*
 * COMPONENT_NAME: LIBODM odmi.h
 *
 * ORIGIN: IBM
 *
 * Copyright International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */


/* ifdef of size_t so that odm files will compile in build environment. */

#ifdef R5A
typedef unsigned long    size_t;
#endif

/* Include file to turn on the trace output */
extern int odmtrace;
extern int trace_indent;
#ifdef DEBUG
#define TRC(a,b,c,d,e) if(odmtrace)  print_odm_trace(a,b,c,d,e);
#define MALLOC_CHECK; if(odmtrace) {free(malloc(100)); free(malloc(10000));}
#else
#define TRC(a,b,c,d,e)
#define MALLOC_CHECK;
#endif



#ifdef PERF
#include <sys/trcctl.h>   \
#include <sys/trcmacros.h> \
#include <sys/trchkid.h> \
#define START_ROUTINE(hookid) \
int subhook = hookid; \
odmerrno = 0; trace_indent++;\
TRCHKL1T(HKWD_LIB_ODM|subhook, 0)
#else
#define START_ROUTINE(hookid) \
odmerrno = 0; trace_indent++
#endif

#ifdef PERF
#define STOP_ROUTINE  trace_indent--; TRCHKL1T(HKWD_LIB_ODM|subhook, 1)
#else
#define STOP_ROUTINE  trace_indent--
#endif

