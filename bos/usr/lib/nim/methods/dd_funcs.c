/* @(#)80	1.12  src/bos/usr/lib/nim/methods/dd_funcs.c, cmdnim, bos411, 9439C411f  9/30/94  17:35:36 */
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/dd_funcs.c
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

#include "cmdnim.h"
#include "cmdnim_db.h"

/*------------------------ externs */
extern ATTR_ASS_LIST attr_ass;
extern VALID_ATTR valid_attrs[];
extern VALID_ATTR resources[];
extern char *name;							/* NIM name of object to create */
extern struct nim_object *mobj;			/* object being operated on */
extern struct nim_if nimif;				/* client's primary interface info */
extern LIST allocd;							/* res_access LIST for alloc'd res */
extern LIST params;							/* LIST of parameters for methods */
extern char tmp_param[MAX_VALUE];		/* tmp space for parameters */

/* info for undo */
extern struct res_access *root;
extern struct res_access *paging;
extern struct res_access *dump;
extern int root_created;
extern int paging_created;
extern int dump_created;
extern int boot_created;


/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ common functions for diskless/dataless manipulation 
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

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
	for (i=0; valid_attrs[i].name != NULL; i++)
		if ( valid_attrs[i].required )
		{	for (j=0; j < attr_ass.num; j++)
				if ( attr_ass.list[j]->pdattr == valid_attrs[i].pdattr )
					break;

			/* required attr present? */
			if ( j == attr_ass.num )
				nim_error( ERR_MISSING_ATTR, valid_attrs[i].name, NULL, NULL );
		}

	return( SUCCESS );

} /* end of ck_attrs */
	
/*---------------------------- res_query         ------------------------------
*
* NAME: res_query
*
* FUNCTION:
*		displays the required and optional resources required by this operation
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*		global:
*			resources
*			resources
*
* RETURNS: (int)
*		exits with SUCCESS
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
res_query()

{	int i;

	printf("\n%s\n", MSG_msg(MSG_REQUIRED_RES));
	for (i=0; resources[i].name != NULL; i++)
		if ( resources[i].required )
			printf("\t%s\n", ATTR_msg(resources[i].pdattr));

	printf("\n%s\n", MSG_msg(MSG_OPTIONAL_RES));
	for (i=0; resources[i].name != NULL; i++)
		if ( ! resources[i].required )
			printf("\t%s\n", ATTR_msg(resources[i].pdattr));

	exit( 0 );

} /* end of res_query */
	
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

	/* check for resource query */
	if ( ! strcmp( argv[1], "-q" ))
		res_query();

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:q")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
				break;

			case 'q': /* display valid_attrs */
				mstr_what( valid_attrs, resources );
				exit( 0 );
			break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
				break;

			case '?': /* unknown option */
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_COP_SYNTAX), NULL );
				break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_COP_SYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_COP_SYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/*---------------------------- ok_to_dd_inst    ------------------------------
*
* NAME: ok_to_dd_inst
*
* FUNCTION:
*		determines whether the specified machine can be installed in the
*			specified configuration
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
*			type					= type of configuration
*			op						= name of operation
*		global:
*			mobj
*			nimif					= nim_if struct
*			allocd
*
* RETURNS: (int)
*		SUCCESS					= ok to install the machine
*		FAILURE					= not ok - something missing or wrong state or ...
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_dd_inst(	int type,
					char *op )

{	int rc = SUCCESS;
	int i;
	char *msg = NULL;
	char *info = NULL;

	/* configuration type must be diskless or dataless */
	if ( mobj->type->attr != type )
		ERROR(	ERR_OP_NOT_ALLOWED, op, ATTR_msg(type), NULL )

	/* check the Cstate - it must be "ready" */
	if ( (i = find_attr( mobj, NULL, NULL, 0, ATTR_CSTATE )) < 0 )
		ERROR( ERR_BAD_OBJECT, ATTR_msg(ATTR_CLASS_MACHINES), mobj->name, NULL )
	if ( ! same_state( mobj->attrs[i].value, STATE_CREADY ) )
		ERROR( ERR_STATE, mobj->name, NULL, NULL )

	/* remove any missing attrs laying around - ignore the return code because */
	/*		it doesn't really matter whether they get deleted or not */
	rm_attr( mobj->id, NULL, ATTR_MISSING, 0, NULL );
	rm_attr( mobj->id, NULL, ATTR_INFO, 0, NULL );
	rm_attr( mobj->id, NULL, ATTR_BOOT_INFO, 0, NULL );
	rm_attr( mobj->id, NULL, ATTR_CRESULT, 0, NULL );
	rm_attr( mobj->id, NULL, ATTR_TRANS, 0, NULL );

	/* we need to check the allocated resources, so generate the list now */
	if ( LIST_res_access( mobj, &allocd ) == FAILURE )
		return( FAILURE );

	/* verify that all the required resources have been allocated */
	for (i=0; resources[i].name != NULL; i++)
		if (	(resources[i].required) &&
				(find_res_access( resources[i].pdattr, &allocd ) < 0) )
		{	if ( rc == SUCCESS )
			{	/* first error - print message */
				nene( ERR_MISSING_RES, mobj->name, NULL, NULL );
				protect_errstr = TRUE;
				msg = MSG_msg( MSG_MISSING_RES );
				info = nim_malloc( strlen(msg) + strlen(op) + 1 );
				sprintf( info, msg, op );
				mk_attr( mobj->id, NULL, info, 0, ATTR_INFO, ATTR_INFO_T);
			}
			rc = FAILURE;
			mk_attr(	mobj->id, NULL, resources[i].name, 0, 
						ATTR_MISSING, ATTR_MISSING_T);
			fprintf( stderr, "\t%s\n", resources[i].name );
		}

	/* get the target's PIF info */
	if ( (i = find_attr( mobj, NULL, NULL, 1, ATTR_IF )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, PIF, mobj->name, NULL )
	else if ( s2nim_if( mobj->attrs[i].value, &nimif ) == FAILURE )
		ERROR( ERR_NET_INFO, PIF, mobj->name, NULL )

	return( rc );

} /* end of ok_to_dd_inst */
	
