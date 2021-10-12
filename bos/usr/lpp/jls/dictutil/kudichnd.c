static char sccsid[] = "@(#)28	1.7.1.3  src/bos/usr/lpp/jls/dictutil/kudichnd.c, cmdKJI, bos411, 9428A410j 3/23/94 04:40:06";
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

/********************* START OF MODULE SPECIFICATIONS ***********************
 * MODULE NAME:         kudichnd
 *
 * DESCRIPTIVE NAME:    user dictionary handler
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
 *  MODULE SIZE:        9499 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudichnd
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            void main(argc, argv)
 *
 *  INPUT:              argc : parameter number
 *                      argv : parameter pointer
 *
 *  OUTPUT:             NA.
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
 *                              kudicadh
 *                              kudicuph
 *                              kudicrcv
 *                              kudisply
 *                              kudisply
 *                              kuconvm
 *                      Standard Liblary.
 *                              open
 *                              read
 *                              fstat
 *                              fcntl
 *                              close
 *                              write
 *                              getenv
 *                              calloc
 *                              lseek
 *                              free
 *                              fileno
 *                              memcpy
 *                              strcpy
 *                              strcat
 *                              fprintf
 *                              tcgetattr
 *                              tcsetattr
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
#include <stdlib.h>
#include <termios.h>    /* POSIX line disciplines                       */
/*#include <memory.h>*/ /* Memory Package                               */
#include <fcntl.h>      /* File Control Package                         */
#include <unistd.h>     /*                                              */
#include <signal.h>     /*                                              */
#include <sys/stat.h>   /*                                              */
#include <errno.h>      /* Standard Error Number Package                */
#include <string.h>     /* Stirng Package                               */
#include <sys/lockf.h>  /*                                              */

#include <locale.h>     /*                                              */
#include <nl_types.h>   /*                                              */
#include <limits.h>     /*                                              */

/*----------------------------------------------------------------------*
 * Include Kanji Project.
 *----------------------------------------------------------------------*/
#include "kje.h"
#if defined(HZ)
#undef HZ
#endif
#include "kut.h"        /* Utility Define File                          */

extern nl_catd catd;
extern int     cnvflg;

/*----------------------------------------------------------------------*
 * Copyright Identify.
 *----------------------------------------------------------------------*/
static char *cprt1="XXXX-XXX COPYRIGHT IBM CORP 1989, 1992     ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*----- default message file name --------------------------------------*/
#define   U_MSGFLE         "/usr/lpp/jls/etc/kjdict.jmsg"

/*----- DISPLAY Position -----------------------------------------------*/
#define   MNU_lin1         (  1)
#define   MNU_lng1         ( 40)
#define   MNU_col1         (  8)
#define   MNU_lin2         (  3)
#define   MNU_lng2         ( 54)
#define   MNU_col2         (  0)
#define   MNU_lin3         (  5)
#define   MNU_lng3         ( 29)
#define   MNU_col3         (  3)
#define   MNU_lin4         ( 16)
#define   MNU_lng4         ( 11)
#define   MNU_col4         (  0)
#define   MSG_lin          ( 15)
#define   MSG_col          (  0)
#define   MSG_lng          ( 60)
#define   LNGMAX           ( 44)
#define   MNU_line         ( 17)
#define   MNU_col          ( 54)

/*----------------------------------------------------------------------*
 * this static area is needed if you use getcharcter.
 *----------------------------------------------------------------------*/
int    wait_time;                    /* flash wait got term buffer*/
int    scrollflg = 0;                /* Action just before is     */
int    fkmaxlen, nkey;               /* Function key string max   */
struct fkkey fkkeytbl[KEY_MAXC];     /* Function key string array */
char   trigertable[KEY_MAXC];        /* array of top character of */
ushort user_flg;                     /* user dictionary error flag*/

#if defined(CNVSTOC)
int kudichnd( argc, argv )
#else
void main( argc, argv )
#endif

