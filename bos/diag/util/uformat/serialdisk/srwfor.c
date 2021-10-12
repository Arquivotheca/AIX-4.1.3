static char sccsid[] = "@(#)71	1.8.2.3  src/bos/diag/util/uformat/serialdisk/srwfor.c, dsauformat, bos411, 9428A410j 2/7/94 16:07:30";
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS: clean_up
 *		command_certify
 *		command_format
 *		command_prolog
 *		int_handler
 *		main
 *		process_sense_data
 *		tu_test
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include        <stdio.h>
#include        <sys/types.h>
#include        <fcntl.h>
#include        <nl_types.h>
#include        <limits.h>
#include	<locale.h>
#include        <memory.h>
#include        <errno.h>
#include        <sys/buf.h>
#include	<signal.h>
#include        <diag/har2_atu.h>
#include        <sys/scsi.h>
#include        <sys/scdisk.h>

#include "diag/da.h"            /* FRU Bucket Database */
#include "diag/da_rc.h"
#include "diag/dcda_msg.h"
#include "diag/diago.h"
#include "diag/dascsi.h"
#include "diag/diag_exit.h"     /* return codes for DC */
#include "diag/tmdefs.h"        /* diagnostic modes and variables     */
#include "sys/cfgodm.h"
#include "ufmt_msg.h"

#include        "uformat.h"

/*  #define DEBUG_SRWFOR  */



int     progmode;       /* defines what device we are testing    */
int	certify_after=0; /* flag to determine what to set in Mode select */
int grandparent_cfg_state = -1;
int parent_cfg_state = -1;
int device_cfg_state = -1;
char grandparent_name[32];
char parent_name[32];
char devname[32];
char dd_name[32];
DRIVE_INFO      drive_info;     /* information about drive               */
uchar           tu_buffer[128 * 512];  /* for write and request sense    */
uchar           ms_buffer[256]; /* buffer for mode sense                 */
HAR2_TUTYPE     tucb;           /* structure for scsi commands           */
int             i_reserved_it;  /* set to true when a reserve is done    */
int             is_working;     /* true when format or certify drive     */
int		pvid_destroyed; /* true then need to cleanup data base   */
int             fdes;           /* file descriptor of device             */
int             maxblock;       /* maximum block address on drive        */
int             blocklength;    /* block length                          */
int             curblock;       /* next block to read from drive         */
int             old_percent;
int             error_block;    /* lba of first block in error           */
char            connwhere[16];
short		reassign_block=0;

extern		int             disp_menu();
extern		int		exectu();
struct		listinfo	obj_info;
struct		CuDv		*cudv;
struct		CuDv		*cudv_selected;
extern		void	clear_pvid();
struct  	da_menu	dmnu;

#define PROLOG_STEP_1 1
#define PROLOG_STEP_2 2
#define PROLOG_STEP_3 3
#define PROLOG_STEP_5 5
#define PROLOG_STEP_6 6
#define PROLOG_STEP_7 7
#define PROLOG_STEP_8 8
#define PROLOG_STEP_9 9
#define PROLOG_STEP_10 10
#define PROLOG_NUM_STEPS 10

#define CERTIFY_STEP_1 1
#define CERTIFY_STEP_2 2
#define CERTIFY_STEP_3 3
#define CERTIFY_STEP_4 4
#define CERTIFY_STEP_5 5
#define CERTIFY_STEP_6 6
#define CERTIFY_STEP_7 7
#define CERTIFY_STEP_8 8
#define CERTIFY_STEP_9 9
#define CERTIFY_NUM_STEPS 9

#define FORMAT_STEP_1 1
#define FORMAT_STEP_2 2
#define FORMAT_STEP_3 3
#define FORMAT_STEP_4 4
#define FORMAT_STEP_5 5
#define FORMAT_STEP_6 6
#define FORMAT_STEP_7 7
#define FORMAT_STEP_8 8
#define FORMAT_NUM_STEPS 8

/* begin main */

