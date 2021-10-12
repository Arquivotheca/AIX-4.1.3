static char sccsid[] = "@(#)46	1.2  src/bos/usr/lib/nls/loc/ZH.im/zhedabc.c, ils-zh_CN, bos41J, 9509A_all 2/25/95 13:38:47";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: AUTO_disp
 *		AbcCandidates
 *		AbcCloseAux
 *		AbcCls
 *		AbcFilter
 *		AbcFreeCandidates
 *		AbcInitial
 *		AbcMain
 *		AbcSelect
 *		AbcSetOption
 *		AbcShowCursor
 *		AbcVProc
 *		Analize
 *		BackwordProc
 *		CinoPunctuation
 *		CurPos
 *		DelAndReconvert
 *		DispAutoSelect
 *		DispSpecChar
 *		FindCharByWord
 *		HalfInit
 *		IfAlphOrNot
 *		IfFirstKey
 *		IfNumberOrNot
 *		IfPunct
 *		InputInit
 *		ListBox
 *		MessageBeep
 *		MoveResult
 *		OutResult
 *		SameAsBackwords
 *		SendMsg
 *		SendOneChar
 *		ShowChar
 *		ShowString
 *		SubFilter
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************
	The ABC convert steps mainly are:

				|
				v
		 -------------------------------------
		 | send message into input buffer    |
		 -------------------------------------
				|
				v
		 --------------------------------------
		 |      analyze the input string      |
		 --------------------------------------
				|
				v
		 --------------------------------------
		 |      convert the input string      |
		 --------------------------------------
				|
				v
	       ------------------------------------------
	       | display the auto and reference results |
	       ------------------------------------------
				|
				v
		 ---------------------------------------
		 |     wait for selecting the result   |
		 ---------------------------------------
				|
				v
		 ---------------------------------------
		 |      send the result to main screen |
		 ---------------------------------------

*************************************************************************/

/************************ START OF MODULE SPECIFICATION ***************** *****/
/*                                                                            */
/* MODULE NAME:        zhedabc                                                */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method For AbcMain Mode                  */
/*                                                                            */
/* FUNCTION:           AbcMain           : Entry Point Of Abc Input Method    */
/*                                                                            */
/*                     AbcFilter         : Abc Input Method filter            */
/*                                                                            */
/*                     AbcInitial        : Initial ABC mothed                 */
/*                                                                            */
/*                     AbcCls            : Erase All Radicals                 */
/*                                                                            */
/*                     AbcCandidates     : Find Satisfied Candidates          */
/*                                                                            */
/*                     AbcSelect         : Process Input Key On Cand. ListBox */
/*                                                                            */
/*		       MoveResult	 : Move the result to the result area */
/*                                                                            */
/*		       OutResult         : Send the seleted result to screen  */
/*                                                                            */
/*		       DispAutoSelect    : Send the seleted result to result  */
/*					   display area                       */
/*                                                                            */
/*		       SubFilter()	 : Send the string received from the  */
/*					   keyboard to the in.buffer          */
/*									      */
/*		       Analize()	 : Process the biaodian,and analize   *//*                                         the first byte of the input        */
/*                                         information                        */
/*									      */
/*		       IfFirstKey        : Adjust if the correct first key    */
/*									      */
/*		       IfAlphOrNot()	 : judge if the input is Aph char     */
/*									      */
/*		       IfNumberOrNot	 : judge if the input is number char  */
/*									      */
/*		       IfPunct()	 : judge if the input is a punctua-   */
/*					 : tion char			      */
/*		       DelAndReconvert() : Process the back space key         */
/*                                                                            */
/*		       BackwordProc()    : Process the "alt+'-'" function     */
/*									      */
/*		       SameAsBackwords() : Redisplay the in.buffer in         */
/*                                         "alt+'-'"                          */
/*									      */
/*		       FindCharByWord()  : Get a char by word use "]", "["    */
/*									      */
/*		       CinoPunctuation() : Process the chinese punctuation    */
/*									      */
/*		       AbcShowCursor     : For show cursor.                   */
/*                                                                            */
/*		       InputInit()       : Erase the display area, and give   */
/*					   the init_value to paraments.       */
/*									      */
/*                     HalfInit          :Initialize parameters for input     */
/*                                                                            */
/*                     ListBox           : Process PageUp/PageDown On List Box*/
/*                                                                            */
/*                     AbcCloseAux       : Close Candidate List Box           */
/*                                                                            */
/*		       DispSpecChar      : Display a string of one special    */
/*                                         char                               */
/*									      */
/* 		       ShowChar          : Send/Change input char to echo buf.*/
/*									      */
/*		       ShowString        : Send/Change string to echo buf.    */
/*                                                                            */
/*		       CurPos            : Set cursor position                */
/*									      */
/* 		       SendMsg           : Give out the selected results.     */
/*                                                                            */
/*                     SendOneChar       : Give out one char                  */
/*                                                                            */
/*                     MessageBeep       : Make a beep and display message on */
/*					   indicator line                     */
/*									      */
/* MODULE TYPE:        C                                                      */
/*                                                                            */
/* COMPILER:           AIX C                                                  */
/*                                                                            */
/* AUTHOR:             Tang Bosong, WuJian                                    */
/*                                                                            */
/* STATUS:             Chinese Input Method Version 1.0                       */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/
#include "locale.h"
#include "cjk.h"
#include  "zhed.h"
#include  "zhedacc.h"
#include  "chinese.h"
#include "data.h"
#include "abc.h"
#include "extern.h" 
#include "zhedud.h"

extern CwpProc(int);
extern SendBackMsg();
extern TempRemProc();
extern PinduAdjust();

#define UNC 5
BYTE kb_buffer[35];
int end_flg;
int input_count=0;
BYTE msg_bf[32];
int msg_count=0;
unsigned short last_keep = 0;

/******************************************************************************/
/* FUNCTION    : AbcMain                                                      */
/* DESCRIPTION : Entry Point of ABC Input Method.                             */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

