static char sccsid[] = "@(#)55  1.22  src/bos/kernext/cat/cat_pcaio.c, sysxcat, bos411, 9428A410j 2/22/94 16:51:43";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: cat_pos_read(), cat_pos_readrc(), cat_pos_write(),
 * cat_pos_write_rc(), cat_read(), cat_readrc(), cat_write(), cat_writerc(),
 * letni16(), letni32(), cat_wait_sram(), cat_user_write(), cat_user_read(),
 * cat_check_status(), cat_init_fifos(), cat_get_stat(), cat_put_cmd(),
 * cat_get_cfb(), cat_put_rfb(), cat_get_sfb(), cat_ret_buf()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define FNUM  12
#include <sys/types.h>
#include <sys/except.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/ioacc.h>
#include <sys/sleep.h>
#include <sys/poll.h>
#include <sys/signal.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>

#include "catdd.h"


void cat_init_fifos ();
void cat_shutdown ();

extern int caglobal_lock;
/**************************************************************************
** This module contains "primitives" to access the PCA.  It contains
** functions that access shared memory and pos registers.
**
** The following routines are not accessed directly, but through macros
** defined in catdd.h:
**
**      cat_pos_read is accessed through CAT_POSREAD
**
**      cat_pos_write is accessed through CAT_POSWRITE
**
**      cat_read and cat_readrc are accessed through CAT_READ and CAT_READ1
**
**      cat_write and cat_writerc are accessed through CAT_WRITE and CAT_WRITE1
**
** These are the routines that read the Microchannel bus.  They all
** assume that the proper attach routine (BUSMEM_ATT or IOCC_ATT) has
** already been done and that it's returned value is stored in
** loc->bus_addr.  The only difference between cat_pos_read and cat_read
** is that a slightly different (?) macro is used to do the actual i/o
** and the debug printing is different.
**
** NOTE:  all of the above macros can be called from both the
**              process and interrupt environments
**
**************************************************************************/


/*****************************************************************************
** NAME:        cat_pos_read
**
** FUNCTION:    Read the specified pos register.
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**              This routine assumes that IOCC_ATT is in effect.
**
** RETURNS:
**
*****************************************************************************/
int
cat_pos_read(struct pos_rd *loc)
{
        *loc->data = BUSIO_GETC(loc->bus_addr + loc->addr);
        return 0;
} /* cat_pos_read() */


/*****************************************************************************
** NAME:        cat_pos_readrc
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**
** RETURNS:
**
*****************************************************************************/
int
cat_pos_readrc(
struct mem_acc *loc,
int action,
struct pio_except *infop)
{
        return(0);
} /* cat_pos_readrc() */


/*****************************************************************************
** NAME:                cat_pos_write
**
** FUNCTION:    Write the specified pos register.
**
** EXECUTION
** ENVIRONMENT:
**
** NOTES:
**                              This routine assumes that IOCC_ATT is in effect.
**
** Input:
**
** Output:
**
** Called By:
**
** Calls To:
**
*****************************************************************************/
int
cat_pos_write(struct pos_wr *loc)
{
        BUSIO_PUTC(loc->bus_addr + loc->addr, loc->data);
        return 0;
} /* cat_pos_write() */


/*****************************************************************************
** NAME:        cat_pos_writerc
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**
** RETURNS:
**
*****************************************************************************/
int
cat_pos_writerc(
struct mem_acc *loc,
int action,
struct pio_except *infop)
{
        return(0);
} /* cat_pos_writerc() */


/*****************************************************************************
** NAME:                cat_read
**
** FUNCTION:    Read the specified address in shared ram.
**
** EXECUTION
** ENVIRONMENT:
**
** NOTES
**                              Since the segment register value contains the top 4 bits,
**                              this address should only contain the bottom 28 bits.
**                              This routine assumes that BUSMEM_ATT is in effect.
**
** Input:
**
** Output:
**
** Called By:
**
** Calls To:
**
*****************************************************************************/
int
cat_read(struct mem_acc *loc)
{
#ifndef FASTMEM
        int     len = loc->len;
        uchar * data = loc->data;
        unsigned long   addr = loc->addr & 0x0fffffff;

        while (len >= 4) {
                BUSIO_GETSTR(data, loc->bus_addr + addr, 4);
                data += 4;
                addr += 4;
                len -= 4;
        }
        if (len > 0) {
                BUSIO_GETSTR(data, loc->bus_addr + addr, len);
        }
/*   #else */
        BUSIO_GETSTR(loc->data, loc->bus_addr + (loc->addr & 0x0fffffff), loc->len);
#endif
        return 0;
} /* cat_read() */


/*****************************************************************************
** NAME:        cat_readrc
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**
** RETURNS:
**
*****************************************************************************/
int
cat_readrc(
struct mem_acc *loc,
int action,
struct pio_except *infop)
{
        loc->ca->retry++;
        if (action == PIO_RETRY)
                cat_read(loc);
        return(0);
} /* cat_readrc() */


