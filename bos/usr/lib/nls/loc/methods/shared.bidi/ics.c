static char sccsid[] = "@(#)44	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/ics.c, cfgnls, bos411, 9428A410j 8/30/93 15:02:46";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: CheckLTRSpecialCases
 *		CheckLevels
 *		CheckRTLSpecialCases
 *		ContainsEnglishChar
 *		EndLevel1
 *		EndLevel2
 *		End_L1_L2
 *		Swap1046
 *		Swap856
 *		Swap8859_8
 *		arab_num
 *		display_selection
 *		get_level
 *		getgroup_A1046
 *		getgroup_H856
 *		getgroup_H8859_8
 *		ics
 *		ics_block
 *		reverse
 *		to_hindi
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
/**************************** SOURCE FILE HEADER ****************************/
/*                                                                          */
/* SOURCE FILE NAME: ICS.C                                                  */
/*                                                                          */
/* DESCRIPTIVE NAME: Implicit Control Support.                              */
/*                                                                          */
/* FUNCTION: This module contains the routines that are required to convert */
/*          a logical order of text into a visual order.                    */  
/*                                                                          */
/* ENTRY POINTS:                                                            */
/*             ics_block                                                    */
/*             cal_cursor                                                   */
/*             reverse                                                      */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/

#include <stdio.h>
#include <sys/errno.h>
#include <sys/lc_layout.h>
#include "bdstruct.h"
#include "ics_def.h"
#include "ics_tables.h"
#define Arabic 1
#define Hebrew 2

/*********** Global pointers to tables and procedures that ************/
/************** vary with orientation and language ********************/
/* Tables */
int  (*StateTable)[][NUM_GRPS];

void  (*CheckSpecialCases) ();
int  (*GetGroup) ();
int  (*Swaps) ();
int lang;

/***********************************************************************/

unsigned short ContainsEnglishChar(buf,Start,End)
char *buf;
short Start,End;
                                      /* This routine returns TRUE when buf 
                                         contains an English character.    */
{ 
 short i;
 int group;

  for (i=Start; i<=End ; i++) {
      group = GetGroup (buf[i]);
      if (group == LDEF) return TRUE;
  } /* endfor */
  return FALSE;
}

/***********************************************************************/

void EndLevel2(IcsRec,StartL2,EndL2)
ICSPARAMRec *IcsRec;
short StartL2,EndL2;

                                  /* This routine ends the Level-2 segment by 
                                     reversing it only if flip_flag is true. */
{
  void reverse ();

   if (IcsRec->flip_flag) 
   {
       reverse(IcsRec->buffer,StartL2,EndL2,1);
       if (IcsRec->TrgToSrcMap)
          reverse(IcsRec->TrgToSrcMap,StartL2*sizeof (unsigned int),
                  EndL2*sizeof (unsigned int),sizeof (unsigned int)); 
   }
}
/***********************************************************************/

void EndLevel1(IcsRec,StartL1,EndL1)
ICSPARAMRec *IcsRec;
short StartL1, EndL1;

                      /* This routine ends the Level-1 segment. If the segment
                         contains an English character, the numbers are 
                         displayed as Arabic numerics else they are displayed 
                         as Hindi numerics.                                   */
{
  void reverse ();

   if ((IcsRec->num_flag==NUMERALS_CONTEXTUAL) && (IcsRec->orient==ORIENTATION_RTL) 
                      && (lang==Arabic)   && (EndL1 >= StartL1))
   {
     if (ContainsEnglishChar(IcsRec->buffer,StartL1,EndL1))
          Handlenumbers (IcsRec->buffer,StartL1,EndL1,
                         NUMERALS_NOMINAL);
     else Handlenumbers (IcsRec->buffer,StartL1,EndL1,
                         NUMERALS_NATIONAL);
   } 
   if (IcsRec->flip_flag) 
   {
      reverse(IcsRec->buffer,StartL1,EndL1,1);
      if (IcsRec->TrgToSrcMap)
        reverse(IcsRec->TrgToSrcMap,StartL1*sizeof (unsigned int),
                EndL1*sizeof (unsigned int),sizeof (unsigned int)); 
   } 
}

/***********************************************************************/

void End_L1_L2(IcsRec,StartL1,StartL2,EndL1_2)
ICSPARAMRec *IcsRec;
short *StartL1, *StartL2;
short EndL1_2;

                   /* This routine ends the Level-2 segment if there is one, 
                      then it ends the Level-1 segment if there is one.     */

