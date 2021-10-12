static char sccsid[] = "@(#)99  1.1  src/bos/usr/lpp/kls/dictutil/hudicadh.c, cmdkr, bos411, 9428A410j 5/25/92 14:42:27";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicadh.c
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
 *  Module:       hudicadh.c
 *
 *  Description:  Handle Addition of a key and candidate.
 *
 *  Functions:    hudicadh()
 *
 *  History:      5/22/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include file.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Memory Package                               */
#include "hut.h"        /* Utility Define File                          */

extern void hudickc();


	/* display menu number */

#define         U_AF11           0              /*  field No. f11 (dim) */
#define         U_AF12           1              /*  field No. f12 (dim) */
#define         U_AF13           2              /*  field No. f13 (dim) */
#define         U_AF14           3              /*  field No. f14 (dim) */
#define         U_AF15           4              /*  field No. f15 (dim) */
#define         U_AF16           5              /*  field No. f16 (dim) */
#define         U_AF17           6              /*  field No. f17 (dim) */
#define         U_AF18           7              /*  field No. f18 (dim) */
#define         U_AF19           8              /*  field No. f19 (dim) */
#define         U_AF20           9              /*  field No. f20 (dim) */
#define         U_AF21           10             /*  field No. f21 (dim) */

	/* display menu x coordinate */

#define         U_X12            0              /*  field column f12    */
#define         U_X13            3              /*  field column f13    */
#define         U_X14            3              /*  field column f14    */
#define         U_X15            3              /*  field column f15    */
#define         U_X16            3              /*  field column f16    */
#define         U_X17            3              /*  field column f17    */
#define         U_X18            22             /*  field column f18    */
#define         U_X19            33             /*  field column f19    */
#define         U_X20            U_X14 + 7      /*  field column f20    */
#define         U_X21            U_X15 + 13      /*  field column f21    */

	/* display menu y coordinate */

#define         U_Y11            1              /*  field line   f11    */
#define         U_Y12            3              /*  field line   f12    */
#define         U_Y13            5              /*  field line   f13    */
#define         U_Y14            7              /*  field line   f14    */
#define         U_Y15            9              /*  field line   f15    */
#define         U_Y16            14             /*  field line   f16    */
#define         U_Y17            16             /*  field line   f17    */
#define         U_Y18            16             /*  field line   f18    */
#define         U_Y19            16             /*  field line   f19    */
#define         U_Y20            7              /*  field line   f20    */
#define         U_Y21            9              /*  field line   f21    */

int hudicadh(udcbptr,sdcbptr)

  UDCB  *udcbptr ; /* user dictionary control block pointer   */
  SDCB  *sdcbptr ; /* system dictionary control block pointer */
{
  short   msg_no             ; /* error message number   */
  int     keyflg             ; /* key field select flag */
  long    keylen             ; /* key data length       */
  long    candlen            ; /* cand data length       */
  short   ret_cod1           ; /* return code No. 1      */
  int     ret_cod2           ; /* return code No. 2      */
  int     ret_cod3           ; /* return code No. 3      */
  short   mode               ; /* mode                   */
  int     i                  ; /* loop counter           */
  short   mflag              ; /* check flag             */
  int     err_flg            ; /* error flag             */
  int     add_flg            ; /* addition check flag    */
  int     hu_ret             ; /* utility return code    */
  short   iplen              ;
  uchar   *csdata            ; /* check system dictionary offset   */
  long    cslen              ; /* check system dictionary length   */
  short   wkbuf              ; /* work buffer            */
  union
  {
   uchar  buf1[sizeof(short)] ; /* work buffer characters */
   ushort buf2                ; /* work buffer short      */
  }       wk1, wk2            ; /* work buffer            */

  UECB    *msg_tbl            ; /* message table pointer  */

  /*
   * Initial proces and CRT display
   */

  long   x[11]            ; /* x coordinate of lower left */
  long   y[11]            ; /* y coordinate of lower left */
  long   fld_ln[11]       ; /* fields length             */
  uchar *dsfld[9]         ; /* display fields address    */

  /* blank field */
  static uchar blnk_fld[] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
			      0x00,0x00                           } ;



  /* blank message */
  static uchar blnk_msg[]  = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                               0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                               0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                               0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                               0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                               0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                               0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                               0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			       0x20, 0x20, 0x20, 0x20, 0x20, 0x20 } ;

  char  *msg_ptr;

  /* field no. 11 message */
  static uchar field_f11[40] = "** User Dictionary Registration **";

