/* @(#)36	1.10  src/bos/kernel/sys/deviceq.h, sysxqm, bos411, 9428A410j 6/16/90 00:26:13 */
#ifndef _H_DEVICEQ
#define _H_DEVICEQ
/*
 * COMPONENT_NAME: (SYSQM) Device Queue Management header file
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	This header file defines the interface between kernel
 *	extensions and the device queue services.
 */
#ifdef _KERNEL 
#include <sys/types.h>

typedef ulong_t		cba_id_t;	/* control block ID */
typedef cba_id_t	cba_id;		/* control block ID */
typedef ulong_t		cba_t;		/* CBA index or block count */

#define	NULL_CBA	(0xFFFFFFFF)	/* invalid CBA identifier */

/*
 *	Return codes for the device queue services.
 */
#define	RC_GOOD		0		/* successful completion	*/
#define	RC_NONE		(-1)		/* no resources 		*/
#define	RC_ID		4		/* invalid identifier		*/
#define	RC_OBJ		12		/* object does not exist 	*/
#define	RC_IN_USE	20		/* object in use		*/
#define RC_MAX		28		/* limit exceeded		*/
#define RC_DEVICE	256		/* device-specific errors	*/
 
/*
 *	Queue element types
 */
#define ACKNOWLEDGE	0		/* acknowledgment		*/
#define SEND_CMD	1		/* send command 		*/
#define START_IO	2		/* start I/O			*/
#define GEN_PURPOSE	3		/* general purpose		*/
#define CONTROL 	4		/* control			*/
 
/*
 *	Queue element priorities
 */
#define QE_BEST_PRTY	0		/* most favored queue priority	*/
#define QE_WORST_PRTY	1		/* least favored queue priority	*/
 
/*
 *	Device queue acknowledgment types
 */
#define NO_ACK		0		/* no acknowledgment		*/
#define SHORT_ACK	1		/* acknowledge via e_post	*/
#define LONG_ACK	2		/* acknowledge via enque	*/
#define INTR_ACK	3		/* acknowledge via virtual intr	*/
 
/*
 *	Attach device queue (attchq) acknowledge parameter structure
 */
struct	attchq
{
	cba_id		ack_parm1;	/* acknowledge parameter one	*/
	ulong		ack_parm2;	/* acknowledge parameter two	*/
	uchar		ack_type;	/* acknowledge type		*/
	uchar		ack_depth;	/* acknowledge depth count	*/
} ;

#define MAX_ACK_DEPTH	15		/* maximum ack depth count	*/
#define MAX_QE_DEPTH	15		/* maximum queue elem depth count*/

#define VINTR(x)	((0xFF)&(x))	/* strip off virtual intr level	*/

/*
 *	Create queue (creatq) constant
 */
#define MAX_QUEUE_PATH	127		/* maximum explicit path limit	*/
 
/*
 *	Queue element operation options bit definitions
 */
#define ACK_COMPLETE	0x8000		/* acknowledge all completions	*/
#define ACK_ERRORS	0x4000		/* acknowledge errors only	*/
#define SYNC_REQUEST	0x2000		/* synchronous request		*/
#define CHAINED 	0x1000		/* chained control blocks	*/
#define CONTROL_OPT	0x0800		/* control operation		*/
#define DEVICE_OPT_MASK 0x07FF		/* device operation msak	*/
 
/*
 *	Request queue element
 */
struct	req_qe
{
	cba_t		next;		/* next qe on priority list	*/
	cba_id		path_id;	/* path identifier		*/
	uchar		type;		/* qe type			*/
	uchar		priority;	/* queue element priority	*/
	ushort		options;	/* operation options		*/
	long		data[13];	/* user data			*/
};
 
/*
 *	Dequeue queue element (deque) options
 */
#define SUPPRESS_ACK	1		/* suppress acknowledgment	*/
#define OVERRIDE_VINTR	2		/* override virtual interrupt	*/
 
/*
 *	Acknowledgment queue element
 */
struct	ack_qe
{
	cba_t		next;		/* next qe on priority list	*/
	cba_id		path_id;	/* path identifier		*/
	uchar		type;		/* type = ACKNOWLEDGE		*/
	char		pad1;		/* reserved			*/
	char		pad2;		/* reserved			*/
	uchar		overrun;	/* overrun count		*/
	long		data[13];	/* user data			*/
};
 
/*
 *	Peek queue (peekq) constant
 */
#define QE_MAX_OFFSET	10		 /* maximum queue element offset  */
 
/*
 *	Query identifier (queryi) structure
 */
struct	queryi
{
	cba_id		queue_id;	/* queue identifier		*/
	ulong		event_mask;	/* queue's event mask		*/
};
 
