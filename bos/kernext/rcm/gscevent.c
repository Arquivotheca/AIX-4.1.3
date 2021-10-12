static char sccsid[] = "@(#)53	1.14.1.6  src/bos/kernext/rcm/gscevent.c, rcm, bos411, 9437A411a 9/8/94 17:51:14";

/*------------
* COMPONENT_NAME: (rcm) Graphics System Call Event Service Routines
*
* FUNCTIONS: Event support routines for graphics devices
*
* ORIGINS: 27
*
*   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*   combined with the aggregated modules for this product)
*                    SOURCE MATERIALS
*
*   (C) COPYRIGHT International Business Machines Corp. 1989-1994
*   All Rights Reserved
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/

  /*-----------
  This main line file contains the graphics device driver interface
  routines:

    gsc_sync_event - Invoke an event syncronously
    gsc_async_event - Invoke an event asyncronously
    gsc_get_event - Return an event array
    gsc_event_buffer - Allocate a buffer to store event information
    gsc_enable_event - Enable an event on an adapter

  ------------*/

#include <lft.h>                    /* includes for all lft related data */
#include <sys/sysmacros.h>		/* Macros for MAJOR/MINOR */
#include <sys/adspace.h>
#include <sys/dma.h>			/* direct memory addressing */
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/user.h>			/* user structure */
#include <sys/syspest.h>
#include "rcm_mac.h"
#include "rcmras.h"			/* error defines */
#include "xmalloc_trace.h"
/* #include "event.h"  */

BUGVDEF(gsc_evt, 0);

static int  makinfo ();
static int  relinfo ();

/******************
These defines belong in aixgsc.h
 ******************/
/* We need the algorithm to get the current pProc stuct !!! **/

/*
 *  event_free - free up resources like memory allocation.  Deal with
 *		 any pending event activity (shutdown).
 */
int  event_free (pdev, pproc)
{
    /*
     *  To close down the event world we would normally make some
     *  kind of call to the DD level.  This call would cause the DD
     *  level to purge processing and stop pending interrupts which
     *  were particular to this rcm process structure.
     *
     *  HOWEVER, such a DD call is not architecturally supported.
     *
     *  Instead, we are relying on the DD call 'unmake_gp' to
     *  perform that processing.  This means that this function
     *  ('event_free') should only be called from 'rcm_unmake_gp',
     *  AFTER 'rcm_unmake_gp' has made the DD level 'unmake_gp' call.
     */

    /* Release allocated memory */
    relinfo (pproc);

    return  0;
}

/*-----------------
  callback function

  This routine takes care of logging the event into the async array or
  sync report field of EventInfo.

  ------------------*/

