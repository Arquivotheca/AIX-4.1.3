static char sccsid[] = "@(#)39	1.2  src/bos/usr/lib/nls/loc/methods/shared.bidi/csd.c, cfgnls, bos411, 9428A410j 9/10/93 11:10:14";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: ConnLamAlef
 *		Group
 *		InitMidShape
 *		InitialShape
 *		IsoFinalShape
 *		IsoLamAlef
 *		SpecialAlef
 *		ThreeQuarterSeen
 *		Vowel
 *		YehFinal
 *		csd_engine
 *		csd_special
 *		reset_alefs
 *		reset_tail
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
#include <sys/lc_layout.h>
#include "csd.h"
/************************************************************************/


/****************************************************************************/
void csd_engine(current,prec1,prec2,prec3,
                state,OSflag,Onecell)
   char *current;  /* current new character */
   char *prec1;    /* first preceding character */
   char *prec2;    /* second preceding character */
   char *prec3;    /* third preceding character */
   char *state;    /* the current state */
   int OSflag;              /* in Arabic, AIX or HOST shaping mode*/
   int Onecell;             /* in AIX, seen on onecell or two cells */
{/* start engine */
   char temp;    /* to hold temporary characters */

   reset_alefs(current); /* set special alefs to normal */
   reset_tail(current); /* set tail to space */
   switch (Group(*current))
   {
     case Symbols_Group :
            /* in all cases, set the previous char to final,
               if the previous char is a vowel, set also 
                the second previous to final */
            *prec1=IsoFinalShape(*prec1,*prec2,OSflag);
            if (Vowel(*prec1)) 
                 *prec2=IsoFinalShape(*prec2,*prec3,OSflag);
            *state=InitialState; /* set to intial, because symbols 
                                    do not connect */
            break;
           /* end case Symbols */
     case Spaces_Group :
            switch (*state)
            {
              case MiddleState  :
              case LamIsoState  :  /* all the normal characters */
              case LamConnState :
              case InitialLamConnState :
              case InitialLamIsoState :
                            /* set the previous char to final,
                               if the previous char is a vowel, set also 
                               the second previous to final */
                           *prec1=IsoFinalShape(*prec1,*prec2,
                                                OSflag);
                           if (Vowel(*prec1)) 
                               *prec2=IsoFinalShape(*prec2,*prec3,
                                                    OSflag);
                           break;
              case InitialSeenState :
              case SeenState :  /* previous char was a seen, 
                                   so consider a tail */
                          if (!Onecell)
                            if (Vowel(*prec1))
                                                   /* a vowel after the seen*/
                                 {
                                  /* put the vowel in current, and set prec2
                                     and prec1 to threequarterseen and to tail
                                     respectively */
                                  *prec2=ThreeQuarterSeen(*prec2);
                                  *current=IsoFinalShape(*prec1,*prec2,
                                                    OSflag);
                                  *prec1=A_TAIL;
                                 }
                            else /* no vowel after seen */
                                 {/* set prec1 and current to threequarterseen
                                     and tail respectively */
                                  *prec1=ThreeQuarterSeen(*prec1);
                                  *current=A_TAIL;
                                 }
                          else  /* onecell seen, even if there is space */
                          {    
                           *prec1=IsoFinalShape(*prec1,*prec2,
                                                OSflag);
                           if (Vowel(*prec1)) 
                              *prec2=IsoFinalShape(*prec2,*prec3,
                                                   OSflag);
                          }
                          break;
              case YehHamzaState : /*previous char was a yehhamza */
                            if ((OSflag==TEXT_STANDARD) 
                             || (OSflag==TEXT_COMPOSED))
                                { /* in AIX, yehhamza is handled like a 
                                     normal char */
                                  *prec1=IsoFinalShape(*prec1,*prec2,
                                                       OSflag);
                                  if (Vowel(*prec1)) 
                                     *prec2=IsoFinalShape(*prec2,*prec3,
                                                    OSflag);
                                }
                            else /* HOST mode */
                                 /* in HOST, yehhamza is split in two chars,
                                     if followed by a space */
                                  if (Vowel(*prec1))
                                       /* we have a vowel after the yehhamza */
                                   {
                                   /* put the vowel in current and set
                                   prec2 and prec1 to yeh and hamza
                                   respectively */
                                   *prec2=YehFinal(*prec2,*prec3);
                                   *current=IsoFinalShape(*prec1,*prec2,
                                                   OSflag);
                                   *prec1=A_HAMZA;
                                   }
                                  else /* novowel after the yehhamza */
                                   {
                                    *prec1=YehFinal(*prec1,*prec2);
                                    *current=A_HAMZA;
                                   }
                            break;
            }/* end switch state */
            *state=InitialState; /* set to intial, because spaces 
                                    do not connect */
            break;
     case Alef_Group :
              switch (*state)
              {
                case InitialState         :
                case MiddleState          :
                case SeenState            :
                case InitialSeenState     :
                case YehHamzaState        :
                            if (Vowel(*prec1)) 
                             {
                                 *prec1=InitialShape(*prec1,*state);
                                 *prec2=InitMidShape(*prec2,*prec3);
                             }
                             else *prec1=InitMidShape(*prec1,*prec2);
                            *current=IsoFinalShape(*current,*prec1,
                                              OSflag);
                            break;
                case LamConnState  :
                case InitialLamConnState  :
                            if ((OSflag==TEXT_STANDARD) 
                             || (OSflag==TEXT_COMPOSED))
                             {
                               if (Vowel(*prec1))
                               {
                                /* set lam to middle */
                                *prec2=(CHRGRP[(*prec2)-128][MID]);
                                /* switch places of alef and vowel */
                                temp=*current;
                                *current=IsoFinalShape(*prec1,temp,
                                                       OSflag);
                                *prec1=SpecialAlef(temp);
                               }
                               else  /* no vowel, normal AIX lamalef case */
                               {
                                /* set lam to middle */
                                *prec1=(CHRGRP[(*prec1)-128][MID]);
                                *current=SpecialAlef(*current);
                               }
                             }
                            else /* Arabic HOST mode */ 
                               if (Vowel(*prec1)
                                                ) /*put lamalef, vowel, space */
                               {
                                  *prec2=ConnLamAlef(*current);
                                  *prec1=IsoFinalShape(*prec1,*prec2,
                                                   OSflag);
                                  *current=A_RSP;
                               }
                               else /* put lamalef , space */
                               {
                                  *prec1=ConnLamAlef(*current);
                                  *current=A_RSP;
                               }
                            break;
                case LamIsoState   :
                case InitialLamIsoState   :
                            if ((OSflag==TEXT_STANDARD) 
                              || (OSflag==TEXT_COMPOSED))
                             {
                               if (Vowel(*prec1))
                               {
                                /* set lam to middle */
                                *prec2=(CHRGRP[(*prec2)-128][MID]);
                                /* switch places of alef and vowel */
                                temp=*current;
                                *current=IsoFinalShape(*prec1,*prec2,
                                                     OSflag);
                                *prec1=SpecialAlef(temp);
                               }
                               else  /* no vowel, normal AIX lamalef case */
                               {
                                /* set lam to middle */
                                *prec1=(CHRGRP[(*prec1)-128][MID]);
                                *current=SpecialAlef(*current);
                               }
                             }
                            else /* HOST mode */ 
                               if (Vowel(*prec1)) 
                                                  /*put lamalef, vowel, space */
                               {
                                  *prec2=IsoLamAlef(*current);
                                  *prec1=IsoFinalShape(*prec1,*prec2,
                                                   OSflag);
                                  
                                  *current=A_RSP;
                               }
                               else /* put lamalef , space */
                               {
                                  *prec1=IsoLamAlef(*current);
                                  *current=A_RSP;
                               }
                            break;
              }
            *state=InitialState; /* set to intial, because alefs 
                                    do not connect to the left */
            break;
     case R_Conn_Group :
            if (Vowel(*prec1)) 
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=IsoFinalShape(*current,*prec1,OSflag); 
            *state=InitialState; /* set to intial, because Rconnectors 
                                    do not connect to the left */
            break;
     case Normal_Group :
            if (Vowel(*prec1)) 
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=InitialShape(*current,*state);
            *state=MiddleState; /* normal chars connect left and right */
            break;
     case Seen_Group :
            if (Vowel(*prec1)) 
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=InitialShape(*current,*state);
            *state=SeenState; 
            break;
     case Yeh_Hamza_Group :
            if (Vowel(*prec1)) 
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=InitialShape(*current,*state);
            *state=YehHamzaState; 
            break;
     case Lam_Group :
            if (Vowel(*prec1)) 
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=InitialShape(*current,*state);
            if (*state==InitialState)
               *state=LamIsoState; 
            else
               *state=LamConnState; 
            break;
     case Vowels_Group :
            if (Vowel(*prec1)) 
            {
               *prec1=InitialShape(*prec1,*state);
               *prec2=InitMidShape(*prec2,*prec3);
            }
            else *prec1=InitMidShape(*prec1,*prec2);
            *current=InitialShape(*current,*state);
           /* If we have more than one vowel in a sequence,
              LamIso and LamConn and Seen states are all reset to Middle,
              because if we have more than one vowel in a sequence, we do 
              not handle these special cases any more. Otherwise the state
              is not affected by a vowel. */
            if (Vowel(*prec1))
              if (*state!=InitialState) *state=MiddleState;
            break;
     case Iso_Vowels_Group :
            /* in all cases, set the previous char to final,
               if the previous char is a vowel, set also 
                the second previous to final */
            *prec1=IsoFinalShape(*prec1,*prec2,OSflag);
            if (Vowel(*prec1)) 
                 *prec2=IsoFinalShape(*prec2,*prec3,OSflag);
            /* if more than one vowel in a sequence,
               set state to initial, because these vowels do not connect.
               If only one vowel, set state to initial, but preserve the 
               data of the seen and lam, because we can handle them after
               one vowel only. */
            if (Vowel(*prec1))
               *state=InitialState;
            else
              switch (*state)
              {
                case SeenState    : *state=InitialSeenState; break;
                case LamIsoState  : *state=InitialLamIsoState; break;
                case LamConnState : *state=InitialLamConnState; break;
                default           : *state=InitialState; break;
              }
            break;
   } /* end switch on group of current char */
}/* end engine */
/****************************************************************************/
/* reset tail character to space */
void reset_tail(ch)
  char *ch;
{
  if (*ch==A_TAIL)
     *ch=SPACE;
}
/****************************************************************************/
/* reset special alefs to normal alefs */
void reset_alefs(ch)
  char *ch;
{
     if (*ch==A_ALEF_HAMZA_SPECIAL)
        *ch=A_ALEF_HAMZA_ISOLATED; 
     else if (*ch==A_ALEF_MADDA_SPECIAL)
        *ch=A_ALEF_MADDA_ISOLATED; 
     else if (*ch==A_ALEF_SPECIAL)
        *ch=A_ALEF_ISOLATED; 
     else if (*ch==A_ALEF_HAMZA_UNDER_SPECIAL)
        *ch=A_ALEF_HAMZA_UNDER_ISOLATED; 
}
/****************************************************************************/
/* checks if a character is a vowel */
int Vowel(ch)
 char ch;
{
  return ((Group(ch)==Vowels_Group) || (Group(ch)==Iso_Vowels_Group));
}
/****************************************************************************/
/* Returns the group of the given characters, all characters under 0x80,
   except space, are considered symbols. For characters over 0x80, the
   lookup table CHRGRP is referenced. 
   */ 
