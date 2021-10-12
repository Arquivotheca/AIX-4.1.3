static char sccsid[] = "@(#)51	1.10  src/bos/kernext/scsi/pscsi720ddt.c, sysxscsi, bos411, 9432A411a 7/30/94 16:02:26";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver Top Half
 *
 * FUNCTIONS:
 *    p720_config, p720_open, p720_close, p720_fail_open, p720_inquiry
 *    p720_start_unit, p720_test_unit_rdy, p720_readblk, p720_adp_str_init,
 *    p720_ioctl, p720_build_command, p720_script_init, p720_diagnostic,
 *    p720_run_diagnostics, p720_loop_test_diagnotics, p720_diag_arb_select,
 *    p720_diag_move_byte_out, p720_diag_move_byte_in, p720_register_test,
 *    p720_pos_register_test, p720_diag_reset_scsi_bus p720_start_dev,
 *    p720_stop_dev, p720_issue_abort, p720_issue_BDR
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*                                                                      */
/* COMPONENT:   SYSXSCSI                                                */
/*                                                                      */
/* NAME:        pscsi720ddt.c                                           */
/*                                                                      */
/* FUNCTION:    IBM SCSI Adapter Driver Top Half Source File            */
/*                                                                      */
/*      This adapter driver is the interface between a SCSI device      */
/*      driver and the actual SCSI adapter.  It executes commands       */
/*      from multiple drivers which contain generic SCSI device         */
/*      commands, and manages the execution of those commands.          */
/*      Several ioctls are defined to provide for system management     */
/*      and adapter diagnostic functions.                               */
/*                                                                      */
/* STYLE:                                                               */
/*                                                                      */
/*      To format this file for proper style, use the indent command    */
/*      with the following options:                                     */
/*                                                                      */
/*      -bap -ncdb -nce -cli0.5 -di8 -nfc1 -i4 -l78 -nsc -nbbb -lp      */
/*      -c4 -nei -nip                                                   */
/*                                                                      */
/*      Following formatting with the indent command, comment lines     */
/*      longer than 80 columns will need to be manually reformatted.    */
/*      To search for lines longer than 80 columns, use:                */
/*                                                                      */
/*      cat <file> | untab | fgrep -v sccsid | awk "length >79"         */
/*                                                                      */
/*      The indent command may need to be run multiple times.  Make     */
/*      sure that the final source can be indented again and produce    */
/*      the identical file.                                             */
/*                                                                      */
/************************************************************************/

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/dma.h>
#include <sys/sysdma.h>
#include <sys/ioacc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/except.h>
#include <sys/param.h>
#include <sys/lockl.h>
#include <sys/priv.h>
#include <sys/watchdog.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/xmem.h>
#include <sys/time.h>
#include <sys/errids.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/trcmacros.h>
#include <sys/adspace.h>
#include <sys/scsi.h>
#include <sys/pscsi720dd.h>
/* END OF INCLUDED SYSTEM FILES  */

/************************************************************************/
/* Global pinned device driver static data areas                        */
/************************************************************************/
/* global static structure to hold the driver's EPOW handler struct     */
extern struct    intr    epow_struct;

/* global driver component dump table pointer                           */
extern struct  p720_cdt_table    *p720_cdt;

/* global pointer for adapter structure                                 */
extern struct    adapter_def    adp_str;

extern int    adp_str_inited;

extern ULONG   A_abort_select_failed_Used[];
extern ULONG   A_abort_io_complete_Used[];
extern ULONG   A_uninitialized_reselect_Used[];

extern ULONG   E_scsi_0_lun_Used[];
extern ULONG   E_scsi_1_lun_Used[];
extern ULONG   E_scsi_2_lun_Used[];
extern ULONG   E_scsi_3_lun_Used[];
extern ULONG   E_scsi_4_lun_Used[];
extern ULONG   E_scsi_5_lun_Used[];
extern ULONG   E_scsi_6_lun_Used[];
extern ULONG   E_scsi_7_lun_Used[];

extern ULONG   INSTRUCTIONS;
/************************************************************************/
/* Global pageable device driver static data areas                      */
/************************************************************************/
/* global adapter device driver lock word                               */
lock_t    p720_lock = LOCK_AVAIL;

#include "pscsi720tss.h"

/************************************************************************/
/*                                                                      */
/* NAME:        p720_config                                             */
/*                                                                      */
/* FUNCTION:    Adapter Driver Configuration Routine                    */
/*                                                                      */
/*      For the INIT option, this routine allocates and initializes     */
/*      data structures required for processing user requests to the    */
/*      adapter.  If the TERM option is specified, this routine will    */
/*      delete a previously defined device and free the structures      */
/*      associated with it.  If the QVPD option is specified, this      */
/*      routine will return the adapter vital product data.             */
/*                                                                      */
/*      During device define time, this routine allocates and           */
/*      initializes data structures for processing of requests from     */
/*      the device driver heads to the scsi chip device driver.  The    */
/*      code is set up to handle only a single device instance.         */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called by a process.                   */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure                     */
/*      adap_ddi - adapter dependent information structure              */
/*      uio     - user i/o area struct                                  */
/*      devsw   - kernel entry point struct                             */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      op      - operation code (INIT, TERM, or QVPD)                  */
/*      uiop    - pointer to uio structure for data for the             */
/*                specified operation code                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      EFAULT  - return from uiomove                                   */
/*      EBUSY   - on terminate, means device still opened               */
/*      ENOMEM  - memory space unavailable for required allocation      */
/*      EINVAL  - invalid config parameter passed                       */
/*      EIO     - bad operation, or permanent I/O error                 */
/*                                                                      */
/************************************************************************/
int
p720_config(
           dev_t devno,
           int op,
           struct uio * uiop)
{
    struct devsw p720_dsw;
    struct adap_ddi local_ddi;
    int     ret_code, rc;
    extern int nodev();

    DEBUG_0 ("Entering p720_config routine.\n")

    ret_code = 0;       /* default return code */
    rc = lockl(&(p720_lock), LOCK_SHORT);        /* serialize this */
    if (rc == LOCK_SUCC)
    {   /* lock succeded */
        switch (op)
        {       /* begin op switch */
        case CFG_INIT:
            if (adp_str_inited)
            {   /* pointer already initialized */
                ret_code = EIO;
                break;
            }
            else
            {   /* new uninitialized pointer */
                /* set up entry points to the driver    */
                /* for the device switch table          */
                p720_dsw.d_open = (int (*) ()) p720_open;
                p720_dsw.d_close = (int (*) ()) p720_close;
                p720_dsw.d_read = nodev;
                p720_dsw.d_write = nodev;
                p720_dsw.d_ioctl = (int (*) ()) p720_ioctl;
                p720_dsw.d_strategy = (int (*) ()) p720_strategy;
                p720_dsw.d_ttys = 0;
                p720_dsw.d_select = nodev;
                p720_dsw.d_config = (int (*) ()) p720_config;
                p720_dsw.d_print = nodev;
                p720_dsw.d_dump = (int (*) ()) p720_dump;
                p720_dsw.d_mpx = nodev;
                p720_dsw.d_revoke = nodev;
                p720_dsw.d_dsdptr = 0;
                p720_dsw.d_selptr = 0;
                p720_dsw.d_opts = 0;

                rc = devswadd(devno, &p720_dsw);
                if (rc != 0)
                {       /* failed to add to dev switch table */
                    ret_code = EIO;
                    break;
                }

            }   /* new uninitialized pointer */

            /* move adapter configuration data (from uio  */
            /* space) into local area (kernel space).     */
            rc = uiomove((caddr_t) (&local_ddi),
                         (int) sizeof(struct adap_ddi),
                         UIO_WRITE, (struct uio *) uiop);
            if (rc != 0)
            {   /* copy failed */
                (void) devswdel(devno); /* clean up */
                ret_code = EIO;
                break;
            }

            /* */
            /* do any data validation here.  tce area must  */
            /* start on a tce boundary, and must be twice   */
            /* MAXREQUEST in length interrupt priority must */
            /* be CLASS2, bus type must be Micro Channel,   */
            /* and base address must be on a 4K boundary    */
            /* */
            if ((local_ddi.tcw_length < (2 * MAXREQUEST)) ||
                (local_ddi.tcw_start_addr & (PAGESIZE - 1)) ||
                (local_ddi.int_prior != INTCLASS2) ||
                (local_ddi.bus_type != BUS_MICRO_CHANNEL) ||
                !((local_ddi.base_addr == 0x0080)))
            {   /* problem with ddi data */
                (void) devswdel(devno); /* clean up */
                ret_code = EINVAL;
                break;
            }

            /* copy local ddi to global, static adapter    */
            /* structure                                   */
            bcopy(&local_ddi, &adp_str.ddi,
                  sizeof(struct adap_ddi));
            /* do any required processing on the            */
            /* configuration data                           */
            adp_str.ddi.card_scsi_id &= 0x07;

            /* if the tce area is greater than 1MB plus the */
            /* number of 4K tces used, then scale up the    */
            if (adp_str.ddi.tcw_length >
                (0x100000 + (TCE_TABLE_SIZE * PAGESIZE)))
                adp_str.max_request = (adp_str.ddi.tcw_length -
                     (0x100000 + (TCE_TABLE_SIZE * PAGESIZE))) +
                     MAXREQUEST;
            else
                adp_str.max_request = MAXREQUEST;

            /* This table reflects the min xfer period value */
            /* returned by the device during negotiate   */
            adp_str.xfer_max[0] = 0x19;
            adp_str.xfer_max[1] = 0x1F;
            adp_str.xfer_max[2] = 0x25;
            adp_str.xfer_max[3] = 0x2B;
            adp_str.xfer_max[4] = 0x32;
            adp_str.xfer_max[5] = 0x3E;
            adp_str.xfer_max[6] = 0x4B;
            adp_str.xfer_max[7] = 0x57;
            adp_str.xfer_max[8] = 0x64;
            adp_str.xfer_max[9] = 0x70;
            adp_str.xfer_max[10] = 0x7D;
            adp_str.xfer_max[11] = 0x89;

#ifdef P720_TRACE
            adp_str.current_trace_line = 1;
            adp_str.trace_ptr = (struct p720_trace_struct *)
            xmalloc(sizeof(struct p720_trace_struct), 4, pinned_heap);
#endif
            TRACE_1 ("**P720START*** ", 0)
            TRACE_1 ("in config adp ", 0)
            rc = p720_config_adapter(NULL);
            TRACE_1 ("out config adp", 0)
            if (rc != 0)
            {   /* unsuccessful adapter config */
                (void) devswdel(devno); /* clean up */
                ret_code = EIO;
                break;
            }

            adp_str.devno = devno;
            adp_str.opened = FALSE;
            adp_str_inited = TRUE;
            break;      /* CFG_INIT case */

            /* handle request to terminate an adapter here      */
        case CFG_TERM:
            if (!adp_str_inited)
            {   /* device already deleted */
                ret_code = 0;
                break;
            }

            if (adp_str.opened)
            {   /* error, someone else has it opened */
                ret_code = EBUSY;
                break;
            }

            /* Disable the chip */
            (void) p720_write_POS(POS2, 0x00);

            (void) devswdel(devno);     /* clean up */
#ifdef P720_TRACE
            (void) xmfree(adp_str.trace_ptr, pinned_heap);
#endif
            adp_str_inited = FALSE;

            break;      /* CFG_TERM case */

            /* handle query for adapter VPD here                */
        case CFG_QVPD:
            ret_code = ENXIO;
            break;      /* CFG_QVPD case */

            /* handle invalid config parameter here */
        default:
            ret_code = EINVAL;

        }
    }
    else
    {   /* lock failed */
        ret_code = EIO; /* error--kernel service call failed */
        return (ret_code);
    }

    DEBUG_0 ("Leaving p720_config routine.\n")
    unlockl(&(p720_lock));
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_open                                               */
/*                                                                      */
/* FUNCTION:    Adapter Driver Open Routine                             */
/*                                                                      */
/*      This routine opens the scsi chip and makes it ready.  It        */
/*      allocates adapter specific structures and initializes           */
/*      appropriate fields in them.  The adapter is marked as           */
/*      opened.                                                         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      devflag - & with D_KERNEL                                       */
/*      chan    - unused                                                */
/*      ext     - extended data; this is 0 for normal use, or           */
/*                a value of SC_DIAGNOSTIC selects diagnostic mode      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      EIO     - kernel service failed or invalid operation            */
/*      EPERM   - authority error                                       */
/*      EACCES  - illegal operation due to current mode (diag vs norm)  */
/*      ENOMEM  - not enough memory available                           */
/*                                                                      */
/************************************************************************/
int
p720_open(
         dev_t devno,
         ulong devflag,
         int chan,
         int ext)
{
    int     rc, ret_code;
    int i;
    int     remaining_tces;
    int     undo_level;
    uint    dma_addr;

    DEBUG_0 ("Entering p720_open routine.\n")

    ret_code = 0;
    undo_level = 0;

    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_OPEN, ret_code, devno, devflag,
            chan, ext, 0);

