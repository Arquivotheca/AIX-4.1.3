/* ?? */
/*
** COMPONENT_NAME: BAUD Device Driver Read Routine/DMA Read Routine.
**
** FUNCTIONS: baud_read, baud_dma_read
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
 * NAME: baud_read (AIX entry point)
 * This is the read entry point to the baud device driver. It is invoked via
 * the read or fp_read system calls.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

int
baud_read(devno, uiop, chan, ext)
dev_t devno;                            /* major and minor device number */
struct uio *uiop;                       /* pointer to user's buffer */
int chan;                               /* channel value assigned in mpx */
int ext;                                /* extension passed by readx() */
{                                       /*  ext = byte count for MCI chip */

    int rc, minor_dev;
    int samples_per_transfer;
    struct baud_control *fcp;
    char    *regs_addr,temp;

    samples_per_transfer = ext;         /* dump copy */
    minor_dev = minor(devno);

    fcp = (struct baud_control *) baud_ctrl.baudcp[minor_dev];

    fcp->cxfer_buf.bufaddr = uiop->uio_iov->iov_base;
    fcp->cxfer_buf.buflen = uiop->uio_iov->iov_len;
    fcp->cxfer_buf.dp.aspace_id = XMEM_INVAL;

    /* GET DIRECT POINTER TO REGS */
    regs_addr = fcp->fdds.bus_io_addr + fcp->fdds.bus_segr;

    /*
    ** I am going to cross memory attach to the users buffer.
    */

    rc = xmattach(fcp->cxfer_buf.bufaddr, fcp->cxfer_buf.buflen,
                  &(fcp->cxfer_buf.dp), uiop->uio_segflg);

    if (rc != XMEM_SUCC) {
        DEBUG2("BAUDDD: baud_read: Cross Memory Attached Failed: %d\n", rc);
        return(EFAULT);
        };

    rc = pinu(fcp->cxfer_buf.bufaddr, fcp->cxfer_buf.buflen, UIO_USERSPACE);

    /* Save the segment register value for the interrupt routine to use */
    fcp->cxfer_buf.UsrSeg =
        as_geth( getadsp(), fcp->cxfer_buf.bufaddr);

    if (rc) {
        DEBUG2("BAUDDD: baud_read: Pin of Memory Failed: %d\n", rc);
        return(EFAULT);
        };


    fcp->xfer_error = 0;                       /* Initialize */

    /* SET UP THE DMA SYSTEM HARDWARE */
    d_slave(fcp->fdds.channel_id_rd, DMA_READ,
                                   fcp->cxfer_buf.bufaddr,
                                   fcp->cxfer_buf.buflen,
                                   &(fcp->cxfer_buf.dp));

    rc = 0;


    /* START THE ADAPTER DMA FUNCTION */
    *(regs_addr + AUDIO_INDEX_REG) = CAPTURE_LOCOUNT_REG | TRD;
    *(regs_addr + AUDIO_INDEXED_DATA_REG) = samples_per_transfer & 0xff;
    *(regs_addr + AUDIO_INDEX_REG) = CAPTURE_UPCOUNT_REG | TRD;
    *(regs_addr + AUDIO_INDEXED_DATA_REG) =
                               (samples_per_transfer & 0xff00) >> 8;

    *(regs_addr + AUDIO_INDEX_REG) = CAPTURE_LOCOUNT_REG | TRD;
if((*(regs_addr + AUDIO_INDEXED_DATA_REG))!= (samples_per_transfer & 0xff) )
       {
       rc = 0x22;
       };
    *(regs_addr + AUDIO_INDEX_REG) = CAPTURE_UPCOUNT_REG | TRD;
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
    *(regs_addr + MCI_REG1) = EN_BURST1 | EN_DMA1 | temp;

    *(regs_addr + AUDIO_INDEX_REG) = PIN_CNTL_REG | TRD;
    *(regs_addr + AUDIO_INDEXED_DATA_REG) = 0x02;      /* Enable codec int */

    fcp->fdds.capture_busy = 1;
    *(regs_addr + AUDIO_INDEX_REG) = INTERFACE_REG | TRD; /*               */
    temp = *(regs_addr + AUDIO_INDEXED_DATA_REG);
    *(regs_addr + AUDIO_INDEXED_DATA_REG) = CEN | temp;   /* Capture enable */

    return(0);
    }
  else
    {
    fcp->fdds.capture_busy = 0;

    /* TELL THE SYSTEM THAT THE INTERRUPT IS COMPLETE */
    d_complete(fcp->fdds.channel_id_rd,
             DMA_READ,
             fcp->cxfer_buf.bufaddr,
             fcp->cxfer_buf.buflen,
             &(fcp->cxfer_buf.dp),
             NULL);
    xmdetach(&(fcp->cxfer_buf.dp));
    unpinu(fcp->cxfer_buf.bufaddr, fcp->cxfer_buf.buflen, UIO_USERSPACE);
    return(rc);
    };
}   /* end of read */

