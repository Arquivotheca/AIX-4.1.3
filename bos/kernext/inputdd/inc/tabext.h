/* @(#)73	1.3  src/bos/kernext/inputdd/inc/tabext.h, inputdd, bos41J, 9509A_all 2/14/95 13:20:00  */
/*
 * COMPONENT_NAME: (INPUTDD) Tablet Extension - tabext.h
 *
 * FUNCTIONS: none
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

/*****************************************************************************/
/* local storage data structure                                              */
/*****************************************************************************/

struct tabext {
   struct   rcb rcb;                   /* ring control block:                */
   dev_t    tab_devno;                 /* devno of tablet                    */

   ushort   tab_resolution;            /* resolution in lines per inch       */
   ushort   tab_sample_rate;           /* data points per second             */
   ushort   tab_hor_active;            /* active region of tablet            */
   ushort   tab_ver_active;            /* active region of tablet            */
   ushort   tab_hor_deadzone;          /* tablet hor dead zone               */
   ushort   tab_ver_deadzone;          /* tablet ver dead zone               */

   short    tab_x_pos;                 /* tablet x position                  */
   short    tab_y_pos;                 /* tablet y position                  */

   uchar    tab_conversion;            /* tablet conversion                  */
   uchar    tabstat;                   /* tablet status                      */
   uchar    tablet_model;              /* tablet model (see inputdd.h)       */
   uchar    input_device;              /* input device (see inputdd.h)       */
   uchar    tab_block_data[6];         /* 6 bytes of data from tablet        */
   uchar    oflag;                     /* TRUE if tablet open                */
   uchar    tab_origin;                /* tablet origin                      */
};

/*****************************************************************************/
/* proto types and references                                                */
/*****************************************************************************/

/* functions                                                                 */
extern void  tablet_proc(struct tabext *);
extern int   tabletopen(struct common *, struct tabext *);
extern int   tabletioctl(struct common *, struct tabext *, int, caddr_t);
extern int   set_resolution(struct common *, struct tabext *, uint);
extern long  set_origin(struct common *, struct tabext *, uint);
extern long  set_sample_rate(struct common *com, struct tabext *, uint);
extern long  set_dead_zone  (struct tabext *,ulong);
extern void  set_active_area(struct tabext *);
extern int   get_tablet_id(struct common *, struct tabext *);
extern int   inittab(struct common *,struct tabext *,struct ktsmdds *,dev_t);

/* data                                                                      */
extern OFRAME tab_default_cmds[];

