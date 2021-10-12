/* @(#)32       1.2.2.2  src/bos/kernext/lft/inc/fkprocFont.h, bos, bos410 10/25/93 13:41:10 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: isKSRfont
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_FKPROCFONT
#define _H_FKPROCFONT

#include<sys/types.h>        /* all 3 for shared memory */
#include<sys/errno.h>
#include<sys/xmem.h>
#include <sys/display.h>  

/* -------------------------- FONT RELATED INFORMATION  --------------- */


/*-----------------------------
   AT most 2 fonts can be pinned by an adapter - one for a huge font to
   be pinned for the duration of application; the other for small fonts
   which adapter has to pin and unpin if it wants to pin another. There 
   is a finite amount of resource (memory) so we have to set a limit of 
   pinned memory per adapter.
------------------------------ */

#define MAX_PIN_FONTS_ALLOW		2

/*-----------------------------
   X FONTS:
	For X fonts, they are in a known shared memory segment.  We
        get the virtual address to this segment at make_gp time and save
        it in struct midddf.  When a font fault occurs we get the fontid 
        which is a 28 bit offset into this segment.  To get the virtual 
        address of where this font is (in order to pin or unpin it) we 
        just have to extract the segment id from the shared memory segment 
        virtual address and OR it with the offset.

        This mask is use to extract the segment id from the segment
        id of the shared memory. 
------------------------------ */

#define SEGID_MASK     0xf0000000  /* only want segment id (upper 4 bits) */ 
#define SEG_OFFSET_MASK 0x0fffffff  /* want last 28 bits of of a virtual addr */ 

/*---------------------------
   KSR FONTS:
	KSR fonts are parts of the lft.  These fonts are in a different
        segment.  Through the phys_display structure follow the pointer
        to common data, we get to the font pallettes.  For this kind
        of font faults, we get a font id (32 bit number) which have
        the following information:

	The upper 4 bits are set so we can tell it is KSR font id 
        (the X font id's have the upper 4 bits unset).  The rest of
        the bits contain a value (0-7) which we use as index into the
        font pallette.  Note the size of the font pallette (an array) 
        is 8 
---------------------------- */

#define KSR_FONT_MASK		0xf0000000    /* is it KSR font */


/*-----------------------
   No font id (X or KSR) can be of this value.  Use to initialize 
   list of pinned fonts.  Note each adapter can have 2 pinned fonts at most.
-------------------------- */
#define NO_FONT_ID		0


/*-----------------------
   structure used to compute font address in memory and its length 
-------------------------- */
typedef struct _font_addr_n_len {
	caddr_t addr;			/* virtual address of the font */
	unsigned int len;		/* its length		       */ 
	int	unused[2];
} font_addr_n_len;

/* ------------------- COMMAND QUEUE RELATED INFORMATION  --------------- */

/*-----------------------
   Maximum outstanding requests to do.  This value must be a power of 
   2 for the method of calculating wrap around to work properly.

   Each adapter is allowd to pin at most 2 fonts (resource usage).  Since
   we might have up to 4 adapters in the same machine, up to 8 fonts can 
   be pinned at any given point in time.  Adapters won't issue command 
   to unpin a font until it gets it and does somthing with it.  Say
   in a worse case we have unpin commands for every pinned fonts; this
   number can't be higher than 8.  So in the worse case, a queue of 16 
   elements is enough.  Since this is a circular queue, as we take
   things off the queue, we free more space.  It is very unlikely that
   we will ever have 16 entries in the queue.   

   This value was increased to 32 to account for additional use by the
   rby adapter.
------------------------ */

#define MAX_QES     32                   /* capacity of command queue */ 




/*---------------------------
   Q element:
	The q element contains all the information needed to pin a font,
        create shared memory segment, destroy shared memory segment, etc. 
 
 --------------------------- */
typedef struct _fkprocQE {

    int         command;        /* command / req code,  EG. pin (a font)  */

   /*---------------------------
     Generic data pointer for most requests (obviously) request dependent.
     The different request structures are defined below. 
    --------------------------- */
   char * request_data ;

   /*---------------------------
    data specifically for the UNPIN request
    --------------------------- */
   char * font_addr;
   unsigned int font_len;

   /*---------------------------
    data specifically for the FKPROC_COM_WAIT synchronization capability
    --------------------------- */

   struct  fkproc_com_wait
   {
	int   done;
	int   sleep_done;
	int   unused[2];
   } *pfkproc_com_wait;

   int        unused[4];
} fkprocQE, *fkprocQEPtr;


/*----------------------------
   Font Kproc Q:
	The data structure for the command queue: head, tail indexes
        and the queue itself.   
 ---------------------------- */
typedef struct _fkprocQ {
    long        head;                 /* head ptr of q */
    long        tail;                 /* tail ptr of q */
    int		unused[2];
    fkprocQE       qe[MAX_QES];    /* space for q elements */
} fkprocQ, *fkprocQPtr;


