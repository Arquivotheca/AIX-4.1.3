static char	sccs_id[] = " @(#)27 1.13  src/bos/usr/lib/nim/methods/m_mkpdir.c, cmdnim, bos411, 9428A410j  5/26/94  16:21:42";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ck_attrs
 *		main
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

extern int	valid_pdattr_ass();
extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

#define MAX_VALID_ATTRS				4
VALID_ATTR valid_attrs[] = 
{	
	{ ATTR_SERVER, 			ATTR_SERVER_T,				TRUE,		valid_pdattr_ass },
	{ ATTR_LOCATION, 			ATTR_LOCATION_T, 			TRUE,		valid_pdattr_ass },
	{ ATTR_COMMENTS, 			ATTR_COMMENTS_T, 			FALSE, 	valid_pdattr_ass },
 	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
 	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{ 0, 							NULL, 						FALSE, 	NULL }
};


char	*name = NULL;		/* NIM name of object to create */
char	*type = NULL;		/* object's type */
char	*server = NULL;		/* ptr to name of server */
char	*location = NULL;	/* ptr to target location for this resource */

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

{	
	int	i, j;

	/* check for required attrs */
	for (i = 0; i < MAX_VALID_ATTRS; i++)
		if ( valid_attrs[i].required ) {	
			for (j = 0; j < attr_ass.num; j++)
				if ( attr_ass.list[j]->pdattr == valid_attrs[i].pdattr )
					break;

			/* required attr present? */
			if ( j == attr_ass.num )
				nim_error( ERR_MISSING_ATTR, valid_attrs[i].name, NULL, NULL );
			else if ( attr_ass.list[j]->pdattr == ATTR_SERVER )
				server = attr_ass.list[j]->value;
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

{	
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;
	int	syntax_err = FALSE;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:t:q")) != -1 ) {	
		switch (c) {
		case 'a': /* attribute assignment */
			if (!parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
				nim_error( 0, NULL, NULL, NULL );
			break;

		case 't': /* object type */
			if ( type != NULL )
				syntax_err = TRUE;
			else if ( get_pdattr(	NULL, NULL, ATTR_CLASS_RESOURCES,
			    ATTR_SUBCLASS_PDIR_TYPE, 0, 0, optarg ) <= 0 )
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
			nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_GENERIC_MKSYNTAX), NULL );
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_MKSYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc - 1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_MKSYNTAX), NULL, NULL );
	else if ( type == NULL )
		nim_error( ERR_MISSING_OPT, 't', NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/*---------------------------- mk_pdir           ------------------------------
*
* NAME: mk_pdir
*
* FUNCTION:
*		creates a parent directory resource
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
*			name
*			type
*
* RETURNS: (int)
*		SUCCESS					= pdir resource created
*		FAILURE					= error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mk_pdir()

{	long id;
	FILE 	*c_stdout=NULL;
	char path[MAX_TMP];
	char	*Args[] = { C_MKDIR, path, NULL };
	int rc;
	char *tmp;

	/* create a new resource object */
	if ( (id = mk_robj( name, type, &server, FALSE, &attr_ass )) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	VERBOSE("   creating the %s:%s directory\n",server,location,NULL,NULL)

	/* create the actual directory */
	/* NOTE that we're not NFS exporting this directory.  This is done because */
	/*		of the way NFS works.  NFS will not allow subdirectories of an */
	/*		exported directory to be exported.  The "pdir" type of resource is */
	/*		used to create subdirectories in (one for each client) and these */
	/*		subdirectories need to be exported, so we can't export the parent dir*/
	sprintf( path, "-a%s=%s", ATTR_LOCATION_T, location );
	if (	(master_exec(server, &rc, &c_stdout, Args) == FAILURE) ||
			(rc > 0) )
	{	/* remove the new object */
		tmp = niminfo.errstr;
		niminfo.errstr = NULL;
		rm_object( id, name );
		nim_error( ERR_METHOD, server, tmp, NULL );
	}

	/* any attrs returned from the method? */
	/* any error encountered here is NOT fatal, so ignore it */
	attrs_from_FILE( id, c_stdout );
	fclose( c_stdout );

	/* make the resource available */
	finish_robj( name, server );

	return( SUCCESS );

} /* end of mk_pdir */

/**************************       main         ********************************/

main(	int argc,
		char *argv[] )

{	
	long	id;
	char	*server;
	int	rc;

	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( !get_list_space( &attr_ass, MAX_VALID_ATTRS, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_mkpdir: define the %s parent-directory type resource\n",name,
				NULL,NULL,NULL)

	/* check for missing attrs */
	ck_attrs();

	/* make the resource */
	mk_pdir();

	exit( 0 );
}
