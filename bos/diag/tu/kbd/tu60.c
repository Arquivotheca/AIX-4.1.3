/* static char sccsid[] = "@(#)03  1.4  src/bos/diag/tu/kbd/tu60.c, tu_kbd, bos411, 9433A411a 7/13/94 09:37:32"; */
/*
 * COMPONENT_NAME: tu_kbd 
 *
 * FUNCTIONS:  	tu60.c 
 *		Speaker Frequency, Duration, and Volume Test
 *
 * DESCRIPTION: Reset POS register
 *		Send Keyboard Echo Command
 *		Set Speaker to high volume
 *		Send Higher byte of Set 30 Hz Frequency
 *		Send Lower byte of Set 30 Hz Frequency
 *		Send Higher byte of Set 2 Sec Duration 
 *		Send Lower byte of Set 2 Sec Duration 
 *		Wait for 4 secs for the completion of the speaker tone
 *		Set Speaker to low volume
 *		Send Higher byte of Set 30 Hz Frequency
 *		Send Lower byte of Set 30 Hz Frequency
 *		Send Higher byte of Set 2 Sec Duration 
 *		Send Lower byte of Set 2 Sec Duration 
 *		Wait for 4 secs for the completion of the speaker tone
 *		Set Speaker to high volume
 *		Send Higher byte of Set 12000 Hz Frequency
 *		Send Lower byte of Set 12000 Hz Frequency
 *		Send Higher byte of Set 6 Sec Duration 
 *		Send Lower byte of Set 6 Sec Duration 
 *		Wait for 7 secs for the completion of the speaker tone
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

int tu60(int fdes, TUTYPE *tucb_ptr)
{
    extern int errno;
    unsigned char cdata, kbddata;
    unsigned char line_buff[100];
    ushort data;
    
    static int rc = SUCCESS;
    
#ifdef DIAGNOSTICS
  int  mrc; /* Message return code */
#endif

