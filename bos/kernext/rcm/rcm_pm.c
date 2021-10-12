static char sccsid[] = "@(#)65  1.3  src/bos/kernext/rcm/rcm_pm.c, rcm, bos41J, 9517B_all 4/26/95 12:37:32";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager - PM 
 *
 * FUNCTIONS:  quiesce_graphics_IO, unblock_graphics_IO
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <rcmdds.h>                     /* RCM dds structure   */
#include <sys/rcm_win.h>  
#include <sys/rcm.h>                    /* #define RCM_MAX_DEV */
#include "rcm_pm.h"
#include "rcm_mac.h"                    /* RCM_TRACE  macro    */
#include <sys/sleep.h> 
#include <sys/intr.h> 
#include <signal.h> 
#include <sys/pm.h> 

/*---------------------------------------------------------------------------
 |                                                                          |
 | global variable -- set by rcm_pm_handler function (registered with PM    |
 |                    core).  This occurs when the system has to enter      |
 |                    suspend or hibernation state.                         |
 |                                                                          |
 |                 -- clear by rcm_pm_handler function (registered with PM  |
 |                    core.  This occurs when the system resumes from       |
 |                    suspend or hibernation state.                         |
 |                                                                          |
 |                 -- ioctl(GSC_GRAHPICS_INIT_STARTS) uses this flag to     |
 |                    determine if the system is trying to enter suspend/   |
 |                    hibernation state so it the ioctl will fail.  The X   | 
 |                    server is required to check for errno and return code |
 |                    from the system call when it issues this ioctl        |
 |                                                                          |
 ---------------------------------------------------------------------------*/

uchar pm_event = 0;

/* -------------------------------------------------------------------------|
 | global data    -- modified by ioctls GSC_HANDLE, GSC_GRAPHICS_INIT_STARTS|
 |                   GSC_HANDLE_RELEASE, GSC_GRAPHICS_INIT_ENDS,            |
 |                   GSC_GRAPHICS_DATA_SAVED, GSC_GRAPHICS_SAVE_FAILED      |
 |                                                                          |
 |                --  modified by save_graphics_displays_data and           | 
 |                    restore_graphics_displays_data                        |
 |                                                                          |
 | The array contains information on each server that needs to save data    |
 | when graphic displays devices have to enter suspend or hibernation mode  |
 |                                                                          |
 ---------------------------------------------------------------------------*/

struct rcm_pm_data current_pm_status[RCM_MAX_DEV];

/*---------------------------------------------------------------------------
 |                                                                          |
 | global variable -- used by save_graphic_data function.  This occurs when |
 |                    a graphics device PM handle is invoked by PM core to  |
 |                    put the device in suspend/hibernation mode.  However  |
 |                    untill all server(s), if there is any, complete its   |
 |                    initialization, the callback can't determine the mode,|
 |                    KSR or GRAPHICS, of each display.  Therefore it is    |
 |                    necessary to suspend the execution of the device PM   |
 |                    thread temporarily until all servers issue the ioctl  |
 |                    GSC_GRAPHICS_INIT_ENDS before the save_graphics_data  |
 |                    can proceed to determine what mode, KSR or GRAPHICS,  |
 |                    a display is in                                       |
 |                                                                          |
 ---------------------------------------------------------------------------*/

int X_init_sleep_word = EVENT_NULL;


/*---------------------------------------------------------------------------
 |                                                                          |
 | global data --  use to register RCM as PM awared device.  This structure |
 |                 is mainly used by PM core only                           |
 |                                                                          |
 ---------------------------------------------------------------------------*/

struct pm_handle pmdata;


/*---------------------------------------------------------------------------
 |                                                                          |
 | Declarations of functions and external variables                         |
 |                                                                          |
 ---------------------------------------------------------------------------*/

void  X_pm_timed_out();
int  rcm_pm_handler();

extern rcm_dds *rcmdds;

/* --------------------------------------------------------------------------
 |                                                                          |
 |    initialized RCM's PM data structures -- called by rcminit once        |
 |                                                                          |
 ---------------------------------------------------------------------------*/

