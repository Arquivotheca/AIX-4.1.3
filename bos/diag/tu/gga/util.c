static char sccsid[] = "@(#)93	1.1  src/bos/diag/tu/gga/util.c, tu_gla, bos41J, 9515A_all 4/6/95 09:27:35";
/*
 *   COMPONENT_NAME: tu_gla
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/devinfo.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <sys/types.h>
#include <sys/termio.h>
#include <sys/ioctl.h>

#include "exectu.h"
#include "tu_type.h"
#include "ggamisc.h"
#include "ggapci.h"

extern unsigned int gga_x_max;
extern unsigned int gga_y_max;
extern unsigned int gga_mode;

/* Local prototypes */
void  out32le(ulong, ulong);
void  out32(ulong, ulong);
void  out8(ulong, uchar);
void  BR_WL(ulong, ulong);
void  WL(ulong, ulong);
void  WC(uchar, ulong);
ulong in32le(ulong);
ulong in32(ulong);
uchar in8(ulong);
ulong BR_RL(ulong);
ulong RL(ulong);
ulong RL_NO_WAIT(ulong);
uchar RC(ulong);


/*-------------------------------------------------------------------
  W r i t e _ R e g _ B i t f i e l d ( )
  ---------------------------------------------------------------------
  MODULE NAME:   Write_Reg_Bitfield()
  WRITTEN BY:    Bob Montague, 10-15-90, created for Salmon
  MODIFIED BY:   Ashley Cambpell, 5-19-92, for use in GARDIAN

  REVIEWS:

  REVISIONS:
  Changed routine to allow access to any register, not just the
  SGA control register.

  PURPOSE:
  Set a bitfield in the SGA control register without altering any
  other bitfield value.

  ASSUMPTIONS:
  All parameters have legal values.
  field_position is the value of the most significant bit of the
  bitfield using IBM's backward numbering notation.

  ALGORITHM:
  First read the current value of the control register.
  Then create a mask such that the mask contains 0's in the bit
  positions corresponding to the bitfield to be changed and 1's in
  all other bit positions.
  Create a word of all 1's.
  Shift the 1's left by the bit length of the bitfield.
  Invert this result so that the mask contains bitfield length 1's
  in the lowest order bits.
  Shift the 1's left to align with the bitfield position.
  The mask now contains 1's in the bitfield location and 0's
  everywhere else.
  Invert the mask to produce a mask of all 0's aligned to the
  bitfield and all 1's everywhere else.
  Logically AND the mask with the control register value, which will
  clear the control register bitfield to be modified.
  Take the new value of the bitfield and shift it to align with the
  bitfield location.
  Logiaclly OR this value with the control register value which will
  put the new bitfield value in place of the cleared out value.
  Write this new value back to the control register.

  GLOBALS USED:

  OTHER:
  See SGA Hardware Reference for explanation of the SGA control
  register bitfields.
  -------------------------------------------------------------------*/

void Write_Reg_Bitfield(unsigned long address,
                        unsigned long value,
                        unsigned long field_position,
                        unsigned long field_length)
{
    unsigned cntl_value, /* value of Control Register */
    mask, /* used to mask bits not in bit field */
    field; /* bit field value shifted to field position */
    unsigned long base;

    cntl_value = RL( address );

    /* -------------------------------------------------------------- */
    /* Clear bit field to be set (allows new value to be ANDed in)    */
    /* -------------------------------------------------------------- */

    /* first put field_length '1's in low order bits                  */
    mask = ~( 0xFFFFFFFF << field_length);

    /* next shift '1's to align with field position and convert to 0's*/
    mask = ~( mask << ( 32 - field_length - field_position));

    /* finally, clear bit field                                       */
    cntl_value = cntl_value & mask;

    /* -------------------------------------------------------------- */
    /* set bit field in control register                              */
    /* -------------------------------------------------------------- */

    /* first align value with bit field location                      */
    field = ( value << ( 32 - field_length - field_position));

    /* then OR in the value and write to the selected Register         */
    cntl_value = cntl_value | field;
    WL( cntl_value, address);

} /* end Write_Reg_Bitfield */



