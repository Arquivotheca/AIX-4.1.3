static char sccsid[] = "@(#)01  1.2  src/bos/usr/lpp/kls/dictutil/hudiccom.c, cmdkr, bos411, 9428A410j 11/30/93 16:33:58";
/*
 * COMPONENT_NAME:	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS:		hudiccom.c
 *
 * ORIGINS:		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hudiccom.c
 *
 *  Description:  Handle Combination of a key and candidate.
 *
 *  Functions:    hudiccom()
 *                hunewdc()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Memory Package                               */
#include <fcntl.h>      /* File Control Package                         */
#include <unistd.h>     /*                                              */
#include <sys/stat.h>   /*                                              */
#include <errno.h>      /* Standard Error Number Package                */
#include <string.h>     /* Stirng Package                               */
#include "hut.h"        /* Utility Define File                          */

			   /* DISPLAY Position */
#define   MSG_lin          ( 14)
#define   MSG_col          (  3)
#define   MNU_line         ( 17)
#define   MNU_col          ( U_MAXCOL )

#define   M_L1             (  1)
#define   M_L2             (  4)
#define   M_L3             (  5)
#define   M_L4             (  7)
#define   M_L5             (  8)
#define   M_L6             (  9)
#define   M_L7             ( 10)
#define   M_L8             ( 11)
#define   M_L9             ( 12)
#define   M_L10            ( 16)

#define   M_C2             (  3)
#define   M_C3             (  3)
#define   M_C4             (  3)
#define   M_C5             (  5)
#define   M_C6             (  3)
#define   M_C7             (  5)
#define   M_C8             (  3)
#define   M_C9             (  5)
#define   M_C10            (  3)

#define   M_IX0            (  0)
#define   M_IX1            (  1)
#define   M_IX2            (  2)
#define   M_IX3            (  3)
#define   M_IX4            (  4)
#define   M_IX5            (  5)
#define   M_IX6            (  6)
#define   M_IX7            (  7)
#define   M_IX8            (  8)
#define   M_IX9            (  9)
#define   M_IX10           ( 10)

#define   M_FIELD          ( 12)
#define   DICNAM_L         (  2)
#define   DICNAM_C         (  0)

#define   POSI_1           (  1)
#define   POSI_2           (  2)
#define   POSI_3           (  3)
#define   LNGMAX           ( 44)
#define   NAMESIZ          ( LNGMAX+1 )


#define   DSIZE_L          ( 7168  )   /* add Dictionary LOW          */
#define   DSIZE_H          ( 54272 )   /* add Dictionary HIGH         */
#define   DIVD             ( 1024   )   /* Dictionary Size             */
#define   STSAD            ( 0   )   /* states of data set position */


/*----------------------------------------------------------------------*/
/*                      Begining of hudiccom.                           */
/*----------------------------------------------------------------------*/
int hudiccom ( udcptr, sdcptr, wkname  )

  UDCB        *udcptr     ;     /* user   dictionary file control block */
  SDCB        *sdcptr     ;     /* system dictionary file control block */
  uchar       *wkname     ;     /* user Dictionary name        */

