static char sccsid[] = "@(#)42	1.11.1.7  src/bos/kernel/ios/sysconfig.c, sysios, bos41J, 9513A_all 3/28/95 16:44:53";
/*
 * COMPONENT_NAME: (SYSIOS) sysconfig system call
 *
 * FUNCTIONS:	load_kmod,	unload_kmod,	config_kmod,
 *		config_dd,	query_devsw,	getparms,
 *		setparms,	sysconfig,	cfgnadd,
 *		cfgndel
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
                                                 
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/var.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/ldr.h>
#include <sys/intr.h>
#include <sys/priv.h>
#include <sys/sysmacros.h>
#include <sys/user.h>
#ifdef _POWER_MP
#include <sys/m_types.h>
#include <sys/processor.h>
#include <sys/ppda.h>
#endif

struct cfgncb	*cfgncb = NULL;	/* head of config notification cb chain	*/
int cfgcb_lock = LOCK_AVAIL;

extern int	copyin();
extern int	copyout();
extern int	suword();
extern int	fuword();
extern int	kmod_unload(mid_t kmid, uint flags);
extern int	kmod_load(caddr_t path, uint flags, caddr_t libpath, mid_t *kmid);
extern void	(*(kmod_entrypt)(mid_t kmid, uint flags))();

/*
 * NAME:  load_kmod
 *
 * FUNCTION:  Load a kernel extension module into kernel memory.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is only called by the sysconfig system call.
 *      It cannot page fault.
 *
 * NOTES:  This routine is used to load a module (typically a kernel
 *	extension) specified by pathname into the kernel and return
 *	a module ID for that instance of the module.  The returned
 *	module ID may then be used for subsequent calls of the
 *	module's entry point (using config_kmod) or for unloading the
 *	module (using unload_kmod).
 *
 * DATA STRUCTURES:  cfg_load
 *
 * RETURN VALUE DESCRIPTION:	CONF_FAIL, upon error completion, and
 * 				ERRNO set to EFAULT or the return value
 *				from kmod_load(); or
 * 				CONF_SUCC, upon successful completion, and
 * 				the value of cfg_load->kmid.
 *
 * EXTERNAL PROCEDURES CALLED:  copyin,
 * 				kmod_load,
 * 				suword,
 * 				kmod_unload
 */
int
load_kmod(register struct cfg_load *cfg_load, register int cfg_load_len, register int kcmd)

/* register struct cfg_load	*cfg_load;	*/
/* register int			cfg_load_len;	*/
/* register int			kcmd		*/
{
	register int		rc;
	register uint		flags;	/* load flags			*/
	struct cfg_load		kcfg_load;

	/*
	 * Copy data from user memory to kernel memory.
	 */
	rc = copyin(cfg_load, &kcfg_load, sizeof(struct cfg_load));
	if (rc != 0)
	{
		u.u_error = EFAULT;
		return(CONF_FAIL);
	}

	/*
	 * Load the module into the kernel.
	 */
	switch (kcmd) 
	{

	case SYS_QUERYLOAD:
	    flags = (LD_QUERY);
	    break;

	case SYS_KLOAD:
	    flags = (LD_KERNELEX | LD_USRPATH);
	    break;

	case SYS_SINGLELOAD:
	    flags = (LD_SINGLELOAD | LD_KERNELEX | LD_USRPATH);
	    break;

	default:
	    u.u_error = EINVAL;
	    return(CONF_FAIL);
	    break;

	}    
		
	rc = kmod_load(kcfg_load.path, flags, kcfg_load.libpath, &(kcfg_load.kmid));
	if (rc != 0)
	{
		u.u_error = rc;
		return(CONF_FAIL);
	}

	/*
	 * Copy kmid from kernel memory to user memory.
	 */
	rc = suword(&(cfg_load->kmid), kcfg_load.kmid);
	if (rc != 0)
	{
		flags = 0;
		(void)kmod_unload(kcfg_load.kmid, flags);
		u.u_error = EFAULT;
		return(CONF_FAIL);
	}

	return(CONF_SUCC);

}  /* end load_kmod */

