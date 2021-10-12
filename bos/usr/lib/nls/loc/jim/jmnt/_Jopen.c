static char sccsid[] = "@(#)36	1.4.1.3  src/bos/usr/lib/nls/loc/jim/jmnt/_Jopen.c, libKJI, bos411, 9428A410j 6/10/94 14:51:59";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : _Jopen
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <ctype.h>      /* Character Type Class Check.                  */
#include <memory.h>     /* Memory Operation.                            */

/*
 *      include Kanji Project.
 */
#include "kj.h"         /* Kanji Project Define File.                   */
#include "ext.h"        /* Extended Information Control Block.          */
#include "kcb.h"        /* Kanji Control Block.                         */
#include "_Jopen.t"     /* Kanji Monitor Profile Keyword.               */
/*
 *      Copyright Identify.
 */
static char *cprt1="5601-061 COPYRIGHT IBM CORP 1988           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

static int   _Jop11(); 	/* Profile Initialize Values gets.      */
static int   _Jop12(); 	/* KCB Initialize values sets.          */
static void  _Jop13(); 	/* KMISA Initialize values sets.        */
static void  _Jop14(); 	/* EXT Initialize values sets KCB,KMISA */
static int   _Jop15(); 	/* Ascii Numeric Code Convert to Int.   */

/*
 *      Allocate Kanji Monitor Control Block Memory And
 *      KKC Interface Initialize.
 */
int     _Jopen( ppt,csid,ext )

KCB     **ppt;          /* Pointer to Kanji Control Block area pointer  */
                        /* to Save Area                                 */
short   csid;           /* Character set id.                            */
EXT     *ext;           /* Pointer to Extended Information Block        */

