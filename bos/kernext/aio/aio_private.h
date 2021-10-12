/* @(#)98       1.1.2.6  src/bos/kernext/aio/aio_private.h, sysxaio, bos41J, 9521A_all 5/23/95 11:58:02 */
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: aio_private.h
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "kernel_services.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/pri.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/xmem.h>
#include <sys/adspace.h>
#include <sys/mode.h>
#include <sys/vmuser.h>
#include <sys/seg.h>
#include <sys/sleep.h>
#include <sys/except.h>
#include <sys/lockl.h>
#include <sys/fullstat.h>
#include <sys/syspest.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/fp_io.h>
#include <sys/fs_hooks.h>
#include <sys/ioctl.h>
#include <sys/buf.h>
#include <sys/pin.h>
#include <sys/dasd.h>
#include <sys/sysinfo.h>
#include <sys/device.h>

#include "debug.h"
#include "aio_interface.h"
#include <sys/aio.h>

/*
 * default configuration values
 */

#ifndef DEFMINSERVERS
#define DEFMINSERVERS 1		/* start with this many kprocs */
#endif /* DEFMINSERVERS */
#ifndef DEFMAXSERVERS
#define DEFMAXSERVERS 10	/* create no more than this many kprocs */
#endif /* DEFMAXSERVERS */
#define QTABSIZ (256+2)		/* size of array of queue headers
				   The extra 2 are for problems and
				   sockets */
#define DEFMAXREQUESTS 200	/* maximum requests outstanding */
#define DEFSPRIORITY (PUSER-1)	/* priority of kprocs */

#define STRATBUFS    20         /* buf pool block size for strategy requests*/
#define STRATREQS    10         /* request pool block size */
#define MAXREQPOOL   PAGESIZE/sizeof(struct request) /* max size of req pool*/
#define MAXBUFPOOL   2*MAXREQPOOL                    /* max size of buf pool*/
#define MAXSYSREQ    (1<<16)   /* maximum number of requests in the system*/

/*
 *  values of inprog field of request struct
 */
#define QUEUED	0	/* it's been placed on the queue, no one has it */
#define STARTED	1	/* a kproc has taken it */
#define AIO_DONE    2   /* the i/o has completed */

/*
 * return values from suspend_set
 * Numbers >= 0 indicate a request was already complete.
 */
#define SS_OK		(-1)
#define SS_ALL_NULL	(-2)

/*
 * The following define is used by the queue table routines to determine
 * what event to post to a queue server.  The value of EVENT_AQUEUE should
 * be a mask that does not conflict with the EVENT_KERNEL mask.  Adding
 * 1 to EVENT_KERNEL is a tricky way to set this mask to the first bit
 * that is not used by EVENT_KERNEL.
 */

#define EVENT_AQUEUE (EVENT_KERNEL+1)
#define EVENT_ASUSPEND (EVENT_AQUEUE<<1)

/*
 * The generation number is used to differentiate between a suspender
 * that corresponds to the io just completed and one that is for a
 * later io (the original suspender having been deleted when another
 * io on the original suspend list completed).  Each time any process
 * calls iosuspend this number is incremented and the suspender and
 * any requests we are waiting on are branded with this number.
 * Requests that aren't being iosuspended have a gen of 0.
 * We don't have to serialize access to the global generation number
 * because processes don't share aiocbs (and the gen field is reset
 * when aiocbs are reused), thus it is only necessary that each
 * invocation of iosuspend by one process produce aiocbs and requests
 * with a generation unique within its own aiocbs and requests.
 * We have to skip 0 in case we wrap, but this can be handled by the
 * calling process.
 */
#define GEN(n) do (((n) = ++aio_generation) || ++(n)); while (0)
#define ARL_PRIORITY    INTIODONE       /* priority of interrupt handler*/

/*
 * The following are MACROS defined for each simple lock in AIO.
 *
 * 1. aioq_lock   -  global - serializes addition of elements to any queue.
 *			      but does not serial deletions.
 * 2. servers_lock - global - serializes addition and deletion of servers from
 *			      the list of available servers.
 * 3. knots_lock  -  global - serializes addition and deletions of knots from
 *			      the global knot_list.
 * 4. susp_lock   -  global - serializes additions and deletions of suspenders
 *			      on the global suspenders_list.
 * 5. queue_lock  -  queue  - serializes additions and deletions of requests
 *		 	      from a particular queue.
 * 6. lvm_active - global - serialize access to active lvm request list.
 *
 * 7. devtab_lock -  global - serializes addition and deletions from global
 *			      device table.
 *
 * Hierarchy of AIO locks:
 *                      AIOQ_LOCK                LVM_LOCK       DEVTAB_LOCK
 *                          |                      /  \
 *                      QUEUE_LOCK        KNOTS_LOCK  SUSP_LOCK
 *			    |
 *			  /   \
 *		SERVERS_LOCK  SUSP_LOCK
 */


