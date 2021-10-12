#ifndef lint
static char sccsid[] = "@(#)78 1.60 src/bos/kernext/pty/spty.c, sysxpty, bos41J, 9520A_all 5/11/95 08:30:14";
#endif
/*
 * COMPONENT_NAME: SYSXPTY  - sptydd streams module
 *
 * FUNCTIONS: 	ptsm_config, pts_open, pts_close, pts_wput, pts_wsrv, pts_rput, 
 * 		pts_rsrv, pty_setnameattr, pty_vsetattr, ptm_open, ptm_close, 
 *              ptm_wput, ptm_wsrv, ptm_rput, ptm_rsrv, ptm_send_pckt, 
 *              pty_mctl, pty_strioctl_comm, pty_ioctl_comm, pty_bsd43_ioctl, 
 *              pty_ctl, pty_pts_xfer, pts_put, ptm_put, pty_ptm_xfer, 
 *              ptm_create_ptp,  pty_open_comm,	call_pty_open_comm, 
 *              call_pty_close_comm, pty_close_comm, pty_kdb, trace_prsc, 
 *              trace, trace_mblk_t, trace_cmd, trace_error, pty_print
 * 
 * ORIGINS: 40, 71, 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1991 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/
/*
 * OSF/1 1.2
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*                    +------------------------------+
 *                    |Streams based pseudo terminal.|
 *                    +------------------------------+
 *
 *
 * This file contains a streams based pty slave device (names starting
 * with "pts_"), plus a streams based, System V style master device (names
 * starting with "ptm_"). Communication between slave and master device is
 * done by two symetrics functions (pty_ptm_xfer() and pty_pts_xfer()).
 * Common procedures for master and slave devices have their names starting
 * with "pty_"
 * Debbuging facility exists with macro starting with "PTY_PRINT_". Trace
 * level is configurable changing value of 'PTY_TRACE' integer.
 * When under debbuger it is possible to check data structure of each
 * open device (pty_s structure) calling pty_kdb() function.
 * 
 * Each function has a banner and could be quicky found looking for ^L
 * character. 
 * 
 * 
 *
 * Master device driver:
 *----------------------
 *          OPEN                     --> ptm_open()
 *          CLOSE                    --> ptm_close()
 *          WRITE PUT                --> ptm_wput()
 *          READ PUT                 --> ptm_rput()
 *          WRITE SERVICE            --> ptm_wsrv()
 *          READ SERVICE             --> ptm_rsrv()
 *          SEND PACKET CONTROL CHAR --> ptm_send_pckt()
 *          
 * Slave device driver:
 *--------------------- 
 *          OPEN                     --> pts_open()
 *          CLOSE                    --> pts_close()
 *          WRITE PUT                --> pts_wput()
 *          READ PUT                 --> pts_rput()
 *          WRITE SERVICE            --> pts_wsrv()
 *          READ SERVICE             --> pts_rsrv()
 *          WRITE SERVICE and CLOSE 
 * 
 * Common procedures to slave and master device driver :
 * -----------------------------------------------------
 *          CONFIGURATION            --> ptsm_config()
 *          OPEN                     --> pty_open_comm() and
 *					 call_pty_open_comm()
 *          CLOSE                    --> pty_close_comm() and
 *					 call_pty_close_comm()
 *          BUILD M_CTL MESSAGE      --> pty_ctl()
 *          PROCESS M_IOCTL          --> pty_strioctl_comm()
 *                                   --> pty_ioctl_comm()
 *          PROCESSING M_CTL MESSAGE --> pty_mctl()
 *          PROCESS BSD43 M_IOCTL    --> pty_bsd43_ioctl()
 *          
 *          
 * Send data from slave device driver to master device driver :
 * ------------------------------------------------------------
 *          SEND DATA TO MASTER SIDE --> pty_ptm_xfer()
 * 
 * Send data from master device driver to slave  device driver :
 * ------------------------------------------------------------
 *          SEND DATA TO SLAVE SIDE --> pty_pts_xfer()
 * 
 * 
 * An example of the layout at runtime is this:
 *
 *    +-------------+                       +-------------+
 *    |   server    |                       | application |
 * ---------------------------------------------------------
 *    | stream head |                       | stream head |
 *    +-------------+                       +-------------+
 *       |       ^                             |       ^
 *       |       |                             v       |
 *       |       |                          +-------------+
 *       |       |                          |    tioc     |
 *       |       |                          +-------------+
 *       |       |                             |       ^
 *       |       |                             v       |
 *    +-------------+                       +-------------+
 *    |    tioc     |                       |   ldterm    |
 *    +-------------+                       +-------------+
 *       |       ^                             |       ^
 *       v       |                             v       |
 *    +-------------+                       +-------------+
 *    |   (PTM)     | <--pty_ptm_xfer()---  |    (PTS)    |
 *    |   master    | ---pty_pts_xfer()---> |    slave    |
 *    +-------------+                       +-------------+
 */

/************************************************************************/
/*                                                                      */
/*      INCLUDE FILE                                                    */
/*                                                                      */
/************************************************************************/

#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/chownx.h"
#include "sys/vattr.h"
#include "sys/audit.h"
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/param.h>
#include <sys/stat.h>
#include <sys/vattr.h>
#include <sys/auditk.h>
#include<sys/user.h>            /* uio.h needs this and  
                                   remote mode looks at u space */
#include<sys/uio.h>             /* uio struct */
#include<sys/errno.h>           /* ENOMEM */
#include<sys/lock_alloc.h>
#include<sys/lockname.h>
#include<sys/intr.h>
#include<sys/sleep.h>           /* EVENT_NULL */
#include<sys/device.h>          /* CFG_INIT CFG_TERM */
#include<sys/strstat.h>
#include<sys/termio.h>
#include<sys/poll.h>            /* POLLIN,etc */
#include<sys/ttydefaults.h>
#include<sys/trchkid.h>         /* trace */
#include<sys/eucioctl.h>


#include "./spty.h"

/************************************************************************/
/*                                                                      */
/*      TRACE                                                           */
/*                                                                      */
/************************************************************************/


/* give some informations on output     */

#define out_1                   0x1001
#define out_2                   0x1002
#define out_3                   0x1003
#define out_4                   0x1004
#define out_5                   0x1005
#define out_6                   0x1006
#define out_7                   0x1007
#define out_8                   0x1008
#define out_9                   0x1009
#define out_10                  0x1010
#define out_11                  0x1011
#define out_12                  0x1012
#define out_13                  0x1013
#define out_14                  0x1014
#define out_15                  0x1015
#define out_16                  0x1016
#define out_17                  0x1017
#define out_18                  0x1018
#define out_19                  0x1019
#define out_20                  0x1020
#define out_21                  0x1021
#define out_22                  0x1022
#define out_23                  0x1023
#define out_24                  0x1024
#define out_25                  0x1025
#define out_26                  0x1026
#define out_27                  0x1027
#define out_28                  0x1028
#define out_29                  0x1029
#define out_30                  0x1030
#define out_31                  0x1031
#define out_32                  0x1032
#define out_33                  0x1033
#define out_34                  0x1034
#define out_35                  0x1035
#define out_36                  0x1036
#define out_37                  0x1037
#define out_38                  0x1038
#define out_39                  0x1039
#define out_40                  0x1040
#define out_41                  0x1041

#ifdef PTYDD_DEBUG
#define IN_PTY_TRACE            0x00000001 /* Print Input               */
#define CMD_PTY_TRACE           0x00000002 /* Print IOCTL command       */
#define MBLK_T_PTY_TRACE        0x00000004 /* Print message             */
#define LOCK_PTY_TRACE          0x00000008 /* Print lock                */
#define PTP_PTY_TRACE           0x00000010 /* Print pty_s structure     */
#define OUT_PTY_TRACE           0x00000020 /* Print Output              */
#define PRINT_PTY_TRACE         0x00000040 /* Print                     */

int     PTY_TRACE = 0;
#define INT_TRACE_OUT           int trace_out = out_1;

/* trace command */

#define PTY_PRINT_CMD(A,B)      if(PTY_TRACE & CMD_PTY_TRACE)           \
                                trace_cmd((A),(B))

/* trace message */
#define PTY_PRINT_MBLK_T(A,B)   if(PTY_TRACE & MBLK_T_PTY_TRACE)        \
                                trace_mblk_t((A),(B))


/* trace in */
#define PTY_PRINT_IN(A,B)       if(PTY_TRACE & IN_PTY_TRACE)            \
                                trace((A),(B))

/* trace out */
#define PTY_PRINT_OUT(A,B,C)    if(PTY_TRACE & OUT_PTY_TRACE)           \
                                trace_error((A),(B),(C))                

/* print */
#define PTY_PRINT(A)            if(PTY_TRACE & PRINT_PTY_TRACE)         \
                                trace((A),NULL)

#define PTY_TRACE_OUT(A)        (A)

#define PTY_ASSERT(A)           ASSERT((A))

#else /* PTYDD_DEBUG  */
#define PTY_PRINT_OUT(A,B,C)
#define PTY_PRINT_IN(A,B)
#define PTY_PRINT_CMD(A,B)
#define PTY_PRINT_MBLK_T(A,B)
#define PTY_PRINT(A)            
#define PTY_TRACE_OUT(A)
#define PTY_ASSERT(A)
#define INT_TRACE_OUT
#endif /* PTYDD_DEBUG */

/************************************************************************/
/*                                                                      */
/*      LOCKS                                                           */
/*                                                                      */
/************************************************************************/

#ifndef SIMPLE_LOCK_AVAIL
#define SIMPLE_LOCK_AVAIL       0
#endif /* SIMPLE_LOCK_AVAIL */

#define INT_PTYSAVPRI_LRC    int ptysavpri, lrc

static int                     lockl_ptp          = LOCK_AVAIL;
static Simple_lock             lock_open_close    = {SIMPLE_LOCK_AVAIL};
static int                     perf_lock          = 0;
static int                     wait_open_close    = EVENT_NULL;
static int                     open_close         = 0;
static short                   ident              = 0;

#ifdef PTYDD_DEBUG

#define PTY_LOCK(l)                                                        \
                            if(PTY_TRACE & LOCK_PTY_TRACE)                 \
                            printf("LOCK \n");                             \
                            ptysavpri = disable_lock(INTOFFL0, (l))

#define PTY_UNLOCK(l)                                                      \
                            unlock_enable(ptysavpri, (l));                 \
                            if(PTY_TRACE & LOCK_PTY_TRACE)                 \
                            printf("UNLOCK \n")

#define PTY_LOCKL(l)                                                       \
                            if(PTY_TRACE & LOCK_PTY_TRACE)                 \
                            printf("LOCKL\n");                             \
                            lrc=lockl((l),LOCK_SHORT)

#define PTY_UNLOCKL(l)                                                     \
                            if(PTY_TRACE & LOCK_PTY_TRACE)                 \
                            printf("UNLOCKL\n");                           \
                            if(lrc == LOCK_SUCC) unlockl((l)) 

#else /* PTYDD_DEBUG */


#define PTY_LOCK(l)                                                       \
                            ptysavpri = disable_lock(INTOFFL0, (l))

#define PTY_UNLOCK(l)                                                     \
                            unlock_enable(ptysavpri, (l))

#define PTY_LOCKL(l)                                                       \
                            lrc=lockl((l),LOCK_SHORT)      

#define PTY_UNLOCKL(l)                                                     \
                            if(lrc == LOCK_SUCC) unlockl((l))



#endif /* PTYDD_DEBUG */


/************************************************************************/
/*                                                                      */
/* LIST OF FUNCTIONS                                                    */
/*                                                                      */
/************************************************************************/


/* -------------------------------------------------------------------- */
int             ptsm_config     (dev_t dev, int cmd, struct uio *uiop);
/* -------------------------------------------------------------------- */
static int      pts_open        (queue_t *q, dev_t  *devp, int flag, 
                                 int sflag, cred_t *credp);
/* -------------------------------------------------------------------- */
static int      pts_close       (queue_t *q, int flag, cred_t *credp);
/* -------------------------------------------------------------------- */
static int      pts_wput        (queue_t *q, mblk_t *mp);
/* -------------------------------------------------------------------- */
static int      pts_wsrv        (queue_t *q);
/* -------------------------------------------------------------------- */
static int      pts_rput        (queue_t *q, mblk_t *mp);
/* -------------------------------------------------------------------- */
static int      pts_rsrv        (queue_t *q);
/* -------------------------------------------------------------------- */
static int      pty_setnameattr (char *pathname, int cmd, int arg1, 
                                 int arg2, int arg3, int lflag);
/* -------------------------------------------------------------------- */
static int      pty_vsetattr(register struct vnode *vp, register int cmd, 
	                    register int arg1, register int arg2, 
	                    register int arg3, struct ucred *crp);
/* -------------------------------------------------------------------- */
static int      ptm_open        (queue_t *q, dev_t  *devp, int flag, 
                                 int sflag,
                                 cred_t *credp);
/* -------------------------------------------------------------------- */
static int      ptm_close       (queue_t *q, int flag, cred_t *credp);
/* -------------------------------------------------------------------- */
static int      ptm_wput        (queue_t *q, mblk_t *mp);
/* -------------------------------------------------------------------- */
static int      ptm_wsrv        (queue_t *q);
/* -------------------------------------------------------------------- */
static int      ptm_rput        (queue_t *q, mblk_t *mp);
/* -------------------------------------------------------------------- */
static int      ptm_rsrv        (queue_t *q);
/* -------------------------------------------------------------------- */
static int      ptm_send_pckt   ( queue_t *q);
/* -------------------------------------------------------------------- */
static int      pty_mctl        (queue_t *q, mblk_t *mp, int master_call);
/* -------------------------------------------------------------------- */
static int     pty_strioctl_comm(queue_t *q, mblk_t *mp, 
                                  struct pty_s *ptp, int master_call);
/* -------------------------------------------------------------------- */
static int     pty_ioctl_comm(queue_t *q, mblk_t *mp, 
                                  struct pty_s *ptp, int master_call);
/* -------------------------------------------------------------------- */
static int      pty_bsd43_ioctl (int cmd, caddr_t data,
                                 struct termios *termp,int *compatflagsp); 
/* -------------------------------------------------------------------- */
static mblk_t * pty_ctl         (queue_t *q, int command, int size);
/* -------------------------------------------------------------------- */
static int      pty_pts_xfer     (register struct pty_s *ptp,
                                 register mblk_t *mp);
/* -------------------------------------------------------------------- */
static void     pts_put          ( register struct pty_s  *ptp, 
                                   register mblk_t *mp, int kind_put);
/* -------------------------------------------------------------------- */
static void     ptm_put          ( register struct pty_s  *ptp, 
                                   register mblk_t *mp, int kind_put);
/* -------------------------------------------------------------------- */
static int      pty_ptm_xfer     (register struct pty_s *ptp,
                                 register mblk_t *mp);
/* -------------------------------------------------------------------- */
static int      pty_create_ptp   (struct pty_s **pt_o_ptp, queue_t *q, 
                                  uint minor, int mode, int call_from);
/* -------------------------------------------------------------------- */
int             pty_open_comm  (queue_t *q, dev_t *devp, int mode,
                                 int call_from);
/* -------------------------------------------------------------------- */
int             call_pty_open_comm  (queue_t *q, dev_t *devp, int mode,
                                 int call_from);
/* -------------------------------------------------------------------- */
int             call_pty_close_comm (queue_t * q, int call_from);
/* -------------------------------------------------------------------- */
int             pty_close_comm (queue_t * q, int call_from);
/* -------------------------------------------------------------------- */

/************************************************************************/
/*                                                                      */
/* DEBUG FUNCTIONS                                                      */
/*                                                                      */
/************************************************************************/

#ifdef PTYDD_DEBUG
/* -------------------------------------------------------------------- */
static int     pty_kdb         () ;
/* -------------------------------------------------------------------- */
static int     trace_prsc      (char *string, int length);
/* -------------------------------------------------------------------- */
static int     trace           (char *routine_name, int trace);
/* -------------------------------------------------------------------- */
static int     trace_mblk_t    (char *routine_name, mblk_t *mp);
/* -------------------------------------------------------------------- */
static int     trace_cmd       (char *routine_name, int cmd );
/* -------------------------------------------------------------------- */
static int     trace_error     (int trace_out, char *routine_name, 
                                int  err_number
                               );
#endif /* PTYDD_DEBUG */
/* -------------------------------------------------------------------- */
extern  int     pty_print();
/* -------------------------------------------------------------------- */


/************************************************************************/
/*                                                                      */
/*      STRUCTURE AND GLOBALE VARIABLE                                  */
/*                                                                      */
/************************************************************************/

#define PUTHERE		0
#define PUTNEXT		1

struct spty_dds         spty_dds; 
struct pty_o_s *        pty_s_head_ATT;         /* used in pty_open_comm and 
                                                   in pty_close_comm    */
struct pty_o_s *        pty_s_head_BSD;         /* used in pty_open_comm and 
                                                   in pty_close_comm    */
int                     config_max_pts;         /* used in ptsm_config  */
int                     config_max_ttyp;        /* used in ptsm_config  */

int                     state_sak = SAK_NOTHING; /* ^X^R automaton variable */


static struct tioc_reply pty_tioc_reply[]={
        { TXTTYNAME,TTNAMEMAX,TTYPE_COPYOUT },
};

#ifdef TTYDBG
static struct str_module_conf
    ttydbg_conf = { COMPONENT_NAME, 'd', PTY_PDPF };
#endif  /* TTYDBG */
/************************************************************************/
/*                                                                      */
/*      Streams control structures.                                     */
/*                                                                      */
/************************************************************************/

/* -------------------------------------------------------------------- */
/* ---  AT&T master ---                                                 */
/* -------------------------------------------------------------------- */

static struct module_info ptc_info={
        7609,"ptc",0,INFPSZ,SYSV_MAST_HIWATER,SYSV_MAST_LOWATER
};
static struct qinit ptc_rinit={
        ptm_rput,ptm_rsrv,ptm_open,ptm_close,0,&ptc_info,0
};
static struct qinit ptc_winit={
        ptm_wput,ptm_wsrv,0,0,0,&ptc_info,0
};
struct streamtab ptcinfo={ &ptc_rinit,&ptc_winit };

/* -------------------------------------------------------------------- */
/* ---  AT&T slave ---                                                  */
/* -------------------------------------------------------------------- */

static struct module_info pts_info={
        7608,"pts",0,INFPSZ,SLAVE_HIWATER,SLAVE_LOWATER
};
static struct qinit pts_rinit={
        pts_rput,pts_rsrv,pts_open,pts_close,0,&pts_info,0
};
static struct qinit pts_winit={
        pts_wput,pts_wsrv,0,0,0,&pts_info,0
};
struct streamtab ptsinfo={ &pts_rinit,&pts_winit };


/* -------------------------------------------------------------------- */
/* ---  BSD master ---                                                  */
/* -------------------------------------------------------------------- */

static struct module_info ptyp_info={
        7607,"ptyp",0,INFPSZ,SYSV_MAST_HIWATER,SYSV_MAST_LOWATER
};
static struct qinit ptyp_rinit={
        ptm_rput,ptm_rsrv,ptm_open,ptm_close,0,&ptyp_info,0
};
static struct qinit ptyp_winit={
        ptm_wput,ptm_wsrv,0,0,0,&ptyp_info,0
};
struct streamtab ptypinfo={ &ptyp_rinit,&ptyp_winit };

/* -------------------------------------------------------------------- */
/* ---  BSD slave ---                                                   */
/* -------------------------------------------------------------------- */

static struct module_info ttyp_info={
        7606,"ttyp",0,INFPSZ,SLAVE_HIWATER,SLAVE_LOWATER
};
static struct qinit ttyp_rinit={
        pts_rput,pts_rsrv,pts_open,pts_close,0,&ttyp_info,0
};
static struct qinit ttyp_winit={
        pts_wput,pts_wsrv,0,0,0,&ttyp_info,0
};
struct streamtab ttypinfo={ &ttyp_rinit,&ttyp_winit };


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : ptsm_config                                       */
/*                                                                     */
/*        PARAMETER :  dev     : the BSD dev pair                      */
/*                     cmd     : CFG_INIT or CFG_TERM                  */
/*                     *uiop   : spty_dds structure                    */ 
/*                                                                     */
/*        RETURN : error if failure 0 otherwise                        */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        This configuration routine is called to initialize the       */
/*        PTY module.                                                  */
/*                                                                     */
/***********************************************************************/

int
ptsm_config(
            dev_t       dev,
            int         cmd,
            struct uio  *uiop
           )

