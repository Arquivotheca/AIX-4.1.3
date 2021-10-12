static char sccsid[] = "@(#)17  1.4  src/bos/kernel/ios/pm_kernel.c, sysios, bos41J, 9517A_all 4/24/95 08:29:27";
/*
 *   COMPONENT_NAME: SYSIOS
 *
 *   FUNCTIONS: Power Management Kernel Code
 *              pm_register_handle, register_handle, unregister_handle,
 *              pm_register_planar_control_handle,
 *              register_planar_control_handle,
 *              unregister_planar_control_handle,
 *              pm_planar_control
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef PM_SUPPORT
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/intr.h>
#include <sys/lock_def.h>
#include <sys/pm.h>

struct _pm_kernel_data pm_kernel_data
		 = {{0}, NULL, NULL, NULL, NULL, NULL, NULL, NULL, {0}};

static int register_handle( struct pm_handle *);
static int unregister_handle( struct pm_handle *);
static int register_planar_control_handle( struct pm_planar_control_handle *);
static int unregister_planar_control_handle( struct pm_planar_control_handle *);

/*
 * NAME:  pm_register_handle
 *
 * FUNCTION:  Registers/unregisters a PM handle to the PM core.
 *            This subroutine needs to be called from config
 *            entry point of each PM aware DD.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from a process or an interrupt handler.
 *      It cannot page fault.
 *
 * NOTES:
 *
 * RETURN:
 *      PM_SUCCESS     Successfully completed.
 *      PM_ERROR       An error occurred.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      disable_lock/unlock_enable routines
 */
int
pm_register_handle(struct pm_handle *pmh, int cmd)
{
    if( pmh == NULL ) {
	return PM_ERROR;
    }

    switch( cmd ) {
      case PM_REGISTER:
	return register_handle(pmh);

      case PM_UNREGISTER:
	return unregister_handle(pmh);

      default:
	return PM_ERROR;
    }
}


/*
 * NAME:  register_handle
 *
 * FUNCTION:  Registers a PM handle to the PM core.
 *            This subroutine is called from pm_register_handle().
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from a process or an interrupt handler.
 *      It cannot page fault.
 *
 * NOTES:
 *
 * RETURN:
 *      PM_SUCCESS     Successfully completed.
 *      PM_ERROR       An error occurred.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      disable_lock/unlock_enable routines
 */
static int
register_handle(struct pm_handle *pmh)
{
    struct pm_handle	*p;
    int			opri;

    /* serialize access to pm_kernel_data */
    opri = disable_lock(INTMAX, &(pm_kernel_data.lock));

    p = pm_kernel_data.handle_head;

    /* Check if not yet registered */
    while( p != NULL ) {
	if( p == pmh ){
	    break;
	}
	p = p->next1;
    }

    if( p == NULL ) {
	pmh->next1 = pm_kernel_data.handle_head;
	pm_kernel_data.handle_head = pmh;
    } else {
	unlock_enable(opri, &(pm_kernel_data.lock));
	return PM_ERROR;
    }

    if( pmh->attribute & PM_GRAPHICAL_INPUT ) {
	pmh->next2 = pm_kernel_data.graphical_input_handle_head;
	pm_kernel_data.graphical_input_handle_head = pmh;
	pmh->idle_counter = 0;

    } else if( pmh->attribute & PM_GRAPHICAL_OUTPUT ) {
	pmh->next2 = pm_kernel_data.graphical_output_handle_head;
	pm_kernel_data.graphical_output_handle_head = pmh;
	pmh->idle_counter = 0;

    } else {		/* default is PM_OTHER_DEVICE */
	pmh->next2 = pm_kernel_data.other_device_handle_head;
	pm_kernel_data.other_device_handle_head = pmh;
	pmh->idle_counter = pmh->device_idle_time;
    }

    unlock_enable(opri, &(pm_kernel_data.lock));
    return PM_SUCCESS;
}


/*
 * NAME:  unregister_handle
 *
 * FUNCTION:  Unregisters a PM handle from the PM core.
 *            This subroutine is called from pm_register_handle().
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from a process or an interrupt handler.
 *      It cannot page fault.
 *
 * NOTES:
 *
 * RETURN:
 *      PM_SUCCESS     Successfully completed.
 *      PM_ERROR       An error occurred.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      disable_lock/unlock_enable routines
 */
static int
unregister_handle(struct pm_handle *pmh)
{
    struct pm_handle	**p;
    int	    		find = 0;
    int			opri;

    /* serialize access to pm_kernel_data */
    opri = disable_lock(INTMAX, &(pm_kernel_data.lock));

    p = &(pm_kernel_data.handle_head);
    while( *p != NULL ) {
	if ( (*p) == pmh ) {
	    *p = (*p)->next1;
	    find = 1;
	}
	p = &((*p)->next1);
    }

    p = &(pm_kernel_data.graphical_input_handle_head);
    while( *p != NULL ) {
	if ( (*p) == pmh ) {
	    *p = (*p)->next2;
	}
	p = &((*p)->next2);
    }

    p = &(pm_kernel_data.graphical_output_handle_head);
    while( *p != NULL ) {
	if ( (*p) == pmh ) {
	    *p = (*p)->next2;
	}
	p = &((*p)->next2);
    }

    p = &(pm_kernel_data.other_device_handle_head);
    while( *p != NULL ) {
	if ( (*p) == pmh ) {
	    *p = (*p)->next2;
	}
	p = &((*p)->next2);
    }

    unlock_enable(opri, &(pm_kernel_data.lock));

    if (find) {
	return PM_SUCCESS;
    } else {
	return PM_ERROR;
    }
}



