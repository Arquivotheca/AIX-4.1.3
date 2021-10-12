/* @(#)13	1.8  src/bos/kernext/disp/ccm/inc/cdd.h, dispccm, bos411, 9428A410j 7/5/94 11:33:45 */

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

/******************************************************************************
 
 TITLE:  Character Mode Device Driver Standard Include Files

 FUNCTIONS:     (none)

 TYPEDEFS:	cdd_procs_t	
		cdd_svcs_t
		cdd_device_attrs_t
		cdd_command_attrs_t
		cdd_exts_t
		cdd_header_t
		cdd_vpd_t
		cdd_init_cmd_t
		cdd_init_out_t
		cdd_set_pos_cmd_t
		cdd_set_out_cmd_t
		cdd_qvpd_cmd_t
		cdd_qvpd_out_t
		cdd_blit_cmd_t
		cdd_load_ucode_cmd_t
		cdd_reset_hw_cmd_t
		cdd_set_mem_space_cmd_t

 PURPOSE: Defines the standard data structures used by all CDD routines.

          The CDD is a small set of routines that provide a basic BLT
          interface into the adapter.  The only code which uses these
          routines is the CCM VDD code and the CCM config method code.

 
 INPUTS:  n/a

 OUTPUTS: n/a

 DATA:    A set of typedefs for the CDD functions:

 *************************************************************************/

#ifndef __H_CDD
#define __H_CDD



/*=======================================================================
 |                                                                      |
 | cdd defined symbols and macros                                       |
 |                                                                      |
 | Note: Where possible, we will use ENUM rather than DEFINE to set	|
 | 	 the value of constants.  This is the preferred manner of 	|
 |	 coding and it certainly helps when debugging.			|
 |                                                                      |
 |======================================================================*/

enum
{
	/*-------------------------------------------------
	|  Symbols relating to the Micro Channel bus and
	|  to the segment register contents of the RS/6000
	|  architecture
	--------------------------------------------------*/

	CDD_BUS0			= (int) 0x02000000,
	CDD_BUSx			= (int) 0x02000000,
	CDD_BUS1			= (int) 0x02100000,
	CDD_BUS2			= (int) 0x02200000,
 	CDD_BUS3			= (int) 0x02300000,
 
	CDD_BUID_MASK			= (int) 0x0ff00000,
	CDD_BUS_MASK			= (int) 0x00f00000,
	CDD_BUS_OFFSET			= (int) 20,

	CDD_SEG_T_BIT			= (int) 0x80000000,
	CDD_IOCC_SELECT			= (int) 0x00000080,
	CDD_BYPASS_TCW			= (int) 0x00000020,
	CDD_ADDR_CHK			= (int) 0x00080000,
	CDD_ADDR_INCR			= (int) 0x00040000,
	CDD_ADDR_RTMODE			= (int) 0x00000040,
	CDD_BUS_IO_MASK			= (int) 0x0fff0000,
	CDD_SEG_REG_MASK		= (int) 0xf0000000,
	CDD_ADDR_MASK			= (int) 0x0fffffff,

	CDD_IOCC_0_BID			= (int) 0x820c00e0,
	CDD_IOCC_1_BID			= (int) 0x821c00e0,
	CDD_IOCC_2_BID			= (int) 0x822c00e0,
	CDD_IOCC_3_BID			= (int) 0x823c00e0,

	CDD_IOCC_BASE		    	= (int) 0x00400000,
	CDD_IOCC_TOP			= (int) 0x00500000,

	CDD_IOCC_SLOT_MASK		= (int) 0x000f0000,


	/*-------------------------------------------------
	|  Symbols relating to the 60x bus and
	|  to the segment register contents of the S/6000
	|  architecture
	--------------------------------------------------*/

