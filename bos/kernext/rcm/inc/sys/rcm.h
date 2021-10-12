/* @(#)85     1.31.2.14  src/bos/kernext/rcm/inc/sys/rcm.h, rcm, bos41J, 9513A_all 3/16/95 16:47:44 */

/*
 * COMPONENT_NAME: (rcm) AIX Rendering Context Manager structure definitions
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989-1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
	
#ifndef _H_RCX
#define _H_RCX

#include <sys/systemcfg.h>  /* system configuration */
#include <sys/except.h>     /* exception structures */
#include <sys/proc.h>	    /* state change structure */
#ifdef  _RCM
#include <sys/dispauth.h>   /* display bus access control */
#endif
#include <sys/aixgsc.h>     /* gscdma structure */
#include <sys/rcm_wg.h>     /* RCM window geom structures */

/*
#include "rcmhsc.h"
*/

#define MAX_DOMAINS	4

/* -----------------------------------------------------------------
			I/O STUFF
   -------------------------------------------------------------- */
typedef ulong	  authMask;

/* -----------------------------------------------------------------
			POINTER TYPE DEFINITIONS
   -------------------------------------------------------------- */

typedef struct _gscCom	    *gscComPtr;
typedef struct _gscComProc  *gscComProcPtr;
typedef struct _gscDev	    *gscDevPtr;
typedef struct _devDomain   *devDomainPtr;
typedef struct _rcmProc     *rcmProcPtr;
typedef struct _rcx	    *rcxPtr;
typedef struct _rcmWG	    *rcmWGPtr;
typedef struct _rcmWA	    *rcmWAPtr;
typedef struct _rcxp	    *rcxpPtr;
typedef struct _rcxph	    *rcxphPtr;
typedef struct _rcmCm	    *rcmCmPtr;
typedef struct _rcmTrace    *rcmTracePtr;

/* -----------------------------------------------------------------
		EVENTS GRAPHICS PROCESSES CAN WAIT ON
   -------------------------------------------------------------- */

#define RCX_WAIT_EVENT	    0x80000000	  /* event to wait on and post for */
					  /* heavy context switches */

/* -----------------------------------------------------------------
		COLORMAP LIST ENTRY DECLARATION
   -------------------------------------------------------------- */
typedef struct _rcmCm
{
    int hwd_map;      /* Actual hardware map number - assigned by
			 Server.				  */
    rcmCmPtr nxtCm;
    int    unused[2];
} rcmCm;


/* ----------------------------------------------------------------
		Event Info data
   ---------------------------------------------------------------*/

typedef struct _eventInfo {
  pid_t 	   gPid;	  /* Process id for this call */
  eventMasks	   masks;	  /* Event type masks */
  eventReport	   lastEvent;	  /* Last sync event that occurred */
  eventArrayPtr    pArray;	  /* event array      */
  genericPtr	   pData;	  /* pointer to event buffer */
  genericPtr	   buffer_ptr;	     /* pointer to event buffer */
  int		   bufleft,numhits;  /* event buffer length  */
  int		   length;	  /* event buffer length  */
  int		   wait;	  /* Wait flag forsync event processing */
  int              unused[2];
} eventInfo;






/* -----------------------------------------------------------------
			GSC COMMON STRUCTURE ANCHOR

    This anchor structure is static. It provides an anchor point
    for the dynamically allocated GSC common structure, a static
    lock word used to control the basic open/close/ioctl/resource
    allocation functions.  There is a structure to define the
    state change handler to the system.  The state change handler
    is always active whenever the RCM is configured.
   -------------------------------------------------------------- */

typedef struct _comAnchor {
    int 	rlock;	    /* a lock for serializing access to the
			       anchor and RCM common structure */
    gscComPtr	pCom;	    /* pointer to the RCM common structure */

    int         unused[2];
    struct proch  state_chng;   /* to register state chg handler */
} comAnchor;

#define apCom	comAnc.pCom	/* shorthand for RCM Common pointer */
extern comAnchor comAnc;


