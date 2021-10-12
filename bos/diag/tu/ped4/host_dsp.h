/* @(#)45	1.1  src/bos/diag/tu/ped4/host_dsp.h, tu_ped4, bos411, 9428A410j 6/11/93 14:52:57 */
/*
 * COMPONENT_NAME: (tu_ped4) Pedernales Graphics Adapter Test Units
 *
 * FUNCTIONS: Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * MODULE NAME: host_dsp.h
 *
 * STATUS: Release 1, EC 00, EVT Version 1
 *
 * DEPENDENCIES: None
 *
 * RESTRICTIONS: None
 *
 * EXTERNAL REFERENCES:
 *
 *      OTHER ROUTINES: None
 *
 *      DATA AREAS: None
 *
 *      TABLES: None
 *
 *      MACROS: None
 *
 * COMPILER/ASSEMBLER
 *
 *      TYPE, VERSION: AIX C Compiler
 *
 *      OPTIONS:
 *
 * NOTES:  This file contains :
 *         1. CONSTANTS shared between the microcode and the host programs.
 *         2. TYPEDEFS shared between the microcode and the host programs.
 *
 * CHANGE ACTIVITIES:
 *
 *    EC00, VERSION 00 (ORIGINAL), 1/08/91 !KM!
 *
 */



/******  COMMANDS FROM HOST TO DSP TEST CONTROLLER  *******/

#define GET_READY        0x87654321  /* GET READY for next command */
#define NEXT_PAT         0xF00000    /* request next pattern for   */
#define NEXT_PAT2        0xFF0000    /*  the initial protocol      */

#define GIVE_SECONDARY_ADDRESS       0xCCE0  /* request address of */
#define GIVE_C30B_BOOTSTRAP_ADDRESS  0xC3E0  /* special buffers on */
#define GIVE_C30B_MEM_MCODE_ADDRESS  0x3CE0  /* the adapter, to be */
#define GIVE_C30B_DIAG_MCODE_ADDRESS 0xCC10  /* accessed from host */

#define SEND_INTERRUPT  0xD00D      /* tell C30M to write to DSP COMMO */
#define CONTINUE        0xC0C0      /* tell C30M to continue dynamic test */

/**********  RESPONSES FROM DSP TO HOST ****************************/

#define READY           0x12345678  /* indicate DSP READY for commands */
#define OPCODE_ERROR    0xB0B0B0B0  /* indicates Bad OPCODE received */
#define DSP_COMMO_INT_VALUE 0xABCDFACE /* value used to check the DSP COMMO */
                                       /* interrupt */
#define SEND_DATA       0xDADA      /* used in bim_dynamic test */



/* patterns used in communication and testing */

#define PAT0           0xaa55aa55  /* Five patterns for verifying */
#define PAT1           0x00000000  /* data & interrupt integrity  */
#define PAT2           0x69696969  /* of DSP_COMMO Register       */
#define PAT3           0x18273645  /* and also used in the dynamic */
#define PAT4           0xffffffff  /* test.                        */

#define PAT5           0xBABEFACE
#define PAT6           0xCAFEBABE
#define PAT7           0x69CAFE69
#define PAT8           0x11BABE11
#define PAT9           0xDAD00DAD
#define PAT10          0x11111111
#define PAT11          0x33333333
#define PAT12          0xCCCCCCCC
#define PAT13          0xFDFDFDFD
#define PAT14          0xF00DF00D
#define PAT15          0xDEAFBABE
#define PAT16          0xAAAAAAAA
#define PAT17          0xDEDE5555
#define PAT18          0x22222222
#define PAT19          0x66666666
#define PAT20          0x77777777
#define PAT21          0x88888888
#define PAT22          0x54545454
#define PAT23          0x123123EA
#define PAT24          0x7865942f
#define PAT25          0x09099009
#define PAT26          0x12345432
#define PAT27          0x44444444
#define PAT28          0x41234564
#define PAT29          0xC3C3C3C3
#define PAT30          0x3C3C3C3C
#define PAT31          0x00000000
#define PAT32          0x69696969
#define PAT33          0x18273645
#define PAT34          0xffffffff
#define PAT35          0xaa55aa55
#define PAT36          0x00000000
#define PAT37          0x69696969
#define PAT38          0x18273645
#define PAT39          0xffffffff


#define INC_PATTERN    0x00010203  /* Used in BIM DMA test */


/* other shared constants */

#define C30B_DIAG_MCODE_SIZE 20000     /* size of C30B diagnostics microcode */
#define C30B_MEM_MCODE_SIZE  0x2000    /* size of C30B memory test microcode */

#define DMA_SE_OPCODE 0xA005      /* OPCODE defined for DIAGNOSTICS DMA SE */
#define DMA_SE_LENGTH 8           /* LENGTH of DMA SE in words */

#define SPECIAL_SECONDARY_OFFSET   9 /* offset location into the */
                                     /* secondary buffer used as */
                                     /* special memory location  */
                                     /* in the dynamic test.     */

#define DYNAMIC_TEST_SPECIAL_VALUE 0xFFFFFFFF /* value used to initialize    */
                                              /* the special memory location */
                                              /* on the adapter shared in    */
                                              /* the dynamic test            */

/* secondary return information buffer length follows */
/* NOTE: This value is a copy of the one in tu_type.h */
#define SEC_BUF_LEN 10

/* shared data structure types */

/* DMA Structure Element TYPE DEFINITION
 *
 * Note : The DMA SE for diagnostics has been defined as a structure element
 *        that is sent by the device driver to the BIM Priority Command Buffer.
 *        The SE is organized as shown below :
 *
 *                        bits  31        16 15        0
 *                             ÚÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄ¿
 *                      word 0 ³ SE length  ³ SE OPCODE ³
 *                             ÃÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄ´
 *                      word 1 ³ Correlator ³ flags     ³
 *                             ÃÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄ´
 *                      word 2 ³ HOST buffer address    ³
 *                             ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 *                      word 3 ³ DMA length in bytes    ³
 *                             ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 *                      word 4 ³ DMA width factor       ³
 *                             ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 *                      word 5 ³ DMA stride factor      ³
 *                             ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 *                      word 6 ³ Host buffer offset     ³
 *                             ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
 *                      word 7 ³ reserved location 1    ³
 *                             ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
 *
 *        The only fields in the structure that are accessed by the device
 *        driver are the SE length(checked by DD), SE OPCODE(checked by DD),
 *        Correlator(supplied by the DD), and host address(changed by the DD).
 *        The rest of the fields are left alone and sent to the adapter with
 *        whatever information the caller put in there.
 *
 *        The DMA SE TYPE is defined below.
 */

typedef struct {
   int length_opcode;  /* SE Length and SE opcode */
   int corr_flags;     /* DMA correlator and optional flags */
   int host_address;   /* address of host's DMA buffer */
   int dma_length;     /* length of DMA transfer in bytes (optional) */
   int width;          /* DMA width factor */
   int stride;         /* DMA stride factor */
   int host_offset;    /* Host address offset to be added to host_address */
   int reserved;       /* reserved location */
} DMA_SE_TYPE, *DMA_SE_PTR_TYPE;   /* end typedef */

