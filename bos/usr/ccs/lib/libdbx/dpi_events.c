static char sccsid[] = "@(#)73	1.9.1.3  src/bos/usr/ccs/lib/libdbx/dpi_events.c, libdbx, bos411, 9428A410j 5/11/94 14:23:01";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: dpi_get_eventHistory, ehinit, eventHistoryList_append,
 *	      eventHistory_alloc, eventIsStop, getEventVar, dpi_why_stopped
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/termio.h>
#include "defs.h"
#include "envdefs.h"
#include "runtime.h"
#include "events.h"
#include "lists.h"
#include "process.h"
#include "tree.h"
#include "operators.h"

extern	int *envptr;			/* setjmp/longjmp data */
extern boolean	isXDE;		/* true if debugger which uses DPI */

extern	Ttyinfo ttyinfo;
Boolean	eventIsStop( );
char	*srcfilename( );
char	*getEventVar( );
private	EventType	getEventType( );

typedef List EventHistoryList;

#define eventHistoryList_append(eventHistory, el) \
	list_append(list_item(eventHistory), nil, el)

private EventHistoryList eventHistoryList; /* FIFO list of active events */
					   /* "active" means the event	 */
					   /* has not been removed by a	 */
					   /* call to dpi_get_history()	 */


/*
 * NAME: ehinit
 *                                                                    
 * FUNCTION: Empty the eventHistoryList.
 *                                                                    
 * EXECUTION ENVIRONMENT: normal user process
 *                                                                   
 * NOTES: If the eventHistoryList has already been created then
 *	we need to free up everything that is currently in the
 *	list, but we do not need to reallocate it.
 *
 * DATA STRUCTURES: Sets up eventHistoryList as an empty list.
 *
 * RETURNS: NONE
 */  
public void ehinit( )
{
    ListItem	ehitem;	/* the event history list item which	*/
			/* holds the event history element (eh)	*/
			/* which is to be deleted from the	*/
			/* eventHistoryList and freed		*/
    BkptStruct	eh;	/* used to empty out the eventHistoryList	*/

    if ( !eventHistoryList )
	/*
	 * The list needs to be created from scratch.
	 */
	eventHistoryList = list_alloc();
    else {
	/*
	 * Clear out the event history list.
	 */
	foreach( BkptStruct, eh, eventHistoryList )
	    dispose(eh->file);
	    ehitem = list_curitem(eventHistoryList);
	    dispose( ehitem->element );
	    list_delete( ehitem, eventHistoryList);
	endfor
    }
}


/*
 * NAME: eventHistory_alloc
 *                                                                    
 * FUNCTION: This function is called every time a break or trace is set.
 *	If the Breakpoint passed in is not temporary then convert
 *	the Breakpoint ( which can be either a break or trace event )
 *	to a BkptStruct and append it to the eventHistoryList.
 *                                                                    
 * EXECUTION ENVIRONMENT: normal user process
 *                                                                   
 * NOTES:  The eventHistoryList is a FIFO list.  It is important that
 *	the new item be appended to the list because the list contains
 *	both set and unset events and if the events are not processed
 *	in the order that they occur then one could end up marking a
 *	breakpoint as being set when it had actually been deleted.
 *
 * DATA STRUCTURES: a new item is appended to the eventHistoryList.
 *
 * RETURNS: NONE
 */  
public void eventHistory_alloc( bp, type )
Breakpoint	bp;
BkptType	type;
{
    register BkptStruct eh;	/* Event history item which is created	*/
				/* and appended to the eventHistoryList	*/
    char 		*tfile;	/* Holds the source filename associated	*/
				/* with the Breakpoint bp.		*/

    /*
     * Don't add the event to the history list if it is only
     * temporary since it should be hidden from the user.
     * Don't add the event to the history list if not using a DPI debugger.
     */
    if (!isXDE || bp->event->temporary == true)
	return;

    /*
     * Allocate storage for the new item.
     */
    eh = new( BkptStruct );

    /*
     * Fill in the variable name.
     */
    eh->variable = getEventVar( bp );

    /*
     * Allocate space and fill in the file field with the filename if
     * we have one.
     */
    if (eh->variable) {
	eh->file = nil;
    } else {
	tfile = srcfilename( bp->bpaddr );
	if (tfile) {
	    /*
	     * We have a filename to associate with the event.
	     */
	    eh->file = malloc( strlen( tfile ) + 1);
	    strcpy( eh->file, tfile );
	} else {
	    /*
	     * There's no filename associated with the event.
	     */
	    eh->file = nil;
	}
    }

    /*
     * Fill in the source line number the breakpoint is on.
     */
    if (bp->bpline == 0 && eh->variable == nil)
	/*
	 * If we have a machine level breakpoint set then attempt to get the
	 * line number from the address the breakpoint is set at.
	 */
	eh->line = srcline( bp->bpaddr );
    else
	/*
	 * The line number passed in is ok to use.
	 */
	eh->line = bp->bpline;

    /*
     * Fill in the rest of the structure.
     */
    eh->address = bp->bpaddr;
    eh->type = type;
    eh->eventnum = bp->event->id;
    eh->event_type = getEventType( bp );

    /*
     * The event history structure is completely filled in now
     * so stick on the end of the list.
     */
    eventHistoryList_append(eh, eventHistoryList);
}