/* -----------------------------------------------------------------
			GSC COMMON STRUCTURE

    This structure serves as the anchor point for all other RCM
    structures. It also contains the structures with which to
    register RCM "hooks" to kernel services (fault handler,
    state change handler). It also contains the HSC information.
   -------------------------------------------------------------- */

typedef struct _gscCom {
    struct uexcepth except;	      /* structure for exception handler;
					 catches graphics faults */
    gscDevPtr	    pDevList;	      /* head of linked list of devices */
    gscComProcPtr   pProcList;	      /* head of linked list of graphics
					 process common structures */
    struct _rcxp    *pRcxParts;       /* linked list of context parts */
    int             unused[9];
    struct callbacks {
	int (*give_up_timeslice)(devDomainPtr, rcmProcPtr);
	int (*guard_domain)(devDomainPtr, rcmProcPtr);
	int (*unguard_domain)(devDomainPtr);
	int (*put_on_fault_list)(rcxPtr);
	int (*block_graphics_process)(devDomainPtr, rcmProcPtr);
	int (*unblock_graphics_process)(devDomainPtr, rcmProcPtr);
	int (*make_cur_and_guard_dom)(rcxPtr);
	int  *unused[4];
    } *rcm_callback;

    void * lftCommonPtr;	        /* ptr to lft common structure.   */ 
			/* Needed for font support (unmake_shm in rcmgp.c) */
} gscCom;


/* -----------------------------------------------------------------
			GSC COMMON PROCESS STRUCTURE

    A common process structure exists for every process that becomes
    a graphics process (gp). The gp also has a device process
    structure for every device for which it becomes a gp.

   -------------------------------------------------------------- */

#define RCM_MAX_DEV	4	/* maximum number of devices for RCM */

#define MAX_BUSMEM_KEYS  7	/* maximum no. of bus memory access keys */
                                /* There are 7 keys, 1 to 7 inclusively  */

#define FIRST_BUSMEM_KEY_VAL  	1	/* keys must be 1 or greater */

/*
 *  Bus memory access key allocation control database entry.
 */
struct busmem_key
	{
	    short      key;		/* available value (must be between 
                                          1 and 7 inclusively            */
	    short      spare;		/* put to use if we ever need it */
	    gscDevPtr  owner;		/* owning dev structure */
	    int        unused[2];
	};

#define RCM_MAX_USR_BUFFER	8		/* max number of list headers */

typedef struct _rcm_uaddr_t			/* structure for 60-bit addrs */
	{
	    vmhandle_t  	srval;		/* seg reg content */
	    caddr_t		eaddr;		/* effective addr */
	    int			unused[2];
	} rcm_uaddr_t ;

typedef struct usrbuf			/* for recording shared mem attach */
	{
	    struct usrbuf	*next;
            vmhandle_t          srval;          /* seg reg content */
            caddr_t             start;          /* effective addr */
	    long		length;
	    int			unused[2];
	    struct xmem 	xmemusr;	/* for cross mem attach */
	} usrbuf_t ;

struct segreg
	{
	    struct  segreg  *next;
	    ulong   srval;
	    int     usage;
	    caddr_t segment;
	    int     unused[2];
	};

typedef struct _gscComProc {
    gscComProcPtr   pNext;	/* pointer to next proc */
    pid_t	    pid;	/* process id */
    tid_t	    tid;	/* thread id */
    int 	    count;	/* number of device specific process
				   structures for this common */
    ulong	    flags;	/* flag word */
    struct busmem_key  *pBusKey; /* pointer to gp leader bus mem protect key
				   database */
    struct segreg   *pSegreg;	/* list of seg regs assigned to process */
    struct usrbuf **pusrbufhdr;	/* points to usr buffer headers */
    int		    unused[10];
#if 0
    devProcPtr	    pProcList;	/* head of list of device specific processes */
    struct {
	gscDevPtr   pDev;	/* pointer to device */
	devProcPtr  pProc;	/* pointer to device specific process */
    } devProc[RCM_MAX_DEV];	/* array of device process pointers, one
				   for each possible device upon which this
				   process can be a graphics process */
#endif
} gscComProc;

