static char sccsid[] = "@(#)06  1.3  src/bos/usr/lpp/kls/dictutil/hudichnd.c, cmdkr, bos411, 9428A410j 11/30/93 17:05:10";
/*
 * COMPONENT_NAME:	(KRDU) - Korean Dictionary Utility
 *
 * FUNCTIONS:		hudichnd.c
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
 *  Module:       hudichnd.c
 *
 *  Description:  User Dictionary Main Handler
 *
 *  Functions:    main()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

#include <stdio.h>

/*
 *      include file.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <termios.h>    /* POSIX line disciplines                       */
#include <memory.h>     /* Memory Package                               */
#include <fcntl.h>      /* File Control Package                         */
#include <unistd.h>     /*                                              */
#include <signal.h>     /*                                              */
#include <sys/stat.h>   /*                                              */
#include <errno.h>      /* Standard Error Number Package                */
#include <string.h>     /* Stirng Package                               */
#include <sys/lockf.h>	/*                                              */
#include <limits.h>

#include "hut.h"        /* Utility Define File                          */

#define   DUERRMSGCAT      "/usr/lpp/kls/etc/hdict.cat"
                           /* default message catalog name           */

#define   U_SYSDIC         "/usr/lpp/kls/dict/sysdict"
                           /* default system dictionary file name    */

			   /* DISPLAY Position */
#define   MNU_lin1         (  1)
#define   MNU_lin2         (  3)
#define   MNU_col2         (  0)
#define   MNU_lin3         (  5)
#define   MNU_col3         (  3)
#define   MNU_lin4         ( 16)
#define   MNU_col4         (  3)

#define   MSG_lin          ( 15)
#define   MSG_col          (  3)
#define   LNGMAX           ( 44)
#define   MNU_line         ( 17)
#define   MNU_col          ( U_MAXCOL)


/*
 *      this static area is need, if you use getcharcter.
 */
  int    wait_time;                    /* flash wait got term buffer*/
  int    scrollflg = 0;                /* Action just before is     */
  int    fkmaxlen, nkey;               /* Function key string max   */
  struct fkkey fkkeytbl[KEY_MAXC];     /* Function key string array */
  char   trigertable[KEY_MAXC];        /* array of top character of */
  short user_flg;                     /* user dictionary error flag*/

void main(argc, argv)

  int          argc       ; /*  parameter count                         */
  char        *argv[]     ; /*  paramter pointer                        */

