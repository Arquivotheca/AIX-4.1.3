static char sccsid[] = "@(#)37	1.1  src/bos/usr/lib/nls/loc/imk/hgcomp/hgcomp.c, libkr, bos411, 9428A410j 5/25/92 15:35:37";
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		hgcomp.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/*			 MODULE DESCRIPTION			             */ 
/*****************************************************************************/
/*									     */
/*   Module Name      :  HG_Composer				             */
/*									     */
/*   Calling Sequence :  HG_Composer(State, String, HG_Char_Buff, kimed)           */
/*									     */
/*   Description      :  - This routine is HanGeul Automata to generate KS   */
/*                         HG Code.                                          */
/*                       - State is HG Composing state                       */
/*                       - "String" contains KS code for HanGeul Jamo input. */
/*                       - This routine generates KS HG code and put them in */
/*                         HG_Char_Buff.                                     */
/*   Return value     :  generated char length in bytes                      */
/*                             0 : input jamo is not valid for HG composing  */
/*                             2 : interim char is generated                 */
/*                             4 : final char and interim char are generated */
/*									     */
/*   Include file	:  "hgcomptbl.h"                                      */
/*		           "hgcomp.h"                                       */
/*									     */
/*****************************************************************************/

/************************************************************************/
/*		      Header file definition				*/
/************************************************************************/

#include "hgcomptbl.h"    /* KS HG code table             */
#include "hgcomp.h"     /* constants and tables         */
#include "ked.h"

static    int   Curr_Hg      ;
static    int   Pre_Hg       ;
static    int   Curr_Com_Hg  ;
static    int   Dup_Flag     ;
          int   Jamo_Attr    ;
          int   Input_Jamo   ;    
          int   Length       ;
          int   Err_Flag     ; 
          int   *State       ;
          char  *HG_Char_Buff ; 
	  KIMED *kimed;

/************************************************************************/
/*			Starting of Main Module 			*/
/************************************************************************/

int hg_composer(int *Comp_State, char *String, char *Buff, KIMED *h_kimed)
{
	kimed = h_kimed;
	if (*Comp_State == State_Delete)
	{
		kimed->hg_status_ps -= 1;
		*State = kimed->hg_status_buf[kimed->hg_status_ps].state;
		Curr_Com_Hg = kimed->hg_status_buf[kimed->hg_status_ps].cmpshg;
		Curr_Hg = kimed->hg_status_buf[kimed->hg_status_ps].cmplhg;
		Pre_Hg = kimed->hg_status_buf[kimed->hg_status_ps-1].cmplhg;
        	Dup_Flag = 0 ;
		*Buff = Curr_Hg >> 8  ;  
        	Buff++ ;
        	*Buff = Curr_Hg & 0x00ff  ;
        	Buff++ ;
        	Length = 2 ;
		return(Length) ;
	}
        State = Comp_State ;
        HG_Char_Buff = Buff ;
	Err_Flag = 0 ;			/* Initialize error flag	*/
        Length   = 0 ;
        Input_Jamo = *String << 8  ;    /* first byte of HG Jamo Input  */
        Input_Jamo += (*(String+1) & 0x00ff) ; /*second byte of HG Jamo Input*/
	Jamo_Attr = Jamo_Attr_Tbl[Input_Jamo - Conson_G] ;
					/* Jamo_Atte = HG attribute	*/
	switch(*State)			/* Run current state processor	*/
          {
	    case State_0 :		/* | Case State_0		*/
		   State_0_Machine() ;	/* | |	  Run State_0_Machine	*/
		   break ;		/* | |				*/
	    case State_1 :		/* | Case State_1		*/
		   State_1_Machine() ;	/* | |	  Run State_1_Machine	*/
	       	   break ;		/* | |				*/
	    case State_2_1 :		/* | Case State_2_1		*/
		   State_2_1_Machine() ;/* | |	  Run State_2_1_Machine */
		   break ;		/* | |				*/
	    case State_2_2 :		/* | Case State_2_2		*/
		   State_2_2_Machine() ;/* | |	  Run State_2_2_Machine */
		   break ;		/* | |				*/
	    case State_2_3 :		/* | Case State_2_3		*/
		   State_2_3_Machine() ;/* | |	  Run State_2_3_Machine */
		   break ;		/* | |				*/
	    case State_3_1 :		/* | Case State_3_1		*/
		   State_3_1_Machine() ;/* | |	  Run State_3_1_Machine */
		   break ;		/* | |				*/
	    case State_3_2 :		/* | Case State_3_2		*/
		   State_3_2_Machine() ;/* | |	  Run State_3_2_Machine */
	   } ;				/* End Case			*/
	return(Length) ;		/*				*/
}					/*				*/