#define COMPROC_SIGNALLED	(1L << 0)	/* SIGTERM sent already */
#define COMPROC_FUNNELLED	(1L << 1)	/* was previously funnelled */




/* -----------------------------------------------------------------
			RCM ADAPTER DOMAIN STRUCTURE
   -------------------------------------------------------------- */

typedef struct _devDomain {
    int 	    rlock;	      /* lock field for locking the domain */
				      /* structure, not the domain itself; */
				      /* intended for use with */
				      /* lockl/unlockl to serialize access */
				      /* to domain structure */
    int 	    guardlist;	      /* field for gps waiting to guard
					 the domain */
    gscDevPtr	    pDev;	      /* pointer to device structure */
    int		    domain;	      /* number of the domain */
    ulong	    auth;	      /* authorization mask used to allow */
				      /* process access to bus memory for */
				      /* domain */
#ifdef _RCM
    ioRange	    range;	      /* bus I/O range for domain */
#else
/**//* sizeof (ioRange) MUST == 4 or we are broken! */
    ulong	    dummy;	      /* nonRCM doesn't need this */
#endif
    time_t	    timestamp;	      /* time current RCX put on domain */
				      /* relative to last boot */
    rcxPtr	    pCur;	      /* current rendering context  */
    rcmProcPtr	    pCurProc;	      /* gp owning current rcx	*/
    rcmProcPtr	    pOldCurProc;      /* gp owning previous rcx	*/
    rcxPtr	    pFault;	      /* linked link of rendering contexts */
				      /* faulting on this domain */
    rcmProcPtr	    pLockProc;	      /* pointer to locking graphics */
				      /* process */
    struct trb	    *pDevTimer;       /* timer structure for timer */
				      /* services used for time slice */
				      /* expiration */
    ulong	    flags;
    int		    unused[6];
} devDomain;




/* flags values */
#define DOMAIN_SWITCHING    0x1       /* actually switching, due to device
					 independent requests */
#define DOMAIN_GUARDED	    0x2       /* guarded from switching, but not really
					 switching, due to device dependent
					 requests */
#define DOMAIN_LOCKED	    0x4       /* due to gp request */
#define DOMAIN_TIMER_ON     0x8       /* graphics time slice not expired
					 (timer going) */
#define DOMAIN_DEV_LOCKED   0x10      /* domain device locked, which must
					 be different from domain locked so
					 that gp does not get signalled, and
					 must be different from guarded so
					 can guard when locked */
#define DOMAIN_BLOCKED	    (DOMAIN_LOCKED | DOMAIN_DEV_LOCKED | DOMAIN_GUARDED)
#define DOMAIN_SUSPENDED    0x20      /* domain contains a context, but the
					 graphics process that owns the context
					 does not have authority to access
					 the domain - this situation can result
					 in a fault by the gp that has
					 a rcx on the domain ! */
#define DOMAIN_GUARD_WAKED  0x40	/* e_wakeup issued on guardlist */
#define DOMAIN_GUARD_PENDING  0x80	/* e_wakeup issued on guardlist */




/* -----------------------------------------------------------------
			GSC DEVICE STRUCTURE
   -------------------------------------------------------------- */

