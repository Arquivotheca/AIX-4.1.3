static char sccsid[] = "@(#)45  1.2  baud_config.c, bos, bos320 4/14/93 16:40:52";
/*
** COMPONENT_NAME: Device Driver Top Half Routine for Foxclub Diagnostics
**
** FUNCTIONS: cfgbuad              load_devsw
**            initialize_device    setup_adapter
**
** ORIGINS:
*/

#include <sys/types.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/sleep.h>
#include <sys/watchdog.h>
#include <sys/dump.h>
#include <sys/trcmacros.h>
#include <sys/m_param.h>
#include <sys/timer.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/xmem.h>
#include <sys/sysconfig.h>
#include "bauddd.h"
#include "baudregs.h"

/* external definitions */
extern int nodev();
extern struct baud_cdt_tab *baud_cdt;
extern void baud_watchdog(struct watchdog *watchdog_ptr);
extern void baud_timer_func();
extern int baud_intr_handler();
extern baud_ctrl_t baud_ctrl;
extern struct cdt * baud_cdt_func(int arg);

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    global data definitions
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

int baud_open(dev_t devno, ulong devflag, chan_t chan, int ext);
int baud_close(dev_t devno, chan_t chan, int ext);
int baud_read(dev_t devno, struct uio *uiop, chan_t chan, int ext);
int baud_write(dev_t devno, struct uio *uiop, chan_t chan, int ext);
int baud_mpx(dev_t devno, chan_t *p_chan, char *p_channame);
int baud_ioctl(dev_t devno,int cmd,int arg,ulong devflag,chan_t chan,int ext);

driver_t driver_ctrl;                          /* Device Driver Lock */

lock_t ConfigLock = LOCK_AVAIL;

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * NAME: baud_config (AIX entry point)
 * This is the main entry point to the module. It is invoked via the
 * sysconfig system call from the configuration method, cfgbaud.
 *
 * Parameter:
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

int
baud_config(devno, cmd, uiop)
dev_t   devno;        /* major-minor device number    */
int     cmd;          /* command                      */
struct uio *uiop;     /* uio structure containing device dependent data */
{
    register int rc;
    register int first_configure = FALSE;
    struct baud_control *fcp;
    int minor_dev;

    /* Obtain a non-blocking lock during configuration of driver */
    if (lockl(&ConfigLock, LOCK_NDELAY) != LOCK_SUCC) {
        DEBUG1("DIAGDD: baud_config: Unable To Obtain ConfigLock.\n");
        return(EBUSY);
    }

