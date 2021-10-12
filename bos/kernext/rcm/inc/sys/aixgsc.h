/* @(#)74	1.23  src/bos/kernext/rcm/inc/sys/aixgsc.h, rcm, bos41B, 9504A 12/13/94 16:18:39 */

/*
 * COMPONENT_NAME: (rcm) AIX Rendering Context Manager Sys Call Definitions
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989-1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
	

/* ---	NOTICE	----------------------------------------------------
	
	One should include sys/rcm_win.h to use this file successfully

 ------------------------------------------------------------------*/


#ifndef _H_GSC
#define _H_GSC

typedef char *genericPtr;	     /* generic pointer type */
typedef char *RCX_Handle;	     /* rcx handle (not to be used as a pointer  */
typedef char *RCXP_Handle;	     /* rcx handle (not to be used as a pointer  */
typedef char *WA_Handle;	     /* wa handle (not to be used as a pointer	*/
typedef char *WG_Handle;	     /* wg handle (not to be used as a pointer	*/
typedef char *CM_Handle;	     /* cm handle (not to be used as a pointer) */
typedef int  globalID;		     /* rcxp global identifier */

/*
 *	commands processed by the aixgsc() system call ********************
 */

#define GSCIO		('G'<<8)	/* graphics system call */
#define MAX_PRIORITY	    100
#define DEFAULT_PRIORITY    1		  
#define GSC_FAST_SYSCALL_ERROR_BASE	(1L << 10)

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, MAKE_GP, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	make_gp arg;
 */

#define MAKE_GP 	(GSCIO | 1)

typedef struct _make_gp {
	int	    error;		/* error report */
	caddr_t     segment;		/* segment base address */
	genericPtr  pData;		/* device specific adapter addresses */
	int	    length;		/* length of device specific data */
	int	    access;		/* access authority */
#	define SHARE_ACCESS	0	/* process shares access to adapter */
#	define EXCLUSIVE_ACCESS 1	/* process cannot share access */
} make_gp;

/*	-----------------------------------------------------------------
 *
 *	aixgsc (gsc_handle, UNMAKE_GP, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	unmake_gp arg;
 */

#define UNMAKE_GP  (GSCIO | 2)

typedef struct _unmake_gp {
	int	error;		/* error report */
} unmake_gp;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, SET_GP_PRIORITY, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	set_gp_priority arg;
 */

#define SET_GP_PRIORITY (GSCIO | 3)

typedef struct _set_gp_priority {
	int	error;		    /* error report */
	pid_t	pid;		    /* process id */
	int	priority;	    /* grpahics process priority */
} set_gp_priority;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, CREATE_RCX, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	create_rcx arg;
 */

#define CREATE_RCX	(GSCIO | 4)

typedef struct _create_rcx {
	int		error;		/* error report */
	int		domain; 	/* domain id */
	struct DomainLock {		/* shared structure with kernel to */
		int DomainLocked;	/* manage domain locking protocol */
		int TimeSliceExpired;
	} *pDomainLock;
	genericPtr	pData;		/* device dependent data */
	int		length; 	/* ddi length */
	RCX_Handle	rcx_handle;	/* returned id */
} create_rcx;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DELETE_RCX, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	delete_rcx arg;
 */

#define DELETE_RCX	(GSCIO | 5)

typedef struct _delete_rcx {
	int		error;		/* error report */
	RCX_Handle	rcx_handle;	/* from CREATE_RCX */
} delete_rcx;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, CREATE_WIN_GEOM, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	create_win_geom arg;
 */

#define CREATE_WIN_GEOM (GSCIO | 6)

typedef struct _create_win_geom {
	int		error;		/* error report */
	gWinGeomAttrPtr pWG;		/* pointer to geometry */
	WG_Handle	wg;		/* window geometry handle */
} create_win_geom;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DELETE_WIN_GEOM, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	delete_win_geom arg;
 */

#define DELETE_WIN_GEOM (GSCIO | 7)

typedef struct _delete_win_geom {
	int		error;		/* error report */
	WG_Handle	wg;		/* window geometry handle */
} delete_win_geom;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, UPDATE_WIN_GEOM, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	update_win_geom arg;
 */

#define UPDATE_WIN_GEOM (GSCIO | 8)