got_report(rcmp,event_report)
rcmProcPtr rcmp;
eventReport *event_report;
{
  int rc = 0;
  long next;

  /* Check to see if was an async or sync event and handle  */
  /* accordingly					    */

  /* Synchronous events take higher precedence so check for this */
  /* first							 */
  BUGLPR(gsc_evt, BUGNFO,
	 ("In callback have event \n"));

  if (rcmp->procHead.pEvents->masks.s_mask & event_report->event)
  {
    /* The event contains information which was requested in sync */
    /* mode so record info in structure and signal or wake process*/

  BUGLPR(gsc_evt, BUGNFO,
	 ("In callback have sync event \n"));
    rcmp->procHead.pEvents->lastEvent.event = event_report->event;
    rcmp->procHead.pEvents->lastEvent.time  = event_report->time;
    rcmp->procHead.pEvents->lastEvent.rcx   = (RCX_Handle) rcmp->pDomainCur[0];
    rcmp->procHead.pEvents->lastEvent.wa    = (WA_Handle) rcmp->procHead.pWA;
    rcmp->procHead.pEvents->lastEvent.data[0]  = event_report->data[0];
    rcmp->procHead.pEvents->lastEvent.data[1]  = event_report->data[1];
    rcmp->procHead.pEvents->lastEvent.data[2]  = event_report->data[2];
    rcmp->procHead.pEvents->lastEvent.data[3]  = event_report->data[3];
    if ( rcmp->procHead.pEvents->wait )
    {
      /* If we are here then the event has occurred and we need to */
      /* return event and buffer to app  this will be done in	   */
      /* sync event so all we need to do is return		   */
      return(0);
    }
    else
    {
      /* No wait was specified so signal process that event is in */
       pidsig(rcmp->procHead.pEvents->gPid, SIGMSG);
       BUGLPR(gsc_evt, BUGNFO,
	    ("In callback with sync no wait signalling process"));
    }

    rc = 0;
  }
  else
  {

    if (rcmp->procHead.pEvents->masks.a_mask & event_report->event)
    {  /* Event has async info for us */

      if ( rcmp->procHead.pEvents->pArray->number_req ==
	   rcmp->procHead.pEvents->pArray->number_used )

      {  /* event array is full. set number_used to negative and return ? */

	BUGLPR(gsc_evt, BUGNFO,
	      ("event array is full \n"));
	rcmp->procHead.pEvents->pArray->number_used = 0 -
		      rcmp->procHead.pEvents->pArray->number_used;

	return(-1);
      }

      /* There is room left in the array store the report into it */
      next = rcmp->procHead.pEvents->pArray->number_used;
      rcmp->procHead.pEvents->pArray->event[next].event = event_report->event;
      rcmp->procHead.pEvents->pArray->event[next].time	= event_report->time;
      rcmp->procHead.pEvents->pArray->event[next].rcx	= event_report->rcx;
      rcmp->procHead.pEvents->pArray->event[next].wa	= event_report->wa;
      rcmp->procHead.pEvents->pArray->event[next].data[0]  = event_report->data[0];
      rcmp->procHead.pEvents->pArray->event[next].data[1]  = event_report->data[1];
      rcmp->procHead.pEvents->pArray->event[next].data[2]  = event_report->data[2];
      rcmp->procHead.pEvents->pArray->event[next].data[3]  = event_report->data[3];
      rcmp->procHead.pEvents->pArray->number_used++;
	BUGLPR(gsc_evt, BUGNFO,
	     ("number used is %d\n",rcmp->procHead.pEvents->pArray->number_used));
      if (rcmp->procHead.pEvents->pArray->number_used == 1)
      {
	 /* Signal the process to indicate the first event has taken place */
	pidsig(rcmp->procHead.pEvents->gPid, SIGMSG);
	BUGLPR(gsc_evt, BUGNFO,
	     ("Signalling process \n"));
      }
      rc = 0;
    }
    else
     /* event was not requested to be recorded should not have happened?*/
       BUGLPR(gsc_evt, BUGNFO,
	 ("event not requested!!!! \n"));
     rc = 0;
  }

  return(rc);

}



/*-----------------
 Async event handler service

 This routine copies in the event mask into kernel memory and passes
 the information into the device driver function which will handle
 the actual processing.

  ------------------*/

gsc_async_event(pd,arg)
struct phys_displays *pd;
struct async_event *arg;
{
  int rc;
  long msize;
  struct async_event a_event;
  rcmProcPtr ptProc;

  BUGLPR(gsc_evt, BUGNFO,
	 ("in async event.\n"));
  gsctrace (ASYNC_EVENT, PTID_ENTRY);

  /* calculate procptr */
  FIND_GP(pd->pGSC,ptProc);
  if (ptProc == NULL)
    return  EINVAL;

  /*  Make sure pEvent structure exists */
  if (makinfo (ptProc))
    return  ENOMEM;

  /* copy arguments into local variable for use */
    rc = copyin(arg, &(a_event), sizeof(a_event));
    if (rc != 0) {
	rcmerr("GSC","aixgsc","copyin",rc,0,UNIQUE_2);
	return(EFAULT);
    }

  BUGLPR(gsc_evt, BUGNFO,
	 ("copyin success async event.\n"));

  /* check for reset of buffer */
  if ( a_event.num_events == 0 )
  {
    BUGLPR(gsc_evt, BUGNFO,
	 ("Request to clear array out.\n"));

    /* Call Device driver to turn interrupts off */
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    rc = (*pd->async_mask)(pd->visible_vt, &(a_event), got_report);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);

    /* reset requested - free old array */
    if (ptProc->procHead.pEvents->pArray != NULL)
    {
        xmfree((caddr_t) ptProc->procHead.pEvents->pArray, pinned_heap);
        ptProc->procHead.pEvents->pArray = NULL;
    }