{
                                      /* If we are ending a level2 segment */
                                                             /* reverse it */
   if (*StartL2!=NOT_SET)
   {
       EndLevel2(IcsRec, *StartL2, EndL1_2);
       *StartL2 = NOT_SET;                         /* Ended level2 segment */
   }
                                      /* If we are ending a level1 segment */
                                      /* reverse it. This will always be   */
                                      /* TRUE if there was a level 2       */
   if (*StartL1!=NOT_SET)
   {
      EndLevel1(IcsRec,*StartL1,EndL1_2);
      *StartL1 = NOT_SET;
    }
}

/***********************************************************************/

void display_selection (orient,level,digit,num_flag)
int orient,num_flag;
int level;
char *digit;

                   /* This routine determines the appearance of numbers. It
                      works only in case of Arabic language, not hebrew. In
                      case of upon context numeric flag, numbers are considered
                      Arabic so are displayed as hindi numerals if they exist 
                      at Level-2 in LTR, or if they follow a character of 
                      Level-0 in RTL. Otherwise they are considered English and
                      are displayed as arabic numerals.                      */

{
   void arab_num ();
   void to_hindi ();

   if (lang==Hebrew) return;

   if (num_flag == NUMERALS_NONE)
       return;
   if (num_flag == NUMERALS_NOMINAL)
          arab_num (digit);
   else if (num_flag == NUMERALS_NATIONAL)
	  to_hindi (digit);
   else {
       if (((orient == ORIENTATION_LTR) && (level == 2)) 
        || ((orient == ORIENTATION_RTL) && (level == 0)) )
            to_hindi (digit);
       else arab_num(digit);
	   }
}

/***********************************************************************/

void CheckLevels (level,IcsRec,pos,StartL1,StartL2)
int level, pos;
ICSPARAMRec *IcsRec;
short *StartL1, *StartL2;

      /* This function handles the display of characters according to their 
         levels. It handles the display of directional characters and numbers.
         Directional characters are swapped when they exist in arabic or
         hebrew segments; ie Level-0 in RTL or Level-1 in LTR. It starts/ends 
         Level-1/2 segments when the level changes.                          */
{
      switch (level)
      {
        case 0:                              /* The character is at level 0 */
           if (IcsRec->symmetric && (IcsRec->orient==ORIENTATION_RTL))
                 IcsRec->buffer[pos] = Swaps (IcsRec->buffer[pos]);
                                         /* End both Level-1 and 2 segment. */
           End_L1_L2 (IcsRec,StartL1,StartL2,pos-1);
           if (GetGroup (IcsRec->buffer[pos]) == DIGIT)
                display_selection (IcsRec->orient,level,IcsRec->buffer+pos,
                                   IcsRec->num_flag);
        break;

        case 1:                              /* The character is at level 1 */
           if (IcsRec->symmetric && (IcsRec->orient==ORIENTATION_LTR))
              IcsRec->buffer[pos] = Swaps (IcsRec->buffer[pos]);
           if (GetGroup (IcsRec->buffer[pos]) == DIGIT)
                display_selection (IcsRec->orient,level,IcsRec->buffer+pos,
                                   IcsRec->num_flag);

                               /* If we are not already in a level1 segment */
                               /* save the index of the level1 beginning.   */
           if (*StartL1==NOT_SET)
               *StartL1 = pos;
           else                                       /* Already in level1 */
           {
                                      /* If we are ending a level2 segment */
                                                             /* reverse it */
             if (*StartL2!=NOT_SET)
             {
               EndLevel2(IcsRec, *StartL2, (pos-1));
               *StartL2 = NOT_SET;                 /* Ended level2 segment */
             }
           }
        break;

        case 2:                             /* The character is at level 2 */
                              /* If we are not already in a level2 segment */
                              /* save the index of the level2 beginning.   */
           if (*StartL2==NOT_SET)
               *StartL2 = pos;
         /*  if (GetGroup (buf[pos]) == DIGIT) */
               display_selection (IcsRec->orient,level,IcsRec->buffer+pos,
                                  IcsRec->num_flag);
        break;
      }
}

/***********************************************************************/

