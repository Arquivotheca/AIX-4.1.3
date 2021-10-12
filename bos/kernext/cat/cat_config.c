static char sccsid[] = "@(#)47	1.26  src/bos/kernext/cat/cat_config.c, sysxcat, bos411, 9428A410j 7/6/94 15:52:57";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS:   cat_get_vpd, cat_init_dev, shutdown_adapter, cat_set_pos,
 *              cat_term_dev, catalloc, catconfig, catfree, catget
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

#define FNUM 3
#include <sys/device.h>
#include <sys/dma.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/comio.h>
#include <sys/malloc.h>
#include <sys/devinfo.h>
#include <sys/adspace.h>
#include <sys/uio.h>
#include <sys/iocc.h>
#include <sys/pin.h>
#include <sys/intr.h>
#include <sys/lockl.h>
#include <sys/sleep.h>
#include <sys/except.h>
#include <errno.h>

#include "catdd.h"

/*
** The following macro gives a 0 based index to the base address map
** by masking and shifting the appropriate bits of the POS 2 value
** (This might have been done with a union and some bit fields.)
*/
#define INT_BITS(x)     ((x) & 0x03)

/*************************************************************************
** The following table is extracted from the PSCA Hardware Specification
** in the section describing the functionality of the POS registers.
** It describes the mapping between the bits in the POS register and the
** Starting shared memory address (base address).
*************************************************************************/
unsigned long cat_addr_map[] = {
                        /* Bit  7 6 5 4                                 */
        0x00800000,     /*      0 0 0 0  Early cards use this addr      */
        0x00f40000,     /*      0 0 0 1  256K slots at high end of      */
        0x00f80000,     /*      0 0 1 0      24 bit address space       */
        0x00fc0000,     /*      0 0 1 1                                 */
        0x01000000,     /*      0 1 0 0  1 Meg slots starting at 16M    */
        0x01100000,     /*      0 1 0 1                                 */
        0x01200000,     /*      0 1 1 0                                 */
        0x01300000,     /*      0 1 1 1                                 */
        0x01400000,     /*      1 0 0 0                                 */
        0x01500000,     /*      1 0 0 1                                 */
        0x01600000,     /*      1 0 1 0                                 */
        0x01700000,     /*      1 0 1 1                                 */
        0xffc00000,     /*      1 1 0 0  1 Meg slots at high end of     */
        0xffd00000,     /*      1 1 0 1      32 bit address space       */
        0xffe00000,     /*      1 1 1 0                                 */
        0x00000000      /*      1 1 1 1  Board disabled                 */
};


/*************************************************************************
** The following table is extracted from the PSCA Hardware Specification
** in the section describing the functionality of the POS registers.
** It describes the mapping between the bits in the POS register and the
** interrupt level.
*************************************************************************/
int cat_int_map[] = {
                /* Bit  2 1 0 */
        4,      /*      0 0 0 */
        5,      /*      0 0 1 */
        6,      /*      0 1 0 */
        7,      /*      0 1 1 */
        9,      /*      1 0 0 */
        10,     /*      1 0 1 */
        11,     /*      1 1 0 */
        14      /*      1 1 1 */
};


static int cat_get_vpd(struct ca *ca);
int cat_set_pos(struct ca *ca);
int cat_init_dev(struct ca *ca);
int cat_term_dev(struct ca *ca);

/*
** Externs
*/
struct ca *caheader = NULL;                    /* Head of ca Linked List */
int catopened = 0;                             /* driver opened count */
extern int nodev(void);
extern cdt_t catcdt;

int caglobal_lock = LOCK_AVAIL;         /* Global Lock Variable */
int pio_rc;                             /* return code for PIO ops */
int global_num_adapters = 0;

#ifdef DEBUG
struct ft func_trace;                   /* Function trace table */
int cadebug = 0;                        /* Default debug level */
#endif /* DEBUG */