int
main(argc, argv, envp)
        int             argc;   /* argument list count           */
        char          **argv;   /* argument list vector          */
        char          **envp;   /* environment list vector       */
{
        void            clean_up();     /* what to do on exit            */
        int             command_prolog();
        int             command_certify();
        int             command_format();
	void		int_handler(int);
	int		process_sense_data();
        char            criteria[40];   /* temp for odm get */
        int             temp_i; /* temporary int */
	int             rc;
        int             ipl_flag;
	struct		sigaction	act;

	setlocale(LC_ALL,"");
	 act.sa_handler = int_handler;
	 sigaction(SIGINT, &act, (struct sigaction *)NULL);

        is_working = 0; /* drive is not currently formatting */
	pvid_destroyed = 0; /* pvid if exists is still intact */
        i_reserved_it = 0;      /* no reservation yet            */
        drive_info.percent_complete = 0;        /* nothing complete yet */
        drive_info.drive_capacity = 0;
        drive_info.rec_data_errors = 0;
        drive_info.unrec_data_errors = 0;
        drive_info.rec_equ_check_errors = 0;
        drive_info.unrec_equ_check_errors = 0;


        init_dgodm();           /* set up the odm */
        diag_asl_init(0);       /* set up asl */

        strcpy(drive_info.loc_code, "");

        dmnu.catfile = "ufmt.cat";

        /*
         * Now, lets actually to something usefull.
         */

        /* get the device name and mode from argv[] */

        strcpy(devname,argv[1]);
        progmode = (*argv[3] - '0');
	strcpy(connwhere,argv[4]);
	strcpy(parent_name,argv[5]);
	strcpy(grandparent_name,argv[6]);
        certify_after = (*argv[7] - '0');

        /*
         *  config stuff
         */
	grandparent_cfg_state = configure_device(grandparent_name);
	parent_cfg_state = configure_device(parent_name);
	device_cfg_state = configure_device(devname);
	if ( (grandparent_cfg_state == -1) ||
	     (parent_cfg_state == -1) ||
	     (device_cfg_state == -1) ) {
		disp_menu(MENU_TERMINATED);
		clean_up();
	}	

	strcpy(dd_name,"/dev/r");
	strcat(dd_name,argv[1]);

        fdes = openx(dd_name, O_RDWR, NULL, SC_DIAGNOSTIC);
        if (fdes < 0) {
                ipl_mode( &ipl_flag );
                if( ipl_flag == 0 ) {
                        if (progmode == UFORMAT) {
                                disp_menu(MENU_F_USE_FLOPPY);
                        } else {
                                disp_menu(MENU_C_USE_FLOPPY);
                        }
                } else {
                        disp_menu(MENU_TERMINATED);
                }
                clean_up();
        }
	rc = command_prolog();
        if (rc != 0) {
#ifdef DEBUG_SRWFOR
	diag_asl_msg("rc from command_prolog() = %d\n",rc);
#endif
                if( fdes > 0 )
                        close(fdes);
                clean_up();
        }
        if (progmode == UFORMAT) {
		rc = command_format();
                if (rc != 0) {
#ifdef DEBUG_SRWFOR
	diag_asl_msg("rc from command_format() = %d\n",rc);
#endif
                        if( fdes > 0 )
                                close(fdes);
                        clean_up();
                }
		if(certify_after)
			progmode=UCERTIFY;
	}
        if (progmode == UCERTIFY) {
		rc = command_certify();
                if (rc != 0) {
#ifdef DEBUG_SRWFOR
	diag_asl_msg("rc from command_certify() = %d\n",rc);
#endif
                        if( fdes > 0 )
                                close(fdes);
                        clean_up();
                }
        }
        if( fdes > 0 )
                close(fdes);

        clean_up();             /* DOES NOT RETURN */

        return (0);             /* keep lint happy */
}                               /* main end */


/*
 * NAME: int command_prolog()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *      This function must have sole use of the device to be tested.
 *
 * NOTES: More detailed description of the function, down to
 *      what bits / data structures, etc it manipulates.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: NONE
 */