int ics (IcsRec)
ICSPARAMRec *IcsRec;

              /* This function calls the get_level routine to assign nesting 
                 levels to each character in the buffer. Then it calls the    
                 CheckLevels routine to check the level given and to do the  
                 required reordering. */
{
  int get_level ();
  int position = 0;            /* Position of the current character handled */  
  int pending   = NOT_SET;                    /* Start of pending string.   */
  int state     = 0;                                       /* Machine state */
  short StartL1 = NOT_SET,              /* Index to start of level1 segment */
        StartL2 = NOT_SET;              /* Index to start of level2 segment */


                    /* go through the line and calculate the nesting levels. */ 
  while (position < IcsRec->num)
  {
     get_level (IcsRec,position,&pending,&state,
                               &StartL1,&StartL2);
     CheckLevels (IcsRec->A_level[position],IcsRec,position,
                  &StartL1,&StartL2);
     position ++;
  }
                       /* When the buffer has ended, check to see if there  */
                       /* are any segments that still need to be reversed.  */
  End_L1_L2 (IcsRec,&StartL1,&StartL2,position-1);
  return (0);
}

/***********************************************************************/

int get_level (IcsRec,position,pending,state,StartL1,StartL2)
ICSPARAMRec *IcsRec;
int *pending,*state;
short *StartL1, *StartL2;

    /* This routine calculates the level of the new character at 'position'. 
       It looks at the ICS tables and gets the new level and new state 
       according to the current state and character group. It also handles 
       unconditional and stable states by setting the pending string. If the 
       character belongs to one of the stable groups it stabilizes the pending 
       string to its level unless indicated by 'special_type'. If it belongs to
       one of the conditional groups, it either starts or continues the pending
       string. The temporary level given depends on the ICS tables.          */

{
  register int NewGroup;                               /* Character group.  */
  register int NewLevel;           /* New level given to character */
  register int OldState;                     /* State of previous character */
  int PendingLevel;          /* Level of pending chars except last */
  int LastCharLevel;                 /* Level of last pending char */
  int special_type;                              /* Flag for special cases. */

  NewGroup = GetGroup (IcsRec->buffer[position]);

                            /* In Word Break mode, consider spaces as ZDEF */
  if ((IcsRec->wordbreak) && (NewGroup == SPACE))
        NewGroup = ZDEF;

                                /* Save the state of the original character */
  OldState = *state;
                       /* Determine new state, new level and set the flags to 
                          indicate special cases using the ICS tables where 
                          they are preceded by a minus sign.                 */ 
  *state = (*StateTable) [OldState][NewGroup];
  special_type = FALSE;
  if (*state<0) {                                            /* Special case */
        *state *= -1;
        PendingLevel =
        LastCharLevel =
        NewLevel = (*StateTable) [*state][0];                   /* New level */
        CheckSpecialCases(OldState,
                          NewGroup,
                          position,
                          &special_type,
                          &PendingLevel,
                          &LastCharLevel,
                          StartL1,
                          StartL2,
                          pending);
  } /* endif */
  else
  {
    PendingLevel =
    LastCharLevel =
    NewLevel = (*StateTable) [*state][0];                      /* New level */
  }

  if (NewLevel < 0)                                           /*  unstable  */
  {
    NewLevel *= -1;
                                               /* Start a pending string if */
                                               /* there is not one already  */
    if (*pending == NOT_SET)
        *pending = position;

     if (special_type == CONT_SPACE)
     {
       if (*StartL2!=NOT_SET)
       {  
         EndLevel2(IcsRec, *StartL2, *pending-1);
         *StartL2 = NOT_SET;                        /* Ended level2 segment */
       }
       IcsRec->A_level[position-1] = PendingLevel;
    }
     if (special_type == CONT_PREFIX)
     {
       if (*StartL1!=NOT_SET)
       {  
         EndLevel1(IcsRec,*StartL1,position-2);
         *StartL1 = position;             /*Set start of Level-1 to position */
         *pending = position;           
       }
       IcsRec->A_level[position-1] = PendingLevel;
    }
 }    /* endif not stable*/
 else                                                       /* Stable state */
                     /* In case of stable character groups, set the pending 
                        string to a stable level according to the flags set,
                        and set the pending variable to -1.                 */
                                               /* There is a pending string */
   if (*pending != NOT_SET)
   {
                            /* Set pending string to level of new character */

     if ((NewLevel == 0) || (LastCharLevel == 0)) 
                                           /* End level 1 and/or 2 if needed */
         End_L1_L2 (IcsRec,StartL1,StartL2,*pending-1); 


     if ( (IcsRec->orient == ORIENTATION_LTR)
          && ((NewLevel == 1) || (LastCharLevel == 1)) )
       if (*StartL2!=NOT_SET)
       {  
         EndLevel2(IcsRec, *StartL2, *pending-1);
         *StartL2 = NOT_SET;                         /* Ended level2 segment */
       }
     while (*pending < (position-1))
     {
                             /* Set pending string to level of new character */
        if (IcsRec->symmetric &&
            (NewLevel == 0))
                                          /* Swap the directional character. */
                IcsRec->buffer [*pending] = Swaps (IcsRec->buffer[*pending]);
         IcsRec->A_level [*pending] = PendingLevel;
         *pending +=1;
     }
     if (IcsRec->symmetric &&
        (LastCharLevel == 0))
                                          /* Swap the directional character. */
                IcsRec->buffer [*pending] = Swaps (IcsRec->buffer[*pending]);
     IcsRec->A_level [*pending] = LastCharLevel;
     *pending = NOT_SET;
   }
   IcsRec->A_level [position] = NewLevel;
  return ;
}