	CDD_60x_32BIT_SET_T_BIT		= ( int ) 0x80000000,
	CDD_60x_32BIT_SUP_ST_KEY	= ( int ) 0x40000000,
	CDD_60x_32BIT_PROB_ST_KEY	= ( int ) 0x20000000,
	CDD_60x_MOD250_BUID		= ( int ) 0x07f00000,		/* Rainbow	*/
	CDD_60x_R2G_BUID		= ( int ) 0x01000000,		/* Rios 2G	*/
	CDD_60x_BUS_MEM_SPACE_BEGIN	= ( int ) 0x10000000,
	CDD_60x_BUS_MEM_SPACE_END	= ( int ) 0xefffffff,
	CDD_60x_CFG_SPACE_BEGIN		= ( int ) 0xff200000,
	CDD_60x_CFG_SPACE_END		= ( int ) 0xff2fffff,
	CDD_60x_VPD_FRS_SPACE_BEGIN	= ( int ) 0xffa00000,
	CDD_60x_VPD_FRS_SPACE_END	= ( int ) 0xffc00000,
	CDD_POWER_601_INT_SERVER	= ( int ) 0xff,
	CDD_POWER_RS2_INT_SERVER	= ( int ) 0x0,

	/* 
	 * Values for architecture field
 	 */

	CDD_POWER_RS			 = (int) 0x0001,	/* Power Classic architecture */
	CDD_POWER_PC			 = (int) 0x0002,	/* Power PC architecture */

	/* 
	 * Values for   implementation field
	 */

	CDD_POWER_RS1			 = (int) 0x0001,	/* RS1 class CPU */
	CDD_POWER_RSC			 = (int) 0x0002,	/* RSC class CPU */
	CDD_POWER_RS2			 = (int) 0x0004,	/* RS2 class CPU */
	CDD_POWER_601			 = (int) 0x0008,	/* 601 class CPU */
	CDD_POWER_603			 = (int) 0x0010,	/* 603 class CPU */
	CDD_POWER_604			 = (int) 0x0020,	/* 604 class CPU */


	/*-------------------------------------------------------
	| Symbols relating to the CDD structures and the use
	| of the CDD structures by the Video ROM modules
	|
	| Includes the command values for the various CDD commands
	| and the string values for various CDD strings
	|-------------------------------------------------------*/

	CDD_CURRENT_SW_VERSION		= (int) 0x100,

 	CDD_DDF_STANDARD_SIZE		= (int) 0x2000,	/* 8 K */

	CDD_MAXNAMELEN			= (int) 256,

		/*----------------------------------------
		| IPL ROS Interface to CDD 
		|----------------------------------------*/

	CDD_ROS_COMPONENT_LOC_SIZE	= (int) 5,
	CDD_ROS_PRODUCT_SRN_SIZE	= (int) 7,

	CDD_ROS_PRODUCT_NAME_SIZE	= (int) 41,


		/*----------------------------------------
		| CDD Services Structure and Functions 
		|----------------------------------------*/

	CDD_TO_BUS			= (int) 0,
	CDD_FROM_BUS			= (int) 1,

		/*----------------------------------------
		| CDD Function Return Codes
		| Found in p_cdd_command_attrs->rc
		|----------------------------------------*/

	E_CDD_PASS			= (int)  0,
	E_CDD_FAIL			= (int)  -1,
	E_CDD_IO_FAILED			= (int)  -2,
	E_CDD_DDS			= (int)  -3,
	E_CDD_HDR			= (int)  -4,
	E_CDD_CMD_ARGS			= (int)  -5,
	E_CDD_DEV_ARGS			= (int)  -6,
	E_CDD_PROCS			= (int)  -7,
	E_CDD_SVCS			= (int)  -8,
	E_CDD_EXTS			= (int)  -9,
	E_CDD_VERSION			= (int)  -10,
	E_CDD_BAD_CMD			= (int)  -11,
	E_CDD_IO_EXCEPTION		= (int)  -12,
	E_CDD_CMD_BLIT			= (int)  -13,
	E_CDD_CMD_LOAD_UCODE		= (int)	 -14,
	E_CDD_CMD_SET_POS		= (int)  -15,
	E_CDD_CMD_INIT			= (int)  -16,
	E_CDD_CMD_QVPD			= (int)  -17,
	E_CDD_CMD_SET_MEM_SPACE		= (int)  -18,
	E_CDD_BAD_UCODE_FILE		= (int)  -19,
	E_CDD_BAD_HW_OP			= (int)  -20,
	E_CDD_INTERNAL_SW_ERROR		= (int)  -21,
	E_CDD_CMD_INTERRUPT		= (int)  -22,
	E_CDD_INTR_HDR			= (int)  -23,
	E_CDD_INTR_DEV_ARGS		= (int)  -24,
	E_CDD_INTR_EXTS			= (int)  -25,
	E_CDD_INTR_VERSION		= (int)  -26,
	E_CDD_INTR_DDF			= (int)  -27,
	E_CDD_NO_MONITOR		= (int)  -28,

