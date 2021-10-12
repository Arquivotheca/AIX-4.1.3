static char sccsid[] = "@(#)33	1.3.1.13  src/bos/kernext/lft/fonts/fkproc.c, sysxdisp, bos41J, 9521A_all 5/23/95 11:04:43";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: create_fkproc
 *              fkproc
 *              fkproc_attach_shm
 *              fkproc_detach_shm
 *              fkproc_dev_dep_fun
 *              get_font_addr_len
 *              kill_fkproc
 *              pin_font
 *              pin_part_font
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define Bool  unsigned               /* for aixfont.h */

#ifdef   _KERNSYS
#include <sys/processor.h>
#else
#define  _KERNSYS  1
#include <sys/processor.h>
#undef   _KERNSYS
#endif

#include <lft.h>
#include <sys/aixfont.h>
#include <sys/sleep.h>
#include <sys/syspest.h>                /* debugging printf macros & flags */
#include <sys/ipc.h>                    /* shared memory */
#include <sys/shm.h>
#include <sys/seg.h>                    /* segment size */
#include <sys/dma.h>                    /* dma  */
#include <graphics/gs_trace.h>
#include <lft_debug.h>



/* external functions without .h files */
extern pid_t    creatp();
extern lft_ptr_t	lft_ptr;

BUGVDEF(db_fkproc,0);


/*------------------------------------------------------------------------- 
   Create_fkproc:

	It is called at the time we do lft configuration (lftinit.c) to 
        create the font kernel process, fkproc, which is needed to 
        process font faults on the mid-level graphics adapter

        Font faults occur because the adapter has limited address space
        for font data.  Thus, often times the adapter will not have the
        font data stored locally when a X-client or lft issues command to 
        use a particular font for some text data.  When this occurs, the 
        adapter raises an interrupt to the CPU called the font request.
	The font request is routed through the 1st interrupt handler to
        the 2nd level interrupt handler (SLIH - see midintr.c) which is 
        the adapter specific interrupt handler.  It would be nice if SLIH 
        could handle all the processing necessary to send the font data
        to the adapter.  However, it cannot for the reason explained in 
        the next paragraph.  Therefore, for a font request SLIH merely
        schedules (wakes up) a separate special process.  It is this
        process which actually prepares the font for the adapter.  For
        clarity, let's call this process, the font kernel process or fkproc 

	A quick recap tells us that we are trying to get the font data
	to the adapter.  This is accomplished by putting the font data
        in known spot and notify the adapter when it is there.  The
	adapter is responsible for transferring (via DMA) the font data
        as it is required.  As it turns out, the "known spot" will
        be a shared memory segment being created by X server When the first 
        graphics process (happens to be X server) calls gsc_make_gp we will 
        attach this segment to the font kernel process so that it has access 
        (reading) to this segment.  This is how the shared memroy segment will 
        be used by X-server and SLIH in order to make fonts accessible for both

	Again all the fkproc needs to do is to get the font data into a 
        known spot.  First, the font identifer (provided by adapter)
        must be converted into a font address and length -- this provides 
        the location of the font.  Then we must ensure that it stays 
        there, i.e, we must ensure that the data is not swapped out.  
        This is done with a pin.  Since, the pin generates an access to 
        each page comprising the font, the pin can generate a page fault.  
        This page fault is the reason that the SLIH could not handle this 
        task:  the SLIH runs under the interrupt handler and the interrupt 
        handler is not allowed to generate a page fault (or any other 
        interrupt)

	With this background in mind, this function, create_fkproc is
        responsible for creating the font kernel process, fkproc.  

	Once fkproc is running, all work requests such as creating shared
        memory, destroying shared memory, pin a font, and terminating 
        fkproc will be handled by fkproc itself.  All requests for 
        fkproc should be communicated via the queue with fsp_enq function


   Pseudo-code:

        call creatp() to create a new process entry in process table
        and save the returned pid of the child process in the lft struct
       
        if (pid == -1) 
        {
           echo error message
           return (-1)
        } 

        initialize the queue, its head and tail, and flag

        get pid of the process who called create_fkproc() and
        save the pid in the lft struct

        call initp to exec the font kernel function, fkproc.  we pass it the 
        pointer to the the lft struct

        if (return code of initp != 0) 
        {
           echo error message
           return (-1)
        } 

        check flag to see if fkproc is running 
        if (not running)
        {
            while not receive signal from fkproc then wait
        }

   Invoke: creatp, initp, getpid, e_wait

   Called by:  lftconfig (CFG_INIT) 

---------------------------------------------------------------------------*/
create_fkproc()
{
    int         rc, old_pri;
    int         fkproc();               
    char *      process_name = "fkpr";

    GS_ENTER_TRC0(HKWD_GS_LFT,fkproc,1,create_fkproc);

    BUGLPR(db_fkproc, BUGNFO, 
	("-> entering create_fkproc, lft_ptr=%x\n",lft_ptr));

    /* create fkproc process */
    lft_ptr->lft_fkp.fsq.pid = creatp();
    if (lft_ptr->lft_fkp.fsq.pid == (pid_t)(-1) ) 
    {
	/* log error and return error code */

	lfterr(NULL, "LFTDD", "fkproc", "creatp", -1, LFT_CREATE_FKPROC, UNIQUE_1);
	BUGPR(("ERROR: create_fkproc can't create font kernel process\n"));
        GS_EXIT_TRC1(HKWD_GS_LFT,fkproc,1,create_fkproc,-1);
	return(-1); 
    }
	
    /* initialize the queue structure */
    lft_ptr->lft_fkp.fsq.flags = 0;    /* font Kproc is not created yet */

    lft_ptr->lft_fkp.fsq.Q.head = 0;   /* queue is empty */
    lft_ptr->lft_fkp.fsq.Q.tail = 0;

    /* get the process id of the creator of fkproc */
    lft_ptr->lft_fkp.fsq.i_pid = getpid();

    /* initialize the fkproc process.  Note the 3rd argument we have */
    /* to pass a pointer to a pointer to lft struct  because of the  */
    /* way initp works.  Giving a pointer to a block of data and its */
    /* length, it copies the data on the stack of the new process. In*/
    /* this case we only want to pass the pointer to the lft struct  */
    /* data so we pass only the address of where the pointer is      */


    BUGLPR(db_fkproc, BUGNFO, 
	("create_fkproc: before initp, ppid=%d, pid=%d\n",
	lft_ptr->lft_fkp.fsq.i_pid, lft_ptr->lft_fkp.fsq.pid));

    rc = initp (lft_ptr->lft_fkp.fsq.pid, fkproc, &lft_ptr, 
		sizeof(lft_ptr), process_name );


    /* Was the process initialized */
    if (rc != 0) {
	/* Log error and return error code */
	lfterr(NULL, "LFTDD", "fkproc", "initp", -1, LFT_FKPROC_INITP, UNIQUE_2);
	BUGPR(("###### ERROR: create_fkproc can't initialize font process \n"));
        GS_EXIT_TRC1(HKWD_GS_LFT,fkproc,1,create_fkproc,rc);
	return (rc);
    }

    /*
     *  Wait until fkproc is ready to process queue entries.
     */
    e_wait (FKPROC_WAIT_INIT,FKPROC_WAIT_INIT,0);

    BUGLPR(db_fkproc,BUGNFO,
	("===== exit create_fkproc, pid=%d\n",lft_ptr->lft_fkp.fsq.pid));

    GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,create_fkproc);

    return (0);
}