typedef struct _update_win_geom {
	int		error;		/* error report */
	WG_Handle	wg;		/* window geometry handle */
	gWinGeomAttrPtr pWG;		/* pointer to geometry */
	int		changes;	/* change mask */
					/* change mask described in gai.h */
} update_win_geom;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, CREATE_WIN_ATTR, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	create_win_attr arg;
 */

#define CREATE_WIN_ATTR (GSCIO | 9)

typedef struct _create_win_attr {
	int		error;		/* error report */
	gWindowAttrPtr	pWA;		/* pointer to attribute */
	WA_Handle	wa;		/* window attr handle */
} create_win_attr;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DELETE_WIN_ATTR, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	delete_win_attr arg;
 */

#define DELETE_WIN_ATTR (GSCIO | 10)

typedef struct _delete_win_attr {
	int		error;		/* error report */
	WA_Handle	wa;		/* window attr handle */
} delete_win_attr;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, UPDATE_WIN_ATTR, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	update_win_attr arg;
 */

#define UPDATE_WIN_ATTR (GSCIO | 11)

typedef struct _update_win_attr {
	int		error;		/* error report */
	WA_Handle	wa;		/* window attr handle */
	gWindowAttrPtr	pWA;		/* pointer to attribute */
	int		changes;	/* change mask */
					/* change mask described in gai.h */
} update_win_attr;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, BIND_WINDOW, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	bind_window arg;
 */

#define BIND_WINDOW	(GSCIO | 12)

typedef struct _bind_window {
	int		error;		/* error report */
	RCX_Handle	rcx;		/* rcx to bind to */
	WG_Handle	wg;		/* window geometry to use. */
	WA_Handle	wa;		/* window attributes to use. */
					/* Note that these must have been
					   created */
} bind_window;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, SET_RCX, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	set_rcx arg;
 */

#define SET_RCX 	(GSCIO | 13)

typedef struct _set_rcx {
	int		error;		/* error report */
	RCX_Handle	rcx;
	int		domain; 	/* domain of NULL rcx */
} set_rcx;


/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, LOCK_HW, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	lock_hw arg;
 */

#define LOCK_HW 	(GSCIO | 14)

typedef struct _lock_hw {
	int		error;		/* error report */
	int		wait;		/* wait for lock to clear */
	int		status; 	/* pid of blocking process */
} lock_hw;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, UNLOCK_HW, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct unlock_hw arg;
 */

#define UNLOCK_HW	(GSCIO | 15)

typedef struct _unlock_hw {
	int		error;		/* error report */
} unlock_hw;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, LOCK_DOMAIN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	lock_domain arg;
 */

#define LOCK_DOMAIN	(GSCIO | 16)

typedef struct _lock_domain {
	int		error;		/* error report */
	int		domain; 	/* domain id */
	int		wait;		/* wait for lock to clear */
	int		status; 	/* pid of blocking process */
} lock_domain;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, UNLOCK_DOMAIN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	unlock_domain arg;
 */

#define UNLOCK_DOMAIN	(GSCIO | 17)

typedef struct _unlock_domain {
	int		error;		/* error report */
	int		domain; 	/* domain id */
} unlock_domain;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, GIVE_UP_TIMESLICE, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	give_up_timeslice arg;
 */

#define GIVE_UP_TIMESLICE	(GSCIO | 22)

typedef struct _give_up_timeslice {
	int		error;		/* error report */
	int		domain; 	/* domain id */
} give_up_timeslice;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, CREATE_RCXP, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	create_rcxp arg;
 */

#define CREATE_RCXP	(GSCIO | 18)

typedef struct _create_rcxp {
	int		error;
	globalID	id;
	int		priority;
	int		flags;
	genericPtr	pData;
	int		length;
	RCXP_Handle	rcxp;
} create_rcxp;

/*
 *	Values possible for the create_rcxp flags
 */
#define RCXP_RELOCATABLE	0x1
#define RCXP_INDIRECT		0x2

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DELETE_RCXP, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	delete_rcxp arg;
 */

#define DELETE_RCXP	(GSCIO | 19)

typedef struct _delete_rcxp {
	int		error;
	RCXP_Handle	rcxp;
} delete_rcxp;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, ASSOCIATE_RCXP, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	associate_rcxp arg;
 */

#define ASSOCIATE_RCXP	(GSCIO | 20)