/*****************************************************************************
** NAME:        catconfig
**
** FUNCTION:    Either initializes a given adapter, terminates a given
**              adapter, or queries an adapter for its VPD information.
**
**              CFG_INIT:
**                      Allocate a per-adapter structure.
**                      Read in DDS from user space
**                      Verify adapter CARD ID
**                      Set adapter values on card
**                      Add functions to the devsw table
**                      Register cat dump routine
**              CFG_TERM:
**                      Verify that no process has an active open()
**                      Free the adapter structure memory
**              CFG_QVPD:
**                      Move the VPD information to user space
**
** EXECUTION ENVIRONMENT: Can be called from the process environment only.
**
** NOTES:
**    Input:
**              A device number, a command (config, unconfig, or
**              query VPD), a uio structure describing the Device
**              Dependent Information.
**    Output:
**              Success status
**    Called From:
**              sysconfig()
**    Calls:
**              lockl() catget() unlockl() catalloc() uiomove()
**              catfree() devswadd()
**              devswdel() dmp_add() cat_get_vpd()
**
** RETURNS:     0 - Success
**
**              EINTR - get of global lock was interrupted
**              EEXIST - Adapter structure (ca) doesn't exist
**              ENOMEM - catalloc() couldn't allocate a 'ca' structure
**              ENOMEM, EIO, ENOSPC, EFAULT -1 - uiomove() failed
**              EEXIST, ENOMEM, EINVAL - devswadd() failed
**              (-1) - dmp_add() failed
**              EEXIST - catconfig(CFG_TERM) couldn't find 'ca' structure
**              EBUSY - catconfig(CFG_TERM) found an open to the device still
**              ENOMEM - catfree() could not free memory
**              EACCES - catconfig(CFG_QVPD) device not initialized
**              EINVAL - invalid command to catconfig()
*****************************************************************************/
int
catconfig(
        dev_t devno,                    /* major-minor device number */
        int cmd,                        /* command */
        struct uio *uiop)               /* uio struct containing DDS */
{
        struct ca *ca;
        struct devsw cat_dsw;
        int rc=0;
        int i;

        DDHKWD2(HKWD_DD_CATDD,DD_ENTRY_CONFIG,0,devno,cmd);
        /*
        ** Lock Global Lock, waits for lock or returns early on signal
        */
        if (lockl(&caglobal_lock, LOCK_SIGRET) == LOCK_SIG) {
                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,EINTR,devno);
                return EINTR;
        }

        switch (cmd) {
        case CFG_INIT:  /* Add a device (adapter) */
                /*
                ** Pin all of the driver code.
                */
                if (rc = pincode(catconfig)) {
                        unlockl(&caglobal_lock);
                        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,rc,devno);
                        return rc;
                }

                if (caheader == NULL) { /* First adapter to be added */
                        /*
                        ** Add our driver to the devsw[] table.
                        */
                        cat_dsw.d_open           = catopen;
                        cat_dsw.d_close          = catclose;
                        cat_dsw.d_read           = catread;
                        cat_dsw.d_write          = catwrite;
                        cat_dsw.d_ioctl          = catioctl;
                        cat_dsw.d_strategy       = nodev;
                        cat_dsw.d_ttys           = (struct tty *)NULL;
                        cat_dsw.d_select         = catselect;
                        cat_dsw.d_config         = catconfig;
                        cat_dsw.d_print          = nodev;
                        cat_dsw.d_dump           = nodev;
                        cat_dsw.d_mpx            = catmpx;
                        cat_dsw.d_revoke         = nodev;
                        cat_dsw.d_dsdptr         = NULL;
                        cat_dsw.d_selptr         = NULL;
                        cat_dsw.d_opts           = 0;
                        if (rc = devswadd(devno, &cat_dsw)) {
                                unpincode(catconfig);
                                unlockl(&caglobal_lock);
                                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,rc,devno);
                                return rc;
                        }
                        /*
                        ** We leave the code pinned for the rest of this case
                        ** because cat_set_pos calls pio_assist().
                        */
                } /* end of first-time stuff... */

                /*
                ** Make sure this device wasn't already configured.
                */
                if (ca = catget(minor(devno))) {
                        unlockl(&caglobal_lock);
                        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,EEXIST,devno);
                        return EEXIST;
                }

                /*
                ** make sure that they don't try to exceed the
                ** maximum number of adapters
                */
                if (global_num_adapters > MAX_ADAPTERS) {
                        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,ENXIO,devno);
		        unlockl(&caglobal_lock);
                        return ENXIO;
                }
                global_num_adapters++;

                /*
                ** Allocate a ca structure.
                */
                if ((ca = catalloc()) == NULL) {
                        rc = ENOMEM;
                        goto release_drvr;
                }
                ca->adap_lock = LOCK_AVAIL;
                ca->locking_adap_lock = LOCK_AVAIL;
                ca->in_kernel = 0;


                /*
                ** Copy cfg mthd data into device dependent structure.
                */
                if (rc = uiomove(&ca->caddp,sizeof(ca->caddp),UIO_WRITE,uiop)) {
                        catfree(ca);
                        goto release_drvr;
                }

                ca->dev = devno;
                if (rc = cat_set_pos(ca)) {
                        catfree(ca);
                        goto release_drvr;
                }

                /*
                ** Always copy the VPD information into the ca structure,
                ** because catioctl() assumes it will be there.
                */
                if (rc = cat_get_vpd(ca)) {
                        catfree(ca);
                        goto release_drvr;
                }

                ca->flags |= CATINIT;
                if (catopened == 0)
                        unpincode(catconfig);
                break;

        case CFG_TERM:
                /*
                ** Get the ca structure.
                */
                if ((ca = catget(minor(devno))) == NULL) {
                        unlockl(&caglobal_lock);
                        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,EACCES,devno);
                        return EACCES;
                }

                /*
                ** Don't terminate if there are outstanding open()s.
                */
                if (ca->num_opens) {
                        unlockl(&caglobal_lock);
                        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,EBUSY,devno);
                        return EBUSY;
                }

                /*
                ** Make sure the driver is pinned
                */
                if (catopened == 0)
                        pincode(catconfig);

                /*
                ** Shutdown the adapter
                */
                shutdown_adapter(ca);

                /*
                ** Free system resources.
                */
                global_num_adapters--;
                catfree(ca);
                if (caheader == NULL)
                        rc = devswdel(devno);
                if (caheader == NULL || catopened == 0)
                        rc = unpincode(catconfig);
                break;

        case CFG_QVPD:
                if ((ca = catget(minor(devno))) == NULL) {
                        unlockl(&caglobal_lock);
                        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,EACCES,devno);
                        return EACCES;          /* No such device */
                }

                if (rc = cat_get_vpd(ca)) {
                        unlockl(&caglobal_lock);
                        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,rc,devno);
                        return rc;
                }

                if (rc = uiomove(ca->vpd, ca->vpd_length, UIO_READ, uiop)) {
                        unlockl(&caglobal_lock);
                        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,rc,devno);
                        return rc;
                }
                break;

        default:
                unlockl(&caglobal_lock);
                DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,EINVAL,devno);
                return EINVAL;
        }

        unlockl(&caglobal_lock);        /* Unlock Global Lock Variable */
        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,0,devno);
        return 0;