#define	AIOQ_LOCK()		simple_lock(&aio_qlock)
#define	AIOQ_UNLOCK()		simple_unlock(&aio_qlock)
#define	SERVERS_LOCK()		simple_lock(&servers.lock)
#define	SERVERS_UNLOCK()	simple_unlock(&servers.lock)
#define KNOTS_LOCK()		disable_lock(ARL_PRIORITY, &knots.lock)
#define KNOTS_UNLOCK(x)		unlock_enable(x, &knots.lock)
#define SUSP_LOCK()		disable_lock(ARL_PRIORITY, &suspenders.lock)
#define SUSP_UNLOCK(x)		unlock_enable(x, &suspenders.lock)
#define LVM_LOCK()		disable_lock(ARL_PRIORITY, &lvm_lock)
#define LVM_UNLOCK(x)		unlock_enable(x, &lvm_lock)
#define DEVTAB_LOCK()           simple_lock(&devtab_lock)
#define DEVTAB_UNLOCK()         simple_unlock(&devtab_lock)
#define QUEUE_LOCK(x)           simple_lock(&x->lock)
#define QUEUE_UNLOCK(x)         simple_unlock(&x->lock)
#define FILE_LOCK(x)            simple_lock(&(x)->f_lock)
#define FILE_UNLOCK(x)          simple_unlock(&(x)->f_lock)

/*
 * state flags to serialize multiple aio configs
 */

#define AIO_UNCONFIGURED	0
#define AIO_CONFIGURED		1
#define AIO_CONFIGURING		2

/*
 * typedefs
 */

typedef struct lio_knot	/* The object that ties together a list */
{
	int	cmd;		/* the cmd in the original listio call */
	int	count;		/* the number of requests outstanding  */
	int	interested;	/* is the originator still interested? */
	struct lio_knot	*next;
	struct lio_knot	*prev;
}
lio_knot;

typedef struct knot_list
{
	lio_knot *list;	/* linked list of knots */
	Simple_lock     lock;
}
knot_list;

typedef struct request {        /* I/O request data                     */
	struct request  *next;          /* next request on queue */
	struct request  *prev;          /* back pointer for hashing  */
	pid_t           pid;            /*  pid of requestor  */
	tid_t           tid;            /* thread id of requestor */
	struct file     *fp;            /* I/O target file pointer      */
	int             fd;             /* I/O target file descriptor   */
	int             reqtype;        /* indicates read or write      */
	struct aiocb    *aiocbp;        /* user space address of  aiocb */
	struct xmem     bufd;           /* xmem descriptor for buf */
	struct xmem     aiocbd;         /* xmem descriptor for aiocb */
	int             inprog;         /* request state                */
	lio_knot        *knotp;         /* used to group listio requests */
	ulong           susp_gen;       /* generation number for iosuspend */
	uint            runpath;        /* indicates LVM or KPROC request */
	int             buf_cnt;        /* elts in the buffer chain     */
	daddr_t         b_blkno;        /* starting block # in high-order buf*/
	uint            b_bcount;       /* transfer count in high-order buf*/
	uint            b_resid;        /* residual in high-order buf   */
	struct aiocb    kaiocb;         /* kernel copy of the aiocb     */
} request;

/* values for request.runpath   */
#define AIO_KPROC       0x00000000      /* request on the kproc path    */
#define AIO_LVM         0x00000001      /* request on the lvm path      */

struct hash_req {               /* hash table entry     */
	request *next;          /* forward pointer      */
	request *prev;          /* back pointer         */
};

/* number of hash table anchors must be a power of 2    */
#define NHREQ   PAGESIZE/(16*sizeof(struct hash_req))

/* hash function, argument is: (struct aiocb *)^(pid)  */
#define ARLHASH(x)      (&lvmio[(x) & (NHREQ-1)])

struct deventry {
	dev_t   devid;          /* devid numbers                */
	int     devtype;        /* indicates LVM or non-LVM device */
};