void rcm_pm_data_init(devno)
dev_t devno;
{
   int i;

   /* printf("current_pm data =%x, trace_all =%x\n",current_pm_status,&trace_all); */

   for (i = 0 ; i < RCM_MAX_DEV ; i ++)
   {
       bzero(& current_pm_status[i], sizeof(struct rcm_pm_data));

      /* initialize each watchdog structure */

      current_pm_status[i].wd.count = 0; 
      current_pm_status[i].wd.next = NULL;
      current_pm_status[i].wd.prev = NULL; 
      current_pm_status[i].wd.func = X_pm_timed_out; 
      current_pm_status[i].wd.restart = MAX_2D_DATA_SAVING_TIME;
     
      /* initialize all sleep words */

      current_pm_status[i].X_pm_sleep_word        = EVENT_NULL;
      current_pm_status[i].X_pm_done_sleep_word   = EVENT_NULL;
      current_pm_status[i].terminate_X_sleep_word = EVENT_NULL;

   }

   /* initialize the handle in order to register our handler with PM core */
   pmdata.handler              = rcm_pm_handler; 

   pmdata.devno                = devno;               /* when devno is set to 0, pmctrl does not */ 
                                                      /* list the device.  Do I want to do this  */
                                                      /* for RCM ??                              */

   pmdata.mode                 = PM_DEVICE_FULL_ON;
   pmdata.private              = NULL; 
   pmdata.activity             = -1;
   pmdata.attribute            = 0;                   /* since it does not have any time-outs    */ 
   pmdata.idle_counter         = 0; 
   pmdata.device_idle_time     = 0; 
   pmdata.device_standby_time  = 0; 
   pmdata.device_idle_time1    = 0; 
   pmdata.device_idle_time2    = 0; 
   pmdata.device_logical_name  = "rcm0";               /* there is only one instance of RCM! */ 
   pmdata.pm_version           = 0x100;                /* code indicating dd supports PM2    */ 
   
}

/* --------------------------------------------------------------------------
 |                                                                          |
 |    update an entry in current_pm_status array.  It is called by:         | 
 |    - ioctl GSC_HANDLE to increment the count of displays a particular    |
 |    server owns                                                           |
 |    - ioctl GSC_HANDLE_RELEASE to decrement the count of displays a       |
 |    server owns.  Note  GSC_HANDLE_RELEASE is not used by the server      |
 |    but to match the logics in GSC_HANDLE                                 |
 |    - rcmclose() when each display is released                            |
 |    - rcm state change handler when it release a display                  |
 |                                                                          |
 ---------------------------------------------------------------------------*/

int update_pm_data(pid,cmd)
int pid;
int cmd;
{
	int j;

        for (j = 0 ; j < RCM_MAX_DEV ; j ++ )
        {
        	if (current_pm_status[j].pid == pid) break;
        }

        /* ioctl GSC_GRAPHICS_INIT_STARTS was not issued by this server */
        if (j >= RCM_MAX_DEV)   
	{
              return (0);
	}

	switch (cmd)
	{
	   case ADD_DSP:

		/* this server has been granted persmission so 
           	   add 1 to the count of how many displays it owns 
        	*/ 
		current_pm_status[j].num_of_dsp_owned += 1;

	   break;

           case REMOVE_DSP:

                /* this server has been granted persmission so substract
                   1 from the count of how many this displays it owns
                */

 		if (current_pm_status[j].num_of_dsp_owned > 0)
		{
 		   current_pm_status[j].num_of_dsp_owned -= 1;
		}

           break;

	   default:
		;
	}

	return (0);
}


/* --------------------------------------------------------------------------
 |                                                                          |
 | called by rcmclose() to reset an entry in the current_pm_status array    |
 |                                                                          |
 ---------------------------------------------------------------------------*/

