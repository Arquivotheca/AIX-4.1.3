static char sccsid[] = "@(#)87	1.26.1.4  src/bos/kernext/dlc/sdl/sdlcommon.c, sysxdlcs, bos41J, 9520B_all 5/18/95 18:28:58";
  
/*
 * COMPONENT_NAME: (SYSXDLCS) SDLC Data Link Control
 *
 * FUNCTIONS: sdlcommon.c
 *      create_ls()
 *      delete_ls()
 *      add_ls()
#ifdef MULT_PU
 *      add_retry()
 *      del_retry()
 *      add_sta_to_list()
#endif
 *      pop_ls()
 *      clear_buffer()
 *      in_list()
 *      exists()
#ifdef MULT_PU
 *      mpu_remove_station()
 *      run_excepts()
#endif
 *      sdl_abort()
 *      clear_list()
 *      sdlmonitor()
 *      update_counter()
 *      pl_blowout()
 *      sdl_close()
 *      close_device()
 *      len_port_cb()
 *      max_opens()
 *      transmit_trace()
 *      timer_trace()
 *      session_trace()
 *      receive_trace()
 *      performance_trace()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/

/*
**      File Name      : 87
**
**      Version Number : 1.20
**      Date Created   : 92/12/15
**      Time Created   : 11:32:23
*/

#ifdef MULT_PU
	#include <sys/poll.h>
#endif

#include "sdlc.h"

   #define MAX_SDLC_OPENS       1



/************************************************************************/
/*									*/
/*	common sdlc routines (called by more than one module)		*/
/*									*/
/************************************************************************/

				/* functions declared in this module	*/

LINK_STATION	*create_ls();	/* create an sdlc link station ctl blk	*/
int		delete_ls();	/* remove link station from a poll list	*/
void		add_ls();	/* add the link station to a poll list	*/
#ifdef MULT_PU
void		add_sta_to_list(); /* add an mpu station to list */
void		add_retry();       /* add retry entry for mpu    */
void		del_retry();       /* delete a retry entry for mpu */
#endif
LINK_STATION	*pop_ls();	/* remove top station from a poll list	*/
void		clear_buffer();	/* free all buffers in transmit queue	*/
int		in_list();	/* queries list for specified station	*/
int		exists();	/* queries all lists for spec. station	*/
#ifdef MULT_PU
void            mpu_remove_station(); /* remove and free an mpu station */
void            run_excepts(); /* run thru channels and issue exceptions */
#endif
void		sdl_abort();	/* aborts all logical links		*/
void		clear_list();	/* clear a poll list			*/
void		sdlmonitor();	/* write an internal trace record	*/
int		update_counter();
void		pl_blowout();	/* close physical link (event driven)	*/
void		sdl_close();	/* start procedures to close pl		*/
void		close_device();	/* issue close command to dh		*/
int		len_port_cb();	/* returns the size (bytes) of the port	*/
				/* control block			*/
int		max_opens();	/* returns max number of opens allowed	*/
void		transmit_trace();
void		timer_trace();
void		session_trace();
void		receive_trace();
void		performance_trace();
/************************************************************************/





/************************************************************************/
/*									*/
/* Function								*/
/* Name:	create_ls						*/
/*									*/
/* Description: Create an sdlc link station				*/
/*									*/
/* Function:	Malloc the space needed for a new link station. 	*/
/*									*/
/* Input:	None							*/
/*									*/
/* Output:	Returns a pointer to the new link station		*/
/*									*/
/* Normal Exit: Return pointer to new link station			*/
/*									*/
/* Error Exit:	Return code 0						*/
/*									*/
/* Return Type: LINK_STATION *						*/
/*									*/
/************************************************************************/

LINK_STATION	*create_ls()

{
	LINK_STATION	*temp;

	temp = (LINK_STATION *) malloc(sizeof(LINK_STATION));

	if (temp)
	{
		bzero(temp, sizeof(LINK_STATION));
		return(temp);	  
	}
	else
		return(0);

}	/*****************************************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	delete_ls						*/
/*									*/
/* Description: Delete sdlc link station				*/
/*									*/
/* Function:	The address of the station to be removed is passed to	*/
/*		the subroutine.  If the station was deleted sucessfully */
/*		and there are still more items on the list, then a 0	*/
/*		is returned. If either the list pointer or station to	*/
/*		be deleted are NULL, a -1 is returned.			*/
/*									*/
/* Input:	The address of a link station node			*/
/*									*/
/* Output:	Updated station list					*/
/*									*/
/* Normal Exit: 0  - Sucessfully deleted the link station		*/
/*									*/
/* Error Exit:	-1 - One of the paramaters passed in was NULL		*/
/*		-2 - The specified station is not in the list		*/
/*									*/
/* Return Type: int							*/
/*									*/
/************************************************************************/

int	delete_ls(cb, list, station)

PORT_CB			*cb;
struct	poll_list	*list;
LINK_STATION		*station;

#define NULL_POINTER	-1
#define NOT_FOUND	-2

