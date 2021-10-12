static char sccsid[] = "@(#)75	1.10.2.4  src/bos/usr/ccs/lib/libdbx/dpi_location.c, libdbx, bos411, 9428A410j 11/15/93 13:59:32";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: dpi_current_location, get_location, dpi_find_function
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include <setjmp.h>
#include <sys/termio.h>
#include "defs.h"
#include "envdefs.h"
#include "decode.h"
#include "mappings.h"
#include "symbols.h"
#include "process.h"
#include "overload.h"
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

extern char	*strrchr(const char *s, int c);

extern Address pc;
extern int *envptr;            /* setjmp/longjmp data */
extern Ttyinfo ttyinfo;
extern Ttyinfo ttyinfo_in;
extern int lazy;

int dpi_current_location( dpi_info )
ExecStruct dpi_info;
{
    int len;
    jmp_buf env;
    int *svenv;
    char *temp;
    cases tempcase;

    dpi_info->file = nil;		/* file = nil && line = 0 */
    dpi_info->function = ".";		/* if file is not compile */
    dpi_info->line = 0;			/* with -g 		  */
    dpi_info->address = NOADDR;

    svenv = envptr;
    envptr = env;
    if (setjmp( env )) {
        restoretty( stdout, &ttyinfo );
        restoretty( stdin, &ttyinfo_in );
        envptr = svenv;
        return -1;
    }

    if (pc == 0 || pc == (Address) -1) {
        dpi_info->address = pc;
        envptr = svenv;
        return 0;
    }
    /* save off symcase because of the side effect */
    /* in calling srcfilename.                     */
    tempcase = symcase;
    temp = findsource(srcfilename(pc), NULL);
    symcase = tempcase;
    dpi_info->file = malloc( ( len = strlen( temp ) ) + 1 );
    strcpy( dpi_info->file, temp );
    if (dpi_info->file != nil)
    {
       if ( len && dpi_info->file[len - 1] == '/') {
        dpi_info->file = nil;
       }
    }
    dpi_info->function = symname( whatblock( pc ) );
    if (dpi_info->function == nil || *dpi_info->function == nil )
        dpi_info->function = ".";
    if (dpi_info->file == nil)
      dpi_info->line = 0;
    else dpi_info->line = srcline( pc );
    dpi_info->address = pc;
    envptr = svenv;
    return 0;
}

get_location( file, line, dpi_info )
char        *file;
int            line;
ExecStruct    dpi_info;
{
    int            len;

    dpi_info->line = line;
    dpi_info->file = findsource(file, NULL);
    if (dpi_info->file != nil)
    {
      if ( (len = strlen(dpi_info->file) ) 
        && dpi_info->file[len - 1] == '/') {
        dpi_info->file = nil;
      }
    }
    if (dpi_info->file != nil)
      dpi_info->address = objaddr( line, file );
    dpi_info->function = symname( whatblock( dpi_info->address ) );
    if (dpi_info->function == nil || *dpi_info->function == nil )
        dpi_info->function = ".";
}


/*
 * NAME: dpi_find_function
 *
 * FUNCTION: Returns the filename and line number of the specified function
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *
 * PARAMETERS:
 *	function	- Name of the function to look for
 *	file		- Name of the file is returned here, caller should not
 *			  free this memory, and it should not depend on it
 *			  staying around
 *	line		- Line number in the file where the function is found
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *
 * RETURNS: -1 for failure.
 *	     0 for success, but user did not make any selection if
 *	       the function was overloaded.
 *	     1 for success, a selection was made.
 */
int dpi_find_function( function, file, line )
char *function;
char **file;
int *line;
{
    extern char	scanner_linebuf[];
    Name	funcname;
    Symbol	s;
    Address addr;
    cases	savecase;
    jmp_buf env;
    int *svenv;
    int 	rc = 1;
    char	*basename;
    Node	n;

    svenv = envptr;
    envptr = env;
    if (setjmp( env )) {
        envptr = svenv;
        return 0;  /* Cancel was activated in the resolve dialog */
    }

    if( *function == '' ) {
	sscanf( function, "%x", &s );
    } else {
	/*
	 * Get the basename of the function
	 */
	basename = strrchr(function, '.');
	if (!basename) {
	    basename = function;
	} else {
	    basename++;
	}

	/*
	 *  scanner_linebuf gets passed to the resolve callback.
	 *  Since we don't know what the command will be just
	 *  make it the name of the function.
	 */
	strcpy(scanner_linebuf, basename);

	funcname = identname(basename, false);
	if ((n = resolveFns(funcname, (unsigned long)0)) == nil)
	    return -1;
	s = n->value.sym;
	free(n);
    }

    /*
     * If we are using fastload, make sure the symbolic information has been
     * read in for the specified function
     */
    if( lazy ) {
	touch_file( s );
    }

    /*
     * Get the address for the first executable line in the function
     */
    addr = codeloc(s);
    if (addr != NOADDR) {
	if (!nosource(s)) {
	    /*
	     * Get the filename and line number associated with the function.
	     * Save our current case since srcfilename() may affect it.
	     */
	    savecase = symcase;
	    *file = srcfilename(addr);
	    *line = srcline(addr);
	    symcase = savecase;
	} else {
	    rc = -1;
	}
    } else {
	rc = -1;
    }

    envptr = svenv;
    return rc;
}
