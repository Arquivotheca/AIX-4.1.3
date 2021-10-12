static char sccsid[] = "@(#)70	1.17  src/bos/diag/da/iop/iop_main.c, daiop, bos411, 9428A410j 4/21/94 12:17:31";
/*
 *   COMPONENT_NAME: DAIOP
 *
 *   FUNCTIONS: MALLOC
 *		clean_up
 *		main
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <locale.h>
#include <fcntl.h>
#include <nl_types.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>


#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"
#include "iop.h"                /* generic defines                */
#include "ioptuname.h"          /* test unit names defines        */
#include "diop_msg.h"           /* message pointer                */
#include "tu_type.h"


#define MALLOC(x)    ((x *) malloc(sizeof(x)))



extern int getdainput();        /* lib function                   */
extern long getdamode();        /* found in "get_mode.c"          */
extern void chk_asl_return();
extern DIOP_LOOP_TEST_message();
extern nl_catd diag_catopen();
void clean_up();


struct tucb_t header;

long  dev_fdes;                 /* dev file pointer               */
struct tm_input da_input;       /* data passed by diag controller */
nl_catd catd;                   /* poiter to catalog file         */
long tmode;                     /* test mode from getdamode()     */
int rc;

                                /* Global variables               */
int time_reset = FALSE;
int battery_failed = FALSE;

/* ========================================================================= */
/*                         B E G I N    M A I N                              */
/* ========================================================================= */

void main(argc,argv,envp)
int argc;
char **argv;
char **envp;

{
	/* .................................... */
	/*              Initialize              */
	/* .................................... */

	setlocale(LC_ALL,"");
        DA_SETRC_TESTS(DA_TEST_FULL);
        DA_SETRC_STATUS(DA_STATUS_GOOD);
        DA_SETRC_ERROR(DA_ERROR_NONE);
        DA_SETRC_MORE(DA_MORE_NOCONT);

        /* .....................................*/
        /* initiate the diagnostic function     */
        /* object data manager                  */
        /* .....................................*/


        if(init_dgodm() == -1)
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

        /* ..................................... */
        /* get the diagnostic data passed by the */
        /* diagnostic controller                 */
        /* ..................................... */

        if(getdainput(&da_input) != 0)
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }


        /* .....................................*/
        /* initiate the diagnostic ASL          */
        /* .....................................*/

        if (da_input.console == CONSOLE_TRUE)
        {
                chk_asl_return(diag_asl_init(ASL_INIT_DEFAULT));

                /* .....................................*/
                /* Open the catalog file for the        */
                /* message text                         */
                /* .....................................*/

                catd = diag_catopen(MF_DIOP,0);
        }

        /* .....................................*/
        /* Get the mode (ALL,LOOP,NO_CONSOLE)   */
        /* from the da_input structure passed   */
        /* by the diagnostic controller         */
        /* .....................................*/


        tmode = getdamode(da_input);

        if(tmode == AIX_ERROR)
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }
        /* .............................................................. */
        /*     Testing is not done on exiting loop mode (D_EXITLM)        */
        /* .............................................................. */


        if (da_input.loopmode == LOOPMODE_EXITLM)
        {
		if (da_input.console == CONSOLE_TRUE) 
		{
			DIOP_LOOP_TEST_message();
			sleep(2);
		}
                rc = getdavar(da_input.dname,"TIME_FLAG",
                              DIAG_INT,&time_reset);
                if (rc == AIX_ERROR)
                        time_reset = FALSE;
                clean_up();
        }
        /* ........................................*/
        /* Begin doing the series of test required */
        /* for the type of mode (tmode)            */
        /* ........................................*/

        if(da_input.dmode != DMODE_ELA )
		{
               do_iop_tests();
		}

	if (da_input.console == CONSOLE_TRUE)
		chk_asl_return(diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,NULL,NULL));

        clean_up();
}

/* ========================================================================= */
/*                             E N D    M A I N                              */
/* ========================================================================= */

/* ..................................... */
/* clean_up() close all open files and   */
/* exit back to the dianostic controller */
/* ..................................... */


void clean_up()
{
        int rc;


        if ( time_reset == TRUE && (da_input.loopmode == LOOPMODE_ENTERLM
              || da_input.loopmode == LOOPMODE_INLM) )
                rc = putdavar(da_input.dname,"TIME_FLAG",
                              DIAG_INT,&time_reset);

        /* .............................. */
        /* If the time was reset then     */
        /* remind the user to set the     */
        /* day date to the correct values */
        /* .............................. */

        if(time_reset == TRUE && da_input.console == CONSOLE_TRUE
           && (da_input.loopmode ==  LOOPMODE_NOTLM
               || da_input.loopmode == LOOPMODE_EXITLM) )
                TOD_SET_message();
        term_dgodm();
        if(catd != CATD_ERR)
                catclose(catd);
        if(dev_fdes > 0 )
                close(dev_fdes); 

		if (da_input.console == CONSOLE_TRUE) 
			diag_asl_quit(NULL); 

		DA_EXIT();
}


run_new_tu(int fdes)
{
	TUTYPE tucb;
	int i;
	int rc;

	int tus[]={START_TEST,SUSP_TEST,INTR_TEST,OSC_TEST,PERIODIC,TODNVRAM,
				 SAVE_ENABLE,LEAP_YEAR, /*HOUR12,*/CLOCK_STOP,LOW_BATTERY,
				 TOD_IRQ_STAT,PF_IRQ,NULL};

	tucb.header.mfg = 0;

	i=0;
	while(tus[i] != NULL)
	{

		tucb.header.loop = 1;
		tucb.header.tu = tus[i];
		rc = exectu(fdes,&tucb);
		if(rc != 0)
		{
			assign_iop_frub(TIME_OF_DAY_TEST_FAILURE,TIME_OF_DAY_TEST1);
		}
		i++;
	}
	return(0);
	
}
