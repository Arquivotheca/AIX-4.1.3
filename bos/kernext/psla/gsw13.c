static char sccsid[] = "@(#)02  1.21  src/bos/kernext/psla/gsw13.c, sysxpsla, bos41J, 9519A_all 5/8/95 09:54:38";
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
/************************************************************************/
/*                                                                      */
/* PURPOSE:     Device driver for MSLA on R2                            */
/*                                                                      */
/*              There are four parts :                                  */
/*                  gsw13.c - main entry points                         */
/*                  gsw23.c - internal routines used by top             */
/*                  gsw33.c - internal routines used by bottom          */
/*                  gswintr.c - interrupt handler                       */
/*                                                                      */
/*                                                                      */
/* MAIN ENTRY POINTS: (gsw13.c)                                         */
/*                                                                      */
/*      gswconfig()    performs one time initialization                 */
/*                                                                      */
/*      gswopen()      opens device                                     */
/*                                                                      */
/*      gswclose()     closes device                                    */
/*                                                                      */
/*      gswread()      read data from 5080 buffer                       */
/*                                                                      */
/*      gswwrite()     write data to 5080 buffer                        */
/*                                                                      */
/*      gswioctl()     handles ioctl requests                           */
/*;                                                                     */
/*; notes:                                                              */
/*;                                                                     */
/*;     this version permits one overlapped io: if the user specifies   */
/*;     async_io on readx/writex/ioctl(k_lcw), a zero return code is    */
/*;     returned to the user after io operation is queued up.           */
/*;     when the sio completion is returned, the user is only notified  */
/*;     if the io failed ( notification is done through SIGIOINT(27) ). */
/*;     else, if a 2nd io is queued up, it is then started.             */
/*;                                                                     */
/*;      with 2 io's, ccws and ccbs are still malloced at init          */
/*;      time  rather than dynamically mallocing the space.             */
/*;      Therefore, less readable, but more efficient.                  */
/*                                                                      */
/*; bb 012490   Changed ending login in gswclose.                       */
/*; bb 013090   Added FATAL_ERROR case in gswopen.                      */
/*; bb 020290   Changed gswopen to do e_sleep instaed of e_sleepl.      */
/*; MJ 020290   Moved GET_LOCK in gswclose up.				*/
/*; bb 020790   Zeroed out oflags in gswclose if non-zero.              */
/*;             Removed dma_bus_addr assignment. take from dds.         */
/*; MJ 021590   Disabling of dma channel was added to gswconf CFG_TERM. */
/*;             Made the CFG_INIT and CFG_TERM cases in gswconf to      */
/*;             (de)configure for all the devices once they are entered.*/
/*;             UIOMOVE in gswconf is done once now.			*/
/*; MJ 021690   Made pointer to gsw_dds structure (ddsp) global rather  */
/*;             than having a separate ptr in each control block.       */
/*;             DDS is allocated once now.				*/
/*; MJ 021990   Removed the variable 'init_count' in gswconf.		*/
/*;             Made pointer to structure 'intr' global and removed it  */
/*;             devices control block.					*/
/*;		Allocated space for 'intr' structure separately.	*/
/*; MJ 022090   Removed the test for 'not_to_ready' flag in gswopen     */
/*;             when doing IPL. If for some reason the flag were TRUE,  */
/*;             then we would not have waited for IPL completion.	*/
/*; MJ 022190   Removed 'gchk_det' calls in gswopen in the two places   */
/*;		done for ports.						*/
/*; MJ 022190   att_cnt variable and att_done var. in gswcb structure   */
/*;	        removed, instead 'ipl_flag' variable was introduced.    */
/*; MJ 022190   K_STOP_ALL_DEVS Ioctl does the job of old 'gchk_det'    */
/*;		function with some modification regarding use 'ipl_flag'*/
/*;		instead of 'att_cnt' and 'att_done'; 'gchk_det' not     */
/*;		called anymore. Ioctl call detaches all devices.	*/
/*; MJ 022190   Global variable ipl_flag was set TRUE in gswconf INIT.  */
/*; MJ 022390   Replaced GET_LOCK macro in gswopen with 'lockl()' and   */
/*;		with LOCK_NDELAY flag;					*/
/*;             All other locks in other entry points are changed to    */
/*;		LOCK_SHORT which means they wait to get the lock.       */
/*;             In gswclose, 'lockl()' has no SIGRET flag, so it won't  */
/*;             interrupted.						*/
/*; bb 030190   Add #define IPLTEST to test ipl_start in gswconfig to   */
/*;             load ucode at config time.                              */
/*; bb 030290   Added frst_pass = TRUE at end of unconfig to get ready  */
/*;             for next config                                         */
/*;             Moved switch statement before test of frst_pass so that */
/*;             allocations only happen in INIT case.                   */
/*; bb 030290   Make ipl_start a default in gswconfig unless compiled   */
/*;             with NOIPLTEST                                          */
/*;             HALT_MSLA at start of CFG_TERM (unconfig time)          */
/*; bb 030590   Put back code to test for device open in CFG_TERM.      */
/*;             chris schwendamem says the driver is responsible to     */
/*;             reject the unconfig if device still open.               */
/*;             Adjust tests at start of routines for unconfigured or   */
/*;             already open (replaced ENXIO with ETXTBSY for uncofig). */
/*; bb 030590   Only use lockl for open,close and config. all else,     */
/*;             GET_LOCK and FREE_LOCK are null.                        */
/*;             Removed ext parm on close routine. no longer supported. */
/*;             (search for 'close' in my 'all notebook a')             */
/*; bb 030690   Remove all references to 'frst_pass', cleanup gswconfig.*/
/*; MJ 030990   Added 'lockl' call to QVPD case of gswconf.		*/
/*;             Added LOCK_NDELAY flag to gswopen 'lockl' call.		*/
/*;             Commented out check for invalid devno at begining of    */
/*;		gswopen.						*/
/*; MJ 030990   Corrected argument types in 'd_init', 'd_master',       */
/*		'd_complete', and 'i_disable' calls.			*/
/*; MJ 031290   Modified parameter types in IOCC_ATT IOCC_DET as_att    */
/*;             as_det and 'gfree_spac' macroes and system calls.	*/
/*; MJ 031290   Changed the parameter 'sfp' to 'arg' in the 'gio_ioctl' */
/*;             call (paramter type had to be 'union ctl_args').        */
/*;		'bcopy'ed 'sfp' pointer to the union before the call.   */
/*; MJ 031290   Removed 'size' parameter from 'gfree_spac' call.	*/
/*; MJ 031390   Removed check for dev invalid from all entries but      */
/*;		'gswconf', and 'gswopen'.		        	*/
/*;		Check for minor dev # less than zero was removed at the */
/*;		'gswconf' and 'gswopen'.                                */
/*; bb 031690   Ifdef'ed around K_STOP_DEVS in gswioctl because         */
/*;             stop_psla no longer used.                               */
/*; MJ 032090   Made the flag 'diag_mode' a global and removed it from  */
/*;		'gswcb, oflag' structures.				*/
/*;             Added check for 'diag_mode' flag before opening a device*/
/*;		to avoid interrupting a running diagnostic.		*/
/*;             Removed 'diag_mode=FALSE' from MSLA_STOP_DIAG ioctl.	*/
/*; bb 032090   Changed DOSLEEPL to DO_SLEEP and removed lock parm,     */
/*;             removed GET_LOCK/FREE_LOCK references,                  */
/*;             eliminate gswdefine.                                    */
/*; bb 040290   FOR HYDRA TESTING - removed the ifdef HYDRA code, which */
/*;             was no longer applicable.                               */
/*; bb 040990   Add 'parityrc' and 'parbufadr', increased # of          */
/*;             'res_unique' values to 40.                              */
/*;             Used space after dds for 'parbufadr'.                   */
/*; bb 041290   Using 'parbuf' in 'g' struct for setjmpx/clrjmpx.       */
/*              Moved the 'dma_enabled' code in CFG_TERM before the     */
/*              gfree_spac calls.                                       */
/*              NOTE: austin has supplied NO DOCUMENTATION to me        */
/*              regarding this 'sethmpx/clrjmpx' code. i have used the  */
/*              skyway and gemini device drivers as a basis for adding  */
/*              code to our psla driver. SINCE AUSTIN HAS NOT GIVEN ME  */
/*              THE REQUESTED DOCUMENTATION, I AM NOT 100% SURE THAT    */
/*              THIS CODE COVERS ALL SITUATIONS OR IS EVEN THE CORRECT  */
/*              WAY TO IMPLEMENT.                                       */
/*; bb 041990   Change two places in gswopen: one where DMA_FAIL, add   */
/*              G_ENABE and UN_LOCK before return(EIO) ; second, do     */
/*              UN_LOCK before END_SLEEP.                               */
/*;bb 052290    Fixed problem using UNLOCK in CFG_TERM after storage    */
/*              released. Maintain location of locks in 'g' for future  */
/*              extension to hydra.                                     */
/*              Also change return for unavailable lock to EBUSY - this */
/*              tells config that CFG_TERM did not clean up and exited  */
/*              early.                                                  */
/*;bb 053190    Check if ucode_len = 0. if so, we are in diag mode and  */
/*              do not call ipl_start in CFG_INIT.                      */
/*              Return EIO if d_complete fails in ioctl(MSLA_STOP_DMA)  */
/*              THIS WILL GO INTO UPDATE, NOT REL 1.                    */
/*;bb 102090    Getspace for intr_cntptr in gswopen after we decide     */
/*;             that we are in diagnostic mode, remove orig getspace    */
/*;             from ioctl(MSLA_START_DIAG) and freespace in            */
/*;             ioctl(MSLA_LOADUCODE) just after setting diag_mode ==   */
/*;             FALSE.                                                  */
/*;bb 050691    Reset io_pend_cnt to 0 at end of gswopen to match       */
/*;             other changes done in gsw23.c and gswintr.c             */
/*;bb 050891    Replaced dma_bus_id with bus_id, since d_init only      */
/*;             used the buid 8bits of the word value.                  */
/* fjp 050595 	Added ltunpin call to unpin psladd_lock		        */
/*;                                                                     */
/************************************************************************/

/* INCLUDE FILES   */
#include "gswincl.h"

/* DEFINES         */
#include "gswdefs.h"

/* EXTERN ROUTINES */
#include "gswextr.h"

/* DIAGNOSTIC INCLUDE FILE */
#include "gswdiag.h"

#include <sys/trcmacros.h>
#include <sys/trchkid.h>

 /*------------------------------------------------------------------*/
 /* LEGEND:                                                          */
 /*                                                                  */
 /* mgcb_ptr    -  ptr to 1st graphics control block                 */
 /* g           -  ptr to the graphics control block associated      */
 /*                with a particular 'dev'                           */
 /* this_qel    -  ptr a particular queue element                    */
 /* pa          -  structure containing pointers and lengths of      */
 /*                config-time malloced areas so that the space can  */
 /*                be freed at unconfig-time.                        */
 /* dma_chanid  -  dma channel id gotten from d_init call.           */
 /* dflt_size   -  array of input data received for each type of     */
 /*                unsolicted interrupt                              */
 /* tbl_size    -  array indicating # of entries in intrp table for  */
 /*                each type of interrupt (based on enum g_type)     */
 /* start_busmem - 28bit value (top nibble zero) containing the      */
 /*                start of adapter bus memory (e.g. adr of ucode).  */
 /* start_busio -  28bit value (top nibble zero) containing the      */
 /*                start of adapter bus io (e.g. adr of registers)   */
 /* wsf_typ     -  type od write structured field, using emun        */
 /*                wsf_type in gswdef.h                              */
 /* wsf_len     -  length of write structured field for wsf_typ.     */
 /* comm_lock   -  common lock used between routines that is set to  */
 /*                1 when an io is being processed and reset to 0    */
 /*                when no io processing is happening.               */
 /* adapter_slot - frequently used variable. same as value in dds.   */
 /* mf          -  msla flags that are set per adapter, not minor    */
 /*                number (like adapter ipl in progress)             */
 /* rmiq       -  queue of minor number that needs an rmi but was    */
 /*               unable to be done because an io was in progess     */
 /*               when the unsolicited interrupt was received by the */
 /*               interrupt handler                                  */
 /* pndq       -  queue of minor number that needs an i/o but was    */
 /*               unable to be done because an i/o was in progess    */
 /*               when the sio was requested by 'setup_sio'.         */
 /* bufstat    -  array of FPGI buffer status. TO BE MOVED TO 'G'    */
 /*               STRUCT.                                            */
 /* parityrc   -  return code from call to 'setjmpx', which is used  */
 /*               in case a Parity error occurs during PIO to the    */
 /*               adapter. (each local routine declares it)          */
 /*                                                                  */
 /*------------------------------------------------------------------*/
