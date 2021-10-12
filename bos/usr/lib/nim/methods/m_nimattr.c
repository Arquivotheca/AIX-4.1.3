char	sccs_id[] = " @(#)81	1.1  src/bos/usr/lib/nim/methods/m_nimattr.c, cmdnim, bos411, 9428A410j  9/30/93  10:11:37";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_nimattr.c
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

#include "cmdnim_mstr.h"

extern int	valid_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

#define MAX_VALID_ATTRS				9
VALID_ATTR valid_attrs[] = 
{
	{ 0, 	NULL, 	FALSE, 	valid_pdattr_ass },
	{ 0, 	NULL, 	FALSE, 	NULL }
};


char	*name = NULL;
char	*type = NULL;

/*---------------------------- parse_args        ------------------------------
*
* NAME: parse_args
*
* FUNCTION:
*		parses command line arguments
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls nim_error
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			argc	= argc from main
*			argv	= argv from main
*		global:
*
* RETURNS: (int)
*		SUCCESS		= no syntax errors on command line
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
parse_args(	int argc, 
char *argv[] )

{	
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:")) != -1 ) {	
		switch (c) {
		case 'a': /* attribute assignment */
			if (parse_attr_ass( &attr_ass, valid_attrs, 
			    optarg, TRUE ) == FAILURE)
				nim_error( 0, NULL, NULL, NULL );
			break;

		case ':': /* option is missing a required argument */
			nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

		case '?': /* unknown option */
			nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_M_MKMAC_SYNTAX), NULL );
			break;
		}
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc - 1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_M_MKMAC_SYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */


/**************************       main         ********************************/
main( int argc, char *argv[] )
{	
	long	id;
	int	i;

	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( !get_list_space( &attr_ass, MAX_VALID_ATTRS, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	/* make sure object exists */
	if ( (id = get_id(name)) == 0 )
		nim_error( 0, NULL, NULL, NULL );

	for (i = 0; i < attr_ass.num; i++)
		if ( ch_attr( id, NULL, attr_ass.list[i]->value,
		    attr_ass.list[i]->seqno,
		    attr_ass.list[i]->pdattr,
		    attr_ass.list[i]->name ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );

	exit( 0 );
}
