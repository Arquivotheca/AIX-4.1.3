static char sccsid[] = "@(#)71  1.2  src/bos/diag/tu/wga/inttu.c, tu_wga, bos411, 9428A410j 4/23/93 15:29:23";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: interrupt_tu
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <stdio.h>

#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"
#include "wga_reg.h"
#include "wga_regval.h"

/*
 * NAME : interrupt_tu
 *
 * DESCRIPTION :
 *
 * Attempts to write to a reserved memory area.
 * Status register should set bit 1. ERRADDR_REG should show address of
 * memory location that we tried to access. (Normally this function will
 * generate an interrupt, but we are running with interrupts disabled).
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

#define RESERVED_ADDR  (ADSTAT_ADDR + 1)         /* illegal location         */

int interrupt_tu(void)
{
  ulong_t tmp, adstat, err_reg, int_reg, interrupt;
  int rc;

  rc = SUCCESS;
  disable_video ();                              /* do not show random data  */

  disable_int (ERROR_INT);
  /* perform an access to an illegal location  for the WGA */
  tmp = WGA_REG_READ (RESERVED_ADDR);

  get_wga_reg (&adstat, (uchar_t) ADSTAT_REG);
  get_wga_reg (&err_reg, (uchar_t) ERRADDR_REG);
  adstat &= STAT_ERR_MASK;

  if (adstat != RESV_ACCESS)
    rc = BAD_ERR_ST_ERR;
  else
    if (err_reg != ((ulong_t) RESERVED_ADDR -  bus_base_addr))
       rc = BAD_ERRADDR_ERR;

  /* generate W8720 interrupt (c.vlanked, c.picked, c.de_idle                */
  interrupt = VBLANKED_INT;                        /*setup VBLANKED interrupt*/
  write_igc_reg ((uchar_t) INTERRUPT_REG, interrupt);
  write_igc_reg ((uchar_t) INTERRUPT_ENBL_REG,
                   (ulong_t) (MASTER_ENABLE_INT | VBLANKED_INT));
  get_igc_reg ((uchar_t) INTERRUPT_REG, &tmp);
  if ((tmp & VBLANKED_INT) == 0)
    rc = BAD_VBLANKED_INT_ERR;

  interrupt = DE_IDLE_INT;                       /* setup DE_IDLE interrupt  */
  write_igc_reg ((uchar_t) INTERRUPT_REG, interrupt);
  write_igc_reg ((uchar_t) INTERRUPT_ENBL_REG,
                   (ulong_t) (MASTER_ENABLE_INT | DE_IDLE_INT));
  get_igc_reg ((uchar_t) INTERRUPT_REG, &tmp);
  if ((tmp & DE_IDLE_INT) == 0)
    rc = BAD_DE_IDLE_INT_ERR;

  enable_video ();

  return (rc);
}

