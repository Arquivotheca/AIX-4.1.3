/* static char sccsid[] = "@(#)01  1.5  src/bos/diag/tu/kbd/tu40.c, tu_kbd, bos411, 9433A411a 7/13/94 09:37:18"; */
/*
 * COMPONENT_NAME: tu_kbd
 *
 * FUNCTIONS:   tu40, inputsig, genkeymap, updatescree,
 *              finish, gotinput
 *		Key Pad and Key Type Test
 *
 * DESCRIPTION: Reset POS register
 *              Identify keyboard (Number of keys)
 *              Set-up Device Driver input ring
 *              Display keyboard 
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

/**************************************************************************
* NOTE: This function is called by Hardware exerciser (HTX),Manufacturing *
*       application and Diagnostic application to invoke a test unit (TU) *
*                                                                         *
*       If the mfg mode in the tu control block (tucb) is set to be       *
*       invoked by HTX then TU program will look at variables in tu       *
*       control block for values from the rule file. Else, TU program     *
*       uses the predefined values.                                       *
*                                                                         *
***************************************************************************/

#include "tu_type.h"

extern int mktu_rc();
int finish(WINDOW *);
int updatescreen(WINDOW *);
WINDOW *genkeymap();

int keypos, mode[133];
int keyboard = 0;
int i, j, linei, columni, line, column;

/*********************************************************************/
/* inputsig - process input ring event notification                  */
/*********************************************************************/

void inputsig(sig_type)
int sig_type;
{
     (int) signal (SIGMSG, inputsig);       /* re-arm signal handler    */

}


int tu40(int fdes, TUTYPE *tucb_ptr)
{
    extern int errno;
    unsigned char cdata, kbddata, byte1, byte2;
    ushort data;
    int i, rc = SUCCESS;
    WINDOW *w;
    struct inputring *ir;                   /* input ring   */
    struct uregring reg;            /* register ring structure */
    char quit;

 /* for new menu stuff to support new kbds */
    char    msgstr[512];
/*  char    gerpart[] = "52G1023";   * Codigo para el teclado Aleman */
    char    brazpart[] = "88G3936";  /* Part # for new Brazilian kbd */

    
#ifdef DIAGNOSTICS
  int  mrc;                              /* Message return code */
#endif

    (int) signal (SIGMSG, inputsig);       /* arm signal handler    */

    /* Open Device Driver in Diagnostics mode */
    rc = open_kbd(tucb_ptr);

    /* Reset Keyboard Adapter Hardware */
    if (( rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_HDW_RSET_ERR));
    
    usleep(500 * 1000);

    /* send read keyboard id command */
    data = KBD_ID_CMD;
    if ((rc = send_kbd(fdes, data)) != SUCCESS)
      return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST)); 

    /* Read port A for keyboard ID */
    if ((rc = get_data(fdes, 1, 0, &kbddata)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST));

    byte1= kbddata;

    /* Second Byte */

    /* Read port A for keyboard ID */
    if ((rc = get_data(fdes, 1, 0, &kbddata)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST));

    byte2 = kbddata;

    if (byte1 == KBD_1_101_ID && byte2 == KBD_2_101_ID) /* 101 or 102 kbd */    
       {
          /* Send keyboard layout command */
          data =  KBD_LAY_ID_CMD;
          if ((rc = send_kbd(fdes, data)) != SUCCESS)
              return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST));

          /* To read and verify the keyboard layout ID bytes */
          /* First Byte */
          /* Read port A for keyboard layout ID */
          if ((rc = get_data(fdes, 1, 0, &kbddata)) != SUCCESS)
                return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST));

          byte1 = kbddata;
          if (byte1 == KBD_LAY_101)
              keyboard = 0;               /* 101 keyboard */

          if (byte1 == KBD_LAY_102)
              keyboard = 1;               /* 102 keyboard */

       }  /* if 101 or 102 keyboard */

    else if (byte1 == KBD_1_106_ID && byte2 == KBD_2_106_ID)
              keyboard = 2;           /* 106 Kanji keyboard */

    else 
      {
          rc = FAILURE;
          return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST));
      }

    if(keyboard != 2) 
       {
          /* Second Byte */
          /* Read port A for keyboard ID */
          if ((rc = get_data(fdes, 1, 0, &kbddata)) != SUCCESS)
              return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST));

          if (kbddata != KBD_2ND_LAY_ID)
              return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST));
       }

    /* Send keyboard Echo command */
    data =  KBD_ECHO_CMD;
    if ((rc = send_kbd(fdes, data)) != SUCCESS)
             return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST));

    /* Verify the echoed data */
    /* Read port A for echoed data */
    if ((rc = get_data(fdes, 8, ECHO_DATA, &kbddata)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST));

    if (kbddata != ECHO_DATA)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_PAD_TEST));

    /* Instructions to the User */

