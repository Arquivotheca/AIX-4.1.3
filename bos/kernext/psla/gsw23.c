static char sccsid[] = "@(#)03  1.10.1.7  src/bos/kernext/psla/gsw23.c, sysxpsla, bos41J, 9515A_all 3/28/95 14:14:06";
#ifndef KERNEL
#define KERNEL
#endif
/*
 *   COMPONENT_NAME: SYSXPSLA
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* PURPOSE:     Device driver for MSLA on R2 internal routines called   */
/*              from the main entry point routines                      */
/*                                                                      */
/*;bb 020190    Put defines for errlog in tpath local sys file for      */
/*;             local compile so no change needed for files sent to     */
/*;             austin.                                                 */
/*;bb 030690    Add init_free(), remove init_vars().                    */
/*;MJ 031290    Modified parameter types of function calls: gfree_spac  */
/*;		acc_regs usrunpin gerr_log IOCC_ATT and IOCC_DET.	*/
/*;MJ 031290    Removed parameter size from gfree_spac function.	*/
/*;bb 032090    Changed DOSLEEPL to DO_SLEEP and removed lock parm,     */
/*;             changed END_SLEEP_NO_LOCK to END_SLEEP because we are   */
/*;             no longer doing sleepl,                                 */
/*;             added code from gswdefine to gfil_gcb.                  */
/*;bb 110590    Fixed window in gswio where hang could occur (before    */
/*;             DISABLE was issued after setup_sio call                 */
/*;bb 042291    Changed logic in gswio for checking if an io outstanding*/
/*;             before SLEEP code.                                      */
/*;bb 050791    Added check in gswio for i/o on pndq for this minor dev */
/*;             using a new variable, io_on_q.                          */
/*;FP 021595    work with >1 user data segment by making greater use    */
/*;             of copyin function in gio_ioctl K_LCW case              */
/*;             change flag on xmattach to USER_ADSPACE                 */
/*;             added trace hooks for ccw chains                        */
/*                                                                      */
/*                                                                      */
/* INTERNAL ENTRY POINTS:                                               */
/*                                                                      */
/*      acc_regs()      set addresses of adapter registers in variables */
/*      com_pin()       pin memory                                      */
/*      com_unpin()     unpin memory                                    */
/*      fpgibufin()     called by opn_x to setup access to the inbound  */
/*                      fpgi buffers                                    */
/*      fpgiclose()     called at close time to unpin and free the      */
/*                      fpgi data areas                                 */
/*      gcln_intr()     clean pointers to specific IQT and zero         */
/*                      out qels. used by close.                        */
/*      gcln_iqt()      clean out entries in specific IQT               */
/*      gerr_log()      logs sio errors                                 */
/*      gfil_ccb()      build ccb for i/o from read or write routine    */
/*      gfil_gcb()      fill in gswcb. used by init.                    */
/*      gfil_iqt()      fill in slot in the interrupt qualifier table   */
/*      gfnd_iqt()      find a slot in the interrupt qualifier table    */
/*                      for a given interrupt type                      */
/*      gfnd_qel()      find an available qel for interrupt routine     */
/*      gfnd_qual()     find a given qualifier in a specific IQT        */
/*      gfree_spac()    unpin space and zero out.                       */
/*      gget_spac()     allocate space, pin it and zero out.            */
/*      gget_qel()      get an event queue element for ioctl call       */
/*      gint_io()       perform read/write operation in interrupt       */
/*                      handler for the given interrupt                 */
/*      gio_ioctl()     do io from ioctl (WRITESF, READ_CUR,SENSE,      */
/*                      standard ioctl write structured field.          */
/*      gopn_gsw()      do 5080 specific open functions.                */
/*      gopn_x()        implements the ext parameter of openx           */
/*      gswio()         issues S_XSIO and checks return code            */
/*      gswtimer()      timer routine                                   */
/*      init_free()     free malloc'ed areas in CFG_INIT when err occurs*/
/*      loadpos()       load POS registers                              */
/*      loaducode()     load microcode onto adapter                     */
/*      usrunpin()      unpinu user buffers                             */
/*                                                                      */
/************************************************************************/

/* INCLUDE FILES   */
#include "gswincl.h"

/* DEFINES         */
#include "gswdefs.h"

/* EXTERN ROUTINES */
#include "gswextr.h"

/* EXTERN DATA     */
#include "gswextd.h"

/* KERNEL IO FILE */
#include <sys/fp_io.h>

/* begin new includes to trace CCWs */

#include <sys/trcmacros.h>
#include <sys/trchkid.h>
#define hkwd_IOCTL_R12 HKWD_PSLA_IOCTL | 12

/* end new includes to trace CCWs */

/*------------ A C C _ R E G S    -----------------------------------*/
/*
 * NAME:        acc_regs()
 *
 * PURPOSE: set ptrs to adapter registers
 *
 * PARMS:       ioadr   bus io addr (shifted 8 bits to the right)
 *
 * CALLING ROUTINE:
 *             loadpos
 *
 * RETURNS:    void
 */
void acc_regs(ioadr)
char ioadr;
{
    uint base_ioadr;
#if 0
    /* 
     * This code is no longer used.  Everywhere these registers are
     * accessed, they are initialized appropriately with BUSMEM_ATT
     * calls.
     */

    /*------------------------------------------------------------------*/
    /* shift 8 bits for adr of 'F000yyrr' where yy = ioadr, rr = reg adr*/
    /*------------------------------------------------------------------*/
    base_ioadr = IOCCADDR() + (((uint)ioadr) << 8);

    PRINT(("acc_regs: base_ioadr = 0x %x \n", base_ioadr));
    rst_rgp  =  (char *)(base_ioadr + ResetIOAdr);
    disi_rgp =  (char *)(base_ioadr + DisIntrIOAdr);
    enai_rgp = (char *)(base_ioadr + EnaIntrIOAdr);
    int_rgp  =  (char *)(base_ioadr + IntIOAdr);
    stop_rgp =  (char *)(base_ioadr + StopIOAdr);
    strt_rgp =  (char *)(base_ioadr + StartIOAdr);
    rsti_rgp =  (char *)(base_ioadr + ResetIntIOAdr);
    stat_rgp =  (char *)(base_ioadr + RdStatIOAdr);
#endif
}

/*---------------  Pin Memory      ------------------------------------*/
int com_pin(adr,len)
caddr_t adr;
int     len;
{
int     rc;
   PRINT(("com_pin:   adr=0x%x,len=0x%x\n",adr,len));
   rc = pinu(adr,len,(short)UIO_SYSSPACE);
   return(rc);
}

/*---------------  Unpin Memory    ------------------------------------*/
int com_unpin(adr,len)
caddr_t adr;
int     len;
{
int     rc;
   PRINT(("com_unpin: adr=0x%x,len=0x%x\n",adr,len));
   rc = unpinu(adr,len,(short)UIO_SYSSPACE);
   return(rc);
}


#ifdef FPGICOMPILE
/*------------ F P G I B U F I N  -----------------------------------*/
/*
 * NAME:        fpgibufin()
 *
 * PURPOSE: pin the user's buffers passed on the OPENX call
 *
 * PARMS:       dev     minor device number
 *              fpgiinp pointer to inbound buffer
 *
 * CALLING ROUTINE:
 *              gswopen (->gopn_x)
 *
 * RETURNS:    (int)
 *              0  -  success
 *              1  -  FPGI mode failed
 *              2  -  FPGI inbound buffers not accessible
 *              3  -  FPGI inbound buffer structure not accessible
 *              4  -  pin of user buffer area failed
 */

int
fpgibufin(dev,fpgiinp)
int dev;
struct fpgi_in *fpgiinp;                /* local var - ptr to fpgi_in   */
{

  int i;
  int rc;
  int num_buf;
  int buf_len;
  int data_len;
  static short srmadata[5] = {0xffee,0,0,0,0};

  register GSWCB *g;
  struct com_rfp     *ccbptr;           /* local var - ptr to ccbs      */
  struct ccw3        *ccwptr;           /* local var - ptr to ccws      */
  struct fpgiincb    *fpcbp;            /* local var - ptr to cb areas  */
  struct in_buf_hdr **balp;             /* local var - ptr to inbuf list*/
  struct in_buf_hdr **pbp;              /* local var - ptr to inbuf list*/

  g = mgcb_ptr + dev;

  /*--------------------------------------------------------*/
  /* Pin the 'fpgiinp' area so the fields can be accessed.  */
  /*--------------------------------------------------------*/
  rc = com_pin(fpgiinp,sizeof(struct fpgi_in));
  if (rc)
      return(4);

  /*--------------------------------------------------------*/
  /* We now have pinned the struct fpgi_in.                 */
  /* Set up local variable to access the user's fpgi_in     */
  /* structure.                                             */
  /* data area len = total length - header                  */
  /*--------------------------------------------------------*/
  num_buf  = fpgiinp->numbuf;
  buf_len  = fpgiinp->buflen;
  data_len = buf_len - sizeof(struct in_buf_hdr);
  balp     = fpgiinp->balp;
  g->num_buf   = num_buf;
  g->buf_len   = buf_len;
  g->fp_inbufs = TRUE;          /*inbound bfrs available    */

  /*--------------------------------------------------------*/
  /* Now pin the list of inbound buffer pointers.           */
  /*--------------------------------------------------------*/
  rc = com_pin(balp,sizeof(long) * num_buf);
  if (rc)
      return(3);

  /*--------------------------------------------------------*/
  /* The list of inbound buffer ptrs is pinned.             */
  /* Now loop thru list of ptrs, pin each of those areas.   */
  /*--------------------------------------------------------*/
  for (i = 0; i < num_buf; i++)
  {
       rc = com_pin((*balp+i),buf_len);
       if (rc) return(2);
  }

  /*--------------------------------------------------------*/
  /* Malloc space for a copy of the user list of buffer     */
  /* pointers.                                              */
  /* Copy those addresses kernel space for later reference. */
  /*--------------------------------------------------------*/
  pbp = (struct in_buf_hdr **)
       gget_spac(sizeof(long) * num_buf,3);
  if (pbp == NULL)
     return(ENOMEM);
  for (i = 0; i < num_buf; i++)
  {
	*(pbp+i) = (struct in_buf_hdr *) (*(balp+i));
  }
  g->pbp = pbp;                     /* put ptr in LD        */

  /*--------------------------------------------------------*/
  /* Everything is now pinned and the g  struct has ptr to  */
  /*    the fpgiin    struct for use when AIX issues a CLOSE*/
  /*--------------------------------------------------------*/

