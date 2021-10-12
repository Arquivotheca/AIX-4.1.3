static char sccsid[] = "@(#)70	1.13.1.1  src/bos/kernel/io/auditdev.c, sysio, bos411, 9428A410j 12/15/93 09:07:48";
/*
 * COMPONENT_NAME: (SYSAUDIT) Auditing Management
 *
 * FUNCTIONS: /dev/audit device driver
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
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * audit device special file
 *
 */

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<sys/dir.h>
#include	<sys/syspest.h>
#include	<sys/signal.h>
#include	<sys/user.h>
#include	<sys/errno.h>
#include	<sys/systm.h>
#include	<sys/proc.h>
#include	<sys/ioctl.h>
#include	<sys/devinfo.h>
#include	<sys/pri.h>
#include	<sys/malloc.h>
#include	<sys/audit.h>
#include	<sys/auditk.h>
#include	<sys/uio.h>
#include 	<sys/lockl.h>

#define 	SERIALIZE(lp) 	lockl(lp,LOCK_SIGWAKE)
#define 	UNSERIALIZE(lp) unlockl(lp)
#define		EOF	    0x0001	/* have reported EOF to some reader */
int		(*auditdev)(int , char *, int);
struct	chan	*channels = NULL;

struct auditext {
	char	*buf;
	int	len;
};

struct chan {
	int	c_flags;    		/* channel status */
	int	c_classes;   		/* classes to be audited */
	char	*c_buf;     		/* buffer of classes */
	char	*c_bufend;
	char	*c_in;      		/* records inserted here by kernel */
	char	*c_out;     		/* records removed by application */
	int	c_alloc;    		/* allocated size of buffer */
	int	c_free;     		/* free space in buffer */
	struct	chan	*c_next;	/* next channel description */
	int	Locker;
	int AnchorRead;			/* wake process */
};

static int auditwrite(int, char *, int);

auditopen(dev_t dev, int rw, struct chan *chan, struct auditext *ext){

	int	classes;
	
	if(!chan){
		u.u_error = EINVAL;
		return (-1);
	}

	/* 
	 * By default, the ALL class is recorded on the channel 
	 */

	if(ext){

		SetClasses(&classes, ext->buf, ext->len);

		if(u.u_error){
			return(-1);
		}

	}
	else {
		classes = (ulong)0xFFFFFFFF;
	}

	SERIALIZE(&chan->Locker);

	chan->c_classes = classes;

	/* 
	 * Assure that auditdev() pointer is set up for auditwrite() 
	 */

	auditdev = auditwrite;

	/* 
	 * link for auditwrite() 
	 */

	chan->c_next = channels;
	channels = chan;

	UNSERIALIZE (&chan->Locker);
	return(0);
}


auditclose(dev_t dev, struct chan *chan, int ext){

	struct	chan	*holder;

	SERIALIZE (&chan->Locker);

	/* 
	 * Unlink channels 
	 */

	holder = channels;
	if(holder == NULL){
		u.u_error = EIO;
		UNSERIALIZE (&chan->Locker);
		return(-1);
	}

	/* 
	 * At head of list 
	 */

	if(holder == chan){

		channels = chan->c_next;

		/* 
		 * NULL out auditdev() if this was last channel 
		 */

		if(channels == NULL){
			auditdev = NULL;
		}
	}
	else {

		/* 
		 * Down on the linked list
		 */

		while (1) {

			if(holder->c_next == chan){
				break;
			}

			if(holder->c_next == NULL){

				u.u_error = EIO;
				UNSERIALIZE (&chan->Locker);
				return(-1);

			}

			holder = holder->c_next;

		}
		holder->c_next = chan->c_next;

	}

	UNSERIALIZE(&chan->Locker);

	return(0);
}

/* 
 * Called at allocation and deallocation of
 * the channels
 */

auditmpx(dev, chanp, channame)
dev_t	dev;
int	*chanp;
char 	*channame;{

	struct	chan	*chan;

	if(channame != NULL){

		/* 
		 * Allocate channel 
		 */

		chan = (struct chan *)malloc(sizeof(struct chan));

		if(chan == NULL){

			u.u_error = ENXIO;
			return(-1);

		}

		bzero(chan, sizeof(struct chan));
	
		/*
	 	* Allocate buffer
	 	* the current strategy assumes the normal 
		* state immediately after
	 	* an auditread() is to have the channel empty.
	 	* A large buffer is allocated.
	 	* When the channel is drained, the pointers are reset to use
	 	* the first portion of the buffer (to avoid touching a lot
	 	* of pages unnecessarily).
	 	*/

		chan->c_alloc = 32 * 1024;
		if((chan->c_buf = palloc(chan->c_alloc, BSHIFT)) == NULL){

			free(chan);
			u.u_error = ENXIO;
			return(-1);

		}
	
		/* 
		 * Finish initialization of channel 
		 */

		chan->Locker = LOCK_AVAIL;
		chan->AnchorRead = EVENT_NULL;
		chan->c_bufend = &(chan->c_buf[chan->c_alloc]);
		chan->c_in = chan->c_out = chan->c_buf;
		chan->c_free = chan->c_alloc;
	
	
		/* 
		 * Assign return value of chanp 
		 */

		*chanp = (int)chan;
	}
	else {

		/* 
		 * Deallocate channel 
		 * Called after auditclose.
		 */

		free(((struct chan *)*chanp)->c_buf);
		free((void *)*chanp);
	}

	return(0);

}