int
command_prolog()
{
        extern int      i_reserved_it;  /* set when reserve is good      */
        extern int      fdes;   /* file descriptor of device     */
        int             rc;     /* return code from tu_test()    */
        int             tu_test();
        /*
         * counter for reaching various checkpoints in the test procedure
         */
        int             stepcount[PROLOG_NUM_STEPS + 1];
        int             step;   /* current step executing        */
        int             temp_i; /* temporary int */
        int             disp_menu();


        drive_info.percent_complete = 0;
	if (progmode == UFORMAT) {
		temp_i = disp_menu(MENU_F_WARNING);
		if (temp_i != 1)
			clean_up();
	}
	else
        	temp_i = disp_menu(MENU_PLEASE_STAND_BY);

        /* clear accumulative step counters */
        for (step = 0; step <= PROLOG_NUM_STEPS; ++step)
                stepcount[step] = 0;

        /* start at step 1 */
        step = PROLOG_STEP_1;

        /* Run all of the necessary tests  */
        while (step) {

#ifdef DEBUG_SRWFOR
        diag_asl_msg("In command_prolog, step = %d\n",step);
#endif

                ++stepcount[step];
                switch (step) {
                case PROLOG_STEP_1:     /* RESERVE UNIT */
                        rc = tu_test(fdes, SCATU_RESERVE_UNIT);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                return (-1);
                        case SCATU_TIMEOUT:
                                step = PROLOG_STEP_5;
                                break;
                        case SCATU_BUSY:        /* or command_error */
                                if (stepcount[PROLOG_STEP_1] == 1) {
                                        step = PROLOG_STEP_6;
                                } else {
                                        step = PROLOG_STEP_5;
                                }
                                break;
                        case SCATU_RESERVATION_CONFLICT:
                                if (stepcount[PROLOG_STEP_1] == 1) {
                                        step = PROLOG_STEP_7;
                                } else {
                                        step = PROLOG_STEP_5;
                                }
                                break;
                        case SCATU_CHECK_CONDITION:
                                step = PROLOG_STEP_2;
                                break;
                        case SCATU_GOOD:
                                i_reserved_it = 1;
                                step = PROLOG_STEP_8;
                                break;
                        default:
                                return (-1);
                        }
                        break;
                case PROLOG_STEP_2:     /* REQUEST SENSE */
                        rc = tu_test(fdes, SCATU_REQUEST_SENSE);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                return (-1);
                        case SCATU_BUSY:
                                if (stepcount[PROLOG_STEP_2] == 1) {
                                        step = PROLOG_STEP_6;
                                } else {
                                        step = PROLOG_STEP_5;
                                }
                                break;
                        case SCATU_TIMEOUT:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                                step = PROLOG_STEP_5;
                                break;
                        case SCATU_GOOD:
                                step = PROLOG_STEP_3;
                                break;
                        default:
                                return (-1);
                        }
                        break;
                case PROLOG_STEP_3:     /* examine the request sense data */

                        if (tucb.scsiret.sense_key == 0x06) {
                                /* UNIT ATTENTION */
                                if (stepcount[PROLOG_STEP_3] == 1)
					/* if check from a Mode select, retry */
					if(stepcount[PROLOG_STEP_10] == 1)
						step = PROLOG_STEP_10;
					else
                                        	step = PROLOG_STEP_1;
                                else 
                                        step = PROLOG_STEP_5;
                                break;
                        } else if (tucb.scsiret.sense_key == 0x02) {
                                switch( tucb.scsiret.sense_code ) {
                                        case 0x0400 :
                                        case 0x0401 :
                                        case 0x0402 :
                                                step = PROLOG_STEP_8;
                                                break;
                                        default:
                                                step = PROLOG_STEP_5;
                                                break;
                                }
                                break;
                        }
                        /* unaccounted for sense key, dead drive */
                        step = PROLOG_STEP_5;
                        break;
                case PROLOG_STEP_5:     /* unable to use drive */
                        disp_menu(MENU_TERMINATED);
                        return (-1);
                case PROLOG_STEP_6:     /* we got here because of a BUSY */
                        if( fdes > 0 )
                                close(fdes);

                        /* Reopen device, sending SCSI device reset */
                        fdes = openx(dd_name, O_RDWR, NULL, SC_FORCED_OPEN);

                        /* if open ok, close and reopen in diag mode */
                        if (fdes < 0) {
                                step = PROLOG_STEP_5;
                        } else {
                                if( fdes > 0 )
                                        close(fdes);
                                fdes = openx(dd_name, O_RDWR, NULL,
                                             SC_DIAGNOSTIC);
                        }

                        /* Reopen device, in diagnostic mode */
                        /* and test if open ok.              */
                        if (fdes < 0) {
                                step = PROLOG_STEP_5;
                        } else {
                                step = PROLOG_STEP_1;
                        }
                        break;
                case PROLOG_STEP_7:     /* RESERVATION CONFLICT */
                        step = PROLOG_STEP_5;
                        break;
                case PROLOG_STEP_8:     /* do a START_UNIT */
                        rc = tu_test(fdes, SCATU_START_STOP_UNIT);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                return (-1);
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                        case SCATU_TIMEOUT:
                                step = PROLOG_STEP_5;
                                break;
                        case SCATU_GOOD:
				if(progmode == UCERTIFY)
	                                step = PROLOG_STEP_9;
				else
					step = PROLOG_STEP_10;
                                break;
                        default:
                                return (-1);
                        }
                        break;
                case PROLOG_STEP_9:     /* do a MODE_SENSE */
                        rc = tu_test(fdes, SCATU_MODE_SENSE);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                return (-1);
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                        case SCATU_TIMEOUT:
                                step = PROLOG_STEP_5;
                                break;
                        case SCATU_GOOD:
                                memcpy(ms_buffer, tu_buffer, 256);
                                /* if block size not 512 do a format */
                                if (((tu_buffer[10] != 0x02) ||
                                     (tu_buffer[11] != 0x00)) &&
                                    (progmode == UCERTIFY)) {
                                        disp_menu(MENU_SHOULD_FORMAT);
                                }
                                step = PROLOG_STEP_10;
                                break;
                        default:
                                return (-1);
                        }
                        break;
                case PROLOG_STEP_10:    /* do a MODE_SELECT */
                        rc = tu_test(fdes, SCATU_MODE_SELECT);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                return (-1);
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_TIMEOUT:
                                step = PROLOG_STEP_5;
                                break;
                        case SCATU_CHECK_CONDITION:
				step = PROLOG_STEP_2;
				break;
                        case SCATU_GOOD:
                                /* get out */
                                step = 0;
                                break;
                        default:
                                return (-1);
                        }
                        break;

                default:

                        return (-1);

                }
        }                       /* end of while and switch */
        return (0);
}                               /* end of command_prolog */