{
   struct    spty_dds tmp_dds;
   register  struct   pty_o_s * pty_o;
   INT_TRACE_OUT
   int       error=0;
   strconf_t confptc= { "ptc",  &ptcinfo, STR_NEW_OPEN | STR_MPSAFE, -1, };
   strconf_t confpts= { "pts",  &ptsinfo, STR_NEW_OPEN | STR_MPSAFE, -1, };
   strconf_t confptyp={ "ptyp", &ptypinfo,STR_NEW_OPEN | STR_MPSAFE, -1, };
   strconf_t confttyp={ "ttyp", &ttypinfo,STR_NEW_OPEN | STR_MPSAFE, -1, };
   INT_PTYSAVPRI_LRC;

   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_CONFIG,dev,0,cmd,0,0);
   PTY_PRINT_IN("ptsm_config IN",NULL);
   PTY_PRINT_CMD("ptsm_config",cmd);

   PTY_ASSERT(dev != NULL);
   PTY_ASSERT(cmd == CFG_INIT || cmd == CFG_TERM);
   PTY_ASSERT(uiop != NULL);

   PTY_LOCKL(&lockl_ptp);

   confpts.sc_sqlevel=SQLVL_QUEUEPAIR;
   confptc.sc_sqlevel=SQLVL_QUEUEPAIR;
   confptyp.sc_sqlevel=SQLVL_QUEUEPAIR;
   confttyp.sc_sqlevel=SQLVL_QUEUEPAIR;

   switch(cmd) {
   case CFG_INIT:

      if(error=uiomove((caddr_t)&tmp_dds, sizeof(spty_dds), UIO_WRITE, uiop)) {
         break;
      }
      spty_dds=tmp_dds;

      /* sanity checks,device major given in sysconfig is same as 
         BSD master device major */
      if(spty_dds.ptyp_dev != dev) {

         /* Exit function */
         PTY_UNLOCKL(&lockl_ptp);
         PTY_TRACE_OUT(trace_out = out_2);
         /* Invalid argument */
         PTY_PRINT_OUT(trace_out,"ptsm_config",EINVAL); 
         Return(EINVAL);
      }

      /* If CFG_INIT has already been done,config_max_pts and 
         config_max_ttyp may have changed */
      if(pty_s_head_ATT && pty_s_head_BSD) {

         /* Check AT&T device */
         if(NRPTY(ATT)<config_max_pts) {

            /* Check if a device over new config_max_pts is opened */
            pty_o=pty_s_head_ATT;

            while(pty_o) {
               if(pty_o->pt_o_flag &&  pty_o->pt_o_minor>=NRPTY(ATT)) {

                  /* Exit function */
                  PTY_UNLOCKL(&lockl_ptp); 
                  PTY_TRACE_OUT(trace_out = out_3);
                  /* Operation not permitted */
                  PTY_PRINT_OUT(trace_out,"ptsm_config",EPERM); 
                  Return(EPERM);
               }
               pty_o=pty_o->pt_o_next;
            }
         }

         /* check BSD device */                   
         pty_o=pty_s_head_BSD;
         if(NRPTY(BSD)<config_max_ttyp) {

            /* Check if a device over new config_max_pts is opened */
            while(pty_o) {
               if(pty_o->pt_o_flag &&  pty_o->pt_o_minor>=NRPTY(BSD)) {

                  /* Exit function */
                  PTY_UNLOCKL(&lockl_ptp); 
                  PTY_TRACE_OUT(trace_out = out_4);
                  /* Operation not permitted */
                  PTY_PRINT_OUT(trace_out,"ptsm_config",EPERM); 
                  Return(EPERM);
               }
               pty_o=pty_o->pt_o_next;
            }
         }
   
         /* Save new value of NRPTY */
         config_max_pts=NRPTY(ATT);
         config_max_ttyp=NRPTY(BSD);

         /* Exit function */
         PTY_UNLOCKL(&lockl_ptp);
         PTY_TRACE_OUT(trace_out = out_5);
         PTY_PRINT_OUT(trace_out,"ptsm_config",0);
         Return(0);
      }
      /* Allocate memory for first device AT&T opened */ 
      PTY_UNLOCKL(&lockl_ptp); 
      if(!(pty_s_head_ATT=(struct pty_o_s *)he_alloc(sizeof(struct pty_o_s),
                                                     BPRI_MED))) {

         /* Exit function */
         PTY_TRACE_OUT(trace_out = out_6);
         PTY_PRINT_OUT(trace_out,"ptsm_config",ENOMEM); /* Not enough space */
         Return(ENOMEM);
      }
      bzero(pty_s_head_ATT,sizeof(struct pty_o_s));

      /* Allocate memory for first device BSD opened */ 
      if(!(pty_s_head_BSD=(struct pty_o_s *)he_alloc(sizeof(struct pty_o_s),
                                                     BPRI_MED))) {

         /* Exit function */
         PTY_TRACE_OUT(trace_out = out_7);
         PTY_PRINT_OUT(trace_out,"ptsm_config",ENOMEM); /* Not enough space */
         Return(ENOMEM);
      }
      bzero(pty_s_head_BSD,sizeof(struct pty_o_s));

      PTY_LOCKL(&lockl_ptp); 
      confptc.sc_major=major(spty_dds.ptc_dev);
      confpts.sc_major=major(spty_dds.pts_dev);
      confptyp.sc_major=major(spty_dds.ptyp_dev);
      confttyp.sc_major=major(spty_dds.ttyp_dev);

      /* load AT&T slave */
      error=str_install(STR_LOAD_DEV,&confpts);

      /* load AT&T master if slave loaded */
      if(!error) {
         error=str_install(STR_LOAD_DEV,&confptc);
         if(error)(void)str_install(STR_UNLOAD_DEV,&confptc);
         else {

            /* load BSD slave */
            error=str_install(STR_LOAD_DEV,&confttyp);

            /* load BSD master if slave loaded */
            if(!error) {
               error=str_install(STR_LOAD_DEV,&confptyp);
               if(error)(void)str_install(STR_UNLOAD_DEV,&confptyp);
            }
         }
      }
      
      if(error) {
         PTY_TRACE_OUT(trace_out = out_8);
         break;
      }

      /* Everything is ok,pty module is loaded  */
      config_max_pts=NRPTY(ATT);
      config_max_ttyp=NRPTY(BSD);
      PTY_UNLOCKL(&lockl_ptp);

#ifdef TTYDBG
      /* Call ttydbg for debug and dump services */
      tty_db_register(&ttydbg_conf);
#endif /* TTYDBG */

      /* Exit function */
      PTY_TRACE_OUT(trace_out = out_9);
      PTY_PRINT_OUT(trace_out,"ptsm_config",0);
      Return(0);

   case CFG_TERM:
                
      /* Check coherency */
      if(!pty_s_head_ATT || !pty_s_head_BSD) {

         /* Exit function */
         PTY_UNLOCKL(&lockl_ptp); 
         PTY_TRACE_OUT(trace_out = out_10);
         /* Operation not permitted */
         PTY_PRINT_OUT(trace_out,"ptsm_config",EPERM); 
         Return(EPERM);
      }

      /* Check if no device is opened */
      /* Check AT&T */
      pty_o=pty_s_head_ATT;
      while(pty_o) {
         if(pty_o->pt_o_flag) {

            /* Exit function */
            PTY_UNLOCKL(&lockl_ptp); 
            PTY_TRACE_OUT(trace_out = out_11);
            /* Operation not permitted */
            PTY_PRINT_OUT(trace_out,"ptsm_config",EBUSY); 
            Return(EBUSY);
         }
         pty_o=pty_o->pt_o_next;
      }


      /* Check BSD */
      pty_o=pty_s_head_BSD;
      while(pty_o) {
         if(pty_o->pt_o_flag) {

            /* Exit function */
            PTY_UNLOCKL(&lockl_ptp); 
            PTY_TRACE_OUT(trace_out = out_12);
            /* Operation not permitted */ 
            PTY_PRINT_OUT(trace_out,"ptsm_config",EBUSY); 
            Return(EBUSY);
         }
         pty_o=pty_o->pt_o_next;
      }

      confptc.sc_major=major(spty_dds.ptc_dev);
      confpts.sc_major=major(spty_dds.pts_dev);
      confptyp.sc_major=major(spty_dds.ptyp_dev);
      confttyp.sc_major=major(spty_dds.ttyp_dev);

      error=str_install(STR_UNLOAD_DEV,&confpts);
      error |= str_install(STR_UNLOAD_DEV,&confptc);
      error |= str_install(STR_UNLOAD_DEV,&confptyp);
      error |= str_install(STR_UNLOAD_DEV,&confttyp);
      PTY_TRACE_OUT(trace_out = out_13);

      if(!error) {
         PTY_UNLOCKL(&lockl_ptp);

         /* for performance free lock if lock_alloc called once */
         if(perf_lock) lock_free(&lock_open_close);

#ifdef TTYDBG
         /* Call ttydbg for debug and dump services */
         tty_db_unregister(&ttydbg_conf);
#endif /* TTYDBG */

         PTY_PRINT_OUT(trace_out,"ptsm_config",error);
         Return(0);
      }

   default:
      error=EINVAL; /* Invalid argument */
   }

   
   /* Exit function */ 
   PTY_UNLOCKL(&lockl_ptp); 
   PTY_PRINT_OUT(trace_out,"ptsm_config",error);
   Return(error);
}

/*        --- end of ptsm_config() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pts_open()                                        */
/*                                                                     */
/*        PARAMETER :  *q         : stream queue pointer               */
/*                     *devp      : slave dev pair                     */
/*                     flag       :                                    */
/*                     sflag      :                                    */
/*                     *credp     : credential pointer                 */
/*                                                                     */
/*        RETURN : error if pts_open failed 0 otherwise                */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        pts_open() opens slave device.                               */
/*                                                                     */
/***********************************************************************/
  
static int
pts_open(
         queue_t *q,
         dev_t   *devp,
         int     flag,
         int     sflag,
         cred_t  *credp
        )

{ 
   register struct pty_s *ptp;
   mblk_t                *mp;
   struct stroptions     *option;
   int                   error=0;
   struct tty_to_reg       *ttydbg_decl;
   INT_TRACE_OUT
   int                   mode; /* AT&T or BSD */
   INT_PTYSAVPRI_LRC;

   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_OPEN,*devp,0,flag,sflag,0);
   PTY_PRINT_IN("pts_open IN",NULL);

   PTY_ASSERT(q != NULL);
   PTY_ASSERT(devp != NULL);

   /* for performance we have to use lock_alloc */
   if(!perf_lock) {
      lock_alloc(&lock_open_close,LOCK_ALLOC_PIN,PTY_LOCK_CLASS,ident++);
      perf_lock=1;
   }

   /* The slave cannot be cloned. */ 
   if(sflag == CLONEOPEN) {

      /* Exit function */
      PTY_PRINT_OUT(trace_out,"pts_open",ENXIO); /* No such device or address */
      Return(ENXIO);
   }

   PTY_PRINT("pts_open : Read mode");
   /* Read mode */
   if(major(*devp) == major(spty_dds.ttyp_dev)) {
      PTY_PRINT("pts_open : BSD mode");
      mode=BSD;
   }
   else {
      PTY_PRINT("pts_open : ATT mode or error");
      mode=ATT;
   }

   /* Because we can't execute more than one pty_open_comm() and 
      pty_close_comm() routine at the same time, we serialize here 
      access to that routine (call_pty_close_comm of close side) */

   if(error=call_pty_open_comm(q,devp,mode,SLAVE_CALL)) {

      /* Exit function */
      PTY_TRACE_OUT(trace_out = out_2);
      PTY_PRINT_OUT(trace_out,"pts_open",error);
      Return(error);
   } 
   else {
 
      ptp=(struct pty_s *)q->q_ptr;
      PTY_ASSERT(ptp != NULL);
   }
   PTY_LOCK(&ptp->lock_ptp);

   /* In any case those values must be initialized here */
   ptp->pt_dev_slave=*devp;
   ptp->pt_open_count++;

   if((ptp->pt_flags & (PF_SOPEN | PF_MOPEN)) && 
      (ptp->pt_flags & PF_XCLUDE)) {
         if(credp->cr_uid != 0) {
            PTY_PRINT("PF_XCLUDE ON and credp->cr_uid != 0");
            error=EBUSY; /* Resouce busy */
            PTY_TRACE_OUT(trace_out = out_3);
            goto out;
         }
   }

   /* Wait for the master open. Should be waken up from master open 
    * or by a signal.
    *
    */
   if((flag & (O_NDELAY|O_NONBLOCK)) == 0) {
      for(;;) {
         if(ptp->pt_flags & PF_MOPEN) {
            break; 
         }
         ptp->pt_flags  |= PF_SWOPEN;
         PTY_UNLOCK(&ptp->lock_ptp);
         if(e_sleep(&ptp->pt_wait_master,EVENT_SIGRET) == EVENT_SIG) {
            error=EINTR; /* Interrupted system call */ 
            PTY_TRACE_OUT(trace_out = out_4);
            PTY_LOCK(&ptp->lock_ptp);
            goto out;
         } 
         PTY_LOCK(&ptp->lock_ptp);
      }
      ptp->pt_flags  &= ~PF_SWOPEN;
   }

out:
   PTY_UNLOCK(&ptp->lock_ptp);
   if(error) {
      PTY_LOCK(&ptp->lock_ptp);
      if (ptp->pt_open_count==1) {
         PTY_UNLOCK(&ptp->lock_ptp);
         call_pty_close_comm(q,SLAVE_CALL);
      }
      else {
         ptp->pt_open_count--;
         PTY_UNLOCK(&ptp->lock_ptp);
      }

      /* Exit function */
      PTY_PRINT_OUT(trace_out,"pts_open",error);
      Return(error);
   }
   else {
      if(!(mp=allocb(sizeof(struct stroptions),BPRI_MED))) {
         PTY_LOCK(&ptp->lock_ptp);
         if (ptp->pt_open_count==1) {
            PTY_UNLOCK(&ptp->lock_ptp);
            call_pty_close_comm(q,SLAVE_CALL);
         }
         else {
            ptp->pt_open_count--;
            PTY_UNLOCK(&ptp->lock_ptp);
         }
         error=ENOMEM; /* Not enough space */
         PTY_TRACE_OUT(trace_out = out_5);
         PTY_PRINT_OUT(trace_out,"pts_open",error);
         Return(error);
      }

#ifdef TTYDBG
      /* Call ttydbg for debug and dump services */
      if (!(ttydbg_decl=(struct tty_to_reg *)he_alloc(sizeof(struct tty_to_reg),
                                                      BPRI_MED))) {
         PTY_LOCK(&ptp->lock_ptp);
         if (ptp->pt_open_count==1) {
            PTY_UNLOCK(&ptp->lock_ptp);
            call_pty_close_comm(q,SLAVE_CALL);
         }
         else {
            ptp->pt_open_count--;
            PTY_UNLOCK(&ptp->lock_ptp);
         }
         error=ENOMEM; /* Not enough space */
         PTY_TRACE_OUT(trace_out = out_6);
         PTY_PRINT_OUT(trace_out,"pts_open",error);
         Return(error);
      }

      /* pin memory */
      if(error = pin(ttydbg_decl, sizeof(struct tty_to_reg))) {

         PTY_LOCK(&ptp->lock_ptp);
         if (ptp->pt_open_count==1) {
            PTY_UNLOCK(&ptp->lock_ptp);
            call_pty_close_comm(q,SLAVE_CALL);
         }
         else {
            ptp->pt_open_count--;
            PTY_UNLOCK(&ptp->lock_ptp);
         }

         /* Exit function */
         PTY_TRACE_OUT(trace_out = out_7);

         /* error return by pin */
         PTY_PRINT_OUT(trace_out,"pts_open",error);
         Return(error);
     }

     ttydbg_decl->dev=ptp->pt_dev_slave;
     sprintf(ttydbg_decl->ttyname,"%s",ptp->pt_name);
     strcpy(ttydbg_decl->name,COMPONENT_NAME);
     ttydbg_decl->private_data=ptp;

     PTY_LOCK(&ptp->lock_ptp);
     ptp->ttydbg_decl=ttydbg_decl;
     PTY_UNLOCK(&ptp->lock_ptp);

     tty_db_open(ttydbg_decl);
#endif /* TTYDBG */

      mp->b_datap->db_type=M_SETOPTS;
      mp->b_wptr=mp->b_rptr + sizeof(struct stroptions);
      option=(struct stroptions *)mp->b_rptr;
      option->so_flags=SO_READOPT | SO_ISTTY;
      option->so_readopt=RMSGN | RPROTDAT;

      PTY_LOCK(&ptp->lock_ptp);
      ptp->pt_flags  |= PF_SOPEN;
      PTY_UNLOCK(&ptp->lock_ptp);

      if(pty_pts_xfer(ptp,mp)) freemsg(mp);

      /* Exit function */
      PTY_TRACE_OUT(trace_out = out_8);
      PTY_PRINT_OUT(trace_out,"pts_open",error);
      Return(0);
   }

}

/*        --- end of pts_open() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pts_close()                                       */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     flag    :                                       */
/*                     *credp  : credential pointer                    */
/*                                                                     */
/*        RETURN : 0                                                   */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        pts_close() closes slave device.                             */
/*                                                                     */
/***********************************************************************/

static int
pts_close(
          queue_t  *q,
          int      flag,
          cred_t   *credp
        )

{
   register struct pty_s *ptp = (struct pty_s *)q->q_ptr;
   register mblk_t       *eof_msg;
   struct tty_to_reg     *ttydbg_decl;
   INT_TRACE_OUT
   INT_PTYSAVPRI_LRC;

   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_CLOSE,ptp->pt_dev_slave,(int)ptp,flag,0,0);
   PTY_PRINT_IN("pts_close IN",NULL);

   PTY_ASSERT(q != NULL);
   PTY_ASSERT(ptp != NULL);

   qenable(ptp->pt_swq);
   qenable(ptp->pt_srq);
   PTY_LOCK(&ptp->lock_ptp);

   /* Send EOF to master -- this will have the side effect
    * of waking up any sleeping reads.
    */

   if((ptp->pt_flags & PF_MOPEN)){ /* check if master still alive */
      /* Get a zero length M_DATA */
      if(eof_msg = allocb(0, BPRI_WAITOK)) {
         PTY_UNLOCK(&ptp->lock_ptp);
         ptm_put(ptp,eof_msg,PUTNEXT);
         PTY_LOCK(&ptp->lock_ptp);
      }
   }

   /* Drain output before closing */
   while((ptp->pt_flags & PF_MOPEN) && ptp->pt_swq->q_first) {
      qenable(ptp->pt_mrq);
      ptp->pt_flags  |= PF_SWDRAIN;
      PTY_UNLOCK(&ptp->lock_ptp);
      if(e_sleep(&ptp->pt_swait_drain,
         EVENT_SIGRET) == EVENT_SUCC) {
         PTY_LOCK(&ptp->lock_ptp);
         break;
      }
      else {
         PTY_LOCK(&ptp->lock_ptp);
      }
   }
   ptp->pt_flags  &= ~PF_SWDRAIN;

   /* Wait to exit pts_put() routine */
   while(ptp->pt_flags & PF_PTS_XFER) {
      ptp->pt_flags |= PF_SWXFER;
      PTY_UNLOCK(&ptp->lock_ptp);
      if(e_sleep(&ptp->pt_swait_xfer, EVENT_SIGRET) == EVENT_SUCC) {
         PTY_LOCK(&ptp->lock_ptp);
         break;
      }
      else {
         PTY_LOCK(&ptp->lock_ptp);
      }
   }
   ptp->pt_flags  &= ~PF_SWXFER;
   ptp->pt_flags  &= ~(PF_SOPEN | PF_SFLOW);
   PTY_UNLOCK(&ptp->lock_ptp);

#ifdef TTYDBG
   /* Call ttydbg for debug and dump services */
   ttydbg_decl=ptp->ttydbg_decl;
   tty_db_close(ttydbg_decl);
   unpin(ttydbg_decl,sizeof(struct tty_to_reg));
   he_free(ttydbg_decl);
#endif /* TTYDBG */

   (void)call_pty_close_comm(q,SLAVE_CALL);

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_2);
   PTY_PRINT_OUT(trace_out,"pts_close",0);
   Return(0);

}

/*        --- end of pts_close() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pts_wput()                                        */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     *mp     : message block pointer                 */ 
/*                                                                     */
/*        RETURN : 0                                                   */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Slave write put routine.                                     */
/*        The following message are processed :                        */
/*                  M_IOCTL                                            */
/*                  M_FLUSH                                            */
/*                  M_DATA                                             */
/*                  M_START                                            */
/*                  M_STOP                                             */
/*                  M_STARTI                                           */ 
/*                  M_STOPI                                            */
/*                  M_CTL                                              */
/*                  default : free message.                            */
/*                                                                     */
/***********************************************************************/

static int
pts_wput(
         queue_t *q,
         mblk_t  *mp
        )

{
   register struct pty_s  *ptp = (struct pty_s *)q->q_ptr;
   int                    size;
   int                    error; /* dummy */
   INT_TRACE_OUT
   INT_PTYSAVPRI_LRC;

   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_WPUT,ptp->pt_dev_slave,(int)ptp,mp,
         mp->b_datap->db_type,0);
   PTY_PRINT_IN("pts_wput IN",NULL);
   PTY_PRINT_MBLK_T("pts_wput",mp);

   PTY_ASSERT(q != NULL);
   PTY_ASSERT(mp != NULL);
   PTY_ASSERT(ptp != NULL);


   switch(mp->b_datap->db_type) {

      case M_DATA:
         PTY_PRINT("pts_wput : M_DATA");

         if(!ptp->pt_swq->q_first) {
            if(pty_ptm_xfer(ptp,mp)) {
               putq(ptp->pt_swq,mp);
            }
         }
         else putq(ptp->pt_swq,mp);
         PTY_TRACE_OUT(trace_out = out_2);
         break;

      case M_IOCTL:
         switch(((struct iocblk *)mp->b_rptr)->ioc_cmd) {
            case TCSBRK:    /* this wait output to drain */
            case TCSBREAK:  /* this wait output to drain */
            case TIOCSETAW: /* this wait output to drain */
            case TIOCSETAF: /* this wait output to drain */

               /* just queue them in order */
               putq(ptp->pt_swq,mp);
               PTY_TRACE_OUT(trace_out = out_3);
               break;

               /* else (other IOCTLs) falls through */
            default:
               pty_strioctl_comm(q,mp,ptp,SLAVE_CALL);
               qreply(q,mp);
               PTY_TRACE_OUT(trace_out = out_4);
         } 
         break;

      case M_FLUSH:
         PTY_LOCK(&ptp->lock_ptp);
         if (ptp->pt_flags & PF_PKT) {
            if (*mp->b_rptr & FLUSHW)
               ptp->pt_send |= TIOCPKT_FLUSHWRITE;
            if (*mp->b_rptr & FLUSHR) 
               ptp->pt_send |= TIOCPKT_FLUSHREAD;
         }
         PTY_UNLOCK(&ptp->lock_ptp);


         if(*mp->b_rptr & FLUSHW)  flushq(ptp->pt_swq,FLUSHDATA);
         if(*mp->b_rptr & FLUSHR) {
            flushq(ptp->pt_srq,FLUSHDATA); 

            PTY_LOCK(&ptp->lock_ptp);
            if (ptp->pt_flags & PF_MOPEN) {
               PTY_UNLOCK(&ptp->lock_ptp);
               qenable(ptp->pt_mwq);
            }
            else PTY_UNLOCK(&ptp->lock_ptp);
         }

         if(pty_ptm_xfer(ptp,mp)) freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_5);
         break;

      case M_START:
         PTY_LOCK(&ptp->lock_ptp);
         if(ptp->pt_flags & PF_PKT) {

            /* Mode packet */
            ptp->pt_send  &= ~TIOCPKT_STOP;
            ptp->pt_send  |= TIOCPKT_START;
         }

         ptp->pt_flags  &= ~ PF_TTSTOP; /* start slave --> master */
         PTY_UNLOCK(&ptp->lock_ptp);
 

         /* wake up everybody ! */
         if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_swq);
         if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mrq);
         if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mwq);
         if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_srq);

         freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_6);
         break;

      case M_STOP:
         PTY_LOCK(&ptp->lock_ptp);
         if(ptp->pt_flags & PF_PKT) {

            /* Mode packet */
            ptp->pt_send  &= ~TIOCPKT_START;
            ptp->pt_send  |= TIOCPKT_STOP;
         }
         ptp->pt_flags  |= PF_TTSTOP; /* stop slave --> master */
         PTY_UNLOCK(&ptp->lock_ptp);


         freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_7);
         break;

      case M_STARTI:
         PTY_LOCK(&ptp->lock_ptp);
         ptp->pt_flags  &= ~ PF_TTINSTOP; /* start master --> slave */
         PTY_UNLOCK(&ptp->lock_ptp);

         /* wake up everybody ! */
         if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_swq);
         if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mrq);
         if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mwq);
         if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_srq);

         freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_8);
         break;

      case M_STOPI:
         PTY_LOCK(&ptp->lock_ptp);
         ptp->pt_flags  |= PF_TTINSTOP; /* stop master --> slave */
         PTY_UNLOCK(&ptp->lock_ptp);
         freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_9);
         break;

      case M_CTL:
         if(size=pty_mctl(q,mp,SLAVE_CALL)) {
            putq(ptp->pt_swq,mp);
         }
         PTY_TRACE_OUT(trace_out = out_10);
         break;

      default:
         freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_11);
         break; 
   }


   /* Exit function */
   PTY_PRINT_OUT(trace_out,"pts_wput",0);
   Return(0);
}

/*        --- end of pts_wput() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pts_wsrv()                                        */
/*                                                                     */
/*        PARAMETER :  *q        : streams queue pointer               */
/*                                                                     */
/*        RETURN : FLOW_CONTROL if slave output flow control           */ 
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        The following messages are processed :                       */
/*                  M_DATA                                             */
/*                  M_CTL                                              */
/*                  M_IOCTL                                            */
/*                  default : free message                             */
/*                                                                     */
/***********************************************************************/

static int
pts_wsrv(
          queue_t *q
         )

