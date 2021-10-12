static char sccsid[] = "@(#)94	1.5  src/bos/diag/util/u5081/wga/uwga.c, dsau5081, bos411, 9428A410j 1/17/94 17:20:40";

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
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<stdio.h>
#include	<signal.h>

#include	"wgamisc.h"	/* Get header from wga TU directory */
#include	"exectu.h"	/* Get header from wga TU directory */
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
	fdebug=fopen("/tmp/uwga.debug","a+");
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
	int	tus[]={ RED_TU, GREEN_TU, BLUE_TU,
			BLACK_TU, WHITE_TU, WB_9X9_TU, WB_9X11_TU,
			COLOR_BAR_TU,SQUARE_BOX_50MM_TU, DISPLAY_AT_TU};
   	int 	error_num = 0;		/* error number from the test unit*/
	int	model;


	TUCB	wga_tu;

	(void) memset(&wga_tu,0,sizeof(wga_tu));

	wga_tu.header.loop = 1;
	wga_tu.header.tu = TU_OPEN;   /* First TU */
	error_num = exectu (name,&wga_tu);
		

	#if	DEBUG
	fprintf(fdebug,"name = %d, tu num = %d, ptr = %d\n",
		name,tus[tu],tu);
	fflush(fdebug);
	#endif

	wga_tu.header.loop = 1;
      wga_tu.header.tu = tus[tu];    /* requested tu */
      error_num = exectu (name,&wga_tu); 


	diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, TRUE,NULL);


	wga_tu.header.loop = 1;
	wga_tu.header.tu = TU_CLOSE;	/* TU CLOSE */
	error_num = exectu (name,&wga_tu);

	return(error_num);
}
