/* @(#)17	1.7  src/bos/diag/tu/ped4/tu_type.h, tu_ped4, bos411, 9428A410j 2/22/94 18:54:43 */
/*
 * COMPONENT_NAME: (tu_ped4) Pedernales Graphics Adapter Test Units
 *
 * FUNCTIONS: Declarations only
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

#define ulong		unsigned long 

/* microcode files' equates follow */

/* diagnostics microcode for C30M */
#define C30M_DIAG_MCODE "8ee3p4d1.00"

/* memory test microcode for C30M */
#define C30M_MEM_MCODE  "8ee3p4d2.00"

/* bootstrap loader for C30B */
#define C30B_BOOTSTRAP  "8ee3p4d3.00"

/* diagnostics microcode for C30B */
#define C30B_DIAG_MCODE "8ee3p4d4.00"

/* memory test microcode for C30B */
#define C30B_MEM_MCODE  "8ee3p4d5.00"

/* secondary return information buffer length follows */
#define SEC_BUF_LEN 10

/* Test Unit number definitions follow */

#define TU_01 0x01    /* POS registers test */
#define TU_02 0x02    /* VPD test */
#define TU_03 0x03    /* host_bim_test */
#define TU_04 0x04    /* host_mem_test */
#define TU_05 0x05    /* Master C30 memory test */
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
#define TU_20 0x14    /* Float to float test */
#define TU_21 0x15    /* C30M/C30B dynamic test */
#define TU_22 0x16    /* Frame Buffer B test */
#define TU_23 0x17    /* BLAST functional test */
#define TU_24 0x18    /* Mini-Wix test */

#define TU_25 0x19    /* */
#define TU_26 0x1A    /* */
#define TU_27 0x1B    /* */
#define TU_28 0x1C    /* */
#define TU_29 0x1D    /* */

#define TU_30 0x1E    /* Color Bar screen */
#define TU_31 0x1F    /* Red screen */
#define TU_32 0x20    /* Green screen */
#define TU_33 0x21    /* Blue screen */
#define TU_34 0x22    /* White screen */
#define TU_35 0x23    /* Black screen */
#define TU_36 0x24    /* 9x9 Cross Hatch */
#define TU_37 0x25    /* 11x9 Cross Hatch */

#define TU_38 0x26    /* NOT USED */

#define TU_39 0x27    /* overlays, windows, visual test */
#define TU_40 0x28    /* Scrolling H test */
#define TU_41 0x29    /* diag_buf back end debug tool */
#define TU_42 0x2A    /* BLAST GETHOT thermal exerciser */
#define TU_43 0x2B    /* BIM GETHOT thermal exerciser */
#define TU_44 0x2C    /* BLAST GETHOT and BIM GETHOT running together */
#define TU_45 0x2D    /* diag_buf back end debug tool */

#define TU_50 0x32    /* FrameBuf A grn  */
#define TU_51 0x33    /* FrameBuf B grn  */
#define TU_52 0x34    /* FrameBuf A red  */
#define TU_53 0x35    /* FrameBuf A red  */

/* PIPE TUS */
#define TU_71 0x47    /* Pipe Data Path test */
#define TU_72 0x48    /* P1 Memory Test */
#define TU_73 0x49    /* P2 Memory Test */
#define TU_74 0x4a    /* P3 Memory Test */
#define TU_75 0x4b    /* P4 Memory Test */

#define TU_60 0x3C    /* Set CRT refresh rate to 60 Hz. */
#define TU_77 0x4D    /* Set CRT refresh rate to 77 Hz. */
#define TU_90 0x5A    /* ENTER monitor mode to allow START of testing */
#define TU_91 0x5B    /* EXIT monitor mode and STOP testing */

#define TU_OPEN  0x5A    /* Open path thru OS to allow START of testing */
#define TU_CLOSE 0x5B    /* Close rcm0 device and STOP testing */

/* Define Data struct used for calling the exectu() func in AIX 3.2.x */
   struct tucb_t {
      long tu,     /* test unit number */
           mfg,    /* manufacturing mode flag, one (1) is True */
           loop;   /* tu loop count */
      long r1,     /* reserved word 1 */
           r2;     /* reserved word 2 */
   };  /* end struct tucb_t */

/* Define Data struct used for calling the exectu() function for AIX 4.x*/
   typedef struct {
      ulong tu;     /* test unit number */
      ulong loop;   /* tu loop count */
      long secondary_ptr[SEC_BUF_LEN]; /* secondary return information */
   } PED4_TU_TYPE;