  /*--------------------------------------------------------------------*/
  /* Malloc space for ccb,ces,ccws and the RMI area here.               */
  /* Initialize all fields in a 'for' loop.                             */
  /* Note: the srmadata is the same for all.                            */
  /*       seg  for last CE is user data segid, all others              */
  /*       are in kernel data seg.                                      */
  /*--------------------------------------------------------------------*/
  fpcbp = (struct fpgiincb *)
	   gget_spac(num_buf * sizeof(struct fpgiincb) + RmiSize, 3);
  if (fpcbp == NULL)
     return(ENOMEM);
  g->fpcbp = fpcbp;
  g->rmidata = (char *)(fpcbp+1);
  for(i = 0;i < num_buf; i++)
  {
      ccbptr = &((fpcbp+i)->rfp);
      ccwptr = &((fpcbp+i)->ccw3);

      FCCB(*(ccbptr+i));
      FCEX(*(ccbptr+i),0,LINK1, 24, (ccwptr+i),DMA_KERNEL);
      FCEX(*(ccbptr+i),1,LINK1,  3, g->rmidata,DMA_KERNEL);
      FCEX(*(ccbptr+i),2,LINK1, 10, srmadata  ,DMA_KERNEL);
      FCEX(*(ccbptr+i),3,    0, data_len,(*(pbp+i)+1),DMA_USER);
			    /* data starts after the header */
      FCCW(((ccwptr+i)->rmi),  CCWRMI,  CCWCC+CCWSSLI);
      FCCW(((ccwptr+i)->srma), CCWSRMA, CCWCC);
      FCCW(((ccwptr+i)->rma),  CCWRMA,  0);
  }
  return(0);
}

/*------------ F P G I C L O S E  -----------------------------------*/
/*
 * NAME:        fpgiclose()
 *
 * PURPOSE: perform close operations when in fpgi mode
 *
 * PARMS:       dev     minor device number
 *
 * CALLING ROUTINE:
 *              gswclose
 *
 * RETURNS:    (int)
 */

int
fpgiclose(dev)
int dev;
{

  int i,rc;
  int unique1 = 1;
  int unique2 = 2;
  int unique3 = 3;
  register GSWCB *g;
  struct fpgi_in     *fpgiinp;          /* local var - ptr to fpgi_in   */
  struct fpgiincb    *fpcbp;            /* local var - ptr to cb areas  */
  struct in_buf_hdr **balp;             /* local var - ptr to inbuf list*/
  struct in_buf_hdr **pbp;              /* local var - ptr to inbuf list*/

  g = mgcb_ptr + dev;
  /*-------------------------------------------------------------------*/
  /* Unpin the pinned user areas.                                      */
  /*  g->pp   = the area pointed to by fpgiinp, which points to        */
  /*            the list of pointers to buffer areas, which points to  */
  /*            the first user buffer area                             */
  /* Reverse order of unpinning to ensure access to pointers.          */
  /*-------------------------------------------------------------------*/

  fpgiinp = g->fpgiinp;
  balp    = g->fpgiinp->balp;


  for (i = 0; i < g->num_buf; i++)
  {
    rc = com_unpin(*(balp+i), g->buf_len);      /* user buffers         */
    if (rc)  return(rc);
  }
  rc = com_unpin(balp,sizeof(long) * g->num_buf);  /*list of user bufrs */
  if (rc)  return(rc);
  rc = com_unpin(fpgiinp,sizeof(struct fpgi_in));  /*user's inbnd struct*/
  if (rc)  return(rc);

  /*-------------------------------------------------------------------*/
  /* Unpin and free pin_parms malloced area,                           */
  /* free fpgiincb malloced area.                                      */
  /*-------------------------------------------------------------------*/

  rc = gfree_spac((char **)&g->fpcbp);         /* free mallc'd space   */
  if (rc)
    PRNTE(("fpgiclose: Failed on 'gfree_spac' for 'g->fpcbp'\n"));
  g->fpcbp    = (struct fpgiincb *)0;          /* zero out ptr         */
  g->num_pins = 0;
  g->num_buf  = 0;

  return(rc);
}
#endif /* FPGICOMPILE */


/*------------G C L N _ I N T R -----------------------------------*/
/*
 * NAME:        gcln_intr()
 *
 * PURPOSE: clean ptrs to iqt in headers, zero out qels, release xm.dp
 *
 * PARMS:       dev     minor device number
 *
 * CALLING ROUTINE:
 *              gswclose
 *              gswioctl(K_STOP_DEVICE)
 *
 * RETURNS:     int
 *
 * NOTE:
 *      In the loop to free the IQTs, the assignment to a temp variable
 *      'p' is necessary. Once interrupts are enabled, it is possible that
 *      an interrupt could occur and check '(g->hdr_ptr+i)->intrp'.
 *      If it were not zero, the intrpt hdlr would think it could use it.
 */
int gcln_intr(dev)
dev_t dev;
{
  register GSWCB *g;
  register int i;
  int oldmask;
  int rc;
  int err_val = 0;
  char *p;

  PRINT(("gcln_intr: entered. dev = 0x%x\n",dev));
  g = mgcb_ptr + dev;
  G_DISABLE
  if ( g->io_pend_cnt) {                /* is there an outstanding io?  */
	g->oflag.slp_pending = TRUE;

/* Changed - JCC */
/*    rc = DO_SLEEP(&g->sleep_sio,EVENT_SIGRET); */
      rc = DO_SLEEP(&g->sleep_sio, LOCK_HANDLER | INTERRUPTIBLE);

/* Changed - JCC */
/*    if (rc != EVENT_SUCC) */           /* terminated - didnt get event */
      if (rc != THREAD_AWAKENED)           /* terminated - didnt get event */
		END_SLEEP

	g->oflag.wokeup      = FALSE;
	g->oflag.slp_pending = FALSE;

	if (g->oflag.sig_pending)
	{
		SIGAPPL(g->signal);
		g->oflag.sig_pending = FALSE;
	}
  }
						/*----------------------*/
  if (g->hdr_ptr)                               /* free IQT mallocs     */
  {                                             /*----------------------*/
	for (i = 0; i < ALL_MAX_INP ; i++)
	{
		if ( (g->hdr_ptr+i)->intrp )
		{
			p = (char *)((g->hdr_ptr+i)->intrp);
			(g->hdr_ptr+i)->intrp = 0;
			G_ENABLE
			rc = gfree_spac((char **)&p);
			if (rc)
                           err_val = rc;
			G_DISABLE
		}
	}
  }
  if (g->q.top)                                 /*----------------------*/
  {                                             /* zero qel indices     */
	bzero((char *)(g->q.top),INTRQ_SIZE);   /*----------------------*/
	g->q.head = g->q.top;
	g->q.tail = g->q.top;
  }
  *((short *)&g->oflag) = 0;
  G_ENABLE
  if (g->xm.dp != 0)                            /*----------------------*/
  {                                             /* clean xmem space     */
						/*----------------------*/
	rc = gfree_spac((char **)&g->xm.dp);
	if (rc)
		err_val = rc;
	g->xm.indx = 0;
  }

  /*-------------------------------------------------------------------*/
  /* Free area used for STOP and START struct fields.                  */
  /*-------------------------------------------------------------------*/
  if (g->basesfp)
  {
	rc = gfree_spac((char **)&g->basesfp);
	if (rc)
		err_val = rc;
  }

  /*--------------------------------------------------------------------*/
  /* Unpinu user buffers from last io.                                  */
  /*--------------------------------------------------------------------*/
  if (g->last_ccp)                      /* unpinu usr bfrs from last io */
  {
	rc = usrunpin(&g->last_ccp);
	if (rc)
		err_val = rc;
  }

  /*-------------------------------------------------------------------*/
  /* Free area used for copy of USER ccws.                             */
  /*-------------------------------------------------------------------*/
  if (g->usrccwp)
  {
	rc = gfree_spac((char **)&g->usrccwp);
	if (rc)
		err_val = rc;
  }
  return(err_val);
} /* end of gcln_intr */


/*------------G C L N _ I Q T -----------------------------------*/
/*
 * NAME:        gcln_iqt()
 *
 * PURPOSE:     clean entries in interrupt qualifier table
 *              Loop thru IQTs. When qualifier equals arg.qualifier,
 *              set mode to DISABLE and clean the 'dirty' flag.
 *
 * PARMS:       t_adr   ptr to IQT
 *              arg     ptr to struct k_disable
 *              typ     enum g_type
 *
 * CALLING ROUTINE:
 *              gswioctl(K_DISABLE)
 *
 * RETURNS:     void
 */
void gcln_iqt(t_adr, arg,typ)
register IQT *t_adr;
register struct k_disable *arg;
int typ;
{
  int i;
  int last_indx = tbl_size[typ];

  for (i = 0; i < last_indx; i++) {
	if  ( (t_adr+i)->qualifier == arg->qualifier ) {
		(t_adr+i)->flags.dirty_slot = FALSE;
		(t_adr+i)->flags.mode = GDISABLE;
	}
  }
} /* end of gcln_iqt */


/*------------G E R R _ L O G --------------------------------------*/
/*
 * NAME:        gerr_log()
 *
 * PURPOSE:     log errors before returning to caller
 *              Keep interface the same as PSLA; therefore,
 *                   'fn', typ', 'reason' parms remain PLUS add the
 *                   parms from standard errlog routine.
 *
 * PARMS:       see below.
 *
 * CALLING ROUTINE:
 *              all external and internal driver routines
 *
 * RETURNS:     void
 */

void gerr_log(dev,dmodule,fmodule,err_type,err_reason,ras_uniq)
dev_t dev;

char   *dmodule;        /* Detecting module is module that called       */
			/* failing module Exp: if in VTMOUT and call    */
			/* VTMUPD() and VTMUPD returns error. VTMOUT is */
			/* detecting module & VTMUPD is failing module. */

char   *fmodule;        /* Failing module is module or system call that */
                        /* returned bad  return code.                   */

int    err_type;        /* error type (catagory).                       */

int    err_reason;      /* return code from system call or error reason */
                        /* number residing in RAS.h.                    */

char    ras_uniq;       /* Unique RAS code used to identify specific err*/
			/* locations for error logging. See RAS.H.      */

