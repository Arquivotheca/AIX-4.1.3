/* @(#)36	1.4  src/bos/diag/da/gem/dgraphfru.h, dagem, bos411, 9428A410j 2/1/93 11:11:11 */
/*
 * COMPONENT_NAME: dagem
 *		   (dgraphfru.h)
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*----------------
   This file contains the data structures having information as to which 
FRU is to be assigned for the error codes returned from the application 
test units.  ie. the mapping from error codes to FRU bucket. 
                  ---------------*/

#define LED    0x871          /* LED no. for hispd3d */


/*----------------
 	Definitions for struct 'fru_bucket.frus'. The struct fru_bucket is defined in
 the diagnostics controller include file 'diag/da.h'. 
               -------------------*/

			   /* Conf. fname    loc.  msg.        fru_flag   fru_exempt */

#define FRU1                {  100 ,""      ,"" , 0           , DA_NAME  ,NONEXEMPT }
#define FRU2                {  10   ,""      ,"" , 0          , DA_NAME  ,NONEXEMPT }
#define FRU3                {  95  ,""      ,"" , 0           , DA_NAME  ,NONEXEMPT }
#define FRU4                {  90  ,"CVME"  ,"" ,DGEM_MSG_MAG , NOT_IN_DB, EXEMPT }
#define FRU5                {  100 ,"CVME"  ,"" ,DGEM_MSG_MAG , NOT_IN_DB, EXEMPT }
#define FRU6                {  95  ,"CVME"  ,"" ,DGEM_MSG_MAG , NOT_IN_DB, EXEMPT }
#define FRU7                {  85  ,"CVME"  ,"" ,DGEM_MSG_MAG , NOT_IN_DB, EXEMPT }
#define FRU8                {  10  ,"CVME"  ,"" ,DGEM_MSG_MAG , NOT_IN_DB, EXEMPT }
#define FRU9                {   5  ,"DRP"   ,"" ,DGEM_MSG_DRP , NOT_IN_DB, EXEMPT }
#define FRU10               {   5  ,"SHP"   ,"" ,DGEM_MSG_SHP , NOT_IN_DB, EXEMPT }
#define FRU11               {   5  ,"GCP"   ,"" ,DGEM_MSG_GCP , NOT_IN_DB, EXEMPT }
#define FRU12               { 100  ,"DRP"   ,"" ,DGEM_MSG_DRP , NOT_IN_DB, EXEMPT }
#define FRU13               { 100  ,"SHP"   ,"" ,DGEM_MSG_SHP , NOT_IN_DB, EXEMPT }
#define FRU14               { 100  ,"GCP"   ,"" ,DGEM_MSG_GCP , NOT_IN_DB, EXEMPT }
#define FRU16               {  90  ,"DRP"   ,"" ,DGEM_MSG_DRP , NOT_IN_DB, EXEMPT }
#define FRU17               {  90  ,"SHP"   ,"" ,DGEM_MSG_SHP , NOT_IN_DB, EXEMPT }
#define FRU18               {  90  ,"GCP"   ,"" ,DGEM_MSG_GCP , NOT_IN_DB, EXEMPT }
#define FRU19               { 100  ,"EPM"   ,"" ,DGEM_MSG_EPM , NOT_IN_DB, EXEMPT }
#define FRU20               { 1    ,"Cable" ,"" ,DGEM_MSG_CAB , NOT_IN_DB, EXEMPT }
#define FRU21               {  5   ,""      ,"" ,0            , PARENT_NAME ,NONEXEMPT }
#define FRU22               {  85  ,"EPM"   ,"" ,DGEM_MSG_EPM , NOT_IN_DB, EXEMPT }
#define FRU23               {  15  ,"Monitor" ,"" ,DGEM_MSG_MON , NOT_IN_DB, EXEMPT }
#define FRU24               {  95  ,"GCP"   ,"" ,DGEM_MSG_GCP , NOT_IN_DB, EXEMPT }
#define FRU25               {  95  ,"DRP"   ,"" ,DGEM_MSG_DRP , NOT_IN_DB, EXEMPT }
/* added on Jul 26th '90 */
#define FRU27               {  80  ,"DRP"   ,"" ,DGEM_MSG_DRP , NOT_IN_DB, EXEMPT }
#define FRU28               {  20  ,"EPM"   ,"" ,DGEM_MSG_EPM , NOT_IN_DB, EXEMPT }
#define FRU29               {  10  ,"DRP"   ,"" ,DGEM_MSG_DRP , NOT_IN_DB, EXEMPT }
#define FRU30               {  90  ,"EPM"   ,"" ,DGEM_MSG_EPM , NOT_IN_DB, EXEMPT }
#define FRU31               {  96  ,"GCP"   ,"" ,DGEM_MSG_GCP , NOT_IN_DB, EXEMPT }
#define FRU32               {   4  ,"SHP"   ,"" ,DGEM_MSG_SHP , NOT_IN_DB, EXEMPT }
#define FRU33               {  96  ,"SHP"   ,"" ,DGEM_MSG_SHP , NOT_IN_DB, EXEMPT }
#define FRU34               {   4  ,"GCP"   ,"" ,DGEM_MSG_GCP , NOT_IN_DB, EXEMPT }
#define FRU35               {   4   ,""      ,"" ,0            ,PARENT_NAME ,NONEXEMPT }
/* added on Dec 3th 90 */
#define FRU36               {  55  ,"DRP"   ,"" ,DGEM_MSG_DRP , NOT_IN_DB, EXEMPT }
#define FRU37               {  30  ,"EPM"   ,"" ,DGEM_MSG_EPM , NOT_IN_DB, EXEMPT }
#define FRU38               {  40  ,"GCP"   ,"" ,DGEM_MSG_GCP , NOT_IN_DB, EXEMPT }
#define FRU39               {  35  ,"DRP"   ,"" ,DGEM_MSG_DRP , NOT_IN_DB, EXEMPT }
#define FRU40               {  25  ,"SHP"   ,"" ,DGEM_MSG_SHP , NOT_IN_DB, EXEMPT }
#define FRU41               {  55  ,"GCP"   ,"" ,DGEM_MSG_GCP , NOT_IN_DB, EXEMPT }
#define FRU42               {  45  ,"DRP"   ,"" ,DGEM_MSG_DRP , NOT_IN_DB, EXEMPT }


