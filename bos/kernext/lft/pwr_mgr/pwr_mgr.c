static char sccsid[] = "@(#)97  1.4  src/bos/kernext/lft/pwr_mgr/pwr_mgr.c, lftdd, bos411, 9437C411a 9/15/94 13:59:27";
/*
 * COMPONENT_NAME: LFTDD
 *
 * FUNCTIONS: - lft_pwrmgr()
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

#include <lft.h>
#include <sys/syspest.h>

#include <graphics/gs_trace.h> 	        /* system trace stuff                                  */
#include <lft_debug.h>                  /* lft trace hooks and ids                             */

extern lft_ptr_t lft_ptr;               /* lft's anchor - initialized by lftconfig.c during    */
                                        /* CFG_INIT of LFT device                              */

struct watchdog kb_active_wd;           /* since LFT dd is pinned, these structures are pinned */
struct watchdog pwr_down_disp_wd;       

int  wdogs_init = FALSE;         	/* Boolean: T/F =(1,0) - TRUE if watchdogs have been init */
int  keystroke  = FALSE;         	/* Boolean: TRUE if keystroke(s) has taken place       */

int current_pwr_saving_state = 0;       /* 1 = full-on, 2 =standby, 3=suspend, 4= off          */

void pwr_timer();
void detect_kb_active_timer();

struct phys_displays * default_pd = NULL; /* pointer to default display                          */ 

#define KEYBOARD_CHECK_TIME 2           /* monitor for presence of keystroke(s) every 2 seconds*/


BUGVDEF(db_lft_pwrmgr,0);
BUGVDEF(db_pwr_timer,0);
BUGVDEF(db_kb_active_timer,0);

GS_MODULE(pwr_mgr);                     /* only use for full debug version */


/*----------------------------------------------------------------------------------------------
 *
 * LFT display power manager:  
 * --------------------------
 *
 * Description:       provides LFT means to control 2 watchdog timers for Display Power 
 *                    Management (DPM).
 *
 *                 
 * Flow Of Execution: during configuration of LFT driver (CFG_INIT), lft_init 
 *                    function is called.  Inside this function a check is made 
 *                    to see if DPM is enabled (i.e., the first time-out in ODM is non 
 *                    zero).  If it is, each display is turned off in the loop where 
 *                    LFT opens and initializes each display.  If all goes well (at least 
 *                    the default display is initialized), lft_init then issues the 
 *                    LFT_START_DPM_WD command to start 2 timers for DPM.  Their job is 
 *                    to turn off the the default display when it is not used and turn it 
 *                    on when keystroke(s) is detected.
 *
 *                    One timer (so called DPM timer) is used to call the display driver 
 *                    (of the default display) to power down the display gradually according 
 *                    to the 3 timeouts specified in the ODM.  It does not power down the
 *                    default display if someone is using the machine (i.e., keystrokes are
 *                    detected).  It does it by checking a flag maintained by lftKiCb and keyboard 
 *                    activity timer.
 *
 *                    The other so called keyboard activity timer is used to detect when to 
 *                    turn on the default display.  It does it by cheking a flag every 2 seconds.  
 *                    This flag is set by the LFT keyboard callback function, lftKiCb.  The 
 *                    callback function is invoked by the keyboard driver every time it puts 
 *                    a scan code in the keyboard input ring which is empty.  If the flag is 
 *                    set to 1 (TRUE), it means the machine is being used by someone.  Therefore 
 *                    the keyboard activity timer will stop the DPM timer, call the display 
 *                    driver to set the DEFAULT display to full-on if the display is not on, 
 *                    clear the keyboard activity flag to zero (FALSE). and then restart the 
 *                    both timers again. 
 *
 *                    What is a default display?  The default display is the display 
 *                    which is displaying text in LFT mode.  On a system with several 
 *                    available displays, only one display can be the default at
 *                    any time.  Users select the default display by issuing the 
 *                    chdisp -d <display name> command.
 *       
 *                    Since all displays are turned off, except for the default display 
 *                    when an user selects a different default display, the 
 *                    the LFT_SET_DFLT_DISP command is issued to the LFT stream code 
 *                    (lftsi.c).  In here we issue LFT_RESTART_DPM_WD command to inform 
 *                    the two timers of the new default display.  In this case, we stop 
 *                    both timers, call the display driver to turn off the old default
 *                    display and turn on the new default display, restart the DPM timer
 *                    timer with phase 1 timeout value, and restart the keyboard
 *                    activity timer.  From now on both timers should operate on 
 *                    this new default display which we keep track of
 *
 *                    When X or TU requests to use a display (or displays), the RCM
 *                    (part of make_gp) will issue an ioctl(LFT_ACQUIRE_DSP) to
 *                    grasp the display(s).  At this point LFT will do two things:
 *                    if the acquired display happens to be the defualt display, LFT
 *                    issues the LFT_STOP_DPM_WD command to stop the both watchdog 
 *                    timers and calls the driver to turn on the display; if the acquired 
 *                    display is not the default display, LFT only calls the driver to turn 
 *                    it on. 
 *
 *                    When X or TU releases a display (or displays), the RCM
 *                    (part of unmake_gp) issues an ioctl(LFT_REL_DISP) to the LFT 
 *                    driver.  Here the LFT checks to see if the released
 *                    display is the default display.  If it is, it issues
 *                    the LFT_START_DPM_WD command.  If the released display 
 *                    is a non default display, the LFT just calls the driver
 *                    to turn it off. 
 *
 *                    All these timers are started as part of LFT configuration 
 *                    It's important to stop and remove them when LFT is
 *                    unconfigured.  When LFT is unconfigured, lftterm() 
 *                    is called.  In here LFT turns on each display and
 *                    issues LFT_REMOVE_DPM_WD command to remove both timers.
 *
 *   Files To Study :  src/bos/kernext/lft/config/lftconfig.c
 *                     src/bos/kernext/lft/config/lftterm.c
 *                     src/bos/kernext/lft/config/lftinit.c
 *                     src/bos/kernext/lft/stream/lftki.c (lftKiCb)
 *                     src/bos/kernext/lft/stream/lftsi.c (lftwput, lftwsrv)
 *                     src/bos/kernext/lft/inc/lft_dds.h
 *
 -----------------------------------------------------------------------------------------------*/