AbcMain(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{

/*    fep = fepcb;                */
    if ( last_keep != 0 )
        AbcFilter(fepcb, last_keep);
    if (( step_mode == SELECT) || (step_mode == RESELECT))
    {
        AbcFilter(fepcb, key);
        return;
    }
    if ( step_mode == SETOPTION )       /* Set ABC Input Method Options      */
    {
        AbcSetOption(fepcb, key);
        return;
    }

    switch( key )
    {
        case ALPHA_NUM_KEY :
            AbcFreeCandidates(fepcb);   /* Free Abc Candidates Buffer         */
            AnInitial(fepcb);           /* Initialize Alph_Num Input Method   */
            break;

        case PINYIN_KEY :
            AbcFreeCandidates(fepcb);   /* Free Abc Candidates Buffer         */
            PyInitial(fepcb);           /* Initialize Pinyin Input Method     */
            break;

        case ENGLISH_CHINESE_KEY :
            AbcFreeCandidates(fepcb);   /* Free Abc Candidates Buffer         */
            EnInitial(fepcb);           /* Initialize English_Chinese Input   */
                                        /* Method                             */
            break;

        case ABC_KEY :
            AbcFreeCandidates(fepcb);   /* Free Abc Candidates Buffer         */
            AbcInitial(fepcb);          /* Initialize ABC Input Method        */
            break;

        case USER_DEFINED_KEY :
            AbcFreeCandidates(fepcb);   /* Free Abc Candidates Buffer         */
            if(udimcomm->ret == UD_FOUND)
               (*udimcomm->UserDefinedInitial)(udimcomm); 
                                        /* Initialize User_Defined Input      */
                                        /* Method                             */
            else
               return;
            break;

        case TSANG_JYE_KEY :
            AbcFreeCandidates(fepcb);   /* Free Abc Candidates Buffer         */
            TjInitial(fepcb);           /* Initialize Row_Column Input Method */
            break;

        case FULL_HALF_KEY :            /* Set FULL/HALF Mode                 */
            fepcb->indchfg = TRUE;

            if( fepcb->inputlen > 0 )
            {
                fepcb->imode.ind5 = ERROR1;
                fepcb->isbeep = ON;
                break;
            }

            if( fepcb->imode.ind1 == HALF )
                fepcb->imode.ind1 = FULL;
            else
                fepcb->imode.ind1 = HALF;
            break;

        case IMED_SET_OPTION_KEY :     /* Set ABC Input Method Option        */
            AbcSetOption(fepcb, key);
            break;

        default :
            AbcFilter(fepcb, key);      /* Process Abc Input Method          */
            break;
    }
    return;
}

/******************************************************************************/
/* FUNCTION    : AbcFilter                                                    */
/* DESCRIPTION : Filters the Abc radicals and special keys and goes into      */
/*               appropriatly process.                                        */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

AbcFilter(fepcb, key)
FEPCB           *fepcb;                /* FEP Control Block                  */
unsigned short  key;                   /* Ascii Code                         */
{
    int            leibie;             /* What kind the sting is             */
    int    input_char_type;            /* What kind the key is               */

    last_keep = 0;
    if (fepcb->imode.ind1 == FULL)
    {
       AnProcess(fepcb, key);
       return(0);
    }

    switch(key)
    {
        case BACKWORD_KEY:
            return(BackwordProc(fepcb));
    
        case FORWORD_KEY:
            if ((step_mode!=ONINPUT)&&(group_no>1))
                step_mode=SELECT;
            return(0);
    
        case INSERT_KEY :
            if( fepcb->imode.ind4 == INSERT_MODE )
                fepcb->imode.ind4 = REPLACE_MODE;
            else
                fepcb->imode.ind4 = INSERT_MODE;
            return(0);
    }
    

    switch(step_mode)            /* step_mode indicate the input step        */
    {                            /* step_mode take this value:               */
                                 /*    START=0  first input Abc              */
                                 /*    SELECT=1 can select/cansel the result */
                                 /*    RESELECT=2 use FORCE SELECT KEY       */
                                 /*    ONINPUT=3 just in inputing step       */
        case SELECT:
    
            switch(key)
            {
                case CONVERT_KEY:
                case RETURN_KEY:
                    if(!OutResult(fepcb))   /* Result Has Been Sent to Screen */
                        step_mode=RESELECT;      
                    return 0;

        
                case ESC_KEY:
                    AbcCls(fepcb);
                    if ( fepcb->auxuse == USE )
                        AbcCloseAux(fepcb);
                    step_mode=START;
                    return 0;

                case BACK_SPACE_KEY:
                    if (word_back_flag!=0x55)
                        DelAndReconvert(fepcb);   /* Produce BACKSPACE     */
                    return 0;
        
                default:
                    if (IfAlphOrNot(key))         /* Key is An Alphabet   */
                    {
                        while(OutResult(fepcb));  /* Send Convert Results */
                        step_mode=START;
                        last_keep = key;
                        return(0);
                    }
                    else
                    {
                        AbcSelect(fepcb,key);    /* Select A Candidate   */
                        return(0);
                    }
            } 

        case RESELECT:
        case START:
    
            if (( fepcb->auxuse == USE ) &&      /* RESELECT Mode       */
                  (((HOME_KEY <= key )&& (key <=PGDN_KEY)) 
                  || (key == ESC_KEY) || (key >= 0xFF00)))
            {
                if (key >= 0xFF00)
                    key &= 0xFF;
                AbcSelect(fepcb,key);           /* Reselect A Candidate */
                return(0);
            }
            else
            {
                if ( fepcb->auxuse == USE )
                    AbcCloseAux(fepcb);
                if (IfFirstKey(key))            /* Key Is First Key     */
                {
                    if (key=='v')                
                        V_Flag = 1;
                    else 
                        V_Flag = 0; 

                    step_mode=ONINPUT;
                    SendBackMsg(fepcb);        /* Remember New Term     */
                    InputInit(fepcb);
                }
                else
                {
                    if ( step_mode == RESELECT )
                    {
                        step_mode = START;
                        if ( key != IMED_SET_OPTION_KEY )
                           return(AbcMain(fepcb,key));/*Send key to echo buffer */
                        else
                           return(MessageBeep(fepcb,ERROR1));
                    }
                    return(SendOneChar(fepcb,key)); /*Send key to echo buffer */
                }
            }

        case ONINPUT:
            if ((input_cur==0) && (in.true_length > 0)) 
                if (key=='v')
                    V_Flag = 1;
            if ( (V_Flag == 1) && (in.true_length == 1) )
            {
                AbcVProc(fepcb, key);               /* Process 'v' Convert   */
                return(0);
            }

            input_char_type=SubFilter(fepcb,key);   /* Get The Radical Type */

            switch(input_char_type)
            {
                case REINPUT:
                    step_mode=START;
                    return(0);

                case CLC:
                    return(0);       /* Continue input.*/

                case UNC:             /* Inputing is over and without convert */
                    SendMsg(fepcb, &in.buffer[0], in.true_length); /* skip the 'v'*/
                    step_mode = 0;
                    return(0);
        
                case STC:              /* input finished */
                     if (V_Flag)
                     {
                         SendMsg(fepcb, &in.buffer[1], in.true_length-1); /* skip the 'v'  */
                         V_Flag = 0;
                         step_mode = START;
                         return(0);
                    }

                    step_mode=4 ;    /* enter proccessing step */
                    leibie=Analize(fepcb);
                    if (leibie==BIAODIAN_ONLY)
                    {
                        out_pointer=2;
                        OutResult(fepcb);
                        step_mode = START;
                        return(0);
                    }             /*only bioadian case*/

                    return(AbcCandidates(fepcb,leibie));
            }

    } /*switch step_mode*/
    return(0);
}

/******************************************************************************/
/* FUNCTION    : AbcCandidates                                                */
/* DESCRIPTION : Find satisfied candidates and shows candidate list box       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb,mtype                                                  */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        : CwpProc,ListBox, MoveResult, ShowErrMsg, DispAutoSelect      */
/*             : MessageBeep                                                  */
/******************************************************************************/

AbcCandidates(fepcb,mtype)
FEPCB    *fepcb;                                /* FEP control block      */
int mtype;                                      /* Convert string type    */
{

    /* Initialize      */
    jiyi_mode=0;
    in.buffer[in.true_length]=0;
    cp_ajust_flag=0;
    group_no=0;
    unit_length=0;

    if (CwpProc(mtype)!=1)                      /* Convert       */
    {
        MessageBeep(fepcb,ERROR2);                 /*word exchange is wrong*/
        if (result_area_pointer > 0)
        {
            jiyi_mode=0;                        /* Send Candidates to Screen */
            unit_length=result_area_pointer;     
            OutResult(fepcb);
            step_mode = START;
            return(0);
        }
        else
        {
            step_mode=ONINPUT;
            CurPos(fepcb,0);                   /* Cursor in Start Position  */
            input_cur=0;
            return(0);
        }
    } 

    if (msg_type==2)
    {
        current_no=0;       
        MoveResult(fepcb);             /* Move Prompt Result to Result area */
        OutResult(fepcb);              /* special change*/
        return(0);
    }

    FindCharByWord();                   

    fepcb->ret = FOUND_CAND;           /* Fill abcstruct Struct            */
    fepcb->abcstruct.allcandno = group_no;
    fepcb->abcstruct.cand=(unsigned char *)&out_svw;
    fepcb->abcstruct.curptr = fepcb->abcstruct.cand;
    fepcb->abcstruct.more = fepcb->abcstruct.allcandno;

    current_no=0;
    if (group_no==1)                  /* Just One Candidate               */
    {
        if (cp_ajust_flag!=0)
            result_area_pointer-=unit_length;
        MoveResult(fepcb);
        step_mode=1;
        fepcb->auxchfg=ON;
        fepcb->auxuse = USE;

        return(0);
    }
    else
    {
        disp_tail=0;
        if (cp_ajust_flag==1)          /* Word Frequency Has Benn Adjusted */
            AUTO_disp(fepcb);          /* Display Cand. in Pre_edit Area   */
        else
            MoveResult(fepcb);         /* Move Select Cand. to Result Area */
        fepcb->auxchfg=ON;
        fepcb->auxuse = USE;
        ListBox(fepcb,0);              /* Display List Box                 */
        step_mode=1;
        return(0);
    }
}

/******************************************************************************/
/* FUNCTION    : AbcSelect                                                    */
/* DESCRIPTION : select the word or turn to the next or up page               */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, input_char                                            */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        : ListBox,AbcCloseAux,AbcCls,IfPunct,SubFiler,CinoPunction,    */
/*             : OutResult,IfNumerOrNot,MoveResult,PinduAdjust,MessageBeep    */
/******************************************************************************/

AbcSelect(fepcb, input_char)
FEPCB    *fepcb;
unsigned short input_char;
{
    int x;
    switch (input_char){


    case PGUP_KEY :
        if (disp_head==0)
            MessageBeep(fepcb, ERROR1);      /* In First Page, Beep    */
        else{
            disp_head=disp_head-now.fmt_group; /* Show The Last Page   */
            disp_tail=disp_head;
            ListBox(fepcb);
        }
        return(1);

    case PGDN_KEY :
        if (disp_tail>=group_no)
            MessageBeep(fepcb, ERROR1);     /* In Last Page, Beep    */
        else
            ListBox(fepcb);                 /* Show The Next Page   */
        return(1);                          /*means break the STD MODE*/

    case HOME_KEY:
        disp_head=0;                        /* Show Fisrt Page      */
        disp_tail=0;
        ListBox(fepcb);
        return(1);

    case END_KEY:
        disp_head=group_no/10*10;            /* Show Last Page      */
        disp_tail=disp_head;
        ListBox(fepcb);
        return(1);

    case ESC_KEY :
        AbcCloseAux(fepcb);        /* Disappear Candidate List Box And   */
        AbcCls(fepcb);             /* Return To Radical Input Phase      */
        break;

    case DELETE_KEY :
        AbcCloseAux(fepcb);        /* Disappear Candidate List Box And   */
                                   /* Return To Radical Input Phase      */
        AbcCls(fepcb);             /* Clear All Radical At Off_the_Spot  */
        break;

    case NUM_KEY0 :                /*  Select A Candidate */
    case NUM_KEY1 :
    case NUM_KEY2 :
    case NUM_KEY3 :
    case NUM_KEY4 :
    case NUM_KEY5 :
    case NUM_KEY6 :
    case NUM_KEY7 :
    case NUM_KEY8 :
    case NUM_KEY9 :
        input_char=input_char&0xff;

    default:
        if (IfPunct(input_char))
        { 
            SubFilter(fepcb,input_char);    /* this produce the situation    */
            CinoPunctuation();              /* when the result had display   */
            if(!OutResult(fepcb))           /* and then input the punctuation*/
                step_mode=START;
            return(1);         
        }

        if ( !IfNumberOrNot(input_char) ) 
        { 
           if ( input_char != IMED_SET_OPTION_KEY )
           {
               while(OutResult(fepcb));     /* if input is not number send   */
               step_mode = START;           /* the result and set start step */
               AbcCloseAux(fepcb); 
               AbcMain(fepcb, input_char);
            }
            else
               MessageBeep(fepcb,ERROR1);
            return(1);
        }                                   

        x=(input_char-0x30)+disp_head;

        if (x>=group_no)
        {
            MessageBeep(fepcb,ERROR1);
            return(0);
        }
        else
            current_no=x;
        result_area_pointer-=unit_length;    /*if recall, unit_length=8*/
        if (in.info_flag==1)                 /*result_area_pointer maybe small*/
            result_area_pointer=0;           /*than zero, so reset it =0*/
        MoveResult(fepcb);
        PinduAdjust();
        step_mode=RESELECT;                  /*Note: pindu_ajust must      */
        OutResult(fepcb);                    /* above out_result step_mode */
        return(1);                           /* must be set to RESELECT    */
    }
}

/******************************************************************************/
/* FUNCTION    : MoveResult                                                   */
/* DESCRIPTION : move prompt results to result area                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        : DispAutoResult                                               */
/******************************************************************************/
MoveResult(fepcb)
{
    int i,j;
    WORD x;
    BYTE *p;

    p=(BYTE *)msx_area;

    x=unit_length;
    j=unit_length*current_no;
    for (i=0; i<x; i++)
        result_area[result_area_pointer++]=out_svw[j+i];

    if ( msg_type != 2 )
    DispAutoSelect(fepcb);  /* Display the default Cand. in Pre-edit area */

}

/******************************************************************************/
/* FUNCTION    : OutResult                                                    */
/* DESCRIPTION : send the select result to the screen                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        : AbcCandidates, TempRemProc, AddExtLib, SendMsg               */
/******************************************************************************/
OutResult(fepcb)
{
    int i,j,utf_length=0;
    wchar_t y;
    BYTE disp_bf[120];        /* Jan. 12 95  Modified by Tang Bosong   */

    if (jiyi_mode==1)                     /* All Input Message Hasn't Been */
    {                                     /* Converted Completely          */
        if (word_back_flag==0x99)
        {
            word_back_flag=2;
            return(AbcCandidates(fepcb,13));
        }
        else
        {
            AbcCandidates(fepcb,12);
            return(1);
        }
    } 
    else
    {
        if (result_area_pointer!=unit_length)     /* Create a new word      */
            if (msg_type!=2)
                if (in.info_flag!=1)
                    new_no=result_area_pointer;


        if (!(msg_type&2))                       /* Input Message is not mix */
        {                                        /* of Chinese and English   */
            if (result_area_pointer>0)
            {
                TempRemProc();                   
                /* Change the convert results from internal code to EUC code */
                for (i=0; i<result_area_pointer; i=i+2) 
                {
                    y=FindHanzi(result_area[i]*0x100+result_area[i+1]);
                    utf_length += wctomb(&disp_bf[utf_length],y);
                }
            } 
            last_out_length=out_length;
            out_length=result_area_pointer/2;
            AddExtLib();                        /* Adjust word remember area */
        } 
        else
        {                                       /* Output the Convert Results */
            last_out_length=out_length;
            out_length=result_area_pointer/2;
            for ( j = 0; j < result_area_pointer; j += 2)
               utf_length += wctomb(&disp_bf[utf_length], result_area[j]*0x100 + result_area[j+1]);
            if (in.buffer[0]!='v')
            {
                step_mode=0;
            }
        }

        if (biaodian_value!=0)                    /* Input Message Has Punct.*/
        {
            utf_length += wctomb(&disp_bf[utf_length], biaodian_value);
        }
        SendMsg(fepcb, disp_bf, utf_length);
        return(0);
    }
}

/******************************************************************************/
/* FUNCTION    : DispAutoSelect                                               */
/* DESCRIPTION : send the select result to the screen                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        : CurPos, ShowString, FindHanzi, DispSpecChar                  */
/******************************************************************************/
DispAutoSelect(fepcb)
FEPCB    *fepcb;
{
    WORD x,y,lng;
    int i,j;
    BYTE disp_bf[120];       /* Jan.12 95 Modified by Tang Bosong */
    int n; 

    if (result_area_pointer==0)
        return(0);
    CurPos(fepcb,input_msg_disp);                   /* Set cursor position */
    if (msg_type==2)
    {                                               /* Show Mix String    */
        for ( j = i = 0; j < result_area_pointer; j += 2)
           i += wctomb(&disp_bf[i], result_area[j]*0x100 + result_area[j+1]);
        ShowString(fepcb, disp_bf, i);
        CurPos(fepcb,input_msg_disp+i);
    }
    else
    {        /* Change the convert results from internal code to EUC code */
        for (j=0, i=0; j<result_area_pointer; j=j+2, i=i+3)
        {
            x=result_area[j]*0x100+result_area[j+1];
            y=FindHanzi(x);
            wctomb(&disp_bf[i], (wchar_t)y);
        }
        lng=in.true_length;
        if (jiyi_mode!=0)
        {
            if (wp.xsyjw!=0)
            {
                if (wp.yj_ps[wp.xsyjw-1]<in.true_length)
                {/* Convert isn't over, fill the rest radicals in echo buffer */
                    j=wp.yj_ps[wp.xsyjw-1];
                    x=in.true_length-wp.yj_ps[wp.xsyjw-1];
                    for (n=0; n<x; n++)
                        disp_bf[i++]=in.buffer[j++];
                }
            }
        }

        AbcCls(fepcb);
        ShowString(fepcb,disp_bf, i);

        CurPos(fepcb, input_msg_disp+(result_area_pointer*3)/2);
    }
}

AUTO_disp(fepcb)
FEPCB    *fepcb;
{
    int i,j,flag= 0;
 
    result_area_pointer -= unit_length;
    for (i=0; i<group_no; i++)
    {
       flag = 0;
       for (j=0; j<unit_length; j++)
       {
           if ( result_area[j+result_area_pointer] != out_svw[i*unit_length+j] )
           {
               flag = 1;
               break;
           }
       }

       if ( flag == 0 )
       {
           current_no = i;
           break;
       }
    }
    result_area_pointer += unit_length;
    DispAutoSelect(fepcb);
}

/******************************************************************************/
/* FUNCTION    : SubFilter                                                    */
/* DESCRIPTION : send the received string to in.buffer                        */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb,input_char                                             */
/* OUTPUT      : REINPUT, RECALL, STC, CLC                                    */
/* CALLED      :                                                              */
/* CALL        : InputInit, MessageBeep, CurPos, ShowString, IfPunct, ShowChar*/
/******************************************************************************/
SubFilter(fepcb,input_char)
FEPCB   *fepcb;
int  input_char;
{
    int bd_find=0,i;

    switch(input_char){

    case ESC_KEY:                      /*ESC_KEY=0x1b*/
        InputInit(fepcb);              /* Erase All Radicals           */
        return(REINPUT);   

    case CONVERT_KEY:                  /*CK_SPACE=0x20*/
        in.info_flag=BY_WORD;
        return(STC);      

    case BACK_SPACE_KEY:               /*BACK_SPACE_KEY=0x08*/
        if ((!input_cur)||(!in.true_length)) 
        {   /* Cursor is in start position or input length is 0 , then beep */
            MessageBeep(fepcb,BLANK);
            if (!in.true_length)
                return(REINPUT); 
            else
                return(CLC);
        }

        in.true_length--;              
        if ( (V_Flag == 2) && (in.true_length == 1) )
            V_Flag = 1;
            
        if ( !in.true_length ) 
        {
            end_flg=0;
            input_cur=0;
            MessageBeep(fepcb,BLANK);
            InputInit(fepcb);
            V_Flag=0;       
            return(REINPUT);
        }
        else
        {                         /* Delect a radical                 */
            input_cur--;         
            for (i=0;i<in.true_length-input_cur;i++)
                in.buffer[input_cur+i]=in.buffer[input_cur+i+1];
            in.buffer[in.true_length]=0x20;
            AbcCls(fepcb);
            CurPos(fepcb, 0);
            ShowString(fepcb,&in.buffer,in.true_length+1);
            CurPos(fepcb, input_cur);
            return(CLC);
        }

    case RETURN_KEY:                    /*RETURN_KEY=0x0d*/
        in.info_flag=BY_CHAR;
        new_no=0;
        return(STC);     
    case NON_CONVERT_KEY:
        return(UNC);
   
   /* LEFT_ARROW_KEY and RIGHT_ARROW_KET, move cursor position */
    case LEFT_ARROW_KEY:               
        if (input_cur>0) input_cur--;
        CurPos(fepcb, input_cur);
        return (CLC);
    case RIGHT_ARROW_KEY:
        if (input_cur<in.true_length) input_cur++;
        CurPos(fepcb, input_cur);
        return (CLC);
    case UP_KEY:
        input_cur=0;
        CurPos(fepcb, input_cur);
        return(CLC);
    case DOWN_KEY:
        input_cur=in.true_length;
        CurPos(fepcb, input_cur);
        return (CLC);

    /* Set " use the word to get the last/first char." flag  */
    case ']':
    case '[':
        if (input_char==']')
            jlxw_mode=1;
        else
            jlxw_mode=-1;
        in.info_flag=BY_WORD;
        return(STC);    

    default:
        if ((input_char&0xff00)!=0||(input_char<=0x20)) 
        {                                      /* Input Char is illegal */
            MessageBeep(fepcb,ERROR1);
            return(CLC); 
        }
        if (IfPunct(input_char) && (!V_Flag))  
            bd_find=1;                         /* Set punctuation find flag */

        if (in.max_length<=in.true_length)
        {
            MessageBeep(fepcb,ERROR1);
            return(CLC);  
        }
        else
        {
            if(input_cur>=in.true_length)
            {   /* Cursor is at the end of input string, show input char */
                in.buffer[in.true_length++]=input_char;
                input_cur++;
                ShowChar(fepcb,input_char);

                if (bd_find==1)
                    return(STC); 
                else
                    return(CLC);
            }
            else
            {
                if (fepcb->imode.ind4==INSERT_MODE)
                    for (i=0; i<in.true_length-input_cur;i++)
                        in.buffer[in.true_length-i] =in.buffer[in.true_length-i-1];  
                in.buffer[input_cur++]=input_char;
                if (!bd_find)
                {
                    if (fepcb->imode.ind4==INSERT_MODE)
                        in.true_length++;
                    CurPos(fepcb, 0);
                    ShowString(fepcb,&in.buffer,in.true_length);
                    CurPos(fepcb, input_cur);
                    return(CLC);
                }
                else
                {
                    for(i=input_cur;i<in.true_length;i++)
                        in.buffer[i]=0x20;
                    CurPos(fepcb, 0);
                    ShowString(fepcb,&in.buffer,in.true_length);
                    CurPos(fepcb, input_cur);
                    in.true_length=input_cur; 
                    return(STC);
                }

            }/*#4 if now_cs...else*/
        }/*#3 if max...else*/
    }/*#2 case*/

}/*#0*/


/******************************************************************************/
/* FUNCTION    : Analize                                                      */
/* DESCRIPTION : process the punctuation, and analize the first byte of input */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb,input_char                                             */
/* OUTPUT      : BIAODIAN_ONLY ---- input only chinese punctuation            */
/*                           0 ---- standard change                           */
/*                           1 ---- ABBR                                      */
/*                           2 ---- "I" change                                */
/*                           3 ---- "i" change                                */
/*                           4 ---- "u" change                                */
/*                        0xff ---- trun to "force remember"                  */
/* CALLED      :                                                              */
/* CALL        : CinoPunctuation                                              */
/******************************************************************************/
Analize()
{
    if (CinoPunctuation())
        return(BIAODIAN_ONLY);     /* only have the chinese punctuation */


    switch (in.buffer[0])
    {
        case 'I':
            return(2);             /* special change: "I" change */
        case 'i':
            return(3);              /* special change: "i" change */
        case 'u':
        case 'U':
            in.buffer[0]='U';
            if (in.true_length==1)
                return(0xff);       /*  trun to the "remember forced" */
            else
                return(4);          /* special change: "u" change */
    }
    if ((in.buffer[0]&0x20)==0)  
        return(1);                          /* ABBR */
    else
        return(0);                      /* mark of the standard change */


}


/******************************************************************************/
/* FUNCTION    : IfFirstKey                                                   */
/* DESCRIPTION : judge if the first input key is correct                      */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : input_char                                                   */
/* OUTPUT      : CLC ---- input is aph or punctuation.                        */
/*               STC ---- input is number                                     */
/* CALLED      :                                                              */
/* CALL        : IfNumberOrNot, IfAlphOrNot, IfPunct                          */
/******************************************************************************/
IfFirstKey(input_char)
WORD input_char;
{
    if(IfNumberOrNot(input_char))
        return(STC);       /*the first key is number,it's not allowed*/
    if( input_char == 'U' )
        return(STC);
    if(IfAlphOrNot(input_char))
        return(CLC);                               /*the first key is zimu*/
    if((int_asc_mode==1)||(bdd_flag==1))
        return(IfPunct(input_char));
    return(STC);
}

/******************************************************************************/
/* FUNCTION    : IfAlphOrNot                                                  */
/* DESCRIPTION : Test the input key is a Abc radical.                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/******************************************************************************/
BOOL IfAlphOrNot(x)
BYTE x;
{
    if (('A'<=x) && (x<='Z'))
        return(TRUE);
    if (('a'<=x) && (x<='z'))
        return(TRUE);
    return(FALSE);

}

/******************************************************************************/
/* FUNCTION    : IfNumberOrNot                                                */
/* DESCRIPTION : Test the input key is a number.                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/******************************************************************************/
IfNumberOrNot(c)
BYTE c;
{
    if ((c<'0')||(c>'9'))
        return(STC);
    else
        return(CLC);

}


/******************************************************************************/
/* FUNCTION    : IfPunct                                                      */
/* DESCRIPTION : Test the input key is a punctuation.                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : key   = 1 byte unsigned char                                 */
/* OUTPUT      : TRUE or FALSE                                                */
/* CALLED      :                                                              */
/******************************************************************************/
BOOL IfPunct(x)
BYTE x;
{
    int i;
    for (i=0; i<strlen(biaodian_table); i++)
    {
        if (x==biaodian_table[i])
        {
            biaodian_pos=i;       /* record the position in punctuation_table */
            return(TRUE);
        }
    }
    return(FALSE);
}


/******************************************************************************/
/* FUNCTION    : DelAndReconvert                                              */
/* DESCRIPTION : process the BACK key                                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        : SameAsBackword, AbcCandidates                                */
/******************************************************************************/
DelAndReconvert(fepcb)
FEPCB        *fepcb;
{
    int x;

    if (word_back_flag==0xaa)       /* Convert is 'Getting char by word' mode */
        return(SameAsBackwords(fepcb));  /* Redisplay input Pre-edit area     */

    wp.dw_count--;
    x=wp.dw_stack[wp.dw_count+1]-wp.dw_stack[wp.dw_count];
    if (x==1)
    {                                    /* if the single word exchange       */
        if (!wp.dw_count)
            return(SameAsBackwords(fepcb));
        else
        {
            wp.dw_count--;
            x=wp.dw_stack[wp.dw_count+1]-wp.dw_stack[wp.dw_count]+1;
        }
    }
    result_area_pointer-=x*2;
    word_back_flag=x-1;
    wp.xsyjw=wp.dw_stack[wp.dw_count];
    return(AbcCandidates(fepcb,13));

}


/******************************************************************************/
/* FUNCTION    : BackwordProc                                                 */
/* DESCRIPTION : process "alt+'-'"                                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        : SameAsBackword, SendMsg, AbcCandidates                       */
/******************************************************************************/
BackwordProc(fepcb)
FEPCB        *fepcb;
{
    switch(step_mode)
    {
        case SELECT:
            return(SameAsBackwords(fepcb));
            break;
    
        case RESELECT:
        case START:
            if (!msg_type)
                return(SameAsBackwords(fepcb));
            else
                SendMsg(fepcb,msg_bf,msg_count);
            break;
        case ONINPUT:
            return(AbcCandidates(fepcb,14));
            break;
    }
}


/******************************************************************************/
/* FUNCTION    : SameAsBackwords                                              */
/* DESCRIPTION : Redisplay the in.buffer in "alt+'-'"                  */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        : AbcCloseAux, CurPos, ShowChar, HaltInit                      */
/******************************************************************************/
SameAsBackwords(fepcb)
FEPCB        *fepcb;
{
    int i;

    if (in.buffer[0]!='v')
    {
        AbcCloseAux(fepcb);
        if (in.true_length>=1)
            if (in.buffer[0]!='U')          /* First radical isn't 'U' */
            {                               /* Not captial mode        */
                CurPos(fepcb,input_msg_disp);
                AbcCls(fepcb);
                for (i=0; i<in.true_length; i++)
                    ShowChar(fepcb,in.buffer[i]);
                CurPos(fepcb,input_msg_disp+in.true_length);
                HalfInit(fepcb);
                step_mode=ONINPUT;
                return(0);
            }
    }
    out_length=last_out_length;
    return(REINPUT);

}


/******************************************************************************/
/* FUNCTION    : FindCharByWord                                               */
/* DESCRIPTION : get a char by word                                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
FindCharByWord()
{
    int x,i,j;
    WORD *out_svw_p;

    out_svw_p=(WORD *)out_svw;

    if (jlxw_mode==0)            /* Use word to get char is not set */
        return(0);
    if (jlxw_mode<0){            /* Use word to get first char  */
        if (cp_ajust_flag==1)
                                 /* Adjust result buffer pointer */
            result_area_pointer=result_area_pointer-unit_length+2;
        x=0;
    }
    else
    {
        if (cp_ajust_flag==1)
        {                        /* Adjust result buffer conent and pointer */
            x=result_area_pointer-unit_length;
            result_area[x++]=result_area[result_area_pointer-2];
            result_area[x++]=result_area[result_area_pointer-1];
            result_area_pointer=x;
        }
        x=unit_length-2;
    }

    jlxw_mode=0;
    word_back_flag=0xaa;
    if (unit_length<=2)
        return(0);

    j=0;
    x=x/2;                          /*out_svw_p transmit by word;*/
    for (i=0; i<group_no; i++){
        out_svw_p[j++]=out_svw_p[x];
        x+=unit_length/2;
    }
    unit_length=2;

    for (i=0; i<group_no; i++)
        out_svw_p[i+100]=out_svw_p[i];

    x=0;
    for (i=0; i<group_no; i++)
    {
        if (out_svw_p[i+100])
        {
            out_svw_p[x]=out_svw_p[i+100];
            for (j=i+1; j<group_no; j++)
            {
                if (out_svw_p[j+100]==out_svw_p[x])
                    out_svw_p[j+100]=0;
            }
            x++;
        }
    }

    group_no=x;
    return(0);

}