{

	LINK_STATION	*temp;
	int	return_code;

	/* scan the list and make sure station exists in the list	*/
	temp = list->head;
	while (temp != station && temp)
		temp = temp->next;

	if (temp == NULL)
	{
		sdlmonitor(cb, USER_ERROR, station, list, "RMLS", 4);
		error_log(cb, ERRID_SDL8068, NON_ALERT, 0, FN, LN);
		return(NOT_FOUND);
	}

	/* case one: At least one param is NULL */
	if (((list->head) == NULL) || (station == NULL))
		return(NULL_POINTER);

	return_code = 0;

	/* case two: the station is the only item on the list	*/
	if (list->head == list->tail)
	{
		list->head = NULL;
		list->tail = NULL;
	}
	/* case three: station first item on list */
	else if (list->head == station)
	{
		list->head = station->next;
		list->head->back = NULL;
	}
	/* case four: station at end of list */
	else if (list->tail == station)
	{
		list->tail = station->back;
		station->back->next = NULL;
	}
	/* case five: station in middle of list */
	else
	{
		station->back->next = station->next;
		station->next->back = station->back;
	}

	--(list->count);
	station->next = NULL;
	station->back = NULL;

	return(return_code);

}	/*****************************************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	add_ls							*/
/*									*/
/* Description: Add an existing link station to a new list		*/
/*									*/
/* Function:	Add an existing link station to the bottom of the	*/
/*		specified list. 					*/
/*									*/
/* Input:	station list pointer (poll, idle, quiesce)		*/
/*		link station pointer					*/
/*									*/
/* Output:	updated station list					*/
/*									*/
/* Normal Exit: return from call					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Return Type: void							*/
/*									*/
/************************************************************************/

void	add_ls(cb, list, station)

PORT_CB			*cb;
struct	poll_list	*list;
LINK_STATION		*station;

{
	
	/* make sure station is not already in the list */
	if (in_list(list, station))
	{
		sdlmonitor(cb, USER_ERROR, station, list, "ADLS", 4);
		error_log(cb, ERRID_SDL8069, NON_ALERT, 0, FN, LN);
		return;
	}

	/* if poll mode is null, then set it to active */
	if (!station->poll_mode)
		station->poll_mode = ACTIVE;

#ifdef MULT_PU
	/* add station to list */
	add_sta_to_list(cb, list, station);
#endif

	/*
	** if station is being added to the active list, then set the
	** to the no_active flag to false
	*/
	if (list == &cb->active_list)
		cb->flags.no_active = FALSE;
	/*
	** if the station is being added to the quiesce list and it
	** there are no stations on the active list, then set the
	** no_active flag to TRUE
	*/
	if (list == &cb->quiesce_list && cb->active_list.count == 0)
		cb->flags.no_active = TRUE;

	/*
	** if the station being added is the active_ls
	** then zero the active_ls field
	*/
	if (station == cb->active_ls)
		cb->active_ls = NULL;

}	/*** end of add_ls ***********************************************/


#ifdef MULT_PU
/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:    add_retry                                                   */
/*                                                                      */
/* Description: Add a link station to the retry list                    */
/*                                                                      */
/* Function: Add a link station to the retry list anchored in cb.       */
/*                                                                      */
/* Input:   control block pointer                                       */
/*          link station pointer                                        */
/*                                                                      */
/* Output:  updated retry list                                          */
/*                                                                      */
/* Normal Exit: return from call                                        */
/*                                                                      */
/* Error Exit:  none                                                    */
/*                                                                      */
/* Return Type: void                                                    */
/*                                                                      */
/************************************************************************/
void add_retry (cb, station)
PORT_CB      *cb;
LINK_STATION *station;
{
	/* double check that station is not already on list. If all mbuf
	   pointers are NULL, this is presumed to be the case */
	if ((station->x_m == NULL) && (station->i_m == NULL)) { 
		/* check if list is empty */
		if (cb->retry_list != NULL) {
			/* put station at front of list */
			station->retry_next = cb->retry_list;
			/* point back to station */
			cb->retry_list->retry_back = station;
		}
		/* point the anchor to new list regardless of emptiness */
		cb->retry_list =  station;
	}
}
#endif 


#ifdef MULT_PU
/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:    del_retry                                                   */
/*                                                                      */
/* Description: Delete a link station from the retry list               */
/*                                                                      */
/* Function: Delete a link station from the retry list.                 */
/*                                                                      */
/* Input:   control block pointer                                       */
/*          link station pointer                                        */
/*                                                                      */
/* Output:  updated retry list                                          */
/*                                                                      */
/* Normal Exit: return from call                                        */
/*                                                                      */
/* Error Exit:  none                                                    */
/*                                                                      */
/* Return Type: void                                                    */
/*                                                                      */
/************************************************************************/
void del_retry (cb, station)
PORT_CB      *cb;
LINK_STATION *station;
{
	/* Check all of the mbuf pointers for NULL. If all pointers are NULL
	   the station can come out of the retry list */
	if ( (station->i_m == NULL) && (station->x_m == NULL)) {
		/* make sure not last element */
		if (station->retry_next != NULL) {
			/* make sure not first element */
			if (station->retry_back != NULL) {
				/* case where station is in middle of list */
				station->retry_back->retry_next =
							 station->retry_next;
				station->retry_next->retry_back =
							 station->retry_back;
			}
			else /* case where first (but not only) station
				in list */
			{
				station->retry_next->retry_back = NULL;
				cb->retry_list = station->retry_next;
			}
		}
		else /* it's the last or only element on the list */
		{
			if (station->retry_back  != NULL) {
				/* case where element is last on list */
				station->retry_back->retry_next = NULL;
			}
			else {
				/* case  where only element on list */
				cb->retry_list = NULL;
			}
		}
		station->retry_next = NULL;
		station->retry_back = NULL;
	} /* end of if all ptrs NULL */			
}
#endif

