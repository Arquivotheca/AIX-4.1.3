static char sccs_id[] = "@(#)38	1.15  src/bos/usr/lib/nim/methods/m_chmac.c, cmdnim, bos411, 9428A410j  6/23/94  11:00:07"; 
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_chmac.c
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

extern int	valid_pdattr_ass();
extern int	ch_pdattr_ass();

/* --------------------------- module globals    ---------------------------- */
ATTR_ASS_LIST attr_ass;
VALID_ATTR valid_attrs[] = 
{	
	{ ATTR_IF, 				ATTR_IF_T, 					FALSE, 	ch_pdattr_ass },
	{ ATTR_CONTROL,		ATTR_CONTROL_T, 			FALSE, 	ch_pdattr_ass },
	{ ATTR_PULL_REQUEST,	ATTR_PULL_REQUEST_T, 	FALSE, 	ch_pdattr_ass },
	{ ATTR_COMMENTS,		ATTR_COMMENTS_T,			FALSE, 	ch_pdattr_ass },
	{ ATTR_CPUID,			ATTR_CPUID_T,				FALSE, 	ch_pdattr_ass },
	{ ATTR_INFO,			ATTR_INFO_T,				FALSE, 	ch_pdattr_ass },
	{ ATTR_FORCE,			ATTR_FORCE_T,				FALSE, 	ch_pdattr_ass },
	{ ATTR_RING_SPEED,	ATTR_RING_SPEED_T,		FALSE, 	ch_pdattr_ass },
	{ ATTR_CABLE_TYPE,	ATTR_CABLE_TYPE_T,		FALSE, 	ch_pdattr_ass },
	{ ATTR_IPLROM_EMU,	ATTR_IPLROM_EMU_T,		FALSE, 	ch_pdattr_ass },
	{ 0, 						NULL, 						FALSE, 	NULL }
};

char	*name = NULL;
NIM_OBJECT( mobj, minfo )

/*---------------------------- undo              ------------------------------
*
* NAME: undo
*
* FUNCTION:
*		undoes changes made by ch_mac
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
*			err					= error code
*			str1					= string 1 of err msg
*			str2					= string 2 of err msg
*			str3					= string 3 of err msg
*		global:
*			mobj
*			minfo
*
* RETURNS: (int)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

void
undo(	int err,
		char *str1,
		char *str2,
		char *str3 )

{

	/* no interruptions */
	disable_err_sig();

	/* set errstr? */
	if ( err > 0 )
		errstr( err, str1, str2, str3 );
	protect_errstr = TRUE;

	/* restore the object's original values */
	rest_object( mobj, &minfo );

	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */

/*---------------------------- ok_to_ch_if       ------------------------------
*
* NAME: ok_to_ch_if
*
* FUNCTION:
*		determines whether it's ok to change an interface for a machine object
*			by checking to see if the machine serves a resource which is
*			currently allocated to another machine
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			mobj					= ptr to nim_object struct for machine
*		global:
*
* RETURNS: (int)
*		SUCCESS					= ok to change interface
*		FAILURE					= not ok
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ok_to_ch_if(	struct nim_object *mobj )

{	int rc=SUCCESS;
	NIM_OBJECT( robj, rinfo )
	int i,j;

	/* for each resource this machines serves */
	i = -1;
	while ( (i = find_attr( mobj, &i, NULL, 0, ATTR_SERVES )) >= 0 )
	{	/* get the resource info */
		if ( lag_object( 0, mobj->attrs[i].value, &robj, &rinfo ) > 0 )
		{	/* find the allocation count */
			if ( (j = find_attr( robj, NULL, NULL, 0, ATTR_ALLOC_COUNT )) >= 0 )
			{	/* is this resource allocated? */
				if ( strtol( robj->attrs[j].value, NULL, 0 ) > 0 )
					ERROR( ERR_SERVER_BUSY, mobj->name, mobj->attrs[i].value, NULL )
			}

			uaf_object( robj, &rinfo, FALSE );
		}
		else
			return( FAILURE );
	}

	return( SUCCESS );

} /* end of ok_to_ch_if */
	
/* --------------------------- update_control
 *
 * NAME: update_control
 *
 * FUNCTION: changes the control attribute for each resource object
 *		"owned" by this machine. 
 *
 * RETURNS: (int)
 *		SUCCESS	= changed
 *		FAILURE	= error encountered
 * --------------------------------------------------------------------------*/
int
update_control(int ndx)

