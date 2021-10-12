static char sccsid[] = "@(#)18	1.40.1.8  src/bos/kernel/ipc/msg.c, sysipc, bos411, 9431A411a 7/22/94 10:13:38";

/*
 * COMPONENT_NAME: (SYSIPC) IPC message handling services
 *
 * FUNCTIONS: msgconv msgfree msgctl kmsgctl rmsgctl msgget kmsgget
 *	rmsgget msgrcv msgxrcv kmsgrcv rmsgrcv msgsnd kmsgsnd rmsgsnd
 *	msgsleep msgselect msg_lock_init
 *
 * ORIGINS: 27, 3, 26, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

#include <sys/user.h>
#include <sys/msg.h>
#ifndef _POWER_MP
#include <sys/lockl.h>
#endif
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/sysinfo.h>
#include <sys/poll.h>
#include <sys/utsname.h>
#include <sys/syspest.h>
#include <sys/malloc.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/priv.h>
#include <sys/audit.h>
#include <sys/id.h>
#include <sys/intr.h>

#ifdef _POWER_MP
#include <sys/atomic_op.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif

extern struct msqid_ds  msgque[];       /* msg queue headers */
extern struct msginfo   msginfo;        /* message parameters */
extern time_t           time;           /* system idea of date */

struct msqid_ds         *ipcget(),
                        *msgconv();

#ifndef _POWER_MP
lock_t mes_lock = LOCK_AVAIL;		/* ipc message lock */
#else
/*
 * The follwing value is a tunable value. The current value (16) might be small.
 */
#define	NB_MSG_LOCKS    16		/* Number of locks for MSGMNI queues  */

Simple_lock	lock_msg_creat;		/* Lock to create a non private queue */
Simple_lock	lock_msg_qid[NB_MSG_LOCKS];
					/* Locks to access message queues     */
#endif

uint_t msg_mark = 0;			/* high water mark  */

/* messages debug variable */
BUGVDEF(mdb, 0);

void rmsgctl();
/*
 * NAME: msg_lock_init
 *
 * FUNCTION: This function is called in ipc_lock_init() in ipc.c.
 *           ipc_lock_init() is called during system initialization when
 *           _POWER_MP is defined.
 *
 * EXECUTION ENVIRONMENT:
 *
 * DATA STRUCTURES: lock_msg_creat, lock_msg_qid[]
 *
 * RETURNS: None.
 */ 

#ifdef _POWER_MP
void
msg_lock_init()
{
	int i;

	/* initialize the message queue locks */
	lock_alloc(&lock_msg_creat,LOCK_ALLOC_PAGED,
		MSG_LOCK_CLASS, 32767);
	simple_lock_init(&lock_msg_creat);
	for (i=0;i < NB_MSG_LOCKS;i++) {
		lock_alloc(&lock_msg_qid[i],LOCK_ALLOC_PAGED,
			MSG_LOCK_CLASS, i);
		simple_lock_init(&lock_msg_qid[i]);
	}
}
#endif
/*
 * NAME: msgconv
 *
 * FUNCTION: Convert a user supplied message queue id into a ptr to a
 *	msqid_ds structure.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine runs at process level
 *
 * NOTES: Locks the msqid_ds structure before returning
 *
 * DATA STRUCTURES: sets u.u_error if an error is detected
 *
 * RETURNS: Pointer to msqid_ds structure. NULL is structure is not found
 *
 * MP MODIFICATION:
 *	Upon success, hold the lock protecting the access to the message queue.
 */

struct msqid_ds *
msgconv(id)
uint_t	id;
{
	register struct msqid_ds *qp;	/* ptr to associated q slot	*/
	uint_t index;			/* descriptor index		*/
	unsigned short seq;		/* sequence number		*/

	seq = id / msginfo.msgmni;
	index = id % msginfo.msgmni;

	if ( index >= msg_mark ) {
		u.u_error = EINVAL;
		return( NULL );
	}

        qp = &msgque[index];

#ifdef _POWER_MP
        simple_lock(&lock_msg_qid[id % NB_MSG_LOCKS]);
#endif

        if (    (seq != qp->msg_perm.seq) ||
		!(qp->msg_perm.mode & IPC_ALLOC)  ) {
#ifdef _POWER_MP
		simple_unlock(&lock_msg_qid[id % NB_MSG_LOCKS]);
#endif
                u.u_error = EINVAL;
                return(NULL);
        }

        return(qp);
}

/*
 * NAME: msgfree
 *
 * FUNCTION: Free up space and message header, relink pointers on q,
 *	and wakeup anyone waiting for resources.
 *
 * EXECUTION ENVIRONMENT:
 *	Called from message system calls
 *
 * RETURNS: NONE
 */

void
msgfree(qp, pmp, mp, msgid)
register struct msqid_ds        *qp;    /* ptr to q of mesg being freed */
register struct msg             *mp,    /* ptr to msg being freed */
                                *pmp;   /* ptr to mp's predecessor */
