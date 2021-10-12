/* @(#)27  1.5.1.4  src/bos/kernext/disp/ped/pedmacro/mid.h, pedmacro, bos411, 9428A410j 3/23/94 17:09:05 */

/*
 *   COMPONENT_NAME: PEDMACRO
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _MID_H_
#define _MID_H_

/*-----------
	 This spec documents the software interface to the Middle Level
	Graphics Adapter Device Driver.  This device driver is used by:
	1) hardware diagnostics and hardware test, 2) the high function
	terminal (HFT), 3) RMS and 2D, 4) 3DM1 and 3DM2, and 5) the config
	method.

	Applications communicate with the Middle Level Graphics Adapter
	Device Driver (MLGADD), and any other graphics adapter device
	drivers for that matter, through the AIX graphics system call
	(aixgsc()).

	The interface to the aixgsc() call is described in 'The Rendering
	Context Manager and the Display Device Handler Architecture and
	Programming Specifications' manual. Comments to the RCM spec are
	currently being collected by Jeanne Smith (JKSMITH@AUSVMQ ,
	x3-5246).

	Using an aixgsc() call requires reading and understanding the
	the RCM spec: this interface does not replace it.

	The RCM spec describes device dependent components of the
	aixgsc() calls. This spec describes the implementation for
	those.

	To make aixgsc() calls, an application must include aixgsc.h.
	To include the structures described here, an application must
	include mid.h.
-------------*/


/*------------
	MID_MAKE_GP:
	For information on the graphics system call to make a process
	a graphics process, consult 2.2.1 of the RCM spec.
	For information about the device dependent requirements to the
	make_gp() call, consult 2.4.1 of the RCM spec.
	For further information refer to the graphics system call interface
	file, aixgsc.h.

	The mid_make_gp struct documents the mid device dependent
	make graphics process interface. The calling application creates a
	copy of this structure, and passes a pointer to it in the pData
	element of the aixgsc make_gp call. The aixgsc() call caused the
	data structure to be filled with the data necessary to write to
	the adpater.

	NOTE FOR DIAGNOSTICS: diagnostics should use the EXCLUSIVE_ACCESS
	flag in the make_gp structure.
-------------*/

/*-------------

Suggestions for coding the aixgsc() MAKE_GP call.

	aixgsc(gsc_handle, MAKE_GP, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	make_gp         arg;
	make_gp.pData = mid_make_gp;
	make_gp.length = sizeof(mid_make_gp);
-------------*/

typedef struct mid_make_gp {
	char *bus_addr;                 /* bus address without segment reg    */
	ulong hwconfig;                 /* hardware configuration flags       */
#       define  MID_VPD_PPR     0x01    /* host i/f & processor card    */
#       define  MID_VPD_PGR     0x02    /* screen buf & graphics card   */
#       define  MID_VPD_POP     0x04    /* 24 bit option card           */
#       define  MID_VPD_PPC     0x08    /* process pipe card            */
#       define  MID_VPD_PPZ     0x10    /* process Z buffer card        */
#       define  MID_CONFIG      0x40    /* diagnostics mode             */
#       define  MID_BAD_UCODE   0x80    /* bad ucode download           */
	ulong ucode_version;            /* microcode version                  */
	ulong screen_width_mm;          /* width from config database         */
	ulong screen_height_mm;         /* height from config database        */
} mid_make_gp_t;



/*-------------
    MID_DMA_SERVICE:

	For information on the graphics system call to perform a direct
	memory access, consult 2.2.1 of the RCM spec.
	For information about the device dependent requirements to the
	dma_service() call, consult 2.4.1 of the RCM spec.
	For further information refer to the graphics system call interface
	file, aixgsc.h.

	The mid device driver offers 6 different DMA interfaces: read, write,
	read and send the DMA request through the PCB, write the same way,
	DMA a large structure element to the FIFO, and perform DMA for
	diagnostics.  The two reads use the same data structure, likewise
	with the first two writes.  DMA'ing the structure element to the
	FIFO uses a third data structure. The diagnostics support requires
	a fourth.

	To implement a DMA call, create the device dependent data structure
	for whichever DMA required (mid_dma_read, mid_dma_write,etc). Fill
	in the structure refering to the Software Interface Specification.

	Next, create the device dependent data structure middma_t. Fill
	this data structure as follows:
	1) set the flag for the DMA type required,
	2) set the size field using the sizeof function,
	3) set the data pointer to the device dependent data struture
	created as described above.

	Finally, create the device independent dma data structure gscdma
	(refer to aixgsc.h), fill in the fields as described in the RCM
	spec. Set the data pointer to the middma_t structure created as
	described above.
-------------*/

