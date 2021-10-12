static char sccsid[] = "@(#)77    1.12.1.1  src/bos/usr/ccs/lib/libdbx/dpi_message.c, libdbx, bos41J, 9511A_all 2/7/95 16:32:55";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: dpi_message, dpi_help
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
#include "defs.h"
#include <varargs.h>
#include <stdio.h>

/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

char *realloc( );

char *msgptr = nil;
int msgbufsiz = BUFSIZ;

#define NUM_HELP_MSGS	20	/* Number of help messages for dpi_help */
#define	LEN_HELP_BUF	1600	/* Amount of memory allocated for buf */

/*
 * NAME: dpi_message
 *
 * FUNCTION: Message handling routine
 *
 * NOTES: If the file descriptor is stdout or stderr, then the message is 
 *	  concatenated on the end of msgptr.  (More space is allocated if
 *	  necessary.)  Otherwise the message is sent to stderr.
 *
 * PARAMETERS: Unspecified number of arguments accepted.
 *	       Expects a (FILE *) which describes where the message is to go
 *	       Expects a (char *) which is the message to print
 *	       All parameters after that are treated as printf() arguments
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: msgptr will be modified if the first parameter is stdout
 *	or stderr
 *
 * RETURNS: Always 0
 */
int dpi_message(va_alist)
va_dcl
{
    FILE *stream;
    char *fmt;
    va_list ap;
    char buffer[BUFSIZ];

    if ( !msgptr ) {	/* Initialize msgptr */
        msgbufsiz = BUFSIZ;
        msgptr = malloc( msgbufsiz );
        *msgptr = '\0';
    }

    /*
     * Get the expected arguments from the list
     */
    va_start(ap);
    stream = va_arg(ap, FILE *);
    fmt = va_arg(ap, char *);
    vsprintf (buffer, fmt, ap);
    if ((stream == stdout) || (stream == stderr)) {
        while (strlen( msgptr ) + strlen( buffer ) >= msgbufsiz - 1) {
            msgptr = realloc( msgptr, msgbufsiz + BUFSIZ );
            msgbufsiz += BUFSIZ;
        }
        strcat( msgptr, buffer );
    }
    else
        fprintf(stderr, buffer );
    va_end(ap);
    return( 0 );
}


/*
 * NAME: dpi_help
 *
 * FUNCTION: Returns help text for dbx in parameter buf
 *
 * PARAMETERS:
 *	buf	- Filled in with the help text for dbx
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Always 0
 */
int dpi_help( buf, outputFun )
char **buf;
#ifndef _NO_PROTO
int             (*outputFun)(FILE *, const char *, ...);
#else
int             (*outputFun)();
#endif
{
  int	index;
  int	len_buf = 0;	/* Holds current length of messages in buf */
  int	size_buf = LEN_HELP_BUF;	/* Indicates amount of memory allocated
					   for buf */
  int	cur_len = 0;	/* Holds length of current message */
  char	*cur_msg;	/* Holds current message retrieved from catalog */

  struct {
	int	msg_no;
	char	*def_msg;
  } help_msgs[NUM_HELP_MSGS] = {
  {MSG_190, "run                    - begin execution of the program"},
  {MSG_191, "print <exp>            - print the value of the expression"},
  {MSG_192, "where                  - print currently active procedures"},
  {MSG_193, "stop at <line>         - suspend execution at the line"},
  {MSG_194, "stop in <proc>         - suspend execution when <proc> is called"},
  {MSG_195, "cont                   - continue execution"},
  {MSG_196, "step                   - single step one line"},
  {MSG_197, "next                   - step to next line (skip over calls)"},
  {MSG_198, "trace <line#>          - trace execution of the line"},
  {MSG_199, "trace <proc>           - trace calls to the procedure"},
  {MSG_200, "trace <var>            - trace changes to the variable"},
  {MSG_201, "trace <exp> at <line#> - print <exp> when <line> is reached"},
  {MSG_202, "status                 - print trace/stop's in effect"},
  {MSG_203, "delete <number>        - remove trace or stop of given number"},
  {MSG_204, "screen                 - switch dbx to another virtual terminal"},
  {MSG_205, "call <proc>            - call a procedure in program"},
  {MSG_206, "whatis <name>          - print the declaration of the name"},
  {MSG_207, "list <line>, <line>    - list source lines"},
  {MSG_208, "registers              - display register set"},
  {MSG_209, "quit                   - exit dbx"}
    };

  rpt_output = outputFun;
  if ( scmc_catd == nil ) {
     scmc_catd=catopen("dbx.cat", NL_CAT_LOCALE);
  }

  *buf = malloc(size_buf * sizeof( char )); 
  for( index = 0; index < NUM_HELP_MSGS; index++ ) {
	cur_msg = catgets( scmc_catd, MS_eval, help_msgs[index].msg_no,
			   help_msgs[index].def_msg );
	/*
	 * Check if the new message plus newline,
	 * and NULL will fit into allocated space
	 */
	cur_len = strlen( cur_msg ) + 2;
	if( (len_buf + cur_len ) >= size_buf ) {
	    size_buf += LEN_HELP_BUF;
	    *buf = realloc( *buf, size_buf * sizeof( char ) );
	}
	len_buf += sprintf( *buf + len_buf, "%s\n", cur_msg );
  }
  return 0; 
}