/*
 *	Query path (queryp) structure
 */
struct	queryp
{
	cba_id		qp_path_id;	/* path identifier		*/
	cba_id		qp_from_id;	/* attchq from identifier	*/
	cba_id		qp_to_id;	/* attchq to identifier	 	*/
	cba_id		qp_queue_id; 	/* queue identifier		*/
	cba_id		qp_ack_parm1;	/* acknowledgment parameter one	*/
	ulong		qp_ack_parm2;	/* acknowledgment parameter two	*/
	uchar		qp_ack_type;	/* acknowledgment type		*/
	pid_t		qp_from_server;	/* from server's identifer	*/
	pid_t		qp_to_server;	/* to server's identifier	*/
};

/*
 *	Type values for routines called by the kernel.
 */
#define CHECK_PARMS	20		/* check parameters routine	*/

/*
 *	The following are the definitions of the device queue services.
 */
int	ackque();			/* send an acknowledgment qe	*/
/* arguments:
 *	struct ack_qe	*qe;		acknowledge queue element
 *	int		flags;		operation flags
 *	int		results;	operation results
 */

int	attchq();			/* attach to a device queue	*/
/* arguments:
 *	cba_id	from_id;		from identifier
 *	cba_id	to_id;			to identifier
 *	cba_id	*path_id;		returned path name
 *	struct	attchq	*ptr;		acknowledge parameters
 */

int	canclq();			/* cancel queue elements	*/
/* arguments:
 *	cba_id	path_id;		path name
 */

cba_id	creatd();			/* create a device 		*/
/* arguments:
 *	ushort	iodn;			device queue name
 *	cba_id	queue_id;		queue id of the device queue
 *	int	(*attach) ();		attach routine function ptr
 *	int	(*detach) ();		detach routine function ptr
 *	caddr_t	ptr;			device dependent info
 *	int	count;			length of device dep. info
 *	caddr_t	dev_parms;		driver's device mgmt routines
 */

cba_id	creatq();			/* create a device queue	*/
/* arguments:
 *	pid_t	server_id;		server identifier
 *	uchar	worst_prty;		worst queue element priority
 *	uint	max_path;		maximum number of paths
 *	uint	max_qe;			maximum number of queue elements
 *	int	(*check) ();		check parameters routine
 *	void	(*cancel) ();		cancel queue element routine
 */

int	deque();			/* qe completion processing	*/
/* arguments:
 *	cba_id	queue_id;		queue identifier
 *	int	options;		acknowledge/override options
 *	struct	ack_qe	*qe;		acknowledgment queue element
 *	int	results;		operation results
 */

int	detchq();			/* invalidate a queue path	*/
/* arguments:
 *	cba_id	path_id;		input path identifier
 */

int	dstryd();			/* destroy a device 		*/
/* arguments:
 *	cba_id	device_id;		device id of the device queue
 */

int	dstryq();			/* destroy a device queue	*/
/* arguments:
 *	cba_id	queue_id;		queue identifier
 */

int	enque();			/* send a request queue element	*/
/* arguments:
 *	struct	req_qe	*qe;		queue element to be sent
 */

struct	req_qe *peekq();		/* peek into a device queue	*/
/* arguments:
 *	cba_id	queue_id;		queue to peek into
 *	int	offset;			queue element offset
 */

void	qmterm();			/* destroy paths/devices for proc*/
/* arguments:
 *	none
 */

int	qryds();			/* query device information	*/
/* arguments:
 *	cba_id	device_id;		device identifier
 *	caddr_t	ptr;			addr of dev. dependent data
 *	int	count;			size of output buffer
 */

cba_id	queryd();			/* query device identifier	*/
/* arguments:
 *	ushort	iodn;			device queue name
 */

int	queryi();			/* query queue information	*/
/* arguments:
 *	cba_id	query_id;		query identifier
 *	struct	queryi	queue_ids[];	returned queue information
 *	int	size;			size of queue_ids
 */

int	queryp();			/* query path information	*/
/* arguments:
 *	struct	queryp	*ptr;		returned path information
 */

struct	req_qe *readq();		/* look at active queue element	*/
/* arguments:
 *	cba_id	queue_id;		queue identifier to read
 */

void	vec_clear();			/* remove virtual intr handler	*/
/* arguments:
 *	int	levels;
 */

int	vec_init();			/* define virtual intr handler	*/
/* arguments:
 *	int	level;	
 *	int	(*func)();
 *	int	arg;
 */

struct	req_qe *waitq();		/* wait for queue element	*/	
/* arguments:
 *	cba_id	queue_id;		device queue identifier
 */

#endif	/* _KERNEL */
#endif  /* _H_DEVICEQ */