int  ipl_flag = TRUE;			/* flag indicating need for IPL */
int  diag_mode = FALSE;			/* diagnostic running indicator */
int  cursigmask  = 0;                   /* used after sleep wakeup      */
int  save_bus;
int  dma_chanid;                        /* dma channel id from d_init   */
int   dflt_size[ALL_MAX_INP] = { SEN_MAX,  /* GPICK   */
				 RMI_MAX,  /* GANK    */
				 RMI_MAX,  /* GPFK    */
				 RMI_MAX,  /* GLPFK   */
				 SEN_MAX,  /* GTABLET */
				 RMI_MAX,  /* GSMI    */
				 SEN_MAX,  /* GGEOP   */
				 SEN_MAX,  /* GPGM_ERR*/
				 0,        /* KNO_5080 */
				 0,        /* KLINK_SW */
				 0         /* KNOT_TO_READY */
			 };
				/* ------------------------------------ */
				/* number of entries in intrp table     */
				/* for each type of interrupt.          */
				/* ------------------------------------ */

int tbl_size[ALL_MAX_INP] = { 1,1,1,1,1,MAX_INTR,MAX_INTR,1,1,1,1};

ulong start_busmem;                     /* start of adapter busmem      */
ulong start_busio;                      /* start of adapter busio       */
ulong bus_id;                           /* bus id passed in dds         */
int intr_priority;                      /* intr_priority oassed in dds  */

short wsf_len[14] = { 0,6,6,6,10,10,10,6,10,10,10,12,8,12};
short wsf_typ[14] = { 0x0000, 0x710b, 0x7107, 0x7127, 0x7117,
			   0x7117, 0x710f, 0x711f, 0x711b, 0x712b,
			   0x713b, 0x7141, 0x7146, 0x7142};

ushort comm_lock = 0;                   /* communication area lock      */

char adapter_slot = -1;                 /* adapter slot in the wkstation*/
char *ucp;                              /* ucode area ptr               */
char *intr_name       = "GSWINTR";
char *setupsio_name   = "SETUP_S";
char *dodma_name      = "DO_DMA ";
char *undodma_name    = "UNDO_DM";
char *sendsol_name    = "SEND_SO";
char *senduns_name    = "SEND_UN";
char *stopstrt_name   = "STOPSTR";
char *stataccp_name   = "STATACC";
char *fpsetupsio_name = "FPSETUP";
char *fpsendsol_name  = "FPSEND_";
char *timer_name      = "GSWTIMER";
char *fndqel_name     = "QFND_QEL";
char *cfg_name        = "GSWCONFI";
char *open_name       = "GSWOPEN ";
char *ioctl_name      = "GSWIOCTL";
char *gswio_name      = "GSWIO   ";
char *names[16] = { "gsw0", "gswa", "gswb",
		    "gsw1", "gsw1a", "gsw1b",
		    "gsw2", "gsw2a", "gsw2b",
		    "gsw3", "gsw3a", "gsw3b",
		    "gsw4", "gsw4a", "gsw4b","xtra"};
struct msla_flags mf   = { 0,0,0,1,0,0,0,0,0,0,0,0 };
char ras_unique[40] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
			20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,
			36,37,38,39};
					/* used to number 'gerr_log' calls
					   in a DD source file.		*/
GSWCB *mgcb_ptr;
struct fpgi_send *fp_sendptr;
struct xmem *xmemdp;
struct msla_intr_count * intr_cntptr;   /* msla diag intr. counters ptr */
struct intr *intrstrucp;                /* structure intr pointer       */

char     ae_exp_lda = 0xff;             /* adapter end expected lda     */
char     ae_exp_cmd = 0xff;             /* adapter end expected cmd     */

ushort   rmiq[NumDevSupp+1];            /* rmi queue -                  */

ushort   rmiqhead  = 0;                 /* rmi queue head               */
ushort   rmiqtail  = 0;                 /* rmi queue tail               */

ushort   pndq[NumDevSupp+1];            /* cmd pend q -                 */

ushort   pndqhead = 0;                  /* cmd pend q head              */
ushort   pndqtail = 0;                  /* cmd pend q tail              */

ushort   bufstat[NumDevSupp+1];         /* device buffer status         */

struct ccw        *ccw_offset;          /* set in setup_io              */
struct com_elm    *ce_offset;           /* set in setup_io              */
struct ccb        *hdr_offset;          /* set in setup_io              */

struct ccw        *fpccw_offset;        /* set in setup_io              */
struct com_elm    *fpce_offset;         /* set in setup_io              */
struct ccb        *fphdr_offset;        /* set in setup_io              */
struct in_buf_hdr *free_buffer;         /* set in setup_io              */

struct mr_comm_area volatile *cap;      /* ptr to comm area-msla_to_rt  */
struct rm_comm_area volatile *rap;      /* ptr to comm area-rt_to_msla  */

struct pinned_areas pa[NumDevSupp];     /* record malloced areas to be  */
					/* freed at unconfig            */
struct gsw_dds *ddsp;			/* ptr to dds structure		*/

char *rst_rgp;                          /* reset reg adr on adapter     */
char *disi_rgp;                         /* disable intr reg adr on adptr*/
char *enai_rgp;                         /* enable  intr reg adr on adptr*/
char *int_rgp;                          /* intrpt reg adr on adapter    */
char *stop_rgp;                         /* halt  reg adr on adapter     */
char *strt_rgp;                         /* start reg adr on adapter     */
char *rsti_rgp;                         /* reset intr reg adr on adpt   */
char *stat_rgp;                         /* status     reg adr on adpt   */

/* Added - JCC */
Simple_lock psladd_lock;   /* For simple locks */

/*------------G S W C O N F I G -------------------------------------*/
/*
 * NAME:        gswconfig()  (AIX entry point)
 *
 * PURPOSE: one time configuration  for each minor device
 *          Called during AIX startup procedures to configure or
 *          delete the device driver for each device.
 *          The device switch table is initialized and added
 *          to the kernel.
 *
 *
 * PARMS:       devno   device number
 *              oc      operation code - INIT, TERM QVPD
 *              uiop    pointer to uio struct
 *
 * RETURNS:    (int)
 *             ENXIO     no such device
 *             EBUSY     mount device busy
 *             ENOMEM    not enough core
 *             ENOLCK    no locks available
 *             EINVAL    invalid argument
 *
 * SPECIAL NOTES:
 *      . global lock variable must be initialized to LOCK_AVAIL at
 *        compile time
 *      . global variable device_switch_table_added must be initialized
 *        to 0 at compile time
 *
 *
 */

int
gswconfig(devno,oc, uiop)
dev_t devno;
int oc;
struct uio *uiop;               /* dds ptr for INIT case                */
{
   register GSWCB *g;
   int    rc, i, j, k;
   int    ret_code = 0;
   int    dev;
   int    parityrc;                     /* retcode for setjmpx parity   */
   uint   count = 0;
   ulong  segreg;
   char   vpddat[VPD_LEN];
   char  *pos3p;
   char  *pos6p;
   struct devsw gsw_devsw;
   struct file *ucd_fp;                 /* ucode file ptr               */

/* Refer to gswdefs.h for debugging aids PRINT and PRNTE */
   PRINT(("gswconfig: devno = 0x%x, oc = %d, uiop = 0X%x\n",devno,oc,uiop));
   TRCHKL3T(HKWD_PSLA_CONF,devno,oc,(int)uiop);


   dev  = minor(devno);
   if (dev >= NumDevSupp)
   {
	PRNTE(("gswconfig: invalid minor number %d\n",dev));
        TRCHKL2T(hkwd_CONF_R8,ENXIO,dev); 
	return(ENXIO);
   }

