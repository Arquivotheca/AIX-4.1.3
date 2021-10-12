static char sccsid[] = "@(#)92	1.4.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kc0init.c, libKJI, bos411, 9428A410j 7/23/92 02:51:01";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kc0init
 *
 * DESCRIPTIVE NAME:  INITIALIZE KKC ENVIRONMENT
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):    success
 *                    0x0108(WSPOVER):    overflow of work storage pool
 *                    0x0582(USR_LSEEK):  error of lseek()
 *                    0x0584(FZK_LSEEK):  error of lseek()
 *                    0x0682(USR_READ):   error of read()
 *                    0x0684(FZK_READ):   error of read()
 *                    0x0882(USR_LOCKF):  error of lockf()
 *                    0x0884(FZK_LOCKF):  error of lockf()
 *                    0x0b10(RQ_RECOVER): request of recovery()
 *                    0x7fff(UERROR):     unpredictable error
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES                                                    
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcrcb.h"                   /* Define Return Code Structure */

/*----------------------------------------------------------------------* 
 *      INCLUDE CONSTANT TABLES                                               
 *----------------------------------------------------------------------*/
#include   "_Kcfzk.t"                   /* Fuzokugo Data Table          */
#include   "_Kcjxt.t"                   /* Jititsu-go Position Table    */

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short _Kc0init(z_kcbptr)

struct KCB      *z_kcbptr;              /* get address of KCB           */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern void            _Kccinit();   /* initialize table header      */
   extern short           _Kczread();   /* read the dictionary file     */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcfkx.h"   /* Fuzokugo Kj hyoki eXchange table entry (FKX) */
#include   "_Kcfte.h"   /* Fuzokugo Table Entry (FTE)                   */
#include   "_Kcfwe.h"   /* Fuzokugo Work table Entry (FWE)              */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
#include   "_Kcgrm.h"   /* GRammer Map entry (GRM)                      */
#include   "_Kcjkj.h"   /* Jiritsugo KanJi hyoki table entry (JKJ)      */
#include   "_Kcjle.h"   /* Jiritsugo Long-word table Entry (JLE)        */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcmce.h"   /* Mora Code Table Entry (MCE)                  */
#include   "_Kcpte.h"   /* Path Table Entry (PTE)                       */
#include   "_Kcsei.h"   /* SEIsho buffer entry (SEI)                    */
#include   "_Kcsem.h"   /* SEisho Map entry (SEM)                       */
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */
#include   "_Kcymi.h"   /* YoMI buffer (YMI)                            */
#include   "_Kcymm.h"   /* YoMi Map entry (YMM)                         */
#include   "_Kcype.h"   /* Previous Yomi code table Entry (YPE)         */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   long                z_i;             /* declare loop counter         */
   long                z_totsz;         /* declare tatal size           */
   uschar              *z_wspcu;        /* declare current WSP pointer  */

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */

/*----------------------------------------------------------------------*
 *      INITIALIZE CONTENTS OF KCB
 *----------------------------------------------------------------------*/
   kcb.func    = 0x00;
   kcb.mode    = 0x00;
   kcb.cnvx    = 0x00;
   kcb.cnvi    = 0x00;
   kcb.charl   = 0x00;
   kcb.charr   = 0x00;
   kcb.ymill1  = 0x00;
   kcb.ymill2  = 0x00;
   kcb.seill   = 0x00;
   kcb.semll   = 0x00;
   kcb.ymmll   = 0x00;
   kcb.grmll   = 0x00;
   kcb.totcand = 0x00;
   kcb.reqcand = 0x00;
   kcb.outcand = 0x00;
   kcb.maxsei  = 0x00;
   kcb.env     = 0x00;
   kcb.possta  = 0x00;
   kcb.posend  = 0x00;

   kcb.ymimaxll= YMIMAX;
   kcb.seimaxll= SEIMAX;
   kcb.semmaxll= SEMMAX;
   kcb.ymmmaxll= YMMMAX;
   kcb.grmmaxll= GRMMAX;
   kcb.maxfkx  = MAXFKX;
   kcb.maxjtx  = MAXJTX;
   kcb.jthmxjte= MAXJTE;
   kcb.jkhmxjke= MAXJKJ;
   kcb.jlhmxjle= MAXJLE;
   kcb.fthmxfte= MAXFTE;
   kcb.fwhmxfwe= MAXFWE;
   kcb.bthmxbte= MAXBTE;
   kcb.pthmxpte= MAXPTE;
   kcb.ychmxyce= MAXYCE;
   kcb.yphmxype= MAXYPE;
   kcb.mchmxmce= MAXMCE;