/*---------------------------------------------------------------------------
   Kill_fkproc:

	enqueue a command to fkproc to tell the font process, fkproc 
        to exit itself

   Invoke: fsp_enq

   Called by: lftconfig (CFG_TERM) 
---------------------------------------------------------------------------*/
kill_fkproc()
{
    int             flag, rc;
    fkprocQE        qe;               /* queue a command element to kproc */

    GS_ENTER_TRC0(HKWD_GS_LFT,fkproc,1,kill_fkproc);

    BUGLPR(db_fkproc, BUGNFO, 
	("-> entering kill_fkproc, lft_ptr=%x\n",lft_ptr));

    /* send a terminate command to the fkproc, and wait for completion */

    qe.command = FKPROC_COM_TERM | FKPROC_COM_WAIT;

    if ((*lft_ptr->lft_fkp.fsp_enq) (&qe,lft_ptr) != FKPROC_Q_SUCC) 
    {
	    BUGPR(("###### kill_fkproc ERROR bad nq \n"));
    }

    GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,kill_fkproc);

    BUGLPR(db_fkproc, BUGNFO, ("===== exiting kill_fkproc\n"));
}


/*---------------------------------------------------------------------------
   fkproc:

	the font kernel process which is responsible for attaching the 
        shared memory segment, detaching it, and pinning fonts for 
        font faults on mid-size graphics adapter

   Pseudo-code:
       
        isolate us from current group's signals

        set flag to indicate to parent process that it starts running
 
        post a signal to tell parent process not to wait for it any more

        while (flag == true)
        {
           case pin font:
               call pin_font() to do whatever it takes to pin a font

           case attach shared memory :
               call fkproc_attach_shm() to attach shared memory to fkproc 

           case detach shared memory :
               call fkproc_detach_shm() to detach shared memory from fkproc 

           case terminate process:
               flag = false;

           default:

               echo error message
        }


   Invoke: setpgid, e_post, fsp_deq, pin_font, 

   Called by: None.  With the queuing mechanism (fsp_enque,fsp_deq) fkproc
                     is put to sleep (by fsp_deq) when queue is empty and 
                     awaken (by fsp_enq) when there is something in queue.
---------------------------------------------------------------------------*/
fkproc(flg,lft_pptr,len)
int 	flg;                         /* we just ignore.  See initp and */ 
lft_t	**lft_pptr;
int 	len;                         /* we just ignore this too        */
{
    int             flag, rc;
    fkprocQE        qe;
    lft_t 	    *lftptr;

    switch_cpu (MP_MASTER, SET_PROCESSOR_ID);

    lftptr = *lft_pptr;

    GS_ENTER_TRC0(HKWD_GS_LFT,fkproc,1,fkproc);

    BUGLPR(db_fkproc, BUGNFO, 
	("-> entering fkproc, lft_ptr ptr=%x\n",lftptr));

    /* isolate us from the current group's signals */
    setpgid (0,0);

    /* make the 'init' process our parent */
    setpinit ();

    /* purge any signals received to this point */
    purge_sigs ();

    /* indicate that kproc is initialized.  Note the */
    /* queue is in the lft sturct		     */

    lftptr->lft_fkp.fsq.flags |= FKPROC_INIT;

    BUGLPR(db_fkproc, BUGNFO, 
	("fkproc: set flag & e_post pproc ppid=%d\n",lftptr->lft_fkp.fsq.i_pid));

    e_post (FKPROC_WAIT_INIT, lftptr->lft_fkp.fsq.i_pid);

    /***********************************************/
    /* go into infinite loop, wait for items to be */
    /* queued to the fkproc.                       */
    /***********************************************/

    flag = TRUE;
    while (flag) 
    {
	/* get a command */
        BUGLPR(db_fkproc, BUGNFO, 
		("fkproc: in while & call deq, addr=%x\n",lftptr->lft_fkp.fsp_deq));

	rc = (*lftptr->lft_fkp.fsp_deq) (&qe,lftptr);


	if (rc) {
	    lfterr(NULL, "LFTDD", "fkproc", "fsp_deq", -1, 
				LFT_FKPROC_DEQ, UNIQUE_3);
	    BUGPR(("###### kproc ERROR cannot get qe, rc=%d \n", rc));
            GS_EXIT_TRC1(HKWD_GS_LFT,fkproc,1,fkproc, rc);
	    return;
	}
	
	BUGLPR(db_fkproc,BUGACT,("fkproc:fsp_deq command=%d\n",qe.command));

	/* decode the command */
	switch (qe.command & FKPROC_COM_CMD_MASK)
        {

	   case FKPROC_COM_PIN: /* pin a font  */
	      BUGLPR(db_fkproc,BUGNFO,("fkproc: command is PIN FONT\n"));
	      rc = pin_font(&qe);
	      break;



	   case FKPROC_COM_UNPIN: /* unpin a X font */

	      BUGLPR(db_fkproc,BUGNFO,("fkproc: command is UNPIN\n"));

	      rc = unpin(qe.font_addr,qe.font_len);
              if (rc)
              {
	         BUGLPR(db_fkproc,0,("fkproc: unpin failed addr=0x%x,len=%d\n",
                            qe.font_addr,qe.font_len));
                 rc = -1; 
              }

	      break;



	   case FKPROC_COM_ATTACH: /* attach shared mem. seg. */
	      BUGLPR(db_fkproc,BUGNFO,("fkproc: command is ATTACH SHM\n"));
	      rc = fkproc_attach_shm(lftptr);  
	      break;



	   case FKPROC_COM_DETACH: /* detach shared mem. seg. */
	      BUGLPR(db_fkproc,BUGNFO,("fkproc: command is DETACH SHM\n"));
	      rc = fkproc_detach_shm(lftptr);
	      break;



	   case FKPROC_COM_TERM: /* exit the Kproc */

	      BUGLPR(db_fkproc,BUGNFO,("fkproc: cmd is TERMINATE fkproc\n"));

	      flag = FALSE;
	      break;

          case FKPROC_COM_PIN_PART: /* pin part of a font  */
              BUGLPR(db_fkproc,BUGNFO,("fkproc: command is PIN PART FONT\n"));
              rc = pin_part_font(&qe);
              break;

           case FKPROC_COM_DDF:
              BUGLPR(db_fkproc,BUGNFO,("fkproc: command is DEV DEP FUN\n"));
              rc = fkproc_dev_dep_fun(&qe);
              break;

	   default:
	      lfterr(NULL, "LFTDD", "fkproc" ,NULL, 0, 
				LFT_FKPROC_BADCMD, UNIQUE_4);
	      BUGPR(("###### fkproc ERROR bad qe \n"));
	}

	/* wakeup waiter, if called for */
	if (qe.command & FKPROC_COM_WAIT)
	{
	    BUGLPR (db_fkproc, BUGNFO, ("wakeup: qe.pfkproc_com_wait 0x%x\n",
							qe.pfkproc_com_wait));
	    qe.pfkproc_com_wait->done = 1;
	    e_wakeup (&qe.pfkproc_com_wait->sleep_done);
	}

    } /* end while */

    GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,fkproc);

    BUGLPR(db_fkproc, BUGNFO, ("===== exit fkproc\n"));
}


