/* @(#)50	1.5  src/bos/kernext/disp/ccm/inc/frs_60x.h, dispccm, bos411, 9428A410j 7/5/94 11:34:23 */

/*
 *
 * COMPONENT_NAME: (dispcfg) ROM Scan code for ROS IPL
 *
 * FUNCTIONS: RSCAN_60X_BOOL_INCREMENT_1
 *	      RSCAN_60X_BOOL_INCREMENT_2
 *	      RSCAN_60X_BOOL_USES_32_BIT_ROM
 *	      RSCAN_60X_BOOL_USES_8_BIT_ROM
 *	      RSCAN_60X_MAC_VPD_BYTE_COUNT
 *	      RSCAN_60X_MAC_VPD_BYTE_STRIDE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 *   ROM Header for Power PC ROM Scan adapter 
 */

#ifndef	_H_FRS_60X
#define	_H_FRS_60X

/*=========================================================================
|	The following public macros are defined in this file:
|	
|		the following return boolean values
|
|	RSCAN_60X_BOOL_USES_32_BIT_ROM( w )	
|	RSCAN_60X_BOOL_USES_8_BIT_ROM( w )
|	RSCAN_60X_BOOL_INCREMENT_1( w )	
|	RSCAN_60X_BOOL_INCREMENT_2( w )
|
|		the following are used in for() loops for VPD-only
|
|	RSCAN_60X_MAC_VPD_BYTE_COUNT( w )
|	RSCAN_60X_MAC_VPD_BYTE_STRIDE( w, i )
|	
|=========================================================================*/

/*========================================================================
|       The typedefs in this file are:
|
|	rom_pad_t			- used to show padding words
|
|	rscan_60x_devid_t		- defines the Device Characteristics Reg
|	rscan_60x_romchar_t		- defines the FRS Characteristics Reg
|	rscan_60x_frs_type_t		- defines the type of the FRS ROM 
|	
|	rscan_60x_std_config_regs_t	- defines the unpadded std config regs
|	rscan_60x_rom_header_t	- defines the generic FRS header
|	rscan_60x_vpd_rom_header_t	- defines FRS header for vpd-only romchar
|	rscan_60x_frs_rom_header_t	- defines FRS header for conforming romchar
|	
| NOTE: padding is assumed if 
|		w = contents of RSCAN_60X_REG_DEVCHAR
|		TRUE == RSCAN_60X_BOOL_INCREMENT_2( w )
| 	
|=========================================================================*/

/*=======================================================================
|
| CAUTION:
|	The typedefs and constants in this file are used by hardware
|	and by software development groups.  ROMs are built based on 
|	these structures that are included in planars and in adapters.
|	Changes to these structures should be made with the understanding
|	of the backward and forward compatibility issues associated with
|	all released hardware and future hardware offerings.
|
|	When using this file to form the contents of a ROM, be sure to
| 	define the proper setting of the RSCAN_60X_HW_* defintiions
|
|========================================================================*/


/*=======================================================================
|
|	The following constants are defined for use by all Feature ROM
|	Scan modules which interact with the Power PC Bus
|
|========================================================================*/

enum
{

	/*-------------------------------------------------
	| values that define the locations of system registers
	|--------------------------------------------------*/

	RSCAN_60X_REG_DEVCHAR		= (int) 0xff200000,
	RSCAN_60X_REG_ROMBASE		= (int) 0xffA00000,


	/*-------------------------------------------------
	| values that help compute the size of VPD when there
	| is only VPD in the ROM
	|-------------------------------------------------*/

	RSCAN_60X_SIZE_VPD_32		= (int) 248,
	RSCAN_60X_SIZE_VPD_8		= (int) 248 * 4,

	/*----------------------------------------------
	| flags that define the Device Characteristics Register
	|----------------------------------------------*/

	RSCAN_60X_DEVCHAR_TYPE_MASK		= (int) 0xf0000000,
	RSCAN_60X_DEVCHAR_TYPE_NOT_READY	= (int) 0,
	RSCAN_60X_DEVCHAR_TYPE_IO		= (int) 0x30000000,
	RSCAN_60X_DEVCHAR_TYPE_NOT_PRESENT	= (int) 0xf0000000,

