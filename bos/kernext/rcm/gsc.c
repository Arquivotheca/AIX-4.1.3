static char sccsid[] = "@(#)83	1.10.5.11  src/bos/kernext/rcm/gsc.c, rcm, bos41J, 9520A_all 5/3/95 11:44:27";

/*
 *   COMPONENT_NAME: (rcm) Rendering Context Manager Aixgsc Syscall Mgr.
 *
 *   FUNCTIONS: aixgsc
 *		command_list_service
 *		do_cmd
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989-1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>			/* includes for all lft related data */
#include <sys/device.h>	
#include <sys/sysmacros.h>		/* for MAJOR/MINOR */
#include <sys/malloc.h> 		/* memory allocation routines */
#include <sys/syspest.h>
#include <sys/uio.h>
#include "rcmras.h"			/* error logging defines */
#include "rcm_mac.h"
#include <rcmdds.h>			/* RCM dds structure */
#ifdef RCM_SIGSYS
#include <sys/signal.h>
#endif
#include "xmalloc_trace.h"

BUGVDEF(dbg_gsc, 1);
extern struct _trace_all trace_all;
extern rcm_dds *rcmdds;

/*------------
  Function Declarations
  ------------*/
static int command_list_service(struct phys_displays *, command_list *);
static int do_cmd(struct phys_displays *, int, void *,
						int, int, int, int, int);

/*------------
  Name: 	aixgsc
  Description:	Graphic System Call
  ------------*/