#ifdef nodiag
    printf("\nPressing each key on the keyboard will toggle the corresponding X");
    printf("\non the display between normal and reverse video.");
    printf("\nGO.............to the keyboard attached to the system keyboard adapter.");
    printf("\nPRESS..........each of the keys on the keyboard (pressing the Esc key");
    printf("\n               will exit from this test).");
    printf("\nOBSERVE........the corresponding X on the display.");
    printf("\nWhen finished, press the Esc key on the keyboard attached to the system");
    printf("\nkeyboard adapter.");
    printf("\nPress <ENTER> to start the test.\n");
#endif

#ifdef nodiag    
    scanf("%1c", &cdata);
#endif

    /* Clear mode array - Each item of this array corresponds to a
     * position for the keyboard. Whenever a key is pressed, the mode
     * for that key will be toggled from 0 to 1, and vice versa. This
     * is done in the updatescreen() function. */

    for ( i = 0; i < KEY_POS_CNT; ++i)
        mode[i] = 0;
    
    /* Display keyboard menu and obtain character to display the
     * keyboard keymap from the catalog file */


#ifdef DIAGNOSTICS
   mtp++;
#endif

   if (keyboard == 1)
   {

    /* If keyboard is type 102 ask if it's a Brazilian  or teclado Aleman 
     * keyboard */

     close(kbdtufd);

#ifdef DIAGNOSTICS
     memset (menu_102, 0, sizeof (menu_102));

     /* Do following stuff to insert part #'s into the ask_102 msg's */
     /* Copy msg strings from ask_102 struct to menu_102 */
     diag_display(mtp->msgnum, tucb_ptr->tuenv.catd,mtp->mlp, DIAG_MSGONLY,
            ASL_DIAG_KEYS_ENTER_SC, &menutypes, menu_102);

/*
     sprintf(msgstr, menu_102[1].text, gerpart);
     free (menu_102[1].text);
     menu_102[1].text = (char *) malloc (strlen(msgstr)+1);
     strcpy (menu_102[1].text, msgstr);
*/
     sprintf(msgstr, menu_102[1].text, brazpart);
     free (menu_102[1].text);
     menu_102[1].text = (char *) malloc (strlen(msgstr)+1);
     strcpy (menu_102[1].text, msgstr);

     /* Set indexes so that when menu is displayed, the last (default) choice
        will be highlighted.  Ex: if you want the 3rd and last choice to be
        highlighted, you must have cur_index = 3, max_index = (# of items - 1)
        in the msg struct (excluding the NULL entry)  */

/*
     menutypes.cur_index = 3;
     menutypes.max_index = 4;
*/

     menutypes.cur_index = 2;
     menutypes.max_index = 3;

    /* Ask what kind 102 keyboard - Put up the menu */
     mrc = chk_stat(diag_display(mtp->msgnum,tucb_ptr->tuenv.catd,NULL,
                        DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                        &menutypes, menu_102));

     if (mrc < 0)
       return(rc);
     if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
       return(mrc);
/*
     if (menutypes.cur_index == 1)  * Teclado Aleman? *
       keyboard = 4;
*/
     if (menutypes.cur_index == 1)  /* Brazil kbd? */
       keyboard = 3;
     rc = open_kbd(tucb_ptr);
#endif
    } /* if keyboard code is 1 */

    close(kbdtufd);

#ifdef DIAGNOSTICS
   (void) putmsg(tucb_ptr,++mtp); /* keypad_frame msg */
#endif

  /* Open kbd device driver in normal mode */
  if((kbdtufd = open(tucb_ptr->tuenv.kbd_fd, O_RDWR)) < 0)
     return(FAILURE);

  /* Set up to use input ring service of kbd DD */

   ir = (struct inputring *) malloc(RINGSIZE);
   if (!ir) 
      return(FAILURE);

  /* Initialize reg structure */

   reg.ring = (caddr_t) ir;
   reg.size = RINGSIZE;
   reg.report_id = 0xa5;

  /* Initialize ring_buffer */

   ir->ir_size = RINGSIZE - sizeof(struct inputring);
   ir->ir_head = (caddr_t) ir + sizeof(struct inputring);
   ir->ir_tail = ir->ir_head;
   ir->ir_notifyreq = IRSIGEMPTY;
   ir->ir_overflow = IROFCLEAR;

   rc = ioctl(kbdtufd, KSREGRING, &reg);

    /* Generate the keyboard picture */

    w = genkeymap();

  if (rc == SUCCESS)
  {

   /* Get keyboard data until user escapes */

      quit = 0;
      while (!quit) {     /* while ESC not pressed */
         ir->ir_notifyreq = IRSIGALWAYS;
         rc = sleep(10);
         if (rc > 0) {
            ir->ir_notifyreq = IRSIGEMPTY;
            gotinput(ir,&quit,w,kbdtufd);
         }
      }

  }  /* if rc == SUCCESS */

  /* Disable input from device to ring */

  ioctl(kbdtufd,KSREGRING, NULL);

  close(kbdtufd);

  rc = open_kbd(tucb_ptr);
  /* Free memory for inputring */
  free(ir);
  close(kbdtufd);
