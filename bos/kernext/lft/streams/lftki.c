static char sccsid[] = "@(#)99  1.6  src/bos/kernext/lft/streams/lftki.c, lftdd, bos411, 9435D411a 9/1/94 19:41:53";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: lftKiCb
 *		lftKiInit
 *		lftKiOffl
 *		lftKiSak
 *		lftKiTerm
 *
 *   ORIGINS: 27 ,83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*------------

  This file contains the keyboard/sound driver interface :
      lftKiCb		- LFT keyboard Call-back routine
      lftKiInit		- Init keyboard special file driver
      lftKiTerm		- terminate keyboard device driver
      lftKiSak		- Send a sak signal up
      lftKiOffl		- Off-level interrupt handler
  ------------*/
/***********************************************************
Include Files
***********************************************************/
#include  <sys/types.h>
#include  <sys/errno.h>
#include  <sys/device.h>
#include  <sys/pri.h>
#include  <sys/proc.h>
#include  <sys/intr.h>
#include  <sys/termio.h>
#include  <sys/malloc.h>
#include  <sys/signal.h>

/* Private includes
   =============
*/
#include <lft.h>	
#include <sys/inputdd.h>
#include <sys/lft_ioctl.h>
#include <sys/display.h>
#include <sys/syspest.h>
#include <graphics/gs_trace.h>
#include <lft_debug.h>

/* Streams and tty includes
   ========================
*/
#include <sys/stream.h>
#include <lftsi.h>              /* LFT streams information */

/* global variable (pwr_mgr/pwr_mgr.c) */
extern int keystroke;

GS_MODULE(lftki);
BUGVDEF(db_lftki, 0);                  /* define initial debug level */

/***********************************************************
void lftKiCb ()
This call back function runs at INTCLASS3 priority.  

INPUT : Keyboard events from inputring

OUTPUT: Keyboard event passed to off-level interrupt
function

RETURNS : NONE

ERROR CODES LOGGED: None

***********************************************************/
void lftKiCb ()
{
/* TRACE "Entering lftKiCb"*/
GS_ENTER_TRC(HKWD_GS_LFT,lftki,1,lftKiCb,0,0,0,0,0);

        /* 
         *   set the flag to indicate keyboard activity has been detected.
         *  A watchdog timer has been started in lftinit and it will
         *  check for this flag every five seconds or so.  If the 
         *  flag is set, it will call the driver for this display
         *  to turn off power saving mode of the default display. 
         */

        keystroke = TRUE;   /* TRUE == 1 */

	i_sched(&LFT_UP.offl_hdl);
return;
}
/***********************************************************
void lftKiSak(void);

This function is called by the keyboard device driver
when a secure attention key sequence is detected.

INPUT : NONE

OUTPUT: SIGSAK is sent or lftKiSak will be recalled later
	by bufcall if allocb failed

RETURNS :NONE

ERROR CODES LOGGED:

***********************************************************/
void lftKiSak()
{
mblk_t *mp;
GS_ENTER_TRC0(HKWD_GS_LFT,lftki,1,lftKiSak);
	mp = allocb(LFTMSGSIZE,BPRI_MED);
	if(mp ==0){
		LFT_UP.msg_sak=bufcall(LFTMSGSIZE,BPRI_MED,lftKiSak,0);
		return;
	}
	LFT_UP.msg_sak=0;
	mp->b_datap->db_type = M_PCSIG;
	*(unsigned char *)mp->b_rptr = SIGSAK;
	mp->b_wptr = mp->b_rptr+ sizeof (unsigned char);
	putnext(lft_ptr->strlft->lft_rdqp,mp);
GS_EXIT_TRC0(HKWD_GS_LFT,lftki,1,lftKiCb);
return ;
}

