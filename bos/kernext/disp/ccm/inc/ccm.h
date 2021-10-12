/* @(#)17       1.8  src/bos/kernext/disp/ccm/inc/ccm.h, dispccm, bos411, 9428A410j 7/5/94 11:32:56 */
/*
 *
 * COMPONENT_NAME: (SYSXDISPCCM) Common Character Mode Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* ---------------------------------------------------------------------------

 PURPOSE:	Defines the typedefs and data structures used by
		the common character mode VDD and by the various
		configuration methods for display adapters which
		interact with the common character mode VDD.

		It also provides for external references to global
		data structures shared by the object files in a 
		config method. 

 
 INPUTS:  	n/a

 OUTPUTS: 	n/a

 FUNCTIONS/SERVICES CALLED:	n/a 

 DATA:		Provides the typedefs and extern references for the
		following:

	    	(1) n/a
		(2) ccm_dds_t
		(3) ccm_pathnames_t
		(4) ccm_dd_data_t
		(5) ccm_ksr_data_t
		(6) various macros to support the above structures
		(7) ccm_rectangle_t
		(8) ccm_dd_intr_data_t
		(9) ccm_pd_ddf_t

 *****************************************************************************
 ********************** MODULE HEADER BOTTOM *********************************
 *****************************************************************************/





#ifndef __H_CCM
#define __H_CCM

/*===================================================================
|
|	CONSTANTS USED IN THE CCM
|
|====================================================================*/

enum
{

	CCM_LATIN_SPACE_CHAR		= (int) 0x00200000,

	CCM_PS_ATTRIBUTES_MASK		/* from vt.h */
		= (int) (FG_COLOR_MASK | BG_COLOR_MASK | FONT_SELECT_MASK
			 | NO_DISP_MASK | REV_VIDEO_MASK | UNDERSCORE_MASK ),

	CCM_PS_ATTRIBUTES_INITIAL	= (int) ATTRIBUTES_INITIAL,
	CCM_PS_MASK_CHAR		= (int) 0xFF00FFFF,

	CCM_MAX_NUM_FONTS		= (int) 8,
	CCM_CURSOR_DEFAULT_COLOR	= (int) 1,
	CCM_CURSOR_DIMENS_MAX		= (int) 64,
	CCM_CURSOR_AREA_MAX		= (int) ( 64 * 64 ),
	CCM_CURSOR_AREA_MAX_IN_ULONG
		= (int) ((CCM_CURSOR_AREA_MAX / sizeof(ulong)) / 8 ),

	CCM_CURSOR_INVISIBLE		= (int) 0,
	CCM_CURSOR_SINGLE_UNDERSCORE	= (int) 1,
	CCM_CURSOR_DOUBLE_UNDERSCORE	= (int) 2,
	CCM_CURSOR_LOW_HALF_BLOB	= (int) 3,
	CCM_CURSOR_MID_DOUBLE_LINE	= (int) 4,
	CCM_CURSOR_FULL_BLOB		= (int) 5,
	

	CCM_BUS_TYPE_MICROCHANNEL	= (int) ( 1UL << 0 ),
	CCM_BUS_TYPE_220_BUID40		= (int) ( 1UL << 1 ),
	CCM_BUS_TYPE_60x		= (int) ( 1UL << 2 ),
	CCM_60x_ENABLE_CONFIGURATION	= (int) 0x80000000,

	CCM_COLORS_MAX			= (int) 2,
	CCM_COLORS_MAX_FG		= (int) 1,
	CCM_COLORS_MAX_BG		= (int) 1,

	CCM_PD_INFO_COLOR_MONITOR	= (int) (1UL << 31 ),
	CCM_PD_INFO_RW_CMAP		= (int) (1UL << 30 ),
	CCM_PD_INFO_HW_NOT_OK		= (int) (1UL << 29 ),

	CCM_PD_INFO_APA			= (int) (1UL << 31 ),
	CCM_PD_INFO_HAS_BLINK		= (int) (1UL << 30 ),
	
	CCM_PD_INFO_FONT_W		= (int) 12,
	CCM_PD_INFO_FONT_H		= (int) 30,

	CCM_PD_INFO_BITS_PER_PEL	= (int) 1,
	
	CCM_MAX_UCODE_INCR		= (int) ( 16 * 1024 ),

