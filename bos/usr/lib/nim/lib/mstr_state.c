static char sccs_id[] = " @(#)88 1.32  src/bos/usr/lib/nim/lib/mstr_state.c, cmdnim, bos411, 9434B411a  8/24/94  15:02:33";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ck_mac_def
 *		exec_trans_event
 *		same_state
 *		set_cstate
 *		set_state
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


/******************************************************************************
 *
 *                        NIM master function library
 *
 * this library contains NIM database manipulation routines, which should only
 *		be accessed by the NIM master
 ******************************************************************************/

#include "cmdnim.h"
#include "cmdnim_obj.h"
#include <varargs.h> 

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ transition event processing                      $$$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- add_trans_event   ------------------------------
*
* NAME: add_trans_event
*
* FUNCTION:
*		adds the specified transition event to the specified object's definition
*		the object may be specified by either its <id> or <name>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		sets errstr on failure
*		this function ASSUMES that <obj> has been retrieved using lag_object
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= id of nim_object
*			name					= name of nim_object
*			state_attr			= which state
*			old_state			= old state
*			new_state			= new state
*			target				= machine to execute <cmd> on
*			cmd					= command to execute upon specified transition
*			args					= args for <cmd>
*		global:
*
* RETURNS: (int)
*		SUCCESS					= transition event added
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
add_trans_event(	long id,
						char *name,
						int state_attr,
						int old_state,
						int new_state,
						char *target,
						char *cmd,
						char *args )

{	char *tmp;
	int rc=SUCCESS;

	VERBOSE3("      add_trans_event: id=%d; state_attr=%d; target=%s; cmd=%s;\n",
				id,state_attr,target,cmd)

	/* malloc enough space to initialize the trans event string */
	tmp = (char *) nim_malloc( strlen(target) + strlen(cmd) + 
										strlen(args) + 12 );

	/* initialize the string */
	sprintf( tmp, "%d %d %d %s %s %s", state_attr, old_state, new_state,
				target, cmd, args );

	/* add an ATTR_TRANS for <obj> */
	rc = mk_attr( id, name, tmp, 0, ATTR_TRANS, ATTR_msg(ATTR_TRANS) );

	free( tmp );

	return( rc );

} /* end of add_trans_event */
	
/*---------------------------- exec_trans_event  ------------------------------
*
* NAME: exec_trans_event
*
* FUNCTION:
*		executes the specified transition events
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			trans					= ptr to array of nim_attr structs
*			info					= ptr to listinfo struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= operations performed (maybe)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
exec_trans_event (	struct nim_attr *trans,
							struct listinfo *info )

{	int i,j;
	regmatch_t pmatch[ERE_ATTR_TRANS_NUM];
	char *target;
	int rc;
	char *ptr;
	LIST args;
	int ignore_blanks;

	VERBOSE3("      exec_trans_event: trans=%s;\n",trans->value,NULL,NULL,NULL)

	if ( get_list_space( &args, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		ERROR( 0, NULL, NULL, NULL )

	/* make sure transitions are executed in the order they were added */
	order_attrs( trans, info );

	/* for each transition event... */
	for (i=0; i < info->num; i++)
	{	
		/* first, remove this ATTR_TRANS so that we don't execute it again */
		rm_attr( trans[i].id, NULL, trans[i].pdattr->attr, 
					trans[i].seqno, NULL );

		/* transition events are formatted thus: */
		/*	  <state attr> <current state> <new state> <target> <command> <args> */
		/* separate these fields */
		if (	(regexec(	nimere[ATTR_TRANS_ERE].reg, trans[i].value,
								ERE_ATTR_TRANS_NUM, pmatch, 0 ) != 0) ||
				(pmatch[1].rm_so < 0) ||
				(pmatch[2].rm_so < 0) ||
				(pmatch[3].rm_so < 0) ||
				(pmatch[4].rm_so < 0) ||
				(pmatch[5].rm_so < 0) ||
				(pmatch[6].rm_so < 0) )
			/* trans event wasn't formatted right - ignore & keep going */
			continue;

		/* setup args */

		/* target name */
		trans[i].value[ pmatch[4].rm_eo ] = NULL_BYTE;
		target = trans[i].value + pmatch[4].rm_so;

		/* command to execute */
		trans[i].value[ pmatch[5].rm_eo ] = NULL_BYTE;
		if ( add_to_LIST( &args, (trans[i].value + pmatch[5].rm_so) ) == FAILURE )
			ERROR( 0, NULL, NULL, NULL )

		/* args to the command */
		ptr = trans[i].value + pmatch[6].rm_so;
		ignore_blanks = FALSE;
		j = -1;
		while ( ptr[++j] != NULL_BYTE )
		{
			switch (ptr[j])
			{
				case '"':
				case '\'':
					ignore_blanks = ! ignore_blanks;
				;;

				case ' ':
					if ( ! ignore_blanks )
					{
						/* new arg - separate from others */
						ptr[j] = NULL_BYTE;
						if ( add_to_LIST( &args, ptr ) == FAILURE )
							ERROR( 0, NULL, NULL, NULL )

						ptr = ptr + j + 1;
						j = -1;
					}
				;;
			}
		}
		if ( (*ptr != NULL_BYTE) && (add_to_LIST( &args, ptr ) == FAILURE) )
			ERROR( 0, NULL, NULL, NULL )

		/* execute the operation */
		master_exec( target, &rc, NULL, &args.list[0] );

		/* reset the parameter LIST */
		reset_LIST( &args );
	}

	free_list( &args );

	return( SUCCESS );

} /* end of exec_trans_event */
	
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ state manipulation                               $$$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- same_state        ------------------------------
*
* NAME: same_state
*
* FUNCTION:
*		compares the ASCII representation of a STATE with its corresponding
*			integer representation
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			str					= string rep
*			state					= integer rep
*		global:
*
* RETURNS: (int)
*		TRUE						= states are the same
*		FALSE						= states are different
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
same_state(	char *str,
				int state )