/*
 * NAME: getEventType
 *                                                                    
 * FUNCTION: determine the event type of the given event
 *                                                                    
 * EXECUTION ENVIRONMENT: normal user process
 *                                                                   
 * NOTES: We determine the event type by looking at the actions
 *	associated with the event.  This function will return:
 *	
 *	EVENT_BREAK - for stop commands with no if:
 *		command			action list
 *		stop in func		O_STOPX
 *		stop at line		O_STOPX
 *		stopi at addr		O_STOPX
 *		stopi in func		O_STOPX
 *		stop var		O_TRACEON - O_STOPIFCHANGED
 *		stopi addr		O_TRACEON - O_STOPIFCHANGED
 *
 *	EVENT_COND - for stop commands with an if condition:
 *		command			action list
 *		stopi in func if cond	O_TRACEON - O_IF - O_STOPX
 *		stop in func if cond	O_TRACEON - O_IF - O_STOPX
 *		stopi at addr if cond	O_IF - O_STOPX
 *		stop at line if cond	O_IF - O_STOPX
 *		stop var if cond	O_TRACEON - O_IF - O_STOPIFCHANGED
 *		stop var in f if cond	O_TRACEON - O_IF - O_STOPIFCHANGED
 *		stop if cond		O_TRACEON - O_IF - O_STOPIFCHANGED
 *		stopi addr if cond	O_TRACEON - O_IF - O_STOPIFCHANGED
 *		stopi addr in f if cond	O_TRACEON - O_IF - O_STOPIFCHANGED
 *
 *	EVENT_TRACE - for everything else ( trace commands )
 *		trace ...
 *
 *	We will decode the action list to determine the event type.
 *
 * RETURNS: EVENT_TRACE, EVENT_COND, or EVENT_BREAK depending on the
 *	type of the event passed in.  See notes
 */  
private EventType getEventType( bp )
Breakpoint bp;	/* trace, break, or conditional break event	*/
{
    Command cmd, ifcmd, stopcmd;
    EventType	rc;

    /*
     * Assume the event is a trace and prove otherwise.
     */
    rc = EVENT_TRACE;

    cmd = list_element(Command, list_head(bp->event->actions));
    if (cmd->op==O_STOPX)
	/*
	 * stop in func | stop at line	| stopi at addr	| stopi in func
	 */
	rc = EVENT_BREAK;
    else if (cmd->op == O_IF) {
	stopcmd = list_element(Command,list_head(cmd->value.trace.actions));
	if (stopcmd->op == O_STOPX)
	    /*
	     * stopi at addr if cond | stop at line if cond
	     */
	    rc = EVENT_COND;
    }
    else if (cmd->op == O_TRACEON) {
	ifcmd = list_element(Command, list_head(cmd->value.trace.actions));
	if (ifcmd->op == O_STOPIFCHANGED ) 
	    /*
	     * stop var | stopi addr
	     */
	    rc = EVENT_BREAK;
        else if (ifcmd->op == O_IF) {
	    stopcmd=list_element(Command,list_head(ifcmd->value.trace.actions));
	    if (stopcmd->op == O_STOPX || stopcmd->op == O_STOPIFCHANGED)
		/*
		 * stopi in func if cond | stop in func if cond
		 * or
		 * stop var if cond | stop var in f if cond |
		 * stopi addr if cond | stopi addr in f if cond
		 * stop if cond |
		 */
	        rc = EVENT_COND;
        }
    }
    return rc;
}


/*
 * NAME: getEventVar
 *                                                                    
 * FUNCTION: determine the variable associated with the given event
 *                                                                    
 * EXECUTION ENVIRONMENT: normal user process
 *                                                                   
 * NOTES: We determine the event type by looking at the actions
 *
 * RETURNS: a pointer to a character string containing the variable
 *	that is associated with the event.  nil is returned if there
 *	is no variable associated with the event.
 */  
