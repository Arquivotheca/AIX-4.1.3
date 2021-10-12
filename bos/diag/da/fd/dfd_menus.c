static char sccsid[] = "@(#)83	1.12  src/bos/diag/da/fd/dfd_menus.c, dafd, bos411, 9428A410j 12/17/92 10:53:26";
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: ADAPTER_TEST_message
 *		DENSITY_CHECK_menu
 *		DESELECT_TEST_menu
 *		DFD_EXER_TEST_menu
 *		DFD_LOOP_TEST_message
 *		DIAG_NUM_ENTRIES
 *		DISK_CHANGE_TEST_menu
 *		DISK_CHANGE_TEST_message
 *		DISK_type
 *		DISK_type_insert1126
 *		DOOR_type
 *		DRIVE_LOCATION_query
 *		DRIVE_size
 *		READ_WRITE_INSERT_message
 *		READ_WRITE_RUNNING_message
 *		READ_WRITE_query
 *		RECALIBRATE_TEST_message
 *		SELECT_TEST_message
 *		TEST_mode
 *		fd_exit_message
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



#include <sys/types.h>
#include "fd_set.h"
#include "diag/diago.h"
#include "dfd_msg.h"            /* menu text catalog offsets     */
#include "diag/tm_input.h"
#include "diag/tmdefs.h"

extern nl_catd catd;
extern struct fd_drive *fd;
extern int Scratch_Disk_Available ;
extern struct tm_input da_input;
extern int diskette_inserted;   /* used in fd_exit_message()     */
extern int drive_4MB;
extern char *diag_cat_gets();

char *DRIVE_size();
char *TEST_mode();
char *DISK_type();
char *DISK_type_insert();
char *DOOR_type();

ASL_SCR_TYPE asl_menu_type = DM_TYPE_DEFAULTS;



/* =========================== N O T E  ==============================

The EXAMPLE SCREENs show before each display may not  be exactly correct.
     The examples are shown to indicate the INTENT of each display .

    ( SEE  dfd.msg and dfd_msg.h for struct values and text strings.)

====================================================================== */

/*
 * NAME: void ADAPTER_TEST_message()
 *
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Called from do_fd_tests()
 *
 *
 * RETURNS: NONE
*/

/*............. E X A M P L E ....... S C R E E N ......................

TESTING 3.5 Inch (DRIVE 0) DISKETTE DRIVE IN ADVANCED MODE  935000

   Please Standby.

.................................................................... */

struct msglist adapter_list[] = {
        { Diskette_Diagnostics, Testing_msg },
        { Diskette_Diagnostics, Stand_by},
        (int)NULL
};
ASL_SCR_INFO  adapter_msg[DIAG_NUM_ENTRIES(adapter_list)];

void ADAPTER_TEST_message()
{
        int rc;
        int mn;                             /* menu number */
        char Top_Text[1024];
        char *Ds = DRIVE_size();
        char *Mode = TEST_mode();
        mn = 0x935000;
        if(fd->size == FIVE25 )
            mn = 0x936000;
        rc = diag_display(mn,catd, adapter_list, DIAG_MSGONLY,
                           ASL_DIAG_OUTPUT_LEAVE_SC,
                           &asl_menu_type,adapter_msg);
        sprintf(Top_Text,adapter_msg[0].text,Ds,da_input.dname,Mode);
        free(adapter_msg[0].text);
        adapter_msg[0].text = (char *)malloc(strlen(Top_Text) +1);
        strcpy(adapter_msg[0].text,Top_Text);

        rc = diag_display(mn, catd, NULL ,DIAG_IO ,
                          ASL_DIAG_OUTPUT_LEAVE_SC,
                          &asl_menu_type,adapter_msg);
        chk_asl_return(rc);
}

/*
 * NAME: int DESELECT_TEST_menu()
 *
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Called from do_fd_tests()
 *
 *
 * RETURNS: ANSWERED_YES (1) or  !1
*/

/*............. E X A M P L E ....... S C R E E N ......................

TESTING IBM 5.25 Inch (DRIVE 1) DISKETTE DRIVE IN ADVANCED MODE  935080

Is the Diskette Drive that is being tested (DRIVE 1)
an Internal or External drive.

       Move curser to the selection , and then press enter.

EXTERNAL DRIVE
INTERAL DRIVE

.................................................................... */

struct msglist loc_query_list[] = {
        { Diskette_Diagnostics, Testing_msg },
        { Diskette_Diagnostics, External_Option },
        { Diskette_Diagnostics, Internal_Option },
        { Diskette_Diagnostics, Drive_Loc_Query },
        (int)NULL
};

ASL_SCR_INFO loc_menu[DIAG_NUM_ENTRIES (loc_query_list) ];

