static char sccsid[] = "@(#)38	1.12  src/bos/kernext/lft/streams/lftsi.c, lftdd, bos41J, 9517B_all 4/26/95 17:36:58";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: lft_buf_alloc
 *		lft_ctl
 *		lft_flush
 *		lftclose
 *		lftopen
 *		lftrsrv
 *		lftwput
 *		lftwsrv
 *
 *   ORIGINS: 27, 83
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

  This main line file contains the driver streams interface routines:
     lftopen   - open lft
     lftclose  - close lft
     lftwput   - write put queue procedure
     lftwsrv   - write service procedure
     lftrsrv   - read service procedure
     lftrint   - off level interrupt procedure
  ------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/malloc.h>
#include <sys/errno.h>		/* System error numbers */
#include <sys/termio.h>		/* Terminal input/output */
#include <sys/syspest.h>

/* Private includes
   ================
*/
#include <lft.h>	
#include <sys/lft_ioctl.h>
#include <sys/inputdd.h>
#include <sys/display.h>
#include <graphics/gs_trace.h>
#include <lft_debug.h>

/* Streams and tty includes
   ========================
*/
#include <sys/stream.h>
#include <sys/stropts.h>
#include <unistd.h>
#include <sys/ttydefaults.h>
#include <sys/str_tty.h>
#include <lftsi.h>		/* LFT streams information */

GS_MODULE(lftsi);
BUGVDEF(db_lftsi, 0);                  /* define initial debug level */


static struct tioc_reply     /* structure for tioc-module/M_CTL */
lft_tioc_reply[6] = {
        { LFT_SET_DFLT_DISP, NAMESIZE, TTYPE_COPYIN },
        { LFT_ACQ_DISP, sizeof(dev_t), TTYPE_COPYIN },
        { LFT_REL_DISP, sizeof(dev_t), TTYPE_COPYIN },
        { LFT_SET_DIAG_OWNER, sizeof(dev_t), TTYPE_COPYIN },
        { LFT_QUERY_LFT, sizeof(lft_query_t), TTYPE_COPYOUT },
        { LFT_QUERY_DISP, sizeof(lft_disp_query_t), TTYPE_COPYINOUT },
};
extern int lftKiOffl();
void lft_buf_alloc();
void lft_flush();

extern int lft_pwrmgr();     /* Display Power Management (pwr_mgr/pwr_mgr.c) */