/******************************************************************************/
/* FUNCTION    : CinoPunctuation                                              */
/* DESCRIPTION : process the chinese punctuation                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : TRUE  ---- only have the chinese punctuation                 */
/*               FALES ---- have chinese punctuation and other char           */
/* CALLED      :                                                              */
/* CALL        : IfPunct                                                      */
/******************************************************************************/
BOOL CinoPunctuation()
{
    BYTE x;

    x=in.buffer[in.true_length-1];
    if (!IfPunct(x))
        return(FALSE);

    /* Get the Chinese punctuation's Unicode     */
    biaodian_value=cc_biaodian[biaodian_pos*2]+
        cc_biaodian[biaodian_pos*2+1]*0x100;
    if (x==0x22)
    {                                                 /*match qoute*/
        if (yinhao_flag==1)
            biaodian_value=cc_biaodian[(biaodian_pos+1)*2]
                +cc_biaodian[(biaodian_pos+1)*2+1]*0x100;
        yinhao_flag=!yinhao_flag;           
    }
    in.true_length--;
    if (in.true_length==0)
        return(TRUE);
    else
        return(FALSE);
}

/******************************************************************************/
/* FUNCTION    : AbcShowCursor                                                */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/******************************************************************************/
AbcShowCursor(fepcb)      
FEPCB *fepcb;
{
    if (fepcb->eccrpsch == ON)         /*  for  Showing  Cursor   */
    {
        memset(fepcb->echobufa, REVERSE_ATTR, fepcb->echoacsz);
        if (fepcb->echocrps < fepcb->echoacsz)
        {
            memset(fepcb->echobufa, REVERSE_ATTR, fepcb->echoacsz);
            fepcb->echobufa[fepcb->echocrps] = 1;
            fepcb->echobufa[fepcb->echocrps+1] = 1;
            fepcb->echochfg.flag = ON;
            fepcb->echochfg.chtoppos = 0;
            fepcb->echochfg.chlenbytes = 1;
        }
    }
}


