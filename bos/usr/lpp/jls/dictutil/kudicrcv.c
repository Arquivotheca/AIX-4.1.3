static char sccsid[] = "@(#)30	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicrcv.c, cmdKJI, bos411, 9428A410j 7/23/92 01:17:08";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicrcv
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

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kudicrcv
 *
 * DESCRIPTIVE NAME:    User Dictionary Recovery
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
 * FUNCTION:            user dictionary recovery
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        4752 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicrcv
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicrcv( udcbptr )
 *
 *  INPUT:              udcbptr  : pointer to UDCB Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         UDSUCC : Success of Execution.
 *
 * EXIT-ERROR:          UDRIMPE  : Recovery Unpossibl  Error Code
 *                      UDRNVDW  : Nothing Data  Error Code
 *                      UDRDPDW  : Invalid Data  Error Code
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              kudcread : User Dictionaly read
 *                      Standard Liblary.
 *                              memcmp  :  compare memory buffer string
 *                              memcpy  :  copy    memory buffer string
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
#include <stdio.h>      /* Standard I/O Package.                        */
/*#include <memory.h>*/ /* Memory Package                               */

/*
 *      include Kanji Project.
 */
#include "kut.h"        /* Utility Define File                          */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

int kudicrcv( udcbptr )

 UDCB      *udcbptr ;   /* pointer to user dictionary cntrol block */