/**********************************************************************
Function : lftopen
Input parameters :
	q	: Pointer to LFT read queue
	devp	: Pointer to Major and minor device numbers
	flag	: Open file control flags
	sflag	: Steams open flags
	credp	: Pointer  to Extension parameter for device dependent 
		  functions

Remark : stream head ensure that opens are serialized no need locking

**********************************************************************/
int lftopen (
queue_t *q,
dev_t *devp,
int flag,
int sflag,
cred_t *credp)
{
	int     rc;                     /* return code                  */
	int      i;
	int min_dev;
	uint status;
	mblk_t *mp;
	struct termios *tios;
	struct vtmstruc *vp;


/* TRACE "Entering lftopen" */
GS_ENTER_TRC(HKWD_GS_LFT,lftsi,1,lftopen,q,*devp,0,0,0);

	rc=0;

	min_dev = minor( *devp );       /* Determine the minor number   */

	if(lft_ptr == NULL){
		BUGLPR(db_lftsi,BUGNFO,("lft_ptr not initialized\n"));
           	lfterr(NULL,"LFTDD", "lftopen", NULL, 0, LFT_STR_NOINIT, UNIQUE_1);
		return(ENODEV);			/* should not occur */
	}
	if(lft_ptr->initialized==0){
		BUGLPR(db_lftsi,BUGNFO,("lft not initialized\n"));
           	lfterr(NULL,"LFTDD", "lftopen", NULL, 0, LFT_STR_NOINIT, UNIQUE_2);
		return(ENODEV);
	}
	if(lft_ptr->open_count == 0){ /* This is the first open */
/* allocation of strlft structure */
		lft_ptr->strlft = 
			(strlft_ptr_t)xmalloc(sizeof(strlft_t),3,pinned_heap);
		if(lft_ptr->strlft==NULL){
			BUGLPR(db_lftsi,BUGNFO,("lft Memory allocation failed (strlft)\n"));
           		lfterr(NULL,"LFTDD", "lftopen", NULL, 0, LFT_STR_ERRMEM, UNIQUE_3);
			return (ENOMEM);
		}
		bzero((char * )lft_ptr->strlft,sizeof(strlft_t));
/* initialization of strlft structure:
   up_stream :
   		- Offlevel handler info initialization
		- reset of indicators 
		- 1rst message allocation (lft_alloc_buf)
		- Flags init
		- reset ascii seq buffer
		- reset debugging dump of last 32 keystrokes
   down_stream :
		- Flags init
		- line_data init

   others :
		- init termios struct
		- keyboard initialization (lftKiInit)
 */

		INIT_OFFL3( &LFT_UP.offl_hdl, lftKiOffl, 0 );
		LFT_UP.msg_sak=LFT_UP.msg_alloc=0;
		LFT_UP.next_buf=NULL;
		lft_buf_alloc();
		LFT_UP.in_process=NULL;
		LFT_UP.input_flag=0;
		LFT_UP.ds_state_flag=INIT_STATE;
		LFT_UP.state_flag=ALPHANUM_STATE;
		LFT_UP.ichrstr_len=0;

		LFT_DOWN.stop_output=0;
		LFT_DOWN.parser_state=NORML;
		LFT_DOWN.numparm=0;
		LFT_DOWN.ds_mode_flag=DEFAULT_DS_MODES;	/* set the wrap mode */

/*------------
alloc and free line_data are no more mandatory since the screen
has a fixed size (SCR_WIDTH*SCR_HEIGHT) so line data area is
a fixed size table allocated inside strlft struct.
  ------------*/
		/* Initialize each line */
		for ( i=1; i <= SCR_HEIGHT; ++i ) {
			LFT_DOWN.line_data[i].line_length = 0;
			/* set tab rack back to default and set tab at SCR_WIDTH */
			LFT_DOWN.line_data[i].tab_settings[0] = DEF_TAB_RACK1;
			LFT_DOWN.line_data[i].tab_settings[1] = DEF_TAB_RACK2;
			LFT_DOWN.line_data[i].tab_settings[2] = DEF_TAB_RACK3;
		}

		lft_ptr->strlft->lft_mode_flag=INIT_MOD_FLAG;
		tios = &lft_ptr->strlft->lft_tios;
				/* For defect #155851, I changed NCC to NCCS */
				/* in the following line of code...	     */
		bcopy(ttydefchars, tios->c_cc, NCCS); /* from now ldtty... */
		tios->c_iflag = TTYDEF_IFLAG;
		tios->c_oflag = TTYDEF_OFLAG;
		tios->c_lflag = TTYDEF_LFLAG;
		/* We never actually use c_cflag -- we just don't want it to
		 * look ridiculous in "stty -a" output.
		 */
		tios->c_cflag = TTYDEF_CFLAG;
		tios->c_cflag &= ~_CBAUD;
		tios->c_cflag |= (TTYDEF_SPEED & _CBAUD);

		lft_ptr->strlft->lft_rdqp = q;

/*------------
 Keyboard initialization
 ------------*/
		if(rc=lftKiInit()){
/*------------
 Release all the allocated stuff
 ------------*/
			if(LFT_UP.msg_alloc!=0){
				unbufcall(LFT_UP.msg_alloc);
				LFT_UP.msg_alloc=0;
			}
			if(LFT_UP.next_buf!=NULL){
				freemsg(LFT_UP.next_buf);
			}
			xmfree(lft_ptr->strlft,pinned_heap);
			lft_ptr->strlft=-1;
			BUGLPR(db_lftsi,BUGNFO,("lft keyboard initialization failed: %d\n",rc));
           		lfterr(NULL,"LFTDD", "lftopen", "lftKiInit", rc, LFT_STR_KBDINIT, UNIQUE_4);
			return(rc);
		}
	}else{  /* this is not the 1rst open */
/*----------
 In case of exclusive open we cannot open it again
 ----------*/
		if(lft_ptr->strlft->lft_mode_flag&OPENEXCL) return(EBUSY); /* only one open at a time */
	}
	lft_ptr->open_count++;

/* TRACE "Leaving lftopen" */
	GS_EXIT_TRC1(HKWD_GS_LFT,lftsi,1,lftopen, SUCCES);
return (SUCCES);
}
/**********************************************************
Function : lftclose 
Input Parametrs :
	q     : Pointer to LFT driver read queue
	flag  : Open file control flags
	credp : Pointer to Extension parameter for device
             dependent functions
**********************************************************/

