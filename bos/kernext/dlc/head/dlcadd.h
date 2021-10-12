/* @(#)71       1.12  src/bos/kernext/dlc/head/dlcadd.h, sysxdlcg, bos411, 9428A410j 6/3/94 15:26:32 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: common header file for Head Code
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* #define         _KERNEL   */
#include        <sys/signal.h>
#include        <sys/file.h>            /*  FREAD, FWRITE definition      */
#include        <fcntl.h>
#include 	<sys/device.h>
#include        <sys/buf.h>
#include        <sys/types.h>
#include        <sys/erec.h>
#include        <sys/tty.h>
#include        <sys/sleep.h>
#include        <sys/lockl.h>
/*
#include        <sys/comio.h>
#include        "enet.h"
*/
#include        <sys/user.h>
#include	<sys/uio.h>
#include	<net/spl.h>
#include	<sys/mbuf.h>
#include        <sys/pri.h>
#include        <sys/errno.h>
#include        <sys/gdlextcb.h>
#include        <sys/ioctl.h>
#include        <sys/signal.h>
#include        <sys/devinfo.h>
#include 	<sys/malloc.h>
#include	<sys/poll.h>
#include	<sys/pin.h>


/* defect 122577 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 122577 */

#define	DPOLLIN		POLLIN
#define	DPOLLOUT	POLLOUT
#define	DPOLLPRI	POLLPRI
#define	DPOLLSYNC	POLLSYNC
/************************************************************************/
/*                                                                      */
/*  dlcadd.h:   Header file that defines the IOCTL command values and   */
/*              other values used by users of the DLC access AIX        */
/*              device driver.                                          */
/* 11/1/88                                                              */
/************************************************************************/



#define DD_DLC  'D'     /* DLC Access device */
#define MAXDATASIZE 4096  
#define INIT 1
#define TERM 2
                        /* define the PORT_LOCK instances - defect 127690 */
#define PORT_CB_LOCK       1
#define PORT_RINGQ_LOCK    2
                        /* end defect 127690 */


struct dlc_port 
{
	Simple_lock	lock;
	Simple_lock	ringq_lock;         /* defect 127690 */
/* defect 115819 */
	int             term_kproc;
/* end defect 115819 */
	ulong 		chan_count;
	ulong		maxchan;
	struct dlc_chan	*cid;
	struct dlc_chan	*kcid;
	int		chpid;
	struct ring_queue *rcv_ringq;
	char		namestr[20];
/* <<< THREADS >>> */
	int		kproc_tid;           /* kproc's thread id */
/* <<< end THREADS >>> */

};
	
	
/* Multiplexed Channel Structure                                            */

struct dlc_chan 
{
	Simple_lock 		lock;
	ulong 			kernel;
	struct dlc_chan 	*mpx;
	struct dlc_chan		*bmpx;
	struct dlc_port		*cb;
	int 			(*rcvi_fa)();   /* receive I-frame fa*/
	int 			(*rcvx_fa)();   /* receive XID fa*/
	int 			(*rcvd_fa)();   /* receive Datagram fa*/
	int 			(*rcvn_fa)();   /* receive Network data fa*/
	int 			(*excp_fa)();   /* exception handler fa*/
	ulong 			maxsaps;
	ulong 			saps;
	dev_t 			dev;
	ulong 			proc_id;
	struct mbuf 		*readlist;      /* list of data recv */
	struct mbuf 		*exceptlist;    /* list of except recv */
/****************************************************************************/
/*                                                                          */
/*  I decided that the most compact way to encode the channel state is      */
/*  a set of one-bit flags.  The usual way this is done is to define a      */
/*  set of "masks" corresponding to the revelant bit positions.  An         */
/*  example of this can be found in K & R, section 6.7 Fields.  BEWARE:     */
/*  IF NEW "MASKS" ARE DEFINED, THEY MUST BE UNIQUE AND POWERS OF TWO.      */
/*                                                                          */
/****************************************************************************/

	ulong 			state;         /* channel's state */
#define BLOCKED_READ    0x04            /* is the program to perform        */
                                        /*  blocked reads                   */
#define ECOL            0x100           /* exception collision indicator.   */
#define WCOL            0x200           /* write collision indicator.       */
#define RCOL            0x400           /* read collision indicator.        */
#define S_CLOSE         0x2000          /* sleeping till CLOSE completes    */
#define S_READ          0x1000          /* sleeping till CLOSE completes    */
#define S_HALT          0x40000         /* sleeping till HALT completes     */
#define KERN            0x80000         /* sleeping till HALT completes     */
	struct proc 		*readsleep;     /* read proc_id when blocking */
	int 			writesleep;
	short 			revents;
};

struct s_mpx_tab
{
	Simple_lock 		lock;
	struct dlc_chan 	*chanp;
	struct dlc_chan 	*tmpx;
	int 			maxq;
} ;   

#define EMPTY           0
#define SET             1
#define WORD            0x02