/* values for deventry.devtype field            */
#define AIO_LVMDEV 1            /* LVM device   */
#define AIO_OTHDEV 2            /* other device */

#define NUMVGS  20      /* number of entries in a device number block */

typedef struct devtab {
	struct devtab  *next;   /* pointer to the next block of LVM devs */
	int    empty;           /* next empty slot in this dev block     */
	struct deventry deventry[NUMVGS];  /* device entries       */
} devtab;

typedef struct queue		/* Async I/O queue header */
{
	int		req_count;
	request	       *head;
	request	       *tail;
	int		s_count;
	Simple_lock     lock;
}
queue;

typedef int key;

typedef struct server
{
	tid_t           tid;    /* thread id of kproc */
	queue	       *qp;	/* pointer to queue server is on or NULL */
	struct server  *next;	/* all servers are linked into a list */
}
server;

typedef struct s_list
{
	server 		*s_avail;	/* linked list of available servers */
	int		scount;		/* how many kprocs are running */
	Simple_lock     lock;
}
s_list;

typedef struct suspender
{
	pid_t                   pid;
	tid_t                   tid;
	struct aiocb	       *aiocbp;
	unsigned long		gen;
	struct suspender       *next;
	struct suspender       *prev;
}
suspender;

typedef struct susp_list
{
	suspender 	*list;	/* linked list of suspenders */
	Simple_lock     lock;
}
susp_list;

/*
 * global data
 */
extern aio_state;                       /* global configuration state of aio */
extern knot_list knots;                 /* list of knots */
extern susp_list suspenders;            /* list of suspenders */
extern s_list   servers;                /* list of available servers */
extern int      minservers;             /* starting number of kprocs */
extern int      maxservers;             /* max kprocs we will create */
extern queue    qtab[QTABSIZ];          /* array of queue headers */
extern Simple_lock   aio_qlock;          /* serializes access to queues */
extern int      maxreqs;                /* maximum requests outstanding */
extern int      requestcount;           /* total requests outstanding */
extern int      s_priority;             /* priority of kprocs */
extern ulong    aio_generation;         /* generation number for iosuspends */

extern Simple_lock   devtab_lock;        /* serialize access to devtab  */
extern struct devtab  *devnop;         /* LVM dev number table         */
extern struct hash_req lvmio[];         /* active lvm requests */
extern struct buf *freebufp;            /* buf free list        */
extern int      freebufsize;            /* free buf pool size   */
extern struct request *freereqp;        /* request free list    */
extern int      freereqsize;            /* free request pool size*/

/*
 * prototypes
 */
server  *new_server(void);
void     add_server(server *sp);
tid_t    get_free_server(queue *qp);
queue   *find_work(void);
queue   *find_queue(struct file *fp);
void       add_request(request *rp);
void       delete_request(request *rp, queue *qp);
request   *get_request(queue *qp);
request   *find_request(queue *qp, struct aiocb *cbp, pid_t pid);
int        cancel_request(pid_t pid, struct file *fp, int fd, struct aiocb *cbp,
			  queue *qp, request *rp);
int        cancel_fd(pid_t pid, struct file *fp, int fd, int block);
int        suspend_set(int cnt, struct aiocb *cbpa[], pid_t pid,
		       suspender *susp);
void       server_main(int data, server **spp, int plen);
int        untie_knot(lio_knot *lkp, int i);
lio_knot  *create_knot(int cmd, int nent);
void       slip_knot(lio_knot *lkp);
void       destroy_knot(lio_knot *lkp);
int        arl_rdwr(request *rp, dev_t *devp);
void       arl_iodone(struct buf *bufp);
int        arl_suspend(suspender *susp, struct aiocb *cbp, pid_t pid);
struct buf      *fetchbufs(request *rp); /* transfer request to bufs */
struct buf      *morebufs(int size);     /* allocates pinned bufs        */
struct request  *morereqs(int size);     /* allocate pinned request blocks*/
struct devtab   *moredevs();            /* allocates LVM dev table blocks */
request         *getreq();       /* get a request from request pool */
suspender *create_suspender(pid_t pid, tid_t tid, ulong gen);
extern Simple_lock   lvm_lock;           /* lock for LVM structures     */
void       remove_suspender(suspender *susp);
void       check_suspender(request *rp);
void       check_knot(request *rp);
void       releasereq(request *rp);
void       remdev(dev_t dev);
