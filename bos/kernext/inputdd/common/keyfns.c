static char sccsid[] = "@(#)00  1.17  src/bos/kernext/inputdd/common/keyfns.c, inputdd, bos41J, 9510A_all 3/7/95 09:55:21";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard DD - keyfns.c
 *
 * FUNCTIONS: keympx, keyopen, keyclose, keyioctl, keysetup,
 *            free_kap, kring, get_kbd_id, exit_diag, initkbd
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

#include "ktsm.h"
#include "common.h"
#include "kbdext.h"
#include <sys/ktsmdds.h>

                                      /* copy to local memory                */
#define COPYIN(a)                                                             \
   if (flag & DKERNEL) {                                                      \
      bcopy(arg, (char *) &(a), sizeof((a)));                                 \
   }                                                                          \
   else {                                                                     \
      rc = copyin(arg, (char *) &(a), sizeof(a));                             \
   }
                                      /* copy from local memory              */
#define COPYOUT(a)                                                            \
   if (flag & DKERNEL) {                                                      \
      bcopy((char *) &(a), (char *) arg,  sizeof(a));                         \
   }                                                                          \
   else {                                                                     \
      rc = copyout((char *) &(a), (char *) arg, sizeof((a)));                 \
   }

#define TYPAPARM       3              /* array index of typamatic rate/delay */
#define KOREANPARM     13             /* array index to set key 1 M/B only   */
#define NUMLOCKPARM    14             /* array index to set NumLock M/B only */
#define CHGLEDMASK     (KSCAPLOCK | KSNUMLOCK | KSCROLLLOCK)

extern void   *service_vector[];

/*****************************************************************************/
/*                                                                           */
/* NAME:        keympx                                                       */
/*                                                                           */
/* FUNCTION:    Allocate/deallocate keyboard channel                         */
/*                                                                           */
/* INPUTS:      key = pointer to keyboard extension                          */
/*              chanp = channel number                                       */
/*              channame = channel name                                      */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENXIO = invalid device number                                */
/*              EBUSY  = all channels allocated                              */
/*              ECHRNG = invalid channel number                              */
/*              EINVAL = user requested specific channel                     */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int  keympx(struct kbdext *key, int *channum, char *channame)
{

  int rc = 0;
  int i;

  if (channame)   {                    /* if allocating channel ...          */

    if (*channame != '\0') {           /*   user can not specify channel num */
      rc = EINVAL;
    }
    else {
      if (key->diag) {                 /*   error if in diagnostic mode      */
         rc = EBUSY;
      }
      else {
                                       /*   look for free channel            */
        for(i = 0;i<KBD_NUM_CH; i++) {
          if(!key->ccb[i].inuse) break;
        }

        if (i == KBD_NUM_CH)  {        /*   error, no free channels          */
          rc = EBUSY;
        }
        else {
                                       /*   channel is not open              */
          key->ccb[i].oseq = NOTOPEN;
                                       /*   channel is allocated             */
          key->ccb[i].inuse = TRUE;
          *channum = i;                /*   return channel number            */
        }
      }
    }
  }
  else {                               /* else de-allocating channel ...     */

                                       /*   invalid channel number if too    */
                                       /*   big, too small, channel not      */
                                       /*   allocated, or channel open       */
    if ((*channum >=  KBD_NUM_CH) ||
        (*channum < 0) ||
        (!key->ccb[*channum].inuse) ||
        (key->ccb[*channum].oseq != NOTOPEN)) {
           rc = ECHRNG;
    }
    else {                             /*   free channel                     */
            key->ccb[*channum].inuse = FALSE;
    }
  }

  return(rc);
}



/*****************************************************************************/
/*                                                                           */
/* NAME:        keyopen                                                      */
/*                                                                           */
/* FUNCTION:    process keyboard open request                                */
/*                                                                           */
/* INPUTS:      com = pointer to common                                      */
/*              key = pointer to keyboard extension                          */
/*              flag = open flags                                            */
/*              chan = channel number from open                              */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              ENODEV = invalid keyboard                                    */
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

int keyopen(struct common *com, struct kbdext *key, uint flag, chan_t chan)
{
   int rc;

                                       /* invalid channel number if too      */
                                       /* big, too small, or not allocated   */
   if ((chan >=  KBD_NUM_CH) || (chan < 0) || (!key->ccb[chan].inuse)) {
     rc = ECHRNG;
   }
                                       /* error if channel already open      */
   else {                              /* or kbd in diagnostic mode          */
     if ((key->ccb[chan].oseq != NOTOPEN) || key->diag) {              
       rc = EBUSY;
     }
     else {
       if (key->act_ch == NO_ACT_CH) { /* if no other channel open then      */

                                       /* pin code and register intr handler */
         if (!(rc = reg_intr(FALSE))) {

                                       /* reset to starting (dds) values     */
            key->volume = key->dds_volume;
            key->click = key->dds_click;
            key->typa_rate_delay = key->dds_typa_rate_delay;

            rc = keysetup(com, key);   /* initialize keyboard                */

            if (rc)  ureg_intr();      /* if failure, close down adapter and */
                                       /* driver                             */
         }
       }
       else {                          /* else another channel is open so    */
          stop_sound();                /* turn off alarm and clr sound queue */
                                       /* clear toggle key status bits       */
           rc = key_stat(com, key, (ushort)(KATAKANA | CAPS_LOCK |
                    NUM_LOCK | SCR_LOCK), (ushort) 0);
       }

       if (!rc) {                     /* if all ok                           */
                                      /* update open seq number              */
         key->ccb[chan].oseq = key->act_ch + 1;
                                       /* no input ring                      */
         key->ccb[chan].rcb.ring_ptr = NULL;
                                       /* keep alive poll is disabled        */
         key->ccb[chan].kpseq = NULL;
                                       /* save pid of channel owner          */
         if (flag & DKERNEL)
           key->ccb[chan].owner_pid = KERNEL_PID;
         else
           key->ccb[chan].owner_pid = getpid();
                                       /* this channel is the active channel */
         key->act_ch = chan;
       }
     }
   }

   return(rc);

}


