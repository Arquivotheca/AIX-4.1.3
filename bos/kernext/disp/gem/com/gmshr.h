/* @(#)78	1.5.2.6  src/bos/kernext/disp/gem/com/gmshr.h, sysxdispgem, bos411, 9428A410j 1/22/93 09:19:26 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*--------------------------------------------------------------------*/
/* Typedefs for the shared memory.                                  @2*/
/*--------------------------------------------------------------------*/
  typedef struct _shmFifo {             /* Shared memory              */
      union {                           /*   FIFO ptrs                */
	char   *cp;                     /*   FIFO char ptr            */
	unsigned short *sp;             /*  FIFO unsigned short ptr   */
	unsigned long  *lp;             /*  FIFO unsigned long ptr    */
      } fifo[4];
      unsigned short  fi[4];            /* FIFO index next avail byte */
      unsigned long    user_lock;	/* lock word for GAI to use   */
      unsigned long    rcm_lock;	/* lock word for RCM to use   */
  } shmFifo, *shmFifoPtr;

typedef	struct _FifoLock {
      unsigned long    user_lock;	/* lock word for GAI to use   */
      unsigned long    rcm_lock;	/* lock word for RCM to use   */
    } FifoLock, *FifoLockPtr;

/************************************************************************/
/* Adapter Interrupt Event Codes                                        */
/************************************************************************/
#define THRESHOLD_EVENT     0x01        /* Threshold Event              */
#define SYNC_CTR_EVENT      0x02        /* Sync Counter Event           */
#define FIFO_SYNC_EVENT     0x04        /* FIFO Syncronization Event    */
#define PICK_EVENT          0x08        /* Pick Event                   */
#define ERROR_EVENT         0x10        /* Adapter Error Event          */

/************************************************************************/
/* Event codes used for synchronous and asynchronous events             */
/************************************************************************/
#define NULL_CMD            0x00000000
#define SYNC_WAIT_CMD       0x00000001
#define SYNC_NOWAIT_CMD     0x00000002
#define ASYNC_CMD           0x00000004
#define PICK_CMD            0x00000008
#define GSYNC_CMD           0x00000010
#define gto_WAIT            0x00000001

/*----------------------------------------------------------------------*/
/* Type flags								*/
/*----------------------------------------------------------------------*/
#define IMM_RCXTYPE	0x01
#define TRAV_RCXTYPE	0x02
#define PART_RCXTYPE    0x04
#define GL_ENHANCED	0x80

/*----------------------------------------------------------------------*/
/* DEV_DEP_FUN constants						*/
/*----------------------------------------------------------------------*/
#define	DEF_GROUPS	1
#define WAKEUP_SWITCH   2
#define SAVE_STATE	3
#define RSTR_STATE	4
#define LOAD_OLAY_CT    5
#define GEMDDF1		6
#define GEMDDF2		7
#define GEMDDF3		8


/*----------------------------------------------------------------------*/
/* Domain constants      						*/
/*----------------------------------------------------------------------*/
#define GM_MAX_DOMAINS 		3
#define IMMEDIATE_CREG_DOMAIN	0
#define TRAVERSAL_FIFO_DOMAIN   1
#define TRAVERSAL_GLOBAL_DOMAIN TRAVERSAL_FIFO_DOMAIN
#define IMMEDIATE_FIFO_DOMAIN	2

/*----------------------------------------------------------------------*/
/* DEV_DEP_FUN data structures						*/
/*----------------------------------------------------------------------*/

typedef struct _gOlay_Colors {
    int             firstindex; 	/* first entry in table that needs to
						be reloaded 		*/
    int             lastindex;		/* last entry to load		*/
    
    ushort 	    realcolormapID;    	/*  colormap ID  */ 
    ulong	    colors[16];
    } gOlay_Colors, *gOlay_ColorsPtr;

typedef	struct _rDefGroups {
     ushort    depth ;  
     ushort    type  ;
   } rDefGroups, *rDefGroupsPtr;

