static char sccsid[] = "@(#)92	1.26.1.12  src/bos/kernel/io/console.c, sysxcons, bos41J, 9513A_all 3/24/95 14:57:58";
/*
 * COMPONENT_NAME: SYSXCONS   /dev/console  Console Device Driver
 *
 * FUNCTIONS: conconfig, conopen, consoleopen, conclose, conmpx,
 *            conwrite, conread, conrevoke, concpyuio, conselect,
 *            conioctl, concpypath, concpyoutpath, conreopen,
 *            conerr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * FUNCTION: The console driver is a built in device driver that redirects
 *           /dev/console operations to an underlying device, file, or
 *           possibly the bit bucket. The sysconfig system call is used
 *           to configure or query the console driver about its state and
 *           who it is redirecting to. The console driver supports the 
 *	     concept of a "message" console and a default or "login"
 *           console. The message console mode is used to temporarily
 *           change the console output to a new target without affecting
 *	     the operation of the default or login console supported by 
 *           getty. When /dev/console is specified as a target for getty,
 *           it will query the console driver for the pathname of the login
 *           console and open it directly. The console driver is no longer
 *           involved. When this console driver is supporting the message
 *	     console output mode, if an error other that EINTR is detected
 *           on a write or open, the driver will switch output to the 
 *	     default or login console and not report the error. The driver
 *           keeps track of current redirection state, either NULL, message
 *	     or default (login) is supported. There is also "debugger"
 *	     output code for sending all writes to the kernel debugger 
 *	     using kernel printfs. This is predominately for phase 1 of 
 *	     system boot where the tty drivers have not been loaded, and
 *	     the only output device is supported by the debugger. 
 *
 */

/*------------
  driver interface routines:
     conconfig    - config con device driver
     conopen      - open console
     conclose     - close console
     conwrite     - write console
     conioctl     - console ioctl command
     conselect    - console select (poll) system calls
     conmpx       - console multiplex routine 

  local subroutines
     conerr       - error logging routine
     conreopen    - redirect console to new target when already open
     consoleopen  - direct console to new target when not already open
     concpyuio    - console copy uio structure
     concpypath   - console copy pathname routine

  Function:
     If the Con is directed to a "character" device, the following requests
     are passed to the device: open, close, write, ioctl and select.

     If the Con is directed to a "file", only the data from a write request
     is sent to the file.  All other calls return "FAIL".  This failure 
     is NOT error logged.

  Error Philosophy:
     All calls to external functions require logging an error at the point
     of the error.  Functions that are internal log their own errors and
     return the error code to the calling routine.  If no error found then
     a zero (0) is returned.

  NOTE:
     The term "con" is used to describe or identify this code. The term
     "console" is used to describe or identify the device or file that this
     code sends data to (ie. "the console") .

  ------------*/
#include <sys/types.h>                      /* declaration types            */

#include <sys/stat.h>                       /* used to determine file type  */
#include <sys/device.h>                     /* Device Information (devsw)   */
#include <sys/errids.h>                     /* error ids                    */
#include <sys/errno.h>                      /* System error numbers         */
#include <sys/lockl.h>                      /* lock control	            */
#include <fcntl.h>                          /* flags for open call          */
#include <sys/file.h>                       /* for select flag              */
#include <sys/fp_io.h>                      /* flags for file process       */
#include <sys/mode.h>                       /* flags for open call          */
#include <sys/sysmacros.h>                  /* Macros for MAJOR/MINOR       */
#include <sys/trchkid.h>                    /* trace stuff                  */
#include <sys/uio.h>                        /* User/Kernel I/O inferface    */
#include <sys/poll.h>                       /* poll definitions             */
#include <sys/ioctl.h>			    /* misc ioctl definitions       */
#include <sys/console.h>                    /* console ioctl definitions    */

/* return codes */
#define SUCCESS         0                   /* Successful return code       */
#define FAIL           -1                   /* FAIL return code             */

/* console_state definitions */
#define OPEN            1                   /* console opened               */
#define CLOSED          0                   /* console closed               */

/* console_flags */
#define DEBUG_ON	0x2		   /* debug mode enabled	   */
#define IS_DEVICE	0x4		   /* console is a device */
#define HELD_OPEN	0x8		   /* not used - console device is held open  */
#define CTTY_ON		0x10		   /* console is controlling tty */
#define CONS_TEE	0x20		   /* Console Output Tee active */
/* console_redir state */
#define NULLCONS	0		   /* console is null             */  
#define LOGINCONS	1		   /* console is login  	  */
#define PMSGCONS	2		   /* console is primary msg	  */

/* debug buffer size */
#define DBUFSIZE	200		   /* size of debug buffer 	  */

