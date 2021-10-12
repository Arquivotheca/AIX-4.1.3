static char sccsid[] = "@(#)85	1.2  src/bos/diag/tu/ktat/exectu.c, tu_ktat, bos41J, 9519A_all 5/3/95 15:01:56";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: exectu
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
 *****************************************************************************/

#include <stdio.h>
#include "kent_defs.h"
#include "kent_tu_type.h"

int exectu(char *device_name, TU_TYPE *tucb_ptr)
{
  int rc;
  int i;

/* if loop count is zero, return indicating no exection of TU   */

  if (tucb_ptr -> kent.loop == 0)
    return(loop_count_was_zero);

/*  call requested TU            */

  for(i = 0; i < tucb_ptr -> kent.loop; i++)
  {
    switch(tucb_ptr -> kent.tu)
    {
      case TU_OPEN:
        rc = tu_open(device_name);
        break;

      case CONFIG_REG_TEST:
        rc = tu001();
        break;
    
      case IO_REG_TEST:
        rc = tu002();
        break;
 
      case INIT_TEST:
        rc = tu003();
        break;

      case INTERNAL_WRAP1:
        rc = tu004();
        break;

      case INTERNAL_WRAP2:
        rc = tu005();
        break;

      case EXTERNAL_WRAP_AUI:
        rc = tu006();
        break;

      case EXTERNAL_WRAP_10Base2:
        rc = tu006();
        break;

      case EXTERNAL_WRAP_10BaseT:
        rc = tu008();
        break;

      case TU_CLOSE:
        rc = tu_close(device_name);
        break;

      default:
        rc = INVALID_TU_NUMBER;

    }  /*  end of switch  */

    if(rc)
    {
/*      printf("TU %2d failed, rc = 0x%08x\n", tucb_ptr ->kent.tu, rc); */
      break;
    }

  }  /*  end of for loop  */

  return(rc);

}  /*  end of exectu   */