/*---------------------------- undo              ------------------------------
*
* NAME: undo    
*
* FUNCTION:
*		undoes operations performed in m_dkls_inst or m_dtls_inst
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
*			errno					= error code
*			str1					= str1 of error msg
*			str2					= str2 of error msg
*			str3					= str3 of error msg
*		global:
*			mobj
*			allocd
*
* RETURNS: (int)
*		SUCCESS					= ok to install the machine
*		FAILURE					= not ok - something missing or wrong state or ...
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

void
undo(	int errno,
		char *str1,
		char *str2,
		char *str3 )

{	FILE *c_stdout = NULL;

	disable_err_sig();

	/* errno given? */
	if ( errno > 0 )
		errstr( errno, str1, str2, str3 );

	/* protect the current errstr */
	protect_errstr = TRUE;

	/* deallocate the boot resource */
	if (dealloc_res( mobj, ATTR_BOOT_T, &c_stdout) == SUCCESS )
		/* reset the Cstate */
		Cresult( mobj->name, RESULT_FAILURE );

	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */
	
/*---------------------------- add_params        ------------------------------
*
* NAME: add_params
*
* FUNCTION:
*		adds the specified parameter to the params LIST
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls undo no failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			format				= format for sprintf
*			str1					= first field
*			str2					= 2nd field
*			str3					= 3rd field
*		global:
*
* RETURNS: (int)
*		SUCCESS
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
add_params(	char *format,
				char *str1,
				char *str2,
				char *str3 )

{
	sprintf( tmp_param, format, str1, str2, str3 );
	if ( add_to_LIST( &params, tmp_param ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	return( SUCCESS );

} /* end of add_params */