/*****************************************************************************/
/*                                                                           */
/* NAME:        keyclose                                                     */
/*                                                                           */
/* FUNCTION:    Process keyboard close request                               */
/*                                                                           */
/* INPUTS:      com = pointer to common                                      */
/*              key = pointer to keyboard extension                          */
/*              chan = channel number                                        */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENXIO = invalid device number                                */
/*              EPERM  = error while defining interrupt handler to system    */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int keyclose(struct common *com, struct kbdext *key, chan_t chan)
{

  int   rc = 0;
  int   j,n;
  uchar cptr;

                                       /* big, too small, or not allocated   */
  if ((chan >=  KBD_NUM_CH) || (chan < 0) || (!key->ccb[chan].inuse)) {
    rc = ECHRNG;
  }
  else {                               /* continue only if channel open      */
    if (key->ccb[chan].oseq != NOTOPEN) {

                                       /* disable SAK if kernel chan closing */
      if (key->ccb[chan].owner_pid == KERNEL_PID) {
        key->sak_enabled = FALSE;
        key->sak_callback = NULL;
      }
      else  {                          /* else user channel so               */

        free_kap(&key->ccb[chan]);     /*  free keep alive poll resources    */
                                       /*  unregister input ring             */
        ktsm_uring(&key->ccb[chan].rcb);
      }


      if (chan == key->act_ch) {       /* if active channel is closing       */

        exit_diag(com,key);            /* exit diag mode as required         */

                                       /* new active channel is previously   */
                                       /* opened channel                     */
        key->act_ch = key->ccb[chan].oseq - 1;

                                       /* if no other channel open           */
        if (key->act_ch == NO_ACT_CH) {
                                       /* disable keyboard scanning          */
           put_oq1(com, DEFAULT_DISABLE_CMD);
           ureg_intr();                /* close down adapter and driver      */
        }
        else {                         /* if another channel is open then    */
           stop_sound();               /* turn off alarm and clr sound queue */
                                       /* clear toggle key status bits       */
           key_stat(com, key, (ushort)(KATAKANA | CAPS_LOCK |
               NUM_LOCK | SCR_LOCK), (ushort) 0);
                                       /* reset any partially keyed kill seq */
           if (key->ccb[key->act_ch].kpindex > 0) {
             key->ccb[key->act_ch].kpindex = 0;
           }
        }
      }
      else {                           /* else active chan is not closing    */
                                       /* so update open sequence numbers    */
                                       /* removing channel that is closing   */
                                       /* channel is no longer open          */
        j = key->act_ch;
        for(;;) {
          n = key->ccb[j].oseq - 1;
          if (n == chan) {
            key->ccb[j].oseq = key->ccb[chan].oseq;
            break;
          }
          j = n;
        }
      }

      key->ccb[chan].oseq = NOTOPEN;
    }
  }

  return(rc);

}


/*****************************************************************************/
/*                                                                           */
/* NAME:        keyioctl                                                     */
/*                                                                           */
/* FUNCTION:    Process ioctl request for keyboard device                    */
/*                                                                           */
/* INPUTS:      com = pointer to common                                      */
/*              key = pointer to keyboard extension                          */
/*              cmd = function requested via ioctl                           */
/*              arg = pointer to argument passed via ioctl                   */
/*              flag = open flags                                            */
/*              chan = channel number                                        */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENXIO = invalid device number                                */
/*              EIO    = I/O error                                           */
/*              EINVAL = invalid cmd or argument                             */
/*              EPERM  = could not allocate timer                            */
/*              ENOMEM = insufficient memory                                 */
/*              ENOSPC = insufficient file system or paging space            */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/