int DRIVE_LOCATION_query()
{
        int rc;                            /* return code                    */
        int mn;                            /* menu number                    */
        char Top_Text[1024];
        int setnum  = Diskette_Diagnostics;
        char *Ds    = DRIVE_size();
        char *Mode  = TEST_mode();
        mn = 0x935080;
        if(fd->size == FIVE25 )
                mn = 0x936080 ;
        rc = diag_display( mn, catd, loc_query_list , DIAG_MSGONLY,
                           ASL_DIAG_LIST_CANCEL_EXIT_SC,
                           &asl_menu_type,loc_menu);
        sprintf(Top_Text,loc_menu[0].text,Ds,da_input.dname,Mode);
        free(loc_menu[0].text);
        loc_menu[0].text = (char *)malloc(strlen( Top_Text )+1);
        strcpy(loc_menu[0].text, Top_Text );

        sprintf(Top_Text,loc_menu[3].text,da_input.dname);
        free(loc_menu[3].text);
        loc_menu[3].text = (char *)malloc(strlen(Top_Text)+1);
        strcpy(loc_menu[3].text, Top_Text);
        rc = diag_display(mn,catd, NULL , DIAG_IO ,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC ,
                          &asl_menu_type,loc_menu);
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(asl_menu_type);
        return ( rc );
}


/*............. E X A M P L E ....... S C R E E N ......................

TESTING IBM 3.5 Inch (DRIVE 0) DISKETTE DRIVE IN ADVANCED MODE  935025

Diskette Select & Deselect Tests

Observe the in-use Light on the diskette drive(0).
If the in-use light is still on wait 5 seconds
and then answer the following question.

Was the in-use light on for approximately 5 seconds and then go off?
       Move curser to the selection , and then press enter.

YES
NO

.................................................................... */

struct msglist d_query_list[] = {
        { Diskette_Diagnostics, Testing_msg },
        { Diskette_Diagnostics, Yes_option },
        { Diskette_Diagnostics, No_option },
        { Diskette_Diagnostics, Diditdoit },
        (int)NULL
};

ASL_SCR_INFO dq_menu[DIAG_NUM_ENTRIES (d_query_list) ];

int DESELECT_TEST_menu()
{
        int rc;                            /* return code                    */
        int mn;                            /* menu number                    */
        char Top_Text[1024];
        int setnum  = Diskette_Diagnostics;
        char *Ds    = DRIVE_size();
        char *Mode  = TEST_mode();
        mn = 0x935025;
        if(fd->size == FIVE25 )
                mn = 0x936025;
        rc = diag_display( mn, catd, d_query_list , DIAG_MSGONLY,
                           ASL_DIAG_LIST_CANCEL_EXIT_SC,
                           &asl_menu_type,dq_menu);
        sprintf(Top_Text,dq_menu[0].text,Ds,da_input.dname,Mode);
        free(dq_menu[0].text);
        dq_menu[0].text = (char *)malloc(strlen( Top_Text )+1);
        strcpy(dq_menu[0].text, Top_Text );

        sprintf(Top_Text,dq_menu[3].text,da_input.dname);
        free(dq_menu[3].text);
        dq_menu[3].text = (char *)malloc(strlen(Top_Text)+1);
        strcpy(dq_menu[3].text, Top_Text);
        rc = diag_display(mn,catd, NULL , DIAG_IO ,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC ,
                          &asl_menu_type,dq_menu);
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(asl_menu_type);
        return ( rc );
}

struct msglist d_check_list[] = {
        { Diskette_Diagnostics, Testing_msg },
        { Diskette_Diagnostics, Yes_option },
        { Diskette_Diagnostics, No_option },
        { Diskette_Diagnostics, Density_check_msg },
        (int)NULL
};

ASL_SCR_INFO d_check_menu[DIAG_NUM_ENTRIES (d_check_list) ];

int DENSITY_CHECK_menu()
{
        int rc;                            /* return code                    */
        int mn;                            /* menu number                    */
        char Top_Text[1024];
        int setnum  = Diskette_Diagnostics;
        char *Ds    = DRIVE_size();
        char *Mode  = TEST_mode();
        mn = 0x935085;
        if(fd->disk_density == HIGH_DENSITY)
        {
                mn = 0x935086;
        }
        if(fd->disk_density == LOW_DENSITY)
        {
                mn = 0x935087;
        }
        if(fd->size == FIVE25)
                mn += 0x1000;
        rc = diag_display( mn, catd, d_check_list , DIAG_MSGONLY,
                           ASL_DIAG_LIST_CANCEL_EXIT_SC,
                           &asl_menu_type,d_check_menu);
        sprintf(Top_Text,d_check_menu[0].text,Ds,da_input.dname,Mode);
        free(d_check_menu[0].text);
        d_check_menu[0].text = (char *)malloc(strlen( Top_Text )+1);
        strcpy(d_check_menu[0].text, Top_Text );

        if (fd->disk_density == LOW_DENSITY)
                sprintf(Top_Text,
                          diag_cat_gets(catd,setnum,Low_Density_check_msg,""),
                          da_input.dname);
        else
                sprintf(Top_Text,
                          d_check_menu[3].text,
                          da_input.dname);

        free(d_check_menu[3].text);
        d_check_menu[3].text = (char *)malloc(strlen(Top_Text)+1);
        strcpy(d_check_menu[3].text, Top_Text);
        rc = diag_display(mn,catd, NULL , DIAG_IO ,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC ,
                          &asl_menu_type,d_check_menu);
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(asl_menu_type);
        return ( rc );
}