{
   register struct pty_s  *ptp = (struct pty_s *)q->q_ptr;
   mblk_t                 *mp;
   mblk_t                 *wait_drain;
   int                    size;
   int			  error;
   INT_TRACE_OUT
   INT_PTYSAVPRI_LRC;


   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_WSRV,ptp->pt_dev_slave,(int)ptp,q->q_count,0,0);
   PTY_PRINT_IN("pts_wsrv IN",NULL);

   PTY_ASSERT(q != NULL);
   PTY_ASSERT(ptp != NULL);

   while(mp=getq(q))
   {
      switch(mp->b_datap->db_type) {
      case M_DATA:

         /* Packet mode */
         if(((ptp->pt_flags & PF_PKT) || (ptp->pt_flags & PF_UCNTL)) &&
            (ptp->pt_flags & PF_MOPEN) &&
            (ptp->pt_send || ptp->pt_ucntl))
               ptm_send_pckt(ptp->pt_mrq);


         PTY_PRINT("pts_wsrv : M_DATA");
         if(error=pty_ptm_xfer(ptp,mp)) {
            if(error==ENODEV) freemsg(mp);
            else {

               /* flow control */
               /* Master will qenable/wake us when flow is back on.  */
               putbq(q,mp);
                                   
               /* Exit function */
               /* Flow control */
               PTY_PRINT_OUT(trace_out,"pts_wrsv",FLOW_CONTROL);
               Return(FLOW_CONTROL);
            }
         }
         continue;

      case M_CTL:
         PTY_LOCK(&ptp->lock_ptp);
         if (!(ptp->pt_flags & PF_SWDRAIN)) {
            PTY_UNLOCK(&ptp->lock_ptp);
            if(size=pty_mctl(q,mp,SLAVE_CALL)) {
                  putbq(q,mp);    
   
                  /* Exit function */
                  PTY_TRACE_OUT(trace_out = out_2);
                  PTY_PRINT_OUT(trace_out,"pts_wsrv",0);
                  Return(0);
            }
         }
         else {
            PTY_UNLOCK(&ptp->lock_ptp);
         }
         continue;

      case M_IOCTL:
         switch(((struct iocblk *)mp->b_rptr)->ioc_cmd) {
            case TCSBRK:    /* this wait output to drain */
            case TCSBREAK:  /* this wait output to drain */
            case TIOCSETAW: /* this wait output to drain */
            case TIOCSETAF: /* this wait output to drain */

	       /* Sends a M_RSE message to the master and turns on pt_flag 
		  with PF_RSE. When master receives M_PCRSE message it turns 
		  on pt_flag with PF_RSE_OK telling to slave that data 
		  are drained, and it wakes up slave write service routine */

		/* First case, a M_RSE must be sent */
		PTY_LOCK(&ptp->lock_ptp);
		if (!(ptp->pt_flags & PF_M_RSE)) {
		   PTY_UNLOCK(&ptp->lock_ptp);

		   /* Send a M_RSE message */
		   if(!(wait_drain=allocb(0, BPRI_WAITOK))) {
                      putbq(ptp->pt_swq,mp);

                   /* Exit function */
                   PTY_TRACE_OUT(trace_out = out_3);
                   PTY_PRINT_OUT(trace_out,"pts_wsrv",0);
                   Return(0);
                   }

		   wait_drain->b_datap->db_type=M_RSE;
		   wait_drain->b_flag |= MSDRAIN;
                   PTY_LOCK(&ptp->lock_ptp);
		   ptp->pt_flags |=PF_M_RSE;
                   PTY_UNLOCK(&ptp->lock_ptp);
		   ptm_put(ptp,wait_drain,PUTNEXT);

		   /* We will be scheduled later by the master */
		   putbq(ptp->pt_swq,mp);

   		   /* Exit function */
   		   PTY_TRACE_OUT(trace_out = out_4);
   		   PTY_PRINT_OUT(trace_out,"pts_wsrv",0);
   		   Return(0);
		}

		/* Second case, a M_PCRSE message has been received */
		else if((ptp->pt_flags & PF_RSE_OK)) {
		        ptp->pt_flags &= ~(PF_M_RSE | PF_RSE_OK);
			PTY_UNLOCK(&ptp->lock_ptp);
			/* and falls through default case */
                     }

                /* third case, still waiting for M_PCRSE message */
		else { /* PF_M_RSE and !PF_RSE_OK */                   
		   /* We will be scheduled later by the master */
                   PTY_UNLOCK(&ptp->lock_ptp);
                   putbq(ptp->pt_swq,mp);

                   /* Exit function */
                   PTY_TRACE_OUT(trace_out = out_5);
                   PTY_PRINT_OUT(trace_out,"pts_wsrv",0);
                   Return(0);
                }

               /* else falls through */
            default:
               pty_strioctl_comm(q,mp,ptp,SLAVE_CALL);
               qreply(q,mp);
         }
         continue; 

      default:
         freemsg(mp);
         continue;
      }
   }

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_6);
   PTY_PRINT_OUT(trace_out,"pts_wsrv",0);
   Return(0);
}

/*        --- end of pts_wsrv() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pts_rput()                                        */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     *mp     : message block pointer                 */
/*                                                                     */
/*        RETURN : 0                                                   */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Slave read put routine.                                      */
/*        The following messages are processed :                       */
/*                  M_DATA                                             */
/*                  M_FLUSH                                            */
/*                  default : send to slave read service               */ 
/*                                                                     */
/***********************************************************************/

static int
pts_rput(
         queue_t *q,
         mblk_t  *mp 
        )
{

   register struct pty_s *ptp = (struct pty_s *)q->q_ptr;
   INT_TRACE_OUT
   INT_PTYSAVPRI_LRC;



   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_RPUT,ptp->pt_dev_slave,(int)ptp,mp,
         mp->b_datap->db_type,0);
   PTY_PRINT_IN("pts_rput IN",NULL);
   PTY_PRINT_MBLK_T("pts_rput",mp);

   PTY_ASSERT(q != NULL);
   PTY_ASSERT(mp != NULL);
   PTY_ASSERT(ptp != NULL);


   switch(mp->b_datap->db_type) {

      case M_DATA : 

         PTY_LOCK(&ptp->lock_ptp);
         if((ptp->pt_flags & PF_MFLOW) && 
            (ptp->pt_srq->q_count <= ptp->pt_srq->q_lowat)) {
            ptp->pt_flags  &= ~PF_MFLOW;
            if(ptp->pt_flags & PF_MOPEN) {
               PTY_UNLOCK(&ptp->lock_ptp);
               qenable(ptp->pt_mwq);
               PTY_LOCK(&ptp->lock_ptp);
            }
         }
         PTY_UNLOCK(&ptp->lock_ptp);


         PTY_PRINT("pts_rput : M_DATA");

         if((!ptp->pt_srq->q_first) && (canput(ptp->pt_srq->q_next))) {
               putnext(ptp->pt_srq,mp);
            }
         else putq(ptp->pt_srq,mp);
         break;

      case M_FLUSH:
         if(*mp->b_rptr & FLUSHR) {
            flushq(ptp->pt_srq,FLUSHDATA);

            PTY_LOCK(&ptp->lock_ptp);
            if (ptp->pt_flags & PF_MOPEN) {
               PTY_UNLOCK(&ptp->lock_ptp);
               qenable(ptp->pt_mwq);
            }
            else PTY_UNLOCK(&ptp->lock_ptp);

         }
         if(*mp->b_rptr & FLUSHW)flushq(ptp->pt_swq,FLUSHDATA); 
         if(canput(ptp->pt_srq->q_next)) putnext(ptp->pt_srq,mp); 
         else putq(ptp->pt_srq,mp);
         PTY_TRACE_OUT(trace_out = out_2);
         break; 

      default:
         if((mp->b_datap->db_type >= QPCTL) || (!ptp->pt_srq->q_first && 
            canput(ptp->pt_srq->q_next)))putnext(ptp->pt_srq,mp);
         else putq(ptp->pt_srq,mp);
         PTY_TRACE_OUT(trace_out = out_3);
         break;
   }

   /* Exit function */
   PTY_PRINT_OUT(trace_out,"pts_rput",0);
   Return(0);
}

/*        --- end of pts_rput() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pts_rsrv()                                        */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     *mp     : message block pointer                 */
/*                                                                     */
/*        RETURN : 0                                                   */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Slave read service.                                          */
/*        All messages received are put next, and flow control is done */
/*        updating pt_flags with PF_MFLOW as soon as q_count < q_lowat */  
/*                                                                     */
/***********************************************************************/

static int
pts_rsrv(
         queue_t *q
        )

{
   register struct pty_s *ptp = (struct pty_s *)q->q_ptr;
   register mblk_t       *mp;
   INT_PTYSAVPRI_LRC;
   INT_TRACE_OUT

   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_RSRV,ptp->pt_dev_slave,(int) ptp,q->q_count,0,0);
   PTY_PRINT_IN("pts_rsrv IN",NULL);

   PTY_ASSERT(q != NULL);
   PTY_ASSERT(ptp != NULL);


   while(mp=getq(q)) {
      if(mp->b_datap->db_type >= QPCTL || canput(ptp->pt_srq->q_next)) {
         putnext(ptp->pt_srq,mp);
      }
      else {
         putbq(ptp->pt_srq,mp);
                           
         /* Exit function */
         PTY_TRACE_OUT(trace_out = out_2);
         PTY_PRINT_OUT(trace_out,"pts_rsrv",0);
         Return(0);
      }
   }

   /* flow control */
   PTY_LOCK(&ptp->lock_ptp);
   if((ptp->pt_flags & PF_MFLOW) && 
      (ptp->pt_srq->q_count < ptp->pt_srq->q_lowat)) {
      ptp->pt_flags  &= ~PF_MFLOW;
      if (ptp->pt_flags & PF_MOPEN) {
         PTY_UNLOCK(&ptp->lock_ptp);
         qenable(ptp->pt_mwq);
         PTY_LOCK(&ptp->lock_ptp);
      }
   }
   if(ptp->pt_flags & PF_MWDRAIN) {
      PTY_UNLOCK(&ptp->lock_ptp);
      e_wakeup(&ptp->pt_mwait_drain);
      PTY_LOCK(&ptp->lock_ptp);
   }
   PTY_UNLOCK(&ptp->lock_ptp);

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_3);
   PTY_PRINT_OUT(trace_out,"pts_rsrv",0);
   Return(0);

}

/*        --- end of pts_rsrv() function ---        */

/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_setnameattr()                                 */
/*								       */
/*        PARAMETERS: pathname : name of the file		       */
/*                    cmd : desired operation			       */
/*                    arg1 					       */
/*                    arg2					       */
/*                    arg3  					       */
/*								       */
/*        RETURN VALUES: returns any error codes		       */
/*								       */
/*        COMMENT: Same function as defined in kernel/lfs/setattr.c    */
/*                 parameter USR is just changed to SYS in lookupname  */
/*								       */
/***********************************************************************/
static int
pty_setnameattr(char *pathname, 
		int cmd, 
		int arg1, 
		int arg2, 
		int arg3, 
		int lflag)
{
   register int rc;
   struct vnode *vp;
   static int tcbmod = 0;
   struct ucred *crp;
   INT_TRACE_OUT
   uid_t  temp_cr_ruid; 
   uid_t  temp_cr_uid;

   /* trace in */
   PTY_PRINT_IN("pty_setnameattr IN",NULL);

   /* get current credentials and take root privilege */
   crp = crref();
   temp_cr_ruid=crp->cr_ruid;
   temp_cr_uid=crp->cr_uid;
   crp->cr_ruid=crp->cr_uid=0;

   /* let's look up the name and get the vnode for it */
   rc = lookupname(pathname, SYS, L_SEARCH|lflag, NULL, &vp, crp);

   if (rc == 0)
   {
      /* we have a vnode, let's set the attributes */
      rc = pty_vsetattr(vp, cmd, arg1, arg2, arg3, crp);

      /* release the vnode and return any code */
      VNOP_RELE(vp);
   }

   crp->cr_ruid=temp_cr_ruid;
   crp->cr_uid=temp_cr_uid;
   crfree(crp);

   /* Exit function */
   PTY_PRINT_OUT(trace_out,"pty_setnameattr",rc);
   return (rc);
}

/*        --- end of pty_setnameattr() function ---        */

/***********************************************************************/
/*								       */
/* 	FUNCTION: pty_vsetattr()				       */
/*								       */
/* 	PARAMETERS:  Same as the VNOP_SETATTR.			       */
/*								       */
/* 	RETURNS:     Same as VNOP_SETATTR.			       */
/*								       */
/* 	PURPOSE: To check for a read only file system and call the     */
/*               SETATTR vnode op (isolate it to one call).	       */
/*								       */
/***********************************************************************/
static int
pty_vsetattr(register struct vnode *vp, 
	     register int cmd, 
	     register int arg1, 
	     register int arg2, 
	     register int arg3, 
             struct ucred *crp)
{
   INT_TRACE_OUT
   static int tcbmod = 0;
   int rc;

   /* trace in */
   PTY_PRINT_IN("pty_vsetattr IN",NULL);

   if (vp->v_vfsp->vfs_flag & VFS_READONLY) {
      /* Exit function */
      PTY_PRINT_OUT(trace_out,"pty_vsetattr",EROFS);
      return(EROFS);
   }
   rc = VNOP_SETATTR(vp, cmd, arg1, arg2, arg3, crp);

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_2);
   PTY_PRINT_OUT(trace_out,"pty_vsetattr",rc);
   return(rc);
}

/*        --- end of pty_vsetattr() function ---        */

/***********************************************************************/
/*                                                                     */
/*        FUNCTION : ptm_open()                                        */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     *devp   : master dev pair                       */
/*                     flag    :                                       */
/*                     sflag   :                                       */
/*                     *credp  : credential pointer                    */
/*                                                                     */
/*        RETURN : error on failure 0 otherwise                        */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        open master device.                                          */
/*                                                                     */
/***********************************************************************/

static int
ptm_open(
         queue_t *q,
         dev_t   *devp,
         int     flag,
         int     sflag,
         cred_t  *credp 
        )

{
   mblk_t                  *mp;
   struct stroptions       *option;
   register struct pty_s   *ptp;
   int                     error=0;
   int                     mode;
   INT_TRACE_OUT
   char                    *data;
   INT_PTYSAVPRI_LRC;

   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_OPEN,*devp,0,flag,sflag,0);
   PTY_PRINT_IN("ptm_open IN",NULL);

   PTY_ASSERT(q !=NULL);
   PTY_ASSERT(devp != NULL);

   /* Read mode */
   if(major(*devp) == major(spty_dds.ptyp_dev))mode=BSD;
   else mode=ATT;

   /* check coherence with sflag */
   if(mode == BSD && (sflag == CLONEOPEN || sflag == MODOPEN)) {

      /* Exit function */
      PTY_PRINT_OUT(trace_out,"ptm_open",EPERM); /* Operation not permitted */
      Return(EPERM);
   }

   /* Because we can't execute more than one pty_open_comm() and 
      pty_close_comm() routine at the same time, we serialize here 
      access to that routine (call_pty_close_comm on close side) */

   if(error=call_pty_open_comm(q,devp,mode,MASTER_CALL)) {

      /* Exit function */
      PTY_TRACE_OUT(trace_out = out_2);
      PTY_PRINT_OUT(trace_out,"ptm_open",error);
      Return(error);
   } 
   else {

      ptp=(struct pty_s *)q->q_ptr;
      PTY_ASSERT(ptp != NULL);
   }  

   PTY_LOCK(&ptp->lock_ptp);
   ptp->pt_flags  |= PF_MOPEN; /* WARNING! This flag Must be turned on here */ 

   /* Wakeup slave open waiting for the master side open.  */
   if(ptp->pt_flags & PF_SWOPEN) {
      PTY_UNLOCK(&ptp->lock_ptp);
      e_wakeup(&ptp->pt_wait_master);
      
   }
   else {
      PTY_UNLOCK(&ptp->lock_ptp);
   }

   if(!(mp=allocb(sizeof(struct stroptions),BPRI_MED))) {

      /* Master can't be open twice, it's not necessary to
         check whether it's a reopen or not */

      PTY_LOCK(&ptp->lock_ptp);

      /* Wait to exit ptm_put() routine */
      while(ptp->pt_flags & PF_PTM_XFER) {
         ptp->pt_flags |= PF_MWXFER;
         PTY_UNLOCK(&ptp->lock_ptp);
         if(e_sleep(&ptp->pt_mwait_xfer, EVENT_SIGRET) == EVENT_SUCC) {
            PTY_LOCK(&ptp->lock_ptp);
            break;
         }
         else {
            PTY_LOCK(&ptp->lock_ptp);
         }
      }
      ptp->pt_flags  &= ~PF_MWXFER;
      ptp->pt_flags  &= ~(PF_MOPEN | PF_MFLOW);
      PTY_UNLOCK(&ptp->lock_ptp);

      call_pty_close_comm(q,MASTER_CALL);
      error=ENOMEM; /* Not enough space */

      PTY_TRACE_OUT(trace_out = out_4);
      PTY_PRINT_OUT(trace_out,"ptm_open",0);
      Return(error);
   }
   mp->b_datap->db_type=M_SETOPTS;
   mp->b_wptr=mp->b_rptr + sizeof(struct stroptions);
   option=(struct stroptions *)mp->b_rptr;
   option->so_flags=SO_READOPT | SO_ISTTY;
   option->so_readopt=RMSGN | RPROTDAT;


   if(pty_ptm_xfer(ptp,mp)) freemsg(mp);

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_5);
   PTY_PRINT_OUT(trace_out,"ptm_open",0);
   Return(0);
}

/*        --- end of ptm_open() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : ptm_close()                                       */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     flag    :                                       */
/*                     *credp  : credential pointer                    */
/*                                                                     */
/*        RETURN : 0 or error                                          */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/***********************************************************************/

static int
ptm_close(
          queue_t *q,
          int     flag,
          cred_t  *credp 
        )

{
   register struct pty_s *ptp = (struct pty_s *)q->q_ptr;
   register mblk_t       *bye_msg;
   INT_TRACE_OUT
   INT_PTYSAVPRI_LRC;

   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_CLOSE,ptp->pt_dev_slave,(int)ptp,flag,0,0);
   PTY_PRINT_IN("ptm_close IN",NULL);

   PTY_ASSERT(q != NULL);
   PTY_ASSERT(ptp != NULL);

   qenable(ptp->pt_mwq);
   qenable(ptp->pt_mrq);
   PTY_LOCK(&ptp->lock_ptp);

   /* Send M_HANGUP to slave */

   if((ptp->pt_flags & PF_SOPEN)){ /* check if slave still alive */

      if(bye_msg=allocb(0,BPRI_WAITOK)) {
         bye_msg->b_datap->db_type=M_HANGUP;
         PTY_UNLOCK(&ptp->lock_ptp);
         pts_put(ptp,bye_msg,PUTHERE);
         PTY_LOCK(&ptp->lock_ptp);
      }
   }

   /* Drain output before closing */
   while((ptp->pt_flags & PF_SOPEN) && ptp->pt_mwq->q_first) {
      qenable(ptp->pt_srq);
      ptp->pt_flags  |= PF_MWDRAIN;
      PTY_UNLOCK(&ptp->lock_ptp);
      if(e_sleep(&ptp->pt_mwait_drain,
         EVENT_SIGRET) == EVENT_SUCC) {
         PTY_LOCK(&ptp->lock_ptp);
         break;
      }
      else {
         PTY_LOCK(&ptp->lock_ptp);
      }
   }
   ptp->pt_flags  &= ~PF_MWDRAIN;

   /* Wait to exit ptm_put() routine */
   while(ptp->pt_flags & PF_PTM_XFER) {
      ptp->pt_flags |= PF_MWXFER;
      PTY_UNLOCK(&ptp->lock_ptp);
      if(e_sleep(&ptp->pt_mwait_xfer, EVENT_SIGRET) == EVENT_SUCC) {
         PTY_LOCK(&ptp->lock_ptp);
         break;
      }
      else {
         PTY_LOCK(&ptp->lock_ptp);
      }
   }

   ptp->pt_flags  &= ~PF_MWXFER;
   ptp->pt_flags  &= ~(PF_MOPEN | PF_MFLOW);
   PTY_UNLOCK(&ptp->lock_ptp);

   (void)call_pty_close_comm(q,MASTER_CALL);

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_2);
   PTY_PRINT_OUT(trace_out,"ptm_close",0);
   Return(0);
}

/*        --- End of ptm_close() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : ptm_wput()                                        */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     *mp     : message block pointer                 */
/*                                                                     */
/*        RETURN : 0                                                   */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Write put master.                                            */
/*        The following message are processed :                        */
/*                  M_IOCTL                                            */
/*		    M_PCRSE					       */
/*                  M_FLUSH                                            */
/*                  M_DATA                                             */
/*                  M_READ                                             */
/*                  M_START                                            */
/*                  M_STOP                                             */
/*                  M_STARTI                                           */
/*                  M_STOPI                                            */
/*                  M_CTL                                              */
/*                  default : free message.                            */
/*                                                                     */
/***********************************************************************/

static int
ptm_wput(
         queue_t *q,
         mblk_t  *mp 
        )

{
   register struct pty_s *ptp = (struct pty_s *)q->q_ptr;
   caddr_t         data;
   int             size;
   int             error = 0;
   INT_PTYSAVPRI_LRC;
   INT_TRACE_OUT


   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_WPUT,ptp->pt_dev_slave,(int)ptp,mp,
         mp->b_datap->db_type,0);
   PTY_PRINT_IN("ptm_wput IN",NULL);
   PTY_PRINT_MBLK_T("ptm_wput",mp);

   PTY_ASSERT(q != NULL);
   PTY_ASSERT(mp != NULL);
   PTY_ASSERT(ptp != NULL);

   switch(mp->b_datap->db_type) {

      case M_DATA:

         /* Nothing to send in this case (stty -cread) */
         if(!(ptp->pt_flags & PF_CREAD))
         {
            freemsg(mp);
      
            /* Exit function */
            PTY_PRINT_OUT(trace_out,"ptm_wput",0);
            break;
         }

         PTY_PRINT("ptm_wput : M_DATA"); 

         if(!(ptp->pt_flags & PF_SAK) && (!ptp->pt_mwq->q_first)) {
            if(pty_pts_xfer(ptp,mp)) {
               /* Master output is flow controlled:
                  we will be qenabled by slave later.  */
               putq(ptp->pt_mwq,mp);
            }
         }
         else putq(ptp->pt_mwq,mp);
         PTY_TRACE_OUT(trace_out = out_2);
         break;
   
     case M_IOCTL:
         switch(((struct iocblk *)mp->b_rptr)->ioc_cmd) {
            case TCSBRK:    /* this wait output to drain */
            case TCSBREAK:  /* this wait output to drain */
            case TIOCSETAW: /* this wait output to drain */
            case TIOCSETAF: /* this wait output to drain */
            if (ptp->pt_mwq->q_first) {
                  /* so just queue them in order */
                  putq(ptp->pt_mwq,mp);
                  PTY_TRACE_OUT(trace_out = out_3);
                  break;
	    }

            /* else fall through */
            default:
               pty_strioctl_comm(q,mp,ptp,MASTER_CALL);
               qreply(ptp->pt_mwq,mp);
               PTY_TRACE_OUT(trace_out = out_4);
         }
         break;

      case M_PCRSE:

	/* Check if a M_RSE message was sent by slave before */
	PTY_LOCK(&ptp->lock_ptp);
	if (ptp->pt_flags & PF_M_RSE) {
           ptp->pt_flags |= PF_RSE_OK;
           if (ptp->pt_flags & PF_SOPEN) {
	      PTY_UNLOCK(&ptp->lock_ptp);
              qenable(ptp->pt_swq);
	   }
           else PTY_UNLOCK(&ptp->lock_ptp);
	}
	else PTY_UNLOCK(&ptp->lock_ptp);
	freemsg(mp);
	break;

      case M_FLUSH:
         PTY_LOCK(&ptp->lock_ptp);
         if (ptp->pt_flags & PF_PKT) {
            if (*mp->b_rptr & FLUSHW)
               ptp->pt_send |= TIOCPKT_FLUSHREAD;
            if (*mp->b_rptr & FLUSHR)
               ptp->pt_send |= TIOCPKT_FLUSHWRITE;
         }
         PTY_UNLOCK(&ptp->lock_ptp);

         if(*mp->b_rptr & FLUSHW)flushq(ptp->pt_mwq,FLUSHDATA);
         if(*mp->b_rptr & FLUSHR) {
            flushq(ptp->pt_mrq,FLUSHDATA); 

            PTY_LOCK(&ptp->lock_ptp);
            if (ptp->pt_flags & PF_SOPEN) {
               PTY_UNLOCK(&ptp->lock_ptp);
               qenable(ptp->pt_swq);
            }
            else PTY_UNLOCK(&ptp->lock_ptp);
         }

         if(pty_pts_xfer(ptp,mp)) freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_5);
         break;
   
      case M_READ: 
         /* If we are in packet or user control mode we may
            have to send a control char (case of no select() function) 

         if(((ptp->pt_flags & PF_PKT) || (ptp->pt_flags & PF_UCNTL)) &&
            (ptp->pt_flags & PF_MOPEN)) {
               qenable(ptp->pt_mrq);
               ptm_send_pckt(ptp->pt_mrq);
         } */
         freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_6);
         break;
   
      case M_CTL:
         if(size=pty_mctl(q,mp,MASTER_CALL)) {
            putq(ptp->pt_mwq,mp);
         }
         PTY_TRACE_OUT(trace_out = out_7);
         break;
   
      case M_START:
         PTY_LOCK(&ptp->lock_ptp);
         if(ptp->pt_flags & PF_PKT) {

            /* Mode packet */
            ptp->pt_send  &= ~TIOCPKT_STOP;
            ptp->pt_send  |= TIOCPKT_START;
         }

         ptp->pt_flags  &= ~ PF_TTSTOP; /* start slave --> master */
         PTY_UNLOCK(&ptp->lock_ptp);



         /* Wake up everybody ! */
         if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_swq);
         if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mrq);
         if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mwq);
         if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_srq);
         freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_8);
         break;

      case M_STOP:
         PTY_LOCK(&ptp->lock_ptp);
         if(ptp->pt_flags & PF_PKT) {

            /* Mode packet */
            ptp->pt_send  &= ~TIOCPKT_START;
            ptp->pt_send  |= TIOCPKT_STOP;
         }
         else {
            ptp->pt_flags  |= PF_TTSTOP; /* stop slave --> master */
         }
         PTY_UNLOCK(&ptp->lock_ptp);


         freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_9);
         break;

      case M_STARTI:
         PTY_LOCK(&ptp->lock_ptp);
         ptp->pt_flags  &= ~ PF_TTINSTOP; /* start master --> slave */
         PTY_UNLOCK(&ptp->lock_ptp);

         /* Wake up everybody ! */
         if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_swq);
         if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mrq);
         if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mwq);
         if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_srq);
         freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_10);
         break;

      case M_STOPI:
         PTY_LOCK(&ptp->lock_ptp);
         ptp->pt_flags  |= PF_TTINSTOP; /* stop master --> slave */
         PTY_UNLOCK(&ptp->lock_ptp);
         freemsg(mp);
         PTY_TRACE_OUT(trace_out = out_11);
         break;

      default:
         /* don't expect other msgs */   
         freemsg(mp);
   }

   /* Exit function */
   PTY_PRINT_OUT(trace_out,"ptm_wput",error);
   Return(error);
}