long lft_pwrmgr( command )
  int command;
{
	int rc=0;

	lft_dds_t * dds_ptr = lft_ptr->dds_ptr; /* pointer to LFT dds structure */ 

	GS_ENTER_TRC(HKWD_GS_LFT,pwr_mgr,0,lft_pwrmgr,default_pd,command,dds_ptr->pwr_mgr_time[0],0,0);

	BUGLPR(db_lft_pwrmgr, 0, ("enter lft_pwrmgr\n"));

	/* if phase 1 timeout is 0 then don't do Display Power Manager */
	if( dds_ptr->pwr_mgr_time[0] == 0 )
        {
		BUGLPR(db_lft_pwrmgr,0,("phase 1 timeout = zero\n"));

		GS_EXIT_TRC(HKWD_GS_LFT,pwr_mgr,0,lft_pwrmgr,0,0,0,0,0);

		return(0);
        }


        BUGLPR(db_lft_pwrmgr,3,("timeouts %d %d %d\n",dds_ptr->pwr_mgr_time[0], 
                                           dds_ptr->pwr_mgr_time[1],
                                           dds_ptr->pwr_mgr_time[2])); 
	
	switch( command )
	{
	      /*  start 2 timers - one to detect keyboard activity and  
               *  the other to power down the default display if noone 
               *  uses the machine
               */
	  case START_LFT_DPM_WD:		

			BUGLPR(db_lft_pwrmgr,2,("cmd = START_LFT_DPM_WD\n"));

			if (! wdogs_init)
			{

			   default_pd = dds_ptr->displays[dds_ptr->default_disp_index].vtm_ptr->display;

			   kb_active_wd.restart = KEYBOARD_CHECK_TIME;  
			   kb_active_wd.next    = kb_active_wd.prev = NULL;
			   kb_active_wd.func    = detect_kb_active_timer;
			   kb_active_wd.count   = 0;

			   pwr_down_disp_wd.restart = dds_ptr->pwr_mgr_time[0];
			   pwr_down_disp_wd.next    = pwr_down_disp_wd.prev = NULL;
			   pwr_down_disp_wd.func    = pwr_timer;
			   pwr_down_disp_wd.count   = 0;

			   w_init( & kb_active_wd);
			   w_init( & pwr_down_disp_wd);

			   wdogs_init = TRUE;

			}
			else
			{

			   BUGLPR(db_lft_pwrmgr,0,("wds already init. so restart them\n"));

			   w_stop( & kb_active_wd);  
			   w_stop( & pwr_down_disp_wd); 

			   pwr_down_disp_wd.restart = dds_ptr->pwr_mgr_time[0];

			}

			/* turn on the default display */
			if (default_pd->vttpwrphase != NULL)
			{
			   rc = (* default_pd->vttpwrphase)(default_pd,1);
			}

			current_pwr_saving_state = 1;   /*  current power saving mode, full-on */ 

			w_start( & kb_active_wd);
			w_start( & pwr_down_disp_wd);

		
		break;

	  case STOP_LFT_DPM_WD:		

			BUGLPR(db_lft_pwrmgr,2,("cmd = STOP_LFT_DPM_WD\n"));

			if (wdogs_init)
			{

			   w_stop( & pwr_down_disp_wd);
			   w_stop( & kb_active_wd);

			   keystroke = FALSE;
			   current_pwr_saving_state =0;

	 		   rc = PWRPROC_SUCC;
			}
			else
			{

			   BUGLPR(db_lft_pwrmgr,0,("STOP_LFT_DPM_WD failed\n"));
			   rc =ILLEGAL_OP;
			}

		break;

	      /* 
               * stop and remove the idle watchdog timer
               * should be called by lft during CFG_TERM only.
               */
	  case REMOVE_LFT_DPM_WD:

			BUGLPR(db_lft_pwrmgr,2,("cmd = REMOVE_LFT_DPM_WD\n"));

			if (wdogs_init)
			{

			   w_stop( & kb_active_wd);
			   w_stop( & pwr_down_disp_wd);

			   w_clear(& kb_active_wd);
			   w_clear(& pwr_down_disp_wd);

			   wdogs_init = FALSE;
			   default_pd = NULL;
			   current_pwr_saving_state =0;
			   keystroke = FALSE;

	 		   rc = PWRPROC_SUCC;
			}
			else
			{

			   BUGLPR(db_lft_pwrmgr,0,("REMOVE_LFT_DPM_WD failed\n"));
			   rc = ILLEGAL_OP;
			}

		break;

	  case RESTART_LFT_DPM_WD:			/* "chdisp -d" has been issued */

		BUGLPR(db_lft_pwrmgr, 2,("cmd = RESTART_LFT_DPM_WD\n"));

                if (wdogs_init)
                {
		   /* stop both timer since we have new default display */

                   w_stop( & kb_active_wd);
                   w_stop( & pwr_down_disp_wd);

		   /* since we have new default display, turn off the old default display */
		   if (default_pd->vttpwrphase != NULL)
		   {
		      rc = default_pd->vttpwrphase(default_pd,4);
		   }

		   /* update current default display pointer  */
		   default_pd = dds_ptr->displays[dds_ptr->default_disp_index].vtm_ptr->display;

		   /* turn on the new/current default display */
		   if (default_pd->vttpwrphase != NULL)
		   {
		      rc |= default_pd->vttpwrphase(default_pd,1);
		   }

		   current_pwr_saving_state =1;
		   keystroke = FALSE;

		   pwr_down_disp_wd.restart = dds_ptr->pwr_mgr_time[0]; /* set the time-out for phase 1 */ 

                   w_start( & kb_active_wd);
                   w_start( & pwr_down_disp_wd);

                 }
                 else
                 {

                      BUGLPR(db_lft_pwrmgr,0,("RESTART_LFT_DPM_WD failed\n"));
                      rc = ILLEGAL_OP;
                 }

		break;

	  default:			/* return bad command code */

		BUGLPR(db_lft_pwrmgr, 2, ("cmd = UNKNOWN\n"));
		rc = BAD_COMMAND;
	}

	BUGLPR(db_lft_pwrmgr, 0, ("exit lft_pwrmgr rc =%d\n", rc));

	GS_EXIT_TRC(HKWD_GS_LFT,pwr_mgr,0,lft_pwrmgr,default_pd,command,rc,0,0);

	return(rc);
}



