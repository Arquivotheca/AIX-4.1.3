/* @(#)98	1.2 6/4/91 15:26:04 */

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
 * MODULE NAME:       _Kctbl.h
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
/**************************************************************************/
/* Fuzokugo Attribute table Entry (FAE) format                            */
/* FAE entry numbers (0-) are also referred to as FUZOKUGO Numbers        */
/**************************************************************************/
struct FAE
{
 uschar      hin;        /* fzknxt */   /* Hinshi code of this fuzokugo   */

 uschar      len;        /* nstr   */   /* actual fuzokugo length - 1     */

 uschar      pen;        /* fzkjno */   /* penalty value (p2/q2)          */

 uschar      rsv01;                     /* reserved                       */

 short       frypos;     /* frevix */   /* starting position (0-) of the  */
                                        /* associated reversed yomi (FRY) */

 short       fkjpos;     /* fzkkix */   /* starting position (0-) of the  */
                                        /* associated kj hyoki (FKJ)      */

 short       flendx;     /* fzkpvi */   /* entry no (0-) of the associatd */
                                        /* linkage table entry (FLE)      */

 short       rsv02;                     /* reserved                       */
};

/**************************************************************************/
/* Fuzokugo Attribute table indeX (FAX)                                   */
/*                                                                        */
/* Entry numbers of FAX (0-) corresponds mora codes.                      */
/* The last letter of the fuzokugo currently being processed should be    */
/* used when looking up the Fuzokugo attribute table via this index.      */
/**************************************************************************/
struct FAX
{
 short       faendx;     /* fzkx1   */  /* Entry no (0-) of the first FAE */
                                        /* which contains the attribute   */
                                        /* information of a fuzokugo      */
                                        /* which ends in the letter       */
                                        /* implied by the current FAX     */
                                        /* entry no (mora code).          */
                                        /* The next faendx points to the  */
                                        /* beginning FAE of the other set */
                                        /* of fuzokugo"s ending in another*/
                                        /* letter. Thus all the FAEs upto */
                                        /* one before this entry forms the*/
                                        /* possible candidates for the    */
                                        /* current FAX entry.             */
};

/**************************************************************************/
/* Fuzokugo KanJi hyoki table (FKJ)                                       */
/**************************************************************************/
struct FKH                              /* FKJ Header declare       */
{                                       /* modified 06/11/87 by Y.K */
 uschar    hdr1;                        /* modified 07/10/87 by OH  */
                     /* unsigned    lvll:4;     */
                     /* unsigned    lvlr:4;     */
 uschar    hdr2;
                     /* unsigned    nmora:4;    */
                     /* unsigned    nkj:4;      */
} ;

struct FKE
{
 uschar      kj[2] ;
} ;

union  FKJ
{
 struct FKH  hdr ;
 struct FKE  elm ;
} ;

/**************************************************************************/
/* Fuzokugo Linkage table Entry (FLE) format                              */
/**************************************************************************/
struct FLE
{
 short       lkvt[7];    /* fzkpvtx */  /* 128 bits linkage vector.       */
                                        /* Each bit corresponds to one of */
                                        /* the hinshi codes 1-128. Bit 0  */
                                        /* shows the connectability to    */
                                        /* the hinshi code 1.             */

 short       fpendx;     /* fzklvli */  /* Starting entry no (0-) of the  */
                                        /* associated penalty adjustment  */
                                        /* table entry (FPE).             */

 short       rsv01;                     /* reserved                       */
};

/**************************************************************************/
/* Fuzokugo Penalty adjustment table Entry (FPE) format                   */
/**************************************************************************/
struct FPE
{
 short       hin;        /* lvlpos  */  /* Hinshi code of the preceding   */
                                        /* word. The starting entry no    */
                                        /* is indicated at the fle.fpendx */
                                        /* field and related entries will */
                                        /* follow upto the point where    */
                                        /* the hinshi is indicated as     */
                                        /* positive value. All hinshi code*/
                                        /* values for non-last entries are*/
                                        /* shown as minus values.         */

 short       pen;        /* lvlv    */  /* Penalty adjustment value       */
                                        /* (p4/p5/q4/q5) for when the     */
                                        /* current fuzokugo follows the   */
                                        /* word (maybe other fuzokugo or  */
                                        /* jiritsugo) indicated by the    */
                                        /* fpe.hin field.                 */
};

/**************************************************************************/
/* Fuzokugo Reversed Yomi table (FRY)                                     */
/**************************************************************************/
struct FRY
{
 uschar      kana;     /* fzkrev  */    /* Yomi in mora code.             */
                                        /* fae.frypos points to the start */
                                        /* position of the reversed yomi. */
                                        /* For example, if the fuzokugo is*/
                                        /* "zo.n.ji.a.ge", the fae.frypos */
                                        /* points to the "a". The four    */
                                        /* entries following the "a"      */
                                        /* contains "ji","n" and zo"      */
                                        /* respectively. "ge" is not      */
                                        /* stored in this tbl explicitly, */
                                        /* instead, it is considered to be*/
                                        /* stored in the FAT index table  */
                                        /* implicitly.                    */
};

/**************************************************************************/
/* Jiritsugo Tag eXchange table (JTX)                                     */
/**************************************************************************/
struct JTX
{
 uschar      pos[3];                    /* Tot no of occurrences in the   */
};
