static char	sccs_id[] = " @(#)32 1.12  src/bos/usr/lib/nim/methods/m_mk_lpp_source.c, cmdnim, bos411, 9428A410j  7/13/94  16:53:16";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ck_attrs
 *		define_simg
 *		main
 *		parse_args
 *		undo
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
#include <sys/wait.h>
#include <sys/vfs.h>
#include <sys/vmount.h>

extern int	valid_pdattr_ass();
extern int	ch_pdattr_ass();

/* --------------------------- module globals    ---------------------------- */
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] = 
{
	{ ATTR_SERVER,			ATTR_SERVER_T,		TRUE,		valid_pdattr_ass },
	{ ATTR_LOCATION,		ATTR_LOCATION_T,	TRUE,		valid_pdattr_ass },
	{ ATTR_SOURCE,			ATTR_SOURCE_T,		FALSE,	valid_pdattr_ass },
	{ ATTR_OPTIONS,		ATTR_OPTIONS_T,	FALSE,	valid_pdattr_ass },
	{ ATTR_COMMENTS,		ATTR_COMMENTS_T,	FALSE,	valid_pdattr_ass },
	{ ATTR_FORCE,			ATTR_FORCE_T, 		FALSE,	ch_pdattr_ass },
	{ ATTR_VERBOSE,		ATTR_VERBOSE_T,	FALSE,	ch_pdattr_ass },
	{ 0,						NULL,					FALSE,	NULL }
};

char *res_name=NULL;
char *server=NULL;
char *location=NULL;
char *source=NULL;
char *options=NULL;
long id;
int lpps_allocated = FALSE;
NIM_OBJECT( new_server, nsinfo )

/* --------------------------- ck_attrs        
 *
 * NAME: ck_attrs
 *
 * FUNCTION:
 *	checks to make sure that the information supplied by user is sufficient
 *	to complete object definition
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
 *	global:
 *		valid_attrs
 *		attr_ass
 *
 * RETURNS: (int)
 *	SUCCESS	= nothing missing
 *	FAILURE	= definition incomplete
 *
 * OUTPUT:
 * ---------------------------------------------------------------------------*/

int
ck_attrs ( )

{
	int	i, j;
	ATTR_ASS     * aa_ptr;
	char st_vfstype[MAX_TMP];

	/* 
	 *  check for required attrs 
	 */
	for (i=0; valid_attrs[i].name != NULL; i++ )
		if ( valid_attrs[i].required ) {
			for (j = 0; j < attr_ass.num; j++)
				if ( attr_ass.list[j]->pdattr == valid_attrs[i].pdattr )
					break;

			/* required attr present? */
			if ( j == attr_ass.num )
				nim_error( ERR_MISSING_ATTR,valid_attrs[i].name, NULL,NULL );
		}
	/* 
	 * get the location, server and source info. 
	 */
	for (i = 0, j = 0; i < attr_ass.num ; i++)
	{
		aa_ptr = attr_ass.list[i];
		switch ( aa_ptr->pdattr )
		{
		case ATTR_SERVER :
			server = aa_ptr->value;
		break;

		case ATTR_SOURCE :
			source = aa_ptr->value;
		break;

		case ATTR_LOCATION :
			location = aa_ptr->value;
		break;

		case ATTR_OPTIONS :
			options = aa_ptr->value;
		break;
		}
	}

	/* create this resource or just define it? */
	if ( source == NULL )
	{
		/* not going to create it, so location must already exist and be a */
		/*		local JFS filesystem */
		sprintf( st_vfstype, "%d %d", MNT_JFS, MNT_CDROM );
		if ( stat_file(	NULL, st_vfstype, ATTR_LPP_SOURCE_T, server,
								location ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );
	}

} /* end of ck_attrs */

/* --------------------------- parse_args        
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
 *	parameters:
 *		argc	= argc from main
 *		argv	= argv from main
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS	= no syntax errors on command line
 *
 * ----------------------------------------------------------------------- */

int
parse_args(	int argc, 
				char *argv[] )

{
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;
	int	syntax_err = FALSE;

	/* 
	 * loop through all args 
	 */
	while ( (c = getopt(argc, argv, "a:t:q")) != -1 ) {
		switch (c) {
		case 'a': /* 
			   * attribute assignment 
			   */
			if (parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) == FAILURE)
				nim_error( 0, NULL, NULL, NULL );
		break;

		case 'q': /* display valid_attrs */
			mstr_what( valid_attrs, NULL );
			exit( 0 );
		break;

		case ':': /* 
			   * option is missing a required argument 
			   */
			nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
		break;

		case '?': /* unknown option */
			nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_M_MKRES_SYNTAX), NULL );
		break;

		case 't': 
			break; 
		}


	}
	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_M_MKRES_SYNTAX), NULL, NULL );

	res_name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */ 

/* --------------------------- undo 
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
 * OUTPUT:
 * ------------------------------------------------------------------------ */

void
undo(	int errno,
		char *str1,
		char *str2,
		char *str3 )