{
        short   _Kcopen();      /* KKC Routine.                         */
        char    *memcpy();      /* Specified # Character Copy.          */
        register KCB *pt;       /* Pointer to KCB.                      */
        register KMISA *kjsvpt; /* Pointer to KMISA.                    */
        TRB     *tracep;        /* Pointer to TRB.                      */

        int     ret_code;       /* Return Code.                         */
        int     ret_wcod;       /* Return Code Warning.                 */

        KMPF    kmpf[2];        /* KMPF Initialze Value Save Area.      */

        char    *kcbbase;       /* Kanji Control Block Base Address.    */
        char    *alocbase;      /* Allocation Base Address.             */
        int     size;           /* Allocate Memory Size (Per Byte).     */
        int     kcbmsg;         /* KCB Header Message Size.             */
        int     kcbsize;        /* KCB Substance Size.                  */
        int     kmisamsg;       /* KMISA Header Message Size.           */
        int     kmisasz;        /* KMISA Substance Size.                */
        int     fsbsize;        /* FSB Substance Size.                  */
        int     hlstlen;        /* Hilighting & String Length.          */
        int     aux1;           /* Auxiliary Area No.1 Substance Size.  */
	int	i;

        /*
         ****************************************************************
         *      0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Initial Return Code & Warning Level Return Code.
         */
        ret_code = KMSUCC;
        ret_wcod = KMSUCC;

        /*
         *      Collection Return Point.
         */
        do {
                /*
                 ********************************************************
                 *      1. Input Parameter Error Check.
                 ********************************************************
                 */
                /*
                 *      Display Width & Height Check(By Columns & Row).
                 */
                ret_code = KMMXSTCE;
                if(  ext->maxstc < M_MNMXSC )
                        break;

                ret_code = KMMXSTRE;
                if(  ext->maxstr!= M_MAXSTR )
                        break;

                /*
                 ********************************************************
                 *      2. Profile Information Get from 'Profile Data
                 *      file'.
                 ********************************************************
                 */
                /*
                 *      KMPF(Kanji Monitor Profile) Open,and its values
                 *      sets respect fields.
                 */
                ret_code = _Jop11( ext->prokmpf,kmpf );
                if( ret_code != IMSUCC ) {
                        /*
                         *      Check Warning Level Error.
                         */
                        if( !ISWARN(ret_code) ) {
                                /*
                                 *      Fatal Error Occure Return to
                                 *      Application.
                                 */
                                break;
                        } else {
                                /*
                                 *      Save Warning Level Warning.
                                 */
                                ret_wcod = ret_code;
                                /*
                                 *      Ignore Warning Level Error.
                                 */
                                ret_code = IMSUCC;
                        };
                };

                /*
                 ********************************************************
                 *      3. Allocate Control Block.
                 ********************************************************
                 */
                /*
                 *      Allocate Control Block.
                 *      KCB     :Header.
                 *      KCB     :Kanji Monitor Control Block.
                 *      KCB     :KCB Substatnce.
                 *      KMISA   :Header.
                 *      KMISA   :Kanji Monitor Internal Save Area.
                 *      KMISA   :KMISA Substance.
                 *      FSB     :Field Save Block.
                 *      FSB     :FSB Substance.
                 *      TRB     :Trace Block.
                 *
                 */
                /*
                 *      Kanji Control Block Header Message.
                 */
                kcbmsg = (sizeof(K_KCBMSG)/sizeof(long))*sizeof(long);

                /*
                 *      Kanji Monitor Internal Save Area Header Message.
                 */
                kmisamsg=(sizeof(K_ISAMSG)/sizeof(long))*sizeof(long);

                /*
                 *      Get Length of Input Field Hilighting Data.
                 */
                hlstlen = ALIGN(ext->maxstc * ext->maxstr,sizeof(long));

                /*
                 *      Get Auxiliaryu Area Size.
                 */
                if(    (ext->maxa1c <=0) || ((short)kmpf[0].aux1maxc<=0)
                    || (ext->maxa1r <=0) || ((short)kmpf[0].aux1maxr<=0 ) )
                        aux1 = 0;
                else
                        aux1 =   MIN( ext->maxa1r ,kmpf[0].aux1maxr )
                               * MIN( ext->maxa1c ,kmpf[0].aux1maxc );
                aux1    = ALIGN(aux1,sizeof(long));

                /*
                 *      Kanji Monitor Area Substatnce Size Calculate.
                 */
                kcbsize = hlstlen + aux1 + aux1;

                /*
                 *      Kanji Monitor Internal Save Area Substance
                 *      Size Caluculate.
                 */
                kmisasz =
                    /* Get Length of Kanji Conversion Map.              */
                    ALIGN(hlstlen+C_DBCS,sizeof(long))

                    /* Get Length of Background Save String Area.       */
                 +  hlstlen

                    /* Get Length of Kana Map.                          */
                 +  ALIGN( (hlstlen*4/C_BITBYT)+1,sizeof(long) )

                    /* Get Length of Kana Data.                         */
                 +  ALIGN( (hlstlen*4/C_DBCS),sizeof(long) )

                    /* Get Length of Grammer Map.                       */
                 +  ALIGN( hlstlen + 1,sizeof(long) )

                    /* Get Length of First Conversion String Save Area. */
/*======================================================================*/
/* #(B) Sept. 27 1988 Satoshi Higuchi                                   */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      +  ALIGN( hlstlen + 2,sizeof(long) )                            */
/* New source.                                                          */
/*      +  ALIGN( (hlstlen)*M_MULT + 2,sizeof(long) )                   */
/*======================================================================*/
                 +  ALIGN( (hlstlen)*M_MULT + 2,sizeof(long) )

                    /* Get Length of First Kanji Conversion Map Save Are*/
/*======================================================================*/
/* #(B) Sept. 27 1988 Satoshi Higuchi                                   */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      +  ALIGN( hlstlen/C_DBCS + 2,sizeof(long) )                     */
/* New source.                                                          */
/*      +  ALIGN( (hlstlen/C_DBCS)*M_MULT + 2,sizeof(long) )            */
/*======================================================================*/
                 +  ALIGN( (hlstlen/C_DBCS)*M_MULT + 2,sizeof(long) )

                    /* Get Length of First Grammer Map Save Area.       */
/*======================================================================*/
/* #(B) Sept. 27 1988 Satoshi Higuchi                                   */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      +  ALIGN( hlstlen + 1,sizeof(long) )+                           */
/* New source.                                                          */
/*      +  ALIGN( (hlstlen)*M_MULT + 1,sizeof(long) )                   */
/*======================================================================*/
                 +  ALIGN( (hlstlen)*M_MULT + 1,sizeof(long) )



/* #(B) 1987.12.14. Flying Conversion Add */
                    /* Get Length of Flying Conversion String Save Area.*/
                 +  ALIGN( hlstlen + 2,sizeof(long) )

                    /* Get Length of Flying Kanji Conversion Map Save Ar*/
                 +  ALIGN( hlstlen/C_DBCS + 2,sizeof(long) )

                    /* Get Length of Flying Grammer Map Save Area.      */
                 +  ALIGN( hlstlen + 1,sizeof(long) )

                    /* Get Length of Flying Conversion Kana Map.        */
                 +  ALIGN( (hlstlen*4/C_BITBYT)+1,sizeof(long) )

                    /* Get Length of Flying Conversion Kana Data.       */
                 +  ALIGN( (hlstlen*4/C_DBCS),sizeof(long) )
/* #(E) 1987.12.14. Flying Conversion Add */



                    /* Get Length of Internal Work Area.(For Input Field*/
                    /* Internal Work Area No.1-No.3                     */
                 +  ALIGN( hlstlen + 2,sizeof(long) )*3

                    /* Get Length of Internal Work Area No.4( For       */
                    /* Auxiliary Area.)                                 */
                 +  aux1;

                /*
                 *      Field Save Area Size Caluculate.
                 */
                fsbsize = hlstlen + hlstlen;

                /*
                 *      Total Allocation Size Caluculate.
                 */
                size    =  kcbmsg   + sizeof(KCB)     + kcbsize
                         + kmisamsg + sizeof(KMISA)   + kmisasz
                                    + sizeof(FSB) * 2 + fsbsize * 2
                         + sizeof(TRB);
                /*
                 *      Allocate Memory By Word(4Byte) Alignment
                 *      with Zerofill.
                 */
                kcbbase   = (char *)calloc( size/sizeof(long),sizeof(long));
                ret_code = KMMALOCE;
                if( kcbbase == NULL )
                        break;

                /*
                 *      Set Memory Block Allocation Work Variable.
                 */
                alocbase = kcbbase;

                /*
                 ********************************************************
                 *      4. Kanji Control Block Allocate & Initial Value
                 *      Sets.
                 ********************************************************
                 */
                /*
                 *      Allocate KCB Header Message
                 */
                (void)memcpy(alocbase,K_KCBMSG,kcbmsg);
                alocbase += kcbmsg;

                /*
                 *      Allocate KCB Static Data Area.
                 */
                pt = *ppt = (KCB *)alocbase;
                alocbase += sizeof(KCB);

                /*
                 *      Initialize Kanji Control Block.
                 */
                ret_code =
                        _Jop12(pt,kmpf,csid,kcbsize,&alocbase,hlstlen,aux1);
                if( ret_code != IMSUCC )
                        break;

                /*
                 ********************************************************
                 *      5. Kanji Monitor Internal Save Area Allocate
                 *      & Intial Value Sets.
                 ********************************************************
                 */
                /*
                 *      Allocate KMISA Header Message.
                 */
                (void)memcpy(alocbase,K_ISAMSG,kmisamsg);
                alocbase += kmisamsg;

                (void)_Jop13( pt,kmisasz,fsbsize,&alocbase,hlstlen,aux1 );
                kjsvpt       = pt->kjsvpt;

                /*
                 ********************************************************
                 *      6. Field Information Save Area Allocate &
                 *      Initial Value Sets.
                 ********************************************************
                 */
                /*
                 *      FSB(Field Save Block) Inititialize.
                 */
                /*
                 *      Allocate General Message Save Area.
                 */
                kjsvpt->ifsaveo = (FSB *)alocbase;
                alocbase += sizeof(FSB);

                /* Initialize Length of String Save Area.               */
                kjsvpt->ifsaveo->strmax  = hlstlen;
                /* Initialize Length of Hilight Attribute Save Area.    */
                kjsvpt->ifsaveo->hlatmax = hlstlen;
                /* Length of Save String.                               */
                kjsvpt->ifsaveo->length  = 0;
                /* Init Cursor Column Position.                         */
                kjsvpt->ifsaveo->curcol  = C_COL;
                /* Init Cursor Row Position.                            */
                kjsvpt->ifsaveo->currow  = C_ROW;
                /* Init Last Character Position.                        */
                kjsvpt->ifsaveo->lastch  = C_COL;
                /* Init Left Margin.                                    */
                kjsvpt->ifsaveo->curleft = C_COL;
                /* Init Right Margin.                                   */
                kjsvpt->ifsaveo->curright= C_COL+C_DBCS;

                /*
                 *      Allocate Dictionary Message Save Area.
                 */
                kjsvpt->ifsaved = (FSB *)alocbase;
                alocbase += sizeof(FSB);

                /* Initialize Length of String Save Area.               */
                kjsvpt->ifsaved->strmax  = hlstlen;
                /* Initialize Length of Hilight Attribute Save Area.    */
                kjsvpt->ifsaved->hlatmax = hlstlen;
                /* Length of Save String.                               */
                kjsvpt->ifsaveo->length  = 0;
                /* Init Cursor Column Position.                         */
                kjsvpt->ifsaved->curcol  = C_COL;
                /* Init Cursor Row Position.                            */
                kjsvpt->ifsaved->currow  = C_ROW;
                /* Init Last Character Position.                        */
                kjsvpt->ifsaved->lastch  = C_COL;
                /* Init Left Margin.                                    */
                kjsvpt->ifsaved->curleft = C_COL;
                /* Init Right Margin.                                   */
                kjsvpt->ifsaved->curright= C_COL+C_DBCS;

                /*
                 *      Allocate String Save Area.
                 */
                kjsvpt->ifsaveo->string =  (uchar *)alocbase;
                alocbase += hlstlen;

                /*
                 *      Allocate Hilighting Attribute Area.
                 */
                kjsvpt->ifsaveo->hlatst =  (uchar *)alocbase;
                alocbase += hlstlen;

                /*
                 *      Allocate String Save Area.
                 */
                kjsvpt->ifsaved->string =  (uchar *)alocbase;
                alocbase += hlstlen;

                /*
                 *      Allocate Hilighting Attribute Area.
                 */
                kjsvpt->ifsaved->hlatst =  (uchar *)alocbase;
                alocbase += hlstlen;

                /*
                 ********************************************************
                 *      7. Trace Control Block Allocate & Initialize
                 *      Value Sets.
                 ********************************************************
                 */
                /*
                 *      TRB(Trace Block) Initialize.
                 */
                pt->tracep = (TRB *)alocbase;
                tracep     = pt->tracep;
                alocbase  += sizeof(TRB);

                /*
                 *      Set Trace Block ID.
                 */
                (void)memcpy(tracep->trbblk.trblch,K_TRBID,
                                sizeof(tracep->trbblk.trblch));
                /*
                 *      Trace Block Length Set.
                 */
                tracep->trbblk.trblklen= sizeof(TRB);

                /*
                 *      Trace Block Data Address Set.
                 */
                tracep->trbblk.troutadr= sizeof(tracep->trbblk);

                /*
                 *      Trace Block Number Initialize First.
                 */
                tracep->trbblk.troutsno= 1;

                /*
                 *      Output Device Not Activate.
                 */
                tracep->trbblk.filds   = 0L;

                /*
                 *      Trace Mode Flag.
                 */
                tracep->trbblk.trflag  = 0;

                /*
                 *      Trace Block Usage End Offset.
                 */
                tracep->trbblk.trendadr= 0;

                /*
                 ********************************************************
                 *      8. Extended Information Block Data Reflect
                 *      Kanji Control Block.
                 ********************************************************
                 */
                (void)_Jop14(ext,pt,kmpf);

                /*
                 ********************************************************
                 *      9. Profile Initial Value Sets Kanji Monitor
                 *      Internal Save Area Profile Member.
                 ********************************************************
                 */
                /*
                 *      KMPF Initialize Value sets KMISA.
                 */
                (void)memcpy(kjsvpt->kmpf,kmpf,sizeof(kjsvpt->kmpf));

                /*
                 ********************************************************
                 *      10. Kana/Kanji Conversion Routine Open &
                 *      Return Code Analize.
                 ********************************************************
                 */
                /*
                 *      Open KKC. _Kcopen CALL.
                 */
 		 /* pass dict name to KKC 6/28/89 */
#if DBG_MSG
for(i=0; ext->dicts->sys[i] != NULL;i++ )
  fprintf(stderr, "jmnt:_Jopen(): sys name[%d] = [%s] being passed to kkc\n", i,ext->dicts->sys[i]);
fprintf(stderr, "jmnt:_Jopen(): user name = %s being passed to kkc\n", ext->dicts->user);
fprintf(stderr, "jmnt:_Jopen(): adj name  = %s being passed to kkc\n", ext->dicts->adj);
#endif

#ifdef DICTDEBUG 
fprintf(stderr, "sys name = %s being passed to kkc\n",
			ext->dicts->sys);
fprintf(stderr, "user name = %s being passed to kkc\n",
 			ext->dicts->user);
fprintf(stderr, "adj name = %s being passed to kkc\n",
			ext->dicts->adj);
#endif /* DICTDEBUG */
                kjsvpt->kkcrc = ret_code = _Kcopen(&kjsvpt->kkcbsvpt, 
 							    ext->dicts);
                if( !KKCPHYER(ret_code) ) {
			if (KKCMAJOR(ret_code) == K_KCDCTE) {
				/*
				 *	when the usrdict is broken, come here.
				 */
				free(kcbbase);
				*ppt = NULL;
				return KKUSDCOE;
			}
			else if (KKCMAJOR(ret_code) == K_KCLOGE) {
				free(kcbbase);
				*ppt = NULL;
				return KKFATALE;
			}
                        /*
                         *      Kana Kanji Routine Open Complete.
                         */
                        ret_code = KMSUCC;
                        break;
                };
                /*
                 *      Deallocate KCB,KMISA,TRB,FSB.
                 */
                (void)free( kcbbase );
                *ppt = NULL;

                /*
                 *      Phigical Error Occure.
                 */
                switch( KKCMAJOR(ret_code) ) {
                /*
                 * Physical error on System Dictionary.
                 */
                case K_KCSYPE :
                        ret_code = KKSYDCOE;    /* Set return code.     */
                        break;
                /*
                 * Physical error on User   Dictionary.
                 */
                case K_KCUSPE :
                        /*
                         *      Analize Minor Error Code.
                         */
                        ret_code = KKUSDCOE;    /* Set return code.     */
                        break;
                /*
                 * Physical error on Adjunct Dictionary.
                 */
                case K_KCFZPE :
                        ret_code = KKFZDCOE;    /* Set return code.     */
                        break;
                /*
                 * Memory allocation error.
                 */
                case K_KCMALE :
                        ret_code = KKMALOCE;    /* Set return code.     */
                        break;
                /*
                 *      If Unknown Error Accept then 'KKC Return Code'
                 *      Return to Application.
                 */
                default:
                        ret_code = KKFATALE;    /* Set return code.     */
                        break;
                };
        } while( NILCOND );

        /*
         ****************************************************************
         *      11. Return to Caller.
         ****************************************************************
         */
        if( ret_code == KMSUCC )
                ret_code = ret_wcod;

        /*
         *      Debugging Output.
         */
        return( ret_code );
}