int cleanup_pm_data(pid)
ulong pid;
{
      int j, flags, old_lvl;

      for (j = 0 ; j < RCM_MAX_DEV; j ++ )
      {
          if (current_pm_status[j].pid == pid) break;
      }

      if (j >= RCM_MAX_DEV)
      {
            return (0);
      }

      /* this process terminates so reset PM data structure  */ 

      if (current_pm_status[j].num_of_dsp_owned == 0)
      {
         old_lvl = i_disable(INTMAX);   

         /* check various bit fields to know what to do */
         flags = current_pm_status[j].flags;

	 /* did we start a watchog timer waiting for this server 
            to issue ioctl GSC_GRAPHICS_DATA_SAVED or 
            GSC_GRAPHICS_SAVE_FAILED but it didn't.  Things to do:
            1.  stop and remove timer if it is still active 
            2.  need to wake up the device PM thread suspended inside
                the callback save_graphics_dsp_data
         */ 
	 if (flags &  X_PM_WD_ON)     
	 {
	    w_stop( & current_pm_status[j].wd);     
	    w_clear( & current_pm_status[j].wd);

	    e_wakeup(& current_pm_status[j].X_pm_sleep_word);
         }

         /* did we suspend device PM thread waiting for some server 
            to issue ioctl GSC_GRAPHICS_INIT_ENDS
          */ 
         if (X_init_sleep_word != EVENT_NULL) 
	 {
	    e_wakeup(& X_init_sleep_word);
         }

	 /* did we suspend device PM thread before sending SIGKILL 
            to this server because it failed to save data within
            a reasonable amount of time allowed. 
         */ 
	 if (flags &  KILL_X_SERVER)
         {
            e_wakeup(& current_pm_status[j].terminate_X_sleep_word);
         }

         /* Now reset all variables -- */

          current_pm_status[j].flags = 0;   

          current_pm_status[j].X_pm_sleep_word        = EVENT_NULL;
          current_pm_status[j].X_pm_done_sleep_word   = EVENT_NULL;
          current_pm_status[j].terminate_X_sleep_word = EVENT_NULL;

	  current_pm_status[j].pid   = 0;

          i_enable(old_lvl);
      }
}


/*---------------------------------------------------------------------------
 | Watchdog timer used to protect PM thread from waiting forever on some    |
 | server trying to save graphics data                                      |
 ---------------------------------------------------------------------------*/

void X_pm_timed_out(w)
struct watchdog * w;
{
   struct rcm_pm_data * p_pm_data;
   p_pm_data = (struct rcm_pm_data *) w;

   p_pm_data->flags &= (~X_PM_WD_ON);
   p_pm_data->flags |= X_PM_TIMED_OUT;

   /* sever took too long to save its data in a reasonable amount
      of time so PM thread/drier can't wait any longer
   */
   e_wakeup( &(p_pm_data->X_pm_sleep_word) );
}