	CCM_MAXNAMELEN			= (int) 256,
	CCM_HOOKUP			= (int) 0x6969,

	/*
	 * Extensions for 60x bus 
	 */

	CCM_60x_ADDR_TO_SEG_MASK	= ( int ) 0xfffffff0,
	CCM_60x_ADDR_TO_SEG_SHIFT	= ( int ) 28,
	CCM_60x_UPPER_NIBBLE_MASK	= ( int ) 0x0fffffff,
	CCM_60x_UPPER_28BIT_MASK	= ( int ) 0x0000000f,


	/*------------------------------
	| always define a dummy, to avoid 
	| problems with commas on the last
	| entry
	--------------------------------*/

	__CCM_LAST_0		= (int) 0

};


	/*-----------------------------------
	| strings cannot be defined in the enum
	|-----------------------------------*/

#define	CCM_IPLROS	"SYS_IPLROS"
#define	CCM_VIDEO_ROS	"ADAPT_VIDEO_ROS"




/*========================================================================
|
|  Common Character Mode Device Driver Define Device Structure
|
|   This structure is the means of exchange between the configuration
|   method for the particular adapter, the CCM VDD, and the system 
|   utilties which support the device switch table.  A pointer to this
|   structure is also provided to the CDD Video ROM scan modules via
|   the device attributes structure.
|
|=========================================================================*/


		/*=================================
		|  ccm_dds_t
		|=================================*/

typedef struct
{
   ulong    io_bus_mem_start[CDD_MAX_BUSMEM_RANGES];       
				    /* adapter address start              */
   ulong    io_bus_mem_length[CDD_MAX_BUSMEM_RANGES];       
				    /* adapter address start              */
   ulong    RISC_rom_addr;	    /* Base address of Video Feqature ROM */
   ushort   slot_number;            /* slot number                        */
   short    int_level;              /* interrupt level                    */
   short    int_priority;           /* interrupt priority                 */
   short    dma_arb_level;	    /* dma_arb_level for HW set pos       */
   ulong    bus_id;		    /* bus id for multiple bus support    */
   ulong    bus_type;		    /* bus type for multiple bus support  */
   int	    device_num;		    /* the device number of the ccm_dd    */
   short    screen_width_pel;	    /* how wide is the screen in pixels	  */
   short    screen_height_pel;	    /* how high is the screen in pixels	  */
   short    screen_width_mm;        /* screen width in millimeters        */
   short    screen_height_mm;       /* screen height in millimeters       */
   ulong    display_id;             /* unique display id                  */
   ulong    ksr_color_table[16];    /* ksr color pallet                   */
   int      microcode_fd;           /* file pointer for display microcode */
   char     component[CCM_MAXNAMELEN];   /* configurable device/component name */
   char     ucode_name[CCM_MAXNAMELEN];  /* microcode path name                */

   mid_t    dd_kmid;			/* kmid of the CCM VDD or full VDD	*/
   mid_t    dd_pin_kmid;		/* kmid of the IH of whichever VDD	*/

   mid_t    cdd_kmid;			/* loaded in cfgmethod			*/
   mid_t    cdd_pin_kmid;		/* usually the IH; may be NULL		*/
   mid_t    cdd_ext_kmid;		/* no extensions planned a.t.t		*/
   mid_t    cdd_ext_pin_kmid;		/* no extensions planned a.t.t		*/

   void *   p_ccm_dd_data;	     /* Forward pointer to ccm dd structure */
   

   	/*---------------------------------
	| the following are all in units of pixels
	| and apply to screen pixel addresses from
	| an origin  (0,0)
	|---------------------------------*/

	ulong	x_min, x_max;		/* min and max range of width */
	ulong	y_min, y_max;		/* min and max range of heigth */

/* Processor & architecture extension for all busses */
/* 
 *These fields are obtained from the config method using the macro
 * provided by systemcfg.h.  The actual values of these fields is NOT
 * the same as in the iplcb.
 */

	int     architecture;           /* CDD_POWER_RS or CDD_POWER_PC */
	int     implementation;         /* CDD_POWER_[RS1 | RS2 | RSC | 601] */


	/*
	 * Extensions for 60xbus
	 */
	
   	short 		int_server;           /* interrupt server   */
        ulong_t         buid_60x[ CDD_MAX_60x_BUID_REGS ];
        ulong_t         segment_60x;

} ccm_dds_t;


