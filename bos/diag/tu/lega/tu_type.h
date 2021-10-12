/* @(#)20       1.9.1.2  src/bos/diag/tu/lega/tu_type.h, tu_lega, bos411, 9428A410j 2/10/94 13:30:13 */
/*
 * COMPONENT_NAME: (tu_lega) Low End Graphics Adapter Test Units
 *
 * FUNCTIONS: Declarations only
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * MODULE NAME: tu_type.h
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
 * NOTES: There is a partial copy of this file in c30_m_b.h which needs to be
 *        updated if this file changes.
 *
 */

/* microcode files' equates follow */

/* diagnostics microcode for C30M */
#define C30M_DIAG_MCODE "8ee3ld1.00"

/* memory test microcode for C30M */
#define C30M_MEM_MCODE  "8ee3ld2.00"

/* bootstrap loader for C30B */
#define C30B_BOOTSTRAP  "8ee3ld3.00"

/* diagnostics microcode for C30B */
#define C30B_DIAG_MCODE "8ee3ld4.00"

/* memory test microcode for C30B */
#define C30B_MEM_MCODE  "8ee3ld5.00"

/* secondary return information buffer length follows */
#define SEC_BUF_LEN 10

/* Test Unit number definitions follow */

#define TU_01 0x01    /* POS registers test */
#define TU_02 0x02    /* VPD test */
#define TU_03 0x03    /* host_bim_test */
#define TU_04 0x04    /* host_mem_test (LEGA and LEGA3 only) */
#define TU_05 0x05    /* Master C30 memory test (LEGA and LEGA3 only) */
#define TU_06 0x06    /* Blast C30 memory test */
#define TU_07 0x07    /* Bim test */
#define TU_08 0x08    /* Host interrupt test */
#define TU_09 0x09    /* BIM DMA test */
#define TU_10 0x0A    /* BIM dynamic test */
#define TU_11 0x0B    /* C30M/C30B dynamic test */
#define TU_12 0x0C    /* BLAST presence test */
#define TU_13 0x0D    /* Ramdac test */
#define TU_14 0x0E    /* Frame Buffer A test */
#define TU_15 0x0F    /* Z Buffer 1 test */
#define TU_16 0x10    /* Z Buffer 2 test */
#define TU_17 0x11    /* Z Buffer 3 test */
#define TU_18 0x12    /* Z Buffer (all three) test */
#define TU_19 0x13    /* WPG Buffer test */
#define TU_20 0x14    /* BLAST functional test (LEGA and LEGA2 only) */
#define TU_21 0x15    /* overlays, windows, visual test */
#define TU_22 0x16    /* Color Bar screen */
#define TU_23 0x17    /* Red screen */
#define TU_24 0x18    /* Green screen */
#define TU_25 0x19    /* Blue screen */
#define TU_26 0x1A    /* White screen */
#define TU_27 0x1B    /* Black screen */
#define TU_28 0x1C    /* Cross Hatch 9x9 */
#define TU_29 0x1D    /* Cross Hatch 11x9 */
#define TU_30 0x1E    /* Full Float To Float Converter Test */
#define TU_31 0x1F    /* */
#define TU_32 0x20    /* Frame Buffer B test */
#define TU_33 0x21    /* BLAST functional test (LEGA3 only) */
#define TU_34 0x22    /* host_mem_test (LEGA2 only) */
#define TU_35 0x23    /* Master C30 memory test (LEGA2 only) */
#define TU_36 0x24    /* */

#define TU_40 0x28    /* Scrolling H test */
#define TU_41 0x29    /* diag_buf back end debug tool */
#define TU_42 0x2A    /* BLAST GETHOT thermal exerciser */
#define TU_43 0x2B    /* BIM GETHOT thermal exerciser */
#define TU_44 0x2C    /* BLAST GETHOT and BIM GETHOT running together */
#define TU_45 0x2D    /* */

#define TU_60 0x3C    /* Set CRT refresh rate to 60 Hz. */
#define TU_77 0x4D    /* Set CRT refresh rate to 77 Hz. */

#define TU_OPEN  0x5A    /* Open path thru OS to allow START of testing */

#define TU_CLOSE 0x5B    /* Clean up and Close path thru OS and STOP testing */


/* Define Data structures used for calling the exectu() function */
   typedef struct {
     ulong tu;                         /* test unit number */
     ulong loop;                       /* tu loop count    */
     ulong secondary_ptr[SEC_BUF_LEN]; /* secondary return information */
   } LEGA_TU_TYPE;