release_drvr:
        if (caheader == NULL)
                rc = devswdel(devno);
        if (caheader == NULL || catopened == 0)
                rc = unpincode(catconfig);
        unlockl(&caglobal_lock);
        DDHKWD1(HKWD_DD_CATDD,DD_EXIT_CONFIG,rc,devno);
        return rc;
} /* catconfig() */


/*****************************************************************************
** NAME:        shutdown_adapter
**
** FUNCTION:    This code resets and disables the adapter
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**
** RETURNS:     nothing (void)
**
*****************************************************************************/
void
shutdown_adapter(struct ca *ca)
{
        uchar reg = 0;
        uchar pos4;
        ulong iocc;
        ulong mem;
        int spl;

        /*
        ** Shutdown the adapter.
        ** NOTE: This will only work if POS Registers can
        ** be accessed.
        */
        reg = 0x00;             /* value for OPERLVL */
        DISABLE_INTERRUPTS(spl);
        iocc = CAT_IOCC_ATT;
        CAT_POS_READ(iocc, 4, &pos4);                   /* get value of POS reg 4 */
        CAT_POS_WRITE(iocc, 4, pos4 | 0x04);    /* Turn on reset */
        mem = CAT_MEM_ATT;
        CAT_WRITE1(mem, OPERLVL, &reg);                 /* Clear OPERLVL */
        CAT_WRITE1(mem, READY, &reg);                   /* Clear READY */
        BUSMEM_DET(mem);                                                /* change back to iocc */
        ENABLE_INTERRUPTS(spl);
        delay(50);                                                              /* hold reset for a bit */
        DISABLE_INTERRUPTS(spl);
        CAT_POS_WRITE(iocc, 4, pos4);                   /* Turn off reset */
        IOCC_DET(iocc);                                                 /* release access to MCI bus */
        ENABLE_INTERRUPTS(spl);

        /*
        ** Mark the adapter dead - RIP
        */
        ca->flags |= CATDEAD;
} /* shutdown_adapter */



