static char sccsid[] = "@(#)74	1.17  src/bos/diag/da/fd/do_fd_test.c, dafd, bos412, 9446C 11/18/94 13:38:23";
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: do_fd_tests
 *		read_write_tests
 *		
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <sys/devinfo.h>
#include <sys/types.h>

#include <stdio.h>
#include <NLchar.h>
/* #include <NLctype.h> */
#include <fcntl.h>              /* file control defines etc.            */
#include <nl_types.h>
#include <sys/fd.h>
#include <sys/errno.h>
#include <sys/devinfo.h>

#include "fd_set.h"
#include "fd_drvf.h"
#include "fda_rc.h"             /* define tu return codes               */
#include "fdatuname.h"          /* define TU test names                 */
#include "dfd_msg.h"            /* catalog offsets values               */
#include "diag/diago.h"
#include "diag/diag_exit.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"

extern nl_catd catd;
extern struct fd_drive *fd;
extern int tmode;
extern struct  tm_input da_input;
extern int diskette_inserted;
extern int test_type;
extern int drive_4MB;
int dev_fdes;
int density_check_status = 0;
void read_write_tests();

/* ************************************************************************* */
/*  do_fd_tests() preform the series of required tests                       */
/* ************************************************************************* */

/*
 * NAME: void do_fd_tests()
 *
 * FUNCTION: Do the required tests depending on the mode
 *
 * EXECUTION ENVIRONMENT: called from main() SEE: dfd_main.c
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
*/

