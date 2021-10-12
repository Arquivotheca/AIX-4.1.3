static char sccsid[] = "@(#)87	1.5  src/bos/kernext/inputdd/tabletdd/tabconfig.c, inputdd, bos41J, 9509A_all 2/14/95 13:21:12";
/*
 * COMPONENT_NAME: (INPUTDD) Tablet DD - tabconfig.c
 *
 * FUNCTIONS: tabconfig, cleanup, addswitch, qvpd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include  "tab.h"

lock_t  tab_lock=LOCK_AVAIL;           /* process level lock                */


/*****************************************************************************/
/*                                                                           */
/* NAME:             tabconfig                                               */
/*                                                                           */
/* FUNCTION:         This module manages the task of allocating storage for  */
/*                   and configuring the tablet                              */
/*                                                                           */
/* INPUTS:           devno = major and minor device number                   */
/*                   cmd   = command, i.e. what to do: initialize, terminate,*/
/*                              or query vital product data                  */
/*                   uiop  = pointer to a uio structure containing the dds   */
/*                                                                           */
/* OUTPUTS:          0      = success                                        */
/*                   ENODEV                                                  */
/*                   ENOMEM                                                  */
/*                   EINVAL                                                  */
/*                   EPERM                                                   */
/*                   ENXIO                                                   */
/*                   EBUSY                                                   */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  tabconfig(dev_t devno, long cmd, struct uio *uiop)
{

  struct ktsmdds    dds;               /* local copy of dds                  */
  struct tabext *tab;
  long               rc = 0;

  KTSMDTRACE(tabconfig, enter, devno, cmd, 0, 0, 0);


                                       /* get lock                           */
  if (lockl(&tab_lock,LOCK_SHORT) != LOCK_SUCC) {
    rc = EPERM;
  }
  else {

    switch(cmd) {                    /* switch on command                  */

                                       /*------------------------------------*/
        case CFG_INIT:                 /* case: initialize                   */
                                       /*------------------------------------*/


                                       /* move dds to kernel memory          */
          if (uiop == (struct uio *) NULL) {
             rc = EPERM;
             break;
          }
          if (rc = uiomove((caddr_t) &dds, sizeof(struct ktsmdds),
             UIO_WRITE, uiop)) {
            if (rc < 0) rc = EPERM;
            break;
          }
                                       /* allocate local storage             */
          if (!local) {
            local = (struct local *)
                xmalloc(sizeof(struct local), 2, pinned_heap);
            if (!local) {
              rc = ENOMEM;
              break;
            }
                                       /* clear out local storage            */
            bzero ((char *) local, sizeof(struct local));

                                       /* allocate I/O wait timer            */
            if ((local->com.stimer = talloc()) == NULL ){
              rc = EPERM;              /* ...error                           */
              break;
            }

                                       /* allocate watch dog timer           */
            if ((local->com.wdtimer = talloc()) == NULL ) {
              rc = EPERM;              /* ...error                           */
              break;
            }
                                       /* initialize sleep anchor            */
            local->com.asleep = EVENT_NULL;
          }

                                       /* configuring adapter                */
          if (dds.device_class == ADAPTER) {
            rc = initadpt(&dds, devno);
          }
          else {                       /* configuring tablet                 */
            if (dds.device_class == TABLET) {
              rc = inittab(&local->com, &local->tab, &dds, devno);
            }
            else {
              rc = EINVAL;              /*  invalid device class              */
            }
          }
          break;



                                       /*------------------------------------*/
      case CFG_TERM:                   /* case: terminate                    */
                                       /*------------------------------------*/

        if (!local) {                  /* error if not configured            */
          rc = ENXIO;
        }
        else {    
          tab = &local->tab;           /* pointer to tablet extension        */
                                       /* terminating adapter                */
          if (devno == local->com.adpt_devno) {
                                       /*   error if tablet is configured    */
            if (tab->tab_devno) {
                rc = EBUSY;
            }
            else {                     /*   mark adapter as termianted       */
              local->com.adpt_devno = 0;
            }
          }
          else {                       /* terminating tablet                 */
            if (devno == tab->tab_devno) {
              if (tab->oflag) {        /*   error if device is open          */
                rc = EBUSY;
              }
              else {                   /*   mark tablet as terminated        */
                tab->tab_devno = 0;
              }
            }
            else {                     /* error if not any known device      */
              rc = ENXIO;
            }
          }
        }
        break;


                                       /*------------------------------------*/
      case CFG_QVPD:                   /* case: query vital product data     */
                                       /*------------------------------------*/

        if (!local) {                  /* error if not configured            */
          rc = ENXIO;
        }
        else {                         /* only valid if for adapter          */
          if (devno != local->com.adpt_devno) {
            rc = EINVAL;
          }
          else {                       /* error if no uio structure          */
            if (uiop == (struct uio *) NULL) {
              rc = EINVAL;
            }
            else {
              rc = qvpd(uiop);
            }
          }
        }
        break;

                                       /*------------------------------------*/
        default:                       /* default: unknown command           */
                                       /*------------------------------------*/

          rc = EINVAL;
          break;

      }
      cleanup(devno);                  /* release resource                   */
      unlockl(&tab_lock);              /* release lock                       */
  }

  KTSMTRACE(tabconfig, exit, rc, devno, cmd,  0, 0);

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             cleanup                                                 */
/*                                                                           */
/* FUNCTION:         this module frees all local resources                   */
/*                                                                           */
/* INPUTS:           devno = major and minor device numbers                  */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void cleanup(dev_t devno)
{
  struct tabext *tab;

  tab = &local->tab;                   /* pointer to tablet extension        */

   if (local) {                        /* if resources allocated and         */
                                       /* nothing configured then            */
     if ((!local->com.adpt_devno) &&
       (!tab->tab_devno)) {

       devswdel(devno);                /* clear switch table                 */

       if (local->com.stimer)          /* free I/O wait timer                */
         tfree(local->com.stimer);
       if (local->com.wdtimer)         /* free watch dog timer               */
         tfree(local->com.wdtimer);
                                       /* clear out local memory             */
       bzero ((char *) local, sizeof(struct local));
                                       /* free it                            */
       xmfree((caddr_t) local, pinned_heap);

       local = NULL;
     }
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             initadpt                                                */
/*                                                                           */
/* FUNCTION:         This module initializes the adapter                     */
/*                                                                           */
/* INPUTS:           devno = device number                                   */
/*                   dds_ptr = pointer to dds structure                      */
/*                                                                           */
/* OUTPUTS:          0 = ok                                                  */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int initadpt(struct ktsmdds *dds_ptr, dev_t devno)
{
   int rc = 0;

   if (!local->com.adpt_devno) {       /* skip if already configured         */
                                       /* save bus ID                        */
     local->com.bus_id = dds_ptr->bus_id | 0x000C0020;
                                       /* save slot address                  */
     local->com.slot_addr = (dds_ptr->slot_addr << 16) | 0x00400000;
                                       /* save adapter I/O address           */
     local->com.bus_addr = dds_ptr->bus_io_addr;
                                       /* save interrupt level               */
     local->com.intr_level = dds_ptr->bus_intr_lvl;
                                       /* save interrupt priority            */
     local->com.intr_priority = dds_ptr->intr_priority;
     if (!(rc = reg_intr(TRUE))) {     /* reset adapter                      */
       ureg_intr();
       rc = addswitch(devno);          /* add device to switch table         */
       if (!rc)                        /* if all ok then mark adapter        */
                                       /* configured by saving our devno     */
         local->com.adpt_devno = devno;  
     }
   }
   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             addswitch                                               */
/*                                                                           */
/* FUNCTION:         This module adds the device driver to the device        */
/*                   switch table.                                           */
/*                                                                           */
/* INPUTS:           devno = major and minor device numbers                  */
/*                                                                           */
/* OUTPUTS:          0      = success                                        */
/*                   EINVAL                                                  */
/*                   EEXIST                                                  */
/*                   ENOMEM                                                  */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:            Outputs other than 0 are returned by the devswadd       */
/*                   kernel service.                                         */
/*                                                                           */
/*****************************************************************************/

int  addswitch(dev_t devno)
{

   int           rc;
   struct devsw  tmp_devsw;

   void nodev(void);
                                       /* copy pointers to functions into    */
                                       /* the local device switch table      */
   tmp_devsw.d_open     = (int (*) ()) tabopen;
   tmp_devsw.d_close    = (int (*) ()) tabclose;
   tmp_devsw.d_read     = (int (*) ()) nodev;
   tmp_devsw.d_write    = (int (*) ()) nodev;
   tmp_devsw.d_ioctl    = (int (*) ()) tabioctl;
   tmp_devsw.d_strategy = (int (*) ()) nodev;
   tmp_devsw.d_select   = (int (*) ()) nodev;
   tmp_devsw.d_config   = (int (*) ()) tabconfig;
   tmp_devsw.d_print    = (int (*) ()) nodev;
   tmp_devsw.d_dump     = (int (*) ()) nodev;
   tmp_devsw.d_mpx      = (int (*) ()) nodev;
   tmp_devsw.d_revoke   = (int (*) ()) nodev;
   tmp_devsw.d_dsdptr   = NULL;
   tmp_devsw.d_ttys     = NULL;
   tmp_devsw.d_selptr   = NULL;
   tmp_devsw.d_opts     = 0;
                                       /* add to the device switch table     */
   return(devswadd(devno, &tmp_devsw));

}


/*****************************************************************************/
/*                                                                           */
/* NAME:             qvpd                                                    */
/*                                                                           */
/* FUNCTION:         This module returns information on what device(s)       */
/*                   are connected to adapter                                */
/*                                                                           */
/* INPUTS:           uio pointer which points to an area of memory where the */
/*                      data will be stored                                  */
/*                                                                           */
/* OUTPUTS:          0      = success                                        */
/*                   EPERM                                                   */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  qvpd(struct uio *uiop )
{
   uchar  data[31];
   int rc;
   int index = 0;
   struct tabext *tab;

   tab = &local->tab;                  /* pointer to tablet extension        */

                                       /* get tablet ID if tablet has        */
   if (!tab->tab_devno) {              /* not yet been configured            */
     if (!reg_intr(FALSE)) {
        get_tablet_id(&local->com, tab);
        ureg_intr();
     }
   }

   switch (tab->tablet_model)
   {
      case TAB6093M11:
         bcopy("tablet/std_t/6093_m11 tablet ", data ,29);
         index = 29;
         break;
      case TAB6093M12:
         bcopy("tablet/std_t/6093_m12 tablet ", data ,29);
         index = 29;
         break;
      default:
         break;
   }

   data[index] = '\0';                 /* null terminate                     */

                                       /* write data out to user space       */
   rc = uiomove(data, index+1, UIO_READ, uiop);
   if (rc < 0) rc = EPERM;

   return(rc);
}

