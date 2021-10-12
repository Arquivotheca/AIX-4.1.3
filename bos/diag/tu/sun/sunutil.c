static char sccsid[] = "@(#)36  1.5  src/bos/diag/tu/sun/sunutil.c, tu_sunrise, bos41J, 9523C_all 6/9/95 15:59:11";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: cksum_32
 *              lswap
 *              msgfile
 *              msgmenu
 *              msgout
 *              pio_mcread
 *              pio_mcwrite
 *              pio_read
 *              pio_write
 *              readchip
 *              scalar_read
 *              scalar_write
 *              swap
 *              write2chip
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*******************************************************************/
/*                                                                 */
/*      SUNUTIL.C                                                  */
/*                                                                 */
/*******************************************************************/

#define PUTQP_DEBUG
#undef  PUTQP_DEBUG
#define GETQP_DEBUG
#undef  GETQP_DEBUG

#include <stdio.h>
#include <stddef.h>
#include "suntu.h"
#include "regs.h"
#include "error.h"
#include "sun_tu_type.h"
#include <sys/diagex.h>

extern diag_struc_t  *handle;                   /* handle for Diagex       */

/*******************************************************************
* pio_mcwrite ()
*      write a value to Sunrise's address space thru micro channel
*      All Access is 32-bit size
* Inputs
*      int  addr;      // absolute address to write to
*      int  value;     // data to write to address
*      int  tl;        // Times to TightLoop (-1=forever)
* Output
*      none
*******************************************************************/
int pio_mcwrite(unsigned int addr, unsigned int value, int tl) {
       int rc;
       unsigned int data;

       if (addr != _SCP)
               data = lswap(value);
       else
               data = value;
       for (; tl>0;  tl--) {
               /* loop until tight loop count expires */
               /* or forever if tl == -1          */
               /* Write thru micro-channel address */
               if (addr != _SCP)
                  rc=diag_io_write(handle, IOLONG, addr, data, NULL, PROCLEV);
               else
                  rc=diag_io_write(handle, IOCHAR, addr, data, NULL, PROCLEV);
               if (rc)
               {
                  error.io_readwrite.error_code = ERRORPIO_MCWRITE;
                  error.io_readwrite.io_dgx_rc = rc;
                  error.io_readwrite.address = addr;
                  error.io_readwrite.data = value;
                  log_error(error);
                  return(ERRORPIO_MCWRITE);
               }
        }
        return (OK);
} /* end pio_mcwrite() */

/****************************************************************
* pio_mcread ()
*      read a value from Sunrise's address space thru micro channel
*      All Access is 32-bit size
* Inputs
*      int  addr;      // absolute address to write to
*      int  value;     // data to write to address
*      int  tl;        // Times to TightLoop (-1=forever)
* Output
*      returns last value read
****************************************************************/
int pio_mcread(unsigned int addr, unsigned int *data, int tl) {

       unsigned int  val;        /* value read */
       int rc;

       for (; tl>0;  tl--) {
               /* loop until tight loop count expires */
               /* or forever if tl == -1          */
               if (rc=diag_io_read(handle, IOLONG, addr, &val, NULL, INTRKMEM))
               {
                  error.io_readwrite.error_code = ERRORPIO_MCREAD;
                  error.io_readwrite.io_dgx_rc = rc;
                  error.io_readwrite.address = addr;
                  error.io_readwrite.data = lswap(val);
                  log_error(error);
                  return(ERRORPIO_MCREAD);
               }
               val = lswap(val);
               /* if addr is MDATA then store value in temp. storage for      */
               /* using in screen refresh (in gmenu.c)                        */
        }       /* end for tl */
       *data = val;
       return (OK);
} /* end pio_mcread() */

/****************************************************************
* pio_write ()
*      write a value to Sunrise's address space
*      All Access is 32-bit size
* Inputs
*      int  addr;      // absolute address to write to
*      int  value;     // data to write to address
*      int  tl;        // Times to TightLoop (-1=forever)
* Output
*      none
****************************************************************/
int pio_write(unsigned int address, unsigned int value, int tl) {
       int rc;
       unsigned int data, addr;

       addr = lswap(address);
       data = lswap(value);
       for (; tl>0;  tl--) {
               /* loop until tight loop count expires */
               /* or forever if tl == -1          */
               /* Write the Local Bus address to HSBR register of Miami */
               if (rc=diag_io_write(handle, IOLONG, _HSBR, addr, NULL, PROCLEV))
               {
                  error.io_readwrite.error_code = ERRORPIO_WRITE;
                  error.io_readwrite.io_dgx_rc = rc;
                  error.io_readwrite.address = address;
                  error.io_readwrite.data = value;
                  log_error(error);
                  return(ERRORPIO_WRITE);
               }
               /* Write data to MDATA register of Miami */
               if (rc=diag_io_write(handle, IOLONG, _MDATA, data, NULL, PROCLEV))
               {
                  error.io_readwrite.error_code = ERRORPIO_WRITE;
                  error.io_readwrite.io_dgx_rc = rc;
                  error.io_readwrite.address = address;
                  error.io_readwrite.data = value;
                  log_error(error);
                  return(ERRORPIO_WRITE+1);
               }
        }       /* end for tl */
        return (OK);

} /* end pio_write() */