/*---------------------------- mk_root            ------------------------------
*
* NAME: mk_root  
*
* FUNCTION:
*		initializes the client's root directory
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
*			mobj					= ptr to nim_object struct
*			root					= ptr to res_access struct
*			params				= LIST to use for parameters
*			allocd			= LIST of res_access structs
*
* RETURNS: (int)
*		SUCCESS					= root initialized
*		FAILURE					= not ok - something missing or wrong state or ...
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mk_root()

{	int i;
	struct res_access *raccess;
	int rc;
	FILE *c_stdout = NULL;
	char *tmp = NULL;
	char *ring_speed = NULL;
	char *cable_type = NULL;
	NIM_ATTR( boot_info, binfo )
	int local_res=FALSE;

	/* reset the LIST */
	reset_LIST( &params );

	/* get root location */
	if ( (i = find_res_access( ATTR_ROOT, &allocd )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_ROOT_T, mobj->name, NULL )
	root = (struct res_access *) allocd.list[i];

	/* get the client's boot info */
	if ( get_attr( &boot_info, &binfo, mobj->id, NULL, 0, ATTR_BOOT_INFO ) <= 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_BOOT_INFO_T, mobj->name, NULL )

	/* is the primary interface a token-ring? */
	if ( (i = find_attr( mobj, NULL, NULL, 0, ATTR_RING_SPEED )) >= 0 )
		ring_speed = mobj->attrs[i].value;

	/* has cable_type been specified? */
	if ( (i = find_attr( mobj, NULL, NULL, 0, ATTR_CABLE_TYPE )) >= 0 )
		cable_type = mobj->attrs[i].value;

	/* build the params LIST */
	add_params( "%s", C_MKROOT, NULL, NULL );
	add_params( "-a%s=%s", ATTR_NAME_T, mobj->name, NULL );
	add_params( "-a%s=%s", ATTR_TYPE_T, mobj->type->name, NULL );
	add_params( "-a%s=%s", ATTR_HOSTNAME_T, nimif.hostname, NULL );
	add_params( "-a%s=%s", ATTR_BOOT_INFO_T, boot_info->value, NULL );
	if ( ring_speed != NULL )
		add_params( "-a%s=%s", ATTR_RING_SPEED_T, ring_speed, NULL );
	if ( cable_type != NULL )
		add_params( "-a%s=%s", ATTR_CABLE_TYPE_T, cable_type, NULL );
	add_params( "-a%s=%s", ATTR_ADPT_NAME_T, nimif.adapter, NULL );
	add_params( "-a%s=%s", ATTR_SERVER_T, root->nimif.hostname, NULL );
	for (i=0; i < allocd.num; i++)
	{	
		raccess = (struct res_access *) allocd.list[i];

		/* is resource local or remote? */
		local_res = ( strcmp( raccess->server, root->server ) == 0 );

		/* what type of resource? */
		switch (raccess->type)
		{
			case ATTR_MASTER:
				add_params( "-a%s=%s", ATTR_MASTER_T, raccess->nimif.hostname,NULL);
			break;

			case ATTR_SHARED_HOME:
				/* C_MKROOT doesn't know what shared_home is, so just pass as if */
				/*		it was "home" */
				if ( local_res )
					add_params( "-a%s=%s", ATTR_HOME_T, raccess->location, NULL );
				else
					add_params( "-a%s=%s:%s", ATTR_HOME_T,
									raccess->nimif.hostname, raccess->location );
			break;

			default:
				if ( local_res )
					add_params( "-a%s=%s", ATTR_msg(raccess->type), 
									raccess->location, NULL );
				else
					add_params( "-a%s=%s:%s", ATTR_msg(raccess->type),
									raccess->nimif.hostname, raccess->location );
			break;
		}
	}

	/* has root directory already been initialized previously? */
	if ( find_attr( mobj, NULL, NULL, 0, ATTR_ROOT_INITIALIZED ) >= 0 )
		add_params( "-a%s=yes", ATTR_ROOT_INITIALIZED_T, NULL, NULL );

	VERBOSE("   creating a root directory on %s\n",root->server,NULL,NULL,NULL)

	/* call the client method to create the root */
	if (	(master_exec(root->server,&rc,&c_stdout,&params.list[0])== FAILURE)||
			(rc > 0) )
	{	/* cache err info */
		tmp = nim_malloc( strlen( niminfo.errstr ) + 1 );
		strcpy( tmp, niminfo.errstr );
		ERROR( ERR_METHOD, root->server, tmp, NULL )
	}

	return( SUCCESS );

} /* end of mk_root */