   switch (oc)
   {
   case CFG_INIT:
   /*-------------------------------------------------------------------*/
   /* Allocate space for control blocks,                                */
   /* initialize static variables.                                      */
   /* Whenever premature exit due to failure, call 'init_free' to free  */
   /* allocated space.                                                  */
   /*                                                                   */
   /* Note that the configurator does not configure the devices until   */
   /*   this routine returns successfully. Therefore, if another        */
   /*   process attempts to open a psla device before this CFG_INIT     */
   /*   completes, it will fail on the first test in gswopen when       */
   /*   checking the minor device number.                               */
   /*   For this reason, we do not have to use lockl or check if        */
   /*   g->gsw_open is TRUE. Also, if the user attempts to config the   */
   /*   psla when it is already configured, the configurator will       */
   /*   reject the request and not call this CFG_INIT.                  */
   /*-------------------------------------------------------------------*/
     mgcb_ptr = (GSWCB *)gget_spac(NumDevSupp*sizeof(struct gswcb),12);
     if (mgcb_ptr == NULL ) {
	     TRCHKL1T(hkwd_CONF_R1,ENOMEM);
	     return(ENOMEM);
     }
     intrstrucp = (struct intr *)gget_spac(PAGESIZE,12);
     if (intrstrucp == NULL ) {
	     TRCHKL1T(hkwd_CONF_R1,ENOMEM);
	     init_free();                       /* cleanup routine      */
	     return(ENOMEM);
     }
     for (i = 0; i <= NumDevSupp; i++)          /* rmi and io pending   */
     {                                          /*   queues             */
	     rmiq[i]    = -1;
	     pndq[i]    = -1;
	     bufstat[i] = 0;
     }


     rc = pincode(gswintr);                     /* pin the intrpt hndlr */
     if (rc != 0)
     {
	     PRNTE(("gswconfig: FAILURE of call to pincode\n"));
	     TRCHKL1T(hkwd_CONF_R1,rc);
	     init_free();                       /* cleanup routine      */
	     return(rc);
     }
	PRINT(("rc from pincode(gswintr): %d\n", rc));

     /* Added - JCC */
     if (ltpin(&psladd_lock, sizeof(Simple_lock))) {
	     TRCHKL1T(hkwd_CONF_R1,ENOMEM);
	     init_free();
             return(ENOMEM);
     }
	PRINT(("psladd_lock ltpinned\n"));

     lock_alloc(&psladd_lock, LOCK_ALLOC_PIN, PSLA_LOCK_CLASS, -1);
     simple_lock_init(&psladd_lock);

     ddsp  = (struct gsw_dds *)gget_spac(PAGESIZE,12);
     for (k = 0; k < NumDevSupp; k++, devno++, dev++) {
	g = mgcb_ptr + k;               /* ptr to cb of specific device */

	g->devno = devno;
	g->name  = names[dev];
	g->lock  = LOCK_AVAIL;
	g->sleep_sio    = (int *)EVENT_NULL;
	g->sleep_eventq = (int *)EVENT_NULL;
	g->sleep_open   = (int *)EVENT_NULL;

	/*--------------------------------------------------------------*/
	/* Allocate space for GEOP/SMI/RMI in intrpt handler.           */
	/*--------------------------------------------------------------*/
	g->rmip  = gget_spac(5*PAGESIZE,12);    /* for RMI intrpts      */
	if (g->rmip == NULL)
	{
		init_free();                    /* cleanup routine      */
	        TRCHKL1T(hkwd_CONF_R1,ENOMEM);
		return(ENOMEM);
	}
	g->rmidp = (struct xmem *)   (g->rmip + PAGESIZE);
	g->intp  =                    g->rmip + 2*PAGESIZE;
	g->intdp = (struct xmem *)   (g->rmip + 3*PAGESIZE);
	g->zerop = (ushort *)        (g->rmip + 4*PAGESIZE);

	if (k == 0) {                   /* first time - get ucode       */
       	   rc = uiomove((caddr_t)ddsp,(int)sizeof(struct gsw_dds),
		     (enum uio_rw)UIO_WRITE,uiop);
	   if (rc != 0)
	   {
	      PRNTE(("gswconfig: FAILURE of uiomove.\n"));
              TRCHKL4T(hkwd_CONF_R3,rc,(int)ddsp,sizeof(struct gsw_dds),
                      (int)uiop); 
	      init_free();                      /* cleanup routine      */
	      return(rc);
	   }
        }

	rc = gfil_gcb(g);
        if (rc) {
	   PRNTE(("gswconfig: Failure of call to gfil_gcb.\n"));
           TRCHKL3T(hkwd_CONF_R4,rc,devno,(int)ddsp); 
	   init_free();                         /* cleanup routine      */
	   return(rc);
	}
	g->dma_bus_addr = (char *) ddsp->sd.dma_bus_addr;
	if (k == 0)                     /* first time - get ucode       */
	{
	    bus_id        = ddsp->sd.bus_id;
	    intr_priority = ddsp->sd.intr_priority;
	    /*----------------------------------------------------------*/
	    /* Initialize all entry points for driver in devsw table    */
	    /*----------------------------------------------------------*/
	    gsw_devsw.d_open     = gswopen;
	    gsw_devsw.d_close    = gswclose;
	    gsw_devsw.d_read     = gswread;
	    gsw_devsw.d_write    = gswwrite;
	    gsw_devsw.d_ioctl    = gswioctl;
	    gsw_devsw.d_strategy = nodev;
	    gsw_devsw.d_ttys     = NULL;
	    gsw_devsw.d_select   = nodev;
	    gsw_devsw.d_config   = gswconfig;
	    gsw_devsw.d_print    = nodev;
	    gsw_devsw.d_dump     = nodev;
	    gsw_devsw.d_mpx      = nodev;
	    gsw_devsw.d_revoke   = nodev;
	    gsw_devsw.d_dsdptr   = (caddr_t)ddsp;
	    gsw_devsw.d_selptr   = (caddr_t)NULL;
	    gsw_devsw.d_opts     = (long)NULL;

	    rc = devswadd(devno, &gsw_devsw);
	    if (rc) {
		PRNTE(("gswconfig: FAILURE of call to devswadd.\n"));
		TRCHKL1T(hkwd_CONF_R5,rc);
		init_free();                    /* cleanup routine      */
		return(rc);
	    }
	    /*----------------------------------------------------------*/
	    /* Setup interrupt handler values.                          */
	    /*----------------------------------------------------------*/
	    intrstrucp->next      = (struct intr *)NULL;
	    intrstrucp->handler   = gswintr;
	    intrstrucp->bus_type  = BUS_MICRO_CHANNEL;
	    intrstrucp->flags     = 0;
	    intrstrucp->level     = ddsp->sd.intr_level;
	    intrstrucp->priority  = ddsp->sd.intr_priority;
	    intrstrucp->bid       = ddsp->sd.bus_id;

	    rc = i_init(intrstrucp);
	    if (rc) {
		PRNTE(("gswconfig: FAILURE of i_init. rc = 0x%x\n",rc));
		devswdel(devno);
		TRCHKL0T(hkwd_CONF_R6);
		init_free();                    /* cleanup routine      */
		return(rc);
	    }
	    /*--------------- diag mode --------------------------------*/
	    if (ddsp->sd.ucode_len == 0)        /* indicates diag mode  */
	    {
		ucp = (char *) xmalloc((uint)4, /* dummy - for CFG_TERM */
				   (uint)3,kernel_heap);
		timeoutcf(MSLA_MAX_TIMEOUTS);
	    }
	    /*--------------- diag mode --------------------------------*/

	    else
	    {
		if (fp_getf(ddsp->sd.ucode_fd,&ucd_fp))
			return EBADF;
		ucp = (char *) xmalloc((uint)ddsp->sd.ucode_len,
				       (uint)3,kernel_heap);
		if (ucp == NULL)
		{
		    PRNTE(("gswconfig: FAILURE of xmalloc.ucode_len = %d \n",
			      ddsp->sd.ucode_len));
		    devswdel(devno);
		    i_clear(intrstrucp);
		    init_free();                /* cleanup routine      */
		    TRCHKL1T(hkwd_CONF_R7,ENOMEM);
		    /* Decrement the use count on the file descriptor */
		    ufdrele(ddsp->sd.ucode_fd);
		    return(ENOMEM);
		}
		rc = fp_read(ucd_fp,ucp,ddsp->sd.ucode_len,
			     0, UIO_SYSSPACE,&count);
		ufdrele(ddsp->sd.ucode_fd);
		if (rc) {
		    PRNTE(("gswconfig: Failure of fp_read\n"));
		    devswdel(devno);
		    i_clear(intrstrucp);
		    init_free();                /* cleanup routine      */
		    TRCHKL1T(hkwd_CONF_R1,rc);
		    return(rc);
		}
		timeoutcf(MSLA_MAX_TIMEOUTS);
	    }

	    rc = loadpos();
	    if (rc) {
		PRNTE(("gswconfig: FAILURE of call to loadpos.\n"));
		devswdel(devno);
		i_clear(intrstrucp);
		init_free();                    /* cleanup routine      */
		TRCHKL1T(hkwd_CONF_R9,rc);
		return(rc);
	    }
	    start_busmem = ddsp->sd.start_busmem;
	    start_busio  = ddsp->sd.start_busio;
	}
	g->buid  = BUS_ID;
      }
      if (ddsp->sd.ucode_len == 0)              /* indicates diag mode  */
	break;                                  /* so exit              */

      SET_PARITYJMPRC(0,&(mgcb_ptr->parbuf),ras_unique[1],cfg_name,
		      PIOParityError,NO_UNLOCK);
      ipl_start();
      CLR_PARITYJMP(&(mgcb_ptr->parbuf))
      ipl_flag = FALSE;
      break;

    case CFG_TERM:
     /*-----------------------------------------------------------------*/
     /* Check if any minor devices open or locked. if so, exit.         */
     /* It is the responsibility of this routine to check that a        */
     /* process does not have a psla device open. If so, we cannot      */
     /* unconfig psla. Returning a non-zero return code tells the       */
     /* configurator to fail the unconfig request.                      */
     /* If no processes are using any psla devices, we disable          */
     /* interrupts on the adapter and reset it.                         */
     /*-----------------------------------------------------------------*/
	PRINT(("entered CFG_TERM\n"));
     for (k = 0; k < NumDevSupp; k++) {
	g = mgcb_ptr + k;               /* ptr to cb of specific device */
	if (g->lock != LOCK_AVAIL) {
	   PRNTE(("gswconfig: (CFG_TERM) lock unavailable, dev %d \n",k));
	   TRCHKL1T(hkwd_CONF_R1,ENOLCK);
	   return(EBUSY);
	}
	if (g->oflag.gsw_open) {        /* check if device already open */
	   PRNTE(("gswconfig: (CFG_TERM) minor dev %d already open\n",k));
	   TRCHKL1T(hkwd_CONF_R1,EBUSY);
	   return(EBUSY);
	}
     }
	PRINT(("checked for open or locked minor devices\n"));

     SET_PARITYJMPRC(0,&(mgcb_ptr->parbuf),ras_unique[2],cfg_name,
		      PIOParityError,NO_UNLOCK);
	PRINT(("parity set\n"));
     DIS_INTR_MSLA                      /* disable interrupts           */
	PRINT(("interrupts disabled\n"));
     RST_MSLA                           /* reset the adapter            */
	PRINT(("adapter reset\n"));
     CLR_PARITYJMP(&(mgcb_ptr->parbuf))
	PRINT(("parity cleared\n"));
     for (k = 0; k < NumDevSupp; k++) {
	g = mgcb_ptr + k;               /* ptr to cb of specific device */
        if (lockl(&g->lock,LOCK_NDELAY) != LOCK_SUCC)  
        {                                               
	   PRNTE(("gswconfig: (CFG_TERM) lock not available\n"));
	   TRCHKL1T(hkwd_CONF_R1,ENOLCK);
	   return(EBUSY);
        }

	PRINT(("devices locked\n"));
	/*--------------------------------------------------------------*/
	/* Free areas used for setmode/rmi and rmi, struct xmemdp, dds. */
	/*--------------------------------------------------------------*/
	rc = gfree_spac((char **)&g->rmip);
	if (rc) { 
	    PRNTE(("gswconfig: FAILURE gfree_spac of g->rmip.\n"));
            ret_code = rc;
        }

	PRINT(("g->rmip gfree_spaced\n"));
	/*--------------------------------------------------------------*/
	/* If last one,                                                 */
	/*              stop adapter                                    */
	/*              remove entry pts from devsw table               */
	/*              unpin interrupt handler                         */
	/*--------------------------------------------------------------*/
	if (k == (NumDevSupp-1))
	{
		PRINT(("LAST ONE\n"));
                rc = gfree_spac((char **)&ddsp);
                if (rc) { 
		    PRNTE(("gswconfig: FAILURE gfree_spac of ddsp.\n"));
                    ret_code = rc;
                }
		PRINT(("ddsp gfree_spaced\n"));
		rc = xmfree(ucp,kernel_heap);
		if (rc) { 
		    PRNTE(("gswconfig: FAILURE xmfree of ucode.\n"));
                    ret_code = rc;
                }
		PRINT(("ucode xmfreed\n"));
		ucp = (char *)0;
                i_clear(intrstrucp);

		rc = devswdel(devno);
		if (rc) {
		    PRNTE(("gswconfig: FAILURE of call to devswdel.\n"));
                    ret_code = rc;
		}


		rc = unpincode(gswintr);
		if (rc)
		{
		   PRNTE(("gswconfig: FAILURE of call to unpincode.\n"));
                   ret_code = rc;
		}
		PRINT(("rc from unpincode(gswintr): %d\n", rc));

		/*------------------------------------------------------*/
		/* Free malloced areas from INIT (via define & gfil_gcb */
		/* These areas include:                                 */
		/*      - structs used in IOCTL calls for wsf           */
		/*      - ccws,ccbs,ces,smas used in 'g' struct         */
		/*      - g->q.top                                      */
		/*      - g->q.sen_hdrp                                 */
		/*      - g->hdr_ptr                                    */
		/*------------------------------------------------------*/
		for (i = 0; i < NumDevSupp; i++)
			for (j = 0; j < pa[i].cnt; j++) {
			    rc = gfree_spac((char **)&pa[i].ma[j].adr);
                            if (rc) {
		               PRNTE((
                                "gswconfig: FAILURE of xmfree \n"));
                               ret_code = rc;
                            }
			}
		PRINT(("INIT malloced areas freed\n"));
		/*------------------------------------------------------*/
		/* Cancel all timers.                                   */
		/*------------------------------------------------------*/
		CANCEL_TMR(IoTmrMask);
		CANCEL_TMR(StopStrtTmrMask);
		CANCEL_TMR(ProgRptTmrMask);
		CANCEL_TMR(NrtrTmrMask);
		timeoutcf(-(MSLA_MAX_TIMEOUTS));
		PRINT(("timers cancelled\n"));
		if (mf.dma_enabled) {
		    d_mask(dma_chanid);         /* disable DMA channel  */
		    d_clear(dma_chanid);        /* free    DMA channel  */
		    SET_PARITYJMPRC(0,&(mgcb_ptr->parbuf),ras_unique[3],
				    cfg_name,PIOParityError, DO_UNLOCK)
		    DIS_INTR_MSLA               /* disable interrupts   */
		    CLR_PARITYJMP(&(mgcb_ptr->parbuf))
		}
		PRINT(("DMA channel disabled and freed, interrupts disabled\n"));
		UN_LOCK
		PRINT(("UN_LOCK\n"));
		rc = gfree_spac((char **)&mgcb_ptr);
                if (rc)
		{
		    PRNTE(("gswconfig: FAILURE gfree_spac of mgcb_ptr\n"));
		    ret_code = rc;
		}
		PRINT(("mgcb_ptr gfree_spaced\n"));
                rc = gfree_spac((char **)&intrstrucp);
                if (rc) { 
		    PRNTE(("gswconfig: FAILURE gfree_spac of intrstrucp.\n"));
                    ret_code = rc;
                }
		PRINT(("intrstrucp gfree_spaced\n"));

		rc = ltunpin(&psladd_lock, sizeof(Simple_lock));
		if (rc) {
		    PRNTE(("gswconfig: FAILURE ltunpin of psladd_lock.\n"));
                    ret_code = rc;
     		}
		PRINT(("psladd_lock ltunpinned\n"));

		break;
	}
	UN_LOCK
	PRINT(("UN_LOCK\n"));
      }
	PRINT(("leaving CFG_TERM\n"));
      break;

    case CFG_QVPD:
	/*--------------------------------------------------------------*/
	/* Returns the Vital Product Data.                              */
	/* POS6 contains the adr of the VPD data when POS3 is READ.     */
	/*--------------------------------------------------------------*/
	/*--------------------------------------------------------------*/
	/* Check if any minor devices open or locked. if so, exit.      */
	/*--------------------------------------------------------------*/
	for (k = 0; k < NumDevSupp; k++) {
	    g = mgcb_ptr + k;           /* ptr to cb of specific device */
	    if (g->lock != LOCK_AVAIL) {
	      PRNTE(("gswconfig: (CFG_TERM) lock unavailable, dev %d \n",k));
	      TRCHKL1T(hkwd_CONF_R1,ENOLCK);
	      return(ENOLCK);
	    }
	    if (g->oflag.gsw_open) {    /* check if device already open */
	      PRNTE(("gswconfig: (CFG_TERM) minor dev %d already open\n",k));
	      TRCHKL1T(hkwd_CONF_R1,EBUSY);
	      return(EBUSY);
	    }
	}
	g = mgcb_ptr + dev;
        if (lockl(&g->lock,LOCK_NDELAY) != LOCK_SUCC)  
        {                                               
	   PRNTE(("gswconfig: (CFG_TERM) lock not available\n"));
	   TRCHKL1T(hkwd_CONF_R1,ENOLCK);
	   return(ENOLCK);                            
        }
	segreg = (ulong)IOCC_ATT((ulong)g->buid, (ulong)0);
	pos3p  = (char *)(segreg + POSREG(3,adapter_slot) + IO_IOCC);
	pos6p  = (char *)(segreg + POSREG(6,adapter_slot) + IO_IOCC);
	vpddat[0] = 0;                          /* 1st byte always zero */
	for (i = 1; i < VPD_LEN; i++)
	{
		*pos6p = i;                     /* POS6 has VPD data adr*/
		vpddat[i] = *pos3p;             /* read POS3 vpd data   */
	}
	IOCC_DET(segreg);
	/*--------------------------------------------------------------*/
	/* Write VPD data to user space.                                */
	/*--------------------------------------------------------------*/
	rc = uiomove(vpddat,VPD_LEN,((enum uio_rw)UIO_READ),uiop);
        if (rc) {
	   PRNTE(("gswconfig: Failure of uiomove in CFG_QVPD case\n"));
           ret_code = rc;
        }
        UN_LOCK;
	break;

    default:
	PRNTE(("gswconfig: entered default case \n"));
	break;
    }