register int			msgid;	/* message id for selnotify	*/
{
	int rv;

        /* Unlink message from the q. */
        if(pmp == NULL)
                qp->msg_first = mp->msg_next;
        else
                pmp->msg_next = mp->msg_next;
        if(mp->msg_next == NULL)
                qp->msg_last = pmp;
        qp->msg_qnum--;
        if(qp->msg_perm.mode & MSG_WWAIT) {
                qp->msg_perm.mode &= ~MSG_WWAIT;
                e_wakeupx(&qp->msg_wwait, E_WKX_NO_PREEMPT);
        }

        /* Free up header */
	rv = xmfree(mp, kernel_heap );
	ASSERT( 0 == rv );

        /* wakeup processes waiting on select send */
        if (qp->msg_reqevents & POLLOUT)
	{
		selnotify(POLL_MSG, msgid, POLLOUT);
                qp->msg_reqevents &= ~POLLOUT;
        }
}

/*
 * NAME: msgctl
 *
 * FUNCTION: Provides message control operations
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES: changes u.u_error
 *
 * RETURNS: 0 = successful	-1 = not successful
 */

int
msgctl(int msgid, int cmd, struct msqid_ds *buf)
/* int			msgid;	*/
/* int			cmd;	*/
/* struct msqid_ds *	buf;	*/
{
	struct msqid_ds	ds;	/* queue work area */
	mtyp_t		mtype;	/* mtype work area */
	static int svcnumO = 0;
	static int svcnumM = 0;

	TRCHKGT_SYSC(MSGCTL, msgid, cmd, buf, NULL, NULL);
#ifndef _POWER_MP
	sysinfo.msg++;
	cpuinfo[CPUID].msg++;
#else
	fetch_and_add((atomic_p) &sysinfo.msg,1);
	fetch_and_add((atomic_p) &cpuinfo[CPUID].msg,1);
#endif

        switch(cmd) {

        case IPC_SET:
                if(copyin(buf, &ds, MSG_STAT)) {
                        u.u_error = EFAULT;
                        break;
                }
		if(audit_flag && audit_svcstart("MSG_Owner", &svcnumO, 3,
		msgid, ds.msg_perm.uid, ds.msg_perm.gid)){
			audit_svcfinis();
		}
                rmsgctl(msgid, cmd, &ds);
                break;

        case IPC_STAT:
                rmsgctl(msgid, cmd, &ds);
		if(u.u_error)
                        break;
		if(audit_flag && audit_svcstart("MSG_Mode", &svcnumM, 1, ds.msg_perm.mode)){
			audit_svcfinis();
		}
                if(copyout(&ds, buf, MSG_STAT))
                        u.u_error = EFAULT;
                break;

        default:
                rmsgctl(msgid, cmd, &ds);
        }

	return u.u_error ? -1 : 0;
}

/*
 * NAME: kmsgctl
 *
 * Function: Kernel Interface to message control function
 *
 * EXECUTION ENVIRONMENT:
 *	Caller must be a process
 *
 * NOTES:
 *	This interface preserves the value of u.u_error
 *
 * RETURNS:
 *	0 if successful
 *	non zero errno value if not successful
 */

int
kmsgctl(msgid, cmd, buf)
int msgid;			/* mesage queue identifier	*/
int cmd;			/* mesctl command		*/
struct msgqid_ds *buf;		/* messgqid descriptor		*/

{
	int error_save;		/* save u.u_error here		*/
	int rc;			/* return code			*/

	error_save = u.u_error;
	u.u_error = 0;

	rmsgctl(msgid, cmd, buf);

	rc = u.u_error;
	u.u_error = error_save;
	return( rc );

}

/*
 * NAME: rmsgctl
 *
 * FUNCTION: common message control function
 *
 * EXECUTION ENVIRONMENT:
 *	Part of msgctl system call, or for kmsgctl
 *
 * DATA STRUCTURES: Changes u.u_error
 *
 * RETURNS: 0 = successful	-1 = not successful
 *
 * MP MODIFICATION:
 *	Reset the key to IPC_PRIVATE while removing a message queue.
 */

