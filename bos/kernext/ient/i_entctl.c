static char sccsid[] = "@(#)37	1.12  src/bos/kernext/ient/i_entctl.c, sysxient, bos41J, 9519A_all 5/8/95 14:50:18";
/*****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS:
 *              ient_ioctl
 *              ient_getmib
 *              ient_getstat
 *              multi_add
 *              multi_del
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

#include <stddef.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
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

extern int ient_dump(ndd_t *p_ndd, int cmd, caddr_t arg);
uchar ent_broad_adr[ENT_NADR_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
extern ethernet_all_mib_t ient_mib_status;
extern ient_dev_ctl_t *p_dev_ctl;


/*****************************************************************************/
/*
 * NAME:     ient_ioctl
 *
 * FUNCTION:  IOCTL entry point for the Integrated Ethernet Device Driver
 *
 * EXECUTION ENVIRONMENT: Process thread only.
 *
 * NOTES:
 *
 * CALLED FROM: NS user by using the ndd_ctl field in the NDD on the NDD chain.
 *
 * CALLS TO:    bcopy
 *              bzero
 *              COPY_NADR
 *              delay
 *              ient_action  (i_entopen.c)
 *              ient_getmib
 *              ient_getstat
 *              lockl
 *              multi_add
 *              multi_del
 *              unlockl
 *
 * INPUTS:   p_ndd       - pointer to the ndd.
 *           cmd         - control command
 *           arg         - pointer to the argument for the control command
 *           length      - length of the argument.
 *
 * RETURNS:  0 (success) or errno
 *
 */
/*****************************************************************************/

