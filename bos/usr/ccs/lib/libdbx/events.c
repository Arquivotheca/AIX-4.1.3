static char sccsid[] = "@(#)42	1.12.2.12  src/bos/usr/ccs/lib/libdbx/events.c, libdbx, bos411, 9428A410j 5/11/94 14:22:01";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: bp_alloc, bpact,
 *	      bpfree, bpinit, bplist_append, builtinsym, callnews, canskip,
 *	      chklist, clearbps, clearbps_i, condbp, deleteall, delevent,
 *	      event_alloc, eventlist_append, findtrinfo, fixbps, printcond,
 *	      printevent, printeventid, printifchanged, printnews, printrmtr,
 *	      setallbps, status, stopifchanged, tcontainer, traceoff, traceon,
 *	      translate, trfree, unsetallbps
 *
 * ORIGINS: 26, 27, 83
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
 * LEVEL 1,  5 Years Bull Confidential Information
*/
#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * Event/breakpoint managment.
 */

#include "defs.h"
#include "envdefs.h"
#include "events.h"
#include "main.h"
#include "symbols.h"
#include "tree.h"
#include "eval.h"
#include "source.h"
#include "mappings.h"
#include "runtime.h"
#include "process.h"
#include "machine.h"
#include "lists.h"

public boolean inst_tracing;
public boolean single_stepping;
public boolean isstopped;
public boolean hexaddr = false;

public Symbol linesym;
public Symbol procsym;
public Symbol pcsym;
public Symbol retaddrsym;
public integer eventId;
extern Boolean indirect;
extern char		*bpset;
extern char		*bpdeleted;

struct Watch {
    unsigned int id;		/* Event associated with this watch cmd. */
    Node exp;			/* Expression to watch */
};

typedef List Eventlist;
typedef List Bplist;
typedef List Wlist;
typedef struct Watch *Watch;

#define eventlist_append(event, el) list_append(list_item(event), nil, el)
#define bplist_append(bp, bl) list_append(list_item(bp), nil, bl)

private Eventlist eventlist;		/* list of active events */
public  Bplist bplist;			/* list of active breakpoints */
private Wlist wlist;			/* list of active watches */
private Event curevent;			/* most recently created event */
private integer eventid = 0;		/* id number of current event */
private integer trid = 0;		/* id number of current trace */
private integer curtrace = 0;		/* event # of current trace */

typedef struct Trcmd {
    Integer trid;
    Event event;
    Cmdlist cmdlist;
} *Trcmd;

private List eachline;		/* commands to execute after each line */
private List eachinst;		/* cmds to execute after each instruction */

private Breakpoint bp_alloc();
private Breakpoint condbp();
private translate();
private printeventid();
private printcond();
private printrmtr();

/*
 * Initialize breakpoint information.
 */

private Symbol builtinsym(str, class, type)
String str;
Symclass class;
Symbol type;
{
    Symbol s;

    s = insert(identname(str, true));
    s->language = findlanguage(".s");
    s->class = class;
    s->type = type;
    return s;
}

public bpinit()
{
    linesym = builtinsym("$line", VAR, t_int);
    procsym = builtinsym("$proc", PROC, nil);
    pcsym = lookup(identname("$pc", true));
    if (pcsym == nil) {
	panic( catgets(scmc_catd, MS_events, MSG_78,
			      "cannot find symbol for program counter ($pc) "));
    }
    retaddrsym = builtinsym("$retaddr", VAR, t_int);
    eventlist = list_alloc();
    bplist = list_alloc();
    wlist = list_alloc();
    eachline = list_alloc();
    eachinst = list_alloc();
}

/*
 * Trap an event and do the associated commands when it occurs.
 */

public Event event_alloc(istmp, econd, cmdlist)
boolean istmp;
Node econd;
Cmdlist cmdlist;
{
    register Event e;

    e = new(Event);
    ++eventid;
    e->id = eventid;
    e->temporary = istmp;
    e->condition = econd;
    e->actions = cmdlist;
    e->exp = NULL;
    e->cond = NULL;
    eventlist_append(e, eventlist);
    curevent = e;
    translate(e);
    return e;
}