/*----------------------------------------------------------------------*
 *      ASSIGN WORKING SPACE TO INTERNAL TABLES
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------------* 
    *      COMPARE THE SIZE OF WORKING STORAGE 
    *                        TO THE REQUESTED SIZE FOR INTERNAL TABLES
    *-------------------------------------------------------------------*/
   z_totsz = 0;
   z_totsz += (sizeof(jte)*kcb.jthmxjte);/* Jiritsu-go                  */
   z_totsz += (sizeof(jle)*kcb.jlhmxjle);/* Long Word                   */
   z_totsz += (sizeof(jkj)*kcb.jkhmxjke);/* Kanji Hyouki                */
   z_totsz += (sizeof(fte)*kcb.fthmxfte);/* Fuzoku-go                   */
   z_totsz += (sizeof(fwe)*kcb.fwhmxfwe);/* Fuzoku work                 */
   z_totsz += (sizeof(bte)*kcb.bthmxbte);/* Bunsetsu                    */
   z_totsz += (sizeof(pte)*kcb.pthmxpte);/* Path                        */
   z_totsz += (sizeof(yce)*kcb.ychmxyce);/* Yomi                        */
   z_totsz += (sizeof(ype)*kcb.yphmxype);/* Prev Yomi                   */
   z_totsz += (sizeof(mce)*kcb.mchmxmce);/* Mora                        */
   z_totsz += (sizeof(fkx)*kcb.maxfkx);  /* Fuzoku-go hyoki exchange    */
   z_totsz += kcb.ymimaxll;             /* yomi                         */
   z_totsz += kcb.seimaxll + 2;         /* Seisho                       */
   z_totsz += kcb.semmaxll + 2;         /* Seisho Map                   */
   z_totsz += kcb.ymmmaxll + 1;         /* Yomi Map                     */
   z_totsz += kcb.grmmaxll + 1;         /* Grammer Map                  */
   z_totsz += sizeof(gpw);              /* General purpose work area    */

   z_totsz += 3 * 19;                   /* boundary correcting          */
                                        /* 3byte * (number of tables)   */

   if (z_totsz > kcb.wsplen)
      return( WSPOVER );

   /*-------------------------------------------------------------------* 
    *        ASSIGN LOCATION IN WSP TO THE POINTERS    
    *-------------------------------------------------------------------*/
   z_wspcu = kcb.wsp;
   kcb.jthchh.chepool = (struct JTE *)z_wspcu;  /* Jiritsu-go           */
   z_wspcu += (sizeof(jte)*kcb.jthmxjte);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.jlhchh.chepool = (struct  JLE *)z_wspcu; /* Long Word            */
   z_wspcu += (sizeof(jle)*kcb.jlhmxjle);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.jkhchh.chepool = (struct  JKJ *)z_wspcu; /* Kanji Hyouki         */
   z_wspcu += (sizeof(jkj)*kcb.jkhmxjke);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.fthchh.chepool = (struct  FTE *)z_wspcu; /* Fuzoku-go            */
   z_wspcu += (sizeof(fte)*kcb.fthmxfte);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.fwhchh.chepool = (struct  FWE *)z_wspcu; /* Fuzoku work          */
   z_wspcu += (sizeof(fwe)*kcb.fwhmxfwe);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.bthchh.chepool = (struct  BTE *)z_wspcu; /* Bunsetsu             */
   z_wspcu += (sizeof(bte)*kcb.bthmxbte);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.pthchh.chepool = (struct  PTE *)z_wspcu; /* Path                 */
   z_wspcu += (sizeof(pte)*kcb.pthmxpte);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.ychyce = (struct  YCE *)z_wspcu; /* Yomi                         */
   z_wspcu += (sizeof(yce)*kcb.ychmxyce);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.yphype = (struct  YPE *)z_wspcu; /* Prev Yomi                    */
   z_wspcu += (sizeof(ype)*kcb.yphmxype);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.mchmce = (struct  MCE *)z_wspcu; /* Mora                         */
   z_wspcu += (sizeof(mce)*kcb.mchmxmce);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.fkxfkx = (struct  FKX *)z_wspcu; /* Fuzoku-go hyoki exchang      */
   z_wspcu += (sizeof(fkx)*kcb.maxfkx);
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.ymiaddr= (struct  YMI *)z_wspcu; /* Yomi                         */
   z_wspcu += kcb.ymimaxll;
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.seiaddr= (struct  SEI *)z_wspcu; /* Seisho                       */
   z_wspcu += kcb.seimaxll + 2;
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.semaddr= (struct  SEM *)z_wspcu; /* Seisho Map                   */
   z_wspcu += kcb.semmaxll + 2;
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.ymmaddr= (struct  YMM *)z_wspcu; /* Yomi Map                     */
   z_wspcu += kcb.ymmmaxll + 1;
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.grmaddr= (struct  GRM *)z_wspcu; /* Grammer Map                  */
   z_wspcu += kcb.grmmaxll + 1;
   z_wspcu += (4-(long)z_wspcu%4)&3;

   kcb.gpwgpe = (struct  GPW *)z_wspcu;  /* General purpose work area   */
   z_wspcu += sizeof(gpw);
   z_wspcu += (4-(long)z_wspcu%4)&3;

