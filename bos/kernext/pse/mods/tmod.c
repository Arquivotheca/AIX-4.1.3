static char sccsid[] = "@(#)47        1.5  src/bos/kernext/pse/mods/tmod.c, sysxpse, bos411, 9428A410j 8/27/93 09:43:11";
/*
 *   COMPONENT_NAME: SYSXPSE
 *
 *   ORIGINS: 27 63 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990, 1991  Mentat Inc.
 ** tmod.c 2.4, last change 4/9/91
 **/

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#include <sys/stream.h>
#include <sys/strlog.h>
#include <sys/stropts.h>	/**/
#include <pse/common.h>
#include <pse/tmod.h>

staticf	void	tmod_adjmsg(   queue_t * q   );
staticf	int	tmod_admin(   void   );
staticf	void	tmod_bufcall(   queue_t * q   );
staticf	int	tmod_bufcall_check(   long arg   );
staticf	void	tmod_canput(   queue_t * q   );
staticf	void	tmod_copy(   queue_t * q   );
staticf	void	tmod_dup(   queue_t * q   );
staticf int	tmod_close(   queue_t * q   );
staticf	void	tmod_esballoc(   queue_t * q   );
staticf	void	tmod_esballoc_free(   char * arg, char * datap   );
staticf	void	tmod_flush(   queue_t * q   );
staticf	void	tmod_getadmin(   queue_t * q   );
staticf	void	tmod_getmid(   queue_t * q   );
staticf	void	tmod_getq_putq(   queue_t * q   );
staticf	void	tmod_insq(   queue_t * q   );
staticf	void	tmod_linkb(   queue_t * q   );
staticf int	tmod_open(   queue_t * q, dev_t * devp, int flag, int sflag, cred_t * credp   );
staticf	void	tmod_pullupmsg(   queue_t * q   );
staticf	void	tmod_putbq(   queue_t * q   );
staticf	void	tmod_qenable(   queue_t * q   );
staticf	void	tmod_rmvb(   queue_t * q   );
staticf	void	tmod_rmvq(   queue_t * q   );
staticf void	tmod_rput(   queue_t * q, MBLKP mp   );
staticf void	tmod_rsrv(   queue_t * q   );
staticf	void	tmod_strlog(   queue_t * q   );
staticf	void	tmod_strqget(   queue_t * q   );
staticf	void	tmod_strqset(   queue_t * q   );
staticf void	tmod_wput(   queue_t * q, MBLKP mp   );
staticf void	tmod_wsrv(   queue_t * q   );

static struct module_info minfo =  {
#define	MODULE_ID	5004
	MODULE_ID, "tmod", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	(pfi_t)tmod_rput, (pfi_t)tmod_rsrv, tmod_open, tmod_close, tmod_admin, &minfo
};

static struct qinit winit = {
	(pfi_t)tmod_wput, (pfi_t)tmod_wsrv, nil(pfi_t), nil(pfi_t), nil(pfi_t), &minfo
};

struct streamtab tmodinfo = { &rinit, &winit };

	int	tmoddevflag = 0;

static	char	tmod_g_buf[128];

staticf void
tmod_adjmsg (q)
	queue_t	* q;
{
	int	i1;
	MBLKP	mp;
	MBLKP	mp1;
	unsigned char	* rptr;
	unsigned char	* wptr;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing adjmsg");
	
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: allocb failed");
		return;
	}
	mp->b_wptr += 60;
	
	/* Simple positive bytes */
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying simple adjmsg to trim bytes from front of a single message block");
	rptr = mp->b_rptr + 10;
	if (!adjmsg(mp, 10)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg failed");
	} else {
		if (mp->b_rptr == rptr
		&& (mp->b_wptr - mp->b_rptr) == 50)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: adjmsg successful");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg moved rptr incorrectly (0x%X should be 0x%X)",
				mp->b_rptr, rptr);
	}

	/* Simple negative bytes */
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + 60;
	wptr = mp->b_wptr - 10;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying simple adjmsg to trim bytes from end of a single message block");
	if (!adjmsg(mp, -10)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg failed");
	} else {
		if (mp->b_wptr == wptr
		&& (mp->b_wptr - mp->b_rptr) == 50)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: adjmsg successful");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg moved wptr incorrectly (0x%X should be 0x%X)",
				mp->b_wptr, wptr);
	}
	
	/* Allocate more message blocks for the other tests */
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + 60;
	if (!(mp1 = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: allocb failed");
		freemsg(mp);
		return;
	}
	mp1->b_wptr += 42;
	linkb(mp, mp1);

	if (!(mp1 = allocb(500, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: allocb failed");
		freemsg(mp);
		return;
	}
	mp1->b_wptr += 306;
	linkb(mp, mp1);

	if (!(mp1 = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: allocb failed");
		freemsg(mp);
		return;
	}
	mp1->b_wptr += 12;
	linkb(mp, mp1);

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying positive adjmsg across different message types");
	mp->b_cont->b_datap->db_type = M_PROTO;
	if (adjmsg(mp, 250)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg successful");
		freemsg(mp);
		return;
	}
	/* Switch the message type back to M_DATA so it will be counted by msgdsize */
	mp->b_cont->b_datap->db_type = M_DATA;
	i1 = msgdsize(mp);
	if (i1 != 420) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed the message (size %d should be 420)", i1);
		freemsg(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: adjmsg failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying negative adjmsg across different message types");
	mp->b_cont->b_datap->db_type = M_PROTO;
	if (adjmsg(mp, -400)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg successful");
		freemsg(mp);
		return;
	}
	/* Switch the message type back to M_DATA so it will be counted by msgdsize */
	mp->b_cont->b_datap->db_type = M_DATA;
	i1 = msgdsize(mp);
	if (i1 != 420) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed the message (size %d should be 420)", i1);
		freemsg(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: adjmsg failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying adjmsg to trim bytes from front of multiple message blocks");
	mp->b_cont->b_datap->db_type = M_DATA;
	if (!adjmsg(mp, 200)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg failed");
	} else {
		i1 = msgdsize(mp);
		if (i1 == 220) {
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: adjmsg successful with 220 bytes left in message");
			if (mp->b_wptr != mp->b_rptr)
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed first block incorrectly");
			mp1 = mp->b_cont;
			if (mp1->b_wptr != mp1->b_rptr)
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed second block incorrectly");
			mp1 = mp1->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[208])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed third block incorrectly");
			mp1 = mp1->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[12])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed fourth block incorrectly");
		} else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg trimmed the message incorrectly (%d bytes, should be %d)",
				i1, 220);
	}
	
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying adjmsg to trim bytes from end of multiple message blocks");
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + 60;
	mp1 = mp->b_cont;
	mp1->b_rptr = mp1->b_datap->db_base;
	mp1->b_wptr = mp1->b_rptr + 42;
	mp1 = mp1->b_cont;
	mp1->b_rptr = mp1->b_datap->db_base;
	mp1->b_wptr = mp1->b_rptr + 306;
	if (!adjmsg(mp, -200)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg failed");
	} else {
		i1 = msgdsize(mp);
		if (i1 == 220) {
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: adjmsg successful with 220 bytes left in message");
			if (mp->b_wptr != &mp->b_rptr[60])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed first block incorrectly");
			mp1 = mp->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[42])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed second block incorrectly");
			mp1 = mp1->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[118])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed third block incorrectly");
			mp1 = mp1->b_cont;
			if (mp1->b_wptr != mp1->b_rptr)
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed fourth block incorrectly");
		} else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg trimmed the message incorrectly (%d bytes, should be %d)",
				i1, 220);
	}
	
	/* Zero byte adjust */
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying adjmsg with 0 adjust");
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + 60;
	mp1 = mp->b_cont;
	mp1->b_rptr = mp1->b_datap->db_base;
	mp1->b_wptr = mp1->b_rptr + 42;
	mp1 = mp1->b_cont;
	mp1->b_rptr = mp1->b_datap->db_base;
	mp1->b_wptr = mp1->b_rptr + 306;
	mp1 = mp1->b_cont;
	mp1->b_rptr = mp1->b_datap->db_base;
	mp1->b_wptr = mp1->b_rptr + 12;
	if (adjmsg(mp, 0)) {
		i1 = msgdsize(mp);
		if (i1 == 420) {
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: adjmsg failed");
			if (mp->b_wptr != &mp->b_rptr[60])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed first block incorrectly");
			mp1 = mp->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[42])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed second block incorrectly");
			mp1 = mp1->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[306])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed third block incorrectly");
			mp1 = mp1->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[12])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed fourth block incorrectly");
		} else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: adjmsg failed, but size of message changed (%d should be %d)",
				i1, 420);
	} else {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg successful");
		freemsg(mp);
		return;
	}

	/* Positive bytes with not enough data in message */
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying adjmsg with positive adjust greater than bytes in message");
	if (!adjmsg(mp, 1000)) {
		i1 = msgdsize(mp);
		if (i1 == 420) {
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: adjmsg failed");
			if (mp->b_wptr != &mp->b_rptr[60])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed first block incorrectly");
			mp1 = mp->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[42])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed second block incorrectly");
			mp1 = mp1->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[306])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed third block incorrectly");
			mp1 = mp1->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[12])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed fourth block incorrectly");
		} else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: adjmsg failed, but size of message changed (%d should be %d)",
				i1, 420);
	} else {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg successful");
		freemsg(mp);
		return;
	}

	/* Negative bytes with not enough data in message */
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying adjmsg with negative adjust greater than bytes in message");
	if (!adjmsg(mp, -1000)) {
		i1 = msgdsize(mp);
		if (i1 == 420) {
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: adjmsg failed");
			if (mp->b_wptr != &mp->b_rptr[60])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed first block incorrectly");
			mp1 = mp->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[42])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed second block incorrectly");
			mp1 = mp1->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[306])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed third block incorrectly");
			mp1 = mp1->b_cont;
			if (mp1->b_wptr != &mp1->b_rptr[12])
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg changed fourth block incorrectly");
		} else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: adjmsg failed, but size of message changed (%d should be %d)",
				i1, 420);
	} else {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: adjmsg successful");
		freemsg(mp);
		return;
	}

	freemsg(mp);
	return;
}