/*----------------------------------------------------------------------*
 *      Profile Data Set Profile Work Variable.
 *----------------------------------------------------------------------*/
static int  _Jop11( profile, kmpf )
KMPF    *profile;       /* KMPF Initialize Value Source Area.		*/
			/* If this is NULL, kmpf is set to defaults.	*/
KMPF    *kmpf;          /* KMPF Initialize Value Save Area.             */
{
        char    *memset();      /* Memory Set Specified Character.      */
        int     ret_code;       /* Return Code.                 	*/
        uchar   readonly;       /* Readonly Indicator.          	*/

        /*--------------------------------------------------------------*
         *      Initialize Return Default Value.
         *--------------------------------------------------------------*/
        ret_code = IMSUCC;

        /*--------------------------------------------------------------*
         *      KMPF All Field Set Zero.
         *--------------------------------------------------------------*/
        (void)memset(kmpf,'\0',sizeof(KMPF)*2);

	if ( profile != NULL ) {

		/*----- Copy the contents from 'profile' to 'kmpf' -----*/
		kmpf[0].allcand  = profile->allcand;
		kmpf[0].insert   = profile->insert;
		kmpf[0].kjin     = profile->kjin;
		kmpf[0].reset    = profile->reset;
		kmpf[0].conversn = profile->conversn;
		kmpf[0].kjno     = profile->kjno;
		kmpf[0].aux1     = profile->aux1;
		kmpf[0].aux2     = profile->aux2;
		kmpf[0].aux3     = profile->aux3;
		kmpf[0].aux4     = profile->aux4;
		kmpf[0].aux1maxc = profile->aux1maxc;
		kmpf[0].aux1maxr = profile->aux1maxr;
		kmpf[0].aux2maxc = profile->aux2maxc;
		kmpf[0].aux2maxr = profile->aux2maxr;
		kmpf[0].aux3maxc = profile->aux3maxc;
		kmpf[0].aux3maxr = profile->aux3maxr;
		kmpf[0].aux4maxc = profile->aux4maxc;
		kmpf[0].aux4maxr = profile->aux4maxr;
		kmpf[0].beep     = profile->beep;
		kmpf[0].regist   = profile->regist;
		kmpf[0].mix      = profile->mix;
		kmpf[0].indicat  = profile->indicat;
		kmpf[0].rkc      = profile->rkc;
		kmpf[0].kana     = profile->kana;
		kmpf[0].kblock   = profile->kblock;
		kmpf[0].cursout  = profile->cursout;
		kmpf[0].katakana = profile->katakana;
		kmpf[0].alphanum = profile->alphanum;
		kmpf[0].learning = profile->learning;
		kmpf[0].kanjinum = profile->kanjinum;
		kmpf[0].pfkreset = profile->pfkreset;
		kmpf[0].convtype = profile->convtype;
		kmpf[0].kuten    = profile->kuten;
		kmpf[0].shmatnum = profile->shmatnum;
		kmpf[0].udload   = profile->udload;
		kmpf[0].modereset = profile->modereset;
		kmpf[0].dnames.sys = profile->dnames.sys;
		kmpf[0].dnames.user = profile->dnames.user;
		kmpf[0].dnames.adj = profile->dnames.adj;
	}
	else {
		/*----- 'profile' is not defined correctly -------------*/
        	/*------------------------------------------------------*
        	 * Insert/Replace Switch Set Default Value.	
        	 * DEFAULT      :INSERT
        	 * ALTERNATE KEY:One of INSERT or RESET
        	 *------------------------------------------------------*/
        	kmpf[0].insert   = K_INSERT;

        	/*------------------------------------------------------*
        	 * Kanji Number or Dictinary Registration Decide Key.
        	 * DEFAULT      :ACTION or ENTER or CR
        	 * ALTERNATE KEY:Which is ACTION or ENTER or CR
        	 *------------------------------------------------------*/
        	kmpf[0].kjin     = K_DAACT | K_DAENT | K_DACR;

        	/*------------------------------------------------------*
        	 * Cancel Error Reset Key Default Value Set.
        	 * DEFAULT      :ACTION or ENTER or CR or RESET
        	 * ALTERNATE KEY:Which is ACTION or ENTER or CR or RESET
        	 *------------------------------------------------------*/
        	kmpf[0].reset    = K_REACT | K_REENT | K_RECR | K_RERES;

        	/*------------------------------------------------------*
        	 * Kanji Conversion Mode Default Value Set.
        	 * DEFAULT      :MPHASE
        	 * ALTERNATE KEY:One of WORD or SPHASE
        	 * WORD         :Multi Phrase Conversion Mode.
        	 * SPHASE       :Single Phrase Conversion Mode.
        	 * MPHASE       :Multi Phrase Conversion Mode.
        	 *-----------------------------------------------------*/
        	kmpf[0].conversn = K_RENBUN;

        	/*-----------------------------------------------------*
        	 * Kanji Number Input Mode Default Value Set.
        	 * DEFAULT      :JIS
        	 * ALTERNATE KEY:Which is JIS or EBCDIC or KJNO or PC
        	 * JIS          :JIS Kanji Segment Number.
        	 * EBCDIC       :EBCDIC DBCS Kanji Number.
        	 * KJNO         :Internal Kanji Number.
        	 * PC           :Japanese PC KanjiNumber(Shift Jis Number).
        	 *-----------------------------------------------------*/
        	kmpf[0].kjno     = K_KJIS;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.1 CursorDisplay Position DefaultValue.
        	 * DEFAULT      :CURSOR
        	 * ALTERNATE KEY:One of CURSOR or CENTER or UL or UR or
        	 *               LL or LR.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux1     = K_AUXNEA;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.1 Screen Columns Default Value.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux1maxc = 0;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.1 Screen Row Default Value.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux1maxr = 0;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.2 CursorDisplay Position DefaultValue.
        	 * DEFAULT      :CURSOR
        	 * ALTERNATE KEY:One of CURSOR or CENTER or UL or UR or
        	 *                    LL or LR.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux2     = K_AUXNEA;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.2 Screen Columns Default Value.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux2maxc = 0;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.2 Screen Row Default Value.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux2maxr = 0;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.3 CursorDisplay Position DefaultValue.
        	 * DEFAULT      :CURSOR
        	 * ALTERNATE KEY:One of CURSOR or CENTER or UL or UR or
        	 *               LL or LR.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux3     = K_AUXNEA;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.3 Screen Columns Default Value.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux3maxc = 0;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.3 Screen Row Default Value.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux3maxr = 0;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.4 CursorDisplay Position DefaultValue.
        	 * DEFAULT      :CURSOR
        	 * ALTERNATE KEY:One of CURSOR or CENTER or UL or UR or
        	 *               LL or LR.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux4     = K_AUXNEA;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.4 Screen Columns Default Value.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux4maxc = 0;

        	/*-----------------------------------------------------*
        	 * Auxiliary Area No.4 Screen Row Default Value.
        	 *-----------------------------------------------------*/
        	kmpf[0].aux4maxr = 0;

        	/*-----------------------------------------------------*
        	 * Caution Bell Default Value Set.
        	 * DEFAULT      :ON
        	 * ALTERNATE KEY:One of ON or OFF
        	 *-----------------------------------------------------*/
        	kmpf[0].beep     = K_BEEPOF;

        	/*-----------------------------------------------------*
        	 * Dictionary Registration Mode Default Value Set.
        	 * DEFAULT      :ON
        	 * ALTERNATE KEY:One of ON or OFF
        	 *-----------------------------------------------------*/
       		kmpf[0].regist   = K_DICNG;

        	/*-----------------------------------------------------*
        	 * DBCSField MIXModeSupport Enable/DIable DefaultValue Set.
        	 * DEFAULT      :OFF
        	 * ALTERNATE KEY:One of ON or OFF
        	 *-----------------------------------------------------*/
        	kmpf[0].mix      = K_MIXNG;

        	/*-----------------------------------------------------*
        	 * Input Field Indicator Length Default Value Set.
        	 * DEFAULT      :Numberic Number Zero.
        	 * ALTERNATE KEY:Numberic Number must be Even Number.
        	 *-----------------------------------------------------*/
        	kmpf[0].indicat  = 0;

        	/*-----------------------------------------------------*
        	 * Romaji/Kana Conversion Enable/Diable Default Value Set.
        	 * DEFAULT      :OFF
        	 * ALTERNATE KEY:One of ON or OFF
        	 *-----------------------------------------------------*/
        	/***********************************************************/
        	/*#(B)  Bug Fix. Mon Dec 14,1987                           */
        	/*      Modify Reason.                                     */
        	/*              Default RKC Mode is OFF,But Now ON.        */
        	/*      Change Source.                                     */
        	/*              kmpf[0].rkc = K_ROMON                      */
        	/*                 v                                       */
        	/*              kmpf[0].rkc = K_ROMOFF                     */
        	/***********************************************************/
        	/***********************************************************/
        	/*#(B)  Bug Fix. Mon Dec 18,1987                           */
        	/*      Modify Reason.                                     */
        	/*              Default RKC Mode is ON,But Now OFF.        */
        	/*      Change Source.                                     */
        	/*              kmpf[0].rkc = K_ROMOFF                     */
        	/*                 v                                       */
        	/*              kmpf[0].rkc = K_ROMON                      */
        	/***********************************************************/
        	kmpf[0].rkc      = K_ROMON;

        	/*-----------------------------------------------------*
        	 * Initial Romaji/Kana Conversion Mode Default Value Set.
        	 * DEFAULT      :HIRAGANA
        	 * ALTERNATE KEY:One of KATAKANA
        	 * HIRAGANA     :Japanese 'hiragana' Code Conversion.
        	 * KATAKANA     :Japanese 'katakana' Code Conversion.
        	 *-----------------------------------------------------*/
        	kmpf[0].kana     = K_KANA;

        	/*-----------------------------------------------------*
        	 * Keyboard Lock Indicator Display Enable/Disable Default
        	 * Value Set.
        	 * DEFAULT      :ON
        	 * ALTERNATE KEY:One of ON or OFF
        	 *-----------------------------------------------------*/
        	kmpf[0].kblock   = K_KBLON;

        	/*-----------------------------------------------------*
        	 * Cursor Move Range Infield/Contains Outfield Defautl
        	 * Value Set.
        	 * DEFULT       :OFF
        	 * ALTERNATE KEY:One of ON or OFF
        	 *-----------------------------------------------------*/
        	kmpf[0].cursout  = K_CURIN;

        	/*-----------------------------------------------------*
        	 * Initial Keyboard Kana/Hiragana Default Value Set.
        	 * DEFAULT      :HIRAGANA
        	 * ALTERNATE KEY:One of KATAKANA or HIRAGANA
        	 *-----------------------------------------------------*/
        	kmpf[0].katakana = K_KANAON;

        	/*-----------------------------------------------------*
        	 * Alphanumeric DBCS Conversion Enable/Disable Defaut Value
        	 * Sets.
        	 * DEFAULT      :ON
        	 * ALTERNATE KEY:One of ON or OFF.
        	 *-----------------------------------------------------*/
        	kmpf[0].alphanum = K_ALPON;

        	/*-----------------------------------------------------*
        	 * Dictionary Learing Enable/Disable.
        	 * DEFAULT      :ON
        	 * ALTERNATE KEY:One of ON or OFF.
        	 *-----------------------------------------------------*/
        	kmpf[0].learning = K_LEAON;

        	/*-----------------------------------------------------*
        	 * All Candidate Mode Candidate List Number Default Value.
        	 * DEFAUTT      :0
        	 * ALTERNATE KEY:From 0 to 32767.
        	 *-----------------------------------------------------*/
        	kmpf[0].allcand  = 0;

        	/*-----------------------------------------------------*
        	 * Kanji Number Conversion Enable/Disable.
        	 * DEFAULT      :ON
        	 * ALTERNATE KEY:One of ON or OFF.
        	 *-----------------------------------------------------*/
        	kmpf[0].kanjinum = K_KJNMON;

        	/*-----------------------------------------------------*
        	 * Control Sequence reatch from Input Data when
        	 * Current Conversion Decide or Ignore?
        	 * ON           :If Message Display in Auxiliary Area or 
		 *	      	 Input Field then Decide Conversion Data.
        	 * OFF          :Ignore All.
        	 * DEFAULT      :OFF
        	 * ALTERNATE KEY:One of ON or OFF.
        	 *-----------------------------------------------------*/
        	kmpf[0].pfkreset = K_PFKOFF;

	/* #(B) 1987.12.15. Flying Conversion Add */
        	/*-----------------------------------------------------*
        	 * Conversion Type.
        	 * DEFAULT      :CHIKUJI
        	 * ALTERNATE KEY:One of CHIKUJI or IKKATU
        	 * CHIKUJI      :Flying Conversion.
        	 * IKKATU       :Conversion.
        	 *-----------------------------------------------------*/
        	kmpf[0].convtype = K_CIKUJI;
	/* #(E) 1987.12.15. Flying Conversion Add */

		kmpf[0].kuten = K_JIS83;

		/*-----------------------------------------------------*
		 * Change dictionary files for KKC if need.
		 * DEFAULT     :1
		 * K_SHNUMDEF  :1
		 * else        :0<=N<=255
		 *-----------------------------------------------------*/
		kmpf[0].shmatnum = K_SHNUMDEF;

		/*-----------------------------------------------------*
		 * Loading User Dictionary Automatically.
		 * DEFAULT    :Load user dictionary if it has been updated
		 * K_UDLOADOFF:Not load user dictionary
		 * K_UDLOADON :Load user dictionary if it has been updated
		 *-----------------------------------------------------*/
		kmpf[0].udload = K_UDLOADON;

		/*-----------------------------------------------------*
		 * Reset input mode with ESC key.
		 * DEFAULT     :Reset input mode with ESC key
		 * K_RESETOFF  :Not reset input mode with ESC key
		 * K_RESETON   :Reset input mode with ESC key
		 *-----------------------------------------------------*/
		kmpf[0].modereset  = K_RESETON;
	}

        /*-------------------------------------------------------------*
         *      Modify Permission Set.
         *-------------------------------------------------------------*/
        readonly = C_SWOFF;

	/*----- all KMPF[1] areas are set to C_SWOFF ------------------*/
        kmpf[1].allcand = readonly;
        kmpf[1].insert  = readonly;
        kmpf[1].kjin    = readonly;
        kmpf[1].reset   = readonly;
        kmpf[1].conversn = readonly;
        kmpf[1].kjno    = readonly;
        kmpf[1].aux1    = readonly;
        kmpf[1].aux2    = readonly;
        kmpf[1].aux3    = readonly;
        kmpf[1].aux4    = readonly;
        kmpf[1].aux1maxc = readonly;
        kmpf[1].aux1maxr = readonly;
        kmpf[1].aux2maxc = readonly;
        kmpf[1].aux2maxr = readonly;
        kmpf[1].aux3maxc = readonly;
        kmpf[1].aux3maxr = readonly;
        kmpf[1].aux4maxc = readonly;
        kmpf[1].aux4maxr = readonly;
        kmpf[1].beep    = readonly;
        kmpf[1].regist  = readonly;
        kmpf[1].mix     = readonly;
        kmpf[1].indicat = readonly;
        kmpf[1].rkc     = readonly;
        kmpf[1].kana    = readonly;
        kmpf[1].kblock  = readonly;
        kmpf[1].cursout = readonly;
        kmpf[1].katakana = readonly;
        kmpf[1].alphanum = readonly;
        kmpf[1].learning = readonly;
        kmpf[1].kanjinum = readonly;
        kmpf[1].pfkreset = readonly;
        kmpf[1].convtype = readonly;
        kmpf[1].kuten    = readonly;
        kmpf[1].shmatnum = readonly;
        kmpf[1].udload   = readonly;
        kmpf[1].modereset = readonly;

        return( ret_code );
}