/*---------------------------------------------------------------------------
    Pin_font:

	Pin a font in shared memory  

    Pseudo-code:  

            compute font's address and length

            Pin font (pin is called if KSR font; pinu otherwise ) 

            save font address and length.  Also get a pointer to
            the data structure needed to setup DMA for this font

	    set up cross memory pointer with xmattach (different segflag
            is used depending on the font type, KSR or X.  For X font
            USER_ADSPACE is used; for KSR font, SYS_ADSPACE is used)

            d_master to set up TCW (this is done in non-hidden only mode) 

	    call pinned_font_ready, device dependent function to ask
	    the hardware to start DMA (actually it does several things)

   Invoke: 

   Called by: fkproc

---------------------------------------------------------------------------*/
pin_font (qe)
fkprocQE *qe;                /* pointer to queue element */
{

    fkproc_font_pin_req_t  *req_data ;
    struct phys_displays  *pd ;

    font_addr_n_len font_addr_len;	/* to hold font's address and length */

    int old_pri, segflag, status, rc =0; 

    struct xmem * xm_dsp_ptr ;


    GS_ENTER_TRC0(HKWD_GS_LFT,fkproc,1,pin_font);
    req_data =  (fkproc_font_pin_req_t *)(qe-> request_data) ;


    BUGLPR(db_fkproc, 2, ("-> entering pin_font fid=%x\n",req_data->font_ID ));


    pd = req_data -> pd ; 

    BUGLPR(db_fkproc, 0, ("req_data = 0x%8X, pd = 0x%8X \n",req_data, pd ));
    BUGLPR(db_fkproc, 0, ("passed chan  = 0x%8X \n", req_data-> DMA_channel ));
    BUGLPR(db_fkproc, 0, ("passed bus addr = 0x%8X \n", req_data-> bus_addr ));

    status = 0;

    rc = get_font_addr_len (pd, req_data->font_ID, &font_addr_len);

    if (rc == -1)
    {
       lfterr(NULL, "LFTDD", "fkproc", "get_font_addr_len", rc, 
					LFT_FKPROC, UNIQUE_5);
       BUGLPR(db_fkproc, BUGNFO, ("pin_font: get_font_addr_len failed\n"));

       /* WHAT ELSE TO DO HERE ?? */ 
       status |= 1;
       goto error;
    }

    BUGLPR(db_fkproc, BUGNFO, 
	("pin_font: addr=%x, len=%d\n",font_addr_len.addr,font_addr_len.len));

    /* -------------------------

       Note: font data, KSR or X, is in kernel space         
             For KSR, the fonts are in the device driver palette.  
             For X, the fonts are in the shared memory created  by  
             X server, an user process.  Since we attached this    
             shared memory to the font kernel process, this shared 
             memory is mapped into the kernel space of the font    
             process.  Thus, from the point of view of the fkproc, 
             we should access the font data in the kernel space.

     --------------------------- */

    rc = pin(font_addr_len.addr,font_addr_len.len);

    if (rc)
    {
       
       lfterr(NULL, "LFTDD", "fkproc", "pin", rc, LFT_PIN_FAIL, UNIQUE_6);
       BUGLPR(db_fkproc, 0 , ("pin_font: pin kernel call failed rc =%d\n",rc));
       status |= 2;
       goto error;
    }

    /* ----------------- 
     set up the cross memory pointer to it.  
    ----------------- */

    /* segflag should be SYS_ADSPACE.  See explanation above */
    req_data->xm.aspace_id = XMEM_INVAL; 

    BUGLPR(db_fkproc, BUGNFO, ("pin_font: call xmattach\n"));

    rc=xmattach(font_addr_len.addr,font_addr_len.len, &(req_data->xm), SYS_ADSPACE);

    if (rc != XMEM_SUCC)
    {
       lfterr(NULL, "LFTDD", "fkproc", "xmattach", rc, LFT_XMATTACH, UNIQUE_7);
       BUGLPR(db_fkproc,0, ("pin_font: xmattach failed code=%d\n",rc));
       status |= 4;
       goto error;
    }

    /* ---------------------------------------------------------------------
	SET UP the TCWs (via d-master)

	First, however, we will save all the parameters in the passed
	request block, to ensure the d_complete it done identically.

	Also, the bus addr is aligned with the system (font) address.

       Note that font DMA is always done in non-hidden mode.
       d_master(channel_id, flags, dma_data_addr, data_length,
                xm_descriptor, dma_cmd_addr)                   
    ------------------------------------------------------------------------ */

    BUGLPR(db_fkproc, BUGNFO, ("pin_font: call d_master\n"));


    req_data-> flags = DMA_WRITE_ONLY ;

    /* ------------------- 
       Save font 's length and virtual address so that later
       we don't have to calculate again when the same font is
       requested to be unpinned.  
    ------------------- */

    req_data-> sys_addr = font_addr_len.addr ;
    req_data-> length = font_addr_len.len ;

    req_data-> bus_addr = (char *)((((int)(req_data-> bus_addr)) & 0xFFFFF000) +
    			            ((int)(req_data-> sys_addr)  & 0x00000FFF));

    BUGLPR(db_fkproc,0, ("corrected bus addr 0x%8X \n", req_data-> bus_addr ));


    d_master (
		req_data-> DMA_channel, 
    		req_data-> flags,
    		req_data-> sys_addr,
    		req_data-> length,
    		&(req_data-> xm),
    		req_data-> bus_addr );



    /* -----------------
       call device dependent function to cause DMA transferring (done by
       adapter).   
    -------------------- */

   BUGLPR(db_fkproc, BUGNFO, ("pin_font: call pinned_font_ready\n"));

   (* pd->pinned_font_ready)(qe,&font_addr_len) ;  /* 2nd arg. is not used */


   BUGLPR(db_fkproc, BUGNFO, ("===== exiting pin_font\n"));

   GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,pin_font);

   return(0);