{	FILE *c_stdout = NULL;

	if ( errno > 0 )
		errstr( errno, str1, str2, str3 );

	protect_errstr = TRUE;

	if ( lpps_allocated )
		dealloc_res( new_server, source, &c_stdout );

	/* remove the newly created object */
	rm_object( id, NULL );

	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */

/*---------------------------- define_lppsrc     ------------------------------
*
* NAME: define_lppsrc
*
* FUNCTION:
*		defines a new lpp_source object and creates and/or checks the lpp_source
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls nim_error on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*		global:
*
* RETURNS: (int)
*		SUCCESS					= lpp_source defined
*		FAILURE					= fatal error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
define_lppsrc()

{	
	int 	rc;
	FILE	*c_stdout=NULL;
	char	*args[] = { C_MK_LPP_SOURCE, NULL, NULL, NULL, NULL };
	int	count = 0;
	NIM_OBJECT( lpps, linfo )
	int i;
	struct nim_if nimif;
	LIST res_access_list;
	struct res_access *raccess;

	/* create a new resource object to represent the lpp_source */
	if ( (id = mk_robj(	res_name, ATTR_LPP_SOURCE_T, &server, FALSE,
			    				&attr_ass )) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	undo_on_interrupt = undo;

	/* initialize parameters for C_MK_LPP_SOURCE */

	/* lpp_source location */
	args[++count] = nim_malloc( strlen(ATTR_LOCATION_T) + strlen(location) + 4 );
	sprintf( args[count], "-a%s=%s", ATTR_LOCATION_T, location );

	/* were specific options specified? */
	if ( options != NULL )
	{
		args[++count] = nim_malloc( strlen(ATTR_OPTIONS_T) + strlen(options) + 4);
		sprintf( args[count], "-a%s=%s", ATTR_OPTIONS_T, options );
	}

	/* actually create the lpp_source? */
	if ( source != NULL )
	{
		/* is source local directory or device? (ie, local to the machine */
		/*		which is going to serve the resource we're creating) */
		if ( source[0] == '/' )
		{
			args[++count] = nim_malloc( strlen(ATTR_SOURCE_T) + strlen(source)+ 4);
			sprintf( args[count], "-a%s=%s", ATTR_SOURCE_T, source );
		}
		else
		{
			/* not a local device, therefore, must be a valid lpp_source */
			if ( lag_object( 0, source, &lpps, &linfo ) == FAILURE )
				undo( 0, NULL, NULL, NULL );
			else if ( lpps->type->attr != ATTR_LPP_SOURCE )
				undo( ERR_SOURCE, source, NULL, NULL );

			/* going to allocate this to the server, so get the server's PIF */
			if ( lag_object( 0, server, &new_server, &nsinfo ) == FAILURE )
				undo( 0, NULL, NULL, NULL );
			else if ( (i = find_attr( new_server, NULL, NULL, 1, ATTR_IF )) < 0 )
				undo( ERR_OBJ_MISSING_ATTR, PIF, new_server->name, NULL );
			else if ( s2nim_if( new_server->attrs[i].value, &nimif ) == FAILURE )
				undo( ERR_NET_INFO, PIF, new_server->name, NULL );

			/* allocate it for the server's use */
			if ( alloc_res( new_server, source, &nimif, &c_stdout ) == FAILURE )
				undo( 0, NULL, NULL, NULL );
			lpps_allocated = TRUE;

			/* re-get the resource server's object so that allocated lpp_source */
			/*		is reflected in it's definition */
			odm_free_list( new_server, &nsinfo );
			if ( lag_object( 0, server, &new_server, &nsinfo ) == FAILURE )
				undo( 0, NULL, NULL, NULL );

			/* figure out what lpp_source server interface to use */
			if ( LIST_res_access( new_server, &res_access_list ) == FAILURE )
				undo( 0, NULL, NULL, NULL );
			for (i=0; i < res_access_list.num; i++)
			{
				raccess = (struct res_access *) res_access_list.list[i];
				if ( strcmp( raccess->name, source ) == 0 )
				{
					/* is lpp_source local or remote? */
					if ( strcmp( raccess->server, new_server->name ) == NULL )
					{
						args[++count] = nim_malloc(	strlen(ATTR_SOURCE_T) + 
																strlen(raccess->location) + 4 );
						sprintf( args[count], "-a%s=%s", ATTR_SOURCE_T, 
									raccess->location );
					}
					else
					{
						args[++count] = nim_malloc(	strlen(ATTR_SOURCE_T) + 
																strlen(raccess->nimif.hostname)+
																strlen(raccess->location) + 5 );
						sprintf( args[count], "-a%s=%s:%s", ATTR_SOURCE_T,
									raccess->nimif.hostname, raccess->location );
					}
				} /* if (strcmp... */
			} /* for ... */
		} /* source is an lpp_source */
	} /* if source != NULL */

	/* invoke C_MK_LPP_SOURCE on the lpp_source server */
	if ( master_exec( server, &rc, &c_stdout, args ) == FAILURE )
		undo( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		undo( ERR_METHOD, server, niminfo.errstr, NULL );

	/* no interrupts here */
	disable_err_sig();

	/* any attr assignments on stdout? (expecting ATTR_SIMAGES if the */
	/*		lpp_source has all the simages in it */
	attrs_from_FILE( id, c_stdout );

	/* make the resource available for use */
	finish_robj( res_name, server );

	/* deallocate source if necessary */
	if (	(lpps_allocated) && 
			(dealloc_res( new_server, source, &c_stdout ) == FAILURE) )
		warning( 0, NULL, NULL, NULL );

	undo_on_interrupt = NULL;

	return( SUCCESS );

} /* end of define_lppsrc */

/**************************       main         ********************************/

main( int argc, char *argv[] )
{

	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( !get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_mk_lpp_source: define the %s lpp_source\n",res_name,NULL,NULL,
				NULL)

	/* check for missing attrs */
	ck_attrs( );

	/* define the resource */
	define_lppsrc( );

	exit( 0 );
}