    if (ret_code)
       TRCHKL1T(hkwd_CONF_R1,rc); 
    else
       TRCHKL0T(hkwd_CONF_R); 

PRINT(("ret_code from gswconfig: %d\n", ret_code));

    return(ret_code);
}

/*------------G S W O P E N -----------------------------------------*/
/*
 * NAME:        gswopen()    AIX entry point
 *
 * PURPOSE:     initialize device and attach it.
 *
 * PARMS:       devno   major and minor device number
 *              mode    defined in file.h
 *              channel defined in file.h
 *              ext     ptr to extension structure
 *
 * RETURNS:     int     see devsw.h for return code values
 *
 * ALGORITHM:
 *
 */

int gswopen(devno,mode,channel,ext)
dev_t   devno;
int     mode;
int     channel;
struct opnparms *ext;
{
  register GSWCB *g;
  int rc;
  int ret_code = 0;
  int oldmask;
  int dev;                              /* minor number                 */
  int parityrc;                         /* retcode for setjmpx parity   */
  int istatus;                          /* ipl status                   */
  ulong  dma_bus_id = IOCC_BID;

  TRCHKL4T(HKWD_PSLA_OPEN,devno,mode,channel,(int)ext);
  /*--------------------------------------------------------------------*/
  /* Validate device,                                                   */
  /* Lock gswcb structure                                               */
  /*--------------------------------------------------------------------*/
  dev = minor(devno);
  g   = mgcb_ptr + dev;

  if (lockl(&g->lock,LOCK_NDELAY+LOCK_SIGRET) != LOCK_SUCC)
  {                                                  
     PRNTE(("gswopen: lock not available\n"));
     TRCHKL2T(hkwd_OPEN_R1,ENOLCK,dev);
     return(ENOLCK);                               
  }
  PRINT(("gswopen: entered. dev = 0x%x,&g->iflag = 0x%x\n",dev,&g->iflag));
  if (dev >= NumDevSupp) {
                                         /*   if invalid minor number,  */      
	UN_LOCK                          /*    don't allow to open      */ 
	TRCHKL2T(hkwd_OPEN_R1,ETXTBSY,dev);
	return(ETXTBSY);
  }
  if (g->oflag.gsw_open ) {             /* if device already open,      */
	UN_LOCK                         /*   return                     */
        TRCHKL2T(hkwd_OPEN_R1,ENXIO,dev);
	return(ENXIO);
  }
  else
      g->oflag.gsw_open = TRUE;         /* set it here. reset it if err */

  if (diag_mode == TRUE) {		/* if diagnostic is running,    */ 
	UN_LOCK                         /* return.                      */
        g->oflag.gsw_open = FALSE; 
        TRCHKL2T(hkwd_OPEN_R1,ENXIO,dev);
	return(ENXIO);
  }
					/* always reset -               */
  g->sen_hdrp->head = 0;                /* zero out sense data index    */
  g->sen_hdrp->tail = 0;                /* zero out sense data index    */
  bzero(g->sen_datap,SENSE_MODULO * sizeof(union u_sen_data));
  g->devmode = SYS5080_MODE;            /* initialize to 5080 mode      */


  /*--------------------------------------------------------------------*/
  /* For diagnostics, do an early return.                               */
  /*--------------------------------------------------------------------*/
  if ( (ext) && (ext->diag_mode) ) {    /* openx used .  diagostic mode */
     G_DISABLE
     if (dev == 0)			/* only gsw0 gets into diag mode*/
     {
	     intr_cntptr = (struct msla_intr_count *)gget_spac(
		   sizeof(struct msla_intr_count),2);
	     if (intr_cntptr == NULL)
	     {
		 TRCHKL2T(hkwd_OPEN_R,ENOMEM,dev);
		 return(ENOMEM);
	     }
	     diag_mode = TRUE;
     }
     if (mf.dma_enabled == FALSE)
     {       
					/* initialize DMA               */
            rc = d_init((int)ddsp->sd.dma_level,
	                MICRO_CHANNEL_DMA,
			/**dma_bus_id); replaced 6/8/91 **/
			bus_id);
            if (rc == DMA_FAIL)
            {   
	      PRNTE(("gswopen: FAILURE of dma enable. RETURNING NOW\n"));
              TRCHKL1T(hkwd_OPEN_R2,EIO);
	      g->oflag.gsw_open = FALSE;
	      G_ENABLE
	      UN_LOCK
              return(EIO); 
            }  
            dma_chanid = rc;             /* save channel id              */
            mf.dma_enabled = TRUE;
            d_unmask(dma_chanid);        /* enable the DMA channel       */
     }
     G_ENABLE
     UN_LOCK
     TRCHKL0T(hkwd_OPEN_R);
     return(0);
  }

  /*--------------------------------------------------------------------*/
  /*     If this is the first attach for any gsw device                 */
  /*     call 'ipl_start' which                                         */
  /*     calls 'ipl_msla'.                                              */
  /*     If not yet received NRTR, set timer and sleep.                 */
  /*--------------------------------------------------------------------*/
  if (ipl_flag == TRUE)                 /* this is the 1st attach ever! */
  {
       PRNTE(("gswopen: ipl_flag code section\n"));
       SET_PARITYJMPRC(0,&(g->parbuf),ras_unique[1],
		       open_name,PIOParityError, DO_UNLOCK)
       ipl_start();
       CLR_PARITYJMP(&(g->parbuf))
       G_DISABLE
       SET_TMR(NrtrTmrVal,0,NrtrTmrMask);
       g->oflag.slp_pending = TRUE;
       /*---------------------------------------------------------------*/
       /* hold on to lock so another process cannot also open device    */
       /*---------------------------------------------------------------*/

/* Changed - JCC */
/*        rc = DO_SLEEP(&g->sleep_open,EVENT_SIGRET); */
       rc = DO_SLEEP(&g->sleep_open, LOCK_HANDLER | INTERRUPTIBLE);

/* Changed - JCC */
/*        if (rc != EVENT_SUCC) */    /* terminated - didnt get event */
       if (rc != THREAD_AWAKENED)    /* terminated - didnt get event */
       {
	  CANCEL_TMR(NrtrTmrMask);
          g->oflag.wokeup = FALSE;
          g->oflag.slp_pending = FALSE;
	  g->oflag.gsw_open = FALSE;
	  UN_LOCK
          END_SLEEP
       }
       CANCEL_TMR(NrtrTmrMask);
       g->oflag.wokeup = FALSE;
       g->oflag.slp_pending = FALSE;
       ipl_flag = FALSE;
       G_ENABLE
  }

  G_DISABLE

  /*--------------------------------------------------------------------*/
  /* The flags in 'iflag' can be independently set. The following code  */
  /* establishes the priority of these flags.                           */
  /*--------------------------------------------------------------------*/
  if ( !(g->iflag.io_allowed) )
  {
		istatus = IO_NOT_ALLOWED;
		switch (g->uns_res)
		{
			case FATAL_ERROR :
			case MSLA_NO_5080 :
				ret_code = ENODEV;
				break;
			case NOT_CONFIGURED :
				ret_code = ETXTBSY;
				break;
			case MSLA_LM :
			default :
				ret_code = EBUSY;
				break;
		}
  }
  else if (g->iflag.not_configured)
  {
		PRNTE(("gswopen: entered 'not_configured' else code\n"));
		istatus = GSW_NOT_CONFIGURED;
		ret_code = ETXTBSY;
  }
  else if (g->iflag.gsw_switched)
  {
		PRNTE(("gswopen: entered 'gsw_switched' else code \n"));
		istatus = SWITCHED_TO_HOST;
		ret_code = EBUSY;
  }
  else if ( !(g->iflag.not_to_ready) )
  {
		PRNTE(("gswopen: entered 'not_to_ready' else code \n"));
		istatus = NOT_YET_READY;
  }
  else
		istatus = GSW_OK;

  G_ENABLE
  switch (istatus)
  {
	case IO_NOT_ALLOWED :
	case GSW_NOT_CONFIGURED :
	case SWITCHED_TO_HOST :
		/*------------------------------------------------------*/
		/* check detach if a port.                              */
		/*------------------------------------------------------*/
		UN_LOCK
                TRCHKL3T(hkwd_OPEN_R3,ENOTTY,dev,istatus);
		g->oflag.gsw_open = FALSE;
		return(ret_code);

	case NOT_YET_READY :
		if  (!(g->iflag.not_to_ready) ) /* still not ready. exit */
		{
			PRNTE(("gswopen: 'not_to_ready' case code \n"));
			ret_code = ENODEV;
			UN_LOCK
                        TRCHKL3T(hkwd_OPEN_R3,ENODEV,dev,istatus);
			g->oflag.gsw_open = FALSE;
			return(ret_code);
		}
		break;

	case GSW_OK :
		break;
	default :
		ret_code = EIO;
		UN_LOCK
                TRCHKL2T(hkwd_OPEN_R1,ret_code,dev);
		g->oflag.gsw_open = FALSE;
		return(ret_code);
  }

  /*--------------------------------------------------------------------*/
  /* Device is now ready...                                             */
  /* call gopn_gsw to do set mode and rmi io.                           */
  /*--------------------------------------------------------------------*/
  g->io_pend_cnt = 0;
  if ( (rc = gopn_gsw(dev)) != 0) {
	*((short *)&g->oflag) = 0;
	/*--------------------------------------------------------------*/
	/* check if gopn_gsw was interrupted (e.g. a BREAK caused EINTR)*/
	/*--------------------------------------------------------------*/
	ret_code = EIO;
	UN_LOCK
        TRCHKL2T(hkwd_OPEN_R1,ret_code,dev);
	g->oflag.gsw_open = FALSE;
	return(ret_code);
  }
  g->signal = 0;

  /*--------------------------------------------------------------------*/
  /* Allocate space for Base Write Structure Fields , stop and start    */
  /* (used for stopping and starting display list buffer before/after   */
  /* SRMA/RMA and SWMA/WMA).                                            */
  /*--------------------------------------------------------------------*/
  g->basesfp = (struct basic_ws *)gget_spac(PAGESIZE,12);/*stop/start sf*/
  if (g->basesfp == NULL)
  {
	UN_LOCK
	g->oflag.gsw_open = FALSE;
	return(ENOMEM);
  }
  g->basesfp->b.sflen     = 6;
  g->basesfp->b.t_c       = STOP_TC;            /* type and code        */
  g->basesfp->b.rsvd      = 0;
  (g->basesfp+1)->b.sflen = 6;
  (g->basesfp+1)->b.t_c   = START_TC;           /* type and code        */
  (g->basesfp+1)->b.rsvd  = 0;