/***********************************************************************/

int getgroup_A1046 (ch_grp)
register int ch_grp;
                         /* This routine gets the group of a certain character 
                            according to the Arabic code page IBM1046.       */
{
 if (I_Init_0_A (ch_grp))                    /* Check initial level-0 group. */
	 return (INIT);
 if (Z_Def_0_A (ch_grp))                    /* Check definite level-0 group. */
	 return (ZDEF);
 if (R_Def_RTL_A (ch_grp))                      /* Check definite RTL group. */
	 return (RDEF);
 if (L_Def_LTR_A (ch_grp))                      /* Check definite LTR group. */
	 return (LDEF);
 if (D_Digit_A (ch_grp))                              /* Check digits group. */
	 return (DIGIT);
 if (X_Prefix_A (ch_grp))                      /* Check prefix/suffix group. */
         return (XPRE);
 if (P_Punct_A (ch_grp))                 /* Check numeric punctuation group. */ 
         return (PUNCT);
 if (N_Neutral_A (ch_grp))                          /* Check neutrals group. */
	 return (NEUTR);
 if (S_Space (ch_grp))                                 /* Check space group. */
	 return (SPACE);
 if (Q_RSP (ch_grp))                          /* Check required space group. */
         return (QRSP);
 return (NEUTR);                                    /* Consider it a neutral */
}

/***********************************************************************/

int Swap1046 (ch_grp)
register int ch_grp;
                              /* This routine swaps directional characters
                                 according to the Arabic code page IBM1046. */
{
 return (swap_A1046 (ch_grp));
}

/***********************************************************************/

int getgroup_H856 (ch_grp)
register int ch_grp;
                         /* This routine gets the group of a certain character 
                            according to the Hebrew code page IBM856.        */
{
                                                 
 if (I_Init_0_H856 (ch_grp))                 /* Check initial level-0 group. */
	 return (INIT); 
 if (Z_Def_0_H856 (ch_grp))                 /* Check definite level-0 group. */
	 return (ZDEF);
 if (R_Def_RTL_H856 (ch_grp))                   /* Check definite RTL group. */
	 return (RDEF);
 if (L_Def_LTR_H856 (ch_grp))                   /* Check definite LTR group. */
	 return (LDEF);
 if (D_Digit_H856 (ch_grp))                           /* Check digits group. */
	 return (DIGIT);
 if (X_Prefix_H856 (ch_grp))                   /* Check prefix/suffix group. */
         return (XPRE);
 if (P_Punct_H856 (ch_grp))              /* Check numeric punctuation group. */
         return (PUNCT);
 if (N_Neutral_H856 (ch_grp))                       /* Check neutrals group. */
	 return (NEUTR);
 if (S_Space (ch_grp))                                 /* Check space group. */
	 return (SPACE);
 if (Q_RSP_H856 (ch_grp))                     /* Check required space group. */
         return (QRSP);
 return (NEUTR);                                    /* Consider it a neutral */
}

/***********************************************************************/

int Swap856 (ch_grp)
register int ch_grp;
                              /* This routine swaps directional characters
                                 according to the Hebrew code page IBM856. */
{
 return (swap_H856 (ch_grp));
}

/***********************************************************************/