typedef struct _associate_rcxp {
	int		error;
	RCX_Handle	rcx;
	int		length;
	RCXP_Handle	asso[5];
} associate_rcxp;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DISASSOCIATE_RCXP, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	disassociate_rcxp arg;
 */

#define DISASSOCIATE_RCXP	(GSCIO | 21)

typedef struct _disassociate_rcxp {
	int		error;
	RCX_Handle	rcx;
	int		length;
	RCXP_Handle	dasso[5];
} disassociate_rcxp;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, COMMAND_LIST, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	command_list arg;
 */

#define COMMAND_LIST	(GSCIO | 29)

#define MAX_CMDS	5

typedef struct _command_list {
	int		error;
	int		count;
	struct {
		int command;
		char *carg;
	} cmd_list[MAX_CMDS];
} command_list;


/*	-----------------------------------------------------------------
  Direct Memory Addressing structures

	aixgsc(gsc_handle, DMA_SERVICE, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	gscdma arg;
 */
#define DMA_SERVICE	(GSCIO | 30)	/* direct memory addressing */

#define MAX_SUBAREAS	0x04		/* maximum number of dma buffers
					   in one transaction */

typedef struct _gscdma {
    ulong error;		/* GSC specific error code */
#define DMA_INCOMPLETE	0x01	/* response for DMA_POLL; error=0 if complete */
    ulong flags;		/* function flags */

#define DMA_WAIT	0x01	/* wait until a DMA operation is complete */
				/* else return without waiting */
#define DMA_POLL	0x02	/* see if DMA has completed */
#define DMA_READ	0x80	/* adapter to system DMA operation */
#define DMA_WRITE	0x40	/* system to adapter DMA operation */
#define DMA_RDWR	0x20	/* two-way DMA operations */
#define DMA_FREE	0x100	/* free all DMA resources for the process */
#define DMA_START_FLAG	0x200	/* look at DMA_START flag on sw */
#define DMA_DIAGNOSTICS 0x8000	/* diagnostics mode operations */

    caddr_t dma_cmd;		/* ptr to device-dependent dma cmd info */
    long cmd_length;		/* dma command length */
    int num_sw; 		/* number of subwindows */
    struct sw {
	int sw_flag;		/* subwindow flags */
#define DMA_PINNED	0x04	/* data is already pinned */
#define LEAVE_PINNED	0x08	/* leave data buffers pinned for subsequent
					operations */
#define DMA_INVALIDATE	0x10	/* invalidate cache but do not flush */
#define DMA_START_SW	0x20	/* start this slave subwindow if given */
	caddr_t sw_addr;	/* subwindow address */
	int sw_length;		/* length of subwindow */
    } subwindow [MAX_SUBAREAS];
} gscdma;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	dev_dep_fun arg;
 */

#define DEV_DEP_FUN	(GSCIO | 50)

typedef struct _dev_dep_fun {
	int	  error;	  /* error code */
	int	  cmd;		  /* device specific operation to perform */
	caddr_t   ddi;		  /* device dependent data */
	int	  ddi_len;	  /* length of ddi */
} dev_dep_fun;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, GSC_RCM_REQ, cmd, parm1, parm2, parm3, parm4, parm5)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 */

#define GSC_RCM_REQ	(GSCIO | 51)

/* permitted values for cmd */
#define gsc_SHM_INIT		1
#define gsc_SHM_EXTEND		2
#define gsc_SHM_DELETE		3

/* permitted values for parm1, for the gsc_SHM_* cmds */
#define gsc_SHM_Unused		0		/**//* dsgn did not define */
#define gsc_SHM_CurrDispBuff	1
#define gsc_SHM_MBX		2
#define gsc_SHM_hwSync		3
#define gsc_SHM_Spare4		4
#define gsc_SHM_Spare5		5
#define gsc_SHM_Spare6		6
#define gsc_SHM_Spare7		7

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, GSC_DEVICE_REQ, cmd,
 *					parm1, parm2, parm3, parm4, parm5)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 */

#define GSC_DEVICE_REQ	(GSCIO | 52)

/*------------------
  Event handling types and structures
  -----------------*/
#define BAD_ARRAY_SIZE 99

typedef ulong eventMask;      /* Device specific mask for event types */

typedef struct _eventMasks {
     eventMask	  s_mask;    /* synchronous event mask */
     eventMask	  a_mask;    /* asynchronous event mask */
     eventMask	  e_mask;    /* events requested for continous enable */
     eventMask	  occurred;  /* interrupts pending service */
} eventMasks;


