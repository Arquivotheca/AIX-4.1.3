static char sccsid[] = "@(#)16	1.7  src/bos/diag/util/ufd/fdsamenu.c, dsaufd, bos411, 9428A410j 7/13/93 15:25:32";
/*
 *   COMPONENT_NAME: DSAUFD
 *
 *   FUNCTIONS: 
 *		DFD_DEFAULT_query
 *		DFD_DESCRIPTOR_message
 *		DFD_ERROR_message
 *		DFD_INSERT_message
 *		DFD_REMOVE_message
 *		DFD_RESULT_message
 *		DFD_SELECT_menu
 *		DFD_TESTING_message
 *		DIAG_NUM_ENTRIES
 *		disk_msg_type
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

#include "diag/diago.h"
#include "fd_sa.h"
#include "ufd_msg.h"

#ifndef SECTOR_SIZE
#define SECTOR_SIZE 512
#endif

extern nl_catd  catd;
extern int diskette_index;     /*  0,1,2, or 3 -> 3.5/5.25/ hi/low  */
extern int selected_drive;     /* drive 0 or 1                      */
extern int diskette_inserted;  /* TRUE or FALSE                     */
extern int error_found;        /* TRUE or FALSE                     */
extern int sectors_read;     /* NUmber of cylinders checked       */
extern char *diag_cat_gets();


struct msglist fd_sa_msg[] = {
        { DISKETTE_SA ,  DISKETTE_SA_TITLE },
        { DISKETTE_SA ,  AID_DESCRIPTION   },
        (int)NULL
};

ASL_SCR_INFO   fd_sa_msg_list[DIAG_NUM_ENTRIES(fd_sa_msg)];
ASL_SCR_TYPE   menutype = DM_TYPE_DEFAULTS;

/* ............................................................................

DISKETTE MEDIA SERVICE AIDS                               802060

Diskette Media Service Aid Description:

This Service Aid allows you to determine if the diskette media
has any defects.

This is accomplished by reading the entire diskette
to detect any error.

Press Enter to continue:

............................................................................ */

void DFD_DESCRIPTOR_message(void)
{
        int rc;
        rc = diag_display( 0x802060, catd ,fd_sa_msg , DIAG_IO,
                                         ASL_DIAG_KEYS_ENTER_SC,
                                         &menutype,fd_sa_msg_list);
        chk_asl_return(rc);
}

struct msglist fd_sa_select[] = {
        { DISKETTE_SA ,  DISKETTE_SA_TITLE },
        { DISKETTE_SA ,  FD_288  },
        { DISKETTE_SA ,  FD_144  },
        { DISKETTE_SA ,  FD_720  },
        { DISKETTE_SA ,  FD_12   },
        { DISKETTE_SA ,  FD_360  },
        { DISKETTE_SA ,  S_HEAD  },
        (int)NULL
};

ASL_SCR_INFO   fd_sa_select_menu[DIAG_NUM_ENTRIES(fd_sa_select)];

/* ............................................................................

DISKETTE MEDIA SERVICE AIDS   - DISKETTE VERIFICATION            802061

Diskette Type And Drive Location Selection

Move the cursor to the type of Diskette that requires verification,
                 then press Enter to continue:

        | 1.44 Mb . .|. . 3.5  Inch . .|. .  Drive 0  |
        |  720 Kb . .|. . 3.5  Inch . .|. .  Drive 0  |
        | 1.2  Mb . .|. . 5.25 Inch . .|. .  Drive 1  |
        |  360 Kb . .|. . 5.25 Inch . .|. .  Drive 1  |

............................................................................ */


int DFD_SELECT_menu(void)
{
        int rc;
		int i;
        extern int drive_4MB;
        extern int drive_525;

        rc = diag_display( 0x802061 , catd , fd_sa_select , DIAG_MSGONLY,
                                          ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                          &menutype,fd_sa_select_menu);
        chk_asl_return(rc);

		if(drive_525 == 0)
		{
			fd_sa_select_menu[4] = fd_sa_select_menu[6];
			menutype.max_index -= 2;
		}

		if(drive_4MB == 0)
		{
			for(i=1;i<menutype.max_index;i++)
			{
				fd_sa_select_menu[i] = fd_sa_select_menu[i+1];
			}
			menutype.max_index --;
		}
		
        rc = diag_display( 0x802061 , catd , NULL , DIAG_IO,
                                          ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                          &menutype,fd_sa_select_menu);
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(menutype) - drive_4MB;
        return(rc) ;
}

