static char sccsid[] = "@(#)80    1.8  src/bos/usr/ccs/lib/libdbx/dpi_token.c, libdbx, bos411, 9428A410j 1/20/92 17:50:30";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: dpi_get_token_type
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include <stdio.h>
#include <setjmp.h>
#include <sys/termio.h>
#include "defs.h"
#include "envdefs.h"
#include "process.h"
#include "scanner.h"

SymbolType symbol_type;		/* set in the symtype command */

/*
 * NAME: dpi_get_token_type
 *
 * FUNCTION: Fill in the type of the specified token.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: The setjmp/longjmp series does not need to be
 *	done for this function since the only call to dbx
 *	functions that can return errors is dpi_command()
 *	which will set up that code itself.
 *
 *	The type is retrieved by issuing a "symtype token"
 *	command to dbx via dpi_command.
 *
 *	The possible values for the type are VARIABLE, FUNCTION
 *	and UNKNOWN.  UNKNOWN is the default.
 *
 * PARAMETERS:
 *	token - input, the token whose type is in question.
 *	type  - output, the type of the input token.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES:
 *	symbol_type is filled in with the type of the token
 *	from the symtype token command.
 *
 * RETURNS:
 *	0	Successful completion
 *	-1	Unsuccessful completion
 */
int dpi_get_token_type( token, type )
char *token;
int *type;
{
    unsigned int dpi_state = 0;	/* dummy for call to dpi_command  */
    int  err;			/* rc from dpi_command (0 or -1)  */
    char *dummy;		/* for tossing any error messages */
    char line[MAXLINESIZE];	/* to hold symtype token command  */

    /*
     * Make sure the token isn't too big for the symtype command.
     * 10 is added to the size of token to account for the symtype,
     * blank, newline, and NULL characters that are part of the
     * command.
     */
    if (strlen(token)+10 >= MAXLINESIZE) {
      *type = UNKNOWN;
      return 0;
    }

    symbol_type = UNKNOWN;	/* initial value                  */

    /*
     * Ignore any error messages.
     */
    errbegin;

    /*
     * Create the command to determine the type and send it off.
     */
    sprintf( line, "symtype %s\n", token );
    resetinput();
    err = dpi_command(line, &dpi_state);
    errend( dummy );
    dispose( dummy );

    /*
     * Fill in the output parameter with the determined type.
     */
    *type = symbol_type;
    return err;
}