#define MAX_SINGLE_DMA  0x500000        /* 1280 x 1024 x 4 bytes        */

typedef struct mid_dma_read
{
  ushort   length;                    /* Structure element length             */
  ushort   opcode;                    /* Structure element opcode             */
  ushort   corr;                      /* Correlator                           */
  ushort   bflags;                    /* 8/24, win/scr, pack, y dir, color map*/
  ulong    hostaddr;                  /* Host destination address             */
  ulong    stride;                    /* Stride factor                        */
  ulong    sourcebp;                  /* Source bit plane                     */
  short   sourcex;                    /* Source upper left x coordinate       */
  short   sourcey;                    /* Source upper left y coordinate       */
  ushort   width;                     /* Blit width in pixels                 */
  ushort   height;                    /* Blit height in pixels                */
} mid_dma_read_t;

typedef struct mid_dma_write
{
  ushort   length;                    /* Structure element length             */
  ushort   opcode;                    /* Structure element opcode             */
  ushort   corr;                      /* Correlator                           */
  ushort   bflags;                    /* 8/24,win/scr,DMA/PIO,cexp,ydir,clrmap*/
  ulong    hostaddr;                  /* Host destination address             */
  ulong    stride;                    /* Stride factor                        */
  ulong    destinbp;                  /* Destination bit plane                */
  short   destinx;                    /* Destination upper left x coordinate  */
  short   destiny;                    /* Destination upper left y coordinate  */
  ushort   width;                     /* Blit width in pixels                 */
  ushort   height;                    /* Blit height in pixels                */
  float    mcx;                       /* Modelling upper left x corrdinate    */
  float    mcy;                       /* Modelling upper left y corrdinate    */
  float    mcz;                       /* Modelling upper left z corrdinate    */
  ulong    dbpos;                     /* Byte position of first pixel / row   */
  ushort   xrepl;                     /* X replication count                  */
  ushort   yrepl;                     /* Y replication count                  */
  ulong    fgcolor;                   /* Foreground color                     */
  ulong    bgcolor;                   /* Background color                     */
  ushort   bgop;                      /* Backgroung mix math/logical op       */
  ushort   fgop;                      /* Foregroung mix math/logical op       */
} mid_dma_write_t;

typedef struct mid_dma_se_write
{
  ushort   length;                    /* Structure element length             */
  ushort   opcode;                    /* Structure element opcode             */
  ushort   corr;                      /* Correlator                           */
  ushort   reserved;                  /* Reserved                             */
  ulong    hostaddr;                  /* Host source address                  */
  ulong    hostlen;                   /* Host source length                   */
} mid_dma_se_write_t;

typedef struct mid_dma_trace_read
{
  ushort   length;                    /* Structure element length             */
  ushort   opcode;                    /* Structure element opcode             */
  ushort   corr;                      /* Correlator                           */
  ushort   reserved;                  /* Reserved                             */
  ulong    hostaddr;                  /* Host source address                  */
  ulong    hostlen;                   /* Host source length                   */
} mid_dma_trace_read_t;

typedef struct mid_dma_font_write
{
  ushort   length;                    /* Structure element length             */
  ushort   opcode;                    /* Structure element opcode             */
  ushort   corr;                      /* Correlator                           */
  ushort   reserved;                  /* Reserved                             */
  ulong    hostaddr;                  /* Host source address                  */
  ulong    hostlen;                   /* Host source length                   */
  ulong    font_id;                   /* Font id of pinned font               */
} mid_dma_font_write_t;

