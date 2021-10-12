static char sccsid[] = "@(#)79    1.9.1.4  src/bos/usr/ccs/lib/libdbx/dpi_stack.c, libdbx, bos411, 9428A410j 11/15/93 13:59:38";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: dpi_get_traceback, getcallinfo, printtb, tbfree, tbinit,
 *            tracebackList_append, traceback_alloc
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
#include <setjmp.h>
#include <signal.h>
#include <sys/termio.h>
#include "defs.h"
#include "envdefs.h"
#include "runtime.h"
#include "frame.h"
#include "events.h"
#include "lists.h"
#include "process.h"
#include "dbx_msg.h"

extern int *envptr;            /* setjmp/longjmp data */

extern Ttyinfo ttyinfo;
extern Ttyinfo ttyinfo_in;
extern Boolean walkingstack;

extern nl_catd	scmc_catd;	/* Catalog descriptor for message conversion */

extern char *realloc();

char    *srcfilename( );
private void getcallinfo();

typedef List TracebackList;

#define tracebackList_append(traceback, el) list_append(list_item(traceback), \
                        nil, el)

private TracebackList tracebackList;        /* list of active tracebacks */


/*
 * NAME: tbinit
 *
 * FUNCTION: Allocated and initializes the tracebackList by calling list_alloc()
 *
 * PARAMETERS: NONE
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
public void tbinit( )
{
    tracebackList = list_alloc();
}


/*
 * NAME: traceback_alloc
 *
 * FUNCTION: Allocates new traceback element and fills in with information;
 *	     The new element is then appended to the end of tracebackList
 *
 * PARAMETERS:
 *	file		- File name to be put in traceback element
 *	function	- Function name to be put in traceback element
 *	line		- Line number to be put in traceback element
 *	address		- Address to be put in traceback element
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Newly allocated traceback element
 */
private ExecStruct traceback_alloc( file, function, line, address )
char *file;
char *function;
int line;
unsigned int address;
{
    register ExecStruct tb;

    /*
     * Allocate new traceback table element and fill in
     */
    tb = new(ExecStruct);
    tb->file = file;
    tb->function = function;
    tb->line = line;
    tb->address = address;

    /*
     * Append the new traceback table element to tracebackList
     */
    tracebackList_append(tb, tracebackList);
    return tb;
}


/*
 * NAME: tbfree
 *
 * FUNCTION: Free the traceback table
 *
 * PARAMETERS: NONE
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void tbfree( )
{
    ExecStruct tb;
    ListItem	cur_item;

    foreach (ExecStruct, tb, tracebackList)
	dispose(tb->file);
	dispose(tb->function);
	dispose( tb );
    endfor
    tracebackList->nitems = 0;
    tracebackList->head = nil;
    tracebackList->tail = nil;

}


/*
 * NAME: dpi_get_traceback
 *
 * FUNCTION: Fills in dpi_info with the current traceback information
 *
 * PARAMETERS:
 *	dpi_info	- Allocates an array to hold all items in the traceback
 *			  list.  Allocates an element for each of the items in
 *			  the traceback list.  The file and function name of
 *			  each traceback element is either allocated or set to
 *			  NULL.  This array will have the last item set to NULL.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: showaggrs	- set to false so aggregate values will not be
 *				  part of the name of the function, this is so
 *				  the function can fit on a single line easily
 *
 * RETURNS: 0 for success
 *	   -1 for failure
 */
