static char sccsid[] = "@(#)49	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/shape.c, cfgnls, bos411, 9428A410j 8/30/93 15:03:47";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: BidiShape
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "csd.h"
#include <sys/lc_layout.h>
#include <sys/lc_layoutP.h>
int  BidiShape ( char *Buffer,
             size_t Length,
             int Onecell,
             int OS_flag,
             int orient,
             int csd) 
{
 char state=InitialState; 
                     /*this variable holds the state of the state machine */
 int           counter,  /* loop counter */
               i;
 char          *current,     /* to hold characters for shaping */
               *first_prec,
               *sec_prec,
               *third_prec,
               *next,
                blank=0x20,
                symbol=0x21;  /* ! */
/********************** start of processing ***********************************/

   switch (csd)
   {
     case TEXT_SHAPED : /* auto shaping */
       if (orient==ORIENTATION_LTR)  /* LTR */
         {
           /* loop on whole string to be shaped */
           for (counter=Length-1;counter>=0;counter--)
            {
              current=&Buffer[counter];
              /* check if we have a first preceding charcater */
              if (counter<(Length-1)) 
                  first_prec=&(Buffer[counter+1]);
              else 
                  first_prec=&blank; 
              /* check if we have a second preceding charcater */
              if (counter<(Length-2)) 
                 sec_prec=&(Buffer[counter+2]);
              else
                sec_prec=&blank;
              /* check if we have a third preceding charcater */
              if (counter<(Length-3)) 
                 third_prec=&(Buffer[counter+3]);
              else  third_prec=&blank; 
              /* now shape current and the preceding chars */
              csd_engine(current,first_prec,sec_prec,third_prec,
                         &state, OS_flag,Onecell);
            }
            /* now assume we have a symbol after the string and reshape 
               last three chars. We do not assume a blank, because
               in case last char is a seen, we do not want it to
               overwrite the blank with a tail.  */
            third_prec=sec_prec;  
            sec_prec=first_prec; 
            first_prec=&(Buffer[0]); /* this is the last char in the string */
            current=&symbol;         /* this is a hypothetical symbol */
            csd_engine(current,first_prec,sec_prec,third_prec,
                       &state, OS_flag,Onecell);
         }
       else /* RTL */
         {
           /* loop on whole string to be shaped */
           for (counter=0;counter<Length;counter++)
            {
              current=&Buffer[counter];
              /* check if we have a firct preceding charcater */
              if (counter>0) 
                 first_prec=&(Buffer[counter-1]);
              else  first_prec=&blank; 
              /* check if we have a second preceding charcater */
              if (counter>1) 
                 sec_prec=&(Buffer[counter-2]);
              else  sec_prec=&blank; 
              /* check if we have a third preceding charcater */
              if (counter>2) 
                 third_prec=&(Buffer[counter-3]);
              else  third_prec=&blank; 
              /* now shape current and the preceding chars */
               csd_engine(current,first_prec,sec_prec,third_prec,
                          &state, OS_flag,Onecell);
            }
            /* now assume we have a symbol after the string and reshape 
               last three chars. We do not assume a blank, because
               in case last char is a seen, we do not want it to
               overwrite the blank with a tail.  */
            third_prec=sec_prec;
            sec_prec=first_prec;  
            first_prec=&(Buffer[Length-1]);
            current=&symbol;         /* this is a hypothetical symbol */
            csd_engine(current,first_prec,sec_prec,third_prec,
                       &state, OS_flag,Onecell);
         }
       break;
     default   : /* all special shapes */
       if (orient==ORIENTATION_LTR)  /* LTR */
         {
           next=&(Buffer[Length-1]); /* initial value */
           /* loop on whole string to be shaped */
           for (counter=Length-1;counter>=0;counter--)
            {
              current=next;
              /* check if we have a next charcater */
              if (counter>0) 
                  next=&(Buffer[counter-1]);
              else 
                  next=&symbol;
              csd_special(csd,current,next);
            }
         }
       else /* RTL */
         {
           next=&(Buffer[0]); /* initial value */
           /* loop on whole string to be shaped */
           for (counter=0;counter<Length;counter++)
            {
              current=next;
              /* check if we have a next charcater */
              if (counter<(Length-1)) 
                  next=&(Buffer[counter+1]);
              else 
                  next=&symbol; 
              csd_special(csd,current,next);
            }
         }
         break;
   } /* end switch */
 return (0);
}