/*
 * NAME: int DISK_CHANGE_TEST_menu()
 *
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Called from do_fd_tests()
 *
 *
 * RETURNS: ANSWERED_YES (1) or !1
*/


/*............. E X A M P L E ....... S C R E E N ......................

TESTING IBM 3.5 Inch (DRIVE 0) DISKETTE DRIVE IN ADVANCED MODE  935040

Diskette Change Test

The following tests require a good 3.5 Inch High Density Diskette.

Skipping these test means the Diskette Drive will not be tested adequately.
                  Do you have the required diskette?

             Move curser to slection and the press Enter:
   YES
   NO
.................................................................... */

struct msglist disk_change_list[] = {
        { Diskette_Diagnostics, Testing_msg },
        { Diskette_Diagnostics, Yes_option },
        { Diskette_Diagnostics, No_option },
        { Diskette_Diagnostics, Doyouhave },
        (int)NULL
};
ASL_SCR_INFO  dc_menu[DIAG_NUM_ENTRIES(disk_change_list)] ;

int DISK_CHANGE_TEST_menu()
{
        int rc = 0, mn;
        char Top_Text[1024];
        int setnum   = Diskette_Diagnostics;
        char *Dt     = DISK_type();
        char *Mode   = TEST_mode();
        char *Ds     = DRIVE_size();
        char *Test   = (char *)malloc(1024);
        char *Prot   = (char *) malloc(1024);

        Prot = diag_cat_gets(catd,setnum,Is_protected,"");
        Test = diag_cat_gets(catd,setnum,Disk_Change_Test ,"");
        mn = 0x935040;                /* Default 3.5" Diskette  */
        if(fd->size == FIVE25 )
                mn = 0x936040;        /* Optional 5.25 Diskette */
        rc = diag_display(mn,catd, disk_change_list, DIAG_MSGONLY,
                               ASL_DIAG_LIST_CANCEL_EXIT_SC,
                               &asl_menu_type,dc_menu);
        sprintf(Top_Text,dc_menu[0].text,Ds,da_input.dname,Mode);
        free(dc_menu[0].text);
        dc_menu[0].text = (char *)malloc(strlen( Top_Text)+1);
        strcpy( dc_menu[0].text, Top_Text );
        sprintf(Top_Text,dc_menu[ 3 ].text,Test,Dt,Prot);
        free(dc_menu[3].text);
        dc_menu[3].text = (char *)malloc(strlen(Top_Text)+1 );
        strcpy(dc_menu[ 3 ].text ,Top_Text);
        rc = diag_display( mn, catd, NULL , DIAG_IO,
                                 ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                 &asl_menu_type, dc_menu);
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(asl_menu_type);
        return(rc);
}


/*
 * NAME: void DISK_CHANGE_TEST_message()
 *
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Called from do_fd_tests()
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
*/

/*............. E X A M P L E ....... S C R E E N ......................

TESTING IBM 3.5 Inch (DRIVE 0) DISKETTE DRIVE IN ADVANCED MODE  935045

Diskette Change Test


REMOVE ....... the diskette, if any , from the Diskette Drive (DRIVE 0)
INSERT ....... a diskette into DRIVE 0.

Press Enter to continue.

.................................................................... */

struct msglist  insert_disk_list[] = {
        { Diskette_Diagnostics, Testing_msg },
        { Diskette_Diagnostics,Insert_Disk_Message },
        (int)NULL
};
ASL_SCR_INFO  id_menu[DIAG_NUM_ENTRIES(insert_disk_list)] ;

void DISK_CHANGE_TEST_message()
{
        int rc = 0 ;
        int mn;
        int setnum = Diskette_Diagnostics;
        char Top_Text[1024] ;
        char *Dt    = DISK_type_insert() ;
        char *Door  = DOOR_type();
        char *Ds    = DRIVE_size();
        char *Mode  = TEST_mode();
        char *Prot   = (char *) malloc(1024);
        char *Test = (char *)malloc(1024);

        Test = diag_cat_gets(catd,setnum,Disk_Change_Test ,"");
        mn= 0x935045;
        if(fd->size  == FIVE25) {
                Prot = diag_cat_gets(catd,setnum,Protect_note_Five25,"");
                mn = 0x936045;
        } else
                Prot = diag_cat_gets(catd,setnum,Protect_note_Three5,"");

        rc = diag_display(mn,catd, insert_disk_list, DIAG_MSGONLY,
                               ASL_DIAG_ENTER_SC,
                               &asl_menu_type,id_menu);
        sprintf(Top_Text , id_menu[0].text,Ds,da_input.dname,Mode);
        free(id_menu[0].text);
        id_menu[0].text = (char *)malloc(strlen( Top_Text )+1);
        strcpy(id_menu[0].text, Top_Text );

        sprintf(Top_Text,id_menu[1].text,
                  Test,NULL,NULL,da_input.dname,Dt,
                  da_input.dname,
                  Door,Prot);
        free(id_menu[1].text);
        id_menu[1].text = (char *)malloc(strlen(Top_Text) +1);
        strcpy(id_menu[1].text, Top_Text);

        rc = diag_display( mn, catd, NULL, DIAG_IO,
                                 ASL_DIAG_KEYS_ENTER_SC,
                                 &asl_menu_type,id_menu) ;
        chk_asl_return(rc);
}