void do_fd_tests()
{
        int tn;
        int rc  = NO_ERROR;
        int prev_density;
        char d_driver[80];

        diskette_inserted = FALSE;

        if(fd->location == DRIVE_0)
        {
                if (drive_4MB)
                        strcpy(d_driver,RFD_0_35_36);
                else
                        strcpy(d_driver,RFD_0_35_H);
        }
        else
                strcpy(d_driver,RFD_1_525_H);

        dev_fdes =  open( d_driver , O_RDWR | O_NDELAY );

        if (dev_fdes <= 0 )
        {
		switch (errno)
		{/* defined with errno.h */
			case EIO:           /* DD has already been in use */
			case ENXIO:
				assign_fd_frub(4,ADAPTER_TEST);
				DA_SETRC_STATUS(DA_STATUS_BAD);
				clean_up();
			case EBUSY:         /* DD has already been in use */
				DA_SETRC_TESTS(DA_TEST_NOTEST);
				DA_SETRC_ERROR(DA_ERROR_OPEN);
				clean_up();
			default:
				DA_SETRC_ERROR(DA_ERROR_OTHER);
				clean_up();
		} /* end switch */

        }

        if ( (da_input.console == CONSOLE_TRUE) &&
             (fd->size == FIVE25) &&
             (da_input.loopmode == LOOPMODE_ENTERLM) &&
             (da_input.system == SYSTEM_FALSE) )
        {
                if (da_input.exenv != EXENV_SYSX)
                        rc = DRIVE_LOCATION_query();
                else
                        rc = 1;
                if (rc == 1)
                        fd->exORin = EXTERNAL_DISKETTE_DRIVE;
                else
                        fd->exORin = INTERNAL_DISKETTE_DRIVE;

                putdavar(da_input.dname,"EXORIN",DIAG_INT,&fd->exORin);
        }

        if ( !(da_input.exenv == EXENV_SYSX) )
                run_tu_test(ADAPTER_TEST);

        switch(tmode)
        {
        case INTERACTIVE_TEST_MODE:
                if ( da_input.exenv == EXENV_SYSX )
                {
                        /* System Exerciser with media */
			if(da_input.advanced == ADVANCED_TRUE )
                        	rc = DFD_EXER_TEST_menu();
			else	rc = ANSWERED_YES+1;
                        if (rc == ANSWERED_YES)
                        {
                                  READ_WRITE_INSERT_message();
                                  test_type = READ_TEST;
                        }
                        else
                        {
                                  test_type = ADAPTER_TEST;
                        }
                        /* This will tell the cntrl to */
                        /* go on to the next DA.       */
                        diag_asl_quit();
                        clrdainput();

			/* don't run test in interaction pass	*/
                        break;
                }
                test_type = ADAPTER_TEST;
                /* ................................................. */
                /* SELECT/DESELECT turns the drive motor on and      */
                /* then off after 5 seconds. The user is then asked  */
                /* if the drive light went on and off                */
                /* ................................................. */
                SELECT_TEST_message();
                run_tu_test(SELECT_TEST);
                sleep(3);
                run_tu_test(DESELECT_TEST);

                rc = DESELECT_TEST_menu();
                if(rc != ANSWERED_YES )
                        assign_fd_frub( rc ,DESELECT_TEST);
                RECALIBRATE_TEST_message();
                run_tu_test(RECALIB_TEST);
                sleep(1);
                run_tu_test(DESELECT_TEST);
                rc = DISK_CHANGE_TEST_menu();
                if(rc == ANSWERED_YES )
                {
                        DISK_CHANGE_TEST_message();
                        diskette_inserted = TRUE;
                        run_tu_test(DISK_CHANGE_TEST);
                        run_tu_test(DESELECT_TEST);
                        test_type = DISK_WR_PROT_TEST;
                        run_tu_test(DISK_WR_PROT_TEST);
                        sleep(1);
                        run_tu_test(DESELECT_TEST);
                        test_type = READ_TEST;
                        read_write_tests(TRUE);
                }
                if (drive_4MB)
                {
                        /* do the 2MB read/write tests */
                        fd->disk_density = HIGH_DENSITY;
                        rc = READ_WRITE_query();
                        if(rc == ANSWERED_YES)
                        {
                                test_type = READ_TEST;
                                read_write_tests(TRUE);
                        }
                        else
                                fd->disk_density = HIGH_DENSITY_4MB;
                }
                /* low density test */
                prev_density = fd->disk_density;
                fd->disk_density = LOW_DENSITY;
                rc = READ_WRITE_query();
                if(rc == ANSWERED_YES)
                {
                        test_type = READ_TEST;
                        read_write_tests(TRUE);
                }
                else
                        fd->disk_density = prev_density;

                if (diskette_inserted == TRUE
                    && da_input.loopmode == LOOPMODE_NOTLM)
                        fd_exit_message();

                break;
        case NO_VIDEO_TEST_MODE:
                run_tu_test(SELECT_TEST);
                run_tu_test(DESELECT_TEST);
                run_tu_test(RECALIB_TEST);
                run_tu_test(DESELECT_TEST);
                break;
        case TITLES_ONLY_TEST_MODE:

                if ( da_input.system == SYSTEM_TRUE )
                {
                        if (da_input.loopmode == LOOPMODE_NOTLM)
                        {
                                ADAPTER_TEST_message();
                        }
                        else
                        {
                                if (da_input.loopmode == LOOPMODE_ENTERLM)
                                {
                                        DFD_LOOP_TEST_message();
                                }
                        }
                }

                if(da_input.loopmode ==  LOOPMODE_INLM)
                {
                        rc = getdavar(da_input.dname,"DISK_D",
                            DIAG_INT,&fd->disk_density);
                        rc = getdavar(da_input.dname,"DRIVE_L",
                            DIAG_INT,&fd->location);
                        rc = getdavar(da_input.dname,"DRIVE_S",
                            DIAG_INT,&fd->size);
                        rc = getdavar(da_input.dname,"DRIVE_C",
                            DIAG_INT,&fd->capacity);
                        rc = getdavar(da_input.dname,"DRIVE_D",
                            DIAG_INT,&fd->drive_density);
                        rc = getdavar(da_input.dname,"IS_DISK",
                            DIAG_INT,&diskette_inserted);
                        rc = getdavar(da_input.dname,"T_TYPE",
                            DIAG_INT,&test_type);
                        if (da_input.exenv != EXENV_SYSX)
                                DFD_LOOP_TEST_message();
                }

                /* run the tests that don't require user interaction. */
                run_tu_test(SELECT_TEST);
                sleep(3);
                run_tu_test(DESELECT_TEST);
                run_tu_test(RECALIB_TEST);
                sleep(1);
                run_tu_test(DESELECT_TEST);

                /* run the last test the user selected in ENTER_LM.   */
                switch(test_type)
                {
                case ADAPTER_TEST:
                        break;
                case DISK_WR_PROT_TEST:
                        run_tu_test(DISK_WR_PROT_TEST);
                        run_tu_test(DESELECT_TEST);
                        break;
                case READ_TEST:
                        read_write_tests(FALSE);
                        break;
                default:
                        break;
                } /* end test_type switch */
                break;
        default:
                break;
        } /* end tmode switch */

        close(dev_fdes);

} /* end do_fd_test */