/*
 * NAME:  pm_register_planar_control_handle
 *
 * FUNCTION:  Registers/unregisters a PM planar control handle to the PM core.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from a process or an interrupt handler.
 *      It cannot page fault.
 *
 * NOTES:
 *
 * RETURN:
 *      PM_SUCCESS     Successfully completed.
 *      PM_ERROR       An error occurred.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      disable_lock/unlock_enable routines
 */
int
pm_register_planar_control_handle(struct pm_planar_control_handle *ppch,
						int cmd)
{
    if( ppch == NULL ) {
	return PM_ERROR;
    }

    switch( cmd ) {
      case PM_REGISTER:
	return register_planar_control_handle(ppch);

      /* register with pmdev major number */
      case PM_REGISTER | PMDEV_MAJOR_NUMBER:
	/* the pmdev must have only pmdev major number */
	if( ppch->devid & PMDEV_MINOR_MASK ) {
	    return PM_ERROR;
	}
	ppch->devid |= PMDEV_MAJOR_NUMBER;
	return register_planar_control_handle(ppch);

      case PM_UNREGISTER:
	return unregister_planar_control_handle(ppch);

      default:
	return PM_ERROR;
    }
}



/*
 * NAME:  register_planar_control_handle
 *
 * FUNCTION:  Registers a PM planar control handle to the PM core.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from a process or an interrupt handler.
 *      It cannot page fault.
 *
 * NOTES:
 *
 * RETURN:
 *      PM_SUCCESS     Successfully completed.
 *      PM_ERROR       An error occurred.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      disable_lock/unlock_enable routines
 */
static int
register_planar_control_handle(struct pm_planar_control_handle *ppch)
{
    struct pm_planar_control_handle	*p;
    int					opri;

    /* serialize access to pm_kernel_data */
    opri = disable_lock(INTMAX, &(pm_kernel_data.planar_lock));

    p = pm_kernel_data.planar_control_handle_head;

    /* Check not yet registered */
    while( p != NULL ) {
	if( p->devid == ppch->devid ){
	    break;
	}
	p = p->next;
    }

    if( p == NULL ) {
	ppch->next = pm_kernel_data.planar_control_handle_head;
	pm_kernel_data.planar_control_handle_head = ppch;
	unlock_enable(opri, &(pm_kernel_data.planar_lock));
	return PM_SUCCESS;
    } else {
	unlock_enable(opri, &(pm_kernel_data.planar_lock));
	return PM_ERROR;
    }
}


/*
 * NAME:  unregister_planar_control_handle
 *
 * FUNCTION:  Unregisters a PM planar control handle from the PM core.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from a process or an interrupt handler.
 *      It cannot page fault.
 *
 * NOTES:
 *
 * RETURN:
 *      PM_SUCCESS     Successfully completed.
 *      PM_ERROR       An error occurred.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      disable_lock/unlock_enable routines
 */
static int
unregister_planar_control_handle(struct pm_planar_control_handle *ppch)
{
    struct pm_planar_control_handle	**p;
    int				 	find = 0;
    int					opri;

    /* serialize access to pm_kernel_data */
    opri = disable_lock(INTMAX, &(pm_kernel_data.planar_lock));

    p = &(pm_kernel_data.planar_control_handle_head);
    while( *p != NULL ) {
	if ( (*p)->devid == ppch->devid ) {
	    *p = (*p)->next;
	    find = 1;
	}
	p = &((*p)->next);
    }

    unlock_enable(opri, &(pm_kernel_data.planar_lock));

    if (find) {
	return PM_SUCCESS;
    } else {
	return PM_ERROR;
    }
}


/*
 * NAME:  pm_planar_control
 *
 * FUNCTION:  Executes a PM planar control subroutine.
 *            
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from a process or an interrupt handler.
 *      It cannot page fault.
 *
 * NOTES:
 *
 * RETURN:
 *      PM_SUCCESS     Successfully completed.
 *      PM_ERROR       An error occurred.
 *
 * EXTERNAL PROCEDURES CALLED:
 *      disable_lock/unlock_enable routines
 */
int
pm_planar_control(dev_t devno, int devid, int cmd)
{
    struct pm_planar_control_handle	*p;
    int					rc;
    int					(*control)();
    int					opri;

    /* serialize access to pm_kernel_data */
    simple_lock(&(pm_kernel_data.planar_lock));

    p = pm_kernel_data.planar_control_handle_head;
    while( p != NULL ) {
	if( (p->devid == devid) ||		/* devid is equal */
	    ((p->devid & PMDEV_MAJOR_NUMBER) &&	/* major number part is equal */
	     ((p->devid & PMDEV_MAJOR_MASK) == (devid & PMDEV_MAJOR_MASK)))) {
	    control = p->control;
	    if( control != NULL ) {
		rc = (*control)(devno, devid, cmd);
	    } else {
		rc = PM_ERROR;
	    }
    	    simple_unlock(&(pm_kernel_data.planar_lock));
	    return rc;
	}
	p = p->next;
    }

    switch(cmd) {
    case PM_PLANAR_QUERY:
	rc = 0;			/* no pm_planar_control feature is supported. */
	break;
    case PM_PLANAR_CURRENT_LEVEL:
	rc = PM_PLANAR_ON;	/* always normal state due to no pm_planar_control*/
	break;
    default:
	rc = PM_SUCCESS;	/* never error in case of no pm_planar_control */
	break;
    } /* endswitch */	

    simple_unlock(&(pm_kernel_data.planar_lock));
    return rc;
}
#endif /* PM_SUPPORT */
