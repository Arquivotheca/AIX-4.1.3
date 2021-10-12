static char	sccs_id[] = " @(#)01 1.29  src/bos/usr/lib/nim/methods/m_mkspot.c, cmdnim, bos411, 9438C411a  9/24/94  11:31:13";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_mkspot.c
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

/* --------------------------- module globals    ---------------------------- */
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{ATTR_SERVER,				ATTR_SERVER_T,				TRUE,		valid_pdattr_ass},
	{ATTR_LOCATION,			ATTR_LOCATION_T,			TRUE,		valid_pdattr_ass},
 	{ATTR_SOURCE,				ATTR_SOURCE_T,				TRUE,		valid_pdattr_ass},
 	{ATTR_INSTALLP_FLAGS,	ATTR_INSTALLP_FLAGS_T,	FALSE,	valid_pdattr_ass},
	{ATTR_AUTO_EXPAND,		ATTR_AUTO_EXPAND_T,		FALSE,	ch_pdattr_ass},
 	{ATTR_COMMENTS,			ATTR_COMMENTS_T,			FALSE,	valid_pdattr_ass},
 	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
 	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
 	{ATTR_DEBUG,				ATTR_DEBUG_T,				FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE,	NULL}
};

char *name=NULL;							/* NIM name of object to create */
char *type=ATTR_SPOT_T;					/* object's type */
long id;										/* id of the new SPOT object */
char *location=NULL;						/* location of SPOT */
char *parent_dir=NULL;					/* parent directory of the SPOT */
int usr_spot=FALSE;						/* TRUE if SPOT is a /usr filesystem */
LIST res_access_list;					/* LIST of res_access structs */
LIST args;									/* LIST of parameters for methods */
LIST alloc_list;							/* LIST of alloc'd resources */
char tmp[MAX_VALUE];						/* tmp storage for parameter str */
char *flags;								/* value of ATTR_INSTALLP_FLAGS */
char *source=NULL;						/* source for SPOT files */
int src_type;								/* type of source device */
int local_source_device=FALSE;		/* TRUE if src_type == ATTR_DEVICE */
NIM_OBJECT( spot, sinfo ) 				/* the new SPOT's NIM definition */
NIM_OBJECT( server, serveri ) 		/* the server's NIM definition */
NIM_OBJECT( src, srcinfo )				/* nim_object for source */
int simages = FALSE;						/* TRUE if lpp_source has "simages" */
int create_object_only=FALSE;			/* TRUE if "-o" supplied */
char *auto_expand="yes";   		   /* reflects ATTR_AUTO_EXPAND attr */
	
/* --------------------------- parse_args        -------------------------------
 *
 * NAME: parse_args
 *
 * FUNCTION:
 *		parses command line arguments
 *
 * NOTES:
 *		calls nim_error
 *
 * DATA STRUCTURES:
 *	parameters:
 *		argc	= argc from main
 *		argv	= argv from main
 *	global:
 *
 * RETURNS: (int)
 *	SUCCESS		= no syntax errors on command line
 *
 * -------------------------------------------------------------------------- */

int
parse_args(	int argc, 
		char *argv[] )

{	extern char *optarg;
	extern int optind, optopt;
	int c;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:oqt:")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
			break;

			case 'o': /* create the object only */
				create_object_only = TRUE;
			break;

			case 'q': /* display valid_attrs */
				mstr_what( valid_attrs, NULL );
				exit( 0 );
			break;

			case 't': /* object type - must be "spot" */
				if ( strcmp( optarg, ATTR_SPOT_T ) > 0 )
					nim_error( ERR_CONTEXT, optarg, NULL, NULL );
				type = optarg;
			break; 

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

			case '?': /* unknown option */
				nim_error( ERR_BAD_OPT, optopt,MSG_msg(MSG_GENERIC_MKSYNTAX), NULL);
			break;
		}
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_MKSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/* --------------------------- undo_pop ----------------------------------------
 *
 * NAME: undo_pop
 *
 * FUNCTION:
 *		cleans up after pop_spot function
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
 *		errno	= error code
 *		str1	= str1 for error msg
 *		str2	= str2 for error msg
 *		str3	= str3 for error msg
 *	global:
 *
 * RETURNS: (void)
 *
 * OUTPUT:
 * -------------------------------------------------------------------------- */