    rc = lockl(&(p720_lock), LOCK_SHORT);        /* serialize this */
    if (rc == LOCK_SUCC)
    {   /* no problem with lock word */

        if (!adp_str_inited)
        {
            ret_code = EIO;
            p720_fail_open(undo_level, ret_code, devno);
            return (ret_code);
        }
        if ((ext & SC_DIAGNOSTIC) && adp_str.opened)
        {       /* cannot open in DIAG if already opened */
            ret_code = EACCES;
            p720_fail_open(undo_level, ret_code, devno);
            return (ret_code);
        }
        if (adp_str.opened && (adp_str.open_mode == DIAG_MODE))
        {       /* cannot open if already opened in DIAG MODE */
            ret_code = EACCES;
            p720_fail_open(undo_level, ret_code, devno);
            return (ret_code);
        }
        if ((ext & SC_DIAGNOSTIC) && (privcheck(RAS_CONFIG) != 0))
        {   /* trying to open in DIAG MODE without RAS_CONFIG authority */
            ret_code = EPERM;
            p720_fail_open(undo_level, ret_code, devno);
            return (ret_code);
        }
        if ((privcheck(DEV_CONFIG) != 0) && (!(devflag & DKERNEL)))
        {       /* must be normal open */
            ret_code = EPERM;
            p720_fail_open(undo_level, ret_code, devno);
            return (ret_code);
        }
        if (!adp_str.opened)
        {       /* not opened yet */
            adp_str.errlog_enable = FALSE;

            rc = pincode(p720_intr);
            if (rc != 0)
            {   /* pin failed */
                ret_code = EIO;
                p720_fail_open(undo_level, ret_code, devno);
                return (ret_code);
            }
            undo_level++;

            if (!(ext & SC_DIAGNOSTIC)) /* NORMAL_MODE */
            {
                /* init reset watchdog timer struct */
                adp_str.reset_watchdog.dog.next = NULL;
                adp_str.reset_watchdog.dog.prev = NULL;
                adp_str.reset_watchdog.dog.func = p720_watchdog;
                adp_str.reset_watchdog.dog.count = 0;
                adp_str.reset_watchdog.dog.restart = RESETWAIT;
                adp_str.reset_watchdog.timer_id = PSC_RESET_TMR;

                /* init restart watchdog timer struct */
                adp_str.restart_watchdog.dog.next = NULL;
                adp_str.restart_watchdog.dog.prev = NULL;
                adp_str.restart_watchdog.dog.func = p720_watchdog;
                adp_str.restart_watchdog.dog.count = 0;
                adp_str.restart_watchdog.dog.restart = 0;
                adp_str.restart_watchdog.timer_id = PSC_RESTART_TMR;

                /* initialize the system timers */
                w_init(&adp_str.reset_watchdog.dog);
                w_init(&adp_str.restart_watchdog.dog);
                undo_level++;

                /* initialize the adapter structure */
                p720_adp_str_init();
                rc = i_init(&(adp_str.intr_struct));
                if (rc != 0)
                {       /* i_init of scsi chip interrupt handler */
                    ret_code = EIO;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }
                undo_level++;

                INIT_EPOW(&epow_struct, (int (*) ()) p720_epow,
                          adp_str.ddi.bus_id);
                rc = i_init(&epow_struct);
                if (rc != 0)
                {       /* i_init of epow structure failed */
                    ret_code = EIO;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }
                undo_level++;

                rc = p720_write_reg_disable((uint) ISTAT, (uchar) ISTAT_SIZE, 
			0x40);

                if (rc != 0)
                {       /* error in chip reset */
                    ret_code = EIO;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }       /* error in chip reset */

                delay(1);

                rc = p720_write_reg_disable((uint) ISTAT, (char) ISTAT_SIZE, 
			0x00);
                if (rc != 0)
                {       /* error in chip reset */
                    ret_code = EIO;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }       /* error in chip reset */

                rc = p720_chip_register_init(NULL);
                if (rc != 0)
                {       /* error in chip setup */
                    ret_code = EIO;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }

                /* allocate and set up the component dump table entry */
                p720_cdt = (struct p720_cdt_table *) xmalloc(
                                        (uint) sizeof(struct p720_cdt_table),
                                                     (uint) 2, pinned_heap);
                if (p720_cdt == NULL)
                {       /* error in dump table memory allocation */
                    ret_code = ENOMEM;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }
                undo_level++;

                /* initialize the storage for the dump table */
                bzero((char *) p720_cdt, sizeof(struct p720_cdt_table));
                rc = dmp_add(p720_cdt_func);
                if (rc != 0)
                {
                    ret_code = ENOMEM;
                    p720_fail_open(undo_level, ret_code,
                                  devno);
                    return (ret_code);
                }
                undo_level++;

                /* set up STA tables and malloc the work area   */
                /* set up index for STA in the TCE static table */
                /* The first tce is used for the STA. */

                /* malloc the 4K work area for the STAs */
                adp_str.STA[0] = (char *) xmalloc((uint)
                                     PAGESIZE, (uint) PGSHIFT, kernel_heap);
                if (adp_str.STA[0] == NULL)
                {       /* error in malloc for STA */
                    ret_code = ENOMEM;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }

                undo_level++;
                if (ltpin(adp_str.STA[0], PAGESIZE))
                {
                    ret_code = EIO;
                    TRACE_1 ("bad ltpin", ret_code)
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }

		ALLOC(adp_str.TCE_alloc[STA_TCE / PSC_WORDSIZE],
			STA_TCE % PSC_WORDSIZE)
                DEBUG_1 ("STA Table@ = %x\n", adp_str.STA[0])

                for     (i = 0; i < NUM_STA; i++)
                {       /* initialize the STA management table */
                    adp_str.STA[i] = adp_str.STA[0] +
                        (i * ST_SIZE);
                }
                /* mark all the STAs as unused */
                adp_str.STA_alloc[0] = 0xFFFFFFFF << (PSC_WORDSIZE - NUM_STA);

                undo_level++;

                /* malloc the 4K work area for the indirect table */
                adp_str.IND_TABLE.system_ptr = (ulong *) xmalloc((uint)
                                     PAGESIZE, (uint) PGSHIFT, kernel_heap);
                if (adp_str.IND_TABLE.system_ptr == NULL)
                {       /* error in malloc for indirect table */
                    ret_code = ENOMEM;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }
                undo_level++;

                if (ltpin(adp_str.IND_TABLE.system_ptr, PAGESIZE))
                {
                    ret_code = EIO;
                    TRACE_1 ("bad ltpin", ret_code)
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }
                /* set up the area for scripts and copy. */
                /* set up index for SCRIPTS in the TCE static table */
                adp_str.IND_TABLE.dma_ptr = (ulong *)
			DMA_ADDR(adp_str.ddi.tcw_start_addr, STA_TCE + 1);
		adp_str.IND_TABLE.TCE_index = 1;
                ALLOC(adp_str.TCE_alloc[(STA_TCE + 1) / PSC_WORDSIZE],
                        (STA_TCE + 1) % PSC_WORDSIZE)
                undo_level++;

                /* set up the 4K Transfer indices into the static */
                /* TCE status table                               */

                /* set up SCRIPTS tables and malloc the work area */
                /* malloc the initial 4K work area for the SCRIPTS */
                adp_str.SCRIPTS[0].script_ptr = (ulong *) xmalloc((uint)
                                     PAGESIZE, (uint) PGSHIFT, kernel_heap);
                if (adp_str.SCRIPTS[0].script_ptr == NULL)
                {       /* error in malloc for scripts */
                    ret_code = ENOMEM;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }

                undo_level++;
                if (ltpin(adp_str.SCRIPTS[0].script_ptr, PAGESIZE))
                {
                    ret_code = EIO;
                    TRACE_1 ("bad ltpin", ret_code)
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }

                DEBUG_1 ("SCRIPTS Table@ = %x\n",
                                 adp_str.SCRIPTS[0].script_ptr)
                /* set up the area for scripts and copy. */
                /* set up index for SCRIPTS in the TCE static table */
                        dma_addr = DMA_ADDR(adp_str.ddi.tcw_start_addr,
                                                    STA_TCE + 2);
                for (i = 0; i < INSTRUCTIONS * 2; i++)
                {
                    *(adp_str.SCRIPTS[0].script_ptr + i) =
                        PSC_SCRIPT[i];
                }
                adp_str.SCRIPTS[0].dma_ptr = (ulong *) dma_addr;
                DEBUG_1 ("p720_open: SCRIPTS@=%x\n", adp_str.SCRIPTS)
                        adp_str.SCRIPTS[0].TCE_index = STA_TCE + 2;
                adp_str.SCRIPTS[0].in_use = SCR_UNUSED;
                ALLOC(adp_str.TCE_alloc[(STA_TCE + 2) / PSC_WORDSIZE],
                        (STA_TCE + 2) % PSC_WORDSIZE)
                undo_level++;

                /* Now we have to calculate the "left over space"   */
                /* for the Large Transfer Area.  We take the space  */
                /* taken up by the STA, 4K Transfer Area, and the   */
                /* SCRIPTS and find what is left from the original  */
                /* area given to us in our dds.                     */

                remaining_tces = (adp_str.ddi.tcw_length -
                                  TCE_TABLE_SIZE * PAGESIZE) / LARGESIZE;
                adp_str.large_tce_start_addr = adp_str.ddi.tcw_start_addr +
                    (TCE_TABLE_SIZE * PAGESIZE);
                /* allocate enough longwords containing enough bits */
                /* to track the allocation of the large TCEs        */
                adp_str.large_TCE_alloc = (uint *) xmalloc((uint)
                      ((remaining_tces + PSC_WORDSIZE-1) / PSC_WORDSIZE) * 4,
                      (uint) 4, pinned_heap);
                if (adp_str.large_TCE_alloc == NULL)
                {       /* table malloc failed */
                    ret_code = ENOMEM;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }
                for (i = 0; i < (remaining_tces + PSC_WORDSIZE-1) /
                                PSC_WORDSIZE; i++)
                    adp_str.large_TCE_alloc[i] = TCE_UNUSED;
                /* initialize the table to unused */
                adp_str.large_req_end = remaining_tces;

                TRACE_1 ("ltce init", (int) adp_str.large_TCE_alloc)
                adp_str.errlog_enable = TRUE;
                adp_str.epow_state = 0;
                undo_level++;

                DEBUG_2 ("dma_lvl=%x, busid=%x\n",
                                 adp_str.ddi.dma_lvl, adp_str.ddi.bus_id)
                        adp_str.channel_id = d_init(adp_str.ddi.dma_lvl,
                                              DMA_INIT, adp_str.ddi.bus_id);
                if (adp_str.channel_id == DMA_FAIL)
                {       /* failed to init the dma channel */
                    ret_code = EIO;
                    p720_fail_open(undo_level, ret_code, devno);
                    return (ret_code);
                }

                d_unmask(adp_str.channel_id);
                /* lock in the dma areas for STA and SCRIPTS */
                dma_addr = adp_str.ddi.tcw_start_addr;
                DEBUG_2 ("tce_start_address = %x, dma_addr = %x\n",
                                 adp_str.ddi.tcw_start_addr, dma_addr)
                DEBUG_1 ("buffer = %x\n", adp_str.STA[0])

                d_master(adp_str.channel_id, DMA_NOHIDE,
                              (char *) adp_str.STA[0],
                              (size_t) PAGESIZE,
                              &adp_str.xmem_STA, (char *) dma_addr);

                /* execute d_master on the Table Indirect area */
                dma_addr = (uint) adp_str.IND_TABLE.dma_ptr;
                DEBUG_1 ("table dma_addr = %x\n", dma_addr)
                DEBUG_1 ("table system addr = %x\n", 
				adp_str.IND_TABLE.system_ptr)

                d_master(adp_str.channel_id, DMA_NOHIDE,
                   (char *) adp_str.IND_TABLE.system_ptr, 
		   (size_t) PAGESIZE, &adp_str.xmem_MOV, 
		   (caddr_t) dma_addr);

                /* execute d_master on the SCRIPTS area */
                dma_addr = (uint) adp_str.SCRIPTS[0].dma_ptr;
                DEBUG_2 ("tce_start_address = %x, dma_addr = %x\n",
                                 adp_str.ddi.tcw_start_addr, dma_addr)
                DEBUG_1 ("buffer = %x\n", adp_str.SCRIPTS[0].script_ptr)

                d_master(adp_str.channel_id, DMA_NOHIDE,
                   (char *) adp_str.SCRIPTS[0].script_ptr, (size_t) PAGESIZE,
                   &adp_str.xmem_SCR, (caddr_t) dma_addr);

                DEBUG_1 ("TCE @ = %x\n", adp_str.TCE_alloc)
            }   /* opened in normal mode */

            if (ext & SC_DIAGNOSTIC)
                adp_str.open_mode = DIAG_MODE;
            else
                adp_str.open_mode = NORMAL_MODE;

        }       /* not opened yet */
    }   /* no problem with lock word */
    else
    {
        ret_code = EIO;
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);
        return (ret_code);
    }
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);

    adp_str.opened = TRUE;
    unlockl(&(p720_lock));
    DEBUG_0 ("Leaving p720_open routine.\n")
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_close                                              */
/*                                                                      */
/* FUNCTION:    Adapter Driver Close Routine                            */
/*                                                                      */
/*      This routine closes the scsi chip instance and releases any     */
/*      resources (as well as unpins the code) that were setup at open  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      chan    - 0; unused                                             */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      0       - successful                                            */
/*      EIO     - kernel service failed or invalid operation            */
/*                                                                      */
/************************************************************************/
int
p720_close(
          dev_t devno,
          int chan)
{
    int     ret_code, rc;
    int i;

    DEBUG_0 ("Entering p720_close routine.\n")
    ret_code = 0;       /* default to good completion */
    DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_CLOSE, ret_code, devno);

    rc = lockl(&(p720_lock), LOCK_SHORT);        /* serialize this */
    if (rc != LOCK_SUCC)
    {   /* lock kernel call failed */
        ret_code = EIO;
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
        return (ret_code);
    }

    if (!adp_str.opened)
    {   /* adapter never opened */
        ret_code = EIO;
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
        unlockl(&(p720_lock));
        return (ret_code);
    }

    /* Loop through the device hash table and stop devices */
    /* that are still started.                            */
    for (i = 0; i < MAX_DEVICES; i++)
    {
        if (adp_str.device_queue_hash[i] != NULL)
        {       /* If there's a device still started */
            p720_stop_dev(i);    /* stops and frees resources */
        }
    }
    if (adp_str.open_mode == NORMAL_MODE)
    {   /* normal open */
        d_mask(adp_str.channel_id);
        d_clear(adp_str.channel_id);    /* free DMA channel */
        (void) ltunpin(adp_str.STA[0], PAGESIZE);
        (void) xmfree((void *) adp_str.STA[0],
                      kernel_heap);

        (void) ltunpin(adp_str.IND_TABLE.system_ptr, PAGESIZE);
        (void) xmfree((void *) adp_str.IND_TABLE.system_ptr, kernel_heap);

        (void) ltunpin(adp_str.SCRIPTS[0].script_ptr, PAGESIZE);
        (void) xmfree((void *) adp_str.SCRIPTS[0].script_ptr,
                      kernel_heap);
        (void) xmfree((void *) adp_str.large_TCE_alloc,
                      pinned_heap);
        /* Disable dma and SCSI interrupts from the chip */
        (void) p720_write_reg_disable((uint) SIEN, (uchar) SIEN_SIZE, 0x0000);
        (void) p720_write_reg_disable((uint) DIEN, (uchar) DIEN_SIZE, 0x00);

        i_clear(&(adp_str.intr_struct));
        /* clear and free the cdt_table */
        (void) dmp_del(p720_cdt_func);
        (void) xmfree((void *) p720_cdt, pinned_heap);
        i_clear(&epow_struct);
        /* clear all system timers */
        w_clear(&adp_str.reset_watchdog.dog);
        w_clear(&adp_str.restart_watchdog.dog);
    }

    (void) unpincode(p720_intr);
    adp_str.opened = FALSE;

    DEBUG_0 ("Leaving p720_close routine.\n")
    unlockl(&(p720_lock));
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
    return (ret_code);

}  /* end p720_close */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_fail_open                                                  */
/*                                                                        */
/* FUNCTION:  Cleans up during failed "open" processing.                  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine is called to perform processing when a failure    */
/*         of the open occurs. This entails freeing storage, unpinning    */
/*         the code, etc.                                                 */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      undo_level  -  value of amount of cleanup required.               */
/*      ret_code    -  return value used for trace.                       */
/*      devno       -  device major/minor number.                         */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      None                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
void
p720_fail_open(
              int undo_level,
              int ret_code,
              dev_t devno)
{

    DEBUG_0 ("Entering p720_fail_open routine.\n")

    switch  (undo_level)
    {   /* begin switch */
    case 13:
        /* free dynamic table for Large Transfer Area */
        (void) xmfree((void *) adp_str.large_TCE_alloc,
                      pinned_heap);
    case 12:
        /* unpin memory taken by SCRIPTS */
        (void) ltunpin(adp_str.SCRIPTS[0].script_ptr, PAGESIZE);
    case 11:
        /* free memory taken by SCRIPTS */
        (void) xmfree((void *) adp_str.SCRIPTS[0].script_ptr,
                      kernel_heap);
    case 10:
        /* unpin memory taken by IND_TABLE */
        (void) ltunpin(adp_str.IND_TABLE.system_ptr, PAGESIZE);
    case 9:
        /* free memory taken by IND_TABLE */
        (void) xmfree((void *) adp_str.IND_TABLE.system_ptr, kernel_heap);
    case 8:
        /* unpin memory taken by STA */
        (void) ltunpin(adp_str.STA[0], PAGESIZE);
    case 7:
        /* free memory taken by STA */
        (void) xmfree((void *) adp_str.STA[0],
                      kernel_heap);
    case 6:
        /* clear the cdt_table */
        (void) dmp_del(p720_cdt_func);
    case 5:
        /* free the cdt_table */
        (void) xmfree((void *) p720_cdt, pinned_heap);
    case 4:
        /* clear the EPOW structure */
        i_clear(&epow_struct);
    case 3:
        /* clear out the scsi chip interrupt handler */
        i_clear(&(adp_str.intr_struct));
    case 2:
        /* clear all system timers */
        w_clear(&adp_str.reset_watchdog.dog);
        w_clear(&adp_str.restart_watchdog.dog);
    case 1:
        /* unpin the device driver */
        (void) unpincode(p720_intr);
    default:
        break;

    }   /* end switch */

    DEBUG_0 ("Leaving p720_fail_open routine.\n")
    DEBUG_0 ("Leaving p720_open through p720_fail_open (with errors)\n")
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);
    unlockl(&(p720_lock));
    return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_inquiry                                                    */
