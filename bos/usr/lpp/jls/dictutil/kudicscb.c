static char sccsid[] = "@(#)31	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicscb.c, cmdKJI, bos411, 9428A410j 7/23/92 01:21:58";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicscb, buff_clr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         kudicscb
 *
 * DESCRIPTIVE NAME:    User Dictionary Data Search
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            uer dictionary data search
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        3680 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicscb
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicscb( smd, crb, ymd, yml, kjd, kjl, scp )
 *
 *  INPUT               smd     : search mode
 *                      *crb    : pointer to current buffer record
 *                      *ymd    : pointer to yomidata
 *                      yml     : length of yomidata
 *                      *kjd    : pointer to kjdata
 *                      kjl     : length of kjdata
 *
 *  OUTPUT:             **scp   : pointer to poniter to search buffer record
 *
 * EXIT-NORMAL:         search buffer record : success return
 *
 * EXIT-ERROR:          NULL    : error return
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              kudicymc : pc-code to 7bit-code conversion
 *                              kudicycr : 7bit-code to pc-code conversion
 *                      Standard Liblary.
 *                              memcmp  : compare memory buffer string
 *                      Advanced Display Graphics Support Liblary(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Liblary.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Liblary.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include  <stdio.h>

/*
 *      include Kanji Project.
 */
#include  "kut.h"

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */

void kudicscb( smd, crb, ymd, yml, kjd, kjl, scp )

 short      smd ;   /* search mode                       : (i)            */
 UDCS      *crb ;   /* pointer to current buffer record  : (i)            */
 uchar     *ymd ;   /* pointer to yomidata               : (i)            */
 short      yml ;   /* length of yomidata                : (i)            */
 uchar     *kjd ;   /* pointer to kjdata                 : (i)            */
 short      kjl ;   /* length of kjdata                  : (i)            */
 UDCS     **scp ;   /* pointer to search buffer record   : (o)            */

{
 UDCS  *wrk     ;   /* local current buffer record                        */
 UDCS  *wrk2    ;   /* local current buffer record                        */
 UDCS  *lst_wrk ;   /* local current buffer record : last                 */
 uchar  t_cmp   ;   /* local <for> statement compelete flag               */
 uchar  t_cmp1  ;   /* local <for> statement compelete flag : 1           */
 uchar  hd_flag ;   /* local search current flag : now                    */
 uchar  hl_flag ;   /* local search current flag : last                   */
 uchar  stat_fr_flag ;  /* local search current flag : front              */
 uchar  stat_rv_flag ;  /* local search current flag : reverse            */

 uchar  ym_data[U_BUFFUL] ;  /* local yomidata convert buffer             */
 uchar  ym_set[U_BUFFUL]  ;  /* local wrk->ymd convert buffer             */
 short  type    ;   /* local convert pc-code to 7bit-code  type           */
 short  ym_len  ;   /* local yml buffer                                   */
 short  yd_len  ;   /* local wrk->yomilen buffer                          */


 void   kudicymc(); /* convert pc-code to 7bit-code                       */
 void   kudicycr(); /* convert 7bit-code to pc-code                       */

 /*
  *   program  start
  */
 wrk  = crb ;       /* calling current buffer pointer to local area       */
 wrk2 = crb ;       /* calling current buffer pointer to local area       */
 lst_wrk = crb ;    /* calling current buffer pointer to local area : last*/

 hd_flag = 0 ;      /* search current flag : now  is <0> clear            */
 hl_flag = 0 ;      /* search current flag : last is <0> clear            */

 stat_fr_flag = 0 ;     /* local search current flag : front              */
 stat_rv_flag = 0 ;     /* local search current flag : reverse            */

 buff_clr( &ym_data[0] ) ;
 kudicymc( ymd, yml, &ym_data[0], &ym_len, &type ) ;   /* convert input   */
                                          /*           yomidata convert   */
 kudicycr( &ym_data[0], ym_len, ymd, &yml ) ;

                                          /* search mode check            */
 switch ( smd )                           /* <0>:yomi only  <1>:yomi & kj */
 {                                        /* <2>:gerric                   */

 /*******************************/
 /* if search mode is yomi only */
 /*******************************/

  case U_SD_0 :

   for (t_cmp = 0 ; t_cmp == 0 ;)         /* search compelete check       */
   {
                                          /* current bufffer yomi length  */
    if (wrk->yomilen == yml)              /* & search buffer yomi length  */
    {                                     /* compare                      */
     if (memcmp(wrk->yomi,ymd,yml) == 0)
                                          /* current buffre yomi & */
     {                                    /* search buffer yomi compare   */
      for ( t_cmp1 = 0 ; t_cmp1 == 0 ; )  /* search compelete check       */
      {
       if ( wrk->pr_pos == NULL )
       {
        t_cmp1 = 1;                       /* <for> complete flag  on      */
       }

       else
       {
        wrk = wrk->pr_pos ;
                                           /* yomi change check            */
        if ( ( memcmp(wrk->yomi,ymd,yml) != 0) ||
                      ( wrk->yomilen != yml ))
        {
         wrk = wrk->nx_pos ;
         t_cmp1 = 1 ;
        }
       }
      }

      for ( t_cmp1 = 0 ; t_cmp1 == 0 ; )  /* search compelete check       */
      {
       if ( ( wrk->status == U_S_YOMD ) ||
            ( wrk->status == U_S_KNJD ) ||
            ( wrk->status == U_S_ADEL ) )  /*  status  check              */
       {
        if ( wrk->nx_pos == NULL )         /* next point     null  check  */
        {
         *scp   = NULL ;                   /* return to NULL              */
         t_cmp  = 1    ;                   /* search compelete flag  on   */
         t_cmp1 = 1    ;                   /* search compelete flag  on   */
        }

        else
        {
         wrk = wrk->nx_pos ;               /* next point set              */
                                           /* yomi change check            */
         if ( ( wrk->yomilen != yml ) ||
              ( memcmp(wrk->yomi,ymd,yml) != 0 ))
         {
          *scp = NULL ;                    /* return to NULL              */
          t_cmp  = 1  ;                    /* <for>  compelete flag  on   */
          t_cmp1 = 1  ;                    /* <for>  compelete flag  on   */
         }
        }
       }

       else
       {
        *scp   = wrk ;                    /* search point set             */
        t_cmp  = 1   ;                    /* <for> comeplete flag on      */
        t_cmp1 = 1  ;                     /* <for> comeplete flag on      */
       }
      }
     }

     else
     {
      buff_clr( &ym_set[0] ) ;
      kudicymc( wrk->yomi, wrk->yomilen, &ym_set[0], &yd_len, &type ) ;
                                          /* convert search buffer        */
                                          /*           yomidata convert   */

      if (memcmp(&ym_set[0], &ym_data[0], ym_len) < 0)
                                          /* current buffer yomi  &       */
                                          /* search buffer yomi           */
      {                                   /*      comapre                 */
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                  /* current flag : front         */
      }

      else
      {
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }
     }
    }

    else
    {
     buff_clr( &ym_set[0] ) ;
     kudicymc( wrk->yomi, wrk->yomilen, &ym_set[0], &yd_len, &type ) ;
                                          /* convert search buffer        */
                                          /*           yomidata convert   */

                                          /* current buffer yomi          */
     if (wrk->yomilen < yml)              /* length & search buf-         */
     {                                    /* -fer length compare          */
      if (memcmp(wrk->yomi,ymd,wrk->yomilen) == 0)
                                          /* current buffer yomi          */
      {                                   /* & search buffer yomi compare */
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                  /* current flag : front         */
      }

      else if (memcmp(&ym_set[0], &ym_data[0], yd_len) > 0)
                                          /* current buffer yomi          */
                                          /* & search buffer              */
      {                                   /* yomi compare                 */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }

      else
      {
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                  /* current flag : front         */
      }
     }

     else
     {
      if (memcmp(wrk->yomi,ymd,yml) == 0)
      {                                   /* current buffer yomi & search */
                                          /*   buffer yomi compare        */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }

      else if (memcmp(&ym_set[0], &ym_data[0], ym_len) > 0)
                                          /* current buffer yomi len-     */
                                          /* -gth & seach buffer yomi */
      {                                   /* length compare               */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }

      else
      {
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                  /* current flag : front         */
      }
     }
    }
                                          /* search current               */
    if ( ((hl_flag != 0) && (hd_flag != hl_flag)) || /* check or next/pr- */
         (wrk == NULL) )                  /* -evious point null check     */
    {
     *scp  = NULL ;                       /* search buffer point <null>   */
     t_cmp = 1 ;                          /* <for> compelete flag on      */
    }

    else
    {
     hl_flag = hd_flag ;                  /* current flag now to last     */
    }
   }
   break ;



 /*******************************/
 /* yomi & kanji search process */
 /*******************************/

   case U_SD_1 :

    for (t_cmp = 0  ; t_cmp == 0 ;)       /* search compelete cehck       */
    {
     if ( (wrk->yomilen == yml) &&
          (memcmp(wrk->yomi,ymd,yml) == 0) )
                                          /* current buffer string &      */
                                          /* search buffre string che-    */
     {                                    /* -ck                          */
      if (wrk->pr_pos == NULL)            /* previous point null cehck    */
      {
       t_cmp   = 1 ;                      /* <for> compelete flag on      */
       hd_flag = U_FRONT ;                  /* current flag : front         */
      }

      else
      {
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_FRONT ;                  /* current flag : front         */
      }
     }

     else
     {
      t_cmp = 1 ;                         /* <for> compelte flag on       */
     }
    }

    for (t_cmp = 0 ; t_cmp == 0 ;)        /* search compelet check        */
    {
                                          /* current buffer yomi length & */
     if (wrk->yomilen == yml)             /* search buffre yomi length    */
     {                                    /* compare                      */
      if (memcmp(wrk->yomi,ymd,yml) == 0)
                                          /* current buffer yomi &        */
      {                                   /* search buffer yomi compare   */
       t_cmp  = 1   ;                     /* <for> compelete flag on      */
      }

      else
      {
       buff_clr( &ym_set[0] ) ;
       kudicymc( wrk->yomi, wrk->yomilen, &ym_set[0], &yd_len, &type ) ;
                                          /* convert search buffer        */
                                          /*           yomidata convert   */

       if (memcmp(&ym_set[0], &ym_data[0], ym_len) < 0)
                                          /* current buffer yomi &        */
                                          /* search buffer yomi compare   */
       {
        wrk = wrk->nx_pos ;               /* next point set               */
        hd_flag = U_FRONT ;                 /* current flag : front         */
       }

       else
       {
        wrk = wrk->pr_pos ;               /* previous point set           */
        hd_flag = U_REVES ;               /* current flag : reverse       */
       }
      }
     }
                                          /* current buffer yomi length & */
     else if (wrk->yomilen < yml)         /* search buffer yomi length    */
     {                                    /* compare                      */
      if (memcmp(wrk->yomi,ymd,wrk->yomilen) == 0)
                                          /* current buffer yomi &        */
      {                                   /*  search buffer yomi compare  */
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                /* current flag : front         */
      }

      else
      {
       buff_clr( &ym_set[0] ) ;
       kudicymc( wrk->yomi, wrk->yomilen, &ym_set[0], &yd_len, &type ) ;
                                          /* convert search buffer        */
                                          /*           yomidata convert   */

       if (memcmp(&ym_set[0], &ym_data[0], yd_len) > 0)
                                          /* current buffer yomi & search */
       {                                  /* buffer yomi  compare         */
        wrk = wrk->pr_pos ;               /* previous point set           */
        hd_flag = U_REVES ;               /* current flag : reverse       */
       }

       else
       {
        wrk = wrk->nx_pos ;               /* next poitn set               */
        hd_flag = U_FRONT ;                 /* current flag : front         */
       }
      }
     }

     else
     {                                    /* current buffer yomi & search */
      if (memcmp(wrk->yomi,ymd,yml) == 0)
      {                                   /* buffer yomi comapre          */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }

      else
      {
       buff_clr( &ym_set[0] ) ;
       kudicymc( wrk->yomi, wrk->yomilen, &ym_set[0], &yd_len, &type ) ;
                                          /* convert search buffer        */
                                          /*           yomidata convert   */

       if (memcmp(&ym_set[0], &ym_data[0], ym_len) > 0)
                                          /* current buffer yomi & search */
       {                                  /* buffer yomi compare          */
        wrk = wrk->pr_pos ;               /* previous point set           */
        hd_flag = U_REVES ;               /* current flag : reverse       */
       }

       else
       {
        wrk = wrk->nx_pos ;               /* next point set               */
        hd_flag = U_FRONT ;                 /* current flag : front         */
       }
      }
     }
                                          /* current flag chcek or next/  */
     if ( ((hl_flag != 0) && (hd_flag != hl_flag)) || /* previuos point   */
                             (wrk == 0) )             /* check            */
     {
      t_cmp  = 1 ;                        /* <for> compelete flag on      */
      hd_flag = 0 ;                       /* current flag : null          */
     }

     else
     {
      hl_flag = hd_flag ;                 /* current flag now to last     */
     }
    }

    if (hd_flag == 0)                     /* current flag null check      */
    {
     *scp = NULL ;                        /* search point null set        */
    }

    else
    {
     for (t_cmp = 0 ; t_cmp == 0 ;)       /* search compelete check       */
     {
      if ((wrk->kanlen == kjl) &&         /* current buffer kanji/length  */
          (memcmp(wrk->kan,kjd,kjl) == 0) && /*search buffre kanji/length */
          (wrk->status != U_S_YOMD ) &&
          (wrk->status != U_S_KNJD ) &&
          (wrk->status != U_S_ADEL ) )
      {                                   /* compare                      */
       *scp   = wrk ;                     /* search point set             */
        t_cmp = 1 ;                       /* <for> compelete flag on      */
      }

      else
      {
       if (hd_flag == U_FRONT)              /* current flag front cehck     */
       {
        if (wrk->nx_pos == NULL)          /* next point null check        */
        {
         *scp  = NULL ;                   /* search point null set        */
         t_cmp = 1 ;                      /* <for> compelete flag on      */
        }

        else
        {
         wrk = wrk->nx_pos ;              /* next point set               */
        }
       }

       else
       {
        if (wrk->pr_pos == 0)             /* previous point null check    */
        {
         *scp  = NULL ;                   /* search point null set        */
         t_cmp = 1 ;                      /* <for> compelete flag on      */
        }

        else
        {
         wrk = wrk->pr_pos ;              /* previous point set           */
        }
       }
                                          /* yomi change check            */
       if ( (t_cmp == 0) && ((wrk->yomilen != yml) ||
            (memcmp(wrk->yomi,ymd,yml) != 0)) )
       {
        *scp  = NULL ;                    /* search point null            */
        t_cmp = 1 ;                       /* <for> compelete flag on      */
       }
      }
     }
    }

    break ;


 /*****************************************/
 /* if search mode is yomi only           */
 /*     near buffer recorde return        */
 /*****************************************/

  case U_SD_2 :

   for (t_cmp = 0 ; t_cmp == 0 ;)         /* search compelete check       */
   {
    if (wrk->yomilen == yml)              /* & search buffer yomi length  */
    {                                     /* compare                      */
     if (memcmp(wrk->yomi,ymd,yml) == 0)
                                          /* current buffre yomi & */
     {                                    /* search buffer yomi compare   */
      for ( t_cmp1 = 0 ; t_cmp1 == 0 ; )  /* search compelete check       */
      {
       if ( wrk->pr_pos == NULL )         /* previous point null check    */
       {
        t_cmp1 = 1;                       /* <for>  compelete flag on     */
       }

       else
       {
        lst_wrk = wrk ;                   /* set search point             */
        wrk = wrk->pr_pos ;               /* set previous point           */

                                          /*  change length & yomi  check */
        if ( ( memcmp(wrk->yomi,ymd,yml) != 0)
                                   || ( wrk->yomilen != yml ))
        {
         lst_wrk = wrk ;                  /* set search point             */
         wrk = wrk->nx_pos ;              /* set next point               */
         t_cmp1 = 1 ;                     /* <for>  compelete flag  on    */
        }
       }
      }

      lst_wrk = wrk ;                     /* set search point             */
      stat_rv_flag = 0 ;                  /* search flag initilize        */
      stat_fr_flag = 0 ;                  /* search flag initilize        */

      if (wrk->nx_pos == NULL)            /* next point  null check       */
      {
       stat_rv_flag = 1 ;                 /* search flag (reverse)  on    */
      }

      if (wrk->pr_pos == NULL)            /* previous point  null check   */
      {
       stat_fr_flag = 1 ;                 /* search flag (front)  on      */
      }

                                          /* next point & previous point  */
                                          /*                 null check   */
      if ( (wrk->nx_pos != NULL) && (wrk->pr_pos != NULL) )
      {
       stat_fr_flag = 1 ;                 /* search flag (front)  on      */
       stat_rv_flag = 1 ;                 /* search flag (reverse)  on    */
      }

      wrk2 = wrk ;                        /* set  search point            */
     }

     else
     {
      buff_clr( &ym_set[0] ) ;
      kudicymc( wrk->yomi, wrk->yomilen, &ym_set[0], &yd_len, &type ) ;
                                          /* convert search buffer        */
                                          /*           yomidata convert   */

      if (memcmp(&ym_set[0], &ym_data[0], ym_len) < 0)
                                          /* current buffer yomi  &       */
                                          /* search buffer yomi           */
      {                                   /*      comapre                 */
       lst_wrk = wrk ;                    /* search point set             */
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                  /* current flag : front         */
      }

      else
      {
       lst_wrk = wrk ;                    /* search point set             */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }
     }
    }

    else
    {
     buff_clr( &ym_set[0] ) ;
     kudicymc( wrk->yomi, wrk->yomilen, &ym_set[0], &yd_len, &type ) ;
                                          /* convert search buffer        */
                                          /*           yomidata convert   */

                                          /* current buffer yomi          */
     if (wrk->yomilen < yml)              /* length & search buf-         */
     {                                    /* -fer length compare          */
      if (memcmp(wrk->yomi,ymd,wrk->yomilen) == 0)
                                          /* current buffer yomi          */
      {                                   /* & search buffer yomi compare */
       lst_wrk = wrk ;
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                /* current flag : front         */
      }

      else if (memcmp(&ym_set[0], &ym_data[0], yd_len) > 0)
                                          /* current buffer yomi          */
                                          /* & search buffer              */
      {                                   /* yomi compare                 */
       lst_wrk = wrk ;                    /* search point set             */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }

      else
      {
       lst_wrk = wrk ;                    /* search point set             */
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                  /* current flag : front         */
      }
     }

     else
     {
      if (memcmp(wrk->yomi,ymd,yml) == 0)
      {                                   /* current buffer yomi & search */
                                          /*   buffer yomi compare        */
       lst_wrk = wrk ;                    /* search point set             */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }

      else if (memcmp(&ym_set[0], &ym_data[0], ym_len) > 0)
                                          /* current buffer yomi len-     */
                                          /* -gth & seach buffer yomi */
      {                                   /* length compare               */
       lst_wrk = wrk ;                    /* search point set             */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }

      else
      {
       lst_wrk = wrk ;                    /* search point set             */
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                  /* current flag : front         */
      }
     }
    }

                                         /* search flag (front & reverse) */
                                         /*             null check        */
    if ( (stat_rv_flag == NULL) && (stat_fr_flag == NULL) )
    {
     stat_rv_flag = 0 ;                  /* search flag (front & reverse) */
     stat_fr_flag = 0 ;                  /*                 initilize     */

                                          /* current flag (now & last)    */
                                          /*            check             */
     if ( (hd_flag == U_REVES) && (hl_flag == U_FRONT) )
     {
      wrk2 = wrk->nx_pos ;                /*  next point set              */
      stat_rv_flag = 1 ;                 /* search flag (front & reverse) */
      stat_fr_flag = 1 ;                 /*           on                  */
     }

                                          /* current flag (now & last)    */
                                          /*            check             */
     else  if ( (hd_flag == U_FRONT) && (hl_flag == U_REVES) )
     {
      wrk2 = wrk ;                        /* next point  check            */
      stat_rv_flag = 1 ;                 /* search flag (front & reverse) */
      stat_fr_flag = 1 ;                 /*           on                  */
     }
                                          /* current flag (now) &         */
                                          /*   next point    check        */
     else  if ( (lst_wrk->nx_pos == NULL) && (hd_flag == U_FRONT) )
     {
      wrk2 = lst_wrk ;                    /* local record point (last) set*/
      stat_rv_flag = 1 ;                  /* search flag (reverse)  on    */
     }

                                          /* current flag (now) &         */
                                          /*   previous point check       */
     else  if ( (lst_wrk->pr_pos == NULL) && (hd_flag == U_REVES) )
     {
      wrk2 = lst_wrk ;                    /* local record point (last) set*/
      stat_fr_flag = 1 ;                  /* search flag (front)  on      */
     }

     else
     {
      hl_flag = hd_flag ;                 /* current flag now to last     */
     }
    }

    else
    {
     hl_flag = hd_flag ;                  /* current flag now to last     */
    }

    if (stat_fr_flag == 1)                /* search flag (front) check    */
    {
     wrk = wrk2 ;                         /* local record point set       */
     for ( ; stat_fr_flag == 1 ; )        /* search compelete check       */
     {
      if ( ( wrk->status != U_S_YOMD ) &&
           ( wrk->status != U_S_KNJD ) &&
           ( wrk->status != U_S_ADEL )    ) /* status  check              */
      {                                   /* case  <ok>                   */
       *scp = wrk ;                       /* search point  set            */
       stat_fr_flag = 0 ;                 /* search flag (front & reverse)*/
       stat_rv_flag = 0 ;                 /*           initilize          */
       t_cmp = 1 ;                        /* <for> compelete flag  on     */
      }

      else                                /* case <ng>                    */
      {
       if (wrk->nx_pos != NULL)           /* next point  null  check      */
       {
        wrk = wrk->nx_pos ;               /* next point  set              */
       }

       else
       {
        stat_fr_flag = 0 ;                /* search flag (front) initilize*/

        if (stat_rv_flag == 0)            /* search flag (reverse)  check */
        {
         *scp = NULL ;                    /* search point null set        */
         t_cmp = 1 ;                      /* <for> compelete flag  on     */
        }
       }
      }
     }
    }

    if (stat_rv_flag == 1)                /* search flag (reverse)  check */
    {
     wrk = wrk2 ;                         /* local record point  set      */
     for ( ; stat_rv_flag == 1 ; )        /* search compelete check       */
     {
      if ( ( wrk->status != U_S_YOMD ) &&
           ( wrk->status != U_S_KNJD ) &&
           ( wrk->status != U_S_ADEL )    ) /* status  check              */
      {                                   /* case <ok>                    */
       *scp = wrk ;                       /* search point set             */
       stat_rv_flag = 0 ;                 /* search flag (reverse)        */
                                          /*             initilize        */
       t_cmp = 1 ;                        /* <for> compelete flag  on     */
      }

      else
      {                                   /* case  <ng>                   */
       if (wrk->pr_pos != NULL)           /* previous point  check        */
       {
        wrk = wrk->pr_pos ;               /* previous point set           */
       }

       else
       {
        *scp = NULL ;                     /* search point null set        */
        stat_rv_flag = 0 ;                /* search flag (reverse)        */
                                          /*           initilize          */
        t_cmp = 1 ;                       /* <for> compelete flag  on     */
       }
      }
     }
    }

   }
   break ;

 }

}

/********************************/
/*   convert  buffer  clear     */
/********************************/
buff_clr( buff )
uchar  *buff ;
{
uchar  i ;

 for ( i = 0 ; i < 20 ; i++ )
 {
  *(buff+i) = NULL ;
 }
}
