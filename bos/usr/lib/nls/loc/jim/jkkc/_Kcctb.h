/* @(#)48	1.2 6/4/91 10:23:05 */

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
 * MODULE NAME:       _Kcctb.h
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
/************************************************************************/
/* _Kkc _Control _Block (KCB)					*/
/*                                                                      */
/*    The KCB is a control block which is used to form a Standard KKC   */
/*  interface (OFKKC).  Because the Standard KKC Interface is supposed  */
/*  to be shared by many departments, any modification of the KCB must  */
/*  be made under the agreement of the related departments including    */
/*  IP E/S, TRL and YAMATO.                                             */
/************************************************************************/
struct KCB
{
 
 
 
 
/**************************************************************************/
/**************************************************************************/
/* Communication Section                                                  */
/*                                                                        */
/*    This section is used as communication area between OFKKC and the    */
/*  user.  When a user requests one of the OFKKC functions, appropriate   */
/*  fields in this KCB must be set prior to making a function call.       */
/*  Upon completion of a function request, the OFKKC places outputs       */
/*  in some of the KCB fields. No other section than this Communications  */
/*  Section is allowed to be referenced by the user. All other sections   */
/*  are for OFKKC internal use only.                                      */
/**************************************************************************/
/**************************************************************************/
uschar      func;                       /* function code                  */
#define     FUNOPEN    0x00             /* KKC open                       */
#define     FUNCONV    0x00             /* Initial conversion             */
#define     FUNNXTCV   0x01             /* Next conversion                */
#define     FUNPRVCV   0x02             /* Previous conversion            */
#define     FUNRTFCN   0x03             /* Return to first candidate      */
#define     FUNRTLCN   0x04             /* Return to last candidate       */
#define     FUNALLOP   0x05             /* All candidates open            */
#define     FUNALLFW   0x06             /* All candidate forward          */
#define     FUNALLBW   0x07             /* All candidate backward         */
#define     FUNSGLOP   0x08             /* Single kanji open              */
#define     FUNSGLFW   0x09             /* Single kanji forward           */
#define     FUNSGLBW   0x0A             /* Single kanji backward          */
#define     FUNLEARN   0x0B             /* Learning                       */
#define     FUNDCTIQ   0x0C             /* User Dictionary inquirery      */
#define     FUNDCTAD   0x0D             /* User Dictionary Addtion        */
#define     FUNOPNSD   0x0E             /* Open system dictionary         */
#define     FUNCLSSD   0x0F             /* Close system dictionary        */
#define     FUNOPURW   0x10             /* Open user dicttionary R/W      */
#define     FUNOPURO   0x11             /* Open user dicttionary R/O      */
#define     FUNCLSUD   0x12             /* Close user dicttionary         */
#define     FUNLRNWR   0x13             /* Writhe MRU data                */
#define     FUNRSRVD   0x14             /* RESERVED                       */
#define     FUNDCTRC   0x15             /* User dictionary recovery       */
#define     FUNINITW   0x16             /* Initialize work storage        */
 
/*------------------------------------------------------------------------*/
 uschar      mode;                      /* Conversion mode                */
#define     MODFUKU   0x00              /* Fukugou-go                     */
#define     MODTAN    0x01              /* Bunsetsu (Single)              */
#define     MODREN    0x02              /* Bunsetsu (Multiple)            */
#define     MODALKAN  0x0E              /* Alphabet & Kansuji             */
#define     MODALPHA  0x0F              /* Alphabet                       */
#define     MODRYAKU  0x10              /* Ryakushou                      */
#define     MODKANSU  0x11              /* Kansuji                        */
/*------------------------------------------------------------------------*/
 uschar      cnvx;                      /* Explicit req for kakutei       */
                                        /* x'00': no  (use KAKNO below)   */
                                        /* other: yes                     */
 
 uschar      cnvi;                      /* Req for internal kakutei       */
                                        /* (valid when cnvx=00)           */
                                        /* x'00': do not kakutei internlly*/
                                        /* other: do kakutei whenever     */
                                        /*        possible                */
 
#define      KAKNO    0x00
/*------------------------------------------------------------------------*/
 uschar      charl;                     /* Jishu-Code, left to the        */
                                        /*             current yomi       */
 
