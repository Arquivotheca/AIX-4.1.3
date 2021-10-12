static char sccsid[] = "@(#)61	1.2.2.2  src/bos/diag/da/iopsl/diopsl_mn.c, daiopsl, bos411, 9428A410j 7/13/93 14:07:54";
/*
 * COMPONENT_NAME: DAIOPSL
 *
 * FUNCTIONS:  void IOP_TESTING_message()
 *             void DIOP_LOOP_TESTING_message()
 *             void key_lock_test_start_message()
 *             void key_position_message()
 *             void LED_TEST_message()
 *             int LED_TEST_menu()
 *             int DEAD_BATTERY_TEST_menu()
 *             void TOD_TEST_message()
 *             char *TEST_mode()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "diopsl.h"
#include "diag/diago.h"         /* ASL functions                        */
#include "diop_msg.h"           /* message catalog numbers              */
#include "diag/tmdefs.h"
#include "diag/tm_input.h"

extern char *diag_cat_gets();
extern struct tm_input da_input;
extern nl_catd catd;

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
       NLsprintf(Top_text,IOP_TESTING_msg[0].text,da_input.dname,Mode);
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

        NLsprintf(Top_Text, iop_loop_menu[0].text,da_input.dname,Mode);
        free(iop_loop_menu[0].text);
        iop_loop_menu[0].text = (char*)malloc(sizeof(Top_Text)+1);
        strcpy(iop_loop_menu[0].text,Top_Text);

        NLsprintf(Top_Text,iop_loop_menu[1].text ,
                              da_input.lcount,da_input.lerrors);
        free(iop_loop_menu[1].text);
        iop_loop_menu[1].text =(char *) malloc(strlen(Top_Text)+1);
        strcpy(iop_loop_menu[1].text,Top_Text);
        rc = diag_display(mn, catd ,NULL , DIAG_IO,
                          ASL_DIAG_OUTPUT_LEAVE_SC,
                           &menutype,iop_loop_menu );
        chk_asl_return(rc);

}


/*struct msglist lock_position_list[] = {
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
*/

struct msglist switch_position_list[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, SWITCH_TESTING },
        NULL
};

struct msglist switch_test_start_list[] = {
        { IO_Planar_Diagnostics, TESTING_msg },
        { IO_Planar_Diagnostics, SWITCH_START },
        NULL
};


ASL_SCR_INFO switch_test_start_msg[ DIAG_NUM_ENTRIES(switch_test_start_list) ];
ASL_SCR_INFO switch_position_msg[ DIAG_NUM_ENTRIES(switch_position_list) ];



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
		extern int mach_type;
        int rc;
        int setnum = IO_Planar_Diagnostics;
		char * k_or_s;  /* key or switch */
        char *Top_text[1024];
        char *Mode = TEST_mode();
        rc = diag_display(0x814080,catd,switch_test_start_list, DIAG_MSGONLY,
                          ASL_DIAG_KEYS_ENTER_SC,
                          &menutype,switch_test_start_msg );

        sprintf(Top_text,switch_test_start_msg[0].text,da_input.dname,Mode ) ;
        free(switch_test_start_msg[0].text);
        switch_test_start_msg[0].text = (char *)malloc(strlen(Top_text)+1);
        strcpy(switch_test_start_msg[0].text,Top_text);

		/* 
		 * Determine if this machine has a keylock
		 */
		if(mach_type != NO_KEY_LOCK )
		{
			k_or_s = diag_cat_gets( catd,setnum,KEY);
		}
		else
		{
			k_or_s = diag_cat_gets( catd,setnum,NO_KEY);
		}

        sprintf(Top_text,switch_test_start_msg[1].text,k_or_s,k_or_s);
        free(switch_test_start_msg[1].text);
        switch_test_start_msg[1].text = (char *)malloc(strlen(Top_text)+1);
        strcpy(switch_test_start_msg[1].text,Top_text);


        rc = diag_display(0x814080,catd,NULL, DIAG_IO,
                          ASL_DIAG_KEYS_ENTER_SC,
                          &menutype,switch_test_start_msg );
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
		int cuat_mod;
		char * turn;  /* hold data for key or null and turn or move */
		char * type;
        int setnum = IO_Planar_Diagnostics;
        char Top_text[1024];
        char *Mode = TEST_mode();
		char * position;
		char * key_or_switch;
		extern int mach_type;

        int mn = 0x814080 +key_p;

        rc = diag_display(mn,catd,switch_position_list,
                                DIAG_MSGONLY,
                                ASL_DIAG_KEYS_ENTER_SC,
                                &menutype,switch_position_msg);
        sprintf(Top_text,switch_position_msg[0].text,da_input.dname,Mode ) ;

        free(switch_position_msg[0].text);
        switch_position_msg[0].text = (char *)malloc(strlen(Top_text)+1);
        strcpy(switch_position_msg[0].text,Top_text);

		if(mach_type != NO_KEY_LOCK )
		{
			type = diag_cat_gets( catd,setnum,KEY);
			turn = diag_cat_gets(catd,setnum,TURN);
			key_or_switch = type;
		}
		else
		{
			type = diag_cat_gets( catd,setnum,NO_KEY);
			turn = diag_cat_gets(catd,setnum,MOVE);
			key_or_switch = diag_cat_gets(catd,setnum,SWITCH);
		}



        switch(key_p)
        {
        	case SERVICE_KEY_POSITION:
				position = diag_cat_gets(catd,setnum,Service);
                break;
        	case NORMAL_KEY_POSITION:
				position = diag_cat_gets(catd,setnum,Normal);
                break;
        	case LOCKED_KEY_POSITION:
				position = diag_cat_gets(catd,setnum,Secure);
                break;
        }

        sprintf(Top_text,switch_position_msg[1].text,
					type,turn,key_or_switch,position);

        free(switch_position_msg[1].text);
        switch_position_msg[1].text = (char *)malloc(strlen(Top_text)+1);
        strcpy(switch_position_msg[1].text,Top_text);

        rc = diag_display(mn, catd, NULL, DIAG_IO,
                            ASL_DIAG_KEYS_ENTER_SC,
                             &menutype,switch_position_msg);
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
        NLsprintf(Top_text,DEAD_BATTERY_menu[0].text,da_input.dname,Mode);
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
        NLsprintf(Top_text,LED_message[0].text,da_input.dname,Mode);
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
        NLsprintf(Top_text,LED_menu[0].text,da_input.dname,Mode);
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
        NLsprintf(Top_text,TOD_set_message[0].text,da_input.dname,Mode);
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
            mode = NLcatgets(catd,setnum,Advanced_Mode,"");
      else
             mode = NLcatgets(catd,setnum,No_Mode,"");
      return(mode);
      }