/*****************************************************************************
** NAME:        cat_write
**
** FUNCTION:    Write to the specified address in shared ram.
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**              Since the segment register value contains the top 4 bits,
**              this address should only contain the bottom 28 bits.
**              This routine assumes that BUSMEM_ATT is in effect.
**
** RETURNS:
**
*****************************************************************************/
int
cat_write(struct mem_acc *loc)
{
        /* BUSIO_PUTSTR(loc->bus_addr + (loc->addr & 0x0fffffff), loc->data, loc->len); */
        return 0;
} /* cat_write() */


/*****************************************************************************
** NAME:        cat_writerc
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT:
**
** RETURNS:
**
*****************************************************************************/
int
cat_writerc(
        struct mem_acc *loc,
        int action,
        struct pio_except *infop)
{
        loc->ca->retry++;
        if (action == PIO_RETRY)
                cat_write(loc);
        return(0);
} /* cat_writerc() */


/*****************************************************************************
** NAME:                letni16
**
** FUNCTION:    Swap bytes (in a word) for the adapter.
**
** EXECUTION
** ENVIRONMENT:
**
** RETURNS:
**
*****************************************************************************/
void
letni16(twobyte_t *data_in)
{
#ifndef NOSWAP
        register int tmp;

        tmp = data_in->byte1;
        data_in->byte1 = data_in->byte2;
        data_in->byte2 = tmp;
#endif
        return;
} /* letni16() */


/*****************************************************************************
** NAME:        letni32
**
** FUNCTION:    Swap bytes in both words then swap words in the
**                              double word for the adapter.
**
** EXECUTION ENVIRONMENT:
**
** Input:       B1.B2.B3.B4
**
** Output:      B4.B3.B2.B1
**
** Called By:   Everybody
**
** Calls:       None
**
** RETURNS:
**
*****************************************************************************/
void
letni32(fourbyte_t *data_in)
{
#ifndef NOSWAP
        register int tmp;

        tmp = data_in->byte1;
        data_in->byte1 = data_in->byte4;
        data_in->byte4 = tmp;
        tmp = data_in->byte2;
        data_in->byte2 = data_in->byte3;
        data_in->byte3 = tmp;
#endif
        return;
} /* letni32() */


/*****************************************************************************
** NAME:        cat_wait_sram
**
** FUNCTION:    Wait for a change in the value at an address.
**              Given an address and a value, this routine will return after
**              n "ticks" or when the address no longer contains the value.
**              This assumes the mem is not attached and will exit that way.
**
** EXECUTION ENVIRONMENT:       process thread only
**
** Input:       pointer to channel adapter structure
**                      pointer to open structure
**                      bus memory address
**                      comparison value
**                      pointer to timeout register, value is in ticks
**                      pointer to value register
**
** Output:      The value read is output in the value register.
**              The number of ticks left is output in the  timeout register.
**
** Returns:     -1 ---- timeout occurred
**              0  ---- Successful completion
**              EINTR - a signal interrupted execution
**              EIO --- a PIO failure occurred
**
** Called By:   do_dnld() cu_load() do_diag()
**
** Calls To:    delay(), CAT_READ(), BUSMEM_DET(), sig_chk(), pidsig()
**
*****************************************************************************/
int
cat_wait_sram(
        struct ca       *ca,
        open_t          *openp,
        ulong           addr,
        ulong           orig_val,
        int             waittime,               /* in ticks */
        uchar           *val)
{
#define WAIT_RES                (HZ/4)          /* resolution in ticks */
        int             rc, ticks;
        ulong   bus;

        bus = CAT_MEM_ATT;
        /* CAT_READ sets ca->piorc */
        CAT_READ1(bus, addr, val);
        BUSMEM_DET(bus);                /* release access to MCI bus */
        if( ca->piorc ) {
                cat_shutdown( ca );
                ca->flags |= CATDEAD;
                return( ca->piorc );
        }
        /*
        ** this loop might wait too long, it really should be looking at
        ** the real-time clock, instead of counting on delay (and the loop)
        ** to be accurate.  But it's not critical ...
        */
        while (orig_val == *val && waittime > 0) {
                ticks = waittime > WAIT_RES ? WAIT_RES : waittime;
                waittime -= ticks;
                delay(ticks);                           /* sleep for a bit */
                bus = CAT_MEM_ATT;
                /* CAT_READ sets ca->piorc */
                CAT_READ1(bus, addr, val);
                BUSMEM_DET(bus);                /* release access to MCI bus */
                if( ca->piorc ) {
                        cat_shutdown( ca );
                        ca->flags |= CATDEAD;
                        return( ca->piorc );
                }
        }
        if (orig_val == *val)
                return(-1);                             /* timeout occurred */
        else
                return(0);
} /* cat_wait_sram() */