{	int tmp;

	VERBOSE4("      same_state: str=%s; state=%d\n",str,state,NULL,NULL)

	/* convert string to integer */
	tmp = (int) strtol( str, NULL, 0 );

	if ( tmp == state )
		return( TRUE );

	return( FALSE );

} /* end of same_state */
	
/*---------------------------- set_state        --------------------------------
*
* NAME: set_state
*
* FUNCTION:
*		sets the specified state attr to the specified value for the 
*			object specified by either its <id> or <name>, then executes any
*			transition events which apply to this state change
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		ASSUMES that object has already been locked
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			id						= id of nim_object
*			name					= name of nim_object
*			state_attr			= integer rep of state attribute
*			new_state			= new state
*		global:
*
* RETURNS: (int)
*		SUCCESS					= state set
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
set_state (	long id,
				char *name,
				int state_attr,
				int new_state )

{	ODMQUERY
	NIM_ATTR( trans, tinfo )
	int current_state;
	char tmp[MAX_TMP];

	VERBOSE3("      set_state: id=%d; name=%s; state_attr=%d; new_state=%d;\n",
				id,name,state_attr,new_state)

	/* get id if not passed in */
	if ( (id <= 0) && ((id = get_id( name )) <= 0) )
			return( FAILURE );

	/* get current state */
	current_state = get_state( id, name, state_attr );

	/* set the new & prev state */
	sprintf( tmp, "%d", new_state );
	if ( ch_attr(id, NULL, tmp, 0, state_attr, ATTR_msg(state_attr)) == FAILURE)
		return( FAILURE );
	sprintf( tmp, "%d", current_state );
	ch_attr( id, NULL, tmp, 0, ATTR_PREV_STATE, ATTR_PREV_STATE_T );

	/* if this is an Mstate... */
	if ( state_attr == ATTR_MSTATE )
	{
		/* remove the ATTR_INFO attribute so we don't keep stale info around */
		rm_attr( id, NULL, ATTR_INFO, 0, NULL );
	}

	/* look for state transitions */
	/* first 3 fields are: */
	/*		<state attr > <current state> <new state> */
	/* 3 ways these can be specified */
	
	/* method #1: both current & new state specified explicitly */
	sprintf(	query, "%d %d %d *", state_attr, current_state, new_state );
	if ( get_attr( &trans, &tinfo, id, query, 0, ATTR_TRANS ) > 0 )
	{	exec_trans_event( trans, &tinfo );
		odm_free_list( trans, &tinfo );
	}
	
	/* method #2: current state specified, new state is "any" */
	sprintf(	query, "%d %d %d *", state_attr, current_state, STATE_ANY );
	if ( get_attr( &trans, &tinfo, id, query, 0, ATTR_TRANS ) > 0 )
	{	exec_trans_event( trans, &tinfo );
		odm_free_list( trans, &tinfo );
	}

	/* method #3: current state is "any", new state explicitly specified */
	sprintf(	query, "%d %d %d *", state_attr, STATE_ANY, new_state );
	if ( get_attr( &trans, &tinfo, id, query, 0, ATTR_TRANS ) > 0 )
	{	exec_trans_event( trans, &tinfo );
		odm_free_list( trans, &tinfo );
	}

	return( SUCCESS );

} /* end of set_state */
	