int
ient_ioctl(ndd_t *p_ndd, int cmd, caddr_t arg, int length)
{
    int rc = 0;
    int   ElemLen, i, count = 0;
    ulong io_addr;
    nad_t *MultiPtr;
    uchar save_cfg_val;
    ndd_mib_addr_t       *TablePtr;
    ndd_mib_addr_elem_t  *ElemPtr;
    ns_com_status_t      *StatPtr;

    TRACE_BOTH(HKWD_IENT_OTHER, "IctB", length, cmd, (ulong)arg);

    if ((cmd == NDD_ADD_FILTER) || (cmd == NDD_DEL_FILTER)) {
        TRACE_BOTH(HKWD_IENT_OTHER, "IctE", EOPNOTSUPP, 0, 0);
	return(EOPNOTSUPP);
    }


    /*
    ** This driver does not issue concurrent action commands
    ** Wait till the interrupt for any prior command is received.
    ** NOTE: We can use a delay here only if function is called
    ** with interrupts enabled, otherwisel, just loop.
    */

    while ((WRK.control_pending != FALSE)&&(p_dev_ctl->device_state == OPENED))
        delay(1);

    switch (p_dev_ctl->device_state)
    {
      case OPENED:
          break;

      case DEAD:                  /* the LSA has cratered      */
      {
          TRACE_BOTH(HKWD_IENT_ERR,"Ict1", p_dev_ctl->device_state, ENETDOWN, 0);
          return(ENETDOWN);
      }

      case LIMBO:                 /* LSA is in error recovery  */
      {
          TRACE_BOTH(HKWD_IENT_ERR, "Ict2", p_dev_ctl->device_state,
                     ENETUNREACH, 0);
          return(ENETUNREACH);
      }

      default:                    /* should never happen       */
      {
          TRACE_BOTH(HKWD_IENT_ERR, "Ict3", p_dev_ctl->device_state, EINVAL, 0);
          return(EINVAL);
      }
    }

    /*
    ** Serialize access to this routine by the use of a lock.
    */

    if ((rc = lockl((int *)&WRK.lock_anchor, LOCK_SIGRET)) != LOCK_SUCC)
    {
        TRACE_BOTH(HKWD_IENT_ERR, "Ict4", p_dev_ctl->device_state, rc, 0);
        return(rc);
    }

    switch(cmd)
    {
      /*
      ** The following command obtains the generic and ethernet specific
      ** statistics for the device.
      */

      case NDD_GET_STATS:
      {
          if (length != sizeof(ent_ndd_stats_t))
          {
              rc = EINVAL;
              break;
          }
  
          (void) ient_getstat();
  
          NDD.ndd_genstats.ndd_elapsed_time = ((lbolt - WRK.ndd_stime) / HZ);
          ENTSTATS.dev_elapsed_time = ((lbolt - WRK.dev_stime) / HZ);
  
  
          /* copy statistics to user's buffer */
          bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
          bcopy(&(ENTSTATS), arg + sizeof(ndd_genstats_t),
                sizeof(ent_genstats_t));
  
          break;
      }
  
      /*
      ** The following retrieves all the statistics for the device.  This
      ** includes generic, ethernet and integrated ethernet stats.
      */

      case NDD_GET_ALL_STATS:
      {
          if (length != sizeof(ient_all_stats_t))
          {
              rc = EINVAL;
              break;
          }
  
          (void) ient_getstat();
  
          NDD.ndd_genstats.ndd_elapsed_time = ((lbolt - WRK.ndd_stime) / HZ);
          ENTSTATS.dev_elapsed_time = ((lbolt - WRK.dev_stime) / HZ);
  
          bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));

          bcopy(&(ENTSTATS), arg + sizeof(ndd_genstats_t),
                sizeof(ent_genstats_t) + sizeof(ient_stats_t));
  
          break;
      }
  
  
      /*
      ** This command will clear all of the statistics for this device.
      */

      case NDD_CLEAR_STATS:
      {
          /*
          ** Reset the start time for both ndd and device
          */
          WRK.ndd_stime = WRK.dev_stime = lbolt;
  
          /*
          ** Retrieve all stats from the SCB.  This resets the SCB stats
          ** to zero.
          */
          
          (void) ient_getstat();
  
          bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));
          bzero(&ENTSTATS, sizeof(struct ent_genstats));
          bzero(&DEVSTATS, sizeof(struct ient_stats));
          bzero(&MIB, sizeof(ethernet_all_mib_t));
          ENTSTATS.device_type = ENT_IENT;
          bcopy (ETH_MIB_Intel82596, MIB.Generic_mib.ifExtnsEntry.chipset,
                 CHIPSETLENGTH);
  
          break;
      }
  
      /*
      ** Enable the adapter to receive all of the multicast packets on the
      ** network. The driver will switch to the all-multicast mode in order to
      ** support this function.
      */

    case NDD_ENABLE_MULTICAST:
      {
          WRK.multi_count++;     /* inc the reference counter */
  
        /*
        ** When we go to multicast mode we have to reconfigure the
        ** Intel 82596.
        */

        if (WRK.multi_count == 1)
        {
            /* Config the adapter to receive all multi packets */
            save_cfg_val = WRK.cur_cfg.preamble;
            WRK.cur_cfg.preamble = CFG_PREAMBLE & 0xDF;

            rc = ient_action(INTERNAL_CFG_CMD, FALSE);
            if (!(rc))
            {
                p_ndd->ndd_flags |= NDD_MULTICAST;
            }
            else                   /* Failed, restore initial values. */
            {
              if (WRK.multi_count) WRK.multi_count--;
              WRK.cur_cfg.preamble = save_cfg_val;
            }
        }
        break;
      }
  
      /*
      ** Disable the all multicast function. The adapter will go back to
      ** use the multicast filter if this is the last disable_multicast
      ** operation. If there is no other reason to stay in the multicast
      ** mode, the adapter will be re-configed to be out of the multicast
      ** mode.
      */
      
      case NDD_DISABLE_MULTICAST:
      {
          if (!WRK.multi_count)
          {
              rc = EINVAL;
              break;
          }
  
          WRK.multi_count--;

          /*
          ** If multi_count goes to zero we have to reconfigure the Intel
          ** 82596 to its normal operation mode, CFG_PREAMBLE.
          */

          if (!WRK.multi_count)
          {
              save_cfg_val = WRK.cur_cfg.preamble;
              WRK.cur_cfg.preamble = CFG_PREAMBLE;

            rc = ient_action(INTERNAL_CFG_CMD, FALSE);
            if (!(rc))
            {
                p_ndd->ndd_flags &= ~NDD_MULTICAST;
            }
            else                   /* Failed, restore initial values. */
            {
                WRK.multi_count++;
                WRK.cur_cfg.preamble = save_cfg_val;
            }
          }
          break;
      }
  
      /*
      ** Enable the promiscuous mode. If this is the first promiscuous on
      ** operation and the adapter is not in promiscuous mode already, the
      ** driver will config the adapter to run in the promiscuous mode.
      */
  
      case NDD_PROMISCUOUS_ON:
      {
          WRK.promiscuous_count++;        /* inc the reference counter */

          if (WRK.promiscuous_count == 1)
          {
              save_cfg_val = WRK.cur_cfg.promiscuous;
              WRK.cur_cfg.promiscuous = 1;

              rc = ient_action(INTERNAL_CFG_CMD, FALSE);
              if (!(rc))
              {
                  p_ndd->ndd_flags |= NDD_PROMISC;
                  MIB.Generic_mib.ifExtnsEntry.promiscuous = PROMTRUE;
              }
              else                   /* Failed, restore initial values. */
              {
                  WRK.promiscuous_count--;
                  WRK.cur_cfg.promiscuous = save_cfg_val;
              }
          }
          break;
      }
  
      /*
      ** Disable the promiscuous mode. If this is the last promiscuous off
      ** operation and there is no other reason to stay in the promiscuous mode,
      ** the adapter will be re-configed to be out of the promiscuous mode.
      */
      case NDD_PROMISCUOUS_OFF:
      {
          if (!WRK.promiscuous_count)
          {
              rc = EINVAL;
              break;
          }
  
          WRK.promiscuous_count--;        /* dev the reference counter */

          if (!WRK.promiscuous_count)
          {
              save_cfg_val = WRK.cur_cfg.promiscuous;
              WRK.cur_cfg.promiscuous = CFG_PROMISCUOUS;

              rc = ient_action(INTERNAL_CFG_CMD, FALSE);
              if (!(rc))
              {
                  MIB.Generic_mib.ifExtnsEntry.promiscuous = PROMFALSE;
                  p_ndd->ndd_flags &= ~NDD_PROMISC;
              }
              else                   /* Failed, restore initial values. */
              {
                  WRK.promiscuous_count++;
                  WRK.cur_cfg.promiscuous = save_cfg_val;
              }
          }
          break;
      }
  
      /*
      ** Add a filter. If this is the first add filter operation, enable
      ** the adapter to receive packets by setting a NULL filter on the adapter.
      ** The NULL filter is telling the adapter to give the driver all the
      ** packets that it receives without doing any type filtering, the demuxer
      ** is going to do that job, either the device driver or the adapter won't
      ** keep track of the filter anymore.
      */
      
      case NDD_ADD_FILTER:
      {
          TRACE_BOTH(HKWD_IENT_ERR, "Ict5", EOPNOTSUPP, 0, 0);
          rc=EOPNOTSUPP;
          break;
      }
  
      /*
      ** Delete a filter. If this is the last filter get deleted, the driver
      ** will set a BAD filter on the adapter to disable receiving any more
      ** packets.
      */
      case NDD_DEL_FILTER:
      {
          TRACE_BOTH(HKWD_IENT_ERR, "Ict6", EOPNOTSUPP, 0, 0);
          rc = EOPNOTSUPP;
          break;
      }
  
      /*
      ** Query the MIB support status on the driver.
      */
      
      case NDD_MIB_QUERY:
      {
          if (length != sizeof(ethernet_all_mib_t))
          {
              rc = EINVAL;
              break;
          }
  
          /* copy status to user's buffer */
          bcopy(&ient_mib_status, arg, sizeof(ethernet_all_mib_t));
          break;
  
      }
  
      /*
      ** Get all MIB values.
      */
      case NDD_MIB_GET:
      {
          if (length != sizeof(ethernet_all_mib_t))
          {
              rc = EINVAL;
              break;
          }
  
          (void) ient_getstat();
          (void) ient_getmib();
  
          /* copy mibs to user's buffer */
          bcopy(&MIB, arg, sizeof(ethernet_all_mib_t));
  
          break;
      }
  
      /*
      ** Get receive address table (mainly for MIB variables).
      ** The receive address table consists of all the addresses that the
      ** adapter is armed to receive packets with. It includes the host
      ** network address, the broadcast address and the currently registered
      ** multicast addresses. This operation doesn't report the device state.
      */
      
      case NDD_MIB_ADDR:
      {
          if (length < sizeof(ndd_mib_addr_t))
          {
              rc = EINVAL;
              break;
          }
          
          TablePtr = (ndd_mib_addr_t *) arg;
          MultiPtr = (nad_t *) WRK.alt_list;
          ElemLen = sizeof(ndd_mib_addr_elem_t) + ENT_NADR_LENGTH - 2;

          length -= sizeof(u_int);          /* room for count field */
          arg += 4;
  
          /* copy the specific network address in use first */
          
          if (length >= ElemLen)
          {
              ElemPtr = (ndd_mib_addr_elem_t *) arg;
              ElemPtr->status = NDD_MIB_VOLATILE;
              ElemPtr->addresslen = ENT_NADR_LENGTH;
              COPY_NADR(WRK.ent_addr, ElemPtr->address);
              length -= ElemLen;
              arg += ElemLen;
              count++;
          }
          else
          {
              rc = E2BIG;
              break;
          }
  
          /* copy the broadcast address */
          
          if (length >= ElemLen)
          {
              ElemPtr = (ndd_mib_addr_elem_t *) arg;
              ElemPtr->status = NDD_MIB_NONVOLATILE;
              ElemPtr->addresslen = ENT_NADR_LENGTH;
              COPY_NADR(ent_broad_adr, ElemPtr->address);
              length -= ElemLen;
              arg += ElemLen;
              count++;
          }
          else
          {
              rc = E2BIG;
              break;
          }
          
          while ((ulong) MultiPtr)
          {
              if (length >= ElemLen)
              {
                  ElemPtr = (ndd_mib_addr_elem_t *) arg;
                  ElemPtr->status = NDD_MIB_VOLATILE;
                  ElemPtr->addresslen = ENT_NADR_LENGTH;
                  COPY_NADR(MultiPtr->nadr, ElemPtr->address);
                  length -= ElemLen;
                  arg += ElemLen;
                  count++;
              }
              else
              {
                  rc = E2BIG;
              }

              MultiPtr = (nad_t *) MultiPtr->link;
          }

          /* put the final count into the buffer */

          TablePtr->count = count;
  
          break;
      }
  
      /*
      ** Add an asynchronous status filter. The integrated driver only
      ** tracks the NDD_BAD_PKTS status filter. If this is the first time
      ** the bad packets status is added, the driver will config the adapter
      ** to start receiving bad packets on the network.
      */
  
      case NDD_ADD_STATUS:
      {
  
          if (length < sizeof(ns_com_status_t))
          {
              rc = EINVAL;
              break;
          }
          
          StatPtr = (ns_com_status_t *) arg;

          if (StatPtr->filtertype == NS_STATUS_MASK)
          {
              if (StatPtr->mask & NDD_BAD_PKTS)
              {
                  WRK.badframe_user_count++;
                  
                  if (WRK.badframe_user_count == 1)
                  {
                      save_cfg_val = WRK.cur_cfg.save_bf;
                      WRK.cur_cfg.save_bf = CFG_BAD_FRAMES + 0x80;

                      rc = ient_action(INTERNAL_CFG_CMD, FALSE);
                      if (!(rc))
                      {
                          p_ndd->ndd_flags |= ENT_RCV_BAD_FRAME;
                      }
                      else             /* Failed, restore initial values. */
                      {
                          WRK.badframe_user_count--;
                          WRK.cur_cfg.save_bf = save_cfg_val;
                      }
                  }
              }
          }
          else
          {
              rc = EINVAL;
          }
          break;
      }
  
      /*
      ** Delete an asynchronous status filter. The driver only tracks the
      ** NDD_BAD_PKTS status filter. If this is the last time the bad packets
      ** status is deleted, the driver will config the adapter to stop receiving
      ** bad packets on the network.
      */
      
      case NDD_DEL_STATUS:
      {
          if (length < sizeof(ns_com_status_t))
          {
              rc = EINVAL;
              break;
          }

          StatPtr = (ns_com_status_t *) arg;

          if (!WRK.badframe_user_count)
          {
              rc = EINVAL;
              break;
          }
          
          if (StatPtr->filtertype == NS_STATUS_MASK)
          {
              if (StatPtr->mask & NDD_BAD_PKTS)
              {
                  WRK.badframe_user_count--;
                  if (!WRK.badframe_user_count)
                  {
                      save_cfg_val = WRK.cur_cfg.save_bf;
                      WRK.cur_cfg.save_bf = CFG_BAD_FRAMES;
                      rc = ient_action(INTERNAL_CFG_CMD, FALSE);
                      if (!(rc))
                      {
                          p_ndd->ndd_flags &= ~ENT_RCV_BAD_FRAME;
                      }
                      else             /* Failed, restore initial values. */
                      {
                          WRK.badframe_user_count++;
                          WRK.cur_cfg.save_bf = save_cfg_val;
                      }
                  }
              }
          }
          else
          {
              rc = EINVAL;
          }

          break;
      }
  
      /*
      ** Add a multicast address to the multicast filter. The adapter only
      ** support a filter of n multicast addresses. When there are less than
      ** n multicast addresses in the table, update the adapter's multicast
      ** filter as required. When there are more than n multicast addresses in
      ** the table, the driver will enable the all multicast mode on the adapter
      ** in order to get all of the multicast packets on the network.
      */
      
      case NDD_ENABLE_ADDRESS:
      {
          if (length == ENT_NADR_LENGTH)
          {
              if (multi_add(arg) == 0)
              {
                  /*
                  ** Address has been successfully added to the table and the
                  ** adapter has been configured accordingly.
                  */
                  p_ndd->ndd_flags |= NDD_ALTADDRS;
                  break;
              }
          }
  
          rc = EINVAL;
          break;
      }
  
      /*
      ** Delete a multicast address from the multicast filter. The adapter only
      ** support a filter of n multicast addresses. When there are more than n
      ** multicast addresses in the table, the adapter will stay in the
      ** all-multicast mode as mentioned above. When there are fewer than n
      ** multicast addresses left in the table, the driver will update the
      ** adapter's multicast filter as required and disable the all-multicast
      ** mode on the adapter.
      */
  
      case NDD_DISABLE_ADDRESS:
      {
          /*
          ** If not found and deleted from the table, return error.
          ** There is no need to check for a valid multicast address
          ** because only valid multicast addresses will ever be added
          ** to the table.
          */
          if (length == ENT_NADR_LENGTH)
          {
              if (multi_del(arg) == 0)
              {
                  /*
                  ** Address has been successfully deleted and the
                  **  adapter configured accordingly
                  */
                  if (WRK.alt_count == 0)
                      p_ndd->ndd_flags &= ~NDD_ALTADDRS;
                  break;
              }
          }
          /*
          ** If we fall through to here either the length is less than an
          ** legal address length, or the address was not found in the table.
          */
  
          rc = EINVAL;
          break;
      }
  
      /*
      ** Setup the NDD with a pointer to the function that handles remote
      ** dumps.
      */
      
      case NDD_DUMP_ADDR:
      {
          if (arg == 0)
          {
              rc = EINVAL;
              break;
          }
          *(uint *)arg = (uint) ient_dump;
          break;
      }
  
      /*
      ** The following is for the manufacturing test group.  For the Stilwell
      ** class machine they can tell the type of riser card installed in the
      ** box.
      */

      case MFG_TEST_DATA:
      {
          if (WRK.machine != MACH_MEM_BASED)
          {
              if (arg == 0)
              {
                  TRACE_BOTH(HKWD_IENT_ERR, "MfgA", arg, EINVAL, 0);
                  rc = EINVAL;
                  break;
              }

              if (length != sizeof(uchar))
              {
                  TRACE_BOTH(HKWD_IENT_ERR, "MfgL", length, EINVAL, 0);
                  rc = EINVAL;
                  break;
              }
              
              io_addr = (ulong) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);
              BUS_GETCX((caddr_t) (io_addr + ENT_CA_IO_BASED), arg);
              TRACE_DBG(HKWD_IENT_OTHER, "MfgV", arg, 0, 0);
              (void) io_det(io_addr);
          }
          else
          {
              TRACE_BOTH(HKWD_IENT_ERR, "MfgA", arg, EINVAL, 0);
              rc = EINVAL;
          }
          break;
      }
      default:
      {
          TRACE_BOTH(HKWD_IENT_ERR, "Ictx", EOPNOTSUPP, 0, 0);
          rc = EOPNOTSUPP;
          break;
          }
    }

    (void) unlockl((int *)&WRK.lock_anchor);      /* Release the lock */

    TRACE_BOTH(HKWD_IENT_OTHER, "IctE", rc, 0, 0);
    return(rc);
}