void	concpyuio();
/*
 * allocate and initialize the lock word.
 */

extern	int	enter_dbg;		     /* debugger available flag   */
lock_t cons_lock = LOCK_AVAIL;
static  char   login_path[255]="NULL";      /* path name of login console */
static  char   primsg_path[255]="NULL";     /* path name of pri msg console */
static  char   current_path[255]="NULL";    /* path name of current console */
static  ushort console_state = CLOSED;	    /* console state                */
static  ushort console_redir = NULLCONS;    /* console redirection flags    */
ushort console_flags = DEBUG_ON|CTTY_ON;    /* console flags	    */
static  struct file *console_fp;            /* file ptr from fp_open        */
static  char debugbuf[DBUFSIZE];	    /* debug buffer 		    */
static  struct stat stat_buf;               /* stat structure for fp_fstat  */
static	ushort	cons_isatty;		    /* 1 if console is a tty */
static  ulong  open_cnt = 0;		    /* mpx open count               */
extern	caddr_t	kernel_heap;		    /* kernel heap */
static	ulong	consmode;		    /* console device open mode */
static  struct tcons_info tcsav;	    /* console tee info save */
/*
**  Name:         conconfig
**  Description:  Configures the con device driver
**  Parameters:
**      input   dev_t   devno;          Device Number of con
**      input   int     command;        Command to Perform
**      input   struct  uio  *uio_ptr;  User IO data area (ptr to dds)
**
**  Process:
**      If the command is CONSOLE_CONFIG then
**         copy the cons_config structure into local storage
**
**  Returns:
**      0 - Successful
**      Otherwise an error occured
*/

int
conconfig( devno, command, uio_ptr )

dev_t   devno;                           /* Major and Minor device number   */
int     command;                         /* Command to perform              */
struct  uio     *uio_ptr;                /* User IO data area (ptr to dds)  */
{
	int     rc, mode;                /* Return Code, copy mode          */
	struct	cons_config	config;	 /* local copy of config parms */
	struct tcons_info tctmp;	 /* console tee info temp save */
	uint len;			 /* copystr() actual count */

	rc = SUCCESS;
	mode = uio_ptr->uio_segflg;	/* set up copy mode */
	if ((command != CONSOLE_CFG) && (command != CHG_CONS_TEE))
		return (EINVAL);
	else
		if (command == CHG_CONS_TEE) {
			int was_locked = lockl(&cons_lock, LOCK_SHORT);
			if (devno != 0) {   
				/* this is a request to redirect the console tee function */
				/* if console is already redirected then indicate error   */
				if (console_flags & CONS_TEE)
					rc = EBUSY;
				else
					if ((rc = uiomove(&tcsav, (sizeof(struct tcons_info)),
						UIO_WRITE, uio_ptr)) == 0)
						console_flags |= CONS_TEE; 
			}
			else {     /* reset console tee function */
				if ((rc = uiomove(&tctmp, (sizeof(struct tcons_info)), 
					UIO_WRITE, uio_ptr)) == 0) {
					if (tctmp.tcons_devno == tcsav.tcons_devno && 
						tctmp.tcons_chan == tcsav.tcons_chan) 
							console_flags &= ~CONS_TEE;
					else
						rc = EPERM;
				}
			}
			if (!was_locked)
			unlockl(&cons_lock);
			return(rc);
	    }
		else /* command is CONSOLE_CFG    */ 
	    {	
		lockl(&cons_lock, LOCK_SHORT);
		if (!(rc = uiomove(&config, (sizeof(struct cons_config)),
			UIO_WRITE, uio_ptr)))
		{	
			switch (config.cmd)
			{	
			case CONSETDFLT:  /* set default console device */
				rc = concpypath (mode,config.path,login_path);
				break;

			case CONSETPRIM:   /* set primary console device */
				rc = concpypath (mode,config.path,primsg_path);
				break;

			case CONSGETDFLT:	/* get default console path */
				rc = concpyoutpath (mode,login_path,config.path);
				break;

			case CONSGETPRIM:	/* get primary console path */
				rc = concpyoutpath (mode,primsg_path,
					config.path);
				break;

			case CONSGETCURR:	/* get current console path */
				rc = concpyoutpath (mode,current_path,
					config.path);
				break;

			case CONS_ACTDFLT:
				if (!(rc=conreopen (login_path,devno)))
				{				       
					bcopy (login_path, current_path,
					       sizeof(login_path));
					console_redir = LOGINCONS;
				}
				break;

			case CONS_ACTPRIM:
				if (!(rc=conreopen (primsg_path,devno)))
				{				       
					bcopy (primsg_path, current_path,
					       sizeof(primsg_path));
					console_redir = PMSGCONS;
				}
				break;

			case CONS_NULL:
				if (console_redir != NULLCONS)  
				{
					if (console_state == OPEN)
					{
						while(open_cnt)	
							(void) conclose(devno,
									0);
						/* final close of device */
						fp_close(console_fp);
					}
				}
				copystr("NULL",current_path,5,&len);
				console_redir = NULLCONS;
				break;

			case CONS_DEBUG:
				console_flags |= DEBUG_ON;	
				break;

			case CONS_NODEBUG:
				console_flags &= ~DEBUG_ON;
				break;

			case CONS_CTTY:
				console_flags |= CTTY_ON;	
				break;

			case CONS_NCTTY:
				console_flags &= ~CTTY_ON;
				break;

			default:
				rc = ENXIO;
			} /* end switch */
		}	
		unlockl(&cons_lock);
	}
	return (rc);	
}