void
rmsgctl(msgid, cmd, hdsp)
int msgid, cmd;
mtyp_t *hdsp;
{
        register struct msqid_ds        *qp;    /* ptr to associated q */
        struct msqid_ds                 *dsp;   /* ditto */
	int				rv;
	static int svcnumR = 0;
	uid_t				current_uid;

#ifndef _POWER_MP
	rv = lockl(&mes_lock, LOCK_SHORT);
	ASSERT( rv == LOCK_SUCC );
#endif

	/*
	 * MP mode : On successful completion,
	 *           msgconv() holds the lock protecting the message queue.
	 */
        if((qp = msgconv(msgid)) == NULL) {
#ifndef _POWER_MP
		unlockl(&mes_lock);
#endif
		return;
	}

        dsp = (struct msqid_ds *) hdsp;

	current_uid = getuidx(ID_EFFECTIVE);
        switch(cmd) {

        case IPC_RMID:
		if(audit_flag && audit_svcstart("MSG_Delete",&svcnumR, 1, msgid)){
			audit_svcfinis();
		}
                if(current_uid != qp->msg_perm.uid && current_uid != qp->msg_perm.cuid
                && !priv_req(BYPASS_DAC_WRITE)) {
                        u.u_error = EPERM;
                        break;
                }
                while(qp->msg_first)
                        msgfree(qp, NULL, qp->msg_first, msgid);
                qp->msg_cbytes = 0;
                qp->msg_perm.seq++;
                if(qp->msg_perm.mode & MSG_RWAIT)
                        e_wakeup(&qp->msg_rwait);
                if(qp->msg_perm.mode & MSG_WWAIT)
                        e_wakeup(&qp->msg_wwait);

/*
 * Wakeup processes blocked on select read and write for queue being removed.
 */

		if ( qp->msg_reqevents )
		{
			selnotify(POLL_MSG, msgid, qp->msg_reqevents);
		   	qp->msg_reqevents = 0;
		}
#ifdef _POWER_MP
		/*
		 * MP mode : Avoid a concurrent ipcget() finding a released queue.
		 */
		qp->msg_perm.key = IPC_PRIVATE;
#endif
		qp->msg_perm.mode = 0;
                break;

        case IPC_SET:
                if(current_uid != qp->msg_perm.uid && current_uid != qp->msg_perm.cuid
                && !priv_req(SET_OBJ_DAC)) {
                        u.u_error = EPERM;
                        break;
                }
                if(dsp->msg_qbytes > qp->msg_qbytes &&
					!priv_req(SET_OBJ_STAT)) {
                        u.u_error = EPERM;
                        break;
                }
                qp->msg_perm.uid = dsp->msg_perm.uid;
                qp->msg_perm.gid = dsp->msg_perm.gid;
                qp->msg_perm.mode = (qp->msg_perm.mode & ~0777) |
                                    (dsp->msg_perm.mode & 0777);
                qp->msg_qbytes = dsp->msg_qbytes;
                qp->msg_ctime = time;
                break;

        case IPC_STAT:
                if(ipcaccess(&qp->msg_perm, MSG_R))
                        break;
                *dsp = *qp;
                break;

        default:
                u.u_error = EINVAL;
                break;
        }

#ifndef _POWER_MP
	unlockl(&mes_lock);
#else
	/* Cast to unsigned int is necessary while msgconv() does so */
	simple_unlock(&lock_msg_qid[((uint_t) msgid) % NB_MSG_LOCKS]);
#endif
}


/*
 * NAME: msgget
 *
 * FUNCTION: msgget system call
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES: changes u.u_error
 *
 * RETURNS:
 *	-1 = not successful
 *	message queue identifier if successful
 */

int
msgget(key_t key, int msgflg)
/* key_t key;	*/
/* int msgflg;	*/
{
	int rval;	/* return value: -1 or msgid */
	static int svcnum = 0;

	if(audit_flag && audit_svcstart("MSG_Create", &svcnum, 2, key, msgflg)){
		audit_svcfinis();
	}

	TRCHKLT_SYSC(MSGGET, key);
#ifndef _POWER_MP
	sysinfo.msg++;
	cpuinfo[CPUID].msg++;
#else
	fetch_and_add((atomic_p) &sysinfo.msg,1);
	fetch_and_add((atomic_p) &cpuinfo[CPUID].msg,1);
#endif

	return(rmsgget(key, msgflg));

}

/*
 * NAME: kmsgget
 *
 * FUNCTION: kernel interface to message get function
 *
 * EXECUTION ENVIRONMENT:
 *	Caller must be a process
 *
 * NOTES:
 *	This interface preserves the value of u.u_error
 *	*msgid will be set to to the message queue id if successful
 *	and to -1 of not successful
 *
 * RETURNS:
 *	0 if successful
 *	non-zero value of errno if not successful
 */

int
kmsgget(key, msgflg, msgid)
key_t key;			/* key for message queue	*/
int msgflg;			/* flags for msgget call	*/
int *msgid;			/* value of message id		*/
{
	int error_save;		/* save u.u_error here		*/
	int rc;			/* return value			*/

	error_save = u.u_error;
	u.u_error = 0;

	*msgid = rmsgget(key, msgflg);

	rc = u.u_error;
	u.u_error = error_save;
	return( rc );

}

/*
 * NAME: rmsgget
 *
 * FUNCTION: common message get routine
 *
 * EXECUTION ENVIRONMENT:
 *	Called from msgget system call
 *
 * DATA STRUCTURES: u.u_error changed
 *
 * RETURNS: queue id if successful	-1 = not successful
 *
 * MP MODIFICATION:
 *	Take the lock_msg_creat lock to create a non private message queue.
 *	Set the value of the key as the last initialisation operation.
 */

