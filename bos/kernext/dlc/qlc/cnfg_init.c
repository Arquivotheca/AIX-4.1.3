static char sccsid[] = "@(#)46	1.6  src/bos/kernext/dlc/qlc/cnfg_init.c, sysxdlcq, bos411, 9430C411a 7/25/94 11:31:40";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: config_init
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/device.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/pin.h>

#define E_SOFTWARE 1
#define E_UNIX 2
#define E_UNIXDD1 3
#define E_INFO 4

struct dlcconfig {
  dev_t   dev;
};

extern void qpm_receive_function();
extern void qpm_exception_function();
extern int qlcconfig();
extern int qlcmpx();
extern int qlcopen();
extern int qlcclose();
extern int qlcread();
extern int qlcwrite();
extern int qlcioctl();
extern int qlcselect();
extern int nodev();

/*****************************************************************************/
/* Function:        config_init                                              */
/*                                                                           */
/* Description: System INIT procedure for config routine.                    */
/*                                                                           */
/* Function:    Config is called from qlcconfig for the INIT case.           */
/*                                                                           */
/* Input:       dev     - device number.                                     */
/*              uiop    - pointer to the uio structure                       */
/*                                                                           */
/* Output:                                                                   */
/*                                                                           */
/* Normal Exit:                                                              */
/*                                                                           */
/* Error Exit:                                                               */
/*                                                                           */
/* Return Type:                                                              */
/*                                                                           */
/*****************************************************************************/
int config_init(
  struct uio  *uiop)

{
  int                     rc;
  int							  rc1;
  int							  rc2;
  struct devsw            dlcdev;
  struct dlcconfig        config;

  rc = uiomove(&config, sizeof (config), UIO_WRITE, uiop); 

  /* config.dev = makedev(24,0); */ 


#ifdef WSD_LOCAL_BUILD
  (void)printf("major=%d minor=%d\n",major(config.dev),minor(config.dev));
  (void)printf("qlcopen  =%x\n",  qlcopen);
  (void)printf("qlcclose =%x\n",  qlcclose);
  (void)printf("qlcread  =%x\n",  qlcread);
  (void)printf("qlcwrite =%x\n",  qlcwrite);
  (void)printf("qlcioctl =%x\n",  qlcioctl);
#endif

  /***************************************************************************/
  /* Define device to device switch                                          */
  /***************************************************************************/
  dlcdev.d_open = qlcopen;
  dlcdev.d_close = qlcclose;
  dlcdev.d_read = qlcread;
  dlcdev.d_write = qlcwrite;
  dlcdev.d_ioctl = qlcioctl;
  dlcdev.d_strategy = nodev;
  dlcdev.d_select = qlcselect;
  dlcdev.d_config = qlcconfig;
  dlcdev.d_print = nodev;
  dlcdev.d_dump = nodev;
  dlcdev.d_mpx = qlcmpx;
  dlcdev.d_revoke = nodev;
  dlcdev.d_opts = DEV_MPSAFE;    /* defect 155399 */


  rc = devswadd(config.dev,&dlcdev);

  if (rc != 0)
  {
    (void)printf("dlcconfig : devswadd failed.\n");
    return(-1);
  }
  else
  {
    rc1 = pincode( (void *) qpm_receive_function );
    rc2 = pincode( (void *) qpm_exception_function );
    rc = pincode( (void *) qlcconfig );
    if( rc1 != 0 || rc2 != 0 || rc != 0)
    {
      (void)devswdel(config.dev);
      return(rc);
    }
    else
      return(0);
  }
}