    return(0);
  }

  /* Requesting an event buffer with non-zero size */

  if (ptProc->procHead.pEvents->pArray != NULL)
  {
        xmfree((caddr_t) ptProc->procHead.pEvents->pArray, pinned_heap);
        ptProc->procHead.pEvents->pArray = NULL;
  }

  /* allocate space for event array */
  msize = ((a_event.num_events - 1) * sizeof(eventReport)) +
	 sizeof(eventArray); /* eventArray includes 1 eventReport itself */

  BUGLPR(gsc_evt, BUGNFO,
	 ("Size- async array %x for %d events\n",msize,a_event.num_events));

  ptProc->procHead.pEvents->pArray = (eventArrayPtr) xmalloc (msize,2,pinned_heap);

  if (ptProc->procHead.pEvents->pArray == NULL )
  {
     /* log error */
     BUGLPR(gsc_evt, BUGNFO,
	 ("Malloc failed of pArray \n"));
     return (ENOMEM);
  }

  bzero(ptProc->procHead.pEvents->pArray, msize);

  /* Initialize data fields */

  ptProc->procHead.pEvents->masks.a_mask = a_event.mask;
  BUGLPR(gsc_evt, BUGNFO,
	 ("event mask is %x %x\n", a_event.mask,
  ptProc->procHead.pEvents->masks.a_mask ));
  ptProc->procHead.pEvents->pArray->number_req = a_event.num_events;
  ptProc->procHead.pEvents->pArray->number_used = 0;
  ptProc->procHead.pEvents->pArray->a_size = msize;
  ptProc->procHead.pEvents->gPid = getpid();

  /* Call vdd to initialize device specific data and to enable interrupts */

  BUGLPR(gsc_evt, BUGNFO,
	 ("calling vdd %x \n", &a_event));

  RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
  rc = (*pd->async_mask)(pd->visible_vt, &(a_event), got_report);
  RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);

  /* return */
  gsctrace (ASYNC_EVENT, PTID_EXIT);
  return(rc);
}



/*
 *  gsc_event_buffer - Release old Data event buffer, if it exists,
 *		       and allocate a new one.
 */
gsc_event_buffer(pd,arg)
struct phys_displays *pd;
struct event_buffer *arg;
{
  int rc = 0;
  struct event_buffer event_b;
  rcmProcPtr ptProc;

  BUGLPR(gsc_evt, BUGNFO,
	 ("In event buffer \n"));
  gsctrace (EVENT_BUFFER, PTID_ENTRY);

  /* copy arguments into local variable for use */
  rc = copyin(arg, &(event_b), sizeof(event_b));
  if (rc != 0) {
      rcmerr("GSC","aixgsc","copyin",rc,0,UNIQUE_2);
      return(EFAULT);
  }

  BUGLPR(gsc_evt, BUGNFO,
	 ("In event buffer about to malloc\n"));

  /* calculate procptr */
  FIND_GP(pd->pGSC,ptProc);
  if (ptProc == NULL)
    return  EINVAL;

  /*  Make sure pEvent structure exists */
  if (makinfo (ptProc))
    return  ENOMEM;

  /* Check to see if old buffer is around */

  if (ptProc->procHead.pEvents->pData != NULL)
  {
    BUGLPR(gsc_evt, BUGNFO,
	  ("Freeing Old Buffer	%x %x\n",
	    ptProc->procHead.pEvents->pData,
	    ptProc->procHead.pEvents->length));

    xmfree(ptProc->procHead.pEvents->pData,pinned_heap);
    ptProc->procHead.pEvents->pData = NULL;
  }

  /* Malloc event buffer the size the user has asked for */
  ptProc->procHead.pEvents->pData =
	   xmalloc(event_b.length, 3, pinned_heap);

  if (ptProc->procHead.pEvents->pData == NULL )
  {
     /* log error */
     BUGLPR(gsc_evt, BUGNFO,
	 ("Malloc failed of pBuffer \n"));
     return(ENOMEM);
  }

  ptProc->procHead.pEvents->length = event_b.length;
  bzero(ptProc->procHead.pEvents->pData, event_b.length);