void pwr_timer(w)
struct watchdog * w;
{
	int next_pwr_saving_state, rc=0;

        BUGLPR(db_pwr_timer, 0,("enter pwr_timer\n"));

	if(keystroke)
	{
       	   pwr_down_disp_wd.restart = lft_ptr->dds_ptr->pwr_mgr_time[0];
           w_start( &pwr_down_disp_wd );
	   return;
	}

	next_pwr_saving_state = current_pwr_saving_state +1;	

	if( (next_pwr_saving_state <= 4 ) ) 
	{

      	   BUGLPR(db_pwr_timer, 0,("pwr_timer: call change_phase - new phase =%d\n",next_pwr_saving_state)); 
           if( default_pd->vttpwrphase != NULL ) /* put display in next power saving state */
           {
                rc = (* default_pd->vttpwrphase)(default_pd, next_pwr_saving_state);

                BUGLPR(db_lft_pwrmgr, 0,("called and returned from vttpwrphase - rc = %d\n", rc));
           }

	   /* when next state is 4, the default display is turned off so we don't
            * want to restart the watchog.  Note index to array would be out of  
            * range if we did restart the watchdog since there is only 3 time-outs
            */
	   if (next_pwr_saving_state < 4)   
	   {
       	      pwr_down_disp_wd.restart = lft_ptr->dds_ptr->pwr_mgr_time[next_pwr_saving_state - 1];

              w_start( &pwr_down_disp_wd );
	   }

	   current_pwr_saving_state = next_pwr_saving_state ; 

	}

