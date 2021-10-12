static char sccsid[] = "@(#)20	1.5.1.1  src/bos/usr/lpp/jls/dictutil/kudicadh.c, cmdKJI, bos411, 9428A410j 7/23/92 01:05:03";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicadh
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kudicadh
 *
 * DESCRIPTIVE NAME:    user dictionary addition handler
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
 * FUNCTION:            user dictionary addition handler
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        4956 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         Module Entry Point Name
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicadh(udcbptr, sdcbptr )
 *
 *  INPUT:              udcbptr  : pointer to UDCB Control Block.
 *                      sdcbptr  : pointer to SDCB Control Block.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      UDSUCC :Success of Execution.
 *
 * EXIT-ERROR:          Wait State Codes.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              kustrl
 *                              kuwaku
 *                      Kanji Project Subroutines.
 *                              kudicadp
 *                              kuhkfc
 *                              kudiccs
 *                              kudicycr
 *                              kudicmrc
 *                              kudcmrua
 *                              kudispl
 *                              kuconvm
 *                      Standard Liblary.
 *                              strcspn
 *                              memcpy
 *                              memcmp
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

#include <locale.h>     /*                                              */
#include <nl_types.h>   /*                                              */
#include <limits.h>     /*                                              */


/*
 *      include Kanji Project.
 */
#include "kut.h"        /* Utility Define File                          */
#include "kje.h"        /* Utility Define File                          */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

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

#define         U_X11            13             /*  field column f11    */
#define         U_X12            0              /*  field column f12    */
#define         U_X13            2              /*  field column f13    */
#define         U_X14            2              /*  field column f14    */
#define         U_X15            2              /*  field column f15    */
#define         U_X16            0              /*  field column f16    */
#define         U_X17            0              /*  field column f17    */
#define         U_X18            12             /*  field column f18    */
#define         U_X19            21             /*  field column f19    */
#define         U_X20            10             /*  field column f20    */
#define         U_X21            10             /*  field column f21    */

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

#define         U_FLEN11        28              /* field no.11 length   */
#define         U_FLEN12        54              /* field no.12 length   */
#define         U_FLEN13        40              /* field no.13 length   */
#define         U_FLEN14        30              /* field no.13 length   */
#define         U_FLEN15        50              /* field no.15 length   */
#define         U_FLEN16        54              /* field no.18 length   */
#define         U_FLEN17        10              /* field no.18 length   */
#define         U_FLEN18         7              /* field no.18 length   */
#define         U_FLEN19         9              /* field no.19 length   */
#define         U_FLEN20         20             /* field no.20 length   */
#define         U_FLEN21         40             /* field no.21 length   */

#if defined(CNVEVT)
extern  int     cnvflg;         /* Conversion Type Code                 */
char    cnvdt[U_MEMSIZ];        /* Conversion Output Data Area          */
size_t     ilen;                /* Conversion Input Length              */
size_t     olen;                /* Conversion Output Buffer Length      */
size_t     cnvlen;              /* Conversion Output Length             */
#endif