  gsctrace (EVENT_BUFFER, PTID_EXIT);
  return(0);
}

/*-----------------
  Get events routine - This routine returns to the graphics process the
  event array it requested.
  -----------------*/

gsc_get_events(pd,arg)
struct phys_displays *pd;
struct get_events *arg;
{
  int rc = 0;
  struct get_events g_events;
  long msize;
  rcmProcPtr ptProc;

  BUGLPR(gsc_evt, BUGNFO,
	 ("In get event \n"));
  gsctrace (GET_EVENTS, PTID_ENTRY);

  /* copy arguments into local variable for use */
  rc = copyin(arg, &(g_events), sizeof(g_events));
  if (rc != 0) {
      rcmerr("GSC","aixgsc","copyin",rc,0,UNIQUE_2);
      return(EFAULT);
  }

  /* calculate procptr */
  FIND_GP(pd->pGSC,ptProc);
  if (ptProc == NULL)
    return  EINVAL;

  RCM_ASSERT (ptProc->procHead.pEvents, 0, 0, 0, 0, 0);
  RCM_ASSERT (ptProc->procHead.pEvents->pArray, 0, 0, 0, 0, 0);

  /*  Make sure pEvent structure exists and has data */
  if ( (ptProc->procHead.pEvents == NULL) || 
       (ptProc->procHead.pEvents->pArray == NULL) )
    return  EINVAL;

  /* Compare the number of array entries requested to be read to the */
  /* number requested to be tracked. If they aren't reading the same */
  /* number then return an error in the arg structure		     */

  if (g_events.num_events < ptProc->procHead.pEvents->pArray->number_req)
  {
    g_events.error = BAD_ARRAY_SIZE;
    copyout(&(g_events),arg,sizeof(g_events));
    return(EINVAL);
  }

  /* Number of events requested is ok calculate size of array to move */
  /* and copy it out to user memory.				      */

  msize = ((g_events.num_events - 1) * sizeof(eventReport)) +
	  sizeof(eventArray); /* eventArray includes 1 eventReport itself */

  copyout(ptProc->procHead.pEvents->pArray, g_events.array, msize);

  /* Now copy out get event structure indicating no errors */

  g_events.error = 0;
  g_events.num_events = ptProc->procHead.pEvents->pArray->number_used;
  copyout(&(g_events), arg, sizeof(g_events));
  BUGLPR(gsc_evt, BUGNFO,
	 ("In event buffer copying out %d events \n",g_events.num_events));


  /* Reset counter for how many events have happened */
  ptProc->procHead.pEvents->pArray->number_used = 0;

  gsctrace (GET_EVENTS, PTID_EXIT);
  return(0);


}



/*-----------------
 Sync event handler service

 This routine copies in the event mask into kernel memory and passes
 the information into the device driver function which will handle
 the actual processing.

  ------------------*/

gsc_wait_event(pd,arg)
struct phys_displays *pd;
struct wait_event *arg;
{

  int rc;
  struct wait_event s_event;
  char * user_buf;
  rcmProcPtr ptProc;

  /* check to see if eventinfo is there yet if not allocate it. */
  BUGLPR(gsc_evt, BUGNFO,
	 ("In sync event \n"));
  gsctrace (WAIT_EVENT, PTID_ENTRY);

  /* calculate procptr */
  FIND_GP(pd->pGSC,ptProc);
  if (ptProc == NULL)
    return  EINVAL;

  /*  Make sure pEvent structure exists */
  if (makinfo (ptProc))
    return  ENOMEM;

  BUGLPR(gsc_evt, BUGNFO,
	 ("In sync event about to copyin in args\n"));

  /* copy arguments into local variable for use */
  rc = copyin(arg, &s_event, sizeof(s_event));
  if (rc != 0) {
      rcmerr("GSC","gsc_wait_event","copyin",rc,0,UNIQUE_1);
      return(EFAULT);
  }