	RSCAN_60X_BUID_ALLOC_MASK	= (int) 0x0c000000,
	RSCAN_60X_BUID_ALLOC_NONE	= (int) 0,
	RSCAN_60X_BUID_ALLOC_1		= (int) 0x04000000,
	RSCAN_60X_BUID_ALLOC_2		= (int) 0x08000000,
	RSCAN_60X_BUID_ALLOC_4		= (int) 0x0c000000,

	RSCAN_60X_MEM1_ALLOC_MASK	= (int) 0x00f00000,
	RSCAN_60X_MEM1_ALLOC_NONE	= (int) 0,
	RSCAN_60X_MEM1_ALLOC_256M	= (int) 0x00900000,
	RSCAN_60X_MEM1_ALLOC_512M	= (int) 0x00a00000,

	RSCAN_60X_MEM2_ALLOC_MASK	= (int) 0x000f0000,
	RSCAN_60X_MEM2_ALLOC_NONE	= (int) 0,
	RSCAN_60X_MEM2_ALLOC_256M	= (int) 0x00090000,
	RSCAN_60X_MEM2_ALLOC_512M	= (int) 0x000a0000,

	RSCAN_60X_ROM_INDICATOR_MASK	= (int) 0x00000060,
	RSCAN_60X_ROM_INDICATOR_NO_ROM	= (int) 0,
	RSCAN_60X_ROM_INDICATOR_BYTE	= (int) 0x00000040,
	RSCAN_60X_ROM_INDICATOR_WORD	= (int) 0x00000060,

	RSCAN_60X_WORD_INCREMENT_MASK	= (int) 0x0000001C,
	RSCAN_60X_WORD_INCREMENT_1	= (int) 0,
	RSCAN_60X_WORD_INCREMENT_2	= (int) 0x00000004,

	/*-------------------------------------------------
	| values that define the Device ID Register
	|--------------------------------------------------*/

	RSCAN_60X_DEVID_OEM_BIT		= (int) 0x80000000,
	RSCAN_60X_DEVID_SUB_OEM_BIT	= (int) 0x80,

	RSCAN_60X_DEVID_TYPE_MASK	= (int) 0x00ffff00,
	RSCAN_60X_DEVID_TYPE_GRAPHICS	= (int) 0x00004000,
	RSCAN_60X_DEVID_SUB_TYPE_GRAPHICS = (int) 0x0040,

	/*--------------------------------------------------
	| values that define bits in the ROM Characteristics Register
	|--------------------------------------------------*/

	RSCAN_60X_ROMCHAR_VPD_ONLY	= (int) 0,

	/*-------------------------------------------------
	| values that help compute the size of VPD when there
	| is only VPD in the ROM
	|-------------------------------------------------*/

	RSCAN_60X_VPD_BASE_SIZE		= (int) 248,
	RSCAN_60X_VPD_DUMMY		= (int) 1,

	/*------------------------------------------------
	| values that define generic types of ROM contents
	|------------------------------------------------*/

	RSCAN_60X_ROM_DEV_ONLY_BIT	= (int) 0x80000000,
	RSCAN_60X_ROM_DEVTYPE_ONLY_BIT	= (int) 0x40000000,
	RSCAN_60X_ROM_VPD		= (int) 0x01,
	RSCAN_60X_ROM_FRS		= (int) 0x02,


        __RSCAN_LAST_4                  = (int) 0
};


/*=====================================================================
| set up the VPD size and type and some other basic macros
| They are functions of the Device Feature/VPD ROM Indicator
| and of the Device Word Address Increment values of the
| 60X bus Device Characteristics Register 
|
| For VPD, if nobody has set a hardware value, then force
| the defines to use the small structure size versions
|======================================================================*/

	/*----------------------------------------------------
	| macros that return Boolean and that operate on the value
	| of the Device Characteristics Register
	|----------------------------------------------------*/

#define	RSCAN_60X_BOOL_USES_32_BIT_ROM( w )		\
	( ((w) & RSCAN_60X_ROM_INDICATOR_MASK) == RSCAN_60X_ROM_INDICATOR_WORD )

#define	RSCAN_60X_BOOL_USES_8_BIT_ROM( w )		\
	( ((w) & RSCAN_60X_ROM_INDICATOR_MASK) == RSCAN_60X_ROM_INDICATOR_BYTE )

