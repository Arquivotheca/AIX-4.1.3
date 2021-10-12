static char sccsid[] = "@(#)52  1.1.1.4  src/bos/diag/tu/ethsal/iotu.c, tu_ethsal, bos411, 9428A410j 3/24/94 17:45:31";
/*
 *   COMPONENT_NAME: tu_ethsal
 *
 *
 *   FUNCTIONS: io_tu
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
#include <sys/mdio.h>
#include <stdio.h>

#include "exectu.h"
#include "tu_type.h"

#define FUSE_MASK         0x2
#define GOOD_FUSE(reg)     ((reg & FUSE_MASK) == FUSE_MASK)
#define BIT_SET(reg,mask) ((reg & mask) != 0)
#define SYNC_BIT          0x10
#define NUM_ERRORS        4

/*
 * NAME : io_tu
 *
 * DESCRIPTION :
 *
 * The following tests on I/O registers are performed :
 *
 *  1. Fuse test.
 *  2. Checks that sync. error, parity, feedback, and
 *     channel check bits are off.
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

int io_tu(void)
{
  int i, rc, mdd_fd;
  ulong_t status, mask;

  static int err_code[NUM_ERRORS] = { SYNC_BIT_ERR, PARITY_BIT_ERR,
                                      FEEDBACK_ERR, CHANNEL_CHECK_ERR,
                                    };

  rc = SUCCESS;
  mdd_fd = get_fd(MDD_FD);

  start_ether();  /* Enable eth. card */

  if(rc == SUCCESS)
    rc = get_word(mdd_fd, &status, IO_STATUS_REG_ADDR, MIOBUSGET);

  if (rc == SUCCESS)
  {
    if (power_flag)
    {
     if(!GOOD_FUSE(status))
       rc = FUSE_ERR;
    }

    else
    {
     if (GOOD_FUSE(status))  /* For RSC products, the logic is reversed for
			        the fuse status bit */
       rc = FUSE_ERR;	  
    }
  }

  if(rc == SUCCESS)
  {
    for(i = 0, mask = SYNC_BIT; i < NUM_ERRORS; i++)
    {
      if(BIT_SET(status, mask))
      {
        rc = err_code[i];
        break;
      }

      mask <<= 1;
    }
  }

  halt_ether();  

  return(rc);
}




