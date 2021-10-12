static char sccsid[] = "@(#)19  1.3.1.7  uprintf.c, sysproc, bos410 11/3/93 16:26:32";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: uprintf
 *		chktty
 *		freestrs
 *		getpargs
 *		getstr
 *		printmsg
 *		upfput
 *
 *   ORIGINS: 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/uprintf.h>
#include <sys/malloc.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/intr.h>
#include <sys/sleep.h>
#include <sys/lockl.h>
#include <sys/specnode.h>
#include <sys/syspest.h>
#include <sys/systm.h>

/*
 * NAME: 	uprintf(fmt,va_arglist)
 *
 * FUNCTION:	queue a message to the process's controlling tty.
 *
 *		a uprintf struct is constructed from the specified
 *		print format string and print argument list and
 *		queued for delivery by the uprintf process.
 *		
 *		this procedure can only be called by a process and
 *		may page fault.
 *
 * PARAMETERS:
 *            	fmt 	- pointer to print format string.
 *		va_arglist - va_list containing print arguements.
 *
 * RETURN VALUES:
 *              0 	- success
 *		ESRCH	- uprintf process not active
 *		ENOMEM	- no memory for upfbuf or string buffers
 *		ENODEV	- no controlling tty
 *		EINVAL	- process cannot receive messages or invalid
 *			  parameter.
 *	
 */

uprintf(fmt,va_arglist)
char *fmt;
va_list va_arglist;
{
	struct uprintf uprintf;
	int arg, nargs, rc;
	ushort fmtvec;	
	char **argp;

	/* init pointer to default msg.
	 */
	uprintf.upf_defmsg = fmt;

	/* check if the message can be sent.
	 */
	if (rc = upcheck(&uprintf,UP_NONLS,&fmtvec,&nargs))
		return(rc);

	/* get print arguments. note that upf_nargs is set by
	 * upcheck() for the non-NLS case (UP_NONLS).
	 */
	argp = (char **) &va_arglist;
	for (arg = 0; arg < nargs; arg++)
	{
		uprintf.upf_args[arg] = *argp;
		argp++;
	}

	/* no NLS stuff.
	 */
	uprintf.upf_NLcatname = NULL;
	uprintf.upf_NLsetno = 0;
	uprintf.upf_NLmsgno = 0;

	/* queue the message.
	 */
	rc = upquemsg(&uprintf,UP_NONLS,fmtvec,nargs);

	return(rc);

}

/*
 * NAME: 	upfput(message)
 *
 * FUNCTION:	message put system call to write a uprintf message to
 *		a target process's controlling tty.
 *
 *	 	upfput() first checks if the calling process is a
 *		slave uprintf process holding a upfbuf.  if so, the
 *		user supplied message will be copied in and written
 *		to the proccess and controlling tty described by the
 *		upfbuf.  message completion processing will be performed
 *		for the upfbuf.
 *		
 * PARAMETERS:
 *            	message	- pointer to a user supplied message buffer.
 *
 * RETURN VALUE:
 *	0 	- success
 *	-1	- error occurred and u.u_error set to one of the following
 *		  errnos:
 *
 *		 ENOMEM	- memory allocation for message buffer failed, or
 *		 EFAULT	- error in copying in message, or
 *		 ENODEV	- process no longer has same controlling tty, or
 *		 EIO	- could not write to controlling tty
 *		 EPERM	- caller did not have required privilege 
 *	
 */

upfput(message)
char *message;
{
	struct upfbuf *up;
	char *msgbuf = NULL;
	int len, rc;

	/* check if caller has required privilege.
	 */
	if (privcheck(SET_PROC_RAC) == EPERM)
	{
		u.u_error = EPERM;
		return(-1);
	}

	/* check for a upfbuf (slave uprintf process).
	 */
	if ((up = u.u_message) == NULL)
	{
		u.u_error = EINVAL;
		return(-1);
	}

	/* clear the upfbuf pointer in the ublock.
	 */
	u.u_message = NULL;

	/* get storage for the message.
	 */
	if ((msgbuf = (char *) malloc(UP_MAXMSG+1)) == NULL)
	{
		u.u_error = ENOMEM;
		goto closeout;
	}

	/* copy in the message string.
	 */
	if (copyinstr(message,msgbuf,UP_MAXMSG+1,&len))
	{
		u.u_error = EFAULT;
		goto closeout;
	}

	/* make source that the target process still has the same
	 * controlling tty.
	 */
	if (u.u_error = chktty(up))
		goto closeout;

	/* write the message to the tty.
	 */
	u.u_error = printmsg(msgbuf,len,up->up_ttyd,up->up_ttyc);

	closeout:

	/* free the message buffer.
	 */
	if (msgbuf)
		free(msgbuf);
	
	/* finish up processing for this message.
	 */
	updone(up);

	return(u.u_error ? -1 : 0);
}

/*
 * NAME: 	printmsg(msg,len,dev,chan)
 *
 * FUNCTION:	print a message to a controlling tty. open the tty,
 *		write the message, and close the tty.
 *		
 *
 * PARAMETERS:
 *            	msg	- pointer to buffer containing the message
 *		len	- length of message
 *		dev	- devid of the controlling tty
 *		chan	- channel number of the controlling tty
 *
 * RETURN VALUE:
 *              0	- success.
 *		EIO	- could not write to tty
 *	
 */