int          argc; 	/*  parameter count                         */
char        *argv[]; 	/*  paramter pointer                        */
{

    FILE         *fp_err;	/* FILE pointer to stderr       	*/
    int          o_menun;       /* pre menu number      		*/
    int          menun;         /* input menu number    		*/
    int          clr = 0;       /* parameteer fo IOCTL  		*/
    int          alleng;        /* length of file name for display      */
    static uchar wktabl[60]; 	/*  user dictionary name wark table     */
    int          MAX_line; 	/* screen max line 			*/
    int          MAX_col; 	/* screen max col  			*/

    /*uchar       *calloc()   ;*/
    char	*getenv();
    void        perror();
    long        lseek();
    int         kudicuph();
    int         kudicadh();
    int         kudicrcv();
    int         kugeti();
    int         kugetc();
    int         kumsg();
    int         kugetcmp();
    void        kuconvm();
    int	        kuusrrd();
    int	      	kusysrd();
    int	      	kuopbkf();

    char        *usrname; 	/* user dictionary file name		*/
    int          tmpdes; 	/* temporary file descripter            */
#if !defined(MSGFILE)
    int          mfldes; 	/* message file descripter              */
#endif
    uint         mnelem; 	/* message size buffer                  */
    struct stat  stbuf; 	/* file status work buffer              */
    int          fatal_f; 	/* fatal error flag                     */
    int          tmpsts; 	/* fatal error temporary flag           */
    int          output_device;	/* output device                        */
    int          i, j; 		/* loop counter                         */
    long         x1, y1; 	/* work x, y offset position            */
    long         mode; 		/* echo mode                            */
    long         fildes;
    int          de_ret; 	/* editor return code                   */
    int          ku_ret; 	/* utility return code                  */
    int          wkbuf; 	/* work buffer                          */
    char         wkstr[3]; 	/* work string buffer                   */
    uchar        *wkname; 	/* work name buffer for user dictionary */
    uchar        comname[60]; 	/* work name for combain process 	*/
    int          namesz; 	/* user dictionary file name size       */
    int          cpypos; 	/* file name copy position              */
    short        col;           /* work display x axis position 	*/
    short        line;          /* work display y axis position 	*/
    short        msgid;         /* message ID   			*/
    short        nuflg;         /* data nothing flag    		*/
    short        update_f;      /* updatede flag        		*/
    short        merflg;        /* message erse flag    		*/
    short        msgx;          /* message area x axis  		*/
    short        msgy1;         /* message area y axis 1        	*/
    short        msgy2;         /* message area y axis 2        	*/
    int          triger;        /* getkey triger        		*/
    int          wrc;           /* ret code     			*/
    ushort       wksht ;        /* work short   			*/
    ushort       user_flg_dummy;/* user dictionary error flag    	*/
    struct       flock flck;    /* flock structure for fcntl()          */

    static UDCB  udcptr; 	/* user   dictionary file control block */
    static SDCB  sdcptr[MAX_SYSDICT];/* system dictionary file control block*/

    static char  nmlsts = 0x00; /* normal status code buffer            */
    static char  wrtsts = 0xf1; /* write status code buffer             */
    static char  cmpsts = 0x00; /* complate status code buffer          */

    static uchar *titol_name;	/* menu titol name  for display 	*/
    static uchar usrflname[1025];/* usr dictionary file name for display */
    static uchar *selmsg;	/* select massege for display 		*/
    static uchar *footmsg;	/* Enter menue for display    		*/

    /*----- menu select handling key -----------------------------------*/
    static int      ICHI[5] = {0x31,0x8250,0x82ca,0x836b,0xc7};
    static int      NI[5]   = {0x32,0x8251,0x82d3,0x8374,0xcc};
    static int      SAN[5]  = {0x33,0x8252,0x82a0,0x8341,0xb1};
    static int      YON[5]  = {0x34,0x8253,0x82a4,0x8345,0xb3};
    static int      GO[5]   = {0x35,0x8254,0x82a6,0x8347,0xb4};
    static int      KYU[5]  = {0x39,0x8258,0x82e6,0x8388,0xd6};

    /*----- menu X & length --------------------------------------------*/
    static short MNU_y[6] = { 7 , 8 , 9 , 10 , 11 , 13 };
    static short MNU_l    = 23;
    static short MNU_x    = 3;

    /*----- menu display  " 1 " ----------------------------------------*/
    static char  MNU_data[6][30];

    static uchar *nomsg;	/* message for message file access error*/
    static uchar *d_err;	/* Display size error message   	*/

    /*------------------------------------------------------------------*
     * Starting program
     *------------------------------------------------------------------*/
    titol_name   = CU_MNTITL;
    strcpy(usrflname, CU_MNUFNM);
    selmsg       = CU_MNSMSG;
    footmsg      = CU_MNFMSG;

    strcpy(MNU_data[0], CU_MNDAT1);
    strcpy(MNU_data[1], CU_MNDAT2);
    strcpy(MNU_data[2], CU_MNDAT3);
    strcpy(MNU_data[3], CU_MNDAT4);
    strcpy(MNU_data[4], CU_MNDAT5);
    strcpy(MNU_data[5], CU_MNDAT6);

    nomsg        = CU_MNMAE1;
    d_err        = CU_MNDERR;

/* B.EXTCUR */
   (void)kuinit();
/* E.EXTCUR */

   signal(SIGINT,SIG_IGN);
   signal(SIGQUIT,SIG_IGN);

   /*
    *  reopen stderr to fp_err
   fp_err = freopen(ERR_FILE,"w",stderr);
    */

   /*-------------------------------------------------------------------*
    * user, system dictionary and message file memory copy
    *-------------------------------------------------------------------*/
   fatal_f  = IUSUCC ;            /* initial fatal error flag           */
   msgid    = U_ENDID;            /* set normal message number          */
   mode     = U_ECHO ;            /* set echo mode                      */
   update_f = U_FOF;
   nuflg    = C_SWOFF;

   /*-------------------------------------------------------------------*
    * get characteristics of the display and adapter
    *-------------------------------------------------------------------*/
/* B.EXTCUR */
   MAX_line = LINES;
   MAX_col  = COLS;
/* E.EXTCUR */

   udcptr.ymax = MAX_line;                 /* set up maximum            */

   udcptr.yoff = ( MAX_line - MNU_line )  / 2 ;  /* set up x,y offset   */
   udcptr.xoff = ( MAX_col  - MNU_col  )  / 2 ;

   if(( MAX_line < MNU_line ) || ( MAX_col < MNU_col )) {
   	/*----- display error ------------------------------------------*/
/* B.EXTCUR */								
   	clear();
   	refresh();
   	printf( "%s\n", d_err );        /*    display error message   */
   	(void)kureset();
/* E.EXTCUR */
#if defined(CNVSTOC)
     	return( UDDISPE );
#else
     	exit( UDDISPE );
#endif
   }

   /*-------------------------------------------------------------------*
    * Initialize
    *-------------------------------------------------------------------*/
   udcptr.updflg = U_FOF;

   /*-------------------------------------------------------------------*
    * Reading User dictionary
    *-------------------------------------------------------------------*/
   fatal_f = kuusrrd( argv[1], &udcptr, &msgid );
   if ( fatal_f == IUFAIL || fatal_f == IURECOV )
     	user_flg = U_FON;	/* user dictionary error flag on	*/
   else if ( fatal_f == IUUPDAT )
	mode = U_NECHO;   	/* set non echo mode  			*/
	
   /*-------------------------------------------------------------------*
    * Opening User dictionary ( backup )
    *-------------------------------------------------------------------*/
   if ( fatal_f != IUFAIL ) {
   	tmpsts = fatal_f;      	/* store status to temporary buffer     */
   	fatal_f = kuopbkf( argv[1], &udcptr, &msgid );
   	if ( fatal_f == IUFAIL )
   	    user_flg = U_FON;
        else 
	    fatal_f = tmpsts;
   }

   /*-------------------------------------------------------------------*
    * user dictionary file name convert for display
    *-------------------------------------------------------------------*/
   switch( *(char *)argv[2] ) {
    	case '1' : usrname = "./usrdict"; 	break;
    	case '2' : usrname = "$HOME/.usrdict"; 	break;
    	default  : usrname = argv[1]; 		break;
   }

   /*-------------------------------------------------------------------*
    * making user dictionary file name for display
    *-------------------------------------------------------------------*/
   namesz = strlen(usrname);
   wkname = (uchar *)calloc(namesz+1, 1);
   for ( i=namesz-1, j=namesz-1, cpypos=NULL; i>=NULL; i--, j-- ) {
   	if( (*(usrname + i) == '/') && ((namesz - i) < LNGMAX) ) {
     	    cpypos = j;
   	}
   	wkname[j] = *( usrname + i );
   }
   wkname[namesz] = NULL;

   if ( namesz <= LNGMAX ) {
    	memcpy( &usrflname[9], wkname, namesz );
    	memcpy( comname, wkname, namesz );
    	comname[namesz] = NULL;
    	j = namesz;
   } else {
    	j = namesz - cpypos;
    	usrflname[8] = 0x2d;
    	memcpy(&usrflname[9], &wkname[cpypos], j );
    	comname[0] = 0x2d;
    	memcpy(&comname[1], &wkname[cpypos], j );
    	comname[j + 1] = NULL;
   }

   if ( cnvflg == 1 ) {         /* sjis */
  	usrflname[9 + j] = 0x81;
  	usrflname[9 + (j + 1)] = 0x6a;
   }

   if ( cnvflg == 2 ) {         /* euc */
  	usrflname[9 + j] = 0xa1;
  	usrflname[9 + (j + 1)] =0xcb;
   }

   usrflname[9 + (j + 2)] = NULL;

   alleng = (54 - (j + 10)) / 2;
   alleng = ((alleng > 0) ? alleng-- : 0);

   memset( wktabl, 0x20, 55 );

   memcpy( &wktabl[alleng], usrflname , j+13 );

   udcptr.udfname = wktabl;

   /*-------------------------------------------------------------------*
    * Reading System dictionary
    *-------------------------------------------------------------------*/
   if ( fatal_f != IUFAIL ) {
	tmpsts = fatal_f;       /* store status to temporary buffer     */
	fatal_f = kusysrd( sdcptr, &msgid );
	if ( fatal_f != IUFAIL )
	    fatal_f = tmpsts;
   }

#if !defined(MSGFILE)
   /*-------------------------------------------------------------------*
    * message file open & check
    *-------------------------------------------------------------------*/
   if ( (mfldes = open( U_MSGFLE, (O_RDONLY | O_NDELAY)) ) == U_FILEE ) {
    	fatal_f = INOMSGFL;		/* set fatal error             	*/
    	mode = U_ECHO;                  /* set echo mode               	*/
   }
   else {
    	/*----- open successful ----------------------------------------*/
    	de_ret = fstat( mfldes, &stbuf );/* get message file status     */
    	mnelem = stbuf.st_size;          /* get message file size(byte) */

    	/*----- allocate memory for the message file -------------------*/
    	udcptr.erptr = (UECB *)calloc( mnelem, sizeof(uchar) );
    	if ( udcptr.erptr == NULL ) { 	/* cannot allocate memory 	*/
      	    fatal_f = INOMSGFL;         /* set fatal error        	*/
      	    mode = U_ECHO;              /* set echo mode          	*/
    	}

    	/*----- file read & check --------------------------------------*/
    	else if ( (read(mfldes, (uchar *)udcptr.erptr, mnelem)) == U_FILEE ) {
      	    fatal_f = INOMSGFL;         /* set fatal error        	*/
    	}
    	de_ret = close(mfldes);         /* close message file     	*/
   }
#endif

/*******************************************************/
/*                MAIN     LOOP                        */
/*******************************************************/

   /* set select menu & flag       */
   menun = 0;
   merflg = C_SWOFF;
   msgx  = MSG_col + udcptr.xoff;
   msgy2 = MSG_lin + udcptr.yoff;
   msgy1 = msgy2 - 1;

   while(TRUE) {        /* loop of process. (endloop mark is '!!!!')    */

/* B.EXTCUR */
	clear();
	refresh();
/* E.EXTCUR */

     /* display head label      */
     ku_ret = kudisply( &udcptr, udcptr.yoff+MNU_lin1,
			 udcptr.xoff+MNU_col1, titol_name,
			 MNU_lng1                     );
     ku_ret = kudisply( &udcptr, udcptr.yoff+MNU_lin2,
			 udcptr.xoff+MNU_col2, udcptr.udfname,
			 MNU_lng2                     );
     ku_ret = kudisply( &udcptr, udcptr.yoff+MNU_lin3,
			 udcptr.xoff+MNU_col3, selmsg,
			 MNU_lng3                     );
     /* display foot label      */
     ku_ret = kudisply( &udcptr, udcptr.yoff+MNU_lin4,
			 udcptr.xoff+MNU_col4, footmsg,
			 MNU_lng4                     );
     /* display menu    */
     o_menun = -1;
     col = MNU_x + udcptr.xoff;
     for(i=0;i<6;i++) {
       line = MNU_y[i] + udcptr.yoff;
       wrc = kudisply(&udcptr,line,col,MNU_data[i],MNU_l);
     };

     /* set conversion mode     */
     kuconvm( (short)(U_FOF) );

     while(TRUE) { /* loop of menu control. (endloop mark is '####')    */
       ioctl( stdin, TCFLSH, clr );
       while(TRUE) {  /* loop of key input. (endloop mark is '$$$$')      */
	 /* revers select menu  */
	 if(menun != o_menun) {
	   col = MNU_x + udcptr.xoff;
	   line = MNU_y[o_menun] + udcptr.yoff;
	   wrc = kudisply(&udcptr,line,col,MNU_data[o_menun],MNU_l);

/* B.EXTCUR */ 
		tputs(enter_reverse_mode,1,putchar);
/* E.EXTCUR */ 

	   line = MNU_y[menun] + udcptr.yoff;
	   wrc = kudisply(&udcptr,line,col,MNU_data[menun],MNU_l);

/* B.EXTCUR */
		tputs(exit_attribute_mode,1,putchar);
/* E.EXTCUR */

	   o_menun = menun;
	 };

	 /* display massage    */
	 if( fatal_f == INOMSGFL )
	   {
	     CURSOR_MOVE(msgy2,msgx);
	     fprintf(stdout,"%s", nomsg);
	   } else {
	     /*    error message display     */
	     if(msgid != U_ENDID) {
	       if(msgid == U_AKMSGN) {
		 nuflg = C_SWON;
		(void)kumsg(&udcptr,msgy2,msgx,(short)(U_ALMSGN));
		(void)kumsg(&udcptr,msgy1,msgx,(short)(U_AKMSGN));
	       } else {
		(void)kumsg(&udcptr,msgy2,msgx,msgid);
	       };
	     };
	 };
	 CURSOR_MOVE( udcptr.yoff+MNU_y[menun] , udcptr.xoff+MNU_x );
	 fflush(stdout);

	 /* input key   */
	 triger = kugetc();
	 /* check  getcode match with mode       */
	 i = kugetcmp( triger );

	 if(i == 2) {
	   /* check DBCS         */
	   i = kugetc();
	   triger = (triger<<8) | i;
	 };

	 /* message erase process.   */
	 if( merflg == C_SWON )  {
	   (void)kumsg(&udcptr,msgy1,msgx,(short)(U_ENDID));
	   (void)kumsg(&udcptr,msgy2,msgx,(short)(U_ENDID));
	   merflg = C_SWOFF;
	 };


	 if(nuflg == C_SWON) {
	   if( (triger == U_RESETKEY) || (triger == U_PF12KEY) ) {

/* B.EXTCUR */
		(void)kureset();
		clear();
		refresh();
/* E.EXTCUR */

#if defined(CNVSTOC)
		return( 0 );
#else
	     exit( 0 );
#endif
	   } else if( (triger == U_CRKEY) || (triger == U_ENTERKEY) ||
	       (triger == U_ACTIONKEY)                                )  {
/* --- */
	     /* Enter key entered process.       */
	     nuflg = C_SWOFF;
	     msgid = U_ENDID;
	     fatal_f = IUSUCC;
	     *(udcptr.dcptr + U_STATUS) = NULL;
	     (void)kumsg(&udcptr,msgy1,msgx,(short)(U_ENDID));
	     (void)kumsg(&udcptr,msgy2,msgx,(short)(U_ENDID));
	     merflg = C_SWOFF;
	     continue;
/* --- */
	   } else {
	     continue;
	   };
	 };

	 msgid = U_ENDID;
	 /* check input key    */
	 if( (triger == ICHI[0]) ||
	     (triger == ICHI[4]) ) {
	     /* select 1 key */
	     menun = 0;
	     break;
	 };
	 if( (triger == NI[0]) ||
	     (triger == NI[4]) ) {
	     /* select 2 key */
	     menun = 1;
	     break;
	 };
	 if( (triger == SAN[0]) ||
	     (triger == SAN[4]) ) {
	     /* select 3 key */
	     menun = 2;
	     break;
	 };
	 if( (triger == YON[0]) ||
	     (triger == YON[4]) ) {
	     /* select 4 key */
	     menun = 3;
	     break;
	 };
	 if( (triger == GO[0]) ||
	     (triger == GO[4]) ) {
	     /* select 5 key */
	     menun = 4;
	     break;
	 };
	 if( (triger == KYU[0]) ||
	     (triger == KYU[4]) ) {
	     /* select 9 key */
	     menun = 5;
	     break;
	 };
	 if( (triger == U_C_DOWN ) ||
	     (triger == U_C_RIGHT) ||
	     (triger == U_TABKEY ) )   {
	   /* cursor down key        */
	   menun++;
	   if(menun > 5) menun = 0;
	   continue;
	 };
	 if( (triger == U_C_UP   ) ||
	     (triger == U_C_LEFT ) ||
	     (triger == U_BTABKEY)  ) {
	   /* cursor up key  */
	   menun--;
	   if(menun < 0) menun = 5;
	   continue;
	 };
	 if( (triger ==  U_ENTERKEY)  || (triger ==  U_CRKEY   ) ||
	     (triger ==  U_ACTIONKEY)                               ) {
	   /* process decision       */
	   break;
	 };

       }; /* end loop of '$$$$'      */

       /* check process */
       if(menun == 5) {
	 /* go to end process   */
	 break;
       };

       /* check data exist      */
       wksht = GETSHORT(udcptr.dcptr + U_UDIL) ;

       if(( menun == 0 ) && ( fatal_f == IUSUCC )) {
	  /* select menu 1      */
	  break;
       };

       if( menun == 4 ) {
	 /* select menue 5      */
	 if( fatal_f ==  IURECOV ) {
	   break;
	 } else if( fatal_f == IUSUCC) {
	   msgid = U_EMSGN;
	   merflg = C_SWON;
	   continue;
	 } else {
	   continue;
	 };
       };

       if(wksht <= U_INITIL) {
	 /* case not exist data */
	 msgid = U_AAMSGN ;
	 continue;
       };

       if(( menun == 1 )  && ( fatal_f  == IUSUCC )) {
	 /* select menu 2      */
	    break;
       } else if(( menun == 2 ) && ( fatal_f == IUSUCC )) {
	 /* select menu 3 */
	  break;
       } else if(( menun == 3 ) && ( fatal_f == IUSUCC )) {
	 /* select menu 4       */
	 break;
       };

     }; /* endlopp #### */

     /* sub routine call        */
     if(menun  == 0) {
       /* call registration handler  '1' */
       ku_ret = kudicadh( &udcptr, sdcptr );
       if( ku_ret == UDSUCC )   {
	 msgid = U_ENDID;
       }
       else if ( ku_ret == UDWRITEE ) {
	 msgid = U_AGMSGN;        /* set message number          */
	 fatal_f = IUFAIL;        /* set fatal error code        */
	 update_f = U_FOF;
       }
       else {
	 msgid  = U_FMSGN;
	 fatal_f = IUFAIL;
	 update_f = U_FOF;
       };
     } else if(menun  == 1) {
       /* call update handler '2'      */
       ku_ret = kudicuph( &udcptr, sdcptr );
       if(ku_ret == UDSUCC) {
	 msgid = udcptr.uurmf;
	 merflg = C_SWON;
       }
       else if ( ku_ret == UDWRITEE ) {
	 msgid = U_AGMSGN;        /* set message number          */
	 fatal_f = IUFAIL;        /* set fatal error code        */
	 update_f = U_FOF;
       }
       else {
	 msgid = U_FMSGN ;        /* set message number          */
	 fatal_f = IUFAIL ;        /* set fatal error code        */
	 update_f = U_FOF;
       }
     } else if( menun == 2 ) {
       /* call ichiranhyou handler      */
       ku_ret = kutable( &udcptr );
     } else if( menun == 3 ) {
       /* call combaind handler */
       ku_ret = kudiccom( &udcptr, sdcptr, comname ) ;
       if(ku_ret == UDSUCC) {
	 msgid = U_ENDID;
       } else {
	 update_f = U_FOF;
       };

     } else if( menun == 4 ) {
       /* recovery process  '5'  */
       (void)kumsg(&udcptr,msgy2,msgx,U_AQMSGN); /* recoverying... */
       ku_ret = kudicrcv(&udcptr) ;/* call recovery handler       */
       switch(ku_ret)
       {
       case  UDSUCC : msgid = U_CMSGN  ;    /* set message number  */
		      fatal_f = IUSUCC  ;   /* set normal code     */
		      user_flg = U_FOF;
		      udcptr.updflg = U_FON;
		      break ;

       case  UDRNVDW: msgid = U_APMSGN ;    /* set message number  */
		      fatal_f = IUSUCC  ;    /* set normal code     */
		      user_flg = U_FOF;
		      udcptr.updflg = U_FON;
		      break ;

       case  UDRDPDW: msgid = U_IMSGN  ;    /* set message number  */
		      fatal_f = IUSUCC  ;    /* set normal code     */
		      user_flg = U_FOF;
		      udcptr.updflg = U_FON;
		      break ;

       case  UDRIMPE: msgid = U_ZMSGN  ;    /* set message number  */
		      fatal_f = IUFAIL  ;    /* set normal code     */
		      user_flg = U_FON;
		      udcptr.updflg = U_FOF;
		      break ;
       };
       if( udcptr.updflg == U_FON ) {
	 /* write user dictionary (temp) */
	 ku_ret = kutmwrt( udcptr.orgfd, udcptr.dcptr, udcptr.ufilsz );
	 if( ku_ret != IUSUCC) {
	   msgid = U_AGMSGN;
	   fatal_f = IUFAIL ;        /* set fatal error code        */
	   update_f = U_FOF;
	 };
       };

     } else if( menun == 5 ) {
       /* end process  '9'  */

/* B.EXTCUR */
	clear();
	refresh();
/* E.EXTCUR */

       if ( ((fatal_f == IUSUCC) && (user_flg == U_FOF))
	  	||         	/* normal terminate process 	*/
	    ((fatal_f == IUFAIL)  && (user_flg == U_FOF))
	  	||              /* sysdict error   		*/
	    ((fatal_f == INOMSGFL) && (user_flg == U_FOF)) )
			    	/* no message file              */
       {
         /* write user dictionary (backup) */
         if( udcptr.updflg == U_FON ) {
             ku_ret = kutmwrt( udcptr.tmpfd, udcptr.thdbuf, udcptr.ufilsz );
             if( ku_ret == IUSUCC) {
	        update_f = U_FON;
	        udcptr.updflg = U_FOF;
             } else {
	        msgid = U_AGMSGN;
	        fatal_f = IUFAIL;        /* set fatal error code        */
	        update_f = U_FOF;
             }
         }
         else {
            if ( stat( udcptr.tmpname, &stbuf ) == 0 ) {
                if ( stbuf.st_size == 0 )
                    unlink( udcptr.tmpname );
            }
         }

  	 flck.l_type = F_WRLCK;
  	 flck.l_whence = flck.l_start = flck.l_len = 0;
  	 for ( i=0; i<U_TRYLOK; i++) {
     	    if ( (de_ret = fcntl( udcptr.orgfd, F_SETLK, &flck )) != -1 )
             	break;
  	 }
  	 if ( de_ret != -1 ) {
	    if ( (lseek(udcptr.orgfd, (long)U_STATUS, 0)) == U_FILEE )
	   	perror("File Seek Error ");
	    if ( (write(udcptr.orgfd, &cmpsts, 1)) == U_FILEE )
	   	perror("File Write Error ");
  	    flck.l_type = F_UNLCK;
  	    for ( i=0; i<U_TRYLOK; i++ ) {
     		if ( (de_ret = fcntl( udcptr.orgfd, F_SETLK, &flck )) != -1 )
             	    break;
  	    }
            de_ret = close( udcptr.orgfd );/* close user dictionary file*/
        }
       }

       /*----- Close the backup file ---------------------------------*/
       if ( udcptr.tmpfd != U_FILEE ) {
            if ( stat( udcptr.tmpname, &stbuf ) == 0 ) {
                if ( stbuf.st_size == 0 )
                    unlink( udcptr.tmpname );
		else
            	    close( udcptr.tmpfd );
            }
       }
       break;
     } else {
	/* no operatoin */
     };

   }; /* endloop !!!!   */
   /* set conversion mode       */
   kuconvm( (short)(U_FON) );

   /*   reset  the  TERMINFO      */
   free(wkname) ;

/* B.EXTCUR */
	(void)kureset();
/* E.EXTCUR */

#if defined(CNVSTOC)
	return( update_f );
#else
   exit(update_f) ;
#endif
}

struct termios saveterm;

kuinit()
{

/* B.EXTCUR */ 
	initscr();
	noecho();
	raw();
	keypad(TRUE);
/* E.EXTCUR */ 

   return(OK);
}

kureset()
{

/* B.EXTCUR */
	echo();
	noraw();
	endwin();
/* E.EXTCUR */

   return(OK);
}
