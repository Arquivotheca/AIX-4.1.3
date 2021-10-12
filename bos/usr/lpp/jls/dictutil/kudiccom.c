static char sccsid[] = "@(#)22	1.4.1.2  src/bos/usr/lpp/jls/dictutil/kudiccom.c, cmdKJI, bos411, 9428A410j 11/24/93 23:36:22";
/*
 *   COMPONENT_NAME: cmdKJI
 *
 *   FUNCTIONS: User Dictionary Utility for Japanese
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

 /******************** START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kudiccom
 *
 * DESCRIPTIVE NAME:    user dictionary combain handler
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:    OCO Source Material - IBM Confidential.
 *                    (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        10332 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudiccom
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            int kudiccom( udcptr, sdcptr, wkname  )
 *
 *  INPUT:              udcptr          : pointer to UDCB
 *                      sdcptr          : pointer to SDCB
 *                      wkname          : name of user dictionary
 *
 *  OUTPUT:
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                      NA.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                      NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              kuhtdc
 *                              kudicadp
 *                              kudcmrua
 *                              kudisply
 *                              kugetc
 *                              kumsg
 *                              kuconvm
 *                      Standard Liblary.
 *                              open
 *                              read
 *                              fstat
 *                              close
 *                              write
 *                              calloc
 *                              lseek
 *                              free
 *                              memcpy
 *                              strcpy
 *                              strcat
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

/*----------------------------------------------------------------------*
 * Include Standard.
 *----------------------------------------------------------------------*/
#include <stdio.h>      /* Standard I/O Package.                        */
/*#include <memory.h>*/ /* Memory Package                               */
#include <fcntl.h>      /* File Control Package                         */
#include <unistd.h>     /*                                              */
#include <sys/stat.h>   /*                                              */
#include <errno.h>      /* Standard Error Number Package                */
#include <string.h>     /* Stirng Package                               */
#include <locale.h>     /*                                              */
#include <nl_types.h>   /*                                              */
#include <limits.h>     /*                                              */

/*----------------------------------------------------------------------*
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kje.h"        /* Utility Define File                          */
#include "kut.h"        /* Utility Define File                          */

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char *cprt1="XXXX-XXX COPYRIGHT IBM CORP 1989, 1992     ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*----- DISPLAY Position -----------------------------------------------*/
#define   MSG_lin          ( 14)
#define   MSG_col          (  0)
#define   LNGMAX           ( 44)
#define   MNU_line         ( 17)
#define   MNU_col          ( 54)

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

#define   M_C1             ( 13)
#define   M_C2             (  3)
#define   M_C3             (  3)
#define   M_C4             (  3)
#define   M_C5             (  5)
#define   M_C6             (  3)
#define   M_C7             (  5)
#define   M_C8             (  3)
#define   M_C9             (  5)
#define   M_C10            (  0)

#define   M_FLNG1          ( 30)
#define   M_FLNG2          ( 32)
#define   M_FLNG3          ( 30)
#define   M_FLNG4          ( 22)
#define   M_FLNG5          ( 48)
#define   M_FLNG6          ( 20)
#define   M_FLNG7          ( 48)
#define   M_FLNG8          ( 18)
#define   M_FLNG9          ( 48)
#define   M_FLNG10         ( 23)

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
#define   LNG_MAX          ( 54)
#define   NAMESIZ          ( 44)

#define   DSIZE_L          ( 59392  )   /* add Dictionary LOW          	*/
#define   DSIZE_H          ( 261120 )   /* add Dictionary HIGH         	*/
#define   DIVD             ( 1024   )   /* Dictionary Size             	*/
#define   STSAD            ( 7170   )   /* states of data set position 	*/

int kudiccom( udcptr, sdcptr, wkname  )

UDCB	*udcptr; 	/* user   dictionary file control block 	*/
SDCB	*sdcptr;        /* system dictionary file control block 	*/
uchar	*wkname;        /* user Dictionary name        			*/

