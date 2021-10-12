static char sccsid[] = "@(#)74	1.1  src/htx/usr/lpp/htx/lib/hga/bt485dac.c, tu_hga, htx410 6/2/94 11:36:58";
/*
 *   COMPONENT_NAME: tu_hga
 *
 *   FUNCTIONS: Bt485_init
 *		RegWrapTestle
 *		s3_BT485_RegWrapTest
 *		s3_test_bt485
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


#include <stdio.h>
#include <sys/types.h>
#include "tu_type.h"

#include "hga_extern.h"
#include "s3regs.h"
#include "Bt485.h"

#define	DAC_MASK	0x3C6
#define FAIL6 	1

int s3_test_bt485(void) {
int             i,j,k;
int tmp;
unsigned char   reg_data;

    if (RegWrapTestle(BT485_PIXEL_MASK, 0,7,0xff,8))
       return 1;
    if (RegWrapTestle(BT485_RAM_W_ADDR,0,7,0xff,8))
       return 1;

/*    if (s3_BT485_RegWrapTest(BT485_CUR_X_LOW_REG,0,7,0xff)) 
       return FAIL6;

    if (s3_BT485_RegWrapTest(BT485_CUR_Y_LOW_REG,0,7,0xff)) 
       return FAIL6;

    if (s3_BT485_RegWrapTest(BT485_CUR_X_HIGH_REG,0,7,0xff)) 
       return FAIL6;
    if (s3_BT485_RegWrapTest(BT485_CUR_Y_HIGH_REG,0,3,0x0f)) 
       return FAIL6; */

   /* Test Brooktree palette */
   /* Wrap ones to all 8 bits for each of the 256 palettes */

   for (i = 0; i < 6; i++) {
      /* Set start address to point at palette RAM in brooktree chip offset 0 */
/*      S3WriteRegister(S3DACWriteIndexRegister, 0x00); */

      /* For loop is * 3 there is 1 write/read each for 
	 red, green, and blue.*/
      for (j = 0; j < (256); j++){
      /* Set start address to point at palette RAM in brooktree chip offset 0 */
        S3WriteRegister(S3DACWriteIndexRegister, j);
	for(k=0;k<3;k++)
	{
         reg_data = 1 << i;
      	 S3WriteRegister(S3DACDataRegister, reg_data);
	} 

      S3WriteRegister(0x3C7, j);
      for(k=0;k<3;k++)
      {
	 S3ReadRegister(S3DACDataRegister, reg_data);
         if (reg_data != (1 << i))
	 {
printf("\ntest failed reg_data = %x, j=%d, k = %d, i = %d",reg_data,j,k,i);
           return FAIL6;
	 } 

      }

      }

   }
   printf("next");

   /* Test Brooktree Cursor and Overscan Color palette */
   /* Wrap ones to all 8 bits for each of the palettes total = 12 */
   for (i = 0; i < 8; i++) {

      /* Set starting address point */
      Bt485_register_write(cur_color_w_addr, 0x00);
      for (j = 0; j < 4 * 3; j++)
	  Bt485_register_write(cur_color_data, 1 << i);

      /* reset starting address point */
      Bt485_register_write(cur_color_w_addr, 0x00);
      for (j = 0; j < 256 * 3; j++) {
	  Bt485_register_read(cur_color_data, reg_data);
printf("\nreg_data 2 = %x",reg_data);
          if (reg_data != (1 << i))
             return FAIL6;
      }
   }

   /* Test Brooktree cursor RAM */
   /* Wrap ones to all 8 bits for each of the 64 bytes of cusor ram */
   for (i = 0; i < 8; i++) {
      /* Set starting address point in brooktree chip */
      Bt485_register_write(cur_color_w_addr, 0x00);

      /* For loop is * 32 * 2 at 4 bytes a row */
      for (j = 0; j < 32*2 ; j++){
         for (k=0;k<4 ;k++ )
	    Bt485_register_write(cur_ram_array, 1 << i);
      }

      /* Set starting address point in brooktree chip */
      Bt485_register_write(cur_color_w_addr, 0x00);

      /* For loop is * 3 as there is 1 write/read each for */
      for (j = 0; j < 32*2; j++) {
         for (k=0; k<4 ;k++ ) {
	    Bt485_register_read(cur_ram_array, reg_data);
printf("\nreg_data 3 = %x",reg_data);
            if (reg_data != (1 << i))
              return FAIL6;
         }
      }
   }

   s3_load_palette( NULL );
   return 0;
}

