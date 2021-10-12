static char sccsid[] = "@(#)51  1.12  src/bos/usr/lib/nim/methods/m_dkls_inst.c, cmdnim, bos411, 9439C411f 9/30/94 17:38:05";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_mkres.c
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
	{ATTR_VERBOSE,				ATTR_VERBOSE_T,				FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE,	NULL}
};

VALID_ATTR resources[] =
{	{ATTR_ROOT,					ATTR_ROOT_T,				TRUE,		NULL},
	{ATTR_SPOT,					ATTR_SPOT_T,				TRUE,		NULL},
	{ATTR_PAGING,				ATTR_PAGING_T,				TRUE,		NULL},
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

/*---------------------------- dkls_init          ------------------------------
*
* NAME: dkls_init
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
dkls_init()

{	FILE *c_stdout = NULL;

	/* prepare for interrupts */
	undo_on_interrupt = undo;

	VERBOSE("   initializing the client's paging file\n",NULL,NULL,NULL,NULL)

	/* create a paging file */
	if ( mk_paging( TRUE ) == FAILURE )
		undo( 0, NULL, NULL, NULL );

	VERBOSE("   initializing the client's dump file\n",NULL,NULL,NULL,NULL)

	/* create a dump file */
	if ( mk_dump() == FAILURE )
		undo( 0, NULL, NULL, NULL );

	VERBOSE("   allocating a network boot resource\n",NULL,NULL,NULL,NULL)

	/* allocate the network boot resource */
	/* this is done BEFORE the root is created because mk_root needs the */
	/*		info supplied by the ATTR_BOOT_INFO attribute, which is created */
	/*		when the boot resource is allocated */
	/* also, ignore the current Cstate */
	if (	(add_attr_ass( &attr_ass, ATTR_IGNORE_STATE, 
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

	/* remember that we've initialized the client's root directory */
	ch_attr( mobj->id, NULL, "yes", 0, ATTR_ROOT_INITIALIZED,
				ATTR_ROOT_INITIALIZED_T );

	/* set the Cstate to dd_boot */
	Cresult( mobj->name, RESULT_SUCCESS );
	undo_on_interrupt = NULL;

	return( SUCCESS );

} /* end of dkls_init */

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

	VERBOSE("m_dkls_inst: perform diskless initialization for %s\n",name,NULL,
				NULL,NULL)

	/* check for missing attrs */
	ck_attrs();

	/* lock-and-get the object */
	if ( lag_object( 0, name, &mobj, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* ok to initialize diskless resources for this machine? */
	if ( ok_to_dd_inst( ATTR_DISKLESS, ATTR_DKLS_INIT_T ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

   /* set the new Cstate */
	if ( set_state( mobj->id,NULL,ATTR_CSTATE, STATE_DKLS_INIT ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* go for it! */
	dkls_init();

	exit( 0 );
}

