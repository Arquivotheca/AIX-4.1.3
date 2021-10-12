static char sccsid[] = "@(#)43	1.1  src/bos/usr/lib/nls/loc/methods/shared.bidi/groups.c, cfgnls, bos411, 9428A410j 8/30/93 15:02:33";
/*
 *   COMPONENT_NAME: LIBMETH
 *
 *   FUNCTIONS: D_Digit_A
 *		D_Digit_H856
 *		D_Digit_H8859
 *		I_Init_0_A
 *		I_Init_0_H856
 *		I_Init_0_H8859
 *		L_Def_LTR_A
 *		L_Def_LTR_H856
 *		L_Def_LTR_H8859
 *		N_Neutral_A
 *		N_Neutral_H856
 *		N_Neutral_H8859
 *		P_Punct_A
 *		P_Punct_H856
 *		P_Punct_H8859
 *		Q_RSP
 *		Q_RSP_H856
 *		R_Def_RTL_A
 *		R_Def_RTL_H856
 *		R_Def_RTL_H8859
 *		S_Space
 *		X_Prefix_A
 *		X_Prefix_H856
 *		X_Prefix_H8859
 *		Z_Def_0_A
 *		Z_Def_0_H856
 *		Z_Def_0_H8859
 *		swap_A1046
 *		swap_H856
 *		swap_H8859_8
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
/* SOURCE FILE NAME: GROUPS.C                                               */
/*                                                                          */
/* DESCRIPTIVE NAME: Checks whether a certain character is in a certain     */ 
/*                   group or not.                                          */
/*                                                                          */
/* FUNCTION:  This module contains the routines that define each of the     */
/*            character groups needed in the ICS for each code page.        */
/*                                                                          */
/* ENTRY POINTS: I_Init_0_A                                                 */
/*               I_Init_0_H856                                              */
/*               I_Init_0_H8859                                             */
/*               Z_Def_0_A                                                  */
/*               Z_Def_0_H856                                               */
/*               Z_Def_0_H8859                                              */
/*               R_Def_RTL_A                                                */
/*               R_Def_RTL_H856                                             */
/*               R_Def_RTL_H8859                                            */
/*               L_Def_LTR_A                                                */
/*               L_Def_LTR_H856                                             */
/*               L_Def_LTR_H8859                                            */
/*               D_Digit_A                                                  */
/*               D_Digit_H856                                               */
/*               D_Digit_H8859                                              */
/*               X_Prefix_A                                                 */
/*               X_Prefix_H856                                              */
/*               X_Prefix_H8859                                             */
/*               P_Punct_A                                                  */
/*               P_Punct_H856                                               */
/*               P_Punct_H8859                                              */
/*               N_Neutral_A                                                */
/*               N_Neutral_H856                                             */
/*               N_Neutral_H8859                                            */
/*               S_Space                                                    */
/*               Q_RSP                                                      */
/*               Q_RSP_H856                                                 */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/


#define TRUE 1
#define FALSE 0
/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: I_Init_0                                               */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the Initial level-0 character groups.          */
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            For Arabic, there is no Initial Level-0 characters. For       */
/*            Hebrew, it contains the Tab character for PC code pages.      */
/*                                                                          */
/*  ENTRY POINT: I_Init_0                                                   */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/


         /* Characters from this group force level 0. */
int I_Init_0_A (x)
int x;
{
  return (FALSE);
}

int I_Init_0_H856 (x)
int x;
{
            /* Controls */
  if ((x == 0x09) || (x == 0x0a))
          return (TRUE);
  return (FALSE);
}

int I_Init_0_H8859 (x)
int x;
{
  if ((x == 0x09) || (x == 0x0a))
            /* Controls */
          return (TRUE);
  return (FALSE);
}

/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: Z_Def_0                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the Definite level-0 character groups.         */
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            This group includes all controls and synonyms.                */ 
/*                                                                          */
/*  ENTRY POINT: Z_Def_0                                                    */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/


         /* Characters from this group force level 0. */
int Z_Def_0_A (x)
int x;
{
   if ( ((x >= 0x00) && (x <= 0x1f)) || ((x >= 0x88) && (x <= 0x8f)) ) 
                                                        /* Controls */
          return (TRUE);
   return (FALSE);
}

