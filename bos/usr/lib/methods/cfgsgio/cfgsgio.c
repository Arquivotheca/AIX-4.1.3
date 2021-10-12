static char sccsid[] = "@(#)41	1.2  src/bos/usr/lib/methods/cfgsgio/cfgsgio.c, inputdd, bos41J, 9516B_all 4/18/95 14:43:26";
/*
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS:
 *		err_exit
 *		err_undo
 *		main
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Include files needed by this module */
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "sgio_dds.h"
#include "cfgdebug.h"

/* external functions */
extern int pparse();
extern char *malloc();
extern int *genminor();
extern int mk_sp_file();

/* Set permissions for special files */
#define FTYPE   S_IFMPX
#define FPERM   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define MODE    FTYPE | FPERM

/*
 * NAME: main
 *
 * FUNCTION:
 *
 *  The purpose of cfgsgio is to configure the Serial GIO pseudo device
 *  into the system and make it ready for use.  It is called with the
 *  name of the logical device representing either the Dials or LPFKeys
 *  and possibly a flag representing which phase the configuration is
 *  taking place in.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 */
main(int argc, char **argv)
{
  int ipl_phase;
  int rc, errflg, c;
  int how_many;

  long majorno, minorno, *minor_ptr;


  char *logical_name;
  char criteria[256];

  struct PdDv pddv;
  struct CuAt cuat;
  struct CuDv cudv;

  struct cfg_dd dd;
  struct sgio_dds dds;

  /* Parse Parameters */
  ipl_phase = RUNTIME_CFG;
  errflg = 0;
  logical_name = NULL;
  while ((c = getopt(argc,argv,"l:12")) != EOF)
  {
    switch (c)
    {
      case 'l':
        if (logical_name != NULL)
          errflg++;
        logical_name = optarg;
        break;
      case '1':
        if (ipl_phase != RUNTIME_CFG)
          errflg++;
        ipl_phase = PHASE1;
        break;
      case '2':
        if (ipl_phase != RUNTIME_CFG)
          errflg++;
        ipl_phase = PHASE2;
        break;
      default:
        errflg++;
    }
  }

  /* If error parsing parameters... */
  if (errflg)
  {
    DEBUG_0("cfgsgio: command line error\n");
    exit(E_ARGS);
  }

  /* Validate Parameters - logical name must be specified */
  if (logical_name == NULL)
  {
    DEBUG_0("cfgsgio: logical name must be specified\n");
    exit(E_LNAME);
  }

  /* Start up odm for further use */
  if ((rc = odm_initialize()) < 0)
  {
     DEBUG_0("cfgsgio: cannot initialize ODM\n");
     exit(E_ODMINIT);
  }

  /* Get CuDv object with this logical name */
  sprintf(criteria, "name = '%s'", logical_name);
  if ((rc = (int) odm_get_first(CuDv_CLASS, criteria, &cudv)) == 0)
  {
    DEBUG_1("cfgsgio: no CuDv object found for %s\n", logical_name);
    err_exit(E_NOCuDv);
  }
  else if (rc == -1)
  {
    DEBUG_1("cfgsgio: ODM failure getting CuDv, crit=%s\n", criteria);
    err_exit(E_ODMGET);
  }

  /* Get the PdDv object for this device. The CuDv.PdDvLn_Lvalue provides
     the qualifying uniquetype for the PdDv class. */
  sprintf(criteria, "uniquetype = '%s'", cudv.PdDvLn_Lvalue);
  if ((rc = (int) odm_get_first(PdDv_CLASS, criteria, &pddv)) == 0)
  {
    DEBUG_1("cfgsgio: no PdDv object found for crit=%s\n", criteria);
    err_exit(E_NOPdDv);
  }
  else if (rc == -1)
  {
    DEBUG_1("cfgsgio: ODM failure getting PdDv, crit=%s\n", criteria);
    err_exit(E_ODMGET);
  }

  /* If this device is being configured during an ipl phase... */
  if (ipl_phase != RUNTIME_CFG)
  {
    /* Set system LEDs */
    DEBUG_0("setting leds\n");
    setleds(pddv.led);
  }

  /* Check to see if the device is already defined.  We configure the
     device only if the device is defined. Configuring the device in this
     case refers to the process of checking for attribute consistency,
     building a DDS, loading the device driver, etc...  */
  if (cudv.status == DEFINED)
  {
    /* Get CuAt ttydevice object with this logical name */
    sprintf(criteria, "name = '%s' AND attribute = 'ttydevice'", logical_name);
    if ((rc = (int) odm_get_first(CuAt_CLASS, criteria, &cuat)) == 0)
    {
      DEBUG_2("cfgsgio: no CuAt object found for %s, crit=%s\n", logical_name,
        criteria);
      err_exit(E_NOATTR);
    }
    else if (rc == -1)
    {
      DEBUG_1("cfgsgio: ODM failure getting CuAt, crit=%s\n", criteria);
      err_exit(E_ODMGET);
    }

    /* Load sgio device driver. */
    if ((dd.kmid = loadext(pddv.DvDr, TRUE, FALSE)) == NULL)
    {
      DEBUG_1("cfgsgio: error loading device driver %s\n", pddv.DvDr);
      err_exit(E_LOADEXT);
    }

    /* Initialize the dds device class */
    if (strcmp(pddv.class, "dial") == 0)
      dds.device_class = S_DIALS;
    else if (strcmp(pddv.class, "lpfk") == 0)
      dds.device_class = S_LPFKS;

    /* Initialize the dds device logical name */
    sprintf(dds.devname, "%s", logical_name);

    /* Initialize tty device */
    sprintf(dds.tty_device, "%s", cuat.value);

    /* Create a major number for this device if one has not yet been
       created. If one already exists, it will be returned. */
    if ((majorno = genmajor(pddv.DvDr)) == -1)
    {
      DEBUG_0("cfgsgio: error generating major number\n");
      err_undo(pddv.DvDr);
      err_exit(E_MAJORNO);
    }

    /* Get minor number, creating a new one if necessary. */
    minor_ptr = getminor(majorno, &how_many, logical_name);
    if (minor_ptr == NULL || how_many == 0)
    {
      if ((minor_ptr = genminor(logical_name, majorno, -1,1,1,1)) == NULL)
      {
        reldevno(logical_name, TRUE);
        err_undo(pddv.DvDr);
        DEBUG_0("cfgsgio: error creating minor number\n");
        err_exit(E_MINORNO);
      }
    }
    minorno = *minor_ptr;

    /* Create devno for this device */
    dd.devno = makedev(majorno, minorno);

    /* Initialize ddsptr and ddslen */
    dd.ddsptr = (caddr_t) &dds;
    dd.ddslen = sizeof(struct sgio_dds);

    /* Now call sysconfig() to configure sgio.  */
    dd.cmd = CFG_INIT;
    if (sysconfig(SYS_CFGDD, &dd, sizeof(struct cfg_dd )) == -1)
    {
      DEBUG_1("cfgsgio: error configuring driver of %s", logical_name);
      err_undo(pddv.DvDr);
      err_exit(E_CFGINIT);
    }

    /* Now make the special files that this device will be accessed through. */
    if (mk_sp_file(dd.devno, logical_name, MODE) != 0)
    {
      DEBUG_2("cfgsgio: error making special file for %s, devno=0x%x",
        logical_name, dd.devno);

      /* Terminate and unload device */
      dd.cmd = CFG_TERM;
      if (sysconfig(SYS_CFGDD, &dd, sizeof(struct cfg_dd )) == -1)
      {
        DEBUG_1("cfgsgio: error unconfiguring driver of %s", logical_name);
        err_exit(E_CFGTERM);
      }
      err_undo(pddv.DvDr);
      return(-1);
    }

    /* Update the CuDv object to reflect the device's current status.
       The device status field should be changed to AVAILABLE. */
    cudv.status = AVAILABLE;
    if ((rc = odm_change_obj(CuDv_CLASS, &cudv)) < 0)
    {
      DEBUG_0("cfgsgio: ODM failure updating CuDv object\n");
      err_exit(E_ODMUPDATE);
    }
  }

  /* Terminate the ODM and exit with success */
  odm_terminate();
  exit(E_OK);
}

/*
 * NAME: err_exit
 *
 * FUNCTION: Terminates ODM.  Used to back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.
 *
 * NOTES:
 *
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */

err_exit(char exitcode)
{
  /* Terminate the ODM. */
  odm_terminate();
  exit(exitcode);
}

/*
 * NAME: err_undo
 *
 * FUNCTION: Unloads the device's driver.  Used to back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.
 *
 * NOTES:
 *
 * void
 *   err_undo( DvDr )
 *      DvDr = pointer to device driver name.
 *
 * RETURNS:
 *               None
 */

err_undo(char *DvDr)
{
  /* Unload device driver. */
  if (loadext(DvDr,FALSE,FALSE) == NULL)
  {
    DEBUG_0("cfgsgio: error unloading driver\n");
  }
  return;
}
