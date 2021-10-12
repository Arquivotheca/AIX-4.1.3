static char sccsid[] = "@(#)41	1.2  src/bos/usr/lib/nls/loc/methods/shared.bidi/edit.c, cfgnls, bos411, 9428A410j 9/10/93 11:10:21";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: BidiEditShape
 *		FindState
 *		adjust_sec_succ
 *		handle_edit
 *		handle_input
 *		handle_special
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
#include "bdstruct.h"
#include <sys/lc_layoutP.h>
#include "csd.h"
#include <iconv.h>
#include <errno.h>
int  BidiEditShape
               (LayoutObjectP plh,
                BooleanValue EditType,
                size_t *index,
                const char *InpBuf,
                size_t *InpSize,
                void *OutBuf,
                size_t *OutSize)
                
{
  BidiValuesRec bidivalues;
  char *tmpbuf;
  size_t num_of_chars;
  int RC=0;

   bidivalues=(BidiValuesRec)plh->core->Values;
   /* check if shaping is active */
   if (plh->core->ShapeEditing==FALSE) return(0);

   if (bidivalues->Shaping.in==bidivalues->Shaping.out)
                      return(0);

   if ((EditType==EDITINPUT) && (InpBuf==NULL))  /* reset ShapeState */
      {
        bidivalues->ShapeState=InitialState;
        return(0);
      }

   if (*OutSize==0)   /* calculate OutSize needed */
   {
     if (EditType==EDITINPUT)
           *OutSize=bidivalues->ContextSize.back+1;
      else *OutSize=bidivalues->ContextSize.back+1+
                    bidivalues->ContextSize.front;
     return(0);
   }

   if (*InpSize==-1)  /* InpBuf is null terminated */
     *InpSize=strlen(InpBuf);
 
   tmpbuf=malloc(bidivalues->ContextSize.front+
                 bidivalues->ContextSize.back+1);

/********************** start of processing ***********************************/
   switch (bidivalues->Shaping.out)
   {
     case TEXT_SHAPED : /* auto shaping */
            if (EditType==EDITINPUT)
            RC = handle_input( bidivalues->OneCell.out,
                               bidivalues->SpecialSh.out,
                               bidivalues->Orient.in,
                               bidivalues->Shaping.out,
                               InpBuf,InpSize,
                               tmpbuf,&num_of_chars,index,
                               &bidivalues->ShapeState);
            else /* edit replace */
            RC = handle_edit ( bidivalues->OneCell.out,
                               bidivalues->SpecialSh.out,
                               bidivalues->Orient.in,
                               bidivalues->Shaping.out,
                               InpBuf,InpSize,
                               tmpbuf,&num_of_chars,index);
       break;
     default : /* all special shapes */
            handle_special ( bidivalues->Orient.in, 
                             bidivalues->Shaping.out,  
                             InpBuf,InpSize,
                             tmpbuf,&num_of_chars,index);
       break;
   } /* end switch */

   *InpSize=num_of_chars;

  /* copy temporary buffer to output buffer */
CopyOut : 
  if (OutBuf)
  {
     if (*OutSize>num_of_chars*bidivalues->OutCharSetSize)
        *OutSize=num_of_chars*bidivalues->OutCharSetSize;
     /* check if translation is required */
     if (bidivalues->iconv_handle!=-1)
     {
       size_t innum,outnum;
 
       innum=outnum=*OutSize;
       RC=iconv(bidivalues->iconv_handle,&tmpbuf,&innum,&OutBuf,&outnum);
       if (RC) return (errno);
     }
     else memcpy(OutBuf,tmpbuf,*OutSize); /* just copy, no translation */

     /* check if there was space for all the data */
     if (*OutSize<num_of_chars*bidivalues->OutCharSetSize)
        RC=E2BIG;
     free(tmpbuf);
  }

 return (RC);
}
/******************************************************************************/
/* Given the original character that is correctly shaped according to its
   successor, and the new character that is correctly shaped according to
   its predecessor, it decides the correct shape of the character.  
   This function is needed to determine the shape of the second succeeding
   character in edit shaping .
   There are four possibilities :
   1) the character was not changed at all.
   2) the character was a space and is replaced by a vowel, this
      is in case we had seen+vowel+space, and changed to seen+tail+vowel
   3) the character was an alef and is changed to a vowel, this is
      in case we had lam+vowel+alef, and is changed to lam+alef+vowel
      Or it is an alef and chaged to a space, in case we had a lam
      and an alef, and is changed to lam-alef + space, in Hosr mode.
   4) the characetr is the same char, but different shape, this is the case
      if we had dal+vowel+middle ein, and is changed to dal+vowel+
      initial ein.
*/
void adjust_sec_succ
          (char old_char,
           char *new_char)
{
    if (old_char==*new_char) return;             /* case 1 */
    if ((Group(old_char)==Spaces_Group) &&   /* case 2 */
        (Group(*new_char)==Vowels_Group)) return;
    if (((Group(old_char)==Alef_Group) || 
        (Group(old_char)==Spaces_Group)) &&     /* case 3 */
        (Group(*new_char)==Vowels_Group)) return;
    /* Now we are sure that we are in case 4 */
    /* If new char is initial, check if it needs to be initial or isolated */
    /* If it were initial or middle it means that it was left connected,
       so since it is already delayed shaped, do nothing. 
       If it was final or isolated, it means that it is not left connected,
       so change it accordingly. */
    if (*new_char==CHRGRP[*new_char-128][1])  /* initial */
             if ((old_char==CHRGRP[old_char-128][3])   /* final */
             || (old_char==CHRGRP[old_char-128][0]))   /* or isolated */
            *new_char=CHRGRP[old_char-128][0];  /* set to isolated */ 
    if (*new_char==CHRGRP[*new_char-128][2])  /* middle */
             if ((old_char==CHRGRP[old_char-128][3])   /* final */
             || (old_char==CHRGRP[old_char-128][0]))   /* or isolated */
            *new_char=CHRGRP[old_char-128][3];  /* set to final */ 
}
/******************************************************************************/
/* given the preceeding characters, this function returns the state
   of the current character */
