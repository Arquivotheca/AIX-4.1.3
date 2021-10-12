static char sccsid[] = "@(#)52	1.1  src/bos/usr/lib/methods/startsgio/startsgio.c, inputdd, bos41J, 9510A_all 2/28/95 15:57:10";
/*
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS: main
 *              config_sgio
 *              err_exit
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Include files needed by this module */
#include <sys/types.h>
#include <sys/errno.h>
#include <stdio.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"

/* Local defines */
#define FALSE 0
#define TRUE  1

/*
 *****************************************************************************
 * NAME:  main
 *
 * FUNCTION: 
 *
 *   The purpose of this program is to configure defined serial Dials/LPFKeys.
 *   Configuration of serial Dials/LPFKeys depends upon the devices being
 *   previously defined.
 *
 *****************************************************************************
 */
main(int argc, char **argv)
{
  int c, ipl_phase, rc;

  /* Retrieve ipl phase from argument list */
  ipl_phase = PHASE2;
  while((c=getopt(argc,argv,":123?"))!=EOF)
  {
      switch(c)
      {
          case '1':
              ipl_phase = PHASE1;
              break;
          case '2':
              ipl_phase = PHASE2;
              break;
          case '3':
              ipl_phase = PHASE2MAINT;
              break;
          default:
              break;
      }
  }


  /* Start up ODM for further use */
  if ((rc = odm_initialize()) < 0)
  {
     DEBUG_0("startsgio: cannot initialize ODM\n");
     exit(E_ODMINIT);
  }

  /* Configure defined Dials/LPFKeys */
  start_sgio("dials", ipl_phase);
  start_sgio("lpfkeys", ipl_phase);
	
  /* Terminate the ODM connection and exit with success */
  odm_terminate();
  exit(E_OK);
}

/*
 *****************************************************************************
 * NAME:  start_sgio
 *****************************************************************************
 */
start_sgio(char *sgiotype, int ipl_phase)
{
  struct CuAt       cuat;           /* customized device object */
  struct CuDv       cudv;           /* customized device object */
  struct PdDv       pddv;           /* predefined device object */
  char              criteria[256];  /* search criteria string */
  int               device_found;   /* CuDv object located flag */
  int               rc;             /* return status */

  /* Get the first predefined device object for the sgiotype device
     with subclass sgio. */
  sprintf(criteria, "type = '%s' AND subclass = 'sgio'", sgiotype);
  if ((rc = (int) odm_get_obj(PdDv_CLASS, criteria, &pddv, ODM_FIRST)) == 0)
  {
    DEBUG_1("startsgio: no PdDv object found for serial %s device\n", sgiotype);
    err_exit(E_NOPdDv);
  }
  else if (rc == -1)
  {
    DEBUG_1("startsgio: ODM failure getting PdDv for %s\n", sgiotype);
    err_exit(E_ODMGET);
  }

  /* Get all defined sgiotype devices. The pddv.uniquetype provides the
     qualifying PdDvLn for the CuDv class. */
  device_found = FALSE;
  sprintf(criteria, "PdDvLn = '%s' AND status = '%d'", pddv.uniquetype, DEFINED);
  rc = (int) odm_get_obj(CuDv_CLASS, criteria, &cudv, ODM_FIRST);
  do
  {
    if ((rc == 0) && (device_found == FALSE))
    {
      DEBUG_1("startsgio: no defined serial %s devices found\n", sgiotype);
      break;
    }
    else if (rc == -1)
    {
      DEBUG_1("startsgio: ODM failure getting CuDv for %s\n", sgiotype);
      err_exit(E_ODMGET);
    }
    else if (cudv.status == DEFINED) 
    {
      if ((ipl_phase == PHASE2) || (ipl_phase == PHASE2MAINT))
      {
        /* Get autoconfig attribute. The cudv.name provides the qualifying
           (logical) name for the CuAt class.  */
        sprintf(criteria, "name = '%s' AND attribute = 'autoconfig'", cudv.name);
        if ((rc = (int) odm_get_first(CuAt_CLASS, criteria, &cuat)) == 0)
        {
          DEBUG_2("startsgio: no CuAt object found for %s, crit=%s\n",
            logical_name, criteria);
        }
        else if (rc == -1)
        {
          DEBUG_1("cfgsgio: ODM failure getting CuAt, crit=%s\n", criteria);
          err_exit(E_ODMGET);
        }
        else
        {
          /* If user has specified device not be configured during ipl... */
          if (strcmp(cuat.value, "defined") == 0)
            continue;
        }
      }

      /* echo out serial device name so it will be configured by cfgmgr */
      fprintf(stdout, "%s\n", cudv.name);
    }
 
    /* Object located for sgiotype device (status not important) */
    device_found = TRUE;
  }
  while ((rc = (int) odm_get_obj(CuDv_CLASS, criteria, &cudv, ODM_NEXT)) != 0);
}

err_exit(char exitcode)
{
  odm_terminate();
  exit(exitcode);
}