/****************************************************************
* pio_read ()
*      read a value from Sunrise's address space
*      All Access is 32-bit size
* Inputs
*      int  addr;      // absolute address to write to
*      int  value;     // data to write to address
*      int  tl;        // Times to TightLoop (-1=forever)
* Output
*      returns last value read
****************************************************************/
int pio_read(unsigned int address, unsigned int *data, int tl) {

       unsigned int  val, addr;        /* value read */
       int rc;

       addr = lswap(address);
       for (; tl>0;  tl--) {
               /* loop until tight loop count expires */
               /* or forever if tl == -1          */
               /* Write the Local Bus address to HSBR register of Miami */
               if (rc=diag_io_write(handle, IOLONG, _HSBR, addr, NULL, PROCLEV))
               {
                  error.io_readwrite.error_code = ERRORPIO_READ;
                  error.io_readwrite.io_dgx_rc = rc;
                  error.io_readwrite.address = address;
                  error.io_readwrite.data = 0;  /* Dummy data */
                  log_error(error);
                  return(ERRORPIO_READ);
               }
               /* Read data from MDATA register of Miami */
               if (rc=diag_io_read(handle, IOLONG, _MDATA, &val, NULL, INTRKMEM))
               {
                  error.io_readwrite.error_code = ERRORPIO_READ;
                  error.io_readwrite.io_dgx_rc = rc;
                  error.io_readwrite.address = address;
                  error.io_readwrite.data = lswap(val);
                  log_error(error);
                  return(ERRORPIO_READ+1);
               }
               else
               val = lswap(val);
               /* Store value in temp. storage for                            */
               /* using in screen refresh (in gmenu.c)                        */
              /*   MDATAvalue = val; */
        }       /* end for tl */
        *data = val;
        return (OK);

} /* end pio_read() */

/****************************************************************
* scalar_write ()
*      write a value to Sunrise's scalar (2070) chip
*      32-bit size access
* Inputs
*      int  addr;      // INDEX address of scalar chip
*      int  value;     // data to write to address
*      int  tl;        // Times to TightLoop (-1=forever)
* Output
*      none
****************************************************************/
int scalar_write(unsigned int addr, unsigned int data, int tl) {
  int rc;

       for (; tl>0;  tl--) {
         /* if accessing the HIU_* registers */
         if (addr < 5) {
           if (rc=pio_write((SCbaseaddr+(addr << 2)), data, tl)) return (rc);
         /* else accessing the scalar registers */
         } else {
               /* loop until tight loop count expires */
               /* or forever if tl == -1          */
               /* Write to HIU_2 register w/ the index value of Miami register */
               if (pio_write(HIU_2, addr, tl))   return(rc);
               /* Write to HIU_3 register to write data to Miami register */
               if (pio_write(HIU_3, data, tl))   return(rc);
         } /* endif */
         UpdateGen();
       }       /* end for tl */
       return (OK);

} /* end scalar_write() */

/****************************************************************
* scalar_read ()
*      read a value from Sunrise's scalar (2070) chip
*      32-bit size access
* Inputs
*      int  addr;      // INDEX address of scalar chip
*      int  value;     // data to write to address
*      int  tl;        // Times to TightLoop (-1=forever)
* Output
*      returns last value read
****************************************************************/
int scalar_read(unsigned int addr, unsigned int *data, int tl) {

       unsigned int  val;        /* value read */
       int rc;

       for (; tl>0;  tl--) {
         /* if accessing the HIU_* registers */
         if (addr < 5) {
           if (rc = pio_read((SCbaseaddr+(addr << 2)), &val, tl)) return(rc);
         /* else accessing the scalar registers */
         } else {
               /* loop until tight loop count expires */
               /* or forever if tl == -1          */
               /* Write to HIU_2 register w/ the index value of Miami register */
               if (rc=pio_write(HIU_2, addr, tl))  return(rc);
               /* Write to HIU_3 register to read data from Miami register */
               if (rc= pio_read(HIU_3, &val, tl))  return(rc);
         } /* endif */
       }       /* end for tl */

       *data = val;
       return (OK);

} /* end scalar_read() */