{
  FILE         *fp_err;         /* FILE pointer to stderr       */
  int          o_menun;         /* pre menu number      */
  int          menun;           /* input menu number    */
  int          clr = 0;         /* parameteer fo IOCTL  */
  int          alleng;          /* length of file name for display      */
  static uchar wktabl[55] ; /*  user dictionary name wark table         */
  int          MAX_line   ; /* screen max line */
  int          MAX_col    ; /* screen max col  */

  uchar       *calloc()   ;
  char        *getenv()   ;
  void         perror()   ;
  long         lseek()    ;
  int         hudicuph();
  int         hudicadh();
  int         hudicrcv();
  int         hugeti();
  int         hugetc();
  int         humsg();
  int         hugetcmp();
  void	      hudickc();

  char        *usrname    ; /* user dictionary file name                */
  char        *sysname    ; /* system dictionary file name              */
  int          ufldes     ; /* user   dictionary file descripter        */
  int          tmpdes     ; /* temporary file descripter                */
  int          sfldes     ; /* system dictionary file descripter        */
  int          mfldes     ; /* message file descripter                  */
  uint         snelem     ; /* system dictionary file size buffer       */
  uint         mnelem     ; /* message size buffer                      */
  struct stat  stbuf      ; /* file status work buffer                  */
  int          fatal_f    ; /* fatal error flag                         */
  int          tmpsts     ; /* fatal error temporary flag               */
  int          output_device ; /* output device                         */
  int          i, j       ; /* loop counter                             */
  long         x1, y1     ; /* work x, y offset position                */
  long         mode       ; /* echo mode                                */
  long         fildes     ;
  int          de_ret     ; /* editor return code                       */
  int          hu_ret     ; /* utility return code                      */
  int          re_cod     ; /* standard return code                     */
  int          wkbuf      ; /* work buffer                              */
  char         wkstr[3]   ; /* work string buffer                       */
  uchar        *wkname    ; /* work name buffer for user dictionary     */
  uchar        comname[LNGMAX+1]; /* work name for combain process    */
  uchar	       *dicindex;   /* User Dictionary Index Block */
  static uchar tmpname[257];/* temporary file name                      */
  int          namesz     ; /* user dictionary file name size           */
  int          cpypos     ; /* file name copy position                  */
  short        col;             /* work display x axis position */
  short        line;            /* work display y axis position */
  short        msgid;           /* message ID   */
  short        nuflg;           /* data nothing flag    */
  short        update_f;        /* updatede flag        */
  short        merflg;          /* message erse flag    */
  short        msgx;            /* message area x axis  */
  short        msgy1;           /* message area y axis 1        */
  short        msgy2;           /* message area y axis 2        */
  short        sts_err;		/* Status errflg		*/
  int          triger;          /* getkey triger        */
  int          wrc;             /* ret code     */
  int          recv_flag;
  short       i_len ;          /* work short   */
  short       user_flg_dummy;  /* user dictionary error flag    */

  static UDCB  udcptr     ; /* user   dictionary file control block     */
  static SDCB  sdcptr     ; /* system dictionary file control block     */
  static short prev_sts;
  static short wrtsts = 0x00ff ; /* write status code buffer              */
  int	      recv_need = IUSUCC;

  char  *msg_ptr;
  /* menu title name  for display */
  static uchar title_name[40] = "** User Dictionary Utility **";

#define   MNU_col1         (  (U_MAXCOL - strlen(title_name)) / 2 )

  /* usr dictionary file name for display */
  static uchar usrflname[]  = "(dictionary :                                 ";


  /* select massege for display */
  static uchar selmsg[50]     = "Move cursor to desired item and press Enter";

  /* Enter menue for display    */
  static uchar footmsg[20]    = "Enter = Do";

  /* menu select handling key     */
  static int      ONE[2] = {0x31,0xa3b1};
  static int      TWO[2]   = {0x32,0xa3b2};
  static int      THREE[2]  = {0x33,0xa3b3};
  static int      FOUR[2]  = {0x34,0xa3b4};
  static int      FIVE[2]   = {0x35,0xa3b5};
  static int      NINE[2]  = {0x39,0xa3b9};

  /* menu X & length      */
  static short MNU_y[6] = { 7 , 8 , 9 , 10 , 11 , 13 };
  static short MNU_x    = 3;

  /* menu display  " 1 "   */
  static char  MNU_data[6][50] = {
	" 1. Registration                  ",
	" 2. Update(replacement, deletion) ",
	" 3. List                          ",
	" 4. Combination                   ",
	" 5. Recovery                      ",
	" 9. Exit                          "
	};


  huinit();
  setupterm( 0, 1, 0 );            /* initial the TERMINFO                */
  hugeti( );                       /* initial the KEYIN                   */

  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);

  /*
   *  reopen stderr to fp_err
  fp_err = freopen(ERR_FILE,"w",stderr);
   */


  /*
   *  user, system dictionary and message file memory copy
   */

  fatal_f  = IUSUCC ;              /* initial fatal error flag            */
  msgid   = U_ENDID;              /* set normal message number           */
  mode     = U_ECHO ;              /* set echo mode                       */
  update_f = U_FOF;
  nuflg = C_SWOFF;
  recv_flag = U_FOF;

  /*
   *  get characteristics of the display and adapter
   */

   hu_ret = husize( &MAX_line, &MAX_col );   /* screen max line,col     */
   udcptr.ymax = MAX_line;                 /* set up maximum            */

   udcptr.yoff = ( MAX_line - MNU_line )  / 2 ;  /* set up x,y offset   */
   udcptr.xoff = ( MAX_col  - MNU_col  )  / 2 ;

   udcptr.msg_catd = catopen(DUERRMSGCAT, NL_CAT_LOCALE) ;

