/* @(#)81	1.1  src/htx/usr/lpp/htx/lib/hga/Bt485.h, tu_hga, htx410 6/2/94 11:37:15  */
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: Bt485_register_read
 *		Bt485_register_write
 *		Bt485_select_64X64X2_cursor
 *		Bt485_set_color
 *		Bt485_set_up_2MSB_BITS
 *		DECL_BT485_PALETTE
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _BT485_H
#define _BT485_H


#define vu_long   volatile unsigned long
#define vu_char   volatile unsigned char

typedef struct _Bt485RegPtrs {
    int ram_w_addr;
    int ram_r_addr;
    int palette;
    int cur_ram_array;
    int cmd_reg_0;
    int cmd_reg_1;
    int cmd_reg_2;
    int cmd_reg_3;
    int status_reg;
    int cur_color_w_addr;
    int cur_color_r_addr;
    int cur_color_data;
    int cur_x_low_reg;
    int cur_x_high_reg;
    int cur_y_low_reg;
    int cur_y_high_reg;
} Bt485_regs;

Bt485_regs	Bt485;

/********************************************************************
*  Brooktree 485 Register Offsets
********************************************************************/

/*
 *  ---------------------------------------------------------------------------
 *	The two "hi" bits de-note the RS (register select) RS3-2.  Which 
 *	register bank do you want.
 *
 *	The two "lo" bits select the exact register in the specified
 *	bank you want to write, RS 1-0.
 * note:
 *	- addresses 0x000003c[6-9] are all located inside the dac.
 *		- when 3c7 is referenced with a read it acts as the 
 *		  DAC status register, the DAC status register is physically
 *		  located in the 928.
 *	- 0x0#000000 the "#" is the register bank select, it is used to denote 
 *	which register bank is to be used.
 *
 *	What writing to the video DAC registers does:
 *		- 0x?00003C6 -	causes a 10b to be placed on SD[9:8] (System
 *				Data pins) which drive RS[1:0] (Register Select)
 *				so hence, 3C6 will be used for the following 
 *				registers:
 *					- BT485_PIXEL_MASK,
 *					- BT485_CMD_REG_0,
 *					- BT485_CMD_REG_3,
 *					- BT485_STATUS_REG,
 *					- BT485_CUR_Y_LOW_REG.
 *	
 *  ---------------------------------------------------------------------------
 */	

/* RS:0 */						     /* hi lo */
#define BT485_RAM_W_ADDR	0x000003c8		     /* 00 00 */
#define BT485_PALETTE_DATA	0x000003c9		     /* 00 01 */
#define BT485_PIXEL_MASK	0x000003c6		     /* 00 10 */
#define BT485_RAM_R_ADDR	0x000003c7		     /* 00 11 */

/* RS:1 */
#define BT485_CUR_COLOR_W_ADDR	0x010003c8		     /* 01 00 */
#define BT485_CUR_COLOR_DATA  	0x010003c9		     /* 01 01 */
#define BT485_CMD_REG_0 	0x010003c6		     /* 01 10 */
#define BT485_CUR_COLOR_R_ADDR	0x010003c7		     /* 01 11 */

/* RS:2 */
#define BT485_CMD_REG_1 	0x020003c8		     /* 10 00 */
#define BT485_CMD_REG_2 	0x020003c9		     /* 10 01 */
#define BT485_CMD_REG_3 	0x020003c6		     /* 10 10 */
#define BT485_STATUS_REG	0x020003c6		     /* 10 10 */
#define BT485_CUR_RAM_ARRAY	0x020003c7		     /* 10 10 */

/* RS:3 */
#define BT485_CUR_X_LOW_REG 	0x030003c8		     /* 11 00 */
#define BT485_CUR_X_HIGH_REG 	0x030003c9		     /* 11 01 */
#define BT485_CUR_Y_LOW_REG 	0x030003c6		     /* 11 10 */
#define BT485_CUR_Y_HIGH_REG 	0x030003c7		     /* 11 11 */


/********************************************************************
*  Brooktree 485 Register Bits 
********************************************************************/

#define Bt485_X_CURSOR_ON		0x03 /* Command register 2 RS=1001 */
#define Bt485_ALL_CURSORS_OFF           0x00 /* Command register 2 RS=1001 */