#ifndef	DD_AUDIT
#	define	DD_AUDIT	'A'
#endif
auditioctl(dev_t dev, int cmd, int arg, ulong devflag, struct chan *chan, 
int ext){

	SERIALIZE (&chan->Locker);

	switch(cmd){

		case IOCTYPE:
			break;

		case IOCINFO:
		{
			static	struct	devinfo	devinfo =
			/* 
			 * First field is devtype 
			 */

			{ DD_AUDIT };

			if (copyout(&devinfo, arg, sizeof devinfo))
				u.u_error = EFAULT;
			break;
		}
		
		case AIO_EVENTS:
		{
			int	a;

			a = AuditClasses(arg);
			if(u.u_error){

				UNSERIALIZE (&chan->Locker);
				return(-1);

			}
			chan->c_classes = a;
			break;
		}
		
		default:

			u.u_error = EINVAL;
			UNSERIALIZE (&chan->Locker);
			return(-1);

	}
	UNSERIALIZE (&chan->Locker);
}

/* 
 * Read side of driver
 */

int
auditread(dev_t dev, struct uio *uiop, struct chan *chan, int ext){

	int	Available;
	int	to_end;
	int	error;
	struct iovec *iov = uiop->uio_iov;

	/* 
	 * Wait for data availability 
	 */

	while(1){

		int Ret;

		SERIALIZE(&chan->Locker);

		Available = chan->c_alloc - chan->c_free;
		if(Available > 0){

			UNSERIALIZE(&chan->Locker);
			break;

		}

		/* 
		 * Auditwrite saying that's all
		 */

		if(chan->c_flags & EOF){

			UNSERIALIZE(&chan->Locker);
			return(0);

		}

		/* 
		 * No data available
		 * sleep abit 
		 */

		chan->AnchorRead = EVENT_NULL;

		Ret = e_sleepl(&chan->Locker, &chan->AnchorRead, EVENT_SIGRET);


		if(Ret == EVENT_SIG){

			UNSERIALIZE(&chan->Locker);
			return(0);

		}

	}

	SERIALIZE(&chan->Locker);

	/* 
	 * Adjust amount down to user buffer 
	 * size 
	 */

	if(Available > iov->iov_len){

		Available = iov->iov_len;

	}
	
	/* 
	 * Copy to end of buffer 
	 * Wrap around buffer 
	 */

	to_end = chan->c_bufend - chan->c_out;
	if(Available > to_end){

		if(error = uiomove(chan->c_out, to_end, UIO_READ, uiop)){

			UNSERIALIZE (&chan->Locker);
			return(error);

		}

		/* 
		 * Go to the beginning
		 * of the buffer
		 */

		chan->c_out = chan->c_buf;
		chan->c_free += to_end;
		Available -= to_end;
	}

	/* 
	 * Copy data.
	 * If wrap around then this is from the beginning
	 * of the buffer.
	 */

	if(error = uiomove(chan->c_out, Available, UIO_READ, uiop)){

		UNSERIALIZE(&chan->Locker);
		return(error);

	}

	chan->c_out += Available;

	/* 
	 * Special case: At the end.  Reset pointer 
	 * to the beginning 
	 */

	if(chan->c_out >= chan->c_bufend){

		chan->c_out = chan->c_buf;

	}

	chan->c_free += Available;

	/* 
	 * When channel empties, reset pointers
	 * (so tend to stay in first pages of buffer)
	 */

	if(chan->c_free >= chan->c_alloc){

		chan->c_in = chan->c_out = chan->c_buf;
	}

	UNSERIALIZE(&chan->Locker);

	return(0);
}


/* 
 * write side of device driver
 */