while (1) {

     /* Open Device Driver in Diagnostics Mode */
     rc = open_kbd(tucb_ptr);
    
    /* Reset Keyboard Adapter Hardware */
    if (( rc = Kbd_Reset(fdes)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, ADP_HDW_RSET_ERR));

#ifdef nodiag
    printf("\nThis test will exercise the speaker volume, frequency, and");
    printf("\nduration functions.");
    printf("\nWhen the test starts you should:");
    printf("\nLISTEN.........for a high volume, low frequency, tone for 2 seconds.");
    printf("\nLISTEN.........for a low volume, low frequency, tone for 4 seconds.");
    printf("\nLISTEN.........for a high volume, high frequency, tone for 6 seconds.");
#endif  

   close(kbdtufd);

#ifdef DIAGNOSTICS
   (void) putmsg(tucb_ptr,++mtp); /* speaker explain msg */
   (void) putmsg(tucb_ptr,++mtp); /* speaker no enter msg */
#endif

   rc = open_kbd(tucb_ptr);

    /* Send keyboard Echo command*/

    data = KBD_ECHO_CMD;
    rc = send_kbd(fdes, data);
    
    /* Verify the echoed data */
    /* Read port A for echoed data */

    rc = get_data(fdes, 8, ECHO_DATA, &kbddata);
    
    /* Set speaker volume to high */
    data = SPK_HVOL_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_SPK_HVOL_ERR));

    /* Set speaker to 30 Hz frequency */
    /* This is a two byte command and the value of both the bytes are    
     * calculated using the table in the sub-heading of Adapter Speaker
     * Control. 
     * A sample calculation is given below    
     * The high byte is called Cx and for 30Hz frequency it is defined as
     * decimal 64 which 0x40. It is tagged along with 0x08 of the Set
     * Speaker Frequency command.
     * The low byte is is called Ct and is calculated as (6000/30) - 9.31
     * where 30 is the given frequency. The decimal value is 191 and the
     * hex value is 0xBF which is tagged along with 0x09 of the Set Speaker
     * Frequency command */
    
    /* Send high order byte */
    data = SPK_30HZ_HBYT_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_30_HBYT_ERR));
    
    /* Send low order byte */
    data = SPK_30HZ_LBYT_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_30_LBYT_ERR));

    /* To set speaker duration for 2 secs */
    /* This a two byte command and is as follows.
     * The first byte which is the high byte is tagged along with 0x07 
     * which is the Set Speaker Duration command  
     * The second byte which is the low byte is tagged along with 0x02
     * which is Write to Speaker Command since this is specified in the
     * last two lines of Speaker Duration Control write up. */
    
    /* The high order byte */
    data = SPK_2SEC_DUR_HBYT_CMD;
    rc = Send_Data(fdes, data, 0, 0);
    
    /* Send low order byte */
    data = SPK_2SEC_DUR_LBYT_CMD;
    rc = Send_Data(fdes, data, 0, SPK_CMD_ACCP);

    /* A delay of 4 secs to take into account the duration of the
     * speaker tone. Without this delay the speaker is executing
     * the tone and the adapter is processing the next command.
     * Due to this the buffer is filled with the ack from the
     * next command where as the ack of the speaker tone is still
     * due. This delay will wait for the ack of the speaker tone
     * before the next command is executed. */
    
    usleep(4000 * 1000);
    
    /* Set speaker volume to low */
    data = SPK_LVOL_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_SPK_HVOL_ERR));
    
    /* Set speaker to 30 Hz frequency */
    
    /* Send high order byte */
    data = SPK_30HZ_HBYT_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_30_HBYT_ERR));
    
    /* Send low order byte */
    data = SPK_30HZ_LBYT_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_30_LBYT_ERR));
    
    /* To set speaker duration for 4 secs */
    
    /* The high order byte */
    data = SPK_4SEC_DUR_HBYT_CMD;
    rc = Send_Data(fdes, data, 0, 0);
    
    /* Send low order byte */
    data = SPK_4SEC_DUR_LBYT_CMD;
    rc = Send_Data(fdes, data, 0, SPK_CMD_ACCP);
    
    /* A delay of 4 secs to take into account the duration of the
     * speaker tone. Without this delay the speaker is executing
     * the tone and the adapter is processing the next command.
     * Due to this the buffer is filled with the ack from the
     * next command where as the ack of the speaker tone is still
     * due. This delay will wait for the ack of the speaker tone
     * before the next command is executed. */
    
    usleep(4000 * 1000);
    
    /* Set speaker volume to high */
    data = SPK_HVOL_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_SPK_HVOL_ERR));
    
    /* Set speaker to 12000 Hz frequency */
    
    /* This is a two byte command and the value of both the bytes are    
     * calculated using the table in the sub-heading of Adapter Speaker
     * Control. 
     *  A sample calculation is given below    
     * The high byte is called Cx and for 12000Hz frequency it is defined as
     * decimal 1 which 0x01. It is tagged along with 0x08 of the Set
     * Speaker Frequency command.
     * The low byte is is called Ct and is calculated as (384000/120000) - 12.95 
     * where 12000 is the given frequency. The decimal value is 19 and the
     * hex value is 0x13 which is tagged along with 0x09 of the Set Speaker
     * Frequency command */
    
    /* Send high order byte */
    data = SPK_12000HZ_HBYT_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_30_HBYT_ERR));
    
    /* Send low order byte */
    data = SPK_12000HZ_LBYT_CMD;
    if ((rc = Send_Data(fdes, data, 0, 0)) != SUCCESS)
        return(mktu_rc(fdes,tucb_ptr->header.tu, KBD_ERR, DEV_30_LBYT_ERR));
    
    /* To set speaker duration for 6 secs */
    
    /* The high order byte */
    data = SPK_6SEC_DUR_HBYT_CMD;
    rc = Send_Data(fdes, data, 0, 0);
    
    /* Send low order byte */
    data = SPK_6SEC_DUR_LBYT_CMD;
    rc = Send_Data(fdes, data, 0, SPK_CMD_ACCP);
    
    /* A delay of 7 secs to take into account the duration of the
     * speaker tone. Without this delay the speaker is executing
     * the tone and the adapter is processing the next command.
     * Due to this the buffer is filled with the ack from the
     * next command where as the ack of the speaker tone is still
     * due. This delay will wait for the ack of the speaker tone
     * before the next command is executed. */

    usleep(7000 * 1000);
    
#ifdef nodiag
    printf("\nDid the speaker work correctly? (y/n) ");
    scanf("%s", line_buff);
    if (line_buff[0] == 'n'  || line_buff[0] == 'N')
        return(mktu_rc(fdes,tucb_ptr->header.tu, LOG_ERR, ADP_FAIL_VOL_TEST));
#endif
    
    close(kbdtufd);

#ifdef DIAGNOSTICS
  mrc = putmsg(tucb_ptr,++mtp); /* speaker yes/no msg */
  if (mrc != YES)
  {
   if (mrc == RETRY)
      {
        mtp--;
        mtp--;
        mtp--;
        continue;
      }
   if (mrc < 0)
     return(rc);
   else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
     return(mrc);
   else
    return(mktu_rc(fdes,tucb_ptr->header.tu, SYS_ERR, ADP_FAIL_VOL_TEST));
  }
#endif
  break;
  } /* end while */
    return(rc);
}