/*
 * Delete the event with the given id.
 * Returns whether it's successful or not.
 */

public boolean delevent (id)
unsigned int id;
{
    Event e;
    Breakpoint bp;
    ListItem	cur_item;
    Trcmd t;
    boolean found;

    found = false;
    foreach (Trcmd, t, eachline)
	if (t->event->id == id) {
	    found = true;
	    printrmtr(t);
	    cur_item = list_curitem(eachline);
	    dispose( cur_item->element );
	    list_delete(cur_item, eachline);
	}
    endfor
    foreach (Trcmd, t, eachinst)
	if (t->event->id == id) {
	    found = true;
	    printrmtr(t);
	    cur_item = list_curitem(eachinst);
	    dispose( cur_item->element );
	    list_delete(cur_item, eachinst);
	}
    endfor
    foreach (Event, e, eventlist)
	if (e->id == id) {
	    found = true;
	    foreach (Breakpoint, bp, bplist)
		if (bp->event == e) {
		    if (tracebpts) {
			(*rpt_output)
			  (stdout, "deleting breakpoint at 0x%x\n", bp->bpaddr);
		    }

		    if ((not bp->temporary) && (not e->temporary))
			eventHistory_alloc( bp, BREAKPOINT_DELETED );
		    /*
		     * Free Node parts of this event
		     */
		    tfree(e->condition);
		    tfree(e->exp);
		    tfree(e->cond);
		    e->condition = e->exp = e->cond = NULL;
		    cur_item = list_curitem(bplist);
		    dispose( cur_item->element );
		    list_delete(cur_item, bplist);
		}
	    endfor
	    cur_item = list_curitem(eventlist);
	    dispose( cur_item->element );
	    list_delete(cur_item, eventlist);
	    break;
	}
    endfor
    if (list_size(eachinst) == 0) {
	inst_tracing = false;
	if (list_size(eachline) == 0) {
	    single_stepping = false;
	}
    }
    return found;
}

/*
 * Translate an event into the appropriate breakpoints and actions.
 * While we're at it, turn on the breakpoints if the condition is true.
 */

private translate(e)
Event e;
{
    Symbol s;
    Node place;
    Lineno line;
    Breakpoint bp;
    cppSymList address_list;
    cppSymList temp_address_list;

    checkref(e->condition);
    switch (e->condition->op) {
	case O_EQ:
	    if (e->condition->value.arg[0]->op == O_SYM) {
		s = e->condition->value.arg[0]->value.sym;
		place = e->condition->value.arg[1];
		if (s == linesym) {
		    if (place->op == O_QLINE) {
			line = place->value.arg[1]->value.lcon;
                        address_list = get_address_list (line,
                              place->value.arg[0]->value.scon, e->id);
		    } else {
			eval(place);
			line = pop(long);
			address_list = get_address_list (line,
                              cursource, e->id);
		    }
		    if (address_list == NULL) {
			if (!delevent(e->id)) {
	                  panic (catgets(scmc_catd, MS_eval, MSG_107,
			      "unknown event %ld"), e->id);
			}
			beginerrmsg();
			(*rpt_error)(stderr,  catgets(scmc_catd, MS_events,
				      MSG_101, "no executable code at line "));
			prtree( rpt_error, stderr, place);
			enderrmsg();
		    }
                    while (address_list != NULL)
                    {
                      bp = bp_alloc(e, address_list->line_addr, 
                                    line, e->actions);
                      eventHistory_alloc (bp, BREAKPOINT_SET);
                      temp_address_list = address_list;
                      address_list = address_list->chain;
                      free(temp_address_list);
                    }
                    return;
		} else if (s == procsym) {
		    eval(place);
		    s = pop(Symbol);
		    bp = bp_alloc(e, codeloc(s), 0, e->actions);
		    if ((pc != codeloc(program)) and isactive(s)) {
			evalcmdlist(e->actions, false);
		    }
		} else if (s == pcsym) {
		    eval(place);
		    bp = bp_alloc(e, pop(Address), 0, e->actions);
		} else {
		    bp = condbp(e);
		}
	    } else {
		bp = condbp(e);
	    }
	    break;

	/*
	 * These should be handled specially.
	 * But for now I'm ignoring the problem.
	 */
	case O_AND:
	case O_OR:
	default:
	    bp = condbp(e);
	    break;
    }
    eventHistory_alloc( bp, BREAKPOINT_SET );
}

