/* @(#)64	1.5  src/bos/kernext/inputdd/inc/sys/sgio.h, inputdd, bos41J, 9520A_all 5/15/95 16:44:08 */
/*
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SGIO
#define _H_SGIO

#include "sgiotrace.h"              /* must be first */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/errids.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/inputdd.h>            /* common inputdd structs/defines */
#include <sys/ioctl.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/pm.h>
#include <sys/pmdev.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <sys/sleep.h>
#include <sys/termio.h>
#include <sys/timer.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/xmem.h>
#include "sgio_dds.h"               /* sgio device dependent structs/defines */


/*****************************************************************************/
/* input ring control block structure (matches definition in ktsm.h)         */
/*****************************************************************************/
struct rcb {
   pid_t   owner_pid;                  /* process ID of ring's owner         */
#define KERNEL_PID   -1                /*   kernel process owns channel      */

   caddr_t ring_ptr;                   /* pointer to input ring              */
                                       /*   (NULL if no ring registered)     */
   int     ring_size;                  /* ring size                          */
   struct  xmem  xmdb;                 /* cross memory descriptor block      */
   uchar   rpt_id;                     /* report source identifier           */
};


/*****************************************************************************/
/* sgio device driver structures                                             */
/*****************************************************************************/
typedef struct sgio_device
{
  dev_t devno;
  sgio_dds_t device_dds;
  struct rcb rcb;
  uint device_open;
  struct file *fp;
  struct sgio_device *next;
  uint command;
  int status;
  uint pm_status;
  uint recent_io;
  uint device_present;
  struct  pm_handle pm;
  union
  {
    struct
    {
      uchar select;
      uchar last;
      char  granularity[8];
      short position[8];
    } dials;
    char lpfks[4];
  } device_data;
  struct trb *timer;
  struct trb *retry_timer;
  int retry_count;
  int sleep_event;
  lock_t device_lock;
} sgio_device_t;

typedef struct sgio
{
  tid_t sgiodd_tid;
  pid_t sikproc_pid;
  uint sikproc_term;
  uint list_changed;
  uint device_count;
  uint open_count;
  uint close_count;
  uint query_devices;
  uint pm_complete;
  uint pm_mode;
  struct trb *pm_timer;
  lock_t pm_lock;
  sgio_device_t *device;
  lock_t sgio_lock;
} sgio_t;

/*****************************************************************************/
/* sgio device driver defines                                                */
/*****************************************************************************/
#define SGIO_PRIORITY INTCLASS3

#define SKPROC_WAIT_INIT 0x80000000

#define RES_NAME_SGIO "SGIODD"

#define FALSE 0
#define TRUE 1
#define PM_TRUE 2

#define NONE 0
#define FAIL 1
#define SUCCESS 2
#define PENDING 3
#define REINIT 4
#define PM_REINIT 5
#define PM_RESET 6
#define PM_LOW_POWER 7
#define PM_LOW_POWER_COMPLETE 8

#define XMEMCPY_ERROR 1
#define SIKPROC_ERROR 2
#define PM_REG_ERROR 3
#define PM_HANDLER_ERROR 3

#define RESET     0x01
#define READ_CONF 0x06
#define ENABLE    0x08
#define DISABLE   0x09
#define READ_DIAL 0x0b
#define WRAP      0x0e
#define UNWRAP    0x0f
#define LPFK_NAK  0x80
#define LPFK_ACK  0x81
#define LPFK_SET  0x94
#define DIALS_SET 0xc0

#define LPFK_RESP 0x03
#define LPFK_TYPE 0x05
#define DIAL_RESP 0x08
#define DIAL_TYPE 0x06
#define LOOP_RESP READ_CONF     /* Read config comes back if wrapped */

#define SGIO_INIT_CMD   0x01
#define SGIO_TERM_CMD   0x02
#define SGIO_OPEN_CMD   0x03
#define SGIO_CLOSE_CMD   0x04
#define SGIO_IOCTL_CMD   0x05
#define SGIO_LPFKS_CMD   0x06
#define SGIO_DIALS_CMD   0x07

#define SGIO_DIALS_SET1   0x01
#define SGIO_DIALS_SET2   0x02

#define SGIO_RESET_DELAY 500000000 /* value in nanoseconds = 500 milliseconds */
#define SGIO_CMD_DELAY 2000000     /* value in nanoseconds = 2 milliseconds */


/*****************************************************************************/
/* function proto types                                                      */
/*****************************************************************************/
extern int addswitch(dev_t);
extern int add_dev(struct sgio_device *);
extern int create_sikproc();
extern int dials_io(struct sgio *, struct sgio_device *, struct trb *);
extern int dials_read(struct sgio_device *, int);
extern sgio_device_t *find_dev(dev_t, int);
extern int lpfks_io(struct sgio *, struct sgio_device *, struct trb *);
extern int lpfks_read(struct sgio_device *, int);
extern int nodev();
extern int perform_io(struct sgio *, struct trb *, int);
extern int poll_update(struct sgio *, struct pollfd **, int *, int *, int *);
extern int purge_sigs();
extern void reg_pm(struct sgio_device *);
extern int remove_dev(struct sgio_device *);
extern int sgio_close(dev_t, chan_t, int);
extern int sgio_config(dev_t, int, struct uio *);
extern int sgio_ioctl(dev_t, int, void *, int, chan_t, int);
extern void sgio_log(char *, char *, uint, uint, char *);
extern int sgio_open(dev_t, int, chan_t, int);
extern long sgio_pm_handler(caddr_t, int);
extern void sgio_putring(struct rcb *, struct ir_report *, void *);
extern void sgio_rflush(int, struct rcb *);
extern int sgio_rring(struct rcb *, char *);
extern int sgio_sleep(struct trb *, int, int, pid_t, int *);
extern int sgio_tty_open(struct sgio_device *);
extern void sgio_uring(struct rcb *);
extern void sgio_watchdog(struct trb *);
extern void sgio_xcpi(caddr_t, caddr_t, uchar, struct rcb *);
extern void sgio_xcpo(caddr_t, caddr_t, uchar, struct rcb *);
extern void sikproc_query(struct trb *);
extern void sikproc_retry(struct trb *);
extern int term_sikproc();
extern void ureg_pm(struct sgio_device *);
extern void _vsprintf(char *, char *, va_list);

#endif /* _H_SGIO */
