static char sccsid[] = "@(#)17  1.7  src/bos/diag/util/ufd/fdsatest.c, dsaufd, bos411, 9428A410j 3/17/92 09:53:38";
/*
 * COMPONENT_NAME: dsaufd
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/fd.h>
#include <unistd.h>
#include <fcntl.h>
#include "diag/diago.h"
#include "fd_sa.h"              /* FD Service Aids Devines              */
#include "ufd_msg.h"
#include "../../tu/fd/fdatu.h"

#ifndef NO_ERROR
#define NO_ERROR 0
#endif

#ifndef SECTOR_SIZE
#define SECTOR_SIZE  512
#endif

#define CYLINDER_FAILURE   -1

/* NAME: void do_fdsa_tests(void)
*
* FUNCTION: Do the required verification test on the diskette.
*
* EXECUTION ENVIRONMENT: Called from main();
*
* RETURNS:
*/

extern void clean_up();
extern void check_ioctl_return();
extern void chk_asl_return();
extern void DFD_RESULT_message();
extern void DFD_ERROR_message();
extern int DFD_DEFAULT_query();

void check_diskette_status();
void do_fdsa_tests();
void check_diskette_status();
int cylinder_by_cylinder();
int sector_by_sector ();

extern long dev_fdes;             /* from fdsamain.c                     */
extern int diskette_inserted;
extern int drive_4MB;
extern struct diskette_tucb_t *test_tucb_ptr;


union {
        char datc[SECTOR_SIZE];
        unsigned long data[SECTOR_SIZE/4];
}datacl;

union {
        char cyl_datc[SECTOR_SIZE * 2 * LAST_SECTOR];
        unsigned long cyl_data[( SECTOR_SIZE * 2 * LAST_SECTOR)/4];
}cyl_datacl;



/*
 * NAME: int do_fdsa_tests();
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:)
 *
 *
 * RETURNS:
*/


/* globals */

int error_found;

void do_fdsa_tests(diskette_index)
        int diskette_index;
{
        int rc;                       /* return code          */
        int nh;                       /* number of heads      */
        int spt;                      /* sectors per track    */
        int last_track;               /* last cylinder        */
        int continue_flag = TRUE;
        int bad_sector_cnt;
        int total_sectors , good_sectors ;

        int msg_rc;                    /* message return code */
        int sector_cnt = 0;
        int cylinder_num;
        char cbuff[80];
        char *cp;
        cp = &cbuff[0];

        error_found = FALSE;

        check_ioctl_return(ioctl(dev_fdes,FDIOCRETRY));   /*enable retry */

        nh = 2;                 /* number of heads */

        last_track  = 80;       /* default */

        switch( diskette_index ) {
                case 0:
			/* 3.5 inch 4MB */
                        spt = 36;
                        break;
                case 1:
			/* 3.5 inch 2MB */
                        spt = 18;
                        break;
                case 2:
			/* 3.5 inch 1MB */
                        spt = 9;
                        break;
                case 3:
			/* 5.25 inch 1.2MB */
                        spt = 15;
                        break;
                case 4:
			/* 5.25 inch 360KB */
                        last_track  = 40;
                        spt = 9;
                        break;
        } /* end switch */

        total_sectors  = nh * last_track * spt ;
        continue_flag  = TRUE;
        bad_sector_cnt = 0;
        cylinder_num   = 0;

        rc = lseek( dev_fdes ,  0 , SEEK_SET);

        if(rc == -1)

        /* .................................................... */
        /* Couldn't find the track ........                     */
        /* .................................................... */

        {
                check_diskette_status( SECOND_STAGE  );
                DFD_ERROR_message(FORMAT_ERROR);
                rc = NO_ERROR;
        }
        else
        {
                do{
                        /* .............................................  */
                        /* Check each cylinder.  Cylinder is top & bottom */
                        /* track.                                         */
                        /* .............................................  */

                        rc = cylinder_by_cylinder( nh, spt,  cylinder_num);

                        /* .............................................  */
                        /* If the return is -1 then assume the whole      */
                        /* cylinder is bad. Increment the cylinder_num.   */
                        /* (continue_flag == FALSE in sector_by_sector()  */
                        /* .............................................. */

                        if(rc == CYLINDER_FAILURE)
                        {
                                bad_sector_cnt += (nh*spt);
                                sector_cnt     += (nh*spt);
                        }

                        /* ...............................................  */
                        /*    Add the return code (0 or the number of bad   */
                        /* sectors  to the bad_sector_cnt.                  */
                        /* ...............................................  */

                        else
                        {
                                bad_sector_cnt += rc ;
                                sector_cnt     += rc;
                        }

                        /* ............................................... */
                        /* A way to break out of the loop                  */
                        /* ............................................... */

                        chk_asl_return(diag_asl_read(
                            ASL_DIAG_LIST_CANCEL_EXIT_SC,
                            FALSE,cp));

                        ++cylinder_num ;   /* increment the cylinder index */

                        rc = lseek( dev_fdes , (cylinder_num * nh  * spt
                                               * SECTOR_SIZE ) ,
                                               SEEK_SET);
                        if(rc == -1)
                                check_diskette_status( SECOND_STAGE);

                        /* ................................................. */
                        /* Monitor the sector_cnt. If it is greater than the */
                        /* DEFAULT_FAILURES (10) then display the query      */
                        /* ................................................. */

                        if(sector_cnt >= DEFAULT_FAILURES)
                        {
                                msg_rc=DFD_DEFAULT_query(total_sectors,
                                                         cylinder_num * nh *spt,
                                                         bad_sector_cnt);

                                /* ......................................... */
                                /* If the user answers 'NO' then do NOT      */
                                /* continue with the verification of the     */
                                /* diskette.                                 */
                                /* ......................................... */

                                if(msg_rc != ANSWERED_YES)
                                        continue_flag = FALSE;
                                else
                                {
                                        /* ................................. */
                                        /* Reset sector_cnt and continue     */
                                        /* until 10 more bad sectors are     */
                                        /* found then query the user again.  */
                                        /* ................................. */

                                        error_found = TRUE;

                                        DFD_TESTING_message();

                                        sector_cnt = 0;
                                }
                        }
                }while(cylinder_num<=(last_track-1) && continue_flag==TRUE );
        }

        check_ioctl_return(ioctl(dev_fdes,FDIOCDSELDRV));


        if( continue_flag == TRUE )
        {
                good_sectors = total_sectors - bad_sector_cnt;
                DFD_RESULT_message( total_sectors,
                                    bad_sector_cnt,
                                    good_sectors);

        }

        /* ........................................................... */
        /* The user has stopped the verification but there were errors */
        /* ........................................................... */

        if( continue_flag == FALSE)
        {
                if( bad_sector_cnt != 0)
                        DFD_ERROR_message(ERROR_RECORD);
        }
}




