static char   sccs_id[] = " @(#)86 1.28  src/bos/usr/lib/nim/methods/m_bos_inst.c, cmdnim, bos41J, 9520A_all  5/12/95  16:07:57";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_bos_inst.c
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cmdnim_mstr.h"
#include <varargs.h>
#include <setjmp.h>

extern int valid_pdattr_ass();
extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{ATTR_SOURCE,				ATTR_SOURCE_T,					FALSE, valid_pdattr_ass},
	{ATTR_PULL,					ATTR_PULL_T,					FALSE, valid_pdattr_ass},
	{ATTR_NO_NIM_CLIENT,		ATTR_NO_NIM_CLIENT_T,		FALSE, valid_pdattr_ass},
	{ATTR_INSTALLP_FLAGS,	ATTR_INSTALLP_FLAGS_T,		FALSE, ch_pdattr_ass},
	{ATTR_INSTALLP_OPTIONS,	ATTR_INSTALLP_OPTIONS_T,	FALSE, ch_pdattr_ass},
	{ATTR_PULL_REQUEST,		ATTR_PULL_REQUEST_T,			FALSE, ch_pdattr_ass},
	{ATTR_PRESERVE_RES,		ATTR_PRESERVE_RES_T,			FALSE, ch_pdattr_ass},
	{ATTR_DEBUG,				ATTR_DEBUG_T,					FALSE, ch_pdattr_ass},
	{ATTR_FORCE,				ATTR_FORCE_T,					FALSE, ch_pdattr_ass},
	{ATTR_VERBOSE,				ATTR_VERBOSE_T,				FALSE, ch_pdattr_ass},
	{ATTR_FORCE_PUSH,			ATTR_FORCE_PUSH_T,			FALSE, ch_pdattr_ass},
	{ATTR_NO_CLIENT_BOOT,	ATTR_NO_CLIENT_BOOT_T,		FALSE, ch_pdattr_ass},
	{ATTR_SET_BOOTLIST,	ATTR_SET_BOOTLIST_T,		FALSE, ch_pdattr_ass},
	{ATTR_AUTO_EXPAND,		ATTR_AUTO_EXPAND_T,			FALSE, ch_pdattr_ass},
	{0,							NULL, 							FALSE, ch_pdattr_ass}
};

VALID_ATTR resources[] =
{	
	{ATTR_SPOT,					ATTR_SPOT_T,				TRUE,		NULL},
	{ATTR_LPP_SOURCE,			ATTR_LPP_SOURCE_T,		TRUE,		NULL},
	{ATTR_BOSINST_DATA,			ATTR_BOSINST_DATA_T,		FALSE,	NULL},
	{ATTR_IMAGE_DATA,			ATTR_IMAGE_DATA_T,		FALSE,	NULL},
	{ATTR_INSTALLP_BUNDLE,	ATTR_INSTALLP_BUNDLE_T,	FALSE,	NULL},
	{ATTR_MKSYSB,				ATTR_MKSYSB_T,				FALSE,	NULL},
	{ATTR_SCRIPT,				ATTR_SCRIPT_T,				FALSE,	NULL},
	{0,							NULL, 						0,			NULL}
};

char *name=NULL;					/* NIM name of object to create */
NIM_OBJECT( target, tinfo )	/* object for machine being operated on */
LIST alloc_list;					/* LIST of res_access structs */
struct res_access *spot;		/* the info to the allocated spot */
char *source=NULL;				/* ptr to value of ATTR_SOURCE */
int pull_interface=FALSE;		/* TRUE if ATTR_PULL specified */
int remain_client=TRUE;			/* FALSE if ATTR_NO_NIM_CLIENT specified */
char *installp_flags=NULL;		/* ptr to ATTR_INSTALLP_FLAGS value */
char *installp_options=NULL;	/* ptr to ATTR_INSTALLP_OPTIONS value */
char *installp_bundle=NULL;	/* ptr to ATTR_INSTALLP_BUNDLE value */
int client_initiated=FALSE;	/* >0 if ATTR_PULL_REQUEST specified */
int preserve_res=FALSE;			/* >0 if ATTR_PRESERVE_RES specified */
int	do_reboot=TRUE;			/* Sez if we should reboot the target */
int	nim_client_push=TRUE;	/* Sez target is a nim client */
int	do_not_expand=FALSE;		/* used for the nonnim copy stuff */
int	bootlist_only=FALSE;		/* used for setting boot list when no_client_boot=yes */

jmp_buf got_ack_sig;				/* stack context for longjmp when client is */
										/* shitting down							 */
int boot_allocd=FALSE;			/* flag to indicate boot res allocated */

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

