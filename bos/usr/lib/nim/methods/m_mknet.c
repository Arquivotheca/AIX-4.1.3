static char	sccs_id[] = " @(#)87 1.12.1.2  src/bos/usr/lib/nim/methods/m_mknet.c, cmdnim, bos411, 9434B411a  8/24/94  15:07:28";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_mknet.c
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

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{ATTR_NET_ADDR,			ATTR_NET_ADDR_T,			TRUE,		valid_pdattr_ass},
 	{ATTR_SNM,					ATTR_SNM_T,					TRUE,		valid_pdattr_ass},
 	{ATTR_ROUTING,				ATTR_ROUTING_T,			FALSE,	valid_pdattr_ass},
 	{ATTR_OTHER_NET_TYPE,	ATTR_OTHER_NET_TYPE_T,	FALSE,	valid_pdattr_ass},
 	{ATTR_COMMENTS,			ATTR_COMMENTS_T,			FALSE,	valid_pdattr_ass},
 	{ATTR_FORCE,				ATTR_FORCE_T,				FALSE,	ch_pdattr_ass},
 	{ATTR_VERBOSE,				ATTR_VERBOSE_T,			FALSE,	ch_pdattr_ass},
	{0,							NULL, 						FALSE,	NULL}
};

char *name=NULL;
char *type=NULL;
long id=0;

/*---------------------------- definition_ck           -------------------------
*
* NAME: definition_ck
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
definition_ck ()

{	int i,j;
	char	*new_addr=NULL;
	long	l_addr; 
	int	addr_ndx; 
	char	*new_netaddr=NULL;
	long	l_netaddr; 
	char	*new_snm=NULL;
	long	l_snm; 

	NIM_ATTR( attr, attr_i );

	/* check for required attrs */
	i=-1;
	while ( valid_attrs[ ++i ].pdattr > 0 )
		if ( 	(valid_attrs[i].required) && 
				(find_attr_ass( &attr_ass, valid_attrs[i].pdattr ) < 0) )
			nim_error( ERR_MISSING_ATTR, valid_attrs[i].name, NULL, NULL );

	/* get the new address and netmask. check for net address collisions */
	for (i=0; i < attr_ass.num; i++)
		if ( attr_ass.list[i]->pdattr == ATTR_NET_ADDR ) {
			new_addr = attr_ass.list[i]->value;
			addr_ndx = i; 
		}	
		else if ( attr_ass.list[i]->pdattr == ATTR_SNM ) 
			new_snm = attr_ass.list[i]->value;
		

	l_addr = inet_addr(new_addr); 
	l_snm  = inet_addr(new_snm); 
	l_netaddr = l_addr&l_snm; 
	new_netaddr = inet_ntoa(l_netaddr);

	if ( get_attr( &attr, &attr_i, 0, new_netaddr,  0, ATTR_NET_ADDR ) > 0 )
				nim_error(	ERR_ONLY_ONE, MSG_msg(MSG_NET_DEFINITION), 
								new_netaddr, NULL );
	/* 
	 * no duplicate found, replace the user inputted network address with the 
	 * "netmasked" one. 
	 */ 
	/* NOTE that we must malloc space to cache the new network address because */
	/*		inet_ntoa uses a static variable to return it's info, so next time */
	/*		inet_ntoa gets called, it will write over what we've already got if */
	/*		we don't copy the info somewhere else */
	attr_ass.list[addr_ndx]->value = nim_malloc( strlen(new_netaddr) + 1 );
	strcpy( attr_ass.list[addr_ndx]->value, new_netaddr ); 

	return( SUCCESS );

} /* end of definition_ck */
	
/*---------------------------- un_mknet          ------------------------------
*
* NAME: un_mknet
*
* FUNCTION:
*		undoes anything that mknet has done anytime FAILURE is encountered
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
*			errno			= error code
*			str1			= str1 of err message
*			str2			= str2 of err message
*			str3			= str3 of err message
*		global:
*			id				= id of newly created object
*
* RETURNS: (void)
*		doesn't
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

void
un_mknet(	int errno,
				char *str1,
				char *str2,
				char *str3 )

{
   disable_err_sig();

   /* errno given? */
   if ( errno > 0 )
      errstr( errno, str1, str2, str3 );

   /* protect the current errstr */
   protect_errstr = TRUE;

	/* remove the new object */
	if ( id > 0 )
		rm_object( id, NULL );

	/* bye-bye */
	nim_error( 0, NULL, NULL, NULL );

} /* end of un_mknet */

/*---------------------------- mknet             ------------------------------
*
* NAME: mknet
*
* FUNCTION:
*		creates a new network object
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
*		SUCCESS					= object created
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mknet()

{	int rc=SUCCESS;
	int i;
	char tmp[MAX_TMP];
	struct routing_ass rass;

	/* check for completeness of object definition */
	definition_ck();

	/* create the object */
	if ( (id = mk_object( name, type, 0 )) <= 0 )
		nim_error( 0, NULL, NULL, NULL );

	/* initialize undo in case signal received */
	undo_on_interrupt = un_mknet;

	/* add the nim_attrs */
	for (i=0; i < attr_ass.num; i++)
	{	if ( attr_ass.list[i]->pdattr == ATTR_ROUTING )
		{	if ( (rc = mk_net_route(	id, name, attr_ass.list[i]->value, 
												attr_ass.list[i]->seqno, 
												&backups )) == FAILURE )
				un_mknet( 0, NULL, NULL, NULL );
		}
		else if ( (rc = mk_attr( id, NULL, attr_ass.list[i]->value,
										attr_ass.list[i]->seqno, attr_ass.list[i]->pdattr,
										attr_ass.list[i]->name )) == FAILURE )
				un_mknet( 0, NULL, NULL, NULL );
	}

	/* success! - clear all undo operations */
	undo_on_interrupt = NULL;
	clear_backups( &backups );

	VERBOSE("   setting the network's Nstate\n",NULL,NULL,NULL,NULL)

	/* set the Nstate */
	set_nstate( name );

	return( SUCCESS );

} /* end of mknet */

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
	while ( (c = getopt(argc, argv, "a:t:q")) != -1 )
	{	switch (c)
		{	
			case 'a': /* attribute assignment */
				if (! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
			break;

			case 't': /* object type */
				if ( type != NULL )
					syntax_err = TRUE;
				else if ( get_pdattr(	NULL, NULL, ATTR_CLASS_NETWORKS,
												ATTR_SUBCLASS_TYPE, 0, 0, optarg ) <= 0 )
					nim_error(	ERR_BAD_TYPE_FOR, optarg, 
									ATTR_CLASS_NETWORKS_T, NULL );
				else
					type = optarg;
			break;

			case 'q': /* display valid_attrs */
				mstr_what( valid_attrs, NULL );
				exit( 0 );
			break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

			case '?': /* unknown option */
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_M_MKNET_SYNTAX), NULL );
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_M_MKNET_SYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_M_MKNET_SYNTAX), NULL, NULL );
	else if ( type == NULL )
		nim_error( ERR_MISSING_OPT, 't', NULL, NULL );

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

	VERBOSE("m_mknet: define the %s network\n",name,NULL,NULL,NULL)

	/* create a new network object */
	mknet();

	exit( 0 );

}