    switch(cmd) {
    case CFG_INIT:

        rc = CONF_SUCC;
        if (!baud_ctrl.configured) {   /* First CFG_INIT must install device */
                                       /* switch table, dump table, etc.     */

            first_configure = TRUE;

            /* Try adding the device switch table entries */
            if (rc = load_devsw(devno))
                break;

            /*
            ** Initialize the component dump table for the baud device
            ** driver. When the system goes belly up, I would like to have my
            ** stuff accessible via crash
            */

            baud_cdt = (struct baud_cdt_tab *) xmalloc((uint)
                         (sizeof(struct baud_cdt_tab) * NADAPTERS),
                         (uint) 2, pinned_heap);

            if (baud_cdt == NULL) {             /* xmalloc failed */
                (void) devswdel(devno);
                rc = EIO;
                break;
            }

            bzero((char *) baud_cdt, sizeof(struct baud_cdt_tab));

            /*
            ** This routine fdds the dump table information by calling
            ** the baud_cdt_func().
            */
            rc = dmp_add((void (*) ()) baud_cdt_func);
            if (rc != 0) {
                DEBUG2("DIAGDD: baud_config: dmp_add failed: %d\n", rc);
                (void) devswdel(devno);
                (void) xmfree((caddr_t) baud_cdt, pinned_heap);
                break;
            }

            /*
            ** Since we have added our dump table to the system and that
            ** later on we will have code running out of interrupt context,
            ** lets pin the bottom half of the device driver.
            */

            if (rc = pincode(baud_watchdog)) {
                DEBUG2("DIAGDD: baud_config: pincode failed: %d\n", rc);
                (void) devswdel(devno);
                (void) dmp_del(((void (*)()) baud_cdt_func));
                (void) xmfree((caddr_t) baud_cdt, pinned_heap);
                break;
            }

            driver_ctrl.driver_lock = LOCK_AVAIL;   /* Initialize driver lock */
            baud_ctrl.configured = TRUE;

        }     /* Close of "if (!baud_ctrl.configured)" */


        /* For all devices attached we call the initialize_device() */
        rc = initialize_device(devno, uiop);
        if (rc && first_configure) {
            /* Possible errors are EINVAL, ENOMEM, EIO, EFAULT */
            /* Also, we only do the following on the first time through */
            (void) devswdel(devno);
            (void) dmp_del(((void (*)()) baud_cdt_func));
            (void) xmfree((caddr_t) baud_cdt, pinned_heap);
            (void) unpincode(baud_watchdog);
            baud_ctrl.configured = FALSE;
            break;
        }

        break;

    case CFG_TERM:

        rc = CONF_SUCC;
        if (!baud_ctrl.configured) {         /* Don't do if it hasn't been */
            rc = EINVAL;                     /* configured yet.            */
            DEBUG1("DIAGDD: baud_config: Device not configured.\n");
            break;
        }

        /*
        ** We will use the minor number to tell which control structure
        ** needs to be deleted.
        */

        minor_dev = minor(devno);

        if (minor_dev >= NADAPTERS) {
            DEBUG2("DIAGDD: baud_config: Too many devices: %d\n", minor_dev);
            return(EINVAL);
        }

        if (baud_ctrl.baudcp[minor_dev] == NULL) {
            /* This device has never been configured. */
            rc = EINVAL;
            DEBUG1("DIAGDD: baud_config: Device hasn't been setup.\n");
            break;
        }

        /*
        ** Setup a temporary pointer otherwise the pointer arithmetic gets
        ** really messy.
        */
        fcp = (struct baud_control *) baud_ctrl.baudcp[minor_dev];

        /* First unallocate the Timer Request Block and WatchDog Timers. */
        (void) tfree(fcp->ftrbp);

        /* Free up the control structure associated with this device */
        (void) xmfree((caddr_t) baud_ctrl.baudcp[minor_dev], pinned_heap);

        baud_ctrl.num_dds--;

        if (baud_ctrl.num_dds == 0) {
            /*
            ** This is the last device that is being unconfigured so we
            ** have to do some additional work.
            */

            /* Remove dump table allocation */
            if (baud_cdt != (struct baud_cdt_tab *) NULL) {
                (void) dmp_del(baud_cdt_func);
                (void) xmfree((caddr_t) baud_cdt, pinned_heap);
            }

            if (devswdel(devno)) {    /* Remove switch table entries */
                DEBUG1("DIAGDD: baud_config: Cannot remove devsw entries.\n");
                };

            /* Ok to unpin the bottom half now */
            (void) unpincode(baud_watchdog);
        }

        break;


    case CFG_QVPD:
        if (!baud_ctrl.configured) {        /* Don't do if it hasn't been */
            rc = EINVAL;                    /* configured yet.            */
            DEBUG1("DIAGDD: baud_config: CFG_QVPD: Device not configured.\n");
            break;
        }

    default:
        DEBUG1("    >>>> baud_config : invalid operation. <<<<\n");
        rc = EINVAL;
        break;
    }


    unlockl(&ConfigLock);
    return(rc);

}    /* end baud_config */

/*
** This section of code will install the device switch table entries into
** the Kernel's Device Switch Table.  Remember in AIX Version 3, there is
** only one switch table for both character and block devices.
*/

