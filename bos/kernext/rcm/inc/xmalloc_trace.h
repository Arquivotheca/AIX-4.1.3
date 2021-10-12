/* @(#)44	1.2.2.1  src/bos/kernext/rcm/inc/xmalloc_trace.h, rcm, bos41J, 9520A_all 5/3/95 13:58:38 */

/*
 * COMPONENT_NAME: (rcm) AIX Rendering Context Manager structure definitions
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989-1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  Define traps for memory handling.
 *
 *  The pin/unpin tracing won't balance if the buffer crosses
 *  segment boundaries.
 */

#ifdef RCMDEBUG

void  *  xmalloc_trace (int, int, caddr_t, char *, int);
int      xmfree_trace (caddr_t, caddr_t, char *, int);
int      xmalloc_trace_report (int);

struct trb  *talloc_trace (char *, int);
void     tfree_trace (struct trb *, char *, int);
int      talloc_trace_report (int);

int      pinu_trace (caddr_t, int, short, char *, int);
int      unpinu_trace (caddr_t, int, short, char *, int);
int      pinu_trace_report (int);

void     d_master_trace (int, int, caddr_t, size_t, struct xmem *, caddr_t, char *, int);
int      d_complete_trace (int, int, caddr_t, size_t, struct xmem *, caddr_t, char *, int);
int      d_master_trace_report (int);

int      xmattach_trace (char *, int, struct xmem *, int, char *, int);
int      xmdetach_trace (struct xmem *, char *, int);
int      xmattach_trace_report (int);

#define xmalloc(A,B,C)	xmalloc_trace (A, B, C, __FILE__, __LINE__)
#define xmfree(A,B)	xmfree_trace  (A, B,    __FILE__, __LINE__)

#define talloc()	talloc_trace (__FILE__, __LINE__)
#define tfree(A)	tfree_trace  (A, __FILE__, __LINE__)

#define pinu(A,B,C)	pinu_trace    (A, B,  C, __FILE__, __LINE__)
#define pin(A,B)	pinu_trace    (A, B, -1, __FILE__, __LINE__)

#define unpinu(A,B,C)	unpinu_trace  (A, B,  C, __FILE__, __LINE__)
#define unpin(A,B)	unpinu_trace  (A, B, -1, __FILE__, __LINE__)

#define d_master(A,B,C,D,E,F)		\
		d_master_trace (A, B, C, D, E, F, __FILE__, __LINE__)
#define d_complete(A,B,C,D,E,F)		\
		d_complete_trace (A, B, C, D, E, F, __FILE__, __LINE__)

#define xmattach(A,B,C,D)		\
		xmattach_trace (A, B, C, D, __FILE__, __LINE__)
#define xmdetach(A)			\
		xmdetach_trace (A, __FILE__, __LINE__)

#endif