{
  register GSWCB *g;
  struct log_info l_info;
  static char *psla_name = "SYSXPSLA";
  int  len = sizeof(struct log_info);
  int  tail;
  int  rindex;                          /* Index for detail_data        */
  int  senselen = 0;                    /* length of sense data         */
  uint tmpint;                          /* local temporary int          */

#define REC_ERR _uerr_rec               /* Err rec struct in errids.h   */
  ERR_REC(80) _uerr_rec;

  /*--------------------------------------------------------------------*/
  PRINT(("gerr_log: entered .dev = 0x%x,fmod=%s,retcode=0x%x \n",
		dev, fmodule, err_type));
  PRINT(("      err_reason = 0x%x,ras_uniq=0x%x \n",
		err_reason,ras_uniq));
  g               = mgcb_ptr + (dev % NumDevSupp);
  l_info.class    = G_DVR_CLASS;
  l_info.subclass = G_DVR_SUBCLASS;
  l_info.type     = 0;

  /*--------------------------------------------------------------------*/
  /* If errlog from interrupt handler, check if there is associated     */
  /* sense info. If so, the tail pointer has already been bumped to     */
  /* prepare for the next sense. Back down the tail index by one for    */
  /* the local 'tail' value.                                            */
  /*--------------------------------------------------------------------*/
  if (strcmp(fmodule,"GSWINTR") == 0)
  {
     if ((err_type == BAD_SIO_SENSE) || (err_type == BAD_ASY_SENSE))
     {
	tail = (g->sen_hdrp->tail + SENSE_MODULO - 1) % SENSE_MODULO;
        bcopy( (g->sen_datap) + tail, l_info.data, SEN_MAX);
	senselen = SEN_MAX;
     }
     else
        bzero(l_info.data,SEN_MAX);
  }
  else
     bzero(l_info.data,SEN_MAX);

  l_info.mask       = 2;                /* always use err struct        */
  l_info.len        = (len >> 2);       /* len in words                 */

  /*  'fn' parameter has been removed for AIX V3.			*/
  l_info.err_fn     = 0;
  l_info.err_typ    = err_type;
  l_info.err_reason = err_reason;

  if (g->u_err_area)                    /* copy to user area (from OPEN)*/
	copyout((char *)&l_info,g->u_err_area,len);

  /*--------------------------------------------------------------------*/
  /* Have completed original PSLA error log, except calling 'errsave'.  */
  /* This was necessary to be compatible with current PSLA user i/f.    */
  /* Now do V3 error log.                                               */
  /*--------------------------------------------------------------------*/
  if ((err_type >= SW_ERRVALS_START) && (err_type <= SW_ERRVALS_END))
	REC_ERR.error_id = ERRID_PSLA002;       /* software error       */
  else if (err_type == MSLA_NO_HOST)
	REC_ERR.error_id = ERRID_PSLA003;       /* link failure         */
  else
	REC_ERR.error_id = ERRID_PSLA001;       /* hardware error       */

					/* Failing  SW component name   */
  bcopy(psla_name, (char *)REC_ERR.resource_name,8);

  rindex = 0;
  bcopy(dmodule,(char *)&REC_ERR.detail_data[rindex],8);/*Detectng module*/
  rindex += 8;
  bcopy(fmodule,(char *)&REC_ERR.detail_data[rindex],8);/* failing module*/
  rindex += 8;
  bcopy((char *)&(g->devno),(char *)&REC_ERR.detail_data[rindex],4);
  rindex += 4;                          /* major/minor device           */
  bcopy((char *)&err_type,(char *)&REC_ERR.detail_data[rindex],4);
  rindex += 4;                          /* error type                   */
  bcopy((char *)&err_reason,(char *)&REC_ERR.detail_data[rindex],4);
  rindex += 4;                          /* err reason                   */
  tmpint = (uint)ras_uniq;
  bcopy((char *)&tmpint,(char *)&REC_ERR.detail_data[rindex],4);
  rindex += 4;                          /* ras unique value             */
  if (senselen)                         /* if sense data present        */
  {                                     /* (only for sw error)          */
      bcopy(l_info.data,(char *)&REC_ERR.detail_data[rindex],senselen);
      rindex += senselen;               /* Increment rindex             */
  }
					/* End string with NULL         */
  bcopy('\0',(char *)&REC_ERR.detail_data[rindex + 1], 1);

			/* Call system error logging routine            */
			/* ERRSAVE(). The parameters are a pointer      */
			/* to the error record structure and the        */
			/* number of byte in error record structure     */
  errsave(&REC_ERR, ERR_REC_SIZE + rindex);
  /*****errsave(&REC_ERR, ERR_REC_SIZE + rindex + 2);*****/
} /* end of gerr_log */

/*------------G F I L _ C C B -----------------------------------*/
/*
 * NAME:        gfil_ccb()
 *
 * PURPOSE: build ccb for read and write i/o operations
 *
 * PARMS:
 *              dev             minor device number
 *              uiop            ptr to uio struct
 *              ext             union rw_args
 *              io_type         io type (G_READ or G_WRITE)
 *
 * CALLING ROUTINE:
 *              gswread
 *              gswwrite
 *
 * DESCRIPTION:
 *              fill in CCBs and CEs for read and write operations.
 *
 * RETURNS:    int  ( 0 for success)
 */

int gfil_ccb(dev,uiop,ext,io_type)
register dev_t  dev;            /* minor device number             */
struct uio     *uiop;           /* pre to uio struct               */
union  rw_args  ext;            /* structure                       */
int             io_type;
{

  register int    len;          /* for quick use of user's bfr len   */
  register char  *bfr;          /* for quick use of user's bfr adr   */
  register RWCCB *rw ;
  register GSWCB *g;
  register struct ccw4 *ccw_ptr;
  register SMA   *sm;

  /*-----------------------------------------------------------------*/
  /* Update ccb and set u.u_offset and u.u_count if good io          */
  /*-----------------------------------------------------------------*/
  g   = mgcb_ptr + dev;
  len = uiop->uio_iov->iov_len;
  bfr = uiop->uio_iov->iov_base;
  rw  = g->rw_ccbp + g->rw_io;          /* flip-flop ccbs for async io  */
  sm  = g->rw_smap + g->rw_io;          /* flip-flop smas for async io  */

				       /* flip-flop ccws for async io   */
				       /* note: 4ccws in each group     */
  ccw_ptr = ((struct ccw4 *)(g->rw_ccwp)) + g->rw_io;

  /*-----------------------------------------------------------------*/
  /* Setup ccw based on whether it's a read or a write.              */
  /*-----------------------------------------------------------------*/
  switch(io_type) {
	case G_READ :
		ccw_ptr->rw_sma.cmdcode = CCWSRMA;
		ccw_ptr->rw.cmdcode     = CCWRMA;
		break;
	case G_WRITE :
		ccw_ptr->rw_sma.cmdcode = CCWSWMA;
		ccw_ptr->rw.cmdcode     = CCWWMA;
		break;
	default :
		return(EINVAL);
  }
  sm->memofs = uiop->uio_offset;                /* lseek offset         */
  sm->memcnt = len;                             /* user bfr's length    */

  /*-----------------------------------------------------------------*/
  /* A vanilla r/w operation needs only 2 ccws -                     */
  /*         SRMA/RMA (or SWMA/WMA)                                  */
  /* If the user wants to stop the 5080's DLB, the ccws are          */
  /* prefixed with an additional ccw with a STOP structured field.   */
  /* If the user wants to start the 5080's DLB, the ccws are         */
  /* postfixed with an additional ccw with a START structured field. */
  /* Therefore, there can be from 2-4 ccws associated with the       */
  /* r/w operation (and therefore from 3-5 ce's).                    */
  /*-----------------------------------------------------------------*/
  if (ext.value == 0) {
	FCEX(*rw, 0 , LINK1, 16,&(ccw_ptr->rw_sma),DMA_KERNEL);
	FCEX(*rw, 1 , LINK1, 10, sm,               DMA_KERNEL);
	FCEX(*rw, 2 ,    0 ,len, bfr,              DMA_USER);
	ccw_ptr->rw.flags &= (~CCWCC);
  }
  else {
	if (ext.valuep->stop)  {

		FCEX(*rw, 0 , LINK1, 24, &(ccw_ptr->stop),DMA_KERNEL);
		FCEX(*rw, 1 , LINK1,  6, g->basesfp,      DMA_KERNEL);
		FCEX(*rw, 2 , LINK1, 10, sm,              DMA_KERNEL);
		FCEX(*rw, 3 ,    0 ,len, bfr,             DMA_USER);

		if (ext.valuep->start) {
			(*rw).ce[0].tranlen = 32;
			(*rw).ce[3].res1    = (ushort)LINK1;
			FCEX(*rw, 4 , 0 , 6,(g->basesfp+1),DMA_KERNEL);
			ccw_ptr->rw.flags  |= CCWCC;
		}
		else
			ccw_ptr->rw.flags &= (~CCWCC);
	}
	else {
		FCEX(*rw, 0 , LINK1, 16, &(ccw_ptr->rw_sma),DMA_KERNEL);
		FCEX(*rw, 1 , LINK1, 10, sm,                DMA_KERNEL);
		FCEX(*rw, 2 ,    0 ,len, bfr,               DMA_USER);
		ccw_ptr->rw.flags &= (~CCWCC);
	}
  }
  return(0);
}

/*------------G F I L _ G C B -------------------------------------*/
/*
 * NAME:        gfil_gcb()
 *
 * PURPOSE:     fill in gcb control block
 *
 * PARMS:
 *              g       ptr to GSWCB
 *
 * CALLING ROUTINE:
 *              gswconfig at INIT time
 *
 * DESCRIPTION:
 *              malloc space, set ptrs for areas used for life of driver
 *
 * RETURNS:   (int)   0 success
 *                   !0 failure (ERRNO)
 */