  /* Processing is different for wait vs nowait */
  if (s_event.wait)
  {
    /* wait has been requested so set all necessary fields and */
    /* call the device driver. When we return we have been     */
    /* woken up and the event has taken place so set error=0   */
    /* and return buffer etc.				       */

    BUGLPR(gsc_evt, BUGNFO,
	  ("In sync event with wait \n"));
    ptProc->procHead.pEvents->wait = 1;
    ptProc->procHead.pEvents->masks.s_mask = s_event.mask;

    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    rc = (*pd->sync_mask)(pd->visible_vt, &(s_event), got_report);
    RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);

    /* When we reach here the event has taken place */

    /* Check for info in the buffer */
    if (ptProc->procHead.pEvents->pData != NULL)
    {
       /* There has been a buffer requested copy contents into user space */
       BUGLPR(gsc_evt, BUGNFO,
	     ("copying out buffer \n"));

       copyout(ptProc->procHead.pEvents->pData, s_event.pData,
	       ptProc->procHead.pEvents->length);

       s_event.length = ptProc->procHead.pEvents->length;

       /* clear out buffer */

       xmfree(ptProc->procHead.pEvents->pData,
	      pinned_heap );
       ptProc->procHead.pEvents->pData = NULL;
    }


    /* Copy the event report into the user's structure */
    /* set error code to 0 and copyout s_event structure */
    s_event.error = 0;
    BUGLPR(gsc_evt, BUGNFO,
	  ("copying out buffer time of event is %x\n",s_event.report.time));
    copyout(&(s_event), arg, sizeof(s_event));

    /* Zero out report structure */
    ptProc->procHead.pEvents->lastEvent.event = 0;
    ptProc->procHead.pEvents->lastEvent.time  = 0;
    ptProc->procHead.pEvents->lastEvent.rcx   = 0;
    ptProc->procHead.pEvents->lastEvent.wa    = 0;
    ptProc->procHead.pEvents->lastEvent.data[0]  = 0;
    ptProc->procHead.pEvents->lastEvent.data[1]  = 0;
    ptProc->procHead.pEvents->lastEvent.data[2]  = 0;
    ptProc->procHead.pEvents->lastEvent.data[3]  = 0;
    ptProc->procHead.pEvents->masks.s_mask = 0;


    rc = 0;
  }
  else	 /* No wait specified */
  {
    BUGLPR(gsc_evt, BUGNFO,
	 ("In sync event with no wait \n"));

    if (s_event.mask == 0)  /* If wait and mask = 0 then request */
    {			    /* is to read the nowait sync event  */

       s_event.mask   = ptProc->procHead.pEvents->lastEvent.event;
       s_event.report = ptProc->procHead.pEvents->lastEvent;

       BUGLPR(gsc_evt, BUGNFO,
	 ("no wait requesting event %x\n",s_event.mask));

       BUGLPR(gsc_evt, BUGNFO,
	 ("no wait requesting buffer ptr is %x\n",
	  ptProc->procHead.pEvents->pData));

       /* Check for info in the buffer */
       if (ptProc->procHead.pEvents->pData != NULL)
       {

	  /* There has been a buffer requested copy contents into user space */
	  BUGLPR(gsc_evt, BUGNFO,
		("copying out buffer \n"));
	  copyout(ptProc->procHead.pEvents->pData, s_event.pData,
		  ptProc->procHead.pEvents->length);

	  s_event.length = ptProc->procHead.pEvents->length;
	  /* clear out buffer */
	  BUGLPR(gsc_evt, BUGNFO,
		("freeing buffer \n"));
	  bzero(ptProc->procHead.pEvents->pData,
		ptProc->procHead.pEvents->length);
	  BUGLPR(gsc_evt, BUGNFO,
		("freeing successful \n"));
       }

	s_event.report = ptProc->procHead.pEvents->lastEvent;

	/* set error code to 0 and copyout s_event structure */
	s_event.error = 0;
	copyout(&(s_event), arg, sizeof(s_event));

	/* Zero out report structure */
	ptProc->procHead.pEvents->lastEvent.event = 0;
	ptProc->procHead.pEvents->lastEvent.time  = 0;
	ptProc->procHead.pEvents->lastEvent.rcx   = 0;
	ptProc->procHead.pEvents->lastEvent.wa	  = 0;
	ptProc->procHead.pEvents->lastEvent.data[0]  = 0;
	ptProc->procHead.pEvents->lastEvent.data[1]  = 0;
	ptProc->procHead.pEvents->lastEvent.data[2]  = 0;
	ptProc->procHead.pEvents->lastEvent.data[3]  = 0;
	ptProc->procHead.pEvents->masks.s_mask = 0;
    }
    else
    {
      /* Since we aren't waiting for the event to happen, set flags */
      /* call the device driver and return			    */

      ptProc->procHead.pEvents->wait = 0;
      ptProc->procHead.pEvents->masks.s_mask = s_event.mask;
      ptProc->procHead.pEvents->gPid = getpid();
      BUGLPR(gsc_evt, BUGNFO,
	 ("In sync event with no wait calling vdd\n"));

      RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
      rc = (*pd->sync_mask)(pd->visible_vt, &(s_event), got_report);
      RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
    }
  }
