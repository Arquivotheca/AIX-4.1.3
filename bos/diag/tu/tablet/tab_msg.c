/* static char sccsid[] = "@(#)15  1.4  src/bos/diag/tu/tablet/tab_msg.c, tu_tab, bos411, 9428A410j 5/12/94 15:13:24"; */
/*
 * COMPONENT_NAME: tu_tab
 *
 * FUNCTIONS: tab_msg.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "tu_type.h"

/*
 * NAME: putmsg
 *
 * FUNCTION:  Builds a title line that describes the adapter/device under test
 *            and asks the user to reply to a question.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: Return code from diag_display().
 */
int     putmsg(TUTYPE *tucb_ptr, long menu_num, unsigned short tm_3)
{
        char    msgstr[1024];
        long    rc = 0;
        struct  msglist da_menu[] =
        {
                { TABLET_DIAG, TM_1A },
                { TABLET_DIAG, TM_5  },
                { TABLET_DIAG, TM_6  },
		{ TABLET_DIAG, 0     },
                { (unsigned short)NULL,(unsigned short)NULL}
        };
        ASL_SCR_TYPE    menutype =
            {
                ASL_DIAG_ENTER_SC,3,1
            };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        da_menu[3].msgid = tm_3;
        memset (menu_da,0, sizeof (menu_da));
        if (tucb_ptr->tuenv.ad_mode == 1)
                da_menu[0].msgid = TM_2A;
        
	diag_display (menu_num, tucb_ptr->tuenv.catd,da_menu,DIAG_MSGONLY,
            ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
        sprintf (msgstr, menu_da[3].text,
            diag_cat_gets (tucb_ptr->tuenv.catd, TABLET_DIAG, TM_7));
        free (menu_da[3].text);
        menu_da[3].text = (char *) malloc (strlen(msgstr)+1);
        strcpy (menu_da[3].text, msgstr);
        rc = chk_stat(diag_display (menu_num,tucb_ptr->tuenv.catd,
                        NULL,DIAG_IO,ASL_DIAG_LIST_CANCEL_EXIT_SC,
      			&menutype, menu_da));
 
	if (rc != 0)
                return(rc);
        else
           if (TRUE)
                return(menutype.cur_index);
           else
                return(0);

}  /* putmsg end */


/*
 * NAME:
 *      chk_stat
 *
 * FUNCTION:
 *      Handles the return code from an asl or diag_asl procedure call.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called by the putmsg procedure. 
 *
 * RETURNS:
 *      0 if okay.
 *      -1 if menu fails to display.
 *      EXIT_KEY_ENTERED if user presses exit key.
 *      CANCEL_KEY_ENTERED if user presses cancel key.
 */

int	chk_stat( long returned_code)
{
        if ( returned_code == DIAG_ASL_CANCEL )
                return( CANCEL_KEY_ENTERED );
        else
                if ( returned_code == DIAG_ASL_EXIT )
                        return( EXIT_KEY_ENTERED );
                else
                        if ( returned_code == DIAG_MALLOCFAILED )
                                return(-1);
                        else
                                return(0);
}
/*
 * NAME:
 *       setmenunumberbase
 *
 * FUNCTION:
 *      This function alters the menu number values for the global
 *      message lists when running with different tablets.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called as a function by main.
 *
 * (NOTES:)
 *      This function:
 *      Sets the msgnum to the value for the type of tablet
 *      being tested.
 *
 * (RECOVERY OPERATION:)
 *      None needed.
 *
 * (DATA STRUCTURES:)
 *      This function modifies the following global structures and variables:
 *
 * RETURNS:
 *      None.
 */
void    setmenunumberbase(int tabtype)
{
        int     i;

        for ( i = 0; i < 8; i++ )
                msgtab[i] = menunums[tabtype][i];
}

/*
 * NAME: dsply_test_msg
 *
 * FUNCTION: Builds a title line that describes the adapter/device under test,
 *           and explains the test setup requirements.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: Return code from diag_display().
 */

int	dsply_test_msg (TUTYPE *tucb_ptr, long msgnum, unsigned short tm_1)
{
	char    msgstr[1024];
	int    rc = 0;
        struct  msglist da_menu[] =
        {
                { TABLET_DIAG, TM_1A, },
                { TABLET_DIAG, 0,     },
                { TABLET_DIAG, TM_9,  },
                {(unsigned short)NULL, (unsigned short)NULL,},
        };
        ASL_SCR_TYPE    menutype =
            {
                ASL_DIAG_ENTER_SC,2,1
        };
        ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];

        da_menu[1].msgid = tm_1;
        memset (menu_da,0, sizeof (menu_da));
        if (tucb_ptr->tuenv.ad_mode == 1)
	       da_menu[0].msgid = TM_2A;

        diag_display (msgnum, tucb_ptr->tuenv.catd, da_menu, DIAG_MSGONLY,
            ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
        rc = chk_stat(diag_display (msgnum, tucb_ptr->tuenv.catd, NULL, 
		DIAG_IO, ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da));
        if (rc != 0)
		return(rc);
	return(rc);
}

/*
 * NAME:
 *      open_kbd
 *
 * FUNCTION:
 *      This function opens device before executing test unit and
 *      sets diagnostic mode to enable.
 *
 */

open_kbd(TUTYPE *tucb_ptr)
{
    int rc;
    uint arg;

       /* in case that the tablet test code is executed using a serial terminal
          and the systems does not have a console keyboard */
          
       if(tucb_ptr->tuenv.kbd_fd == NULL)
	  return(SUCCESS);

       if((kbdtufd = open(tucb_ptr->tuenv.kbd_fd, O_RDWR)) < 0)
          return(FAILURE);

       arg = KSDENABLE;
       rc = ioctl(kbdtufd, KSDIAGMODE, &arg);
       if (rc != SUCCESS)
          return(FAILURE);
    return(SUCCESS);
}
