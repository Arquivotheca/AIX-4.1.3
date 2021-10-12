/* @(#)93        1.2  src/bos/diag/tu/artic/artictst.h, tu_artic, bos41J, 9508A 2/15/95 17:50:37 */
/*
 * COMPONENT_NAME:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * FUNCTIONS: Test Unit Header File
 *
 */
/*****************************************************************************

Header HTX/Mfg.  Adapter Definitions File

Module Name :  artictst.h

Header file contains basic definitions needed by three applications for
testing the ARTIC Adapter:

        1)  Hardware exerciser invoked by HTX,
        2)  Manufacturing application invoked by mda
        3)  Diagnostic application invoked by da

*****************************************************************************/

#include <sys/errno.h>
#include <sys/types.h>

/*
 * definition of constant to let exectu() know whether or not
 * it is being invoked from the HTX hardware exerciser.
 */

#define INVOKED_BY_HTX   99


/*
 * Global definitions and limits
 */

/* Maximum number of adapters supported */
#define MAX_CARDS   16

/*
 * Adapter types returned by the device driver
 */

#define         X25                      0
#define         MP2_4P232                1
#define         MP2_6PSYNC               2
#define         MP2_8P232                3
#define         MP2_8P422                4
#define         MP2_4P4224P232           5
#define         MP2_UNRECOG_EIB          99

#define         MPQP                     100
#define         PM_6PV35                 101
#define         PM_6PX21                 102
#define         PM_8P232                 103
#define         PM_8P422                 104
#define         PM_UNRECOG_EIB           199

#define         SP5                      200
#define         SP5_UNRECOG_EIB          299

#define         C1X                      300
#define         MP_UNRECOG_EIB           399

#define         GALE_MPQP                400
#define         GALE_6PV35               401
#define         GALE_6PX21               402
#define         GALE_8P232               403
#define         GALE_8P422               404
#define         GALE_UNRECOG_EIB         499

/*
 * Macros used to determine base type from adapter type
 */
#define         IS_PORTMASTER(x)         ((x>=MPQP)&&(x<=PM_UNRECOG_EIB))
#define         IS_GALE(x)               ((x>=GALE_MPQP)&&(x<=GALE_UNRECOG_EIB))
#define         IS_SP5(x)                ((x>=SP5)&&(x<=SP5_UNRECOG_EIB))
#define         IS_MP2(x)                ((x>=X25)&&(x<=MP2_UNRECOG_EIB))
#define         IS_MP(x)                 ((x>=C1X)&&(x<=MP_UNRECOG_EIB))

/*
 * Offset of error code in Sandpiper V tasks message buffer
 */
#define SP5_STAT_OFFSET  0x066F

/*
 * Used to convert Sandpiper V task error codes to a
 * consistent format (i.e. 0x5XXX)
 */
#define SP5_ERROR_BASE      0x5000

/*****************************************************/
/*   ARTIC Adapter Board Test Units Header File      */
/*****************************************************/

/*
 * Times used to wait for microcode response to tests
 */
#define TWO_MINUTES   120000
#define FIVE_SECONDS    5000

/*
 * Primary Status Byte of Diagnostic microcode
 */
#define TASK_RUNNING  3

/*
 * standard structure used by manufacturing diagnostics.
 */

struct tucb_t
   {
        long tu,        /* test unit number   */
             loop,      /* loop test of tu    */
             mfg;       /* flag = 1 if running mfg. diagnostics, else 0  */

        long r1,        /* reserved */
             r2;        /* reserved */
   };

/*
 * error types and error codes for test units.
 */
#define SYS_ERR     0x00
#define LOG_ERR     0x02

struct _artic_info
   {
        int reserved;
   };

/*
 * definition of structure passed by BOTH hardware exerciser and
 * diagnostics to "exectu()" function for invoking
 * test units.
 */
#define TUTYPE struct _artic_tu

struct _artic_tu
   {
        struct tucb_t header;
        int mdd_fd;
        int slot;
        struct _artic_info artic_info;
   };