/*
**  Name:               conopen
**  Description:        Open the Console
**  Parameters:
**      input   dev_t   devno;          Device Number of con
**      input   int     rwflag;         Read/Write flag
**      input   char    *channel_name;  Name of channel to open
**      input   int     ext;            Extension
**
**  Process:
**      If con has not been initialized
**         error log
**      else
**         if console closed
**            open the console
**
**  Returns:
**      0       Success
**      Otherwise an error code is returned
*/

int
conopen( devno, rwflag, channel, ext )

dev_t   devno;                              /* Device Number                */
ulong     rwflag;                           /* Open for read/write or both  */
chan_t  channel;                            /* channel to use               */
int     ext;                                /* Extension                    */
{
	int     rc;	 	        /* return code   */
	dev_t	consdevno;		/* device number of target */
	struct	file	*temp_fp;	/* temporary console device file ptr */
	chan_t	conschan;		/* channel number of target */
	uint	cons_type;		/* console driver type */	

	rc = SUCCESS;
	lockl(&cons_lock, LOCK_SHORT);
	if (console_state == CLOSED)
	{
	/* setup initial console open modes */
		consmode =  (rwflag & ~FREAD & ~FWRITE) | O_CREAT;
		if (rwflag & (DREAD|DWRITE)) consmode |= O_RDWR;
		else if (rwflag & DWRITE)    consmode |= O_WRONLY;
		else consmode |= O_RDONLY;  
		if (!(console_flags & CTTY_ON))	/* not the cntr tty */
		    consmode |= O_NOCTTY;
		if (console_redir != NULLCONS )
		{
			if (console_redir == LOGINCONS)
			{
				rc = consoleopen(login_path,consmode,ext);
			}
			else
			{			
				rc = consoleopen(primsg_path,consmode,ext);
			}	
		}
	}
	/* if it is a tty device then we need to redrive the open
	   to the lower device driver to possibly set up controlling
	    terminal
	 */
	else if (console_redir != NULLCONS && cons_isatty)
	{ 
		temp_fp = console_fp;
		rc = fp_getdevno(temp_fp, &consdevno, &conschan);
		if (rc == 0)
			rc = devswqry(consdevno,&cons_type,NULL); 
		if (rc == 0)
		{
			(void) fp_hold(temp_fp);
			rwflag |= DNDELAY;
			if (!(console_flags & CTTY_ON))
			    rwflag |= O_NOCTTY;
			unlockl(&cons_lock);
			rc =  (*devsw[major(consdevno)].d_open) 
				(consdevno, rwflag, conschan, ext);
			(void) fp_close(temp_fp);
			lockl(&cons_lock,LOCK_SHORT);	
			if (rc) /*  low level open to device failed */
			{
				if ((console_redir == PMSGCONS) &&
					(rc != EINTR)) 
				{
				/* attempt to switch to backup */
					if ((conreopen(login_path,devno))
						 == 0)
					/* switch over was successful */	
					{
						bcopy (login_path,
						 current_path,
						 sizeof(login_path));
						console_redir = LOGINCONS;
						rc = SUCCESS;
					}
				}
			}
			else /* no error - so if mpx then increment cnt */

			/* this count is used to drive the correct number
			   of closes to the console device if it is 
		    	   a multiplexed device driver.		        */

			{
				if (cons_type & DSW_MPX)	
					open_cnt++;	
			}
		}
	}
	if (rc == SUCCESS)
	{
		console_state = OPEN;
	}
	unlockl(&cons_lock);
	return( rc );
}
/*
**  Name:               consoleopen
**  Description:        Open the redirected Console
**  Parameters:
**      input   char *  pathptr;        pointer to pathname
**      input   int     ext;            Extension
**
**  Process:
** This routine is always called while having the cons_lock, since this
** routine is only called on first open.
**
**  Returns:
**      0       Success
**      Otherwise an error code is returned
*/