#ifdef MULT_PU
/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:    add_sta_to_list                                             */
/*                                                                      */
/* Description: Add an existing link station to a new list              */
/*                                                                      */
/* Function:    Add an existing link station to the bottom of the       */
/*              specified list.                                         */
/*                                                                      */
/* Input:   control block pointer                                       */
/*          pointer to head of station list                             */
/*          link station pointer                                        */
/*                                                                      */
/* Output:  updated station list                                        */
/*                                                                      */
/* Normal Exit: return from call                                        */
/*                                                                      */
/* Error Exit:  none                                                    */
/*                                                                      */
/* Return Type: void                                                    */
/*                                                                      */
/************************************************************************/

void    add_sta_to_list(cb, list, station)

PORT_CB         *cb;
struct  poll_list   *list;
LINK_STATION        *station;

{
    /* if list is empty */
    if (list->count == 0)
    {
        list->head = station;
        list->tail = station;
    }

    else /* list is not empty */
    {
	/* add station to bottom of list  */
        list->tail->next = station;
        station->back = list->tail; /* connect back pointer */
        list->tail = station;
    }

    ++(list->count);
}

/************ end add_sta_to_list***************/
#endif




/************************************************************************/
/*									*/
/* Function								*/
/* Name:	pop_ls							*/
/*									*/
/* Description:	remove link station from top of station list		*/
/*									*/
/* Function:	remove the link station from the top of the specified	*/
/*		poll list						*/
/*									*/
/* Input:	pointer to poll list					*/
/*									*/
/* Output:	pointer to link station 				*/
/*									*/
/* Normal Exit:	return from call					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Return Type:	pointer to type LINK_STATION				*/
/*									*/
/************************************************************************/

LINK_STATION	*pop_ls(list)

struct	poll_list	*list;

{
	LINK_STATION	*station;

	if (list->head == NULL)
		station = NULL;

	else if (list->head == list->tail)
	{
		station = list->head;
		list->head = NULL;
		list->tail = NULL;
		--(list->count);
		station->next = NULL;
		station->back = NULL;
	}
	else
	{
		station = list->head;
		list->head = station->next;
		--(list->count);
		station->next = NULL;
		station->back = NULL;
	}


	return(station);

}	/**** end of pop_ls *********************************************/


/************************************************************************/
/*									*/
/* Function								*/
/* Name:	clear_buffer						*/
/*									*/
/* Description: clear buffer						*/
/*									*/
/* Function:	Free all the transmission buffers in the transmission	*/
/*		queue.							*/
/*		Set logical link status to CLOSED			*/
/*									*/
/* Input:	Pointer to the control block				*/
/*		Pointer to the station that is being cleared		*/
/*									*/
/* Output:	Empty transmission queue				*/
/*		Buffers returned to the buffer pool			*/
/*									*/
/* Normal Exit: Return from call					*/
/*									*/
/* Error Exit:	None							*/
/*									*/
/* Return Type: void							*/
/*									*/
/************************************************************************/

void	clear_buffer(cb, station)

PORT_CB *cb;
LINK_STATION	*station;

{

	int	i;

	if (station->ll_status != CLOSED)
	{
		station->ll_status = CLOSED;
		--(cb->num_opened);
	}

	/* if the active list is empty */
	if (cb->active_list.count == 0)
	{
		cb->flags.no_active = TRUE;

		/* if there are no link stations opened at all */
		if (cb->num_opened == 0)
			/* then reset the station type */
			cb->station_type = 0;
	}

	if (station->mode == NRM)
		station->mode = NDM;

	/* free all the transmission buffers	*/
	i = station->ack_nr;
	while(station->in != i)
	{
		m_freem(station->tx_que[i].m);
		station->tx_que[i].m = NULL;
		/* removed station->tx_que[i].held = FALSE; 103136 */
		IMOD(i);
	}

	/* Clear the XID buffer and the test buffer */
	if (station->xid.m)
	{
		m_freem(station->xid.m);
		station->xid.m = NULL;
	}

	if (station->test.m)
	{
		m_freem(station->test.m);
		station->test.m = NULL;
	}
#ifdef MULT_PU
	/* Free any retry mbufs held by station */
	if (station->x_m != NULL) {
		m_freem (station->x_m);
		station->x_m = NULL;
	}
	if (station->i_m != NULL) {
		m_freem (station->i_m);
		station->i_m = NULL;
	}
	del_retry (cb,station);  /* delete retry entry from list if needed */
	
	/* wakeup the process sleeping on write (Access through station) */
	if (station->cid->writesleep != EVENT_NULL) 
		e_wakeup (&(station->cid->writesleep));

	/* wakeup the process if sleeping on a read */
	if (station->cid->readsleep != EVENT_NULL)
		e_wakeup((int *)&(station->cid->readsleep));

}       /**** end of clear_buffer ***************************************/
#endif



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	in_list 						*/
/*									*/
/* Description: Tests if the station is in the specified list		*/
/*									*/
/* Function:	The routine searches through the station list for	*/
/*		the link station address it is passed.	If the station	*/
/*		is found, then the routine returns TRUE otherwise	*/
/*		it returns FALSE.					*/
/*									*/
/* Input:	station address, list pointer				*/
/*									*/
/* Output:	TRUE or FALSE						*/
/*									*/
/* Normal Exit: return TRUE or FALSE					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Return Type: int							*/
/*									*/
/************************************************************************/