/*---------------------------------------------------------------------------
 |                                                                          |
 |                                                                          |
 |  This comment blocks is for quiesce_graphics_IO and unblock_graphics_IO  |
 |                             --------------------    -------------------  |
 |                                                                          |
 |                                                                          |
 |  When the system enters supend or hibernation state, PM core calls each  |
 |  driver to shut down its device.  To shut down the devices in the right  |
 |  sequence, PM core keeps track when each device registers its handle     |
 |  This information forms a road map or tree structure which PM core will  |
 |  use.  It informs devices at the bottom of the tree first and work       |
 |  upward.  Since the ordering must be kept, PM core can only send         |
 |  the shut down command to one device at any time and it must wait for    |
 |  this device to complete before proceeding to the next device in the tree|
 |  This by itself makes everyone's task is much easier.                    | 
 |                                                                          |
 |  It is a requirement for all drivers for physical devices on a system    | 
 |  which supports AIX Power Management to be PM aware.  The RCM is a       |
 |  a pseudo-device, however, it is also a PM aware.   This is designed     | 
 |  this way so that we (graphics subsystem) have a means to quiesce IO     |
 |  when some or all displays are in graphics mode (i.e., X server is       |
 |  running).                                                               | 
 |                                                                          |
 |  The original design calls for 2 new functions in the RCM which each     |
 |  which each graphics driver could use to accomplish this task.  However  |
 |  this idea makes the implementation complicated and error-prone.  The RCM|
 |  has difficulty knowing when to supend/wake up the server in case of     | 
 |  error occuring down below at the driver level which causes the supend or|
 |  hibernation attempt by PM core to be aborted.  The new approach is to   | 
 |  to make RCM a PM aware driver and its PM handles should be responsible  |
 |  for quiescing IO for displays in graphics mode.                         | 
 |                                                                          |
 |  The new approach is made possible based on the following facts:         |
 |                                                                          |
 |  1. the RCM device driver is configured after all graphics display device|
 |     drivers have be configured.                                          | 
 |                                                                          |
 |  2. Beacause PM core broadcasts suspend/hibernation commands in reveres  |  
 |     of configuration sequence (i.e., last device configured first), the  |
 |     RCM will be called before any graphics display device drivers.       | 
 |                                                                          | 
 |  3. Beacause PM core broadcast suspend/hibernation-resume commands in    |  
 |     the configuration sequence (i.e., first device configured first),    |
 |     RCM will be called after all graphics display device drivers have    | 
 |     resumed from suspend/hibernation mode.                               |
 |                                                                          |
 |  3. When the system attempts to enter suspend/hibernation state, PM core | 
 |     will broadcast PAGE_FREEZE_COMMAND to all PM aware drivers to inform |
 |     them to prepare for suspend/hibernation state.  If any device fails  |
 |     this command PM core will send PAGE_UNFREEZ_COMMAND to device drivers|
 |     which have successfully complete the PAGE_FREEZE_COMMAND.  This      | 
 |     inform these device driver to the attempt by the system has fail so  |
 |     these PM aware device driver must undo the PAGE_FREEZ_COMMAND.       | 
 |                                                                          |
 |     If all PM aware device drivers successfully complete the PAGE_FREE   |
 |     command, PM core will broadcast PM_DEVICE_SUSPEND or                 | 
 |     PM_DEVICE_HIBERNATION next to inform each driver one at a time to    | 
 |     save its data and shut down its device.                              |
 |                                                                          |
 |  4. When the system resumes from a successfully supend or hibernation    |
 |     operation, PM core broadcast PM_ENABLE/PM_IDLE command to all PM     |
 |     aware device drivers to instruct each driver to power on its device  |
 |     and restore its data.  Then PM core broadcasts the PAGE_UNFREEZE     |
 |     command again to all drivers for it to undo any PAGE_FREEZE          |
 |                                                                          |
 |  In genernal most driver will just ignore the PAGE_FREEZE command because|
 |  it does not need to do anything.  However for the RCM, it has to        |
 |  quiesce any IO in graphics mode when it receives the command.           |
 |  The reason is that when PM core broadcasts this command, all user       |
 |  proceeses are still running -- all user processes will be stopped when  | 
 |  PM core broadcasts the PM_DEVICE_SUSPEND or PM_DEVICE_HIBERNATION.      |
 |  The  X server is a user process so it will be stopped too!              |
 |                                                                          |
 |  To quiesce IO, the RCM needs to signal all servers to stop any new IO   |
 |  operations.  It does it by sending SIGUSER2 to each servers, one at a   |
 |  time and wait for its acknowlegement (via ioctl) before proceeding to   | 
 |  the next server.                                                        |
 |                                                                          |
 |  A server might own one or more displays.  However the RCM only sends    |
 |  one signal, SIGUSR2.  When a server receives the signal, it will save   |
 |  its data on all the displays it own on the next loop through its event  |
 |  handler code.  The RCM's PM handle meanwhile puts the threads from which|
 |  it is invokde (i.e., the PM thread) to sleep until this servers         |
 |  completes saving data.                                                  |
 |                                                                          |
 |  Each server indicates its has save its data successfully with the RCM   |  
 |  by issuing the ioctl(GRAPHICS_SAVED).  The ioctl puts the caller to     |
 |  sleep and wakes up the PM thread.  At this point execution returns to   |
 |  the RCM's PM handle.  This process is repeated until all servers are    | 
 |  signaled and put to sleep.  At this point, the RCM's PM handle returns  |
 |  to the PM thread (PM core) which proceeds to the next device.           |
 |                                                                          |
 |  If any device fails the PAGE_FREEZE command, the RCM will be informed   |
 |  with the PAGE_UNFREEZE command.  The RCM's PM handle will wake up all   |
 |  the servers it has put to sleep and then return to PM core.             | 
 |                                                                          |
 |  When all PM device drivers have successfully completed the PAGE_FREEZE  |
 |  command, PM core begins to broadcasts PM_DEVIC_SUSPEND or               |
 |  PM_DEVICE_HIBERNATION to inform each driver to save its data and turn   |
 |  off the device if needed.  The RCM ignores them -- it only concerns     |
 |  with PAGE_FREEZE and PAGE_UNFREEZE commands.  On the other hands,       | 
 |  graphics drivers will proceed to quiesce LFT node, saves its data if    |
 |  needed, and turn off its device for suspend mode.  The RCM has put      |
 |  all servers to sleep so graphics drivers should be safe to do its       |
 |  jobs.                                                                   | 
 |                                                                          |
 |  If for some reasons any device fails to suspend/hibernation, PM core    |
 |  will broadcast commands (PM_DEVICE_ENABLE/PM_DEVICE_IDLE) to all those  |
 |  have successfully completed the suspend/hibernation operation.          |
 |  Graphics device drivers will be infromed to resume first.  By the time  | 
 |  the RCM is informed to resume, all graphics devices have been powered   |
 |  on, initialied, and available.  The RCM is safe to wake up any servers  |
 |  that have been put to sleep.                                            | 
 |                                                                          |
 |  For a normal suspend/hibernation - resume sequence, the RCM will wake   |
 |  all server it put to sleep when it receives the PAGE_UNFREEZE command.  |
 |  At this point in time the RCM can be sure all graphics displays have    |
 |  been powered on, initialied, and available.                             |
 |                                                                          |
 |  When a server is wake up after it is put to sleep, it proceeds to       |
 |  restore its data and initialize the device for  the graphics mode.      | 
 |                                                                          |
 |  Also one time I thought about the scenario when the server died because |
 |  it was terminated by users or internal errors.  While the server was in |
 |  the process of terminating/cleaning up, the system had to enter suspend/|
 |  hibernation due to the battery running low.  The firt thought comes to  |
 |  mimd is that the RCM's PM handle has to put PM thread to sleep while    |
 |  this server is going down.  Untill this server terminates completely,   | 
 |  the RCM  can't quiesce IO in graphics mode.  After looking at the code  |
 |  and talking to Boyd, I'm ok.  In some cases,  this code might signal the| 
 |  X server which might or not might not be alive any more. If it does not |
 |  respond, it will be killed.  It is ok trying to kill a dead process!    | 
 |                                                                          |
 ---------------------------------------------------------------------------*/


