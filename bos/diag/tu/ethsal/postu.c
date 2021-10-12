static char sccsid[] = "@(#)53  1.1.1.2  src/bos/diag/tu/ethsal/postu.c, tu_ethsal, bos411, 9428A410j 10/20/93 14:55:41";
/*
 *   COMPONENT_NAME: tu_ethsal
 *
 *
 *   FUNCTIONS: pos_tu, enable_eths, disable_eths, check_pos_id, 
 *              check_pos_status, pos_wr_test
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <stdio.h>
#include <sys/mdio.h>

#include "exectu.h"
#include "tu_type.h"

static int check_pos_id(int);
static int check_pos_status(int);
static int pos_wr_test(int);
static int disable_eths(void);

/*
 * NAME : pos_tu
 *
 * DESCRIPTION :
 *
 * Exercisers the POS registers. The following tests
 * are performed :
 *
 *  1. POS ID is verified.
 *  2. Channel check bit in POS status register should
 *     be off.
 *  3. POS write/read/verify test.
 *
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int pos_tu(void)
{
  int rc, fd;

  rc = SUCCESS;
  fd = get_fd(MDD_FD);
  rc = check_pos_id(fd);

  if(rc == SUCCESS)
    rc = check_pos_status(fd);

 /* Don't run following test */
/*
  if(rc == SUCCESS)
   {
    rc = pos_wr_test(fd);
   }
*/

  return(rc);
}



/*
 * NAME : enable_eths
 *
 * DESCRIPTION :
 *
 * The Ethernet adapter is enabled by enabling the appropriate
 * bits in the Component Reset and Ethernet Control registers.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   Ethernet card is enabled.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

/* enable Ethernet adapter */

#define UNRESET_ETHER   0x2  /* in component reset register  */
#define UNRESET_SIO     0x1  /* in component reset register  */
#define ENABLE_ETHER    0x1  /* in Ethernet Control Register */

int enable_eths(void)
{
  uchar_t creg;
  ulong_t lreg;
  int rc, mdd_fd;

  rc = SUCCESS;
  mdd_fd = get_fd(MDD_FD);

/* enable standard I/O in Component Reset Register */

  rc = get_word(mdd_fd, &lreg, creset_reg_addr, MIOBUSGET);

  if(rc == SUCCESS)
  {
    if ((lreg & UNRESET_ETHER) == 0 ||
                 (lreg & UNRESET_SIO) == 0)
    {
      lreg |= ((ulong_t) UNRESET_ETHER | (ulong_t) UNRESET_SIO);
      rc = put_word(mdd_fd, lreg, creset_reg_addr, MIOBUSPUT);
    }
  }

  if(rc == SUCCESS)
    rc = Get_byte(mdd_fd, (uchar_t *) &creg, (ulong_t) CONTROL_REG_ADDR, MIOCCGET);

  if(rc == SUCCESS)
  {
    if((creg & ENABLE_ETHER) == 0)
    {
      creg |= ENABLE_ETHER;
      rc = put_byte(mdd_fd, (uchar_t) creg, (ulong_t) CONTROL_REG_ADDR, MIOCCPUT);
    }
  }

  return(rc);
}