/*========================================================================
| 
| 	define some symbols used throughout code dealing with CCM
| 
|=======================================================================*/

#define GOOD_CCM_MCODE_INSTALLED 	0x31      
#define BAD_CCM_MCODE_INSTALLED 	0x32      


#define NUM_DVDR_DIRECTORIES    2       /* number of device driver directories*/
					/* the number must correpsond with    */
					/* the number in dvdr_dirctories      */




/*===========================================================================
|
|	Every device driver keeps a local data structure that is device
|	specific and that hangs off of the phys_display structure.
|	The common character mode device driver does this as well.
|	The ld pointer is used to access this data.  The data defined
|	in the CCM VDD ld structure is intended to work for all versions
|	of hardware which have a CDD VIdeo ROM interface
|
|============================================================================*/


	/*---- This typedef requires the following to have been 
	previously included into the module:

	 include	<sys/aixfont.h>
	 include	<vt.h>
	------------------------------------------------------*/

#define VTT_ACTIVE      1
#define VTT_INACTIVE    0
#define VTT_MONITORED   0
#define VTT_CHARACTER   1

#ifndef max
#define max(val1,val2)          (val1 > val2) ? (val1) : (val2);
#endif

		/*=================================
		| ccm_ksr_data_t
		|================================*/


typedef struct  {

/**********************************************************************/
/*                                                                    */
/* Virtual Driver Mode: Current mode of the Virtual Device Driver     */
/*                             0 => monitored mode                    */
/*                             1 => charaacter mode (KSR)             */
/*                             2 => APA mode (AVT)                    */
/*                                                                    */
/*                      NOTE: the current state of the real device    */
/*                            (rscr_mode) is stored in the RMA.       */
/*                                                                    */
/*                      NOTE: the default mode is character           */
/*                                                                    */
/**********************************************************************/

    long        vtt_mode;

/**********************************************************************/
/*                                                                    */
/* Virtual Driver State: Current state of the Virtual Display Driver  */
/*                             0 => inactive (presentation space      */
/*                                            updated)                */
/*                             1 => active (frame buffer updated)     */
/*                                                                    */
/*                        NOTE: the default state is inactive.        */
/*                                                                    */
/**********************************************************************/


    long        vtt_active;


/**********************************************************************/
/*                                                                    */
/* Offset into the presentation space.  Rather than scroll the        */
/* contents of the presentation space, the Middle moves a pointer,    */
/* scroll_offset, up and down the presentation space.  This pointer   */
/* points to the beginning of the presentation space data, rather     */
/* than the top of the data structure.                                */
/*                                                                    */
/**********************************************************************/


	short           scroll_offset;

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  The data shown on the screen must be centered.  The centering     */
/*  is done with offsets from the lower left corner of the screen.    */
/*                                                                    */
/*--------------------------------------------------------------------*/

	unsigned short          centering_x_offset;

	unsigned short          centering_y_offset;

/**********************************************************************/
/*                                                                    */
/*  Presentation Space:                                               */
/*                                                                    */
/*                      NOTE: the character in the presentation space */
/*                            is initialized as a "space" with a      */
/*                            "green character/black background"      */
/*                            attribute.                              */
/*                                                                    */
/**********************************************************************/

    unsigned long        *pse ;

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Presentation Space Size (bytes)                                   */
/*                                                                    */
/*              Contains the total number of bytes in the ps .        */
/*              (width = height = -1 implies the ps is not allocated).*/
/*                                                                    */
/*              NOTE: default value is -1                             */
/*                                                                    */
/*  Presentation Space Size (full characters)                         */
/*                                                                    */
/*              Contains the current width and height of the ps       */
/*              (width = height = -1 implies the ps is not allocated) */
/*              in characters.                                        */
/*                                                                    */
/*              NOTE: default value is -1                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

   long ps_bytes;                     /* number of bytes in ps         */
   long ps_words;                     /* number of words in ps         */

   struct {
	short ht;                    /* ps height (row)               */
	short wd;                    /* ps width (height)             */
   } ps_size;                        /* dimensions of ps              */

/*--------------------------------------------------------------------*/
/*                                                                    */
/* Font Table:                                                        */
/*                                                                    */
/*   An array of FONT_HEAD pointers pointing to the fonts             */
/*   currently being used by the virtual terminal.                    */
/*                                                                    */
/*   Also an array index for the current font.                        */
/*                                                                    */
/*--------------------------------------------------------------------*/

	aixFontInfoPtr	info_ptr;
	aixCharInfoPtr	char_ptr;
	aixGlyphPtr	glyph_ptr; 


/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Color table                                                       */
/*                                                                    */
/*  This is a shadow copy of what is in the color table on the        */
/*  adapter for this virtual terminal.  This is used to offer         */
/*  different color pallettes for each VT.                            */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/

/* data for number of entries in colorpal */
#  define         	VT_NUM_OF_COLORS  	16

   struct colorpal 	color_table;

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Character glyph Descriptors                                       */
/*                                                                    */
/*  Since KSR does not support proportional spacing, these values     */
/*  apply for all character glyphs in a given font.                   */
/*  Also, since KSR only supports one font type (size) at a time,     */
/*  these values are for any current font.                            */
/*                                                                    */
/*  Note: only lft_sd can legally be negative: all others are error   */
/*  checked in the code.                                              */
/*                                                                    */
/*--------------------------------------------------------------------*/

    struct {
	short ht;              		/* height of character (pels)    */
	short wd;              		/* width of character (pels)     */
	short lft_sd;			/* char base to left of box      */
	short ascnt;			/* baseline to top of box        */
    } char_box;                         /* dimensions of character box   */


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Cursor Attributes:                                                 */
/*                                                                    */
/*              Characteristics of the cursor prior  to the           */
/*              execution of the next VDD command.                    */
/*                                                                    */
/*              NOTE: if the virtural terminal is active, this        */
/*              field contains the attributes of the cursor that      */
/*              is currently being displayed.                         */
/*                                                                    */
/*--------------------------------------------------------------------*/

    struct {
	unsigned short fg;              /* cursor foreground color       */
	unsigned short bg;              /* cursor background color       */
    } cursor_color;                     /* cursor color                  */

    unsigned  short *cursor_data;

/**********************************************************************/
/*                                                                    */
/* Cursor Shape: width and height of the cursor shape                 */
/*                                                                    */
/*               NOTE: the default value is a DOUBLE underscore       */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

    struct cursr_shape {
	ushort		top;
	ushort		bot;
	ushort		blank;
	ushort		new;
	ulong	 	glyph[ CCM_CURSOR_AREA_MAX_IN_ULONG ];
    } cursor_shape ;

    long cursor_select;
    long cursor_show;


/*--------------------------------------------------------------------*/
/*                                                                    */
/* Cursor Position: character offset into the frame buffer            */
/*                  or index into the presentation space              */
/*                  ( ((row-1) * 80) + col-1 )                        */
/*                                                                    */
/*                  NOTE: the default value is 0 (the upper-left      */
/*                        corner of the screen)                       */
/*                                                                    */
/*--------------------------------------------------------------------*/

    struct {
	int                 col  ;
	int                 row  ;
    } last_cursor_pos ;

    struct {
	int                 col  ;
	int                 row  ;
    } cursor_pos ;

/*--------------------------------------------------------------------*/
/*                                                                    */
/* Screen position of top left of TEXT portion in pels.               */
/*                                                                    */
/*--------------------------------------------------------------------*/

     struct {
	  unsigned short pel_x;         /* pel indentation to tl of     */
	                                /* TEXT portion of screen.      */
	  unsigned short pel_y;         /* pel indentation to tl of     */
     } screen_pos;                      /* TEXT portion of screen.      */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Attribute:                                                        */
/*                                                                    */
/*       Contains the foreground color, background color,             */
/*       and the current font.                                        */
/*                                                                    */
/*--------------------------------------------------------------------*/

	unsigned long current_attr;

/*--------------------------------------------------------------------*/
/*                                                                    */
/*  Rendering Context Save Area:                                      */
/*                                                                    */
/*       Contains all information needed to save a rendering          */
/*       context when deactivating a monitor mode virtual terminal    */
/*       and to restore a rendering context when activating a monitor */
/*       mode virtual terminal.                                       */
/*                                                                    */
/*--------------------------------------------------------------------*/

	struct _rcx *virtual_rcx;


/*-------------------------------------------------------------------
|
|	Data that were added just for the CCM design
|
|-------------------------------------------------------------------*/

ulong		assume_hw_is_bad;	/* Boolean state of CDD hardware */

/*----------------------------------------------------------------------
|
|	Data that are copied from the CDD INIT routine
|
|-----------------------------------------------------------------------*/

ulong		cdd_max_width;
ulong		cdd_max_height;
ulong		cdd_max_area;
ulong		cdd_origin_type;


} ccm_ksr_data_t;      /* END of ccm_ksr_data structure */





		/*=================================
		| struct psf_type
		|================================*/



