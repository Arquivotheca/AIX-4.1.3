static char sccsid[] = "@(#)10  1.4  src/bos/usr/lib/nls/loc/imt/tfep/tedinit.c, libtw, bos411, 9428A410j 4/21/94 01:59:47";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: TedFreeDictionaryName
 *              TedInit
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************** START OF MODULE SPECIFICATION ***************************/
/*                                                                            */
/* MODULE NAME:         TedInit                                               */
/*                                                                            */
/* DESCRIPTIVE NAME:    TIMFEP TIMED Interface                                */
/*                                                                            */
/* FUNCTION:            Tedinit : Initial Internal Used Data Structure.       */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/* COMPILER:            AIX C                                                 */
/*                                                                            */
/* AUTHOR:              Andrew Wu                                             */
/*                                                                            */
/* STATUS:              Chinese Input Method Version 1.0                      */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/

/*----------------------------------------------------------------------------*/
/*                      include files                                         */
/*----------------------------------------------------------------------------*/

#include "ted.h"
#include "tedacc.h"
#include "tedinit.h"

/*----------------------------------------------------------------------------*/
/*                      external reference                                    */
/*----------------------------------------------------------------------------*/

extern  int     TjLoadSysFileMI();
extern  int     PhLoadSysFileMI();
extern  int     TjLoadUsrFileMI();
extern  int     PhLoadUsrFileMI();

/*----------------------------------------------------------------------------*/
/*                      Beginning of procedure                                */
/*----------------------------------------------------------------------------*/

int  TedInit( echobufs, echobufa, edendbuf, echosize, auxbufs, auxbufa,
              auxsize, auxformat, profile_str)
     unsigned char   *echobufs ;    /* echo buffer address                */
     unsigned char   *echobufa ;    /* echo attribute buffer address      */
     unsigned char   *edendbuf ;    /* edit end buffer address            */
     int             echosize  ;    /* echo/edit end buffer sizes in byte */
     unsigned char   **auxbufs ;    /* pointer to aux buf addresses       */
     unsigned char   **auxbufa ;    /* pointer to aux att buf addresses   */
     AuxSize         *auxsize  ;    /* pointer to max aux buffer size     */
     int             auxformat ;    /* aux format                         */
     TedProfile      *profile_str;  /* pointer to profile structure       */
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
        fepcb->starpos = 0;           /* allocate space for pointer */
        fepcb->preinpbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->curinpbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->preedbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->ctrl_r_buf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->radeucbuf=(unsigned char *)calloc(fepcb->echosize,sizeof(char));
        fepcb->ret = 0;
        memset(fepcb->ctime.phtime,NULL,30);
        memset(fepcb->ctime.tjtime,NULL,30);
        fepcb->mi.phsysmi = NULL;
        fepcb->mi.phusrmi = NULL;
        fepcb->fd.phsysfd = NULL;
        fepcb->fd.phusrfd = NULL;
        fepcb->fd.tjsysfd = NULL;
        fepcb->fd.tjusrfd = NULL;
        fepcb->stjstruct.stjcand = NULL;
        fepcb->stjstruct.allcandno = 0;
        fepcb->stjstruct.curcandno = 0;
        fepcb->stjstruct.headcandno = 0;
        fepcb->stjstruct.tailcandno = 0;
        fepcb->strokestruct.strokecand = NULL;
        fepcb->strokestruct.allcandno = 0;
        fepcb->strokestruct.curcandno = 0;
        fepcb->strokestruct.headcandno = 0;
        fepcb->strokestruct.tailcandno = 0;
        fepcb->phstruct.phcand = NULL;
        fepcb->phstruct.curptr = NULL;
        fepcb->phstruct.allcandno = 0;
        fepcb->phstruct.more = 0;
        fepcb->edendacsz = 0;   /* KC Lee/Jim R. added => makeoutputbuffer() use it     */
        fepcb->inputlen = 0;    /* Jim Roung added -> for IC the 1st key press  */
        fepcb->fname.phsysfname = malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.phusrfname = malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.tjsysfname = malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.tjusrfname = malloc(sizeof(char)*DICTNAME_LEN);
        fepcb->fname.learnname = malloc(sizeof(char)*DICTNAME_LEN); /* big5 */
        strcpy(fepcb->fname.phsysfname,profile_str->dictname.PH_sys_dict);
        strcpy(fepcb->fname.phusrfname,profile_str->dictname.PH_usr_dict);
        strcpy(fepcb->fname.tjsysfname,profile_str->dictname.TJ_sys_dict);
        strcpy(fepcb->fname.tjusrfname,profile_str->dictname.TJ_usr_dict);
        fepcb->learning = profile_str->initlearn;                  /* V410 */
        fepcb->Lang_Type = profile_str->initcodeset;               /* @big5*/
        fepcb->iconv_flag = profile_str->iconv_flag;               /* @big5*/
        fepcb->keylayout = profile_str->initkeyboard;              /* @big5*/
        if (profile_str->initlearn)                                /* V410 */
        {                                                          /* V410 */
          strcpy(fepcb->fname.learnname,profile_str->learnfile);   /* V410 */
          init_learning(fepcb);                                    /* V410 */
        }                                                          /* V410 */

        if(TjLoadSysFileMI(fepcb) == ERROR )
        {                           /* load Tsang-Jye system file */
            free(fepcb->preinpbuf);
            free(fepcb->curinpbuf);
            free(fepcb->preedbuf);
            free(fepcb->ctrl_r_buf);
            free(fepcb->radeucbuf);
            TedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }
        if(PhLoadSysFileMI(fepcb) == ERROR )
        {                           /* load Phonetic system file */
            free(fepcb->preinpbuf);
            free(fepcb->curinpbuf);
            free(fepcb->preedbuf);
            free(fepcb->ctrl_r_buf);
            free(fepcb->radeucbuf);
            TedFreeDictionaryName(fepcb);
            free(fepcb);
            return(ERROR);
        }
        TjLoadUsrFileMI(fepcb); /* load Tsang-Jye user file */
        PhLoadUsrFileMI(fepcb); /* load Phonetic user file  */
        return ((int)fepcb) ;
} /* end of TedInit */

TedFreeDictionaryName(fepcb)
FEPCB *fepcb;
{
     free(fepcb->fname.phsysfname);
     free(fepcb->fname.tjsysfname);
     free(fepcb->fname.phusrfname);
     free(fepcb->fname.tjusrfname);
     free(fepcb->fname.learnname);                         /* @big5 */
}