int gfil_gcb(g)
register GSWCB *g;
{
  register struct ccw4 *ccw_ptr;
  register struct ccw2 *fccw_ptr;
  char *p;
  int   i;
  int   cnt;
  int   malloc_size;
  dev_t dev;

   /*-------------------------------------------------------------------*/
   /* Initialize cnt of struct recording malloced area                  */
   /* Get space for structures used in IOCTL                            */
   /* Set 'g' fields 'sfp,sflp,smp,ccp,cbp'.                            */
   /*-------------------------------------------------------------------*/
   dev = minor(g->devno);               /* minor number                 */
   malloc_size = sizeof(struct basic_ws) + sizeof(struct sflpat) +
		 sizeof(RWCCB)           + sizeof(struct sma) +
		 sizeof(ccw_t) * 4;
   p = (char *)gget_spac(malloc_size,12);
   if (p == NULL)
      return(ENOMEM);
   pa[dev].ma[0].adr = p;
   pa[dev].ma[0].len = malloc_size;
   pa[dev].cnt = 1;
   g->sfp  = (struct basic_ws *)p;
   g->sflp = (struct sflpat *)(g->sfp  + 1);
   g->cbp  = (RWCCB *)        (g->sflp + 1);
   g->smp  = (struct sma *)   (g->cbp  + 1);
   g->ccp  = (ccw_t *)        (g->smp  + 1);

  /*-----------------------------------------------------------------*/
  /* Malloc a large area and carve up.                               */
  /* Fill in static fields of gcb.                                   */
  /* For rw and ws, two sets of ccws, ccbs and smas are used         */
  /*     for dual io                                                 */
  /* Put adr and len in 'pa' struct for freeing  at TERM time.       */
  /*-----------------------------------------------------------------*/

  malloc_size =                         /*                 quantity     */
	 (sizeof(RWCCB)    << 1) +      /*  g->rw_ccbp        2         */
	 (sizeof(ccw_t)    << 3) +      /*  g->rw_ccwp        8         */
	 (sizeof(SMA)      << 1) +      /*  g->rw_smap        2         */
	 (sizeof(RWCCBFP)  << 1) +      /*  g->fp_ccbp        2         */
	 (sizeof(ccw_t)    << 2) +      /*  g->fp_ccwp        4         */
	 (sizeof(SMA)      << 1) +      /*  g->fp_smap        2         */
	 (sizeof(WSCCB)    << 1) +      /*  g->ws_ccbp        2         */
	 (sizeof(ccw_t)    << 1) +      /*  g->ws_ccwp        2         */
	  sizeof(RWCCB)          +      /*  g->in_ccbp        1         */
	  4*sizeof(ccw_t)        +      /*  g->in_ccwp        4         */
	  sizeof(SMA)            +      /*  g->in_swmap       1         */
	  sizeof(SMA)            +      /*  g->in_srmap       1         */
	  PAGESIZE;                     /*  extra page for 'flush' (dma)*/

  p = (char *)gget_spac(malloc_size, 12);
  if (p == NULL)                        /* could not get space          */
	return(ENOMEM);

  /*-----------------------------------------------------------------*/
  /* Use 'pa' for saving pin'ed area ptrs and size (to be unpin'ed   */
  /* at 'close' time).                                               */
  /*-----------------------------------------------------------------*/
  dev = minor(g->devno);
  cnt = pa[dev].cnt;
  pa[dev].ma[cnt].adr = p;              /* save adr of malloced area    */
  pa[dev].ma[cnt].len = malloc_size;    /* save len of malloced area    */
  pa[dev].cnt++;                        /* incr cnt of # pinned areas   */

  g->rw_ccbp = (RWCCB *)   p;
  g->rw_ccwp = (ccw_t *)   (g->rw_ccbp  + 2);
  g->rw_smap = (SMA *)     (g->rw_ccwp  + 8);
  g->fp_ccbp = (RWCCBFP *) (g->rw_smap  + 2);
  g->fp_ccwp = (ccw_t *)   (g->fp_ccbp  + 2);
  g->fp_smap = (SMA *)     (g->fp_ccwp  + 4);
  g->ws_ccbp = (WSCCB *)   (g->fp_smap  + 2);
  g->ws_ccwp = (ccw_t *)   (g->ws_ccbp  + 2);
  g->in_ccbp = (RWCCB *)   (g->ws_ccwp  + 2);
  g->in_ccwp = (ccw_t *)   (g->in_ccbp  + 1);
  g->in_swmap = (SMA *)    (g->in_ccwp  + 4);
  g->in_srmap = (SMA *)    (g->in_swmap + 1);

  /*-----------------------------------------------------------------*/
  /* Malloc space for event queue, sense area and intr table         */
  /*-----------------------------------------------------------------*/
  g->q.top     = (q_qel *)gget_spac(INTRQ_SIZE + 20,3);
  g->sen_hdrp  = (struct u_sen_hdr *)gget_spac(
		 (sizeof(union u_sen_data))*(SENSE_MODULO) +
		 sizeof(struct u_sen_hdr),3);
  g->hdr_ptr   = (q_hdr *)
		 gget_spac(ALL_MAX_INP * sizeof(q_hdr), 2);

  if (
     (g->q.top    == NULL)  ||
     (g->hdr_ptr  == NULL)  ||
     (g->sen_hdrp == NULL) )
	return(ENOMEM);
  g->q.bottom       = (q_qel *) ( ((char *)g->q.top) + INTRQ_SIZE );
  g->q.head         = g->q.top;
  g->q.tail         = g->q.top;
  g->q.bottom->type = GALL;
  g->sen_datap      = (union u_sen_data *)(g->sen_hdrp + 1);

  /*--------------------------------------------------------------------*/
  /* Put adr and len in 'pa' struct for freeing  at TERM time.          */
  /*       q.top, sen_hdrp, hdrptr.                                     */
  /*--------------------------------------------------------------------*/
  cnt = pa[dev].cnt;
  pa[dev].ma[cnt].adr = (char *)g->q.top;               /* save adr     */
  pa[dev].ma[cnt].len = INTRQ_SIZE + 20;                /* save len     */
  pa[dev].cnt++;                                        /* incr cnt     */

  cnt = pa[dev].cnt;
  pa[dev].ma[cnt].adr = (char *)g->sen_hdrp;            /* save adr     */
  pa[dev].ma[cnt].len = sizeof(struct u_sen_hdr) +      /* save len     */
			(sizeof(union u_sen_data))*(SENSE_MODULO);
  pa[dev].cnt++;                                        /* incr cnt     */

  cnt = pa[dev].cnt;
  pa[dev].ma[cnt].adr = (char *)g->hdr_ptr;             /* save adr     */
  pa[dev].ma[cnt].len = ALL_MAX_INP * sizeof(q_hdr);    /* save len     */
  pa[dev].cnt++;                                        /* incr cnt     */

  /*-----------------------------------------------------------------*/
  /* fill in r/w ccb static fields                                   */
  /*-----------------------------------------------------------------*/

  ccw_ptr  = (struct ccw4 *)(g->rw_ccwp);
  fccw_ptr = (struct ccw2 *)(g->fp_ccwp);

  FSMA(*(g->in_srmap),DLBAREA,0,10);
  FSMA(*(g->in_swmap),DLBAREA,0,10);

  FCCB(*(g->in_ccbp));
					/*------------------------------*/
					/* need loop for dual io        */
					/*------------------------------*/
  for ( i = 0; i < MAX_MULTI_IO ; i++)
  {
	FCCB(*(g->rw_ccbp + i));
	FCCB(*(g->ws_ccbp + i));
	FCCB(*(g->fp_ccbp + i));                            /* FOR FPGI */

	FCEX(*(g->ws_ccbp + i), 0, LINK1, 8, 0,DMA_KERNEL);
	FCEX(*(g->ws_ccbp + i), 1, 0,     0, 0,DMA_KERNEL);

/****   need  another CE for IOCTL call FPGI_SEND *******************/
	FCEX(*(g->fp_ccbp + i),0,LINK1,sizeof(struct ccw2),0,DMA_KERNEL);
	FCEX(*(g->fp_ccbp + i),1,LINK1,10,                 0,DMA_USER);

	FSMA(*(g->rw_smap + i),DLBAREA,  0,10);
	FSMA(*(g->fp_smap + i),FSMAMEMID,0,10); /* FOR FPGI */

	/*-------------------------------------------------------------*/
	/* fill in sma fields for read and write and iosinit intr hdlr */
	/*-------------------------------------------------------------*/

	FCCW(  (ccw_ptr+i)->stop    ,CCWWS  ,CCWCC);
	FCCW(  (ccw_ptr+i)->rw_sma  ,CCWSRMA,CCWCC);
	FCCW(  (ccw_ptr+i)->rw      ,CCWRMA , 0   );
	FCCW(  (ccw_ptr+i)->start   ,CCWWS  , 0   );
	FCCW( *(g->ws_ccwp+i)       ,CCWWS  , 0   );
	FCCW(  (fccw_ptr+i)->rw_sma ,CCWSWMA,CCWCC);
	FCCW(  (fccw_ptr+i)->rw     ,CCWWMA , 0   );
  }
  return(0);
} /* end of gfil_gcb */


/*------------G F I L _ I Q T -----------------------------------*/
/*
 * NAME:        gfil_iqt()
 *
 * PURPOSE:     fill a slot in the interrupt qualifier table
 *
 * PARMS:
 *              t_adr   ptr to IQT
 *              arg     ptr to struct k_disable
 *              typ     enum g_type
 *
 * CALLING ROUTINE:
 *              gopn_x
 *              ioctl(K_ENABLE)
 *              ioctl(K_REQUEST)
 *
 * RETURNS:    void
 */
void gfil_iqt(t_adr, arg,typ)
register IQT *t_adr;
register struct k_enable *arg;
int typ;
{
  PRINT(("gfil_iqt: t_adr = 0x%x,arg=0x%x,typ=0x%x\n",t_adr,arg,typ));
  t_adr->type             = typ;
  t_adr->buf_adr          = arg->buf_adr;
  t_adr->data_len         = arg->data_len;
  t_adr->slot_len         = sizeof(q_qel) +
			    ((arg->data_len+3) & ZERO_LOW2BITS) ;
  t_adr->qualifier        = arg->qualifier;
  t_adr->w_adr            = arg->w_adr;
  t_adr->flags.dirty_slot = TRUE;
}
/* end of gfil_iqt */

/*------------G F N D _ I Q T -----------------------------------*/
/*
 * NAME:        gfnd_iqt()
 *
 * PURPOSE:     find a slot in the interrupt qualifier table
 *              used only for SMI and GEOP interrupts
 *
 * PARMS:
 *              t_adr   ptr to IQT
 *              arg     ptr to struct k_disable
 *              typ     enum g_type
 *
 * CALLING ROUTINE:
 *              gopn_x
 *              ioctl(K_ENABLE)
 *              ioctl(K_REQUEST)
 *
 * DESCRIPTION:
 *              If qualifier = -1, then use last entry in table, which
 *              is used specifically for the "anything else" qualifiers.
 *              Else, search thru table until qualifier matches or
 *              last entry reached.
 *
 * RETURNS:    IQT *    adr of table entry or NULL if not found
 */
IQT *gfnd_iqt(t_adr, arg,typ)
register IQT *t_adr;
register struct k_enable *arg;
int typ;
{
  int i;
  int last_indx = tbl_size[typ] - 1;


  if ( ((typ == (int)Gsmi) || (typ == (int)Ggeop)) &&
     (arg->qualifier == -1) )
	return(t_adr+last_indx);
  for (i = 0; i < last_indx; i++,t_adr++) {
	if ( (t_adr->qualifier == arg->qualifier) ||
	     (t_adr->flags.dirty_slot == 0) ) break;
  }
  return( (i == last_indx) ? NULL : t_adr );
} /* end of gfnd_iqt */

/*------------G F N D _ Q E L -----------------------------------*/
/*
 * NAME:        gfnd_qel()
 *
 * PURPOSE:     find a qel for the interrupt handler
 *
 * PARMS:       dev     minor device number
 *              size    qel size needed (varies with interrupt type)
 *
 * CALLING ROUTINE:
 *              gswintr
 *
 * RETURNS:    q_qel *  adr of qel or NULL  if cannot get one
 */
q_qel *gfnd_qel(dev,size)
dev_t dev;
int   size;
{
  register GSWCB *g;
  register q_qel *ptr,*end_ptr,*head,*tail,*top,*bottom;

  g       = mgcb_ptr + dev;
  head    = g->q.head;
  tail    = g->q.tail;
  top     = g->q.top;                           /* top of queue         */
  bottom  = g->q.bottom;                        /* bottom of queue      */
  end_ptr = g->q.bottom;                        /* end of space availabl*/

  if (head < tail)                              /* if head before tail, */
	end_ptr = tail;                         /* set end-of-space ptr */
  else {
	if ( ((int)bottom - (int)head) < size ) {
		end_ptr    = tail;              /* end-of-space ptr     */
		head->type = GALL;              /* marks unused area    */
		if (head == tail)               /* if empty, strt at top*/
			tail = top;
		head = top;                     /* no room, strt at top */
	}
  }

  if ( ((int)end_ptr - (int)head) < size )      /* no room.             */
	return(NULL);

  if (head == tail)                             /* empty -> non-empty   */
	g->oflag.q_chng_state = TRUE;

  ptr       = head;
  g->q.head = (q_qel *)((int)head + size);
  return(ptr);
} /* end of gfnd_qel */

/*------------G F N D _ Q U A L ---------------------------------*/
/*
 * NAME:        gfnd_qual()
 *
 * PURPOSE:     find a qualifier in the interrupt qualifier table
 *
 * PARMS:       t_adr      ptr to IQT
 *              type       enum g_type
 *              qualifier  qualifier
 *
 * CALLING ROUTINE:
 *              gswintr
 *
 * RETURNS:    IQT *    ptr to the entry or NULL if not found
 */