struct psf_type                             /* will be initialized */
{                                           /* to 0x20022002       */
  unsigned            ps_char1  : 8 ;       /* character code */
  unsigned            ps_attr1  : 8 ;       /* attribute code */
  unsigned            ps_char2  : 8 ;       /* character code */
  unsigned            ps_attr2  : 8 ;       /* attribute code */
} ;

extern struct psf_type *psf ;


/*----------------------------------------------------------------------|
|	Common Character Mode Box Dimension Structure			|
|									|
| For historical purposes, the box dimensions are given as the upper	|
| left corner , the width, and the height.				|
|-----------------------------------------------------------------------*/


		/*=================================
		| ccm_rectangle_t
		|================================*/


typedef struct 
{

	long	rulx, ruly;	/* rectangle origin, upper left, (x,y)	*/

	long	rwth, rht;	/* rectangle size (width, height)	*/

} ccm_rectangle_t;


/*======================================================================
|
|	DATA STRUCTURES FOR THE INTERRUPT HANDLER
|
|=======================================================================*/


/*----------------------------------------------------------
|
|	ccm_dd_intr_data_t
|
|-----------------------------------------------------------*/

typedef struct
{
	cdd_intr_head_t *	p_cdd_intr_head;
	cdd_intr_dev_attrs_t *	p_cdd_intr_dev_attrs;
	cdd_svcs_t *		p_cdd_intr_svcs;
	
	int			(* ccmdd_interrupt)( );
	int			(* cdd_interrupt)( );

} ccm_dd_intr_data_t;