int lftclose (
queue_t *q,
int     flag,
cred_t *credp
)
{
	mblk_t *bye_msg;
	int     rc,old_intr;
	struct inputring *tmp_ip;
/*
 * All the memory allocated during the lft_open routine
 * is deallocated here.
 */

/* LFTERR TRACE "Entering lftclose" */
GS_ENTER_TRC1(HKWD_GS_LFT,lftsi,1,lftclose,q);
	rc=0;
	if(lft_ptr == NULL){
		BUGLPR(db_lftsi,BUGNFO,("lft not initialized\n"));
           	lfterr(NULL,"LFTDD", "lftclose", NULL, 0, LFT_STR_NOINIT, UNIQUE_5);
		return( ENODEV );        /* No such device or address */
	}
/*--------
 Now ensure that the offlevel routine doesn't work at the time.
 If it's still working we just wait and then we just reset the
 inputring pointer to aware it it's no more time to play.
 --------*/
	old_intr = i_disable(INT_LFT);
	while(LFT_UP.input_flag&OFFL_INUSE){
		i_enable(old_intr);
		delay(10);
		old_intr = i_disable(INT_LFT);
	}
	tmp_ip=LFT_UP.l_ip;
	LFT_UP.l_ip=-1;
	i_enable(old_intr);
		
/*--------
 Keyboard termination
 --------*/
	if(rc=lftKiTerm(tmp_ip)){
		BUGLPR(db_lftsi,BUGNFO,("lft keyboard termination error: %d\n",rc));
           	lfterr(NULL,"LFTDD", "lftclose", "lftKiTerm", rc, LFT_STR_KBDTERM, UNIQUE_6);
		return(rc);
	}

/*--------
 send HANGUP upstream
--------*/
	bye_msg = allocb(0, BPRI_WAITOK);
	if(bye_msg != NULL){
		bye_msg->b_datap->db_type = M_HANGUP;
		putnext(lft_ptr->strlft->lft_rdqp, bye_msg);
	}else{
           	lfterr(NULL,"LFTDD", "lftclose", "allocb", 0, LFT_STR_NOMSG, UNIQUE_7);
	}

/*--------
 cancel bufcall calls if there are some
 free allocated buffers if there are some
 flush the queues
 --------*/
	if(LFT_UP.msg_alloc!=0){
		unbufcall(LFT_UP.msg_alloc);
		LFT_UP.msg_alloc=0;
	}
	if(LFT_UP.msg_sak!=NULL){
		unbufcall(LFT_UP.msg_sak);
		LFT_UP.msg_sak=0;
	}
	if(LFT_UP.next_buf!=NULL){
		freemsg(LFT_UP.next_buf);
	}
	lft_flush(FLUSHR|FLUSHW,1,0);
	
/*--------
 deallocate strlft struct
 --------*/

	rc=xmfree((caddr_t)lft_ptr->strlft,pinned_heap);
	lft_ptr->strlft=-1;
	lft_ptr->open_count=0;
	if(rc!=0){
		BUGLPR(db_lftsi,BUGNFO,("lft memory deallocation failure: %d\n",rc));
           	lfterr(NULL,"LFTDD", "lftclose", "xmfree", rc, LFT_STR_ERRMEM, UNIQUE_8);
		return(rc);
	}
/* LFTERR TRACE "Leaving lftclose" */
	GS_EXIT_TRC1(HKWD_GS_LFT,lftsi,1,lftclose,SUCCES);
	return (SUCCES);
}
/**********************************************************
Function : lft_flush 
Input Parametrs :
	rw      : Flag read or write queue to be flushed
	message : If an M_CTL FLUSH message is to be sent
	kbd_flsh: If the inputring is to be flush.
Remark : the TO_BE_FLUSH flag is set to aware the offlevel
         routine to flush the input ring. This is for 
	 synchro problems.
	 
**********************************************************/

static void
lft_flush(int rw,int message,int kbd_flsh)
{
int old_intr;

	if (rw & FLUSHW) {
		flushq(WR(lft_ptr->strlft->lft_rdqp),FLUSHDATA);
	}
	if (rw & FLUSHR) {
		flushq((lft_ptr->strlft->lft_rdqp),FLUSHDATA);
		if (message){
			putctl1((lft_ptr->strlft->lft_rdqp),M_FLUSH,FLUSHR);
		}else{
			if(kbd_flsh){
				old_intr = i_disable(INT_LFT);
				LFT_UP.input_flag|=TO_BE_FLUSH;
				i_enable(old_intr);
			}
		}
	}
}
/***********************************************************
Function : lft_buf_alloc
Input Parametrs : NONE
***********************************************************/

void lft_buf_alloc ()
{
int old_intr;

	/* should not be necessary */
	if(LFT_UP.next_buf == NULL){
		LFT_UP.next_buf=allocb(LFTMSGSIZE,BPRI_LO);
		if(LFT_UP.next_buf == NULL){
			LFT_UP.msg_alloc=bufcall(LFTMSGSIZE,BPRI_LO,lft_buf_alloc,0);
		}else{
			/* initialized as a M_DATA buffer by default */
			if (LFT_UP.msg_alloc){
				LFT_UP.msg_alloc=0;
				old_intr = i_disable(INT_LFT);
				if(!(LFT_UP.input_flag&OFFL_INUSE)){
					/* schedule lftKiOffl by calling i_sched */
					i_sched(&LFT_UP.offl_hdl);
				}
				i_enable(old_intr);
			}
		}
	}
}
/***********************************************************
Function : lftwput
Input Parametrs :
	q  : LFT driver write queue
	mp : Message pointer
***********************************************************/

