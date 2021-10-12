/* @(#)01	1.2 6/4/91 15:27:49 */

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
 * MODULE NAME:       _Kcwsp.h
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
/**************************************************************************
 * Fuzokugo Kanji hyoki eXchange table entry (FKX)
 **************************************************************************/
struct FKX
{
 uschar      stap;                      /* mora position, start mora pos  */
                                        /* for this F.                    */
 uschar      flag;                      /* ctl flag                       */
#define      FKXCONT 0x80               /* F chain continuation flag.     */
                                        /* Other fuzokugo follows this F. */
                                        /* If FKXCONT is off then it means*/
                                        /* this is the last F of the      */
                                        /* F chain.                       */
#define      FKXKJYN 0x40               /* KJ hyoki exists for this F.    */
                                        /* If FKXKJYN is off it means     */
                                        /* no KJ hyoki exists for this F. */
 short       kjfp;                      /* FKJ entry no. Valid when       */
                                        /* FKXKJYN is on.                 */
};
 
 
/**************************************************************************/
/* Fuzokugo Table Entry (FTE)                                             */
/**************************************************************************/
struct FTE
{
 short       chef;                      /* next ele sub.(forward sub.)  */
                                        /* NEGA if LAST free element    */
 short       cheb;                      /* prev ele sub.(backward sub.) */
                                        /* NEGA if free elemet          */
 unsigned    actv :1;                   /* active element flag, 1=yes   */
 unsigned    top  :1;                   /* top  ele in the chain flag,  */
                                        /* only used for active element */
 unsigned    last :1;                   /* last ele in the chain flag   */
 unsigned    rsv01:13;                  /* reserved                     */
 
 short       stap;                      /* mora pos, start mora         */
 short       fhtx;                      /* FKJ                          */
 short       hinl;                      /* hinshi code, left            */
 short       hinr;                      /* hinshi code, right           */
 short       pen;                       /* tot penalty for this fuzokugo*/
 uschar      setsu[3];                  /* connectability, 24bit format */
 uschar      rsv02;                     /* reserved */
};
 
/************************************************************************/
/* Fuzokugo Work table Entry (FWE) ------- Old TFT                      */
/************************************************************************/
struct FWE
{
 short       chef;                      /* next ele sub.(forward sub.)  */
                                        /* NEGA if LAST free element    */
 short       cheb;                      /* prev ele sub.(backward sub.) */
                                        /* NEGA if free elemet          */
 unsigned    actv :1;                   /* active element flag, 1=yes   */
 unsigned    top  :1;                   /* top  ele in the chain flag,  */
                                        /* only used for active element */
 unsigned    last :1;                   /* last ele in the chain flag   */
 unsigned    rsv01:13;                  /* reserved                     */
 
 struct FWE *prnt;                      /* addr of parent FWE           */
 short       stap;                      /* mora pos, start mora         */
 short       fno;                       /* Fuzokugo Number              */
 short       kjn;                       /* kj hyoki pointer ??          */
 short       pen;                       /* penalty value                */
};
 
/**************************************************************************
 * CHain Header map (CHH)
 **************************************************************************/
struct CHH
{
 uschar     *chepool;                   /* addr of ele pool               */
                                        /* char -> uschar 07/07/87        */
 short       fre;                       /* 1st ele addr on the free chain */
                                        /* NULL if chain is empty         */
 short       act;                       /* 1st ele addr on the actv chain */
                                        /* NULL if chain is empty         */
 short       size;                      /* element size in bytes          */
 short       mxele;                     /* max no of elements             */
 short       actv;                      /* no of active elements          */
};
 
/**************************************************************************
 * CHain Element map (CHE)
 **************************************************************************/
struct CHE
{
 short       chef;                      /* next ele ptr (forward ptr)     */
                                        /* NULL if LAST free element      */
 short       cheb;                      /* prev ele ptr (backward ptr)    */
                                        /* NULL if free elemet            */
 unsigned    actv :1;                   /* active element flag, 1=yes     */
 unsigned    top  :1;                   /* top  ele in the chain flag,    */
                                        /* only used for active element   */
 unsigned    last :1;                   /* last ele in the chain flag     */
 unsigned    rsv01:13;                  /* reserved                       */
};
 
/************************************************************************
 * Jiritsugo KanJi hyoki table entry (JKJ)
 ************************************************************************/
struct JKJ
{
 short       chef;                      /* next ele sub.(forward sub.)  */
                                        /* NEGA if LAST free element    */
 short       cheb;                      /* prev ele sub.(backward sub.) */
                                        /* NEGA if free elemet          */
 unsigned    actv :1;                   /* active element flag, 1=yes   */
 unsigned    top  :1;                   /* top  ele in the chain flag,  */
                                        /* only used for active element */
 unsigned    last :1;                   /* last ele in the chain flag   */
 unsigned    rsv01:13;                  /* reserved                     */
 
 uschar      kj[2];                     /* kanji hyoki string for a J.  */
                                        /* MSB for the last kj in the   */
                                        /* hyoki string is set off(MSB=1)*/
                                        /*                              */
};
 