/*****************************************************************************
**
** NAME:        cat_user_write
**
** FUNCTION:    Copy data from a user address to a shared ram address.
**
** EXECUTION
** ENVIRONMENT: process thread
**
** Input:               channel adapter structure pointer
**                              user's buffer address
**                              channel adapter shared RAM offset
**                              data transfer mode flag (DMA or PIO)
**                              data transfer length
**
** Called By:   do_dnld() do_sram_cmd()
**
** Calls To:    xmalloc() xmfree()
**
** Returns:     0       - success
**              ENOMEM  - memory for intermediate buffer not available
**              EFAULT  - could not move data from user to kernel address space
**              EINVAL  - invalid transfer mode specified
**
*****************************************************************************/
int
cat_user_write(
        register struct ca *ca,
        uchar *uaddr,           /* user space address */
        ulong saddr,            /* shared RAM address */
        ulong mode,             /* data transfer mode */
        ulong len)              /* # of bytes to copy */
{
        uchar *data;
        ulong bus;
        int rc = 0;

        if (len == 0)
                return(0);

        switch (mode) {
        case CAT_USE_DMA:
        case CAT_USE_PIO:
        case (CAT_USE_PIO|NO_FAIL):

                data = xmalloc(len, 3, pinned_heap);
                if (data == NULL) {
                        cat_logerr(ca, ERRID_CAT_ERR4);
                        return ENOMEM;
                }
                if (rc = copyin(uaddr, data, len)) {
                        xmfree(data, pinned_heap);
                        return EFAULT;
                }
                bus = CAT_MEM_ATT;
                /* CAT_WRITE sets ca->piorc */
                CAT_WRITE(bus, saddr, data, len);
                BUSMEM_DET(bus);                /* release access to MCI bus */
                if (ca->piorc && !(mode & NO_FAIL)) {
                        cat_shutdown(ca);
                        ca->flags |= CATDEAD;
                }
                xmfree(data, pinned_heap);
                break;
        default:
                return EINVAL;
        }

        return ca->piorc;
} /* cat_user_write() */


/*****************************************************************************
**
** NAME:        cat_user_read
**
** FUNCTION:    Copy data from a shared ram address to a user address.
**
** EXECUTION ENVIRONMENT:
**
** Input:
**
** Output:
**
** Returns:
**
** Called By:   do_sram_cmd()
**
** Calls To:    xmalloc() xmfree() copyout()
**
*****************************************************************************/
int
cat_user_read(
register struct ca *ca,
        uchar *uaddr,
        ulong saddr,
        ulong mode,
        ulong len)
{
        uchar   *data;
        ulong   bus;
        int     rc = 0;

        if (len == 0)
                return(0);

        switch (mode) {
        case CAT_USE_DMA:
        case CAT_USE_PIO:
        case (CAT_USE_PIO|NO_FAIL):
                data = xmalloc(len, 3, pinned_heap);
                if (data == NULL) {
                        cat_logerr(ca, ERRID_CAT_ERR4);
                        return(ENOMEM);
                }
                bus = CAT_MEM_ATT;
                /* CAT_READ sets ca->piorc */
                CAT_READ(bus, saddr, data, len);
                BUSMEM_DET(bus);                /* release access to MCI bus */
                if( ca->piorc && !(mode & NO_FAIL)) {
                        cat_shutdown( ca );
                        ca->flags |= CATDEAD;
                }
                if (copyout(data, uaddr, len)) {
                        xmfree(data, pinned_heap);
                        return(EFAULT);
                }
                xmfree(data, pinned_heap);
                break;
        default:
                return(EINVAL);
        }
        return(ca->piorc);
} /* cat_user_read() */


/*****************************************************************************
**
** NAME:                cat_check_status
**
** FUNCTION:    Read important parts from status area of card and swap bytes
**                              if needed.  Allocate WRK.data based on xbuflen if necessary.
**
** EXECUTION
** ENVIRONMENT:
**
** NOTES:
**
** Input:
**
** Returns:
**
** Output:
**
** Called By:   catioctl() load_cu()
**
** Calls To:    CAT_READ() BUSMEMDET() CAT_MEM_ATT() letni16() letni32()
**                              cat_init_fifos() strncmp()
**
*****************************************************************************/
int
cat_check_status(struct ca *ca)
{
        ulong bus;
        int rc = 0;

        /*
        ** Use PIO for this since we have no idea what ucode is
        ** currently running on the board.
        */
        bus = CAT_MEM_ATT;
        /* CAT_READ sets ca->piorc */
        CAT_READ(bus, STATDATA, &ca->status, sizeof(STATAREA));
        BUSMEM_DET(bus);                /* release access to MCI bus */
        if (ca->piorc) {
                cat_shutdown(ca);
                ca->flags |= CATDEAD;
                return ca->piorc;
        }

        /*
        ** Swap bytes on all words and long-words
        */
        letni16(&ca->status.cutables);  /* # Control Unit Tables loaded */
        letni16(&ca->status.sbchact);   /* # subchannels currently active */
        letni16(&ca->status.xbuflen);   /* Length of transmit buffers */
        letni16(&ca->status.rbuflen);   /* Length of receive buffers */
        letni16(&ca->status.cbuflen);   /* Length of command/status buffers */
        letni16(&ca->status.xbufno);    /* # transmit data buffers */
        letni16(&ca->status.rbufno);    /* # receive data buffers */
        letni16(&ca->status.cbufno);    /* # command buffers */
        letni16(&ca->status.sbufno);    /* # response buffers */
        letni16(&ca->status.xbuffisz);  /* # slots in Transmit Buffer fifo */
        letni16(&ca->status.rbuffisz);  /* # slots in Receive Buffer fifo */
        letni16(&ca->status.cntlfisz);  /* # slots in Control fifo */
        letni16(&ca->status.respfisz);  /* # slots in Response fifo */
        letni16(&ca->status.cbuffisz);  /* # slots in Control Buffer fifo */
        letni16(&ca->status.sbuffisz);  /* # slots in Response Buffer fifo */
        letni16(&ca->status.debug);     /* adapter debug level */
        letni16(&ca->status.trace);     /* adapter trace level */
        letni16(&ca->status.maxlstsz);  /* Max # buffers in PSCAXLST */
        letni16(&ca->status.adapflgs);  /* flags field from set adap cfg cmd */
        letni16(&ca->status.xbufadds);  /* Transmit buffers added to FIFO */
        letni16(&ca->status.xbufsubs);  /* Transmit buffers deleted from FIFO */
        letni16(&ca->status.xbufwait);  /* MCA waiting for buffers */
        letni32(&ca->status.xmitbase);  /* offset/base of transmit buffer pool*/
        letni32(&ca->status.recvbase);  /* offset/base of receive buffer pool */
        letni32(&ca->status.datasize);  /* size in bytes of data buffer area */
        letni32(&ca->status.databufs);  /* offset of data buffer area */
        letni32(&ca->status.cntlbufs);  /* offset of cmd/status buffer area */
        letni32(&ca->status.xbuffifo);  /* offset of Transmit Buffer fifo */
        letni32(&ca->status.rbuffifo);  /* offset of Receive Buffer fifo */
        letni32(&ca->status.cntlfifo);  /* offset of Command fifo */
        letni32(&ca->status.respfifo);  /* offset of Status fifo */
        letni32(&ca->status.cbuffifo);  /* offset of Command Buffer fifo */
        letni32(&ca->status.sbuffifo);  /* offset of Status Buffer fifo */
        letni32(&ca->status.tracdata);  /* offset of Trace Data Area */

        if (ca->status.ready != 1) {
                /*
                ** functional ucode not running
                */
                ca->flags &= ~CATFUNC;
                return 0;
        }

        if ((ca->flags & CATFUNC) == 0) {
                cat_init_fifos(ca);     /* init fifos when first recognized */
                ca->flags |= CATFUNC;
        }
        return 0;
} /* cat_check_status() */