/******************************************************************************/
/* FUNCTION    : AbcInitial                                                   */
/* DESCRIPTION : Initialization.                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

AbcInitial(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{

    AbcCls(fepcb);           /* Erase All Radicals At Off_The_Spot */

    /* Clear Edit_End Buffer              */
    if ( fepcb->edendacsz == 0 )
    {
       (void)memset(fepcb->edendbuf, NULL, fepcb->echosize);
       fepcb->edendacsz = 0;
    }
    if( fepcb->auxchfg != ON )
        fepcb->auxchfg = OFF;
    fepcb->auxuse = NOTUSE;
    fepcb->auxcrpsch = OFF;
    fepcb->auxacsz.itemsize = 0;
    fepcb->auxacsz.itemnum = 0;
    fepcb->indchfg = ON;
    fepcb->imode.ind0 = ABC_MODE;
    fepcb->imode.ind2 = SELECT_OFF;
    fepcb->imode.ind4 = REPLACE_MODE;
    fepcb->imode.ind5 = BLANK;
    fepcb->isbeep = BEEP_OFF;
    fepcb->inputlen = 0;
    
    step_mode = START;
    bdd_flag_keep = bdd_flag;
    auto_mode_keep = auto_mode;
    territory_keep = territory;
}


/******************************************************************************/
/* FUNCTION    : AbcCls                                                       */
/* DESCRIPTION : Erase all radicals at Over_The_Spot.                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
AbcCls(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
    /* Clear Echo Buffer                  */
    (void)memset(fepcb->echobufs, NULL, fepcb->echosize);
    (void)memset(fepcb->echobufa, NULL, fepcb->echosize);

    fepcb->echochfg.flag = TRUE;
    fepcb->echochfg.chtoppos = 0;
    fepcb->echochfg.chlenbytes = fepcb->echoacsz;
    fepcb->echocrps = 0;
    fepcb->eccrpsch = ON;
    fepcb->echoacsz = 0;
    fepcb->echoover = fepcb->echosize;
    fepcb->inputlen = 0;
}