IQT *gfnd_qual(t_adr,type, qualifier)
register IQT *t_adr;
int type,qualifier;
{
  int i;
  int last_indx = tbl_size[type] - 1;

  for (i = 0; i < last_indx; i++,t_adr++) {
	if (  (t_adr->qualifier == qualifier) &&
	      (t_adr->flags.dirty_slot == TRUE) )
		break;
  }
  if ( (i == last_indx) && (t_adr->flags.dirty_slot == FALSE) )
	return(NULL);
  return(t_adr);
} /* end of gfnd_qual */

/*------------G F R E E _ S P A C   -----------------------------*/
/*
 * NAME:        gfree_spac()
 *
 * PURPOSE:     free space alloced in gget_spac(), unpin area, zero ptr
 *
 * PARMS:       ptr        adr of ptr to allocated space
 *              size       # bytes allocated
 *
 * CALLING ROUTINE:
 *              many driver routines
 *
 * RETURNS:    int      0 for success, non-zero for error
 */
int   gfree_spac(ptr)
char **ptr;
{
  int   rc;

  PRINT(("gfree_spac: *ptr = 0x %x, \n", *ptr));
  rc = xmfree((void *)(*ptr),pinned_heap);
  if (rc != 0)
	PRNTE(("gfree_spac: FAILURE of xmfree. *ptr = 0x%x\n",
		    *ptr));
  *ptr = (char *)0;             /* zero out ptr indicating free  */
  return(rc);
} /* end of gfree_spac */

/*------------G G E T _ S P A C ---------------------------------*/
/*
 * NAME:        gget_spac()
 *
 * PURPOSE:     xmalloc space, pin it, zero it out
 *
 * PARMS:       ptr        ptr to allocated space
 *              size       # bytes allocated
 *
 * CALLING ROUTINE:
 *              many driver routines
 *
 * RETURNS:    char *   returns NULL if error, else return ptr to storage
 */
char *gget_spac(size,align)
int   size;
int   align;
{
  char *ptr;

  ptr = (char *)xmalloc(size,align,pinned_heap);
  PRINT(("gget_spac: space=0x%x,size=%d,align=0x%x\n", ptr, size,align));
  if (ptr)
	    bzero(ptr,size);
  else                                  /* failure to get space         */
	    PRNTE(("gget_spac: FAILURE of xmalloc. size = %d\n",size));
  return(ptr);
} /* end of gget_spac */

/*------------G G E T _ Q E L -------------------------------------*/
/*
 * NAME:        gget_qel()
 *
 * PURPOSE:     get a qel to be passed back to user
 *
 * PARMS:       dev        minor device number
 *
 * CALLING ROUTINE:
 *              ioctl(K_POLL)
 *              ioctl(K_WAIT)
 *
 * RETURNS:    q_qel *  ptr to qel or NULL if queue empty
 */
q_qel *gget_qel(dev)
dev_t dev;
{
  register GSWCB *g;
  register q_qel *qel_ptr;
  register q_qel *head;
  register q_qel *tail;
  int             oldmask;

  g = mgcb_ptr + dev;
  G_DISABLE

  head = g->q.head;
  tail = g->q.tail;
  if (head == tail) {                           /* queue empty          */
	G_ENABLE
	return(NULL);
  }

  if (tail->type == GALL) {                     /* bottom one is unused */
	if (tail != g->q.bottom)                /* free it for next time*/
		tail->type = GNULL;
	tail = g->q.top;                        /* use the top qel      */
  }

  qel_ptr   = tail;                             /* set ptr to a qel.    */
						/* update tail ptr for  */
						/*   next time.         */
  g->q.tail = (q_qel *)((char *)tail + tail->data_len + sizeof(q_qel) +
			 (3*(tail->data_len)%4));
  G_ENABLE
  return(qel_ptr);

} /* end of gget_qel   */

/*------------G I N T _ I O -------------------------------------*/
/*
 * NAME:        gint_io())
 *
 * PURPOSE:     do r/w operation from interrupt hdlr for unsol intrpts.
 *
 * PARMS:       dev     minor device number
 *              t_adr   ptr to IQT
 *              rmap    ptr to RMA data
 *
 * CALLING ROUTINE:
 *              gswintr
 *
 * RETURNS:     returns 0 if ok, non-zero if failure
 */
int  gint_io(dev, t_adr, rmap)
dev_t dev;
register IQT *t_adr;
char *rmap;
{
  int rc;
  register GSWCB *g;

  g = mgcb_ptr + dev;
  /*--------------------------------------------------------------------*/
  /* Check if data is to be read when an interrupt occurs.              */
  /* In  the vanilla situation, a SRMA/RMA is done.                     */
  /* Additionally, there are special cases for Gsmi and Ggeop, as       */
  /* described below.                                                   */
  /*--------------------------------------------------------------------*/
  PRINT(("gint_io: dev=0x%x,t_adr=0x%x,rmap=0x%x\n",dev,t_adr,rmap));
  if (t_adr->data_len) {
	FCCW(*(g->in_ccwp),CCWSRMA,CCWCC);
	FCCW(*(g->in_ccwp+1),CCWRMA,0);

	g->in_srmap->memofs = (unsigned)t_adr->buf_adr;
	g->in_srmap->memcnt = t_adr->data_len;

	FCEX(*(g->in_ccbp),0,LINK1,16,g->in_ccwp,DMA_KERNEL);
	FCEX(*(g->in_ccbp),1,LINK1,10,g->in_srmap,DMA_KERNEL);
	FCEX(*(g->in_ccbp),2,0    ,t_adr->data_len,rmap, DMA_KERNEL);

	(g->in_ccwp+1)->flags = 0;

	if (t_adr->w_adr != (char *) -1) {
	    switch(t_adr->type) {
	    case Gsmi :
		/*------------------------------------------------------*/
		/* Check 'w_adr' value passed by user. if w_adr = -1,   */
		/* there is no write operation performed.               */
		/* However, if w_adr != -1, two bytes of 0 are written  */
		/* to the 5080 buffer at an address specified by the    */
		/* user. This requires a SWMA/WMA operation.            */
		/*------------------------------------------------------*/
		FCCW(*(g->in_ccwp+2),CCWSWMA,CCWCC);
		FCCW(*(g->in_ccwp+3),CCWWMA,0);

		(g->in_ccbp)->ce[0].tranlen = (unsigned)32;
		(g->in_ccbp)->ce[2].res1    = (ushort)LINK1;

		FCEX(*(g->in_ccbp),3,LINK1,10,g->in_swmap,DMA_KERNEL);
		FCEX(*(g->in_ccbp),4,0    , 2,g->zerop   ,DMA_KERNEL);

		g->in_swmap->memofs   = (unsigned)t_adr->w_adr;
		g->in_swmap->memcnt   = 2;
		(g->in_ccwp+1)->flags = CCWCC;

		break;

	      case Ggeop :
		/*------------------------------------------------------*/
		/* Check 'w_adr' value passed by user. if w_adr = -1,   */
		/* then do not restart the 5080 DLB for Geop.           */
		/* However, if w_adr != -1, a restart of the 5080 buffer*/
		/* is to be done. This requires the START structured    */
		/* field operation.                                     */
		/*------------------------------------------------------*/
		FCCW(*(g->in_ccwp+2),CCWWS,0);

		(g->in_ccbp)->ce[0].tranlen = (unsigned)24;
		(g->in_ccbp)->ce[2].res1    = (ushort)LINK1;

		FCEX(*(g->in_ccbp),3,0    ,6,(g->basesfp+1),DMA_KERNEL);
		break;

	      default :
		break;
	    }
	}
  }
  else if (t_adr->w_adr != (char *) -1) {
	switch(t_adr->type) {
	    case Gsmi :
		/*------------------------------------------------------*/
		/* See explanation above.                               */
		/*------------------------------------------------------*/
		FCCW(*(g->in_ccwp),CCWSWMA,CCWCC);
		FCCW(*(g->in_ccwp+1),CCWWMA,0);

		FCEX(*(g->in_ccbp),0,LINK1,16,g->in_ccwp,DMA_KERNEL);
		FCEX(*(g->in_ccbp),1,LINK1,10,g->in_swmap,DMA_KERNEL);
		FCEX(*(g->in_ccbp),2,0    , 2,g->zerop   ,DMA_KERNEL);

		g->in_swmap->memofs = (unsigned)t_adr->w_adr;
		g->in_swmap->memcnt = 2;
		break;

	    case Ggeop :
		/*------------------------------------------------------*/
		/* See explanation above.                               */
		/*------------------------------------------------------*/
		FCCW(*(g->in_ccwp),CCWWS,0);
		FCEX(*(g->in_ccbp),0,LINK1,8,g->in_ccwp,    DMA_KERNEL);
		FCEX(*(g->in_ccbp),1,0    ,6,(g->basesfp+1),DMA_KERNEL);
		break;

	    default :
		return(0);
	}
  }
  if ( !(g->iflag.io_allowed) ) {
	switch (g->uns_res) {
		case MSLA_NO_5080 :
			rc = ENODEV;
			break;
		case NOT_CONFIGURED :
			rc = ETXTBSY;
			break;
		case MSLA_LM :
		default :
			rc = EBUSY;
			break;
	}
	return(rc);
  }
  mf.intio_in_prog = TRUE;
  return(setup_sio(g->in_ccbp,dev,g->intdp));
} /* end of gint_io */

/*------------G I O _ I O C T L -----------------------------------*/
/*
 * NAME:        gio_ioctl()
 *
 * PURPOSE:     do i/o operation from gswioctl
 *		changes to K_LCW case to work with more		02/15/95
 *		than 1 user data segment			02/15/95
 *		Performance may be degraded for single segment	02/15/95
 *		case because always using copyin.		02/15/95
 *
 * PARMS:       dev             minor device number
 *              ioctl_type      case from ioctl
 *              arg             parm for case
 *              wslen           write structured length
 *
 * CALLING ROUTINE:
 *              gswioctl
 *
 * RETURNS:     returns 0 if ok, non-zero if failure
 */
