static char sccsid[] = "@(#)58    1.11.1.7  src/bos/usr/ccs/lib/libdbx/dpi_file.c, libdbx, bos411, 9428A410j 4/8/94 17:51:06";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: dpi_current_file, dpi_resolve_file, dpi_src_lang
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
#include "process.h"
#include "symbols.h"
#include "runtime.h"
#include "frame.h"

extern Lineno lastlinenum;
extern String prevsource;
extern int    *envptr;            /* setjmp/longjmp data */
extern Ttyinfo    ttyinfo;
extern Ttyinfo    ttyinfo_in;


/*
 * NAME: dpi_current_file
 *
 * FUNCTION: Returns the contextual information about the next source line to be
 *	     displayed via the dbx 'list' command.  If there is no source
 *	     available for the current context it will just return the address
 *	     if possible.
 *
 * PARAMETERS:
 *	dpi_info	- Filled in with current file, function, line and
 *			  address of that context.  If dpi->file is not NULL
 *			  then it has been allocated by this function.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: -1 for failure; 0 Success
 */
int dpi_current_file( dpi_info )
ExecStruct dpi_info;
{
    register Lineno cur_line;
    int len;
    jmp_buf env;
    int *svenv;
    char *dummy;
    char *temp;

    /*
     * If there is no current context information than we will default to the
     * following values
     */
    dpi_info->file = nil;
    dpi_info->function = ".";
    dpi_info->line = 0;
    dpi_info->address = NOADDR;

    svenv = envptr;
    envptr = env;
    if (setjmp( env )) {
	/*
	 * Since objaddr() is the only one of the functions that get called
	 * that might do a longjmp() we know we did an errbegin, so we must
	 * do an errend
	 */
        errend(dummy);
        dispose(dummy);
        restoretty(stdout, &ttyinfo );
        restoretty(stdin, &ttyinfo_in );
        envptr = svenv;
	if (numerrors() > 0)		/* check if longjmp is caused by */
	  return 0;			/* error. If so return 0.	 */
	else
          return -1;
    }

    if (cursource == nil) {
	/*
	 * We don't have any source information for the current function, so we
	 * will just update the address if we have a current frame
	 */
	if( curframe != NULL ) {
	    dpi_info->address = curframe->save_pc;
	}
        envptr = svenv;
        return 0;
    }
    temp = findsource(cursource, NULL);
    len = strlen( temp );
    dpi_info->file = malloc( ( len + 1 ) * sizeof( char ) );
    strcpy( dpi_info->file, temp );
    if ( len && dpi_info->file[len - 1] == '/') {
        dpi_info->file = nil;
	envptr = svenv;
        return 0;
    }
    if( canReadSource() == false || lastlinenum == 0 ) {
        envptr = svenv;
        return 0;
    }
    /*
     * cursrcline is kept up to date by dbx
     * cursrcline points to the first line to start the next listing from in
     * dbx, the DPI debuggers need to know what the real current line is (i.e.
     * the last line that was listed).
     */
    cur_line = cursrcline == LASTLINE ? lastlinenum : cursrcline - 1;
    cur_line = (cur_line < 1) ? 1 : cur_line;
    if (cur_line > lastlinenum) {
	/* This indicates an error condition - don't set line number */
        envptr = svenv;
        return 0;
    }
    /*
     * Redirect error message output to avoid user seeing
     * "file not compiled -g" message
     */
    errbegin;
    dpi_info->line = cur_line;
    if ((dpi_info->address = objaddr( cur_line, cursource )) != NOADDR)
	dpi_info->function = symname( curfunc );
    errend(dummy);
    dispose(dummy);
    envptr = svenv;
    return 0;
}


/*
 * NAME: dpi_resolve_file
 *
 * FUNCTION: Returns the full path name for a file based on the use
 *	path.
 *
 * PARAMETERS:
 *	file - file name to resolve
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: -1 for failure; 0 Success
 */
int dpi_resolve_file( file, response )
char	*file;
char	**response;
{
    char	*result;

    result = findsource(file, NULL);
    if (result)
	*response = strdup(result);
    else
	*response = nil;

    return result ? 0 : -1;
}


/*
 * NAME: dpi_src_lang
 *
 * FUNCTION: Returns the language of the current source file.
 *
 * PARAMETERS: None
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: Global variable cursrclang.
 *
 * RETURNS: Returns the language of the current source file.
 */
VarLanguage dpi_src_lang()
{
    return((VarLanguage)cursrclang);
}
