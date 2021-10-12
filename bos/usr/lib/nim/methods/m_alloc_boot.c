static char     sccs_id[] = " @(#)85 1.18.1.5  src/bos/usr/lib/nim/methods/m_alloc_boot.c, cmdnim, bos41J, 9520A_all  5/15/95  16:23:23";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: add_nim_hosts
 *		add_nim_mounts
 *		alloc_boot
 *		main
 *		mk_niminfo
 *		parse_args
 *		undo
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "cmdnim_mstr.h"

extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{0,							NULL,							FALSE,	ch_pdattr_ass}
};

char *name=NULL;							/* target object name */
int boot_image=FALSE;					/* >0 if boot image was created */
int info_file=FALSE;						/* >0 if info file was created */
struct nim_if nimif;						/* nim_if struct for client */
char nimfo_path[MAX_TMP];				/* .info file pathname for client */
struct res_access *spot = NULL;		/* res_access struct for SPOT */

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
				mstr_what( valid_attrs, NULL );
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
	
/*---------------------------- undo              ------------------------------
*
* NAME: undo
*
* FUNCTION:
*		backs out changes made by alloc_boot
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
*			str1					= err str1
*			str2					= err str2
*			str3					= err str3
*		global:
*			nimif
*			spot
*			boot_image
*			info_file
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
	char attr[MAX_VALUE];
	char *booti[] = { C_DEALLOC_BOOT, attr, NULL };
	char *infof[] = { RM, "-fr", nimfo_path, NULL };

	disable_err_sig();

	/* initialize the errstr */
	if ( errno > 0 )
		errstr( errno, str1, str2, str3 );
	protect_errstr = TRUE;

	/* remove boot resource? */
	if ( boot_image )
	{
		sprintf( attr, "-a%s=%s", ATTR_HOSTNAME_T, nimif.hostname );
		master_exec( spot->server, &rc, &c_stdout, booti );
	}

	/* remove info file? */
	if ( info_file )
		master_exec( spot->server, &rc, &c_stdout, infof );

	/* bye-bye */
	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */
	
/*------------------------- alloc_boot            ----------------------------
*
* NAME: alloc_boot
*
* FUNCTION:
*		allocates a network boot image to the specified machine
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
*----------------------------------------------------------------------------*/

int
alloc_boot()