  /*--------------------------------------------------------------------*/
  /* if openx used, call gopn_x                                         */
  /*--------------------------------------------------------------------*/
  if ( (ext) && ((ret_code = gopn_x(dev,ext))!= 0) ) {
	UN_LOCK
        TRCHKL2T(hkwd_OPEN_R1,ret_code,dev);
	g->oflag.gsw_open = FALSE;
	return(ret_code);
  }

  g->io_pend_cnt    = 0;
  g->devno          = devno;
  g->mode           = mode;
  g->open_channel   = channel;
  g->pid            = getpid();                 /* pid for later use    */
  UN_LOCK
  TRCHKL0T(hkwd_OPEN_R);
  return(0);
}

/*------------G S W C L O S E ---------------------------------------*/
/*
 * NAME:        gswclose()
 *
 * PURPOSE:     clean up pointers and make device available
 *
 *
 * PARMS:       devno   major and minor device number
 *              mode    defined in file.h
 *              channel defined in file.h
 *
 * RETURNS:     int     see devsw.h for return code values
 *
 * ALGORITHM:
 *              reset the open flag
 *              free input device element queues
 *              zero out entries in device queue ptr structure
 */

int gswclose(devno,mode,channel)
dev_t   devno;
int     mode;
int     channel;
{
  register GSWCB *g;
  int i;
  int dev;                              /* minor number                 */
  int rc  = 0;
  int err = FALSE;

  TRCHKL4T(HKWD_PSLA_CLOSE,devno,mode,channel,0);

  PRINT(("gswclose: entered. devno = 0x%x \n", devno));
  /*--------------------------------------------------------------------*/
  /* Validate device,                                                   */
  /* Lock gswcb structure                                               */
  /*--------------------------------------------------------------------*/
  dev = minor(devno);
  g   = mgcb_ptr + dev;

  if (lockl(&g->lock,LOCK_SIGRET) != LOCK_SUCC)
  {                                                  
     PRNTE(("gswclose: lock not available\n"));
     return(ENOLCK);                               
  }

					/*------------------------------*/
					/* when switching done while opn*/
					/*------------------------------*/
  if ( (!(g->iflag.gsw_switched)) && (g->iflag.not_to_ready) )
	g->iflag.io_allowed = TRUE;
					/*------------------------------*/
					/* cleanup ptrs and intrpts     */
					/*------------------------------*/
  rc = gcln_intr(dev);
  if (rc == EINTR)                      /* interrupted. do again.       */
	rc = gcln_intr(dev);
  if (rc && (rc != EINTR)) err = TRUE;  /*------------------------------*/
					/* free up malloced lcw space   */
					/*------------------------------*/
  for (i = 0; i < MAX_MULTI_IO ; i++)
  {
	if (g->lcwccb_adr[i])
	{
		rc = gfree_spac((char **)&g->lcwccb_adr[i]);
		g->lcwccb_adr[i] = 0;
	}
  }


#ifdef FPGICOMPILE
  if (g->devmode == FPGI_MODE)
  {
	rc = fpgiclose(dev);
	if (rc) err = TRUE;
  }
#endif /* FPGICOMPILE */


  if ( *((short *)(&g->oflag)) )        /* ensure no open flags set     */
  {
	err = TRUE;
	*((short *)&g->oflag) = 0;
  }
  UN_LOCK

  if (err)                              /* error above. notify caller.  */
  {
     TRCHKL2T(hkwd_CLOSE_R1,EIO,dev);
     return(EIO);
  }
  else
  {
     TRCHKL0T(hkwd_CLOSE_R);
     return(0);
  }
}

/*------------G S W R E A D -----------------------------------------*/
/*
 * NAME:        gswread()
 *
 * PURPOSE:     Entry to device driver for read requests.
 *
 * PARMS:       devno   major and minor device number
 *              uiop    ptr to uio struct
 *              channel channel number
 *              ext     ptr to extension structure
 *
 *
 * RETURNS:     int     completion code (see gswcb.h)
 *
 * ALGORITHM:
 *
 *      Put uiop->uio_iov->iov_base into 3rd ce memaddr field
 *      Put uiop->iov_offset into memofs of sma structure
 *      Put uiop->uio_resid into 3rd ce tranlen field
 *      call gfil_ccb, gswio
 *      update uiop fields
 *      return 0
 */

int gswread( devno,uiop,channel,ext)
dev_t   devno;
struct uio *uiop;
int     channel;
union rw_args ext;
{

  register int    cnt;
  register GSWCB *g;
  int dev;                                      /* minor nubmer         */
  int io_type = 0;                              /* syncio or asyncio    */
  int rc = 0;

  TRCHKL4T(HKWD_PSLA_READ,devno,(int)uiop,channel,&ext);
  PRINT(("gswread: entered.devno=0x%x,uiop=0x%x,ext=0x%x\n",devno,uiop,ext));
  /*--------------------------------------------------------------------*/
  /* Validate device,                                                   */
  /* Lock gswcb structure                                               */
  /*--------------------------------------------------------------------*/
  dev = minor(devno);
  g   = mgcb_ptr + dev;

  /*--------------------------------------------------------------------*/
  /* Update ccb and set uiop->iov_offset and uiop->iov_resid  if good   */
  /*--------------------------------------------------------------------*/
  cnt = uiop->uio_iov->iov_len;

  if(g->devmode == FPGI_MODE)           /* no read allowed in FPGI mode */
  {
        TRCHKL3T(hkwd_READ_R2,EISDIR,dev,g->devmode);
	return(EISDIR);
  }
  if ( (cnt & RD_EXCESS) || ( cnt == 0) )
  {
        TRCHKL3T(hkwd_READ_R3,EINVAL,dev,cnt);
	return(EINVAL);
  }
  /*--------------------------------------------------------------------*/
  /* Set up seek address from  readx parameter.                         */
  /* The 'offset' field has to be updated with the new 5080 dlb address.*/
  /*--------------------------------------------------------------------*/
  if ( (ext.value) && ((int)(ext.valuep->adr.dlb_adr) !=  -1) )
	uiop->uio_offset = ext.valuep->adr.dlb_adr;
						/*----------------------*/
						/* is it async io ?     */
						/*----------------------*/
  if (ext.value)
	io_type = ext.valuep->async_io;
						/*----------------------*/
						/* fill ccb and do io   */
						/*----------------------*/
  if ((rc = gfil_ccb(dev,uiop,ext,G_READ)) != 0)
  {
        TRCHKL2T(hkwd_READ_R1,rc,dev);
	return(rc);
  }
  if ((rc = gswio(dev,g->rw_ccbp + g->rw_io,io_type,RW_IO)) != 0)
  {
	return(rc);
  }
  uiop->uio_offset += cnt;                      /* for next seek        */
  uiop->uio_resid -= cnt;                       /* residual count       */
  TRCHKL0T(hkwd_READ_R);
  return (0) ;

}

/*------------G S W W R I T E  --------------------------------------*/
/*
 * NAME:        gswwrite()
 *
 * PURPOSE:     Entry to device driver for write requests.
 *
 * PARMS:       devno   major and minor device number
 *              uiop    ptr to uio struct
 *              channel channel number
 *              ext     ptr to extension structure
 *
 *
 * RETURNS:     int     completion code (see gswcb.h)
 *
 * ALGORITHM:
 *
 *      Put uiop->uio_iov->iov_base into 3rd ce memaddr field
 *      Put uiop->iov_offset into memofs of sma structure
 *      Put uiop->uio_resid into 3rd ce tranlen field
 *      call gfil_ccb, gswio
 *      update uiop fields
 *      return 0
 *
 */

int gswwrite( devno,uiop,channel,ext)
dev_t   devno;
struct uio *uiop;
int     channel;
union rw_args ext;
{

  register int    cnt;
  register GSWCB *g;
  int dev;                                      /* minor nubmer         */
  int io_type = 0;                              /* syncio or asyncio    */
  int rc = 0;

  TRCHKL4T(HKWD_PSLA_WRITE,devno,(int)uiop,channel,&ext);
  PRINT(("gswwrite: entered. devno = 0x%x \n", devno));
  PRINT(("uiop = 0x%x,channel=0x%x,ext=0x%x \n", uiop,channel,ext));
  /*--------------------------------------------------------------------*/
  /* Validate device, lock code.                                        */
  /*--------------------------------------------------------------------*/
  dev = minor(devno);
  g   = mgcb_ptr + dev;

  /*--------------------------------------------------------------------*/
  /* update ccb and set uiop->iov_offset and uiop->iov_resid  if good   */
  /*--------------------------------------------------------------------*/
  cnt = uiop->uio_iov->iov_len;

  if(g->devmode == FPGI_MODE)           /* no read allowed in FPGI mode */
  {
        TRCHKL3T(hkwd_WRITE_R2,EISDIR,dev,g->devmode);
	return(EISDIR);
  }
  if ( (cnt & WT_EXCESS) || ( cnt == 0) )
  {
        TRCHKL3T(hkwd_WRITE_R3,EINVAL,dev,cnt);
        return(EINVAL);
  }

  /*--------------------------------------------------------------------*/
  /* Set up seek address from  readx parameter.                         */
  /* The 'offset' field has to be updated with the new 5080 dlb address.*/
  /*--------------------------------------------------------------------*/
  if ( (ext.value) && ((int)(ext.valuep->adr.dlb_adr) !=  -1) )
	uiop->uio_offset = ext.valuep->adr.dlb_adr;
						/*----------------------*/
						/* is it async io ?     */
						/*----------------------*/
  if (ext.value)
	io_type = ext.valuep->async_io;
						/*----------------------*/
						/* fill ccb and do io   */
						/*----------------------*/
  if ((rc = gfil_ccb(dev,uiop,ext,G_WRITE)) != 0)
  {
        TRCHKL2T(hkwd_WRITE_R1,rc,dev);
	return(rc);
  }
  if ((rc = gswio(dev,g->rw_ccbp + g->rw_io,io_type,RW_IO)) != 0)
  {
	return(rc);
  }
  uiop->uio_offset += cnt;                      /* for next seek        */
  uiop->uio_resid -= cnt;                       /* residual count       */
  TRCHKL0T(hkwd_WRITE_R);
  return (0) ;

}

/*------------G S W I O C T L ---------------------------------------*/
/*
 * NAME:        gswioctl()
 *
 * PURPOSE:     Handle ioctl functions
 *
 * PARMS:       devno   major and minor device number
 *              cmd     selects operation to be performed
 *              arg     argument passed by caller
 *              flag    file status bits as defined in fcntl.h
 *              channel currently ignored
 *              ext     currently ignored
 *
 * RETURNS:     (int)
 *              EFAULT  bad address
 *              EINVAL  invalid argument
 *              ENXIO   no such device
 *              EIO     i/o error
 *              0       successful
 *
 * ALGORITHM:
 *              use switch to branch to correct case
 */

int  gswioctl( devno, cmd, arg, flag, channel, ext)
int channel,ext,cmd,flag;
register dev_t devno;
union ctl_args arg;
{
  register GSWCB *g;
  register IQT   *tablep;
  register q_qel *this_qel;
  register q_hdr *hdr_ptr;

  char bit   = BIT7;
  char found = FALSE;

  int i;
  int rc;
  int dev;                                      /* minor number         */
  int parityrc;                                 /* rc for setjmpx parity*/
  int data_len;
  int ccw_cnt;
  int ret     = 0;
  int oldmask = 0;
  int wslen   = 0;
  ulong segreg;                                 /* local seg reg value  */

  char *chp;
  char *p;                                      /* for malloced pointer */

  q_qel tmp_qel;

  union ctlparms   val;
  struct devinfo   devinfo;
  enum wsf_type    ws_typ = unused;

  union ctlparms  *parmsp  = &val;      /* ptr to local data area       */
  struct basic_ws *sfp;                 /* ptr to struct field area     */
  struct sflpat   *sflp;                /* ptr to struct field area     */