/************************************************************************/
/*			Starting of Error()				*/
/************************************************************************/

void
Error()
{
	Err_Flag = 1;			/* Set error flag	           */
        Length = 0 ;                    /* Return value of HG_Composer = 0 */ 
	return ;			
}					
				

/************************************************************************/
/*			Starting of C0_Handler				*/
/************************************************************************/

void
C0_Handler()
{

	int Code_Converter() ;		/* Declaration of Procedure	    */
	void Put_Intrim() ;
	void New_Char() ;

	int Flag ;			/* Declaration of Variables	    */
	int Intrim_Char ;

					/* Starting of procedure	    */
	Flag = Jamo_Attr & C3 ; 	/*  If Jamo_Attr = C3		    */
	if (Flag)			/*  |				    */
	   New_Char() ; 		/*  |	run New_Char		    */
	else				/*  Else (Jamo_ttr = C1 or C2)	    */
	   {				/*  |				    */
	     Curr_Com_Hg &= Ci_V_Null ; /*  |	Make CI:V:CF		    */
	     Curr_Com_Hg |= Cf_Tbl[Input_Jamo - Conson_G];
	     Intrim_Char = Code_Converter(Curr_Com_Hg) ;
	     if (Intrim_Char == 0)	/*  |	If Not in character set     */
		 New_Char() ;		/*  |	|  run New_Char 	    */
	     else			/*  |	Else			    */
	       {			/*  |	|			    */
		if ((Input_Jamo == Conson_GG) || (Input_Jamo == Conson_SS))
					/*  |	|  IF Input_jamo = GG or SS */
		    Dup_Flag = 1 ;	/*  |	|  |  Dup_Dlag = 1	    */
		if (( Jamo_Attr & C1))	/*  |	|  If Jamo_Attr = C1	    */
		   *State = State_3_1 ;	/*  |	|     State = 3.1	    */
		else			/*  |	|  Else 		    */
		   *State = State_3_2 ;	/*  |	|     State = 3.2	    */
		Put_Intrim(Intrim_Char);/*  |	|  Save interim code	    */
		kimed->hg_status_ps += 1 ;
		kimed->hg_status_buf[kimed->hg_status_ps].cmpshg = Curr_Com_Hg ;
		kimed->hg_status_buf[kimed->hg_status_ps].cmplhg = Intrim_Char ;
		   } ;			/*  |	Endif			    */
	     }				/*  Endif			    */
}

/************************************************************************/
/*			Starting of V0_Handler				*/
/************************************************************************/

