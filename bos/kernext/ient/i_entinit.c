static char sccsid[] = "@(#)41  1.9  src/bos/kernext/ient/i_entinit.c, sysxient, bos411, 9438B411a 9/21/94 20:54:18";
/*****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS:
 *              ient_init
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************/
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>

#include <sys/cdli.h>
#include <sys/generic_mibs.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>

#include "i_entdds.h"
#include "i_entmac.h"
#include "i_enthw.h"
#include "i_entsupp.h"
#include "i_ent.h"

extern ient_dev_ctl_t *p_dev_ctl;


/*****************************************************************************/
/*
 * NAME: ient_init
 *
 * FUNCTION: Device specific initialization of the dds.
 *
 * EXECUTION ENVIRONMENT:
 *
 *           This routine runs only under the process thread.
 *
 * NOTES:
 *
 * CALLED FROM:
 *              config_init
 *
 * CALLS TO:
 *
 * INPUT: none
 *
 * RETURN:  0 = always
 */
/*****************************************************************************/

int
ient_init()
{
    ulong   ioa;                    /* IO handle */
    uchar   pos_id0, pos_id1;

    TRACE_BOTH(HKWD_IENT_OTHER, "CinB", (ulong)p_dev_ctl, 0, 0);

    ioa = (ulong) io_att((DDS.bus_id | IOCC_SELECT), 0);
    ioa = ioa + POS_REG_OFFSET + (DDS.slot << 16);

    BUS_GETCX((caddr_t) ioa, &pos_id0);
    BUS_GETCX((caddr_t) (ioa + 1), &pos_id1);

    WRK.do_ca_on_intr = 0;

    switch(pos_id0)
    {
        case PID_LSB_IO_BASED:            /* Stillwell */
        {
            WRK.machine = MACH_IO_BASED;
            WRK.aram_mask = 0xFFF;        /* generate ASIC RAM addresses */
            break;
        }

        case PID_LSB_MEM_BASED:           /* Salmon */
        {
            WRK.machine = MACH_MEM_BASED;
            WRK.aram_mask = 0xFFFFFFFF;
            break;
        } 
        case PID_LSB_RAINBOW:
        {
            WRK.machine = MACH_MEM_BASED; /* Rainbow. */
            WRK.aram_mask = 0xFFFFFFFF;
            /*
            ** We do a channel_attention for every intr.
            ** this will allow us to get the mc-intr reset.
            */
            WRK.do_ca_on_intr |= IS_PPC_BASED;
            break;
        }
        default:
        {
            TRACE_SYS(HKWD_IENT_OTHER, "Cin1", (ulong)p_dev_ctl, pos_id0, 0);
            return(EIO);
        }
    }
    io_det(ioa);

    WRK.pos_reg[POS_0] = pos_id0;
    WRK.pos_reg[POS_1] = pos_id1;

    /*
    **  this routine will get the configured values to set-up the device
    **  driver workspace.  The configuration values were passed from
    **  the config methods. Divide the allocated bus memory between
    **  the Shared Memory Structure (SMS), the xmit area, the action
    **  command area and the receive area
    */

    WRK.dma_base = (ulong) DDS.tcw_bus_mem_addr;  /* bus mem base addr */

    /* Put Network Address in DDS  */
    COPY_NADR(DDS.eth_addr, WRK.ent_addr);

    WRK.channel_allocated = FALSE;          /* got a DMA channel */
    WRK.alt_count = 0;
    WRK.multi_count = 0;
    WRK.promiscuous_count = 0;
    WRK.badframe_user_count = 0;
    WRK.lock_anchor = LOCK_AVAIL;

    TRACE_BOTH(HKWD_IENT_OTHER, "CinE", (ulong)p_dev_ctl, pos_id0, pos_id1);
    return(0);
}