struct fru_map {
                int testno;                 /* TU no. */
                unsigned long lo_errno;     /* lowest possible error code from TU  */
                unsigned long hi_errno;     /* highest possible error code from TU */
                unsigned long fru_arrptr;   /* index to array frub_graph[]         */
               };

struct fru_map fru_map[] =
	{
              /* TU no.   Lo err      Hi err       */

              { MEMTEST, 0x21002000, 0x211FFFFF, 0 },
              { MEMTEST, 0x22002000, 0x221FFFFF, 1 },
              { MEMTEST, 0x23002000, 0x231FFFFF, 2 },
              { MEMTEST, 0x24002000, 0x241FFFFF, 3 },
              { MEMTEST, 0x25002000, 0x251FFFFF, 4 },
              { MEMTEST, 0x26002000, 0x261FFFFF, 5 },
              { MEMTEST, 0x27002000, 0x271FFFFF, 6 },
              { MEMTEST, 0x28002000, 0x281FFFFF, 7 },
              { MEMTEST, 0x29002000, 0x291FFFFF, 8 },
              { MEMTEST, 0x2A002000, 0x2A1FFFFF, 9 },

              { REGTEST, 0x21000300, 0x21001FFF, 10},
              { REGTEST, 0x22000300, 0x22001FFF, 11},
              { REGTEST, 0x23000300, 0x23001FFF, 12},
              { REGTEST, 0x24000300, 0x24001FFF, 13},
              { REGTEST, 0x25000300, 0x25001FFF, 14},
              { REGTEST, 0x26000300, 0x26001FFF, 15},
              { REGTEST, 0x27000300, 0x27001FFF, 16},
              { REGTEST, 0x28000300, 0x28001FFF, 17},
              { REGTEST, 0x29000300, 0x29001FFF, 18},
              { REGTEST, 0x2A000300, 0x2A001FFF, 19},
              { REGTEST, 0x2B000300, 0x2B001FFF, 22},
      
              { MBCTEST, 0x21000200, 0x21000224, 24},
              { MBCTEST, 0x22000200, 0x22000224, 25},
              { MBCTEST, 0x23000200, 0x23000224, 26},
              { MBCTEST, 0x24000200, 0x24000224, 27},
              { MBCTEST, 0x25000200, 0x25000224, 28},
 		
              { DMATEST, 0x2F000001, 0x2F000001, 20},

              { FIFOTEST, 0x2F000002, 0x2F000002, 21},

              { SHPTEST, 0x20000000, 0x20000000, 29},
              { GCPTEST, 0x30000000, 0x30000000, 30},
              { GCPTEST, 0x30000001, 0x300FFFFF, 31},
              { GCPTEST, 0x31000000, 0x31000000, 32},  /* This return code can be got  */
              { BUSTEST, 0x31000000, 0x31000000, 32},  /* during bustest or load test  */
              { DRPTEST, 0x40000000, 0x40000000, 33},
              { DRPTEST, 0x40000001, 0x400FFFFF, 34},
              { DRPTEST, 0x41000000, 0x41000000, 35},  /* This return code can be got  */
              { BUSTEST, 0x41000000, 0x41000000, 35},  /* during bustest or load test  */
              { DRPTEST, 0x50000000, 0x500FFFFF, 36},  /* EPM error codes */
              { SHPTEST, 0x60000000, 0x60000000, 37},
              { SHPTEST, 0x60000001, 0x600FFFFF, 38}, 
              { SHPTEST, 0x61000000, 0x61000000, 39},  /* This return code can be got  */ 
              { BUSTEST, 0x61000000, 0x61000000, 39},  /* during bustest or load test  */

              { PATTERNS, 0x90000001, 0x90000001,40}, /* Microcode load fail (90000000) */
              { PATTERNS, 0x90000003, 0x90000003,40}, /* is taken as software error for */
              { PATTERNS, 0x90000002, 0x90000002,41}, /* now. However SRN and msg. are  */
              { PATTERNS, 0x90000004, 0x90000004,41}, /* available if it is decided to  */
              { PATTERNS, 0x90000005, 0x90000005,42}, /* have a FRU assigned.           */

              { VPDTEST, 0x13000000, 0x13000000, 43},
              { VPDTEST, 0x23000000, 0x23000000, 44},
              { VPDTEST, 0x33000000, 0x33000000, 45},
              { VPDTEST, 0x43000000, 0x43000000, 46},
              { VPDTEST, 0x53000000, 0x53000000, 47},
              { VPDTEST, 0x63000000, 0x63000000, 48},

              { BUSTEST, 0x21000000, 0x21000000, 49},
	      { INTERACTION, 0x70009F00, 0x7001ffff, 50},
	      { INTERACTION, 0x71009F00, 0x7101ffff, 51},
	      { PATTERNS, 0x90000009, 0x90000009,30},
	      { 0, 0, 0, 52},
	};