/*        --- end of ptm_wput() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : ptm_wsrv()                                        */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                                                                     */
/*        RETURN : 0                                                   */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Write put master services.                                   */
/*        The following message are processed :                        */
/*                  M_CTL                                              */
/*                  M_IOCTL                                            */
/*                  M_DATA                                             */
/*                                                                     */
/***********************************************************************/

static int
ptm_wsrv(
         queue_t *q 
        )

{
   register struct pty_s *ptp = (struct pty_s *)q->q_ptr;
   register mblk_t       *mp;
   register mblk_t       *mp1;
   register mblk_t       *mp_sak;
   int                   size;
   unsigned char         *sak;   
   int                   error;
   INT_TRACE_OUT
   INT_PTYSAVPRI_LRC;

   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_WSRV,ptp->pt_dev_slave,(int)ptp,q->q_count,0,0);
   PTY_PRINT_IN("ptm_wsrv IN",NULL);

   PTY_ASSERT(ptp != NULL);

   while(mp=getq(q)) {

    switch(mp->b_datap->db_type) {

       case M_DATA:
         PTY_PRINT("ptm_wsrv : M_DATA");
         PTY_LOCK(&ptp->lock_ptp);

         /* Look for 'Ctrl X Ctrl R' sequence for Security */
         if(ptp->pt_flags & PF_SAK) {
            PTY_UNLOCK(&ptp->lock_ptp);
            mp1=mp;
            sak=mp1->b_rptr;
            do {
               do {
                  switch(*sak) {
                     case CTRL(x):   
                        state_sak=SAK_CTRL_X;
                        break;
      
                     case CTRL(r):   
                        if(state_sak == SAK_CTRL_X)state_sak=SAK_CTRL_R;
                        else state_sak=SAK_NOTHING;
                        break;
                     default : 
                        state_sak=SAK_NOTHING;
                  }  
                  (unsigned char *)sak++;
               } while(state_sak != SAK_CTRL_R && sak < mp1->b_wptr);
               mp1=mp1->b_cont;
            } while(mp1 && state_sak != SAK_CTRL_R);
           

            if(state_sak == SAK_CTRL_R) {
               if(!(mp_sak=allocb(sizeof(u_char),BPRI_MED))) {

                  putbq(ptp->pt_mwq,mp);
                  /* Exit function */
                  /* Not enough space */
                  PTY_TRACE_OUT(trace_out = out_2);
                  PTY_PRINT_OUT(trace_out,"ptm_wsrv",ENOMEM); 
                  Return(ENOMEM);
               }
               mp_sak->b_datap->db_type=M_PCSIG;
               *(unsigned char *)mp_sak->b_rptr=(u_char)SIGSAK;
               mp_sak->b_wptr=mp_sak->b_rptr+ sizeof(char);
   
               /* Don't need canput testing since M_PCSIG is a
                  high priority message */
               putnext(ptp->pt_mrq,mp_sak);
            }
         }
         else {
            PTY_UNLOCK(&ptp->lock_ptp);
         }
         if(error=pty_pts_xfer(ptp,mp)) {
            if(error==ENODEV) freemsg(mp);
            else {

               /* Master output is flow controlled:
                  we will be qenabled by slave later.  */
               putbq(ptp->pt_mwq,mp);
   
               /* Exit function */
               PTY_TRACE_OUT(trace_out = out_3);
               PTY_PRINT_OUT(trace_out,"ptm_wsrv",0);
               Return(0);
            }
         }
         break;

      case M_CTL:
         PTY_LOCK(&ptp->lock_ptp);
         if (!(ptp->pt_flags & PF_MWDRAIN)) {
            PTY_UNLOCK(&ptp->lock_ptp);
            if(size=pty_mctl(q,mp,MASTER_CALL)) {
                  putbq(ptp->pt_mwq,mp);
   
                  /* Exit function */
                  PTY_PRINT_OUT(trace_out,"ptm_wsrv",0);
                  Return(0);
            }
         }
         else {
            PTY_UNLOCK(&ptp->lock_ptp);
         }
         continue;

       case M_IOCTL:
          pty_strioctl_comm(q,mp,ptp,MASTER_CALL);
          qreply(ptp->pt_mwq,mp);
          continue;
      default :
         /* Don't expect other messages */
         freemsg(mp);
      }
   }

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_4);
   PTY_PRINT_OUT(trace_out,"ptm_wsrv",0);
   Return(0);

}

/*        --- End of ptm_wsrv() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : ptm_rput()                                        */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     *mp     : message block pointer                 */
/*                                                                     */
/*        RETURN : 0                                                   */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        master read put                                              */
/*        The following messages are processed :                       */
/*                  M_DATA                                             */
/*                  M_FLUSH                                            */
/*                                                                     */
/***********************************************************************/

static int
ptm_rput(
         queue_t *q,
         mblk_t  *mp 
        )

{

   register struct pty_s *ptp = (struct pty_s *)q->q_ptr;
   mblk_t                *pckt_ucntl_mp; /* packet and user ctrl mode */
   INT_TRACE_OUT
   INT_PTYSAVPRI_LRC;


   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_RPUT,ptp->pt_dev_slave,(int)ptp,mp,
         mp->b_datap->db_type,0);
   PTY_PRINT_IN("ptm_rput IN",NULL);
   PTY_PRINT_MBLK_T("ptm_rput",mp);

   PTY_ASSERT(ptp != NULL);
   switch(mp->b_datap->db_type) {
   
      case M_DATA : 

         PTY_LOCK(&ptp->lock_ptp);
         if((ptp->pt_flags & PF_SFLOW) && 
            (ptp->pt_mrq->q_count <= ptp->pt_mrq->q_lowat)) {
            ptp->pt_flags  &= ~PF_SFLOW;
            if(ptp->pt_flags & PF_SOPEN) {
               PTY_UNLOCK(&ptp->lock_ptp);
               qenable(ptp->pt_swq);
               PTY_LOCK(&ptp->lock_ptp);
            }
         }
         PTY_UNLOCK(&ptp->lock_ptp);

         /* Packet mode */
         PTY_LOCK(&ptp->lock_ptp);
         if(((ptp->pt_flags & PF_PKT) || (ptp->pt_flags & PF_UCNTL)) &&
            (ptp->pt_flags & PF_MOPEN)) {
            if(ptp->pt_send || ptp->pt_ucntl) {
               PTY_UNLOCK(&ptp->lock_ptp);
               ptm_send_pckt(ptp->pt_mrq);
            }
            else PTY_UNLOCK(&ptp->lock_ptp);
             if ((!ptp->pt_mrq->q_first) && canput(ptp->pt_mrq->q_next)) {

              /*   check if there is enough space to include control character
                  in existing buffer  */

              if(mp->b_datap->db_lim > mp->b_wptr) {
                  memmove(mp->b_rptr+sizeof(char),mp->b_rptr,
                          mp->b_wptr-mp->b_rptr); 

                  *mp->b_rptr=(u_char)TIOCPKT_DATA;
                  mp->b_wptr+=sizeof(u_char);
                  mp->b_flag &= ~MSGCOMPRESS;
                  mp->b_flag |= MSGPKT;
                  putnext(ptp->pt_mrq,mp);
                  break;
               }  

               /* We create a TIOCPKT_DATA M_DATA message */
               if(!(pckt_ucntl_mp=allocb(sizeof(u_char), BPRI_MED))) {
                  putq(ptp->pt_mrq,mp);
                  break;
               }

               /* initialization of TIOCPKT_DATA M_DATA message: if size of
               packet is bigger than asked by read routine, b_flag = MSGPKT
               asks to the stream head to do the work for us ! */
               pckt_ucntl_mp->b_datap->db_type=M_DATA;
               *pckt_ucntl_mp->b_rptr=(u_char)TIOCPKT_DATA;
               pckt_ucntl_mp->b_wptr=pckt_ucntl_mp->b_rptr + sizeof(u_char);
               pckt_ucntl_mp->b_cont=mp;
               pckt_ucntl_mp->b_flag |= MSGPKT;
               putnext(ptp->pt_mrq,pckt_ucntl_mp);
               break;
            }
         }
         else PTY_UNLOCK(&ptp->lock_ptp);

         PTY_PRINT("ptm_rput : M_DATA");
         if((!ptp->pt_mrq->q_first) && canput(ptp->pt_mrq->q_next)) 
            putnext(ptp->pt_mrq,mp);
        else  putq(ptp->pt_mrq,mp);
         break;
  
      case M_FLUSH:
         if(*mp->b_rptr & FLUSHR) {
            flushq(ptp->pt_mrq,FLUSHDATA);

            PTY_LOCK(&ptp->lock_ptp);
            if (ptp->pt_flags & PF_SOPEN) {
               PTY_UNLOCK(&ptp->lock_ptp);
               qenable(ptp->pt_swq);
            }
            else PTY_UNLOCK(&ptp->lock_ptp);
         }

         if(*mp->b_rptr & FLUSHW)flushq(ptp->pt_mwq,FLUSHDATA);
         
         if(canput(ptp->pt_mrq->q_next)) putnext(ptp->pt_mrq,mp);
         else putq(ptp->pt_mrq,mp);
         PTY_TRACE_OUT(trace_out = out_2);
         break; 
   
      default:
         if((mp->b_datap->db_type >= QPCTL) || 
            (!ptp->pt_mrq->q_first && canput(ptp->pt_mrq->q_next)))
            putnext(ptp->pt_mrq,mp);
         else putq(ptp->pt_mrq,mp);
         PTY_TRACE_OUT(trace_out = out_3);
         break;
   }

   /* Exit function */
   PTY_PRINT_OUT(trace_out,"ptm_rput",0);
   Return(0);

}

/*        --- end of ptm_rput() function ---        */

/***********************************************************************/
/*                                                                     */
/*        FUNCTION : ptm_send_pckt()                                   */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                                                                     */
/*        RETURN : nothing                                             */
/*                                                                     */
/*        PURPOSE :                                                    */
/*        This procedure sends in pckt control mode,  value of pt_send */
/*        and in user control mode, value of pt_ucntl                  */
/*                                                                     */
/***********************************************************************/
static int
ptm_send_pckt(
                  queue_t *q
                 )
{
   mblk_t                 *pckt_ucntl_mp; /* packet and user ctrl mode */
   register struct pty_s  *ptp=(struct pty_s *)q->q_ptr;
   u_char                 send_control_char;
   INT_PTYSAVPRI_LRC;
   INT_TRACE_OUT

   /* For packet mode : ptp->pt_send=                       */
   /* TIOCPKT_START | TIOCPKT_STOP                          */
   /* TIOCPKT_FLUSHREAD | TIOCPKT_FLUSHWRITE,               */
   /* TIOCPKT_NOSTOP | TIOCPKT_DOSTOP                       */

   /* For user control mode : ptp->pt_ucntl=                */
   /* Value between 1 and 255                               */
   /* Look in pty_ioctl_comm() function for UIOCCMD(0)...   */


   /* trace in */
   PTY_PRINT_IN("ptm_send_pckt IN",NULL);
   PTY_ASSERT(ptp != NULL);

   PTY_LOCK(&ptp->lock_ptp);
   if(ptp->pt_send || ptp->pt_ucntl) 
      send_control_char = ptp->pt_send | ptp->pt_ucntl;
   else { 
      send_control_char = TIOCPKT_DATA;
   }

   if(!(pckt_ucntl_mp=allocb(sizeof(char), BPRI_MED))) {
      PTY_UNLOCK(&ptp->lock_ptp);

      /* Exit function */
      PTY_TRACE_OUT(trace_out = out_2);
      PTY_PRINT_OUT(trace_out,"ptm_send_pckt",ENOMEM); /* Not enough space */
      return;
   }

   PTY_UNLOCK(&ptp->lock_ptp);
   pckt_ucntl_mp->b_datap->db_type=M_PCPROTO;
   *pckt_ucntl_mp->b_rptr=(u_char)send_control_char;
   pckt_ucntl_mp->b_wptr=pckt_ucntl_mp->b_rptr + sizeof(u_char);
   if(canput(ptp->pt_mrq->q_next)) {
      ptm_put(ptp,pckt_ucntl_mp,PUTNEXT);

      /* clear PACKET AND USER MODE flags */
      PTY_LOCK(&ptp->lock_ptp);
      ptp->pt_send=0;
      ptp->pt_ucntl=0;
      PTY_UNLOCK(&ptp->lock_ptp);

      /* Exit function */
      PTY_TRACE_OUT(trace_out = out_3);
      PTY_PRINT_OUT(trace_out,"ptm_send_pckt",0);
      return;
   }
   else {
 
      /* We free the message, and we'll recreate a new message later 
         called by ptm_rsrv because ptp->pt_send or ptp->pt_ucntl not
         equal to zero */
      freemsg(pckt_ucntl_mp);
      PTY_TRACE_OUT(trace_out = out_4);
      PTY_PRINT_OUT(trace_out,"ptm_send_pckt",0);
      return;
   }
}
/*          --- End of ptm_send_pckt() ---         */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : ptm_rsrv()                                        */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                                                                     */
/*        RETURN : error on failure 0 otherwise.                       */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        master read services.                                        */
/*        The following messages are processed :                       */
/*                   M_DATA                                            */
/*                                                                     */
/*        COMMENT :                                                    */
/*        For packet mode and user control mode, messages are linked   */
/*        together using mp->b_cont link. When no message is found in  */
/*        current queue (get(q) == NULL) first message including the   */
/*        control char is sent up.                                     */
/*                                                                     */
/***********************************************************************/

static int
ptm_rsrv(
         queue_t *q 
        )

{
   register struct pty_s  *ptp = (struct pty_s *)q->q_ptr;
   mblk_t                 *mp;
   mblk_t                 *last_mp;
   mblk_t                 *pckt_ucntl_mp; /* for packet and user ctrl mode */
   int                    pckt_ucntl_control;
   int                    pckt_ucntl_data;
   INT_PTYSAVPRI_LRC;
   INT_TRACE_OUT

   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_RSRV,ptp->pt_dev_slave,(int)ptp,q->q_count,0,0);
   PTY_PRINT_IN("ptm_rsrv IN",NULL);

   PTY_ASSERT(ptp != NULL);

   pckt_ucntl_control=TRUE; /* TRUE == We have to add a control char */
   pckt_ucntl_data=FALSE;   /* FALSE == No data to send              */

   while(mp=getq(q)) {

      PTY_LOCK(&ptp->lock_ptp);
      if((mp->b_datap->db_type == M_DATA) &&  
         ((ptp->pt_flags & PF_PKT) || (ptp->pt_flags & PF_UCNTL))) {

         /* packet mode */
         PTY_UNLOCK(&ptp->lock_ptp);
         if(pckt_ucntl_control==TRUE) { 

            /* Send control character */
            PTY_LOCK(&ptp->lock_ptp);
            if((ptp->pt_send || ptp->pt_ucntl)) { 
               PTY_UNLOCK(&ptp->lock_ptp);
               ptm_send_pckt(ptp->pt_mrq);
            }
            else { 
               PTY_UNLOCK(&ptp->lock_ptp);
            }

            /* We create a TIOCPKT_DATA M_DATA message */
            if(!(pckt_ucntl_mp=allocb(sizeof(u_char), BPRI_MED))) {
               putbq(ptp->pt_mrq,mp);
  
               /* Exit function */
               PTY_TRACE_OUT(trace_out = out_2);
 
               /* not enough space */
               PTY_PRINT_OUT(trace_out,"ptm_rsrv",ENOMEM); 
               Return(ENOMEM);
            }
  
            /* initialization of TIOCPKT_DATA M_DATA message: if size of
               packet is bigger than asked by read routine, b_flag = MSGPKT
               asks to the stream head to do the work for us ! (Good grief) */
            pckt_ucntl_mp->b_datap->db_type=M_DATA;
            *pckt_ucntl_mp->b_rptr=(u_char)TIOCPKT_DATA;
            pckt_ucntl_mp->b_wptr=pckt_ucntl_mp->b_rptr + sizeof(u_char);
            pckt_ucntl_mp->b_cont=mp;
            pckt_ucntl_mp->b_flag |= MSGPKT;
            pckt_ucntl_control=FALSE;
            pckt_ucntl_data=TRUE;
            last_mp=mp; 

            /* look for last_mp */
            do {
               if(last_mp->b_cont) last_mp=last_mp->b_cont;
            } while (last_mp->b_cont);
               
         }
         else { 

            /* We link last message to first TIOCPKT_DATA M_DATA message 
               and look for last mp message */
  
            /* Link new block to last block */

            PTY_ASSERT(last_mp != NULL);
            last_mp->b_cont=mp;
            last_mp=mp;
            do {
               if(last_mp->b_cont) last_mp=last_mp->b_cont;
            } while (last_mp->b_cont);
         }
 
         /* If flow control problem send packet */

         PTY_LOCK(&ptp->lock_ptp);
         if((ptp->pt_flags & PF_SFLOW) && (pckt_ucntl_data==TRUE))  {
            PTY_UNLOCK(&ptp->lock_ptp);
            if((pckt_ucntl_mp->b_datap->db_type >= QPCTL) ||
               (canput(ptp->pt_mrq->q_next))) {
               putnext(ptp->pt_mrq,pckt_ucntl_mp);
               PTY_LOCK(&ptp->lock_ptp);
               pckt_ucntl_control=TRUE; 
               pckt_ucntl_data=FALSE; 
               if(ptp->pt_mrq->q_count <= ptp->pt_mrq->q_lowat) {
                  ptp->pt_flags  &= ~PF_SFLOW;
                  if(ptp->pt_flags & PF_SOPEN) {
                     PTY_UNLOCK(&ptp->lock_ptp);
                     qenable(ptp->pt_swq);
                  }
                  else {
                     PTY_UNLOCK(&ptp->lock_ptp);
                  }
               }
               else {
                  PTY_UNLOCK(&ptp->lock_ptp);
               }
            }
            else {
               putbq(ptp->pt_mrq,pckt_ucntl_mp->b_cont);
               PTY_ASSERT(pckt_ucntl_mp != NULL);
               freeb(pckt_ucntl_mp);
   
               /* Exit function */
               PTY_TRACE_OUT(trace_out = out_3);
               PTY_PRINT_OUT(trace_out,"ptm_rsrv",0);
               Return(0);
            }
         }
         else {
            PTY_UNLOCK(&ptp->lock_ptp);
         }
      }
      else { 

         /* Not packet mode or M_DATA message, just putnext it */
        if((mp->b_datap->db_type >= QPCTL) ||  
            canput(ptp->pt_mrq->q_next)) {
            PTY_UNLOCK(&ptp->lock_ptp);
            putnext(ptp->pt_mrq,mp);
         }
         else { 
            PTY_UNLOCK(&ptp->lock_ptp);
            putbq(ptp->pt_mrq,mp);
      
            /* Exit function */
            PTY_TRACE_OUT(trace_out = out_4);
            PTY_PRINT_OUT(trace_out,"ptm_rsrv",0);
            Return(0);
         }
      }
   } /* End of while ... */
   
   /* Packet mode */
   PTY_LOCK(&ptp->lock_ptp);
   if((ptp->pt_flags & PF_PKT) || (ptp->pt_flags & PF_UCNTL)) {
      PTY_UNLOCK(&ptp->lock_ptp);

      /* Look if there is a message to send upstreams */
      if(pckt_ucntl_data==TRUE) {
         if(canput(ptp->pt_mrq->q_next)) {
            putnext(ptp->pt_mrq,pckt_ucntl_mp);
         }
         else {

            /* To avoid to be rescheduled immediately we call noenable() */
            noenable(ptp->pt_mrq);
            putbq(ptp->pt_mrq,pckt_ucntl_mp->b_cont);
            enableok(ptp->pt_mrq);
            PTY_ASSERT(pckt_ucntl_mp != NULL);
            freeb(pckt_ucntl_mp);

            /* Exit function */
            PTY_TRACE_OUT(trace_out = out_5);
            PTY_PRINT_OUT(trace_out,"ptm_rsrv",0);
            Return(0);
         }
      }
   }
   else {
      PTY_UNLOCK(&ptp->lock_ptp);
   }

   /* flow control */
   PTY_LOCK(&ptp->lock_ptp);
   if((ptp->pt_flags & PF_SFLOW) &&
      (ptp->pt_mrq->q_count <= ptp->pt_mrq->q_lowat)) {
      ptp->pt_flags  &= ~PF_SFLOW;
      if(ptp->pt_flags & PF_SOPEN) {
         PTY_UNLOCK(&ptp->lock_ptp);
         qenable(ptp->pt_swq);
         PTY_LOCK(&ptp->lock_ptp);
      }
   }

   /* Drain output */
   if(ptp->pt_flags & PF_SWDRAIN) {
      PTY_UNLOCK(&ptp->lock_ptp);
      e_wakeup(&ptp->pt_swait_drain);
      PTY_LOCK(&ptp->lock_ptp);
   }
   PTY_UNLOCK(&ptp->lock_ptp);

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_6);
   PTY_PRINT_OUT(trace_out,"ptm_rsrv",0);
   Return(0);
}       

/*        --- end of ptm_rsrv() function ---        */

