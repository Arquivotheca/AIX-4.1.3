static char sccsid[] = "@(#)05	1.6  src/bos/kernext/inputdd/common/tabletfns.c, inputdd, bos41J, 9509A_all 2/14/95 13:21:30";
/*
 * COMPONENT_NAME: (INPUTDD) Tablet DD - tabletfns.c
 *
 * FUNCTIONS:  tabletopen, tabletioctl, set_conversion, set_resolution,
 *             set_origin, set_sample_rate, set_dead_zone, get_tablet_id
 *             inittab
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

#include  "ktsm.h"
#include  "common.h"
#include  "tabext.h"

/*****************************************************************************/
/*                                                                           */
/* NAME:        tabletopen                                                   */
/*                                                                           */
/* FUNCTION:    Process open request                                         */
/*                                                                           */
/* INPUTS:      com = pointer to common area                                 */
/*              tab = pointer to tablet extension                            */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EINTR = interrupted system call                              */
/*              ENXIO = invalid device number                                */
/*              EACCES = already open                                        */
/*              EPERM = failed to register interrupt handler                 */
/*              EIO = I/O error                                              */
/*              ENOMEM = unable to pin driver, out of memory                 */
/*              ENOSPC = insufficient paging space                           */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  tabletopen(struct common *com, struct tabext *tab)
{

   int  rc, retry;
   IFRAME buff[6];
   int cnt;

                                       /* reset tabext to defaults           */
   tab->tab_conversion   = DFT_TAB_CONVERSION;
   tab->tab_resolution   = DFT_TAB_RESOLUTION;
   tab->tab_origin       = DFT_TAB_ORIGIN;
   tab->tab_sample_rate  = DFT_TAB_SAMPLE_RATE;
   tab->tabstat          = 0;
   tab->tab_hor_deadzone = 0;
   tab->tab_ver_deadzone = 0;

   set_active_area(tab);               /* set active area boundaries         */

   tab->rcb.ring_ptr = NULL;           /* indicate no input ring attached    */

   if (!(rc = reg_intr(FALSE)))  {     /* pin and register intr handler      */

     retry=R_RETRY;                    /* initialize retry counters          */
     rc = EIO;
     while(retry--) {
                                       /* set tablet to default conditions   */
       put_oq(com, tab_default_cmds);
                                       /* wait for all I/O to complete       */
       if (!wait_oq(com)) {
                                       /*  Since the input device type (puck */
                                       /*  or stylus) is not defined in the  */
                                       /*  ODM, read status and fill in here.*/
         put_oq1(com, (OFRAME) READ_TAB_STATUS_CMD);
         if ((cnt = get_iq(com, 6, RCVTO, buff)) != 6) {
           if (cnt >= 0) {             /* if no reponse then                 */
             if (retry) continue;      /* try again if retries not exhausted */
             io_error("tabletopen", FALSE, TIO_ERROR, 0, NULL);
           }
         }
         else {
           switch (buff[0] & INPUT_DEVICE_TYPE) {
                                       /* stylus input device attached       */
             case STYLUS_INPUT_DEVICE:
               tab->input_device = TABSTYLUS;
               break;
                                       /* puck input device attached         */
             case PUCK_INPUT_DEVICE:
               tab->input_device = TABPUCK;
               break;
                                       /* unsupported input device attached  */
             default:
               tab->input_device = TABUNKNOWN;
               break;
           }                           /* Enable tablet data transmission    */
           put_oq1(com, (OFRAME) ENABLE_TAB_CMD);
           rc = wait_oq(com);
         }
       }
       break;
     }
     if (rc) {
       ureg_intr();                    /* failed, unregister intr handler    */
     }
   }

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        tabletioctl                                                  */
/*                                                                           */
/* FUNCTION:    Process ioctl request                                        */
/*                                                                           */
/* INPUTS:      com = pointer to common area                                 */
/*              tab = pointer to tablet extension                            */
/*              cmd = function requested via ioctl                           */
/*              arg = pointer to argument passed via ioctl                   */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENXIO = invalid device number                                */
/*              EIO    = I/O error                                           */
/*              EINVAL = invalid cmd or argument                             */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/


int  tabletioctl(struct common *com, struct tabext *tab, int cmd, caddr_t arg)
{
  int   rc = 0;
  ulong ldata;
  uint  data;
  struct devinfo info;
  struct tabqueryid t_query;


  switch(cmd) {
                                       /*------------------------------------*/
                                       /* Return devinfo structure           */
                                       /*------------------------------------*/

    case IOCINFO:                      /* error if NULL argument passed      */
      if (arg == (caddr_t) NULL) {
        rc = EINVAL;
      }
      else {                           /* zero devinfo structure             */
        bzero(&info, sizeof(info));
                                       /* set type field                     */
        info.devtype = INPUTDEV;
                                       /* copy to caller's memory            */
        rc = copyout((char *) &info, (char *) arg,
                sizeof(info));
      }
      break;
                                       /*------------------------------------*/
                                       /* query tablet device identifier     */
                                       /*------------------------------------*/

    case TABQUERYID:                   /* error if NULL argument passed      */
      if (arg == (caddr_t) NULL) {
        rc = EINVAL;
      }
      else {                           /* copy id to user space              */
        t_query.model = tab->tablet_model;
        t_query.input_device = tab->input_device;
        rc = copyout((char *) &t_query, (char *) arg,
             sizeof(struct tabqueryid));
      }
      break;
                                       /*------------------------------------*/
                                       /* register input ring                */
                                       /*------------------------------------*/

    case TABREGRING:                   /* error if NULL argument passed      */
      if (arg == (caddr_t) NULL)  {
        rc = EINVAL;
      }
      else {                           /* register input ring in user space  */
        rc = ktsm_rring(&tab->rcb, (char *) arg);
      }

      break;
                                       /*------------------------------------*/
                                       /* flush input ring                   */
                                       /*------------------------------------*/
    case TABRFLUSH:
      ktsm_rflush(com,&tab->rcb);
      break;

                                       /*------------------------------------*/
                                       /* set tablet conversion              */
                                       /*------------------------------------*/
    case TABCONVERSION:                /* error if NULL argument passed      */
      if (arg == (caddr_t) NULL)  {
        rc = EINVAL;
      }
      else {                           /* get calling parm                   */
        if (!(rc = copyin(arg, (char *) &data, sizeof(data)))) {
                                       /* error if invalid value             */
          if ((data != TABINCH) && (data != TABCM)) {
            rc = EINVAL;
          }
          else {                       /* save conversion mode               */
            tab->tab_conversion = data;
          }
        }
      }
      break;

                                       /*------------------------------------*/
                                       /* set tablet resolution              */
                                       /*------------------------------------*/
    case TABRESOLUTION:                /* error if NULL argument passed      */
      if (arg == (caddr_t) NULL)  {
        rc = EINVAL;
      }
      else {                           /* get calling parm                   */
        if (!(rc = copyin(arg, (char *) &data, sizeof(data)))) {
                                       /* set resolution                     */
          rc = set_resolution(com, tab, data);
        }
      }
      break;

                                       /*------------------------------------*/
                                       /* set tablet origin                  */
                                       /*------------------------------------*/
    case TABORIGIN:                    /* error if NULL argument passed      */
      if (arg == (caddr_t) NULL)  {
        rc = EINVAL;
      }
      else {                           /* get calling parm                   */
        if (!(rc = copyin(arg, (char *) &data, sizeof(data)))) {
                                       /* set origin                         */
          rc = set_origin(com, tab, data);
        }
      }
      break;

                                       /*------------------------------------*/
                                       /* set tablet sample rate             */
                                       /*------------------------------------*/
    case TABSAMPLERATE:                /* error if NULL argument passed      */
      if (arg == (caddr_t) NULL)  {
        rc = EINVAL;
      }
      else {                           /* get calling parm                   */
        if (!(rc = copyin(arg, (char *) &data, sizeof(data)))) {
                                       /* set sample rate                    */
          rc = set_sample_rate(com, tab, data);
        }
      }
      break;

                                       /*------------------------------------*/
                                       /* set tablet dead zone               */
                                       /*------------------------------------*/
    case TABDEADZONE:                  /* error if NULL argument passed      */
      if (arg == (caddr_t) NULL)  {
        rc = EINVAL;
      }
      else {                           /* get calling parm                   */
        if (!(rc = copyin(arg, (char *) &data, sizeof(data)))) {
                                       /* set dead zone                      */
          rc = set_dead_zone(tab, data);
        }
      }
      break;
                                       /*------------------------------------*/
                                       /* invalid command                    */
                                       /*------------------------------------*/
    default:
      rc = EINVAL;

  }

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        set_resolution                                               */
/*                                                                           */
/* FUNCTION:    Process ioctl request to set tablet resolution               */
/*                                                                           */
/* INPUTS:      com = pointer to common area                                 */
/*              tab = pointer to tablet extension                            */
/*              argument = indicates resolution value                        */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EIO    = I/O error                                           */
/*              EINVAL = invalid cmd or argument                             */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  set_resolution(struct common *com, struct tabext *tab, uint argument)
{
   uchar    res_hi;
   uchar    res_lo;

                                       /* if resolution is in cm's           */
   if (tab->tab_conversion == TABCM) {
                                       /*   error if arg too big             */
      if (argument > MAX_TAB_RESOLUTION_CM) {
         return(EINVAL);
      }                                /*   convert arg to inches            */
      argument = ((argument * 254) + 49) / 100;
   }
   else {                              /* else resolution is in inches       */
                                       /*   error if arg too big             */
      if (argument > MAX_TAB_RESOLUTION_INCH) {
         return(EINVAL);
      }
   }
                                       /* return if no change                */
   if (tab->tab_resolution == argument) return(0);

                                       /* encode resolution                  */
   res_hi = argument / 5;
   res_lo = ((argument - (5 * (uint) res_hi)) * 256) / 5;

                                       /* send high byte of resolution       */
   put_oq2(com, (OFRAME) SET_TAB_RES_H,
        (OFRAME) ((res_hi << 8) | WRITE_TAB_CMD));
   if (wait_oq(com)) {
      return(EIO);
   }

                                       /* send low  byte of resolution       */
   put_oq2(com, (OFRAME) SET_TAB_RES_L,
        (OFRAME) ((res_lo << 8) | WRITE_TAB_CMD));
   if (wait_oq(com)) {
      return(EIO);
   }

   tab->tab_resolution = argument;     /* save resolution                    */

   set_active_area(tab);               /* set active area boundaries         */

   return(0);

}

/*****************************************************************************/
/*                                                                           */
/* NAME:        set_origin                                                   */
/*                                                                           */
/* FUNCTION:    Process ioctl request to set tablet origin                   */
/*                                                                           */
/* INPUTS:      com = pointer to common area                                 */
/*              tab = pointer to tablet extension                            */
/*              argument = indicates either lower left origin of center      */
/*                         origin                                            */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EIO    = I/O error                                           */
/*              EINVAL = invalid cmd or argument                             */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       dead zones are cleared                                       */
/*                                                                           */
/*****************************************************************************/

long  set_origin(struct common *com, struct tabext *tab, uint argument)
{

                                       /* error if invalid value specified   */
   if ((argument != TABORGLL) && (argument != TABORGC)) {
      return(EINVAL);
   }
                                       /* return if no change                */
   if (tab->tab_origin == argument) return(0);

   if (argument == TABORGLL) {
                                       /* send cmd to set origin to lower lft*/
      put_oq2(com, (OFRAME) SET_TAB_ORG, (OFRAME) SET_LOWER_LEFT);
      if (wait_oq(com)) {
         return(EIO);
      }
   }
   else{                              /* send cmd to set origin to center    */
      put_oq2(com, (OFRAME) SET_TAB_ORG, (OFRAME) SET_CENTER);
      if (wait_oq(com)) {
         return(EIO);
      }
   }

   tab->tab_origin = argument;         /* save origin specificaiton          */

   tab->tab_hor_deadzone = 0;          /* reset dead zone                    */
   tab->tab_ver_deadzone = 0;

   set_active_area(tab);               /* set active area boundaries         */

   return(0);
}



/*****************************************************************************/
/*                                                                           */
/* NAME:        set_sample_rate                                              */
/*                                                                           */
/* FUNCTION:    Process ioctl request to set tablet sample rate              */
/*                                                                           */
/* INPUTS:      com = pointer to common area                                 */
/*              tab = pointer to tablet extension                            */
/*              argument = indicates sample rate value                       */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EIO    = I/O error                                           */
/*              EINVAL = invalid cmd or argument                             */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

long  set_sample_rate(struct common *com, struct tabext *tab, uint argument)
{

                                       /* error if invalid value             */
   if ((argument < MIN_TAB_SAMPLE_RATE) || (argument > MAX_TAB_SAMPLE_RATE))
   {
      return(EINVAL);
   }

                                       /* return if no change                */
   if (tab->tab_sample_rate == argument) return(0);

                                       /* set tablet sample rate             */
   put_oq2(com, (OFRAME) SET_TAB_SAM_RATE,
       (OFRAME)((argument << 8) | WRITE_TAB_CMD));
   if (wait_oq(com)) {
      return(EIO);
   }

   tab->tab_sample_rate =  argument;

   return(0);

}

/*****************************************************************************/
/*                                                                           */
/* NAME:        set_dead_zone                                                */
/*                                                                           */
/* FUNCTION:    Process ioctl request to set tablet sample rate              */
/*                                                                           */
/* INPUTS:      tab = pointer to tablet extension                            */
/*              argument = indicates english or metric                       */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EIO    = I/O error                                           */
/*              EINVAL = invalid cmd or argument                             */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

long  set_dead_zone  (struct tabext *tab,ulong argument)
{

   ushort  hor_dz;
   ushort  ver_dz;

                                       /* extract horz/vert. values          */
   hor_dz  = (ushort)((argument >> 16) & 0x0000FFFF);
   ver_dz  = (ushort) (argument & 0x0000FFFF);
                                       /* error if too big                   */
   if ((hor_dz > MAX_TAB_DEADZONE) || (ver_dz > MAX_TAB_DEADZONE)) {
      return(EINVAL);
   }

   tab->tab_hor_deadzone =  hor_dz;
   tab->tab_ver_deadzone =  ver_dz;

   set_active_area(tab);               /* set active area boundaries         */

   return(0);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        set_active_area                                              */
/*                                                                           */
/* FUNCTION:    set active area boundaries                                   */
/*                                                                           */
/* INPUTS:      tab = pointer to tablet extension                            */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void set_active_area(struct tabext *tab)
{

   ushort   hor_dz;
   ushort   ver_dz;
   ushort   max;


                                       /* biggest x,y value                  */
   if (tab->tablet_model == TAB6093M11) {
     max = (ushort)(((uint)tab->tab_resolution * TAB_MOD_11_SIZE)/ 1000);
   }
   else {
     max = (ushort)(((uint)tab->tab_resolution * TAB_MOD_12_SIZE)/ 1000);
   }

                                       /* half it if center origin           */
   if (tab->tab_origin == TABORGC) {
      max = max / 2;
   }
                                       /* dead zone must be less than or     */
                                       /* equal to biggest x,y value         */
   hor_dz =  (tab->tab_hor_deadzone <= max) ? tab->tab_hor_deadzone :  max;
   ver_dz =  (tab->tab_ver_deadzone <= max) ? tab->tab_ver_deadzone :  max;

                                       /* set active boundaries              */
   tab->tab_hor_active   =  max - hor_dz;
   tab->tab_ver_active   =  max - ver_dz;

}

/*****************************************************************************/
/*                                                                           */
/* NAME:             get_tablet_id                                           */
/*                                                                           */
/* FUNCTION:         Reset device and get ID                                 */
/*                                                                           */
/* INPUTS:           com = pointer to common area                            */
/*                   tab = pointer to tablet extension                       */
/*                                                                           */
/* OUTPUTS:          0 = successful                                          */
/*                   EIO                                                     */
/*                   EPERM                                                   */
/*                   ENOMEM                                                  */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:            In most cases, a tablet will not be hooked to the       */
/*                   workstation, so in order to reduce the normal config    */
/*                   time, the tablet is "polled" with a read configuration  */
/*                   command. If we get a response, then a tablet is         */
/*                   attached and we go thru the entire reset procedure.     */
/*                   Note that wrap mode is cleared just in case tablet      */
/*                   was left in this mode by diagnostics.                   */
/*                                                                           */
/*****************************************************************************/

int get_tablet_id(struct common *com, struct tabext *tab)
{
  int rc, retry;
  IFRAME  model;
  int cnt;

  rc = EIO;
  retry=R_RETRY;                       /* initialize retry counters          */
  tab->tablet_model = 0;               /* clear device ID                    */
  while(retry--) {
                                       /* see if tablet is connected to      */
                                       /* workstation (see notes)            */
    put_oq2(com, (OFRAME) RESET_WRAP_MODE_CMD, (OFRAME) READ_TAB_CONFIG_CMD);
    if ((cnt = get_iq(com, 1, RCVTO, &model)) <= 0) {
      if (!cnt) {                      /* if no response                     */
        if (retry) continue;           /* try again if retries not exhausted */
#ifdef GS_DEBUG_TRACE
        io_error("get_tab_id", FALSE, TIO_ERROR, 0, NULL);
#endif
      }
    }
    else {                             /* it is so send RESET cmd to tablet  */
      put_oq1(com, (OFRAME) TAB_RESET_CMD);
      if (!wait_oq(com)) {
                                       /* read configuration                 */
        put_oq1(com, (OFRAME) READ_TAB_CONFIG_CMD);
        if ((cnt = get_iq(com, 1, RCVTO, &model)) <= 0) {
          if (!cnt) {                  /* if no response                     */
            if (retry) continue;       /* try again if retries not exhausted */
            io_error("get_tab_id", FALSE, TIO_ERROR, 1, NULL);
          }
        }
        else {

          switch (model) {
                                       /* 6093 model 11 CursorPad            */
            case TAB_MODEL_11:
              tab->tablet_model = TAB6093M11;
              rc = 0;
              break;
                                       /* 6093 model 12 Tablet               */
            case TAB_MODEL_12:
              tab->tablet_model = TAB6093M12;
              rc = 0;
              break;
                                       /* unsupported tablet attached        */
            default:
              io_error("get_tab_id", FALSE, TABTYPE_ERROR, 2, "%02x", model);
          }
        }
      }
    }
    break;
  }
  return(rc);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:             inittab                                                 */
/*                                                                           */
/* FUNCTION:         This module initializes the tablet                      */
/*                                                                           */
/* INPUTS:           com = pointer to common                                 */
/*                   tab = pointer to tablet extension                       */
/*                   dds_ptr = pointer to dds structure                      */
/*                   devno = device number                                   */
/*                                                                           */
/* OUTPUTS:          0 = ok                                                  */
/*                   !0 = error                                              */
/*                                                                           */
/* ENVIRONMENT:      Process                                                 */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int inittab(struct common *com, struct tabext *tab,
            struct ktsmdds *dds_ptr, dev_t devno)
{
  int  rc = 0;

  if (!com->adpt_devno) {              /* bad news if adapter has not been   */
    rc = EINVAL;                       /* configured                         */
  }
  else {
   if (!tab->tab_devno) {              /* skip if tablet already configured  */
  
                                       /* if run time config then            */
    if (dds_ptr->ipl_phase == RUNTIME_CFG) {

      if (!(rc=reg_intr(FALSE))) {     /*   register interrupt handler       */
                                       /*   reset tablet and get id          */
        rc = get_tablet_id(com, tab);
        ureg_intr();                   /*   unregister interrupt handler     */
        if (!rc) {                     /*   if physical tablet is not same as*/
                                       /*   defined device then              */
          if (dds_ptr->device_type != tab->tablet_model) {
            rc = ENODEV;               /*     set error code                 */
          }
        }
      }
    }
    else {                             /* else ipl config so just            */
                                       /*   set device type from dds         */
      tab->tablet_model = dds_ptr->device_type;
    }

    if (!rc) {                         /* if all ok, mark tablet configured  */
      tab->tab_devno = devno;          /* by saving device number            */
    }
   }
  }
  return(rc);
}

