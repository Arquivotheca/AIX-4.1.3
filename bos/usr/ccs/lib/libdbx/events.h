/* @(#)43	1.5  src/bos/usr/ccs/lib/libdbx/events.h, libdbx, bos411, 9428A410j 8/10/93 20:08:47 */
#ifndef _h_events
#define _h_events
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: (macros) addevent, event_once
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

struct Event {
    unsigned int id;
    boolean temporary;
    Node condition;
    Cmdlist actions;
    Operator op;
    Node exp;
    Node cond;
};

typedef struct Event *Event;

struct Breakpoint {
    Event event;
    Address bpaddr;	
    Lineno bpline;
    Cmdlist actions;
    boolean temporary;
};

typedef struct Breakpoint *Breakpoint;
 
#include "symbols.h"

#define addevent(cond, cmdlist) event_alloc(false, cond, cmdlist)
#define event_once(cond, cmdlist) event_alloc(true, cond, cmdlist)

/*
 * When tracing variables we keep a copy of their most recent value
 * and compare it to the current one each time a breakpoint occurs.
 * MAXTRSIZE is the maximum size variable we allow.
 */

#define MAXTRSIZE 512

extern boolean inst_tracing;
extern boolean single_stepping;
extern boolean isstopped;
extern Symbol linesym;
extern Symbol procsym;
extern Symbol pcsym;
extern Symbol retaddrsym;
extern bpinit(/*  */);
extern Event event_alloc(/* istmp, econd, cmdlist */);
extern boolean delevent (/* id */);
extern Symbol tcontainer(/* exp */);
extern boolean canskip(/* f */);
extern status(/*  */);
extern printevent(/* e */);
extern bpfree(/*  */);
extern boolean bpact(/*  */);
extern traceon(/* inst, event, cmdlist */);
extern traceoff(/* id */);
extern printnews(/*  */);
extern callnews(/* iscall */);
extern printifchanged(/* p */);
extern stopifchanged(/* p */);
extern trfree(/*  */);
extern fixbps(/*  */);
extern setallbps(/*  */);
extern unsetallbps(/*  */);
#endif /* _h_events */
