static char sccsid[] = "@(#)87  1.9  src/bos/diag/da/fd/fda_main.c, dafd, bos411, 9428A410j 12/17/92 10:58:42";
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: MALLOC
 *		clean_up
 *		main
 *		tu_test
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* fda_main.c */

/************************************************************************/
/*                                                                      */
/*      Diagnostic Application for Diskette Drive Adapter               */
/*      This test can run in all modes and configurations               */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <locale.h>
#include <fcntl.h>
#include <nl_types.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/fd.h>

#include "diag/tm_input.h"      /* faking the ODM                       */
#include "diag/tmdefs.h"        /* tm_input.xxxxx CONSOLE_FALSE etc     */
#include "diag/diag_exit.h"
#include "diag/diago.h"

#include "../../tu/fd/fdatu.h"
#include "fda_rc.h"             /* return codes form the test units     */
#include "fdatuname.h"          /* define TU names                      */
#include "fd_set.h"
#include "dfd_msg.h"

#define MALLOC(x)   (( x*)malloc(sizeof(x)))

struct diskette_tucb_t *tucb_ptr;
struct tm_input da_input;
nl_catd catd;
long dev_fdes;
struct fd_drive *fd;

extern void disp_fda_menu();
extern void chk_asl_return();
extern int getdamode();
extern nl_catd diag_catopen();
extern void DFDA_LOOP_TEST_message();
void tu_test();
void clean_up();

void main(argc,argv,envp)
int argc;
char **argv;
char ** envp;
{
        int  rc;
        long   mode;


        /* ......................................... */
        /*               Initialization              */
        /* ......................................... */

	setlocale(LC_ALL,"");
        DA_SETRC_STATUS(DA_STATUS_GOOD);
        DA_SETRC_ERROR(DA_ERROR_NONE);
        DA_SETRC_TESTS(DA_TEST_FULL);
        DA_SETRC_MORE(DA_MORE_NOCONT);

        if(init_dgodm() == AIX_ERROR )
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }
        rc = getdainput(&da_input);
        if ( rc != NO_ERROR )
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }
        if (da_input.console == CONSOLE_TRUE)
        {
                chk_asl_return(diag_asl_init(ASL_INIT_DEFAULT));
                if((catd = diag_catopen(MF_DFD , 0)) == CATD_ERR)
                {
                        DA_SETRC_ERROR(DA_ERROR_OPEN);
                        clean_up();
                }
        }
        mode = getdamode(da_input);
        if(mode == INVALID_TM_INPUT)
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();

        }

        /*  ---------------------------------------------------- */
        /* Display testing message if a console exists           */
        /*  ---------------------------------------------------- */

        if (da_input.console == CONSOLE_TRUE)
        {
                if (da_input.loopmode == LOOPMODE_NOTLM)
                        disp_fda_menu();
                else
                        DFDA_LOOP_TEST_message();
                sleep(2);
        }
        if (da_input.loopmode == LOOPMODE_EXITLM)
                clean_up();

        /*  ---------------------------------------------------- */
        /* Need to allow opening the device driver               */
        /* The system will always have an internal 3.5 drive     */
        /*  ---------------------------------------------------- */

        dev_fdes = open("/dev/rfd0", O_RDWR | O_NDELAY);

        if (dev_fdes == AIX_ERROR)
        {
                DA_SETRC_ERROR(DA_ERROR_OPEN);
                clean_up();
        }

        tu_test(DISABLE_RETRY);
        tu_test(ADAPTER_TEST);
        tu_test(ENABLE_RETRY);

        clean_up();
}


/*
 * NAME: void tu_test(tstnum)
 *
 * FUNCTION: Call and run the test passed (tstnum). If an error occures
 *           then assign a fru bucket.
 *
 * EXECUTION ENVIRONMENT: called from main();
 *
 *
 * RETURNS: NONE
*/

/************************************************************************/
/*                                                                      */
/* tu_test will execute the test unit if possible and report error      */
/* messages with the FRU number and the percent confidence of the FRU.  */
/*                                                                      */
/************************************************************************/

void tu_test(tstnum)
int tstnum ;
{
        int rc;
        tucb_ptr =MALLOC( struct diskette_tucb_t);
        tucb_ptr->tucb.tu = tstnum;
        tucb_ptr->tucb.loop = 1;
        tucb_ptr->tucb.mfg = tucb_ptr->tucb.r1 = tucb_ptr->tucb.r2 = 0;
        /*  Run the test */
        rc = exectu(dev_fdes, tucb_ptr );        /* Execute the test unit */
        /* .................................... */
        /* If the returned value is not 0 then  */
        /* assign a fru                         */
        /* .................................... */
        if (rc != 0 )
                assign_fda_frub(rc, tucb_ptr->tucb.tu);
}


/*
 * NAME: void clean_up()
 *
 * FUNCTION: Close any open files prior to exiting to the diagnostic controller
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: NONE
*/

void clean_up()
{
        term_dgodm();
        if(catd != CATD_ERR)
                catclose(catd);
        if(dev_fdes > 0 )
                close(dev_fdes);
        if (da_input.console == CONSOLE_TRUE)
                diag_asl_quit();
        DA_EXIT();
}