int	in_list(list, station)

struct	poll_list	*list;
LINK_STATION		*station;

{

	LINK_STATION	*temp;

	temp = list->head;
	while (temp && (temp != station))
		temp = temp->next;

	if (temp)
		return(TRUE);
	else
		return(FALSE);

}	/*****************************************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	exists							*/
/*									*/
/* Description:	checks for the existance of the specified station	*/
/*									*/
/* Function:	search all the station lists (poll, idle, quiesce)	*/
/*		for the specified station, if it is found, return	*/
/*		the address of the list pointer, otherwise return	*/
/*		FALSE							*/
/*									*/
/* Input:	port control block, station pointer			*/
/*									*/
/* Output:	address of list pointer or FALSE			*/
/*									*/
/* Normal Exit:	return from call					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Return Type:	int							*/
/*									*/
/************************************************************************/

int	exists(cb, station)

PORT_CB	*cb;
LINK_STATION	*station;

{
	LINK_STATION	*temp;
	int		found;

	/* check for NULL link station address */
	if (station == NULL)
		return(FALSE);

	found = FALSE;

	/* if station is the active link station */
	if (station == cb->active_ls)
	{
		/* then return true immediatly */
		return(TRUE);
	}

#ifdef MULT_PU
	/* if operating in mpu mode */
	if (cb->flags.mpu_enabled)
	{
		/* check the mpu station list */
		/* if there is at least one station in the list */
		temp = cb->mpu_sta_list.head;
		while (temp && (temp != station))
			temp = temp->next;
		/* if a station match was found */
		if (temp != NULL)
			/* return true (no list pointer is needed */
			return(TRUE);
	}
#endif
	/* check poll list */
	temp = cb->active_list.head;
	while (temp && (temp != station))
		temp = temp->next;

	/* if station not in active list */
	if (temp == NULL)
	{

		/* then check quiesce list */
		temp = cb->quiesce_list.head;
		while (temp && (temp != station))
			temp = temp->next;

		/* if station in quiese list */
		if (temp)
		{
			/* then set found to TRUE */
			found = (int) &cb->quiesce_list;
		}
	}
	else
		found = (int) &cb->active_list;

	return (found);

}	/**** end of exists *********************************************/



#ifdef MULT_PU
/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:    mpu_remove_station                                          */
/*                                                                      */
/* Description: Free any mbufs, remove from station list, and           */
/*              free the station cb.                                    */
/*                                                                      */
/* Input:   control block pointer                                       */
/*          link station pointer                                        */
/*                                                                      */
/* Output:  updated station list                                        */
/*                                                                      */
/* Normal Exit: return from call                                        */
/*                                                                      */
/* Error Exit:  none                                                    */
/*                                                                      */
/* Return Type: void                                                    */
/*                                                                      */
/************************************************************************/
void mpu_remove_station (cb, station)
PORT_CB      *cb;
LINK_STATION *station;
{
        clear_buffer (cb,station);  /* free any held mbufs */

        cb->active_ls = NULL ; /* defect 109826 */

	/* NULL out array entry */
	cb->link_station_array[station->ll.sdl.secladd] = NULL;

	/* delete station from the station list */
	delete_ls (cb, &(cb->mpu_sta_list), station);

	/* if link trace is enabled.		*/
	if ((station->ll.sls.flags & DLC_TRCO) != FALSE)
	{
		/* call session trace routine. */
		session_trace(cb, TRACE_CLOSE);
	}
	/* free up the station */
	free(station);

}  /****** End of mpu_remove_station *****/
#endif

#ifdef MULT_PU
/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:    run_excepts                                                 */
/*                                                                      */
/* Description: Find the anchor to the list of channels for the given   */
/*              port.  Run through the list and compare the port id in  */
/*              the channel cb with the current port id.                */
/*              If they match, send an exception to that cid. This is   */
/*              called on catastrophic errors and is needed to cover    */
/*              the applications sleeping on a select in the case of    */
/*              multiple opens on a single port.                        */
/*                                                                      */
/* Input:   control block pointer                                       */
/*          getx arg pointer                                            */
/*                                                                      */
/* Output:  none.                                                       */
/*                                                                      */
/* Normal Exit: return from call                                        */
/*                                                                      */
/* Error Exit:  none                                                    */
/*                                                                      */
/* Return Type: void                                                    */
/*                                                                      */
/************************************************************************/
void run_excepts(cb, st_block)
PORT_CB *cb;
struct dlc_getx_arg *st_block;
{
struct dlc_chan *ptr;