load_devsw(devno)
dev_t devno;
{
    int rc = CONF_SUCC;
    struct devsw baud_dsw;

    /* Initialize the device switch table */
    baud_dsw.d_open     = (int(*)())baud_open;    /* open routine */
    baud_dsw.d_close    = (int(*)())baud_close;   /* close routine */
    baud_dsw.d_read     = (int(*)())baud_read;    /* read routine */
    baud_dsw.d_write    = (int(*)())baud_write;   /* write routine */
    baud_dsw.d_ioctl    = (int(*)())baud_ioctl;   /* ioctl routine */
    baud_dsw.d_strategy = nodev;                   /* strategy routine */
    baud_dsw.d_ttys     = NULL;                    /* tty device structure */
    baud_dsw.d_select   = nodev;                   /* select routine */
    baud_dsw.d_config   = (int(*)())baud_config;  /* config routine */
    baud_dsw.d_print    = nodev;                   /* print routine */
    baud_dsw.d_dump     = nodev;                   /* dump routine */
    baud_dsw.d_mpx      = (int(*)())baud_mpx;     /* mpx routine */
/*  baud_dsw.d_mpx      = nodev;                     mpx routine */
    baud_dsw.d_revoke   = nodev;                   /* revoke routine */
    baud_dsw.d_dsdptr   = (caddr_t) &baud_ctrl;     /* device specific data */
    baud_dsw.d_selptr   = NULL;                    /* device specific data */
    baud_dsw.d_opts     = 0;                       /* device specific data */


    /* Add device entry to dev switch table, and get pointer */
    if (rc = devswadd(devno, &baud_dsw))
        DEBUG1("FAILED to add entry points.\n");

    return(rc);
}


/*
** This is section of code that will set up the device through the POS
** registers.
*/

initialize_device(devno, uiop)
dev_t devno;
struct uio *uiop;
{
    char posval = 0;
    ulong posid = 0;
    ulong ioaddr;
    int minor_no, rc;
    struct baud_control *fcp;

    minor_no = minor(devno);

    if (minor_no >= NADAPTERS) {
        DEBUG2("DIAGDD: initialize_device: Too many devices: %d\n", minor_no);
        return(EINVAL);
    }

    /*
    ** Be a real bummer if I got passed the wrong DDS structure.  Milage may
    ** vary on how far I or the system goes before "888". uio_resid tells me
    ** how much configure method is passing.
    */
    if (uiop->uio_resid != sizeof(struct baud_dds)) {
        DEBUG1("DIAGDD: initialize_device: Wrong DDS being passed.\n");
        return(EINVAL);
    }

    /* Finally, let's make sure that this hasn't been done before. */

    if (baud_ctrl.baudcp[minor_no] != NULL) {
        DEBUG1("DIAGDD: initialize_device: DDS already setup.\n");
        return(EINVAL);
    }

    baud_ctrl.baudcp[minor_no] = (struct baud_control *)
                xmalloc(sizeof(struct baud_control), 3, pinned_heap);

    if (baud_ctrl.baudcp[minor_no] == NULL) {
        DEBUG1("DIAGDD: initialize_device: xmalloc failed.\n");
        return(ENOMEM);
    }

    fcp = baud_ctrl.baudcp[minor_no];
    bzero(fcp, sizeof(struct baud_control));    /* Zero it out */

    /*
    ** Copy in device dependent information.  This comes from the ddsptr
    ** argument in the cfg_dd structure of baud_config's sysconfig call.
    */

    if ((rc=uiomove(&(fcp->fdds),sizeof(struct baud_dds),UIO_WRITE,uiop)) != 0)
    {
        DEBUG1("DIAGDD: initialize_device: Error moving DDS with uiomove.\n");
        (void) xmfree(baud_ctrl.baudcp[minor_no], pinned_heap);
        baud_ctrl.baudcp[minor_no] = NULL;
        return(rc);
    }

    /*
    ** One more security check before we go on.  Check to see if the POS ID
    ** matches the slot number and the BUS that is passed to us.
    */

    ioaddr = io_att(((0x0ff00000 & fcp->fdds.bus_id) | 0x800c00e0), POSBASE);
    ioaddr = ioaddr | ((fcp->fdds.slotno - 1) << 16);