typedef struct mid_dma_pick_m1_write
{
  ushort   length;                    /* Structure element length             */
  ushort   opcode;                    /* Structure element opcode             */
  ushort   corr;                      /* Correlator                           */
  ushort   reserved;                  /* Reserved                             */
  ulong    hostaddr;                  /* Host source address                  */
  ulong    hostlen;                   /* Host source length                   */
  ulong    pick_x;                    /* Pick point x coordinate              */
  ulong    pick_y;                    /* Pick point y coordinate              */
  ulong    pick_path;                 /* Pick path                            */
  ulong    pick_device;               /* Pick device                          */
} mid_dma_pick_m1_write_t;

typedef struct mid_dma_pick_m1m_write
{
  ushort   length;                    /* Structure element length             */
  ushort   opcode;                    /* Structure element opcode             */
  ushort   corr;                      /* Correlator                           */
  ushort   reserved;                  /* Reserved                             */
  ulong    hostaddr;                  /* Host source address                  */
  ulong    hostlen;                   /* Host source length                   */
} mid_dma_pick_m1m_write_t;

typedef struct mid_dma_diagnostics_write
{
  ushort   length;                    /* Structure element length             */
  ushort   opcode;                    /* Structure element opcode             */
  ushort   corr;                      /* Correlator                           */
  ushort   flags;                     /* Diagnostics flags                    */
  ulong    hostaddr;                  /* Host source address                  */
  ulong    reserved1;                 /* Reserved field for diagnostics       */
  ulong    reserved2;                 /* Reserved field for diagnostics       */
  ulong    reserved3;                 /* Reserved field for diagnostics       */
  ulong    reserved4;                 /* Reserved field for diagnostics       */
  ulong    reserved5;                 /* Reserved field for diagnostics       */
} mid_dma_diagnostics_write_t;

/*------------

Suggestions for coding the aixgsc() DMA_SERVICE call.

	aixgsc(gsc_handle, DMA_SERVICE, &dma)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct _gscdma dma;
	dma.dma_cmd = &(mid_dma_t)middma;
	dma.cmd_length = sizeof(mid_dma_t);
-------------*/

typedef struct mid_dma {
	ulong flags;                    /*  Define the specific DMA request   */
#define MID_DMA_READ            0x1     /*  H/W to CPU DMA - though FIFO      */
#define MID_DMA_WRITE           0x2     /*  CPU to H/W DMA - though FIFO      */
#define MID_DMA_PCB_READ        0x4     /*  H/W to CPU DMA - though PCB       */
#define MID_DMA_PCB_WRITE       0x8     /*  CPU to H/W DMA - though PCB       */
#define MID_DMA_SE_WRITE        0x10    /*  DMA SE to H/W                     */
#define MID_DMA_DIAGNOSTICS     0x100   /*  diagnostics DMA                   */
#define MID_DMA_FORCE_ERROR     0x200   /*  diagnostics can force DMA error   */
	ulong se_size;                  /*  size of mid DMA struct            */
	void *se_data;                  /*  pointer to mid DMA struct         */
	ulong error;                    /*  error return codes                */
#define MID_DMA_NOERROR         0x0     /*  All OK                            */
#define MID_DMA_TIMEOUT         0x1     /*  DMA timed out                     */
#define MID_DMA_BAD_STRUCT      0x2     /*  DMA data structure or request     */
	                                /*  invalid: Ex: 24 bit req on 8 bit  */
	                                /*  H/W.                              */
} mid_dma_t;



/*--------------
    MID EVENT SUPPORT:

	For information on the graphics system calls to perform event
	support, consult 4.5.1 async_mask(), and 4.5.2 sync_mask(), of
	the RCM spec.
	For further information refer to the graphics system call interface
	file, aixgsc.h.

	The event support data structure are described in the RCM
	spec.  The mid device driver does implement event support.
	The list below includes the supported interrupts.
-------------*/

