static char   sccs_id[] = " @(#)14 1.20  src/bos/usr/lib/nim/methods/m_alloc_ns.c, cmdnim, bos41J, 9511A_all  3/6/95  09:16:32";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_alloc_ns.c
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

extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{0,							NULL,								FALSE, ch_pdattr_ass}
};

char *name=NULL;							/* target object name */
char *server=NULL;						/* server of nim_script */
char *pathname=NULL;						/* pathname of nim_script */

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

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:q")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
			break;

			case 'q': /* display valid_attrs */
				cmd_what( valid_attrs );
				exit( 0 );
			break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

			case '?': /* unknown option */
				nim_error(	ERR_BAD_OPT, optopt, 
								MSG_msg(MSG_GENERIC_ALSYNTAX), NULL );
			break;
		}
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_ALSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/*---------------------------- mk_nim_script     ------------------------------
*
* NAME: mk_nim_script
*
* FUNCTION:
*		constructs a NIM customization script at the specified location
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <client> was retrieved using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			client				= ptr to nim_object for client
*			alloc_list			= LIST of res_access structs
*			server				= name of machine where script is to reside
*			location				= pathname of the script file
*		global:
*			attr_ass
*
* RETURNS: (int)
*		SUCCESS					= script file created
*		FAILURE					= nothing to do or odm error or rcat error
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mk_nim_script(	struct nim_object *client,
					LIST *alloc_list,
					char *server,
					char *location )

{	int installp_flags;
	int mk_nimclient;
	int instfix = FALSE;
	int fixes = FALSE;
	int fix_bundle = FALSE;
	int installp_bundle = FALSE;
	int filesets = FALSE;
	int async;
	struct res_access *raccess;
	struct res_access *simages=NULL;
	int cust_present=FALSE;
	ODMQUERY
	struct nim_pdattr pdattr;
	FILE *fp;
	int i,j;
	int local;
	char tmp[MAX_TMP];
	char *chmod[] = { CHMOD, "755", location, NULL };
	int rc;
	FILE *c_stdout = NULL;
	struct nim_if nimif;
	int pif = -1;
	int snm = -1;
	int ring_speed = -1;
	int cable_type = -1;
	NIM_OBJECT( net, ninfo )
	char cmd_offset[MAX_TMP];
	int debug = FALSE;
	char *bundle = NULL;
	LIST buns;
	int boot_env;
	int net_type;

	VERBOSE("   building the nim_script\n",NULL,NULL,NULL,NULL)

	/* initialize installp_bundle LIST */
	if ( get_list_space( &buns, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		return( FAILURE );

	/* look for attr flags */
	mk_nimclient = (find_attr_ass( &attr_ass, ATTR_NO_NIM_CLIENT ) < 0);
	async = (find_attr_ass( &attr_ass, ATTR_ASYNC ) >= 0);
	installp_flags = find_attr_ass( &attr_ass, ATTR_INSTALLP_FLAGS );
	filesets = (find_attr_ass( &attr_ass, ATTR_INSTALLP_OPTIONS ) >= 0);
	fixes = (find_attr_ass( &attr_ass, ATTR_FIXES ) >= 0);
	boot_env = ( find_attr_ass( &attr_ass, ATTR_BOOT_ENV ) >= 0 );
	if ( boot_env )
		strcpy( cmd_offset, SPOT_OFFSET );
	else
		cmd_offset[0] = NULL_BYTE;
	debug = ( find_attr_ass( &attr_ass, ATTR_DEBUG ) >= 0 );

	/* reserve the first entry in <buns> for installp_options or fixes */
	if (fixes)
	{
		if ( add_to_LIST( &buns, 
				attr_value(&attr_ass,ATTR_FIXES) ) == FAILURE )
			return( FAILURE );
	}
	else
	{
		if ( add_to_LIST( &buns, 
				attr_value(&attr_ass,ATTR_INSTALLP_OPTIONS) ) == FAILURE )
			return( FAILURE );
	}

	/* look for lpp_source and other cust type resources */
	VERBOSE("   looking for cust type resources\n",NULL,NULL,NULL,NULL)
	for (i=0; i < alloc_list->num; i++)
	{	
		raccess = (struct res_access *) alloc_list->list[i];
	
		VERBOSE2("   Checking \"%s\"\n",raccess->name,NULL,NULL,NULL)
		/* what kind of resource? */
		switch (raccess->type)
		{
			case ATTR_LPP_SOURCE:
			   VERBOSE2("   found \"%s\"\n",raccess->name,NULL,NULL,NULL)
				simages = raccess;
			break;

			case ATTR_INSTALLP_BUNDLE:
			case ATTR_FIX_BUNDLE:
			   VERBOSE2("   found \"%s\"\n",raccess->name,NULL,NULL,NULL)
				if (raccess->type == ATTR_INSTALLP_BUNDLE)
					installp_bundle = TRUE;
				else if (raccess->type == ATTR_FIX_BUNDLE)
					fix_bundle = TRUE;

				/* is resource local or remote to the client? */
				if ( strcmp( client->name, raccess->server ) == 0 )
				{
					/* bundle is local to client */
					bundle = raccess->location;
				}
				else
				{
					/* bundle is remote to client */
					bundle = nim_malloc( strlen(raccess->nimif.hostname) +
												strlen(raccess->location) + 2 );
					sprintf( bundle, "%s:%s", raccess->nimif.hostname, 
								raccess->location );
				}

				/* add it to the installp_bundle LIST */
				if ( add_to_LIST( &buns, bundle ) == FAILURE )
					return( FAILURE );
			break;

			default:
				if ( attr_in_subclass( raccess->type, ATTR_SUBCLASS_CUST ) ) {
			   	VERBOSE2("   found \"%s\"\n",raccess->name,NULL,NULL,NULL)
					cust_present = TRUE;
				}
			break;

		} /* switch */
	} /* for */

	/* check for conflicts (can't mix-n-match fix attributes and */
     	/* 	resources with installp attributes and resources).   */
        if (fixes && filesets)
                nim_error(  ERR_ATTR_CONFLICT, ATTR_FIXES_T,
                                ATTR_INSTALLP_OPTIONS_T, NULL);
        if (fixes && installp_bundle)
                nim_error(  ERR_ATTR_RES_CONFLICT, ATTR_FIXES_T,
                                ATTR_INSTALLP_BUNDLE_T, NULL);
        if (fix_bundle && filesets)
                nim_error(  ERR_ATTR_RES_CONFLICT, ATTR_INSTALLP_OPTIONS_T,
				ATTR_FIX_BUNDLE_T, NULL);
        if (fix_bundle && installp_bundle)
                nim_error(  ERR_RES_CONFLICT, ATTR_FIX_BUNDLE_T,
                                ATTR_INSTALLP_BUNDLE_T, NULL);

	instfix = (fixes || fix_bundle);

	/* check to see if lpp_source is required */
	if ( (mk_nimclient) || (*buns.list[0] != NULL_BYTE) || (buns.num > 1) )
	{
		if ( simages == NULL )
		{
			nene( ERR_MISSING_RES, client->name, NULL, NULL );
			fprintf( stderr, "\t%s\n", ATTR_LPP_SOURCE_T );
			return( FAILURE );
		}
		else
			cust_present = TRUE;
	}

	/* must have at least one customization resource to continue */
	if (mk_nimclient) {
		if ( ! cust_present ) {
		   VERBOSE("   No cust type resources found\n",NULL,NULL,NULL,NULL)
		   if ( simages != NULL )
			ERROR( ERR_NO_FILESETS, NULL, NULL, NULL )
		   else
			ERROR( ERR_NO_CUST, client->name, NULL, NULL )
		}
	}

   /* if we get here, we've got something we need to build a cust script for */
   /* where should we build the script? */
   if ( strcmp( niminfo.nim_name, server ) == 0 )
   {  
		/* script will be local to current machine */
      /* open the FILE now */
      if ( (fp = fopen( location, "w" )) == NULL )
         ERROR( ERR_ERRNO, NULL, NULL, NULL )
   }
   else
   {  
		/* script will reside on another machine */
      /* open a remote file pointer */
      if ( rcat( server, location, &fp ) == FAILURE )
         return( FAILURE );
   }

	/* if going to be NIM client, then need the primary interface info */
	if ( mk_nimclient )
	{
		/* get client's primary IF and convert it */
		if ( (pif = find_attr( client, NULL, NULL, 1, ATTR_IF )) < 0 )
			ERROR( ERR_OBJ_MISSING_ATTR, PIF, client->name, NULL )
		else if ( s2nim_if( client->attrs[pif].value, &nimif ) == FAILURE )
			ERROR( ERR_NET_INFO, PIF, client->name, NULL )
		else if ( (net_type = a2net_type( nimif.adapter )) == -1 )
			ERROR( 0, NULL, NULL, NULL )

		/* get object corresponding to the client's primary net */
		if ( lag_object( 0, nimif.network, &net, &ninfo ) == FAILURE )
			return( FAILURE );

		/* find the ATTR_SNM */
		if ( (snm = find_attr( net, NULL, NULL, 0, ATTR_SNM )) < 0 )
			ERROR( ERR_OBJ_MISSING_ATTR, ATTR_SNM_T, net->name, NULL )

		/* ATTR_RING_SPEED required? */
		if (	(net_type == ATTR_TOK) &&
				((ring_speed = 
						find_attr( client, NULL, NULL, 1, ATTR_RING_SPEED )) < 0) )
			ERROR( ERR_OBJ_MISSING_ATTR, ATTR_RING_SPEED_T, client->name, NULL )

		/* ATTR_CABLE_TYPE required? */
		if (	(net_type == ATTR_ENT) &&
				((cable_type = 
						find_attr( client, NULL, NULL, 1, ATTR_CABLE_TYPE )) < 0) )
			ERROR( ERR_OBJ_MISSING_ATTR, ATTR_CABLE_TYPE_T, client->name, NULL )
	}

	/* ready to generate the script */

	/* header info */
	fprintf( fp, "#!/bin/ksh\n\n" );
	fprintf( fp, "# %s for %s\n", ATTR_NIM_SCRIPT_T, client->name );
	fprintf( fp, "result=%s\n\n", RESULT_SUCCESS_T );

	/* include NIM client initialization? */
	if ( mk_nimclient )
	{
		fprintf( fp, MSG_msg (MSG_NIMC_INIT));
		if ( debug )
			fprintf(	fp, "%s%s -v \\\n", SPOT_OFFSET, C_MK_NIMCLIENT );
		else
			fprintf(	fp, "%s%s \\\n", SPOT_OFFSET, C_MK_NIMCLIENT );
			fprintf( fp, "\t-a%s=%s \\\n", ATTR_HOSTNAME_T, nimif.hostname );
		fprintf( fp, "\t-a%s=%s \\\n", ATTR_IP_T, nimif.ip );
		if ( ring_speed >= 0 )
			fprintf( fp, "\t-a%s=%s \\\n", ATTR_RING_SPEED_T, 
						client->attrs[ring_speed].value );
		if ( cable_type >= 0 )
			fprintf( fp, "\t-a%s=%s \\\n", ATTR_CABLE_TYPE_T, 
						client->attrs[cable_type].value );
		fprintf( fp, "\t-a%s=%s\n", ATTR_SNM_T, net->attrs[snm].value );

		/* check the return code */
		fprintf( fp, "[[ $? != 0 ]] && result=%s\n\n", RESULT_FAILURE_T );
	}

	/* NOTE that we want to construct this script such that installp */
	/*		operations are performed first, then scripts are executed */
	/* this is VERY important so that customers can count on the order in */
	/*		which customization is performed */

	/* look for installp resources */
	for (i=0; i < alloc_list->num; i++)
	{	
		raccess = (struct res_access *) alloc_list->list[i];

		/* is resource local or remote to the client? */
		if ( strcmp( client->name, raccess->server ) == 0 )
			local = TRUE;
		else
			local = FALSE;

		/* what kind of resource? */
		switch (raccess->type)
		{	
			case ATTR_LPP_SOURCE:
				if (	(raccess == simages) && 
						(mk_nimclient) &&
						(*buns.list[0] == NULL_BYTE) &&
						(buns.num <= 1) )
					/* skip the next part - SIMAGES already being used for */
					/*		some other purpose */
					continue;

				/* install using options and/or bundles */
				for (j=0; j < buns.num; j++)
				{
					if ( (j==0) && (*buns.list[0] == NULL_BYTE) )
						/* no ATTR_INSTALLP_OPTIONS or IATTR_FIXES were passed */
						continue;

					fprintf( fp, "# %s=%s\n", ATTR_LPP_SOURCE_T, raccess->name );
					if ( debug )
						fprintf( fp,"%s%s -v \\\n", cmd_offset, C_INSTALLP );
					else
						fprintf( fp,"%s%s \\\n", cmd_offset, C_INSTALLP );

					/* ATTR_BOOT_ENV specified by caller? */
					if ( boot_env )
						fprintf( fp, "\t-a%s=yes \\\n", ATTR_BOOT_ENV_T );

					if ( j == 0 )
					{
						/* ATTR_INSTALLP_OPTIONS occupy the first entry in <buns> */
						if (instfix)

							fprintf( fp, "\t-a%s=\"%s\" \\\n", ATTR_FIXES_T,
									buns.list[j] );
						else
							fprintf( fp, "\t-a%s=\"%s\" \\\n", ATTR_INSTALLP_OPTIONS_T,
									buns.list[j] );
					}
					else
					{
						if (instfix)
							fprintf( fp, "\t-a%s=%s \\\n", ATTR_FIX_BUNDLE_T,
									buns.list[j] );
						else
							fprintf( fp, "\t-a%s=%s \\\n", ATTR_INSTALLP_BUNDLE_T,
									buns.list[j] );
					}

					/* use flags? */
					if ( installp_flags >= 0 )
						fprintf( fp,"\t-a%s=\"%s\" \\\n", ATTR_INSTALLP_FLAGS_T, 
									attr_ass.list[ installp_flags ]->value ); 

					if ( local )
						fprintf( fp, "\t-a%s=%s\n", ATTR_LPP_SOURCE_T,
									raccess->location );
					else
						fprintf( fp, "\t-a%s=%s:%s\n", ATTR_LPP_SOURCE_T,
									raccess->nimif.hostname, raccess->location );

					/* check the return code */
					fprintf( fp, "[[ $? != 0 ]] && result=%s\n\n", RESULT_FAILURE_T);
				}
			break;
		}
	}

	/* NOW look for scripts */
	for (i=0; i < alloc_list->num; i++)
	{	
		raccess = (struct res_access *) alloc_list->list[i];

		/* is resource local or remote to the client? */
		if ( strcmp( client->name, raccess->server ) == 0 )
			local = TRUE;
		else
			local = FALSE;

		/* what kind of resource? */
		switch (raccess->type)
		{	
			case ATTR_SCRIPT:
				fprintf( fp, "# %s=%s\n", ATTR_SCRIPT_T, raccess->name );
				if ( debug )
					fprintf( fp, "%s%s -v \\\n", cmd_offset, C_SCRIPT );
				else
					fprintf( fp, "%s%s \\\n", cmd_offset, C_SCRIPT );
				if ( local )
					fprintf( fp, "\t-a%s=\"%s\"\n", ATTR_LOCATION_T,
								raccess->location );
				else
					fprintf( fp, "\t-a%s=\"%s:%s\"\n", ATTR_LOCATION_T,
								raccess->nimif.hostname, raccess->location );

				/* check the return code */
				fprintf( fp, "[[ $? != 0 ]] && result=%s\n\n", RESULT_FAILURE_T );
			break;
		}
	}

	if ( async )
	{	
		/* add a call to nimclient so that we know when the script finishes */
		fprintf( fp, MSG_msg(MSG_CHG_CONTROL_STATE) );
		fprintf( fp, "%s%s -R ${result}\n\n", cmd_offset, NIMCLIENT );
	}

	/* 
	 * if we dont want to be a nim client after the install
	 * and this is the boot environment then put a rm /etc/niminfo 
	 * into the /etc/firstboot
	 */ 
	if ( !mk_nimclient && boot_env ) { 
		fprintf( fp, "%s \"%s -f %s\" >> %s\n", ECHO, RM, NIM_ENV_FILE, FIRSTBOOT);
	}
	/* if any failure occurs within the script, exit non-zero */
	fprintf( fp, "[[ ${result} = %s ]] && exit 0 || exit 1\n\n",
				RESULT_SUCCESS_T );

	/* all done with the FILE */
	fclose( fp );

	/* set execute perms for the script */
	if (	(master_exec( server, &rc, &c_stdout, chmod ) == FAILURE) ||
			(rc > 0) )
		return( FAILURE );

	return( SUCCESS );

} /* end of mk_nim_script */

/*---------------------------- undo              ------------------------------
*
* NAME: undo
*
* FUNCTION:
*		backs out changes made by alloc_nimscript
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
*			errno					= err_code
*			str1					= err str1
*			str2					= err str2
*			str3					= err str3
*		global:
*			server
*			pathname
*
* RETURNS: (void)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

void
undo(	int errno,
		char *str1,
		char *str2,
		char *str3 )

{	int rc;
	FILE *c_stdout=NULL;
	char *args[] = { RM, "-f", pathname, NULL };

	disable_err_sig();

	/* initialize the errstr */
	if ( errno > 0 )
		errstr( errno, str1, str2, str3 );
	protect_errstr = TRUE;

	/* remove nim_script if it was created */
	if ( (server != NULL) && (pathname != NULL) )
		master_exec( server, &rc, &c_stdout, args );

	/* exit with error */
	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */

/*------------------------- alloc_nimscript            -------------------------
*
* NAME: alloc_nimscript
*
* FUNCTION:
*		creates a nim_script for the specified client
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
*		SUCCESS					= boot resource allocated
*		FAILURE					= error encountered
*
* OUTPUT:
*		writes nim_script location to stdout
*-----------------------------------------------------------------------------*/

int
alloc_nimscript()

{	NIM_OBJECT( client, cinfo )
	NIM_OBJECT( res, rinfo )
	LIST list;
	int i,j;
	char *server;
	char *location;
	struct nim_if nimif;
	struct nim_if server_nimif;
	int Cstate;

	/* get the client's object */
	if ( lag_object( 0, name, &client, &cinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* cache the current Cstate (this operation which called us) */
	if ( (i = find_attr( client, NULL, NULL, 0, ATTR_CSTATE )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_CSTATE_T, client->name, NULL );
	Cstate = (int) strtol( client->attrs[i].value, NULL, 0 );

	/* find the client's PIF */
	if ( (i = find_attr( client, NULL, NULL, 1, ATTR_IF )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, PIF, client->name, NULL );

	/* convert to nim_if */
	if ( s2nim_if( client->attrs[i].value, &nimif ) == FAILURE )
		nim_error( ERR_NET_INFO, PIF, client->name, NULL );

	/* generate a LIST of res_access structs */
	if ( LIST_res_access( client, &list ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* get the nim_script object */
	if ( lag_object( 0, ATTR_NIM_SCRIPT_T, &res, &rinfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* get the server & location of the nim_script (right now, it's the master */
	if ( (i = find_attr( res, NULL, NULL, 0, ATTR_SERVER )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T, res->name, NULL );
	else if ( (j = find_attr( res, NULL, NULL, 0, ATTR_LOCATION )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_LOCATION_T, res->name, NULL );
	server = res->attrs[i].value;
	location = res->attrs[j].value;
	
	/* construct the pathname of the client's nim_script */
	pathname = nim_malloc(	strlen(location) + strlen(client->name) + 9 );
	sprintf( pathname, "%s/%s.script", location, client->name );

	VERBOSE("   nim_script pathname is %s:%s\n",server,pathname,NULL,NULL)

	/* allocation checks */
	if ( ok_to_allocate( client, nimif.network, res, &server_nimif ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* construct the client's nim_script */
	if ( mk_nim_script( client, &list, server, pathname ) == FAILURE )
	{
		/* if the calling operation is bos_inst... */
		if ( Cstate == STATE_BOS_INST )
		{
			/* ignore certain errors */
			if (	(niminfo.errno == ERR_NO_FILESETS) || 
					(niminfo.errno == ERR_NO_CUST) )
				return( SUCCESS );
		}

		nim_error( 0, NULL, NULL, NULL );
	}
	undo_on_interrupt = undo;

	/* finish allocation (administrative stuff) */
	if ( do_allocation( client, res, pathname, &server_nimif, NULL ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	/* success! */
	uaf_object( res, &rinfo, FALSE );
	undo_on_interrupt = NULL;
	return( SUCCESS );

} /* end of alloc_nimscript */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_alloc_ns: allocate a nim_script to %s\n",name,NULL,NULL,NULL)

	/* create a nim_script for the specified client */
	alloc_nimscript();

	exit( 0 );

}