staticf int
tmod_admin () {
	return 0;
}

staticf void
tmod_bufcall (q)
	queue_t	* q;
{
	int	id;
	MBLKP	mp;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing bufcall");
	if (mp = allocb(64, BPRI_HI)) {
		if (id = bufcall(64, BPRI_HI, tmod_bufcall_check, (long)1)) {
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: bufcall returned id %d", id);
			unbufcall(id);
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: unbufcall called, now freeing message");
			freeb(mp);
		} else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: bufcall failed, id %d", id);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: couldn't allocate 64 byte message");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing testb");
	if (!testb(64, BPRI_HI))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: testb failed on 64 bytes");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testb successful");
}

staticf int
tmod_bufcall_check (arg)
	long	arg;
{
	strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: tmod_bufcall_check called with arg %ld", arg);
	return 0;
}

staticf int
tmod_close (q)
	queue_t	* q;
{
	return 0;
}

staticf void
tmod_canput (q)
	queue_t	* q;
{
	int	i1;
	MBLKP	mp;
	unsigned char	pri;

	pri = q->q_nband+5;
	if (bcanput(q, pri))
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: bcanput succeeded on non-existent band");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: bcanput failed on non-existent band");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: creating band %d and setting the high water mark to 64", pri);
	i1 = strqset(q, QHIWAT, pri, (long)64);
	if (i1 == 0  &&  q->q_nband == pri) {
		if (mp = allocb(64, BPRI_HI)) {
			mp->b_band = pri;
			mp->b_wptr += 64;
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ing a message to band %d", pri);
			if (putq(q, mp)) {
				if (bcanput(q, pri))
					strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: bcanput succeeded on full band");
				else
					strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: bcanput failed on full band");
				if (bcanput(q, 0))
					strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: bcanput succeeded on band 0");
				else
					strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: bcanput failed on band 0");
				strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: getq'ing a message from band %d", pri);
				if (mp = getq(q))
					freemsg(mp);
				else
					strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: getq didn't return a message");
				if (bcanput(q, pri))
					strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: bcanput succeeded on empty band");
				else
					strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: bcanput failed on empty band");
			} else {
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
				freemsg(mp);
			}
		} else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset returned %d, q_nband now %d", i1, q->q_nband);

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing canput");
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: setting the queue high water mark to 64", pri);
	i1 = strqset(q, QHIWAT, 0, (long)64);
	if (i1 == 0) {
		if (mp = allocb(64, BPRI_HI)) {
			mp->b_wptr += 64;
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ing a message");
			if (putq(q, mp)) {
				if (canput(q))
					strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: canput succeeded on full queue");
				else
					strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: canput failed on full queue");

				strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: getq'ing a message");
				if (mp = getq(q))
					freemsg(mp);
				else
					strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: getq didn't return a message");

				if (canput(q))
					strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: canput succeeded on empty queue");
				else
					strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: canput failed on empty queue");
			} else {
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
				freemsg(mp);
			}
		} else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset returned %d", i1);
}

staticf void
tmod_copy (q)
	queue_t	* q;
{
	unsigned char	* cp;
	MBLKP	mp1;
	MBLKP	mp2;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing copyb and copymsg");

	if (!(mp1 = allocb(300, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp1->b_datap->db_type = M_PROTO;
	mp1->b_wptr += 250;
	for (cp = mp1->b_rptr; cp < mp1->b_wptr; cp++)
		*cp = 'Z';

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling copyb with single message block");
	if (mp2 = copyb(mp1)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: copyb successful");
		if (mp2->b_datap->db_ref != 1)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in copied message", mp2->b_datap->db_ref);
		if (mp2->b_datap->db_type != M_PROTO)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copyb did not copy message type");
		if (mp2->b_wptr - mp2->b_rptr != 250)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copyb did not copy correct number of bytes (%d should be 50)",
				mp2->b_wptr - mp2->b_rptr);
		for (cp = mp2->b_rptr; cp < mp2->b_wptr; cp++) {
			if (*cp != 'Z') {
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copyb did not copy correctly");
				break;
			}
		}
		freeb(mp2);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copyb failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling copymsg with single message block");
	if (mp2 = copymsg(mp1)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: copymsg successful");
		if (mp2->b_datap->db_ref != 1)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in copied message", mp2->b_datap->db_ref);
		if (mp2->b_datap->db_type != M_PROTO)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copymsg did not copy message type");
		if (mp2->b_wptr - mp2->b_rptr != 250)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copymsg did not copy correct number of bytes (%d should be 50)",
				mp2->b_wptr - mp2->b_rptr);
		for (cp = mp2->b_rptr; cp < mp2->b_wptr; cp++) {
			if (*cp != 'Z') {
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copymsg did not copy correctly");
				break;
			}
		}
		freeb(mp2);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copymsg failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling copyb with multiple message block");
	if (!(mp1->b_cont = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp1);
		return;
	}
	mp1->b_cont->b_wptr += 41;
	for (cp = mp1->b_cont->b_rptr; cp < mp1->b_cont->b_wptr; cp++)
		*cp = 'e';
	if (mp2 = copyb(mp1)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: copyb successful");
		if (mp2->b_datap->db_ref != 1)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in copied message", mp2->b_datap->db_ref);
		if (mp2->b_cont)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: b_cont of copied block is not NULL");
		if (mp2->b_datap->db_type != M_PROTO)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copyb did not copy message type");
		if (mp2->b_wptr - mp2->b_rptr != 250)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copyb did not copy correct number of bytes (%d should be 50)",
				mp2->b_wptr - mp2->b_rptr);
		for (cp = mp2->b_rptr; cp < mp2->b_wptr; cp++) {
			if (*cp != 'Z') {
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copyb did not copy correctly");
				break;
			}
		}
		freeb(mp2);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copyb failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling copymsg with multiple message blocks");
	if (mp2 = copymsg(mp1)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: copymsg successful");
		if (mp1->b_datap->db_ref != 1)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in message", mp1->b_datap->db_ref);
		if (mp2->b_datap->db_ref != 1)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in copied message", mp2->b_datap->db_ref);
		if (!mp2->b_cont)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: b_cont of copied block is NULL");
		if (mp2->b_datap->db_type != M_PROTO)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copymsg did not copy message type");
		if (mp2->b_wptr - mp2->b_rptr != 250)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copymsg did not copy correct number of bytes (%d should be 50)",
				mp2->b_wptr - mp2->b_rptr);
		for (cp = mp2->b_rptr; cp < mp2->b_wptr; cp++) {
			if (*cp != 'Z') {
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copymsg did not copy correctly");
				break;
			}
		}
		if (mp2->b_cont) {
			if (mp2->b_cont->b_wptr - mp2->b_cont->b_rptr != 41)
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copymsg did not copy correct number of bytes in 2nd block (%d should be 50)",
					mp2->b_cont->b_wptr - mp2->b_cont->b_rptr);
			for (cp = mp2->b_cont->b_rptr; cp < mp2->b_cont->b_wptr; cp++) {
				if (*cp != 'e') {
					strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copymsg did not copy 2nd block correctly");
					break;
				}
			}
		}
		freemsg(mp2);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: copymsg failed");
	freemsg(mp1);
}

staticf void
tmod_dup (q)
	queue_t	* q;
{
	unsigned char	* cp;
	MBLKP	mp1;
	MBLKP	mp2;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing dupb and dupmsg");

	if (!(mp1 = allocb(300, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp1->b_datap->db_type = M_PROTO;
	mp1->b_wptr += 250;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling dupb with single message block");
	if (mp2 = dupb(mp1)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: dupb successful");
		if (mp1->b_datap->db_ref != 2)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in message", mp1->b_datap->db_ref);
		if (mp2->b_datap->db_ref != 2)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in dup'ed message", mp2->b_datap->db_ref);
		if (mp2->b_datap != mp1->b_datap)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: datap pointers are not the same");
		if (mp2->b_wptr - mp2->b_rptr != 250)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: dupb did not dup correct number of bytes (%d should be 50)",
				mp2->b_wptr - mp2->b_rptr);
		freeb(mp2);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: dupb failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling dupmsg with single message block");
	if (mp2 = dupmsg(mp1)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: dupmsg successful");
		if (mp1->b_datap->db_ref != 2)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in message", mp1->b_datap->db_ref);
		if (mp2->b_datap->db_ref != 2)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in dup'ed message", mp2->b_datap->db_ref);
		if (mp2->b_wptr - mp2->b_rptr != 250)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: dupmsg did not dup correct number of bytes (%d should be 50)",
				mp2->b_wptr - mp2->b_rptr);
		freeb(mp2);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: dupmsg failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling dupb with multiple message block");
	if (!(mp1->b_cont = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp1);
		return;
	}
	mp1->b_cont->b_wptr += 41;
	if (mp2 = dupb(mp1)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: dupb successful");
		if (mp1->b_datap->db_ref != 2)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in message", mp1->b_datap->db_ref);
		if (mp2->b_datap->db_ref != 2)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in dup'ed message", mp2->b_datap->db_ref);
		if (mp2->b_cont)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: b_cont of dup'ed block is not NULL");
		if (mp2->b_wptr - mp2->b_rptr != 250)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: dupb did not dup correct number of bytes (%d should be 50)",
				mp2->b_wptr - mp2->b_rptr);
		freeb(mp2);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: dupb failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling dupmsg with multiple message blocks");
	if (mp2 = dupmsg(mp1)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: dupmsg successful");
		if (mp1->b_datap->db_ref != 2)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in message", mp1->b_datap->db_ref);
		if (mp2->b_datap->db_ref != 2)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in dup'ed message", mp2->b_datap->db_ref);
		if (mp1->b_cont->b_datap->db_ref != 2)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in 2nd block of message", mp1->b_cont->b_datap->db_ref);
		if (mp2->b_wptr - mp2->b_rptr != 250)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: dupmsg did not dup correct number of bytes (%d should be 50)",
				mp2->b_wptr - mp2->b_rptr);
		if (mp2->b_cont) {
			if (mp2->b_cont->b_datap->db_ref != 2)
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ref is %d in 2nd block of dup'ed message", mp2->b_cont->b_datap->db_ref);
			if (mp2->b_cont->b_wptr - mp2->b_cont->b_rptr != 41)
				strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: dupmsg did not dup correct number of bytes in 2nd block (%d should be 50)",
					mp2->b_cont->b_wptr - mp2->b_cont->b_rptr);
		} else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: b_cont of dup'ed block is NULL");
		freemsg(mp2);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: dupmsg failed");
	freemsg(mp1);
}

staticf void
tmod_esballoc (q)
	queue_t	* q;
{
	frtn_t	frtn;
	MBLKP	mp;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing esballoc");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing esballoc with nil free function");
	frtn.free_func = nil(pfv_t);
	frtn.free_arg = tmod_g_buf;
	if (mp = esballoc((unsigned char *)tmod_g_buf, sizeof(tmod_g_buf), BPRI_HI, &frtn)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: esballoc succeeded");
		freeb(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: esballoc failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing esballoc with nil base");
	frtn.free_func = tmod_esballoc_free;
	frtn.free_arg = (char *)&tmodinfo;
	if (mp = esballoc(nilp(unsigned char), sizeof(tmod_g_buf), BPRI_HI, &frtn)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: esballoc succeeded");
		freeb(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: esballoc failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing esballoc with nil return structure");
	if (mp = esballoc((unsigned char *)tmod_g_buf, sizeof(tmod_g_buf), BPRI_HI, nilp(frtn_t))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: esballoc succeeded");
		freeb(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: esballoc failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing valid esballoc");
	frtn.free_func = tmod_esballoc_free;
	frtn.free_arg = (char *)&tmodinfo;
	if (mp = esballoc((unsigned char *)tmod_g_buf, sizeof(tmod_g_buf), BPRI_HI, &frtn)) {
		if (mp->b_datap->db_base == (unsigned char *)&tmod_g_buf[0]
		&&  mp->b_datap->db_lim == (unsigned char *)&tmod_g_buf[sizeof(tmod_g_buf)]
		&&  mp->b_rptr == (unsigned char *)&tmod_g_buf[0]
		&&  mp->b_wptr == (unsigned char *)&tmod_g_buf[0]
		&&  mp->b_datap->db_frtnp == &frtn)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: esballoc succeeded");
		else {
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: esballoc succeeded, but the message is not initialized correctly");
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: buffer 0x%x, size %d, db_base 0x%x, db_lim 0x%x",
				tmod_g_buf, sizeof(tmod_g_buf), mp->b_datap->db_base, mp->b_datap->db_lim);
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: b_rptr 0x%x, b_wptr 0x%x, db_freep 0x%x (should be 0x%x)",
				mp->b_rptr, mp->b_wptr, mp->b_datap->db_freep, &frtn);
		}
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: freeing the message");
		freeb(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: esballoc failed");
}

staticf void
tmod_esballoc_free (arg, datap)
	char	* arg;
	char	* datap;
{
	if (arg == (char *)&tmodinfo)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: tmod_esballoc_free called with correct arg");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: tmod_esballoc_free called with arg 0x%x (should be 0x%x)", arg, &tmodinfo);
	if (datap == tmod_g_buf)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: tmod_esballoc_free called with correct data pointer");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: tmod_esballoc_free called with data pointer 0x%x (should be 0x%x)", datap, tmod_g_buf);
}

staticf void
tmod_flush (q)
	queue_t	* q;
{
	int	i1;
	unsigned char	pri;
	qband_t	* qb;
	MBLKP	mp;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing flushband");

	pri = q->q_nband+5;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing flushband on band %d", pri);
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_band = pri;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ing a M_DATA message on band %d", pri);
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_datap->db_type = M_CTL;
	mp->b_band = pri;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ing a M_CTL message on band %d", pri);
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}

	/* Find the band */
	for (i1 = 1, qb = q->q_bandp; i1 < pri; i1++)
		qb = qb->qb_next;
	if (qb->qb_first) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling flushband with band and FLUSHDATA");
		flushband(q, pri, FLUSHDATA);
		if (qb->qb_first != mp)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: flushband removed M_CTL message");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: flushband succeeded");

		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling flushband with band and FLUSHALL");
		flushband(q, pri, FLUSHALL);
		if (qb->qb_first)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: flushband didn't clear the qband");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: flushband succeeded");
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: no message on the band after putq");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing flushband on band 0");
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ing on band 0");
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}
	flushband(q, 0, FLUSHALL);
	if (q->q_first)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: flushband didn't clear the q");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: flushband succeeded");

	/* flushq tests start here. */
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing flushq");
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ing a M_DATA message");
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_datap->db_type = M_CTL;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ing a M_CTL message");
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}

	if (q->q_first) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling flushq with FLUSHDATA");
		flushq(q, FLUSHDATA);
		if (q->q_first != mp)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: flushq removed M_CTL message");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: flushq succeeded");

		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling flushq with FLUSHALL");
		flushq(q, FLUSHALL);
		if (q->q_first)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: flushq didn't clear the queue");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: flushq succeeded");
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: no message on the queue after putq");
}

staticf void
tmod_getadmin (q)
	queue_t	* q;
{
	pfi_t	adminp;
	ushort	mid;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing getadmin");
	adminp = getadmin(MODULE_ID);
	if (adminp)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: retrieved admin 0x%x for tmod (tmod_admin 0x%x)", adminp, tmod_admin);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: failed for tmod id %d", MODULE_ID);
	if (mid = getmid("echo")) {
		adminp = getadmin(mid);
		if (adminp)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: retrieved admin 0x%x for echo", adminp);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: failed for echo id %d", mid);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: unable to retrieve module id for echo device");
	adminp = getadmin(0xffff);
	if (adminp)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: retrieved admin 0x%x for module id 0xffff", adminp);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: no admin for module id 0xffff");
}