int cylinder_by_cylinder( nh, spt , cylinder_num)
        int nh;
        int spt;
        int cylinder_num;
{
        int rc = NO_ERROR ;

        rc = read(dev_fdes, cyl_datacl.cyl_datc,SECTOR_SIZE * spt *nh );

        if(rc  != (SECTOR_SIZE *spt *nh) )
                rc = sector_by_sector( nh , spt , cylinder_num );
        else
                rc = 0;
        return(rc);
}



int sector_by_sector ( nh, spt, cylinder_num)
        int nh;
        int spt;
        int cylinder_num;
{
        int rc;
        int cylinder ;
        int sector;
        int bad_sectors = 0;
        int continue_flag  = TRUE;
        int logical_offset;
        char cbuff[80];
        char *cp;
        cp = &cbuff[0];

        sector = 0;

        logical_offset = cylinder_num * nh *spt * SECTOR_SIZE;

        do{
                rc = lseek( dev_fdes,( sector * SECTOR_SIZE)
                    + logical_offset  , SEEK_SET);

                if( rc == -1)
                {
                        check_diskette_status(SECOND_STAGE);
                        continue_flag = FALSE;
                }
                else
                {
                        rc = read(dev_fdes, datacl.datc , SECTOR_SIZE);
                        if(rc != SECTOR_SIZE )
                                ++bad_sectors;
                }
                ++sector;
                chk_asl_return(diag_asl_read( ASL_DIAG_LIST_CANCEL_EXIT_SC,
                            FALSE,cp));
        }while( ( sector <= (spt-1) ) &&  (continue_flag == TRUE) );

        if(continue_flag == FALSE)
                return(CYLINDER_FAILURE);
        else
                return(bad_sectors);

}


void check_diskette_status(stage)
        int stage;
{
        int rc;
        /* ...........................................................  */
        /* check to see if a diskette has been inserted into the drive  */
        /* This check (FIRST STAGE) is only done once when the diskette */
        /* is first inserted.                                           */
        /* ...........................................................  */

        switch(stage)
        {
        case FIRST_STAGE :
                check_ioctl_return( ioctl(dev_fdes,FDIOCRECAL      ));
                check_ioctl_return( ioctl(dev_fdes , FDIOCSELDRV   ));
                check_ioctl_return( ioctl(dev_fdes , FDIOCRECAL    ));
                check_ioctl_return( ioctl(dev_fdes , FDIOCSEEK  ,1 ));
                check_ioctl_return( ioctl(dev_fdes,FDIOCSTATUS,
                    &test_tucb_ptr->diskette_status));
                break;
        case SECOND_STAGE:
                check_ioctl_return( ioctl(dev_fdes,FDIOCSTATUS,
                    &test_tucb_ptr->diskette_status));
                break;
        default:
        case THIRD_STAGE:
                break;
        }
        if((test_tucb_ptr->diskette_status.dsktchng) & DISK_CHANGED)
        {
                check_ioctl_return(ioctl(dev_fdes,FDIOCDSELDRV));
                diskette_inserted = FALSE;
                DFD_ERROR_message(NO_DISK);
                clean_up();
       }

}