void
V0_Handler()
{
	int Code_Converter() ;		/* Declaration of Procedures	*/
	void Put_Final() ;
	void Put_Intrim() ;
	void State_Set() ;
	int Ci_Converter() ;

	int Cf ;			/* Declaration of Variables	*/
	int Intrim_Char ;
        int Temp_Hg, D_Cmps_Hg, D_Cmpl_Hg ;
	

        Temp_Hg = Curr_Com_Hg ;
	Cf = Curr_Com_Hg & Cf_Mask ;	/* Get current CF		*/
	Curr_Com_Hg &= Null ;		/*				*/
	Curr_Com_Hg |= C5_Split_Tbl[Cf - 2] ;
					/* Use C5_Split_tbl to get CI	*/
	if (Dup_Flag)			/* If CI = GG or SS		*/
	    Curr_Com_Hg += 0x0400 ;	/*    Code = Code+400h		*/
	D_Cmps_Hg = Curr_Com_Hg ;
	Curr_Com_Hg |= V_Tbl[Input_Jamo - Vowel_A] ;
					/* Use V_Tbl to get V		*/
	Curr_Com_Hg |= Ci_V_Fill ;	/*				*/
	Intrim_Char = Code_Converter(Curr_Com_Hg) ;
        if (Intrim_Char == 0)
           { 
             Error() ;
             Curr_Com_Hg = Temp_Hg ;
             return ;
            }
	Put_Final();			/* Run Put_Final (Pre Hg is final*/
	kimed->hg_status_ps = 0 ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmpshg = D_Cmps_Hg ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmplhg = Ci_Converter(D_Cmps_Hg) ;
	Put_Intrim(Intrim_Char) ;	/* Run Put_Intrim		*/
	kimed->hg_status_ps++ ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmpshg = Curr_Com_Hg ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmplhg = Intrim_Char ;
	State_Set() ;			/* Set state			*/
}					/*				*/

/************************************************************************/
/*			Starting of Code_Converter			*/
/************************************************************************/

Code_Converter(Key)
int Key ;				/* Parameter : Key		*/

{
	int Flag, Found ;		/* Declaration of Variables	*/
	int Index, Low, High ;
	int Code ;

	Flag = 1 ;			/* Initialize Variables 	*/
	Found = 0 ;			/*				*/
	Low = 0 ;			/*				*/
	High = 2349 ;			/*				*/
	Index = High / 2 ;		/*				*/
	while (Flag)			/* While Flag = 1		*/
	    {				/*				*/
		if (Key == Hg_Code_Tbl[Index])
		    {			/*   If found			*/
			Found = 1 ;	/*   |	Set Found flag		*/
			Flag = 0 ;	/*   |	Reset Flag		*/
		     }			/*   |				*/
		else			/*   Else (Not found)		*/
		    {			/*   |				*/
		      if (Key < Hg_Code_Tbl[Index])
					/*   |	Calculate the value of	*/
					/*   |	   High / Low / Index	*/
			  High = Index - 1 ;
		      else
			  Low = Index + 1 ;
		      if (Low > High)	/*   |				*/
			  Flag = 0 ;	/*   |				*/
		      else		/*   |				*/
			  Index = Low + (High - Low) / 2 ;
		     }			/*   |				*/
	      } ;			/*   Endif			*/
	if (Found)			/*   If found			*/
	   {				/*   |				*/
	      int Quo, Rem ;		/*   |	Make KS HG code 	*/
					/*   |				*/
	      Quo = Index / 94 ;	/*   |				*/
	      Rem = Index % 94 ;	/*   |				*/
	      Code = 0xb0a1 + Quo * 16 * 16 + Rem ;
	    }				/*   |				*/
	else				/*   Else			*/
	    {
	      int i, flag ;

	      flag = 0 ;		/* Support for missing interim char */ 
	      for ( i = 0 ; i < Missing_Char_Num ; i++)	/* in KS code       */
		  if (Missing_Char_Tbl[i] == Key)	/* Temporary codes  */
		     {					/* are assigned to  */
			Code = Mchar_Code_Start + i ;	/* them from 0xada1 */
			flag = 1 ;			/* to 0xada5	    */
			break ;
		     }
	      if (!flag)
	      	 Code = 0 ;		/*	Code = 0		*/
	    }
	return(Code) ;			/*   Return			*/
}					/*				*/

/************************************************************************/
/*			Starting of New_Char				*/
/************************************************************************/

