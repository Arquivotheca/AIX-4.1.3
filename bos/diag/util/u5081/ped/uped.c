static char sccsid[] = "@(#)91	1.3.1.5  src/bos/diag/util/u5081/ped/uped.c, dsau5081, bos411, 9428A410j 3/17/94 16:33:21";
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
	int	tus[]={ TU_OPEN, TU_31, TU_32, TU_33, TU_35, TU_34,
			TU_36, TU_37, TU_30, TU_CLOSE};
   	int 	error_num = 0;		/* error number from the test unit*/

	PED4_TU_TYPE	ped_tu;

	(void) memset(&ped_tu,0,sizeof(ped_tu));

        ped_tu.tu = tus[0];    
	ped_tu.loop = 1;
        error_num = exectu (name,&ped_tu);
	if (speed == 77)
	    ped_tu.tu = 77;
	else
	    ped_tu.tu = 60;

	ped_tu.loop = 1;
        error_num = exectu (name,&ped_tu);
        ped_tu.tu = tus[tu];    /* requested tu */

/*
	if(testingself)
	{
                error_num = exectu (name,&ped_tu);
		invec.sa_handler =  ret_handler;
		sigaction( SIGMSG, &invec, (struct sigaction *) NULL );
		while(!keytouch)
			pause();
		keytouch=0;
		ped_tu.tu = tus[9];
		error_num = exectu (name,&ped_tu);
	}
	else
*/
	{
		error_num = exectu (name,&ped_tu);
		diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, TRUE,NULL);
		ped_tu.loop = 1;
		ped_tu.tu = tus[9];
		error_num = exectu (name,&ped_tu);
	}

	return(error_num);
}

int	ret_handler(int sig)
{ 
	keytouch++;
}
