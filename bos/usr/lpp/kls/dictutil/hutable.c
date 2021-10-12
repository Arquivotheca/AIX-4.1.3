static char sccsid[] = "@(#)29  1.1  src/bos/usr/lpp/kls/dictutil/hutable.c, cmdkr, bos411, 9428A410j 5/25/92 14:48:28";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hutable.c
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
 *  Module:       hutable.c
 *
 *  Description:  user dictionary display handler
 *
 *  Functions:    hutable()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include file.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Memory package.                              */
#include <fcntl.h>      /* File contorol Package.                       */
#include <errno.h>      /* system call error ID.                        */
#include <sys/stat.h>

#include "hut.h"        /* Dictionary Utility Define File.   */

int     hutable ( udcbptr )

UDCB    *udcbptr;       /* pointer to User Dictionary Control Block     */
{
        void    hudcread();     /* user dictionary read.                */
	int     hugetcmp();     /* get cusor move point.*/
	int     humsg();        /* display message.     */
	int     hudisply();     /* display character string     */
	int     huipfld();      /* input string data    */
	int     hufout();       /* output user dictionary to file       */
	int     hushow();       /* display data */
	int     hufnc();        /* check file name      */

	UDCS    *topptr;        /* pointer to top of UDCS.              */
        UDCS    *lastptr;       /* pointer to last of UDCS.             */
        UDCS    *curptr;        /* pointer to current of UDCS.          */
        UDMS    *dmappt;        /* pointer to UDMS.                     */

        uchar   *dpt;           /* pointer to data area.                */
        uchar   cchar;          /* sampl data for check.                */
	short 	i_len,
 		i_p,
		i_rrn,
		i_keylen,
		d_len,
		d_keylen,
		d_p,
		posc,
		d_candlen,
		lastcflg;
	uchar   d_keydata [U_KEY_MX],
		l_canddata [U_CAN_MX],
		d_canddata [U_CAN_MX];		
	uchar	*dicdata, *dicindex;

        short   uplen;          /* active field character length.       */
        short   msgid;          /* message ID.                          */
        short   hajime;         /* start Y axis of input field.         */
        short   max_line;          /* maximum input line number.           */
        short   maxfld;         /* maximum field number.                */

	int     triger;         /* triger key.  */
	short   col;            /* input field x axis   */
	short   line;           /* input field y axis   */
	short   dtlen;          /* length of input field        */
	short   ipmode;         /* input mode.  */
	uchar    *data;          /* pointer to input data        */

	int     rc;             /* return code. */
        int     wrc;            /* return code.                         */
        int     i,j,k;          /* work variable.                       */
	uchar    outfname[20];   /* output file name     */
	uchar    *pwd;           /* dir  */
    	uchar    tmpf[40];       /* pointer to temporary file name       */
	uchar   merflg;         /* message area erase flag.             */
	uchar   nebflg;         /* numeirc enterd break flag.           */
	int     fd;             /* file discrdicindexer      */

	/* parameter for system */
	uchar    pkick[1024];

	/* menu select handling key     */
	static int      ONE[2] = {0x31,0xa3b1};
	static int      TWO[2]   = {0x32,0xa3b2};
	static int      THREE[2]  = {0x33,0xa3b3};

	/* head label data length.      */
	static int     hl3_l[2] = {  26 ,  U_MAXCOL };

        char *msg_ptr;
	/* head label data.     */
	static  uchar    hl3_d[40] = "** User Dictionary List **";

	/* message datd X axis. */
	short     msg_x = 3;

	/* message datd Y axis. */
	short     msg_y = 14;

	/* function number data X,Y axis.       */
	/* change IBM-J         */ 
	static int     pf_x1 = 3;
	static int     pf_x2 = 19;
	static int     pf_y1 = 16;
	static int     pf_y2 = 16;


	/* function number data.        */
	/* change IBM-J         */ 
	static  uchar    pf_d1[14] = "Enter = Do";
	static  uchar    pf_d2[14] =  "F3 = End";

	/* menu area X,Y axis & datalegth & data        */
	int             menun;
	static int      menux = 3;
	static int      menuy[3] = { 8 , 9 , 10 };
	static uchar    menud[][14] = {
		" 1. Screen  ",
		" 2. Printer ",
		" 3. File    "
	};

	static int      filex = 17;
	static int      filel = 16;

	/* text data X,Y axis & data & data length      */
	static int      textx = 3;
	/* change IBM-J         */ 
	static int      texty[2] = {5,6};

	/* change IBM-J         */ 
	static uchar     textd[2][45] = {
		"This function lists content of dictionary",
		"Move cursor to desired item and press Enter"
	};


/* 0.
*     display character dada  and  allocate display map,data.
*/

	/* set default file name        */
	memcpy(outfname,"dict.list           ",(int)20);
	outfname[19] = NULL;

	/* erase display area.  */
	putp(clear_screen);

	msg_ptr = catgets(udcbptr->msg_catd, 1, U_LHDMSGN, "dummy");
	if (strcmp(msg_ptr, "dummy") != 0)
		strncpy(hl3_d, msg_ptr, 40) ;

	/* display head label.  */
	col  = ((U_MAXCOL-strlen(hl3_d))/2) + udcbptr->xoff;
	line = 1 + udcbptr->yoff;
	wrc = hudisply(udcbptr,line,col,hl3_d,strlen(hl3_d));
	col  = 0 + udcbptr->xoff;
	line = 3 + udcbptr->yoff;
	wrc = hudisply(udcbptr,line,col,(char *)(udcbptr->udfname),strlen(udcbptr->udfname));

	/* display function number.     */
	/* change IBM-J         */ 
	col = pf_x1 + udcbptr->xoff;
	line = pf_y1 + udcbptr->yoff;
	msg_ptr = catgets(udcbptr->msg_catd, 1, U_FOOTMSGN, "dummy");
	if (strcmp(msg_ptr, "dummy") !=0)
	   strncpy(pf_d1, msg_ptr, 14) ;
	wrc = hudisply(udcbptr,line,col,pf_d1,strlen(pf_d1));
	col = pf_x2 + udcbptr->xoff;
	line = pf_y2 + udcbptr->yoff;
	msg_ptr = catgets(udcbptr->msg_catd, 1, U_F18MSGN, "dummy");
	if (strcmp(msg_ptr, "dummy") !=0)
	   strncpy(pf_d2, msg_ptr, 14) ;
	wrc = hudisply(udcbptr,line,col,pf_d2,strlen(pf_d2));

	/* display text data    */
	col = textx + udcbptr->xoff;
	line = texty[0] + udcbptr->yoff;
	msg_ptr = catgets(udcbptr->msg_catd, 1, U_LTX1MSGN, "dummy");
 	if (strcmp(msg_ptr, "dummy") != 0)
	   strncpy(textd[0], msg_ptr, 45);
	wrc = hudisply(udcbptr,line,col,textd[0],strlen(textd[0]));
	line = texty[1] + udcbptr->yoff;
	msg_ptr = catgets(udcbptr->msg_catd, 1, U_LTX2MSGN, "dummy");
 	if (strcmp(msg_ptr, "dummy") != 0)
	   strncpy(textd[1], msg_ptr, 45);
	wrc = hudisply(udcbptr,line,col,textd[1],strlen(textd[1]));

	/* get line number of input field.      */
	max_line = udcbptr->ymax - 6;

	/* allocate display map.        */
        dmappt = (UDMS *)malloc( sizeof(UDMS) );
        if( dmappt == NULL )  return( IUFAIL );
	dmappt->fld = (UDMFLD *)malloc( sizeof(UDMFLD) * max_line );
        if( dmappt->fld == NULL )  return( IUFAIL );



/*  1.
*      make user dictionary buffer.
*/

	/* init pointer to user dictionary buffer.      */
	topptr  = NULL;
	lastptr = NULL;
	curptr  = NULL;

        hudcread(udcbptr , (short)3 , 0);
        dicindex = udcbptr->rdptr;

        /* get length of index area. */
        i_len = getil(dicindex);

        i_p = U_ILLEN + U_HARLEN + U_NARLEN;
        while(TRUE)  {
            if( i_p >= i_len ) break;
            i_keylen = nxtkeylen(dicindex, i_p, U_UIX_A);
            (void)getrrn(dicindex+i_p+i_keylen, &i_rrn);
            hudcread(udcbptr , (short)4 , i_rrn);
            dicdata = udcbptr->rdptr;
            d_len = getrl(dicdata);
            d_p   = U_RLLEN;
            while(TRUE) {
                if( d_p >= d_len )  break;
                d_keylen = nxtkeylen(dicdata, d_p, U_REC_L);
                memcpy(d_keydata, dicdata+d_p, d_keylen);
		makeksstr(d_keydata, d_keylen);
                d_p += d_keylen;
                posc = 0;
                do {
                   d_candlen = nxtcandlen(dicdata, d_p, &lastcflg, U_REC_L);
                   memcpy(d_canddata, dicdata+d_p, d_candlen);
                   /* alloc dictionary buffer.        */
                   /* --------------------------------*/
                   /* make double linked list of UDCS */
                   /* --------------------------------*/
                   /* ---------------------------------------*/
                   /* topptr                                 */
                   /*   |                                    */
                   /*   |                                    */
                   /*  \|/                                   */
                   /*        --->        ---> --> -->        */
                   /* UDCS 1      UDCS 2              UDCS n */
                   /*        <---        <--- <-- <--        */
                   /*                                  /|\   */
                   /*                                   |    */
                   /*                                   |    */
                   /*                                lastptr */
                   /* ---------------------------------------*/
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
                    /* set position  of UDCS.*/
                    posc++;
                    curptr->pos    = (uchar)posc;
                    curptr->status = (uchar)U_S_INIT;
                    curptr->keylen = (uchar)d_keylen;
                    memcpy(curptr->key, d_keydata, (int)d_keylen);
                    /* set cand data of UDCS.*/
                    curptr->candlen = (uchar)d_candlen;
		    makeksstr(d_canddata, d_candlen);
                    memcpy(curptr->cand, d_canddata, (int)d_candlen);
                    lastptr = curptr;
                    lastptr->nx_pos = NULL;
                    d_p += d_candlen;
                } while(lastcflg == U_FOF);
            }
	    i_p += (i_keylen+U_RRNLEN);
         }  

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

	/* process loop. (endloop mark is '%%%%')       */
	while(TRUE)
	{
	/* display file input area   */
		col = filex + udcbptr->xoff;
		line = menuy[2] + udcbptr->yoff;
		wrc = hudisply(udcbptr,line,col,outfname,20);
		/* Draw Bracket */
		CURSOR_MOVE(line,col-1);
		printf("[");
		CURSOR_MOVE(line,col+filel);
		printf("]");

	/* display massage    */
		(void)humsg(udcbptr,msg_y,msg_x,msgid);

		while(TRUE)
		{ /* loop of key input. (endloop mark is '!!!!')    */

		/*
		*    display menu
		*/
			col = menux + udcbptr->xoff;
			for(i=0;i<3;i++)
			{
				line = menuy[i] + udcbptr->yoff;
				if(i == menun)
				{
					putp(enter_reverse_mode);
				};
				msg_ptr = catgets(udcbptr->msg_catd, 1, U_LNM1MSGN + i, "dummy");
				if (strcmp(msg_ptr, "dummy") != 0)
					strncpy(menud[i], msg_ptr, 14);
				wrc = hudisply(udcbptr,line,col,menud[i],strlen(menud[i]));
				if(i == menun)
				{
					putp(exit_attribute_mode);
				}
			}

		/* message erase process.   */
			if( merflg == C_SWON )
			{
				msgid = U_ENDID;
				(void)humsg(udcbptr,msg_y,msg_x,msgid);
				merflg = C_SWOFF;
			};
			CURSOR_MOVE(udcbptr->yoff+menuy[menun],col);
                        fflush(stdout);

		/* check key pattern */
			if(nebflg == C_SWON)
			{
				nebflg = C_SWOFF;
				break;
			};

		/* get input key      */
			triger = hugetc();

		/* check  getcode match with mode       */
			i = hugetcmp( triger );
			if(i == 2)
			{
			/* check DBCS         */
				i = hugetc();
				triger = triger<<8 | i;
			};

		/* check input key    */
			if( (triger == ONE[0]) ||
				(triger == ONE[1]) )
			{
			/* select 1 key */
				menun = 0;
				nebflg = C_SWON;
				continue;
			};
			if( (triger == TWO[0]) ||
				(triger == TWO[1]) )
			{
			/* select 2 key */
				menun = 1;
				nebflg = C_SWON;
				continue;
			};
			if( (triger == THREE[0]) ||
				(triger == THREE[1]) )
			{
			/* select 3 key */
				menun = 2;
				nebflg = C_SWON;
				continue;
			};
			if( (triger == U_C_DOWN)  ||
				(triger == U_C_RIGHT) ||
					(triger == U_TABKEY)   )
			{
			/* cursor down key        */

				menun++;
				if(menun > 2) menun = 0;

			/* message erase */
				if((msgid == U_BAMSGN ) ||
					(msgid == U_AZMSGN))
				/* Finished printing user dictionary.     */
				/* Finished writing user dictionary to the above file */
				{
					msgid = U_ENDID;
					(void)humsg(udcbptr,msg_y,msg_x,msgid);
				}

				continue;
			};
			if( (triger == U_C_UP)    ||
				(triger == U_C_LEFT)  ||
					(triger == U_BTABKEY)   )
			{
			/* cursor up key  */

				menun--;
				if(menun < 0) menun = 2;

			/* message erase */
				if((msgid == U_BAMSGN ) ||
					(msgid == U_AZMSGN))
				/* Finished printing user dictionary.     */
				/* Finished writing user dictionary to the above file */
				{
					msgid = U_ENDID;
					(void)humsg(udcbptr,msg_y,msg_x,msgid);
				}

				continue;
			};
			if( (triger ==  U_ENTERKEY)  ||
				(triger ==  U_CRKEY   ) ||
					(triger ==  U_ACTIONKEY) ||
						(triger ==  U_PF3KEY )  )
			{
			/* process decision       */
				break;
			};

			merflg = C_SWON;
		}; /* end loop of '!!!!'      */

	/* check process      */
		if(triger == U_PF3KEY)
		{
		/* make the process to be ended */
			break;
		};

		if(menun == 0)
		{
		/* table display    */
			wrc = hushow(udcbptr,dmappt,topptr,lastptr);
		/* display head label.  */
			col  = ((U_MAXCOL-strlen(hl3_d))/2) + udcbptr->xoff;
			line = 1 + udcbptr->yoff;
			wrc = hudisply(udcbptr,line,col,hl3_d,strlen(hl3_d));
			col  = 0 + udcbptr->xoff;
			line = 3 + udcbptr->yoff;
			wrc = hudisply(udcbptr,line,col,(char *)(udcbptr->udfname),strlen(udcbptr->udfname));

		/* display function number.     */
		/* change IBM-J         */ 
			col = pf_x1 + udcbptr->xoff;
			line = pf_y1 + udcbptr->yoff;
			wrc = hudisply(udcbptr,line,col,pf_d1,strlen(pf_d1));
			col = pf_x2 + udcbptr->xoff;
			line = pf_y2 + udcbptr->yoff;
			wrc = hudisply(udcbptr,line,col,pf_d2,strlen(pf_d2));

		/* display text data    */
			col = textx + udcbptr->xoff;
			line = texty[0] + udcbptr->yoff;
			wrc = hudisply(udcbptr,line,col,textd[0],strlen(textd[0]));
			line = texty[1] + udcbptr->yoff;
			wrc = hudisply(udcbptr,line,col,textd[1],strlen(textd[1]));
			continue;
		};

		if(menun == 1)
		{  /* if of menu 2 or 3 (endif mark is '++++') */

		/* case print out   */
		/* make temporary file      */
			pwd = (char *)getcwd((char *)NULL,64);
			if(pwd == NULL)
			{
				msgid = U_BBMSGN;
				/* File access permission denied */
				continue;
			};

/*
			tmpf = tempnam(pwd,"huprn");
*/
			strcpy(tmpf,getenv("HOME"));
			strcat(tmpf,"/.huprn");
			if(tmpf == NULL)
			{
				msgid = U_BBMSGN;
				/* File access permission denied */
				continue;
			};
			msgid = U_BAMSGN;
			/* Finished printing user dictionary.     */

		/* output to temp file      */
			fd=creat(tmpf,(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
			if(fd < 0)
			{
				msgid = U_BBMSGN;
				/* File access permission denied */
				continue;
			};
			rc = hufout(fd,topptr);
			close(fd);

		/* set msg  */
			if(rc != IUSUCC)
			{
				msgid = U_BBMSGN;
				/* File access permission denied */
				continue;
			};

		/* set system parameter to print    */
			sprintf(pkick,PRINT,tmpf);
			wrc = system(pkick);
			if(wrc != 0)
			{
				msgid = U_BCMSGN;
				/* Printing Error occurred.  */
				continue;
			};
/*
			sprintf(pkick,"sh -c '(/bin/rm %s)'",tmpf);
			system(pkick);
*/
		}
		else
		{
		/* case file output */
		/* get output file name     */
			col  = filex + udcbptr->xoff;
			line = menuy[2] + udcbptr->yoff;
			dtlen = 16;
			ipmode = T_FILE;
			msgid = U_ENDID;

			while(TRUE)
			{ /* loop of file out. (endloop mark is '####') */

				(void)humsg(udcbptr,msg_y,msg_x,msgid);
				msgid = U_ENDID;
				while(TRUE)
				{
					triger = huipfld(udcbptr,line,col,
					outfname, dtlen,ipmode,&uplen,C_SWON, HUTABLE);

					if( (triger == U_C_DOWN) ||
						(triger == U_C_UP) ||
						(triger == U_TABKEY) ||
						(triger == U_BTABKEY) ||
						(triger == U_ENTERKEY) ||
						(triger == U_ACTIONKEY) ||
						(triger == U_CRKEY) ||
						(triger == U_PF3KEY) )
					{
						break;
					};
				};

			/* cursor move    */
				if( (triger == U_C_UP   ) ||
					(triger == U_BTABKEY)   )
				{
					menun--;
					break;
				};
				if( (triger == U_C_DOWN ) ||
					(triger == U_TABKEY )   )
				{
					menun++;
					if(menun > 2) menun = 0;
					break;
				};

			/* end process */
				if( triger == U_PF3KEY )
				{
					break;
				};

			/* check input data length        */
				if(uplen <= 0)
				{
					msgid = U_ARMSGN;
					/* Please input file name.    */
					continue;
				};

			/* open outfile   */
				outfname[uplen] = NULL;

			/* check input name       */
				if(hufnc(outfname) != IUSUCC)
				{
					msgid = U_BFMSGN;
					/* this file name is invalid.    */
					continue;
				};

				fd = open(outfname,O_WRONLY);
				if(fd < 0)
				{
				/* case open error      */
					if(errno == ENOENT)
					{
					/* file not exist      */
					fd=creat(outfname,(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));
					}
					else
					{
					/* file is exist, but permision error */
					msgid = U_BBMSGN;
					/* File access permission denied */
					continue;
					};
				}
				else
				{
				/* case open success	*/
					msgid = U_ASMSGN;
					/* File name already exists. Enter = Do  F12 = Cancel */
					(void)humsg(udcbptr,msg_y,msg_x,msgid);
					msgid = U_ENDID;

				/* check over write	*/
					while(TRUE)
					{
						triger = hugetc();
						if( (triger == U_RESETKEY ) ||
							(triger == U_PF12KEY  ) ||
							(triger == U_ACTIONKEY) ||
							(triger == U_CRKEY    ) ||
							(triger == U_ENTERKEY ) )
						{
							break;
						};
					};
					if( (triger == U_RESETKEY ) ||
						(triger == U_PF12KEY  ) )
					{
					/* cancel output	*/
						continue;
					}
					else
					{
					/* over write OK!	*/
					/* truncate  file	*/
						rc = ftruncate(fd,(unsigned long)(0));
					};
				};

			/* output to temp file      */
				rc = hufout(fd,topptr);
				close(fd);
			/* set msg  */
				if(rc != IUSUCC)
				{
					msgid = U_BBMSGN;
					/* File access permission denied */
					continue;
				}
				else
				{
					msgid = U_AZMSGN;
					/* Finished writing user dictionary to the above file */
				};
				break;
			};  /* #### endloop */

		/* end process */
			if( triger == U_PF3KEY )
			{
				break;
			};

		}; /* ++++ endif */

	}; /* %%%% endloop */

	/*
	*      free display map area and dictionary area.
	*/

	/* free display map.    */
	free( (char *)(dmappt->fld) );
	free( (char *)dmappt );

        /* free dictionary buffer.      */
         while(TRUE)  {
           curptr = lastptr->pr_pos;
           free( (char *)lastptr );
           if( curptr == NULL ) break;
           lastptr = curptr;
         }

	/* return.      */
	return( rc );
};
