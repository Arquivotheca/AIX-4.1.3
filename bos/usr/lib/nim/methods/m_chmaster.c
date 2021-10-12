static char	sccs_id[] = " @(#)52 1.17  src/bos/usr/lib/nim/methods/m_chmaster.c, cmdnim, bos411, 9428A410j  6/14/94  16:26:42";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ch_master
 *		main
 *		master_if_ass
 *		parse_args
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

extern int master_if_ass();
extern int valid_pdattr_ass();
extern int ch_pdattr_ass();

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{
 	{ATTR_IF,					ATTR_IF_T,					FALSE,	master_if_ass},
 	{ATTR_RING_SPEED,			ATTR_RING_SPEED_T,		FALSE,	valid_pdattr_ass},
 	{ATTR_CABLE_TYPE,			ATTR_CABLE_TYPE_T,		FALSE,	valid_pdattr_ass},
 	{ATTR_COMMENTS,			ATTR_COMMENTS_T,			FALSE,	ch_pdattr_ass},
 	{ATTR_MASTER_PORT,		ATTR_MASTER_PORT_T,		FALSE,	NULL},
	{0,							NULL, 						FALSE,	NULL}
};

char *name=ATTR_MASTER_T;
	
/*---------------------------- master_if_ass --------------------------------
*
* NAME: master_if_ass
*
* FUNCTION:
*		used as a "wrapper" around ch_pdattr_ass so that we can catch attr
*			assignments rejected due to the state of a network
*		this is required because it's kind of a "catch-22" condition:
*			1) the network's NSTATE can only be "ready" if it has a route to the
*					MASTER, but
*			2) can't connect a machine to a network that's not "ready", so...
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			list					= ptr to ATTR_ASS_LIST
*			name					= pdattr.name
*			value					= value for the pdattr
*			seqno					= seqno for the pdattr
*			dummy					= not used; only included to match parameters
*										passed by parse_attr_ass
*		global:
*
* RETURNS: (int)
*		SUCCESS					= valid attr assignment
*		FAILURE					= invalid assignment
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
master_if_ass(	ATTR_ASS_LIST *list,
					char *name,
					char *value,
					int seqno,
					int dummy )

{	int rc=SUCCESS;
	ODMQUERY
	struct nim_pdattr pdattr;
	NIM_ATTR( curif, curifinfo )
	NIM_ATTR( macifs, minfo )
	struct nim_if old_nimif;
	struct nim_if new_nimif;
	int removing_access=FALSE;
	int i;
	int cstate;
	long	master_id;


	/* the master's interface may be changed under the following conditions: */
	/*		1) this is a new interface (ie., an addition) */
	/*		2) there are no "active" machine objects which think they should use */
	/*				this interface to communicate with the master */

	/* need the nim_pdattr.attr value for <name> */
	sprintf( query, "name=%s", name );
	if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
		ERROR( ERR_VALUE, name, MSG_msg(MSG_ATTR_NAME), NULL )
	/* need the masters id, so get it... */
	if ( ( master_id = get_id(ATTR_MASTER_T)) == 0 )
		ERROR( ERR_DNE, ATTR_MASTER_T, NULL, NULL )
	/* nothing extra to do if this is a new interface; otherwise... */
	if ( get_attr( &curif, &curifinfo, master_id, NULL, seqno,pdattr.attr)>0)
	{	/* change to an existing interface - got some checking to do */

		/* convert the current interface stanza */
		if ( s2nim_if( curif->value, &old_nimif ) == FAILURE )
			return( FAILURE );

		/* is the new value NULL? */
		if ( value[0] == '\0' )
		{	/* this means that a currently defined interface is being removed */
			removing_access = TRUE;
		}
		else
		{	/* changing some field within the stanza */
			/* in order to figure out which field is changing, we need to */
			/* 	compare the old & new stanzas */
			if ( s2nim_if( value, &new_nimif ) == FAILURE )
				return( FAILURE );
			if ( strcmp( old_nimif.network, new_nimif.network ) )
			{	/* changing what network the interface is connected to */
				/* this change will remove access to the master for those */
				/*		machines currently using that network, so... */
				removing_access = TRUE;
			}
		}
		
		/* will access to the master be removed? */
		if ( removing_access )
		{	/* check the activity on that network & all connected networks */
			/* if there is any NIM activity which would be damaged from a */
			/*		change to this network's routing, then disallow the change */
			/* NOTE that ok_to_chnet will lock all effected objects, so if */
			/*		we are able to continue, we can effect the change without */
			/*		worrying about other NIM ops changing things under us */
			if ( ok_to_chnet( 0, old_nimif.network ) == FAILURE )
				return( FAILURE );
		}
	}
	else if ( value[0] == '\0' )
	{	/* not an existing interface, therefore, a new interface, which means */
		/*		that the value field must be supplied */
		if ( seqno )
			sprintf( query, "%s%d=", name, seqno );
		else
			sprintf( query, "%s=", name );
		ERROR( ERR_MISSING_VALUE, query, NULL, NULL )
	}

	/* if we get here, it's ok to change this interface */
	/* validate the attr assignment */
	if ( (rc = valid_pdattr_ass( list, name, value, seqno, TRUE )) == FAILURE )
	{	/* was failure due to the network state? */
		if ( niminfo.errno == ERR_STATE )
		{	/* ignore the state - accept the attr */
			sprintf( query, "name=%s", name );
			if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
				ERROR( ERR_VALUE, name, MSG_msg(MSG_ATTR_NAME), NULL )
			return( add_attr_ass( list, pdattr.attr, name, value, seqno ) );
		}
	}

	return( rc );

} /* end of master_if_ass */

