static char sccsid[] = "@(#)35  1.14  src/bos/kernext/ient/i_entcfg.c, sysxient, bos41B, 412_41B_sync 12/8/94 14:24:28";
/*************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS:
 *              ient_config
 *              config_init
 *              config_remove
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
/*************************************************************************/
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/dump.h>
#include <sys/adspace.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/mbuf.h>
#include <sys/err_rec.h>
#include <sys/trcmacros.h>
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

extern int ient_open();
extern int ient_close();
extern int ient_ioctl();
extern int ient_output();
extern int ient_slih();

extern void ient_timeout();

/*************************************************************************/
/*  Global data structures                                               */
/*************************************************************************/

/*
** This is the master control structure for the Integrated Ethernet
** Device Driver.
*/
ient_dev_ctl_t *p_dev_ctl;

/*
** The following structure is the internal trace table.
*/
ient_trace_t ient_trace_tbl =
{
             0, 
             0x49454e54,                       /* "IENT" */
             0x54524143,                       /* "TRAC" */
             0x4554424c,                       /* "ETBL" */
             IENT_TRACE_END
};

/*
**  MIB status table - this table defines the MIB variable status returned
**  on MIB query operation.
*/
ethernet_all_mib_t ient_mib_status = {

    /* Generic Interface Extension Table */
    MIB_READ_ONLY,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,     /* ifExtnsChipSet */
    MIB_NOT_SUPPORTED,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* ifExtnsRevWare */
    MIB_READ_ONLY,                  /* ifExtnsMulticastsTransmittedOks */
    MIB_READ_ONLY,                  /* ifExtnsBroadcastsTransmittedOks */
    MIB_READ_ONLY,                  /* ifExtnsMulticastsReceivedOks */
    MIB_READ_ONLY,                  /* ifExtnsBroadcastsReceivedOks */
    MIB_READ_ONLY,                  /* ifExtnsPromiscuous */

    /* Generic Interface Test Table */
    MIB_NOT_SUPPORTED,              /* ifEXtnsTestCommunity */
    MIB_NOT_SUPPORTED,              /* ifEXtnsTestRequestId */
    MIB_NOT_SUPPORTED,              /* ifEXtnsTestType */
    MIB_NOT_SUPPORTED,              /* ifEXtnsTestResult */
    MIB_NOT_SUPPORTED,              /* ifEXtnsTestCode */

    /* Generic Receive Address Table */
    MIB_READ_ONLY,                  /* RcvAddrTable */

    /* Ethernet-like Statistics group */
    MIB_READ_ONLY,                  /* dot3StatsAlignmentErrors */
    MIB_READ_ONLY,                  /* dot3StatsFCSErrors */
    MIB_READ_ONLY,                  /* dot3StatsSingleCollisionFrames */
    MIB_READ_ONLY,                  /* dot3StatsMultipleCollisionFrames */
    MIB_NOT_SUPPORTED,              /* dot3StatsSQETestErrors */
    MIB_READ_ONLY,                  /* dot3StatsDeferredTransmissions */
    MIB_READ_ONLY,                  /* dot3StatsLateCollisions */
    MIB_READ_ONLY,                  /* dot3StatsExcessiveCollisions */
    MIB_READ_ONLY,                  /* dot3StatsInternalMacTransmitErrors */
    MIB_READ_ONLY,                  /* dot3StatsCarrierSenseErrors */
    MIB_READ_ONLY,                  /* dot3StatsFrameTooLongs */
    MIB_READ_ONLY,                  /* dot3StatsInternalMacReceiveErrors */

    /* Ethernet-like Collision Statistics group */
    MIB_NOT_SUPPORTED,              /* dot3CollCount */
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_NOT_SUPPORTED,
    MIB_READ_ONLY,                 /* dot3CollFrequencies */
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY,
    MIB_READ_ONLY
};

/*****************************************************************************/
/*
 * NAME:     ient_config
 *
 * FUNCTION: Configuration entry point of the device driver
 *
 * EXECUTION ENVIRONMENT: Process thread only.
 *
 * NOTES:
 *
 * CALLED FROM: Configuration routine for the Integrated Ethernet Device.
 *              Also the entry point for this device driver.
 *
 * CALLS:  bzero
 *         config_init
 *         copyin
 *         ns_detach
 *         uiomove
 *         xmalloc
 *         xmfree
 *         strcpy
 *
 * INPUT:  cmd       - CFG_INIT, CFG_TERM, CFG_QVPD
 *         p_uio     - uio struct with ndd_config_t structure
 *
 * RETURNS:  0 (Success) or errno
 */
