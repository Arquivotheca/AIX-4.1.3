static char	sccs_id[] = " @(#)03 1.4  src/bos/usr/lib/nim/methods/m_ls_lpp_source.c, cmdnim, bos411, 9428A410j  5/25/94  13:16:25";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_lslpp_source.c
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

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{  
   {0,                     NULL,                   FALSE,   NULL}
};


char *name=NULL;
	
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

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/*---------------------------- ls_options             --------------------------
*
* NAME: ls_options
*
* FUNCTION:
*		displays the options which are available from the specified lpp_source
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
*		global:
*			name
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= error encountered
*
* OUTPUT:
*		info written to stdout
*-----------------------------------------------------------------------------*/

int
ls_options()

{	NIM_OBJECT( obj, info )
	int server;
	int location;
	int rc;
	char *args[] = { C_MK_LPP_SOURCE, "-l", NULL, NULL };

	/* get all the info about this object */
	if ( lag_object( 0, name, &obj, &info ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* get the server & location */
	if ( (server = find_attr( obj, NULL, NULL, 0, ATTR_SERVER )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, obj->name, NULL );
	if ( (location = find_attr( obj, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, obj->name, NULL );

	/* initialize the paramters for C_MK_LPP_SOURCE */

	/* source always required */
	args[2] = nim_malloc(	strlen(ATTR_LOCATION_T) + 
									strlen(obj->attrs[location].value) + 4 );
	sprintf( args[2], "-a%s=%s", ATTR_LOCATION_T, obj->attrs[location].value );

	/* execute C_MK_LPP_SOURCE */
   if ( master_exec( obj->attrs[server].value, &rc, NULL, args ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
      warning( ERR_METHOD, obj->attrs[server].value, niminfo.errstr, NULL );

	return( SUCCESS );

} /* end of ls_options */

/**************************       main         ********************************/

main( int argc, char *argv[] )
{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

   /* initialize the attr_ass LIST */
	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	/* display options info */
	ls_options();

	exit( 0 );
}

