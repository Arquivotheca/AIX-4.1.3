/* @(#)95	1.8  src/bos/diag/tu/gem/tugem.h, tu_gem, bos411, 9428A410j 3/23/94 08:56:32 */
/*
 * COMPONENT_NAME: tu_gem (tugem.h) 
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "dgsload.h"
#include "hispd3d.h"
#include  "gem_diag.h"
/******************************************************************************
* DEFINE TEST UNIT NUMBERS                                                    *
******************************************************************************/
#define VPDTEST              1
#define MEMTEST              2
#define REGTEST              3
#define MBCTEST              4
#define FIFOTEST             5
#define DMATEST              6
#define BUSTEST              7
#define DRPTEST              8
#define SHPTEST              9
#define GCPTEST              10
#define PATTERNS             11
#define BITPLANE             12
#define IMGTEST              13
#define INTERACTION          14

#define NGSLOTS     11
#define CARD_MASK   0x00FF0000
#define SYSFAIL     0x80000000
#define IMP_ID      0x44
#define QUICKMEM    1
#define LONGMEM     0
#define  GEMINI         1
#define  TAURUS         2
#define  NOMAGIC       -99

/************ card level definitions  *********/
/* older levels and 8 bit fbb set to zero     */
#define SHP_FST     8      /* 1000  */
#define DRP_AA      4      /* 0100  */
#define FBB_24      2      /* 0010  */
#define FBB_ISO     1      /* 0001  */

/******************************************************************************
* Test Unit Control Blocks                                                    *
******************************************************************************/
struct gemtucb_t {
    long  tu, mfg, loop;  /* test unit control block header */
    long  r1,r2;          /* reserved for future expansion  */
};

struct gemtu {
     struct gemtucb_t header;
     int tu1;
     int tu2;
     int r0;
     unsigned gmbase;      /* memory base returned from busopen    */
     int  fd ;             /* file discriptor from busopen         */
     int  hft_fd ;         /* file discriptor from hft open        */
     struct dgsret dgsrc;  /* returned info (defined in hispd3d.h) */
};