/*
 * NAME:  unload_kmod
 *
 * FUNCTION:  Unload a previously-loaded kernel extension module from
 *	kernel memory.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is only called by the sysconfig system call.
 *      It cannot page fault.
 *
 * NOTES:  This routine is used to unload a previously-loaded kernel
 *	module, using the module ID that was returned when the
 *	module was loaded with the load_kmod routine.
 * 	Care must be taken to ensure that a module to be unloaded is no
 *	longer in use by the system and has freed all of its system
 *	resources before being unloaded.  For example, a device driver
 *	is typically prepared for unloading by using the driver's
 *	ddconfig routine to call the device driver's module entry point
 *	with an operation code specifying termination.
 *
 * DATA STRUCTURES:  cfg_load
 *
 * RETURN VALUE DESCRIPTION:	CONF_FAIL, upon error completion, and
 * 				ERRNO set to EFAULT or the return value
 *				from kmod_load(); or
 * 				CONF_SUCC, upon successful completion.
 *
 * EXTERNAL PROCEDURES CALLED:  fuword,
 *				kmod_unload
 */
int
unload_kmod(register struct cfg_load *cfg_load, register int cfg_load_len)

/* register struct cfg_load	*cfg_load;	*/
/* register int			cfg_load_len;	*/
{
	register int		rc;
	register uint		flags;	/* unload flags			*/
	mid_t			kmid;
	struct cfg_load		kcfg_load;

	/*
	 * Copy data from user memory to kernel memory.
	 */
	kmid = (mid_t)fuword(&(cfg_load->kmid));
	if (kmid == (mid_t)-1)
	{
		u.u_error = EFAULT;
		return(CONF_FAIL);
	}

	/*
	 * Unload the module from the kernel.
	 */
	flags = 0;
	rc = kmod_unload(kmid, flags);
	if (rc != 0)
	{
		u.u_error = rc;
		return(CONF_FAIL);
	}

	return(CONF_SUCC);

}  /* end unload_kmod */

/*
 * NAME:  config_kmod
 *
 * FUNCTION:  Call the specified module at its module entry point.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is only called by the sysconfig system call.
 *      It cannot page fault.
 *
 * NOTES:  This routine is used to call a previously loaded kernel
 *	module at its module entry point, typically for initialization
 *	or termination functions.  If the module's entry point is called,
 *	it is the called module's responsibility to set an integer-type
 *	return code prior to returning to the config system call to indicate
 *	success or failure of the operation requested.
 *
 * DATA STRUCTURES:  cfg_kmod
 *
 * RETURN VALUE DESCRIPTION:	CONF_FAIL, upon error completion, and
 * 				ERRNO set to EFAULT, EINVAL, or the return
 *				value from the kernel module's entry point; or
 * 				CONF_SUCC, upon successful completion.
 *
 * EXTERNAL PROCEDURES CALLED:  copyin,
 * 				kmod_entrypt,
 * 				kernel module's entry point
 */
int
config_kmod(register struct cfg_kmod *cfg_kmod, register int cfg_kmod_len)

/* register struct cfg_kmod	*cfg_kmod;	*/
/* register int			cfg_kmod_len;	*/
{
	register int		rc;
	register uint		flags;
	struct iovec		uio_iov;
	struct uio		uio;
	struct cfg_kmod		kcfg_kmod;
	int			(*func_ptr)();

	/*
	 * Copy data from user memory to kernel memory.
	 */
	rc = copyin(cfg_kmod, &kcfg_kmod, sizeof(struct cfg_kmod));
	if (rc != 0)
	{
		u.u_error = EFAULT;
		return(CONF_FAIL);
	}

	/*
	 * Get the module's entry point.
	 */
	flags = 0;
	func_ptr = (int(*)())kmod_entrypt(kcfg_kmod.kmid, flags);
	if (func_ptr == NULL)
	{
		u.u_error = EINVAL;
		return(CONF_FAIL);
	}

	if (kcfg_kmod.mdiptr == (caddr_t)NULL)
	{
		/*
	 	 * Call the module's entry point with a cmd
		 * and a NULL pointer.
		 */
		rc = (*func_ptr)(kcfg_kmod.cmd, (struct uio *)NULL);
	}
	else
	{
		uio_iov.iov_base = kcfg_kmod.mdiptr;
		uio_iov.iov_len = kcfg_kmod.mdilen;
		uio.uio_iov = &uio_iov;
		uio.uio_iovcnt = 1;
		uio.uio_offset = (off_t)0;
		uio.uio_segflg = UIO_USERSPACE;
		uio.uio_fmode = 0;
		uio.uio_resid = kcfg_kmod.mdilen;
		/*
	 	 * Call the module's entry point with a cmd
		 * and a uio struct pointer as parameters.
		 */
		rc = (*func_ptr)(kcfg_kmod.cmd, &uio);
	}
	if (rc != 0)
	{
		u.u_error = rc;
		return(CONF_FAIL);
	}

	return(CONF_SUCC);
	
}  /* end config_kmod */