/*-------------------------------------------------------------------
  R e a d _ R e g _ B i t f i e l d ( )
  ---------------------------------------------------------------------
  MODULE NAME:   Read_Reg_Bitfield()
  WRITTEN BY:    Bob Montague, 10-15-90, created for Salmon
  MODIFIED BY:   Ashley Campbell, 5-19-92, for use in GARDIAN

  REVIEWS:

  REVISIONS:
  Changed routine to allow access to any register, not just the
  SGA control register.

  PURPOSE:
  Return the value of a given bitfield from the SGA control register.

  ASSUMPTIONS:
  field and length parameters are legal values.
  field is the value of the most significant bit of the bitfield
  using IBM's backward numbering notation.

  ALGORITHM:
  Read the control register value.
  Shift the bitfield into the least significant bits of the word.
  Mask off the upper bitfields, leaving only the one to be returned.

  GLOBALS USED:

  OTHER:
  See SGA Hardware Reference for explanation of the SGA control
  register bitfields.
  -------------------------------------------------------------------*/

int Read_Reg_Bitfield(unsigned long address, unsigned long field,
                      unsigned long length)
{
    unsigned cntl; /* return value of control register */
    unsigned long base;

    /* first, read control register */
    cntl = RL( address );

    /* then shift field into low order bits so that the "length" */
    /* lowest order bits contain the bit field                   */
    cntl = ( cntl >> ( 32 - field - length) );

    /* mask off all but the "length" lowest order bits by first, */
    /* shifting ones "length" positions to the left which leaves */
    /* zeros in the "length" lowest order bits, and then         */
    /* inverting the bits and logically ANDing them with the     */
    /* value of cntl which leaves only the "length" lowest order */
    /* bits intact                                               */

    cntl = ( cntl & ( ~( 0xffffffff << length) ) );

    return( cntl);
} /* end Read_Reg_Bitfield */


unsigned long gga_Get_Color(int color)
{
    unsigned long colordata; /* return value explained above */

    int bits_pixel, /* value of bitx/pixel field returned from ctl reg */
    copies, /* # copies of color value in word = 32 / bits_pixel */
    i; /* loop counter */

    bits_pixel = 8;

    /* colordata must contain 32/Bits_Pixel copies of color         */
    /* according to full word format detailed in SGA Hardware Spec  */

    /* first, set number of color fields in word                    */
    copies = 32 / bits_pixel;

    /* next initialize first field with color value                 */
    colordata = color;

    for ( i = 1; i <= copies; i++) {
        /* shift color value to align with next field                */
        color = color << bits_pixel;

        /* logically OR the color value into the data word           */
        colordata = colordata | color;
    } /* endfor */

    return( colordata);
} /* end Get_Color */


void gga_cls(int color)
{
    unsigned long temp1, temp2, dummy;
    unsigned int busy, base;

    wait_for_wtkn_ready();

    Write_Reg_Bitfield( W9100_RASTER, FMASKOVER, MINTERM_FIELD, 16);
    wait_for_wtkn_ready();
    Write_Reg_Bitfield( W9100_P_WINMIN, 0, WIN_X_FIELD, 13 );
    wait_for_wtkn_ready();
    Write_Reg_Bitfield( W9100_P_WINMIN, 0, WIN_Y_FIELD, 13 );
    wait_for_wtkn_ready();
    Write_Reg_Bitfield( W9100_P_WINMAX, gga_x_max, WIN_X_FIELD, 13 );
    wait_for_wtkn_ready();
    Write_Reg_Bitfield( W9100_P_WINMAX, gga_y_max, WIN_Y_FIELD, 13);
    wait_for_wtkn_ready();
    Write_Reg_Bitfield( W9100_B_WINMIN, 0, WIN_X_FIELD, 13 );
    wait_for_wtkn_ready();
    Write_Reg_Bitfield( W9100_B_WINMIN, 0, WIN_Y_FIELD, 13 );
    wait_for_wtkn_ready();
    Write_Reg_Bitfield( W9100_B_WINMAX, gga_x_max, WIN_X_FIELD, 13 );
    wait_for_wtkn_ready();
    Write_Reg_Bitfield( W9100_B_WINMAX, gga_y_max, WIN_Y_FIELD, 13);
    wait_for_wtkn_ready();

    WL( gga_Get_Color( color), W9100_FOREGND);
    WL( 0, W9100_META_RECT_X);
    WL( 0, W9100_META_RECT_Y);
    WL( gga_x_max, W9100_META_RECT_X);
    WL( gga_y_max, W9100_META_RECT_Y);
    temp1 = Read_Reg_Bitfield( W9100_QUAD_CMD, 1, 1); /* Initiate operation */
    do
      {
        temp1 = Read_Reg_Bitfield( W9100_STATUS, 1, 1);
      } while( temp1 != 0);
}



