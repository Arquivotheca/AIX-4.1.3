/* @(#)50       1.12  src/bos/diag/tu/sun/error.h, tu_sunrise, bos411, 9437A411a 9/13/94 08:42:40 */
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: DEBUG_0
 *              DEBUG_1
 *              DEBUG_2
 *              DEBUG_3
 *              DEBUG_4
 *              DEBUG_5
 *              DEBUG_6
 *              DEBUG_7
 *              DEBUG_8
 *              DEBUG_9
 *              LOG_COMP_ERROR
 *              LOG_CRC_ERROR
 *              LOG_HWINIT_ERROR
 *              LOG_MEM_ERROR
 *              LOG_POS_ERROR
 *              LOG_REGISTER_ERROR
 *              LOG_VIDSETUP_ERROR
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

/****************************************************************************
        File:   error.h
*****************************************************************************/

/* This file contains ALL the error return codes for SUNRISE Test Units */

#define         OK              0x0

/*  Return codes for Sunrise Initialization */
#define         ERRORHW_INIT            0x010
#define         ERRORGETSUN             0x020
#define         ERRORHW_CLEAN           0x030

/*  COMMON Return codes */
#define         ERRORPIO_MCWRITE        0x050
#define         ERRORPIO_MCREAD         0x060
#define         ERRORPIO_WRITE          0x070
#define         ERRORPIO_READ           0x080
#define         ERRORINVALID_ID         0x090
#define         NO_CODEC                0x099   /* CODEC card not present */

/* Error codes returned from 'sun_exectu.c' */
#define LOOP_COUNT_WAS_ZERO     0x0A0
#define INVALID_TU_NUMBER       0x0FF

/*  Return codes for POS registers test */
#define         ERRORPOSRW      0x110
#define         ERRORPOSSAVE    0x120
#define         ERRORPOSRESTORE 0x130
#define         ERRORPOS0       0x100
#define         ERRORPOS1       0x101
#define         ERRORPOS2       0x102
#define         ERRORPOS3       0x103
#define         ERRORPOS4       0x104
#define         ERRORPOS5       0x105
#define         ERRORPOS6       0x106
#define         ERRORPOS7       0x107

/*  Return codes for base VPD test */
#define         ERRORVPD_BASE   0x210
/*
#define         ERRORVPD_NOVPD  0x201
#define         ERRORVPD_CRC    0x202
*/

/*  Return codes for MIAMI registers test */
#define         ERRORMIAMI_WRITE        0x310
#define         ERRORMIAMI_READ         0x320
#define         ERRORMIAMI_REGCMP       0x330

/*  Return codes for PIXEL registers test */
#define         ERROR2070_HIUREGS       0x410
#define         ERROR2070_VIUREGS       0x420
#define         ERROR2070_VSUREGS       0x430
#define         ERROR2070_VPUREGS       0x440
#define         ERROR2070_IPU1REGS      0x450
#define         ERROR2070_IPU2REGS      0x460
#define         ERROR2070_OPUREGS       0x470
#define         ERROR2070_MMUREGS       0x480
#define         ERROR2070_OBUREGS       0x490
#define         ERROR2070_SIUREGS       0x4A0
#define         ERROR2070_ALUREGS       0x4B0  /* Not use */
#define         ERROR2070_DWUREGS       0x4C0  /* Not use */

/* Return codes for Video Setup test */
#define         ERRORVID_INIT8584       0x510
#define         ERRORVID_LOAD7191       0x520
#define         ERRORVID_LOAD7199       0x530
#define         ERRORVID_CHKODD         0x540
#define         ERRORVID_CHKVSYNC       0x550
#define         ERRORVID_I2CBUSY        0x501
#define         ERRORVID_I2CACK         0x502
#define         ERRORVID_I2CXMIT        0x503
#define         ERRORVID_TIMEOUT0       0x504
#define         ERRORVID_TIMEOUT1       0x505

/* Return codes for Frame Buffer test */
#define         ERRORFB_2070INIT        0x610
#define         ERRORFB_TEST1           0x620
#define         ERRORFB_TEST2           0x630
#define         ERRORFB_TEST3           0x640

/* Return codes for DMA test */
#define         ERRORDMA_2070INIT       0x710
#define         ERRORDMA_WRITEDATA      0x720
#define         ERRORDMA_ALLOCATE       0x730
#define         ERRORDMA_READDATA       0x740
#define         ERRORDMA_COMPARE        0x750
#define         ERRORDMA_WRITEZERO      0x760
#define         ERRORDMA_HALFWORDSWAP   0x770
#define         ERRORDMA_NOSWAP         0x780
#define         ERRORDMA_MALLOC         0x701
#define         ERRORDMA_SETUP          0x702
#define         ERRORDMA_PXWRITE        0x703
#define         ERRORDMA_PXREAD         0x704
#define         ERRORDMA_START          0x705
#define         ERRORDMA_WAIT4INTR      0x706
#define         ERRORDMA_COMPLETE       0x707