{

    int         fatal_f; 	/* fatal error flag                     */
    int         wait_flg; 	/* waitting    flag                     */
    int         user_flg; 	/* user dictionary file check  error flag*/
    int         flag; 		/* temp file dopy flag                  */
    int		alleng;

    /*uchar       *calloc();*/
    void        perror();
    int         kunewdc();
    int         kufnc();     	/* check file name  			*/
    void        kuconvm();      /* set conversion  mode 		*/

    int         sw; 		/* main loop escape switch              */
    int         s_sw; 		/* standard switch                      */
    int         i, j; 		/* loop counter                         */

    static char  stsno = 0x00; 	/*states of data set buffer             */
    static char  no_sts = 0x00;	/*states of data set buffer             */
    struct stat  stbuf; 	/* file status work buffer              */
    struct stat  stadd; 	/* file status work buffer              */
    struct stat  stnew; 	/* file status work buffer              */
    struct stat  storg; 	/* file status work buffer              */

    int		ufldes; 	/* user   dictionary file descripter    */
    int         adfdes; 	/* addition dictionary file descripter  */
    int         newdes; 	/* new      dictionary file descripter  */
    int         tmpdes;
    int         opn_ret1; 	/* file open descripter                 */
    int         opn_ret2; 	/* file open descripter                 */
    int         sfldes; 	/* system dictionary file descripter    */
    int         mfldes; 	/* message file descripter              */
    short       msg_no; 	/* message number buffer                */
    uchar       *dcptr; 	/* pointer to base address              */
    uchar       *bak_dcptr; 	/* pointer to base address              */
    uchar       *ad_dcptr; 	/* pointer to base address              */

    int         de_ret; 	/* editor return code                   */
    int         ku_ret; 	/* utility return code                  */
    int         re_cod; 	/* standard return code                 */
    int         rc; 		/* standard return code                 */

    int         worklen; 	/* work buffer                          */
    char        *workbf; 	/* work string buffer                   */
    char        *tmpbf; 	/* work string buffer                   */
    int         namesz; 	/* user dictionary file name size       */

    char        *pas_ptr; 	/* user dictionary name address         */
    char        *rc_ptr; 	/* return code                          */
    char        file_n[50]; 	/* dictionary name the origin           */
    long        addlen; 	/* addition dictionary file size        */
    long        newlen; 	/* new      dictionary file size        */
    char        *fname; 	/* new dictionary file name             */

    ushort      lin[M_FIELD]; 	/* line cordnate                        */
    ushort      clm[M_FIELD]; 	/* column cordnate                      */
    ushort      fldlng[M_FIELD];/* fields length                        */
    uchar       *dspfld[M_FIELD];/* display field address               */

    int         lin_posi; 	/* display line position                */
    int         clm_posi; 	/* display col  position                */
    int         datalen; 	/* input data length                    */
    int         in_mod;
    char        *disdata; 	/* input data                           */
    short       fld_len1; 	/* input data  by addtion dictionary    */
    short       fld_len2; 	/* input data  by new     dictionary    */
    short       *iplen;
    int         posi;
    int		ret;
    struct flock flck;          /* flock structure for fcntl()          */

    char        add_fld[45];
    char        new_fld[45];
    char        new_buf[45];

    static  UDCB adcptr;    	/* add dictionary control block 	*/

    static  char fcopy[200];
    static char  wrtsts = 0xf1; /* write status code buffer             */
    static char  cmpsts = 0x00; /* complate status code buffer          */

    /*----- menu titol name  for display -------------------------------*/
    static uchar fld_f1[31];	/* menu display  " 1 "   		*/
    static uchar fld_f2[33]; 	/* menu display  " 2 "   		*/
    static uchar fld_f3[31]; 	/* menu display  " 3 "   		*/
    static uchar fld_f4[23]; 	/* menu display  " 4 "   		*/
    static uchar fld_f5[49]; 	/* menu display  " 5 "   		*/
    static uchar fld_f6[21]; 	/* menu display  " 6 "   		*/
    static uchar fld_f7[49]; 	/* menu display  " 7 "   		*/
    static uchar fld_f8[19]; 	/* menu display  " 8 "   		*/
    static uchar fld_f9[49]; 	/* menu display  " 9 "   		*/
    static uchar fld_f10[24]; 	/* menu display  "10 "   		*/

    strcpy(fld_f1, CU_MNCMGT);
    strcpy(fld_f2, CU_MNCMGK);
    strcpy(fld_f3, CU_MNCMGF);
    strcpy(fld_f4, CU_MNCMGO);
    strcpy(fld_f5, CU_MNKAK1);
    strcpy(fld_f6, CU_MNADIC);
    strcpy(fld_f7, CU_MNKAK1);
    strcpy(fld_f8, CU_MNNDIC);
    strcpy(fld_f9, CU_MNKAK1);
    strcpy(fld_f10, CU_MNKMSG);

    namesz = strlen(wkname);
    memcpy( &fld_f5[2], wkname, namesz ) ;

    lin[M_IX1]  = udcptr->yoff + M_L1; 	/* set line cordinate f1   	*/
    lin[M_IX2]  = udcptr->yoff + M_L2;  /* set line cordinate f2        */
    lin[M_IX3]  = udcptr->yoff + M_L3;  /* set line cordinate f3        */
    lin[M_IX4]  = udcptr->yoff + M_L4; 	/* set line cordinate f4        */
    lin[M_IX5]  = udcptr->yoff + M_L5; 	/* set line cordinate f5        */
    lin[M_IX6]  = udcptr->yoff + M_L6; 	/* set line cordinate f6        */
    lin[M_IX7]  = udcptr->yoff + M_L7; 	/* set line cordinate f7        */
    lin[M_IX8]  = udcptr->yoff + M_L8; 	/* set line cordinate f8        */
    lin[M_IX9]  = udcptr->yoff + M_L9; 	/* set line cordinate f9        */
    lin[M_IX10] = udcptr->yoff + M_L10; /* set line cordinate f10       */

    clm[M_IX1]  = udcptr->xoff + M_C1; 	/* set column cordinate f1      */
    clm[M_IX2]  = udcptr->xoff + M_C2; 	/* set column cordinate f2      */
    clm[M_IX3]  = udcptr->xoff + M_C3; 	/* set column cordinate f3      */
    clm[M_IX4]  = udcptr->xoff + M_C4; 	/* set column cordinate f4      */
    clm[M_IX5]  = udcptr->xoff + M_C5; 	/* set column cordinate f5      */
    clm[M_IX6]  = udcptr->xoff + M_C6; 	/* set column cordinate f6      */
    clm[M_IX7]  = udcptr->xoff + M_C7; 	/* set column cordinate f7      */
    clm[M_IX8]  = udcptr->xoff + M_C8; 	/* set column cordinate f8      */
    clm[M_IX9]  = udcptr->xoff + M_C9; 	/* set column cordinate f9      */
    clm[M_IX10] = udcptr->xoff + M_C10; /* set column cordinate f10     */

    fldlng[M_IX1]  = M_FLNG1;   	/* set field length f1         	*/
    fldlng[M_IX2]  = M_FLNG2;           /* set field length f2         	*/
    fldlng[M_IX3]  = M_FLNG3;           /* set field length f3         	*/
    fldlng[M_IX4]  = M_FLNG4;           /* set field length f4         	*/
    fldlng[M_IX5]  = M_FLNG5;           /* set field length f5         	*/
    fldlng[M_IX6]  = M_FLNG6;           /* set field length f6         	*/
    fldlng[M_IX7]  = M_FLNG7;           /* set field length f7         	*/
    fldlng[M_IX8]  = M_FLNG8;           /* set field length f8         	*/
    fldlng[M_IX9]  = M_FLNG9;           /* set field length f9         	*/
    fldlng[M_IX10] = M_FLNG10;          /* set field length f10        	*/

    dspfld[M_IX1]  = fld_f1;            /* set field address f1        	*/
    dspfld[M_IX2]  = fld_f2;            /* set field address f2        	*/
    dspfld[M_IX3]  = fld_f3;            /* set field address f3        	*/
    dspfld[M_IX4]  = fld_f4;            /* set field address f4        	*/
    dspfld[M_IX5]  = fld_f5;            /* set field address f5        	*/
    dspfld[M_IX6]  = fld_f6;            /* set field address f6        	*/
    dspfld[M_IX7]  = fld_f7;           	/* set field address f7        	*/
    dspfld[M_IX8]  = fld_f8;           	/* set field address f8        	*/
    dspfld[M_IX9]  = fld_f9;           	/* set field address f9        	*/
    dspfld[M_IX10] = fld_f10;           /* set field address f10       	*/

    fatal_f  = IUSUCC;
    msg_no   = U_ENDID;

    /*----- user dictionary back up to memory --------------------------*/
    bak_dcptr = calloc( udcptr->ufilsz, sizeof(uchar) );

    /*----- copy dictionary --------------------------------------------*/
    memcpy( bak_dcptr, udcptr->dcptr, udcptr->ufilsz );

    /*------------------------------------------------------------------*
     * DISPLAY MENU
     *------------------------------------------------------------------*/
/* B.EXTCUR */
    clear();
    refresh();
/* E.EXTCUR */

    kuconvm( (short)(U_FON) );  	/* set conversion mode     	*/

    for( i = M_IX1 ; i <= M_IX10 ; i++ ) { 	/* display field loop 	*/
	ku_ret = kudisply( udcptr, lin[i], clm[i], dspfld[i], fldlng[i] );
    }

    ku_ret = kudisply( udcptr, udcptr->yoff+DICNAM_L, udcptr->xoff+DICNAM_C,
			   	udcptr->udfname, LNG_MAX );

    fflush( stdout );


    /*------------------------------------------------------------------*
     * MAIN LOOP
     *------------------------------------------------------------------*/
    /*------------------------------------------------------------------*
     * INITALAZE
     *------------------------------------------------------------------*/
    add_fld[0] = new_fld[0] = NULL;
    fld_len1   = fld_len2   = 0;

    clm_posi = M_C5 + 2;
    posi     = POSI_1; 		/* position swich              		*/
    iplen    = &fld_len1;
    wait_flg = U_FOF;
    msg_no   = U_ENDID;

    for ( sw = TRUE;  sw == TRUE;    ) {

	user_flg = U_FOF;

    	/*----- KEY IN LOOP --------------------------------------------*/
      	switch ( posi ) {
	case  POSI_1 :
	    lin_posi = M_L7;
	    posi     = POSI_2;
	    iplen    = &fld_len1;
	    disdata  = add_fld;
	    break;

	case  POSI_2 :
	    lin_posi = M_L9;
	    posi     = POSI_1;
	    iplen    = &fld_len2;
	    disdata  = new_fld;
	    break;

	default :
	    if ( lin_posi == M_L7 ) {
	    	lin_posi = M_L9;
		iplen    = &fld_len2;
		disdata  = new_fld;
	    }
	    else {
		lin_posi = M_L7;
		iplen    = &fld_len1;
		disdata  = add_fld;
	    }
	    break;
	}

      	/*----- Display Error Massage ----------------------------------*/
      	ku_ret = kumsg( udcptr, udcptr->yoff+MSG_lin,
			       udcptr->xoff+MSG_col, msg_no );

      	fflush( stdout );

      	if ( wait_flg == U_FOF ) {

	    /*----- INPUT FIELD ----------------------------------------*/
	    ku_ret = kuipfld( udcptr, (short)(udcptr->yoff+lin_posi),
			   (short)(udcptr->xoff+clm_posi),
			   disdata, (short)LNGMAX,
			   (short)T_FILE, iplen, C_SWOFF, U_SJIS );

	    add_fld[fld_len1] = NULL;
	    new_fld[fld_len2] = NULL;
      	}
      	else {

	    /*----- waitting for triger key ----------------------------*/
	    for ( ; ; ) {
	    	ku_ret = kugetc( );
	    	if ( (ku_ret == U_RESETKEY ) ||
			(ku_ret == U_PF12KEY  ) ||
			(ku_ret == U_ACTIONKEY) ||
			(ku_ret == U_ENTERKEY ) ||
			(ku_ret == U_CRKEY    ) )
	       		break;
	    }
      	}

      	if ( ku_ret == U_PF3KEY ) {
	    sw = FALSE;
        }
      	else if ( (ku_ret == U_RESETKEY) || (ku_ret == U_PF12KEY) ) {
	    msg_no   = U_ENDID;
	    wait_flg = U_FOF;
	    posi     = POSI_1;
      	}
      	else if ( (ku_ret == U_TABKEY) || (ku_ret == U_BTABKEY) ||
		(ku_ret == U_C_UP) || (ku_ret == U_C_DOWN) ) {
	    posi   = POSI_3;
	    msg_no = U_ENDID;
      	}
      	else if ( (ku_ret == U_ACTIONKEY) || (ku_ret == U_ENTERKEY) ||
			(ku_ret == U_CRKEY) ) {

	    /*----- field check ----------------------------------------*/
	    if ( fld_len1 == NULL ) {
	    	posi = POSI_1;  	/* set field position 		*/
	    	msg_no = U_ARMSGN;
	    } else if( kufnc(add_fld) != IUSUCC ) {
	    	posi = POSI_1;  	/* set field position 		*/
	    	msg_no = U_BFMSGN;
	    }
	    else if ( fld_len2 == NULL ) {
	    	posi = POSI_2;  	/* set field position 		*/
	    	msg_no = U_ARMSGN;
	    }
	    else if ( kufnc(new_fld) != IUSUCC ) {
	    	posi = POSI_2;  	/* set field position 		*/
	    	msg_no = U_BFMSGN;
	    }
	    else {
	    	if ( wait_flg == U_FOF ) {
		  do {
		    /*--------------------------------------------------*
	      	     * user dictionary file check( add file )
		     *--------------------------------------------------*/
	       	    adfdes = open( add_fld, O_RDONLY );
	       	    posi = POSI_1;

	            if ( adfdes == U_FILEE ) { /* File Open Error     	*/
		  	if ( errno == ENOENT ) {/* No Such File        	*/
		     	    msg_no   = U_AWMSGN;
		     	    user_flg = U_FON;
			    break;
		  	}
		  	else {                  /* The Others          	*/
		     	    msg_no   = U_ATMSGN;
		     	    user_flg = U_FON;
			    break;
		  	}
	       	    }

		    /*----- get user dictionary file status ------------*/
		    de_ret = fstat( adfdes, &stadd );
		    de_ret = fstat( udcptr->orgfd, &storg );
		    addlen = stadd.st_size;

		    /*----- file name check   duplicate ----------------*/
		    if ( storg.st_ino == stadd.st_ino ) {
		    	msg_no   = U_AXMSGN;
		        user_flg = U_FON;
		        de_ret = close( adfdes );
			break;
		    }

		    /*----- user dictionary size check -----------------*/
		    if ( ((DSIZE_L > addlen) && (addlen > DSIZE_H)) ||
		      			((addlen % DIVD) != NULL ) ) {
		    	msg_no   = U_AYMSGN;
		      	user_flg = U_FON;
		     	de_ret = close( adfdes );
			break;
		    }

		    /*----- allocate memory for dictionary -------------*/
		    if ( (ad_dcptr = calloc( addlen, sizeof(uchar)))
							== NULL ) {
		    	msg_no   = U_FMSGN;
		        user_flg = U_FON;
		        de_ret = close( adfdes );
			break;
		    }

    		    /*------- Lock User dictionary ---------------------*/
    		    flck.l_type = F_RDLCK;
    		    flck.l_whence = flck.l_start = flck.l_len = 0;
    		    for ( i=0; i<U_TRYLOK; i++) {
        	    	if ( (ret = fcntl( adfdes, F_SETLK, &flck )) != -1 )
                	    break;
    		    }
    		    if ( ret == -1 ) {
		       	msg_no   = U_ATMSGN;
		       	user_flg = U_FON;
		       	de_ret = close( adfdes );
		        free( ad_dcptr );
			break;
    		    }

		    /*----- file read & check --------------------------*/
		    if( (read(adfdes, ad_dcptr, addlen)) == U_FILEE ) {
		    	msg_no   = U_FMSGN;
		        user_flg = U_FON;
		        de_ret = close( adfdes );
		        free( ad_dcptr );
			break;
		    }

		    /*----- status of the data set check -----------*/
		    if ( (de_ret = lseek( adfdes, (long)STSAD, 0 ))
						    == U_FILEE ) {
		       	msg_no   = U_FMSGN;
		     	user_flg = U_FON;
		     	de_ret = close( adfdes );
		     	free( ad_dcptr );
			break;
		    }
		    if ( (de_ret = read(adfdes, &stsno, 1)) == U_FILEE ) {
		     	msg_no   = U_FMSGN;
		     	user_flg = U_FON;
		     	de_ret = close( adfdes );
		     	free( ad_dcptr );
			break;
		    }
		    if ( (stsno != 0xf1) && (stsno != 0x00) ) {
		     	msg_no   = U_BDMSGN;
		     	user_flg = U_FON;
		     	de_ret = close( adfdes );
		     	free( ad_dcptr );
			break;
		    }

    		    /*----- Unlock User dictionary ---------------------*/
    		    flck.l_type = F_UNLCK;
    		    for ( i=0; i<U_TRYLOK; i++) {
        		if ( (ret = fcntl( adfdes, F_SETLK, &flck )) != -1 )
                	    break;
    		    }
    		    if ( ret == -1 ) {
		    	msg_no   = U_FMSGN;
		     	user_flg = U_FON;
		     	de_ret = close( adfdes );
		     	free( ad_dcptr );
			break;
    		    }

		    de_ret = close( adfdes );/* add file close 	*/

		    /*--------------------------------------------------*
	      	     * user dictionary file check( new file )
		     *--------------------------------------------------*/
		    newdes = open( new_fld, O_RDWR );/*new file open*/
		    posi = POSI_2;

		    if ((newdes == U_FILEE ) && ( errno != ENOENT )) {
			msg_no   = U_ATMSGN;
			user_flg = U_FON;
			free( ad_dcptr );
			break;
		    }
		    else if ( newdes == U_FILEE ) {
			break;
		    }

		    /*----- get user dictionary file status -----------*/
		    de_ret = fstat( newdes, &stnew );
		    newlen = stnew.st_size;

		    /*----- file name check duplicate -----------------*/
		    if ( stnew.st_ino == stadd.st_ino ) {
		    	msg_no   = U_AXMSGN;
			user_flg = U_FON;
		    	de_ret   = close( newdes );
		    	free( ad_dcptr );
			break;
		    }

		    /*----- user dictionary size check ----------------*/
		    /* if ( ((DSIZE_L > newlen) &&
		     *  		(newlen > DSIZE_H)) ||
		     *      	((newlen % DIVD) != NULL ) ) {
		     *      msg_no   = U_AYMSGN;
		     *      user_flg = U_FON;
		     *      de_ret   = close( newdes );
		     *      free( ad_dcptr );
		     *	    break;
		     * }
		     */

		    /*----- status of the data set check -------------*/
		    if( (de_ret = lseek( newdes, (long)STSAD, 0 ))
							  == U_FILEE ) {
		 	msg_no   = U_FMSGN;
		    	user_flg = U_FON;
		    	de_ret   = close( newdes );
		    	free( ad_dcptr );
			break;
		    }
		    if( (de_ret = read(newdes, &stsno, 1 )) == U_FILEE ) { 
		   	msg_no   = U_FMSGN;
			user_flg = U_FON;
			de_ret   = close( newdes );
			free( ad_dcptr );
			break;
		    }

		    if ( (stsno == 0xf1) &&
				(storg.st_ino != stnew.st_ino) ) {
		    	msg_no = U_BEMSGN;
		   	user_flg = U_FON;
			de_ret   = close( newdes );
			free( ad_dcptr );
			break;
		    }

                    msg_no   = U_ASMSGN;
                    posi     = POSI_2;
                    wait_flg = U_FON;
                    user_flg = U_FON;

		  } while(0);
	    	}  /***  user dictionary file check end  ****************/

	    	if ( user_flg == U_FOF ) {

	      	    /*----- Display Error Massage ----------------------*/
	      	    msg_no = U_AUMSGN;
	      	    ku_ret = kumsg( udcptr, udcptr->yoff+MSG_lin,
			      udcptr->xoff+MSG_col, msg_no );
	      	    fflush( stdout );

	      	    /*----- User Dictionary combination start ----------*/
	      	    adcptr.dcptr = ad_dcptr;
	      	    de_ret       = kunewdc( udcptr, &adcptr );
	      	    wait_flg     = U_FOF;
	      	    free( ad_dcptr );

	      	    if ( storg.st_ino == stnew.st_ino ) {
			flag = U_FOF;
	      	    } else {
			flag = U_FON;
	      	    }

	      	    if ( (de_ret == 0) || (de_ret == 1 ) || (de_ret == 2) ) {

			/*----- set success message ID -----------------*/
			if ( de_ret == 0 ) {
		  	    msg_no = U_AVMSGN;
			} else {
		  	    msg_no = U_BGMSGN;
			}

			/*----- create new file ------------------------*/
			if ( newdes == U_FILEE ) {
			    newdes = creat( new_fld, 
				(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) );
			}

			/*----- save new file --------------------------*/
			ku_ret = 
			    kutmwrt( newdes, udcptr->dcptr, udcptr->ufilsz );
		  	if( ku_ret != IUSUCC ) {
		    	    msg_no = U_AGMSGN;
		     	    user_flg = U_FON;
		  	} else {
			    if ( flag == U_FON ) {
			    	lseek( newdes, (U_MRU_A+U_ILLEN), 0 );
			    	ku_ret = write( newdes, &no_sts, 1 );
			    }
			    else {
		    	    	udcptr->updflg = U_FON;
			    }
		  	}

	      	    } else {
			msg_no = U_FMSGN;
	      	    }

	      	    /*----- copy bak dic data --------------------------*/
	      	    if ( flag == U_FON ) {
			memcpy( udcptr->dcptr, bak_dcptr, udcptr->ufilsz );
			flag = U_FOF;
	      	    }
	    	}
	    }
      	}
        else {   /* Invalid Key     */
	    if( lin_posi == M_L7 )
	    	posi   = POSI_1;
	    else
	    	posi   = POSI_2;

	    msg_no = U_GMSGN;
      	}

    }    /*******    for( )  loop  end   *******/

    free( bak_dcptr );

    clear();
    refresh();
    return( user_flg );

}