#ifdef nodiag
    printf("\nWas the test successful? (y/n)");
    scanf("%2c", &cdata);
    if (cdata == 'n' || cdata == 'N')
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, DEV_FAIL_PAD_TEST));
#endif


#ifdef DIAGNOSTICS

  /* Call special_display to solve the menutypes global problem, since
     special_display will initialize its own local copy.  */

  mrc = special_display(tucb_ptr,++mtp); /* keyboard yes/no msg */

  if (mrc != YES)
  {
   if (mrc < 0)
     return(rc);
   else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
     return(mrc);
   else
    return(mktu_rc(fdes,tucb_ptr->header.tu, SYS_ERR, DEV_FAIL_PAD_TEST));
  }
#endif
    
    return(rc);
}

/************************************************************
 * This are the curses routines declared in the include lib
 * which are used specifically in this TU to generate the
 * keymap for the testing the keyboard
 *************************************************************/

/* This function generates the keymap */
WINDOW *genkeymap()
{
    int j;
    WINDOW *window;
    
    linei = 1;
    columni = 5;
#ifdef nodiag
    initscr(); 
#endif
    window = newwin(10,50,13,15);
    for(j = 0; j < KEY_POS_CNT; j++)
    {
        if (indexes[keyboard][j] == 1)
            mvwaddch(window,linei+key[0][j], columni+key[1][j],'X');
    }
    
    /* Place cursor in lower right hand corner of the box */
    
    wmove(window,8,48);
    wrefresh(window);
    return(window);
}

/* This function updates the screen after a key is pressed and released */
int updatescreen(WINDOW *win)
{
    int chmode;
    int x,y;
    
    if (keypos == ESC_KEY)    /* This is 'ESC' key position 110 */
    {
        getyx(win,y,x);
        wmove(win,(linei+key[0][keypos-1]), (columni+key[1][keypos-1]));
        wchgat(win,1,STANDOUT);
        wmove(win,y,x);
        wrefresh(win);
        sleep(1);
        werase(win);
        wrefresh(win);
        delwin(win);
#ifdef nodiag
        endwin();
#endif
        return(1);
    }
    
    /* Make sure that the key to be filled in on the screen exists on the
     * keyboard currently being tested */
    if (indexes[keyboard][keypos-1] == 1)
    {
        getyx(win,y,x);
        wmove(win,(linei+key[0][keypos-1]), (columni+key[1][keypos-1]));
        if (mode[keypos-1] == 0)
        {
            mode[keypos-1] = 1;
            chmode = STANDOUT;    /* Key is highlighted */
        }
        else {
            mode[keypos-1] = 0;
            chmode = NORMAL;     /* Highlighted removed */
        }
        wchgat(win,1,chmode);
        wmove(win,y,x);
        wrefresh(win);
    }
    return(0);
}

/* This function ends the curses routines */

int finish(WINDOW *win)
{
    werase(win);
    wrefresh(win);
    delwin(win);
#ifdef nodiag
    endwin();
#endif
}

/*********************************************************************/
/* gotinput - process input ring events                              */
/*********************************************************************/
/* Input arguments:

   *ir          : ptr to struct inputring
   *quit        : ptr to quit - tell calling function whether we have
                  quit or not
   *w           : ptr to WINDOW - for updating keyboard picture
   fd           : file descriptor for kbd device
*/

gotinput(struct inputring *ir, char *quit, WINDOW *w, long fd)
{
   struct ir_kbd rr;
   int i;
   static int odd = 0;
   char *sce;
   char *sink;
   int rc;

   while(ir->ir_head != ir->ir_tail) {

     sce = (char *) ir->ir_head;
     sink = (char *) &rr;

     for(i=0;i<sizeof(struct ir_kbd);i++) {
        *sink = *sce;
        sink++;
        sce++;
        if (sce >= ((char *)(ir) + RINGSIZE)) {
           sce = (char *) (ir) + sizeof(struct inputring);
        }
     }

     ir->ir_head = (caddr_t) sce;

     if (odd) {  /* Since for every keystroke there are two data reports, */
                 /* we set up to just look at every other report received. */

       keypos = rr.kbd_keypos;
       updatescreen(w);   /* Update keyboard picture with new key info */

       /* If user hits ESC, quit */
       if (rr.kbd_keypos == 110) *quit = 1;  /* quit if Esc */

       odd = 0;
     } /* if odd */

     else odd = 1;

   } /* while */

   if (ir->ir_overflow == IROVERFLOW)
      rc = ioctl(fd, KSRFLUSH, NULL);
   return(rc);
}

