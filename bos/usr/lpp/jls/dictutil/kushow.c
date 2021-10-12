static char sccsid[] = "@(#)55	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kushow.c, cmdKJI, bos411, 9428A410j 7/23/92 01:29:32";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kushow
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
 * MODULE NAME:         kushow
 *
 * DESCRIPTIVE NAME:    user dictionary display
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
 *  MODULE SIZE:        2874  Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         kushow
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kushow ( udcbptr )
 *
 *  INPUT               *udcbptr: pointer to User Dictionary Control Block
 *
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
 *                              kudisply
 *                              kudicscb
 *                              kudicupm
 *                              kuhkfc
 *                              kudisply
 *                      Standard Liblary.
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
#include "kje.h"        /* Kanji Utility Define File.   */
#include "kut.h"        /* Kanji Utility Define File.   */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

extern	int	cnvflg;		/* Conversiopn Type Code */

/*
 *      Please Descripte This Module.
 */
int     kushow ( udcbptr , dmappt , topptr , lastptr )

UDCB    *udcbptr;       /* pointer to UDCB.     */
UDMS    *dmappt;        /* pointer to UDMS.     */
UDCS    *topptr;        /* pointer to top of UDCS.      */
UDCS    *lastptr;       /* pointer to last of UDCS.     */

{
        void    kudicscb();     /* user dictionary check.               */
	int     kumsg();        /* display message.     */
        int     kudicupm();     /* user dictionary buffer display       */
                                /*         delete status code set       */
	int     kudisply();     /*      */
	int     kugetc();       /*      */

        DIFLD   *df;            /* pointer to DIFLD.                    */
        UDCS    *curptr;        /* pointer to current of UDCS.          */
        UDCS    *scp;           /* work pointer to UDCS.                */

        uchar   merflg;         /* message area erase flag.             */
	uchar   yomidata[U_YOMFLD]; /* yomi (PC code) string.           */
	uchar   knjdata[U_GOKFLD];  /* Kanji (PC code) string.          */
        short   promode;        /* process mode.                        */

        short   knjlen;         /* length of Kanji data.                */
        short   actfld;         /* active field number.                 */
        short   dmode;          /* display mode.                        */
        short   smode;          /* search mode.                         */
        short   uplen;          /* active field character length.       */
        short   msgid;          /* message ID.                          */
        short   hajime;         /* start Y axis of input field.         */
        short   gyosu;          /* maximum input line number.           */
        short   maxfld;         /* maximum field number.                */


	int     triger;         /* triger key.  */
	short   col;            /* input field x axis   */
	short   line;           /* input field y axis   */
	short   dtlen;          /* length of input field        */
	short   ipmode;         /* input mode.  */

        int     wrc;            /* return code.                         */
	int     i;              /* work variable.                       */

	/* bracket data */
	static char kakko[2][4];

	/* serch data x axis    */
	short     sedi_x = 30;

	/* head label data X axis.      */
	static short     hl3d_x[2] = { 13 , 0 };

	/* head label data Y axis.      */
	static short     hl3d_y[2] = { 0 , 1 };

	/* head label data length.      */
	static short     hl3d_l[2] = {  28 ,  54 };

	/* head label data.     */
	static  char     hl3d_d[32];

	/* message datd X axis. */
	short     msg_x;

	/* message datd Y axis. */
	short     msg_y[2];

	/* function number data X,Y axis.       */
	/* change IBM-J		*/
	static short     pf_x[4] = {  0 ,  9 , 18 , 27 };
        /*
	static short     pf_x[4] = {  1 ,  12 , 24 , 41 };
        */

	/* function number length.      */
	/* change IBM-J		*/
	static short     pf_l[4] = {  7 ,  7 ,  7 ,  13 };

        /*
	static short     pf_l[4] = {  10 ,  10 ,  16 ,  11 };
	*/

	/* function number data.        */
	static  char    pf_d[4][20];


        strcpy(kakko[0], CU_MNKAK2);
        strcpy(kakko[1], CU_MNKAK3);

        strcpy(hl3d_d,   CU_MNDTIT);

        strcpy(pf_d[0], CU_MNKEY3);
        strcpy(pf_d[1], CU_MNKEY7);
        strcpy(pf_d[2], CU_MNKEY8);
        strcpy(pf_d[3], CU_MNKEY9);


        /* 0.
         *     display character dada  and  allocate display map,data.
         */

	/* erase display area.  */

/* B.EXTCUR */
	clear();
	refresh();
/* E.EXTCUR */

        /* display head label.                                          */
	col  = hl3d_x[0] + udcbptr->xoff;
	line = hl3d_y[0];
	wrc = kudisply(udcbptr,line,col,hl3d_d,hl3d_l[0]);

	col  = hl3d_x[1] + udcbptr->xoff;
	line = hl3d_y[1];
	wrc = kudisply(udcbptr,line,col,(char *)(udcbptr->udfname),hl3d_l[1]);

        /* display function number.                                     */
	line = udcbptr->ymax - 1;
	for(i=0;i<4;i++) {
	  col = pf_x[i] + udcbptr->xoff;
	  wrc = kudisply(udcbptr,line,col,pf_d[i],pf_l[i]);
	};

        /* 2.
         *      init update process data.
         */
	/* get start position of input field.   */
	hajime = 3;

	/* get line number of input field.      */
	/* change IBM-J		*/
	gyosu = udcbptr->ymax - 7;
        /*
	gyosu = udcbptr->ymax - 6;
        */

	/* get maximum field number.    */
	maxfld = gyosu - 1;

	/* set pointer to head of dictionary buffer.    */
        curptr = topptr;

	/* init process flag.   */
        promode = U_P_UPM;

	/* init display mode.   */
        dmode = U_DISP;

	/* init active field number.                                    */
	actfld = 0;

	/* initialize message erase X Y axis.   */
	msg_x = udcbptr->xoff;
	/* change IBM-J		*/
	msg_y[0] = udcbptr->ymax - 4;
	msg_y[1] = udcbptr->ymax - 3;
        /*
	msg_y[0] = udcbptr->ymax - 3;
	msg_y[1] = udcbptr->ymax - 2;
        */

	/* initialize message area erase flag.                          */
	merflg = C_SWOFF;
	/* set pointer to df    */
	df = (DIFLD *)malloc(sizeof(DIFLD) * gyosu);

        /* 3.
	 *      display process.
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
	       *      wait event.
               */
	      /* data input loop. (endloop mark is '&&&&')      */
	      while(TRUE)  {
		fflush(stdout);
		/* wait input data.     */
		triger = kugetc();

		if(triger == U_PF7KEY) {
		  /* F7key entered.       */
		  /* set next process mode        */
		  /*          to 'display update data'    */
		  promode = U_P_UPM;

		  /* set display mode. (next page)        */
		  dmode = U_BEFORP;

		  break;
		};

		if(triger == U_PF8KEY)  {
		  /* F8 key entered.      */
		  /* set next process mode        */
		  /*          to 'display update data'    */
		  promode = U_P_UPM;

		  /* set display mode. (previous page)    */
		  dmode = U_NEXTP;

		  break;
		};

		if(triger == U_PF9KEY)  {
		  /* F9 key entered.      */
		  /* set next process mode        */
		  /*          to 'search Yomigana'        */
		  promode = U_P_SEAR;

		  break;
		};

		if(triger == U_PF3KEY)  {
		  /* F12 key entered.     */
		  /* set next process mode        */
		  /*          to 'end return'     */
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
		(void)kumsg(udcbptr,msg_y[0],msg_x,(short)U_ENDID);
		(void)kumsg(udcbptr,msg_y[1],msg_x,(short)U_ENDID);
		/* message erase flag to be off.                      */
		merflg = C_SWOFF;
	      };
	      if(triger == U_PF3KEY) {
		break;
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
	      col   = sedi_x + udcbptr->xoff;
	      dtlen = U_YOMLEN;
	      ipmode = T_YOMI;
	      yomidata[0] = NULL;

	      /* write right bracket      */
	      CURSOR_MOVE(msg_y[1],col-2);
	      fprintf(stdout,"%s",kakko[0]);

	      /* write left bracket       */
	      CURSOR_MOVE(msg_y[1],col+U_YOMLEN);
	      fprintf(stdout,"%s",kakko[1]);

              while(TRUE) {
		/* get yomigana data.   */
		line = msg_y[1];
		triger = kuipfld(udcbptr,line,col,(char *)yomidata,
					dtlen,ipmode,&uplen,C_SWOFF,cnvflg);

		/* check inputed key is Action key ?    */
		if( (triger == U_ACTIONKEY) ||
		    (triger == U_CRKEY    ) ||
		    (triger == U_ENTERKEY ) ||
		    (triger == U_PF12KEY  ) ||
		    (triger == U_RESETKEY )     )  {
		  break;
		};

		/* display discard data enterd message. */
		(void)kumsg(udcbptr,msg_y[0],msg_x,(short)U_GMSGN);

		/* set message erase flag.      */
                merflg = C_SWON;
              };

	      /* check enterd Yomi data.        */
	      if( (uplen  == 0          ) ||
		  (triger == U_PF12KEY  ) ||
		  (triger == U_RESETKEY )     )  {
		/* case : enterd data is nothing or process reset       */
		/* message erase process. */
		(void)kumsg(udcbptr,msg_y[0],msg_x,(short)U_ENDID);
		(void)kumsg(udcbptr,msg_y[1],msg_x,(short)U_ENDID);

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
	      (void)kumsg(udcbptr,msg_y[0],msg_x,(short)U_ENDID);
	      (void)kumsg(udcbptr,msg_y[1],msg_x,(short)U_ENDID);

	      /* set next data display mode.    */
              dmode = U_DISP;

              /* set pointer to current pointer and active field.       */
              curptr = scp;
	      actfld = 0;

	      /* set next process mode. */
	      /*          to 'display update data'      */
              promode = U_P_UPM;
          };


	};      /* endloop is '%%%%'    */

	free( (char *)df     );
	/* erase display area.  */

/* B.EXTCUR */
	clear();
	refresh();
/* E.EXTCUR */

	return( wrc );

}
