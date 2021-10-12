static char sccsid[] = "@(#)58	1.5.1.1  src/bos/usr/lpp/jls/dictutil/kutable.c, cmdKJI, bos411, 9428A410j 7/23/92 01:30:40";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kutable
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
 * MODULE NAME:         kutable
 *
 * DESCRIPTIVE NAME:    User Dictionary Table handler
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
 * FUNCTION:            user dictionary table handler
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        6554  Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kutable
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kutable ( udcbptr )
 *
 *  INPUT               *udcbptr: pointer to User Dictionary Control Block
 *
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         IUSUCC  : sucess return
 *
 * EXIT-ERROR:          IUFAIL  : error
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              kudisply
 *                              kudcread
 *                              kudicmp
 *                              kudicycr
 *                              kuipfld
 *                              kufout
 *                              kushow
 *                              kufnc
 *                      Standard Liblary.
 *                              memcmp  :
 *                              memcpy  :
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
/*#include <memory.h>*/ /* Memory package.                              */
#include <fcntl.h>      /* File contorol Package.                       */
#include <errno.h>      /* system call error ID.                        */
#include <sys/stat.h>

#include <locale.h>     /*                                              */
#include <nl_types.h>   /*                                              */
#include <limits.h>     /*                                              */

/*
 *      include Kanji Project.
 */
#include "kje.h"
#include "kut.h"        /* Kanji Utility Define File.                   */

extern  int     cnvflg;         /* Conversion Type Code                 */

/*
 *      work define.
 */
#if defined(PARMADD)
extern  char    *printdv;   /* Printer Device Pointer                   */
#endif

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
int     kutable ( udcbptr )