int
rmsgget(key, msgflg)
key_t key;
int msgflg;
{
        struct msqid_ds *qp;		/* ptr to associated q */
        int s;                          /* ipcget status return */
	int id;
	int rv;

#ifndef _POWER_MP
	rv = lockl(&mes_lock, LOCK_SHORT);
	ASSERT( rv == LOCK_SUCC );
#else
	if ((key != IPC_PRIVATE) && (msgflg & IPC_CREAT))
		simple_lock(&lock_msg_creat);
#endif

        qp = ipcget(key,msgflg,msgque,msginfo.msgmni,sizeof(*qp),&s,
				&msg_mark);
        if(qp == NULL)
		goto outx;

        if(s) {
        /* This is a new queue.  Finish initialization. */
                qp->msg_first = qp->msg_last = NULL;
                qp->msg_qnum = 0;
                qp->msg_cbytes = 0;
                qp->msg_qbytes = msginfo.msgmnb;
                qp->msg_lspid = qp->msg_lrpid = 0;
                qp->msg_stime = qp->msg_rtime = 0;
                qp->msg_ctime = time;
		qp->msg_reqevents = 0;
		qp->msg_rwait = EVENT_NULL;
		qp->msg_wwait = EVENT_NULL;
#ifdef _POWER_MP
		qp->msg_perm.key = key;
#endif
        }

	id = (qp->msg_perm.seq * msginfo.msgmni) + (qp - msgque);

outx:

#ifndef _POWER_MP
	unlockl(&mes_lock);
#else
	if ((key != IPC_PRIVATE) && (msgflg & IPC_CREAT))
		simple_unlock(&lock_msg_creat);
#endif
	return u.u_error ? -1 : id;

}

/*
 * NAME: msgrcv
 *
 * FUNCTION: msgrcv system call
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES: changes u.u_error
 *
 * RETURNS:
 *	number of bytes received in message if successful
 *	-1 if not successful
 */

int
msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
/* int		msqid;	*/
/* void 		*msgp;	*/
/* size_t		msgsz;	*/
/* long		msgtyp;	*/
/* int		msgflg;	*/
{
	int		rv;
	static int svcnum = 0;

	if(audit_flag && audit_svcstart("MSG_Read", &svcnum, 3, msqid, 
	U.U_uid, U.U_procp->p_pid)){
		audit_svcfinis();
	}

	TRCHKGT_SYSC(MSGRCV, msqid, msgp, msgsz, msgtyp, msgflg);
#ifndef _POWER_MP
	sysinfo.msg++;
	U.U_ru.ru_msgrcv++;
	cpuinfo[CPUID].msg++;
#else
	fetch_and_add((atomic_p) &sysinfo.msg,1);
	fetch_and_add((atomic_p) &(U.U_ru.ru_msgrcv),1);
	fetch_and_add((atomic_p) &cpuinfo[CPUID].msg,1);
#endif

#ifndef _POWER_MP
	rv = lockl(&mes_lock,LOCK_SHORT);
#endif

	rv =  rmsgrcv(msqid,msgp,msgsz,msgtyp,msgflg,0);

#ifndef _POWER_MP
	unlockl(&mes_lock);
#endif

	return(rv);
}

/*
 * NAME: msgxrcv
 *
 * FUNCTION: msgxrcv system call
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES: u.u_error changed
 *
 * RETURNS:
 *	number of bytes received in message if successful
 *	-1 if not successful
 */

int
msgxrcv(int msqid, struct msgxbuf *msgp, 
        int msgsz, mtyp_t msgtyp, int msgflg)
{
	int		rv;

	TRCHKGT_SYSC(MSGXRCV, msqid, msgp, msgsz, msgtyp, msgflg);
#ifndef _POWER_MP
	sysinfo.msg++;
	U.U_ru.ru_msgrcv++;
	cpuinfo[CPUID].msg++;
#else
	fetch_and_add((atomic_p) &sysinfo.msg,1);
	fetch_and_add((atomic_p) &(U.U_ru.ru_msgrcv),1);
	fetch_and_add((atomic_p) &cpuinfo[CPUID].msg,1);
#endif

#ifndef _POWER_MP
	rv = lockl(&mes_lock,LOCK_SHORT);
	ASSERT( rv == LOCK_SUCC );
#endif

	rv =  rmsgrcv(msqid,msgp,msgsz,msgtyp,msgflg,XMSG);

#ifndef _POWER_MP
	unlockl(&mes_lock);
#endif

	return(rv);
}

/*
 * NAME: kmsgrcv
 *
 * FUNCTION: Kernel interface to msgrcv.
 *
 * EXECUTION ENVIRONMENT:
 *	Caller must be a process
 *
 * NOTES:
 *	This interface preserves the value of u.u_error.
 *
 * RETURNS:
 *	if successful, 0 and *bytes is set to the number of bytes in message
 *	if not successful, -1 and *bytes is set to -1
 */