void
undo_pop(	int errno,
			char *str1,
			char *str2,
			char *str3 )

{	FILE *c_stdout = NULL;

	disable_err_sig();

	/* set the errstr */
	if ( errno > 0 )
		errstr( errno, str1, str2, str3 );
	protect_errstr = TRUE;

	/* dealloc any allocated resources */
	dealloc_LIST( server->name, NULL, &alloc_list, &c_stdout );

	/* remove the new object */
	rm_object( 0, name );

	/* remove the ATTR_SERVES from the server */
	rm_attr( 0, server->name, ATTR_SERVES, 0, name );

	nim_error( 0, NULL, NULL, NULL );

} /* end of undo_pop */

/* --------------------------- undo_ckspot -------------------------------------
 *
 * NAME: undo_ckspot
 *
 * FUNCTION:
 *		sets the SPOT's Rstate when error has occurred (probably interrupt)
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
 *		errno	= error code
 *		str1	= str1 for error msg
 *		str2	= str2 for error msg
 *		str3	= str3 for error msg
 *	global:
 *
 * RETURNS: (void)
 *
 * OUTPUT:
 * -------------------------------------------------------------------------- */

void
undo_ckspot(	int errno,
					char *str1,
					char *str2,
					char *str3 )

{	
	errno = nene( errno, str1, str2, str3 );

	/* check the SPOT */
	if ( check_spot( name, server->name, &alloc_list ) == FAILURE )
		warning( 0, NULL, NULL, NULL );

	exit( errno );

} /* end of undo_ckspot */
	
/* --------------------------- ck_attrs           ------------------------------
 *
 * NAME: ck_attrs
 *
 * FUNCTION:
 *  checks to make sure that the information supplied by user is sufficient
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
 *		SUCCESS	= nothing missing
 *		FAILURE	= definition incomplete
 *
 * OUTPUT:
 * -------------------------------------------------------------------------- */

int
ck_attrs ()