/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_mctl()                                        */
/*                                                                     */
/*        PARAMETER :  *q          : streams queue pointer             */
/*                     *mp         : message block pointer             */
/*                     master_call : SLAVE_CALL or MASTER_CALL         */
/*                                   (origin of the message)           */
/*                                                                     */
/*        RETURN : Size of termios structure requested. A non zero     */
/*                 RETURN value indicates a memory allocation problem. */
/*                 0 otherwise.                                        */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        The following commands are processed:                        */
/*                  TIOCGETA (TCGETS)                                  */
/*                  TIOC_REQUEST                                       */
/*                  TIOC_REPLY                                         */
/*                  MC_CANONQUERY                                      */
/*                  default : free message.                            */
/*                                                                     */
/***********************************************************************/

static int
pty_mctl(
         queue_t *q,
         mblk_t  *mp,
         int     master_call
        )

{
   struct iocblk   *iocp;
   struct pty_s    *ptp=(struct pty_s *)q->q_ptr;
   struct termios  *tios;
   mblk_t          *mp1;
   register mblk_t *new_mp;
   register int    r_size;
   INT_PTYSAVPRI_LRC;
   INT_TRACE_OUT


   /* trace in */
   PTY_PRINT_IN("pty_mctl IN",NULL);
   PTY_PRINT_MBLK_T("pty_mctl",mp);

   PTY_ASSERT(q !=NULL);
   PTY_ASSERT(mp != NULL);
   PTY_ASSERT(ptp != NULL);

   iocp=(struct iocblk *)mp->b_rptr;
   r_size=sizeof(pty_tioc_reply);

   PTY_PRINT_CMD("pty_mctl",iocp->ioc_cmd);

   switch(iocp->ioc_cmd) {

      case TIOCGETA:
        if ((mp1 = mp->b_cont) &&
            (mp1->b_datap->db_lim - mp1->b_rptr >= sizeof(struct termios))) {

            PTY_LOCK(&ptp->lock_ptp);
            bcopy(&ptp->pt_tio,mp1->b_rptr,sizeof(struct termios));
            PTY_UNLOCK(&ptp->lock_ptp);
            mp1->b_wptr=mp1->b_rptr + sizeof(struct termios);
            qreply(q,mp);
            break;
        } 
        else {
            freemsg(mp);
            PTY_TRACE_OUT(trace_out = out_2);
            break;
        }

      case MC_CANONQUERY:
        if ((mp1 = mp->b_cont) &&
            (mp1->b_datap->db_lim - mp1->b_rptr >= sizeof(struct termios))) {
            tios = (struct termios *)mp1->b_rptr;
            bzero((char * )tios,sizeof(struct termios));
            iocp->ioc_cmd = MC_PART_CANON;
            mp1->b_wptr = mp1->b_rptr + sizeof( struct termios);
            qreply(q,mp);
            PTY_TRACE_OUT(trace_out = out_3);
            break;
        } 
        else {
            freemsg(mp);
            PTY_TRACE_OUT(trace_out = out_4);
            break;
        }

      case TIOC_REQUEST:
      case TIOC_REPLY:
         if(!(new_mp=allocb(r_size,BPRI_MED))) {

            /* Exit function */
            PTY_TRACE_OUT(trace_out = out_5);
            PTY_PRINT_OUT(trace_out,"pty_mctl",0);
            return(r_size); 
         }
         else {
            new_mp->b_cont=mp->b_cont;
            mp->b_cont=new_mp;
            new_mp->b_datap->db_type=M_DATA;
            new_mp->b_wptr=new_mp->b_rptr + r_size;
            bcopy(pty_tioc_reply,new_mp->b_rptr,r_size);
            iocp->ioc_cmd=TIOC_REPLY;
            qreply(q,mp);
            PTY_TRACE_OUT(trace_out = out_6);
            break;
         }

      	case TXTTYNAME:
        if (!(mp1 = mp->b_cont) ||
             (mp1->b_datap->db_lim - mp1->b_rptr < TTNAMEMAX)) {
            freemsg(mp);

            /* Exit function */
            PTY_TRACE_OUT(trace_out = out_7);
            PTY_PRINT_OUT(trace_out,"pty_mctl",0);
            return(0);
        }
        bcopy(ptp->pt_name,(char *)mp1->b_rptr, sizeof(ptp->pt_name));
        mp1->b_wptr = mp1->b_rptr + sizeof(ptp->pt_name);
        qreply(q,mp);
        break;

      default:
         freemsg(mp);
   }

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_8);
   PTY_PRINT_OUT(trace_out,"pty_mctl",0);
   return(0);

}

/*        --- end of pty_mctl() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_strioctl_comm()                               */
/*                                                                     */
/*        PARAMETER :  *q          : streams queue pointer             */      
/*                     *mp         : message block pointer             */
/*                     *ptp        : pty_s structure pointer           */
/*                     master_call : MASTER_CALL or SLAVE_CALL         */
/*                                   (origin of the message)           */
/*                                                                     */
/*                                                                     */
/*        RETURN  : 0 (messages are sent back via *mp)                 */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Process an M_IOCTL streams message.                          */ 
/*        The ioctls are processed in pty_ioctl_comm(). An M_IOCACK or */
/*        M_IOCNAK message is sent back according to result of         */
/*        treatment done by pty_ioctl_comm().                          */
/*                                                                     */
/***********************************************************************/

static int
pty_strioctl_comm(
                  queue_t      *q,
                  mblk_t       *mp,
                  struct pty_s *ptp,
                  int          master_call 
                 )

{
   int                error=0;
   struct iocblk      *iocp;
   INT_TRACE_OUT

   /* trace in */
   Enter(HKWD_STTY_PTY | TTY_IOCTL,ptp->pt_dev_slave,(int)ptp,0,0,0);
   PTY_PRINT_IN("pty_strioctl_comm IN",NULL);
   PTY_PRINT_MBLK_T("pty_strioctl_comm",mp);
 
   PTY_ASSERT(q != NULL);
   PTY_ASSERT(mp != NULL);
   PTY_ASSERT(ptp != NULL);

   iocp=(struct iocblk *)mp->b_rptr;

   if(error=pty_ioctl_comm(q,mp,ptp,master_call)) {
      mp->b_datap->db_type=M_IOCNAK;
      iocp->ioc_error=error;
      iocp->ioc_count=0;
      if(mp->b_cont) {
         freemsg(mp->b_cont);
         mp->b_cont=NULL;
      }

      PTY_PRINT_MBLK_T("pty_strioctl_comm (M_IOCNAK)",mp);
   } 
   else {
      mp->b_datap->db_type=M_IOCACK;
      iocp->ioc_error=0;
      iocp->ioc_count=0;
      PTY_PRINT_MBLK_T("pty_strioctl_comm (M_IOCACK)",mp);
   }

   /* Exit function */
   PTY_PRINT_OUT(trace_out,"pty_strioctl_comm",0);
   Return(0);

}

/*        --- end of pty_strioctl_comm() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_ioctl_comm()                                  */
/*                                                                     */
/*        PARAMETER :  *q          : streams queue pointer             */      
/*                     *mp         : message block pointer             */
/*                     *ptp        : pty_s structure pointer           */
/*                     master_call : indicates the ioctl source        */
/*                                   SLAVE_CALL : down the slave side  */
/*                                   MASTER_CALL: down the master side */
/*                                                                     */
/*        RETURN : errno on failure or 0. 			       */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        common ioctl for all the pty driver. pty_ioctl_comm() is     */
/*        called by pty_strioctl_comm() routine.                       */
/*                                                                     */
/*        IOCTL processed in this procedure are:                       */
/*             TIOCPKT           Packet mode (ON/OFF)                  */
/*             TIOCUCNTL         User control mode (ON/OFF)            */
/*             TIOCREMOTE        Remote mode (ON/OFF)                  */
/*             TIOCSTOP          Stop terminal output == M_STOP        */
/*             TIOCSTART         Start terminal output == M_START      */
/*             TCSAK             Security (ON/OFF)                     */
/*             TCQSAK            Security state                        */
/*             TXTTYNAME         Name of slave device driver           */
/*             TIOCGETA          Get termios data (TCGETS)             */
/*             TIOCSETAF         Updates local termio data             */ 
/*             TIOCSETA          Updates local termio data (TCSETS)    */ 
/*             TIOCSETAW         Updates local termio data             */ 
/*             TIOCEXCL          Exclude (ON)                          */
/*             TIOCNXCL          Exclude (OFF)                         */
/*             TIOCGWINSZ        Update local window sizes             */
/*             TIOCSWINSZ        Send M_CTL message including          */
/*                               window sizes                          */
/*             TIOCSETD          NOP in streams                        */ 
/*             EUC_WSET          NOP in streams if comes from slave    */
/*             EUC_WGET          NOP in streams if comes from slave    */
/*             EUC_IXLON         NOP in streams if comes from slave    */
/*             EUC_IXLOFF        NOP in streams if comes from slave    */
/*             EUC_OXLON         NOP in streams if comes from slave    */
/*             EUC_OXLOFF        NOP in streams if comes from slave    */
/*             EUC_MSAVE         NOP in streams if comes from slave    */
/*             EUC_MREST         NOP in streams if comes from slave    */
/*             TIOCCONS          NOP in streams if comes from slave    */
/*             TCSBRK            Send M_BREAK with break_interrupt on  */ 
/*             TCSBREAK          same as TCSBRK                        */ 
/*             UIOCCMD(i)        for User Control Mode                 */
/*                                                                     */
/*  The following ioctl are processed in pty_bsd43_ioctl() :           */
/*             TIOCGETP  termios to sgttyb                             */
/*             TIOCSETP  sgttyb to termios                             */
/*             TIOCSETN  sgttyb to termios                             */
/*             TIOCGETC  termios to tchars                             */
/*             TIOCSETC  tchars to termios                             */
/*             TIOCGLTC  termios to ltchars                            */
/*             TIOCSLTC  ltchars to termios                            */
/*             TIOCLGET  get termios flag from ldtty_compatgetflags    */
/*             TIOCLBIS  set termios flag with flags_to_termios        */
/*             TIOCLBIC  set termios flag with flags_to_termios        */
/*             TIOCLSET  set termios flag with flags_to_termios        */
/*                                                                     */
/***********************************************************************/

static int
pty_ioctl_comm(
                queue_t               *q,
		mblk_t                *mp,
                struct pty_s 	      *ptp,
                int                   master_call 
                )

{
   mblk_t             *mp1;
   struct iocblk      *iocp;
   int                cmd;
   int                error=0;
   struct termios     t,*termp1= &t;
   struct stroptions  *option;        /* packet mode initialization */
   speed_t            getospeed;  
   caddr_t   	      data;
   char               null_data=0;
   INT_PTYSAVPRI_LRC;
   INT_TRACE_OUT


   /* trace in */
   PTY_PRINT_IN("pty_ioctl_comm IN",NULL);
   PTY_PRINT_CMD("pty_ioctl_comm",cmd);

   PTY_ASSERT(q !=NULL);
   PTY_ASSERT(ptp != NULL);

   /* initialize data */

   iocp=(struct iocblk *)mp->b_rptr;
   cmd=iocp->ioc_cmd;
   if(mp->b_cont) data = (caddr_t)mp->b_cont->b_rptr; 
   else data=&null_data; 

   /* Verify that master-only ioctl's don't come down the slave.  */
   if(!master_call) {
      switch(cmd) {
         case TIOCPKT:
         case TIOCUCNTL:
         case TIOCREMOTE:

            /* Exit function */
            /* Invalid argument */
            PTY_PRINT_OUT(trace_out,"pty_ioctl_comm",EINVAL); 
            return(EINVAL);

         default: break;
      }
   }

   switch(cmd) {
  
      case TIOCPKT: 

         PTY_LOCK(&ptp->lock_ptp);
         if(**(int **)data) {
            if(ptp->pt_flags & PF_UCNTL) {
               error=EINVAL; /* Invalid argument */
               PTY_UNLOCK(&ptp->lock_ptp);
               PTY_TRACE_OUT(trace_out = out_2);
               break;
            }
            else {
               PTY_UNLOCK(&ptp->lock_ptp);

               if(!(mp1=allocb(sizeof(struct stroptions),BPRI_MED))) {
                  error=ENOMEM; /* Not enough space */
                  PTY_TRACE_OUT(trace_out = out_3);
                  break;
               }
               mp1->b_datap->db_type=M_SETOPTS;
               mp1->b_wptr=mp1->b_rptr + sizeof(struct stroptions);
               option=(struct stroptions *)mp1->b_rptr;
               option->so_flags=SO_READOPT | SO_ISTTY; 
               option->so_readopt=RMSGN | RPROTDAT;
               if(pty_ptm_xfer(ptp,mp1)) freemsg(mp1);
               PTY_LOCK(&ptp->lock_ptp);
               ptp->pt_flags  |= PF_PKT;  /*  Packet mode on */
               PTY_UNLOCK(&ptp->lock_ptp);
            }
         }
         else {
               ptp->pt_flags  &= ~PF_PKT;
               PTY_UNLOCK(&ptp->lock_ptp);
         }
         PTY_TRACE_OUT(trace_out = out_4);
         break;

      case TIOCUCNTL:
         PTY_LOCK(&ptp->lock_ptp);
         if(**(int **)data) {
            if(ptp->pt_flags & PF_PKT) {
               error=EINVAL; /* Invalid argument */
               PTY_UNLOCK(&ptp->lock_ptp);
               PTY_TRACE_OUT(trace_out = out_5);
               break;
            }
            else {
               PTY_UNLOCK(&ptp->lock_ptp);

               if(!(mp1=allocb(sizeof(struct stroptions),BPRI_MED))) {
                  error=ENOMEM; /* Not enough space */
                  PTY_TRACE_OUT(trace_out = out_6);
                  break;
               }
               mp1->b_datap->db_type=M_SETOPTS;
               mp1->b_wptr=mp1->b_rptr + sizeof(struct stroptions);
               option=(struct stroptions *)mp1->b_rptr;
               option->so_flags=SO_READOPT | SO_ISTTY;
               option->so_readopt=RMSGN | RPROTDAT;
               if(pty_ptm_xfer(ptp,mp1)) freemsg(mp1);
               PTY_LOCK(&ptp->lock_ptp);
               ptp->pt_flags  |= PF_UCNTL;  /*  Packet mode on */
               PTY_UNLOCK(&ptp->lock_ptp);
            }
         }
         else {
               ptp->pt_flags  &= ~PF_UCNTL;
               PTY_UNLOCK(&ptp->lock_ptp);
         }

         PTY_TRACE_OUT(trace_out = out_7);
         break;

      case TIOCREMOTE:
         /* Verif y that slave is there.  */
         PTY_LOCK(&ptp->lock_ptp);
         if(!(ptp->pt_flags &  PF_SOPEN)) {
            PTY_UNLOCK(&ptp->lock_ptp);
            error=ENODEV; /* No such device */
            PTY_TRACE_OUT(trace_out = out_8);
            break;
         }
         else {
            PTY_UNLOCK(&ptp->lock_ptp);
         }

         /* Now send M_FLUSH up to the slave.  */
         if(!(mp1=allocb(sizeof(char),BPRI_MED))) {
            error=ENOMEM; /* Not enough space */
            PTY_TRACE_OUT(trace_out = out_9);
            break;
         }

         mp1->b_datap->db_type=M_FLUSH;
         *mp1->b_rptr=(char)FLUSHW;
         if(pty_pts_xfer(ptp,mp1)) freemsg(mp1);
         if(**(int **)data) {

           /* process a M_CTL message */
            mp1=pty_ctl(q,MC_NO_CANON,0); 
            if(!mp1) {
               error=ENOMEM; /* Not enough space */
               PTY_TRACE_OUT(trace_out = out_10);
               break;
            }
            PTY_LOCK(&ptp->lock_ptp);
            ptp->pt_flags  |= PF_REMOTE;
            PTY_UNLOCK(&ptp->lock_ptp);

         } 
         else {

            /* process a M_CTL message */
            mp1=pty_ctl(q,MC_DO_CANON,0); 
            if(!mp1) {
               error=ENOMEM; /* Not enough space */
               PTY_TRACE_OUT(trace_out = out_11);
               break;
            }
            PTY_LOCK(&ptp->lock_ptp);
            ptp->pt_flags  &= ~PF_REMOTE;
            PTY_UNLOCK(&ptp->lock_ptp);
         }

         /* Now send M_CTL with new setting for line discipline.  */
         if(pty_pts_xfer(ptp,mp1)) freemsg(mp1);
         PTY_TRACE_OUT(trace_out = out_12);
         break;

      case TCSAK :

         if (iocp->ioc_count < sizeof(int) || !mp->b_cont) {
            error = ENOSPC;
            break;
         }

         PTY_LOCK(&ptp->lock_ptp);
         if(**(int **)data)ptp->pt_flags  |= PF_SAK;
         else ptp->pt_flags  &= ~PF_SAK;
         PTY_UNLOCK(&ptp->lock_ptp);
         PTY_TRACE_OUT(trace_out = out_13);
         break;

      case TCQSAK :
         PTY_LOCK(&ptp->lock_ptp);
         **(int **)data=(ptp->pt_flags & PF_SAK);
         PTY_UNLOCK(&ptp->lock_ptp);
         PTY_TRACE_OUT(trace_out = out_14);
         break;

      case TCXONC : /* same as M_STOP coming from master side */
         if(*(int *)data == TCOOFF) {
            PTY_LOCK(&ptp->lock_ptp);
            if(ptp->pt_flags & PF_PKT) {
   
               /* Mode packet */
               ptp->pt_send  &= ~TIOCPKT_START;
               ptp->pt_send  |= TIOCPKT_STOP;
            }
            
            ptp->pt_flags  |= PF_TTSTOP; /* stop slave --> master */
            PTY_UNLOCK(&ptp->lock_ptp);
   
            PTY_TRACE_OUT(trace_out = out_15);
         }
         else if (*(int *)data == TCOON) {
            PTY_LOCK(&ptp->lock_ptp);
            if(ptp->pt_flags & PF_PKT) {
      
               /* Mode packet */
               ptp->pt_send  &= ~TIOCPKT_STOP;
               ptp->pt_send  |= TIOCPKT_START;
            }
      
            ptp->pt_flags  &= ~ PF_TTSTOP; /* start slave --> master */
            PTY_UNLOCK(&ptp->lock_ptp);
   
            /* Wake up everybody ! */
            if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_swq);
            if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mrq);
            if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mwq);
            if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_srq);
            PTY_TRACE_OUT(trace_out = out_16);
         }
         break;

      case TIOCSTOP : /* same as M_STOP coming from master side */
         PTY_LOCK(&ptp->lock_ptp);
         if(ptp->pt_flags & PF_PKT) {

            /* Mode packet */
            ptp->pt_send  &= ~TIOCPKT_START;
            ptp->pt_send  |= TIOCPKT_STOP;
         }
         
         ptp->pt_flags  |= PF_TTSTOP; /* stop slave --> master */
         PTY_UNLOCK(&ptp->lock_ptp);
         PTY_TRACE_OUT(trace_out = out_17);
         break;

      case TIOCSTART : /* same as M_START coming from master side */
         PTY_LOCK(&ptp->lock_ptp);
         if(ptp->pt_flags & PF_PKT) {
  
            /* Mode packet */
            ptp->pt_send  &= ~TIOCPKT_STOP;
            ptp->pt_send  |= TIOCPKT_START;
         }

         ptp->pt_flags  &= ~ PF_TTSTOP; /* start slave --> master */
         PTY_UNLOCK(&ptp->lock_ptp);

         /* Wake up everybody ! */
         if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_swq);
         if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mrq);
         if(ptp->pt_flags & PF_MOPEN)qenable(ptp->pt_mwq);
         if(ptp->pt_flags & PF_SOPEN)qenable(ptp->pt_srq);
         PTY_TRACE_OUT(trace_out = out_18);
         break;

      case TXTTYNAME: 

         if (iocp->ioc_count < TTNAMEMAX || !mp->b_cont) {
            error = ENOSPC;
            break;
         }
         sprintf(data,"%s",ptp->pt_name);
         PTY_TRACE_OUT(trace_out = out_19);
         break;

      case TIOCGETA: /* TCGETS */

         if (iocp->ioc_count < sizeof(struct termios) || !mp->b_cont) {
            error = ENOSPC;
            break;
         }

         /* fill in values */
         *(struct termios *)data=ptp->pt_tio;
         PTY_TRACE_OUT(trace_out = out_20);
         break;

      PTY_TRACE_OUT(trace_out = out_21);
      break;

      case TIOCEXCL:
         PTY_LOCK(&ptp->lock_ptp);
         ptp->pt_flags  |= PF_XCLUDE;
         PTY_UNLOCK(&ptp->lock_ptp);
         PTY_TRACE_OUT(trace_out = out_22);
         break;

      case TIOCNXCL:
         PTY_LOCK(&ptp->lock_ptp);
         ptp->pt_flags  &= ~PF_XCLUDE;
         PTY_UNLOCK(&ptp->lock_ptp);
         PTY_TRACE_OUT(trace_out = out_23);
         break;

      case TIOCGWINSZ:
         PTY_LOCK(&ptp->lock_ptp);
         if(!ptp->pt_ws.ws_col) {
            /* See V.4 Streams Programmer's Guide,p. 12-18.  */
            error=EINVAL; /* Invalid argument */
         }
         else {
            bcopy(&ptp->pt_ws,data,sizeof(struct winsize));
         }
         PTY_UNLOCK(&ptp->lock_ptp);
         PTY_TRACE_OUT(trace_out = out_24);
         break;

      case TIOCSWINSZ:
         PTY_LOCK(&ptp->lock_ptp);
         ptp->pt_ws=*(struct winsize *)data;
         if(master_call && (ptp->pt_flags & PF_SOPEN)) {

            /* process a M_CTL message */
            mp1=pty_ctl(q,TIOCSWINSZ,sizeof(struct winsize)); 
            if(!mp1) {
               error=ENOMEM; /* Not enough memory */
               PTY_TRACE_OUT(trace_out = out_25);
               PTY_UNLOCK(&ptp->lock_ptp);
               break;
            }
            bcopy(data,mp1->b_cont->b_rptr,sizeof(struct winsize));
            mp1->b_cont->b_wptr=mp1->b_cont->b_rptr + sizeof(struct winsize);
            PTY_UNLOCK(&ptp->lock_ptp);
            if(pty_pts_xfer(ptp,mp1)) freemsg(mp1);
         }
         else {
            PTY_UNLOCK(&ptp->lock_ptp);
         }
         PTY_TRACE_OUT(trace_out = out_26);
         break;

      /* The line discipline converts most of these to "normal"
       *(i.e.,termios-style)ioctls on the way down,but
       * a few applications send these down the master side,
       * so we must process them here.
       */
      case TIOCGETP:
      case TIOCSETP:
      case TIOCSETN:
      case TIOCGETC:
      case TIOCSETC:
      case TIOCSLTC:
      case TIOCGLTC:
      case TIOCLBIS:
      case TIOCLBIC:
      case TIOCLSET:
      case TIOCLGET:
      case TIOCGETD:
      case TIOCSETD:
         *termp1=ptp->pt_tio;

         /* Call COMPAT_43 ioctls routine */
         if(pty_bsd43_ioctl(cmd,data,termp1,&ptp->pt_compatflags))
            goto seta_postproc; /* ---------------------------------+   */
         PTY_TRACE_OUT(trace_out = out_27);			/*  |   */
         break;                            			/*  |   */
                                           			/*  |   */
      case TIOCSETAF:			                        /*  |   */
      case TIOCSETA: /* TCSETS */      			        /*  |   */
      case TIOCSETAW:                      			/*  |   */
         if (iocp->ioc_count != sizeof(struct termios) ||       /*  |   */
             !mp->b_cont) {					/*  |   */
                  error = ENOSPC;				/*  |   */
                  break;					/*  |   */
         }							/*  |   */
                                           			/*  |   */
        termp1=(struct termios *)data;     			/*  |   */
                                           			/*  |   */