BUGLPR(gsc_evt, BUGNFO,
     ("Leaving sync event rc = %x \n",rc));

gsctrace (WAIT_EVENT, PTID_EXIT);
return(rc);

}

/*-----------------
 Enable event service

 This routine copies in the event mask into kernel memory and passes
 the information into the device driver function which will handle
 the actual processing. This function allows continuous use of an
 event on the adapter without reports going to application.

  ------------------*/

gsc_enable_event(pd,arg)
struct phys_displays *pd;
struct enable_event *arg;
{
  int rc = 0;
  struct enable_event enable_buf;
  rcmProcPtr ptProc;

  BUGLPR(gsc_evt, BUGNFO,
	 ("In enable event \n"));
  gsctrace (ENABLE_EVENT, PTID_ENTRY);

  /* calculate procptr */
  FIND_GP(pd->pGSC,ptProc);
  if (ptProc == NULL)
    return  EINVAL;

  /*  Make sure pEvent structure exists */
  if (makinfo (ptProc))
    return  ENOMEM;

  /* copy arguments into local variable for use */
  rc = copyin(arg, &(enable_buf), sizeof(enable_buf));
  if (rc != 0) {
      rcmerr("GSC","aixgsc","copyin",rc,0,UNIQUE_2);
      return(EFAULT);
  }

  /* Set enable event mask into event structure */
  ptProc->procHead.pEvents->masks.e_mask = enable_buf.e_event;

  /* call vdd enable event entry point to get event going */
  BUGLPR(gsc_evt, BUGNFO, ("In enable event - calling vdd\n"));

  RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);
  rc = (*pd->enable_event)(pd->visible_vt, &(enable_buf));
  RCM_ASSERT (i_disable (INTBASE) == INTBASE, 0, 0, 0, 0, 0);

  BUGLPR(gsc_evt, BUGNFO, ("Leaving enable event rc = %x\n",rc));

  gsctrace (ENABLE_EVENT, PTID_EXIT);
  return(rc);
}


/*
 *  makinfo - Make sure that the Event structure exists.
 */
static int  makinfo (ptProc)
rcmProcPtr ptProc;
{
    if (ptProc->procHead.pEvents == NULL)
    {
        ptProc->procHead.pEvents =
		(eventInfo *) xmalloc (sizeof (eventInfo), 3, pinned_heap);

        if (ptProc->procHead.pEvents == NULL)
	    return  ENOMEM;

        bzero (ptProc->procHead.pEvents, sizeof (eventInfo));
    }

    return  0;
}


/*
 *  relinfo - Release the Event structure.  You have to release the
 *	      substructures (if any) first.  (A deep-free!)
 */
static int  relinfo (ptProc)
rcmProcPtr ptProc;
{
    if (ptProc->procHead.pEvents != NULL)
    {
	if (ptProc->procHead.pEvents->pArray != NULL)
	{
	    xmfree ((caddr_t) ptProc->procHead.pEvents->pArray, pinned_heap);
            ptProc->procHead.pEvents->pArray = NULL;
	}

	if (ptProc->procHead.pEvents->pData != NULL)
	{
	    xmfree ((caddr_t) ptProc->procHead.pEvents->pData, pinned_heap);
            ptProc->procHead.pEvents->pData = NULL;
	}

        xmfree ((caddr_t) ptProc->procHead.pEvents, pinned_heap);
        ptProc->procHead.pEvents = NULL;
    }

    return  0;
}