/*
 * NAME:  config_dd
 *
 * FUNCTION:  Call the specified device driver configuration routine (ddconfig)
 *	(module entry point).
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is only called by the sysconfig system call.
 *      It cannot page fault.
 *
 * NOTES:  This routine is used to call a previously loaded device driver
 *	at its config (module) entry point.  This service is typically
 *	used to initialize or terminate a device driver, or to request
 *	device vital product data.  The config system call puts no
 *	restrictions on the operation code passed to the device driver
 *	to allow for the possibility of additional services being provided
 *	by the device driver's ddconfig entry point.  If the device driver's
 *	config entry point is successfully called, it is the device driver's
 *	responsibility to set an integer-type return code prior to returning
 *	to the config system call to indicate success or failure of the
 *	operation requested.
 *
 * DATA STRUCTURES:  cfg_dd
 *
 * RETURN VALUE DESCRIPTION:	CONF_FAIL, upon error completion, and
 * 				ERRNO set to EFAULT, ENODEV, EINVAL, or the
 *				return value from the device driver's ddconfig
 *				routine; or
 * 				CONF_SUCC, upon successful completion.
 *
 * EXTERNAL PROCEDURES CALLED:  copyin,
 * 				devswqry,
 * 				kmod_entrypt,
 *				device driver's ddconfig routine.
 */
int
config_dd(register struct cfg_dd *cfg_dd, register int cfg_dd_len)

/* register struct cfg_dd	*cfg_dd;	*/
/* register int		cfg_dd_len;		*/
{
	register int		rc;
	register uint		flags;
	struct iovec		uio_iov;
	struct uio		uio;
	struct cfg_dd		kcfg_dd;
	uint			status;
	int			(*func_ptr)();
#ifdef _POWER_MP
	int			wasfunneled;
#endif

	/*
	 * Copy data from user memory to kernel memory.
	 */
	rc = copyin(cfg_dd, &kcfg_dd, sizeof(struct cfg_dd));
	if (rc != 0)
	{
		u.u_error = EFAULT;
		return(CONF_FAIL);
	}

	if (kcfg_dd.kmid == 0)
	{
		/*
		 * Check the status of the device switch table
		 * entry for this device number.
		 */
		rc = devswqry(kcfg_dd.devno, &status, NULL);
		if ((rc != CONF_SUCC) || (status == DSW_UNDEFINED))
		{
			u.u_error = ENODEV;
			return(CONF_FAIL);
		}

		uio_iov.iov_base = kcfg_dd.ddsptr;
		uio_iov.iov_len = kcfg_dd.ddslen;
		uio.uio_iov = &uio_iov;
		uio.uio_iovcnt = 1;
		uio.uio_offset = (off_t)0;
		uio.uio_segflg = UIO_USERSPACE;
		uio.uio_fmode = 0;
		uio.uio_resid = kcfg_dd.ddslen;
		/*
	 	 * Call the device driver's config entry point
		 * directly from the device switch table.
		 */
#ifndef _POWER_MP
		rc = ((*devsw[major(kcfg_dd.devno)].d_config)
		      (kcfg_dd.devno, kcfg_dd.cmd, &uio));
#else
		if (devsw[major (kcfg_dd.devno)].d_opts & DEV_MPSAFE) {
			rc = ((*devsw[major(kcfg_dd.devno)].d_config)
			      (kcfg_dd.devno, kcfg_dd.cmd, &uio));
		} else {
			wasfunneled =
			  switch_cpu(MP_MASTER, SET_PROCESSOR_ID);

			rc = ((*devsw[major(kcfg_dd.devno)].d_config)
			      (kcfg_dd.devno, kcfg_dd.cmd, &uio));

			if (!wasfunneled)
			  switch_cpu(0, RESET_PROCESSOR_ID);
		}
#endif /* !_POWER_MP */
		if (rc != 0)
		{
			u.u_error = rc;
			return(CONF_FAIL);
		}

		return(CONF_SUCC);
	}
	else
	{
		/*
	 	 * Get the module's entry point.
	 	 */
		flags = 0;
		func_ptr = (int(*)())kmod_entrypt(kcfg_dd.kmid, flags);
		if (func_ptr == NULL)
		{
			u.u_error = EINVAL;
			return(CONF_FAIL);
		}

		uio_iov.iov_base = kcfg_dd.ddsptr;
		uio_iov.iov_len = kcfg_dd.ddslen;
		uio.uio_iov = &uio_iov;
		uio.uio_iovcnt = 1;
		uio.uio_offset = (off_t)0;
		uio.uio_segflg = UIO_USERSPACE;
		uio.uio_fmode = 0;
		uio.uio_resid = kcfg_dd.ddslen;
		/*
	 	 * Call the device driver's config entry point
		 * with a uio struct pointer as a parameter.
		 */
#ifndef _POWER_MP
		rc = (*func_ptr)(kcfg_dd.devno, kcfg_dd.cmd, &uio);
#else
		wasfunneled =
		  switch_cpu(MP_MASTER, SET_PROCESSOR_ID);

		rc = (*func_ptr)(kcfg_dd.devno, kcfg_dd.cmd, &uio);

		if (!wasfunneled)
		  switch_cpu(0, RESET_PROCESSOR_ID);
#endif /* !_POWER_MP */
		if (rc != 0)
		{
			u.u_error = rc;
			return(CONF_FAIL);
		}

		return(CONF_SUCC);
	}

}  /* end config_dd */