staticf void
tmod_getmid (q)
	queue_t	* q;
{
	ushort	mid;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing getmid");
	mid = getmid("tmod");
	if (mid == MODULE_ID)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: retrieved mid %d for 'tmod'", mid);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: retrieved mid %d for 'tmod'", mid);
	mid = getmid("echo");
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: retrieved mid %d for 'echo'", mid);
	mid = getmid("foo");
	if (mid == 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: retrieved mid %d for 'foo'", mid);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: retrieved mid %d for 'foo'", mid);
}

staticf void
tmod_getq_putq (q)
	queue_t	* q;
{
	int	i1;
	MBLKP	mp;
	unsigned char	pri;
	qband_t	* qb;

	pri = q->q_nband;
	if (pri == 0)
		pri = 5;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing getq and putq with band %d", (int)pri);
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_band = pri;
	mp->b_wptr++;
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ed a message to band %d", (int)pri);
	/* Find the band */
	for (i1 = 1, qb = q->q_bandp; i1 < pri; i1++)
		qb = qb->qb_next;
	if (q->q_count != 0  ||  qb->qb_count == 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_count %d or qb_count %d not right", q->q_count, qb->qb_count);

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ed a message to band 0");

	
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_band = pri;	/* this should be cleared by putq */
	mp->b_datap->db_type = M_PCPROTO;
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ed a high priority message to band 0");

	
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_band = pri;
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ed a message to band %d", (int)pri);

	if (qsize(q) != 4) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q does not contain 4 messages");
		flushq(q, FLUSHALL);
		return;
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: q now contains 4 messages");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_PCPROTO  &&  mp->b_band == 0)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: first getq retrieved high priority message");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: first getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: first getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_DATA  &&  mp->b_band == pri)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: second getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: second getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: second getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_PROTO  &&  mp->b_band == pri)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: third getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: third getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: third getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_DATA  &&  mp->b_band == 0)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: fourth getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: fourth getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: fourth getq failed");

	if (mp = getq(q)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: fifth getq got a message");
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: fifth getq returned NULL");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: checking q count and flag after tests");
	if (q->q_count != 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_count %d after tests", q->q_count);
	if ((q->q_flag & QWANTR) == 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_flag does not have QWANTR set (0x%x)", q->q_flag);
	if (qb->qb_count != 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: qb_count %d after tests", qb->qb_count);
}

staticf void
tmod_insq (q)
	queue_t	* q;
{
	int	i1;
	MBLKP	mp;
	MBLKP	mp1;
	unsigned char	pri;
	qband_t	* qb;

	pri = q->q_nband;
	if (pri == 0)
		pri = 5;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing insq with band %d", (int)pri);
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_band = pri;
	mp->b_wptr++;
	if (!insq(q, nil(MBLKP), mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq'ed a message into empty band %d", (int)pri);

	/* Find the band */
	for (i1 = 1, qb = q->q_bandp; i1 < pri; i1++)
		qb = qb->qb_next;
	if (q->q_count != 0  ||  qb->qb_count == 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_count %d or qb_count %d not right", q->q_count, qb->qb_count);

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_band = pri;
	if (!insq(q, qb->qb_first, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq failed");
		freeb(mp);
		flushq(q, FLUSHALL);
		return;
	}
	if (qb->qb_first != mp) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq did not work correctly");
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq'ed a message before qb_first");

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_datap->db_type = M_PCPROTO;
	if (insq(q, qb->qb_last, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq successful, placed M_PCPROTO message in the middle of band %d", pri);
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq failed to place a M_PCPROTO message in the middle of the band");

	if (insq(q, nil(MBLKP), mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq successful, placed M_PCPROTO message at the end of the queue");
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq failed to place a M_PCPROTO message at the end of the queue");

	mp->b_datap->db_type = M_DATA;
	mp->b_band = 0;
	if (insq(q, qb->qb_last, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq successful, placed a band 0 message in the middle of band %d", pri);
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq failed to place a band 0 message in the middle of the band");
	
	mp->b_datap->db_type = M_DATA;
	mp->b_band = pri;
	if (!insq(q, qb->qb_last, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq failed");
		freeb(mp);
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq'ed a message into the middle of band %d", (int)pri);

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		flushq(q, FLUSHALL);
		return;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_band = pri;
	if (!insq(q, nil(MBLKP), mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq failed");
		freeb(mp);
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq'ed a message into the end of band %d", (int)pri);

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		flushq(q, FLUSHALL);
		return;
	}
	mp->b_datap->db_type = M_PCPROTO;
	if (!insq(q, qb->qb_first, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq failed");
		freeb(mp);
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq'ed a M_PCPROTO message at the front of the queue");

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		flushq(q, FLUSHALL);
		return;
	}
	mp->b_datap->db_type = M_DATA;
	mp->b_band = pri + 5;
	if (insq(q, qb->qb_last, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq successful, placed a band %d message in the middle of band %d", (int)pri + 5, (int)pri);
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq failed to place a band %d message in the middle of the band", pri + 5);

	mp->b_band = 0;
	if (!insq(q, nil(MBLKP), mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq failed");
		freeb(mp);
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq'ed a band 0 message at the end of the queue");

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		flushq(q, FLUSHALL);
		return;
	}
	mp->b_datap->db_type = M_PROTO;
	if (!insq(q, q->q_last, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq failed");
		freeb(mp);
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq'ed a band 0 message");

	mp1 = mp;
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		flushq(q, FLUSHALL);
		return;
	}
	mp->b_datap->db_type = M_DATA;
	mp->b_band = pri + 5;
	if (insq(q, nil(MBLKP), mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq successful for band %d and emp nil", (int)pri + 5);
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq failed for band %d and emp nil", (int)pri + 5);

	if (!insq(q, mp1, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: insq failed");
		freeb(mp);
		flushq(q, FLUSHALL);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: insq'ed a band %d message", pri + 5);

	/*
	 * Find the band again, insq to band pri+5 reallocated the band
	 * structures.
	 */
	for (i1 = 1, qb = q->q_bandp; i1 < pri; i1++)
		qb = qb->qb_next;
	if (qsize(q) != 8) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q does not contain 8 messages");
		flushq(q, FLUSHALL);
		return;
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: q now contains 8 messages (q_count %d, qb_count %d)", q->q_count, qb->qb_count);

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_PCPROTO  &&  mp->b_band == 0)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: first getq retrieved high priority message");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: first getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: first getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_PROTO  &&  mp->b_band == pri)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: second getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: second getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: second getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_DATA  &&  mp->b_band == pri)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: third getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: third getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: third getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_DATA  &&  mp->b_band == pri)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: fourth getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: fourth getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: fourth getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_PROTO  &&  mp->b_band == pri)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: fifth getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: fifth getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: fifth getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_DATA  &&  mp->b_band == pri + 5)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: sixth getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: sixth getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: sixth getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_PROTO  &&  mp->b_band == 0)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: seventh getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: seventh getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: seventh getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_DATA  &&  mp->b_band == 0)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: eighth getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: eighth getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: eighth getq failed");

	if (mp = getq(q)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: ninth getq got a message");
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: ninth getq returned NULL");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: checking q count and flag after tests");
	if (q->q_count != 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_count %d after tests", q->q_count);
	if ((q->q_flag & QWANTR) == 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_flag does not have QWANTR set (0x%x)", q->q_flag);
	if (qb->qb_count != 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: qb_count %d after tests", qb->qb_count);
}

staticf void
tmod_linkb (q)
	queue_t	* q;
{
	int	i1;
	MBLKP	mp1;
	MBLKP	mp2;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing linkb, unlinkb, and msgdsize");

	if (!(mp1 = allocb(300, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp1->b_wptr += 212;

	if (!(mp1->b_cont = allocb(30, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp1);
		return;
	}
	mp1->b_cont->b_wptr += 24;

	i1 = msgdsize(mp1);
	if (i1 == 236)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: msgdsize returned correct number of bytes in first message");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: msgdsize returned %d when expecting 236", i1);

	if (!(mp2 = allocb(1000, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp1);
		return;
	}
	mp2->b_wptr += 956;

	i1 = msgdsize(mp2);
	if (i1 == 956)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: msgdsize returned correct number of bytes in first message");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: msgdsize returned %d when expecting 956", i1);

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: linking two messages");
	linkb(mp1, mp2);
	if (mp1->b_cont->b_cont != mp2) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: linkb failed");
		freemsg(mp1);
		return;
	}
	i1 = msgdsize(mp1);
	if (i1 == 1192)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: linkb successful");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: linkb ok, but msgdsize returned %d when expecting 1192", i1);

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: unlinking");
	mp2 = unlinkb(mp1);
	freeb(mp1);
	i1 = msgdsize(mp2);
	if (i1 == 980)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: unlinkb successful");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: unlinkb ok, but msgdsize returned %d when expecting 980", i1);

	mp1 = mp2;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: unlinking");
	mp2 = unlinkb(mp1);
	freeb(mp1);
	i1 = msgdsize(mp2);
	if (i1 == 956)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: unlinkb successful");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: unlinkb ok, but msgdsize returned %d when expecting 956", i1);

	mp1 = mp2;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: unlinking with only one block");
	mp2 = unlinkb(mp1);
	freeb(mp1);
	if (!mp2)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: unlinkb successful");
	else {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: last unlinkb did not return nil");
		freemsg(mp2);
	}
}

staticf int
tmod_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	noenable(q);
	noenable(WR(q));
	return 0;
}

staticf void
tmod_pullupmsg (q)
	queue_t	* q;
{
	int	i1;
	MBLKP	mp;
	MBLKP	mp1;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing pullupmsg");
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_wptr += 60;
	mp->b_rptr += 3;
	if (!(mp1 = allocb(128, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp);
		return;
	}
	mp1->b_wptr += 104;
	mp->b_cont = mp1;
	if (!(mp1 = allocb(128, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp);
		return;
	}
	mp1->b_wptr += 104;
	mp1->b_cont = mp->b_cont;
	mp->b_cont = mp1;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying pullupmsg with not enough bytes in message");
	if (pullupmsg(mp, 1000)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg succeeded");
		freemsg(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: pullupmsg failed");
	i1 = msgdsize(mp);
	if (i1 != 265)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg changed the message (%d bytes should be %d)",
			i1, 265);

	mp->b_cont->b_datap->db_type = M_PROTO;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying pullupmsg with blocks of different types");
	if (pullupmsg(mp, 200)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg succeeded");
		freemsg(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: pullupmsg failed");
	mp->b_cont->b_datap->db_type = M_DATA;
	i1 = msgdsize(mp);
	if (i1 != 265)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg changed the message (%d bytes should be %d)",
			i1, 265);

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying pullupmsg just to fix alignment");
	/*
	 * NOTE: this test assumes that pullupmsg aligns to a 4-byte boundary.
	 * This may not be true for some systems.
	 */
	if (pullupmsg(mp, 40)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: pullupmsg succeeded");
		i1 = mp->b_wptr - mp->b_rptr;
		if (i1 != 57)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg changed the size of the 1st block from 57 to %d", i1);
		i1 = msgdsize(mp);
		if (i1 != 265)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg changed the total message size (%d bytes should be %d)",
				i1, 265);
		if ((long)mp->b_rptr & 0x3)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: b_rptr is not aligned (0x%x)", mp->b_rptr);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying pullupmsg across multiple blocks");
	mp->b_rptr++;	/* Odd alignment again */
	if (pullupmsg(mp, 200)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: pullupmsg succeeded");
		i1 = mp->b_wptr - mp->b_rptr;
		if (i1 != 200)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg did not place 200 bytes in 1st block (%d)", i1);
		i1 = msgdsize(mp);
		if (i1 != 264)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg changed the total message size (%d bytes should be %d)",
				i1, 264);
		if ((long)mp->b_rptr & 0x3)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: b_rptr is not aligned (0x%x)", mp->b_rptr);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: trying pullupmsg across multiple blocks with -1 length");
	if (!(mp1 = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp);
		return;
	}
	mp1->b_wptr += 64;
	mp1->b_cont = mp->b_cont;
	mp->b_cont = mp1;
	if (pullupmsg(mp, -1)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: pullupmsg succeeded");
		i1 = mp->b_wptr - mp->b_rptr;
		if (i1 != 328)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg did not place 328 bytes in 1st block (%d)", i1);
		if (mp->b_cont)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: b_cont is not NULL");
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg failed");

	/* check lossage of more than one mblk */
	if (!(mp1 = allocb(512, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp);
		return;
	}
	mp1->b_wptr += 256;	/* enough to cause another reallocation */
	mp1->b_cont = mp->b_cont;
	mp->b_cont = mp1;
	if (pullupmsg(mp, -1)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: pullupmsg succeeded");
		i1 = mp->b_wptr - mp->b_rptr;
		if (i1 != 584)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg did not place 584 bytes in 1st block (%d)", i1);
		if (mp->b_cont)
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: b_cont is not NULL");
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: pullupmsg failed");

	freemsg(mp);
}

staticf void
tmod_putbq (q)
	queue_t	* q;
{
	int	i1;
	MBLKP	mp;
	unsigned char	pri;
	qband_t	* qb;

	pri = q->q_nband;
	if (pri == 0)
		pri = 5;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing putbq with band %d", (int)pri);
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_band = pri;
	mp->b_wptr++;
	if (!putbq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putbq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putbq'ed a message to band %d", (int)pri);
	/* Find the band */
	for (i1 = 1, qb = q->q_bandp; i1 < pri; i1++)
		qb = qb->qb_next;
	if (q->q_count != 0  ||  qb->qb_count == 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_count %d or qb_count %d not right", q->q_count, qb->qb_count);

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	if (!putbq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putbq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putbq'ed a message to band 0");

	
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_band = pri;	/* this should be cleared by putbq */
	mp->b_datap->db_type = M_PCPROTO;
	if (!putbq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putbq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putbq'ed a high priority message to band 0");

	
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_band = pri;
	if (!putbq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putbq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putbq'ed a message to band %d", (int)pri);

	if (qsize(q) != 4) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q does not contain 4 messages");
		flushq(q, FLUSHALL);
		return;
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: q now contains 4 messages");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_PCPROTO  &&  mp->b_band == 0)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: first getq retrieved high priority message");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: first getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: first getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_PROTO  &&  mp->b_band == pri)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: second getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: second getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: second getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_DATA  &&  mp->b_band == pri)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: third getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: third getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: third getq failed");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_DATA  &&  mp->b_band == 0)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: fourth getq retrieved band %d message", mp->b_band);
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: fourth getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: fourth getq failed");

	if (mp = getq(q)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: fifth getq got a message");
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: fifth getq returned NULL");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: checking q count and flag after tests");
	if (q->q_count != 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_count %d after tests", q->q_count);
	if ((q->q_flag & QWANTR) == 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_flag does not have QWANTR set (0x%x)", q->q_flag);
	if (qb->qb_count != 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: qb_count %d after tests", qb->qb_count);
}

staticf void
tmod_qenable (q)
	queue_t	* q;
{
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: enabling write side queue (log message should appear from service routine)");
	qenable(q);
}

staticf void
tmod_rmvb (q)
	queue_t	* q;
{
	int	i1;
	MBLKP	mp;
	MBLKP	mp2;
	MBLKP	mp3;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing rmvb");
	if (!(mp = allocb(300, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_wptr += 212;

	if (!(mp2 = allocb(30, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp);
		return;
	}
	mp2->b_wptr += 24;
	linkb(mp, mp2);

	if (!(mp2 = allocb(1000, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp);
		return;
	}
	mp2->b_wptr += 956;
	linkb(mp, mp2);

	if (!(mp2 = allocb(100, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		freemsg(mp);
		return;
	}

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling rmvb with bp not in message");
	mp3 = rmvb(mp, mp2);
	if (mp3 != (MBLKP)-1) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: rmvb successful");
		freemsg(mp);
		freemsg(mp2);
		return;
	}
	freemsg(mp2);
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: rmvb failed");

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling rmvb with bp in the middle of the message");
	mp2 = mp->b_cont;
	mp3 = rmvb(mp, mp2);
	if (mp3 != mp) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: rmvb returned unexpected message");
		freemsg(mp3);
		freeb(mp2);
		return;
	}
	i1 = msgdsize(mp);
	if (i1 == 1168)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: rmvb successful");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: rmvb ok, but msgdsize returned %d when expecting 1168", i1);

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling rmvb with bp at the end of the message");
	/* first, link the 24 byte message block back into the message */
	mp2->b_cont = mp->b_cont;
	mp->b_cont = mp2;
	mp2 = mp->b_cont->b_cont;
	mp3 = rmvb(mp, mp2);
	if (mp3 != mp) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: rmvb returned unexpected message");
		freemsg(mp3);
		freeb(mp2);
		return;
	}
	i1 = msgdsize(mp);
	if (i1 == 236)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: rmvb successful");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: rmvb ok, but msgdsize returned %d when expecting 236", i1);
	freeb(mp2);

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling rmvb with the first block in the message");
	mp2 = mp->b_cont;
	mp3 = rmvb(mp, mp);
	if (mp3 != mp2) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: rmvb returned unexpected message");
		freeb(mp);
		freeb(mp2);
		return;
	}
	i1 = msgdsize(mp2);
	if (i1 == 24)
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: rmvb successful");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: rmvb ok, but msgdsize returned %d when expecting 24", i1);
	freeb(mp);

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: calling rmvb with the only block in the message");
	mp3 = rmvb(mp2, mp2);
	if (mp3) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: rmvb returned unexpected message");
		freeb(mp2);
		return;
	}
	freeb(mp2);
}

staticf void
tmod_rmvq (q)
	queue_t	* q;
{
	int	i1;
	MBLKP	mp;
	unsigned char	pri;
	qband_t	* qb;

	pri = q->q_nband;
	if (pri == 0)
		pri = 5;
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing rmvq with band %d", (int)pri);
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_band = pri;
	mp->b_wptr++;
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ed a message to band %d", (int)pri);
	/* Find the band */
	for (i1 = 1, qb = q->q_bandp; i1 < pri; i1++)
		qb = qb->qb_next;
	if (q->q_count != 0  ||  qb->qb_count == 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_count %d or qb_count %d not right", q->q_count, qb->qb_count);

	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ed a message to band 0");

	
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_band = pri;	/* this should be cleared by putq */
	mp->b_datap->db_type = M_PCPROTO;
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ed a high priority message to band 0");

	
	if (!(mp = allocb(64, BPRI_HI))) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: allocb failed");
		return;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_band = pri;
	if (!putq(q, mp)) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: putq failed");
		freeb(mp);
		return;
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: putq'ed a message to band %d", (int)pri);

	if (qsize(q) != 4) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q does not contain 4 messages");
		flushq(q, FLUSHALL);
		return;
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: q now contains 4 messages");

	mp = qb->qb_first;
	rmvq(q, mp);
	freemsg(mp);
	if (qb->qb_first->b_datap->db_type != M_PROTO)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: rmvq didn't remove the first message from band %d", (int)pri);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: rmvq removed first message from band %d", (int)pri);

	mp = q->q_last;
	rmvq(q, mp);
	freemsg(mp);
	if (qb->qb_last->b_datap->db_type != M_PROTO  ||  qb->qb_last->b_band != pri)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: rmvq didn't remove the last message from the queue");
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: rmvq removed the last message from the queue");

	if (qsize(q) != 2) {
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q does not contain 2 messages");
		flushq(q, FLUSHALL);
		return;
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: q now contains 2 messages");

	if (mp = getq(q)) {
		if (mp->b_datap->db_type == M_PCPROTO  &&  mp->b_band == 0)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: getq retrieved M_PCPROTO message");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: getq retrieved message with type 0%o and band %d",
				mp->b_datap->db_type, mp->b_band);
		freemsg(mp);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: getq failed");

	mp = qb->qb_first;
	rmvq(q, mp);
	freemsg(mp);
	if (qb->qb_first)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: rmvq didn't remove the last message from band %d", (int)pri);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: rmvq removed the last message from band %d", (int)pri);


	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: checking q count and flag after tests");
	if (q->q_first)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_first not nil");
	if (qb->qb_first)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: qb_first not nil");
	if (q->q_count != 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: q_count %d after tests", q->q_count);
	if (qb->qb_count != 0)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: qb_count %d after tests", qb->qb_count);
}

staticf void
tmod_strlog (q)
	queue_t	* q;
{
	strlog(MODULE_ID, 0, 1, SL_CONSOLE | SL_TRACE, "tmod: this message should appear on the console and in the strace output");

	strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: this message should have the ERROR bit set");

	strlog(MODULE_ID, 0, 1, SL_TRACE|SL_NOTIFY|SL_ERROR, "tmod: this message has the NOTIFY bit set (it should be sent in an e-mail message)");

	strlog(MODULE_ID, 0, 1, SL_TRACE|SL_FATAL|SL_ERROR, "tmod: this message has the FATAL bit set");

	strlog(MODULE_ID, 0, 1, SL_TRACE|SL_WARN, "tmod: this message has the WARN bit set");

	strlog(MODULE_ID, 0, 1, SL_TRACE|SL_NOTE, "tmod: this message has the NOTE bit set");
}

staticf void
tmod_strqget (q)
	queue_t	* q;
{
	int	err;
	int	i1;
	unsigned char	pri;
	qband_t	* qb;
	long	val;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing strqget on queue fields");

	if (err = strqget(q, QHIWAT, 0, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QHIWAT", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QHIWAT value %ld (should be %ld)", val, q->q_hiwat);

	if (err = strqget(q, QLOWAT, 0, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QLOWAT", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QLOWAT value %ld (should be %ld)", val, q->q_lowat);

	if (err = strqget(q, QMAXPSZ, 0, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QMAXPSZ", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QMAXPSZ value %ld (should be %ld)", val, q->q_maxpsz);

	if (err = strqget(q, QMINPSZ, 0, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QMINPSZ", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QMINPSZ value %ld (should be %ld)", val, q->q_minpsz);

	if (err = strqget(q, QCOUNT, 0, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QCOUNT", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QCOUNT value %ld (should be %ld)", val, q->q_count);

	if (err = strqget(q, QFIRST, 0, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QFIRST", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QFIRST value %ld (should be %ld)", val, q->q_first);

	if (err = strqget(q, QLAST, 0, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QLAST", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QLAST value %ld (should be %ld)", val, q->q_last);

	if (err = strqget(q, QFLAG, 0, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QFLAG", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QFLAG value %ld (should be %ld)", val, q->q_flag);

	if (err = strqget(q, QBAD, 0, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned err %d on QBAD", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned QBAD value %ld", val);

	pri = q->q_nband;
	if (pri == 0) {
		pri = 5;
		if (strqset(q, QHIWAT, pri, q->q_hiwat) != 0  ||  q->q_nband != pri) {
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: couldn't create band %d for test", pri);
			return;
		}
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing strqget on qband %d", pri);
	/* Find the band */
	for (i1 = 1, qb = q->q_bandp; i1 < pri; i1++)
		qb = qb->qb_next;

	if (err = strqget(q, QHIWAT, pri, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QHIWAT", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QHIWAT value %ld (should be %ld)", val, qb->qb_hiwat);

	if (err = strqget(q, QLOWAT, pri, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QLOWAT", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QLOWAT value %ld (should be %ld)", val, qb->qb_lowat);

	if (err = strqget(q, QMAXPSZ, pri, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QMAXPSZ", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QMAXPSZ value %ld (should be %ld)", val, q->q_maxpsz);

	if (err = strqget(q, QMINPSZ, pri, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QMINPSZ", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QMINPSZ value %ld (should be %ld)", val, q->q_minpsz);

	if (err = strqget(q, QCOUNT, pri, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QCOUNT", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QCOUNT value %ld (should be %ld)", val, qb->qb_count);

	if (err = strqget(q, QFIRST, pri, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QFIRST", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QFIRST value %ld (should be %ld)", val, qb->qb_first);

	if (err = strqget(q, QLAST, pri, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QLAST", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QLAST value %ld (should be %ld)", val, qb->qb_last);

	if (err = strqget(q, QFLAG, pri, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned err %d on QFLAG", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned QFLAG value %ld (should be %ld)", val, qb->qb_flag);

	if (err = strqget(q, QBAD, pri, &val))
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqget returned err %d on QBAD", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqget returned QBAD value %ld", val);
}

staticf void
tmod_strqset (q)
	queue_t	* q;
{
	int	err;
	int	i1;
	unsigned char	pri;

	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing strqset on queue fields");

	if (err = strqset(q, QHIWAT, 0, 302))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset returned err %d on QHIWAT", err);
	else if (q->q_hiwat != 302)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset set QHIWAT to %ld (should be %ld)", q->q_hiwat, 302);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset succeeded for QHIWAT");

	if (err = strqset(q, QLOWAT, 0, 304))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset returned err %d on QLOWAT", err);
	else if (q->q_lowat != 304)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset set QLOWAT to %ld (should be %ld)", q->q_lowat, 304);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset succeeded for QLOWAT");

	if (err = strqset(q, QMAXPSZ, 0, 308))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset returned err %d on QMAXPSZ", err);
	else if (q->q_maxpsz != 308)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset set QMAXPSZ to %ld (should be %ld)", q->q_maxpsz, 308);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset succeeded for QMAXPSZ");

	if (err = strqset(q, QMINPSZ, 0, 400))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset returned err %d on QMINPSZ", err);
	else if (q->q_minpsz != 400)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset set QMINPSZ to %ld (should be %ld)", q->q_minpsz, 400);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset succeeded for QMINPSZ");

	if (err = strqset(q, QCOUNT, 0, 302)) {
		if (err == EPERM)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset failed with EPERM on QCOUNT");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset failed with err %d on QCOUNT", err);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset succeeded on QCOUNT");

	if (err = strqset(q, QFIRST, 0, 302)) {
		if (err == EPERM)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset failed with EPERM on QFIRST");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset failed with err %d on QFIRST", err);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset succeeded on QFIRST");

	if (err = strqset(q, QLAST, 0, 302)) {
		if (err == EPERM)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset failed with EPERM on QLAST");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset failed with err %d on QLAST", err);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset succeeded on QLAST");

	if (err = strqset(q, QFLAG, 0, 302)) {
		if (err == EPERM)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset failed with EPERM on QFLAG");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset failed with err %d on QFLAG", err);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset succeeded on QFLAG");

	if (err = strqset(q, QBAD, 0, 302))
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset returned err %d on QBAD", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset succeeded on QBAD");

	pri = q->q_nband;
	if (pri == 0) {
		pri = 5;
		if (strqset(q, QHIWAT, pri, q->q_hiwat) != 0  ||  q->q_nband != pri) {
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: couldn't create band %d for test", pri);
			return;
		}
	}
	strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: testing strqset on qband %d", pri);

	if (err = strqset(q, QHIWAT, pri, 302))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset returned err %d on QHIWAT", err);
	else if (q->q_hiwat != 302)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset set QHIWAT to %ld (should be %ld)", q->q_hiwat, 302);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset succeeded for QHIWAT");

	if (err = strqset(q, QLOWAT, pri, 304))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset returned err %d on QLOWAT", err);
	else if (q->q_lowat != 304)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset set QLOWAT to %ld (should be %ld)", q->q_lowat, 304);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset succeeded for QLOWAT");

	if (err = strqset(q, QMAXPSZ, pri, 308))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset returned err %d on QMAXPSZ", err);
	else if (q->q_maxpsz != 308)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset set QMAXPSZ to %ld (should be %ld)", q->q_maxpsz, 308);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset succeeded for QMAXPSZ");

	if (err = strqset(q, QMINPSZ, pri, 400))
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset returned err %d on QMINPSZ", err);
	else if (q->q_minpsz != 400)
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset set QMINPSZ to %ld (should be %ld)", q->q_minpsz, 400);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset succeeded for QMINPSZ");

	if (err = strqset(q, QCOUNT, pri, 302)) {
		if (err == EPERM)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset failed with EPERM on QCOUNT");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset failed with err %d on QCOUNT", err);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset succeeded on QCOUNT");

	if (err = strqset(q, QFIRST, pri, 302)) {
		if (err == EPERM)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset failed with EPERM on QFIRST");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset failed with err %d on QFIRST", err);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset succeeded on QFIRST");

	if (err = strqset(q, QLAST, pri, 302)) {
		if (err == EPERM)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset failed with EPERM on QLAST");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset failed with err %d on QLAST", err);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset succeeded on QLAST");

	if (err = strqset(q, QFLAG, pri, 302)) {
		if (err == EPERM)
			strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset failed with EPERM on QFLAG");
		else
			strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset failed with err %d on QFLAG", err);
	} else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset succeeded on QFLAG");

	if (err = strqset(q, QBAD, pri, 302))
		strlog(MODULE_ID, 0, 1, SL_TRACE, "tmod: strqset returned err %d on QBAD", err);
	else
		strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: strqset succeeded on QBAD");
}

staticf void
tmod_rput (q, mp)
	queue_t	* q;
	MBLKP	mp;
{
strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: rput called!\n");
	putnext(q, mp);
}

staticf void
tmod_rsrv (q)
	queue_t	* q;
{
	MBLKP	mp;

	while (mp = getq(q)) {
		if (!canput(q->q_next)) {
strlog(MODULE_ID, 0, 1, SL_TRACE|SL_ERROR, "tmod: ERROR: rsrv flow controlled\n");
			putbq(q, mp);
			break;
		}
		putnext(q, mp);
strlog(MODULE_ID, 0, 0, SL_TRACE|SL_ERROR, "tmod: ERROR: rsrv back enabled\n");
	}
}

staticf void
tmod_wput (q, mp)
	queue_t	* q;
	MBLKP	mp;
{
	struct iocblk	* iocp;

	if (mp->b_datap->db_type != M_IOCTL) {
		putnext(q, mp);
		return;
	}
	iocp = (struct iocblk *)mp->b_rptr;
	mp->b_datap->db_type = M_IOCACK;
	switch (iocp->ioc_cmd) {
	case TMOD_ADJMSG:
		tmod_adjmsg(q);
		break;
	case TMOD_CANPUT:
		tmod_canput(q);
		break;
	case TMOD_BUFCALL:
		tmod_bufcall(q);
		break;
	case TMOD_COPY:
		tmod_copy(q);
		break;
	case TMOD_DUP:
		tmod_dup(q);
		break;
	case TMOD_ESBALLOC:
		tmod_esballoc(q);
		break;
	case TMOD_FLUSH:
		tmod_flush(q);
		break;
	case TMOD_GETADMIN:
		tmod_getadmin(q);
		break;
	case TMOD_GETMID:
		tmod_getmid(q);
		break;
	case TMOD_GETQ:
		tmod_getq_putq(q);
		break;
	case TMOD_INSQ:
		tmod_insq(q);
		break;
	case TMOD_LINKB:
		tmod_linkb(q);
		break;
	case TMOD_PULLUPMSG:
		tmod_pullupmsg(q);
		break;
	case TMOD_PUTQ:
		tmod_getq_putq(q);
		break;
	case TMOD_PUTBQ:
		tmod_putbq(q);
		break;
	case TMOD_RMVB:
		tmod_rmvb(q);
		break;
	case TMOD_RMVQ:
		tmod_rmvq(q);
		break;
	case TMOD_QENABLE:
		tmod_qenable(q);
		break;
	case TMOD_STRLOG:
		tmod_strlog(q);
		break;
	case TMOD_STRQGET:
		tmod_strqget(q);
		break;
	case TMOD_STRQSET:
		tmod_strqset(q);
		break;
	default:
		mp->b_datap->db_type = M_IOCNAK;
		break;
	}
	qreply(q, mp);
	return;
}

staticf void
tmod_wsrv (q)
	queue_t	* q;
{
	MBLKP	mp;

	strlog(MODULE_ID, 1, 0, SL_TRACE, "tmod: write service routine: hello");
	while (mp = getq(q)) {
		if (!canput(q->q_next)) {
			putbq(q, mp);
			break;
		}
		putnext(q, mp);
	}
}

#include <sys/device.h>
#include <sys/strconf.h>

int
tmod_config(cmd, uiop)
	int cmd;
	struct uio *uiop;
{
	static strconf_t conf = {
		"tmod", &tmodinfo, STR_NEW_OPEN,
	};

	switch (cmd) {
	case CFG_INIT:	return str_install(STR_LOAD_MOD, &conf);
	case CFG_TERM:	return str_install(STR_UNLOAD_MOD, &conf);
	default:	return EINVAL;
	}
}