 uschar      charr;                     /* Jishu-Code, right to the       */
                                        /*             current yomi       */
/*------------------------------------------------------------------------*/
 struct YMI *ymiaddr;                   /* Addr of associated yomi buffer */
 short       ymimaxll;                  /* Max yomi buffer len, bytes     */
 short       ymill1;                    /* Length of input yomi in bytes  */
 short       ymill2;                    /* Offset to the J/F boundary     */
/*------------------------------------------------------------------------*/
 struct SEI *seiaddr;                   /* Addr of seisho buffer          */
 short       seimaxll;                  /* Max seisho buffer len, bytes   */
 short       seill   ;                  /* Current seisho len, bytes      */
/*------------------------------------------------------------------------*/
 struct SEM *semaddr;                   /* Addr of seisho map area        */
 short       semmaxll;                  /* Max seisho map area len, bytes */
 short       semll   ;                  /* Current seisho map len, bytes  */
/*------------------------------------------------------------------------*/
 struct YMM *ymmaddr;                   /* Addr of yomi map area          */
 short       ymmmaxll;                  /* Max yomi map area len, bytes   */
 short       ymmll   ;                  /* Current yomi map len, bytes    */
/*------------------------------------------------------------------------*/
 struct GRM *grmaddr;                   /* Addr of grammer map area       */
 short       grmmaxll;                  /* Max grammer map len, bytes     */
 short       grmll   ;                  /* Current grammer map len, bytes */
/*------------------------------------------------------------------------*/
 
 
 
 
 
 short       totcand;                   /* Total no of candidates for the */
                                        /* current environment.           */
 short       reqcand;                   /* No of candidates to be returnd */
                                        /* to the caller.                 */
 short       outcand;                   /* No of candidates in the current*/
                                        /* output buffer.                 */
 short       currfst;                   /* Number of the fast candidate   */
                                        /* in the current output buffer   */
 short       currlst;                   /* Number of the last candidate   */
                                        /* in the current output buffer   */
/*------------------------------------------------------------------------*/
 short       maxsei;                    /* Max seisho len, in bytes, for  */
                                        /* zenkoho operation.             */
/*------------------------------------------------------------------------*/
 short       retcd;                     /* Return code, major             */
 short       retcd2;                    /* Return code, minor (Reason c)  */
/*------------------------------------------------------------------------*/
 uschar      alpkan;                    /* status of alpha/kansu conv   */
                                        /*  0x01: alpha conv only       */
                                        /*  0x02: kansu conv only       */
                                        /*  0x03: alpha & kansu conv    */
 
