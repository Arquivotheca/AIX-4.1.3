static char sccsid[] = "@(#)08	1.1  src/bos/usr/lpp/kls/dictutil/hudicrcv.c, cmdkr, bos411, 9428A410j 5/25/92 14:44:02";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicrcv.c
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
/************************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hudicrcv.c
 *
 *  Description:  User Dictionary Recovery
 *
 *  Functions:    hudicrcv()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ************************************************************************/
  
/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Memory Package                               */
#include "hut.h"        /* Utility Define File                          */

/*----------------------------------------------------------------------*/
/*                      Begining of hudicrcv.                           */
/*----------------------------------------------------------------------*/
int 	  hudicrcv( udcbptr , recv_flag)
UDCB      *udcbptr ;   	/* pointer to user dictionary cntrol block 		*/
int       recv_flag;    /* Flag, which is need to recoverary.			*/
{
 UDCB  *wrk     ;                /* local current buffer record         	*/
 uchar *sts_wrtptr;		 /* User Dictionary Status wirte ptr.		*/
 uchar *dicindex;		 /* User Dictionary Index Block Pointer 	*/
 short i_len;			 /* User Dictionary Index Block Length  	*/
 uchar *i_wrtptr;		 /* User Dictionary Index Block Write Pointer 	*/
 short har, nar;		 /* High Allocated Rrn & Next Allocatable Rrn	*/
 short i_chkp;			 /* U.D. Index BLock Check pos.			*/
 short i_p;			 /* U.D. Index Block Search Base Pos.		*/
 short lst_i_keylen;		 /* Index Block Last Key Data Length.		*/
 short i_keylen;		 /* Index Block Key data length.		*/
 uchar i_key [U_KEY_MX];	 /* Index BLock Key daya.			*/
 short i_rrn=-1;		 /* Index BLock RRN.				*/
 short rst;			 /* Last key and cur key compare result.	*/
 uchar lst_i_keydata[U_KEY_MX];  /* Index Block Last Key Data.			*/
 uchar *d_wrtptr;		 /* Data Block Write Pointer.			*/
 short d_chkp;			 /* Data BLock Check pos.			*/
 short d_p;			 /* Data Block Search Pos.			*/
 short d_cbchkp;		 /* Data BLock's Candidate Block Check Pos.	*/
 short d_len;			 /* Data Block Length.				*/
 short d_cbsz;			 /* Data Block's Candidate Block Size.		*/
 uchar d_key [U_KEY_MX];	 /* Data BLock Key 				*/
 short d_keylen;		 /* Data Block Key Length.			*/
 short lst_d_keylen;		 /* Data Block Last Key Length.			*/
 uchar lst_d_keydata [U_KEY_MX]; /* Data Block Last Key Data.			*/
 short d_cbp;			 /* Data Block's Candidate Block Check Pos.	*/
 uchar d_cb [U_CB_MX];		 /* Data Block's Candidate Block Data.		*/
 short d_cbcandlen;		 /* Candidate Data Length.			*/
 uchar d_cbcand [U_CAN_MX];	 /* Candidate Data.				*/
 uchar *mrudata;		 /* User Dictionary MRU data Pointer.		*/
 uchar *m_wrtptr;		 /* User Dictionary MRU Write Pointer.		*/
 short m_chkp;		 	 /* User Dictionary MRU Check  Pos.		*/
 short m_p;			 /* User Dictionary MRU Search Base Pos.	*/ 
 short m_len;			 /* User Dictionary MRU Block Length.		*/
 uchar m_key  [U_KEY_MX];	 /* User Dictionary MRU Key Data.		*/
 uchar m_cand [U_CAN_MX];	 /* User Dictionary MRU Candidate Data.		*/
 uchar byte;			 /* Code Check Temp Buffer.			*/
 short m_candlen;		 /* User Dictionary MRU Candidate Length. 	*/
 short m_keylen;		 /* User Dictionary MRU Key Length.		*/
 short c_status;		 /* */
 short lastcflag;
 short lst_rrn;
 short pre_rrn;
 int    ret_code  ;              /* routine return code set buffer        	*/
 uchar  t_cmp   ;                /* local <while> statement complete flag 	*/
 uchar  t_cmp1  ;                /* local <while> statement complete flag 	*/
 uchar  t_cmp2  ;                /* local <while> statement complete flag 	*/
 uchar  t_cmp3  ;                /* local <while> statement complete flag 	*/
 uchar  err_flag ;               /* local error flag                      	*/
 uchar  re_code ;           /* local recovery current code                	*/
 uchar  mru_req_flag ;      /* local mru recover request flag             	*/
 uchar  idx_req_flag ;      /* local index recover request flag           	*/
 uchar  rrn_req_flag ;      /* local rrn recover request flag             	*/
 uchar  rrn_etr_flag ;      /* local rrn entry request flag               	*/
 int    set_cnt ;           /* local <for> statement data set counter     	*/
 uchar  dicdata[U_REC_L] ;  /* local rrn data area                   		*/

 /***************/
 /*		*/
 /*  initialize */
 /*		*/
 /***************/

 wrk         = udcbptr ;  /* calling current buffer pointer to local area */
 sts_wrtptr  = wrk->dcptr;

 mru_req_flag = 0 ;      /* local mru recover request flag    initialize  */
 idx_req_flag = 0 ;      /* local index recover request flag  initialize  */
 rrn_req_flag = 0 ;      /* local rrn recover request flag    initialize  */
 rrn_etr_flag = 0 ;      /* local rrn entry request flag      initialize  */
 err_flag     = 0 ;      /* local error flag                  initialize  */
 re_code      = U_ST_CD0 ;   /* local recovery  code  flag        initialize  */
 ret_code     = UDSUCC ;    /* local return code                 initialize  */

 memset(lst_i_keydata, 	0xff, U_KEY_MX);
 memset(lst_d_keydata, 	0xff, U_KEY_MX);
 memset(i_key, 		0xff, U_KEY_MX);
 memset(d_key, 		0xff, U_KEY_MX); 
 memset(lst_d_keydata, 	0xff, U_KEY_MX);
 memset(m_key, 		0xff, U_KEY_MX); 
 memset(m_cand, 	0xff, U_CAN_MX);
 memset(d_cbcand, 	0xff, U_CAN_MX);

 /*********************/
 /*		      */
 /* Status Check .    */ 
 /*		      */
 /*********************/
 (void)getudstat(udcbptr->dcptr, &c_status);
 if (c_status != 0x0000 && c_status != 0x000f &&
     c_status != 0x00f0 && c_status != 0x00ff)
 {
	if (recv_flag == U_FOF) 
        {
		return (UDRNEED);
	}
	else 
	{
		err_flag = 1;
		re_code = U_ST_CD7;
	}
 }
 

 /*********************/
 /*		      */
 /* Mru Block Check   */
 /*		      */
 /*********************/

 mrudata     = wrk->dcptr ;             /* copy from user dictionary      */
 m_wrtptr    = wrk->dcptr ; 		/*   pointer to local wrok pointer*/
 m_chkp      = (U_STSLEN+U_MRULEN);     /* Mru Area Check index pointer */
 m_p	     = m_chkp;		 	/* Mru Area Search index pointer */
 m_len	     = getmrulen(mrudata);      /* Mru Area Length */

 /* Checks mru length. */
 if ( (m_len < U_STSLEN+U_MRULEN) || (m_len > U_MRU_A) )
 {                                      /* mru length check          */
  mru_req_flag = 1 ;                    /* mru error flag  on        */
  err_flag     = 1 ;
 }

 t_cmp = 0 ;
 if (m_len == U_STSLEN+U_MRULEN) t_cmp = 1;
   while (( err_flag == 0 ) && ( t_cmp == 0 ))    
   {                                               
    m_keylen  = nxtkeylen(mrudata, m_p, U_MRU_A) ;
    /* Checks key length. */
    if (m_keylen < 0) {
        mru_req_flag = 1;
        err_flag = 1;
        break;
    }    
    /* Checks Overflow. */
    if (m_p+m_keylen > m_len)
    {
        mru_req_flag = 1;
        err_flag     = 1;
        break;
    }
    /* Checks mru key code. */
    if (( m_keylen >= U_MN_KEY ) && ( m_keylen <= U_MX_KEY ))
    {                                              
     memcpy(m_key, mrudata+m_p, m_keylen);    
     for (set_cnt = 0 ; set_cnt < m_keylen && err_flag == 0; set_cnt+=2 )
     {
      /******************/
      /* Key Data Check */
      /******************/
	/* if not last character of key */
  	if (set_cnt < m_keylen - 2)
  	{
  	   if (m_key[set_cnt] & 0x80)
  	   {
  		err_flag = 1;
  	   }
  	   else
  	   {
  		byte = m_key[set_cnt] | 0x80;
  		if ( (byte == 0xa3) || ((byte >= 0xb0) && (byte <= 0xc8)) )
  		{
  		  byte = m_key[set_cnt+1];
  		  if (byte & 0x80)
  		  {
  			if ( byte < 0xa1 || byte > 0xfe )
  			{
  			   err_flag = 1;
  			}
  		  }
  		  else
  		  {
  			err_flag = 1;
  		  }
  		}
  		else
  		{
  		  err_flag = 1;
  		}
  	   }
  	}
	/* if last character of key */
  	else
  	{
  	   
  		byte = m_key[set_cnt];
  		if ( (byte == 0xa3) || ((byte >= 0xb0) && (byte <= 0xc8)) )
  		{
  			byte = m_key[set_cnt+1];
  			if ( byte < 0xa1 || byte > 0xfe )
  			{
  			   err_flag = 1;
  			}
  		}
  		else
  		{
  		  err_flag = 1;
  		}
  	}
     }	/* end of for */
    }
    /* key length is out of range */
    else
    {
     err_flag = 1 ;                                 /* error flag   on    */
    }
  
    /* Checks mru candidate start. */
    m_candlen = nxtcandlen(mrudata, m_p+m_keylen, &lastcflag, U_MRU_A);
  
    /* Checks mru candidate length */
    if (m_candlen < 0) {
  	mru_req_flag = 1;
  	err_flag = 1;
  	break;
    }
    /* Check overflow */ 
    if (m_p + m_keylen + m_candlen > m_len)
    {
  	mru_req_flag = 1;
  	err_flag = 1;
  	break;
    }
    if ( err_flag == 0 ) /* error flag  check    */
    {
     if (( m_candlen >= U_MN_CAN ) && ( m_candlen <= U_MX_CAN ))
     { 
      /* Local Copy */
      memcpy(m_cand, mrudata+m_p+m_keylen, m_candlen);
      for (set_cnt=0;((set_cnt < m_candlen)&&(err_flag == 0 ));set_cnt+=2)
      {                                            /* check candidate data     */
	/* if not last character of candidate */
  	if (set_cnt < m_candlen - 2)
  	{
  	   if (m_cand[set_cnt] & 0x80)
  	   {
  		err_flag = 1;
  	   }
  	   else
  	   {
  		byte = m_cand[set_cnt] | 0x80;
  		if ( byte >= 0xa1 && byte <= 0xfe )
  		{
  		  if (m_cand[set_cnt+1] & 0x80)
  		  {
  			err_flag = 1;
  		  }
  		  else
  		  {
  		  	byte = m_cand[set_cnt+1] | 0x80;
  			if ( byte < 0xa1 || byte > 0xfe )
  			{
  			   err_flag = 1;
  			}
  		  }
  		}
  		else
  		{
  		  err_flag = 1;
  		}
  	   }
  	}
	/* if last character of candidate */
  	else
  	{
  	   
  		byte = m_cand[set_cnt];
  		if ( byte >= 0xa1 && byte <= 0xfe )
  		{
  			byte = m_cand[set_cnt+1];
  			if ( byte < 0xa1 || byte > 0xfe )
  			{
  			   err_flag = 1;
  			}
  		}
  		else
  		{
  		  err_flag = 1;
  		}
  	}
      } /* end of for */
     }
     /* candidate length is out of range */
     else
     {                                              /* error flag  on       */
      err_flag = 1 ;
     }
    }
  
    /* Advance mru search base pos */
    if ( err_flag == 0 )                       /* error flag     check      */
    {
     m_chkp += (m_keylen+m_candlen);      /* mru check pointer      */
     m_p     = m_chkp ;
     if ( m_len <= m_p )   		/* mru length & mru check    */
     {                                    /*   pointer compare         */
      if ( m_len != m_p )    /* mru length & mru check    */
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
  /**************************************/
  /* Mru Data Candidate Block Check End */
  /**************************************/

 if ( err_flag == 1 )                     /*  error flag  check           */
 {
  if (recv_flag == U_FOF) return UDRNEED;
  mru_req_flag = 1 ;                      /* mru recover request flag on  */
  err_flag     = 0 ;
 }

 /******************************/
 /*  index . data check Start  */
 /******************************/

 if ( err_flag == 1 )
 {
  err_flag = 0 ;
 }

 if ( err_flag == 0 )
 {
  /* Init. For index block serach. */
  hudcread( wrk , (short)3 , NULL ) ;     /* get index area address get   */
  dicindex = wrk->rdptr ;
  i_len    = getil(dicindex);
  i_wrtptr = dicindex;
  har      = gethar(dicindex);
  nar      = getnar(dicindex);
  if ( wrk->ufilsz != ( (har+1) * U_REC_L ) )
  {                                      /* File Size   check           */
   re_code  = U_ST_CD2 ;                   /* set recovery code           */
   err_flag = 1 ;                          /* error flag on               */
   if (recv_flag == U_FOF) return UDRNEED;
  }
 }

 /* Check active index block length. */
 if ( err_flag == 0 )
 {
  if (i_len < (U_ILLEN+U_HARLEN+U_NARLEN) || i_len > U_UIX_A)
  {
   idx_req_flag = 1 ;                   /* set index recover request flag */
   err_flag = 1 ;                       /* error flag on                  */
  }
 }

 /* Check har */
 if ( err_flag == 0 )                     /* recovery code check         */
 {
  if ( har < U_HAR_V1 || har > U_HAR_V2 ) 
  {                                       /* index har limit check       */
   re_code  = U_ST_CD2 ;                  /* set recovery code           */
   err_flag = 1 ;                         /* error flag on               */
   if (recv_flag == U_FOF) return UDRNEED;
  }
 }

 /* Check nar */
 if ( err_flag == 0 )
 {
  if( (nar < U_NARMIN) || (nar > (har+1)) || (lst_rrn=getlastrrn(dicindex, i_len))+1 != nar)
  {                                      /* index nar limit check        */
   re_code  = U_ST_CD2 ;                 /* set recovery code            */
   err_flag = 1 ;                        /* error flag on                */
   if (recv_flag == U_FOF) return UDRNEED;
  }
 }

 /****************/
 /*		 */
 /* index check  */
 /*		 */
 /****************/
 /* Init, index serach base pos. */
 i_chkp = (U_ILLEN+U_HARLEN+U_NARLEN);
 i_p    = (U_ILLEN+U_HARLEN+U_NARLEN);	/* Index Block Search Pointer */
 lst_i_keylen = 0;

 t_cmp1 = 0 ;
 if (i_len == (U_ILLEN+U_HARLEN+U_NARLEN)) t_cmp1 = 1;
   while (( err_flag == 0 ) && ( t_cmp1 == 0 ))    /* serach error flag &   */
   {                                               /*   complete flag check */
    /*****************************/
    /*			         */
    /* In this case, Index block */
    /* is empty. 	   	 */
    /*			         */
    /*****************************/
    lst_d_keylen  = 0 ;                               /* clear last rl knl     */
    i_keylen = nxtkeylen(dicindex, i_p, U_UIX_A);
    /* Check index block key length. */ 
    if ((i_keylen < 0) || (i_p + i_keylen > i_len)) {
       idx_req_flag = 1;
       err_flag = 1;
       break;
    }
    /* Check index block key length. */ 
    if (( i_keylen >= U_MN_KEY ) && ( i_keylen <= U_MX_KEY ))
    {                                           /* index knl limit check    */
     if (i_p + i_keylen + U_RRNLEN > i_len) {
       idx_req_flag = 1;
       err_flag = 1;
       break;
     }
     memcpy(i_key, dicindex+i_p, i_keylen);
 /******/ 
     pre_rrn = i_rrn;
 /******/ 
     (void)getrrn(dicindex+i_p+i_keylen, &i_rrn);
     /* Check rrn. */ 
     if (i_rrn < U_NAR_V1 || i_rrn > U_NAR_V2 || nar <= i_rrn) {
       idx_req_flag = 1;
       err_flag = 1;
       break;
     }
     /* Check key code. */ 
     for (set_cnt = 0 ; set_cnt < i_keylen && err_flag == 0; set_cnt+=2 )   
     {                                                      
      /******************/
      /* Key Data Check */
      /******************/
  	if (set_cnt < i_keylen - 2)
	/* if not last character of key */
  	{
  	   if (i_key[set_cnt] & 0x80)
  	   {
  		idx_req_flag = 1;
  		err_flag = 1;
  	   }
  	   else
  	   {
  		byte = i_key[set_cnt] | 0x80;
  		if ( (byte == 0xa3) || ((byte >= 0xb0) && (byte <= 0xc8)) )
  		{
  		  byte = i_key[set_cnt+1];
  		  if (byte & 0x80)
  		  {
  			if ( byte < 0xa1 || byte > 0xfe )
  			{
  			   idx_req_flag = 1;
  			   err_flag = 1;
  			}
  		  }
  		  else
  		  {
  			idx_req_flag = 1;
  			err_flag = 1;
  		  }
  		}
  		else
  		{
  		  idx_req_flag = 1;
  		  err_flag = 1;
  		}
  	   }
  	}
	/* if last character of key */
  	else
  	{
  	   
  		byte = i_key[set_cnt];
  		if ( (byte == 0xa3) || ((byte >= 0xb0) && (byte <= 0xc8)) )
  		{
  			byte = i_key[set_cnt+1];
  			if ( byte < 0xa1 || byte > 0xfe )
  			{
  			   idx_req_flag = 1;
  			   err_flag = 1;
  			}
  		}
  		else
  		{
  		  idx_req_flag = 1;
  		  err_flag = 1;
  		}
  	}
     }	/* end of for */
    }
    /* index key length is out of range */
    else
    {
     idx_req_flag = 1 ;                      /* index recovery request      */
                                             /*                flag on      */
     err_flag     = 1 ;                      /*  error   flag   on          */
    }
  
    /*
     *   index Key  data last & now compare
     */
  
    if ( err_flag == 0 )                     /* check  error flag           */
    {
     if ( lst_i_keylen < i_keylen-1 )          /* key  length last&now copare */
     {
      rst = lst_i_keylen ;                    /* set cpmare length  (last)   */
     }
     else
     {
      rst = i_keylen ;                        /* set compare length  now     */
     }

     /************************/
     /*                      */
     /* In order to compare. */
     /*                      */
     /************************/
     makeksstr(lst_i_keydata, lst_i_keylen);
     makeksstr(i_key, i_keylen); 

     rst = memcmp( &lst_i_keydata[0], &i_key[0], rst ) ;
                                             /*   compare last key  data   */
     /************************/
     /*                      */
     /* Restore the code.    */
     /*                      */
     /************************/
     mkrbnk(lst_i_keydata, lst_i_keylen);
     mkrbnk(i_key, i_keylen);
  
     /* check if arrangement is out of order or there exists duplicated data */
     if ( ( rst > 0 ) || (( rst == 0 ) && ( lst_i_keylen >= i_keylen )) )
     {                                       /* result check               */
                                             /* last key  data great       */
      idx_req_flag = 1 ;                     /* index recover              */
                                             /*    request flag  on        */
      err_flag = 1 ;                         /*  error flag  on            */
     }
    }
  
    lst_i_keylen = i_keylen - 1 ;              /* copy from now length  to   */
                                             /*      last length           */
    if ( err_flag == 0 )
    {
     memcpy(lst_i_keydata, i_key, i_keylen);
    }
  
    if ( err_flag == 0 )
    {
     if( ( i_rrn < U_NARMIN ) ||
         ( i_rrn > nar ) ||
         ( i_rrn > har )  )
     {                                      /* rrn limit check             */
      idx_req_flag = 1 ;                    /* recovery request flag on    */
      err_flag = 1 ;                        /* error flag on               */
     }
    }
  
    if ( err_flag == 0 )
    {
     /* Init, for data block check */
     hudcread( wrk , (short)4 , i_rrn ) ;    /* get rrn pointer            */
     d_wrtptr = wrk->rdptr;
     memcpy( &dicdata[0] , wrk->rdptr , U_REC_L1 ) ;
     d_chkp     = U_RLLEN ;
     d_p        = U_RLLEN ;
     d_cbchkp   = 0;
     d_len      = getrl(dicdata);
     if (( d_len < U_RLLEN ) || ( d_len > U_REC_L1 ))
     {
      rrn_req_flag = 1 ;                   /* rrn recover request flag on  */
      err_flag     = 1 ;                   /* error flag on                */
     }
    }
  
    t_cmp2 = 0 ;                             /* complete flag initialize   */
    if (d_len == U_RLLEN) {
	t_cmp2 = 1;
    }
    /* data block check start. */
    while (( err_flag == 0 ) && ( t_cmp2 == 0 )) /*  search error flag &   */
    {                                            /*  complete flag check   */
      d_keylen = nxtkeylen(dicdata, d_p, U_REC_L);
      /* Check key length. */ 
      if (d_keylen < 0 || d_p+d_keylen > d_len) {
         rrn_req_flag = 1;
         err_flag = 1;
         break;
      }
      /* Check key length. */ 
      if (( d_keylen >= U_MN_KEY ) && ( d_keylen <= U_MX_KEY ))
      { 
       memcpy(d_key, dicdata+d_p, d_keylen);
       d_cbchkp   = 0;
       d_cbsz = nxtkoffset(dicdata, d_p+d_keylen, U_REC_L);
       /* checks a key's candidates block length */
       if (d_cbsz < 0 || d_p+d_keylen+d_cbsz > d_len) {
         rrn_etr_flag = 1;
         err_flag = 1;
         break;
       }
       /* Local Copy, Candidates Block */
       memcpy( &d_cb[0] , dicdata+d_p+d_keylen , d_cbsz ) ;
       /* check key code. */
       for (set_cnt = 0 ; set_cnt < d_keylen && err_flag == 0; set_cnt+=2 )   
       {                                                    
	/* if not last character of data key */
  	if (set_cnt < d_keylen - 2)
  	{
  	   if (d_key[set_cnt] & 0x80)
  	   {
  		rrn_req_flag = 1;
  		err_flag = 1;
  	   }
  	   else
  	   {
  		byte = d_key[set_cnt] | 0x80;
  		if ( (byte == 0xa3) || ((byte >= 0xb0) && (byte <= 0xc8)) )
  		{
  		  byte = d_key[set_cnt+1];
  		  if (byte & 0x80)
  		  {
  			if ( byte < 0xa1 || byte > 0xfe )
  			{
  			   rrn_req_flag = 1;
  			   err_flag = 1;
  			}
  		  }
  		  else
  		  {
  			rrn_req_flag = 1;
  			err_flag = 1;
  		  }
  		}
  		else
  		{
  		  rrn_req_flag = 1;
  		  err_flag = 1;
  		}
  	   }
  	}
  	else
  	{
  		byte = d_key[set_cnt];
  		if ( (byte == 0xa3) || ((byte >= 0xb0) && (byte <= 0xc8)) )
  		{
  			byte = d_key[set_cnt+1];
  			if ( byte < 0xa1 || byte > 0xfe )
  			{
  			   rrn_req_flag = 1;
  			   err_flag = 1;
  			}
  		}
  		else
  		{
  		  rrn_req_flag = 1;
  		  err_flag = 1;
  		}
  	}
       }	/* end of for */
      }
      /* data key length is out of range */
      else
      {
       rrn_req_flag = 1;                 /* rrn recover request flag on   */
       err_flag = 1 ;                    /* error flag on                 */
      }
  
    /*
     *   rl Key  data last & now compare
     */
  
     if ( err_flag == 0 )                 /* check  error flag             */
     {
      if ( lst_d_keylen < d_keylen )          /* key  length last & now copare */
      {
       rst = lst_d_keylen ;                  /* set cpmare length   last      */
      }
      else
      {
       rst = d_keylen ;                      /* set compare length  now     */
      }
      /************************/
      /*                      */
      /* In order to compare. */
      /*                      */
      /************************/
      makeksstr(lst_d_keydata, lst_d_keylen);
      makeksstr(d_key, d_keylen); 

      rst = memcmp( &lst_d_keydata[0], &d_key[0], rst ) ;
                                                /* compare last key  data */
      /************************/
      /*                      */
      /* Restore the code.    */
      /*                      */
      /************************/
      mkrbnk(lst_d_keydata, lst_d_keylen);
      mkrbnk(d_key, d_keylen);

      /* ordering test and duplicating test */
      if ( ( rst > 0 ) || ( ( rst == 0 ) && ( lst_d_keylen >= d_keylen ) ))
      {                                         /* result check           */
                                                /* last key  data great   */
       rrn_req_flag = 1 ;                       /* rrn   recover          */
                                                /*    request flag  on    */
       err_flag = 1 ;                           /*  error flag  on        */
      }
     }
  
     lst_d_keylen = d_keylen ;                 /* copy from now length  to   */
                                               /*      last length           */
     if ( err_flag == 0 )
     {
      memcpy(lst_d_keydata, d_key, d_keylen);
      d_cbchkp = 0;
      d_cbp = d_cbchkp;
      if ( ( d_cbsz < U_CB_MN ) || ( d_cbsz > U_CB_MX ))
      {                                   /* dl limit check              */
       rrn_req_flag = 1 ;                 /* rrn recover request flag on */
       err_flag = 1 ;                     /* error flag  on              */
      }
     }
  
     /* checks a key's candidate block start. */
     t_cmp3 = 0 ;                             /* complete flag initialize*/
     while (( err_flag == 0 ) && ( t_cmp3 == 0 )) /*  search error flag &*/
     {                                            /*  complete flag check*/
      if ( err_flag == 0 )
      {
       d_cbcandlen = nxtcandlen(d_cb, d_cbp, &lastcflag, d_cbsz);
       memcpy(d_cbcand, d_cb+d_cbp, d_cbcandlen);	
       /* check a candidate code */
       if (d_cbcand[d_cbcandlen-2] & 0x80)	/* last candidate */
       {
           for(set_cnt = 0 ; set_cnt < d_cbcandlen && err_flag == 0; set_cnt+=2)
           {
  	    if (set_cnt < d_cbcandlen - 2)
	    /* if not last character of last candidate */
  	    {
  	       if (d_cbcand[set_cnt] & 0x80)
  	       {
  		    rrn_etr_flag = 1;
  	    	    err_flag = 1;
  	       }
  	       else
  	       {
  	    	    byte = d_cbcand[set_cnt] | 0x80;
  		    if ( byte >= 0xa1 && byte <= 0xfe )
  		    {
  		      if (d_cbcand[set_cnt+1] & 0x80)
  		      {
  			    rrn_etr_flag = 1;
  		    	    err_flag = 1;
  		      }
  		      else
  		      {
  		      	    byte = d_cbcand[set_cnt+1] | 0x80;
  			    if ( byte < 0xa1 || byte > 0xfe )
  			    {
  			       rrn_etr_flag = 1;
  			       err_flag = 1;
  			    }
  		      }
  		    }
  		    else
  		    {
  		      rrn_etr_flag = 1;
  		      err_flag = 1;
  		    }
  	       }
  	    }
	    /* if last character of last candidate */
  	    else
  	    {
  	   
  	    	byte = d_cbcand[set_cnt];
  		if ( byte >= 0xa1 && byte <= 0xfe )
  		{
  			byte = d_cbcand[set_cnt+1];
  			if ( byte < 0xa1 || byte > 0xfe )
  			{
  			   rrn_etr_flag = 1;
  			   err_flag = 1;
  			}
  		}
  		else
  		{
  		  rrn_etr_flag = 1;
  		  err_flag = 1;
  		}
  	    }
             }  /* for */
        }
        else	/* other than last candidates */
        {
            for(set_cnt = 0 ; set_cnt < d_cbcandlen && err_flag == 0; set_cnt+=2)
            {
    	      if (set_cnt < d_cbcandlen - 2)
	      /* if not last character of non-last candidate */
    	      {
    	         if (d_cbcand[set_cnt] & 0x80)
    	         {
    	  	    rrn_etr_flag = 1;
    	    	    err_flag = 1;
    	         }
    	         else
    	         {
    	      	    byte = d_cbcand[set_cnt] | 0x80;
    	  	    if ( byte >= 0xa1 && byte <= 0xfe )
    	  	    {
    	  	      if (d_cbcand[set_cnt+1] & 0x80)
    	  	      {
    	  		    rrn_etr_flag = 1;
    	  	    	    err_flag = 1;
    	  	      }
    	  	      else
    	  	      {
    	  	      	    byte = d_cbcand[set_cnt+1] | 0x80;
    	  		    if ( byte < 0xa1 || byte > 0xfe )
    	  		    {
    	  		       rrn_etr_flag = 1;
    	  		       err_flag = 1;
    	  		    }
    	  	      }
    	  	    }
    	  	    else
    	  	    {
    	  	      rrn_etr_flag = 1;
    	  	      err_flag = 1;
    	  	    }
    	         }
    	      }
    	      else	/* if last character of non-last candidate */
    	      {
    	   
    		if ( d_cbcand[set_cnt] & 0x80 )
    		{
    		      rrn_etr_flag = 1;
    		      err_flag = 1;
    		}
    		else
    		{
    	    	    byte = d_cbcand[set_cnt] | 0x80;
    		    if ( byte >= 0xa1 && byte <= 0xfe )
    		    {
    			byte = d_cbcand[set_cnt+1];
    			if ( byte < 0xa1 || byte > 0xfe )
    			{
    			   rrn_etr_flag = 1;
    			   err_flag = 1;
    			}
    		    }
    		    else
    		    {
    		      rrn_etr_flag = 1;
    		      err_flag = 1;
    		    }
    		}
    	      }
            }  /* for */
  	}
       }    /* if flag */
  
      if ( err_flag == 0 ) /* check error flag &  */
      {                                              /*    skip flag        */
       if ( ( d_cbcandlen < U_MN_CAN ) || ( d_cbcandlen > U_MX_CAN ))
       {                                         /* limit check             */
                                                 /* candidate data number */
        rrn_etr_flag = 1 ;                       /* rrn entry request flag  */
        err_flag = 1 ;                           /* error flag on    */
       }
      }
  
      /*
       *  renew check pointer
       */
  
      /* advance check pos */
      if ( err_flag == 0 )                       /* check error flag        */
      {
       d_cbchkp += d_cbcandlen ;  		/* normal candidate check      */
       d_cbp = d_cbchkp;
       if ( d_cbsz <= d_cbchkp ) 
       {
        if ( d_cbsz != d_cbchkp )             /* compare dl & check    */
        {                                     /*              pointer  */
         rrn_etr_flag = 1 ;                   /* rrn recover request flag on*/
         err_flag = 1 ;                       /* error flag on              */
        }
        else
        {
         t_cmp3 = 1 ;                       /* <while> complete flag on  */
        }
       }
      }
  
     } /* while , tnp3 */
  
     /* advance data block check pos. */
     if ( err_flag == 0 )
     {
      d_chkp  += (d_keylen+d_cbsz);
      d_p      = d_chkp;
      if ( d_len <= d_chkp )  /* rl check  end           */
      {
       if ( d_len != d_chkp ) /* compare rl & rl check   */
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
     i_chkp += (i_keylen+U_RRNLEN);
     i_p     = i_chkp;
     if ( i_len <= i_chkp )  
     {
      if ( i_len != i_chkp ) /* compare il & il check     */
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
  if ( rrn_etr_flag != 0 )
  {                                       /*      on                      */
   if ( d_cbchkp == 0 )  /* dl check pointer =     */
   {                                            /*       initial  value   */
    rrn_req_flag = 1 ;                    /* rrn recover request flag on  */
   }
   else                                   /* dl check pointer !=          */
   {                                      /*             initial value    */
    d_cbsz = d_cbchkp;
    d_chkp += (d_keylen + d_cbsz) ;  /* renew rl check pointer       */
    rrn_req_flag = 1 ;                    /* rrn recover request flag on  */
   }
  }

  if ( rrn_req_flag != 0 )                /* check rrn recovery request   */
  {                                       /*                     flag     */

   if (recv_flag == U_FOF) {
	return (UDRNEED);
   }
   else if ( d_chkp == U_RLLEN ) /* rl check pointer =      */
   {                                           /*           initial value */
    idx_req_flag = 1 ;                    /* index recover request flag on*/
   }
   else           /* rl check pointer != initial   */
   {                                     /*                        value  */
    d_len = d_chkp;
    /***************************/
    /* Write Data Block Length */
    /***************************/
    setrl(d_wrtptr, d_len);
    i_chkp += (i_keylen+U_RRNLEN); /* renew index check pointer     */
    idx_req_flag = 1 ;             /* index recovery request flag on*/
   } 
  }

  if (recv_flag == U_FOF)
  {
     if (err_flag == 0 && mru_req_flag ==0 &&
		idx_req_flag == 0 && rrn_req_flag == 0 && rrn_etr_flag == 0) {
        return (UDSUCC);
     } else {
	return (UDRNEED);
     }
  }

  hudcread( wrk , (short)3 , NULL ) ;
  har = gethar(wrk->rdptr);
  nar = getnar(wrk->rdptr);
  if ( idx_req_flag != 0 && re_code == U_ST_CD0 ) /* check index recovery request  */
  {                                      /*                      flag     */
   if ( i_chkp == (U_ILLEN+U_HARLEN+U_NARLEN) )    /* index check pointer = initial */
   {                                               /*                       value   */
    if ( ( (har) >= U_HAR_V1 ) && ( (har) <= U_HAR_V2 ) )
    {
     setnar(wrk->rdptr, U_BASNAR); /*  nar  initilize ( 3K )        */
    }

    re_code = U_ST_CD5 ;                 /* set recovery code             */
   }
   else                                  /* index check pointer !=        */
   {                                     /*                initial value  */
/******/
    if (d_chkp == (U_RLLEN))  
    {
     if (pre_rrn == -1)
        setnar(wrk->rdptr, U_BASNAR); /*  nar  initilize ( 3K )        */
     else 
        setnar(wrk->rdptr, pre_rrn+1); 
    }
    else 
     setnar(wrk->rdptr, i_rrn+1); 
/******/
    re_code = U_ST_CD6 ;                 /* set recovery code             */
   }
   i_len = i_chkp;                       /* set index check pointer to il */
   setil( i_wrtptr, i_len);              /* write user dictionary  il     */
  }
 }
 else {
  if (recv_flag == U_FOF) return UDRNEED;
 }

  if ( mru_req_flag == 1 && re_code == U_ST_CD0 )   /* check mru recovery request    */
  {                                   /*                    flag       */
     m_len = m_chkp ; 
     setmrulen(m_wrtptr, m_len);
     re_code = U_ST_CD6 ;              /* set recovery code            */
  }


 /*
  *  set return code
  */

 switch ( re_code )                  /* check recovery code           */
 {
  case U_ST_CD0 :                    /* case  normal end              */
           setudstat(sts_wrtptr, (short)NULL);
           wrk->updflg = U_FON ;            /* update flag   on           */
           ret_code    = UDSUCC ;           /* set normal end code to     */
           break ;                          /*              return code   */

  case U_ST_CD2 :                    /* case  impossible           */
           setudstat(sts_wrtptr, (short)NULL);
           wrk->updflg = U_FON ;            /* update flag   on           */
           ret_code = UDRIMPE ;             /* set recovery impossible to */
           break ;                          /*              return code   */

  case U_ST_CD5 :                    /* case  nothing data            */
           setudstat(sts_wrtptr, (short)NULL);
           wrk->updflg = U_FON ;            /* update flag   on           */
           ret_code    = UDRNVDW ;          /* set nothing data code to   */
           break ;                          /*              return code   */

  case U_ST_CD6 :                    /* case  invalid data            */
           setudstat(sts_wrtptr, (short)NULL);
           wrk->updflg = U_FON ;            /* update flag   on           */
           ret_code    = UDRDPDW ;          /* set invalid data code  to  */
           break ;                          /*              return code   */

  case U_ST_CD7 :                    /* case  invalid status             */
           setudstat(sts_wrtptr, (short)NULL);
           wrk->updflg = U_FON ;            /* update flag   on           */
           ret_code    = UDRDPDW ;           /* set invalid status code to     */
           break ;                          /*              return code   */
 }
 return( ret_code ) ;
}
/*----------------------------------------------------------------------*/
/*                      End of hudicrcv.                                */
/*----------------------------------------------------------------------*/