/*****************************************************************************
** NAME:        cat_set_pos
**
** FUNCTION:    Verifies the CARD ID value and sets the POS register values
**              according the information in the DDS.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environent only.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              catconfig()
**      Calls:
**              pio_assist()
**
** RETURNS:     0 - Success
**              (-1) - CARD ID invalid
**
*****************************************************************************/
int
cat_set_pos(struct ca *ca)
{
        uchar reg;
        uchar reg2;
        ulong bus;
        int spl;

        /*
        ** get access to the I/O bus
        */

        DISABLE_INTERRUPTS(spl);

        bus = CAT_IOCC_ATT;

        /*
        ** Program the pos registers and decipher settings.
        */
        reg2 =
                (ca->caddp.addr_bits << 4)
                | (ca->caddp.parity_opts << 1); /* sync, data and addr parity */
        ca->base_addr =                 /* get base address from table */
                cat_addr_map[ca->caddp.addr_bits];
        reg2 |= 0x06;                   /* enable data parity */

        CAT_POS_WRITE(bus, 2, reg2);

        reg = (ca->caddp.fair << 7)     /* fairness enable */
         | (ca->caddp.dma_lvl << 3)     /* arbitration level */
         | (ca->caddp.burst_bits);      /* BURST release time after PREMPT */
        CAT_POS_WRITE(bus, 4, reg);

        CAT_POS_READ(bus, 5, &reg);     /* read if old settings needed */
        reg = (reg & 0xe0)              /* save bits 5, 6, 7 */
         | (ca->caddp.io_dma << 3)      /* card responds to I/O DMA cycles */
         | (ca->caddp.dma_enable << 4)  /* enable DMA */
         | (ca->caddp.int_bits);        /* interrupt level */
        ca->int_level =                 /* get interrupt level from table */
                cat_int_map[ca->caddp.int_bits];
        CAT_POS_WRITE(bus, 5, reg);

        reg2 |= 0x01;                   /* enable card */
        CAT_POS_WRITE(bus, 2, reg2);

        IOCC_DET(bus);                  /* release access to MCI bus */
        ENABLE_INTERRUPTS(spl);
        return 0;
} /* cat_set_pos() */


/****************************************************************************
**
** NAME: cat_get_vpd
**
** FUNCTION:
**
** EXECUTION ENVIRONMENT: process
**
** NOTES:
**
**
** RETURN:
**
*****************************************************************************/
static int
cat_get_vpd(struct ca *ca)
{
        uchar index;
        ulong bus;
        int rc = 0;
        int spl;

        DISABLE_INTERRUPTS(spl);

        if( ca->vpd[0] != 'V' ) {
                /*
                ** Setup for accessing the POS registers
                */
                bus = CAT_IOCC_ATT;

                /*
                ** Get VPD header
                */
                for (index = 0; index < 7; index++) {
                        CAT_POS_WRITE(bus, 6, index+1);
                        CAT_POS_READ(bus, 3, &ca->vpd[index]);
                }

                /*
                ** If the header seems valid, get the data
                */
                if (ca->vpd[0]=='V' && ca->vpd[1]=='P'
                        && ca->vpd[2]=='D'
                        && (ca->vpd_length =
                                ((ca->vpd[3] << 8) | ca->vpd[4]) * 2)
                        && ca->vpd_length <= CAT_MAX_VPD_LEN) {
                        for (index = 7; index < ca->vpd_length;  index++) {
                                CAT_POS_WRITE(bus, 6, index+1);
                                CAT_POS_READ(bus, 3, &ca->vpd[index]);
                                rc=0;
                        }
                } else {
                        ca->vpd_length = 0;
                        rc = ENODEV;
                }
                IOCC_DET(bus);
        }
        ENABLE_INTERRUPTS(spl);
        return rc;
} /* cat_get_vpd() */


