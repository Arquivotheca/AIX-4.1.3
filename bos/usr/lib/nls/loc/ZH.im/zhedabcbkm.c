static char sccsid[] = "@(#)48	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhedabcbkm.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:26";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: AddExtLib
 *		PinduAdjust
 *		SendBackMsg
 *		TempRemProc
 *		if_multi_rem
 *		new_word
 *		push_down_stack1
 *		rem_new_word
 *		rem_pd1
 *		rem_pd2
 *		rem_pd3
 *		write_new_word
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
/************************* START OF MODULE SPECIFICATION **********************/
/*                                                                            */
/* MODULE NAME:        zhedabcbkm                                             */
/*                                                                            */
/* DESCRIPTIVE NAME:   AIX-ABC Input Method Complementary Function            */
/*                     ( They Remenber New Word Or Adjust Condidations, etc.) */
/*                                                                            */
/* FUNCTION:           PinduAdjust     : Adjust Candidate Frequency           */
/*                                                                            */
/*                     rem_pd1         : Adjust Candidate Frequecy of One     */
/*                                       Chinese Character                    */
/*                                                                            */
/*                     push_down_stack1: Fill The New Candidate into Candidate*/
/*                                       Frequency Area                       */
/*                                                                            */
/*                     rem_pd2         : Adjust Candidate Frequency of Two    */
/*                                       Chinese Character                    */
/*                                                                            */
/*                     rem_pd3         : Adjust Candidate Frequency of Three  */
/*                                       Chinese Character                    */
/*                                                                            */
/*                     rem_new_word    : Fill The New Word In Temp_Rem_Area   */
/*                                                                            */
/*                     AddExtLib       : Get A Temperary Rem Word Into High   */
/*                                       Level Depanding How Many Times It    */
/*                                       Has Been Used                        */
/*                                                                            */
/*                     write_new_word  : Rewrit Temporary Remember area in    */
/*                                       $(HOME)/.abcusrrem                   */
/*                                                                            */
/*                     SendBackMsg     : Remember New Term                    */
/*                                                                            */
/*                     new_word        : Remember New Term                    */
/*                                                                            */
/*                     TempRemProc     : Save The Output In Logging_stack     */
/*                                                                            */
/*                     if_multi_rem    : Find a Term Has Already Remember     */
/*                                                                            */
/*                                                                            */
/************************* END OF SPECIFICATION *******************************/


#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "cjk.h"
#include "data.h"
#include "zhed.h"
#include "zhedinit.h"
#include "extern.h"


BYTE stack1_move_counter;
extern writefile();
extern WORD last_item_name;
extern BYTE UpdateFlag;
extern LPSTR jiyi_wenjian_cuo;
extern unsigned char by_cchar_flag;
extern FEPCB *fep;

/******************************************************************************/
/* FUNCTION    : PinduAdjust                                                  */
/* DESCRIPTION : Adjust Candidate Frequency                                   */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : 0 is Not Adjust Frequency                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
PinduAdjust()
{
    if (auto_mode!=1)              /* Not Allow to Adjust the Cand. Frequency */
        return(0);                          
    if (group_no<=1)               /* No Candidate or Only One Candidate      */
        return(0);
    if (unit_length>6)             /* Each Candidate Has More Than Three Char.*/
        return(0);
    if (msg_type&2)                /* Cand. is Mix of Chinese and English Char.*/
        return(0);

    switch (unit_length)           /* Adjust Frequency                        */
    {
        case 2:                    /* One Chinese Character                   */
            rem_pd1(&out_svw[current_no*unit_length]);
            break;
        case 4:                    /* Two Chinese Characters                  */
            rem_pd2(&out_svw[current_no*unit_length]);
            break;
        default:                   /* Three Chinese Characters                */
            rem_pd3(&out_svw[current_no*unit_length]);
            break;
    }

}


