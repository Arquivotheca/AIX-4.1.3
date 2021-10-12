static char sccsid[] = "@(#)65  1.22  src/bos/kernext/dlc/qlc/qlclutil.c, sysxdlcq, bos411, 9437B411a 9/14/94 11:29:15";
/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: qlm_select_netid, qlm_find_ls_anywhere, qlm_find_ls_in_channel,
 *            qlm_find_ls_in_sap, qlm_find_ls_given_netid,
 *            qlm_find_ls_given_correlator, qlm_find_contending_ls,
 *            qlm_find_listening_ls, qlm_remote_stn_id_is_valid,
 *            qlm_delete_station, qlm_lock_list_and_ls
 *	      qlm_find_ls_in_channel_by_corr, qlm_find_ls_in_sap_by_corr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/* Include QLLC Link Station Manager include file                            */
/*****************************************************************************/
#include "qlcg.h"    
#include "qlcq.h"   
#include "qlcv.h" 
#include "qlcvfac.h" 
#include "qlcb.h"  
#include "qlcp.h"
#include "qlcc.h" 
#include "qlcs.h"  
#include "qlcl.h"
#include "qlclutil.h"
#include "qlcctrc.h"

extern channel_list_type channel_list;

/*****************************************************************************/
/* Function : qlm_find_ls_in_sap                                             */
/*****************************************************************************/
station_type *qlm_find_ls_in_sap(

  sap_type        *sap,
  unsigned short   netid,
  bool            *unlock)
{
  station_type *station_ptr;
  boolean found = FALSE;
  boolean was_locked;

	/* lock list since we will be traversing it */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);
  *unlock = FALSE;

  if ( (station_ptr=sap->station_list_ptr) != NULL )
  {
    do
    {
      if (station_ptr->netid == netid)
      {
	outputf("QSN: found a station with netid = %d\n",netid);
	found = TRUE;
	*unlock = (lockl(&station_ptr->lock, LOCK_SHORT) == LOCK_SUCC);
      }
    } while (found == FALSE
      && (station_ptr = station_ptr->next_station_ptr) != NULL);
  }

  if (!was_locked) unlockl(&channel_list.lock);
  return(station_ptr);
}