void
New_Char()
{
	void Put_Final() ;		/* Declaration of procedures	*/
	int Ci_Converter() ;
	int Intrim_Char ;

					/* Use Ci_Tbl to get CI 	*/
	Curr_Com_Hg = Ci_Tbl[Input_Jamo - Conson_G] ;
	Curr_Com_Hg |= Ci_Fill_Fill ;	/* Curr_Com_Hg = CI:f:f 	*/
					/* Run Ci_Converter		*/
	Intrim_Char = Ci_Converter(Curr_Com_Hg) ;			
	*State = State_1 ;		/* Set State			*/
	Pre_Hg = Curr_Hg ;		/* Finalize current HG		*/
	Put_Final() ;			/* Run Put_Final		*/
	Put_Intrim(Intrim_Char) ;	/* Run Put_Intrim		*/
	kimed->hg_status_ps = 0 ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmpshg = Curr_Com_Hg ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmplhg = Intrim_Char ;
}					/*				*/


/************************************************************************/
/*			Starting of Put_Final				*/
/************************************************************************/

void
Put_Final()
{

	*HG_Char_Buff = Pre_Hg >> 8  ; 	/* Save Pre_hg as a final Char	*/
	HG_Char_Buff++ ; 		/* Increase buffer pointer	*/
        *HG_Char_Buff = Pre_Hg & 0x00ff  ;
        HG_Char_Buff++ ;
	Length += 2 ;		/* increase char length  */
}					/*				*/

/************************************************************************/
/*			Starting of Put_Intrim				*/
/************************************************************************/

void
Put_Intrim( Code )
int Code ;				/* Parameter : Code		*/
{

	*HG_Char_Buff = Code >> 8 ;	/* Save code as a interim	*/
	HG_Char_Buff++ ; 		/* Increase buffer pointer	*/
        *HG_Char_Buff = Code & 0x00ff ;
        HG_Char_Buff++ ;
	Length += 2 ;		        /* increase char length   */
	Pre_Hg = Curr_Hg ;		/* Pre_Hg = Curr_Hg		*/
	Curr_Hg = Code ;		/* Curr_Hg = Current int char	*/
}					/*				*/

/************************************************************************/
/*			Starting of State_0_Machine			*/
/************************************************************************/

State_0_Machine()
{					/* Declaration of procedures	*/
	void   Error() ;
	int   Ci_Converter() ;
	void  Put_Intrim() ;

	int    Flag ;			/* Declaration of Variables	*/
	int    Intrim_Char  ;

        Pre_Hg = 0 ;
        Curr_Hg = 0 ;
        Dup_Flag = 0 ;
	Flag = Jamo_Attr & C0 ; 	/* If Jamo_Attr != C0		*/
	if (Flag == 0)			/* |  Error()			*/
          {
            Error() ;			/* Else (Jamo_Attr = C0)	*/
            return  ;
           }
	Curr_Com_Hg &= Null ;		/* |  Use Ci_Tbl to get CI	*/
	Curr_Com_Hg |= Ci_Tbl[Input_Jamo - Conson_G] ;
	Curr_Com_Hg |= Ci_Fill_Fill ;	/* |  Curr_Com_Hg = Ci:f:f	*/
	Intrim_Char = Input_Jamo  ;
	*State = State_1 ;		/* |  State = 1 		*/
	Put_Intrim(Intrim_Char) ;	/* |  Run Put_Intrim		*/
	kimed->hg_status_ps = 0 ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmpshg = Curr_Com_Hg ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmplhg = Intrim_Char ;
	kimed->hg_status_buf[kimed->hg_status_ps].state = *State;
}					/* Endif			*/



/************************************************************************/
/*			Starting of Ci_Converter			*/
/************************************************************************/

Ci_Converter(Code)
int Code;				/* Parameter : Code		*/
{
	Code &= Ci_Mask ;		/* Converts CI:f:f to KS code	*/
	Code >>= 10 ;			/*				*/
	Code -= 10  ;			/*				*/
	return(Ci_Code_Tbl[Code]);	/*				*/
}					/*				*/


/************************************************************************/
/*			Starting of State_1_Machine			*/
/************************************************************************/