 uschar      rsv01;                     /* reserved */
 
/**************************************************************************/
/**************************************************************************/
/* System Section                                                         */
/*                                                                        */
/*    This section is for OFKKC internal operations, thus no user program */
/*  is allowed to reference or change fields in this section.             */
/*    All field names, formats and meanings are subject to change without */
/*  any notification to the user.                                         */
/**************************************************************************/
/**************************************************************************/
 uschar     *myarea;                    /* Addr of environment descriptor */
                                        /* Control Block. All environment */
                                        /* dependent information which are*/
                                        /* required by the modules near   */
                                        /* the bottom of the program      */
                                        /* structure should be passed to  */
                                        /* them using the CB pointed to by*/
                                        /* this field.                    */
                                        /* As for the Tsukiji interface   */
                                        /* case, a MCB is used as the     */
                                        /* environment descriptor CB.     */
                                        /* Thus this field contains the   */
                                        /* pointer to the MCB.            */
 /*-----------------------------------------------------------------------*/
 struct FAX *faxfax;                    /* Addr of F attribute tbl indx   */
 short       maxfax;                    /* Max No of FAX                  */
 struct FAE *faefae;                    /* Addr of F Attribute tbl        */
 short       maxfae;                    /* Max No of FAE                  */
 struct FLE *flefle;                    /* Addr of F Linkgae tbl          */
 short       maxfle;
 struct FPE *fpefpe;                    /* Addr of F Penalty adjustment tb*/
 short       maxfpe;
 union  FKJ *fkjfkj;                    /* Addr of F KJ hyoki table       */
 short       maxfkj;
 struct FKX *fkxfkx;                    /* Addr of F KJ hyoki eXchange tbl*/
 short       maxfkx;
 short       fkxacfkx;                  /* No of currently active element */
 struct FRY *fryfry;                    /* Addr of F Reversed Yomi table  */
 short       maxfry;
 struct JTX *jtxjtx;                    /* Addr of J Tag eXchange tbl     */
 short       maxjtx;
 struct SXE *sxesxe;                    /* Addr of sys dict index buffer  */
 uschar     *sdesde;                    /* Addr of sys dict buffer        */
 uschar     *txetxe;                    /* Addr of single index buffer    */
 uschar     *tdetde;                    /* Addr of single buffer          */
 struct UXE *uxeuxe;                    /* Addr of user dict index buffer */
 uschar     *udeude;                    /* Addr of user dict buffer       */
 uschar     *mdemde;                    /* Addr of MRU buffer             */
 uschar     *dfgdfg;                    /* Addr of fuzoku dict buffer     */
 uschar     *wsp;                       /* Addr of Working Storage Pool   */
 long        wsplen;                    /* Working Stor Pool len, in bytes*/
 struct GPW *gpwgpe;                    /* Addr of General Purpose Wkarea */
 long        gpwlen;                    /* GPW len, in bytes              */
 
/*------------------------------------------------------------------------*/
/*  Jiritsugo Table header information                                    */
/*------------------------------------------------------------------------*/
union  {
 struct JTE *chepool;                   /* addr of JTE pool               */
 struct CHH  chhchh;                    /* JT chain header                */
} jthchh;
 short       jthmxjte;                  /* max noof JTEs assoc w/JT       */
 
/*------------------------------------------------------------------------*/
/*  Jiritsugo Kanji hyoki table                                           */
/*------------------------------------------------------------------------*/
union  {
 struct JKJ *chepool;                   /* addr of JKJ pool               */
 struct CHH  chhchh;                    /* JK chain header                */
} jkhchh;
 short       jkhmxjke;                  /* max no of JKEs assoc w/JK      */
 
/*------------------------------------------------------------------------*/
/*  Jiritsugo Long-word Table header information                          */
/*------------------------------------------------------------------------*/
union  {
 struct JLE *chepool;                   /* addr of JLE pool               */
 struct CHH  chhchh;                    /* JL chain header                */
} jlhchh;
 short       jlhmxjle;                  /* max no of JLEs assoc w/JL      */
 
/*------------------------------------------------------------------------*/
/*  Fuzokugo Table header information                                     */
/*------------------------------------------------------------------------*/
union  {
 struct FTE *chepool;                   /* addr of FTE pool               */
 struct CHH  chhchh;                    /* FT chain header                */
} fthchh;
 short       fthmxfte;                  /* max no of FTEs                 */
 short       fthfnum;                   /* no of fuzokugo"s in this tbl   */
 short       fthhyklv;                  /* kj hyoki level ?               */
 
/*------------------------------------------------------------------------*/
/*  Fuzokugo Work table header information (Old TFT)                      */
/*------------------------------------------------------------------------*/
union  {
 struct FWE *chepool;                   /* addr of FWE pool               */
 struct CHH  chhchh;                    /* FW chain header                */
} fwhchh;
 short       fwhmxfwe;                  /* max no of FWEs in this FW      */
 
/*------------------------------------------------------------------------*/
/*  Bunsetsu Table header information                                     */
/*------------------------------------------------------------------------*/
union  {
 struct BTE *chepool;                   /* addr of BTE pool               */
 struct CHH  chhchh;                    /* common chain header            */
} bthchh;
 short       bthmxbte;                  /* max no of BTHs in this BT      */
 
/*------------------------------------------------------------------------*/
/*  Path Table header information                                         */
/*------------------------------------------------------------------------*/
union  {
 struct PTE *chepool;                   /* addr of PTE pool               */
 struct CHH  chhchh;                    /* common chain header            */
} pthchh;
 short       pthmxpte;                  /* max no of PTHs in this PT      */
 
/*------------------------------------------------------------------------*/
/*  Yomi Code table header information                                    */
/*------------------------------------------------------------------------*/
 struct YCE *ychyce;                    /* addr of YCE pool               */
 short       ychmxyce;                  /* max no of YCEs in this YC tbl  */
 short       ychacyce;                  /* no of currently active YCEs    */
 
/*------------------------------------------------------------------------*/
/*  Previous Yomi Code table header information                           */
/*------------------------------------------------------------------------*/
 struct YPE *yphype;                    /* addr of YPE pool               */
 short       yphmxype;                  /* max no of YPEs in this YP tbl  */
 short       yphacype;                  /* no of currently active YPEs    */
 
/*------------------------------------------------------------------------*/
/*  Mora Code table header information                                    */
/*------------------------------------------------------------------------*/
 struct MCE *mchmce;                    /* addr of MCE pool               */
 struct MCE *mchmdend;                  /* MCE addr, dict consult limit   */
 short       mchmxmce;                  /* max no of MCEs in this MC tbl  */
 short       mchacmce;                  /* no of currently active MCEs    */
 
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
 uschar      env;                       /* Active environment code        */
#define     ENVNONE  0                  /* no environment exists          */
#define     ENVNEXT  1                  /* Jikoho Environment             */
#define     ENVZEN   2                  /* Zenkoho Environment            */
#define     ENVTAN   3                  /* Tankan(zenkoho) Environemnt    */
#define     ENVALPH  4                  /* Alphanumeric Environemnt       */
#define     ENVALKN  5                  /* Alpha/Alphanum & Kansuji Env */
#define     ENVDICT  6                  /* Dictionary registrasion Env  */
#define     ENVKANSU 7                  /* Alpha/Alphanum & Kansuji Env */
 
 uschar      rsv02;                     /* reserved */
/*------------------------------------------------------------------------*/
 short       possta;                    /* Entry no 1-, the 1st entry no  */
                                        /* of the previous request output.*/
 short       posend;                    /* Entry no 1-, the last entry no */
                                        /* of the previous request output.*/
};