	/* Find the channel anchor by moving back in the list */
	ptr = cb->dlc.cid;    /* start at the current channel, which may
				 be in the middle of the list */

	/* back up until the 1st channel on the list is located */
	while (ptr->bmpx != NULL)
		ptr = ptr->bmpx;

	/* loop through the complete list of channels */
	while (ptr != NULL)
	{
		/* if this channel is using the target port */
		if ((PORT_CB *)ptr->cb == cb)     /* defect 141966 */
			/* then issue the exception */
			(*ptr->excp_fa) (st_block, ptr);
		/* bump to the next channel in the chain */
		ptr = ptr->mpx;
	}
}
/******* end run_excepts() ****************/
#endif

/************************************************************************/
/*									*/
/* Function								*/
/* Name:	sdl_abort						*/
/*									*/
/* Description: Abort all sdlc logical links				*/
/*									*/
/* Function:	Close all logical links 				*/
/*		Clean transmission buffers				*/
/*		Cancel timer(s) 					*/
/*		Remove link station from poll list			*/
/*									*/
/* Input:	Pointer to the control block structure			*/
/*									*/
/* Output:	Abort_pending flag is changed				*/
/*		Logical_link_status is set to closed			*/
/*									*/
/* Normal Exit: Return from call					*/
/*									*/
/* Error Exit:	None							*/
/*									*/
/* Externs:	None							*/
/*									*/
/* Called by:	Nrm_setup						*/
/*		Device_handler_ack					*/
/*									*/
/* Return Type: void							*/
/*									*/
/************************************************************************/

void	sdl_abort(cb)

PORT_CB *cb;

{

#ifdef MULT_PU
	/* if the port's station type is secondary */
	if (cb->station_type == SECONDARY)
	{
		/* loop thru all the link stations */
		while (cb->mpu_sta_list.head != NULL)
		{
			/* remove the station */
			mpu_remove_station (cb,cb->mpu_sta_list.head);
		}
	}
	else /* not secondary */
	{
		/* if the port's station type is primary */
		if (cb->station_type == PRIMARY)
		{
			/* cancel idle poll list timer */
			DISABLE(cb->idle_timer);

			/* close all logical links */

			/* if active link station is not in any list */
			if (cb->active_ls)
			{
				/* then add it to the active list for
				   processing */
				add_ls(cb, &cb->active_list, cb->active_ls);
				cb->active_ls = NULL;
			}

			if (cb->active_list.count != 0)
				clear_list(cb, &cb->active_list);

			if (cb->quiesce_list.count != 0)
				clear_list(cb, &cb->quiesce_list);

			cb->station_type = 0;
		}
		else /* the port's station type not determined yet */
		{
			/* there's no stations so just fall thru */
		}
	}
#endif

}	/**** end of sdl_abort ******************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	clear_list						*/
/*									*/
/* Description: Clear station list that is passed in			*/
/*									*/
/* Function:	Traverse the list passed in, and free each station's	*/
/*		buffers. Cancel the abort timer and remove the station. */
/*									*/
/* Input:	Station (poll) list					*/
/*									*/
/* Output:	Abort timer cancelled					*/
/*									*/
/* Normal Exit: return from call					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Return Type: void							*/
/*									*/
/************************************************************************/

void	clear_list(cb, list)

PORT_CB *cb;
struct	poll_list    *list;	/* pointer to station list to clear	*/

{
	LINK_STATION	*station;   /* current station in list	    */

	station = list->head;

	while (station != NULL)
	{

		/* if logical link status is not closed */
		if (station->ll_status != CLOSED)
		{
			station->ll_status = ABORTED;

			/* cancel abort timer. */
			if (station->abort_running)
			{
				/* disable the abort timer	*/
				DISABLE(station->abort_timer);
				station->abort_running = FALSE;
			}
/* defect mbufs 160103 */
			/* clear all transmission buffers */
			/* associated with this link.	  */
			clear_buffer(cb, station);
/* end defect mbufs 160103 */

			/* if link trace is enabled.		*/
			if ((station->ll.sls.flags & DLC_TRCO) != FALSE)
			{
				/* call session trace routine. */
				session_trace(cb, TRACE_CLOSE);
			}

			/* remove this instance from  */
			/* poll lists.		     */
			delete_ls(cb, list, station);

#ifdef MULT_PU
			/* NULL out array entry (for primary) */
			cb->link_station_array[station->ll.sls.raddr_name[0]] = NULL;

			/* NULL active_ls if it's the current station
			   about to be deleted */
			if (cb->active_ls == station)
				cb->active_ls = NULL;
#endif
			free(station);
			station = list->head;

		}
		else
			station = station->next;
	}

}	/*****************************************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	sdlmonitor						*/
/*									*/
/* Description: write a trace record into the internal trace table	*/
/*									*/
/* Function:	The calling subroutine passes any operation specific	*/
/*		information, and this routine writes makes an entry	*/
/*		into the internal (monitor) trace table 		*/
/*									*/
/* Input:	Trace type, link station address (i.e. correlator),	*/
/*		operation return code, operation specific data (up	*/
/*		to 8 bytes), and the length of the operation data.	*/
/*									*/
/* Output:	Write a record to the trace table			*/
/*									*/
/* Normal Exit: Return from call					*/
/*									*/
/* Error Exit:	None							*/
/*									*/
/* Return Type: Void							*/
/*									*/
/************************************************************************/

