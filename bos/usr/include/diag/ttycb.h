/* @(#)60       1.6.3.8  src/bos/usr/include/diag/ttycb.h, daasync, bos41J, 9519A_all 4/27/95 08:43:16 */
/*
 *   COMPONENT_NAME: daasync
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_TTYCB
#define _H_TTYCB

#ifndef uchar
#define uchar   unsigned char
#endif

#ifndef uint
#define uint    unsigned int
#endif

/*   Global variable used to determine if extended debug should be enabled   */

int     AsyncDebug=0;


/*      Planar/adapter POS 0 (ID register 1)    */

#define SIO             95      /* 0x5f - SIO planar serial ports            */
#define SIO2            230     /* 0xe6 - SIO planar serial ports (Rel 2)    */
#define SIO3            254     /* 0xfe - SIO planar serial ports (SAL/CHAP) */
#define SIO4            207     /* 0xcf - SIO planar serial ports (CAB)      */
#define SIO5            8       /* 0x08 - SIO planar serial ports (RB 3/3+)  */
#define SIO6            9       /* 0x09 - SIO planar serial ports (RB 4/5/6) */
#define SIO7            217     /* 0xD9 - SIO planar serial ports (IOD-P)    */
#define SIO8            218     /* 0xDA - SIO planar serial ports (IOD-F)    */
#define SIO9            163     /* 0xA3 - SIO planar serial ports (IOD-PAN)  */
#define SIO10           10      /* 0x0A - SIO planar serial ports (VIC)      */
#define P8RS232ISA      32      /* 0x20 - 8 port rs232 async adapter (ISA)   */
#define P8RS232         208     /* 0xd0 - 8 port rs232 async adapter         */
#define P8RS422         209     /* 0xd1 - 8 port rs422 async adapter         */
#define P8RS188         210     /* 0xd2 - 8 port mil188 async adapter        */
#define P16RS422        211     /* 0xd3 - 16 port rs422 async adapter        */
#define P16RS232        214     /* 0xd6 - 16 port rs232 async adapter        */
#define P64RS232        253     /* 0xfd - 64 port rs232 async controller     */
#define P64RS232EP      120     /* 0x78 - 64 port rs232 async controller     */
#define P128RS232ISA    64      /* 0x40 - 128 port rs232 async (ISA)         */
#define P128RS232       225     /* 0xe1 - 128 port rs232 async controller    */
#define UNKNOWN         0xff

#define STOP            99
#define READ            1
#define WRITE           2
#define RWC             0
#define MAX_BAUD        15
#define MAX_PARITY      3
#define MAX_SBITS       2
#define MAX_CHARSZ      4
#define MAX_TUS         8
#define DD_NAME_LEN     20
struct ttycb_data
{
        int     pat_size;               /* size of data wrap test pattern    */
        char    pattern[4096];          /* character pattern for data wrap   */
        int     data_mode;              /* 0 = RWC, 1 = READ, 2 = WRITE      */
        int     baud[MAX_BAUD];         /* baud rates to be tested           */
        int     parity[MAX_PARITY];     /* parity settings to be tested      */
        int     sbits[MAX_SBITS];       /* stop bit settings to be tested    */
        int     chars[MAX_CHARSZ];      /* character size for data wrap test */
        uchar   adapter;                /* Adapter type. */
        char    ixon;                   /* toggle xon bit in terms.c_iflag   */
        char    ixoff;                  /* toggle xoff bit in terms.c_iflag  */
        int     mcparity;               /* flag for microchannel parity      */
        int     prtr_att;               /* port configured for printer       */
        int     sal_sio;                /* base register value for salmon    */
        int     brd_conc_id;            /* brd << 3 + concentrator number    */
        int     pinout_conv;            /* 128-p to 64-p pinout conv test    */
        char    dev_drvr[DD_NAME_LEN];  /* device driver name                */
};
struct tucb_data
{
        struct  tucb_t          header; /* Standard TU header structure */
        struct  ttycb_data      ttycb;  /* tty TU specific structure    */
};
#endif /* _H_TTYCB */