void FindState
          (char *State,
           char first_prec,
           char sec_prec,
           char third_prec)
{
    char c1,c2,c3;
    char symbol=0x21;  /* ! */

    /* if we have a vowel we must look at the character before it */
    if (Group(first_prec)==Vowels_Group)
      {
        c1=sec_prec; 
        c2=third_prec; 
        c3=symbol;     
      }
    else
      {
        c1=first_prec; 
        c2=sec_prec;  
        c3=third_prec;
      }

    *State=InitialState;
    switch(Group(c1))
      {
          case Symbols_Group   :
          case Spaces_Group    :    /* no left connection */
          case Alef_Group      :  
          case R_Conn_Group    : *State=InitialState;
                                 break;
          case Vowels_Group    : /* just in case we have two vowels */
          case Normal_Group    : *State=MiddleState;
                                 break;
          case Seen_Group      : *State=SeenState;
                                 break;
          case Yeh_Hamza_Group : *State=YehHamzaState;
                                 break;
          case Lam_Group       : if (Group(c2) <= R_Conn_Group)
                                    *State=LamIsoState;
                                 else *State=LamConnState; 
                                 break;
      }
}

/******************************************************************************/
/* To handle input shaping: we send the current character and the three 
   preceeding to the engine, so the preceeding characters are adjusted,
   and the current shaped as if a wasla will follow.
*/
int handle_input
          (  int Onecell,
             int OS_flag,
             int orient,
             int csd,
             const char *Source,
             size_t *SourceLength,
             char *Target,
             size_t *TargetLength,
             size_t *Index,
             char *State)

