/* @(#)28  1.4  src/bos/kernext/disp/ped/inc/bidefs.h, peddd, bos411, 9428A410j 3/19/93 18:50:55 */
/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: GET_TIMER
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#ifndef _H_BIDEFS
#define _H_BIDEFS
#include   <sys/types.h>

/****************************************************************************/
/*                         General Purpose Definitions                      */
/****************************************************************************/
#define   CARD_ID       0x8EE3
#define   MAX_CARD_CNT  4
#define   MAX_SE_WD_LEN 4096
#define   MAX_TIMER     10
#define   MAX_BLIT_SIZE ((BUFYDELP * BUFXDELP) << 4)

#define   ON            -1
#define   OFF            0

#define   N              0
#define   R              1
#define   W              2

#define   BLANK         -1
#define   NOBLANK        0

#define   DMA_ADDR       0x00600000;      /* This is temporary */

/****************************************************************************/
/*                              Opcode Ranges                               */
/****************************************************************************/
#define   PIO_MIN_OP    0x0000
#define   PIO_MAX_OP    0x83FF
#define   DRV_MIN_OP    0xF000
#define   DRV_MAX_OP    0xFFFF
#define   HCR_MIN_OP    0x9000
#define   HCR_MAX_OP    0x9040
#define   PCB_MIN_OP    0xA000
#define   PCB_MAX_OP    0xA040

/****************************************************************************/
/*                      Command Block Size Definitions                      */
/****************************************************************************/
#define   HCR_BLKSIZE   2                       /* Host Commo Reg block size*/
#define   PCB_BLKSIZE   8                       /* Pri Cmd Buffer block size*/
#define   PIO_BLKSIZE   16                      /* PIO Command block size   */
#define   DRV_BLKSIZE   16                      /* Driver Cmd block size    */
#define   MAX_BLKSIZE   16                      /* Maximum block size       */

/****************************************************************************/
/*                         Blast Screen Definitions                         */
/****************************************************************************/
#define   BUFXMAXP      1279            /* Maximum buffer X value in pixels */
#define   BUFXDELP      1280            /* Delta   buffer X value in pixels */
#define   BUFYMAXP      1023            /* Maximum buffer Y value in pixels */
#define   BUFYDELP      1024            /* Delta   buffer Y value in pixels */

/****************************************************************************/
/*                      Blast Blit Format Definitions                       */
/****************************************************************************/
#define   BIT           0
#define   BYTE          1
#define   WORD          2

/****************************************************************************/
/*                  Register Masks and Format Definitions                   */
/****************************************************************************/
#define   X_MASK        0x000007FF
#define   Y_MASK        0x000003FF

#define   BYTE_MASK     0xFF
#define   NIBBLE_MASK   0x0F

/****************************************************************************/
/*                    Microcode Configuration Definitions                   */
/****************************************************************************/
#define   FILETYPE      0
#define   SECCOUNT      2
#define   LOADLEVEL     4
#define   ITERATION     6
#define   SYMBADDR      8
#define   OPTHEADR     16
#define   FILEFLAG     18

#define   OPTTYPE       0

#define   SECNAME       0
#define   SECPADDR      8
#define   SECVADDR      12
#define   SECSIZE       16
#define   SECRAWDA      20
#define   SECRELDA      24
#define   SECLINDA      28
#define   SECRELNO      32
#define   SECLINNO      34
#define   SECFLAGS      36

#define   FILHDLEN      20
#define   OPTHDLEN      28
#define   SECHDLEN      40
#define   RAWSIZE        4
#define   RELSIZE       10
#define   LINSIZE        6

#define   RETRYCNT      3
#define   WAITCNT       100

#define   FPP_CONFIG    0x00000328              /* FPP bootstrap config data*/

#define   SEG_0_MIN     0x00000000
#define   SEG_0_MAX     0x00001000
#define   SEG_1_MIN     0x00C00000
#define   SEG_1_MAX     0x00C40000

#define   DSP_MC_ADDR   0x00C00000
#define   DSP_ST_ADDR   0x00C10000              /* DSP status block address */
#define   FPP_BS_ADDR   0x00C18000              /* FPP bootstrap address    */
#define   FPP_LD_ADDR   0x00C18042              /* FPP loader address       */
#define   FPP_MC_ADDR   0x00C19000              /* FPP microcode address    */

/****************************************************************************/
/*                         Register Format Definitions                      */
/****************************************************************************/
#define   POS2_DISABLE      0x00
#define   POS2_ENABLE       0x01
#define   POS2_FAIR         0x02
#define   POS2_STREAM       0x04
#define   POS2_RESERVE1     0x08
#define   POS2_PARITY       0x10
#define   POS2_OPTION       0x20
#define   POS2_RESERVE2     0x40
#define   POS2_TIMER        0x80