typedef struct _gDef_Groups  {						/*@4*/
  char          *wa;		/* Window Attribute handle of window */
  				/* for which the groups are being    */
				/* defined.			     */
  int		active_group;   /* Index into groups_used of current group */
  int		num_groups;	/* Number of group attributes 	     */
  rDefGroupsPtr groups_used;   /* Array of group types              */ /*@7*/
} gDef_Groups, *gDef_GroupPtr;

typedef struct _gCopyShmem {
  shmFifoPtr	shmem;
  FifoLockPtr	pFifoLock;						/*@2*/
} gCopyShmem, *gCopyShmemPtr;

typedef struct _gWakeSwitch {
  ulong		lock;			/* Unused */
} gWakeSwitch, *gWakeSwitchPtr;


typedef struct _ggemddf {
  ulong		unused1;		/* Unused */
  ulong		unused2;		/* Unused */
  ulong		unused3;		/* Unused */
  ulong		unused4;		/* Unused */
} ggemddf, *ggemddfPtr;

/*----------------------------------------------------------------------*/
/* Device Dependant data passed to standard RCM functions		*/
/*----------------------------------------------------------------------*/

/*
 * device dependent structure returned by make_gp
 */
#define PLANES24      	0x0001	/* Constant for rMakegpdd.features */
#define MEMORY_4_MEG	0x0002	/* Constant for rMakegpdd.features */
#define NO_SHP      	0x0004	/* Constant for rMakegpdd.features */
typedef struct _rMakegpdd
{
  unsigned long	position_ofst;		/* adapter base address       */
  					/* with out seg reg           */

  struct {
    unsigned long	addr;		/* starting address of domain */
    unsigned long	length;		/* number of bytes in domain  */
  } 			domain_addr[GM_MAX_DOMAINS];
  
  struct {                              /* GM gcard slots          */
    ulong   magic;                    /* slot 0,1                     */
    ulong   drp;                      /* slot 9                       */
    ulong   gcp;                      /* slot 6,2                     */
    ulong   shp;                      /* slot 7,3                     */
    ulong   imp;                      /* slot 0                       */
  } gcardslots;				/* GM backplane config  @3*/
  unsigned long         features;       /* bit 0: 0=>8 1=>24 planes    */
					/* bit 1: 0=>2 1=>4  Meg GM    */
  shmFifoPtr	shmem;						/*@2*/
  FifoLockPtr	pFifoLock;					/*@2*/
  ulong screen_width_mm;              /* screen width from phys disp   */
  ulong screen_height_mm;             /* screen height form phys disp  */

} rMakegpdd, *rMakegpddPtr;

/* @3
 * Input device dependent data structure for create rcx
 */
typedef int gem_cxt;	/* temporary typedef to compilation	      /**/

typedef struct _Gemrcx {	/* structure pointed to by pData in rcx */
    int cxt_type;		/* immediate,traversal, part mode       */
    int status_flags;		/* 1= cxt is in adapter memory		*/

    int start_slot;		/* slot which this rcx is bound to	*/
    int num_slots;		/* number of contigous slots needed     */

    int favored_proc;		/* used for optimizing slot binding	*/

    ulong GMChangeMask;         /* Change mask for differed Z buffer    */
				/* or Overlay buffer protect plane update */
    struct _Gemrcxp {
    	ulong	name;		/* Name by which this rcxp will be queried*/

	int  	create_count;	/* Number of times this rcxp has had	  */
				/* QUERY_RCXP called with its name spec'd */
	} Gemrcxp;

    gem_cxt *pCxt;		/* ptr in kernel memory to cxt		*/

    ulong	*pASL;		/* offset of adapter state list         */

    ulong	*pRetArg;	/* Pointer to user space for output arg */

    ulong  	gWAChangeMask;	/* Change mask for differed update WA   */
    ulong  	gWGChangeMask;	/* Change mask for differed update WG   */

    ulong	win_comp_mask;	/* Compare mask for window planes     @1*/
    ulong	win_comp_val;	/* Compare value for window planes    @1*/
  } rGemrcx, *rGemrcxPtr;

/*----------------------------------------------------------------------*/
/* Device Dependent DMA Command Data structure                          */
/*----------------------------------------------------------------------*/
typedef struct _gem_cmd {
     uint    dma_dest[4];      /* Global Memory offset in adapter       */
     uint   flags;
   } gem_cmd;