int
kmsgrcv(msqid, msgp, msgsz, msgtyp, msgflg, flags, bytes)
int msqid;			/* message queue id			*/
struct msgxbuf *msgp;		/* message receive buffer		*/
int msgsz;			/* size of message expected		*/
mtyp_t msgtyp;			/* message type				*/
int msgflg;			/* same as mesrcv flags			*/
int flags;			/* set to XMSG for extended message	*/
int *bytes;			/* number of bytes in message		*/
{
	int rc;
	int error_save;

	error_save = u.u_error;
	u.u_error = 0;

#ifndef _POWER_MP
	rc = lockl(&mes_lock, LOCK_SHORT);
	ASSERT( rc == LOCK_SUCC );
#endif

	*bytes = rmsgrcv(msqid, msgp, msgsz, msgtyp, msgflg,
			 flags|MSG_SYSSPACE);

#ifndef _POWER_MP
	unlockl(&mes_lock);
#endif
	
	rc = u.u_error;
	u.u_error = error_save;
	return(rc);
}

/*
 * NAME: rmsgrcv
 *
 * FUNCTION: Real msgrcv routine.
 *
 * EXECUTION ENVIRONMENT:
 *	Called from msgrcv and msgxrcv system class
 *
 * DATA STRUCTURES: u.u_error changed
 *
 * RETURNS:
 *	number of bytes received if successful
 *	-1 if not successful
 */

int
rmsgrcv(msqid, msgp, msgsz, msgtyp, msgflg, flags)
int msqid;
struct msgxbuf *msgp;
int msgsz;
mtyp_t msgtyp;
int msgflg;
int flags;
{
	register struct msg		*mp,	/* ptr to msg on q */
					*pmp,	/* ptr to mp's predecessor */
					*smp,	/* ptr to best msg on q */
					*spmp;	/* ptr to smp's predecessor */
	register struct msqid_ds	*qp;	/* ptr to associated q */
	int				sz;	/* transfer byte count */
	int				mx;	/* Set if msgxrcv */
	int				hsz;	/* sizeof message header */

BUGLPR(mdb,BUGACT,("rcv:msqid=%x,msgp=%x,sz=%d,typ=%x,msgflg=%x,flags=%d \n",
			msqid,msgp,msgsz,msgtyp,msgflg,flags));

	/*
	 * MP mode : On successful completion,
	 *           msgconv() holds the lock protecting the message queue.
	 */
	if((qp = msgconv(msqid)) == NULL) {
		BUGLPR(mdb,BUGACT,("rcv:msgconv failed, err %d\n",u.u_error));  
		return(-1);
	}
        if(ipcaccess(&qp->msg_perm, MSG_R)) {
		BUGLPR(mdb,BUGACT,("rcv:ipcaccess failed,err %d\n",u.u_error)); 
		goto err_relse;
	}
	if(msgsz < 0) {
		u.u_error = EINVAL;
		BUGLPR(mdb,BUGACT,("rcv:msgsz < 0,err %d\n",u.u_error)); 
		goto err_relse;
	}
	smp = spmp = NULL;
findmsg:
	pmp = NULL;
	mp = qp->msg_first;
	if(msgtyp == 0)
		smp = mp;
	else
		for(;mp;pmp = mp, mp = mp->msg_next) {
			if(msgtyp > 0) {
				if(msgtyp != mp->msg_attr.mtype)
					continue;
				smp = mp;
				spmp = pmp;
				break;
			}
			if(mp->msg_attr.mtype <= -msgtyp) {
				if(smp && smp->msg_attr.mtype <= mp->msg_attr.mtype)
					continue;
				smp = mp;
				spmp = pmp;
			}
		}
	if(smp) {
		if(msgsz < smp->msg_ts)
			if(!(msgflg & MSG_NOERROR)) {
				u.u_error = E2BIG;
				BUGLPR(mdb,BUGACT,("rcv:err %d\n", u.u_error)); 
				goto err_relse;
			} else
				sz = msgsz;
		else
			sz = smp->msg_ts;
		mx = flags & XMSG; 
		hsz = mx ? sizeof(struct msg_hdr) : sizeof(smp->msg_attr.mtype);

		/*
		 * copy header and text to user buf
		 */
		if (mx) {
		BUGLPR(mdb,BUGNTF,("rcv:bcopy header from %x to %x for %d \n",
			&smp->msg_attr, (caddr_t)msgp, hsz));
			if ( flags & MSG_SYSSPACE )
				bcopy( &smp->msg_attr, (caddr_t)msgp, hsz );
			else if ( copyout( &smp->msg_attr, (caddr_t)msgp,
								hsz ) ){
				u.u_error = EFAULT;
				BUGLPR(mdb,BUGACT,("rcv:err EFAULT\n") );
				goto err_relse;
			}
		} else {
		BUGLPR(mdb,BUGNTF,("msgrcv: put msg type=%x at address %x \n",
			 smp->msg_attr.mtype, *(int *)msgp));
			if ( flags & MSG_SYSSPACE )
				msgp->mtype = smp->msg_attr.mtype;
			else if ( copyout( &smp->msg_attr.mtype, 
			       (caddr_t)msgp, sizeof(smp->msg_attr.mtype) ) ){
				u.u_error = EFAULT;
				BUGLPR(mdb,BUGACT,("rcv:err EFAULT\n") );
				goto err_relse;
			}
		}
		BUGLPR(mdb,BUGNTF,("msgrcv: bcopy text from %x to %x for %d \n",
		smp->msg_spot, (caddr_t)msgp+hsz,sz));

		if ( flags & MSG_SYSSPACE )
			bcopy(smp->msg_spot, (caddr_t)msgp + hsz, sz );
		else if ( copyout(smp->msg_spot, (caddr_t)msgp + hsz, sz ) ){
			u.u_error = EFAULT;
			BUGLPR(mdb,BUGACT,("rcv:err EFAULT\n") );
			goto err_relse;
		}

		/* update message queue */
		qp->msg_cbytes -= smp->msg_ts;
		qp->msg_lrpid = U.U_procp->p_pid;
		qp->msg_rtime = time;
		msgfree(qp, spmp, smp, msqid);
#ifdef _POWER_MP
		/* Cast to unsigned int is necessary while msgconv() does so */
		simple_unlock(&lock_msg_qid[((uint_t) msqid) % NB_MSG_LOCKS]);
#endif
		return(sz);
	}
	if(msgflg & IPC_NOWAIT) {
		u.u_error = ENOMSG;
		BUGLPR(mdb,BUGACT,("rcv:err %d\n", u.u_error)); 
		goto err_relse;
	}
	qp->msg_perm.mode |= MSG_RWAIT;

	BUGLPR(mdb,BUGACT,("call sleep(&qp->msg_qnum %x, PMSG|PCATCH %d)\n",
				&qp->msg_qnum, PMSG|PCATCH));
	if(msgsleep(&qp->msg_rwait, qp)) {
		BUGLPR(mdb,BUGACT,("rcv:err %d\n:", u.u_error));
#ifdef _POWER_MP
		/* Cast to unsigned int is necessary while msgconv() does so */
		simple_unlock(&lock_msg_qid[((uint_t) msqid) % NB_MSG_LOCKS]);
#endif
		return(-1);
	}
	goto findmsg;
err_relse:
	BUGLPR(mdb,BUGACT,("rcv:err_relse, err %d\n",u.u_error));  
#ifdef _POWER_MP
	/* Cast to unsigned int is necessary while msgconv() does so */
	simple_unlock(&lock_msg_qid[((uint_t) msqid) % NB_MSG_LOCKS]);
#endif
	return(-1);
}