{
 UDCB  *wrk     ;                /* local current buffer record           */
 uchar *mruchkptr ;              /* local mru check pointer               */
 uchar *mru_wrk_ptr ;            /* mru start position set pointer        */
 uchar *idchkptr ;               /* local index check pointer             */
 uchar *rlchkptr ;               /* local data area check pointer         */
 uchar *dl_chk_ptr ;             /* local dl check pointer                */
 uchar *m_dat_ptr ;              /* local mru data check pointer          */
 uchar *i_dat_ptr ;              /* local index data read pointer         */
 uchar *d_dat_ptr ;              /* local dl data read pointer            */
 uchar *r_dat_ptr ;              /* local rl data read pointer            */
 uchar *id_wrk    ;              /* local index work pointer              */
 uchar *har_ptr   ;              /* local har  pointer                    */
 uchar *nar_ptr   ;              /* local nar  pointer                    */

 uchar *wrt_mru_ptr ;            /* local mru length write pointer        */
 uchar *wrt_il_ptr ;             /* local il  write pointer               */
 uchar *wrt_rl_ptr ;             /* local rl  write pointer               */
 uchar *wrt_dl_ptr ;             /* local dl  write pointer               */

 int    ret_code  ;              /* routine return code set buffer        */

 uchar  t_cmp   ;                /* local <while> statement complete flag */
 uchar  t_cmp1  ;                /* local <while> statement complete flag */
 uchar  t_cmp2  ;                /* local <while> statement complete flag */
 uchar  t_cmp3  ;                /* local <while> statement complete flag */

 uchar  err_flag ;               /* local error flag                      */
 uchar  skip_flag ;              /* local kanji check  skip flag          */

 uchar  re_code ;           /* local recovery current code                */
 uchar  mru_req_flag ;      /* local mru recover request flag             */
 uchar  idx_req_flag ;      /* local index recover request flag           */
 uchar  rrn_req_flag ;      /* local rrn recover request flag             */
 uchar  rrn_etr_flag ;      /* local rrn entry request flag               */
 uchar  chk_flag     ;      /* kana & kanji check error flag              */

 int    rst ;               /* compare result set buffer                  */

 uchar  set_cnt ;           /* local <for> statement data set counter     */

 short  i_rrn    ;               /* local rrn                             */
 short  i_len    ;               /* local mru length                      */
 short  il_len   ;               /* local index il buffer                 */
 short  i_rl     ;               /* local rl                              */

 uchar  r_dl     ;               /* local dl                              */


 uchar  r_knl     ;               /* local rl knl buffer                  */
 uchar  dl_data_num ;             /* dl knj data num                      */

 uchar  data_area[U_REC_L1] ;    /* local rrn data area                   */
 uchar  dl_data[U_DL_ARA]   ;    /* local dl data buffer                  */
 uchar  lknbuff[U_BUFFUL]   ;    /* local kana data buffer : last         */
 uchar  iknbuff[U_BUFFUL]   ;    /* local kana data buffer : last (index) */
 uchar  iklbuff[U_BUFFUL]   ;    /* local kana data buffer : last (index) */
 uchar  rklbuff[U_BUFFUL]   ;    /* local kana data buffer : last (rl)    */
 uchar  r_kn_buff[U_BUFFUL] ;    /* local kana data buffer : ( rrn )      */
 uchar  r_kl_buff[U_BUFFUL] ;    /*                        : last ( rrn ) */
 uchar  kn_dat[U_BUFFUL]    ;    /* local mru kana data buffer            */
 uchar  kj_dat[U_KJFLD]     ;    /* local mru kanji data buffer           */
 uchar  idx_dat[U_BUFFUL]   ;    /* local index kana data buffer          */
 uchar  rkn_dat[U_BUFFUL]   ;    /* local rl entry kana buffer            */

 uchar  mru_knl      ;           /* local mru kana length buffer          */
 uchar  mru_kjl      ;           /* local mru kanji length buffer         */
 uchar  idx_knl      ;           /* local index kana length buffer        */
 uchar  lst_idx_knl  ;           /* local index kana length buffer : last */
 uchar  lst_r_knl    ;           /* local rl    kana length buffer : last */


 /*********************************/
 /*        program    start       */
 /*********************************/

 /***************/
 /*  initialize */
 /***************/

 wrk         = udcbptr ;  /* calling current buffer pointer to local area */

 mru_req_flag = 0 ;      /* local mru recover request flag    initialize  */
 idx_req_flag = 0 ;      /* local index recover request flag  initialize  */
 rrn_req_flag = 0 ;      /* local rrn recover request flag    initialize  */
 rrn_etr_flag = 0 ;      /* local rrn entry request flag      initialize  */
 err_flag     = 0 ;      /* local error flag                  initialize  */
 re_code      = 0 ;      /* local recovery  code  flag        initialize  */
 ret_code     = 0 ;      /* local return code                 initialize  */


 for ( set_cnt = 0 ; set_cnt < U_BUFFUL ; set_cnt++ ) /* data set         */
 {
  lknbuff[set_cnt] = 0 ;          /* local kana data buffer  initialize   */
  iknbuff[set_cnt] = 0 ;
  iklbuff[set_cnt] = 0 ;
  rklbuff[set_cnt] = 0 ;
  r_kn_buff[set_cnt] = 0 ;
  r_kl_buff[set_cnt] = 0 ;
 }

 /***************/
 /* mru check   */
 /***************/

 mru_wrk_ptr = wrk->dcptr ;             /* copy from user dictionary      */
 wrt_mru_ptr = wrk->dcptr ;             /*   pointer to local wrok pointer*/

 mruchkptr = (wrk->dcptr + U_BUFPTR ) ; /* local mru check pointer        */
                                        /*                     initialize */
 m_dat_ptr = mruchkptr ;                /* local mru entry data pointer   */
                                        /*                      initilize */
 i_len = GMRULEN( mru_wrk_ptr ) ;            /* copy from mru length to   */
                                             /*        local mru length   */
 if ( (i_len < U_BAS_IL) || (i_len > U_MRU_A) )
 {                                           /* mru length check          */
  mru_req_flag = 1 ;                         /* mru error flag  on        */
  err_flag     = 2 ;
 }

 t_cmp = 0 ;
 while (( err_flag == 0 ) && ( t_cmp == 0 ))     /* search error flag  &  */
 {                                               /*   complete flag check */
  mru_knl  = *m_dat_ptr ;                        /* copy from knl to      */
                                                 /*     local buffer      */
  m_dat_ptr += 1 ;                               /* entry dat pointer     */
                                                 /*       incriment       */

  if (( mru_knl >= U_MN_KNL ) && ( mru_knl <= U_MX_KNL ))
  {                                              /* KNL limit check       */
   for (set_cnt = 0 ; set_cnt < mru_knl-1 ; set_cnt++ ) /* kana data copy */
   {
    kn_dat[set_cnt] = *m_dat_ptr ;               /* data copy             */

    m_dat_ptr += 1 ;
   }

   chk_flag = 0 ;                          /* kana check flag clear       */
   skip_flag = 0 ;                         /* kanji check skip flag clear */

   for (set_cnt = 0 ; set_cnt < mru_knl-1 ; set_cnt++ )
   {                                       /* kana code check             */

    if ( set_cnt == 0 )                    /* first kana data check       */
    {
     if ( kn_dat[set_cnt] == U_ESC_CD )    /* first kana data  escape code*/
     {                                     /*                    check    */
      chk_flag = 1 ;                       /* kana check flag on          */
     }

     else
     {
      if ( kn_dat[set_cnt] == U_PND_CD )   /* first kana data  pond code  */
      {                                    /*                  check      */
       skip_flag = 1 ;                     /* kanji check skip flag  on   */
      }

      else
      {
       if ((kn_dat[set_cnt] < U_COD_C1) || (kn_dat[set_cnt] > U_COD_C2))
       {                                   /* limit check                 */
        err_flag = 1 ;                     /*     error flag   on         */
       }
      }

     }
    }

    else
    {
     switch ( chk_flag )                   /* data check flag   check     */
     {
      case  0 :                            /* alphanumeric  limit check   */
         if ((kn_dat[set_cnt] < U_COD_C1) || (kn_dat[set_cnt] > U_COD_C2))
         {
          err_flag = 1 ;                   /* error flag   on             */
         }
         break ;

      case  1 :                            /* kana limit check            */
         if ((kn_dat[set_cnt] < U_COD_C3) || (kn_dat[set_cnt] > U_COD_C4))
         {
          err_flag = 1 ;                   /* error flag   on             */
         }
         break ;
     }
    }
   }
  }

  else
  {
   err_flag = 1 ;                                 /* error flag   on    */
  }

  mru_kjl  = *m_dat_ptr ;                        /* copy from kjl to      */
                                                 /*      local buffer     */
  m_dat_ptr += 1 ;

  if ( err_flag == 0 )                            /* error flag  check    */
  {
   if (( mru_kjl >= U_MN_KJL ) && ( mru_kjl <= U_MX_KJL ))
   {                                              /* kjl limit check      */
    for ( set_cnt = 0 ; set_cnt < mru_kjl-1 ; set_cnt++ )
    {                                             /* copy from kanji data */
     kj_dat[set_cnt] = *m_dat_ptr ;               /* to local kanji buffer*/
     m_dat_ptr += 1 ;
    }

    if ( skip_flag == 0 )                         /* kanji check skip flag*/
    {                                             /*             check    */
     for (set_cnt=0;((set_cnt < mru_kjl-1)&&(err_flag == 0 ));set_cnt+=2)
     {                                            /* check kanji data     */
      if (( kj_dat[set_cnt] & U_LO_COD ) != U_LO_COD )
      {                                           /* msb  check           */
       switch ( kj_dat[set_cnt] & U_CD_CNV )      /*   4 bit  check       */
       {
        case  U_PC_KJ0 :                          /* all bit off          */
        case  U_PC_KJ1 :                          /* 0 bit  on            */
               kj_dat[set_cnt] = kj_dat[set_cnt] | U_LO_COD ;
               break ;                            /* convert pc code      */

        case  U_PC_KJ2 :                          /* 0 bit off, 1 bit on  */
        case  U_PC_KJ3 :                          /* 0 bit on , 1 bit on  */
               kj_dat[set_cnt] = kj_dat[set_cnt] | U_HI_COD ;
               break ;                            /* convert pc code      */
       }
      }

      if(((kj_dat[set_cnt] >= U_FST_C1) && (kj_dat[set_cnt] <= U_FST_C2)) ||
         ((kj_dat[set_cnt] >= U_FST_C3) && (kj_dat[set_cnt] <= U_FST_C4)) )
      {                                           /*first byte limit check*/
       if((kj_dat[set_cnt+1] < U_SEC_C1) || (kj_dat[set_cnt+1] > U_SEC_C4)||
         ((kj_dat[set_cnt+1] > U_SEC_C2) && (kj_dat[set_cnt+1] < U_SEC_C3)))
       {                                          /* second byte limit    */
                                                  /*             check    */
        err_flag = 1 ;                            /* error flag  on       */
       }
      }

      else
      {
       err_flag = 1 ;                             /* error flag  on       */
      }
     }
    }
   }

   else
   {                                              /* error flag  on       */
    err_flag = 1 ;
   }
  }

  if ( err_flag == 0 )                       /* error flag     check      */
  {
   mruchkptr = mruchkptr + mru_knl + mru_kjl ; /* mru check pointer      */
   m_dat_ptr = mruchkptr ;                     /*     renew              */

   if ( i_len <= (mruchkptr-mru_wrk_ptr) )   /* mru length & mru check    */
   {                                         /*   pointer compare         */
    if ( i_len != mruchkptr-mru_wrk_ptr )    /* mru length & mru check    */
    {                                        /* pointer  compare          */
     err_flag = 1 ;                          /* error  flag  on           */
    }

    else
    {
     t_cmp = 1 ;                           /* <while> complete flag on    */
    }
   }
  }
 }

 if ( err_flag == 1 )                     /*  error flag  check           */
 {
  mru_req_flag = 1 ;                      /* mru recover request flag on  */
  err_flag     = 0 ;
 }


 /*************************/
 /*  index . data check   */
 /*************************/

 if ( err_flag == 1 )
 {
  err_flag = 0 ;
 }

 if ( err_flag == 0 )
 {

  kudcread( wrk , (short)3 , NULL ) ;     /* get index area address get   */
  il_len = GETSHORT( wrk->rdptr ) ;       /* copy from il to local il area*/

  wrt_il_ptr = wrk->rdptr ;

  if ( wrk->ufilsz != ( (*(wrk->rdptr + 3)+1) * U_REC_L ) )
  {                                      /* File Size   check           */
   re_code  = U_ST_CD2 ;                   /* set recovery code           */
   err_flag = 1 ;                          /* error flag on               */
  }

 }

 if ( err_flag == 0 )                     /* recovery code check         */
 {
  if (( *(wrk->rdptr+3) < U_HAR_V1 ) || ( *(wrk->rdptr+3) == U_HARIV1 ) ||
     ( *(wrk->rdptr+3) == U_HARIV2 ) || ( *(wrk->rdptr+3) == U_HARIV3 )  )
  {                                       /* index har limit check       */
   re_code  = U_ST_CD2 ;                  /* set recovery code           */
   err_flag = 1 ;                         /* error flag on               */
  }
 }


 if ( err_flag == 0 )
 {
  if( (*(wrk->rdptr+4) < U_NARMIN)     ||
      (*(wrk->rdptr+4) > *(wrk->rdptr+3)+2) )
  {                                      /* index nar limit check        */
   re_code  = U_ST_CD2 ;                 /* set recovery code            */
   err_flag = 1 ;                        /* error flag on                */
  }
 }

 if ( err_flag == 0 )
 {
  if ( il_len <= U_MINNAR )             /* IL limit check                 */
  {
   idx_req_flag = 1 ;                   /* set index recover request flag */
   err_flag = 1 ;                       /* error flag on                  */
  }
 }

 if ( err_flag == 0 )
 {
  if ( *(wrk->rdptr+3) <= U_HAR_V2 )
  {                                          /* index har check          */
   if ( il_len > U_REC_L1 )               /* index il limit check        */
   {
    idx_req_flag = 1 ;                  /* set index recover request flag */
    err_flag = 1 ;                        /* error flag on               */
   }
  }

  else
  {
   if ( *(wrk->rdptr+3) <= U_HAR_V4 )
   {                                      /* index har check             */
    if ( il_len > U_REC_L2 )              /* index il limit check        */
    {
     idx_req_flag = 1 ;                /* set index recover request flag */
     err_flag = 1 ;                       /* error flag on               */
    }
   }

   else
   {
    if ( *(wrk->rdptr+3) <= U_HAR_V6 )
    {                                     /* index har check             */
     if ( il_len > U_REC_L3 )             /* index il limit check        */
     {
      idx_req_flag = 1 ;               /* set index recover request flag */
      err_flag = 1 ;                      /* error flag on               */
     }
    }
   }
  }
 }

 /****************/
 /* index check  */
 /****************/

  id_wrk = wrk->rdptr ;
  idchkptr = wrk->rdptr+U_IL_HED ;      /* index check pointer initialize */
  har_ptr  = wrk->rdptr + 3 ;           /* local har   pointer initialize */
  nar_ptr  = wrk->rdptr + 4 ;           /* local nar   pointer initialize */
  i_dat_ptr = idchkptr ;                /* index read pointer  initialize */
  lst_idx_knl = 0 ;                     /* clear last index knl           */


 t_cmp1 = 0 ;
 while (( err_flag == 0 ) && ( t_cmp1 == 0 ))    /* serach error flag &   */
 {                                               /*   complete flag check */

  lst_r_knl  = 0 ;                               /* clear last rl knl     */
  idx_knl  = *i_dat_ptr ;                        /* copy from knl to      */
                                                 /*     local buffer      */
  i_dat_ptr += 1 ;                               /* entry dat pointer     */
                                                 /*       incriment       */
  if (( idx_knl >= U_MN_KNL ) && ( idx_knl <= U_MX_KNL ))
  {                                           /* index knl limit check    */
   for (set_cnt = 0 ; set_cnt < idx_knl-1 ; set_cnt++ )   /*  data copy   */
   {
    idx_dat[set_cnt] = *i_dat_ptr ;              /* copy kana data        */
    i_dat_ptr += 1 ;
   }

   i_rrn = *i_dat_ptr ;                          /* copy from index rrn   */
                                                 /*     to  local rrn     */

   chk_flag = 0 ;                             /* kana check flag clear    */
   for (set_cnt = 0 ; set_cnt < idx_knl-1 ; set_cnt++ )   /* kana data    */
   {                                                      /*      check   */
    if ( set_cnt == 0 )
    {
     if ( idx_dat[set_cnt] == U_ESC_CD )   /* first kana data escape code */
     {                                     /*                    check    */
      chk_flag = 1 ;                       /* kana check flag on          */
     }

     else
     {
      if ((idx_dat[set_cnt] < U_COD_C1) || (idx_dat[set_cnt] > U_COD_C2))
      {                                    /* limit check                 */
       idx_req_flag = 1 ;                  /* index recovery request      */
                                           /*                 flag   on   */
       err_flag = 1 ;                      /* error  flag   on            */
      }
     }
    }

    else
    {
     switch ( chk_flag )                   /* data check flag  check      */
     {
      case  0 :                            /* alphanumeric  limit check   */
       if ((idx_dat[set_cnt] < U_COD_C1) || (idx_dat[set_cnt] > U_COD_C2))
       {
        idx_req_flag = 1 ;                 /* index recovery request      */
                                           /*                 flag   on   */
        err_flag = 1 ;                     /* error  flag   on            */
       }
       break ;

      case  1 :                            /* kana limit check            */
       if ((idx_dat[set_cnt] < U_COD_C3) || (idx_dat[set_cnt] > U_COD_C4))
       {
        idx_req_flag = 1 ;                 /* index recovery request      */
                                           /*                 flag on     */
        err_flag = 1 ;                     /* error  flag   on            */
       }
       break ;
     }

    }
   }
  }

  else
  {
   idx_req_flag = 1 ;                      /* index recovery request      */
                                           /*                flag on      */
   err_flag     = 1 ;                      /*  error   flag   on          */
  }

  /*
   *   index kana data last & now compare
   */

  if ( err_flag == 0 )                     /* check  error flag           */
  {
   if ( lst_idx_knl < idx_knl-1 )          /* kana length last&now copare */
   {
    rst = lst_idx_knl ;                    /* set cpmare length  (last)   */
   }

   else
   {
    rst = idx_knl ;                        /* set compare length  now     */
   }

   rst = memcmp( &iklbuff[0], &idx_dat[0], rst ) ;
                                           /*   compare last kana data   */
                                           /*    & now kana data         */

   if ( ( rst > 0 ) || (( rst == 0 ) && ( lst_idx_knl > idx_knl )) )
   {                                       /* result check               */
                                           /* last kana data great       */
    idx_req_flag = 1 ;                     /* index recover              */
                                           /*    request flag  on        */
    err_flag = 1 ;                         /*  error flag  on            */
   }
  }

  lst_idx_knl = idx_knl - 1 ;              /* copy from now length  to   */
                                           /*      last length           */
  if ( err_flag == 0 )
  {
   for (set_cnt = 0 ; set_cnt < idx_knl-1 ; set_cnt++ ) /* copy kana     */
   {                                                    /*      data     */
    iklbuff[set_cnt] = idx_dat[set_cnt] ;  /* copy from now data         */
                                           /*   to local last data       */
   }
  }

  if ( err_flag == 0 )
  {
   if( ( i_rrn < U_NARMIN ) ||
       ( i_rrn > *nar_ptr ) ||
       ( i_rrn >= *har_ptr)  )
   {                                      /* rrn limit check             */
    idx_req_flag = 1 ;                    /* recovery request flag on    */
    err_flag = 1 ;                        /* error flag on               */
   }
  }

  if ( err_flag == 0 )
  {
   kudcread( wrk , (short)4 , i_rrn ) ;    /* get rrn pointer            */
   wrt_rl_ptr = wrk->rdptr ;               /* set local rl write pointer */
   wrt_dl_ptr = wrk->rdptr + U_RL_HED ;    /* set local dl write pointer */

   memcpy( &data_area[0] , wrk->rdptr , U_REC_L1 ) ;
                                           /* copy from rrn data to      */
                                           /*      local data buffer     */

   rlchkptr = (&data_area[0] + U_RL_HED) ; /* rl check pointer initialize*/
   r_dat_ptr = rlchkptr ;                  /* rl data read pointer       */
                                           /*                initialize  */
   i_rl = GETSHORT( &data_area[0] ) ;      /* copy from rl to local rl   */

   if (( i_rl < U_RL_HED ) || ( i_rl > U_REC_L1 ))
   {
    rrn_req_flag = 1 ;                   /* rrn recover request flag on  */
    err_flag     = 1 ;                   /* error flag on                */
   }
  }

  t_cmp2 = 0 ;                             /* complete flag initialize   */
  while (( err_flag == 0 ) && ( t_cmp2 == 0 )) /*  search error flag &   */
  {                                            /*  complete flag check   */

    r_knl = *r_dat_ptr ;                 /* copy from rl knl to local    */
    r_dat_ptr += 1 ;                     /*local read pointer incriment  */

    if (( r_knl >= U_MN_KNL ) && ( r_knl <= U_MX_KNL ))
    {                                    /* limit check  rl knl          */
     for ( set_cnt = 0 ; set_cnt < r_knl-1 ; set_cnt++ ) /* rl kana data */
     {                                                   /*  copy        */
      rkn_dat[set_cnt] = *r_dat_ptr ;          /* copy from rl kana data */
      r_dat_ptr += 1 ;                         /*  to local buffer       */
     }

     wrt_dl_ptr = wrt_dl_ptr + r_knl ;         /* dl write pointer       */
                                               /*            initialize  */

     memcpy( &dl_data[0] , r_dat_ptr , *r_dat_ptr ) ;
                                           /* copy from dl data  to      */
                                           /*      local dl buffer       */

     chk_flag = 0 ;                            /* kana check flag clear  */
     skip_flag = 0 ;                           /* kanji check skip flag  */

     for (set_cnt = 0 ; set_cnt < r_knl-1 ; set_cnt++ )   /* kana data */
     {                                                    /*    check  */
      if ( set_cnt == 0 )
      {
       if ( rkn_dat[set_cnt] == U_ESC_CD )   /* first kana data escape code */
       {                                     /*                    check    */
        chk_flag = 1 ;                       /* kana check flag on       */
       }

       else
       {
        if ( rkn_dat[set_cnt] == U_PND_CD ) /* first kana data  pond code  */
        {                                   /*                  check      */
         skip_flag = 1 ;                    /* kanji check skip flag  on   */
        }

        else
        {
         if ((rkn_dat[set_cnt] < U_COD_C1) || (rkn_dat[set_cnt] > U_COD_C2))
         {
          rrn_req_flag = 1;                    /* rrn recover request  */
                                               /*         flag on      */
          err_flag = 1 ;                       /* error flag   on      */
         }
        }

       }
      }

      else
      {
       switch ( chk_flag )                 /* data check flag  check      */
       {
        case  0 :                          /* alphanumeric  limit check   */
         if ((rkn_dat[set_cnt] < U_COD_C1) || (rkn_dat[set_cnt] > U_COD_C2))
         {
          rrn_req_flag = 1;                    /* rrn recover request  */
                                               /*         flag on      */
          err_flag = 1 ;                       /* error flag   on      */
         }
         break ;

       case  1 :                           /* kana limit check            */
        if ((rkn_dat[set_cnt] < U_COD_C3) || (rkn_dat[set_cnt] > U_COD_C4))
        {
         rrn_req_flag = 1;                     /* rrn recover request */
                                               /*         flag on     */
         err_flag = 1 ;                        /* error flag   on     */
        }
        break ;
       }
      }
     }
    }

    else
    {
     rrn_req_flag = 1;                 /* rrn recover request flag on   */
     err_flag = 1 ;                    /* error flag on                 */
    }

  /*
   *   rl kana data last & now compare
   */

   if ( err_flag == 0 )                 /* check  error flag             */
   {
    if ( lst_r_knl < r_knl-1 )          /* kana length last & now copare */
    {
     rst = lst_r_knl ;                  /* set cpmare length   last      */
    }

    else
    {
     rst = r_knl-1 ;                      /* set compare length  now     */
    }

    rst = memcmp( &rklbuff[0], &rkn_dat[0], rst ) ;
                                              /* compare last kana data */
                                              /*  & now kana data       */

    if ( ( rst > 0 ) || ( ( rst == 0 ) && ( lst_r_knl > r_knl ) ))
    {                                         /* result check           */
                                              /* last kana data great   */
     rrn_req_flag = 1 ;                       /* rrn   recover          */
                                              /*    request flag  on    */
     err_flag = 1 ;                           /*  error flag  on        */
    }
   }

   lst_r_knl = r_knl - 1 ;                 /* copy from now length  to   */
                                           /*      last length           */
   if ( err_flag == 0 )
   {
    for (set_cnt = 0 ; set_cnt < r_knl-1 ; set_cnt++ )   /* copy kana  */
    {                                                    /*      data  */
     rklbuff[set_cnt] = rkn_dat[set_cnt] ;     /* copy from now data   */
    }                                          /*   to local last data */

    r_dl = dl_data[U_DL_HED] ;          /* copy from dl to             */
                                        /*     local buffer            */
    r_dat_ptr += 1 ;                      /* local read pointer incriment*/
    dl_chk_ptr = &dl_data[0] + U_DL_RSV ; /*  copy from rl read pointer  */
                                          /*  to dl check pointer        */
    d_dat_ptr = dl_chk_ptr ;              /* copy from dl check pointer  */
                                          /*      to dl data read pointer*/

    if (( r_dl < U_DL_MN ) || ( r_dl > U_DL_MX ))
    {                                   /* dl limit check              */
     rrn_req_flag = 1 ;                 /* rrn recover request flag on */
     err_flag = 1 ;                     /* error flag  on              */
    }
   }

   t_cmp3 = 0 ;                             /* complete flag initialize*/
   while (( err_flag == 0 ) && ( t_cmp3 == 0 )) /*  search error flag &*/
   {                                            /*  complete flag check*/
    d_dat_ptr += U_DL_RSV ;             /*dl read pointer incriment    */
    if ( err_flag == 0 )
    {
     dl_data_num = 0 ;                      /* dl kanji data number clear*/
     set_cnt = 0 ;

     if ( skip_flag == 0 )
     {
      do
      {
       if ( ( *d_dat_ptr & U_LO_COD ) != U_LO_COD )
       {                                      /* msb  check           */
        switch ( *d_dat_ptr & U_CD_CNV )      /*   4 bit  check       */
        {
         case  U_PC_KJ0 :                     /* all bit off          */
         case  U_PC_KJ1 :                     /* 0 bit  on            */
                *d_dat_ptr = *d_dat_ptr | U_LO_COD ;
                break ;                       /* convert pc code      */

         case  U_PC_KJ2 :                     /* 0 bit off, 1 bit on  */
         case  U_PC_KJ3 :                     /* 0 bit on , 1 bit on  */
                *d_dat_ptr = *d_dat_ptr | U_HI_COD ;
                break ;                       /* convert pc code      */
        }                                     /* convert pc code      */
       }

       else
       {
        set_cnt = 1 ;                         /* kanji data end flag  */
       }


       if ( (( *d_dat_ptr >= U_FST_C1 ) && ( *d_dat_ptr <= U_FST_C2 )) ||
            (( *d_dat_ptr >= U_FST_C3 ) && ( *d_dat_ptr <= U_FST_C4 ))   )
       {                                        /* first byte limit check */
        if ( ( *(d_dat_ptr+U_KJ_NXT) < U_SEC_C1 ) ||
             ( *(d_dat_ptr+U_KJ_NXT) > U_SEC_C4 ) ||
             ( ( *(d_dat_ptr+U_KJ_NXT) > U_SEC_C2 )  &&
               ( *(d_dat_ptr+U_KJ_NXT) < U_SEC_C3 ) ) )
        {                                         /* second byte limit    */
                                                  /*             check    */
         rrn_etr_flag = 1 ;                       /* rrn entry request    */
                                                  /*           flag  on   */
         err_flag = 1 ;                           /* error flag  on       */
        }

        dl_data_num += U_KJ_SKP ;              /* kanji data number + 2   */
        d_dat_ptr   += U_KJ_SKP ;              /*dl read pointer incriment*/
       }

       else
       {
        rrn_etr_flag = 1 ;                     /* rrn entry request flag  */
                                               /*               on        */
        err_flag = 1 ;                         /*     error flag  on      */
       }

      } while ( (set_cnt == 0) && (err_flag == 0) ) ;
                                               /* search  kanji data end  */
                                               /*    flag  &  error flag  */
     }
    }

    if ( ( err_flag == 0 ) && ( skip_flag == 0 ) ) /* check error flag &  */
    {                                              /*    skip flag        */
     if ( ( dl_data_num < U_MN_KJL ) || ( dl_data_num > U_MX_KJL ))
     {                                         /* limit check             */
                                               /*       kanji data number */
      rrn_etr_flag = 1 ;                       /* rrn entry request flag  */
      err_flag = 1 ;                           /* error flag        on    */
     }
    }

    /*
     *  renew check pointer
     */

    if ( err_flag == 0 )                       /* check error flag        */
    {
     if ( skip_flag == 0 )                     /* check  skip flag        */
     {
      dl_chk_ptr = d_dat_ptr ;                 /* normal kanji check      */
                                               /* dl check pointer renew  */
     }

     else
     {                                         /* skip kanji check        */
      dl_chk_ptr = dl_chk_ptr + ( r_dl - U_DL_RSV ) ;
                                               /* dl check pointer renew  */
     }

     d_dat_ptr  = dl_chk_ptr ;               /* dl data read pointer renew*/

     /*
      *   check dl end ?
      */

     if ( r_dl <= (dl_chk_ptr - &dl_data[0]) ) /* dl check end            */
     {
      if ( r_dl != (dl_chk_ptr - &dl_data[0] ) ) /* compare dl & check    */
      {                                          /*              pointer  */
       rrn_etr_flag = 1 ;                   /* rrn recover request flag on*/
       err_flag = 1 ;                       /* error flag on              */
      }

      else
      {
       t_cmp3 = 1 ;                       /* <while> complete flag on  */
      }
     }
    }

   }

   /*
    *  renew check pointer
    */

   if ( err_flag == 0 )
   {
    rlchkptr = rlchkptr + (r_knl + r_dl) ;     /* rrn check pointer renew */
    r_dat_ptr = rlchkptr ;

    wrt_dl_ptr = wrt_dl_ptr + r_dl ;           /* write dl pointer renew  */

   /*
    *   check rl end ?
    */

    if ( i_rl <= (rlchkptr - &data_area[0] ))  /* rl check  end           */
    {
     if ( i_rl != (rlchkptr - &data_area[0] )) /* compare rl & rl check   */
     {                                         /*               pointer   */
      rrn_req_flag = 1 ;                    /* rrn recover request flag on*/
      err_flag = 1 ;                        /* error flag on              */
     }

     else
     {
      t_cmp2 = 1 ;                      /* <while> complete flag on   */
     }
    }
   }

  }

  /*
   *  renew check pointer
   */

  if ( err_flag == 0 )
  {
   idchkptr = idchkptr + idx_knl + U_RRN_L1 ;  /* index check pointer  */
                                               /*             renew    */
   i_dat_ptr = idchkptr ;

  /*
   *   check il  end ?
   */

   if ( il_len <= ( idchkptr - id_wrk ))  /* il check end ?            */
   {
    if ( il_len != ( idchkptr - id_wrk )) /* compare il & il check     */
    {                                     /*               pointer     */
     idx_req_flag = 1 ;                /* index recover request flag on*/
     err_flag = 1 ;                    /* error flag on                */
    }

    else
    {
     t_cmp1 = 1 ;                      /* <while> complete flag on     */
    }
   }
  }
 }


 /*
  * code & request   check
  */

 if ( re_code == U_ST_CD0 )               /* check recovery code          */
 {
  if ( rrn_etr_flag != 0 )                /* rrn entry recovery rqeuest ? */
  {                                       /*      on                      */
   if ( dl_chk_ptr == &dl_data[0] + U_DL_RSV )  /* dl check pointer =     */
   {                                            /*       initial  value   */
    rrn_req_flag = 1 ;                    /* rrn recover request flag on  */
   }

   else                                   /* dl check pointer !=          */
   {                                      /*             initial value    */
    r_dl = dl_chk_ptr - &dl_data[0] ;     /* set dl check pointer to dl   */

    *wrt_dl_ptr = r_dl ;                  /* write user dictionary  dl    */

    rlchkptr = rlchkptr + r_knl + r_dl ;  /* renew rl check pointer       */

    rrn_req_flag = 1 ;                    /* rrn recover request flag on  */
   }
  }

  if ( rrn_req_flag != 0 )                /* check rrn recovery request   */
  {                                       /*                     flag     */
   if ( rlchkptr == &data_area[0] + U_RL_HED ) /* rl check pointer =      */
   {                                           /*           initial value */
    idx_req_flag = 1 ;                    /* index recover request flag on*/
   }

   else                                  /* rl check pointer != initial   */
   {                                     /*                        value  */
    i_rl = rlchkptr - &data_area[0] ;    /* set  rl check pointer to rl   */

    SETSHORT( wrt_rl_ptr , i_rl ) ;      /* write user disctionaly rl     */

    idchkptr = idchkptr + idx_knl + 1 ;  /* renew index check pointer     */
    idx_req_flag = 1 ;                   /* index recovery request flag on*/
   }
  }

  if ( idx_req_flag != 0 )               /* check index recovery request  */
  {                                      /*                      flag     */
   if ( idchkptr == id_wrk+U_IL_HED )    /* index check pointer = initial */
   {                                     /*                       value   */
    if ( ( (*har_ptr) >= U_HAR_V1 ) && ( (*har_ptr) <= U_HAR_V2 ) )
    {
     *nar_ptr = U_NAR_1K ;               /*  nar  initilize ( 1K )        */
    }

    if ( ( (*har_ptr) >= U_HAR_V3 ) && ( (*har_ptr) <= U_HAR_V4 ) )
    {
     *nar_ptr = U_NAR_2K ;               /*  nar  initilize ( 2K )        */
    }

    if ( ( (*har_ptr) >= U_HAR_V5 ) && ( (*har_ptr) <= U_HAR_V6 ) )
    {
     *nar_ptr = U_NAR_3K ;               /*  nar  initilize ( 3K )        */
    }

    re_code = U_ST_CD5 ;                 /* set recovery code             */
   }

   else                                  /* index check pointer !=        */
   {                                     /*                initial value  */
    re_code = U_ST_CD6 ;                 /* set recovery code             */
   }

   il_len = idchkptr - id_wrk ;          /* set index check pointer to il */

   SETSHORT( wrt_il_ptr , il_len ) ;     /* write user dictionary  il     */
  }
 }

  if ( mru_req_flag == 1 )            /* check mru recovery request    */
  {                                   /*                    flag       */
   if ( err_flag == 2)
   {
    i_len = U_BUFPTR ;                 /* set mru length  initilize    */
    SETSHORT( wrt_mru_ptr , i_len ) ;  /* write user dictionary        */
                                       /*               mru length     */
    re_code = U_ST_CD2 ;               /* set recovery code            */
   }

   else
   {
    if ( mruchkptr == mru_wrk_ptr+U_BUFPTR ) /* mru check pointer =    */
    {                                        /*          initial value */
     i_len = mruchkptr - mru_wrk_ptr ; /* set mru check pointer to     */
                                       /*                  mru length  */
     SETSHORT( wrt_mru_ptr , i_len ) ; /* write user dictionary        */
                                       /*               mru length     */
    }

    else                               /* mru check pointer !=         */
    {                                  /*                initial value */
     i_len = mruchkptr - mru_wrk_ptr ; /* set mru check pointer to     */
                                       /*                mru length    */
     SETSHORT( wrt_mru_ptr , i_len ) ; /* write user dictionary        */
                                       /*               mru length     */
    }
   }
  }


 /*
  *  set return code
  */

 switch ( re_code )                      /* check recovery code           */
 {
  case U_ST_CD0 :                        /* case  normal end              */
           *(wrt_il_ptr+U_ID_STS) = NULL ;  /* write index sts            */
           wrk->updflg = U_FON ;            /* update flag   on           */
           ret_code    = UDSUCC ;           /* set normal end code to     */
                                            /*             return code    */
           break ;

  case U_ST_CD2 :                           /* case  unpossible           */
           ret_code = UDRIMPE ;             /* set recovery unpossible    */
                                            /*     code to  return code   */
           break ;

  case U_ST_CD5 :                        /* case  nothing data            */
           *(wrt_il_ptr+U_ID_STS) = NULL ;  /* write index sts            */
           wrk->updflg = U_FON ;            /* update flag   on           */
           ret_code    = UDRNVDW ;          /* set nothing data code to   */
           break ;                          /*              return code   */

  case U_ST_CD6 :                        /* case  invalid data            */
           *(wrt_il_ptr+U_ID_STS) = NULL ;  /*  write index sts           */
           wrk->updflg = U_FON ;            /* update flag   on           */
           ret_code    = UDRDPDW ;          /* set invalid data code  to  */
                                            /*             return code    */
           break ;
 }

 return( ret_code ) ;

}
