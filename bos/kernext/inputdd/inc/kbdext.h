/* @(#)66	1.4  src/bos/kernext/inputdd/inc/kbdext.h, inputdd, bos411, 9434B411a 8/25/94 05:52:38  */
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard DD - kbdext.h
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/* keyboard extension                                                        */
/*****************************************************************************/

struct  kbdext  {

#define KBD_NUM_CH           2         /* number of channels                 */

   struct  ccb {                       /* per channel control block          */
      struct rcb rcb;                  /*   ring control block               */
      pid_t   owner_pid;               /*   process ID of channel owner      */
      int     kpindex;                 /*   keep alive poll sequence index   */
      uchar   *kpseq;                  /*   keep alive poll key sequence     */
      struct  trb  *kptimer;           /*   keep alive poll timer            */
      int     oseq;                    /*   ccb index of previous open + 1   */
#define    NOTOPEN           -1        /*   channel is not open              */
#define    OLDESTOPEN        0         /*   channel is oldest open           */
      char    inuse;                   /*   TRUE if channel allocated        */
   } ccb[KBD_NUM_CH];

   dev_t   kbd_devno;                  /* devno of keyboard                  */

   chan_t  act_ch;                     /* active channel number (-1 if no    */
                                       /* channel open)                      */
#define    NO_ACT_CH         -1        /*   no active channels               */

   void    *sak_callback;              /* SAK callback routine               */
   void    *notify_callback;           /* kernel notify callback routine     */

   int     kbd_type;                   /* keyboard type                      */
                                       /* defined in inputdd.h               */
/*         KS101             0x01            101 key keyboard                */
/*         KS102             0x02            102 key keyboard                */
/*         KS106             0x03            106 key keyboard                */
/*         KSPS2             0x04            ps2 keyboard                    */

   ushort  kbdstat;                    /* keyboard status                    */
                                       /* must be same as kbd_status[]       */
                                       /* in inputdd.h                       */
#define    SHIFT             0x8000    /*     1_______ ________              */
#define    CNTL              0x4000    /*     _1______ ________              */
#define    ALT               0x2000    /*     __1_____ ________              */
#define    KATAKANA          0x1000    /*     ___1____ ________              */
#define    CAPS_LOCK         0x0800    /*     ____1___ ________              */
#define    NUM_LOCK          0x0400    /*     _____1__ ________              */
#define    SCR_LOCK          0x0200    /*     ______1_ ________              */
#define    M_B               0x0100    /*     _______1 ________              */
#define    REPEAT            0x0080    /*     ________ 1_______              */
#define    L_SHIFT           0x0040    /*     ________ _1______              */
#define    R_SHIFT           0x0020    /*     ________ __1_____              */
#define    L_ALT             0x0010    /*     ________ ___1____              */
#define    R_ALT             0x0008    /*     ________ ____1___              */

   ushort  save_status0;               /* status at start of SAK seq         */
   ushort  save_status1;               /* status during SAK seq              */

   char    shift_keys[134];            /* TRUE if shift key                  */
   char    kbd_state_table[134];       /* key state table -                  */
#define    KEY_UP            0         /*    key up                          */
#define    KEY_DOWN          1         /*    key down                        */
#define    KEY_IGN           2         /*    key down but not                */
                                       /*    placed on ring                  */

   char    sak_enabled;                /* TRUE if SAK enabled                */
   char    in_sak;                     /* TRUE if SAK sequence started       */
   char    break_code_rcv;             /* TRUE if break code received        */

   uchar   special_106;                /* flag for korean,japanese,chinese kb*/
   uchar   nonum;                      /* flag for no 10-key pad             */

   uchar   volume;                     /* sound volume                       */
   uchar   dds_volume;                 /* default sound volume               */
   uchar   click;                      /* keyboard click control             */
   uchar   dds_click;                  /* default keyboard click control     */
   uchar   typa_rate_delay;            /* typamatic rate and delay           */
   uchar   dds_typa_rate_delay;        /* typamatic rate and delay           */
#define    DELAY_PARM        0x60      /* _11_____                           */
#define    RATE_PARM         0x1F      /* ___11111                           */
   uchar   led_state;                  /*   led state                        */
   uchar   alarm_active;               /* TRUE if alarm is on                */
   uchar   diag;                       /* TRUE if diagnostics mode           */


};

/*****************************************************************************/
/* proto types and references                                                */
/*****************************************************************************/

/* functions                                                                 */
extern void  keyproc(struct common *, struct kbdext *, uchar);
extern void  shift_status(struct common *, struct kbdext *, uchar);
extern void  proc_event(struct kbdext *, uchar, uchar);
extern void  proc_sak(struct kbdext *, uchar, uchar);
extern void  un_sak(struct kbdext *, uchar, uchar);
extern void  put_key(struct kbdext *, uchar, uchar, ushort);
extern void  poll_appl(struct kbdext *, uchar, uchar, ushort);
extern void  appl_killer(struct trb *);
extern int   key_stat(struct common *, struct kbdext *, ushort, ushort);
extern int   kbdleds(struct common *, struct kbdext *);
extern int   sv_proc(struct common *, struct kbdext *, int, caddr_t);

extern int   keympx(struct kbdext *, int *, char *);
extern int   keyopen(struct common *, struct kbdext *, uint, chan_t);
extern int   keyclose(struct common *, struct kbdext *, chan_t);
extern int   keyioctl(struct common *, struct kbdext *, int,
               caddr_t, uint, chan_t);
extern int   keysetup(struct common *, struct kbdext *);
extern void  free_kap(struct ccb *);
extern int   kring(struct kbdext *, struct rcb *, struct kregring *);
extern int   get_kbd_id(struct common *, struct kbdext *);
extern int   exit_diag(struct common *, struct kbdext *);
extern int   initkbd(struct common *, struct kbdext *,
             struct ktsmdds *, dev_t);

/* driver specific routines                                                  */
extern int   addswitch(dev_t);        

/* data                                                                      */
extern uchar  scan_to_posi[];
extern OFRAME kbdinit_101[];
extern OFRAME kbdinit_106[];
extern OFRAME kbdinit_106ps[];
extern char   t_rate_table[];
extern char   t_rate_106[];
