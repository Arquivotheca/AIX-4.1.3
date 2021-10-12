static char sccsid[] = "@(#)60	1.3  src/bos/diag/util/u5081/lega2/ulega2.c, dsau5081, bos411, 9428A410j 3/18/94 15:23:42";
/*
 * COMPONENT : DSAU5081
 *
 *
 * ORIGIN: 27
 *	
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1992, 1994
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a)
*/

#include	<stdio.h>
#include	<memory.h>
#include	<signal.h>
#include	<fcntl.h>

#include	"diag/diago.h"
#include	"diag/da_rc.h"
#include	"diag/diag_exit.h"
#include	"tu_type.h"

int	keytouch =0;

/* interrupt handler structure     */
struct  sigaction       invec;
int	ret_handler(int);



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

	if ((rc = diag_asl_init(NULL)) != DIAG_ASL_OK)
		return(-1);
	rc = tu_test(argv[1],atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));

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
 
int	tu_test( char * name, int tu, int testingself, int speed) 
{
	int	tus[]={ TU_OPEN, TU_23, TU_24, TU_25, TU_27, TU_26,
			TU_28, TU_29, TU_22, TU_CLOSE};
   	int 	error_num = 0;		/* error number from the test unit*/

	LEGA_TU_TYPE	lega_tu;

	(void) memset(&lega_tu,0,sizeof(lega_tu));

        lega_tu.tu = tus[0];    
	lega_tu.loop = 1;
        error_num = exectu (name,&lega_tu);
	if (speed == 77)
	    lega_tu.tu = 77;
	else
	    lega_tu.tu = 60;

	lega_tu.loop = 1;
        error_num = exectu (name,&lega_tu);
        lega_tu.tu = tus[tu];    /* requested tu */

/*
	if(testingself)
	{
                error_num = exectu (name,&lega_tu);
		invec.sa_handler =  ret_handler;
		sigaction( SIGMSG, &invec, (struct sigaction *) NULL );
		while(!keytouch)
			pause();
		keytouch=0;
		lega_tu.tu = tus[9];
		error_num = exectu (name,&lega_tu);
	}
	else
*/
	{
		lega_tu.loop = 1;
		error_num = exectu (name,&lega_tu);
		diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, TRUE,NULL);

		lega_tu.loop = 1;
		lega_tu.tu = tus[9];
		error_num = exectu (name,&lega_tu);
	}

	return(error_num);
}

int	ret_handler(int sig)
{ 
	keytouch++;
}