UDCB    *udcbptr;       /* pointer to User Dictionary Control Block     */
{
        void    kudcread();     /* user dictionary read.                */
        void    kudicycr();     /* 7bit -> PC conversion.               */
	int     kugetcmp();     /* get cusor move point.*/
	int     kumsg();        /* display message.     */
	int     kudisply();     /* display character string     */
	int     kuipfld();      /* input string data    */
	int     kufout();       /* output user dictionary to file       */
	int     kushow();       /* display data */
	int     kufnc();        /* check file name      */

	UDCS    *topptr;        /* pointer to top of UDCS.              */
        UDCS    *lastptr;       /* pointer to last of UDCS.             */
        UDCS    *curptr;        /* pointer to current of UDCS.          */
        UDMS    *dmappt;        /* pointer to UDMS.                     */

        uchar   *ipt;           /* pointer to index area.               */
        uchar   *dpt;           /* pointer to data area.                */
	uchar   yomidata[U_YOMFLD]; /* yomi (PC code) string.           */
	uchar   knjdata[U_GOKFLD];  /* Kanji (PC code) string.          */
        uchar   cchar;          /* sampl data for check.                */

        short   indlen;         /* length of index area of user dic.    */
        short   datlen;         /* length of data  area of user dic.    */
        short   dllen;          /* length of data  DL   of user dic.    */
        short   indpos;         /* position of index area.              */
        short   datpos;         /* position of data  area.              */
        short   dlpos;          /* position of DL    area.              */

        short   rrn;            /* record number ob user dictionary.    */
        short   kanalen;        /* length of kanadata. (7bit code)      */
        short   yomilen;        /* length of yomidata. (PC   code)      */
        short   knjlen;         /* length of Kanji data.                */
        short   posc;           /* position counter.                    */
        short   uplen;          /* active field character length.       */
        short   msgid;          /* message ID.                          */
        short   gyosu;          /* maximum input line number.           */

	int     triger;         /* triger key.  */
	short   col;            /* input field x axis   */
	short   line;           /* input field y axis   */
	short   dtlen;          /* length of input field        */
	short   ipmode;         /* input mode.  */

	int     rc;             /* return code. */
        int     wrc;            /* return code.                         */
	int     i;              /* work variable.                       */
	char    outfname[20];   /* output file name     */
	char    *pwd;           /* dir  */
	char    *tmpf;          /* pointer to temporary file name       */
	uchar   merflg;         /* message area erase flag.             */
	uchar   nebflg;         /* numeirc enterd break flag.           */
	int     fd;             /* file discripter      */

	/* parameter for system */
	char    pkick[1024];
	char	prcode[16];

	/* menu select handling key     */
	static int      ICHI[5] = {0x31,0x8250,0x82ca,0x836b,0xc7};
	static int      NI[5]   = {0x32,0x8251,0x82d3,0x8374,0xcc};
	static int      SAN[5]  = {0x33,0x8252,0x82a0,0x8341,0xb1};

	/* head label data X axis.      */
	static int     hl3_x[2] = { 12 , 0 };

	/* head label data Y axis.      */
	static int     hl3_y[2] = { 1 , 3 };

	/* head label data length.      */
	static int     hl3_l[2] = {  30 ,  54 };

	/* head label data.     */
	static char    hl3_d[32];

	/* message datd X axis. */
	short     msg_x = 0;

	/* message datd Y axis. */
	/* change IBM-J         */ 
	short     msg_y = 14;
	/*
	short     msg_y = 15;
	*/

	/* function number data X,Y axis.       */
	/* change IBM-J         */ 
	static int     pf_x1 = 0;
	static int     pf_x2 = 12;
	static int     pf_y1 = 16;
	static int     pf_y2 = 16;
	/*
	static int     pf_x = 40;
	static int     pf_y = 16;
	*/

	/* function number length.      */
	/* change IBM-J         */ 
	static int     pf_l1 = 10;
	static int     pf_l2 = 7;
	/*
	static int     pf_l = 12;
	*/

	/* function number data.        */
	/* change IBM-J         */ 
	static  char   pf_d1[16];
	static  char   pf_d2[12];

	/* menu area X,Y axis & datalegth & data        */
	int             menun;
	static int      menux = 3;
	static int      menuy[3] = { 8 , 9 , 10 };
	static int      menul= 13;

	static char     menud[3][16];

	static int      filex = 17;
	static int      filel = 16;

	/* text data X,Y axis & data & data length      */
	static int      textx = 3;
	/* change IBM-J         */ 
	static int      texty[2] = {5,6};
	/*
	static int      texty[2] = {12,13};
	*/

	static int      textl[2] = {36,28};
	/* change IBM-J         */ 
	static char     textd[2][40];


        strcpy(hl3_d, CU_MNLTIT);
        strcpy(pf_d1, CU_MNFMSS);
        strcpy(pf_d2, CU_MNKEY3);

        strcpy(menud[0], CU_MNPDT1);
        strcpy(menud[1], CU_MNPDT2);
        strcpy(menud[2], CU_MNPDT3);

        strcpy(textd[0], CU_MNPMSG);
        strcpy(textd[1], CU_MNQMSG);


        /* 0.
         *     display character dada  and  allocate display map,data.
         */
	/* set default file name        */
#if defined(TMPDIR)                             /* 1991.05.11           */
	memset(outfname,'\0',20);               /* 1991.05.11           */
	strncpy(outfname,U_DICLST,(int)20);     /* 1991.05.11           */
#endif                                          /* 1991.05.11           */
	memcpy(outfname,"dict.list           ",(int)20);
	outfname[19] = NULL;

	/* erase display area.  */

/* B.EXTCUR */
	clear();
	refresh();
/* E.EXTCUR */

	/* display head label.  */
	col  = hl3_x[0] + udcbptr->xoff;
	line = hl3_y[0] + udcbptr->yoff;
	wrc = kudisply(udcbptr,line,col,hl3_d,hl3_l[0]);

	col  = hl3_x[1] + udcbptr->xoff;
	line = hl3_y[1] + udcbptr->yoff;
	wrc = kudisply(udcbptr,line,col,(char *)(udcbptr->udfname),hl3_l[1]);

	/* display function number.     */
	/* change IBM-J         */ 
	col = pf_x1 + udcbptr->xoff;
	line = pf_y1 + udcbptr->yoff;
	wrc = kudisply(udcbptr,line,col,pf_d1,pf_l1);
	col = pf_x2 + udcbptr->xoff;
	line = pf_y2 + udcbptr->yoff;
	wrc = kudisply(udcbptr,line,col,pf_d2,pf_l2);
	/*
	col = pf_x + udcbptr->xoff;
	line = pf_y + udcbptr->yoff;
	wrc = kudisply(udcbptr,line,col,pf_d,pf_l);
	*/

	/* display text data    */
	col = textx + udcbptr->xoff;
	line = texty[0] + udcbptr->yoff;
	wrc = kudisply(udcbptr,line,col,textd[0],textl[0]);
	line = texty[1] + udcbptr->yoff;
	wrc = kudisply(udcbptr,line,col,textd[1],textl[1]);

	/* get line number of input field.      */
	gyosu = udcbptr->ymax - 6;

	/* allocate display map.        */
        dmappt = (UDMS *)malloc( sizeof(UDMS) );
        if( dmappt == NULL )  return( IUFAIL );
	dmappt->fld = (UDMFLD *)malloc( sizeof(UDMFLD) * gyosu );
        if( dmappt->fld == NULL )  return( IUFAIL );



	/*  1.
         *      make user dictionary buffer.
         */
	/* init pointer to user dictionary buffer.      */
        topptr  = NULL;
        lastptr = NULL;
        curptr  = NULL;

	/* get pointer to index area.   */
        kudcread(udcbptr , (short)3 , rrn);
        ipt = udcbptr->rdptr;

	/* get length of index area.    */
        indlen = GETSHORT( ipt );

	/* index position init. */
        indpos = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN;

	/* loop of index. (endloop mark is '$$$$')      */
        while(TRUE)  {

	    /* check !  end of index area.      */
            if( indpos >= indlen ) break;

	    /* set index position to RRN.       */
            indpos   = indpos + *(ipt + indpos);

	    /* get RRN. */
            rrn = *(ipt + indpos);

	    /* set index position to next KNL.  */
            indpos   = indpos + U_RRNLEN;

            /*  2.
             *      read data from user dictionary.
             */
            /* get pointer to data area.                                */
            kudcread(udcbptr , (short)4 , rrn);
            dpt = udcbptr->rdptr;

            /* get length of data area.                                 */
            datlen = GETSHORT( dpt );

            /* data position init.  ( to start of entry)                */
            datpos = U_RLLEN;

            /* loop of data area. (endloop mark is '@@@@')              */
            while(TRUE)  {
                /* check ! end of data area,                            */
                if( datpos >= datlen )  break;

                /* get length of yomigana form KNL.                     */
                kanalen = *(dpt + datpos) - U_KNLLEN;

                /* set data position to start of yomidata.              */
                datpos = datpos + U_KNLLEN;

                /* get sampl data.                                      */
                cchar = *(dpt + datpos);

                /* yomigana convert 7bit code to PC Kanji code.         */
                kudicycr( (uchar *)(dpt + datpos) , kanalen
                        , yomidata , &yomilen );

                /* set data position to start of DL.                    */
                datpos = datpos + kanalen;

                /* init kanji position counter.                         */
                posc = 0;

                /* get length of DL area.                               */
                dllen = GETSHORT( (dpt + datpos) );

                /* DL position init.  ( to start of Kanji data)         */
                dlpos = datpos + U_DLLEN + U_RSVLEN;

                /* set data position to start of next entry.            */
                datpos = datpos + dllen;

                /* loop of Yomi  data. (endloop mark is '!!!!')         */
                if( cchar != U_OMIT_C )  {
                  while( TRUE )  {
                    /* check ! end of data area.                        */
                    if( dlpos >= datpos )  break;

                    /* init length of Kanji.                            */
                    knjlen = 0;

                    /* loop of kanji convert. (endloop mark is '####')  */
                    while(TRUE)  {
                        /* check ! end of kanji data.                   */
                        if( *(dpt + dlpos) > U_CONV)  break;

                        /* check ! convert type & convert of high byte. */
                        if( *(dpt + dlpos) > U_7PCCC )
                            knjdata[knjlen] = *(dpt + dlpos) | U_7PCCU;
                        else
                            knjdata[knjlen] = *(dpt + dlpos) | U_7PCCL;

                        /* convert of low byte.                         */
                        dlpos  = dlpos + 1;
                        knjlen = knjlen + 1;
                        knjdata[knjlen] = *(dpt + dlpos);

                        /* count up.                                    */
                        dlpos = dlpos + 1;
                        knjlen = knjlen + 1;

                    };  /* #### endloop                                 */
                    /* last 2 byte convert.                             */
                    knjdata[knjlen] = *(dpt + dlpos);
                    dlpos  = dlpos + 1;
                    knjlen = knjlen + 1;
                    knjdata[knjlen] = *(dpt + dlpos);

                    /* count up.                                       */
                    dlpos  = dlpos + 1;
                    knjlen = knjlen + 1;

                    /* set DL position to next kanji data.              */
                    dlpos = dlpos + U_RSVLEN;

                    /* alloc dictionary buffer.                         */
                    curptr = (UDCS *)malloc( sizeof(UDCS) );
		    if( curptr == NULL )  return( IUFAIL );

                    /* first allocate process.                          */
                    if( topptr == NULL )  {
                        /* save first pointer.                          */
                        topptr = curptr;

                        /* set pointer to previous.                     */
                        curptr->pr_pos = NULL;
                    }
                    else  {
                        /* set pointer to previous.                     */
                        lastptr->nx_pos = curptr;
                        /* set pointer to next.                         */
                        curptr->pr_pos = lastptr;
                    };

                    /* set status information of UDCS.                  */
                    curptr->status = U_S_INIT;

                    /* set yomi data of UDCS.                           */
                    curptr->yomilen = yomilen;
                    for(i=0;i<yomilen;i++)  curptr->yomi[i] = yomidata[i];
                    for(i=yomilen;i<U_B_YOMI;i=i+2)  {
                        curptr->yomi[i]    = C_SPACEH;
                        curptr->yomi[i+ 1] = C_SPACEL;
                    }

                    /* set position  of UDCS.                           */
                    posc = posc + 1;
                    curptr->pos = posc;

                    /* set kanji data of UDCS.                          */
                    curptr->kanlen = knjlen;
                    for(i=0;i<knjlen;i++)  curptr->kan[i] = knjdata[i];
                    for(i=knjlen;i<U_B_KAN;i=i+2)  {
                        curptr->kan[i]    = C_SPACEH;
                        curptr->kan[i+ 1] = C_SPACEL;
                    }

                    /* keep last pointer to UDCS.                       */
                    lastptr = curptr;
                    lastptr->nx_pos = NULL;

                  };  /* !!!! endloop                                     */
                };

            };  /* @@@@ endloop                                         */

         };  /* $$$$ endloop                                            */

        /* 2.
	 *      process loop start
         */

	/* set select menu & flag       */
	menun = 0;
	merflg = C_SWOFF;
	nebflg = C_SWOFF;
	msgid = U_ENDID;
	msg_x = msg_x + udcbptr->xoff;
	msg_y = msg_y + udcbptr->yoff;

	/* set conversion mode    */
	kuconvm( (short)(U_FOF) );

	/* process loop. (endloop mark is '%%%%')       */
        while(TRUE)  {
	  /* display file input arear   */
	  col = filex + udcbptr->xoff;
	  line = menuy[2] + udcbptr->yoff;
	  wrc = kudisply(udcbptr,line,col,outfname,filel);
	  CURSOR_MOVE(line,col);
	  CURSOR_MOVE(line,col-1);
	  printf("[");
	  CURSOR_MOVE(line,col+filel);
	  printf("]");

	  /* display massage    */
	  (void)kumsg(udcbptr,msg_y,msg_x,msgid);

	  while(TRUE) { /* inkey loop. (endloop mark is '!!!!')    */
	    /*
	     *    display menu
	     */
	    col = menux + udcbptr->xoff;
	    for(i=0;i<3;i++) {
	      line = menuy[i] + udcbptr->yoff;
	      if(i == menun) {

/* B.EXTCUR */
		tputs(enter_reverse_mode,1,putchar);
/* E.EXTCUR */

	      };
	      wrc = kudisply(udcbptr,line,col,menud[i],menul);
	      if(i == menun) {

/* B.EXTCUR */
		tputs(exit_attribute_mode,1,putchar);
/* E.EXTCUR */

	      };
	    };

	    /* message erase process.   */
	    if( merflg == C_SWON )  {
	      msgid = U_ENDID;
	      (void)kumsg(udcbptr,msg_y,msg_x,msgid);
	      merflg = C_SWOFF;
	    };
	    CURSOR_MOVE(udcbptr->yoff+menuy[menun],col);
	    fflush(stdout);

	    /* check key patern */
	    if(nebflg == C_SWON) {
	      nebflg = C_SWOFF;
	      break;
	    };

	    /* get input key      */
	    triger = kugetc();
	    /* check  getcode match with mode       */
	    i = kugetcmp( triger );
	    if(i == 2) {
	      /* check DBCS         */
	      i = kugetc();
	      triger = triger<<8 | i;
	    };
	    /* check input key    */
	    if( (triger == ICHI[0]) ||
		(triger == ICHI[4]) ) {
		/* select 1 key */
		menun = 0;
		nebflg = C_SWON;
		continue;
	    };
	    if( (triger == NI[0]) ||
		(triger == NI[4]) ) {
		/* select 2 key */
		menun = 1;
		nebflg = C_SWON;
		continue;
	    };
	    if( (triger == SAN[0]) ||
		(triger == SAN[4]) ) {
		/* select 3 key */
		menun = 2;
		nebflg = C_SWON;
		continue;
	    };
	    if( (triger == U_C_DOWN)  ||
		(triger == U_C_RIGHT) ||
		(triger == U_TABKEY)   )   {
	      /* cursor down key        */
	      menun++;
	      if(menun > 2) menun = 0;

              /* message erase */
              if((msgid == U_BAMSGN ) || (msgid == U_AZMSGN)) {
                msgid = U_ENDID;
                (void)kumsg(udcbptr,msg_y,msg_x,msgid);
              }

	      continue;
	    };
	    if( (triger == U_C_UP)    ||
		(triger == U_C_LEFT)  ||
		(triger == U_BTABKEY)   )   {
	      /* cursor up key  */
	      menun--;
	      if(menun < 0) menun = 2;

              /* message erase */
              if((msgid == U_BAMSGN ) || (msgid == U_AZMSGN)) {
                msgid = U_ENDID;
                (void)kumsg(udcbptr,msg_y,msg_x,msgid);
              }

	      continue;
	    };
	    if( (triger ==  U_ENTERKEY)  || (triger ==  U_CRKEY   ) ||
		(triger ==  U_ACTIONKEY) || (triger ==  U_PF3KEY )  ) {
	      /* process decision       */
	      break;
	    };

	    merflg = C_SWON;
	  }; /* end loop of '!!!!'      */

	  /* check process      */
	  if(triger == U_PF3KEY) {
	    break;
	  };

	  if(menun == 0) {
	    /* table display    */
	    wrc = kushow(udcbptr,dmappt,topptr,lastptr);
	    /* display head label.  */
	    col  = hl3_x[0] + udcbptr->xoff;
	    line = hl3_y[0] + udcbptr->yoff;
	    wrc = kudisply(udcbptr,line,col,hl3_d,hl3_l[0]);

	    col  = hl3_x[1] + udcbptr->xoff;
	    line = hl3_y[1] + udcbptr->yoff;
	    wrc = kudisply(udcbptr,line,col,(char *)(udcbptr->udfname),hl3_l[1]);

	    /* display function number.     */
	/* change IBM-J         */ 
	    col = pf_x1 + udcbptr->xoff;
	    line = pf_y1 + udcbptr->yoff;
	    wrc = kudisply(udcbptr,line,col,pf_d1,pf_l1);
	    col = pf_x2 + udcbptr->xoff;
	    line = pf_y2 + udcbptr->yoff;
	    wrc = kudisply(udcbptr,line,col,pf_d2,pf_l2);
	/*
	    col = pf_x + udcbptr->xoff;
	    line = pf_y + udcbptr->yoff;
	    wrc = kudisply(udcbptr,line,col,pf_d,pf_l);
	*/

	    /* display text data    */
	    col = textx + udcbptr->xoff;
	    line = texty[0] + udcbptr->yoff;
	    wrc = kudisply(udcbptr,line,col,textd[0],textl[0]);
	    line = texty[1] + udcbptr->yoff;
	    wrc = kudisply(udcbptr,line,col,textd[1],textl[1]);
	    continue;
	  };

	  if(menun == 1) {  /* if of menu 2 or 3 (endif mark is '++++') */
#if defined(TMPDIR)
	    tmpf = tempnam(U_TMPDIR,"kuprn");
#else
	    /* case print out   */
	    /* make temporary file      */
	    pwd = (char *)getcwd((char *)NULL,64);
	    if(pwd == NULL) {
	      msgid = U_BBMSGN;
	      continue;
	    };
	    tmpf = tempnam(pwd,"kuprn");
#endif
	    if(tmpf == NULL) {
	      msgid = U_BBMSGN;
	      continue;
	    };
	    msgid = U_BAMSGN;

	    /* output to temp file      */
	    fd = creat(tmpf,(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
	    if(fd < 0) {
	      msgid = U_BBMSGN;
	      continue;
	    };
	    rc = kufout(fd,topptr);
	    close(fd);
	    /* set msg  */
	    if(rc != IUSUCC) {
	      msgid = U_BBMSGN;
	      continue;
	    };

	    /* set system parameter to print    */
	if ( cnvflg == U_EUC )
		strcpy(prcode, U_EUCCD);
	else
		strcpy(prcode, U_SJISCD);
#if defined(PARMADD)
	    sprintf(pkick,"sh -c '(cat %s | qprt %s -X %s; rm %s)'",
			  tmpf,printdv, prcode, tmpf);
#else
	    sprintf(pkick,"sh -c '(cat %s | qprt -X %s; rm %s)'",
			tmpf, prcode, tmpf);
#endif
	    wrc = system(pkick);
	    if(wrc != 0) {
	      msgid = U_BCMSGN;
	      continue;
	    };
	  } else {
	    /* case file output */
	    /* get output file name     */
	    col  = filex + udcbptr->xoff;
	    line = menuy[2] + udcbptr->yoff;
	    dtlen = 16;
	    ipmode = T_FILE;
	    msgid = U_ENDID;

	    /* set conversion mode    */
	    kuconvm( (short)(U_FON) );

	    while(TRUE) { /* loop of file out. (endloop mark is '####') */
	      (void)kumsg(udcbptr,msg_y,msg_x,msgid);
	      msgid = U_ENDID;
	      while(TRUE) {
		triger = kuipfld(udcbptr,line,col,outfname,
					  dtlen,ipmode,&uplen,C_SWOFF,U_SJIS);

		if( (triger == U_C_DOWN  ) || (triger == U_C_UP     ) ||
		    (triger == U_TABKEY  ) || (triger == U_BTABKEY  ) ||
		    (triger == U_ENTERKEY) || (triger == U_ACTIONKEY) ||
		    (triger == U_CRKEY   ) || (triger == U_PF3KEY   ) ) {
		   break;
		};
	      };
	      /* cursor move    */
	      if( (triger == U_C_UP   ) ||
		  (triger == U_BTABKEY)   )  {
		menun--;
		break;
	      };
	      if( (triger == U_C_DOWN ) ||
		  (triger == U_TABKEY )   )  {
		menun++;
		if(menun > 2) menun = 0;
		break;
	      };
              /* end process */
	      if( triger == U_PF3KEY ) {
		break;
	      };

	      /* check input data length        */
	      if(uplen <= 0) {
		msgid = U_ARMSGN;
		continue;
	      };

	      /* open outfile   */
	      outfname[uplen] = NULL;

	      /* check input name       */
	      if(kufnc(outfname) != IUSUCC) {
		msgid = U_BFMSGN;
		continue;
	      };

	      fd = open(outfname,O_WRONLY);
	      if(fd < 0) {
		/* case open error      */
		if(errno == ENOENT) {
		  /* file not exit      */
		  fd = creat(outfname,(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
		} else {
		  /* file is exist, but permision error */
		  msgid = U_BBMSGN;
		  continue;
		};
	      } else {
		/* case open success	*/
		msgid = U_ASMSGN;
		(void)kumsg(udcbptr,msg_y,msg_x,msgid);
		msgid = U_ENDID;
		/* check over write	*/
		while(TRUE) {
		  triger = kugetc();
		  if( (triger == U_RESETKEY ) ||
		      (triger == U_PF12KEY  ) ||
		      (triger == U_ACTIONKEY) ||
		      (triger == U_CRKEY    ) ||
		      (triger == U_ENTERKEY ) ) {
		    break;
		  };
		};
		if( (triger == U_RESETKEY ) ||
		    (triger == U_PF12KEY  ) ) {
		  /* cancel output	*/
		  continue;
		} else {
		  /* over write OK!	*/
		  /* truncate  file	*/
		  rc = ftruncate(fd,(unsigned long)(0));
		};
	      };

	      /* output to temp file      */
	      rc = kufout(fd,topptr);
	      close(fd);
#if defined(TMPDIR)
if ( 1 ) {
char    *chmod;
	      chmod = pkick;
	      memset(chmod,'\0',1024);
	      sprintf(chmod,"chmod 0666 %s",outfname);
	      system(chmod);
};
#endif
	      /* set msg  */
	      if(rc != IUSUCC) {
		msgid = U_BBMSGN;
		continue;
	      } else {
		msgid = U_AZMSGN;
	      };
	      break;
	    };  /* #### endloop */

	    /* set conversion mode    */
	    kuconvm( (short)(U_FOF) );
            /* end process */
	    if( triger == U_PF3KEY ) {
	       break;
	    };

	  }; /* ++++ endif */

	}; /* %%%% endloop */

        /*
         *      free display map area and dictionary area.
         */
	/* free display data.   */
	/*
	 * fprintf(stderr,"free pointer list\n");
	 * fprintf(stderr,"dmappt->fld = %0x\n",dmappt->fld);
	 * fprintf(stderr,"dmappt      = %0x\n",dmappt);
	 */

	/* free display map.    */
        free( (char *)(dmappt->fld) );
        free( (char *)dmappt );

	/* free dictionary buffer.      */
	/*
	 * while(TRUE)  {
	 *  curptr = lastptr->pr_pos;
	 *  free( (char *)lastptr );
	 *  fprintf(stderr,"lastptr  = %0x\n",lastptr);
	 *  if( curptr == NULL ) break;
	 *  lastptr = curptr;
	 * };
	 */

	/* return.      */
        return( rc );

}