int
consoleopen (pathptr, rwflag, ext)
char	*pathptr;
ulong	rwflag;
int	ext;
{
	int retry=TRUE;
	ulong	temp_mode;
	int rc;
	console_flags |= IS_DEVICE;
	temp_mode = rwflag | O_NDELAY ;

	while (retry == TRUE)
	{
		rc = fp_open(pathptr, temp_mode,
			     0x666, ext, FP_SYS, &console_fp);
		if( rc )
			if ((pathptr == login_path) || (rc == EINTR))  
			{
				retry = FALSE;
			}
			else  /* primary msg failed to open */
			{
				pathptr = login_path;
				temp_mode = rwflag;
			}
		else	/* open succeeded */
		{
			/* get file status to determine if   */
			/* this is a special file            */

			rc = fp_fstat(console_fp, &stat_buf, 0,0);
			assert (rc == 0);

			/* determine whether it is a tty */
			cons_isatty = !(fp_ioctl(console_fp, TXISATTY, 0, 0));

			/* if it is a special file and we opened it as 
			   a device then we exit the loop */

			if ((S_ISCHR(stat_buf.st_mode)) &&
				 (console_flags & IS_DEVICE))  
				retry = FALSE;

			/* if it is a not a special file and we opened  as 
			   a file then we exit the loop */

			else if (!(S_ISCHR(stat_buf.st_mode)) &&
				!(console_flags & IS_DEVICE))  
				retry = FALSE;
			else	
			{
				/* we must close the open and re-open
				   with mode flags appropriate for a file */

				(void) fp_close(console_fp);
				console_flags &= ~IS_DEVICE;
				rwflag &= ~O_WRONLY & ~O_RDONLY;	
				rwflag |= O_APPEND|O_CREAT|O_RDWR;	
				temp_mode = rwflag;
			}	
		}
	} /* end of while loop */

	/* if we had to switch to default and were successful change current
	   console setting
	 */

	if (rc == 0 && pathptr == login_path)
	{
		bcopy (login_path, current_path, sizeof(login_path));
		console_redir = LOGINCONS;
	}
	return (rc);
}


/*
**  Name:               conclose
**  Description:        Close the con
**  Parameters:
**      input   dev_t   devno;          Device number of con
**      input   int     channel;        Channel
**      input   int     ext;            Extension
**
**  Process:
**      close console
**
**  Returns:
**      0       Success
**      Otherwise an error code is returned
*/


int
conclose( devno, channel )

dev_t   devno;                              /* Device Number of con         */
chan_t  channel;                            /*                              */
{
	int     rc;                         /* Return Code                  */
	dev_t	consdevno;		/* device number of target */
	chan_t	conschan;		/* channel number of target */
	int	was_locked;

	rc = SUCCESS;
	was_locked = lockl(&cons_lock, LOCK_SHORT);
	if (console_state != CLOSED)
	{
		if ((console_redir != NULLCONS) && (S_ISCHR(stat_buf.st_mode)))
			if (open_cnt)
			{ 
				--open_cnt;
				rc = fp_getdevno(console_fp, &consdevno,
					 &conschan);
				if (rc == 0)
				{
					if (!was_locked)
					{
						unlockl(&cons_lock);
						was_locked = 1;
					}
					rc = (*devsw[major(consdevno)].
						d_close) (consdevno,
							conschan, 0);
				}
			}
	}
	if (!was_locked)
		unlockl(&cons_lock);
	return( rc );
}

int conmpx(dev_t devno, chan_t *chanp, char *name, int openflag)
{

	int	rc;

	rc = SUCCESS;
	if (name) /* allocate requested */
	{
		*chanp = 0;
	}
	else /* de-allocate requested */
	{
		lockl(&cons_lock, LOCK_SHORT);
		if ((console_state != CLOSED) && (console_redir != NULLCONS))
			rc = fp_close(console_fp); /* final close */
		console_state = CLOSED;
		unlockl(&cons_lock);
	}
	return(rc);
}

/*
**  Name:               conwrite
**  Description:        Write request
**  Parameters:
**      input   dev_t   devno;          Device number of con
**      input   struct  uio  *uio_ptr;  UIO pointer
**      input   int     channel;        Channel
**      input   int     ext;            Extension
**
**  Process:
**      if console closed
**         return error
**      else
**         pass request to device/file
**
**  Returns:
**      0       Success
**      Otherwise an error code is returned
*/

int
conwrite( devno, uio_ptr, channel, ext )