/*****  catopen does not return -1 when it cannot find catalog file 
   if (udcptr.msg_catd == -1)
   {
     fatal_f = INOMSGFL ;                   ?* set fatal error             *?
     mode = U_ECHO      ;                   ?* set echo mode               *?
   }
*****/

   if(( MAX_line < MNU_line ) || ( MAX_col < MNU_col )) {
    /* Display size error message   */
     uchar   default_msg[] = "display size err" ;
     
     putp( clear_screen );              /*    display clear           */
     msg_ptr = catgets(udcptr.msg_catd, 1, U_DERRMSGN, default_msg);
     printf( "%s\n", msg_ptr );           /*    display error message   */
     (void)resetterm();
     (void)hureset();
     exit( UDDISPE );
   };

   /* user dictionary file open & check */
   if((ufldes = open( argv[1], (O_RDWR | O_NDELAY))) == U_FILEE) {
     /* user dictionary file open error    */
     fatal_f  = IUFAIL ;          /* set fatal error                    */
     user_flg = U_FON ;     /* user dictionary error flag on*/
     msgid   = U_AGMSGN;         /* set error number                   */
   } else {
     /* open successful */

     /* set user dictionary file descripter     */
     udcptr.orgfd = ufldes;

     /* user dictionary file lock           */
     lseek(ufldes,0L,0);
     for(i=0;((lockf(ufldes,F_ULOCK,NULL))==U_FILEE) && (i<U_TRYLOK);i++) {
       fprintf(stderr,"error = %d\n",errno);
     };

     de_ret = fstat(ufldes, &stbuf);
		    /* get user dictionary file status     */
     udcptr.ufilsz = stbuf.st_size;
                    /* get user dictionary file size(byte) */

     /* allocate memory for the user dictionary */
     udcptr.dcptr  = calloc(udcptr.ufilsz, sizeof(uchar)) ;
     udcptr.secbuf = calloc(udcptr.ufilsz, sizeof(uchar)) ;

     if(udcptr.dcptr == NULL)   {
       /* cannot allocate memory        */
       msgid   = U_FMSGN ;      /* set message number                  */
				/* Can not access user dictionary.        */
       fatal_f  = IUFAIL ;        /* set fatal error                     */
       user_flg = U_FON ;     /* user dictionary error flag on*/
     } else {
       /* file read & check */
       if((read(ufldes, udcptr.dcptr, udcptr.ufilsz)) == U_FILEE) {
	 /* user dictionary file read error */
	 msgid   = U_AGMSGN ;    /* set message number                 */
				 /* Can not access user dictionary.          */
	 fatal_f  = IUFAIL ;        /* set fatal error                    */
       } else {
	 /* user dictionary status check */
	 (void)getudstat(udcptr.dcptr, &prev_sts);
  	 if(prev_sts != 0x0000 && prev_sts != 0x000f &&
		 prev_sts != 0x00f0 && prev_sts != 0x00ff) {
	     msgid = U_DMSGN;	/* Set Recovery Status */
				/* The dictionary file needs recovering.  */
	     fatal_f = IURECOV;	/* recovery status                         */
	     user_flg = U_FON;
	     sts_err = U_FON;
	     if((lseek(ufldes, (long)U_STSPOS, 0)) == U_FILEE) {
	       msgid   = U_AGMSGN  ;    /* set message number   */
					/* Can not access user dictionary.  */
	       fatal_f  = IUFAIL ;        /* set fatal error      */

               msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRFSMSGN, "File Seek Error ");
	       perror(msg_ptr) ;
	       user_flg = U_FON;     /* user dictionary error flag on*/
	     } else {
	       if((write(ufldes, &wrtsts, U_STSLEN)) == U_FILEE) {
	         msgid   = U_AGMSGN;  /* set message number   */
				      /* Can not access user dictionary.  */
	         fatal_f  = IUFAIL;    /* set fatal error    */
                 msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRSWMSGN, "Status Write Error");
	         perror(msg_ptr) ;
	         user_flg = U_FON ;     /* user dictionary error flag on*/
	       };
	     };
	 }
	 else {
	   if(prev_sts == 0x00ff) {
	       msgid   = U_AKMSGN ;  /* set message number        */
				     /* The dictionary is now in use        */
	       fatal_f  = IUUPDAT;    /* set fatal error */
			   	      /* now updating    */
	       mode     = U_NECHO ;   /* set non echo mode  */
  	   } else {
	     if((lseek(ufldes, (long)U_STSPOS, 0)) == U_FILEE) {
	       msgid   = U_AGMSGN  ;    /* set message number   */
					/* Can not access user dictionary.  */
	       fatal_f  = IUFAIL ;        /* set fatal error      */
               msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRFSMSGN, "File Seek Error ");
	       perror(msg_ptr) ;
	       user_flg = U_FON;     /* user dictionary error flag on*/
	     } else {
	       if((write(ufldes, &wrtsts, U_STSLEN)) == U_FILEE) {
	         msgid   = U_AGMSGN;  /* set message number   */
				      /* Can not access user dictionary.  */
	         fatal_f  = IUFAIL;    /* set fatal error    */
                 msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRSWMSGN, "Status Write Error");
	         perror(msg_ptr) ;
	         user_flg = U_FON ;     /* user dictionary error flag on*/
	       };
	     };
	   };
	 };
	 /* copy main buffer to second buffer */
	 memcpy(udcptr.secbuf, udcptr.dcptr, udcptr.ufilsz) ;
       };
     };
   };

  /* user dictionary file name convert for display */
  switch(*(char *)argv[2])
  {
    case '1' : usrname = "./usrdict" ; 
	       break ;

    case '2' : usrname = "$HOME/.usrdict" ; 
	       break ;

    default  : usrname = argv[1] ; 
	       break ;
  }

  if (!strcmp(usrname, "//.usrdict"))
	usrname = "/.usrdict" ;

  /* making user dictionary file name for display */
  namesz = strlen(usrname) ;
  wkname = calloc(namesz+1, 1) ;
  for(i=namesz-1,j=namesz-1,cpypos=0;i>=0;i--,j--) {
   if((*(usrname + i) == '/') && ((namesz - i) < LNGMAX)) {
     cpypos = j;
   };
   wkname[j] = *( usrname + i );
  };
  wkname[namesz] = NULL;

  msg_ptr = catgets(udcptr.msg_catd, 1, U_USRFNMSGN, "dummy");
  if (strcmp(msg_ptr, "dummy") != 0)
  {
      strncpy(usrflname, msg_ptr, 20);
      i = strlen(usrflname);
  }
  else
      i = strlen("(dictionary : ");
 
  if(namesz <= LNGMAX)  {
    memcpy(&usrflname[i], wkname, namesz);
    memcpy(comname, wkname, namesz);
    comname[namesz] = NULL;
    j = namesz ;
  } else {
    j = namesz - cpypos ;
    usrflname[i-1] = '-' ;
    memcpy(&usrflname[i], &wkname[cpypos], j );
    comname[0] = '-' ;
    memcpy(&comname[1], &wkname[cpypos], j );
    comname[j + 1] = NULL;
  };
  usrflname[i + j] = ')' ;
  usrflname[i + (j + 1)] = NULL;

  /* set column position for centering dictionary file name */
  alleng = (U_MAXCOL - (i + j + 1)) / 2;
  alleng = ((alleng > 0) ? alleng-- : 0);
  memset(wktabl,0x20,55);
  memcpy( &wktabl[alleng], usrflname , strlen(usrflname) );
  udcptr.udfname = wktabl;

  if(fatal_f != IUFAIL) /* not occured fatal error */
  {
    tmpsts = fatal_f ;             /* store status to temporary buffer */
    /* get environment variable of system dictionary file */
    if((sysname = getenv("KIMSYSDICT")) == NULL)
    {
      fatal_f = IUFAIL ;           /* set fatal error                  */
    }
    else
    /* system dictionary file open & check */
    if((sfldes = open( sysname, (O_RDONLY | O_NDELAY))) == U_FILEE)
    {
      fatal_f = IUFAIL ;           /* set fatal error                  */
    }

    if(fatal_f == IUFAIL) /* failier first file open */
    {
      /* second open system dictionary */
      if((sfldes = open( U_SYSDIC, (O_RDONLY | O_NDELAY))) == U_FILEE)
      {
        /* system dictionary file open error    */
	msgid = U_AHMSGN ;          /* set message number             */
		/* Can not access system dictionary.        */
      }
      else
      {
        fatal_f = tmpsts ;           /* restore normal code            */
      }
    }
  }

  if(fatal_f != IUFAIL) /* not occured fatal error */
  {
    /* open successful */
    de_ret = fstat(sfldes, &stbuf);/* get system dictionary file status  */
    snelem = stbuf.st_size ;    /* get system dictionary file size(byte) */
    /* allocate memory for the system dictionary */
    sdcptr.dcptr = calloc( snelem, sizeof(uchar) );
    if(sdcptr.dcptr == NULL)           /* cannot allocate memory       */
    {
      msgid = U_AHMSGN ;              /* set message number           */
		/* Can not access system dictionary.        */
      fatal_f = IUFAIL ;               /* set fatal error              */
    }
    else /* file read & check */
    if((read(sfldes, sdcptr.dcptr, snelem)) == U_FILEE)
    {
      /* system dictionary file read error */
      msgid = U_AHMSGN ;              /* set message number           */
		/* Can not access system dictionary.        */
      fatal_f = IUFAIL ;               /* set fatal error              */
    }
    de_ret = close(sfldes) ;           /* close system dictionary file */
  }

  /* message file open & check */
  if((mfldes = open( DUERRMSGCAT, (O_RDONLY | O_NDELAY))) == U_FILEE)
  {
    fatal_f = INOMSGFL ;                   /* set fatal error             */
    mode = U_ECHO      ;                   /* set echo mode               */
  }
  else
  {
    de_ret = close(mfldes) ;                    /* close message file     */
  }

   udcptr.rdptr = NULL  ;
   udcptr.wtptr = NULL  ;
   udcptr.updflg = U_FOF ;
   sdcptr.rdptr = NULL  ;
   sdcptr.wtptr = NULL  ;

       /* create temporary file           */

   strcpy(tmpname, argv[1]) ;
   strcat(tmpname, ".tmp") ;
   if((usrname = strrchr((char *)tmpname, '/')) == NULL)
   {
      usrname = (char *)tmpname ;
   }
   else
   {
      usrname++ ;
   }

   if(strlen(usrname) > U_TNMMAX)
   {
      *(usrname + U_TNMOFF) = NULL ;
      strcat(usrname, ".tmp") ;
   }

   udcptr.tmpname = (uchar *)usrname ;
   if( (tmpdes = creat(udcptr.tmpname, (S_IREAD | S_IWRITE))) != U_FILEE)
   {
      de_ret = close(tmpdes) ;      /* close temporary file   */
   }
   else
   {
      fatal_f = IUFAIL  ;
      msgid  = U_FMSGN ;
		/* Can not access user dictionary.        */
   }

   lseek(ufldes,0L,0);
   for(i=0;((lockf(ufldes,F_ULOCK,NULL))==U_FILEE) && (i<U_TRYLOK);i++) {
     fprintf(stderr,"error = %d\n",errno);
   };