error:


   BUGLPR(db_fkproc, BUGNFO, ("===== exiting pin_font\n"));
   GS_EXIT_TRC1(HKWD_GS_LFT,fkproc,1,pin_font, -1);
   return (-1);
}

/* --------------------------------------------------------------*/
/* Get_font_addr_len:                                            */
/*								 */
/*	From font id, find its memory location (address) and     */
/*      length.                                                  */
/*								 */
/* Return: pointer to struct fnt_addr_len which has addr & len   */
/*								 */
/* Called by: pin_font                                           */
/*								 */
/* --------------------------------------------------------------*/
get_font_addr_len(pd,font_id,addr_len)
struct phys_displays * pd;

ulong font_id;                   /* last 28 bits (out of 32) will be */
                                 /* address where the font is        */ 
font_addr_n_len * addr_len; 

{

        unsigned int header_sz, char_matrix_sz, glyph_sz, segid;

	ulong font_addr;

	lft_t	*lftptr;

	struct shmid_ds statbuf;               /* used for debugging */

        GS_ENTER_TRC0(HKWD_GS_LFT,fkproc,1,get_font_addr);

        BUGLPR(db_fkproc, BUGNFO, 
		("-> entering get_font_addr_len, fid=%x\n",font_id));

        /* ------------------ 

          for KSR, the unmodified virtual address of where each font is 
          will be of this format 0x0XXXXXXX where XXXXXXX are 7 hex number. 
          That is the 4 MS or segment id is always zero.  This is true
          because the kernel data uses segment zero ??.  We also have
          a second kind of font id, X fonts.  Therefore, to distinguish 
          KSR font id's  from the X font id's, whoever called 
          MID_SetActiveFont from the KSR mode set the 4 most MS bits of 
          the virtual address of the KSR font id to 1's.  

          On the other hand, in the case of X font id's the 4 MS bits are 
          masked off.  

          When the font fault occured, the microcode passed us back the
          modified address.  To get the real address, all we to
          do the following:

          a.  for KSR font id, we have to mask off the 4 MS bits or 
              equivalently just extract the lower 28 bits from the font id.  
              This number will be the address of where the font is in virtual
              memory.  we turn this number into a pointer by typecasting it. 
              This pointer can be use to calculate size of font.  The size is
              figured out below.                                          

          b.  for X font id, what we have is the offset into the shared
              memory segment.  We already attached to this shared memory
              segment and saved the its address in the lft struct
              way back.  What we have to do is to construct a 32 bit
              virtual address of where the font is in shared memory.
              We already got the 28 bits offset from the font id, so
              the only piece missing is the 4 MS bits, i.e the segment
              id of the virtual address.  Since we have the address of 
              the beginning of the shared memory segment, we can extract
              the segment id, 4 MS bits easily.  Once we get the segment
              id we just have to OR it with the font id to produce the  
              32 bit virtual address of the the font is in shared
              memory.

        -------------------- */

	if (isKSRfont(font_id))
        {
           /* extract the lower 28 bits of the font id */

	   font_addr = (font_id & SEG_OFFSET_MASK);

           BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: KSR mode, font addr=%d\n",font_addr));


    	}  
	else   /* X (non KSR) font */
        {

	   lftptr = (lft_ptr_t) pd->lftanchor;

           BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: call shmctl for status\n"));
	   if (shmctl(lftptr->lft_fkp.segID,IPC_STAT,&statbuf) != -1)
           {
              BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: status-> segsize %x\n",statbuf.shm_segsz));
              BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: status-> pid of last shmop %d\n",
		statbuf.shm_lpid));
              BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: status-> pid of creator %d\n",
		statbuf.shm_cpid));
              BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: status-> # of attached %d\n",
		statbuf.shm_nattch));
              BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: status-> in memory # of attach %d\n",
		statbuf.shm_cnattch));
	   }
	   else
           {
              BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: shmctl for status failed\n"));
           }

           /* Make sure shared memory was created before accessing it */
	   if (lftptr->lft_fkp.setup_shm == 0)
	   {
	      lfterr(NULL, "LFTDD", "fkproc", NULL, 0, LFT_FKPROC_SHM, UNIQUE_8);
              BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: shm was not setup\n"));
    	      GS_EXIT_TRC1(HKWD_GS_LFT,fkproc,1,get_font_addr, -1);
              return(-1);
	   }

           /* 
               note: we have to change segment number of the pointer      
               Each virtual address has 2 components, segment number 
               which is 4 most significant bits, and 28 bits offset 
               The proper segment is selected with the segment index
               into an array of 16 segments available to a process.
               Even thought the font kernel process, fkproc, and X 
               server attach to the same shared memory segment, it is 
               likely that different segment register are used for each.
               Therefore, we must alter all addresss in the shared 
               memory segment to used the kernels segment number.
           */ 

	   segid = (ulong)(lftptr->lft_fkp.addr_shm_seg) & SEGID_MASK;

           BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: non KSR mode, shm@=%x, segid=%x\n",
		lftptr->lft_fkp.addr_shm_seg,segid));

           /* form the 32 bit virtual address of where the font is */
	   font_addr = segid | font_id;

           BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: Fontheader0 =%x\n",*((long*)font_addr)));
           BUGLPR(db_fkproc, BUGNFO, 
		("get_font_addr_len: Fontheader1 =%x\n",*((long*)font_addr+1)));

	}

        addr_len->addr = (char *)font_addr;

        header_sz = sizeof(aixFontInfo);

        /* ------------------ 
        total size of char. metric area = (number of rows)   *
                                          (number of colums) *
                                          (size of each metric)  
        -------------------- */

        char_matrix_sz = BYTESOFCHARINFO( ((aixFontInfo*)font_addr) );
 
        glyph_sz = BYTESOFGLYPHINFO( ((aixFontInfo *) font_addr) );

       /* ------------------ 
         font size =    size of font header                       
                      + size of character metric area            
                      + size of character glyph area           
       -------------------- */