/*---------------------------- mk_paging          ------------------------------
*
* NAME: mk_paging
*
* FUNCTION:
*		creates a paging file
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
*			only_one				= TRUE if only one paging file should exist
*		global:
*			mobj					= ptr to nim_object struct
*			paging				= ptr to res_access struct
*			nimif					= nim_if struct
*			params				= LIST to use for parameters
*			allocd				= LIST of res_access structs
*
* RETURNS: (int)
*		SUCCESS					= paging file created
*		FAILURE					= not ok - something missing or wrong state or ...
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mk_paging(	int only_one )

{	int i;
	int size = 0;
	int rc;
	FILE *c_stdout = NULL;
	char *tmp = NULL;
	NIM_PDATTR( def_ps_size, dinfo )

	/* reset the LIST */
	reset_LIST( &params );

	/* get the paging location */
	if ( (i = find_res_access( ATTR_PAGING, &allocd )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, ATTR_PAGING_T, mobj->name, NULL )
	paging = (struct res_access *) allocd.list[i];

	/* was the size specified? */
	if ( (i = find_attr_ass( &attr_ass, ATTR_SIZE )) >= 0 )
		size = (int) strtol( attr_ass.list[i]->value, NULL, 0 );
	else
	{
		/* use default from the database */
		if (	(get_pdattr( &def_ps_size, &dinfo, 0, 0, 0, ATTR_SIZE, NULL ) > 0)&&
				(def_ps_size->value[0] != NULL_BYTE) )
			size = (int) strtol( def_ps_size->value, NULL, 0 );
	}

	/* build the params LIST */
	add_params( "%s", C_MKPAGING, NULL, NULL );
	if ( only_one )
		add_params( "-oa%s=%s", ATTR_PAGING_T, paging->location, NULL );
	else
		add_params( "-a%s=%s", ATTR_PAGING_T, paging->location, NULL );
	if ( size > 0 )
		add_params( "-a%s=%d", ATTR_SIZE_T, size, NULL );

	VERBOSE("   creating a paging file on %s\n",paging->server,NULL,NULL,NULL)

	/* call the client method to create the root */
	if (	(master_exec(	paging->server, &rc, &c_stdout,
								&params.list[0] ) == FAILURE) ||
			(rc > 0) )
	{	/* cache err info */
		tmp = nim_malloc( strlen( niminfo.errstr ) + 1 );
		strcpy( tmp, niminfo.errstr );
		ERROR( ERR_METHOD, root->server, tmp, NULL )
	}

	return( SUCCESS );

} /* end of mk_paging */

/*---------------------------- mk_dump            ------------------------------
*
* NAME: mk_dump  
*
* FUNCTION:
*		creates a dump directory & file
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
*			mobj					= ptr to nim_object struct
*			dump					= ptr to res_access struct
*			nimif					= nim_if struct
*			params				= LIST to use for parameters
*			allocd			= LIST of res_access structs
*
* RETURNS: (int)
*		SUCCESS					= dump directory created
*		FAILURE					= not ok - something missing or wrong state or ...
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mk_dump()

{	int i;
	int rc;
	FILE *c_stdout = NULL;
	char *tmp = NULL;

	/* reset the LIST */
	reset_LIST( &params );

	/* get the dump location */
	if ( (i = find_res_access( ATTR_DUMP, &allocd )) < 0 )
		ERROR( ERR_BAD_OBJECT, ATTR_CLASS_MACHINES_T, mobj->name, NULL )
	dump = (struct res_access *) allocd.list[i];

	/* build the params LIST */
	add_params( "%s", C_MKDIR, NULL, NULL );
	add_params( "-fa%s=%s/dump", ATTR_LOCATION_T, dump->location, NULL );

	VERBOSE("   creating a dump file on %s\n",dump->server,NULL,NULL,NULL)

	/* call the client method to create the root */
	if (	(master_exec(dump->server,&rc,&c_stdout,&params.list[0])== FAILURE)||
			(rc > 0) )
	{	/* cache err info */
		tmp = nim_malloc( strlen( niminfo.errstr ) + 1 );
		strcpy( tmp, niminfo.errstr );
		ERROR( ERR_METHOD, root->server, tmp, NULL )
	}

	return( SUCCESS );

} /* end of mk_dump */