/*
 * NAME: int command_certify()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *      This function must have sole use of the device to be tested.
 *
 * NOTES: More detailed description of the function, down to
 *      what bits / data structures, etc it manipulates.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: NONE
 */

int
command_certify()
{
        int             block_with_error = 0;
        extern int      i_reserved_it;  /* set when reserve is good      */
        extern int      fdes;   /* file descriptor of device     */
        int             rc;     /* return code from tu_test()    */
        int             tu_test();
        /*
         * counter for reaching various checkpoints in the test procedure
         */
        int             stepcount[CERTIFY_NUM_STEPS + 1];
        int             step;   /* current step executing        */
        int             temp_i; /* temporary int */
        int             disp_menu();
        int             command_prolog();
        int             retry = 0;

        curblock = 0;
        old_percent = 0;

        drive_info.percent_complete = 0;
        temp_i = disp_menu(MENU_PLEASE_STAND_BY);

        /* clear accumulative step counters */
        for (step = 0; step <= CERTIFY_NUM_STEPS; ++step)
                stepcount[step] = 0;

        /* start at step 1 */
        step = CERTIFY_STEP_1;

        /* Run all the necessary tests  */
        while (step) {

#ifdef DEBUG_SRWFOR
        diag_asl_msg("In command_certify, step = %d\n",step);
#endif

                ++stepcount[step];
                switch (step) {
                case CERTIFY_STEP_1:
                        rc = tu_test(fdes, SCATU_READ_CAPACITY);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                return (-1);
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                        case SCATU_TIMEOUT:
                                step = CERTIFY_STEP_5;
                                break;
                        case SCATU_GOOD:
                                maxblock = (unsigned) tu_buffer[0] << 24 |
                                        (unsigned) tu_buffer[1] << 16 |
                                        (unsigned) tu_buffer[2] << 8 |
                                        (unsigned) tu_buffer[3];
                                blocklength = (unsigned) tu_buffer[4] << 24 |
                                        (unsigned) tu_buffer[5] << 16 |
                                        (unsigned) tu_buffer[6] << 8 |
                                        (unsigned) tu_buffer[7];
                                drive_info.drive_capacity =
                                          (maxblock * blocklength) / 1000000;
                                step = CERTIFY_STEP_2;
                                is_working = 1;
                                break;
                        default:
                                return (-1);
                        }
                        break;
                case CERTIFY_STEP_2:    /* read a chunk */
                        rc = tu_test(fdes, SCATU_READ_EXTENDED);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                is_working = 0;
                                return (-1);
                        case SCATU_TIMEOUT:
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                                step = CERTIFY_STEP_4;
                                break;
                        case SCATU_GOOD:
                                curblock += 128;
                                step = CERTIFY_STEP_3;
                                break;
                        default:
                                is_working = 0;
                                return (-1);
                        }
                        break;
                case CERTIFY_STEP_3:
                        if (curblock > maxblock) {
                                /* finished */
                                step = CERTIFY_STEP_8;
                                drive_info.percent_complete = 100;
                                is_working = 0;
                                disp_menu(MENU_PLEASE_STAND_BY);
                        } else {
                                step = CERTIFY_STEP_2;
                                drive_info.percent_complete =
                                        (100L * curblock) / maxblock;
                                if( drive_info.percent_complete !=
                                        old_percent ) {
                                        disp_menu(MENU_PLEASE_STAND_BY);
                                        old_percent =
                                                drive_info.percent_complete;
                                }
                        }
                        break;
                case CERTIFY_STEP_4:    /* REQUEST SENSE */
                        rc = tu_test(fdes, SCATU_REQUEST_SENSE);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                is_working = 0;
                                return (-1);
                        case SCATU_TIMEOUT:
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                                step = CERTIFY_STEP_5;
                                break;
                        case SCATU_GOOD:
                                step = CERTIFY_STEP_6;
                                break;
                        default:
                                is_working = 0;
                                return (-1);
                        }
                        break;
                case CERTIFY_STEP_5:    /* unable to use drive */
                        is_working = 0;
                        disp_menu(MENU_TERMINATED);
                        return (-1);
                case CERTIFY_STEP_6:    /* examine the request extended sense
                                         * data */
			reassign_block=0;
                        switch (tucb.scsiret.sense_key & 0x0f) {
                        case 6:/* UNIT ATTENTION *//* was reset */
                                if(block_with_error != curblock) {
                                        retry = 0;
                                        block_with_error = curblock;
                                }
                                if(( retry < 2) ||
                                   (curblock != block_with_error)){
                                        ++retry;
                                        if (command_prolog() != 0) {
                                                is_working = 0;
                                                return (-1);
                                        }
                                } else {
                                        is_working = 0;
                                        return(-1);
                                }
                                step = CERTIFY_STEP_3;
                                break;
                        default:
                                step = process_sense_data(
					tucb.scsiret.sense_key & 0x0f,
					tucb.scsiret.sense_code);
                                break;
			}
                        if (step != CERTIFY_STEP_6)
                                break;
                        error_block = ((unsigned) tu_buffer[3]) << 24;
                        error_block += ((unsigned) tu_buffer[4]) << 16;
                        error_block += ((unsigned) tu_buffer[5]) << 8;
                        error_block += (unsigned) tu_buffer[6];
                        curblock = error_block + 1;
                        if((drive_info.unrec_equ_check_errors > 1) ||
                            (drive_info.unrec_data_errors > 10)) {
                                step = CERTIFY_STEP_8;
                                break;
                        }
			if((reassign_block) && (certify_after) &&
					(tu_buffer[0] & 0x80))
				step = CERTIFY_STEP_7;
			else
                        	step = CERTIFY_STEP_3;
                        break;
		case CERTIFY_STEP_7:
			rc = tu_test(fdes, SCATU_REASSIGN_BLOCK);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				is_working = 0;
				return (-1);
			case SCATU_TIMEOUT:
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
				step = CERTIFY_STEP_5;
				break;
			case SCATU_CHECK_CONDITION:
				break;
			case SCATU_GOOD:
				step = CERTIFY_STEP_3;
				break;
			default:
				is_working = 0;
				return (-1);
			}
			if( step != CERTIFY_STEP_7 )
				break;
			/* check condition from the reassign */
			step = CERTIFY_STEP_5;
			break;
                case CERTIFY_STEP_8:
                        is_working = 0;
                        rc = tu_test(fdes, SCATU_RELEASE_UNIT);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                return (-1);
                        case SCATU_TIMEOUT:
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                                step = CERTIFY_STEP_5;
                                break;
                        case SCATU_GOOD:
                                /* it works, get out of this loop */
                                step = CERTIFY_STEP_9;
                                i_reserved_it = 0;
                                break;
                        default:
                                return (-1);
                        }
                        break;
                case CERTIFY_STEP_9:
                        disp_menu(MENU_C_COMPLETE);
                        if((drive_info.unrec_equ_check_errors > 1) ||
                            (drive_info.unrec_data_errors > 10)) {
                                disp_menu(MENU_C_COMPLETE_BAD);
                        }
                        step = 0;
                        break;
                default:

                        return (-1);

                }
        }                       /* end of while and switch */
        return (0);
}                               /* end of command_certify */