/*****************************************************************************
**
** NAME:                cat_init_fifos
**
** FUNCTION:    Initialize the fifos used for this adapter.
**
** EXECUTION
** ENVIRONMENT:
**
** NOTES:
**
** Input:
**
** Returns:
**
** Output:
**
** Called By:   check_status
**
** Calls To:    ENABLE() DISABLE()
**
*****************************************************************************/
void
cat_init_fifos(struct ca *ca)
{
        int     spl;

        /*
        ** Initialize the control structure for the cmd fifo.  This
        ** fifo contains all commands.  We write to it and have the tail.
        */
        DISABLE_INTERRUPTS(spl);
        ca->cmd_f.fifo          = ca->status.cntlfifo;          /* set address of fifo */
        ca->cmd_f.fifosize      = ca->status.cntlfisz;          /* set size of fifo */
        ca->cmd_f.nextslot      = 0;                                            /* tail at top of fifo */

        /*
        ** Initialize the control structure for the status fifo.  This
        ** fifo contains acknowledgements, data available announcements,
        ** and debug dumps.  We read from it and have the head.
        */
        ca->stat_f.fifo         = ca->status.respfifo;          /* set address of fifo */
        ca->stat_f.fifosize     = ca->status.respfisz;          /* set size of fifo */
        ca->stat_f.nextslot     = 0;                                            /* head at top of fifo */

        /*
        ** Initialize the control structure for the dump free buffer fifo.
        ** This fifo contains free buffers for debug dump data.  We write
        ** to it to return used buffers and have the tail.
        */
        ca->dfb_f.fifo          = ca->status.sbuffifo;          /* set address of fifo */
        ca->dfb_f.fifosize      = ca->status.sbuffisz;          /* set size of fifo */
        ca->dfb_f.nextslot      = 0;                                            /* tail at top of fifo */

        /*
        ** Initialize the control structure for the cmd free buffer fifo.
        ** This fifo contains free buffers for small command data.  It is
        ** not used for send/receive data.  We read from it and have the head.
        */
        ca->cfb_f.fifo          = ca->status.cbuffifo;          /* set address of fifo */
        ca->cfb_f.fifosize      = ca->status.cbuffisz;          /* set size of fifo */
        ca->cfb_f.nextslot      = 0;                                            /* tail at top of fifo */
        ENABLE_INTERRUPTS(spl);
} /* cat_init_fifos() */