/*
 * Create a breakpoint for a condition that cannot be pinpointed
 * to happening at a particular address, but one for which we
 * must single step and check the condition after each statement.
 */

private Breakpoint	condbp(e)
Event e;
{
    Breakpoint bp;
    Symbol p;
    Cmdlist actions;

    p = tcontainer(e->condition);
    if (p == nil) {
	p = program;
    }
    actions = buildcmdlist(build(O_IF, e->condition, e->actions));
    actions = buildcmdlist(build(O_TRACEON, false, actions));
    bp = bp_alloc(e, codeloc(p), 0, actions);
    return bp;
}

/*
 * Determine the deepest nested subprogram that still contains
 * all elements in the given expression.
 */

public Symbol tcontainer(exp)
Node exp;
{
    Integer i;
    Symbol s, t, u, v;
    short	numb_args;

    checkref(exp);
    s = nil;
    if (exp->op == O_SYM) {
	if (exp->value.sym->class != MEMBER)
	    s = container(exp->value.sym);
    } else if (not isleaf(exp->op)) {
	numb_args = nargs(exp->op);
	for (i = 0; i < numb_args; i++) {
	    t = tcontainer(exp->value.arg[i]);
	    if (t != nil) {
		if (s == nil) {
		    s = t;
		} else {
		    u = s;
		    v = t;
		    while (u != v and u != nil) {
			u = container(u);
			v = container(v);
		    }
		    if (u == nil) {
			panic( catgets(scmc_catd, MS_events, MSG_102,
				       "bad ancestry for \"%s\""), symname(s));
		    } else {
			s = u;
		    }
		}
	    }
	}
    }
    return s;
}

/*
 * Determine if the given function can be executed at full speed.
 * This can only be done if there are no breakpoints within the function.
 */

public boolean canskip(f)
Symbol f;
{
    Breakpoint p;
    boolean ok;

    ok = true;
    foreach (Breakpoint, p, bplist)
	if (whatblock(p->bpaddr) == f) {
	    ok = false;
	    break;
	}
    endfor
    return ok;
}

/*
 * Print out what's currently being traced by looking at
 * the currently active events.
 *
 * Some convolution here to translate internal representation
 * of events back into something more palatable.
 */

public status()
{
    Event e;

    foreach (Event, e, eventlist)
	if (not e->temporary) {
	    printevent(e);
	}
    endfor
}

/*
 *  Delete all of the breakpoints.
 */

public boolean deleteall()
{
    Event e;
    boolean found;

    found = false;
    foreach (Event, e, eventlist)
	if (not e->temporary) {
	    if ( delevent(e->id))
		found = true;
	}
    endfor
    return found;
}

/*
 *  Delete all of the breakpoints at a given line.
 *  Must get ALL of the breakpoints for that line.
 *  We calculate the address of the line and the line 
 *  following that one and delete all bp's between.
 *  We also make sure all bp's that are deleted
 *  are in the same block..for safety's sake.
 */

public boolean clearbps (place)
Node place;
{
    Breakpoint bp;
    Lineno line1;
    Address addr1, addr2;
    Symbol bp_block, clear_block;
    boolean found;
    cppSymList address_list, temp_address_list;

  found = false;
  line1 = place->value.arg[1]->value.lcon;
  address_list = get_address_list(line1,
                                  place->value.arg[0]->value.scon, 0);

  if (address_list == NULL) {
	beginerrmsg();
	(*rpt_error)(stderr,  catgets(scmc_catd, MS_events,
				      MSG_101, "no executable code at line "));
	prtree( rpt_error, stderr, place);
	enderrmsg();
	return false;
  }
  while (address_list != NULL)
  {
    addr1 = address_list->line_addr;
    addr2 = nextline(addr1+1);
    /* Mark last line in source file */
    if (addr2 == 0) 
       addr2 = 0xffffffff;
    clear_block = whatblock(addr1);

    foreach (Breakpoint, bp, bplist)
	if (not bp->temporary) 
	{
	    bp_block = whatblock(bp->bpaddr);
	    if ((bp->bpaddr >= addr1) && (bp->bpaddr < addr2) &&
	                                             (bp_block == clear_block))
	         if ( delevent(bp->event->id))
		    found = true;
	}
    endfor
    temp_address_list = address_list;
    address_list = address_list->chain;
    free(temp_address_list);
  }

    if (!found) {
	beginerrmsg();
	(*rpt_error)(stderr,  catgets(scmc_catd, MS_events,
				   MSG_542, "no breakpoints found at line "));
	prtree( rpt_error, stderr, place);
	enderrmsg();
    }
    return found;
}