/*
 * NAME: void fd_exit_message()
 *
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Called from clean_up() in fd_main.c
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
*/

/*............. E X A M P L E ....... S C R E E N ......................

TESTING IBM 3.5 Inch (DRIVE 0) DISKETTE DRIVE IN ADVANCED MODE  935999

REMOVE ....... the diskette, if any , from Diskette Drive (DRIVE 0).

Press Enter to continue.

.................................................................... */


struct msglist   exit_list[] = {
            { Diskette_Diagnostics,Testing_msg},
            { Diskette_Diagnostics,Remove_Diskette},
            (int)NULL
};

ASL_SCR_INFO exit_menu[DIAG_NUM_ENTRIES(exit_list)];

void fd_exit_message()
{
        int rc ;
        char Top_Text[1024];
        char *Ds = DRIVE_size();
        char *Mode = TEST_mode();
        rc = diag_display(0x935999,catd,exit_list, DIAG_MSGONLY,
                           ASL_DIAG_KEYS_ENTER_SC,
                           &asl_menu_type,exit_menu);
        sprintf(Top_Text,exit_menu[0].text,Ds,da_input.dname,Mode);
        free(exit_menu[0].text);
        exit_menu[0].text = (char *)malloc(strlen(Top_Text));
        strcpy(exit_menu[0].text,Top_Text );
        sprintf(Top_Text,exit_menu[1].text,da_input.dname);
        free(exit_menu[1].text);
        exit_menu[1].text = (char *)malloc(strlen(Top_Text));
        strcpy(exit_menu[1].text,Top_Text );
        rc = diag_display(0x935999,catd, NULL,  DIAG_IO,
                           ASL_DIAG_KEYS_ENTER_SC,
                           &asl_menu_type,exit_menu);
        /* ............................................................ */
        /* Set diskette_inserted to FALSE to keep from looping back     */
        /* here if the user hits the Esc key.                           */
        /* ............................................................ */
        diskette_inserted = FALSE;
        chk_asl_return(rc);
}


struct msglist fd_exer_test[] = {
            { Diskette_Diagnostics, EXER_MENU },
            { Diskette_Diagnostics, OPTION_EXER_1},
            { Diskette_Diagnostics, OPTION_EXER_2},
            { Diskette_Diagnostics, Choose_an_item },
            (int)NULL
};

ASL_SCR_INFO fd_exer_menu[DIAG_NUM_ENTRIES(fd_exer_test)];

/*
 * NAME: DFD_EXER_TEST_menu()
 *
 * FUNCTION: Display the system exerciser menu for media installation.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: 1 if setup desired, 2 if no setup, -1 on error.
*/

int DFD_EXER_TEST_menu()
{
        int rc = 0, mn;
        char Top_Text[1024];
        char *Dt     = DISK_type();

        mn = 0x935997;                /* Default 3.5" Diskette  */
        if(fd->size == FIVE25 )
                mn = 0x936997;        /* Optional 5.25 Diskette */
        rc = diag_display(mn,catd, fd_exer_test, DIAG_MSGONLY,
                               ASL_DIAG_LIST_CANCEL_EXIT_SC,
                               &asl_menu_type,fd_exer_menu);
        sprintf(Top_Text,fd_exer_menu[0].text,da_input.dname,Dt);
        free(fd_exer_menu[0].text);
        fd_exer_menu[0].text = (char *)malloc(strlen( Top_Text)+1);
        strcpy( fd_exer_menu[0].text, Top_Text );
        rc = diag_display( mn, catd, NULL , DIAG_IO,
                                 ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                 &asl_menu_type, fd_exer_menu);
        if ( (rc == ASL_CANCEL) || (rc == ASL_EXIT) )
                diag_asl_quit();
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(asl_menu_type);
        return(rc);

}

/*
 * NAME:
 *
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Diplay the number of passes and the errors logged
 *                        Called from do_fd_tests()
 *
 *
 * RETURNS:
*/

/*............. E X A M P L E ....... S C R E E N ......................

TESTING IBM 3.5 Inch (DRIVE 0) DISKETTE DRIVE IN ADVANCED MODE  935900

Testing multiple times.

xxx completed passes
xxx error logged

Testing can be stopped by pressing the End key.

.................................................................... */