/******************************************************************************
*                                                                             *
*  Device driver interface structures and function prototypes                 *
*                                                                             *
******************************************************************************/
/* This structure contains configuration information about      */
/* a specific adapter                                           */

typedef struct
{
        ushort io_addr;                /* Base address of adapter's I/O ports */
        uchar maxtask;                 /* Max task number for adapter         */
        uchar maxpri;                  /* Number of task priorities           */
        uchar maxqueue;                /* Max number of queues for adapter    */
        uchar maxtime;                 /* Max number of timers for adapter    */
        uchar int_level;               /* Adapters interrupt level            */
        uchar ssw_size;                /* Size of the shared storage window   */
} ICAPARMS;

/* This structure contains the length and address of an         */
/* input, output or secondary status buffer                                     */

typedef struct
{
        ushort length;                  /* Length of buffer             */
        ushort offset;                  /* Offset of buffer's address   */
        uchar page;                     /* Page of buffer's address     */
} ICABUFFER;

/* Function Prototypes for C Library Routines */

extern unsigned short  icareset();
extern unsigned short  icareadmem();
extern unsigned short  icawritemem();
extern unsigned short  icaintreg();
extern unsigned short  icaintwait();
extern unsigned short  icaintdereg();
extern unsigned short  icaissuecmd();
extern unsigned short  icagetparms();
extern unsigned short  icagetbuffers();
extern unsigned short  icasecstatbuf();
extern unsigned short  icainbuf();
extern unsigned short  icaoutbuf();
extern unsigned short  icasendconfig();
extern unsigned short  icaioread();
extern unsigned short  icaiowrite();
extern unsigned short  icaposread();
extern unsigned short  icaposwrite();
extern unsigned short  icagetprimstat();
extern unsigned short  icagetadaptype();
extern unsigned short  icadmasetup();
extern unsigned short  icadmarel();

/******************************************************************************
*                                                                             *
*  INT0 Command Codes                                                         *
*                                                                             *
******************************************************************************/
#define START_TASK_0        5      /* Start Task 0 command code */

/******************************************************************************
*                                                                             *
*  uCode  Registers                                                           *
*                                                                             *
******************************************************************************/
#define TASK_SEGMENT_LOW    (unsigned long)0x42E
#define TASK_SEGMENT_HIGH   (unsigned long)0x42F
#define TASK_COMMAND        (unsigned long)0x430
#define TASK_ERROR_STATUS   (unsigned long)0x431
#define TASK_NUMBER         (unsigned long)0x440
#define INT_STATUS          (unsigned long)0x441
#define TASK_NUM_2          (unsigned long)0x484
#define M2_MIF              (unsigned long)0x525 /* TASK command pending */
#define PM_MIF              (unsigned long)0x1C5
/******************************************************************************
*                                                                             *
*  Loadable Microcode Command Codes                                           *
*                                                                             *
******************************************************************************/
#define CPU_COM_CODE            0x00
#define RAM_COM_CODE            0x01
#define GATEARRAY_COM_PCODE     0x02
#define CIO_COM_PCODE           0x03
#define GATEARRAY_COM_CODE     0x03
#define CIO_COM_CODE           0x02
#define SCC_COM_CODE            0x04
#define SP5_COM_CODE            0x28
#define SP5STAT_COM_CODE        0x29

/******************************************************************************
*                                                                             *
*  Loadable Microcode Command Codes (DIATSK)  (Multiport/2 specific codes)    *
*                                                                             *
******************************************************************************/
#define WRAPCIO78A_COM_CODE     0x05
#define WRAPCIO78B_COM_CODE     0x06
#define WRAP_P0_COM_CODE        0x07
#define WRAP_P1_COM_CODE        0x08
#define WRAP_P2_COM_CODE        0x09
#define WRAP_P3_COM_CODE        0x0A
#define WRAP_P4_COM_CODE        0x0B
#define WRAP_P5_COM_CODE        0x0C
#define WRAP_P6_COM_CODE        0x0D
#define WRAP_P7_COM_CODE        0x0E
#define COUNTERA_COM_CODE       0x0F
#define COUNTERB_COM_CODE       0x10
/******************************************************************************
*                                                                             *
*  Loadable Microcode Command Codes (DIATSK)  (X25 specific codes)            *
*                                                                             *
******************************************************************************/
#define WRAPV24_COM_CODE        0x11
#define WRAPV35_COM_CODE        0x12
#define WRAPX21_COM_CODE        0x13
#define WRAPCIOV35_COM_CODE     0x15