BUGLPR(db_fkproc, BUGNFO, 
("get_font_addr_len: header size=%d, char matrix size =%d, glyph size = %d\n",
	header_sz,char_matrix_sz,glyph_sz));

        addr_len->len = header_sz + char_matrix_sz + glyph_sz;

        BUGLPR(db_fkproc, BUGNFO, ("-> exiting get_font_addr_len\n"));
        GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,get_font_addr);
	return(0);
}

/*---------------------------------------------------------------------------
    fkproc_attach_shm:

	Attach the shared memory segment created by X-server to the 
	font kernel process so that it can read the font data and pin it
        before the adapter can read (DMA) it.

   Invoke: kernel_ftok, shmget, shmat

   Called by: fkproc   (X server is the very first graphic process that calls
                       gsc_make_gp   This is when the shared memory segment 
                       is attached to fkproc)

---------------------------------------------------------------------------*/
fkproc_attach_shm (lftptr)
lft_t	*lftptr;
{
	int segid, old_pri,rc = 0;
	struct shmid_ds buffer;
	char * addr_shm_seg;

	key_t key;        /* used to attach the shared memory segment */

	key_t kernel_ftok();


        GS_ENTER_TRC0(HKWD_GS_LFT,fkproc,1,fkproc_attach_shm);
        BUGLPR(db_fkproc, BUGNFO, ("===== entering fkproc_attach_shm\n"));

        BUGLPR(db_fkproc, BUGNFO, 
		("fkproc_attach_shm: comm ptr=%x, setup=%d\n",
		lftptr, lftptr->lft_fkp.setup_shm));

 	if (lftptr->lft_fkp.setup_shm)
        {
	   /* only need to do shared memory attached once - already done*/ 
	   lfterr(NULL, "LFTDD", "fkproc", NULL, 0, LFT_FKPROC_SHM, UNIQUE_9);
           BUGLPR(db_fkproc, BUGNFO, 
	      ("fkproc_attach_shm: ERROR: try to attach shm more than once\n"));
           GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,fkproc_attach_shm);
	   return;
	}

	/* FTOK() IS IN LIBC.A SO KERNEL CODE CAN'T LINK TO IT.              */
        /* WE BORROW THIS CODE AND CHANGE IT SO THAT KERNEL CODE CAN CALL IT.*/
        /* IF FTOK IS CHANED IN FUTURE, WE NEED TO MAKE SURE kernel_ftok     */
        /* in kernel_ftok.c HAS TO BE CHANGED ACCORDINGLY.                   */

        /* note: KEY_PATH and KEY_ID have to be the same as those used by    */
        /*       X server                                                    */

        BUGLPR(db_fkproc, BUGNFO, ("calling kernel_ftok\n"));

	key = kernel_ftok(KEY_PATH,KEY_ID);  

        BUGLPR(db_fkproc, BUGNFO, 
		("returned from kernel_ftok computed key =%x\n",key));

	if (key == -1)
	{
    	   BUGLPR(db_fkproc, 0, ("ERROR: kernel_ftok failed to create key \n"));
           GS_EXIT_TRC1(HKWD_GS_LFT,fkproc,1,fkproc_attach_shm, -1);
	   return(-1);
	}

	/* 
            We could have make X server to pass us the segment id of the
            shared memory (returne value of shmget()) but that would require 
            a new interface.  Thus to find out what the segment id is, 
            we can call shmget directly with the same key, and segment size
            to get the same segment id used by X server.

            Note that it is likely that for the kernel a different segment
            register will be used for this segment.  This is why we need
            to save the the return value of shmat()
        */

        BUGLPR(db_fkproc, BUGNFO, ("fkproc_attach_shm: call shmget\n"));
	segid= shmget(key,SEGSIZE, ( S_IRUSR | S_IWUSR |
                                     S_IRGRP | S_IWGRP |
                                     S_IROTH | S_IWOTH)  );

	if (segid == -1)
	{
    	   BUGLPR(db_fkproc,0,("shmget failed to get segid- errno=%d\n",errno));
    	   GS_EXIT_TRC1(HKWD_GS_LFT,fkproc,1,fkproc_attach_shm, -1);
	   return(-1);
	}

	/* attach shared memrory to current process (creator) for READ ONLY*/ 
        BUGLPR(db_fkproc, BUGNFO, ("fkproc_attach_shm: call shmat\n"));
	addr_shm_seg = shmat(segid,0,SHM_RDONLY);


	if (addr_shm_seg == -1)
	{
    	   BUGLPR(db_fkproc,0,("shmat failed to attach shm- errno=%d\n",errno));
    	   GS_EXIT_TRC1(HKWD_GS_LFT,fkproc,1,fkproc_attach_shm, -1);
	   return(-1);
	}

        BUGLPR(db_fkproc, BUGNFO, 
		("fkproc_attach_shm: key=%x, segid=%d shm addr%x\n",
		key,segid,addr_shm_seg));

	/* save segment ID so that later we can detach the shared memory */
	lftptr->lft_fkp.segID = segid;

	/* save beginning address of shared memory segment so that we can  */
        /* locate X fonts to be pinned                                     */

	lftptr->lft_fkp.addr_shm_seg = addr_shm_seg;

        /* shared memory has been set up.  Set flag so we would not    */
        /* access information in shared memory before we have attached */
        /* to out font kernel process.                                 */

        lftptr->lft_fkp.setup_shm = 1;

        BUGLPR(db_fkproc, BUGNFO, ("===== exiting fkproc_attach_shm\n"));
    	GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,fkproc_attach_shm);
	return(0);      /* everything is OK */
}