int getgroup_H8859_8 (ch_grp)
register int ch_grp;
                         /* This routine gets the group of a certain character 
                            according to the Hebrew code page ISO8859-6.     */
{
 if (I_Init_0_H8859 (ch_grp))                /* Check initial level-0 group. */
	 return (INIT);
 if (Z_Def_0_H8859 (ch_grp))                /* Check definite level-0 group. */ 
	 return (ZDEF);
 if (R_Def_RTL_H8859 (ch_grp))                  /* Check definite RTL group. */
	 return (RDEF);
 if (L_Def_LTR_H8859 (ch_grp))                  /* Check definite LTR group. */
	 return (LDEF);
 if (D_Digit_H8859 (ch_grp))                          /* Check digits group. */
	 return (DIGIT);
 if (X_Prefix_H8859 (ch_grp))                  /* Check prefix/suffix group. */
         return (XPRE);
 if (P_Punct_H8859 (ch_grp))             /* Check numeric punctuation group. */
         return (PUNCT);
 if (N_Neutral_H8859 (ch_grp))                      /* Check neutrals group. */
	 return (NEUTR);
 if (S_Space (ch_grp))                                 /* Check space group. */
	 return (SPACE);
 if (Q_RSP (ch_grp))                          /* Check required space group. */
         return (QRSP);
 return (NEUTR);                                    /* Consider it a neutral */
}

/***********************************************************************/

int Swap8859_8 (ch_grp)
register int ch_grp;
                            /* This routine swaps directional characters
                               according to the Hebrew code page ISO8859-8. */
{
 return (swap_H8859_8 (ch_grp));      /* GIL, Changed - 26 Dec 91 */
}

/***********************************************************************/

void CheckLTRSpecialCases(register int   OldState,
                                   int   Group,
                                   int   position,
                          register int *special_type,
                          register int *PendingLevel,
                          register int *LastCharLevel,
                          register short *StartL1,
                          register short *StartL2,
                          register int *pending)
{

  *special_type = FALSE;

                                      /* Neutrals after numeric continuation */
                                                            /* Old flag = b2 */
  if ((OldState == NUM_CONT) || (OldState == NUM_CONT_SA) )
  {
                    /* The new character receives the nesting level specified 
                       in the table, and the preceding character changes from 
                       level 2 to that nesting level.                        */

     int state = (*StateTable) [OldState][Group];               /* New state */
     int NewLevel ;
    

     if (state < 0)
           state = -state;
     NewLevel = (*StateTable) [state][0];                       /* New level */

        if ((NewLevel == 1) || (NewLevel == -1))
        {
          *PendingLevel  = 1;
          *LastCharLevel = 1;           
          *special_type = CONT_SPACE;
          if (NewLevel == -1)
              *pending = position-1;
        }
  }
                                                    /* Digit after prefix */
                                                          /* Old flag = d */
  else if ((Group == DIGIT) &&
      (OldState == PREFIX) || (OldState == PREFIX_SA) ||
      (OldState == L1_SPC_PRE_SA) )
  {
       *PendingLevel  = 1;
       *LastCharLevel = 2;                /* Start level 2 */
       *StartL2 = position -1;
  }
                            /* Digit after level-1+space and/or continuation */
                                                           /* Old flag = c */
  else *pending = NOT_SET;
}

/***********************************************************************/

void CheckRTLSpecialCases(register int   OldState,
                                   int   Group,
                                   int   position,
                          register int *special_type,
                          register int *PendingLevel,
                          register int *LastCharLevel,
                                   short *StartL1,
                          register short *StartL2,
                          register int *pending)
{
  *special_type = FALSE;

                             /* The pending string must be set to Level-0  */
                             /* This applies for a special cases, except g */
     *PendingLevel  =
     *LastCharLevel = 0;
                                       /* Double prefix. The pending prefix 
                                          must be set to Level-0.          */
                                                           /* Old flag = e */
     if ((Group == XPRE) && (OldState == PREFIX))
     {
          *pending = position;
          *StartL1 = position;
     }
                          /* Prefix after numeric continuation. The pending 
                             numeric continuation must be set to Level-0.  */
                                                           /* Old flag = f */
     else if ((Group == XPRE) && (OldState == NUM_CONT))
     {
          *special_type = CONT_PREFIX;
     }
                       /* Digit after prefix in a SA segment. The new digit
                          and the preceding prefix receive Level-1, but any 
                          preceding conditional string receives Level-0.   */
                                                           /* Old flag = g */
     else if ((Group == DIGIT) && (OldState == PREFIX_SA))
     {
         *LastCharLevel = 1;
         *StartL1 = position -1;
     }
}