/*
 *      Sets KCB Initial Value.
 */
static int  _Jop12( pt,kmpf,csid,kcbsize,alocbase,hlstlen,aux1 )

register KCB     *pt;   /* Pointer to KCB.                              */
KMPF    *kmpf;          /* KMPF initial data.                           */
short   csid;           /* Character set id.                            */
int     kcbsize;        /* KCB Substatnce Length.                       */
char    **alocbase;     /* Allocation Base Address.                     */
int     hlstlen;        /* HIghlingting & String Length.                */
int     aux1;           /* Auxiliary Area Size.                         */

{
        /*
         ****************************************************************
         *      1-2. 1. Set Kanji Monitor Control Block Length.
         ****************************************************************
         */
        pt->length   = sizeof(KCB) + kcbsize;

        /*
         ****************************************************************
         *      1-2. 2. Set Kanji Monitor Identifier Sets.
         ****************************************************************
         */
        pt->id       = K_KCBID;

        /*
         ****************************************************************
         *      1-2. 3. Allocate Kanji Control Block Hilighting Area for
         *      Input Field.
         ****************************************************************
         */
        /*
         *      HIghlighting Attribute Area Address Set.
         */
        pt->hlatst   = (uchar *)*alocbase;
        *alocbase    += hlstlen;

        /*
         ****************************************************************
         *      1-2. 4. Allocate Kanji Control Block Trace Control Block
         *      Area.
         *      It's allocated by _Jopen() Master Routine.
         ****************************************************************
         */

        /*
         ****************************************************************
         *      1-2. 5. Allocate Kanji Control Block DBCS String Area for
         *      Auxliary Area No.1
         ****************************************************************
         */
        /*
         *      Auxiliary area No.1 data area Address &
         *      Highlighting Address set.
         */
        pt->aux1     = (uchar *)*alocbase;
        *alocbase   += aux1;

        /*
         ****************************************************************
         *      1-2. 6. Allocate Kanji Control Block Hilighting Area for
         *      Auxliary Area No.1
         ****************************************************************
         */
        pt->hlata1   = (uchar *)*alocbase;
        *alocbase   += aux1;

        /*
         ****************************************************************
         *      1-2. 7. Set Character Set ID.
         ****************************************************************
         */
        /*
         *      Set Character Set ID.
         *      370: Japnese Kanji Code.
         */
        pt->csid     = csid;

        /*
         ****************************************************************
         *      1-2. 8. Kanji Control Block Initialize Value Set.
         ****************************************************************
         */
        pt->string   = NULL;    /* String Addreess NULL Assign.         */
        pt->aux2     = NULL;    /* Auxiliary Area 2 String Init NULL    */
        pt->hlata2   = NULL;    /* Auxiliary Area 2 Hilighting 1nit NULL*/
        pt->aux3     = NULL;    /* Auxiliary Area 3 String Init NULL    */
        pt->hlata3   = NULL;    /* Auxiliary Area 3 Hilighting 1nit NULL*/
        pt->aux4     = NULL;    /* Auxiliary Area 4 String Init NULL    */
        pt->hlata4   = NULL;    /* Auxiliary Area 4 Hilighting 1nit NULL*/
        pt->auxdir   = NULL;    /* Auxiliary DIrect Area Init NULL.     */
        pt->actcol   = C_COL;   /* Active FIeld Column Number Set.      */
        pt->actrow   = C_ROW;   /* Active FIeld Row Number Set.         */
        pt->ax1col   = 0;       /* Auxiliary Area 1 Column Position Set.*/
        pt->ax1row   = 0;       /* Auxiliary Area 1 Row Position Set.   */
        pt->ax2col   = 0;       /* Auxiliary Area 2 Column Position Set.*/
        pt->ax2row   = 0;       /* Auxiliary Area 2 Row Position Set.   */
        pt->ax3col   = 0;       /* Auxiliary Area 3 Column Position Set.*/
        pt->ax3row   = 0;       /* Auxiliary Area 3 Row Position Set.   */
        pt->ax4col   = 0;       /* Auxiliary Area 4 Column Position Set.*/
        pt->ax4row   = 0;       /* Auxiliary Area 4 Row Position Set.   */
        pt->curcol   = C_COL;   /* Input FIeld Column Position Set.     */
        pt->currow   = C_ROW;   /* Input FIeld Row Position Set.        */
        pt->setcsc   = C_COL;   /* Input FIeld Column Display Pos Set.  */
        pt->setcsr   = C_ROW;   /* Input FIeld Row Display Pos Set.     */
        pt->cura1c   = C_COL;   /* Auxilairy Area 1 Column Display Pos. */
        pt->cura1r   = C_ROW;   /* Auxilairy Area 1 Row    Display Pos. */
        pt->cura2c   = C_COL;   /* Auxilairy Area 2 Column Display Pos. */
        pt->cura2r   = C_ROW;   /* Auxilairy Area 2 Row    Display Pos. */
        pt->cura3c   = C_COL;   /* Auxilairy Area 3 Column Display Pos. */
        pt->cura3r   = C_ROW;   /* Auxilairy Area 3 Row    Display Pos. */
        pt->cura4c   = C_COL;   /* Auxilairy Area 4 Column Display Pos. */
        pt->cura4r   = C_ROW;   /* Auxilairy Area 4 Row    Display Pos. */
        pt->chpos    = 0;       /* Input FIeld Redraw Start Position Set*/
        pt->chlen    = 0;       /* Input FIeld Redraw Length Set.       */
        pt->chpsa1   = 0;       /* Auxiliary Area 1 Redraw Start Pos Set*/
        pt->chlna1   = 0;       /* Auxiliary Area 1 Redraw Length Set.  */
        pt->chpsa2   = 0;       /* Auxiliary Area 2 Redraw Start Pos Set*/
        pt->chlna2   = 0;       /* Auxiliary Area 2 Redraw Length Set.  */
        pt->chpsa3   = 0;       /* Auxiliary Area 3 Redraw Start Pos Set*/
        pt->chlna3   = 0;       /* Auxiliary Area 3 Redraw Length Set.  */
        pt->chpsa4   = 0;       /* Auxiliary Area 4 Redraw Start Pos Set*/
        pt->chlna4   = 0;       /* Auxiliary Area 4 Redraw Length Set.  */
        pt->lastch   = 0;       /* Field Last Position Set.             */
        pt->type     = 0;       /* Input Code Type None.                */
        pt->code     = 0;       /* Input Code None.                     */
        pt->flatsd   = K_ODUBYT;/* Input Field Type DBCS.               */
        pt->axuse1   = 0;       /* Auxiliary Area 1 Not Use.            */
        pt->axuse2   = 0;       /* Auxiliary Area 2 Not Use.            */
        pt->axuse3   = 0;       /* Auxiliary Area 3 Not Use.            */
        pt->axuse4   = 0;       /* Auxiliary Area 4 Not Use.            */
        pt->ax2loc   = 0;       /* Auxiliary Area 2 Cursor Center.      */
        pt->ax3loc   = 0;       /* Auxiliary Area 3 Cursor Center.      */
        pt->ax4loc   = 0;       /* Auxiliary Area 4 Cursor Center.      */
        pt->shift    = K_STNOT; /* Shift Status Not Change.             */
        pt->shift1   = K_ST1UDF;/* Shift Status Undefine.               */
        pt->shift3   = 0;       /* RKC Sff.                             */
        pt->shift4   = 0;       /* Single Byte Shift.                   */
        pt->curlen   = K_C2BYTC;/* Cursor is Double Byte.               */
        pt->cnvsts   = 0;       /* Now Not Conversion Data Active.      */
        pt->repins   = 0;       /* Replace/Insert Mode Undef.           */
        pt->beep     = 0;       /* Beep Off.                            */
        pt->discrd   = 0;       /* Internal Loop OFF.                   */
        pt->trace    = K_TLIMIT;/* Trace Mode is on CORE.               */
        pt->conv     = 0;       /* KKC Conversion Disable.              */
        pt->kbdlok   = 0;       /* Keyboard Not Locked.                 */
        pt->rsv1[0]  = 0;       /*--------------------------------------*/
        pt->rsv1[1]  = 0;       /*                                      */
        pt->rsv1[2]  = 0;       /*                                      */
        pt->rsv2     = 0;       /* ***** RESERVED FOR FUTURE USE ****   */
        pt->rsv3     = 0;       /*                                      */
        pt->rsv4     = 0;       /* Initialize Zero.                     */
        pt->rsv5     = 0;       /*                                      */
        pt->rsv6     = 0;       /*                                      */
        pt->rsv7     = 0;       /*                                      */
        pt->rsv8     = 0;       /*--------------------------------------*/

        /*
         ****************************************************************
         *      1-2. 8. Kanji Monitor Profile Initial Data Kanji Control
         *      Block.
         ****************************************************************
         */
        /*
         *      Auxiliary Area 1 Cursor Position Set From Kanji Monitor
         *      Profile.
         */
        pt->ax1loc   = kmpf[0].aux1;

        /*
         *      Indicator Length Set From Kanji Monitor Profile.
         */
        pt->indlen   = kmpf[0].indicat;

        /*
         *      Hiragana/Katakana Conversion Mode Set From
         *      Kanji Monitor Profile.
         */
        pt->shift2   = kmpf[0].rkc;

        /*
         ****************************************************************
         *      1-2. 9. Return to Caller.
         ****************************************************************
         */
        return( IMSUCC );
}