/*
 * NAME: rmsgsnd
 *
 * FUNCTION: Real msgsnd routine.
 *
 * EXECUTION ENVIRONMENT:
 *	Called from msgsnd system call
 *
 * NOTES:
 *	Arguments are as for the  system call, with the addition of flags,
 *	which specifies the address space
 *
 * DATA STRUCTURES: Sets u.u_error on error;
 *
 * RETURNS: NULL if no wakeup is needed, event list if wakeup is needed
 */

void
rmsgsnd(msqid, msgp, cnt, msgflg, flags)
int msqid;
struct msgbuf *msgp;
register int cnt;
int msgflg;
int flags;
{
        register struct msqid_ds        *qp;    /* ptr to associated q */
        register struct msg             *mp;    /* ptr to allocated msg hdr */
        register char                   *spot;  /* ptr to allocated msg spc */
        mtyp_t                          type;
	int				rv;

BUGLPR(mdb,BUGACT,("msgsnd: msqid=%x,msgp=%x,cnt=%d, msgflg=%x, flags=%d \n",
		msqid, msgp, cnt, msgflg, flags));

	/*
	 * MP mode : On successful completion,
	 *           msgconv() holds the lock protecting the message queue.
	 */
        if((qp = msgconv(msqid)) == NULL)
                return;
        if(ipcaccess(&qp->msg_perm, MSG_W)) {
		goto outx;
        }
        if(cnt < 0 || cnt > msginfo.msgmax) {
                u.u_error = EINVAL;
		goto outx;
        }
	/*
	 * check for all resources to be available;
	 * sleep if necessary waiting on resources;
	 * then loop back to check again
	 */
getres:
	/* Allocate space on q, message header, & buffer space. */

	/*
	 * check if maximium number of bytes on queue or maximium number
	 * of messages has been reached
	 */
	if(		(qp->msg_qnum >= msginfo.msgmnm) || 
			(cnt + qp->msg_cbytes > qp->msg_qbytes) ) {
		if (qp->msg_qbytes < cnt) {
			u.u_error = EINVAL;
			goto outx;
		}
		if(msgflg & IPC_NOWAIT) {
			u.u_error = EAGAIN;
			goto outx;
		}
		qp->msg_perm.mode |= MSG_WWAIT;

		if ( msgsleep(&qp->msg_wwait,qp) ) {
			goto outx;
		}
		else
			goto getres;
	}
	if ( (mp = xmalloc(sizeof(struct msg)+cnt, 2, kernel_heap)) == NULL ) {
		BUGPR(("xmalloc failed\n"));
		u.u_error = ENOMEM;
		goto outx;
	}
	if (cnt)
		spot = (char *)mp + sizeof(struct msg);
	else
		spot = NULL;

	if ( flags & MSG_SYSSPACE )
		mp->msg_attr.mtype = msgp->mtype;
	else if ( copyin( &msgp->mtype , &(mp->msg_attr.mtype) ,
			sizeof(msgp->mtype) )  ){
		u.u_error = EFAULT;
		goto out_error;
	}
	BUGLPR(mdb,BUGNTF,("mp %x, mp->msg_attr.mtype %d\n", \
		mp,mp->msg_attr.mtype));

	if ( mp->msg_attr.mtype < 1 ) {
		u.u_error = EINVAL;
		goto out_error;
	}

	/* put message text on message queue*/
	BUGLPR(mdb,BUGNTF,("msgsnd: bcopy from %x to %x for %d bytes\n",
		(caddr_t)msgp+sizeof(long), spot, cnt));

	if ( flags & MSG_SYSSPACE )
		bcopy((caddr_t)msgp + sizeof(long), spot , cnt);
	else if ( copyin((caddr_t)msgp + sizeof(long), spot , cnt) ){
		u.u_error = EFAULT;
		goto out_error;
	}
	/* update message queue header and message header */
        qp->msg_qnum++;
        qp->msg_cbytes += cnt;
        qp->msg_stime = time;
        mp->msg_next = NULL;
       	mp->msg_attr.mtime = time;
	mp->msg_attr.muid = getuidx(ID_EFFECTIVE);
	mp->msg_attr.mgid = getgidx(ID_EFFECTIVE);
	mp->msg_attr.mpid = U.U_procp->p_pid;
        qp->msg_lspid = U.U_procp->p_pid;
        mp->msg_ts = cnt;
        mp->msg_spot = cnt ? spot : NULL;
        if(qp->msg_last == NULL)
                qp->msg_first = qp->msg_last = mp;
        else {
                qp->msg_last->msg_next = mp;
                qp->msg_last = mp;
        }

	/* wakeup any process waiting on receive select */
        if(qp->msg_reqevents & POLLIN) {
                selnotify(POLL_MSG, msqid, POLLIN);
                qp->msg_reqevents &= ~POLLIN;
        }

	/* if wakeup is needed return event list else return NULL */
	if(qp->msg_perm.mode & MSG_RWAIT) {
		qp->msg_perm.mode &= ~MSG_RWAIT;
		e_wakeupx(&qp->msg_rwait, E_WKX_NO_PREEMPT);
	}
	goto outx;

out_error:
	rv = xmfree(mp, kernel_heap);
	ASSERT( rv == 0 );
outx:
#ifdef _POWER_MP
	/* Cast to unsigned int is necessary while msgconv() does so */
	simple_unlock(&lock_msg_qid[((uint_t) msqid) % NB_MSG_LOCKS]);
#endif
	return;

}

