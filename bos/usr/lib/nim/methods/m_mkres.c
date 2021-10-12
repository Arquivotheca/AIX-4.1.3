static char	sccs_id[] = " @(#)14 1.12  src/bos/usr/lib/nim/methods/m_mkres.c, cmdnim, bos411, 9428A410j  4/6/94  09:34:57";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_mkres.c
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

#define MAX_VALID_ATTRS				4
VALID_ATTR valid_attrs[] =
{	{ATTR_SERVER,				ATTR_SERVER_T,				TRUE,		valid_pdattr_ass},
	{ATTR_LOCATION,			ATTR_LOCATION_T,			TRUE,		valid_pdattr_ass},
 	{ATTR_COMMENTS,			ATTR_COMMENTS_T,			FALSE,	valid_pdattr_ass},
 	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
 	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE,	NULL}
};

char *name=NULL;								/* NIM name of object to create */
char *type=NULL;								/* object's type */
char *location=NULL;							/* filesystem location of resource */
NIM_PDATTR( pdattr, pinfo )				/* nim_pdattr for <type> */
	
/*---------------------------- ck_attrs           -------------------------
*
* NAME: ck_attrs
*
* FUNCTION:
*		checks to make sure that the information supplied by user is sufficient
*			to complete object definition
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
*			valid_attrs
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= nothing missing
*		FAILURE					= definition incomplete
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ck_attrs ()

{	int i,j;

	/* check for required attrs */
	for (i=0; i < MAX_VALID_ATTRS; i++)
		if ( valid_attrs[i].required )
		{	for (j=0; j < attr_ass.num; j++)
				if ( attr_ass.list[j]->pdattr == valid_attrs[i].pdattr )
					break;

			/* required attr present? */
			if ( j == attr_ass.num )
				nim_error( ERR_MISSING_ATTR, valid_attrs[i].name, NULL, NULL );
			else if ( attr_ass.list[j]->pdattr == ATTR_LOCATION )
				location = attr_ass.list[j]->value;
		}

	return( SUCCESS );

} /* end of ck_attrs */
	
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
	while ( (c = getopt(argc, argv, "a:t:q")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
				break;

			case 't': /* object type */
				if ( type != NULL )
					syntax_err = TRUE;
				else if ( get_pdattr(	&pdattr, &pinfo, ATTR_CLASS_RESOURCES,
												ATTR_SUBCLASS_TYPE, 0, 0, optarg ) <= 0 )
					nim_error(	ERR_BAD_TYPE_FOR, optarg, 
									ATTR_CLASS_RESOURCES_T, NULL );
				else
					type = optarg;
				break;

			case 'q': /* display valid_attrs */
				mstr_what( valid_attrs, NULL );
				exit( 0 );
			break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
				break;

			case '?': /* unknown option */
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_M_MKRES_SYNTAX), NULL );
				break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_M_MKRES_SYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_M_MKRES_SYNTAX), NULL, NULL );
	else if ( type == NULL )
		nim_error( ERR_MISSING_OPT, 't', NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/*---------------------------- undo              ------------------------------
*
* NAME: undo
*
* FUNCTION:
*		backs out database changes
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
*			id							= id of new object
*			errno						= error code (if not already set)
*			str1						= str1 of error msg
*			str2						= str2 of error msg
*			str3						= str3 of error msg
*		global:
*
* RETURNS: (int)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
undo(	long id,
		int errno,
		char *str1,
		char *str2,
		char *str3 )

{
	if ( errno > 0 )
		errstr( errno, str1, str2, str3 );

	protect_errstr = TRUE;

	/* remove the newly created object */
	rm_object( id, NULL );

	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */
	
/*---------------------------- define_res        ------------------------------
*
* NAME: define_res
*
* FUNCTION:
*		"defines" a resource (ie, allows it to join the NIM environment)
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
*		global:
*
* RETURNS: (int)
*		SUCCESS					= resource defined and exported for use
*		FAILURE					= error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
define_res()

{	long id;
	char *server;

	/* create a new resource object */
	if ( (id = mk_robj( name, type, &server, TRUE, &attr_ass )) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* make the resource available */
	finish_robj( name, server );

	return( SUCCESS );

} /* end of define_res */

/**************************       main         ********************************/

main( int argc, char *argv[] )
{	
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( ! get_list_space( &attr_ass, MAX_VALID_ATTRS, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_mkres: define the %s resource\n",name,NULL,NULL,NULL)

	/* check for missing attrs */
	ck_attrs();

	/* define the resource */
	define_res();

	exit( 0 );
}