/*---------------------------- ck_if_attrs       ------------------------------
*
* NAME: ck_if_attrs
*
* FUNCTION:
*		checks for missing or orphin attrs which are associated with ATTR_IF
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
*			mac					= ptr to nim_object
*			minfo					= ptr to listinfo struct for <mobj>
*		global:
*
* RETURNS: (int)
*		SUCCESS					= object definition is complete for <name>
*		FAILURE					= <name> missing something
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ck_if_attrs(	struct nim_object *mac,
					struct listinfo *minfo )

{	int i;
	struct nim_if nimif;
	int net_type;
	char msg[MAX_TMP];
	char if_name[MAX_TMP];

	VERBOSE2("   checking interface info for %s;\n",mac->name,NULL,NULL,NULL)

	/* for each of this machine's ATTR_SUBCLASS_IF attrs... */
	for (i=0; i < mac->attrs_info->num; i++)
	{
		if ( ! attr_in_list( ATTR_SUBCLASS_IF, mac->attrs[i].pdattr->subclass ) )
			continue;

		/* which ATTR_SUBCLASS_IF attr? */
		switch (mac->attrs[i].pdattr->attr)
		{
		case ATTR_IF:
			/* convert the IF attr to a nim_if struct */
			if ( s2nim_if( mac->attrs[i].value, &nimif ) == FAILURE )
				return( FAILURE );

			/* convert network adapter name into a network type */
			if ( (net_type = a2net_type( nimif.adapter )) == -1 )
				return( FAILURE );

			/* check for attrs required for specific network types */
			sprintf( if_name, "%s%d", ATTR_IF_T, mac->attrs[i].seqno);
			switch ( net_type )
			{
				case ATTR_TOK:
					if ( find_attr(	mac, NULL, NULL, mac->attrs[i].seqno, 
											ATTR_RING_SPEED ) < 0 )
					{
						sprintf( msg, "%s%d", ATTR_RING_SPEED_T, mac->attrs[i].seqno);
						ERROR( ERR_INCOMPLETE_IF, if_name, msg, NULL )
					}
					else if ( find_attr(	mac, NULL, NULL, mac->attrs[i].seqno, 
												ATTR_CABLE_TYPE ) >= 0 )
						ERROR( ERR_NETWORK_TYPE, ATTR_CABLE_TYPE_T, if_name, NULL )
				break;

				case ATTR_ENT:
					if ( find_attr(	mac, NULL, NULL, mac->attrs[i].seqno,
											ATTR_CABLE_TYPE ) < 0 )
					{
						sprintf( msg, "%s%d", ATTR_CABLE_TYPE_T, mac->attrs[i].seqno);
						ERROR( ERR_INCOMPLETE_IF, if_name, msg, NULL )
					}
					else if ( find_attr(	mac, NULL, NULL, mac->attrs[i].seqno, 
												ATTR_RING_SPEED ) >= 0 )
						ERROR( ERR_NETWORK_TYPE, ATTR_RING_SPEED_T, if_name, NULL )
				break;

				case ATTR_FDDI:
					/* cable_type and ring_speed cannot be supplied */
					if ( find_attr(	mac, NULL, NULL, mac->attrs[i].seqno, 
											ATTR_CABLE_TYPE ) >= 0 )
						ERROR( ERR_NETWORK_TYPE, ATTR_CABLE_TYPE_T, if_name, NULL )
					else if ( find_attr(	mac, NULL, NULL, mac->attrs[i].seqno, 
												ATTR_RING_SPEED ) >= 0 )
						ERROR( ERR_NETWORK_TYPE, ATTR_RING_SPEED_T, if_name, NULL )
				break;

			} /* switch (network type) */
		break;

		case ATTR_RING_SPEED:
		case ATTR_CABLE_TYPE:
			/* must have a cooresponding ATTR_IF */
			if ( find_attr( mac, NULL, NULL, mac->attrs[i].seqno, ATTR_IF ) < 0 )
			{
				sprintf( msg, "%s%d", mac->attrs[i].pdattr->name, 
							mac->attrs[i].seqno);
				sprintf( if_name, "%s%d", ATTR_IF_T, mac->attrs[i].seqno);
				ERROR( ERR_INCOMPLETE_IF, msg, if_name, NULL )
			}
		break;

		} /* switch */
	} /* for */

	return( SUCCESS );

} /* end of ck_if_attrs */
	