seta_postproc: /* <-------------------------------------------------+   */

         PTY_LOCK(&ptp->lock_ptp);
         ptp->pt_tio=*termp1; 
         if(ptp->pt_tio.c_cflag & CREAD) ptp->pt_flags |= PF_CREAD;
         else ptp->pt_flags &= ~PF_CREAD;

         /* We also have to send a TIOCPKT_NOSTOP if stop or start
            characters are differents from those received at configuration
            time or a TIOCPKT_DOSTOP if they are equals and IXON flag is
            turned on */
         if((ptp->pt_tio.c_iflag & IXON) && !(ptp->pt_flags & PF_IXON)) {
            ptp->pt_flags |= PF_IXON;
            if((ptp->pt_flags & PF_PKT) &&
               (ptp->pt_vstart==ptp->pt_tio.c_cc[VSTART]) &&
               (ptp->pt_vstop==ptp->pt_tio.c_cc[VSTOP])) {  
               ptp->pt_send |= TIOCPKT_DOSTOP;
               ptp->pt_send &= ~TIOCPKT_NOSTOP;
            }
         }

         if(!(ptp->pt_tio.c_iflag & IXON) && (ptp->pt_flags & PF_IXON)) {
	    ptp->pt_flags &= ~PF_IXON;
            if((ptp->pt_flags & PF_PKT) &&
               ((ptp->pt_vstart!=ptp->pt_tio.c_cc[VSTART]) ||
                (ptp->pt_vstop!=ptp->pt_tio.c_cc[VSTOP]))) {
               ptp->pt_send |= TIOCPKT_NOSTOP;
               ptp->pt_send &= ~TIOCPKT_DOSTOP;
            }
	}

         if(ptp->pt_tio.c_iflag & IXOFF) ptp->pt_flags |= PF_IXOFF;
         else ptp->pt_flags &= ~PF_IXOFF;
         if(ptp->pt_tio.c_iflag & IXANY) ptp->pt_flags |= PF_IXANY;
         else ptp->pt_flags &= ~PF_IXANY;
         PTY_UNLOCK(&ptp->lock_ptp);

         /* set ispeed = ospeed */
         getospeed = cfgetospeed(termp1); 
         if(cfgetispeed(termp1) == 0)cfsetispeed(termp1, getospeed);

         if (master_call) {
            /* process a M_CTL message */
            mp1=pty_ctl(q,TIOCSETA,sizeof(struct termios)); 
            if(!mp1) {
               error=ENOMEM; /* Not enough memory */
               PTY_TRACE_OUT(trace_out = out_28);
               break;
            }

            bcopy(&ptp->pt_tio,mp1->b_cont->b_rptr,sizeof(struct termios));
            mp1->b_cont->b_wptr=mp1->b_cont->b_rptr + sizeof(struct termios);
            if(pty_pts_xfer(ptp,mp1)) freemsg(mp1);
        }

         PTY_TRACE_OUT(trace_out = out_29);
         break;


      case EUC_WSET:
      case EUC_WGET:
      case EUC_IXLON:
      case EUC_IXLOFF:
      case EUC_OXLON:
      case EUC_OXLOFF:
      case EUC_MSAVE:
      case EUC_MREST:
      case TIOCCONS:
         if(master_call) error = EINVAL;
         PTY_TRACE_OUT(trace_out = out_30);
         break;

      case TCSBRK:
      case TCSBREAK:

         if(iocp->ioc_count != sizeof (int) || !mp->b_cont) {
                  error = ENOSPC;
                  break;
         }


         if(master_call) {
            if(!(mp1=allocb(sizeof(enum status),BPRI_MED))) {
               error=ENOMEM; /* Not enough space */
               PTY_TRACE_OUT(trace_out = out_31);
               break;
            }
            mp1->b_datap->db_type=M_BREAK;
	    *(enum status *)mp1->b_rptr=break_interrupt;
            mp1->b_wptr=mp1->b_rptr + sizeof(enum status);
            if(pty_pts_xfer(ptp,mp1)) freemsg(mp1);
         } 
      
         PTY_TRACE_OUT(trace_out = out_32);
         break;

      default:
    
         /* Check UIOCCMD for User Control Mode */
         if((ptp->pt_flags & PF_UCNTL) && (cmd & ~0xff) == UIOCCMD(0)) {
            PTY_LOCK(&ptp->lock_ptp);
            ptp->pt_ucntl=(u_char)(cmd & 0xff); 
            PTY_UNLOCK(&ptp->lock_ptp);
            if (ptp->pt_flags & PF_MOPEN)
               ptm_send_pckt(ptp->pt_mrq);
            PTY_TRACE_OUT(trace_out = out_33);
         }
         else {
            error=EINVAL; /* Invalid argument */
            PTY_TRACE_OUT(trace_out = out_34);
         }
   }


   /* Exit function */
   PTY_PRINT_OUT(trace_out,"pty_ioctl_comm",error);
   return(error);
}

/*        --- end of pty_ioctl_comm() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_bsd43_ioctl()                                 */
/*                                                                     */
/*        PARAMETER :  cmd             : command                       */
/*                     data            :                               */
/*                     *termp          :                               */
/*                     *compatflags    :                               */
/*                                                                     */
/*        RETURN : Termio value                                        */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        ioctl for bsd43 pty driver.                                  */
/*        The following IOCTL are processed:                           */
/*             TIOCGETP  termios to sgttyb                             */
/*             TIOCSETP  sgttyb to termios                             */
/*             TIOCSETN  sgttyb to termios                             */
/*             TIOCGETC  termios to tchars                             */
/*             TIOCSETC  tchars to termios                             */
/*             TIOCGLTC  termios to ltchars                            */
/*             TIOCSLTC  ltchars to termios                            */
/*             TIOCLGET  get termios flag from ldtty_compatgetflags    */
/*             TIOCLBIS  set termios flag with flags_to_termios        */
/*             TIOCLBIC  set termios flag with flags_to_termios        */
/*             TIOCLSET  set termios flag with flags_to_termios        */
/*             TIOCGETD  return 2                                      */ 
/*             default : nothing                                       */ 
/*                                                                     */
/***********************************************************************/

static int
pty_bsd43_ioctl(
                int             cmd,
                caddr_t         data,
                struct termios  *termp,
                int             *compatflagsp
                )

{
   int     ret=0;
   int     tmpflags;
   int     pflags;
   INT_TRACE_OUT


   /* trace in */
   PTY_PRINT_IN("pty_bsd43_ioctl IN",NULL);
   PTY_PRINT_CMD("pty_bsd43_ioctl",cmd);

   switch(cmd) {
      case TIOCGETP:
         termios_to_sgttyb(termp,(struct sgttyb *)data);
         break;

      case TIOCSETP:
      case TIOCSETN:
         sgttyb_to_termios((struct sgttyb *)data,termp,compatflagsp);
         ret=(cmd == TIOCSETP)? TIOCSETAF : TIOCSETA;
         PTY_TRACE_OUT(trace_out = out_2);
         break;
   
      case TIOCGETC:
         termios_to_tchars(termp,(struct tchars *)data);
         PTY_TRACE_OUT(trace_out = out_3);
         break;
   
      case TIOCSETC:
         tchars_to_termios((struct tchars *)data,termp);
         ret=TIOCSETA;
         PTY_TRACE_OUT(trace_out = out_4);
         break;
   
      case TIOCGLTC:
         termios_to_ltchars(termp,(struct ltchars *)data);
         PTY_TRACE_OUT(trace_out = out_5);
         break;
   
      case TIOCSLTC:
         ltchars_to_termios((struct ltchars *)data,termp);
         ret=TIOCSETA;
         PTY_TRACE_OUT(trace_out = out_6);
         break;
   
      case TIOCLGET:
         pflags=-1;
         tmpflags=ldtty_compatgetflags(termp->c_iflag,
                                       termp->c_lflag,
                                       termp->c_oflag,
                                       termp->c_cflag,
                                       &pflags);
   
         *(int *)data=tmpflags >> 16;
         PTY_TRACE_OUT(trace_out = out_7);
         break;
   
      case TIOCLBIS:
      case TIOCLBIC:
      case TIOCLSET:
         flags_to_termios(cmd,*(int *)data,termp,compatflagsp);
         ret=TIOCSETA;
         PTY_TRACE_OUT(trace_out = out_8);
         break;
   
      case TIOCGETD:
         /*
          * This ioctl doesn't have much meaning in STREAMS.
          * It's apparently only used by things wanting to know
          * if job control is supported,and they view "2" as a yes
          * answer,so we RETURN 2 for binary compatibility,since
          * we do support job control.
          */
         *(int *)data=2;
         PTY_TRACE_OUT(trace_out = out_9);
         break;
   
      case TIOCSETD:
         /* NOOP in streams -- just fall through */
   
      default:
         PTY_TRACE_OUT(trace_out = out_10);
         break;
   }

   /* Exit function */
   PTY_PRINT_OUT(trace_out,"pty_bsd43_ioctl",ret);
   return(ret);
}

/*        --- End of pty_bsd43_ioctl() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_ctl()                                         */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     command :                                       */
/*                     size    : flag                                  */
/*                                                                     */
/*        RETURN : A zero RETURN value indicates a memory allocation   */
/*                 problem, otherwise address of current message is    */
/*                 returned.                                           */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        build the specified M_CTL message.                           */ 
/*                                                                     */
/***********************************************************************/

static mblk_t *
pty_ctl(
        queue_t *q,
        int     command,
        int     size 
     )

{
   register mblk_t         *mp;
   register mblk_t         *mp1;
   register struct iocblk  *iocp;
   INT_TRACE_OUT



   /* trace in */
   PTY_PRINT_IN("pty_ctl IN",NULL);

   PTY_ASSERT(q != NULL);

   if(!(mp=allocb(sizeof(struct iocblk),BPRI_MED))) {

      /* Exit function */
      PTY_PRINT_OUT(trace_out,"pty_ctl",0);
      return(0);
   }
   if(size) {
      if(!(mp1=allocb(size,BPRI_MED))) {
         freemsg(mp);

         /* Exit function */
         PTY_TRACE_OUT(trace_out = out_2);
         PTY_PRINT_OUT(trace_out,"pty_ctl",0);
         return(0);
      }
   } 
   else mp1=(mblk_t *)0;

   mp->b_datap->db_type=M_CTL;
   mp->b_cont=mp1;
   mp->b_wptr=mp->b_rptr + sizeof(struct iocblk);
   iocp=(struct iocblk *)mp->b_rptr;
   iocp->ioc_cmd=command;
   iocp->ioc_count=size;
   iocp->ioc_error=iocp->ioc_rval=0;

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_3);
   PTY_PRINT_OUT(trace_out,"pty_ctl",(int)mp);
   return(mp);
}

/*        --- end of pty_ctl() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pts_put()                                         */
/*                                                                     */
/*        PARAMETER :  *ptp    : pty_s structure pointer               */
/*                     *mp     : message block pointer                 */
/*                     kind_put: PUTHERE or PUTNEXT                    */
/*                                                                     */
/*        RETURN :nothing					       */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        The pts_put routine is responsible for moving data to        */
/*        the slave queue and to check that slave is still open        */
/*                                                                     */
/***********************************************************************/
static void pts_put(
                    register struct pty_s  *ptp,
                    register mblk_t *mp,
                    int kind_put 
                   )

{
   INT_PTYSAVPRI_LRC;
   INT_TRACE_OUT

   /* trace in */
   PTY_PRINT_IN("pts_put IN",NULL);
   PTY_PRINT_MBLK_T("pts_put",mp);

   PTY_ASSERT(ptp != NULL);
   PTY_ASSERT(mp != NULL);

   /* Sanity check (slave device not open) */
   PTY_LOCK(&ptp->lock_ptp);
   if(!(ptp->pt_flags & PF_SOPEN)) {
      PTY_UNLOCK(&ptp->lock_ptp);
      freemsg(mp);

      /* Exit function */
      PTY_PRINT_OUT(trace_out,"pts_put",0);
      return;
   }
   ptp->pt_flags |= PF_PTS_XFER;
   PTY_UNLOCK(&ptp->lock_ptp);

   if (kind_put==PUTHERE)
   puthere(ptp->pt_srq,mp);
   else putnext(ptp->pt_srq,mp);

   PTY_LOCK(&ptp->lock_ptp);
   ptp->pt_flags &= ~PF_PTS_XFER;
   if (ptp->pt_flags & PF_SWXFER) {
      PTY_UNLOCK(&ptp->lock_ptp);
      e_wakeup(&ptp->pt_swait_xfer);
   }
   else {
      PTY_UNLOCK(&ptp->lock_ptp);
   }

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_2);
   PTY_PRINT_OUT(trace_out,"pts_put",0);
   return;
}

/*	 --- End of pts_put() function ---	 */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : ptm_put()                                         */
/*                                                                     */
/*        PARAMETER :  *ptp    : pty_s structure pointer               */
/*                     *mp     : message block pointer                 */
/*                     kind_put: PUTHERE or PUTNEXT                    */
/*                                                                     */
/*        RETURN :nothing					       */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        The ptm_put routine is responsible for moving data to        */
/*        the master queue and to check that master is still open      */
/*                                                                     */
/***********************************************************************/
static void ptm_put(
                    register struct pty_s  *ptp,
                    register mblk_t *mp,
                    int kind_put 
                   )

{
   INT_PTYSAVPRI_LRC;
   INT_TRACE_OUT

   /* trace in */
   PTY_PRINT_IN("ptm_put IN",NULL);
   PTY_PRINT_MBLK_T("ptm_put",mp);

   PTY_ASSERT(ptp != NULL);
   PTY_ASSERT(mp != NULL);

   /* Sanity check (master device not open) */
   PTY_LOCK(&ptp->lock_ptp);
   if(!(ptp->pt_flags & PF_MOPEN)) {
      PTY_UNLOCK(&ptp->lock_ptp);
      freemsg(mp);

      /* Exit function */
      PTY_PRINT_OUT(trace_out,"ptm_put",0);
      return;
   }
   ptp->pt_flags |= PF_PTM_XFER;
   PTY_UNLOCK(&ptp->lock_ptp);

   if (kind_put==PUTHERE)
   puthere(ptp->pt_mrq,mp);
   else putnext(ptp->pt_mrq,mp);

   PTY_LOCK(&ptp->lock_ptp);
   ptp->pt_flags &= ~PF_PTM_XFER;
   if (ptp->pt_flags & PF_MWXFER) {
      PTY_UNLOCK(&ptp->lock_ptp);
      e_wakeup(&ptp->pt_mwait_xfer);
   }
   else {
      PTY_UNLOCK(&ptp->lock_ptp);
   }

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_2);
   PTY_PRINT_OUT(trace_out,"ptm_put",0);
   return;
}

/*	 --- End of ptm_put() function ---	 */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_ptm_xfer()                                    */
/*                                                                     */
/*        PARAMETER :  *ptp    : pty_s structure pointer               */
/*                     *mp     : message block pointer                 */
/*                                                                     */
/*        RETURN :If the passed mp is an M_DATA type a RETURN value of */
/*                0 indicates the message was enqueued to the master.  */
/*                A RETURN value of FLOW_CONTROL indicates a flow      */
/*                control situation. The RETURN value is always zero   */
/*                for messages other than M_DATA.                      */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        The pty_ptm_xfer routine is responsible for moving data to   */
/*        the System V master.This routine should only be called on a  */
/*        pty whose master and slave are both open.                    */
/*        the following messages are processed                         */
/*                  M_DATA                                             */
/*                  M_FLUSH                                            */
/*                  default : put on master side                       */
/*                                                                     */
/***********************************************************************/

static int
pty_ptm_xfer(
         register struct pty_s  *ptp,
         register mblk_t *mp 
        )

{
   u_char               l_flush;
   INT_PTYSAVPRI_LRC;
   INT_TRACE_OUT

   /* trace in */
   PTY_PRINT_IN("pty_ptm_xfer IN",NULL);
   PTY_PRINT_MBLK_T("pty_ptm_xfer",mp);
 
   PTY_ASSERT(ptp != NULL);
   PTY_ASSERT(mp != NULL);

   /* Sanity check (master device not open) */
   PTY_LOCK(&ptp->lock_ptp);
   if(!(ptp->pt_flags & PF_MOPEN)) { 
      PTY_UNLOCK(&ptp->lock_ptp);

      /* Exit function */
      /* No such device */
      PTY_PRINT_OUT(trace_out,"pty_ptm_xfer",ENODEV);
      return(ENODEV);
   }
   else {
      switch(mp->b_datap->db_type) {
         case M_DATA:
            PTY_PRINT("pty_ptm_xfer : M_DATA");
   
             /* flow control (M_START/M_STOP) */
            if(ptp->pt_flags & PF_TTSTOP) {
               PTY_UNLOCK(&ptp->lock_ptp);
   
               /* Exit function */
               PTY_TRACE_OUT(trace_out = out_2);
               PTY_PRINT_OUT(trace_out,"pty_ptm_xfer",FLOW_CONTROL);
   
               /* Flow control */
               return(FLOW_CONTROL);
            }
            else {

               /* Packet mode */
               if(((ptp->pt_flags & PF_PKT) || (ptp->pt_flags & PF_UCNTL)) &&
                  (ptp->pt_flags & PF_MOPEN) &&
                  (ptp->pt_send || ptp->pt_ucntl)) {
                     PTY_UNLOCK(&ptp->lock_ptp);
                     ptm_send_pckt(ptp->pt_mrq);
                     PTY_LOCK(&ptp->lock_ptp);
               }
         
               /*
                * Zero length M_DATA coming from the slave write side should
                * be ignored.(See sysV book.)
                */
               if(mp->b_rptr == mp->b_wptr) {
                  freemsg(mp);
            
                  /* Exit function */
                  PTY_UNLOCK(&ptp->lock_ptp);
                  PTY_TRACE_OUT(trace_out = out_3);
                  PTY_PRINT_OUT(trace_out,"pty_ptm_xfer",0);
                  return(0);
               }
      
               if(canput(ptp->pt_mrq)) {
                  PTY_UNLOCK(&ptp->lock_ptp);
                  ptm_put(ptp,mp,PUTHERE);

                  /* Exit function */
                  PTY_TRACE_OUT(trace_out = out_4);
                  PTY_PRINT_OUT(trace_out,"pty_ptm_xfer",0);
                  return(0);
               } 
               else {
                  ptp->pt_flags  |= PF_SFLOW;
                  
                  /* Exit function */
                  PTY_UNLOCK(&ptp->lock_ptp);
                  PTY_TRACE_OUT(trace_out = out_5);
                  /* Flow control */
                  PTY_PRINT_OUT(trace_out,"pty_ptm_xfer",FLOW_CONTROL);
                  return(FLOW_CONTROL);
               }
            } 
            break;
      
         case M_FLUSH:
            PTY_UNLOCK(&ptp->lock_ptp);
            flush_switch_flags(mp->b_rptr);
            ptm_put(ptp,mp,PUTHERE); 
            PTY_TRACE_OUT(trace_out = out_6);
            break;
   
         default:
            PTY_UNLOCK(&ptp->lock_ptp);
            ptm_put(ptp,mp,PUTHERE);
            PTY_TRACE_OUT(trace_out = out_7);
            break;
      }
   
      /* Exit function */
      PTY_PRINT_OUT(trace_out,"pty_ptm_xfer",0);
      return(0);
   }
}
   
/*        --- End of pty_ptm_xfer() function ---        */

/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_pts_xfer()                                    */
/*                                                                     */
/*        PARAMETER :  *ptp    : pty_s structure pointer               */
/*                     *mp     : message block pointer                 */
/*                                                                     */
/*        RETURN :If the passed mp is an M_DATA type a RETURN value of */
/*                0 indicates the message was enqueued to the slave .  */
/*                A RETURN value of FLOW_CONTROL indicates a flow      */
/*                control situation. The RETURN value is always zero   */
/*                for messages other than M_DATA.                      */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        The pty_pts_xfer routine is responsible for moving data to   */
/*        the System V slave.This routine should only be called on a   */
/*        pty whose master and slave are both open.                    */
/*        The following messages are processed:                        */
/*                  M_DATA                                             */
/*                  M_FLUSH                                            */
/*                  default : put on slave  side                       */
/*                                                                     */
/***********************************************************************/

static int
pty_pts_xfer(
         register struct pty_s  *ptp,
         register mblk_t *mp
        )

{
   u_char  l_flush;

   INT_PTYSAVPRI_LRC;
   INT_TRACE_OUT

   /* trace in */
   PTY_PRINT_IN("pty_pts_xfer IN",NULL);
   PTY_PRINT_MBLK_T("pty_pts_xfer",mp);

   PTY_ASSERT(ptp != NULL);
   PTY_ASSERT(mp != NULL);


   /* Sanity check (slave device not open) */
   PTY_LOCK(&ptp->lock_ptp);
   if(!(ptp->pt_flags & PF_SOPEN)) { 
      PTY_UNLOCK(&ptp->lock_ptp);

      /* Exit function */
      /* No such device */
      PTY_PRINT_OUT(trace_out,"pty_pts_xfer",ENODEV);
      return(ENODEV);
   }
   else {
      switch(mp->b_datap->db_type) {
         case M_DATA:
            PTY_PRINT("pty_pts_xfer : M_DATA");
   
            /* flow control (M_STARTI/M_STOPI) */
            if(ptp->pt_flags & PF_TTINSTOP) {
   
               /* Exit function */
               PTY_UNLOCK(&ptp->lock_ptp);
               PTY_TRACE_OUT(trace_out = out_2);
               PTY_PRINT_OUT(trace_out,"pty_pts_xfer",FLOW_CONTROL);
   
               /* Flow control */
               return(FLOW_CONTROL);
            }
            else {
   
               if(canput(ptp->pt_srq)) {
                  PTY_UNLOCK(&ptp->lock_ptp);
                  pts_put(ptp,mp,PUTHERE);

                  /* Exit function */
                  PTY_TRACE_OUT(trace_out = out_3);
                  PTY_PRINT_OUT(trace_out,"pty_pts_xfer",0);
                  return(0);
               }
               else {
                  ptp->pt_flags  |= PF_MFLOW;
      
                  /* Exit function */
                  PTY_UNLOCK(&ptp->lock_ptp);
                  PTY_TRACE_OUT(trace_out = out_4);
      
                  /* Flow control */
                  PTY_PRINT_OUT(trace_out,"pty_pts_xfer",FLOW_CONTROL);
                  return(FLOW_CONTROL);
               }
            }
            break;
   
         case M_FLUSH:
            PTY_UNLOCK(&ptp->lock_ptp);
            flush_switch_flags(mp->b_rptr);
            pts_put(ptp,mp,PUTHERE); 
            PTY_TRACE_OUT(trace_out = out_5);
            break;
   
         default:
            PTY_UNLOCK(&ptp->lock_ptp);
            pts_put(ptp,mp,PUTHERE);
            PTY_TRACE_OUT(trace_out = out_6);
            break;
      }
   
      /* Exit function */
      PTY_PRINT_OUT(trace_out,"pty_pts_xfer",0);
      return(0);
   }
}
   
/*        --- End of pty_pts_xfer() function ---        */

