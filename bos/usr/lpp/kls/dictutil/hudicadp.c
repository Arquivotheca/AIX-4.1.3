static char sccsid[] = "@(#)00  1.1  src/bos/usr/lpp/kls/dictutil/hudicadp.c, cmdkr, bos411, 9428A410j 5/25/92 14:42:38";
/*
 * COMPONENT_NAME :	(KRDU) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicadp.c
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

/************************************************************************
 *
 *  Component:    Korean IM User Dictionary Utility
 *
 *  Module:       hudicadp.c
 *
 *  Description:  Addition Process of a key and candidate.
 *
 *  Functions:    hudicadp() 
 *
 *  History:      5/22/90  Initial Creation.
 *
 ************************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>       /* Standard I/O Package.                        */
#include <memory.h>      /* System memory operation uty.                 */
#include "hut.h"         /* Utility Define File                          */ 
#define  I_FOF	0
#define  I_RIGHT 1

/*----------------------------------------------------------------------*/
/*                      Begining of hudicadp.                           */
/*----------------------------------------------------------------------*/
int  hudicadp ( mode, keydata, keylen, canddata, candlen, udcbptr )
short   mode;            /* Request Mode (Inquiry or Registration)       */
uchar   *keydata;        /* Pointer to Key  ( 7bit Code ) String         */
short   keylen;          /* Length of Key                                */
uchar   *canddata;       /* Pointer to Candidate String Data             */
short   candlen;         /* Length of Candidate String Data              */
UDCB    *udcbptr;        /* Pointer to User Dictionary Control Block     */