static
printmsg(msg,len,dev,chan)
caddr_t msg;
int len;
dev_t dev;
chan_t chan;
{
	struct gnode *gp = NULL;
	struct uio uio;
        struct iovec iovec;
	int rc;

	/* open the tty.
	 */
	if (rc = rdevopen(dev,FWRITE|O_NDELAY,chan,0,&gp))
		goto closeout;

	/* fill out uio stuff.
	 */
        iovec.iov_len = len;
        iovec.iov_base = msg;
        uio.uio_iov = &iovec;
        uio.uio_iovcnt = 1;
        uio.uio_offset = 0;
        uio.uio_segflg = UIO_SYSSPACE;
        uio.uio_resid = len;
        uio.uio_fmode = FWRITE|O_NDELAY;

	/* write the message.
	 */
	rc = rdevwrite(dev,&uio,chan,0);

	/* close the tty.
	 */
	rdevclose(gp,FWRITE);

	closeout:

	return(rc ? EIO : rc);
}

/*
 * NAME: 	chktty(up)
 *
 * FUNCTION:	this routine is called by upfput() to determine if
 *		a process's current controlling tty is the same
 *		as it was when the message was originally queued.
 *		also, check if device still exists.
 *		
 *
 * PARAMETERS:
 *		up	- pointer to upfbuf containing process and
 *			  tty info
 *
 * RETURN VALUE:
 *              0	- tty the same
 *		ENODEV	- no tty or tty not the same
 *	
 */

static
chktty(up)
struct upfbuf *up;
{
	struct user *ublock;
	struct proc *procp;
	chan_t chan;
	int rc = 0;

	/* get addressability to the process's ublock.
	 */
	procp = PROCPTR(up->up_pid);
	ublock = (struct user *) vm_att(procp->p_adspace, &U);

	/* check dev and chan.
	 */
	if (up->up_ttyd != ublock->U_ttyd || up->up_ttyc != ublock->U_ttympx)
		rc = ENODEV;
	
	vm_det(ublock);
	if (rc)
		return(rc);

	/* determine if this is a mpx device.
	 */
	if (mpx_dev(up->up_ttyd))
		chan = up->up_ttyc;
	else
		chan = BASE_MPX;

	/* check if device still exists.
	 */
	return((devqry(up->up_ttyd,chan)) ? 0 : ENODEV);
}

/*
 * NAME: 	getpargs(upf,up,fmtvec,nargs)
 *
 * FUNCTION:	setup print arguments for a upfbuf.  the source of
 *		the print arguments is the specified uprintf struct
 *		and the print arguments are described by the passed
 *		format vector. 'by value' arguments will be copied to
 *		the upfbuf.  string buffers will be setup for string
 *		arguments; the string buffers will be anchored to the
 *		upfbuf.
 *		
 *
 * PARAMETERS:
 *            	upf	- uprintf struct pointer
 *		up	- upfbuf pointer
 *		fmtvec	- format vector
 *		nargs	- number of print arguments 
 *
 * RETURN VALUE:
 *              0	- success
 *		errors from subroutines
 *	
 */

getpargs(upf,up,fmtvec,nargs)
struct uprintf *upf;
struct upfbuf *up;
ushort fmtvec;
int nargs;
{
	int arg, rc, len;

	/* setup print arguments.
	 */
	for (up->up_nargs = 0; up->up_nargs < nargs; up->up_nargs++)
	{
		/* if this is a string argument, setup the string. otherwise,
		 * get the value argument.
		 */
		if (UPF_ISSTR(fmtvec,up->up_nargs))
		{
			/* get the string argument and mark the fmtvec
			 * in the upfbuf for the string argument.
			 */
			if (rc = getstr(upf->upf_args[up->up_nargs],
			    &up->up_args[up->up_nargs],UP_MAXSTR))
				return(rc);

			up->up_fmtvec |=  UPF_VBIT(up->up_nargs);
		}
		else
		{
			/* get the value argument.
			 */
			up->up_args[up->up_nargs] =
				upf->upf_args[up->up_nargs];
		}
	}
	return(0);
}

/*
 * NAME: 	getstr(str,spp,maxlen)
 *
 * FUNCTION:	setup up a string buffer for a string.  setup consists
 *		of allocating memory for the string buffer and copying
 *		the string to the newly allocated buffer.
 *
 * PARAMETERS:
 *		str	- pointer to source string
 *		spp	- returned string buffer pointer
 *		maxlen	- maximum source string length
 *
 * RETURN VALUES:
 *     		 0	- success.
 *		EINVAL	- NULL source string pointer, source string
 *			  length greater than maxlen
 *		ENOMEM	- memory allocation for string buffer failed
 *	
 */

getstr(str,spp,maxlen)
char *str;
char **spp;
int maxlen;
{
	int len;
	char *sp;

	*spp = NULL;

	/* check if the source string pointer is NULL.
	 */
	if (str == NULL)
		return(EINVAL);

	/* get and check the length.
	 */
	if ((len = strlen(str)) > maxlen)
		return(EINVAL);

	/* get memory for the string.
	 */
	if ((sp = (char *) malloc(len+1)) == NULL)
		return(ENOMEM);

	/* copy the string and update spp.
	 */
	bcopy(str,sp,len+1);
	*spp = sp;

	return(0);
}

/*
 * NAME: 	freestrs(up)
 *
 * FUNCTION:	free memory allocated for a upfbuf to hold print
 *		arguments, catalog name or default message string.
 *
 * PARAMETERS:
 *            	up	- upfbuf pointer
 *
 * RETURN VALUE:
 *              NONE
 *	
 */

freestrs(up)
struct upfbuf *up;
{
	int arg;

	assert(!(up->up_flags & UP_INTR));

	/* free storage for print argument strings.
	 */
	for (arg = 0; arg < up->up_nargs; arg++)
	{
		if (UPF_ISSTR(up->up_fmtvec,arg))
			free(up->up_args[arg]);
	}

	/* free catalog name string storage.
	 */
	if (up->up_NLcatname != NULL)
		free(up->up_NLcatname);

	/* free format string storage.
	 */
	if (up->up_defmsg != NULL)
		free(up->up_defmsg);

	return;
}