long quiesce_graphics_IO() 
{
   int rc,i,done;
   ulong X_pid;
   int old_intr_lvl;


   RCM_TRACE(0xA00, getpid(),0,0);

   rc =	PM_SUCCESS;

   /*  
    * delay PM processing until all servers have finished their initializations
    */

   old_intr_lvl = i_disable(INTMAX);    /* serialize with ioctl */

   done = 0;

   while (! done )
   {
      for (i=0 ; i < RCM_MAX_DEV ; i ++)
      {
         /* Is any X server still initializing (i.e., not 
            yet issues the ioctl GSC_GRAPHICS_INIT_ENDS) 
          */

         if ( (current_pm_status[i].pid != 0) && 
              (!(current_pm_status[i].flags & GRAPHICS_INIT_COMPLETE))
            )
	 {
            break;
         }
      }

      /* either all servers finished initialization or there was no server */  

      if (i >= RCM_MAX_DEV)     
      {
         done = 1;
         RCM_TRACE(0xA01, getpid(),0,0);
      }
      else  
      {
         /* some server is stilling initializing so wait. We will be waken up 
            when X issues ioctl GSC_GRAPHICS_INIT_ENDS.

	    The ioctl GSC_GRAHICS_INIT_ENDS will check sleep word.  If it is 
	    not equal to EVENT_NULL, the ioctl wakes up the device PM thread 
         */

         /* need a timer here?  If the server hangs before issuing the ioctl
            GSC_GRAPHICS_INIT_ENDS, we hang here forever!
 
            Note  for whatever reason if this server dies before issuing the ioctl 
            GSC_GRPHICS_INIT_ENDS, the the rcmclose entry point will be invoked.  
            There in addition to release the gsc handle(s), we also perform any 
            clean up for the PM entry in the current_pm_status array and wake up 
            this device PM thread.

            Possible? Is There small window where the server dies and rcmclose 
            tries to wakup us up.  If this e_sleep (below) is executed after 
            that e_wakeup, we will sleep here forever!  It looks like we'll need 
            a watchdog timer after all or better logic?
         */

         RCM_TRACE(0xA02, getpid(),current_pm_status[i].pid ,0);
         if (e_sleep(& X_init_sleep_word, EVENT_SHORT) != EVENT_SUCC)
         {
            done = 1;
            rc = PM_ERROR;
         }
         RCM_TRACE(0xA03, getpid(), current_pm_status[i].pid,rc);
      }

   } /* while */


   /* At this point all the servers with permission to come up have finished 
      initialization.  The system is about to shut down, signal each server
      to stop IO.
    */
   if (rc == PM_SUCCESS)
   {

      for (i=0; i < RCM_MAX_DEV ; i ++)
      {
         if ( (X_pid = current_pm_status[i].pid) != 0)
         {
            rc = signal_X_to_quiesce_dsp_devices(X_pid,i);

	    if (rc != PM_SUCCESS)
	    {
   	       RCM_TRACE(0xA0A, getpid(), X_pid,0);
               break;
	    }
         }
      }
   }

   i_enable(old_intr_lvl);

   RCM_TRACE(0xA80, getpid(), rc,0);

   return (rc);
}

