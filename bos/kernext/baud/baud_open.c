static char sccsid[] = "@(#)45  1.2  baud_open.c, bos, bos320 4/14/93 16:40:52";
/*
** COMPONENT_NAME: BAUD Device Driver Top Half Routine
**
** FUNCTIONS: baud_open
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
#include <sys/dma.h>
#include "bauddd.h"


/* external definitions */
extern baud_ctrl_t baud_ctrl;
extern driver_t driver_ctrl;
extern int baud_intr_handler();

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * NAME: baud_open (AIX entry point)
 * This is the open entry point to the baud device driver. It is invoked via
 * the open, creat or fp_opendev system calls.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

int
baud_open(devno, devflag, chan, ext)
dev_t devno;    /* major and minor device number */
ulong devflag;  /* defined in file.h */
chan_t chan;    /* channel number    */
int ext;        /* extension val     */
{

    int minor_dev, oldpri;
    int rc = 0;
    struct baud_control *fcp;

    minor_dev = minor(devno);

    lockl(&(driver_ctrl.driver_lock), LOCK_SHORT);
    lockl(&(driver_ctrl.adapt[minor_dev].adapter_lock), LOCK_SHORT);
    rc = lockl(&(driver_ctrl.adapt[minor_dev].channels[chan].chan_lock),
                    LOCK_NDELAY);
    if (rc != LOCK_SUCC) {
        DEBUG1("FAKEDD: baud_open: Unable To Obtain Channel Lock.\n");
        unlockl(&(driver_ctrl.driver_lock));
        unlockl(&(driver_ctrl.adapt[minor_dev].adapter_lock));
        return(EBUSY);
    }

    /*
    ** We now have a lock on the channel that we are working with so
    ** now we can let other processes have access to both the driver,
    ** adapter(s) and the other channels.
    */

    unlockl(&(driver_ctrl.adapt[minor_dev].adapter_lock));
    unlockl(&(driver_ctrl.driver_lock));

    fcp = (struct baud_control *) baud_ctrl.baudcp[minor_dev];

    if (!fcp->adapt_ref_cnt) {         /* Only need to set this up on a */
                                       /* per adapter basis, not channel */

        if (i_init(&(fcp->baud_intr)) != INTR_SUCC) {
            FAKEHKWD1(HKWD_FAKE_DD,FAKE_ERROR_RPT,EIO,"i_init");
            DEBUG1("FAKEDD: baud_open: Unable To Initialize Interrupts.\n");
            return(EIO);
        }

        /* Initialize the watchdog timer */
        (void) w_init((struct watchdog *) (&(fcp->baud_watch)));
        fcp->adapt_ref_cnt++;
    }


    /* Initialize the DMA channel */
    fcp->fdds.channel_id_rd = d_init(fcp->fdds.cap_dma_lvl,
                        MICRO_CHANNEL_DMA | DMA_SLAVE,
                        fcp->fdds.bus_id);

    if(fcp->fdds.channel_id_rd == DMA_FAIL)
      {
      DEBUG1("baudDD: baud_open: Unable To Initialize dma CHANNEL\n");
      return(EIO);
      };

    fcp->fdds.channel_id_wr = d_init(fcp->fdds.play_dma_lvl,
                        MICRO_CHANNEL_DMA | DMA_SLAVE,
                        fcp->fdds.bus_id);

    if(fcp->fdds.channel_id_wr == DMA_FAIL)
      {
      DEBUG1("baudDD: baud_open: Unable To Initialize dma CHANNEL\n");
      return(EIO);
      };

    return(0);
}    /* end baud_open */
