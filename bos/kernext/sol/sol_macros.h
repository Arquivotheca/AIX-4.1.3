/* @(#)60	1.7  src/bos/kernext/sol/sol_macros.h, sysxsol, bos411, 9428A410j 4/17/92 10:09:10 */
#ifndef _H_SOL_MACROS
#define _H_SOL_MACROS
/*
 * COMPONENT_NAME: (SYSXSOL) - Serial Optical Link Device Handler Include File
 *
 * FUNCTIONS: sol_macros.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
	macros for sla hardware
*/


#ifdef DIAGNOSTIC
#define SLA_ADDRESS(sla_num)	slaioaddr[sla_num]
#else
#define	SLA_ADDRESS(sla_num)	sla_tbl.sla[sla_num].io_address
#endif

#include <sys/adspace.h>
#define CHECK_REG(slap)

#if 0
#define READ_PRHR(slap,prhr_value)	prhr_value = slap -> prhr.fr;
#define WRITE_THR(slap,thr_value)	slap -> thr.fr = thr_value;
#else
#define READ_PRHR(slap,prhr_value)	cprgs(&(slap -> prhr.fr), &(prhr_value), SLA_THR_SZ);
#define WRITE_THR(slap,thr_value)	cprgs(&(thr_value), &(slap -> thr.fr), SLA_THR_SZ);
#endif


#define PUISTATE(q)	( (q) & 0x00000038)
#define LINKSTATE(q)	( (q) & 0x00000007)
#define ANYCHK(q)	( (q) & 0xe0100000)	/* programm, link and channel
						   check and abort transmitted */


#define TRACE_START(slap,sla_id)	{			\
	IMCS_DBG(IMCSD_START, slaevent(STAMP('t',sla_id), slap -> ccr,	\
	                               slap -> sample, slap -> thr.thr[0]) );\
	IMCS_DBG(IMCSD_START, slaevent(slap -> thr.thr[1], slap -> thr.thr[2],\
                               slap -> thr.thr[3], slap -> thr.thr[4]) );\
					}


#define	OPERATE_CHANNEL(slap,sla_id,s1_val,s2_val)	{		\
	CHECK_REG(slap);						\
	s2_val = slap -> status2;  /* clear it */			\
	slap -> ccr = NATIVE | PRIORITY | RTV_4 | sla_tbl.sla[sla_id].ccr_all;\
	s1_val = slap -> ch_op;						\
	  						}
	

#define	SYNCHR_CHANNEL(slap,sla_id)		{			\
	CHECK_REG(slap);						\
	sla_pstart(slap, sla_id, ACTIVATE | PRIORITY | ISTP |		\
	           RTV_4 | sla_tbl.sla[sla_id].ccr_all & ~ AUTO_SCR);	\
						}


#define	CH_TRANSPARENT(slap,sla_id) 		{			\
	CHECK_REG(slap);						\
	sla_pstart(slap, sla_id,					\
	           TRANSPARENT | PRIORITY | DRTO | CS_XMT |		\
		   sla_tbl.sla[sla_id].ccr_all & ~ AUTO_SCR);		\
  						}


#define	START_SCR(slap,sla_id)  		{			\
	CHECK_REG(slap);						\
	sla_pstart(slap, sla_id, SCR_CCR | sla_tbl.sla[sla_id].ccr_all);\
						}
/* initialize the tag table */
#define INIT_TAGS(slap)		{					\
	int _i;								\
	CHECK_REG(slap);						\
	for (_i = 0; _i < NUM_TCWS; _i++)				\
	  slap -> tcw[_i] = LAST_TAG;					\
				}


/* initialize the transmit and receive registers */
#define INIT_REGS(slap)		{					\
	int _i;								\
	CHECK_REG(slap);						\
	for (_i = 0; _i < 5; _i++) {					\
	  slap -> prhr.prhr[_i] = 0;					\
	  slap -> thr.thr[_i] = 0;					\
	}								\
				}


/* load the tag table */
#define LOAD_TAGS(slap,sla_id)						\
	bcopy(primary_hdr(sla_id) -> tags.tagwords, slap -> tcw,	\
	      NUM_TCWS * sizeof(primary_hdr(sla_id) -> tags.tagwords[0]))


/* load enough tag to do a continous sequence */
#define LOAD_CONT_TAGS(slap,tagword)		{			\
	int _i;								\
	CHECK_REG(slap);						\
	for (_i = 0; _i < NUM_CONT_TCWS; _i++)				\
	  slap -> tcw[_i] = (tagword);					\
	slap -> tcw[NUM_CONT_TCWS] = LAST_TAG;				\
						}

/*  the sla exhibits this funny behavior, and sometimes the first interrupt on 
light loss will be an address mismatch with signal error (but not signal 
failure).  This is probably due to noise which generates a start of frame but 
gargabe after.  Hence the behaviour */
#define SIGNAL_FAILURE(slap,s2_val)	( ((s2_val) & SIG_FAIL) ||	\
				          (((s2_val) & ADDR_MIS) &&	\
					   ((s2_val) & LINK_SIG_ERR) &&	\
					   (slap -> status2 & LINK_SIG_ERR)) )