dev_t   devno;                              /* Device Number                */
struct  uio     *uio_ptr;                   /* UIO pointer                  */
chan_t  channel;                            /* channel                      */
int     ext;                                /* Extension                    */
{
	int     rc = 0;			/* Return Code			*/
	int	n;                      /* temporary count                  */
      	struct	uio	uio_tmp;	/* copy of uio structure        */
       	struct	iovec	*iov_tmp;	/* pointer to iovec structure   */
       	struct	iovec	iov_tmp2;	/* iovec structure for single iovec */
	struct	file	*temp_fp;	/* temporary console device file ptr */
	register struct iovec *iov = uio_ptr->uio_iov;

	lockl(&cons_lock, LOCK_SHORT);

	/* Make sure this console is open.  Otherwise, its an error */
	if( console_state == CLOSED )
	{
		rc = ENXIO;                    /* No such device or address */
		unlockl(&cons_lock);
	}
	else
	{
		if ((uio_ptr->uio_iovcnt) >1)
			/* allocate array of iovec elements from heap */
			iov_tmp = xmalloc(uio_ptr->uio_iovcnt * sizeof(
				struct iovec), 0, kernel_heap);
		else
			iov_tmp = &iov_tmp2; /* use one from stack */

		concpyuio(uio_ptr, &uio_tmp, iov_tmp);
		if ((console_flags & DEBUG_ON) && (enter_dbg))
		{			/* write data to debug console */
			while (uio_tmp.uio_resid != 0)
			{
				n = uio_tmp.uio_resid;
				if( uiomove(debugbuf, DBUFSIZE-1, UIO_WRITE,
					 &uio_tmp))
					 break;
				n -= uio_tmp.uio_resid;
				debugbuf[n] = '\0';	
				printf("%s",debugbuf);
			} /* end of while */
			concpyuio(uio_ptr, &uio_tmp, iov_tmp);
		}
		if (console_redir == NULLCONS)	       
		{
			n = iov->iov_len;	/* send write to bit bucket */
			uio_ptr->uio_offset += n;
			uio_ptr->uio_resid = 0;
			iov->iov_base += n;
			iov->iov_len = 0;
			rc = SUCCESS;
			unlockl(&cons_lock);
		}
		else	/* console redirection is not to NULL */
		{
			temp_fp = console_fp;
			(void) fp_hold(temp_fp);
			unlockl(&cons_lock);

		    /* check for console tee active and if so */
		    /* send request to teed console output    */

			if (console_flags & CONS_TEE)
			{
				rc = (*devsw[major(tcsav.tcons_devno)].d_write) 
					(tcsav.tcons_devno, &uio_tmp, 
					 tcsav.tcons_chan, 0);
				concpyuio(uio_ptr, &uio_tmp, iov_tmp);
			}
		    /* send write request to device or file */
			rc = fp_rwuio(temp_fp, UIO_WRITE, uio_ptr, ext);
			(void) fp_close(temp_fp);
			if (rc == (EAGAIN | (1 << 31))) /* Streams returns */
				rc = EAGAIN;	/* EAGAIN with high bit set */
			if (rc) 	/* write failed */
			{
				if ((console_redir == PMSGCONS) &&
					(rc != EINTR) && (rc != EAGAIN)) 
				{
				        lockl(&cons_lock,LOCK_SHORT);
					/* attempt to switch to backup */
					if (!(conreopen(login_path,devno)))
					{
						bcopy (login_path,
						 current_path,
						 sizeof(login_path));
						console_redir = LOGINCONS;
						concpyuio(&uio_tmp,uio_ptr,
							(struct iovec *)
							(uio_ptr->uio_iov));
						temp_fp = console_fp;
						(void) fp_hold(temp_fp);
						unlockl(&cons_lock);
				       		rc = fp_rwuio(temp_fp, 
							UIO_WRITE, uio_ptr,
								 ext);
						(void) fp_close(temp_fp);
						if (rc == (EAGAIN | (1 << 31)))
							rc = EAGAIN;
						if (rc)
						/* write to default failed*/
						{
							conerr( "console",
								"conwrite",
								"UIO_WRITE",
								rc,NULL);
						}
					}
					else /* conreopen failed */
					{
						unlockl(&cons_lock) ;
					}
				}
				else
				{
					conerr( "console","conwrite",
						"UIO_WRITE",rc,NULL);
				}
			}
		}
	if (iov_tmp != (&iov_tmp2))
		/* if iovec array allocated from heap then free it */
		(void) xmfree(iov_tmp, kernel_heap);
	}
	return( rc );
}
/*
**  Name:               conread
**  Description:        Read request
**  Parameters:
**      input   dev_t   devno;          Device number of con
**      input   struct  uio  *uio_ptr;  UIO pointer
**      input   int     channel;        Channel
**      input   int     ext;            Extension
**
**  Process:
**      if console closed
**         return error
**      else
**         if device
**            pass request to device
**         else (file)
**            return error
**
**  Returns:
**      0       Success
**      Otherwise an error code is returned
*/