static int
auditwrite(int bitmask, char *rec, int len){

	struct	chan	*chan, *next;
	char 	*NewBuf;
	int	NewSize;
	int	NewLen;
	int 	alock;

	/* 
	 * A call with bitmask 0 will force a RESET 
	 */

	if(bitmask == 0){

		for(chan = channels; chan; chan = next){

        		alock = lockl(&chan->Locker, LOCK_SHORT);

			chan->c_flags |= EOF;
			next = chan->c_next;

        		if(alock != LOCK_NEST){
                    		unlockl(&chan->Locker);
        		}

			e_wakeup(&chan->AnchorRead);
		}

		return;
	}

	/* 
	 * Write audit record 
	 * This requires two passes over the list of channels.
	 *
	 * The first pass:
	 * 1)	determines whether any channel is interested in the record
	 * 2)	finds any channel which has interest but no room
	 * 	and increases it channel size.
	 *
	 * The second pass:
	 * 1)	copies the record to all interested channels
	 * 2)	wakes any process waiting on the AuditRead anchor.
	 */

	while(1){

		/* 
		 * On exit from this for() loop,
		 * channel is NULL,
		 * or points to a channel that needs to be
		 * realloc'ed.
		 */

		for(chan = channels; chan; chan = chan->c_next){

        		alock = lockl(&chan->Locker, LOCK_SHORT);

		    	/* 
			 * Check whether this channel is interested 
			 * in this class
			 */

			if(!(bitmask & chan->c_classes)){ 

	        		if(alock != LOCK_NEST){
	                    		unlockl(&chan->Locker);
	        		}
				continue;

			}

			/* 
			 * If we have room (normal case), 
			 * nothing else to check 
			 */

			if(chan->c_free >= len){

	        		if(alock != LOCK_NEST){
	                    		unlockl(&chan->Locker);
	        		}
				continue;

			}

			/* 
			 * Assure that there is room on the channel 
			 * for the record. 
			 */

			else {

	        		if(alock != LOCK_NEST){
	                    		unlockl(&chan->Locker);
	        		}
				break;

			}

		} 

		/* 
		 * Got through the for loop, all channels ok
		 */

		if(chan == NULL)break;

		/* 
		 * Channel does not have room for the record.
		 * Expand buffer for channel.
		 */

       		alock = lockl(&chan->Locker, LOCK_SHORT);

                /* 
		 * Allocate new buffer, keeping its
                 * size a power of 2 
		 * Double it each time.
		 */

                NewSize = chan->c_alloc << 1;

                NewBuf = (char *)palloc(NewSize, BSHIFT);
                if(NewBuf == NULL){

                	u.u_error = ENOMEM;

        		if(alock != LOCK_NEST){
                    		unlockl(&chan->Locker);
        		}

                        return(-1);

                }

                /* 
		 * Copy data from old buffer to new 
		 */

		NewLen = chan->c_in - chan->c_out;
		if(NewLen <= 0){
			int	ToEnd;

			ToEnd = chan->c_bufend - chan->c_out;

			bcopy(chan->c_out, NewBuf, ToEnd);

			NewLen = chan->c_in - chan->c_buf;

			bcopy(chan->c_buf, NewBuf + ToEnd, NewLen);

			NewLen += ToEnd;
		}
		else {
			bcopy(chan->c_out, NewBuf, NewLen);
		}

		free(chan->c_buf);
		chan->c_buf = NewBuf;
		chan->c_bufend = &(NewBuf[NewSize]);
		chan->c_alloc = NewSize;
		chan->c_free = NewSize - NewLen;
		chan->c_out = chan->c_buf;
		chan->c_in = &(chan->c_buf[NewLen]);

       		if(alock != LOCK_NEST){
               		unlockl(&chan->Locker);
       		}
	}

	/* 
	 * All interested channels have room 
	 */

	for(chan = channels; chan; chan = next){

		/* 
		 * No interest 
		 */

		if(!(bitmask & chan->c_classes)){

			next = chan->c_next;
			continue;

		}

		/* 
		 * Grab lock 
		 */

       		alock = lockl(&chan->Locker, LOCK_SHORT);

		next = chan->c_next;
		/* 
		 * Copy partial data, if necessary, to end of 
		 * buffer 
		 */

		{
			int	to_end;

			to_end = chan->c_bufend - chan->c_in;
			if (to_end < len){
				bcopy(rec, chan->c_in, to_end);
				chan->c_free -= to_end;
				chan->c_in = chan->c_buf;
				len -= to_end;
				rec += to_end;
			}
		}

		/* 
		 * Copy data into buffer 
		 */

		bcopy(rec, chan->c_in, len);
		chan->c_free -= len;
		chan->c_in += len;
		if(chan->c_in >= chan->c_bufend){
			chan->c_in = chan->c_buf;
		}
		
		/* 
		 * Awaken any readers 
		 */

       		if(alock != LOCK_NEST){
                   	unlockl(&chan->Locker);
        	}

		e_wakeup(&chan->AnchorRead);
	}
}
