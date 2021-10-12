static char sccsid[] = "@(#)82	1.14  src/bos/diag/da/fd/asn_fd_fru.c, dafd, bos411, 9428A410j 12/17/92 10:52:59";
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: assign_fd_frub
 *		error_check
 *		exit_with_frub
 *		exit_with_unknown
 *		get_sn_rcode
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


#include "diag/da.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/dcda_msg.h"

#include "fd_set.h"
#include "fd_frus.h"            /* fru data base                        */
#include "fdatuname.h"          /* define tu test names                 */
#include "fda_rc.h"             /* tu return codes                      */

/* ....................... */
/* Confidence Percentages  */
/* ....................... */

#define CONF_100     100
#define CONF_95       95
#define CONF_80       80
#define CONF_75       75
#define CONF_20       20
#define CONF_5         5

#define DFD_FNAME_REDRIVE "Redrive Card"

extern void chk_asl_return();
extern struct fd_drive *fd;
extern struct tm_input da_input;
extern int dev_fdes;

/*
 * NAME: assign_fd_frub(return_code , test_number)
 *
 * FUNCTION: IF the return code for the test is a value other than NO_ERROR (0)
 *           the fru bucket structure with the the following data:
 *
 *     Field Replacment Unit type ....... FRUB1              (default)
 *                 service number ....... frub[x].sn
 *                    reason code ....... frub[x].rcode
 *                 reason message ....... frub[x].rmsg
 *
 *          confidence percentage ....... frub[x].frus[y].conf
 *                       fru name ....... frub[x].frus[y].fname
 *                       fru loc  ........frub[x].frus[y].floc
 *               fru text message ....... frub[x].frus[y].fmsg
 *                       fru flag ....... frub[x].frus[y].fru_flag
 *              fru exempt status ....... frub[x].frus[y].fru_exempt
 *
 *              ( SEE: da.h for fru bucket structure information. )
 *              ( SEE: fd_frus.h for TUDSKT fru bucket structures )
 *
 * NOTES:       The user is aske DRIVE_LOCATION_query() if the diskette drive
 *        is external or external before a fru is assigned. (5.25 Inch Drives)
 *             Then then return code from the Test Unit is checked for a value
 *        that indicates a software error. (error_check() )
 *             IF the return codes does indicate a software error, the process
 *        will then exit thru exit_with_unknow().
 *           Otherwise, the fru bucket is populated, and the process is
 *        exited thru exit_with_frub(fru_type).
 *        (   SEE:  exit_with_frub( )     )
 *        (   SEE:  exit_with_unknown()   )
 *
 * RETURNS: NONE
*/

#define NO_REDRIVE 0
#define REDRIVE 1