/***********************************************************
static int lftKiOffl (void);

LFT Keyboard off level interrupt function (priority INTOFFL3) 
scheduled by Key board interface call back function.  

This function takes each event (containing the scan code) in the 
inputring and passes it to the Scan Code Translator until the 
inputring is empty. Once the input ring is empty, or the current
message is full it sends it via putq stream's utility if it's possible.

This function is in charge of allocation of a new buffer if needed
by calling lft_buf_alloc.

This function can be scheduled by 4 ways :
		- lftKiCb: the normal way each time the input ring 
		  become non-empty.
		- lft_buf_alloc: there was no buffer avaliable so
		  we call bufcall but remember that we need to
		  reschedule when we have it.
		- lftrsrv: if an overflow condition occurs in the
		  up queue remember that we need to reschedule
		  when we can do a putnext.
		- lftwput: when this routine receives a M_STOPI
		  input stops until it receives a M_STARTI then 
		  reschedule lftKiOffl
To handle this we have indicators in up_stream struct: 
	- msg_alloc set by lft_buf_alloc when issuing a bufcall
	  reset when allocb succeed.
	- UPQ_OVFL set by lftrsrv when canput felt reset when
	  putnext done.
	- stop_intput set by lftwput when M_STOPI received 
	  reset when M_STARTI received.
Each time one of those indicators are reset lftKiOffl is 
rescheduled if input ring is not empty. Spurious scheduling
can occur in this case lftKiOffl returns without doing anything.
A test is done on the validity of the inputring and lftptr address to
avoid problems with close (freed of memory).

INPUT : Keyboard events from inputring
OUTPUT: Keyboard event passed to scan code translator

RETURNS : -1 if there is an overflow in input_ring or some
           other blocking condition, SUCCESS otherwise.

ERROR CODES LOGGED:

***********************************************************/

