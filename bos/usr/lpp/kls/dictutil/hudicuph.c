static char sccsid[] = "@(#)12  1.1  src/bos/usr/lpp/kls/dictutil/hudicuph.c, cmdkr, bos411, 9428A410j 5/25/92 14:44:54";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicuph.c
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
 *  Module:       hudicuph.c
 *
 *  Description:  Handle Update of a key and candidate.
 *
 *  Functions:    hudicuph()
 *
 *  History:      5/22/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Memory package.                              */
#include "hut.h"        /* Dictionary Utility Define File.   */

int     hudicuph ( udcbptr , sdcbptr )

UDCB    *udcbptr;       /* pointer to User Dictionary Control Block     */
SDCB    *sdcbptr;       /* pointer to System Dictionary Control Block   */

{
        extern void    hudcread();     /* user dictionary read.                */
        extern void    hudickc();     /* PC -> 7bit conversion.               */
        extern void    hudiccs();      /* system dictionary check.             */
        extern void    hudicscb();     /* user dictionary check.               */
	extern int     hugetcmp();     /* get cursor move point.               */
	extern int     humsg();        /* display message.     */
	extern int     hudisply();     /*      */

        extern int     hudicupm();     /* user dictionary buffer display       */
        extern int     hudicupc();     /* user dictionary buffer update        */
        extern int     hudicdlb();     /* user dictionary buffer               */
                                /*         delete status code set       */
        extern int     hudicueh();     /* user dictionary end update handler   */
	extern int     huipfld();      /*      */
	extern int     hugetc();       /*      */
	 
        DIFLD   *df;            /* pointer to DIFLD.                    */
        UDCS    *topptr;        /* pointer to top of UDCS.              */
        UDCS    *lastptr;       /* pointer to last of UDCS.             */
        UDCS    *curptr;        /* pointer to current of UDCS.          */
        UDCS    *scp;           /* work pointer to UDCS.                */
        UDCS    *scp2;          /* work pointer to UDCS.                */
        UDCS    *scp3;          /* work pointer to UDCS.                */
        UDMS    *dmappt;        /* pointer to UDMS.                     */

        uchar   *dicindex;           /* pointer to index area.               */
        uchar   *dicdata;           /* pointer to data area.                */
        uchar   merflg;         /* message area erase flag.             */
	uchar   orgdata[50];
	uchar   key[U_KEY_MX];     /* key (PC code) string.       */
	uchar   cand[U_CAN_MX];      /* Kanji (PC code) string.      */
	uchar   hkdata[U_CAN_MX];       /* Kanji conversion data.       */
        uchar   upmode;         /* update mode.                         */
	uchar   ipdata[U_CAN_MX];       /* pointer to update data.      */
	uchar   cchar;          /* sampl data for check.                */
        uchar   empflg;         /* all dic' buffer data are invalid flag*/
        uchar   updflg;         /* work update flag.                    */

	short   result;
        short   indlen;         /* length of index area of user dic.    */
        short   rl;         /* length of data  area of user dic.    */
        short   dllen;          /* length of data  DL   of user dic.    */
        short   indpos;         /* position of index area.              */
        short   datpos;         /* position of data  area.              */
        short   dlpos;          /* position of DL    area.              */
        short   promode;        /* process mode.                        */
        short   lpcnt;          /* loop counter.                        */

	short 	i_len,
		i_rrn,
		i_p,
		i_keylen,
		d_p,
		d_len,
		d_keylen,
		d_candlen;
	uchar	d_keydata [U_KEY_MX],
		d_canddata [U_CAN_MX];
	short   lastcflg;

        short   erlen;          /* length of error data.                */
        short   rrn;            /* record number ob user dictionary.    */
        short   kanalen;        /* length of kanadata. (7bit code)      */
        short   keylen;        /* length of key. (PC   code)      */
        short   g;         /* length of Kanji data.                */
        short   hklen;          /* length of Kanji conversion.          */
        short   posc;           /* position counter.                    */
        short   actfld;         /* active field number.                 */
        short   erfld;          /* error  field number.                 */
        short   dmode;          /* display mode.                        */
        short   smode;          /* search mode.                         */
        short   uplen;          /* active field character length.       */
        short   msgid;          /* message ID.                          */
        short   mflag;          /* PC code character type flag.         */
        short   start_y;         /* start Y axis of input field.         */
        short   maxline;          /* maximum input line number.           */
        short   maxfld;         /* maximum field number.                */

        /***************long    srcflg;        *******************/
        short    srcflg;       

	int     fldno;          /* field number.        */
	int     triger;         /* triger key.  */
	short   col;            /* input field x axis   */
	short   line;           /* input field y axis   */
	short   dtlen;          /* length of input field        */
	short   ipmode;         /* input mode.  */
	char    *data;          /* pointer to input data        */

        int     rc;             /* return code.                         */
        int     wrc;            /* return code.                         */
	int     i,j,k;          /* work variable.                       */


	/* serch data x axis    */
	int     sed_x = 30;

        char *msg_ptr;

	/* head label data.     */
	static  uchar    hl_d[40] =  "** User Dictionary Update **";


	/* message datd X axis. */
	short     msg_x;

	/* message datd Y axis. */
	short     msg_y[2];

	/* message data length. */
	short     msg_l = U_MAXCOL;

	/* function number data X axis. */
	static short     pf_x[6] = {  3 ,  18 , 33 , 3 , 18, 33 };


	/* function number data.        */
	static  uchar    pf_d[][16] =
	  {
		"F2 = Delete",
		"F3 = End",
		"F7 = Previous",
		"F8 = Next",
		"F9 = Search",
		"F12 = Cancel"
	  };

	/* bracket data */
	static uchar *bracket[] = {  "[" , "]" };

        /* 0.
         *     display character dada  and  allocate display map,data.
         */

        udcbptr->uurmf = U_ENDID;

	/* erase display area.  */
	putp(clear_screen);

	/* get start position of input field.   */
	start_y = 3;

	/* get line number of input field.      */
	maxline = udcbptr->ymax - 7;

	/* get maximum field number.    */
	maxfld = maxline - 1;

	msg_ptr = catgets(udcbptr->msg_catd, 1, U_UHDMSGN, "dummy");
        if (strcmp(msg_ptr, "dummy")!=0)
           strncpy(hl_d, msg_ptr, 40);

        /* display head label.                                          */
	col  = ((U_MAXCOL-strlen(hl_d))/2) + udcbptr->xoff;
	line = 0;
	wrc = hudisply(udcbptr,line,col,hl_d,strlen(hl_d));

	col  = 0 + udcbptr->xoff;
	line = 1;
	wrc = hudisply(udcbptr,line,col,(char *)(udcbptr->udfname),strlen(udcbptr->udfname));

	/* display function date        */
	line = udcbptr->ymax - 2;
	for(i=0;i<3;i++)  {
	    msg_ptr = catgets(udcbptr->msg_catd, 1, U_UF1MSGN+i, "dummy");
	    if (strcmp(msg_ptr, "dummy") != 0)
		strncpy(pf_d[i], msg_ptr, 16) ; 
	    col = pf_x[i] + udcbptr->xoff;
	    wrc = hudisply(udcbptr,line,col,pf_d[i],16);
        };

        /* display function text.                                       */
	line = udcbptr->ymax - 1;
	for(i=3;i<6;i++)  {
	        msg_ptr = catgets(udcbptr->msg_catd, 1, U_UF1MSGN+i, "dummy");
	        if (strcmp(msg_ptr, "dummy") != 0)
			strncpy(pf_d[i], msg_ptr, 14) ; 
		col = pf_x[i] + udcbptr->xoff;
		wrc = hudisply(udcbptr,line,col,pf_d[i],strlen(pf_d[i]));
        };

        /* allocate display map.                                        */
        dmappt = (UDMS *)malloc( sizeof(UDMS) );
        if( dmappt == NULL )  return( IUFAIL );
	dmappt->fld = (UDMFLD *)malloc( sizeof(UDMFLD) * maxline );
        if( dmappt->fld == NULL )  return( IUFAIL );

        /* allocate display data.                                       */
        df = (DIFLD *)malloc( sizeof(DIFLD) * maxline );
        if( df == NULL )  return( IUFAIL );

        /*  1.
         *      make user dictionary buffer.
         */
	/* init pointer to user dictionary buffer.                      */
        topptr  = NULL;
        lastptr = NULL;
        curptr  = NULL;

	/*****************************/
        /* get pointer to index area.*/
	/*****************************/
        hudcread(udcbptr , (short)3 , rrn);
        dicindex = udcbptr->rdptr;

        /* get length of index area. */
	i_len = getil(dicindex);

        /* index position init.                                         */
	/***********************/
	/* Index block pointer */
	/***********************/
        i_p = U_ILLEN + U_HARLEN + U_NARLEN;
        /* loop of index. (endloop mark is '$$$$')                      */
        while(TRUE)  {
            /* check !  end of index area.                              */
            if( i_p >= i_len ) break;
	    i_keylen = nxtkeylen(dicindex, i_p, U_UIX_A);
	    (void)getrrn(dicindex+i_p+i_keylen, &i_rrn);
	    i_p += (i_keylen+U_RRNLEN);
            hudcread(udcbptr , (short)4 , i_rrn);
            dicdata = udcbptr->rdptr;
            /* get length of data area.                                 */
	    d_len = getrl(dicdata);
	    /**********************/
	    /* Data Block Pointer */
	    /**********************/
            d_p = U_RLLEN;
            /* loop of data area. (endloop mark is '@@@@') */
            while(TRUE) {
                /* check ! end of data area,*/
                if( d_p >= d_len )  break;
		/**************************/
		/* Next Key At Data block */
		/**************************/
		d_keylen = nxtkeylen(dicdata, d_p, U_REC_L);
		memcpy(d_keydata, dicdata+d_p, d_keylen);
		/************************/	
		/*                      */
		/* Make Korean Standard */
		/* Code.	        */
		/*                      */
		/************************/	
		makeksstr(d_keydata, d_keylen);
		d_p += d_keylen;
                posc = 0;
		do {
		   d_candlen = nxtcandlen(dicdata, d_p, &lastcflg, U_REC_L);
		   memcpy(d_canddata, dicdata+d_p, d_candlen);
		   /************************/	
		   /*                      */
		   /* Make Korean Standard */
		   /* Code.		   */
		   /*                      */
		   /************************/	
		   makeksstr(d_canddata, d_candlen);
                   curptr = (UDCS *)malloc( sizeof(UDCS) );
                   if( curptr == NULL )  return( IUFAIL );
                    /* first allocate process. */
                    if( topptr == NULL )  {
                        /* save first pointer.*/
                        topptr = curptr;
                        /* set pointer to previous.*/
                        curptr->pr_pos = NULL;
                    }
                    else  {
                        /* set pointer to previous. */
                        lastptr->nx_pos = curptr;
                        /* set pointer to next.*/
                        curptr->pr_pos = lastptr;
                    }
                    /* set status information of UDCS. */
                    curptr->status = (uchar)U_S_INIT;
                    curptr->keylen = (uchar)d_keylen;
		    memcpy(curptr->key, d_keydata, (int)d_keylen);
		    memset(curptr->key+d_keylen, C_SPACE, U_KEY_MX-d_keylen);
		    /*************************/
                    /* set position  of UDCS.*/
		    /*************************/
                    posc++;
                    curptr->pos = (uchar)posc;
		    /*************************/
                    /* set cand data of UDCS.*/
		    /*************************/
		    curptr->candlen = (uchar)d_candlen;
		    memcpy(curptr->cand, d_canddata, (int)d_candlen);
		    memset(curptr->cand+d_candlen, C_SPACE, U_CAN_MX-d_candlen);
		    /*****************************/
                    /* keep last pointer to UDCS.*/
		    /*****************************/
                    lastptr = curptr;
                    lastptr->nx_pos = NULL;
		    /***********************/
		    /* Goto next candidate */
		    /***********************/
		    d_p += d_candlen;
		} while(lastcflg == U_FOF);
            }
         }  /* $$$$ endloop */


	/*****************************/
        /* init update process data. */ 
	/*****************************/
        /* set pointer to head of dictionary buffer.*/
        curptr = topptr;

        /* init process flag.*/
        promode = U_P_UPM;

        /* init active field number.*/
	actfld = 0;

        /* init display mode.*/
        dmode = U_DISP;

        /* initialize message erase X Y axis.*/
	msg_x = udcbptr->xoff + 3;
	msg_y[0] = udcbptr->ymax - 4;
	msg_y[1] = udcbptr->ymax - 3;

        /* initialize message area erase flag.*/
        merflg = C_SWOFF;

        /* initialize all user dic' buffer data are invalid flag.       */
        empflg = C_SWOFF;

        /* 3.
         *      update process.
         */
        /* process loop. (endloop mark is '%%%%') */
        while(TRUE)  {

          if( promode == U_P_UPM )  {
              /* 4.
               *      display update data.
               */
	      /* display update data.   */
              wrc = hudicupm( udcbptr , df      , dmode
                            , dmappt  , actfld  , curptr
                            , topptr  , lastptr
                            , start_y  , maxline );
	      fflush(stdout);

              if( wrc == UDNODTE )  {
                /* case : case user all dic' buffer data are invalid.   */

		/* set next process mode.       */
		/*          to 'empty return process'   */
                promode = U_P_EMP;

                /* set all user dic' buffer data are invalid flag.      */
                empflg  = C_SWON;

                continue;
              }

	      /* set first display field.       */
	      for(i=0;i<=maxfld;i++)  {
                if(dmappt->fld[i].fstat != U_NODIS)  break;
              }
	      actfld = i;
	      curptr = dmappt->fld[actfld].dbufpt;

	      /* set next process mode. */
	      /*   to 'input field to be active and wait event' */
              promode = U_P_ACWT;
          }

          if( promode == U_P_ACWT )  {
              /* 5.
               *      input field to be active and wait event.
               */
	      /* set  col,line,length,data of input field       */

	      line = start_y + actfld;
	      if(dmappt->fld[actfld].fstat == U_KEYF) {
		col    = U_XKEY + udcbptr->xoff;
		dtlen  = U_KEY_MX;
		data   = df[actfld].keyfld;
		ipmode = T_KEY;
	      } else {
		col   = U_XCAND + udcbptr->xoff;
		dtlen = U_CAN_MX;
		data  = df[actfld].candfld;
		ipmode = T_CAND;
	      }

	      /* data input loop. (endloop mark is '$$$$')      */
	      while(TRUE)  {
		/* wait input data.     */
		
		triger = huipfld(udcbptr,line,col,data,
		   dtlen,ipmode,&uplen,C_SWOFF,HUDICUPH);

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
                }

		if(triger == U_PF2KEY)  {
		  /* F2 key entered.    */
		  /* set next process mode.     */
		  /*          to 'delete process'       */
		  promode = U_P_DEL;
		  break;
		}

		if(triger == U_PF12KEY)  {
		  /* F12 key entered.    */
		  /* set next process mode.     */
		  /*          to 'cancel return'        */
			promode = U_P_CAN;
		  break;
		}

                /* display discard key enterd message.                  */
                msgid = U_GMSGN;
			/* Invalid key has been pressed.          */
		(void)humsg(udcbptr,msg_y[0],msg_x,msgid);

                /* set message erase flag.                              */
                merflg = C_SWON;
	      }        /* endloop of '$$$$'    */

              /* message erase process.                                 */
              if( merflg == C_SWON )  {
		msgid = U_ENDID;
		(void)humsg(udcbptr,msg_y[0],msg_x,msgid);
		(void)humsg(udcbptr,msg_y[1],msg_x,msgid);
		/* message erase flag to be off.                      */
		merflg = C_SWOFF;
	      }
          }

          if( promode == U_P_SEAR )  {
              /* 8.
               *      search process.
               */
              /* display Keygana input message.                        */
              msgid = U_VMSGN;
			/* Enter after Key Input.       */
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

              /* define Key input area.                            */
	      col   = sed_x + udcbptr->xoff;
	      line  = msg_y[1];
	      dtlen = U_KEY_MX;
	      ipmode = T_KEY;
	      key[0] = NULL;
		/* Draw Bracket */
	      /* write left bracket       */
	      CURSOR_MOVE(line,col-1);
	      fprintf(stdout,"%s",bracket[0]);
	      /* write right bracket      */
	      CURSOR_MOVE(line,col+U_KEY_MX);
	      fprintf(stdout,"%s",bracket[1]);

              while(TRUE) {
		/* get key.*/

		triger = huipfld(udcbptr,line,col,(char *)key,
					dtlen,ipmode,&uplen,C_SWOFF,HUDICUPH);

		/* check inputed key ?    */
		if( (triger == U_ACTIONKEY) ||
		    (triger == U_ENTERKEY ) ||
		    (triger == U_CRKEY    ) ||
		    (triger == U_PF12KEY  ) ||
		    (triger == U_RESETKEY )   )  {
		  break;
		}

		/* display discard data enterd message. */
                msgid = U_GMSGN;
			/* Invalid key has been pressed.          */
		(void)humsg(udcbptr,msg_y[0],msg_x,msgid);
		/* set message erase flag.      */
                merflg = C_SWON;
              }

	      /* check enterd Key data.        */
	      if( (uplen  == 0          ) ||
		  (triger == U_PF12KEY  ) ||
		  (triger == U_RESETKEY )   )  {
		/* case : enterd data is nothing.       */
		/* message erase process. */
		msgid = U_ENDID;
		(void)humsg(udcbptr,msg_y[0],msg_x,msgid);
		(void)humsg(udcbptr,msg_y[1],msg_x,msgid);

		/* set message area erase flag(OFF).    */
                merflg = C_SWOFF;

		/* set next process mode.       */
                /*       to 'input field to be active and wait event'   */
                promode = U_P_ACWT;
                continue;
              };

              /* display search message.                                */
              msgid = U_AEMSGN;
			/* Now Searching... Please Wait a Moment   */
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

	      /****************************/
	      /* search dictionary buffer.*/
	      /****************************/
              smode = U_SD_2;
              hudicscb( smode, curptr , key, uplen, d_canddata , d_candlen, &scp );

	      /* message erase process. */
	      msgid = U_ENDID;
	      (void)humsg(udcbptr,msg_y[0],msg_x,msgid);
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

	      /* set next data display mode.    */
              dmode = U_DISP;

              /* set pointer to current pointer and active field.       */
              curptr = scp;
	      actfld = 0;

	      /* set next process mode. */
	      /* to 'display update data'      */
              promode = U_P_UPM;
          }

	  if( promode == U_P_IPCH )  {
              /* 11.
               *       input data check process.
               */
	      /* set pointer to UDCS of active field.   */
	      scp = dmappt->fld[actfld].dbufpt;

	      /* check !  active field is Key or Candidate ?        */
	      if( dmappt->fld[actfld].fstat == U_KEYF )  {
		/*******************************/
		/* case : active field is Key. */
		/* check !  blank data.        */
		/*******************************/
		if(uplen == 0)  {
		  /*************************/
                  /* blank data input.     */
                  /* display error message.*/
		  /*************************/
                  msgid = U_LMSGN; 	/* Input Key.    */
		  (void)humsg(udcbptr,msg_y[1],msg_x,msgid);
		  /**************************/
                  /* set message erase flag.*/
		  /**************************/
                  merflg = C_SWON;
		  /**************************/
                  /* set next process mode. */
                  /* to 'input & waite'     */
		  /**************************/
                  promode = U_P_ACWT;
                  continue;
                }
		/**************************/
                /* check !  discard data. */
		/**************************/
/*** 
   Local Copy 
****/
		keylen = uplen;
		memcpy(key, df[actfld].keyfld, uplen);

		hudickc(df[actfld].keyfld , uplen, &mflag);

                /* set error field.*/
                erfld = actfld;

                /* set pointer to UDCS of error field.*/
                scp2 = scp;

                if( mflag == U_INVALD ) { 
		  /**************************/
                  /* invalid key data error.*/
                  /* set message ID.        */
		  /**************************/
                  msgid = U_PMSGN; 	/* Invalid characters exist in Key.       */
		  /**************************/
                  /* set next process mode. */
                  /* to 'display message &  */
		  /* error field'           */
		  /**************************/
                  promode = U_P_MSG;
                  continue;
                }
                if( mflag == U_HNMIX )  {
                  /* Hangeul,Numeric mixed.*/
                  /* set message ID.             */
                  msgid = U_ACMSGN;
			/* The Key must consists of only hangul or only numbers */

                  /* set next process mode.*/
                  /* to 'display message & */
		  /* error field'          */
                  promode = U_P_MSG;
                  continue;
                }

                if( uplen > U_B_KEY ) {
                  /* all data and length over ten.             */
                  /* set message ID.                                    */
                  msgid = U_AJMSGN;
			/* Length of the Key exceeds the limit(10 char) */

                  /* set next process mode.                             */
                  /*          to 'display message & error field'        */
                  promode = U_P_MSG;
                  continue;
                };
                if( uplen == scp->keylen )  {
		  k = memcmp(scp->key,df[actfld].keyfld,(int)uplen);
                  if( k == 0 )  {
		    /*************************/
                    /* case : no update.     */
                    /* set next process mode.*/
                    /* to 'cursor process'   */
		    /*************************/
                    promode = U_P_CUMV;
                    continue;
                  }
                }

                /* display updateing message.*/
                msgid = U_AFMSGN;
			/* Now Updating...  Please Wait a Moment   */
		(void)humsg(udcbptr,msg_y[1],msg_x,msgid);
                /* set message erase flag.*/
                merflg = C_SWON;

		/*******************************/
                /* set input data for hudicupc.*/
		/*******************************/
		memcpy(ipdata, (uchar*)df[actfld].keyfld, uplen);

                /* search same Key data in dictionary buffer.*/
                smode = U_SD_0;
		hudicscb( smode, scp, df[actfld].keyfld , uplen, 
		   d_canddata, d_candlen, &scp2 );
                if( scp2 != NULL )  {
                  /* case : same Key data in dictionary buffer.*/
                  /* set error field.                                   */
                  erfld = actfld;

                  /* set error message ID.                              */
                  msgid = U_ABMSGN;
			/* Already Exists Data in User Dictionary.  */

                  /* set pointer to UDCS of error field.                */
                  scp2 = scp;

                  /* set next process mode.                             */
                  /*          to 'display message & error field'        */
                  promode = U_P_MSG;
                  continue;
                };

                /* get start point of Key data.                        */
                erfld = actfld;
                scp3 = scp2  = scp;
                while(TRUE)  {
                  if( scp3->pr_pos == NULL )  break;
                  scp3 = scp3->pr_pos;
                  if( (scp->keylen != scp3->keylen )  ||
                      (memcmp(scp->key,scp3->key,(int)(scp->keylen))
                                               != 0)  )  break;
                  if(    (scp3->status == U_S_INIT)
                      || (scp3->status == U_S_KEYA)
                      || (scp3->status == U_S_CANU)    )  {
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
                  if( (scp->keylen != scp3->keylen )  ||
                      (memcmp(scp->key,scp3->key,(int)(scp->keylen))
                                               != 0)  )  break;
                  if(    (scp3->status == U_S_INIT)
                      || (scp3->status == U_S_KEYA)
                      || (scp3->status == U_S_CANU)    )  {
                    lpcnt = lpcnt + 1;
                  };
                };

                /* set next process mode.                               */
                /*          to 'Key data error check process'          */
                promode = U_P_KEYCH;
	      } else  {
		/*************************************/
		/* case : active field is Candidate. */
		/* check !  blank data.              */
		/*************************************/

                /* check !  enterd data.        */
                if( uplen == scp->candlen )  {
                  k = memcmp( scp->cand , df[actfld].candfld , (int)uplen );
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
                  msgid = U_JMSGN;	 /* Input Candidate. */
		  (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

		  /* set message erase flag.    */
                  merflg = C_SWON;

		  /* set next process mode.     */
		  /*          to 'input and weit'       */
                  promode = U_P_ACWT;
                  continue;
                }


		/* cand data update process.    */
                lpcnt = 1;
                scp2 = scp;

		/* set next process mode.       */
		/*          to 'Goki data error check'  */
                promode = U_P_CANCH;
              }
          }

          if( promode == U_P_KEYCH )  {
              /* 6.
               *      Key data error check.
               */
	      /* keep pointer   */
              scp3 = scp2;

              /* update error check.                                    */
              /* check Key == Candidate.                                    */
              for(i=0;i<lpcnt;scp2=scp2->nx_pos)  {
                /* Key add flag to be OFF.                             */
                scp2->kcaddflg = C_SWOFF;

                if(    (scp2->status != U_S_INIT)
                    && (scp2->status != U_S_KEYA)
                    && (scp2->status != U_S_CANU)    )  continue;

                if(scp2->candlen == uplen)  {
		  k = memcmp(scp2->cand,df[actfld].keyfld,(int)uplen);
                  if( k == 0)  {
                    /* case : Key == Candidate.                           */
                    /* set error message ID.                          */
                    msgid = U_YMSGN;
			/* The Key and the Candidate should not be the same */

                    /* set next process mode.                         */
                    /*      to 'display message and error field'      */
                    promode = U_P_MSG;
                    break;
                  }
                }
                i++;
              }

              if( promode == U_P_MSG ) continue;

              /* check system dictionary.                               */
              scp2 = scp3;
              for(i=0;i<lpcnt;scp2=scp2->nx_pos)  {

                if(    (scp2->status != U_S_INIT)
                    && (scp2->status != U_S_KEYA)
                    && (scp2->status != U_S_CANU)    )  continue;


		hudiccs( sdcbptr , (short)keylen , key, 
		   (short)(scp2->candlen) , scp2->cand, &srcflg);
			
                if( srcflg == 1 )  {
                  /* case : same data is exist in system dictionary.    */

                  /* display error field.                               */
		  if( (erfld < 0) || (maxfld < erfld) )  {
                    /* case : error data not display.                   */
                    /* error data display process.                      */
                    dmode = U_DISP;
                    wrc = hudicupm( udcbptr , df     , dmode  , dmappt
                                  , erfld   , scp2   , topptr , lastptr
                                  , start_y  , maxline  );

                    for(j=0;j<uplen;j++)  df[0].keyfld[j] = (char)ipdata[j];
                    df[0].keyfld[uplen] = NULL;
		    erfld = 0;
                    dmode = U_REDRW;
                    wrc = hudicupm( udcbptr , df     , dmode  , dmappt
                                  , erfld   , scp2   , topptr , lastptr
                                  , start_y  , maxline  );

		    erfld = 1;
                    curptr = scp2;
		  } else {
		    if(erfld == actfld) {
		      erfld++;
		    }
		  }

                  /* error data revers display process.                 */
		  dmode = U_REVER;
                  wrc = hudicupm( udcbptr , df     , dmode  , dmappt
                                , erfld   , scp2   , topptr , lastptr
                                , start_y  , maxline   );

                  /* display error message.    */
                  msgid = U_OMSGN;
			/* Already Exists in System Dictionary.   */
		  (void)humsg(udcbptr,msg_y[0],msg_x,msgid);

                  /* display error message.                             */
                  msgid = U_ADMSGN;
			/* The data will be deleted. Press Enter to continue */
		  (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

                  /* set message erase flag.                            */
                  merflg = C_SWON;

                  /* wait key input.                                    */
                  while(TRUE)  {
		    wrc = hugetc();
		    j = hugetcmp( wrc );
		    if(j == 2) {
		      j = hugetc();
		      wrc = (wrc<<8) | j;
		    };

		    if( (wrc == U_RESETKEY) || (wrc == U_PF12KEY) ) {
		      /* Reset key entered process.        */
		      /* Key add flag to be  OFF.      */
                      for(j=0;j<lpcnt;j++,scp3=scp3->nx_pos)
                        scp3->kcaddflg = C_SWOFF;

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
		      /* Key add flag to be ON.        */
		      dmappt->fld[erfld].dbufpt->kcaddflg
                                                      = C_SWON;

                      break;
                    };
                  };

                  /* error data reset display process.                  */
                  dmode = U_RESET;
                  wrc = hudicupm( udcbptr , df     , dmode  , dmappt
                                , erfld   , scp2   , topptr , lastptr
                                , start_y  , maxline  );

                };
                if(promode == U_P_UPM)  {
                     /* case : stop Key update.                        */
                     /* set display mode.                               */
                     dmode = U_DISP;
                     break;
                };

                /* set field to next Candidate field.                        */
		if(erfld == actfld) {
		  erfld += 2;
		} else {
		  erfld++;
		}
                i++;
              }

	      /* message erase process. */
	      msgid = U_ENDID;
	      (void)humsg(udcbptr,msg_y[0],msg_x,msgid);
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

              /* set next process mode.                                 */
              /*          to 'update process'                           */
              if( (promode == U_P_UPM) || (promode == U_P_MSG) ) continue;
              promode = U_P_UPDT;

          };

          if( promode == U_P_CANCH )  {
              /* 14.
               *      Candidate data error check.
               */

              /* update error check.                                    */
              erfld = actfld;

              if(scp2->keylen == uplen)  {
		k = memcmp(scp2->key, df[actfld].candfld,(int)uplen );
                if( k == 0)  {
                  /* case : Candidate == Key.                               */
                  /* set message ID.                                    */
                  msgid = U_YMSGN;
			/* The Key and the Candidate should not be the same */

                  /* set next process mode.                             */
                  /*         to display message and error field.        */
                  promode = U_P_MSG;
                  continue;
                }
              };

              /* check user dictionary.                                 */
              smode = U_SD_1;
              hudicscb( smode   , scp2
                      , scp2->key , (short)(scp2->keylen)
		      , df[actfld].candfld , uplen , &scp3 );

              if( scp3 != NULL )  {
                /* case : same data exist in dictionary buffer.         */
                /* set error message ID.                                */
                msgid = U_NMSGN;
			/* Already Exists in User Dictionary.     */

                /* set nect process mode.                               */
                /*         to display message and error field.          */
                promode = U_P_MSG;
                continue;
              }

              /* check if key is valid */
              hudickc( scp2->key , (short)(scp2->keylen) , &mflag );
	      keylen = scp2->keylen;
	      memcpy(key, scp2->key, keylen);

              /* check system dictionary.                               */
	      hudiccs(sdcbptr , keylen , key,
		 uplen, df[actfld].candfld,  &srcflg);

              if( srcflg == 1 )  {
		/* case : same data exist in system dictionary. */
		/* error data revers display process.   */
		dmode = U_REVER;
                wrc = hudicupm( udcbptr , df     , dmode  , dmappt
                              , erfld   , scp2   , topptr , lastptr
                              , start_y  , maxline );

		/* display error message.       */
                msgid = U_OMSGN;
			/* Already Exists in System Dictionary.   */
		(void)humsg(udcbptr,msg_y[0],msg_x,msgid);

		/* display error message.       */
                msgid = U_ADMSGN;
			/* The data will be deleted. Press Enter to continue */
		(void)humsg(udcbptr,msg_y[1],msg_x,msgid);

		/* wait reset input.    */
                while(TRUE)  {
		  wrc = hugetc();
		  j = hugetcmp( wrc );
		  if(j == 2) {
		    j = hugetc();
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
                    hudicdlb(dmappt , erfld );

		    /* set next process mode    */
		    /*          to 'display update data'        */
                    promode = U_P_UPM;
                    break;
                  };
                };

		/* error data reset display process.    */
                dmode = U_RESET;
                wrc = hudicupm( udcbptr , df     , dmode  , dmappt
                              , erfld   , scp2   , topptr , lastptr
                              , start_y  , maxline );

		/* message erase process.       */
		msgid = U_ENDID;
		(void)humsg(udcbptr,msg_y[0],msg_x,msgid);
		(void)humsg(udcbptr,msg_y[1],msg_x,msgid);

		/* set display mode.    */
                if( promode == U_P_UPM )  dmode = U_DISP;
                continue;
              }
              else  {
                /* set input data for hudicupc.                         */
		for(i=0;i<uplen;i++) {
		  ipdata[i] = (uchar)(df[actfld].candfld[i]);
		};

                /* set next process mode.                               */
                /*          to 'update process'                         */
                promode = U_P_UPDT;
              }
          }

          if( promode == U_P_UPDT )  {
              /* 12.
               *      update process.
               */

              scp2 = NULL;
              rc = hudicupc( dmappt , actfld , ipdata , uplen
                           , &topptr , &lastptr , &scp2 );

              if( rc == UDOVFDLE )   {
                /* case : RL overflow.                                  */
                /* set pointer to UDCS of error field.                  */
		scp2    = dmappt->fld[actfld].dbufpt;

                /* set error field ID.                                  */
                erfld   = actfld;

                /* set error message ID.                                */
                msgid   = U_XMSGN;
		/* Length of the Key exceeds the limit(10 char).     */

		/* set next process mode        */
                /*          to 'display error message and field'        */
                promode = U_P_MSG;
                continue;
              };

	      /* message erase process. */
	      msgid = U_ENDID;
	      (void)humsg(udcbptr,msg_y[0],msg_x,msgid);
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

              if( rc != IUSUCC ) {
		/* case : other error occured.  */
		/* set work update flag to be OFF.      */
                updflg = C_SWOFF;
                break;
              };

              /* set update flag.                                       */
              updflg = C_SWON;

              if( scp2 != NULL )  {
                /* case : Key field update.                            */
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
		  wrc = hudicupm( udcbptr , df      , dmode
				, dmappt  , actfld  , curptr
				, topptr  , lastptr
				, start_y  , maxline );

		  /* set active field.                               */
		  for(i=0;i<=maxfld;i++)  {
			  if(dmappt->fld[i].fstat != U_NODIS)  break;
		  };
		  actfld = i;
		  curptr = dmappt->fld[i].dbufpt;

		  /* set next process mode                            */
		  /*          to 'search Keygana'                    */
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
	      /* Enter or C/R or TAB key enterd.*/
	      /* check ! next field is used.    */
	      while(TRUE)  {
		actfld = actfld + 1;
		if( actfld > maxfld )  {
		  /* set first field      */
		  /* because no use filed */
		  /* behind current field */
		  actfld = 0;
		};
		if( dmappt->fld[actfld].fstat != U_NODIS )  {
		  /* next field is used.*/
		  break;
		};
	      };
	      /* set next process mode.*/
	      /* to 'input field to be */
	      /* active and wait event'*/
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
	      /*          to 'search Keygana'        */
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
	      /* check !  delete Candidate data and  */
	      /*          Key has Candidate that's only one.        */
	      if(   dmappt->fld[actfld].fstat == U_CANDF
		&&  dmappt->fld[actfld-1].fstat == U_KEYF ) {

		if(actfld == 1)  {
		  /* case : display at top of display.  */
                  if(    dmappt->prestat == C_SWOFF
		      && dmappt->fld[actfld+1].fstat != U_CANDF ) {
                    /* case : key has Candidate that's only one.            */
                    /* set field to it's Key field.                    */
                    actfld--;
                  };
                } else if(actfld  == maxfld)  {
                  /* case : display at bottom of display.               */
                  if(dmappt->poststat == C_SWOFF)  {
                    /* case : key has Candidate that's only one.            */
                    /* set field to it's Key field.                    */
                    actfld--;
                  };
		} else if(dmappt->fld[actfld+1].fstat != U_CANDF) {
                    /* case : key has Candidate that's only one.            */
                    /* set field to it's Key field.                    */
                    actfld--;
                };

              };

              /* reverse display delete data.                            */
              erfld = actfld;
	      dmode = U_REVER;
              wrc = hudicupm( udcbptr , df      , dmode
                            , dmappt  , actfld  , curptr
                            , topptr  , lastptr
                            , start_y  , maxline );

              /* revers display delete cand data.                       */
	      if( dmappt->fld[actfld].fstat == U_KEYF )  {
                do {
                  erfld++;
		  dmode = U_REVER;
                  wrc = hudicupm( udcbptr , df      , dmode
                                , dmappt  , erfld   , curptr
                                , topptr  , lastptr
                                , start_y  , maxline );
                } while(  erfld < maxfld
		      &&  dmappt->fld[erfld+1].fstat == U_CANDF );
	      };

              /* display error message.                                 */
              msgid = U_TMSGN;
		/* The data will be deleted. Press Enter to continue */
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

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
		/* Update was cancelled. press Enter to go main menu */
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

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
		triger = hugetc();

                /* check ! entered key.                                 */
		if( (triger == U_ACTIONKEY) ||
		    (triger == U_CRKEY    ) ||
		    (triger == U_ENTERKEY )    )  {
		  /* action key entered process.        */
                  if( upmode == C_SWON )  {
		    /* delete data from dictionary buffer.      */
		    hudicdlb(dmappt,actfld);

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
			/* Update has been cancelled.               */
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
		/* Invalid key has been pressed.          */
		(void)humsg(udcbptr,msg_y[0],msg_x,msgid);
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
	      (void)humsg(udcbptr,msg_y[0],msg_x,msgid);
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

              if( upmode == C_SWON  &&  promode != U_P_UPM )  {
		/* reset process.       */
                erfld = actfld;
                dmode = U_RESET;
                wrc = hudicupm( udcbptr , df      , dmode
                              , dmappt  , actfld  , curptr
                              , topptr  , lastptr
                              , start_y  , maxline );

		/* check !  active field is Key or Goki ?      */
		if( dmappt->fld[actfld].fstat == U_KEYF )  {
                  do {
                    erfld++;
                    dmode = U_RESET;
                    wrc = hudicupm( udcbptr , df      , dmode
                                  , dmappt  , erfld   , curptr
                                  , topptr  , lastptr
                                  , start_y  , maxline );
                  } while(  erfld < maxfld
			&&  dmappt->fld[erfld+1].fstat == U_CANDF);
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
                wrc = hudicupm( udcbptr , df     , dmode  , dmappt
                              , erfld   , scp2   , topptr , lastptr
                              , start_y  , maxline );
		erfld = 0;
                curptr = scp2;
              };

	      /* error data revers display process.     */
	      dmode = U_REVER;
              wrc = hudicupm( udcbptr , df     , dmode  , dmappt
                            , erfld   , scp2   , topptr , lastptr
                            , start_y  , maxline );

              /* display error message.                                 */
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);
              /* set message erase flag.                              */
              merflg = C_SWON;
	      CURSOR_MOVE(line,col);
	      fflush(stdout) ;

              /* wait reset input.                                      */
              while(TRUE)  {
		wrc = hugetc();
		if( (wrc == U_RESETKEY) || (wrc == U_PF12KEY) ) {
		  /* reset key entered process. */
                  break;
                };
              };

	      /* error data reset display process.      */
              dmode = U_RESET;
              wrc = hudicupm( udcbptr , df     , dmode  , dmappt
                            , erfld   , scp2   , topptr , lastptr
                            , start_y  , maxline );

	      /* message erase process. */
	      msgid = U_ENDID;
	      (void)humsg(udcbptr,msg_y[0],msg_x,msgid);
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

	      /* set next process mode  */
	      /*          to 'display update data'      */
              promode = U_P_ACWT;
          };

          if( promode == U_P_END )  {
              /* 7.
               *       end return process.
               */
	      /* update user dictionary.        */
              rc = hudicueh( topptr , lastptr , udcbptr , &curptr );
              if(rc == UDSUCC)  {
		/* set update uty return mode flag.     */
                udcbptr->uurmf = U_ENDID;
                break;
              };
	      /* refresh user dictionary.       */
              (void)memcpy(udcbptr->dcptr,udcbptr->secbuf
                                         ,(int)udcbptr->ufilsz);

	      /* set error message id.  */
              if(rc == UDDCFULE)  msgid = U_QMSGN;
					/* The dictionary file size exceeds the limit */
              else                msgid = U_XMSGN;
					/* Length of the Key exceeds the limit(10 char). */

	      /* display error message. */
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

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
		/* No data left in dictionary file.            */
	      (void)humsg(udcbptr,msg_y[0],msg_x,msgid);

              /* display input message.  */
              msgid = U_AOMSGN;
		/* Please press F3 or F12 key.  */
	      (void)humsg(udcbptr,msg_y[1],msg_x,msgid);

	      /* set message erase flag.        */
              merflg = C_SWON;

	      /* wait input data.       */
              while(TRUE)  {
		wrc = hugetc();

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
		(void)humsg(udcbptr,msg_y[0],msg_x,msgid);
              };
	      if(merflg == C_SWON)  {
		/* message erase process. */
		msgid = U_ENDID;
		(void)humsg(udcbptr,msg_y[0],msg_x,msgid);
		(void)humsg(udcbptr,msg_y[1],msg_x,msgid);

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
        if( udcbptr->updflg == C_SWOFF )  udcbptr->updflg = updflg;

        /* return.                                                      */
        return( rc );

}