{
 char current,     /* to hold characters for shaping */
      first_prec,
      sec_prec,
      third_prec;
 char blank=0x20;
 char symbol=0x21;  /* ! */
 size_t newindex;
 int i;

    newindex=*Index;
    *TargetLength =1;

    if (orient==ORIENTATION_LTR)  /* LTR */
      {
           current=(Source[*Index]);
           /* check if we have a first preceding charcater */
           if ((*Index+1)<*SourceLength) 
              {
                *TargetLength+=1;
                first_prec=(Source[*Index+1]);
              }
           else 
                first_prec=blank; 
           /* check if we have a second preceding character */
           if ((*Index+2)<*SourceLength) 
              {
                *TargetLength+=1;
                sec_prec=(Source[*Index+2]);
              }
           else
               sec_prec=blank; 
           /* check if we have a third preceding charcater */
           if ((*Index+3)<*SourceLength) 
              {
                *TargetLength+=1;
                third_prec=(Source[*Index+3]);
              }
           else 
              third_prec=blank;

           /* now shape current and the preceding chars */
           csd_engine(&current,&first_prec,&sec_prec,&third_prec,
                      State, OS_flag,Onecell);

           /* copy characters to target */
           Target[0]=current;
           if (*TargetLength>1)
              Target[1]=first_prec;
           if (*TargetLength>2)
              Target[2]=sec_prec;
           if (*TargetLength>3)
              Target[3]=third_prec;
      }
    else /* RTL */
      {
           current=Source[*Index];
           /* check if we have a first preceding charcater */
           if (*Index>0) 
               {
                 first_prec=(Source[*Index-1]);
                 newindex-=1;
                 *TargetLength+=1;
               }
           else 
                first_prec=blank;
           /* check if we have a second preceding character */
           if (*Index>1) 
              {
                 sec_prec=(Source[*Index-2]);
                 newindex-=1;
                 *TargetLength+=1;
               }
           else
               sec_prec=blank; 
           /* check if we have a third preceding charcater */
           if (*Index>2) 
               {
                 third_prec=(Source[*Index-3]);
                 newindex-=1;
                 *TargetLength+=1;
               }
           else 
              third_prec=blank;
           /* now shape current and the preceding chars */
           csd_engine(&current,&first_prec,&sec_prec,&third_prec,
                      State, OS_flag,Onecell);

           /* copy characters to target */
           i=0;
           if (*TargetLength>3)
              Target[i++]=third_prec;
           if (*TargetLength>2)
              Target[i++]=sec_prec;
           if (*TargetLength>1)
              Target[i++]=first_prec;
           Target[i]=current;
      }
     *Index=newindex;
return(0);
}
/******************************************************************************/
/* To handle edit shaping: we send to the shaping engine three times, once
   with the indexed character as current, to adjust the preceeding chars,
   second time with the character after the indexed as current, to adjust
   the indexed char, third time with second character after the indexed
   as current, to adjust the first character after the indexed. In the last
   call, we must save the status of the character that we are sending, to
   restore it after shaping because the engine will shape it as if a wasla
   follows, but this may not be the case.
*/
int handle_edit
          (  
             int Onecell,
             int OS_flag,
             int orient,
             int csd,
             const char *Source,
             size_t *SourceLength,
             char *Target,
             size_t *TargetLength,
             size_t *Index)
{
 char current,     /* to hold characters for shaping */
      first_prec,
      sec_prec,
      third_prec,
      first_succ,
      sec_succ,
      old_sec_succ;
 char blank=0x20;
 char symbol=0x21;  /* ! */
 char State=InitialState;
 size_t newindex;
 int i,before,after;

   newindex=*Index;
   *TargetLength=1;
   before=0;
   after=0;
   if (orient==ORIENTATION_LTR)   /* LTR */
   {
      current=Source[*Index];
      /* check if we have a first preceding charcater */
      if ((*Index+1)<*SourceLength) 
          {
            *TargetLength+=1;
            after+=1;
            first_prec=(Source[*Index+1]);
          }
      else 
           first_prec=blank; 
      /* check if we have a second preceding character */
      if ((*Index+2)<*SourceLength) 
          {
            *TargetLength+=1;
            after+=1;
            sec_prec=(Source[*Index+2]);
          }
      else
          sec_prec=blank; 
      /* check if we have a third preceding charcater */
      if ((*Index+3)<*SourceLength) 
          {
            *TargetLength+=1;
            after+=1;
            third_prec=(Source[*Index+3]);
         }
      else 
         third_prec=blank; 
      /* check if we have a first succeeding charcater */
      if (*Index>0) 
         {
           newindex-=1;
           *TargetLength+=1;
           before+=1;
           first_succ=(Source[*Index-1]);
         }
      else 
         first_succ=symbol;
      /* check if we have a second succeeding charcater */
      if (*Index>1) 
         {
           newindex-=1;
           *TargetLength+=1;
           before+=1;
           sec_succ=(Source[*Index-2]);
         }
      else 
         sec_succ=symbol;

      /* get state from available chars */
      FindState(&State,first_prec,sec_prec,third_prec);
   
      /* now shape current and the preceding chars */
      csd_engine(&current,&first_prec,&sec_prec,&third_prec,
                 &State, OS_flag,Onecell);
      /* now shape first succeeding char */
      csd_engine(&first_succ,&current,&first_prec,&sec_prec,
                 &State, OS_flag,Onecell);
      /* now shape second succeeding */
      old_sec_succ=sec_succ;  /* save it for after shaping */
      csd_engine(&sec_succ,&first_succ,&current,&first_prec,
                 &State, OS_flag,Onecell);
      adjust_sec_succ(old_sec_succ,&sec_succ);

      /* copy characters to target */
      i=0;
      if (before>1)
          Target[i++]=sec_succ; 
      if (before>0)
          Target[i++]=first_succ; 
      Target[i++]=current;
      if (after>0)
           Target[i++]=first_prec;
      if (after>1)
           Target[i++]=sec_prec;
      if (after>2)
           Target[i++]=third_prec;
   }
  else  /* RTL */
   {
      current=Source[*Index];
      /* check if we have a first preceding charcater */
      if (*Index>0) 
         {
          newindex-=1;
          before+=1;
          *TargetLength+=1;
          first_prec=(Source[*Index-1]);
         }
      else 
           first_prec=blank;
      /* check if we have a second preceding character */
      if (*Index>1) 
         {
          newindex-=1;
          before+=1;
          *TargetLength+=1;
          sec_prec=(Source[*Index-2]);
         }
      else
          sec_prec=blank; 
      /* check if we have a third preceding charcater */
      if (*Index>2) 
         {
          newindex-=1;
          before+=1;
          *TargetLength+=1;
          third_prec=(Source[*Index-3]);
         }
      else 
         third_prec=blank;
      /* check if we have a first succeeding charcater */
      if ((*Index+1)<*SourceLength) 
         {
          after+=1;
          *TargetLength+=1;
          first_succ=(Source[*Index+1]);
         }
      else 
         first_succ=symbol;
      /* check if we have a second succeeding charcater */
      if ((*Index+2)<*SourceLength)
         {
          after+=1;
          *TargetLength+=1;
          sec_succ=(Source[*Index+2]);
         }
      else 
         sec_succ=symbol;

      /* get state from available chars */
      FindState(&State,first_prec,sec_prec,third_prec);
   
      /* now shape current and the preceding chars */
      csd_engine(&current,&first_prec,&sec_prec,&third_prec,
                 &State, OS_flag,Onecell);
      /* now shape first succeeding char */
      csd_engine(&first_succ,&current,&first_prec,&sec_prec,
                 &State, OS_flag,Onecell);
      /* now shape second succeeding */
      old_sec_succ=sec_succ;  /* save it for after shaping */
      csd_engine(&sec_succ,&first_succ,&current,&first_prec,
                 &State, OS_flag,Onecell);
      adjust_sec_succ(old_sec_succ,&sec_succ);

      /* copy characters to target */
      i=0;
      if (before>2)
          Target[i++]=third_prec; 
      if (before>1)
          Target[i++]=sec_prec; 
      if (before>0)
          Target[i++]=first_prec; 
      Target[i++]=current;
      if (after>0)
           Target[i++]=first_succ;
      if (after>1)
           Target[i++]=sec_succ;
   }
   *Index=newindex;
   return(0);
}
/******************************************************************************/
/* To handle special shaping */
int handle_special
           ( int orient,
             int csd,
             const char *Source,
             size_t *SourceLength,
             char *Target,
             size_t *TargetLength,
             size_t *Index)

{
 char current,     /* to hold characters for shaping */
      next;
 char symbol=0x21;  /* ! */

      *TargetLength=1;
     /* all special shapes */
       if (orient==ORIENTATION_LTR)  /* LTR */
         {
            current=Source[*Index];
            /* check if we have a next charcater */
            if (*Index>0) 
             {
               next=(Source[*Index-1]);
               *Index-=1;
               *TargetLength+=1;
             }
            else 
               next=symbol;
           csd_special(csd,&current,&next);
           /* put new char in target */
           if (*TargetLength>1)
               {
                Target[0]=next;
                Target[1]=current;
               }
           else Target[0]=current;
         }
       else /* RTL */
         {
           current=Source[*Index];
           /* check if we have a next charcater */
           if ((*Index+1)<*SourceLength) 
             {
              next=(Source[*Index+1]);
              *TargetLength+=1;
             }
           else 
              next=symbol;
           csd_special(csd,&current,&next);
           /* put new char in target */
           Target[0]=current;
           if (*TargetLength>1)
               Target[1]=next;
         }
}