int
conread( devno, uio_ptr, channel, ext )

dev_t   devno;                              /* Device Number                */
struct  uio     *uio_ptr;                   /* UIO pointer                  */
chan_t  channel;                            /* channel                      */
int     ext;                                /* Unused - (Extension)         */
{
	int     rc;                         /* Return Code                  */
	struct file *temp_fp;		   /* temporary console file ptr */

	lockl(&cons_lock, LOCK_SHORT);

		     /* Make sure console is open.  Otherwise, its an error */
	if( console_state == CLOSED || console_redir == NULLCONS )
	{
		rc = ENXIO;                    /* No such device or address */
		unlockl(&cons_lock);
	}
	else
	{
	 	/* determine if this is a special file */
		if ( S_ISCHR(stat_buf.st_mode))
		{
						  /* Pass request to device */
			temp_fp = console_fp;
			(void) fp_hold(temp_fp);
			unlockl(&cons_lock);
			rc = fp_rwuio(temp_fp, UIO_READ, uio_ptr, ext);
			(void) fp_close(temp_fp);

		}
		else                                       /* file          */
		{
					 /* not allowed to read from a file */
			rc = EACCES ;                  /* PERMISSION DENIED */
			unlockl(&cons_lock);
		}
	}

	return( rc );
}

/*
**  Name:               conrevoke
**  Description:        Handle a revoke request
**  Parameters:
**      input   dev_t   devno;          Device number of con
**      input   int     channel;        Channel
**      input   int     flag;           Flag
**
**  Process:
**      if console closed
**        return error
**      else
**        if device
**          pass request to device
**        else (file)
**          return error
**
**  Returns:
**      0 Success
**      Otherwise an error occured
*/

int
conrevoke( devno, channel, flag)

dev_t   devno;                              /* Device Number                */
chan_t channel;                             /* channel                      */
int     flag;                               /* flag                         */
{
	int     rc;                         /* Return Code                  */
	struct file *temp_fp;		   /* temporary console file ptr */

	lockl(&cons_lock, LOCK_SHORT);

		/* Make sure this console is open.  Otherwise, its an error */
	if( console_state == CLOSED || console_redir == NULLCONS )
	{
		rc = ENXIO;                    /* No such device or address */
		unlockl(&cons_lock);
	}
	else
	{
		/* determine if this is a special file */
		if ( S_ISCHR(stat_buf.st_mode))
		{
			  /* Pass request to device */
			temp_fp = console_fp;
			(void) fp_hold(temp_fp);
			unlockl(&cons_lock);
			rc = fp_revoke(temp_fp, channel, flag);
			(void) fp_close(temp_fp);
			if( rc )
			{
				conerr( "console","conrevoke","fp_revoke",
					 rc,NULL);
			}

		}
		else
		{
			/* not allowed to revoke a file */
			rc = EACCES ;         /* PERMISSION DENIED  */
			unlockl(&cons_lock);	
		}
	}

	return( rc );
}

/*
**  Name:               concpyuio
**  Description:        Copies a uio structure and 1 associated iovec element
**  Parameters:
**      input   struct uio *in_uiop;    "from" uio structure pointer
**      input   struct uio *out_uiop;   "to" uio structure pointer
**      input	struct iovec *out_iovp; "to" iovec structure pointer
**
**  Process:
**
**  Returns: void
*/

void
concpyuio (in_uiop, out_uiop,out_iovp)
struct uio	*in_uiop;
struct uio	*out_uiop;
struct iovec	*out_iovp;
{
	/* copy uio structure then iovec structure */
	bcopy(in_uiop, out_uiop, sizeof(struct uio));		
	bcopy(in_uiop->uio_iov, out_iovp,
		 (in_uiop->uio_iovcnt) * sizeof(struct iovec)); 
	out_uiop->uio_iov = out_iovp;	/* point output uio to output iovec */
	return;
}

/*
**  Name:               conselect
**  Description:        Handle a select request from the user
**  Parameters:
**      input   dev_t   devno;          Device number of con
**      input   ushort  events;         Events
**      input   ushort  *return_events; Returned Events
**      input   int     channel;        Channel
**
**  Process:
**      if console closed
**        return error
**      else
**        if device
**          pass request to device
**        else (file)
**          return error
**
**  Returns:
**      0 Success
**      Otherwise an error occured
*/