/*
 * NAME: int command_format()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *      This function must have sole use of the device to be tested.
 *
 * NOTES: More detailed description of the function, down to
 *      what bits / data structures, etc it manipulates.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: NONE
 */

int
command_format()
{
        extern int      i_reserved_it;  /* set when reserve is good      */
        extern int      fdes;   /* file descriptor of device     */
        extern HAR2_TUTYPE tucb;/* structure for scsi commands   */

        int             rc;     /* return code from tu_test()    */
        int             tu_test();
        /*
         * counter for reaching various checkpoints in the test procedure
         */
        int             stepcount[FORMAT_NUM_STEPS + 1];
        int             step;   /* current step executing        */
        int             temp_i; /* temporary int */
        int             disp_menu();
        int             command_prolog();


        old_percent = 0;
        drive_info.percent_complete = 0;
        temp_i = disp_menu(MENU_PLEASE_STAND_BY);

        /* clear accumulative step counters */
        for (step = 0; step <= FORMAT_NUM_STEPS; ++step)
                stepcount[step] = 0;

        /* start at step 1 */
        step = FORMAT_STEP_1;

        /* Run all the necessary tests  */
        while (step) {

#ifdef DEBUG_SRWFOR
        diag_asl_msg("In command_format, step = %d\n",step);
#endif

                ++stepcount[step];
                switch (step) {
                case FORMAT_STEP_1:
                        rc = tu_test(fdes, SCATU_FORMAT_UNIT);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                return (-1);
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_TIMEOUT:
                                step = FORMAT_STEP_5;
                                break;
                        case SCATU_CHECK_CONDITION:
                                step = FORMAT_STEP_2;
                                break;
                        case SCATU_GOOD:
                                is_working = 1;
				pvid_destroyed = 1;
                                step = FORMAT_STEP_2;
                                break;
                        default:
                                return (-1);
                        }
                        break;
                case FORMAT_STEP_2:     /* REQUEST SENSE */
                        rc = tu_test(fdes, SCATU_REQUEST_SENSE);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                return (-1);
                        case SCATU_TIMEOUT:
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                                step = FORMAT_STEP_5;
                                break;
                        case SCATU_GOOD:
                                step = FORMAT_STEP_3;
                                break;
                        default:
                                return (-1);
                        }
                        break;
                case FORMAT_STEP_3:     /* examine the request extended sense
                                         * data */
                        switch (tucb.scsiret.sense_key & 0x0f) {
                        case 6:/* UNIT ATTENTION *//* was reset */
                                if (stepcount[FORMAT_STEP_1] == 2) {
                                        step = PROLOG_STEP_5;
                                        break;
                                }
                                if (command_prolog() != 0) {
                                        return (-1);
                                }
                                step = FORMAT_STEP_1;
                                break;
                        case 0:/* NO SENSE */
                                step = FORMAT_STEP_4;
                                break;
                        case 1:/* RECOVERED ERROR */
                        case 3:/* MEDIA ERROR */
                        case 4:/* HARDWARE ERROR */
                        case 5:/* ILLEGAL REQUEST */
                        case 0x0b:      /* ABORTED COMMAND */
                        case 0x0e:      /* MISCOMPARE */
                        default:
                                step = FORMAT_STEP_5;
                                break;
                        case 2:/* NOT READY */
                                if (tucb.scsiret.sense_code != 0x00404)
                                        step = FORMAT_STEP_5;
                                break;
                        }
                        if (step != FORMAT_STEP_3)
                                break;
                        if (tu_buffer[15] & 0x80) {
                                drive_info.percent_complete =
                                        ((unsigned) tu_buffer[16]) << 8 |
                                        ((unsigned) tu_buffer[17]);
                                drive_info.percent_complete *= 100;
                                drive_info.percent_complete /= 65536;
                        }
                        if( drive_info.percent_complete != old_percent ) {
                                disp_menu(MENU_PLEASE_STAND_BY);
                                old_percent = drive_info.percent_complete;
                        }
                        sleep(1);
                        step = FORMAT_STEP_2;
                        break;
                case FORMAT_STEP_4:
                        /* finished */
                        is_working = 0;
                        step = FORMAT_STEP_6;
                        drive_info.percent_complete = 100;
                        disp_menu(MENU_PLEASE_STAND_BY);
                        break;
                case FORMAT_STEP_5:     /* unable to use drive */
                        is_working = 0;
diag_asl_msg("Sense key: %x Sense code %x\n", tucb.scsiret.sense_key,
		tucb.scsiret.sense_code);
                        disp_menu(MENU_TERMINATED);
                        return (-1);
                case FORMAT_STEP_6:
                        rc = tu_test(fdes, SCATU_RELEASE_UNIT);
                        switch (rc) {
                        case SCATU_BAD_PARAMETER:
                                return (-1);
                        case SCATU_TIMEOUT:
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                                step = FORMAT_STEP_5;
                                break;
                        case SCATU_GOOD:
                                /* it works, get out of this loop */
                                step = FORMAT_STEP_8;
                                i_reserved_it = 0;
                                break;
                        default:
                                return (-1);
                        }
                        break;
                case FORMAT_STEP_8:
                        disp_menu(MENU_F_COMPLETE);
                        step = 0;
                        break;
                default:

                        return (-1);

                }
        }                       /* end of while and switch */
        return (0);
}                               /* end of command_format */

