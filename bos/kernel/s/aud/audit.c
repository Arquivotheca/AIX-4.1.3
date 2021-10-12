static char sccsid[] = "@(#)84        1.21.1.2  src/bos/kernel/s/aud/audit.c, syssaud, bos412, 9445C412a 11/9/94 04:39:48";

/*
 * COMPONENT_NAME: (SYSAUDIT) Auditing Management
 *
 * FUNCTIONS: audit() system call
 *
 * ORIGINS: 27 83
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
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include	<sys/types.h>
#include	<sys/user.h>
#include	<sys/errno.h>
#include	<sys/auditk.h>
#include	<sys/audit.h>
#include	<sys/priv.h>
#include	<sys/lockl.h>


/*
 * Forward Declarations
 */



static int ABinReset();
static int audit_shutdown_in_progress = FALSE;

/*                                                                              
 * NAME: audit()
 *                                                                             
 * FUNCTION:                                                                   
 *	This system call is used to start and stop auditing.
 *	It can also be used to query audit status. Need to
 *	have AUDIT_CONFIG privilege to execute this system
 *	call
 *
 * INPUTS : 
 *	Cmd : specifies the operation. Valid values specified in
 *            sys/audit.h header file. The valid values are AUDIT_ON
 *	      AUDIT_OFF AUDIT_QUERY and AUDIT_RESET.
 *	Arg : specifies command specific information. Valid values are
 *	      AUDIT_PANIC and AUDIT_NO_PANIC
 *
 * OUTPUTS : None
 *
 * ERRORS:                                                                     
 *	EPERM when the privilege is not AUDIT_CONFIG
 *	EINVAL when the Cmd argument doen't match valid values.
 *
 * TYPE: int 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures.
 *	audit_flag : When the Cmd = AUDIT_QUERY this value is read
 *		     and returned. For other values of Cmd this value
 *		     is set accordingly i.e the values set could be
 *		     AUDIT_ON ORed with the Arg parameter or AUDIT_OFF.
 *	audit_panic: This value is set to either AUDIT_PANIC or 
 *		     AUDIT_NO_PANIC depending on the value of Arg. 
 *                                                                             
 * RETURNS: For a command value of AUDIT_QUERY, the audit system call
 *	    returns after succesful completion, the current on/off status
 *	    of the auditing system (AUDIT_ON for enabled and AUDIT_OFF 
 *	    for disabled). For any other cmd value, the audit system call
 *	    returns 0 on successful completion. If the audit system call
 *          fails, a value of -1 is returned and errno is set to indicate
 *	    the error.
 *
 */                                                                            
int
audit(int Cmd, int Arg){
	int	rc = 0;

	/* 
	 * Privilege check 
	 */

	if(privcheck(AUDIT_CONFIG)){

		u.u_error = EPERM;
		return(-1);

	}

	switch(Cmd){

		/* 
		 * Return value of audit_flag 
		 */

		case AUDIT_QUERY:
			/* 
			 * No need to lock this because the audit status
			 * can change before user receives it
			 */
			rc = audit_flag;
			break;
		
		/* 
		 * Enable auditing 
		 */

		case AUDIT_ON:

		        simple_lock(&audit_lock); 
			if (audit_shutdown_in_progress == TRUE) {
				u.u_error = EAGAIN;
				simple_unlock(&audit_lock);
				break;
			}

			/*
			 * Add all registered events
			 * to the all class if
			 * we're coming from an "audit start"
			 */

			AuditOn();

			/*
			 * Now go through proc table
			 * and update all process's p_auditmask
			 * members to current Registered
			 * values
			 */

			AuditProcs();

			/* 
			 * Check for audit_panic 
			 */

			if(Arg == AUDIT_PANIC){

				audit_panic = AUDIT_PANIC;
				audit_flag |= (AUDIT_ON | AUDIT_PANIC);

			}
			else {

				audit_panic = AUDIT_NO_PANIC;
				audit_flag = AUDIT_ON;

			}

		        simple_unlock(&audit_lock); 
			break;

		/* 
		 * Disable auditing.  
		 */

		case AUDIT_OFF:

			audit_svc();

		        simple_lock(&audit_lock); 

			audit_flag = AUDIT_OFF;
			audit_panic = AUDIT_NO_PANIC;

		        simple_unlock(&audit_lock); 
			break;

		/* 
		 * Shut down the auditing system:
		 * 1)  set auditing flag
		 * 2)  reset the bins
		 * 3)  reset the audit device
		 * 4)  clear out the event information 
		 * 5)  clear out the object information 
		 */

		case AUDIT_RESET:

			audit_svc();

		        simple_lock(&audit_lock); 
			if (audit_shutdown_in_progress == TRUE) {
				u.u_error = EAGAIN;
				simple_unlock(&audit_lock);
				break;
			}
			audit_shutdown_in_progress = TRUE;

			audit_flag = AUDIT_OFF;
			audit_panic = AUDIT_NO_PANIC;
	
			/* 
			 * Bins
			 */

			ABinReset();
		        simple_unlock(&audit_lock); 

			/* 
			 * Objects
			 */

			auditobj(AUDIT_SET, (struct o_event *)NULL, 0);

			/*
			 * Device driver
			 */
			if(auditdev)(*auditdev)(0, 0, 0);

		        simple_lock(&audit_lock); 
			/*
			 * Class information
			 */

			bzero((char *)class_names, sizeof(class_names));

			AuditReset();

			audit_shutdown_in_progress = FALSE;
		        simple_unlock(&audit_lock); 

			break;
			
		default:
			u.u_error = EINVAL;
	}

	if (u.u_error)
		return (-1);
	else
		return (rc);
}

/*                                                                              
 * NAME: ABinReset()
 *                                                                             
 * FUNCTION: Closes the current and next bins before resetting
 *	     them.
 *
 * PARAMETERS : None
 *
 * ERRORS: None
 *
 * TYPE: int
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures.
 *	audit_bin : This value is made NULL after closing the file.
 *	audit_next : This value is made NULL after closing the file.
 *	audit_anchor.error is made 0.
 *
 * RETURNS: Nothing is returned explicitly
 *
 */                                                                            
static
int
ABinReset(){
	if(audit_bin){

		AuditTail(audit_bin);
		audit_bin = NULL;

	}

	if(audit_next){

		fp_close(audit_next);
		audit_next = NULL;

	}
 
	audit_anchor.error = 0;
	e_wakeup(&audit_anchor.lock);
}
