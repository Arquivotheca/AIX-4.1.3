static char	sccs_id[] = " @(#)53 1.8  src/bos/usr/lib/nim/methods/m_rmpdir.c, cmdnim, bos411, 9428A410j  4/6/94  09:41:00";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: main
 *		parse_args
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cmdnim_mstr.h"

extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE,	ch_pdattr_ass}
};

char *name=NULL;										/* object's NIM name */
NIM_OBJECT( robj, rinfo )							/* NIM object for <name> */

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
*		SUCCESS				= no syntax errors on command line
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
	while ( (c = getopt(argc, argv, "q")) != -1 )
	{	switch (c)
		{	
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
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_RMSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/*---------------------------- rm_pdir           ------------------------------
*
* NAME: rm_pdir
*
* FUNCTION:
*		remove the specified parent directory type resource
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			x						= x
*		global:
*			robj
*
* RETURNS: (int)
*		SUCCESS					= x
*		FAILURE					= x
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rm_pdir()

{	int rc;
	FILE *c_stdout=NULL;
	int i,j;
	char *server;
	char *location;
	char params[MAX_TMP];
	char *Args[] = { C_RMDIR, params, NULL };
	int dir_created;

	/* find the server & location */
	if (	((i = find_attr( robj, NULL, NULL, 0, ATTR_SERVER )) < 0) ||
			((j = find_attr( robj, NULL, NULL, 0, ATTR_LOCATION )) < 0) )
		nim_error( ERR_BAD_OBJECT, ATTR_msg(ATTR_CLASS_RESOURCES),
						robj->name, NULL );
	server = robj->attrs[i].value;
	location = robj->attrs[j].value;

	VERBOSE("   verifying that this resource may be removed at this time\n",
				NULL,NULL,NULL,NULL)

	/* ok to remove this resource? */
	if ( ok_to_rm_robj( robj ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* remove the directory? */
	dir_created = find_attr( robj, NULL, NULL, 0, ATTR_DIR_CREATED );
	if ( dir_created >= 0 )
	{
		/* NIM created the directory - remove it now */
		sprintf( params, "-a%s=%s", ATTR_LOCATION_T, 
					robj->attrs[dir_created].value );
		if (	(master_exec( server, &rc, &c_stdout, Args ) == FAILURE) || 
				(rc > 0) )
			nim_error( 0, NULL, NULL, NULL );
	}

	/* remove the resource */
	if ( rm_robj( robj ) == FAILURE ) 
		nim_error( 0, NULL, NULL, NULL );

	return( SUCCESS );

} /* end of rm_pdir */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
   if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
      nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_rmpdir: remove the %s parent-directory type resource\n",name,
				NULL,NULL,NULL)

	/* lock-and-get this object */
	if ( lag_object( 0, name, &robj, &rinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* remove the resource */
	rm_pdir();

	exit( 0 );
}
