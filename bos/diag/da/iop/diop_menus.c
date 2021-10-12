static char sccsid[] = "@(#)64	1.4.1.3  src/bos/diag/da/iop/diop_menus.c, daiop, bos411, 9428A410j 4/14/94 10:12:04";
/*
 *   COMPONENT_NAME: DAIOP
 *
 *   FUNCTIONS: DEAD_BATTERY_TEST_menu
 *		DIAG_NUM_ENTRIES
 *		DIOP_LOOP_TEST_message
 *		IOP_TESTING_message
 *		LED_TEST_menu
 *		LED_TEST_message
 *		TEST_mode
 *		TOD_SET_message
 *		key_lock_test_start_message
 *		key_position_message
 *		LCD_TEST_menu
 *		LCD_TEST_message
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


#include "iop.h"
#include "diag/diago.h"         /* ASL functions                        */
#include "diop_msg.h"           /* message catalog numbers              */
#include "diag/tmdefs.h"
#include "diag/tm_input.h"

extern struct tm_input da_input;
extern nl_catd catd;
extern char *diag_cat_gets();
extern void chk_asl_return();
char *TEST_mode();


struct msglist IOP_list[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, Stand_by },
        NULL
};


ASL_SCR_INFO IOP_TESTING_msg[ DIAG_NUM_ENTRIES(IOP_list) ];
ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

/* .............. E X A M P L E       S C R E E N  ........................

TESTING IO PLANAR - ADVANCED MODE                     84100


Please standby............



.......................................................................... */


void IOP_TESTING_message(void)
{
        int rc;
        int mn = 0x814000;
        char Top_text[1024];
        char *Mode = TEST_mode();
        rc = diag_display(mn, catd, IOP_list,DIAG_MSGONLY ,
                            ASL_DIAG_OUTPUT_LEAVE_SC  ,
                            &menutype,IOP_TESTING_msg);
       sprintf(Top_text,IOP_TESTING_msg[0].text,da_input.dname,Mode);
       free(IOP_TESTING_msg[0].text);
       IOP_TESTING_msg[0].text = (char*)malloc(sizeof(Top_text)+1);
       strcpy(IOP_TESTING_msg[0].text,Top_text);
       rc = diag_display(mn, catd, NULL, DIAG_IO,
                            ASL_DIAG_OUTPUT_LEAVE_SC,
                            &menutype,IOP_TESTING_msg);
       chk_asl_return(rc);
}

/*
 *  NOTE: THE EXAMPLE SCREENS SHOW BELOW ONLY REFLECT THE BASIC
 *       CONCEPTS OF THE MESSAGE & MENUS.
 *       SEE:  diop.msg (source) & diop_msg.h (defines)
 *           (generates   /etc/lpp/diagnostics/catalog/$LANG/diop.cat
 *
*/



struct msglist iop_loop_list[]= {
        {IO_Planar_Diagnostics,TESTING_msg },
        {IO_Planar_Diagnostics,Loop_msg },
        NULL
};


ASL_SCR_INFO iop_loop_menu[DIAG_NUM_ENTRIES(iop_loop_list)];


/* .............. E X A M P L E       S C R E E N  ........................

TESTING IO PLANAR - ADVANCED MODE                   814090


xxx passes completed
xxx errors logged

Testing can be stopped by press the End key.

.......................................................................... */


void DIOP_LOOP_TEST_message(void)
{
        int rc;                            /* return code                 */
        int mn = 0x814090;                 /* menu number                 */
        char Top_Text[1024];
        char *Mode = TEST_mode();
        rc = diag_display(mn, catd,iop_loop_list, DIAG_MSGONLY,
                                ASL_DIAG_OUTPUT_LEAVE_SC,
                                &menutype,iop_loop_menu );

        sprintf(Top_Text, iop_loop_menu[0].text,da_input.dname,Mode);
        free(iop_loop_menu[0].text);
        iop_loop_menu[0].text = (char*)malloc(sizeof(Top_Text)+1);
        strcpy(iop_loop_menu[0].text,Top_Text);

        sprintf(Top_Text,iop_loop_menu[1].text ,
                              da_input.lcount,da_input.lerrors);
        free(iop_loop_menu[1].text);
        iop_loop_menu[1].text =(char *) malloc(strlen(Top_Text)+1);
        strcpy(iop_loop_menu[1].text,Top_Text);
        rc = diag_display(mn, catd ,NULL , DIAG_IO,
                          ASL_DIAG_OUTPUT_LEAVE_SC,
                           &menutype,iop_loop_menu );
        chk_asl_return(rc);

}


