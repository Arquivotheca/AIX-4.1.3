static char	sccs_id[] = " @(#)74 1.9  src/bos/usr/lib/nim/methods/m_mkbosi.c, cmdnim, bos411, 9428A410j  4/6/94  09:39:12";


/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: NIM_PDATTR
 *		ck_attrs
 *		parse_args
 *		undo
 *		define_res
 *		main
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
extern int	ch_pdattr_ass();

/* --------------------------- module globals    ---------------------------- */
ATTR_ASS_LIST attr_ass;

#define MAX_VALID_ATTRS				4
VALID_ATTR valid_attrs[] = 
{	
	{ ATTR_SERVER, 		ATTR_SERVER_T, 		TRUE, 	valid_pdattr_ass },
	{ ATTR_LOCATION, 		ATTR_LOCATION_T, 		TRUE, 	valid_pdattr_ass },
	{ ATTR_COMMENTS, 		ATTR_COMMENTS_T, 		FALSE, 	valid_pdattr_ass },
	{ ATTR_FORCE, 			ATTR_FORCE_T, 			FALSE, 	ch_pdattr_ass },
	{ ATTR_VERBOSE, 		ATTR_VERBOSE_T, 		FALSE, 	ch_pdattr_ass },
	{ 0, 						NULL, 					FALSE, 	NULL }
};

NIM_OBJECT( newobj, newinfo )		/* the newly created NIM object for <name> */
char *name = NULL; 					/* NIM name of BOS image */
char *type=ATTR_RTE_T;				/* default is BOS rte */
LIST params_list;                /* LIST of parameters for methods */

/* --------------------------- ck_attrs           -------------------------
 *
 * NAME: ck_attrs
 *
 * FUNCTION:
 *	checks to make sure that the information supplied by user is sufficient
 *	to complete object definition
 *
 * NOTES:
 *		calls nim_error
 *
 * DATA STRUCTURES:
 *		parameters:
 *		global:
 *			valid_attrs
 *			attr_ass
 *
 * RETURNS: (int)
 *	SUCCESS		= nothing missing
 *	FAILURE		= definition incomplete
 *
 * -------------------------------------------------------------------------- */

int
ck_attrs()

{	
	int	i, j;
	int	found;
	regmatch_t device[ERE_DEVICE_NUM];

	/*
	 * check for required attrs
	 */
	for (i = 0; i < MAX_VALID_ATTRS; i++)
		if (valid_attrs[i].required) 
		{	for (j = 0; j < attr_ass.num; j++)
				if (attr_ass.list[j]->pdattr == valid_attrs[i].pdattr)
					break;
			/*
			 * required attr present?
 	 		 */
			if (j == attr_ass.num)
				nim_error (ERR_MISSING_ATTR, valid_attrs[i].name, NULL, NULL);
			else if ( attr_ass.list[j]->pdattr == ATTR_LOCATION )
			{
				/* verify that the location is NOT a /dev entry */
				/* currently, we cannot allow registration of local devices */
				/*		because we have no method to access, and synchronize access */
				/*		to, remote devices */
				if ( regexec(	nimere[DEVICE_ERE].reg, attr_ass.list[j]->value,
									ERE_DEVICE_NUM, device, 0 ) == 0 )
					nim_error(	ERR_VALUE_CONTEXT, attr_ass.list[j]->value, 
									attr_ass.list[j]->name, NULL );
			}

		}
	
	return (SUCCESS);

} /* end of ck_attrs */

/* --------------------------- parse_args        ------------------------------
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
 * -------------------------------------------------------------------------- */

int
parse_args ( 	int argc, 
					char *argv[] )

{
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;

	/*
	 * loop through all args
	 */
	while ((c = getopt (argc, argv, "a:t:q")) != -1) 
	{
		switch (c)
		{
			case 'a': /* attribute assignment */
				if (!parse_attr_ass (&attr_ass, valid_attrs, optarg, FALSE))
					nim_error (0, NULL, NULL, NULL);
			break;

			case 't': /* object type */
            if ( get_pdattr(	NULL, NULL, ATTR_CLASS_RESOURCES,
										ATTR_SUBCLASS_BOS, 0, 0, optarg ) <= 0 )
               nim_error(  ERR_BAD_TYPE_FOR, optarg,
                           ATTR_CLASS_RESOURCES_T, NULL );
				else
					type = optarg;
			break;

			case 'q': /* display valid_attrs */
				mstr_what( valid_attrs, NULL );
				exit( 0 );
			break;

			case ':': /* option is missing a required argument */
				nim_error (ERR_MISSING_OPT_ARG, optopt, NULL, NULL);
			break;

			case '?': /* unknown option */
				nim_error (ERR_BAD_OPT, optopt, MSG_msg (MSG_M_MKRES_SYNTAX), NULL);
			break;
		}
	}

	/*
	 * check for errors 
	 */
	if (optind == argc)
		nim_error (ERR_MISSING_OPERAND, MSG_msg (MSG_OBJ_NAME), NULL, NULL);
	else if (optind < (argc - 1))
		nim_error (ERR_SYNTAX, MSG_msg (MSG_M_MKRES_SYNTAX), NULL, NULL);
	else if (*type == NULL)
		nim_error (ERR_MISSING_OPT, 't', NULL, NULL);

 	name = argv[optind];

	return (SUCCESS);

} /* end of parse_args */