#define	RSCAN_60X_BOOL_INCREMENT_1( w )			\
	( ((w) & RSCAN_60X_WORD_INCREMENT_MASK) == RSCAN_60X_WORD_INCREMENT_1 )

#define	RSCAN_60X_BOOL_INCREMENT_2( w )			\
	( ((w) & RSCAN_60X_WORD_INCREMENT_MASK) == RSCAN_60X_WORD_INCREMENT_2 )

	/*-----------------------------------------------------
	| macros that indicate size of loop to read VPD-only ROM
	| assuming uchar_t are read in the loop
	|
	| ROM_INDICATOR	 WORD_INCREMENT	 BYTE_COUNT	STRIDE
	|  =========	   ==========	   ======	=======
	|     32	      1		   248		each byte
	|     32	      2		 2*248		read 4, skip 4
	|      8	      1		 4*248		1 byte in 4
	|      8	      2		 8*248		1 byte in 8
	|------------------------------------------------------*/

#undef 	_A
#undef	_B
#undef	_C
#undef	_D
			/*---------------------------
			| define 4 boolean functions 
			| for the 4 cases of VPD-only ROM
			|---------------------------*/
#define	_A(w)								\
(	RSCAN_60X_BOOL_USES_BIT_ROM(w) && RSCAN_60X_BOOL_INCREMENT_1(w)	)

#define	_B(w)								\
(	RSCAN_60X_BOOL_USES_BIT_ROM(w) && RSCAN_60X_BOOL_INCREMENT_2(w)	)

#define	_C(w)								\
(	RSCAN_60X_BOOL_USES_8_BIT_ROM(w) && RSCAN_60X_BOOL_INCREMENT_1(w)	)

#define	_D(w)								\
(	RSCAN_60X_BOOL_USES_8_BIT_ROM(w) && RSCAN_60X_BOOL_INCREMENT_2(w)	)

			/*-------------------------------
			| define the byte count
			|-------------------------------*/

#define	RSCAN_60X_MAC_VPD_BYTE_COUNT( w )				\
(									\
	RSCAN_60X_VPD_BASE_SIZE * 					\
	_A(w) ? 1 : ( _B(w) ? 2 : ( _C(w) ? 4 : ( _D(w) ? 8 : 0 ) ) )	\
)

			/*------------------------------
			| define the byte stride
			|------------------------------*/

#define	RSCAN_60X_MAC_VPD_BYTE_STRIDE( w, i )				\
(									\
_A(w) ? 1 : (_C(w) ? 4 : (_D(w) ? 8 : (_B(w) ? ( (((i) % 4 ) == 3 ) ? 5 : 1 ) : 0 ) ) ) \
)


/*================================================================
| Set up some useful typedefs
|
|================================================================*/
	
	/*----------------------------------------------------
	| make a more explicit definition of pad in structures
	|-----------------------------------------------------*/

typedef	unsigned long	rom_pad_t;


/*===================================================================
|	Device ID Union Structure
|
|	Used to Decode the 32 bit device ID quantity
| 	Can either reference as the component pieces or as the
|	whole word
|
|==================================================================*/

typedef	union
{
	struct
	{
	unsigned char	flags;
			/*
			| holds flags that indicate whether the
			| device is OEM or IBM logo
			| as defined by RSCAN_60X_FLAG_OEM_DEVICE
			| all reserved bits should be set to 0
			|------------------------------------------*/

	unsigned short	type;
			/*
			| holds a descriptor field for different
			| types of devices.
			| The types are allocated by the Power PC systems
			| architects
			|------------------------------------------*/

	unsigned char	id;
			/*
			| unique ID value when associated with type
			| and with OEM flag.
			|-------------------------------------------*/
	} sub;

	unsigned long	word;
			/*
			| the full word representation of the device id
			|-------------------------------------------*/

} rscan_60x_devid_t;
	

/*==================================================================
|	Device ROM Types Union Structure
|
|	Used to Decode the 32 bit I/O Device Feature ROM Characteristics
|	word of the 60X Feature/VPD ROM
|
|===================================================================*/

typedef	union
{
	struct
	{
	unsigned short	reserved_0;
			/*
			| must be 0
			|---------------------------------------------*/

	unsigned char	reserved_1;
			/*
			| must be 0
			|---------------------------------------------*/

	unsigned char	num_items;
			/*
			| encodes the contents of the ROM that follows
			| 0 is special case for VPD-only
			| otherwise holds device-specific rom contents
			| as defined by PowerPC System ARch and unique 
			| device id and device type
			|----------------------------------------------*/

	} sub;

	unsigned long	word;

} rscan_60x_romchar_t;



