static char sccsid[] = "@(#)58	1.2  src/bos/usr/lib/nls/loc/ZH.im/zhedinit.c, ils-zh_CN, bos41J, 9509A_all 2/25/95 13:38:55";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: ZHedFreeDictionaryName
 *		ZHedInit
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATION ***************************/
/*                                                                            */
/* MODULE NAME:         ZHedInit                                              */
/*                                                                            */
/* DESCRIPTIVE NAME:    TIMFEP TIMED Interface                                */
/*                                                                            */
/* FUNCTION:            ZHedinit : Initial Internal Used Data Structure.      */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/

/*----------------------------------------------------------------------------*/
/*                      include files                                         */
/*----------------------------------------------------------------------------*/

#include "zhed.h"
#include "zhedacc.h"
#include "zhedinit.h"
#include "zhedud.h"


/* extern Ud_Switch     ud_switch;      */
extern UdCBptr;
FEPCB   *fep;

/*----------------------------------------------------------------------------*/
/*                      external reference                                    */
/*----------------------------------------------------------------------------*/

extern  int     TjLoadSysFileMI();
extern  int     PyLoadSysFileMI();
extern  int     TjLoadUsrFileMI();
extern  int     PyLoadUsrFileMI();

extern         UdInitialCB();
extern         UdOtherInitialCB();
extern         UdEraseCurRadicalCB();
extern         UdEraseAllRadicalCB();
extern         UdRadicalInputCB();
extern         BackSpaceCB();
extern         NonConvertCB();
extern         ErrorBeepCB();
extern         UdListBoxCB();
extern         UdGetCandCB();
extern         UdCloseAuxCB();
extern         UdFreeCandCB();
extern         CursorCB();
extern         NoWordCB();
extern         UdShowCursorCB();
extern         halffullCB();

/*----------------------------------------------------------------------------*/
/*                      Beginning of procedure                                */
/*----------------------------------------------------------------------------*/