/*---------------------------------------------------------------------------
    fkproc_detach_shm:                                  

 	Detach the shared memory segment from the font kernel process 

   Invoke: shmdt, shmctl 

   Called by: fkproc 

---------------------------------------------------------------------------*/
fkproc_detach_shm (lftptr)
lft_t	*lftptr;
{
	int rc;

        GS_ENTER_TRC0(HKWD_GS_LFT,fkproc,1,fkproc_detach_shm);
        BUGLPR(db_fkproc, BUGNFO, 
		("->entering fkproc_detach_shm, common=%x, setup=%d\n",
		lftptr,lftptr->lft_fkp.setup_shm));

        if (! lftptr->lft_fkp.setup_shm)
	{

/* remove error logging as this happens as normal part of design for many   */
/* adpaters and is not an error                                             */
/*  lfterr(NULL, "LFTDD", "fkproc", NULL, 0, LFT_FKPROC_SHM, UNIQUE_10);    */

    	   BUGLPR(db_fkproc, 0 , ("shmdt: no shard memory to detached\n")); 
           GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,fkproc_detach_shm);
           return(-1);
	}

	rc = shmdt(lftptr->lft_fkp.addr_shm_seg);	
	if (rc == -1)
	{
	   lfterr(NULL, "LFTDD", "fkproc", "shmdt", rc, LFT_FKPROC_SHM, UNIQUE_11);
    	   BUGLPR(db_fkproc, 0 , ("shmdt failed to detach segment\n")); 
           GS_EXIT_TRC1(HKWD_GS_LFT,fkproc,1,fkproc_detach_shm, -1);
	   return(-1);
	}

	lftptr->lft_fkp.setup_shm = 0;

        BUGLPR(db_fkproc, BUGNFO, ("===== exiting fkproc_exiting_shm\n"));
        GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,fkproc_detach_shm);
	return(0);  /* everything is OK */
}