/*
 * NAME: kmsgsnd
 *
 * FUNCTION: Kernel interface to msgsnd
 *
 * EXECUTION ENVIRONMENT:
 *	Caller must be a process
 *
 * NOTES:
 *	This interface preserves u.u_error
 *
 * RETURNS:
 *	0 if successful
 *	non zero errno value if not successful
 */

int
kmsgsnd(msqid, msgp, msgsz, msgflg)
int msqid;			/* message queue id		*/
struct msgbuf *msgp;		/* message to send		*/
int  msgsz;			/* number of bytes in message	*/
int msgflg;			/* flags for msgsnd		*/
{
	int rc;
	int error_save;

	error_save = u.u_error;
	u.u_error = 0;

#ifndef _POWER_MP
	rc = lockl(&mes_lock, LOCK_SHORT);
	ASSERT( rc == LOCK_SUCC );
#endif

	rmsgsnd(msqid, msgp, msgsz, msgflg, MSG_SYSSPACE);

#ifndef _POWER_MP
	unlockl(&mes_lock);
#endif

	rc = u.u_error;
	u.u_error = error_save;
	return(rc);
}

/*
 * NAME: msgsnd
 *
 * FUNCTION: msgsnd system call
 *
 * EXECUTION ENVIRONMENT:
 *	system call
 *
 * DATA STRUCTURES: u.u_error changed
 *
 * RETURNS: 0 if successful	-1 if not successful
 */