typedef struct _gscDev {
    struct _gscDevHead {
	int	    locknest;	      /* nesting count for struct locks */
	gscDevPtr   pNext;	      /* link to next device */
	rcmProcPtr  pLockProc;	      /* device gp locking process */
	ulong	    flags;	      /* flags */
	gscComPtr   pCom;	      /* pointer back to gsc common */
	int	    locklist;	      /* locklist field for gps waiting to lock
					 the device */
	int	    dma_sleep;	      /* list of processes sleeping */
        rcm_wg_hash_table_t   *wg_hash_table;   /* win geom hash table */
	int	    sync_sleep;	      /* for leaders syncing with clients */
        ulong       window_count ;      /* Running count of windows defined */
	rcmProcPtr  pProc;	      /* linked list of graphics process
					 structures */
	rcmCmPtr    pCm;	      /* pointer to color map linked list */

	struct phys_displays	*display;
				      /* display information for this vt */
	int	    unused0;	      /* used to be pvt -- now unused */
	char	    *vttld;	      /* pointer to virtual term private data */
	int	    num_domains;      /* number of domains */
	int	    count;	      /* number of dev_init calls active */

	char	    rcmtrace_ID[8];   /* trace ID characters */ 
	rcmTracePtr rcmtrace;	      /* pointer to trace control block */
	struct usrbuf **pusrbufhdr;	/* point to leader shared mem records */
	pid_t	    leader_pid;	      /* pid of graphics process leader */
	tid_t	    leader_tid;	      /* tid of graphics process leader */
	int	    fault_type;		/* set by make-gp for rcmswitch */
	int	    unused[4];
    } devHead;


    devDomain	    domain[1];	      /* array of domain structures for
					 adapter */
} gscDev;

/* device flags */
#define DEV_GP_LOCKED		0x1	/* indicates device gp locked */
#define DEV_GP_WAITING		0x2	/* indicates a gp is waiting to get */
					/* the gp locked */
#define DEV_DMA_IN_PROGRESS	0x4	/* indicates device doing DMA */
#define DEV_RESERVED		0x8	/* device reserved for exclusive
					   access by single process */
#define DEV_GP_LOCKED_AND_GUARDED 0x10	/* DEV_GP_LOCKED + domains guarded */
#define DEV_SIGNAL_SENT		0x20	/* unmake of leader has sent sig */
#define DEV_HAS_LEADER		0x40	/* set when leader is defined */
#define DEV_DMA_LOCK_ADAPTER	0x80	/* PED/LEGA dd sets this to force */
					/*    dma_service to lock adapter */



/* -----------------------------------------------------------------
			RCM PROCESS STRUCTURE
   -------------------------------------------------------------- */

#ifdef _RCM
struct rcmbusprot
	{
		struct rcmbusprot  *next;
		int		   domain;
		int	    	   unused[2];
		busprt_t	   busProt;
	};
#endif

typedef struct _rcmProc {
    struct _rcmProcHead {
	rcmProcPtr	pNext;		/* pointer to next process block */
	gscComProcPtr	pCproc; 	/* pointer to common process */
	pid_t		pid;		/* process id */
	tid_t		tid;		/* thread id */
	int		priority;	/* graphics process priority */
	rcxPtr		pRcx;		/* linked list of rendering contexts */
	rcmWAPtr	pWA;		/* linked list of window attribute */
					/* structures */
	eventInfo	*pEvents;	/* pointer to event information   */

	gscDevPtr	pGSC ;		/* aixgsc device structure */
	struct _gscdma	gscdma; 	/* DMA control structure */
#define DMA_SW_WRITE_PROTECT	0x80	/* coordinate with aixgsc.h */
	struct _gscdma_ext {		/* for addressing segs to unpin */
			vmhandle_t  sw_seg;	/* seg reg content */
			vmhandle_t  sw_Useg;	/* scratch for seg comput */
		} gscdma_ext[MAX_SUBAREAS];
	struct xmem xmemdma[MAX_SUBAREAS];
					/* array of cross-memory structures */
	genericPtr	pPriv;		/* pointer to device private data */
	ulong		flags;		/* process flags */
	ulong		srval;		/* segment register value */
	struct segreg   *pSegreg;	/* pointer to specific segreg pkt */
#ifdef _RCM
	struct rcmbusprot *pRcmbusprot;	/* list of bus protect pkts per dom */
#else
/**//* this depends on all pointers being the same size or we are broken! */
	void            *dummy;		/* nonrcm doesn't need this */
#endif
	struct _partList
	{
	    struct _partList *pNext;	/* next entry or NULL */
	    rcxpPtr           pRcxp;	/* part pointer used */
	    int		      unused[2];
	} *pParts;
	int		unused[10];
    } procHead;

    /*-----------------------------------------------------------------------
	The following set of pDomainCur pointers specify the context that
	the user has declared as "current" (meaning "to be used") for each
	physical domain on the adapter.  These pDomainCur pointers are set
	with the gsc_set_rcx aixgsc system call.  When an adapter domain is
	loaded with a context for a process, the rcm will use the context
	that the process has marked as "current" for that physical domain.

	These pointers DO NOT imply that the contexts are currently on the
	physical domains of the adapter.
     *-----------------------------------------------------------------------*/
    rcxPtr		pDomainCur[1];	
				
} rcmProc;