/******************************************************************************
*                                                                             *
*  Loadable Microcode Command Codes (PM_DIATSK)  (Portmaster specific codes)  *
*                                                                             *
******************************************************************************/
#define WRAP_P0_COM_PCODE        0x05
#define WRAP_P1_COM_PCODE        0x06
#define WRAP_P2_COM_PCODE        0x07
#define WRAP_P3_COM_PCODE        0x08
#define WRAP_P4_COM_PCODE        0x09
#define WRAP_P5_COM_PCODE        0x0A
#define WRAP_P6_COM_PCODE        0x0B
#define WRAP_P7_COM_PCODE        0x0C
/******************************************************************************
*                                                                             *
*  Loadable Microcode Command Codes (PM_DIATSK)                              *
*                    (Portmaster 4-Port Selectable specific codes)            *
*                                                                             *
******************************************************************************/
#define SEL_RELAY_COM_CODE         0x0D
#define SEL_WRAPRS232_P0_COM_CODE  0x0E
#define SEL_WRAPRS422_P0_COM_CODE  0x0F
#define SEL_WRAPX21_P0_COM_CODE    0x10
#define SEL_WRAPV35_P0_COM_CODE    0x11
#define SEL_WRAPRS232_P1_COM_CODE  0x12
#define SEL_WRAPV35_P1_COM_CODE    0x13
#define SEL_WRAPRS232_P2_COM_CODE  0x14
#define SEL_WRAPRS422_P2_COM_CODE  0x15
#define SEL_WRAPRS232_P3_COM_CODE  0x16
#define SEL_CABLERS232_P0_COM_CODE 0x17
#define SEL_CABLERS422_P0_COM_CODE 0x18
#define SEL_CABLEX21_P0_COM_CODE   0x19
#define SEL_CABLEV35_P0_COM_CODE   0x1A
#define SEL_CABLERS232_P1_COM_CODE 0x1B
#define SEL_CABLEV35_P1_COM_CODE   0x1C
#define SEL_CABLERS232_P2_COM_CODE 0x1D
#define SEL_CABLERS422_P2_COM_CODE 0x1E
#define SEL_CABLERS232_P3_COM_CODE 0x1F

/******************************************************************************
*                                                                             *
*  Test unit number for SP5 DSP test                                          *
*                                                                             *
******************************************************************************/
#define SP5DSP_TEST             71

/******************************************************************************
* Adapter I/O port structure                                                  *
******************************************************************************/
#define adapter_LOCREG0   0x0 /* 0 location register ls part */
#define adapter_LOCREG1   0x1 /* 1 location register ms part */
#define adapter_PTRREG    0x2 /* 2 pointer register          */
#define adapter_DREG      0x3 /* 3 data register             */
#define adapter_TREG      0x4 /* 4 task register             */
#define adapter_CPUPG     0x5 /* 5 page number of window into RAM */
#define adapter_COMREG    0x6 /* 6 command register          */

#define indirect_INITREG2 0x08
#define indirect_INTCOM   0x09
#define indirect_PAR0     0x0A
#define indirect_PAR1     0x0B
#define indirect_CAD0     0x0C
#define indirect_CAD1     0x0D
#define indirect_CAD2     0x0E
#define indirect_GAID     0x0F
#define indirect_INITREG1 0x10
#define indirect_PAR2     0x11
#define indirect_INITREG0 0x12
#define indirect_INITREG3 0x13

