static char sccsid[] = "@(#)84  1.11  src/bos/diag/da/fd/do_fd_tu.c, dafd, bos411, 9428A410j 12/17/92 10:57:49";
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: MALLOC
 *		run_test_patterns
 *		run_test_with_retry
 *		run_tu_test
 *		test_cylinders
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



/************************************************************************/
/*                                                                      */
/*      Diagnostic Application for Diskette Drives                      */
/*      All Test Units have to run in Full Test Mode                    */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <fcntl.h>

#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/fd.h>

#include "../../tu/fd/fdatu.h"

#include "diag/tm_input.h"      /* faking the ODM                       */
#include "diag/diago.h"         /* ASL functions                        */

#include "fd_set.h"
#include "fd_tstp.h"            /* test patterns */
#include "fda_rc.h"             /* define tu return codes               */


#include "fdatuname.h"          /* define TU test names                 */
#include "dfd_msg.h"            /* message catalog numbers              */


#define MALLOC(x)     ((x*)malloc(sizeof(x)))


extern int dev_fdes;
extern struct fd_drive *fd;
extern nl_catd  catd;
struct diskette_tucb_t *tucb_ptr;
extern int density_check_status;

int test_pattern[] = {
        0,
        TEST_PATTERN1,
        TEST_PATTERN2,
        TEST_PATTERN3,
        0};


/*
 * NAME: void run_test_with_retry(tucb_ptr )
 *
 * FUNCTION: During some tests a error may occure which does NOT mean that the
 *           diskette drive is faulty. If these types of errors occure then the
 *           test unit is done again. If another error occures then a fru bucket
 *           is assigned.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
*/

void run_test_with_retry(tucb_ptr )
        struct diskette_tucb_t *tucb_ptr;
{
        int rc,tn;

        tn = tucb_ptr->tucb.tu;      /* save the test number   */
        tucb_ptr->tucb.loop = 1;
        rc = exectu(dev_fdes,tucb_ptr);

        if (rc != NO_ERROR && tn == LOW_DENSITY_TEST_1MB)
        {
                density_check_status = rc;
                return;
        }

        if (rc != NO_ERROR)
        {
                tucb_ptr->tucb.tu = ENABLE_RETRY;
            	tucb_ptr->tucb.loop = 1;
                rc = exectu(dev_fdes, tucb_ptr );
                if(rc != AIX_ERROR)
                {
                		tucb_ptr->tucb.loop = 1;
                        tucb_ptr->tucb.tu = tn;
                        rc = exectu(dev_fdes, tucb_ptr );
                }
        }
        if(rc != NO_ERROR)
                assign_fd_frub(rc,tn);
}

/*
 * NAME: void run_test_patterns(tucb_ptr )
 *
 * FUNCTION: During write & compare tests the ip2 element of tucb_ptr is
 *           loaded with a set of known characters.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
*/


void run_test_patterns(  cyl1, cyl2,tucb_ptr)
int cyl1,cyl2;
struct diskette_tucb_t *tucb_ptr;
{
        int i = 1 ;
        tucb_ptr->ip1 = cyl1;
        while(test_pattern[i] != 0 )
        {
                tucb_ptr->ip2 = test_pattern[i];
                run_test_with_retry( tucb_ptr );
                ++i;
        };
        i = 1;
        tucb_ptr->ip1 = cyl2;
        while(test_pattern[i] != 0 )
        {
                tucb_ptr->ip2 = test_pattern[i];
                run_test_with_retry( tucb_ptr );
                ++i;
        };

}



/*
 * NAME: void test_cylinders(set1, set2, tucb_ptr )
 *
 * FUNCTION: Test the cylinders(set1 & set2) of the diskette
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
*/



void test_cylinders( set1, set2, tucb_ptr)
int set1,set2;
struct diskette_tucb_t *tucb_ptr;
{
        tucb_ptr->ip1 = set1;

        run_test_with_retry( tucb_ptr);

        tucb_ptr->ip1 = set2;

        run_test_with_retry( tucb_ptr);
}


