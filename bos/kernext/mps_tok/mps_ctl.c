static char sccsid[] = "@(#)62  1.9  src/bos/kernext/mps_tok/mps_ctl.c, sysxmps, bos41J, 9520B_all 5/18/95 11:24:38";
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: getmib
 *		mps_ctl
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stddef.h>
#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/sleep.h>
#include <sys/err_rec.h>
#include <sys/errids.h>
#include <sys/dump.h>
#include <sys/mbuf.h>
#include <sys/ndd.h>
#include <sys/cdli.h>

#include <sys/cdli_tokuser.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>

#include "mps_dslo.h"
#include "mps_mac.h"
#include "mps_dds.h"
#include "mps_dd.h"
#include "mps_cmd.h"
#include "tr_mps_errids.h"

#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

extern int mps_priority (ndd_t *p_ndd, struct mbuf *p_mbuf);
extern int mps_dump     (ndd_t *p_ndd, int cmd, caddr_t arg);
/*
 *  MIB status table - this table defines the MIB variable status returned
 *  on MIB query operation.
 */
token_ring_all_mib_t mps_mib_status = {

  /*
   * Generic Interface Extension Table
   */
  MIB_READ_ONLY,                 /* ifExtnsChipSet                  */
  MIB_READ_ONLY,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /* ifExtnsRevWare  */
  MIB_READ_ONLY,                 /* ifExtnsMulticastsTransmittedOks */
  MIB_READ_ONLY,                 /* ifExtnsBroadcastsTransmittedOks */
  MIB_READ_ONLY,                 /* ifExtnsMulticastsReceivedOks    */
  MIB_READ_ONLY,                 /* ifExtnsBroadcastsReceivedOks    */
  MIB_READ_ONLY,                 /* ifExtnsPromiscuous              */

  /*
   * Generic Interface Test Table
   */
  MIB_NOT_SUPPORTED,              /* ifEXtnsTestCommunity            */
  MIB_NOT_SUPPORTED,              /* ifEXtnsTestRequestId            */
  MIB_NOT_SUPPORTED,              /* ifEXtnsTestType                 */
  MIB_NOT_SUPPORTED,              /* ifEXtnsTestResult               */
  MIB_NOT_SUPPORTED,              /* ifEXtnsTestCode                 */

  /*
   * Generic Receive Address Table
   */
  MIB_READ_ONLY,                  /* ifExtnsRcvAddress               */

  /*
   * The TOK Interface Group
   */
  MIB_READ_ONLY,                 /* dot5Commands;                   */
  MIB_READ_ONLY,                 /* dot5RingStatus;                 */
  MIB_READ_ONLY,                 /* dot5RingState;                  */
  MIB_READ_ONLY,                 /* dot5RingOpenStatus;             */
  MIB_READ_ONLY,                 /* dot5RingSpeed;                  */
  MIB_READ_ONLY,0,0,0,0,0,       /* dot5UpStream;                   */
  MIB_READ_ONLY,                 /* dot5ActMonParticipate;          */
  MIB_READ_ONLY,0,0,0,0,0,       /* dot5Functional;                 */

  /*
   * The TOK Statistics Group
   */
  MIB_READ_ONLY,                  /* dot5StatsLineErrors;            */
  MIB_READ_ONLY,                  /* dot5StatsBurstErrors;           */
  MIB_READ_ONLY,                  /* dot5StatsACErrors;              */
  MIB_READ_ONLY,                  /* dot5StatsAbortTransErrors;      */
  MIB_READ_ONLY,                  /* dot5StatsInternalErrors;        */
  MIB_READ_ONLY,                  /* dot5StatsLostFrameErrors;       */
  MIB_READ_ONLY,                  /* dot5StatsReceiveCongestions;    */
  MIB_READ_ONLY,                  /* dot5StatsFrameCopiedErrors;     */
  MIB_READ_ONLY,                  /* dot5StatsTokenErrors;           */
  MIB_READ_ONLY,                  /* dot5StatsSoftErrors;            */
  MIB_READ_ONLY,                  /* dot5StatsHardErrors;            */
  MIB_READ_ONLY,                  /* dot5StatsSignalLoss;            */
  MIB_READ_ONLY,                  /* dot5StatsTransmitBeacons;       */
  MIB_READ_ONLY,                  /* dot5StatsRecoverys;             */
  MIB_READ_ONLY,                  /* dot5StatsLobeWires;             */
  MIB_READ_ONLY,                  /* dot5StatsRemoves;               */
  MIB_READ_ONLY,                  /* dot5StatsSingles;               */
  MIB_READ_ONLY,                  /* dot5StatsFreqErrors;            */

  /*
   * The TOK Timer Group
   */
  MIB_NOT_SUPPORTED,              /* dot5TimerReturnRepeat;          */
  MIB_NOT_SUPPORTED,              /* dot5TimerHolding;               */
  MIB_NOT_SUPPORTED,              /* dot5TimerQueuePDU;              */
  MIB_NOT_SUPPORTED,              /* dot5TimerValidTransmit;         */
  MIB_NOT_SUPPORTED,              /* dot5TimerNoToken;               */
  MIB_NOT_SUPPORTED,              /* dot5TimerActiveMon;             */
  MIB_NOT_SUPPORTED,              /* dot5TimerStandbyMon;            */
  MIB_NOT_SUPPORTED,              /* dot5TimerErrorReport;           */
  MIB_NOT_SUPPORTED,              /* dot5TimerBeaconTransmit;        */
  MIB_NOT_SUPPORTED,              /* dot5TimerBeaconReceive;         */
};