int  ZHedInit( echobufs, echobufa, edendbuf, echosize, auxbufs, auxbufa,
              auxsize, auxformat, profile_str)
     unsigned char   *echobufs ;      /* echo buffer address                  */
     unsigned char   *echobufa ;      /* echo attribute buffer address        */
     unsigned char   *edendbuf ;      /* edit end buffer address              */
     int             echosize  ;      /* echo/edit end buffer sizes in byte   */
     unsigned char   **auxbufs ;      /* pointer to aux buf addresses         */
     unsigned char   **auxbufa ;      /* pointer to aux att buf addresses     */
     AuxSize         *auxsize  ;      /* pointer to max aux buffer size       */
     int             auxformat ;      /* aux format                           */
     ZHedProfile     *profile_str;    /* pointer to profile structure         */
{
        FEPCB           *fepcb ;

        /*********************************************/
        /*  checks if the arguments are acceptable.  */
        /*********************************************/

        if( echosize < MIN_ECHOSIZE )
        {
            return(ERROR);
        }

        if( auxformat == LONG_AUX_FMT )
        {
           if((auxsize->itemsize<AUXROWMAX)||(auxsize->itemnum<AUXCOLMAX))
                    return(ERROR);
        }
        else  /* aux of short format */
        {
           if((auxsize->itemsize < MIN_AUX) || (auxsize->itemnum < 1))
                    return(ERROR);
        }


        if( !( fepcb = (FEPCB*)malloc( sizeof(FEPCB) ) ) )
              return(ERROR);

        /*********************************/
        /*  initializes values in FEPCB  */
        /*********************************/

        fepcb->echobufs = echobufs ;
        fepcb->echobufa = echobufa ;
        fepcb->edendbuf = edendbuf ;
        fepcb->auxbufs  = auxbufs ;
        fepcb->auxbufa  = auxbufa ;
        fepcb->echosize = echosize ;
        fepcb->echoover = echosize ;
        fepcb->auxsize.itemsize = auxsize->itemsize;
        fepcb->auxsize.itemnum  = auxsize->itemnum;
        fepcb->echochfg.flag = OFF;
        fepcb->echochfg.chtoppos = 0;
        fepcb->echochfg.chlenbytes = 0;
        fepcb->echocrps = 0 ;
        fepcb->eccrpsch = OFF ;
        fepcb->echoacsz = 0 ;
        fepcb->auxchfg  = OFF ;
        fepcb->auxuse   = NOTUSE ;
        fepcb->auxcrps.colpos = -1;
        fepcb->auxcrps.rowpos = -1;
        fepcb->auxcrpsch = OFF ;
        fepcb->auxacsz.itemsize = 0;
        fepcb->auxacsz.itemnum = 0;
        fepcb->indchfg  = OFF ;
        fepcb->isbeep = OFF;
        fepcb->beepallowed = profile_str->initbeep;
        fepcb->imode.ind0 = profile_str->initchar; /* set indicator value */
        fepcb->imode.ind1 = profile_str->initsize;
        fepcb->imode.ind2 = SELECT_OFF;
        fepcb->imode.ind3 = NORMAL_MODE;
        fepcb->imode.ind4 = REPLACE_MODE;
        fepcb->imode.ind5 = BLANK;
        fepcb->imode.ind6 = INPUT_NO_SELECTED;

        /* Set Legend ON/OFF   */
        if(profile_str->dictname.Le_sys_dict||profile_str->dictname.Le_usr_dict)
            fepcb->imode.ind7 = LEGEND_ON;
        else
            fepcb->imode.ind7 = LEGEND_OFF;

        /* Allocate memory for input method buffer    */
        fepcb->rcinbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->pyinbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->tjinbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->eninbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->abcinbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->udinbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->ret = 0;

        /* Initial creating time for input method dictionary file   */
        memset(fepcb->ctime.pytime,NULL,30);
        memset(fepcb->ctime.tjtime,NULL,30);
        memset(fepcb->ctime.letime,NULL,30);
        memset(fepcb->ctime.entime,NULL,30);
        memset(fepcb->ctime.abctime,NULL,30);
        memset(fepcb->ctime.udtime,NULL,30);
        fepcb->mi.abcsysmi = NULL;
        fepcb->fd.pysysfd[COMM] = NULL;
        fepcb->fd.pysysfd[GB] = NULL;
        fepcb->fd.pysysfd[CNS] = NULL;
        fepcb->fd.pysysfd[JK] = NULL;
        fepcb->fd.pyusrfd = NULL;
        fepcb->fd.tjsysfd = NULL;
        fepcb->fd.tjusrfd = NULL;
        fepcb->fd.lesysfd = NULL;
        fepcb->fd.leusrfd = NULL;
        fepcb->fd.ensysfd = NULL;
        fepcb->fd.enusrfd = NULL;
        fepcb->fd.abcsysfd[ABCCWD_S] = NULL;
        fepcb->fd.abcsysfd[ABCCWD_T] = NULL;
        fepcb->fd.abcsysfd[ABCOVL] = NULL;
        fepcb->fd.abcusrfd[USRREM] = 0;
        fepcb->fd.abcusrfd[USR] = 0;

        fepcb->pystruct.cand = NULL;
        fepcb->pystruct.curptr = NULL;
        fepcb->pystruct.allcandno = 0;
        fepcb->pystruct.more = 0;

        fepcb->tjstruct.cand = NULL;
        fepcb->tjstruct.curptr = NULL;
        fepcb->tjstruct.allcandno = 0;
        fepcb->tjstruct.more = 0;

        fepcb->lestruct.cand = NULL;
        fepcb->lestruct.curptr = NULL;
        fepcb->lestruct.allcandno = 0;
        fepcb->lestruct.more = 0;

        fepcb->enstruct.cand = NULL;
        fepcb->enstruct.curptr = NULL;
        fepcb->enstruct.allcandno = 0;
        fepcb->enstruct.more = 0;

        fepcb->abcstruct.cand = NULL;
        fepcb->abcstruct.curptr = NULL;
        fepcb->abcstruct.allcandno = 0;
        fepcb->abcstruct.more = 0;

        fepcb->udstruct.cand = NULL;
        fepcb->udstruct.curptr = NULL;
        fepcb->udstruct.allcandno = 0;
        fepcb->udstruct.more = 0;

        fepcb->edendacsz = 0;   /* makeoutputbuffer() use it     */
        fepcb->inputlen = 0;    /* for IC the 1st key press  */
        fepcb->fname.pysysfname[COMM] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.pysysfname[GB] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.pysysfname[CNS] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.pysysfname[JK] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.pyusrfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.tjsysfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.tjusrfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.ensysfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.enusrfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.abcsysfname[ABCCWD_S] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.abcsysfname[ABCCWD_T] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.abcsysfname[ABCOVL] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.abcusrfname[USRREM] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.abcusrfname[USR] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.udfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        strcpy(fepcb->fname.pysysfname[COMM],profile_str->dictname.Py_sys_dict_comm);
        strcpy(fepcb->fname.pysysfname[GB],profile_str->dictname.Py_sys_dict_gb);
        strcpy(fepcb->fname.pysysfname[CNS],profile_str->dictname.Py_sys_dict_cns);
        strcpy(fepcb->fname.pysysfname[JK],profile_str->dictname.Py_sys_dict_jk);
        strcpy(fepcb->fname.pyusrfname,profile_str->dictname.Py_usr_dict);
        strcpy(fepcb->fname.tjsysfname,profile_str->dictname.Tj_sys_dict);
        strcpy(fepcb->fname.tjusrfname,profile_str->dictname.Tj_usr_dict);
        strcpy(fepcb->fname.ensysfname,profile_str->dictname.En_sys_dict);
        strcpy(fepcb->fname.enusrfname,profile_str->dictname.En_usr_dict);
        strcpy(fepcb->fname.abcsysfname[ABCCWD_S],profile_str->dictname.Abc_sys_dict_cwd_s);
        strcpy(fepcb->fname.abcsysfname[ABCCWD_T],profile_str->dictname.Abc_sys_dict_cwd_t);
        strcpy(fepcb->fname.abcsysfname[ABCOVL],profile_str->dictname.Abc_sys_dict_ovl);
        strcpy(fepcb->fname.abcusrfname[USRREM],profile_str->dictname.Abc_usr_rem);
        strcpy(fepcb->fname.abcusrfname[USR],profile_str->dictname.Abc_usr_dict);
        strcpy(fepcb->fname.udfname,profile_str->dictname.Ud_dict);

        fep = fepcb;
        udimcomm = (UdimCommon*)malloc(sizeof(UdimCommon));
        udimcomm->udcb.UdInitialCB = UdInitialCB;
        udimcomm->udcb.UdOtherInitialCB = UdOtherInitialCB;
        udimcomm->udcb.UdEraseCurRadicalCB = UdEraseCurRadicalCB;
        udimcomm->udcb.UdEraseAllRadicalCB = UdEraseAllRadicalCB;
        udimcomm->udcb.UdRadicalInputCB = UdRadicalInputCB;
        udimcomm->udcb.BackSpaceCB = BackSpaceCB;
        udimcomm->udcb.NonConvertCB = NonConvertCB;
        udimcomm->udcb.ErrorBeepCB = ErrorBeepCB;
        udimcomm->udcb.UdListBoxCB = UdListBoxCB;
        udimcomm->udcb.UdGetCandCB = UdGetCandCB;
        udimcomm->udcb.UdCloseAuxCB = UdCloseAuxCB;
        udimcomm->udcb.UdFreeCandCB = UdFreeCandCB;
        udimcomm->udcb.CursorCB = CursorCB;
        udimcomm->udcb.NoWordCB = NoWordCB;
        udimcomm->udcb.UdShowCursorCB= UdShowCursorCB;
        udimcomm->udcb.halffullCB= halffullCB;
        udimcomm->FileName = fepcb->fname.udfname;   

        fepcb->fname.lesysfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        if (profile_str->dictname.Le_sys_dict) 
            strcpy(fepcb->fname.lesysfname,profile_str->dictname.Le_sys_dict);
        fepcb->fname.leusrfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        if (profile_str->dictname.Le_usr_dict)
            strcpy(fepcb->fname.leusrfname,profile_str->dictname.Le_usr_dict);

  /************************************************************************/
  /*    The User Defined Input Method Module don't provide temporarily    */
  /************************************************************************/
        if ( profile_str->ud_im_func ) 
        {
            udimcomm = (UdimCommon*)malloc(sizeof(UdimCommon));
            udimcomm->FileName = fepcb->fname.udfname;   
            udimcomm->UserDefinedInit = profile_str->ud_im_func;
            if((*udimcomm->UserDefinedInit)(udimcomm))
               udimcomm->ret = UD_FOUND;
            else
               udimcomm->ret = UD_NOT_FOUND;
            
        }
        /* load English-Chinese system file  */
        if(EnLoadSysFileMI(fepcb) == ERROR )
        {   
            free(fepcb->pyinbuf);
            free(fepcb->tjinbuf);
            free(fepcb->eninbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            ZHedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }   

        /* load Pinyin system file              */
        if(PyLoadSysFileMI(fepcb) == ERROR )
        {      
            free(fepcb->pyinbuf);
            free(fepcb->tjinbuf);
            free(fepcb->eninbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            ZHedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }

        /* load Tsang_Jye system file              */
        if(TjLoadSysFileMI(fepcb) == ERROR )
        {      
            free(fepcb->pyinbuf);
            free(fepcb->tjinbuf);
            free(fepcb->eninbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            ZHedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }

       AbcUpdateDict(fepcb->fname.abcusrfname[USRREM], fepcb->fname.abcusrfname[USR], 0);

       /* load ABC system file                    */
        if(AbcLoadSysFileMI(fepcb) == ERROR )
        { 
            free(fepcb->pyinbuf);
            free(fepcb->tjinbuf);
            free(fepcb->eninbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            ZHedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }

        LeLoadSysFileMI(fepcb);       /* load Pinyin Legend system file       */

        LeLoadUsrFileMI(fepcb);       /* load Pinyin Legend user file         */
        EnLoadUsrFileMI(fepcb);       /* load English-Chinese user file       */
        PyLoadUsrFileMI(fepcb);       /* load Pinyin user file                */
        TjLoadUsrFileMI(fepcb);       /* load Tsang_Jye user file             */
        AbcLoadUsrFile(fepcb);        /* load ABC user file                   */
        return ((int)fepcb) ;
}                                     /* end of ZHedInit                      */

ZHedFreeDictionaryName(fepcb)
FEPCB *fepcb;
{
     free(fepcb->fname.pysysfname[COMM]);
     free(fepcb->fname.pysysfname[GB]);
     free(fepcb->fname.pysysfname[CNS]);
     free(fepcb->fname.pysysfname[JK]);
     free(fepcb->fname.tjsysfname);
     free(fepcb->fname.lesysfname);
     free(fepcb->fname.ensysfname);
     free(fepcb->fname.abcsysfname[ABCCWD_S]);
     free(fepcb->fname.abcsysfname[ABCCWD_T]);
     free(fepcb->fname.abcsysfname[ABCOVL]);
     free(fepcb->fname.pyusrfname);
     free(fepcb->fname.tjusrfname);
     free(fepcb->fname.leusrfname);
     free(fepcb->fname.enusrfname);
     free(fepcb->fname.abcusrfname[USRREM]);
     free(fepcb->fname.abcusrfname[USR]);
     free(fepcb->fname.udfname); 
}