/*
 * NAME:  query_devsw
 *
 * FUNCTION:  Query the device switch table and return the status of
 *	the requested device:  is the device in use?  has the device
 *	been opened?  is the device in use by a character device driver?
 *	by a block device driver? etc.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is only called by the sysconfig system call.
 *      It cannot page fault.
 *
 * NOTES:  This routine is used to query the device switch table entry
 *	for a given device.
 *
 * DATA STRUCTURES:  qry_devsw
 *
 * RETURN VALUE DESCRIPTION:	CONF_FAIL, upon error completion, and
 * 				ERRNO set to EINVAL or EFAULT; or
 * 				CONF_SUCC, upon successful completion, and
 * 				the value of qry_devsw->status, as follows:
 *
 * 				DSW_UNDEFINED if the device is not defined;
 *				DSW_DEFINED if the device is defined (i.e. in
 *				use;
 *				DSW_CREAD if the device is in use by a device
 *				driver that does char/raw reads.
 *				DSW_CWRITE if the device is in use by a device
 *				driver that does char/raw writes.
 *				DSW_BLOCK if the device is in use by a block
 *				device driver;
 *				DSW_MPX if the device is in use by a multiplexed
 *				device driver;
 *				DSW_TTY if the device is in use by a TTY device
 *				driver;
 *				DSW_SELECT if the device is in use by a device
 *				driver that provides a routine for handling the
 *				Poll/Select system calls;
 *				DSW_DUMP if the device is in use by a device
 *				driver that provides the capability to support
 *				its devices as targets for a kernel dump;
 *				DSW_TCPATH if the device is in use by a device
 *				driver that provides support for the Revoke
 *				system call;
 *				DSW_OPENED if the device is in use and has at
 *				least 1 outstanding open.
 *
 * EXTERNAL PROCEDURES CALLED:  copyin,
 *				devswqry,
 *				suword
 */
int
query_devsw(register struct qry_devsw *qry_devsw, register int qry_devsw_len)

/* register struct qry_devsw	*qry_devsw;	*/
/* register int			qry_devsw_len;		*/
{
	register int		rc;
	struct qry_devsw	kqry_devsw;

	/*
	 * Copy data from user memory to kernel memory.
	 */
	rc = copyin(qry_devsw, &kqry_devsw, sizeof(struct qry_devsw));
	if (rc != 0)
	{
		u.u_error = EFAULT;
		return(CONF_FAIL);
	}

	rc = devswqry(kqry_devsw.devno, &(kqry_devsw.status), NULL);
	if (rc != CONF_SUCC)
	{
		u.u_error = rc;			/* EINVAL		*/
		return(CONF_FAIL);
	}

	/*
	 * Copy field from kernel memory to user memory.
	 */
	rc = suword(&(qry_devsw->status), kqry_devsw.status);
	if (rc != 0)
	{
		u.u_error = EFAULT;
		return(CONF_FAIL);
	}

	return(CONF_SUCC);

}  /* end query_devsw */

