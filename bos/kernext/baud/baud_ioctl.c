static char sccsid[] = "@(#)45  1.2  baud_ioctl.c, bos, bos320 4/14/93 16:40:52"
;
/*
** COMPONENT_NAME: baud_ioctl - io control for BAUD device driver
**
** FUNCTIONS: baud_ioctl
**
** ORIGINS: ME
*/

#include <sys/adspace.h>
#include <sys/dma.h>
#include <sys/devinfo.h>
#include <sys/uio.h>
#include <sys/rcm_win.h>
#include <sys/types.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/sleep.h>
#include <sys/ioacc.h>
#include <sys/trcmacros.h>
#include <sys/m_param.h>
#include <sys/vmuser.h>
#include "bauddd.h"

/* external definitions */
extern baud_ctrl_t baud_ctrl;
extern driver_t driver_ctrl;


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * NAME: baud_ioctl (AIX entry point)
 * This is the ioctl entry point to the baud device driver. It is invoked via
 * the ioctl or fp_ioctl system calls.
 * Inputs:
 *    cmd:  The operation to be performed, ie., read device Id.
 *    arg:  The POS register to be read in some cases, slot otherwise.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

int
baud_ioctl(devno, cmd, arg, devflag, chan, ext)
dev_t devno;
int cmd;
int arg;
ulong devflag;
chan_t chan;
int ext;
{
    caddr_t  eaddr;
    uchar    pos0, pos1;
    ulong    posreg = 0;
    uchar    slotnum, posnum;
    int      minor_dev;
    int      rc = 0;
    struct   baud_control *fcp;

    minor_dev = minor(devno);

    lockl(&(driver_ctrl.driver_lock), LOCK_SHORT);

    /* Get pointer to control structure based on minor dev number */
    fcp = (struct baud_control *) baud_ctrl.baudcp[minor_dev];

    /* Another way to do iocc attach */
    eaddr = IOCC_ATT(fcp->fdds.bus_id, 0);          /* Attach to IOCC */
    switch (cmd) {
    case READ_DEVID_AT_SLOT:   /* Read device id at configured slot location */

        BUS_GETCX((eaddr+((fcp->fdds.slotno - 1) << 16) + POSBASE), &pos0);
        BUS_GETCX((eaddr+((fcp->fdds.slotno - 1) << 16) + POSBASE), &pos1);
        break;

    case READ_POS_REG_AT_SLOT: /* Read POS register value at configured slot */
        posreg = arg;                 /* Get POS register to access from arg */
        BUS_GETCX((eaddr+((fcp->fdds.slotno - 1) << 16) +
                          (POSBASE + posreg)), &pos0);
        break;

    case READ_ALL_DEVICE_IDS:
        for (slotnum = 0; slotnum < 16; slotnum++) {
           BUS_GETCX((eaddr+((slotnum) << 16)+ POSBASE), &pos0);
           BUS_GETCX((eaddr+((slotnum) << 16)+(POSBASE+1)),&pos1);
           /*
           printf("===== >>>> Slot %d: Device Id: %x%x\n",slotnum, pos0, pos1);
           */
        }
        break;

    case READ_ALL_POS_REGISTERS: /* Read all POS registers at specified slot */
        slotnum = arg;
        /*  printf("===== >>>> Slot %d:\n", slotnum);   */
        for (posnum = 0; posnum < 8; posnum++) {
            BUS_GETCX((eaddr+((slotnum-1) << 16) + (POSBASE + posnum)), &pos0);
        /*  printf("      POS%d: %x\n", posnum, pos0);  */
        }
        break;

    case GET_PTR_TO_IOCC:    /* Get a handle to allow direct addressing */
        {
        int   rcode;
        char  *iocc_addr;

        if(fcp->fdds.adsp == NULL)
           fcp->fdds.adsp = getadsp();

        if(fcp->fdds.pos_segr == NULL)
           fcp->fdds.pos_segr = as_att(fcp->fdds.adsp, IOCC_SEG_ADR, 0);

        /* ADD POS BASE AND SLOT OFFSET */
        iocc_addr =
            ((fcp->fdds.slotno - 1) << 16) + POSBASE + fcp->fdds.pos_segr;
        rcode = copyout(&iocc_addr,  (char *)arg,  4);
        break;
        };

    case REL_PTR_TO_IOCC:    /* Get a handle to allow direct addressing */
        {
        int   rcode;

        /* DETACH */
        rcode = as_det(fcp->fdds.adsp, fcp->fdds.pos_segr);
        fcp->fdds.pos_segr = NULL;
        if(fcp->fdds.bus_segr == NULL)
           fcp->fdds.adsp = NULL;
        break;
        };

    case GET_PTR_TO_REGS:  /* Get status of the DMA record process */
        {
        int   rcode;
        char  *regs_addr;

        if(fcp->fdds.adsp == NULL)
           fcp->fdds.adsp = getadsp();

        if(fcp->fdds.bus_segr == NULL)
           fcp->fdds.bus_segr = as_att(fcp->fdds.adsp, CARD_SEG_ADR, 0);

        /* ADD OFFSET OF THIS CARD */
        regs_addr = fcp->fdds.bus_io_addr + fcp->fdds.bus_segr;

        rcode = copyout(&regs_addr,  (char *)arg,  4);
        break;
        };

    case REL_PTR_TO_REGS:    /* Get a handle to allow direct addressing */
        {
        int   rcode;

        /* DETACH */
        rcode = as_det(fcp->fdds.adsp, fcp->fdds.bus_segr);
        fcp->fdds.bus_segr = NULL;
        if(fcp->fdds.pos_segr == NULL)
           fcp->fdds.adsp = NULL;
        break;
        };

    case CAPTURE_DMA_STATUS:  /* Get a handle to allow direct addressing */
        {
        int   rcode,status;
        status = 0x1111;
        if(fcp->fdds.capture_busy) status = 0x5555;
        rcode = copyout((int *)&status,  (int *)arg,  4);
        break;
        };

    case PLAYBACK_DMA_STATUS:  /* Get a handle to allow direct addressing */
        {
        int   rcode,status;
        status = 0x1111;
        if(fcp->fdds.playback_busy) status = 0x5555;
        rcode = copyout((int *)&status,  (int *)arg,  4);
        break;
        };

    default:
        DEBUG2("===== >>>> Invalid IOCTL cmd: %d\n", cmd);
        break;
    }
    IOCC_DET(eaddr);                        /* Release the IOCC */
    unlockl(&(driver_ctrl.driver_lock));

    return(0);
}


