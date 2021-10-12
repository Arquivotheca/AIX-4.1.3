static char sccsid[] = "@(#)34	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudicuph.c, cmdKJI, bos411, 9428A410j 7/23/92 01:23:44";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudicuph
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * comb/m/messined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/********************* START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:         kudicuph
 *
 * DESCRIPTIVE NAME:    User Dictionary Update handler
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1989, 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            user dictionary update handler
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        12190  Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kudicuph
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudicuph ( udcbptr , sdcbptr )
 *
 *  INPUT               *udcbptr: pointer to User Dictionary Control Block
 *                      *sdcbptr: pointer to System Dictionary Control Block
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         UDSUCC  : sucess return
 *
 * EXIT-ERROR:          IUFAIL  : error
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Kanji Project Subroutines.
 *                              kuipfld : Wait for an event.
 *                              kudisply: Displa stringth.
 *                              kudcread: User Dictionary read.
 *                              kudicymc: PC -> 7bit conversion.
 *                              kudicycr: 7bit -> PC conversion.
 *                              kudiccs : System Dictionary Check.
 *                              kudicscb: User Dictionary Check.
 *                              kudicupm: User Dictionary Buffer Display.
 *                              kudicupc: User Dictionary Buffer update.
 *                              kudicdlb: User Dictionary Buffer
 *                                                   Delete Status Code Set.
 *                              kudicueh: Kanji User Dictionary
 *                                                   End Update Handler
 *                              kuhkfc  : Hiragana or Katakana Check Routine.
 *                              kuconvm : Set conversion mode
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

#include <locale.h>     /*                                              */
#include <nl_types.h>   /*                                              */
#include <limits.h>     /*                                              */

/*
 *      include Kanji Project.
 */
#include "kje.h"        	/* Kanji Utility Define File.           */
#include "kut.h"                /* Kanji Utility Define File.   	*/

extern  int     cnvflg;   	/* Conversion Type Code                 */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

/*
 *      Please Descripte This Module.
 */
int     kudicuph ( udcbptr , sdcbptr )

UDCB    *udcbptr;       /* pointer to User Dictionary Control Block     */
SDCB    *sdcbptr;       /* pointer to System Dictionary Control Block   */