struct msglist dfd_loop_list[]= {
        { Diskette_Diagnostics,Testing_msg },
        { Diskette_Diagnostics,Loop_msg },
        (int)NULL
};

ASL_SCR_INFO dfd_loop_menu[DIAG_NUM_ENTRIES(dfd_loop_list)];

void DFD_LOOP_TEST_message()
{
        int rc;                            /* return code                 */
        int mn;                            /* menu number                 */
        int setnum = Diskette_Diagnostics; /* catalog offset pointer      */
        char Top_Text[1024];
        char *Ds = DRIVE_size();
        char *Mode = TEST_mode();
        mn = 0x935900 ;              /* 3.5 inch DRIVE..  DEFAULT   */
        if(fd->size == FIVE25)
                mn = 0x936900;
        rc = diag_display(mn, catd,dfd_loop_list, DIAG_MSGONLY,
                                ASL_DIAG_OUTPUT_LEAVE_SC,
                                &asl_menu_type,dfd_loop_menu );
        sprintf(Top_Text,dfd_loop_menu[0].text,Ds,da_input.dname,Mode);
        free(dfd_loop_menu[0].text) ;
        dfd_loop_menu[0].text = (char *) malloc(strlen(Top_Text)+1);
        strcpy(dfd_loop_menu[0].text,Top_Text);

        sprintf(Top_Text,dfd_loop_menu[1].text ,
                              da_input.lcount,da_input.lerrors);
        free(dfd_loop_menu[1].text);
        dfd_loop_menu[1].text =(char *) malloc(strlen(Top_Text)+1);
        strcpy(dfd_loop_menu[1].text,Top_Text);
        rc = diag_display(mn, catd ,NULL , DIAG_IO,
                               ASL_DIAG_OUTPUT_LEAVE_SC,
                               &asl_menu_type,dfd_loop_menu );
        chk_asl_return(rc);

}

/*
 * NAME: void RECALIBRATE_TEST_message()
 *
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Called from do_fd_tests()
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
*/

struct msglist recal_list[] = {
        { Diskette_Diagnostics, Testing_msg },
        { Diskette_Diagnostics, Recalibrate_msg},
        { Diskette_Diagnostics, Stand_by},
        (int)NULL
};

ASL_SCR_INFO  recal_msg[DIAG_NUM_ENTRIES(recal_list)];

void RECALIBRATE_TEST_message()
{
        int rc;
        int mn;                             /* menu number */
        char Top_Text[1024];
        char *Ds     = DRIVE_size();
        char *Mode   = TEST_mode();
        mn = 0x935030;
        if(fd->size == FIVE25 )
            mn = 0x936030;
        rc = diag_display(mn,catd, recal_list, DIAG_MSGONLY,
                               ASL_DIAG_OUTPUT_LEAVE_SC,
                               &asl_menu_type,recal_msg);
        sprintf(Top_Text,recal_msg[0].text,Ds,da_input.dname,Mode);
        free(recal_msg[0].text);
        recal_msg[0].text = (char *)malloc(strlen(Top_Text)+1);
        strcpy(recal_msg[0].text,Top_Text);
        rc = diag_display(mn, catd, NULL ,DIAG_IO ,
                                ASL_DIAG_OUTPUT_LEAVE_SC,
                                &asl_menu_type,recal_msg);
        chk_asl_return(rc);
}
/*
 * NAME: int READ_WRITE_query()
 *
 * FUNCTION:
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Called from do_fd_tests()
 *
 * NOTE:  Tests numbers 6,7,8,9,10,11,12,13,14  require this set of menus
 *  Ask the user if he has the correct disk  .... 2nd chance to stop test
 *
 * RETURNS:  ANSWERED_YES 1 or !1
*/


/*............. E X A M P L E ....... S C R E E N ......................

TESTING IBM 3.5 Inch (DRIVE 0) DISKETTE DRIVE IN ADVANCED MODE  935065

Read\Write\Compare Tests

The following tests require a good 3.5 Inch High Density
formatted scratch diskette that is NOT write protected.

                          --- NOTE  ----
Skipping thes test means the Diskette Drive will not be tested adequately.

                  Do you have the required diskette?

            Move the curser to selection , then press Enter.
   YES
   NO

........................................................................ */

struct msglist ask_for_disk_list[] = {
        { Diskette_Diagnostics, Testing_msg },
        { Diskette_Diagnostics, Yes_option },
        { Diskette_Diagnostics, No_option },
        { Diskette_Diagnostics, Doyouhave },
        (int)NULL
};
ASL_SCR_INFO ask_menu[DIAG_NUM_ENTRIES(ask_for_disk_list) ];