/************************************************************************
 * Jiritsugo Table Entry (JTE)
 ************************************************************************/
struct JTE
{
 short       chef;                      /* next ele sub.(forward sub.)  */
                                        /* NEGA if LAST free element    */
 short       cheb;                      /* prev ele sub.(backward sub.) */
                                        /* NEGA if free elemet          */
 unsigned    actv :1;                   /* active element flag, 1=yes   */
 unsigned    top  :1;                   /* top  ele in the chain flag,  */
                                        /* only used for active element */
 unsigned    last :1;                   /* last ele in the chain flag   */
 unsigned    rsv01:13;                  /* reserved                     */
 
 struct JKJ *jkjaddr;                   /* Addr of assoc J kj hyoki.    */
                                        /* If the J does not have KJ hyk*/
                                        /* (hiragana-go or katakana-go) */
                                        /* tjis field is set to 0 (NULL)*/
 short       usage;                     /* Tot no of occurrences in the */
                                        /* current B table.             */
 uschar      hinpos;                    /* position of hinshi code(24 bit)*/
                                        /*                 in pos table */
 uschar      stap;                      /* mora pos, start mora         */
 uschar      len;                       /* length, in no of mora"s      */
 uschar      dtype;                     /* type & grammar code          */
 uschar      dflag[2];                  /* dflag, see nxt stmts for dtail*/
 /* unsigned bit0  :1;                     1=this byte contains dflag   */
 /* unsigned inf01 :4;                     grammar information          */
 /* unsigned freq  :3;                     frequency, 000(hi) - 111(lo) */
 /* unsigned bit1  :1;                     1=this byte contains dflag   */
 /* unsigned Inf02 :7;                     grammar information          */
};
 
/************************************************************************
 * Jiritsugo Long-word table Entry (JLE)
 ************************************************************************/
struct JLE
{
 short       chef;                      /* next ele sub.(forward sub.)  */
                                        /* NEGA if LAST free element    */
 short       cheb;                      /* prev ele sub.(backward sub.) */
                                        /* NEGA if free elemet          */
 unsigned    actv :1;                   /* active element flag, 1=yes   */
 unsigned    top  :1;                   /* top  ele in the chain flag,  */
                                        /* only used for active element */
 unsigned    last :1;                   /* last ele in the chain flag   */
 unsigned    rsv01:13;                  /* reserved                     */
 
 struct JKJ *jkjaddr;                   /* Addr of assoc kj hyoki.      */
                                        /* If the J does not have KJ hyki*/
                                        /* (hiragana-go or katakana-go) */
                                        /* this field is set to 0 (NULL).*/
 uschar      hinpos;                    /* hinshi code, in 24bit format */
 uschar      len;                       /* length in no of mora"s,      */
                                        /* of the J this JLE represents.*/
 uschar      dtype;                     /* type & grammar code          */
 uschar      dflag[2];                  /* dflag, see nxt stmts for dtail*/
 /* unsigned bit0  :1;                     1=this byte contains dflag   */
 /* unsigned inf01 :4;                     grammar information          */
 /* unsigned freq  :3;                     frequency, 000(hi) - 111(lo) */
 /* unsigned bit1  :1;                     1=this byte contains dflag   */
 /* unsigned Inf02 :7;                     grammar information          */
 uschar      stap;                      /* mora pos, points to leading 3*/
                                        /* yomi"s of the J this JLE     */
                                        /* represents.                  */
 uschar      yomi[12];                  /* yomi in mora code,           */
                                        /* excluding leading 3 mora"s.  */
};
 
/************************************************************************
 * Bunsetsu Table Entry (BTE)
 ************************************************************************/
struct BTE
{
 short       chef;                      /* next ele sub.(forward sub.)  */
                                        /* NEGA if LAST free element    */
 short       cheb;                      /* prev ele sub.(backward sub.) */
                                        /* NEGA if free elemet          */
 unsigned    actv :1;                   /* active element flag, 1=yes   */
 unsigned    top  :1;                   /* top  ele in the chain flag,  */
                                        /* only used for active element */
 unsigned    last :1;                   /* last ele in the chain flag   */
 unsigned    rsv01:13;                  /* reserved                     */
 
 struct JTE *jteaddr;                   /* addr of associated jiritsugo */
 short       stap;                      /* mora pos, of start mora      */
 short       endp;                      /* mora pos, of end   mora      */
 short       kjh1;                      /* KHT indx no, of 1st KJ fuzokg*/
 short       kjh2;                      /* KHT indx no, of 2nd KJ fuzokg*/
 uschar      kjf1;                      /* mora pos,    of 1st KJ fuzokg*/
 uschar      kjf2;                      /* mora pos,    of 2nd KJ fuzokg*/
 uschar      hinl;                      /* hinshi code, left            */
 uschar      hinr;                      /* hinshi code, right           */
 uschar      usage;                     /* noof times, buns occ in path */
 uschar      pen;                       /* total penalty for this bunset*/
 uschar      fzkflg;                    /* bit            ON  /   OFF   */
                                        /* 3:Kanji Hyoki Not Exist/ Exst*/
                                        /* if bit 3 is OFF  then        */
                                        /* 2:Kanji Hyoki   2  /    1    */
                                        /* 1:2nd KJ Hoki Used /Not Used */
                                        /* 0:1nd KJ Hoki Used /Not Used */
 uschar      dmyflg;                    /* if this bunsetsu if dummy for*/
                                        /* left char  ON or OFF         */
};
 