/*-----------------------------------------------------------
|
|	ccm_pd_ddf_t
|
|-------------------------------------------------------------*/

typedef struct
{
	ccm_dd_intr_data_t *	p_dd_intr_data;

} ccm_pd_ddf_t;


/*==========================================================================
|
|	The CCM VDD also has a global structure that is defined within the
|	config module and exposed to all of the other VDD and VTT functions.
|	The structures below form the parts of that global data.  This is
|	an extension of the DDS concept, but it holds data that are not
|	really appropriate for storage in the DDS itself.
|
|===========================================================================*/


		/*=================================
		|  ccm_pathnames_t
		|=================================*/

typedef struct  
{

struct file 	* ucode_file_ptr;		/* ptr to the ucode file  */
char   		dev_ucodename[CCM_MAXNAMELEN];	/* full microcode file name   */
char   		ccm_ucodename[CCM_MAXNAMELEN];	/* ccm microcode file name   */
char   		device_driver[CCM_MAXNAMELEN];	/* full device driver name */
char   		device_pinned[CCM_MAXNAMELEN];	/* full device driver name */
char		ccm_dd_name[CCM_MAXNAMELEN];	/* full ccm driver name	*/
char		ccm_dd_pinned[CCM_MAXNAMELEN];	/* full ccm driver name	*/
char   		cdd_kmod_entry[CCM_MAXNAMELEN];	/* full cdd extenension name  */
char   		cdd_kmod_interrupt[CCM_MAXNAMELEN]; /* full cdd extenension name  */

} ccm_pathnames_t;


		/*=================================
		|  ccm_dd_data_t
		|=================================*/


typedef struct
{

cdd_header_t *		p_cdd_header;	 
cdd_svcs_t *		p_cdd_svcs;	
cdd_procs_t *		p_cdd_procs;
cdd_device_attrs_t *   	p_cdd_device_attrs; 
cdd_command_attrs_t *  	p_cdd_command_attrs;

ccm_pathnames_t *	p_ccm_paths;	

ccm_dds_t *		p_ccm_dds;

mid_t			kmid_entry;
mid_t			kmid_interrupt;
mid_t			kmid_ccmdd_pin;

ulong			has_intr_kmod;

ccm_ksr_data_t *	p_ccm_ksr_data;

ulong			vtt_fast_io;
ulong			vtt_busmem_att;
ulong			vtt_iocc_att;

} ccm_dd_data_t;


#endif		/* __H_CCM */