/*****************************************************************************
** NAME:        cat_init_dev
**
** FUNCTION:    Sets the POS register values according the
**                              information in the DDS.
**                              Install the interrupt handler.
**                              Install the off-level interrupt handler.
**                              Acquire a DMA channel.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environent only.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              catconfig()
**      Calls:
**              pio_assist()
**
** RETURNS:     0  - Success
**              ~0 - Failure
**
*****************************************************************************/
int
cat_init_dev(struct ca *ca)
{
        open_t *openp;
        xmit_elem_t *xmitp;
        recv_elem_t *recvp;
        dma_req_t *dmap;
        int i;
        int j;
        int val;
        int spl;
        ulong bus;
        uchar reg;
        caddr_t p;
        char cdt_buf[8];
        struct ca *tmp_ca;

CATDEBUG(("in cat_init_dev() \n"));
        /*
        ** pin the driver code
        */
        if (catopened++ == 0) {
                pincode(catconfig);

                /*
                ** initialize component dump table
                */
                catcdt.header._cdt_magic = DMP_MAGIC;
                strncpy (catcdt.header._cdt_name, DD_NAME_STR,
                        sizeof(catcdt.header._cdt_name));
                catcdt.header._cdt_len = sizeof(catcdt.header);

                /*
                ** enable the component dump table routine
                */
                dmp_add(cat_cdt_func);

#ifdef DEBUG
                /*
                ** add the function trace to the component dump table
                */
                cat_add_cdt("func_trc", &func_trace, sizeof(func_trace));
#endif /* DEBUG */
        }

        /*
        ** add the adapter structure to the component dump table
        */
        for (i=0, tmp_ca = caheader; tmp_ca; tmp_ca = tmp_ca->next, i++) {
                if (tmp_ca == ca) {
                        break;
                }
        }
        sprintf(cdt_buf, "adap_%d  ", i);
        cat_add_cdt(cdt_buf, ca, sizeof(struct ca));

        /*
        ** Initialize the adapter structure
        */
        ca->watch.next = NULL;
        ca->watch.prev = NULL;
        ca->watch.func = (void (*)())ginny;
        ca->watch.restart = 1;
        ca->watch.count = 0;

        ca->resource_timer = NULL;

        ca->pcabuf_event = EVENT_NULL;
        ca->mbuf_event = EVENT_NULL;
        ca->mbuf_num = 0;             /*d50453new*/
        ca->ca_cmd.cmd_event = EVENT_NULL;  /* d51376 */

        ca->recv_free = NULL;

        ca->xmitbuf_event = EVENT_NULL;
        ca->xmit_free = NULL;
        ca->xmit_act = NULL;

        ca->dmabuf_event = EVENT_NULL;
        ca->dma_free = NULL;
        ca->dma_act = NULL;

        /*
        ** Allocate the receive elements
        ** The receive elements form a doubly-linked ring with a NULL next
        ** pointer terminating the ring.
        */
        i = MAX_RECV_ELEMS;
        ca->recv_cdtsize = ALIGN32(i*sizeof(recv_elem_t));
        if ((recvp = xmalloc(ca->recv_cdtsize,2,pinned_heap)) == NULL) {
                ca->recv_cdtsize = 0;
                cat_logerr(ca, ERRID_CAT_ERR4);
                cat_term_dev(ca);
                return ENOMEM;
        }
        ca->recv_cdtaddr = (caddr_t)recvp;
        bcopy("recv", cdt_buf, 4);
        cat_add_cdt(cdt_buf, recvp, ca->recv_cdtsize);
        bzero(recvp, ca->recv_cdtsize);
        recvp->rc_last = recvp;
        ca->recv_free = recvp;
        while (--i > 0) {
                recvp++;
                recvp->rc_last = ca->recv_free->rc_last;
                recvp->rc_last->rc_next = recvp;
                ca->recv_free->rc_last = recvp;
        }

        /*
        ** Allocate the transmit elements
        ** The transmit elements form a doubly-linked ring with a NULL next
        ** pointer terminating the ring.
        */
        i = ca->caddp.config_params.xmitno;
        ca->xmit_cdtsize = ALIGN32(i*sizeof(xmit_elem_t));
        if ((xmitp = xmalloc(ca->xmit_cdtsize,2,pinned_heap)) == NULL) {
                ca->xmit_cdtsize = 0;
                cat_logerr(ca, ERRID_CAT_ERR4);
                cat_term_dev(ca);
                return ENOMEM;
        }
        ca->xmit_cdtaddr = (caddr_t)xmitp;
        bcopy("xmit", cdt_buf, 4);
        cat_add_cdt(cdt_buf, xmitp, ca->xmit_cdtsize);
        bzero(xmitp, ca->xmit_cdtsize);
        xmitp->xm_last = xmitp;
        ca->xmit_free = xmitp;
        xmitp->xm_cmd.correl = i;
        while (--i > 0) {
                xmitp++;
                xmitp->xm_last = ca->xmit_free->xm_last;
                xmitp->xm_last->xm_next = xmitp;
                ca->xmit_free->xm_last = xmitp;
                xmitp->xm_cmd.correl = i;
        }

        /*
        ** Allocate the DMA elements
        ** The DMA elements form a doubly-linked ring with a NULL next
        ** pointer terminating the ring.
        */
        i = ca->dma_nfree = MAX_DMA_ELEMS;
        ca->dma_cdtsize = ALIGN32(ca->dma_nfree*sizeof(dma_req_t));
        if ((dmap = xmalloc(ca->dma_cdtsize, 2, pinned_heap)) == NULL) {
                ca->dma_cdtsize = 0;
                cat_logerr(ca, ERRID_CAT_ERR4);
                cat_term_dev(ca);
                return ENOMEM;
        }
        ca->dma_cdtaddr = (caddr_t)dmap;
        bcopy("dmas", cdt_buf, 4);
        cat_add_cdt(cdt_buf, dmap, ca->dma_cdtsize);
        bzero(dmap, ca->dma_cdtsize);
        dmap->dm_last = dmap;
        ca->dma_free = dmap;
        while( --i > 0 ) {
                dmap++;
                dmap->dm_last = ca->dma_free->dm_last;
                dmap->dm_last->dm_next = dmap;
                ca->dma_free->dm_last = dmap;
        }

        /*
        ** Allocate memory to hold a copy of the status
        ** and cmd fifos and add it to the CDT.
        */
        ca->fifos_cdtsize = CDT_FIFO_SIZE;
        if ((ca->fifos_cdt = xmalloc(ca->fifos_cdtsize, 2, pinned_heap)) == NULL) {
                ca->fifos_cdtsize = 0;
                cat_logerr(ca, ERRID_CAT_ERR4);
                cat_term_dev(ca);
                return ENOMEM;
        }
        bcopy("fifo", cdt_buf, 4);
        cat_add_cdt(cdt_buf, ca->fifos_cdt, ca->fifos_cdtsize);

        /*
        ** Install interrupt handler.
        */
        ca->base_addr                           = cat_addr_map[ca->caddp.addr_bits];
        ca->caih_struct.next            = NULL;
        ca->caih_struct.handler         = catintr;
        ca->caih_struct.bus_type        = ca->caddp.bus_type;
        ca->int_level                           = cat_int_map[ca->caddp.int_bits];
        ca->caih_struct.level           = ca->int_level;
        ca->caih_struct.flags           = 0;
        ca->caih_struct.priority        = ca->caddp.intr_priority;
        ca->caih_struct.bid                     = ca->caddp.bus_id;
        ca->caih_struct.i_count         = 0;

        if (i_init(&(ca->caih_struct)) != 0) {
                cat_logerr(ca, ERRID_CAT_ERR6);
                cat_term_dev( ca );
                return EBUSY;
        }
        ca->flags |= CATIINSTALL;       /* interrupt handler installed */

        /*
        ** Install off-level interrupt handler.
        */
        ca->ofl.next            = NULL;
        ca->ofl.handler         = cat_offlvl;
        ca->ofl.bus_type        = BUS_NONE;
        ca->ofl.flags           = 0;
        ca->ofl.level           = INT_OFFL2;
        ca->ofl.priority        = INTOFFL2;
        ca->ofl.bid                     = ca->caddp.bus_id;
        ca->ofl.i_count         = 0;

        /*
        ** Acquire a DMA channel to use
        */
        ca->dma_channel =
        d_init(ca->caddp.dma_lvl, (DMA_SLAVE|MICRO_CHANNEL_DMA),
                ca->caddp.bus_id);
        if (ca->dma_channel == DMA_FAIL) {
                cat_logerr(ca, ERRID_CAT_ERR6);
                cat_term_dev(ca);
                return EBUSY;
        }
        ca->flags |= CATDMACHAN;        /* DMA channel acquired */

        /*
        ** Install the watchdog timer handler
        */
        w_init ((struct watchdog *)(&(ca->watch)));
        ca->flags |= CATWDINSTALL;      /* watchdog handler installed */
        ca->resource_timer = (struct trb *)talloc();
        if (ca->resource_timer == NULL) {
                cat_logerr(ca, ERRID_CAT_ERR6);
                return EBUSY;
        }
        ca->resource_timer->flags &= ~(T_ABSOLUTE);
        ca->resource_timer->func = (void (*)())resource_timeout;
        ca->resource_timer->func_data = (unsigned long)ca;
        ca->resource_timer->ipri = ca->caih_struct.priority;
        ca->flags &= ~CAT_TIMER_ON;
        ca->flags |= CATRTINSTALL;

        bcopy( ca->caddp.clawset, ca->param_table, MAX_SUBCHAN);

        /*
        ** Make sure interrupt is cleared and enable the cards interrupts.
        */
        bus = CAT_MEM_ATT;
        val = 0;
        /* CAT_WRITE sets ca->piorc */
        CAT_WRITE(bus, RSTUCIRQ, &val, sizeof(val));
        if( ca->piorc ) {                  /* Reset interrupts on MCI. */
                BUSMEM_DET(bus);        /* Release access to MCI bus. */
                cat_term_dev(ca);
                return EIO;
        }
        reg = 1;
        /* CAT_WRITE1 sets ca->piorc */
        CAT_WRITE1(bus, INTMCI, &reg);  /* MCI ready for interrupts. */
        if (ca->piorc) {
                BUSMEM_DET(bus);
                cat_term_dev(ca);
                return EIO;
        }
        BUSMEM_DET(bus);                /* Release access to MCI bus. */

        return 0;
} /* cat_init_dev() */