/*
 *  Delete all of the breakpoints at a given address.
 */

public boolean clearbps_i (place)
Node place;
{
    Breakpoint bp;
    Address addr;
    boolean found;

    found = false;
    eval(place->value.arg[0]);
    addr = pop(long);
    foreach (Breakpoint, bp, bplist)
	if (not bp->temporary) {
	    if (bp->bpaddr == addr)
	         if (delevent(bp->event->id))
		    found = true;
	}
    endfor
    if (!found) {
	beginerrmsg();
	(*rpt_error)(stderr,  catgets(scmc_catd, MS_events,
				MSG_543, "no breakpoints found at address "));
	(*rpt_error)(stderr,  "0x%x", addr);
	enderrmsg();
    }
    return found;
}

/*
 *  Delete all of the watches of a given variable.
 */
/*  This code deleted until further notice.  Use delete command instead.

public clearwatch (varnode)
Node varnode;
{
    Symbol varblock, watchblock;
    Symbol vartype, watchtype;
    Boolean found = false;
    Watch w;
    ListItem	cur_item;

    varblock = tcontainer(varnode);
    vartype = varnode->nodetype;
    foreach (Watch, w, wlist)
	watchblock = tcontainer(d->exp);
	watchtype = w->exp->nodetype;
	if ((varblock == watchblock) && (vartype == watchtype)) {
	    delevent(w->id);
	    found = true;
	    cur_item = list_curitem(wlist);
	    dispose( cur_item->element );
	    list_delete(cur_item, wlist);
	}
    endfor
    if (!found)
	error("Not currently watched");
}
*/

public printevent(e)
Event e;
{
  Command cmd;
  Node exp = e->exp;
  Node cond = e->cond;

  if (not isredirected()) 
  {
    printeventid(e->id);
  }

  cmd = list_element(Command, list_head(e->actions));

  if (cmd->op == O_PRINTCALL)
  {
    (*rpt_output)(stdout, "trace ");
    printname (rpt_output, stdout, cmd->value.sym, true);
  }
  else
  {
    /*  print the string form of "op"  */ 
    (*rpt_output)(stdout, "%s", opinfo[ord(e->op)].opstring);

    /*  if there is an "expression" to print  */
    if (exp != NULL) 
    {
      /*  print it  */
      (*rpt_output)(stdout, " ");
      prtree (rpt_output, stdout, exp);
    }

    /*  print the "location"  */
    printcond(e->condition);

    /*  if an "if" condition exists  */
    if (cond != NULL)
    {
      /*  print the "if" condition  */
      (*rpt_output)(stdout, " ");
      (*rpt_output)(stdout, "if ");
      prtree(rpt_output, stdout, cond);
    }
  }
  (*rpt_output)(stdout, "\n");
}

private printeventid (id)
integer id;
{
    (*rpt_output)(stdout, "[%d] ", id);
}

/*
 * Print out a condition.
 */

