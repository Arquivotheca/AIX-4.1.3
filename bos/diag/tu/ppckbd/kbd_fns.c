static char sccsid[] = "@(#)17  1.1  src/bos/diag/tu/ppckbd/kbd_fns.c, tu_ppckbd, bos41J, 9520A_all 5/6/95 14:31:32";
/*
 *   COMPONENT_NAME: tu_ppckbd
 *
 *   FUNCTIONS: clean_output_buf
 *              kbd_reset
 *              kbd_restore
 *              led_off
 *              led_on
 *              output_buf_empty
 *              output_buf_full
 *              rd_byte
 *              send_data
 *              wr_byte
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "tu_type.h"

static unsigned char idata;

/****************************************************************************/
/* FUNCTION: wr_byte

   DESCRIPTION: Uses the machine device driver to write ONE BYTE to
                the specified address

****************************************************************************/

int wr_byte(int fdes, unsigned char *pdata,unsigned int addr)
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;
  rc = ioctl(fdes, MIOBUSPUT, &iob);
  return (rc);
}

/****************************************************************************/
/* FUNCTION: rd_byte

   DESCRIPTION: Uses the machine device driver to read ONE BYTE to
                the specified address

****************************************************************************/

int rd_byte(int fdes, unsigned char *pdata,unsigned int addr)
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;
  rc = ioctl(fdes, MIOBUSGET, &iob);
  return (rc);
}

/***************************************************************************

 FUNCTION:  kbd_reset(fd)
            - file descriptor

 DESCRIPTION: This funtion resets the system keyboard and executes the BAT
              test...

 ***************************************************************************/

kbd_reset(fd)
int fd;
{
  unsigned char cdata, kbd_data = 0;
  int rc = SUCCESS;

  if ((rc = in_out_buffers_empty(fd)) != SUCCESS)
        return(rc);

  /* Send RESET Command to the keyboard */

  cdata = KBD_RSET_CMD;
  wr_byte(fd, &cdata, DATA_PORT);

  /* First the keyboard will respond with ACK = 0xFA, and then
   * completion code = 0xAA
   */

  read_data(fd, &kbd_data);

  if (kbd_data & KBD_ACK != KBD_ACK)
        return(FAILURE);

  /* wait for BAT to complete, a max. time of 3 sec. */

  usleep(500 * 1000);
  kbd_data = 0;
  read_data(fd, &kbd_data);
  if (kbd_data & KBD_CMP_CDE != KBD_CMP_CDE)
        rc = FAILURE;
  return(rc);
}
/***************************************************************************

  FUNCTION: output_buff_full(fdes)
            - fdes file descriptor

  DESCRIPTION: Check Status register (port 0x64) for output buffer full.
               Bit 0 in the Status Register is set to 1.

****************************************************************************/

int output_buf_full(fdes)
int fdes;
{
  int count = 0;
  int rc = SUCCESS;
  unsigned char csr;

  usleep(3 * 1000);

  /* Read Port 0x64 bit 0 (=1) to check output buffer  is full*/

  for (count = 0; count < REPEAT_COUNT; count++)
     {
        rd_byte(fdes, &csr, CMD_PORT);
        if ((csr & CSR_BIT_6) == CSR_BIT_6)
           {
              rc = FAILURE;
              break;
           }
        if ((csr & CSR_BIT_0) == CSR_BIT_0)
              break;
        usleep(2 * 1000);
     }
  if(count >=  REPEAT_COUNT)
        rc = FAILURE;
  return(rc);

}

/***************************************************************************/
/* FUNCTION NAME: kbd_restore

   DESCRIPTION: Restore keyboard indicator status
                Set indicators off
                Set keys to typamatic make/break
                Set key type to make/break
                Set Speaker volume to medium
                Stop reporting of completion of spkr tone
                Enable keyboard

   NOTES:       This function restores the keyboard to its default
                values to exit the TU gracefully.
***************************************************************************/