/******************************************************************************/
/* FUNCTION    : rem_pd1                                                      */
/* DESCRIPTION : Adjust Candidate Frequecy of One Chinese Character           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : buffer                                                       */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
rem_pd1(buffer)
WORD *buffer;
{
    lseek(fep->fd.abcusrfd[USRREM], 0, 0);
    if (lockf(fep->fd.abcusrfd[USRREM], F_LOCK, sizeof(tmmr.stack1)) == 0)
    {
        if ((wp.bx_stack[wp.xsyjs] == 0) && (wp.cmp_stack[wp.xsyjs]==4) ) 
                   /* Radical With Strokes Append Simple Spelling Flag       */
    {
            lseek(fep->fd.abcusrfd[USRREM], 0, 0);
            lockf(fep->fd.abcusrfd[USRREM], F_ULOCK, sizeof(tmmr.stack1));
            return(0); /* Radical is Only Initial Consonant Not Adjust Frequency */
}
        push_down_stack1();
        tmmr.stack1[0]=buffer[0];
    }
    lseek(fep->fd.abcusrfd[USRREM], 0, 0);
    lockf(fep->fd.abcusrfd[USRREM], F_ULOCK, sizeof(tmmr.stack1));
    return(0);

}

/******************************************************************************/
/* FUNCTION    : push_down_stack1                                             */
/* DESCRIPTION : Fill The New Candidate into Candidate Frequency Area         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
push_down_stack1()
{
    int i;

    i=sizeof(tmmr.stack1)-2;
    memmove((BYTE *)&tmmr.stack1[1],(BYTE *)&tmmr.stack1[0],i);
    stack1_move_counter++;

    /* This Cand. Needs To Adjust Frequency More Than Four Times, Rewrite This*/
    /* Cand. Frequency                                                        */
    if (stack1_move_counter>=4)   
        writefile(0l, &tmmr.stack1, sizeof tmmr.stack1);
    return(0);

}

/******************************************************************************/
/* FUNCTION    : rem_pd2                                                      */
/* DESCRIPTION : Adjust Candidate Frequecy of Two Chinese Character           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : buffer                                                       */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
rem_pd2(buffer)
WORD *buffer;
{
    int i;

    lseek(fep->fd.abcusrfd[USRREM], sizeof(tmmr.stack1), 0);
    if (lockf(fep->fd.abcusrfd[USRREM], F_LOCK, sizeof(tmmr.stack2)) == 0)
    {
        i=sizeof(tmmr.stack2)-8;
        memmove((BYTE *)&tmmr.stack2[2],(BYTE *)&tmmr.stack2[0],i);
        tmmr.stack2[0]=buffer[0];
        tmmr.stack2[1]=buffer[1];
        writefile((LONG)sizeof tmmr.stack1, tmmr.stack2, sizeof tmmr.stack2);
    }
    lseek(fep->fd.abcusrfd[USRREM], sizeof(tmmr.stack1), 0);
    lockf(fep->fd.abcusrfd[USRREM], F_ULOCK, sizeof(tmmr.stack2));

}

/******************************************************************************/
/* FUNCTION    : rem_pd3                                                      */
/* DESCRIPTION : Adjust Candidate Frequecy of Three Chinese Character         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : buffer                                                       */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
rem_pd3(buffer)
WORD *buffer;
{
    int i;
    i=sizeof(tmmr.stack3)-6;
    lseek(fep->fd.abcusrfd[USRREM], sizeof(tmmr.stack1)+sizeof(tmmr.stack2), 0);
    if (lockf(fep->fd.abcusrfd[USRREM], F_LOCK, sizeof(tmmr.stack3)) == 0)
    {
        memmove((BYTE *)&tmmr.stack3[3],(BYTE *)&tmmr.stack3[0],i);
        for (i=0; i<3; i++)
            tmmr.stack3[i]=buffer[i];
        writefile((LONG)(sizeof tmmr.stack1+sizeof tmmr.stack2),
            tmmr.stack3, sizeof tmmr.stack3);
    }
    lseek(fep->fd.abcusrfd[USRREM], sizeof(tmmr.stack1)+sizeof(tmmr.stack2), 0);
    lockf(fep->fd.abcusrfd[USRREM], F_ULOCK, sizeof(tmmr.stack3));

}