int READ_WRITE_query()
{
        int rc = 0;
        int mn;
        int setnum   = Diskette_Diagnostics;
        char Top_Text[1024];
        char *Dt     =  DISK_type() ;
        char *Ds     =  DRIVE_size();
        char *Mode   =  TEST_mode();
        char *Warn   = (char *) malloc(1024);
        char *Prot   = (char *) malloc(1024);

        Warn = diag_cat_gets(catd,setnum,Warning_msg,"");
        Prot = diag_cat_gets(catd,setnum,Is_not_protected,"");
        mn = 0x935065;
        if(fd->disk_density == HIGH_DENSITY)
        {
                mn = 0x935066;
        }
        if(fd->disk_density == LOW_DENSITY)
        {
                mn = 0x935067;
        }
        if(fd->size == FIVE25)
                mn += 0x1000;
        asl_menu_type.cur_index = 1;
        rc = diag_display(mn,catd , ask_for_disk_list , DIAG_MSGONLY,
                               ASL_DIAG_LIST_CANCEL_EXIT_SC,
                               &asl_menu_type,ask_menu);
        sprintf(Top_Text,ask_menu[0].text,Ds,da_input.dname,Mode);
        free(ask_menu[0].text);
        ask_menu[0].text = (char*)malloc(strlen( Top_Text) +1);
        strcpy(ask_menu[0].text,Top_Text) ;
        sprintf(Top_Text,ask_menu[3].text,Warn, Dt,Prot);
        free(ask_menu[3].text);
        ask_menu[3].text = (char*)malloc(strlen(Top_Text)+1);
        strcpy(ask_menu[3].text,Top_Text) ;
        rc = diag_display(mn,catd , NULL , DIAG_IO,
                               ASL_DIAG_LIST_CANCEL_EXIT_SC,
                               &asl_menu_type,ask_menu);
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(asl_menu_type);
        return(rc);
}

/*
 * NAME: void READ_WRITE_INSERT_message(void)
 *
 * FUNCTION:
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Called from do_fd_tests()
 *
 *
 * RETURNS: NONE
*/

/*............. E X A M P L E ....... S C R E E N ......................

TESTING IBM 3.5 Inch (DRIVE 0) DISKETTE DRIVE IN ADVANCED MODE  93507X

Diskette Read/Write/Compare Tests - High Compatability

WARNING!  THE FOLLOWING TESTS MAY DESTROY ALL DATA ON THE DISKETTE

REMOVE ....... the diskette, if any , from the Diskette Drive (DRIVE 0)
INSERT ....... a diskette into DRIVE 0.

NOTE: The Diskette MUST NOT be write protected.
NOTE: The Diskette MUST be formatted as HIGH DENSITY.

Press Enter to continue.
........................................................................ */

ASL_SCR_INFO insert_menu[DIAG_NUM_ENTRIES(insert_disk_list) ];

void READ_WRITE_INSERT_message(void)
{
        int rc = 0 , mn , setnum = Diskette_Diagnostics;
        char Top_Text[1024];
        char *Dt     =  DISK_type_insert();
        char *Door   =  DOOR_type();
        char *Ds     =  DRIVE_size();
        char *Mode   =  TEST_mode();
        char *Comp   = (char *)malloc(1024);
        char *test   = (char *)malloc(1024);
        char *Note_1 = (char *)malloc(1024);
        char *Warn   = (char *)malloc(1024);

        test = diag_cat_gets(catd,setnum,RWC_test,"");
        Comp = diag_cat_gets(catd,setnum,HiCompatability,"");
        mn = 0x935075;
        if(fd->disk_density == HIGH_DENSITY)
        {
                if (drive_4MB)
                        Comp = diag_cat_gets(catd,setnum,LowCompatability,"");
                else
                        Comp = diag_cat_gets(catd,setnum,HiCompatability,"");
                mn = 0x935076;
        }
        if(fd->disk_density == LOW_DENSITY)
        {
                Comp = diag_cat_gets(catd,setnum,LowCompatability,"");
                mn = 0x935077;
        }
        Warn = diag_cat_gets(catd,setnum, Warning_msg,"");
        if(fd->size == FIVE25)
        {
                Note_1 = diag_cat_gets(catd,setnum, Not_Protect_note_Five25,"");
                mn += 0x1000;
        }
        else
                Note_1 = diag_cat_gets(catd,setnum, Not_Protect_note_Three5,"");

        rc = diag_display(mn,catd , insert_disk_list , DIAG_MSGONLY,
                               ASL_DIAG_ENTER_SC,
                               &asl_menu_type,insert_menu);
        sprintf(Top_Text,insert_menu[0].text,Ds,da_input.dname,Mode) ;
        free(insert_menu[0].text);
        insert_menu[0].text = (char*)malloc(strlen(Top_Text) +1);
        strcpy(insert_menu[0].text, Top_Text );

        sprintf(Top_Text,insert_menu[1].text,
                  test,Comp,Warn,da_input.dname,Dt,
                  da_input.dname,
                  Door,Note_1);
        free(insert_menu[1].text);
        insert_menu[1].text = (char*)malloc(strlen(Top_Text) +1);
        strcpy(insert_menu[1].text, Top_Text ) ;
        rc = diag_display(mn,catd, NULL, DIAG_IO,
                               ASL_DIAG_ENTER_SC,
                               &asl_menu_type,insert_menu);
        if ( (da_input.exenv == EXENV_SYSX) &&
             ((rc == ASL_CANCEL) || (rc == ASL_EXIT)) )
                diag_asl_quit();
        chk_asl_return(rc);
}