/***********************************************************************/


void arab_num (num)
char *num;

                      /* This routine converts numbers to arabic numerals. */
{
  if ((*num >= 0xB0) && (*num <= 0xB9))         
    *num -= 0x80;   
}

/***********************************************************************/

void to_hindi (num)
char *num;

                       /* This routine converts numbers to hindi numerals. */
{
  if ((*num >= 0x30) && (*num <= 0x39))         
    *num += 0x80;
}

/***********************************************************************/

void reverse (buf,begin,end,increment)
register char *buf;
register int begin;
register int end;
register int increment;

                                      /* This routine reverses a part of the 
                                         string in place from begin to end. */
{
 register char *temp; 
 register int i;
 register int j;

 temp = malloc (sizeof(char) *(increment+2));
 memset (temp,'\0',(sizeof(char) * (increment+2)));
 for (i=begin, j=end; i<j; i += increment,j -= increment)
 {
   memcpy(temp,&(buf[i]),increment);
   memcpy(&(buf[i]),&(buf[j]),increment);
   memcpy(&(buf[j]),temp,increment);
 }
 free (temp);

}

/***********************************************************************/

int ics_block (IcsRec)
ICSPARAMRec *IcsRec;

              /* This is the main ICS block. It converts a logical buffer to
                 a visual one. */
{
  int NeedLevelMap = FALSE;
  int i; 
  int local_vis = FALSE;
  unsigned int *local_vis_map;


        /* Set the language and the GetGroup and Swaps functions 
           according to the code page. */

      if ((strcmp(IcsRec->codeset,"IBM-856")==0)
      ||  (strcmp(IcsRec->codeset,"IBM-862")==0))
         {
           GetGroup = getgroup_H856;  
           Swaps = Swap856;
           lang=Hebrew;
         }
      else if (strcmp(IcsRec->codeset,"ISO8859-8")==0)
         {
           GetGroup = getgroup_H8859_8;
           Swaps = Swap8859_8;
           lang=Hebrew;
         }
      else if ((strcmp(IcsRec->codeset,"IBM-1046")==0)
      ||  (strcmp(IcsRec->codeset,"ISO8859-6")==0))
         {
           GetGroup = getgroup_A1046;
           Swaps = Swap1046;
           lang=Arabic;
         }
       else 
         return(0);  /* codepage not supported */

               /* Set the StateTable and CheckSpecialCases function according 
                  to the orientation and the language (arabic/hebrew).      */
   switch (IcsRec->orient)
   {
      case ORIENTATION_LTR:
         StateTable        = LTR_state;
         CheckSpecialCases = CheckLTRSpecialCases;
      break;

      case ORIENTATION_RTL:
         if (lang==Arabic)
            StateTable        = RTL_Astate;
         else StateTable        = RTL_Hstate;
         CheckSpecialCases = CheckRTLSpecialCases;
      break;

   } /* endif */

   if (!IcsRec->A_level)
        {
           IcsRec->A_level = (unsigned char *) malloc(IcsRec->num
                                                    * sizeof(unsigned char));
           if (!IcsRec->A_level) return ENOMEM;
           memset (IcsRec->A_level, 0, IcsRec->num 
                                          * sizeof(unsigned char));
           NeedLevelMap = TRUE;
        }
    else NeedLevelMap=FALSE;

    if (IcsRec->SrcToTrgMap && (!IcsRec->TrgToSrcMap)) 
    {
        local_vis_map = (unsigned long *) malloc (IcsRec->num 
                                          * sizeof (unsigned int));
        memset (local_vis_map,0,IcsRec->num 
                                   * sizeof (unsigned int));
        IcsRec->TrgToSrcMap = local_vis_map;
        local_vis = TRUE;
    } 
    if (IcsRec->TrgToSrcMap)
      for (i=0; i< IcsRec->num;i++)
          IcsRec->TrgToSrcMap[i] = i;


   /* Call the ics routine to convert the logical string into a visual one. */
    ics (IcsRec);
    if (IcsRec->SrcToTrgMap)
      for (i=0; i< IcsRec->num;i++)
         IcsRec->SrcToTrgMap[IcsRec->TrgToSrcMap[IcsRec->TrgToSrcMap[i]]] = 
                                                       IcsRec->TrgToSrcMap[i];

    if (local_vis)
      free (local_vis_map);
    if (NeedLevelMap)
      free (IcsRec->A_level);
    return (0);
}

/***********************************************************************/