        BUGLPR(db_pwr_timer, 0,("exit pwr_timer\n"));
}


void detect_kb_active_timer(w)
struct watchdog * w;            
{

  	int  rc=0;

        BUGLPR(db_kb_active_timer, 1,("enter detect_kb_active_timer\n"));

        BUGLPR(db_kb_active_timer, 1,("index =%d, default pd=%x\n", dds_ptr->default_disp_index, default_pd));
       /* 
        * this flag is set by lftKiCb() in lftki.c
        * everytime the keyboard dd receives keyboard intr. 
        * and it puts the keystrokes in the empty input ring, it
        * calls lftKiCb().  There the "keystroke" flag is set to
        * tell this timer someone is using the machine.
        */ 

	if (keystroke)
	{
            BUGLPR(db_kb_active_timer, 0,("keyboard activity detected\n"));


	    /* 
	     * stop Display Power Management watchdog timer
	     */

	     w_stop(&pwr_down_disp_wd);  

	     /* lftKiCb (lftki.c) sets it to TRUE if keystrokes are detected
	      * We need to clear it here so that the DPM timer knows to 
              * power down the default display whenever it pops
              */
	     keystroke = FALSE;    

	     if (default_pd->vttpwrphase != NULL)
	     {
                /* if the default display has been turned off, turn it 
                 * on and and restart the DPM timer for phase 1
		 */
	        if (current_pwr_saving_state !=1)
		{
	           rc = (* default_pd->vttpwrphase)(default_pd, 1);
	   	   current_pwr_saving_state = 1; 

	           /* reset timeout value for phase 1 */
       	           pwr_down_disp_wd.restart = lft_ptr->dds_ptr->pwr_mgr_time[0];
		}
	     }
	     else
	     {
                BUGLPR(db_kb_active_timer, 0,("kb_active_timer: vttpwrphase is null\n"));
             }

	     /* restart DPM timer for phase 1 every time keystroke(s) is detected 
	      * so that the DPM timer only turns off the display iff phase 1 time-out 
	      * expires since the last detected keystroke (i.e., keystroke == 0 (FALSE))
              */ 
	     w_start(&pwr_down_disp_wd);

	}
	else
	{
            BUGLPR(db_kb_active_timer, 1,("keyboard activity not detected\n"));
	}

	/* restart the watchdog */
	w_start(w);

        BUGLPR(db_kb_active_timer, 0,("exit detect_kb_active_timer rc =%d\n", rc));
}