/*---------------------------------------------------------------------------
    Pin_part_font:

	Pin part of a font - this function merely makes sure that the
	data is accessible to the system, and then calls a device
	dependent function to set up the actual data, map and pin it,
	and then inform the adapter.
	   We considered the design of having a device dependent
	function set up the data, then call a fkproc function to do the
	mapping and pinning (since that is device independent), which
	would in turn call another device dependent function to inform
	the adapter.  This seemed a complex design, with no obvious
	benefits.

   Invoke: 

   Called by: fkproc

---------------------------------------------------------------------------*/
pin_part_font (qe)
fkprocQE *qe;			/* pointer to queue element */
{
  fkproc_font_pin_part_req_t	*req_data ;
  int				rc = 0;

  GS_ENTER_TRC0(HKWD_GS_LFT,fkproc,1,pin_part_font);
  /*
   * Initialize local pointer to request data structure
   */
  req_data = (fkproc_font_pin_part_req_t *)qe->request_data;

  BUGLPR(db_fkproc, BUGNFO, ("-> entering pin_part_font req_data=%x\n",
			     req_data ));

  /*
   * Check to see if data was DMA'ed into font request buffer - if so,
   * we need to do a d_complete to allow the system side to access it
   * properly.  Report any error on dma to device dependent function.
   */
  if (req_data->font_buf_flags & FONT_BUF_IS_DMA)
  { rc = d_complete(req_data->DMA_channel, req_data->font_buf_dma_flags,
		    req_data->font_req_buf, req_data->font_buf_len,
		    &req_data->font_buf_xmd, req_data->font_buf_bus_addr);
    if (rc == DMA_SUCC)
      rc = 0;
  }
      

  /*
   * Call device dependent function to prepare glyph data to be
   * transferred to adapter.  It will then call pin_data_blocks.
   */
  (req_data->prepare_font_data)(req_data->pd, req_data->font_req_buf, rc);

  /* 
   * This will cause the CPU cache lines for this memory to be
   * invalidated so that when the Host Font Manager reads the next
   * request, it reads it from real memory.
   */
  if (req_data->font_buf_flags & FONT_BUF_IS_DMA && !(__power_pc()))
   { d_cflush(req_data->DMA_channel, req_data->font_req_buf,
	      req_data->font_buf_len, req_data->font_buf_bus_addr);
   }

  /*
   * Some situations may require synchronization with this request.  In
   * these cases, the caller sets its pid and the event it wants to wait
   * on in the request, and we will do a post here just before we exit.
   * Obviously, this cannot be done for requests due to an interrupt.
   */
  if (req_data->pid != (pid_t)0)
{
delay(10);
    e_post(req_data->event, req_data->pid);
}

  BUGLPR(db_fkproc, BUGNFO, ("===== exiting pin_part_font\n"));

  GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,pin_part_font);
  return(0);
}


/*---------------------------------------------------------------------------
    Fkproc_Device_Dependent_Function

	Device dependent function - this function is used when all
	required processing is device dependent; the font kernel process
	is called only to off-load the work from the interrupt handler.

   Invoke: 

   Called by: fkproc

---------------------------------------------------------------------------*/
fkproc_dev_dep_fun (qe)
fkprocQE *qe;                /* pointer to queue element */
{
  fkproc_dev_dep_fun_req_t  *req_data ;

  GS_ENTER_TRC0(HKWD_GS_LFT,fkproc,1,fkproc_dev_dep_fun);
  /*
   * Initialize local pointer to request data structure
   */
  req_data = (fkproc_dev_dep_fun_req_t *)qe->request_data;

  BUGLPR(db_fkproc, BUGNFO, ("-> entering fkproc_dev_dep_fun req_data=%x\n",
			     req_data ));

  (req_data->dev_dep_function)(req_data);

  BUGLPR(db_fkproc, BUGNFO, ("===== exiting fkproc_dev_dep_fun\n"));

  GS_EXIT_TRC0(HKWD_GS_LFT,fkproc,1,fkproc_dev_dep_fun);
  return(0);
}