int  keyioctl(struct common *com, struct kbdext *key, int cmd,
              caddr_t arg, uint flag, chan_t chan)
{

  struct ksalarm ksalarm;
  struct devinfo info;
  uint   data;
  uchar  cdata;
  uchar  *cptr;
  int    rc = 0;


                                       /* big, too small, or not allocated   */
  if ((chan >=  KBD_NUM_CH) || (chan < 0) || (!key->ccb[chan].inuse)) {
    rc = ECHRNG;
  }
  else {                               /* error if channel is not open       */
    if (key->ccb[chan].oseq == NOTOPEN) {
      rc = EACCES;
    }
    else {
      switch(cmd) {                    /* process command                    */

                                       /*------------------------------------*/
                                       /* Return devinfo structure           */
                                       /*------------------------------------*/

        case IOCINFO:                  /* error if NULL argument passed      */
          if (arg == (caddr_t) NULL) {
            rc = EINVAL;
          }
          else {                       /* zero devinfo structure             */
            bzero(&info, sizeof(info));
                                       /* set type field                     */
            info.devtype = INPUTDEV;
            COPYOUT(info)              /* copy to caller's memory            */
          }
          break;
                                       /*------------------------------------*/
                                       /* query keyboard device identifier   */
                                       /*------------------------------------*/

        case KSQUERYID:                /* error if NULL argument passed      */
          if (arg == (caddr_t) NULL) {
            rc = EINVAL;
          }
          else {                       /* copy ID to caller's memory         */
            COPYOUT(key->kbd_type)
          }
          break;
                                       /*------------------------------------*/
                                       /* query keyboard service vector      */
                                       /*------------------------------------*/

        case KSQUERYSV:                /* error if NULL argument passed      */
          if (arg == (caddr_t) NULL) {
            rc = EINVAL;
          }
          else {                       /* kernel extension owns channel ...  */
            if (flag & DKERNEL) {
                                       /* pass address of vector to user     */
              *((void **) arg) = service_vector;
            }
            else  {                    /* cmd not supported if user owns chan*/
              rc = EINVAL;
            }
          }
          break;

                                       /*------------------------------------*/
                                       /* register input ring                */
                                       /*------------------------------------*/

        case KSREGRING:                /* error if NULL argument passed      */
          if (arg == (caddr_t) NULL)  {
            rc = EINVAL;
          }
          else {                       /* register input ring in kernel space*/
            if (flag & DKERNEL) {
              rc = kring(key, &key->ccb[chan].rcb, (struct kregring *) arg);
            }
            else {                     /* register input ring in user space  */
              rc = ktsm_rring(&key->ccb[chan].rcb, (char *) arg);
            }
          }

          break;
                                       /*------------------------------------*/
                                       /* flush input ring                   */
                                       /*------------------------------------*/

        case KSRFLUSH:
          ktsm_rflush(com,&key->ccb[chan].rcb);
          break;
                                       /*------------------------------------*/
                                       /* set/reset keyboard LEDs            */
                                       /*------------------------------------*/

        case KSLED:                    /* process only if for active channel */
          if (chan == key->act_ch) {
                                       /* error if NULL argument passed      */
            if (arg == (caddr_t) NULL) {
              rc = EINVAL;
            }
            else {            
              if (key->diag) {         /* busy if in diag mode               */
                rc = EBUSY;
              }
              else {
                COPYIN(data)           /* copy args to local memory          */
                if (!rc) {             /* if copy successful then            */

                  KTSMDTRACE1(keyioctl, leds, data);

                                       /* make sure only valid bits specified*/
                  if (data & (~CHGLEDMASK)) {
                    rc = EINVAL;
                  }
                  else {               /* update leds                        */
                    rc = key_stat(com, key, (ushort)(CAPS_LOCK |
                       NUM_LOCK | SCR_LOCK), (ushort) (data << 9) );
                  }
                }
              }
            }
          }
          break;

                                       /*------------------------------------*/
                                       /* configure clicker                  */
                                       /*------------------------------------*/

        case KSCFGCLICK:               /* process only if for active channel */
          if (chan == key->act_ch) {
                                       /* error if NULL argument passed      */
            if (arg == (caddr_t) NULL) {
              rc = EINVAL;
            }
            else {
              if (key->diag) {         /* busy if in diag mode               */
                rc = EBUSY;
              }
              else {
                COPYIN(data)           /* copy arg to local memory           */
                if (!rc) {             /* if copy successful then            */

                  KTSMDTRACE1(keyioctl, clicker, data);

                                       /* validate argument                  */
                  if ((data != KSCLICKOFF) &&
                      (data != KSCLICKLOW) &&
                      (data != KSCLICKMED) &&
                      (data != KSCLICKHI)) {
                    rc = EINVAL;       /* argument is invalid                */
                  }
                  else {               /* arg is ok                          */
                                       /* process volume change              */
                      rc = chg_clicker(data);
                  }
                }
              }
            }
          }
          break;

                                       /*------------------------------------*/
                                       /* set alarm volume                   */
                                       /*------------------------------------*/

        case KSVOLUME:                 /* process only if for active channel */
          if (chan == key->act_ch) {
                                       /* error if NULL argument passed      */
            if (arg == (caddr_t) NULL) {
              rc = EINVAL;
            }
            else {
              if (key->diag) {         /* busy if in diag mode               */
                rc = EBUSY;
              }
              else {
                COPYIN(data)           /* copy arg to local memory           */
                if (!rc) {             /* if copy successful then            */

                  KTSMDTRACE1(keyioctl, volume, data);

                                       /* validate argument                  */
                  if ((data != KSAVOLOFF) &&
                      (data != KSAVOLLOW) &&
                      (data != KSAVOLMED) &&
                      (data != KSAVOLHI)) {
                     rc = EINVAL;      /* argument is invalid                */
                  }
                  else {               /* arg is ok                          */
                    key->volume = (uchar) data;
                  }
                }
              }
            }
          }
          break;

                                       /*------------------------------------*/
                                       /* sound alarm                        */
                                       /*------------------------------------*/

        case KSALARM:                  /* process only if for active channel */
          if (chan == key->act_ch) {
                                       /* error if NULL argument passed      */
            if (arg == (caddr_t) NULL) {
              rc = EINVAL;
            }
            else {
              if (key->diag) {         /* busy if in diag mode               */
                rc = EBUSY;
              }
              else {
                COPYIN(ksalarm)        /* copy arg to local memory           */
                if (!rc) {             /* if copy successful then            */
                                       /* process request                    */
                  rc = put_sq(&ksalarm);
                }
              }
            }
          }
          break;

                                       /*------------------------------------*/
                                       /* set keyboard typamatic rate        */
                                       /*------------------------------------*/

        case KSTRATE:                  /* process only if for active channel */
          if (chan == key->act_ch) {
                                       /* error if NULL argument passed      */
            if (arg == (caddr_t) NULL) {
              rc = EINVAL;
            }
            else {
              if (key->diag) {         /* busy if in diag mode               */
                rc = EBUSY;
              }
              else {
                COPYIN(data)           /* copy arg to local memory           */
                if (!rc) {             /* if copy successful then            */

                  KTSMDTRACE1(keyioctl, trate, data);

                                       /* error if arg out of range          */
                  if ((data < MIN_TYPA_RATE) ||
                     (data > MAX_TYPA_RATE)) {
                    rc = EINVAL;
                  }
                  else {               /* generate new rate/delay            */
                    if (key->kbd_type == KS106) {
                       cdata = (key->typa_rate_delay & DELAY_PARM) |
                              t_rate_106[data];
                    }
                    else {
                       cdata = (key->typa_rate_delay & DELAY_PARM) |
                              t_rate_table[data];
                    }
                                       /* send to keyboard and wait on I/O   */
                                       /* if rate has been changed           */
                    if (cdata != key->typa_rate_delay) {
                      key->typa_rate_delay = cdata;
                      put_oq2(com, (OFRAME) SET_RATE_DELAY_CMD,
                         (OFRAME) ((cdata<<8) | WRITE_KBD_CMD));
                      rc = wait_oq(com);
                    }
                  }
                }
              }
            }
          }
          break;

                                       /*------------------------------------*/
                                       /* set keyboard typamatic delay       */
                                       /*------------------------------------*/

        case KSTDELAY:                 /* process only if for active channel */
          if (chan == key->act_ch) {
                                       /* error if NULL argument passed      */
            if (arg == (caddr_t) NULL) {
              rc = EINVAL;
            }
            else {
              if (key->diag) {         /* busy if in diag mode               */
                rc = EBUSY;
              }
              else {
                COPYIN(data)           /* copy arg to local memory           */
                if (!rc) {             /* if copy successful then            */

                  KTSMDTRACE1(keyioctl, tdelay, data);

                  switch(data) {       /* generate new rate/delay            */
                    case KSTDLY250:
                      cdata = (key->typa_rate_delay & RATE_PARM) |
                               DELAY_250MS;
                      break;

                    case KSTDLY500:
                      cdata = (key->typa_rate_delay & RATE_PARM) |
                               DELAY_500MS;
                      break;

                    case KSTDLY750:
                      cdata = (key->typa_rate_delay & RATE_PARM) |
                               DELAY_750MS;
                      break;

                    case KSTDLY1000:
                      cdata = (key->typa_rate_delay & RATE_PARM) |
                               DELAY_1000MS;
                      break;

                    default:
                      rc = EINVAL;
                  }
                                       /*  send to keyboard and wait on I/O  */
                                       /*  if arg ok and delay changed       */
                  if ((!rc) && (cdata != key->typa_rate_delay)) {
                    key->typa_rate_delay = cdata;
                    put_oq2(com, (OFRAME) SET_RATE_DELAY_CMD,
                      (OFRAME) ((cdata<<8) | WRITE_KBD_CMD));
                    rc = wait_oq(com);
                  }
                }
              }
            }
          }
          break;

                                       /*------------------------------------*/
                                       /* enable/disable keep alive poll     */
                                       /*------------------------------------*/

        case KSKAP:
          if  (flag & DKERNEL) {       /* error if call from kernel extension*/
            rc = EINVAL;
          }
          else {
            free_kap(&key->ccb[chan]); /* free keep alive poll resources     */
                                       /* if argument is not NULL then       */
            if (arg != (caddr_t) NULL) {
                                       /* get new sequence size              */
              if (!(rc = copyin(arg, (char *) &cdata,
                  sizeof(uchar)))) {

                if (cdata == 0) {      /* size of zero is invalid            */
                  rc = EINVAL;
                }
                else {                 /* allocate memory for sequence       */
                  data = 1 +  2*(uint)cdata;
                  cptr =  (uchar *)
                      xmalloc(data, 1, pinned_heap);
                  if (!cptr)  rc = ENOMEM;
                  else {               /* copy sequence to local memory      */
                    if (rc = copyin(arg, (char *) cptr, data)) {
                      xmfree((caddr_t) cptr, pinned_heap);
                    }
                    else {             /* allocate keep alive poll timer     */
                      if ((key->ccb[chan].kptimer = talloc()) == NULL ) {
                        xmfree((caddr_t) cptr, pinned_heap);
                        rc = EPERM;
                      }
                      else {           /* save sequence and enable function  */
                        key->ccb[chan].owner_pid = getpid();
                        key->ccb[chan].kpindex = 0;
                        key->ccb[chan].kpseq = cptr;
                      }
                    }
                  }
                }
              }
            }
          }

          KTSMDTRACE(keyioctl, poll, key->ccb[chan].kpseq,
               *((uchar*)(cptr)), *((uchar*)(cptr+1)),
               *((uchar*)(cptr+2)), *((uchar*)(cptr+3)));

          break;

                                       /*------------------------------------*/
                                       /* keep alive poll acknowledge        */
                                       /*------------------------------------*/

        case KSKAPACK:                 /* error if call from kernel extension*/
          if  (flag & DKERNEL) {
            rc = EINVAL;
          }
          else {                       /* if keep alive poll sequence defined*/
            if (key->ccb[chan].kpseq) {
                                       /* if poll active                     */
              if (key->ccb[chan].kpindex < 0) {
                                       /* stop timer                         */
                tstop(key->ccb[chan].kptimer);
                                       /* reset index                        */
                key->ccb[chan].kpindex = 0;
              }
            }
          }
          break;

                                       /*------------------------------------*/
                                       /* enter/exit diagnostic mode         */
                                       /*------------------------------------*/

        case KSDIAGMODE:               /* error if called from kernel ext    */
          if (flag & DKERNEL) {
            rc = EINVAL;
          }
          else {                       /* error if not active channel        */
            if (chan != key->act_ch) {
              rc = EBUSY;    
            }
            else {                     /* error if NULL argument             */
              if (arg == (caddr_t) NULL) {
                rc = EINVAL;
              }
              else {           
                COPYIN(data)           /* copy arg to local memory           */
                if (!rc) {             /* if copy successful then            */

                  KTSMDTRACE(keyioctl, diag, data, 0, 0, 0, 0);

                                       /* enter diagnostics mode             */
                  if (data == KSDENABLE) {
                     rc = enter_diag();
                  }
                  else {               /* exit diagnostic mdoe               */
                    if (data == KSDDISABLE) {
                      rc = exit_diag(com,key);
                    }
                    else {              /* invalid request                   */
                      rc = EINVAL;
                    }
                  }
                }
              }
            }
          }
          break;

                                       /*------------------------------------*/
                                       /* invalid command                    */
                                       /*------------------------------------*/
        default:
          rc = EINVAL;

      }
    }
  }

  return(rc);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        keysetup                                                     */
/*                                                                           */
/* FUNCTION:    clear keyboard status and initialize keyboard device         */
/*                                                                           */
/* INPUTS:      com = pointer to common                                      */
/*              key = pointer to keyboard extension                          */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              ENDEV = invalid device number                                */
/*              EIO   = I/O error                                            */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int keysetup(struct common *com, struct kbdext *key)

{
   int rc;

   key->kbdstat = 0;                   /* reset status                       */
   key->led_state = 0;
                                       /* clear keyboard state table         */
                                       /* (all keys released state)          */
   bzero (key->kbd_state_table, sizeof(key->kbd_state_table));
   key->break_code_rcv = FALSE;
                                       /* switch on keyboard type            */
   switch (key->kbd_type) {

      case KS101   :                   /* setup 101/102 keyboards            */
      case KS102:
                                       /*   set typamatic rate/delay parm    */
         kbdinit_101[TYPAPARM] =  (OFRAME)
              ((key->typa_rate_delay << 8) | WRITE_KBD_CMD);
                                       /*   queue up kbd init cmds           */
         put_oq(com, kbdinit_101);
         rc = wait_oq(com);            /*   wait till I/O complete           */

         break;

      case KSPS2:                      /* setup PS/2 keyboards               */

         if (key->special_106) {       /* if JP, KR, TW(Chinese), then       */
                                       /* special handling is needed.        */

                                       /*   set typamatic rate/delay parm    */
            kbdinit_106ps[TYPAPARM] = (OFRAME)
                 ((key->typa_rate_delay << 8) | WRITE_KBD_CMD);

                                       /* if space saver kbd, set ScrollLock */
                                       /* key to Make/Break                  */
            if (key->nonum) {
              kbdinit_106ps[NUMLOCKPARM] = SET_SCRLLOCK_SCAN_CODE;
            }
                                       /*   queue up kbd init cmds           */
            put_oq(com, kbdinit_106ps);
            rc = wait_oq(com);         /*   wait till I/O complete           */

         }
         else {                        /* other standard countries           */
                                       /*   set typamatic rate/delay parm    */
            kbdinit_101[TYPAPARM] =  (OFRAME)
                 ((key->typa_rate_delay << 8) | WRITE_KBD_CMD);

                                       /* if space saver kbd, set ScrollLock */
                                       /* key to Make/Break                  */
            if (key->nonum) {
              kbdinit_101[NUMLOCKPARM] = SET_SCRLLOCK_SCAN_CODE;
            }
                                       /*   queue up kbd init cmds           */
            put_oq(com, kbdinit_101);
            rc = wait_oq(com);         /*   wait till I/O complete           */
         }
         break;

      case KS106:                      /* setup 106 keyboards                */
                                       /*   set typamatic rate/delay parm    */
         kbdinit_106[TYPAPARM] =  (OFRAME)
               ((key->typa_rate_delay << 8) | WRITE_KBD_CMD);
                                       /*   remove key 1 from list of cmds   */
                                       /*   that make keys M/B only if this  */
                                       /*   is a Korean keyboard. Key 1 is   */
                                       /*   removed by overwriting it (yuck) */
         if (key->special_106 == KOREAN_MAP) {
            kbdinit_106[KOREANPARM] = SET_106K_131_SCAN_CODE;
         }
                                       /*   queue up kbd init cmds           */
         put_oq(com, kbdinit_106);
         rc = wait_oq(com);            /*   wait till I/O complete           */

         break;

      default:                         /*    invalid keyboard                */
            rc = ENODEV;
   }

   if (!rc) {                          /* set clicker volume                */
      rc = chg_clicker(key->click);
   }

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        free_kap                                                     */
/*                                                                           */
/* FUNCTION:    free keep alive poll resources                               */
/*                                                                           */
/* INPUTS:      ccb = pointer to channel control block                       */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void free_kap(struct ccb *ccb)
{

   caddr_t cptr;

                                       /*  if keep alive poll seq.  defined  */
   if (ccb->kpseq) {
                                       /*   disable keep alive poll          */
      cptr = ccb->kpseq;
      ccb->kpseq = (uchar *) NULL;
                                       /*   stop and free keep alive timer   */
      tstop(ccb->kptimer);
      tfree(ccb->kptimer);
                                       /*   free keep alive poll sequence buff*/
      xmfree((caddr_t) cptr, pinned_heap);
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        kring                                                        */
/*                                                                           */
/* FUNCTION:    register a input ring that is in kernel address space        */
/*                                                                           */
/* INPUTS:      key = pointer to keyboard extension                          */
/*              rcb = pointer to rcb for channel                             */
/*              arg = pointer to kregring structure                          */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EINVAL = invalid argument                                    */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int kring(struct kbdext *key, struct rcb *rcb, struct kregring *arg)
{
   caddr_t start, end;
   int rc = 0;

                                       /* unregister input ring if arg       */
                                       /* contains NULL ring pointer         */
   if (arg->ring == (caddr_t) NULL) {
      rcb->ring_ptr = (caddr_t) NULL;
      key->sak_callback = NULL;
   }
   else {                              /* if new ring, verify that parms are */
                                       /* ok (for now anyway)                */
      start = arg->ring + sizeof(struct inputring);
      end = start + ((struct inputring *)(arg->ring))->ir_size;
      if ((((struct inputring *)(arg->ring))->ir_size ==  0 )  ||
          (((struct inputring *)(arg->ring))->ir_tail >= end ) ||
          (((struct inputring *)(arg->ring))->ir_head >= end ) ||
          (((struct inputring *)(arg->ring))->ir_tail < start) ||
          (((struct inputring *)(arg->ring))->ir_head < start)) {
              rc = EINVAL;
      }
      else {                           /* if ok register ring by moving data */
                                       /* to keyboard extension              */
                                       /* note: ring_ptr set to NULL first so*/
                                       /* changes can be made with worrying  */
                                       /* about interrupts; likewise, the    */
                                       /* ptr is set to new value at the end */
         rcb->ring_ptr = (caddr_t) NULL;
         rcb->owner_pid = KERNEL_PID;
         rcb->rpt_id = arg->report_id;
         key->notify_callback = (void *) arg->notify_callback;
         key->sak_callback = (void *) arg->sak_callback;
         rcb->ring_ptr = arg->ring;
      }
   }
   return(rc);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        get_kbd_id                                                   */
/*                                                                           */
/* FUNCTION:    Reset keyboard and get ID                                    */
/*                                                                           */
/* INPUTS:      com = pointer to common                                      */
/*              key = pointer to keyboard extension                          */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*                  EIO                                                      */
/*                  EPERM                                                    */
/*                  ENOMEM                                                   */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int get_kbd_id(struct common *com, struct kbdext *key)
{

  int rc, retry, cnt;
  IFRAME buff[3];


  rc = EIO;
  retry = R_RETRY;
  key->kbd_type = 0;

  while(retry--) {
                                       /* send RESET cmd to keyboard         */
    put_oq1(com, (OFRAME) KBD_RESET_CMD);
                                       /* get BAT completion code            */
    if ((cnt = get_iq(com,3, RESETTO, buff)) <= 0) {
      if (!cnt) {                      /* if BAT not received                */
        if (retry) continue;           /* try again if retries not exhausted */
                                       /* else log error                     */
        io_error("get_kbd_id", FALSE, KIO_ERROR, 0, "%04x",
           com->in_progress);
      }
    }
    else {
      if (buff[0] != KBD_BAT_CC) {     /* log error if invalid BAT           */
        io_error("get_kbd_id", FALSE, KBDBAT_ERROR, 1, "%02x", buff[0]);
      }
      else {                           /* if unsoliciated ID frames received */
                                       /* then must be Brothers keyboard     */
        if ((cnt == 3) && (buff[1] == KBD_POR_ID_1C)) {
                                       /* ID for 106 key keyboard received   */
          if (buff[2] == KBD_POR_ID_2C) {
            key->kbd_type = KS106;
            rc = 0;     
          }
          else {                       /* invalid kbd ID received , log error*/
            io_error("get_kbd_id", FALSE, KBDID_ERROR, 2, "%02x %02x",
                buff[1], buff[2]);
          }
        }
        else {                         /* else this is not a Brothers kbd    */
                                       /* so send LAYOUT ID cmd              */
          put_oq1(com, (OFRAME) LAYOUT_ID_CMD);
                                       /* get ID frames                      */
          switch (cnt = get_iq(com, 2, RCVTO, buff)) {
            case 2:
                                       /* ID for 101 key keyboard received   */
              if ((buff[0] == KBD_POR_ID_2A) &&
                  (buff[1] == KBD_POR_ID_1C)) {
                key->kbd_type = KS101;
                rc = 0;
              }
              else {                   /* ID for 102 key keyboard received   */
                if ((buff[0] == KBD_POR_ID_2B) &&
                    (buff[1] == KBD_POR_ID_1C)) {
                   key->kbd_type = KS102;
                   rc = 0;
                }
                else {                 /* invalid kbd ID received, log error */
                  io_error("get_kbd_id", FALSE,  KBDID_ERROR, 3,
                      "%02x %02x", buff[1], buff[0]);
                }
              }
              break;

            case 1:
                                       /* if RESEND was received then        */
              if (buff[0] == KBD_RESEND) {
                                       /* send Read ID command               */
                put_oq1(com, (OFRAME) READ_ID_CMD);

                if ((cnt = get_iq(com, 2, RCVTO, buff)) != 2) {
                                       /* if ID frames not received then     */
                  if (cnt >= 0) {
                                       /* try again if retries not exhausted */
                    if (retry) continue;
                                       /* else log error                     */
                    io_error("get_kbd_id", FALSE, KIO_ERROR, 4,
                         "%04x", com->in_progress);
                  }
                }
                else {
                  if (buff[0] == KBD_POR_ID_2D) {
                    if ((buff[1] == KBD_POR_ID_1) ||
                        (buff[1] == KBD_POR_ID_1B)) {
                                       /* set PS/2 keyboard type             */
                      key->kbd_type = KSPS2;
                      rc = 0;
                    }
                    else {
                                       /* invalid kbd ID received, log error */
                      io_error("get_kbd_id", FALSE, KBDID_ERROR, 5, "%02x %02x",
                           buff[1], buff[0]);
                    }
                  }
                  else {
                                       /* invalid kbd ID received, log error */
                    io_error("get_kbd_id", FALSE, KBDID_ERROR, 6, "%02x %02x",
                         buff[1], buff[0]);
                  }
                }
              }
              else {                   /* not RESEND                         */
                                       /* try again if retries not exhausted */
                if (retry) continue;
                                       /* else log error                     */
                io_error("get_kbd_id", FALSE, KIO_ERROR, 7, "%04x",
                     com->in_progress);
              }
              break;

            default:

              if (cnt >= 0) {          /* if ID frames not received then     */
                if (retry) continue;   /* try again if retries not exhausted */
                                       /* else log error                     */
                io_error("get_kbd_id", FALSE, KIO_ERROR, 8, "%04x",
                   com->in_progress);
              }
              break;
          }                            /* end of switch statement            */
        }
      }
    }
    break;
  }                                    /* end of while loop                  */
  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        exit_diag                                                    */
/*                                                                           */
/* FUNCTION:    exit diagnostics mode                                        */
/*              - reinstall interrupt handler                                */
/*              - reintialize keyboard                                       */
/*                                                                           */
/* INPUTS:      com = pointer to common                                      */
/*              key = pointer to keyboard extension                          */
/*                                                                           */
/* OUTPUTS:     0      = success                                             */
/*              EIO    = I/O error                                           */
/*              EPERM  = error while defining interrupt handler to system    */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int exit_diag(struct common *com, struct kbdext *key)
{
   int rc;

   if (!key->diag) {                   /* if not in diagnostics mode then    */
     rc = 0;                           /*   just return ok                   */
   }
   else {                              /* diagnostics mode active so ...     */
     if (rc=reg_intr(TRUE)) {          /* register interrupt handler         */
        com->perr = TRUE;              /* failed, we're hosed                */
     }
     else {
        rc = keysetup(com,key);        /* intialize keyboard                 */
     }
     key->diag = FALSE;                /* clear diagnostic mode flag         */
   }
   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:             initkbd                                                 */
/*                                                                           */
/* FUNCTION:         This module initializes the keyboard                    */
/*                                                                           */
/* INPUTS:           com = pointer to common                                 */
/*                   key = pointer to keyboard extension                     */
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

int initkbd(struct common *com, struct kbdext *key,
            struct ktsmdds *dds_ptr, dev_t devno)
{
  int  rc = 0;

  if (!com->adpt_devno) {              /* bad news if adapter has not been   */
    rc = EINVAL;                       /* configured                         */
  }
  else {
   if (!key->kbd_devno) {               /* skip if kbd already configured     */
                                  
                                       /* if run time config then            */
    if (dds_ptr->ipl_phase == RUNTIME_CFG) {

      if (!(rc=reg_intr(FALSE))) {     /*   register interrupt handler       */
                                       /*   reset keyboard and get id        */
        rc = get_kbd_id(com, key);
        ureg_intr();                   /*   unregister interrupt handler     */
        if (!rc) {                     /*   if physical kbd is not same as   */
                                       /*   defined device then              */
          if (dds_ptr->device_type != key->kbd_type) {
            rc = ENODEV;               /*     set error code                 */
          }
        }
      }
    }
    else {                             /* else ipl config so just            */
                                       /*   set device type from dds         */
      key->kbd_type = dds_ptr->device_type;
    }

    if (!rc) {                         /* if all ok                          */
                                       /* set alarm volume                   */
       if ((dds_ptr->volume != KSAVOLOFF) &&
          (dds_ptr->volume != KSAVOLLOW) &&
          (dds_ptr->volume != KSAVOLMED) &&
          (dds_ptr->volume != KSAVOLHI)) {
         key->dds_volume = KSAVOLMED;
       }
       else {
         key->dds_volume = dds_ptr->volume;
       }
                                       /* set clicker volume                 */
       if ((dds_ptr->click != KSCLICKOFF) &&
          (dds_ptr->click != KSCLICKLOW) &&
          (dds_ptr->click != KSCLICKMED) &&
          (dds_ptr->click != KSCLICKHI)) {
         key->dds_click = KSCLICKMED;
       }
       else {
         key->dds_click = dds_ptr->click;
       }
                                       /* set typamatic rate                 */
       if ((dds_ptr->typamatic_rate < MIN_TYPA_RATE) ||
              (dds_ptr->typamatic_rate  > MAX_TYPA_RATE)) {
         dds_ptr->typamatic_rate = DEF_TYPA_RATE;
       }
                                       /* set typamatic delay                */
       if ((dds_ptr->typamatic_delay < 250) ||
               (dds_ptr->typamatic_delay > 1000)) {
         dds_ptr->typamatic_delay = DEF_DELAY;
       }
                                       /* init rate/delay variable           */
       if (key->kbd_type == KS106) {
          key->dds_typa_rate_delay =
                  t_rate_106[dds_ptr->typamatic_rate] |
                  ((((dds_ptr->typamatic_delay / 250) - 1) << 5) & DELAY_PARM);
       }
       else {
          key->dds_typa_rate_delay =
                  t_rate_table[dds_ptr->typamatic_rate] |
                  ((((dds_ptr->typamatic_delay / 250) - 1) << 5) & DELAY_PARM);
       }

                                       /* initalize shift key table          */
       key->shift_keys[30] = TRUE;     /*    caps_lock                       */
       key->shift_keys[44] = TRUE;     /*    left shift                      */
       key->shift_keys[57] = TRUE;     /*    right shift                     */
       key->shift_keys[58] = TRUE;     /*    control key                     */
       key->shift_keys[60] = TRUE;     /*    left alt                        */
       key->shift_keys[62] = TRUE;     /*    right alt                       */

                                       /* set special map flag               */
       key->special_106 = FALSE;
       if (dds_ptr->map) {
         key->special_106 = dds_ptr->map;
       }
                                       /* set space saver keyboard type      */
       key->nonum = FALSE;
       if ((dds_ptr->type == NO_NUM_PAD) &&
           (key->kbd_type == KSPS2)) {
         key->nonum = TRUE;
       }

                                       /* mark kbd configured                */
       key->kbd_devno = devno;         /* by saving device number            */
    }
   }
  }

  return(rc);
}


/*****************************************************************************/
/*                                                                           */
/*        table to translate typamatic rate to kbd command parm              */
/*                                                                           */
/*****************************************************************************/

char    t_rate_table[] =
        {
           0x1F,             /* 11111  invalid rate - only for robustness    */
           0x1F,             /* 11111  invalid rate - only for robustness    */
           0x1F,             /* 11111  for typamatic rate value  2 cps       */
           0x1A,             /* 11010  for typamatic rate value  3 cps       */
           0x17,             /* 10111  for typamatic rate value  4 cps       */
           0x14,             /* 10100  for typamatic rate value  5 cps       */
           0x12,             /* 10010  for typamatic rate value  6 cps       */
           0x11,             /* 10001  for typamatic rate value  7 cps       */
           0x0F,             /* 01111  for typamatic rate value  8 cps       */
           0x0D,             /* 01101  for typamatic rate value  9 cps       */
           0x0C,             /* 01100  for typamatic rate value 10 cps       */
           0x0B,             /* 01011  for typamatic rate value 11 cps       */
           0x0A,             /* 01010  for typamatic rate value 12 cps       */
           0x09,             /* 01001  for typamatic rate value 13 cps       */
           0x09,             /* 01001  for typamatic rate value 14 cps       */
           0x08,             /* 01000  for typamatic rate value 15 cps       */
           0x08,             /* 01000  for typamatic rate value 16 cps       */
           0x07,             /* 00111  for typamatic rate value 17 cps       */
           0x06,             /* 00110  for typamatic rate value 18 cps       */
           0x05,             /* 00101  for typamatic rate value 19 cps       */
           0x05,             /* 00101  for typamatic rate value 20 cps       */
           0x04,             /* 00100  for typamatic rate value 21 cps       */
           0x04,             /* 00100  for typamatic rate value 22 cps       */
           0x03,             /* 00011  for typamatic rate value 23 cps       */
           0x03,             /* 00011  for typamatic rate value 24 cps       */
           0x02,             /* 00010  for typamatic rate value 25 cps       */
           0x02,             /* 00010  for typamatic rate value 26 cps       */
           0x02,             /* 00010  for typamatic rate value 27 cps       */
           0x01,             /* 00001  for typamatic rate value 28 cps       */
           0x01,             /* 00001  for typamatic rate value 29 cps       */
           0x01              /* 00001  for typamatic rate value 30 cps       */
        };

char    t_rate_106[] =
        {
           0x1F,             /* 11111  invalid rate - only for robustness    */
           0x1F,             /* 11111  invalid rate - only for robustness    */
           0x1F,             /* 11111  for typamatic rate value  2 cps       */
           0x1D,             /* 11101  for typamatic rate value  3 cps       */
           0x1A,             /* 11010  for typamatic rate value  4 cps       */
           0x18,             /* 11000  for typamatic rate value  5 cps       */
           0x15,             /* 10101  for typamatic rate value  6 cps       */
           0x13,             /* 10011  for typamatic rate value  7 cps       */
           0x12,             /* 10010  for typamatic rate value  8 cps       */
           0x11,             /* 10001  for typamatic rate value  9 cps       */
           0x10,             /* 10000  for typamatic rate value 10 cps       */
           0x0E,             /* 01110  for typamatic rate value 11 cps       */
           0x0D,             /* 01101  for typamatic rate value 12 cps       */
           0x0C,             /* 01100  for typamatic rate value 13 cps       */
           0x0B,             /* 01011  for typamatic rate value 14 cps       */
           0x0B,             /* 01011  for typamatic rate value 15 cps       */
           0x0A,             /* 01010  for typamatic rate value 16 cps       */
           0x09,             /* 01001  for typamatic rate value 17 cps       */
           0x09,             /* 01001  for typamatic rate value 18 cps       */
           0x08,             /* 01000  for typamatic rate value 19 cps       */
           0x08,             /* 01000  for typamatic rate value 20 cps       */
           0x07,             /* 00111  for typamatic rate value 21 cps       */
           0x07,             /* 00111  for typamatic rate value 22 cps       */
           0x06,             /* 00110  for typamatic rate value 23 cps       */
           0x05,             /* 00101  for typamatic rate value 24 cps       */
           0x05,             /* 00101  for typamatic rate value 25 cps       */
           0x04,             /* 00100  for typamatic rate value 26 cps       */
           0x04,             /* 00100  for typamatic rate value 27 cps       */
           0x03,             /* 00011  for typamatic rate value 28 cps       */
           0x03,             /* 00011  for typamatic rate value 29 cps       */
           0x03              /* 00011  for typamatic rate value 30 cps       */
        };


