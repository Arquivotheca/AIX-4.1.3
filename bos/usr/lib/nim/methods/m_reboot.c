static char     sccs_id[] = " @(#)75 1.4  src/bos/usr/lib/nim/methods/m_reboot.c, cmdnim, bos41J, 9521B_all  5/25/95  08:48:58";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_reboot.c
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cmdnim_mstr.h"
#include <varargs.h>


extern int	valid_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] = 
{
	{ ATTR_INST_WARNING, ATTR_INST_WARNING_T, FALSE, valid_pdattr_ass},
	{ 0, NULL, FALSE, valid_pdattr_ass }
};


char	*name = NULL;	/* object name */
int	inst_warn=TRUE; /* says whether re-install warning should be walled on target */
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

/* --------------------------- do_reboot
 *
 * NAME: do_reboot
 *
 * FUNCTION: Cause (or attempt to cause) the machine represented by the object name 
 *	to reboot.
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
 *
 * RETURNS: (int)
 *
 * OUTPUT:
 * ----------------------------------------------------------------------------
 */

void
do_reboot()

{
	NIM_OBJECT( target, info )

	struct nim_if nimif;
	int	rc,i;
	int	Cstate;
	LIST 	args;
	char tmp[MAX_VALUE];

	if ( !get_list_space( &args, 20, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* 
	 * lock-and-get the object 
	 */
	if ( lag_object( 0, name, &target, &info ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	/*
         * Test that we can get to the machine via a ping. Preset the state so 
	 * we know if it changes... 
	 */
	VERBOSE("	testing access to machine", NULL, NULL, NULL, NULL);
	set_state( target->id, NULL, ATTR_MSTATE, STATE_SHUTDOWN );
	set_Mstate( target->id, NULL );
	if ( get_state( target->id, NULL, ATTR_MSTATE) != STATE_RUNNING )
		nim_error( ERR_NO_PING, target->name, NULL, NULL);

	/* 
	 * get the target's nim_if 
	 */
	if ( (i = find_attr( target, NULL, NULL, 1, ATTR_IF )) < 0 )
		nim_error( ERR_OBJ_MISSING_ATTR, PIF, target->name, NULL );

	if ( s2nim_if( target->attrs[i].value, &nimif ) == FAILURE )
		nim_error( ERR_NET_INFO, PIF, target->name, NULL );

	/* build the m_doreboot command with the target attribute */
	sprintf( tmp, "-a%s=%s", "target", nimif.hostname);
	if ( bld_args( &args, M_DOREBOOT, tmp, NULL) != SUCCESS )
		nim_error( 0, NULL, NULL, NULL );

	if (inst_warn)
	{
		/* Add the inst_warning attribute IF the client has */
		/* a state of BOS Install Has been enabled.         */
		if ((Cstate = get_state( target->id, NULL, ATTR_CSTATE )) == STATE_BOS_INST)
		{
			sprintf( tmp, "-a%s=%s", ATTR_INST_WARNING_T, "yes");
			if ( bld_args( &args, tmp, NULL, NULL) != SUCCESS )
				nim_error( 0, NULL, NULL, NULL );
		}
	}

	/* 
	 * Call the reboot script (it'll be in background)
	 */
	if ( (client_exec(&rc, NULL, args.list)==FAILURE) || ( rc > 0 ) ) 
		nim_error( ERR_METHOD, ATTR_MASTER_T, niminfo.errstr, NULL );
}

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
*			argc		= argc from main
*			argv		= argv from main
*		global:
*
* RETURNS: (int)
*		SUCCESS			= no syntax errors on command line
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
parse_args(	int argc, 
				char *argv[] )

{	extern char *optarg;
	extern int optind, optopt;
	int c,i;

	/* if "-q" used, it must be the first arg */
	if (! strcmp( argv[1], "-q" ) ) {
		mstr_what(valid_attrs, NULL);
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

	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_COP_SYNTAX), NULL, NULL );

	name = argv[optind];

	/* Check if optional attrs were specified. */
	for (i=0; i < attr_ass.num; i++)
		if ( attr_ass.list[i]->pdattr == ATTR_INST_WARNING )
		{
			if ( (strcmp( attr_ass.list[i]->value, "yes") != 0) &&
			     (strcmp( attr_ass.list[i]->value , "no" ) != 0) )
				nim_error( ERR_ATTR_YES_NO,
				ATTR_INST_WARNING_T, NULL, NULL);
			inst_warn = (strcmp( attr_ass.list[i]->value, "yes") == 0);
		}


	/* return */
	return( SUCCESS );

} /* end of parse_args */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* 
	 * initialize NIM environment 
	 */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

        /* parse command line */
        parse_args( argc, argv );

	VERBOSE("m_reboot: rebooting %s\n", name, NULL, NULL, NULL)
	do_reboot();
	exit( 0 );
}
