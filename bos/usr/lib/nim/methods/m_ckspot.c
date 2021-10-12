static char     sccs_id[] = " @(#)17 1.15.1.4  src/bos/usr/lib/nim/methods/m_ckspot.c, cmdnim, bos41J, 9519A_all  5/4/95  08:45:58";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_ckspot.c
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

extern int ch_pdattr_ass();
extern int ok_to_op_spot();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{
	{ATTR_AUTO_EXPAND,		ATTR_AUTO_EXPAND_T,		FALSE,	ch_pdattr_ass},
	{ATTR_INSTALLP_FLAGS,	ATTR_INSTALLP_FLAGS_T,	FALSE,	ch_pdattr_ass},
	{ATTR_IGNORE_STATE,		ATTR_IGNORE_STATE_T,		FALSE,	ch_pdattr_ass},
	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{ATTR_DEBUG,				ATTR_DEBUG_T,				FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE,	ch_pdattr_ass}
};

char *name=NULL;							/* NIM name of object to create */
NIM_OBJECT( spot, sinfo )				/* SPOT's object */
char *c_ckspot_msgs = NULL;			/* messages from C_CKSPOT */
char *auto_expand="yes";			   /* do auto expand.. */
	
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
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_GENERIC_MKSYNTAX),NULL);
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_MKSYNTAX), NULL, NULL );
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
	
/*---------------------------- spot_Rstate       ------------------------------
*
* NAME: spot_Rstate
*
* FUNCTION:
*		sets the Rstate for the SPOT
*		if any ATTR_MISSING exist, then Rstate set to STATE_INCOMPLETE;
*			otherwise, Rstate set to STATE_AVAILABLE
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
*			name
*			spot
*
* RETURNS: (int)
*		SUCCESS
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
spot_Rstate()

{
	/* free memory */
	if ( spot != NULL )
		odm_free_list( spot, &sinfo );

	/* get the latest info */
	if ( lag_object( 0, name, &spot, &sinfo ) == SUCCESS )
	{
		/* any ATTR_IF_SUPPORTED attrs? */
		if ( find_attr( spot, NULL, NULL, 0, ATTR_IF_SUPPORTED ) >= 0 )
		{
			/* there may be ATTR_MISSING attrs, but, since there is at least */
			/*		one network boot image available, the SPOT can support */
			/*		some kind of client, so make it available */
			set_state( spot->id, NULL, ATTR_RSTATE, STATE_AVAILABLE ); 

			/* remove any ATTR_MISSING attrs so we don't confuse the users */
			rm_attr( spot->id, NULL, ATTR_MISSING, 0, NULL );

			return( SUCCESS );
		}
		else
			errstr( ERR_MISSING, name, NULL, NULL );
	}

	/* some type of error has occurred: either the SPOT is missing stuff */
	/*		or we were unable to lag_object */
	protect_errstr = TRUE;
	set_state( spot->id, NULL, ATTR_RSTATE, STATE_INCOMPLETE ); 

	return( FAILURE );

} /* end of spot_Rstate */
	
/* --------------------------- undo ----------------------------------------
 *
 * NAME: undo
 *
 * FUNCTION:
 *		sets SPOT's Rstate to incomplete
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
 *		spot
 *
 * RETURNS: (void)
 *
 * OUTPUT:
 * -------------------------------------------------------------------------- */

void
undo(	int errno,
		char *str1,
		char *str2,
		char *str3 )

{	
	disable_err_sig();

	errno = nene( errno, str1, str2, str3 );

	/* set the new Rstate */
	if ( spot_Rstate() == FAILURE )
		warning( 0, NULL, NULL, NULL );

	/* bye-bye */
	exit( errno );

} /* end of undo */
	