/******************************************************************************/
/* FUNCTION    : rem_new_word                                                 */
/* DESCRIPTION : Fill The New Word In Temp_rem_area                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
rem_new_word()
{
    WORD i,count;                          
    WORD *result_area_p;


    if(AbcUsrDictFileChange(USRREM, sizeof(tmmr.stack1)+sizeof(tmmr.stack2)+
      sizeof(tmmr.stack3), sizeof(tmmr.temp_rem_area), tmmr.temp_rem_area)==-1)
         return;

    count=(sizeof tmmr.temp_rem_area)-(new_no+2); 
    count=(sizeof tmmr.temp_rem_area)-(new_no+2); 
    memmove(&tmmr.temp_rem_area[(new_no+2)/2],     /* Fill The New Term In The*/
        tmmr.temp_rem_area,                        /* Termporary Remember Area*/
        count);

    tmmr.temp_rem_area[0]=new_no*0x100+jiyi_pindu;

    result_area_p=(WORD *)result_area;
    for (i=0; i<new_no/2; i++)
        tmmr.temp_rem_area[i+1]=result_area_p[i];

    write_new_word(1);                 /* Rewrite The Temporary Remember Area */

}

/******************************************************************************/
/* FUNCTION    : AddExtLib                                                    */
/* DESCRIPTION : Get a Temperary Rem Word Into High Level Depanding How Many  */
/*               Times It Has Been Used                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
extern hWnd;
AddExtLib()
{
    int x,count,i;
    BYTE *ptr1, *ptr2;

    if((unit_length>=4)&&(unit_length<=18))              /*rem word limited*/
        if(msx_area[current_no].from==2)
        {                                                /* If a rem word? */
            x=msx_area[current_no].addr/2;         /*get word addr in temp */

            if (x<sizeof(tmmr.temp_rem_area)/2) 
            {                                      /*Insure random errof out limited*/
                if ((tmmr.temp_rem_area[x]&0xff00)!=unit_length * 0x100)
                    return(STC);

                                                   /* ADD By WU JIAN      
                ptr1 = (BYTE *)(msx_area[current_no].from + tmmr.temp_rem_area);
                ptr2 = (BYTE *)out_svw + current_no * unit_length;
                for ( i = 0; i < unit_length; i++ ) 
                    if ( ptr1[i] != ptr2[i] )
                        return(STC);               */ /* End Add             */

                tmmr.temp_rem_area[x]+=1;           /* Increase used times.*/

                if((tmmr.temp_rem_area[x]&0xf)>=0x3)
                {                                         /* If used times>=3 */
                    count=(sizeof(tmmr.rem_area))
                        -(unit_length+2);              /* Push down middle rem area */
                    memmove(&tmmr.rem_area[(unit_length+2)/2],
                        tmmr.rem_area, count);

                    for(i=0; i<unit_length/2+1; i++)
                        tmmr.rem_area[i]=tmmr.temp_rem_area[x+i]; /* move to middle rem area.*/

                    count=sizeof(tmmr.temp_rem_area)-x*2
                        -(unit_length+2);                 /* delete it from temp */
                    memmove(&tmmr.temp_rem_area[x],
                        &tmmr.temp_rem_area[x+unit_length/2+1], count);

                    write_new_word(0);
                }/*#3*/ /*write changes*/
                else
                    write_new_word(1);
            }/*#2*/

        }/*#1*/
}/*#0*/

/******************************************************************************/
/* FUNCTION    : write_new_word                                               */
/* DESCRIPTION : Rewrit Temporary Remember area in $(HOME)/.abcusrrem         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : flag                                                         */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
write_new_word(flag)
int flag;
{
    int count;
    LONG distance;
    WORD *p;

    /* Get The Distance Pointer, Bytes Sum and Buffer Pointer */
    distance=sizeof(tmmr.stack1)+sizeof(tmmr.stack2)+sizeof(tmmr.stack3);
    count=sizeof(tmmr.temp_rem_area);
    p=tmmr.temp_rem_area;

    if (flag!=1)
    {   
        distance=sizeof(tmmr.stack1)+sizeof(tmmr.stack2)
                   +sizeof(tmmr.stack3)+sizeof(tmmr.temp_rem_area);
        p=tmmr.rem_area;
        if (flag<1)
        {
           distance=sizeof(tmmr.stack1)+sizeof(tmmr.stack2)+sizeof(tmmr.stack3);
           p=tmmr.temp_rem_area;
           count=sizeof(tmmr.temp_rem_area)+sizeof(tmmr.rem_area);
        }
    }
    writefile(distance,p,count);          /* Write $(HOME)/.abcusrrem */
}