/****************************************************************************
**
** NAME:        cat_get_stat
**
** FUNCTION:    Gets a notify element from the stat queue on the PCA
**
** EXECUTION ENVIRONMENT: interrupt only (off-level)
**
** NOTES:
**      Retrieve a single entry from the status fifo.
**      Swap necessary bytes, copy element into structure.
**      Return non-zero if no status available.
**      Set the mbuf_low_timer and return non-zero if not enough mbufs.
**      Return 0 when status retrieved.
**
** Input:
**      pointer to channel adapter structure
**      pointer to stat queue element
** Output:
**      0  -- good return
**      -1 -- stat queue empty
** Called By:
**      clean_queue()
** Calls To:
**      CAT_READ CAT_WRITE, letni32, letni16, DISABLE, ENABLE
**
*****************************************************************************/
int
cat_get_stat(
        struct ca *ca,
        CTLFIFO *value)                         /* where to put the value obtained */
{
        ulong slot_addr;
        ulong valid;
        ulong bus_addr;
        int   spl;
CATDEBUG(("cat_get_stat()\n"));

        slot_addr = ca->stat_f.fifo + ca->stat_f.nextslot * sizeof(CTLFIFO);

        DISABLE_INTERRUPTS(spl);
        bus_addr = CAT_MEM_ATT;
        CAT_READ(bus_addr, slot_addr, value, sizeof(CTLFIFO));
        if( ca->piorc ) {
                BUSMEM_DET(bus_addr);           /* release access to MCI bus */
                ENABLE_INTERRUPTS(spl);
                cat_shutdown( ca );
                ca->flags |= CATDEAD;
                return( ca->piorc );
        }
        valid = *(ulong *)((uchar *)value+12);
        letni32(&valid);                        /* Byte-swap */

        if ((valid & 0x01) == 0) {              /* if empty*/
                BUSMEM_DET(bus_addr);           /* release access to MCI bus */
                ENABLE_INTERRUPTS(spl);
                return( -1 );
        }

        valid &= ~1;
        letni32(&valid);

        CAT_WRITE(bus_addr, slot_addr + 12, &valid, sizeof(ulong));
        if( ca->piorc ) {                          /* CAT_WRITE() sets ca->piorc */
                BUSMEM_DET(bus_addr);           /* release access to MCI bus */
                ENABLE_INTERRUPTS(spl);
                cat_shutdown( ca );
                ca->flags |= CATDEAD;
                return( ca->piorc );
        }
        BUSMEM_DET(bus_addr);                   /* release access to MCI bus */
        ENABLE_INTERRUPTS(spl);

        if (++ca->stat_f.nextslot >= ca->stat_f.fifosize)
                ca->stat_f.nextslot = 0;        /* Increment and wrap fifo's slot */

        letni32(&value->buffer);
        letni16(&value->length);
        value->buffer &= ~0x01;                 /* reset valid bit in value */

        return(0);
} /* cat_get_stat() */



/*****************************************************************************
**
** NAME:                cat_put_cmd
**
** FUNCTION:    writes a command block to the pca command fifo
**
** EXECUTION
** ENVIRONMENT:  process and interrupt
**
** NOTES:
**                              Add a single entry to the cmd fifo.
**                              swap necessary bytes, copy structure into cmd fifo.
**                              If FIFO is full return -1.
**
** Input:               pointer to channel adapter structure
**                              pointer to command structure
**
** Returns:
**                              0  -- good return
**                              EAGAIN -- no available fifo entries
**                              EIO -- PIO error
**
** Called By:   transmit_mgr, do_setadap, do_diag, do_cutable, do_setsub,
**                              do_stopsub, do_startsub, dma_dequeue
**
** Calls To:    letni32, letni16, ENABLE, DISABLE, CAT_READ, BUSMEM_DET
**
*****************************************************************************/
int
cat_put_cmd(
        struct ca *ca,
        CTLFIFO *value)                 /* points to data to be copied into fifo */
{
        int slot;
        int rc;
        int old_level;
        ulong addr_valid;
        ulong bus;
        uchar valid;
	   unsigned int *cmdptr;
	   CTLFIFO  tmpcmd; 
        extern int delaytime;

        letni32(&value->buffer);
        letni16(&value->length);

        /*
        ** Set up indices to fifo.
        */
        DISABLE_INTERRUPTS(old_level);
        slot = ca->cmd_f.nextslot;
        if (++(ca->cmd_f.nextslot) >= ca->cmd_f.fifosize) {
                ca->cmd_f.nextslot = 0;
        }

	   /* Trace the command being issued */

	   /* this gets a little tricky since the length is byte swapped */
        /* we copy the cmd localy and then reswap the length          */ 
	   bcopy(value,&tmpcmd,sizeof(CTLFIFO));
        cmdptr = (unsigned int *)&tmpcmd;
	   letni16(&tmpcmd.length);

        DDHKWD5(HKWD_DD_CATDD, PSCA_CMD, 0, ca->dev,
                *(cmdptr),*(cmdptr+1),*(cmdptr+2),*(cmdptr+3));

	   /*
        ** Wait until slot available to write.
        */
        addr_valid = ca->cmd_f.fifo + slot * sizeof(CTLFIFO) + 12;
        bus = CAT_MEM_ATT;
        /* CAT_READ1 sets ca->piorc */
        CAT_READ1(bus, addr_valid, &valid);
        if (ca->piorc) {
                BUSMEM_DET(bus);                /* release access to MCI bus */
                ENABLE_INTERRUPTS(old_level);
                return EIO;
        }

        if (valid & 0x01) {                     /* if not available */
                BUSMEM_DET(bus);                /* release access to MCI bus */
                ENABLE_INTERRUPTS(old_level);
                return EAGAIN;
        }

        /*
        ** Add current slot and make it valid.
        */
        /* CAT_WRITE1 sets ca->piorc */
        CAT_WRITE(bus,
                ca->cmd_f.fifo + slot * sizeof(CTLFIFO),
                (char *)value,
                sizeof(CTLFIFO));
        if (ca->piorc) {
                BUSMEM_DET(bus);                /* release access to MCI bus */
                ENABLE_INTERRUPTS(old_level);
                return EIO;
        }

        valid = *((char *)value + 12) | 0x01;
        /* CAT_WRITE1 sets ca->piorc */
        CAT_WRITE1(bus, addr_valid, &valid) ;
        if (ca->piorc) {
                BUSMEM_DET(bus);                /* release access to MCI bus */
                ENABLE_INTERRUPTS(old_level);
                return EIO;
        }

        BUSMEM_DET(bus);                /* release access to MCI bus */
        ENABLE_INTERRUPTS(old_level);

        return 0;
} /* cat_put_cmd() */