/*===================================================================
|	Feature ROM Type Union Definition
|
|	Used to Decode the 32 bit Feature ROM Type Field
| 	Can either reference as the component pieces or as the
|	whole word
|
|==================================================================*/

typedef	union
{
	struct
	{
	unsigned char	flags;
			/*
			| holds flags that indicate whether the
			| device is OEM or IBM logo
			| as defined by RSCAN_60X_FLAG_OEM_DEVICE
			| all reserved bits should be set to 0
			|------------------------------------------*/

	unsigned char	reserved_0;
			/*
			| must be 0
			|------------------------------------------*/

	unsigned char	reserved_1;
			/*
			| must be 0
			|------------------------------------------*/

	unsigned char	id;
			/*
			| unique ID value when associated with type
			| and with OEM flag.
			|-------------------------------------------*/
	} sub;

	unsigned long	word;
			/*
			| the full word representation of the device id
			|-------------------------------------------*/

} rscan_60x_frs_type_t;
	

/*====================================================================
| define the template structure for the standard device registers
| one or the other structure will be used based on the results
| of the macro that tests the WORD INCREMENT
| 
| If RSCAN_60X_BOOL_INCREMENT_1( RSCAN_60X_REG_DEVCHAR ) == TRUE
| then use the 32 bit form.
| 
|====================================================================*/

	/*--------------------------------------------------
	| for word increment == 1 (32 bit version)
	|---------------------------------------------------*/

typedef	struct
{
	ulong_t			devchar_reg;
				/*
				| read only
				| The Device Characteristics Register
				|-------------------------------------------*/

	rscan_60x_devid_t	devid_reg;
				/* 
				| read only
				| The Device ID Register
				|-------------------------------------------*/
	
	ulong_t			buid_0_reg;
	ulong_t			buid_1_reg;
	ulong_t			buid_2_reg;
	ulong_t			buid_3_reg;
				/*
				| read/write
				| There are four possible registers in which the
				| system can defined BUC ID values for the device.
				| THe devchar_reg defines how many of these 
				| are actually used in the bits under the
				| RSCAN_60X_BUID_ALLOC_MASK
				|----------------------------------------------*/

	ulong_t			base_addr_0_reg;
	ulong_t			base_addr_1_reg;
				/*
				| read/write
				| There are two possible memory spaces that the
				| adapter can use.  The devchar_reg defines how
				| many and what size to use, under the two masks:
				| RSCAN_60X_MEM1_ALLOC_MASK
				| RSCAN_60X_MEM2_ALLOC_MASK
				|-----------------------------------------------*/

} rscan_60x_std_config_regs_t;


/*====================================================================
| 	32 bit versions of headers
|
|       PowerPC Feature/VPD Header
|       Required.  Defined by IBM AWSD, Austin, TX
|
|       Supplied by all types of adapters that support
|       all kinds of ROM SCAN on the Power PC 60X Bus
|
|       This structure coordinated
|       in System ROS Dept. D97
|
|	The form of this structure is for adapters that only decode
|	32 bits of the data bus.  In this form, the words of the 
|	adapter are double-word aligned and must be padded.
|
|====================================================================*/

	/*===============================================
	| rscan32_60x_rom_header_t
	|
	| generic structure for all devices and rom types
	| used until the device and characteristics can
	| be queried
	|===============================================*/

typedef struct
{
	rscan_60x_devid_t	dev_id_reg;
				/*
				| this is a 32 bit quantity that holds the 
				| unique device ID and some flags
				|---------------------------------------*/

	rscan_60x_romchar_t	romchar_reg;
				/*
				| this is a 32 bit quantity that holds the
				| characteristics field(s) of the ROM
				|---------------------------------------*/

} rscan_60x_rom_header_t;


	/*=============================================
	| rscan32_60x_vpd_rom_header_t
	|
	| if the characteristics.sub.num_items == 0, then
	| the rom holds VPD only, and this template
	| applies
	|
	|=============================================*/

