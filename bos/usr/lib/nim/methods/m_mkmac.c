static char	sccs_id[] = " @(#)00 1.21.1.1  src/bos/usr/lib/nim/methods/m_mkmac.c, cmdnim, bos411, 9430C411a  7/22/94  11:35:51";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_mkmac.c
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

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] = 
{
	{ ATTR_IF, 					ATTR_IF_T, 					TRUE, 	valid_pdattr_ass },
	{ ATTR_IPLROM_EMU,		ATTR_IPLROM_EMU_T,		FALSE, 	valid_pdattr_ass },
	{ ATTR_CPUID, 				ATTR_CPUID_T, 	 			FALSE, 	valid_pdattr_ass },
	{ ATTR_COMMENTS, 			ATTR_COMMENTS_T, 			FALSE, 	valid_pdattr_ass },
	{ ATTR_RING_SPEED, 		ATTR_RING_SPEED_T,		FALSE, 	valid_pdattr_ass },
	{ ATTR_CABLE_TYPE, 		ATTR_CABLE_TYPE_T,		FALSE, 	valid_pdattr_ass },
	{ ATTR_PLATFORM, 			ATTR_PLATFORM_T,			FALSE, 	valid_pdattr_ass },
	{ ATTR_FORCE, 				ATTR_FORCE_T,				FALSE, 	ch_pdattr_ass },
	{ ATTR_VERBOSE, 			ATTR_VERBOSE_T,			FALSE, 	ch_pdattr_ass },
	{ ATTR_MASTER_PORT,		ATTR_MASTER_PORT_T,		FALSE,	NULL },
	{ 0, 							NULL, 						FALSE, 	NULL }
};

char	*name = NULL;
char	*type = NULL;
NIM_PDATTR( pdtype, pinfo )

/*---------------------------- ck_req_attrs           -------------------------
*
* NAME: ck_req_attrs
*
* FUNCTION:
*		makes sure the required attributes were specified
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
ck_req_attrs ()

{
	int	is_master=0, has_master_port=-1, i, j;
	int platform;
	NIM_PDATTR( pd_platform, platinfo )

	/* check for required attrs */
	i = -1;
	while ( valid_attrs[ ++i ].name != NULL )
	{
		if ( valid_attrs[i].required )
		{
			for (j = 0; j < attr_ass.num; j++)
				if ( attr_ass.list[j]->pdattr == valid_attrs[i].pdattr )
					break;

			/* required attr present? */
			if ( j == attr_ass.num )
				nim_error( ERR_MISSING_ATTR, valid_attrs[i].name, NULL, NULL );
		}
	}

	/* 
	 * If this is a master definition, we need to know the master_port
	 * else if master_port is specified its an error !
	 */
 	has_master_port = (find_attr_ass( &attr_ass, ATTR_MASTER_PORT ) != -1);
	is_master = ( strcmp(type, ATTR_MASTER_T) == 0 );

	if (is_master) {
		if (!has_master_port)
			nim_error( ERR_MISSING_ATTR, ATTR_MASTER_PORT_T, NULL, NULL );
	}
	else
		if (has_master_port)
			nim_error( ERR_NO_CH_ATTR, ATTR_MASTER_PORT_T, NULL, NULL );

	/* use rs6k if ATTR_PLATFORM not specified */
	if ( (platform = find_attr_ass( &attr_ass, ATTR_PLATFORM )) < 0 )
	{
		if ( add_attr_ass(	&attr_ass, ATTR_PLATFORM, ATTR_PLATFORM_T, 
									ATTR_RS6K_T, 0 ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );
		platform = find_attr_ass( &attr_ass, ATTR_PLATFORM );
	}

	/* verify that specified platform is valid */
	if ( get_pdattr(	&pd_platform, &platinfo, ATTR_CLASS_MACHINES,
							ATTR_SUBCLASS_PLATFORM, 0, 0,
							attr_ass.list[platform]->value ) <= 0 )
		nim_error( ERR_PLATFORM, attr_ass.list[platform]->value, NULL, NULL );

	/* make sure specified platform type supports specified configuration type */
	if ( ! attr_in_list( pdtype->attr, pd_platform->type ) )
		nim_error(	ERR_TYPE_PLATFORM, pdtype->name, 
						attr_ass.list[platform]->value, NULL );

	return( SUCCESS );

} /* end of ck_req_attrs */

/*---------------------------- mkmac             ------------------------------
*
* NAME: mkmac
*
* FUNCTION:
*		creates a new machine object
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
*			type_num
*
* RETURNS: (int)
*		SUCCESS					= object created
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mkmac()

{
	int	rc = SUCCESS;
	long	id;
	int	i;

	/* check for required attrs */
	ck_req_attrs();

	/* create the object */
	if ( (id = mk_object( name, type, 0 )) <= 0 )
		nim_error( 0, NULL, NULL, NULL );

	/* add the nim_attrs */
	for (i = 0; i < attr_ass.num; i++) {
		if ( attr_ass.list[i]->pdattr == ATTR_IF )
			if ( (rc=verify_if(id, attr_ass.list[i]->seqno,
					attr_ass.list[i]->value, NULL)) == FAILURE )
				break;	
					
		if (!mk_attr( id, NULL, attr_ass.list[i]->value,
		    attr_ass.list[i]->seqno, attr_ass.list[i]->pdattr,
		    attr_ass.list[i]->name ) ) 
		{	/* error occured - object cannot be defined; remove it */
			rc = FAILURE;
			protect_errstr = TRUE;
			break;
		}
	}

	/* any errors during definition? */
	if ( (rc == FAILURE) || (set_cstate( id, NULL ) == STATE_INCOMPLETE) ) 
	{	/* yup - remove everything related to this object */
		rm_object( id, NULL );
		nim_error( 0, NULL, NULL, NULL );
	}

	/* set the Mstate */
	set_Mstate( id, NULL );

	/* set the CSTATE */
	set_state( id, NULL, ATTR_CSTATE, STATE_CREADY );

	return( SUCCESS );

} /* end of mkmac */

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

{
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;
	int	syntax_err = FALSE;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:t:q")) != -1 ) {
		switch (c)
		{
			case 'a': /* attribute assignment */
				if (!parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
					nim_error( 0, NULL, NULL, NULL );
			break;

			case 't': /* object type */
				if ( type != NULL )
					syntax_err = TRUE;
				else if ( get_pdattr(	&pdtype, &pinfo, ATTR_CLASS_MACHINES,
			    								ATTR_SUBCLASS_TYPE, 0, 0, optarg ) <= 0 )
					nim_error(	ERR_BAD_TYPE_FOR, optarg, 
				    				ATTR_CLASS_MACHINES_T, NULL );
				else if (!strcmp( optarg, ATTR_msg(ATTR_MASTER) ) )
					nim_error(	ERR_ONLY_ONE, ATTR_msg(ATTR_MASTER),
				    				ATTR_msg(ATTR_MASTER), NULL );
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
				nim_error( ERR_BAD_OPT, optopt, MSG_msg(MSG_M_MKMAC_SYNTAX), NULL );
			break;
		}

		if ( syntax_err )
			nim_error( ERR_SYNTAX, MSG_msg(MSG_M_MKMAC_SYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind == argc )
		nim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL );
	else if ( optind < (argc - 1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_M_MKMAC_SYNTAX), NULL, NULL );
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

	if ( !get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) )
		nim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	VERBOSE("m_mkmac: define the %s machine\n",name,NULL,NULL,NULL)

	/* create a new machine object */
	mkmac();

	exit( 0 );

}


