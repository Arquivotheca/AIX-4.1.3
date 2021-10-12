/* ?? */
/*
** COMPONENT_NAME: BAUD Device Driver Bottom Half Routine
**
** FUNCTIONS: baud_intr_handler
**
** ORIGINS:
*/

#include <stdio.h>
#include <sys/device.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/adspace.h>
#include <sys/sleep.h>
#include <sys/ioacc.h>
#include <sys/dump.h>
#include <sys/m_param.h>
#include <sys/timer.h>
#include <sys/trcmacros.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include "bauddd.h"
#include "baudregs.h"


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * NAME: baud_intr_handler
 * This is the interrupt handler for the baud device driver.  It is invoked
 * when the device raises the interrupt request line and the FLIH vectors to
 * this routine.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

int
baud_intr_handler(intrp)
struct intr *intrp;
{
    struct baud_control *fcp;
    int my_interrupt = 0;
    char *ioaddr;
    char intr_enable, intr_status, intr_pending, temp;
    int  rc;

    /*
    ** Since the first element of the baud_control structure is the
    ** interrupt handler structure we know the address of the control
    ** structure for the device that has interrupted.
    */
    fcp = (struct baud_control *) intrp;

    /* CHECK TO SEE IF AUDIO DEVICE HAS AN INTERRUPT PENDING */
    /* Get addressability to the io registers */

    ioaddr = io_att(((0x0ff00000 & fcp->fdds.bus_id) | 0x800c0020), 0);

    ioaddr = ioaddr + fcp->fdds.bus_io_addr;
    /* Read the interrupt enable and status registers of the MCI chip */

    BUS_GETCX(ioaddr + MCI_INTR_EN, &intr_enable);
    BUS_GETCX(ioaddr + MCI_INTR_ST, &intr_status);

    intr_pending = intr_enable & intr_status;
    my_interrupt = intr_pending;

    /* Clear all pending interrupts */
/*  BUS_PUTCX(ioaddr + MCI_INTR_AK, intr_pending);    */

    if( intr_pending & TMR_INTR)
      {
      BUS_PUTCX(ioaddr + MCI_INTR_AK, intr_pending);
      BUS_PUTCX(ioaddr + MCI_INTR_EN, 0);
      };

    if( intr_pending & INT4)
      {
      };

    if( intr_pending & INT3)
      {
      };

    if( intr_pending & INT2)
      {
      };

    if( intr_pending & INT1)      /* CODEC interrupt line */
      {
      /* Clear the microchannel interface interrupt bit */
      *(ioaddr + MCI_INTR_AK) = 0x04;            /* reset MCI intr bit */
      /* Get Codec status register */
      BUS_GETCX(ioaddr + AUDIO_STATUS_REG, &temp);
      if(temp & 0x01)      /* if interrupt pending */
        {
        /* Get the alternate status register */
        BUS_PUTCX(ioaddr + AUDIO_INDEX_REG, ALT_FEATURE_STAT_REG | TRD);
        BUS_GETCX(ioaddr + AUDIO_INDEXED_DATA_REG, &temp);
        if( temp & RECORD_INTR )      /* if WRITE dma interrupt */
          {
          /* Stop the capture dma */
          BUS_PUTCX(ioaddr + AUDIO_INDEX_REG, INTERFACE_REG | TRD);
          BUS_GETCX(ioaddr + AUDIO_INDEXED_DATA_REG, &temp);
          temp = temp & ~CEN;
          BUS_PUTCX(ioaddr + AUDIO_INDEXED_DATA_REG, temp );

          /* Clear the interrupt pending in the codec */
          BUS_PUTCX(ioaddr + AUDIO_INDEX_REG, ALT_FEATURE_STAT_REG | TRD);
          BUS_PUTCX(ioaddr + AUDIO_INDEXED_DATA_REG, ~RECORD_INTR);

          if(fcp->fdds.capture_busy)
            {
            fcp->fdds.capture_busy = 0;

            /* TELL THE SYSTEM THAT THE INTERRUPT IS COMPLETE */
            d_complete(fcp->fdds.channel_id_rd,
                     DMA_READ,
                     fcp->cxfer_buf.bufaddr,
                     fcp->cxfer_buf.buflen,
                     &(fcp->cxfer_buf.dp),
                     NULL);


            {
            caddr_t  newbase;       /* temporary segment register */

            /* attach to saved segment register to allow proper unpinning
               of memory pinned in the process environment */
            newbase = vm_att (fcp->cxfer_buf.UsrSeg,
                              fcp->cxfer_buf.bufaddr);

            rc =
            unpin(newbase, fcp->cxfer_buf.buflen);

            vm_det(newbase);
            };

            if(rc) assert(0);

            rc = xmdetach(&(fcp->cxfer_buf.dp));
            if(rc != XMEM_SUCC) assert(0);

            }
          else
            {
            assert(0);
            };
          };

        if( temp & PLAYBACK_INTR )    /* if WRITE dma interrupt */
          {
          /* Stop the playback dma */
          BUS_PUTCX(ioaddr + AUDIO_INDEX_REG, INTERFACE_REG | TRD);
          BUS_GETCX(ioaddr + AUDIO_INDEXED_DATA_REG, &temp);
          temp = temp & ~PEN;
          BUS_PUTCX(ioaddr + AUDIO_INDEXED_DATA_REG, temp );

          /* Clear the interrupt pending in the codec */
          BUS_PUTCX(ioaddr + AUDIO_INDEX_REG, ALT_FEATURE_STAT_REG | TRD);
          BUS_PUTCX(ioaddr + AUDIO_INDEXED_DATA_REG, ~PLAYBACK_INTR);

          if(fcp->fdds.playback_busy)
            {
            fcp->fdds.playback_busy = 0;

            /* TELL THE SYSTEM THAT THE INTERRUPT IS COMPLETE */
            d_complete(fcp->fdds.channel_id_wr,
                     DMA_WRITE_ONLY,
                     fcp->pxfer_buf.bufaddr,
                     fcp->pxfer_buf.buflen,
                     &(fcp->pxfer_buf.dp),
                     NULL);
            {
            caddr_t  newbase;       /* temporary segment register */

            /* attach to saved segment register to allow proper unpinning
               of memory pinned in the process environment */
            newbase = vm_att (fcp->pxfer_buf.UsrSeg,
                              fcp->pxfer_buf.bufaddr);

            rc =
            unpin(newbase, fcp->pxfer_buf.buflen);

            vm_det(newbase);
            };

            if(rc)
              {
              assert(0);
              };

            rc = xmdetach(&(fcp->pxfer_buf.dp));
            if(rc != XMEM_SUCC) assert(0);

            }
          else
            {
            assert(0);
            };

          };    /* end playback intr */

        };   /* end CODEC intr */

      }; /* end of INT 1 */

    io_det(ioaddr);

    if (!my_interrupt) {
        return(INTR_FAIL);
        }
    else {
        /* Here is where I would do a d_complete if I was doing DMA */
        i_reset(&(fcp->baud_intr));
   /*   e_wakeup(&(fcp->sleep_anchor));   */
        return(INTR_SUCC);
        };

      /* DISABLE ALL INTERRUPTS */
  /*  BUS_PUTCX(ioaddr + MCI_INTR_EN, 0);    */
}