struct msglist lock_position_list[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, Keylock_Testing },
        NULL
};

struct msglist lock_test_start_list[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, Keylock_Start },
        NULL
};


ASL_SCR_INFO lock_test_start_msg[ DIAG_NUM_ENTRIES(lock_test_start_list) ];
ASL_SCR_INFO lock_position_msg[ DIAG_NUM_ENTRIES(lock_position_list) ];



/* .............. E X A M P L E       S C R E E N  ........................

TESTING IO PLANAR - ADVANCED MODE                   814080

       Operator Panel Switch Test

The following tests will be used to ensure the Mode Switch
in the operator panel is working properly.
You will be asked to move the Mode Swith to
various functional positions.

When read press Enter to start the tests.

.......................................................................... */


void key_lock_test_start_message(void)
{
        int rc;
        char Top_text[1024];
        char *Mode = TEST_mode();
        rc = diag_display(0x814080,catd,lock_test_start_list, DIAG_MSGONLY,
                          ASL_DIAG_KEYS_ENTER_SC,
                          &menutype,lock_test_start_msg );
        sprintf(Top_text,lock_test_start_msg[0].text,da_input.dname,Mode ) ;
        free(lock_test_start_msg[0].text);
        lock_test_start_msg[0].text = (char *)malloc(strlen(Top_text)+1);
        strcpy(lock_test_start_msg[0].text,Top_text);

        rc = diag_display(0x814080,catd,NULL, DIAG_IO,
                          ASL_DIAG_KEYS_ENTER_SC,
                          &menutype,lock_test_start_msg );
        chk_asl_return(rc);
}


/* .............. E X A M P L E       S C R E E N  ........................

TESTING IO PLANAR - ADVANCED MODE                   814081

      Operator Panel Switch Test

TURN ....... the key to the SERVICE position.


Press enter to continue.

.......................................................................... */



void key_position_message(key_p)
{
        int rc;
        int setnum = IO_Planar_Diagnostics;
        char Top_text[1024];
        char *Mode = TEST_mode();

        int mn = 0x814080 +key_p;

        rc = diag_display(mn,catd,lock_position_list,
                                DIAG_MSGONLY,
                                ASL_DIAG_KEYS_ENTER_SC,
                                &menutype,lock_position_msg);
        sprintf(Top_text,lock_position_msg[0].text,da_input.dname,Mode ) ;
        free(lock_position_msg[0].text);
        lock_position_msg[0].text = (char *)malloc(strlen(Top_text)+1);
        strcpy(lock_position_msg[0].text,Top_text);

        switch(key_p)
           {
        case SERVICE_KEY_POSITION:
                sprintf(Top_text,lock_position_msg[1].text,
                               diag_cat_gets(catd,setnum,Service,""));
                break;
        case NORMAL_KEY_POSITION:
                sprintf(Top_text,lock_position_msg[1].text,
                              diag_cat_gets(catd,setnum,Normal,""));
                break;
        case LOCKED_KEY_POSITION:
                sprintf(Top_text,lock_position_msg[1].text,
                             diag_cat_gets(catd,setnum,Secure,""));
                break;
        }
        free(lock_position_msg[1].text);
        lock_position_msg[1].text =(char *) malloc(strlen(Top_text)+1);
        strcpy(lock_position_msg[1].text,Top_text);

        rc = diag_display(mn, catd, NULL, DIAG_IO,
                            ASL_DIAG_KEYS_ENTER_SC,
                             &menutype,lock_position_msg);
        chk_asl_return(rc);

}

/* .............. E X A M P L E       S C R E E N  ........................

TESTING IO PLANAR - ADVANCED MODE                   814020

Has the battery recently been replaced or unplugged while the power
to the system was OFF?

Move cursor to selection, then press ENTER.

.......................................................................... */

struct msglist DEAD_BATTERY_list[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, Yes_option },
        { IO_Planar_Diagnostics, No_option },
        { IO_Planar_Diagnostics, NVRAM_TST },
        NULL
};