/*                                                                        */
/* FUNCTION:  Issues a SCSI inquiry command to a device.                  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to issue an inquiry to a device.        */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EINVAL - Device not opened.                                       */
/*      EACCES - Adapter not opened in normal mode.                       */
/*      ENOMEM - Could not allocate an scbuf for this command.            */
/*      ENODEV - Device could not be selected.                            */
/*      ENOCONNECT - No connection (SCSI bus fault).                      */
/*      ETIMEDOUT - The inquiry command timed out.                        */
/*      EIO    - Error returned from p720_strategy.                       */
/*      EIO    - No data returned from inquiry command.                   */
/*      EFAULT - Bad copyin or copyout.                                   */
/*                                                                        */
/**************************************************************************/
int
p720_inquiry(
            dev_t devno,
            int arg,
            ulong devflag)
{
    int     ret_code, dev_index, inquiry_length;
    struct sc_buf *bp;
    struct sc_inquiry sc_inq;

    DEBUG_0 ("Entering p720_inquiry routine.\n")
    ret_code = 0;
    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
        ret_code = copyin((char *) arg, &sc_inq,
                          sizeof(struct sc_inquiry));
        if (ret_code != 0)
        {
            DEBUG_0 ("Leaving p720_inquiry routine.\n")
            return (EFAULT);
        }
    }
    else
    {   /* buffer is in kernel space */
        bcopy((char *) arg, &sc_inq, sizeof(struct sc_inquiry));
    }
    if (adp_str.open_mode == DIAG_MODE)
    {   /* adapter opened in diagnostic mode */
        DEBUG_0 ("Leaving p720_inquiry routine.\n")
        return (EACCES);
    }
    /* Obtain device structure from hash on the scsi id */
    /* and lun id.                                      */
    dev_index = INDEX(sc_inq.scsi_id, sc_inq.lun_id);

    if (adp_str.device_queue_hash[dev_index] == NULL)
    {   /* device queue structure not already allocated */
        DEBUG_0 ("Leaving p720_inquiry routine.\n")
        return (EINVAL);
    }
    bp = p720_build_command();
    if (bp == NULL)
    {
        DEBUG_0 ("Leaving p720_inquiry routine.\n")
        return (ENOMEM);        /* couldn't allocate command */
    }

    bp->scsi_command.scsi_id = sc_inq.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_INQUIRY;
    if (sc_inq.get_extended)
    {
        bp->scsi_command.scsi_cmd.lun = (sc_inq.lun_id << 5) | 0x01;
        bp->scsi_command.scsi_cmd.scsi_bytes[0] = sc_inq.code_page_num;
    }
    else
    {
        bp->scsi_command.scsi_cmd.lun = sc_inq.lun_id << 5;
        bp->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    }
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    /* Always set for the maximum amt of inquiry data */
    bp->scsi_command.scsi_cmd.scsi_bytes[2] = 255;
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* Set the no disconnect flag and the synch/asynch flag  */
    /* for proper data tranfer to occur.                     */
    bp->scsi_command.flags = (sc_inq.flags & SC_ASYNC) | SC_NODISC;

    bp->bufstruct.b_bcount = 255;
    bp->bufstruct.b_flags |= B_READ;
    bp->bufstruct.b_dev = devno;
    bp->timeout_value = 15;

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the inquiry is only running single-threaded  */
    /* to this device.                                           */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the inquiry command. */
    if (p720_strategy(bp))
    {   /* an error was returned */
        (void) xmfree(bp, pinned_heap); /* release buffer */
        DEBUG_0 ("Leaving p720_inquiry routine.\n")
        return (EIO);
    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {   /* an error occurred */
        if (bp->status_validity & SC_ADAPTER_ERROR)
        {       /* if adapter error */
            switch (bp->general_card_status)
            {
            case SC_CMD_TIMEOUT:
                p720_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT,
                           55, 0, NULL, FALSE);
                ret_code = ETIMEDOUT;
                break;
            case SC_NO_DEVICE_RESPONSE:
                ret_code = ENODEV;
                break;
            case SC_SCSI_BUS_FAULT:
                ret_code = ENOCONNECT;
                break;
            default:
                ret_code = EIO;
                break;
            }
        }
        else
        {
            ret_code = EIO;
        }
    }

    /* if no other errors, and yet no data came back, then fail */
    if ((ret_code == 0) &&
        (bp->bufstruct.b_resid == bp->bufstruct.b_bcount))
    {
        ret_code = EIO;
    }
    /* give the caller the lesser of what he asked for, or */
    /* the actual transfer length                          */
    if (ret_code == 0)
    {
        inquiry_length = bp->bufstruct.b_bcount - bp->bufstruct.b_resid;
        if (inquiry_length > sc_inq.inquiry_len)
            inquiry_length = sc_inq.inquiry_len;
        /* Copy out the inquiry data. If the buffer resides */
        /* user space, use copyin, else use bcopy.          */
        if (!(devflag & DKERNEL))
        {
            ret_code = copyout(bp->bufstruct.b_un.b_addr,
                               sc_inq.inquiry_ptr, inquiry_length);
            if (ret_code != 0)
            {
                ret_code = EFAULT;
            }
        }
        else
        { /* buffer is in kernel space */
            bcopy(bp->bufstruct.b_un.b_addr, sc_inq.inquiry_ptr,
                  inquiry_length);
        }
    }
    (void) xmfree(bp, pinned_heap);     /* release buffer */
    DEBUG_1 ("Leaving p720_inquiry routine. %d\n", ret_code);
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_start_unit                                                 */
/*                                                                        */
/* FUNCTION:  Issues a SCSI start unit command to a device.               */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to to issue a start unit command to a   */
/*         device.                                                        */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - Device not opened.                                         */
/*    EACCES - Adapter not opened in normal mode.                         */
/*    ENOMEM - Could not allocate an scbuf for this command.              */
/*    ENODEV - Device could not be selected.                              */
/*    ETIMEDOUT - The start unit command timed out.                       */
/*    ENOCONNECT - No connection (SCSI bus fault).                        */
/*    EIO    - Error returned from p720_strategy.                         */
/*    EFAULT - Bad copyin or copyout.                                     */
/*                                                                        */
/**************************************************************************/
int
p720_start_unit(
               dev_t devno,
               int arg,
               ulong devflag)
{
    int     ret_code, dev_index;
    struct sc_buf *bp;
    struct sc_startunit sc_stun;

    DEBUG_0 ("Entering p720_start_unit.\n")
            ret_code = 0;
    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
        ret_code = copyin((char *) arg, &sc_stun,
                          sizeof(struct sc_startunit));
        if (ret_code != 0)
        {
            DEBUG_0 ("EFAULT: Leaving p720_start_unit routine.\n")
            return (EFAULT);
        }
    }
    else
    {   /* buffer is in kernel space */
        bcopy((char *) arg, &sc_stun, sizeof(struct sc_startunit));
    }
    if (adp_str.open_mode == DIAG_MODE)
    {   /* adapter opened in diagnostic mode */
        DEBUG_0 ("Leaving p720_start_unit routine. EACCES\n")
        return (EACCES);
    }
    /* Obtain device structure from hash on the scsi id */
    /* and lun id.                                      */
    dev_index = INDEX(sc_stun.scsi_id, sc_stun.lun_id);

    if (adp_str.device_queue_hash[dev_index] == NULL)
    {   /* device queue structure not allocated */
        DEBUG_0 ("Leaving p720_start_unit .EINVAL\n")
        return (EINVAL);
    }
    bp = p720_build_command();
    if (bp == NULL)
    {
        DEBUG_0 ("Leaving p720_start_unit ENOMEM.\n")
        return (ENOMEM);        /* couldn't allocate command */
    }

    bp->scsi_command.scsi_id = sc_stun.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_START_STOP_UNIT;
    /* The immediate bit for this command is set depending */
    /* on the value of the immediate flag.                 */
    bp->scsi_command.scsi_cmd.lun = (sc_stun.lun_id << 5) |
        (sc_stun.immed_flag ? 0x01 : 0);
    bp->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    /* Set the command start flag */
    bp->scsi_command.scsi_cmd.scsi_bytes[2] =
        (sc_stun.start_flag ? 0x01 : 0);
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* Set the no disconnect flag and the synch/asynch flag  */
    /* for proper data tranfer to occur.                     */
    bp->scsi_command.flags = (sc_stun.flags & SC_ASYNC) | SC_NODISC;

    bp->bufstruct.b_bcount = 0;
    bp->bufstruct.b_dev = devno;
    bp->timeout_value = sc_stun.timeout_value;  /* set timeout value */

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the inquiry is only running single-threaded  */
    /* to this device.                                           */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the start unit cmd. */
    if (p720_strategy(bp))
    {   /* an error was returned */
        (void) xmfree(bp, pinned_heap); /* release buffer */
        DEBUG_0 ("Leaving p720_start_unit .\n")
        return (EIO);
    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {   /* an error occurred */
        if (bp->status_validity & SC_ADAPTER_ERROR)
        {       /* if adapter error */
            switch (bp->general_card_status)
            {
            case SC_CMD_TIMEOUT:
                p720_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT,
                           60, 0, NULL, FALSE);
                ret_code = ETIMEDOUT;
                break;
            case SC_NO_DEVICE_RESPONSE:
                ret_code = ENODEV;
                break;
            case SC_SCSI_BUS_FAULT:
                ret_code = ENOCONNECT;
                break;
            default:
                ret_code = EIO;
                break;
            }
        }
        else
        {
            ret_code = EIO;
        }
    }

    (void) xmfree(bp, pinned_heap);     /* release buffer */
    DEBUG_0 ("Leaving p720_start_unit .\n")
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_test_unit_rdy                                              */
/*                                                                        */
/* FUNCTION:  Issues a SCSI test unit ready command to a device.          */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to to issue a test unit ready command   */
/*         to a device.                                                   */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - Device not opened.                                         */
/*    EACCES - Adapter not opened in normal mode.                         */
/*    ENOMEM - Could not allocate an scbuf for this command.              */
/*    ENODEV - Device could not be selected.                              */
/*    ETIMEDOUT - The start unit command timed out.                       */
/*    ENOCONNECT - No connect (SCSI bus fault).                           */
/*    EIO    - Error returned from p720_strategy.                          */
/*    EFAULT - Bad copyin or copyout.                                     */
/*                                                                        */
/**************************************************************************/
int
p720_test_unit_rdy(
                  dev_t devno,
                  int arg,
                  ulong devflag)
{
    int     ret_code, ret_code2, dev_index;
    struct sc_buf *bp;
    struct sc_ready sc_rdy;

    DEBUG_0 ("Entering p720_test_unit_rdy routine.\n")

    ret_code = 0;
    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
        ret_code = copyin((char *) arg, &sc_rdy,
                          sizeof(struct sc_ready));
        if (ret_code != 0)
        {
            DEBUG_0 ("Leaving p720_test_unit_rdy routine. EFAULT\n")
            return (EFAULT);
        }
    }
    else
    { /* buffer is in kernel space */
        bcopy((char *) arg, &sc_rdy, sizeof(struct sc_ready));
    }
    if (adp_str.open_mode == DIAG_MODE)
    {   /* adapter opened in diagnostic mode */
        DEBUG_0 ("Leaving p720_test_unit_rdy routine. EACCES\n")
        return (EACCES);
    }
    /* Obtain device structure from hash on the scsi id */
    /* and lun id.                                      */
    dev_index = INDEX(sc_rdy.scsi_id, sc_rdy.lun_id);
    if (adp_str.device_queue_hash[dev_index] == NULL)
    {   /* device queue structure not allocated */
        DEBUG_0 ("Leaving p720_test_unit_rdy routine. EINVAL\n")
        return (EINVAL);
    }
    bp = p720_build_command();
    if (bp == NULL)
    {
        DEBUG_0 ("Leaving p720_test_unit_rdy routine. ENOMEM\n")
        return (ENOMEM);        /* couldn't allocate command */
    }

    bp->scsi_command.scsi_id = sc_rdy.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_TEST_UNIT_READY;
    bp->scsi_command.scsi_cmd.lun = (sc_rdy.lun_id << 5);
    bp->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[2] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* There is no need to set the no disconnect flag for the */
    /* test unit ready command so just set the synch/asynch  */
    /* flag for the sc_rdy structure.                        */
    bp->scsi_command.flags = sc_rdy.flags & SC_ASYNC;
    bp->bufstruct.b_bcount = 0;
    bp->bufstruct.b_dev = devno;

    /* Initialize default status to zero.                    */
    sc_rdy.status_validity = 0;
    sc_rdy.scsi_status = 0;

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the inquiry is only running single-threaded  */
    /* to this device.                                           */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the start unit cmd. */
    if (p720_strategy(bp))
    {   /* an error was returned */
        (void) xmfree(bp, pinned_heap); /* release buffer */
        DEBUG_0 ("Leaving p720_test_unit_rdy strategy error EIO.\n")
        return (EIO);

    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {   /* an error occurred */
        if (bp->status_validity & SC_ADAPTER_ERROR)
        {       /* if adapter error */
            switch (bp->general_card_status)
            {
            case SC_CMD_TIMEOUT:
                p720_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT,
                           65, 0, NULL, FALSE);
                ret_code = ETIMEDOUT;
                break;
            case SC_NO_DEVICE_RESPONSE:
                ret_code = ENODEV;
                break;
            case SC_SCSI_BUS_FAULT:
                ret_code = ENOCONNECT;
                break;
            default:
                ret_code = EIO;
                break;
            }
        }
        else
            if (bp->status_validity & SC_SCSI_ERROR)
            {   /* if a scsi status error */
                sc_rdy.status_validity = SC_SCSI_ERROR;
                sc_rdy.scsi_status = bp->scsi_status;
                ret_code = EIO;
            }
            else
            {   /* if general error (fall through case) */
                ret_code = EIO;
            }
    }

    /* Copy out the device status to the st_ready structure      */
    /* passed in by the calling application.                     */
    if (!(devflag & DKERNEL))
    {
        ret_code2 = copyout(&sc_rdy, (char *) arg,
                            sizeof(struct sc_ready));
        if (ret_code2 != 0)
        {
            ret_code = EFAULT;
        }
    }
    else
    {   /* buffer is in kernel space */
        bcopy(&sc_rdy, (char *) arg, sizeof(struct sc_ready));
    }
    (void) xmfree(bp, pinned_heap);     /* release buffer */
    DEBUG_0 ("Leaving p720_test_unit_rdy routine.\n")
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_readblk                                                    */
/*                                                                        */
/* FUNCTION:  Issues a SCSI read command to a device.                     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to read command to a device.            */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - Device not opened, or transfer size is > 1 page            */
/*    EACCES - Adapter not opened in normal mode.                         */
/*    ENOMEM - Could not allocate an scbuf for this command.              */
/*    ENODEV - Device could not be selected.                              */
/*    ETIMEDOUT - The inquiry command timed out.                          */
/*    ENOCONNECT - SCSI bus fault.                                        */
/*    EIO    - Error returned from p720_strategy.                         */
/*    EIO    - No data returned from read command.                        */
/*    EFAULT - Bad copyin or copyout.                                     */
/*                                                                        */
/**************************************************************************/
int
p720_readblk(
            dev_t devno,
            int arg,
            ulong devflag)
{
    int     ret_code, dev_index;
    struct sc_buf *bp;
    struct sc_readblk sc_rd;

    DEBUG_0 ("Entering p720_readblk routine.\n")
    ret_code = 0;
    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
        ret_code = copyin((char *) arg, &sc_rd,
                          sizeof(struct sc_readblk));
        if (ret_code != 0)
        {
            DEBUG_0 ("Leaving p720_readblk routine.\n")
            return (EFAULT);
        }
    }
    else
    {   /* buffer is in kernel space */
        bcopy((char *) arg, &sc_rd, sizeof(struct sc_readblk));
    }
    if (adp_str.open_mode == DIAG_MODE)
    {   /* adapter opened in diagnostic mode */
        DEBUG_0 ("Leaving p720_readblk routine.\n")
        return (EACCES);
    }
    /* Obtain device structure from hash on the scsi id */
    /* and lun id.                                      */
    dev_index = INDEX(sc_rd.scsi_id, sc_rd.lun_id);

    if (adp_str.device_queue_hash[dev_index] == NULL)
    {   /* device queue structure not allocated */
        DEBUG_0 ("Leaving p720_readblk routine.\n")
        return (EINVAL);
    }
    if ((int) sc_rd.blklen > PAGESIZE)
    {   /* The tranfer length is too long to be allowed. */
        DEBUG_0 ("Leaving p720_readblk routine.\n")
        return (EINVAL);
    }
    bp = p720_build_command();
    if (bp == NULL)
    {
        DEBUG_0 ("Leaving p720_readblk routine.\n")
        return (ENOMEM);        /* couldn't allocate command */
    }

    /* Xmalloc a page to be used for the data transfer.     */
    bp->bufstruct.b_un.b_addr = xmalloc((uint) PAGESIZE,
                                        (uint) PGSHIFT,
                                        pinned_heap);

    if (bp->bufstruct.b_un.b_addr == NULL)
    {   /* Unable to get required storage */
        (void) xmfree(bp, pinned_heap); /* release sc_buf */
        DEBUG_0 ("Leaving p720_readblk routine.\n")
        return (ENOMEM);
    }

    bp->scsi_command.scsi_id = sc_rd.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_READ;
    /* Set up the byte count for this command */
    bp->scsi_command.scsi_cmd.lun = (sc_rd.lun_id << 5) |
        ((sc_rd.blkno >> 16) & 0x1f);
    bp->scsi_command.scsi_cmd.scsi_bytes[0] = (sc_rd.blkno >> 8) & 0xff;
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = sc_rd.blkno & 0xff;
    bp->scsi_command.scsi_cmd.scsi_bytes[2] = 1;        /* single block */
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* Set the no disconnect flag and the synch/asynch flag  */
    /* for proper data tranfer to occur.                     */
    bp->scsi_command.flags = (sc_rd.flags & SC_ASYNC) | SC_NODISC;
    /* put no disconnect flag back in after fix */

    bp->bufstruct.b_bcount = (unsigned) sc_rd.blklen;
    bp->bufstruct.b_flags |= B_READ;
    bp->bufstruct.b_dev = devno;
    bp->timeout_value = sc_rd.timeout_value;

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the readblk is only running single-threaded  */
    /* to this device.                                           */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the readblk command. */
    if (p720_strategy(bp))
    {   /* an error was returned */
        (void) xmfree(bp->bufstruct.b_un.b_addr, pinned_heap);
        (void) xmfree(bp, pinned_heap); /* release sc_buf */
        DEBUG_0 ("Leaving p720_readblk routine.\n")
        return (EIO);
    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {   /* an error occurred */
        if (bp->status_validity & SC_ADAPTER_ERROR)
        {       /* if adapter error */
            switch (bp->general_card_status)
            {
            case SC_CMD_TIMEOUT:
                p720_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT,
                           70, 0, NULL, FALSE);
                ret_code = ETIMEDOUT;
                break;
            case SC_NO_DEVICE_RESPONSE:
                ret_code = ENODEV;
                break;
            case SC_SCSI_BUS_FAULT:
                ret_code = ENOCONNECT;
                break;
            default:
                ret_code = EIO;
                break;
            }
        }
        else
        {
            ret_code = EIO;
        }
    }

    /* if no other errors, and yet no data came back, then fail */
    if ((ret_code == 0) &&
        (bp->bufstruct.b_resid == bp->bufstruct.b_bcount))
    {
        ret_code = EIO;
    }
    /* No errors, so give the calling routine the data     */
    if (ret_code == 0)
    {
        /* Copy out the readblk data. If the buffer resides */
        /* user space, use copyin, else use bcopy.          */
        if (!(devflag & DKERNEL))
        {
            ret_code = copyout(bp->bufstruct.b_un.b_addr,
                               sc_rd.data_ptr, sc_rd.blklen);
            if (ret_code != 0)
            {
                ret_code = EFAULT;
            }
        }
        else
        {       /* buffer is in kernel space */
            bcopy(bp->bufstruct.b_un.b_addr, sc_rd.data_ptr,
                  sc_rd.blklen);
        }
    }

    (void) xmfree(bp->bufstruct.b_un.b_addr, pinned_heap);
    (void) xmfree(bp, pinned_heap);     /* release sc_buf */
    DEBUG_0 ("Leaving p720_readblk routine.\n")
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_adp_str_init                                               */
/*                                                                        */
/* FUNCTION:  Initializes adapter stucture variables.                     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine is called to initialize the adapter structure     */
/*         variables, arrays, etc.                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      None                                                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      None                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
void
p720_adp_str_init()
{

    int     i;

    DEBUG_0 ("Entering p720_adp_str_init routine \n")

    /* init the adapter interrupt handler struct */
    adp_str.intr_struct.next = (struct intr *) NULL;
    adp_str.intr_struct.handler = p720_intr;
    adp_str.intr_struct.bus_type = adp_str.ddi.bus_type;
    adp_str.intr_struct.flags = 0;
    adp_str.intr_struct.level = adp_str.ddi.int_lvl;
    adp_str.intr_struct.priority = adp_str.ddi.int_prior;
    adp_str.intr_struct.bid = adp_str.ddi.bus_id;

    /* initialize other basic global variables to the adp_str */
    adp_str.channel_id = 0;
    adp_str.xmem_STA.aspace_id = XMEM_GLOBAL;
    adp_str.xmem_SCR.aspace_id = XMEM_GLOBAL;
    adp_str.xmem_MOV.aspace_id = XMEM_GLOBAL;
    adp_str.iowait_inited = FALSE;
    adp_str.dump_inited = FALSE;
    adp_str.dump_started = FALSE;
    adp_str.dump_pri = 0;

    /* initialize the various pointers in the structure */
    adp_str.DEVICE_ACTIVE_head = NULL;
    adp_str.DEVICE_ACTIVE_tail = NULL;
    adp_str.DEVICE_WAITING_head = NULL;
    adp_str.DEVICE_WAITING_tail = NULL;
    adp_str.REQUEST_WFR_head = NULL;
    adp_str.REQUEST_WFR_tail = NULL;
    adp_str.blocked_bp = NULL;
    adp_str.ABORT_BDR_head = NULL;
    adp_str.ABORT_BDR_tail = NULL;
    adp_str.large_TCE_alloc = NULL;

    for (i = 0; i < NUM_TAG; i++)
    {
        adp_str.command[i].tag = (unsigned char)i;
        adp_str.command[i].in_use = FALSE;
    }
 
    for (i = 0; i < TAG_TABLE_SIZE; i++)
        adp_str.TAG_alloc[i] = TAG_UNUSED;

    for (i = 0; i < MAX_DEVICES; i++)
        adp_str.device_queue_hash[i] = NULL;

    adp_str.IND_TABLE.system_ptr = NULL;
    adp_str.IND_TABLE.dma_ptr = NULL;
    adp_str.IND_TABLE.TCE_index = 0;

    for (i = 0; i < MAX_SCRIPTS; i++)
    {
        adp_str.sid_info[i].dma_script_ptr = NULL;
        adp_str.SCRIPTS[i].script_ptr = NULL;
        adp_str.SCRIPTS[i].dma_ptr = 0;
        adp_str.SCRIPTS[i].TCE_index = 0;
        adp_str.SCRIPTS[i].in_use = SCR_UNUSED;
    }
    for (i = 0; i < TCE_TABLE_SIZE/PSC_WORDSIZE; i++)
        adp_str.TCE_alloc[i] = TCE_UNUSED;

    DEBUG_0 ("Leaving p720_adp_str_init routine \n")
    return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_ioctl                                                      */