void lftwput (
queue_t *q,
mblk_t *mp
)
{
	struct phys_displays * pd;

	register mblk_t *nmp;
	switch (mp->b_datap->db_type) {

	case M_BREAK:
	case M_DELAY:
		freemsg(mp);
		break;
	case M_DATA:
		/* if nomore left, it returns */
		while (mp) {
			nmp = unlinkb(mp);
			putq(q,mp);
			mp = nmp;
		}
		break;
	case M_IOCTL:
	{
		lft_query_t *qry;
		lft_disp_query_t *qry_d;
		lft_disp_info_t *disp;
		struct copyreq *cqp;
		struct vtmstruc * vtm;
		struct termios *tios;
		struct iocblk *iocp;
		struct winsize winsize;
		struct intr *intr;
		unsigned char outtype;
		unsigned long outcnt;
		dev_t *nodev;
		int i,rc,nb_disp,nam_ln;

		/* unexpected transparent ioctls */

		iocp = (struct iocblk *)mp->b_rptr;
		if (iocp->ioc_count == TRANSPARENT) {
			mp->b_datap->db_type = M_IOCNAK; /*Not Okay */
			iocp->ioc_error = EINVAL;
			qreply(q,mp);
			return;
		}
		iocp->ioc_error =0;
		outtype = M_IOCACK;
		outcnt = 0;
		
		switch (iocp->ioc_cmd) {
			/*
			 * LFT SPECIAL IOCTLS
			 */
		case LFT_SET_DFLT_DISP :
			putq(q,mp);
			return;
		case LFT_REL_DISP :
			nodev= (dev_t *)mp->b_cont->b_rptr;
			for(i=0;i<lft_ptr->dds_ptr->number_of_displays;i++){
				if(*nodev==lft_ptr->dds_ptr->displays[i].devno){
					lft_ptr->dds_ptr->displays[i].fp_valid = TRUE;
					/* If DIAGEX is the owner of the display */
					/* we've got to reinitialize the intr hdl */
					/* since diagnostics had registered its own */
					if(lft_ptr->dds_ptr->displays[i].flags & APP_IS_DIAG){
						/* get the pointer to the interrupt struct */
						intr=&lft_ptr->dds_ptr->displays[i].vtm_ptr->display->interrupt_data.intr;
						/* reinstate the interrupt hdl */
						if(intr->handler)
						if((rc=i_init(intr))==INTR_FAIL){
							lfterr(NULL,"LFTDD", "lftwput", "i_init", rc, LFT_STR_HDLINIT, UNIQUE_10);
						}
						/* Clear the diagnostic flag */
					lft_ptr->dds_ptr->displays[i].flags &= ~APP_IS_DIAG;
					}

					/*
 					 * if the released display is the default display, then
 					 * we need to restart the watchdog timer for Display Power
                                         * Management. 
 					 */
					if ((lft_ptr->dds_ptr->pwr_mgr_time[0] != 0) && lft_ptr->dds_ptr->enable_dpms)
					{
					   if (i == lft_ptr->dds_ptr->default_disp_index)
					   {
   					      lft_pwrmgr(START_LFT_DPM_WD);
					   }
					   else  /* for non default display, we just turn power off */
					   {
					      pd = lft_ptr->dds_ptr->displays[i].vtm_ptr->display;

					      if (pd->vttpwrphase != NULL)
					      {
					         (*pd->vttpwrphase)(pd,4);
					      }
					   }
					}

					break;
				}
			}
			/* we didn't found it */
			if(i==lft_ptr->dds_ptr->number_of_displays){
				lfterr(NULL,"LFTDD", "lftwput", NULL, 0, LFT_STR_NODISP, UNIQUE_11);
				iocp->ioc_error = ENODEV;
				outtype = M_IOCNAK;
			}
			break;
		case LFT_ACQ_DISP :
			nodev= (dev_t *)mp->b_cont->b_rptr;
			for(i=0;i<lft_ptr->dds_ptr->number_of_displays;i++){
				if(*nodev==lft_ptr->dds_ptr->displays[i].devno){
					if(lft_ptr->dds_ptr->displays[i].fp_valid==FALSE){
						lfterr(NULL,"LFTDD", "lftwput", NULL, 0, LFT_STR_BUSYD, UNIQUE_12);
						iocp->ioc_error = EBUSY;
						outtype = M_IOCNAK;
					}else{
						/*
 						 * if the acquired display is the default display,
 						 * we stop the watchdog timers for LFT Display Power Management. 
 						 * Also, we want to turn on the display 
 						 */
						if ((lft_ptr->dds_ptr->pwr_mgr_time[0] != 0) && lft_ptr->dds_ptr->enable_dpms)
						{
						   if (i == lft_ptr->dds_ptr->default_disp_index)
						   {
   						      lft_pwrmgr(STOP_LFT_DPM_WD);
						   }
						   pd = lft_ptr->dds_ptr->displays[i].vtm_ptr->display;
						   if (pd->vttpwrphase != NULL)
						   {
						      (*pd->vttpwrphase)(pd,1);
						   }
						}

						lft_ptr->dds_ptr->displays[i].fp_valid = FALSE;
					}
					break;
				}
			}
			/* we didn't found it */
			if(i==lft_ptr->dds_ptr->number_of_displays){
				lfterr(NULL,"LFTDD", "lftwput", NULL, 0, LFT_STR_NODISP, UNIQUE_13);
				iocp->ioc_error = ENODEV;
				outtype = M_IOCNAK;
			}
			break;
		case LFT_SET_DIAG_OWNER :
			nodev= (dev_t *)mp->b_cont->b_rptr;
			for(i=0;i<lft_ptr->dds_ptr->number_of_displays;i++){
				if(*nodev==lft_ptr->dds_ptr->displays[i].devno){
					if(lft_ptr->dds_ptr->displays[i].fp_valid==TRUE){
						/* LFT_ACQ_DISP hasn't been issued */
						lfterr(NULL,"LFTDD", "lftwput", NULL, 0, LFT_STR_BUSYD, UNIQUE_14);
						iocp->ioc_error = EBUSY;
						outtype = M_IOCNAK;
					}else{
						/* set the diagnostic info and cancel interupts */
						lft_ptr->dds_ptr->displays[i].flags |= APP_IS_DIAG;
						intr=&lft_ptr->dds_ptr->displays[i].vtm_ptr->display->interrupt_data.intr;

						if(intr->handler)
						i_clear(intr);
					}
					break;
				}
			}
			/* we didn't found it */
			if(i==lft_ptr->dds_ptr->number_of_displays){
				lfterr(NULL,"LFTDD", "lftwput", NULL, 0, LFT_STR_NODISP, UNIQUE_15);
				iocp->ioc_error = ENODEV;
				outtype = M_IOCNAK;
			}
			break;
		case LFT_QUERY_LFT :
			i = sizeof(lft_query_t);
			if (iocp->ioc_count < i || !mp->b_cont) {
				lfterr(NULL,"LFTDD", "lftwput", NULL, 0, LFT_STR_NOSPC, UNIQUE_16);
				outtype = M_IOCNAK;
				iocp->ioc_error = ENOSPC;
				break;
			}
			qry = (lft_query_t *)mp->b_cont->b_rptr;
			/* Now compute the size of the buffer to be returned*/
			
			qry->lft_devno = lft_ptr->dds_ptr->lft.devno;
			qry->kbd_devno = lft_ptr->dds_ptr->kbd.devno;
			qry->number_of_displays = lft_ptr->dds_ptr->number_of_displays;
			qry->default_display = lft_ptr->dds_ptr->default_disp_index;
			if(lft_ptr->dds_ptr->swkbd_file!=NULL)
				strcpy(qry->swkbd_file,lft_ptr->dds_ptr->swkbd_file);
			else
				qry->swkbd_file[0]='\0';
			if(lft_ptr->dds_ptr->kbd.devname!=NULL)
				strcpy(qry->keyboard_name,lft_ptr->dds_ptr->kbd.devname);
			else
				qry->keyboard_name[0]='\0';
			qry->fkproc_started = lft_ptr->dds_ptr->start_fkproc;
			outcnt=sizeof(lft_query_t);
			break;
		case LFT_QUERY_DISP :
			i = sizeof(lft_disp_query_t);
			if (iocp->ioc_count < i || !mp->b_cont) {
				lfterr(NULL,"LFTDD", "lftwput", NULL, 0, LFT_STR_NOSPC, UNIQUE_17);
				outtype = M_IOCNAK;
				iocp->ioc_error = ENOSPC;
				break;
			}
			qry_d = (lft_disp_query_t *)mp->b_cont->b_rptr;
			LFT_DOWN.sv_dsp_info.num_of_disps = qry_d->num_of_disps;
			LFT_DOWN.sv_dsp_info.lft_disp = qry_d->lft_disp;
			nb_disp = MIN(qry_d->num_of_disps,lft_ptr->dds_ptr->number_of_displays);

			/* asking for an copy out */
			cqp = (struct copyreq *)mp->b_rptr;
			cqp->cq_size = nb_disp*sizeof(lft_disp_info_t);
			cqp->cq_addr = (caddr_t)qry_d->lft_disp;
			cqp->cq_flag = 0;
			
			freemsg(mp->b_cont);
			if((mp->b_cont = allocb(cqp->cq_size,BPRI_MED)) ==NULL){
				lfterr(NULL,"LFTDD", "lftwput", "allocb", 0, LFT_STR_NOMSG, UNIQUE_18);
				mp->b_cont=NULL;
				mp->b_datap->db_type = M_IOCNAK;
				iocp->ioc_error = EAGAIN;
				qreply(q,mp);
				return;
			}
			disp = (lft_disp_info_t *)mp->b_cont->b_rptr;
			for(i=0;i<nb_disp;i++){
				disp[i].disp_devno = lft_ptr->dds_ptr->displays[i].devno;
				strcpy(disp[i].disp_name,lft_ptr->dds_ptr->displays[i].devname);
				disp[i].disp_enable = lft_ptr->dds_ptr->displays[i].fp_valid;
			}
			mp->b_cont->b_wptr=mp->b_cont->b_rptr+cqp->cq_size;
			mp->b_datap->db_type = M_COPYOUT;
			mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
			qreply(q,mp);
			return;		/* next step M_IOCDATA */

			break;
			/* 
			 * STANDARD TTY IOCTLS
			 */
		case TIOCSETAW:
		case TIOCSETAF:
			putq(q,mp);
			return;
		case TCSBRK:
			lft_flush(FLUSHW|FLUSHR,0,0);
			outcnt=0;
			break;
		case TIOCGETA:
			if (iocp->ioc_count < sizeof(struct termios) || !mp->b_cont) {
				lfterr(NULL,"LFTDD", "lftwput", NULL, 0, LFT_STR_NOSPC, UNIQUE_19);
				outtype = M_IOCNAK;
				iocp->ioc_error = ENOSPC;
				break;
			}
			tios= (struct termios *)mp->b_cont->b_rptr;
			bcopy((char * )&lft_ptr->strlft->lft_tios,(char * )tios,sizeof(struct termios));
			outcnt = sizeof(struct termios);
			break;
		case TIOCSETA:
			if (iocp->ioc_count < sizeof(struct termios) || !mp->b_cont) {
				lfterr(NULL,"LFTDD", "lftwput", NULL, 0, LFT_STR_NOSPC, UNIQUE_20);
				outtype = M_IOCNAK;
				iocp->ioc_error = ENOSPC;
				break;
			}
			/* we just copy the structure for the next TC_GETA */
			/* No check done on values */
			tios= (struct termios *)mp->b_cont->b_rptr;
			bcopy((char * )tios,(char * )&lft_ptr->strlft->lft_tios,sizeof(struct termios));
			break;
		case TXTTYNAME:
			bcopy(lft_ptr->dds_ptr->lft.devname,(char *)mp->b_cont->b_rptr, NAMESIZE);
			outcnt=NAMESIZE;
			break;
			
			/* Get the window size only if default display is set*/
		case TIOCGWINSZ:
			if (NO_DEFLT_DISP){
				lfterr(NULL,"LFTDD", "lftwput", NULL, 0, LFT_STR_NODISP, UNIQUE_21);
				iocp->ioc_error = ENODEV;
				outtype = M_IOCNAK;
			}else{

				vtm = lft_ptr->dds_ptr->displays[DEFLT_DISP_IDX].vtm_ptr;
				winsize.ws_row = SCR_HEIGHT;
				winsize.ws_col = SCR_WIDTH;
				winsize.ws_xpixel = vtm->fonts->font_width*SCR_WIDTH;
				winsize.ws_ypixel = vtm->fonts->font_height*SCR_HEIGHT;
				bcopy(&winsize, mp->b_cont->b_rptr, sizeof(struct winsize));
				outcnt=sizeof(struct winsize);
			}
			break;
			
		case TIOCEXCL:
			lft_ptr->strlft->lft_mode_flag|=OPENEXCL;
			break;

		case TIOCNXCL:
			lft_ptr->strlft->lft_mode_flag&=~OPENEXCL;
			break;

		case TIOCOUTQ:
			/* ldterm should have already put its stuff we just add ours */
			*(int *)mp->b_cont->b_rptr += lft_ptr->strlft->lft_rdqp->q_count;
		break;

		case TIOCSWINSZ:
			/* unchangeable */
			break;
		case TCSAK:
		{
		int cmdsak;
		register int	arg =*(int *)mp->b_cont->b_rptr;

			if (arg == TCSAKON) {
				cmdsak=KSSAKENABLE;
			}
			else if (arg == TCSAKOFF) {
				cmdsak=KSSAKDISABLE;
			}
#ifndef KBDRQD
			if (lft_ptr->dds_ptr->kbd.devno != -1)         /* keyboard attached */
#endif
			(*lft_ptr->strlft->lft_ksvtbl[KSVSAK])(lft_ptr->dds_ptr->kbd.devno,&cmdsak);
		}
		break;
			
		default:
			outtype = M_IOCNAK;
			iocp->ioc_error = EINVAL;
			break;
		}
		mp->b_datap->db_type = outtype;
		iocp->ioc_count = outcnt;
		if(mp->b_cont!=NULL)
			mp->b_cont->b_wptr = mp->b_cont->b_rptr + outcnt;
		qreply(q,mp);
		break;
	}
	case M_IOCDATA:	/* here only for aknowledgement of LFT_QUERY_DISP */
	{
		struct iocblk *iocp;
		struct copyresp *csp;
		csp = (struct copyresp *)mp->b_rptr;
		if(csp->cp_cmd != LFT_QUERY_DISP){ /* spurious M_IOCDATA */
			freemsg(mp);
			break;
		}
		if(csp->cp_rval){
			freemsg(mp);
			break;
		}
		iocp = (struct iocblk *)mp->b_rptr;
		/* as LFT_QUERY_DISP ioctl is an IN/OUT ioctl we must set the out struct */
		/* and first allocate it */
		if((mp->b_cont = allocb(sizeof(lft_disp_query_t),BPRI_MED)) ==NULL){
			mp->b_datap->db_type = M_IOCNAK;
			lfterr(NULL,"LFTDD", "lftwput", "allocb", 0, LFT_STR_NOMSG, UNIQUE_23);
			iocp->ioc_error = EAGAIN;
			qreply(q,mp);
			return;
		}
		/* then fill it */
		bcopy((char * )&LFT_DOWN.sv_dsp_info,(char * )mp->b_cont->b_rptr,
		      sizeof(lft_disp_query_t));
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(lft_disp_query_t);

		/* data successfully copied out: ack */
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_count = sizeof(lft_disp_query_t);
		mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
		iocp->ioc_rval = 0;
		iocp->ioc_error = 0;
		qreply(q,mp);
		
	}
		break;
	case M_CTL:
		lft_ctl(q,mp);
		break;
	case M_FLUSH:{
		int flag = 0;

		if(*mp->b_rptr & FLUSHW) {
			flag |= FLUSHW;
			*mp->b_rptr &= ~FLUSHW;
		}
		if( *mp->b_rptr & FLUSHR) {
			flag |= FLUSHR;
			qreply(q,mp);
		}
		else 
			freemsg(mp);
		lft_flush(flag,0,0);	
		}
		break;

	case M_STOP:
		LFT_DOWN.stop_output=1;
		freemsg(mp);
		break;
	case M_START:
		LFT_DOWN.stop_output=0;
		freemsg(mp);
		qenable(q);	/* to enable write service routine */
		break;
	case M_STOPI:
	{
		int old_intr;
		old_intr = i_disable(INT_LFT);
		LFT_UP.input_flag|=STOP_INPUT;
		i_enable(old_intr);
		freemsg(mp);
	}
		break;
	case M_STARTI:
	{
		int old_intr;
		old_intr = i_disable(INT_LFT);
		LFT_UP.input_flag&=~STOP_INPUT;
		i_enable(old_intr);
		freemsg(mp);
		old_intr = i_disable(INT_LFT);
		if(REMAINING_EVTS){
			/* schedule lftKiOffl by calling i_sched */
			i_sched(&LFT_UP.offl_hdl);
		}
		i_enable(old_intr);
	}
		break;
	default:
		freemsg(mp);
		break;
	}
}
int
lft_ctl(queue_t *q, mblk_t *mp)
{
    mblk_t *mp1;
    struct iocblk *iocp;
    struct termios *tios;

    iocp = (struct iocblk *)mp->b_rptr;
   
	switch (iocp->ioc_cmd) {
    	case TIOCGETA:
		if (!mp->b_cont) {
			mp1 = allocb(sizeof(struct termios), BPRI_MED);
			/*
			 * if allocation fails try again later
			 */
			if (!mp1){
				lfterr(NULL,"LFTDD", "lft_ctl", "allocb", 0, LFT_STR_NOMSG, UNIQUE_24);
				iocp->ioc_error = EAGAIN;
				break;
			}
			mp->b_cont = mp1;
		} else  {
			mp1 = mp->b_cont;
			if ( (mp1->b_datap->db_lim - mp1->b_rptr) 
			     < sizeof(struct termios) ) {
				freeb(mp1);
				mp1 = allocb(sizeof(struct termios), BPRI_MED);
				/*
				 * if allocation fails try again later
				 */
				mp->b_cont = mp1;
				if (!mp1){
					lfterr(NULL,"LFTDD", "lft_ctl", "allocb", 0, LFT_STR_NOMSG, UNIQUE_25);
					iocp->ioc_error = EAGAIN;
					break;
				}
			}
		}
		tios= (struct termios *)mp->b_cont->b_rptr;
		bcopy((char * )&lft_ptr->strlft->lft_tios,(char * )tios,sizeof(struct termios));
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof( struct termios);
		break;
	case TIOC_REQUEST:
	{
		register int r_size = sizeof(lft_tioc_reply);
		
		iocp->ioc_cmd = TIOC_REPLY;
		if (mp->b_cont) {
			freeb(mp->b_cont);
			mp->b_cont=NULL;
		}
		mp1 = allocb(r_size, BPRI_MED);
		mp->b_cont=mp1;
		if (!mp1) {
			iocp->ioc_error = EAGAIN;
			lfterr(NULL,"LFTDD", "lft_ctl", "allocb", 0, LFT_STR_NOMSG, UNIQUE_26);
			break;
		}
		mp1->b_datap->db_type = M_DATA;
		mp1->b_wptr = mp1->b_rptr + r_size;
		bcopy(lft_tioc_reply, mp1->b_rptr, r_size);
	}
		break;
	case TIOCGETMODEM:
#if 0
		*(++(mp->b_cont->b_wptr)) = 0;
		break;
#endif
	default:
		freemsg(mp);
		return;
	}
	qreply(q,mp);
	return;
}
/***********************************************************
Function : lftwsrv
Input Parametrs :
	q  : LFT driver write queue
***********************************************************/

