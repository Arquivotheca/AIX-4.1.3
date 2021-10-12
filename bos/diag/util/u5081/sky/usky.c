static char sccsid[] = "@(#)45	1.3  src/bos/diag/util/u5081/sky/usky.c, dsau5081, bos411, 9428A410j 3/17/94 16:28:30";
/*
 *   COMPONENT_NAME: DSAU5081
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<stdio.h>
#include	<signal.h>

#include	"skytu.h"	/* Get header from sky TU directory */
#include	"exectu.h"	/* Get header from sky TU directory */
#include	"diag/da.h"
#include	"diag/diago.h"
#include	"diag/da_rc.h"
#include	"diag/diag_exit.h"

#if DEBUG
FILE	*fdebug;
#endif

/*
 * NAME: main()
 *
 * FUNCTION: Main driver for Display Power Graphic Adapter Diagnostic
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS:  
 */

int	main(argc,argv)
int	argc;
char	*argv[];
{

	int	rc=0;
	#if	DEBUG
	fdebug=fopen("/tmp/usky.debug","a+");
	#endif


	if( (rc = diag_asl_init("NO_TYPE_AHEAD")) != DIAG_ASL_OK)
		exit(-1);
	rc = tu_test(argv[1],atoi(argv[2])-1);

	(void)diag_asl_quit( NULL );
	exit(rc);

}  /* main end */
 
/*
 * NAME: tu_test()
 *
 * FUNCTION: Designed to execute the test units, requested by the user.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described
 *	in Diagnostics subsystem CAS.
 *
 * NOTE:
 *
 * RETURNS: NONE
 */
 
int	tu_test( char * name, int tu) 
{
	int	tus[]={ 0xe053,0xe054,0xe055,0xe051,0xe052,0xe006,0xe004,0xe030};
   	int 	error_num = 0;		/* error number from the test unit*/
	int	model;


	SKYTYPE	sky_tu;

	(void) memset(&sky_tu,0,sizeof(sky_tu));

	sky_tu.header.loop = 1;
	sky_tu.header.tu = TU_OPEN;   /* First TU */
	error_num = exectu (name,&sky_tu);
		

	#if	DEBUG
	fprintf(fdebug,"name = %d, tu num = %d, ptr = %d\n",
		name,tus[tu],tu);
	fflush(fdebug);
	#endif

	sky_tu.header.loop = 1;
        sky_tu.header.tu = tus[tu];    /* requested tu */
        error_num = exectu (name,&sky_tu); 


	diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, TRUE,NULL);


	sky_tu.header.loop = 1;
	sky_tu.header.tu = TU_CLOSE;	/* TU CLOSE */
	error_num = exectu (name,&sky_tu);

	return(error_num);
}