int gio_ioctl(dev,ioctl_type,arg,wslen)
dev_t          dev;
int            ioctl_type;
union ctl_args arg;
int            wslen;
{
  register GSWCB *g;
  char            cc_or_cd;

  int       i;
  int       rc;
  int       ccw_cnt;
  RWCCB    *lclcbp;                     /* local ccp ptr                */
  ccw_t    *ccw_ptr;                    /* local ccw ptr                */

  union parms  {
	struct k_enable k_ena;
	struct k_disable k_dis;
	struct k_request k_req;
	struct g_writesf g_wsf;
	struct g_ld_blnk g_blnk;
	struct g_ld_line g_line;
	struct k_lcw k_lcw;
  } val;
  union parms *parmsp = &val;

  g = mgcb_ptr + dev;
  switch(ioctl_type) {
	case G_WRITESF :
		copyin( arg.valuep,(char *)parmsp,12);

		lclcbp = g->ws_ccbp + g->ws_io;
		lclcbp->ce[0].memaddr  = (caddr_t)(g->ws_ccwp + g->ws_io);
		lclcbp->ce[1].tranlen  = parmsp->g_wsf.data_len;
		lclcbp->ce[1].memaddr  = (caddr_t)(parmsp->g_wsf.buf_adr);
		lclcbp->ce[1].res3 = DMA_USER;

		rc = gswio(dev,lclcbp,parmsp->g_wsf.async_io,WS_IO);
		break;

	case G_READ_CUR :
		g->smp->memid  = DLBAREA;
		g->smp->flags  = 0x0040;
		g->smp->memcnt = 4;
		g->smp->memofs = 0;

		FCCW(*g->ccp,     CCWSRMA,CCWCC);
		FCCW(*(g->ccp +1),CCWRMA, 0);

		FCCB(*g->cbp);

		FCEX(*g->cbp, 0, LINK1, 16, g->ccp,    DMA_KERNEL);
		FCEX(*g->cbp, 1, LINK1, 10, g->smp,    DMA_KERNEL );
		FCEX(*g->cbp, 2,    0 ,  4, arg.valuep,DMA_USER);

		rc = gswio(dev,g->cbp,G_SYNC_IO,NULL);
		break;

	case G_SENSE :
		FCCW(*g->ccp,CCWSENSE ,0);
		FCCB(*g->cbp);
		FCEX(*g->cbp, 0, LINK1,8,       g->ccp,    DMA_KERNEL);
		FCEX(*g->cbp, 1,    0 ,SEN_MAX, arg.valuep,DMA_USER);

		rc = gswio(dev,g->cbp,G_SYNC_IO,NULL);
		break;

	case G_IOCTL :
		lclcbp                 = g->ws_ccbp + g->ws_io;
		lclcbp->ce[0].memaddr  = (caddr_t)(g->ws_ccwp + g->ws_io);
		lclcbp->ce[1].tranlen  = wslen;
		lclcbp->ce[1].memaddr  = (caddr_t)(arg.valuep);
		lclcbp->ce[1].res3     = DMA_KERNEL;
		rc = gswio(dev,lclcbp,G_SYNC_IO,WS_IO);
		break;

	case K_LCW :
		{
		/* all references to any of the following 4 fields
		are part of the change to handle more than one
		user data segment, so all those lines will not be
		explicitly marked with a change activity date	02/15/95 */
		#define TIC 8 /* add transfer in chain handling	02/15/95 */
		ccw_t local_ccw;  /* for copyin use 		02/15/95 */
		ccw_t *root_ptr;  /* save start addr 		02/15/95 */
		ccw_t *local_ptr; /* for copyin use 		02/15/95 */

		copyin( arg.valuep,(char *)parmsp,8); /*fetch root & async flg*/
		local_ptr = parmsp->k_lcw.adr;
		root_ptr = local_ptr; /* fetch ptr and save it */

		cc_or_cd = CCWCC + CCWCD;
		copyin(local_ptr, &local_ccw, sizeof(ccw_t)); /* prime ccw */

		TRCHKL3T(hkwd_IOCTL_R12,  /* trace the address and data */
			local_ptr, *(int*)&local_ccw, local_ccw.dataptr);

		if (local_ccw.cmdcode == TIC) /* handle the initial TIC */
		  local_ccw.flags = CCWCC;

					/*------------------------------*/
					/* count the number of ccws     */
					/*------------------------------*/
		for (ccw_cnt = 1; local_ccw.flags & cc_or_cd;
		     ccw_cnt++) {
			if (local_ccw.cmdcode == TIC) { /* through TC */
			  copyin((char*)local_ccw.dataptr, &local_ptr, 
				sizeof(local_ptr));
			  copyin(local_ptr, &local_ccw, sizeof(ccw_t));

			TRCHKL3T(hkwd_IOCTL_R12,  /* trace the address and data */
				local_ptr, *(int*)&local_ccw, local_ccw.dataptr);

			}
			else  {
			  local_ptr++;
			  copyin(local_ptr, &local_ccw, sizeof(ccw_t)) ;

			TRCHKL3T(hkwd_IOCTL_R12,  /* trace the address and data */
				local_ptr, *(int*)&local_ccw, local_ccw.dataptr);

			}
                        
                }

		local_ptr = root_ptr; /* start over  */
		copyin(local_ptr, &local_ccw, sizeof(ccw_t)); /* prime cc */

					/*------------------------------*/
					/* free last lcw ccbs space     */
					/*------------------------------*/
		if (g->lcwccb_adr[g->lc_io])
        	   rc = gfree_spac((char **)&g->lcwccb_adr[g->lc_io]);
		lclcbp = (RWCCB *)gget_spac((int)(sizeof(struct ccb) +
			(1+ccw_cnt)*(sizeof(struct com_elm))),3);
					/*------------------------------*/
					/* Fill in lcw fields of 'g'.   */
					/*------------------------------*/
		g->lcwccw_adr[g->lc_io] = local_ptr; /* ccw_ptr;  ccws */
		g->lcwccb_adr[g->lc_io] = lclcbp;               /* ccbs */
		g->lcwccw_cnt[g->lc_io] = ccw_cnt;              /*# ccws*/

		if (lclcbp == NULL)
			return(ENOMEM);

		FCCB(*lclcbp);
		FCEX(*lclcbp,0,LINK1,ccw_cnt<<3, root_ptr /*parmsp->k_lcw.adr*/,
						   DMA_USER);
					/*------------------------------*/
					/* fill ce's                    */
					/*------------------------------*/
		for (i = 1; i < ccw_cnt; i++) {
		  if (local_ccw.cmdcode == TIC) { /* through TC */
		    copyin((char*)local_ccw.dataptr, &local_ptr, 
			sizeof(local_ptr));
		    copyin(local_ptr, &local_ccw, sizeof(ccw_t));
		  }
			FCEX(*lclcbp,i,LINK1,local_ccw.count,
			    local_ccw.dataptr,DMA_USER);

		  local_ptr++;
		  copyin(local_ptr, &local_ccw, sizeof(ccw_t));
		}
		FCEX(*lclcbp,i,0,local_ccw.count, local_ccw.dataptr, DMA_USER);
		rc = gswio(dev,lclcbp,parmsp->k_lcw.async_io,LC_IO);
		break;
		}

	default :
		rc = EINVAL;
		break;
  }
  return(rc);
} /* end of gio_ioctl */

/*------------G O P N _ G S W -------------------------------------*/
/*
 * NAME:        gopn_gsw()
 *
 * PURPOSE:     do 5080-specific open functions
 *
 * PARMS:       dev             minor device number
 *
 * CALLING ROUTINE:
 *              gswopen
 *
 * RETURNS:     returns 0 if ok, non-zero if failure
 */
int gopn_gsw(dev)
dev_t dev;
{

  int i;
  int rc, ret;
  register GSWCB *g;
  RWCCB *cbp;
  ccw_t *opnccwp;               /* ptr to ccw malloced later            */
  ushort *smrmip;               /* data area for setmode/rmi            */
  char *pinp;                   /* pinned page for struct xmem and ccws */

  g = mgcb_ptr + dev;

  smrmip  = (ushort *)gget_spac(PAGESIZE,12);   /* for setmode and rmi */
  if (smrmip == NULL)
	return(ENOMEM);
  pinp    = (char *)(smrmip + 20);                      /* for xmem     */
  opnccwp = (ccw_t *)(pinp + sizeof(struct xmem));      /* for ccws     */
  cbp     = (RWCCB *)(opnccwp + 4);                     /* for ccbs     */

  switch (g->devmode)
  {
      case FPGI_MODE:
	*smrmip = SETMODE_FPGI;
	break;
      case SYS5080_MODE:
      default:
	*smrmip = SETMODE_5080;
	break;
  }

  /*---------------------------------------------------------------*/
  /* set mode to force 24 bytes returned for a sense;              */
  /* read  manual input register to unlock keyboard.               */
  /* build ccb for sio.                                            */
  /*---------------------------------------------------------------*/
  FSMA(*(g->rw_smap),  DLBAREA,0,10);
  FSMA(*(g->rw_smap+1),DLBAREA,0,10);

  FCCB(*cbp);
  FCEX(*cbp, 0, LINK1,16, opnccwp,  DMA_KERNEL);
  FCEX(*cbp, 1, LINK1, 2, smrmip,   DMA_KERNEL);
  FCEX(*cbp, 2, 0    , 3,(smrmip+2),DMA_KERNEL);

  FCCW(*opnccwp,CCWSETM,CCWCC);
  FCCW(*(opnccwp+1),CCWRMI,0);

  ret = gswio(dev, cbp,G_SYNC_IO,NULL);
  if (ret)
  {
     PRNTE(("gopn_gsw: Failure in call to gswio().ret = 0x%x\n",ret));
     if (ret == EINTR)          /* loop until dma completed or long wait*/
	for (i = 0;
	     (g->dma_location != NO_DMA_IN_PROG) && (i < IoTmrVal*ScfTmrVal);
	     i++);
  }

  /*-------------------------------------------------------------------*/
  /* Free areas used for setmode/rmi and rmi, struct xmemdp.           */
  /*-------------------------------------------------------------------*/
  rc = gfree_spac((char **)&smrmip);
  if (rc)
     PRNTE(("gopn_gsw: Failure in freeing 'smrmip'\n"));
  return(ret);
} /* end of gopn_gsw */

/*------------G O P N _ X -----------------------------------------*/
/*
 * NAME:        gopn_x()
 *
 * PURPOSE:     do openx-specific open functions
 *
 * PARMS:       dev     minor device number
 *              ext     ptr to ext struct
 *
 * CALLING ROUTINE:
 *              gswopen
 *
 * RETURNS:     returns 0 if ok, non-zero if failure
 */
int gopn_x(dev,ext)
dev_t dev;
struct opnparms *ext;
{
  register GSWCB *g;
  int    bit = (BIT7 << MAX_INP);
  int    i;
  int    oldmask;
  int    type;
  int    err;                           /* bad return reason for mode   */
  q_hdr *hdr_ptr;
  IQT   *tablep;
  struct k_enable k_ena;

  PRINT(("gopn_x: dev = 0x%x, ext = 0x%x\n",dev,ext));
  g = mgcb_ptr + dev;

  switch (ext->devmode)
    {
      case SYS5080_MODE:
      case FPGI_MODE:
      case SYS_MODE:
	    g->devmode = ext->devmode;
	    break;
      default:
	    g->u_err_area = ext->u_err_area;
	    err = UnknownDevmode;
	    if (ext->u_err_area)
	       copyout((char *)&err,(char *)&(
		((struct log_info *)(ext->u_err_area))->err_reason),4);
	    return(EFBIG);             /* see user i/f document        */
    }

  hdr_ptr = g->hdr_ptr;
  bzero((char *)&k_ena,sizeof(struct k_enable));

   g->u_err_area = (char *)(ext->u_err_area);