/*                                                                        */
/* FUNCTION:  SCSI Chip Device Driver Ioctl Routine                       */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine will accept commands to perform specific function */
/*         and diagnostic operations on the scsi chip.  Supported commands*/
/*         are:                                                           */
/*                                                                        */
/*         IOCINFO      - Returns information about the tape device.      */
/*         SCIOSTART    - Open a SCSI ID/LUN                              */
/*         SCIOSTOP     - Close a SCSI ID/LUN.                            */
/*         SCIOINQU     - Issues a SCSI Inquiry command to a device.      */
/*         SCIOSTUNIT   - Issues a SCSI start unit command to a device.   */
/*         SCIOTUR      - Issues a SCSI test unit ready command to device.*/
/*         SCIOREAD     - Issues a SCSI 6-byte read command to device.    */
/*         SCIOHALT     - Issues a abort to SCSI device.                  */
/*         SCIORESET    - Issues a Bus Device Reset to a SCSI device.     */
/*         SCIODIAG     - Run chip diagnostics.                           */
/*         SCIOTRAM     - No operation.  Returns ENXIO.                   */
/*         SCIODNLD     - No operation.  Returns ENXIO.                   */
/*         SCIOSTARTTGT - No operation.  Returns ENXIO.                   */
/*         SCIOSTOPTGT  - No operation.  Returns ENXIO.                   */
/*         SCIOEVENT    - No operation.  Returns ENXIO.                   */
/*         SCIOGTHW     - No support for gathered writes.  Returns EINVAL.*/
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*                                                                        */
/* INPUTS:                                                                */
/*    devno  - major/minor number                                         */
/*    cmd    - code used to determine which operation to perform.         */
/*    arg    - address of a structure which contains values used in the   */
/*             'arg' operation.                                           */
/*    devflag - & with D_KERNEL                                           */
/*    chan   - not used (for multiplexed devices).                        */
/*    ext    - not used.                                                  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EIO - lock failed or adapter not initialized                      */
/*      EFAULT - failed copyout                                           */
/*      ENXIO - called no-op ioctl command                                */
/*      EINVAL - SCIOGTHW or unknown ioctl command                        */
/*                                                                        */
/**************************************************************************/
int
p720_ioctl(
          dev_t devno,  /* major and minor device numbers */
          int cmd,      /* operation to perform */
          int arg,      /* pointer to the user structure */
          ulong devflag,        /* not used */
          int chan,     /* not used */
          int ext)      /* not used */

{
    int     rc, ret_code;
    struct devinfo scinfo;
    DEBUG_0 ("Entering p720_ioctl routine.\n")
    DEBUG_6("devno=%x,cmd=%x,arg=%x,devflag=%x,chan=%x,ext=%x\n",
             devno, cmd, arg, devflag, chan, ext)

    ret_code = 0;       /* default to no errors found  */
    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_IOCTL, ret_code, devno,
            cmd, devflag, chan, ext);
    /* lock the global lock to serialize with open/close/config */
    rc = lockl(&(p720_lock), LOCK_SHORT);        /* serialize this */
    if (rc != LOCK_SUCC)
    {
        DEBUG_0 ("Leaving p720_ioctl routine.\n")
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);
        return (EIO);   /* error--kernel service call failed */
    }
    if ((!adp_str_inited) || (!adp_str.opened))
    {   /* scsi chip has not been inited, defined, or opened */
        DEBUG_0 ("Leaving p720_ioctl routine.\n")
        unlockl(&(p720_lock));
        DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);
        return (EIO);
    }

    switch (cmd)
    {   /* cmd switch */
    case IOCINFO:       /* get device information */
        scinfo.devtype = DD_BUS;
        scinfo.flags = 0;
        scinfo.devsubtype = DS_SCSI;
        scinfo.un.scsi.card_scsi_id = (char) adp_str.ddi.card_scsi_id;
        scinfo.un.scsi.max_transfer = adp_str.max_request;
        if (!(devflag & DKERNEL))
        {  /* for a user process */
            rc = copyout(&scinfo, (char *) arg,
                         sizeof(struct devinfo));
            if (rc != 0)
                ret_code = EFAULT;
        }       /* for a user process */
        else
        {  /* for a kernel process */
            bcopy(&scinfo, (char *) arg, sizeof(struct devinfo));
        }  /* for a kernel process */
        break;
    case SCIOSTART:     /* start a device */
        if (arg >> 11)  /* scsi id > 7 */
	    ret_code = EINVAL;
	else
            ret_code = p720_start_dev(INDEX(arg >> 8, arg));
        break;
    case SCIOSTOP:      /* stop a device */
        if (arg >> 11)  /* scsi id > 7 */
	    ret_code = EINVAL;
	else
            ret_code = p720_stop_dev(INDEX(arg >> 8, arg));
        break;
    case SCIOHALT:      /* issue a SCSI abort cmd */
        ret_code = p720_issue_abort(INDEX(arg >> 8, arg));
        break;
    case SCIORESET:     /* issue a SCSI abort cmd */
        ret_code = p720_issue_BDR(INDEX(arg >> 8, arg));
        break;
    case SCIOINQU:      /* issue a SCSI device inquiry cmd */
        ret_code = p720_inquiry(devno, arg, devflag);
        break;
    case SCIOSTUNIT:    /* issue a SCSI device start unit */
        ret_code = p720_start_unit(devno, arg, devflag);
        break;
    case SCIOTUR:       /* issue  SCSI device test unit ready */
        ret_code = p720_test_unit_rdy(devno, arg, devflag);
        break;
    case SCIOREAD:      /* issue a SCSI read cmd (6-byte) */
        ret_code = p720_readblk(devno, arg, devflag);
        break;
    case SCIODIAG:      /* run adapter diagnostics command */
        ret_code = p720_diagnostic(arg, devflag);
        break;
    case SCIOTRAM:      /* no-op, no chip ram to test */
    case SCIODNLD:      /* no-op, no microcode to download */
    case SCIOSTARTTGT:  /* no-op, target mode not supported */
    case SCIOSTOPTGT:   /* no-op, target mode not supported */
    case SCIOEVENT:     /* no-op, async event notification not supported */
        ret_code = ENXIO;
        break;
    default:    /* catch unknown ioctls and SCIOGTHW here */
        ret_code = EINVAL;
        break;
    }   /* cmd switch */

    unlockl(&(p720_lock));
    DEBUG_0 ("Leaving p720_ioctl routine.\n")
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:        p720_build_command                                        */
/*                                                                        */
/* FUNCTION:    Builds an internal command for ioctl routines.            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to initialize fields within the sc_buf  */
/*         structure that is allocated via this routine.  This routine is */
/*         is called by ioctl routines that issue a command via the       */
/*         p720_strategy routine.                                         */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      sc_buf  - input/output request struct used between the adapter    */
/*                driver and the calling SCSI device driver               */
/*                                                                        */
/* INPUTS:                                                                */
/*      none                                                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      returns a pointer to the sc_buf, or NULL, if it could not         */
/*      be allocated.                                                     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      NULL - was not able to allocate memory for bp              	  */
/*                                                                        */
/**************************************************************************/
struct sc_buf *
p720_build_command()
{
    struct sc_buf *bp;

    DEBUG_0 ("Entering p720_build_command routine.\n")

    /* Allocate a sc_buf area for this command  */
    bp = (struct sc_buf *) xmalloc((uint) PAGESIZE,
                                   (uint) PGSHIFT, pinned_heap);
    if (bp == NULL)
    {   /* xmalloc failed--return NULL pointer */
        DEBUG_0 ("Leaving p720_build_command routine.\n")
        return (NULL);
    }

    /* Clear the sc_buf structure to insure all */
    /* fields are initialized to zero.          */
    bzero(bp, sizeof(struct sc_buf));

    /* Initialize other fields of the sc_buf.   */
    bp->bufstruct.b_forw = NULL;
    bp->bufstruct.b_back = NULL;
    bp->bufstruct.av_forw = NULL;
    bp->bufstruct.av_back = NULL;
    bp->bufstruct.b_iodone = (void (*) ()) p720_iodone;
    bp->bufstruct.b_vp = NULL;
    bp->bufstruct.b_event = EVENT_NULL;
    bp->bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
    bp->bufstruct.b_un.b_addr = (char *) bp + sizeof(struct sc_buf);

    /* Additional sc_buf initialization */
    bp->bp = NULL;      /* set for non-spanned command */
    bp->timeout_value = LONGWAIT;       /* set default timeout value */

    DEBUG_0 ("Leaving p720_build_command routine.\n")
    return (bp);
}

/************************************************************************/
/*                                                                      */
/* NAME:        p720_script_init                                        */
/*                                                                      */
/* FUNCTION:    Adapter Script Initialization Routine                   */
/*      This function will patch all interrupts with the half-word      */
/*      that makes up the hash index into the device structure hash     */
/*      table.  Next, it will patch all the target ids referenced       */
/*      within the script.  Then, it will patch the jump command of the */
/*      global IO_WAIT script to jump to this script when the target    */
/*      device does a reselection of the chip.  Finally, we prepare     */
/*      the negotiation, abort, and bdr scripts with the appropriate    */
/*      messages in case they are ever needed by this device.           */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *iowait_vir_addr - the virtual address pointing to the          */
/*              IO_WAIT script that all the scripts will be dependent   */
/*              on as a router.                                         */
/*      *script_vir_addr - the virtual address of the script just       */
/*              created and copied into memory.  The script that needs  */
/*              to be initialized.                                      */
/*      dev_info_hash - the hash value needed by the interrupt handler  */
/*              to determine which device-script caused the programmed  */
/*              interrupt.                                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
p720_script_init(
                uint * iowait_vir_addr,
                uint * script_vir_addr,
                int dev_info_hash,
                uint iowait_dma_addr,
                uint script_dma_addr)

{
    ulong   id_bit_mask;
    ulong  *target_script, *io_wait_ptr;
    ulong   word_buffer;
    uint    jump_address;
    int     i, scsi_id;

    target_script = (ulong *) script_vir_addr;

    /*********************************************************************/
    /* We will patch all the jump values with the virtual address where  */
    /* we want the jump to go to.  We take the relative jump value       */
    /* (in bytes) sitting at the jump address and add it to the base,    */
    /* virtual address and write that back into the jump address         */
    /* location.  We must be sure to take the WORD REVERSE value before  */
    /* and after the addition of the base address.                       */
    /*********************************************************************/
    DEBUG_1 ("script dma address is >>> %x\n", script_dma_addr);
    for (i = 0; i < PATCHES; i++)
    {
        jump_address = word_reverse(PSC_SCRIPT[LABELPATCHES[i]]);
        jump_address += script_dma_addr;
        target_script[LABELPATCHES[i]] = word_reverse(jump_address);
    }

    /*********************************************************************/
    /* Patch the interrupt for the case where we have been reselected by */
    /* a target but we don't have a valid, set jump point.               */
    /*********************************************************************/
    io_wait_ptr = (ulong *) iowait_vir_addr;
    /* We only want to do this once, at the first open of the device */
    if ((io_wait_ptr == target_script) && (!(adp_str.iowait_inited)))
    {
        /***************************************************************/
        /* Find out what the initiator's SCSI ID is, then make this an */
        /* illegal target id.  If a target ever selects the script     */
        /* with this id, we will hit the unknown_reselect_id interrupt.*/
        /* We start at the iowait patch point (the first jump          */
        /* based on the reselecting id), and patch at the instruction  */
        /* corresponding to the card's SCSI id.                        */
        /***************************************************************/
        i = (Ent_iowait_patch_point/4) + (int) adp_str.ddi.card_scsi_id * 2;
        io_wait_ptr[i] = word_reverse(0x800C00FF);

        for (i = 0; i < 8; i++)
        {
            io_wait_ptr[E_scsi_0_lun_Used[i]] =
                word_reverse(((A_uninitialized_reselect_Used[0]
                               - 1) * 4 + script_dma_addr));
            io_wait_ptr[E_scsi_1_lun_Used[i]] =
                word_reverse(((A_uninitialized_reselect_Used[1]
                               - 1) * 4 + script_dma_addr));
            io_wait_ptr[E_scsi_2_lun_Used[i]] =
                word_reverse(((A_uninitialized_reselect_Used[2]
                               - 1) * 4 + script_dma_addr));
            io_wait_ptr[E_scsi_3_lun_Used[i]] =
                word_reverse(((A_uninitialized_reselect_Used[3]
                               - 1) * 4 + script_dma_addr));
            io_wait_ptr[E_scsi_4_lun_Used[i]] =
                word_reverse(((A_uninitialized_reselect_Used[4]
                               - 1) * 4 + script_dma_addr));
            io_wait_ptr[E_scsi_5_lun_Used[i]] =
                word_reverse(((A_uninitialized_reselect_Used[5]
                               - 1) * 4 + script_dma_addr));
            io_wait_ptr[E_scsi_6_lun_Used[i]] =
                word_reverse(((A_uninitialized_reselect_Used[6]
                               - 1) * 4 + script_dma_addr));
            io_wait_ptr[E_scsi_7_lun_Used[i]] =
                word_reverse(((A_uninitialized_reselect_Used[7]
                               - 1) * 4 + script_dma_addr));
            io_wait_ptr[E_lun_msg_addr_Used[i]] =
                word_reverse(iowait_dma_addr + Ent_lun_msg_buf);
        }

        io_wait_ptr[(Ent_lun_msg_buf / 4)] = 0x00000000;
        io_wait_ptr[(Ent_lun_msg_buf / 4) + 1] = 0x00000000;

        adp_str.iowait_inited = TRUE; /* only do this once - ever */
    }

    /*********************************************************************/
    /* We need to set the SCSI ID bits used in all the SELECT with ATN   */
    /* commands to start the SCSI protocol with a target device.         */
    /*********************************************************************/
    scsi_id = SID(dev_info_hash);
    for (i = 0; i < S_COUNT(R_target_id_Used); i++)
    {

        word_buffer = word_reverse(target_script[R_target_id_Used[i]]);
        word_buffer = ((word_buffer & 0xFF00FFFF) | (scsi_id << 16));
        target_script[R_target_id_Used[i]] = word_reverse(word_buffer);
    }

    /* patch the address of the table-indirect table (IND_TABLE) into */
    /* the appropriate script microcode instructions. */
    word_buffer = word_reverse(
			target_script[A_NEXUS_data_base_adr0_Used[0]]);
    word_buffer = ((word_buffer & 0xFFFF00FF) | 
		(((ulong) adp_str.IND_TABLE.dma_ptr & 0x000000FF) << 8));
    target_script[A_NEXUS_data_base_adr0_Used[0]] = word_reverse(word_buffer);

    word_buffer = word_reverse(target_script[A_NEXUS_data_base_adr1_Used[0]]);
    word_buffer = ((word_buffer & 0xFFFF00FF) | 
		((ulong) adp_str.IND_TABLE.dma_ptr & 0x0000FF00));
    target_script[A_NEXUS_data_base_adr1_Used[0]] = word_reverse(word_buffer);

    word_buffer = word_reverse(target_script[A_NEXUS_data_base_adr2_Used[0]]);
    word_buffer = ((word_buffer & 0xFFFF00FF) | 
		(((ulong) adp_str.IND_TABLE.dma_ptr & 0x00FF0000) >> 8));
    target_script[A_NEXUS_data_base_adr2_Used[0]] = word_reverse(word_buffer);

    word_buffer = word_reverse(target_script[A_NEXUS_data_base_adr3_Used[0]]);
    word_buffer = ((word_buffer & 0xFFFF00FF) | 
		(((ulong) adp_str.IND_TABLE.dma_ptr & 0xFF000000) >> 16));
    target_script[A_NEXUS_data_base_adr3_Used[0]] = word_reverse(word_buffer);

    /**********************************************************************/
    /* point to the location in memory where our identify_msg_buf resides */
    /* This is used for the regular command, sync, abort, or bdr cases.   */
    /**********************************************************************/
    target_script[E_identify_msg_addr_Used[0]] =
        word_reverse(script_dma_addr + Ent_identify_msg_buf);
    /**********************************************************************/
    /* initialize the identify_msg_buffer to the lun id of the device     */
    /* this script is associated with.  Bit 7 is set to show that it is   */
    /* an identify message.  Bit 6 is set to show we allow disconnections.*/
    /* The lun pattern of (0-8) is held in Bits 2-0.                      */
    /**********************************************************************/
    id_bit_mask = 0xC0000000;
    id_bit_mask |= ((ulong) LUN(dev_info_hash) << 24);
    target_script[(Ent_identify_msg_buf / 4)] = id_bit_mask;
    target_script[(Ent_identify_msg_buf / 4) + 1] = 0x00000000;

    /**************** BEGIN BUFFER PATCHES ****************/
    /* point to the location in memory where our cmd_msg_in_buf resides. */
    for (i = 0; i < S_COUNT(E_cmd_msg_in_addr_Used); i++)
    {
        target_script[E_cmd_msg_in_addr_Used[i]] =
            word_reverse(script_dma_addr + Ent_cmd_msg_in_buf);
    }
    /* Clear out the message buffers used in synchronous negotiation.    */
    target_script[(Ent_cmd_msg_in_buf / 4)] = 0x00000000;
    target_script[(Ent_cmd_msg_in_buf / 4) + 1] = 0x00000000;
    for (i = 0; i < S_COUNT(E_status_addr_Used); i++)
    {
        target_script[E_status_addr_Used[i]] =
            word_reverse(script_dma_addr + Ent_status_buf);
    }
    target_script[E_reject_msg_addr_Used[0]] =
        word_reverse(script_dma_addr + Ent_reject_msg_buf);
    target_script[(Ent_reject_msg_buf / 4)] = 0x07000000;
    target_script[(Ent_reject_msg_buf / 4) + 1] = 0x00000000;
 
    /* patch address of q-tag buffer */
    target_script[E_tag_msg_addr_Used[0]] =
        word_reverse(script_dma_addr + Ent_tag_msg_buf);
    target_script[(Ent_tag_msg_buf / 4)] = 0x00000000;
    target_script[(Ent_tag_msg_buf / 4) + 1] = 0x00000000;

    /* point to the location in memory where our sync_msg_out_buf resides. */
    target_script[E_sync_msg_out_addr_Used[0]] =
        word_reverse(script_dma_addr + Ent_sync_msg_out_buf);
    target_script[E_sync_msg_out_addr2_Used[0]] =
        word_reverse(script_dma_addr + Ent_sync_msg_out_buf2);

    /* Patch the messages needed to do synchronous negotiation.           */
    id_bit_mask = 0xC0000000;
    id_bit_mask |= ((ulong) LUN(dev_info_hash) << 24);
    target_script[(Ent_sync_msg_out_buf / 4)] = 0x00010301 | id_bit_mask;
    word_buffer = 0;
    word_buffer = DEFAULT_MIN_PHASE;
    word_buffer = (word_buffer << 8) | DEFAULT_BYTE_BUF;
    word_buffer = (word_buffer << 16);
    target_script[(Ent_sync_msg_out_buf / 4) + 1] = word_buffer;

    /* Patch the messages needed to do synchronous negotiation.           */
    target_script[(Ent_sync_msg_out_buf2 / 4)] = (0x01030100 |
                                                  (ulong) DEFAULT_MIN_PHASE);
    target_script[(Ent_sync_msg_out_buf2 / 4) + 1] =
        ((ulong) DEFAULT_BYTE_BUF << 24);

    /* Clear out the status buffers                                       */
    target_script[(Ent_status_buf / 4)] = 0x00000000;
    target_script[(Ent_status_buf / 4) + 1] = 0x00000000;

    /* point to the location in memory where our extended_msg_buf resides. */
    for (i = 0; i < S_COUNT(E_extended_msg_addr_Used); i++)
        target_script[E_extended_msg_addr_Used[i]] =
            word_reverse(script_dma_addr + Ent_extended_msg_buf);

    /* Clear out the extended msg buffers                                 */
    target_script[(Ent_extended_msg_buf / 4)] = 0x00000000;
    target_script[(Ent_extended_msg_buf / 4) + 1] = 0x00000000;

    /* point to the location in memory where our abort_msg_out_buf resides */
    for (i = 0; i < S_COUNT(E_abort_bdr_msg_out_addr_Used); i++)
        target_script[E_abort_bdr_msg_out_addr_Used[i]] =
            word_reverse(script_dma_addr + Ent_abort_bdr_msg_out_buf);

    /* Clear out the abort_msg_out msg buffers and set the abort message  */
    target_script[(Ent_abort_bdr_msg_out_buf / 4)] = 0x06000000;
    target_script[(Ent_abort_bdr_msg_out_buf / 4) + 1] = 0x00000000;

    /* point to location in memory where our abort_bdr_msg_in_buf resides. */
    for (i = 0; i < S_COUNT(E_abort_bdr_msg_in_addr_Used); i++)
        target_script[E_abort_bdr_msg_in_addr_Used[i]] =
            word_reverse(script_dma_addr + Ent_abort_bdr_msg_in_buf);

    /* Clear out the abort_msg_in msg buffers                             */
    target_script[(Ent_abort_bdr_msg_in_buf / 4)] = 0x00000000;
    target_script[(Ent_abort_bdr_msg_in_buf / 4) + 1] = 0x00000000;
    /**************** END BUFFER PATCHES ******************/
    return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_diagnostic                                                 */