    BUS_GETCX((caddr_t) ioaddr, &posval);
    posid = posval;
    BUS_GETCX((caddr_t) (ioaddr+1), &posval);
    posid = (posid << 8) | posval;
    io_det(ioaddr);

    if (posid != BAUD_CARD_ID) {
        DEBUG2("DIAGDD: initialize_device: Wrong device: %08x\n", posid);
        (void) xmfree(baud_ctrl.baudcp[minor_no], pinned_heap);
        baud_ctrl.baudcp[minor_no] = NULL;
        return(EINVAL);
    }

    /*
    ** Now is the time to go out and set up the adapter card's POS registers.
    */
    if ((rc = setup_adapter(fcp)) != 0) {
        DEBUG2("DIAGDD: initialize_device: Setup of adapter failed: %d\n", rc);
        (void) xmfree(baud_ctrl.baudcp[minor_no], pinned_heap);
        baud_ctrl.baudcp[minor_no] = NULL;
        return(EINVAL);
    }

    /*
    ** Now initialize the timers and interrupt handler structures.
    ** Later in the other sections of the driver, they will be used.
    */

    fcp->baud_intr.next = (struct intr *) NULL;    /* Initialize interrupts */
    fcp->baud_intr.handler = baud_intr_handler;
    fcp->baud_intr.bus_type = BUS_MICRO_CHANNEL;
    fcp->baud_intr.flags = 0;
    fcp->baud_intr.level = fcp->fdds.bus_intr_lvl;
    fcp->baud_intr.priority = fcp->fdds.intr_priority;
    fcp->baud_intr.bid = fcp->fdds.bus_id;
    fcp->baud_intr.i_count = 0;

    fcp->baud_watch.next = NULL;            /* Initialize watchdog struct */
    fcp->baud_watch.prev = NULL;
    fcp->baud_watch.func = baud_watchdog;
    fcp->baud_watch.count = 0;
    fcp->baud_watch.restart = 5;            /* Sleep for 5 seconds */

    fcp->sleep_anchor = EVENT_NULL;          /* Initializing event words */

    /*
    ** Initialize all of this adapters lock words and the control lock.
    */
    driver_ctrl.adapt[minor_no].adapter_lock = LOCK_AVAIL;
    driver_ctrl.adapt[minor_no].channels[0].chan_lock = LOCK_AVAIL;
    driver_ctrl.adapt[minor_no].channels[1].chan_lock = LOCK_AVAIL;
    driver_ctrl.adapt[minor_no].channels[2].chan_lock = LOCK_AVAIL;
    fcp->ctrl_lock = EVENT_NULL;

    fcp->ftrbp = (struct trb *) talloc();    /* Initialize timer */
    if (fcp->ftrbp == NULL) {
        DEBUG1("DIAGDD: initialize_device: Unable to allocate trb struct.\n");
        (void) xmfree(baud_ctrl.baudcp[minor_no], pinned_heap);
        baud_ctrl.baudcp[minor_no] = NULL;
        return(ENOMEM);
    }

    /* Setup initial TRB structure.  timeout element filled out later */
    fcp->ftrbp->flags &= ~(T_ABSOLUTE);      /* Use incremental timing */
    fcp->ftrbp->func = (void (*)())baud_timer_func;
    fcp->ftrbp->t_func_data = (ulong) fcp;
    fcp->ftrbp->ipri = INTCLASS3;


    baud_ctrl.num_dds++;               /* Bump number of dds configured */

    return(CONF_SUCC);
}


setup_adapter(fcp)
struct baud_control *fcp;
{
    int rc;
    uchar pos[6];                /* POS Registers of adapter card */
    uchar val;                   /* Card number read from I/O address space */
    uchar window_sz_val;         /* We only store a value to represent size */
    caddr_t ioaddr;              /* IOCC address space */
    int intrlvl;                 /* Assign interrupt level based on table */

    /*
    ** Obtain the interrupt level from the device dependent structure
    ** that is passed from the configuration method to the driver.
    ** Verify that the interrupt level is correct.
    */