void lftKiOffl ()
{
struct ir_kbd ir_kbd;
struct inputring *ip;
mblk_t *mp;
int evt;
int old_intr;

/* TRACE "Entering lftKiOffl"*/
GS_ENTER_TRC0(HKWD_GS_LFT,lftki,1,lftKiOffl);
/* just to ensure to be not disturbed */
	old_intr = i_disable(INT_LFT);
	LFT_UP.input_flag|=OFFL_INUSE;
	i_enable(old_intr);

	if((lft_ptr->strlft==-1)||(LFT_UP.l_ip==-1)){
		/* close in progress */
		GS_EXIT_TRC0(HKWD_GS_LFT,lftki,1,lftKiOffl);
		return;
	}
	mp = LFT_UP.in_process;

/* input stopped */
	old_intr = i_disable(INT_LFT);
	if(LFT_UP.input_flag&STOP_INPUT){
		LFT_UP.input_flag&=~OFFL_INUSE;
		i_enable(old_intr);
/* TRACE "Leaving lftKiOffl input stopped"*/
		GS_EXIT_TRC0(HKWD_GS_LFT,lftki,1,lftKiOffl);
		return;
	}
	i_enable(old_intr);

/* flush request has been issued */
	old_intr = i_disable(INT_LFT);
	if(LFT_UP.input_flag&TO_BE_FLUSH){
		LFT_UP.input_flag&=~TO_BE_FLUSH;
		LFT_UP.input_flag&=~OFFL_INUSE;
		i_enable(old_intr);
		(*lft_ptr->strlft->lft_ksvtbl[KSVRFLUSH])(lft_ptr->dds_ptr->kbd.devno);
/* TRACE "Leaving lftKiOffl flush requested"*/
		GS_EXIT_TRC0(HKWD_GS_LFT,lftki,1,lftKiOffl);
		return;
	}
	i_enable(old_intr);

/* Last time we left there was no msg block avaliable */
	if(LFT_UP.in_process == NULL){
/* Now there is one */
		if(LFT_UP.next_buf != NULL){
			mp = LFT_UP.in_process = LFT_UP.next_buf;
			LFT_UP.next_buf = NULL;
/* allocate a new one */
			lft_buf_alloc();
			if(LFT_UP.key_struc.key_buff_cnt != 0){
				bcopy(LFT_UP.key_struc.ascii_buffer,
				      (char *)mp->b_wptr,
				      LFT_UP.key_struc.key_buff_cnt);
				mp->b_wptr += LFT_UP.key_struc.key_buff_cnt;
				LFT_UP.key_struc.key_buff_cnt = 0;
			}
		}else{
			old_intr = i_disable(INT_LFT);
			LFT_UP.input_flag&=~OFFL_INUSE;
			i_enable(old_intr);
/* TRACE "Leaving lftKiOffl no buffer avaliable"*/
			GS_EXIT_TRC0(HKWD_GS_LFT,lftki,1,lftKiOffl);
			return;
		}
	}

/* Flow control management */
	old_intr = i_disable(INT_LFT);
	if(LFT_UP.input_flag&UPQ_OVFL){
		LFT_UP.input_flag&=~OFFL_INUSE;
		i_enable(old_intr);
/* TRACE "Leaving lftKiOffl upper queue overflow"*/
		GS_EXIT_TRC0(HKWD_GS_LFT,lftki,1,lftKiOffl);
		return;
	}
	i_enable(old_intr);

/* input ring events processing */

	evt = 0;
	ip=LFT_UP.l_ip;
	while(REMAINING_EVTS){
		bcopy(ip->ir_head,&ir_kbd,sizeof(struct ir_kbd));
		if(ip->ir_head == LFT_UP.last_evt)
			ip->ir_head = LFT_UP.first_evt;
		else
			ip->ir_head+= sizeof(struct ir_kbd);
/* Process the event */
		lftst(&ir_kbd);
/* clear the overflow condition if water mark reached */
		if((evt == IR_WM_EVTS)&&(ip->ir_overflow==IROVERFLOW)){
			evt = 0;
			ip->ir_overflow=IROFCLEAR;
		}
		evt++;
/* If there is room in current msg block */
		if((mp->b_datap->db_lim - mp->b_wptr)>
		   LFT_UP.key_struc.key_buff_cnt){
			bcopy(LFT_UP.key_struc.ascii_buffer,
			      (char *)mp->b_wptr,
			      LFT_UP.key_struc.key_buff_cnt);
			mp->b_wptr += LFT_UP.key_struc.key_buff_cnt;
			LFT_UP.key_struc.key_buff_cnt = 0;
		}else{

/* No place send the previous buffer */
			putq(lft_ptr->strlft->lft_rdqp,mp);
			mp = LFT_UP.in_process = NULL;
			if(LFT_UP.next_buf != NULL){
				mp = LFT_UP.in_process = LFT_UP.next_buf;
				LFT_UP.next_buf = NULL;
/* allocate a new one */
				lft_buf_alloc();
				if(LFT_UP.key_struc.key_buff_cnt != 0){
					bcopy(LFT_UP.key_struc.ascii_buffer,
					      (char *)mp->b_wptr,
					      LFT_UP.key_struc.key_buff_cnt);
					mp->b_wptr += LFT_UP.key_struc.key_buff_cnt;
					LFT_UP.key_struc.key_buff_cnt = 0;
				}
			}else{
				old_intr = i_disable(INT_LFT);
				LFT_UP.input_flag&=~OFFL_INUSE;
				i_enable(old_intr);
/* TRACE "Leaving lftKiOffl no buffer avaliable"*/
				GS_EXIT_TRC0(HKWD_GS_LFT,lftki,1,lftKiOffl);
				return;
			}
/* Flow control management */
			old_intr = i_disable(INT_LFT);
			if(LFT_UP.input_flag&UPQ_OVFL){
				LFT_UP.input_flag&=~OFFL_INUSE;
				i_enable(old_intr);
/* TRACE "Leaving lftKiOffl upper queue overflow"*/
				GS_EXIT_TRC0(HKWD_GS_LFT,lftki,1,lftKiOffl);
				return;
			}
			i_enable(old_intr);
		}
	}	/* end while */

/* input ring is empty */
/* send the remaining buffer if thereis something in it */
	if(mp->b_wptr!=mp->b_rptr){
		putq(lft_ptr->strlft->lft_rdqp,mp);
		LFT_UP.in_process = NULL;
		if(LFT_UP.next_buf != NULL){
			LFT_UP.in_process = LFT_UP.next_buf;
			LFT_UP.next_buf = NULL;
/* allocate a new one */
			lft_buf_alloc();
		}
	}
	old_intr = i_disable(INT_LFT);
	LFT_UP.input_flag&=~OFFL_INUSE;
	i_enable(old_intr);

GS_EXIT_TRC0(HKWD_GS_LFT,lftki,1,lftKiOffl);
return;
}