void	sdlmonitor(cb, type, correlator, results, data, data_len)

PORT_CB *cb;
uchar	*type;			/* 4 char mnemonic			*/
ulong	correlator;		/* link station or other addr		*/
ulong	results;		/* op return code or any int data	*/
uchar	*data;			/* operation specific data		*/
ulong	data_len;		/* length of data field being passed	*/

{
	ulong	trace_word1;
	ulong	trace_word2;
	ulong	trace_word3;
	ulong	trace_word4;
	ulong	port;

	/*
	** save in internal trace table
	*/
	bcopy(type, cb->trace_tbl[cb->trace_index].type, TYPE_BYTES);
	cb->trace_tbl[cb->trace_index].corr = correlator;
	cb->trace_tbl[cb->trace_index].results = results;

	/*
	** create an entry in the AIX generic trace
	*/
	bcopy(type, &trace_word1, 4);
	bcopy(cb->dlc.namestr, &port, 4);
	trace_word2 = correlator;
	trace_word3 = results;
	trace_word4 = 0;

	if (data_len)	
	{
		bcopy(data, cb->trace_tbl[cb->trace_index].data,  data_len);
		bcopy(data, &trace_word4, data_len);
	}

	/*
	** save the generic trace entry
	*/
	trchkgt(HKWD_SYSX_DLC_MONITOR | ((0x08 << 8) | DLC_DL_SDLC),
		trace_word1, trace_word2, trace_word3, trace_word4, port);

	/* bump the trace table pointer */
	cb->trace_index = (cb->trace_index + 1) % TRACE_TBL_SIZE;

}	/*****************************************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	update_counter						*/
/*									*/
/* Description:	update the RAS and threshold counters passed in 	*/
/*									*/
/* Function:	If the RAS counter is less than the maximum value	*/
/*		then the counter will be updated, and NORMAL will be	*/
/*		returned, if the RAS counter has reached it's sample	*/
/*		limit, then REACHED_LIMIT will be returned		*/
/*									*/
/* Input:	address of counters to be updated			*/
/*									*/
/* Output:	updated counter 					*/
/*									*/
/* Normal Exit:	NORMAL return code (counter was updated)		*/
/*		REACHED_LIMIT	   (counter reached sample limit)	*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Return Type:	int							*/
/*									*/
/************************************************************************/

int	update_counter(ras_counter, threshold)

ulong	*ras_counter;			/* RAS counter to be updated	*/
ulong	*threshold;			/* threshhold count		*/

{
	int	rc;

	rc = NORMAL;

	if (*ras_counter < (ulong) MAX_COUNT)
		++(*ras_counter);
	else
		rc = REACHED_LIMIT;

	++(*threshold);

	return(rc);

}	/**** end of update_counters ************************************/



/************************************************************************/
/*									*/
/* Function								*/
/* Name:	pl_blowout						*/
/*									*/
/* Description:	physical link blowout					*/
/*									*/
/* Function:	This routine gets called when the physical link needs	*/
/*		to be closed due to some event. 			*/
/*									*/
/* Input:	SDLC control block					*/
/*									*/
/* Output:	none							*/
/*									*/
/* Normal Exit:	return from call					*/
/*									*/
/* Error Exit:	none							*/
/*									*/
/* Return Type:	void							*/
/*									*/
/************************************************************************/

void	pl_blowout(cb)

PORT_CB	*cb;

{

	int	rc;


	/* if physical link has not been started */
	if ((cb->pl_status == CLOSED) && (!cb->flags.phy_starting))
	{
		/* then ignore the request */
	}


	/* if physical link is opened	*/
	else if ((cb->pl_status == OPENED) ||
		((cb->pl_status == CLOSED) && (cb->flags.phy_starting)))
	{
		/*
		** enqueue halt command to device handler
		** set pl_status to indicate closing
		*/

		rc = fp_ioctl(cb->fp, CIO_HALT, &cb->session, NULL);

		if (cb->sdllc_trace)
			sdlmonitor(cb, "HALT_DEVICE", 0, rc, "PLBO", 4);

		if (rc)
		{
			error_log(cb, ERRID_SDL8051, NON_ALERT, 0, FN, LN);
			cb->rc = rc;
		}

		/* set physical link closing status */
		cb->pl_status = CLOSING;
	}


}	/**** end of pl_blowout *****************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	sdl_close                                               */
/*                                                                      */
/* Description:	close the physical link                                 */
/*                                                                      */
/* Function:	If the physical link is not in a CLOSING state, then    */
/*              this is an abnormal close, so set the sap abort flag    */
/*              and halt the adapter and close all of the logical links */
/*		If the physical link is CLOSING, then clean up the link	*/
/*		and wake up the head code when completed		*/
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*                                                                      */
/* Output:	updated link status                                     */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void	sdl_close(cb)

