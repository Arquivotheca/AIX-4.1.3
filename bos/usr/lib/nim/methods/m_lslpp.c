static char	sccs_id[] = " @(#)29 1.7.1.1  src/bos/usr/lib/nim/methods/m_lslpp.c, cmdnim, bos411, 9430C411a  7/22/94  11:39:22";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_lslpp.c
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
#include <swvpd.h>

extern int valid_pdattr_ass();
extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{  
	{ATTR_LSLPP_FLAGS,   ATTR_LSLPP_FLAGS_T,  FALSE,   valid_pdattr_ass},
	{ATTR_FORCE,   		ATTR_FORCE_T,  		FALSE,   ch_pdattr_ass},
	{ATTR_VERBOSE,   		ATTR_VERBOSE_T,  		FALSE,   ch_pdattr_ass},
   {0,						NULL,						FALSE,   NULL}
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

/*---------------------------- lslpp             --------------------------
*
* NAME: lslpp
*
* FUNCTION:
*		retrieves LPP info from the specified object (must be either a
*			standalone machine or a SPOT)
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
*		LPP info written to stdout
*-----------------------------------------------------------------------------*/

int
lslpp()

{	NIM_OBJECT( obj, info )
	int i;
	char *location;
	char *server;
	int rc;
	FILE *c_stdout=NULL;
	char st_applied[MAX_TMP];
	char st_committed[MAX_TMP];
	char *pathname=NULL;
	char objname[MAX_TMP];
	char flags[MAX_TMP];
	char *args[] = { C_CKSPOT, "-l", st_applied, st_committed, NULL, NULL, NULL, 
								NULL, NULL };
	char line[MAX_VALUE];
	int argcnt = 3;

	/* get all the info about this object */
	if ( lag_object( 0, name, &obj, &info ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* initialize the paramters for C_CKSPOT */

	/* values of ST_APPLIED and ST_COMMITTED, which are used in the */
	/*		ck_spot_options function, which is called by C_CKSPOT */
	sprintf( st_applied, "-a%s=%d", ATTR_ST_APPLIED_T, ST_APPLIED );
	sprintf( st_committed, "-a%s=%d", ATTR_ST_COMMITTED_T, ST_COMMITTED );

	/* ATTR_PLATFORM is required, but not used in this case, so we can pass */
	/*		anything, as along as it's non-NULL */
	args[ ++argcnt ] = nim_malloc( strlen(ATTR_PLATFORM_T) + 7 );
	sprintf( args[ argcnt ], "-a%s=yes", ATTR_PLATFORM_T );

	/* object's name */
	sprintf( objname, "-a%s=%s", ATTR_NAME_T, name );
	args[ ++argcnt ] = objname;

	/* any flags specified? */
	for (i=0; i < attr_ass.num; i++)
		if ( attr_ass.list[i]->pdattr == ATTR_LSLPP_FLAGS )
		{
			sprintf(	flags, "-a%s=%s", ATTR_LSLPP_FLAGS_T, 
						attr_ass.list[i]->value );
			args[ ++argcnt ] = flags;
			break;
		}

	/* what type of object? */
	if ( obj->type->attr == ATTR_SPOT )
	{	
		/* get SPOT's location */
   	if ( (i = find_attr( obj, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
			nim_error( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, name, NULL );
		location = obj->attrs[i].value;
		pathname = nim_malloc( strlen(ATTR_LOCATION_T) + strlen(location) + 4 );
		sprintf( pathname, "-a%s=%s", ATTR_LOCATION_T, location );

		/* get the SPOT server's name */
   	if ( (i = find_attr( obj, NULL, NULL, 0, ATTR_SERVER )) < 0 )
			nim_error( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, name, NULL );
		server = obj->attrs[i].value;

	}
	else if (	(obj->type->attr == ATTR_STANDALONE) ||
					(obj->type->attr == ATTR_MASTER) )
	{
		/* location is always /usr */
		pathname = nim_malloc( strlen(ATTR_LOCATION_T) + 8 );
		sprintf( pathname, "-a%s=/usr", ATTR_LOCATION_T );

		/* server is always the machine itself */
		server = name;

	}
	else
		nim_error(	ERR_OP_NOT_ALLOWED, ATTR_LSLPP_T, ATTR_msg(obj->type->attr),
						NULL );
	args[ ++argcnt ] = pathname;

	/* execute C_CKSPOT */
   if (  (master_exec(  server, &rc, NULL, &args[0] ) == FAILURE) ||
         (rc > 0) )
      warning( ERR_METHOD, server, niminfo.errstr, NULL );

	return( SUCCESS );

} /* end of lslpp */

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

	VERBOSE("m_lslpp: use lslpp to display option information for %s\n",name,
				NULL,NULL,NULL)

	/* display the lslpp info */
	lslpp();

	exit( 0 );
}