{  
   NIM_OBJECT( robj, rinfo )
   int i,j;

   /* for each resource this machines serves */
   i = -1;
   while ( (i = find_attr( mobj, &i, NULL, 0, ATTR_SERVES) ) >= 0 ) 
   {  /* get the resource info */
      if ( lag_object( 0, mobj->attrs[i].value, &robj, &rinfo ) > 0 )
      {  /* update the control attr, its ok if errno is ERR_ATTR_RM_DNE */
			if ( ch_attr(	robj->id, NULL, attr_ass.list[ndx]->value,
				attr_ass.list[ndx]->seqno, attr_ass.list[ndx]->pdattr,
				attr_ass.list[ndx]->name ) == FAILURE )
				if ( niminfo.errno != ERR_ATTR_RM_DNE )
					undo( 0, NULL, NULL, NULL );
         uaf_object( robj, &rinfo, FALSE );
      }
      else
			undo( 0, NULL, NULL, NULL );
   }

   return( SUCCESS );
}
/* --------------------------- ch_mac
 *
 * NAME: ch_mac
 *
 * FUNCTION: changes attributes for a machine object
 *
 * NOTES:
 *	calls nim_error
 *
 * DATA STRUCTURES:
 *	parameters:
 *	global:
 *		name
 *
 * RETURNS: (int)
 *		SUCCESS	= attrs changed
 *		FAILURE	= error encountered
 * ----------------------------------------------------------------------------*/

int
ch_mac()

{	
	int	rc = SUCCESS;
	int	Cstate = 0;
	int	i;
	int if_change = FALSE;

	/* backup all info about current object */
	if ( lag_object( 0, name, &mobj, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	if ( (i = find_attr( mobj, NULL, NULL, 0, ATTR_CSTATE )) >= 0 )
		Cstate = strtol( mobj->attrs[i].value, NULL, 0 );
	else
		nim_error( ERR_BAD_OBJECT, ATTR_CLASS_MACHINES, name, NULL );

	VERBOSE("   verifying that a change may be performed at this time\n",NULL,
				NULL,NULL,NULL)

	/* ok to make a change? */
	if (	(! force) &&
			(Cstate != STATE_CREADY) &&
			(Cstate != STATE_INCOMPLETE) )
		nim_error( ERR_STATE, name, NULL, NULL );

	/* any network interface changes? */
	/*  -- Or how about control changes ? */
	for (i=0; i < attr_ass.num; i++) {
		switch ( attr_ass.list[i]->pdattr ) {
			case ATTR_IF:
				/* verify that the ATTR_IF is in the correct format */
				if (	(attr_ass.list[i]->value[0] != NULL_BYTE) &&
						(verify_if( 	mobj->id, attr_ass.list[i]->seqno,
										attr_ass.list[i]->value, NULL ) == FAILURE) )
					nim_error( 0, NULL, NULL, NULL );

				/* NOTE: no "break" here because we want to continue on with */
				/*		common processing */

			case ATTR_RING_SPEED:
			case ATTR_CABLE_TYPE:
				if_change = TRUE;
				/* make sure it's ok to change the machines's interfaces */
				if ( ok_to_ch_if( mobj ) == FAILURE )
					nim_error( 0, NULL, NULL, NULL );
			break; 	

			case ATTR_CONTROL:
				update_control(i);
			break;   

		}
	}
	/* prepare for interrupts */
	undo_on_interrupt = undo;

	/* remove any ATTR_MISSING attrs */
	rm_attr( mobj->id, NULL, ATTR_MISSING, 0, NULL );

	VERBOSE("   changing attribute values\n",NULL,NULL,NULL,NULL)

	/* change the specified attrs */
	for (i = 0; i < attr_ass.num; i++)
			if ( ch_attr(	mobj->id, NULL, attr_ass.list[i]->value,
		    				attr_ass.list[i]->seqno, attr_ass.list[i]->pdattr,
		    				attr_ass.list[i]->name ) == FAILURE )
				if ( niminfo.errno != ERR_ATTR_RM_DNE )
					undo( 0, NULL, NULL, NULL );

	if ( if_change )
	{
		VERBOSE("   verifying change to network interface\n",NULL,NULL,NULL,NULL)

		/* check the definition */
		if ( set_cstate( mobj->id, NULL ) == STATE_INCOMPLETE )
			undo( 0, NULL, NULL, NULL );
	}

	/* successful changes */
	undo_on_interrupt = NULL;
	return( SUCCESS );

} /* end of ch_mac */

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
 *		argc = argc from main
 *		argv = argv from main
 *	global:
 *
 * RETURNS: (int)
 *	SUCCESS	= no syntax errors on command line
 *
 * OUTPUT:
 * ----------------------------------------------------------------------------*/

int
parse_args(	int argc, 
				char *argv[] )

{	
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;
	int	syntax_err = FALSE;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:q")) != -1 ) {	
		switch (c) {
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
			nim_error(ERR_BAD_OPT, optopt, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL);
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL, NULL );
	}

	/* only one operand and it had better NOT be master */
	if ( (optind != (argc - 1)) || (strcmp( argv[optind], ATTR_MASTER_T ) == 0) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL, NULL );

	name = argv[optind];

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/**************************       main         ********************************/

main( int argc, char *argv[] )
{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
	if ( !get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_chmac: change an attribute of the %s machine\n",name,NULL,NULL,
				NULL)

	/* change the specified attrs */
	ch_mac();

	exit( 0 );
}