/*****************************************************************************/
/*
 * NAME:     ient_getmib
 *
 * FUNCTION: Gather the current statistics from the adapter to the MIB table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:  ient_ioctl
 *
 * CALLS TO:
 *
 *
 * INPUTS: none
 *
 * RETURNS:  none
 */
/*****************************************************************************/

int
ient_getmib()
{
    int fq_no;
    
    TRACE_BOTH(HKWD_IENT_OTHER, "IgmB", (ulong)p_dev_ctl, 0, 0);

    /*
    ** Read four bytes of counters from adapter.
    ** Update the MIB table.
    */

    /* Ethernet Statistics table. */
    MIB.Generic_mib.RcvAddrTable = WRK.multi_count + 2;
    MIB.Ethernet_mib.Dot3StatsEntry.align_errs    = ENTSTATS.align_errs;
    MIB.Ethernet_mib.Dot3StatsEntry.fcs_errs      = ENTSTATS.fcs_errs;
    MIB.Ethernet_mib.Dot3StatsEntry.defer_tx      = ENTSTATS.tx_timeouts;
    MIB.Ethernet_mib.Dot3StatsEntry.late_collisions = ENTSTATS.late_collisions;
    MIB.Ethernet_mib.Dot3StatsEntry.excess_collisions = 
        ENTSTATS.excess_collisions;

    MIB.Ethernet_mib.Dot3StatsEntry.mac_tx_errs =
        ENTSTATS.underrun + ENTSTATS.cts_lost + ENTSTATS.tx_timeouts;

    MIB.Ethernet_mib.Dot3StatsEntry.carriers_sense = ENTSTATS.carrier_sense;
    MIB.Ethernet_mib.Dot3StatsEntry.long_frames    = ENTSTATS.long_frames;
    MIB.Ethernet_mib.Dot3StatsEntry.mac_rx_errs    =
        ENTSTATS.overrun + ENTSTATS.short_frames + ENTSTATS.no_resources +
        ENTSTATS.rx_drop;

    /* Ethernet Collision Statistics Group. */
    MIB.Ethernet_mib.Dot3CollEntry.count[0] = 0;
    
    for (fq_no = 0; fq_no < 16; fq_no++)
    {
        MIB.Ethernet_mib.Dot3CollEntry.freq[fq_no] = DEVSTATS.coll_freq[fq_no];
    }

    TRACE_BOTH(HKWD_IENT_OTHER, "IgmE", 0, 0, 0);
    return(0);
}