/*---------------------------- ch_master             ---------------------------
*
* NAME: ch_master
*
* FUNCTION:
*		changes attributes for the "master"
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
*		SUCCESS					= attrs changed
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ch_master()

{	int rc=SUCCESS;
	NIM_OBJECT( mobj, minfo )
	NIM_OBJECT( after_ch, ainfo )
	ODMQUERY
	struct nim_pdattr pdattr;
	int current_cstate=0;
	int i;

	/* backup all info about current object */
	if ( lag_object( 0, ATTR_MASTER_T, &mobj, &minfo ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	if ( (i = find_attr( mobj, NULL, NULL, 0, ATTR_CSTATE )) >= 0 )
		current_cstate = strtol( mobj->attrs[i].value, NULL, 0 );

	/* remove any ATTR_MISSING attrs */
	rm_attr( mobj->id, NULL, ATTR_MISSING, 0, NULL );

	VERBOSE("   changing the specified attributes\n",NULL,NULL,NULL,NULL)

	/* change the specified attrs */
	for (i=0; i < attr_ass.num; i++)
	{
		/* look for ATTR_IF assignments */
		if ( strcmp( attr_ass.list[i]->name, ATTR_IF_T) == 0 )
		{
			/* verify that the stanza is in the correct format, etc */
			if (	(attr_ass.list[i]->value[0] != NULL_BYTE) &&
					(verify_if( mobj->id, attr_ass.list[i]->seqno, 
									attr_ass.list[i]->value, NULL ) == FAILURE) )
			{
				rc = FAILURE;
				break;
			}
		}

		if ( ch_attr(	mobj->id, NULL, attr_ass.list[i]->value,
							attr_ass.list[i]->seqno, attr_ass.list[i]->pdattr,
							attr_ass.list[i]->name ) == FAILURE )
		{	rc = FAILURE;
			break;
		}
	}

	if ( rc == SUCCESS )
	{
		/* check attrs associated with ATTR_IF */
		if ((rc = get_object(&after_ch, &ainfo, 0, ATTR_MASTER_T, 0, 0))==SUCCESS)
			rc = ck_if_attrs( after_ch, &ainfo );
	}

	/* any failures? */
	if ( rc == FAILURE )
	{	protect_errstr = TRUE;
		rest_object( mobj, &minfo );
		nim_error( 0, NULL, NULL, NULL );
	}

	VERBOSE("   checking the Nstate of all network objects\n",NULL,NULL,NULL,
				NULL)

	/* set all the ATTR_NSTATEs (network states) & connected machines */
	set_all_nstates();

	/* now check the master's definition */
	set_cstate( mobj->id, NULL );

	return( SUCCESS );

} /* end of ch_master */

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
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, TRUE ) )
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
				nim_error(ERR_BAD_OPT, optopt, MSG_msg(MSG_GENERIC_CHSYNTAX),NULL);
				break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_GENERIC_CHSYNTAX), NULL, NULL );
	}

	/* only one operand and it must be "master" */
	if ( (optind != (argc-1)) || (strcmp( argv[optind], ATTR_MASTER_T )) )
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

	if ( ! get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_chmaster: change an attribute of the master\n",NULL,NULL,NULL,
				NULL)

	/* create a new network object */
	ch_master();

	exit( 0 );

}