struct msglist fd_sa_insert[] = {
        { DISKETTE_SA ,  DISKETTE_SA_TITLE },
        { DISKETTE_SA ,  INSERT_DISK       },
        (int)NULL
};

ASL_SCR_INFO   fd_sa_insert_menu[DIAG_NUM_ENTRIES(fd_sa_insert)];




/* ...........................................................................

DISKETTE MEDIA SERVICE AIDS                                 802062

DISKETTE REQUIRED. : 1.44 Mb 3.5 Inch High Capacity

INSERT . . . . . . . the required diskette into Drive x.

CLOSE  . . . . . . . the Drive.

Press Enter to continue:

............................................................................ */

void DFD_INSERT_message(void)
{
        int rc;
        char text[A_SCREEN];
        int panel_number = 0x802062;
        char *disk_msg_type();

        rc = diag_display( panel_number,catd,fd_sa_insert, DIAG_MSGONLY ,
                                          ASL_DIAG_KEYS_ENTER_SC,
                                          &menutype,fd_sa_insert_menu);

        if(selected_drive == DRIVE_1)
                sprintf(text,fd_sa_insert_menu[ 1 ].text,
                                           disk_msg_type(DISK_144),
                                           selected_drive,
                                           diag_cat_gets( catd , DISKETTE_SA ,
                                           CLOSE_DOOR , "" ));
         else
                sprintf(text,fd_sa_insert_menu[ 1 ].text,
                                           disk_msg_type(DISK_144),
                                           selected_drive,
                                           diag_cat_gets( catd , DISKETTE_SA ,
                                           ZIP , "" ));

        free(fd_sa_insert_menu[ 1 ].text);
        fd_sa_insert_menu[ 1 ].text = (char*)malloc(strlen(text)+1);
        strcpy(fd_sa_insert_menu[ 1 ].text,text);

        rc = diag_display( panel_number,catd ,  NULL , DIAG_IO ,
                                           ASL_DIAG_KEYS_ENTER_SC,
                                           &menutype,fd_sa_insert_menu);
        chk_asl_return(rc);

}

struct msglist fd_sa_testing[] = {
        { DISKETTE_SA ,  DISKETTE_SA_TITLE },
        { DISKETTE_SA ,  FD_SA_TESTING     },
        (int)NULL
};
/* ...........................................................................


DISKETTE MEDIA SERVICE AIDS                                 802063

DISKETTE LOCATION . . . . . .: Drive 0

DISKETTE TYPE . . . . . . . .: 1.44 Mb 3.5 Inch High Capacity


Verification Test In Progress.
Please standby.


............................................................................ */


ASL_SCR_INFO   fd_sa_testing_msg[DIAG_NUM_ENTRIES(fd_sa_testing)];

void DFD_TESTING_message(void)
{
        int rc;
        int panel_number = 0x802063;
        char text[A_SCREEN];
        char *error_status = (char *)malloc(A_LINE);
        char *disk_msg_type();

        rc = diag_display( panel_number,catd,fd_sa_testing, DIAG_MSGONLY ,
                                          ASL_DIAG_OUTPUT_LEAVE_SC,
                                          &menutype,fd_sa_testing_msg);
        if(error_found == TRUE)
               error_status = diag_cat_gets(catd,DISKETTE_SA,ERROR_RECORD,"" );
        else
               error_status = diag_cat_gets(catd,DISKETTE_SA,ZIP,"" );

        sprintf(text,fd_sa_testing_msg[ 1 ].text,
                                          selected_drive,
                                          disk_msg_type(TESTING_DISK_144),
                                          error_status );
        free(fd_sa_testing_msg[ 1 ].text);
        fd_sa_testing_msg[1].text = (char*)malloc(strlen(text)+1);
        strcpy(fd_sa_testing_msg[ 1 ].text,text);

        rc = diag_display( panel_number,catd,NULL, DIAG_IO ,
                                          ASL_DIAG_OUTPUT_LEAVE_SC,
                                          &menutype,fd_sa_testing_msg );
        chk_asl_return(rc);

}