/*****************************************************************************/
/* Function : qlm_find_ls_in_channel                                         */
/*****************************************************************************/
station_type *qlm_find_ls_in_channel(

  channel_type   *channel,
  unsigned short  netid,
  bool           *unlock)
{
  station_type *station_ptr;
  sap_type     *sap_ptr,*sap_list;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (sap_list=channel->sap_list_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    for (
      sap_ptr = sap_list;
      (station_ptr = qlm_find_ls_in_sap(sap_ptr,netid,unlock)) == NULL
      && sap_ptr->next_sap_ptr != NULL;
      sap_ptr = sap_ptr->next_sap_ptr)
        outputf("QSN: checking sap %d\n",sap_ptr);
    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}

/*****************************************************************************/
/* Function : Find ls anywhere                                               */
/*****************************************************************************/
station_type *qlm_find_ls_anywhere(
  
  unsigned short  netid,
  bool           *unlock)
{
/* defect 151159 */
  station_type *station_ptr = NULL;
  channel_type *channel_ptr,*chan;
  boolean was_locked;

  /* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if (channel_ptr=channel_list.channel_ptr) {
    for (
      chan = channel_ptr;
      (station_ptr = qlm_find_ls_in_channel(chan,netid,unlock)) == NULL
      && chan->next_channel_ptr != NULL;
      chan = chan->next_channel_ptr);
  }

  if (!was_locked) unlockl(&channel_list.lock);
  return(station_ptr);
/* end defect 151159 */
}

/*****************************************************************************/
/* Function     qlm_select_netid                                             */
/*                                                                           */
/* Description  This procedure chooses a netid and searches the list         */
/*              of stations to ensure that it is unique.                     */
/*              A netid must bne unique to a station, and the                */
/*              station could be on any SAP, and on any channel.             */
/*              It is therefore necessary to search all                      */
/*              the station lists for all SAPs for all Channels.             */
/*                                                                           */
/*                                                                           */
/* Return       station address if station found                             */
/*              null if station not found                                    */
/*                                                                           */
/* Parameters                                                                */
/*              none                                                         */
/*                                                                           */
/*****************************************************************************/
unsigned short qlm_select_netid(void)

{
  unsigned short netid = 1;
  boolean netid_unique = FALSE;
  station_type *station_ptr;
  bool unlock;

  do
  {
    outputf("QSN: trying netid=%d\n",netid);
    station_ptr = qlm_find_ls_anywhere(netid,&unlock);
    if (station_ptr == NULL)
    {
      netid_unique = TRUE;
    }
    else
    {
      if (unlock) unlockl(&station_ptr->lock);
      netid++;
    }
  } while (netid_unique == FALSE);
  return(netid);
}


/*****************************************************************************/
/* Function     qlm_find_ls_given_netid                                      */
/*                                                                           */
/* Description  This procedure searches the linked list of stations          */
/*              to find the one with the netid passed as an                  */
/*              argument. It returns the address of the station.             */
/*              As a netid is the only key we have to find a                 */
/*              station, then the station could be on any SAP, and on        */
/*              any Channel. It is therefore necessary to search all         */
/*              the station lists for all SAPs for all Channels.             */
/*                                                                           */
/*                                                                           */
/* Return       station address if station found                             */
/*              null if station not found                                    */
/*                                                                           */
/* Parameters                                                                */
/*              netid                                                        */
/*                                                                           */
/*****************************************************************************/
station_type *qlm_find_ls_given_netid(

  unsigned short  netid,
  bool           *unlock)
{
  return(qlm_find_ls_anywhere(netid,unlock));
}

/*****************************************************************************/
/* Function : qlm_find_ls_in_channel_by_corr                                 */
/*****************************************************************************/
station_type *qlm_find_ls_in_channel_by_corr(

  channel_type   *channel,
  correlator_type    correlator,
  bool           *unlock)
{
  station_type *station_ptr=NULL;
  sap_type     *sap_ptr,*sap_list;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (sap_list=channel->sap_list_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    for (
      sap_ptr = sap_list;
      (station_ptr = qlm_find_ls_in_sap_by_corr(sap_ptr,correlator,unlock)) == NULL
      && sap_ptr->next_sap_ptr != NULL;
      sap_ptr = sap_ptr->next_sap_ptr)
        outputf("QSN: checking sap %d\n",sap_ptr);
    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}

/*****************************************************************************/
/* Function : qlm_find_ls_in_sap_by_corr                                     */
/*		Find a link station given correlator and sap                 */
/*                                                                           */
/* Return       station address if station found                             */
/*              null if station not found                                    */
/*                                                                           */
/*****************************************************************************/
station_type *qlm_find_ls_in_sap_by_corr(

  sap_type        *sap,
  correlator_type    correlator,
  bool           *unlock)
{
  station_type *station_ptr;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);
  *unlock = FALSE;

  for ( station_ptr = sap->station_list_ptr;
        station_ptr != NULL;
	station_ptr = station_ptr->next_station_ptr)
  {
      if (correlator == station_ptr->qllc_ls_correlator)
      {
	outputf("QSN: found a station with correlator = %x\n",correlator);
	*unlock = (lockl(&station_ptr->lock, LOCK_SHORT) == LOCK_SUCC);
	break;
      }
  }

  if (!was_locked) unlockl(&channel_list.lock);
  return(station_ptr);
}

/*****************************************************************************/
/* Function     qlm_find_ls_given_correlator                                 */
/*                                                                           */
/* Description  This procedure searches the linked list of stations          */
/*              to find the one with the lcn passed to it as an              */
/*              argument. It returns the address of the station rec.         */
/*              As an lcn is the only key we have to find a station,         */
/*              then the station could be on any SAP, and on                 */
/*              any Channel. It is therefore necessary to search all         */
/*              the station lists for all SAPs for all Channels.             */
/*                                                                           */
/*                                                                           */
/* Return       station address if station found                             */
/*              null if station not found                                    */
/*                                                                           */
/* Parameters                                                                */
/*              correlator                                                   */
/*****************************************************************************/
station_type *qlm_find_ls_given_correlator(

  correlator_type    correlator,
  bool           *unlock)
{
  station_type *station_ptr;
  channel_type *channel_ptr,*chan;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (channel_ptr=channel_list.channel_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    for (
      chan = channel_ptr;
      (station_ptr = qlm_find_ls_in_channel_by_corr(chan,correlator,unlock)) == NULL
      && chan->next_channel_ptr != NULL;
      chan = chan->next_channel_ptr);
    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}

/*****************************************************************************/
/* Function : qlm_find_cont_ls_in_sap                                        */
/*            This function is sensitive to the case of a NULL netid. This   */
/*            means that the call to find contention was on the synchronous  */
/*            thread, on a Start, rather than the asynchronous, Start Done.  */
/*            As NULL is an illegal netid, all stations will be checked.     */
/*            If netid is not NULL, it means there is already a station in   */
/*            in the list, which is the one being compared to, and it will   */
/*            skipped over to avoid self-matching.                           */
/*****************************************************************************/
station_type *qlm_find_cont_ls_in_sap(
  sap_type        *sap,
  char            *addr,
  char            *local,            /* defect 156503 */
  unsigned short   netid,
  bool           *unlock)
{
  station_type *station_ptr;
  boolean found = FALSE;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);
  *unlock = FALSE;

  if ( (station_ptr=sap->station_list_ptr) != NULL )
  {
    do
    {
      if (station_ptr->netid != netid)
      {
	/*********************************************************************/
	/* This function will only report a contending station with a netid  */
	/* different to that passed in as arg. This prevents stations        */
	/* finding themselves in the list.                                   */
	/*********************************************************************/
        /*********************************************************************/
        /* Note that an incomplete (un-called) listening station will not    */
        /* have a valid address in remote_addr, an may well pick up an old   */
        /* calling station's control block if allocate is called straight    */
        /* a free of a calling station. The length field is used to prevent  */
        /* any interpretation of a noisy field like that being treated as a  */
        /* valid address, and causing spurious contention reports.           */
        /*********************************************************************/
        if (station_ptr->remote_addr_len != 0)
        {
/* defect 156503 */
	  if ((strcmp(station_ptr->remote_addr,addr)==0) &&
	      (strcmp(sap->local_x25_address,local)==0))
/* end defect 156503 */
	  {
	    outputf("QSN: found a station with same addr, netid = %d\n",
	      station_ptr->netid);
	    found = TRUE;
	    *unlock = (lockl(&station_ptr->lock, LOCK_SHORT) == LOCK_SUCC);
          }
	}
      }
    } while (found == FALSE
      && (station_ptr = station_ptr->next_station_ptr) != NULL);
  }

  if (!was_locked) unlockl(&channel_list.lock);
  return(station_ptr);
}

/*****************************************************************************/
/* Function : qlm_find_cont_ls_in_channel                                    */
/*****************************************************************************/
station_type *qlm_find_cont_ls_in_channel(

  channel_type   *channel,
  char           *addr,
  char           *local,          /* defect 156503 */
  unsigned short  netid,
  bool           *unlock)
{
  station_type *station_ptr;
  sap_type     *sap_ptr,*sap_list;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (sap_list=channel->sap_list_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    for (
      sap_ptr = sap_list;
/* defect 156503 */
      (station_ptr = qlm_find_cont_ls_in_sap(sap_ptr,addr,
                                             local,netid,unlock)) == NULL
      && sap_ptr->next_sap_ptr != NULL;
/* end defect 156503 */
      sap_ptr = sap_ptr->next_sap_ptr);
    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}

/*****************************************************************************/
/* Function : Find contending ls anywhere                                    */
/*****************************************************************************/
station_type *qlm_find_cont_ls_anywhere(
  char            *addr,
  char            *local,               /* defect 156503 */
  unsigned short   netid,
  bool           *unlock)
{
  station_type *station_ptr;
  channel_type *channel_ptr,*chan;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (channel_ptr=channel_list.channel_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    for (
      chan = channel_ptr;
/* defect 156503 */
      (station_ptr = qlm_find_cont_ls_in_channel(chan,addr,
                                                 local,netid,unlock)) == NULL
      && chan->next_channel_ptr != NULL;
/* end defect 156503 */
      chan = chan->next_channel_ptr);
    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}
/*****************************************************************************/
/* Function     qlm_find_contending_ls                                       */
/*                                                                           */
/* Description  This procedure searches the linked list of stations          */
/*              to find one with the same remote_addr as the                 */
/*              remote station id that is passed to it as an                 */
/*              argument. It returns the address of the station rec.         */
/*              The function accepts two parameters. The first is a          */
/*              correlator, which is used only if the station is already     */
/*              in the station list. This is so that it can avoid returning  */
/*              the address of the station with which it should be comparing */
/*              address. If the station is not already in the map, i.e. this */
/*              is a prelim check before allocating a station struct in the  */
/*              qlclsync.c module's start_ls procedure, then the correlator  */
/*              should be set to NULL.                                       */
/*                                                                           */
/* Return       station address if station found                             */
/*              null if station not found                                    */
/*                                                                           */
/* Parameters                                                                */
/*            correlator of station to be compared against                   */
/*            address - you have to pass this because of case on Start when  */
/*                    - the station is not yet allocated.                    */
/*                                                                           */
/*            The caller passes in one of the parameters. The other can be   */
/*            set to NULL. In the synchronous case, the address is known but */
/*            there can be no station yet. This function accepts a NULL for  */
/*            correlator arg, and does the checking of any stations that ARE */
/*            in the list/s.                                                 */
/*            In the asynchronous case, to save having adrress parms in the  */
/*            main functional code, this function will accept just the       */
/*            correlator of the station, and a NULL for address, and will    */
/*            find out the address itself.                                   */
/*                                                                           */
/*            This function relies on the addresses being null terminated    */
/*****************************************************************************/
station_type *qlm_find_contending_ls(
  sap_type          *sap,      /* pointer to sap, defect 156503 */
  station_type      *station_ptr,
  char              *addr,
  bool           *unlock)
{
  unsigned short  netid;
  station_type   *cont_station;
  char           *local;       /* pointer to local nua, defect 156503 */

  if (station_ptr == NULL)
  {
    /*************************************************************************/
    /* This is the synch case. The station to which we are comparing is not  */
    /* yet in the map, as this is a preliminary check on issue of a Start.   */
    /*************************************************************************/
    netid = NULL;
  }
  else
  {
    /*************************************************************************/
    /* This is the asynch case. This function will fill in the address on    */
    /* behalf of the caller.                                                 */
    /*************************************************************************/
    netid = station_ptr->netid;
    addr = station_ptr->remote_addr;
  }
/* defect 156503 */
  local = sap->local_x25_address;
  cont_station = qlm_find_cont_ls_anywhere(addr,local,netid,unlock);
/* end defect 156503 */
  return(cont_station);
}


/*****************************************************************************/
/* Function     qlm_find_listening_ls                                        */
/*                                                                           */
/* Description  This procedure searches the linked list of stations          */
/*              to find one which is the current listener                    */
/*              It returns the address of the station rec.                   */
/*              The scheme adopted is as follows: The caller knows the port  */
/*              id (as there can only be one listener on a port, and the port*/
/*              id is known as this function is used for routing incoming    */
/*              calls (by the QPM)). Therefore all channels which are open   */
/*              on that port must be scanned for a listener.                 */
/*              A more efficient scheme could be implemented in which the QPM*/
/*              remembers the listener's netid, or the channels are marked   */
/*              to show which has the listener, but this scheme is felt to   */
/*              give the most autonomy between sub-components (at least for  */
/*              the time-being).                                             */
/*                                                                           */
/* Return       station address if station found                             */
/*              null if station not found                                    */
/*                                                                           */
/* Parameters                                                                */
/*              correlator                                                   */
/*                                                                           */
/*****************************************************************************/
station_type *qlm_find_listening_ls(

  port_type *port_id,
  bool           *unlock)
{
  /***************************************************************************/
  /* This function is not used now that the netid is stored in the port.     */
  /***************************************************************************/
  *unlock = FALSE;
  return (NULL);
}

/*****************************************************************************/
/* Function     qlm_remote_stn_id_is_valid                                   */
/*                                                                           */
/* Description  This procedure verifies that a remote station id is          */
/*              valid                                                        */
/*                                                                           */
/* Return       TRUE or FALSE                                                */
/*                                                                           */
/* Parameters                                                                */
/*              remote station id                                            */
/*                                                                           */
/*****************************************************************************/
bool    qlm_remote_stn_id_is_valid(

  char            *address,
  unsigned long    length)
{
  bool rc;
  int loop_index;
  /***************************************************************************/
  /* Initialise local variables                                              */
  /***************************************************************************/
  outputf("QLM_REMOTE_UTIL: rem id len=%d\n",length);
  outputf("QLM_REMOTE_UTIL: rem id add=%s\n",address);
  if (length >=1)
  {
    /*************************************************************************/
    /* All characters in the address are numeric, unless                     */
    /* the vc is a PVC, in which case the first character                    */
    /* is a P/p.                                                             */
    /*************************************************************************/
    if (  address[0] == 'P'
      ||  address[0] == 'p'
      || (address[0] >= '0' && address[0] <= '9')
      )
    {
      /***********************************************************************/
      /* The first character is valid. Now loop along the remainder of the   */
      /* address and test the characters. The loop will terminate when an    */
      /* invalid character is found, or when the entire id has been scanned. */
      /* On termination, the loop index is used to establish whether the     */
      /* loop reached the end of the id, (in which case the id is valid), or */
      /* whether it terminated because it found an invalid character.        */
      /***********************************************************************/
      loop_index = 1;
      while (  loop_index < length
	&& address[loop_index] >='0'
	&& address[loop_index] <='9'
	)
      {
	loop_index++;
      }
      /***********************************************************************/
      /* Test whether loop reached end of id or not                          */
      /***********************************************************************/
      if (loop_index >= length)
      {
	outputf("loop index >= length so address is valid\n");
	rc = TRUE;      /* the id is valid                                   */
      }
      else
      {
	outputf("loop index < length so address invalid\n");
	rc = FALSE;     /* the id is not valid                               */
      }
    }
    else
    {
      /* if length is 0 or first char invalid, the id is invalid             */
      rc = FALSE;
    }
  }
  return(rc);
}


/*****************************************************************************/
/* Function	qlm_lock_list_and_ls					     */
/*                                                                           */
/* Description  This procedure is used to acquire the list lock when already */
/*              already holding a lock for a link station.                   */
/*              The station_ptr is passed in and its correlator is           */
/*              extracted. The list lock is then acquired, and the station   */
/*              correlator is then converted back into a pointer. This       */
/*              may fail if another process deleted the station while this   */
/*              process waited for the list lock.                            */
/*                                                                           */
/* Returns      LOCK_SUCC - both list and LS are now locked                  */
/*              LOCK_NEST - list was already locked by this process          */
/*              LOCK_FAIL - unable to reacquire station lock, station is now */
/*                          gone, list is not locked                         */
/*                                                                           */
/*****************************************************************************/
lock_t
qlm_lock_list_and_ls(
  station_type *station_id)
{
  correlator_type  correlator;
  station_type    *station_ptr;
  lock_t           rc;
  bool             unlock;

  /***************************************************************************/
  /* Get correlator out of station                                           */
  /***************************************************************************/
  correlator = station_id->qllc_ls_correlator;

  /***************************************************************************/
  /* Try without waiting to get the list lock. If successful, just return    */
  /***************************************************************************/
  rc = lockl(&channel_list.lock, LOCK_NDELAY);
  if (rc != LOCK_FAIL)
    return rc;

  /***************************************************************************/
  /* Unlock station now so we can lock the list                              */
  /***************************************************************************/
  unlockl(&station_id->lock);
  lockl(&channel_list.lock, LOCK_SHORT);

  /***************************************************************************/
  /* Now, recover pointer to station, and verify its the same                */
  /* This call also re-acquires the station lock.                            */
  /* If NULL returned, station was deleted while we slept.                   */
  /***************************************************************************/
  station_ptr = qlm_find_ls_given_correlator(correlator, &unlock);
  if (station_ptr == NULL)
  {
    unlockl(&channel_list.lock);
    return LOCK_FAIL;
  }
  else
  {
    return LOCK_SUCC;
  }
}

/*****************************************************************************/
/* Function     qlm_delete_station                                           */
/*                                                                           */
/* Description  This procedure cleans up when an unnrecoverable              */
/*              error condition exists. It calls x25_vc_closed,              */
/*              and qllc_l3nop.                                              */
/*              It then frees the sna_ls.                                    */
/*              It also frees any pending buffers in the ls.                 */
/*              It also makes the station list good, after removing the ls.  */
/*              This procedure is called when things have really gone awry.  */
/*              There is no attempt to do things gracefully, like change     */
/*              state variables to reflect the reason for a closure, etc.    */
/*              For this reason, the calls to the qvm and qqm aren't strictly*/
/*              necessary, because all that is being atempted here is to free*/
/*              any resources, but the calls to qv, and qqm are included for */
/*              "conceptual integrity" (Great!). i.e. If we ever need to add */
/*              calls to qvm/qqm to do cleanup type things, then we'll know  */
/*              where the calls go - here.                                   */
/*              Call qvm_vc_closed to notify it that the vc must be cleared  */
/*              up.                                                          */
/*              Also call QQM to notify it that the level 3 is no longer     */
/*              operational.                                                 */
/*              Finally free the link station resources, and send a station  */
/*              halted result to the user                                    */
/*                                                                           */
/* Return       none                                                         */
/*                                                                           */
/* Parameters                                                                */
/*              station_ptr                                                  */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
void qlm_delete_station(

  station_type *station_id,
  correlator_type other_info,
  gen_buffer_type *buffer_ptr)

/*****************************************************************************/
/* The other_info field is used to pass the correlator of a contending       */
/* station if the close is caused by contention. Otherwise it is NULL.       */
/*****************************************************************************/
/*****************************************************************************/
/* Buffer ptr is used when there is a buffer to be freed by the qvm_vc_closed*/
/* procedure. If no buffer, arg is NULL.                                     */
/*****************************************************************************/

{
  channel_type     *channel_id;
  sap_type         *sap_ptr;
  int               reason_for_closure;
  correlator_type   user_sap_correlator;
  correlator_type   user_ls_correlator;
  correlator_type   qllc_ls_correlator;
  station_type     *station_ptr;
  station_type     *tmp_station_ptr;
  boolean           station_found;
  boolean           no_result = FALSE;
  boolean           list_was_locked;

  /***************************************************************************/
  /* Find which channel and sap this station is on                           */
  /***************************************************************************/
  qllc_ls_correlator = station_id->qllc_ls_correlator;
  channel_id = station_id->channel_id;
  sap_ptr = (sap_type *)station_id->qllc_sap_correlator;

  /***************************************************************************/
  /* Need to get sap lock, which means we must release station lock first.   */
  /* Afterwards, we will need to re-find the station ptr by its correlator   */
  /* to make sure it has not gone away.                                      */
  /***************************************************************************/
  switch (qlm_lock_list_and_ls(station_id))
  {
  case LOCK_SUCC:
    list_was_locked = FALSE;
    break;
  case LOCK_NEST:
    list_was_locked = TRUE;
    break;
  default:	/* station deleted while we waited on lock */
    if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);  /* Defect 120268 */
    return;
    break;
  }

/* defect 149350 */
  /* free any leftover command buffer that was being held for repoll */
  qllc_clear_pending_cmd (&(station_id->link_station));
/* end defect 149350 */

  /***************************************************************************/
  /* Initialise local variables                                              */
  /***************************************************************************/
  reason_for_closure = station_id->reason_for_closure;
  TRACE_HALT(station_id,reason_for_closure);
  user_sap_correlator =
    QSM_RETURN_USER_SAP_CORRELATOR(station_id->qllc_sap_correlator);
  user_ls_correlator = station_id->user_ls_correlator;
  if(station_id->silent_halt == TRUE)
  {
    outputf("QLM_DELETE_STATION: set silent halt\n");
    no_result = TRUE;
  }
  /***************************************************************************/
  /* Update the Virtual Circuit.                                             */
  /* In fact, the call to the qvm is not strictly necessary, as there is     */
  /* nothing that the vc contains, which is not cleared up by freeing the    */
  /* station. i.e. all the resources in the vc are allocated from heap on    */
  /* the allocation of the station, there are no buffers there, or any       */
  /* separately allocated heap storage.                                      */
  /***************************************************************************/
  (void)qvm_vc_closed(
    &(station_id->virt_circuit),
    buffer_ptr
    );
  /***************************************************************************/
  /* Clean up the QLLC Link Station.                                         */
  /* In fact, the call to the qqm is not strictly necessary, as there is     */
  /* nothing that the qllc link station contains, not cleared up by freeing  */
  /* the station.                                                            */
  /* i.e. all the resources in the vc are allocated from heap on the         */
  /* allocation of the station, there are no buffers there, or any           */
  /* separately allocated heap storage.                                      */
  /***************************************************************************/
  /*  outputf("QLM_DELETE_LS: calling qllc_l3nop\n"); */
  /*  (void)qllc_l3nop(&(station_id->link_station)); */
  /***************************************************************************/
  /* The above call was removed, as qllc_clrst is used for incoming clears,  */
  /* and this call to make the fsm go inoperative will result in an error    */
  /* log entry which is not wanted/needed.                                   */
  /***************************************************************************/

  /***************************************************************************/
  /* Release timers                                                          */
  /***************************************************************************/
  outputf("QLM_DELETE_STATION: w_clear() all station timers\n");
  w_stop(&(station_id->link_station.repoll_dog));
/* defect 111172 */
  while (w_clear(&(station_id->link_station.repoll_dog)));
  w_stop(&(station_id->inact_dog));
  while (w_clear(&(station_id->inact_dog)));
  w_stop(&(station_id->halt_dog));
  while (w_clear(&(station_id->halt_dog)));
  w_stop(&(station_id->retry_dog));
  while (w_clear(&(station_id->retry_dog))); 
/* end defect 111172 */
  outputf("QLM_DELETE_STATION: all timers cleared\n");
  /***************************************************************************/
  /* Take station out of linked list                                         */
  /***************************************************************************/
  /***************************************************************************/
  /* The algorithm used is as follows:                                       */
  /***************************************************************************/
  /***************************************************************************/
  /* Sap is locked, so OK to access list                                     */
  /***************************************************************************/
  if (sap_ptr->station_list_ptr == NULL)
  {
    outputf("QLM_DELETE_LS: trivial case - list empty\n");
    /*************************************************************************/
    /* Trivial case - list is empty                                          */
    /*************************************************************************/
    if (!list_was_locked) unlockl(&channel_list.lock);
    return;
  }
  else
  {
    /*************************************************************************/
    /* List is not empty - that's a good start!                              */
    /*************************************************************************/
    station_found = FALSE;

    station_ptr = sap_ptr->station_list_ptr;
    while (station_ptr != NULL && !station_found)
    {
      if (station_ptr == station_id)
      {
        station_found = TRUE;

	/*********************************************************************/
	/* Defect 103652 - Clear the driver DLC_LOCAL_BUSY state. This is    */
	/* as a workaround to the X25 device driver not handling a HALT      */
 	/* properly while the last station is in LOCAL BUSY.		     */
        /*********************************************************************/

        if ((station_ptr->station_sub_state & DLC_LOCAL_BUSY) == DLC_LOCAL_BUSY)
        {
            outputf ("QLM_DELETE_STATION: exit local busy\n");
            (void)qpm_exit_local_busy(
                      QCM_RETURN_PORT_ID(station_ptr->channel_id),
                      QVM_RETURN_SESSION_ID(&(station_ptr->virt_circuit)));
            station_ptr->station_sub_state &= ~DLC_LOCAL_BUSY;
        }

	/*********************************************************************/
	/* End of defect 103652.                                             */
        /*********************************************************************/

	if (station_ptr->next_station_ptr != NULL)
	  station_ptr->next_station_ptr->prev_station_ptr =
		station_ptr->prev_station_ptr;

	if (station_ptr->prev_station_ptr == NULL)
	  sap_ptr->station_list_ptr = station_ptr->next_station_ptr;
	else /* prev_station != NULL */
	  station_ptr->prev_station_ptr->next_station_ptr =
		station_ptr->next_station_ptr;

	/*********************************************************************/
	/* Free the station record, and all associated resources.            */
	/*********************************************************************/
	while (station_ptr->receive_data_queue.first != NULL)
	{
	  outputf("QLM_DELETE_STATION:flush station data queue\n");
          buffer_ptr = QBM_DEQUE_BUFFER(&(station_ptr->receive_data_queue));
	  if (buffer_ptr != NULL) QBM_FREE_BUFFER(buffer_ptr);
	}

/* defect 156503 */
        /*********************************************************************/
        /* If station is a listener, remove it from listen list..            */
        /*********************************************************************/
        if (!(station_ptr->flags & DLC_SLS_LSVC)) {
          outputf("QLM_DELETE_STATION: remove netid from listener list\n");
          rm_listen_netid(station_ptr->netid,
                QCM_RETURN_PORT_ID(station_ptr->channel_id));
        }
/* end defect 156503 */

	tmp_station_ptr = station_ptr;
	station_ptr = station_ptr->next_station_ptr;

	/* always unlock(), since we are about to free */
	unlockl(&(tmp_station_ptr->lock));
	outputf("QLM_DELETE_LS: free station\n");
	if (xmfree((char *)tmp_station_ptr, pinned_heap) != NULL)
	{
	  return;
	}
      }
      else
      {
	station_ptr = station_ptr->next_station_ptr;
      }
    }
    if (station_found == FALSE)
    {
      outputf("QLM_DELETE_LS: station not found\n");
      assert(0);
      return;
    }
  }

  /* unlock list now if it wasn't locked on entry */
  if (!list_was_locked) unlockl(&channel_list.lock);

  if (no_result == FALSE)
  {
    /*************************************************************************/
    /* Send result to user                                                   */
    /*************************************************************************/
    switch (reason_for_closure)
    {
    case remote_name_already_connected :
      outputf("QLM_DELETE_LS: reason for closure = contention\n");
      qcm_make_contention_result(
	channel_id,
	user_sap_correlator,
	user_ls_correlator,
	other_info                                /* contending_station_ptr */
	);
      break;
    default :
      outputf("QLM_DELETE_LS: reason_for_closure = other than contention\n");
      qcm_make_result(
	channel_id,
	user_sap_correlator,
	user_ls_correlator,
	station_halted,
	reason_for_closure
	);
      break;
    }
  }
  return;
}



/*****************************************************************************/
/* Function : qlm_find_repoll_ls_in_sap                                      */
/*****************************************************************************/
station_type *qlm_find_repoll_ls_in_sap(
  sap_type        *sap,
  bool           *unlock)
{
  station_type *station_ptr;
  boolean found = FALSE;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);
  *unlock = FALSE;

  if ( (station_ptr=sap->station_list_ptr) != NULL )
  {
    do
    {
      if (station_ptr->link_station.repoll_due == TRUE)
      {
	outputf("QSN: found a station needing repoll\n");
	found = TRUE;
	*unlock = (lockl(&station_ptr->lock, LOCK_SHORT) == LOCK_SUCC);
      }
    } while (found == FALSE
      && (station_ptr = station_ptr->next_station_ptr) != NULL);
  }

  if (!was_locked) unlockl(&channel_list.lock);
  return(station_ptr);
}
/*****************************************************************************/
/* Function : qlm_find_repoll_ls_in_channel                                  */
/*****************************************************************************/
station_type *qlm_find_repoll_ls_in_channel(
  channel_type   *channel,
  bool           *unlock)
{
  station_type *station_ptr;
  sap_type     *sap_ptr,*sap_list;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (sap_list=channel->sap_list_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    for (
      sap_ptr = sap_list;
      (station_ptr = qlm_find_repoll_ls_in_sap(sap_ptr,unlock)) == NULL
      && sap_ptr->next_sap_ptr != NULL;
      sap_ptr = sap_ptr->next_sap_ptr);
    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}
/*****************************************************************************/
/* Function : Find repoll ls on port                                         */
/*****************************************************************************/
station_type *qlm_find_repoll_ls_on_port(
  port_type *port_id,
  bool           *unlock)
{
  station_type *station_ptr = NULL;
  channel_type *channel_ptr,*chan;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (channel_ptr=channel_list.channel_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    chan = channel_ptr;
    do
    {
      if (chan->port_id == port_id)
      {
	station_ptr = qlm_find_repoll_ls_in_channel(chan,unlock);
      }
    } while((station_ptr==NULL) && ((chan=chan->next_channel_ptr) != NULL));

    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}


/*****************************************************************************/
/* Function : qlm_find_inact_ls_in_sap                                       */
/*****************************************************************************/
station_type *qlm_find_inact_ls_in_sap(
  sap_type        *sap,
  bool           *unlock)
{
  station_type *station_ptr;
  boolean found = FALSE;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);
  *unlock = FALSE;

  if ( (station_ptr=sap->station_list_ptr) != NULL )
  {
    do
    {
      if (station_ptr->inactivity_detected == TRUE)
      {
	outputf("QSN: found an inactive station\n");
	found = TRUE;
	*unlock = (lockl(&station_ptr->lock, LOCK_SHORT) == LOCK_SUCC);
      }
    } while (found == FALSE
      && (station_ptr = station_ptr->next_station_ptr) != NULL);
  }

  if (!was_locked) unlockl(&channel_list.lock);
  return(station_ptr);
}

/*****************************************************************************/
/* Function : qlm_find_inact_ls_in_channel                                   */
/*****************************************************************************/
station_type *qlm_find_inact_ls_in_channel(
  channel_type   *channel,
  bool           *unlock)
{
  station_type *station_ptr;
  sap_type     *sap_ptr,*sap_list;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (sap_list=channel->sap_list_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    for (
      sap_ptr = sap_list;
      (station_ptr = qlm_find_inact_ls_in_sap(sap_ptr,unlock)) == NULL
      && sap_ptr->next_sap_ptr != NULL;
      sap_ptr = sap_ptr->next_sap_ptr)
        outputf("QSN: checking sap %d\n",sap_ptr);
    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}


/*****************************************************************************/
/* Function : Find inact ls on port                                          */
/*****************************************************************************/
station_type *qlm_find_inact_ls_on_port(
  port_type *port_id,
  bool           *unlock)
{
  station_type *station_ptr = NULL;
  channel_type *channel_ptr,*chan;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (channel_ptr=channel_list.channel_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    chan = channel_ptr;
    do
    {
      outputf("QSN: checking channel %d\n",chan);
      if (chan->port_id == port_id)
      {
	station_ptr = qlm_find_inact_ls_in_channel(chan,unlock);
      }
    } while((station_ptr==NULL) && ((chan=chan->next_channel_ptr) != NULL));

    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}

/*****************************************************************************/
/* Function : qlm_find_forced_ls_in_sap                                      */
/*****************************************************************************/
station_type *qlm_find_forced_ls_in_sap(
  sap_type        *sap,
  bool           *unlock)
{
  station_type *station_ptr;
  boolean found = FALSE;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);
  *unlock = FALSE;

  if ( (station_ptr=sap->station_list_ptr) != NULL )
  {
    do
    {
      if (station_ptr->forced_halt_due == TRUE)
      {
	outputf("QSN: found a station pending forced halt\n");
	found = TRUE;
	*unlock = (lockl(&station_ptr->lock, LOCK_SHORT) == LOCK_SUCC);
      }
    } while (found == FALSE
      && (station_ptr = station_ptr->next_station_ptr) != NULL);
  }

  if (!was_locked) unlockl(&channel_list.lock);
  return(station_ptr);
}

/*****************************************************************************/
/* Function : qlm_find_forced_ls_in_channel                                  */
/*****************************************************************************/
station_type *qlm_find_forced_ls_in_channel(
  channel_type   *channel,
  bool           *unlock)
{
  station_type *station_ptr;
  sap_type     *sap_ptr,*sap_list;
  boolean was_locked;

	/* lock list of SAPs in channel */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (sap_list=channel->sap_list_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    for (
      sap_ptr = sap_list;
      (station_ptr = qlm_find_forced_ls_in_sap(sap_ptr,unlock)) == NULL
      && sap_ptr->next_sap_ptr != NULL;
      sap_ptr = sap_ptr->next_sap_ptr);
    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}


/*****************************************************************************/
/* Function : Find forced ls on port                                         */
/*****************************************************************************/
station_type *qlm_find_forced_ls_on_port(
  port_type *port_id,
  bool           *unlock)
{
  station_type *station_ptr = NULL;
  channel_type *channel_ptr,*chan;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (channel_ptr=channel_list.channel_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    chan = channel_ptr;
    do
    {
      if (chan->port_id == port_id)
      {
	station_ptr = qlm_find_forced_ls_in_channel(chan,unlock);
      }
    } while((station_ptr==NULL) && ((chan=chan->next_channel_ptr) != NULL));

    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}


/*****************************************************************************/
/* Function : qlm_find_retry_ls_in_sap                                       */
/*****************************************************************************/
station_type *qlm_find_retry_ls_in_sap(
  sap_type        *sap,
  bool           *unlock)
{
  station_type *station_ptr;
  boolean found = FALSE;
  boolean was_locked;

	/* get list lock */
  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);
  *unlock = FALSE;

  if ( (station_ptr=sap->station_list_ptr) != NULL )
  {
    do
    {
      if (station_ptr->retry_pending == TRUE)
      {
	outputf("QSN: found a station pending a retry\n");
	found = TRUE;
	*unlock = (lockl(&station_ptr->lock, LOCK_SHORT) == LOCK_SUCC);
      }
    } while (found == FALSE
      && (station_ptr = station_ptr->next_station_ptr) != NULL);
  }

  if (!was_locked) unlockl(&channel_list.lock);
  return(station_ptr);
}

/*****************************************************************************/
/* Function : qlm_find_retry_ls_in_channel                                   */
/*****************************************************************************/
station_type *qlm_find_retry_ls_in_channel(
  channel_type   *channel,
  bool           *unlock)
{
  station_type *station_ptr;
  sap_type     *sap_ptr,*sap_list;
  boolean was_locked;

  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (sap_list=channel->sap_list_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    for (
      sap_ptr = sap_list;
      (station_ptr = qlm_find_retry_ls_in_sap(sap_ptr,unlock)) == NULL
      && sap_ptr->next_sap_ptr != NULL;
      sap_ptr = sap_ptr->next_sap_ptr);

    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}


/*****************************************************************************/
/* Function : Find retry ls on port                                          */
/*****************************************************************************/
station_type *qlm_find_retry_ls_on_port(
  port_type *port_id,
  bool           *unlock)
{
  station_type *station_ptr = NULL;
  channel_type *channel_ptr,*chan;
  boolean was_locked;

  was_locked = (lockl(&channel_list.lock, LOCK_SHORT) == LOCK_NEST);

  if ( (channel_ptr=channel_list.channel_ptr)==NULL )
  {
    if (!was_locked) unlockl(&channel_list.lock);
    return(NULL);
  }
  else
  {
    chan = channel_ptr;
    do
    {
      if (chan->port_id == port_id)
      {
	station_ptr = qlm_find_retry_ls_in_channel(chan,unlock);
      }
    } while((station_ptr==NULL) && ((chan=chan->next_channel_ptr) != NULL));
    if (!was_locked) unlockl(&channel_list.lock);
    return(station_ptr);
  }
}
