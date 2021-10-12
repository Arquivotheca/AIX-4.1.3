/* @(#)69	1.2  src/bos/diag/tu/msla/mslatu.h, tu_msla, bos411, 9428A410j 6/15/90 17:24:07 */
/*
 * COMPONENT_NAME: (mslatu.h) header file for MSLA diagnostic
 *			application.
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************************
* DEFINE TEST UNIT NUMBERS                                                    *
******************************************************************************/
#define POSTEST              1
#define MEMTEST              2
#define REGTEST              3
#define FTP20TEST            4
#define VPDTEST              5
#define WRAPTEST             6
#define INT_RIOS             7
#define DMATEST              8

#define FTP19TEST            WRAPTEST
#define FTP18TEST            WRAPTEST

#define BIN_PATHNAME         "bin/"
#define FILE_LEN             51

/******************************************************************************
* Test Unit Control Blocks                                                    *
******************************************************************************/
struct tucb_t {
       long  tu, mfg, loop;  /* test unit control block header */
       long  r1,r2;          /* reserved for future expansion  */
};

struct dgsfile {
       char *msla20dgs;     /* msla ftp20 diagnostic ucode  */
       char *pslab_dgs;     /* msla psla level-B micro-code diagnostic */
       char *sslab_dgs;     /* msla ssla level-B micro-code diagnostic */
};

struct mslatu {
      struct tucb_t header;

      int tu1;
      int tu2;
      int r0;
      unsigned int gmbase;      /* memory base on RT/RIOS    busopen */
      unsigned int gibase;      /* i/o    base on RT/RIOS    busopen */
      unsigned int msla_membase;/* memory base on MSLA POS register  */
      unsigned int msla_iobase; /* i/o    base on MSLA POS register  */
      int  fd ;              /* file discriptor from busopen         */
      int slot;              /* slot number of msla   uchannel card  */
/*    struct dgsret dgsrc;      returned info (defined in dgsvars.h) */
      struct dgsfile dgssrc; /* name of microcode files to load      */
/*    struct ldr ldrs;*/

};

struct mslaerrcnt {
        unsigned int  posbad;
        unsigned int  vpdbad;
        unsigned int  membad;
        unsigned int  regbad;
        unsigned int  ftp20bad;
        unsigned int  ftp19bad;
        unsigned int  int_rios;
        unsigned int  dmatest;
        unsigned int  loopcnt;
};