/*
 * NAME: int tu_test
 *
 * FUNCTION: Set up and Execute a particular test unit.
 *
 * NOTES: The large internal buffer used when necesary is tu_buffer.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: It returns the return code from exectu().
 */


int
tu_test(fdescr, tnum)
        int             fdescr;
        int             tnum;
{
        int             rc;        /* functions return code         */
        int             logical_unit;   /* LUN of device on scsi id */
        int             blocks;    /* temporary in read_extended    */

        logical_unit = connwhere[0]-'0';
        if (logical_unit == -1)
                return (SCATU_BAD_PARAMETER);

        tucb.header.tu = tnum;     /* test unit number              */
        tucb.header.mfg = 0;       /* not in manufacturing          */

        rc = scsitu_init(&tucb);   /* init the structure            */
        if (rc != SCATU_GOOD)
                return (SCATU_BAD_PARAMETER);
        tucb.scsitu.flags = 0;
        tucb.scsitu.ioctl_pass_param = PASS_PARAM;
        tucb.scsitu.data_length = 0;
        tucb.scsitu.data_buffer = NULL;
        tucb.scsitu.cmd_timeout = 30;
        tucb.scsitu.scsi_cmd_blk[1] |= logical_unit << 5;
	tucb.scsitu.resvd5 = 0x00;

        switch (tnum) {
        case SCATU_TEST_UNIT_READY:
                /* do nothing special */
                break;
        case SCATU_START_STOP_UNIT:
                tucb.scsitu.scsi_cmd_blk[4] &= 0x01;
                tucb.scsitu.cmd_timeout = 60;
                break;
        case SCATU_REQUEST_SENSE:
                /* zero out the request sense data */
                memset(tu_buffer, 0x0ff, 255);
                tucb.scsitu.scsi_cmd_blk[4] = 255;
                tucb.scsitu.data_length = 255;
                tucb.scsitu.data_buffer = tu_buffer;
                tucb.scsitu.flags |= B_READ;
                break;
        case SCATU_READ_CAPACITY:
                /* zero out the data */
                memset(tu_buffer, 0x000, 8);
                tucb.scsitu.data_length = 8;
                tucb.scsitu.data_buffer = tu_buffer;
                tucb.scsitu.flags |= B_READ;
                break;
        case SCATU_RESERVE_UNIT:
                break;
        case SCATU_MODE_SENSE:
                tucb.scsitu.flags |= B_READ;
                tucb.scsitu.scsi_cmd_blk[4] = 255;
                tucb.scsitu.data_length = 255;
                tucb.scsitu.data_buffer = tu_buffer;
                memset(tu_buffer, 0x0ff, 255);
                break;
        case SCATU_RELEASE_UNIT:
                /* do nothing special */
                break;
        case SCATU_INQUIRY:
                tucb.scsitu.flags |= B_READ;
                tucb.scsitu.scsi_cmd_blk[4] = 255;
                tucb.scsitu.data_length = 255;
                tucb.scsitu.data_buffer = tu_buffer;
                break;
        case SCATU_READ_EXTENDED:
                tucb.scsitu.flags |= B_READ;
                blocks = (maxblock - curblock) + 1;
                if (blocks > 128)
                        blocks = 128;
                tucb.scsitu.data_length = blocks * 512;
                tucb.scsitu.scsi_cmd_blk[2] = (uchar) (curblock >> 24);
                tucb.scsitu.scsi_cmd_blk[3] = (uchar) (curblock >> 16);
                tucb.scsitu.scsi_cmd_blk[4] = (uchar) (curblock >> 8);
                tucb.scsitu.scsi_cmd_blk[5] = (uchar) (curblock);
                tucb.scsitu.scsi_cmd_blk[7] = (uchar) (blocks >> 8);
                tucb.scsitu.scsi_cmd_blk[8] = (uchar) (blocks);
                tucb.scsitu.data_buffer = tu_buffer;
                break;
        case SCATU_MODE_SELECT:
                tucb.scsitu.cmd_timeout = 30;
                tucb.scsitu.flags |= B_WRITE;
                /* MODE SELECT HEADER */
                tu_buffer[0] = 0x00;    /* Rsvd=0         */
                tu_buffer[1] = 0x00;    /* Medium Type=0  */
                tu_buffer[2] = 0x00;    /* Rsvd=0         */
                tu_buffer[3] = 8;    /* Blk Desc lgth - 0 = none
                                         * sent */
		/* BLOCK DESCRIPTOR */
		tu_buffer[4] = 0x00;
		tu_buffer[5] = 0x00;
		tu_buffer[6] = 0x00;
		tu_buffer[7] = 0x00;
		tu_buffer[8] = 0x00;
		tu_buffer[9] = 0x00;
		tu_buffer[10] = 0x02;
		tu_buffer[11] = 0x00;

                /* PAGE DESCRIPTORS  */
                /* **************** Page 0 **************** */
                tu_buffer[12] = 0x00;    /* byte 0 - Pg Code */
                tu_buffer[13] = 0x01;    /* byte 1 - Pg Lgth */
                tu_buffer[14] = 0x00;    /* QPE set to 0   */
                /* **************** Page 1 **************** */
                tu_buffer[15] = 0x01;    /* byte 0 - Pg Code */
                tu_buffer[16] = 0x01;    /* byte 1 - Pg Lgth */
		if(certify_after)
                	tu_buffer[17] = 0x60;    /* byte 2         */
		else
			tu_buffer[17] = 0x20;
                /* *************** Page 7 ***************** */
                tu_buffer[18] = 0x07;   /* Pg Code        */
                tu_buffer[19] = 0x01;   /* Pg Lgth        */
                tu_buffer[20] = 0x01;   /* byte 6         */
                /* *************** Page 8 ***************** */
                tu_buffer[21] = 0x08;   /* Pg Code        */
                tu_buffer[22] = 0x01;   /* Pg Lgth        */
                tu_buffer[23] = 0x01;   /* RCD=1          */
                tucb.scsitu.data_length = 24;
                tucb.scsitu.data_buffer = tu_buffer;
                tucb.scsitu.scsi_cmd_blk[4] = 24;

                break;

        case SCATU_FORMAT_UNIT:
                tucb.scsitu.flags |= B_WRITE;
                tucb.scsitu.cmd_timeout = 5400;
		tucb.scsitu.scsi_cmd_blk[1] |= 0x10;
                tucb.scsitu.scsi_cmd_blk[2] = 0x00;
                tucb.scsitu.scsi_cmd_blk[3] = 0x00;
                tucb.scsitu.scsi_cmd_blk[4] = 0x00;
                tucb.scsitu.scsi_cmd_blk[5] = 0x00;
                tu_buffer[0] = 0x00;
                tu_buffer[1] = 0x02;
                tu_buffer[2] = 0x00;
                tu_buffer[3] = 0x00;
                tucb.scsitu.data_length = 4;
                tucb.scsitu.data_buffer = tu_buffer;
                break;
	case SCATU_REASSIGN_BLOCK:
		tucb.scsitu.cmd_timeout = 120;
		tucb.scsitu.flags |= B_WRITE;
		tucb.scsitu.data_length = 8;
		tucb.scsitu.data_buffer = tu_buffer;
		tu_buffer[0] = 0;
		tu_buffer[1] = 0;
		tu_buffer[2] = 0;
		tu_buffer[3] = 4;
		tu_buffer[4] = ((unsigned) error_block >>24) & 0x00ff;
		tu_buffer[5] = ((unsigned) error_block >>16) & 0x00ff;
		tu_buffer[6] = ((unsigned) error_block >> 8) & 0x00ff;
		tu_buffer[7] = ((unsigned) error_block     ) & 0x00ff;
		break;
        default:
                /* error */
                return (SCATU_BAD_PARAMETER);
        }


        rc = exectu(fdescr, &tucb);


#ifdef DEBUG_SRWFOR
	diag_asl_msg("rc from TU # %x = %d\n",tnum,rc);
#endif

        return (rc);
}                               /* end of tu_test() */