  /*--------------------------------------------------------------------*/
  /* Specifying 'link_sw' on opnparms indicates that we should ENABLE   */
  /* the link switch interrupts for EVENT mode. The user wants to get   */
  /* these interrupts.                                                  */
  /*--------------------------------------------------------------------*/
  if (ext->link_sw)
  {
	type = (int)OPNX_INTRS;
	for (i = MAX_INP; i < ALL_MAX_INP ; bit <<= 1,i++)
	{
		if (type & bit)
		{
			if ( ((hdr_ptr+i)->intrp = (IQT *)
			 (gget_spac((int)(tbl_size[i]*sizeof(IQT)),2)))
			  == NULL )
				return(ENOMEM);
			G_DISABLE
			tablep = (hdr_ptr+i)->intrp;
			gfil_iqt(tablep,&k_ena,i);
			tablep->flags.mode = GEVENT;
			G_ENABLE
		}
	}
	g->oflag.link_sw = TRUE;
  }
  g->signal = ext->signal;

/******************** FOR FPGI ****************************************/
#ifdef FPGIMODETEST
  if(g->devmode == FPGI_MODE)
  {
	rc = fpgibufin(dev,ext->fpgiinp);
	if (rc) {                       /* rc is value for u_err_data   */
	    rc = EFBIG;                 /* see user i/f document        */
	    if (ext->u_err_area)
		copyout((char *)&rc,
		  &(((struct log_info *)(ext->u_err_area))->err_reason),
		  4);
	    return(rc);
	}
  }
#endif /* FPGIMODETEST */
/******************** FOR FPGI ****************************************/

  return(0);
} /* end gopn_x */


/*------------G S W I O ---------------------------------------------*/
/*
 * NAME:        gswio()
 *
 * PURPOSE:     perform S_XSIO and check return code
 *
 * PARMS:       dev     minor device number
 *              ccp     ccb address
 *              io_type type of io is sync(FALSE) or async(TRUE)
 *              from    type of routine from which it is called.
 *
 * RETURNS:     int     completion code: 0 if good, non-zero if bad
 *
 */

int gswio(dev, ccp,io_type,from)
register dev_t   dev;                   /* passed minor number          */
register RWCCB *ccp;
int io_type;
int from;
{
  register GSWCB *g;
  int rc = 0;
  int ret = 0;
  int oldmask;
  int i;
  int rc_sleep;
  int ccw_cnt;
  int parityrc;                         /* rc for setjmpx parity        */
  struct com_elm *cep;                  /* CE ptr for pin loop          */
  static int io_on_q[NumDevSupp];        /* set after setuo_sio issued   */

 PRINT(("gswio: dev=0x%x,ccp=0x%x,io_type=0x%x,from=0x%x\n",
	 dev,ccp,io_type,from));
  g = mgcb_ptr + dev;

  /*--------------------------------------------------------------------*/
  /* If io_allowed flag is FALSE, no i/o permitted, so exit.            */
  /*--------------------------------------------------------------------*/

  G_DISABLE
  if (!(g->iflag.io_allowed))
  {
	switch (g->uns_res)
	{
		case MSLA_NO_5080 :
			rc = ENODEV;
			break;
		case NOT_CONFIGURED :
			rc = ETXTBSY;
			break;
		case MSLA_LM :
			rc = EBUSY;
			break;
		default :
			rc = ECHILD;
			break;
	}
	G_ENABLE
	return(rc);
  }
  /*--------------------------------------------------------------------*/
  /* Check if another i/o operation is on the pending queue for this    */
  /* minor device. Other processes can be using the plotter at the      */
  /* same time and queue up i/o operations.                             */
  /* io_on_q is set to true if there is a queued up io operation for dev*/
  /*--------------------------------------------------------------------*/
  io_on_q[dev] = FALSE;
  for (i = 0; i < (NumDevSupp +1); i++) {
	    if ( (pndq[i] == dev) || (rmiq[i] == dev) ) {
		io_on_q[dev] = TRUE;
		break;
	    }
  }
  /**if (mf.sol_in_prog || mf.rmi_in_prog) **/  /* replaced 5/07/91     */
  while (                                       /* added    5/07/91     */
	  (io_on_q[dev] == TRUE) ||             /* added    5/4/91      */
	  (g->c.ae_exp) || (g->c.de_exp)  ||    /* added    5/4/91      */
	 g->oflag.io_waiting )                  /* added    5/4/91      */

  {
	g->oflag.slp_pending = TRUE;
 
 /* Changed - JCC */
 /*    rc_sleep = DO_SLEEP(&g->sleep_sio,EVENT_SIGRET); */
       rc_sleep = DO_SLEEP(&g->sleep_sio, LOCK_HANDLER | INTERRUPTIBLE);
 
 /* Changed - JCC */
 /*    if (rc_sleep != EVENT_SUCC) */     /* terminated - didnt get event */
       if (rc_sleep != THREAD_AWAKENED)     /* terminated - didnt get event */
		END_SLEEP

	g->oflag.wokeup      = FALSE;
	g->oflag.slp_pending = FALSE;

	/*--------------------------------------------------------------*/
	/* Check if another i/o operation is on the pending queue for   */
	/* this minor device.                                           */
	/*--------------------------------------------------------------*/
	io_on_q[dev] = FALSE;
	for (i = 0; i < (NumDevSupp +1); i++)  {
	    if ( (pndq[i] == dev) || (rmiq[i] == dev) )  {
		io_on_q[dev] = TRUE;
		break;
	    }
	}

  /*--------------------------------------------------------------------*/
  /* While the interrupt handler was waiting for the i/o operation      */
  /* to complete, it may have received notification to signal the       */
  /* appl (e.g., appl ENABLEd a particular set of unsolicited intrps).  */
  /* The intrpt handler did not want to wakeup this routine's sleep.    */
  /* So, the intrpt handler set 'sig_pending' flag so that this         */
  /* routine would issue the wakeup after it processed the i/o          */
  /* completion.                                                        */
  /*--------------------------------------------------------------------*/

	if (g->oflag.sig_pending)
	{
		SIGAPPL(g->signal);             /* signal appl          */
		g->oflag.sig_pending = FALSE;   /* turn off flag        */
	}

	if ( g->op_res != 0 )           /* last async io failed, so     */
	{                               /* dont bother issuing this one */
		rc = ENOTDIR;
		G_ENABLE
		return(rc);
	}

  }

  /*--------------------------------------------------------------------*/
  /* Reaching here implies that any outstanding i/o operations have     */
  /* completed and we are ready to issue the next i/o.                  */
  /* There are four  different types of i/o:                            */
  /*    . Write structured                                              */
  /*    . Read/Write                                                    */
  /*    . K_LCW from IOCTL                                              */
  /*    . FPGI                                                          */
  /* Each has a pair of  CCBs and each have a different variable        */
  /* to indicate which of the pair is currently in use.                 */
  /* The variables used are g->ws_io, g->rw_io, g->lc_io and g->fp_io.  */
  /* The 'from' variable indicates which routine called gswio, and      */
  /* hence which variable needs to be updated.                          */
  /* FOR REFERENCE:                                                     */
  /* This concept of three different variables grew as the design of    */
  /* dual i/o and multiple types of i/o florished. It was not in the    */
  /* original design, and is obviously an add-on.                       */
  /*--------------------------------------------------------------------*/
  G_ENABLE
  switch (from)
  {
	case WS_IO :
		g->ws_io = g->ws_io ? 0 : 1 ;
		break;
	case RW_IO :
		g->rw_io = g->rw_io ? 0 : 1 ;
		break;
	case LC_IO :
		g->lc_io = g->lc_io ? 0 : 1 ;
		break;
	case FP_IO :
		g->fp_io = g->fp_io ? 0 : 1 ;
		break;
	default :
		break;
  }

  /*--------------------------------------------------------------------*/
  /* Unpinu previous pinu'ed user buffers,                              */
  /* Perform the current i/o.                                           */
  /* If async i/o, this routine returns after drivr puts it successfully*/
  /* on its queue. The actual completion may occur later, at which      */
  /* time the intrpt handler would signal the AIX driver.               */
  /*                                                                    */
  /*                                                                    */
  /*    Pin all user buffers here because we cannot pin them in         */
  /*    the interrupt handler (if they are paged out, the intrpt        */
  /*    handler will abend).                                            */
  /*    Pinning is done by looping thru the CEs, checking 'res3' field  */
  /*    for DMA_USER. The unpinning will be done by the intrpt          */
  /*    handler after each completed ccw.                               */
  /*--------------------------------------------------------------------*/
  if (g->last_ccp)                      /* unpinu usr bfrs from last io */
  {
     rc = usrunpin((RWCCB **)&g->last_ccp);
     if (rc)
     {
        PRNTE(("gswio: FAILURE of usrunpin last ccp= 0x%x\n",g->last_ccp));
        return(rc);
     }
  }
  cep = ccp->ce;                                /* adr of ce[0] (ccws)  */
  ccw_cnt = cep->tranlen >> 3;                  /* ccw is 8bytes long   */

  if (g->xm.dp != 0)
  {
	rc = gfree_spac((char **)&g->xm.dp);
	if (rc)
		return(rc);
  }
  g->xm.cnt  = ccw_cnt;
  g->xm.indx = 1;
  g->xm.dp   = (struct xmem *)gget_spac((ccw_cnt+1)*sizeof(struct xmem),12);
  if (g->xm.dp == NULL)
     return(ENOMEM);

  /*--------------------------------------------------------------------*/
  /*    For ccws: if from USER, 'copyin' to kernel space to avoid       */
  /*    having to do 'io_att/io_det' for every access to ccw fields.    */
  /*--------------------------------------------------------------------*/
  if ( cep->res3 == DMA_USER )          /* ccws in user space           */
  {                                     /* copy                         */
	if (g->usrccwp)
	    rc = gfree_spac((char **)&g->usrccwp);

	g->usrccwp = (caddr_t)gget_spac(cep->tranlen,12);
       if (g->usrccwp == NULL)
	  return(ENOMEM);
	g->usrccwlen = cep->tranlen;
	copyin(cep->memaddr,            /*    from user                 */
	       g->usrccwp,              /*    to kernel                 */
	       g->usrccwlen);           /*    for this length.          */
	cep->memaddr = g->usrccwp;      /* ptr to ccws in kernel        */
	cep->res3 = DMA_KERNEL;         /* set mode                     */
  }
  /*--------------------------------------------------------------------*/
  /* For each data area in user space, call 'pin' and 'xmemat'.         */
  /* These xmem struct values will be used during 'xmemdt' in 'undo_dma'*/
  /*--------------------------------------------------------------------*/
  for (i = 0; i <= ccw_cnt; i++,cep++)
  {
	if ( cep->res3 == DMA_USER )
	{
						       /* pinu user bfr */
		ret = pinu(cep->memaddr, (int)cep->tranlen,
		          (short)UIO_USERSPACE);
		if (ret != 0)
		{
			PRNTE(("gswio: FAILURE of pin.cep = 0x%x\n",cep));
			/*----------------------------------------------*/
			/* Unpinu previous ones.                        */
			/*----------------------------------------------*/
			ccw_cnt = i;
			cep = ccp->ce;
			for (i = 0; i < ccw_cnt; i++,cep++)
			{
				rc = unpinu(cep->memaddr,
				  (int)cep->tranlen, (short)UIO_USERSPACE);
				if (rc != 0)
				  PRNTE(("gswio: FAIL unpinu. i=%d\n",i));
			}
			return(ret);
		}
		(g->xm.dp+i)->aspace_id = XMEM_INVAL;
		ret = xmattach(cep->memaddr, cep->tranlen,
/* Changed - JCC */
/*	 		       g->xm.dp+i,  SYS_ADSPACE); */
			       g->xm.dp+i,  USER_ADSPACE);
		if (ret != XMEM_SUCC)
		{
			PRNTE(("gswio: FAILURE of xmattach.i = 0x%x\n",i));
			/*----------------------------------------------*/
			/* Unpinu previous ones.                        */
			/*----------------------------------------------*/
			ccw_cnt = i;
			cep = ccp->ce;
			for (i = 0; i < ccw_cnt; i++,cep++)
			{
				rc = unpinu(cep->memaddr,
				    (int)cep->tranlen, (short)UIO_USERSPACE);
				if (rc != 0)
				  PRNTE(("gswio: FAIL unpinu. i=%d\n",i));
			}
			return(ret);
		}
	}
	else                                            /* not usr bfr  */
		(g->xm.dp+i)->aspace_id = XMEM_GLOBAL;
  }
  g->last_ccp = ccp;                                    /* save ptr     */

  SET_PARITYJMPRC(0,&(g->parbuf),ras_unique[1],gswio_name,PIOParityError,
		  NO_UNLOCK);
  if ( (rc = setup_sio(ccp,dev,g->xm.dp+1)) == 0 )
  {
	CLR_PARITYJMP(&(g->parbuf))
	
        G_DISABLE
	/*--------------------------------------------------------------*/
	/* Check if another i/o operation is on the pending queue for   */
	/* this minor device.                                           */
	/*--------------------------------------------------------------*/
	io_on_q[dev] = FALSE;
	for (i = 0; i < (NumDevSupp +1); i++) {
	    if ( (pndq[i] == dev) || (rmiq[i] == dev) ) {
		io_on_q[dev] = TRUE;
		break;
	    }
	}

	if ( (g->c.de_exp == TRUE) ||   /* check if io not yet done     */
	       (io_on_q[dev] == TRUE) ) /* or if on pndq                */
	{
	    g->io_pend_cnt++;
	    if (io_type == G_ASYNC_IO)  /* if async i/o, return without */
	    {                           /* waiting for real completion  */
		g->oflag.io_waiting = TRUE;
		G_ENABLE
		return(0);
	    }
	    g->oflag.slp_pending = TRUE;
 
/* Changed - JCC */
/*        rc_sleep = DO_SLEEP(&g->sleep_sio, EVENT_SIGRET); */
          rc_sleep = DO_SLEEP(&g->sleep_sio, LOCK_HANDLER | INTERRUPTIBLE);

/* Changed - JCC */
/*        if (rc_sleep != EVENT_SUCC) */ /* terminated - didnt get event */
          if (rc_sleep != THREAD_AWAKENED) /* terminated - didnt get event */
		END_SLEEP
	    g->oflag.wokeup      = FALSE;
	    g->oflag.slp_pending = FALSE;
        }

	if (g->oflag.sig_pending)               /* issue signal since   */
	{                                       /* it wasnt done in intr*/
		SIGAPPL(g->signal);             /* hdlr to avoid        */
		g->oflag.sig_pending = FALSE;   /* awakening sleep      */
	}

	G_ENABLE
	if (g->op_res) rc = EIO;
	return(rc);
  }
  else
  {
	CLR_PARITYJMP(&(g->parbuf))
	rc = EIO;
  }
  return(rc);
} /* end of gswio */