ASL_SCR_INFO DEAD_BATTERY_menu[ DIAG_NUM_ENTRIES(DEAD_BATTERY_list) ];

int DEAD_BATTERY_TEST_menu(void)
{
        int rc;
        int mn = 0x814020;
        char Top_text[1024];
        char *Mode = TEST_mode();
        rc = diag_display(mn,catd, DEAD_BATTERY_list, DIAG_MSGONLY ,
                           ASL_DIAG_LIST_CANCEL_EXIT_SC,
                           &menutype, DEAD_BATTERY_menu );
        sprintf(Top_text,DEAD_BATTERY_menu[0].text,da_input.dname,Mode);
        free(DEAD_BATTERY_menu[0].text);
        DEAD_BATTERY_menu[0].text = (char *)malloc(strlen(Top_text) +1);
        strcpy(DEAD_BATTERY_menu[0].text,Top_text);
        rc = diag_display(mn,catd,NULL,DIAG_IO,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC,
                          &menutype,DEAD_BATTERY_menu);
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(menutype);
        return(rc);
}

struct msglist LED_list[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, Yes_option },
        { IO_Planar_Diagnostics, No_option },
        { IO_Planar_Diagnostics, LED_Test_End },
        NULL
};

struct msglist LED_msg[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, LED_Test_Start },
        NULL
};


ASL_SCR_INFO LED_menu[ DIAG_NUM_ENTRIES(LED_list) ];
ASL_SCR_INFO LED_message[ DIAG_NUM_ENTRIES(LED_msg) ];

/* .............. E X A M P L E       S C R E E N  ........................

TESTING IO PLANAR - ADVANCED MODE                   816140

         Operator Panel  3-Digit Display Test

The following tests ensure that the 3-Digit Display
in the operator panel is working properly.

When the test is started, the 3 digist will display 666 and 999
for about 5 seconds each.

Observe the 3-Digit Display and press Enter to start the tests.
.......................................................................... */


void LED_TEST_message(void)
{
        int rc;
        int mn = 0x816140;
        char Top_text[1024];
        char *Mode = TEST_mode();

        rc = diag_display(mn, catd, LED_msg, DIAG_MSGONLY,
                                   ASL_DIAG_KEYS_ENTER_SC,
                                   &menutype, LED_message);
        sprintf(Top_text,LED_message[0].text,da_input.dname,Mode);
        free(LED_message[0].text);
        LED_message[0].text = (char *)malloc(strlen(Top_text) +1);
        strcpy(LED_message[0].text,Top_text);
        rc = diag_display(mn, catd, NULL, DIAG_IO,
                              ASL_DIAG_KEYS_ENTER_SC,
                              &menutype,LED_message);
        chk_asl_return(rc);
}


/* .............. E X A M P L E       S C R E E N  ........................

TESTING IO PLANAR - ADVANCED MODE                   816159

         Operator Panel  3-Digit Display Test

Did the values 666 and 999 show up on the 3-Digit display?

Move cursor to selection, then press Enter:

.......................................................................... */



int LED_TEST_menu(void)
{
        int rc;
        int mn = 0x816159;
        char Top_text[1024];
        char *Mode = TEST_mode();
        rc = diag_display(mn,catd, LED_list, DIAG_MSGONLY ,
                           ASL_DIAG_LIST_CANCEL_EXIT_SC,
                           &menutype, LED_menu );
        sprintf(Top_text,LED_menu[0].text,da_input.dname,Mode);
        free(LED_menu[0].text);
        LED_menu[0].text = (char *)malloc(strlen(Top_text) +1);
        strcpy(LED_menu[0].text,Top_text);
        rc = diag_display(mn,catd,NULL,DIAG_IO,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC,
                          &menutype,LED_menu);
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(menutype);
        return(rc);
}




struct msglist TOD_set_list[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, TOD_Set_msg },
        NULL
};

ASL_SCR_INFO TOD_set_message[ DIAG_NUM_ENTRIES(TOD_set_list) ];


/* .............. E X A M P L E       S C R E E N  ........................

TESTING IO PLANAR - ADVANCED MODE                   816013

        Time of Day Reset
A previous test has set the time to January 1, 1989.

The time and date should be reset to the current time after
exiting these diagnostics tests.

Press Enter to continue.

.......................................................................... */