/***********************************************************************/
/* WritePalette simply writes all or part of a palette out to the DAC. */
/***********************************************************************/
void WritePalette(int start,int num,unsigned char *pal)
        {
        unsigned long value, templ;
        int i;

        value = start & 0xff;
        value |= (value << 8) | (value << 16) | (value << 24);
        templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
        WL(value, W9100_PALCURRAMW);
        dac_workaround();
        for (i=0;i<num;i++)
                {
                value = *pal++;
                value |= (value << 8) | (value << 16) | (value << 24);
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                WL(value, W9100_PALDATA);
                dac_workaround();

                value = *pal++;
                value |= (value << 8) | (value << 16) | (value << 24);
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                WL(value, W9100_PALDATA);
                dac_workaround();

                value = *pal++;
                value |= (value << 8) | (value << 16) | (value << 24);
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                WL(value, W9100_PALDATA);
                dac_workaround();
                }
        return;
        }


/***********************************************************************/
/* ReadPalette simply reads all or part of a palette from the DAC.     */
/***********************************************************************/
void ReadPalette(int start,int num,unsigned char *pal)
        {
        unsigned long value, templ;
        int i;

        /* Set starting address */
        value = start & 0xff;
        value |= (value << 8) | (value << 16) | (value << 24);
        templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
        WL(value, W9100_PALCURRAMR);
        dac_workaround();
        for (i=0;i<num;i++)
                {
                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                value = RL(W9100_PALDATA) >> 8;
                dac_workaround();
                *pal++ = (unsigned char)value;

                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                value = RL(W9100_PALDATA) >> 8;
                dac_workaround();
                *pal++ = (unsigned char)value;

                templ = RL(FRAME_BUFFER+RAMDAC_HW_BUG);
                value = RL(W9100_PALDATA) >> 8;
                dac_workaround();
                *pal++ = (unsigned char)value;
                }
        return;
        }

/***********************************************************************/
/* Test the ramdac palette                                             */
/***********************************************************************/

void palettecheck(void)
        {
        unsigned char testpalette[768],
                                verifypalette[768];
        int i, errorcount;

        errorcount = 0;

        for (i = 0 ; i < 768 ; i++)
                testpalette[i] = i;

        WritePalette(0,256,testpalette);
        ReadPalette(0,256,verifypalette);

        for (i = 0 ; i < 768 ; i++)
                {
                if (testpalette[i] != verifypalette[i])
                        {
                        if (errorcount < 9)
                                printf("Error: got $%02X expected $%02X\n",
                                        verifypalette[i], testpalette[i]);
                        errorcount++;
                        }
                }
        if (errorcount == 0)
                printf("No errors!\n");
        else
                printf("Counted %d errors.\n", errorcount);
        }


/***********************************************************************/
/* Programs the RAMDAC with a 332 palette                              */
/***********************************************************************/

void colorpalette(void)
        {
        unsigned char palette[768];
        unsigned char recon_rg[] = {0,36,73,109,146,182,219,255};
        unsigned char recon_b[] = {0,85,170,255};
        unsigned int i;


        for (i = 0 ; i < 256 ; i++)
                {
                palette[i*3] = recon_rg[i>>5];
                palette[i*3+1] = recon_rg[(i>>2)&0x7];
                palette[i*3+2] = recon_b[i&0x3];
                }

        WritePalette(0,256,palette);
        }