{
        void    kudcread();     /* user dictionary read.                */
        void    kudicycr();     /* 7bit -> PC conversion.               */
        void    kudicymc();     /* PC -> 7bit conversion.               */
#if defined(_OLD_AIX320)
        void    kudiccs();      /* system dictionary check.             */
#else
        void    kudiccs3();      /* system dictionary check.             */
#endif /* defined(_OLD_AIX320) */
        void    kudicscb();     /* user dictionary check.               */
	void    kuconvm();      /* Set conversion mode  */
	int     kugetcmp();     /* get cursor move point.               */
	int     kumsg();        /* display message.     */
        int     kudicupm();     /* user dictionary buffer display       */
        int     kudicupc();     /* user dictionary buffer update        */
        int     kudicdlb();     /* user dictionary buffer               */
                                /*         delete status code set       */
        int     kudicueh();     /* user dictionary end update handler   */
        int     kuhkfc();       /* Hiragana or Katakana check           */
	int     kudisply();     /*      */
	int     kuipfld();      /*      */
	int     kugetc();       /*      */

        DIFLD   *df;            /* pointer to DIFLD.                    */
        UDCS    *topptr;        /* pointer to top of UDCS.              */
        UDCS    *lastptr;       /* pointer to last of UDCS.             */
        UDCS    *curptr;        /* pointer to current of UDCS.          */
        UDCS    *scp;           /* work pointer to UDCS.                */
        UDCS    *scp2;          /* work pointer to UDCS.                */
        UDCS    *scp3;          /* work pointer to UDCS.                */
        UDMS    *dmappt;        /* pointer to UDMS.                     */

        uchar   *ipt;           /* pointer to index area.               */
        uchar   *dpt;           /* pointer to data area.                */
        uchar   merflg;         /* message area erase flag.             */
	uchar   yomidata[U_YOMFLD];     /* yomi (PC code) string.       */
	uchar   knjdata[U_GOKFLD];      /* Kanji (PC code) string.      */
	uchar   hkdata[U_GOKFLD];       /* Kanji conversion data.       */
        uchar   upmode;         /* update mode.                         */
	uchar   ipdata[U_GOKFLD];       /* pointer to update data.      */
	uchar   cchar;          /* sampl data for check.                */
        uchar   empflg;         /* all dic' buffer data are invalid flag*/
        uchar   updflg;         /* work update flag.                    */

        short   indlen;         /* length of index area of user dic.    */
        short   datlen;         /* length of data  area of user dic.    */
        short   dllen;          /* length of data  DL   of user dic.    */
        short   indpos;         /* position of index area.              */
        short   datpos;         /* position of data  area.              */
        short   dlpos;          /* position of DL    area.              */
        short   promode;        /* process mode.                        */
        short   lpcnt;          /* loop counter.                        */

        short   rrn;            /* record number ob user dictionary.    */
        short   kanalen;        /* length of kanadata. (7bit code)      */
        short   yomilen;        /* length of yomidata. (PC   code)      */
        short   knjlen;         /* length of Kanji data.                */
        short   hklen;          /* length of Kanji conversion.          */
        short   posc;           /* position counter.                    */
        short   actfld;         /* active field number.                 */
        short   erfld;          /* error  field number.                 */
        short   dmode;          /* display mode.                        */
        short   smode;          /* search mode.                         */
        short   uplen;          /* active field character length.       */
        short   msgid;          /* message ID.                          */
        short   mflag;          /* PC code character type flag.         */
        short   hajime;         /* start Y axis of input field.         */
        short   gyosu;          /* maximum input line number.           */
        short   maxfld;         /* maximum field number.                */

        long    srcflg;         /* search flag.                         */

	int     triger;         /* triger key.  */
	short   col;            /* input field x axis   */
	short   line;           /* input field y axis   */
	short   dtlen;          /* length of input field        */
	short   ipmode;         /* input mode.  */
	char    *data;          /* pointer to input data        */

        int     rc;             /* return code.                         */
        int     wrc;            /* return code.                         */
	int     i,j,k;          /* work variable.                       */

	char	euc_data[U_GOKLEN * 2];
	size_t 	euclen, pclen;
	char	dummy[1000];

	/* serch data x axis    */
	int     sed_x = 30;

	/* head label data X axis.      */
	static short     hl_x[2] = { 13 , 0 };

	/* head label data Y axis.      */
	static short     hl_y[2] = { 0 , 1 };

	/* head label data length.      */
	static short     hl_l[2] = {  28 ,  54 };

	/* head label data.     */
	char    hl_d[31];

	/* message datd X axis. */
	short     msg_x;

	/* message datd Y axis. */
	short     msg_y[2];

	/* function number data X axis. */
	static short     pf_x[6] = {  0 ,  9 , 18 , 27 , 36 };

	/* function number length.      */
/*	static short     pf_l[6] = {  7 ,  7 ,  7 ,  7 , 13 , 12 };
*/	static short     pf_l[6] = {  8 ,  8 ,  8 ,  8 , 14 , 13 };

	/* function number data.        */
	static char    *pf_d[6];

	/* bracket data */
	static char    *kakko[2];

/*        strcpy(hl_d, CU_MNCTIT);
*/        memcpy(hl_d, CU_MNCTIT, 29);
        pf_d[0] = CU_MNKEY2;
        pf_d[1] = CU_MNKEY3;
        pf_d[2] = CU_MNKEY7;
        pf_d[3] = CU_MNKEY8;
        pf_d[4] = CU_MNKEY9;
        pf_d[5] = CU_MNKY12;
        kakko[0]= CU_MNKAK2;
        kakko[1]= CU_MNKAK3;

        /* 0.
         *     display character dada  and  allocate display map,data.
         */

	/* erase display area.  */
#if defined(EXTCUR)
	clear();
	refresh();
#else
	putp(clear_screen);
#endif
	/* set conversion mode  */
	kuconvm( (short)(U_FON) );

	/* get start position of input field.   */
	hajime = 3;

	/* get line number of input field.      */
	gyosu = udcbptr->ymax - 7;

	/* get maximum field number.    */
	maxfld = gyosu - 1;

        /* display head label.                                          */
	col  = hl_x[0] + udcbptr->xoff;
	line = hl_y[0];
	wrc = kudisply(udcbptr,line,col,hl_d,hl_l[0]);

	col  = hl_x[1] + udcbptr->xoff;
	line = hl_y[1];
	wrc = kudisply(udcbptr,line,col,(char *)(udcbptr->udfname),hl_l[1]);

	/* display function date        */
	line = udcbptr->ymax - 2;
	for(i=0;i<5;i++)  {
	    col = pf_x[i] + udcbptr->xoff;
	    wrc = kudisply(udcbptr,line,col,pf_d[i],pf_l[i]);
        };

        /* display function text.                                       */
	line = udcbptr->ymax - 1;
	col = pf_x[0] + udcbptr->xoff;
	wrc = kudisply(udcbptr,line,col,pf_d[5],pf_l[5]);

        /* allocate display map.                                        */
        dmappt = (UDMS *)malloc( sizeof(UDMS) );
        if( dmappt == NULL )  return( IUFAIL );
	dmappt->fld = (UDMFLD *)malloc( sizeof(UDMFLD) * gyosu );
        if( dmappt->fld == NULL )  return( IUFAIL );

        /* allocate display data.                                       */
        df = (DIFLD *)malloc( sizeof(DIFLD) * gyosu );
        if( df == NULL )  return( IUFAIL );

        /*  1.
         *      make user dictionary buffer.
         */
	/* init pointer to user dictionary buffer.                      */
        topptr  = NULL;
        lastptr = NULL;
        curptr  = NULL;

        /* get pointer to index area.                                   */
        kudcread(udcbptr , (short)3 , rrn);
        ipt = udcbptr->rdptr;

        /* get length of index area.                                    */
        indlen = GETSHORT( ipt );

        /* index position init.                                         */
        indpos = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN;

        /* loop of index. (endloop mark is '$$$$')                      */
        while(TRUE)  {

            /* check !  end of index area.                              */
            if( indpos >= indlen ) break;

            /* set index position to RRN.                               */
            indpos   = indpos + *(ipt + indpos);

            /* get RRN.                                                 */
            rrn = *(ipt + indpos);

            /* set index position to next KNL.                          */
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
         *      init update process data.
         */
        /* set pointer to head of dictionary buffer.                    */
        curptr = topptr;

        /* init process flag.                                           */
        promode = U_P_UPM;

        /* init active field number.                                    */
	actfld = 0;

        /* init display mode.                                           */
        dmode = U_DISP;

        /* initialize message erase X Y axis.                           */
	msg_x = udcbptr->xoff;
	msg_y[0] = udcbptr->ymax - 4;
	msg_y[1] = udcbptr->ymax - 3;

        /* initialize message area erase flag.                          */
        merflg = C_SWOFF;

        /* initialize all user dic' buffer data are invalid flag.       */
        empflg = C_SWOFF;

        /* 3.
         *      update process.
         */
        /* process loop. (endloop mark is '%%%%')                       */
        while(TRUE)  {

          if( promode == U_P_UPM )  {
              /* 4.
               *      display update data.
               */
	      /* display update data.   */
              wrc = kudicupm( udcbptr , df      , dmode
                            , dmappt  , actfld  , curptr
                            , topptr  , lastptr
                            , hajime  , gyosu );
	      fflush(stdout);

              if( wrc == UDNODTE )  {
                /* case : case user all dic' buffer data are invalid.   */

		/* set next process mode.       */
		/*          to 'empty return process'   */
                promode = U_P_EMP;

                /* set all user dic' buffer data are invalid flag.      */
                empflg  = C_SWON;

                continue;
              };

	      /* set first display field.       */
	      for(i=0;i<=maxfld;i++)  {
                if(dmappt->fld[i].fstat != U_NODIS)  break;
              };
	      actfld = i;
	      curptr = dmappt->fld[actfld].dbufpt;

	      /* set next process mode. */
	      /*   to 'input field to be active and wait event' */
              promode = U_P_ACWT;
          };

          if( promode == U_P_ACWT )  {
              /* 5.
               *      input field to be active and wait event.
               */
	      /* set  col,line,length,data of input field       */

	      line = hajime + actfld;
	      if(dmappt->fld[actfld].fstat == U_YOMIF) {
		col    = U_XYOMI + udcbptr->xoff;
		dtlen  = U_YOMLEN;
		data   = df[actfld].yofld;
		ipmode = T_YOMI;
	      } else {
		col   = U_XGOKU + udcbptr->xoff;
		dtlen = U_GOKLEN;
		data  = df[actfld].gofld;
		ipmode = T_GOKU;
	      };

	      /* data input loop. (endloop mark is '&&&&')      */
	      while(TRUE)  {
		/* wait input data.     */
                  triger = kuipfld(udcbptr,line,col,data,
                                        dtlen,ipmode,&uplen,C_SWOFF,cnvflg);

                /* check ! entered key.                                 */

		if( (triger == U_ENTERKEY) || (triger == U_CRKEY  ) ||
		    (triger == U_TABKEY  ) || (triger == U_BTABKEY) ||
		    (triger == U_C_UP    ) || (triger == U_C_DOWN ) ||
		    (triger == U_PF7KEY  ) || (triger == U_PF8KEY ) ||
		    (triger == U_PF3KEY  ) || (triger == U_PF9KEY )  ) {
		  /* case : input triger key of cursor move.    */
		  /* Enter or C/R or Tab or Back Tab key or     */
		  /* PF7 key or PF8 key or PF3  key entered     */
		  /* set next process mode.     */
		  /*          to  'data check process'  */
                    promode = U_P_IPCH;
                    break;
                };

		if(triger == U_PF2KEY)  {
		  /* F2 key entered.    */
		  /* set next process mode.     */
		  /*          to 'delete process'       */
		  promode = U_P_DEL;
		  break;
		};

		if(triger == U_PF12KEY)  {
		  /* F12 key entered.    */
		  /* set next process mode.     */
		  /*          to 'cancel return'        */
		  promode = U_P_CAN;
		  break;
		};

                /* display discard key enterd message.                  */
                msgid = U_GMSGN;
		(void)kumsg(udcbptr,msg_y[0],msg_x,msgid);

                /* set message erase flag.                              */
                merflg = C_SWON;
	      };        /* endloop of '$$$$'    */

              /* message erase process.                                 */
              if( merflg == C_SWON )  {
		msgid = U_ENDID;
		(void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
		(void)kumsg(udcbptr,msg_y[1],msg_x,msgid);
		/* message erase flag to be off.                      */
		merflg = C_SWOFF;
	      };
          };

          if( promode == U_P_SEAR )  {
              /* 8.
               *      search process.
               */
              /* display Yomigana input message.                        */
              msgid = U_VMSGN;
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

              /* define Yomigana input area.                            */
	      col   = sed_x + udcbptr->xoff;
	      line  = msg_y[1];
	      dtlen = U_YOMLEN;
	      ipmode = T_YOMI;
	      yomidata[0] = NULL;
	      /* write right bracket      */
	      CURSOR_MOVE(line,col-2);
	      fprintf(stdout,"%s",kakko[0]);
	      /* write left bracket       */
	      CURSOR_MOVE(line,col+U_YOMLEN);
	      fprintf(stdout,"%s",kakko[1]);

              while(TRUE) {
		/* get yomigana data.   */
                  triger = kuipfld(udcbptr,line,col,(char *)yomidata,
                                        dtlen,ipmode,&uplen,C_SWOFF,cnvflg);

		/* check inputed key ?    */
		if( (triger == U_ACTIONKEY) ||
		    (triger == U_ENTERKEY ) ||
		    (triger == U_CRKEY    ) ||
		    (triger == U_PF12KEY  ) ||
		    (triger == U_RESETKEY )   )  {
		  break;
		};

		/* display discard data enterd message. */
                msgid = U_GMSGN;
		(void)kumsg(udcbptr,msg_y[0],msg_x,msgid);

		/* set message erase flag.      */
                merflg = C_SWON;
              };

	      /* check enterd Yomi data.        */
	      if( (uplen  == 0          ) ||
		  (triger == U_PF12KEY  ) ||
		  (triger == U_RESETKEY )   )  {
		/* case : enterd data is nothing.       */
		/* message erase process. */
		msgid = U_ENDID;
		(void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
		(void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

		/* set message area erase flag(OFF).    */
                merflg = C_SWOFF;

		/* set next process mode.       */
                /*       to 'input field to be active and wait event'   */
                promode = U_P_ACWT;
                continue;
              };

              /* display search message.                                */
              msgid = U_AEMSGN;
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

	      /* search dictionary buffer.      */
              smode = U_SD_2;
              kudicscb( smode   , curptr , yomidata , uplen
                      , knjdata , knjlen , &scp );

	      /* message erase process. */
	      msgid = U_ENDID;
	      (void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

	      /* set next data display mode.    */
              dmode = U_DISP;

              /* set pointer to current pointer and active field.       */
              curptr = scp;
	      actfld = 0;

	      /* set next process mode. */
	      /*          to 'display update data'      */
              promode = U_P_UPM;
          };

	  if( promode == U_P_IPCH )  {
              /* 11.
               *       data check process.
               */
	      /* set pointer to UDCS of active field.   */
	      scp = dmappt->fld[actfld].dbufpt;

	      /* check !  active field is Yomi or Goku ?        */
	      if( dmappt->fld[actfld].fstat == U_YOMIF )  {

		/* case : active field is Yomi. */
		/* check !  blank data. */
		if(uplen == 0)  {
                  /* blank data input.                                  */
                  /* display error message.                             */
                  msgid = U_LMSGN;
		  (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

                  /* set message erase flag.                            */
                  merflg = C_SWON;

                  /* set next process mode.                             */
                  /*          to 'input & waite'                        */
                  promode = U_P_ACWT;
                  continue;
                };

                /* check !  discard data.                               */
		kudicymc( df[actfld].yofld , uplen , yomidata , &yomilen
                        , &mflag );

                /* set error field.                                     */
                erfld = actfld;

                /* set pointer to UDCS of error field.                  */
                scp2 = scp;

                if( mflag == U_INVALD || *yomidata == U_CHOUON
                                      || *yomidata == U_OMIT_C )  {
                  /* invalid yomi data error.                           */
                  /* set message ID.                                    */
                  msgid = U_PMSGN;

                  /* set next process mode.                             */
                  /*          to 'display message & error field'        */
                  promode = U_P_MSG;
                  continue;
                };

                if( mflag == U_HEMIX )  {
                  /* Kana,Alphabet,Numeric mixed.                       */
                  /* set message ID.                                    */
                  msgid = U_ACMSGN;

                  /* set next process mode.                             */
                  /*          to 'display message & error field'        */
                  promode = U_P_MSG;
                  continue;
                };

                if(    ( (mflag == U_CAPON) || (mflag == U_CAPOFF) )
                    && ( uplen >= U_B_YOMI )                         )  {
                  /* all Alphabet data and length over ten.             */
                  /* set message ID.                                    */
                  msgid = U_AJMSGN;

                  /* set next process mode.                             */
                  /*          to 'display message & error field'        */
                  promode = U_P_MSG;
                  continue;
                };

                if( mflag == U_CAPOFF  ||  mflag == U_KATA )  {
                  /* case : chege CAPSOFF to CAPSON , Katakana to Hiragana,
                   *        when enterd data is CAPSOFF or Katakana.
                   *        and redrow.
                   */
		  kudicycr(yomidata,yomilen,df[actfld].yofld,&uplen);
                  wrc = kudicupm( udcbptr , df      , (short)U_REDRW
                                , dmappt  , actfld  , curptr
                                , topptr  , lastptr
                                , hajime  , gyosu );
                };

                if( uplen == scp->yomilen )  {
		  k = memcmp(scp->yomi,df[actfld].yofld,(int)uplen);
                  if( k == 0 )  {
                    /* case : no update.                                */
                    /* set next process mode.                           */
                    /*          to 'cursor process'                     */
                    promode = U_P_CUMV;
                    continue;
                  };
                };

                /* display updateing message.                           */
                msgid = U_AFMSGN;
		(void)kumsg(udcbptr,msg_y[1],msg_x,msgid);
                /* set message erase flag.                              */
                merflg = C_SWON;

                /* set input data for kudicupc.                         */
		for(j=0;j<uplen;j++) {
		  ipdata[j] = (uchar)(df[actfld].yofld[j]);
		};

                /* search same Yomi data in dictionary buffer.          */
                smode = U_SD_0;
		kudicscb( smode   , scp    , df[actfld].yofld , uplen
                        , knjdata , knjlen , &scp2 );
                if( scp2 != NULL )  {
                  /* case : same Yomi data in dictionary buffer.        */
                  /* set error field.                                   */
                  erfld = actfld;

                  /* set error message ID.                              */
                  msgid = U_ABMSGN;

                  /* set pointer to UDCS of error field.                */
                  scp2 = scp;

                  /* set next process mode.                             */
                  /*          to 'display message & error field'        */
                  promode = U_P_MSG;
                  continue;
                };

                /* get start point of Yomi data.                        */
                erfld = actfld;
                scp3 = scp2  = scp;
                while(TRUE)  {
                  if( scp3->pr_pos == NULL )  break;
                  scp3 = scp3->pr_pos;
                  if( (scp->yomilen != scp3->yomilen )  ||
                      (memcmp(scp->yomi,scp3->yomi,(int)(scp->yomilen))
                                               != 0)  )  break;
                  if(    (scp3->status == U_S_INIT)
                      || (scp3->status == U_S_YOMA)
                      || (scp3->status == U_S_KNJU)    )  {
		    erfld --;
                    scp2 = scp3;
                  };
                };

                /* initialize loop counter.                             */
                lpcnt = 1;
                scp3 = scp2;
                while(TRUE)  {
                  if(scp3->nx_pos == NULL)  break;
                  scp3 = scp3->nx_pos;
                  if( (scp->yomilen != scp3->yomilen )  ||
                      (memcmp(scp->yomi,scp3->yomi,(int)(scp->yomilen))
                                               != 0)  )  break;
                  if(    (scp3->status == U_S_INIT)
                      || (scp3->status == U_S_YOMA)
                      || (scp3->status == U_S_KNJU)    )  {
                    lpcnt = lpcnt + 1;
                  };
                };

                /* set next process mode.                               */
                /*          to 'Yomi data error check process'          */
                promode = U_P_YOCH;
	      } else  {
		/* case : active field is Goku. */

		/* check !  enterd data.        */
                if( uplen == scp->kanlen )  {
		  k = memcmp( scp->kan , df[actfld].gofld , (int)uplen );
                  if( k == 0 )  {
                    /* case no update.                                  */
                    /* set next process mode.                           */
                    /*          to 'cursor process'                     */
                    promode = U_P_CUMV;
                    continue;
                  };
                };

                if( uplen == 0 )  {
		  /* case : enterd data is nothing.     */
		  /* display error message.     */
                  msgid = U_JMSGN;
		  (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

		  /* set message erase flag.    */
                  merflg = C_SWON;

		  /* set next process mode.     */
		  /*          to 'input and weit'       */
                  promode = U_P_ACWT;
                  continue;
                };


		/* goku data update process.    */
                lpcnt = 1;
                scp2 = scp;

		/* set next process mode.       */
		/*          to 'Goki data error check'  */
                promode = U_P_GOCH;
              };
          };

          if( promode == U_P_YOCH )  {
              /* 6.
               *      Yomi data error check.
               */
	      /* keep pointer   */
              scp3 = scp2;

              /* update error check.                                    */
              /* check Yomi == Goku.                                    */
              for(i=0;i<lpcnt;scp2=scp2->nx_pos)  {
                /* Yomi add flag to be OFF.                             */
                scp2->ycaddflg = C_SWOFF;

                if(    (scp2->status != U_S_INIT)
                    && (scp2->status != U_S_YOMA)
                    && (scp2->status != U_S_KNJU)    )  continue;

                if(scp2->kanlen == uplen)  {
		  k = memcmp(scp2->kan,df[actfld].yofld,(int)uplen);
                  if( k == 0)  {
                    /* case : Yomi == Goku.                           */
                    /* set error message ID.                          */
                    msgid = U_YMSGN;

                    /* set next process mode.                         */
                    /*      to 'display message and error field'      */
                    promode = U_P_MSG;
                    break;
                  };
                };
                i++;
              };

              if( promode == U_P_MSG ) continue;

              /* check system dictionary.                               */
              scp2 = scp3;
              for(i=0;i<lpcnt;scp2=scp2->nx_pos)  {

                if(    (scp2->status != U_S_INIT)
                    && (scp2->status != U_S_YOMA)
                    && (scp2->status != U_S_KNJU)    )  continue;

                /* Hiragana , Katakana same Yomi code process.          */
                wrc = kuhkfc( yomidata , yomilen , scp2->kan
                            , (short)(scp2->kanlen) , hkdata , &hklen );

                /* search same data in system dictionary.               */
                if( wrc == U_DIVHK ) {
                  /* case : other.                                      */
#if defined(_OLD_AIX320)
                  kudiccs( sdcbptr , yomilen , yomidata
                         , (short)(scp2->kanlen) , scp2->kan , &srcflg );
#else
                  kudiccs3( sdcbptr , yomilen , yomidata
                         , (short)(scp2->kanlen) , scp2->kan , &srcflg );
#endif /* defined(_OLD_AIX320) */
                }
                else {
                  /* case : same Kana data.                             */
#if defined(_OLD_AIX320)
                  kudiccs( sdcbptr , yomilen , yomidata
                         , hklen   , hkdata  , &srcflg );
#else
                  kudiccs3( sdcbptr , yomilen , yomidata
                         , hklen   , hkdata  , &srcflg );
#endif /* defined(_OLD_AIX320) */
                };

                if( srcflg == 1 )  {
                  /* case : same data is exist in system dictionary.    */
                  /* display error field.                               */
		  if( (erfld < 0) || (maxfld < erfld) )  {
                    /* case : error data not display.                   */
                    /* error data display process.                      */
                    dmode = U_DISP;
                    wrc = kudicupm( udcbptr , df     , dmode  , dmappt
                                  , erfld   , scp2   , topptr , lastptr
                                  , hajime  , gyosu  );

                    for(j=0;j<uplen;j++)  df[0].yofld[j] = (char)ipdata[j];
                    df[0].yofld[uplen] = NULL;
		    erfld = 0;
                    dmode = U_REDRW;
                    wrc = kudicupm( udcbptr , df     , dmode  , dmappt
                                  , erfld   , scp2   , topptr , lastptr
                                  , hajime  , gyosu  );

		    erfld = 1;
                    curptr = scp2;
		  } else {
		    if(erfld == actfld) {
		      erfld++;
		    };
		  };

                  /* error data revers display process.                 */
		  dmode = U_REVER;
                  wrc = kudicupm( udcbptr , df     , dmode  , dmappt
                                , erfld   , scp2   , topptr , lastptr
                                , hajime  , gyosu   );

                  /* display error message.                             */
                  msgid = U_OMSGN;
		  (void)kumsg(udcbptr,msg_y[0],msg_x,msgid);

                  /* display error message.                             */
                  msgid = U_ADMSGN;
		  (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

                  /* set message erase flag.                            */
                  merflg = C_SWON;

                  /* wait key input.                                    */
                  while(TRUE)  {
		    wrc = kugetc();
		    j = kugetcmp( wrc );
		    if(j == 2) {
		      j = kugetc();
		      wrc = (wrc<<8) | j;
		    };
		    if( (wrc == U_RESETKEY) || (wrc == U_PF12KEY) ) {
		      /* Reset key entered process.        */
		      /* Yomi add flag to be  OFF.      */
                      for(j=0;j<lpcnt;j++,scp3=scp3->nx_pos)
                        scp3->ycaddflg = C_SWOFF;

		      /* set next process mode  */
		      /*          to 'display update data'      */
                      promode = U_P_UPM;
                      break;
                    };
		    if( (wrc == U_ACTIONKEY) ||
			(wrc == U_CRKEY    ) ||
			(wrc == U_ENTERKEY )     ) {
		      /* Enter key entered process.       */
		      /* delete data in user dictionary.        */
		      /* Yomi add flag to be ON.        */
		      dmappt->fld[erfld].dbufpt->ycaddflg
                                                      = C_SWON;

                      break;
                    };
                  };

                  /* error data reset display process.                  */
                  dmode = U_RESET;
                  wrc = kudicupm( udcbptr , df     , dmode  , dmappt
                                , erfld   , scp2   , topptr , lastptr
                                , hajime  , gyosu  );

                };
                if(promode == U_P_UPM)  {
                     /* case : stop Yomi update.                        */
                     /* set display mode.                               */
                     dmode = U_DISP;
                     break;
                };

                /* set field to next Goku field.                        */
		if(erfld == actfld) {
		  erfld += 2;
		} else {
		  erfld++;
		};
                i++;
              };

	      /* message erase process. */
	      msgid = U_ENDID;
	      (void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

              /* set next process mode.                                 */
              /*          to 'update process'                           */
              if( (promode == U_P_UPM) || (promode == U_P_MSG) ) continue;
              promode = U_P_UPDT;

          };

          if( promode == U_P_GOCH )  {
              /* 14.
               *      Goku data error check.
               */

              /* update error check.                                    */
              erfld = actfld;

              if(scp2->yomilen == uplen)  {
		k = memcmp(scp2->yomi,df[actfld].gofld,(int)uplen );
                if( k == 0)  {
                  /* case : Goku == Yomi.                               */
                  /* set message ID.                                    */
                  msgid = U_YMSGN;

                  /* set next process mode.                             */
                  /*         to display message and error field.        */
                  promode = U_P_MSG;
                  continue;
                }
              };

              /* check user dictionary.                                 */
              smode = U_SD_1;
              kudicscb( smode   , scp2
                      , scp2->yomi , (short)(scp2->yomilen)
		      , df[actfld].gofld , uplen , &scp3 );

              if( scp3 != NULL )  {
                /* case : same data exist in dictionary buffer.         */
                /* set error message ID.                                */
                msgid = U_NMSGN;

                /* set nect process mode.                               */
                /*         to display message and error field.          */
                promode = U_P_MSG;
                continue;
              }

              /* convert Yomi data of updated Goku data                 */
              /*         to 7bit code.                                  */
              kudicymc( scp2->yomi , (short)(scp2->yomilen)
                      , yomidata   , &yomilen , &mflag );

              /* check system dictionary.                               */
              /* Hiragana,Katakana same Yomi data process.              */
	      wrc = kuhkfc( yomidata , yomilen , df[actfld].gofld
                          , uplen    , hkdata  , &hklen );

              /* search system dictionary.                              */
              if( wrc == U_DIVHK ) {
                  /* case : other.                                      */
#if defined(_OLD_AIX320)
                  kudiccs( sdcbptr , yomilen       , yomidata
			 , uplen   , df[actfld].gofld  , &srcflg );
#else
                  kudiccs3( sdcbptr , yomilen       , yomidata
			 , uplen   , df[actfld].gofld  , &srcflg );
#endif /* defined(_OLD_AIX320) */
                }
                else {
                  /* case : Hiragana,Yomigana same Yomi data.           */
#if defined(_OLD_AIX320)
                  kudiccs( sdcbptr , yomilen , yomidata
                         , hklen   , hkdata  , &srcflg );
#else
                  kudiccs3( sdcbptr , yomilen , yomidata
                         , hklen   , hkdata  , &srcflg );
#endif /* defined(_OLD_AIX320) */
                };

              if( srcflg == 1 )  {
		/* case : same data exist in system dictionary. */
		/* error data revers display process.   */
		dmode = U_REVER;
                wrc = kudicupm( udcbptr , df     , dmode  , dmappt
                              , erfld   , scp2   , topptr , lastptr
                              , hajime  , gyosu );

		/* display error message.       */
                msgid = U_OMSGN;
		(void)kumsg(udcbptr,msg_y[0],msg_x,msgid);

		/* display error message.       */
                msgid = U_ADMSGN;
		(void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

		/* wait reset input.    */
                while(TRUE)  {
		  wrc = kugetc();
		  j = kugetcmp( wrc );
		  if(j == 2) {
		    j = kugetc();
		    wrc = (wrc<<8) | j;
		  };
		  if( (wrc == U_RESETKEY) || (wrc == U_PF12KEY) ) {
		    /* Reset key entered process.  */
		    /* set next process mode    */
                    /*     to 'input field to be active and wait event' */
                    promode = U_P_ACWT;
                    break;
                  };
		  if( (wrc == U_ACTIONKEY) ||
		      (wrc == U_ENTERKEY ) ||
		      (wrc == U_CRKEY    )    )  {
		    /* Enter key entered process. */
		    /* delete data in user dictionary.  */
                    kudicdlb(dmappt , erfld );

		    /* set next process mode    */
		    /*          to 'display update data'        */
                    promode = U_P_UPM;
                    break;
                  };
                };

		/* error data reset display process.    */
                dmode = U_RESET;
                wrc = kudicupm( udcbptr , df     , dmode  , dmappt
                              , erfld   , scp2   , topptr , lastptr
                              , hajime  , gyosu );

		/* message erase process.       */
		msgid = U_ENDID;
		(void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
		(void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

		/* set display mode.    */
                if( promode == U_P_UPM )  dmode = U_DISP;
                continue;
              }
              else  {
                /* set input data for kudicupc.                         */
		for(i=0;i<uplen;i++) {
		  ipdata[i] = (uchar)(df[actfld].gofld[i]);
		};

                /* set next process mode.                               */
                /*          to 'update process'                         */
                promode = U_P_UPDT;
              };
          };

          if( promode == U_P_UPDT )  {
              /* 12.
               *      update process.
               */

              scp2 = NULL;
              rc = kudicupc( dmappt , actfld , ipdata , uplen
                           , &topptr , &lastptr , &scp2 );

              if( rc == UDOVFDLE )   {
                /* case : DL overflow.                                  */
                /* set pointer to UDCS of error field.                  */
		scp2    = dmappt->fld[actfld].dbufpt;

                /* set error field ID.                                  */
                erfld   = actfld;

                /* set error message ID.                                */
                msgid   = U_XMSGN;

		/* set next process mode        */
                /*          to 'display error message and field'        */
                promode = U_P_MSG;
                continue;
              };

	      /* message erase process. */
	      msgid = U_ENDID;
	      (void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

              if( rc != IUSUCC ) {
		/* case : other error occured.  */
		/* set work update flag to be OFF.      */
                updflg = C_SWOFF;
                break;
              };

              /* set update flag.                                       */
              updflg = C_SWON;

              if( scp2 != NULL )  {
                /* case : Yomi field update.                            */
                /* set current pointer to UDCS.                         */
                curptr  = scp2;

                /* set display mode.                                    */
                dmode   = U_DISP;

		/* set next process mode        */
		/*          to 'display update data'    */
                promode = U_P_UPM;

		if(triger == U_PF9KEY) {
		  /* F9 key entered.                                  */
		  /* display update data.                             */
		  wrc = kudicupm( udcbptr , df      , dmode
				, dmappt  , actfld  , curptr
				, topptr  , lastptr
				, hajime  , gyosu );

		  /* set active field.                               */
		  for(i=0;i<=maxfld;i++)  {
			  if(dmappt->fld[i].fstat != U_NODIS)  break;
		  };
		  actfld = i;
		  curptr = dmappt->fld[i].dbufpt;

		  /* set next process mode                            */
		  /*          to 'search Yomigana'                    */
		  promode = U_P_SEAR;
		};

		if(triger == U_PF3KEY)  {
		  /* F3 key entered.                                 */
		  /* set next process mode                            */
		  /*          to 'end return'                         */
		  promode = U_P_END;
                };
                continue;
              };

              /* set next process mode                                  */
              /*          to 'cursor process'                           */
              promode = U_P_CUMV;
          };

          if( promode == U_P_CUMV )  {
	    /* 13.
	     *      cursor process.
	     */
	    /* check ! event data.    */

	    if( (triger == U_ENTERKEY) || (triger == U_CRKEY) ||
		(triger == U_TABKEY  ) || (triger == U_C_DOWN)  ) {
	      /* Enter or C/R or TAB key enterd.     */
	      /* check ! next field is used.  */
	      while(TRUE)  {
		actfld = actfld + 1;
		if( actfld > maxfld )  {
		  /* set first field  */
		  /*   because no use filed behind current field      */
		  actfld = 0;
		};
		if( dmappt->fld[actfld].fstat != U_NODIS )  {
		  /* next field is used.      */
		  break;
		};
	      };
	      /* set next process mode.       */
	      /*          to 'input field to be active and wait event'*/
	      promode = U_P_ACWT;

	    };

	    if( (triger == U_BTABKEY) || (triger == U_C_UP) ) {
	      /* Back Tab key entered.        */
	      /* check ! previous field is used.      */
	      while(TRUE)  {
		actfld = actfld - 1;

		if( actfld < 0 )  {
		  /* set last  field  */
		  /*   because no use filed forward current field     */
		  actfld = maxfld;
		};
		if( dmappt->fld[actfld].fstat != U_NODIS )  {
		  /* forward field is used.   */
		  break;
		};
	      };
	      /* set next process mode.       */
	      /*          to 'input field to be active and wait event'*/
	      promode = U_P_ACWT;

	    };

	    if(triger == U_PF7KEY) {
	      /* F7key entered.       */
	      /* set next process mode        */
	      /*          to 'display update data'    */
	      promode = U_P_UPM;

	      /* set display mode. (next page)        */
	      dmode = U_BEFORP;

	      continue;
	    };

	    if(triger == U_PF8KEY)  {
	      /* F8 key entered.      */
	      /* set next process mode        */
	      /*          to 'display update data'    */
	      promode = U_P_UPM;

	      /* set display mode. (previous page)    */
	      dmode = U_NEXTP;

	      continue;
	    };

	    if(triger == U_PF9KEY)  {
	      /* F9 key entered.      */
	      /* set next process mode        */
	      /*          to 'search Yomigana'        */
	      promode = U_P_SEAR;

	      continue;
	    };

	    if(triger == U_PF3KEY)  {
	      /* F3 key entered.     */
	      /* set next process mode        */
	      /*          to 'end return'     */
	      promode = U_P_END;

	      continue;
	    };
          };

	  if(promode == U_P_DEL)  {
              /* 9.
               *      delete process.
               */
	      /* check !  delete Goku data and  */
	      /*          Yomi has Goku that's only one.        */
	      if(   dmappt->fld[actfld].fstat == U_GOKUF
		&&  dmappt->fld[actfld-1].fstat == U_YOMIF ) {

		if(actfld == 1)  {
		  /* case : display at top of display.  */
                  if(    dmappt->prestat == C_SWOFF
		      && dmappt->fld[actfld+1].fstat != U_GOKUF ) {
                    /* case : yomi has Goku that's only one.            */
                    /* set field to it's Yomi field.                    */
                    actfld--;
                  };
                } else if(actfld  == maxfld)  {
                  /* case : display at bottom of display.               */
                  if(dmappt->poststat == C_SWOFF)  {
                    /* case : yomi has Goku that's only one.            */
                    /* set field to it's Yomi field.                    */
                    actfld--;
                  };
		} else if(dmappt->fld[actfld+1].fstat != U_GOKUF) {
                    /* case : yomi has Goku that's only one.            */
                    /* set field to it's Yomi field.                    */
                    actfld--;
                };

              };

              /* reverse display delete data.                            */
              erfld = actfld;
	      dmode = U_REVER;
              wrc = kudicupm( udcbptr , df      , dmode
                            , dmappt  , actfld  , curptr
                            , topptr  , lastptr
                            , hajime  , gyosu );

              /* revers display delete goku data.                       */
	      if( dmappt->fld[actfld].fstat == U_YOMIF )  {
                do {
                  erfld++;
		  dmode = U_REVER;
                  wrc = kudicupm( udcbptr , df      , dmode
                                , dmappt  , erfld   , curptr
                                , topptr  , lastptr
                                , hajime  , gyosu );
                } while(  erfld < maxfld
		      &&  dmappt->fld[erfld+1].fstat == U_GOKUF );
	      };

              /* display error message.                                 */
              msgid = U_TMSGN;
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

              /* set message erase flag.                                */
              merflg = C_SWON;

              /* set next process mode                                  */
              /*          to 'cancel , delete fix process'              */
              promode = U_P_FIX;
              upmode = C_SWON;
          };

          if( promode == U_P_CAN )  {
              /* 10.
               *      cancel return process.
               */
              /* display error message.                                 */
              msgid = U_UMSGN;
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

              /* set message erase flag.                                */
              merflg = C_SWON;

              /* set next process mode                                  */
              /*          to 'cancel , delete fix process'              */
              promode = U_P_FIX;
              upmode  = C_SWOFF;
          };

          if( promode == U_P_FIX )  {
              /* 15.
               *       cancel , delete fix process.
               */
	      /* wait key input.        */
              while(TRUE)  {
		triger = kugetc();

                /* check ! entered key.                                 */
		if( (triger == U_ACTIONKEY) ||
		    (triger == U_CRKEY    ) ||
		    (triger == U_ENTERKEY )    )  {
		  /* action key entered process.        */
                  if( upmode == C_SWON )  {
		    /* delete data from dictionary buffer.      */
		    kudicdlb(dmappt,actfld);

		    /* update flag ON.  */
                    updflg = C_SWON;

		    /* set display mode.        */
                    dmode = U_DISP;

		    /* set next process mode    */
		    /*          to 'display update data'        */
                    promode = U_P_UPM;
		  } else {
		    /* cancel process.  */
                    rc = UDSUCC;

		    /*  set update uty return mode flag.        */
                    if (updflg == C_SWON)  {
                        udcbptr->uurmf = U_AMMSGN;
		    } else {
                        udcbptr->uurmf = U_ENDID;
                    }

		    /* update flag OFF. */
                    updflg = C_SWOFF;

                    promode = 999;
                  }
                  break;
                };

		if( (triger == U_RESETKEY) || (triger == U_PF12KEY) ) {
		  /* reset key entered process. */
		  /* set next process mode      */
                  /*        to 'input field to be active and wait event'*/
                  promode = U_P_ACWT;
                  break;
                };

		/* discard process.     */
		/* display error message.       */
                msgid = U_GMSGN;
		(void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
		/* set message erase flag.      */
                merflg = C_SWON;

              };

              if( promode == 999 )  break;  /* case cancel return.      */
              if( empflg == C_SWON  &&  promode != U_P_END )
                {
                   promode = U_P_EMP;
                };

	      /* message erase process. */
	      msgid = U_ENDID;
	      (void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

              if( upmode == C_SWON  &&  promode != U_P_UPM )  {
		/* reset process.       */
                erfld = actfld;
                dmode = U_RESET;
                wrc = kudicupm( udcbptr , df      , dmode
                              , dmappt  , actfld  , curptr
                              , topptr  , lastptr
                              , hajime  , gyosu );

		/* check !  active field is Yomi or Goki ?      */
		if( dmappt->fld[actfld].fstat == U_YOMIF )  {
                  do {
                    erfld++;
                    dmode = U_RESET;
                    wrc = kudicupm( udcbptr , df      , dmode
                                  , dmappt  , erfld   , curptr
                                  , topptr  , lastptr
                                  , hajime  , gyosu );
                  } while(  erfld < maxfld
			&&  dmappt->fld[erfld+1].fstat == U_GOKUF);
                };
              };

          };

          if( promode == U_P_MSG )  {
              /* 16.
               *       display message and error field process.
               */
	      /* display error field.   */
	      if( (erfld < 0) || (maxfld < erfld) )  {
		/* case : error field not display.      */
		/* error data display process.  */
                dmode = U_DISP;
                wrc = kudicupm( udcbptr , df     , dmode  , dmappt
                              , erfld   , scp2   , topptr , lastptr
                              , hajime  , gyosu );
		erfld = 0;
                curptr = scp2;
              };

	      /* error data revers display process.     */
	      dmode = U_REVER;
              wrc = kudicupm( udcbptr , df     , dmode  , dmappt
                            , erfld   , scp2   , topptr , lastptr
                            , hajime  , gyosu );

              /* display error message.                                 */
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

              /* set message erase flag.                              */
              merflg = C_SWON;

              /* wait reset input.                                      */
              while(TRUE)  {
		wrc = kugetc();
		if( (wrc == U_RESETKEY) || (wrc == U_PF12KEY) ) {
		  /* reset key entered process. */
                  break;
                };
              };

	      /* error data reset display process.      */
              dmode = U_RESET;
              wrc = kudicupm( udcbptr , df     , dmode  , dmappt
                            , erfld   , scp2   , topptr , lastptr
                            , hajime  , gyosu );

	      /* message erase process. */
	      msgid = U_ENDID;
	      (void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

	      /* set next process mode  */
	      /*          to 'display update data'      */
              promode = U_P_ACWT;
          };

          if( promode == U_P_END )  {
              /* 7.
               *       end return process.
               */
	      /* updete user dictionary.        */
              rc = kudicueh( topptr , lastptr , udcbptr , &curptr );
              if(rc == UDSUCC)  {
		/* set update uty return mode flag.     */
                udcbptr->uurmf = U_BMSGN;
                break;
              };
	      /* reflesh user dictionary.       */
              (void)memcpy(udcbptr->dcptr,udcbptr->secbuf
                                         ,(int)udcbptr->ufilsz);

	      /* set error message id.  */
              if(rc == UDDCFULE)  msgid = U_QMSGN;
              else                msgid = U_XMSGN;

	      /* display error message. */
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

	      /* set message erase flag.        */
              merflg = C_SWON;

	      /* set next process mode  */
	      /*          to 'display update data'      */
              promode = U_P_UPM;
	      /* set display mode.      */
              dmode   = U_DISP;

          };

          if( promode == U_P_EMP )  {
              /* 17.
               *       emptey return process.
               */
	      /* display empty message. */
              msgid = U_ANMSGN;
	      (void)kumsg(udcbptr,msg_y[0],msg_x,msgid);

              /* display input message.                                 */
              msgid = U_AOMSGN;
	      (void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

	      /* set message erase flag.        */
              merflg = C_SWON;

	      /* wait input data.       */
              while(TRUE)  {
		wrc = kugetc();

		/* check ! entered key. */
		if(wrc == U_PF12KEY)  {
		  /* F12 key entered.   */
		  /* set next process mode      */
		  /*          to 'cancel return process'        */
		  promode = U_P_CAN;
		  break;
		};

		if(wrc == U_PF3KEY)  {
		  /* F3 key entered.    */
		  /*          to 'end return process'   */
		  promode = U_P_END;
		  break;
		};

		/* discard process.     */
		/* display error message.       */
                msgid = U_GMSGN;
		(void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
              };
	      if(merflg == C_SWON)  {
		/* message erase process. */
		msgid = U_ENDID;
		(void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
		(void)kumsg(udcbptr,msg_y[1],msg_x,msgid);

                merflg = C_SWOFF;
              };
          };
	};      /* endloop is '%%%%'    */

        /*
         *      free display map area and dictionary area.
         */
        /* free display data.                                           */
        free( (char *)df     );

        /* free display map.                                            */
        free( (char *)(dmappt->fld) );
        free( (char *)dmappt );

        /* free dictionary buffer.                                      */
        while(TRUE)  {
          curptr = lastptr->pr_pos;
          free( (char *)lastptr );
          if( curptr == NULL ) break;
          lastptr = curptr;
        };

        /* set update flag.                                             */
	if ( updflg == C_SWON ) {
	    udcbptr->updflg = updflg;
            msgid = U_UPDATING;
	    (void)kumsg(udcbptr,msg_y[0],msg_x,msgid);
            wrc = kutmwrt( udcbptr->orgfd, udcbptr->dcptr, udcbptr->ufilsz );
            if ( wrc != IUSUCC ) {
	    	udcbptr->updflg = C_SWOFF;
               	rc = UDWRITEE;
            }
	}

        /* return.                                                      */
        return( rc );

}