/*---------------------------- ck_mac_def        ------------------------------
*
* NAME: ck_mac_def
*
* FUNCTION:
*		determines if a machine's definition is complete or not
*		the machine may be specified by either its <id> or <name>
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
*			id						= id of nim_object for the machine
*			name					= name of nim_object for the machine
*		global:
*
* RETURNS: (int)
*		SUCCESS					= object definition is complete for <name>
*		FAILURE					= <name> missing something
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
ck_mac_def(	long id,
				char *name )

{	NIM_OBJECT( mac, minfo )
	int pif;
	struct nim_if nimif;
	char msg[MAX_TMP];

	VERBOSE2("   checking the object definition of %s;\n",name,NULL,NULL,NULL)

	/* lock-and-get the machine's object */
	if ( lag_object( id, name, &mac, &minfo ) == FAILURE )
		return( FAILURE );

	/* first, remove any informational attrs which might be lying around */
	rm_attr( mac->id, NULL, ATTR_MISSING, 0, NULL );
	rm_attr( mac->id, NULL, ATTR_INFO, 0, NULL );

	/* check for attrs associated with ATTR_IF */
	if ( ck_if_attrs( mac, &minfo ) == FAILURE )
		return( FAILURE );

	/* check the primary interface */
	if ( (pif = find_attr( mac, NULL, NULL, 1, ATTR_IF )) < 0 )
	{
		mk_attr( mac->id, NULL, PIF, 0, ATTR_MISSING, ATTR_MISSING_T );
		ERROR( ERR_OBJ_MISSING_ATTR, PIF, mac->name, NULL )
	}
	else if ( s2nim_if( mac->attrs[pif].value, &nimif ) == FAILURE )
	{
		/* check the state of the network the pif connects to */
		/* we're doing this here because the state of the network may */
		/*		change when one of the master's interfaces changes */
		if ( get_state( 0, nimif.network, ATTR_NSTATE ) != STATE_NREADY )
		{	
			/* network is not usable */
			sprintf( msg, MSG_msg( MSG_CHECK_NSTATE ), nimif.network );
			mk_attr( mac->id, NULL, msg, 0, ATTR_INFO, ATTR_INFO_T );
			ERROR( ERR_STATE, nimif.network, NULL, NULL )
		}
	}

	/* unlock-and-free object */
	uaf_object( mac, &minfo, FALSE );

	return( SUCCESS );

} /* end of ck_mac_def */
	
/*---------------------------- set_Mstate        ------------------------------
*
* NAME: set_Mstate
*
* FUNCTION:
*		attempts to determine what the Mstate of the specified machine should
*			be
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
*			id						= id of machine to check
*			name					= name of machine to check
*		global:
*
* RETURNS: (int)
*		SUCCESS					= Mstate has been updated
*		FAILURE					= unable to update the Mstate
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
set_Mstate(	long id,
				char *name )

{	int i;
	NIM_OBJECT( mac, minfo )
	struct nim_if nimif;
	int Mstate;
	int netboot_enabled=FALSE;
	int can_ping=FALSE;
	int rc;
	FILE *c_stdout=NULL;
	char	*Args[] =  { PING, "-c", "1", NULL, NULL	};

	/* get the machine's object */
	if ( lag_object( id, name, &mac, &minfo ) == FAILURE )
		return( FAILURE );

	VERBOSE2("   set_Mstate: name=%s\n",mac->name,NULL,NULL,NULL)

	/* find the primary interface and convert it into a nim_if struct */
	if ( (i = find_attr( mac, NULL, NULL, 1, ATTR_IF )) < 0 )
		ERROR( ERR_OBJ_MISSING_ATTR, mac->name, PIF, NULL )
	else if ( s2nim_if( mac->attrs[i].value, &nimif ) == FAILURE )
		return( FAILURE );

	/* get the current Mstate */
	Mstate = get_state( mac->id, NULL, ATTR_MSTATE );

	/* is there a "boot" resource allocated to this machine? */
	netboot_enabled = ( find_attr( mac, NULL, NULL, 0, ATTR_BOOT) >= 0 );

	/* can we PING the machine? */
	Args[3] = nimif.hostname;
	can_ping = ( (client_exec( &rc, &c_stdout, Args ) == SUCCESS) && (rc == 0) );

	/* figure out what the Mstate "probably" is based on the results of PING */
	/* 	and what NIM is currently doing with this machine */
	switch (Mstate)
	{
		case STATE_BOOTING:
			if ( ! netboot_enabled )
			{
				if ( can_ping )
					set_state( mac->id, NULL, ATTR_MSTATE, STATE_RUNNING );
				else
					set_state( mac->id, NULL, ATTR_MSTATE, STATE_SHUTDOWN );
			}
		break;

		case STATE_RUNNING:
			if ( ! can_ping )
				set_state( mac->id, NULL, ATTR_MSTATE, STATE_SHUTDOWN );
		break;

		default:
			if ( can_ping )
				set_state( mac->id, NULL, ATTR_MSTATE, STATE_RUNNING );
			else
				set_state( mac->id, NULL, ATTR_MSTATE, STATE_SHUTDOWN );
		break;
	}

	return( SUCCESS );

} /* end of set_Mstate */
	