		/*----------------------------------------
		| CDD Init Command
		|----------------------------------------*/

	CDD_CMD_INIT			= (int) 0x3344,

	CDD_FLAGBIT_IPLROS		= (int) (1UL << 0 ),
	CDD_FLAGBIT_VIDEO_ROS		= (int) (1UL << 1 ),

			/*----------------------------------
			| NOTE: Flag Bit Values
			|	Bits 24-27 are reserved for the test
			|	mode defined by the IPL ROS
			|	These bits have IPL ROS meaning
			|	and are defined as follows
			------------------------------------*/

	CDD_FLAG_ROSTEST_MASK		= (int) ( 0xF << 4 ),

	CDD_FLAG_ROSTEST_NORMAL		= (int) ( 0x1 << 4 ),
	CDD_FLAG_ROSTEST_BLD_CFG	= (int) ( 0x2 << 4 ),
	CDD_FLAG_ROSTEST_RUN_IN		= (int) ( 0x3 << 4 ),
	CDD_FLAG_ROSTEST_BOX_FINAL	= (int) ( 0x4 << 4 ),
	/* 5 is not defined */
	CDD_FLAG_ROSTEST_ECAT_FUNC	= (int) ( 0x6 << 4 ),
	CDD_FLAG_ROSTEST_CDIAG		= (int) ( 0x7 << 4 ),
	CDD_FLAG_ROSTEST_ADIAG		= (int) ( 0x8 << 4 ),
	CDD_FLAG_ROSTEST_MODE_TEST	= (int) ( 0x9 << 4 ),

			/*----------------------------------
			| NOTE: Flag Bit Values
			|	Bits 20-23 are reserved for
			|	CCM to indicate version level
			|	These bits must *NOT* be used
			|	by ROS in any fashion
			------------------------------------*/
	CDD_FLAG_CCM_VERSION_MASK	= (int) ( 0xF << 8 ),
	CDD_FLAG_CCM_VERSION_1		= (int) ( 0x1 << 8 ),


		/*----------------------------------------
		| CDD Enable Command
		|----------------------------------------*/

	CDD_CMD_ENABLE			= (int) 0x33AA,

		/*----------------------------------------
		| CDD Enable Command
		|----------------------------------------*/

	CDD_CMD_DISABLE			= (int) 0x33BB,


		/*----------------------------------------
		| CDD Blit Command
		|----------------------------------------*/
 
	CDD_CMD_BLIT			= (int) 0x3355,

	CDD_ORIGIN_UL			= (int) 0x222,
	CDD_ORIGIN_LL			= (int) 0x333,

	CDD_BLIT_ALIGN_BYTE		= (int) 0x444,
	CDD_BLIT_ALIGN_WORD		= (int) 0x666,
 
		/*----------------------------------------
		| CDD Load Ucode Command
		|----------------------------------------*/

	CDD_CMD_LOAD_UCODE		= (int) 0x3366,
 
		/*----------------------------------------
		| CDD Reset HW Command
		|----------------------------------------*/

	CDD_CMD_RESET_HW		= (int) 0x3377,
 
		/*----------------------------------------
		| CDD Query VPD Command
		|----------------------------------------*/

	CDD_CMD_QVPD			= (int) 0x3388,

        CDD_VPD_MAX_LEN			= (int) 256,

        CDD_VPD_MAX_SLOTS		= (int) 6,

        CDD_VPD_VERSION_LENGTH		= (int) 33,

	CDD_MAX_NUM_MCA_CARDS 		= (int) 8,

		/*----------------------------------------
		| CDD Set POS Command
		|----------------------------------------*/

	CDD_CMD_SET_POS			= (int) 0x3399,

	CDD_MAX_BUSMEM_RANGES		= (int) 8,

	/*
	 * Extensions for 60x bus
	 */

	 CDD_MAX_60x_BUID_REGS		= 4,
	 CDD_MAX_60x_ADDRESS_RANGES		= 2,

	/*--- this is here just to prevent syntax errors of having
	      one too many commas in the list		---*/
	__CDD_LAST			= (int) 0 

};