/*******************************************************/
/*                MAIN     LOOP                        */
/*******************************************************/

   /* set select menu & flag       */
   menun = 0;
   merflg = C_SWOFF;
   msgx  = MSG_col + udcptr.xoff;
   msgy2 = MSG_lin + udcptr.yoff;
   msgy1 = msgy2 - 1;

   recv_flag = U_FOF;
   if (fatal_f != INOMSGFL && fatal_f != IUFAIL &&
	 (recv_need = hudicrcv(&udcptr, recv_flag)) == UDRNEED)
   {
	 /* call recovery handler       */
       	      msgid = U_DMSGN ;    /* set message number  */
				   /* The dictionary file needs recovering.  */
	      fatal_f = IUFAIL  ;    /* set normal code     */
	      user_flg = U_FOF;
   };

   msg_ptr = catgets(udcptr.msg_catd, 1, U_TITLEMSGN, "dummy");
   if (strcmp(msg_ptr, "dummy") != 0)
      strncpy(title_name, msg_ptr, 40);

   msg_ptr = catgets(udcptr.msg_catd, 1, U_SELMSGN, "dummy");
   if (strcmp(msg_ptr, "dummy") != 0)
      strncpy(selmsg, msg_ptr, 50);

   msg_ptr = catgets(udcptr.msg_catd, 1, U_FOOTMSGN, "dummy");
   if (strcmp(msg_ptr, "dummy") != 0)
      strncpy(footmsg, msg_ptr, 20);

   while(TRUE) {        /* loop of process. (endloop mark is '!!!!')    */
     putp( clear_screen );          /* clear display */

     /* display head label      */
     hu_ret = hudisply( &udcptr, udcptr.yoff+MNU_lin1,
			 udcptr.xoff+MNU_col1, title_name, strlen(title_name));
     hu_ret = hudisply( &udcptr, udcptr.yoff+MNU_lin2,
			 udcptr.xoff+MNU_col2, udcptr.udfname, strlen(udcptr.udfname));
     hu_ret = hudisply( &udcptr, udcptr.yoff+MNU_lin3,
			 udcptr.xoff+MNU_col3, selmsg, strlen(selmsg));
     /* display foot label      */
     hu_ret = hudisply( &udcptr, udcptr.yoff+MNU_lin4,
			 udcptr.xoff+MNU_col4, footmsg, strlen(footmsg));

     /* display menu    */
     o_menun = -1;
     col = MNU_x + udcptr.xoff;
     for(i=0;i<6;i++) {

       msg_ptr = catgets(udcptr.msg_catd, 1, U_MNU1MSGN+i, "dummy");
       if (strcmp(msg_ptr, "dummy") != 0)
          strncpy(MNU_data[i], msg_ptr, 50);

       line = MNU_y[i] + udcptr.yoff;

       wrc = hudisply(&udcptr,line,col,MNU_data[i],strlen(MNU_data[i]));

     };


     while(TRUE) { /* loop of menu control. (endloop mark is '####')    */
       ioctl( stdin, TCFLSH, clr );
       while(TRUE) {  /* loop of key input. (endloop mark is '$$$$')      */
	 /* reverse select menu  */
	 if(menun != o_menun) {
	   col = MNU_x + udcptr.xoff;
	   line = MNU_y[o_menun] + udcptr.yoff;
	   wrc = hudisply(&udcptr,line,col,MNU_data[o_menun],strlen(MNU_data[o_menun]));

	   putp(enter_reverse_mode);
	   line = MNU_y[menun] + udcptr.yoff;

	   wrc = hudisply(&udcptr,line,col,MNU_data[menun],strlen(MNU_data[menun]));
	   putp(exit_attribute_mode);
	   o_menun = menun;
	 }

	 /* display massage    */
	 if( fatal_f == INOMSGFL )
	   {
	     /* message for message file access error */
             uchar nomsg[]      = "message file access error";

	     CURSOR_MOVE(msgy2,msgx);
	     fprintf(stdout,nomsg);
	   } else {
	     /*    error message display     */
	     if(msgid != U_ENDID) {
	       if(msgid == U_AKMSGN) {
			/* The dictionary is now in use        */
		 nuflg = C_SWON;	/* data nothing flag */
		(void)humsg(&udcptr,msgy2,msgx,(short)(U_ALMSGN));
		(void)humsg(&udcptr,msgy1,msgx,(short)(U_AKMSGN));
		/* Your update will not be applied.  Enter = Start F12 = Exit */

	       } else {
		(void)humsg(&udcptr,msgy2,msgx,msgid);
	       };
	     };
	 };
	 CURSOR_MOVE( udcptr.yoff+MNU_y[menun] , udcptr.xoff+MNU_x );
	 fflush(stdout);

	 /* input key   */
	 triger = hugetc();
	 /* check  getcode match with mode       */
	 i = hugetcmp( triger );

	/* make DBCS code */
	 if(i == 2) {
	   /* check DBCS         */
	   i = hugetc();
	   triger = (triger<<8) | i;
	 };

	 /* message erase process.   */
	 if( merflg == C_SWON )  {
	   (void)humsg(&udcptr,msgy1,msgx,(short)(U_ENDID));
	   (void)humsg(&udcptr,msgy2,msgx,(short)(U_ENDID));
	   merflg = C_SWOFF;
	 };


	/* data nothing flag */
	 if(nuflg == C_SWON) {
	   if( (triger == U_RESETKEY) || (triger == U_PF12KEY) ) {
	     /* Cancel key process.        */
	     (void)resetterm();
	     (void)hureset();
	     putp(clear_screen);
	     exit( 0 );
	   } else if( (triger == U_CRKEY) || (triger == U_ENTERKEY) ||
	       (triger == U_ACTIONKEY)                                )  {
	     /* Enter key entered process.       */
	     if((lseek(ufldes, (long)U_STSPOS, 0)) == U_FILEE) {
	       msgid =U_AGMSGN  ;       /* set message number           */
					/* Can not access user dictionary.    */
	       fatal_f = IUFAIL ;       /* set fatal error              */
               msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRFSMSGN, "File Seek Error ");
	       perror(msg_ptr) ;
	       user_flg = U_FON;
	     } else {
	       if((write(ufldes, 0x0000, U_STSLEN)) == U_FILEE) {
		 msgid = U_AGMSGN ;     /* set message number           */
					/* Can not access user dictionary.    */
		 fatal_f = IUFAIL ;     /* set fatal error              */
                 msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRSWMSGN, "Status Write Error");
	         perror(msg_ptr) ;
		 user_flg = U_FON;
	       } else {
		 nuflg = C_SWOFF;
		 msgid = U_ENDID;
		 fatal_f = IUSUCC;
	         setudstat(udcptr.dcptr, 0x0000);
		 (void)humsg(&udcptr,msgy1,msgx,(short)(U_ENDID));
		 (void)humsg(&udcptr,msgy2,msgx,(short)(U_ENDID));
		 merflg = C_SWOFF;
		 continue;
	       };
	     };
	   } else {
	     continue;
	   };
	 };

	 msgid = U_ENDID;
	 /* check input key    */
	 if( (triger == ONE[0]) ||
	     (triger == ONE[1]) ) {
	     /* select 1 key */
	     menun = 0;
	     break;
	 };
	 if( (triger == TWO[0]) ||
	     (triger == TWO[1]) ) {
	     /* select 2 key */
	     menun = 1;
	     break;
	 };
	 if( (triger == THREE[0]) ||
	     (triger == THREE[1]) ) {
	     /* select 3 key */
	     menun = 2;
	     break;
	 };
	 if( (triger == FOUR[0]) ||
	     (triger == FOUR[1]) ) {
	     /* select 4 key */
	     menun = 3;
	     break;
	 };
	 if( (triger == FIVE[0]) ||
	     (triger == FIVE[1]) ) {
	     /* select 5 key */
	     menun = 4;
	     break;
	 };
	 if( (triger == NINE[0]) ||
	     (triger == NINE[1]) ) {
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
		/* exit menu */
		/* go to end process   */
	 break;
       };

       /* check data exist      */
       hudcread(&udcptr, (short)3, (short)NULL);
       dicindex = udcptr.rdptr;
       i_len = getil(dicindex);

       if(( menun == 0 ) && ( fatal_f == IUSUCC )) {
		/* select menu 1      */
		/* registration menu */
	  break;
       };

       if( menun == 4 && fatal_f != INOMSGFL ) {
		/* select menue 5      */
		/* recovery menu */
	 if( recv_need ==  UDRNEED ) {
	   break;
	 } 
	 else {
	   msgid = U_EMSGN;
		/* Nothing is required for recovery.      */
	   merflg = C_SWON;
	   continue;
	 }
       };

       if(i_len <= (U_ILLEN+U_HARLEN+U_NARLEN)) {
	 /* case not exist data */
	 msgid = U_HMSGN ;
		/* There is no data in user dictionary, so you can't update */
	 continue;
       };

       if(( menun == 1 )  && ( fatal_f  == IUSUCC )) {
	 /* select menu 2 - update     */
	    break;
       } else if(( menun == 2 ) && ( fatal_f == IUSUCC )) {
	 /* select menu 3- list  */
	  break;
       } else if(( menun == 3 ) && ( fatal_f == IUSUCC )) {
	 /* select menu 4 - combine       */
	 break;
       };

     }; /* endlopp #### */

     /* sub routine call        */
     if(menun  == 0) {
       /* call registration handler  '1' */
       hu_ret = hudicadh(&udcptr,&sdcptr);
       if( hu_ret == UDSUCC )   {
	 msgid = U_ENDID;
	 if( udcptr.updflg == U_FON )   {
	   udcptr.updflg = U_FOF;
	   update_f = U_FON;
	 };
       } else {
	 msgid  = U_FMSGN;
	 fatal_f = IUFAIL;
	 update_f = U_FOF;
       };
     } else if(menun  == 1) {
       /* call update handler '2'      */
       hu_ret = hudicuph( &udcptr, &sdcptr );

       if(hu_ret == UDSUCC) {
	 msgid = udcptr.uurmf;
	 merflg = C_SWON;
	 if( udcptr.updflg == U_FON ) {
	   /* write user dictionary (temp) */
	   hu_ret = hutmwrt( udcptr.tmpname, udcptr.dcptr,
					     udcptr.ufilsz );
	   if( hu_ret == IUSUCC) {
	     update_f = U_FON;
	     udcptr.updflg = U_FOF ;
	   } else {
	     msgid = U_AGMSGN;
			/* Can not access user dictionary.          */
	     fatal_f = IUFAIL ;        /* set fatal error code        */
	     update_f = U_FOF;
	   };
	 };
       } else {
	 msgid = U_FMSGN ;      /* set message number          */
				/* Can not access user dictionary.        */
	 fatal_f = IUFAIL ;        /* set fatal error code        */
	 update_f = U_FOF;
       };
     } else if( menun == 2 ) {
       /* call table handler      */
       hu_ret = hutable( &udcptr );
     } else if( menun == 3 ) {
       /* call combine handler */
       hu_ret = hudiccom( &udcptr, &sdcptr, comname ) ;
       if(hu_ret == UDSUCC) {
	 msgid = U_ENDID;
       } else {
       };

     } else if( menun == 4 ) {
       /* call recovery handler  */
       recv_flag = U_FON;
       hu_ret = hudicrcv(&udcptr, recv_flag) ;/* call recovery handler       */
       switch(hu_ret)
       {
       case  UDSUCC : msgid = U_CMSGN  ;    /* set message number  */
					    /* All data is collect. */
		      fatal_f = IUSUCC  ;   /* set normal code     */
		      user_flg = U_FOF;	    /* user error flag off */
		      udcptr.updflg = U_FON;
		      break ;

       case  UDRNEED: msgid = U_DMSGN ;    /* set message number  */
		      fatal_f = IUFAIL  ;    /* set normal code     */
		      user_flg = U_FON;
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
		      udcptr.updflg = U_FON;
		      break ;
       };
       if( udcptr.updflg == U_FON ) {
	 /* write user dictionary (temp) */
	 hu_ret = hutmwrt( udcptr.tmpname, udcptr.dcptr,
					   udcptr.ufilsz );
	 if( hu_ret == IUSUCC) {
	   update_f = U_FON;
	   udcptr.updflg = U_FOF ;
	 } else {
	   msgid = U_AGMSGN;
		/* Can not access user dictionary.          */
	   fatal_f = IUFAIL ;        /* set fatal error code        */
	   update_f = U_FOF;
	 };
       };

     } else if( menun == 5 ) {
       /* end process  '9'  */
       putp( clear_screen );            /*     display clear           */
       if(((fatal_f == IUSUCC) && (user_flg == U_FOF))
	  ||                /* normal terminate process */
	 ((fatal_f == IUFAIL)  && (user_flg == U_FOF))
	  ||                /* sysdict error   */
	 ((fatal_f == INOMSGFL) && (user_flg == U_FOF)) )
			    /* no message file                       */
       {
 	 /**************************/
	 /* Update User Dictionary */
	 /* depends on the status. */
 	 /**************************/
 	 /*********************/
	 /* Update Data Block */
 	 /*********************/
   	 lseek(ufldes,0L,0);
         for(i=0;((lockf(ufldes,F_TLOCK,NULL))==U_FILEE) && (i<U_TRYLOK);i++);
         if((lseek(ufldes, (long)0L, 0)) == U_FILEE)  {
           msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRFSMSGN, "File Seek Error");
	   perror(msg_ptr) ;
         };
	 if((write(ufldes, udcptr.dcptr, udcptr.ufilsz)) == U_FILEE)    {
           msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRFWMSGN, "File Write Error ");
	   perror(msg_ptr) ;
         };
   	 lseek(ufldes,0L,0);
   	 for(i=0;((lockf(ufldes,F_ULOCK,NULL))==U_FILEE) && (i<U_TRYLOK);i++) {
     	     fprintf(stderr,"error = %d\n",errno);
   	 };

         /* write user dictionary (temp) */
         if( udcptr.updflg == U_FON ) {
             hu_ret = hutmwrt( udcptr.tmpname, udcptr.dcptr,
				                   udcptr.ufilsz );
             if( hu_ret == IUSUCC) {
	            update_f = U_FON;
	            udcptr.updflg = U_FOF;
             } else {
	            msgid = U_AGMSGN;
			/* Can not access user dictionary.          */
	            fatal_f = IUFAIL ;        /* set fatal error code        */
	            update_f = U_FOF;
             };
         };

       };
       if (sts_err != U_FON ||  recv_flag != U_FON) {
   	 lseek(ufldes,0L,0);
         for(i=0;((lockf(ufldes,F_TLOCK,NULL))==U_FILEE) && (i<U_TRYLOK);i++);
         if((lseek(ufldes, (long)0L, 0)) == U_FILEE)  {
           msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRFSMSGN, "File Seek Error");
	   perror(msg_ptr) ;
         };
	 if((lseek(ufldes, (long)U_STSPOS, 0)) == U_FILEE)  {
           msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRFSMSGN, "File Seek Error");
	   perror(msg_ptr) ;
	 };
	 if((write(ufldes, &prev_sts, U_STSLEN)) == U_FILEE)    {
           msg_ptr = catgets(udcptr.msg_catd, 1, U_ERRFWMSGN, "File Write Error");
	   perror(msg_ptr) ;
	 };
   	 lseek(ufldes,0L,0);
   	 for(i=0;((lockf(ufldes,F_ULOCK,NULL))==U_FILEE) && (i<U_TRYLOK);i++) {
     	     fprintf(stderr,"error = %d\n",errno);
   	 };
       }
       de_ret = close(ufldes);    /* close user dictionary file  */
       break;
     } else {
	/* no operatoin */
     };

   }; /* endloop !!!!   */

   catclose(udcptr.msg_catd);
   /*   reset  the  TERMINFO      */
   free(wkname) ;
   (void)resetterm();
   (void)hureset();
   exit(update_f) ;
};