/* The array 'fru_rcode' is used to update the R.code field of the
   fru_bucket array.  This is needed because there are multiple levels
   of cards that need to be called out correctly for the same error.
   For instance an rc 0f 0x90000005 in the fru_map points to entry
   42 in the fru_bucket array.  However, depending on whether an 8 bit
   or 24 bit or a 60 or 77 HZ FBB is present, the R.code field will be
   modified to point to 130, 230, 167 or 267.  These will result in
   an SRN e.g]. 871-130
		-----------------*/
	   uint fru_r_code[][16]=   {

/* 0 */   {0},
/* 1 */   {0},
/* 2 */   {0},
/* 3 */   {0},
/* 4 */   {0},
/* 5 */   {0},
/* 6 */   {0},
/* 7 */   {0},
/* 8 */   {0},
/* 9 */   {0},

/* 10 */  {0},
/* 11 */  {0},
/* 12 */  {0},
/* 13 */  {0},
/* 14 */  {0},
/* 15 */  {0},
/* 16 */  {0},
/* 17 */  {0},
/* 18 */  {0},
/* 19 */  {0},

/* 20 */  {0},
/* 21 */  {0},
/* 22 */  {0},
/* 23 */  {0},
/* 24 */  {0},
/* 25 */  {0},
/* 26 */  {0},
/* 27 */  {0},
/* 28 */  {0},
/* 29 */  {0x130,0x130,0x130,0x130,0x167,0x167,0x167,0x167,
	    0x230,0x230,0x230,0x230,0x267,0x267,0x267,0x267},

/* 30 */  {0x166,0x166,0x166,0x166,0x166,0x166,0x166,0x166,
	    0x266,0x266,0x266,0x266,0x266,0x266,0x266,0x266},
/* 31 */  {0x166,0x166,0x166,0x166,0x166,0x166,0x166,0x166,
	    0x266,0x266,0x266,0x266,0x266,0x266,0x266,0x266},
/* 32 */  {0},
/* 33 */  {0},
/* 34 */  {0},
/* 35 */  {0},
/* 36 */  {0},
/* 37 */  {0},
/* 38 */  {0},
/* 39 */  {0},
/* 40 */  {0},
/* 41 */  {0},
/* 42 */  {0},
/* 43 */  {0},
/* 44 */  {0},
/* 45 */  {0},
/* 46 */  {0},
/* 47 */  {0},
/* 48 */  {0},
/* 49 */  {0},
/* 50 */  {0},
/* 51 */  {0}
   };