/*------------ G S W T I M E R    -----------------------------------*/
/*
 * NAME:        gswtimer()
 *
 * PURPOSE: handle timer interrupts
 *
 * PARMS:       arg     timer mask
 *
 * CALLING ROUTINE:
 *              entered when timer pops.
 *
 * RETURNS:    (void)
 */

void
gswtimer(arg)
ulong arg;
{
   int i;
   PRINT(("gswtimer: entered ; arg = 0x %x \n", arg));
   /*-------------------------------------------------------------------*/
   /* Timer can pop with multiple values set.                           */
   /*-------------------------------------------------------------------*/
   if ((uint)arg & ResetTmrMask)        /* reset timeout - very serious */
   {
	gerr_log((dev_t)0,NULL,timer_name,RSET_TIMEOUT,0,ras_unique[1]);
	ipl_msla();
	return;
   }
   if ((uint)arg & NrtrTmrMask)         /* NotReadyToReady timeout      */
   {
	for (i = 0; i < NumDevSupp; i++)
	    AWAKE_OPEN(i);
	return;
   }
   else                                 /* check for combinations       */
   {
    if ((uint)arg & IoTmrMask)
     gerr_log((dev_t)0,NULL,timer_name,IO_TIMEOUT,0,ras_unique[2]);
    if ((uint)arg & StopStrtTmrMask)
     gerr_log((dev_t)0,NULL,timer_name,STOPSTRT_TIMEOUT,0,ras_unique[3]);
    if ((uint)arg & ScfTmrMask)
     gerr_log((dev_t)0,NULL,timer_name,SCF_TIMEOUT,0,ras_unique[4]);
    if ((uint)arg & ProgRptTmrMask)
     gerr_log((dev_t)0,NULL,timer_name,PROGRPT_TIMEOUT,0,ras_unique[5]);
    cancel_sio(NumDevSupp);   /* cancel active sio's          */
    comm_lock     = 0;            /* free the communications area */
   }
   return;
}


/*------------ I N I T _ F R E E  -----------------------------------*/
/*
 * NAME:        init_free()
 *
 * PURPOSE: free malloc'ed space when error occurs in CFG_INIT (gswconfig)
 *
 * PARMS:       none
 *
 * CALLING ROUTINE:
 *              gswconfig(CFG_INIT)
 *
 * RETURNS:    void
 */
void
init_free()
{
    int rc,i;
   /*-------------------------------------------------------------------*/
   /* Free areas used for setmode/rmi and rmi, struct xmemdp, dds.      */
   /* If pointer is non-zero, free space.                               */
   /*-------------------------------------------------------------------*/
   for (i = 0; i < NumDevSupp; i++) {
       if ((mgcb_ptr+i)->rmip) {
	    rc = gfree_spac((char **)&((mgcb_ptr+i)->rmip));
	    if (rc)
		PRNTE(("init_free: FAILURE gfree_spac of rmip.\n"));
       }
   }
   if (mgcb_ptr) {
	rc = gfree_spac((char **)&mgcb_ptr);
	if (rc)
	    PRNTE(("init_free: FAILURE gfree_spac of mgcb_ptr.\n"));
   }
   if (intrstrucp) {
	rc = gfree_spac((char **)&intrstrucp);
	if (rc)
	    PRNTE(("init_free: FAILURE gfree_spac of intrstrucp.\n"));
   }
   if (ddsp) {
	rc = gfree_spac((char **)&ddsp);
	if (rc)
	    PRNTE(("init_free: FAILURE gfree_spac of ddsp.\n"));
   }
   if (ucp) {
	rc = xmfree(ucp,kernel_heap);
	ucp = (char *)0;
	if (rc)
	    PRNTE(("gswconfig: FAILURE xmfree of ucp.\n"));
   }
}

/*------------ L O A D P O S  ---------------------------------------*/
/*
 * NAME:        loadpos()
 *
 * PURPOSE: load the POS regs
 *
 * CALLING ROUTINE:
 *              gswconfig(CFG_INIT)
 *
 *
 * RETURNS:    (int)
 */

int
loadpos()
{
   volatile char *pptr;
   caddr_t iocc_addr;

   adapter_slot =  ddsp->sd.slot_number;        /* get  adapter slot    */

   iocc_addr = 
      IOCC_ATT((ulong)BUS_ID,(ulong)(POSREG(2,adapter_slot) + IO_IOCC));
   pptr = (char *)iocc_addr;

   /*-------------------------------------------------------------------*/
   /* First disable adapter, set up memory address and then             */
   /* enable adapter.                                                   */
   /*-------------------------------------------------------------------*/
   *pptr++ = ddsp->sd.intr_level << 4;        /* Ena intr+dis adptrPOS2 */
   *pptr++ = ddsp->sd.start_busio >> 8;         /* io adr          POS3 */
   *pptr++ = ddsp->sd.start_busmem >> 16;       /* mem adr         POS4 */
   *pptr++ = 0xC0 + (ddsp->sd.dma_level);       /* DMA reg         POS5 */
   pptr    = (char *)iocc_addr;                 /* Ena the adapter POS2 */
   *pptr   = (ddsp->sd.intr_level << 4) + Ena_Adapter + BusMasterDelay;

   IOCC_DET((ulong)iocc_addr);
   acc_regs((char *)(ddsp->sd.start_busio >> 8)); /* ptrs to adptr regs */

   return(0);
}

/*------------ L O A D U C O D E  -----------------------------------*/
/*
 * NAME:        loaducode()
 *
 * PURPOSE: load the ucode into memory then onto adapter
 *
 *
 * CALLING ROUTINE:
 *              gswconfig(CFG_INIT)
 *
 * RETURNS:    (int)
 */

int
loaducode()
{

   int i;
   uint   *source;                      /* local vars for load ucode    */
   uint   *target;                      /* local vars for load ucode    */
   ulong  segreg;                       /* local segment register       */

    segreg = (ulong)(BUSMEM_ATT((ulong)BUS_ID,(ulong)0x00));
    source = (uint *)(ucp);
    target = (uint *)(segreg + start_busmem);

    for (i = 0; i < (ddsp->sd.ucode_len >> 2); i++)
	*target++ = *source++;
    for ( ; i < (MAX_UCODE_SIZE >> 2); i++)     /* zero out rest of mem */
	*target++ = (uint)0;

    BUSMEM_DET(segreg);
    return(0);
}


/*------------ U S R U N P I N    -----------------------------------*/
/*
 * NAME:        usrunpin()
 *
 * PURPOSE: unpinu user buffers
 *
 * PARMS:       ccpp    ptr to ptr to RWCCB struct
 *
 * CALLING ROUTINE:
 *              gswio
 *              gcln_intr
 *
 * RETURNS:    (int)    0 successful
 *                     -1 failure
 */

int
usrunpin(ccpp)
RWCCB **ccpp;
{
  int i;
  int rc;
  int ccw_cnt;
  struct com_elm *cep;

  cep = (*ccpp)->ce;                            /* adr of ce[0] (ccws)  */
  ccw_cnt = cep->tranlen >> 3;                  /* ccw is 8bytes long   */

  *ccpp = (RWCCB *)0;                            /* zero out ptr        */
  /*--------------------------------------------------------------------*/
  /* For each data area in user space, call 'unpinu'.                   */
  /*--------------------------------------------------------------------*/
  for (i = 0; i <= ccw_cnt; i++,cep++)
  {
	if ( cep->res3 == DMA_USER )
	{
		rc = unpinu(cep->memaddr, (int)cep->tranlen,
				(short)UIO_USERSPACE);
		if (rc != 0)
		{
		    PRNTE(("usrunpin: FAILURE unpinu.cep=0x%x,rc=0x%x\n",
				cep,rc));
		    return(rc);
		}
	}
  }
  return(0);
}