/*---------------------------------------------------------------------------*/
/* SUBROUTINE NAME: s3_BT485RegWrapTest                                      */
/*                                                                           */
/* DESCRIPTION: This routine rolls a single bit through a Brooktree DAC      */
/*              register.                                                    */
/*                                                                           */
/*                                                                           */
/* CALLING EXAMPLE:  x = s3_BT485_RegWrapTest(bt485_cursor_x_high, 0, 3,0x0f)*/
/*                                                                           */
/* INPUT:    unsigned long reg                                               */
/*           int           start                                             */
/*           int           end                                               */
/*           unsigned char regmask                                           */
/*                                                                           */
/* OUTPUT:   returns a 0 for success                                         */
/*           returns a 1 if register test fails                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int   s3_BT485_RegWrapTest(unsigned long reg,
                                  int           start,
                                  int           end,
                                  unsigned char regmask)
{
int             i;
unsigned char   out_data;
unsigned char   reg_data;
uchar Bt485_register_write_tmp, Bt485_register_read_tmp;

      for (i = start; i <= end; i++){
        out_data = (1 << i) & regmask;
	/* Only write ones to bits that are testable */
        if (out_data) {
		/* Select and clear the DAC register select bits. */
		S3ReadDP  (S3ExtDACControl, Bt485_register_write_tmp);
		Bt485_register_write_tmp &= 0xFC;
		S3WriteDP (S3ExtDACControl, Bt485_register_write_tmp);
		/* We have to select which bank we will be writing into, this 
		   is done by setting RS3 and RS2 (the left two most bits)
		*/
	 	S3WriteData ((volatile unsigned char)(reg >> 24));
		/* (register & 0xf0ffffff) clears the encoded register bank 
		   select bits.
		*/
	 	S3WriteRegister ((reg & 0xf0ffffff), out_data);

	/* Now read the data back in */
		S3ReadDP  (S3ExtDACControl, Bt485_register_read_tmp);
		Bt485_register_read_tmp &= 0xFC;
		S3WriteDP (S3ExtDACControl, Bt485_register_read_tmp);
		S3WriteData ((volatile unsigned char)(reg >> 24));
		S3ReadRegister ((reg & 0xf0ffffff), reg_data);	

          	if ((reg_data & regmask) != out_data)
		{
printf("out_data = %x,reg_data = %x, regmask = %x,i = %d",out_data,reg_data,regmask,i);
			
           		return 1;
		}
        }
      }
      return 0;
}

int   
RegWrapTestle(ulong reg, int start, int end, ulong regmask, int regsize)
{

      int   i,j;
      char  rdata;
      ulong reg_data;

      for (i = end; i >= start; i--){
         switch(regsize){
         case 8:
            S3WriteRegister(reg, 1 << (7 - i));
	    S3ReadRegister(reg, rdata);
            reg_data = (ulong) rdata;
            if ( ( (uchar) reg_data & (uchar) regmask) != (1 << (7 - i)) )
              return(-1);
            break;
         case 16:
#if 0
            S3WriteRegisterSwappedShort(reg, 1 << (15 - i));
            reg_data = in16le(reg);
            if ( ( (short) reg_data & (short) regmask) != (1 << (15 - i)) )
              return(FAIL);
#endif
            break;
         case 32:
#if 0
            out32le(reg, 1 << (31 - i));
            reg_data = in32le(reg);
            if ( ( reg_data         &         regmask) != (1 << (31 - i)) )
              return(FAIL);
#endif
            break;
         default:
            break;
         } /* end switch */
      }
      return(0);
}  /* RegWrapTest */

Bt485_init()
{
/* 
 *  ---------------------------------------------------------------------------
 *	Initialize addresss for Bt485
 *  ---------------------------------------------------------------------------
 */
    Bt485.ram_w_addr      = BT485_RAM_W_ADDR;
    Bt485.ram_r_addr      = BT485_RAM_R_ADDR;
    Bt485.palette         = BT485_PALETTE_DATA;
    Bt485.cur_ram_array   = BT485_CUR_RAM_ARRAY;
    Bt485.cmd_reg_0       = BT485_CMD_REG_0;
    Bt485.cmd_reg_1       = BT485_CMD_REG_1;
    Bt485.cmd_reg_2       = BT485_CMD_REG_2;
    Bt485.cmd_reg_3       = BT485_CMD_REG_3;
    Bt485.status_reg      = BT485_STATUS_REG;
    Bt485.cur_color_w_addr  = BT485_CUR_COLOR_W_ADDR;
    Bt485.cur_color_r_addr  = BT485_CUR_COLOR_R_ADDR;
    Bt485.cur_color_data  = BT485_CUR_COLOR_DATA;
    Bt485.cur_x_low_reg   = BT485_CUR_X_LOW_REG;
    Bt485.cur_x_high_reg  = BT485_CUR_X_HIGH_REG;
    Bt485.cur_y_low_reg   = BT485_CUR_Y_LOW_REG;
    Bt485.cur_y_high_reg  = BT485_CUR_Y_HIGH_REG;

#if 0
    Bt485_select_64X64X2_cursor();
    S3WriteDP (S3ExtDACControl, 0x38); /* set bits 3,4 and 5. */
    S3WriteDP (S3CursorMode, 0x21);
#endif
}