/* Return codes for scaler test */
#define         ERRORSCALE_2070INIT     0x810
#define         ERRORSCALE_WRITEDATA    0x820
#define         ERRORSCALE_WRITEZERO    0x830
#define         ERRORSCALE_INIT         0x840
#define         ERRORSCALE_ALLOCATE     0x850
#define         ERRORSCALE_READDATA     0x860
#define         ERRORSCALE_CRCCHK       0x870

/*  Return codes for CODEC VPD test */
#define         ERRORVPD_CODEC  0x910
/*
#define         ERRORVPD_NOVPD  0x901
#define         ERRORVPD_CRC    0x902
*/

/*  Return codes for CODEC registers test */
#define         ERROR560_HUFFYACREGS    0xA10
#define         ERROR560_HUFFYDCREGS    0xA20
#define         ERROR560_HUFFCACREGS    0xA30
#define         ERROR560_HUFFCDCREGS    0xA40
#define         ERROR560_QUANTREGS      0xA50
#define         ERROR560_FIFOREGS       0xA60    /* Not use */
#define         ERROR560_ADDRTEST       0xA01
#define         ERROR560_PATTEST        0xA02
#define         ERROR560_FIFOCOUNT      0xA03    /* Not use */

/* Return codes for FIELD MEMORY test */
#define         ERRORFMEM_VIDSETUP      0xB10
#define         ERRORFMEM_2070INIT      0xB20
#define         ERRORFMEM_WRITEDATA     0xB30
#define         ERRORFMEM_WRITEZERO     0xB40
#define         ERRORFMEM_V1OUT         0xB50
#define         ERRORFMEM_STOREFIELD    0xB60
#define         ERRORFMEM_FREEZEFIELD   0xB70
#define         ERRORFMEM_DISPLAYFIELD  0xB80
#define         ERRORFMEM_V1IN          0xB90
#define         ERRORFMEM_ALLOCATE      0xBA0
#define         ERRORFMEM_READDATA      0xBB0
#define         ERRORFMEM_COMPARE       0xBC0

/* Return codes for COMPRESSION test */
#define         ERRORCOMP_VIDSETUP      0xC10
#define         ERRORCOMP_WRCTRL        0xC20
#define         ERRORCOMP_WRITEDATA     0xC30
#define         ERRORCOMP_V1OUT         0xC40
#define         ERRORCOMP_STOREFIELD    0xC50
#define         ERRORCOMP_FREEZEFIELD   0xC60
#define         ERRORCOMP_560TO2070     0xC70
#define         ERRORCOMP_COMPRESS      0xC80
#define         ERRORCOMP_2070INIT      0xC90
#define         ERRORCOMP_ALLOCATE      0xCA0
#define         ERRORCOMP_READDATA      0xCB0
#define         ERRORCOMP_CRCCHK        0xCC0
#define         ERRORCOMP_SETUP         0xC01
#define         ERRORCOMP_STATUSTO      0xC02
#define         ERRORCOMP_VSYNCTO       0xC03
#define         ERRORCOMP_FIFOTO        0xC04
#define         ERRORCOMP_STOPTO        0xC05

/* Return codes for DECOMPRESSION test */
#define         ERRORDECOMP_VIDSETUP     0xD10
#define         ERRORDECOMP_LOADDATA     0xD20
#define         ERRORDECOMP_2070INIT     0xD30
#define         ERRORDECOMP_560FROM2070  0xD40
#define         ERRORDECOMP_DECOMP       0xD50
#define         ERRORDECOMP_DISPLAYFIELD 0xD60
#define         ERRORDECOMP_V1IN         0xD70
#define         ERRORDECOMP_ALLOCATE     0xD80
#define         ERRORDECOMP_READDATA     0xD90
#define         ERRORDECOMP_CRCCHK       0xDA0
#define         ERRORDECOMP_FOPEN        0xD01
#define         ERRORDECOMP_MALLOC       0xD02
#define         ERRORDECOMP_WRITEDATA    0xD03
#define         ERRORDECOMP_SETUP        0xD04
#define         ERRORDECOMP_FILLFIFO     0xD05
#define         ERRORDECOMP_TIMEOUT      0xD06

/* Return codes for EMC DMA test */
#define         ERROREMCDMA_VIDSETUP       0xE10
#define         ERROREMCDMA_2070INIT       0xE20
#define         ERROREMCDMA_WRITEDATA      0xE30
#define         ERROREMCDMA_V1OUT          0xE40
#define         ERROREMCDMA_ALLOCATE       0xE50
#define         ERROREMCDMA_READDATA       0xE60
#define         ERROREMCDMA_CRCCHK         0xE70

