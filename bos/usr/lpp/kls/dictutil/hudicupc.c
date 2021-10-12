static char sccsid[] = "@(#)11	1.1  src/bos/usr/lpp/kls/dictutil/hudicupc.c, cmdkr, bos411, 9428A410j 5/25/92 14:44:36";
/*
 * COMPONENT_NAME :	(CMDKR) - Korean Dictionary Utility
 *
 * FUNCTIONS :		hudicupc.c
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
 *  Module:       hudicupc.c
 *
 *  Description:  User Dictionary buffer update
 *
 *  Functions:    hudicupc()
 *
 *  History:      5/20/90  Initial Creation.
 *
 ************************************************************************/

/*----------------------------------------------------------------------*/
/*                      Include Header.                                 */
/*----------------------------------------------------------------------*/

#include <stdio.h>      /* Standard I/O Package.   			*/
#include "hut.h"        /* user dictionary utility 			*/

/*----------------------------------------------------------------------*/
/*                      Begining of hudicupc.                           */
/*----------------------------------------------------------------------*/
int    hudicupc ( dmappt, fldno, ipdata, iplen, topptr, lastptr, newptr )
UDMS    *dmappt;        /* Pointer to Display Map                            */
short    fldno;         /* Field No to be Updated                            */
uchar   *ipdata;        /* Pointer to Input Key  or Cand Data String         */
short    iplen;         /* Input Key  or Cand Data String Length             */
UDCS   **topptr;        /* Pointer to Pointer to Top  Dictionary Buffer      */
UDCS   **lastptr;       /* Pointer to Pointer to Last Dictionary Buffer      */
UDCS   **newptr;        /* Pointer to Pointer to                             */
                        /*  Additional New Dictionary Buffer                 */
{
    UDCS    *pt;            /* Pointer to Dictionary Buffer for Work         */
    UDCS    *w_pt;          /* Pointer to Dictionary Buffer for Alloc        */
    UDCS    *bgnpos;        /* Pointer to Dictionary Buffer for First        */
    UDCS    *endpos;        /* Pointer to Dictionary Buffer for Last         */
    UDCS    *inspos;        /* Pointer to Dictionary Buffer for Insert       */
    short   stat;           /* Field Status                                  */
    uchar   keydata[U_KEY_MX]; /* Transfer Before Key  Data Save Area        */
    uchar   l_keydata[U_KEY_MX]; /* Transfer Before 7bit Code Key  Data      */
    uchar   udcs_keydata[U_KEY_MX]; /* Transfer Before 7bit Code Key  Data WK*/
    uchar   space[U_CAN_MX]; /* Key  & Cand Area Space Data Padding Data     */
    short   keylen;           /* Transfer Before Key  Data length Save Area  */
    short   l_keylen;         /* Transfer Before 7bit Code Key  Data Length  */
    short   l_candlen;
    short   udcs_keylen;        /* Transfer Before 7bit Code Key  Data Length WK */
    short   mflag;          /* 7bit Code Key  Data Type Flag                 */
    int     cmplen;         /* Key  Data Compare Length                      */
    int     rc;             /* Memory Compare Return Code                    */
    int     i;              /* Counter Work Area                             */

    /*
     *   Initialize
     */
    memset(space, 0xff, U_CAN_MX);
    /*
     *   Get Data from Display Map
     */
    stat = dmappt->fld[fldno].fstat;    /* Get input field status       */
    pt   = dmappt->fld[fldno].dbufpt;   /* Get input buffer pointer     */
                                       /* Input Dictionary Buffer Data Save  */
    keylen = pt->keylen;                          /* Key  Length Save         */
    memcpy ( keydata, pt->key, (int)keylen );        /* Key  String Save         */
    l_keylen = keylen;
    memcpy ( l_keydata, keydata, (int)l_keylen);
   /*
    *   Cand Field Data Process
    */
    if ( stat == U_CANDF )
       {
                               /* Check Same Key  All Cand Length    */
       l_candlen = iplen;      /* Set Initial DL Length    */
       if ( pt->nx_pos != NULL )
          for ( w_pt = pt->nx_pos; w_pt != NULL; w_pt = w_pt->nx_pos )
             {                         /* Search for Same Key for Next      */
	     /* Local Copy */
	     udcs_keylen = w_pt->keylen;
  	     memcpy(udcs_keydata, w_pt->key, (int)udcs_keylen);
             if ( udcs_keylen != l_keylen  ||    /* Check Key  Data          */
                  memcmp ( udcs_keydata, l_keydata, (int)l_keylen ) != NULL )
                break;                 /* Difference Key  Search is End      */
             if ( w_pt->status != U_S_KEYD  &&   /* Check Status Code        */
                  w_pt->status != U_S_CAND  &&  w_pt->status != U_S_ADEL )
                l_candlen += w_pt->candlen ;  /* Increment Cand Length */
             }
       if ( pt->pr_pos != NULL )
          for ( w_pt = pt->pr_pos; w_pt != NULL; w_pt = w_pt->pr_pos )
             {                         /* Search for Same Key  for Previous  */
	     /* Local Copy */
	     udcs_keylen = w_pt->keylen;
	     memcpy(udcs_keydata, w_pt->key, (int)udcs_keylen);
             if ( udcs_keylen != l_keylen  ||          /* Check Key  Data    */
                  memcmp ( udcs_keydata, l_keydata, (int)l_keylen ) != NULL )
                break;                 /* Difference Key  Search is End      */ 
	     if ( w_pt->status != U_S_KEYD  &&   /* Check Status Code        */
                  w_pt->status != U_S_CAND  &&  w_pt->status != U_S_ADEL )
                l_candlen += w_pt->candlen ;  /* Increment Cand Length */
             }
       if ( l_candlen > U_CB_MX )
              return ( UDOVFDLE );               /* Error Entry RL Size Over */

       memcpy ( pt->cand, space, U_CAN_MX );     /* Cand Data Space Clear  */
       if ( pt->status != U_S_KEYA )
          pt->status = U_S_CANU;                 /* Set Status Code          */
       pt->candlen = iplen;                       /* Set Cand String Length   */
       memcpy ( pt->cand, ipdata, (int)iplen );   /* Set Cand String          */
       }
   /*
    *   Key  Field Data Process
    */
    else
       {
       bgnpos = NULL;                            /* Set NULL First  Pointer  */
       endpos = NULL;                            /* Set NULL Last   Pointer  */
       inspos = NULL;                            /* Set NULL Insert Pointer  */
       /* Local Copy */
       l_keylen = iplen;
       memcpy(l_keydata, ipdata, (int)l_keylen);

      /*
       *  Same Key  Search (Delete & Insert Position) All Buffer
       */
       for ( pt = *topptr; pt != NULL; pt = pt->nx_pos )
          {
          /* In the Buffer Pc Code string convert to 7bit Code               */
	  udcs_keylen = pt->keylen;
	  memcpy(udcs_keydata, pt->key, (int)udcs_keylen);

         /*
          *  Search for Insert Buffer Pointer
          */
          if ( inspos == NULL )
             {                                /* Compare Length Set          */
             cmplen = ( l_keylen < udcs_keylen ) ? l_keylen : udcs_keylen ;
                                              /* Key  Data Compare           */
             rc = memcmp ( l_keydata, udcs_keydata, (int)cmplen );
             if ( rc < NULL  || ( rc == NULL && iplen < pt->keylen ) )
                inspos = pt;                  /* Set Insert Buffer Pointer   */
             }
         /*
          *  Search for Same Key  Buffer
          */
          if ( pt->keylen == keylen               /* Check Key  Data          */
            && memcmp ( pt->key, keydata, (int)keylen ) == NULL )
             {
             if ( pt->status != U_S_KEYD  &&     /* Check Status Code        */
                  pt->status != U_S_CAND  &&   pt->status != U_S_ADEL  )
                {                                /* New Buffer Allocation    */
                if ( pt->kcaddflg == C_SWON ) /* Key Add flag When Changed   */
                   {
                     /* Set Old Status Key  Delete.                          */
                     if ( pt->status == U_S_KEYA )
                         pt->status   = U_S_ADEL;
                     else
                         pt->status   = U_S_KEYD;
                   }
                else
                   {
                   w_pt = ( UDCS * ) malloc ( sizeof ( UDCS ) );
                   if ( w_pt == NULL )
                      return ( UDCALOCE );       /* Allocation Error End     */
                                                 /* Old Buffer Data All Copy */
                   memcpy ( w_pt, pt, sizeof ( UDCS ) );
                   w_pt->keylen = iplen;        /* Set Input Key  Length    */
                                                 /* Key  Data Space Clear    */
                   memcpy ( w_pt->key, space, U_KEY_MX );
                                                 /* Set Input Key  Data      */
                   memcpy ( w_pt->key, ipdata, (int)iplen );
                   if ( pt->status == U_S_KEYA ) /* Set Old Status Key  Del  */
                      pt->status   = U_S_ADEL;
                   else
                      pt->status   = U_S_KEYD;

                   w_pt->status = U_S_KEYA;      /* Set New Status Key  Add  */
                                                 /* Set Buffer Chain Pointer */
                   if ( bgnpos == NULL )
                      {
                      bgnpos = w_pt;             /* Save First Pointer       */
                      endpos = w_pt;             /* Save Last  Pointer       */
                      }
                   else
                      {
                      w_pt->pr_pos = endpos;     /* Set Buffer Chain Pointer */
                      endpos->nx_pos = w_pt;     /* Set Buffer Chain Pointer */
                      endpos = w_pt;             /* Save Last Pointer        */
                      }
                   }
                }
             }
          }
      /*
       *  New Buffer Pointer Chain Set
       */
       if ( bgnpos == NULL )
          {                                   /* Case add data nothing.   */
	  *newptr = dmappt->fld[fldno].dbufpt;
          }
       else
          {
          if ( inspos == NULL )
             {                                /* Insert Position is Last  */
             bgnpos->pr_pos = *lastptr;       /* Set Buffer Chain Pointer */
             pt = *lastptr;                   /* Set Last Pointer         */
             pt->nx_pos = bgnpos;             /* Set Buffer Chain Pointer */
             endpos->nx_pos = NULL;           /* Set Last Chain is NULL   */
             *lastptr = endpos;               /* Update to Last Pointer   */
             }
          else if ( inspos == *topptr )
             {                                /* Insert Position is Last  */
             endpos->nx_pos = *topptr;        /* Set Buffer Chain Pointer */
             pt = *topptr;                    /* Set Top Pointer          */
             pt->pr_pos = endpos;             /* Set Buffer Chain Pointer */
             bgnpos->pr_pos = NULL;           /* Set First Chain is NULL  */
             *topptr = bgnpos;                /* Update to Top Pointer    */
             }
          else
             {                                /* Insert Position Halfway  */
             pt = inspos->pr_pos;             /* Set New Previous pt.     */
             bgnpos->pr_pos = inspos->pr_pos; /* Set New Previous pt.     */
             inspos->pr_pos = endpos;         /* Set Insert Previous pt.  */
             endpos->nx_pos = inspos;         /* Set New Next pt.         */
             pt->nx_pos = bgnpos;             /* Set Insert Next pt.      */
             }
          *newptr = bgnpos;                   /* Set Add New Pointer      */
          }
       }
    return( IUSUCC );                         /* Return Process Success   */
}
/*----------------------------------------------------------------------*/
/*                      End of hudicupc.                           	*/
/*----------------------------------------------------------------------*/