/* --------------------------------------------------------------------------
 |                                                                          |
 |  Inform X to quiesce IO and save its data                                |
 |                                                                          |
 ---------------------------------------------------------------------------*/

int signal_X_to_quiesce_dsp_devices(pid,i)

   ulong pid;
   int   i;

{
   int rc;

   rc = PM_SUCCESS;

   /* signal this server to prepare for devices being shutdown */

   RCM_TRACE(0xA04, getpid(), pid,0);

   pidsig(pid,SIGUSR2);

   current_pm_status[i].flags |= SIGUSR2_SENT;  

   /*
       Nothing else to do but wait for this server to save its data
       so the RCM has to block the PM thread/driver.

       need to start a watchod timer here, so the RCM does not 
       block the PM thread/driver forever. 
   */

   /* register each watchdog with kernel */
   w_init(& current_pm_status[i].wd);

   w_start( & current_pm_status[i].wd );

   current_pm_status[i].flags |= X_PM_WD_ON;  
      
   RCM_TRACE(0xA05, getpid(),pid,0);
   if (e_sleep(& current_pm_status[i].X_pm_sleep_word, EVENT_SHORT) != EVENT_SUCC)
   {
      rc = PM_ERROR;       /* big problem.  Can't tell what might happens next */ 
   }

   RCM_TRACE(0xA06, getpid(),pid,rc);

   /* waken up by expired timer? */ 
   if (current_pm_status[i].flags & X_PM_TIMED_OUT)  
   {

      /* 
          server failed to issue ioctl GSC_GRAPHICS_DATA_SAVED 
          in a reasonable amount of time.  It either hung or
          took too long to save its data.  Therefore we have
          no choice but to kill it.

	  Remove watchdog timer

          Sent the kill signal to this server

          The rcmclose() will check the "flags" and knows that it 
          has to wake up this PM thread.  Also, it needs to clean up 
          the current_pm_status entry.  All of this is done by calling
          cleanup_pm_data() which I'm thinking about move the call to
          the state change handler. 
      */

      w_clear( & current_pm_status[i].wd );  

      current_pm_status[i].flags |= KILL_X_SERVER;  

      RCM_TRACE(0xA07, getpid(),pid,0);
      pidsig(pid,SIGKILL);

      e_sleep( & current_pm_status[i].terminate_X_sleep_word, EVENT_SHORT); 

      RCM_TRACE(0xA08, getpid(),pid,0);
   }
   else 
   {
      /* 
         We are waken up by 3 possibilities:

         1. timer didn't expire -- server successffuly saved its data and issued 
            the ioctl GSC_GRAPHICS_DATA_SAVED in a reasonable amount of time. 

         1. timer didn't expire -- server unsuccessffuly saved its data and issued 
            the ioctl GSC_GRAPHICS_SAVE_FAILED in a reasonable amount of time. 

         3. this server died so rcmclose was invoked.   All sorts of cleanups were done 
            including PM. 
            
         Note the ioctl or rcmclose already stopped and removed the timer if it was active 

         If the server died, rcmclose reset all variable so the "flags" word is NULL! 
      */
      if ( current_pm_status[i].flags  && (!(current_pm_status[i].flags & X_PM_SUCCESS) ))
      {
         rc = PM_ERROR;
      }
      RCM_TRACE(0xA09, getpid(),pid,rc);
   }

   return (rc);
}