aixgsc(gsc_handle, cmd, arg, parm1, parm2, parm3, parm4, parm5)
ulong gsc_handle; 		/* identifies a head */
int cmd;			/* command flag */
void *arg;			/* buffer pointer, usually */
int parm1, parm2, parm3, parm4, parm5; /* usually for nonpointer inputs */
{
    struct phys_displays *pd;   /* Physical display structure pointer */
    int     i;
    int	    rc;			/* return code */
    static  int  first_time = 1;
    int     aixgsc_funnelled;	/* save incoming funnelling status */

#if RCM_FUNNEL_TEST
    if (gsc_handle == 0 && cmd == 10)	/* funnelled state test for debug */
    	return (am_i_funnelled ());

    if (gsc_handle == 0 && cmd == 11)	/* exit funnelled state for debug */
    	return (exit_funnel_nest ());

    if (gsc_handle == 0 && cmd == 10)	/* enter funnelled state for debug */
    	return (enter_funnel_nest ());
#endif

    /*
     *  Funnel ourselves, if not already funnelled.
     */
    if (__power_mp ())
	aixgsc_funnelled = enter_funnel_nest ();
    else
	aixgsc_funnelled = 1;		/* uniproc was "always" funnelled */

    if (cmd != MAKE_GP && !aixgsc_funnelled)
    {
	setuerror (EINVAL);
#ifdef  RCM_SIGSYS
	pidsig (getpid (), SIGSYS);
#endif
	exit_funnel_nest ();

	return (-1);
    }

    /*
     *  For make/unmake-gp send pointer to variable aixgsc_funnelled into
     *  the maw of the processing, so that the first cproc_init of a
     *  multihead process and the last cproc_term can read and write the
     *  value of aixgsc_funnelled.  This is so nesting of funnelling works for
     *  multihead.  The first make-gp of a multihead process should funnel
     *  the thread (if not already funnelled), and the last unmake-gp for
     *  that process should unfunnel it (provided the process was funnelled
     *  when make-gp was first called).
     *
     *  The first cproc_init of a multihead bringup will record the value
     *  of aixgsc_funnelled in the common process table, and will then set
     *  aixgsc_funnelled to the value TRUE, so that exit from the make-gp
     *  operation will not unfunnel.
     *
     *  Likewise, the final cproc_term of a multihead unmake-gp will restore
     *  the original funnelling status saved in the common processor table into
     *  our local variable aixgsc_funnelled.  This will cause restoration of the
     *  original funnelling status that existed before the first make-gp call
     *  for the multihead bringup.
     */
    if (cmd == MAKE_GP || cmd == UNMAKE_GP)
	parm1 = (int) &aixgsc_funnelled;	/* send pointer to make-gp */

    /*
     *  Facilitate diagnosis.
     *
     *  NOTE:  We don't serialize access to the trace table to avoid
     *  the overhead.  We take the risk of looking at clobbered data.
     */
    if (first_time)	/* if no graphics processes up */
    {
	/*
	 *  Clear the trace buffer the first time only.
	 */
	first_time = 0;

	bzero (&trace_all, sizeof (struct _trace_all));
    }

#ifdef  RCMDEBUG
    /*
     *  Dump out the trace.
     */
    if (gsc_handle == 0 && cmd == 0 && (unsigned int) arg < TRACE_MAX)
	dump_rcm_trace (arg);

    /*
     *  Print out current trace reports.
     */
    if (gsc_handle == 0 && cmd == 3)
	all_trace_reports ((int) arg);

    /* pass on through */
#endif

    BUGLPR(dbg_gsc, BUGNFO,
        ("\n----> Entering aixgsc(gsc_handle = 0x%x, cmd = 0x%x, arg = 0x%x)\n",
        gsc_handle,cmd,arg));

    gsctrace (AIXGSC,PTID_ENTRY);

    /*  find the display in the array of displays */
    for (i=0; i<rcmdds->number_of_displays; i++)
    {
	if (rcmdds->disp_info[i].flags & RCM_DEVNO_IN_USE &&
	    rcmdds->disp_info[i].handle == gsc_handle        )
	{
	    break;
	}
    }
    if (i >= rcmdds->number_of_displays)
    {
        BUGLPR(dbg_gsc, 0, ("The handle 0x%x is invalid\n", gsc_handle));
	setuerror (EINVAL);
#ifdef  RCM_SIGSYS
	pidsig (getpid (), SIGSYS);
#endif
	if (!aixgsc_funnelled)		/* if was not funnelled when called */
	    exit_funnel_nest ();

	return (-1);
    }

    pd = rcmdds->disp_info[i].pd;

    /* issue the gsc command */
    rc = do_cmd (pd, cmd, arg, parm1, parm2, parm3, parm4, parm5);
    /* For last unmake-gp for a thread, aixgsc_funnelled will be updated  */

    /*
     *  What we use internally as return codes from functions are defined in the
     *  traditional set of E* error codes defined in /usr/include/sys/errno.h.
     *  These are the codes that get returned in the user 'errno' variable.
     *
     *  Therefore, if our rc is nonzero at this point, rc goes into the
     *  user's 'errno' variable, and we return -1.  If our rc is zero at
     *  this point, we don't touch his 'errno' variable, and we return zero.
     *  This is how regular system calls work.
     *
     *  SPECIAL NOTE:  Some new rcm functions may return non-standard 'errno'
     *  values.  This includes but may not be limited to the new "fast" rcm
     *  interface which passes many arguments in the registers.  The new
     *  return codes will be signed >= GSC_FAST_SYSCALL_ERROR_BASE.  These new
     *  return code values are returned as the value of the system call.  The
     *  user 'errno' cell will remain untouched in this case.
     */
    if (rc && rc < GSC_FAST_SYSCALL_ERROR_BASE)
    {
	setuerror (rc);
	rc = -1;
    }

    BUGLPR(dbg_gsc, BUGNFO, ("aixgsc returned %d, errno = %d\n",
	rc, getuerror()));
    gsctrace (AIXGSC,PTID_EXIT);
#ifdef  RCM_SIGSYS
    if (rc)
	pidsig (getpid (), SIGSYS);
#endif

    /*
     *  Restore the funnelling status on entry to this aixgsc syscall.
     *
     *  Note that the aixgsc_funnelled variable can be manipulated by
     *  gsc_make_gp and gsc_unmake_gp.
     */
    if (!aixgsc_funnelled)
	exit_funnel_nest ();

    return  rc;
}