#define   POS3_INTLVL       0x01
#define   POS3_ARBLVL       0x10

#define   POS5_AUTOINC      0x01
#define   POS5_VPD_0        0x00
#define   POS5_VPD_1        0x02
#define   POS5_VPD_2        0x04
#define   POS5_CHKST_DI     0x40
#define   POS5_CHKCH_DI     0x80

#define   IND_DSPPRO        0x01
#define   IND_IOBPRO        0x02
#define   IND_BIMREG        0x00
#define   IND_DSPMEM        0x04
#define   IND_IOBMEM        0x08
#define   IND_READ          0x10
#define   IND_WRITE         0x00
#define   IND_AUTOINC       0x20
#define   IND_RESET         0x00
#define   IND_RUN           0x40

#define   PCB_FREE          0x10
#define   DSP_COMMO_INT     0x02
#define   PIO_MODE          0x2000

#define   RESET_COMPLETE    0x00010000
#define   CRC_PASS          0x00020000
#define   CRC_FAIL          0x80030000

/****************************************************************************/
/*                         Blit Mode Flag Definitions                       */
/****************************************************************************/
#define   MODE_8_VS_24  0x8000
#define   COLOR_EXPAND  0x1000

/****************************************************************************/
/*                             Macro Definitions                            */
/****************************************************************************/
#define   GET_TIMER(se_data,se_size,t)                             \
  t = 0;                                                           \
  if (se_size > 1)                                                 \
  {                                                                \
    if (se_data[1] > MAX_TIMER)                                    \
      app_error(INV_TIMER,se_data[1],MAX_TIMER);                   \
    t = se_data[1];                                                \
  }
/****************************************************************************/
/*                              Type Definitions                            */
/****************************************************************************/
typedef   struct
{
   int    (*sub)();
   char   *cmd;
   char   *des;
} OPTDEF, *OPTPTR;

typedef   struct
{
   int    mask;
   int    *var;
   int    ival;
   char   *cmd;
   char   *des;
} FLGDEF, *FLGPTR;

typedef   struct
{
   int    flag;
   int    sourcebp;
   int    bflags;
   int    mode;
   char   *filename;
} BLTDEF, *BLTPTR;

typedef   struct
{
  int     addr;
  int     read;
  int     write;
  char    *name;
} REGDEF, *REGPTR;

typedef   struct
{
  int     addr;
  int     read;
  int     write;
  char    *name;
} POSDEF, *POSPTR;

typedef struct
{
   int    type;
   int    fieldlen;
   char   *name;
} STADEF, *STAPTR;

typedef struct
{
   union
   {
      int   tint;
      float tfloat;
      short tshorts[2];
      char  tchars[4];
   } val_union;
} val_t;

typedef struct
{
  ushort   length;                    /* Structure element length             */
  ushort   opcode;                    /* Structure element opcode             */
  ulong    ctx_id;                    /* Context id                           */
  ulong    ctx_adr;                   /* Host address of pinned memory        */
  ulong    ctx_len;                   /* Host length of pinned memory         */
} PINMEM, *PINPTR;

typedef struct
{
  ushort   length;                    /* Structure element length             */
  ushort   opcode;                    /* Structure element opcode             */
  ushort   old_fate;                  /* Old context fate:                    */
                                      /*   0 = no old context                 */
                                      /*   1 = save context on adapter        */
                                      /*   2 = remove ctx to sys with max len */
                                      /*   3 = remove ctx to sys with calc len*/
                                      /*   4 = terminate context              */
  ushort   new_fate;                  /* New context fate:                    */
                                      /*   0 = no new context                 */
                                      /*   1 = initiate new                   */
                                      /*   2 = switch to new                  */
                                      /*   3 = restore new from system        */
  ulong    old_adr;                   /* Old context mem address(remove only) */
  ulong    old_len;                   /* Old context mem length (remove only) */
  ulong    new_id;                    /* New context id                       */
  ulong    new_type;                  /* New context type:                    */
                                      /*   0 = 2d                             */
                                      /*   1 = 3dm1                           */
                                      /*   3 = 3dm1m                          */
  ulong    new_adr;                   /* New context mem address(restore only)*/
  ulong    new_len;                   /* New context mem length (restore only)*/
  ushort   high_water;                /* High water interrupt point           */
  ushort   low_water;                 /* Low water interrupt point            */
} CONTEXT, *CTXPTR;
#endif /* bidefs.h */
