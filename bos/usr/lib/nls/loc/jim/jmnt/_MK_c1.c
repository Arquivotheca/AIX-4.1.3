static char sccsid[] = "@(#)64	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/_MK_c1.c, libKJI, bos411, 9428A410j 7/23/92 03:19:24";
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         _MK_c1
 *
 * DESCRIPTIVE NAME:    Input DBCS String Primary Conversion to DBCS.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential - Restricted when aggregated)
 *
 * FUNCTION:            DBCS String Convet to Primary Candidates.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        6712 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         Module Entry Point Name
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            _MK_c1( pt )
 *
 *  INPUT:              pt      :Pointer to Kanji Monitor Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      IMSUCC  :Successful of Execution.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      Other   :Return Code by _Kcconv(Phigical Error Code).
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              _MK_c11 :Kana Kanji Conversion.
 *                              _MK_c12 :Alpha-Numeric Conversion.
 *                              _MK_c13 :DBCS Numeric Code Conversion.
 *                              _MK_c14 :Conversion Backend Processing.
 *                              _MK_c15 :Cannot Conversion Status Proc.
 *                              _MK_c16 :Kana Kanji Conversion Result
 *                                       Reflect.
 *                      Kanji Project Subroutines.
 *                              _Kcconv :Yomi Convert DBCS String.
 *                              WatchUdict:Watch/Load User dictionary
 *                              _MM_rtn :Conversion Mode Set.
 *                              _Mckbk  :Phrase Area Get.
 *                              _Mgetchm:DBCS Character Type Get.
 *                              _Mindset:Indicator Set.
 *                              _Mlfrtc :DBCS Character Class Get.
 *                              _Msetch :Redraw Area Set.
 *                              _Myomic :DBCS Yomi to 7bit yomi.
 *                      Standard Library.
 *                              memcpy  :Specified # Char Copy.
 *                              memset  :Specified Char Padding.
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      See Below.
 *
 *   INPUT:             Kanji Monitor Control Block(KCB).
 *                              curcol   kjsvpt   repins   string
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              alphanum cconvlen cconvpos conversn
 *                              convlen  convnum  convpos
 *                              curleft  gramap1s grammap  kanadata
 *                              kanalen  kanamap  kanjinum katakana
 *                              kjcvmap  kjdata1s kjmap1s  kkcbsvpt
 *                              lastch
 *                      Trace Brock(TRB).
 *                              NA.
 *                      Kana Kanji Contror Brock(KKCB).
 *                              grammap  gramaprn kanalen1 kanamap
 *                              kanamax  kjdata   kjlen    kjmap
 *                              kjmapln
 *
 *   OUTPUT:            Kanji Monitor Control Brock(KCB).
 *                              hlatst  curcol
 *                      Kanji Monitor Internal Save Area(KMISA,FSB).
 *                              actc3   cconvlen cconvpos convimp
 *                              convlen convnum  convpos  gramap
 *                              gramp1s iws1     iws2     kanamap
 *                              kjcvmap kjdata1s kjmap1s  kkcflag
 *                              kkcrc   leftchar nextact
 *                      Trace Brock(TRB).
 *                              NA.
 *                      Kana Kanji Control Brock(KKCB).
 *                              convmode extfunc  grammap   intconv
 *                              kanadata kanalen1 kjdata    kjmap
 *                              kjlen    leftchar rightchar
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              ALIGN   :Adjust Alignment Number.
 *                              CHPTTOSH:Char Pointer Data convert to short.
 *                              IDENTIFY:Module Identify Create.
 *                              KKCPHYER:KKC Phigical Error Check.
 *                              MAX     :Each of Bigger Number Get.
 *                              MIN     :Each of Lower Number Get.
 *                              SHTOCHPT:short Data Set Char Pointer Area.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     Bug Fix.
 *                      1988,01/27 Invalid Shift Number.
 *                                 (Kanji Conversion Map)
 *
 *                      Sept. 21 1988 Added by Satoshi Higuchi
 *                      Overflow of the kjdataf etc. support.
 *                      Added a few lines logic.
 *
 *                      06/09/92 call WatchUdict() for loading user dict.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standar I/O Header.                          */
#include <memory.h>     /* Memory Operation.                            */
/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Define File.                           */
#include "kcb.h"        /* Kanji Monitor Control Block.                 */
#include "kjmacros.h"   /* Kanji Macros.                                */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

static int	_MK_c11();
static int	_MK_c12();
static int	_MK_c13();
static int	_MK_c14();
static int	_MK_c15();
static int	_MK_c16();

int     _MK_c1( pt )

register KCB *pt;       /* Pointer to Kanji Control Block(KCB).         */