/*****************************************************************************
** NAME:     cat_term_dev
**
** FUNCTION:    Disable the adapter's interrupts, remove the interrupt
**              handler, unpin the code, and release the DMA channel.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              catclose()
**      Calls:
**              pio_assist() i_clear() unpincode() d_clear()
**
** RETURNS:     0 - Success
**
*****************************************************************************/
int
cat_term_dev(struct ca *ca)
{
        open_t *openp;
        xmit_elem_t *xmitp;
        recv_elem_t *recvp;
        dma_req_t *dmap;
        uchar val = 0;
        ulong bus;
        int i;
        int spl;
        int j;
        subchannel_t *sc;
        char cdt_buf[8];
        struct ca *tmp_ca;

        /*
        ** Make sure interrupts are cleared and disable new interrupts.
        */
        DISABLE_INTERRUPTS(spl);
        bus = CAT_MEM_ATT;
        val = 0;
        CAT_WRITE1(bus, RSTUCIRQ, &val);        /* Reset interrupts on MCI. */
        CAT_WRITE1(bus, INTMCI, &val);          /* MCI NOT ready for int's */
        BUSMEM_DET(bus);                        /* Release access to MCI bus. */
        ENABLE_INTERRUPTS(spl);
        /*
        ** Complete DMA that is in progress.
        */
        if ((dmap = ca->dma_act) && dmap->dm_state == DM_STARTED) {
                d_complete(ca->dma_channel, dmap->dm_flags, dmap->dm_buffer,
                                dmap->dm_length, dmap->dm_xmem, NULL);
        }
		LOCK(ca->lcklvl);

        /*
        ** Shutdown the adapter and free all memory resources
        ** associated with this device.
        */
        cat_shutdown(ca);

        /*
        ** Get the adapter suffix for CDT entries.
        */
        for (i=0, tmp_ca = caheader; tmp_ca; tmp_ca = tmp_ca->next, i++) {
                if (tmp_ca == ca) {
                        break;
                }
        }
        sprintf(cdt_buf, "    _%d  ", i);

        /*
        ** Now free all of the DMA elements
        */
        bcopy("dmas", cdt_buf, 4);
        cat_del_cdt(cdt_buf, ca->dma_cdtaddr, ca->dma_cdtsize);
        KFREE(ca->dma_cdtaddr);
        ca->dma_free = ca->dma_act = NULL;
        ca->dma_cdtaddr = NULL;
        ca->dma_nfree = 0;

        /*
        ** Free any mbufs in the active receive elements
        */
        for (recvp = (recv_elem_t *)ca->recv_cdtaddr, i = 0;
                i < MAX_RECV_ELEMS; recvp++, i++) {
                /*
                ** Free any mbufs associated with this element
                */
                if (recvp->rc_mbuf_head != NULL) {
                        m_freem(recvp->rc_mbuf_head);
                        recvp->rc_mbuf_head = NULL;
                }
        }

        /*
        ** Now, free all of the receive elements
        */
        bcopy("recv", cdt_buf, 4);
        cat_del_cdt(cdt_buf, ca->recv_cdtaddr, ca->recv_cdtsize);
        KFREE(ca->recv_cdtaddr);
        ca->recv_free = NULL;
        ca->recv_cdtaddr = NULL;

        /*
        ** Free any mbufs in the active xmit elements
        */
        while (xmitp = ca->xmit_act) {
                ca->xmit_act = xmitp->xm_next;
                /*
                ** Free any mbufs associated with this element
                */
                if (xmitp->xm_mbuf != NULL
                && (xmitp->xm_open->op_mode&DKERNEL) == 0)
                        m_freem(xmitp->xm_mbuf);
        }

        /*
        ** Free the CDT copy of the adapters SRAM.
        */
        bcopy("fifo", cdt_buf, 4);
        cat_del_cdt(cdt_buf, ca->fifos_cdt, ca->fifos_cdtsize);
        KFREE(ca->fifos_cdt);
        ca->fifos_cdtsize = 0;

        /*
        ** Now, free all of the xmit elements
        */
        bcopy("xmit", cdt_buf, 4);
        cat_del_cdt(cdt_buf, ca->xmit_cdtaddr, ca->xmit_cdtsize);
        KFREE(ca->xmit_cdtaddr);
        ca->xmit_free = ca->xmit_act = NULL;
        ca->xmit_cdtaddr = NULL;

        /*
        ** Free all the subchannels and links
        */
        for (i=0; i<MAX_SUBCHAN; i++) {
                if ((sc= ca->sc[i]) != NULL) {
                        if (sc->specmode & CAT_CLAW_MOD) {
                                /* read and write sc share the same sc struct */
                                ca->sc[i + 1]= NULL;
                                for (j=0; j<MAX_LINKS; j++) {
                                        if (sc->links[j] != NULL) {
                                                KFREE(sc->links[j]);
                                        }
                                }
                        } else {
                                KFREE(sc->links[0]); /* non claw mode have only 1 link */
                        }
                        KFREE(sc->links);
                        KFREE(sc);
                        ca->sc[i] = NULL;
                }
        }

        /*
        ** Delete the adapter structure from the component dump table.
        */
        bcopy("adap", cdt_buf, 4);
        cat_del_cdt(cdt_buf, ca, sizeof(struct ca));

        /*
        ** Delete the component dump table.
        */
        dmp_del(cat_cdt_func);

        /*
        ** unpin the driver code
        */
        if (catopened == 0 || --catopened == 0)
                unpincode(catconfig);

		UNLOCK(ca->lcklvl);
        return 0;
} /* cat_term_dev() */