    fcp->fdds.capture_busy = 0;
    fcp->fdds.playback_busy = 0;



    /************************************************************************
    **                                                                     **
    ** The following table shows the layout of the POS Registers for this  **
    ** card:                                                               **
    **              bit   7    6    5    4    3    2    1    0             **
    **   pos                                                               **
    **    0             ..............devid byte 1............             **
    **    1             ..............devid byte 2............             **
    **    2 (prg last)  LBAE  POS BURST  .dma ch1 arb lvl.  CE  00XAAAA1   **
    **    3             ...(bits 15-8 for address decode).....             **
    **    4             ...............................IRQ MAP  00000001   **
    **    5             .....CHCK......FAIR .dma ch2 arb lvl..  1000AAAA   **
    **    6             ......................................  00000000   **
    **    7               ...port1 clock... ....window size...             **
    **                                                                     **
    **                                                                     **
    **                                                                     **
    **   I/O Registers                                                     **
    **                                                                     **
    ************************************************************************/

#if 0
    printf("bus_intr_lvl = %x\n", fcp->fdds.bus_intr_lvl);
    printf("intr_priority= %x\n", fcp->fdds.intr_priority);
#endif

    switch(fcp->fdds.bus_intr_lvl)
      {
      case 9:
         intrlvl = 0x00;
         break;

      case 10:
         intrlvl = 0x55;
         break;

      case 11:
         intrlvl = 0xAA;
         break;

      case 12:
         intrlvl = 0xFF;
         break;

      default:
         return(EINVAL);
      }


    pos[2] = ENABLE | ((fcp->fdds.cap_dma_lvl & 0xf) << 1) | TRD;
    pos[3] = (char)(fcp->fdds.bus_io_addr >> 8);
    pos[4] = intrlvl;
    pos[5] = CHCK_N | (fcp->fdds.play_dma_lvl & 0xf);
    pos[6] = 0;


    /*
    ** Attach to the IOCC address space to setup POS registers for this
    ** adapter card.  POSBASE = 0x400000.  Slotnumber is or'ed into this.
    */

    ioaddr = io_att(((0x0ff00000 & fcp->fdds.bus_id) | 0x800c00e0), POSBASE);
    ioaddr = ((ulong) ioaddr) | ((fcp->fdds.slotno - 1) << 16);
    BUS_PUTCX(ioaddr+6, pos[6]);          /* Initialize POS 6 */
    BUS_PUTCX(ioaddr+5, pos[5]);          /* Initialize POS 5 */
    BUS_PUTCX(ioaddr+4, pos[4]);          /* Initialize POS 4 */
    BUS_PUTCX(ioaddr+3, pos[3]);          /* Initialize POS 3 */
    BUS_PUTCX(ioaddr+2, pos[2]);          /* Initialize POS 2, Enable card */

    io_det(ioaddr);

    /*
    ** Attach to BUS I/O and BUS MEM address space to verify setup is
    ** correct.  ioaddr will end up pointing to the base of MEM/IO.
    ** Remember that the first 64k is reserved for I/O.
    */

    /* PROGRAM THE MICROCHANNEL CHIP TO INITIALIZE CARD */

    ioaddr = io_att(((0x0ff00000 & fcp->fdds.bus_id) | 0x800c0020), 0);

    BUS_PUTCX(ioaddr+fcp->fdds.bus_io_addr+0, 0x30);
    BUS_PUTCX(ioaddr+fcp->fdds.bus_io_addr+2, 0x04);
    BUS_PUTCX(ioaddr+fcp->fdds.bus_io_addr+3, 0);
    BUS_PUTCX(ioaddr+fcp->fdds.bus_io_addr+4, 0);

    BUS_GETCX(ioaddr+fcp->fdds.bus_io_addr + 0, &val);

    /* EXAMPLE
    BUS_GETCX(ioaddr+fcp->fdds.bus_io_addr + CARD_ADDRESS, &val);
    */

    io_det(ioaddr);

    return(0);
}