#define BEGINPICKM1M            0x1000
#define ENDPICKM1M              0x2000
#define BEGINPICKM1             0x4000
#define ENDPICKM1               0x8000
#define GSYNC                   0x10000
#define DD_PIO_HIGH_WATER       0x1
#define DD_WRITE_TO_DSP_COMMO   0x2
#define DD_CHANNEL_CHECK        0x4
#define DD_DSP_HAS_READ_HCR     0x8
#define DD_DSP_SOFT_INTR_0      0x10
#define DD_DSP_SOFT_INTR_1      0x20
#define DD_DSP_SOFT_INTR_2      0x40
#define DD_DSP_SOFT_INTR_3      0x80
#define DD_PRIORITY_FIFO_EMPTY  0x100
#define DD_PRIORITY_FIFO_FULL   0x200
#define DD_DIAGNOSTICS_COMPLETE 0x400



/*-----------
    DEVICE DEPENDENT FUNCTIONS:

	For information on the graphics system calls that provide device
	dependent function, consult 4.5.4 of the RCM spec.
	For further information refer to the graphics system call interface
	file, aixgsc.h.

	The mid device driver offers 8 other device dependent aixgsc()
	functions:
	1) get color - get the current rendering color,
	2) get character position - get the current character position,
	3) get modelling matrix - get the current modelling matrix,
	4) get projection matrix - get the current modelling matrix,
	5) swap buffers - swap rendering and work buffer,
	6) get condition - graPHIGS status, culling and pruning data,
	7) get text font index - graPHIGS current index into the font table,
	8) end render - issue the graPHIGS End Rendering command element to
	   the adapter and wait for a response.

	The calling processes are put to sleep while any of these calls
	are pending (except for swapbuffers).
-------------*/

/*-----------
	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct dev_dep_fun arg;
	struct mid_getcolor ddi;

	        arg.cmd = MID_GETCOLOR;
	        arg.ddi = &ddi;
	        arg.ddi_len = sizeof (mid_getcolor_t);
-------------*/

#       define MID_GETCOLOR                 330

typedef struct mid_getcolor {
	float red;                      /* red value                          */
	float green;                    /* green value                        */
	float blue;                     /* blue value                         */
} mid_getcolor_t;

/*-----------

Suggestions for coding the aixgsc() DEV_DEP_FUN call.

	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct dev_dep_fun arg;
	struct mid_getcpos ddi;

	        arg.cmd = MID_GETCPOS;
	        arg.ddi = &ddi;
	        arg.ddi_len = sizeof (mid_getcpos_t);

-------------*/

#       define MID_GETCPOS                  327

typedef struct mid_getcpos {
	ushort xcpos;                   /* current character position in x    */
	ushort ycpos;                   /* current character position in y    */
} mid_getcpos_t;

/*-----------

Suggestions for coding the aixgsc() DEV_DEP_FUN call.

	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct dev_dep_fun arg;
	struct mid_get_projection_matrix ddi;

	        arg.cmd = MID_GET_PROJECTION_MATRIX;
	        arg.ddi = &ddi;
	        arg.ddi_len = sizeof (mid_get_projection_matrix_t);

-------------*/

#       define MID_GET_PROJECTION_MATRIX    328

typedef struct mid_get_projection_matrix {
	float projection_matrix[4][4];  /* projection matrix                  */
} mid_get_projection_matrix_t;

/*-----------

Suggestions for coding the aixgsc() DEV_DEP_FUN call.

	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct dev_dep_fun arg;
	struct mid_get_modelling_matrix ddi;

	        arg.cmd = MID_GET_MODELLING_MATRIX;
	        arg.ddi = &ddi;
	        arg.ddi_len = sizeof (mid_get_modelling_data_t);

-------------*/

#       define MID_GET_MODELLING_MATRIX    329

typedef struct mid_get_modelling_matrix {
	float modelling_matrix[4][4];   /* modelling matrix                   */
	float inv_trans_matrix[3][3];   /* inverse transpose matrix           */
	ushort  renormalize_flag;       /* renormalize bit flags              */
	ushort  matrix_type;            /* matrix composition type            */
} mid_get_modelling_matrix_t;

