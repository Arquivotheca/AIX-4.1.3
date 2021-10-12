static char sccsid[] = "@(#)82  1.2  src/bos/diag/tu/wga/vpdtu.c, tu_wga, bos411, 9428A410j 4/23/93 15:31:45";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: get_machine_model
 *              set_machine_model
 *              vpd_tu
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
#include "vpd.h"


static int machine_model = DEFAULT_MACHINE_MODEL;


/*
 * NAME : vpd_tu
 *
 * DESCRIPTION :
 *
 * Tests WGA VPD. The two VPD registers are accessed and read.
 * The first register, VPD(0) is verified to contain 0x04.
 * The second register, VPD(1) is verified to contain a valid
 * EC level of the WGA datapter.
 *
 * INPUT :
 *
 *  None.
 *
 * OUTPUT :
 *
 *  None.
 *
 * RETURNS :
 *
 *  Error code or SUCCESS.
 *
*/

#define NUM_VPD_REGS          2
#define WGA_VPD_0             0x00000004         /* WGA adapter detected     */

int vpd_tu(void)
{
  int rc;
  ulong_t vpd[NUM_VPD_REGS];
  int model;

#ifdef LOGMSGS
  char msg[80];
#endif

  disable_video ();                              /* do not show random data  */
  rc = SUCCESS;
  model = get_machine_model ();                  /* get machine model        */
  get_wga_reg(&vpd[0], (uchar_t) VPD0_REG);      /* get EC level (VPD 0)     */
  get_wga_reg(&vpd[1], (uchar_t) VPD1_REG);      /* get EC level (VPD 1)     */

#ifdef DEBUG_WGA
  printf ("\nVPD data read from card:   VPD (0) = 0x%08x     VPD (1) = 0x%08x      default  model = %d",
          vpd[0], vpd[1], model);
  fflush (stdout);
#endif

  switch (model)
  {
    case MACHINE_MODEL_220 :                     /* for SGA machine          */
    case MACHINE_MODEL_230 :                     /* for WGA machine          */
            if ( (vpd[0] != WGA_VPD_0) ||
                 ((vpd[1] & EC_LEVEL_MASKS) != WGA_EC_LEVEL) )
              rc = VPD_ERR;
            break;

    default :
            rc = VPD_ERR;
            break;

  } /* endswitch */

  if (rc != SUCCESS)
  {
#ifdef LOGMSGS
    sprintf(msg, "WGA EC Level: VPD0 = 0x%08x   VPD1 = 0x%08x     default model =%d",
            vpd[0], vpd[1], model);
    LOG_ERROR (msg);
    sprintf(msg, "Expecting - VPD(0) = 0x%08x   VPD(1) = 0x%08x", WGA_VPD_0,
            WGA_EC_LEVEL);
    LOG_ERROR (msg);
#endif
  }

  enable_video ();
  return (rc);
}



/*
 * NAME : get_machine_model
 *
 * DESCRIPTION :
 *
 *   Return machine model.
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
 *   Machine model.
 *
*/

int get_machine_model (void)
{
  return (machine_model);
}



/*
 * NAME : set_machine_model
 *
 * DESCRIPTION :
 *
 *   Update the machine model flag.
 *
 * INPUT :
 *
 *   Machine model.
 *
 * OUTPUT :
 *
 *   Update the internal machine model flag
 *
 * RETURNS :
 *
 *   None.
 *
*/

void set_machine_model (int model)
{
  machine_model = model;
  return;
}