/* --------------------------- undo              ------------------------------
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
 *	parameters:
 *		errno	= error code (if not already set)
 *		str1	= str1 of error msg
 *		str2	= str2 of error msg
 *		str3	= str3 of error msg
 *	global:
 *
 * RETURNS: (int)
 *
 * -------------------------------------------------------------------------- */

int
undo( int errno,
		char *str1,
		char *str2,
		char *str3 )

{
	if (errno > 0)
		errstr( errno, str1, str2, str3 );

	protect_errstr = TRUE;

	/* remove the newly created object */
	rm_robj( newobj );

	/* display err & exit */
	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */

/* --------------------------- define_res        ------------------------------
 *
 * NAME: define_res
 *
 * FUNCTION:
 *		"defines" a resource (ie, allows it to join the NIM environment)
 *
 * NOTES:
 *		calls nim_error on failure
 *
 * DATA STRUCTURES:
 *		parameters:
 *		global:
 *			attr_ass
 *			newobj
 *			name
 *			type
 *
 * RETURNS: (int)
 *		SUCCESS		= resource defined and exported for use
 *
 * -------------------------------------------------------------------------- */

int
define_res()

{	long	id;
	char	*server;
	char *location = NULL;
	int i;
	int rc;
	FILE *c_stdout = NULL;
	char params[MAX_VALUE];

	/* create a new resource object */
	if ( (id = mk_robj( name, type, &server, TRUE, &attr_ass )) == FAILURE )
		nim_error (0, NULL, NULL, NULL);

	/* now get it */
	if ( lag_object( id, NULL, &newobj, &newinfo ) == FAILURE )
      undo( 0, NULL, NULL, NULL );

	/* find the location */
   if ( (i = find_attr( newobj, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
		undo( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, name, NULL );
   location = newobj->attrs[i].value;

   /* initialize the parameters to C_CKLEVEL */
   if ( add_to_LIST( &params_list, C_CKLEVEL ) == FAILURE )
      undo( 0, NULL, NULL, NULL );

   /* source location */
   sprintf( params, "-a%s=%s", ATTR_SOURCE_T, location );
   if ( add_to_LIST( &params_list, params ) == FAILURE )
      undo( 0, NULL, NULL, NULL );

   /* source type */
   sprintf( params, "-a%s=%s", ATTR_TYPE_T, type );
   if ( add_to_LIST( &params_list, params ) == FAILURE )
      undo( 0, NULL, NULL, NULL );

	VERBOSE("   verifying the release and version level - please wait\n",
				NULL,NULL,NULL,NULL)

	/* execute the C_CKLEVEL method to check the version/release level */
   if ( master_exec( server, &rc, &c_stdout, &params_list.list[0] ) == FAILURE )
      undo( 0, NULL, NULL, NULL );
   else if ( rc > 0 )
      undo( ERR_METHOD, server, niminfo.errstr, NULL );

	/* check for any attr assignments on the stdout */
   if ( attrs_from_FILE( id, c_stdout) != SUCCESS )
      undo( 0, NULL, NULL, NULL );

	/* make the resource available */
   finish_robj( name, server );

	return (SUCCESS);

} /* end of define_res */

/* --------------------------------  main  ---------------------------------- */

main(	int argc,
		char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	/* initialize attr ass LIST */
	if ( ! get_list_space( &attr_ass, MAX_VALID_ATTRS, TRUE ) )
		nim_error (0, NULL, NULL, NULL);

   /* initialize the parameter LIST */
   if ( ! get_list_space( &params_list, DEFAULT_CHUNK_SIZE, TRUE ) )
      nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_mkbosi: define the %s BOS image resource\n",name,NULL,NULL,NULL)

	/* check for missing attrs */
	ck_attrs();

	/* define the resource */
	define_res();

	/* all done & successful */
	exit( 0 );
}