/*****************************************************************************
**
** NAME:                cat_get_cfb
**
** FUNCTION:    get a control buffer from the PCA
**
** EXECUTION
** ENVIRONMENT: process and interrupt
**
** NOTES:
**                              Retrieve a single entry from the ctrl free buffer fifo.
**                              Swap necessary bytes, copy element into structure.
**
** Input:               pointer to channel adapter structure
**                              pointer to fifo element
**
** Returns:             0  -- got control buffer
**                              -1 -- no control buffer available
**
** Called By:   transmit_mgr, do_setsub
**
** Calls To:    DISABLE, ENABLE, CAT_READ, CAT_MEM_ATT, BUS_MEM_ATT
**
*****************************************************************************/
int
cat_get_cfb(
        register struct ca *ca,
        BUFFIFO *value)         /* where to put the value obtained */
{
        ulong addr;
        ulong bus;
        int rc;
        int slot;
        int old_level;
        int spl;
        uchar valid;

        /*
        ** Set up indices to fifo.
        */
        DISABLE_INTERRUPTS(old_level);
        slot = ca->cfb_f.nextslot++;
        if (ca->cfb_f.nextslot >= ca->cfb_f.fifosize)
                ca->cfb_f.nextslot = 0;

        /*
        ** Wait until slot is valid to read.
        */
        addr = ca->cfb_f.fifo + slot * sizeof(BUFFIFO);
        bus = CAT_MEM_ATT;
        CAT_READ1(bus, addr, &valid) ;
        /* CAT_READ1 sets ca->piorc */
        if (ca->piorc) {
                BUSMEM_DET(bus);                        /* release access to MCI bus */
                ENABLE_INTERRUPTS(old_level);
                cat_shutdown(ca);
                ca->flags |= CATDEAD;
                return ca->piorc;
        }
        if (valid & 0x01 != 0x01) {             /* if not valid */
                BUSMEM_DET(bus);                        /* release access to MCI bus */
                ENABLE_INTERRUPTS(old_level);
                cat_logerr(ca, ERRID_CAT_ERR7);
                return -1;
        }

        /*
        ** read current slot and make it invalid
        */
        /* CAT_READ sets ca->piorc */
        CAT_READ(bus, addr, (char *) value, sizeof(BUFFIFO));
        if (ca->piorc) {
                BUSMEM_DET(bus);                        /* release access to MCI bus */
                ENABLE_INTERRUPTS(old_level);
                cat_shutdown(ca);
                ca->flags |= CATDEAD;
                return ca->piorc;
        }
        valid = 0;
        /* CAT_WRITE1 sets ca->piorc */
        CAT_WRITE1(bus, addr, &valid);          /* mark invalid */
        if (ca->piorc) {
                BUSMEM_DET(bus);                        /* release access to MCI bus */
                ENABLE_INTERRUPTS(old_level);
                cat_shutdown(ca);
                ca->flags |= CATDEAD;
                return ca->piorc;
        }
        BUSMEM_DET(bus);                /* release access to MCI bus */
        ENABLE_INTERRUPTS(old_level);
        *((char *)value) &= ~0x01;              /* reset valid bit in value */
        letni32(&value->buffer);                /* swap bytes */
        return 0;
} /* cat_get_cfb() */


/*****************************************************************************
**
** NAME:                cat_put_rfb
**
** FUNCTION:    put a receive buffer back on the pca fifo
**
** EXECUTION
** ENVIRONMENT: interrupt
**
** Input:               pointer to channel adapter
**                              pointer to receive fifo
**
** Output:              0  -- good return
**                              -1 --
**
** Called by:   clean_queue
**
** calls to:    DISABLE, ENABLE, CAT_MEMATT, BUSMEM_DET, CAT_READ
**
*****************************************************************************/
int
cat_put_rfb(
        struct ca       *ca,
        ulong           *value)         /* data to be copied to fifo */
{
        ulong   status, bus;
        int             rc;
        int             x;
        int     old_level;

CATDEBUG(("cat_put_rfb()\n"));
        letni32(value);
        /*
        ** wait until slot available to write
        */
        DISABLE_INTERRUPTS(old_level);
        bus = CAT_MEM_ATT;
        /* CAT_READ sets ca->piorc */
        CAT_READ(bus, RBUFRESV, &status, sizeof(status));
        if( ca->piorc ) {
                BUSMEM_DET(bus);                /* release access to MCI bus */
                ENABLE_INTERRUPTS(old_level);
                cat_shutdown( ca );
                ca->flags |= CATDEAD;
                return(ca->piorc);
        }
        letni32(&status);
        /*
        ** this shouldn't happen ????
        */
        if (status & RBUFRFUL)          /* if not available */
        {
           BUSMEM_DET(bus);
           ENABLE_INTERRUPTS(old_level);
           return(-1);
        }

        /* CAT_WRITE sets ca->piorc */
        CAT_WRITE(bus, RBUFDATA, (char *)value, sizeof(ulong));
        if( ca->piorc ) {
                BUSMEM_DET(bus);                /* release access to MCI bus */
                ENABLE_INTERRUPTS(old_level);
                cat_shutdown( ca );
                ca->flags |= CATDEAD;
                return(ca->piorc);
        }
        BUSMEM_DET(bus);                /* release access to MCI bus */
        ENABLE_INTERRUPTS(old_level);
CATDEBUG(("good cat_ret_buf...\n"));
        return(0);
} /* cat_put_rfb() */