/*                                                                        */
/* FUNCTION:  Runs chip diagnostics.                                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine causes diagnostics to be run.                     */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    cmd       - command arguments.                                      */
/*    devflag   - device flag.                                            */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EFAULT - fail kernel service                                        */
/*    EINVAL - adapter not opened                                         */
/*    EACCES - adapter not opened in diag mode                            */
/*                                                                        */
/**************************************************************************/
int
p720_diagnostic(
               int cmd,
               ulong devflag)
{
    int     rc, ret_code;
    struct sc_card_diag *diag_ptr;
    struct sc_card_diag diag;
    caddr_t iocc_addr;

    DEBUG_0 ("Entering p720_diagnostic\n")
    if (!(devflag & DKERNEL))
    {   /* handle user process */
        rc = copyin((char *) cmd, &diag, sizeof(struct sc_card_diag));
        if (rc != 0)
        {
            return (EFAULT);
        }
    }
    else
    {
        /* handle kernel process */
        bcopy((char *) cmd, &diag, sizeof(struct sc_card_diag));
    }

    rc = 0;
    diag_ptr = &diag;
    bzero(&diag_ptr->diag_rc, sizeof(struct rc));
    if (adp_str.opened != TRUE)
    {
        DEBUG_0 ("p720_diag: adapter not opened\n")
        return EINVAL;
    }
    if (adp_str.open_mode == NORMAL_MODE)
    {
        DEBUG_0 ("p720_diag: adapter not opened in diag mode\n")
        return EACCES;
    }
    switch (diag_ptr->option)
    {
    case SC_CARD_DIAGNOSTICS:
        rc = p720_run_diagnostics(diag_ptr);
        break;
    case SC_RESUME_DIAGNOSTICS:
        rc = p720_run_diagnostics(diag_ptr);
        break;
    case SC_CARD_SCSI_WRAP:
        rc = p720_loop_test_diagnostics(diag_ptr);
        break;
    case SC_CARD_REGS_TEST:
        rc = p720_register_test(diag_ptr);
        if (rc != 0)
        {
            diag_ptr->diag_rc.diag_stat = SC_DIAG_MISCMPR;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
        }
        break;
    case SC_CARD_POS_TEST:
        rc = p720_pos_register_test();
        if (rc != 0)
        {
            diag_ptr->diag_rc.diag_stat = SC_DIAG_MISCMPR;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
        }
        break;
    case SC_SCSI_BUS_RESET:
        rc = p720_diag_reset_scsi_bus();
        break;
    default:
        DEBUG_0 ("p720_diag: Invalid command arg\n")
        rc = EINVAL;
        break;
    }
    /* Copy the status bytes to the user space provided. */
    if (!(devflag & DKERNEL))
    {   /* handle user process */
        ret_code = copyout(diag_ptr, (char *) cmd,
                           sizeof(struct sc_card_diag));
        if (ret_code != 0)
            rc = EFAULT;
    }
    else
    {   /* handle kernel process */
        bcopy(diag_ptr, (char *) cmd, sizeof(struct sc_card_diag));
    }
    /* Assert a chip reset */
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    BUS_PUTCX((iocc_addr + ISTAT), 0x40);
    p720_delay(1);
    BUS_PUTCX((iocc_addr + ISTAT), 0x00);
    /* chip register init */
    BUS_PUTCX((iocc_addr + SCNTL0), SCNTL0_INIT);

    /* EXC initially off */
    BUS_PUTCX((iocc_addr + SCNTL1), SCNTL1_EXC_OFF); 

    /* Initially set for 10 Mbyte/sec */
    BUS_PUTCX((iocc_addr + SCNTL3), SCNTL3_INIT_FAST); 

    BUS_PUTCX((iocc_addr + SIEN0), SIEN0_MASK);
    BUS_PUTCX((iocc_addr + SIEN1), SIEN1_MASK);
 
    /* Enable response to reselection and OR in card id */
    BUS_PUTCX((iocc_addr + SCID), (SCID_INIT | adp_str.ddi.card_scsi_id));

    /* Set the SCSI id that the chip responds to when being reselected */
    BUS_PUTCX((iocc_addr + RESPID0), 
                       (uchar) (0x01 << adp_str.ddi.card_scsi_id));

    BUS_PUTCX((iocc_addr + SXFER), SXFER_INIT);
    BUS_PUTCX((iocc_addr + DMODE), DMODE_INIT);
    BUS_PUTCX((iocc_addr + DIEN), DIEN_MASK);
    BUS_PUTCX((iocc_addr + DCNTL), DCNTL_INIT);
    BUS_PUTCX((iocc_addr + CTEST0), CTEST0_INIT);
    BUS_PUTCX((iocc_addr + STIME0), STIME0_INIT);
    BUS_PUTCX((iocc_addr + STEST3), STEST3_INIT);

    BUSIO_DET(iocc_addr);
    DEBUG_0 ("Leaving p720_diagnostic\n");
    return rc;
}  /* end p720_diagnostic */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_run_diagnostics                                            */
/*                                                                        */
/* FUNCTION:  Runs a test of the chip DMA and SCSI FIFOs.                 */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine causes chip diagnostics to be run.  This involves */
/*         doing write, read, and compare of the DMA ans SCSI FIFO on the */
/*         chip.  Parity is also tested during this phase.  In addition, a*/
/*         test of the chip parity logic is performed.                    */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    diag_ptr - diagnostic structure pointer                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EFAULT - one of several errors occured                            */
/*                                                                        */
/**************************************************************************/
int
p720_run_diagnostics(
                    struct sc_card_diag * diag_ptr)

{
    int     rc, count1, count2;
    char    pio_rc;
    unsigned short   pios_rc;
    caddr_t iocc_addr;

