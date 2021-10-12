static char sccsid[] = "@(#)05	1.1  src/bos/usr/lpp/kls/dictutil/hudicdlp.c, cmdkr, bos411, 9428A410j 5/25/92 14:43:28";
/*
 * COMPONENT_NAME :	(KRDU) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicdlp.c
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
 *  Module:       hudicdlp.c
 *
 *  Description:  User Dictionary Delete Process
 *
 *  Functions:    hudicdlp()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ************************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package.   			*/
#include <memory.h>     /*                         			*/
#include "hut.h"        /* user dictionary utility 			*/

/*----------------------------------------------------------------------*/
/*                      Begining of hudicdlp.                           */
/*	Delete a key's position candidate of Data Block.		*/
/*----------------------------------------------------------------------*/
void    hudicdlp ( mode, keydata, keylen, pos, udcbptr )
short    mode;          /* Delete Mode                              	*/
uchar   *keydata;       /* Key  Data                                	*/
short    keylen;        /* Key  Data Length                         	*/
uchar    pos;           /* Candidate Position                       	*/
UDCB    *udcbptr;       /* Pointer to User Dictionary Control Block 	*/
{
    void    humvch();               /* Character Move           Function     */
    void    hudeixsc();             /* Dictionary Index Search  Function     */
    void    hudedtsc();             /* Dictionary Data  Search  Function     */
    void    huderepl();             /* Dictionary Index Replace Function     */
    void    hudcread();             /* Dictionary Data Read     Function     */
    void    hudcwrit();             /* Dictionary Data Write    Function     */

    uchar   dicindex[U_UIX_A],     /* Dictionary Index Save Area             */
            dicdata[U_REC_L],       /* Dictionary Data Save Area             */
            clrstr[U_REC_L],        /* Clear String Data Area                */
            dummydata,              /* Candidate Data ( Dummy )              */
            *l_canddata,            /* Pointer to Candidate Data ( Dummy )   */
            l_keydata[U_KEY_MX];    /* Last Entry Flag (ON==Last or OFF)     */
    short   il,                     /* Active Index Area's Size.             */
	    /*
	     * Modification Comments
	     * src
	     *		lastflg
	     * our 	
	     *		firstflg
	     *		Our Index Block Key is the first Key of Data BLock.
	     */
	    /* (1) { */
	    modify_pos,		    /* Last candidate modificaition pos.     */
	    firstfg,		    /* flag, which is the first data of      
							Data Block.	     */ 
	    /* (1) } */
            nar,                    /* Next Available RRN.                   */
            wk_rrn,                 /* Relative Record Number.               */
            indxpos,                /* Index Entry Position                  */
            indxpos1,               /* Index Entry Position (Previous)       */
            rl,                     /* Active Data's Size.                   */
            dl,                     /* Data Length                           */
	    i_keylen,
	    dummylen,		    /* dummy data length 		     */
	    d_keylen,		    /* Data Block key length.		     */
	    d_cbsz,		    /* Data Block a key's all candidates 
							length.		     */
	    d_candlen,		    /* Data Block a key's candidate length.  */
            l_candlen,              /* Candidate Data Length                 */
	    t_candlen,
            datapos,                /* Candidate Entry Position              */
            datapos1,               /* Candidate Entry Position (Previous)   */
	    i_rrn,		    /* Index Block RRN.			     */
	    lastcflag,		    /* Flag, which is the last candidate.    */
            del_f;                  /* Candidate Entry Delete Flag           */
    int     i,                      /* Loop Counter.                         */
            dlen,                   /* Data Delete Length                    */
            dpos,                   /* Data Delete Position                  */
            ddst;                   /* Data Delete Distance                  */

    /* (1) */
    /***************/
    /*		   */
    /* Initialize. */
    /*		   */
    /***************/

    memset(clrstr, 0xff, U_REC_L);

    /* (2) */
    /****************************************/
    /*					    */
    /* Dictinary Index & Data Read Process. */
    /*					    */
    /****************************************/

    hudcread ( udcbptr, 3, NULL );            /* Read Dictionary Index       	*/
    memcpy   ( dicindex, udcbptr->rdptr, U_UIX_A );
    il = getil(dicindex);
    nar = getnar(dicindex);

    hudeixsc( dicindex, keydata, keylen, &indxpos, &indxpos1 );

    i_keylen = nxtkeylen(dicindex, indxpos, U_UIX_A); 
    (void)getrrn(dicindex+indxpos+i_keylen, &i_rrn);

    hudcread ( udcbptr, 4, i_rrn );             /* Read Target RRN Data        	*/
    memcpy   ( dicdata, udcbptr->rdptr, U_REC_L );
    rl = getrl(dicdata);

    dummydata = NULL;                            /* Dummy Data                  */
    dummylen = 1;                                /* Dummy Data                  */
    hudedtsc( dicdata, keydata, keylen, 
	&dummydata, dummylen, &datapos, &datapos1 , &firstfg, &t_candlen);

    /* (3) */
    /**************************************/
    /*					  */
    /* KeyData  or Candidate Data Delete. */
    /*					  */
    /**************************************/

    d_keylen = nxtkeylen(dicdata, datapos1, U_REC_L);
    d_cbsz = nxtkoffset(dicdata, datapos1+d_keylen, U_REC_L);
    del_f = TRUE;                             /* Set Delete Flag             	*/

    if ( mode != U_S_KEYD ) {
        del_f = FALSE;                        /* Reset Delete Flag           	*/
        dpos = datapos1 + d_keylen ;      /* Initial Delete Position Set 	*/
        for ( i = 1 ; i <= pos ; i++ ) {
	    d_candlen = nxtcandlen(dicdata, dpos, &lastcflag, U_REC_L);
	    dpos += d_candlen;
	    if (i == pos)
                {
		ddst = d_candlen;
		break;
                }
        }

        /* Set MRU Delete Candidate Data   */
        l_canddata   = &dicdata[dpos - ddst];
        l_candlen  = ddst;

        /* MRU Data Delete             */
        hudcmrud ( mode, keydata, keylen, l_canddata, l_candlen, udcbptr );

	/*********************************************************/
	/*							 */
	/* If there is two candidates and one candidate will be  */
 	/* deleted, one candidate will be last candidate.        */
	/*							 */
	/*********************************************************/
	if (pos != 1 && lastcflag == U_FON) {
	   modify_pos = dpos-ddst-2;
	   *(dicdata+modify_pos+0) |= 0x80;
	}

        /* Set Delete Length */
        dlen = rl - dpos; 
        /* Set Delete Flag */
        if ( pos == 1  &&  dpos == datapos )  
        {
	   del_f = TRUE;
	}
    } 

    /* (4) */
    /****************************/
    /*                          */
    /* Dictionary Data  Update. */
    /*                          */
    /****************************/

    if ( del_f == TRUE )
        {                           /* 1 Entry Delete Postion & Length Set   */
        dpos = datapos;                       /* Set Delete Position         */
        dlen = rl - datapos;                  /* Set Delete Length           */
        ddst = d_keylen + d_cbsz;             /* Set Delete Distance         */
        }
    if ( rl == datapos  &&  del_f == TRUE )
	/***************************************/
	/*				       */
	/* Target Data Delete ( Data is Last ) */
	/*				       */
	/***************************************/
	memset(dicdata+datapos1, 0xff, U_REC_L-datapos1);
    else {                         
	/***************************************/
	/*				       */
	/* Target Data Delete ( Entry or Data )*/
	/*				       */
	/***************************************/
        humvch ( dicdata, dpos, dlen, U_FORWD, ddst, TRUE, clrstr, clrstr[0], ddst );
	if (dlen == 0 && dpos == rl) {
	   memset (dicdata+(dpos-ddst), 0xff, ddst);
	}
    }
    rl -= ddst;                               /* Update Active Data Size     */
    setrl(dicdata, rl);                       /* Set Active Data Size        */

    /* (5) */
    /****************************/
    /*				*/
    /* Dictionary Index Update. */
    /*				*/
    /****************************/

    if ( del_f == TRUE )
        if ( rl == U_RLLEN )
           {                                
	   					/* Get Key  Data Length     	*/
	   i_keylen = nxtkeylen(dicindex, indxpos, U_UIX_A);
           dpos = indxpos + i_keylen + U_RRNLEN; /* Set Delete Position      	*/
           dlen = il - dpos;                     /* Set Delete Length        	*/
           ddst = i_keylen + U_RRNLEN;                /* Set Delete Distans     */
           if ( dpos == il )                /* Dictionary Index Entry Delete 	*/
	       memset(dicindex+indxpos, 0xff, (il-indxpos));
           else
               humvch ( dicindex, dpos, dlen, U_FORWD, ddst,
                        TRUE, clrstr, clrstr[0], ddst );

           il -= ddst;                      /* Update Active Index Area Size 	*/
	   setil(dicindex, il);		    /* Set Active Index Area Size    	*/
           nar--;                           /* Update Next Available RRN     	*/
                                            /* Set Next Available RRN        	*/
	   setnar(dicindex, nar);
           if ( nar !=  i_rrn )
              {                             /* Set Index First Entry Position	*/
	      i = indxpos ;
              for ( ; i < il; i += i_keylen + U_RRNLEN )
                  {
						 /* Get Key  Data Length     	*/
		  i_keylen = nxtkeylen(dicindex, i, U_UIX_A);
						 /* Get Record Number        	*/
		  (void)getrrn(dicindex+i+i_keylen, &wk_rrn);
		  setrrn(dicindex+i+i_keylen, wk_rrn-1);
				 /* Set New Record Number    	*/

		  hudcread(udcbptr, 4, wk_rrn) ;
                  memcpy(dicdata, udcbptr->rdptr, U_REC_L) ;
                  udcbptr->wtptr = &dicdata[0];
                  hudcwrit( udcbptr, 4, wk_rrn-1);
                  }
              }
           memcpy(dicdata, clrstr, U_REC_L) ;
           udcbptr->wtptr = &dicdata[0];
           hudcwrit( udcbptr, 4, nar);
           udcbptr->wtptr = dicindex;         /* Index Data Write               	*/
           hudcwrit ( udcbptr, 3, NULL );
           return;
           }
        else
           {
                                    /* Compare of Input Key  & index Key     	*/
	   /* Local Copy */
	   memcpy(l_keydata, keydata, keylen);
	   mkrbnk(l_keydata, keylen);
           if ( memcmp( &dicindex[indxpos], l_keydata, keylen ) == NULL )
              {
              dummydata = NULL;        /* Dummy Data                            */
              dummylen  = 1;           /* Dummy Data                            */
                                       /* Search Index Entry Key  Data          */
              hudedtsc( dicdata, keydata, keylen,
                        &dummydata, dummylen, &datapos, &datapos1, &firstfg, &t_candlen );
                                       /* Index Entry Replace                   */
              huderepl( dicindex, indxpos,  
		 (short)nxtkeylen(dicindex,indxpos, U_UIX_A),
                 &dicdata[datapos1], 
		 (short)nxtkeylen(dicdata,datapos1, U_REC_L) );
              }
           }

	/* (6) */
	/**************************/
	/*			  */
        /* Dictionary Data Write. */
	/*			  */
	/**************************/

        udcbptr->wtptr = dicdata;          /* Dictionary Data Write          	*/
        hudcwrit ( udcbptr, 4, i_rrn );

	/* (7) */
	/***************************/
	/*			   */
        /* Dictionary index write. */
	/*			   */
	/***************************/

        udcbptr->wtptr = dicindex;         /* Index Data Write               	*/
        hudcwrit ( udcbptr, 3, NULL );

        return;
}
/*----------------------------------------------------------------------*/
/*                      End of hudicdlp.                                */
/*----------------------------------------------------------------------*/
