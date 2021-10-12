/* @(#)21	1.5  src/bos/diag/da/msla/dslafrub.h, damsla, bos411, 9428A410j 12/10/92 09:10:53 */
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


#include "dsladef.h"

/**************************************************************************/
/*                                                                        */
/*      da_fru structure holds related information to the FRU bucket      */
/*      variables in this structure are explained in DA CAS (SJDIAG)      */
/*      under the function name ADDFRU.                                   */
/*      note : there is a fru structure entry for each possible value     */
/*             of error-return code , hence, there will be multiple       */
/*             set of parent-child confidence level values ofr each test  */
/*                                                                        */
/**************************************************************************/

struct fru_map {  
                unsigned long real_errno;   /* Error return codes */
                unsigned long fru_arrptr;   /* fru bucket array index */
               };
struct mapping {
                int testnumb;               /* TU no. */
                struct fru_map fru_map[25]; /* max no. entries in fru bucket is 25 */
               };

#define FRU1               {  100 ,""  ,"" ,0 , DA_NAME , EXEMPT },
#define FRU2               {  95  ,""  ,"" ,0 , DA_NAME , EXEMPT },
#define FRU3               {  5   ,""  ,"" ,0 , PARENT_NAME , NONEXEMPT },
#define LED    0x858
        
/*.............. error return codes to fru bucket array index table ........*/
struct mapping err_to_frumap[] =
    {
       /* mapping for frub_postest */
              {MSLA_POS_TEST     , {
                    { 0x11001, 0},
                    { 0x11002, 1},
                    { 0x12001, 2},
                    { 0x12002, 3},
                    { 0x12003, 4},
                    { 0x12004, 5},
                    { 0x12005, 6},
                    { 0x12006, 7},
                    { 0x12007, 8},
                    { 0x12008, 9},
                    { 0x12009, 10},
                    { 0x1200A, 11},
                    { 0x13001, 12},
                    { 0x13002, 13},
                    { 0x13003, 14},
                    { 0x13004, 15},
                    { 0x13005, 16},
                    {0 , 0},
                  },
               },
       /* mapping for frub_memtest  */
              {MSLA_MEM_TEST     , {  
                    { 0x21000, 0 },
                    { 0x21001, 1 },
                    { 0x22000, 2 }, 
                    { 0x22002, 3 },
                    {0,0}, 
                   },
              },
      /* mapping for frub_ioregtest */
                { MSLA_REG_TEST     , { 
                       { 0x31000, 0 },
                       { 0x31001, 1 },
                       { 0x31002, 2 },
                       { 0x31003, 3 },
                       { 0x31004, 4 },
                       { 0x31005, 5 },
                       { 0,0 },
                    },
                 },
       /* mapping for hw test */
                { MSLA_HW_TEST      , {
                       { 0x41000, 0 },
                       { 0x41001, 1 },
                       { 0x41002, 2 },
                       { 0x41003, 3 },
                       { 0x41004, 4 },
                       { 0x41005, 5 },
                       { 0x41006, 6 },
                       { 0x41007, 7 },
                       { 0x41008, 8 },
                       { 0x41009, 9 },
                       { 0x4100A, 10 },
                       { 0x4100B, 11 },
                       { 0x4100C, 12 },
                       { 0x4100D, 13 },
                       { 0x4100E, 14 },
                       { 0x4100F, 15 },
                       { 0x41010, 16 },
                       { 0x41011, 17 },
                       { 0x41012, 18 },
                       { 0x41013, 19 },
                       { 0x41014, 20 },
                       { 0x41015, 21 },
                       { 0x41016, 22 },
                       { 0,  0 },
                     },
                 },
/* mapping for VPD test */
		{ MSLA_VPD_TEST,     {
			{ 0x51001, 0 },
                        { 0, 0      },
		   },
		},
       /* mapping for wrap test */
               { MSLA_WRAP_TEST    , {
                      { 0x61000, 0 },
                      { 0x61001, 1 },
                      { 0x61002, 2 },
                      { 0x61003, 3 },
                      { 0x61004, 4 },
                      { 0x61005, 5 },
                      { 0x62000, 6 },
                      { 0x62001, 7 },
                      { 0x62002, 8 },
                      { 0, 0        },
                    },
               },
    /* mapping for  intr. test */
               { MSLA_INTR_TEST    ,  {
                     { 0x71000, 0 },
                     { 0x71001, 1 },
                     { 0x71002, 2 },
                     { 0x72000, 3 },
                     { 0x72001, 4 },
                     { 0x72002, 5 },
                     { 0x73000, 6 },   
                     { 0x73001, 7 },   
                     { 0  , 0     },
                    },
               },
    /* mapping for dma test */
               { MSLA_DMA_TEST     ,  {
                       { 0x81000, 0 },
                       { 0x81001, 1 },
                       { 0x82000, 2 },
                       { 0x82001, 3 },
                       { 0, 0       },
                    },
               },
/* end of array */
               { 0, {  
                      {0,0},
                    },
                },
   };
 