private printcond(cond)
Node cond;
{
    Symbol s;
    Node place;

    if (cond->op == O_EQ and cond->value.arg[0]->op == O_SYM) {
	s = cond->value.arg[0]->value.sym;
	place = cond->value.arg[1];
	if (s == procsym) {
	    if (place->value.sym != program) {
		(*rpt_output)(stdout, " in ");
		if (isinline(place->value.sym))
		    (*rpt_output)(stdout, "unnamed block ");
		printname( rpt_output, stdout, place->value.sym, true);
	    }
	} else if (s == linesym) {
	    (*rpt_output)(stdout, " at ");
	    prtree( rpt_output, stdout, place);
	} else if (s == pcsym or s == retaddrsym) {
	    (*rpt_output)(stdout, " at ");
	    hexaddr = true;
	    eval(place);
   	    addrprint( rpt_output, stdout, pop(long), false, 0);
	    hexaddr = false;
	} else {
	    (*rpt_output)(stdout, " when ");
	    prtree( rpt_output, stdout, cond);
	}
    } else {
	(*rpt_output)(stdout, " when ");
	prtree( rpt_output, stdout, cond);
    }
}

/*
 * Add a breakpoint to the list and return it.
 */

private Breakpoint bp_alloc(e, addr, line, actions)
Event e;
Address addr;
Lineno line;
Cmdlist actions;
{
    register Breakpoint p;

    p = new(Breakpoint);
    p->event = e;
    p->bpaddr = addr;
    p->bpline = line;
    p->actions = actions;
    p->temporary = false;
    if (tracebpts) {
	if (e == nil) {
	    (*rpt_output)(stdout,"new bp at 0x%x for event ??\n", addr, e->id);
	} else {
	    (*rpt_output)(stdout,"new bp at 0x%x for event %d\n", addr, e->id);
	}
    }
    bplist_append(p, bplist);
    return p;
}

/*
 * Free all storage in the event and breakpoint tables.
 */

public bpfree()
{
    register Event e;
    ListItem	cur_item;

    fixbps();
    foreach (Event, e, eventlist)
	if (not delevent(e->id)) {
	    (*rpt_output)(stdout, 
			      "!! dbx.bpfree: cannot delete event %d\n", e->id);
	}
    endfor
}

/*
 * Determine if the program stopped at a known breakpoint
 * and if so do the associated commands.
 */

extern long bp_skips;
integer eventIdLastBp;

