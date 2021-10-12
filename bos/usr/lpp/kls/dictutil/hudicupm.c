static char sccsid[] = "@(#)13	1.1  src/bos/usr/lpp/kls/dictutil/hudicupm.c, cmdkr, bos411, 9428A410j 5/25/92 14:45:04";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicupm.c
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
 *  Module:       hudicupm.c
 *
 *  Description:  display user dictionary buffer.
 *
 *  Functions:    hudicupm()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ******************************************************************/

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
#include <memory.h>     /* Memory package.                              */
#include "hut.h"        /* Dictionary Utility Define File.   */

int     hudicupm( udcb   , df     , dmode   , dmappt , fldno
                 , dbufpt , topptr , lastptr , start_y , max_line   )

UDCB    *udcb;          /* pointer to UDCB                              */
DIFLD   *df;            /* pointer to display field data area           */
short   dmode;          /* display mode                                 */
UDMS    *dmappt;        /* pointer to display map                       */
short   fldno;          /* field No. to be reversed                     */
UDCS    *dbufpt;        /* pointer to update buffer                     */
UDCS    *topptr;        /* pointer to top of update buffer              */
UDCS    *lastptr;       /* pointer to last of update buffer             */
short   start_y;         /* start Y axis of input field                  */
short   max_line;          /* maximum line number of input field           */