/* GIL, Changed 26 Dec 91, add this function */
int Z_Def_0_H856 (x)
int x;
{
   if ( ((x >= 0x00) && (x <= 0x08)) || ((x >= 0x0b) && (x <= 0x1f)) ||
        (x == 0x7f) || (x == 0x9b) || (x == 0x9d) ||
        ((x >= 0x9f) && (x <= 0xa8)) || (x == 0xad) ||
        ((x >= 0xb5) && (x <= 0xb7)) ||
        ((x >= 0xc6) && (x <= 0xc7)) || ((x >= 0xd0) && (x <= 0xd8)) ||
        (x == 0xde) ||
        ((x >= 0xe0) && (x <= 0xe5)) || ((x >= 0xe7) && (x <= 0xed)) )
          return (TRUE);
   return (FALSE);
}

int Z_Def_0_H8859 (x)
int x;
{
   if ( ((x >= 0x00) && (x <= 0x08)) || ((x >= 0x0b) && (x <= 0x1f)) ||
        ((x >= 0x7f) && (x <= 0x9f)) || (x == 0xa1) ||
        ((x >= 0xbf) && (x <= 0xde)) || ((x >= 0xfb) && (x <= 0xff)))
          return (TRUE);
   return (FALSE);
}

/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: R_Def_RTL                                              */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the Definite right-to-left character groups.   */
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            For Arabic, it contains all arabic alphabetics and arabic-only*/
/*            specials. For Hebrew, it contains the Hebrew alphabet.        */
/*                                                                          */
/*  ENTRY POINT: R_Def_RTL                                                  */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/


          /* These characters have a nesting level of 0
             RTL fields and a level of 1 in LTR ones.  */
int R_Def_RTL_A (x)
int x;
{
   if ( ((x >= 0x80) && (x <= 0x87)) || ((x >= 0x90) && (x <= 0x9f)) || ((x >= 0xA1) && (x <= 0xaf)) || ((x >= 0xBA) && (x <= 0xfe)) )
          return (TRUE);
   return (FALSE);
}

int R_Def_RTL_H856 (x)
int x;
{
   if  ((x >= 0x80) && (x <= 0x9a))
          return (TRUE);
   return (FALSE);
}

int R_Def_RTL_H8859 (x)
int x;
{
   if  ((x >= 0xe0) && (x <= 0xfa))
          return (TRUE);
   return (FALSE);
}


/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: L_Def_LTR                                              */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the Definite left-to-right character groups.   */
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            It contains all english alphabetics and english-only specials.*/
/*                                                                          */
/*  ENTRY POINT: L_Def_LTR                                                  */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/


          /* These characters have a nesting level of 0
             LTR fields and a level of 1 in RTL ones.  */
int L_Def_LTR_A (x)
int x;
{
   if ( ((x >= 65) && (x <= 90)) || (x==59) || (x==38) ||
         (x == 64) || (x==63) || ((x >= 97) && (x <= 122)) )
          return (TRUE);
   return (FALSE);
}

/* GIL, Changed 26 Dec 91, add this function */
int L_Def_LTR_H856 (x)
int x;
{
   if ( ((x >= 0x40) && (x <= 0x5a)) || ((x >= 0x61) && (x <= 0x7a)) ||
        (x == 0x26)) 
          return (TRUE);
   return (FALSE);
}

int L_Def_LTR_H8859 (x)
int x;
{
   if ( ((x >= 0x40) && (x <= 0x5a)) || ((x >= 0x61) && (x <= 0x7a)) ||
        (x == 0x26)) 
          return (TRUE);
   return (FALSE);
}


/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: D_Digit                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the Digit character groups.                    */
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            For Arabic, it contains both english and arabic numerics. For */
/*            Hebrew, it contains the digits, currency symbols, the number  */
/*            sign and the percent symbol.                                  */
/*                                                                          */
/*  ENTRY POINT: D_Digit                                                    */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/

     
    /* These characters have a nesting level of 1 in RTL fields and can
      create a level-2 segment in LTR ones if contained in arabic context. */
          /* It contains both english and arabic numerics. */    
int D_Digit_A (x)
int x;
{
   if ( ((x >= 48) && (x <= 57)) || ((x >= 0xb0) && (x <= 0xb9)) )
          return (TRUE);
   return (FALSE);
}

/* GIL, Changed 30 Dec 91 */
int D_Digit_H856 (x)
int x;
{
   if ( ((x >= 0x23) && (x <= 0x25)) || ((x >= 0x30) && (x <= 0x39)) ||
        (x == 0x9c) || (x == 0xcf) || (x == 0xbd) || (x == 0xbe) ||
        ((x >= 0xab) && (x <= 0xac)) || (x == 0xf3) || (x == 0xf8) ||
        ((x >= 0xfb) && (x <= 0xfd)) )
          return (TRUE);
   return (FALSE);
}

