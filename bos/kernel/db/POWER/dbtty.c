static char sccsid[] = "@(#)21	1.17  src/bos/kernel/db/POWER/dbtty.c, sysdb, bos411, 9428A410j 10/15/93 07:01:58";
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: tty_dump
 *
 * ORIGINS: 83
 *
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <stddef.h>
#include <sys/types.h>
#include "parse.h"

/*
 * this pointer is exported,
 * it is initialized to 0,
 * at initialization, the tty subsystem sets in this pointer the address of 
 * the function that dumps the tty structures.
 */
void (* db_tty_dump_ptr)(char * comand_line) = NULL;

extern char *in_string;		/* command line (declared in dbdebug.c) */

/*
* NAME: tty_dump
*
* FUNCTION: checks if the tty subsystem has initialized the address of
*	the dump function.
*	calls this function, passing to it the command line as parameter.
*
* PARAMETERS:
*	INPUT:	pointer to the parser structure.
*	OUTPUT: none
*
* RETURN VALUE: 0 is always returned to the caller
*
*/
int tty_dump(p)
struct parse_out *p;
{
    if (db_tty_dump_ptr == NULL) {
	/* tty subsystem not initialized or function not available */
	printf ("tty dump function not available\n");
    } else {
	/* call the dump function from the tty subsystem */
	db_tty_dump_ptr(in_string);
    }
    return 0;
}