PORT_CB	*cb;

{
/* defect 101311 */
	/* if there is no device file pointer (open failure) */
	if (!cb->fp)
	{
/* defect 115819 */
		cb->dlc.term_kproc = TRUE;
/* end defect 115819 */
/* defect 141966 */
		sdlmonitor(cb, "TERM", 0, 0, "NODV", 4);
		/* removed the delay and e_wakeup to the last breath of the kproc,
		   so that the head code cannot remove the port control block
                   prior to close completion */
/* end defect 141966 */
		return;
	}
/* end defect 101311 */

#ifdef  MULT_PU
	/* if the device handler must be halted/closed */
	if ( (cb->dlc.chan_count == 1)  || (cb->sap_cid == cb->dlc.kcid) )
	{
#endif
	
/* defect 141966 */
		/* then just close the device */
		close_device(cb);
/* end defect 141966 */
#ifdef MULT_PU
	}
	else /* this is a close that is not the sap owner and is not
		the last close for the port */
	{
		/* insure that dlcmpx is able to issue e_sleep
		   by forcing the dispatcher to cycle the ready
		   processes */
		do
		{
			delay(10);
		} while (cb->dlc.kcid->proc_id == EVENT_NULL);

		/* wake up the head code process */
/* defect 101311, 141966 */
		sdlmonitor(cb, "WAKE", cb->dlc.chan_count, 0, "NOTL", 4);
/* end defect 101311, 141966 */
		e_wakeup((int *)&cb->dlc.kcid->proc_id);
	}
#endif

}	/**** end of sdl_close ******************************************/



/************************************************************************/
/*                                                                      */
/* Function                                                             */
/* Name:	close_device                                            */
/*                                                                      */
/* Description:	close multiprotocol adapter                             */
/*                                                                      */
/* defect 115819                                                        */
/* Function:    Send a close command to the multiprotocol adapter, set  */
/*              the term_kproc flag so that the link_manager process    */
/* end defect 115819                                                    */
/*              will terminate.                                         */
/*                                                                      */
/* Input:	pointer to the port control block                       */
/*                                                                      */
/* Output:	updated link status                                     */
/*                                                                      */
/* Normal Exit:	return from call                                        */
/*                                                                      */
/* Error Exit:	none                                                    */
/*                                                                      */
/* Return Type:	void                                                    */
/*                                                                      */
/************************************************************************/

void	close_device(cb)

PORT_CB	*cb;

{
	int	rc=0;

	/* send close command */

	/* if the device has been opened */
	if (cb->flags.device_opened)
		rc = fp_close(cb->fp);

	if (cb->sdllc_trace)
		sdlmonitor(cb, CLOSE_DEVICE, 0, rc, 0, 0);

	/* if the close did not complete */
	if (rc)
		error_log(cb, ERRID_SDL8062, NON_ALERT, 0, FN, LN);
		
	cb->flags.device_opened = FALSE;
	cb->pl_status = CLOSED;

#ifdef MULT_PU
	/* if it is the last close */
	if (cb->dlc.chan_count == 1)
/* defect 141966 */
	{
		/* set terminate process flag */
		cb->dlc.term_kproc = TRUE;
		sdlmonitor(cb, "TERM", 0, 0, "LAST", 4);

		/* Note - the wakeup will occur at the last breath of the kproc */
	}
	else /* there are still open channels on this "dead" port */
	{
		/* insure that dlcmpx is able to issue e_sleep
		   by forcing the dispatcher to cycle the ready
		   processes */
		do
		{
			delay(10);
		} while (cb->dlc.kcid->proc_id == EVENT_NULL);

		/* wake up the head code process */
		sdlmonitor(cb, "WAKE", cb->dlc.chan_count, 0, "MORE", 4);
		e_wakeup((int *)&cb->dlc.kcid->proc_id);

	}
/* end defect 141966 */
#endif
}



/************************************************************************/
/*                                                                      */
/* Name:	len_port_cb                                             */
/*                                                                      */
/* Function:	return size of port control block structure             */
/*                                                                      */
/* Notes:	used by the head code to determine how much space to	*/
/*		malloc for the sdlc port control block			*/
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	integer                                                 */
/*                                                                      */
/************************************************************************/

int	len_port_cb()

{
	return(sizeof(PORT_CB));
}



/************************************************************************/
/*                                                                      */
/* Name:	max_opens                                               */
/*                                                                      */
/* Function:	return maximum number of opens calls allowed            */
/*                                                                      */
/* Notes:	used by the head code to determine when an open call	*/
/*		should fail						*/
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	integer                                                 */
/*                                                                      */
/************************************************************************/

int	max_opens()

{
	return(MAX_SDLC_OPENS);
}



