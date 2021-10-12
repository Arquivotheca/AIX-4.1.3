/*
#ifndef lint
static char sccsid[] = "@(#)77 1.14 src/bos/kernext/pty/spty.h, sysxpty, bos41J, 9515B_all 4/5/95 04:14:28";
#endif
*/

/*
 * COMPONENT_NAME: SYSXPTY - sptydd streams module
 *
 * FUNCTIONS:
 *
 * ORIGINS: 71, 83
 *
 */
/*
 * OSF/1 1.2
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_SPTY
#define _H_SPTY

/************************************************************************/
/*                                                                      */
/*      INCLUDE FILE                                                    */
/*                                                                      */
/************************************************************************/

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/strconf.h>
#include <sys/str_tty.h>
#include <termios.h>
#include<sys/lock_def.h>
#include <sys/lockl.h>
#include <sys/i_machine.h>
/************************************************************************/
/*                                                                      */
/*      DEFINE                                                          */
/*                                                                      */
/************************************************************************/

#define COMPONENT_NAME          "ptydd"

/* Mode */
#define ATT                     0x1234          /* for ATT naming mode  */
#define BSD                     0x5678          /* for BSD naming mode  */
#define PTY_KDB                 0x9876          /* dummy value for kdb  */

/* Call from for pty_open_comm and pty_close_comm */
#define SLAVE_CALL              0
#define MASTER_CALL             1

/* Automaton state when looking for Ctrl_X Ctrl_R */
#define SAK_NOTHING             -1
#define SAK_CTRL_X              1
#define SAK_CTRL_R              2

#define NRPTY(mode)             (mode == ATT ? (spty_dds.max_pts) : \
                                (spty_dds.max_ttyp))
#ifndef BPRI_WAITOK
#define BPRI_WAITOK (255)
#endif

#define FLOW_CONTROL            255

#define USED			0x0000001
#define GO_TO_SLEEP             0x0000002	


/* Specific define for fixing defect 171660, must be  included in stream.h ASAP */

#ifndef MSDRAIN
#define MSDRAIN         0x0800  
#endif

/* State bits */

#define PF_MOPEN                0x00000001      /* master is open       */
#define PF_SOPEN                0x00000002      /* slave is open        */
#define PF_XCLUDE               0x00000004      /* exclusive owner      */
#define PF_TTSTOP               0x00000008      /* output stopped       */
#define PF_TTINSTOP             0x00000010      /* input stopped        */
#define PF_MFLOW                0x00000020      /* master resource flow
                                                   controlled           */
#define PF_SFLOW                0x00000040      /* slave resource flow
                                                   controlled           */
#define PF_SWOPEN               0x00000080      /* Slave waiting for
                                                   master open          */
#define PF_REMOTE               0x00000100      /* REMOTE or noncanonical
                                                   mode                 */
#define PF_SWDRAIN              0x00000200      /* Slave closing; waiting
                                                   for drain            */
#define PF_PKT                  0x00000400      /* in PACKET Mode       */
#define PF_NOSTOP               0x00000800      /* Ctrl_s/Ctrl_q are not
                                                   stop char            */
#define PF_UCNTL                0x00001000      /* USER CONTROL Mode    */
#define PF_SAK                  0x00002000      /* Security mode        */
#define PF_CREAD                0x00004000      /* Enable receiver	*/
#define PF_IXON                 0x00008000	/* start/stop output	*/
#define PF_IXOFF                0x00010000	/* start/stop input	*/
#define PF_IXANY                0x00020000	
#define PF_MWDRAIN              0x00040000      /* Master closing; waiting
                                                   for drain            */
#define PF_PTM_XFER		0x00080000      /* transfert to master  */
#define PF_PTS_XFER		0x00100000	/* transfert to slave   */
#define PF_MWXFER		0x00200000      /* Master waiting for 
   						  transfert	        */
#define PF_SWXFER		0x00400000	/* Slave waiting for
						   transfert            */	
#define PF_M_RSE		0x00800000      /* slave wait for data 
						   drained on master side */

#define PF_RSE_OK		0x01000000	/* data drained on master
						   side */


/* High water and low water marks for flow control in the 2 "sub-drivers". */
/* Don't change these values for any reason ! */

#define SLAVE_HIWATER           512
#define SLAVE_LOWATER           128

#define SYSV_MAST_HIWATER       512
#define SYSV_MAST_LOWATER       128


#define flush_switch_flags(flagp)                                       \
        do {                                                            \
                int tmpflag=*(flagp);                                   \
                                                                        \
                *(flagp) &= ~(*(flagp));                                \
                if(tmpflag & FLUSHR)                                    \
                        *(flagp) |= FLUSHW;                             \
                if(tmpflag & FLUSHW)                                    \
                        *(flagp) |= FLUSHR;                             \
        } while(0)


/************************************************************************/
/*                                                                      */
/*      STRUCTURE                                                       */
/*                                                                      */
/************************************************************************/


/* CONFIGURATION */

struct spty_dds {
        dev_t   ptc_dev;        /* The major of the master ATT          */
        dev_t   pts_dev;        /* The major of the slave ATT           */
        int     max_pts;        /* The maximum number of pts ATT        */
        dev_t   ptyp_dev;       /* The major of the master BSD          */
        dev_t   ttyp_dev;       /* The major of the slave BSD           */
        int     max_ttyp;       /* The maximum number of tys BSD        */
};                               

/* INTERNAL STRUCTURE OF EACH OPEN DEVICE */

struct pty_s {
        /* Master and Slave queue pointer */
        queue_t *pt_mrq;                 /* master read queue           */
        queue_t *pt_mwq;                 /* master write queue          */
        queue_t *pt_swq;                 /* slave write queue           */
        queue_t *pt_srq;                 /* slave read queue            */

        /* state flag */
        uint              pt_flags;       /* pty global state flags      */
        struct termios    pt_tio;         /* termios structure           */
        struct winsize    pt_ws;          /* window size struct.         */
        struct tty_to_reg *ttydbg_decl;   /* ttydbg structure		 */
        u_char            pt_send;        /* Ctrl char for PCKT MODE     */
        u_char            pt_ucntl;       /* Ctrl char for USER MODE     */
        int               pt_compatflags; /* for BSD compatibility       */
        uint              pt_minor;       /* slave minor                 */
        dev_t             pt_dev_slave;   /* slave major/minor           */
        uint              pt_mode;        /* ATT or BSD naming           */
	char		  pt_name[TTNAMEMAX]; /* name of current slave   */
        char              pt_vstart;      /* vstart character at init    */
        char              pt_vstop;       /* vstop character at init     */
        int               pt_open_count;  /* Open counter                */

        /* synchronous event */
        int             pt_wait_master;
        int             pt_swait_drain;
        int             pt_mwait_drain;
        int             pt_mwait_xfer;
        int             pt_swait_xfer;

        /* lock */

        Simple_lock	lock_ptp;
};      

struct pty_o_s {
        struct pty_o_s  *pt_o_next;       /* next channel open            */
        uint             pt_o_minor;      /* slave minor                  */
        uint             pt_o_flag;       /* master/slave open flag       */
        struct pty_s    *pt_o_ptp;        /* ptp master and slave         */
};

#endif /* _H_SPTY */