/*=======================================================================
|	
| cdd_vpd_t 
|
|	The Common Character Mode VDD standardizes the manner in which
|	VPD is read from the diverse and (partially non-compliant)
|	Micro Channel display adapters of it might encounter.  It plans
|	for adapters with many cards, all of which have VPD.
|
|	It defines a VPD structure which holds together the various 
|	data objects associated with device VPD
|
|	This cdd_vpd_t structure is and must remain the same as the 
|	ccm_vpd_t structure in ccm,h.
|
|=========================================================================*/


typedef struct 
{

ulong		num_valid_units;

struct	{

	/*--------------------------------------------
	| the following fields are read by the device
	| dependent query_vpd routine of the CDD kmod
	| and are device dependent in their 
	| interpretation
	|--------------------------------------------*/

	size_t	buf_length;
	char	buffer[ CDD_VPD_MAX_LEN ];
	ulong	min_ucode_lvl;
	char	dev_version[ CDD_VPD_VERSION_LENGTH ];
	ulong	dev_max_ucode_lvl;
	ulong	dev_flags;
	ulong	dev_slot_id;
	ulong	dev_feature_id;

	}	per_unit_data[ CDD_VPD_MAX_SLOTS ];


} cdd_vpd_t;



/*-----------------------------------------------------------------------
 |                                                                      |
 | cdd_procs                                                            |
 |                                                                      |
 |      A data structure which defines the functions passed back by     |
 |      entry( ) routine.  The CCM VDD will invoke the functions        |
 |      through a pointer variable.  This provides for separation of    |
 |      the CDD name space and build environment from the CCM VDD.      |
 |									|
 |	NOTE:  Unless the svcs structure is pinned, the interrupt 	|
 |             entry point cannot be safely accessed by the interrupt	|
 |	       handler.							|
 |                                                                      |
 |----------------------------------------------------------------------*/


typedef struct 
    {

    int  	(*entry_point)( );	/* ptr to std entry point    */
    int  	(*interrupt)( );	/* ptr to interrupt handler */

    } cdd_procs_t;



/*----------------------------------------------------------------------
 *									|
 * cdd_svcs_t								|
 *									|
 *----------------------------------------------------------------------*/

typedef struct
    {

    int			(* lockl	)();
    void		(* unlockl	)();
    int			(* i_disable	)( );
    void		(* i_enable 	)( );
    int			(* strncmp	)( );

		/*-----------------------------------------------
		| the following are the basic I/O routines that
		| are taken directly from the kernel interfaces.
		| These routines are defined by <sys/ioacc.h>
		| and the source is located in $(TOP)/R2/sys/ml
		| in the file pioutil.s
		|------------------------------------------------*/

    int			(* bus_get_l	)( );
    int			(* bus_get_s	)( );
    int			(* bus_get_c	)( );
    int			(* bus_put_l	)( );
    int			(* bus_put_s	)( );
    int			(* bus_put_c	)( );
    int			(* bus_cpy	)( );

#ifdef DEBUG
	int		(*printf	)( );
#endif 

    } cdd_svcs_t;
     


/*-----------------------------------------------------------------------
 |                                                                      |
 | cdd_device_attrs                                                     |
 |                                                                      |
 |      These are data that are supplied to the CDD or that are used    |
 |      by the CDD.  This data is "global"; it is shared with all the   |
 |      CDD routines and with the CCM VDD.                              |
 |                                                                      |
 |----------------------------------------------------------------------*/


typedef struct
{


/* Pointer to the CDD device specific work scratchpad 
 * this data does not need to be pinned.  The interrupt 
 * handler will NEVER touch this data
 */
	void *			ddf_scratchpad;
	size_t			ddf_len;

/* Pointer to the CDD interrupt handler ddf scratchpad .  This
 * data is only used by the interrupt handler and by the other
 * cdd routines inside i_enable/i_disable brackets.  This data
 * must be pinned.  Only the interrupt handler may touch this
 * code.
 */

	void *			ddf_intr_scratchpad;
	size_t			ddf_intr_len;

/* These hold the contents of the segment register itself */
	ulong		busmem_att;
	ulong		iocc_att;

/* POS operations need the slot number of the adapter */
	ulong		slot;

/* exception code returned by the bus services	*/
	long		exception_code;

/* Base address and length of each address space on the adapter	*/

	ulong		busmem_hw_base[CDD_MAX_BUSMEM_RANGES];
	size_t		address_space_length[CDD_MAX_BUSMEM_RANGES];
	ulong		iocc_addr_base;


/* Extensions for 60x bus */

	ulong_t		buid_reg[ CDD_MAX_60x_BUID_REGS ];
	short		int_server;
	ulong		segment;

} cdd_device_attrs_t;        