/*---------------------------- ck_spot        ------------------------------
*
* NAME: ck_spot
*
* FUNCTION:
*		scrutinizes the SPOT to determine if it has everything a SPOT must have
*			in order for clients to use it
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
*		SUCCESS					= SPOT is viable
*		FAILURE					= internal err or something is missing from the SPOT
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ck_spot()

{	int Rstate;
	int method_call=0, debug_present=0, nobooti_added=0, remove_if_supported=0;
	int sx;
	int lx;
	int rc;
	FILE *c_stdout=NULL;
	char *flags = NULL;
	int arg = 0;
	char *ckspot[] = { 	C_CKSPOT, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
								NULL, NULL, NULL, NULL, NULL };
	char *warning_msg = NULL;
	int errno = 0;
	NIM_PDATTR( platform, pinfo )
	int i;
	int chcnt;

	/* get the SPOT's object */
	if ( lag_object( 0, name, &spot, &sinfo ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	/* 
	 * If this is called via a method (in particular instspot) the ignore_state 
	 * attr was added by that method (via the function check_spot). This is a clue that 
	 * we've been called from a method and not via nim (nim -o check...)..  
	 * We need to check if ignore_state is present without the force attr and  
	 * this needs to occur before the call to ok_to_op_spot. ok_to_op_spot will set the force 
	 * flag based on ignore_state and the force flag...  
	 */ 
	if (!force)
		method_call=(find_attr_ass( &attr_ass, ATTR_IGNORE_STATE ) >= 0 );

	/* ok to perform this operation? */
	if ( ok_to_op_spot( spot ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	/* find server & location */
	if ( (sx = find_attr( spot, NULL, NULL, 0, ATTR_SERVER )) < 0 )
		undo( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, name, NULL );
	else if ( (lx = find_attr( spot, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
		undo( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, name, NULL );

	/* get the predefined platform information */
	if ( get_pdattr(	&platform, &pinfo, ATTR_CLASS_MACHINES, 
							ATTR_SUBCLASS_PLATFORM, 0, 0, NULL ) <= 0 )
		undo( ERR_BAD_OBJECT, "predefined", ATTR_SUBCLASS_PLATFORM_T, NULL );

	/* make sure SPOT's Rstate gets set */
	undo_on_interrupt = undo;

	/* initialize parameter string for C_CKSPOT */

	/* SPOT name */
	ckspot[ ++arg ] = nim_malloc( strlen(ATTR_NAME_T) + strlen(name) + 4 );
	sprintf( ckspot[arg], "-a%s=%s", ATTR_NAME_T, name );

	/* location */
	ckspot[ ++arg ] = nim_malloc( strlen(ATTR_LOCATION_T) + 
											strlen(spot->attrs[lx].value) + 4 );
	sprintf( ckspot[arg], "-a%s=%s", ATTR_LOCATION_T, spot->attrs[lx].value );

	/* value of ST_APPLIED */
	ckspot[ ++arg ] = nim_malloc( strlen(ATTR_ST_APPLIED_T) + 10 );
	sprintf( ckspot[arg], "-a%s=%d", ATTR_ST_APPLIED_T, ST_APPLIED );

	/* value of ST_COMMITTED */
	ckspot[ ++arg ] = nim_malloc( strlen(ATTR_ST_COMMITTED_T) + 10 );
	sprintf( ckspot[arg], "-a%s=%d", ATTR_ST_COMMITTED_T, ST_COMMITTED );

	/* platform information */
	chcnt = 0;
	for (i=0; i < pinfo.num; i++)
		chcnt += strlen(platform[i].name) + strlen(platform[i].value) + 3;
	ckspot[ ++arg ] = nim_malloc( strlen(ATTR_PLATFORM_T) + chcnt + 5 );
	sprintf( ckspot[arg], "-a%s= ", ATTR_PLATFORM_T );
	for (i=0; i < pinfo.num; i++)
	{
		chcnt = strlen( ckspot[arg] );
		sprintf( (ckspot[arg] + chcnt), "%s=%s ", platform[i].name,
					platform[i].value );
	}

	/* 
	 * was ATTR_DEBUG specified? 
	 */
	if ( find_attr_ass( &attr_ass, ATTR_DEBUG ) >= 0 ) {
		if ( strcmp(attr_value(&attr_ass, ATTR_DEBUG ),"yes") == 0 ) {
			/*
			 * pass ATTR_DEBUG so we get the kernel 
			 * debugger turned on for the network boot images 
			 */
			ckspot[ ++arg ] = nim_malloc( strlen(ATTR_DEBUG_T) + 7 );
			sprintf( ckspot[arg], "-a%s=yes", ATTR_DEBUG_T );
		}

		VERBOSE("   debug is present \n",NULL,NULL,NULL,NULL)
		remove_if_supported++;
	}
	else {
		if ( force && !method_call ) { 
			VERBOSE("   force via cmdline\n",NULL,NULL,NULL,NULL)
			remove_if_supported++;
		}
		else {
			if ( find_attr( spot, NULL, NULL, 0, ATTR_MK_NETBOOT) < 0 ) {
				VERBOSE("   spot does not have mk_netboot\n",NULL,NULL,NULL,NULL)
				ckspot[ ++arg ] = nim_malloc( strlen(ATTR_NO_MKBOOTI_T) + 7 );
				sprintf( ckspot[arg], "-a%s=yes", ATTR_NO_MKBOOTI_T );
			}
			else {
				VERBOSE("   spot has mk_netboot, remove if_supported\n",NULL,NULL,NULL,NULL)
				remove_if_supported++;
			}
		}
	}
	ckspot[ ++arg ] = nim_malloc( strlen(ATTR_AUTO_EXPAND_T) + 7 );
	/* did user specify ATTR_AUTO_EXPAND value? */
	if ( find_attr_ass( &attr_ass, ATTR_AUTO_EXPAND ) >= 0 )
		sprintf( ckspot[arg], "-a%s=%s", ATTR_AUTO_EXPAND_T, 
			attr_value(&attr_ass, ATTR_AUTO_EXPAND) );
	else
		sprintf( ckspot[arg], "-a%s=%s", ATTR_AUTO_EXPAND_T, auto_expand );
		

	/* 
	 * pass ATTR_INSTALLP_FLAGS? 
	 * we pass these in case they have the auto expand flag ("X") 
	 */
	if ( (flags = attr_value( &attr_ass, ATTR_INSTALLP_FLAGS )) != NULL )
	{
		ckspot[ ++arg ] = nim_malloc( strlen(ATTR_INSTALLP_FLAGS_T) + 
					strlen(flags) + 4 );
		sprintf( ckspot[arg], "-a%s=%s", ATTR_INSTALLP_FLAGS_T, flags );
	}

	VERBOSE("   going to check the SPOT at %s:%s\n",spot->attrs[sx].value,
				spot->attrs[lx].value,NULL,NULL)
	VERBOSE("   this may take some time - please wait\n",NULL,NULL,NULL,NULL)

	/* invoke C_CKSPOT on SPOT's server */
	if ( master_exec(spot->attrs[sx].value,&rc,&c_stdout,ckspot) == FAILURE )
		undo( 0, NULL, NULL, NULL );
	else if ( (rc > 0) && (rc != ERR_MISSING) )
	{
		/* cache the warning to be displayed after errors (if any) */
		warning_msg = niminfo.errstr;
		niminfo.errstr = NULL;
	}
		
	/* no interruptions here (please) */
	undo_on_interrupt = NULL;
	disable_err_sig();

	/* remove any info attrs the SPOT had before this operation */
	rm_attr( spot->id, NULL, ATTR_MISSING, 0, NULL );
	rm_attr( spot->id, NULL, ATTR_INFO, 0, NULL );

	if (remove_if_supported) {
		rm_attr( spot->id, NULL, ATTR_ENTER_DBG, 0, NULL );
		rm_attr( spot->id, NULL, ATTR_IF_SUPPORTED, 0, NULL );
	}

	/* 
	 * add any attrs which were written to stdout 
	 */
	attrs_from_FILE( spot->id, c_stdout );
	 
	/* re-get the SPOT's object after the updates */
	if ( lag_object( 0, name, &spot, &sinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/*
	 * if we removed the if_supported attr then check if boot build 
	 * was ok. If it was then removed the mk_netboot attr. 
	 */
	if (remove_if_supported) {
		if ( find_attr( spot, NULL, NULL, 0, ATTR_IF_SUPPORTED) >= 0 )
			rm_attr( spot->id, NULL, ATTR_MK_NETBOOT, 0, NULL );
	}

	/* set the new Rstate */
	if ( spot_Rstate() == FAILURE )
		errno = nene( 0, NULL, NULL, NULL );

	/* display warnings */
	if ( warning_msg != NULL )
		warning( ERR_METHOD, spot->attrs[sx].value, warning_msg, NULL );

	/* all done - bye bye */
	exit( errno );

} /* end of ck_spot */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	spot = NULL;
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_ckspot: check the %s SPOT\n",name,NULL,NULL,NULL)

	/* check the SPOT in order to determine what it's current state should be */
	ck_spot();

	exit( 0 );
}