/*
 * NAME:  getparms
 *
 * FUNCTION:  Get the current values of runtime system parameters and
 *	return them in a var structure to the caller.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is only called by the sysconfig system call.
 *      It cannot page fault.
 *
 * NOTES:  This routine is used to return to the caller the values of runtime
 *	system parameters in the caller-provided var structure.  This
 *	structure may be used for informational purposes or may be used to set
 *	specific system parameters by modifying the required fields in the
 *	structure and using the SYS_SETPARMS sysconfig command to change the
 *	system runtime operating parameters to the desired state.  These
 *	parameters represent only the runtime tunable parameters of the kernel.
 *
 * DATA STRUCTURES:  var
 *
 * RETURN VALUE DESCRIPTION:	CONF_FAIL, upon error completion, and
 * 				ERRNO set to EFAULT; or
 * 				CONF_SUCC, upon successful completion, and
 * 				the var structure copied to the address in
 * 				u_var.
 *
 * EXTERNAL PROCEDURES CALLED:  copyout,
 *				i_disable,
 *				i_enable
 */
int
getparms(register struct var *u_var, register int parmlen)

/* register struct var	*u_var;		addr of user's var struct	*/
/* register int		parmlen;	amt of data to return to user*/
{
	register int		rc;
	register int		size;	/* size of caller's var struct	*/
	register int		waslocked;


	/*
	 * Determine the size of the data transfer:  either the user's parmlen
	 * or the var struct's size, whichever is smaller.  Then, use
	 * this size to determine how much of the var struct to return.
	 */
	size = MIN((sizeof(struct var)), parmlen);
	if (size == 0)
	{
		return(0);
	}

	waslocked = lockl(&cfgcb_lock, LOCK_SHORT);
	/*
	 * Increment generation number for each read.
	 */
	v.var_hdr.var_gen++;
	if (waslocked != LOCK_NEST)
	{
		unlockl(&cfgcb_lock);
	}

	/*
	 * Copy values from kernel memory to user memory.
	 */
	rc = copyout(&v, u_var, size);
	if (rc != 0)
	{
		u.u_error = EFAULT;
		rc = CONF_FAIL;
	}

	return(rc);

}  /* end getparms */

/*
 * NAME:  setparms
 *
 * FUNCTION:  Set the runtime system parameters to the values provided by
 *	the caller.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is only called by the sysconfig system call.
 *      It cannot page fault.
 *
 * NOTES:  This routine is used to set the runtime system parameters from a
 *	copy of the system parameter structure provided by the caller.  The
 *	parameters in this structure represent only the runtime tunable
 *	parameters of the kernel.  If the var_vers and var_gen values in the
 *	caller-provided structure do not match their counterparts in the current
 *	system parameter structure, then no parameters will be modified and an
 *	error will be returned.
 *
 * DATA STRUCTURES:  var
 *
 * RETURN VALUE DESCRIPTION:	CONF_FAIL, upon error completion, and
 * 				ERRNO set to EFAULT, EINVAL, or EAGAIN; or
 * 				CONF_SUCC, upon successful completion, and
 * 				the var struct updated with the values in
 *				the user's var struct (u_var is the address
 * 				of this struct).
 *
 * EXTERNAL PROCEDURES CALLED:  copyin,
 *				lockl,
 *				unlockl
 */
int
setparms(register struct var *u_var, register int parmlen)