State_1_Machine()
{
	void  Error() ; 		/* Declaration of Procedures	*/
	int Code_Converter() ;
	void State_Set() ;
	void Put_Intrim() ;

	int Flag ;			/* Declaration of Varables	*/
	int Intrim_Char ;
        int Temp_Hg  ;

        Temp_Hg = Curr_Com_Hg ;
	Flag = Jamo_Attr & V0 ; 	/* If Jamo_Flag != V0		*/
	if (Flag == 0)			/* |  Error()			*/
          { 
	    Error() ;			/* Else 			*/
            return  ;
          }
	Curr_Com_Hg &= Ci_Mask ;	/* |  Use V_Tbl to get V	*/
	Curr_Com_Hg |= V_Tbl[Input_Jamo - Vowel_A] ;
	Curr_Com_Hg |= Ci_V_Fill ;	/* |  Curr_Com_Hg = Ci:V:f	*/
					/* |  Run Code_Converter	*/
	Intrim_Char = Code_Converter(Curr_Com_Hg) ;
        if (Intrim_Char ==0)
           {
             Error() ;
             Curr_Com_Hg = Temp_Hg ;
             return ;
            }
	State_Set() ;			/* |  Set State 		*/
	Put_Intrim(Intrim_Char) ;	/* |  Run Put_Intrim		*/
	kimed->hg_status_ps += 1 ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmpshg = Curr_Com_Hg ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmplhg = Intrim_Char ;
	kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
}					/* Endif			*/

/************************************************************************/
/*			Starting of State_Set				*/
/************************************************************************/

void
State_Set()
{
	int Attr ;			/* Declaration of Variable	*/

	Attr = Jamo_Attr & V2 ; 	/* If Jamo_Attr = V2		*/
	if (Attr)			/*    State = 2.1		*/
	   *State = State_2_1 ;		/*				*/
	else				/* Else 			*/
	   {				/*    If Jamo_Attr = V1 	*/
	     Attr = Jamo_Attr & V1 ;	/*				*/
	     if (Attr)			/*	 State = 2.2		*/
		 *State = State_2_2 ;	/*    Else			*/
	     else			/*	 State = 2.3		*/
		 *State = State_2_3 ;	/*    Endif			*/
	    } ; 			/* Endif			*/
}					/*				*/


/************************************************************************/
/*			Starting of State_2_1_Machine			*/
/************************************************************************/