/* process flags */
#define PROC_DMA_IN_PROGRESS	(1L << 0)	/* DMA operation in progress */

/* -----------------------------------------------------------------
	The following two flags control the process blocking function in
	rcmswitch.c. 
   -------------------------------------------------------------- */

#define PROC_DD_BLOCK_REQUEST	(1L << 1)	/* DD requests process block */
#define PROC_PROCESS_BLOCKED	(1L << 2)	/* Process actually blocked */

#define PROC_GP_LEADER		(1L << 3)	/* lead gp on device */

/* ------------------------------------------------------------------
	This flag is set while a graphics process is in unmake_gp.
	FIND_GP checks this flag and functions as if this process
	is no longer a graphics process.  This inhibits the execution
	of aixgsc functions which use FIND_GP.
   ------------------------------------------------------------------ */

#define PROC_UNMAKE_GP		(1L << 4)	/* Process in unmake - FIND_GP
						   checks this */

/* -----------------------------------------------------------------
			RCM RENDERING CONTEXT STRUCTURE
   -------------------------------------------------------------- */

typedef struct _rcx {
    rcxPtr	    pNext;	    /* pointer to next rcx for process */
    rcxPtr	    pNextFlt;	    /* pointer to next rcx in fault */
				    /* list for adapter/domain */
    rcmProcPtr	    pProc;	    /* process pointer */
    int 	    priority;	    /* process priority, according to */
				    /* graphics dispatcher */
    genericPtr	    pData;	    /* pointer to device dependent rcx */
				    /* structure	     */
    int 	    domain;	    /* domain number */
    devDomainPtr    pDomain;	    /* pointer to domain array entry */
    rcmWGPtr	    pWG;	    /* ptr to associated window geometry */
    rcmWAPtr	    pWA;	    /* ptr to associated window */
				    /* attributes */
    rcxPtr	    pLinkWG;	    /* pointer to next rcx in list */
				    /* using WG */
    rcxPtr	    pLinkWA;	    /* pointer to next rcx in list */
				    /* using WA */
    rcxphPtr	    pRcxph;	    /* ptr to associated rcx parts list */
    ulong	    flags;	    /* flags	    */
    int		    timeslice;	    /* time slice (ns), SHARED W/ DD CODE! */
    caddr_t	    pDomainLock;    /* ptr to user structure for rcm use */
    struct xmem	    XmemDomainLock; /* xmem desc for user shared struct */
    struct DomainLock DomLockCopy;  /* area to copy user data to */
    int             unused[4];
} rcx;

/* rcx flags */

#define RCX_UNALLOCATED		0x0
#define RCX_ALLOCATED		0x1
#define RCX_READY		0x2
#define RCX_CURRENT		0x4
#define RCX_DELETING		0x8
#define RCX_SWITCH_IN		0x10
#define RCX_SWITCH_OUT		0x20
#define RCX_BLOCKED		0x80	/* process blocked, wake with unblock */
#define RCX_TIME_SLICE		0x100	/* context has/had timeslice */
#define RCX_WANTS_LOCK		0x200	/* process wants to gp lock domain */
#define RCX_NULL		0x400	/* this is a null context... no need */
					/* to save or restore it during switch*/
#define RCX_ON_FAULT_LIST	0x800	/* Context already on fault list */