/*
 * NAME: void READ_WRITE_RUNNING_message(void)
 *
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Called from do_fd_tests()
 *
 *
 * RETURNS: NONE
*/

/*............. E X A M P L E ....... S C R E E N ......................

TESTING IBM 3.5 Inch (DRIVE 0) DISKETTE DRIVE IN ADVANCED MODE  935090

Please Standby.

........................................................................ */

struct msglist RW_tests_running[] = {
        { Diskette_Diagnostics,Testing_msg},
        { Diskette_Diagnostics,Stand_by },
        (int)NULL
};
ASL_SCR_INFO RW_msg[DIAG_NUM_ENTRIES(RW_tests_running) ] ;

void READ_WRITE_RUNNING_message(void)
{
        int rc = 0;
        int mn;
        char Top_Text[1024];
        int setnum  = Diskette_Diagnostics;
        char *Ds    = DRIVE_size();
        char *Mode  = TEST_mode() ;
        mn = 0x935090;
        if(fd->size == FIVE25)
                mn = 0x936090;
        rc = diag_display(mn,catd ,RW_tests_running ,DIAG_MSGONLY,
                               ASL_DIAG_OUTPUT_LEAVE_SC,
                               &asl_menu_type,RW_msg);
        sprintf(Top_Text,RW_msg[0].text,Ds,da_input.dname,Mode);
        free(RW_msg[0].text);
        RW_msg[0].text = (char*)malloc(strlen( Top_Text )+1);
        strcpy( RW_msg[0].text, Top_Text );
        rc = diag_display(mn,catd, NULL , DIAG_IO,
                               ASL_DIAG_OUTPUT_LEAVE_SC,
                                &asl_menu_type,RW_msg);
        chk_asl_return(rc);
}

/*
 * NAME: void SELECT_TEST_message()
 *
 * FUNCTION: See the following EXAMPLE SCREEN
 *
 * EXECUTION ENVIRONMENT: Called from do_fd_tests()
 *
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
*/

/*............. E X A M P L E ....... S C R E E N ......................

TESTING IBM 3.5 Inch (DRIVE 0) DISKETTE DRIVE IN ADVANCED MODE  935010

Diskette Select/Deselect Tests

The following tests are used to insure the
selected Diskette Drive (DRIVE 0) is working.
When the test is started, the Diskette Drive (DRIVE 0) in-use light
will illuminate for approximately 5 seconds.
Observe the in-use light while press Enter to begin the tests:

........................................................................ */

struct msglist  select_list[]= {
        { Diskette_Diagnostics,Testing_msg },
        { Diskette_Diagnostics,Select_Test },
        (int)NULL
};
ASL_SCR_INFO sl_menu[DIAG_NUM_ENTRIES(select_list)];

void SELECT_TEST_message()
{
        int rc;                            /* return code                 */
        int mn;                            /* menu number                 */
        int setnum = Diskette_Diagnostics; /* catalog offset pointer      */
        char Top_Text[1024];
        char *Ds     =  DRIVE_size();
        char *Mode   =  TEST_mode();

        mn = 0x935010 ;              /* 3.5 inch DRIVE..  DEFAULT   */
        if(fd->size == FIVE25)
                mn = 0x936010;

        rc = diag_display(mn, catd,select_list, DIAG_MSGONLY,
                                ASL_DIAG_KEYS_ENTER_SC,
                                &asl_menu_type,sl_menu );
        sprintf(Top_Text,sl_menu[0].text,Ds,da_input.dname,Mode);

        free(sl_menu[0].text) ;
        sl_menu[0].text =(char *) malloc(strlen( Top_Text )+1);
        strcpy(sl_menu[0].text, Top_Text );

        sprintf(Top_Text,sl_menu[1].text,da_input.dname,da_input.dname);
        free(sl_menu[1].text);
        sl_menu[1].text =(char *) malloc(strlen(Top_Text)+1);
        strcpy(sl_menu[1].text,Top_Text);

        rc = diag_display(mn, catd ,NULL , DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC,
                                &asl_menu_type,sl_menu );
        chk_asl_return(rc);

}

/*
 * NAME: char *TEST_mode()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: called from all message & menu functions
 *
 *
 * RETURNS: Character string either 'ADVANCED MODE' or ''
*/

char *TEST_mode()
{
      int setnum = Diskette_Diagnostics;
	  char * tmp;
      char *mode;
      if(da_input.advanced == ADVANCED_TRUE )
            tmp = diag_cat_gets(catd,setnum,Advanced_Mode,"");
      else
             tmp = diag_cat_gets(catd,setnum,No_mode,"");

	  mode = (char *) malloc(strlen(tmp)+1);
	  strncpy(mode,tmp,strlen(tmp)+1);

      return(mode);
}