    DEBUG_0 ("Entering p720_run_diagnostics\n")
    TRACE_1 ("in run_diag", (int) diag_ptr);
    diag_ptr->diag_rc.diag_stat = 0;
    diag_ptr->diag_rc.diag_validity = 0;
    diag_ptr->diag_rc.ahs_validity = 0;
    diag_ptr->diag_rc.ahs = 0;
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    /* The proceeding section of code writes the entire chip DMA */
    /* FIFO and then performs a readback check.                  */
    for (count1 = 4; count1 < 8; count1++)
    {
        /* enable the the DMA FIFO lane. */
        if ((BUS_PUTCX((iocc_addr + CTEST4), count1)) != 0)
        {
            DEBUG_0 ("p720_run_diag: error enabling DMA FIFO lane\n");
            TRACE_1 ("failed C4", 0);
            TRACE_1 ("out run_diag", EFAULT);
            BUSIO_DET(iocc_addr);
            return EFAULT;
        }
        for (count2 = 1; count2 < 9; count2++)
        {
            /* force even parity, leaving CDIS equal to one. */
            if ((BUS_PUTCX((iocc_addr + CTEST0), 0x80)) != 0)
            {
                DEBUG_0 ("p720_run_diag: error writing CTEST0\n");
                TRACE_1 ("failed C0", 0);
                TRACE_1 ("out run_diag", EFAULT);
                BUSIO_DET(iocc_addr);
                return EFAULT;
            }
            /* write a data pattern with even parity to the DMA FIFO. */
            if ((BUS_PUTCX((iocc_addr + CTEST6), 0xAA)) != 0)
            {
                DEBUG_0 ("p720_run_diag: error writing CTEST6\n");
                TRACE_1 ("failed C6", 0);
                TRACE_1 ("out run_diag", EFAULT);
                BUSIO_DET(iocc_addr);
                return EFAULT;
            }
        }
    }
    /* Read DSTAT reg to check DMA FIFO Empty bit. */
    BUS_GETCX((iocc_addr + DSTAT), &pio_rc);
    pio_rc &= 0x80;
    if (pio_rc != 0)
    {
        DEBUG_0 ("p720_run_diag: DMA FIFO empty\n");
        TRACE_1 ("dfifo empty", 0);
        TRACE_1 ("out run_diag", EFAULT);
        (void) BUS_PUTCX((iocc_addr + CTEST4), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    for (count1 = 4; count1 < 8; count1++)
    {
        /* enable the the DMA FIFO lane. */
        if ((BUS_PUTCX((iocc_addr + CTEST4), count1)) != 0)
        {
            DEBUG_0 ("p720_run_diag: error enabling DMA FIFO lane\n");
            TRACE_1 ("failed CT4", 0);
            TRACE_1 ("out run_diag", EFAULT);
            diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
            BUSIO_DET(iocc_addr);
            return EFAULT;
        }
        for (count2 = 1; count2 < 9; count2++)
        {
            BUS_GETCX((iocc_addr + CTEST6), &pio_rc);
            if (pio_rc != 0xAA)
            {
                DEBUG_0 ("p720_run_diag: compare error CTEST6\n");
                TRACE_1 ("failed CT6", 0);
                TRACE_1 ("out run_diag", EFAULT);
                BUS_PUTCX((iocc_addr + CTEST4), 0x00);
                diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
                diag_ptr->diag_rc.diag_validity = 0x01;
                diag_ptr->diag_rc.ahs_validity = 0x01;
                diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
                BUSIO_DET(iocc_addr);
                return EFAULT;
            }
            BUS_GETCX((iocc_addr + CTEST2), &pio_rc);
            pio_rc &= 0x08;  /* check dfp bit */
            if (pio_rc != 0)
            {
                DEBUG_0 ("p720_run_diag: Even parity error in CTEST2\n");
                TRACE_1 ("even par err", 0xc2);
                TRACE_1 ("out run_diag", EFAULT);
                diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
                diag_ptr->diag_rc.diag_validity = 0x01;
                diag_ptr->diag_rc.ahs_validity = 0x01;
                diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
                BUS_PUTCX((iocc_addr + CTEST4), 0x00);
                BUSIO_DET(iocc_addr);
                return EFAULT;
            }
        }       /* end count2 loop */
    }   /* end count1 loop */
    BUS_GETCX((iocc_addr + DSTAT), &pio_rc);
    pio_rc &= 0x80;
    if (pio_rc != 0x80)
    {
        DEBUG_0 ("p720_run_diag: DMA FIFO not empty\n");
        BUS_PUTCX((iocc_addr + CTEST4), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    for (count1 = 4; count1 < 8; count1++)
    {
        /* enable the the DMA FIFO lane. */
        if ((BUS_PUTCX((iocc_addr + CTEST4), count1)) != 0)
        {
            DEBUG_0 ("p720_run_diag: error enabling DMA FIFO lane\n");
            TRACE_1 ("failed CTS4", 0);
            TRACE_1 ("out run_diag", EFAULT);
            diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
            BUSIO_DET(iocc_addr);
            return EFAULT;
        }
        for (count2 = 1; count2 < 9; count2++)
        {
            /* force odd parity. */
            if ((BUS_PUTCX((iocc_addr + CTEST0), 0x88)) != 0)
            {
                DEBUG_0 ("p720_run_diag: error writing CTEST0\n");
                TRACE_1 ("failed CTS0", 0);
                TRACE_1 ("out run_diag", EFAULT);
                diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
                diag_ptr->diag_rc.diag_validity = 0x01;
                diag_ptr->diag_rc.ahs_validity = 0x01;
                diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
                BUSIO_DET(iocc_addr);
                return EFAULT;
            }
            /* write a data pattern with odd parity to the DMA FIFO. */
            if ((BUS_PUTCX((iocc_addr + CTEST6), 0x55)) != 0)
            {
                DEBUG_0 ("run_diag:error CTEST6 odd parity\n");
                TRACE_1 ("failed CTS6", 0);
                TRACE_1 ("out run_diag", EFAULT);
                diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
                diag_ptr->diag_rc.diag_validity = 0x01;
                diag_ptr->diag_rc.ahs_validity = 0x01;
                diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
                BUSIO_DET(iocc_addr);
                return EFAULT;
            }
        }       /* end count2 loop */
    }   /* end count1 loop */

    /* Read DSTAT reg to check DMA FIFO Empty bit. */
    BUS_GETCX((iocc_addr + DSTAT), &pio_rc);
    pio_rc &= 0x80;
    if (pio_rc == 0x80)
    {
        DEBUG_0 ("p720_run_diag: DMA FIFO empty\n");
        TRACE_1 ("dfifo empty", 0);
        TRACE_1 ("out run_diag", EFAULT);
        BUS_PUTCX((iocc_addr + CTEST4), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    for (count1 = 4; count1 < 8; count1++)
    {
        /* enable the the DMA FIFO lane. */
        if ((BUS_PUTCX((iocc_addr + CTEST4), count1)) != 0)
        {
            DEBUG_0 ("p720_run_diag: error enabling DMA FIFO lane\n");
            TRACE_1 ("failed CTST4", 0);
            TRACE_1 ("out run_diag", EFAULT);
            diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
            BUSIO_DET(iocc_addr);
            return EFAULT;
        }
        for (count2 = 1; count2 < 9; count2++)
        {
            BUS_GETCX((iocc_addr + CTEST6), &pio_rc);
            if (pio_rc != 0x55)
            {
                DEBUG_0 ("p720_run_diag: error comparing CTEST6 odd parity\n");
                TRACE_1 ("failed CTST6", 0);
                TRACE_1 ("out run_diag", EFAULT);
                diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
                diag_ptr->diag_rc.diag_validity = 0x01;
                diag_ptr->diag_rc.ahs_validity = 0x01;
                diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
                BUS_PUTCX((iocc_addr + CTEST4), 0x00);
                BUSIO_DET(iocc_addr);
                return EFAULT;
            }
            BUS_GETCX((iocc_addr + CTEST2), &pio_rc);
            pio_rc &= 0x08;     /* check dfp bit */
            if (pio_rc != 0x08)
            {
                DEBUG_0 ("p720_run_diag:Odd parity err CTEST2\n");
                TRACE_1 ("odd par err", 0);
                TRACE_1 ("out run_diag", EFAULT);
                diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
                diag_ptr->diag_rc.diag_validity = 0x01;
                diag_ptr->diag_rc.ahs_validity = 0x01;
                diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
                BUS_PUTCX((iocc_addr + CTEST4), 0x00);
                BUSIO_DET(iocc_addr);
                return EFAULT;
            }
        }       /* end count2 loop */
    }   /* end count1 loop */

    BUS_GETCX((iocc_addr + DSTAT), &pio_rc);
    pio_rc &= 0x80;
    if (pio_rc != 0x80)
    {
        DEBUG_0 ("p720_run_diag: DMA FIFO not empty\n");
        TRACE_1 ("dfifo !empty", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
        BUS_PUTCX((iocc_addr + CTEST4), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    /* The following section of code writes the entire chip SCSI  */
    /* FIFO and then performs a readback check.                   */
    if ((BUS_PUTCX((iocc_addr + SCNTL0), 0x04)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error enabling parity in SCNTL0\n");
        TRACE_1 ("enable par err", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    /* start check for even parity logic */
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x04)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error forcing even parity in SCNTL1\n");
        TRACE_1 ("even par sctnl1", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    /* set STW in STEST3 so we can write to the SCSI FIFO */
    if ((BUS_PUTCX((iocc_addr + STEST3), 0x01)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error enabling SCSI FIFO in STEST3\n");
        TRACE_1 ("failed en sfifo", 0);
        TRACE_1 ("out run_diag", EFAULT);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    for (count1 = 1; count1 < 9; count1++)
    {
        if ((BUS_PUTSX((short *)(iocc_addr + SODL), 0xAAAA)) != 0)
        {
            DEBUG_0 ("p720_run_diag: error writing SODL for even parity\n");
            TRACE_1 ("failed sodl", 1);
            TRACE_1 ("out run_diag", EFAULT);
            diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
            BUSIO_DET(iocc_addr);
            return EFAULT;
        }
    }
    /* set STR in STEST3 so we can read the SCSI FIFO */
    if ((BUS_PUTCX((iocc_addr + STEST3), 0x40)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error enabling read of SCSI FIFO in STEST3\n");
        TRACE_1 ("failed en sfifo", 0);
        TRACE_1 ("out run_diag", EFAULT);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    
    for (count1 = 1; count1 < 9; count1++)
    {
        BUS_GETSX((short *)(iocc_addr + SODL), (short *) &pios_rc);
        if (pios_rc != 0xAAAA)
        {
            DEBUG_0 ("p720_run_diag: error comparing SODL\n");
            TRACE_1 ("failed sodl2", pios_rc);
            TRACE_1 ("out run_diag", EFAULT);
            diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
            BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
            BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
            BUS_PUTCX((iocc_addr + STEST3), 0x00);
            BUSIO_DET(iocc_addr);
            return EFAULT;
        }
        BUS_GETCX((iocc_addr + STEST1), &pio_rc);
        pio_rc &= 0x03;  /* check sfp bits */
        if (pio_rc != 0)
        {
            DEBUG_0 ("p720_run_diag: error checking parity in STEST1\n");
            TRACE_1 ("failed st1", pio_rc);
            TRACE_1 ("out run_diag", EFAULT);
            diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
            BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
            BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
            BUS_PUTCX((iocc_addr + STEST3), 0x00);
            BUSIO_DET(iocc_addr);
            return EFAULT;
        }
    }   /* end count1 loop */

    /* Now check the odd parity logic */
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x00)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error forcing even parity in SCNTL1\n");
        TRACE_1 ("failed scntl1", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    /* set STW in STEST3 so we can write to the SCSI FIFO */
    if ((BUS_PUTCX((iocc_addr + STEST3), 0x01)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error enabling SCSI FIFO in STEST3\n");
        TRACE_1 ("failed en sfifo", 0);
        TRACE_1 ("out run_diag", EFAULT);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    for (count1 = 1; count1 < 9; count1++)
    {
        if ((BUS_PUTSX((short *)(iocc_addr + SODL), 0x5555)) != 0)
        {
            DEBUG_0 ("run_diag: error writing SODL odd parity\n");
            TRACE_1 ("failed odd par", SODL);
            TRACE_1 ("out run_diag", EFAULT);
            diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
            BUSIO_DET(iocc_addr);
            return EFAULT;
        }
    }
    /* set STR in STEST3 so we can read the SCSI FIFO */
    if ((BUS_PUTCX((iocc_addr + STEST3), 0x40)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error enabling read of SCSI FIFO in STEST3\n");
        TRACE_1 ("failed en sfifo", 0);
        TRACE_1 ("out run_diag", EFAULT);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    for (count1 = 1; count1 < 9; count1++)
    {
        BUS_GETSX((short *)(iocc_addr + SODL), (short *) &pios_rc);
        if (pios_rc != 0x5555)
        {
            DEBUG_0 ("run_diag: compare error SODL odd parity\n");
            TRACE_1 ("failed compare", 0);
            TRACE_1 ("out run_diag", EFAULT);
            diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
            BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
            BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
            BUS_PUTCX((iocc_addr + STEST3), 0x00);
            BUSIO_DET(iocc_addr);
            return EFAULT;
        }
        BUS_GETCX((iocc_addr + STEST1), &pio_rc);
        pio_rc &= 0x03;
        if (pio_rc != 0x03)
        {
            DEBUG_0 ("p720_run_diag: error checking parity in STEST1\n");
            TRACE_1 ("failed par chk", 0);
            TRACE_1 ("out run_diag", EFAULT);
            diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
            diag_ptr->diag_rc.diag_validity = 0x01;
            diag_ptr->diag_rc.ahs_validity = 0x01;
            diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
            BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
            BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
            BUS_PUTCX((iocc_addr + STEST3), 0x00);
            BUSIO_DET(iocc_addr);
            return EFAULT;
        }
    }   /* end count1 loop */

    /* read SCSI FIFO one more time.  This should cause an error because */
    /* it should be empty at this point.                                 */
    BUS_GETSX((short *)(iocc_addr + SODL), (short *) &pios_rc);
    /* check for SCSI Gross Error in SIST0 */
    BUS_GETCX((iocc_addr + SIST0), &pio_rc);
    pio_rc &= 0x08;
    if (pio_rc != 0x08)
    {
        DEBUG_0 ("p720_run_diag: error checking Gross error in SIST0\n");
        TRACE_1 ("failed gross chk", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
        BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
        BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
        BUS_PUTCX((iocc_addr + STEST3), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }

    if ((BUS_PUTCX((iocc_addr + STEST2), 0x10)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error enabling loopback mode\n");
        TRACE_1 ("STEST2 error", 0);
        TRACE_1 ("out run_diag", EFAULT);
        BUS_PUTCX((iocc_addr + STEST2), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + SCNTL0), 0x0c)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error enabling parity in SCNTL0\n");
        TRACE_1 ("failed scntl0", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x04)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error forcing even parity in SCNTL1\n")
        TRACE_1 ("failed scntl1 par", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + CTEST0), 0x84)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error forcing even parity in CTEST0\n");
        TRACE_1 ("failed ct0 par", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    if ((BUS_PUTSX((short *)(iocc_addr + SODL), 0x0007)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error writing data to SODL\n");
        TRACE_1 ("failed sodl data   ", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x44)) != 0)
    {
        DEBUG_0 ("p720_run_diag: error asserting data in SCNTL1\n");
        TRACE_1 ("scntl1 data assert  ", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_GETCX((iocc_addr + SIST0), &pio_rc);
    /* check for SCSI parity error in SIST0 */
    pio_rc &= 0x01;
    if (pio_rc != 0)
    {
        DEBUG_0 ("p720_run_diag: error checking parity error in SIST0\n")
        TRACE_1 ("par chk err    ", 0);
        TRACE_1 ("out run_diag", EFAULT);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
        BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
        BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
        BUS_PUTCX((iocc_addr + STEST3), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    DEBUG_0 ("Leaving p720_run_diagnostics\n");
    BUSIO_DET(iocc_addr);
    return (0);
}  /* end p720_run_diagnostics */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_loop_test_diagnotics                                       */
/*                                                                        */
/* FUNCTION:  Runs a loop test of the chip SCSI buss.                     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine runs a loop test of the chip scsi bus.  Signals   */
/*         and some register testing is done here.                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    diag_ptr - diagnostic structure pointer                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EFAULT = error raising SCSI reset line                            */
/*                                                                        */
/**************************************************************************/
int
p720_loop_test_diagnostics(
                          struct sc_card_diag * diag_ptr)
{
    int rc;
    char    pio_rc, count;
    uchar command[6] = {0x1D, 0x03, 0x00, 0x00, 0x10, 0x00};
    caddr_t iocc_addr;

    DEBUG_0 ("Entering p720_loop_test_diagnostics\n")
    TRACE_1 ("in test ", 0);
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);

    /* set chip, arbitrate for bus, and select the target */
    if (rc = p720_diag_arb_select(diag_ptr, iocc_addr, FALSE))
        return rc;

    /* Successful selection phase */
    /* Get to message out phase, first by clearing SEL, */
    /* then by asserting MSG and CMD. */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xE8)); 
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x06)); 

    /* Send identify message to the target */
    if (rc = p720_diag_move_byte_out(diag_ptr, iocc_addr, 
                        0xC0, TRUE))
        return rc;

    /* Get to the command phase by clearing MSG */
    /* and asserting CMD. */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xFB)); 

    for (count=0; count < 6; count++)
    {
        /* Send command byte to the target */
        if (rc = p720_diag_move_byte_out(diag_ptr, iocc_addr, 
                  command[count], FALSE))
            return rc;
    }

    /* Get to data out phase by clearing MSG, C/D, and I/O */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xE8)); 

    TRACE_1 ("strt data out", pio_rc);
    for (count = 0; count < 16; count++)
    {
	if (rc = p720_diag_move_byte_out(diag_ptr, iocc_addr, 
		     (unsigned char) count, FALSE))
	    return rc;
    }  

    /* Write the bus phases in the SOCL reg to bring the chip */
    /* to the command completion phase                        */

    /* Get to status phase (assert I/0 & C/D, deassert MSG) */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), ((pio_rc | 0x03) & 0xFB));

    /* indicate good status */
    if (rc = p720_diag_move_byte_in(diag_ptr, iocc_addr, 0x00))
	    return rc;

    /* Get to message in phase (assert I/0 & MSG) */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x05));

    /* indicate command completion */
    if (rc = p720_diag_move_byte_in(diag_ptr, iocc_addr, 0x00))
	    return rc;

    /* Go to bus free (deassert all signals) */
    BUS_PUTCX((iocc_addr + SOCL), 0x00);

    /* Raise the SCSI reset line */
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x08)) != 0)
    {
        DEBUG_0 ("Error writing to SCNTL1\n");
        BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    delay(HZ); /* 1 second */
    BUS_PUTCX((iocc_addr + SCNTL1), 0x00);

    /* Repeat test, going through the data in phase.  First, */
    /* set chip, arbitrate for bus, and select the initiator */
    if (rc = p720_diag_arb_select(diag_ptr, iocc_addr, TRUE))
        return rc;

    /* Successful selection phase */
    /* Get to message out phase, first by clearing SEL, */
    /* then by asserting MSG, CMD, and I/O. */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xEF)); 
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x07)); 

    /* send identify message to initiator */
    if (rc = p720_diag_move_byte_in(diag_ptr, iocc_addr, 0x80))
	    return rc;

    /* Move to data in phase by clearing MSG and CMD */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xE9));

    TRACE_1 ("strt data in", pio_rc);
    for (count = 0; count < 16; count++)
    {
	if (rc = p720_diag_move_byte_in(diag_ptr, iocc_addr, 
		     (unsigned char) count))
	    return rc;
    }

    /* Write the bus phases in the SOCL reg to bring the chip */
    /* to the command completion phase                        */

    /* Get to status phase (assert I/0 & C/D, deassert MSG) */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), ((pio_rc | 0x03) & 0xFB));

    /* indicate good status */
    if (rc = p720_diag_move_byte_in(diag_ptr, iocc_addr, 0x00))
	    return rc;

    /* Get to message in phase (assert I/0 & MSG) */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x05));

    /* indicate command completion */
    if (rc = p720_diag_move_byte_in(diag_ptr, iocc_addr, 0x00))
	    return rc;

    /* Go to bus free (deassert all signals) */
    BUS_PUTCX((iocc_addr + SOCL), 0x00);
    TRACE_1 ("out test", 0);
    DEBUG_0 ("Leaving p720_loop_test_diagnostics\n")

    BUSIO_DET(iocc_addr);
    return (PSC_NO_ERR);
}  /* end p720_loop_test_diagnostics */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_diag_arb_select                                            */
/*                                                                        */
/* FUNCTION:  Resets chip and starts loopback test, taking chip through   */
/*            arbitration and (re)selection phases.                       */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine runs a loop test of the chip scsi bus.  Signals   */
/*         and some register testing is done here.                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    diag_ptr - diagnostic structure pointer                             */
/*    iocc_addr - base address for pio                                    */
/*    target_role - indicates whether to act as target or initiator       */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, EFAULT otherwise     */
/*                                                                        */
/* ERROR DESCRIPTION:  pio problem or problem connecting to scsi bus      */
/*                                                                        */
/**************************************************************************/
int 
p720_diag_arb_select(struct sc_card_diag * diag_ptr,
	    caddr_t iocc_addr, uchar target_role)
{
    int rc, count;
    char pio_rc;

    /* reset the chip */
    rc = BUS_PUTCX((iocc_addr + ISTAT), 0x40);
    if (rc != 0)
    {
        DEBUG_0 ("error writing to chip reset in ISTAT\n")
        BUS_PUTCX((iocc_addr + ISTAT), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    rc = BUS_PUTCX((iocc_addr + ISTAT), 0x00);
    if (rc != 0)
    {
        DEBUG_0 ("error clearing chip reset in ISTAT\n")
        BUS_PUTCX((iocc_addr + ISTAT), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }

    /* The chip reset set all the chip registers to their */
    /* default values.  We next set specific registers w/ */
    /* the non-default values needed for the loop test.   */

    /* Set SCSI Control Enable, clear SLB and LOW */
    rc = BUS_PUTCX((iocc_addr + STEST2), 0x80);
    if (rc != 0)
    {
        DEBUG_0 ("error setting SCE in STEST2\n")
        BUS_PUTCX((iocc_addr + STEST2), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }

    TRACE_1 ("arb mode ", (0xC0 | target_role));
    rc = BUS_PUTCX((iocc_addr + SCNTL0), (0xC0 | target_role));
    if (rc != 0)
    {
        DEBUG_0 ("error writing to arbitration mode in SCNTL0\n")
        BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    /* Set destination ID */
    rc = BUS_PUTCX((iocc_addr + SDID), 0x00);
    if (rc != 0)
    {
        DEBUG_0 ("error writing dest. id in SDID\n")
        BUS_PUTCX((iocc_addr + SDID), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    /* Set card ID and enable selection and reselection */
    rc = BUS_PUTCX((iocc_addr + SCID), 0x67);
    if (rc != 0)
    {
        DEBUG_0 ("error writing chip id in SCID\n")
        BUS_PUTCX((iocc_addr + SCID), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    /* Set IDs chip responds to from SCSI bus */
    rc = BUS_PUTCX((iocc_addr + RESPID0), 0x81);
    if (rc != 0)
    {
        DEBUG_0 ("error writing chip id in RESPID0\n")
        BUS_PUTCX((iocc_addr + RESPID0), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }

    TRACE_1 ("start seq ", ((target_role) ? 0xe1 : 0xf0))
    /* Set target mode and WATN/ correctly, and start full arbitration */
    rc = BUS_PUTCX((iocc_addr + SCNTL0), ((target_role) ? 0xe1 : 0xf0));
    if (rc != 0)
    {
        DEBUG_0 ("error writing to start sequence bit in SCNTL0\n")
        BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    /* read ISTAT to see if we are connected to the bus  */
    /* to tell if we won arbitration                     */
    count = 5;
    pio_rc = 0;
    TRACE_1 ("arb check ", 0)
    while ((count > 0) && (pio_rc != 0x08))
    {
        BUS_GETCX((iocc_addr + ISTAT), &pio_rc);
        pio_rc &= 0x08;
        TRACE_1 ("ISTAT   ", pio_rc)
        if (pio_rc != 0x08)
            delay(HZ/4);
        count--;
    }
    if (pio_rc != 0x08)
    {
        DEBUG_0 ("Unsuccessful arbitration phase\n")
        BUS_PUTCX((iocc_addr + SOCL), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }

    /* Go to selection phase */
    BUS_GETCX((iocc_addr + SBCL), &pio_rc);
    if (pio_rc < 0)
    {
        DEBUG_0 ("error reading SBCL\n")
        BUS_PUTCX((iocc_addr + SOCL), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }

    /* begin driving BSY */
    TRACE_1 ("mid sel", pio_rc);
    pio_rc |= 0x20;
    rc = BUS_PUTCX((iocc_addr + SOCL), pio_rc);
    if (rc != 0)
    {
        DEBUG_0 ("error writing to selection phase in SOCL\n")
        BUS_PUTCX((iocc_addr + SOCL), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    count = 5;
    pio_rc = 0;
    while ((count > 0) && (pio_rc != 0x40))
    {
        BUS_GETCX((iocc_addr + SIST0), &pio_rc);
        /* check if CMP is set */
        pio_rc &= 0x40;
        if (pio_rc != 0x40)
            delay(HZ/4);
        count--;
    }
    TRACE_1 ("end sel", (int) pio_rc);
    if (pio_rc != 0x40)
    {
        DEBUG_0 ("Unsuccessful selection phase\n")
        BUS_PUTCX((iocc_addr + SOCL), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }

    return (PSC_NO_ERR);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_diag_move_byte_out                                         */
/*                                                                        */
/* FUNCTION:  Moves 1 byte of data onto and off of the scsi bus, handling */
/*            REQ/ACK handshaking for an asynchronous data out phase.     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine assumes SCE is already set, and scsi bus control  */
/*         lines are also set appropriately.                              */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    diag_ptr - diagnostic structure pointer                             */
/*    iocc_addr - base address for pio                                    */
/*    data - 1 byte of data to write and read from scsi bus               */
/*    clear_atn - flag to indicate whether to clear the ATN signal        */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  EFAULT for data miscompare, O otherwise     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int p720_diag_move_byte_out(struct sc_card_diag * diag_ptr,
	    caddr_t iocc_addr, uchar data, uchar clear_atn)
{
    char    pio_rc;

    /* Assert REQ to indicate target wants to receive data */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x80));

    /* Initiator writes data onto bus */
    BUS_PUTCX((iocc_addr + SODL), data);
    BUS_GETCX((iocc_addr + SCNTL1), &pio_rc);
    BUS_PUTCX((iocc_addr + SCNTL1), (pio_rc | 0x40));

    if (clear_atn)
    {
        /* Clear ATN to indicate target can change state after */
        /* receiving the data. */
        BUS_GETCX((iocc_addr + SOCL), &pio_rc);
        BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xF7));
    }

    /* assert ACK to acknowledge data was sent */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x40));

    /* Target receives data, compare data received */
    BUS_GETCX((iocc_addr + SBDL), &pio_rc);
    if (pio_rc !=  data)
    {
        DEBUG_0 ("error comparing data out of SBDL\n")
        TRACE_2 ("cmp err", data, pio_rc)
        BUS_PUTCX((iocc_addr + SOCL), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }

    /* deassert REQ */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0x7F));

    /* deassert ACK */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xBF));

    return (PSC_NO_ERR);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_diag_move_byte_in                                          */
/*                                                                        */
/* FUNCTION:  Moves 1 byte of data onto and off of the scsi bus, handling */
/*            REQ/ACK handshaking for an asynchronous data in phase.      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine assumes SCE is already set, and scsi bus control  */
/*         lines are also set appropriately.                              */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    diag_ptr - diagnostic structure pointer                             */
/*    iocc_addr - base address for pio                                    */
/*    data - 1 byte of data to write and read from scsi bus               */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  EFAULT for data miscompare, O otherwise     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/**************************************************************************/
int p720_diag_move_byte_in(struct sc_card_diag * diag_ptr,
	    caddr_t iocc_addr, uchar data)
{
    char    pio_rc;

    /* write data onto bus */
    BUS_PUTCX((iocc_addr + SODL), data);
    BUS_GETCX((iocc_addr + SCNTL1), &pio_rc);
    BUS_PUTCX((iocc_addr + SCNTL1), (pio_rc | 0x40));

    /* assert REQ */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x80));
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);

    /* assert ACK */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x40));

    /* deassert REQ */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0x7F));

    /* compare data received */
    BUS_GETCX((iocc_addr + SBDL), &pio_rc);
    if (pio_rc != data)
    {
        DEBUG_0 ("error comparing data out of SBDL\n")
        TRACE_2 ("miscmp ", data, pio_rc);
        BUS_PUTCX((iocc_addr + SOCL), 0x00);
        diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
        diag_ptr->diag_rc.diag_validity = 0x01;
        diag_ptr->diag_rc.ahs_validity = 0x01;
        diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }

    /* deassert ACK */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xBF));

    return (PSC_NO_ERR);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_register_test                                              */
/*                                                                        */
/* FUNCTION:  Runs a test of the chip register logic circuits.            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine runs test of chip register logic circuitry.  Data */
/*         patterns are written to each applicable register and read back */
/*         for compare purposes.                                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    diag_ptr - diagnostic structure pointer                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EFAULT - bad PIO read/write                                       */
/*                                                                        */
/**************************************************************************/
int
p720_register_test(
                  struct sc_card_diag * diag_ptr)
{
    int     rc;
    char    pio_rc;
    caddr_t iocc_addr;

    DEBUG_0 ("Entering p720_register_test\n")
    TRACE_1 ("in reg_test       ", (int) diag_ptr);
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    BUS_PUTCX((iocc_addr + SCNTL0), 0xCA);
    BUS_GETCX((iocc_addr + SCNTL0), &pio_rc);
    if (pio_rc != 0xCA)
    {
        DEBUG_0 ("write to SCNTL0 of 0xCA failed\n");
        TRACE_1 ("SCNTL0 failed", pio_rc);
        BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCNTL0), 0x35);
    BUS_GETCX((iocc_addr + SCNTL0), &pio_rc);
    if (pio_rc != 0x35)
    {
        DEBUG_0 ("write to SCNTL0 of 0x35 failed\n");
        TRACE_1 ("E_scntl0 35    ", pio_rc);
        BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
    BUS_PUTCX((iocc_addr + SCNTL1), 0xA0);
    BUS_GETCX((iocc_addr + SCNTL1), &pio_rc);
    if (pio_rc != 0xA0)
    {
        DEBUG_0 ("write to SCNTL1 of 0xA0 failed\n");
        TRACE_1 ("E_scntl1 A0    ", pio_rc);
        BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCNTL1), 0x04);
    BUS_GETCX((iocc_addr + SCNTL1), &pio_rc);
    if (pio_rc != 0x04)
    {
        DEBUG_0 ("write to SCNTL1 of 0x04 failed\n");
        TRACE_1 ("E_scntl1 04    ", pio_rc);
        BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCNTL1), 0x00);

    BUS_PUTCX((iocc_addr + SCNTL3), 0x2A);
    BUS_GETCX((iocc_addr + SCNTL3), &pio_rc);
    if (pio_rc != 0x2A)
    {
        DEBUG_0 ("write to SCNTL3 of 0x2A failed\n");
        TRACE_1 ("E_scntl3 2A    ", pio_rc);
        BUS_PUTCX((iocc_addr + SCNTL3), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCNTL3), 0x55);
    BUS_GETCX((iocc_addr + SCNTL3), &pio_rc);
    if (pio_rc != 0x55)
    {
        DEBUG_0 ("write to SCNTL3 of 0x55 failed\n");
        TRACE_1 ("E_scntl3 55    ", pio_rc);
        BUS_PUTCX((iocc_addr + SCNTL3), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCNTL3), 0x00);
    BUS_PUTCX((iocc_addr + SDID), 0x0A);
    BUS_GETCX((iocc_addr + SDID), &pio_rc);
    if (pio_rc != 0x0A)
    {
        DEBUG_0 ("write to SDID of 0x0A failed\n");
        TRACE_1 ("E_sdid 0A    ", pio_rc);
        BUS_PUTCX((iocc_addr + SDID), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SDID), 0x05);
    BUS_GETCX((iocc_addr + SDID), &pio_rc);
    if (pio_rc != 0x05)
    {
        DEBUG_0 ("write to SDID of 0x05 failed\n")
        TRACE_1 ("E_sdid 05    ", pio_rc);
        BUS_PUTCX((iocc_addr + SDID), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SDID), 0x00);
    BUS_PUTCX((iocc_addr + SIEN0), 0xAA);
    BUS_GETCX((iocc_addr + SIEN0), &pio_rc);
    if (pio_rc != 0xAA)
    {
        DEBUG_0 ("write to SIEN0 of 0xAA failed\n");
        TRACE_1 ("E_sien0 AA    ", pio_rc);
        BUS_PUTCX((iocc_addr + SIEN0), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SIEN0), 0x55);
    BUS_GETCX((iocc_addr + SIEN0), &pio_rc);
    if (pio_rc != 0x55)
    {
        DEBUG_0 ("write to SIEN0 of 0x55 failed\n");
        TRACE_1 ("E_sien0 55    ", pio_rc);
        BUS_PUTCX((iocc_addr + SIEN0), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SIEN0), 0x00);
    BUS_PUTCX((iocc_addr + SIEN1), 0x02);
    BUS_GETCX((iocc_addr + SIEN1), &pio_rc);
    if (pio_rc != 0x02)
    {
        DEBUG_0 ("write to SIEN1 of 0x02 failed\n");
        TRACE_1 ("E_sien1 02    ", pio_rc);
        BUS_PUTCX((iocc_addr + SIEN1), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SIEN1), 0x05);
    BUS_GETCX((iocc_addr + SIEN1), &pio_rc);
    if (pio_rc != 0x05)
    {
        DEBUG_0 ("write to SIEN1 of 0x05 failed\n");
        TRACE_1 ("E_sien1 05    ", pio_rc);
        BUS_PUTCX((iocc_addr + SIEN1), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SIEN1), 0x00);
    BUS_PUTCX((iocc_addr + SCID), 0x2A);
    BUS_GETCX((iocc_addr + SCID), &pio_rc);
    if (pio_rc != 0x2A)
    {
        DEBUG_0 ("write to SCID of 0x2A failed\n")
        TRACE_1 ("E_scid 2A    ", pio_rc);
        BUS_PUTCX((iocc_addr + SCID), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCID), 0x45);
    BUS_GETCX((iocc_addr + SCID), &pio_rc);
    if (pio_rc != 0x45)
    {
        DEBUG_0 ("write to SCID of 0x45 failed\n")
        TRACE_1 ("E_scid 45    ", pio_rc);
        BUS_PUTCX((iocc_addr + SCID), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCID), 0x00);
    BUS_PUTCX((iocc_addr + SXFER), 0xAA);
    BUS_GETCX((iocc_addr + SXFER), &pio_rc);
    if (pio_rc != 0xAA)
    {
        DEBUG_0 ("write to SXFER of 0xAA failed\n");
        TRACE_1 ("E_sxfer AA    ", pio_rc);
        BUS_PUTCX((iocc_addr + SXFER), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SXFER), 0x45);
    BUS_GETCX((iocc_addr + SXFER), &pio_rc);
    if (pio_rc != 0x45)
    {
        DEBUG_0 ("write to SXFER of 0x45 failed\n")
        TRACE_1 ("E_sxfer 45    ", pio_rc);
        BUS_PUTCX((iocc_addr + SXFER), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SXFER), 0x00);
    BUS_PUTCX((iocc_addr + DCMD), 0xAA);
    BUS_GETCX((iocc_addr + DCMD), &pio_rc);
    if (pio_rc != 0xAA)
    {
        DEBUG_0 ("write to DCMD of 0xAA failed\n")
        TRACE_1 ("E_dcmd AA    ", pio_rc);
        BUS_PUTCX((iocc_addr + DCMD), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DCMD), 0x55);
    BUS_GETCX((iocc_addr + DCMD), &pio_rc);
    if (pio_rc != 0x55)
    {
        DEBUG_0 ("write to DCMD of 0x55 failed\n");
        TRACE_1 ("E_dcmd 55    ", pio_rc);
        BUS_PUTCX((iocc_addr + DCMD), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DCMD), 0x00);
    BUS_PUTLX((long *)(iocc_addr + DNAD), 0xAA55AA55);
    BUS_GETLX((long *)(iocc_addr + DNAD), (long *)&rc);
    if (rc != 0xAA55AA55)
    {
        DEBUG_0 ("write to DNAD of 0xAA55AA55 failed\n");
        TRACE_1 ("E_dnad AA55AA55   ", rc);
        BUS_PUTLX((long *)(iocc_addr + DNAD), 0x00000000);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DNAD), 0x55AA55AA);
    BUS_GETLX((long *)(iocc_addr + DNAD), (long *)&rc);
    if (rc != 0x55AA55AA)
    {
        DEBUG_0 ("write to DNAD of 0x55AA55AA failed\n");
        TRACE_1 ("E_dnad 55AA55AA   ", rc);
        BUS_PUTLX((long *)(iocc_addr + DNAD), 0x00000000);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DNAD), 0x00000000);
    /* set MAN bit in DMODE, to allow us to test DSP w/o causing fetches */
    BUS_PUTCX((iocc_addr + DMODE), 0x01);
    BUS_PUTLX((long *)(iocc_addr + DSP), 0xAA55AA55);
    BUS_GETLX((long *)(iocc_addr + DSP), (long *)&rc);
    if (rc != 0xAA55AA55)
    {
        DEBUG_0 ("write to DSP of 0xAA55AA55 failed\n");
        TRACE_1 ("E_dsp AA55AA55   ", rc);
        BUS_PUTLX((long *)(iocc_addr + DSP), 0x00000000);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DSP), 0x55AA55AA);
    BUS_GETLX((long *)(iocc_addr + DSP), (long *)&rc);
    if (rc != 0x55AA55AA)
    {
        DEBUG_0 ("write to DSP of 0x55AA55AA failed\n")
        TRACE_1 ("E_dsp 55AA55AA   ", rc);
        BUS_PUTLX((long *)(iocc_addr + DSP), 0x00000000);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DSP), 0x00000000);
    BUS_PUTLX((long *)(iocc_addr + DSPS), 0xAA55AA55);
    BUS_GETLX((long *)(iocc_addr + DSPS), (long *)&rc);
    if (rc != 0xAA55AA55)
    {
        DEBUG_0 ("write to DSPS of 0xAA55AA55 failed\n");
        TRACE_1 ("E_dsps AA55AA55   ", rc);
        BUS_PUTLX((long *)(iocc_addr + DSPS), 0x00000000);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DSPS), 0x55AA55AA);
    BUS_GETLX((long *)(iocc_addr + DSPS), (long *)&rc);
    if (rc != 0x55AA55AA)
    {
        DEBUG_0 ("write to DSPS of 0x55AA55AA failed\n")
        TRACE_1 ("E_dsps 55AA55AA   ", rc);
        BUS_PUTLX((long *)(iocc_addr + DSPS), 0x00000000);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DSPS), 0x00000000);
    BUS_PUTCX((iocc_addr + DMODE), 0xAA);
    BUS_GETCX((iocc_addr + DMODE), &pio_rc);
    if (pio_rc != 0xAA)
    {
        DEBUG_0 ("write to DMODE of 0xAA failed\n")
        TRACE_1 ("E_dmode AA    ", pio_rc);
        BUS_PUTCX((iocc_addr + DMODE), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DMODE), 0x00);
    BUS_PUTCX((iocc_addr + DIEN), 0x2A);
    BUS_GETCX((iocc_addr + DIEN), &pio_rc);
    if (pio_rc != 0x2A)
    {
        DEBUG_0 ("write to DIEN of 0x2A failed\n")
        TRACE_1 ("E_dien 2A    ", pio_rc);
        BUS_PUTCX((iocc_addr + DIEN), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DIEN), 0x55);
    BUS_GETCX((iocc_addr + DIEN), &pio_rc);
    if (pio_rc != 0x55)
    {
        DEBUG_0 ("write to DIEN of 0x55 failed\n")
        TRACE_1 ("E_dien 55    ", pio_rc);
        BUS_PUTCX((iocc_addr + DIEN), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DIEN), 0x00);
    BUS_PUTCX((iocc_addr + DCNTL), 0xA0);
    BUS_GETCX((iocc_addr + DCNTL), &pio_rc);
    if (pio_rc != 0xA0)
    {
        DEBUG_0 ("write to DCNTL of 0xA0 failed\n")
        TRACE_1 ("E_sdid A0    ", pio_rc);
        BUS_PUTCX((iocc_addr + DCNTL), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DCNTL), 0x50);
    BUS_GETCX((iocc_addr + DCNTL), &pio_rc);
    if (pio_rc != 0x50)
    {
        DEBUG_0 ("write to DCNTL of 0x50 failed\n")
        TRACE_1 ("E_sdid 50    ", pio_rc);
        BUS_PUTCX((iocc_addr + DCNTL), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DCNTL), 0x00);
    DEBUG_0 ("Leaving p720_register_test\n")
    TRACE_1 ("out reg test   ", 0);
    BUSIO_DET(iocc_addr);
    return (0);
}  /* end p720_register_test */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_pos_register_test                                          */
/*                                                                        */
/* FUNCTION:  Runs a test of the POS register logic circuits.             */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine runs a test of the POS registers by first writing */
/*         a data pattern and then reading it for compare.                */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:  none                                                          */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EFAULT - error on PIO read/write                                  */
/*                                                                        */
/**************************************************************************/
int
p720_pos_register_test()
{
    caddr_t iocc_addr;
    char    pio_rc, pos_save;

    DEBUG_0 ("Entering p720_pos_register_test\n")
    iocc_addr = IOCC_ATT(adp_str.ddi.bus_id, 0);
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS0), &pio_rc);
    if (pio_rc != POS0_VAL)
    {
        DEBUG_0 ("Error reading POS0\n");
        TRACE_1 ("err pos0", pio_rc);
        return EFAULT;
    }
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS1), &pio_rc);
    if (pio_rc != POS1_VAL)
    {
        DEBUG_0 ("Error reading POS1\n");
        TRACE_1 ("err pos1", pio_rc);
        return EFAULT;
    }
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), &pos_save);
    BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), 0x0A);
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), &pio_rc);
    pio_rc &= 0x0A;
    if (pio_rc != 0x0A)
    {
        DEBUG_0 ("Error reading POS4\n");
        TRACE_1 ("err1pos4", pio_rc);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), 0x15);
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), &pio_rc);
    pio_rc &= 0x15;
    if (pio_rc != 0x15)
    {
        DEBUG_0 ("Error reading POS4\n");
        TRACE_1 ("err2pos4", pio_rc);
        return EFAULT;
    }
    BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), pos_save);
    IOCC_DET(iocc_addr);
    DEBUG_0 ("Leaving p720_pos_register_test\n");
    return (0);
}  /* end p720_pos_register_test */

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_diag_reset_scsi_bus                                        */
/*                                                                        */
/* FUNCTION:  Resets the SCSI bus lines.                                  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine will toggle the SCSI bus reset line for 30 micro- */
/*         seconds to assert a reset on the SCSI bus. This routines exe-  */
/*         cutes to reset the bus during diagnostic execution.            */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS: None                                                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EFAULT - error on PIO read/write                                  */
/*                                                                        */
/**************************************************************************/
int
p720_diag_reset_scsi_bus()
{
    caddr_t iocc_addr;

    DEBUG_0 ("Entering p720_diag_reset_scsi_bus\n")
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x08)) != 0)
    {
        DEBUG_0 ("Error writing to SCNTL1\n");
        BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
        BUSIO_DET(iocc_addr);
        return EFAULT;
    }
    delay(HZ); /* 1 second */
    BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
    BUSIO_DET(iocc_addr);
    DEBUG_0 ("Leaving p720_diag_reset_scsi_bus\n")
    return (PSC_NO_ERR);
}  /* end p720_diag_reset_scsi_bus */