/*---------------------------- set_cstate        ------------------------------
*
* NAME: set_cstate
*
* FUNCTION:
*		initializes a machine's Cstate
*		the machine may be specified by either its <id> or <name>
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
*			id						= id of nim_object for the machine
*			name					= name of nim_object for the machine
*		global:
*
* RETURNS: (int)
*		>0							= new CSTATE of the machine
*		0							= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
set_cstate(	long id,
				char *name )

{	int new_state;

	/* check the machine's definition */
	if ( ck_mac_def( id, name ) == SUCCESS )
		new_state = STATE_CREADY;
	else
		new_state = STATE_INCOMPLETE;

	VERBOSE2("   setting the Cstate of %s to %s\n",name,STATE_msg(new_state),
				NULL,NULL)

	set_state( id, name, ATTR_CSTATE, new_state );

	return( new_state );

} /* end of set_cstate */
	
/*---------------------------- Cresult           ------------------------------
*
* NAME: Cresult
*
* FUNCTION:
*		records the result of the current Cstate and transitions the object to
*			the next Cstate
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
*			name					= name of nim_object for the machine
*			result				= result of current Cstate
*		global:
*
* RETURNS: (int)
*		>0							= new CSTATE of the machine
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
Cresult(	char *name,
			int result )

{	long id;
	ODMQUERY
	struct nim_attr cstate;
	int new_state = STATE_CREADY;
	int i;
	int current_state;
	char tmp[MAX_TMP];

	VERBOSE3("      Cresult: name=%s; result=%d;\n",name,result,NULL,NULL)

	/* README: since this function is used to trasition objects between states,*/
	/*		it is VERY important that it continue from beginning to end, doing */
	/*		as much as it can.  This means that most errors will be ignored */

	/* attempt to lock the object */
	lock_object( 0, name );

	/* get it's id */
	if ( (id = get_id( name )) <= 0 )
		nim_error( 0, NULL, NULL, NULL );

	/* attempt to add the Cresult */
	ch_attr(	id, name, RESULT_msg(result), 0,
				ATTR_CRESULT, ATTR_msg(ATTR_CRESULT) );

	/* find the current Cstate */
	sprintf( query, "id=%d and pdattr=%d", id, ATTR_CSTATE );
	if ( odm_get_first( nim_attr_CLASS, query, &cstate ) != NULL )
	{
		current_state = (int) strtol( cstate.value, NULL, 0 );

		/* set the next Cstate */
		switch (current_state)
		{	
			case STATE_INCOMPLETE	:
			case STATE_CREADY			:
			case STATE_SCHEDULED		:	
					/*????? not expecting a Cresult at this point */
					/*???? how'd we get one ????*/
			break;

			case STATE_CUST			:
			case STATE_DIAG			:
			case STATE_DD_BOOT		:
			case STATE_BOS_INST_3	:
			case STATE_MAINT			:
					/* nothing to do here - next state is "ready" by default */
			break;

			case STATE_DKLS_INIT		:
					if ( result == RESULT_SUCCESS )
						new_state = STATE_DD_BOOT;
			break;

			case STATE_DTLS_INIT		:
					/* remember that we've initialized client */
					/* when this attribute is present, m_alloc_boot will not add */
					/*		NIM_MK_DATALESS variable to the client's .info file */
					if ( result == RESULT_SUCCESS ) {
						ch_attr( id, NULL, "yes", 0, ATTR_ROOT_INITIALIZED,
								ATTR_ROOT_INITIALIZED_T );
						new_state = STATE_DD_BOOT;
					}
			break;

			case STATE_BOS_INST		:
					/* BOS install has begun executing */
					if ( result == RESULT_SUCCESS )
						new_state = STATE_BOS_INST_1;
			break;

			case STATE_BOS_INST_1	:
					/* base installation done - customization has begun */
					if ( result == RESULT_SUCCESS )
						new_state = STATE_BOS_INST_2;
			break;

			case STATE_BOS_INST_2	:
					/* customization has finished - post install processing now */
					/* NOTE that it doesn't matter whether customization failed */
					/*		or not - we continue on; the policy on the customiztion */
					/*		script is that it will return failure if ANY command */
					/*		executed in the script fails */
					if ( result != RESULT_ABORT )
						new_state = STATE_BOS_INST_3;
			break;

		}
	}

	set_state( id, NULL, ATTR_CSTATE, new_state );

	/* make sure no ATTR_TRANS in the "ready" state */
	if ( new_state == STATE_CREADY )
		rm_attr( id, NULL, ATTR_TRANS, 0, NULL );

	return( new_state );

} /* end of Cresult */
	
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$ resource state manipulation by server            $$$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
	