#define Bt485_ENABLE_REG3		0x80 /* Command register 0 RS=0110 */
#define Bt485_SEL_64X64X2_CURSOR	0x04 /* Command register 3 RS=1010, when bit CR07 in command register 0 is set to logical 1 */

#define Bt485_CUR_COLOR_1		0x01
#define Bt485_CUR_COLOR_2		0x02
#define Bt485_CUR_COLOR_3		0x03

#define Bt485_CUR_RAM_ARRAY_0		0x000
#define Bt485_2MSB_BIT_INIT_VAL		0xFC

/********************************************************************
*  Brooktree 485 Miscellaneous Definitions
********************************************************************/
#define DECL_BT485_PALETTE(a)		vu_long *(a) = Bt485.palette

/*
 * ---------------------------------------------------------------------------
 *	macro: Bt485_register_write/Bt485_register_read
 *	description: Writes to the Extended DAC control registers on the
 *		     Bt485.
 *
 *	note: this is a consolidation of two equivalent fairway macros, where 
 *	      fairway had "FW_set_ramdac_addr" and "FW_register_write" 
 *	      homestake will have only Bt485_register_write.
 * ---------------------------------------------------------------------------
 */
#define Bt485_register_write(register, data)				\
{									\
    uchar Bt485_register_write_tmp;					\
/*									\
 * Select and clear the DAC register select bits. XXX:SM - optimize	\
 */									\
    S3ReadDP  (S3ExtDACControl, Bt485_register_write_tmp);		\
    Bt485_register_write_tmp &= 0xFC;					\
    S3WriteDP (S3ExtDACControl, Bt485_register_write_tmp);		\
									\
/* 									\
 * We have to select which bank we will be writing into, this is done 	\
 * by setting RS3 and RS2 (the left two most bits)			\
 */									\
    S3WriteData ((volatile unsigned char)(Bt485.register >> 24));	\
									\
/*									\
 * (register & 0xf0ffffff) clears the encoded register bank select 	\
 * bits.								\
 */									\
    S3WriteRegister ((Bt485.register & 0xf0ffffff), data);		\
}

#define Bt485_register_read(register, data)				\
{									\
    uchar Bt485_register_read_tmp;					\
    S3ReadDP  (S3ExtDACControl, Bt485_register_read_tmp);		\
    Bt485_register_read_tmp &= 0xFC;					\
    S3WriteDP (S3ExtDACControl, Bt485_register_read_tmp);		\
    S3WriteData ((Bt485.register >> 24));				\
    S3ReadRegister ((Bt485.register & 0xf0ffffff), data);		\
}


#define Bt485_set_color(red,green,blue)					\
{									\
    S3WriteRegister ((Bt485.palette & 0xf0ffffff), red);		\
    S3WriteRegister ((Bt485.palette & 0xf0ffffff), green);		\
    S3WriteRegister ((Bt485.palette & 0xf0ffffff), blue);		\
}

/********************************************************************
*   See Bt485 manual for detail of how to access Command Register 3 
*   on Bt485.
********************************************************************/

#define Bt485_select_64X64X2_cursor()					\
{									\
    uchar __tmp;							\
									\
    Bt485_register_read (cmd_reg_0, __tmp);				\
    __tmp |= Bt485_ENABLE_REG3;						\
    Bt485_register_write (cmd_reg_0, __tmp);				\
									\
    Bt485_register_write (ram_w_addr, 0x01);				\
									\
    Bt485_register_read (cmd_reg_3, __tmp);				\
    __tmp |= Bt485_SEL_64X64X2_CURSOR;					\
    Bt485_register_write (cmd_reg_3, __tmp);				\
}
#define Bt485_set_up_2MSB_BITS(n)              				\
{									\
    uchar __tmp;							\
									\
    Bt485_register_read  (cmd_reg_0, __tmp);				\
    Bt485_register_write (cmd_reg_0, __tmp | Bt485_ENABLE_REG3);	\
    EIEIO;								\
									\
    Bt485_register_write (ram_w_addr, 0x01);				\
    Bt485_register_read  (cmd_reg_3, __tmp);				\
    __tmp &= Bt485_2MSB_BIT_INIT_VAL;					\
    __tmp |= ((~Bt485_2MSB_BIT_INIT_VAL) & (n));			\
    Bt485_register_write (cmd_reg_3, __tmp);				\
    EIEIO;								\
}

#endif /* _BT485_H */
