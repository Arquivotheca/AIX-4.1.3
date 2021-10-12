/* @(#)25	1.3  src/bos/diag/da/msla/dsla_tu.h, damsla, bos411, 9428A410j 12/10/92 09:02:33 */
/*
 *   COMPONENT_NAME: DAMSLA
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <diag/atu.h>
struct dgsfile
             {
              char *msla20dgs;     /* msla ftp20 diagnostic ucode  */
              char *pslab_dgs;     /* msla psla level-B micro-code diagnostic */
              char *sslab_dgs;     /* msla ssla level-B micro-code diagnostic */
             };

struct _mslatu
              {
               struct tucb_t tucb_header;
               int tu1;
               int tu2;
               int r0;
               unsigned int gmbase;      /* memory base on RT/RIOS    busopen */
               unsigned int gibase;      /* i/o    base on RT/RIOS    busopen */
               unsigned int msla_membase;/* memory base on MSLA POS register  */
               unsigned int msla_iobase; /* i/o    base on MSLA POS register  */
               int  fd ;              /* file discriptor from busopen         */
               int slot;              /* slot number of msla   uchannel card  */
               struct dgsfile dgssrc; /* name of microcode files to load      */
              } ;
		