/* GIL, Changed 30 Dec 91 */
int D_Digit_H8859 (x)
int x;
{
   if ( ((x >= 0x23) && (x <= 0x25)) || ((x >= 0x30) && (x <= 0x39)) ||
        ((x >= 0xa2) && (x <= 0xa5)) || ((x >= 0xb2) && (x <= 0xb3)) ||
        (x == 0xb9) || ((x >= 0xbc) && (x <= 0xbe)) || (x == 0xb0))
          return (TRUE);
   return (FALSE);
}

/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: X_Prefix                                               */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the numeric prefixes/suffixes character groups.*/
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            For Arabic, it contains the english comma and period. For     */
/*            Hebrew, it contains Minus and plus.                           */
/*                                                                          */
/*  ENTRY POINT: X_Prefix                                                   */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/


            /* These characters may start a numeric token
           and get the same level as the following number. */
int X_Prefix_A (x)
int x;
{
   if ((x == 44) || (x == 46))
          return (TRUE);
   return (FALSE);
}

int X_Prefix_H856 (x)
int x;
{
   if ((x == 0x2b) || (x == 0x2d) || (x == 0xf1))
          return (TRUE);
   return (FALSE);
}

int X_Prefix_H8859 (x)
int x;
{
   if ((x == 0x2b) || (x == 0x2d) || (x == 0xb1))
          return (TRUE);
   return (FALSE);
}


/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: P_Punct                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the numeric punctuation character groups.      */
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            For Arabic, this group contains the english colon. For Hebrew,*/
/*            it includes the colon, asterisk, comma, dot symbol, slash and */
/*            equal.                                                        */
/*                                                                          */
/*  ENTRY POINT: P_Punct                                                    */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/

  
  /* These characters may appear within and continue a numeric token. The
 character gets the same level as the surrounding number (on both sides). */
int P_Punct_A (x)
int x;
{
   if (x == 58)
          return (TRUE);
   return (FALSE);
}

/* GIL, Changed 30 Dec 91 */
int P_Punct_H856 (x)
int x;
{
  if ((x==0x2a) || (x==0x2c) || (x==0x2e) ||
      (x==0x2f) || (x==0x3a) || (x==0x3d) ||
      ((x == 0x9e) || (x == 0xf6)))
          return (TRUE);
   return (FALSE);
}

int P_Punct_H8859 (x)
int x;
{
  if ((x==0x2a) || (x==0x2c) || (x==0x2e) ||
      (x==0x2f) || (x==0x3a) || (x==0x3d) ||
      (x==0xaa) || (x==0xba) )
          return (TRUE);
   return (FALSE);
}


/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: N_Neutral                                              */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the neutrals character groups.                 */
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            It contains all other characters that are not in the other    */
/*            groups and could belong to both national language and english */
/*            text. For arabic, this group includes the remaining symbols,  */
/*            slash, cent, dollar, number sign and percent sign. For Hebrew,*/
/*            it includes all remaining special symbols, except the space   */
/*            and required space.                                           */
/*                                                                          */
/*  ENTRY POINT: N_Neutral                                                  */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/

          
        /* These characters get a level of 1 in LTR fields
             and create a pending string in RTL fields. */
int N_Neutral_A (x)
int x;
{
   if ( ((x >= 33) && (x <= 37)) || ((x >= 39) && (x <= 43)) || (x == 45) || (x == 47) || ((x >= 60) && (x <= 62)) || ((x >= 91) && (x <= 96)) || ((x >= 123) && (x <= 126)) )
          return (TRUE);
   return (FALSE);
}

/* GIL, Changed 30 Dec 91 */
int N_Neutral_H856 (x)
int x;
{
   if ( (x==0x21) || (x==0x22) || ((x >= 0x27) && (x <= 0x29)) ||
       ((x >= 0x3b) && (x <= 0x3c)) || ((x >= 0x3e) && (x <= 0x3f)) || 
       ((x >= 0x5b) && (x <= 0x60)) || ((x >= 0x7b) && (x <= 0x7e)) ||
       (x == 0xa9) || (x == 0xaa) || (x == 0xae) || (x == 0xaf) ||
       (x == 0xb8) ||
       (x == 0xdd) || (x == 0xe6) || (x == 0xee) || (x == 0xef) ||
       (x == 0xf0) || (x == 0xf2) || ((x >= 0xf4) && (x <= 0xf5)) ||
       (x == 0xf7) || (x == 0xf9) || (x == 0xfa) || (x == 0xfe) )
          return (TRUE);
   return (FALSE);
}

