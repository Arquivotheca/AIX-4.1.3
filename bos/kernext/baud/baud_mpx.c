static char sccsid[] = "@(#)45  1.2  baud_mpx.c, bos, bos320 4/14/93 16:40:52"
;
/*
** COMPONENT_NAME: BAUD Device Driver Top Half Routine
**
** FUNCTIONS: baud_mpx
**
** ORIGINS: ME
*/

#include <sys/device.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/dump.h>
#include <sys/trcmacros.h>
#include <sys/watchdog.h>
#include <sys/m_param.h>
#include <sys/timer.h>
#include "bauddd.h"

/* external definitions */
extern int nodev();
extern driver_t driver_ctrl;


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * NAME: baud_mpx (AIX entry point)
 * This is the mpx entry point to the baud device driver. It is invoked via
 * the open, creat or fp_opendev system calls and is called before the
 * baud_open entry point.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

int
baud_mpx(devno, chanp, channame)
dev_t devno;        /* major and minor device number */
chan_t *chanp;      /* Specifies channel ID, passed by reference */
char *channame;     /* Points to pathname extension for the channel */
{

    int minor_dev;

    lockl(&(driver_ctrl.driver_lock), LOCK_SHORT);

    minor_dev = minor(devno);

    if (channame == NULL) {
        /* Release the channel lock that we acquired in the open. */
        unlockl(&(driver_ctrl.adapt[minor_dev].channels[*chanp].chan_lock));

    } else {

        switch(channame[0]) {
        case 'H':
            *chanp = 0;
            break;

        case 'M':
                *chanp = 1;
            break;

        case 'L':
            *chanp = 2;
            break;

        default:
            *chanp = 0;
            break;
        }
    }


    unlockl(&(driver_ctrl.driver_lock));

    return(0);
}
