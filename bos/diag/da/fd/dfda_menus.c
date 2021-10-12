static char sccsid[] = "@(#)61	1.5  src/bos/diag/da/fd/dfda_menus.c, dafd, bos411, 9428A410j 12/17/92 10:57:39";
/*
 *   COMPONENT_NAME: dafd
 *
 *   FUNCTIONS: DFDA_LOOP_TEST_message
 *		DIAG_NUM_ENTRIES
 *		TEST_mode
 *		disp_fda_menu
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
#include "diag/tmdefs.h"
#include "diag/tm_input.h"
#include "dfd_msg.h"		/* message catalog numbers		*/
#include "fd_set.h"

extern nl_catd catd;
extern struct fd_drive *fd;
extern struct tm_input da_input;
extern char *diag_cat_gets();
extern chk_asl_return();
char *TEST_mode();


/*
 * NAME: void disp_fda_menu()
 *                                                                    
 * FUNCTION: Display a message showing the user that the adapter is being  
 *           tested. 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *
 * RETURNS: 
*/

struct msglist fda_list[] = {
                           { Diskette_Diagnostics, Diskette_Adapter },
                           { Diskette_Diagnostics, Stand_by         },
                            (int)NULL
}; 

ASL_SCR_INFO fda_menu[DIAG_NUM_ENTRIES(fda_list)];
ASL_SCR_TYPE asl_menu_type = DM_TYPE_DEFAULTS;


void disp_fda_menu()
{
	int rc;
        char Top_Text[1024];
        char *Mode = TEST_mode();
        rc = diag_display(0x828000, catd, fda_list, DIAG_MSGONLY,
                              ASL_DIAG_OUTPUT_LEAVE_SC,
                              &asl_menu_type,fda_menu);
        chk_asl_return(rc);
        sprintf(Top_Text,fda_menu[0].text,Mode);
        free(fda_menu[0].text);
        fda_menu[0].text = (char *)malloc(strlen(Top_Text)+1);
        strcpy(fda_menu[0].text,Top_Text);
        rc = diag_display(0x828000, catd, NULL, DIAG_IO,
                              ASL_DIAG_OUTPUT_LEAVE_SC,
                              &asl_menu_type,fda_menu);
        chk_asl_return(rc);
}

/* end disp_fda_menu */

/*
 * NAME: void DFDA_LOOP_TEST_message()
 *                                                                    
 * FUNCTION: Display the number of test passes & the number of errors logged 
 *                                                                    
 * EXECUTION ENVIRONMENT: called by main()   
 *                        SEE: fda_main.c
 *                                                                   
 *
 * RETURNS: NONE 
*/

struct msglist dfda_loop_list[]= {
	{ Diskette_Diagnostics,Diskette_Adapter },
	{ Diskette_Diagnostics,Loop_msg },
	(int)NULL
};

ASL_SCR_INFO dfda_loop_menu[DIAG_NUM_ENTRIES(dfda_loop_list)];

void DFDA_LOOP_TEST_message()
{
	int rc;                            /* return code                 */
	int mn;                            /* menu number                 */
	char Top_Text[1024];
	char *Mode = TEST_mode();
	mn = 0x935020 ;              /* 3.5 inch DRIVE..  DEFAULT   */
	if(fd->size == FIVE25)
		mn = 0x936020;
	rc = diag_display(mn, catd,dfda_loop_list, DIAG_MSGONLY,
	                      ASL_DIAG_OUTPUT_LEAVE_SC,
	                      &asl_menu_type,dfda_loop_menu );
	sprintf(Top_Text,dfda_loop_menu[0].text,Mode);
	free(dfda_loop_menu[0].text) ;
	dfda_loop_menu[0].text =(char *) malloc(strlen(Top_Text)+1);
	strcpy(dfda_loop_menu[0].text,Top_Text);

	sprintf(Top_Text,dfda_loop_menu[1].text , 
	                            da_input.lcount,da_input.lerrors);
	free(dfda_loop_menu[1].text);
	dfda_loop_menu[1].text =(char *) malloc(strlen(Top_Text)+1);
	strcpy(dfda_loop_menu[1].text,Top_Text);

	rc = diag_display(mn, catd ,NULL , DIAG_IO, 
	                       ASL_DIAG_OUTPUT_LEAVE_SC,
	                       &asl_menu_type,dfda_loop_menu );
	chk_asl_return(rc);

}


/*
 * NAME: char *TEST_mode()
 *                                                                    
 * FUNCTION: To return a string to indicate the test mode 
 *                                                                    
 * EXECUTION ENVIRONMENT: called by all menu & message displays functions
 *                                                                   
 *
 * RETURNS: char string "ADVANCED MODE" or " " 
*/

char *TEST_mode()
      {
      int setnum = Diskette_Diagnostics;
      char *mode = (char *)malloc(A_LINE);
      if(da_input.advanced == ADVANCED_TRUE )
            mode = diag_cat_gets(catd,setnum,Advanced_Mode,"");
      else           
             mode = diag_cat_gets(catd,setnum,No_mode,"");
      return(mode);
      }