int kbd_restore(fdes)
int fdes;
{
  int rc = SUCCESS;
  unsigned char kbddata, cdata;
  unsigned char data;

  /* Restore all keyboard indicator status. In this case all the
   * LED indicators are turned off which is the default value. The
   * RESET_MODE_INDICATOR_CMD is a two byte command */
  /* First byte */

  data = RST_MDE_IND_CMD;
  rc |= send_data(fdes, data);

  /* Second byte */

  data = SET_IND_OFF_CMD;
  rc |= send_data(fdes, data);

  /* Set all key types to typamatic make/break */

  data = KEY_TYP_MK_BRK_CMD;
  rc |= send_data(fdes, data);

  /* Set key type to make/break */

  data = SET_MK_BRK_CMD;
  rc |= send_data(fdes, data);

  /* Keyboard Scan Code command */

  data = KBD_SCAN_CDE_CMD;
  rc |= send_data(fdes, data);

  /* Select scan code set 3 */

  data = KBD_SCAN_SET3_CMD;
  rc |= send_data(fdes, data);

  /* Send Scan set query command */
  /* The Query is F0 00 , send first 0xF0, and then 0x00 */

  data = KBD_SCAN_CDE_CMD;
  rc |= send_data(fdes, data);
  data =  KBD_SCAN_QRY_CMD;
  rc |= send_data(fdes, data);
  rc |= read_data(fdes, &kbddata);
  if (kbddata != SCAN_SET3)
        rc = FAILURE;
  data = KBD_ENBL_CMD;
  rc |= send_data(fdes, data);
  return(rc);
}

/***************************************************************************

  FUNCTION: send_data(fd, cmd)
            - fd file descriptor
            - Command to be sent to keyboard device

  DESCRIPTION:

   To send the data to the keyboard device, first we tell the controller that
   we want to send the data to keyboard.

   So we send the command 0xD4 to controller at port 0x64.

****************************************************************************/

send_data(fd, cmd)
int fd;
unsigned char cmd;
{
  unsigned char ack_data, c_data;
  int i, rc = SUCCESS;

  i = 0;
  while (i < REPEAT_COUNT)
     {
        if ((rc = in_out_buffers_empty(fd)) != SUCCESS)
           {
              if (rc == OUTPUT_BUFF_FULL)
                    clean_output_buf(fd);
              else
                    return(rc);
           }

     /* Send the command to the keyboard, by writing at the DATA_PORT */

        wr_byte(fd, &cmd, DATA_PORT);

     /* Keyboard does not send ACK to RESEND and ECHO command */

        if ( cmd == KBD_RESEND_CMD || cmd == KBD_ECHO_CMD )
           {
              rc = SUCCESS;
              break;
           }
        usleep(500 * 1000);
        ack_data = 0;
        read_data(fd, &ack_data);

        /* Now check for ACK from kbd, if kbd returns with an RESEND cmd,
         * then resend the kbd data
         */

        if ( ack_data != KBD_RESEND_CMD )
           {
              if ((ack_data != KBD_ACK) ||

                        /* if kbd responds with overrun */

                 (ack_data == 0xFF) || (ack_data == KBD_CMD_ACCP))
                    rc = FAILURE;

                        /* We found the ACK from kbd */

              break;
           }
        i++;
     }
  if (i > REPEAT_COUNT)
        rc = FAILURE;
  return(rc);
}

/***************************************************************************

 FUNCTION: led_on(fdes)
           - fdes file descriptor

 DESCRIPTION:
           Turn LEDs on keyboard on (NumLock, CapsLock, ScrollLock).

 ***************************************************************************/

led_on(fdes)
int fdes;
{
  uchar   data;
  int     rc = SUCCESS;

  /* Send Restore Indicator command */

  data = RST_MDE_IND_CMD;
  rc = send_data(fdes, data);

  /* Send Indicators ON command */

  data =  SET_IND_ON_CMD;
  rc = send_data(fdes, data);
  return(rc);
}