/* register struct var	*u_var;		addr of user's var struct	*/
/* register int		parmlen;	#byte of var struct to update*/
{
	register int		rc;	/* rc from copyin/notif. routine*/
	int			offset;	/* earliest invalid var		*/
	register int		i;	/* loop control variable	*/
	register int		size;	/* size of caller's var struct	*/
	struct var		k_var;	/* kernel copy of user var struct*/
	struct cfgncb		*cb;	/* ptr to config notification chain*/
	register int		waslocked;

	/* offsets of all RO vars in the struct */
	int ro_vars[] = {
	     OFFSET_VLOCK,
	     OFFSET_VFILE,
	     OFFSET_VPROC,
	     OFFSET_VCLIST,
	     OFFSET_VTHREAD,
	     OFFSET_VNCPUS,
	     OFFSET_VNCPUSCFG,
	     OFFSET_VELOCK,
	     OFFSET_VEFILE,
	     OFFSET_VEPROC,
	     OFFSET_VETHREAD,
	     OFFSET_VBPROC,
	     OFFSET_VBTHREAD,
	};

	waslocked = lockl(&cfgcb_lock, LOCK_SHORT);

	/*
	 * Caller must provide at least enough data to specify
	 * the var_hdr structure within the var structure.
	 */
	if (parmlen < sizeof(struct var_hdr))
	{
		u.u_error = EINVAL;
		if (waslocked != LOCK_NEST)
		{
			unlockl(&cfgcb_lock);
		}
		return(CONF_FAIL);
	}

	/*
	 * If caller provided only enough data to specify the
	 * var_hdr structure within the var structure, then
	 * no variables in var will be updated.
	 */
	else if (parmlen == sizeof(struct var_hdr))
	{
		if (waslocked != LOCK_NEST)
		{
			unlockl(&cfgcb_lock);
		}
		return(CONF_SUCC);
	}

	/*
	 * Determine the size of the data transfer:  either the user's parmlen
	 * or the var struct's current size, whichever is smaller.  Then, use
	 * this size to determine how much of the var struct to update.
	 * Make a copy of the current var structure in the temporary area and
	 * then overlay the callers changes.
	 */

	size = MIN((sizeof(struct var)), parmlen);
	
	bcopy ((char *)&v,(char *)&k_var, sizeof (struct var));

	/*
	 * Copy values from user memory to kernel memory.
	 */
	rc = copyin(u_var, &(k_var), size);
	if (rc != 0)
	{
		u.u_error = EFAULT;
		if (waslocked != LOCK_NEST)
		{
			unlockl(&cfgcb_lock);
		}
		return(CONF_FAIL);
	}

	/*
	 * Caller's version number does not match
	 * the current system version number.
	 */
	if (k_var.var_hdr.var_vers != v.var_hdr.var_vers)
	{
		u.u_error = EINVAL;
		if (waslocked != LOCK_NEST)
		{
			unlockl(&cfgcb_lock);
		}
		return(CONF_FAIL);
	}

	/*
	 * Caller's generation number does not match
	 * the current system generation number.
	 */
	if (k_var.var_hdr.var_gen != v.var_hdr.var_gen)
	{
		u.u_error = EAGAIN;
		if (waslocked != LOCK_NEST)
		{
			unlockl(&cfgcb_lock);
		}
		return(CONF_FAIL);
	}

	/*
	 * Caller's structure size does not match
	 * the current system structure size
	 */
	if (k_var.var_hdr.var_size != v.var_hdr.var_size)
	{
		k_var.var_hdr.var_size = v.var_hdr.var_size;
	}

	/*
	 * At this point, check to see that none of the RO vars are 
	 * being changed.  Use an array of offsets to the RO vars within 
	 * the struct.  
	 * Compare all the values of the RO vars in the k_var struct to 
	 * those in the v struct.  If none of the RO vars have been changed, 
	 * then continue, else quit with error.
	*/

	/* check if attempting to modify past the 3.2 RW boundary.
	   If so, must verify that no RO vars are being changed
	 */
	if (size > v.var_hdr.var_size) {
		/* compare all the RO vars */
		for (i = 0; i < NUM_VARS; i++) {
			if (*(int*)(((char*)&k_var) + ro_vars[i]) != 
				*(int*)(((char*)&v) + ro_vars[i])) {
				u.u_error = EINVAL;
				/* set var_vers field to value of bad parm */
				offset = ro_vars[i];	
				copyout(&offset, &(u_var->var_hdr.var_vers), sizeof(long));
				if (waslocked != LOCK_NEST) {
					unlockl(&cfgcb_lock);
				}
				return(CONF_FAIL);
			}
		}
	}


	/*
	 * Run the config notification control block chain,
	 * calling each control block's func routine with the
	 * CFGV_PREPARE command.  If any of these func routines
	 * return a non-zero value, then stop processing and
	 * give this value back to the caller.
	 */
	cb = cfgncb;
	if (cb != NULL)
	{
		offset = 0;
		do
		{
			rc = (*cb->func)(CFGV_PREPARE, &v, &k_var);
			if (rc != 0)
			{
				if (offset == 0)
				{
					offset = rc;
				}
				else
				{
					offset = MIN(offset, rc);
				}
			}
			cb = cb->cbnext;
		} while (cb != cfgncb);
		if (offset != 0)
		{
			/*
		 	 * Copy error return code (byte offset of
		 	 * invalid system parameter) into caller's
			 * var_vers field.
		 	 */
			copyout(&offset, &(u_var->var_hdr.var_vers), sizeof(long));
			u.u_error = EINVAL;
			if (waslocked != LOCK_NEST)
			{
				unlockl(&cfgcb_lock);
			}
			return(-1);
		}
	}

	/*
	 * Again run the config notification control block chain,
	 * calling each control block's func routine with the
	 * CFGV_COMMIT command.
	 */
	cb = cfgncb;
	if (cb != NULL)
	{
		do
		{
			(void)(*cb->func)(CFGV_COMMIT, &v, &k_var);
			cb = cb->cbnext;
		} while (cb != cfgncb);
	}

	bcopy((char *)&k_var, (char *)&v, size);

	if (waslocked != LOCK_NEST)
	{
		unlockl(&cfgcb_lock);
	}
	return(CONF_SUCC);

}  /* end setparms */