void assign_fd_frub(rc, tstnum )
{
        void error_check();
        void exit_with_frub();
        void get_sn_rcode();
        void exit_with_unknown();
        int ipl_mod;
        int cuat_mod;
		int redrive_flag = NO_REDRIVE;


        if (da_input.console == CONSOLE_TRUE)
                chk_asl_return(diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,NULL,NULL));

        if (rc == 4 || rc == 7)
                DA_SETRC_MORE(DA_MORE_CONT);

        if(fd->size == FIVE25)
        {
                rc = getdavar(da_input.dname,"EXORIN",
                              DIAG_INT,&fd->exORin);
                if (rc == AIX_ERROR || fd->exORin == AIX_ERROR)
                {
                        if (da_input.console == CONSOLE_TRUE)
                        {
                                rc =  DRIVE_LOCATION_query();
                                if(rc == 1)
                                        fd->exORin = EXTERNAL_DISKETTE_DRIVE;
                                else
                                        fd->exORin = INTERNAL_DISKETTE_DRIVE;
                        }
                        else
                                fd->exORin = EXTERNAL_DISKETTE_DRIVE;

                        rc = putdavar(da_input.dname,"EXORIN",
                                      DIAG_INT,&fd->exORin);
                }
        }
        else
                fd->exORin = INTERNAL_DISKETTE_DRIVE; /* 3.5 Inch Default */

        /*
         * Find out if this is a system with a redriver card
         */
        ipl_mod = get_cpu_model(&cuat_mod);
        ipl_mod = ipl_mod & 0xff;

		/* 
  		 * check to see if this is a deskside with a redriver card
		 */
		if(ipl_mod == 0x30 || /* 520 */ 
		   ipl_mod == 0x34 || /* 520h */
		   ipl_mod == 0x10 || /* 530 or 730(?)*/
		   ipl_mod == 0x14 || /* 540 */
		   ipl_mod == 0x18 || /* 530h */
		   ipl_mod == 0x1C || /* 550 */
		   ipl_mod == 0x5C )  /* 550h or 560 */
		{
			redrive_flag = REDRIVE;
		}
		else
			redrive_flag = NO_REDRIVE;


        switch(tstnum)
        {
        case ADAPTER_TEST :
                frub[ 0 ].frus[ 0 ].conf = CONF_100;
                frub[ 0 ].sn = 0x828;
                frub[ 0 ].rcode = 0x501;
                if(fd->size == THREE5)
                        frub[ 0 ].rmsg = DFD144_DISKETTE_ADAPTER_ERR;
                if(fd->size == FIVE25)
                        frub[ 0 ].rmsg = DFD12_DISKETTE_ADAPTER_ERR;
                exit_with_frub( 0 );
                break;
        case SELECT_TEST:
                frub[ 1 ].frus[ 0 ].conf = CONF_80;
                frub[ 1 ].frus[ 1 ].conf = CONF_20;
                get_sn_rcode(tstnum, 1);
                if(fd->size == THREE5)
                        frub[ 1 ].rmsg = DFD_SELECT_ERR;
                if(fd->size == FIVE25)
                        frub[ 1 ].rmsg = DFD_SELECT_ERR;
                exit_with_frub( 1 );
                break;
        case DESELECT_TEST:
                frub[ 1 ].frus[ 0 ].conf = CONF_80;
                frub[ 1 ].frus[ 1 ].conf = CONF_20;
                get_sn_rcode(tstnum, 1);
                if(fd->size == THREE5)
                        frub[ 1 ].rmsg = DFD_SELECT_ERR;
                if(fd->size == FIVE25)
                        frub[ 1 ].rmsg = DFD_SELECT_ERR;
                exit_with_frub( 1 );
                break;
        case RECALIB_TEST:
        case DISK_CHANGE_TEST:
        case DISK_WR_PROT_TEST:
        case INDEX_TEST:
        case STEP_TEST:

			if(redrive_flag == REDRIVE)
			{
                frub[ 2 ].frus[ 0 ].conf = CONF_75;
                frub[ 2 ].frus[ 1 ].conf = CONF_20;
                frub[ 2 ].frus[ 2 ].conf = CONF_5;
                get_sn_rcode(tstnum, 2);
                if(fd->size ==FIVE25)
                {
                        strncpy( frub[ 2 ].frus[ 2 ].fname,
                            DFD_FNAME_REDRIVE ,NAMESIZE);
                        frub[ 2 ].frus[2].fmsg = DFD12_REDRIVE;
                        frub[ 2 ].frus[2].fru_flag =NOT_IN_DB;
                        switch(tstnum)
                        {
                        case STEP_TEST:
                                frub[ 2 ].rmsg = DFD_STEP_ERR;
                                break;
                        case INDEX_TEST:
                                frub[ 2 ].rmsg = DFD_INDEX_ERR;
                                break;
                        case DISK_WR_PROT_TEST:
                                frub[ 2 ].rmsg = DFD_WRITE_PROT_ERR;
                                break;
                        case DISK_CHANGE_TEST:
                                frub[ 2 ].rmsg = DFD_DISK_CHANGE_ERR;
                                break;
                        case RECALIB_TEST:
                                frub[ 2 ].rmsg = DFD_RECALIBRATE_ERR;
                                break;
                        }
                }
                if(fd->size == THREE5)
                {
                        strncpy( frub[ 2 ].frus[ 2 ].fname,
                            DFD_FNAME_REDRIVE ,NAMESIZE);
                        frub[ 2 ].frus[ 2].fmsg  = DFD144_REDRIVE;
                        frub[ 2 ].frus[2].fru_flag =NOT_IN_DB;
                        switch(tstnum)
                        {
                        case STEP_TEST:
                                frub[ 2 ].rmsg = DFD_STEP_ERR;
                                break;
                        case INDEX_TEST:
                                frub[ 2 ].rmsg = DFD_INDEX_ERR;
                                break;
                        case DISK_WR_PROT_TEST:
                                frub[ 2 ].rmsg = DFD_WRITE_PROT_ERR;
                                break;
                        case DISK_CHANGE_TEST:
                                frub[ 2 ].rmsg = DFD_DISK_CHANGE_ERR;
                                break;
                        case RECALIB_TEST:
                                frub[ 2 ].rmsg = DFD_RECALIBRATE_ERR;
                                break;
                        }

                }
                exit_with_frub( 2 );
			}
			else
			{
                frub[ 1 ].frus[ 0 ].conf = CONF_80;
                frub[ 1 ].frus[ 1 ].conf = CONF_20;
                get_sn_rcode(tstnum, 1);

                switch(tstnum)
                {
                    case STEP_TEST:
                            frub[ 1 ].rmsg = DFD_STEP_ERR;
                            break;
                    case INDEX_TEST:
                            frub[ 1 ].rmsg = DFD_INDEX_ERR;
                            break;
                    case DISK_WR_PROT_TEST:
                            frub[ 1 ].rmsg = DFD_WRITE_PROT_ERR;
                            break;
                    case DISK_CHANGE_TEST:
                            frub[ 1 ].rmsg = DFD_DISK_CHANGE_ERR;
                            break;
                    case RECALIB_TEST:
                            frub[ 1 ].rmsg = DFD_RECALIBRATE_ERR;
                            break;
                    }
                }

                exit_with_frub( 1 );

            	break;
        case READ_TEST:
                frub[ 0].frus[ 0 ].conf = CONF_100;
                get_sn_rcode(tstnum, 0 );
                if(fd->size ==THREE5)
                        frub[ 0 ].rmsg = DFD_READ_ERR;
                if(fd->size == FIVE25)
                        frub[ 0 ].rmsg = DFD_READ_ERR;
                exit_with_frub( 0 );
                break;
        case WRITE_TEST:
                frub[ 1 ].frus[ 0 ].conf = CONF_80;
                frub[ 1 ].frus[ 1 ].conf = CONF_20;
                get_sn_rcode(tstnum, 1);
                if(fd->size ==THREE5)
                        frub[ 1 ].rmsg = DFD_WRITE_ERR;
                if(fd->size == FIVE25)
                        frub[ 1 ].rmsg = DFD_WRITE_ERR;
                exit_with_frub( 1 );
                break;
        case WR_READ_CMP_TEST:
                frub[ 1 ].frus[ 0 ].conf = CONF_95;
                frub[ 1 ].frus[ 1 ].conf = CONF_5;
                get_sn_rcode(tstnum, 1);
                if(fd->size ==THREE5)
                        frub[ 1 ].rmsg = DFD_DATA_COMPARE_ERR;
                if(fd->size == FIVE25)
                        frub[ 1 ].rmsg = DFD_DATA_COMPARE_ERR;
                exit_with_frub( 1 );
                break;
        case LOW_DENSITY_TEST_1MB:
        case LOW_DENSITY_TEST_2MB:
        case SPEED_TEST:
        case HEAD_SETTLE_TEST:
                frub[ 0].frus[ 0 ].conf = CONF_100;
                get_sn_rcode(tstnum, 0);
                if(fd->size == FIVE25)
                {
                        switch(tstnum)
                        {
                        case LOW_DENSITY_TEST_1MB:
                        case LOW_DENSITY_TEST_2MB:
                                frub[ 0 ].rmsg = DFD_DENSITY_FORMAT_ERR;
                                break;
                        case SPEED_TEST:
                                frub[ 0 ].rmsg = DFD_MOTOR_SPEED_ERR;
                                break;
                        case HEAD_SETTLE_TEST:
                                frub[ 0 ].rmsg = DFD_HEAD_SETTLE_ERR;
                                break;
                        }
                }
                if(fd->size == THREE5 )
                {
                        switch(tstnum)
                        {
                        case LOW_DENSITY_TEST_1MB:
                        case LOW_DENSITY_TEST_2MB:
                                frub[ 0 ].rmsg = DFD_DENSITY_FORMAT_ERR;
                                break;
                        case SPEED_TEST:
                                frub[ 0 ].rmsg = DFD_MOTOR_SPEED_ERR;
                                break;
                        case HEAD_SETTLE_TEST:
                                frub[ 0 ].rmsg = DFD_HEAD_SETTLE_ERR;
                                break;
                        }
                }
                exit_with_frub( 0 );
                break;
        default:
                exit_with_unknown();
                break;
        }
} /* end assign_frub */