/**************************************************************************/
/*                                                                        */
/* NAME:        p720_start_dev                                            */
/*                                                                        */
/* FUNCTION:  Allocates resources and starts a device                     */
/*                                                                        */
/*      This routine initializes the device queue to prepare for          */
/*      command processing.                                               */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      dev_index - index to device information                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION                                                      */
/*      EINVAL - device not opened                                        */
/*      EIO - unable to pin code                                          */
/*      ENOMEM - unable to xmalloc memory                                 */
/*      EACCES - adapter not opened in normal mode                        */
/*                                                                        */
/**************************************************************************/
int
p720_start_dev(
              int dev_index)
{

    int     i, old_level;
    struct dev_info *dev_ptr;
    struct scsi_id_info *sid_ptr;
    int     index, scr_index, TCE_index;
    uint    dma_addr;

    DEBUG_0 ("Entering p720_start_dev routine.\n")

    if (adp_str.open_mode == DIAG_MODE)
    {  /* adapter opened in diagnostic mode */
        return (EACCES);
    }
    if (adp_str.device_queue_hash[dev_index] != NULL)
    {   /* device queue structure already allocated */
        return (EINVAL);
    }
    if (adp_str.ddi.card_scsi_id == SID(dev_index))
    {   /* Device SCSI ID matches adapter SCSI ID.  */
        return (EINVAL);
    }

    /* device queue structure doesn't exist yet */
    adp_str.device_queue_hash[dev_index] = xmalloc(
                            (uint) sizeof(struct dev_info), 4, pinned_heap);
    if (adp_str.device_queue_hash[dev_index] == NULL)
    {   /* malloc failed */
        return (ENOMEM);
    }
    dev_ptr = adp_str.device_queue_hash[dev_index];

    bzero(dev_ptr, (int) sizeof(struct dev_info));

    if (adp_str.sid_info[SID(dev_index)].dma_script_ptr == NULL)
    {
        /* Search through the device queue area for a free script */
        /* instance.                                              */
        if (adp_str.SCRIPTS[0].in_use == SCR_UNUSED)
        {   /* A scripts sequence has been found */
            scr_index = 0;  /* used to address scripts */
        }
        else
        /* If no free script was found, then it's necessary  */
        /* to malloc another page and create additional      */
        /* script instances.                                 */
        {
            for (scr_index = 1; scr_index < MAX_SCRIPTS; scr_index++)
            {
                if (adp_str.SCRIPTS[scr_index].script_ptr == NULL)
                    break;
            }

            /* malloc an additional 4K work area for the SCRIPTS */
            adp_str.SCRIPTS[scr_index].script_ptr = (ulong *) 
		xmalloc((uint) PAGESIZE, (uint) PGSHIFT, kernel_heap);
            if (adp_str.SCRIPTS[scr_index].script_ptr == NULL)
            {   /* Unable to malloc script area. */
                adp_str.device_queue_hash[dev_index] = NULL;
                (void) xmfree(dev_ptr, pinned_heap);
                return (ENOMEM);
            }
            if (ltpin(adp_str.SCRIPTS[scr_index].script_ptr, PAGESIZE))
            {   /* Unable to pin script area. */
                (void) xmfree(adp_str.SCRIPTS[scr_index].script_ptr, 
				kernel_heap);
                adp_str.device_queue_hash[dev_index] = NULL;
                (void) xmfree(dev_ptr, pinned_heap);
                return (EIO);
            }

 	    /* pin the code prior to disabling interrupts */
    	    i = pincode(p720_start_dev);
    	    if (i != 0)
    	    {
                (void) ltunpin(adp_str.SCRIPTS[scr_index].script_ptr, 
			       PAGESIZE);
                (void) xmfree(adp_str.SCRIPTS[scr_index].script_ptr, 
				kernel_heap);
        	adp_str.device_queue_hash[dev_index] = NULL;
        	(void) xmfree(dev_ptr, pinned_heap);
        	return (EIO);
    	    }

            /* find a tce from the 4K xfer area to use */
            old_level = i_disable(adp_str.ddi.int_prior);

            for (i=0 ; i < TCE_TABLE_SIZE/PSC_WORDSIZE; i++)
            if ( (index = p720_getfree (adp_str.TCE_alloc[i])) < PSC_WORDSIZE)
            { /* a free TCE was found - allocate it */
                TCE_index = i * PSC_WORDSIZE + index;  /* which bit in array */
                ALLOC(adp_str.TCE_alloc[i], index)
		break;
            }

            i_enable(old_level);
            (void) unpincode (p720_start_dev);

            /* set up the area for scripts and copy. */
            /* set up index for SCRIPTS in the TCE static table */
            dma_addr = DMA_ADDR(adp_str.ddi.tcw_start_addr, TCE_index);
            for (i = 0; i < INSTRUCTIONS * 2; i++)
            {
                *(adp_str.SCRIPTS[scr_index].script_ptr + i) =
                    PSC_SCRIPT[i];
            }
            adp_str.SCRIPTS[scr_index].dma_ptr = (ulong *) dma_addr;
            DEBUG_1 ("p720_open: SCRIPTS@=%x\n", adp_str.SCRIPTS)
                adp_str.SCRIPTS[scr_index].TCE_index = TCE_index;
            d_master(adp_str.channel_id, DMA_NOHIDE,
                 (char *) adp_str.SCRIPTS[scr_index].script_ptr,
                 (size_t) PAGESIZE,
                 &adp_str.xmem_SCR, (char *) dma_addr);
        }

        adp_str.SCRIPTS[scr_index].in_use = SCR_IN_USE;

	/* initialize the sid_info structure */
	sid_ptr = &adp_str.sid_info[SID(dev_index)];
	sid_ptr->scsi_id = SID(dev_index);
	sid_ptr->negotiate_flag = TRUE;
	sid_ptr->async_device = FALSE;
	sid_ptr->restart_in_prog = FALSE;
	sid_ptr->disconnect_flag = FALSE;
	sid_ptr->tag_flag = adp_str.ddi.card_scsi_id << 3; /* init. to a */
					          /* value never used. */
	sid_ptr->tag_msg_flag = 0xFF;
	sid_ptr->lun_flag = 0xFF;
	sid_ptr->dma_script_ptr = (uint) adp_str.SCRIPTS[scr_index].dma_ptr;
	sid_ptr->script_index = scr_index;
	sid_ptr->bdring_lun = NULL;
	
	/* initialize the script */
        p720_script_init((uint *) (adp_str.SCRIPTS[0].script_ptr),
                (uint *) (adp_str.SCRIPTS[scr_index].script_ptr), dev_index,
                (uint) adp_str.SCRIPTS[0].dma_ptr, sid_ptr->dma_script_ptr);
    }
    else
    {
        sid_ptr = &adp_str.sid_info[SID(dev_index)];
    }
    dev_ptr->sid_ptr = sid_ptr;

    /* Initialize device queue flags.             */
    dev_ptr->DEVICE_ACTIVE_fwd = NULL;
    dev_ptr->DEVICE_ACTIVE_bkwd = NULL;
    dev_ptr->ABORT_BDR_fwd = NULL;
    dev_ptr->ABORT_BDR_bkwd = NULL;
    dev_ptr->active_head = NULL;
    dev_ptr->active_tail = NULL;
    dev_ptr->wait_head = NULL;
    dev_ptr->wait_tail = NULL;
    dev_ptr->cmd_save_head = NULL;
    dev_ptr->cmd_save_tail = NULL;
    dev_ptr->bp_save_head = NULL;
    dev_ptr->bp_save_tail = NULL;

    dev_ptr->scsi_id = SID(dev_index);
    dev_ptr->lun_id = LUN(dev_index);
    dev_ptr->ioctl_wakeup = FALSE;
    dev_ptr->ioctl_event = EVENT_NULL;
    dev_ptr->stop_event = EVENT_NULL;
    dev_ptr->flags = RETRY_ERROR;

    /* init watchdog timer struct                 */
    dev_ptr->dev_watchdog.dog.next = NULL;
    dev_ptr->dev_watchdog.dog.prev = NULL;
    dev_ptr->dev_watchdog.dog.func = p720_watchdog;
    dev_ptr->dev_watchdog.dog.count = 0;
    dev_ptr->dev_watchdog.dog.restart = 0;
    dev_ptr->dev_watchdog.save_time = 0;
    dev_ptr->dev_watchdog.timer_id = PSC_COMMAND_TMR;
    w_init(&(dev_ptr->dev_watchdog.dog));

    /* init command state flags                    */
    dev_ptr->queue_state = ACTIVE;
    dev_ptr->opened = TRUE;
    dev_ptr->device_state = NOTHING_IN_PROGRESS;

    DEBUG_1 ("Device pointer address = %x\n", dev_ptr)
    DEBUG_0 ("Leaving p720_start_dev routine.\n")
    return (0);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  p720_stop_dev                                                   */
/*                                                                        */
/* FUNCTION:  Stops a device and deallocates resources.                   */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to stop a device.  Any further command  */
/*         processing for the device will be halted from this point on.   */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      dev_index - index to device information                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - device not opened.                                         */
/*    EACCES - Adapter not opened in normal mode.                         */
/*    EIO - unable to pin code                                            */
/*                                                                        */
/**************************************************************************/
int
p720_stop_dev(
             int dev_index)
{
    int     i, old_level, TCE_index, id_hash;
    uchar last_lun;
    struct dev_info *dev_ptr;
    struct scsi_id_info *scsi_ptr;

    DEBUG_0 ("Entering p720_stop_dev routine.\n")

    if (adp_str.open_mode == DIAG_MODE)
    {   /* adapter opened in diagnostic mode */
        return (EACCES);
    }
    if ((dev_ptr = adp_str.device_queue_hash[dev_index]) == NULL)
    {   /* device queue structure never allocated */
        return (EINVAL);
    }
    /* Obtain device structure pointer and disable */
    /* interrupts for processing.                 */

    if (pincode(p720_stop_dev))
    {
        return (EIO);
    }

    old_level = i_disable(adp_str.ddi.int_prior);
    dev_ptr->queue_state |= STOPPING;

    /* Search all the queues of this device to determine if there are */
    /* any commands outstanding.  If so, wait for them to complete.   */
    while ((dev_ptr->active_head != NULL) || (dev_ptr->wait_head != NULL)
	|| (dev_ptr->flags & BLOCKED) || (dev_ptr->cmd_save_head != NULL) ||
	   (dev_ptr->bp_save_head != NULL)) 
    {
        e_sleep(&dev_ptr->stop_event, EVENT_SHORT);
    }
    i_enable(old_level);        /* let interrupts in */

    /* Free the device's script entry for other   */
    /* use, if all other LUNs are closed.         */
    dev_ptr->opened = FALSE;
    last_lun = TRUE;

    /* change this for 16 scsi ids */ 
    id_hash = (dev_ptr->scsi_id & 0x07) << 3;
    i = 0;
    while ((i < MAX_LUNS) && (last_lun == TRUE))
    {
	if ((adp_str.device_queue_hash[id_hash | i] != NULL) &&
	    (adp_str.device_queue_hash[id_hash | i]->opened))
	    last_lun = FALSE;
        i++;
    }

    if (last_lun)
    {
        scsi_ptr = dev_ptr->sid_ptr;
        /* script 0 is freed when the device is closed */
        if (dev_ptr->sid_ptr->script_index != 0)
        {
            old_level = i_disable(adp_str.ddi.int_prior);
            TCE_index = adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].
				TCE_index;
	    FREE(adp_str.TCE_alloc[TCE_index/PSC_WORDSIZE], TCE_index % 
				PSC_WORDSIZE)
            i_enable(old_level);    /* let interrupts in */
            adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].in_use = SCR_UNUSED;
            adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].dma_ptr = 0;
            adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].TCE_index = 0;
            (void) ltunpin(adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].
			script_ptr, PAGESIZE);
            (void) xmfree(adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].
			script_ptr, kernel_heap);
            adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].script_ptr = NULL;
        }
        else
        {
            adp_str.SCRIPTS[dev_ptr->sid_ptr->script_index].in_use = SCR_UNUSED;
        }
	scsi_ptr->dma_script_ptr = NULL;
    }
    /* Free the device's watchdog timer entry.   */
    w_clear(&(dev_ptr->dev_watchdog.dog));
    /* Free the device information table and clear */
    /* the hash table entry.                      */
    adp_str.device_queue_hash[dev_index] = NULL;
    (void) xmfree(dev_ptr, pinned_heap);
    (void) unpincode(p720_stop_dev);
    DEBUG_0 ("Leaving p720_stop_dev routine.\n")
    return (0);
}