/*
 *  The 'timeslice' member of the structure above is primed by the rcm
 *  but may be altered by the DD level.  DD level constants are defined
 *  here for now.
 */
#define RCM_DEFAULT_TIMESLICE		60*1000000  /* 60ms in nanoseconds */
#define PED_FIFO_DEFAULT_TIMESLICE	30*1000000  /* 30ms in nanoseconds */
#define PED_PCB_DEFAULT_TIMESLICE	30*1000000  /* 30ms in nanoseconds */




/* -----------------------------------------------------------------
			RCM WINDOW GEOMETRY STRUCTURE
   -------------------------------------------------------------- */

typedef struct _rcmWG {
    rcmWGPtr		    pNext;	  /* linked list of win */
					  /* geometry */
    rcxPtr		    pHead;	  /* head of linked list of */
					  /* rcx referencing this wg */
    rcmProcPtr		    pProc;	  /* owning process */
    genericPtr		    pPriv;	  /* pointer to device */
					  /* dependent private data */
    ulong		    flags;
    rcmWAPtr		    pLastWA;	  /* last bound window attr */
    int			    unused[2];
    gWinGeomAttributes	    wg; 	  /* kernel's copy of */
					  /* creator's window geometry */
} rcmWG;

/* window flags */
#define WG_CHANGED 0x00000001
#define WG_DELETED 0x00000002


/* -----------------------------------------------------------------
			RCM WINDOW ATTRIBUTE STRUCTURE
   -------------------------------------------------------------- */

typedef struct _rcmWA {
    rcmWAPtr		    pNext;	  /* linked list of win */
    /* attributes */
    rcxPtr		    pHead;	  /* head of linked list of */
					  /* rcx referencing this wa */
    genericPtr		    pPriv;	  /* pointer to device */
					  /* dependent private data */
    ulong		    flags;
    int			    unused[2];
    gWindowAttributes	    wa; 	  /* kernel's copy of */
					  /* creator's win attributes */
} rcmWA;

/* window attribute flags */
#define WA_CHANGED 0x00000001
#define WA_DELETED 0x00000002


/* -----------------------------------------------------------------
			RCM RENDERING CONTEXT PART STRUCTURE
   -------------------------------------------------------------- */

typedef int		global_ID;

typedef struct _rcxp {
    rcxpPtr	    pNext;	      /* pointer to next rcxp */
    global_ID	    glob_id;	      /* global id */
    int 	    priority;	      /* part priority */
    int 	    users;	      /* use count */
    genericPtr	    pData;	      /* pointer to device dependent rcxp */
				      /* structure     */
    int 	    length;	      /* length of data */
    time_t	    timestamp;	      /* when "deleted" */
    struct xmem     xmemd;	      /* xmem descriptor       */
    ulong	    flags;	      /* flags	      */
    int		    unused[2];
} rcxp;

typedef struct _rcxp	*RCXP_handle;

typedef struct _rcxph {
	rcxphPtr	pNext;	     /* pointer to next rcxp header */
	rcxphPtr	pPrev;	     /* pointer to previous rcxp header */
	rcxpPtr 	pRcxp;	      /* pointer to rcx part */
	ulong		flags;	      /* flags	      */
	int		unused[2];
} rcxph;

/* rcxph flags */
#define RCXPH_READY	 0x1
#define RCXPH_CURRENT	 0x2
#define RCXPH_KERNEL	 0x4


/* -----------------------------------------------------------------
			RCM TRACE STRUCTURE
   -------------------------------------------------------------- */

typedef struct _rcmTrace {
	struct _ptrace_entry *performance_trace_table;
	int	ptt_ndx;		/* next entry to do in above table */
	int	ptt_size;		/* number of entries in table */
	char   *perf_trace_flags;	/* trace selection buffer */
	struct	xmem xmemdesc_flags;	/* cross-mem descriptor of flags */
	struct	xmem xmemdesc_table;	/* cross-mem descriptor of table */
	pid_t	pid;			/* process id of owning process */
	int	unused[2];
} rcmTrace;


#endif /* _H_RCX */
