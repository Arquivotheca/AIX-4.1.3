/* ?? */
/*
** COMPONENT_NAME: FAKE Device Driver Top Half Routine
**
** FUNCTIONS: baud_write, baud_dma_write
**
*/

#include <stdio.h>
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
#include <sys/dma.h>
#include "bauddd.h"
#include "baudregs.h"

/* external definitions */
extern baud_ctrl_t baud_ctrl;
extern char global_buf[512];

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * NAME: baud_write (AIX entry point)
 * This is the write entry point to the baud device driver. It is invoked via
 * the write or fp_write system calls.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

int
baud_write(devno, uiop, chan, ext)
dev_t devno;                            /* major and minor device number */
struct uio *uiop;                       /* pointer to user's buffer */
int chan;                               /* channel value assigned in mpx */
int ext;                                /* extension passed by writex() */
{
    int rc, minor_dev;
    int samples_per_transfer;
    struct baud_control *fcp;
    char    *regs_addr, temp;

    samples_per_transfer = ext;         /* dump copy */
    minor_dev = minor(devno);

    fcp = (struct baud_control *) baud_ctrl.baudcp[minor_dev];

    fcp->pxfer_buf.bufaddr = uiop->uio_iov->iov_base;
    fcp->pxfer_buf.buflen = uiop->uio_iov->iov_len;
    fcp->pxfer_buf.dp.aspace_id = XMEM_INVAL;

    /* GET DIRECT POINTER TO REGS */
    regs_addr = fcp->fdds.bus_io_addr + fcp->fdds.bus_segr;

    /*
    ** I am going to cross memory attach to the users buffer.
    */

    rc = xmattach(fcp->pxfer_buf.bufaddr, fcp->pxfer_buf.buflen,
                  &(fcp->pxfer_buf.dp), uiop->uio_segflg);

    if (rc != XMEM_SUCC) {
        DEBUG2("BAUDDD: baud_writ: Cross Memory Attached Failed: %d\n", rc);
        return(EFAULT);
        };

    rc = pinu(fcp->pxfer_buf.bufaddr, fcp->pxfer_buf.buflen, UIO_USERSPACE);

    /* Save the segment register value for the interrupt routine to use */
    fcp->pxfer_buf.UsrSeg =
        as_geth( getadsp(), fcp->pxfer_buf.bufaddr);


    if (rc) {
        DEBUG2("BAUDDD: baud_writ: Pin of Memory Failed: %d\n", rc);
        return(EFAULT);
        };

    fcp->xfer_error = 0;                       /* Initialize */

    /* SET UP THE DMA SYSTEM HARDWARE */
    d_slave(fcp->fdds.channel_id_wr, DMA_WRITE_ONLY,
                                   fcp->pxfer_buf.bufaddr,
                                   fcp->pxfer_buf.buflen,
                                   &(fcp->pxfer_buf.dp));

    rc = 0;

    /* START THE ADAPTER DMA FUNCTION */
    *(regs_addr + AUDIO_INDEX_REG) = PLAYBACK_LOCOUNT_REG | TRD;
    *(regs_addr + AUDIO_INDEXED_DATA_REG) = samples_per_transfer & 0xff;
    *(regs_addr + AUDIO_INDEX_REG) = PLAYBACK_UPCOUNT_REG | TRD;
    *(regs_addr + AUDIO_INDEXED_DATA_REG) =
                               (samples_per_transfer & 0xff00) >> 8;

    *(regs_addr + AUDIO_INDEX_REG) = PLAYBACK_LOCOUNT_REG | TRD;
if((*(regs_addr + AUDIO_INDEXED_DATA_REG))!=(samples_per_transfer & 0xff))
       {
       rc = 0x22;
       };
    *(regs_addr + AUDIO_INDEX_REG) = PLAYBACK_UPCOUNT_REG | TRD;
if((*(regs_addr + AUDIO_INDEXED_DATA_REG))!=
                              ((samples_per_transfer & 0xff00) >> 8))
       {
       rc = 0x22;
       };

  if(rc == 0)
    {
    *(regs_addr + MCI_INTR_AK) = INT1;                 /* clear pending */
    *(regs_addr + MCI_INTR_EN) = INT1;                 /* enable int 1  */
    temp = *(regs_addr + MCI_REG1);
    *(regs_addr + MCI_REG1)    = EN_BURST2 | EN_DMA2 | temp;

    *(regs_addr + AUDIO_INDEX_REG) = PIN_CNTL_REG | TRD;
    *(regs_addr + AUDIO_INDEXED_DATA_REG) = 0x02;      /* Enable codec int */

    fcp->fdds.playback_busy = 1;
    *(regs_addr + AUDIO_INDEX_REG) = INTERFACE_REG | TRD; /*               */
    temp = *(regs_addr + AUDIO_INDEXED_DATA_REG);
    *(regs_addr + AUDIO_INDEXED_DATA_REG) = PEN | temp;     /* Play enable */

    return(rc);
    }
  else
    {
    fcp->fdds.playback_busy = 0;

    /* TELL THE SYSTEM THAT THE INTERRUPT IS COMPLETE */
    d_complete(fcp->fdds.channel_id_wr,
             DMA_WRITE_ONLY,
             fcp->pxfer_buf.bufaddr,
             fcp->pxfer_buf.buflen,
             &(fcp->pxfer_buf.dp),
             NULL);
    xmdetach(&(fcp->pxfer_buf.dp));
    unpinu(fcp->pxfer_buf.bufaddr, fcp->pxfer_buf.buflen, UIO_USERSPACE);

    return(rc);
    };
}    /* end of write */

