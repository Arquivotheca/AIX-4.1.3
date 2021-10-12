static char sccsid[] = "@(#)56	1.9  src/bos/kernext/rcm/rcmioctl.c, rcm, bos41J, 9517B_all 4/25/95 11:33:26";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Ioctls
 *
 * FUNCTIONS:  rcmioctl (devname, cmd, arg)
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989-1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * FUNCTIONAL DESCRIPTIONS:
 *
 * rcmioctl:
 *	  Error Philosophy:
 *			  All calls to non internal routines require logging an
 *			  error at the point of the error. Routines the are
 *			  internal (the vtm*'s) are required to log their own
 *			  errors and return the error code to this routine.
 *			  If no error found then return a 0 to the user.
*/


#include <lft.h>		/* includes for all lft related data */
#include <sys/lft_ioctl.h>		/* includes for all lft related data */
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/priv.h>
#include <sys/termio.h>
#include <sys/device.h>
#define _DISPLAYTRACEFLAG
#include <sys/disptrc.h>
#include "rcm_mac.h"
#include <rcmdds.h>		/* RCM dds structure */
#include <sys/rcmioctl.h>
#include "xmalloc_trace.h"
#include "rcm_pm.h"
#include <sys/sleep.h>

BUGVDEF(dbg_rcmioctl,99);

#ifndef BUGPVT
#define BUGPVT 99
#endif

extern rcm_dds *rcmdds;
extern update_pm_data();
 
ulong get_handle (int);

/*----------------------------------------------------------------------
  rcmioctl
 	  Error Philosophy:
 	  All calls to non internal routines require logging an		
 	  error at the point of the error. Routines the are		
 	  internal (the vtm*'s) are required to log their own	
 	  errors and return the error code to this routine.	   
 	  If no error found then return a 0 to the user.	
 *----------------------------------------------------------------------*/

rcmioctl (dev, cmd, arg, mode, mpx, ext)
dev_t dev;	/* major, minor number of device */
int cmd;	/* ioctl command */
int arg;	/* buffer pointer, usually */
ulong mode;	/* ??? */
int mpx;	/* vt_id, channel */
int ext;
{
	int rc = 0;	 /* return code */
	int i,j,k;	 /* generic indexes for loops */
	int  lock_stat;  /* for global lock status */
	int old_lvl;

	RCM_LOCK (&comAnc.rlock, lock_stat);

	if (rcmdds->open_count == 0)
	{
		RCM_UNLOCK (&comAnc.rlock, lock_stat);

#ifdef  RCM_SIGSYS
		pidsig (getpid (), SIGSYS);
#endif
		return (ENXIO);
	}

	switch (cmd)
	{
	    case GSC_HANDLE:	/* GP leader get handle for device */
	    {
		gsc_handle gschandle;
		int  inuse = 0;

		/* copy in handle info into the kernel space */
		if ( rc = rcm_copyin(arg,&gschandle,sizeof(gsc_handle),mode) )
		{
		    BUGLPR(dbg_rcmioctl,BUGPVT,
					("copyin of hndl info failed\n"));
		    break;
		}

		/* search array of displays for the handle name passed in */
		for ( i=0; i < rcmdds->number_of_displays; i++ )
		{
		    if( ! strcmp(rcmdds->disp_info[i].lname, gschandle.devname))
			break;
		}
	
		/* error condition if handle name not found */
		if ( i >= rcmdds->number_of_displays )
		{
		    BUGLPR(dbg_rcmioctl, BUGPVT,
				("Device %s is not available for the LFT\n",
						gschandle.devname));
		    rc = ENXIO;
		    break;
		} 
	
		/*
		 *  If the driver did not open properly, check for this
		 *  and return an error.
		 */
		if (rcmdds->disp_info[i].flags & RCM_DRIVER_NOTOK)
		{
		    BUGLPR(dbg_rcmioctl, BUGPVT,
				("Device %s did not open properly.\n",
						rcmdds->disp_info[i].lname));
		    rc = EIO;
		    break;
		}

		/*
		 *  Perform test and set for 'in use' flag.
		 *
		 *  Flag test and set is all under protection of the common
		 *  lock.  Clearing the flag is sometimes done in the
		 *  devno_state_change handler without protection.  The
		 *  logic is closed, but be careful.
		 */
		if (rcmdds->disp_info[i].flags & RCM_DEVNO_IN_USE)
		{
		    /*  
		     *  Actually allow a re-GSC_HANDLE from a process that 
		     *  already owns it.
		     *
		     *  Tid's are systemwide global, so no pid check need
		     *  be made.
		     */
		    if (rcmdds->disp_info[i].tid != thread_self ())
		    {
			BUGLPR(dbg_rcmioctl, BUGPVT,
				("Device %s is owned by someone else.\n",
						rcmdds->disp_info[i].lname));

			rc = EBUSY;

			break;
		    }

		    BUGLPR(dbg_rcmioctl, BUGPVT,
				("Device %s is owned by this process.\n",
						rcmdds->disp_info[i].lname));

		    inuse = 1;		/* prevent release in err path below */
		}
		else
		{
		    /* Pid identifies the leader. set in-use flag last */
		    rcmdds->disp_info[i].pid = getpid ();
		    rcmdds->disp_info[i].tid = thread_self ();
		    rcmdds->disp_info[i].handle = get_handle (i);
		    rcmdds->disp_info[i].flags |= RCM_DEVNO_IN_USE;

		    /* AIX Power Management -- increment number of displays this sever owns */
		    update_pm_data(rcmdds->disp_info[i].pid,ADD_DSP);     
		    
		    BUGLPR(dbg_rcmioctl, BUGPVT,
			("Device %s has been acquired by this process.\n",
						rcmdds->disp_info[i].lname));
		}

		/* Write out handle back to user space */
		gschandle.handle = rcmdds->disp_info[i].handle;
		if (rc = rcm_copyout(&gschandle,arg,sizeof(gsc_handle),mode))
		{
		    BUGLPR(dbg_rcmioctl,BUGPVT,
					("copyout of handle info failed\n"));

		    /* if we didn't mark it in use on this call */
		    if (!inuse)
		    {
		        /* AIX Power Management -- decrement number of displays this sever owns */
		        update_pm_data(rcmdds->disp_info[i].pid,REMOVE_DSP);

			/* Clear in-use first. */
			rcmdds->disp_info[i].flags &= ~RCM_DEVNO_IN_USE;
			rcmdds->disp_info[i].pid = 0;
			rcmdds->disp_info[i].tid = 0;
			rcmdds->disp_info[i].handle = 0;
		    }
		}
		else
		{
		  BUGLPR (dbg_rcmioctl, BUGNFO,
		    ("rcmioctl: GSC_HANDLE: display %s, handle 0x%x\n",
		    rcmdds->disp_info[i].lname, rcmdds->disp_info[i].handle));
		}

		break;
	    }

	    case GSC_HANDLE_RELEASE:	/* GP leader (?) releases device */
	    {
		gsc_handle gschandle;
		int  count;

		/* copy in handle info into the kernel space */
		if ( rc = rcm_copyin(arg,&gschandle,sizeof(gsc_handle),mode) )
		{
		    BUGLPR(dbg_rcmioctl,BUGPVT,
					("copyin of hndl info failed\n"));
		    break;
		}

		count = 0;
		for (i=0; i<rcmdds->number_of_displays; i++)
		{
		    if ((rcmdds->disp_info[i].flags & RCM_DEVNO_IN_USE) &&
			    rcmdds->disp_info[i].handle == gschandle.handle    )
		    {
			BUGLPR (dbg_rcmioctl, BUGNFO,
				   ("rcmioctl: GSC_HANDLE_RELEASE: %s\n",
					rcmdds->disp_info[i].lname));

			/* dev_init'd and not dev_term'd? */
			if (rcmdds->disp_info[i].flags & RCM_DEVNO_MAKE_GP)
			{
			    rc = EBUSY;
			    break;
			}

		        /* AIX Power Management -- decrement number of displays this sever owns */
		        update_pm_data(rcmdds->disp_info[i].pid,REMOVE_DSP);   

			/* clear in-use flag first */
			rcmdds->disp_info[i].flags &= ~RCM_DEVNO_IN_USE;
			rcmdds->disp_info[i].pid    = 0;
			rcmdds->disp_info[i].tid    = 0;
			rcmdds->disp_info[i].handle = 0;


			count++;
			break;		/* can be only one */
		    }
		}

		if (!rc && count < 1)
		    rc = EINVAL;

		break;
	    }

	    case RCM_QUERY:			/* return data to caller */
		break;

	    case RCM_SET_DIAG_OWNER:		/* assign ownership to diag */
	    {
		gsc_handle handle;
		
		/* copy in handle into the kernel space */
		if ( rc = rcm_copyin(arg, &handle, sizeof(gsc_handle), mode) )
		{
			BUGLPR(dbg_rcmioctl,BUGPVT,
					("copyin of handle failed\n"));
			break;
		}

		for (i=0; i<rcmdds->number_of_displays; i++)
		{
			if ((rcmdds->disp_info[i].flags & RCM_DEVNO_IN_USE) &&
			    rcmdds->disp_info[i].handle == handle.handle       )
				break;
		}
		if (i == rcmdds->number_of_displays)
		{
			rc = EINVAL;
			break;
		}

		/* issue ioctl to lft informing it of diagnostics intentions */
		if ( (rc = fp_ioctl(rcmdds->lft_info.fp, LFT_SET_DIAG_OWNER, 
			&rcmdds->disp_info[i].devno, 0)) != 0 )
		{
			BUGLPR(dbg_rcmioctl,BUGPVT,("ioctl to lft failed\n"));
		 	break;
		}
		
		break;
	    }

	    case GSC_GRAPHICS_INIT_STARTS:
	    {
	       ulong pid;

	       pid = getpid();

      	       /* entire system is entering suspend or hibernation state, so
                  don't allow any more xinit.  Note the X server has to
                  check for return code from this ioctl.  If the system call fails
                  X should check errno to determine to determine if it can proceed 

                     1.  if due to a PM event, the errno is set to EACCES.  In this case
                         the server must terminate.

                     2.  if due to internal error, the errno is set to EFAULT or ENOSPC.  In 
                         this case the server must terminate.

                     3.  if invalid ioctl, the errno is set to EINVAL.  In this case, the
                         server can proceed as usual.  This could happen because the new 
 			 X server with AIX Power Managemet can run without the new rcm with
 		         AIX Power Management enhancement.

                  It would be nice if we could put the server to sleep and wake
                  it up when the system resumes.  However the complexity of
                  the design and implememtation would increase many folds, just
                  for a little convenience!  It is not worth the trouble.

	          Note no RCM_TRACE is invoked for this ioctl because when the very
                  first aixgsc command is issued, the RCM zeroes out the entire
                  trace buffer.  Therefore, the trace buffer does not contain
                  any information when trace this ioctl.  So don't try to trace it. 
               */
	       old_lvl = i_disable(INTMAX);    /* make sure we aren't interrupted in the middle */ 
   	       if (pm_event)
   	       {
                  rc = EACCES;                 
   	       }
               else  
               {
                  /* system is not entering suspend/hibernation state so RCM grants
                     this sever the permission to come up
		
                     First do some sanitary checking
               	   */
               	  for (i=0 ; i < RCM_MAX_DEV ; i ++)
               	  {
                      if (pid == current_pm_status[i].pid) 
		      {
               	         rc = EFAULT;
		 	 break;
		      }
               	  }

               	  /* look for an unsed entry in current_pm_status to
               	     grant this server permission to come up
               	   */
		   if (rc == 0)
		   {
               	       for (i=0 ; i < RCM_MAX_DEV ; i ++)
               	       {
                          if (current_pm_status[i].pid == 0) break;
               	       }
               	       if (i >= RCM_MAX_DEV)
               	       {
                          /* only handle up to four xinit's -- one too many ! */
                  	  rc = ENOSPC;
               	       }
               	       else
               	       {
		          /* record permisson granted for this server */
                  	  current_pm_status[i].pid = pid;  
               	       }
	           }
               }

	       i_enable(old_lvl);

	       break;
	    }

            case GSC_GRAPHICS_INIT_ENDS:
            {
		
	        RCM_TRACE(0xC00, getpid(),0,0);

                for (i=0 ; i < RCM_MAX_DEV ; i ++)
                {
                   if ( getpid() == current_pm_status[i].pid) break;
                }

                if (i >= RCM_MAX_DEV)
                {
                   /* something is very wrong - RCM does not have record of
                      this sever being granted to come up.
                    */
                    rc=ESRCH;
	            RCM_TRACE(0xC01, 0,0,0);
                }
		else
		{
		   /* serialize with save_graphics_display_data() */

		    old_lvl = i_disable(INTMAX);    

                    current_pm_status[i].flags |= GRAPHICS_INIT_COMPLETE;

	            RCM_TRACE(0xC02, 0,0,0);
                    /* if any PM thread/driver is waiting for this sever
                       to finish its initializaton, now it can proceed.
                     */
               	    if (X_init_sleep_word != EVENT_NULL)
                    {
	               RCM_TRACE(0xC03,0,X_init_sleep_word,0);
                       e_wakeup(& X_init_sleep_word);
               	    }
	
		    i_enable(old_lvl);
		}

	        RCM_TRACE(0xC04, getpid(),rc,0);

                break;
            }

            case GSC_GRAPHICS_DATA_SAVED:
            {
	        RCM_TRACE(0xC05, getpid(),0,0);

                for (i=0 ; i < RCM_MAX_DEV ; i ++)
                {
                   if ( getpid() == current_pm_status[i].pid) break;
                }

		old_lvl = i_disable(INTMAX);   /* our PM timer might interfere */

                if (i >= RCM_MAX_DEV)
                {
                   /* something is very wrong - RCM does not have record of
                      this sever being granted to come up.
                    */

                   rc=ESRCH;

		   RCM_UNLOCK (&comAnc.rlock, lock_stat);   /* because of the return below */
                }
		else
		{
                   /* This sever finished saving its data, so the PM thread/driver
                      can now proceed to save additional data and turn off devices

                      Meanwhile we need to stop this server until the entire
                      system resumes from suspend or hibernation state.  At that
                      time each driver will resume and call the RCM callback,
                      restore_graphics_data, to wake up this server
               	   */

	       	   RCM_TRACE(0xC06, getpid(), current_pm_status[i].flags,0);

		   current_pm_status[i].flags |= X_PM_SUCCESS;   

         	   if (current_pm_status[i].flags & X_PM_WD_ON)
         	   {
	       	      RCM_TRACE(0xC07, 0,0,0);
            	      w_stop (& current_pm_status[i].wd ) ;
            	      w_clear( & current_pm_status[i].wd );
            	      current_pm_status[i].flags &= (~ X_PM_WD_ON);
		   }

		   /* wake up PM thread/driver */
                   e_wakeup(& current_pm_status[i].X_pm_sleep_word);  
	
		   /*
                    * stop this server.  Note we have to release the lockl acquired upon
                    * entering the ioctl system call.  The reason is that kernel PM code
                    * (pm_proc_stop) can't quiesce processes preparing for system suspend
                    * or hibernation if any process sleeps and still owns a lock.  Instead of
                    * releasing the lock explicitly, we could have used e_sleepl() which 
                    * releases the lock implicitly.  Someone could easily miss this so
                    * the we have decided to release the lock explicitly.  Although it makes
                    * the code a bit ugly.
                    */

		   RCM_UNLOCK (&comAnc.rlock, lock_stat); 

                   e_sleep(& current_pm_status[i].X_pm_done_sleep_word, EVENT_SHORT); 
		}

		i_enable(old_lvl);

	       	RCM_TRACE(0xC08, getpid(),rc,0);

		BUGLPR (dbg_rcmioctl, BUGNFO, ("rcmioctl: rc %d\n", rc));

	        return(rc);   /* --> exit ioctl system call */

		break;       
            }

            case GSC_GRAPHICS_SAVE_FAILED:
            {

	       	RCM_TRACE(0xC09, getpid(),0,0);

                for (i=0 ; i < RCM_MAX_DEV ; i ++)
                {
                        if ( getpid() == current_pm_status[i].pid) break;
                }

		old_lvl = i_disable(INTMAX);  /* our PM timer might interfere */

                if (i >= RCM_MAX_DEV)
                {
                   /* something is very wrong - RCM does not have record of
                      this sever being granted to come up.
                    */
                   rc=ESRCH;
                }
		else
		{
                   /* 
                       This sever failed to save its data, so wake up PM thread/driver
                       and stop the watchdog timer (started by callback save_graphics_dsp_data)
                       if it is active.
                   */

	       	   RCM_TRACE(0xC0A,current_pm_status[i].flags,0,0);

         	   if (current_pm_status[i].flags & X_PM_WD_ON)
         	   {
            	      w_stop (& current_pm_status[i].wd ) ;
            	      w_clear( & current_pm_status[i].wd );
            	      current_pm_status[i].flags &= (~ X_PM_WD_ON);
		   }

		   /* wake up PM thread/driver */
                   e_wakeup(& current_pm_status[i].X_pm_sleep_word);      
		}

		i_enable(old_lvl);

	       	RCM_TRACE(0xC0B,getpid(),rc,0);

		break;
            }

            case GSC_GRAPHICS_QUERY_PM_STATUS:
            {

	       	RCM_TRACE(0xC0C,getpid(),pm_event,0);
                for (i=0 ; i < RCM_MAX_DEV ; i ++)
                {
                   if ( getpid() == current_pm_status[i].pid) break;
                }

                if (i >= RCM_MAX_DEV)
                {
                   /* something is very wrong - RCM does not have record of
                      this sever being granted to come up.
                    */
                   rc=ESRCH;
                }
		else
		{
                   /* This sever has received SIGUSR2.  It wants to know if 
                      the signal was sent due to system suspend/hibernation 
                   */

         	   if (current_pm_status[i].flags & SIGUSR2_SENT)
	           {
			rc = rcm_copyout(&pm_event,arg,sizeof(pm_event),mode);
	       	        RCM_TRACE(0xC0D,getpid(),rc,0);
		   }
		}

	       	RCM_TRACE(0xC0E,getpid(),rc,0);
		break;
            }

	    default: /* unknown command so set error code */
		rc = EINVAL;
	}

	RCM_UNLOCK (&comAnc.rlock, lock_stat);

	BUGLPR (dbg_rcmioctl, BUGNFO, ("rcmioctl: rc %d\n", rc));

#ifdef  RCM_SIGSYS
	if (rc)
		pidsig (getpid (), SIGSYS);
#endif
	return(rc);
}


rcm_copyout(src,dst,len,mode)
char *src,*dst;
long len;
ulong mode;
{
  long rc = 0;

  if (!(mode & FKERNEL)) {
	 if (rc = copyout(src,dst,len)) {
		return(EFAULT);
	 }
  }
  else {
	 bcopy(src,dst,len);
  }
  return(rc);
}

rcm_copyin(src,dst,len,mode)
char *src,*dst;
long len;
ulong mode;
{
  long rc = 0;

  if (!(mode & FKERNEL)) {
	 if (rc = copyin(src,dst,len)) {
		return(EFAULT);
	 }
  }
  else {
	 bcopy(src,dst,len);
  }
  return(rc);
}

ulong get_handle (i)
int  i;				/* force difference based on dev index */
{
	struct timestruc_t ct;
	ulong  gsc_handle;
#define  HASH  0x21351823

	curtime(&ct);

	/* swap order of bytes in nanoseconds */
	gsc_handle  = (ct.tv_nsec & 0x000000f0); /* LSB's maybe 0's */
	gsc_handle |= (         i & 0x0000000f); /* unique: rolls after 16 */

	gsc_handle |= (ct.tv_nsec & 0x0000ff00) << 16; /* move bytes around */
	gsc_handle |= (ct.tv_nsec & 0x00ff0000);
	gsc_handle |= (ct.tv_nsec & 0xff000000) >> 16;

	gsc_handle ^= HASH;			/* make it look confusing */

	return (gsc_handle);
}