/***********************************************************
lftKiInit();
PROCESS :
This function initializes the keyboard interface.

INPUT : NONE

OUTPUT: Keyboard device driver is opened
	Keyboard interface ans structures are initialized and set
	allocated memory is pinned

RETURNS : error code of fp_opendev, fp_ioctl and xmalloc
		otherwise 0.

ERROR CODES LOGGED:

***********************************************************/
int lftKiInit ()
{
int rc;
unsigned int ir_size;
dev_t devno;
struct inputring *ir;
struct kregring kr;
struct file *fp;

GS_ENTER_TRC1(HKWD_GS_LFT,lftki,1,lftKiInit,lft_ptr);

	devno = lft_ptr->dds_ptr->kbd.devno;     /* Get devno  */

/* Open device*/
   	if ( devno != -1 ){
		rc = fp_opendev( devno, DREAD, NULL, NULL, 
				&fp );
	}else{
#ifndef KBDRQD
		return (SUCCES);
#else
		GS_EXIT_TRC1(HKWD_GS_LFT,lftki,1,lftKiInit,ENODEV);
		lfterr(NULL,"LFTDD", "lftKiInit", NULL, 0, LFT_STR_NOKBD, UNIQUE_1);
		return (ENODEV);
#endif
	}
/* Open device failed */
   	if ( rc != SUCCESS ) {
		lfterr(NULL,"LFTDD", "lftKiInit", "fp_opendev", rc, LFT_STR_OPENDEV, UNIQUE_2);
		GS_EXIT_TRC1(HKWD_GS_LFT,lftki,1,lftKiInit,rc);
		return(rc);
	}
	lft_ptr->dds_ptr->kbd.fp=fp;

/* query service vector */
	rc = fp_ioctl(fp,KSQUERYSV,&lft_ptr->strlft->lft_ksvtbl,0);
/* query service vector failed*/
	if ( rc != SUCCESS ) {
		lfterr(NULL,"LFTDD", "lftKiInit", "fp_ioctl", rc, LFT_STR_IOCTL, UNIQUE_3);
		GS_EXIT_TRC1(HKWD_GS_LFT,lftki,1,lftKiInit,rc);
		fp_close(lft_ptr->dds_ptr->kbd.fp);
		return(rc);
	}

/* query keyboard device identifier */
	rc = fp_ioctl(fp,KSQUERYID,&lft_ptr->dds_ptr->kbd.kbd_type,0);
/* query keyboard device identifier failed */
	if ( rc != SUCCESS ) {
		lfterr(NULL,"LFTDD", "lftKiInit", "fp_ioctl", rc, LFT_STR_IOCTL, UNIQUE_4);
		GS_EXIT_TRC1(HKWD_GS_LFT,lftki,1,lftKiInit,rc);
		fp_close(lft_ptr->dds_ptr->kbd.fp);
		return(rc);
	}

/* Allocates and initializes the inputring buffer */
	ir_size = sizeof(struct inputring)+
		  IR_NB_EVTS*sizeof(struct ir_kbd);
	LFT_UP.l_ip= (struct inputring *)
		xmalloc(ir_size,3,pinned_heap );
	if(LFT_UP.l_ip==NULL){
		lfterr(NULL,"LFTDD", "lftKiInit", "xmalloc", NULL, LFT_STR_ERRMEM, UNIQUE_5);
		GS_EXIT_TRC1(HKWD_GS_LFT,lftki,1,lftKiInit,ENOMEM);
		fp_close(lft_ptr->dds_ptr->kbd.fp);
		return (ENOMEM);
	}

/* This is for an easier wrap in input ring buffer */
	LFT_UP.first_evt = (caddr_t)LFT_UP.l_ip+sizeof(struct inputring);
	LFT_UP.last_evt = LFT_UP.first_evt+((IR_NB_EVTS-1)*sizeof(struct ir_kbd));


/* set input ring buffer and call back function */
	LFT_UP.l_ip->ir_size = 
		IR_NB_EVTS*sizeof(struct ir_kbd);
	LFT_UP.l_ip->ir_head = 
	LFT_UP.l_ip->ir_tail =
	LFT_UP.first_evt;
	LFT_UP.l_ip->ir_overflow = IROFCLEAR;
	LFT_UP.l_ip->ir_notifyreq = IRSIGEMPTY;

/* set argument for ioctl */
	kr.ring = (caddr_t) LFT_UP.l_ip;
	kr.report_id = lft_ptr->dds_ptr->kbd.kbd_type;
	kr.notify_callback = lftKiCb;
	kr.sak_callback = lftKiSak;

/* register inputring */
	rc = fp_ioctl(fp,KSREGRING,&kr,0);
/* register inputring failed */
	if ( rc != SUCCESS ) {
		lfterr(NULL,"LFTDD", "lftKiInit", "fp_ioctl", rc, LFT_STR_IOCTL, UNIQUE_6);
		GS_EXIT_TRC1(HKWD_GS_LFT,lftki,1,lftKiInit,rc);
		fp_close(lft_ptr->dds_ptr->kbd.fp);
		xmfree((caddr_t)LFT_UP.l_ip,pinned_heap);
		return(rc);
	}
/* keystroke xlate buffer is empty             */
	LFT_UP.key_struc.key_buff_cnt = 0;
/* from this point lftKiCb can be called */
GS_EXIT_TRC1(HKWD_GS_LFT,lftki,1,lftKiInit,SUCCES);
return(SUCCES);
}