{
        uchar   _Mgetchm();     /* DBCS Character Type.                 */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        KKCB    *kkcbsvpt;      /* Pointer to KKCB.                     */

        register int loop;      /* Loop Counter.                        */
                                /* or Temporaty Counter.                */
        int     ret_code;       /* Return Code.                         */

        uchar   mode;           /* KKC Conversion Mode.                 */
        uchar   chmd;           /* DBCS Character type.                 */

        uchar   kanaflag;       /* Kana Conversion Flag.                */
        uchar   alphflag;       /* Alphabetic or Numeric Conversion Flag*/
        uchar   impoflag;       /* Cannot Conversion Flag.              */

        short   pos;            /* Offset to KCB String.                */
        uchar   *yomimap;       /* Pointer to DBCS yomi code.           */
        short   mappos;         /* Length of yomimap.                   */
        uchar   *ostr;          /* Pointer to Converted String.         */
        short   ostrlen;        /* Length of ostr.                      */

        /*
         *      Debugging Snap.
         */
        snap3(SNAP_KMISA | SNAP_KKCB,SNAP_MK_c1,"Start");

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer & Variable.
         ****************************************************************
         */
        /*
         *      Work Pointer & Variable Initialize.
         */
        kjsvpt  = pt->kjsvpt;           /* Get Pointer to Kanji Monitor */
                                        /* Internal Save Area.          */
        kkcbsvpt= kjsvpt->kkcbsvpt;     /* Get Pointer to Kana/Kanji    */
                                        /* Control Block.               */
        ret_code= IMSUCC;               /* Set Initial Return Code.     */

        /*
         ****************************************************************
         *      1. Check Conversion Limit.
         ****************************************************************
         */
        /*
         *      Conversion Length Check.
         */
        if(    ((kjsvpt->cconvlen/C_DBCS)>kkcbsvpt->kanamax )
            || (kjsvpt->cconvlen == 0 )                         ) {
                /*
                 *      Cannot Convrsion.
                 */
                ret_code = _MK_c15( pt );
                return( ret_code );
        };

        /*
         ****************************************************************
         *      2. Internal Work Araa Initialize.
         ****************************************************************
         */
        /*
         *      Internal Work Interface Parameter Initialize.
         */
        /*
         *      DBCS Character Class Save Area & Counter Initialize.
         */
        mappos  = 0;                    /* Number of DBCS Character     */
                                        /* Class.                       */
        yomimap = (uchar *)kjsvpt->iws1;/* Pointer to DBCS Character    */
                                        /* Class Save Area.             */
        /*
         *      Yomi Code Converted DBCS String Save Area & Usage
         *      Buffer Length Counter.
         */
        ostrlen = 0;                    /* Total Length of Converted    */
                                        /* DBCS Kanji String.           */
        ostr    = (uchar *)kjsvpt->iws2;/* Get Pointer to DBCS Kanji    */
                                        /* String Save Area.            */
        /*
         *      Initialize Counter of Successful 'Yomi to DBCS String'.
         */
        kjsvpt->convnum = 0;            /* Count of Successful Kana Kan-*/
                                        /* ji Convertion.               */
        /*
         ****************************************************************
         *      3. Get Conversional Target DBCS String Information
         *      with analize Data Type.
         ****************************************************************
         */
        /*
         ****************************************************************
         *      3. 1. Intitialize Conversion Mode Switch.
         ****************************************************************
         */
        /*
         *      Initialize Internal Work Variable.
         */
        kanaflag = C_SWOFF;     /* DBCS String Not Contains Kana.       */
        alphflag = C_SWOFF;     /* DBCS String Not Contains Alphnumeric.*/
        impoflag = C_SWOFF;     /* DBCS String Not Contains Symbol.     */

        /*
         ****************************************************************
         *      3. 2. Analize DBCS Charcter Type.
         ****************************************************************
         */
        /*
         *      Generate DBCS String to 7bit yomi ccode.
         */
        for( loop = 0; loop < kjsvpt->cconvlen ; loop += C_DBCS ) {

                /*
                 *      KCB String Position Get.
                 */
                pos     = kjsvpt->cconvpos + loop;

                /*
                 *      DBCS Character Type Get.
                 */
                chmd = _Mgetchm( pt->string, pos , kjsvpt );

                /*
                 *      Analize Character Type.
                 */
                switch( chmd ) {
                /*
                 *      DBCS Hiragana.
                 */
                case K_CHHIRA:
                        yomimap[mappos] = M_YMKANA;
                        kanaflag = C_SWON;
                        break;
                /*
                 *      DBCS Katakana.
                 */
                case K_CHKATA:
                        if( kjsvpt->kmpf[0].katakana == K_KANAON ) {
                                yomimap[mappos] = M_YMKANA;
                                kanaflag = C_SWON;
                        } else {
                                yomimap[mappos] = M_YMIMPO;
                                impoflag = C_SWON;
                        };
                        break;
                /*
                 *      DBCS Alphabetic.
                 */
                case K_CHALPH:
                        if( kjsvpt->kmpf[0].alphanum == K_ALPON ) {
                                yomimap[mappos] = M_YMALNM;
                                alphflag = C_SWON;
                        } else {
                                yomimap[mappos] = M_YMIMPO;
                                impoflag = C_SWON;
                        };
                        break;
                /*
                 *      DBCS Numeric.
                 */
                case K_CHNUM:
                        if(   (kjsvpt->kmpf[0].alphanum == K_ALPOFF)
                           && (kjsvpt->kmpf[0].kanjinum == K_KJNMOF) ) {
                                yomimap[mappos] = M_YMIMPO;
                                impoflag = C_SWON;
                        } else
                                yomimap[mappos] = M_YMALNM;
                        break;

                /*
                 *      DBCS Other Type.
                 */
                default:
                        yomimap[mappos] = M_YMIMPO;
                        impoflag = C_SWON;
                        break;
                };

                /*
                 *      Increment Counter.
                 */
                mappos++;
        };

        /*
         ****************************************************************
         *      4. Kana/Kanji Conversion Mode Analize.
         ****************************************************************
         */
             if( mappos == 0 )
                mode = K_CHKIGO;
        else if( kanaflag == C_SWON )
                mode = K_CHHIRA;
        else if( impoflag == C_SWON )
                mode = K_CHKIGO;
        else if( alphflag == C_SWON )
                mode = K_CHALPH;
        else if(kjsvpt->kmpf[0].kanjinum == K_KJNMON )
                mode = K_CHNUM;
        else
                mode = K_CHALPH;

        /*
         ****************************************************************
         *      6. Select Kana/Kanji Conversion Mode.
         ****************************************************************
         */
        switch( mode ) {
        /*
         *      Hiragana/Katakana Conversion.
         */
        case K_CHHIRA:
                ret_code = _MK_c11( pt,yomimap,mappos,ostr,&ostrlen);
                break;
        /*
         *      Alphabetic or Numeric Conversion.
         */
        case K_CHALPH:
                ret_code = _MK_c12( pt,yomimap,mappos,ostr,&ostrlen);
                break;
        /*
         *      DBCS Numeric Conversion.
         */
        case K_CHNUM:
                ret_code = _MK_c13( pt,yomimap,mappos,ostr,&ostrlen);
                break;
        /*
         *      No Conversion.
         */
        case K_CHKIGO:
        default      :
                ret_code = IMSUCC;
                break;
        };

        /*
         ****************************************************************
         *      6. Kana/Kanji Conversion Phgical Error Occure then
         *      Force Return to Caller,Otherwiser Normal Conversion
         *      Backend Processing.
         ****************************************************************
         */
        /*
         *      Conversion Backend Process if Conversion Succes.
         */
        if( !KKCPHYER(ret_code) ) {
                if( kjsvpt->convnum != 0 )
                        ret_code = _MK_c14( pt,ostr,ostrlen);
                else
                        ret_code = _MK_c15( pt );
        };

        /*
         *      Debugging Snap.
         */
        snap3(SNAP_KMISA | SNAP_KKCB,SNAP_MK_c1,"End");

        /*
         ****************************************************************
         *      7. Return to Caller.
         ****************************************************************
         */
        return( ret_code );
}

/*
 *      Kana yomi Data Convert to DBCS Kanji Code.
 */

static  int     _MK_c11( pt,yomimap,ymaplen,ostr,ostrlen)

register KCB *pt;       /* Pointer to Kanji Control Block.              */
uchar   *yomimap;       /* DBCS yomi type.                              */
short   ymaplen;        /* Length of yomimap.                           */
uchar   *ostr;          /* Pointer to Converted String Save Area.       */
short   *ostrlen;       /* Length of ostr.                              */