#ifdef K_THREADS
extern Address addr_dbsubn;
#endif /* K_THREADS  */
public boolean bpact()
{
    register Breakpoint p;
    boolean found;
    integer eventId;
    ListItem	cur_item;

    found = false;
    eventIdLastBp = -1;
    foreach (Breakpoint, p, bplist)
#ifdef K_THREADS
        /* the break-point addr_dbsubn will never be reached : the running  */
        /* is looping : so we have to restore its registers                 */
        /* if (p->bpaddr == pc) { */
        if (p->bpaddr == pc || p->bpaddr == addr_dbsubn) {
#else
        if (p->bpaddr == pc) {
#endif /* K_THREADS  */
	    if (bp_skips-- > 0)   /* if we are doing a skip command, return */
	    {                     /* true...we have found a bp but we don't */
	       found = true;      /* want to do anything with it. Decrement */
	    }                     /* our breakpoint counter.                */
	    else
	    {
	       if (tracebpts) {
		   (*rpt_output)
		   (stdout, "breakpoint for event %d found at location 0x%x\n",
		       p->event->id, pc);
	       }
	       found = true;
	       if (!p->event->temporary && !p->temporary)
		   eventIdLastBp = p->event->id;
	       if (p->event->temporary) {
		   if (not delevent(p->event->id)) {
		       (*rpt_output)(stdout,
		          "!! dbx.bpact: cannot find event %d\n", p->event->id);
		   }
	       }
	       evalcmdlist(p->actions, false);
	       if (isstopped) {
		   eventId = p->event->id;
	       }
	       if (p->temporary) {
		   cur_item = list_curitem(bplist);
		   dispose( cur_item->element );
		   list_delete(cur_item, bplist);
	       }
	   }
	}
    endfor
    if (isstopped) {
	if (found) {
          printeventid(eventId);
	}
	printstatus();
    }
    fflush(stdout);
    return found;
}

/*
 * Begin single stepping and executing the given commands after each step.
 * If the first argument is true step by instructions, otherwise
 * step by source lines.
 *
 * We automatically set a breakpoint at the end of the current procedure
 * to turn off the given tracing.
 */

public traceon(inst, event, cmdlist)
boolean inst;
Event event;
Cmdlist cmdlist;
{
    register Trcmd trcmd;
    Breakpoint bp;
    Cmdlist actions;
    Address ret;
    Event e;

    if (event == nil) {
	e = curevent;
    } else {
	e = event;
    }
    trcmd = new(Trcmd);
    ++trid;
    trcmd->trid = trid;
    trcmd->event = e;
    trcmd->cmdlist = cmdlist;
    single_stepping = true;
    if (inst) {
	inst_tracing = true;
	list_append(list_item(trcmd), nil, eachinst);
    } else {
	list_append(list_item(trcmd), nil, eachline);
    }
    ret = return_addr();
    /* Should not do traceoff at end of procedure if */
    /* the trace is for the whole program.           */
    if ( ret && (e->condition->value.arg[0]->op == O_SYM) &&
                (e->condition->value.arg[0]->value.sym == procsym)) {
       Symbol s;
       eval(e->condition->value.arg[1]);        /* get the place */
       s = pop(Symbol);
       if (s == program)
          ret = 0;
    }
    if (ret != 0) {
	actions = buildcmdlist(build(O_TRACEOFF, trcmd->trid));
	bp = bp_alloc(e, (Address) ret, 0, actions);
	bp->temporary = true;
    }
    if (tracebpts) {
	(*rpt_output)(stdout, "adding trace %d for event %d\n",
							   trcmd->trid, e->id);
    }
}

/*
 * Turn off some kind of tracing.
 * Strictly an internal command, this cannot be invoked by the user.
 */

public traceoff(id)
Integer id;
{
    register Trcmd t;
    register boolean found;
    ListItem	cur_item;

    found = false;
    foreach (Trcmd, t, eachline)
	if (t->trid == id) {
	    printrmtr(t);
	    cur_item = list_curitem(eachline);
	    dispose( cur_item->element );
	    list_delete(cur_item, eachline);
	    found = true;
	    break;
	}
    endfor
    if (not found) {
	foreach (Trcmd, t, eachinst)
	    if (t->trid == id) {
		printrmtr(t);
		cur_item = list_curitem(eachinst);
		dispose( cur_item->element );
		list_delete(cur_item, eachinst);
		found = true;
		break;
	    }
	endfor
    }
    if (list_size(eachinst) == 0) {
	inst_tracing = false;
	if (list_size(eachline) == 0) {
	    single_stepping = false;
	}
    }
}

/*
 * If breakpoints are being traced, note that a Trcmd is being deleted.
 */

private printrmtr(t)
Trcmd t;
{
    if (tracebpts) {
	(*rpt_output)(stdout, "removing trace %d", t->trid);
	if (t->event != nil) {
	    (*rpt_output)(stdout, " for event %d", t->event->id);
	}
	(*rpt_output)(stdout, "\n");
    }
}

/*
 * NAME: printnews
 *
 * FUNCTION: print out news during single step tracing.
 *
 * PARAMETERS: first_line - flag to determine if special handling
 *                          to print first line is needed.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: nothing
 */

public printnews(first_line)
Boolean first_line;
{
    register Trcmd t;

    foreach (Trcmd, t, eachline)
      curtrace = t->event->id;
      evalcmdlist(t->cmdlist, first_line);
    endfor

    foreach (Trcmd, t, eachinst)
      curtrace = t->event->id;
      evalcmdlist(t->cmdlist, first_line);
    endfor
    if (!first_line)
      bpact();
}

/*
 * A procedure call/return has occurred while single-stepping,
 * note it if we're tracing lines.
 */

private boolean chklist();

public callnews(iscall)
boolean iscall;
{
    if (not chklist(eachline, iscall)) {
	chklist(eachinst, iscall);
    }
}

private boolean chklist(list, iscall)
List list;
boolean iscall;
{
    register Trcmd t;
    register Command cmd;

    setcurfunc(whatblock(pc));
    foreach (Trcmd, t, list)
	foreach (Command, cmd, t->cmdlist)
	    if (cmd->op == O_PRINTSRCPOS and
	      (cmd->value.arg[0] == nil or cmd->value.arg[0]->op == O_QLINE)) {
		if (iscall) {
		    printentry(curfunc);
		} else {
		    printexit(curfunc);
		}
		return true;
	    }
	endfor
    endfor
    return false;
}

/*
 * List of variables being watched.
 */

typedef struct Trinfo *Trinfo;

struct Trinfo {
    Node variable;
    Address traddr;
    Symbol trblock;
    char *trvalue;
    Boolean inreg;
    Boolean indir;
};

private List trinfolist;

/*
 * Find the trace information record associated with the given record.
 * If there isn't one then create it and add it to the list.
 */

private Trinfo findtrinfo(p)
Node p;
{
    register Trinfo tp;
    boolean isnew;

    isnew = true;
    if (trinfolist == nil) {
	trinfolist = list_alloc();
    } else {
	foreach (Trinfo, tp, trinfolist)
	    if (tp->variable == p) {
		isnew = false;
		break;
	    }
	endfor
    }
    if (isnew) {
	if (tracebpts) {
	    (*rpt_output)(stdout, "adding trinfo for \"");
	    prtree( rpt_output, stdout, p);
	    (*rpt_output)(stdout, "\"\n");
	}
	tp = new(Trinfo);
	tp->variable = p;
	tp->traddr = lval(p);
	tp->trvalue = nil;
	tp->inreg = (Boolean) (p->nodetype->storage == INREG);
	tp->indir = indirect;
	indirect = false;
	list_append(list_item(tp), nil, trinfolist);
    } else if (tp->indir) {
	tp->traddr = lval(p);
	indirect = false;
    }
    return tp;
}

/*
 * Print out the value of a variable if it has changed since the
 * last time we checked.
 */

public printifchanged(p)
Node p;
{
    register Trinfo tp;
    register int n;
    char buff[MAXTRSIZE];
    int *regbuf;
    Filename curfile;
    static Lineno prevline;
    static Filename prevfile;
    struct TraceStruct trdata;
    Symbol tracesym = nil;

    /* Find variable being traced */
    if (p && p->op == O_RVAL) {
        tracesym = p->value.arg[0]->value.sym;
    } else if (p && p->op == O_SYM) {
        tracesym = p->value.sym;
    }
    /* If variable being traced is not active, just return! */
    if (isvariable(tracesym) && tracesym != program
                             && !isactive(container(tracesym))) {
       return;
    }
    tp = findtrinfo(p);
    n = size(p->nodetype);
    if (tp->inreg) {
       regbuf = (int *) buff; 
       *regbuf = reg(tp->traddr);
    } else {
       dread(buff, tp->traddr, n);
    }
    curfile = srcfilename(pc);
    if (tp->trvalue == nil) {
	tp->trvalue = newarr(char, n);
	mov(buff, tp->trvalue, n);
	mov(buff, sp, n);
	sp += n;
		/*
		 * Start of trace output.  Indicate by setting
		 * eventnum to nil.
		 */
		trdata.reporting_trace_output = 1;
		(*rpt_trace)( &trdata );

		/****************************+
		*  get data for trace report *
		+****************************/

		dpi_current_location( &trdata );
		trdata.eventnum = eventId;
	
		msgbegin;	
   		prtree( rpt_output, stdout, p);
		msgend( trdata.token );

		msgbegin;	
   		printval(p->nodetype, 0);
		msgend( trdata.value );
	
	   (*rpt_output)(stdout, "initially (at line %d in \"%s\"):\t",
                                 curline, basefile(curfile));
	   (*rpt_output)(stdout, "%s = %s\n", trdata.token, trdata.value );
		trdata.reporting_trace_output = 0;
		(*rpt_trace)( &trdata );
		dispose( trdata.token );
		dispose( trdata.value );

    } else if (cmp(tp->trvalue, buff, n) != 0) {
	mov(buff, tp->trvalue, n);
	mov(buff, sp, n);
	sp += n;
		/*
		 * Start of trace output.  Indicate by setting
		 * eventnum to nil.
		 */
		trdata.reporting_trace_output = 1;
		(*rpt_trace)( &trdata );

		/****************************+
		*  get data for trace report *
		+****************************/

		get_location( prevfile, prevline, &trdata );
		trdata.eventnum = eventId;
	
		msgbegin;
   		prtree( rpt_output, stdout, p);
		msgend( trdata.token );

		msgbegin;
   		printval(p->nodetype, 0);
		msgend( trdata.value );

	    (*rpt_output)(stdout, "after line %d in \"%s\":\t",
                                  prevline, basefile(prevfile));
	    (*rpt_output)(stdout, "%s = %s\n", trdata.token, trdata.value );
		trdata.reporting_trace_output = 0;
		(*rpt_trace)( &trdata );
		dispose( trdata.token );
		dispose( trdata.value );
    }
    prevline = curline;
    prevfile = curfile;
}

/*  
 * initialiaze the trinfo structure with the "initial" value
 */

initialize_trinfo(p)
Node p;
{
  Trinfo tp;
  int n;
  char buff[MAXTRSIZE];

  tp = findtrinfo(p);
  n = size(p->nodetype);
  dread(buff, tp->traddr, n);

  if (tp->trvalue == NULL)
  {
    tp->trvalue = newarr(char, n);
    mov(buff, tp->trvalue, n);
  } 
}

/*
 * Stop if the value of the given expression has changed.
 */

public stopifchanged(p)
Node p;
{
    register Trinfo tp;
    register int n;
    char buff[MAXTRSIZE];
    static Lineno prevline;

    tp = findtrinfo(p->value.trace.exp);
    n = size(p->value.trace.exp->nodetype);
    dread(buff, tp->traddr, n);

    if (cond(p->value.trace.cond))
    {
      if (tp->trvalue == nil)
      {
	tp->trvalue = newarr(char, n);
	mov(buff, tp->trvalue, n);
	isstopped = true;
      }
      else if (cmp(tp->trvalue, buff, n) != 0)
      {
	mov(buff, tp->trvalue, n);
	mov(buff, sp, n);
	sp += n;

        /*  if we know prevline  */
        if (prevline != 0)
          /*  print out the 'previous' line number  */
          (*rpt_output)(stdout, "after line %d:\t", prevline);

        /*  print new value  */
	prtree( rpt_output, stdout, p->value.trace.exp);
	(*rpt_output)(stdout, " = ");
	printval(p->value.trace.exp->nodetype, 0);
	(*rpt_output)(stdout, "\n");
	isstopped = true;
      }
    }
    prevline = curline;
}

/*
 * Free the tracing table.
 */

public trfree()
{
    register Trinfo tp;
    ListItem	cur_item;

    foreach (Trinfo, tp, trinfolist)
	dispose(tp->trvalue);
	cur_item = list_curitem(trinfolist);
	dispose( cur_item->element );
	list_delete(cur_item, trinfolist);
    endfor
}

/*
 * Fix up breakpoint information before continuing execution.
 *
 * It's necessary to destroy events and breakpoints that were created
 * temporarily and still exist because the program terminated abnormally.
 */

public fixbps()
{
    register Event e;
    register Trcmd t;
    ListItem	cur_item;

    single_stepping = false;
    inst_tracing = false;
    trfree();
    foreach (Event, e, eventlist)
	if (e->temporary) {
	    if (not delevent(e->id)) {
		(*rpt_output)(stdout, "!! dbx.fixbps: cannot find event %d\n",
									e->id);
	    }
	}
    endfor
    foreach (Trcmd, t, eachline)
	printrmtr(t);
	cur_item = list_curitem(eachline);
	dispose( cur_item->element );
	list_delete(cur_item, eachline);
    endfor
    foreach (Trcmd, t, eachinst)
	printrmtr(t);
	cur_item = list_curitem(eachinst);
	dispose( cur_item->element );
	list_delete(cur_item, eachinst);
    endfor
}

/*
 * Set all breakpoints in object code.
 */

public setallbps()
{
    register Breakpoint p;

    foreach (Breakpoint, p, bplist)
	setbp(p->bpaddr);
    endfor
}

/*
 * Undo damage done by "setallbps".
 */

public unsetallbps()
{
    register Breakpoint p;

    foreach (Breakpoint, p, bplist)
	unsetbp(p->bpaddr);
    endfor
}
