/* static char sccsid[] = "@(#)00  1.5  src/bos/diag/tu/kbd/tu30.c, tu_kbd, bos411, 9433A411a 7/13/94 09:37:12"; */
/*
 * COMPONENT_NAME: tu_kbd 
 *
 * FUNCTIONS:  	tu30.c
 *		Keyboard Indicators Test	        
 *
 * DESCRIPTION: Reset POS register
 *		Send Restore Indicator command
 *		Send Indicators "ON" command
 *		Send Restore Indicator command
 *		Send Indicators "OFF" command
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


int tu30(int fdes, TUTYPE *tucb_ptr)
{
    extern int errno;
    unsigned char cdata;
    ushort data;
    static int rc = SUCCESS;


#ifdef DIAGNOSTICS
  int  mrc; /* Message return code */
#endif

while (1) {

#ifdef DIAGNOSTICS
  mtp = &msgtab[0];    /* Initialize pointer into message table */
  mrc = putmsg(tucb_ptr, mtp); /* keyboard_explain msg */
  if (mrc != 0) 
	return(mrc);
  mrc = putmsg(tucb_ptr, ++mtp); /* keyboard_no_enter msg (same w/out yes/no) */
  if (mrc != 0) 
        return(mrc);
#endif

  /* Open Device Driver in Diagnostics Mode */
  rc = open_kbd(tucb_ptr);

#ifdef nodiag

   printf("\nThe LED indicators on the keyboard attached to");
   printf("\nthe system keyboard adapter should now be on.");
   printf("\nfor 3 seconds. \n");

#endif

    /* Reset Keyboard Adapter Hardware */
    if (( rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_HDW_RSET_ERR));
    
    /* Send Restore Indicator command */
    data = RST_MDE_IND_CMD;
    if ((rc = send_kbd(fdes, data)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_IND_ON_ERR));
  
    /* Send Indicators ON command */
    data =  SET_IND_ON_CMD;
    if ((rc = send_kbd(fdes, data)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_IND_ON_ERR));
   
    sleep(3);

    /* Send Restore Indicator command */
    data = RST_MDE_IND_CMD;
    if (( rc = send_kbd(fdes, data)) != SUCCESS)
          return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_IND_TEST));

    /* Send Indicators OFF command */
    data = SET_IND_OFF_CMD;
    if ((rc = send_kbd(fdes, data)) != SUCCESS)
         return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_FAIL_IND_TEST));

    close(kbdtufd);

#ifdef nodiag
   printf("\n Did the keyboard LED indicators work? (y/n)");
   cdata=NULL;
   scanf("%c", &cdata);
   if (cdata == 'n' || cdata == 'N')
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, DEV_FAIL_IND_TEST));
#endif

#ifdef DIAGNOSTICS

  mrc = putmsg(tucb_ptr, ++mtp); /* LEDs on? yes/no msg */
				
  if (mrc != YES)    /* User should just hit <Enter> or ESC */
  {
    if (mrc == RETRY)
         continue;
    if (mrc < 0)
         return(rc);
    else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
      return(mrc);
    else
      return(mktu_rc(fdes,tucb_ptr->header.tu, SYS_ERR, DEV_IND_ON_ERR));
  }
  mrc = putmsg(tucb_ptr, ++mtp); /* LEDs off yes/no msg */

  if (mrc != YES)    /* User should just hit <Enter> or ESC */
  {
    if (mrc == RETRY)
         continue;
    if (mrc < 0)
         return(rc);
    else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
      return(mrc);
    else
      return(mktu_rc(fdes,tucb_ptr->header.tu, SYS_ERR, DEV_FAIL_IND_TEST));
  }

#endif
  break;
  }  /* end while */

    return(rc);
}
