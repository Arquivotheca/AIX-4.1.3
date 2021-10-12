static char sccsid[] = "@(#)45  1.2  baud_close.c, bos, bos320 4/14/93 16:40:52";
/*
**
** COMPONENT_NAME: BAUD Device Driver Top Half Routine
**
** FUNCTIONS: baud_close
**
** ORIGINS: ME
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
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/xmem.h>
#include <sys/sysconfig.h>
#include "bauddd.h"

/* external definitions */
extern baud_ctrl_t baud_ctrl;
extern driver_t driver_ctrl;

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * NAME: baud_close (AIX entry point)
 * This is the close entry point to the baud device driver. It is invoked via
 * the close or fp_close system calls.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

int
baud_close(devno, chan, ext)
dev_t devno;
chan_t chan;
int ext;
{
    int minor_dev;
    struct baud_control *fcp;


    lockl(&(driver_ctrl.driver_lock), LOCK_SHORT);

    minor_dev = minor(devno);

    fcp = (struct baud_control *) baud_ctrl.baudcp[minor_dev];

    fcp->adapt_ref_cnt--;
    if (!fcp->adapt_ref_cnt) {
        /*
        ** Clear the interrupt handler for this adapter
        */
        (void) i_clear((struct intr *) &(fcp->baud_intr));
        (void) w_clear((struct watchdog *) (&(fcp->baud_watch)));
    }

    /* CLEAR DMA CHANNEL */
    d_clear(fcp->fdds.channel_id_rd);
    d_clear(fcp->fdds.channel_id_wr);

    unlockl(&(driver_ctrl.driver_lock));

    return(0);
}    /* end baud_close */