/* GIL, Changed 30 Dec 91 */
int N_Neutral_H8859 (x)
int x;
{
   if ( (x==0x21) || (x==0x22) || ((x >= 0x27) && (x <= 0x29)) ||
       ((x >= 0x3b) && (x <= 0x3c)) || ((x >= 0x3e) && (x <= 0x3f)) || 
       ((x >= 0x5b) && (x <= 0x60)) || ((x >= 0x7b) && (x <= 0x7e)) ||
       ((x >= 0xa6) && (x <= 0xa9)) || ((x >= 0xab) && (x <= 0xaf)) || 
       ((x >= 0xb4) && (x <= 0xb8)) || (x == 0xbb) || (x == 0xdf) )
          return (TRUE);
   return (FALSE);
}


/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: S_Space                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the space character groups.                    */
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            It contains the space only.                                   */
/*                                                                          */
/*  ENTRY POINT: S_Space                                                    */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/

          
        /* This character behaves like neutrals but is a special 
        case when following RSP where the current state is set to 0. */
int S_Space (x)
int x;
{
   if (x == 32)
          return (TRUE);
   return (FALSE);
}


/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: Q_RSP                                                  */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the required space character groups.           */
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            It contains the required space only.                          */
/*                                                                          */
/*  ENTRY POINT: Q_RSP                                                      */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/

      
      /* This character is a level stabilizer. When it follows       
        a space it causes the current state to be set to 0. */
int Q_RSP (x)
int x;
{
  if (x == 0xa0)                /* 64 @ 160 */
          return (TRUE);
  return (FALSE);
}
 
int Q_RSP_H856 (x)
int x;
{
  if (x == 0xff)                /* 64 @ 160 */
          return (TRUE);
  return (FALSE);
}
 

/**************************** START OF SPECIFICATIONS ***********************/
/*                                                                          */
/*  SUBROUTINE NAME: Swaps                                                  */
/*                                                                          */
/*  DESCRIPTIVE NAME: Checks the Directional character groups.              */
/*                                                                          */
/*  FUNCTION: This function is called when we need to check whether a       */
/*            certain character belongs to this group. It returns TRUE      */ 
/*            when the character belongs to that group and FALSE otherwise. */ 
/*            It contains the directional characters like brackets, braces, */  
/*            parentheses and greater than/less than signs.                 */ 
/*                                                                          */
/*  ENTRY POINT: Swaps                                                      */
/*      LINKAGE: called from GetGroup (ics.c)                               */
/*                                                                          */
/*  INPUT: x            int           The character to be checked           */
/*                                                                          */
/************************** END OF SPECIFICATIONS ***************************/

           
          /* These characters behave like neutrals but they
      might be changed to their symmetric symbols in arabic fields. */ 
int swap_A1046 (x)
int x;
{
   switch (x)
   {
      case 40: return (41);
      case 41: return (40);
      case 60: return (62);
      case 62: return (60);
      case 91: return (93);
      case 93: return (91);
      case 123: return (125);
      case 125: return (123);
      case 140: return (141);
      case 141: return (140);
      case 142: return (143);
      case 143: return (142);
      default : return x;
   }
}

/* GIL, Changed 26 Dec 91, fix the switch */
int swap_H856 (x)
int x;
{
   switch (x)
   {
      case 40: return (41);
      case 41: return (40);
      case 60: return (62);
      case 62: return (60);
      case 91: return (93);
      case 93: return (91);
      case 123: return (125);
      case 125: return (123);
      case 191: return (218);
      case 218: return (191);
      case 192: return (217);
      case 217: return (192);
      case 187: return (201);
      case 201: return (187);
      case 174: return (175);
      case 175: return (174);
      case 188: return (200);
      case 200: return (188);
      case 185: return (204);
      case 204: return (185);
      case 195: return (180);
      case 180: return (195);
      default : return x;
   }
}

/* GIL, Changed 26 Dec 91, add this function */
int swap_H8859_8 (x)
int x;
{
   switch (x)
   {
      case 40: return (41);
      case 41: return (40);
      case 60: return (62);
      case 62: return (60);
      case 91: return (93);
      case 93: return (91);
      case 123: return (125);
      case 125: return (123);
      case 171: return (187);
      case 187: return (171);
      default : return x;
   }
}