{

  int          fatal_f    ; /* fatal error flag                         */
  int          wait_flg   ; /* waitting    flag                         */
  int          user_flg   ; /* user dictionary file check  error flag   */
  int          flag       ; /* temp file dopy flag                      */
  int          alleng     ;

  uchar       *calloc()   ;
  void         perror()   ;
  int          hunewdc()  ;
  int          hufnc();     /* check file name  */

  int          sw         ; /* main loop escape switch                  */
  int          s_sw       ; /* standard switch                          */
  int          i, j       ; /* loop counter                             */

  static short stsno = 0x0000; /* states of data set buffer                */
  struct stat  stbuf      ; /* file status work buffer                  */
  struct stat  stadd      ; /* file status work buffer                  */
  struct stat  stnew      ; /* file status work buffer                  */
  struct stat  storg      ; /* file status work buffer                  */

  int          ufldes     ; /* user   dictionary file descrdicindexer        */
  int          adfdes     ; /* addition dictionary file descrdicindexer      */
  int          newdes     ; /* new      dictionary file descrdicindexer      */
  int          tmpdes     ;
  int          opn_ret1   ; /* file open descrdicindexer                     */
  int          opn_ret2   ; /* file open descrdicindexer                     */
  int          sfldes     ; /* system dictionary file descrdicindexer        */
  int          mfldes     ; /* message file descrdicindexer                  */
  short        msg_no     ; /* message number buffer                    */
  uchar       *dcptr      ; /* pointer to base address                  */
  uchar       *bak_dcptr  ; /* pointer to base address                  */
  uchar       *ad_dcptr   ; /* pointer to base address                  */

  int          de_ret     ; /* editor return code                       */
  int          hu_ret     ; /* utility return code                      */
  int          re_cod     ; /* standard return code                     */
  int          rc         ; /* standard return code                     */

  int          worklen    ; /* work buffer                              */
  char        *workbf     ; /* work string buffer                       */
  char        *tmpbf      ; /* work string buffer                       */
  int          namesz     ; /* user dictionary file name size           */

  char        *pas_ptr    ; /* user dictionary name address             */
  char        *rc_ptr     ; /* return code                              */
  char        file_n[50]  ; /* dictionary name the origin               */
  long        addlen      ; /* addition dictionary file size            */
  long        newlen      ; /* new      dictionary file size            */
  char        *fname      ; /* new dictionary file name                 */



  short       lin[M_FIELD] ; /* line cordnate                          */
  short       clm[M_FIELD] ; /* column cordnate                        */
  uchar       *dspfld[M_FIELD] ; /* display field address               */

  int          lin_posi   ; /* display line position                    */
  int          clm_posi   ; /* display col  position                    */
  int          datalen    ; /* input data length                        */
  int          in_mod     ;
  char        *disdata    ; /* input data                               */
  short        fld_len1   ; /* input data  by addtion dictionary        */
  short        fld_len2   ; /* input data  by new     dictionary        */
  short       *iplen      ;
  int          posi       ;

  char         add_fld[NAMESIZ];
  char         new_fld[NAMESIZ];
  char         new_buf[NAMESIZ];

  static  UDCB adcptr     ;    /* add dictionary control block          */

  static char fcopy[200];
  static char  wrtsts = 0xf1 ; /* write status code buffer              */
  static char  cmpsts = 0x00 ; /* complate status code buffer           */

  char *msg_ptr;

  /* menu title name  for display */

  /* menu display  " 1 "   */
  static uchar fld_f1[40]  = "** User Dictionary Combination **";

#define   M_C1             ( (U_MAXCOL - strlen(fld_f1)) / 2 )

  /* menu display  " 2 "   */
  static uchar fld_f2[50]  = "This function combines two user dictionaries";

  /* menu display  " 3 "   */
  static uchar fld_f3[30]  = "Enter the file names";


  /* menu display  " 4 "   */
  static uchar fld_f4[30]  = "Original User Dictionary";


  /* menu display  " 5 "   */
  static uchar fld_f5[]  = " [                                            ]";


  /* menu display  " 6 "   */
  static uchar fld_f6[30]  = "Additional User Dictionary";

  /* menu display  " 7 "   */
  static uchar fld_f7[]  = " [                                            ]";


  /* menu display  " 8 "   */
  static uchar fld_f8[30]  = "New User Dictionary";


  /* menu display  " 9 "   */
  static uchar fld_f9[]  = " [                                            ]";

  /* menu display  "10 "   */
  static uchar fld_f10[30]  = "Enter = Combine  F3 = End";


  /* erase message */
  static uchar spcmsg[]     = {
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
			      0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };


  namesz = strlen(wkname);
  if (namesz > LNGMAX) namesz = LNGMAX;
  memcpy( &fld_f5[2], wkname, namesz ) ;


   lin[M_IX1]  = udcptr->yoff + M_L1  ; /* set line cordinate f1       */
   lin[M_IX2]  = udcptr->yoff + M_L2  ; /* set line cordinate f2       */
   lin[M_IX3]  = udcptr->yoff + M_L3  ; /* set line cordinate f3       */
   lin[M_IX4]  = udcptr->yoff + M_L4  ; /* set line cordinate f4       */
   lin[M_IX5]  = udcptr->yoff + M_L5  ; /* set line cordinate f5       */
   lin[M_IX6]  = udcptr->yoff + M_L6  ; /* set line cordinate f6       */
   lin[M_IX7]  = udcptr->yoff + M_L7  ; /* set line cordinate f7       */
   lin[M_IX8]  = udcptr->yoff + M_L8  ; /* set line cordinate f8       */
   lin[M_IX9]  = udcptr->yoff + M_L9  ; /* set line cordinate f9       */
   lin[M_IX10] = udcptr->yoff + M_L10 ; /* set line cordinate f10      */

   clm[M_IX1]  = udcptr->xoff + M_C1  ; /* set column cordinate f1     */
   clm[M_IX2]  = udcptr->xoff + M_C2  ; /* set column cordinate f2     */
   clm[M_IX3]  = udcptr->xoff + M_C3  ; /* set column cordinate f3     */
   clm[M_IX4]  = udcptr->xoff + M_C4  ; /* set column cordinate f4     */
   clm[M_IX5]  = udcptr->xoff + M_C5  ; /* set column cordinate f5     */
   clm[M_IX6]  = udcptr->xoff + M_C6  ; /* set column cordinate f6     */
   clm[M_IX7]  = udcptr->xoff + M_C7  ; /* set column cordinate f7     */
   clm[M_IX8]  = udcptr->xoff + M_C8  ; /* set column cordinate f8     */
   clm[M_IX9]  = udcptr->xoff + M_C9  ; /* set column cordinate f9     */
   clm[M_IX10] = udcptr->xoff + M_C10 ; /* set column cordinate f10    */

   msg_ptr = catgets(udcptr->msg_catd, 1, U_F1MSGN, "dummy");
   if (strcmp(msg_ptr, "dummy") !=0)
      strncpy(fld_f1, msg_ptr, 40);
   msg_ptr = catgets(udcptr->msg_catd, 1, U_F2MSGN, "dummy");
   if (strcmp(msg_ptr, "dummy") !=0)
      strncpy(fld_f2, msg_ptr, 50);
   msg_ptr = catgets(udcptr->msg_catd, 1, U_F3MSGN, "dummy");
   if (strcmp(msg_ptr, "dummy") !=0)
      strncpy(fld_f3, msg_ptr, 30);
   msg_ptr = catgets(udcptr->msg_catd, 1, U_F4MSGN, "dummy");
   if (strcmp(msg_ptr, "dummy") !=0)
      strncpy(fld_f4, msg_ptr, 30);
   msg_ptr = catgets(udcptr->msg_catd, 1, U_F6MSGN, "dummy");
   if (strcmp(msg_ptr, "dummy") !=0)
      strncpy(fld_f6, msg_ptr, 30);
   msg_ptr = catgets(udcptr->msg_catd, 1, U_F8MSGN, "dummy");
   if (strcmp(msg_ptr, "dummy") !=0)
      strncpy(fld_f8, msg_ptr, 30);
   msg_ptr = catgets(udcptr->msg_catd, 1, U_F10MSGN, "dummy");
   if (strcmp(msg_ptr, "dummy") !=0)
      strncpy(fld_f10, msg_ptr, 30);
   dspfld[M_IX1]  = fld_f1  ;           /* set field address f1        */
   dspfld[M_IX2]  = fld_f2  ;           /* set field address f2        */
   dspfld[M_IX3]  = fld_f3  ;           /* set field address f3        */
   dspfld[M_IX4]  = fld_f4  ;           /* set field address f4        */
   dspfld[M_IX5]  = fld_f5  ;           /* set field address f5        */
   dspfld[M_IX6]  = fld_f6  ;           /* set field address f6        */
   dspfld[M_IX7]  = fld_f7  ;           /* set field address f7        */
   dspfld[M_IX8]  = fld_f8  ;           /* set field address f8        */
   dspfld[M_IX9]  = fld_f9  ;           /* set field address f9        */
   dspfld[M_IX10] = fld_f10 ;           /* set field address f10       */

   fatal_f  = IUSUCC;
   msg_no   = U_ENDID;


   /* user dictionary back up to memory  */
   bak_dcptr = calloc( udcptr->ufilsz, sizeof(uchar) ) ;

   /* copy dictionary */
   memcpy( bak_dcptr, udcptr->dcptr, udcptr->ufilsz );



/*****************************************************/
/*                DISPLAY MENU                       */
/*****************************************************/

   putp( clear_screen );                     /* clear display      */

   for( i = M_IX1 ; i <= M_IX10 ; i++ )      /* display field loop */
   {
	/* display field      */

	hu_ret = hudisply( udcptr, lin[i], clm[i],
			   dspfld[i], strlen(dspfld[i]));
   }

   hu_ret = hudisply( udcptr,udcptr->yoff+DICNAM_L,
			      udcptr->xoff+DICNAM_C,
			   udcptr->udfname, U_MAXCOL );
   fflush( stdout ) ;

/*****************************************************/
/*                MAIN   LOOP                        */
/*****************************************************/

/*
 *  INITALAZE
 */
   add_fld[0] = new_fld[0] = NULL;
   fld_len1   = fld_len2   = 0;

   clm_posi = M_C5 + 2 ;
   posi     = POSI_1;                  /* position swich              */
   iplen    = &fld_len1;
   wait_flg = U_FOF;
   user_flg = U_FOF;
   msg_no   = U_ENDID;

   for( sw = TRUE;  sw == TRUE;    )
   {

     /*************    KEY  IN  LOOP    *************/

      switch( posi )
      {
	  case  POSI_1 :
		  lin_posi = M_L7 ;
		  posi     = POSI_2 ;
		  iplen    = &fld_len1;
		  disdata  = add_fld ;
		break ;


	  case  POSI_2 :
		  lin_posi = M_L9 ;
		  posi     = POSI_1 ;
		  iplen    = &fld_len2;
		  disdata  = new_fld ;
		break ;

	  default :
		  if( lin_posi == M_L7 )
		  {
		      lin_posi = M_L9 ;
		      iplen    = &fld_len2;
		      disdata  = new_fld ;
		  }
		  else
		  {
		      lin_posi = M_L7 ;
		      iplen    = &fld_len1;
		      disdata  = add_fld ;
		  }
		  break ;
      }


      /* Display Error Massage */
      hu_ret = humsg( udcptr, udcptr->yoff+MSG_lin,
			       udcptr->xoff+MSG_col, msg_no ) ;
      fflush( stdout ) ;

      if( wait_flg == U_FOF )
      {

	 /*   INPUT FIELD    */
	 hu_ret = huipfld( udcptr, (short)(udcptr->yoff+lin_posi),
			   (short)(udcptr->xoff+clm_posi),
			   disdata, (short)LNGMAX,
			   (short)T_FILE, iplen , C_SWOFF, HUDICCOM);

	 add_fld[fld_len1] = NULL;
	 new_fld[fld_len2] = NULL;
      }
      else
      {
	 /* waitting for triger key */
	 for( ; ; )
	 {
	    hu_ret = hugetc( );
	    if( (hu_ret == U_RESETKEY ) ||
		(hu_ret == U_PF12KEY  ) ||
		(hu_ret == U_ACTIONKEY) ||
		(hu_ret == U_ENTERKEY ) ||
		(hu_ret == U_CRKEY    ) )
	       break;
	 }
      }


      if( hu_ret == U_PF3KEY )
      {
	  sw = FALSE ;
      }
      else  if( (hu_ret == U_RESETKEY) ||
		(hu_ret == U_PF12KEY) )
      {
	 msg_no   = U_ENDID;
	 wait_flg = U_FOF;
	 posi     = POSI_1;
      }
      else if(  (hu_ret == U_TABKEY  ) ||
		(hu_ret == U_BTABKEY ) ||
		(hu_ret == U_C_UP    ) ||
		(hu_ret == U_C_DOWN  ) )
      {
	 posi   = POSI_3;
	 msg_no = U_ENDID;
      }
      else  if( (hu_ret == U_ACTIONKEY) ||
		(hu_ret == U_ENTERKEY ) ||
		(hu_ret == U_CRKEY    ) )
      {

	/* field check */
	 if( fld_len1 == NULL )
	 {
	    posi = POSI_1 ;  /* set field position */
	    msg_no = U_ARMSGN; /* Please input file name.   */
	 } else if( hufnc(add_fld) != IUSUCC ) {
	    posi = POSI_1 ;  /* set field position */
	    msg_no = U_BFMSGN;	/* this file name is invalid.    */
	 }
	 else  if( fld_len2 == NULL ) {
	    posi = POSI_2 ;  /* set field position */
	    msg_no = U_ARMSGN;	/* Please input file name.   */
	 }
	 else  if( hufnc(new_fld) != IUSUCC ) {
	    posi = POSI_2 ;  /* set field position */
	    msg_no = U_BFMSGN;	/* this file name is invalid.    */
	 }
	 else
	 {

	    if( wait_flg == U_FOF )
	    {
	      /***** user dictionary  file check( add file ) *****/
	       adfdes = open(add_fld, O_RDONLY);
	       posi = POSI_1;

	       if( adfdes == U_FILEE )        /* File Open Error     */
	       {
		  if( errno == ENOENT )         /* No Such File        */
		  {
		     msg_no   = U_AWMSGN ;
				/* Additional dictionary is not exist.   */
		     user_flg = U_FON;
		  }
		  else                        /* The Others          */
		  {
		     msg_no   = U_ATMSGN ;
				/* File access permission denied.     */
		     user_flg = U_FON;
		  }
	       }
	       else
	       {
		 /* get user dictionary file status    */
		  de_ret = fstat( adfdes, &stadd );
		  de_ret = fstat( udcptr->orgfd,  &storg );
		  addlen = stadd.st_size;

		 /* file name check   duplicate */
		  if( storg.st_ino == stadd.st_ino )
		  {
		     msg_no   = U_AXMSGN;
				/* It is not allowed to combine the same files.  */
		     user_flg = U_FON;
		     de_ret = close( adfdes ) ;
		  }

		 /* user dictionary size check */
		  else
		  if( (DSIZE_L > addlen) || (addlen > DSIZE_H) ||
		      ((addlen % DIVD) != NULL ) )
		  {
		     msg_no   = U_AYMSGN;
				/* second file is not user dictionary.       */
		     user_flg = U_FON;
		     de_ret = close( adfdes ) ;
		  }

		 /* allocate memory for dictionary */
		  else
		  if( (ad_dcptr = calloc( addlen, sizeof(uchar)))
							== NULL )
		  {
		    /* cannot allocate memory */
		     msg_no   = U_FMSGN ;
				/* Can not access user dictionary.        */
		     user_flg = U_FON;
		     de_ret = close( adfdes ) ;
		  }

		 /* file read & check  */
		  else
		  if( (read(adfdes, ad_dcptr, addlen)) == U_FILEE )
		  {
		     /* read error  */
		     msg_no   = U_FMSGN;
				/* Can not access user dictionary.        */
		     user_flg = U_FON;
		     de_ret = close( adfdes ) ;
		     free( ad_dcptr );
		  }

		 /* status of the data set check */
		  else
		  if( (de_ret = lseek( adfdes, 0, 0 ))
						    == U_FILEE )
		  {
		     msg_no   = U_FMSGN;
				/* Can not access user dictionary.        */
		     user_flg = U_FON;
		     de_ret = close( adfdes );
		     free( ad_dcptr );
		  }
		  else
		  if( (de_ret = read(adfdes, &stsno, U_STSLEN ))
					     == U_FILEE )
		  {
		     /* read error  */
		     msg_no   = U_FMSGN;
				/* Can not access user dictionary.        */
		     user_flg = U_FON;
		     de_ret = close( adfdes );
		     free( ad_dcptr );
		  }
		  else
		  if( (stsno != 0x00ff) && (stsno != 0x0000) &&
			(stsno != 0x00f0) && (stsno != 0x000f) )
		  {
		     msg_no   = U_BDMSGN;
				/* The dictionary file needs recovering. */
		     user_flg = U_FON;
		     de_ret = close( adfdes );
		     free( ad_dcptr );
		  }

		 /***** user dictionary  file check( new file ) *****/
		  else
		  {

		     de_ret = close( adfdes );          /* add file close */
		     newdes = open( new_fld, O_RDWR );  /* new file open  */
		     posi = POSI_2;

		     if( (newdes == U_FILEE ) && ( errno != ENOENT ) )
		     {
			msg_no   = U_ATMSGN;
				/* File access permission denied.     */
			user_flg = U_FON;
			free( ad_dcptr );
		     }
		     else
		     if( newdes != U_FILEE )
		     {
		       /* get user dictionary file status    */
			de_ret = fstat( newdes, &stnew );
			newlen = stnew.st_size;

		       /* file name check   duplicate */
			if( stnew.st_ino == stadd.st_ino )
			{
			   msg_no   = U_BIMSGN;
				/* Additional dictionary and new dictionary should not be same */
			   user_flg = U_FON;
			   de_ret   = close( newdes ) ;
			   free( ad_dcptr );
			}

		       /* status of the data set check */

			else
			if( (de_ret = lseek( newdes, (long)U_STSPOS, 0 ))
							  == U_FILEE )
			{
			   msg_no   = U_FMSGN;
				/* Can not access user dictionary.        */
			   user_flg = U_FON;
			   de_ret   = close( newdes ) ;
			   free( ad_dcptr );
			}
			else
			if( (de_ret = read(newdes, &stsno, U_STSLEN ))
						   == U_FILEE )
			{
			   /* read error  */
			   msg_no   = U_FMSGN;
				/* Can not access user dictionary.        */
			   user_flg = U_FON;
			   de_ret   = close( newdes ) ;
			   free( ad_dcptr );
			}
			else
			if( (stsno == 0x00ff) && (storg.st_ino != stnew.st_ino) )
			{
			   msg_no = U_BEMSGN;
				/* New dictionary file is now in use.    */
			   user_flg = U_FON;
			   de_ret   = close( newdes ) ;
			   free( ad_dcptr );
			}
			else
			{
			   de_ret   = close( newdes ) ;  /* close new dictionary */

			   msg_no   = U_ASMSGN;
				/* File name already exists. Enter = Do  F12 = Cancel */
			   posi     = POSI_2;
			   wait_flg = U_FON;
			   user_flg = U_FON;
			}
		     }
		  }
	       }
	    }     /***  user dictionary file check end  ***/


	    if( user_flg == U_FOF ) {
	      /* Display Error Massage */
	      msg_no = U_AUMSGN;
			/* Combining...                           */
	      hu_ret = humsg( udcptr, udcptr->yoff+MSG_lin,
			      udcptr->xoff+MSG_col, msg_no ) ;
	      fflush( stdout ) ;

	      /* User Dictionary combination start */
	      adcptr.dcptr = ad_dcptr;
	      de_ret       = hunewdc( udcptr, &adcptr );
	      wait_flg     = U_FOF;
	      free( ad_dcptr );

	      if( storg.st_ino == stnew.st_ino ) {
		flag = U_FOF;
	      } else {
		flag = U_FON;
	      };

	      if( (de_ret == 0) || (de_ret == 1 ) || (de_ret == 2) ) {
		/* set success message ID       */
		if(de_ret == 0) {
		  msg_no = U_AVMSGN;
			/* The file combination was completed.     */
		} else {
		  msg_no = U_BGMSGN;
			/* File combination successed, but some data deleted */
		};

		/* create temporary file  */
		if( flag == U_FOF ) {
		  tmpbf = (char *)udcptr->tmpname;
		} else {
		  strcpy(file_n,new_fld);
		  strcat(file_n,".tmp");
		  tmpbf = (char *)strrchr( file_n, '/' );
		  if( tmpbf == NULL ) {
		    tmpbf = file_n;
		  } else {
		    tmpbf++;
		  };
		  if(strlen(tmpbf) > U_TNMMAX) {
		    *(tmpbf + U_TNMOFF) = NULL;
		    strcat(tmpbf,".tmp");
		  };
		};

		if( (tmpdes=creat(tmpbf
			    ,(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) )
							    != U_FILEE ) {
		  /* write user dictionary (temp)  */
		  de_ret = write( tmpdes, udcptr->dcptr,
					  udcptr->ufilsz );

		  if( de_ret == U_FILEE ) {
		    msg_no = U_FMSGN;
			/* Can not access user dictionary.        */
		  } else {
		    de_ret = close( tmpdes );
		    udcptr->updflg = U_FOF;

		    fcopy[0] = NULL;
		    strcat( fcopy, "cp " );
		    strcat( fcopy, tmpbf );
		    strcat( fcopy, " " );
		    strcat( fcopy, new_fld );
		    system( fcopy );

		    /* delete temporary file  */
		    if( storg.st_ino != stnew.st_ino ) {
		      fcopy[0] = NULL;
		      strcat( fcopy, "rm " );
		      strcat( fcopy, tmpbf );
		      system( fcopy );
		    };

		  };
		} else {
		   msg_no = U_FMSGN;
			/* Can not access user dictionary.        */
		};
	      } else {
		msg_no = U_FMSGN;
			/* Can not access user dictionary.        */
	      };
	      /* copy bak dic data      */
	      if( flag == U_FON ) {
		memcpy(udcptr->dcptr, bak_dcptr, udcptr->ufilsz );
		flag = U_FOF;
	      };

	    };
	    user_flg = U_FOF;

	 }

      }
      else    /* Invalid Key     */
      {
	 if( lin_posi == M_L7 )
	    posi   = POSI_1;
	 else
	    posi   = POSI_2;

	 msg_no = U_GMSGN;
		/* Invalid key has been pressed.          */
      }


   }    /*******    for( )  loop  end   *******/


   free( bak_dcptr );

   putp( clear_screen );            /*     display clear           */

   return( user_flg );

}
/*----------------------------------------------------------------------*/
/*                      End of hudiccom.                                */
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/*                      Begining of hunewdc.                            */
/*----------------------------------------------------------------------*/
int    hunewdc( udcptr, adcptr )
UDCB   *udcptr;
UDCB   *adcptr;
{
   int    fst_sw ;                  /* index loop escape switch        */
   int    sec_sw ;                  /* data  loop escape switch        */
   int    rc     ;                  /* return  code                    */
   short  mode   ;                  /* mode                            */
   int    hu_ret ;                  /* utility return code             */
   int    de_ret ;                  /* editor return  code             */
   short  rrn    ;                  /* relative record no              */
   int    posc   ;                  /* init Candidate position counter.*/
   uchar *dicindex    ;                  /* index area address         */
   uchar *dicdata    ;                  /* data  area address          */
   short  il ;                  	/* index area length           */
   short  i ;                  		/* index area position         */
   short  rl ;                  	/* data  area length           */
   short  j ;                  		/* data  area position         */
   short  dllen  ;                  	/* DL    area length           */
   short  dlpos  ;                  	/* DL    area position         */
   short  candlen ;                  	/* Candidate data length       */
   short  keylen;                  	/* Key   data length           */
   short  i_keylen;
   short lastcflg;
   uchar keydata[U_KEY_MX] ;               /*    Key   data buffer     */
   uchar canddata[U_CAN_MX];               /*    Candidate data buffer */


   /*****   MAKING OF NEW DICTIONARY  LOOP *****/
   rc = 0;

   /************************/
   /*			   */
   /* Gets the Index Block */
   /*			   */
   /************************/
   hudcread( adcptr, (short)3, 0 );
   dicindex = adcptr->rdptr;

   /****************************/
   /*			       */	
   /* get length of index area */
   /*			       */	
   /****************************/
   il = getil(dicindex);

   /***********************/
   /*		          */
   /* index position init */
   /*		          */
   /***********************/
   i = U_ILLEN + U_HARLEN + U_NARLEN;

   /**********************/
   /*			 */
   /* Search Index Block */
   /*			 */
   /**********************/
   for( fst_sw = TRUE; fst_sw == TRUE;  )
   {
      /***************************/
      /*			 */
      /* check end of index area */
      /*			 */
      /***************************/
      if( i >= il )
      {
	  /* END of Combination */
	  fst_sw = FALSE;
      }
      else
      {
	i_keylen = nxtkeylen(dicindex, i, U_UIX_A);
	(void)getrrn(dicindex+i+i_keylen, &rrn);
	i += (i_keylen + U_RRNLEN);
	/* get pointer to data area */
	hudcread( adcptr, (short)4, rrn ) ;
	dicdata = adcptr->rdptr;
	rl = getrl( dicdata );
	j = U_RLLEN;
	for( sec_sw = TRUE; sec_sw == TRUE;  )
	 {
	    /***************************/
	    /*			       */
	    /* check  end of data area */
	    /*			       */
	    /***************************/
	    if( j >= rl )
	    {
	       sec_sw = FALSE;
	    }
	    else
	    {
		keylen = nxtkeylen(dicdata, j, U_REC_L);
	       	memcpy( keydata, (uchar *)(dicdata+j), keylen );
		makeksstr(keydata, keylen);
	       	j += keylen;
	       	posc = 0;
		do {
		   candlen = nxtcandlen(dicdata, j, &lastcflg, U_REC_L);
		   memcpy(canddata, dicdata+j, candlen);
		   makeksstr(canddata, candlen);
		   mode = U_REGIST;
		   hu_ret = hudicadp(mode, keydata, keylen, 
			canddata, candlen, udcptr);
		   /* return check */
		   if( hu_ret == UDSUCC  )
		   {
		     /*  addition successful  */
		     /*  MRU addition         */
		     hu_ret = hudcmrua(udcptr->dcptr, keydata,  (ushort)keylen,
			canddata, (ushort)candlen  );
		     udcptr->updflg = U_FON;
		   }
		   else if( hu_ret == UDDCEXTE )
		   {
		     /* Already Exist */
		     /* Nothing       */
		   }
		   else if( hu_ret == UDDCFULE )
		   {
		     /* no soace to add */
		     rc = 2;
		     fst_sw = sec_sw = FALSE ;
		     break;
		   }
		   else if( hu_ret == UDOVFDLE)
		   {
		     /* no space for key */
		     rc = 1;
		     break;
		   }
		   else
		   {
		     /* Access Error */
		     rc     = U_FON;
		     fst_sw = sec_sw = FALSE ;
		     break;
		   }
		   j += candlen;
	       } while(lastcflg == U_FOF); 
	    }
	}          
      }
   }                
   return(rc);
}
/*----------------------------------------------------------------------*/
/*                      End of hunewdc.                                 */
/*----------------------------------------------------------------------*/
