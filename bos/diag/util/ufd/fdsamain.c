static char sccsid[] = "@(#)15  1.10  src/bos/diag/util/ufd/fdsamain.c, dsaufd, bos411, 9428A410j 7/13/93 15:22:28";
/*
 * COMPONENT_NAME: TUDSKT
 *
 * FUNCTIONS: main() ,  void  clean_up() ,   void check_ioctl_return(rc)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <locale.h>
#include <sys/cfgodm.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/fd.h>
#include <unistd.h>
#include <fcntl.h>
#include<errno.h>

#include "diag/diago.h"
#include "diag/diag_exit.h"

#include "../../tu/fd/fdatu.h"

#include "ufd_msg.h"
#include "fd_sa.h"


#ifndef NO_ERROR
#define NO_ERROR 0
#endif

nl_catd catd;
long dev_fdes = 0;

int selected_drive;           /* the selected drive                        */
int diskette_index;           /* the selected type / location of the drive */
int drive_4MB;                /* 1 if 4MB drive, 0 if not */
int drive_525;				  /* 1 if 5.25" drive, 0 if not */
int diskette_inserted;        /* diskette inserted flag                    */

struct diskette_tucb_t tucb_t , *test_tucb_ptr;

struct fdinfo diskette_fdinfo;

char *fd_drive[]= {
        RFD_0_35_HH ,
        RFD_0_35_H  ,
        RFD_0_35_L  ,
        RFD_1_525_H ,
        RFD_1_525_L
};

extern void DFD_DESCRIPTOR_message();
extern void DFD_INSERT_message();
extern void DFD_TESTING_message();
extern int  DFD_SELECT_menu();
extern int  DFD_4MB_SELECT_menu();
extern int do_fdsa_tests();
extern void check_diskette_status();
extern nl_catd diag_catopen(char *, int);

void clean_up();
void check_ioctl_return();

/*
 * NAME: void main( )
 *
 * FUNCTION: Initialize the diag_asl devaults.
 *           Display the Verify function message.
 *           Select the diskette type to be Verified.
 *           Do the tests.
 *           Exit.
 *
 * RETURNS: NONE
*/

void main( argc , argv , envp )
int argc;
char ** argv;
char ** envp;
{

        int rc , i ;
		extern int errno;
        struct fdinfo diskette_fdinfo;

        setlocale(LC_ALL,"");
        test_tucb_ptr = &tucb_t;

        diskette_inserted = FALSE;               /* Disketted NOT inserted   */

        if (init_dgodm() == -1)
        {
                DFD_ERROR_message(NOT_USABLE);
                clean_up();
        }

        if (diag_asl_init(ASL_INIT_DEFAULT) == -1)
        {
                DFD_ERROR_message(NOT_USABLE);
                clean_up();
        }

        catd = diag_catopen(MF_UFD,0);

        DFD_DESCRIPTOR_message();

        /* Find out if the drive is a 4MB drive. */

        drive_4MB = drive_type(0);
        if (drive_4MB == -1)
        {
                DFD_ERROR_message(NOT_USABLE);
                clean_up();
        }
		if(drive_4MB == -2)
		{
                DFD_ERROR_message(NO_DRIVES);
                clean_up();
		}

		drive_525 = drive_type(1);
        if (drive_525 == -1)
        {
                DFD_ERROR_message(NOT_USABLE);
                clean_up();
        }

        diskette_index = DFD_SELECT_menu();

        if ( drive_4MB )
                selected_drive = DRIVE_0;
        else
                if (diskette_index == 1 || diskette_index == 2)
                        selected_drive = DRIVE_0;
                else
                        selected_drive = DRIVE_1;

        dev_fdes = open(fd_drive[diskette_index], O_RDWR | O_NDELAY);

        if(dev_fdes <= 0)
        {
				if(errno == ENOENT || errno == ENODEV || errno == ENXIO )
					DFD_ERROR_message(NOT_ACCESSIBLE);
				else
	                DFD_ERROR_message(NOT_USABLE);

                clean_up();
        }

        DFD_INSERT_message();    /* INSERT the Diskette                    */

        diskette_inserted = TRUE;

        /* ............................................................... */
        /* Check to see if the diskette has been inserted                  */
        /* ............................................................... */

        check_diskette_status(FIRST_STAGE);

        DFD_TESTING_message();  /* display the Standby.... message  */

        do_fdsa_tests(diskette_index);

        clean_up();
}

/*
 * NAME: int drive_type();
 *
 * FUNCTION: Look at the value field in the CuAt database.
 *
 * RETURNS: 1 if drive is 4MB, 0 if drive is not a 4MB,
 *          -1 if error.
 */

int drive_type(int drive_num)
{
      struct CuAt *cuat;
	  struct CuDv *cudv;
      struct listinfo obj_info;
      char criteria[128];

   	  sprintf(criteria,"name like fd?");
      cudv = get_CuDv_list(CuDv_CLASS,criteria,&obj_info,1,1);
      if (cudv == (struct CuDv *) NULL) {
              return(-2);
      }

   	  sprintf(criteria,"name = fd%d",drive_num);
      cuat = get_CuAt_list(CuAt_CLASS,criteria,&obj_info,1,1);

      if (cuat == (struct CuAt *) - 1) {
              return(-1);
      }


      if ( strcmp(cuat->value,"3.5inch4Mb") == 0 ) {
              return(1);
      }
      else if ( strcmp(cuat->value,"5.25inch") == 0 ){
			  return(2);
	  } else {
              return(0);
     }


} /* end drive_type() */

/*
 * NAME: void check_ioctl_return(int return_code);
 *
 * FUNCTION: If the return code from the ioctl() is -1 then exit();
 *
 * RETURNS: NONE
*/

void check_ioctl_return(rc)
{

        if(rc == AIX_ERROR)
        {
                DFD_ERROR_message(NOT_USABLE);
                clean_up();
        }
}

/*
 * NAME: void clean_up()
 *
 * FUNCTION: IF a diskette was inserted into a drive tell the user to remove it.
 *           Close any open files prior to exiting to the diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: NONE
*/


void clean_up()
{

        void DFD_REMOVE_message();

        if(diskette_inserted == TRUE )
                DFD_REMOVE_message();
        if(dev_fdes > 0)
        {
                check_ioctl_return(ioctl(dev_fdes,FDIOCDSELDRV));
                close(dev_fdes);
        }
        if(catd != CATD_ERR)
                catclose(catd);
        diag_asl_quit(NULL);
        exit(0);
}