/*****************************************************************************
**
** NAME:                cat_get_sfb
**
** FUNCTION:    get a single entry from the send free buffer fifo
**
** EXECUTION
** ENVIRONMENT: process and interrupt
**
** Input:               pointer to channel adapter
**                              pointer to free buffer
**
** Output:              0  -- goo dreturn
**                              -1 -- no buffer available
**
** Called By:   reserve_pca_xmit
**
** Calls To:    CAT_MEMATT, BUS_MEM_DET, DISABLE, ENABLE
**
*****************************************************************************/
int
cat_get_sfb(
        struct ca       *ca,
        BUFFIFO         *value)         /* where to put the value obtained */
{
        ulong   status, bus;
        int             rc, spl;

        /*
        ** See if slot is vailable to write.
        */
        DISABLE_INTERRUPTS(spl);
        bus = CAT_MEM_ATT;
        /* CAT_READ sets ca->piorc */
        CAT_READ(bus, XBUFRESV, &status, sizeof(status));
        if( ca->piorc ) {
                BUSMEM_DET(bus);                        /* release access to MCI bus */
                ENABLE_INTERRUPTS(spl);
                cat_shutdown( ca );
                ca->flags |= CATDEAD;
                return(ca->piorc );
        }
        letni32(&status);
        if (status & XBUFREMP) {                /* if not available */
                BUSMEM_DET(bus);                        /* release access to MCI bus */
                ENABLE_INTERRUPTS(spl);
                return(-1);
        }

        /*
        ** read current slot
        */
        /* CAT_READ sets ca->piorc */
        CAT_READ(bus, XBUFDATA, (char *) value, sizeof(long));
        if( ca->piorc ) {
                BUSMEM_DET(bus);                        /* release access to MCI bus */
                ENABLE_INTERRUPTS(spl);
                cat_shutdown( ca );
                ca->flags |= CATDEAD;
                return(ca->piorc );
        }
        if( ca->xmit_bufs_avail > 0 )
                ca->xmit_bufs_avail--;
        BUSMEM_DET(bus);                                /* release access to MCI bus */
        ENABLE_INTERRUPTS(spl);
        letni32(value);                                 /* swap bytes */
        return(0);
}


/*****************************************************************************
**
** NAME:        sfb_avail
**
** FUNCTION:    See if there is a free status element available.
**
** EXECUTION ENVIRONMENT:       process and interrupt
**
** RETURNS:
**
*****************************************************************************/
int
sfb_avail( struct ca *ca )
{
        ulong   status, bus;
        int             rc, spl;

        /*
        ** See if slot is vailable to write.
        */
        DISABLE_INTERRUPTS(spl);
        bus = CAT_MEM_ATT;
        /* CAT_READ sets ca->piorc */
        CAT_READ(bus, XBUFSTAT, &status, sizeof(status));
        if (ca->piorc) {
                BUSMEM_DET(bus);
                ENABLE_INTERRUPTS(spl);
                cat_shutdown(ca);
                ca->flags |= CATDEAD;
                return ca->piorc;
        }
        BUSMEM_DET(bus);                /* release access to MCI bus */
        ENABLE_INTERRUPTS(spl);
        letni32(&status);
        if (status & XBUFREMP)          /* if no reservations available */
                return -1;
        return 0;
}



/*****************************************************************************
**
** NAME:                cat_ret_buf
**
** FUNCTION:    returns a pca buffer to the fifo
**
** EXECUTION
** ENVIRONMENT: process and interrupt
**
** Input:               pointer to channel adapter
**                              pointer to pca buffer
**
** Output:              0  -- buffer returned
**                              -1 -- not command buffer available
**
** Called By:   return_xmitbufs
**
** Calls To:    cat_put_cmd
**
*****************************************************************************/
int
cat_ret_buf(
        struct ca       *ca,
        ulong           buf_addr,
        int                     buf_type)               /* 0 = SFB, 1 = CFB with SFB list, 2 = CFB */
{
        int             spl;
        cmd_t   cmd;

        bzero(&cmd,sizeof(CTLFIFO)); /* d51376 */
        /*
        ** Setup command structure to return the unused xmit buf
        */
        cmd.command              = PSCARBUF;
        cmd.cmdmod               = 0;
        cmd.correl               = 0;
        cmd.length               = 0;
        cmd.buffer               = buf_addr;
        cmd.data[0]              = 0;
#ifdef notdef
        /*
        ** This will need to be used if we ever use the SFBLIST_TYPE.
        */
        if( buf_type == SFBLIST_TYPE )
                cmd.data[0] = nbufs;
#endif
        cmd.data[1]              = buf_type;

        if (cat_put_cmd(ca, &cmd)) {
                return(EIO);
        }
        DISABLE_INTERRUPTS( spl );
        if( ca->xmit_bufs_avail < ca->status.xbufno )
                ca->xmit_bufs_avail++;
        ENABLE_INTERRUPTS( spl );
        return(0);
}