/*****************************************************************************
** NAME: catalloc
**
** FUNCTION: Allocates memory for ca structure and puts into a linked list.
**      Also allocates sub-structures for dds and timer.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES: Assumes we have the global lock...
**
**    Input:
**              nothing
**    Output:
**              pointer to the allocated channel adapter structure
**    Called From:
**              catconfig()
**    Calls:
**              xmalloc() bzero()
**
** RETURNS:     pointer to the allocated ca structure - Success
**              NULL - allocation failed
**
*****************************************************************************/
struct ca *catalloc(void)
{
        struct ca *ca;

        /*
        ** allocate ca structure and put into linked list
        */
        if ((ca = KMALLOC(struct ca)) == NULL)
                return((struct ca *) NULL);

        bzero((caddr_t)ca, sizeof(struct ca ));

        if (caheader)
                ca->next = caheader;
        else
                ca->next = NULL;
        caheader = ca;

        return(ca);
} /* catalloc() */



/*****************************************************************************
** NAME:        catfree
**
** FUNCTION:    Finds the given channel adapter structure and frees it.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES:
**    Input:
**              Pointer to a channel adapter structure
**    Output:
**              status code
**    Called From:
**              catconfig()
**    Calls:
**              xmalloc()
**
** RETURNS:
**              0 - Success
**              EINVAL - couldn't find 'ca' structure to free
**              ENOMEM - KFREE() (xmfree()) failed to free allocated memory
**
*****************************************************************************/
int     catfree(struct ca *ca)
{
        struct ca **nextptr;

        for (nextptr = &caheader; *nextptr != ca; nextptr = &(*nextptr)->next) {
                if (*nextptr == NULL)
                        return(EINVAL);
        }
        *nextptr = ca->next;

        KFREE(ca);
        return 0;
} /* catfree() */


/*****************************************************************************
** NAME:        catget
**
** FUNCTION:    Searches for and returns the ca structure associated with the
**              minor device number.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES:
**    Input:
**              Major/Minor device numbers
**    Output:
**              Pointer to a channel adapter structure
**    Called From:
**
**    Calls:
**              nothing...
**
** RETURNS:     pointer to a channel adapter structure - Success
**              NULL - couldn't find the given channel adapter structure
**
*****************************************************************************/
struct ca *catget(dev_t dev)
{
        struct ca *ca;

        for (ca = caheader; ca != NULL; ca = ca->next) {
                if (minor(ca->dev) == dev)
                        break;
        }
        return(ca);
} /* catget() */