#define SOLIDLY_ON(slap,s2_val,value)	( ((s2_val) & (value)) &&	\
				 	  (slap -> status2 & (value)) &&\
				 	  (slap -> status2 & (value)) )


/* swap the two halves in a word */
#define	SWAP(word)		( (word) << 16 | (word) >> 16)

/* extract the link addresses in a prhr address field */
#define EXTRACT_SADDR(did_sid)	((uchar) ( (did_sid & 0x0000ff00) >> 8) )
#define EXTRACT_DADDR(did_sid)	((uchar) ( (did_sid & 0xff000000) >> 24) )


/* extract the imcs qid and destination id from the B register */
#define EXTRACT_QID(B)		((int) ( (B) & 0x0000ffff) )
#define EXTRACT_DID(B)		((int) ( (B) & 0xffff0000) >> 16)

#ifdef DEBUG
#define PRT_MSG(routine,msg1,msg2,sla_id,s1_val,s2_val)		\
	{							\
	printf("%s : %s on sla %d\n", routine, msg1, (sla_id));	\
	printf("     status1 "); print_status1((s1_val));	\
	printf("     status2 "); print_status2((s2_val));	\
	printf("     %s\n",msg2);				\
	}
#else
#define PRT_MSG(routine,msg1,msg2,sla_id,s1_val,s2_val)	
#endif	

#define SLA_TIMER(sla_id,data,routine,sec,nsec)				\
START_TIMER(sla_tbl.sla[sla_id].time_struct, routine, data, sec, nsec);

/*
	macros to deal with timer
*/

#define START_TIMER(addr,routine,data,sec,nsec){	\
		struct trb *_t;				\
		_t = (struct trb *) (addr);		\
		ASSERT(!(_t -> flags & T_ACTIVE) );	\
		_t -> flags &= ~(T_ABSOLUTE);		\
		_t -> func = (void (*) ()) routine;	\
		_t -> t_func_data = *((ulong *)&data);	\
		_t -> timeout.it_value.tv_sec =		\
			(sec);				\
		_t -> timeout.it_value.tv_nsec =	\
			(nsec);				\
		tstart(_t);				\
	}

#define STOP_TIMER(addr){			\
		struct trb *_t;			\
		_t = (struct trb *) (addr);	\
		if (_t -> flags & T_ACTIVE)	\
		tstop(_t);			\
	}
						
#define HASH_CDDQ(q)	((int) (q) & CDDQ_HASH_MASK)

#define RJT_INFO1(q)            ( (q) & 0xff000000)
/* extracts the first byte of the info field for RJT frames */
#define RJT_INFO2(q)            ( (q) & 0x00ff0000)
/* extracts the second byte of the info field for RJT frames */

/* the first byte of a device frame is the device control field */
#define DEV_CTL(q)  	( (q) & 0xff000000)     /* extracts device ctl field */

#define DEV_FG_FR_CNT(q)        ( (int) ( ( (q) & 0x00030000) >> 16) )
/* extracts the number of frames in automatic sequence (modulo 8) */

  
#define DEV_SUBCH(q)	( (int)((q) & 0x0000ffff) )
#define EOF_RCVD(q)	( (q) & 0x60000000)
#define CNT_RCVD(q) 	( (q) & 0x00ffffff) 
 
#define SLAIOP(p)	struct slaregs volatile *p

#define EXTRACT_CONFIG(q)	(uchar) (( (q) & 0xff000000) >> 24) 

#define HASH_SQ(pid,qid) 	( ((int) (pid)^(int) (qid)) & SQ_HASH_MASK) 

#define HASH_IRQ(q) 		((q) & RQ_HASH_MASK)


#ifdef DEBUG
#define SOL_TRACE(tag,arg1,arg2,arg3) \
	 	sol_trace(tag,arg1,arg2,arg3,0)
#define SYS_SOL_TRACE(tag,arg1,arg2,arg3) \
	 	sol_trace(tag,arg1,arg2,arg3,1)
#define PANIC(s) 	panic(s)
#define PRINTF(s,x,y)	printf(s,x,y)
#else 
#define PANIC(s)
#define PRINTF(s,x,y)
#define SOL_TRACE(tag,arg1,arg2,arg3) 
#define SYS_SOL_TRACE(tag,arg1,arg2,arg3) \
		TRCHKGT(HKWD_DD_SOL | HKTY_GT | 4, *(ulong *)tag, arg1, \
			arg2, arg3, 0)
#endif

#define WAIT_SLA(action)        {                                       \
        action = WAIT_CODE;                                             \
        delay_sec = SEND_TIME_SEC;                                      \
        delay_nsec = SEND_TIME_NSEC;                                    \
                                }


#define NOTIFY(hdr)     {                               \
	SOL_TRACE("noti",hdr,hdr->outcome,0);		\
        TRCHKL0T(HKWD_USER5);                           \
        (* (hdr) -> notify_address) (hdr);              \
        TRCHKL0T(HKWD_USER6);                           \
                        }


#define SOL_MTOCL(m) ((caddr_t) (((int) (m)->m_data) & ~(PAGESIZE-1)))
#endif _H_SOL_MACROS