{
        int     _Myomic();      /* DBCS String Convert to 7bit code.    */
        uchar   _Mlfrtc();      /* DBCS Character Class Code.           */
        int     _Kcconv();      /* Conversion Word.                     */
	short 	WatchUdict();	/* Watch/Load User dictionary		*/
        char    *memcpy();      /* Copy # of Character.                 */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        register KKCB  *kkcbsvpt;/* Pointer to KKCB.                    */

        short   cvmappos;       /* kjcvmap access offset.               */
        short   mappos;         /* yomimap index.                       */
        short   strpos;         /* KCB string index.                    */
        short   startpos;       /* Previous Conversion Word Position.   */
        uchar   *dbcstr;        /* Pointer to DBCS String(yomi data).   */
        short   dbcslen;        /* DBCS word length.                    */
        short   cdbcslen;       /* Conveted word Length.                */
        short   convlen;        /* kjcvmap convlen Real Value.          */
        short   cconvlen;       /* kjcvmap cconvlen Real Value.         */

        /*
         ****************************************************************
         *      1-1. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Working Pointer & Varaible Initialize.
         */
        kjsvpt  = pt->kjsvpt;           /* Get Pointer to Kanji Monitor */
                                        /* Internal Save Block.         */
        kkcbsvpt= kjsvpt->kkcbsvpt;     /* Get Pointer to Kana Kanji    */
                                        /* Conversion Control Block.    */
        /*
         *      Save Current Conversion Position & Length.
         */
        convlen = kjsvpt->convlen;      /* Get Length of Conversion     */
                                        /* String.                      */
        cconvlen= kjsvpt->cconvlen;     /* Get Length of Current Conv-  */
                                        /* ersion String.               */
        /*
         ****************************************************************
         *      1-1. 1. Set Kana/Kanji Conversion Routine Conversion Mode
         ****************************************************************
         */
        /*
         *      KKC Conversion Mode Set.
         */
        kkcbsvpt->convmode = kjsvpt->kmpf[0].conversn;
                                        /* Set Kana/Kanji Convertion    */
                                        /* Mode.                        */
        kkcbsvpt->extconv  = K_EXTALL;  /* All Kana Data Have to be     */
                                        /* Conveted.                    */
        kkcbsvpt->intconv  = K_INTALL;  /* All Conveted Data Should Be  */
                                        /* Return.                      */
        /*
         *      Kanji Yomi String Read Point Initial Position.
         */
        strpos  = kjsvpt->cconvpos;     /* Get First Position of Currnet*/
                                        /* Conversion Data.             */
        /*
         *      Kanji Yomi String Start Position(Phrase first Position).
         *      Related 'dbcsstr,dbcslen'.
         */
        startpos= kjsvpt->cconvpos;     /* Get First Phrase Position of */
                                        /* Current Conversion Data.     */
        /*
         *      KKC Yomi Data Area & Yomi Length Initialize.
         */
        dbcstr  = NULL;                 /* Pointer to Single Phrase.    */
        dbcslen = 0;                    /* Length of Single Phrase.     */

        /*
         *      KKC Converted DBCS String Length Initialize.
         */
        cdbcslen= 0;                    /* Length of Conveted DBCS      */
                                        /* String.                      */
        /*
         *      Kanji Conversion Map Point Get.
         */
        cvmappos= kjsvpt->cconvpos - kjsvpt->convpos;

        /*
         ****************************************************************
         *      1-1. 2. Kana/Kanji Convert By Phrase.
         ****************************************************************
         */
        /*
         *      7bit yomi map conversion.
         */
        for( mappos = 0; mappos < ymaplen ; mappos++ ) {

                /*
                 *      7bit yomi map Analize.
                 */
                switch( yomimap[mappos] ) {
                /*
                 ********************************************************
                 *      1-1. 2.1 Get Kana One Phrase.
                 ********************************************************
                 */
                /*
                 *      Hiragana/Katakana.
                 */
                case M_YMKANA:
                        /*
                         *      Hiragana/Katakana yomi first position
                         *      save.
                         */
                        if( dbcstr==NULL )
                                dbcstr = &pt->string[strpos];

                        /*
                         *      Length of Hiragana/Katakana Calculate.
                         */
                        dbcslen += C_DBCS;

                        /*
                         *      Next Character is Hiragana or Kantana?
                         */
                        if( (mappos+1) <ymaplen ) {
                                /*
                                 *      Continious Word?
                                 */
                                if( yomimap[mappos+1]==M_YMKANA )
                                        break;
                        };

                        /*
                         *      KKC Interface Parameter Intialize.
                         */
                        kkcbsvpt->kanalen1 =
                        kkcbsvpt->kanalen2 =
                                _Myomic( dbcstr,dbcslen,kkcbsvpt->kanadata,
                                         K_CHHIRA );

                        /*
                         *      Left Character Class Code Sets.
                         */
                        if( startpos < (C_DBCS+kjsvpt->curleft) )
                                kkcbsvpt->leftchar = M_HIRA;
                        else
                                kkcbsvpt->leftchar =
                                    _Mlfrtc(pt->string+startpos-C_DBCS);

                        /*
                         *      Right Character Class Code Sets.
                         */
                        if(    (     (   (strpos+C_DBCS)
                                       ==(kjsvpt->convpos+kjsvpt->convlen)
                                     )
                                  && (pt->repins == K_REP)
                               )
                            || ((strpos+C_DBCS)>=pt->lastch)  )
                                kkcbsvpt->rightchr = M_HIRA;
                        else
                                kkcbsvpt->rightchr =
                                    _Mlfrtc(pt->string+strpos+C_DBCS);

                        /*
                         *      KKC Call.
                         */
        		if ( kjsvpt->kmpf[0].udload )
                		(void)WatchUdict( kkcbsvpt );
                        kjsvpt->kkcflag = M_KKNOP;      /* KKC Call.    */

                        snap3(SNAP_KKCB,SNAP_USER(0),"KCB CALL");
                        kjsvpt->kkcrc   = _Kcconv( kkcbsvpt );
                        snap3(SNAP_KKCB,SNAP_USER(0),"KCB RETURN");

                        /*
                         *      KKC Return Code Analize.
                         *      If PhigicalError On KKC Return to Caller.
                         */
                        if( KKCPHYER(kjsvpt->kkcrc) )
                                return( kjsvpt->kkcrc );

                        /*
                         *      KKC Call Backend Process.
                         */
                        cdbcslen = dbcslen;
                        if( kjsvpt->kkcrc == K_KCSUCC ) {
                                /*
                                 *      Conversion Successful.
                                 */
                                if(_MK_c16( pt,*ostrlen,
                                        cvmappos,&convlen,&cconvlen,
                                        &cdbcslen,&dbcstr)==IMKJCVOE) {
                                        /*
                                         *      **** OVERFLOW ****
                                         *      Kanji Convertion Map.
                                         *      EXIT TO MAIN.
                                         */
                                        if( cdbcslen==0 ) {
                                                mappos = ymaplen;
                                                continue;
                                        };
                                };
                        };
                        break;
                /*
                 ********************************************************
                 *      1-1. 2.2 Get Alphanumeric Phrase.
                 ********************************************************
                 */
                /*
                 *      Alphaberic & Numeric Conversion.
                 */
                case M_YMALNM:
                        dbcstr  = &pt->string[strpos];
                        dbcslen = C_DBCS;
                        cdbcslen= C_DBCS;
                        break;
                /*
                 ********************************************************
                 *      1-1. 2.2 Get Unconverional Phrase.
                 ********************************************************
                 */
                /*
                 *      Cannot Convesion Character.
                 */
                case M_YMIMPO:
                default:
                        dbcstr  = &pt->string[strpos];
                        dbcslen = C_DBCS;
                        cdbcslen= C_DBCS;
                        kjsvpt->kjcvmap[cvmappos+1] = M_KSCNUM;
                        break;
                };

                /*
                 ********************************************************
                 *      1-1. 2.4 Converted or Nont Converted String
                 *      Add Converted String Save Area.
                 ********************************************************
                 */
                /*
                 *      Save Conveted String in Converted String
                 *      Save Area.
                 */
                if( cdbcslen > 0 ) {
                        /*
                         *      Next Phrase Start Position Set.
                         */
                        startpos += dbcslen;

                        /*
                         *      Check Buffer Overflow.
                         */
                        if( cvmappos >=kjsvpt->kjcvmax )
                                break;
                        /*
                         *      Next Kanji Map Access Position Set.
                         */
                        cvmappos += cdbcslen;
                        if( cvmappos > kjsvpt->kjcvmax )
                                cdbcslen -= cvmappos - kjsvpt->kjcvmax;

                        /*
                         *      DBCS String Save DBCS String Buffer.
                         */
                        (void)memcpy( (char *)&ostr[*ostrlen],
                                      (char *)dbcstr,cdbcslen);
                        *ostrlen += cdbcslen;

                        /*
                         *      Reset DBCS Buffer Convrole Variable.
                         */
                        dbcslen = 0;
                        cdbcslen= 0;
                        dbcstr  = NULL;
                };

                /*
                 ********************************************************
                 *      1-1. 2.5 Get Next Converting String Positing
                 ********************************************************
                 */
                /*
                 *      Get Next DBCS String Position.
                 */
                strpos   += C_DBCS;
        };

        /*
         ****************************************************************
         *      1-1. 3. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}


/*
 *      Alphabetic or Numeric Katakana/Hiragana Conversion.
 */
static  int     _MK_c12( pt,yomidata,ymaplen,ostr,ostrlen)

register KCB *pt;       /* Pointer to KCB.                              */
uchar   *yomidata;      /* DBCS yomi type.                              */
short   ymaplen;        /* Length of yomidata.                          */
uchar   *ostr;          /* Pointer to Converted String Save Area.       */
short   *ostrlen;       /* Length of ostr.                              */

{
        int      _Myomic();     /* DBCS String Convert to 7bit code.    */
        char    *memcpy();      /* Copy # of Character.                 */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        register KKCB  *kkcbsvpt;/* Pointer to KKCB.                    */

        short   cvmappos;       /* kjcvmap access offset.               */
        short   cdbcslen;       /* Conveted word Length.                */

        short   convlen;        /* kjcvmap convlen Real Value.          */
        short   cconvlen;       /* kjcvmap cconvlen Real Value.         */

        /*
         ****************************************************************
         *      1-2. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Working Pointer & Varaible Initialize.
         */
        kjsvpt  = pt->kjsvpt;
        kkcbsvpt= kjsvpt->kkcbsvpt;
        convlen = kjsvpt->convlen;
        cconvlen= kjsvpt->cconvlen;

        /*
         ****************************************************************
         *      1-2. 1. Set Kana/Kanji Conversion Routine Conversion Mode
         ****************************************************************
         */
        /*
         *      KMISA kjcvmap Real Position Get.
         */
        cvmappos = kjsvpt->cconvpos - kjsvpt->convpos;

        /*
         *      Set Alphabet Conversion Mode.
         */
        kkcbsvpt->convmode = K_ALPCON;

        /*
         *      DBCS Code convert to 7bit code.
         */
        yomidata = pt->string + kjsvpt->cconvpos;
        kkcbsvpt->kanalen1 =
        kkcbsvpt->kanalen2 = _Myomic( yomidata,cconvlen,
                                      kkcbsvpt->kanadata,K_CHALPH);

        if( (kkcbsvpt->kanalen1*C_DBCS) != cconvlen )
                return( IMSUCC );

        /*
         ****************************************************************
         *      1-2. 3. Call Kana/Kanji Conversion Routine.
         ****************************************************************
         */
        if ( kjsvpt->kmpf[0].udload )
                (void)WatchUdict( kkcbsvpt );
        kjsvpt->kkcflag = M_KKNOP;
        kjsvpt->kkcrc   = _Kcconv(kkcbsvpt);

        /*
         ****************************************************************
         *      1-2. 4. Kana/Kanji Conversion Result Backend Processing.
         ****************************************************************
         */
        /*
         *      KKC Phigical Error Check.
         */
        if( KKCPHYER(kjsvpt->kkcrc) )
                return( kjsvpt->kkcrc );

        /*
         *      KKC Return Code Check.
         */
        cdbcslen = cconvlen;
        if( kjsvpt->kkcrc == K_KCSUCC ) {
                (void)_MK_c16( pt,*ostrlen,cvmappos,&convlen,&cconvlen,
                               &cdbcslen,&yomidata);
        };

        /*
         *      Conversion Result Sets.
         */
        (void)memcpy( (char *)&ostr[*ostrlen],(char *)yomidata,cdbcslen);
        *ostrlen += cdbcslen;

        /*
         ****************************************************************
         *      1-2. 5. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}

/*
 *      DBCS Numeric Conversion.
 */
static  int     _MK_c13( pt,yomidata,ymaplen,ostr,ostrlen)

register KCB *pt;       /* Pointer to KCB.                              */
uchar   *yomidata;      /* DBCS yomi type.                              */
short   ymaplen;        /* Length of yomidata.                          */
uchar   *ostr;          /* Pointer to Converted String Save Area.       */
short   *ostrlen;       /* Length of ostr.                              */

{
        int      _Myomic();     /* DBCS String Convert to 7bit code.    */
        char    *memcpy();      /* Copy # of Character.                 */
        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        register KKCB  *kkcbsvpt;/* Pointer to KKCB.                    */

        short   cvmappos;       /* kjcvmap access offset.               */
        short   cdbcslen;       /* Conveted word Length.                */

        short   convlen;        /* kjcvmap convlen Real Value.          */
        short   cconvlen;       /* kjcvmap cconvlen Real Value.         */

        /*
         ****************************************************************
         *      1-3. 0. Set Up General Work Pointer & Variable.
         ****************************************************************
         */
        /*
         *      Working Pointer & Varaible Initialize.
         */
        kjsvpt  = pt->kjsvpt;
        kkcbsvpt= kjsvpt->kkcbsvpt;
        convlen = kjsvpt->convlen;
        cconvlen= kjsvpt->cconvlen;

        /*
         ****************************************************************
         *      1-3. 1. Set Kana/Kanji Conversion Routine Conversion Mode
         ****************************************************************
         */
        /*
         *      KMISA kjcvmap Real Position Get.
         */
        cvmappos = kjsvpt->cconvpos - kjsvpt->convpos;

        /*
         *      KKC Conversion Mode Set.
         */
        kkcbsvpt->convmode = K_KANCON;

        /*
         ****************************************************************
         *      1-3. 2. Generate Kana/Kanji Conversion Routine Interface
         *      Parameter.
         ****************************************************************
         */
        /*
         *      DBCS Code convert to 7bit code.
         */
        yomidata = pt->string + kjsvpt->cconvpos;

        kkcbsvpt->kanalen1 =
        kkcbsvpt->kanalen2 = _Myomic( yomidata,cconvlen,
                                      kkcbsvpt->kanadata,K_CHHIRA);

        /*
         ****************************************************************
         *      1-3. 3. Call Kana/Kanji Conversion Routine.
         ****************************************************************
         */
        if ( kjsvpt->kmpf[0].udload )
                (void)WatchUdict( kkcbsvpt );
        kjsvpt->kkcflag = M_KKNOP;
        kjsvpt->kkcrc   = _Kcconv(kkcbsvpt);

        /*
         ****************************************************************
         *      1-3. 4. Kana/Kanji Conversion Result Backend Processing.
         ****************************************************************
         */
        /*
         *      KKC Phigical Error Check.
         */
        if( KKCPHYER(kjsvpt->kkcrc) )
                return( kjsvpt->kkcrc );

        /*
         *      Check KKC Return Code.
         */
        cdbcslen = cconvlen;
        if( kjsvpt->kkcrc == K_KCSUCC ) {
                (void)_MK_c16( pt,*ostrlen,cvmappos,&convlen,&cconvlen,
                               &cdbcslen,&yomidata);
        };

        /*
         *      Sets Conversion Result.
         */
        (void)memcpy( (char *)&ostr[*ostrlen],(char *)yomidata,cdbcslen);
        *ostrlen += cdbcslen;

        /*
         ****************************************************************
         *      1-3. 5. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}

/*
 *      Conversion Backend Process.
 */
static  int     _MK_c14( pt,ostr,ostrlen)

register KCB *pt;       /* Pointer to KCB.                              */
uchar   *ostr;          /* Pointer to Converted String Save Area.       */
short   ostrlen;        /* Length of ostr.                              */

{
        int     _Mckbk();       /* Get Phrase Area Range.               */
        int     _MM_rtn();      /* Mode Change.                         */
        char    *memset();      /* Set Memory.                          */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */

        short   convmax;        /* KCB String convmax Position.         */
        short   atpos;          /* Highlighting Attribute Position.     */
        short   ist;            /* Next Phrase Start Offset.            */
        short   ied;            /* Next Phrase End   Offset.            */
        short   flag;           /* Phrase Attribute Set Flags.          */
        short   mlastch;        /* Last Character Position.             */
        /*
         ****************************************************************
         *      1-4. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        kjsvpt  = pt->kjsvpt;   /* Get Pointer to Kanji Monitor Internal*/
                                /* Save Area.                           */
        /*
         ****************************************************************
         *      1-4. 1. Yomi DBCS String Area Replace DBCS Converted
         *      'Kanji' String.
         ****************************************************************
         */
        mlastch = pt->lastch;   /* Save Current Last Character Position */
        /*
         *      Display String Update.
         */
        _Mexchng(pt,ostr,0,ostrlen,kjsvpt->cconvpos,kjsvpt->cconvlen);
        mlastch = MAX(mlastch,pt->lastch);
                                /* Get Maximim Last Character Position  */
                                /* ,which is Previous or Current?       */
        /*
         ****************************************************************
         *      1-4. 2. Set Hilighting Attribute.
         *              Set Cursor Move Region.
         *              Set Convertion Mode.
         ****************************************************************
         */
        /*
         *      HIghlighting Attribute Sets.
         */
        if( kjsvpt->convlen == 0 ) {
                /*
                 ************************************************
                 *      1-4. 2-1. Overflow Conversion Data
                 *      Not Available Conversion Data.
                 ************************************************
                 */
                /*
                 *      Cursor Move Current Converting Data First Column.
                 */
                pt->curcol  = kjsvpt->cconvpos;

                /*
                 *      Current Conversion Data Nothing Now.
                 */
                kjsvpt->cconvlen= 0;
                kjsvpt->cconvpos= kjsvpt->curleft;

                /*
                 *      Change Current Mode to Primary Input Mode.
                 */
                (void)_MM_rtn(pt,A_1STINP);
        } else {
                /*
                 ************************************************
                 *      1-4. 2-2. Conversion Data Available.
                 ************************************************
                 */
                /*
                 *      Conversion Result Reflect Display Area.
                 */
                /*
                 *      Cursor Posotion Move Converted String Last Position.
                 */
                pt->curcol += ostrlen - kjsvpt->cconvlen;

                /*
                 *      Get Phrase Start Position & End Position.
                 */
                atpos = kjsvpt->cconvpos + ostrlen - C_DBCS;
                (void)_Mckbk(pt,atpos,&ist,&ied,&flag);

                /*
                 *      Current Phrase Attribute Set.
                 */
                convmax = kjsvpt->convpos + kjsvpt->convlen;
                if( flag == C_SWON ) {
                        /*
                         *      Cursor Display Area is Not
                         *      Conversional DBCS String.
                         */
                        (void)memset(
                                (char *)&pt->hlatst[kjsvpt->cconvpos],
                                K_HLAT2,
                                convmax - kjsvpt->cconvpos);

                        (void)memset(
                                (char *)&pt->hlatst[convmax],
                                K_HLAT0,
                                mlastch - convmax);
                        /*
                         *      Converted Length Set Zero.
                         */
                        kjsvpt->cconvlen = 0;
                } else {
                        /*
                         *      ied offset convert to position.
                         */
                        ied += C_POS;
                        /*
                         *      Before Current Converted String Attribute
                         *      Set.
                         */
                        (void)memset(
                                (char *)&pt->hlatst[kjsvpt->cconvpos],
                                K_HLAT2,
                                ist - kjsvpt->cconvpos);

                        /*
                         *      Current Conveted Attribute Set.
                         */
                        (void)memset(
                                (char *)&pt->hlatst[ist],
                                K_HLAT3,
                                ied - ist);

                        /*
                         *      After Current Convertd Attribute Set.
                         */
                        (void)memset(
                                (char *)&pt->hlatst[ied],
                                K_HLAT2,
                                convmax - ied);

                        /*
                         *      If After DBCS Convertd String Length
                         *      less than Before it then Previous
                         *      Hilighting Area Need Reset
                         *      Non Attribute.
                         */
                        (void)memset(
                                (char *)&pt->hlatst[convmax],
                                K_HLAT0,
                                mlastch - convmax);

                        /*
                         *      Current Conversion Posotion Sets.
                         */
                        kjsvpt->cconvpos = ist;
                        kjsvpt->cconvlen = ied - ist;

                };

                /*
                 *      Backend Conversion Status Set.
                 */
                if( pt->curcol == (kjsvpt->convpos + kjsvpt->convlen) ) {
                        /*
                         *      Cursor Position Conversion Last Position
                         *      when Which String is Non Converted
                         *      String Set Editing Mode A
                         *      ,Otherwiase Cotinuous Conversion Mode.
                         */
                        if(
                            kjsvpt->kjcvmap[pt->curcol-kjsvpt->convpos
                                        - C_DBCS +1] == M_KSCNVK )
                                kjsvpt->actc3 = A_CNVMOD;
                        else
                                kjsvpt->actc3 = A_EDTMOA;
                } else
                        kjsvpt->actc3 = A_EDTMOA;
        };

        /*
         ****************************************************************
         *      1-4. 3. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}

/*
 *      Cannot Conversional Word Processing.
 */

static  int     _MK_c15( pt )

register KCB *pt;       /* Pointer to KCB.                              */

{
        int     _Msetch();      /* Display Refresh String Area Set.     */
        int     _Mindset();     /* Indicator Status Set.                */
        char    *memset();      /* Set Memory.                          */
        uchar   hlat;           /* HIlighting Attribute.                */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */

        /*
         ****************************************************************
         *      1-5. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        kjsvpt  = pt->kjsvpt;   /* Get Pointer to Kanji Monitor Internal*/
                                /* Save Block.                          */
        /*
         *      Collection Exit Point.
         */
        do {
                /*
                 *******************************************************
                 *      1-5  1. Caution Bell On.
                 *******************************************************
                 */
                /*
                 *      Editor Action Code Sets.
                 */
                kjsvpt->actc3 = A_BEEP | A_EDTMOA;

                /*
                 *******************************************************
                 *      1-5  2. Cannot Conversion Indicator On.
                 *******************************************************
                 */
                /*
                 *      Set Cannot Conversion Indicator.
                 */
                kjsvpt->convimp = C_SWON;
                (void)_Mindset(pt,M_INDL);
                /*
                 *      Set Next Action Code(Draw Cannnot Conversion
                 *      Indicator).
                 */
                kjsvpt->nextact |= M_CNRSON;

                /*
                 *******************************************************
                 *      1-5  3. Highlinghting Attribute Set.
                 *******************************************************
                 */
                /*
                 *      Attribute Data Not Active?
                 */
                if( kjsvpt->cconvlen < C_DBCS )
                        break;

                /*
                 *      Display Refresh Area Sets.
                 */
                (void)_Msetch(pt,kjsvpt->cconvpos,kjsvpt->cconvlen);

                /*
                 *      Cursor Positon Set.
                 */
                pt->curcol = kjsvpt->cconvpos;

                /*
                 *      Hilighting Attribute Sets.
                 */
                if( kjsvpt->cconvlen > C_DBCS ) {
                        (void)memset(
                                (char *)&pt->hlatst[kjsvpt->cconvpos+C_DBCS],
                                K_HLAT2,
                                kjsvpt->cconvlen - C_DBCS);
                };

                /*
                 *      Conversion Actve Length & Hilighting Set.
                 */
                switch(
                    kjsvpt->kjcvmap[kjsvpt->cconvpos-kjsvpt->convpos+1]) {
                case M_KSNCNV:  /* Conversion DBCS String.              */
                case M_KSCNVK:  /* Conversion DBCS(Single Word).        */
                case M_KSCNSK:  /* Converting Yomi DBCS.                */
                case M_KSCNVY:  /* Converting Yomi DBCS(Word).          */
                        hlat    = K_HLAT3;
                        kjsvpt->cconvlen = C_DBCS;
                        break;
                case M_KSCNUM:  /* Numberic or Symbol Character.        */
                default      :
                        hlat    = K_HLAT2;
                        kjsvpt->cconvlen = 0;
                        break;
                };

                /*
                 *      Cursor Highlighting Attribute Set.
                 */
                (void)memset((char *)&pt->hlatst[kjsvpt->cconvpos],
                              hlat,C_DBCS);

        } while( NILCOND );

        /*
         ****************************************************************
         *      1-5. 4. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}

/*
 *      KKC Call Backend Processing.
 */
static  int     _MK_c16(
                        pt,ostrlen,cvmappos,convlen,cconvlen,dbcslen,dbcstr)
register KCB *pt;       /* Pointer to KCB.                         (I/ )*/
short   cvmappos;       /* Kanji Convertion Map Index.(KMISA)      (I/ )*/
short   ostrlen;        /* Total Number of Convertd String         (I/ )*/
short   *convlen;       /* Current convlen.                        (I/O)*/
short   *cconvlen;      /* Current cconvlen.                       (I/O)*/
short   *dbcslen;       /* Length of DBCS Before Conversion.       (I/O)*/
uchar   **dbcstr;       /* Converted DBCS String.                  ( /O)*/

{
        char    *memcpy();      /* Copy # of Character.                 */

        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        register KKCB  *kkcbsvpt;/* Pointer to KKCB.                    */
        short   kjlendef;       /* Difference from Converted Length     */
                                /* to Previous Conveted Length.         */
        short   convmax;        /* Conversion Position Last Position.   */
        uchar   *kjcvmap;       /* kjcvmap Real Address.                */

        register int loop;      /* Loop Counter.                        */
        int     loopend;        /* Loop End Counter.                    */
        short   length;         /* Map Length.                          */
        int     phranum;        /* Phrase Number.                       */
        int     adjnum;         /* Adjunct Number.                      */
        int     movlen;         /* Character or Bits Move Length.       */
        int     rotbyt;         /* Rotete Bit Number Count.             */
        int     kanabyte;       /* Kanamap Byte Number.                 */
        int     kanabits;       /* Kanamap Byte BIt Position.           */
        int     totwrd;         /* Total Word Number.                   */
        uchar   *wrkpos;        /* Work Bit Pattern Byte Pointer.       */
        uchar   *srcpos;        /* Source Bit Pattern Byte Pointer.     */
        uchar   *dstpos;        /* Destination Bit Pattern Byte Pointer.*/
        uchar   wrkpat;         /* Bit Operation Work.                  */
        uchar   srcpat;         /* BIt Operation Source Byte Work.      */
        uchar   dstpat;         /* Bit Operation Destination Byte Work. */
        int     actbit;         /* Useful Bit Pattern Number.           */
        int     rembit;         /* I can Use Bit Number.                */
        int     srcbit;         /* Source Bit Pattern Byte Bit Position.*/
        int     dstbit;         /* Destination Bit Pattern Byte Bit Pos.*/
        short   grampos;        /* Grammer Map Index.                   */
        short   gramposs;       /* Saved Grammer Map Index.             */
        short   kjmappos;       /* kjmap1s pos.                         */
        short   kjdatpos;       /* kjdata1s pos.                        */
        uchar   candflag;       /* Last Candidate Found Flag.           */
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* Added source.                                                        */
/*     short length1;                                                   */
/*     short length2;                                                   */
/*======================================================================*/
	short   length1;        /* length of the work area              */
	short   length2;        /* length of the work area              */

        /*
         ****************************************************************
         *      1-6. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Working Pointer & Varaible Initialize.
         */
        kjsvpt  = pt->kjsvpt;
        kkcbsvpt= kjsvpt->kkcbsvpt;

        /*
         ****************************************************************
         *      1-6. 1. Total Number of Successful Conversion.
         ****************************************************************
         */
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/* Added source.                                                        */
/*      length1 = CHPTTOSH(kjsvpt->kjdata1s);                           */
/*      length2 = CHPTTOSH(kkcbsvpt->kjdata);                           */
/*      if(kjsvpt->dat1smax < length1+(length2-C_DBCS))                 */
/*          return(IMSUCC);                                             */
/*                                                                      */
/*      length1 = CHPTTOSH(kjsvpt->kjmap1s);                            */
/*      length2 = CHPTTOSH(kkcbsvpt->kjmap);                            */
/*      if(kjsvpt->map1smax < length1+(length2-C_DBCS))                 */
/*          return(IMSUCC);                                             */
/*                                                                      */
/*      length1 = kjsvpt->gramap1s[0];                                  */
/*      length2 = kkcbsvpt->grammap[0];                                 */
/*      if(kjsvpt->gra1smax < length1+(length2-C_ANK))                  */
/*          return(IMSUCC);                                             */
/*======================================================================*/
	length1 = CHPTTOSH(kjsvpt->kjdata1s);
	length2 = CHPTTOSH(kkcbsvpt->kjdata);
	if(kjsvpt->dat1smax < length1+(length2-C_DBCS))
	    return(IMSUCC);

	length1 = CHPTTOSH(kjsvpt->kjmap1s);
	length2 = CHPTTOSH(kkcbsvpt->kjmap);
	if(kjsvpt->map1smax < length1+(length2-C_DBCS))
	    return(IMSUCC);

	length1 = kjsvpt->gramap1s[0];
	length2 = kkcbsvpt->grammap[0];
	if(kjsvpt->gra1smax < length1+(length2-C_ANK))
	    return(IMSUCC);

        /*
         *      Conversion Counter Increment.
         */
        kjsvpt->convnum++;

        /*
         ****************************************************************
         *      1-6. 2. Kana Kanji Control
         *      for Kana/Kanji Controle Block Data Area,it's Data Reflect.
         ****************************************************************
         */
        /*
         *      Get Kanji Convertin Map Active Point.
         */
        kjcvmap = &kjsvpt->kjcvmap[cvmappos];
        convmax = kjsvpt->convpos  + *convlen - cvmappos;

        /*
         *      Count Word & Adujnct & Before Candidates Check.
         *      (Word Number equal to Phrase Number).
         */
        candflag = C_SWOFF;
        for( loop = 0; loop < convmax ; loop += C_DBCS ) {
                /*
                 *      Next Word Check.
                 */
                switch(kjcvmap[loop+1]) {
                case M_KSCNVK:  /* Conversion DBCS String.              */
                case M_KSCNSK:  /* Conversion DBCS(Single Word).        */
                case M_KSCNVY:  /* Converting Yomi DBCS.                */
                case M_KSCNSY:  /* Converting Yomi DBCS(Word).          */
                        candflag = C_SWON;
                        break;
                };
        };

        phranum = 0;    /* Count for Phrase.                            */
        adjnum  = 0;    /* Count for Adjunct.                           */

        for( loop = 0 ; loop < cvmappos ; loop += C_DBCS ) {

                /*
                 *  Continuous Adjunct Word Check.
                 */
                switch( kjsvpt->kjcvmap[loop] ) {
                case M_KJMNCV:  /* No Conversion Data.                  */
                case M_KJMCTN:  /* Continuous Data.                     */
                        continue;
                case M_KJMJAN:  /* Adjunct Data.                        */
                        adjnum++;
                        continue;
                };

                /*
                 *  Count Phrase Number.
                 */
                phranum++;
        };

        /*
         *      Kanji Conversion Map Update.
         */
        kjlendef = (kkcbsvpt->kjlen - 2) - *dbcslen;
        /*
         *      Get Pointer to DBCS String.
         */
        *dbcstr  = &kkcbsvpt->kjdata[2];

        /*
         *      In Case of Overflow Kanji Convertion Map.
         */
        if( (*convlen + kjlendef)>kjsvpt->kjcvmax ) {
                /*
                 *      Get Length of DBCS String.
                 */
                *dbcslen += kjlendef;

                /*
                 *      Check Maximum Length of Kanji Conversion Map.
                 */
                     if( ostrlen >= kjsvpt->kjcvmax ) {
                        /* **** NO MORE SPACE FOR SAVE AREA *****/
                        *dbcslen = 0;
                } else if( (ostrlen+*dbcslen)>=kjsvpt->kjcvmax ) {
                        /* **** TOO LONG STRING FOR SAVE AREA   */
                        /* .... TRUNCATE STRING ....            */
                        *dbcslen -= (ostrlen+*dbcslen) - kjsvpt->kjcvmax;
                } else {
                        /* **** A LOT OF SPACE REMAIN ****      */
                };

                /*
                 *      Set Conversion Map Overflow.
                 *      **** CAUTION ****.
                 *      Call Process must be call '_Mstlcvl()'
                 *      Overflow Processing Routine,Otherwise
                 *      unexpected Result Reflect in Control Block.
                 */
                *convlen  = kjsvpt->kjcvmax;    /* Set Maximum Convertio*/
                                                /* n Position.          */
                *cconvlen = kjsvpt->kjcvmax;    /* Set Maximum Current  */
                                                /* Conversion Position. */
                return( IMKJCVOE );
        };

        /*
         *      Copy Kana Kanji Control Block Data to Kanji Monitor
         *      Internal Save Area.
         */
        if( kjlendef>0 ) {
                /*
                 *      Previous Conversion Length less equal than
                 *      Current Conversion Length.
                 *      Insert Different Number of Character
                 *      & Replace Current Conversion Data.
                 */
                for( loop=convmax-1;loop>=0; loop-- ) {
                        kjcvmap[loop+kjlendef] = kjcvmap[loop];
                };

        } else if( kjlendef<0) {
                /*
                 *      Current Conversion Length less equual than
                 *      Previous Conversion Length.
                 *      Delete Different Number of Character
                 *      & Replace Current Conversion Data.
                 */
/************************************************************************/
/* #(B) 1988.01.27. Change.                                             */
/*                                                                      */
/*      Old:    (void)memcpy( (char *)&kjcvmap[*dbcslen+kjlendef],      */
/*                            (char *)&kjcvmap[*dbcslen],               */
/*                             convmax+kjlendef);                       */
/*                                                                      */
/*      New:    (void)memcpy( (char *)&kjcvmap[*dbcslen+kjlendef],      */
/*                            (char *)&kjcvmap[*dbcslen],               */
/*                             convmax-*dbcslen);                       */
/************************************************************************/

                (void)memcpy( (char *)&kjcvmap[*dbcslen+kjlendef],
                              (char *)&kjcvmap[*dbcslen],
                               convmax-*dbcslen);
                (void)memset((char *)&kjcvmap[convmax + kjlendef],
                                '\0',-kjlendef);
        };

        /*
         *      Kanji Conversion Length & Current Conversion Length
         *      Increment.
         */
        *convlen += kjlendef;
        *cconvlen+= kjlendef;

        /*
         *      Set Length of DBCS String.
         */
        *dbcslen += kjlendef;

        /*
         *      KMISA Kanji Conversion Map Conversion Status Sets.
         */
        for( loop = 0 ; loop < *dbcslen ; loop += C_DBCS ) {
                kjcvmap[loop  ] = kkcbsvpt->kjmap[2+(loop/C_DBCS)];
                kjcvmap[loop+1] = M_KSCNVK;
        };

        /*
         ****************************************************************
         *      1-6.3. Kanji Monitor Internal Save Area Interface Variable
         *      for Kana/Kanji Control Block Data Area,it's Data Reflect.
         ****************************************************************
         */
        /*
         *      KKCB kana map Reflect to KMISA kana map.
         */
        wrkpos= &kjsvpt->kanamap[1];
        rotbyt= C_BITBYT;
        wrkpat= *wrkpos++;
        totwrd= phranum + adjnum + 1;
        kanabits= 0;

        loop   = kjsvpt->kanalen;
        while( loop> 0 ) {
                /*
                 *      Check MSB Bit On.
                 */
                if( wrkpat & (1<<(C_BITBYT-1)) ) {
                        /*
                         *      Adjunct Word End?
                         */
                        if( --totwrd<= 0 )
                              break;
                };

                wrkpat <<= 1;
                /*
                 *      Kana data Length Make.
                 */
                kanabits++;
                loop--;

                /*
                 *      Next kana map Data Get.
                 */
                if( --rotbyt<=0 ) {
                        wrkpat = *wrkpos++;
                        rotbyt = C_BITBYT;
                };
        };

        kanabyte = (kanabits/C_BITBYT);

        /*
         *      Grammer Map Current Active Position Get.
         */
        grampos = 1;
        for( loop = 1; loop <=phranum ; loop++ ) {
                if( kjsvpt->grammap[grampos] & (1<<(C_BITBYT-1)) )
                        grampos += 2;
                else
                        grampos++;
        };

        /*
         *      Get Grammer Map Position.
         */
        gramposs= 1;
        for( loop = 1; loop <=phranum ; loop++ ) {
                if( kjsvpt->gramap1s[gramposs] & (1<<(C_BITBYT-1)) )
                        gramposs += 2;
                else
                        gramposs ++;
        };

        /*
         *      KMISA Previous Kanji Data & Kanji Conversion Map Update.
         *      Kanji Map Current Phrase Position.
         */
        kjmappos = 0;

        /*
         *      Get Previous Kanji Map Length.
         */
        length   = CHPTTOSH(kjsvpt->kjmap1s);

        /*
         *      Kanji Map Phrase Number Counter Init.
         *      if Previos Conversion Data is Availabele case
         *      Skip Previous Phrase Number & Position.
         */
        loopend = 0;

        /*
         *      KMISA Previous Kanji Map Search for Specified
         *      number of 'Current Kanji Converstion Map Phrase Number'.
         */
        for( loop = 2 ; loop < length ; loop++ ) {
                /*
                 *      One Phrase Last Position Get.
                 */
                switch( kjsvpt->kjmap1s[loop] ) {
                case M_KJMCTN:  /* Continuous Data.     */
                case M_KJMJAN:  /* Adjnuct Data.        */
                        /*
                         *      Last Phrase Position Set.
                         */
                        kjmappos++;
                        continue;
                };

                /*
                 *      All Phrase Reached.
                 */
                if( loopend >= phranum )
                        break;

                /*
                 *      Last Phrase Position Set.
                 */
                kjmappos++;

                /*
                 *      Single Phrase Counter Count Up.
                 */
                loopend++;
        };
        kjdatpos = kjmappos * C_DBCS;

        /*
         *      KMISA Data is Contains After Conversion Position.
         *      Insert Conversion Posion Each Data Area.
         */
        if( candflag ) {
                /*
                 *      Grammer Map Map Shift Right.
                 */
                movlen = kkcbsvpt->grmapln -1;
                for( loop=kjsvpt->grammap[0]-1;loop >=grampos;loop--) {
                        kjsvpt->grammap[loop + movlen ]
                                = kjsvpt->grammap[ loop ];
                };

                /*
                 *      Kana map Shift Right N bits.
                 */
                movlen   = kkcbsvpt->kanalen1;
                loop     = kjsvpt->kanalen - kanabits;
                srcpos   = &kjsvpt->kanamap[1+(kjsvpt->kanalen/C_BITBYT)];
                dstpos   = &kjsvpt->kanamap
                                [1+((kjsvpt->kanalen+movlen)/C_BITBYT)];
                srcbit   = kjsvpt->kanalen % C_BITBYT;
                dstbit   = (kjsvpt->kanalen+movlen) % C_BITBYT;

                /*
                 *
                 *                  srcbit
                 *                  v
                 *      +----------------------------+
                 *      |                            |
                 *      +----------------------------+
                 *
                 *                     dstbit
                 *                      v
                 *      +----------------------------+
                 *      |                            |
                 *      +----------------------------+
                 *
                 */
                while( loop > 0 ) {
                        /*
                         *      Decrement Source Bit Position.
                         */
                        if( srcbit <= 0 ) {
                                srcpos--;
                                srcbit += C_BITBYT;
                        };

                        /*
                         *      Increment Destination Bit Position.
                         */
                        if( dstbit <=0  ) {
                                dstpos--;
                                dstbit += C_BITBYT;
                        };

                        /*
                         *      Available Bit Number Get.
                         */
                        actbit = MIN( loop ,dstbit);
                        actbit = MIN(actbit,srcbit);
                        rembit = C_BITBYT - dstbit;

                        /*
                         *      Get Source Pattern.
                         */
                        srcpat = *srcpos;
                        srcpat >>= (C_BITBYT - srcbit);
                        srcpat <<= (C_BITBYT - actbit);
                        srcpat >>= (dstbit - actbit);

                        /*
                         *      Get Destination Pattern.
                         */
                        wrkpat = dstpat = *dstpos;
                        dstpat <<= dstbit;
                        dstpat >>= dstbit;
                        wrkpat >>= (rembit + actbit);
                        wrkpat <<= (rembit + actbit);
                        dstpat |= wrkpat;

                        /*
                         *      New Pattern Sets.
                         */
                        *dstpos = srcpat | dstpat;
                        srcbit -= actbit;
                        dstbit -= actbit;
                        loop   -= actbit;
                };

                /*
                 *      Kana data Shift Right.
                 */
                movlen = kkcbsvpt->kanalen1;
                for( loop=kjsvpt->kanalen-1
                    ;loop>=kanabits
                    ;loop--) {
                        kjsvpt->kanadata[loop + movlen ]
                                = kjsvpt->kanadata[ loop ];
                };

                /*
                 *      KKCB Kanji Map Length Data Length Get.
                 */
                movlen   = kkcbsvpt->kjmapln - 2;

                /*
                 *      KMISA kjdata1s kjmap1s Shift Right.
                 *      KMISA First Conversion Kanji Data,
                 *      First Conversion Kanji Map Shift Right.
                 */
                length = CHPTTOSH( kjsvpt->kjmap1s );
                for( loop=length - 1
                    ;loop>=kjmappos+2
                    ;loop--) {
                        kjsvpt->kjmap1s[loop + movlen ]
                                = kjsvpt->kjmap1s[ loop ];
                };

                length = CHPTTOSH(kjsvpt->kjdata1s);
                for( loop=length - 1
                    ;loop>=kjdatpos+2
                    ;loop--) {
                        kjsvpt->kjdata1s[loop + movlen*C_DBCS ]
                                = kjsvpt->kjdata1s[ loop ];
                };
                /*
                 *      gramap1s shift left.
                 */
                movlen = kkcbsvpt->grmapln-1;
                for( loop = kjsvpt->gramap1s[0]-1
                    ;loop>=gramposs
                    ;loop--) {
                        kjsvpt->gramap1s[loop + movlen] =
                                kjsvpt->gramap1s[loop];
                };
        };

        /*
         *--------------------------------------------------
         *      KMISA Interface Variable Copy from KKCB.
         *--------------------------------------------------
         */

        /*
         *      Copy Grammer Map.
         */
        /*
         *      KKCB Grammer Map Length Get.
         */
        movlen = kkcbsvpt->grmapln - 1;

        /*
         *      Copy KKCB Grammer Map Copy to KMISA Grammer Map.
         */
        (void)memcpy((char *)&kjsvpt->grammap[grampos],
                    (char *)&kkcbsvpt->grammap[1],
                    movlen);
        /*
         *      KMISA Grammer Map Length Adjust.
         */
        kjsvpt->grammap[0] += movlen;

        /*
         *      Copy KKCB Kana Map to KMISA Kana Map.
         */
        /*
         *      KKCB Kana Map Data Area Address & Bit Position Get.
         */
        srcpos   = &kkcbsvpt->kanamap[1];
        srcbit   = 0;

        /*
         *      KMISA Kana Map Data Area Address & Bit Position Get.
         */
        dstpos   = &kjsvpt->kanamap[kanabyte+1];
        dstbit   = kanabits % C_BITBYT;

        /*
         *      KKCB Kana Map Length Get.
         */
        loop     = kkcbsvpt->kanalen1;

        /*
         *      KKCB Kana Map Copy to KMISA Kana Map.
         */
        while( loop > 0 ) {
                /*
                 *      Available Bit Get.
                 */
                rembit = C_BITBYT - dstbit;
                actbit = MIN( loop ,rembit );
                actbit = MIN( actbit, C_BITBYT - srcbit );

                /*
                 *      KKCB Source Pattern Get.
                 *
                 *                      +----- actbit
                 *                    < |  >
                 *                   |    |
                 *      +------------+------+
                 *      |ZZZXXXXXXXXXYYYYYYA|
                 *      +---+--------+------+
                 *          ^        ^< |  >|
                 *          |        |  +--- rembit
                 *          |        +------ dstbit;
                 *          +--------------- srcbit;
                 *
                 *              |
                 *              V
                 *      +--------------------+
                 *      |XXXXXXXXXYYYYYYA0000|
                 *      +--------------------+
                 *              |
                 *              V
                 *      +--------------------+
                 *      |000000000000XXXXXXXX|
                 *      +--------------------+
                 *              |
                 *              V
                 *      +--------------------+
                 *      |000000000000XXXXXXXX|
                 *      +--------------------+
                 *              |
                 *              V
                 *      +--------------------+
                 *      |00000000000XXXXXXXX0|
                 *      +--------------------+
                 *              |
                 *              V
                 *      +--------------------+
                 *      |000000000000XXXXXXX0|
                 *      +--------------------+
                 */
                srcpat = *srcpos;
                srcpat <<= srcbit;
                srcpat >>= dstbit;
                srcpat >>= (rembit - actbit);
                srcpat <<= (rembit - actbit);

                /*
                 *      KMISA Target Pattern Get.
                 *
                 *                      +----- actbit
                 *                    < |  >
                 *                   |    |
                 *      +------------+------+
                 *      |ZZZXXXXXXXXXYYYYYYA|
                 *      +---+--------+------+
                 *          ^        ^< |  >|
                 *          |        |  +--- rembit
                 *          |        +------ dstbit
                 *          +--------------- srcbit
                 *
                 *              |       +----------------|
                 *              V                        V
                 *      +-------------------+   +-------------------+
                 *      |0000000ZZZXXXXXXXXX|   |A00000000000XXXXXXX|
                 *      +-------------------+   +-------------------+
                 *              |                        |
                 *              V                        V
                 *      +-------------------+   +-------------------+
                 *      |ZZZXXXXXXXXX0000000|   |000000000000000000A|
                 *      +-------------------+   +-------------------+
                 *              |                        |
                 *              V------------------------+
                 *      +-------------------+
                 *      |ZZZXXXXXXXXX000000A|
                 *      +-------------------+
                 */
                wrkpat = dstpat = *dstpos;
                dstpat >>= rembit;
                dstpat <<= rembit;
                wrkpat <<= dstbit + actbit;
                wrkpat >>= dstbit + actbit;
                dstpat |= wrkpat;

                /*
                 *      New Pattern Sets.
                 */
                *dstpos = srcpat | dstpat;
                srcbit += actbit;
                dstbit += actbit;
                loop   -= actbit;

                /*
                 *      Increment Source Bit Position.
                 */
                if( srcbit >= C_BITBYT ) {
                        srcpos++;
                        srcbit -= C_BITBYT;
                };

                /*
                 *      Increment Destination Bit Position.
                 */
                if( dstbit >= C_BITBYT ) {
                        dstpos++;
                        dstbit -= C_BITBYT;
                };
        };

        /*
         *      Kana map Length Update.
         */
        kjsvpt->kanamap[0] = 1 +
                  ALIGN( kjsvpt->kanalen + kkcbsvpt->kanalen1,C_BITBYT)
                / C_BITBYT;

        /*
         *      Copy New Kana data.
         */
        movlen = kkcbsvpt->kanalen1;
        (void)memcpy( (char *)&kjsvpt->kanadata[kanabits],
                      (char *)kkcbsvpt->kanadata,
                      movlen);

        /*
         *      Kana data Length Adjust.
         */
        kjsvpt->kanalen += movlen;

        /*
         *      KKCB Kanji Map Length Data Length Get.
         */
        movlen   = kkcbsvpt->kjmapln - 2;

        /*
         *      KKCB Data Copy To KMISA.
         */
        (void)memcpy( (char *)&kjsvpt->kjmap1s[kjmappos+2],
                      (char *)&kkcbsvpt->kjmap[2],
                      movlen);

        /*
         *      KMISA Saved Kanji Map Length Set.
         */
        length = CHPTTOSH( kjsvpt->kjmap1s ) + movlen;
        SHTOCHPT(kjsvpt->kjmap1s,length);

        /*
         *      KMISA Saved Kanji Data Set.
         */
        (void)memcpy( (char *)&kjsvpt->kjdata1s[kjdatpos+2],
                      (char *)&kkcbsvpt->kjdata[2],
                      movlen*C_DBCS);
        /*
         *      KMISA Saved Kanji Data Length Set.
         */
        length = CHPTTOSH( kjsvpt->kjdata1s) + movlen * C_DBCS;
        SHTOCHPT( kjsvpt->kjdata1s,length);


        /*
         *      Copy Saved Grammer Map.
         */
        movlen = kkcbsvpt->grmapln-1;
        (void)memcpy( (char *)&kjsvpt->gramap1s[gramposs],
                      (char *)&kkcbsvpt->grammap[1],
                      movlen);
        /*
         *      Adjust Saved Grammer Map Length.
         */
        kjsvpt->gramap1s[0] += movlen;

        /*
         *      Debugging Output.
         */
        snap3(SNAP_KMISA | SNAP_KKCB,SNAP_MK_c1,"KKCB->KMISA");

        /*
         ****************************************************************
         *      1-6. 4. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}