/*-----------------------------------------------------------------------
 |                                                                      |
 | cdd_command_attrs                                                    |
 |                                                                      |
 |      This data is the generic command interface which is used when   |
 |      calls are made into a CDD.  This data holds the command and     |
 |      a pointer to command specific data.                             |
 |                                                                      |
 |----------------------------------------------------------------------*/


typedef struct
    {

/* holds command to CDD routine */

    ulong		command;		

/* holds pointer to and length of caller data */
/* this data is controlled by the cdd command structures */

    void *		pDataIn;
    void *		pDataOut;		
    ulong		length_in;
    ulong		length_out;

/* the next set are outputs from the CDD */

    int			rc_out;		

/* 
 * The following is the string which holds the character value associated
 * with a failing component on a card.  The contents of the string are
 * device specific.  The location is assumed to be single byte code page
 * ISO 8859 Latin-1 characters and the string is assumed to be null-terminated
 */

    char		component_loc[ CDD_ROS_COMPONENT_LOC_SIZE ];

/*
 * The following string holds the Service Reference Number of the adapter.
 * The SRN is assume to be single byte code page ISO 8859 Latin-1 characters.
 * It must be null-terminated.
 */

    char		product_SRN[ CDD_ROS_PRODUCT_SRN_SIZE ];


    } cdd_command_attrs_t;        



/*--------------------------------------------------------------------
 * cdd_exts                                                           
 *                                                                   
 *      A data structure which defines extensions to the data used   
 *      by the CCM VDD  
 --------------------------------------------------------------------*/ 


#ifndef _H_CCM_EXTENSIONS
#define _H_CCM_EXTENSIONS

typedef struct
    {
    char    	name[ CDD_MAXNAMELEN ];	  /* extension name               */
	
    struct cdd_exts	* p_NextExt;      /* next extension               */
    struct cdd_exts	* p_PrevExt;      /* previous extension           */
    void		* p_Attr;         /* extension attributes         */
    void		* p_Proc;         /* extension procedures         */

    } cdd_exts_t;

#endif



/*------------------------------------------------------------------------
 *                                                                      
 * cdd_head                                                             
 *                                                                      
 *      A data structure which defines the set of data passed by the    
 *      CDD into the CCM VDD.                                           
 *      The attributes and procedures are accessed via the header.      
 *                                                                      
 ------------------------------------------------------------------------*/


typedef struct 
{

    cdd_command_attrs_t	*	p_cdd_command_attrs;
    cdd_device_attrs_t	*	p_cdd_device_attrs;
    cdd_procs_t		*	p_cdd_procs;      
    cdd_svcs_t 		*	p_cdd_svcs;
    cdd_exts_t		*	p_cdd_extensions; 
    ulong	     		version;
    void 		*	p_iplrom_vrs_scratchpad;

} cdd_header_t;   



/*==========================================================================
|| 
||	Command Specific Structures for CDD Commands
||
||=========================================================================*/

	/*----------------------------------------------------------
	| init
	|
	|----------------------------------------------------------*/  

typedef	struct
{
	ulong	cdd_flags;		/* various states permitted at init */

/* Processor & architecture extension for all busses */
/* These fields are obtained from the iplcb by the config method */

	int	architecture;		/* POWER_RS or POWER_PC	*/
	int	implementation;		/* POWER_[RS1 | RS2 | RSC | 601] */

} cdd_init_cmd_t;