/*
 * NAME: error_check(int test_unit_return_code)
 *
 * FUNCTION: check for one of the following return codes:
 *
 *      UNDEFINDED_DEVICE_DRIVER_ERROR.... ( 255 )....
 *      INVALID_TEST_UNIT_PASSED ......... ( 254 )....
 *      AIX_ERROR ........................ (  -1 )....
 *
 *      IF one of these return codes was passed then exit_with_unknown_error()
 *
 * RETURNS: NONE
*/

void error_check(rc)
{
        switch(rc)
        {
        case UNDEFINED_DEVICE_DRIVER_ERROR:
        case INVALID_TEST_UNIT_PASSED:
        case AIX_ERROR:
                exit_with_unknown();
                break;
        default:
                break;
        }
}

/*
 * NAME: exit_with_frub( fru_number)
 *
 * FUNCTION: Return to Diagnostic Function Controller with the a fru bucket
 *           that is populated with values that correspond to the Test Unit
 *           and return code.
 *
 * RETURNS: NONE
*/

void exit_with_frub(fru_number)
{
        int sn;
        int rcode;
        int rmsg;



        if(fru_number == 999)
        {
                fru_number = 2;
                get_sn_rcode( 28 , 2 ) ; /* set the frub[x].sn   */
                frub[fru_number].rmsg =  DFD_ELA;
                frub[ fru_number ].frus[ 0 ].conf = CONF_75;
                frub[ fru_number ].frus[ 1 ].conf = CONF_20;
                frub[ fru_number ].frus[ 2 ].conf = CONF_5;
                strncpy( frub[ 2 ].frus[ 2 ].fname,
                         DFD_FNAME_REDRIVE ,NAMESIZE);
                frub[ 2 ].frus[2].fmsg = DFD12_REDRIVE;
                frub[ 2 ].frus[2].fru_flag =NOT_IN_DB;
        }

        sn    = frub[fru_number].sn;
        rcode = frub[fru_number].rcode;
        rmsg  = frub[fru_number].rmsg;
        strcpy(frub[fru_number].dname,da_input.dname);
        insert_frub(&da_input,&frub[  fru_number  ]);
        frub[fru_number].sn    = sn   ;
        frub[fru_number].rcode = rcode;
        frub[fru_number].rmsg  = rmsg ;
        addfrub(&frub[  fru_number  ]);

        DA_SETRC_STATUS(DA_STATUS_BAD);
        DA_SETRC_TESTS(DA_TEST_FULL);
        clean_up();
}


