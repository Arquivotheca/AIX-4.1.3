static char	sccs_id[] = " @(#)82 1.2  src/bos/usr/lib/nim/methods/m_rm_lock.c, cmdnim, bos411, 9428A410j  2/11/94  14:55:53";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/lib/nim/methods/m_rm_lock.c
 *		
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cmdnim_mstr.h"

ATTR_ASS_LIST attr_ass;

main( int argc, char *argv[] )

{ 

	int loop; 

	mstr_init( argv[0], ERR_POLICY_METHOD, NULL );
	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	for ( loop=1; loop!=argc; loop++ ) {
		if (rm_attr( 0, argv[loop], ATTR_LOCKED, 0, NULL ) != SUCCESS)
			nene(0,NULL, NULL, NULL); 
	}
} 