  /*--------------------------------------------------------------------*/
  /* diagnostic variables                                               */
  /*--------------------------------------------------------------------*/
  ulong  ioaddr;                        /* return from as_att           */
  ulong   adr_for_tcw;              /* 'tcw adr.' ptr sent to diag. test*/
  static caddr_t io_addr;               /* used in DMA_START.           */
					/* msla mem/io start addr. str. */
  static struct msla_map * msla_mapptr = (struct msla_map *)0;
					/* diag. dma test data str ptr  */
  static struct dma_test_parms * dma_tst_parmp;
  static char * dma_tdataptr;
  volatile caddr_t iocc_addr;
  volatile char * pptr;                         /* ptr to POS regs      */

/**********************FPGI POINTERS *****************/
#ifdef FPGICOMPILE
  struct com_rwfp        *ccbptr;
  struct ccw2            *ccwptr;
  struct sma             *smaptr;
  struct conn_req        *ptr;
  struct fpgi_connection *arg1;
#endif /* FPGICOMPILE */
/**********************FPGI POINTERS *****************/

  TRCHKL4T(HKWD_PSLA_IOCTL,devno,cmd,&arg,flag);
  PRINT(("gswioctl: devno=0x%x,cmd=0x%x,arg=0x%x,flag=0x%x,ext=0x%x \n",
		 devno,cmd,arg,flag,ext));
  /*--------------------------------------------------------------------*/
  /* Validate device, lock code.                                        */
  /*--------------------------------------------------------------------*/
  dev = minor(devno);
  g   = mgcb_ptr + dev;

  sfp  = g->sfp;
  sflp = g->sflp;
  bzero((char *)sfp, sizeof(struct basic_ws));
  bzero((char *)sflp, sizeof(struct sflpat));
  hdr_ptr = g->hdr_ptr;

  /*--------------------------------------------------------------------*/
  /* Cases for ioctl                                                    */
  /*--------------------------------------------------------------------*/