/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_create_ptp()                                  */
/*                                                                     */
/*        PARAMETER pt_o_ptp   : structure to init	               */
/*                  q          : queue				       */
/*                  minor      : slave minor			       */
/*                  mode       : ATT or BSD			       */
/*                  call_from  : SLAVE or MASTER		       */
/*                                                                     */
/*        RETURN : error on failure 0 otherwise                        */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        create a new pty_s structure 				       */
/*                                                                     */
/***********************************************************************/
static int
pty_create_ptp(struct pty_s **pt_o_ptp,
               queue_t *q,
               uint    minor,
               int     mode,
               int     call_from
              )
{

   INT_TRACE_OUT
   int fmode;
   char fname[15];
   int error;
   struct pty_s *ptp;


   /* trace in */
   PTY_PRINT_IN("pty_create_ptp IN",NULL);

   if(!(*pt_o_ptp=ptp=(struct pty_s *)he_alloc(sizeof(struct pty_s),
                       BPRI_MED))) {
      /* Not enough space */
      PTY_PRINT_OUT(trace_out,"pty_pty_create_ptp",ENOMEM);
      return(ENOMEM);
   }

   /* pin memory */
   if(error = pin(ptp, sizeof(struct pty_s))) {

      /* Exit function */
      PTY_TRACE_OUT(trace_out = out_2);
   
      /* error return by pin */
      PTY_PRINT_OUT(trace_out,"pty_create_ptp",error);
       return(error);
   }

   /* link private data structure to q_ptr */
   q->q_ptr=(char *)ptp;
   OTHERQ(q)->q_ptr=(char *)ptp;

   /* Init private data structure */
   bzero(ptp,sizeof(struct pty_s));

   /* init event */
   ptp->pt_wait_master=EVENT_NULL;
   ptp->pt_swait_drain=EVENT_NULL;
   ptp->pt_mwait_drain=EVENT_NULL;
   ptp->pt_swait_xfer=EVENT_NULL;
   ptp->pt_mwait_xfer=EVENT_NULL;

   /* record slave minor */
   ptp->pt_minor=minor;

   /* record slave and master read/write queue */
   if(call_from == MASTER_CALL) {
      ptp->pt_mrq=q;
      ptp->pt_mwq=OTHERQ(q);
   }
   else {
      ptp->pt_srq=q;
      ptp->pt_swq=OTHERQ(q);
   }

   /* default termios settings */
   bcopy((caddr_t)ttydefchars,(caddr_t)ptp->pt_tio.c_cc,
          sizeof(ptp->pt_tio.c_cc));
   ptp->pt_tio.c_iflag=TTYDEF_IFLAG;
   ptp->pt_tio.c_oflag=TTYDEF_OFLAG;
   ptp->pt_tio.c_lflag=TTYDEF_LFLAG;
   ptp->pt_tio.c_cflag=TTYDEF_CFLAG;
   if(ptp->pt_tio.c_cflag & CREAD) ptp->pt_flags |= PF_CREAD;
   else ptp->pt_flags &= ~PF_CREAD;
   if(ptp->pt_tio.c_iflag & IXON) ptp->pt_flags |= PF_IXON;
   else ptp->pt_flags &= ~PF_IXON;
   if(ptp->pt_tio.c_iflag & IXOFF) ptp->pt_flags |= PF_IXOFF;
   else ptp->pt_flags &= ~PF_IXOFF;
   if(ptp->pt_tio.c_iflag & IXANY) ptp->pt_flags |= PF_IXANY;
   else ptp->pt_flags &= ~PF_IXANY;
   cfsetospeed(&ptp->pt_tio,TTYDEF_SPEED);
   cfsetispeed(&ptp->pt_tio,TTYDEF_SPEED);
   ptp->pt_vstart=ptp->pt_tio.c_cc[VSTART];
   ptp->pt_vstop=ptp->pt_tio.c_cc[VSTOP];

   /* record ATT or BSD setting */
   ptp->pt_mode=mode;      

   /* lock */
   lock_alloc(&(ptp->lock_ptp),
   LOCK_ALLOC_PIN,PTY_LOCK_CLASS,ident++);
   simple_lock_init(&(ptp->lock_ptp));
 
   /* Init mode of device and name of slave device */
   if(mode == BSD) {

      /* BSD naming mode */
      sprintf(ptp->pt_name,"tty");
      ptp->pt_name[3]="pqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      [ptp->pt_minor >> 4];
      ptp->pt_name[4]="0123456789abcdef"[ptp->pt_minor & 0x0f];
      ptp->pt_name[5]=0;
   }
   else {

       /* AT&T naming mode */
       sprintf(ptp->pt_name,"pts/%d\0",ptp->pt_minor);
   }
   sprintf(fname,"/dev/%s",ptp->pt_name);
   fmode=0666; /* rw-rw-rw-*/
   error=pty_setnameattr(fname,V_MODE,fmode, 0, 0, 0);

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_3);
   PTY_PRINT_OUT(trace_out,"pty_create_ptp",error);
   return(error);
}

/*	--- end of pty_create_ptp() function ---		*/

/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_open_comm()                                   */
/*                                                                     */
/*        PARAMETER :  *q          : streams queue pointer             */
/*                     *devp       : master or slave dev pair          */
/*                     mode        : AT&T or BSD                       */
/*                     call_from   : MASTER_CALL or SLAVE_CALL         */
/*                                                                     */
/*        RETURN : error on failure 0 otherwise                        */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        create a new pty_s structure and link it to the others       */
/*        Common open and close routines.                              */
/*                                                                     */
/***********************************************************************/

int
pty_open_comm(
                queue_t *q,
                dev_t   *devp,
                int     mode,
                int     call_from
                )
{
   register struct pty_o_s * pty_o;        
   register struct pty_o_s * last_pty_o;   
   int	dev_minor;
   int	error;
   INT_TRACE_OUT
   INT_PTYSAVPRI_LRC;


#ifdef PTYDD_DEBUG
   /************** Only to have a kdb access to pty_kdb() ********************/
   /* NEVER EXECUTED -->*/ if(mode==PTY_KDB)pty_kdb() ; /*<--NEVER EXECUTED  */
   /************** Only to have a kdb access to pty_kdb() ********************/
#endif /* PTYDD_DEBUG */

   /* trace in */
   PTY_PRINT_IN("pty_open_comm IN",NULL);

   if(!q) /* Sanity check */
   {       
      /* Exit function */
      PTY_PRINT_OUT(trace_out,"pty_open_comm",EFAULT); /* Bad address */ 
      return(EFAULT);
   }

   if(mode == ATT)pty_o=pty_s_head_ATT;
   else pty_o=pty_s_head_BSD;

   dev_minor=minor(*devp); 
   last_pty_o=pty_o;
   PTY_ASSERT(last_pty_o != NULL);

   if(mode == ATT) {
      /* we have to create a new entry as soon as we find one free */       
      for(;;) {

         /*******************************************************/
	 /* TRY TO OPEN SLAVE FIRST,NEW LIST ENTRY (ATT MODE)	*/
         /*******************************************************/

         /* It is not possible to open slave before master in AT&T mode  */
         if(!pty_o && call_from == SLAVE_CALL) {
                      
            /* Exit function */
            PTY_TRACE_OUT(trace_out = out_2);

            /* resource temporarily unavailable */
            PTY_PRINT_OUT(trace_out,"pty_open_comm",EAGAIN); 
            return(EAGAIN);
         }

	 /*************************************************/
	 /* OPEN MASTER FIRST, NEW LIST ENTRY (ATT MODE)  */
	 /*************************************************/

         /* Here we create a new entry if we could */
         if(!pty_o && call_from == MASTER_CALL) {

            /* check if we have enough room */
            if(last_pty_o->pt_o_minor + 1 >= NRPTY(mode)) {

               /* Exit function */
               PTY_TRACE_OUT(trace_out = out_3);

               /* Too many open files */
               PTY_PRINT_OUT(trace_out,"pty_open_comm",EMFILE); 
               return(EMFILE);
            }

            /* create pty_o new entry */
            if(!(pty_o=(struct pty_o_s *)he_alloc(sizeof(struct pty_o_s), 
                                                  BPRI_MED))) {

               /* Exit function */
               PTY_TRACE_OUT(trace_out = out_4);

               /* Not enough space */
               PTY_PRINT_OUT(trace_out,"pty_open_comm",ENOMEM); 
               return(ENOMEM);
            }
            bzero(pty_o,sizeof(struct pty_o_s));

            /* increment device minor value */
            pty_o->pt_o_minor=last_pty_o->pt_o_minor + 1;
      
            /* create ptp structure and link it to current queue structure */
            if (error=pty_create_ptp(&pty_o->pt_o_ptp,
                                     q,
                                     pty_o->pt_o_minor,
                                     mode,
                                     call_from)){
               /* free pty_o not available ! */
               he_free((struct pty_o_s *)pty_o);
  
               /* Exit function */
               PTY_TRACE_OUT(trace_out = out_5);
               PTY_PRINT_OUT(trace_out,"pty_open_comm",error); 
               return(error);
            }
            /* Turn on flag to assign this entry */
            pty_o->pt_o_flag=PF_MOPEN; /* master is opened now */
   
            /* link with last one */
            last_pty_o->pt_o_next=pty_o;
   
            PTY_TRACE_OUT(trace_out = out_6);
            goto out;
         }

        /*******************************************************/
        /* TRY TO OPEN SLAVE FIRST, USE OLD ENTRY  (ATT MODE)  */
        /*******************************************************/

         /* An entry exists but master is not opened and we try to open slave 
            it is impossible */
         if(!(pty_o->pt_o_flag) && (pty_o->pt_o_minor == dev_minor) &&
             (call_from == SLAVE_CALL)) {

            /* Exit function */
            PTY_TRACE_OUT(trace_out = out_7);

            /* Operation not permitted */
            PTY_PRINT_OUT(trace_out,"pty_open_comm",EPERM); 
            return(EPERM);
         }

	 /***********************************************/
	 /* OPEN MASTER FIRST, USE OLD ENTRY (ATT MODE) */
	 /***********************************************/

         /* An entry exists and we use now this entry to open master */
         if((!pty_o->pt_o_flag) && (call_from == MASTER_CALL)) {

            /* create ptp structure and link it to current queue structure */
            if (error=pty_create_ptp(&pty_o->pt_o_ptp,
                                     q,
                                     pty_o->pt_o_minor,
                                     mode,
                                     call_from)) {

               /* Exit function */
               PTY_TRACE_OUT(trace_out = out_8);
               PTY_PRINT_OUT(trace_out,"pty_open_comm",error); 
               return(error);
            }

            /* Turn on flag to assign this entry */
            pty_o->pt_o_flag  |= PF_MOPEN; /* master is opened now */
            PTY_TRACE_OUT(trace_out = out_9);
            goto out;
         }

	 /************************************************************/
	 /* OPEN SLAVE WHEN MASTER OPENED, USE OLD ENTRY (ATT MODE)  */
	 /************************************************************/

         /* master is already opened, we just open slave, if slave already 
            opened, we just fall here and RETURN silently */

         if((pty_o->pt_o_flag & PF_MOPEN) && (call_from == SLAVE_CALL) && 
            (pty_o->pt_o_minor == dev_minor)) {

            /* Slave is not yet opened */
            if(!(pty_o->pt_o_flag & PF_SOPEN)) {
               pty_o->pt_o_flag  |= PF_SOPEN; /* slave is opened now */
               q->q_ptr=(char *)pty_o->pt_o_ptp;
               pty_o->pt_o_ptp->pt_swq=OTHERQ(q);
               OTHERQ(q)->q_ptr=(char *)pty_o->pt_o_ptp;
               pty_o->pt_o_ptp->pt_srq=q;
            }

            /* else we just reopen slave and exit silently */

            PTY_TRACE_OUT(trace_out = out_10);
            goto out;
         }

         /* go ahead we look for a new entry */
         last_pty_o=pty_o;
         pty_o=pty_o->pt_o_next;
      }
   }

   /* call from master BSD,here it is possible to open slave before master */

   else if(mode == BSD) {
         for(;;) {

	    /**********************************************************/
	    /* OPEN MASTER OR SLAVE FIRST, NEW LIST ENTRY (BSD MODE)  */
	    /**********************************************************/

            if(!pty_o) {

               /* check if mknod should exist */
               if(dev_minor >= NRPTY(mode)) {
   
                  /* Exit function */
                  PTY_TRACE_OUT(trace_out = out_11);

                  /* Too many open files */
                  PTY_PRINT_OUT(trace_out,"pty_open_comm",EMFILE); 
                  return(EMFILE);
               }

               /* create a new entry */
               if(!(pty_o=(struct pty_o_s *)he_alloc(sizeof(struct pty_o_s),
                                                     BPRI_MED))) {
   
                  /* Exit function */
                  PTY_TRACE_OUT(trace_out = out_12);

                  /* Not enough space */
                  PTY_PRINT_OUT(trace_out,"pty_open_comm",ENOMEM); 
                  return(ENOMEM);
               }
               bzero(pty_o,sizeof(struct pty_o_s));
   
               /* initialize device minor value */
               pty_o->pt_o_minor=dev_minor;

               /* create ptp structure and link it to current queue structure */
               if(error=pty_create_ptp(&pty_o->pt_o_ptp,
                                       q,
                                       pty_o->pt_o_minor,
                                       mode,
                                       call_from)) {
                  he_free((struct pty_o_s *)pty_o);
   
                  /* Exit function */
                  PTY_TRACE_OUT(trace_out = out_13);
                  PTY_PRINT_OUT(trace_out,"pty_open_comm",error); 
                  return(error);
               }

	       /* Turn on flag to assign this entry */
               if(call_from == MASTER_CALL) pty_o->pt_o_flag  |= PF_MOPEN; 
               else pty_o->pt_o_flag  |= PF_SOPEN; 
   
               /* link with last one */
               last_pty_o->pt_o_next=pty_o;
                                   
               PTY_TRACE_OUT(trace_out = out_14);
               goto out;
            }

            /**********************************************************/
            /* OPEN MASTER OR SLAVE FIRST, OLD LIST ENTRY (BSD MODE)  */
            /**********************************************************/

            /* Otherwise we look for the entry if neither master nor slave is 
               opened */
            if((pty_o->pt_o_minor == dev_minor) && (!pty_o->pt_o_flag)) {
               if(error=pty_create_ptp(&pty_o->pt_o_ptp,
                                       q,
			               pty_o->pt_o_minor,
                                       mode,
                                       call_from)) {

                  /* Exit function */
                  PTY_TRACE_OUT(trace_out = out_15);
                  PTY_PRINT_OUT(trace_out,"pty_open_comm",error); 
                  return(error);
               }

	       /* Turn on flag to assign this entry */
               if(call_from == MASTER_CALL) pty_o->pt_o_flag  |= PF_MOPEN; 
               else pty_o->pt_o_flag  |= PF_SOPEN; 
   
               PTY_TRACE_OUT(trace_out = out_16);
               goto out;
            }

            /**********************************************************/
            /* TRY TO OPEN MASTER TWICE, NEW LIST ENTRY (BSD MODE)    */
            /**********************************************************/

            /* If we try to reopen master we RETURN an error */
            if((pty_o->pt_o_minor == dev_minor) && 
               (pty_o->pt_o_flag & PF_MOPEN) && 
               (call_from == MASTER_CALL)) {

               /* Exit function */
               PTY_TRACE_OUT(trace_out = out_17);

               /* Permission denied */
               PTY_PRINT_OUT(trace_out,"pty_open_comm",EACCES); 
               return(EACCES);
            }
   
            /**********************************************************/
            /* OPEN MASTER XOR SLAVE TO COMPLETE CONNECTION, OLD LIST */
            /* ENTRY (BSD MODE)                                       */
            /**********************************************************/

            /* Otherwise slave or(exclusive)master already opened and we 
               complete structure or RETURN silently */
            if((pty_o->pt_o_minor == dev_minor) && pty_o->pt_o_flag) {

               /* Open master side */
               if((call_from == MASTER_CALL) && 
                  (!(pty_o->pt_o_flag & PF_MOPEN))) {
                  pty_o->pt_o_flag  |= PF_MOPEN; /* master is opened now */
                  pty_o->pt_o_ptp->pt_mwq=OTHERQ(q);
                  q->q_ptr=(char *)pty_o->pt_o_ptp;
                  OTHERQ(q)->q_ptr=(char *)pty_o->pt_o_ptp;
                  pty_o->pt_o_ptp->pt_mrq=q;
               }

               /* Open slave side */
               else if(call_from == SLAVE_CALL) {
                       pty_o->pt_o_flag  |= PF_SOPEN; /* slave is opened now */
                       pty_o->pt_o_ptp->pt_swq=OTHERQ(q);
                       pty_o->pt_o_ptp->pt_srq=q;
                       q->q_ptr=(char *)pty_o->pt_o_ptp;
                       OTHERQ(q)->q_ptr=(char *)pty_o->pt_o_ptp;
               }

               /* else we just reopen slave and exit silently */
               PTY_TRACE_OUT(trace_out = out_18);
               goto out;
            }
                   
         /* go ahead */
         last_pty_o=pty_o;
         pty_o=pty_o->pt_o_next;
      }       
   }
   
out:

   *devp=makedev(major(*devp),pty_o->pt_o_minor);

   /* Exit function */
   PTY_PRINT_OUT(trace_out,"pty_open_comm",0);
   return(0);
}

/*        --- End of pty_open_comm() function ---        */


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : call_pty_open_comm()                              */
/*                                                                     */
/*        PARAMETER :  *q          : streams queue pointer             */
/*                     *devp       : master or slave dev pair          */
/*                     mode        : AT&T or BSD                       */
/*                     call_from   : MASTER_CALL or SLAVE_CALL         */
/*                                                                     */
/*        RETURN : error on failure 0 otherwise                        */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Call pty_open_comm as a critical section with open_close     */
/*        token                                                        */
/*                                                                     */
/***********************************************************************/
int
call_pty_open_comm(
                queue_t *q,
                dev_t   *devp,
                int     mode,
                int     call_from
                  )

{

   int error;
   INT_TRACE_OUT
   INT_PTYSAVPRI_LRC;

   /* trace in */
   PTY_PRINT_IN("call_pty_open_comm IN",NULL);

   PTY_LOCK(&lock_open_close);

   /* if USED then we wait until routine is ready to used */
   while(open_close & USED) {
      open_close |= GO_TO_SLEEP;
      PTY_UNLOCK(&lock_open_close);
      if(e_sleep(&wait_open_close, EVENT_SIGRET) == EVENT_SUCC) {
         PTY_LOCK(&lock_open_close);
         break;
      }
      else {
         PTY_LOCK(&lock_open_close);
      }
   }
   
   /* We take the token */
   open_close |= USED;
   PTY_UNLOCK(&lock_open_close);

   PTY_PRINT("pts_open : call pty_open_comm");
   if(error=pty_open_comm(q,devp,mode,call_from)) {

      /* If GO_TO_SLEEP wake up everybody */
      PTY_LOCK(&lock_open_close);
      if (open_close & GO_TO_SLEEP) {
         open_close &= ~(USED | GO_TO_SLEEP);
         PTY_UNLOCK(&lock_open_close);
         e_wakeup(&wait_open_close);  
      }
      else {
         open_close &= ~USED;
         PTY_UNLOCK(&lock_open_close);
      }
            
      /* Exit function */
      PTY_PRINT_OUT(trace_out,"call_pty_open_comm",error);
      return(error);
   } 
   else {
 
      /* If GO_TO_SLEEP wake up everybody */
      PTY_LOCK(&lock_open_close);
      if (open_close & GO_TO_SLEEP) {
         open_close &= ~(USED | GO_TO_SLEEP);
         PTY_UNLOCK(&lock_open_close);
         e_wakeup(&wait_open_close); 
      }
      else {
         open_close &= ~USED;
         PTY_UNLOCK(&lock_open_close);
      }
   }

   /* Exit function */
   PTY_TRACE_OUT(trace_out = out_2);
   PTY_PRINT_OUT(trace_out,"call_pty_open_comm",0);
   return(0);

}

/*	--- end of call_pty_open_comm function ---	*/


/***********************************************************************/
/*                                                                     */
/*        FUNCTION : call_pty_close_comm()                             */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     call_from   : MASTER_CALL or SLAVE_CALL         */
/*                                                                     */
/*        RETURN : 0                                                   */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Call pty_close_comm as a critical section with open_close    */
/*        token                                                        */
/*                                                                     */
/***********************************************************************/
int
call_pty_close_comm(
                    queue_t * q,
                    int     call_from
                   )

{

   INT_TRACE_OUT
   INT_PTYSAVPRI_LRC;

   /* trace in */
   PTY_PRINT_IN("call_pty_close_comm IN",NULL);

   PTY_LOCK(&lock_open_close);

   /* if USED then we wait until routine is ready to used */
   while(open_close & USED) {
      open_close |= GO_TO_SLEEP;
      PTY_UNLOCK(&lock_open_close);
      if(e_sleep(&wait_open_close, EVENT_SIGRET) == EVENT_SUCC) {
         PTY_LOCK(&lock_open_close);
         break;
      }
      else {
         PTY_LOCK(&lock_open_close);
      }
   }

   /* We take the token */
   open_close |= USED;
   PTY_UNLOCK(&lock_open_close);

   (void)pty_close_comm(q,call_from);

   /* If GO_TO_SLEEP wake up everybody */
   PTY_LOCK(&lock_open_close);
   if (open_close & GO_TO_SLEEP) {
      open_close &= ~(USED | GO_TO_SLEEP);
      PTY_UNLOCK(&lock_open_close);
      e_wakeup(&wait_open_close);
   }
   else {
      open_close &= ~USED;
      PTY_UNLOCK(&lock_open_close);
   }


   /* Exit function */
   PTY_PRINT_OUT(trace_out,"call_pty_close_comm",0);
   return(0);


}
/*	--- end of call_pty_close_comm function ---	*/

/***********************************************************************/
/*                                                                     */
/*        FUNCTION : pty_close_comm()                                  */
/*                                                                     */
/*        PARAMETER :  *q      : streams queue pointer                 */
/*                     call_from   : MASTER_CALL or SLAVE_CALL         */
/*                                                                     */
/*        RETURN : 0                                                   */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Common close routine.                                        */
/*                                                                     */
/***********************************************************************/

int
pty_close_comm(
               queue_t * q,
               int     call_from
              )
 
{
   register struct pty_s   * ptp=(struct pty_s *)q->q_ptr;
   register struct pty_o_s * pty_o;
   register struct pty_o_s * last_pty_o;

   INT_TRACE_OUT

   /* trace in */
   PTY_PRINT_IN("pty_close_comm IN",NULL);

   PTY_ASSERT(ptp != NULL);

   if(ptp->pt_mode == ATT)pty_o=pty_s_head_ATT;
   else pty_o=pty_s_head_BSD;      

   for(;;) {
      if(!pty_o) {
              
         /* We should never be here */

         /* Exit function */
         PTY_PRINT_OUT(trace_out,"pty_close_comm",0);
         return(0);
      }
      else if((pty_o->pt_o_minor == ptp->pt_minor) && 
              pty_o->pt_o_flag) {
            if(call_from == MASTER_CALL) {
               if(!(pty_o->pt_o_flag & PF_SOPEN)) {
                  lock_free(&(pty_o->pt_o_ptp->lock_ptp));
	          unpin(pty_o->pt_o_ptp,sizeof(struct pty_s));
                  if(pty_o->pt_o_ptp)he_free(pty_o->pt_o_ptp);
               }
               pty_o->pt_o_flag  &= ~PF_MOPEN;
            }
            else {
               if(!(pty_o->pt_o_flag & PF_MOPEN)) {
                  lock_free(&(pty_o->pt_o_ptp->lock_ptp));
	          unpin(pty_o->pt_o_ptp,sizeof(struct pty_s));
                  if(pty_o->pt_o_ptp)he_free(pty_o->pt_o_ptp);
               }
               pty_o->pt_o_flag  &= ~PF_SOPEN;
            }
   
            /* Exit function */
            PTY_TRACE_OUT(trace_out = out_2);
            PTY_PRINT_OUT(trace_out,"pty_close_comm",0);
            return(0);
           }
      last_pty_o=pty_o;
      pty_o=pty_o->pt_o_next;
   }
}
/*        --- End of pty_close_comm() function ---        */

/* END OF PTYDD SOURCE CODE */

/* ------------------------ TRACE FUNCTION ----------------------------*/
/* ------------------------ TRACE FUNCTION ----------------------------*/
/* ------------------------ TRACE FUNCTION ----------------------------*/
/* ------------------------ TRACE FUNCTION ----------------------------*/

#ifdef PTYDD_DEBUG