/*
 * NAME: void read_write_tests(menu_status)
 *
 * FUNCTION: Perform the read write test (IF menu_status is TRUE then
 *                                        display messages &/or menus )
 *
 * EXECUTION ENVIRONMENT: called from do_fd_tests();
 *
 *
 * RETURNS: NONE
*/

void read_write_tests(menu_status)
int menu_status;
{
        int tn;
        int rc;
        int read_rc = -1;
        char read_buffer[4097];
        char d_driver[80];

        /** ....................................................... **/
        /** TUs 6 - INDEX_TEST ,  7 - STEP_TEST , 8 - READ_TEST     **/
        /**     9 - WRITE_TEST , 10 WR_READ_CMP_TEST                **/
        /**    11 - LOW_DENSITY, 12 - VERIFY (SKIPPED)              **/
        /**    13 - SPEED_TEST , 14 - HEAD_SETTLE_TEST              **/
        /**                                                         **/
        /** require a formatted diskette that is NOT copy protected **/
        /** NOTE: TU 12 (VERIFY TEST) is skipped (it takes to long) **/
        /**       TU 11 (LOW_DENSITY) requires correct LD diskette  **/
        /** ....................................................... **/


        if(menu_status == TRUE) {
                READ_WRITE_INSERT_message();
                diskette_inserted = TRUE;
                READ_WRITE_RUNNING_message();
        }

        if (fd->disk_density != LOW_DENSITY) {
                if (drive_4MB && fd->disk_density == HIGH_DENSITY)
                {
                        /* close the 4MB dd, open the 2MB dd */
                        if (dev_fdes > 0)
                                close(dev_fdes);
                        strcpy(d_driver,RFD_0_35_18);
                        dev_fdes =  open( d_driver , O_RDWR | O_NDELAY );
                        if (dev_fdes <= 0 )
                        {
					switch (errno)
					{/* defined with errno.h */
						case EIO:
						case ENXIO:
							assign_fd_frub(4,ADAPTER_TEST);
							DA_SETRC_STATUS(DA_STATUS_BAD);
							clean_up();
						case EBUSY:
						/* DD has already been in use */
							DA_SETRC_TESTS(DA_TEST_NOTEST);
							DA_SETRC_ERROR(DA_ERROR_OPEN);
							clean_up();
						default:
							DA_SETRC_ERROR(DA_ERROR_OTHER);
							clean_up();
					} /* end switch */
                        }
                }
                /*********************************************/
                /*               DENSITY CHECK               */
                /* Check for correct density diskette before */
                /* running the read/write tests.             */
                /*********************************************/
                do {
                        run_tu_test(RECALIB_TEST);
                        run_tu_test(DISABLE_RETRY);
                        read_rc = read(dev_fdes,read_buffer,4096);
                        read_rc = read(dev_fdes,read_buffer,4096);
                        run_tu_test(ENABLE_RETRY);
                        run_tu_test(DESELECT_TEST);
                        if (read_rc == -1) {
                                if (menu_status == TRUE) {
                                        diskette_inserted = FALSE;
                                        rc = DENSITY_CHECK_menu();
                                        if (rc == 1)
                                                assign_fd_frub(0,READ_TEST);
                                        else
                                                READ_WRITE_INSERT_message();
                                        diskette_inserted = TRUE;
                                        READ_WRITE_RUNNING_message();
                                } else {
                                        assign_fd_frub(0,READ_TEST);
                                }
                        }
                } while (read_rc == -1 );
                run_tu_test(SELECT_TEST);
                run_tu_test(RECALIB_TEST);
                run_tu_test( INDEX_TEST);
                run_tu_test(DESELECT_TEST);
                run_tu_test( STEP_TEST );
                run_tu_test(DESELECT_TEST);
                run_tu_test( WRITE_TEST);
                run_tu_test(DESELECT_TEST);
                run_tu_test( READ_TEST );
                run_tu_test(DESELECT_TEST);
                if (drive_4MB && fd->disk_density == HIGH_DENSITY)
                {
                        run_tu_test(LOW_DENSITY_TEST_2MB);
                }
                run_tu_test( WR_READ_CMP_TEST );
                run_tu_test( DESELECT_TEST);


		/*
		 * After causing more problems, it was decided on 11/18/94
		 * that these tests didn't need to be run on any system.
		 *
                if (!drive_4MB) 
		{
       		    if(!(da_input.exenv == EXENV_SYSX)  && 
			   ! (da_input.exenv == EXENV_CONC))
		    {
               	 	run_tu_test( SPEED_TEST    );
                       	run_tu_test(HEAD_SETTLE_TEST); 
		    }
		} */

                run_tu_test(DESELECT_TEST);
        }

        if (fd->disk_density == LOW_DENSITY) {
                if (dev_fdes > 0)
                        close(dev_fdes);
                if(fd->location == DRIVE_0) {
                        if (drive_4MB)
                                strcpy(d_driver,RFD_0_35_9);
                        else
                                strcpy(d_driver,RFD_0_35_L);
                }
                if(fd->location == DRIVE_1)
                                strcpy(d_driver,RFD_1_525_L);
                dev_fdes =  open( d_driver , O_RDWR | O_NDELAY );
                if (dev_fdes <= 0 ) {
                        DA_SETRC_ERROR(DA_ERROR_OPEN);
                        clean_up();
                }
                /*********************************************/
                /*               DENSITY CHECK               */
                /* Check for correct density diskette before */
                /* running the read/write tests.             */
                /*********************************************/
                do {
                        density_check_status = 0;
                        run_tu_test(RECALIB_TEST);
                        run_tu_test(DISABLE_RETRY);
                        run_tu_test(LOW_DENSITY_TEST_1MB);
                        run_tu_test(ENABLE_RETRY);
                        run_tu_test(DESELECT_TEST);
                        if (density_check_status != NO_ERROR) {
                                if (menu_status == TRUE) {
                                        diskette_inserted = FALSE;
                                        rc = DENSITY_CHECK_menu();
                                        if (rc == 1)
                                                assign_fd_frub(0,READ_TEST);
                                        else
                                                READ_WRITE_INSERT_message();
                                        diskette_inserted = TRUE;
                                        READ_WRITE_RUNNING_message();
                                } else {
                                        assign_fd_frub(0,READ_TEST);
                                }
                        }
                } while (density_check_status != NO_ERROR);
                run_tu_test(SELECT_TEST);
                run_tu_test( DESELECT_TEST );
                run_tu_test( INDEX_TEST);
                run_tu_test(DESELECT_TEST);
                run_tu_test( STEP_TEST );
                run_tu_test(DESELECT_TEST);
                run_tu_test( WRITE_TEST);
                run_tu_test(DESELECT_TEST);
                run_tu_test( READ_TEST );
                run_tu_test(DESELECT_TEST);
                run_tu_test( WR_READ_CMP_TEST );
                run_tu_test( DESELECT_TEST);
        }

}

