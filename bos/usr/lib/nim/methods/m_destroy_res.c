static char	sccs_id[] = " @(#)51 1.1  src/bos/usr/lib/nim/methods/m_destroy_res.c, cmdnim, bos411, 9428A410j  5/24/94  14:27:21";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_destroy_res.c
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

extern int valid_pdattr_ass();
extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
 	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
 	{ATTR_IGNORE_STATE,		ATTR_IGNORE_STATE_T,		FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE,	NULL}
};

char *server=NULL;
	
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
*			argc			= argc from main
*			argv			= argv from main
*		global:
*
* RETURNS: (int)
*		SUCCESS					= no syntax errors on command line
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
parse_args(	int argc, 
				char *argv[] )

{	extern char *optarg;
	extern int optind, optopt;
	int c;
	int syntax_err=FALSE;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:q")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
			break;

			case 'q': /* display valid_attrs */
				mstr_what( valid_attrs, NULL );
				exit( 0 );
			break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

			case '?': /* unknown option */
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_GENERIC_RMSYNTAX),NULL);
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_RMSYNTAX), NULL, NULL );
	}

	/* object name (the only operand) is required */
	if ( optind != (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_RMSYNTAX), NULL, NULL );

	server = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{	NIM_OBJECT( sobj, sinfo )
	NIM_OBJECT( robj, rinfo )
	int i;

	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
   if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
      nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_destroy_res: remove all resources served by %s\n",server,
				NULL,NULL,NULL)

	/* get the server's info */
	if ( get_object( &sobj, &sinfo, 0, server, 0, 0 ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* for each resource this machines serves... */
	for (i=0; i < sobj->attrs_info->num; i++)
		if ( sobj->attrs[i].pdattr->attr == ATTR_SERVES )
		{
			/* "destroy" this resource by removing it's NIM object */
			if ( get_object(	&robj, &rinfo, 0, sobj->attrs[i].value, 
									0, 0 ) == FAILURE )
				warning( 0, NULL, NULL, NULL );
			else if ( rm_robj( robj ) == FAILURE ) 
				warning( 0, NULL, NULL, NULL );

			odm_free_list( robj, &rinfo );
		}

	exit( 0 );
}