static int
command_list_service(pd, arg)
struct phys_displays *pd;
command_list *arg;
{
	int cmd;
	int count;
	int rc;
	command_list subarg;

	RCM_TRACE (0x800, getpid (), 0, 0);

	/*--------
	  Copy user argument structure to kernel space
	  --------*/
	
	rc = copyin(arg, &subarg, sizeof(command_list));
	
	if (rc != 0) {
		BUGLPR(dbg_gsc, BUGNTX,
			("copyin of subarg failed.\n"));
		return(EFAULT);
	}
	
	BUGLDM(dbg_gsc, BUGNTX, "subarg structure dump:\n",
		&subarg, sizeof(command_list));

	if (subarg.count > MAX_CMDS
	||  subarg.count <= 0) {
		BUGLPR(dbg_gsc, BUGNTX,
		("Too many or no commands (count = %d).\n", subarg.count));
		return(EINVAL);
	}

	for (count = 0; count < subarg.count; ++count) {
		int  cmd;

		cmd = subarg.cmd_list[count].command;

		/*
		 *  Certain cmds are not allowed in the command
		 *  list because all their arguments can't be
		 *  sent in by this method.
		 */
		if (cmd == GSC_RCM_REQ || cmd == GSC_DEVICE_REQ ||
		    cmd == MAKE_GP     || cmd == UNMAKE_GP         )
			return (EINVAL);

		rc = do_cmd(pd, cmd, subarg.cmd_list[count].carg,
							0, 0, 0, 0, 0);
		if (rc != 0)
			break;
	}

	subarg.error = rc;

	copyout(&subarg, arg, sizeof(command_list));
	RCM_TRACE (0x801, getpid (), rc, 0);

	BUGLPR(dbg_gsc, BUGNFO,
		("command_list_service: %d returned in subarg.error\n",
								subarg.error));
	return  rc;
}


