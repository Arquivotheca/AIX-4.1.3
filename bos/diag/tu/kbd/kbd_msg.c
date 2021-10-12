/* static char sccsid[] = "@(#)93  1.4  src/bos/diag/tu/kbd/kbd_msg.c, tu_kbd, bos411, 9428A410j 5/20/94 07:26:08"; */
/*
 * COMPONENT_NAME: tu_kbd
 *
 * FUNCTIONS:   
 *            
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
 * NAME:
 *       putmsg
 *
 * FUNCTION:
 *      This function displays a menu of some type dependent on an input
 *      parameter.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called as a function by main.
 *
 * (NOTES:)
 *      This function:
 *      Displays the requested menu, waiting for user response if it is
 *      required.
 *
 * (RECOVERY OPERATION:)
 *      Software errors are handled by the call to the chk_stat function.
 *
 * (DATA STRUCTURES:)
 *      None altered.
 *
 * RETURNS:
 *      0 if everything is okay.
 *      -1 in the event that the menu was not able to display.
 *      EXIT_KEY_ENTERED if user presses exit key.
 *      CANCEL_KEY_ENTERED if user presses cancel key.
 */ 

putmsg(TUTYPE *tucb_ptr, struct msgtable *mtp )
{
    int     rc;

    menutypes.cur_index = YES;
    
    switch ( mtp->msgtype )
    {
    case 0:
        rc = chk_stat ( diag_display
                      ( mtp->msgnum                  /* menu_number */
                      , tucb_ptr->tuenv.catd         /* cat_fdes */
                      , mtp->mlp                     /* msglist[] */
                      , DIAG_IO                      /* proctype */
                      , ASL_DIAG_LIST_CANCEL_EXIT_SC /* screen type */
                      , &menutypes                   /* menutype */
                      , NULL                         /* menuinfo */
                      ));

        if ( rc == 0 )
            return(menutypes.cur_index);
        break;
        
    case 1:
        rc = chk_stat ( diag_display
		      ( mtp->msgnum
                      , tucb_ptr->tuenv.catd
                      , mtp->mlp
                      , DIAG_IO
                      , ASL_DIAG_KEYS_ENTER_SC
                      , &menutypes
                      , NULL
                      ));
        break;
        
    case 2:
        rc = chk_stat ( diag_display
		      ( mtp->msgnum
                      , tucb_ptr->tuenv.catd
                      , mtp->mlp
                      , DIAG_IO
                      , ASL_DIAG_LEAVE_NO_KEYS_SC
                      , NULL
                      , NULL
                      ));
        break;
        
    default:
        break;
    }

    return(rc);
}

/*
 * NAME:
 *      chk_stat
 *
 * FUNCTION:
 *      Handles the return code from an asl or diag_asl procedure call.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called by the putmsg procedure which is called as
 *      a function from main.
 *
 * (NOTES:)
 *      This function acts on the return code from an asl or diag_asl
 *      procedure call.
 *
 * (RECOVERY OPERATION:)
 *      None.
 *
 * (DATA STRUCTURES:)
 *      None.
 *
 * RETURNS:
 *      0 if okay.
 *      -1 if menu fails to display.
 *      EXIT_KEY_ENTERED if user presses exit key.
 *      CANCEL_KEY_ENTERED if user presses cancel key.
 */

chk_stat(long returned_code)
{
    switch (returned_code)
    {
    case DIAG_ASL_CANCEL:
        return( CANCEL_KEY_ENTERED );
	break;
	
    case DIAG_ASL_EXIT:
	return( EXIT_KEY_ENTERED );
	break;
	
    case DIAG_MALLOCFAILED:
	return(-1);
	break;
	
    default:
	return(0);
	break;
    }
}
/*
 * NAME:
 *       setmenunumberbase
 *
 * FUNCTION:
 *      This function alters the menu number values for the global
 *      message lists when running with different keyboards.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called as a function by main.
 *
 * (NOTES:)
 *      This function:
 *      Sets the msgtab msgnum to the value for the type of keyboard
 *      being tested.
 *
 * (RECOVERY OPERATION:)
 *      None needed.
 *
 * (DATA STRUCTURES:)
 *      This function modifies the following global structures and variables:
 *      All msgtab msgnum values.
 *
 * RETURNS:
 *      None.
 */

void    setmenunumberbase(kbtype)
int     kbtype;
{
        int     i;
        ulong   modifier;

        for ( i=0; i< (sizeof msgtab / sizeof(struct msgtable)); ++i )
                msgtab[i].msgnum = menunums[kbtype][i];
}

/*
 * NAME:
 *       maketitleadvanced
 *
 * FUNCTION:
 *      This function alters the message id values for the global
 *      message lists when running in advanced mode.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called as a function by main.
 *
 * (NOTES:)
 *      This function:
 *      Sets the msglist msgids to the value for TESTAM, which will
 *      cause the title Testing in advanced mode to be displayed instead
 *      of just Testing.
 *
 * (RECOVERY OPERATION:)
 *      None needed.
 *
 * (DATA STRUCTURES:)
 *      This function modifies the following global structures and variables:
 *      All structures of type msglist.
 *
 * RETURNS:
 *      None.
 */

maketitleadvanced()
{
        ledson_yes_no[0].msgid = TESTAM;
        ledsoff_yes_no[0].msgid = TESTAM;
        keypad_frame[0].msgid = TESTAM;
        kbrd_yes_no[0].msgid = TESTAM;
        clickon_explain[0].msgid = TESTAM;
        clickon_yes_no[0].msgid = TESTAM;
        clickoff_explain[0].msgid = TESTAM;
        clickoff_yes_no[0].msgid = TESTAM;
        speaker_explain[0].msgid = TESTAM;
        speaker_no_enter[0].msgid = TESTAM;
        speaker_yes_no[0].msgid = TESTAM;
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

       /* in case that the kbd test code is executed using a serial terminal
          and the systems does not have a console keyboard "unusual" */

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

/*
 ********************************************************************
 *
 *  NAME: special_function
 *
 *  FUNCTION: This fucntions forces default value to YES.
 *
 */ 

int special_display(TUTYPE *tucb_ptr, struct msgtable *mtp)
{

  ASL_SCR_TYPE    menutypes2= DM_TYPE_DEFAULTS;
  int mrc;

  /* Copy msg strings from kbrd_yes_no struct to menu_kbdyn */
  /* This should cause the menu's default highlight to be on the 'YES' choice */

  diag_display(mtp->msgnum, tucb_ptr->tuenv.catd,mtp->mlp, DIAG_MSGONLY,
               ASL_DIAG_KEYS_ENTER_SC, &menutypes2, menu_kbdyn);


  /* Ask if keyboard keypad test ran OK - Put up the menu */
  mrc = chk_stat(diag_display(mtp->msgnum,tucb_ptr->tuenv.catd,NULL,
                 DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                 &menutypes2, menu_kbdyn));

  if (mrc != 0) {
     return(mrc);
  }

  return(menutypes2.cur_index);

}


