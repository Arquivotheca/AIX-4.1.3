/* static char sccsid[] = "@(#)02  1.5  src/bos/diag/tu/kbd/tu50.c, tu_kbd, bos411, 9433A411a 7/13/94 09:37:26"; */
/*
 * COMPONENT_NAME: tu_kbd
 *
 * FUNCTIONS:   tu50.c 
 *		Keystroke Click Test
 *
 * DESCRIPTION: Reset POS register
 *              set-up Deveice Driver input ring
 *              Set CLICK to high
 *              Set CLICK to disable
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

extern int mktu_rc();

/*********************************************************************/
/* inputsig - process input ring event notification                  */
/*********************************************************************/

void inputsig(sig_type)
int sig_type;
{

  (int) signal (SIGMSG, inputsig);       /* re-arm signal handler    */

}

int tu50(int fdes, TUTYPE *tucb_ptr)
{
    extern int errno;
    unsigned char kbddata, cdata;
    int i;
    uint arg0;
    unsigned char line_buff[100];
    ushort data;
    static int rc = SUCCESS;

    /* ring buffer stuff... */
    struct inputring *ir;                   /* input ring   */
    struct uregring reg;            /* register ring structure */
    char quit;

#ifdef DIAGNOSTICS
  int  mrc; /* Message return code */
#endif
    (int) signal (SIGMSG, inputsig);       /* arm signal handler    */

    /* Open Device Driver in Diagnostics Mode */ 
    rc = open_kbd(tucb_ptr);
    
    /* Reset Keyboard Adapter Hardware */
    if (( rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_HDW_RSET_ERR));

#ifdef nodiag
    printf("\nWhen the keystroke auto-click is enabled and a key");
    printf("\nis pressed the speaker emits a clicking sound.");
    printf("\nKeystroke auto-click is now enabled.");
    printf("\nGO.............to the keyboard attached to the system keyboard adapter.");
    printf("\nPRESS..........each of the keys on the keyboard (pressing the Esc key");
    printf("\n               will exit from this test).");
    printf("\nLISTEN.........for a clicking sound when each key is pressed.");
    printf("\nWhen finished, press the Esc key on the keyboard attached to the system");
    printf("\nkeyboard adapter.");
#endif

   close(kbdtufd);

#ifdef DIAGNOSTICS
   (void) putmsg(tucb_ptr,++mtp); /* clickon explain msg */
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

   arg0 = KSCLICKHI; 
   rc = ioctl(kbdtufd, KSCFGCLICK, &arg0);

   rc = ioctl(kbdtufd, KSREGRING, &reg);

  if (rc == SUCCESS)
  {

   /* Get keyboard data until user escapes */

      quit = 0;
      while (!quit) {     /* while ESC not pressed */
         ir->ir_notifyreq = IRSIGALWAYS;
         rc = sleep(10);
         if (rc > 0) {
            ir->ir_notifyreq = IRSIGEMPTY;
            getinput(ir,&quit,kbdtufd);
         }
      }

  }  /* if rc == SUCCESS */

  /* Disable input from device to ring */

  ioctl(kbdtufd,KSREGRING, NULL);

  close(kbdtufd);

#ifdef nodiag
    printf("\nWas the keystroke auto-click enabled? (y/n");
    scanf("%s", line_buff);
    if (line_buff[0] == 'n' || line_buff[0] == 'N')
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, DEV_FAIL_PAD_TYP_TEST));
#endif

#ifdef DIAGNOSTICS

  /* Call special_display to solve the menutypes global problem, since
     special_display will initialize its own local copy.  */

  mrc = special_display(tucb_ptr,++mtp);  /* clickon yes/no msg */

  if (mrc != YES)
  {
   if (mrc < 0)
     return(rc);
   else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
     return(mrc);
   else
    return(mktu_rc(fdes,tucb_ptr->header.tu, SYS_ERR, ADP_CLICK_ON_ERR));   
  }
#endif

#ifdef nodiag
    printf("\nWhen the keystroke auto-click is disabled and a key");
    printf("\nis pressed the speaker will not emit a clicking sound.");
    printf("\nThe keystroke auto-click is now disabled.");
    printf("\nGO.............to the keyboard attached to the system keyboard adapter.");
    printf("\nPRESS..........each of the keys on the keyboard (pressing the Esc key");
    printf("\n               will exit from this test).");
    printf("\nLISTEN.........for the absence of a clicking sound");
    printf("\n               when each key is pressed.");
    printf("\nWhen finished, press the Esc key on the keyboard attached to the system");
    printf("\nkeyboard adapter.");
#endif    

#ifdef DIAGNOSTICS
   (void) putmsg(tucb_ptr, ++mtp); /* clickoff explain msg */
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

   arg0 = KSCLICKOFF;
   rc = ioctl(kbdtufd, KSCFGCLICK, &arg0);
   rc = ioctl(kbdtufd, KSREGRING, &reg);

  if (rc == SUCCESS)
  {

   /* Get keyboard data until user escapes */

      quit = 0;
      while (!quit) {     /* while ESC not pressed */
         ir->ir_notifyreq = IRSIGALWAYS;
         rc = sleep(10);
         if (rc > 0) {
            ir->ir_notifyreq = IRSIGEMPTY;
            getinput(ir,&quit,kbdtufd);
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
    printf("\nWas the keystroke auto-click disabled? (y/n)");
    scanf("%s", line_buff);
    if (line_buff[0] == 'n' || line_buff[0] == 'N')
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, DEV_FAIL_PAD_TYP_TEST));
#endif

#ifdef DIAGNOSTICS

  /* Call special_display to solve the menutypes global problem, since
     special_display will initialize its own local copy.  */

  mrc = special_display(tucb_ptr,++mtp);  /* clickoff yes/no msg */

  if (mrc != YES)
  {
   if (mrc < 0)
     return(rc);
   else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
     return(mrc);
   else
    return(mktu_rc(fdes,tucb_ptr->header.tu, SYS_ERR, ADP_CLICK_OFF_ERR));
  }
#endif
    
    return(rc);
}

/*********************************************************************/
/* getinput - process input ring events                              */
/*********************************************************************/
/* Input arguments:

   *ir          : ptr to struct inputring
   *quit        : ptr to quit - tell calling function whether we have
                  quit or not
   fd           : file descriptor for kbd device
*/

getinput(struct inputring *ir, char *quit, long fd)
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