void TOD_SET_message(void)
{
        int rc;
        int mn = 0x816013;
        char Top_text[1024];
        char *Mode = TEST_mode();

        rc = diag_display(mn, catd,TOD_set_list , DIAG_MSGONLY ,
                            ASL_DIAG_KEYS_ENTER_SC,
                           &menutype, TOD_set_message);
        sprintf(Top_text,TOD_set_message[0].text,da_input.dname,Mode);
        free(TOD_set_message[0].text);
        TOD_set_message[0].text = (char*)malloc(strlen(Top_text)+1);
        strcpy(TOD_set_message[0].text,Top_text);
        rc = diag_display(mn, catd, NULL ,DIAG_IO,
                            ASL_DIAG_KEYS_ENTER_SC,
                           &menutype,TOD_set_message);
       chk_asl_return(rc);
}

/*
NAME:       TEST_mode()

FUNCTION:   a generic function to generate the ADVANCED MODE string

RETURNS:    char string ADVANCED MODE or ""


*/


char *TEST_mode()
      {
      int setnum = IO_Planar_Diagnostics;
      char *mode = (char *)malloc(A_LINE);
      if(da_input.advanced == ADVANCED_TRUE )
            mode = diag_cat_gets(catd,setnum,Advanced_Mode,"");
      else
             mode = diag_cat_gets(catd,setnum,No_Mode,"");
      return(mode);
      }


struct msglist LCD_list[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, Yes_option },
        { IO_Planar_Diagnostics, No_option },
        { IO_Planar_Diagnostics, LCD_Test_End },
        NULL
};

struct msglist LCD_msg[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, LCD_Test_Start },
        NULL
};


ASL_SCR_INFO LCD_menu[ DIAG_NUM_ENTRIES(LCD_list) ];
ASL_SCR_INFO LCD_message[ DIAG_NUM_ENTRIES(LCD_msg) ];

/* .............. E X A M P L E       S C R E E N  ........................

TESTING IO PLANAR - ADVANCED MODE                   816200

Operator Panel Liquid Crystal Display Test

The following test will be used to ensure the Liquid Crystal Display
in the operator panel is working properly.

When the test is started, the LCD will display U*U* and VAS
for approximately 5 seconds each.

OBSERVE........the Liquid Crystal Display after starting the test.

To start the test, press Enter.
.......................................................................... */


void LCD_TEST_message(void)
{
        int rc;
        int mn = 0x816200;
        char Top_text[1024];
        char *Mode = TEST_mode();

        rc = diag_display(mn, catd, LCD_msg, DIAG_MSGONLY,
                                   ASL_DIAG_KEYS_ENTER_SC,
                                   &menutype, LCD_message);
        sprintf(Top_text,LCD_message[0].text,da_input.dname,Mode);
        free(LCD_message[0].text);
        LCD_message[0].text = (char *)malloc(strlen(Top_text) +1);
        strcpy(LCD_message[0].text,Top_text);
        rc = diag_display(mn, catd, NULL, DIAG_IO,
                              ASL_DIAG_KEYS_ENTER_SC,
                              &menutype,LCD_message);
        chk_asl_return(rc);
}


/* .............. E X A M P L E       S C R E E N  ........................

TESTING IO PLANAR - ADVANCED MODE                   816201

Operator Panel Liquid Crystal Display Test

Did the values U*U* and VAS show up in the Liquid Crystal Display?

Move cursor to selection, then press Enter:

.......................................................................... */



int LCD_TEST_menu(void)
{
        int rc;
        int mn = 0x816201;
        char Top_text[1024];
        char *Mode = TEST_mode();
        rc = diag_display(mn,catd, LCD_list, DIAG_MSGONLY ,
                           ASL_DIAG_LIST_CANCEL_EXIT_SC,
                           &menutype, LCD_menu );
        sprintf(Top_text,LCD_menu[0].text,da_input.dname,Mode);
        free(LCD_menu[0].text);
        LCD_menu[0].text = (char *)malloc(strlen(Top_text) +1);
        strcpy(LCD_menu[0].text,Top_text);
        rc = diag_display(mn,catd,NULL,DIAG_IO,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC,
                          &menutype,LCD_menu);
        chk_asl_return(rc);
        rc = DIAG_ITEM_SELECTED(menutype);
        return(rc);
}