/*
 * NAME : disable_eths
 *
 * DESCRIPTION :
 *
 * Disables the Ethernet adapter.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

static int disable_eths(void)
{
  uchar_t creg;
  int rc, mdd_fd;

  rc = SUCCESS;
  mdd_fd = get_fd(MDD_FD);
  rc = Get_byte(mdd_fd, (uchar_t *) &creg, (ulong_t) CONTROL_REG_ADDR, MIOCCGET);

  if(rc == SUCCESS)
  {
    creg &= ~ENABLE_ETHER;
    rc = put_byte(mdd_fd, (uchar_t) creg, (ulong_t) CONTROL_REG_ADDR, MIOCCPUT);
  }

  return(rc);
}




/*
 * NAME : check_pos_id
 *
 * DESCRIPTION :
 *
 * Ethernet POS ID is verified.
 *
 * INPUT :
 *
 *   1. Machine device driver file descriptor.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

#define NUM_POS_ID_REGS 2

static int check_pos_id(int fd)
{
  int rc, i;
  uchar_t pos;
  char msg[80];
  uchar_t pos_id[NUM_POS_ID_REGS];

  if (power_flag) {
     pos_id[0] = POS0_ID_POWER;
     pos_id[1] = POS1_ID_POWER;
  }
  else {
     pos_id[0] = POS0_ID_RSC;
     pos_id[1] = POS1_ID_RSC;
  }

  rc = SUCCESS;

  for(i = 0; rc == SUCCESS && i < NUM_POS_ID_REGS; i++)
  {
    if((rc = Get_byte(fd, (uchar_t *) &pos, (ulong_t) ID1_REG_ADDR + i, MIOCCGET)) == SUCCESS)
    {
      if(pos != pos_id[i])
      {
        sprintf(msg, "Bad POS%d read=%02x  ref=%02x", i, pos, pos_id[i]);
        LOG_ERROR(msg);
        rc = POS_ID_ERR;
      }
    }
  }

  return(rc);
}


/*
 * NAME : check_pos_status
 *
 * DESCRIPTION :
 *
 *  Verifies that the channel check bit in the POS status
 *  is off.
 *
 * INPUT :
 *
 *   1. Machine device driver file descriptor.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

#define CHANNEL_CHECK(reg) ((reg & 0x80) != 0x80)

static int check_pos_status(int fd)
{
  int rc, i;
  uchar_t pos;
  char msg[80];

  rc = SUCCESS;

  if((rc = Get_byte(fd, (uchar_t *) &pos, (ulong_t) POS_STATUS_REG_ADDR, MIOCCGET)) == SUCCESS)
  {
    if(CHANNEL_CHECK(pos))
    {
      sprintf(msg, "Bad status reg. POS5 = %02x", pos);
      LOG_ERROR(msg);
      rc = POS_STATUS_REG_ERR;
    }
  }

  return(rc);
}



/*
 * NAME : pos_wr_test
 *
 * DESCRIPTION :
 *
 *  A write/read/verify algorithm is performed on the
 *  control, DMA, and VPD address POS registers.
 *
 * INPUT :
 *
 *   1. Machine device driver file descriptor.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

#define NUM_RW_REGS    3
#define TEST_DATA_NUM  4

static int pos_wr_test(int fd)
{
  int rc, i, j;
  uchar_t tmp, ref, pos_save[NUM_RW_REGS];
  char msg[80];

  static test_data[TEST_DATA_NUM] = { 0x0, 0xaa, 0x55, 0xff };

  static struct
  {
    uchar_t mask;
    ulong_t addr;
    int err_code;
  } pos[NUM_RW_REGS] =
    {
      0x06, CONTROL_REG_ADDR, CONTROL_REG_ERR,        /* POS # 2 */
      0x1f, DMA_CONTROL_REG_ADDR, DMA_CONTROL_REG_ERR,/* POS # 4 */
      0xff, VPD_ADDR_REG_ADDR, VPD_ADDR_REG_ERR,      /* POS # 6 */
    };

  rc = SUCCESS;
  rc = disable_eths();

  for(i = 0; i < NUM_RW_REGS && rc == SUCCESS; i++)
  {
    /* Do NOT test VPD_ADDR_REG if a PowerPC machine */
    if (! ((power_flag) && (pos[i].addr == VPD_ADDR_REG_ADDR)) )
    {
      if((rc = Get_byte(fd, (uchar_t *) &pos_save[i], (ulong_t) pos[i].addr, MIOCCGET)) == SUCCESS)
      {
        sprintf(msg, "(%d) %02x", i, pos_save[i]);
        DEBUG_MSG(msg);

        for(j = 0; j < TEST_DATA_NUM && rc == SUCCESS; j++)
        {
          ref = test_data[j] & pos[i].mask;

          if((rc = put_byte(fd, (uchar_t) ref, pos[i].addr, MIOCCPUT)) == SUCCESS)
          {
            if((rc = Get_byte(fd, &tmp, pos[i].addr, MIOCCGET)) == SUCCESS)
              if((tmp & pos[i].mask) != ref)
                rc = pos[i].err_code;
          }
        }

  /* restore register */

        put_byte(fd, pos_save[i], pos[i].addr, MIOCCPUT);
      }
    }
  }

  return(rc);
}