{	NIM_OBJECT( target, minfo )
	NIM_OBJECT( boot, binfo )
	struct nim_attr *snm_attr;
	int net_type;
	char *net_type_str;
	struct nim_route nimroute;
	int i;
	LIST list;
	LIST c_args; 
	char *hostname;
	char *bf;
	char ip[MAX_TMP];
	char ht[MAX_TMP];
	char ha[MAX_TMP];
	char gw[MAX_TMP];
	char sm[MAX_TMP];
	char sa[MAX_TMP]; 
	int rc;
	FILE *c_stdout = NULL;
	FILE *nimfo_fp; 
	int platform;
	char *zp;

	VERBOSE("m_alloc_boot: allocating boot resource to %s\n",name,NULL,NULL,NULL)

	/* get the target object */
	if ( lag_object( 0, name, &target, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* check for errors */
	switch ( get_state( target->id, NULL, ATTR_CSTATE ) )
	{
		case STATE_BOS_INST :
		case STATE_DTLS_INIT :
		case STATE_DKLS_INIT :
		case STATE_DD_BOOT :
		case STATE_DIAG :
			/* correct states for this method */
		break;

		default :
			nim_error( ERR_STATE, name, NULL, NULL );
		break;
	}

	/* boot resource already allocated? */
	if ( (i = find_attr( target, NULL, NULL, 0, ATTR_BOOT )) >= 0 )
		nim_error( ERR_ALREADY_ALLOC, ATTR_BOOT_T, name, NULL );

	/* get info about target's primary net for use later on */
	if ( (i = find_attr( target, NULL, NULL, 1, ATTR_IF )) == -1 )
		nim_error( ERR_OBJ_MISSING_ATTR, PIF, target->name, NULL );
	else if ( s2nim_if( target->attrs[i].value, &nimif ) == FAILURE )
		nim_error( ERR_NET_INFO, PIF, target->name, NULL );
	else if ( (net_type = a2net_type( nimif.adapter )) == -1 )
		nim_error( 0, NULL, NULL, NULL );
	else if ( (snm_attr = nimattr( 0, nimif.network, ATTR_SNM )) == NULL )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_SNM, nimif.network, NULL );
	net_type_str = ATTR_msg( net_type );

	/* find the target's platform type */
	if ( (platform = find_attr( target, NULL, NULL, 0, ATTR_PLATFORM )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, ATTR_PLATFORM_T, target->name, NULL );

   if ( get_list_space(&c_args, 30, TRUE) == FALSE )
			nim_error(0, NULL, NULL, NULL);

	/* generate a LIST of res_access structs */
	if ( LIST_res_access( target, &list ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* find the SPOT in the LIST */
	for (i=0; i < list.num; i++)
	{	spot = (struct res_access *) list.list[i];
		if ( spot->type == ATTR_SPOT )
			break;
		spot = NULL;
	}
	if ( spot == NULL )
	{	
		nene( ERR_MISSING_RES, name, NULL, NULL );
		fprintf( stderr, "\t%s\n", ATTR_SPOT_T );
		nim_error( 0, NULL, NULL, NULL );
	}

	/* initialize parameter string for C_ALLOC_BOOT */

	if (add_to_LIST( &c_args, C_ALLOC_BOOT ) !=SUCCESS)
				return(FAILURE);

	hostname = nim_malloc( strlen(ATTR_HOSTNAME_T) + strlen(nimif.hostname) + 4);
	sprintf( hostname, "-a%s=%s", ATTR_HOSTNAME_T, nimif.hostname );
	if (add_to_LIST( &c_args, hostname ) !=SUCCESS)
				return(FAILURE);

	sprintf( ip, "-a%s=%s", ATTR_IP_T, nimif.ip );
	if (add_to_LIST( &c_args, ip ) !=SUCCESS)
				return(FAILURE);

	sprintf( sm, "-a%s=%s", ATTR_SM_T, snm_attr->value );
	if (add_to_LIST( &c_args, sm ) !=SUCCESS)
				return(FAILURE);

	sprintf( ht, "-a%s=%s", ATTR_HT_T, net_type_str );
	if (add_to_LIST( &c_args, ht ) !=SUCCESS)
				return(FAILURE);
	/*
 	 * If the hardware address is zeros then the user wishes 
	 * to rely on the ip address only. Do not put the hardware 
	 * address into the command line. 
	 */
	for (zp=nimif.hard_addr; (zp != NULL) && (*zp != '\0') && (*zp == '0'); zp++);
	if ((zp == nimif.hard_addr) ||
		((zp != nimif.hard_addr) && (*zp != '\0')))
	    
	{
		sprintf( ha, "-a%s=%s", ATTR_HA_T, nimif.hard_addr );
		if (add_to_LIST( &c_args, ha ) !=SUCCESS)
			return(FAILURE);
	}
	else
		ha[0] = NULL_BYTE;

	sprintf( sa, "-a%s=%s", ATTR_SA_T, spot->nimif.ip );
	if (add_to_LIST( &c_args, sa ) !=SUCCESS)
				return(FAILURE);

	/* construct boot image filename using this format: */
	/*		<SPOT name>.<platform type>.<network interface type> */
	bf = nim_malloc(	strlen(ATTR_BF_T) + strlen(TFTPBOOT) + 
							strlen(spot->name) +
							strlen(target->attrs[platform].value) +
							strlen(net_type_str) + 10 );
	sprintf(	bf, "-a%s=%s/%s.%s.%s", ATTR_BF_T, TFTPBOOT, spot->name, 
				target->attrs[platform].value, net_type_str );
	if (add_to_LIST( &c_args, bf ) !=SUCCESS)
				return(FAILURE);

	/* is SPOT server on same net as target? */
	if ( strcmp( nimif.network, spot->nimif.network ) )
	{	
		/* no, on different networks; gateway info needed for BOOTP */
		/* get routing info between target's net & spot server */
		if ( net_to_net(	nimif.network, spot->nimif.network, 
								&nimroute ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );

		sprintf( gw, "-a%s=%s", ATTR_GW_T, nimroute.gateway );
		if (add_to_LIST( &c_args, gw ) !=SUCCESS)
				return(FAILURE);
	}
	else
		gw[0] = NULL_BYTE;

	VERBOSE("   invoking %s\n",C_ALLOC_BOOT,NULL,NULL,NULL)

	/* invoke C_ALLOC_BOOT on SPOT server */
	if ( master_exec( spot->server,&rc,&c_stdout, c_args.list ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		nim_error( ERR_METHOD, spot->server, niminfo.errstr, NULL );
	boot_image = TRUE;

	/* if we get here, we've created stuff that needs to be undone on error */
	undo_on_interrupt = undo;

	/* initialize the target's .info file */

	/* open a remote fd */
	sprintf( nimfo_path, "%s/%s.info", TFTPBOOT, nimif.hostname ); 
	if ( rcat( spot->server, nimfo_path, &nimfo_fp) != SUCCESS )
		undo( ERR_FILE_MODE, "rcat", nimfo_path, NULL );

	/* create the target's .info file */
	if ( mk_niminfo( target, &list, nimfo_fp ) != SUCCESS ) 
		undo( 0, NULL, NULL, NULL );
	fclose( nimfo_fp );
	info_file = TRUE;

	/* finish allocation */
	if ( lag_object( 0, ATTR_BOOT_T, &boot, &binfo ) == FAILURE )
		undo( 0, NULL, NULL, NULL );
	if ( do_allocation( target, boot, NULL, &(spot->nimif), NULL ) == FAILURE )
		undo( 0, NULL, NULL, NULL );
	uaf_object( boot, &binfo, FALSE );

	/* write info to stdout so that it can be used to initiate BOOTP */
	if ( gw[0] == NULL_BYTE )
	{
		/* The following compensates for flaws in the function   */
		/* init_bootlist_attrs where the decision to generate    */
		/* client ip address is based upon the existence of a    */
		/* gateway address.  The code has been fixed there but   */
		/* providing this here means that nim code on the client */
		/* need not be updated to benefit from the fix.  	 */
		sprintf( gw, "-a%s=%s", ATTR_GW_T, "0.0.0.0");
	}
	printf( "%s= %s %s %s %s %s\n", ATTR_BOOT_INFO_T, ip, ha, gw, sm, sa );

	/* success! */
	return( SUCCESS );

} /* end of alloc_boot */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	/* allocate a network boot resource */
	alloc_boot();

	exit( 0 );

}
