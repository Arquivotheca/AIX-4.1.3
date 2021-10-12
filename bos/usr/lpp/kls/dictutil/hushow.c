static char sccsid[] = "@(#)26  1.1  src/bos/usr/lpp/kls/dictutil/hushow.c, cmdkr, bos411, 9428A410j 5/25/92 14:47:43";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hushow.c
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
 *  Module:       hushow.c
 *
 *  Description:  display user dictionary to screen.
 *
 *  Functions:    hushow()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include file.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Memory package.                              */
#include "hut.h"        /* Dictionary Utility Define File.   */

int     hushow ( udcbptr , dmappt , topptr , lastptr )

UDCB    *udcbptr;       /* pointer to UDCB.     */
UDMS    *dmappt;        /* pointer to UDMS.     */
UDCS    *topptr;        /* pointer to top of UDCS.      */
UDCS    *lastptr;       /* pointer to last of UDCS.     */

{
        void    hudicscb();     /* user dictionary check.               */
        int     hudicupm();     /* user dictionary buffer display       */
                                /*         delete status code set       */
	int     hudisply();     /*      */
	int     humsg();
	int     hugetc();       /*      */

        DIFLD   *df;            /* pointer to DIFLD.                    */
        UDCS    *curptr;        /* pointer to current of UDCS.          */
        UDCS    *scp;           /* work pointer to UDCS.                */

        uchar   merflg;         /* message area erase flag.             */
	uchar   keydata[U_KEY_MX]; /* yomi (PC code) string.           */
	uchar   canddata[U_CAN_MX];  /* Candidate string.          */
        short   promode;        /* process mode.                        */

        short   candlen;         /* length of Candidate data.                */
        short   actfld;         /* active field number.                 */
        short   dmode;          /* display mode.                        */
        short   smode;          /* search mode.                         */
        short   uplen;          /* active field character length.       */
        short   msgid;          /* message ID.                          */
        short   start_y;         /* start Y axis of input field.         */
        short   maxlinenum;          /* maximum input line number.           */
        short   maxfld;         /* maximum field number.                */


	int     triger;         /* triger key.  */
	short   col;            /* input field x axis   */
	short   line;           /* input field y axis   */
	short   dtlen;          /* length of input field        */
	short   ipmode;         /* input mode.  */
	uchar    *data;          /* pointer to input data        */

        int     wrc;            /* return code.                         */
        int     i,j,k;          /* work variable.                       */

	/* bracket data */
	static uchar *bracket[] = { "[", "]" }; 

	/* search data x axis    */
	short     sedi_x = 30;

	char *msg_ptr;

	/* head label data.     */
	static  uchar    hl3d_d[42] = "** User Dictionary Display **";

	/* message datd X axis. */
	short     msg_x;

	/* message datd Y axis. */
	short     msg_y[2];

	/* message data length. */
	short     msg_l = U_MAXCOL;

	/* function number data X,Y axis.       */
	static short     pf_x[5] = {  3 ,  13 , 28 , 39 , 52 };

	/* function number data.        */
	static  uchar    pf_d[][14] = {
		"F3 = End",
		"F7 = Previous",
		"F8 = Next",
		"F9 = Search",
		"F12 = Cancel"
	};


/* 0.
*     display character dada  and  allocate display map,data.
*/

	/* erase display area.  */
	putp(clear_screen);

	msg_ptr = catgets(udcbptr->msg_catd, 1, U_HD3MSGN, "dummy");
	if (strcmp(msg_ptr, "dummy") !=0 )
		strncpy(hl3d_d, msg_ptr, 42);
	/* display head label.                                          */
	col  = ((U_MAXCOL-strlen(hl3d_d))/2) + udcbptr->xoff;
	line = 0;

	wrc = hudisply(udcbptr,line,col,hl3d_d,strlen(hl3d_d));

	col  = 0 + udcbptr->xoff;
	line = 1;
	wrc = hudisply(udcbptr,line,col,(char *)(udcbptr->udfname),strlen(udcbptr->udfname));

	/* display function number.              */
	line = udcbptr->ymax - 1;

	for(i=0;i<4;i++)
	{
		msg_ptr = catgets(udcbptr->msg_catd, 1, U_UF2MSGN+i, "dummy");
		if (strcmp(msg_ptr, "dummy") !=0 )
			strncpy(pf_d[i], msg_ptr,14);
		col = pf_x[i] + udcbptr->xoff;
		wrc = hudisply(udcbptr,line,col,pf_d[i],14);
	};

/* 1.
*      init update process data.
*/

	/* get start position of input field.   */
	start_y = 3;

	/* get line number of input field.      */
	/* change IBM-J		*/
	maxlinenum = udcbptr->ymax - 7;

	/* get maximum field number.    */
	maxfld = maxlinenum - 1;

	/* set pointer to head of dictionary buffer.    */
        curptr = topptr;

	/* init process flag.   */
        promode = U_P_UPM;

	/* init display mode.   */
        dmode = U_DISP;

	/* init active field number.                                    */
	actfld = 0;

	/* initialize message erase X Y axis.   */
	msg_x = udcbptr->xoff + 3;

	/* change IBM-J		*/
	msg_y[0] = udcbptr->ymax - 4;
	msg_y[1] = udcbptr->ymax - 3;

	/* initialize message area erase flag.                          */
	merflg = C_SWOFF;

	/* set pointer to df    */
	df = (DIFLD *)malloc(sizeof(DIFLD) * maxlinenum);

/* 2.
*      display process.
*/

	/* process loop. (endloop mark is '%%%%')                       */
        while(TRUE)
	{

		if( promode == U_P_UPM )
		{

/* 3.
*      display update data.
*/

		/* display update data.   */
			wrc = hudicupm( udcbptr , df      , dmode
				, dmappt  , actfld  , curptr
				, topptr  , lastptr
				, start_y  , maxlinenum );

		/* set first display field.       */
			for(i=0;i<=maxfld;i++)
			{
				if(dmappt->fld[i].fstat != U_NODIS)  break;
			};
			actfld = i;
			curptr = dmappt->fld[actfld].dbufpt;

		/* set next process mode. */
		/*   to 'input field to be active and wait event' */
			promode = U_P_ACWT;
		};

		if( promode == U_P_ACWT )
		{

/* 5.
*      wait event.
*/

		/* data input loop. (endloop mark is '&&&&')      */
			while(TRUE)
			{
				fflush(stdout);
	
			/* wait input data.     */
				triger = hugetc();
	
				if(triger == U_PF7KEY)
				{
				/* F7key entered.       */
				/* set next process mode        */
				/*          to 'display update data'    */
					promode = U_P_UPM;

				/* set display mode. (next page)        */
					dmode = U_BEFORP;

					break;
				};
	
				if(triger == U_PF8KEY)
				{
				/* F8 key entered.      */
				/* set next process mode        */
				/*          to 'display update data'    */
					promode = U_P_UPM;
	
				/* set display mode. (previous page)    */
					dmode = U_NEXTP;
	
					break;
				};

				if(triger == U_PF9KEY)
				{
				/* F9 key entered.      */
				/* set next process mode        */
				/*          to 'search Key'        */
					promode = U_P_SEAR;

					break;
				};

				if(triger == U_PF3KEY)
				{
				/* F12 key entered.     */
				/* set next process mode        */
				/*          to 'end return'     */
					break;
				};

			/* display discard key enterd message.          */
				msgid = U_GMSGN;
				(void)humsg(udcbptr,msg_y[0],msg_x,msgid);

			/* set message erase flag.                   */
				merflg = C_SWON;
			};        /* endloop of '$$$$'    */

		/* message erase process.                                 */
			if( merflg == C_SWON )
			{
				(void)humsg(udcbptr,msg_y[0],msg_x, (short)U_ENDID);
				(void)humsg(udcbptr,msg_y[1],msg_x,(short)U_ENDID);
			/* message erase flag to be off.                      */
				merflg = C_SWOFF;
			};
			if(triger == U_PF3KEY)
			{
				break;
			};

		};

		if( promode == U_P_SEAR )
		{
/* 8.
*      search process.
*/
			line = udcbptr->ymax - 1;
			col  = pf_x[4] + udcbptr->xoff;
			wrc = hudisply(udcbptr,line,col,pf_d[4],strlen(pf_d[4]));

		/* display Key input message.                        */
			msgid = U_VMSGN;
			(void)humsg(udcbptr,msg_y[1],msg_x,msgid);

		/* define Key input area.                            */
			col   = sedi_x + udcbptr->xoff;
			dtlen = U_KEY_MX;
			ipmode = T_KEY;
			keydata[0] = NULL;

		/* write left bracket       */
			CURSOR_MOVE(msg_y[1],col-1);
			fprintf(stdout,"%s",bracket[0]);

		/* write right bracket      */
			CURSOR_MOVE(msg_y[1],col+U_KEY_MX);
			fprintf(stdout,"%s",bracket[1]);

			while(TRUE)
			{
			/* get key data.   */
				line = msg_y[1];
				triger = huipfld(udcbptr,line,col
					,(char *)keydata,dtlen,ipmode,&uplen,C_SWOFF, HUSHOW);

			/* check inputed key is Action key ?    */
				if( (triger == U_ACTIONKEY) ||
					(triger == U_CRKEY) ||
					(triger == U_ENTERKEY) ||
					(triger == U_PF12KEY) ||
					(triger == U_RESETKEY ))
				{
					break;
				};

			/* display discard data enterd message. */
				(void)humsg(udcbptr,msg_y[0],msg_x,(short)U_GMSGN);

			/* set message erase flag.      */
				merflg = C_SWON;
			};

		/* check enterd Yomi data.        */
			if( (uplen  == 0          ) ||
				(triger == U_PF12KEY  ) ||
					(triger == U_RESETKEY )     )
			{
			/* case : enterd data is nothing or process reset   */
			/* message erase process. */
				(void)humsg(udcbptr,msg_y[0],msg_x,(short)U_ENDID);
				(void)humsg(udcbptr,msg_y[1],msg_x,(short)U_ENDID);
				line = udcbptr->ymax - 1;
				col  = pf_x[4] + udcbptr->xoff;
				wrc = hudisply(udcbptr,line,col," ",strlen(pf_d[4]));

			/* set message area erase flag(OFF).    */
				merflg = C_SWOFF;

			/* set next process mode.       */
			/* to 'input field to be active and wait event'   */
				promode = U_P_ACWT;
				continue;
			};

		/* display search message.                                */
			msgid = U_AEMSGN;
			(void)humsg(udcbptr,msg_y[1],msg_x,msgid);

		/* search dictionary buffer.      */
			smode = U_SD_2;
			hudicscb( smode   , curptr , keydata , uplen
				, canddata , candlen , &scp );

		/* message erase process. */
			(void)humsg(udcbptr,msg_y[0],msg_x,(short)U_ENDID);
			(void)humsg(udcbptr,msg_y[1],msg_x,(short)U_ENDID);
			line = udcbptr->ymax - 1;
			col  = pf_x[4] + udcbptr->xoff;
			wrc = hudisply(udcbptr,line,col," ",strlen(pf_d[4]));

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
	putp(clear_screen);
	return( wrc );

}
