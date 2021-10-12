/* @(#)72	1.3  src/bos/kernext/inputdd/inc/tab.h, inputdd, bos41J, 9509A_all 2/14/95 13:20:12  */
/*
 * COMPONENT_NAME: (INPUTDD) Tablet DD - tab.h
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

#include "ktsm.h"
#include "common.h"
#include "tabext.h"


/*****************************************************************************/
/* local storage data structure                                              */
/*****************************************************************************/

struct local {
   struct   common com;             /* common data                        */
   struct   tabext tab;             /* tablet extension                   */
   char     flush_err;              /* flush tablet report                */
   uchar    tab_dframe;             /* TRUE if next frame in group is data*/
   ushort   tab_bytes_read;         /* number of tablet bytes read in     */
};


/*****************************************************************************/
/* proto types and references                                                */
/*****************************************************************************/

/* functions                                                                 */
extern void   send_q_frame(struct common *);
extern void   watch_dog(struct trb *);
extern int    read_port(char *, char *);
extern int    write_port(char *, char);
extern int    reg_intr(int); 
extern void   ureg_intr(void);
extern int    tabintr(struct intr *);
extern void   tab_proc(struct tablet_port *);
extern int    tabconfig(dev_t, long, struct uio *);
extern void   cleanup(dev_t);
extern int    initadpt(struct ktsmdds *, dev_t);
extern int    addswitch(dev_t);
extern int    qvpd(struct uio *);
extern int    tabopen(dev_t, uint, chan_t, caddr_t);
extern int    tabclose(dev_t, chan_t, caddr_t);
extern int    tabioctl(dev_t, int, caddr_t, uint, chan_t, caddr_t);
extern struct cdt * mem_dump(int);

/* data                                                                      */
extern struct  local *local;
extern lock_t  tab_lock;
extern OFRAME tab_default_cmds[];
extern struct cdt my_cdt;