/*
 *      KMISA Initialize.
 */
static void _Jop13( pt,kmisasz,fsbsize,alocbase,hlstlen,aux1 )

register KCB     *pt;   /* Pointer To Kanji Monitor Control Block.      */
int     kmisasz;        /* KMISA Substance Length.                      */
int     fsbsize;        /* FSB Substance Length.                        */
char    **alocbase;     /* Allocation Base Address.                     */
int     hlstlen;        /* HIghlingting & String Length.                */
int     aux1;           /* Auxiliary Area Size.                         */

{
        register KMISA *kjsvpt; /* Pointer to KMISA.                    */

        /*
         ****************************************************************
         *      1-3. 1. Kanji Monitor Internal Save Area Allocate.
         ****************************************************************
         */
        /*
         *      KMISA Address Allocate.
         */
        pt->kjsvpt = (KMISA *)*alocbase;
        kjsvpt     = pt->kjsvpt;
        *alocbase += sizeof(KMISA);

        /*
         *      KMISA   length & ID  sets.
         */
        kjsvpt->length  = sizeof(KMISA) + kmisasz + sizeof(FSB)*2
                         + fsbsize * 2;

        /*
         *      Kanji Monitor Internal Save Area Idenfier.
         */
        kjsvpt->kmisaid = K_ISAID;

        /*
         ****************************************************************
         *      1-3. 2. Kanji Monitor Internal Save Area 'Pointer Area'
         *      Address Initialize.
         ****************************************************************
         */
        /*
         *      Kanji Conversion Map Allocate.
         */
        kjsvpt->kjcvmax = ALIGN(hlstlen+C_DBCS,sizeof(long));
        kjsvpt->kjcvmap = (uchar *)*alocbase;
        *alocbase += kjsvpt->kjcvmax;

        /*
         *      Background String Save Area Allocate.
         */
        kjsvpt->savemax = hlstlen;
        kjsvpt->stringsv= (uchar *)*alocbase;
        *alocbase += kjsvpt->savemax;


        /*
         *      Kana Map Area Allocate.
         */
        kjsvpt->kanabmax=
                ALIGN((hlstlen*4/C_BITBYT)+1,sizeof(long));
        kjsvpt->kanamap = (uchar *)*alocbase;
        *alocbase += kjsvpt->kanabmax;

        /*
         *      Conversion Kana Yomi Data Area Allocate.
         */
        kjsvpt->kanamax = ALIGN( (hlstlen*4/C_DBCS),sizeof(long));
        kjsvpt->kanadata= (uchar *)*alocbase;
        *alocbase += kjsvpt->kanamax;

        /*
         *      Conversion Grammer Map Area Allocate.
         */
        kjsvpt->grammax = ALIGN( hlstlen+1,sizeof(long));
        kjsvpt->grammap = (uchar *)*alocbase;
        *alocbase += kjsvpt->grammax;


        /*
         *      First Conversion Kanji Data Area Allocate.
         */
/*======================================================================*/
/* #(B) Sept. 27 1988 Satoshi Higuchi                                   */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      kjsvpt->dat1smax= ALIGN( hlstlen+2,sizeof(long));               */
/* New source.                                                          */
/*      kjsvpt->dat1smax= ALIGN( (hlstlen)*M_MULT+2,sizeof(long));      */
/*======================================================================*/
        kjsvpt->dat1smax= ALIGN( (hlstlen)*M_MULT+2,sizeof(long));
        kjsvpt->kjdata1s= (uchar *)*alocbase;
        *alocbase += kjsvpt->dat1smax;

        /*
         *      First Conversion Kanji Conversion Map Area Allocate.
         */
/*======================================================================*/
/* #(B) Sept. 27 1988 Satoshi Higuchi                                   */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      kjsvpt->map1smax= ALIGN( hlstlen/C_DBCS + 2,sizeof(long));      */
/* New source.                                                          */
/*      kjsvpt->map1smax= ALIGN((hlstlen/C_DBCS)*M_MULT+2,sizeof(long));*/
/*======================================================================*/
        kjsvpt->map1smax= ALIGN((hlstlen/C_DBCS)*M_MULT+2,sizeof(long));
        kjsvpt->kjmap1s = (uchar *)*alocbase;
        *alocbase += kjsvpt->map1smax;

        /*
         *      First Conversion Grammer Map Area Allocate.
         */
/*======================================================================*/
/* #(B) Sept. 27 1988 Satoshi Higuchi                                   */
/* Changed source.                                                      */
/* Old source.                                                          */
/*      kjsvpt->gra1smax= ALIGN( hlstlen+1,sizeof(long));               */
/* New source.                                                          */
/*      kjsvpt->gra1smax= ALIGN( (hlstlen)*M_MULT+1,sizeof(long));      */
/*======================================================================*/
        kjsvpt->gra1smax= ALIGN( (hlstlen)*M_MULT+1,sizeof(long));
        kjsvpt->gramap1s= (uchar *)*alocbase;
        *alocbase += kjsvpt->gra1smax;



/* #(B) 1987.12.08. Flying Conversion Add */
        /*
         *      Flying Conversion Kanji Data Area Allocate.
         */
        kjsvpt->datafmax = ALIGN( hlstlen+2,sizeof(long));
        kjsvpt->kjdataf  = (uchar *)*alocbase;
        *alocbase += kjsvpt->datafmax;

        /*
         *      Flying Conversion Kanji Conversion Map Area Allocate.
         */
        kjsvpt->mapfmax = ALIGN( hlstlen/C_DBCS + 2,sizeof(long));
        kjsvpt->kjmapf  = (uchar *)*alocbase;
        *alocbase += kjsvpt->mapfmax;

        /*
         *      Flying Conversion Grammer Map Area Allocate.
         */
        kjsvpt->grafmax = ALIGN( hlstlen+1,sizeof(long));
        kjsvpt->gramapf = (uchar *)*alocbase;
        *alocbase += kjsvpt->grafmax;

        /*
         *      Flying Kana Map Area Allocate.
         */
        kjsvpt->knmpfmax = ALIGN((hlstlen*4/C_BITBYT)+1,sizeof(long));
        kjsvpt->kanamapf = (uchar *)*alocbase;
        *alocbase += kjsvpt->knmpfmax;

        /*
         *      Flying Conversion Kana Yomi Data Area Allocate.
         */
        kjsvpt->kanafmax = ALIGN( (hlstlen*4/C_DBCS),sizeof(long));
        kjsvpt->kanadatf = (uchar *)*alocbase;
        *alocbase += kjsvpt->kanafmax;
/* #(E) 1987.12.08. Flying Conversion Add */



        /*
         *      Internal Work Area For Conversion Operation Allocate.
         */
        kjsvpt->iws1max = ALIGN( hlstlen+2,sizeof(long));
        kjsvpt->iws1    = (uchar *)*alocbase;
        *alocbase += kjsvpt->iws1max;

        /*
         *      Internal Work Area For Conversion Operation Allocate.
         */
        kjsvpt->iws2max = ALIGN( hlstlen+2,sizeof(long));
        kjsvpt->iws2    = (uchar *)*alocbase;
        *alocbase += kjsvpt->iws2max;

        /*
         *      Internal Work Area For All Candidate Allocate.
         */
        kjsvpt->iws3max = ALIGN( hlstlen+2,sizeof(long));
        kjsvpt->iws3    = (uchar *)*alocbase;
        *alocbase += kjsvpt->iws3max;

        /*
         *      Internal Work Area For All Candidate Alocate.
         */
        kjsvpt->iws4max = aux1;
        if( kjsvpt->iws4max !=0 )
                kjsvpt->iws4    = (uchar *)*alocbase;
        else
                kjsvpt->iws4    = NULL;
        *alocbase += kjsvpt->iws4max;

        /*
         ****************************************************************
         *      1-3. 3. Sets Kanji Monitor Internal Save Area
         *      Profile is Processing Program _Jopen() Master Module.
         ****************************************************************
         */
        /*
         ****************************************************************
         *      1-3. 4. Sets Auxiliary Area Column & Row Number
         *      Processing Program _Jop14() Routine.
         ****************************************************************
         */


        /*
         ****************************************************************
         *      1-3. 5. Kanji Monitor Internal Save Area Initial Value
         *      Sets.
         ****************************************************************
         */
        /*
         *      Other KMISA field initialize.
         */
        kjsvpt->grammap[0]= 1;  /* Set Initial Grammer Map Length.      */
        kjsvpt->kanamap[0]= 1;  /* Set Initial Kana Map Length.         */
        SHTOCHPT(kjsvpt->kjdata1s,2);
                                /* Set Initial First Conversion Kanji   */
                                /* Data Length.                         */
        SHTOCHPT(kjsvpt->kjmap1s,2);
                                /* Set Initial First Conversion Map     */
                                /* Length.                              */
        kjsvpt->gramap1s[0] = 1;/* Set Initial First Grammer Map.       */



/* #(B) 1987.12.08. Flying Conversion Add */
        SHTOCHPT(kjsvpt->kjdataf,2);
                                /* Set Initial Flying Conversion Kanji  */
                                /* Data Length.                         */
        SHTOCHPT(kjsvpt->kjmapf,2);
                                /* Set Initial Flying Conversion Map    */
                                /* Length.                              */
        kjsvpt->gramapf[0]  = 1;/* Set Initial Flying Grammer Map.      */

        kjsvpt->kanamapf[0] = 1;/* Set Initial Flying Kanamap Length    */
/* #(E) 1987.12.08. Flying Conversion Add */



        kjsvpt->nextact = 0;    /* Next Action Code Reset.              */
        kjsvpt->maxstc  = 0;    /* Input FIeld Max Column Init Zero.    */
        kjsvpt->maxstr  = 0;    /* Input Field Max Row Init Zero.       */
        kjsvpt->kanalen = 0;    /* Kana Data Length Init Zero.          */
        kjsvpt->kkmode1 = 0;    /* Conversion Mode Initial Mode.        */
        kjsvpt->kkmode2 = 0;    /* Conversion Mode Initial Mode.        */
        kjsvpt->rkclen  = 0;    /* Romaji Kana Conversion Length Init.  */
        kjsvpt->regymlen= 0;    /* Registration Yomi Length Init.       */
        kjsvpt->savelen = 0;    /* Background String Save Length Init.  */
        kjsvpt->cconvpos= 0;    /* Current Conversion Posotion Init.    */
        kjsvpt->cconvlen= 0;    /* Current Conversion Length Init.      */
        kjsvpt->convpos = 0;    /* Conversion Posotion Init.            */
        kjsvpt->realcol = 0;    /* Input Field Real Acailavle Length.   */
        kjsvpt->savepos = 0;    /* Background String Save Start Pos Init*/
        kjsvpt->curleft = 0;    /* Cursor Left Move Limit Init.         */
        kjsvpt->curright= 0;    /* Cursor Right Move Limit Init.        */
        kjsvpt->chcodlen= 0;    /* Change Conversion Length Init.       */
        kjsvpt->kkcrc   = 0;    /* KKC Return Code Init.                */
        kjsvpt->convlen = 0;    /* Conversion Lenght Init.              */
        (void)memset( (char *)kjsvpt->allcstge
                      ,'\0',sizeof(kjsvpt->allcstge));
        (void)memset((char *)kjsvpt->allcstgs
                      ,'\0',sizeof(kjsvpt->allcstgs));
        kjsvpt->alcancol= 0;    /* All Candidate Column Init.           */
        kjsvpt->alcanrow= 0;    /* All Candidate Row Init.              */
        kjsvpt->kmact   = 0;    /* Input Field Active Not Active.       */
        kjsvpt->hkmode  = 0;    /* Hiragana/Katakana Conversion Stage   */
                                /* Undefine.                            */
        kjsvpt->kanamap[0]= 1;  /* Kanamap Initial Length Set.          */
        kjsvpt->kkcrmode= 0;    /* None Registration Digtinoary Mode.   */
        kjsvpt->pscode  = 0;    /* Pseude Code Init.                    */
        kjsvpt->chmode  = 0;    /* Character Class Code Undefine.       */
        kjsvpt->msetflg = 0;    /* Active Message None.                 */
        kjsvpt->auxflg1 = 0;    /* Auxiliary Area 1 Message Not Use.    */
        kjsvpt->auxflg2 = 0;    /* Auxiliary Area 2 Message Not Use.    */
        kjsvpt->auxflg3 = 0;    /* Auxiliary Area 3 Message Not Use.    */
        kjsvpt->auxflg4 = 0;    /* Auxiliary Area 4 Message Not Use.    */
        kjsvpt->actc1   = 0;    /* Action Code 1 Not Active.            */
        kjsvpt->actc2   = 0;    /* Action Code 2 Not Active.            */
        kjsvpt->actc3   = 0;    /* Action Code 3 Not Active.            */
        kjsvpt->convimp = 0;    /* Conversion Impossible.               */
        kjsvpt->charcont= 0;    /* RKC Hiragana(KyaKyuko) Not Available.*/
        kjsvpt->convnum = 0;    /* Conversion Successful Number.        */
        kjsvpt->allcanfg= 0;    /* Not All Candidate.                   */
        kjsvpt->tankan  = 0;    /* Not Tankan Mode.                     */
        kjsvpt->knjnumfg= 0;    /* Message Output Area.                 */



/* #(B) 1987.12.08. Flying Conversion Add */
        kjsvpt->ax1lastc = 0;/* Auxiliary Area No.1 Last Character Position.*/
        kjsvpt->ax2lastc = 0;/* Auxiliary Area No.2 Last Character Position.*/
        kjsvpt->ax3lastc = 0;/* Auxiliary Area No.3 Last Character Position.*/
        kjsvpt->ax4lastc = 0;/* Auxiliary Area No.4 Last Character Position.*/
        kjsvpt->alcnmdfg = 0;/* All Candidates Mode Flag.                   */
        kjsvpt->fcnverfg = 0;/* Flying Conversion Error Flag.               */
        kjsvpt->fconvflg = 0;/* Flying Conversion Flag.                     */
        kjsvpt->fconvpos = 0;/* Flying Conversion Start Position.           */
/* #(E) 1987.12.08. Flying Conversion Add */
/*======================================================================*/
/* #(B) Sept. 21 1988 S,Higuchi                                         */
/*      Added source.                                                   */
/*              kjsvpt->fcvovfg = C_SWOFF;                              */
/*======================================================================*/
        kjsvpt->fcvovfg = C_SWOFF;

        /*
         ****************************************************************
         *      1-3. 6. Return to Caller.
         ****************************************************************
         */
        return;
}