/*****************************************************************************/
/*
 * NAME:     ient_getstat
 *
 * FUNCTION: Gather the current statistics from the adapter to the device
 *           stats table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *              ient_ioctl
 *
 * CALLES TO:
 *
 *
 * INPUT: none.
 *
 * RETURNS:
 *      none
 */
/*****************************************************************************/

int
ient_getstat()
{
    ulong temp, ioa;
    int i;

    TRACE_BOTH(HKWD_IENT_OTHER, "IgsB", (ulong)p_dev_ctl, 0, 0);

    NDD.ndd_genstats.ndd_elapsed_time = ELAPSED_TIME(WRK.ndd_stime);

    ENTSTATS.dev_elapsed_time = ELAPSED_TIME(WRK.dev_stime);

    ENTSTATS.ndd_flags = NDD.ndd_flags;
    
    bcopy((caddr_t) WRK.ent_addr, (caddr_t) ENTSTATS.ent_nadr, ENT_NADR_LENGTH);

    ENTSTATS.device_type = ENT_IENT;
    ENTSTATS.sw_txq_len = DDS.xmt_que_size;
    ENTSTATS.hw_txq_len = XMIT_BUFFERS;


    if (WRK.machine != MACH_MEM_BASED)
        ioa = (ulong) io_att((DDS.bus_id | BUSMEM_IO_SELECT), 0);

    ENTSTATS.mcast_rx_ok = MIB.Generic_mib.ifExtnsEntry.mcast_rx_ok;
    ENTSTATS.bcast_rx_ok = MIB.Generic_mib.ifExtnsEntry.bcast_rx_ok;
    ENTSTATS.mcast_tx_ok = MIB.Generic_mib.ifExtnsEntry.mcast_tx_ok;
    ENTSTATS.bcast_tx_ok = MIB.Generic_mib.ifExtnsEntry.bcast_tx_ok;
    ENTSTATS.s_coll_frames = MIB.Ethernet_mib.Dot3StatsEntry.s_coll_frames;
    ENTSTATS.m_coll_frames = MIB.Ethernet_mib.Dot3StatsEntry.m_coll_frames;

    NDD.ndd_genstats.ndd_xmitque_cur = WRK.xmits_queued + WRK.xmits_buffered;

    /*
    ** The following code has been modified to add all SCB statistics to the
    **  ENTSTATS variables and then reset the SCB statistics to zero.
    ** This allows us to collect statistics just before a restart and
    **  thereby prevent losing those statistics when the shared memory
    **  area is bzeroed.
    */

    READ_LONG_REV(WRK.scb_ptr->crc_errs, &temp);
    ENTSTATS.fcs_errs += temp;
    WRITE_LONG(WRK.scb_ptr->crc_errs, 0);

    READ_LONG_REV(WRK.scb_ptr->align_errs, &temp);
    ENTSTATS.align_errs += temp;
    WRITE_LONG(WRK.scb_ptr->align_errs, 0);

    READ_LONG_REV(WRK.scb_ptr->overrun_errs, &temp);
    ENTSTATS.overrun += temp;
    WRITE_LONG(WRK.scb_ptr->overrun_errs, 0);

    READ_LONG_REV(WRK.scb_ptr->frame_errs, &temp);
    ENTSTATS.short_frames += temp;
    WRITE_LONG(WRK.scb_ptr->frame_errs, 0);

    READ_LONG_REV(WRK.scb_ptr->resource_errs, &temp );
    ENTSTATS.no_resources += temp;
    WRITE_LONG(WRK.scb_ptr->resource_errs, 0);

    READ_LONG_REV(WRK.scb_ptr->rcvcdt_errs, &temp);
    ENTSTATS.rx_collisions += temp;
    WRITE_LONG(WRK.scb_ptr->rcvcdt_errs, 0);

    if (WRK.machine != MACH_MEM_BASED)
        io_det(ioa);


    TRACE_BOTH(HKWD_IENT_OTHER, "IgsE", 0, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 * NAME:     multi_add
 *
 * FUNCTION: Add a multicast address to the multicast table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *              ient_ioctl
 * CALLS TO:
 *              ient_action
 *
 * INPUT:
 *      addr            - point to the multicast address to be added
 *
 * NOTES:
 *      After checking that the arg is a legit multicast address,
 *      a search is made through any previously defined addresses.
 *      If a duplicate is foucd, the reference count is incremented and
 *      no hardware action is required.
 *      If the address is not found, a nad_t is malloced, the reference
 *      count is set to 1, and the new element is linked at the start of the
 *      address list.
 *
 *      If the new address causes us to exceed the number of addresses that
 *      are handled by the adapter, we set the all_multicast bit and issue
 *      a configuration action, otherwise, we issue a set-multicast action.
 *
 *             ---------------------------------
 *            |   WRK.alt_list                  |
 *             _________________________________
 *             ---------------------------------
 *            |   count                         |
 *             _________________________________
 *             ---------------------------------
 *            |   link                          |
 *             _________________________________
 *             ---------------------------------
 *            |   expansion address             |
 *             _________________________________
 *             ---------------------------------
 *            |   count                         |
 *             ---------------------------------
 *            |   link (NULL)                   |
 *             _________________________________
 *             ---------------------------------
 *            |   expansion address             |
 *             _________________________________
 *
 * RETURN:  0     = successful
 *         EINVAL = error
 * RETURNS:  0, ENOMEM, or EINVAL
 */
/*****************************************************************************/

int
multi_add(char *addr)
{
    nad_t   *NaddrPtr = WRK.alt_list;
    nad_t   *OldNaddrPtr = NULL;
    int     first = 0, rc = 0;
    uchar   save_cfg_val;

    TRACE_BOTH(HKWD_IENT_OTHER, "ImaB", (ulong)p_dev_ctl, (ulong) addr, 0);
    /*
    ** Verify that it is a valid multicast address
    */
    if (!((*((char *)addr) & MULTI_BIT_MASK)))
    {
        return(EINVAL);
    }

    /*
    ** If the multicast address is already in the multicast table, increment
    ** the reference count and exit. There is no need to issue a command to the
    ** adapter since we are not changing any addresses.
    */

    while (NaddrPtr != NULL)
    {
        if (SAME_NADR(addr, &NaddrPtr->nadr))
        {
            NaddrPtr->count++;
            return(0);
        }
        else
        {
            /* link to the next address */
            OldNaddrPtr = NaddrPtr;
            NaddrPtr = (nad_t *) NaddrPtr->link;
        }
    }

    /*
    ** If we get to this point then the address is not in the table,
    ** xmalloc an address element and link it at start of the table.
    */
    
    NaddrPtr = (nad_t *) xmalloc(sizeof(nad_t), 2, pinned_heap);
    if (!NaddrPtr)
    {
        return(ENOMEM);
    }

    if (WRK.alt_list == (nad_t *) NULL)      /* Assign first to head of list */
    {
        first = TRUE;
        WRK.alt_list = NaddrPtr;
    }
    else
        OldNaddrPtr->link = (struct nad *) NaddrPtr;

    NaddrPtr->count = 1;
    NaddrPtr->link = (struct nad *) NULL;
    COPY_NADR(addr, &NaddrPtr->nadr);

    /*
    ** If incrementing the count will exceed the number of addresses
    ** that we wish to handle in the adapter and if the all-multi
    ** configuration is not already set, we will set all-multi
    ** by issuing a config command to the adapter.  We leave the
    ** current MCA set active in the adapter.
    */

    if (++WRK.alt_count > MAX_MULTI)
    {
        if (WRK.alt_count == (MAX_MULTI + 1))
        {
            save_cfg_val = WRK.cur_cfg.preamble;
            WRK.cur_cfg.preamble &= ~MC_ALL_BIT;

            rc = ient_action(INTERNAL_CFG_CMD, TRUE);
            if (!(rc))
            {
                if (!(WRK.multi_count))
                {
                    NDD.ndd_flags |= NDD_MULTICAST;
                    WRK.multi_count++;              /* Bump the multi-count */
                }
            }
            else             /* Failed, restore initial values. */
            {
                WRK.alt_count--;
                WRK.cur_cfg.preamble = save_cfg_val;
                if (first)
                    WRK.alt_list = (nad_t *) NULL;
                else
                    OldNaddrPtr->link = (struct nad *) NULL;

                xmfree((caddr_t) NaddrPtr, pinned_heap);
            }
        }
    }
    else
    {
        /*
        **  Perform a MC setup action command.
        */

        rc = ient_action(MCS_CMD, TRUE);
        if (rc)
        {
            if (first)
                WRK.alt_list = (nad_t *) NULL;
            else
                OldNaddrPtr->link = (struct nad *) NULL;

            WRK.alt_count--;
            xmfree((caddr_t) NaddrPtr, pinned_heap);
        }
    }

    TRACE_BOTH(HKWD_IENT_OTHER, "ImaE", rc, WRK.alt_count, WRK.multi_count);
    return(rc);

    
}

/*****************************************************************************/
/*
 * NAME:     multi_del
 *
 * FUNCTION: Delete a multicast address from the multicast table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *      Find the address in the table and decrement the reference count.
 *      if reference count is non-zero, return with no hardware action;
 *      otherwise remove the address fronm the table and decrement the
 *      WRK.alt_count.
 *
 *      If decrementing the count brings the number of alt addresses to
 *      or below MAX_MULTI, then issue a MCS set to the adapter.  If
 *      all-multicast is set and it was not set by a control op, then issue
 *      a config to enable filtering at the adapter.
 *
 * CALLED FROM:  ient_ioctl
 *
 * CALLS TO:     ient_action
 *               SAME_NADR
 *               xmfree
 *
 * INPUT:        addr - point to the multicast address to be added
 *
 * RETURNS:  0 (success) or error
 *
 */
/*****************************************************************************/

int
multi_del(char *addr)
{
    nad_t   *NaddrPtr, *OldNaddrPtr = NULL;
    int     first = 0, rc = 0;
    uchar   save_cfg_val;

    TRACE_BOTH(HKWD_IENT_OTHER, "ImdB", (ulong) addr, 0, 0);

    /*
    ** Find the address in the table and decrement the reference count.
    ** if reference count is non-zero, return with no hardware action;
    ** otherwise remove the address fronm the table and decrement the
    ** WRK.alt_count.
    */

    NaddrPtr = WRK.alt_list;

    while (NaddrPtr)
    {
        if (SAME_NADR(&NaddrPtr->nadr, addr))
        {
            break;               /* Found the address we want to delete */
        }
        else
        {
            /* look at the next address */
            OldNaddrPtr = NaddrPtr;
            NaddrPtr = (nad_t *) NaddrPtr->link;
        }
    }
    /*
    ** If we have not found the address, return an error
    */
    if (!NaddrPtr)
    {
        return(EINVAL);
    }

    /*
    ** Decrement the reference count.
    ** If the count does not zero, we need take no further action.
    */
    if (--NaddrPtr->count)
    {
        return(0);
    }

    /*
    ** The count went to zero.  Delete the address and free the element.
    */

    if (OldNaddrPtr)
        OldNaddrPtr->link = NaddrPtr->link;
    else
    {
        first = TRUE;
        WRK.alt_list = (nad_t *) NaddrPtr->link;
    }

    /*
    ** If the count has decreased to the point that hardware can do
    ** the filtering, then do a MC set.
    */
    
    if (--WRK.alt_count != MAX_MULTI)
    {
        rc = ient_action(MCS_CMD, TRUE);
        if (!(rc))
        {
            xmfree(NaddrPtr, pinned_heap);
        }
        else                             /* Failed, so reestablish pointers */
        {
            NaddrPtr->count++;
            WRK.alt_count++;

            if (first)
                WRK.alt_list = NaddrPtr;
            else
                OldNaddrPtr->link = (struct nad *) NaddrPtr;

            TRACE_BOTH(HKWD_IENT_OTHER, "Imd1", (ulong) NaddrPtr,
                                                NaddrPtr->count, 0);
        }
    }
    else                                  /* WRK.alt_count == MAX_MULTI */
    {
        /*
        ** If the decrementation of alt_count brings us to MAX_MULTI then we
        ** can turn off multicast all.
        */

        save_cfg_val = WRK.cur_cfg.preamble;
        WRK.cur_cfg.preamble |= MC_ALL_BIT;

        /*
        ** We set the wait flag to TRUE so we will sleep until this command
        ** completes before going on to the MCS_CMD.
        */

        rc = ient_action(INTERNAL_CFG_CMD, TRUE);
        if (!(rc))
        {
            DELAYMS(100);
            ient_action(MCS_CMD, TRUE);
            NDD.ndd_flags &= ~NDD_MULTICAST;
            xmfree(NaddrPtr, pinned_heap);
            if (WRK.multi_count) WRK.multi_count--;
        }
        else                    /* Failed, so reestablish pointers */
        {
            WRK.cur_cfg.preamble = save_cfg_val;
            NaddrPtr->count++;
            WRK.alt_count++;

            if (first)
                WRK.alt_list = NaddrPtr;
            else
            {
                OldNaddrPtr->link = (struct nad *) NaddrPtr;
                TRACE_BOTH(HKWD_IENT_OTHER, "Imd4", (ulong) NaddrPtr,
                                                            NaddrPtr->count, 0);
            }
        }
    }

    TRACE_BOTH(HKWD_IENT_OTHER,"ImdE", rc, 0, 0);
    return(rc);
}
