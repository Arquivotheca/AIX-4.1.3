static char sccsid[] = "@(#)13	1.2  src/bos/usr/lib/nls/loc/CN.im/cnedinit.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:24";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CNedFreeDictionaryName
 *		CNedInit
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
/* MODULE NAME:         CNedInit                                              */
/*                                                                            */
/* DESCRIPTIVE NAME:    TIMFEP TIMED Interface                                */
/*                                                                            */
/* FUNCTION:            CNedinit : Initial Internal Used Data Structure.      */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/

/*----------------------------------------------------------------------------*/
/*                      include files                                         */
/*----------------------------------------------------------------------------*/

#include "cned.h"
#include "cnedacc.h"
#include "cnedinit.h"

#include "cnedud.h"
/*----------------------------------------------------------------------------*/
/*                      external reference                                    */
/*----------------------------------------------------------------------------*/

extern  int     PhLoadSysFileMI();
extern  int     PhLoadUsrFileMI();

/*----------------------------------------------------------------------------*/
/*                      Beginning of procedure                                */
/*----------------------------------------------------------------------------*/

int  CNedInit( echobufs, echobufa, edendbuf, echosize, auxbufs, auxbufa,
              auxsize, auxformat, profile_str)
     unsigned char   *echobufs ;      /* echo buffer address                  */
     unsigned char   *echobufa ;      /* echo attribute buffer address        */
     unsigned char   *edendbuf ;      /* edit end buffer address              */
     int             echosize  ;      /* echo/edit end buffer sizes in byte   */
     unsigned char   **auxbufs ;      /* pointer to aux buf addresses         */
     unsigned char   **auxbufa ;      /* pointer to aux att buf addresses     */
     AuxSize         *auxsize  ;      /* pointer to max aux buffer size       */
     int             auxformat ;      /* aux format                           */
     CNedProfile     *profile_str;    /* pointer to profile structure         */
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
        fepcb->flag = OFF;
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
        fepcb->eninbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->fssinbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->fsinbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->abcinbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->udinbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->ret = 0;

        /* Initial creating time for input method dictionary file   */
        memset(fepcb->ctime.pytime,NULL,30);
        memset(fepcb->ctime.letime,NULL,30);
        memset(fepcb->ctime.entime,NULL,30);
        memset(fepcb->ctime.fsstime,NULL,30);
        memset(fepcb->ctime.fstime,NULL,30);
        memset(fepcb->ctime.abctime,NULL,30);
        memset(fepcb->ctime.udtime,NULL,30);
        fepcb->mi.abcsysmi = NULL;
        fepcb->fd.pysysfd = NULL;
        fepcb->fd.pyusrfd = NULL;
        fepcb->fd.lesysfd = NULL;
        fepcb->fd.leusrfd = NULL;
        fepcb->fd.ensysfd = NULL;
        fepcb->fd.enusrfd = NULL;
        fepcb->fd.fsssysfd = NULL;
        fepcb->fd.fssusrfd = NULL;
        fepcb->fd.fssjmsysfd = NULL;
        fepcb->fd.fssysfd = NULL;
        fepcb->fd.fsusrfd = NULL;
        fepcb->fd.fsphsysfd = NULL;
        fepcb->fd.fsphusrfd = NULL;
        fepcb->fd.abcsysfd[ABCCWD] = NULL;
        fepcb->fd.abcsysfd[ABCOVL] = NULL;
        fepcb->fd.abcusrfd[USRREM] = NULL;
        fepcb->fd.abcusrfd[USR] = NULL;

        fepcb->pystruct.cand = NULL;
        fepcb->pystruct.curptr = NULL;
        fepcb->pystruct.allcandno = 0;
        fepcb->pystruct.more = 0;

        fepcb->lestruct.cand = NULL;
        fepcb->lestruct.curptr = NULL;
        fepcb->lestruct.allcandno = 0;
        fepcb->lestruct.more = 0;

        fepcb->enstruct.cand = NULL;
        fepcb->enstruct.curptr = NULL;
        fepcb->enstruct.allcandno = 0;
        fepcb->enstruct.more = 0;

        fepcb->fssstruct.cand = NULL;
        fepcb->fssstruct.curptr = NULL;
        fepcb->fssstruct.allcandno = 0;
        fepcb->fssstruct.more = 0;

        fepcb->fsstruct.cand = NULL;
        fepcb->fsstruct.curptr = NULL;
        fepcb->fsstruct.allcandno = 0;
        fepcb->fsstruct.more = 0;

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
        fepcb->fname.pysysfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.pyusrfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.ensysfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.enusrfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.fsssysfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.fssusrfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.fssjmsysfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.fssysfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.fsphsysfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.fsusrfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.fsphusrfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.abcsysfname[ABCCWD] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.abcsysfname[ABCOVL] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.abcusrfname[USRREM] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.abcusrfname[USR] = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.udfname = (unsigned char*)malloc(sizeof(char)*DICTNAME_LEN);
        strcpy(fepcb->fname.pysysfname,profile_str->dictname.Py_sys_dict);
        strcpy(fepcb->fname.pyusrfname,profile_str->dictname.Py_usr_dict);
        strcpy(fepcb->fname.ensysfname,profile_str->dictname.En_sys_dict);
        strcpy(fepcb->fname.enusrfname,profile_str->dictname.En_usr_dict);
        strcpy(fepcb->fname.fsssysfname,profile_str->dictname.Fss_sys_dict);
        strcpy(fepcb->fname.fssusrfname,profile_str->dictname.Fss_usr_dict);
        strcpy(fepcb->fname.fssjmsysfname,profile_str->dictname.Fssjm_sys_dict);
        strcpy(fepcb->fname.fssysfname,profile_str->dictname.Fs_sys_dict);
        strcpy(fepcb->fname.fsusrfname,profile_str->dictname.Fs_usr_dict);
        strcpy(fepcb->fname.fsphsysfname,profile_str->dictname.Fsph_sys_dict);
        strcpy(fepcb->fname.fsphusrfname,profile_str->dictname.Fsph_usr_dict);
        strcpy(fepcb->fname.abcsysfname[ABCCWD],profile_str->dictname.Abc_sys_dict_cwd);
        strcpy(fepcb->fname.abcsysfname[ABCOVL],profile_str->dictname.Abc_sys_dict_ovl);
        strcpy(fepcb->fname.abcusrfname[USRREM],profile_str->dictname.Abc_usr_rem);
        strcpy(fepcb->fname.abcusrfname[USR],profile_str->dictname.Abc_usr_dict);
        strcpy(fepcb->fname.udfname,profile_str->dictname.Ud_dict);

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
            udimcomm->UserDefinedInitial = profile_str->ud_im_func;
            if((*udimcomm->UserDefinedInitial)(udimcomm))
            {
               udimcomm->ret = UD_FOUND;
            }
            else
               udimcomm->ret = UD_NOT_FOUND;
        }

        /* load English-Chinese system file     */
        if(EnLoadSysFileMI(fepcb) == ERROR )
        {   
            free(fepcb->pyinbuf);
            free(fepcb->eninbuf);
            free(fepcb->fssinbuf);
            free(fepcb->fsinbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            CNedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }

        /* load Pinyin system file              */
        if(PyLoadSysFileMI(fepcb) == ERROR )
        {      
            free(fepcb->pyinbuf);
            free(fepcb->eninbuf);
            free(fepcb->fssinbuf);
            free(fepcb->fsinbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            CNedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }

        /* load Five Stroke Style system file              */
        if(FssLoadSysFileMI(fepcb) == ERROR )
        {      
            free(fepcb->pyinbuf);
            free(fepcb->eninbuf);
            free(fepcb->fssinbuf);
            free(fepcb->fsinbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            CNedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }

        /* load Five Stroke Simple Style system file              */
        if(FssJmLoadSysFileMI(fepcb) == ERROR )
        {      
            free(fepcb->pyinbuf);
            free(fepcb->eninbuf);
            free(fepcb->fssinbuf);
            free(fepcb->fsinbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            CNedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }

        /* load Five Stroke system file              */
        if(FsLoadSysFileMI(fepcb) == ERROR )
        {      
            free(fepcb->pyinbuf);
            free(fepcb->eninbuf);
            free(fepcb->fssinbuf);
            free(fepcb->fsinbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            CNedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }

        /* load Five Stroke system file              */
        if(FsPhLoadSysFileMI(fepcb) == ERROR )
        {      
            free(fepcb->pyinbuf);
            free(fepcb->eninbuf);
            free(fepcb->fssinbuf);
            free(fepcb->fsinbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            CNedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }

       AbcUpdateDict(fepcb->fname.abcusrfname[USRREM], fepcb->fname.abcusrfname[USR], 0);
       /* load ABC system file                 */
        if(AbcLoadSysFileMI(fepcb) == ERROR )
        { 
            free(fepcb->pyinbuf);
            free(fepcb->eninbuf);
            free(fepcb->fssinbuf);
            free(fepcb->fsinbuf);
            free(fepcb->abcinbuf);
            free(fepcb->udinbuf);
            CNedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }

        LeLoadSysFileMI(fepcb);       /* load Pinyin Legend system file       */

        EnLoadUsrFileMI(fepcb);       /* load English-Chinese user file       */
        PyLoadUsrFileMI(fepcb);       /* load Pinyin user file                */
        LeLoadUsrFileMI(fepcb);       /* load Pinyin Legend user file         */
        FssLoadUsrFileMI(fepcb);      /* load Five Stroke Style user file     */
        FsLoadUsrFileMI(fepcb);      /* load Five Stroke Style user file     */
        FsPhLoadUsrFileMI(fepcb);    /* load Five Stroke Style user file     */
        AbcLoadUsrFile(fepcb);        /* load ABC user file                   */
        return ((int)fepcb) ;
}                                     /* end of CNedInit                      */

CNedFreeDictionaryName(fepcb)
FEPCB *fepcb;
{
     free(fepcb->fname.pysysfname);
     free(fepcb->fname.lesysfname);
     free(fepcb->fname.ensysfname);
     free(fepcb->fname.fsssysfname);
     free(fepcb->fname.fssjmsysfname);
     free(fepcb->fname.fssysfname);
     free(fepcb->fname.fsphsysfname);
     free(fepcb->fname.abcsysfname[ABCCWD]);
     free(fepcb->fname.abcsysfname[ABCOVL]);
     free(fepcb->fname.pyusrfname);
     free(fepcb->fname.leusrfname);
     free(fepcb->fname.enusrfname);
     free(fepcb->fname.fssusrfname);
     free(fepcb->fname.fsusrfname);
     free(fepcb->fname.fsphusrfname);
     free(fepcb->fname.abcusrfname[USRREM]);
     free(fepcb->fname.abcusrfname[USR]);
/*     free(fepcb->fname.udfname); */
}