/*-----------------
  The array 'frub_graph' contains all possible FRU buckets which can result
  from the diagnostics. The struct 'fru_bucket' is defined in the diagnotics
  controller include file 'diag/da.h'. The msg nos. 'DGEM_MSG_300' etc. refer
  to the no. assigned from dcda.msg.
 		-----------------*/

/*  The following are the failing function codes that can be called out by
    thus DA.  Some are obsolete.
				   old    iso    generic

	 microchannel card         871
	 magic (cmve card)         110
	 gcp                       111
	 drp                       114    B50    C12
	 shp                       113    B51    C13
	 fbb-8                     112    B52    C14
	 fbb-24                    115    B53    C14

	 ----------------------------------------------*/

               struct fru_bucket frub_graph[]=
               {
               /*.      LED   R.code    msg no.        FRU buckets   ........*/

/* 0 */       { "", FRUB1,  LED,  0x101, DGEM_MSG_300 ,  { FRU7, FRU2, FRU35, FRU20}},
/* 1 */       { "", FRUB1,  LED,  0x102, DGEM_MSG_300 ,  { FRU7, FRU2, FRU35, FRU20}},
/* 2 */       { "", FRUB1,  LED,  0x103, DGEM_MSG_300 ,  { FRU5 }},
/* 3 */       { "", FRUB1,  LED,  0x104, DGEM_MSG_300 ,  { FRU5 }},
/* 4 */       { "", FRUB1,  LED,  0x105, DGEM_MSG_300 ,  { FRU5 }},
/* 5 */       { "", FRUB1,  LED,  0x106, DGEM_MSG_300 ,  { FRU5 }},
/* 6 */       { "", FRUB1,  LED,  0x107, DGEM_MSG_300 ,  { FRU5 }},
/* 7 */       { "", FRUB1,  LED,  0x108, DGEM_MSG_300 ,  { FRU5 }},
/* 8 */       { "", FRUB1,  LED,  0x109, DGEM_MSG_300 ,  { FRU5 }},
/* 9 */       { "", FRUB1,  LED,  0x110, DGEM_MSG_300 ,  { FRU5 }},

/* 10 */       { "", FRUB1,  LED,  0x111, DGEM_MSG_400 ,  { FRU5 }},
/* 11 */       { "", FRUB1,  LED,  0x112, DGEM_MSG_400 ,  { FRU5 }},
/* 12 */       { "", FRUB1,  LED,  0x113, DGEM_MSG_400 ,  { FRU5 }},
/* 13 */       { "", FRUB1,  LED,  0x114, DGEM_MSG_400 ,  { FRU5 }},
/* 14 */       { "", FRUB1,  LED,  0x115, DGEM_MSG_400 ,  { FRU5 }},
/* 15 */       { "", FRUB1,  LED,  0x116, DGEM_MSG_400 ,  { FRU5 }},
/* 16 */       { "", FRUB1,  LED,  0x117, DGEM_MSG_400 ,  { FRU7, FRU2, FRU35, FRU20}},
/* 17 */       { "", FRUB1,  LED,  0x118, DGEM_MSG_400 ,  { FRU5 }},
/* 18 */       { "", FRUB1,  LED,  0x119, DGEM_MSG_400 ,  { FRU5 }},
/* 19 */       { "", FRUB1,  LED,  0x120, DGEM_MSG_400 ,  { FRU5 }},

/* 20 */       { "", FRUB1,  LED,  0x121, DGEM_MSG_DMA ,  { FRU7, FRU2, FRU35, FRU20}},
/* 21 */       { "", FRUB1,  LED,  0x122, DGEM_MSG_FIF ,  { FRU5 }},
/* 22 */       { "", FRUB1,  LED,  0x123, DGEM_MSG_400 ,  { FRU5 }},
/* 23 */       { "", FRUB1,  LED,  0x124, DGEM_MSG_400 ,  { FRU5 }},
/* 24 */       { "", FRUB1,  LED,  0x125, DGEM_MSG_500 ,  { FRU5 }},
/* 25 */       { "", FRUB1,  LED,  0x126, DGEM_MSG_500 ,  { FRU5 }},
/* 26 */       { "", FRUB1,  LED,  0x127, DGEM_MSG_500 ,  { FRU5 }},
/* 27 */       { "", FRUB1,  LED,  0x128, DGEM_MSG_500 ,  { FRU5 }},
/* 28 */       { "", FRUB1,  LED,  0x129, DGEM_MSG_500 ,  { FRU5 }},
/* 29 */       { "", FRUB1,  LED,  0x130, DGEM_MSG_710 ,  { FRU7, FRU9, FRU10, FRU11 }},

/* 30 */       { "", FRUB1,  LED,  0x166, DGEM_MSG_701 ,  { FRU31,FRU32 }},
/* 31 */       { "", FRUB1,  LED,  0x166, DGEM_MSG_701 ,  { FRU31,FRU32 }},
/* 32 */       { "", FRUB1,  LED,  0x133, DGEM_MSG_702 ,  { FRU14 }},
/* 33 */       { "", FRUB1,  LED,  0x363, DGEM_MSG_704 ,  { FRU27,FRU28 }},
/* 34 */       { "", FRUB1,  LED,  0x363, DGEM_MSG_704 ,  { FRU27,FRU28 }},
/* 35 */       { "", FRUB1,  LED,  0x336, DGEM_MSG_705 ,  { FRU12 }},
/* 36 */       { "", FRUB1,  LED,  0x364, DGEM_MSG_709 ,  { FRU30,FRU29 }},
/* 37 */       { "", FRUB1,  LED,  0x365, DGEM_MSG_707 ,  { FRU33,FRU34 }},
/* 38 */       { "", FRUB1,  LED,  0x365, DGEM_MSG_707 ,  { FRU33,FRU34 }},
/* 39 */       { "", FRUB1,  LED,  0x340, DGEM_MSG_708 ,  { FRU13 }},

/* 40 */       { "", FRUB1,  LED,  0x351, DGEM_MSG_VIS ,  { FRU24, FRU9  }},
/* 41 */       { "", FRUB1,  LED,  0x352, DGEM_MSG_VIS ,  { FRU25, FRU11 }},
/* 42 */       { "", FRUB1,  LED,  0x353, DGEM_MSG_VIS ,  { FRU36, FRU37, FRU23 }},
/* 43 */       { "", FRUB1,  LED,  0x155, DGEM_VPD_MCA ,  { FRU1  }},
/* 44 */       { "", FRUB1,  LED,  0x160, DGEM_VPD_MAG ,  { FRU5  }},
/* 45 */       { "", FRUB1,  LED,  0x156, DGEM_VPD_GCP ,  { FRU18, FRU8 }},
/* 46 */       { "", FRUB1,  LED,  0x359, DGEM_VPD_DrP ,  { FRU16, FRU8 }},
/* 47 */       { "", FRUB1,  LED,  0x357, DGEM_VPD_FBB ,  { FRU19  }},
/* 48 */       { "", FRUB1,  LED,  0x358, DGEM_VPD_ShP ,  { FRU13  }},
/* 49 */       { "", FRUB1,  LED,  0x161, DGEM_MSG_712 ,  { FRU5  }},
/* 50 */       { "", FRUB1,  LED,  0x301, DGEM_MSG_BUS ,  { FRU38, FRU39, FRU40  }},
/* 51 */       { "", FRUB1,  LED,  0x302, DGEM_MSG_BUS ,  { FRU41, FRU42  }},
		};

