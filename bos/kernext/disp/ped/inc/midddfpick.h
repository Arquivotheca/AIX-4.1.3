/* static char sccsid[] = "@(#)62	1.3  src/bos/kernext/disp/ped/inc/midddfpick.h, peddd, bos411, 9428A410j 3/19/93 18:54:16"; */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_MIDDDFPICK.H
#define _H_MIDDDFPICK.H


/*--------------------------------------------------------------------------

			Pick event buffer structure

---------------------------------------------------------------------------*/

typedef struct pick_event {
	ushort                  correlator;
	pid_t                   pid;
	long                    eventcmd;
	long                    event_mask;
	eventReport             report;
	struct xmem             dp;
	struct wait_event       *waitEvent;
	caddr_t                 sleep_addr;

        /* this flag is used to protect ourself from being waken up 
           accidentally 
        */
	int 			sleep_flag;  

	int                     (*callback)();
	int                     (*dcallback)(); /* diagnostic callback  */
	caddr_t                 callbackarg;
	ulong                   bus_addr;
} pick_event_t;

typedef struct pick_pending {

	/* 
	   This event word is used to serialize the pick event. 
	   Since the ucode can process only one pick event (Begin 
           pick request, End Pick request, pick complete interrupt)
           at any point in time, and more than one application can
           issue pick events,  we have to single thread the pick
           events from the host. 
 
           To serialize pick event, we need to keep track of
           processes waiting to do pick event with a linked list.
            
	   This is how it works: when the Ped. driver is processing
           a pick event, any process issues pick event will be
           put to sleep.  The linked list will be created by the 
           driver to keep track of these processes.  When the driver
           receives the pick complete interrupt from ucode, it
           will wake up the oldest process waiting to proceed with
           the Begin Pick event.  The process of blocking and
           unblocking will be repeated until we exhaust the list 

           Note: the very first pick event will not require any
                 serialization, so we need a flag, BeginPickInProgess
                 in ddf to handle the initial condition
	*/

	int        	        beginPickSleepEventWord;
	int        	        sleep_flag;

	/* pointer to next node on list */
	struct pick_pending     * next_pending_BeginPickPtr;

} pick_pending_t;


#endif /* _H_MIDDDFPICK */