/*######################### TRACE FUNCTION ############################*/
/*                                                                     */
/*        FUNCTION : pty_kdb()                                         */
/*                                                                     */
/*        PARAMETER :                                                  */
/*                                                                     */
/*        RETURN : Nothing                                             */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        this procedure print pty_o_s structure when in KDB (kernel   */
/*        debugger.)                                                   */
/*                                                                     */
/***********************************************************************/
static int
pty_kdb() 
{
   register struct pty_o_s * pty_o;
   register struct pty_o_s * last_pty_o;
   register char   device_name[10];
  
   printf("*** Welcome to KDB PTY ***\n\n");

   /* print AT&T open device */
   pty_o=pty_s_head_ATT;
   while(pty_o) {
      if(pty_o->pt_o_flag) {
         sprintf(device_name,"pts/%d\0",pty_o->pt_o_minor);
         printf("------------------------------------");
         printf("--------------------------\n");
         printf("DEVICE NAME..........: /dev/%s\n",device_name);
         if(!pty_o->pt_o_flag)printf("pty_o->pt_o_flag.....: NULL ");
         else printf("pty_o->pt_o_flag.....:");
         if(pty_o->pt_o_flag & PF_MOPEN)printf(" PF_MOPEN");
         if(pty_o->pt_o_flag & PF_SOPEN)printf(" PF_SOPEN");
         printf("\n");

         if(!pty_o->pt_o_ptp)
         printf("pty_o->pt_o_ptp.....: NULL POINTER\n");
         else {
                 
            printf("pty_o->pt_o_ptp......: 0x%x\n",pty_o->pt_o_ptp);
            pty_print(pty_o->pt_o_ptp,NULL);
         }

         printf("open_close..........:");
         if (open_close & USED) printf(" USED");
         if (open_close & GO_TO_SLEEP) printf(" GO_TO_SLEEP");
         printf("\n");
 
         printf("wait_open_close.....: 0x%x\n",wait_open_close);

      }

       pty_o=pty_o->pt_o_next;
   }


   /* print BSD open device */
   pty_o=pty_s_head_BSD;
   while(pty_o) {
      if(pty_o->pt_o_flag) {
         device_name[0]='t';
         device_name[1]='t';
         device_name[2]='y';
         device_name[3]="pqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
                         [pty_o->pt_o_minor >> 4];
         device_name[4]="0123456789abcdef"[pty_o->pt_o_minor&0x0f];
         device_name[5]=0;

         printf("------------------------------------");
         printf("--------------------------\n");
         printf("DEVICE NAME..........: /dev/%s\n",device_name);
         if(!pty_o->pt_o_flag)printf("pty_o->pt_o_flag.....: NULL ");
         else printf("pty_o->pt_o_flag.....:");
         if(pty_o->pt_o_flag & PF_MOPEN)printf(" PF_MOPEN");
         if(pty_o->pt_o_flag & PF_SOPEN)printf(" PF_SOPEN");
         printf("\n");
         if(!pty_o->pt_o_ptp)printf("pty_o->pt_o_ptp.....: NULL POINTER\n");
         else {
            printf("pty_o->pt_o_ptp......: 0x%x\n",pty_o->pt_o_ptp);
            pty_print(pty_o->pt_o_ptp,NULL);
         }
      }

      pty_o=pty_o->pt_o_next;
   }
   printf("\n*** KDB PTY END OF LIST ***\n");
}
/*        --- End of pty_kdb() function ---        */

/*######################### TRACE FUNCTION ############################*/
/*                                                                     */
/*        FUNCTION : trace_prsc()                                      */
/*                                                                     */
/*        PARAMETER :  *string         : string to print               */
/*                                                                     */
/*        RETURN : Nothing                                             */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        this procedure print character of the string but special     */
/*        characters are not sent to current output but their values   */
/*        are written in hexadecimal on the output.                    */
/*                                                                     */
/***********************************************************************/
static int
trace_prsc(
           char    *string,
           int     length   /* length of string */

           )
{
   int     cpt;    /* counter */

   cpt=0;
   for(cpt=0;cpt<length;cpt++)
   {
      if(string[cpt]<32 || string[cpt]>127)printf("[0x%x]",string[cpt]);
      else printf("%c",string[cpt]);
   }
}
/*        --- end of trace_prsc() function ---        */ 


/*######################### TRACE FUNCTION ############################*/
/*                                                                     */
/*        FUNCTION : trace()                                           */
/*                                                                     */
/*        PARAMETER :  *routine_name   : name of current routine       */
/*                                                                     */
/*        RETURN : Nothing                                             */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Trace routine used for debbuging code                        */
/*                                                                     */
/***********************************************************************/

static int
trace(
           char   *routine_name,
           int    trace
   )
{

   /* Write routine name   */


   if(trace == NULL)printf("%s \n",routine_name);
   else printf("%s 0x%x\n",routine_name,trace);

}
/*        --- End of trace() function ---        */


/*######################### TRACE FUNCTION ############################*/
/*                                                                     */
/*        FUNCTION : trace_mblk_t()                                    */
/*                                                                     */
/*        PARAMETER :  *routine_name   : name of current routine       */
/*                     *mp             :                               */
/*                                                                     */
/*        RETURN : Nothing                                             */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Trace routine used for debbuging code                        */
/*                                                                     */
/*        MBLK_T defined in sys/stream.h                               */
/*        -----                                                        */
/*                                                                     */
/*        struct  msgb {                                               */
/*              struct msgb *   b_next;                                */
/*              struct msgb *   b_prev;                                */
/*              struct msgb *   b_cont;                                */
/*              unsigned char * b_rptr;                                */
/*              unsigned char * b_wptr;                                */
/*              struct datab *  b_datap;                               */
/*              unsigned char   b_band;                                */
/*              unsigned char   b_pad1;                                */
/*              unsigned short  b_flag;                                */
/*              long       b_pad2;                                     */
/*              MSG_KERNEL_FIELDS                                      */
/*        };                                                           */
/*        typedef struct msgb     mblk_t;                              */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

static int
trace_mblk_t(
                   char      *routine_name,
                   mblk_t    *mp
           )
{
   register mblk_t *mp1;


   printf("%s : ",routine_name);
   switch(mp->b_datap->db_type)
   {
/* ------------------------------------------------------------------ */
      case M_IOCTL   :printf("mp->b_datap->db_type=M_IOCTL\n");break;
      case M_CTL     :printf("mp->b_datap->db_type=M_CTL\n");break;
/* ------------------------------------------------------------------ */
      case M_FLUSH   :printf("mp->b_datap->db_type=M_FLUSH : ");
         if(*mp->b_rptr & FLUSHR) printf("[FLUSHR]");
         if(*mp->b_rptr & FLUSHW) printf("[FLUSHW]");
         printf("\n");
         break;
/* ------------------------------------------------------------------ */
      case M_START   :printf("mp->b_datap->db_type=M_START\n");break;
      case M_STARTI  :printf("mp->b_datap->db_type=M_STARTI\n");break;
      case M_STOP    :printf("mp->b_datap->db_type=M_STOP\n");break;
      case M_STOPI   :printf("mp->b_datap->db_type=M_STOPI\n");break;
      case M_IOCNAK  :printf("mp->b_datap->db_type=M_IOCNAK\n");break;
      case M_SETOPTS :printf("mp->b_datap->db_type=M_SETOPTS\n");break;
      case M_READ    :printf("mp->b_datap->db_type=M_READ\n");break;
      case M_PCSIG   :printf("mp->b_datap->db_type=M_PCSIG\n");break;
      case M_PCRSE   :printf("mp->b_datap->db_type=M_PCRSE\n");break;
      case M_RSE     :printf("mp->b_datap->db_type=M_RSE\n");break;
      case M_HANGUP  :printf("mp->b_datap->db_type=M_HANGUP\n");break;
/* ------------------------------------------------------------------ */
      case M_PCPROTO :printf("mp->b_datap->db_type=M_PCPROTO\n");goto print;
      case M_DATA    :printf("mp->b_datap->db_type=M_DATA\n"); goto print;
print:
         mp1=mp;
         printf("%s : mp->b_rptr=",routine_name);
next_block:
         trace_prsc(mp1->b_rptr,mp1->b_wptr - mp1->b_rptr);
         if(mp1->b_cont) {
            mp1=mp1->b_cont;
            goto next_block;
         }
         printf("\n");
         break;
      default :
         printf("mp->b_datap->db_type=UNKNOWN !\n");
   }

}

/*        --- End of trace_mblk_t() function ---        */


/*######################### TRACE FUNCTION ############################*/
/*                                                                     */
/*        FUNCTION : trace_cmd()                                       */
/*                                                                     */
/*        PARAMETER :  *routine_name   : name of current routine       */
/*                     cmd             : command to print              */
/*                                                                     */
/*        RETURN : Nothing                                             */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Trace routine used for debbuging code                        */
/*                                                                     */
/***********************************************************************/

static int
trace_cmd(
           char       *routine_name,
           int     cmd
           )
{


   printf("%s : ",routine_name);
   switch(cmd)
   {

      case CFG_INIT   : printf("cmd=CFG_INIT\n");break;
      case CFG_TERM   : printf("cmd=CFG_TERM \n");break;
      case TIOCGETP   : printf("cmd=TIOCGETP \n");break;
      case TIOCSETP   : printf("cmd=TIOCSETP\n");break;
      case TIOCSETN   : printf("cmd=TIOCSETN\n");break;
      case TIOCGETC   : printf("cmd=TIOCGETC\n");break;
      case TIOCSETC   : printf("cmd=TIOCSETC\n");break;
      case TIOCGLTC   : printf("cmd=TIOCGLTC\n");break;
      case TIOCSLTC   : printf("cmd=TIOCSLTC\n");break;
      case TIOCLGET   : printf("cmd=TIOCLGET\n");break;
      case TIOCLBIS   : printf("cmd=TIOCLBIS\n");break;
      case TIOCLBIC   : printf("cmd=TIOCLBIC\n");break;
      case TIOCLSET   : printf("cmd=TIOCLSET\n");break;
      case TIOCGETD   : printf("cmd=TIOCGETD\n");break;
      case TIOCSETD   : printf("cmd=TIOCSETD\n");break;
      case TIOCUCNTL  : printf("cmd=TIOCUCNTL\n");break;
      case TIOCREMOTE : printf("cmd=TIOCREMOTE\n");break;
      case TIOCPKT    : printf("cmd=TIOCPKT\n");break;
      case TCSAK      : printf("cmd=TCSAK\n");break;
      case TCQSAK     : printf("cmd=TCQSAK\n");break;
      case TIOCSTOP   : printf("cmd=TIOCSTOP\n");break;
      case TIOCSTART  : printf("cmd=TIOCSTART\n");break;
      case TXTTYNAME  : printf("cmd=TXTTYNAME\n");break;
      case TIOCGETA   : printf("cmd=TIOCGETA or TCGETS\n");break;
      case TIOCSETAF  : printf("cmd=TIOCSETAF\n");break;
      case TIOCSETA   : printf("cmd=TIOCSETA or TCSETS\n");break;
      case TIOCSETAW  : printf("cmd=TIOCSETAW\n");break;
      case TIOCEXCL   : printf("cmd=TIOCEXCL\n");break;
      case TIOCNXCL   : printf("cmd=TIOCNXCL\n");break;
      case TIOCFLUSH  : printf("cmd=TIOCFLUSH\n");break;
      case TIOCGWINSZ : printf("cmd=TIOCGWINSZ\n");break;
      case TIOCSWINSZ : printf("cmd=TIOCSWINSZ\n");break;
      case EUC_WSET   : printf("cmd=EUC_WSET\n");break;
      case EUC_WGET   : printf("cmd=EUC_WGET\n");break;
      case EUC_IXLON  : printf("cmd=EUC_IXLON\n");break;
      case EUC_IXLOFF : printf("cmd=EUC_IXLOFF\n");break;
      case EUC_OXLON  : printf("cmd=EUC_OXLON\n");break;
      case EUC_OXLOFF : printf("cmd=EUC_OXLOFF\n");break;
      case EUC_MSAVE  : printf("cmd=EUC_MSAVE\n");break;
      case EUC_MREST  : printf("cmd=EUC_MREST\n");break;
      case TIOCCONS   : printf("cmd=TIOCCONS\n");break;
      case TCSBRK     : printf("cmd=TCSBRK\n");break;
      case TCSBREAK   : printf("cmd=TCSBREAK\n");break;
      case MC_NO_CANON   : printf("cmd=MC_NO_CANON\n");break;
      case MC_CANONQUERY : printf("cmd=MC_CANONQUERY\n");break;
      case MC_DO_CANON   : printf("cmd=MC_DO_CANON\n");break;
      case MC_PART_CANON : printf("cmd=MC_PART_CANON\n");break;
      case TIOCHPCL      : printf("cmd=TIOCHPCL\n");break;

/*    case TIOCGPRP      : printf("cmd=TIOCGPRP\n");break; 
      case TCCBREAK      : printf("cmd=TCCBREAK\n");break;  
      case TIOCSCTTY     : printf("cmd=TIOCSCTTY\n");break;
      case TIOCKNOSESS   : printf("cmd=TIOCKNOSESS\n");break; 
      case TIOCKNOPGRP   : printf("cmd=TIOCKNOPGRP\n");break; 
      case TIOCKPGSIG    : printf("cmd=TIOCKPGSIG\n");break;  
      case OTIOCGETD     : printf("cmd=OTIOCGETD\n");break;
      case OTIOCSETD     : printf("cmd=OTIOCSETD\n");break; */

      case TIOCSTI       : printf("cmd=TIOCSTI\n");break;
      case TCFLSH        : printf("cmd=TCFLSH\n");break;
      case TCGETA        : printf("cmd=TCGETA\n");break;
      case TCSETA        : printf("cmd=TCSETA\n");break;
      case TCSETAW       : printf("cmd=TCSETAW\n");break;
      case TCSETAF       : printf("cmd=TCSETAF\n");break;
      case TIOCSDTR      : printf("cmd=TIOCSDTR\n");break;
      case TIOCCDTR      : printf("cmd=TIOCCDTR\n");break;
      case TIOCMSET      : printf("cmd=TIOCMSET\n");break;
      case TIOCMBIS      : printf("cmd=TIOCMBIS\n");break;
      case TIOCMBIC      : printf("cmd=TIOCMBIC\n");break;
      case TIOCMGET      : printf("cmd=TIOCMGET\n");break;
      case TXISATTY      : printf("cmd=TXISATTY\n");break;
      case TXSETIHOG     : printf("cmd=TXSETIHOG\n");break;
      case TXSETOHOG     : printf("cmd=TXSETOHOG\n");break;
      case TCTRUST       : printf("cmd=TCTRUST\n");break;
      case TCQTRUST      : printf("cmd=TCQTRUST\n");break;
      case TXGPGRP       : printf("cmd=TXGPGRP\n");break;
      case TXSPGRP       : printf("cmd=TXSPGRP\n");break;

      default: 
        if((cmd & ~0xff) == UIOCCMD(0)) 
           printf("cmd=UIOCCMD(0) value = %d\n",(cmd & 0xff));
        else printf("cmd= UNKNOWN : 0x%x\n",cmd);
   }

}
/*        --- end of trace_cmd() function ---        */


/*######################### TRACE FUNCTION ################ ###########*/
/*                                                                     */
/*        FUNCTION : trace_error()                                     */
/*                                                                     */
/*        PARAMETER :  trace_out       : output indicator              */
/*                     *routine_name   : name of current routine       */
/*                     err_number      : errno to print                */
/*                                                                     */
/*        RETURN : Nothing                                             */
/*                                                                     */
/*        PURPOSE :                                                    */
/*                                                                     */
/*        Trace error number                                           */
/*                                                                     */
/***********************************************************************/
static int
trace_error(
                int trace_out,
                char *routine_name,
                int  err_number
               )
{

   printf("%s OUT 0x%x  RETURN", routine_name, trace_out);


   switch(err_number) {
      case EPERM : 
         printf("[Operation not permitted]");
         break;
      case ENOENT : 
         printf("[No such file or directory]");
         break;
      case ESRCH : 
         printf("[No such process]");
         break;
      case EINTR : 
         printf("[interrupted system call]");
         break;
      case EIO : 
         printf("[I/O error]");
         break;
      case ENXIO : 
         printf("[No such device or address]");
         break;
      case E2BIG : 
         printf("[Arg list too long]");
         break;
      case ENOEXEC : 
         printf("[Exec format error]");
         break;
      case EBADF : 
         printf("[Bad file descriptor]");
         break;
      case ECHILD : 
         printf("[No child processes]");
         break;
      case EAGAIN : 
         printf("[Resource temporarily unavailable]");
         break;
      case ENOMEM : 
         printf("[Not enough space]");
         break;
      case EACCES : 
         printf("[Permission denied]");
         break;
      case EFAULT : 
         printf("[Bad address]");
         break;
      case ENOTBLK : 
         printf("[Block device required]");
         break;
      case EBUSY : 
         printf("[Resource busy]");
         break;
      case EEXIST : 
         printf("[File exists]");
         break;
      case EXDEV : 
         printf("[Improper link]");
         break;
      case ENODEV : 
         printf("[No such device]");
         break;
      case ENOTDIR : 
         printf("[Not a directory]");
         break;
      case EISDIR : 
         printf("[Is a directory]");
         break;
      case EINVAL : 
         printf("[Invalid argument]");
         break;
      case ENFILE : 
         printf("[Too many open files in system]");
         break;
      case EMFILE : 
         printf("[Too many open files]");
         break;
      case ENOTTY : 
         printf("[Inappropriate I/O control operation]");
         break;
      case ETXTBSY : 
         printf("[Text file busy]");
         break;
      case EFBIG : 
         printf("[File too large]");
         break;
      case ENOSPC : 
         printf("[No space left on device]");
         break;
      case ESPIPE : 
         printf("[Invalid seek]");
         break;
      case EROFS : 
         printf("[Read only file system]");
         break;
      case EMLINK : 
         printf("[Too many links]");
         break;
      case EPIPE : 
         printf("[Broken pipe]");
         break;
      case EDOM : 
         printf("[Domain error within math function]");
         break;
      case ERANGE : 
         printf("[Result too large]");
         break;
      case ENOMSG : 
         printf("[No message of desired type]");
         break;
      case EIDRM : 
         printf("[Identif ier removed]");
         break;
      case ECHRNG : 
         printf("[Channel number out of range]");
         break;
      case EL2NSYNC : 
         printf("[Level 2 not synchronized]");
         break;
      case EL3HLT : 
         printf("[Level 3 halted]");
            break;
      case EL3RST : 
         printf("[Level 3 reset]");
         break;
      case ELNRNG : 
         printf("[Link number out of range]");
         break;
      case EUNATCH : 
         printf("[Protocol driver not attached]");
         break;
      case ENOCSI : 
         printf("[No CSI structure available]");
         break;
      case EL2HLT : 
         printf("[Level 2 halted]");
         break;
      case EDEADLK : 
         printf("[Resource deadlock avoided]");
         break;
      case ENOTREADY : 
         printf("[Device not ready]");
         break;
      case EWRPROTECT : 
         printf("[Write-protected media]");
         break;
      case EFORMAT : 
         printf("[Unformatted media]");
         break;
      case ENOLCK : 
         printf("[No locks available]");
         break;
      case ENOCONNECT : 
         printf("[no connection]");
         break;
      case ESTALE : 
         printf("[no filesystem]");
         break;
      case EDIST : 
         printf("[old, currently unused AIX errno]");
         break; 
#ifndef _ALL_SOURCE
      case EWOULDBLOCK : 
         printf("[Operation would block]");
         break;
#endif /* _ALL_SOURCE */
      case EINPROGRESS : 
         printf("[Operation now in progress]");
         break;
      case EALREADY : 
         printf("[Operation already in progress]");
         break;
      case ENOTSOCK : 
         printf("[Socket operation on non-socket]");
         break;
      case EDESTADDRREQ : 
         printf("[Destination address required]");
         break;
      case EMSGSIZE : 
         printf("[Message too long]");
         break;
      case EPROTOTYPE : 
         printf("[Protocol wrong type for socket]");
         break;
      case ENOPROTOOPT : 
         printf("[Protocol not available]");
         break;
      case EPROTONOSUPPORT : 
         printf("[Protocol not supported]");
         break;
      case ESOCKTNOSUPPORT : 
         printf("[Socket type not supported]");
         break;
      case EOPNOTSUPP : 
         printf("[Operation not supported on socket]");
         break;
      case EPFNOSUPPORT : 
         printf("[Protocol family not supported]");
         break;
      case EAFNOSUPPORT : 
         printf("[Address family not supported by protocol family]");
         break;
      case EADDRINUSE : 
         printf("[Address already in use]");
         break;
      case EADDRNOTAVAIL : 
         printf("[Can't assign requested address]");
         break;
      case ENETDOWN : 
         printf("[Network is down]");
         break;
      case ENETUNREACH : 
         printf("[Network is unreachable]");
         break;
      case ENETRESET : 
         printf("[Network dropped connection on reset]");
         break;
      case ECONNABORTED : 
         printf("[Software caused connection abort]");
         break;
      case ECONNRESET : 
         printf("[Connection reset by peer]");
         break;
      case ENOBUFS : 
         printf("[No buffer space available]");
         break;
      case EISCONN : 
         printf("[Socket is already connected]");
         break;
      case ENOTCONN : 
         printf("[Socket is not connected]");
         break;
      case ESHUTDOWN : 
         printf("[Can't send after socket shutdown]");
         break;
      case ETIMEDOUT : 
         printf("[Connection timed out]");
         break;
      case ECONNREFUSED : 
         printf("[Connection refused]");
         break;
      case EHOSTDOWN : 
         printf("[Host is down]");
         break;
      case EHOSTUNREACH : 
         printf("[No route to host]");
         break;
      case ERESTART : 
         printf("[restart the system call]");
         break;
      case EPROCLIM : 
         printf("[Too many processes]");
         break;
      case EUSERS : 
         printf("[Too many users]");
         break;
      case ELOOP : 
         printf("[Too many levels of symbolic links]");
         break;
      case ENAMETOOLONG : 
         printf("[File name too long]");
         break;
#ifndef _ALL_SOURCE
      case ENOTEMPTY :
         printf("[Directory not empty]");
         break;
#endif /* _ALL_SOURCE */
      case EDQUOT : 
         printf("[Disc quota exceeded]");
         break;
      case EREMOTE : 
         printf("[Item is not local to host]");
         break;
      case ENOSYS : 
         printf("[Function not implemented  POSIX]");
         break;
      case EMEDIA : 
         printf("[media surface error]");
         break;
      case ESOFT : 
         printf("[I/O completed, but needs relocation]");
         break;
      case ENOATTR : 
         printf("[no attribute found]");
         break;
      case ESAD : 
         printf("[security authentication denied]");
         break;
      case ENOTRUST : 
         printf("[not a trusted program]");
         break; 
      case ETOOMANYREFS : 
         printf("[Too many references:can't splice]");
         break;
      case EILSEQ : 
         printf("[Invalid wide character]");
         break;
      case ECANCELED : 
         printf("[asynchronous i/o cancelled]");
         break;
      case ENOSR : 
         printf("[temp out of streams resources]");
         break;
      case ETIME : 
         printf("[I_STR ioctl timed out]");
         break;
      case EBADMSG : 
         printf("[wrong message type at stream head]");
         break;
      case EPROTO : 
         printf("[STREAMS protocol error]");
         break;
      case ENODATA : 
         printf("[no message ready at stream head]");
         break;
      case ENOSTR : 
         printf("[fd is not a stream]");
         break;

      /* WARNING ! This is a local error, and not an errno.h error */
      case FLOW_CONTROL :
          printf("[[Flow control]");
          break;
      default :
         printf("[0x%x]",err_number);
   }
   printf("\n");

}
/*           --- End of trace_error() function ---           */

#endif /* PTYDD_DEBUG */