struct msglist fd_sa_result[] = {
        { DISKETTE_SA ,  DISKETTE_SA_TITLE },
        { DISKETTE_SA ,  RESULT_MESSAGE    },
        (int)NULL
};

/* ............................................................................

DISKETTE MEDIA SERVICE AIDS                                 802064

DISKETTE LOCATION . . . . . . : 0

DISKETTE TYPE . . . . . . . . : 1.44 Mb 3.5 Inch High Capacity
BAD SECTORS REPORTED  . . . . : xxxxxx Sectors
GOOD SECTORS REMAINING  . . . : xxxxxx Sectors

The Diskette did NOT verify on all sectors.

Press Enter continue:
............................................................................ */

ASL_SCR_INFO   fd_sa_result_msg[DIAG_NUM_ENTRIES(fd_sa_result)];

void DFD_RESULT_message( total_sectors, bad_sectors , good_sectors)
{

        int rc;
        char text[A_SCREEN];
        int diskette_status;
        int panel_number = 0x802064 ;
        char *disk_msg_type();

        rc = diag_display( panel_number,catd,fd_sa_result, DIAG_MSGONLY ,
                                          ASL_DIAG_KEYS_ENTER_SC,
                                          &menutype,fd_sa_result_msg);
        if(bad_sectors != 0)
                 diskette_status = BAD_DISKETTE;
        else
                 diskette_status = GOOD_DISKETTE;

        sprintf(text,fd_sa_result_msg [ 1 ].text,
                                          selected_drive,
                                          disk_msg_type(DISK_144),
                                          total_sectors,
                                          bad_sectors ,
                                          good_sectors,
                                          diag_cat_gets(catd,
                                          DISKETTE_SA,
                                          diskette_status,
                                          ""));
        free(fd_sa_result_msg[ 1 ].text);
        fd_sa_result_msg[1].text = (char*)malloc(strlen(text)+1);
        strcpy(fd_sa_result_msg[ 1 ].text,text);
        rc = diag_display( panel_number,catd ,  NULL , DIAG_IO ,
                                          ASL_DIAG_KEYS_ENTER_SC,
                                          &menutype,fd_sa_result_msg);
        chk_asl_return(rc);

}


struct msglist fd_sa_remove[] = {
        { DISKETTE_SA ,  DISKETTE_SA_TITLE },
        { DISKETTE_SA ,  REMOVE_DISK       },
        (int)NULL,
};

/* ............................................................................

DISKETTE MEDIA SERVICE AIDS                                 802067

REMOVE . . . . . . . the diskette from Drive x

When ready , press Enter:

............................................................................ */

ASL_SCR_INFO   fd_sa_remove_msg[DIAG_NUM_ENTRIES(fd_sa_remove)];

void DFD_REMOVE_message(void)
{
        int rc;
        char text[A_SCREEN];
        rc = diag_display( 0x802067,catd,fd_sa_remove, DIAG_MSGONLY ,
                                          ASL_DIAG_KEYS_ENTER_SC,
                                          &menutype,fd_sa_remove_msg);
        sprintf(text,fd_sa_remove_msg[ 1 ].text, selected_drive );
        free(fd_sa_remove_msg[ 1 ].text);
        fd_sa_remove_msg[1].text = (char*)malloc(strlen(text)+1);
        strcpy(fd_sa_remove_msg[ 1 ].text,text);
        rc = diag_display( 0x802067, catd ,  NULL , DIAG_IO ,
                                          ASL_DIAG_KEYS_ENTER_SC,
                                          &menutype,fd_sa_remove_msg);
        diskette_inserted = FALSE;
        chk_asl_return(rc);

}

struct msglist fd_sa_error[] = {
        { DISKETTE_SA ,  DISKETTE_SA_TITLE },
        { DISKETTE_SA ,  ERROR_MESSAGE     },
        (int)NULL,
};



/* ............................................................................

DISKETTE MEDIA SERVICE AIDS                                 802050

An error was detected before the media verification began.

The error indicates that the diskette was NOT inserted in the drive.
Remove and insert the diskette.

Press Enter to continue.

............................................................................ */

ASL_SCR_INFO   fd_sa_error_msg[DIAG_NUM_ENTRIES(fd_sa_error)];