/*-----------

Suggestions for coding the aixgsc() DEV_DEP_FUN call.

	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct dev_dep_fun arg;
	struct mid_swapbuffers ddi;

	        arg.cmd = MID_SWAPBUFFERS;
	        arg.ddi = &ddi;
	        arg.ddi_len = sizeof (mid_swapbuffers);

-------------*/

#       define MID_SWAPBUFFERS               331

typedef struct mid_swapbuffers {
	ulong uflags;           /* Update flags for draw and display section */
	ulong fbcflags;         /* Frame buffer control flags                */
} mid_swapbuffers_t;


/*-----------

Suggestions for coding the aixgsc() DEV_DEP_FUN call.

	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct dev_dep_fun arg;
	struct mid_getcondition ddi;

	        arg.cmd = MID_GETCONDITION;
	        arg.ddi = &ddi;
	        arg.ddi_len = sizeof (mid_getcondition);

-------------*/

#       define MID_GETCONDITION              332

typedef struct mid_getcondition {
	short correlator;               /* correlator sent by 3DM1           */
	short wait_flag;                /* does the caller want to wait to   */
	                                /* get "current" status word? N=0    */
	ulong status_word;              /* current character position in y    */
} mid_getcondition_t;


/*-----------

Suggestions for coding the aixgsc() DEV_DEP_FUN call.

	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct dev_dep_fun arg;
	struct mid_gettextfontindex ddi;

	        arg.cmd = MID_GETTEXTFONTINDEX;
	        arg.ddi = &ddi;
	        arg.ddi_len = sizeof (mid_gettextfontindex);
	        arg.ddi.correlator = <correlator determined by calling program

-------------*/

#       define MID_GETTEXTFONTINDEX          333

typedef struct mid_gettextfontindex {
	short correlator;          /* correlator sent by 3DM1           */
	ulong textfontindex;       /* current index into the font table */
} mid_gettextfontindex_t;





#       define MID_TRANSFER_WID_TO_DD          334
  /*-----------------------------------------------------------------------
	There is a partitioning of the WIDs (window IDs) between X and the
	device driver.  At initialization time, the device driver gets
	the first 6 WIDS and X get the other 10.  There is also a mechanism
	for X to give the device driver additional WIDs with the following
	DDF function.
   *-----------------------------------------------------------------------*/

#define MID_NUM_WIDS_START_DWA          6
#define MID_NUM_WIDS_MAX_DWA            10
  /*-----------------------------------------------------------------------
	/* Not all the WIDs can be for multi-buffer functions
	at once.  Some must be preserved as "available" to make the algorithm
	work in all cases of a dynamic system.
   *-----------------------------------------------------------------------*/
#define MID_NUM_WIDS_EXCLUDED_FROM_MULTI_BUF   3

/*
Suggestions for coding the aixgsc() DEV_DEP_FUN call.

	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct dev_dep_fun arg;
	struct mid_transfer_WID ddi;

	        arg.cmd = MID_TRANSFER_WID_TO_DD ;
	        arg.ddi = &ddi;
	        arg.ddi_len = sizeof (mid_transfer_WID);
	        arg.ddi.correlator = correlator determined by calling program

  -------------------------------------------------------------------------*/

typedef struct mid_transfer_WID {
	ulong WID;                      /* actual WID to give to DD */
} mid_transfer_WID_t ;



#       define MID_RESET_TO_DEFAULT_WIDS       335
  /*-----------------------------------------------------------------------
	This request is connected with the previous request that X uses to
	transfer a WID to the device driver.  In the event that all DWA
	clients are closed, X can request all the WIDs to be returned.
	(The device driver still retains the option of declining the request
	with a negative return code).
   *-----------------------------------------------------------------------*/

/*
Suggestions for coding the aixgsc() DEV_DEP_FUN call.

	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct dev_dep_fun arg;


	        arg.cmd = MID_TRANSFER_WID_TO_DD ;
	        arg.ddi = NULL;
	        arg.ddi_len = 0 ;

  -------------------------------------------------------------------------*/