/*
 * NAME:  sysconfig
 *
 * FUNCTION:  Load or unload a kernel extension module, call its module
 *	entry point or its device driver ddconfig (module) entry point,
 *	check the status of a device switch entry in the device switch table,
 *	get/set runtime system parameters.
 *
 * EXECUTION ENVIRONMENT:
 *      This system call can be called by device drivers or device managers.
 *      It cannot page fault.
 *
 * NOTES:  This system call is used to call one of several routines; the
 *	'cmd' parameter to this system call tells which function is requested:
 *		SYS_KLOAD means that the load_kmod routine will be called
 *			to load a kernel extension module into the kernel;
 *		SYS_KULOAD means that the unload_kmod routine will be
 *			called to unload a previously-loaded kernel 
 *			extension module from the kernel;
 *		SYS_CFGKMOD means that the config_kmod routine will be called
 *			to call a previously-loaded kernel extension module
 *			at its module entry point;
 *		SYS_CFGDD means that the config_dd routine will be called
 *			to call a previously-loaded device driver at its
 *			ddconfig (module) entry point.
 *		SYS_QDVSW means that the status of a device entry in the
 *			device switch table will be checked.
 *		SYS_GETPARMS means that a structure containing the current
 *			values of runtime system parameters will be returned
 *			to the caller.
 *		SYS_SETPARMS means that runtime system parameters will be set
 *			from the caller-provided structure.
 *
 * DATA STRUCTURES:  none
 *
 * RETURN VALUE DESCRIPTION:	CONF_SUCC upon successful execution of
 *				whichever routine was called; see prologs
 *				of load_kmod, unload_kmod, etc. for other
 *				possible return codes; or
 *				CONF_FAIL and ERRNO set to EACCES if the
 *				caller does not have authority to execute
 *				the command that he requested.
 *
 * EXTERNAL PROCEDURES CALLED:  load_kmod,
 *				unload_kmod,
 *				config_kmod,
 *				config_dd,
 *				query_devsw,
 *				getparms,
 *				setparms
 */
int
sysconfig(register int cmd, void *parmp, register int parmlen)

/* register int	cmd;		requested function: load/unload/etc	*/
/* void		*parmp;		    addr of struct containing cmd info	*/
/* register int	parmlen;	length of parmp information		*/
{
	register int	rc;	/* return value from cmd routines	*/
	extern int	privcheck();/* check privileges			*/

	switch (cmd)
	{
		case SYS_KLOAD:
			if (privcheck(SYS_CONFIG))
			{
				u.u_error = EACCES;
				return(CONF_FAIL);
			}
			rc = load_kmod(parmp, parmlen, SYS_KLOAD);
			break;
		case SYS_SINGLELOAD:
			if (privcheck(SYS_CONFIG))
			{
				u.u_error = EACCES;
				return(CONF_FAIL);
			}
			rc = load_kmod(parmp, parmlen, SYS_SINGLELOAD);
			break;
		case SYS_QUERYLOAD:
			if (privcheck(SYS_CONFIG))
			{
				u.u_error = EACCES;
				return(CONF_FAIL);
			}
			rc = load_kmod(parmp, parmlen, SYS_QUERYLOAD);
			break;
		case SYS_KULOAD:
			if (privcheck(SYS_CONFIG))
			{
				u.u_error = EACCES;
				return(CONF_FAIL);
			}
			rc = unload_kmod(parmp, parmlen);
			break;
		case SYS_CFGKMOD:
			if (privcheck(SYS_CONFIG))
			{
				u.u_error = EACCES;
				return(CONF_FAIL);
			}
			rc = config_kmod(parmp, parmlen);
			break;
		case SYS_CFGDD:
			if (privcheck(DEV_CONFIG))
			{
				u.u_error = EACCES;
				return(CONF_FAIL);
			}
			rc = config_dd(parmp, parmlen);
			break;
		case SYS_QDVSW:
			rc = query_devsw(parmp, parmlen);
			break;
		case SYS_GETPARMS:
			rc = getparms(parmp, parmlen);
			break;
		case SYS_SETPARMS:
			if (privcheck(SYS_CONFIG))
			{
				u.u_error = EACCES;
				return(CONF_FAIL);
			}
			rc = setparms(parmp, parmlen);
			break;
		default:
			rc = CONF_FAIL;
			u.u_error = EINVAL;
			break;
	}  /* end switch */  

	return(rc);

}  /* end sysconfig */