/***********************************************************************/
/* Programs the RAMDAC with a 216 element palette starting at entry 40 */
/***********************************************************************/
void color216palette(void)
        {
        unsigned char palette[768];
        unsigned char recon_rg[] = {0,36,73,109,146,182,219,255};
        unsigned char recon_b[] = {0,85,170,255};
        unsigned int i;

        for (i = 0 ; i < 216 ; i++)
                {
                palette[i*3]   = (i / 36) * 51;
                palette[i*3+1] = ((i / 6) % 6) * 51;
                palette[i*3+2] = (i % 6) * 51;
                }
        WritePalette(40,216,palette);
        }


void dac_workaround()
  {
    unsigned long value;
        /* P9100 ramdac adjust bug workaround                            */
        /* Worst case assumed: 640x480x8 25 MHz pixclk                   */
        /* Need to meet the 6*pixclk dacwr- high time.                   */
        /*                                                               */
        /* x memclks = (6 pixclks) * (50M memclks/sec)/(25M pixclks/sec) */
        /*           = 12 MEMCLKs                                        */
        /*                                                               */
        /* A read from a CRTC register in the P9100 will give us         */
        /* a 3 pixclk delay plus some memory clocks to actually          */
        /* complete the read, so one read should be enough. Two          */
        /* if you're paranoid.                                           */
        /*                                                               */

        value = RL(W9100_PU_CONFIG);
        value = RL(W9100_PU_CONFIG); /*@ OK...so I'm paranoid. Sue me. */
  }

void wait_for_wtkn_ready(void)
  {
    ULONG busy;

    do
      {
        busy = RL_NO_WAIT(W9100_STATUS);
        busy &= BUSYMASK;
      } while( busy != 0);
  }


/***********************************************************************/
/* Write a value to the specified RG525 register                       */
/***********************************************************************/
void WriteIBM525(USHORT index, UCHAR bvalue)
        {
        ULONG value, ltemp;

        dac_workaround();

        value = index & 0x00ff;
        value |= (value << 8) | (value << 16) | (value << 24);
        ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
        WL(value, W9100_INDEXLOW);

        dac_workaround();

        value = ( index & 0xff00 ) >> 8;
        value |= (value << 8) | (value << 16) | (value << 24);
        ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
        WL(value, W9100_INDEXHIGH);

        dac_workaround();

        value = bvalue;
        value |= (value << 8) | (value << 16) | (value << 24);
        ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
        WL(value, W9100_INDEXDATA);
        }


/***********************************************************************/
/* Read a value from the specified RG525 register                      */
/***********************************************************************/
UCHAR ReadIBM525(USHORT index)
        {
        ULONG value, ltemp;
        UINT  bvalue;

        dac_workaround();

        value = index & 0x00ff;
        value |= (value << 8) | (value << 16) | (value << 24);
        ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
        WL(value, W9100_INDEXLOW);

        dac_workaround();

        value = ( index & 0xff00 ) >> 8;
        value |= (value << 8) | (value << 16) | (value << 24);
        ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
        WL(value, W9100_INDEXHIGH);

        dac_workaround();

        ltemp = RL(FRAME_BUFFER+RAMDAC_HW_BUG);       /* hw bug  */
        bvalue = RL(W9100_INDEXDATA);

        return((UCHAR)(bvalue >> 8));
        }


/*******************************************/
/* My replacements for ESW/BUD subroutines */
/*******************************************/

void  out32le(ulong addr, ulong data)
  {
    __iospace_eieio();
    *((ulong *) (addr | 0x60000)) = data;
    __iospace_eieio();
  }
void  out32(ulong addr, ulong data)
  {
    __iospace_eieio();
    *((ulong *) addr) = data;
    __iospace_eieio();
  }
void  out8(ulong addr, uchar data)
  {
    __iospace_eieio();
    *((uchar *) addr) = data;
    __iospace_eieio();
  }
void  BR_WL(ulong data, ulong addr)
  {
    __iospace_eieio();
    *((ulong *) (addr | 0x60000)) = data;
    __iospace_eieio();
  }
void  WL(ulong data, ulong addr)
  {
    __iospace_eieio();
    *((ulong *) addr) = data;
    __iospace_eieio();
  }