typedef struct
{
	/*
	 * These parameters are related to the maximum working space 
	 * available in the cdd blit routine to translate the blit 
	 * input data from the supplied format to that necessary by the adapter.
	 * The smallest max_width permitted by CDD is 32!!
	 * The smallest max_height permitted by CDD is 32 !!
	 * The smallest max_area permitted by CDD is 32 * 32
	 *
	 */

	ulong	max_width;
	ulong	max_height;
	ulong	max_area;
	ulong	origin_type;

	/* 
	 * The following parameters are returned to the calling ROS routine as
	 * suggestions to the parameters to be used in the assignment of the
	 * bus address, and spaces for the adapter to function.
	 */

	ulong		busmem_att;
	ulong		iocc_att;
	ulong		busmem_hw_base[CDD_MAX_BUSMEM_RANGES];
	ulong		address_space_length[CDD_MAX_BUSMEM_RANGES];
	ulong		iocc_addr_base;

	/*
	 * the following are hints related to the interrupt
	 * handler characteristics of the device
	 */

	uchar		has_intr_handler;	/* TRUE or FALSE */
	uchar		can_run_in_polled_mode;	/* TRUE or FALSE */

	ulong		dflt_int_level;		/* range [0..15]	*/

	int		dflt_int_priority;	/* defined in <sys/m_intr.h>	*/

	ulong		dflt_dma_arb_level;	/* range [0..7]			  */
						/* DMA not used, but this parm is */
						/* req'd to init some adapters	  */
	

	/*
	 * The following string holds the product name of the adapter.
	 * The product name is assume to be a single byte code page ISO 8859 Latin-1
	 * It must be null-terminated.
	 */

	char		product_name[ CDD_ROS_PRODUCT_NAME_SIZE ];

	/*
	 * The following are extensions for the 60x bus architecture
	 */

	ulong_t		buid_reg[ CDD_MAX_60x_BUID_REGS ];
	ulong_t		segment;
	short		int_server;

} cdd_init_out_t;




	/*----------------------------------------------------------
	| Enable
	|
	| NOTE: 
	|       Enable does not have a command structure
	|       Enable does not have an output structure
	|
	|-----------------------------------------------------------*/


	/*----------------------------------------------------------
	| Disable
	|
	| NOTE: 
	|       Disable does not have a command structure
	|       Disable does not have an output structure
	|
	|-----------------------------------------------------------*/


	/*----------------------------------------------------------
	| set_POS
	|
	|-----------------------------------------------------------*/

typedef struct
{
	uchar		use_polled_mode;
	ulong		int_level;
	ulong		dma_arb_level;
	ulong		cdd_flags;	/* various states permitted at init */
	ulong		int_priority;
	ulong		int_server;

} cdd_set_pos_cmd_t;

		/*!_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!
		| Defect 61512 moved this structure to load microcde out
		|
		typedef struct
		{
			ulong		x_min, x_max;
			ulong		y_min, y_max;
		
		} cdd_set_pos_out_t;
		|
		|_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!_!*/


	/*----------------------------------------------------------
	| load microcode
	|
	|----------------------------------------------------------*/  


typedef struct 
{
	size_t	length_of_ucode;
	char *	p_ucode;
	ulong	use_R2_rom;	/* Boolean: TRUE or FALSE */
	ulong	cdd_flags;	/* various states permitted at init */

} cdd_load_ucode_cmd_t; 

typedef struct
{
	ulong		x_min, x_max;
	ulong		y_min, y_max;

} cdd_load_ucode_out_t;



	/*----------------------------------------------------------
	| qvpd
	|----------------------------------------------------------*/  


typedef struct 
{
	uchar		start_slot;
	uchar		use_polled_mode;	/* TRUE or FALSE */

} cdd_qvpd_cmd_t; 

typedef struct
{	
	cdd_vpd_t *		p_cdd_vpd;	/* standard vpd structure */

} cdd_qvpd_out_t;

	/*----------------------------------------------------------
	| blit
	|
	| NOTE: blit does not have an output data structure
	|
	|----------------------------------------------------------*/  


typedef struct 
{
	ulong *		p_PixelData;	/* Pixel data to output to screen */
	ushort		PixelDataLength;/* Length of pixel data in bytes */
	ulong		PixelFormat;	/* packing format of pixels	*/
	short		dest_x;		/* Destination x address */
	short		dest_y;		/* Destination y address */
	ushort		width;		/* Width in pixels */
	ushort		height;		/* Height in pixels  */

} cdd_blit_cmd_t; 



	/*----------------------------------------------------------
	| reset_hw
	|
	| NOTE: reset_hw does not have an output data structure
	|
	|----------------------------------------------------------*/  


typedef struct 
{
	ulong	length_of_ucode;
	char *	p_ucode;


} cdd_reset_hw_cmd_t; 



#endif   /* of test for __H_CDD   */