int  kunewdc( udcptr, adcptr )

   UDCB   *udcptr;
   UDCB   *adcptr;
{

   int    fst_sw ;                  /* index loop escape switch        */
   int    sec_sw ;                  /* data  loop escape switch        */
   int    rc     ;                  /* return  code                    */

   short  mode   ;                  /* mode                            */
   int    ku_ret ;                  /* utility return code             */
   int    de_ret ;                  /* editor return  code             */
   short  rrn    ;                  /* relative record no              */
   int    posc   ;                  /* init kanji position counter.    */
   uchar *ipt    ;                  /* index area address              */
   uchar *dpt    ;                  /* data  area address              */
   short  indlen ;                  /* index area length               */
   short  indpos ;                  /* index area position             */
   short  datlen ;                  /* data  area length               */
   short  datpos ;                  /* data  area position             */
   short  dllen  ;                  /* DL    area length               */
   short  dlpos  ;                  /* DL    area position             */
   short  knjlen ;                  /* kanji data length               */
   short  kanalen;                  /* kana  data length               */

   uchar kanadt[20] ;               /*    kana  data buffer            */
   uchar knjdata[40];               /*    kanji data buffer            */


   /*****   MAKING OF NEW DICTIONARY  LOOP *****/

   rc = 0;

   /* get pointer to index area                                    */
   kudcread( adcptr, (short)3, rrn );

   ipt = adcptr->rdptr;

   /* get length of index area                                 */
   indlen = GETSHORT( ipt );

   /* index position init                                      */
   indpos = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN;


   /*$$$$$   loop of index    $$$$$                            */
   for( fst_sw = TRUE; fst_sw == TRUE;  )
   {
       /* check end of index area                              */
      if( indpos >= indlen )
      {
	  /* END of Combination                                */
	  fst_sw = FALSE;
      }
      else
      {
	 /* set index position to RRN                          */
	 indpos = indpos + *(ipt + indpos);

	 /* get RRN                                            */
	 rrn = *(ipt + indpos);

	 /* set index position to next KNL                     */
	 indpos = indpos + U_RRNLEN;

	 /* get pointer to data area                           */
	 kudcread( adcptr, (short)4, rrn ) ;
	 dpt = adcptr->rdptr;

	 /* get length of data area                            */
	 datlen = GETSHORT( dpt );

	 /* data position init (to start entry)                */
	 datpos = U_RLLEN;


	 /*@@@@@    loop of data area    @@@@@                 */
	 for( sec_sw = TRUE; sec_sw == TRUE;  )
	 {
	    /* check  end of data area                         */
	    if( datpos >=datlen )
	    {
	       sec_sw = FALSE;
	    }
	    else
	    {
	       /* get length of yomigana from KNL              */
	       kanalen = *(dpt + datpos) - U_KNLLEN;

	       /* set data position to start of yomidata       */
	       datpos = datpos + U_KNLLEN;


	       /* set of yomigana data                         */
	       memcpy( kanadt, (uchar *)(dpt+datpos), kanalen );


	       /* set data position to strat of DL             */
	       datpos = datpos + kanalen;

	       /* init kanji position counter.                 */
	       posc = 0;

	       /* get length of DL area                        */
	       dllen = GETSHORT((dpt + datpos));

	       /* DL position init  (to start of kanji data)   */
	       dlpos = datpos + U_DLLEN + U_RSVLEN;

	       /* set data position to start of next entry     */
	       datpos = datpos + dllen;

	       /*!!!!!    loop of yomi area    !!!!!           */
	       while( TRUE )
	       {
		  /* check end of data area                    */
		  if( dlpos >= datpos )  break;

		  /* init length of kanji                      */
		  knjlen = 0;

		  /* loop of kanji convert. ( endloop mark is #### ) */
		  while(TRUE) {

			/* check ! end of kanji data           */
			if( *(dpt + dlpos) > U_CONV )  break;

			/* check convert type & convrt of high byte. */
			if( *(dpt + dlpos) > U_7PCCC )
			    knjdata[knjlen] = *(dpt + dlpos) | U_7PCCU;
			else
			    knjdata[knjlen] = *(dpt + dlpos) | U_7PCCL;

			/* convert of low byte.                */
			dlpos  = dlpos  + 1;
			knjlen = knjlen + 1;
			knjdata[knjlen] = *(dpt + dlpos);

			/* count up                            */
			dlpos  = dlpos  + 1;
			knjlen = knjlen + 1;

		  };  /* #### endloop                          */
		  /* last 2byte convert.                       */
		  knjdata[knjlen] = *(dpt + dlpos);
		  dlpos  = dlpos + 1;
		  knjlen = knjlen + 1;
		  knjdata[knjlen] = *(dpt + dlpos);

		  /* count up                                  */
		  dlpos  = dlpos  + 1;
		  knjlen = knjlen + 1;


		  mode = U_REGIST;
		  ku_ret = kudicadp( mode,    kanadt, kanalen,
				     knjdata, knjlen, udcptr   );


		  /* return check */
		  if( ku_ret == UDSUCC  )
		  {
		     /*  addition successful  */
		     /*  MRU addition         */
		     ku_ret = kudcmrua( udcptr->dcptr, kanadt,  (uchar)kanalen,
					       knjdata, (uchar)knjlen  );

		  }
		  else if( ku_ret == UDDCEXTE )
		  {
		     /* Already Exist         */
		     /* Nothing               */
		  }
		  else if( ku_ret == UDDCFULE )
		  {
		     /* no soace to add */
		     rc = 2;
		     fst_sw = sec_sw = FALSE ;
		     break;
		  }
		  else if( ku_ret == UDOVFDLE)
		  {
		     /* no space for yomi       */
		     rc = 1;
		     break;
		  }
		  else
		  {
		     /* Access Error          */
		     rc     = U_FON;
		     fst_sw = sec_sw = FALSE ;
		     break;
		  }

		  /* set DL position to next kanji data        */
		  dlpos = dlpos + U_RSVLEN;

	       }     /*!!!!!  loop of yomi area end   !!!!!*/
	    }
	}           /*@@@@@  loop of data area end   @@@@@*/
      }
   }                /*$$$$$   loop of index end       $$$$$*/

   return(rc);

}