void  WC(uchar data, ulong addr)
  {
    __iospace_eieio();
    *((uchar *) addr) = data;
    __iospace_eieio();
  }
ulong in32le(ulong addr)
  {
    ulong templ;

    __iospace_eieio();
    templ = *((ulong *) (addr | 0x60000));
    __iospace_eieio();
    return(templ);
  }
ulong in32(ulong addr)
  {
    ulong templ;

    __iospace_eieio();
    templ = *((ulong *) addr);
    __iospace_eieio();
    return(templ);
  }
uchar in8(ulong addr)
  {
    uchar tempc;

    __iospace_eieio();
    tempc = *((uchar *) addr);
    __iospace_eieio();
    return(tempc);
  }
ulong BR_RL(ulong addr)
  {
    ulong templ;

    __iospace_eieio();
    templ = *((ulong *) (addr | 0x60000));
    __iospace_eieio();
    return(templ);
  }
ulong RL(ulong addr)
  {
    ulong templ;

    __iospace_eieio();
    templ = *((ulong *) addr);
    __iospace_eieio();
    return(templ);
  }
ulong RL_NO_WAIT(ulong addr)
  {
    ulong templ;

    __iospace_eieio();
    templ = *((ulong *) addr);
    __iospace_eieio();
    return(templ);
  }
uchar RC(ulong addr)
  {
    uchar tempc;

    __iospace_eieio();
    tempc = *((uchar *) addr);
    __iospace_eieio();
    return(tempc);
  }


/****************************************************************************/
/* FUNCTION: wr_cfg_byte

   DESCRIPTION: Uses the machine device driver to write ONE BYTE to
                the specified PCI register

****************************************************************************/

int wr_cfg_byte(int fdes, unsigned char data, unsigned int slot, unsigned int reg)
{
        MACH_DD_IO iob;
        int rc;
        char *pdata;

        pdata = (char *) &data;

        iob.md_data = pdata;
        iob.md_incr = MV_BYTE;
        iob.md_size = 1;
        iob.md_addr = reg;
        iob.md_sla  = slot;
        rc = ioctl(fdes, MIOPCFPUT, &iob);
        return (rc);
}

/****************************************************************************/
/* FUNCTION: wr_cfg_word

   DESCRIPTION: Uses the machine device driver to write ONE WORD to
                the specified PCI register

****************************************************************************/

int wr_cfg_word(int fdes, unsigned long data, unsigned int slot, unsigned int reg)
{
        MACH_DD_IO iob;
        int rc;
        char *pdata;

        pdata = (char *) &data;

        iob.md_data = pdata;
        iob.md_incr = MV_WORD;
        iob.md_size = 1;
        iob.md_addr = reg;
        iob.md_sla  = slot;
        rc = ioctl(fdes, MIOPCFPUT, &iob);
        return (rc);
}

/****************************************************************************/
/* FUNCTION: rd_cfg_byte

   DESCRIPTION: Uses the machine device driver to read ONE BYTE from
                the specified PCI register

****************************************************************************/

int rd_cfg_byte(int fdes, unsigned char *pdata, unsigned int slot, unsigned int reg)
{
        MACH_DD_IO iob;
        int rc;

        iob.md_data = pdata;
        iob.md_incr = MV_BYTE;
        iob.md_size = 1;
        iob.md_addr = reg;
        iob.md_sla  = slot;
        rc = ioctl(fdes, MIOPCFGET, &iob);
        return (rc);
}


/****************************************************************************/
/* FUNCTION: rd_cfg_word

   DESCRIPTION: Uses the machine device driver to read ONE WORD from
                the specified PCI register

****************************************************************************/

int rd_cfg_word(int fdes, unsigned long *pdata, unsigned int slot, unsigned int reg)
{
        MACH_DD_IO iob;
        int rc;

        iob.md_data = (char *)pdata;
        iob.md_incr = MV_WORD;
        iob.md_size = 1;
        iob.md_addr = reg;
        iob.md_sla  = slot;
        rc = ioctl(fdes, MIOPCFGET, &iob);
        return (rc);
}


void Debug_Brkpt(void)
  {
    return;
  }