public int dpi_get_traceback( dpi_info )
ExecStruct **dpi_info;
{
    ExecStruct *tbptr;
    ListItem tbitem;
    register ExecStruct tb;
    int tbnum;

    Frame frp;
    boolean save = walkingstack;
    Symbol f;
    struct Frame frame;
    extern Boolean call_command; 
    jmp_buf env;
    int *svenv;

    svenv = envptr;
    envptr = env;

    /*
     * Initialize dpi_info so that if we fail this will still be valid
     */
    *dpi_info = (ExecStruct *) calloc( 1, sizeof( ExecStruct ));

    if (setjmp( env )) {
        restoretty(stdout, &ttyinfo );
        restoretty(stdin, &ttyinfo_in );
        envptr = svenv;
        walkingstack = save;
        return -1;
    }

    /*
     * Free any old traceback table entries
     */
    tbfree( );

    if (notstarted(process) or isfinished(process)) {
        error( catgets( scmc_catd, MS_runtime, MSG_265,
			"program is not active" ));
        envptr = svenv;
        return -1;
    } else {
        walkingstack = true;

	/*
	 * Set showaggrs so aggregate values won't be printed
	 */
        showaggrs = false;

	/*
	 * Set f to the function associated with the current frame
	 */
        frp = &frame;
        if (getcurfunc(frp, &f) < 0) {
             error( catgets( scmc_catd, MS_runtime, MSG_269,
			     "Could not determine current function" ));
             envptr = svenv;
             return -1;
        }
	/*
	 * If this is not a call() command, then get the block information
	 * for the stack
	 */
        for (;
	     frp != nil and !call_command and !endofstack(frp);
	     frp = nextfunc( frp, &f )) {
	    /*
	     * Gets information about function f and appends to the
	     * tracebackList
	     */
            getcallinfo(f, frp);
        }

	/*
	 * Set tbnum to number of items in tracebackList
	 */
        tbnum = list_size( tracebackList );

	/*
	 * Allocate enough storage for *dpi_info to hold array of traceback
	 * table elements
	 */
	*dpi_info = (ExecStruct *) realloc( *dpi_info,
					    (tbnum + 1) * sizeof( ExecStruct ));

	/*
	 * Put each tracebackList item into *dpi_info array
	 */
        tbptr = *dpi_info;
        foreach( ExecStruct, tb, tracebackList )
            *tbptr++ = tb;
        endfor
        *tbptr = (ExecStruct) nil;
        walkingstack = save;
    }
    envptr = svenv;
    return 0;
}


/*
 * NAME: getcallinfo
 *
 * FUNCTION: Get the information about a call, i.e. routine name, parameter
 *	     values, and source location, put this information into a traceback
 *	     table element and append it to tracebackList.
 *
 * PARAMETERS:
 *	f	- Block to get information on
 *	frp	- Current frame we are interested in
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void getcallinfo( f, frp )
Symbol f;
Frame frp;
{
    Lineno line;
    Address caller;
    Address address;
    char *function;
    char *file;
    char *tfile;
    cases tempcase;

    caller = frp->save_pc;
    if (caller != reg(SYSREGNO(PROGCTR))) {
	/*
	 * Unless we are in the current procedure, we should use pc - 1
	 * instruction for the traceback so that we see the call as the current
	 * location in the caller.  The pc always points to the line following
	 * the call.
	 */
	/* Let's at least back it up to the previous instruction. */
        caller -= 4;
    }

    /*
     * Fill in function with the name of the function and its parameters and
     * values
     */
    msgbegin;
    if (isinline(f)) {
        (*rpt_output)(stdout, "unnamed block ");
    }

    /*
     * Get the name of the function
     */
    if (frp->tb.name_present && frp->name != nil && frp->name[0] != '\0')
	(*rpt_output)(stdout, "%s", frp->name);
    else
	printname( rpt_output, stdout, f, false);

    /*
     * If f is a function get its parameters and their values if noargs is not
     * set
     */
    if ((not isinline(f)) && (!(varIsSet("$noargs")))) {
        printparams(f, frp);
    }
    msgend( function );

    if ( *function == '\0' ) {
	function = realloc( function, 2 );
	strcpy( function, "." );
    }

    /* get around side effect of srcfilename() modifying our current case */
    tempcase = symcase;
    /*
     * Get source file name associated with caller's address
     */
    tfile = srcfilename(caller);
    symcase = tempcase;

    /*
     * Get the line number associated with address of caller;
     * line num is no good if no file name
     */
    line = ((tfile == nil) ? 0 : srcline(caller));

    if (tfile != nil) {
        file = malloc( (strlen( tfile ) + 1) * sizeof( char ) );
        strcpy( file, tfile );
	/*
	 * Get object address of line from the file
	 */
        address = objaddr( line, file );
    } else {
        address = caller;
        file = nil;
    }

    /*
     * Create traceback table element and append to tracebackList
     */
    (void) traceback_alloc( file, function, line, address );
}


#ifdef ADCDEBUG
void printtb( dpi_info )
ExecStruct *dpi_info;
{
    register ExecStruct tb;
    register ExecStruct *tbptr;

    for ( tbptr = dpi_info; *tbptr != nil; tbptr++ ) {
        tb = *tbptr;
        (*rpt_output)(stdout, "file:    \t%s\n", tb->file);
        (*rpt_output)(stdout, "function:\t%s\n", tb->function);
        (*rpt_output)(stdout, "line:    \t%d\n", tb->line);
        (*rpt_output)(stdout, "address: \t0x%x\n\n", tb->address);
        tbfree( );
    }
}
#endif	/* ADCDEBUG */