/************************************************************************
 * Path Table Entry (PTE)
 ************************************************************************/
struct PTE
{
 short       chef;                      /* next ele sub.(forward sub.)  */
                                        /* NEGA if LAST free element    */
 short       cheb;                      /* prev ele sub.(backward sub.) */
                                        /* NEGA if free elemet          */
 unsigned    actv :1;                   /* active element flag, 1=yes   */
 unsigned    top  :1;                   /* top  ele in the chain flag,  */
                                        /* only used for active element */
 unsigned    last :1;                   /* last ele in the chain flag   */
 unsigned    rsv01:13;                  /* reserved                     */
 
 struct BTE *bteaddr[6];                /* addr of assoc bunsetsu tbl en*/
 short       endp;                      /* mora pos, of end mora        */
 short       pen;                       /* total penalty for this path  */
};
 
/**************************************************************************/
/* Yomi Code table Entry (YCE) map                                        */
/**************************************************************************/
struct YCE
{
 uschar      yomi;                      /* yomi in 7bit internal code.    */
                                        /* bit0 is always zero.           */
};
 
/**************************************************************************/
/* Previous Yomi code table Entry (YPE) map                               */
/**************************************************************************/
struct YPE
{
 uschar      prev;                      /* previous yomi save area.       */
};
 
/**************************************************************************/
/* Mora Code table Entry (MCE)                                            */
/**************************************************************************/
struct MCE
{
   struct YCE *yceaddr;                 /* YCE addr, ie yomi pointer      */
   uschar      code;                    /* mora code                      */
   uschar      jdok;                    /* dictionary consultation status */
                                        /*  x00: no need to search        */
                                        /*  x01: search needed            */
                                        /*  x02: searched, zempo ichi fnd */
                                        /*  x09: searched, fnd or not fnd */
   short       maxln;                   /* maximum mora length of candidates
                                         *  whose mora length is longest
                                         */
};
 
/*------------------------------------------------------------------------*
 * General Purpose Workarea                                               *
 *------------------------------------------------------------------------*/
struct GPW {
 struct BTE     *lastbte;               /* Latest created BTE pointer     */
 
 short          pendpos;                /* previous fzk. analasys end pos.*/
 
 short          accfirm;                /* Accumulated Firmed Yomi Length */
                                        /* when Int Fimr up is made, then */
                                        /* this value will have len of    */
                                        /* firmed yomi                    */
 short          moraof2p;               /* distance from the longest 2nd  */
                                        /* bunsetsu to current point      */
 uschar         pmode ;                 /* previous mode                  */
 
 uschar         pkakutei;               /* previous kakutei flag          */
 
 uschar         tbflag;                 /* tanbunsetsu flag               */
 
 uschar         kakuflg;                /* kyousei kakutei flag           */
 
 uschar         hykilvl;                /* Current F hyoki level          */
 
 uschar         leftflg;                /* left word flag                 */
 
 uschar         pthpphin;               /* backwd hinshi of prev bunsetsu */
 
 uschar         ppdflag[2];             /* backwd dflag  of prev bunsetsu */
 
 uschar         ppsahen;                /* backwd sahen  of prev bunsetsu */
 
 
};
 
/************************************************************************/
/* YoMI buffer (YMI) format						*/
/************************************************************************/
 struct YMI
{
 uschar      yomi[vary];                /* yomi                         */
};
 
/************************************************************************/
/* YoMi Map entry (YMM) format						*/
/************************************************************************/
 struct YMM
{
 uschar      ll;                        /* entry length (including LL).   */
 uschar      byte[vary];                /* bit string                     */
};
 
/************************************************************************/
/* SEIsho buffer entry (SEI) format					*/
/************************************************************************/
 struct SEI
{
 uschar      ll[2];                     /* entry length (including LL).   */
                                        /* short format is not used due   */
                                        /* to compatibility reason.       */
 uschar      kj[vary][2];               /* seisho, in KJ format           */
};
 
/************************************************************************/
/* SEisho Map entry (SEM) format					*/
/************************************************************************/
 struct SEM
{
 uschar      ll[2];                     /* entry length (including LL).   */
                                        /* short format is not used due   */
                                        /* to compatibility reason        */
 uschar      code[vary];                /* Seisho Zokusei Code            */
};
 
/************************************************************************/
/* GRammer Map entry (GRM) format					*/
/************************************************************************/
 struct GRM
{
 uschar      ll;                        /* entry length (including LL).   */
 uschar      byte[vary];                /* Hinshicode for J or            */
                                        /* Encoded Fuzokugo Number for F. */
};