{	int i,j,no_cb_attr;

	/* check for command line attr assignments */
	no_cb_attr = FALSE;
	for (i=0; i < attr_ass.num; i++)
		if ( attr_ass.list[i]->pdattr == ATTR_PULL )
			pull_interface = TRUE;
		else if ( attr_ass.list[i]->pdattr == ATTR_NO_NIM_CLIENT )
			remain_client = FALSE;
		else if ( attr_ass.list[i]->pdattr == ATTR_INSTALLP_FLAGS )
			installp_flags = attr_ass.list[i]->value;
		else if ( attr_ass.list[i]->pdattr == ATTR_INSTALLP_OPTIONS )
			installp_options = attr_ass.list[i]->value;
		else if ( attr_ass.list[i]->pdattr == ATTR_INSTALLP_BUNDLE )
			installp_bundle = attr_ass.list[i]->value;
		else if ( attr_ass.list[i]->pdattr == ATTR_PULL_REQUEST )
			client_initiated = TRUE;
		else if ( attr_ass.list[i]->pdattr == ATTR_PRESERVE_RES )
			preserve_res = TRUE;
		else if ( attr_ass.list[i]->pdattr == ATTR_FORCE_PUSH ) {
			nim_client_push = FALSE;
			/* For a force_push, bosinst_data is required. */
			for (j=0; resources[j].name != NULL; j++)
				if ( resources[j].pdattr == ATTR_BOSINST_DATA )
				{
					resources[j].required = TRUE;
					break;
				}
		}
		else if ( attr_ass.list[i]->pdattr == ATTR_NO_CLIENT_BOOT ) {
			no_cb_attr=TRUE;
                        if ( (strcmp( attr_ass.list[i]->value, "yes") != 0) &&
                             (strcmp( attr_ass.list[i]->value , "no" ) != 0) )
                                nim_error( ERR_ATTR_YES_NO, 
					ATTR_NO_CLIENT_BOOT_T, NULL, NULL);
			do_reboot = (strcmp( attr_ass.list[i]->value, "no") 	== 0);
		}
		else if ( attr_ass.list[i]->pdattr == ATTR_SET_BOOTLIST ) {
                        if ( (strcmp( attr_ass.list[i]->value, "yes") != 0) &&
                             (strcmp( attr_ass.list[i]->value , "no" ) != 0) )
                                nim_error( ERR_ATTR_YES_NO, ATTR_SET_BOOTLIST_T,
NULL, NULL);
                        bootlist_only = (strcmp( attr_ass.list[i]->value, "yes")
== 0);
		}
		else if ( attr_ass.list[i]->pdattr == ATTR_AUTO_EXPAND ) {
            		if ( (strcmp( attr_ass.list[i]->value, "yes") != 0) &&
                 	     (strcmp( attr_ass.list[i]->value , "no" ) != 0) )  
                 		nim_error( ERR_ATTR_YES_NO, ATTR_AUTO_EXPAND_T, NULL, NULL);
			do_not_expand = (strcmp( attr_ass.list[i]->value, "no") == 0); 
		}
		else if ( attr_ass.list[i]->pdattr == ATTR_SOURCE )
		{
			source = attr_ass.list[i]->value;

			/* is source a mksysb image? */
			if ( strcmp( source, ATTR_MKSYSB_T ) == 0 )
			{
				/* mksysb is a required resource - make it so */
				for (j=0; resources[j].name != NULL; j++)
					if ( resources[j].pdattr == ATTR_MKSYSB )
					{
						resources[j].required = TRUE;
						break;
					}
			}
		}

	/* look for errors */
	if ( (installp_options != NULL) && (installp_bundle != NULL) )
		nim_error(  ERR_ATTR_CONFLICT, ATTR_INSTALLP_BUNDLE_T,
						ATTR_INSTALLP_OPTIONS_T, NULL );

	if (bootlist_only)
	{
		/* set_bootlist attr was specified */

		/* If no_client_boot was not specified, error off */
		if (! no_cb_attr)
			nim_error(  ERR_SET_BL_NO_CL_BT, NULL, NULL, NULL);

		/* If we are going to reboot anyway or if force_push was */
		/* specified, reset the flag indicating that we don't    */
		/* need to call initiate_bootp to just set the boot list.*/
		if (do_reboot || ! nim_client_push)
			bootlist_only = FALSE;
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

	/* if "-q" used, it must be the first arg */
	if (! strcmp( argv[1], "-q" ) ) {
		mstr_what(valid_attrs, resources);
		exit( 0 );
	}
	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
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
	
/*---------------------------- undo              ------------------------------
*
* NAME: undo    
*
* FUNCTION:
*		undoes operations performed in bos_inst
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
*			target
*			alloc_list
*
* RETURNS: (void)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

void
undo(	int errno,
		char *str1,
		char *str2,
		char *str3,
		int  deallocate_res)

{	int i,j;
	struct nim_object *obj;
	struct nim_attr script;
	FILE *c_stdout = NULL;

	disable_err_sig();

	/* errno given? */
	if ( errno > 0 )
		errstr( errno, str1, str2, str3 );

	/* protect the current errstr */
	protect_errstr = TRUE;

	if ( boot_allocd )
	{
		/* deallocate boot resource */
		dealloc_res( target, ATTR_BOOT_T, &c_stdout );
		dealloc_res( target, ATTR_NIM_SCRIPT_T, &c_stdout );
	}

	/* reset resource states */
	reset_server_res( target );

	if ( !deallocate_res )
	{
		/* Remove the transition event which causes 
		 * the resources to get deallocated.
		 */
		rm_attr( target->id, NULL, ATTR_TRANS, 0, NULL );
	}

	/* reset the Cstate */
	Cresult( target->name, RESULT_FAILURE );

	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */

/*---------------------------- ok_to_bos_inst    ------------------------------
*
* NAME: ok_to_bos_inst
*
* FUNCTION:
*		determines whether the specified machine can be installed
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
*			target
*			alloc_list
*
* RETURNS: (int)
*		SUCCESS					= ok to install the machine
*		FAILURE					= not ok - something missing or wrong state or ...
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_bos_inst()

{	int i,j;
	NIM_OBJECT( robj, rinfo )
	struct res_access *raccess;
	int rc = SUCCESS;
	char *msg = NULL;
	char *info = NULL;

	/* check the Cstate - it must be "ready" */
	if ( (i = find_attr( target, NULL, NULL, 0, ATTR_CSTATE )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_CSTATE_T, target->name, NULL );
	if ( ! same_state( target->attrs[i].value, STATE_CREADY ) )
		nim_error( ERR_STATE, target->name, NULL, NULL );

	/* set Cstate to "bos_inst" */
	if ( set_state( target->id, NULL, ATTR_CSTATE, STATE_BOS_INST ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	else if ( add_attr(	&attr_ass, ATTR_IGNORE_STATE, 
								ATTR_IGNORE_STATE_T, "yes" ) == FAILURE )
		undo( 0, NULL, NULL, NULL, TRUE );

	/* remove any old attrs laying around */
	rm_attr( target->id, NULL, ATTR_MISSING, 0, NULL );
	rm_attr( target->id, NULL, ATTR_INFO, 0, NULL );
	rm_attr( target->id, NULL, ATTR_BOOT_INFO, 0, NULL );
	rm_attr( target->id, NULL, ATTR_CRESULT, 0, NULL );
	rm_attr( target->id, NULL, ATTR_TRANS, 0, NULL );

	/* we need to check the allocated resources, so generate the list now */
	if ( LIST_res_access( target, &alloc_list ) == FAILURE )
		undo( 0, NULL, NULL, NULL, TRUE );

	/* verify that all the required resources have been allocated */
	for (i=0; resources[i].name != NULL; i++) 
		if (	(resources[i].required) &&
				(find_res_access( resources[i].pdattr, &alloc_list ) < 0) )
		{	if ( rc == SUCCESS )
			{	/* first error - print message */
				nene( ERR_MISSING_RES, target->name, NULL, NULL );
				protect_errstr = TRUE;
				msg = MSG_msg( MSG_MISSING_RES );
				info = nim_malloc( strlen(msg) + strlen(ATTR_BOS_INST_T) + 1 );
				sprintf( info, msg, ATTR_BOS_INST_T );
         	mk_attr( target->id, NULL, info, 0, ATTR_INFO, ATTR_INFO_T);
			}
         rc = FAILURE;
         mk_attr( target->id, NULL, resources[i].name, 0,
                  ATTR_MISSING, ATTR_MISSING_T);
         fprintf( stderr, "\t%s\n", resources[i].name );
		}

	/*
	 * Got this far so we _MUST_ have a SPOT, cache it for 
	 * later processing.
	 */
	i = find_res_access( ATTR_SPOT, &alloc_list );		
	spot = (struct res_access *) alloc_list.list[i];
	
	if ( rc == FAILURE )
		undo( 0, NULL, NULL, NULL, TRUE );

	/* check for misc errors */
	for (i=0; i < alloc_list.num; i++)
	{
		raccess = (struct res_access *) alloc_list.list[i];

		/* cannot use resources which the target serves to install itself */
		if ( strcmp( target->name, raccess->server ) == 0 )
			undo( ERR_INSTALL_SELF, raccess->name, target->name, target->name, TRUE );

		/* cannot use the pull interface if BOS image already allocated */
		if (	(pull_interface) &&
				(raccess->type == ATTR_MKSYSB) )
			undo( ERR_ATTR_CONFLICT, ATTR_PULL_T, raccess->name, NULL, TRUE );

		/* the lpp_source must have the "simages" attribute */
		if (	(raccess->type == ATTR_LPP_SOURCE) &&
				(get_attr( 	NULL, NULL, get_id(raccess->name), NULL, 0,
								ATTR_SIMAGES ) <= 0) )
			undo( ERR_OBJ_MISSING_ATTR, ATTR_SIMAGES_T, raccess->name, NULL, TRUE );
	}

	/* now, check to see whether this machine serves a resource which is */
	/*		currently allocated */
	i = -1;
	while ( find_attr( target, &i, NULL, 0, ATTR_SERVES ) >= 0 )
	{	/* lock-and-get the resource */
		if ( lag_object( 0, target->attrs[i].value, &robj, &rinfo ) == FAILURE )
			undo( 0, NULL, NULL, NULL, TRUE );

		/* check the allocation count */
		if ( (j = find_attr( robj, NULL, NULL, 0, ATTR_ALLOC_COUNT )) < 0 )
			undo( ERR_OBJ_MISSING_ATTR, ATTR_ALLOC_COUNT_T, robj->name, NULL, TRUE );
		if ( strtol( robj->attrs[j].value, NULL, 0 ) > 0 )
			undo( ERR_SERVER_BUSY, target->name, robj->name, NULL, TRUE );
	}

	return( SUCCESS );

} /* end of ok_to_bos_inst */
	
/* --------------------------- client_going_down 
 * 
 * NAME: client_going_down
 *
 * FUNCTION:
 *		Performs a longjmp back to the initiate_bootp fucntion which  
 * returns SUCCESS. THis function is executed when we get a SISUSR1
 * which is sent to us via a transition event ( ANY -> SHUTDOWN )
 *
 * DATA STRUCTURES:
 *		global:
 *			got_ack_sig - the context to which we switch... 
 *
 * ----------------------------------------------------------------------------*/
void

client_going_down() 
{

	signal(SIGUSR1, SIG_IGN);
	longjmp( got_ack_sig, 0);

} /* end of client_going_down */

/* --------------------------- bld_args 
 *
 * NAME: bld_args 
 *
 * FUNCTION: Takes args and adds them to the list.
 *
 * RETURNS: (int)
 *    SUCCESS  = list added to the list ok
 *    FAILURE  = Ops.. add to list error 
 *
 * ------------------------------------------------------------------- */ 

int
bld_args(va_alist)
va_dcl 
{
   va_list  ptr; 
   LIST *lptr; 

   va_start(ptr); 

   lptr = va_arg(ptr, LIST *); 

   while (*ptr != NULL)
      if (add_to_LIST( lptr, va_arg(ptr, char *) ) !=SUCCESS)
         return(FAILURE);

   return(SUCCESS);

} /* end of bld_args */

/* --------------------------- client_timeout 
 * 
 * NAME: client_timeout
 *
 * FUNCTION: If the rcmd does not come back in a fixed amount of time 
 *		then we're assuming some network problem or configuration problem
 *		exisits.  So when the alarm goes off we need to pretend that the client 
 *		sent us the shutdown state.... 
 *
 * DATA STRUCTURES:
 *		global:	name 	-	The machine obj name.
 *
 * ----------------------------------------------------------------------------*/
void
client_timeout() 
{
	set_state( 0, name, ATTR_MSTATE, STATE_SHUTDOWN);
	exit(0);
} /* end of client_timeout */

/*---------------------------- initiate_bootp    ------------------------------
*
* NAME: initiate_bootp
*
* FUNCTION:
*		attempts to initiate the BOOTP sequence for the target
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure. 
*
*	Iff the initiate_bootp works the client will send a state update 
* of shutdown.  When this occurs we need break out of this master_exec 
* call. This is done by setting up a transistion event to send this	
* process a SIGUSR1 signal.  When we see the SIGUSR1 we execute a longjmp back 
* to this function to return SUCCESS... 
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			nimif					= ptr to nim_if struct for target
*		global:
*
* RETURNS: (int)
*		SUCCESS					= BOOTP initiated
*		FAILURE					= unable to initiate BOOTP
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
initiate_bootp(	struct nim_if *nimif )

{	
	struct nim_attr *info;
 	struct nim_attr *ros_emu;
		
	int rc;
	FILE *c_stdout=NULL;
	char tmp[MAX_VALUE];
	char *ptr;
	LIST args;

	if ( !get_list_space( &args, 20, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* we'll either call initiate_bootp or m_doreboot.  */
	/*  If force_push was specified, we only need to reboot (since	*/
	/*	the boot list was already set in force_push setup). 	*/
	/*  If force_push was not specified, we'll call c_initiate_bootp*/
	/*	If no_client_boot=yes && set_bootlist=yes, we're only 	*/
	/*	going to call c_initiate_bootp to set the boot list.  	*/

	if (! nim_client_push )
	{
		/* build the m_doreboot command with the inst_warning attr */
		sprintf( tmp, "-a%s=%s", ATTR_INST_WARNING_T, "yes");
		if ( bld_args( &args, M_DOREBOOT , tmp, NULL) != SUCCESS )
			nim_error( 0, NULL, NULL, NULL );
		sprintf( tmp, "-a%s=%s", "target", nimif->hostname);
		if ( bld_args( &args, tmp, NULL) != SUCCESS )
			nim_error( 0, NULL, NULL, NULL );
	}
	else
	{
		if ( bld_args( &args, C_INITIATE_BOOTP, NULL) != SUCCESS )
			nim_error( 0, NULL, NULL, NULL );

		/* put in the network adapter name */
		sprintf( tmp, "-a%s=%s", ATTR_ADPT_NAME_T, nimif->adapter );
		if ( bld_args( &args, tmp, NULL) != SUCCESS )
			nim_error( 0, NULL, NULL, NULL );

		/* if we're iplroming put it in the cmd line too */
		if ( (ros_emu = nimattr( target->id, NULL, ATTR_IPLROM_EMU) ) != NULL ) {
			sprintf( tmp, "-a%s=%s", ATTR_IPLROM_EMU_T, ros_emu->value );
			if ( bld_args( &args, tmp, NULL) != SUCCESS )
				nim_error( 0, NULL, NULL, NULL );
		}

		/* expecting the target's ATTR_BOOT_INFO attr to hold the BOOTP info */
		/*		returned from M_ALLOC_BOOT */
		if ( (info = nimattr( target->id, NULL, ATTR_BOOT_INFO )) == NULL )
			return( FAILURE );

	
		/* add the info from ATTR_INFO */
		ptr = strtok( info->value, " " );
		while ( ptr != NULL )
		{
			if ( bld_args( &args, ptr, NULL) != SUCCESS )
				nim_error( 0, NULL, NULL, NULL );
			ptr = strtok( NULL, " " );
		}	
	}

	/* If just called to modify boot list, set no_client_boot attribute */
	/*    for c_initiate_bootp.  (NOTE: bootlist_only is never true if  */
	/*    force_push was specified.)				    */
	if (bootlist_only) 
	{
		sprintf(tmp, "-a%s=%s", ATTR_NO_CLIENT_BOOT_T, "yes");
		if ( bld_args( &args, tmp, NULL) != SUCCESS )
			nim_error( 0, NULL, NULL, NULL );
	}
	else
	{
		/* set up a transition event to kill us when the client shutsdown */
		sprintf( tmp, "-%d %d", SIGUSR1, getpid() );
		if ( add_trans_event( target->id, NULL, ATTR_MSTATE, STATE_ANY, 
				STATE_SHUTDOWN, ATTR_MASTER_T, "/usr/bin/kill", tmp ) == FAILURE )
				undo( 0, NULL, NULL, NULL, TRUE );

		signal(SIGALRM, client_timeout); 
		alarm(90);
		signal(SIGUSR1, client_going_down); 
		if ( setjmp(got_ack_sig) ) 
			return(SUCCESS); 

		sprintf( tmp, "%d %d %d *", ATTR_MSTATE, STATE_ANY, STATE_SHUTDOWN );
	}


	/* invoke C_INITIATE_BOOTP on target or M_DOREBOOT on master */ 
	if (master_exec( nim_client_push ? target->name : ATTR_MASTER_T, 
				&rc, &c_stdout, args.list ) == FAILURE)
	{
		if (! bootlist_only)
			rm_attr(target->id, NULL, ATTR_TRANS, 0, tmp);	
		return( FAILURE );
	}

	if ( rc > 0 )
	{
		if (! bootlist_only)
			rm_attr(target->id, NULL, ATTR_TRANS, 0, tmp);	
		warning( ERR_METHOD, target->name, niminfo.errstr, NULL );
		return( FAILURE );
	}

	/* We really should never get to here (unless bootlist_only is set )*/ 
	return( SUCCESS );

} /* end of initiate_bootp */

/* --------------------------- setup_prospect    
 *
 * NAME: setup_prospect
 *
 * FUNCTION: attempts to setup a new nim client for a bos 
 *		install
 *
 * NOTES:
 *		sets errstr on failure. 
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *	parameters:
 *		nimif	= ptr to nim_if struct for target
 *	global:
 *
 * RETURNS: (int)
 *	SUCCESS		= Setup ok
 *	FAILURE		= oops something went wrong.
 *
 * OUTPUT:
 * ----------------------------------------------------------------------------
 */

int
setup_prospect ( struct nim_if *nimif )

{
	struct nim_attr *info;

	int	rc;
	FILE 	* c_stdout = NULL;
	FILE	* nim_fp; 
	char	*ptr;
	char	*ptr2;

	LIST	args; 
	LIST	list; 

	char	tmp[MAX_VALUE];
	char	location[MAX_VALUE];
	char	grant[MAX_VALUE];
	char	adpt[MAX_VALUE];
	char	client[MAX_VALUE];
	char	boots[MAX_VALUE];
	char	gway[MAX_VALUE];


	/*
	 * Test that we can get to the machine via a ping. Preset the state so 
	 * we know if it changes... 
	 */ 
 	set_state( target->id, NULL, ATTR_MSTATE, STATE_SHUTDOWN );
	set_Mstate( target->id, NULL );
	if ( get_state( target->id, NULL, ATTR_MSTATE) != STATE_RUNNING )
		undo( ERR_NO_PING, target->name, NULL, NULL, FALSE );

	/* 
 	 * Initialize params to C_CH_NFSEXP to export the methods 
	 * directory. Its going to have to be from the master as the 
	 * master is the only machine setup in the prospects .rhost file.
	 */ 
	if ( !get_list_space( &args, 20, TRUE ) ) 
		undo( 0, NULL, NULL, NULL, FALSE );

        sprintf(location, "-a%s=%s",ATTR_LOCATION_T, NIM_METHODS );
        sprintf(grant, "-a%s=%s",ATTR_GRANT_T, nimif->hostname);

	if ( bld_args( &args, C_CH_NFSEXP , location, grant, NULL) != SUCCESS )
		undo( 0, NULL, NULL, NULL, FALSE );

        if ( (client_exec(&rc, &c_stdout, args.list)==FAILURE) || ( rc > 0 ) ) 
                undo( ERR_METHOD, ATTR_MASTER_T, niminfo.errstr, NULL, FALSE );

	reset_LIST( &args );

	/*
	 * Make sure the client has given the master .rhosts access
	 * and will allow push operations
	 */
	sprintf(tmp, "-c%s", nimif->hostname );
	if ( bld_args( &args, M_NNC_SETUP , tmp, NULL) != SUCCESS )
		undo( 0, NULL, NULL, NULL, FALSE );
        if ( (client_exec(&rc, &c_stdout, args.list)==FAILURE) || ( rc > 0 ) ) 
                undo( ERR_METHOD, ATTR_MASTER_T, niminfo.errstr, NULL, FALSE );
	
	reset_LIST( &args );

	/* 
	 * Mount the directory if client is running nfs; otherwise copy files
	 * to client
	 */
	sprintf(tmp, "-m%s", nimif->hostname );
	if ( bld_args( &args, M_NNC_SETUP , tmp, NULL) != SUCCESS )
		undo( 0, NULL, NULL, NULL, FALSE );

	/* Expand the filesystem if necessary? */
	if (do_not_expand == TRUE)
		if ( bld_args( &args, "-z", NULL) != SUCCESS )
			undo( 0, NULL, NULL, NULL, FALSE );

       	if ( (client_exec(&rc, &c_stdout, args.list)==FAILURE) || ( rc > 0 ) ) 
		undo( ERR_METHOD, ATTR_MASTER_T, niminfo.errstr, NULL, FALSE );

	/* 
	 * generate a LIST of res_access structs for mk_niminfo
	 */
	if ( LIST_res_access( target, &list ) == FAILURE )
		undo( 0, NULL, NULL, NULL, FALSE );

	/*
	 * open a connection with the prospect client... we're going to 
	 * over write (or create) a niminfo file. 
	 */
	if ( rcat( target->name, NIM_ENV_FILE, &nim_fp) != SUCCESS )
		undo( ERR_FILE_MODE, "rcat", NIM_ENV_FILE, NULL, FALSE );

	/* 
	 * create the client's .info file 
	 */
	if ( mk_niminfo( target, &list, nim_fp ) != SUCCESS )
		undo( 0, NULL, NULL, NULL, FALSE );

	fclose( nim_fp );

	/*
	 * call the client method to do the setup work 
	 */
	reset_LIST( &args );

	/* Build the argument list */
	if ( bld_args( &args, C_NNC_SETUP, NULL) != SUCCESS )
		undo( 0, NULL, NULL, NULL, FALSE );

	/* Client IP address */
	sprintf( tmp, "-c%s", nimif->ip);
	if ( bld_args( &args, tmp, NULL) != SUCCESS )
		undo( 0, NULL, NULL, NULL, FALSE );

	/* Client network adapter */
	sprintf( tmp, "-n%s", nimif->adapter);
	if ( bld_args( &args, tmp, NULL) != SUCCESS )
		undo( 0, NULL, NULL, NULL, FALSE );

	/* Expand the filesystem if necessary? */
	if (do_not_expand == FALSE)
		if ( bld_args( &args, "-X", NULL) != SUCCESS )
			undo( 0, NULL, NULL, NULL, FALSE );

	/* 
	 * Get information from target's ATTR_BOOT_INFO attr to get the
	 * server and gateway information
	 */ 
	if ( (info = nimattr( target->id, NULL, ATTR_BOOT_INFO )) == NULL )
		undo( 0, NULL, NULL, NULL, FALSE );

	/* Extract the boot server address and the gateway data */
	ptr = strtok( info->value, " " );
	while ( ptr != NULL )
	{
		if (!strncmp(ptr, "-asa=", 5))
		{
			ptr2 = strchr ( ptr, '=' );
			if ( ptr2 != NULL )
			{
			   ptr2++;
				sprintf( tmp, "-b%s", ptr2);
				if ( bld_args( &args, tmp, NULL) != SUCCESS )
					undo( 0, NULL, NULL, NULL, FALSE );
			}
		}
		else if (!strncmp(ptr, "-agw=", 5))
		{
			ptr2 = strchr ( ptr, '=' );
			if ( ptr2 != NULL )
			{
			   ptr2++;
				sprintf( tmp, "-g%s", ptr2);
				if ( bld_args( &args, tmp, NULL) != SUCCESS )
					undo( 0, NULL, NULL, NULL, FALSE );
			}
		}
		ptr = strtok( NULL, " " );
	}

	if ( (master_exec( target->name, &rc, &c_stdout, args.list)==FAILURE) || ( rc > 0 ) )
                undo( ERR_METHOD, ATTR_MASTER_T, niminfo.errstr, NULL, FALSE );

	reset_LIST( &args );

	return( SUCCESS );
} 
/*---------------------------- bos_inst          ------------------------------
*
* NAME: bos_inst
*
* FUNCTION:
*		initiates the BOS install process
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
*			target
*			alloc_list
*
* RETURNS: (int)
*		SUCCESS					= ok to install the machine
*		FAILURE					= not ok - something missing or wrong state or ...
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
bos_inst()

{	int i;
	struct nim_if nimif;
	long id;
	struct nim_object *obj;
	char tmp[MAX_VALUE];
	struct res_access *raccess;
	int rc;
	FILE *c_stdout=NULL;
	char adpt[MAX_TMP];
	char client[MAX_TMP];
	char ros_emu[MAX_TMP];

	/* prepare for interrupts */
	undo_on_interrupt = undo;

	/* get the target's nim_if info to use for allocation */
	if ( (i = find_attr( target, NULL, NULL, 1, ATTR_IF )) < 0 )
		undo( ERR_OBJ_MISSING_ATTR, PIF, target->name, NULL, TRUE );
	if ( s2nim_if( target->attrs[i].value, &nimif ) == FAILURE )
		undo( ERR_NET_INFO, PIF, target->name, NULL, TRUE );

	/* schedule event transitions */

	VERBOSE("   scheduling event transitions\n",NULL,NULL,NULL,NULL)

	/* Cstate change from "any" to "ready" - deallocate boot resource */
	/*	this must be done BEFORE the SPOT is deallocated because M_DEALLOC_BOOT */
	/*		needs to know where the target's boot image resides */
	/* also, note that ATTR_IGNORE_LOCK is used; this is required because */
	/*		this event will be executed in an environment in which the target */
	/*		will have already been locked by M_CHSTATE, so we must bypass the */
	/*		check in lock_object */
	sprintf(	tmp, "-o %s -F -a%s=yes -a%s=%s %s", 
				ATTR_DEALLOCATE_T, ATTR_IGNORE_LOCK_T, 
				ATTR_SUBCLASS_T, ATTR_SUBCLASS_BOOT_T, target->name );
	if ( add_trans_event(	target->id, NULL, ATTR_CSTATE, 
									STATE_ANY, STATE_CREADY, ATTR_MASTER_T,
									NIM, tmp ) == FAILURE )
		undo( 0, NULL, NULL, NULL, TRUE );

	/* Cstate change from "any" to "ready" - deallocate all resources */
	sprintf(	tmp, "-o %s -F -a%s=yes -a%s=%s %s", 
				ATTR_DEALLOCATE_T, ATTR_IGNORE_LOCK_T,
				ATTR_SUBCLASS_T, ATTR_ALL_T, target->name );
	if ( add_trans_event(	target->id, NULL, ATTR_CSTATE, 
									STATE_ANY, STATE_CREADY, ATTR_MASTER_T,
									NIM, tmp ) == FAILURE )
		undo( 0, NULL, NULL, NULL, TRUE );

	VERBOSE("   setting Rstate for all resources served by %s\n",target->name,
				NULL,NULL,NULL)

	/* make all the resources served by the target "unavailable" */
	if ( reserve_server_res( target ) == FAILURE )
		undo( 0, NULL, NULL, NULL, TRUE );

	/* preserve resources served by this machine? */
	sprintf( tmp, "-a%s=yes -a%s=yes -a %s=yes %s", ATTR_FORCE_T,
				ATTR_IGNORE_STATE_T, ATTR_IGNORE_LOCK_T, target->name );
	if ( preserve_res )
	{
		/* yes, user tells us that the resources served by the target will */
		/*		be available after the install */
		/* so, schedule an event transition to reset the resource states after */
		/*		the target gets installed */
		if ( add_trans_event(	target->id, NULL, ATTR_CSTATE, 
										STATE_ANY, STATE_CREADY, ATTR_MASTER_T,
										M_RESET_RES, tmp ) == FAILURE )
			undo( 0, NULL, NULL, NULL, TRUE );
	}
	else
	{
		/* Cstate "bos_inst_1" to "any" -> remove any resource objects */
		/*		the machine was serving */
		/* the "bos_inst_1" state is a "destructive" state in that the */
		/*		machine's local disks are going to be re-initialized, thus */
		/*		loosing whatever is there currently */
		/* the "destroy" operation removes the object without doing all the */
		/*		checking that's normally done for resource removal */
		if ( add_trans_event(	target->id, NULL, ATTR_CSTATE, 
										STATE_BOS_INST_1, STATE_ANY, ATTR_MASTER_T,
										M_DESTROY_RES, tmp ) == FAILURE )
			undo( 0, NULL, NULL, NULL, TRUE );
	}

	if ( ! remain_client )
	{	
		/* remove the machine object when install operation is finished */
		/* Cstate "bos_inst_3" to "any" -> remove the machine object */
		sprintf( tmp, "-o %s -F -a%s=yes -a%s=yes %s", ATTR_REMOVE_T, 
					ATTR_IGNORE_LOCK_T, ATTR_IGNORE_STATE_T, target->name );
		if ( add_trans_event( target->id, NULL, ATTR_CSTATE, 
										STATE_BOS_INST_3, STATE_ANY, ATTR_MASTER_T,
										NIM, tmp ) == FAILURE )
			undo( 0, NULL, NULL, NULL, TRUE );
	}

	/* allocate the nim_script resource (which performs customization) */
	/* processing will occur in the BOOT environment, so we need */
	/*		ATTR_BOOT_ENV also (so that we get the proper pathname offsets) */
	if (	(add_attr(&attr_ass,ATTR_BOOT_ENV,ATTR_BOOT_ENV_T,"yes") == FAILURE) ||
			(alloc_res( target, ATTR_NIM_SCRIPT_T, &nimif, &c_stdout ) == FAILURE))
		undo( 0, NULL, NULL, NULL, TRUE );
	rm_attr_ass( &attr_ass, ATTR_ASYNC );
	rm_attr_ass( &attr_ass, ATTR_BOOT_ENV );

	/* allocate the network boot resource */
	if ( alloc_res( target, ATTR_BOOT_T, &nimif, &c_stdout ) == FAILURE )
		undo( 0, NULL, NULL, NULL, TRUE );
	boot_allocd = TRUE;

	/* retrieve the object again now that we have new resources. */
	odm_free_list( target, &tinfo );
	if ( lag_object( 0, name, &target, &tinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	if ( !nim_client_push ) {
		VERBOSE("   setup prospect client\n",NULL,NULL,NULL,NULL);
		/* If setup prospect fails, it will error off and exit */
		setup_prospect( &nimif );
	}
	/* 
	 * attempt to initiate the BOOTP sequence 
	 *    (or just set the bootlist)
	 */
	if ( do_reboot || bootlist_only ) {
		if (bootlist_only)
		{
			VERBOSE("   attempting to set client boot list\n",NULL,NULL,NULL,NULL)
		}
		else
		{
			VERBOSE("   attempting to boot client\n",NULL,NULL,NULL,NULL)
			VERBOSE("   initiate_bootp \n",NULL,NULL,NULL,NULL)
		}
		if ( initiate_bootp( &nimif ) == FAILURE ) {
			/* warn the user */
			if (bootlist_only)
				warning( ERR_SET_BOOTLIST, target->name, NULL, NULL );
			else
				warning( ERR_INITIATE_BOOTP, target->name, NULL, NULL );
			mk_attr( target->id, NULL, niminfo.errstr, 0, ATTR_INFO, ATTR_INFO_T );

		}
		else
			rm_attr( target->id, NULL, ATTR_INFO, 0, NULL );
	}
	
	/* all done & EVERYTHING worked! amazing but true */
	undo_on_interrupt = NULL;
	return( SUCCESS );

} /* end of bos_inst */

/**************************       main         ********************************/

main(	int argc,
		char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	/* initialize the LISTs */
	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );
	if ( ! get_list_space( &alloc_list, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_bos_inst: perform BOS install on %s\n",name,NULL,NULL,NULL)

	/* check for missing attrs */
	ck_attrs();

	/* lock-and-get the object */
	if ( lag_object( 0, name, &target, &tinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* ok to bos install this machine? */
	ok_to_bos_inst();

	/* go for it! */
	bos_inst();

	exit( 0 );
}

