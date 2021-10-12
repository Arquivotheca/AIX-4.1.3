/* @(#)76	1.2 6/4/91 15:20:40 */

/*    
 * COMPONENT_NAME: (libKJI) Japanese Input Method 
 *
 * FUNCTIONS: kana-Kanji-Conversion (KKC) Library
 * 
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcrcb.h
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:
 *
 ******************** END OF SPECIFICATIONS *****************************/
/************************************************************************
 * Return Code Block    (RCB) 
 ************************************************************************/
struct RETCBKNJ {                       /*                              */
   short  rc;                           /* return code                  */
   short  kjno;                         /* no of kj to have been proc   */
};

struct RETBPATH {                       /* 06/10/87 temporary defined   */
                                        /* for compile  by c.o          */
   short  rc;                           /* return code                  */
   short  entry;                        /* path enrty                   */
};

struct RETBSBST {
   short  rc;                           /* return code                  */
   struct BTE *bteptr;                  /* BTE pointer                  */
};

struct RETCFKNJ {                       /*                              */
   short  rc;                           /* return code                  */
   short  kjno;                         /* no of kj to have been proc   */
   short  mrno;                         /* no of mora to have been proc */
   short  homoni;                       /* homonim flag 1:exists 0:no   */
};

struct RETCFPTH {                       /*                              */
   short  rc;                           /* return code                  */
   short  pen;                          /* penalty                      */
   struct PTE *pteptr;                  /* PTE ptr                      */
};

struct RETCGET
{
   short        rc;                     /* return code                  */
   struct CHE   *cheptr;                /* pointer of CHE               */
};

struct RETCJKNJ {                       /*                              */
   short  rc;                           /* return code                  */
   short  kjno;                         /* no of kj to have been proc   */
   short  mrno;                         /* no of mora to have been proc */
};

struct RETCSWPP
{
   short        num;                    /* deleted number               */
   struct PTE   *pteptr;                /* pointer of PTE               */
};

struct RETFCHEK
{
   short        rc;                     /* return code                  */
   short        endmora;                /* mora end pointer             */
};

struct RETFFCON {
   short  rc;                           /* return code                  */
   short  oflag;                        /* oflag                        */
};

struct RETFHYKI {
   short  rc;                           /* return code                  */
   struct FKX *fkxptr;                  /* FKX pointer                  */
};

struct RETFSTMP
{
   short        rc;                     /* return code                  */
   struct  FWE  *fweptr;                /* FWE ptr                      */
};

struct RETJCHEK
{
   short        rc;                     /* return code                  */
   short        endmora;                /* mora end pointer             */
};

struct  RETJMKNJ
{
   short rc;                            /* return code                  */
                                        /*    Z_OK      0 : normal      */
                                        /*    Z_OVRFLW  1 : JKJ ovrflw  */
   struct JKJ *jkjptr;                  /* point to the 1st JKJ kanji   */
};

struct RETJSFLG {
   short   rc;                          /* return code                  */
   short   newoff;                      /* new offset to point tne nxt  */
   uschar   flag;                       /* dflag                        */
};

struct RETJSGT1                         /*    rc for kkcjsgt1           */
{
   short   rc;
   short   newoff;                      /*  return code new offset      */
};

struct  RETJSKNJ
{
   short rc;                            /* return code                  */
                                        /*    Z_OK      0 : normal      */
                                        /*    Z_OVRFLW  1 : JKJ ovrflw  */
   struct JKJ *jkjptr;                  /* point to the 1st JKJ kanji   */

   short newoff;                        /* point the nxt fld            */
                                        /*           past kanji fld     */
};

struct RETJSTAG {
   short  rc;                           /* return code                  */
   uschar flag[2];                      /* dflag                        */
   uschar hinpos;                       /* hinshi code                  */
   uschar type;                         /* type code                    */
   short  newoff;                       /* new offset to point to nxt   */
};

struct RETJUGT1                         /*    rc for kkcjugt1           */
{
   short   rc;
   short   newoff;                      /*  return code new offset      */
};

struct RETJUHSH { 
   short  rc;                           /* return code                  */
   short  rrn[5];                       /* rerative record number       */
};

struct  RETJUKNJ
{
   short rc;                            /* return code                  */
                                        /*    Z_OK      0 : normal      */
                                        /*    Z_OVRFLW  1 : JKJ ovrflw  */
   struct JKJ *jkjptr;                  /* point to the 1st JKJ kanji   */

   short newoff;                        /* point the nxt fld            */
                                        /*           past kanji fld     */
};

struct RETNCUTB {
   short   rc;
   short   delno;
};

struct RETNMTCH {                       /*                              */
   short  rc;                           /* return code                  */
   struct BTE *bteptr;                  /* BTE ptr                      */
   short  bteno;                        /* BTE entry no (1-             */
};

struct RETXCMPK {
   short   res;
   short   rc;
};

struct RETXCMPY {
   short   res;
   short   len;
};