/*****************************************************************************
**
** NAME:                cat_shutdown
**
** FUNCTION:    Shuts down the PSCA adapter after a PIO error.
**
** EXECUTION
** ENVIRONMENT: process and interrupt
**
** Input:               pointer to channel adapter
**
** Output:              None
**
** Called By:   CAT_WRITE1(), CAT_WRITE(), CAT_READ1(), CAT_READ(),
**                              cat_term_dev()
**
** Calls To:    CAT_POS_WRITE(), notify_all_opens(),
**                              pidsig(), i_clear(), d_clear(), w_stop(), w_clear()
**
*****************************************************************************/
void
cat_shutdown(register struct ca *ca)
{
        xmit_elem_t *xmitp, *tp;
        dma_req_t *dmap;
        cio_read_ext_t rd_ext;
        int i;
        int spl;
        open_t *openp;
        recv_elem_t *recvp;
        cmd_t *notify;

        if (getpid() == -1)
                return;
        /*
        ** Turn off interrupts until the DMA channel and
        ** interrupt handler are cleared.
        */
        DISABLE_INTERRUPTS(spl);

        /*
        ** Free the DMA channel and remove the interrupt
        ** and watchdog timer handler.
        */
        if (ca->flags & CAT_TIMER_ON)
                tstop(ca->resource_timer);
        if (ca->flags & CATRTINSTALL)
                tfree(ca->resource_timer);
        ca->flags &= ~CATRTINSTALL;
        w_stop(&ca->watch);
        if (ca->flags & CATWDINSTALL)
                w_clear(&ca->watch);
        ca->flags &= ~(CATWDTACT|CATWDINSTALL);
        if (ca->flags & CATDMACHAN)
                d_clear(ca->dma_channel);
        ca->flags &= ~CATDMACHAN;
        if (ca->flags & CATIINSTALL)
                i_clear(&ca->caih_struct);

        /* return saved buffer */
        if (ca->saved_notify.buffer) {
                cat_put_rfb(ca, &(ca->saved_notify.buffer));
/*d50453*/      bzero(&ca->saved_notify,sizeof(CTLFIFO));
        }

        ca->flags &= ~CATIINSTALL;
        ca->flags &= ~CATXMITOWED;

        ENABLE_INTERRUPTS(spl);

        /*
        ** Now, the adapter interrupts are enabled.
        ** Send a status block to all processes that have opened
        ** the adapter.
        */
        if (ca->num_opens > 0) {
                /*
                ** The only way to get here with existing opens
                ** is via a PIO or DMA failure or out of mbufs.
                ** In that case we mark the adapter dead, reset
                ** and shut it down.
                */
                shutdown_adapter(ca);
                notify_all_opens(ca, CIO_HARD_FAIL);

                /*
                ** Wakeup all waiting processes
                */
                for (i=0; i<ca->num_opens; i++) {
                        openp = &ca->open_lst[i];
                        if ((openp->op_flags&OP_OPENED)
                                && (openp->op_flags&XMIT_OWED)) {
                                (*(openp->op_xmit_fn))(openp->op_open_id);
                                openp->op_flags &= ~XMIT_OWED;
                        }
                        DISABLE_INTERRUPTS(spl);
                        if (openp->op_rcv_event != EVENT_NULL)
                                e_wakeup(&openp->op_rcv_event);
                        if (openp->op_select & POLLIN)
                                selnotify((int)ca->dev, openp->op_chan, POLLIN);
                        if (openp->op_select & POLLOUT)
                                selnotify((int)ca->dev, openp->op_chan, POLLOUT);
                        if (openp->op_select & POLLPRI)
                                selnotify((int)ca->dev, openp->op_chan, POLLPRI);
                        ENABLE_INTERRUPTS(spl);
                        (void)pidsig(ca->open_lst[i].op_pid,SIGURG);
                }
                /*
                ** Wakeup anyone waiting on elements
                */
                DISABLE_INTERRUPTS(spl);
                if (ca->xmitbuf_event != EVENT_NULL)
                        e_wakeup(&ca->xmitbuf_event);
                if (ca->dmabuf_event != EVENT_NULL)
                        e_wakeup(&ca->dmabuf_event);
                if (ca->pcabuf_event != EVENT_NULL)
                        e_wakeup(&ca->pcabuf_event);
                ENABLE_INTERRUPTS(spl);
        }
        return;
} /* cat_shutdown() */
