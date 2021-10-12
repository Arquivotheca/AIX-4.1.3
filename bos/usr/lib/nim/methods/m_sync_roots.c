static char	sccs_id[] = " @(#)13 1.1  src/bos/usr/lib/nim/methods/m_sync_roots.c, cmdnim, bos411, 9436D411a  9/7/94  15:09:43";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_sync_roots.c
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
{ATTR_NUM_PARALLEL_SYNCS,ATTR_NUM_PARALLEL_SYNCS_T,FALSE,	valid_pdattr_ass},
	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
	{ATTR_IGNORE_STATE,		ATTR_IGNORE_STATE_T,		FALSE,	ch_pdattr_ass},
	{ATTR_DEBUG,				ATTR_DEBUG_T,				FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE,	ch_pdattr_ass}
};

char *name=NULL;								/* NIM name of object to create */

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
*		calls undo_ckspot
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
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_GENERIC_MKSYNTAX),NULL);
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_MKSYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_MKSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/**************************       main         ********************************/

main( int argc, char *argv[] )

{	int rc;
	int i;
	char *tmp = NULL;
	NIM_OBJECT( spot, sinfo )

	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	/* initialize the attr_ass LIST */
	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_syncroots: perform root syncs for all clients using %s SPOT\n",
				name,NULL,NULL,NULL)

	/* get the SPOT's object */
	if ( lag_object( 0, name, &spot, &sinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	if ( spot->type->attr != ATTR_SPOT )
		nim_error(	ERR_OP_NOT_ALLOWED, ATTR_SYNC_ROOTS_T, 
						ATTR_msg(spot->type->attr), NULL );

	/* sync root directories for all diskless/dataless client using this SPOT */
	if ( sync_roots( spot, NULL, NULL, NULL ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* all done - bye bye */
	exit( 0 );
}