/************************************************************************/
/*                                                                      */
/* Name:	transmit_trace                                          */
/*                                                                      */
/* Function:	logs the system trace entry for a receive packet        */
/*                                                                      */
/* Notes:	                                                        */
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/
void	transmit_trace(cb)
PORT_CB	*cb;
{
	char	*trc_data;		/* pointer to data		*/
	int	trace_len;
	struct  mbuf *tbuf;
/* defect 127727 */
/* removed #define  DATA_ONLY 9 */
/* end defect 127727 */

	trace_len = cb->write.m->m_len;
        if /* short trace is specified and the data is longer than 80 */
        (((cb->active_ls->ll.sls.flags & DLC_TRCL) == 0) &&
					(trace_len > TRACE_SHORT_SIZE))
        /* set the trace length to 80 bytes */
        trace_len = TRACE_SHORT_SIZE;

	trc_data = MTOD(cb->write.m, char *);

	trcgenkt(cb->active_ls->ll.sls.trace_chan, HKWD_SYSX_DLC_XMIT,
		DLC_DL_SDLC, trace_len, trc_data);

	tbuf = cb->write.m->m_next;
        if /* a cluster is attached indicating more trace data */
	    (tbuf) {
	    /* convert address to head of cluster */
	    /* convert to address of data */
	    trc_data = MTOD(tbuf, char *);
	    trace_len = tbuf->m_len;
            if /* short trace is specified and the data is longer than 80 */
            (((cb->active_ls->ll.sls.flags & DLC_TRCL) == 0) &&
					(trace_len > TRACE_SHORT_SIZE))
                 /* set the trace length to 80 bytes */
                 trace_len = TRACE_SHORT_SIZE;
/* defect 127727 */
	    trcgenkt(cb->active_ls->ll.sls.trace_chan, HKWD_SYSX_DLC_XMIT,
		DLC_DL_DATA_ONLY, trace_len, trc_data);
/* end defect 127727 */
	 } /* endif - cluster attached */

}  
	


/************************************************************************/
/*                                                                      */
/* Name:	timer_trace                                             */
/*                                                                      */
/* Function:	logs the system trace entry for the specified timeout   */
/*                                                                      */
/* Notes:	                                                        */
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/
void	timer_trace(cb, timer_type)
PORT_CB	*cb;
int	timer_type;
{
	trcgenkt(cb->active_ls->ll.sls.trace_chan, HKWD_SYSX_DLC_TIMER,
		((DLC_DL_SDLC << 16) | timer_type), 0, timer_type);

}



/************************************************************************/
/*                                                                      */
/* Name:	session_trace                                           */
/*                                                                      */
/* Function:	trace an open or close link                             */
/*                                                                      */
/* Notes:	                                                        */
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/
void	session_trace(cb, trace_type)
PORT_CB	*cb;
{
	ulong	data_word;
	ulong	pl_type;
	ulong	session_type;

	/* find out what physical link is being used */
	if (cb->pl.mpqp.phys_link & PL_232D)
		pl_type = DLC_PL_RS232;
	else if (cb->pl.mpqp.phys_link & PL_422A)
		pl_type = DLC_PL_EIA422;
	else if (cb->pl.mpqp.phys_link & PL_V35)
		pl_type = DLC_PL_V35;
	else if (cb->pl.mpqp.phys_link & PL_X21)
		pl_type = DLC_PL_X21;
	else if (cb->pl.mpqp.phys_link & PL_SMART_MODEM)
		pl_type = DLC_PL_SMART;
	else if (cb->pl.mpqp.phys_link & PL_V25)
		pl_type = DLC_PL_V25BIS;

	/* is this an open or close */
	if (trace_type == TRACE_OPEN)
		session_type = HKWD_SYSX_DLC_START;
	else
		session_type = HKWD_SYSX_DLC_HALT;

	data_word = (DLC_DL_SDLC << 16) | pl_type;

	trcgenkt(cb->active_ls->ll.sls.trace_chan, session_type,
						data_word, 0, NULL);

}


/************************************************************************/
/*                                                                      */
/* Name:	receive_trace                                           */
/*                                                                      */
/* Function:	logs the system trace entry for a receive packet        */
/*                                                                      */
/* Notes:	                                                        */
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/
void	receive_trace(cb)
PORT_CB	*cb;
{
	char	*trc_data;		/* pointer to data		*/
	int	trace_len;

        trace_len = cb->m->m_len; 
        if /* short trace is specified and the data is longer than 80 */
        (((cb->active_ls->ll.sls.flags & DLC_TRCL) == 0) &&
                           (trace_len > TRACE_SHORT_SIZE))
        trace_len = TRACE_SHORT_SIZE;     /* set the trace length to 80 */
        
	trc_data = MTOD(cb->m, char *);

	trcgenkt(cb->active_ls->ll.sls.trace_chan, HKWD_SYSX_DLC_RECV,
		DLC_DL_SDLC, trace_len, trc_data);
}
	


/************************************************************************/
/*                                                                      */
/* Name:	performance_trace                                       */
/*                                                                      */
/* Function:	provide indication of system perfomance                 */
/*                                                                      */
/* Notes:	                                                        */
/*                                                                      */
/* Data									*/
/* Structures:	sdlc port control block                                 */
/*                                                                      */
/* Returns:	void                                                    */
/*                                                                      */
/************************************************************************/

void	performance_trace(cb, perf_type, len)

PORT_CB	*cb;
ulong	perf_type;
int	len;

{
	ulong	data_word;

	data_word = (HKWD_SYSX_DLC_PERF | perf_type | DLC_DL_SDLC);

	trchklt(data_word, len);

}