State_2_1_Machine()
{
	void Error() ;			/* Declaration of Procedures	*/
	void C0_Handler() ;
	int  Code_Converter() ;
	void  Put_Intrim() ;

	int Flag ;			/* declaration of Variables	*/
	int Intrim_Char ;
	int Pre_V ;
        int Temp_Hg ;

	Flag = Jamo_Attr & C0 ; 	/* If Jamo_Attr = C0		*/
	if (Flag)			/* |				*/
	   {				/* |  Run C0_Handler		*/
	     C0_Handler() ;		/* |				*/
	     kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
	     return ;			/* |				*/
	   } ;				/* Else 			*/
	Flag = Jamo_Attr & V4 ; 	/* |  If Jamo_Attr != V4	*/
	if (Flag == 0)			/* |  | 			*/
          {                             /* |  |                         */
	    Error() ;			/* |  |  Error()		*/
            return ;
          }  
	Pre_V = Curr_Com_Hg & V_Mask ;	/* |  Else (Jamo_Attr = V4)	*/
	if (Pre_V == Jamo_O)		/* |	 If current V = O	*/
	    switch(Input_Jamo)          /* |     |                      */
		{			/* |	 |			*/
		   case Vowel_A  :	/* |	 |  case 'A'            */
			Curr_Com_Hg += 0x0020 ;
					/* |	 |  | Curr_Com_Hg += 20h*/
			break ; 	/* |	 |  |			*/
		   case Vowel_AE :	/* |	 |  case 'AE'            */
			Curr_Com_Hg += 0x0040 ;
					/* |	 |  | Curr_Com_Hg += 40h*/
			break ; 	/* |	 |  |			*/
		   case Vowel_I :	/* |	 |  case 'I'            */
			Curr_Com_Hg += 0x0080 ;
					/* |	 |  | Curr_Com_Hg += 80h*/
			break ; 	/* |	 |  |			*/
		   default :		/* |	 |  others		*/
			 Error() ;	/* |	 |     Error		*/
                         return  ;      /* |     |                      */
		 }			/* |	 |	                */
	else				/* |	 Else ( Vowel U )       */
	     switch(Input_Jamo) 	/* |	 |			*/
		 {			/* |	 |			*/
		   case Vowel_EO :	/* |	 |  case 'EO'           */
			Curr_Com_Hg += 0x0040 ;
					/* |	 |  | Curr_Com_Hg += 20h*/
			break ; 	/* |	 |  |			*/
		   case Vowel_E :	/* |	 |  case 'E'            */
	        	Curr_Com_Hg += 0x0060 ;
					/* |	 |  | Curr_Com_Hg += 40h*/
			break ; 	/* |	 |  |			*/
		   case Vowel_I :	/* |	 |  case 'I'            */
			Curr_Com_Hg += 0x0080 ;
					/*	 |  | Curr_Com_Hg += 60h*/
			break ; 	/*	 |  |			*/
		   default :		/*	 |  others		*/
			Error() ;	/*	 |     Error		*/
                        return  ;
		 } ;			/*	 Endif			*/
					/*	 Run Code_Converter	*/
	Intrim_Char = Code_Converter(Curr_Com_Hg) ;
        if (Intrim_Char ==0)
           {
             Error() ;
             Curr_Com_Hg = Temp_Hg ;
             return ;
            }
	if ((Input_Jamo == Vowel_A) || (Input_Jamo == Vowel_EO))
					/*	 If Jamo_Attr = V5	*/
	    *State = State_2_2 ; 	/*	 |  State = 2.2 	*/
	else				/*	 Else (Jamo_Attr = V6)	*/
	    *State = State_2_3 ; 	/*	 |  State = 2.3 	*/
					/*	 Endif			*/
	Put_Intrim(Intrim_Char) ;	/*	 Run Put_Intrim 	*/
	kimed->hg_status_ps += 1 ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmpshg = Curr_Com_Hg ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmplhg = Intrim_Char ;
	kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
	
}					/*				*/


/************************************************************************/
/*			Starting of State_2_2_Machine			*/
/************************************************************************/

State_2_2_Machine()
{
	void Error() ;			/* Declaration of Procedures	*/
	void  C0_Handler() ;
	int  Code_Converter() ;
	void  Put_Intrim() ;

	int Flag ;			/* Declaration of Variables	*/
	int Pre_V ;
	int Intrim_Char ;
        int Temp_Hg ;

	Flag = Jamo_Attr & C0 ; 	/* If Jamo_Attr = C0		*/
	if (Flag)			/*				*/
	   {				/*    Run Co_Handler		*/
	     C0_Handler() ;		/*				*/
	     kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
	     return ;			/* Else 			*/
	    } ; 			/*    If Input_Jamo != '|'      */
	if ( Input_Jamo != Vowel_I )	/*	 Error			*/
          {
	   Error() ;			/*    Else			*/
           return  ;
          }
	Pre_V = Curr_Com_Hg & V_Mask ;	/*	 If Pre_V = 'f'         */
	if (Pre_V == Jamo_EO)		/*	    Curr_Com_Hg += 40h	*/
	    Curr_Com_Hg += 0x0040 ;	/*	 Else			*/
	else				/*	    Curr_Com_Hg += 20h	*/
	    Curr_Com_Hg += 0x0020 ;	/*	 Endif			*/
					/*	 Run Code_Converter	*/
	Intrim_Char = Code_Converter(Curr_Com_Hg) ;
        if (Intrim_Char ==0)
           {
             Error() ;
             Curr_Com_Hg = Temp_Hg ;
             return ;
            }
	*State = State_2_3 ;		/*	 State = 2.3		*/
	Put_Intrim(Intrim_Char) ;	/*	 Run Put_Intrim 	*/
	kimed->hg_status_ps += 1 ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmpshg = Curr_Com_Hg ;
	kimed->hg_status_buf[kimed->hg_status_ps].cmplhg = Intrim_Char ;
	kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
}					/*     Endif			*/
					/*  Endif			*/

