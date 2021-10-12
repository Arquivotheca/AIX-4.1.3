static char	sccs_id[] = " @(#)15 1.6  src/bos/usr/lib/nim/methods/m_dealloc_ns.c, cmdnim, bos411, 9428A410j  7/5/94  15:20:39";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_dealloc_ns.c
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

extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{
	{0,							NULL,							FALSE,	ch_pdattr_ass}
};

char *name=NULL;									/* object name */

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

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:q")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
				break;

			case 'q': /* display valid_attrs */
				cmd_what( valid_attrs );
				exit( 0 );
			break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
				break;

			case '?': /* unknown option */
				nim_error(	ERR_BAD_OPT, optopt, 
								MSG_msg(MSG_GENERIC_ALSYNTAX), NULL );
				break;
		}
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_ALSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/*---------------------------- dealloc_nim_script            -------------------
*
* NAME: dealloc_nim_script
*
* FUNCTION:
*		deallocates a nim_script type resource
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
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= resources deallocated
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
dealloc_nim_script()

{	NIM_OBJECT( client, cinfo )
	NIM_OBJECT( res, rinfo )
	int i,j,k;
	int	ns_index = -1; 
	char *server;
	char *location;
	char *pathname;
	int rc;
	FILE *c_stdout=NULL;
	char *args[] = { RM, "-fr", NULL, NULL };

	/* get the client object */
	if ( lag_object( 0, name, &client, &cinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* make sure client has a nim_script allocated */
	if (	(i = find_attr( client, NULL, NULL, 0, ATTR_NIM_SCRIPT )) < 0 )
		/* doesn't have a nim_script - nothing else to do */
		return( SUCCESS );

	/* get the nim_script object */
	if ( lag_object( 0, ATTR_NIM_SCRIPT_T, &res, &rinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* find the nim_script's server & location */
	if (	((j = find_attr( res, NULL, NULL, 0, ATTR_SERVER )) < 0) ||
			((k = find_attr( res, NULL, NULL, 0, ATTR_LOCATION )) < 0) )
		nim_error( ERR_BAD_OBJECT, ATTR_CLASS_RESOURCES_T, res->name, NULL );
	server = res->attrs[j].value;
	location = res->attrs[k].value;

	/* construct the script's pathname (<nim_script>/<clientname>.script) */
	pathname = nim_malloc( strlen(location) + strlen(client->name) + 9 );
	sprintf( pathname, "%s/%s.script", location, client->name );

	/* Because we cache the nim script filename in an additional nim_script */
	/*	attr we need to remove any that is NOT the name of the nim_script res */
	while ( (k=find_attr(client,&ns_index,NULL,0,ATTR_NIM_SCRIPT)) != -1 )	
		if ( strcmp(client->attrs[k].value, ATTR_NIM_SCRIPT_T) != 0 )
			rm_attr( client->id, NULL, ATTR_NIM_SCRIPT, 0,client->attrs[k].value); 

	/* do the adminstrative stuff */
	if ( do_deallocation( client, res, pathname, &c_stdout ) == FAILURE ) 
		nim_error( 0, NULL, NULL, NULL );
	uaf_object( res, &rinfo, FALSE );

	VERBOSE("   removing the nim_script\n",NULL,NULL,NULL,NULL)

	/* now, actually remove the script */
	args[2] = pathname;
	if ( master_exec( server, &rc, &c_stdout, args ) == FAILURE )
		warning( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		warning( ERR_METHOD, server, niminfo.errstr, NULL );

	return( SUCCESS );

} /* end of dealloc_nim_script */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_dealloc_ns: deallocate the nim_script resource for %s\n",name,
				NULL,NULL,NULL)

	/* deallocate the nim_script */
	dealloc_nim_script();

	exit( 0 );

}