/***********************************************************
lftKiTerm(struct inputring *ip);

This function terminates the keyboard driver .
INPUT : NONE

OUTPUT: keyboard device driver is closed.
	allocated memory is freed

RETURNS : error code of fp_close and xmfree
		otherwise 0.

ERROR CODES LOGGED:

***********************************************************/
lftKiTerm(struct inputring *ip)
{
int rc_close,rc_free;

GS_ENTER_TRC1(HKWD_GS_LFT,lftki,1,lftKiTerm,ip);
rc_free = rc_close = 0;
/* close the keyboard (fp_close) */
#ifndef KBDRQD
	if (lft_ptr->dds_ptr->kbd.devno == -1)      /* no keyboard attached  */
		return(SUCCES);
#endif
	if(lft_ptr->dds_ptr->kbd.fp != -1){
		rc_close = fp_close(lft_ptr->dds_ptr->kbd.fp);
		lft_ptr->dds_ptr->kbd.fp = -1;
	}else{
		lfterr(NULL,"LFTDD", "lftKiTerm", NULL, 0, LFT_STR_NOKBD, UNIQUE_7);
	}
/* From this point inputring is unregistered */
/* deallocation of inputring buffer */
	rc_free = xmfree((caddr_t)ip,pinned_heap );
	if ( rc_free != 0 ){
		lfterr(NULL,"LFTDD", "lftKiTerm", "xmfree", rc_free, LFT_STR_ERRMEM, UNIQUE_8);
		GS_EXIT_TRC1(HKWD_GS_LFT,lftki,1,lftKiTerm,rc_free);
		return(rc_free);
	}
	if ( rc_close != 0 ){
		lfterr(NULL,"LFTDD", "lftKiTerm", "fp_close", rc_close, LFT_STR_CLOSE, UNIQUE_9);
		return(rc_close);
		GS_EXIT_TRC1(HKWD_GS_LFT,lftki,1,lftKiTerm,rc_close);
	}
GS_EXIT_TRC1(HKWD_GS_LFT,lftki,1,lftKiTerm,SUCCES);
return(SUCCES);
}