/*
 * NAME: exit_with_unkown()
 *
 * FUNCTION: return to Diagnostic Function Controller with unknown error
 *
 * RETURNS: NONE
*/

void exit_with_unknown()
{
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_SETRC_TESTS(DA_TEST_FULL);
        clean_up();
}


/*
 * NAME: get_sn_rcode(test_unit_number , type_of_fru_bucket );
 *
 * FUNCTION:    Populates the frub[frutype].rcode with a value formulated
 *            as shown below.  Also populates the frub[frutype].sn with a value
 *            that is spec'ed in the CAS.
 *
 *        ..........+...............+..............+.................
 * NOTE:  Location  |   Drive Size  |   frub[x].sn |  frub[x].rcode |
 *        ..........|...............| .............| ...............|
 *        EXTERNAL  |   3.5 Inch    |   0x935      |  0x4xx         |
 *        ..........|...............|..............|................|
 *        INTERNAL  |   3.5 Inch    |   0x935      |  0x1xx         |
 *        ..........|...............| .............| ...............|
 *        EXTERNAL  |   5.25 Inch   |   0x936      |  0x3xx         |
 *        ..........|...............|..............|................|
 *        INTERNAL  |   5.25 Inch   |   0x936      |  0x2xx         |
 *        ..........|...............|..............|................|
 *
 *
 * RETURNS: NONE
*/


void get_sn_rcode( tstnum, frutype )
{
        if(tstnum >9)
          tstnum+=6;
		/* 
		 * subtract 1 from TU number so that SRN's which were
		 * originally written match up with new TU numbers for 4MB
		 */
		if(tstnum >= 12)
			tstnum --;
        if(fd->size == THREE5)
        {
                frub [ frutype ].sn = 0x935;
                frub [ frutype ].rcode = 0x100 +tstnum;
        }
        if(fd->size == FIVE25)
        {
                frub [ frutype ].sn = 0x936;
                if(fd->exORin == INTERNAL_DISKETTE_DRIVE)
                        frub [ frutype ].rcode = 0x200 + tstnum;
                else
                        frub [ frutype ].rcode  = 0x300 +tstnum;
        }

}