/*****************************************************************************/

int
ient_config(int cmd, struct uio *p_uio)
{

    int rc = 0;                         /* return code */
    int i;                              /* loop index */
    ndd_config_t  ndd_config;           /* config information */


    TRACE_BOTH(HKWD_IENT_OTHER, "CfgB", cmd, (ulong)p_uio, p_dev_ctl);

    if (p_uio->uio_resid != sizeof(ndd_config_t))
    {
        TRACE_SYS(HKWD_IENT_OTHER, "Cfg1", EINVAL, p_uio->uio_resid, 0);
        return(EINVAL);
    }

    if (uiomove((caddr_t) &ndd_config, sizeof(ndd_config_t), UIO_WRITE, p_uio))
    {
        TRACE_SYS(HKWD_IENT_OTHER, "Cfg2", EFAULT, &ndd_config, 0);
        return(EFAULT);
    }

    switch(cmd) {

    /*
    ** Initialize the Integrated Ethernet Device Driver
    */
    case CFG_INIT:

        if (p_dev_ctl != (ient_dev_ctl_t *) NULL)
        {
            TRACE_SYS(HKWD_IENT_OTHER, "Cfg3", EBUSY, p_dev_ctl, 0);
            return(EBUSY);
        }

        /*
        ** Allocate memory for the dev_ctl structure
        */
        p_dev_ctl = (ient_dev_ctl_t *) xmalloc(sizeof(ient_dev_ctl_t), PGSHIFT,
                                               pinned_heap);

        if (p_dev_ctl == (ient_dev_ctl_t *) NULL)
        {
            TRACE_SYS(HKWD_IENT_OTHER, "Cfg4", ENOMEM, p_dev_ctl, 0);
            return(ENOMEM);
        }

        bzero(p_dev_ctl, sizeof(ient_dev_ctl_t));


        /*
        **  Copy in the dds for config manager
        */

        if (copyin(ndd_config.dds, &p_dev_ctl->dds_section, sizeof(ient_dds_t)))
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Cfg5", EIO, &p_dev_ctl->dds_section, 0);
            xmfree((caddr_t) p_dev_ctl, pinned_heap);
            rc = EIO;
            break;
        }

        p_dev_ctl->seq_number = ndd_config.seq_number;

        /*
        **  Setup the component dump table
        */

        CDT.head._cdt_magic = DMP_MAGIC;
        strcpy(CDT.head._cdt_name, IENT_DD_NAME);
        CDT.head._cdt_len = sizeof(struct cdt_head);
        CDT.entry[0].d_len = 0;
        CDT.entry[0].d_ptr = NULL;

        if ((rc = ConfigInit()) != 0)
        {
            xmfree((caddr_t)p_dev_ctl, pinned_heap);  /* Free the resouces. */
            p_dev_ctl = (ient_dev_ctl_t *) NULL;
        }
        else
        {
            p_dev_ctl->device_state = CLOSED;
        } 
        break;
       

    /*
    ** Terminate the Integrated Ethernet Device Driver
    */

    case CFG_TERM:

        /* Does the device exist? */
        if (p_dev_ctl == (ient_dev_ctl_t *) NULL)
        { 
            TRACE_BOTH(HKWD_IENT_OTHER, "Cfg6", ENODEV, p_dev_ctl, 0);
            return(ENODEV);
        }


        /*
        ** Make sure the device is in CLOSED or DEAD state.
        ** Call ns_detach and make sure that it is done
        ** without error.
        */
        if ((p_dev_ctl->device_state != CLOSED &&
             p_dev_ctl->device_state != DEAD) || ns_detach(&(NDD)))
        {
            TRACE_BOTH(HKWD_IENT_ERR, "Cfg7", EBUSY, p_dev_ctl->device_state,
                       IHS.bus_type);
            rc = EBUSY;
        }
        else
        {
            /*
            ** Free the resouces.
            */
            xmfree((caddr_t)p_dev_ctl, pinned_heap);
            p_dev_ctl = (ient_dev_ctl_t *) NULL;
        }
        break;

    case CFG_QVPD:

        TRACE_BOTH(HKWD_IENT_OTHER, "Cfg8", cmd, EOPNOTSUPP, (ulong)p_uio);
        rc = EOPNOTSUPP;                  /* QVPD command is not supported. */
        break;

    default:

        TRACE_SYS(HKWD_IENT_OTHER, "Cfg9", cmd, EINVAL, (ulong)p_uio);
        rc = EINVAL;
    }
    TRACE_BOTH(HKWD_IENT_OTHER, "CfgE", rc, 0, 0);
    return(rc);
}