int
conselect( devno, events, return_events, channel )

dev_t   devno;                              /* Device Number                */
ushort  events;                             /* Events which are checked     */
ushort  *return_events;                     /* Return Events                */
chan_t channel;                             /* channel                      */
{
	int     rc;                         /* Return Code                  */
	struct file *temp_fp;		   /* temporary console file ptr */

	lockl(&cons_lock, LOCK_SHORT);

		/* Make sure this console is open.  Otherwise, its an error */
	if( console_state == CLOSED || console_redir == NULLCONS)
	{
		rc = ENXIO;                    /* No such device or address */
		unlockl(&cons_lock);
	}
	else
	{
		/* determine if this is a special file */
		/* if special file or not read event request then allow */
		if ( S_ISCHR(stat_buf.st_mode) || (!(events & POLLIN)))
		{
			  /* Pass request to device */
			temp_fp = console_fp;
			(void) fp_hold(temp_fp);
			unlockl(&cons_lock);
			rc = fp_select(temp_fp, events,
			       return_events, NULL);
			(void) fp_close(temp_fp);	
		}
		else	/* not allowed to select from file or read event */
			{
			rc = EACCES ;                 /* PERMISSION DENIED */
			unlockl(&cons_lock);
			}
	}

	return( rc );
}


/*
**  Name:         conioctl
**  Description:  process ioctl call
**  Parameters:
**      input   dev_t  devno;                   major, minor number of device
**      input   int    cmd;                     ioctl command
**      input   int    arg;                     buffer pointer, usually
**      input   int    mode;                    mode
**      input   int    channel;                 channel
**      input   int    ext;                     extension
**	input	long  *ioctlrv;			return value for STREAMS
**
**  Process:
**      set flag if called from kernel
**      switch ioctl command
**        case: change console
**            if console open
**               open new console
**            save new console data
**        default:
**            if console is a device
**              pass request to device
**            else (console is a file)
**              return error
**
**  Returns:
**      0 = Success
**      Otherwise an error occured.
*/

conioctl (devno, cmd, arg, mode, channel, ext, ioctlrv)
dev_t devno;                            /* major, minor number of device    */
int cmd;                                /* ioctl command                    */
int arg;                                /* buffer pointer, usually          */
ulong mode;                             /* mode                             */
chan_t channel;                         /* channel                          */
int ext;                                /* extension                        */
long *ioctlrv;				/* return value for STREAMS ioctls  */
{
	int  rc;                      /* return code                        */
	dev_t	consdevno;	      /* redirected console device number */
	chan_t	conschan;	      /* redirected console chan number */
	struct file *temp_fp;		   /* temporary console file ptr */

	lockl(&cons_lock, LOCK_SHORT);
	rc = SUCCESS;                 /* init return code                   */

       /* make sure console is open */
	if ( console_state == CLOSED || console_redir == NULLCONS)
	{
		rc = EACCES ;        /* PERMISSION DENIED      */
		unlockl(&cons_lock);
	}
	else                      /* console is open           */
	{
		/* determine if this is a special file */
		if ( S_ISCHR(stat_buf.st_mode))
		{
			temp_fp = console_fp;
			(void) fp_hold(temp_fp);
			unlockl(&cons_lock);
			rc = fp_getdevno(temp_fp, &consdevno, &conschan);
			if (rc == 0)
			{
			/* Pass request to device can't use fp_ioctl because
			   of DKERNEL flag always set for cmd arg parms  */

				rc =  (*devsw[major(consdevno)].d_ioctl) 
					(consdevno, cmd, arg, mode,
					 conschan, ext, ioctlrv);
			}
			(void) fp_close(temp_fp);
		}
		else                 /* console is file        */
		{
			rc = EACCES ;         /* PERMISSION DENIED */
			unlockl(&cons_lock);
		}
	}
	return(rc);
}

int
concpypath (mode, instr, outstr)
int mode;
char *instr;
caddr_t outstr;
{
	int	n, rc = SUCCESS;
	if (!(mode & UIO_SYSSPACE))
	{
		rc = copyinstr(instr,outstr, sizeof(login_path),&n);
		if( rc )       /* error from copyin            */
		    {
			    conerr( "console", "conconfig", "copyinstr",
				     rc, NULL);
		    }
	}
	else
	{
		/* copy from kernel */
		bcopy(instr, outstr,
			 (sizeof(login_path)));
	}
	return (rc);
}