  switch (cmd)
  {

  case  IOCINFO :                   /*-return dev info---------------*/

    devinfo.devtype = DD_GSW;
    devinfo.flags   = 0;
    copyout((char *)&devinfo, (char *)arg.valuep, sizeof(struct devinfo));
    TRCHKL0T(hkwd_IOCTL_R);
    return(0);


#ifdef FPGICOMPILE
  case  FPGI_CONNECT      :
    /*------------------------------------------------------------------*/
    /* Malloc a large area and carve up.                                */
    /*------------------------------------------------------------------*/
    malloc_size =
		sizeof(struct com_rwfp)  +      /* ccb and ce's         */
		sizeof(struct ccw2)      +      /* SWMA/WMA ccw's       */
		sizeof(struct sma)       +      /* data                 */
		sizeof(struct conn_req);        /* connection request   */
    p = (char *)gget_spac(malloc_size, 3);
    if (p == NULL)
    {
       return(ENOMEM);
    }
    ccbptr  = (struct com_rwfp *)p;
    ccwptr  = (struct ccw2 *)    (ccbptr+1);
    smaptr  = (struct sma *)     (ccwptr+1);
    ptr     = (struct conn_req *)(smaptr+1);

    /*------------------------------------------------------------------*/
    /* All io operation info is moved to kernel space.                  */
    /*------------------------------------------------------------------*/
    arg1 = (struct fpgi_connection *)&(arg.valuep->f_connect);
    ptr->len      = sizeof(struct conn_req);
    ptr->cmd      = CONN_CMD;
    ptr->func     = FALSE;
    ptr->req_size = arg1->req_send_size;
    ptr->max_size = arg1->max_recv_size;
    ptr->target   = arg1->target_name;

    FSMA(*smaptr,FSMAMEMID,0,10);
    FCCB(*ccbptr);
    FCEX(*ccbptr,0,LINK1,sizeof(struct ccw2),ccwptr,DMA_KERNEL);
    FCEX(*ccbptr,1,LINK1,10,smaptr,DMA_KERNEL);
    FCEX(*ccbptr,2,0,sizeof(struct conn_req),ptr,DMA_KERNEL);
    FCCW((ccwptr->rw_sma),CCWSWMA,CCWCC);
    FCCW((ccwptr->rw),CCWWMA,0);
    rc = gswio(dev, ccbptr, G_SYNC_IO, FP_IO);
    if (rc)
       return(rc);
    /*------------------------------------------------------------------*/
    /* Free the space.                                                  */
    /*------------------------------------------------------------------*/
    rc = gfree_spac((char **)&p);
    return(0);

  case  FPGI_DISCONNECT   :
    /*------------------------------------------------------------------*/
    /* Malloc a large area and carve up.                                */
    /*------------------------------------------------------------------*/
    malloc_size =
		sizeof(struct com_rwfp)  +      /* ccb and ce's         */
		sizeof(struct ccw2)      +      /* SWMA/WMA ccw's       */
		sizeof(struct sma)       +      /* SMA data             */
		sizeof(struct conn_req);        /* connection request   */
    p = (char *)gget_spac(malloc_size, 3);
    if (p == NULL)
	return(ENOMEM);
    ccbptr  = (struct com_rwfp *)p;
    ccwptr  = (struct ccw2 *)    (ccbptr+1);
    smaptr  = (struct sma *)     (ccwptr+1);
    ptr     = (struct conn_req *)(smaptr+1);

    /*------------------------------------------------------------------*/
    /* All io operation info is moved to kernel space.                  */
    /*------------------------------------------------------------------*/
    arg1 = (struct fpgi_connection *)&(arg.valuep->f_connect);
    ptr->len      = 0x08;
    ptr->cmd      = CONN_CMD;
    ptr->func     = TRUE;
    ptr->req_size =  NULL;
    ptr->max_size = NULL;
    ptr->target   = NULL;

    FSMA(*smaptr,FSMAMEMID,0,10);
    FCCB(*ccbptr);
    FCEX(*ccbptr,0,LINK1,sizeof(struct ccw2),ccwptr,DMA_KERNEL);
    FCEX(*ccbptr,1,LINK1,10,smaptr,DMA_KERNEL);
    FCEX(*ccbptr,2,0,8,ptr,DMA_KERNEL);
    FCCW((ccwptr->rw_sma),CCWSWMA,CCWCC);
    FCCW((ccwptr->rw),CCWWMA,0);
    rc = gswio(dev, ccbptr, G_SYNC_IO, FP_IO);
    if (rc)
	return(rc);

    /*------------------------------------------------------------------*/
    /* Free the space.                                                  */
    /*------------------------------------------------------------------*/
    rc = gfree_spac((char **)&p);
    return(0);

  case  FPGI_INFREE :
		g->fp_inbufs = TRUE;    /* inbound bfrs now available   */
		return(0);
#ifdef JUNK
  case  FPGI_SEND :

		g->fp_sendptr = (struct fpgi_send *)arg.valuep->f_send;
		FCEX(*(g->fp_ccbp + g->fp_io),2,0,fp_sendptr->buf_len,
			   fp_sendptr->bufp,DMA_USER);
		(g->fp_smap + g->fp_io)->memcnt = fp_sendptr->buf_len;
		rc = gswio(dev,(g->fp_ccbp + g->fp_io),G_ASYNC_IO,FP_IO);
		return(rc);
#endif /* JUNK */

  case  FPGI_WAIT :
		/*------------------------------------------------------*/
		/* For 1st release, only dual i/o is supported, not     */
		/* multiple i/o. therefore, just return, since dual i/o */
		/* logic synchronizes the sio calls.                    */
		/*------------------------------------------------------*/
		return(0);
/*********************** END FPGI CALLS *********************************/
#endif /* FPGICOMPILE */

  case  G_SET_MEM :                 /*-set memory area id------------*/

    g->rw_smap->memid       = arg.value;    /* 1st rw_sma            */
    (g->rw_smap + 1)->memid = arg.value;    /* 2nd rw_sma            */
    TRCHKL0T(hkwd_IOCTL_R);
    return(0);

  case  G_ALARM :                   /*-sound alarm-------------------*/

    ws_typ = galrm;
    break;

  case  G_STOP :                    /*-stop buffer-------------------*/

    ws_typ = gstop;
    break;

  case  G_START :                   /*-start buffer------------------*/

    ws_typ = gstrt;
    break;

  case  G_RESET_CUR :               /*-reset cursor------------------*/

    ws_typ = grcur;
    break;

  case  G_SBF_START :               /*-set buf addr and start--------*/

    ws_typ = gsbfst;
    bcopy((char *)&(arg.value),(char *)&(sfp->half1),4);
    sfp->half3      =  6;
    sfp->half4      = START_TC;
    wslen = 16;
    break;

  case  G_SET_BF :                  /*-set buf addr------------------*/

    ws_typ = gsbf;
    bcopy((char *)&(arg.value),(char *)&(sfp->half1),4);
    break;

  case  G_SET_CUR :                 /*-set cursor and insert---------*/

    ws_typ = gscur;
    bcopy((char *)&(arg.value),(char *)&(sfp->half1),4);
    break;

  case  G_SET_IND :                 /*-set    indicators-------------*/

    ws_typ = gsinds;
    bcopy((char *)&(arg.value),(char *)&(sfp->half1),4);
    break;

  case  G_SET_IND_ON :              /*-set indicators on-------------*/

    ws_typ = gsion;
    bcopy((char *)&(arg.value),(char *)&(sfp->half1),4);
    break;

  case  G_SET_IND_OFF :             /*-set indicators off------------*/

    ws_typ = gsioff;
    bcopy((char *)&(arg.value),(char *)&(sfp->half1),4);
    break;

  case  G_WRITESF :                 /*-write structured--------------*/

    rc = gio_ioctl(dev,G_WRITESF,arg,NULL);
    if(rc)
       TRCHKL4T(hkwd_IOCTL_R2,rc,dev,cmd,&arg);
    else
       TRCHKL0T(hkwd_IOCTL_R);
    return(rc);

  case  G_DEF_MEM :                 /*-define mem area---------------*/

    ws_typ = gdfmem;
    copyin(arg.valuep,(char *)&sfp->half1,6);
    break;                          /*                               */

  case  G_REN_MEM :                 /*-rename mem area---------------*/

    ws_typ = grnmem;
    copyin(arg.valuep,(char *)&sfp->half1,6);
    break;                          /*                               */

  case  G_DEL_MEM :                 /*-delete mem area---------------*/

    ws_typ     = gdlmem;
    sfp->half1 = arg.value;
    break;                          /*                               */

  case  G_LOAD_BLNK :               /*-load blnk pattern-------------*/

    copyin(arg.valuep,(char *)&parmsp->g_blnk,8);
    if ((parmsp->g_blnk.number > 6) || (parmsp->g_blnk.number < 0) ||
	(parmsp->g_blnk.pattern < 2) || (parmsp->g_blnk.pattern > 7) )
    {
	TRCHKL5T(hkwd_IOCTL_R3,EINVAL,dev,G_LOAD_BLNK,
		 parmsp->g_blnk.number,parmsp->g_blnk.pattern);
	return(EINVAL);
    }
    sfp->b.sflen = 8 + ((parmsp->g_blnk.number)<<1);
    sfp->b.t_c   = LD_BLNK_TC;
    sfp->half1   = parmsp->g_blnk.pattern;
    copyin(parmsp->g_blnk.buf_adr, (char *)(&(sfp->half2)),
	   (parmsp->g_blnk.number)<<1 );
    break;

  case  G_LOAD_LINE :               /*-load line pattern-------------*/
				    /*-special case: large sf--------*/
    copyin(arg.valuep,(char *)&parmsp->g_line,8);
    if ((parmsp->g_blnk.number < 0) || (parmsp->g_blnk.number > 12) ||
	(parmsp->g_blnk.pattern < 4) || (parmsp->g_blnk.pattern > 15) )
    {
	    TRCHKL5T(hkwd_IOCTL_R3,EINVAL,dev,G_LOAD_LINE,
		 parmsp->g_blnk.number,parmsp->g_blnk.pattern);
	    return(EINVAL);
    }

    sflp->b.sflen = 8 + (parmsp->g_line.number<<5);
    sflp->b.t_c   = LD_LINE_TC;
    sflp->frst_lp = parmsp->g_line.pattern;
    copyin(parmsp->g_line.buf_adr,(char *)(sflp->lp),
		(parmsp->g_line.number)<<5 );
    sfp = (struct basic_ws *)sflp;
    break;                          /*                               */

  case  G_READ_CUR :                /*-read cursor location----------*/

    rc = gio_ioctl(dev,G_READ_CUR,arg,NULL);
    if (rc)
       TRCHKL4T(hkwd_IOCTL_R2,rc,dev,cmd,&arg);
    else
       TRCHKL0T(hkwd_IOCTL_R);
    return(rc);

  case  G_SENSE :                   /*-perform sense-----------------*/

    rc = gio_ioctl(dev,G_SENSE,arg,NULL);
    if(rc)
       TRCHKL4T(hkwd_IOCTL_R2,rc,dev,cmd,&arg);
    else
       TRCHKL0T(hkwd_IOCTL_R);
    return(rc);

  case  K_LCW :                     /*-link command words------------*/

    rc = gio_ioctl(dev,K_LCW,arg,NULL);
    if (rc)
       TRCHKL4T(hkwd_IOCTL_R2,rc,dev,cmd,&arg);
    else
       TRCHKL0T(hkwd_IOCTL_R);
    return(rc);

       /*-----------------------------------------------------------*/
       /* technique used for the queue manipulation requests:       */
       /*                                                           */
       /* bit7 is equivalent to GPICK (0x01)                        */
       /* bit6 is equivalent to GANK  (0x02)                        */
       /* bit5 is equivalent to GPFK  (0x04)                        */
       /* bit4 is equivalent to GLPFK (0x08)                        */
       /* bit3 is equivalent to GTABLET(0x10)                       */
       /* bit2 is equivalent to GSMI  (0x20)                        */
       /* bit1 is equivalent to GGEOP (0x40)                        */
       /* bit0 is equivalent to GPGM_ERR (0x80)                     */
       /*                                                           */
       /* scan 8 bits from the right (bit7). if any are 1, then     */
       /* the input device argument is valid.                       */
       /* enum type inp_typ is used.                                */
       /*-----------------------------------------------------------*/

  case  K_ENABLE :                   /*-enable type for event mode */
    copyin(arg.valuep,(char *)parmsp,20);
					/*------------------------------*/
					/* check data_len: max to read  */
					/* is MAX_DATA (512).           */
					/*------------------------------*/
    if (parmsp->k_ena.data_len > MAX_DATA)
    {
        TRCHKL4T(hkwd_IOCTL_R4,EINVAL,dev,cmd,parmsp->k_ena.data_len);
	return(EINVAL);
    }

    G_DISABLE
					/*------------------------------*/
					/* search for type and set up   */
					/* interrupt queue  (iqt entry) */
                                        /*------------------------------*/
    TRCHKL3T(hkwd_IOCTL_R10,dev,cmd,(uint)parmsp->k_ena.type);
    for (i = 0; i < MAX_INP ; bit <<= 1,i++)
    {
	if ((int)parmsp->k_ena.type & bit)
	{
		if ( (hdr_ptr+i)->intrp == NULL ) /* 1st time : malloc */
		{
		      G_ENABLE
		      if ((p = gget_spac((int)(tbl_size[i]*sizeof(IQT)),2))
			== NULL )
		      {
                        TRCHKL4T(hkwd_IOCTL_R5,ENOMEM,dev,cmd,i);
			return(ENOMEM);
		      }
		      else
		      {
			    G_DISABLE
			    (hdr_ptr+i)->intrp = (IQT *)p;
		      }
		}
		switch (bit) {
		   case GSMI :
		   case GGEOP :
		      if (parmsp->k_ena.type == GALL)
				parmsp->k_ena.qualifier = -1;
		      if ( (tablep =
			   gfnd_iqt((hdr_ptr+i)->intrp,
			   (struct k_enable *)parmsp,i)) == NULL) {
				G_ENABLE
                                TRCHKL5T(hkwd_IOCTL_R6,ENOMEM,dev,
                                 (hdr_ptr+i)->intrp,parmsp,i);
				return(ENOMEM);
		      }
		      break;
		   default :
		      tablep = (hdr_ptr+i)->intrp;
		      break;
		}
		gfil_iqt(tablep,(struct k_enable *)parmsp,i);
		tablep->flags.mode = GEVENT;
		found = TRUE;
	}
    }
    TRCHKL0T(hkwd_IOCTL_R);
    LEAVE_IT;

  case  K_DISABLE :                 /*-disable     interrupt --------*/

    copyin(arg.valuep, (char *)parmsp,8);
    G_DISABLE
    for (i = 0; i < MAX_INP ; bit <<= 1,i++) {
	if ((int)parmsp->k_dis.type & bit)
	{
		gcln_iqt((hdr_ptr+i)->intrp,(struct k_disable *)parmsp,i);
		(hdr_ptr+i)->intrp->flags.mode = GDISABLE;
		found = TRUE;
	}
    }
    TRCHKL0T(hkwd_IOCTL_R);
    LEAVE_IT;

  case  K_ENA_SIG :                 /*-enable "signal on interrupt"--*/

    G_DISABLE
    g->signal = arg.value;
    found     = TRUE;
    TRCHKL0T(hkwd_IOCTL_R);
    LEAVE_IT;

  case  K_DIS_SIG :                 /*-disable "signal on interrupt"-*/

    G_DISABLE
    g->signal = 0;
    found     = TRUE;
    TRCHKL0T(hkwd_IOCTL_R);
    LEAVE_IT;

  case  K_POLL :                    /*-poll input event  queue-------*/

    G_DISABLE
    this_qel = gget_qel(dev);                        /*--------------*/
    if (this_qel == NULL)                            /* q empty,     */
    {                                                /* no_data flag */
	    G_ENABLE                                 /*--------------*/
	    tmp_qel.flags.no_data = TRUE;
	    copyout((char *)&tmp_qel,(char *)(arg.value),sizeof(q_qel));
            TRCHKL0T(hkwd_IOCTL_R);
	    return(0);
    }
    G_ENABLE
    copyout((char *)this_qel,(char *)arg.value,
	    this_qel->data_len + sizeof(q_qel));
    TRCHKL0T(hkwd_IOCTL_R);
    return(0);

  case  K_WAIT :                    /*-wait on input event  queue----*/

    G_DISABLE
   /*-----------------------------------------------------------*/
   /* if q empty, sleep.                                        */
   /*-----------------------------------------------------------*/
    this_qel = gget_qel(dev);
    if (this_qel == NULL) {
	g->oflag.slp_pending = TRUE;

/* Changed - JCC */
/*	rc = DO_SLEEP(&g->sleep_eventq,EVENT_SIGRET); */
	rc = DO_SLEEP(&g->sleep_eventq, LOCK_HANDLER | INTERRUPTIBLE);

/* Changed - JCC */
/*	if (rc != EVENT_SUCC) */           /* terminated - didnt get event */
	if (rc != THREAD_AWAKENED)           /* terminated - didnt get event */
		END_SLEEP
	g->oflag.wokehdr     = FALSE;   /* reset flag set in intr hndlr */
	g->oflag.slp_pending = FALSE;
	if (g->oflag.sig_pending)
	{
		SIGAPPL(g->signal);
		g->oflag.sig_pending = FALSE;
	}
	if ( (this_qel = gget_qel(dev)) == NULL )
	{
		G_ENABLE
                TRCHKL2T(hkwd_IOCTL_R1,ENOMEM,dev);
		return(ENOMEM);
	}
    }
    G_ENABLE
					/*------------------------------*/
					/* copyout to user space        */
					/*------------------------------*/
    copyout((char *)this_qel,(char *)arg.valuep,
		this_qel->data_len + sizeof(q_qel));
    TRCHKL0T(hkwd_IOCTL_R);
    return(0);


  case  K_FLUSH :                   /*-flush input event  queue------*/

    G_DISABLE
    g->q.head = g->q.top;
    g->q.tail = g->q.top;
    found = TRUE;
    TRCHKL0T(hkwd_IOCTL_R);
    LEAVE_IT;

  case  K_REQUEST :                  /*-request mode --------------*/

    copyin(arg.valuep,(char *)parmsp,24);
					/*------------------------------*/
					/* check data_len: max to read  */
					/* is MAX_DATA (512).           */
					/*------------------------------*/
    if (parmsp->k_req.data_len > MAX_DATA)
    {
        TRCHKL4T(hkwd_IOCTL_R4,EINVAL,dev,cmd,parmsp->k_req.data_len);
	return(EINVAL);
    }
    G_DISABLE
					/*------------------------------*/
					/* search for type and set up   */
					/* interrupt queue              */
					/*------------------------------*/
    TRCHKL3T(hkwd_IOCTL_R10,dev,cmd,(uint)parmsp->k_req.type);
    for (i = 0; i < MAX_INP ; bit <<= 1,i++)
    {
	if ((int)parmsp->k_req.type == bit)
	{
		if ( (hdr_ptr+i)->intrp == 0 )   /* 1st time : malloc */
		{
		      G_ENABLE
		      if ((p = gget_spac((int)(tbl_size[i]*sizeof(IQT)),2))
			== NULL )
		      {
                         TRCHKL4T(hkwd_IOCTL_R5,ENOMEM,dev,cmd,i);
		         return(ENOMEM);
		      }
		      else
		      {
			    G_DISABLE
			    (hdr_ptr+i)->intrp = (IQT *)p;
		      }
		}
		switch (bit) {
		   case GSMI :
		   case GGEOP :
		      if ( (tablep =
		       gfnd_iqt((hdr_ptr+i)->intrp,
		       (struct k_enable *)parmsp,i)) == NULL) {
				G_ENABLE
                                TRCHKL5T(hkwd_IOCTL_R6,ENOMEM,dev,
                                 (hdr_ptr+i)->intrp,parmsp,i);
				return(ENOMEM);
		      }
		      break;
		   default :
		      tablep = (hdr_ptr+i)->intrp;
		      break;
		}
		gfil_iqt(tablep,(struct k_enable *)parmsp,i);
		tablep->flags.mode   = GREQUEST;
		g->oflag.slp_pending = TRUE;
					/*------------------------------*/
		do {                    /* loop until req happens       */
					/*------------------------------*/

					/*------------------------------*/
					/* if serious signal, exit      */
					/*------------------------------*/

/* Changed - JCC */
/*		    rc = DO_SLEEP(&g->sleep_eventq, EVENT_SIGRET); */
		    rc = DO_SLEEP(&g->sleep_eventq, LOCK_HANDLER | INTERRUPTIBLE);

/* Changed - JCC */
/*		    if (rc != EVENT_SUCC) */ /* terminated-didnt get event */
		    if (rc != THREAD_AWAKENED) /* terminated-didnt get event */
			END_SLEEP

		    g->oflag.wokehdr = FALSE;

		} while (!(g->oflag.req_intr));

		g->oflag.wokehdr     = FALSE;
		g->oflag.slp_pending = FALSE;

		if (g->oflag.sig_pending)
		{
			SIGAPPL(g->signal);
			g->oflag.sig_pending = FALSE;
		}

		tablep->flags.mode = GDISABLE;
		G_ENABLE
		data_len = parmsp->k_req.data_len + dflt_size[i];

		copyout((char *)(g->req_data),parmsp->k_req.target_adr,
			    data_len + sizeof(q_qel));
		found = TRUE;
                TRCHKL0T(hkwd_IOCTL_R);
		return(0);
	}
    }
    G_ENABLE
    TRCHKL4T(hkwd_IOCTL_R7,EINVAL,dev,cmd,(uint)parmsp->k_req.type); 
    return(EINVAL);
					   /* end of K_REQUEST */
/*
  case K_STOP_DEVICE :

	stop_devs = TRUE;
	rc = gchk_det(dev);
        if (rc)
           TRCHKL3T(hkwd_IOCTL_R8,rc,dev,cmd);
        else
           TRCHKL0T(hkwd_IOCTL_R);
	return(rc);
*/


#ifdef KEEP_STOPPING_DEVICES
  case K_STOP_ALL_DEVS :                  /* for stop_msla.             */
					  /* if dev att,  detach it     */
	G_DISABLE
	mf.need_start = FALSE;
	mf.need_stop  = FALSE;
	if (mf.start_in_prog)
	{
	   mf.need_stop = TRUE;
	   G_ENABLE
        }
        else                   /* do the STOP now              */
        {
	   mf.stop_in_prog = TRUE;
           comm_lock = 1;     /* lock communication area      */
       	   segreg = (ulong)BUSMEM_ATT((ulong)BUS_ID,(ulong)0);
	   rap = (struct rm_comm_area *)
	          (segreg + start_busmem + RtoMCommAdrOfst);
	   G_ENABLE
	   SET_PARITYJMPRC(0,&(g->parbuf),ras_unique[1],
		       ioctl_name,PIOParityError, NO_UNLOCK)
	   stop_strt();       /* issues stop cmd              */
	   CLR_PARITYJMP(&(g->parbuf))
       	   BUSMEM_DET(segreg);
        }
    /********************************************* FPGI mode ****
    if (g->devmode == FPGI_MODE)
	send_done = FALSE;
    ********************************************* FPGI mode ****/
	for ( i = NumDevSupp - 1; i < 0; i--) {
           (mgcb_ptr+i)->iflag.not_to_ready = FALSE;
           (mgcb_ptr+i)->iflag.io_allowed   = FALSE;
           rc = gcln_intr(i);    /* cln_intr returns int     */
           if (rc)
              ret = rc;
        }

        if (ret)
           TRCHKL3T(hkwd_IOCTL_R8,ret,0,cmd);
        else
           TRCHKL0T(hkwd_IOCTL_R);
        return(ret);
/*
	stop_devs = TRUE;
	for ( i = NumDevSupp - 1; i < 0; i--) {
	      rc = gchk_det(i);
		if (rc)
		   return(rc);
	}
	return(rc);
*/
#endif /* KEEP_STOPPING_DEVICES */



  case MSLA_START_DIAG:
    /*mgcb_ptr->oflag.diag_mode = TRUE;*/
    /*diag_mode = TRUE; */              /*removed 10/20/90 bb . done in gswopen*/
    intr_cntptr->total_cnt    = 0;
    intr_cntptr->parity_cnt   = 0;
    /*  RST_MSLA  done at the micro code level */
    TRCHKL0T(hkwd_IOCTL_R);
    return(0);

  case MSLA_QUERY_DIAG:
    if (diag_mode == FALSE) {
       TRCHKL3T(hkwd_IOCTL_R8,EINVAL,dev,cmd);
       return(EINVAL);
    }
    copyout((char *)intr_cntptr,(char *)arg.value,
	    sizeof(struct msla_intr_count));
    intr_cntptr->total_cnt  = 0;
    intr_cntptr->parity_cnt = 0;
    TRCHKL0T(hkwd_IOCTL_R);
    return(0);

  case MSLA_STOP_DIAG:
    if (diag_mode == FALSE) {
       TRCHKL3T(hkwd_IOCTL_R8,EINVAL,dev,cmd);
       return(EINVAL);
    }
  /*  diag_mode = FALSE;*/
    TRCHKL0T(hkwd_IOCTL_R);
    return(0);

  case MSLA_GET_ADDR:
    msla_mapptr = (struct msla_map *)gget_spac(sizeof(struct msla_map),2);
    if (msla_mapptr == NULL)
    {
       PRNTE(("gswioctl:gget_spac failed for 'msal_map' struct\n"));
       TRCHKL3T(hkwd_IOCTL_R8,ENOMEM,dev,cmd);
       return(ENOMEM);
    }
    ioaddr = (ulong)(as_att((adspace_t *)getadsp()
                           ,(vmhandle_t)BUS_ID,0x00));
    msla_mapptr->msla_mem_start = ioaddr | ((struct gsw_dds * )
		 ddsp)->sd.start_busmem;
    msla_mapptr->msla_io_start = ioaddr | ((struct gsw_dds * )
		 ddsp)->sd.start_busio;
    copyout((char *)msla_mapptr,(char *)arg.value,sizeof(struct msla_map));
    TRCHKL0T(hkwd_IOCTL_R);
    return(0);

  case MSLA_RET_ADDR:
    if(msla_mapptr == NULL) {
       TRCHKL3T(hkwd_IOCTL_R8,ENOMEM,dev,cmd);
       return(ENOMEM);
    }
    as_det((adspace_t *)getadsp(),
           (caddr_t)(msla_mapptr->msla_mem_start&0xF0000000));
    chp = (char *)msla_mapptr;
    rc = gfree_spac((char **)&chp);
    if (rc) {
       PRNTE(("gswioctl: 'gfree_spac' failed for 'msla_map'\n"));
       TRCHKL3T(hkwd_IOCTL_R8,rc,dev,cmd);
    }
    else
       TRCHKL0T(hkwd_IOCTL_R);
    return(0);

  case MSLA_MOD_POS:
    switch(arg.value) {
       case MP_ENA_PARITY:
	  ENA_PARITY_POS;
	  break;
       case MP_DIS_PARITY:
	  DIS_PARITY_POS;
	  break;
       case MP_ENA_CARD:
	  ENA_MSLA_POS;
	  break;
       case MP_DIS_CARD:
	  DIS_MSLA_POS;
	  break;
       case MP_STOP:
	  /*  HALT_MSLA  **** done at micro code level  ****/
	  iocc_addr =
            IOCC_ATT((ulong)BUS_ID,(ulong)(POSREG(2,adapter_slot)+IO_IOCC));
	  pptr = (volatile char *)iocc_addr;

	  /*------------------------------------------------------------*/
	  /* First disable adapter, set up memory address and then      */
	  /* enable adapter.                                            */
	  /*------------------------------------------------------------*/
	  *pptr++ = ddsp->sd.intr_level << 4; /* Enintr+disadptrPOS2 */
	  *pptr++ = ddsp->sd.start_busio >> 8;  /* io adr       POS3 */
	  *pptr++ = ddsp->sd.start_busmem >> 16;/* mem adr      POS4 */
	  *pptr++ = 0xC0 + (ddsp->sd.dma_level);/* DMA reg      POS5 */
	  pptr    = (char *)iocc_addr;             /* Ena adapter  POS2 */
	  *pptr   = (ddsp->sd.intr_level<<4)+Ena_Adapter+BusMasterDelay;

	  IOCC_DET((ulong)iocc_addr);
	  SET_PARITYJMPRC(0,&(g->parbuf),ras_unique[2],
		       ioctl_name,PIOParityError, NO_UNLOCK)
	  ENA_INTR_MSLA
	  CLR_PARITYJMP(&(g->parbuf))
	  break;

       default:
          TRCHKL3T(hkwd_IOCTL_R8,EINVAL,dev,cmd);
	  return(EINVAL);
    }
    TRCHKL0T(hkwd_IOCTL_R);
    return(0);

  case MSLA_LOAD_UCODE:

     iocc_addr =
        IOCC_ATT((ulong)BUS_ID,(ulong)(POSREG(2,adapter_slot)+IO_IOCC));
     pptr = (char *)iocc_addr;

     /*------------------------------------------------------------*/
     /* First disable adapter, set up memory address and then      */
     /* enable adapter.                                            */
     /*------------------------------------------------------------*/
     *pptr++ = ddsp->sd.intr_level << 4; /* Enintr+disadptrPOS2 */
     *pptr++ = ddsp->sd.start_busio >> 8;  /* io adr       POS3 */
     *pptr++ = ddsp->sd.start_busmem >> 16;/* mem adr      POS4 */
     *pptr++ = 0xC0 + (ddsp->sd.dma_level);/* DMA reg      POS5 */
     pptr    = (char *)iocc_addr;          /* Ena the adapter POS2 */
     *pptr   = (ddsp->sd.intr_level<<4)+Ena_Adapter+BusMasterDelay;

     IOCC_DET((ulong)iocc_addr);
     for (i = 0; i < NumDevSupp; i++)
     {
		    (mgcb_ptr+i)->iflag.io_allowed   = FALSE;
		    (mgcb_ptr+i)->iflag.not_to_ready = FALSE;
		    ipl_flag = TRUE;
     }
     comm_lock = 0;
     d_mask(dma_chanid);         /* disable DMA channel          */
     d_clear(dma_chanid);        /* free    DMA channel          */
     SET_PARITYJMPRC(0,&(g->parbuf),ras_unique[3],
		  ioctl_name,PIOParityError, NO_UNLOCK)
     DIS_INTR_MSLA               /* disable interrupts           */
     CLR_PARITYJMP(&(g->parbuf))
     mf.dma_enabled = FALSE;
     TRCHKL0T(hkwd_IOCTL_R);
     diag_mode = FALSE;
     /*---------------------------------------------------------*/
     /* Now that diag_mode flag is turned off (and intrpt hndlr */
     /* wont process diag_mode interrupts), we can free the     */
     /* space for the counters.                                 */
     /*---------------------------------------------------------*/
     chp = (char *)intr_cntptr;
     rc = gfree_spac((char **)&chp);
     if (rc)
	 PRNTE(("gswioctl: 'gfree_spac' failed for 'msla_intr_count'\n"));
     return(0);

  case MSLA_START_DMA:
    dma_tst_parmp = (struct dma_test_parms *)gget_spac(
                     sizeof(struct dma_test_parms),3);
    if (dma_tst_parmp == NULL)
    {
       TRCHKL3T(hkwd_IOCTL_R8,ENOMEM,dev,cmd);
       return(ENOMEM);
    }
    
    copyin((char *)arg.value,(char *)dma_tst_parmp,
	   sizeof(struct dma_test_parms));

    dma_tdataptr = (char *)gget_spac(PAGESIZE,12);
    if (dma_tdataptr == NULL)
    {
       TRCHKL3T(hkwd_IOCTL_R8,ENOMEM,dev,cmd);
       return(ENOMEM);
    }
    copyin((char *)(dma_tst_parmp->ubuff_adr),dma_tdataptr,
	   dma_tst_parmp->count);
    io_addr = dma_tdataptr;

    if (g->xm.dp != 0)
    {
	rc = gfree_spac((char **)&g->xm.dp);
	if (rc)
		PRNTE(("gswioctl: free g->xm.dp FAILED.xm.dp=x%x,rc=x%x\n",
				   g->xm.dp,rc));
    }
    ccw_cnt    = 1;
    g->xm.cnt  = ccw_cnt;
    g->xm.indx = 1;
    g->xm.dp   = (struct xmem *)
		  gget_spac((ccw_cnt+1)*sizeof(struct xmem),12);
    if (g->xm.dp == NULL)
    {
       TRCHKL3T(hkwd_IOCTL_R8,ENOMEM,dev,cmd);
       return(ENOMEM);
    }
    g->xm.dp->aspace_id     = XMEM_GLOBAL;
    (g->xm.dp+1)->aspace_id = XMEM_GLOBAL;

    adr_for_tcw =
        ((ulong)g->dma_bus_addr) | (((ulong)dma_tdataptr) & 0x00000FFF);
    copyout((char *)&adr_for_tcw,(char *)dma_tst_parmp->dma_adr_p,
	     sizeof(ulong));

    d_master(dma_chanid,DMA_READ,io_addr,
             (size_t)dma_tst_parmp->count, g->xm.dp+1, g->dma_bus_addr);
    TRCHKL0T(hkwd_IOCTL_R);
    return(0);

  case MSLA_STOP_DMA:
    rc = d_complete(dma_chanid,DMA_READ,io_addr,
              (size_t)dma_tst_parmp->count, g->xm.dp+1,g->dma_bus_addr);
    ret = rc;


    copyout((char *)dma_tdataptr,(char *)dma_tst_parmp->ubuff_adr,
	    dma_tst_parmp->count);
    chp = (char *)dma_tdataptr;
    rc = gfree_spac((char **)&chp);
    if (rc)
       PRNTE(("gswioctl: 'gfree_spac' failed for 'dma_tdataptr'\n")); 

    chp = (char *)dma_tst_parmp;
    rc = gfree_spac((char **)&chp);
    if (rc) {
       PRNTE(("gswioctl: 'gfree_spac' failed for 'dma_test_parms'\n")); 
       TRCHKL3T(hkwd_IOCTL_R8,rc,dev,cmd);
    }
    else
       TRCHKL0T(hkwd_IOCTL_R);
    if (ret)
       return(EIO);
    return(0);

  default:
    TRCHKL3T(hkwd_IOCTL_R8,EINVAL,dev,cmd);
    return(EINVAL);
  }
  /*-----------------------------------------------------------*/
  /* end of   switch                                           */
  /*-----------------------------------------------------------*/

  /*-----------------------------------------------------------*/
  /* fill in base using index into tables wsf_len and wsf_code */
  /* write ws to 5080 via a wsf command                        */
  /*-----------------------------------------------------------*/

  if (ws_typ != unused ) {
	sfp->b.sflen = wsf_len[(int)ws_typ];
	sfp->b.t_c   = wsf_typ[(int)ws_typ];
  }

  if (wslen == 0)
	wslen = sfp->b.sflen;

  bcopy((char *)&sfp,(char *)&(arg.value),4);
  rc =  gio_ioctl(dev,G_IOCTL,arg,wslen);
  if (rc)
     TRCHKL5T(hkwd_IOCTL_R9,rc,dev,cmd,sfp,wslen);
  else
     TRCHKL0T(hkwd_IOCTL_R);
  return(rc);
} /* end ioctl */