/*-----------------------------
   Fkproc state
	The Fkproc state contains additional information about the
        queue in order to implement enqueue and dequeue.
          
------------------------------ */
typedef struct _fkprocState {
    pid_t       pid;            /* process id of the font Kproc */
    pid_t       i_pid;          /* process id for initializer of Kproc */
    int         flags;          /* status flags */
    int         unused[2];
    fkprocQ        Q;           /* the command queue from driver for fkproc */
} fkprocState, *fprocStatePtr;



/*-----------------------------
   Fkproc PIN Request structure

	The following structure is passed on a pin request.  The device
        dependent layer guarantees uniqueness of this structure.
------------------------------ */

typedef struct _fkproc_font_pin_req {
    ulong 	font_ID ; 		/* passed: font ID */
    struct 	phys_displays * pd;	/* passed: */
    int         DMA_channel ;          	/* passed: DMA channel */
    int         flags;          	/* saved:  DMA flags */
    char       *sys_addr ;       	/* saved:  system memory address */
    int         length ;         	/* saved:  length  */
    struct xmem xm  ;         		/* saved:  xmem addr */
    char       *bus_addr ;       	/* passed:  bus address */
    int         unused[2];
} fkproc_font_pin_req_t  ;


/*-----------------------------
   Fkproc PIN_PART Request structure

	The following structure is passed on a pin partial request.  The device
        dependent layer guarantees uniqueness of this structure.
------------------------------ */

typedef struct _fkproc_font_pin_part_req {
    struct 	phys_displays *pd;
    int         DMA_channel;
    ulong	font_buf_dma_flags;
    char	*font_req_buf;
    int		font_buf_len;
    struct xmem	font_buf_xmd;
    char	*font_buf_bus_addr;
    int		font_buf_flags;
#define FONT_BUF_IS_DMA		(1L<<0)
    pid_t	pid;
    ulong	event;
    int		(*prepare_font_data)();
    int         unused[4];
} fkproc_font_pin_part_req_t  ;


/*-----------------------------
   Fkproc DEV_DEP_FUN Request structure

	The following structure is passed on a dev dep fun request.  The
	device dependent layer guarantees uniqueness of this structure.
------------------------------ */

typedef struct _fkproc_dev_dep_fun_req {
    void	(*dev_dep_function)();
    void        (*unused0)();
    void        (*unused1)();
    void        (*unused2)();
} fkproc_dev_dep_fun_req_t  ;




/* -------------- FONT KERNEL PROCESS RELATED INFORMATION  ------------ */


/*------------------------
    events known to the function creating font kernel support process 
    and the dequeue function 
------------------------- */

#define FKPROC_WAIT_INIT    0x80000000   /* waiting on initialization 
	                    		        of font Kproc             */
#define FKPROC_WAIT_QE      0x40000000   /* waiting on queue element  */

#define FKPROC_INIT        1             /* font Kproc initialized */


/*------------------------
   Font Kproc commnands:
	Commands can be issued to font kernel prococess by driver or
        some other kernel process 
------------------------- */

#define FKPROC_COM_FLAG_MASK 0xffff0000 /* upper bits are for flags */
#define FKPROC_COM_CMD_MASK  0x0000ffff /* lower bits are for commands */

/* commands */
#define FKPROC_COM_PIN                1   /* pin a font */
#define FKPROC_COM_ATTACH             2   /* attach the shared memory */
#define FKPROC_COM_DETACH             3   /* detach the shared memory */
#define FKPROC_COM_TERM               4   /* terminate font kern proc (fkproc)*/
#define FKPROC_COM_UNPIN              5   /* unpin a X font */
#define FKPROC_COM_PIN_PART           6   /* pin part of a font */
#define FKPROC_COM_DDF                7   /* device dependent function */

/* flags */
#define FKPROC_COM_WAIT      0x80000000   /* wait for command to finish */


/*--------------------------
   Return codes for Kproc functions 
--------------------------- */
#define FKPROC_NO_INIT     25      /* return code - not initialized */
#define FKPROC_Q_OVRFLW    26      /* q has overflowed */
#define FKPROC_Q_NO_MEM    27      /* malloc failure during ops */
#define FKPROC_Q_SUCC      0       /* qe queued successfully */






/* ------------------- SHARED MEMROY RELATED INFORMATION  --------------- */

/*-------------------------
   Id requires by ftok() to generate unique key which is subsequently
   used to attach create and/or attach the shared memory segment to the font
   kernel process
-------------------------- */
#define KEY_PATH	"/usr/lpp/fonts"
#define KEY_ID 'X'                   

/* -------------------  FUNCTIONS DECLARATIONS  ---------------- */

extern long pinned_font_ready( fkprocQE * , font_addr_n_len *);

#define isKSRfont(fontid)   ((fontid) & KSR_FONT_MASK)

#endif   /* _H_FKPROCFONT */