/*---------------------------- reserve_server_res -----------------------------
*
* NAME: reserve_server_res 
*
* FUNCTION:
*		"reserves" all the resources which are served by the specified machine
*		this function is called in preparation for installing the server
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
*			obj					= nim_object struct for the machine
*		global:
*
* RETURNS: (int)
*		SUCCESS					= x
*		FAILURE					= x
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
reserve_server_res(	struct nim_object *obj )

{	int i;
	long id;

	VERBOSE("   reserve all resources served by %s\n",obj->name,
				NULL,NULL,NULL)

	/* for each resource this machine serves... */
	for (i=0; i < obj->attrs_info->num; i++)
		if ( obj->attrs[i].pdattr->attr == ATTR_SERVES )
		{
			/* get the resource's object id */
			if ( (id = get_id( obj->attrs[i].value )) <= 0 )
				return( FAILURE );

			/* change its Rstate to "unavailable" */
			set_state( id, NULL, ATTR_RSTATE, STATE_UNAVAILABLE );

			/* tell the user why */
			ch_attr( id, NULL, MSG_msg(MSG_SERVER_INST), 0,
						ATTR_INFO, ATTR_msg(ATTR_INFO_T) );

		}

	return( SUCCESS );

} /* end of reserve_server_res */
	
/*---------------------------- reset_server_res     ----------------------------
*
* NAME: reset_server_res
*
* FUNCTION:
*		resets states for resources which are served by the specified machine 
*		this function is used to reset states after a server has been installed
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
*			obj					= nim_object struct for the machine
*		global:
*
* RETURNS: (int)
*		SUCCESS					= states reset
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
reset_server_res(	struct nim_object *obj )

{	int i;
	long id;
	ODMQUERY
	struct nim_attr attr;
	int prev_state;

	VERBOSE("   reset Rstates for resource served by %s\n",obj->name,
				NULL,NULL,NULL)

	/* for each resource this machine serves... */
	for (i=0; i < obj->attrs_info->num; i++)
		if ( obj->attrs[i].pdattr->attr == ATTR_SERVES )
		{
			/* get the resource's object id */
			if ( (id = get_id( obj->attrs[i].value )) <= 0 )
				continue;

			/* get the previous state for this object */
			sprintf( query, "id=%d and pdattr=%d", id, ATTR_PREV_STATE );
			if ( odm_get_first( nim_attr_CLASS, query, &attr ) == NULL )
				prev_state = STATE_AVAILABLE;
			else
				prev_state = (int) strtol( attr.value, NULL, 0 );

			/* reset the set */
			set_state( id, NULL, ATTR_RSTATE, prev_state );

			/* remove the ATTR_INFO attr */
			rm_attr( id, NULL, ATTR_INFO, 0, NULL );
		}

	return( SUCCESS );

} /* end of reset_server_res */