/*----------------------------------------------------------------------* 
 *       INTIALIZE TABLE HEADER                                
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------------* 
    *        INTIALIZE ALL WSP                                           
    *-------------------------------------------------------------------*/
   z_wspcu = kcb.wsp;

   for( z_i = 0; z_i < ( kcb.wsplen - 1 ); z_i++, z_wspcu++ )
      *z_wspcu = 0;

   /*-------------------------------------------------------------------* 
    *        INTIALIZE JTE TABLE                                 
    *-------------------------------------------------------------------*/
   _Kccinit( &kcb.jthchh, kcb.jthchh.chepool,
            (short)sizeof(jte), kcb.jthmxjte);

   /*-------------------------------------------------------------------* 
    *        INTIALIZE JLE TABLE                                         
    *-------------------------------------------------------------------*/
   _Kccinit(&kcb.jlhchh,kcb.jlhchh.chepool,
            (short)sizeof(jle),kcb.jlhmxjle);

   /*-------------------------------------------------------------------* 
    *        INTIALIZE JKJ TABLE                                         
    *-------------------------------------------------------------------*/
   _Kccinit(&kcb.jkhchh,kcb.jkhchh.chepool,
            (short)sizeof(jkj),kcb.jkhmxjke);

   /*-------------------------------------------------------------------* 
    *        INTIALIZE FTE TABLE                                         
    *-------------------------------------------------------------------*/
   _Kccinit(&kcb.fthchh,kcb.fthchh.chepool,
            (short)sizeof(fte),kcb.fthmxfte);

   /*-------------------------------------------------------------------* 
    *        INTIALIZE FWE TABLE                                         
    *-------------------------------------------------------------------*/
   _Kccinit(&kcb.fwhchh,kcb.fwhchh.chepool,
            (short)sizeof(fwe),kcb.fwhmxfwe);

   /*-------------------------------------------------------------------* 
    *        INTIALIZE BTE TABLE                                         
    *-------------------------------------------------------------------*/
   _Kccinit(&kcb.bthchh,kcb.bthchh.chepool,
            (short)sizeof(bte),kcb.bthmxbte);

   /*-------------------------------------------------------------------* 
    *        INTIALIZE PTE TABLE                                         
    *-------------------------------------------------------------------*/
   _Kccinit(&kcb.pthchh,kcb.pthchh.chepool,
            (short)sizeof(pte),kcb.pthmxpte);

/*----------------------------------------------------------------------* 
 *        INITIALIZE CONTROL VARIABLES IN GPW                          
 *----------------------------------------------------------------------*/
   gpwptr1 = kcb.gpwgpe;                /* set base address of GPW      */
   gpw.tbflag   = 1;                    /* Single phrase. Perfomance    */
                                        /* guarantee for floppy         */
   gpw.pendpos  = -2;                   /* Adjunct ayalysis position    */
   gpw.moraof2p = -1;                   /* Last pos. of 2nd phrase isn't*/
                                        /* defined.                     */
   gpw.pmode    = 0x99;                 /* Previous Mode                */
   gpw.pkakutei = PKAKNON;              /* Previous Kakutei status      */
   gpw.hykilvl  = 1;                    /* hyoki level                  */
   gpw.lastbte  = NULL;                 /* last created busetsu address */
   gpw.accfirm  = 0;                    /* internal kakutei no. yomi    */
   gpw.leftflg  = OFF;                  /* leftword flg                 */
   gpw.kakuflg  = OFF;                  /* internal kakutei flg         */
   gpw.pthpphin = 0;                    /* prev. hinsi                  */
   gpw.ppdflag[0]= 0;                   /* prev. flag[0]                */
   gpw.ppdflag[1]= 0;                   /* prev. flag[1]                */
   gpw.ppsahen  = 0;                    /* prev. sahen                  */

/*----------------------------------------------------------------------* 
 *       ASSIGN LOCATION OF FUZOKU-GO TO THE POINTERS                
 *----------------------------------------------------------------------*/
   kcb.faxfax = faxtbl;               /* Addr of F attribute tbl indx   */
   kcb.faefae = faetbl;               /* Addr of F Attribute tbl        */
   kcb.flefle = fletbl;               /* Addr of F Linkgae tbl          */
   kcb.fpefpe = fpetbl;               /* Addr of F Penalty adjustment tb*/
   kcb.fkjfkj = (union	FKJ *)fkjtbl; /* Addr of F KJ hyoki table   $01 */
   kcb.fryfry = frytbl;               /* Addr of F Reversed Yomi table  */

   kcb.maxfax = MAXFAX;               /* MAX  of F attribute tbl indx   */
   kcb.maxfae = MAXFAE;               /* MAX  of F Attribute tbl        */
   kcb.maxfle = MAXFLE;               /* MAX  of F Linkgae tbl          */
   kcb.maxfpe = MAXFPE;               /* MAX  of F Penalty adjustment tb*/
   kcb.maxfkj = MAXFKJ;               /* MAX  of F KJ hyoki table       */
   kcb.maxfry = MAXFRY;               /* MAX  of F Reversed Yomi table  */

   kcb.jtxjtx = jtxtbl;               /* Addr of J Tag eXchange tbl     */

/*----------------------------------------------------------------------* 
 *       RETURN                                  
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}                
