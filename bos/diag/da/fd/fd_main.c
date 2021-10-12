static char sccsid[] = "@(#)85	1.20  src/bos/diag/da/fd/fd_main.c, dafd, bos411, 9428A410j 4/12/94 04:32:53";
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: clean_up
 *		do_fd_ela
 *		drive_type
 *		int_handler
 *		main
 *		
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */


/************************************************************************/
/*                                                                      */
/*      Diagnostic Application for Diskette Drives                      */
/*      All Test Units have to run in Full Test Mode                    */
/*                                                                      */
/************************************************************************/

/* #define DFD_DEBUG */

#include <stdio.h>
#include <locale.h>
#include <signal.h>
#include <sys/errids.h>
#include <sys/cfgodm.h>

#include "diag/tm_input.h"      /* faking the ODM                       */
#include "diag/tmdefs.h"        /* defs of the tm_input.xxx             */
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"

#include "fd_set.h"
#include "fda_rc.h"             /* define tu return codes               */
#include "fdatuname.h"          /* define TU test names                 */
#include "dfd_msg.h"

extern int getdainput();        /*                                      */
extern long getdamode();        /* found in "get_mode.c"                */
extern void chk_asl_return();
struct tm_input da_input;
struct fd_drive fd_data, *fd;   /* the floppy drive data                */

extern int dev_fdes;            /* device driver file pointer           */
nl_catd catd;                   /* pointer to catalog file              */
extern nl_catd diag_catopen();

int tmode;
                               /* ....................................... */
int diskette_inserted;         /* values saved 'putdavar()' for loop mode */
int test_type;                 /* ....................................... */
int drive_4MB;

int dev_state;                 /* state of device prior to configuration  */

void clean_up();
void do_fd_ela();
void fd_exit_message();
void DFD_LOOP_TEST_message();

void main(argc,argv,envp)
int argc;
char ** argv;
char ** envp;
{
        int rc;                         /* Return Code                     */
        int dt;
        int i;                          /* generic counter                 */
        char devname[80];
        void    int_handler(int);
        struct  sigaction       act;

        /* ......................................... */
        /*               Initialization              */
        /* ......................................... */

        setlocale(LC_ALL,"");
        DA_SETRC_STATUS(DA_STATUS_GOOD);
        DA_SETRC_ERROR(DA_ERROR_NONE);
        DA_SETRC_TESTS(DA_TEST_FULL);
        DA_SETRC_MORE(DA_MORE_NOCONT);

        diskette_inserted = FALSE;
        test_type = ADAPTER_TEST;
        dev_state = AIX_ERROR;
        fd = &fd_data;
        if(init_dgodm() == AIX_ERROR )
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }
        if(getdainput(&da_input) == AIX_ERROR )
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

        if (da_input.exenv == EXENV_SYSX
        	&& da_input.loopmode == LOOPMODE_EXITLM)
        {
		clrdainput();
		clean_up();
        }

        /* set up interrupt handler    */

        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        /* ......................................... */
        /* Ininate the DA asl                        */
        /* ......................................... */

        if(da_input.console == CONSOLE_TRUE)
        {
				if(da_input.loopmode == LOOPMODE_INLM)
                	chk_asl_return(diag_asl_init(ASL_INIT_DEFAULT)) ;
				else
	                chk_asl_return(diag_asl_init("NO_TYPE_AHEAD")) ;

                catd = diag_catopen(MF_DFD,0);
        }

        /* ......................................... */
        /* Configure the device.                     */
        /* ......................................... */

        if (da_input.dmode != DMODE_ELA)
        {
                dev_state = configure_device(da_input.dname);
                if (dev_state == AIX_ERROR)
                {
                        if (da_input.loopmode == LOOPMODE_NOTLM
                            && da_input.dmode == DMODE_PD)
                                do_fd_ela();
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        clean_up();
                }
        }

        /* ......................................... */
        /* If the da_input.dname is fd0 then the     */
        /* drive is the default 3.5" HI density      */
        /* diskette drive                            */
        /* ......................................... */

        /* ......................................... */
        /* If the da_input.dname is fd1 then the     */
        /* drive is the 5.25" HI density             */
        /* diskette drive                            */
        /* ......................................... */

        strcpy(devname,da_input.dname);

        fd->location = DRIVE_0 ;        /* default */

        for( i = 0 ; i < ( strlen(devname)) ; ++i)
        {
                if(devname[i] == '1')
                        fd->location = DRIVE_1;
        }

        if(fd->location == DRIVE_1)
        {
                fd->location = DRIVE_1;
                fd->size = FIVE25;
                fd->capacity = ESIZE_12M;
                fd->disk_density = HIGH_DENSITY;
                fd->drive_density = HIGH_DENSITY;
        }
        else
        {
                fd->location = DRIVE_0 ;        /* default */
                fd->size = THREE5;
                drive_4MB = drive_type();
                if (drive_4MB == AIX_ERROR) {
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        clean_up();
                }

                if (drive_4MB && da_input.exenv != EXENV_SYSX)
                /* in sysx we will use the 2MB diskette */
                {
                        fd->capacity = ESIZE_288M;
                        fd->disk_density = HIGH_DENSITY_4MB;
                        fd->drive_density = HIGH_DENSITY_4MB;
                 }
                 else
                 {
                        fd->capacity = ESIZE_144M;
                        fd->disk_density = HIGH_DENSITY;
                        fd->drive_density = HIGH_DENSITY;
                 }
        }

        /* ......................................... */
        /* get the DA mode , ALL , SYSTEM, LOOP etc  */
        /* ......................................... */

        tmode = getdamode(da_input);

        if(tmode == -1)
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

        /* ......................................... */
        /* Testing is not done on exiting loop mode  */
        /* ......................................... */

        if (da_input.loopmode == LOOPMODE_EXITLM)
        {
                if (da_input.console == CONSOLE_TRUE)
                {
                        rc = getdavar(da_input.dname,"IS_DISK",
                                      DIAG_INT,&diskette_inserted);
                        if (diskette_inserted == TRUE)
                                fd_exit_message();
                        else {
                                DFD_LOOP_TEST_message();
                                sleep(2);
                        }
                }
                clean_up();
        }

        /* ......................................... */
        /* If not ELA mode, do the tests.            */
        /* ......................................... */

        if (da_input.dmode != DMODE_ELA)
                do_fd_tests();

        if(da_input.console == CONSOLE_TRUE)
                chk_asl_return(diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,NULL,NULL));

        /* ......................................... */
        /* Error log analysis                        */
        /* ......................................... */
        if ( da_input.loopmode == LOOPMODE_NOTLM
              && ( da_input.dmode == DMODE_PD || da_input.dmode == DMODE_ELA) )
                do_fd_ela();

        clean_up();
}

