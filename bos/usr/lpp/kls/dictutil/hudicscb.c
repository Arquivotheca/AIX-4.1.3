static char sccsid[] = "@(#)09	1.1  src/bos/usr/lpp/kls/dictutil/hudicscb.c, cmdkr, bos411, 9428A410j 5/25/92 14:44:15";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicscb.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hudicscb.c
 *
 *  Description:  User Dictionary Data Search
 *
 *  Functions:    hudicscb()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

#include  <stdio.h>
#include  "hut.h"

void hudicscb( smd, crb, key, keylen, cand, candlen, scp )

 short      smd ;   /* search mode                       : (i)            */
 UDCS      *crb ;   /* pointer to current buffer record  : (i)            */
 uchar     *key ;   /* pointer to keydata               : (i)            */
 short      keylen ;   /* length of keydata                : (i)            */
 uchar     *cand ;   /* pointer to candata                 : (i)            */
 short      candlen ;   /* length of candata                  : (i)            */
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

 uchar  l_keydata[U_KEY_MX] ;  /* local keydata convert buffer             */
 uchar  udcs_keydata[U_KEY_MX]  ;  /* local wrk->key convert buffer             */
 uchar  i       ;   /* local counter                                      */
 short  type    ;   /* local convert pc-code to 7bit-code  type           */
 short  l_keylen  ;   /* local keylen buffer                                   */
 short  udcs_keylen  ;   /* local wrk->keylen buffer                          */



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

 memset(l_keydata, 0xff, U_KEY_MX);
 l_keylen = keylen;
 memcpy(l_keydata, key, keylen);
                                          /* search mode check            */
 switch ( smd )                           /* <0>:key only  <1>:key & kj */
 {                                        /* <2>:gerric                   */

 /*******************************/
 /* if search mode is key only */
 /*******************************/

  case U_SD_0 :

   for (t_cmp = 0 ; t_cmp == 0 ;)         /* search compelete check       */
   {
                                          /* current bufffer key length  */
    if (wrk->keylen == keylen)              /* & search buffer key length  */
    {                                     /* compare                      */
     if (memcmp(wrk->key,key,keylen) == 0)
                                          /* current buffre key & */
     {                                    /* search buffer key compare   */
      for ( t_cmp1 = 0 ; t_cmp1 == 0 ; )  /* search compelete check       */
      {
       if ( wrk->pr_pos == NULL )
       {
        t_cmp1 = 1;                       /* <for> complete flag  on      */
       }

       else
       {
        wrk = wrk->pr_pos ;
                                           /* key change check            */
        if ( ( memcmp(wrk->key,key,keylen) != 0) ||
                      ( wrk->keylen != keylen ))
        {
         wrk = wrk->nx_pos ;
         t_cmp1 = 1 ;
        }
       }
      }

      for ( t_cmp1 = 0 ; t_cmp1 == 0 ; )  /* search compelete check       */
      {
       if ( ( wrk->status == U_S_KEYD ) ||
            ( wrk->status == U_S_CAND ) ||
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
                                           /* key change check            */
         if ( ( wrk->keylen != keylen ) ||
              ( memcmp(wrk->key,key,keylen) != 0 ))
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
      memset(udcs_keydata, 0xff, U_KEY_MX);
      udcs_keylen = wrk->keylen;
      memcpy(udcs_keydata, wrk->key, wrk->keylen);

      if (memcmp(&udcs_keydata[0], &l_keydata[0], l_keylen) < 0)
                                          /* current buffer key  &       */
                                          /* search buffer key           */
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
     memset(udcs_keydata, 0xff, U_KEY_MX);
     udcs_keylen = wrk->keylen;
     memcpy(udcs_keydata, wrk->key, wrk->keylen);

                                          /* current buffer key          */
     if (wrk->keylen < keylen)              /* length & search buf-         */
     {                                    /* -fer length compare          */
      if (memcmp(wrk->key,key,wrk->keylen) == 0)
                                          /* current buffer key          */
      {                                   /* & search buffer key compare */
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                  /* current flag : front         */
      }

      else if (memcmp(&udcs_keydata[0], &l_keydata[0], udcs_keylen) > 0)
                                          /* current buffer key          */
                                          /* & search buffer              */
      {                                   /* key compare                 */
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
      if (memcmp(wrk->key,key,keylen) == 0)
      {                                   /* current buffer key & search */
                                          /*   buffer key compare        */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }

      else if (memcmp(&udcs_keydata[0], &l_keydata[0], l_keylen) > 0)
                                          /* current buffer key len-     */
                                          /* -gth & seach buffer key */
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
 /* key & candji search process */
 /*******************************/

   case U_SD_1 :

    for (t_cmp = 0  ; t_cmp == 0 ;)       /* search compelete cehck       */
    {
     if ( (wrk->keylen == keylen) &&
          (memcmp(wrk->key,key,keylen) == 0) )
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
                                          /* current buffer key length & */
     if (wrk->keylen == keylen)             /* search buffre key length    */
     {                                    /* compare                      */
      if (memcmp(wrk->key,key,keylen) == 0)
                                          /* current buffer key &        */
      {                                   /* search buffer key compare   */
       t_cmp  = 1   ;                     /* <for> compelete flag on      */
      }

      else
      {
       memset(udcs_keydata, 0xff, U_KEY_MX);
       udcs_keylen = wrk->keylen;
       memcpy(udcs_keydata, wrk->key, wrk->keylen);

       if (memcmp(&udcs_keydata[0], &l_keydata[0], l_keylen) < 0)
                                          /* current buffer key &        */
                                          /* search buffer key compare   */
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
                                          /* current buffer key length & */
     else if (wrk->keylen < keylen)         /* search buffer key length    */
     {                                    /* compare                      */
      if (memcmp(wrk->key,key,wrk->keylen) == 0)
                                          /* current buffer key &        */
      {                                   /*  search buffer key compare  */
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                /* current flag : front         */
      }

      else
      {
       memset(udcs_keydata, 0xff, U_KEY_MX);
       udcs_keylen = wrk->keylen;
       memcpy(udcs_keydata, wrk->key, wrk->keylen);

       if (memcmp(&udcs_keydata[0], &l_keydata[0], udcs_keylen) > 0)
                                          /* current buffer key & search */
       {                                  /* buffer key  compare         */
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
     {                                    /* current buffer key & search */
      if (memcmp(wrk->key,key,keylen) == 0)
      {                                   /* buffer key comapre          */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }

      else
      {
       memset(udcs_keydata, 0xff, U_KEY_MX);
       udcs_keylen = wrk->keylen;
       memcpy(udcs_keydata, wrk->key, wrk->keylen);

       if (memcmp(&udcs_keydata[0], &l_keydata[0], l_keylen) > 0)
                                          /* current buffer key & search */
       {                                  /* buffer key compare          */
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
      if ((wrk->candlen == candlen) &&         /* current buffer candji/length  */
          (memcmp(wrk->cand,cand,candlen) == 0) && /*search buffre candji/length */
          (wrk->status != U_S_KEYD ) &&
          (wrk->status != U_S_CAND ) &&
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
                                          /* key change check            */
       if ( (t_cmp == 0) && ((wrk->keylen != keylen) ||
            (memcmp(wrk->key,key,keylen) != 0)) )
       {
        *scp  = NULL ;                    /* search point null            */
        t_cmp = 1 ;                       /* <for> compelete flag on      */
       }
      }
     }
    }

    break ;


 /*****************************************/
 /* if search mode is key only           */
 /*     near buffer recorde return        */
 /*****************************************/

  case U_SD_2 :

   for (t_cmp = 0 ; t_cmp == 0 ;)         /* search compelete check       */
   {
    if (wrk->keylen == keylen)              /* & search buffer key length  */
    {                                     /* compare                      */
     if (memcmp(wrk->key,key,keylen) == 0)
                                          /* current buffre key & */
     {                                    /* search buffer key compare   */
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

                                          /*  change length & key  check */
        if ( ( memcmp(wrk->key,key,keylen) != 0)
                                   || ( wrk->keylen != keylen ))
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
      memset(udcs_keydata, 0xff, U_KEY_MX);
      udcs_keylen = wrk->keylen;
      memcpy(udcs_keydata, wrk->key, wrk->keylen);

      if (memcmp(&udcs_keydata[0], &l_keydata[0], l_keylen) < 0)
                                          /* current buffer key  &       */
                                          /* search buffer key           */
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
     memset(udcs_keydata, 0xff, U_KEY_MX);
     udcs_keylen = wrk->keylen;
     memcpy(udcs_keydata, wrk->key, wrk->keylen);

                                          /* current buffer key          */
     if (wrk->keylen < keylen)              /* length & search buf-         */
     {                                    /* -fer length compare          */
      if (memcmp(wrk->key,key,wrk->keylen) == 0)
                                          /* current buffer key          */
      {                                   /* & search buffer key compare */
       lst_wrk = wrk ;
       wrk = wrk->nx_pos ;                /* next point set               */
       hd_flag = U_FRONT ;                /* current flag : front         */
      }

      else if (memcmp(&udcs_keydata[0], &l_keydata[0], udcs_keylen) > 0)
                                          /* current buffer key          */
                                          /* & search buffer              */
      {                                   /* key compare                 */
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
      if (memcmp(wrk->key,key,keylen) == 0)
      {                                   /* current buffer key & search */
                                          /*   buffer key compare        */
       lst_wrk = wrk ;                    /* search point set             */
       wrk = wrk->pr_pos ;                /* previous point set           */
       hd_flag = U_REVES ;                /* current flag : reverse       */
      }

      else if (memcmp(&udcs_keydata[0], &l_keydata[0], l_keylen) > 0)
                                          /* current buffer key len-     */
                                          /* -gth & seach buffer key */
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
      if ( ( wrk->status != U_S_KEYD ) &&
           ( wrk->status != U_S_CAND ) &&
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
      if ( ( wrk->status != U_S_KEYD ) &&
           ( wrk->status != U_S_CAND ) &&
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