#define LOG_POS_ERROR(ERROR_CODE, DGX_RC, ADDR, DATA)           \
       error.io_readwrite.error_code = ERROR_CODE;              \
       error.io_readwrite.io_dgx_rc = DGX_RC;                   \
       error.io_readwrite.address = ADDR;                       \
       error.io_readwrite.data = DATA;                          \
       log_error(error);                                        \
       return(ERROR_CODE)

#define LOG_REGISTER_ERROR(ERROR_CODE, SUB_ERROR_CODE, EX_DATA, AC_DATA, ADDR)\
       error.register_test.error_code = ERROR_CODE;              \
       error.register_test.sub_error_code = SUB_ERROR_CODE;      \
       error.register_test.expected_data = EX_DATA;              \
       error.register_test.actual_data = AC_DATA;                \
       error.register_test.address = ADDR;                       \
       log_error(error);                                         \
       return(ERROR_CODE)

#define LOG_HWINIT_ERROR(ERROR_CODE, SYSTEM_ERROR, DIAGEX_ERROR) \
       error.hw_init.error_code = ERROR_CODE;  \
       error.hw_init.system_error_code = SYSTEM_ERROR;  \
       error.hw_init.diagex_error_code = DIAGEX_ERROR;  \
       log_error(error);                                \
       return(ERROR_CODE)

#define LOG_VIDSETUP_ERROR(ERROR_CODE, SUB_ERROR_CODE)           \
       error.vidsetup_test.error_code = ERROR_CODE;              \
       error.vidsetup_test.sub_error_code = SUB_ERROR_CODE;      \
       log_error(error);                                         \
       return(ERROR_CODE)

#define LOG_MEM_ERROR(ERROR_CODE, SUB_ERROR_CODE, EX_DATA, AC_DATA, ADDR)\
       error.mem_test.error_code = ERROR_CODE;              \
       error.mem_test.sub_error_code = SUB_ERROR_CODE;      \
       error.mem_test.expected_data = EX_DATA;              \
       error.mem_test.actual_data = AC_DATA;                \
       error.mem_test.buffer_address = ADDR;                \
       log_error(error);                                    \
       return(ERROR_CODE)

#define LOG_CRC_ERROR(ERROR_CODE, SUB_ERROR_CODE, EX_DATA, AC_DATA)\
       error.crc.error_code = ERROR_CODE;              \
       error.crc.sub_error_code = SUB_ERROR_CODE;      \
       error.crc.crc_expected = EX_DATA;               \
       error.crc.crc_actual = AC_DATA;                 \
       log_error(error);                               \
       return(ERROR_CODE)

#define LOG_COMP_ERROR(ERROR_CODE, SUB_ERROR_CODE, EX_CRC, AC_CRC, EX_CNT, AC_CNT)\
       error.comp.error_code = ERROR_CODE;              \
       error.comp.sub_error_code = SUB_ERROR_CODE;      \
       error.comp.crc_expected = EX_CRC;               \
       error.comp.crc_actual = AC_CRC;                 \
       error.comp.cnt_expected = EX_CNT;               \
       error.comp.cnt_actual = AC_CNT;                 \
       log_error(error);                               \
       return(ERROR_CODE)

/*
 * DEBUGGING AIDS
 */
#undef DEBUG_SUN

#ifdef DEBUG_SUN
#include <stdio.h>
#define DEBUG_0(A)                      {fprintf(stderr,A);fflush(stderr);}
#define DEBUG_1(A,B)                    {fprintf(stderr,A,B);fflush(stderr);}
#define DEBUG_2(A,B,C)                  {fprintf(stderr,A,B,C);fflush(stderr);}
#define DEBUG_3(A,B,C,D)                {fprintf(stderr,A,B,C,D);fflush(stderr);}
#define DEBUG_4(A,B,C,D,E)              {fprintf(stderr,A,B,C,D,E);fflush(stderr);}
#define DEBUG_5(A,B,C,D,E,F)            {fprintf(stderr,A,B,C,D,E,F);fflush(stderr);}
#define DEBUG_6(A,B,C,D,E,F,G)          {fprintf(stderr,A,B,C,D,E,F,G);fflush(stderr);}
#define DEBUG_7(A)                      {printf(A);}
#define DEBUG_8(A,B)                    {printf(A,B);}
#define DEBUG_9(A,B,C)                  {printf(A,B,C);}
#define DEBUGELSE                       else
#else
#define DEBUG_0(A)
#define DEBUG_1(A,B)
#define DEBUG_2(A,B,C)
#define DEBUG_3(A,B,C,D)
#define DEBUG_4(A,B,C,D,E)
#define DEBUG_5(A,B,C,D,E,F)
#define DEBUG_6(A,B,C,D,E,F,G)
#define DEBUG_7(A)
#define DEBUG_8(A,B)
#define DEBUG_9(A,B,C)
#define DEBUGELSE
#endif