/*
 * NAME: void clean_up
 *
 * FUNCTION: Perform what is necessary for an orderly exit from this DA.
 *
 * NOTES: Will release a reservation on the tape being tested if it
 *      is currently reserved.
 *
 * RETURNS: DOES NOT RETURN.
 */


void
clean_up()
{
        extern int      i_reserved_it;
        extern int      fdes;   /* file descriptor of device     */
	int rc;

        if( is_working ) {      /* drive is currently formatting */
                /* we should stop it */
                if( fdes > 0 )
                        close(fdes);

                /* Reopen device, sending SCSI device reset */
                fdes = openx(dd_name, O_RDWR, NULL, SC_FORCED_OPEN);
        } else if (i_reserved_it) {
                /* at least try to release the device */
                (void) tu_test(fdes, SCATU_RELEASE_UNIT);
        }

        if( fdes > 0 )
                close(fdes);

	if( (progmode == UFORMAT) && pvid_destroyed)
		/* Cleanup data base */
		clear_pvid(devname);
        /*
         * config stuff 
         */
	if (device_cfg_state != -1) {
		rc = initial_state(device_cfg_state,devname);
		if (rc != 0) {
			disp_menu(MENU_TERMINATED);
		}
	}

	if (parent_cfg_state != -1) {
		rc = initial_state(parent_cfg_state,parent_name);
		if (rc != 0) {
			disp_menu(MENU_TERMINATED);
		}
	}

	if (grandparent_cfg_state != -1) {
		rc = initial_state(grandparent_cfg_state,grandparent_name);
		if (rc != 0) {
			disp_menu(MENU_TERMINATED);
		}
	}

        term_dgodm();
        diag_asl_quit();
        DA_EXIT();
}