/*************************************************************
*  WRITE2CHIP()
*     Input : cmd  = 'W'
*             chip = Miami, CodecMem, CodecRegs, Scalar(2070)...
*             reg  = register address
*             val  = data value
*             tl   = tight loop count
*    Output :
*************************************************************/
int write2chip(char chip, unsigned int reg, unsigned int val,int tl) {
        int rc;

        switch (chip) {
        case MIAMIREGS:
        case MImenu:
                /* 32-bit access ONLY for MIAMI chip */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc=pio_write (MIbaseaddr+reg, val, tl)) return(rc);
                } /* end for tl */
                break;
        case MCIO:
        case MCmenu:
                /* 16-bit access ONLY for Micro-Channel access */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc=pio_mcwrite (reg, val, tl)) return(rc);
                } /* end for tl */
                break;
        case LOCALREGS:
        case LOmenu:
                /* 16-bit access ONLY for Local Bus access */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        reg = reg << 16; /* Since the LOCAL 16-bit is mapped  */
                        if (rc=pio_write (reg, val, tl)) return(rc);
                } /* end for tl */
                break;
        case CODECREGSMEM:
        case CODECREGS:
        case CODECINITMENU:
        case CDmenu:
                /* 16-bit access ONLY for CODEC chip access */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc=pio_write (CDbaseaddr+reg, val, tl)) return(rc);
                } /* end for tl */
                break;
        case PX2070REGSMENU:
        case SCmenu:
                /* 16-bit access ONLY for SCALAR chip access */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc=scalar_write (reg, val, tl)) return(rc);
                } /* end for tl */
                break;
        case FRONTEND:
        case FEmenu:
                /* 8-bit access ONLY for FRONT END video access */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc=pio_write (FEbaseaddr+reg, val, tl)) return(rc);
                } /* end for tl */
                break;
        case ABmenu:
                /*32-bit access ONLY  absolute addressing */
               for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc=pio_write (reg, val, tl)) return(rc);
                } /* end for tl */
                break;
        default:
                return(ERRORINVALID_ID);
        } /* end switch(chip) */
        return (OK);
} /* end write2chip() */

/*************************************************************
*  ReadChip()
*     Input : cmd  = 'r' or 'R'
*             chip = Miami, CodecMem, CodecRegs, Scalar(2070)
*             reg  = register address
*             tl   = tight loop count
*    Output :
*             val  = data value
*************************************************************/
int readchip(char chip, unsigned int reg, int tl, unsigned int *data) {
        unsigned int val;
        int rc;

        switch (chip) {
        case MIAMIREGS:
        case MImenu:
               /* 32-bit access ONLY for MIAMI chip */
               for (; tl>0;  tl--) {
               /* loop until tight loop count expires */
               /* or forever if tl == -1          */
                        if (rc=pio_read(MIbaseaddr+reg,&val,tl)) return(rc);
               } /* end for tl */
           break;
        case MCIO:
        case MCmenu:
                /* 16-bit access ONLY for Micro-Channel access */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc=pio_mcread (reg, &val, tl)) return(rc);
                } /* end for tl */
                break;
        case LOCALREGS:
        case LOmenu:
                /* 16-bit access ONLY for Local Bus access*/
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        reg = reg << 16;
                        if (rc=pio_read(reg,&val,tl)) return(rc);
                        val = val & 0xFFFF;
                } /* end for tl */
                break;
        case CODECREGSMEM:
        case CODECREGS:
        case CODECINITMENU:
        case CDmenu:
                /* 16-bit access ONLY for CODEC chip access */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc=pio_read(CDbaseaddr+reg,&val,tl)) return(rc);
                        val = val & 0xFFFF;
                } /* end for tl */
                break;
        case PX2070REGSMENU:
        case SCmenu:
                /* 16-bit access ONLY for SCALAR chip access */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc=scalar_read(reg,&val,tl)) return(rc);
                        val = val & 0xFFFF;
                } /* end for tl */
                break;
        case FRONTEND:
        case FEmenu:
                /* 16-bit access ONLY for FRONT END video access */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc=pio_read(FEbaseaddr+reg,&val,tl)) return(rc);
                        val = val & 0xFF;
                } /* end for tl */
                break;
        case VPD:
        case VPmenu:
                /* 8-bit access ONLY for VPD access */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        if (rc = pio_read(VPbaseaddr+reg,&val,tl)) return (rc);
                        val = val & 0xFF;
                } /* end for tl */
                break;
        case ABmenu:
                /*32-bit access ONLY absolute addressing */
                for (; tl>0;  tl--) {
                        /* loop until tight loop count expires */
                        /* or forever if tl == -1          */
                        pio_read(reg,&val,tl);
                } /* end for tl */
                break;
        default:
                *data = 0;
                return(ERRORINVALID_ID);
        } /* end switch(chip) */
        *data = val;
        return (OK);
} /* end readchip() */