/*****************************************************************************/
/*
 * NAME:     mps_ctl
 *
 * FUNCTION: Wildwood driver ioctl routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ndd_ctl field in the NDD on the NDD chain.
 *
 * INPUT:
 *  p_ndd               - pointer to the ndd in the dev_ctl area
 *  cmd                 - control command
 *  arg                 - argument of the control command
 *  length              - length of the argument
 *
 * RETURNS:
 *      0           - successful
 *      ENETUNREACH - device is currently unreachable
 *      ENETDOWN    - device is down
 *      EINVAL      - invalid paramter
 *      ENOMEM      - unable to allocate required memory
 *      EOPNOTSUPP  - operation not supported
 */
/*****************************************************************************/
mps_ctl(
ndd_t   *p_ndd,         /* pointer to the ndd in the dev_ctl area */
int     cmd,            /* control command */
caddr_t arg,            /* argument of the control command */
int     length)         /* length of the argument */


{
  mps_dev_ctl_t   *p_dev_ctl = (mps_dev_ctl_t *)(p_ndd->ndd_correlator);
  int           rc = 0;         /* return code */
  int           ipri;
  int           i,j, ioa;
  ushort        rcv_op;
  int           flag = 0;
  uchar bcast1_addr[CTOK_NADR_LENGTH] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  uchar bcast2_addr[CTOK_NADR_LENGTH] = { 0xC0,0x00,0xFF,0xFF,0xFF,0xFF};

  ndd_mib_addr_t  *p_table = (ndd_mib_addr_t *)arg;
  ndd_mib_addr_elem_t  *p_elem;
  mps_multi_t *p_multi = &WRK.multi_table;
  int elem_len = sizeof(ndd_mib_addr_elem_t) + CTOK_NADR_LENGTH - 2;
  int count = 0;

  TRACE_SYS(MPS_OTHER, "IctB", (ulong)p_dev_ctl, cmd, (ulong)arg);
  TRACE_SYS(MPS_OTHER, "Ictc", (ulong)p_dev_ctl, length, 
					(ulong)p_dev_ctl->device_state);

  /*
   * Locks the complex lock ctl_clock to locks out other ioctl 
   * to the same device.
   */
  lock_write(&CTL_LOCK);

  switch(cmd) {

  /*
   * Returns the address of the device driver's dump routine.
   */
  case NDD_DUMP_ADDR:
        if (arg == 0) {
                rc = EINVAL;
                TRACE_BOTH(MPS_ERR, "Ict1", p_dev_ctl, 0, 0);
                break;
        }
        *(uint *)arg = (uint)mps_dump;
        break;

  /*
   * Returns the address of the device driver's priority transmit routine.
   */
  case NDD_PRIORITY_ADDR:
        if (arg == 0) {
                rc = EINVAL;
                TRACE_BOTH(MPS_ERR, "Ict2", p_dev_ctl, 0, 0);
                break;
        }
       	*(uint *)arg = (uint)mps_priority;
        break;

  /*
   * clears all the statistics.
   */
  case NDD_CLEAR_STATS:
  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(MPS_ERR, "Ict3", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(MPS_ERR, "Ict4", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
        	break;
  	}

        WRK.ndd_stime = WRK.dev_stime =lbolt;

        /*
         * Clears the statistics from the adapter
         */
        if (read_adapter_log(p_dev_ctl, FALSE)) {
                break;
	}

        bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));
        bzero(&TOKSTATS, sizeof(TOKSTATS) );
        bzero(&DEVSTATS, sizeof(DEVSTATS) );
        TOKSTATS.device_type = TOK_MPS;
        COPY_NADR (WRK.mps_addr, TOKSTATS.tok_nadr);
        bcopy(TR_MIB_IBM16,MIB.Generic_mib.ifExtnsEntry.chipset, CHIPSETLENGTH);
        MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_NOPROBLEM;

        break;

  /*
   * Gets all of the device statistics.
   */
  case NDD_GET_ALL_STATS:
  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(MPS_ERR, "Ict5", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(MPS_ERR, "Ict6", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
        	break;
  	}

        if (length != (sizeof(mps_all_stats_t))) {
                rc = EINVAL;
                TRACE_BOTH(MPS_ERR, "Ict5", p_dev_ctl, 0, 0);
                break;
        }

        NDD.ndd_genstats.ndd_elapsed_time = NDD_ELAPSED_TIME(WRK.ndd_stime);
        TOKSTATS.dev_elapsed_time = NDD_ELAPSED_TIME(WRK.dev_stime);
        TOKSTATS.ndd_flags = NDD.ndd_flags;
        TOKSTATS.device_type = TOK_MPS;
        TOKSTATS.sw_txq_len = p_dev_ctl->txq2_len;
        TOKSTATS.hw_txq_len = WRK.tx2_frame_pending;
        TOKSTATS.device_type = TOK_MPS;
        NDD.ndd_genstats.ndd_xmitque_cur = p_dev_ctl->txq2_len + 
							WRK.tx2_frame_pending;

	/* temp use reserved for priority queue */
        TOKSTATS.reserve3 = p_dev_ctl->txq1_len;
        TOKSTATS.reserve4 = WRK.tx1_frame_pending;
        TOKSTATS.reserve1 = p_dev_ctl->txq1_len + WRK.tx1_frame_pending;

        /*
         * Gets the statistics from the adapter
         */
        if (read_adapter_log(p_dev_ctl, FALSE)) {
                TRACE_BOTH(MPS_ERR, "Ict7", p_dev_ctl, 0, 0);
                break;
	}

    	/*
         * Copys the network address 
      	 */
        COPY_NADR (WRK.mps_addr, TOKSTATS.tok_nadr);

        /*
         * Copys the statistics to user's buffer 
       	 */
        bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
        bcopy(&TOKSTATS, arg + sizeof(ndd_genstats_t),
                sizeof(tok_genstats_t) + sizeof(tr_mps_stats_t));
        break;

  /*
   * Gets the statistics.
   */
  case NDD_GET_STATS:
  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(MPS_ERR, "Ict8", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(MPS_ERR, "Ict9", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
        	break;
  	}


        if (length != sizeof(tok_ndd_stats_t)) {
                rc = EINVAL;
                TRACE_BOTH(MPS_ERR, "Icta", p_dev_ctl, 0, 0);
                break;
        }

        NDD.ndd_genstats.ndd_elapsed_time = NDD_ELAPSED_TIME(WRK.ndd_stime);
        TOKSTATS.dev_elapsed_time = NDD_ELAPSED_TIME(WRK.dev_stime);
        TOKSTATS.ndd_flags = NDD.ndd_flags;
        TOKSTATS.sw_txq_len = p_dev_ctl->txq2_len;
        TOKSTATS.hw_txq_len = WRK.tx2_frame_pending;
        TOKSTATS.device_type = TOK_MPS;
        NDD.ndd_genstats.ndd_xmitque_cur = p_dev_ctl->txq2_len + 
							WRK.tx2_frame_pending;

	/* temp use reserved for priority queue */
        TOKSTATS.reserve3 = p_dev_ctl->txq1_len;
        TOKSTATS.reserve4 = WRK.tx1_frame_pending;
        TOKSTATS.reserve1 = p_dev_ctl->txq1_len + WRK.tx1_frame_pending;

        /*
         * Gets the statistics from the adapter
         */
        if (read_adapter_log(p_dev_ctl, FALSE)) {
                break;
	}

    	/*
         * Copys the network address 
      	 */
        COPY_NADR (WRK.mps_addr, TOKSTATS.tok_nadr);

	/*
         * Copys the statistics to user's buffer 
     	 */
        bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
        bcopy(&TOKSTATS, arg + sizeof(ndd_genstats_t),
                sizeof(tok_genstats_t));
        break;

  /*
   * Enables the promiscuous mode.  If this is the first promiscuous on
   * operation and the adapter is not in promiscuous mode already, the
   * driver will configure the adapter to run in the promiscuous mode.
   */
  case NDD_PROMISCUOUS_ON:
  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(MPS_ERR, "Ictb", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        		break;
        	}
  	}

        if ((WRK.promiscuous_count == 0)&&(p_dev_ctl->device_state == OPENED)) {
                if(rc = modify_receive_options(p_dev_ctl, PROMIS_ON, FALSE)) {
                        TRACE_BOTH(MPS_ERR, "Ictc", p_dev_ctl, rc, 0);
                }
        }
        WRK.promiscuous_count++;   /* incr the ref counter */
        MIB.Generic_mib.ifExtnsEntry.promiscuous = PROMTRUE;
        NDD.ndd_flags |= NDD_PROMISC;
        TRACE_SYS(MPS_OTHER, "Ictd", p_dev_ctl, NDD.ndd_flags, 
						WRK.promiscuous_count);
        break;

  /*
   * Disables the promiscuous mode.  If this is the last promiscuous off
   * operation and there is no other reason to stay in the promiscuous mode
   * the adapter will be re-configured to get out of the promiscuous mode.
   */
  case NDD_PROMISCUOUS_OFF:
        if (!WRK.promiscuous_count) {
                rc = EINVAL;
                TRACE_BOTH(MPS_ERR, "Ictf", p_dev_ctl, 0, 0);
                break;
        }

        WRK.promiscuous_count--;
        if (WRK.promiscuous_count == 0) {
               	NDD.ndd_flags &= ~NDD_PROMISC;
               	MIB.Generic_mib.ifExtnsEntry.promiscuous = PROMFALSE;
                TRACE_SYS(MPS_OTHER, "Ictg", p_dev_ctl, 
					p_dev_ctl->device_state, 0);
        	if (p_dev_ctl->device_state == OPENED) {
                	modify_receive_options(p_dev_ctl, PROMIS_OFF, FALSE);
        	}
        }
        break;

  /*
   * Gets receive address table (mainly for MIB variables).  The receive
   * address table is consists of all the addresses that the adapter is
   * armed to receive packets with.  It includes the host network address,
   * the broadcast address and the currently registered multicast addresses.
   * This operation doesn't report the device state.
   */
   case NDD_MIB_ADDR:
       if (arg == 0) {
                rc = EINVAL;
                break;
        }

  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(MPS_ERR, "Icth", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	lock_done(&CTL_LOCK);
                	TRACE_BOTH(MPS_ERR, "Icti", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
                break;
  	}

       if (length < sizeof(ndd_mib_addr_t)) {
                rc = EINVAL;
                TRACE_BOTH(MPS_ERR, "Ictj", p_dev_ctl, 0, 0);
                break;
        }

        length -= sizeof(u_int);   /* reserve room for the count field */
        arg += 4;

	/*
         * Copys the specific network address in use first 
	 */
        if (length >= elem_len) {
                p_elem = (ndd_mib_addr_elem_t *)arg;
                p_elem->status = NDD_MIB_VOLATILE;
                p_elem->addresslen = CTOK_NADR_LENGTH;
                COPY_NADR(WRK.mps_addr, p_elem->address);
                length -= elem_len;
                arg += elem_len;
                count++;
        } else {
                rc = E2BIG;
        }

	/*
         * Copys the first broadcast address 
	 */
        if (length >= elem_len) {
                p_elem = (ndd_mib_addr_elem_t *)arg;
                p_elem->status = NDD_MIB_NONVOLATILE;
                p_elem->addresslen = CTOK_NADR_LENGTH;
                COPY_NADR(bcast1_addr, p_elem->address);
                length -= elem_len;
                arg += elem_len;
                count++;
        } else {
                rc = E2BIG;
        }

	/*
         * Copys the second broadcast address 
	 */
        if (length >= elem_len) {
                p_elem = (ndd_mib_addr_elem_t *)arg;
                p_elem->status = NDD_MIB_NONVOLATILE;
                p_elem->addresslen = CTOK_NADR_LENGTH;
                COPY_NADR(bcast2_addr, p_elem->address);
                length -= elem_len;
                arg += elem_len;
                count++;
        } else {
                rc = E2BIG;
        }

        if ( NDD.ndd_flags & TOK_RECEIVE_FUNC) {
		/*
        	 * Copies the functional address 
		 */
        	if (length >= elem_len) {
                	p_elem = (ndd_mib_addr_elem_t *)arg;
                	p_elem->status = NDD_MIB_VOLATILE;
                	p_elem->addresslen = CTOK_NADR_LENGTH;
                	COPY_NADR(FUNCTIONAL.functional, p_elem->address);
                	length -= elem_len;
                	arg += elem_len;
                	count++;
        	} else {
                	rc = E2BIG;
        	}
	}

	/*
         * Copys the multicast addresses 
	 */
        while (p_multi) {
                for (i=0; i < p_multi->in_use; i++) {
                        if (length >= elem_len) {
                                p_elem = (ndd_mib_addr_elem_t *)arg;
                                p_elem->status = NDD_MIB_VOLATILE;
                                p_elem->addresslen = CTOK_NADR_LENGTH;

                                COPY_NADR(p_multi->m_slot[i].m_addr,
                                                        p_elem->address);
                                length -= elem_len;
                                arg += elem_len;
                                count++;
                        } else {
                                rc = E2BIG;
                                break;
                        }
                }
                if (i < p_multi->in_use) {
                        break;
		}

                p_multi = p_multi->next;
        }

	/*
         * Puts the final count into the buffer 
 	 */
        p_table->count = count;

        break;

  /*
   * Queries MIB support status on the driver.
   */
  case NDD_MIB_QUERY:
       if (arg == 0) {
                rc = EINVAL;
                break;
        }

  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(MPS_ERR, "Ictk", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(MPS_ERR, "Ictl", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
                break;
  	}


        if (length != sizeof(token_ring_all_mib_t)) {
                rc = EINVAL;
                TRACE_BOTH(MPS_ERR, "Ictm", p_dev_ctl, 0, 0);
                break;
        }
	/*
         * Copys the status to user's buffer 
	 */
        bcopy(&mps_mib_status, arg, sizeof(token_ring_all_mib_t));
        break;

  /*
   * Gets all MIB values.
   */
  case NDD_MIB_GET:
       if (arg == 0) {
                rc = EINVAL;
                break;
        }

  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(MPS_ERR, "Ictn", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(MPS_ERR, "Icto", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
                break;
  	}


        if (length != sizeof(token_ring_all_mib_t)) {
                rc = EINVAL;
                TRACE_BOTH(MPS_ERR, "Ictp", p_dev_ctl, 0, 0);
                break;
        }

        /*
         * Gets the statistics from the adapter
         */
        if (read_adapter_log(p_dev_ctl, FALSE)) {
                TRACE_BOTH(MPS_ERR, "Ictq", p_dev_ctl, 0, 0);
                break;
	}

        /*
         * Gathers the current MIB statistics
         */
        getmib(p_dev_ctl);


        MIB.Token_ring_mib.Dot5Entry.commands = 0;
	/*
         * Copys mibs to user's buffer 
	 */
        bcopy(&MIB, arg, sizeof(token_ring_all_mib_t));

        break;

  /*
   * Sets the alternate address
   */
  case NDD_ENABLE_ADDRESS:
  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(MPS_ERR, "Ictr", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
                	break;
        	}
  	}

        TRACE_DBG(MPS_OTHER, "Icts", p_dev_ctl, NDD.ndd_flags, 0);
        flag = 1;

        if (*((char *)(arg + 2)) & MULTI_BIT_MASK) {
        	/*
        	 * Sets the group address
        	 */
                if (rc = multi_add(p_dev_ctl, p_ndd, arg))
                {
                        TRACE_BOTH(MPS_ERR, "Ictt", p_dev_ctl,
                                        NDD.ndd_flags, rc);
                } else {
                        NDD.ndd_flags |= NDD_ALTADDRS | TOK_RECEIVE_GROUP;
                        TRACE_DBG(MPS_ERR, "Ictu", p_dev_ctl,
                                        NDD.ndd_flags, 0);
                }
        } else {
        	/*
        	 * Sets the functional address
        	 */

		/*
		 * Check if it is a functional address
                 */
		if ((*(arg + 0) != 0xC0) | (*(arg + 1) != 0)) { 
                	rc = EINVAL;
			break;
		}

                /*
                 * Keeps a reference count on each of the bits in the address
                 */
                for (i = 0, j = 1; i < 32; i++, j <<= 1) {
                          if ( *((uint *) (arg + 2)) & j) {
                                WRK.func_ref_cnt[i]++;
                          }
                  }

		if ((FUNCTIONAL.functional[2] == *(arg + 2)) &
			(FUNCTIONAL.functional[2] == *(arg + 3)) &
			(FUNCTIONAL.functional[2] == *(arg + 4)) &
			(FUNCTIONAL.functional[2] == *(arg + 5))) {
			break;
		}

		/*
                 * Updates the functional address 
		 */
                FUNCTIONAL.functional[2] |= *(arg + 2);
                FUNCTIONAL.functional[3] |= *(arg + 3);
                FUNCTIONAL.functional[4] |= *(arg + 4);
                FUNCTIONAL.functional[5] |= *(arg + 5);
                NDD.ndd_flags |= NDD_ALTADDRS | TOK_RECEIVE_FUNC;

  		if (p_dev_ctl->device_state == OPENED) {
                	if (rc = set_functional_address( p_dev_ctl,
                                &FUNCTIONAL.functional[2], FALSE)) {
                		TRACE_BOTH(MPS_ERR, "Ictw", p_dev_ctl, 0, 0);
			}
		}
        }

        break;    /* end of NDD_ENABLE_ADDRESS */

  /*
   * Resets the alternate address
   */
  case NDD_DISABLE_ADDRESS:
        flag = 1;

        if (*((char *)(arg + 2)) & MULTI_BIT_MASK) {
                /*
                 * Resets the group address
                 */
                rc = multi_del(p_dev_ctl, p_ndd, arg);
                if (!WRK.multi_count) {
                        NDD.ndd_flags &= ~TOK_RECEIVE_GROUP;
                	if (!(NDD.ndd_flags & TOK_RECEIVE_FUNC)) {
                        	NDD.ndd_flags &= ~NDD_ALTADDRS;
			}
		}

        } else {
		/*
		 * Check if it is a functional address
                 */
		if ((*(arg + 0) != 0xC0) | (*(arg + 1) != 0)) { 
                	rc = EINVAL;
			break;
		}

                /*
                 * Resets the functional address
                 * 32nd bit defined group/functional addr - don't test this bit
                 */
                /* First, Ensure we have a valid address */
                for (i = 0, j = 1; i < 31; i++, j <<= 1) {
                         if ( *((uint *) (arg + 2)) & j) {
                                if (! WRK.func_ref_cnt[i]) {
                                        /* trying to disable a nonenabled bit */
                			TRACE_BOTH(MPS_ERR, "Icty", p_dev_ctl,
							 (*(uint *)(arg+2)), 0);
                                        rc = EINVAL;
                                        break;
                                }
                          }
                }

                if (rc) { 
			break; 
		}

                /* 
		 * Then, Decrement bitwise reference count and (maybe) clear 
                 * Function address bits 
		 */

                for (i = 0, j = 1; i < 31; i++, j <<= 1) {
                         if ( *((uint *) (arg + 2)) & j) {
                                WRK.func_ref_cnt[i]--;
                                if (! WRK.func_ref_cnt[i]) {
                                     *(uint *)(&FUNCTIONAL.functional[2]) &= ~j;
                                }
                          }
                }

                /*
                 * If not receiving data for any functional addresses and
                 * any group addresses then NDD_ALTADDRS is off.
                 */
                if (! (*(uint *)(&FUNCTIONAL.functional[2])) ) {
                          NDD.ndd_flags &= ~TOK_RECEIVE_FUNC;
                          if (!WRK.multi_count) {
                                  NDD.ndd_flags &= ~NDD_ALTADDRS;
			  }
                }

  		if (p_dev_ctl->device_state == OPENED) {
                	if (rc = set_functional_address( p_dev_ctl,
                                &FUNCTIONAL.functional[2], FALSE)) {
                		TRACE_BOTH(MPS_ERR, "Ictz", p_dev_ctl, rc, 0);
			}
		}

        }

        break;    /* end of NDD_DISABLE_ADDRESS */

  default:
        rc = EOPNOTSUPP;
        break;


  }

  /*
   * Checks if the multi_address table need to be updated
   */
  if (flag) {
        /*
         * Checks if the multi_address table need to be allocated
         */
        if (!WRK.new_multi) {
                WRK.new_multi = (mps_multi_t *)
                     xmalloc(sizeof(mps_multi_t),MEM_ALIGN,pinned_heap);
	}

        /*
         * Checks whether need to free the multi_address table
         */
        if (!WRK.free_multi) {
                xmfree(WRK.free_multi, pinned_heap);
                WRK.free_multi = NULL;
        }
  }

  lock_done(&CTL_LOCK);
  TRACE_SYS(MPS_OTHER, "IctE", p_dev_ctl, rc, 0);
  return(rc);

}

/*****************************************************************************/
/*
 * NAME:     getmib
 *
 * FUNCTION: Gather the current statistics from the adapter to the MIB table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      mps_ctl
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 */
/*****************************************************************************/
getmib(
  mps_dev_ctl_t      *p_dev_ctl)     /* pointer to device control area */

{

  int     ioa, i,j;
  ushort  addr;

  TRACE_SYS(MPS_OTHER, "Imbb", p_dev_ctl, 0, 0);
  MIB.Token_ring_mib.Dot5StatsEntry.line_errs = TOKSTATS.line_errs;
  MIB.Token_ring_mib.Dot5StatsEntry.burst_errs = TOKSTATS.burst_errs;
  MIB.Token_ring_mib.Dot5StatsEntry.int_errs = TOKSTATS.int_errs;
  MIB.Token_ring_mib.Dot5StatsEntry.lostframes = TOKSTATS.lostframes;
  MIB.Token_ring_mib.Dot5StatsEntry.rx_congestion= TOKSTATS.rx_congestion;
  MIB.Token_ring_mib.Dot5StatsEntry.framecopies = TOKSTATS.framecopies;
  MIB.Token_ring_mib.Dot5StatsEntry.token_errs = TOKSTATS.token_errs;

  MIB.Token_ring_mib.Dot5StatsEntry.signal_loss = TOKSTATS.signal_loss;
  MIB.Token_ring_mib.Dot5StatsEntry.hard_errs = TOKSTATS.hard_errs;
  MIB.Token_ring_mib.Dot5StatsEntry.soft_errs = TOKSTATS.soft_errs;
  MIB.Token_ring_mib.Dot5StatsEntry.tx_beacons = TOKSTATS.tx_beacons;
  MIB.Token_ring_mib.Dot5StatsEntry.lobewires = TOKSTATS.lobewires;
  MIB.Token_ring_mib.Dot5StatsEntry.removes = TOKSTATS.removes;
  MIB.Token_ring_mib.Dot5StatsEntry.singles = TOKSTATS.singles;
  MIB.Token_ring_mib.Dot5StatsEntry.recoverys = TOKSTATS.recoverys;

  MIB.Generic_mib.ifExtnsEntry.mcast_rx_ok = TOKSTATS.mcast_recv;
  MIB.Generic_mib.ifExtnsEntry.bcast_rx_ok = TOKSTATS.bcast_recv;
  MIB.Generic_mib.ifExtnsEntry.mcast_tx_ok = TOKSTATS.mcast_xmit;
  MIB.Generic_mib.ifExtnsEntry.bcast_tx_ok = TOKSTATS.bcast_xmit;

  MIB.Token_ring_mib.Dot5Entry.ring_ostatus = TR_MIB_LASTOPEN;
  MIB.Token_ring_mib.Dot5Entry.participate = TR_MIB_TRUE;

  /*
   * Gets the functional address 
   */
  for (i=0; i< 6; i++) {
        MIB.Token_ring_mib.Dot5Entry.functional[i] = FUNCTIONAL.functional[i];
  }

  /*
   * Gets the Up stream hardware address 
   */
  if (p_dev_ctl->device_state == OPENED) {
        ioa = (int)BUSIO_ATT( DDS.bus_id, DDS.io_base_addr);

        PIO_PUTSRX(ioa + LAPE, 0x00);
       	PIO_PUTSRX(ioa + LAPA, WRK.parms_addr + 4);
       	for (i=0, j=0; i < 6; j++) {
            	PIO_GETSX(ioa + LAPD_I, &addr);
                MIB.Token_ring_mib.Dot5Entry.upstream[i++] = addr >> 8;
                MIB.Token_ring_mib.Dot5Entry.upstream[i++] = addr &  0xff;
       	}
        if (WRK.pio_rc) {
                BUSIO_DET(ioa);            /* restore I/O Bus */
                TRACE_BOTH(MPS_ERR, "Imb1", p_dev_ctl, WRK.pio_rc, 0);
                mps_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 0, 0, 0);
                return (VPD.status);
        }

        BUSIO_DET(ioa);         /* restore I/O Bus */
  }
  TRACE_SYS(MPS_OTHER, "Imbe", p_dev_ctl, 0, 0);
}