struct termios saveterm;

huinit()
{
   struct termios getterm;
   int i;

   if (tcgetattr(STDIN, &getterm) == -1) {
      return(ERR);
   }
   memcpy(&saveterm, &getterm, sizeof(saveterm));

   getterm.c_iflag     |= IXOFF;
   getterm.c_iflag     &= ~IXON;
   getterm.c_iflag     &= ~ISTRIP;
   getterm.c_iflag     &= ~INPCK;

   getterm.c_lflag     &= ~ICANON; /* if ICANON is off, it is Raw Mode     */
   getterm.c_lflag     &= ~ECHO;   /* if ECHO   is off, it is No echo mode */

   getterm.c_cflag     &= ~CSIZE;
   getterm.c_cflag     |= CS8;
   getterm.c_cflag     &= ~PARENB;

   getterm.c_cc[VINTR]  = CINTR;
   getterm.c_cc[VQUIT]  = CTRL_BACKSLASH;
   getterm.c_cc[VERASE] = CERASE;
   getterm.c_cc[VKILL]  = CKILL;
   /* if ICANON is set, this mean EOF character */
   getterm.c_cc[VEOF]   = MINIMUM_CHR; 
   /* if ICANON is set, this mean EOL character */
   getterm.c_cc[VEOL]   = WAIT_TIME;

   if (tcsetattr(STDIN, TCSADRAIN, &getterm) == -1) {
      return(ERR);
   }
   return(OK);
}

hureset()
{
   if (tcsetattr(STDIN, TCSADRAIN, &saveterm) == -1) {
      return(ERR);
   }
   return(OK);
};
