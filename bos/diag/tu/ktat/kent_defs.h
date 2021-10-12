/* @(#)86	1.2  src/bos/diag/tu/ktat/kent_defs.h, tu_ktat, bos41J, 9519A_all 5/3/95 15:02:19  */
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: COPY_NADR
 *		PRINT
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
 *****************************************************************************/

/* TUs                                       */
#define TU_OPEN                0
#define TU_CLOSE              10
#define CONFIG_REG_TEST        1
#define IO_REG_TEST            2
#define INIT_TEST              3
#define INTERNAL_WRAP1         4
#define INTERNAL_WRAP2         5
#define EXTERNAL_WRAP_AUI      6
#define EXTERNAL_WRAP_10Base2  7
#define EXTERNAL_WRAP_10BaseT  8

/* Error codes                                                               */
#define loop_count_was_zero    0xA1
#define INVALID_TU_NUMBER      0xFF
#define TU_SYS                 3
#define GET_BUFS_FAILED_1      0x88000081
#define GET_BUFS_FAILED_2      0x88000082
#define OUT_OF_MEMORY          0x88000083
/*
#ifdef tu_debug
#define PRINT(args) fprintf args
#endif */

#ifdef tu_debug
#define PRINT(args) printf args
#endif

/* Configuration register definitions                                        */

#define cmd_stat_reg          0x04
#define base_addr_reg         0x10
#define int_reg               0x3c
#define ioen_bmen_reset 0xfaffffff
/* #define ioen_bmen_reset 0xfffffffa */
#define ioen_bmen_set   0x05000000
#define ioen_set        0x01000000
#define enst            0x00800000

/* CSR address offsets               */

#define csr0    0x00000000
#define csr1    0x01000000
#define csr2    0x02000000
#define csr3    0x03000000
#define csr4    0x04000000
#define csr6    0x06000000
#define csr7    0x07000000
#define csr8    0x08000000
#define csr9    0x09000000
#define csr10   0x0a000000
#define csr11   0x0b000000
#define csr12   0x0c000000
#define csr13   0x0d000000
#define csr14   0x0e000000
#define csr15   0x0f000000
#define csr16   0x10000000
#define csr17   0x11000000
#define csr18   0x12000000
#define csr19   0x13000000
#define csr20   0x14000000
#define csr21   0x15000000
#define csr22   0x16000000
#define csr23   0x17000000
#define csr24   0x18000000
#define csr25   0x19000000
#define csr26   0x1a000000
#define csr27   0x1b000000
#define csr28   0x1c000000
#define csr29   0x1d000000
#define csr30   0x1e000000
#define csr31   0x1f000000
#define csr32   0x20000000
#define csr33   0x21000000
#define csr34   0x22000000
#define csr35   0x23000000
#define csr36   0x24000000
#define csr37   0x25000000
#define csr38   0x26000000
#define csr39   0x27000000
#define csr40   0x28000000
#define csr41   0x29000000
#define csr42   0x2a000000
#define csr43   0x2b000000
#define csr44   0x2c000000
#define csr45   0x2d000000
#define csr46   0x2e000000
#define csr47   0x2f000000
#define csr58   0x3a000000
#define csr59   0x3b000000
#define csr60   0x3c000000
#define csr61   0x3d000000
#define csr62   0x3e000000
#define csr63   0x3f000000
#define csr64   0x40000000
#define csr65   0x41000000
#define csr66   0x42000000
#define csr67   0x43000000
#define csr72   0x48000000
#define csr74   0x4a000000
#define csr76   0x4c000000
#define csr78   0x4e000000
#define csr80   0x50000000
#define csr82   0x52000000
#define csr84   0x54000000
#define csr85   0x55000000
#define csr86   0x56000000
#define csr88   0x58000000
#define csr89   0x59000000
#define csr92   0x5c000000
#define csr94   0x5e000000
#define csr100  0x64000000
#define csr112  0x70000000
#define csr114  0x72000000
#define csr122  0x7a000000
#define csr124  0x7c000000

#define bcr2    0x02000000
#define bcr4    0x04000000
#define bcr20   0x14000000
#define software_style 0x02010000

#define reset_reg 0x18    /* soft reset by writting this register        */
#define rap       0x14    /* register access port                        */
#define rdp       0x10    /* csr register data port                      */
#define bdp       0x1c    /* bcr register data port                      */
#define aprom_0   0x00    /* offset of high 2 bytes of physical address  */
#define aprom_1   0x04    /* offset of low 4 bytes of physical address   */

#define t_r_len_mode1 0x84002020  /* initial value for tu003             */
#define t_r_len_mode2 0xcc942020  /* internal wrap w/o MENDEC: tu004     */
#define t_r_len_mode3 0xcc902020  /* internal wrap w/  MENDEC: tu005     */
#define t_r_len_mode4 0x04002020  /* external wrap AUI port:   tu006     */
#define t_r_len_mode5 0x84102020  /* external wrap 10BaseT:    tu007     */
#define t_r_len_mode6 0x0c002020  /* external wrap 10BaseT:    tu008     */

#define baset_port 0x00400000
#define aui_port 0x00000000

#define csr6_initial  0x00220000  /* initial value for csr6              */
/*  if the 2020 in t_r_len_mode1 changes then csr6_initial must change   */

#define ladr_lo4  0x01020304
#define ladr_hi4  0x05060708
#define max_desc  4
#define buf_size 0x400

/* values used in tu002 (io register test      */

#define csr0_wdata     0x44ff0000
#define csr0_stop      0x04ff0000
#define csr0_rmsk      0xffff0000
#define csr_rmsk       0xffff0000
#define csr0_expect    0x44000000
#define csr59_expect   0x05010000
#define csr94_expect   0x00000000
#define csr122_expect  0x00000000
#define csr124r1       0x08000000
#define csr124r2       0x00000000


#define start_init     0x01000000
#define start_xmit     0x02000000
#define stop           0x04000000
#define IDON           0x00010000
#define TINT           0x00020000
#define RINT           0x00040000
#define T_RINT         0x00060000
#define hi_msk         0x0000ffff
#define lo_msk         0xffff0000

#define tmd1_set       0x00fc0083
#define tmd2_set       0x00000000
#define rmd1_set       0x00f80080
#define rmd2_set       0x00000000

/* interrupt masks        */

#define INTR_ENABLE 0x0040
#define INTR_MSK    0x0080
#define BABL_INTR   0x4000
#define MISS_INTR   0x1000
#define MERR_INTR   0x0800
#define RINT_INTR   0x0400
#define TINT_INTR   0x0200
#define IODN_INTR   0x0100
#define INIT_TIMEOUT 2
#define XMIT_TIMEOUT 3


/*  macros       */

#define COPY_NADR(a, b) { \
        *((ulong *)(b)) = *((ulong *)(a)); \
        *((ushort *)((char *)(b) + 4)) = *((ushort *)((char *)(a) + 4)); \ 
        }

#define INTERRUPT_HANDLER_PATH  "/usr/lpp/htx/etc/kernext/ktat_intr"

#ifdef DEBUG_1
#define DEBUG1 printf
#else
#define DEBUG1
#endif