int Group (ch)
 char ch;
{
  if (ch<START_NATIONAL)
     if (ch==SPACE)
          return (Spaces_Group);
     else return (Symbols_Group); /* Latin char */
  else return ((int)CHRGRP[ch-128][4]);
}
/****************************************************************************/
/* If the prev character connects to the left, final is returned, otherwise
   isolated is returned. OSflag is considred in cases of seen and yehhamza */
char IsoFinalShape (ch,prev,OSflag)
 char ch;
 char prev;
 int OSflag;
{
  char newch;
  if (ch<START_NATIONAL) return (ch);  /* Latin ch */
  if (((Group(prev)>=Normal_Group) 
     && (Group(prev)!=Vowels_Group)) ||  
                                   /* i.e. normal, seen, yehhamza, lam */
     ((Group(prev)==Vowels_Group) && (prev==CHRGRP[prev-128][MID])))
                                   /* i.e. a connected vowel */
     newch=CHRGRP[ch-128][FIN];  /* return final */
  else newch=CHRGRP[ch-128][ISO];  /* return isolated */ 
  if (OSflag==TEXT_SPECIAL)
   {
                                               /* onecell final seen chars */
     if ((newch>=A_ONECELL_SEEN) && (newch<=A_ONECELL_DAD))   
        newch=newch-0x50;   /* threequarter seen chars */
     if ((newch==A_YEH_HAMZA_FINAL) 
         || (newch==A_YEH_HAMZA_ISOLATED)) 
        newch=A_YEH_HAMZA_INITIAL;   
   }
 return(newch);
}
/****************************************************************************/
/* given a yehhamza, it returns a yeh, final or isolated depending 
   on the previous char */