/******************************************************************************
* Error codes                                                                 *
******************************************************************************/
#define LRAM            0x11
#define HRAM            0x12
#define POSTER          0x13
#define P_CPU_ER        0x14
#define P_ROS_CK_ER     0x15
#define P_CIO_ER        0x16
#define P_SCC_ER        0x17
#define P_GA_ER         0x18
#define P_PARITY_ER     0x19
#define P_DMA_ER        0x1A
#define POSTTIMEOUT     0x1B
#define POS_ER          0x21
#define INT_ER          0x31
#define CPU_ER          0x41
#define LDRAM_ER        0x51
#define HDRAM_ER        0x52
#define GATE_ER         0x61
#define GATEP_ER        0x61
#define CIO_ER          0x71
#define SCC_ER          0x81
#define WRAP_INT        0x91
#define SP5_ER          0xA1
#define WRAP_BASE       0xA1
#define WX21_37_ER      0xA1
#define WV24_37_ER      0xA2
#define WV35_37_ER      0xA3
#define WP0_78_ER       0xA4
#define WP1_78_ER       0xA5
#define VPD_ER          0xB1
#define WP0_ER          0xB2
#define WP1_ER          0xB3
#define WP2_ER          0xB4
#define WP3_ER          0xB5
#define WP4_ER          0xB6
#define WP5_ER          0xB7
#define WP6_ER          0xB8
#define WP7_ER          0xB9
#define WX21_ER         0xB1
#define WV24_ER         0xC1
#define SEL_WP0232_ER   0xC2
#define SEL_WP0422_ER   0xC3
#define SEL_WP0X21_ER   0xC4
#define SEL_WP0V35_ER   0xC5
#define SEL_WP1232_ER   0xC6
#define SEL_WP1V35_ER   0xC7
#define SEL_WP2232_ER   0xC8
#define SEL_WP2422_ER   0xC9
#define SEL_WP3232_ER   0xCA
#define WV35_ER         0xD1
#define SEL_WCP0232_ER  0xD2
#define SEL_WCP0422_ER  0xD3
#define SEL_WCP0X21_ER  0xD4
#define SEL_WCP0V35_ER  0xD5
#define SEL_WCP1232_ER  0xD6
#define SEL_WCP1V35_ER  0xD7
#define SEL_WCP2232_ER  0xD8
#define SEL_WCP2422_ER  0xD9
#define SEL_WCP3_ER     0xDA
#define WCX21_ER        0xE1
#define WCV24_ER        0xF1
#define WCV35_ER        0x101
#define COMREG_ER       0x125
#define LUCODE_ER       0x131
#define COUNTA_ER       0x135
#define COUNTB_ER       0x136
#define SEL_RELAY_ER    0x300
#define DDGRAM1         0x0321
#define DDGRAM2         0x0322
#define DDINT1          0x0331
#define DDINPB1         0x0341
#define DDINPB2         0x0342
#define DDINPB3         0x0343
#define DDINPB4         0x0344
#define DDINPB5         0x0345
#define DDINPB6         0x0346
#define DDINPB7         0x0347
#define DDINPB8         0x0348
#define DDINPB9         0x0349
#define DDINPR1         0x0351
#define DDINPR2         0x0352
#define DDINPR3         0x0353
#define DDINPR4         0x0354
#define DDINPR5         0x0355
#define DDINPR6         0x0356
#define DDINPR7         0x0357
#define DDINPR8         0x0358
#define READ_ER1        0x0361
#define GATE_RAM        0x0386
#define GATE_RAM1       0x0387
#define GATE_RAM2       0x0388
#define RESET_RAM_TIMEOUT 0x391
#define RESET_ROS_TIMEOUT 0x392
#define RESET_RCP_TIMEOUT 0x393
#define REG_ERR1        0x0401
#define REG_ERR2        0x0402
#define REG_ERR3        0x0403
#define REG_ERR4        0x0404
#define REG_ERR5        0x0405
#define REG_ERR6        0x0406
#define REG_ERR7        0x0407
#define REG_ERR8        0x0408
#define REG_ERR9        0x0409
#define REG_ERR10       0x0410
#define REG_ERR11       0x0411
#define REG_ERR12       0x0412
#define REG_ERR13       0x0413
#define REG_ERR14       0x0414
#define REG_ERR15       0x0415
#define REG_ERR16       0x0416
#define REG_ERR17       0x0417
#define CAD_ER1         0x0501
#define CAD_ER2         0x0502
#define CAD_ER3         0x0503
#define CAD_ER4         0x0504
#define CAD_ER5         0x0505
#define CAD_ER6         0x0506
#define CAD_ER7         0x0507
#define CAD_ER8         0x0508
#define CAD_ER9         0x0509
#define CAD_ER10        0x0510
#define CAD_ER11        0x0511
#define CAD_ER12        0x0512
#define CAD_ER13        0x0513
#define NOUCODE         0x0601
#define NOUCODE2        0x0602
#define PIN_ERR         0x0800
#define UNPIN_ERR       0x0801
#define MALLOC_ERR      0x0802
#define TUTCTO          0x8888
#define ILLEGAL_EIB_PORT  0xEEEE
#define REG_ERR         0xFFFF
#define DRV_ERR         0xFFFE
#define ILLEGAL_TU_ERR  0x9999