/*-----------
     FRU buckets for the errors logged in the error-log
    -------------*/
         struct fru_bucket frub_ela[]=
         {
/* 0 */  { "", FRUB1,  LED,  0x141, DGEM_MSG_ELA ,  { FRU1  }},
/* 1 */  { "", FRUB1,  LED,  0x143, DGEM_MSG_ELA ,  { FRU8, FRU16  }},
/* 2 */  { "", FRUB1,  LED,  0x144, DGEM_MSG_ELA ,  { FRU8, FRU17  }},
/* 3 */  { "", FRUB1,  LED,  0x142, DGEM_MSG_ELA ,  { FRU8, FRU18  }},
/* 4 */  { "", FRUB1,  LED,  0x146, DGEM_MSG_ELA ,  { FRU8, FRU16  }},
/* 5 */  { "", FRUB1,  LED,  0x147, DGEM_MSG_ELA ,  { FRU8, FRU17  }},
/* 6 */  { "", FRUB1,  LED,  0x145, DGEM_MSG_ELA ,  { FRU8, FRU18  }},
/* 7 */  { "", FRUB1,  LED,  0x148, DGEM_MSG_ELA ,  { FRU7, FRU9, FRU10, FRU11 }},
/* 8 */  { "", FRUB1,  LED,  0x149, DGEM_MSG_ELA ,  { FRU7, FRU9, FRU10, FRU11 }},
/* 9 */  { "", FRUB1,  LED,  0x150, DGEM_MSG_ELA ,  { FRU7, FRU9, FRU10, FRU11 }},
         };