/*---------------------------------------------------------------------------
 |  Wake up all servers which the RCM has put to sleep                      |
 ---------------------------------------------------------------------------*/

long unblock_graphics_IO()
{
   uchar i;
   ulong X_pid;

   RCM_TRACE(0xB00, getpid(),0,0);

   for (i=0 ; i < RCM_MAX_DEV ; i ++)
   {
      if ( (current_pm_status[i].pid != 0) && (current_pm_status[i].flags & X_PM_SUCCESS) ) 
      {
         X_pid = current_pm_status[i].pid;

   	 RCM_TRACE(0xB01, getpid(),X_pid,0);

         e_wakeup(& current_pm_status[i].X_pm_done_sleep_word);

         current_pm_status[i].flags = GRAPHICS_INIT_COMPLETE;      /* reset all bit fields except bit one */
      }
   }

   RCM_TRACE(0xB80, getpid(),0,0);

   return (PM_SUCCESS);
}

/*--------------------------------------------------------------------------
|                                                                          |
|  When the system enters supend or hibernation state, PM core calls each  |
|  driver to shut down its device.  To shut down the devices in the right  |
|  sequence, PM core keeps track when each device registers its handle     |
|  This information forms a road map or tree structure which PM core will  |
|  use.  It informs devices at the bottom of the tree first and work       |
|  upward.  Since the ordering must be kept, PM core can only send         |
|  the shut down command to one device at any time and it must wait for    |
|  this device to complete before proceeding to the next device in the tree|
|                                                                          |
|  Since all graphics devices are configured first before the the RCM, we  |
|  will expect the RCM handler to be invoked before any graphics's hanlder |
|  when the system enter suspend/hibernation.  When resuming from those    |
|  state, we'll expect to be called after all graphics devices have resumed|
|  successfully.                                                           | 
|                                                                          |
|  This timing is crucial because we depend on it to set and clear the     |
|  "pm_event" correctly in order to prevent user to start X server when    |
|  graphics devices are about to enter syspend/hibernation mode.           |
|                                                                          |
---------------------------------------------------------------------------*/

rcm_pm_handler(private,mode)
   caddr_t private;
   int mode;
{
	int rc = PM_SUCCESS;

   	RCM_TRACE(0xD00, getpid(),pmdata.mode,mode);

	switch(mode)   /* new mode to go to */
        {
                case PM_DEVICE_FULL_ON:
                case PM_DEVICE_ENABLE:
		case PM_DEVICE_IDLE:

                        pmdata.mode = mode;
		break;

		case PM_PAGE_UNFREEZE_NOTICE:
		
			pm_event = 0;    /* clear flag */

			rc = unblock_graphics_IO();

		break;
		
		case PM_PAGE_FREEZE_NOTICE:

		       /* 
			 "pm_event" is global data and thus will need to
      			  be serialized between ioctl GSC_GRAPHICS_INIT_STARTS,
      			  and PM thread/driver ??

			  set flag so that we won't allow any more xinit.  When the
   			  system resumes from suspend/hibernation we clear this flag
			*/

			pm_event = 1;

			rc = quiesce_graphics_IO();

		break;

	        case PM_DEVICE_SUSPEND:
                case PM_DEVICE_HIBERNATION:

                        pmdata.mode = mode;
		break;
        }

   	RCM_TRACE(0xD01, getpid(),pm_event,0);

	return(rc);
}