int
msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
/* int		msqid;	*/
/* void		*msgp;	*/
/* size_t		msgsz;	*/
/* int		msgflg;	*/
{
	int		rv;
	static int svcnum = 0;

	if(audit_flag && audit_svcstart("MSG_Write", &svcnum, 1, msqid)){
		audit_svcfinis();
	}

	TRCHKGT_SYSC(MSGSND, msqid, msgp, msgsz, msgflg, NULL);
#ifndef _POWER_MP
	sysinfo.msg++;
	U.U_ru.ru_msgsnd++;
	cpuinfo[CPUID].msg++;
#else
	fetch_and_add((atomic_p) &sysinfo.msg,1);
	fetch_and_add((atomic_p) &(U.U_ru.ru_msgsnd),1);
	fetch_and_add((atomic_p) &cpuinfo[CPUID].msg,1);
#endif

#ifndef _POWER_MP
	rv = lockl(&mes_lock,LOCK_SHORT);
	ASSERT( rv == LOCK_SUCC );
#endif

	rmsgsnd(msqid, msgp, msgsz, msgflg, 0);

#ifndef _POWER_MP
	unlockl(&mes_lock);
#endif

	return u.u_error ? -1 : 0;
}


/*
 * NAME: msgsleep
 *
 * FUNCTION: wait on message resources
 *
 * EXECUTION ENVIRONMENT:
 *	Called during msgsnd system call
 *
 * DATA STRUCTURES: Sets u.u_error
 *
 * RETURNS:
 *	0 if resource is free
 *	-1 if message queue was removed or signal received
 *
 * MP MODIFICATION:
 *	Replace the call to e_sleepl by a call to e_sleep_thread, with the
 *         appropriate lock. Modify the test of the return code, too.
 */

int
msgsleep(wait_list, qp)
int	*wait_list;
struct	msqid_ds *qp;		/* message queue requiring resource */
{
	int	sleeprtn;
	unsigned short	seq;
#ifdef _POWER_MP
	int	msgid;
#endif

	seq = qp->msg_perm.seq;
#ifndef _POWER_MP
	sleeprtn = e_sleepl(&mes_lock, wait_list, EVENT_SIGRET);
	if ( sleeprtn == EVENT_SIG )
#else
	msgid = (seq * msginfo.msgmni) + (qp - msgque);
	sleeprtn = e_sleep_thread(wait_list,
			&lock_msg_qid[msgid % NB_MSG_LOCKS],
			LOCK_SIMPLE|INTERRUPTIBLE);
	if ( sleeprtn == THREAD_INTERRUPTED )
#endif
	{	u.u_error = EINTR;
		return (-1);
	}
	else
	{	if ( (qp->msg_perm.mode & IPC_ALLOC) == 0 ||
			(seq != qp->msg_perm.seq) )
		{	u.u_error = EIDRM;
			return (-1);
		}
		else
			return (0);
	}
}

/*
 * NAME: msgselect
 *
 * FUNCTION: Msgselect function from Berkeley
 *
 */

int
msgselect(msg_id, corl, reqevents, rtneventsp)
int msg_id;
int corl;
ushort reqevents;
ushort *rtneventsp;
{       register int	rc = 0;
        register int	s;
        struct  msqid_ds *msgptr;
	int		rv;

	/*
	 * lock message services
	 */
#ifndef _POWER_MP
	rv = lockl(&mes_lock, LOCK_SHORT);
	ASSERT( rv == LOCK_SUCC );
#endif

	/*
	 * Check if id no longer valid. If not, mark all events.
	 */

	/*
	 * MP mode : On successful completion,
	 *           msgconv() holds the lock protecting the message queue.
	 */
        if ((msgptr = msgconv(msg_id)) == NULL)
	{
		*rtneventsp |= reqevents;
		rc = EINVAL;
	}
	else
	{

		/*
		 * Check for data available on queue.
		 */
		if ((reqevents & POLLIN) && (msgptr->msg_qnum != 0))
		{
			*rtneventsp |= POLLIN;
		}

		/*
		 * Check room for more data on queue.
		 */
		if ((reqevents & POLLOUT) &&
			(msgptr->msg_cbytes < msgptr->msg_qbytes))
		{
			*rtneventsp |= POLLOUT;
		}

		/*
		 * If no supported event requested, register none.
		 */
		if (!(reqevents & (POLLIN | POLLOUT | POLLPRI)))
		{
			reqevents |= POLLSYNC;
		}

		/*
		 * If not a synchronous poll request and if no
		 * supported events were reported then remember
		 * pending request.
		 */
		if (!(reqevents & POLLSYNC) && !(*rtneventsp))
		{
			rc = selreg(corl, POLL_MSG, msg_id, reqevents, NULL);
			if (rc == 0)
			{
				msgptr->msg_reqevents |= reqevents & ~POLLSYNC;
			}
		}
#ifdef _POWER_MP
		/* Cast to unsigned int is necessary while msgconv() does so */
		simple_unlock(&lock_msg_qid[((uint_t) msg_id) % NB_MSG_LOCKS]);
#endif
	}

#ifndef _POWER_MP
	unlockl( &mes_lock );
#endif
        return (rc);
}
