static char sccs_id[] = " @(#)88 1.5  src/bos/usr/lib/nim/methods/c_errmsg.c, cmdnim, bos41J, 9516B_all  4/21/95  12:56:04";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/c_stat.c
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cmdnim_cmd.h"
/* The following is just a workaround to resolve an extern in a system
 * include file so that c_errmsg could be used on 3.2 systems for
 * unattended migration purposes.
 */
int _system_configuration = 0;

/* --------------------------- module globals    ---------------------------- */
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{0,							NULL, 						FALSE,	NULL}
};

/*---------------------------- c_errmsg          ------------------------------
*
* NAME: c_errmsg
*
* FUNCTION:
*		displays the specified error or informational (MSG_ set) message
*		this is required for NIM shell scripts because shell scripts to not have
*			a utility analogous to printf
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			1						= msg_no
*			2						= msg_type (0 == info msg, non-0 == error msg)
*			3						= str1 of error msg
*			4						= str2 of error msg
*			5						= str3 of error msg
*		global:
*
* RETURNS: (int)
*
* OUTPUT:
*		error message written to stderr
*-----------------------------------------------------------------------------*/

main( int argc, char *argv[] )

{	int msg_no, msg_type;
	char *msg = NULL;

	/* initialize NIM environment */
	nim_init( argv[0], ERR_POLICY_METHOD, NULL, NULL );

	/* argv[1] should be the msg_no */
	msg_no = (int) strtol( argv[1], NULL, 0 );

        /* argv[2] says whether msg is an error or info msg */
	msg_type = (int) strtol( argv[2], NULL, 0 );

	if (msg_type != 0) {
		/* retrieve the error message */
		if (	((msg = ERR_msg( msg_no )) == NULL) &&
				((msg = ERR_msg( ERR_SYS )) == NULL) )
			exit( 0 );
       
	} else {
		/* retrieve the info message */
		if ((msg = MSG_msg( msg_no )) == NULL)
			exit( 0 );
	}

	/* display the request message */
	fprintf( stderr, msg, argv[3], argv[4], argv[5], argv[6] );
	fprintf( stderr, "\n" );
	fflush( stderr );

	exit( 0 );
}