/*****************************************************************************
*   Begin defines for Sandpiper 5 Error Codes       rcb - 10/05/92
******************************************************************************/
#define SP_RAM          0x5100
#define SP_INT5         0x5205
#define SP_INT6         0x5206
#define SP_INT_TIMEOUT  0x5301
#define SP_INT_DATA_ERR 0x5302
#define SP_DMA_1TIMEOUT 0x5401
#define SP_DMA_1DATAERR 0x5402
#define SP_DMA_MULT_WRT 0x5403
#define SP_DMA_MULT_RD  0x5404
#define SP_DMA_M_DATERR 0x5405
#define SP_HWA_DAT_ERR  0x5500
#define SP_HWA_BIT_INS  0x5502
#define SP_HWA_PROTOCOL 0x5504
#define SP_HWA_CRC      0x5506
#define SP_HWA_BR_CRC   0x550C
#define SP_INT4         0x5600
#define SP_T_SLOT_ENA   0x5701
#define SP_T1_CLR_ERR   0x5702
#define SP_T1_SIG_RAM   0x5705
#define SP_T1_TX_RX     0x5709
#define SP_T1_TX_FIFO   0x570A
#define SP_T1_RX_FIFO   0x570B
#define SP_T1_SIGTX_DATA 0x5715
#define SP_T1_SIGRX_DATA 0x5717
#define SP_DSP_COMP1    0x5801
#define SP_DSP_COMP2    0x5802
#define SP_DSP_COMP3    0x5803
#define SP_DSP_COMP4    0x5804
#define SP_DSP_COMP5    0x5805
#define SP_DSP_COMP6    0x5806
#define SP_DSP_COMP7    0x5807
#define SP_DSP_COMP8    0x5808
#define SP_DSP_COMP9    0x5809
#define SP_DSP_COMPA    0x580A
#define SP_DSP_COMPB    0x580B
#define SP_DSP_COMPC    0x580C
#define SP_DSP_COMPD    0x580D
#define SP_DSP_COMPE    0x580E
#define SP_DSP_COMPF    0x580F
#define SP_DSP_COMP10   0x5810
#define SP_DSP_COMP11   0x5811
#define SP_DSP_COMP12   0x5812
#define SP_DSP_COMP13   0x5813
#define SP_DSP_COMP14   0x5814
#define SP_DSP_COMP15   0x5815
#define SP_DSP_COMP16   0x5816
#define SP_DSP_COMP17   0x5817
#define SP_DSP_COMP18   0x5818
#define SP_DSP_COMP19   0x5819
#define SP_DSP_COMP1A   0x581A
#define SP_DSP_R0INDX   0x581B
#define SP_DSP_R4INDX   0x581C
#define SP_DSP_LDHALF   0x581D
#define SP_DSP_INBYTE   0x581E
#define SP_DSP_CMSKHI   0x581F
#define SP_DSP_CMSKBOTH 0x5820
#define SP_DSP_ICMLO    0x5821
#define SP_DSP_LCMBOTH  0x5822
#define SP_DSP_LCMLO    0x5823
#define SP_DSP_CIRC01   0x5824
#define SP_DSP_CIRC02   0x5825
#define SP_DSP_CIRC03   0x5826
#define SP_DSP_CIRC04   0x5827
#define SP_DSP_CIRC05   0x5828
#define SP_DSP_CIRC06   0x5829
#define SP_DSP_CIRC07   0x582A
#define SP_DSP_CIRC08   0x582B
#define SP_DSP_CIRC09   0x582C
#define SP_DSP_CIRC0A   0x582D
#define SP_DSP_CIRC0B   0x582E
#define SP_DSP_CIRC0C   0x582F
#define SP_DSP_CIRC0D   0x5830
#define SP_DSP_CIRC0E   0x5831
#define SP_DSP_CIRC0F   0x5832
#define SP_DSP_CIRC10   0x5833
#define SP_DSP_CIRC11   0x5834
#define SP_DSP_CIRC12   0x5835
#define SP_DSP_BRCH01   0x5836
#define SP_DSP_BRCH02   0x5837
#define SP_DSP_BRCH03   0x5838
#define SP_DSP_BRCH04   0x5839
#define SP_DSP_BRCH05   0x583A
#define SP_DSP_BRCH06   0x583B
#define SP_DSP_BRCH07   0x583C
#define SP_DSP_BRCH08   0x583D
#define SP_DSP_BRCH09   0x583E
#define SP_DSP_BRCH0A   0x583F
#define SP_DSP_BRCH0B   0x5840
#define SP_DSP_BRCH0C   0x5841
#define SP_DSP_BRCH0D   0x5842
#define SP_DSP_BRCH0E   0x5843
#define SP_DSP_BRCH0F   0x5844
#define SP_DSP_BRCH10   0x5845
#define SP_DSP_BRCH11   0x5846
#define SP_DSP_BRCH12   0x5847
#define SP_DSP_BRCH13   0x5848
#define SP_DSP_BRCH14   0x5849
#define SP_DSP_BRCH15   0x584A
#define SP_DSP_BRCH16   0x584B
#define SP_DSP_BRCH17   0x584C
#define SP_DSP_BRCH18   0x584D
#define SP_DSP_BRCH19   0x584E
#define SP_DSP_BRCH1A   0x584F
#define SP_DSP_BRCH1B   0x5850
#define SP_DSP_BRCH1C   0x5851
#define SP_DSP_BRCH1D   0x5852
#define SP_DSP_BRCH1E   0x5853
#define SP_DSP_BRCH1F   0x5854
#define SP_DSP_BRCH20   0x5855
#define SP_DSP_BRCH21   0x5856
#define SP_DSP_BRCH22   0x5857
#define SP_DSP_BRCH23   0x5858
#define SP_DSP_BRCH24   0x5859
#define SP_DSP_BRCH25   0x585A
#define SP_DSP_BRCH26   0x585B
#define SP_DSP_BRCH27   0x585C
#define SP_DSP_BRCH28   0x585D
#define SP_DSP_BRCH29   0x585E
#define SP_DSP_BRCH2A   0x585F
#define SP_DSP_BRCH2B   0x5860
#define SP_DSP_BRCH2C   0x5861
#define SP_DSP_BRCH2D   0x5862
#define SP_DSP_BRCH2E   0x5863
#define SP_DSP_BRCH2F   0x5864
#define SP_DSP_BRCH30   0x5865
#define SP_DSP_BRCH31   0x5866
#define SP_DSP_BRCH32   0x5867
#define SP_DSP_BRCH33   0x5868
#define SP_DSP_BRCH34   0x5869
#define SP_DSP_BRCH35   0x586A
#define SP_DSP_BRCH36   0x586B
#define SP_DSP_BASINT   0x586C
#define SP_DSP_PRIINT   0x586D
#define SP_DSP_INHINT   0x586E
#define SP_DSP_RSTINT   0x586F
#define SP_DSP_RML      0x5870
#define SP_DSP_RMM      0x5871
#define SP_DSP_RMM1     0x5872
#define SP_DSP_RMH      0x5873
#define SP_DSP_RMH1     0x5874
#define SP_DSP_SPSR     0x5875
#define SP_DSP_SPSR1    0x5876
#define SP_DSP_SPSR2    0x5877
#define SP_DSP_SPSR3    0x5878
#define SP_DSP_RMX1     0x5879
#define SP_DSP_RMX2     0x587A
#define SP_DSP_RMX3     0x587B
#define SP_DSP_RMX4     0x587C
#define SP_DSP_RMX5     0x587D
#define SP_DSP_RMX6     0x587E
#define SP_DSP_RMX7     0x587F
#define SP_DSP_RMX8     0x5880
#define SP_DSP_RMX9     0x5881
#define SP_DSP_RMXA     0x5882
#define SP_DSP_RMXB     0x5883
#define SP_DSP_RMXC     0x5884
#define SP_DSP_UXINT0   0x5890
#define SP_DSP_EINT1    0x5891
#define SP_DSP_EINT2    0x5892
#define SP_DSP_EINT3    0x5893
#define SP_DSP_EINT4    0x5894
#define SP_DSP_EINT5    0x5895
#define SP_DSP_EINT6    0x5896
#define SP_DSP_EINT7    0x5897
#define SP_DSP_ISTRP    0x5898
#define SP_DSP_ALUOF    0x5899
#define SP_DSP_MULTPOF  0x589A
#define SP_DSP_DSIOPTY  0x589B
#define SP_DSP_IOPTY    0x589C
#define SP_DSP_DSPTY    0x589D
#define SP_DSP_UXPOR    0x589E
#define SP_DSP2_DATA    0x5950
#define SP_DSP2_DATAIB  0x5952
#define SP_DSP2_PROTO   0x5954
#define SP_DSP2_CRC     0x5956
#define SP_DSP2_BRCRC   0x595C
#define SP_RAS1         0x5F01
#define SP_RAS2         0x5F02
#define SP_RAS3         0x5F03
#define SP_RAS4         0x5F04
#define SP_RAS15        0x5F15
#define SP_RAS25        0x5F25
#define SP_RAS6         0x5F06
#define SP_RAS7         0x5F07
#define SP_RAS08        0x5F08
#define SP_RAS18        0x5F18
#define SP_RAS28        0x5F28
#define SP_RAS38        0x5F38
#define SP_RAS48        0x5F48
#define SP_RAS09        0x5F09
#define SP_RAS2A        0x5F2A
#define SP_RAS3A        0x5F3A
#define SP_RAS4A        0x5F4A
#define SP_RAS0B        0x5F0B
#define SP_RAS0C        0x5F0C
#define SP_RAS1C        0x5F1C
#define SP_RAS0D        0x5F0D
#define SP_RAS0E        0x5F0E
#define SP_TIMEOUT      0x5F00

#define UNKNOWN_SP5_ERR 0x5FFF


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * EIBs identified and used in the the ARTIC diagnostics.    *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* CF   Multiport/2  w/RS232  6-port                         *
 * C7   Multiport/2  w/RS232  8-port and 4-port              *
 * BE   Multiport/2  w/RS422  8-port                         *
 * C8   Multiport/2  w/4P-RS232 and 4P-RS422                 *
 * 21   Portmaster   w/RS422  8-port                         *
 * 10   Portmaster   w/RS232  8-port                         *
 * 18   Portmaster   w/V.35   6-port                         *
 * BF   Portmaster   4 port selectable                       *
 * C9   X25                                                  */

