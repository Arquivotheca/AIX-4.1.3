static char	sccs_id[] = " @(#)03 1.5  src/bos/usr/lib/nim/methods/m_rmspot.c, cmdnim, bos411, 9428A410j  4/6/94  09:43:11";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_rmspot.c
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

/* --------------------------- module globals    ---------------------------- */
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
 	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
 	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE, NULL}
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

/*---------------------------- rm_spot             -----------------------------
*
* NAME: rmspot
*
* FUNCTION:
*		removes a SPOT & its associated NIM objects
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
*		SUCCESS					= object removed
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
rm_spot()

{	NIM_OBJECT( spot, sinfo )
	int i;
	char *location;
	char *server;
	int rc;
	FILE *c_stdout=NULL;
	char pathname[MAX_VALUE];
	char spotname[MAX_TMP];
	char exported[MAX_TMP];
	char *args[] = { C_RMSPOT, pathname, spotname, exported, NULL };

	/* get all the info about this SPOT */
	if ( lag_object( 0, name, &spot, &sinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* make sure this is really a SPOT type resource */
	if ( spot->type->attr != ATTR_SPOT )
		nim_error(	ERR_OP_NOT_ALLOWED, "m_rmspot", ATTR_msg(spot->type->attr),
						NULL );

	VERBOSE("   verifying that this resource may be removed at this time\n",
				NULL,NULL,NULL,NULL)

	/* ok to remove this resource? */
	if ( ok_to_rm_robj( spot ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* remove the NIM objects for the SPOT */
	if ( rm_robj( spot ) == FAILURE ) 
		nim_error( 0, NULL, NULL, NULL );

	/* initialize the paramters for C_RMSPOT */

	/* SPOT's location */
   if ( (i = find_attr( spot, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
	{	warning( ERR_BAD_OBJECT, ATTR_SPOT_T, name, NULL );
		return( SUCCESS );
	}
	location = spot->attrs[i].value;
	sprintf( pathname, "-a%s=%s", ATTR_LOCATION_T, location );

	/* SPOT's name */
	sprintf( spotname, "-a%s=%s", ATTR_NAME_T, name );

	/* look for ATTR_EXPORTED - if present, then we need to unexport the SPOT */
   if ( (i = find_attr( spot, NULL, NULL, 0, ATTR_EXPORTED )) >= 0 )
		sprintf( exported, "-a%s=%s", ATTR_EXPORTED_T, spot->attrs[i].value );
	else
		exported[0] = NULL_BYTE;

	/* get the server's name */
   if ( (i = find_attr( spot, NULL, NULL, 0, ATTR_SERVER )) < 0 )
	{	warning( ERR_RMRES, ATTR_SPOT_T, location, NULL );
		return( SUCCESS );
	}
	server = spot->attrs[i].value;

	VERBOSE("   removing the SPOT at location %s:%s\n",server,location,NULL,
				NULL)

	/* execute C_RMSPOT on the SPOT's server */
	/* NOTE that C_RMSPOT will handle unexporting the SPOT */
   if (  (master_exec(  server, &rc, &c_stdout, &args[0] ) == FAILURE) ||
         (rc > 0) )
      warning( ERR_METHOD, server, niminfo.errstr, NULL );

	return( SUCCESS );

} /* end of rm_spot */

/**************************       main         ********************************/

main( int argc, char *argv[] )
{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
   if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
      nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_rmspot: remove the %s SPOT\n",name,NULL,NULL,NULL)

	/* remove the specified SPOT */
	rm_spot();

	exit( 0 );
}