/*
 * NAME: void run_tu_test(tn)
 *
 * FUNCTION: Run the Test Unit passed. If an error is found then
 *           assign a fru bucket.
 *
 * EXECUTION ENVIRONMENT: called from do_fd_tests();
 *                        SEE: do_fd_test.c
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
*/

void run_tu_test(tn)
int tn ;
{
        int rc;
        tucb_ptr = MALLOC( struct diskette_tucb_t);

        tucb_ptr->tucb.tu = tn;
        if ((tn >= INDEX_TEST) && (tn <=  WR_READ_CMP_TEST))
                tucb_ptr->tucb.loop = 10;
        else    tucb_ptr->tucb.loop = 1;

        if(tn == HEAD_SETTLE_TEST)
                tucb_ptr->ip1 = -1 ;   /* negative number specs dirve default */

        tucb_ptr->tucb.mfg = tucb_ptr->tucb.r1 = tucb_ptr->tucb.r2 = 0;

        switch (tn) {
        case LOW_DENSITY_TEST_1MB:
        case LOW_DENSITY_TEST_2MB:
                tucb_ptr->ip1 = 0;
                run_test_with_retry( tucb_ptr );
                break;
        case INDEX_TEST:
                test_cylinders( 0,  1, tucb_ptr);
                test_cylinders( 38, 39, tucb_ptr);
                if(fd->disk_density == HIGH_DENSITY)
                        test_cylinders( 78, 79,tucb_ptr);
                break;
        case WRITE_TEST:
        case READ_TEST:

                tucb_ptr->ip2 = 1;    /* Head Number (base 0)   */
                tucb_ptr->ip3 = 1;    /* Sector Nubmer (base 1) */

                if(tn == WRITE_TEST)
                        tucb_ptr->ip4 = TEST_PATTERN4;

               /* 0/1 , 38/39 , and 78,79 are cylinder numbers     */

               for(tucb_ptr->ip2 = 0;tucb_ptr->ip2 <=1;++ tucb_ptr->ip2)
                {
                     test_cylinders( 0  , 1  , tucb_ptr);
                     test_cylinders( 38 , 39 , tucb_ptr);
                     if(fd->disk_density == HIGH_DENSITY)
                           test_cylinders( 78 , 79 , tucb_ptr);
                }
                break;
        case STEP_TEST:
                /* 0/1 , 38/39 , and 78,79 are cylinder numbers     */
                tucb_ptr->ip2 = 0;
                test_cylinders( 0  , 1  , tucb_ptr);
                test_cylinders( 38 , 39 , tucb_ptr);
                if(fd->disk_density == HIGH_DENSITY)
                        test_cylinders( 78 , 79 , tucb_ptr);
                break;
        case WR_READ_CMP_TEST:
                tucb_ptr->ip3 = 1;
                run_test_patterns(  0 ,  1 , tucb_ptr);
                run_test_patterns( 38 , 39 , tucb_ptr);
                if ( fd->disk_density == HIGH_DENSITY)
                        run_test_patterns( 78 , 79 , tucb_ptr);
                break;
        case DISK_WR_PROT_TEST:
                rc = exectu(dev_fdes,tucb_ptr);
                if(rc == DISK_WAS_WRITE_PROTECTED)
                        break;
                else
                {
                        if(rc == 0  )
                                assign_fd_frub(DISK_WAS_NOT_WRITE_PROTECTED,tn) ;
                        else
                                assign_fd_frub(rc , tn) ;
                }
                break;
        case DISK_CHANGE_TEST :
        case SPEED_TEST:
        case HEAD_SETTLE_TEST:
        case DESELECT_TEST:
        case SELECT_TEST:
        case ADAPTER_TEST:
        case RECALIB_TEST:
        case DISABLE_RETRY:
        case ENABLE_RETRY:
                rc = exectu(dev_fdes,tucb_ptr);
                if(rc != NO_ERROR )
                        assign_fd_frub(rc,tn) ;
                break;
        default:
                rc = AIX_ERROR;
                assign_fd_frub(rc, 0);
                break;

        }
        free(tucb_ptr);

}