#define         U_X11           ((U_MAXCOL-strlen(field_f11))/2)   /*  field column f11    */

  /* field no. 13 message */
  static uchar field_f13[40] = "Enter after key and candidate input";


  /* field no. 14 message */
				      /* 20 space */
  static uchar field_f14[40] = "key : [                    ]";

  /* field no. key field */
  uchar keydata[U_KEYFLD];

  /* field no. 15 message */
				                 /* 40 space */
  static uchar field_f15[40] = "candidate : [                    ]";

  /* field no. cand field */
  uchar canddata[U_CAN_MX];

  /* field no. 17 message */
  static uchar field_f17[20] = "Enter = Register";

  /* field no. 18 message */
  static uchar field_f18[20] = "F3 = End";

  /* field no. 19 message */
  static uchar field_f19[20] = "F5 = Clear";


  add_flg = U_FOF ; /* addition flag off */

  keydata[0] = NULL ;

  canddata[0] = NULL ;

  x[U_AF11] = U_X11 + udcbptr->xoff            ; /* set x cordinate f11 */
  x[U_AF12] = U_X12 + udcbptr->xoff            ; /* set x cordinate f12 */
  x[U_AF13] = U_X13 + udcbptr->xoff            ; /* set x cordinate f13 */
  x[U_AF14] = U_X14 + udcbptr->xoff            ; /* set x cordinate f14 */
  x[U_AF15] = U_X15 + udcbptr->xoff            ; /* set x cordinate f15 */
  x[U_AF16] = U_X16 + udcbptr->xoff            ; /* set x cordinate f16 */
  x[U_AF17] = U_X17 + udcbptr->xoff            ; /* set x cordinate f17 */
  x[U_AF18] = U_X18 + udcbptr->xoff            ; /* set x cordinate f18 */
  x[U_AF19] = U_X19 + udcbptr->xoff            ; /* set x cordinate f19 */
  x[U_AF20] = U_X20 + udcbptr->xoff            ; /* set x cordinate f19 */
  x[U_AF21] = U_X21 + udcbptr->xoff            ; /* set x cordinate f19 */

  y[U_AF11] = U_Y11 + udcbptr->yoff            ; /* set y cordinate f11 */
  y[U_AF12] = U_Y12 + udcbptr->yoff            ; /* set y cordinate f12 */
  y[U_AF13] = U_Y13 + udcbptr->yoff            ; /* set y cordinate f13 */
  y[U_AF14] = U_Y14 + udcbptr->yoff            ; /* set y cordinate f14 */
  y[U_AF15] = U_Y15 + udcbptr->yoff            ; /* set y cordinate f15 */
  y[U_AF16] = U_Y16 + udcbptr->yoff            ; /* set y cordinate f16 */
  y[U_AF17] = U_Y17 + udcbptr->yoff            ; /* set y cordinate f17 */
  y[U_AF18] = U_Y18 + udcbptr->yoff            ; /* set y cordinate f18 */
  y[U_AF19] = U_Y19 + udcbptr->yoff            ; /* set y cordinate f19 */
  y[U_AF20] = U_Y20 + udcbptr->yoff            ; /* set y cordinate f19 */
  y[U_AF21] = U_Y21 + udcbptr->yoff            ; /* set y cordinate f19 */

  msg_ptr = catgets(udcbptr->msg_catd, 1, U_F11MSGN, "dummy") ;
  if (strcmp(msg_ptr, "dummy") != 0)
     strncpy(field_f11, msg_ptr, 40) ;
  msg_ptr = catgets(udcbptr->msg_catd, 1, U_F13MSGN, "dummy") ;
  if (strcmp(msg_ptr, "dummy") != 0)
     strncpy(field_f13, msg_ptr, 40) ;
  msg_ptr = catgets(udcbptr->msg_catd, 1, U_F14MSGN, "dummy") ;
  if (strcmp(msg_ptr, "dummy") != 0)
     strncpy(field_f14, msg_ptr, 40) ;
  msg_ptr = catgets(udcbptr->msg_catd, 1, U_F15MSGN, "dummy") ;
  if (strcmp(msg_ptr, "dummy") != 0)
     strncpy(field_f15, msg_ptr, 40) ;
  msg_ptr = catgets(udcbptr->msg_catd, 1, U_F17MSGN, "dummy") ;
  if (strcmp(msg_ptr, "dummy") != 0)
     strncpy(field_f17, msg_ptr, 20) ;
  msg_ptr = catgets(udcbptr->msg_catd, 1, U_F18MSGN, "dummy") ;
  if (strcmp(msg_ptr, "dummy") != 0)
     strncpy(field_f18, msg_ptr, 20) ;
  msg_ptr = catgets(udcbptr->msg_catd, 1, U_F19MSGN, "dummy") ;
  if (strcmp(msg_ptr, "dummy") != 0)
     strncpy(field_f19, msg_ptr, 20) ;

  fld_ln[U_AF11] = strlen(field_f11)           ; /* set field length f11 */
  fld_ln[U_AF12] = strlen(udcbptr->udfname)    ; /* set field length f12 */
  fld_ln[U_AF13] = strlen(field_f13)           ; /* set field length f13 */
  fld_ln[U_AF14] = strlen(field_f14)           ; /* set field length f14 */
  fld_ln[U_AF15] = strlen(field_f15)           ; /* set field length f15 */
  fld_ln[U_AF16] = strlen(blnk_msg)            ; /* set field length f16 */
  fld_ln[U_AF17] = strlen(field_f17)           ; /* set field length f17 */
  fld_ln[U_AF18] = strlen(field_f18)           ; /* set field length f18 */
  fld_ln[U_AF19] = strlen(field_f19)           ; /* set field length f19 */
  fld_ln[U_AF20] = U_KEY_MX		       ; /* set field length f19 */
  fld_ln[U_AF21] = U_CAN_MX		       ; /* set field length f19 */


  dsfld[U_AF11] = field_f11 ; /* set display field address f11        */
  dsfld[U_AF12] = udcbptr->udfname ; /* set display field address f12 */
  dsfld[U_AF13] = field_f13 ; /* set display field address f13        */
  dsfld[U_AF14] = field_f14 ; /* set display field address f14        */
  dsfld[U_AF15] = field_f15 ; /* set display field address f15        */
  dsfld[U_AF16] = blnk_msg  ; /* set display field address f16        */
  dsfld[U_AF17] = field_f17 ; /* set display field address f17 NULL   */
  dsfld[U_AF18] = field_f18 ; /* set display field address f18        */
  dsfld[U_AF19] = field_f19 ; /* set display field address f19        */

   /*****************/
   /* display dlear */
   /*****************/
   putp(clear_screen) ; 

   /*********************************/
   /* define field error check loop */
   /*********************************/
   for(i = U_AF11 ; i <= U_AF19 ; i++) {
        /******************/
	/* display fields */
        /******************/
	ret_cod2 = hudisply(udcbptr, y[i], x[i], dsfld[i], fld_ln[i]);
   }
   msg_no  =  U_ENDID;       
   keylen  =  NULL;       
   candlen =  NULL;   
   keyflg  =  U_FON;     

   /*******************/
   /* Main Loop       */
   /* Acceptable Menu */
   /* 1. Addition     */
   /* 2. Terminate    */
   /* 3. Clear        */
   /*******************/
   while (TRUE) { 
	ret_cod2 = humsg(udcbptr, y[U_AF16], x[U_AF16], msg_no);
	/*
	* input field no. f14 or f16
	*/
	switch(keyflg) { 
	   /*****************************/
	   /* select active input field */ 
	   /*****************************/
	   case U_FON : 
	  	ret_cod3 = huipfld( udcbptr, y[U_AF20], x[U_AF20],
		   keydata, fld_ln[U_AF20], T_KEY, &iplen  , C_SWOFF, HUDICADH) ;
		keylen = iplen ;
		break ;
	   case U_FOF : 
		ret_cod3 = huipfld( udcbptr, y[U_AF21], x[U_AF21],
		   canddata, fld_ln[U_AF21], T_CAND, &iplen  , C_SWOFF, HUDICADH) ;
		candlen = iplen ;
		break ;
	}
	msg_no = U_GMSGN ; 	/* Invalid key has been pressed.          */
	/****************************/
	/* select function No.5 key */
	/* Clear Key and Cand field */
	/****************************/
	if( ret_cod3 == U_PF5KEY ) {
	  keydata[0]  = NULL ;
	  keylen      = NULL ; 
	  canddata[0] = NULL ;
	  candlen     = NULL ; 
	 hudisply(udcbptr,y[U_AF20],x[U_AF20],keydata,fld_ln[U_AF20]);
	 hudisply(udcbptr,y[U_AF21],x[U_AF21],canddata,fld_ln[U_AF21]);
	  keyflg = U_FON  ; 
	  msg_no = U_ENDID ; 
	}
	/****************************/
	/* select function No.3 key */
	/* Returns the Main menu    */
	/****************************/
	if( ret_cod3 == U_PF3KEY ) {
	   if(add_flg == U_FON) {
	   	memcpy(udcbptr->secbuf, udcbptr->dcptr, udcbptr->ufilsz);
	   }
	   /*****************/
	   /* clear display */
	   /*****************/
	   putp(clear_screen) ; 
	   /*********************/
	   /* eixt this routine */
	   /*********************/
	   return( UDSUCC ) ; 
	}

	/********************************/
	/* select enter or new line key */
	/* Now, Key or Cand Accepted.   */
	/********************************/
	if(ret_cod3 == U_ENTERKEY || ret_cod3 == U_CRKEY    ||
	   ret_cod3 == U_TABKEY   || ret_cod3 == U_BTABKEY  ||
	   ret_cod3 == U_C_DOWN   || ret_cod3 == U_C_UP     ||
	   ret_cod3 == U_ACTIONKEY ) 
	{
	   msg_no = U_ENDID ; /* reset message number */
	   err_flg = U_FOF  ; /* error flag off */

	   /* key data length is zero or all blank */
	   if (keylen == NULL) {
		if( ret_cod3 == U_ENTERKEY ||
		    ret_cod3 == U_CRKEY    ||
		    ret_cod3 == U_ACTIONKEY )
		{
		   if(candlen == NULL) {
			msg_no = U_KMSGN ; /* set message number */
					   /* Input Key and its Candidates.    */
		   } else {
			msg_no = U_LMSGN ; /* set message number */
					   /* Input Key.         */
		   }
		} else {
		   msg_no = U_LMSGN ; /* set message number */
				      /* Input Key.         */
		}
		err_flg = U_FON ; /* error flag on */
	   } else if(keyflg == U_FON) {
		/****************************/
		/* Check Is Vaild keydata ? */
		/****************************/

		hudickc(keydata, keylen, &mflag); /* convert hudicymc to hudickc */
		if(keylen <= U_KEY_MX) {
		   if(mflag == U_INVALD) {
			msg_no = U_PMSGN ; /* set message number */
					   /* Invalid characters exist in Key.       */
			err_flg = U_FON ;  /* error flag on */
		   } else if(mflag == U_HNMIX) {
			msg_no = U_ACMSGN ; /* set message number */
					    /* The Key must consists of only hangul or only number */
			err_flg = U_FON ;   /* error flag on */
		   } else {
			msg_no = U_ENDID ; /* set message number */
			err_flg = U_FOF ;   /* error flag off */
		   }
		} else {
		   msg_no = U_AJMSGN ; /* set message number */
				       /* Length of the Key exceeds the limit(10 char) */
		   err_flg = U_FON  ; /* error flag on */
		}
	   }

	   /* hit not action key and no error */
	   if( (( ret_cod3 == U_TABKEY  ) ||
		( ret_cod3 == U_BTABKEY ) ||
		( ret_cod3 == U_C_UP    ) ||
		( ret_cod3 == U_C_DOWN  )    )
		&& (err_flg == U_FOF))
	   {
		switch(keyflg) {
		   /************************/
		   /* exchange input field */
		   /************************/
		   case U_FON : /* case key field */
			keyflg = U_FOF ; /* change to cand field */
			break ;
		   case U_FOF : /* case cand field */
			keyflg = U_FON ; /* change to key field */
			break ;
		}
	   }
	   /*******************************/
	   /* hit action key and no error */
	   /*******************************/
	   if(( ( ret_cod3 == U_ACTIONKEY ) ||
		( ret_cod3 == U_ENTERKEY  ) ||
		( ret_cod3 == U_CRKEY     )    )  && (err_flg == U_FOF))
		{
		   if(candlen == NULL) {
			/*****************************************/
			/* cand data length is zero or all blank */
			/*****************************************/
			msg_no = U_JMSGN ; /* set message number */
					   /* Input Candidate. */
			keyflg = U_FOF;
			err_flg = U_FON ; /* error flag on */
		   }

		   /* no error and same data length and same data */
		   if((keylen == candlen) &&
		      (memcmp(keydata, canddata, (int)keylen) == NULL)) {
			msg_no = U_SMSGN ; /* set message number */
					   /* The Key and the Candidate should not be the same */
			err_flg = U_FON ; /* error flag on */
		   }

		   /* no error */
		   if(err_flg == U_FOF) {	
			/***********************************/
			/* check data by system dictionary */
			/***********************************/
			hudiccs(sdcbptr,(short)keylen,keydata
			   ,(short)candlen,canddata,&ret_cod1) ;
			/**********************************/
			/* already registration this data */ 
			/* in the system dictionary       */
			/**********************************/
			if(ret_cod1 != NULL) {
			   msg_no = U_OMSGN ;  /* set message number */
					       /* Already Exists in System Dictionary.   */
			   err_flg = U_FON ; /* error flag on */
			}
		   }

		/************/
		/* no error */ 
		/************/
		   if(err_flg == U_FOF) {
	   		/* registration this data */
		   	mode = U_REGIST;	/* set registration mode */

	   		ret_cod2 = hudicadp( mode, keydata, keylen, 
	   		   canddata, (int)candlen, udcbptr ) ;

	   		switch(ret_cod2) {
		   	   case UDCANDE  : /* no more registration this key data */
			   	msg_no = U_BJMSGN ; 
	   			break ;
		   	   case UDOVFDLE  : /* no more registration this key data */
			   	msg_no = U_RMSGN ; 
				   	    /* Number of candidates exceeds the limit */
	   			break ;
		   	   case UDDCEXTE  : 
			   	/* already registration this data in user dictionary */
	   			msg_no = U_NMSGN ; 
		   			    /* Already Exists in User Dictionary.     */
			   	break ;
	   		   case UDDCFULE  : /* full the user dictionary */
		   		msg_no = U_QMSGN ; 
				    /* The dictionary file size exceeds the limit */
			   	break ;
	   		   case UDSUCC    : /* success registration */
		   		add_flg = U_FON ;        /* addition flag on */
			   	udcbptr->updflg = U_FON ; /* update flag on  */
	   			wk1.buf2 = keylen ; /* separate key length */
		   		wk2.buf2 = candlen ; /* separate candidate length */
			   	/* addition MRU area */

	   			hudcmrua(udcbptr->dcptr, keydata, keylen, canddata, candlen) ;

		   		/* save dictionary data to temporaly file   */
			   	hu_ret = hutmwrt( udcbptr->tmpname,
			        udcbptr->dcptr, udcbptr->ufilsz );
               			if( hu_ret == IUSUCC ) {
	          		   msg_no = U_MMSGN ; /* set message number */
					      /* The registration has been completed.   */
		               	} else {
        			   msg_no= U_AGMSGN;
					/* Can not access user dictionary.   */
        			   udcbptr->updflg = U_FOF;
		        	}
			        break ;
		        }
	        /*******************************/
	        /* hit action key and no error */
	        /*******************************/
		  }
              }
	   /********************************/
	   /* select enter or new line key */
	   /* Now, Key or Cand Accepted.   */
	   /********************************/
	   }
	/*************/
	/* Main Loop */
	/*************/
	}
}