static int
do_cmd(pd, cmd, arg, parm1, parm2, parm3, parm4, parm5)
struct phys_displays *pd;
int cmd;
void *arg;
int  parm1, parm2, parm3, parm4, parm5;
{
    int rc;

#ifdef  RCMDEBUG
    RCM_TRACE (0x810, getpid (), cmd, 0);
#endif

    switch (cmd) {

	/*------------
	  DISPLAY DEVICE HANDLER PROGRAMMING INTERFACES
	  Async Events
	  ------------*/
	case ASYNC_EVENT:
    BUGLPR(dbg_gsc, BUGNFO,
	   ("calling event support\n"));

	    rc = gsc_async_event(pd, arg);
	    break;

	/*------------
	  Get Events
	  ------------*/
	case GET_EVENTS:
    BUGLPR(dbg_gsc, BUGNFO,
	   ("calling event support\n"));
	    rc = gsc_get_events(pd, arg);
	    break;

	/*------------
	  Event Buffer
	  ------------*/
	case EVENT_BUFFER:
    BUGLPR(dbg_gsc, BUGNFO,
	   ("calling event support\n"));
	    rc = gsc_event_buffer(pd, arg);
	    break;

	/*------------
	  Wait Event
	  ------------*/
	case WAIT_EVENT:
    BUGLPR(dbg_gsc, BUGNFO,
	   ("calling event support\n"));
	    rc = gsc_wait_event(pd, arg);
	    break;

	/*------------
	  Direct Memory Access
	  ------------*/
	case DMA_SERVICE:
	    rc = dma_service(pd, arg);
	    break;

	/*------------
	  Enable Event
	  ------------*/
	case ENABLE_EVENT:
	    rc = gsc_enable_event(pd, arg);
	    break;

	/*------------
	  Device Dependent Functions
	  ------------*/
	case DEV_DEP_FUN:
	    rc = dev_dep_fun_service(pd, arg);
	    break;

	/*------------
	  Extended (fast) device dependent interface.
	  ------------*/
	case GSC_RCM_REQ:
	    rc = rcm_fun_service_fast (pd, arg,
					parm1, parm2, parm3, parm4, parm5);
	    break;

	/*------------
	  Extended (fast) device dependent interface.
	  ------------*/
	case GSC_DEVICE_REQ:
	    rc = dev_dep_fun_service_fast (pd, arg,
					parm1, parm2, parm3, parm4, parm5);
	    break;

	/*------------
	  RENDERING CONTEXT MANAGER PROGRAMMING INTERFACES
	  Make Graphics Process
	  ------------*/
	case MAKE_GP:
	    rc = gsc_make_gp(pd, arg, parm1);	/* parm1 = *prev funl status */
	    break;

	case UNMAKE_GP:
	    rc = gsc_unmake_gp(pd, arg, parm1);	/* parm1 = *prev funl status */
	    break;

	case SET_GP_PRIORITY:
	    rc = gsc_set_gp_priority(pd, arg);
	    break;

	case CREATE_RCX:
	    rc = gsc_create_rcx(pd, arg);
	    break;

	case DELETE_RCX:
	    rc = gsc_delete_rcx(pd, arg);
	    break;

	case CREATE_RCXP:
	    rc = gsc_create_rcxp(pd, arg);
	    break;

	case DELETE_RCXP:
	    rc = gsc_delete_rcxp(pd, arg);
	    break;

	case ASSOCIATE_RCXP:
	    rc = gsc_associate_rcxp(pd, arg);
	    break;

	case DISASSOCIATE_RCXP:
	    rc = gsc_disassociate_rcxp(pd, arg);
	    break;

	case CREATE_WIN_GEOM:
	    rc = gsc_create_win_geom(pd, arg);
	    break;

	case DELETE_WIN_GEOM:
	    rc = gsc_delete_win_geom(pd, arg);
	    break;

	case UPDATE_WIN_GEOM:
	    rc = gsc_update_win_geom(pd, arg);
	    break;

	case CREATE_WIN_ATTR:
	    rc = gsc_create_win_attr(pd, arg);
	    break;

	case DELETE_WIN_ATTR:
	    rc = gsc_delete_win_attr(pd, arg);
	    break;

	case UPDATE_WIN_ATTR:
	    rc = gsc_update_win_attr(pd, arg);
	    break;

	case BIND_WINDOW:
	    rc = gsc_bind_window(pd, arg);
	    break;

	case SET_RCX:
	    rc = gsc_set_rcx(pd, arg);
	    break;

	case LOCK_HW:
	    rc = gsc_lock_hw(pd, arg);
	    break;

	case UNLOCK_HW:
	    rc = gsc_unlock_hw(pd, arg);
	    break;

	case LOCK_DOMAIN:
	    rc = gsc_lock_domain(pd, arg);
	    break;

	case UNLOCK_DOMAIN:
	    rc = gsc_unlock_domain(pd, arg);
	    break;

	case GIVE_UP_TIMESLICE:
	    rc = gsc_give_up_timeslice(pd, arg);
	    break;

	case COMMAND_LIST:
    BUGLPR(dbg_gsc, BUGNFO,
	   ("calling command_list_service\n"));
	    rc = command_list_service(pd, arg);
	    break;

	case CREATE_COLORMAP:
	    rc = gsc_create_colormap(pd, arg);
	    break;

	case DELETE_COLORMAP:
	    rc = gsc_delete_colormap(pd, arg);
	    break;

	case UPDATE_COLORMAP:
	    rc = gsc_update_colormap(pd, arg);
	    break;

	case DISP_PM:     /* Display Power Management */
           rc = gsc_dpm(pd, arg, parm1);
           break;

	/*------------
	  Default.
	  ------------*/
	default:
	    rc = EINVAL;
    } /* end switch(cmd) */

    RCM_ASSERT (i_disable(INTBASE) == INTBASE, 0, 0, 0, 0, 0);

    BUGLPR(dbg_gsc, BUGNFO, ("do_cmd returned %d.\n", rc));

    return  rc;
}