int kudicadh(udcbptr,sdcbptr)

  UDCB  *udcbptr ; /* user dictionary control block pointer   */
  SDCB  *sdcbptr ; /* system dictionary control block pointer */
{
  void     kuconvm();
  short    msg_no             ; /* error message number   */
  int      yomiflg            ; /* yomi field select flag */
  long     yomilen            ; /* yomi data length       */
  long     kjlen              ; /* goku data length       */
  short    hklen              ; /* goku data length(conv) */
  long     ret_cod1           ; /* return code No. 1      */
  int      ret_cod2           ; /* return code No. 2      */
  int      ret_cod3           ; /* return code No. 3      */
  short    mode               ; /* mode                   */
  short    kanalen            ; /* kana length            */
  uchar    kanadata[U_YOMILN] ; /* kana data area         */
  uchar    hkdata[U_KJFLD]    ; /* goku data area (conv)  */
  int      i                  ; /* loop counter           */
  short    mflag              ; /* check flag             */
  int      err_flg            ; /* error flag             */
  int      add_flg            ; /* addition check flag    */
  long     cslen              ; /* check system dictionary*/
                                /* length                 */
  int      ku_ret             ; /* utility return code    */
  short    iplen              ;
  uchar   *csdata             ; /* check system dictionary*/
                                /* offset                 */
  ushort   wkbuf              ; /* work buffer            */
  union
  {
   uchar  buf1[sizeof(short)] ; /* work buffer characters */
   ushort buf2                ; /* work buffer short      */
  }       wk1, wk2            ; /* work buffer            */

  /*
   * Initial proces and CRT display
   */

  long   x[11]            ; /* x cordinate of lower left */
  long   y[11]            ; /* y cordinate of lower left */
  long   fld_ln[11]       ; /* fields length             */
  uchar *dsfld[9]         ; /* display fields address    */

  /* blank message */
  static uchar *blnk_msg;

  /* field no. 11 message */
  static uchar *field_f11;

  /* field no. 13 message */
  static uchar *field_f13;

  /* field no. 14 message */
  static uchar *field_f14;

  /* field no. yomi field */
  uchar yomidata[U_YOMFLD];

  /* field no. 15 message */
  static uchar *field_f15;

  /* field no. goku field */
  uchar kjdata[U_GOKFLD];

  /* field no. 17 message */
  static uchar *field_f17;

  /* field no. 18 message */
  static uchar *field_f18;

  /* field no. 19 message */
  static uchar *field_f19;


      blnk_msg = CU_MNBLNK;
      field_f11 = CU_MNATIT;
      field_f13 = CU_MNYGEM;
      field_f14 = CU_MNYOMI;
      field_f15 = CU_MNGOKU;
      field_f17 = CU_MNKEYE;
      field_f18 = CU_MNKEY3;
      field_f19 = CU_MNKEY5;


  yomidata[0] = NULL ;

  kjdata[0] = NULL ;

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

  fld_ln[U_AF11] = U_FLEN11                    ; /* set field length f11 */
  fld_ln[U_AF12] = U_FLEN12                    ; /* set field length f12 */
  fld_ln[U_AF13] = U_FLEN13                    ; /* set field length f13 */
  fld_ln[U_AF14] = U_FLEN14                    ; /* set field length f14 */
  fld_ln[U_AF15] = U_FLEN15                    ; /* set field length f15 */
  fld_ln[U_AF16] = U_FLEN16                    ; /* set field length f16 */
  fld_ln[U_AF17] = U_FLEN17                    ; /* set field length f17 */
  fld_ln[U_AF18] = U_FLEN18                    ; /* set field length f18 */
  fld_ln[U_AF19] = U_FLEN19                    ; /* set field length f19 */
  fld_ln[U_AF20] = U_FLEN20                    ; /* set field length f19 */
  fld_ln[U_AF21] = U_FLEN21                    ; /* set field length f19 */


  dsfld[U_AF11] = field_f11 ; /* set display field address f11        */
  dsfld[U_AF12] = udcbptr->udfname ; /* set display field address f12 */
  dsfld[U_AF13] = field_f13 ; /* set display field address f13        */
  dsfld[U_AF14] = field_f14 ; /* set display field address f14        */
  dsfld[U_AF15] = field_f15 ; /* set display field address f15        */
  dsfld[U_AF16] = blnk_msg  ; /* set display field address f16        */
  dsfld[U_AF17] = field_f17 ; /* set display field address f17 NULL   */
  dsfld[U_AF18] = field_f18 ; /* set display field address f18        */
  dsfld[U_AF19] = field_f19 ; /* set display field address f19        */

/* B.EXTCUR */
	clear();
	refresh();
/* E.EXTCUR */

  for(i = U_AF11 ; i <= U_AF19 ; i++) /* define field error check loop */
  {
    /* display fields */
    ret_cod2 = kudisply( udcbptr, y[i], x[i], dsfld[i], fld_ln[i] ) ;
  }


  msg_no  =  U_ENDID  ;       /*      set message number          */
  yomilen =  NULL     ;       /*      initial yomi length         */
  kjlen   =  NULL     ;       /*      initial goku length         */
  yomiflg =  U_FON    ;       /*      set yomi flag               */

  /*
   *   main loop
   */

  /* set conversion mode        */
  kuconvm( (short)(U_FON) );

  while (TRUE) /* endless loop */
  {
  /*
   * error message display
   */

    ret_cod2 = kumsg( udcbptr, y[U_AF16], x[U_AF16], msg_no ) ;

  /*
   * input field no. f14 or f16
   */
    switch(yomiflg) /* select active input field */
    {
      case U_FON : /* yomi field active */

		   ret_cod3 = kuipfld( udcbptr, y[U_AF20], x[U_AF20],
				       yomidata, fld_ln[U_AF20],
				       T_YOMI, &iplen, C_SWOFF, cnvflg ) ;
		   yomilen = iplen ;
		   yomidata[yomilen] = '\0';
                   break ;

      case U_FOF : /* goku field active */

		   ret_cod3 = kuipfld( udcbptr, y[U_AF21], x[U_AF21],
				       kjdata, fld_ln[U_AF21],
				       T_GOKU, &iplen, C_SWOFF, cnvflg ) ;
		   kjlen = iplen ;
		   kjdata[kjlen] = '\0';
                   break ;
    }

    msg_no = U_GMSGN ; /* set invalid key message number */

  /*
   *  select function No.5 key
   */

    /* hit function No.5 key */
    if( ret_cod3 == U_PF5KEY )
    {
      yomidata[0] = NULL ;
      yomilen = NULL ; /* initial yomi length */

      kjdata[0] = NULL ;
      kjlen = NULL ; /* initial goku length */

      kudisply( udcbptr, y[U_AF20], x[U_AF20],
		yomidata, fld_ln[U_AF20] ) ;

      kudisply( udcbptr, y[U_AF21], x[U_AF21],
		kjdata , fld_ln[U_AF21] ) ;

      yomiflg = U_FON  ; /* reset yomi mode */
      msg_no = U_ENDID ; /* reset message number */
    }

  /*
   *  select function No.3 key
   */

    /* hit function No.3 key */
    if( ret_cod3 == U_PF3KEY ) {
      if(add_flg == U_FON) {
        memcpy(udcbptr->secbuf, udcbptr->dcptr, udcbptr->ufilsz) ;
      };

/* B.EXTCUR */
	clear();
	refresh();
/* E.EXTCUR */

      return( UDSUCC ) ; /* eixt this routine */
    };

  /*
   *  select enter or new line key
   */

    /* hit enter or new line key */
    if( ret_cod3 == U_ENTERKEY ||
	ret_cod3 == U_CRKEY    ||
	ret_cod3 == U_TABKEY   ||
	ret_cod3 == U_BTABKEY  ||
	ret_cod3 == U_C_DOWN   ||
	ret_cod3 == U_C_UP     ||
	ret_cod3 == U_ACTIONKEY )
    {
      msg_no = U_ENDID ; /* reset message number */
      err_flg = U_FOF  ; /* error flag off */

      /* yomi data length is zero or all blank */
      if(yomilen == NULL) {
	if( ret_cod3 == U_ENTERKEY ||
	    ret_cod3 == U_CRKEY    ||
	    ret_cod3 == U_ACTIONKEY )
        {
          if(kjlen == NULL)
          {
            msg_no = U_KMSGN ; /* set message number */
          }
          else
          {
            msg_no = U_LMSGN ; /* set message number */
          }
        }  
        else
        {
          msg_no = U_LMSGN ; /* set message number */
        }
        err_flg = U_FON ; /* error flag on */
      }

 
      /* yomi data length is not zero and selected  yomi field */
      else if(yomiflg == U_FON)
      {
        /* convert PC -> 7 bit data */
        kudicymc(yomidata, yomilen, kanadata, &wkbuf, &mflag) ;
        if(wkbuf <= U_YMAX) /* kana length max check */
        {
          kanalen = wkbuf ;
          /* mixed kana, katakana, capital, small letter */
          if((mflag == U_KATA) || (mflag == U_CAPOFF))
          {
            /* convert 7bit data -> PC */
            kudicycr(kanadata, kanalen, yomidata, &wkbuf) ;
            yomilen = wkbuf ;
            /* define yomi field */
	    if(cnvflg == U_EUC){
		char	eucstr[100];
		size_t	pclen, euclen;
		euclen = 100;
		pclen = yomilen;
		kjcnvste(yomidata, &pclen, eucstr, &euclen);
		eucstr[100 - euclen] = '\0';
		ret_cod2 = kudisply( udcbptr, y[U_AF20], x[U_AF20],
				 eucstr, fld_ln[U_AF20] ) ;
	    }else{
		ret_cod2 = kudisply( udcbptr, y[U_AF20], x[U_AF20],
				 yomidata, fld_ln[U_AF20] ) ;
	    }
          }

          /* irregular character */
          if(mflag == U_INVALD)
	  {
            msg_no = U_PMSGN ; /* set message number */
            err_flg = U_FON ; /* error flag on */
          }
          else
          /* mix EISUU and KANA */
          if(mflag == U_HEMIX)
          {
            msg_no = U_ACMSGN ; /* set message number */
            err_flg = U_FON ;   /* error flag on */
          }

          /* top of data is chouon */
          if((kanadata[0] == U_CHOUON) || (kanadata[0] == U_OMIT_C))
          {
            msg_no = U_PMSGN ; /* set message number */
            err_flg = U_FON ; /* error flag on */
          }
        }
        else
        {
          msg_no = U_AJMSGN ; /* set message number */
          err_flg = U_FON  ; /* error flag on */
        }
      }

      /* hit not action key and no error */
      if( (  ( ret_cod3 == U_TABKEY  ) || ( ret_cod3 == U_BTABKEY ) ||
	     ( ret_cod3 == U_C_UP    ) || ( ret_cod3 == U_C_DOWN  )    )
       && (err_flg == U_FOF))
      {
        switch(yomiflg) /* exchange input field */
        {
          case U_FON : /* case yomi field */
                       yomiflg = U_FOF ; /* change to goku field */
                       break ;

          case U_FOF : /* case goku field */
                       yomiflg = U_FON ; /* change to yomi field */
                       break ;
        }
      }
      /* hit action key and no error */
      if((  ( ret_cod3 == U_ACTIONKEY ) ||
	    ( ret_cod3 == U_ENTERKEY  ) ||
	    ( ret_cod3 == U_CRKEY     )    )  && (err_flg == U_FOF))
      {
        if(kjlen == NULL) /* goku data length is zero or all blank */
        {
          msg_no = U_JMSGN ; /* set message number */
	  yomiflg = U_FOF;
          err_flg = U_FON ; /* error flag on */
        }

        /* no error and same data length and same data */
        if((err_flg == U_FOF) && (yomilen == kjlen) &&
           (memcmp(yomidata, kjdata, (int)yomilen) == NULL))
        {
          msg_no = U_SMSGN ; /* set message number */
          err_flg = U_FON ; /* error flag on */
        }

        /* no error */
        if(err_flg == U_FOF)
        {
          /* convert check kjdata */
          ret_cod2  = kuhkfc( kanadata, kanalen, kjdata, kjlen, hkdata,
                             &hklen ) ;

          if(ret_cod2 != U_DIVHK)  /* yomi and goku are same */
          {
            cslen = hklen ; /* set hklen */
            csdata = hkdata ; /* set convert data */
          }
          else
          {
            cslen = kjlen ; /* set kjlen */
            csdata = kjdata ; /* set goku data */
          }

          /* check data by system dictionary */
#if defined(_OLD_AIX320)
          kudiccs( sdcbptr, kanalen, kanadata,
				cslen, csdata, &ret_cod1 );
#else
          kudiccs3( sdcbptr, kanalen, kanadata,
				cslen, csdata, &ret_cod1 );
#endif /* defined(_OLD_AIX320) */
          /* already registration this data in the system dictionary */
          if(ret_cod1 != NULL) {
            	msg_no  = U_OMSGN;  /* set message number */
            	err_flg = U_FON; /* error flag on */
          }
	}

        /* no error */
        if(err_flg == U_FOF)
        {
          mode = U_REGIST ;     /* set registration mode */

	  /* display a message ( adding . . . )		*/
    	  ret_cod2 = kumsg( udcbptr, y[U_AF16], x[U_AF16], U_ADDING ) ;

    	  /* registration this data 			*/
          ret_cod2 = kudicadp( mode, kanadata, kanalen, kjdata, (int)kjlen,
                               udcbptr ) ;
          switch(ret_cod2)
          {
            case UDOVFDLE  : /* no more registration this yomi data */
                             msg_no = U_RMSGN ; /* set message number */
                             break ;

            case UDDCEXTE  : /* already registration this data in user
                                dictionary                             */
                             msg_no = U_NMSGN ; /* set message number */
                             break ;

            case UDDCFULE  : /* full the user dictionary */
                             msg_no = U_QMSGN ; /* set message number */
                             break ;

            case UDSUCC    : /* success registration */
                             add_flg = U_FON ;        /* addition flag on */
			     udcbptr->updflg = U_FON ; /* update flag on  */
                             wk1.buf2 = kanalen ; /* separate kana length */
                             wk2.buf2 = kjlen ; /* separate goku length */
                             /* addition MRU area */
                             kudcmrua( udcbptr->dcptr,    kanadata,
                                       wk1.buf1[U_CHLOW], kjdata,
                                       wk2.buf1[U_CHLOW] ) ;
			     /* save dictionary data to temporaly file   */
			     ku_ret = kutmwrt( udcbptr->orgfd, udcbptr->dcptr, 
					       udcbptr->ufilsz );
			     if ( ku_ret == IUSUCC ) {
				msg_no = U_MMSGN ; /* set message number */
			     }
			     else {
      				if(add_flg == U_FON) {
        			    memcpy( udcbptr->secbuf, 
					udcbptr->dcptr, udcbptr->ufilsz );
      				}
				udcbptr->updflg = U_FOF;
				clear();
				refresh();
      				return( UDWRITEE ); /* eixt this routine */
			     }
                             break;
          }
        }
      }
    }
  }
}