void DFD_ERROR_message(sa_error)
int sa_error;
{
        int rc;
        char text[A_SCREEN];

        rc = diag_display( 0x802050,catd,fd_sa_error, DIAG_MSGONLY ,
                                          ASL_DIAG_KEYS_ENTER_SC,
                                          &menutype,fd_sa_error_msg);
        sprintf(text,fd_sa_error_msg[ 1 ].text, diag_cat_gets(catd,
                                                             DISKETTE_SA,
                                                              sa_error,""));
        free(fd_sa_error_msg[ 1 ].text);

        fd_sa_error_msg[1].text = (char*)malloc(strlen(text)+1);
        strcpy(fd_sa_error_msg[ 1 ].text,text);
        rc = diag_display( 0x802067, catd ,  NULL , DIAG_IO ,
                                          ASL_DIAG_KEYS_ENTER_SC,
                                          &menutype,fd_sa_error_msg);
        chk_asl_return(rc);
}


struct msglist fd_default_list[] = {
        { DISKETTE_SA ,  DISKETTE_SA_TITLE },
        { DISKETTE_SA ,  YES_OPTION        },
        { DISKETTE_SA ,  NO_OPTION         },
        { DISKETTE_SA ,  DEFAULT_QUERY     },
        (int)NULL,
};


ASL_SCR_INFO   fd_default_msg[DIAG_NUM_ENTRIES(fd_default_list)];

int DFD_DEFAULT_query( total_sectors, sectors_read , num_bad_sectors)
{
        int rc;
        char TEXT[ A_SCREEN ];
        char *disk_msg_type();

        rc = diag_display( 0x802069 , catd , fd_default_list , DIAG_MSGONLY,
                                          ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                          &menutype,fd_default_msg);

        sprintf(TEXT,fd_default_msg[3].text, selected_drive,
                                          disk_msg_type(DISK_144),
                                           total_sectors,
                                           sectors_read,
                                           num_bad_sectors,
                                           sectors_read - num_bad_sectors,
                                           total_sectors - sectors_read
                                           );
        free(fd_default_msg[3].text);
        fd_default_msg[3].text = (char *)malloc(sizeof(TEXT)+1);
        strcpy(fd_default_msg[3].text,TEXT);
        rc = diag_display( 0x802069 , catd , NULL , DIAG_IO,
                                          ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                          &menutype,fd_default_msg);
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(menutype);
        return(rc) ;
}

char *disk_msg_type(msg_type)
int msg_type;
{
        char *msg_ptr;
        extern int drive_4MB;

        msg_ptr = (char *)malloc(128);

        if (drive_4MB)
        {
                if (msg_type == DISK_144)
                {
                        switch(diskette_index)
                        {
                        case 0:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    DISK_288,"");
                                break;
                        case 1:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    DISK_144,"");
                                break;
                        case 2:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    DISK_720,"");
                                break;
                        case 3:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    DISK_12,"");
                                break;
                        case 4:
                        default:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    DISK_360,"");
                                break;
                        }
                }
                else
                {
                        switch(diskette_index)
                        {
                        case 0:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    TESTING_DISK_288,"");
                                break;
                        case 1:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    TESTING_DISK_144,"");
                                break;
                        case 2:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    TESTING_DISK_720,"");
                                break;
                        case 3:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    TESTING_DISK_12,"");
                                break;
                        case 4:
                        default:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    TESTING_DISK_360,"");
                                break;
                        }
               }
        }
        else
        {
                if (msg_type == DISK_144)
                {
                        switch(diskette_index)
                        {
                        case 1:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    DISK_144,"");
                                break;
                        case 2:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    DISK_720,"");
                                break;
                        case 3:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    DISK_12,"");
                                break;
                        case 4:
                        default:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    DISK_360,"");
                                break;
                        }
                }
                else
                {
                        switch(diskette_index)
                        {
                        case 1:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    TESTING_DISK_144,"");
                                break;
                        case 2:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    TESTING_DISK_720,"");
                                break;
                        case 3:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    TESTING_DISK_12,"");
                                break;
                        case 4:
                        default:
                                msg_ptr = diag_cat_gets(catd,DISKETTE_SA,
                                                    TESTING_DISK_360,"");
                                break;
                        }
                }
        }

        return(msg_ptr);

}