typedef struct
{
	rscan_60x_devid_t	dev_id_reg;
				/*
				| this is a 32 bit quantity that holds the 
				| unique device ID and some flags
				|---------------------------------------*/

	rscan_60x_romchar_t	romchar_reg;
				/*
				| this is a 32 bit quantity that holds the
				| characteristics field(s) of the ROM
				|---------------------------------------*/

	uchar_t			vpd [ 1 ];
				/*
				| the VPD starts here, but its interpretation
				| is subject to the ROM width indicator
				| programmers are better off using pointers
				| than arrays when accessing VPD
				|--------------------------------------------*/
				
} rscan_60x_vpd_rom_header_t;


        /*=============================================
        | rscan32_60x_len_crc_t
        |
        | upper 16 bits of this word has the length of the
        | total ROM in 512 byte units.  The lower 16 bits
        | has the 16 bit crc value calcuated from the BASE +3i
        | to last byte of the ROM, inclusive.
        |
        | Note the BASE is 0xFFA0 0000 and i is either 4 or 8
        | depending on the word increment value in the Device
        | Characteristic Reg.
        |
        |=============================================*/

typedef union
{
        struct
        {
                ushort size;
                ushort crc;
        } sub;

        ulong word ;

} rscan_60x_len_crc_t;



	/*=============================================
	| rscan32_60x_frs_rom_header_t
	|
	| if the characteristics.sub.num_items != 0, then
	| the rom holds long VPD and Feature ROM Scan
	| 
	|=============================================*/

typedef struct
{
	rscan_60x_devid_t	devid_reg;
				/*
				| this is a 32 bit quantity that holds the 
				| unique device ID and some flags
				|---------------------------------------*/

	rscan_60x_romchar_t	romchar_reg;
				/*
				| this is a 32 bit quantity that holds the
				| characteristics field(s) of the ROM
				|---------------------------------------*/

        rscan_60x_len_crc_t     len_n_crc;
                                /*
                                | this is a 32 bit quantity that holds the
                                | size of total ROM in units of 512 bytes in
                                | the | upper 16 bits and a 16 bit crc in the
                                | lower 16 bits
                                |---------------------------------------*/

	struct
	{
	rscan_60x_frs_type_t type;
	ulong_t		     offset;
	ulong_t		     length;
	}
				rom[ 1 ];				
				/*
				| the array of offsets, lengths, and types starts
				| here.  The actual span of the array is stored
				| in the romchar_reg.  Use the actual value to
				| access the required data.  
				|
				| The offset is from RSCAN_60X_REG_ROMBASE and the
				| units are bytes.  The length units are bytes.
				|
				| The type flags define whether the ROM is
				| one of the standard forms, or whether it is
				| applicable to devices of that type, or to
				| just the specific device
				|----------------------------------------*/

} rscan_60x_frs_rom_header_t;


        /*=============================================
        | rscan32_60x_obj_info_t
        |
        | if the characteristics.sub.num_items != 0, then
        | the rom holds long VPD and Feature ROM Scan.  These
        | are the defined object so far.  Each object has
        | three words to describe its type and the location
        | of where it is in ROM.
        |
        | Note the 3 word object decription units begin at
        | BASE + 3i.
        |=============================================*/

typedef struct
{
        rscan_60x_frs_type_t type;
                                /*
                                | this is a 32 bit quantity that holds the
                                | object type.  The format for the word is
                                |
                                |   CDTrrrrr rrrrrrrr IIIIIIII IIIIIIII
                                |
                                | where
                                |   C bit says object common to all Feat/VPD
                                |         ROM I/O device
                                |   T bit says object is defined for any device
                                |         that has a device type of the same
                                |         value as this device
                                |   D  bit says object is only defined for
                                |         objects with the unique 32 bit device
                                |         ID value.
                                |   r  bits are reserved
                                |   I  bits are uniq block ID - VPD (0x1) and
                                |         Feature ROM (0x2)
                                |---------------------------------------*/
        ulong_t              offset;
                                /*
                                | this is a 32 bit quantity that holds the
                                | offset from the BASE to the object,
                                | accounting for the word increment.
                                |---------------------------------------*/
        ulong_t              length;
                                /*
                                | this is a 32 bit quantity that holds the
                                | length of the object in bytes.
                                |---------------------------------------*/

} rscan32_60x_obj_info_t;



#endif	/* _H_FRS_60X */