/*-----------

Suggestions for coding the aixgsc() DEV_DEP_FUN call.

	aixgsc(gsc_handle, DEV_DEP_FUN, &arg)
	GSC_HANDLE gsc_handle - return from ioctl(fd, HFHANDLE, &gsc_handle)
	struct dev_dep_fun arg;

	        arg.cmd = MID_END_RENDER;
	        arg.ddi = NULL;
	        arg.ddi_len = 0;

-------------*/

#       define MID_END_RENDER                   336


/*----------------------------------------------------------------------------
 *         Return codes specific to Device Dependent Functions
 *
 * These return codes must all be greater than MID_DDF_RC_MIN_VALUE
 *
 * Note that general system error codes defined in errno.h may also be
 * returned from the DDFs
 *---------------------------------------------------------------------------*/

#define MID_DDF_RC_MIN_VALUE    (1L << 10)
#define MID_DDF_RC_OK           0
#define MID_DDF_RC_NO_CONTEXT   MID_DDF_RC_MIN_VALUE + 1



/* HFT Diagnostics Query test numbers */
#define MID_POS_TEST            0x01
#define MID_POS2_BIT3_SET       0x02
#define MID_POS2_BIT3_RESET     0x04

/*-----------
    CONFIG METHOD INTERFACE:
	This interface is used by the config method to get microcode level
    information as well as the VPD data.  The config method puts this
    information into the config database.
-------------*/

/*-----------
    Each card has 256 bytes of VPD data.
-------------*/

#       define  MAX_VPD_LEN     256     /* bytes per VPD                */

/*-----------
     Here is the structure returned by the VPD ioctl call.
-------------*/

struct mid_vpd {

	uchar   vpd[3 * MAX_VPD_LEN];   /* 2 or 3 VPDs configured       */
	ulong   microcode_version;      /* version microcode loaded     */
	ulong   configuration;          /* cards configured             */
	                                /* MID_VPD_PPR host i/f & processor card    */
	                                /* MID_VPD_PGR screen buf & graphics card   */
	                                /* MID_VPD_POP 24 bit option card           */
	                                /* MID_VPD_PPC process pipe card            */
	                                /* MID_VPD_PPZ process Z buffer card        */
};

/*-----------
    POS IOCTL INTERFACE:
    This interface is used by the POS ioctl to get the POS register
    information.  The POS ioctl puts this
    information (mid_pos) into the query diagnostics response structure
    pointed by hf_result.  The caller is responsible for allocating all of
    necessary structures.
-------------*/

struct mid_pos {

	char    pos;                    /* POS register status          */
#       define  MID_POS_2_OK    0x04    /* Good POS register 2          */
#       define  MID_POS_4_OK    0x10    /* Good POS register 4          */
#       define  MID_POS_5_OK    0x20    /* Good POS register 5          */
#       define  MID_POS_7_OK    0x80    /* Good POS register 7          */
};






/*-----------
NOTES:
	1) Currently the device driver is running on the first spin of the
	mid uchannel interface chip.
	2) If the device driver tries to malloc and fails, it will return
	this no memory value.
-------------*/

#define MID_NOMEMORY    1

/*-----------
    The following definitions apply to the create context interface.
    A data type (typedef) is defined for the context type.  In addition,
    defines are provided for the 3 types of Ped contexts.
-------------*/

#define MID_FIFO_DOMAIN         0
#define MID_PCB_DOMAIN          1          /* Same as INDirect domain */
#define MID_IND_DOMAIN          1          /* Same as PCB domain */

typedef  int    mid_rcx_type_t ;

typedef struct mid_create_rcx
{
	mid_rcx_type_t  rcx_type;
#                       define  RCX_2D      0    /* 2D context */
#                       define  RCX_3DM1    1    /* 3D mod 1 */
#                       define  RCX_3DM2    3    /* 3D mod 2 */
#                       define  RCX_PCB     4    /* PCB */
} mid_create_rcx_t;

#endif /* _MID_H */