/*****************************************************************************/
/*
 * NAME:     ConfigInit
 *
 * FUNCTION: perform CFG_INIT function. Initialize the dds and the ndd.
 *
 * EXECUTION ENVIRONMENT: Process thread only
 *
 * NOTES:
 *
 * CALLED FROM: ient_config
 *
 * CALLS:  ient_init
 *         ns_attach
 *
 * INPUT:  none
 *
 * RETURNS:  0 (success) or errno
 *
 */
/*****************************************************************************/

int
ConfigInit()
{

    int rc = 0;           /* return code */
    int i;

    TRACE_BOTH(HKWD_IENT_OTHER, "CciB", (ulong)p_dev_ctl, 0, 0);

    /* ensure that the xmt_que_size is a multiple of XMIT_BUFFERS */
    if((DDS.xmt_que_size % XMIT_BUFFERS) != 0)
        DDS.xmt_que_size += XMIT_BUFFERS - (DDS.xmt_que_size % XMIT_BUFFERS);

    /*
    ** We have to guarantee that the RT compatability bit is not set.
    ** Even though it is ignored by RS1 and RS2, PPC (rainbow) will lock
    ** up if it is set.  Here we will extract the Bus ID and then OR in the
    ** appropriate bits later as needed.
    */
    DDS.bus_id = (DDS.bus_id & 0x0FF00000) | 0x80000000;

    /* set up the interrupt control structure section */
    IHS.next     = (struct intr *) NULL;
    IHS.handler  = ient_slih;
    IHS.bus_type = DDS.bus_type;
    IHS.flags    = INTR_NOT_SHARED;
    IHS.level    = DDS.intr_level;
    IHS.priority = DDS.intr_priority;
    IHS.bid      = DDS.bus_id | BUSMEM_IO_SELECT;

    /* set up the watchdog timer control structure section */
    WDT.next     = NULL;
    WDT.prev     = NULL;
    WDT.func     = ient_timeout;
    WDT.restart  = 5;                      /* Watchdog pops in 5 seconds */
    WDT.count    = 0;

    NDD.ndd_name  = (caddr_t) DDS.lname;
    NDD.ndd_alias = (caddr_t) DDS.alias;

    /* put the dd_ctl address here */
    NDD.ndd_correlator = (caddr_t) p_dev_ctl;
    NDD.ndd_addrlen    = ENT_NADR_LENGTH;
    NDD.ndd_hdrlen     = ENT_HDR_LEN;
    NDD.ndd_physaddr   = (caddr_t) WRK.ent_addr;

    NDD.ndd_mtu        = ENT_MAX_MTU;
    NDD.ndd_mintu      = ENT_MIN_MTU;

    NDD.ndd_type = NDD_ISO88023;

#ifdef BEN_TEST
    NDD.ndd_type = NDD_ETHER;
#endif

    NDD.ndd_flags = NDD_UP | NDD_BROADCAST | NDD_SIMPLEX;

#ifdef DEBUG
    NDD.ndd_flags |= NDD_DEBUG;
#endif

    NDD.ndd_open   = (int(*) ())ient_open;
    NDD.ndd_output = (int(*) ())ient_output;
    NDD.ndd_ctl    = (int(*) ())ient_ioctl;
    NDD.ndd_close  = (int(*) ())ient_close;
    NDD.ndd_specstats = (caddr_t) &(ENTSTATS);
    NDD.ndd_speclen   = sizeof(ENTSTATS);

    /* perform device-specific initialization of dds */
    /* if this routine returns non-zero, the device can't be configured */
    
    if ((rc = ient_init()) != 0)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Cci1", rc, p_dev_ctl, 0);
    }
    else if ((rc = ns_attach(&NDD)) != 0)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Cci2", rc, &NDD, 0);
    }
    
    TRACE_BOTH(HKWD_IENT_OTHER, "CciE", rc, DDS.bus_type, IHS.bus_type);
    return(rc);
}
