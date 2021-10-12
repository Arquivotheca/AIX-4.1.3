/* @(#)66	1.2  src/bos/kernext/rcm/inc/rcm_pm.h, rcm, bos41J, 9512A_all 3/20/95 11:29:27 */

/*
 *   COMPONENT_NAME: rcm
 *
 * FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_RCM_PM
#define _H_RCM_PM

/*    

   In aix 410, the RCM and graphic drivers allow multiple xinits.  However only 
   the very first xinit succeeds.  The rest will terminate because it cannot
   open /dev/mouse0.
   
   So what does this mean?  Should I allow more than 1 data structure below?
   Yes.  After talking to Gary Anderson, he said the problem will be fixed 
   to allow multiple xinit.  Since we allow at most 4 adapters in a machine,
   and assuming each xinit needs one display, the maximum number of xinits
   allowed are 4.
    
*/

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/watchdog.h>

/*---------------------------------------------------------------------------
 |                                                                          |
 |  Considering that fact that there are at most 4 adapters can be in the   | 
 |  machine, maximum number of xinit's we can handle is four                |
 |                                                                          |
 |--------------------------------------------------------------------------*/

 /* use  #define RCM_MAX_DEV in rcm.h  */

/*---------------------------------------------------------------------------
 |                                                                          |
 |  How long the RCM can wait after sending the signal to X for it to       | 
 |  quiesce IO and save its data                                            |
 |                                                                          |
 |--------------------------------------------------------------------------*/

#define MAX_2D_DATA_SAVING_TIME 10      /* seconds */

/*---------------------------------------------------------------------------
 |                                                                          |
 | Misceleneous flags                                                       |
 |                                                                          |
 |--------------------------------------------------------------------------*/

#define ADD_DSP  	0x1             /* for rcmioctl.c */
#define REMOVE_DSP      0x2

/* -------------------------------------------------------------------------|
 |                                                                          |
 | information on each server that needs to save data when graphic device(s)|
 | have to enter suspend or hibernation mode                                |
 |                                                                          |
 ---------------------------------------------------------------------------*/


struct rcm_pm_data 
{
   /* Note no thing should be declare above the watchdog structure */
 
   struct watchdog wd;        /* prevent RCM to put PM thread sleep forever*/
                              /* waiting for this server to save its data  */


   ulong pid;                 /* pid of a server granted permisson to come*/
                              /* up. The pid is filled in by ioct         */
                              /* GSC_GRPAHICS_INIT_STARTS                 */

   int flags;                 /* flag word to indicate events              */


                              /* set by the ioctl GSC_GRAPHICS_INIT_ENDS   */
#define GRAPHICS_INIT_COMPLETE  (1 << 0)

                              /* set by callback save_graphics_dsp_data. It*/
                              /* indicates the signal SIGUSR2 has been     */
                              /* sent to this x server                     */
#define SIGUSR2_SENT          (1 << 1)

                              /* callback save_graphics dsp_data starts a */
                              /* timer before sending SIGUSR2.            */
#define X_PM_WD_ON            (1 << 2)

                              /* flag to indicate this server failed      */
                              /* to save its data within a reasonable     */
                              /* amount of time.  Thus the RCM has to kill*/
                              /* it.  The RCM state change handler needs  */
                              /* to perform clean-up and wake up PM thread*/
                              /* sleeping on "terminate_X_sleep_word"     */
#define X_PM_TIMED_OUT        (1 << 3)

                              /* set by callback save_graphics_dsp_data   */
                              /* when it has to terminate this server     */
                              /* state change handler will check for this */
#define KILL_X_SERVER         (1 << 4)

                              /* set by ioctl GSC_GRAPHICS_DATA_SAVED     */
#define X_PM_SUCCESS          (1 << 5)


   uchar num_of_dsp_owned;    /* if this server is allowed to come up, keep*/
			      /* track how many display(s) it has acquired.*/
                              /* Ioctl(GSC_HANDLE) updates variable        */




   int X_pm_sleep_word;       /* used to suspend PM thead (during suspend/ */
                              /* hibernation) while X is saving its data   */ 
                              /* The PM thread is waken up when this server*/
                              /* issues the ioctl GSC_GRAPHICS_DATA_SAVED  */

   int terminate_X_sleep_word;/* use to suspend PM thread when RCM has to  */
                              /* terminate this server because it failed to*/
                              /* issue ioctl GSC_GRAPHICS_DATA_SAVED in a  */
		              /* reasonable amount of time.  This usually  */ 
                              /* causes by the server either hung or took  */
                              /* too long when saving its data             */

   int X_pm_done_sleep_word;  /* used to supend this server after it's saved*/
                              /* data for suspend/hibernation.  It happens */
                              /* when the  server issues the               */ 
                              /* ioctl GSC_GRAPHICS_DATA_SAVED             */

   int spares[5];             /* future usage                              */ 
};

/* -------------------------------------------------------------------------|
 | global data    -- modified by ioctls GSC_HANDLE, GSC_GRAPHICS_INIT_STARTS|
 |                   GSC_GRAPHICS_INIT_ENDS, GSC_HANDLE_RELEASE             |
 |                                                                          |
 |                -- modified by callback functions save_graphics_data and  |
 |                   restore_graphics_data                                  |
 |                                                                          |
 | information on each server that needs to save data when graphic device(s)|
 | have to enter suspend or hibernation mode                                |
 |                                                                          |
 ---------------------------------------------------------------------------*/

extern struct rcm_pm_data current_pm_status[];   

/*---------------------------------------------------------------------------
 |                                                                          |
 | global variable -- set by save_graphic_data function.  This occurs when  |
 |                    graphics devices have to enter suspend/hibernation    |
 |                                                                          |
 |                 -- cleared by restore_graphic_data.  This occurs when    |
 |                    graphics devices resume from suspend/hibernation mode |
 |                                                                          |
 ---------------------------------------------------------------------------*/

extern uchar pm_event;

/*---------------------------------------------------------------------------
 |                                                                          |
 | global variable -- used by save_graphic_data function.  This occurs when | 
 |                    a device PM handle is invoked by PM core to put the   |
 |                    in suspend or hibernation mode.  However until all    |
 |                    server(s), if there is any, complete its initializa-  |
 |                    tion, the callback can't determine the mode -- KSR or |
 |                    GRAPHICS.  Therefore it is necessary to suspend the   |
 |                    execution of the device PM thread temporarily until   |
 |                    some server  issues the ioctl GSC_GRAPHICS_INIT_ENDS  |
 |                                                                          |
 ---------------------------------------------------------------------------*/

extern int X_init_sleep_word; 

extern struct pm_handle pmdata; 

#endif