/*****************************************************************************
*
* lswap
*
*     This routine swaps the Hi and Low words in a 32 bit value.
*
*****************************************************************************/

unsigned int lswap (unsigned int num)
   {
        num = ((num >> 24) & 0x000000FF) | ((num >> 8) & 0x0000FF00) |
              ((num << 24) & 0xFF000000) | ((num << 8) & 0x00FF0000);
        return(num);
   }

/*****************************************************************************
*
* swap
*
*     This routine swaps the Hi and Low bytes
*
*****************************************************************************/

short swap (num)
   short num;
   {
        num = ((num << 8) & 0xFF00) | ((num >> 8) & 0x00FF);
        return(num);
   }

/*------------------ m s g m e n u ( ) --------------------------*/

int msgmenu(char *msg0, char *msg1, char *msg2, char *msg3, char *msg4, char *msg5) {
        return(0);
}/* end msgmenu() */
int msgfile(char *msg0, char *msg1, char *msg2, char *msg3, char *msg4, char *msg5) {

FILE *bf=fopen("/tmp/BLTmsg","a+");

        if (! bf) return(-1);
        fprintf(bf,"*****************************************************\n");
        fprintf(bf, "* %s\n",msg0);
        fprintf(bf, "* %s\n",msg1);
        fprintf(bf, "* %s\n",msg2);
        fprintf(bf, "* %s\n",msg3);
        fprintf(bf, "* %s\n",msg4);
        fprintf(bf, "* %s\n",msg5);
        fprintf(bf,"*****************************************************\n\n");
        close(bf);
        return(0);

}/* end msgout() */
int msgout(char *msg0, char *msg1, char *msg2, char *msg3, char *msg4, char *msg5) {

        fprintf(stderr,"*****************************************************\n");
        fprintf(stderr, "* %s\n",msg0);
        fprintf(stderr, "* %s\n",msg1);
        fprintf(stderr, "* %s\n",msg2);
        fprintf(stderr, "* %s\n",msg3);
        fprintf(stderr, "* %s\n",msg4);
        fprintf(stderr, "* %s\n",msg5);
        fprintf(stderr,"*****************************************************\n\n");
        return(0);

}/* end msgout() */

/***************************************************************************
* NAME: cksum_32()
*
* FUNCTION:
*     crc32 generates a 32 bit "classic" CRC using the following
*     CRC polynomial:
*
*   g(x) = x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8
*               + x^7  + x^5  + x^4  + x^2  + x^1  + x^0
*
*   e.g. g(x) = 1 04c1 1db7
*
* NOTES:
*
*    This function has been optimized for speed and size.
*    By studying the CRC algorithm, we note that
*    the data byte and the high byte of the accumulator are combined
*    together to a value between 0 and 255 which can be precalculated in
*    a table of the 256 possible values.  This table can further be
*    collapsed by computing a table of values for the high nybble and a
*    table of values for the low nybble, which are then XOR'ed into the
*    accumulator.
*
*    INPUT:    pbuff            - 32-bit word data
*              *crc_result      - ptr to the cksum result
*
*    OUTPUT:   return value     - cksum result value
*
***************************************************************************/


static unsigned long crctl[16] = {
  0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
  0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
  0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
  0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD};

static unsigned long crcth[16] = {
  0x00000000, 0x4C11DB70, 0x9823B6E0, 0xD4326D90,
  0x34867077, 0x7897AB07, 0xACA5C697, 0xE0B41DE7,
  0x690CE0EE, 0x251D3B9E, 0xF12F560E, 0xBD3E8D7E,
  0x5D8A9099, 0x119B4BE9, 0xC5A92679, 0x89B8FD09};

unsigned long cksum_32(pbuff,crc_result)
unsigned long pbuff;
unsigned long *crc_result;
{
  unsigned long   i;
  unsigned long   temp;
  char pbuffbyte;

  for (i=0; i<4; i++)
  {
    pbuffbyte = (pbuff>>(i*8)) & 0xff;
    temp = (*crc_result >> 24) ^ pbuffbyte;
    *crc_result <<= 8;
    *crc_result ^= crcth[ temp/16 ];
    *crc_result ^= crctl[ temp%16 ];
  }
  return(*crc_result);
} /* end cksum_32() */