/*
 * NAME: char *DRIVE_size()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: called from all message & menu functions
 *
 *
 * RETURNS: Character string ' 3.5 Inch' or ' 5.25' Inch
*/
/* ............................................................... */
/* Develop the string for the Drive Size ( 3.5" or 5.25"           */
/* ............................................................... */

char *DRIVE_size()
{
        int setnum = Diskette_Diagnostics;
		char * tmp;
        char *Drive_Size;
        switch(fd->size)
        {
        case FIVE25:
                tmp = diag_cat_gets(catd,setnum, Five25,"");
                break;
        case THREE5:
                tmp = diag_cat_gets(catd, setnum, Three5 ,"");
                break;
        }
		Drive_Size = (char *) malloc((strlen(tmp) +1 ) );
		strncpy(Drive_Size,tmp,(strlen(tmp)+1));
        return(Drive_Size);
}

/*
 * NAME: char *DOOR_type()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: called from all message & menu functions
 *
 *
 * RETURNS: Character string ' Close the door' or ' '
*/
/* ............................................................... */
/* Develop the string for the Dor (Shut it or there isn't a door   */
/* ............................................................... */

char *DOOR_type()
{
        int setnum = Diskette_Diagnostics;
		char * tmp;
        char *Door_Type;

        switch(fd->size)
        {
        case FIVE25:
                tmp = diag_cat_gets(catd, setnum, Close_door, "");
                break;
        case THREE5:
				tmp = (char *)malloc(2);
                strncpy(tmp," \0",2);
                break;
        }
		Door_Type = (char *) malloc((strlen(tmp) + 1));
		strncpy(Door_Type,tmp,(strlen(tmp) +1));
        return(Door_Type);
}


/*
 * NAME: char *DISK_type()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: called from all message & menu functions
 *
 *
 * RETURNS:  Develop the string for the Disk Type (3.5" HI/LO Capacity
 *              or    5.25 HI/LO Capacity)
*/

char *DISK_type()
{
        int setnum = Diskette_Diagnostics;
        char *temp_buff = (char *)malloc(1024);
        char *Disk_Type;

       switch(fd->disk_density)
        {
        case HIGH_DENSITY_4MB:
                temp_buff = diag_cat_gets(catd,setnum,Disk_2880_NOPN);
                break;
        case HIGH_DENSITY:
                switch(fd->size)
                {
                case FIVE25:
                        temp_buff = diag_cat_gets(catd,setnum,Disk_1200,"");
                        break;
                case THREE5:
                        temp_buff = diag_cat_gets(catd,setnum,Disk_1440_NOPN,"");
                        break;
                }
                break;
        case LOW_DENSITY:
                switch(fd->size)
                {
                case FIVE25:
                        temp_buff = diag_cat_gets(catd,setnum,Disk_360,"");
                        break;
                case THREE5:
                        temp_buff = diag_cat_gets(catd,setnum,Disk_720_NOPN,"");
                        break;
                }
                break;
        }
		Disk_Type = (char * )malloc((strlen(temp_buff)+1));
		strncpy(Disk_Type,temp_buff,(strlen(temp_buff)+1));
        return(Disk_Type);
}

/*
 * NAME: char *DISK_type_insert()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: called from all message & menu functions
 *
 *
 * RETURNS:  Develop the string for the Disk Type (3.5" HI/LO Capacity
 *              or    5.25 HI/LO Capacity)
*/

char *DISK_type_insert()
{
        int setnum = Diskette_Diagnostics;
        char *temp_buff = (char *)malloc(1024);
        char *Disk_Type;

       switch(fd->disk_density)
        {
        case HIGH_DENSITY_4MB:
                temp_buff = diag_cat_gets(catd,setnum,Disk_2880_insert_NOPN);
                break;
        case HIGH_DENSITY:
                switch(fd->size)
                {
                case FIVE25:
                        temp_buff = diag_cat_gets(catd,setnum,
                                                  Disk_1200_insert,"");
                        break;
                case THREE5:
                        temp_buff = diag_cat_gets(catd,setnum,
                                              Disk_1440_insert_NOPN,"");
                        break;
                }
                break;
        case LOW_DENSITY:
                switch(fd->size)
                {
                case FIVE25:
                        temp_buff = diag_cat_gets(catd,setnum,
                                                  Disk_360_insert,"");
                        break;
                case THREE5:
                        temp_buff = diag_cat_gets(catd,setnum,
                                              Disk_720_insert_NOPN,"");
                        break;
                }
                break;
        }
		Disk_Type = (char *) malloc(strlen(temp_buff) + 1);
		strncpy(Disk_Type,temp_buff,(strlen(temp_buff) + 1));
        return(Disk_Type);
}