/************************************************************************/
/*			Starting of State_2_3_Machine			*/
/************************************************************************/

State_2_3_Machine()
{
	void Error() ;			/* Declaraion of Procedures	*/
	void  C0_Handler() ;

	int Flag ;			/* Declaration of Variable	*/

	Flag = Jamo_Attr & C0 ; 	/* If Jamo_Attr != C0		*/
	if (Flag == 0)			/*    Error			*/
          {
	    Error() ;			/* Else (Jamo_Attr = C0)	*/
            return  ;
          }
	C0_Handler() ;			/*    Run C0_Handler		*/
	kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
}					/* Endif			*/



/************************************************************************/
/*			Starting of State_3_1_Machine			*/
/************************************************************************/

State_3_1_Machine()
{
	void Error() ;			/* Declaration of Procedures	*/
	void V0_Handler() ;
	void New_Char() ;
	void Put_Intrim() ;
	int  Code_Converter() ;

	int Flag ;			/* Declaration of Variables	*/
	int Intrim_Char, Pre_C ;

	Flag = Jamo_Attr & V0 ; 	/* If Jamo_Attr = V0		*/
					/* |				*/
	if (Flag)			/* |				*/
	{				/* |  Run V0_Handler		*/
	    V0_Handler() ;		/* |				*/
	    kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
	    return ;			/* Else 			*/
	} ;				/* |				*/
	Flag = Jamo_Attr & C4 ; 	/* |				*/
					/* |  If Jamo_Attr != C4	*/
	if (Flag == 0)			/* |  | 			*/
	{				/* |  | 			*/
	    New_Char() ;		/* |  |  Run New_Char		*/
	    kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
	    return ;			/* |  |  Return 		*/
	}				/* |  Else			*/
	Pre_C = Curr_Com_Hg & Cf_Mask ; /* |  |  If Current CF = G	*/
	if (Pre_C == Jamo_F_G)		/* |  |  |			*/
	{				/* |  |  |  If Input_Jamo = G	*/
	    if (Input_Jamo == Conson_G) /* |  |  |  |  Curr_Com_Hg += 1 */
		Curr_Com_Hg += 0x0001 ; /* |  |  |  Elseif Input_Jamo = */
	    else if (Input_Jamo == Conson_S)      
		Curr_Com_Hg += 0x0002 ; /* |  |  |  |	Curr_Com_Hg += 2*/
	    else {			/* |  |  |  Else		*/
		New_Char() ;		/* |  |  |  |	Run New_Char	*/
		kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
		return; 		/* |  |  |  |	Return		*/
	    }				/* |  |  |  Endif		*/
	}				/* |  |  Elseif 		*/
	else if (Pre_C == Jamo_F_N)	/* |  |  |  Current CF = N	*/
	{				/* |  |  |			*/
	     if (Input_Jamo == Conson_J)/* |  |  |  If Input_Jamo = J	*/
		  Curr_Com_Hg += 0x0001;/* |  |  |  |  Curr_Com_Hg +=1	*/
	     else if (Input_Jamo == Conson_H)
					/* |  |  |  Elseif H		*/
		  Curr_Com_Hg += 0x0002;/* |  |  |  |  Curr_Com_Hg +=2	*/
	     else			/* |  |  |  Else		*/
	     {				/* |  |  |  |			*/
		 New_Char() ;		/* |  |  |  |  Run New_Char	*/
		 kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
		 return ;		/* |  |  |  |  Return		*/
	     }				/* |  |  |  Endif		*/
	}				/* |  |  Elseif 		*/
	else if ((Pre_C == Jamo_F_B) || (Pre_C == Jamo_F_S))	
					/* |  |  |  Current CF = B or S	*/
	{				/* |  |  |			*/
	    if (Input_Jamo == Conson_S) /* |  |  |  If Input_Jamo = S	*/
		Curr_Com_Hg += 0x0001 ; /* |  |  |  |	Curr_Com_Hg +=	 */
	    else			/* |  |  |  Else		*/
	    {				/* |  |  |  |			*/
		New_Char() ;		/* |  |  |  |  Run New_Char	*/
		kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
		return ;		/* |  |  |  |  Return		*/
	    }				/* |  |  |  Endif		*/
	}				/* |  |  Elseif 		*/
	else if (Pre_C == Jamo_F_L)	/* |  |  |  Current CF = L	*/
	{				/* |  |  |			*/
	     int Flag = 1 ;		/* |  |  |			*/
	     int I ;			/* |  |  |			*/
					/* |  |  |			*/
	     for (I=0 ; (I <= 6) && Flag; I++)
	     {				/* |  |  |     For I = 1 to 6	*/
		 if (Input_Jamo == L_Tbl[I])  
					/* |  |  |	 If combinable	*/
		     Flag = 0;		/* |  |  |	 |  Flag = 0	*/
	     } ;			/* |  |  |	 Endif		*/
	     if (Flag)			/* |  |  |     If Not Combinable*/
	     {				/* |  |  |     |		*/
		 New_Char() ;		/* |  |  |     |  Run New_Char	*/
		 kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
		 return ;		/* |  |  |     |  Return	*/
	     }				/* |  |  |     Else		*/
	     else			/* |  |  |     |  Make CI:V:new */
		 Curr_Com_Hg  +=  I ;	/* |  |  |     Endif		*/
	}				/* |  |  |			*/
	else				/* |  |  Else			*/
	{				/* |  |  |			*/
	     New_Char() ;		/* |  |  |     Run New_Char	*/
	     kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
	     return ;			/* |  |  |     Return		*/
	}				/* |  |  Endif			*/
					/* |  |    Run Code_Converter	*/
	Intrim_Char = Code_Converter(Curr_Com_Hg) ;
	if (Intrim_Char == 0)		/* |  |  If Not in Char set	*/
	{
	    New_Char() ;		/* |  |  |   Run New_Char	*/
	    kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
	}
	else				/* |  |  Else			*/
	{				/* |  |  |			*/
	    *State = State_3_2 ; 	/* |  |  |   State = 3.2	*/
	    Put_Intrim(Intrim_Char) ;	/* |  |  |   Run Put_Intrim	*/
	    kimed->hg_status_ps += 1 ;
	    kimed->hg_status_buf[kimed->hg_status_ps].cmpshg = Curr_Com_Hg ;
	    kimed->hg_status_buf[kimed->hg_status_ps].cmplhg = Intrim_Char ;
	    kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
	}				/* |  |  Endif			*/
}					/* Endif			*/

/************************************************************************/
/*			Starting of State_3_2_Machine			*/
/************************************************************************/

State_3_2_Machine()
{
	void Error() ;			/* Declaration of Procedures	*/
	void New_Char() ;
	void V0_Handler() ;

	int Flag ;			/* Declaration of Variables	*/
					/*				*/
	Flag = Jamo_Attr & C0 ; 	/* If Jamo_Attr C0		*/
	if (Flag)			/*    Run New_Char		*/
	    New_Char() ;		/* Else 			*/
	else				/*    Run V0_Handler		*/
	    V0_Handler() ;		/* Endif			*/
	Dup_Flag &= Null ;		/* Reset Dup_Flag		*/
	kimed->hg_status_buf[kimed->hg_status_ps].state = *State ;
}					/*				*/