/***************************************************************************

 FUNCTION: led_off(fdes)
           - fdes file descriptor

 DESCRIPTION:
           Turn LEDs off keyboard on (NumLock, CapsLock, ScrollLock).

 ***************************************************************************/

led_off(fdes)
int fdes;
{
  uchar   data;
  int     rc = SUCCESS;

  /* Send Restore Indicator command */

  data = RST_MDE_IND_CMD;
  rc = send_data(fdes, data);

  /* Send Indicators OFF command */

  data =  SET_IND_OFF_CMD;
  rc = send_data(fdes, data);
  return(rc);
}

/***************************************************************************

  FUNCTION: output_buf_empty(fd)
            - fd file descriptor

  DESCRIPTION:

  Check to see if the output buffer of controller at port 0x60 is empty or not
  This we check by reading the controller status register at port 0x64.
  Bit 0 and bit 5 will tell us if the buffer is empty or not.

****************************************************************************/

int output_buf_empty(fd)
int fd;
{
  unsigned char csr;
  int rc = SUCCESS;

  csr = 0;
  rd_byte(fd, &csr, CMD_PORT);

  if ((csr & CSR_BIT_0) == CSR_BIT_0)
     {
        rc = FAILURE;
     }
  return(rc);
}

/***************************************************************************

  FUNCTION: clean_output_buf(fd)

  DESCRIPTION: Flush the output buffer at port 0x60.

****************************************************************************/

clean_output_buf(fd)
int fd;
{
  unsigned char csr, data ;
  int rc = FAILURE;
  int i;

  while (i < REPEAT_COUNT)
     {
        data = 0;
        rd_byte(fd, &data, DATA_PORT);
        usleep(1 * 1000);
        if ((rc = output_buf_empty(fd)) == SUCCESS)
              break;
     }
  return(rc);
}

/***************************************************************************

  FUNCTION: read_data(fd, data)
            - fd file descriptor
            - data addess of data

  DESCRIPTION:  Read data from port 0x60

****************************************************************************/

read_data(fd, data)
int     fd;
uchar   *data;
{
  int rc = SUCCESS;
  rc = output_buf_full(fd);
  rc = rd_byte(fd, data, DATA_PORT);
  return(rc);
}

/***************************************************************************

  FUNCTION: in_out_buffers_empty(fd)
            -fd file descriptor

  DESCRIPTION:

  Check to see if the input and output buffers of kbd controller are empty.

****************************************************************************/

in_out_buffers_empty(fd)
int fd;
{
  uchar   csr, rdata;
  int     rc = SUCCESS;

  usleep(4 * 1000);

  /* Read the controller status */

  rd_byte(fd, &csr, CMD_PORT);

  /* If there was a timeout, then bit 6 of the CSR will be set to 1.
   * When there is a timeout, then
   * output buffer = 0xff  for receive timeouts,
   * output buffer - 0xfe  for transmission timeouts
   */

  if ((csr & CSR_BIT_6) == CSR_BIT_6)
     {
        if ((csr & CSR_BIT_7) == CSR_BIT_7)
           {
              rdata = 0;
              rc = rd_byte(fd, &rdata, DATA_PORT);
              if (rdata == 0xFF)
                    rc = FAILURE;  /* keyboard parity error */
           }
        else
           {
              rdata = 0;
              rc = rd_byte(fd, &rdata, DATA_PORT);
              if (rdata == 0xFF)
                    rc = FAILURE;  /* keyboard receive timeout */
              if (rdata == 0xFE)
                    rc = FAILURE;  /* keyboard send timeout */
            }
        rc = FAILURE;  /* keyboard timeout */
     }
  else
     {
        if ((csr & CSR_BIT_0) == CSR_BIT_0)
              rc = OUTPUT_BUFF_FULL;  /* output buffer full */
        if ((csr & CSR_BIT_1) == CSR_BIT_1)
              rc = FAILURE;  /* input buffer full */
     }
  return(rc);
}
