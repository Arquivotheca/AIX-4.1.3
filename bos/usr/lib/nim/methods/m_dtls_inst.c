static char	sccs_id[] = " @(#)79 1.12  src/bos/usr/lib/nim/methods/m_dtls_inst.c, cmdnim, bos411, 9439C411f  9/30/94  17:38:16";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_dtls_init.c
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
extern void undo();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{ATTR_SIZE,					ATTR_SIZE_T,				FALSE,	valid_pdattr_ass},
	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE,	NULL}
};

VALID_ATTR resources[] =
{	{ATTR_ROOT,					ATTR_ROOT_T,				TRUE,		NULL},
	{ATTR_SPOT,					ATTR_SPOT_T,				TRUE,		NULL},
	{ATTR_PAGING,				ATTR_PAGING_T,				FALSE,	NULL},
	{ATTR_DUMP,					ATTR_DUMP_T,				TRUE,		NULL},
	{ATTR_HOME,					ATTR_HOME_T,				FALSE,	NULL},
	{ATTR_SHARED_HOME,		ATTR_SHARED_HOME_T,		FALSE,	NULL},
	{ATTR_TMP,					ATTR_TMP_T,					FALSE,	NULL},
	{0,							NULL, 						0,			NULL}
};

char *name=NULL;							/* NIM name of object to create */
NIM_OBJECT( mobj, minfo )				/* object being operated on */
struct nim_if nimif;						/* client's primary interface info */
LIST allocd;								/* res_access LIST for alloc'd res */
LIST params;								/* LIST of parameters for methods */
char tmp_param[MAX_VALUE];				/* used for tmp parameters */

/* info for undo */
struct res_access *root=NULL;
struct res_access *paging=NULL;
struct res_access *dump=NULL;
int root_created = FALSE;
int paging_created = FALSE;
int dump_created = FALSE;
int boot_created = FALSE;

/*---------------------------- dtls_init          ------------------------------
*
* NAME: dtls_init
*
* FUNCTION:
*		initiates the diskless install process
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
*			mobj
*			allocd
*
* RETURNS: (int)
*		SUCCESS					= diskless resource initialized
*		FAILURE					= not ok - something missing or wrong state or ...
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
dtls_init()

{	int has_init_root=0, i;
	char args[MAX_VALUE];
	FILE *c_stdout = NULL;
	NIM_ATTR( server, sinfo )

	/* prepare for interrupts */
	undo_on_interrupt = undo;

	/* schedule a transition event to remove the NIM_MK_DATALESS env variable */
	/*		from the client's .info file */
	/* this variable is used as a flag to rc.dd_boot; when present, it */
	/*		indicates that a local paging space is to be initialized */
	/* to do this, we need the name of the SPOT server */
	if ( (i = find_attr( mobj, NULL, NULL, 0, ATTR_SPOT )) < 0 )
		undo( ERR_OBJ_MISSING_ATTR, ATTR_SPOT_T, mobj->name, NULL );
	if ( get_attr( &server, &sinfo, get_id(mobj->attrs[i].value),
						NULL, 0, ATTR_SERVER ) <= 0 )
		undo( ERR_OBJ_MISSING_ATTR, ATTR_SERVER_T,mobj->attrs[i].value,NULL);

	/* the client's .info file resides in BOOT_LOCATION on the SPOT server */
	sprintf( args, "-alocation=%s/%s.info %s=", BOOT_LOCATION, nimif.hostname,
				NIM_MK_DATALESS_T );

	/* If root already good do not need this transition event						*/
	has_init_root=(find_attr( mobj, NULL, NULL, 0, ATTR_ROOT_INITIALIZED) >= 0);
	if ( ! has_init_root ) {
		/* remove NIM_MK_DATALESS when Cstate changes from dtls_init to dd_boot */
		if ( add_trans_event( mobj->id, NULL, 
									ATTR_CSTATE, STATE_DTLS_INIT, STATE_DD_BOOT,
									server->value, C_NIMINFO, args ) == FAILURE )
			undo( 0, NULL, NULL, NULL );
	}

	VERBOSE("   initializing the client's paging file\n",NULL,NULL,NULL,NULL)

	/* create a paging file */
	if ( find_res_access( ATTR_PAGING, &allocd ) >= 0 )
	{
		if ( mk_paging( TRUE ) == FAILURE )
			undo( 0, NULL, NULL, NULL );
	}

	VERBOSE("   initializing the client's dump file\n",NULL,NULL,NULL,NULL)

	/* create a dump file */
	if ( mk_dump() == FAILURE )
		undo( 0, NULL, NULL, NULL );

	VERBOSE("   allocating a network boot resource\n",NULL,NULL,NULL,NULL)

	/* allocate the network boot resource */
	/* this is done BEFORE the root is created because mk_root needs the */
	/*		info supplied by the ATTR_BOOT_INFO attribute, which is created */
	/*		when the boot resource is allocated */
   if (  (add_attr_ass( &attr_ass, ATTR_IGNORE_STATE,
                        ATTR_IGNORE_STATE_T, "yes", 0 ) == FAILURE) ||
         (alloc_res( mobj, ATTR_BOOT_T, &nimif, &c_stdout ) == FAILURE) )
      undo( 0, NULL, NULL, NULL );
   boot_created = TRUE;

	/* re-get the client's object in order to pick up new attrs */
	odm_free_list( mobj, &minfo );
	if ( lag_object( 0, name, &mobj, &minfo ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	VERBOSE("   initializing the client's root directory\n",NULL,NULL,NULL,NULL)

	/* make a new root directory */
	if ( mk_root() == FAILURE )
		undo( 0, NULL, NULL, NULL );

	if (has_init_root) 
		/* make the Cstate dd_boot if we already did all that root monkey buss. */
		set_state( mobj->id, NULL, ATTR_CSTATE, STATE_DD_BOOT );

	return( SUCCESS );

} /* end of dtls_init */

/**************************       main         ********************************/

main(	int argc,
		char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );

	/* initialize the LISTs */
	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );
	if ( ! get_list_space( &params, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_dtls_init: perform dataless initialization for %s\n",name,NULL,
				NULL,NULL)

	/* check for missing attrs */
	ck_attrs();

	/* lock-and-get the object */
	if ( lag_object( 0, name, &mobj, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* ok to initialize dataless resources for this machine? */
	if ( ok_to_dd_inst( ATTR_DATALESS, ATTR_DTLS_INIT_T ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* set the Cstate */
	if ( set_state( mobj->id, NULL, ATTR_CSTATE, STATE_DTLS_INIT) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* go for it! */
	dtls_init();

	exit( 0 );
}