{
int     hubracket();      /* write bracket.                       */
int     hudisply();     /* display stringth.                    */

UDCS    *pthdd;         /* pointer to head of display data      */
UDCS    *pthdd1;        /* pointer to head of display data      */
UDCS    *rtopptr;       /* real pointer to top of update buffer */
uchar   *wkey;         /* pointer to key data                 */
uchar   wkeylen;       /* pointer to key len                  */

uchar    *data;          /* pointer to input data field          */

int     rc;             /* return code                          */
int     i;              /* work variable                        */
int     j;              /* work variable                        */
short   maxfld;         /* maximum field nimber                 */

short   col;            /* input field x axis                   */
short   line;           /* input field y axis                   */
short   dtlen;          /* length of input field                */
static uchar ydata[2][2] = { { 0xa1 , 0xe8 } , { 0xa1 , 0xe9 } };
                                        /* allow data.                  */
/*  0.
*      check !  input parameter.
*/

   if( (dmode != U_NEXTP) && (dmode != U_BEFORP) &&
	(dmode != U_RESET) && (dmode != U_REDRW ) &&
	(dmode != U_DISP ) && (dmode != U_REVER ) )
	return( IUFAIL );

/*
*  case(dmode == U_RESET )  reset            process
*  case(dmode == U_REDRW )  redraw            process
*  case(dmode == U_REVER )  reverse & redraw  process
*  case(dmode == U_NEXTP )  next page        process
*  case(dmode == U_BEFORP)  before page      process
*  case(dmode == U_DISP  )  dsplay           process
*/

   if( (dmode == U_REDRW) || (dmode == U_REVER) )
   {
	/*  1.
	*  redrw process.
	*/

	/*
	*  field redraw and display
	*/
	/* check !  redraw field.                                   */
	if( dmappt->fld[fldno].fstat == U_KEYF )
	{
	   /* case : redraw field is Key.                     */
	   /* set displa X_axis.                               */
	   col    = U_XKEY;
	   /* set data length.                                 */
	   dtlen  = U_KEY_MX;
	   /* set pointer to input field.                      */
	   data   = df[fldno].keyfld;
	} 
	else 
	{
	   /* case : redraw field is Cand. */
	   /* set displa X_axis. */
	   col      = U_XCAND;
	   /* set data length. */
	   dtlen  = U_CAN_MX;
	   /* set pointer to input field. */
	   data   = df[fldno].candfld;
	}

	/* set redraw data display field axis. */
	col = col + udcb->xoff;
	line = start_y + fldno;

	/* set bracket */
	rc = hubracket(line,col,dtlen);

	/* field redraw     */
	if(dmode == U_REVER)
	{
	   /* set reverse mode */
	   putp(enter_reverse_mode);
	}

	rc = hudisply(udcb,line,col,data,dtlen);
	if(dmode == U_REVER)
	{
	   /* reset revers mode */
	   putp(exit_attribute_mode);
	}
	fflush(stdout);
	return( IUSUCC );
   }


   if( dmode == U_RESET )
   {
   /*  2.
   *  reset process.
   */

	/* get pointer to UDCS of reset field.                      */
	pthdd = dmappt->fld[fldno].dbufpt;

	if( dmappt->fld[fldno].fstat == U_KEYF )
	{
	   /* case : reset field is Key.                      */
	   col   = U_XKEY;
	   dtlen = U_KEY_MX;
	   data  = df[fldno].keyfld;
	   (void)memcpy(data,pthdd->key,(int)(pthdd->keylen));
	   data[pthdd->keylen] = NULL;
	}
	else
	{
	   /* case : reset field is Cand.                      */
	   col    = U_XCAND;
	   dtlen  = U_CAN_MX;
	   data   = df[fldno].candfld;
	   (void)memcpy(data,pthdd->cand,(int)(pthdd->candlen));
	   data[pthdd->candlen] = NULL;
	}

	/* set reset field display axis.    */
	col = col + udcb->xoff;
	line = start_y + fldno;

	/* set bracket      */
	rc = hubracket(line,col,dtlen);
	/* field redrow     */
	rc = hudisply(udcb,line,col,data,dtlen);
	fflush(stdout);
	return( IUSUCC );
   }

   /* erase display area process. */
   CURSOR_MOVE(start_y,0);
   for(i=0;i<max_line;i++)
   {
	putp(clr_eol);
	putp(cursor_down);
   }

   /* set pointer to UDCS and check.                               */
   i = C_SWOFF;
   pthdd = dbufpt;

   while(TRUE)
   {
	if((pthdd->status == U_S_INIT)
	   || (pthdd->status == U_S_KEYA)
	   || (pthdd->status == U_S_CANU)    )
	{
	   i = C_SWON;
	   break;
	}
	if( pthdd->pr_pos == NULL )  break;
	pthdd = pthdd->pr_pos;
   }

   if( i == C_SWOFF )
   {
	pthdd = dbufpt;
	while(TRUE)
	{
	   if((pthdd->status == U_S_INIT)
	   || (pthdd->status == U_S_KEYA)
	   || (pthdd->status == U_S_CANU)    )
	   	break;

	   /* case : data nothing. */
	   if( pthdd->nx_pos == NULL )  return( UDNODTE );
	   pthdd = pthdd->nx_pos;
	}
   }

   /* get real pointer to top of UDCS.                             */
   rtopptr = topptr;
   while(TRUE)
   {
	if((rtopptr->status == U_S_INIT)
	|| (rtopptr->status == U_S_KEYA)
	|| (rtopptr->status == U_S_CANU))  break;
	rtopptr = rtopptr->nx_pos;
   }

   /* set maxfld. */
   maxfld = max_line - 1;

   if( dmode == U_NEXTP )
   {
   /*  3.
   *  next page process
   */
   	/*
   	*  search last field
   	*/
	for(i=maxfld;dmappt->fld[i].fstat==U_NODIS;i--);
	if(i <= 2)
	{
	   /* case : last to top. */
	   pthdd = rtopptr;
	}
	else
	{
	   /* case : next page. */
	   pthdd = dmappt->fld[i].dbufpt;
	}
   }

   if( dmode == U_BEFORP )
   {
   /*  4.
   *  before page process
   */
	if(dmappt->fld[0].fstat == U_NODIS)
	{
	   /* case : top to last.                                    */
	   pthdd = lastptr;
	}
	else if(dmappt->fld[0].dbufpt == rtopptr)
	{
	   /* case : top to bottom line.                             */
	   /* all display map to be no use.                          */
	   for(i=0;i<=maxfld;i++)
	   dmappt->fld[i].fstat = U_NODIS;
	   /*
	   * display key data and set display map
	   */

	   /* set befor of bottom line no.   */
	   fldno    = max_line - 2;

	   /* set bottom line field.                                 */
	   col      = U_XKEY + udcb->xoff;
	   line     = start_y + fldno;

	   /* set data,length,field No.                              */
	   dtlen  = U_KEY_MX;
	   data   = df[fldno].keyfld;

	   /* set display map.                                       */
	   /* set pointer to UDCS.                                   */
	   dmappt->fld[fldno].dbufpt = rtopptr;
	   /* set status code.                                       */
	   dmappt->fld[fldno].fstat  = U_KEYF;

	   /* display Key field.                                    */
	   /* set init stringth.                                     */
	   (void)memcpy(data,rtopptr->key,(int)(rtopptr->keylen));
	   data[pthdd->keylen] = NULL;
	   rc = hubracket(line,col,dtlen);
	   rc = hudisply(udcb,line,col,data,dtlen);

	   /*
	   * display goki data and set display map
	   */

	   /* set fldno      */
	   fldno++ ;
	   /* set Cand field X_axis.                                 */
	   col    = U_XCAND + udcb->xoff;
	   line++;

	   /* set data,length,field No.                              */
	   dtlen  = U_CAN_MX;
	   data   = df[fldno].candfld;

	   /* set display map.                                       */
	   /* set pointer to UDCS.                                   */
	   dmappt->fld[fldno].dbufpt = rtopptr;
	   /* set status code.                                       */
	   dmappt->fld[fldno].fstat  = U_CANDF;

	   /* display Cand data.                                     */
	   /* set init stringth.                                     */
	   (void)memcpy(data,rtopptr->cand,(int)(rtopptr->candlen));
	   data[pthdd->candlen] = NULL;
	   /* set conversion flag.                                   */
	   rc = hubracket(line,col,dtlen);
	   rc = hudisply(udcb,line,col,data,dtlen);

	   /* write allow sighin.                                    */
	   /* set current key data.                                 */
	   wkey    = rtopptr->key;
	   wkeylen = rtopptr->keylen;
	   pthdd1   = rtopptr;

	   /* check !  need display allow data.*/
	   while(TRUE)
	   {
		pthdd1 = pthdd1->nx_pos;
		if((pthdd1->status == U_S_INIT)
		|| (pthdd1->status == U_S_KEYA)
		|| (pthdd1->status == U_S_CANU) )
		{
		   /* case : data exist.*/
		   if(  (pthdd1->keylen == wkeylen)
		   && (memcmp(pthdd1->key,wkey,wkeylen) == 0) )
		   {
		   	/* case : need display allow data.    */
		   	dmappt->poststat = C_SWON;
		   	col   = U_XARR + udcb->xoff;
		   	dtlen = U_ARRLEN;
		   	rc = hudisply(udcb,line,col,ydata[1],dtlen);
		   }
		   break;
		}
		else
		{
		   if(pthdd1->nx_pos == NULL)  break;
		   pthdd1 = pthdd1->nx_pos;
		}
	   } /* loop end for while  */

	   col = U_XCAND + udcb->xoff + U_CAN_MX;
	   CURSOR_MOVE(line,col);
	   return( IUSUCC );
	}
	else
	{
	   pthdd = dmappt->fld[0].dbufpt;
	}
	/*
	*  search and set pointer to head of display data
	*/
	i = 0;
	wkey = NULL;
	wkeylen = 0;
	pthdd1 = pthdd;
	while(TRUE)
	{
	   if( pthdd1 == NULL )  break;
	   if(    (pthdd1->status == U_S_INIT)
	   || (pthdd1->status == U_S_KEYA)
	   || (pthdd1->status == U_S_CANU))
	   {
		if((wkeylen == pthdd1->keylen )
		&& (memcmp(pthdd1->key,wkey,(int)(wkeylen)) == 0))
		{
		   i++;
		}
		else
		{
		   i += 2;
		   wkey = pthdd1->key;
		   wkeylen = pthdd1->keylen;
		}

		if(i > max_line)
		{
		   break;
		}
		else
		{
		   pthdd = pthdd1;
		}
	   }
	   pthdd1 = pthdd1->pr_pos;
	}
   }


   /*  5
   *   display process
   */

   /*
   *      pre data check
   */

   /* initialize previos flag.*/
   dmappt->prestat = C_SWOFF;

   pthdd1 = pthdd;

   while(TRUE)
   {
	if( pthdd1->pr_pos == NULL )  break;
	/* set pointer to UDCS of next data.*/
	pthdd1 = pthdd1->pr_pos;
	if((pthdd1->status == U_S_INIT)
	|| (pthdd1->status == U_S_KEYA)
	|| (pthdd1->status == U_S_CANU)    )
	{
	   if( (pthdd->keylen == pthdd1->keylen )
	   && (memcmp(pthdd->key,pthdd1->key,(int)(pthdd->keylen)) == 0) )
	   {
		/* case : current Key match previos Key.      */
		/* set previos flag.                            */
		dmappt->prestat = C_SWON;
		/* set axis of display allow.                   */
		col     = U_XARR + udcb->xoff;
		line    = start_y + 1;
		dtlen   = U_ARRLEN;
	
		/* display allow data.                          */
		rc = hudisply(udcb,line,col,ydata[0],dtlen);
	   }
	   break;
	}
   }

   /*
   *      first line display
   */

   /*
   *      init keygana
   */
   wkey    = NULL;
   wkeylen = 0;

   /*
   *      display key data and set display map
   */

   /* set poststat.        */
   dmappt->poststat = C_SWOFF;

   /* init field & line counter    */
   fldno = -1;     /* filed no     */
   while(TRUE)
   {
 	if( (pthdd == NULL) || (fldno >= maxfld) )  break;
	if((pthdd->status == U_S_INIT)
	|| (pthdd->status == U_S_KEYA)
	|| (pthdd->status == U_S_CANU)    )
	{
	   /*
	   *      display key data and set display map
	   */

	   /* set display field No.  */
	   fldno ++;

	   /* set display Y_axis.    */
	   line = start_y + fldno;
	   if((pthdd->keylen != wkeylen )
	   || (memcmp(pthdd->key,wkey,wkeylen) != 0)  )
	   {
		/* case : display Key. */

		/* check display position   */
		if(fldno >= maxfld)
		{
		   dmappt->fld[fldno].fstat = U_NODIS;
		   dmappt->fld[fldno].dbufpt = NULL;
		   break;
		};

		/* keep current Key data.  */
		wkeylen = pthdd->keylen;
		wkey    = pthdd->key;

		/* set display Key X_axis. */
		col    = U_XKEY + udcb->xoff;
		/* set display data length. */
		dtlen  = U_KEY_MX;
		/* set pointer to input field.      */
		data   = df[fldno].keyfld;
		/* set init stringth.       */
		(void)memcpy(data,pthdd->key,(int)(pthdd->keylen));
		data[pthdd->keylen] = NULL;
		/* set display map.         */
		/* set pointer to UDCS.     */
		dmappt->fld[fldno].dbufpt = pthdd;
		/* set status code. */
		dmappt->fld[fldno].fstat  = U_KEYF;
		/* display Key data.       */
		rc = hubracket(line,col,dtlen);
		rc = hudisply(udcb,line,col,data,dtlen);
		continue;
	   }
	   else
	   {
		/* case : display Key. */

		if( (pthdd->nx_pos != NULL) && (fldno == maxfld) )
		{
		   /*
		   *      in case of draw allow to down.
		   */
		   pthdd1 = pthdd;
		   while(TRUE)
		   {
			/* set pointer to next data.      */
			pthdd1 = pthdd1->nx_pos;
			if((pthdd1->status == U_S_INIT)
			|| (pthdd1->status == U_S_KEYA)
			|| (pthdd1->status == U_S_CANU))
			{
			   /* case : this data is varid.   */
			   if(  (pthdd1->keylen == wkeylen)
			   && (memcmp(pthdd1->key,wkey,wkeylen) == 0) )
			   {
				/* case : need display allow data.    */
				/* set post flag.     */
				dmappt->poststat=C_SWON;
				/* set allow data X_axis.     */
				col   = U_XARR + udcb->xoff;
				/* set allow length.  */
				dtlen = U_ARRLEN;
				/* display allow data.        */
				rc = hudisply(udcb,line,col,ydata[1],dtlen);
			   }
			   break;
			}
			else
			{
			   if(pthdd1->nx_pos == NULL)  break;
			   pthdd1 = pthdd1->nx_pos;
			}
		   } /* loop end for while    */
		}

		/*
		*      display candidate data and set display map
		*/

		/* set Cand X_axis.   */
		col    = U_XCAND + udcb->xoff;
		/* set display Cand length.   */
		dtlen  = U_CAN_MX;
		/* set pointer to input field.*/
		data   = df[fldno].candfld;
		/* set init stringth. */
		(void)memcpy( data , pthdd->cand , (int)(pthdd->candlen) );
		data[pthdd->candlen] = NULL;
		/* set display map.*/
	        /* set pointer to UDCS.*/
	        dmappt->fld[fldno].dbufpt = pthdd;
	        /* set status code.*/
	        dmappt->fld[fldno].fstat  = U_CANDF;
	        /* display Cand field. */
	        rc = hubracket(line,col,dtlen);
	        rc = hudisply(udcb,line,col,data,dtlen);
	    }
	}
	/* set pointer to next data.*/
	pthdd = pthdd->nx_pos;
   }

   /* set status code to no use input field.*/
   for(j=fldno+1;j<=maxfld;j++) {
   	dmappt->fld[j].fstat = U_NODIS;
   	dmappt->fld[j].dbufpt = NULL;
   }

   return( IUSUCC );
}