char *getEventVar( bp )
Breakpoint bp;
{
    Command cmd, ifcmd, stopcmd;
    char *eventVar;

    cmd = list_element(Command, list_head(bp->event->actions));
    if (cmd->op == O_TRACEON) {
        ifcmd = list_element(Command, list_head(cmd->value.trace.actions));
        if (ifcmd->op == O_STOPIFCHANGED || ifcmd->op == O_PRINTIFCHANGED) {
   	    checkref(ifcmd);
	    msgbegin;
	    prtree( rpt_output, stdout, ifcmd->value.arg[0]);
	    msgend( eventVar );
	    return eventVar;
	} else if ( ifcmd->op == O_IF ) {
	    stopcmd=list_element(Command,list_head(ifcmd->value.trace.actions));
	    if( stopcmd->op == O_STOPIFCHANGED ||
		stopcmd->op == O_PRINTIFCHANGED ) {
		checkref(stopcmd);
		msgbegin;
		prtree( rpt_output, stdout, stopcmd->value.arg[0]);
		msgend( eventVar );
		return eventVar;
	    } else {
		return nil;
	    }
        } else
	    return nil;
    } else
	return nil;
}


/*
 * NAME: dpi_get_history
 *                                                                    
 * FUNCTION: Fill in the dpi_info output parameter with a null
 *	terminated array of pointers to all the events
 *	that have accumulated in the eventHistoryList since the last
 *	call to this function that are of the same type as indicated
 *	by the input parameter.
 *                                                                    
 * EXECUTION ENVIRONMENT: normal user process
 *                                                                   
 * NOTES: see the declaration of the BkptStruct structure for a
 *	description of the fields that are filled in.
 *
 *	It is the responsibility of the calling program to free the
 *	storage allocated for the dpi_info array and everything that
 *	it points to.
 *
 * DATA STRUCTURES: When this routine returns, all the
 *	events which are in the returned array will have been deleted
 *	from the eventHistoryList.
 *
 * RETURNS: NONE (sets dpi_info as discussed above)
 */  
private	dpi_get_history( dpi_info, type )
BkptStruct	**dpi_info;	/* output parameter, which is	*/
				/* to be filled with the nil	*/
				/* terminated array of pointers	*/
				/* to BkptStructs.		*/
EventType	type;		/* break/trace/code event	*/
{
    int		ehnum;	/* total number of events in the 	*/
			/* eventHistoryList which are of the	*/
			/* type indicated by the type_mask	*/
    BkptStruct *ehptr;	/* a pointer used to fill in the	*/
			/* dpi_info array of BkptsStructs	*/
    ListItem	ehitem;	/* the event history list item which	*/
			/* holds the event history element (eh)	*/
			/* which is to be deleted from the	*/
			/* eventHistoryList and moved to the	*/
			/* dpi_info array			*/
    BkptStruct	eh;	/* an event history element which may	*/
			/* need to be removed from the		*/
			/* eventHistoryList and placed in the	*/
			/* dpi_info array			*/

    /*
     * Get the total number of events in the history list which
     * are of the type specified by the type input parameter.
     */
    ehnum = 0;
    foreach( BkptStruct, eh, eventHistoryList )
	if (eh->event_type & type)
	    ehnum++;
    endfor
    /*
     * Allocate space for the history information to be returned
     * now that we know how many items are in the list.  The extra
     * element is for null termination.
     */
    *dpi_info = newarr( BkptStruct, (ehnum+1) );

    /*
     * Move the events from the eventHistoryList to the list of
     * matching events to be returned to the caller.
     */
    ehptr = *dpi_info;
    foreach( BkptStruct, eh, eventHistoryList )
	if (eh->event_type & type)
	{
	    /*
	     * Add the matching event history element to the list to
	     * be passed back.
	     */
	    *ehptr++ = eh;

	    /*
	     * Now that the event history element has been added to
	     * the return list we must delete it from the eventHistoryList
	     * and free the corresponding list item.  The actual element
	     * must not be freed since its reference is being passed back
	     * to the calling function.
	     */
	    ehitem = list_curitem(eventHistoryList);
	    list_delete( ehitem, eventHistoryList);
	}
    endfor

    /*
     * Null terminate the list.
     */
    *ehptr = (BkptStruct) nil;

    /*
     * Success.
     */
    return 0;
}


public	dpi_get_eventHistory( dpi_info )
BkptStruct	**dpi_info;
{
    return dpi_get_history( dpi_info, EVENT_BREAK );
}

public	dpi_get_traceHistory( dpi_info )
BkptStruct	**dpi_info;
{
    return dpi_get_history( dpi_info, EVENT_TRACE | EVENT_COND );
}


public	dpi_why_stopped( event, signal )
int	*event, *signal;
{
    extern	int	eventIdLastBp;

    *event = *signal = -1;
    if (!process)
	return -1;
    if ( process->signo && process->signo != 5 )
	*signal = process->signo;
    else if (eventIdLastBp > 0)
	*event = eventIdLastBp;
    return 0;
}