void lftwsrv(queue_t *q)
{
mblk_t *mp;
struct vtmstruc *vp;
struct iocblk *iocp;
struct termios *tios;
unsigned char outtype;
unsigned long outcnt;
char * namdev;
int i,nam_ln;
struct phys_displays *pd;

	while (mp=getq(q)) {
		switch(mp->b_datap->db_type) {
		case M_IOCTL:
		{
			/* unexpected transparent ioctls */

			iocp = (struct iocblk *)mp->b_rptr;
			if (iocp->ioc_count == TRANSPARENT) {
				lfterr(NULL,"LFTDD", "lftwsrv", NULL, 0, LFT_STR_INVIOCTL, UNIQUE_27);
				mp->b_datap->db_type = M_IOCNAK; /*Not Okay */
				iocp->ioc_error = EINVAL;
				qreply(q,mp);
				return;
			}
			iocp->ioc_error =0;
			outtype = M_IOCACK;
			outcnt = 0;
		
			switch (iocp->ioc_cmd) {
			case LFT_SET_DFLT_DISP :
				namdev= (char *)mp->b_cont->b_rptr;
				nam_ln= mp->b_cont->b_wptr-mp->b_cont->b_rptr;
				for(i=0;i<lft_ptr->dds_ptr->number_of_displays;i++){
					if(!strncmp(lft_ptr->dds_ptr->displays[i].devname,
					    	namdev, min(sizeof(namdev),nam_ln))){
						if(lft_ptr->dds_ptr->displays[i].fp_valid == TRUE){
							lft_ptr->dds_ptr->default_disp_index = i;

							/* 
							 * if chdisp -d <dev_name> is issued, then
                                                         *  we need to inform Display Power Management code of 
	                                                 *  the new defualt display.
                                                         */ 
							if ((lft_ptr->dds_ptr->pwr_mgr_time[0] != 0) && lft_ptr->dds_ptr->enable_dpms)
							   lft_pwrmgr(RESTART_LFT_DPM_WD);

						}else{
							lfterr(NULL,"LFTDD", "lftwsrv", NULL, 0, LFT_STR_SETINVD, UNIQUE_28);
							iocp->ioc_error = ENODEV;
							outtype = M_IOCNAK;
						}
						break;
					}
				}
				/* we didn't found it */
				if(i==lft_ptr->dds_ptr->number_of_displays){
					lfterr(NULL,"LFTDD", "lftwsrv", NULL, 0, LFT_STR_NODISP, UNIQUE_29);
					iocp->ioc_error = ENODEV;
					outtype = M_IOCNAK;
				}
			break;
			case TIOCSETAW:
			case TIOCSETAF:
				if (iocp->ioc_count != sizeof (struct termios) || !mp->b_cont) {
					lfterr(NULL,"LFTDD", "lftwsrv", NULL, 0, LFT_STR_NOSPC, UNIQUE_30);
					outtype = M_IOCNAK;
					iocp->ioc_error = ENOSPC;
					break;
				}
				/* Wait for the o/p to drain and flush I/p and set params */
				if (iocp->ioc_cmd == TIOCSETAF)
					flushq(RD(q),FLUSHDATA);
				tios= (struct termios *)mp->b_cont->b_rptr;
				bcopy((char * )tios,(char * )&lft_ptr->strlft->lft_tios,sizeof(struct termios));
				break;
			default:
				/* I should never be here */
				break;
			}
			mp->b_datap->db_type = outtype;
			iocp->ioc_count = outcnt;
			mp->b_cont->b_wptr = mp->b_cont->b_rptr + outcnt;
			qreply(q,mp);
		}
			break;
		case M_DATA:
		/* default display must be valid */
			if((!NO_DEFLT_DISP)&&(mp->b_wptr >= mp->b_rptr)){
		/* M_STOP received */
				if(LFT_DOWN.stop_output){
					putbq(q,mp); /* should be only for the 1rst msg after M_STOP */
					return;
				}else{
					vp = lft_ptr->dds_ptr->displays[DEFLT_DISP_IDX].vtm_ptr;
					/* output the message */
					lftout(mp->b_rptr,(mp->b_wptr - mp->b_rptr),vp);
				}
			}
			freemsg(mp);
			break;
		default:
			freemsg(mp);
			break;
		}/* end of switch */
	}/* end of while */
}

/***********************************************************
Function : lftrsrv
Input Parametrs :
	q  : LFT driver write queue
***********************************************************/

void lftrsrv(queue_t *q)
{
mblk_t *mp;
int old_intr;
	while (mp = getq(q)) {

		switch (mp->b_datap->db_type) {
		case M_DATA:
		default:
			if((QPCTL <= mp->b_datap->db_type)|| canput(q->q_next)){
				putnext(q,mp);
			}else {
				putbq(q,mp);
				old_intr = i_disable(INT_LFT);
				LFT_UP.input_flag|=UPQ_OVFL;
				i_enable(old_intr);
				return;
			}
			break;
		}
	}
	old_intr = i_disable(INT_LFT);
	if(LFT_UP.input_flag&UPQ_OVFL){
		LFT_UP.input_flag&=~UPQ_OVFL;
		if(REMAINING_EVTS)
		/* schedule lftKiOffl by calling i_sched */
			i_sched(&LFT_UP.offl_hdl);
	}
	i_enable(old_intr);
return;
}