{	int i;
	char *path;
	int loc_index;
	regmatch_t device[ERE_DEVICE_NUM];

	server = NULL;
	parent_dir = NULL;
	source = NULL;
	src = NULL;

	/* loop through the attr_ass... */
	for (i=0; i < attr_ass.num; i++)
	{
		switch (attr_ass.list[i]->pdattr)
		{
			case ATTR_AUTO_EXPAND:
				auto_expand = attr_ass.list[i]->value;
				if ( (strcmp(auto_expand, "yes") != 0) &&
				     (strcmp(auto_expand, "no" ) != 0) )  
				     nim_error( ERR_ATTR_YES_NO, ATTR_AUTO_EXPAND_T, NULL, NULL);
			break; 
			
			case ATTR_SERVER:
				if ( server != NULL )
					nim_error( ERR_ONLY_ONE_ATTR, ATTR_SERVER_T, NULL, NULL );

				/* get the server's object */
				if ( lag_object( 0, attr_ass.list[i]->value, &server,
										&serveri ) == FAILURE )
					nim_error( 0, NULL, NULL, NULL );
			break;

			case ATTR_LOCATION:
				if ( parent_dir != NULL )
					nim_error( ERR_ONLY_ONE_ATTR, ATTR_LOCATION_T, NULL, NULL );

				parent_dir = attr_ass.list[i]->value;
				loc_index = i;
			break;

			case ATTR_SOURCE:
				if ( source != NULL )
					nim_error( ERR_ONLY_ONE_ATTR, ATTR_SOURCE_T, NULL, NULL );

				source = attr_ass.list[i]->value;
			break;

		} /* switch */
	} /* for */

	/* anything missing? */
	if ( server == NULL )
		nim_error( ERR_MISSING_ATTR, ATTR_SERVER_T, NULL, NULL );
	else if ( parent_dir == NULL )
		nim_error( ERR_MISSING_ATTR, ATTR_LOCATION_T, NULL, NULL );
	else if ( (source == NULL) && (create_object_only == FALSE) )
		nim_error( ERR_MISSING_ATTR, ATTR_SOURCE_T, NULL, NULL );

	/* make sure parent directory exists and is in a JFS filesystem */
	if ( stat_file(	NULL, NULL, ATTR_SPOT_T, server->name, 
							parent_dir ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* is this a /usr SPOT? */
	if ( strcmp( parent_dir, "/usr" ) == 0 )
		usr_spot = TRUE;
	else
	{
		/* construct what will end up being the full pathname of the SPOT */
		path = nim_malloc( strlen(parent_dir) + strlen(name) + 6 );
		sprintf( path, "%s/%s/usr", parent_dir, name );
		attr_ass.list[ loc_index ]->value = path;
	}

	/* skip the rest if we're just creating the object */
	if ( create_object_only )
		return( SUCCESS );

	/* what kind of source? */
	if ( regexec(	nimere[DEVICE_ERE].reg, source,
						ERE_DEVICE_NUM, device, 0 ) == 0 )
	{
		/* source is a device */
		/* ASSUMING that it is local to the server of the SPOT & that */
		/*		it has "simages" */
		local_source_device = TRUE;
		src_type = ATTR_DEVICE;
	}
	else
	{
		/* source must be either an existing spot or lpp_source */
		if ( lag_object( 0, source, &src, &srcinfo ) == FAILURE )
			nim_error( ERR_SOURCE, source, NULL, NULL );
		src_type = src->type->attr;
		switch (src_type)
		{
			case ATTR_SPOT:
				if ( usr_spot )
					nim_error( ERR_CONTEXT, source, NULL, NULL );
			break;
	
			case ATTR_LPP_SOURCE:
				if ( ! usr_spot )
				{
					/* install operation will be performed on a non-/usr SPOT */
					/* in this case, we must prevent the use of a CDROM as the */
					/*		lpp_source because, in order to use a CDROM for non-/usr */
					/*		SPOT installs, we would have to mount it inside the */
					/*		inst_root of the SPOT, which could require unmounting */
					/*		the CDROM from its current mount point, remembering */
					/*		where it was, mounting it inside the SPOT, do the */
					/*		operation, remount the CDROM at the original mount */
					/*		point.  additionally, we'd have to deal with issues like */
					/*		the possibilty of the CDROM being exported for use, etc. */
					/* so, is this lpp_source a CDROM? */
					if ( get_attr( NULL, NULL, src->id, NULL, 0, ATTR_CDROM ) > 0 )
						nim_error( ERR_CDROM_USE, source, NULL, NULL );
				}

				/* source must have the "simages" attribute */
				if ( find_attr( src, NULL, NULL, 0, ATTR_SIMAGES ) < 0 )
					nim_error( ERR_OBJ_MISSING_ATTR, ATTR_SIMAGES_T, source, NULL );
			break;
	
			default:
				nim_error( ERR_CONTEXT, source, NULL, NULL );
			break;
		} /* switch */
	} /* else source is NIM object */

	
	flags = attr_value( &attr_ass, ATTR_INSTALLP_FLAGS );
   /* 
    * If auto_expand == no and installp_flags sez expand error out ! 
    */ 
   if ( ( strcmp(auto_expand, "no") == 0 ) && 
        ( strrchr(flags, 'X') != NULL )    )  
               nim_error( ERR_NO_X, NULL, NULL, NULL );

	return( SUCCESS );

} /* end of ck_attrs */
	
/*---------------------------- prep          ------------------------------
*
* NAME: prep
*
* FUNCTION:
*		performs misc prep tasks to support SPOT creation
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
*		SUCCESS					= NIM env prep'ed for SPOT creation
*		FAILURE					= fatal error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
prep()

{	int i,j;
	struct nim_if nimif;
	int rc;
	FILE *c_stdout=NULL;
	long server_id=0;
	char *server_name;

	/* remove ATTR_LPP_SOURCE from the attr_ass (so it doesn't become part */
	/*		of the SPOT's definition) */
	rm_attr_ass( &attr_ass, ATTR_LPP_SOURCE );

	/* create a new resource object which will represent this SPOT */
	if ( (id = mk_robj( name, type, &server_name, FALSE, &attr_ass )) == FAILURE)
		nim_error( 0, NULL, NULL, NULL );
	
	/* setup for interrupt processing */
	undo_on_interrupt = undo_pop;

	/* 
	 * Always force an attempt to make the boot 
	 * images on spot creation 
	 */
	if ( mk_attr( id,NULL,"yes",0,ATTR_MK_NETBOOT,ATTR_MK_NETBOOT_T) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );

	/* get the new object */
	if ( lag_object( id, NULL, &spot, &sinfo ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );

	/* point to the location */
	if ( (i = find_attr( spot, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
		undo_pop( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, spot->name, NULL );
	location = spot->attrs[i].value;

	/* cache the server's id for later use */
	server_id = server->id;

	VERBOSE("   preparing to create the SPOT at %s:%s\n",server->name,location,
				NULL,NULL)

	/* add an ATTR_SERVES attr for the server now because, if this fails, we  */
	/* 	don't want to continue (waste of time) */
	if ( mk_attr(	0, server->name, name, 0, ATTR_SERVES, 
						ATTR_SERVES_T ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );

	/* skip the rest if we're just creating the object */
	if ( create_object_only )
		return( SUCCESS );

	/* get the server's PIF info */
	if ( (i = find_attr( server, NULL, NULL, 1, ATTR_IF )) < 0 )
		undo_pop( ERR_OBJ_MISSING_ATTR, PIF, server->name, NULL );
	if ( s2nim_if( server->attrs[i].value, &nimif ) == FAILURE )
		undo_pop( ERR_NET_INFO, PIF, server->name, NULL );

	/* is source a NIM object? */
	if ( src != NULL )
	{
		/* add ATTR_IGNORE_STATE so that we bypass some checking */
		if ( add_attr_ass( &attr_ass, ATTR_IGNORE_STATE, ATTR_IGNORE_STATE_T,
									"yes", 0 ) == FAILURE )
			undo_pop( 0, NULL, NULL, NULL );

		/* allocate it for the server's use & add to alloc LIST */
		if (	(alloc_res( server, src->name, &nimif, &c_stdout ) == FAILURE) ||
				(add_to_LIST( &alloc_list, src->name ) == FAILURE) )
			undo_pop( 0, NULL, NULL, NULL );
	}

	/* resources allocated, which means new attrs were added, so re-get server */
	odm_free_list( server, &serveri );
	if ( lag_object( server_id, NULL, &server, &serveri ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );

	/* next step is to generate res_access LIST, which will be used to init */
	/*		the parameter string for C_MKSPOT */
	if ( LIST_res_access( server, &res_access_list ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );

	return( SUCCESS );

} /* end of prep */

/* --------------------------- pop_spot         --------------------------------
 *
 * NAME: pop_spot
 *
 * FUNCTION:
 *		populates a new SPOT, which is the first step in SPOT creation
 *
 * NOTES:
 *		calls nim_error on failure
 *
 * DATA STRUCTURES:
 *	parameters:
 *	global:
 *		name	= NIM name of SPOT
 *
 * RETURNS: (int)
 *		SUCCESS	= new SPOT populated with bos.rte files
 *
 * -------------------------------------------------------------------------- */

int
pop_spot()

{	int i,xpand_idx;
	struct res_access *raccess;
	int rc;
	FILE *c_stdout=NULL;

	/* ready to actually create the SPOT by calling the C_MKSPOT method */

	/* initialize the parameter string for C_MKSPOT */

	/* method pathname */
	if ( add_to_LIST( &args, C_MKSPOT ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );

	/* parent directory location */
	sprintf( tmp, "-a%s=%s", ATTR_LOCATION_T, parent_dir );
	if ( add_to_LIST( &args, tmp ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );

	/* SPOT's name */
	sprintf( tmp, "-a%s=%s", ATTR_NAME_T, name );
	if ( add_to_LIST( &args, tmp ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );

	/* the values for ST_APPLIED and ST_COMMITTED, which are required by */
	/*		the ck_spot_options function, which gets called by c_mkspot */
	sprintf( tmp, "-a%s=%d", ATTR_ST_APPLIED_T, ST_APPLIED );
	if ( add_to_LIST( &args, tmp ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );
	sprintf( tmp, "-a%s=%d", ATTR_ST_COMMITTED_T, ST_COMMITTED );
	if ( add_to_LIST( &args, tmp ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );

	/* pass installp_flags and auto_expand as specified by user */
	/* C_MKSPOT uses them to determine whether to auto expand or not */
	if ( auto_expand != NULL ) {
		sprintf( tmp, "-a%s=%s", ATTR_AUTO_EXPAND_T, auto_expand);
		if ( add_to_LIST( &args, tmp ) == FAILURE )
			undo_pop( 0, NULL, NULL, NULL );
	}

	if ( flags != NULL ) {
		sprintf( tmp, "-a%s=%s", ATTR_INSTALLP_FLAGS_T, flags );
		if ( add_to_LIST( &args, tmp ) == FAILURE )
			undo_pop( 0, NULL, NULL, NULL );
	}

	/* type of source */
	sprintf( tmp, "-a%s=%s", ATTR_TYPE_T, ATTR_msg(src_type) );
	if ( add_to_LIST( &args, tmp ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );

	/* source device */
	switch (src_type)
	{
		case ATTR_DEVICE:
			sprintf( tmp, "-a%s=%s", ATTR_SOURCE_T, source );
			if ( add_to_LIST( &args, tmp ) == FAILURE )
				undo_pop( 0, NULL, NULL, NULL );
		break;

		default:
			/* look for source in the res_access LIST */
			for (i=0; i < res_access_list.num; i++)
			{	
				raccess = (struct res_access *) res_access_list.list[i];
				if ( strcmp( raccess->name, src->name ) == 0 )
				{	
					/* is source local to the server? */
					if ( strcmp( raccess->server, server->name ) == NULL )
						sprintf( tmp, "-a%s=%s", ATTR_SOURCE_T, raccess->location );
					else
						sprintf( tmp, "-a%s=%s:%s", ATTR_SOURCE_T,
									raccess->nimif.hostname, raccess->location );

					/* add to parameter list */
					if ( add_to_LIST( &args, tmp ) == FAILURE )
						undo_pop( 0, NULL, NULL, NULL );

					break;
				}
			}
		break;
	}

	VERBOSE("   creating the SPOT\n",NULL,NULL,NULL,NULL)
	VERBOSE("   this may take some time - please wait\n",NULL,NULL,NULL,NULL)

	/* invoke the C_MKSPOT method on the SPOT's server */
	if ( master_exec( server->name, &rc, &c_stdout, &args.list[0] ) == FAILURE )
		undo_pop( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		undo_pop( ERR_METHOD, server->name, niminfo.errstr, NULL );

	/* if we get here, we've got enough to keep what we've got, however, */
	/* 	we still must be prepared for any interrupt which might occur */
	/*		after this point (ie, make sure SPOT's state gets set correctly) */
	undo_on_interrupt = undo_ckspot;

	/* check for SPOT attrs */
	attrs_from_FILE( id, c_stdout );

	return( SUCCESS );

} /* end of pop_spot */

/* --------------------------- inst_spot        --------------------------------
 *
 * NAME: inst_spot
 *
 * FUNCTION:
 *	installs the options required to make a SPOT viable into the new SPOT
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
 *		SUCCESS	= SPOT installed
 *		FAILURE	= installp error
 *
 * OUTPUT:
 * -------------------------------------------------------------------------- */

int
inst_spot()

{	ODMQUERY
	struct nim_pdattr spot_opt;
	int i;
	int rc;
	FILE *m_stdout=NULL;

	VERBOSE("   preparing to install required options into the SPOT\n",NULL,NULL,
				NULL,NULL)

	/* initialize the parameter string for M_INSTSPOT */
	reset_LIST( &args );
	if ( add_to_LIST( &args, M_INSTSPOT ) == FAILURE )
		return( FAILURE );

	/* where do the LPP images come from? */
	if ( local_source_device )
		sprintf( tmp, "-a%s=%s", ATTR_LPP_SOURCE_T, source );
	else
		sprintf( tmp, "-a%s=%s", ATTR_LPP_SOURCE_T, src->name );
	if ( add_to_LIST( &args, tmp ) == FAILURE )
		return( FAILURE );

	/* ignore object locks */
	sprintf( tmp, "-a%s=yes", ATTR_IGNORE_LOCK_T );
	if ( add_to_LIST( &args, tmp ) == FAILURE )
		return( FAILURE );

	/* ignore the SPOT's Rstate */
	sprintf( tmp, "-a%s=yes", ATTR_IGNORE_STATE_T );
	if ( add_to_LIST( &args, tmp ) == FAILURE )
		return( FAILURE );

	/* any installp_flags? */
	if ( flags != NULL )
	{	
		sprintf(tmp, "-a%s=%s", ATTR_INSTALLP_FLAGS_T, flags );
		if ( add_to_LIST( &args, tmp ) == FAILURE )
			return( FAILURE );
	}

	/* auto expand? */
	if ( auto_expand != NULL ) {	
		sprintf(tmp, "-a%s=%s", ATTR_AUTO_EXPAND_T, auto_expand );
		if ( add_to_LIST( &args, tmp ) == FAILURE )
			return( FAILURE );
	}

	/* use ATTR_DEBUG? */
	if ( attr_value( &attr_ass, ATTR_DEBUG ) != NULL )
	{		
		sprintf(tmp, "-a%s=yes", ATTR_DEBUG_T );
		if ( add_to_LIST( &args, tmp ) == FAILURE )
			return( FAILURE );
	}

	/* operand is SPOT name */
	sprintf( tmp, "%s", spot->name );
	if ( add_to_LIST( &args, tmp ) == FAILURE )
		return( FAILURE );

	VERBOSE("   installing required options into the SPOT\n",NULL,NULL,NULL,NULL)
	VERBOSE("   this may take some time - please wait\n",NULL,NULL,NULL,NULL)

	/* invoke the master install method (local exec) */
	niminfo.errno = 0;
	if ( client_exec( &rc, &m_stdout, &args.list[0] ) == FAILURE )
		undo_ckspot( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		nim_error( ERR_METHOD, niminfo.nim_name, niminfo.errstr, NULL );

	return( SUCCESS );

} /* end of inst_spot */
	
/**************************       main         ********************************/

main( int argc, char *argv[] )

{

	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	/* initialize the attr_ass LIST */
	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* initialize the parameter LIST */
	if ( ! get_list_space( &args, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* initialize the allocation LIST */
	if ( ! get_list_space( &alloc_list, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line assignments */
	parse_args( argc, argv );

	VERBOSE("m_mkspot: define the %s SPOT\n",name,NULL,NULL,NULL)

	/* check for attribute assignment errors */
	ck_attrs();

	/* do misc prep work needed to support this operation */
	prep();

	/* create a directory and populate it with bos.rte files  */
	if ( ! create_object_only )
		pop_spot();

	/* 
	 * if we make it here, we've got enough of a SPOT that we don't want to 
	 * just remove it if any errors are encountered in later processing 
	 */

	/* when the source is a SPOT, we don't need to do any installp stuff */
	if ( (src_type == ATTR_SPOT) || (create_object_only) )
	{
		niminfo.errno = 0;
		if ( check_spot( name, server->name, &alloc_list ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );
	}
	else
		inst_spot();

	/* all done & successful!! */
	undo_on_interrupt = NULL;
	exit( 0 );
}