{
   int     humvch(),         /* Character Move Function                      */
           hudedtsc();       /* User Dictionary Data Search   Function       */
   void    hudeadpr(),       /* User Dictionary Data Add      Function       */
           huderepl(),       /* User Dictionary Index Replace Function       */
           hudcread(),       /* User Dictionary File Read     Function       */
           hudcwrit();       /* User Dictionary File Write    Function       */

   uchar   dicdata[U_REC_L+U_REC_L],     /* User Dictionary Read Area.       */
           dicdata1[U_REC_L],            /* User Dictionary Read Area.       */
           dicdata2[U_REC_L],            /* User Dictionary Read Area.       */
           dicdata3[U_REC_L],            /* User Dictionary Read Area.       */
           dicindex[U_UIX_A],            /* User Dictionary Read Area.       */
           buff[U_REC_L];                         /* Work Character Area.             */
   uchar   dataarea[U_REC_L],       /* Insert Data Area.                     */
           clrstr[U_REC_L];         /* Clear Data Area.                      */
   int     rc;
   short   indexlen,
	   i_len,
	   nar,
 	   har,
           rrn,                     /* Work rrn Value.                       */
           d_len,                      /* Work d_len  Value.                 */
	   recrrn[5],
	   indxpos,
	   indxpos1,
	   lastfg,
	   firstfg, 
	   datapos,
 	   datapos1,
	   inspos,
	   inslen,
	   i_keylen,
	   n_keylen,
	   updtfg,
	   d_keylen,
	   d_cbsz,
	   p_keylen,
	   p_cbsz,
	   shiftlen,
	   rl1, 
	   modify_pos,
	   last_keylen,
	   first_keylen,
	   writfg[5],
           wklen,                   /* Work Key  Length Move Entry.          */
	   insertfg,
	   t_candlen ;
	   
   uchar   l_keydata [U_KEY_MX],
	   last_keydata [U_KEY_MX],
	   first_keydata [U_KEY_MX],
	   data [U_KEY_MX],
	   l_canddata[U_CAN_MX];
   
register   int     i,               /* Work Integer Variable 1.              */
           j,                       /* Work Integer Variable 2.              */
           k;                       /* Work Integer Variable 3.              */

                                    /* Input Parameter Check & Initialize    */
   if ( keylen > U_KEY_MX )  return( UDIVPARE );
   if ( candlen > U_CAN_MX )  return( UDIVPARE );

   insertfg = I_FOF;

   /*********/
   /*	    */
   /* Init. */
   /*	    */
   /*********/
   for ( i=0; i<5; i++ ) writfg[i] = U_FOF;   /* Reset Write Flag            */
   memset ( clrstr,   0xff, U_REC_L );        /* Claer Data Area( NULL Set ) */
   memset ( dicdata,  0xff, U_REC_L+U_REC_L );/* I/O Area Clear ( NULL Set ) */
   memset ( dicdata1, 0xff, U_REC_L );        /* I/O Area Clear ( NULL Set ) */
   memset ( dicdata2, 0xff, U_REC_L );        /* I/O Area Clear ( NULL Set ) */
   memset ( dicdata3, 0xff, U_REC_L );        /* I/O Area Clear ( NULL Set ) */
   memset ( dicindex, 0xff, U_UIX_A );        /* I/O Area Clear ( NULL Set ) */

   /****************/
   /* Local Copy   */
   /* Data Convert */
   /****************/
   memcpy(l_keydata, keydata, (int)keylen);
   mkrbnk(l_keydata, keylen);
   memcpy(l_canddata, canddata, (int)candlen);
   mkrbnlastc(l_canddata, candlen);

   /************************/
   /*			   */
   /* Gets the Index Block */
   /*			   */
   /************************/
   hudcread( udcbptr, 3, 0 );     
   memcpy ( dicindex, udcbptr->rdptr, U_UIX_A );

   har = gethar(dicindex);

   if (U_HAR_V1 <= har && U_HAR_V2 >= har) {
       /*******************/
       /* Get Index Block */
       /*******************/
       indexlen = 1;
   } else {
       return (UDIVHARE);
   } 

   nar = getnar(dicindex);

   /* (1) */
   /******************************/
   /*				 */
   /*  Data is Empty Process     */
   /*				 */
   /******************************/
   /* U_BASNAR , 2KBytes */
   if ( nar == U_BASNAR )
     {
 	/* (1).1 */
	/**********************/
	/*		      */
	/* Update Index Block */
	/*		      */
	/**********************/
        nar++;                                
	setnar(dicindex, nar);
	i = (U_ILLEN+U_HARLEN+U_NARLEN);
        memcpy ( dicindex+i, l_keydata, (int)keylen );
	setrrn(dicindex+i+keylen, U_BASNAR ); 
        i_len = (U_ILLEN + U_HARLEN + U_NARLEN + keylen + U_RRNLEN);
	setil(dicindex, i_len);
        udcbptr->wtptr = dicindex;          /* Control Block Data Set */
        hudcwrit ( udcbptr, 3, NULL );      /* Index Data Write */

 	/* (1).2 */
	/*********************/
	/*		     */
	/* Update Data Block */
	/*		     */
	/*********************/
        d_len  = U_RLLEN + keylen + candlen;/* Active Data Size Calc       */
	setrl(dicdata, d_len);              /* Set Active Data Size        */
        memcpy(dicdata+U_RLLEN, l_keydata, (int)keylen);
        memcpy (dicdata+U_RLLEN+keylen, l_canddata, (int)candlen);
        recrrn[0] = (U_BASNAR);             /* Write Record Number Set     */
        udcbptr->wtptr = dicdata;           /* Control Block Data Set      */
        hudcwrit(udcbptr, 4, recrrn[0]);    /* Dictionary Data Write       */
        return(IUSUCC);                     /* Return Process Successful   */
     }
   
   /* (2) */
   /*********************/
   /*			*/
   /* Update Data Block */
   /*			*/
   /*********************/

   hudeixsc ( dicindex, keydata, keylen, &indxpos, &indxpos1 );
   i_keylen = nxtkeylen(dicindex, indxpos, U_UIX_A); 
   getrrn(dicindex+indxpos+i_keylen, (ushort*)&recrrn[0]);
   hudcread( udcbptr, 4, recrrn[0] );         
   memcpy ( dicdata, udcbptr->rdptr, U_REC_L ); 

   /* Dictionary Data Entry Search */
   rc = hudedtsc ( dicdata, keydata, keylen,
                   canddata, candlen, &datapos, &datapos1, &firstfg, &t_candlen );
   /* Insert Position Set */
   inspos = datapos; 

   switch ( rc ) 
      {
      case U_NOKEY :                
         inslen      = (keylen+candlen);
         memcpy( dataarea, l_keydata, (int)keylen );
	 mkrbnlastc(l_canddata, candlen);
         memcpy( dataarea+keylen, l_canddata, (int)candlen );
         break;
      case U_NOCAN :         
	/* 
	 * Modification comments:
	 */
	/* A Key's candidates total length must be less than 1020 */
	if ((t_candlen + candlen) > U_REC_L)
 	{
          return (UDCANDE);
	}

	/* 
	 * Modify Comments 
	 * src
 	 *	(1) code is inserted in our implementation.
 	 */
	/* (1) */
   	/*****************************/
   	/*                           */
   	/* Modify the last candidate */
   	/*                           */
   	/*****************************/
   	modify_pos = (inspos-2);
   	*(dicdata+modify_pos+0) &= 0x7f;

         inslen = candlen;  
	 mkrbnlastc(l_canddata, candlen);
         memcpy (dataarea, l_canddata, (int)candlen );
         break;
      case U_KEYCAN :            
         return ( UDDCEXTE );   
      }

   d_len = getrl(dicdata);
   har = gethar(dicindex);
   nar = getnar(dicindex);

   if (   (mode == U_MODINQ)                  /* Mode is Inquiry             */
       && ( (har >= nar) || ( d_len + inslen <= U_REC_L) )  )
            return( IUSUCC );                 /* Data Regist is OK           */

   /* (2).1 */
   /***********************/
   /*			  */
   /* Insert Data Process */
   /*			  */
   /***********************/
   if ( d_len > datapos )
     {
        /* Insert Data Area Move */
        rc = humvch ( dicdata, (int)datapos, 
		(int)(d_len - datapos), U_BACKWD,
                (int)inslen, TRUE, clrstr, NULL, (int)inslen );
        /* Copy Insert Data */
        memcpy ( dicdata+datapos, dataarea, (int)inslen );
     }
   else
        /* Insert Data is Last */
        memcpy ( dicdata+d_len, dataarea, (int)inslen );

   d_len += inslen;                              /* Update Active Data Area     */
   setrl(dicdata, d_len);
   writfg[0] = U_FON;                         /* Write Flag On               */

   /* (2).2 */
   /************************/
   /*			   */
   /* INDEX Update Process */
   /*			   */
   /************************/
   if ( firstfg == U_FON )
     {
	memcpy(data, l_keydata, keylen);
	i_keylen = nxtkeylen(dicindex, indxpos, U_UIX_A);

        /* Replace Index Entry */

        huderepl( dicindex, indxpos, i_keylen, data, keylen );

        writfg[4] = U_FON;                    /* Write Flag On */
     }

   /*********************/
   /*			*/
   /* Overflow Proccess */
   /*			*/
   /*********************/
   if ( d_len > U_REC_L )
     {   
        updtfg = U_FOF;          
 	/************************/
	/*			*/
        /* Left Insert Proccess */
	/*			*/
 	/************************/
        if ( indxpos1 < indxpos )
          {
            shiftlen = 0;    /* Shift Length Clear */
            for ( i = U_RLLEN; i < d_len; i += d_keylen+d_cbsz )
              {
		d_keylen = nxtkeylen(dicdata, i, U_REC_L+U_REC_L);
		d_cbsz = nxtkoffset(dicdata, i+d_keylen, U_REC_L+U_REC_L);
                shiftlen += (d_keylen+d_cbsz); /* Update Shift Length */
                if ( (d_len - shiftlen) <  U_REC_L )
                  {
		    i += d_keylen+d_cbsz ;
		    d_keylen = nxtkeylen(dicdata, i , U_REC_L+U_REC_L);
                    /* Set Last Key Data Length   */
		    last_keylen = d_keylen;
		    memcpy(last_keydata, dicdata+i, (int)d_keylen);
                    break;
                  }
              }
	    i_keylen = nxtkeylen(dicindex, indxpos1, U_UIX_A);
	    getrrn(dicindex+indxpos1+i_keylen, (ushort*)&recrrn[1]);
            hudcread ( udcbptr, 4, recrrn[1] );
            memcpy ( dicdata1, udcbptr->rdptr, U_REC_L );
	    /*****************************/
	    /* Data Area Can Moved Check */
            /* Get Active Data Area Size */
	    /*****************************/
	    rl1 = getrl(dicdata1);
            if ( (rl1 + shiftlen) <= U_REC_L )
              { 
		/**************************/
		/* Move Data Area Size OK */
                /* Entry Move for Left    */
		/**************************/
                hudeadpr ( U_DADDLF, dicdata, dicdata1, shiftlen );
		i_keylen = nxtkeylen(dicindex, indxpos, U_UIX_A);
                huderepl ( dicindex, indxpos, i_keylen, 
		   last_keydata, last_keylen );
                writfg[1] = U_FON;     /* Write Flag On  */
                writfg[4] = U_FON;     /* Write Flag On  */
                updtfg = U_FON;        /* Update Flag On */
              }
          }

	i_keylen = nxtkeylen(dicindex, indxpos, U_UIX_A);
	i_len = getil(dicindex);

	/************************/
	/*			*/
        /* Right Insert Process */
	/*			*/
	/************************/
        if ( (indxpos + i_keylen + U_RRNLEN) < i_len && updtfg == U_FOF )
          {
	    short   org_keylen = i_keylen;

	    insertfg = I_RIGHT; 
            i = j = U_RLLEN;                  /* Initialize Move Position    */
            while ( i < d_len )
              {
		d_keylen = nxtkeylen(dicdata, i, U_REC_L+U_REC_L);
		d_cbsz = nxtkoffset(dicdata, i+d_keylen, U_REC_L+U_REC_L);
                if ( (i+d_keylen+d_cbsz) > U_REC_L )
                  {
		    /********************/
                    /* Set Shift Length */
		    /********************/
                    shiftlen = d_len - i;        
		    last_keylen = nxtkeylen(dicdata, j, U_REC_L+U_REC_L);
		    memcpy(last_keydata, dicdata+j, (int)last_keylen);
                    break;
                  }
                 j = i;                  /* Previous Position Set */
                 i += (d_keylen+d_cbsz); /* 1 Entry Length Add    */
              }
            /* After Data Read */
	    i_keylen = nxtkeylen(dicindex, indxpos, U_UIX_A);

            /* Next Entry Position Set */
	    i = (indxpos+i_keylen+U_RRNLEN);

	    i_keylen = nxtkeylen(dicindex, i, U_UIX_A);
	    getrrn(dicindex+i+i_keylen, (ushort*)&recrrn[2]);
            hudcread ( udcbptr, 4, recrrn[2] );
            memcpy ( dicdata2, udcbptr->rdptr, U_REC_L );
            /* Check Data Area Can be Moved          */
	    rl1 = getrl(dicdata2);
            if ( (rl1 + shiftlen) <= U_REC_L )
              { 
		/**************************/
		/* Move Data Area Size OK */
                /* Entry Move for Right   */
		/**************************/
                hudeadpr ( U_DADDRT, dicdata, dicdata2, shiftlen );
		i_keylen = nxtkeylen(dicindex, indxpos, U_UIX_A);
                /* Replace Index Entry */
                huderepl ( dicindex, indxpos, i_keylen, 
		   last_keydata, last_keylen );
                writfg[2] = U_FON;    /* Write Flag On */
                writfg[4] = U_FON;    /* Write Flag On */
                updtfg = U_FON;       /* Update Flag On*/
              } 
	    else
	      {
		i_keylen = org_keylen ;
	      }
          }

	/***********************/
	/*		       */
        /* Add New Data Record */
	/*		       */
	/***********************/
        if ( nar <= har && updtfg == U_FOF )
          {
	    d_len = getrl(dicdata);
            shiftlen = 0;            
	    p_keylen = 0;
	    p_cbsz = 0;
            for (i = U_RLLEN ; i < d_len; i += d_keylen + d_cbsz )
              {
		d_keylen = nxtkeylen(dicdata, i, U_REC_L+U_REC_L);
		d_cbsz = nxtkoffset(dicdata, i+d_keylen, U_REC_L+U_REC_L);
                /* Update Shift Data Length */
                shiftlen += (d_keylen+d_cbsz);         
                if ( shiftlen > ( d_len * U_SPRF / U_SPRD ) )
                  {
		    if (shiftlen >= (d_len-U_RLLEN))
		    {
			shiftlen -= (d_keylen+d_cbsz);
			i -= (p_keylen+p_cbsz);
		        last_keylen = d_keylen;
			memcpy(last_keydata, dicdata+i+p_keylen+p_cbsz, last_keylen);
			break;
		    }
		    last_keylen = nxtkeylen(dicdata, i+d_keylen+d_cbsz, U_REC_L+U_REC_L);
		    memcpy(last_keydata, dicdata+i+d_keylen+d_cbsz, last_keylen);
                    break;
                  }
		  p_keylen = d_keylen;
		  p_cbsz = d_cbsz;
              }
            /**********************/
	    /*			  */
            /* Entry Move for New */
	    /*			  */
            /**********************/
            hudeadpr ( U_DADDNW, dicdata, dicdata3, shiftlen );
	    setrrn(last_keydata+last_keylen, recrrn[0]+1);
            last_keylen += U_RRNLEN;  
            /* Insert Index Data */
	    i_len = getil(dicindex);

            if ( i_len > indxpos )
              { 
		/* next key length */
		n_keylen = nxtkeylen(dicindex, indxpos+i_keylen+U_RRNLEN, U_UIX_A);
		/* Modification */
	        if (n_keylen > 0)
	     	  {
			short  nxt_keylen , i, n;

			i = indxpos+i_keylen+U_RRNLEN ;
			nxt_keylen = n_keylen ;
			n=recrrn[0]+2 ;
	   		while(nxt_keylen > 0)
	     		{
		    	   setrrn(dicindex+i+nxt_keylen, n);
		    	   i += nxt_keylen+U_RRNLEN ;
		    	   nxt_keylen = nxtkeylen(dicindex, i, U_UIX_A);
		    	   n++;
			}
	           }
	
	        /* Move Char Data Area */
                rc = humvch(dicindex, (int)(indxpos+i_keylen+U_RRNLEN),
                        (int)(i_len - (indxpos+i_keylen+U_RRNLEN)),
                        U_BACKWD,
                        (int)last_keylen,
                        TRUE, clrstr, NULL, (int)last_keylen );
                /* Data Copy Insert Entry */
                memcpy (dicindex+(indxpos+i_keylen+U_RRNLEN), 
		   last_keydata, (int)last_keylen );

              } else {
                /* Insert Entry is Last */
                memcpy ( dicindex+i_len, last_keydata, (int)last_keylen );
	      }
	    /***if (insertfg == I_RIGHT)
                 recrrn[3] = recrrn[0];  ***/                  /* Write Record No Set         */
	  /**  else  **/
                 recrrn[3] = recrrn[0]+1;                  /* Write Record No Set         */
	      
            nar++;                            /* Update Next Available RRN   */
            /* Set Next Available RRN */
	    setnar(dicindex, nar);
            i_len += last_keylen;              /* Update Index Active Length  */
            /* Set Index Active Length */
	    setil(dicindex, i_len);
            writfg[3] = U_FON;                /* Write Flag On               */
            writfg[4] = U_FON;                /* Write Flag On               */
            updtfg = U_FON;                   /* Update Flag On              */
         }
                                    /* No Data Add Space                     */
       if ( updtfg == U_FOF )  return( UDDCFULE );
     }
                                    /* Write User Dictionary                 */
   if ( mode != U_MODINQ )
     { /* Mode is Registration        */
	if ((recrrn[0] < (nar -1)) && (insertfg == I_RIGHT))
	    {
		ushort i ;

		for (i=nar-2 ; i > recrrn[0] ; i--)
		{
		   hudcread(udcbptr, 4, i) ;
		   memcpy(buff, udcbptr->rdptr, U_REC_L) ;
           	   udcbptr->wtptr = &buff[0]; 
           	   hudcwrit( udcbptr, 4, i+1);
		}
	    }
       if ( writfg[1] == U_FON )
         {
            udcbptr->wtptr = &dicdata1[0];    /* Set Write Data Address      */
            /* User Dictionary Write       */
            hudcwrit ( udcbptr, 4, recrrn[1] );
         }
       if ( writfg[2] == U_FON )
         {
           udcbptr->wtptr = &dicdata2[0];     /* Set Write Data Address      */
           /* User Dictionary Write       */
           hudcwrit ( udcbptr, 4, recrrn[2] );
         }
       if ( writfg[3] == U_FON )
         {
           udcbptr->wtptr = &dicdata3[0];     /* Set Write Data Address      */
           /* User Dictionary Write       */
           hudcwrit ( udcbptr, 4, recrrn[3] );
         }
       if ( writfg[0] == U_FON )
         {
           udcbptr->wtptr = &dicdata[0];      /* Set Write Data Address      */
           /* User Dictionary Write       */
           hudcwrit ( udcbptr, 4, recrrn[0] );
         }
       if ( writfg[4] == U_FON )
         {
           udcbptr->wtptr = &dicindex[0];     /* Set Write Data Address      */
           /* User Dictionary Index Write */
           hudcwrit ( udcbptr, 3, NULL );
         }
     }
   return( IUSUCC );                          /* Process is Successful       */
}
/*----------------------------------------------------------------------*/
/*                      End of hudicadp.                                */
/*----------------------------------------------------------------------*/