/*
 *      EXT information sest KCB,KMISA
 */
static void _Jop14( ext,pt,kmpf )

register EXT     *ext;   /* pointer to EXT.                     */
register KCB     *pt;    /* pointer to KCB.                     */
register KMPF    *kmpf;  /* KMPF initialize values.             */

{
        /*
         ****************************************************************
         *      1-4. 0. Set Up General Work Pointer.
         ****************************************************************
         */
        /*
         *      Pointer to KMISA.
         */
        register KMISA   *kjsvpt = (KMISA *)pt->kjsvpt;

        /*
         ****************************************************************
         *      1-4. 1. Set Input Field Max Column Number.
         ****************************************************************
         */
        /*
         *      EXT Max Column Number sets KMISA Max Column Number.
         */
        kjsvpt->maxstc   = ext->maxstc;

        /*
         ****************************************************************
         *      1-4. 2. Set Input Field Max Row Number.
         ****************************************************************
         */
        /*
         *      EXT Max Row Number sets KMISA Max Row Number.
         */
        kjsvpt->maxstr   = ext->maxstr;

        /*
         ****************************************************************
         *      1-4. 3. Get Auxiliary Area No.1 Column Number,
         ****************************************************************
         */
        /*
         *      smaller values sets KCB Auxiliary Area No.1 Max Column.
         *      which is EXT Auxiliary Area No.1 MAX Columns or KMPF
         *      Intitialize Auxiliary Area No.1 Max Columns
         */
        pt->maxa1c = MIN( ext->maxa1c , kmpf[0].aux1maxc );

        /*
         ****************************************************************
         *      1-4. 4. Get Auxiliary Area No.1 Row Number
         ****************************************************************
         */
        /*
         *      smaller values sets sets KCB Auxiliary Area No.1 Max
         *      Rows.
         *      which is EXT Auxiliary No.1 Max Rows or KMPF
         *      Intialize Max Rows.
         */
        pt->maxa1r = MIN( ext->maxa1r ,kmpf[0].aux1maxr );

        /*
         ****************************************************************
         *      1-4. 5. Analize Auxiliary Area No.1 Not Available Case,
         *      and Unconditional Data Clear.
         ****************************************************************
         */
        /*
         *      if auxiliary area No Use Case?
         */
        if( MIN(pt->maxa1c,pt->maxa1r)<=0 ) {
                /*
                 *      Auxiliary Area Not Use.
                 */
                pt->maxa1c = 0; /* Auxiliary Area Column Size.          */
                pt->maxa1r = 0; /* Auxiliary Area Row    Size.          */
        };

        /*
         ****************************************************************
         *      1-4. 6. Return to Caller.
         ****************************************************************
         */
        return;
}