/******************************************************************************/
/* FUNCTION    : SendBackMsg                                                  */
/* DESCRIPTION : Remember New Term                                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

SendBackMsg()
{
    if (new_no<=2)                       /* New Term Length < 1 Chinese Char */
        return(0);
    if (new_no>18)                       /* New Term Length > 9 Chinese Char */
        return(0);
    lseek(fep->fd.abcusrfd[USRREM], sizeof(tmmr.stack1)+sizeof(tmmr.stack2)+sizeof(tmmr.stack3), 0);
    if ( lockf(fep->fd.abcusrfd[USRREM], F_LOCK, sizeof(tmmr.rem_area)) == 0 )
        new_word();                      /* Remember New Term    */
    lseek(fep->fd.abcusrfd[USRREM], sizeof(tmmr.stack1)+sizeof(tmmr.stack2)+sizeof(tmmr.stack3), 0);
    lockf(fep->fd.abcusrfd[USRREM], F_ULOCK, sizeof(tmmr.rem_area));
    return(0);

}

/******************************************************************************/
/* FUNCTION    : new_word                                                     */
/* DESCRIPTION : Remember New Term                                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
new_word()
{
    WORD temp_save,suc_flag,i,j;

    by_cchar_flag=0;                 /* Reset Some Convert Parameters        */
    wp.xsyjw=0;
    temp_save=cp_ajust_flag;

    suc_flag=0;

    
    if (convert(0))                  /* Convert Successful                   */
    {
        cp_ajust_flag=temp_save;
        if (unit_length==new_no)     /* New Term Length Equal to this Convert */
        {                            /* Compare the New Term With This Convert*/
            for (i=0; i<group_no; i++)
            {
                suc_flag=0;
                for(j=0; j<new_no; j++)
                    if (result_area[j]!=out_svw[i*unit_length+j])
                        suc_flag=1;  /* The New Term Does Not Needed         */
                if (!suc_flag)
                    return(0);
            }
        }
    }

    cp_ajust_flag=temp_save;
    by_cchar_flag=1;
    return(rem_new_word());         /* Remember This New  Term              */

}


/******************************************************************************/
/* FUNCTION    : TempRemProc                                                  */
/* DESCRIPTION : Save The Output In Logging_stack                             */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
TempRemProc()
{
    int c,i;

    if (in.true_length<2)
        if (in.info_flag!=1)
            return(0);

    if (biaodian_value)
        c=result_area_pointer-2+1;             /* -2 biaodian isn't consider */
    else                                       /* +1 logging_stack struck is */
        c=result_area_pointer+1;               /*   result_area_pointer plus */
                                               /*    one byte counter        */

    memmove(&logging_stack[c],&logging_stack[0],(logging_stack_size-c));

    logging_stack[0]=c-1;                      /* length of storing string   */
    for (i=0; i<logging_stack[0]; i++)
        logging_stack[i+1]=result_area[i];

    if_multi_rem(c);
    return(0);

}

/******************************************************************************/
/* FUNCTION    : if_multi_rem                                                 */
/* DESCRIPTION : Find a Term Has Already Remember                             */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
if_multi_rem(c)
int c;
{
    BYTE cmp_buffer[25]={0};                 /* max input is 10 Chinese words */
    int i,cn;
    char *p;

    for (i=0; i<c; i++)
        cmp_buffer[i]=logging_stack[i];
    cmp_buffer[i]=0;

    p=strstr(&logging_stack[c],cmp_buffer);
    if (p!=NULL)
    {
        c=p-logging_stack;
        cn=logging_stack[c]+1;              /* cn is the length a group in logging_stack */
        memmove(&logging_stack[c],&logging_stack[c+cn],logging_stack_size-c-cn);
    }
    return(0);
}