/**************************************************************************/
/*                                                                        */
/* NAME:    p720_issue_abort                                              */
/*                                                                        */
/* FUNCTION:  Issues a SCSI abort to a device.                            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine causes an abort command to be issued to a SCSI    */
/*         device.  Note that this action will also halt the device queue.*/
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_index - index to device information                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    0 - for good completion, ERRNO value otherwise                      */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL    - device not opened.                                      */
/*    EACCES    - Adapter not opened in normal mode.                      */
/*    ETIMEDOUT - Command timed out.                                      */
/*    ENOCONNECT- SCSI bus fault.                                         */
/*    EIO       - Adapter error or unable to pin code.                    */
/*                                                                        */
/**************************************************************************/
int
p720_issue_abort(
                int dev_index)
{
    struct dev_info *dev_ptr;
    int     old_pri1;
    int     ret_code;

    DEBUG_0 ("Entering p720_issue_abort routine\n")
    if (adp_str.open_mode == DIAG_MODE)
    {
        DEBUG_0 ("Leaving p720_issue_abort routine with EACCES.\n")
        return (EACCES);
    }

    /* Obtain device structure pointer and disable */
    /* interrupts for processing.                 */
    ret_code = pincode(p720_issue_abort);
    if (ret_code != 0)
    {
        return (EIO);
    }
    old_pri1 = i_disable(adp_str.ddi.int_prior);

    if ((dev_ptr = adp_str.device_queue_hash[dev_index]) == NULL)
    {
        DEBUG_0 ("Leaving p720_issue_abort routine with EINVAL.\n")
        i_enable(old_pri1);     /* re-enable */
        (void) unpincode(p720_issue_abort);
        return (EINVAL);
    }

    TRACE_1("abort ioctl", (int) dev_ptr)

    if (dev_ptr->queue_state & STOPPING_or_HALTED_or_FLUSHING)
    {
        i_enable(old_pri1);     /* re-enable */
        (void) unpincode(p720_issue_abort);
        return (EIO);
    }

    /* Handle if device is already being BDRed or Aborted. */
    if (dev_ptr->flags & SCSI_BDR_or_ABORT)
    {
        i_enable(old_pri1);
        (void) unpincode(p720_issue_abort);
        return (EIO);
    }

    /* Handle if there are active commands for the lun */
    if (dev_ptr->active_head != NULL)
    {
        caddr_t eaddr;  /* effective address for pio */

        dev_ptr->flags |= SCSI_ABORT;
        p720_enq_abort_bdr(dev_ptr);

        /* stop chip if it is waiting for reselection */
	eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
	ret_code = p720_write_reg((uint) ISTAT, (uchar) ISTAT_SIZE, 
				   SIGP, eaddr);
  	BUSIO_DET(eaddr);

        if (ret_code == 0)
        {
	    dev_ptr->ioctl_wakeup = TRUE;
	    while (dev_ptr->ioctl_wakeup)
                e_sleep(&dev_ptr->ioctl_event, EVENT_SHORT);
            ret_code = dev_ptr->ioctl_errno;
	}
    }
    else /* just return all the pended commands */
    {
	p720_fail_cmd(dev_ptr);
    }

    DEBUG_1 ("Leaving p720_issue_abort routine. rc=0x%x\n", ret_code);
    i_enable(old_pri1); /* re-enable */
    (void) unpincode(p720_issue_abort);
    return (ret_code);  /* no error */
}  /* p720_issue_abort  */

/**************************************************************************/
/*                                                                        */
/* NAME:    p720_issue_BDR                                                */
/*                                                                        */
/* FUNCTION:  Issues a bus device reset to a scsi device.                 */
/*                                                                        */
/*      This performs actions necessary to send a SCSI bus                */
/*      device reset mesg to the scsi controller.                         */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine causes an reset command to be issued to a SCSI    */
/*         device.  Note that this action will also halt the device queue.*/
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adapter_def - adapter unique data structure                         */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_index - index to device information                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    0 - for good completion, ERRNO value otherwise                      */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL    - device not opened.                                      */
/*    EACCES    - Adapter not opened in normal mode.                      */
/*    ETIMEDOUT - Command timed out.                                      */
/*    ENOCONNECT- No connection.                                          */
/*    EIO       - Adapter error or unable to pin code.                    */
/*                                                                        */
/**************************************************************************/
int
p720_issue_BDR(
              int dev_index)
{
    struct dev_info *dev_ptr, *tmp_dev_ptr;
    struct cmd_info *com_ptr;
    int     old_pri1;
    int     i, ret_code;
    caddr_t eaddr;
    register int rc;


    DEBUG_0 ("Entering p720_issue_BDR routine.\n")
    if (adp_str.open_mode == DIAG_MODE)
    {
        DEBUG_0 ("Leaving p720_issue_BDR routine.\n")
        return (EACCES);
    }

    /* Obtain device structure pointer and disable */
    /* interrupts for processing.                 */
    i = pincode(p720_issue_BDR);
    if (i != 0)
    {
        return (EIO);
    }

    old_pri1 = i_disable(adp_str.ddi.int_prior);

    if ((dev_ptr = adp_str.device_queue_hash[dev_index]) == NULL)
    {
        i_enable(old_pri1);     /* re-enable */
        (void) unpincode(p720_issue_BDR);
        return (EINVAL);
    }

    TRACE_1("BDR ioctl  ", (int) dev_ptr)

    if (dev_ptr->queue_state & STOPPING_or_HALTED_or_FLUSHING)
    {
        i_enable(old_pri1);     /* re-enable */
        (void) unpincode(p720_issue_BDR);
        return (EIO);
    }

    /* Handle if device is already being BDRed. */
    if (dev_ptr->flags & SCSI_BDR)
    {
        i_enable(old_pri1);
        (void) unpincode(p720_issue_BDR);
        return (EIO);
    }

    /* We are going to issue the BDR.  Mark all the luns of the target */
    /* SCSI id, indicating that a BDR is going to be issued.	    */
    dev_index &= 0xF8;
    for (i = 0; i< MAX_LUNS; i++)
    {
        if ((tmp_dev_ptr = adp_str.device_queue_hash[dev_index + i]) != NULL)
	{
    	    tmp_dev_ptr->flags |= SCSI_BDR;
	    tmp_dev_ptr->queue_state &= STOPPING;
	    tmp_dev_ptr->queue_state |= WAIT_FLUSH;
	}
    }

    /* Indicate the BDR is based on this dev_ptr */
    dev_ptr->sid_ptr->bdring_lun = dev_ptr;

    /* get an effective address for pio */
    eaddr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);

    /* issue the command if the chip is idle */
    if (adp_str.DEVICE_ACTIVE_head == NULL)
    {
        /* start timer */
        dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
        w_start(&dev_ptr->dev_watchdog.dog);

        dev_ptr->device_state = BDR_IN_PROGRESS;
        dev_ptr->flags &= ~RETRY_ERROR;
        dev_ptr->retry_count = 1;

	com_ptr = &adp_str.command[adp_str.ddi.card_scsi_id << 3];

	com_ptr->bp = NULL;
	com_ptr->device = dev_ptr;
        com_ptr->resource_state = NO_RESOURCES_USED;
	com_ptr->in_use = TRUE;

        p720_enq_active(dev_ptr, com_ptr);

        /* issue the cmd */
        p720_issue_bdr_script((uint *) adp_str.SCRIPTS[dev_ptr->sid_ptr->
			script_index].script_ptr, (uint *) dev_ptr->sid_ptr->
			dma_script_ptr, dev_index, com_ptr, FALSE, eaddr);
    }
    else
    {
        p720_enq_abort_bdr(dev_ptr);
 
	TRACE_1 ("sigp bdr   ", rc)
        /* stop chip if it is waiting for reselection */
        (void) p720_write_reg(ISTAT, ISTAT_SIZE, SIGP, eaddr);
    }
    BUSIO_DET(eaddr);
 
    dev_ptr->ioctl_wakeup = TRUE;
    while (dev_ptr->ioctl_wakeup)
        e_sleep(&dev_ptr->ioctl_event, EVENT_SHORT);
    ret_code = dev_ptr->ioctl_errno;
    i_enable(old_pri1); /* re-enable */
    (void) unpincode(p720_issue_BDR);
    DEBUG_1 ("Leaving p720_issue_BDR routine. rc=0x%x\n", ret_code);
    return (ret_code);
}  /* p720_issue_BDR */