/************************************************************************/
/*#(B)  Bug Fix. Mon Nov 18,1987                                        */
/*      Modify Reazon.                                                  */
/*              See Change Actity.                                      */
/*      Add Source Code.                                                */
/*              Function _Jop15().                                      */
/************************************************************************/
/*
 *      Ascii Numeric Code Convert to Long Integer.
 *      If Invalid Numeric Code or Overflow Long Integer then
 *      Return Value IMNTNUME,Otherwise IMSUCC.
 */
static int  _Jop15( ascii,num,min_num,max_num )
char    *ascii;
long    *num;
long    min_num;
long    max_num;
{
        long    sign;                   /* Sign Code.                   */
        long    pre_num;                /* Save Previous Number.        */
        int     ret_code;               /* Return Code.                 */


        sign     = 1;                   /* Default Sign is Plus.        */
        *num     = 0;                   /* Set Initial Value.           */
        pre_num  = *num;                /* Set Previous Number.         */

        if( *ascii ) {
                ret_code = IMSUCC;      /* Set Default Return Code      */
        } else {                        /* is Successful.               */
                ret_code = IMNTNUME;    /* Set Default Return Code.     */
        };                              /* Invalid Numeric Code.        */

        while( isspace( *ascii ) )      /* Skip Space String.           */
                ascii++;                /* Space or Tab or CR or LF     */
                                        /* or FF.                       */

        /* Get Sign Value.              */
        while( *ascii=='+' || *ascii == '-') {
                switch( *ascii++) {
                case '+':               /* Plus Sign.                   */
                        sign = 1;
                        break;
                case '-':               /* Minus Sign.                  */
                        sign = -1;
                        break;
                };
        };

        /* Get Numeric Code.            */
        while( isdigit( *ascii ) ) {
                /* Convert Decimal Number.      */
                /* 10 is Decimal Multiplier.    */
                *num = *num * 10 + ((*ascii++) - '0');

                /* Check Overflow.              */
                if( *num < pre_num ) {
                        ret_code = IMNTNUME;    /* Overflow Error.      */
                        break;
                };
                ret_code = IMSUCC;
        };

        /* Sign Set.                    */
        *num *= sign;

        /* Check Numeric Range.         */
        if( (*num < min_num) || (*num >=max_num ) )
                ret_code = IMNTNUME;    /* Numeric Range Error.         */

        /*      Return to Caller.       */
        return( ret_code );
}

/*
 *	return KKCB pointer
 */
KKCB	*jmntGetKKCB(KCB *kp)
{
	return kp->kjsvpt->kkcbsvpt;
}