int
concpyoutpath (mode, instr, outstr)
int mode;
caddr_t instr;
char *outstr;
{
	int	rc = SUCCESS;
	if (!(mode & UIO_SYSSPACE))
	{
		rc = copyout(instr,outstr, strlen(instr)+1);
		if( rc )       /* error from copyout            */
		    {
			    conerr( "console", "conconfig", "copyout",
				     rc, NULL);
		    }
	}
	else
	{
		/* copy to kernel */
		bcopy(instr,outstr,strlen(instr)+1);
	}
	return (rc);
}

/* this routine is always called with the cons_lock */
int
conreopen (pathp,devno)
char *pathp;
dev_t devno;
{
	struct file *temp_fp;         /* temp file ptr from devopen         */
	int	rc, temp_flag;	      /*  return code, temporary flags      */
	int	retry = TRUE;
	struct  stat stat_tmp;	      /* temporary stat buffer */
	ulong	temp_mode;	      /* temporary open mode */
	/* slip in the new device               */
	if (console_state == CLOSED)
	{
		 return (0);
	}
	else
	{
		temp_flag = IS_DEVICE;
		temp_mode = consmode | O_NDELAY;     /* orig flags + NDELAY*/

		while (retry == TRUE)
		{
			rc = fp_open(pathp, temp_mode,
				     0x666, 0 , FP_SYS, &temp_fp);
			if( rc )
			{
				retry = FALSE;
			}
			else	/* open succeeded */
			{
				/* get file status to determine if   */
				/* this is a special file            */
				assert( fp_fstat(temp_fp, &stat_tmp, 0,
					      0) == 0);

				/* determine whether it is a tty */
				cons_isatty = !(fp_ioctl(temp_fp, 
					TXISATTY, 0, 0));

				/* if it is a special file and we opened it as 
				   a device then we exit the loop */
				if (S_ISCHR(stat_tmp.st_mode) &&
					 (temp_flag & IS_DEVICE))
					retry = FALSE;
				/* if it is not a special file and we opened
				   it as such then we must exit the loop */
				else if (!(S_ISCHR(stat_tmp.st_mode)) &&
					!(temp_flag & IS_DEVICE))  
					retry = FALSE;
				else 
				{
					/* we must close the open and re-open
					   with mode flags appropriate for
					 a file */
					(void) fp_close(temp_fp);
					temp_flag &= ~IS_DEVICE;
					temp_mode = consmode;
					temp_mode &= ~O_WRONLY & ~O_RDONLY;
					temp_mode |= O_APPEND|O_CREAT|O_RDWR;
				}	
			}
		} /* end of while loop */
		if (rc == 0)
		{	
			while(open_cnt)	
				(void) conclose(devno,0);
			if (console_redir != NULLCONS)
			{
				fp_close(console_fp); /* final close */
			}
			/* copy information          */
			console_flags &= ~IS_DEVICE;
			console_flags |= temp_flag;
			bcopy (&stat_tmp,&stat_buf,sizeof(stat_buf)); 
			console_fp = temp_fp;
		}
		return (rc);
	}
}


/*
**  Name:         conerr
**  Description:  send system error log
**  Parameters:
**      input   char    *res_name            component name
**      input   char    *dmodule             detecting function name
**      input   char    *fmodule             failing function name
**      input   int     return_code          code from failing function
**      input   int     err_ind              indicator from RAS.h
**
**  Process:
**
**  Returns:
**      N/A
*/

int
conerr(res_name,dmodule,fmodule,return_code,err_indicator)
char   *res_name;       /* Failing software component name.                 */
char   *dmodule;        /* The detecting module called the failing module.  */
char   *fmodule;        /* The failing module returned the bad return code. */
int    return_code;     /* Return code from failing module                  */
int    err_indicator;   /* Unique error indicator number                    */
{

#define MAXNAMESIZE 	10         /* Max sz of component and module names. */

	struct detailData { 
		char 	detectMod[MAXNAMESIZE];
		char 	failMod[MAXNAMESIZE];
	   	int	retCode;
		int	errCode;
	} *detail;

	ERR_REC(sizeof(struct detailData)) ER;
	
	if ((return_code != EINTR) && (return_code != EAGAIN))
	{
		/* Template Id                      */
		ER.error_id = ERRID_CONSOLE;

		/* Copy failing SW comp name into ER. */
		strcpy(ER.resource_name,res_name);

		/* Copy detail data into ER */
		detail = (struct detailData *) ER.detail_data;
		strcpy(detail->detectMod,dmodule);
		strcpy(detail->failMod,fmodule);
		detail->retCode = return_code;
		detail->errCode = err_indicator;

		/* Call system error logging routine ERRSAVE().
		   The parameters are a pointer to the error record structure
		   and the number of bytes in the error record structure.
                 */

		errsave(&ER, ERR_REC_SIZE + sizeof(struct detailData));
	}
	return;
}