/* ==================================================== */
/*                      END MAIN                        */
/* ==================================================== */

/*
 * NAME: int drive_type();
 *
 * FUNCTION: Look at the value field in the CuAt database.
 *
 * RETURNS: 1 if drive is 4MB, 0 if drive is not a 4MB,
 *          -1 if error.
 */

int drive_type()
{
      struct CuAt *cuat;
      struct listinfo obj_info;
      char criteria[128];

      sprintf(criteria,"name = %s",da_input.dname);
      cuat = get_CuAt_list(CuAt_CLASS,criteria,&obj_info,1,1);

      if (cuat == (struct CuAt *) - 1) {
              return(-1);
      }
      if (cuat == (struct CuAt *) NULL) {
              return(0);
      }
      if ( strcmp(cuat->value,NAME_OF_4MB_DRIVE) == 0 ) {
              return(1);
      }
      else {
              return(0);
      }

}

/*
 * NAME: clean_up()
 *
 * FUNCTION: Close all open files. If a diskette was inserted into a
 *           diskette drive then indicate to the user to remove the
 *           test diskette.
 *           Exit back to the DIagnostic Controller
 *
 * EXECUTION ENVIRONMENT: Called prior to exiting process.
 *
 *
 * RETURNS: NONE
*/


void clean_up()
{
        int rc;
        if(da_input.loopmode == LOOPMODE_ENTERLM)
        {
                rc = putdavar(da_input.dname,"DISK_D",
                    DIAG_INT,&fd->disk_density);
                rc = putdavar(da_input.dname,"DRIVE_L",
                    DIAG_INT,&fd->location);
                rc = putdavar(da_input.dname,"DRIVE_S",
                    DIAG_INT,&fd->size);
                rc = putdavar(da_input.dname,"DRIVE_C",
                    DIAG_INT,&fd->capacity);
                rc = putdavar(da_input.dname,"DRIVE_D",
                    DIAG_INT,&fd->drive_density);
                rc = putdavar(da_input.dname,"IS_DISK",
                    DIAG_INT,&diskette_inserted);
                rc = putdavar(da_input.dname,"T_TYPE",
                    DIAG_INT,&test_type);
        }

        if ( (da_input.loopmode == LOOPMODE_NOTLM ||
              da_input.loopmode == LOOPMODE_EXITLM)
            && diskette_inserted == TRUE
            && da_input.console == CONSOLE_TRUE
            && (DA_CHECKRC_USER() == DA_USER_NOKEY) )
                fd_exit_message();

        if (da_input.loopmode == LOOPMODE_NOTLM ||
            da_input.loopmode == LOOPMODE_EXITLM)
        {
                fd->exORin = AIX_ERROR;
                rc = putdavar(da_input.dname,"EXORIN",
                    DIAG_INT,&fd->exORin);
        }

        /* ......................................... */
        /* Return the device to its previous state.  */
        /* ......................................... */
        if(dev_fdes > 0)
                close(dev_fdes);

        if (dev_state != AIX_ERROR)
                if ( (initial_state(dev_state,da_input.dname)) == AIX_ERROR)
                        DA_SETRC_ERROR(DA_ERROR_OTHER);

        term_dgodm();
        if(catd != CATD_ERR)
                catclose(catd);
        if(da_input.console == CONSOLE_TRUE &&
           da_input.exenv != EXENV_SYSX)
                diag_asl_quit(NULL);
        DA_EXIT();
}

/*
 * NAME: do_fd_ela()
 *
 * FUNCTION: Search for the errors in data base
 *
 * RETURNS:  NONE
*/

void do_fd_ela(rc)
{
        int ela_rc , ela_get_rc;
        char crit[128];
        struct errdata errdata_ptr;

        sprintf(crit,"%s -N %s -j %X,%X,%X,%X",
                       da_input.date  ,
                       da_input.dname ,
                       ERRID_DISKETTE_ERR1  ,
                       ERRID_DISKETTE_ERR2  ,
                       ERRID_DISKETTE_ERR4  ,
                       ERRID_DISKETTE_ERR6 );

        ela_get_rc = error_log_get(INIT,crit,&errdata_ptr);
        ela_rc = error_log_get(TERMI,crit,&errdata_ptr);

        if(ela_get_rc >0)
                exit_with_frub( 999 );

}

/*
 * NAME:  int_handler
 *
 * FUNCTION: Perform clean up on receipt of an interrupt
 *
 * NOTES:
 *
 * RETURNS: None
 *
 */
void int_handler(int sig)
{
        diskette_inserted = FALSE;
        clean_up();
}