/******************************************************************************/
/* FUNCTION    : InputInit                                                    */
/* DESCRIPTION : Erase the display area, and give the init_value to paraments */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        : AbcEraseAllRadicale                                          */
/******************************************************************************/
InputInit(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */

{
    int i;
    AbcCls(fepcb);    /* Clears the input display msg */

    input_cur=input_msg_disp;

    for (i=0; i<in.max_length; i++)
        in.buffer[i]=0;

    CurPos(fepcb, input_msg_disp);
    in.true_length=0;
    in.info_flag=0;
    result_area_pointer=0;
    word_back_flag=0;
    biaodian_value=0;
    msg_type=0;
    jiyi_mode=0;
    new_no=0;
    end_flg=0;

}



/******************************************************************************/
/* FUNCTION    : HalfInit                                                     */
/* DESCRIPTION :  give the init_value to paraments                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
HalfInit(fepcb)
FEPCB        *fepcb;
{
    step_mode=ONINPUT;
    result_area_pointer=0;
    biaodian_value=0;
    new_no=0;
    msg_type=0;
    word_back_flag=0;
    jiyi_mode=0;
    input_cur=now_cs;
}

/******************************************************************************/
/* FUNCTION    : ListBox                                                      */
/* DESCRIPTION : Process PageUp and PageDown On Candidate List Box.           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

ListBox(fepcb)
FEPCB           *fepcb;                 /* FEP Control Block                  */
{
    unsigned char   *candptr;           /* Pointer To Candidates              */
    unsigned char   **toauxs;           /* Pointer To Aux. String             */
    unsigned char   **toauxa;           /* Pointer To Aux. Attribute          */
    unsigned char   *tostring;          /* Pointer To Aux. String             */
    unsigned short  item;               /* Item Number                        */
    unsigned short  itemsize;           /* Item Size                          */
    unsigned short  i, j;               /* Loop Counter                       */
    char buffer[4];
    int len;
    int n;
    unsigned char *temp;
    unsigned short k,m;
    unsigned short x,y;                  /* Used when cc_code is converted    */

    static char digit[]={0x20,0x30,0x20,0x20,0x20,0x31,0x20,0x20,0x20,0x32,0x20,
               0x20,0x20,0x33,0x20,0x20,0x20,0x34,0x20,0x20,0x20,0x35,0x20,0x20,
               0x20,0x36,0x20,0x20,0x20,0x37,0x20,0x20,0x20,0x38,0x20,0x20,
               0x20,0x39,0x20,0x20};


    candptr = fepcb->abcstruct.curptr;

    len = strlen(ABCTITLE)+4;   /* Length Of Title & Bottom Of Cand.  */
    if ( len < unit_length )
        fepcb->auxacsz.itemsize = unit_length;
    else
        fepcb->auxacsz.itemsize = len;


    toauxs = fepcb->auxbufs;
    toauxa = fepcb->auxbufa;
                                               /*  clear aux buffer   */
    if (fepcb->auxacsz.itemnum != 0)      
        for( i = 0 ; i< fepcb->auxacsz.itemnum-2; i++ )
            memset( *toauxs++,' ',fepcb->auxacsz.itemsize+1);

    toauxs = fepcb->auxbufs;
    toauxa = fepcb->auxbufa;

    for(k = 0; k < 8 ; k++)
    {
        temp = fepcb->auxbufs[k];
        for(m = 0; m < fepcb->auxacsz.itemsize; memcpy(temp," ",1),
            temp++,m++);
    }/* for */
                                     /* Fill Title                      */
    memcpy(fepcb->auxbufs[0], ABCTITLE, strlen(ABCTITLE));  

                                     /* Fill Line                       */
    memccpy(fepcb->auxbufs[1], LINE, "", fepcb->auxacsz.itemsize);

                                  /* Fill Candidates To Aux. Buffer     */

    disp_head=disp_tail;          /* Display indicator                  */

    for( item=0; item<10 && disp_tail<group_no; item++ )
    {
        itemsize = 0;
        tostring = fepcb->auxbufs[item+2];
                                  /* Fill Number                        */
        for( j=item*4; j<item*4+4; j++ )
        {
            itemsize ++;
            *tostring++ = *(digit+j);
        }

        for (j=0; j<unit_length; j=j+2)
        {                                    /* Get cc_inner_code      */
            k=j+disp_tail*unit_length;
            x=out_svw[k]*0x100+out_svw[k+1];
            y=FindHanzi(x);
            n = wctomb(tostring, (wchar_t)y);
            tostring += n;
        }
        disp_tail++;
    }

    fepcb->abcstruct.more = group_no-disp_tail;
    tostring = fepcb->auxbufs[0]+strlen(ABCTITLE);

    itoa(fepcb->abcstruct.more, buffer, sizeof(buffer));
    memcpy(tostring, buffer, sizeof(buffer));

    memcpy(fepcb->auxbufs[AUXROWMAX-2], BOTTOM1, strlen(BOTTOM1));
                                       /* Fill Bottom Message          */
    memcpy(fepcb->auxbufs[AUXROWMAX-1], BOTTOM2, strlen(BOTTOM2));

    fepcb->auxacsz.itemnum = AUXROWMAX;
    fepcb->auxchfg = ON;
}


/******************************************************************************/
/* FUNCTION    : AbcCloseAux                                                   */
/* DESCRIPTION : Disappear candidate list box and return to radical input     */
/*               phase.                                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

AbcCloseAux(fepcb)
FEPCB           *fepcb;                /* FEP Control Block                  */
{
    char            **toauxs;           /* Pointer To Aux. String Buffer      */
    char            **toauxa;           /* Pointer To Aux. Attribute Buffer   */
    int i;

    toauxs = fepcb->auxbufs;  
    toauxa = fepcb->auxbufa;
    for( i=0; i<AUXROWMAX; i++ )
    {
        (void)memset(*toauxs++, NULL, AUXCOLMAX);
        (void)memset(*toauxa++, NULL, AUXCOLMAX);
    }
    fepcb->auxacsz.itemnum = 0;
    fepcb->auxacsz.itemsize = 0;
    fepcb->imode.ind2 = SELECT_OFF;
    fepcb->echocrps = 0;
    fepcb->eccrpsch = ON;
    fepcb->auxuse = NOTUSE;
    fepcb->auxchfg = ON;
}


/******************************************************************************/
/* FUNCTION    : DispSpecChar                                                 */
/* DESCRIPTION : display a string of same char                                */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/*               1 char                                                       */
/*               1 byte display length                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        : ShowChar                                                     */
/******************************************************************************/
DispSpecChar(fepcb,c,n)
FEPCB     *fepcb;
int c,n;
{
    int i;
    for (i=0; i<n; i++)
        ShowChar(fepcb,c);
}

/******************************************************************************/
/* FUNCTION    : ShowChar                                                     */
/* DESCRIPTION : Send/Change input char to echo buffer.                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, string, count                                         */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

ShowChar(fepcb, input_char)
FEPCB           *fepcb;                /* FEP Control Block                  */
char        input_char;                /* The strings to be displayed        */
{

    if (fepcb->echocrps < fepcb->echosize )
    {
        *(fepcb->echobufs+fepcb->echocrps) = input_char;
        *(fepcb->echobufa+fepcb->echocrps) = REVERSE_ATTR;
    }
    else
    {
        MessageBeep(fepcb,ERROR1);
        return;
    }

    if (fepcb->echochfg.flag == ON)
    {
        if (fepcb->echochfg.chtoppos>=fepcb->echocrps)
        {
            fepcb->echochfg.chlenbytes
            += fepcb->echochfg.chtoppos-fepcb->echocrps;
            fepcb->echochfg.chtoppos=fepcb->echocrps;
        }
        else
            fepcb->echochfg.chlenbytes
                = fepcb->echocrps-fepcb->echochfg.chtoppos;
    }
    else 
    {
        fepcb->echochfg.chtoppos = fepcb->echocrps;
        fepcb->echochfg.chlenbytes=1;
        fepcb->eccrpsch = ON;
    }

    if( fepcb->echocrps >= fepcb->echoacsz )
    {
        fepcb->inputlen ++;
        fepcb->echoacsz += 1;
        fepcb->echoover -= 1;
    }
    fepcb->echocrps += 1;
    fepcb->echochfg.flag = ON;
    fepcb->eccrpsch = ON;
    now_cs=fepcb->echocrps;
}


/******************************************************************************/
/* FUNCTION    : ShowString                                                   */
/* DESCRIPTION : Send/Change string to echo buffer.                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, string, count                                         */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
ShowString(fepcb,string,count)
FEPCB    *fepcb;
unsigned char *string;    /* Jan.12 95 Modified By B.S.Tang            */
int count;
{
    int i;
    for (i=0; i<count; i++)
        ShowChar(fepcb,string[i]);
}


/******************************************************************************/
/* FUNCTION    : CurPos                                                       */
/* DESCRIPTION : set the cursor position                                      */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb                                                        */
/*               1 byte cursor position                                       */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
CurPos(fepcb,x)
FEPCB    *fepcb;
int x;
{
    now_cs=x;
    fepcb->echocrps = x;
    fepcb->echochfg.flag = ON;


}

/******************************************************************************/
/* FUNCTION    : SendMsg                                                      */
/* DESCRIPTION : Give out the selected results.                               */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, msg, count                                            */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
SendMsg(fepcb,msgs,count)
FEPCB          *fepcb;
unsigned char   *msgs;
unsigned short count;
{

    strncpy(msg_bf, msgs, count);
    msg_count = count;

    strncpy(fepcb->edendbuf, msgs, count);
    fepcb->edendacsz = count;

    AbcCls(fepcb);

    fepcb->ret = FOUND_WORD;

}
/******************************************************************************/
/* FUNCTION    : SendOneChar                                                  */
/* DESCRIPTION : Give out one char                                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, msg, count                                            */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
SendOneChar(fepcb,key)
FEPCB          *fepcb;
unsigned short key;
{
    fepcb->ret = IMED_NOTUSED;
}

/******************************************************************************/
/* FUNCTION    : MessageBeep                                                  */
/* DESCRIPTION : Make a beep and display message on indicator line            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb,msg                                                        */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

MessageBeep(fepcb,msg)
FEPCB           *fepcb;                /* FEP Control Block  */
unsigned short  msg;                   /* Message number     */
{
    fepcb->isbeep = BEEP_ON;
    fepcb->indchfg = ON;
    fepcb->imode.ind5 = msg;
    return;
}

/***************************************************************************/
/* FUNCTION    : AbcFreeCandidates                                         */
/* DESCRIPTION : Free allocated memory for ABC candidates                  */
/* EXTERNAL REFERENCES:                                                    */
/* INPUT       : fepcb                                                     */
/* OUTPUT      : fepcb                                                     */
/* CALLED      :                                                           */
/* CALL        :                                                           */
/***************************************************************************/

AbcFreeCandidates(fepcb)
FEPCB *fepcb;
{
    if ( fepcb->abcstruct.cand != NULL )
    {
        /****************************/
        /*   initial abc structure  */
        /****************************/
        fepcb->abcstruct.cand=NULL;
        fepcb->abcstruct.curptr=NULL;
        fepcb->abcstruct.allcandno=0;
        fepcb->abcstruct.more=0;

    }
}



/******************************************************************************/
/* FUNCTION    : AbcSetOption                                                 */
/* DESCRIPTION : Set Abc Input Method Options.                                */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fepcb, key                                                   */
/* OUTPUT      : fepcb                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

AbcSetOption(fepcb, key)
FEPCB           *fepcb;                 /* FEP Control Block                  */
unsigned short  key;                    /* Ascii Code ( 1/2/Ecs/Enter )       */
{
    unsigned char   **toauxs;           /* Pointer To Aux. String             */
    unsigned char   **toauxa;           /* Pointer To Aux. Attribute          */
    unsigned char   *tostring;          /* Pointer To Aux. String             */
    unsigned char   *toattribute;       /* Pointer To Aux. Attribute          */
    unsigned short  item;               /* Item Number                        */
    int i, j;                           /* Loop Counter                       */
    char buffer[24];

    static char line[]={0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,
                       0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,
                       0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,0xa9,0xa5,
                       0xa9,0xa5,0xa9,0xa5,0xa2,0xa5,0xa9,0xa5,0xa9,0xa5  };


    AbcCls(fepcb);

    switch( key )
    {
        case IMED_SET_OPTION_KEY :
            fepcb->auxuse = USE;
            fepcb->auxchfg = ON;
            step_mode = SETOPTION;
            break;

        case NUM_KEY1 :                /* Set punctuation convert switch */
            if ( bdd_flag == ON )
                bdd_flag = OFF;
            else
                bdd_flag = ON;
            break;

        case NUM_KEY2 :               /* Set frequency adjusting mode switch */
            if ( auto_mode == ON )
                auto_mode = OFF;
            else
                auto_mode = ON;
            break;

        case NUM_KEY3 :
           if ( territory!=CHINESE_GB)
                territory = CHINESE_GB;
           break;


        case NUM_KEY4 :
           if ( territory!=CHINESE_CNS)
                territory = CHINESE_CNS;
           break;

        case NUM_KEY5:
           if ( territory!=CHINESE_GLOBLE)
                territory = CHINESE_GLOBLE;
            break;

        case RETURN_KEY :
                   /* Write ABC Option Parameters To $(HOME)/.abcusrrem File */
            auto_mode_keep = auto_mode;
            bdd_flag_keep = bdd_flag;
            territory_keep =  territory;
                          /* Move The Pointer To The Paremeter Area   */
            lseek(fepcb->fd.abcusrfd[USRREM], TMMR_REAL_LENGTH, 0);
            for (i = 0; i < 14; i++)
                 buffer[i] = 0;

            buffer[0] = territory;         /* Transfer The Peremeters */
            buffer[1] = auto_mode;
            buffer[2] = bdd_flag;
            buffer[3] = fepcb->imode.ind8;

            if ( write(fepcb->fd.abcusrfd[USRREM], buffer, 
                   PAREMETER_LENGTH) == -1 )  /* Writer The File */
                 return;
/*            fflush(fepcb->fd.abcusrfd[USRREM]); */

        case ESC_KEY :                 /* Close ABC Option Set Aux. Box */
            bdd_flag = bdd_flag_keep;
            auto_mode = auto_mode_keep;
            territory =  territory_keep;
            toauxs = fepcb->auxbufs;
            toauxa = fepcb->auxbufa;
            for( i=0; i<AUXROWMAX; i++ )      /* Clear aux buff */
            {
                (void)memset(*toauxs++, NULL, AUXCOLMAX);
                (void)memset(*toauxa++, NULL, AUXCOLMAX);
            }
            fepcb->auxacsz.itemnum = 0;
            fepcb->auxacsz.itemsize = 0;
            fepcb->auxuse = NOTUSE;
            fepcb->auxchfg = ON;
            step_mode = START;
            return;

        default :                     /* Error Process               */
            fepcb->isbeep = BEEP_ON;  /* Notify FEP To Beep To Alarm */
            fepcb->indchfg = ON;
            fepcb->imode.ind5 = ERROR1;
            return;
    }

    fepcb->auxacsz.itemsize = 30;        /* Length Of Title & Bottom. */

    toauxs = fepcb->auxbufs;
    toauxa = fepcb->auxbufa;
                                               /*  clear aux buffer   */
    if (fepcb->auxacsz.itemnum != 0)
        for( i = 0 ; i< fepcb->auxacsz.itemnum-2; i++ )
            memset( *toauxs++,' ',fepcb->auxacsz.itemsize+1);

    toauxs = fepcb->auxbufs;
    toauxa = fepcb->auxbufa;

    for(i = 0; i < 6 ; i++)
    {
        tostring = fepcb->auxbufs[i];
        for(j = 0; j < fepcb->auxacsz.itemsize; memcpy(tostring," ",1),
            tostring++,j++);
    }
                                     /* Fill Title                      */
    strncpy(buffer, ABC_OPTION_TITLE, strlen(ABC_OPTION_TITLE));
    buffer[strlen(ABC_OPTION_TITLE)] = '\0';
    memcpy(fepcb->auxbufs[0], buffer, strlen(buffer));

                                     /* Fill Line                       */
    memccpy(fepcb->auxbufs[1], LINE, "", fepcb->auxacsz.itemsize);

                                 /* Fill ABC Options To Aux. Buffer     */
    strncpy(buffer, ABCOPTION1, strlen(ABCOPTION1));
    buffer[strlen(ABCOPTION1)] = '\0';
    if ( bdd_flag == 1 )
        strncat(buffer, "开)", 4);
    else
        strncat(buffer, "关)", 4);
    memcpy(fepcb->auxbufs[2], buffer, strlen(buffer));

    strncpy(buffer, ABCOPTION2, strlen(ABCOPTION2));
    buffer[strlen(ABCOPTION2)] = '\0';
    if ( auto_mode == 1 )
        strncat(buffer, "开)", 4);
    else
        strncat(buffer, "关)", 4);
    memcpy(fepcb->auxbufs[3], buffer, strlen(buffer));


    strncpy(buffer, ABCOPTION3, strlen(ABCOPTION3));
    buffer[strlen(ABCOPTION3)] = '\0';
    if ( territory == CHINESE_GB )
        strncat(buffer, "开)", 4);
    else
        strncat(buffer, "关)", 4);
    memcpy(fepcb->auxbufs[4], buffer, strlen(buffer));

    strncpy(buffer, ABCOPTION4, strlen(ABCOPTION4));
    buffer[strlen(ABCOPTION4)] = '\0';
    if ( territory == CHINESE_CNS )
        strncat(buffer, "开)", 4);
    else
        strncat(buffer, "关)", 4);
    memcpy(fepcb->auxbufs[5], buffer, strlen(buffer));

    strncpy(buffer, ABCOPTION5, strlen(ABCOPTION5));
    buffer[strlen(ABCOPTION5)] = '\0';
    if ( territory == CHINESE_GLOBLE )
        strncat(buffer, "开)", 4);
    else
        strncat(buffer, "关)", 4);
    memcpy(fepcb->auxbufs[6], buffer, strlen(buffer));


                                       /* Fill Bottom Message          */
    strncpy(buffer, OPTION_BOTTOM, strlen(OPTION_BOTTOM));
    buffer[strlen(OPTION_BOTTOM)] = '\0';
    memcpy(fepcb->auxbufs[ABCOPTION_AUXROWMAX-1], buffer, strlen(buffer));
    fepcb->auxacsz.itemnum = ABCOPTION_AUXROWMAX;
    fepcb->auxchfg = ON;
}


/****************************************************************
AbcVProc():       produce the V fuction
*****************************************************************/
AbcVProc(fepcb, input_char)
FEPCB      *fepcb;                 /* FEP Control Block                  */
WORD       input_char;             /* Ascii Code                         */
{
    int i, n;

    n=0;

    if (V_Flag==1)
    {                 /* Jan. 12 95 Modified by Tang Bosong     */
       if((V_Flag == 1) && (fepcb->echocrps == 0))
       {
           switch (input_char)
           {
             case '0':
             case BACK_SPACE_KEY:
                  InputInit(fepcb); /* Erase the Pre-edit area and initialize */
                  step_mode = START;  /* all parameters                       */
                  V_Flag = 0;
                  return;
           }
           AbcCls(fepcb);    /* Clears the input display msg */
           in.buffer[input_cur++]=input_char;
           CurPos(fepcb, 0);
           ShowString(fepcb,&in.buffer,in.true_length);
           CurPos(fepcb, input_cur);
           step_mode = START;      /* all parameters                         */
           V_Flag = 0;
           AbcFilter(fepcb, input_char);
           return;
       }                /* End Modified                                      */
    }

    if ((IfNumberOrNot(input_char)) && (V_Flag!=2))
    {                         /* 'v2' ~ 'v9' convert to Chinese Graphic char */
        for ( i=0; i<95*2; i++ )
            out_svw[i] = 0;
        i = input_char - '1';
        ReadAIabcOvl((head.grp.startp+i*94)*2, out_svw, 94*2);  /* completely spelling */

        for (n = 0; (out_svw[n] != 0)||(out_svw[n+1] != 0) && n < 94*2; n++);
        group_no = n/2;

        unit_length=2;
        current_no=0;
        msg_type = 2;
        disp_tail=0;
        fepcb->ret = FOUND_CAND;             /* Fill fepcb->abcstruct struct */
        fepcb->abcstruct.allcandno = group_no;
        fepcb->abcstruct.cand=(unsigned char *)&out_svw;
        fepcb->abcstruct.curptr = fepcb->abcstruct.cand;
        fepcb->abcstruct.more = fepcb->abcstruct.allcandno;

        V_Flag=0;
        MoveResult(fepcb);
        fepcb->auxchfg=ON;
        fepcb->auxuse = USE;
        ListBox(fepcb);
        word_back_flag=0x55;
        step_mode=SELECT;
    }
    else 
    {
        V_Flag=2;                           /* No convert                   */
        step_mode=ONINPUT;
        AbcFilter(fepcb, input_char);
    }
}