struct fru_bucket frub_postest[]=
{
    /*........ . LED  R.code    msg no.           FRU buckets   ........*/

        { "", FRUB1,  LED,  0x101, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x102, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x103, DMSLA_DA_MSG_100 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x104, DMSLA_DA_MSG_101 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x105, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x106, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x107, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x108, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x109, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x110, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x111, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x112, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x113, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x114, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x115, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x116, DMSLA_DA_MSG_102 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x117, DMSLA_DA_MSG_102 ,  { FRU1 },},
};
struct fru_bucket frub_memtest[]=
{

        { "", FRUB1,  LED,  0x118, DMSLA_DA_MSG_200 ,  { FRU2 FRU3 },},
        { "", FRUB1,  LED,  0x119, DMSLA_DA_MSG_201 ,  { FRU2 FRU3 },},
        { "", FRUB1,  LED,  0x120, DMSLA_DA_MSG_202 ,  { FRU2 FRU3 },},
        { "", FRUB1,  LED,  0x121, DMSLA_DA_MSG_202 ,  { FRU2 FRU3 },},
};
 
struct fru_bucket frub_ioregtest[]=
{
        { "", FRUB1,  LED,  0x122, DMSLA_DA_MSG_300 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x123, DMSLA_DA_MSG_300 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x124, DMSLA_DA_MSG_300 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x125, DMSLA_DA_MSG_300 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x126, DMSLA_DA_MSG_300 ,  { FRU1 },},
        { "", FRUB1,  LED,  0x127, DMSLA_DA_MSG_301 ,  { FRU1 },},

};
struct fru_bucket frub_hwtest[]=
{
        { "", FRUB1,  LED,  0x128, DMSLA_DA_MSG_400 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x129, DMSLA_DA_MSG_401 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x130, DMSLA_DA_MSG_401 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x131, DMSLA_DA_MSG_401 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x132, DMSLA_DA_MSG_402 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x133, DMSLA_DA_MSG_402 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x134, DMSLA_DA_MSG_402 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x135, DMSLA_DA_MSG_402 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x136, DMSLA_DA_MSG_402 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x137, DMSLA_DA_MSG_402 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x138, DMSLA_DA_MSG_402 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x139, DMSLA_DA_MSG_403 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x140, DMSLA_DA_MSG_403 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x141, DMSLA_DA_MSG_404 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x142, DMSLA_DA_MSG_404 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x143, DMSLA_DA_MSG_404 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x144, DMSLA_DA_MSG_404 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x145, DMSLA_DA_MSG_404 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x146, DMSLA_DA_MSG_405 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x147, DMSLA_DA_MSG_405 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x148, DMSLA_DA_MSG_406 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x149, DMSLA_DA_MSG_407 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x150, DMSLA_DA_MSG_408 ,  { FRU1  },},

};

/*
 * SRN and message have been mapped to the best available ones, since SRN and 
 * messages have been frozen already and this test was added later on.
 */
struct fru_bucket frub_vpdtest[] =
{
        { "", FRUB1,  LED,  0x128, DMSLA_DA_MSG_406 ,  { FRU1  },},
};

struct fru_bucket frub_wraptest[] =
{
        { "", FRUB1,  LED,  0x151, DMSLA_DA_MSG_600 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x152, DMSLA_DA_MSG_601 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x153, DMSLA_DA_MSG_602 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x154, DMSLA_DA_MSG_602 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x155, DMSLA_DA_MSG_602 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x156, DMSLA_DA_MSG_602 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x157, DMSLA_DA_MSG_603 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x158, DMSLA_DA_MSG_604 ,  { FRU1  },},
        { "", FRUB1,  LED,  0x159, DMSLA_DA_MSG_605 ,  { FRU1  },},
};

struct fru_bucket frub_dmatest[] =
{
        {"", FRUB1,  LED,   0x160, DMSLA_DA_MSG_800 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x161, DMSLA_DA_MSG_801 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x162, DMSLA_DA_MSG_804 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x163, DMSLA_DA_MSG_805 ,  { FRU2 FRU3 },},
};
struct fru_bucket frub_intrtest[] =
{
        {"", FRUB1,  LED,   0x164, DMSLA_DA_MSG_700 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x165, DMSLA_DA_MSG_701 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x166, DMSLA_DA_MSG_702 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x167, DMSLA_DA_MSG_701 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x168, DMSLA_DA_MSG_701 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x169, DMSLA_DA_MSG_701 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x170, DMSLA_DA_MSG_701 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x171, DMSLA_DA_MSG_704 ,  { FRU2 FRU3 },},
};

struct fru_bucket frub_ela[] =
{
        {"", FRUB1,  LED,   0x172, DMSLA_DA_MSG_ELA1 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x174, DMSLA_DA_MSG_ELA1 ,  { FRU1 },},
        {"", FRUB1,  LED,   0x173, DMSLA_DA_MSG_ELA2 ,  { FRU2 FRU3 },},
        {"", FRUB1,  LED,   0x175, DMSLA_DA_MSG_ELA2 ,  { FRU1 },}
};