/*
 * NAME:  cfgnadd
 *
 * FUNCTION:  Register a system configurable variable "change" notification
 *	routine for use by the sysconfig system call.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can only be called by a routine executing in a process
 *	environment.  It cannot page fault.
 *
 * NOTES:  This routine is used to add the caller-provided cfgncb
 *	structure to the list of routines to be called when a
 *	configurable variable is being changed by the sysconfig
 *	SYS_SETPARMS system call.
 *
 * DATA STRUCTURES:  cfgncb
 *
 * RETURN VALUE DESCRIPTION:	none.
 *
 * EXTERNAL PROCEDURES CALLED:  lockl,
 *				unlockl
 */
void
cfgnadd(register struct cfgncb *cbp)

/* register struct cfgncb		*cbp;	pointer to config ctl blk	*/
{
	register int		waslocked;
	extern struct cfgncb	*cfgncb;

	assert(cbp != NULL);

	waslocked = lockl(&cfgcb_lock, LOCK_SHORT);

	/*
	 * The control block chain is currently empty (NULL), so
	 * just make this control block's next/previous pointers
	 * point to itself.
	 */
	if (cfgncb == NULL)
	{
		cbp->cbnext = cbp;
		cbp->cbprev = cbp;
	}
	else
	{
		cbp->cbnext = cfgncb;
		cbp->cbprev = cfgncb->cbprev;
		cfgncb->cbprev->cbnext = cbp;
		cfgncb->cbprev = cbp;
	}

	/*
	 * Make this control block the first on the chain.
	 */
	cfgncb = cbp;

	if (waslocked != LOCK_NEST)
	{
		unlockl(&cfgcb_lock);
	}
	return;

}  /* end cfgnadd */

/*
 * NAME:  cfgndel
 *
 * FUNCTION:  Remove a system configurable variable "change" notification
 *	routine from use by the sysconfig system call.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can only be called by a routine executing in a process
 *	environment.  It cannot page fault.
 *
 * NOTES:  This routine is used to delete a previously-registered cfgncb
 *	structure from the list of routines to be called when a configurable
 *	variable is being changed by the sysconfig SYS_SETPARMS system call.
 *
 * DATA STRUCTURES:  cfgncb
 *
 * RETURN VALUE DESCRIPTION:	none.
 *
 * EXTERNAL PROCEDURES CALLED:  lockl,
 *				unlockl
 */
void
cfgndel(register struct cfgncb *cbp)

/* register struct cfgncb		*cbp;	pointer to config ctl blk	*/
{
	register int		waslocked;
	extern struct cfgncb	*cfgncb;

	assert(cbp != NULL);

	waslocked = lockl(&cfgcb_lock, LOCK_SHORT);

	/*
	 * The requested control block is the only one
	 * on the chain, so just make the chain NULL.
	 */
	if (cbp->cbnext == cbp)
	{
		cfgncb = NULL;
	}
	else
	{
		if (cfgncb == cbp)
		{
			/*
			 * The requested control block was the first on
			 * the chain, so the next control block is now
			 * the first...
			 */
			cfgncb = cbp->cbnext;
		}
		cbp->cbprev->cbnext = cbp->cbnext;
		cbp->cbnext->cbprev = cbp->cbprev;
	}

	if (waslocked != LOCK_NEST)
	{
		unlockl(&cfgcb_lock);
	}
	return;

}  /* end cfgndel */