typedef struct _eventReport {
  eventMask	  event;    /* Event type mask */
  time_t	  time;     /* timestamp  */
  RCX_Handle	  rcx;	    /* rendering context ID	*/
			    /* If null then no RCX id */
  WA_Handle	  wa;	    /* window attribute ID    */
			    /* If null the process has no window */
  int		  data[4];  /* device dependent event data */
} eventReport;


typedef struct _eventArray {
  int	 a_size;	 /* size of event array */
  int	 number_req;   /* number of events requested */
  int	 number_used;  /* actual number of events reported */
		       /* set to negative if overflowed */
  eventReport event[1]; /* array of events */
} eventArray, *eventArrayPtr;


/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, ASYNC_EVENT, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct async_event arg;
 */

#define ASYNC_EVENT	 (GSCIO | 69)
struct async_event {
   long        error;	   /* error report */
   eventMask   mask;	   /* Mask for events */
   long        num_events; /* number to report */
};

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, GET_EVENTS, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct get_events arg;
 */
#define GET_EVENTS	 (GSCIO | 70)
struct get_events {
   long        error;	   /* error report */
   eventArray  *array;	   /* array to place events in */
   long        num_events; /* number to report */
};



/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, EVENT_BUFFER, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct event_buffer arg;
 */

#define EVENT_BUFFER	 (GSCIO | 71)
struct event_buffer {
   long        error;	    /* error report */
   long        length;	   /* event buffer length */
};

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, WAIT_EVENT, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct wait_event arg;
 */

#define WAIT_EVENT	 (GSCIO | 72)

struct wait_event {
   long        error;	   /* error report */
   eventMask   mask;	   /* Mask for events */
   eventReport report;	   /* Holder to place data into */
   genericPtr  pData;	   /* event buffer */
   long        length;	   /* event buffer length */
   long        wait;	   /* wait flag  1=wait   */
};


/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, ENABLE_EVENT, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	struct enable_event arg;
 */

#define ENABLE_EVENT	   (GSCIO | 73)

struct enable_event {
   long        error;	    /* error report */
   eventMask   e_event;     /* event mask to be enabled */
};


/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, CREATE_COLORMAP, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	create_colormap arg;
 */

#define CREATE_COLORMAP      (GSCIO | 74)

typedef struct _create_colormap {
	int		error;		/* error report */
	CM_Handle	cm_handle;	/* returned id */
} create_colormap;

/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, DELETE_COLORMAP, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	delete_colormap arg;
 */

#define DELETE_COLORMAP      (GSCIO | 75)

typedef struct _delete_colormap {
	int		error;		/* error report */
	CM_Handle	cm_handle;	/* returned id */
} delete_colormap;


/*	-----------------------------------------------------------------
 *
 *	aixgsc(gsc_handle, UPDATE_COLORMAP, &arg)
 *	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 *	update_colormap arg;
 */

#define UPDATE_COLORMAP      (GSCIO | 76)

typedef struct _update_colormap {
	int		error;		/* error report */
	CM_Handle	cm_handle;	/* returned id */
	int		new_hwdmap;	/* requested hardware map number */
} update_colormap;


/*      ------------------------------------------------------ 
 *                                                            
 *      aixgsc(gsc_handle, DISP_PM, cmd, parm1)                
 *      GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
 */                                                           
                                                             
#define DISP_PM (GSCIO | 77)                                   
                                                           
/* permitted values for cmd */                            

#define CHANGE_POWER_STATE   1                                  
                                                        
/* definitions for parm1 
 *
 * parm1 = state to change.  The possble power consumption states are 
 *
 *   1 = display on
 *   2 = display standby
 *   3 = display suspend
 *   4 = display off
 *
 *   The following values are only used by Woodfield and its follow-on products 
 *
 *   5 = LCD_ON 
 *   6 = LCD_OFF 
 *   7 = CRT_ON 
 *   8 = CRT_OFF 
 */

#define LCD_ON   5
#define LCD_OFF  6
#define CRT_ON   7
#define CRT_OFF  8

/*
 *	end of aixgsc() commands	**********************************
 */

/*
 *	Performance trace definitions
 */

#define PTID_ENTRY	0
#define PTID_EXIT	1

#endif /* _H_GSC */