/*  */
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
	clean_up();
}
/*  */

/*
 * NAME: process_sense_data
 *
 * FUNCTION: Determine what kind of error based on sense data.
 *
 * RETURNS: Next step to do in command_certify.
 */

int
process_sense_data(sense_key, sense_code)
int	sense_key;
int	sense_code;
{
	int	return_code;

	return_code = CERTIFY_STEP_6;

	switch(sense_key){
	case 1: /* Recovered Errors */
		++drive_info.rec_data_errors;
		break;
	case 2: /* Not ready */
		switch(sense_code){
		case 0x3100:
		case 0x3101:
		case 0x4080:
		case 0x4081:
		case 0x8200:
		case 0x4c00:
			++drive_info.unrec_equ_check_errors;
			break;
		case 0x0400:
		case 0x0401:
		case 0x0402:
		case 0x2200:
			++drive_info.rec_equ_check_errors;
			break;
		default:
			; /* Ignore any other unknown sense data */
		} /* Switch of sense code for sense key of 2 */
		break;

	case 3: /* Medium error */
		reassign_block=1;
		switch(sense_code){
		case 0x1100:
		case 0x1401:
		case 0x1600:
		case 0x1900:
			++drive_info.unrec_data_errors;
			break;
		default:
			;
		}
		break;
  
	case 4: /* Hardware error */
		switch(sense_code){
		case 0x1b00:
		case 0x0800:
		case 0x0801:
		case 0x0802:
		case 0x4400:
		case 0x4700:
			++drive_info.unrec_equ_check_errors;
			return_code = CERTIFY_STEP_5;
			break;
		case 0x0900:
		case 0x1104:
		case 0x1500:
		case 0x1502:
		case 0x3200:
		case 0x8300:
		case 0x8400:
			++drive_info.unrec_data_errors;
			break;
		default:
			;
		}
		break;

	case 5: /* Illegal Request */
		++drive_info.unrec_equ_check_errors;
		return_code = CERTIFY_STEP_5;
		break;
	case 0x0b:
		++drive_info.unrec_equ_check_errors;
		return_code = CERTIFY_STEP_5;
		break;
	default:
		;
	}
	return(return_code);
}