char YehFinal (ch,prev)
 char ch;
 char prev;
{
  char newch;

  if ((Group(prev)>=Normal_Group) || 
                                       /* i.e. normal, seen, yehhamza, lam */
     ((Group(prev)==Vowels_Group) && (prev==CHRGRP[prev-128][MID])))
                                   /* i.e. a connected vowel */
    return(A_YEH_FINAL);  /* return final yeh */
     
  else return(A_YEH_ISOLATED);   /* return isolated yeh */ 
}
/****************************************************************************/
/* returns the middle or initial state of the current char dependign on */
/* whther the character before it is connected to it or not */
char InitMidShape(ch,prev)
char ch;
char prev;
{
  if (ch<START_NATIONAL)  return (ch);  /* Latin ch */
  if (Group(prev)<5)  /* no connection or R_connect character */
      ch=CHRGRP[ch-128][INIT];  /* return initial */
  else if (Group(prev)<9) /* R+L connector */
      ch=CHRGRP[ch-128][MID];  /* return middle */
  else /* it must be a vowel */
    if (prev==CHRGRP[prev-128][MID]) /* connected vowel */
        ch=CHRGRP[ch-128][MID];  /* return middle */
      else ch=CHRGRP[ch-128][INIT];  /* return initial */
  return(ch);
}
/****************************************************************************/
/* returns the middle or initial state of the current char dependign on state */
char InitialShape(ch,state)
char ch;
char state;
{
  if (ch<START_NATIONAL) return (ch);  /* Latin ch */
  if (state==InitialState)
    return (CHRGRP[ch-128][INIT]);
  else return (CHRGRP[ch-128][MID]);
}
/****************************************************************************/
/* input is a seen character, output is the threequarter shape of that ch */
char ThreeQuarterSeen(ch)
char ch;
{
  switch(ch)
  {
   case A_INITIAL_SEEN      :   
   case A_ONECELL_SEEN      :
   case A_THREEQUARTER_SEEN :
     return (A_THREEQUARTER_SEEN);
   case A_INITIAL_SHEEN      :   
   case A_ONECELL_SHEEN      :
   case A_THREEQUARTER_SHEEN :
     return (A_THREEQUARTER_SHEEN);
   case A_INITIAL_SAD      :   
   case A_ONECELL_SAD      :
   case A_THREEQUARTER_SAD :
     return (A_THREEQUARTER_SAD);
   case A_INITIAL_DAD      :   
   case A_ONECELL_DAD      :
   case A_THREEQUARTER_DAD :
     return (A_THREEQUARTER_DAD);
  }
}
/****************************************************************************/
/* returns the special (AIX lam_alef) shape of the given alef */
char SpecialAlef(alef)
char alef;
{
  switch (alef)
  {
    case A_ALEF_FINAL    :
    case A_ALEF_ISOLATED :
    case A_ALEF_SPECIAL  :
         return (A_ALEF_SPECIAL);
    case A_ALEF_HAMZA_FINAL    :
    case A_ALEF_HAMZA_ISOLATED :
    case A_ALEF_HAMZA_SPECIAL  :
         return (A_ALEF_HAMZA_SPECIAL);
    case A_ALEF_MADDA_FINAL    :
    case A_ALEF_MADDA_ISOLATED :
    case A_ALEF_MADDA_SPECIAL  :
         return (A_ALEF_MADDA_SPECIAL);
    case A_ALEF_HAMZA_UNDER_FINAL: 
    case A_ALEF_HAMZA_UNDER_ISOLATED:
    case A_ALEF_HAMZA_UNDER_SPECIAL:
         return (A_ALEF_HAMZA_UNDER_SPECIAL);
  }
}
/****************************************************************************/
/* given an alef, it returns the isolated lamalef */
char IsoLamAlef(alef)
char alef;
{
  switch (alef)
  {
    case A_ALEF_ISOLATED :   
    case A_ALEF_FINAL    :
    case A_ALEF_SPECIAL  :
         return (A_LAM_ALEF_ISOLATED);
    case A_ALEF_HAMZA_ISOLATED :   
    case A_ALEF_HAMZA_FINAL    :
    case A_ALEF_HAMZA_SPECIAL  :
         return (A_LAM_ALEF_HAMZA_ISOLATED);
    case A_ALEF_MADDA_ISOLATED :   
    case A_ALEF_MADDA_FINAL    :
    case A_ALEF_MADDA_SPECIAL  :
         return (A_LAM_ALEF_MADDA_ISOLATED);
    case A_ALEF_HAMZA_UNDER_ISOLATED :   
    case A_ALEF_HAMZA_UNDER_FINAL    :
    case A_ALEF_HAMZA_UNDER_SPECIAL  :
         return (A_LAM_ALEF_HAMZA_UNDER_ISOLATED);
  }
}
/****************************************************************************/
/* given an alef, it returns the connected lamalef */
char ConnLamAlef(alef)
char alef;
{
  switch (alef)
  {
    case A_ALEF_ISOLATED :   
    case A_ALEF_FINAL    :
    case A_ALEF_SPECIAL  :
         return (A_LAM_ALEF_CONNECTED);
    case A_ALEF_HAMZA_ISOLATED :   
    case A_ALEF_HAMZA_FINAL    :
    case A_ALEF_HAMZA_SPECIAL  :
         return (A_LAM_ALEF_HAMZA_CONNECTED);
    case A_ALEF_MADDA_ISOLATED :   
    case A_ALEF_MADDA_FINAL    :
    case A_ALEF_MADDA_SPECIAL  :
         return (A_LAM_ALEF_MADDA_CONNECTED);
    case A_ALEF_HAMZA_UNDER_ISOLATED :   
    case A_ALEF_HAMZA_UNDER_FINAL    :
    case A_ALEF_HAMZA_UNDER_SPECIAL  :
         return (A_LAM_ALEF_HAMZA_UNDER_CONNECTED);
  }
}
/****************************************************************************/
/* To handle special shapes : base, initial, middle, final, isolated. */
/* We need the next char for lamalef cases in base shape */
void csd_special (csd,current,next)
  int csd;
  char *current;
  char *next;
{
     if((*current>=START_NATIONAL))     /* not Latin character */
     {
        /* handle all special cases */
        switch (*current)
        {
          /* switch all special alefs to normal alefs */
          case A_ALEF_MADDA_SPECIAL :
                    *current=A_ALEF_MADDA_ISOLATED; break;
          case A_ALEF_SPECIAL :
                    *current=A_ALEF_ISOLATED; break;
          case A_ALEF_HAMZA_SPECIAL :
                    *current=A_ALEF_HAMZA_ISOLATED; break;
          case A_ALEF_HAMZA_UNDER_SPECIAL :
                    *current=A_ALEF_HAMZA_UNDER_ISOLATED; break;
          /* replace tail with blank */
          case A_TAIL : /* tail */
                    *current=A_RSP; break;
        }
        /* set character to desired special shape */
        switch(csd)
        {
          case TEXT_NOMINAL :
                            /* in Arabic base shapes are all isolated */
                              *current=CHRGRP[*current-128][ISO];
                            /* handle special cases */
                            switch (*current)
                            {
                            case A_HEH_ISOLATED : /* heh isolated */
                                      *current=A_HEH_INITIAL; /* base heh */
                                      break;
                            /* switch all seen threequarter to normal seen */
                            case A_THREEQUARTER_SEEN :
                                         *current=A_ONECELL_SEEN; break;
                            case A_THREEQUARTER_SHEEN :
                                         *current=A_ONECELL_SHEEN; break;
                            case A_THREEQUARTER_SAD : 
                                         *current=A_ONECELL_SAD; break;
                            case A_THREEQUARTER_DAD :
                                         *current=A_ONECELL_DAD; break;
                            /* handle lam alef cases */
                            case A_LAM_ALEF_CONNECTED: case A_LAM_ALEF_ISOLATED:
                                     if ((*next==SPACE) || (*next==A_RSP))
                                     {
                                       *current=A_LAM_ISOLATED;
                                       *next=A_ALEF_ISOLATED;
                                     }
                            case A_LAM_ALEF_MADDA_CONNECTED: 
                            case A_LAM_ALEF_MADDA_ISOLATED:
                                     if ((*next==SPACE) || (*next==A_RSP))
                                     {
                                       *current=A_LAM_ISOLATED;
                                       *next=A_ALEF_MADDA_ISOLATED;
                                     }
                            case A_LAM_ALEF_HAMZA_CONNECTED: 
                            case A_LAM_ALEF_HAMZA_ISOLATED:
                                     if ((*next==SPACE) || (*next==A_RSP))
                                     {
                                       *current=A_LAM_ISOLATED;
                                       *next=A_ALEF_HAMZA_ISOLATED;
                                     }
                            case A_LAM_ALEF_HAMZA_UNDER_CONNECTED: 
                            case A_LAM_ALEF_HAMZA_UNDER_ISOLATED:
                                     if ((*next==SPACE) || (*next==A_RSP))
                                     {
                                       *current=A_LAM_ISOLATED;
                                       *next=A_ALEF_HAMZA_UNDER_ISOLATED;
                                     }
                            }
                        break;
          case TEXT_INITIAL :
                            *current=CHRGRP[*current-128][INIT];
                            break;
          case TEXT_MIDDLE :
                            *current=CHRGRP[*current-128][MID];
                            break;
          case TEXT_FINAL :
                            *current=CHRGRP[*current-128][FIN];
                            break;
          case TEXT_ISOLATED :
                            *current=CHRGRP[*current-128][ISO];
                            break;
        }/* end switch */
     } /* if Arabic character */
}

/****************************************************************************/
