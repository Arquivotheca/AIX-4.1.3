static char sccsid[] = "@(#)02	1.2  src/bos/usr/lib/nls/loc/CN.im/cnedabcwp.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:16";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CwpProc
 *		FindHanzi
 *		abbr
 *		abbr_entry
 *		abbr_s1
 *		cisu_to_py
 *		cmp_2_and_3
 *		cmp_a_slbl
 *		cmp_a_slbl_with_bx
 *		cmp_bx1
 *		cmp_bx2
 *		cmp_first_letter
 *		cmp_long_word2
 *		convert
 *		copy_input
 *		count_basic_pera
 *		czcx
 *		detail_analyse
 *		fczs1
 *		fenli_daxie
 *		find_long_word2
 *		find_multy_hi
 *		find_next
 *		find_one_hi
 *		find_three_hi
 *		find_two_hi
 *		fu_sm
 *		get_head
 *		get_the_one
 *		get_the_one2
 *		getattr
 *		if_already_in
 *		if_code_equ
 *		input_msg_type
 *		look_for_code
 *		normal
 *		normal_1
 *		order_result2
 *		paidui
 *		pre_cmp
 *		pre_nt_w1
 *		prepare_search1
 *		read_kzk_lib
 *		rzw
 *		sc_gb
 *		sc_gbdy
 *		search_and_read
 *		sfx_proc
 *		slbl
 *		trs_new_word
 *		user_definition
 *		w1_no_tune
 *		yjbx
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************* START OF MODULE SPECIFICATION **********************/
/*                                                                            */
/* MODULE NAME:        cnedabcwp                                              */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method For Pinyin Mode                   */
/*                                                                            */
/* FUNCTION:           CwpProc          : Entry Point of Converting Process   */
/*                                                                            */
/*                     find_next        : Entry Point of Continue Convert     */
/*                                                                            */
/*                     normal           : Entry Point of Regular  Convert     */
/*                                                                            */
/*                     normal_1         : Other Entry Point of Regular Convert*/
/*                                                                            */
/*                     user_definition  : Produce The User Define Word        */
/*                                                                            */
/*                     detail_analyse   : Divide Input String Into Convert    */
/*                                        Cell and Cmp.type                   */
/*                                                                            */
/*                     slbl             :Divide Input String Into Convert cell*/
/*                                                                            */
/*                     getattr          : Get Characters attribute            */
/*                                                                            */
/*                     Covert           : Convert Cells in sb in Chinese      */
/*                                                                            */
/*                     copy_input       : Copy Struct into kbf Struct         */
/*                                                                            */
/*                     input_msg_type   : Get Compare Type for Each Cell      */
/*                                                                            */
/*                     pre_nt_w1        : Single Word Process                 */
/*                                                                            */
/*                     w1_no_tune       : Get Normal Single Word              */
/*                                                                            */
/*                     sc_gb            : Search The Gb2312 Set Area          */
/*                                                                            */
/*                     sc_gbdy          : Search Multi_Voice Area             */
/*                                                                            */
/*                     get_the_one      : Get One Chinese Char from Single    */
/*                                        Voice area                          */
/*                                                                            */
/*                     cmp_bx1          : Compare Writen Stroken              */
/*                                                                            */
/*                     get_the_one2     : Get One Chinese Char from Multy     */
/*                                        Voice area                          */
/*                                                                            */
/*                     cmp_bx2          : Compare Writen Stroken              */
/*                                                                            */
/*                     paidui           : Sort The Results                    */
/*                                                                            */
/*                     fu_sm            : Deels With CH, SH, ZH               */
/*                                                                            */
/*                     find_one_hi      : Get Used Frequency Adjust Word      */
/*                                                                            */
/*                     czcx             : Search for Request Words From       */
/*                                        Remembered Area                     */
/*                                                                            */
/*                     find_multy_hi    : Search High Frequeny Words          */
/*                                                                            */
/*                     find_two_hi      : Search for High Frequeny Words of   */
/*                                        Two Chinese Characters              */
/*                                                                            */
/*                     find_three_hi    : Search for High Frequeny Words of   */
/*                                        Three Chinese Characters            */
/*                                                                            */
/*                     cmp_2_and_3      : Compare Word When Length is 2 or 3  */
/*                                                                            */
/*                     FindHanzi        : Get Chinese Words                   */
/*                                                                            */
/*                     prepare_search1  : Read Dictionary                     */
/*                                                                            */
/*                     search_and_read  : Get Index Character and Read from   */
/*                                        Dictionary                          */
/*                                                                            */
/*                     if_already_in    : Adjust If The Page Has Already In   */
/*                                        The Memory                          */
/*                                                                            */
/*                     count_basic_pera : Count The Sub_library Address, page */
/*                                        Address, And read_write Length      */
/*                                                                            */
/*                     read_kzk_lib     : Search The Expended Lib             */
/*                                                                            */
/*                     abbr_s1          : Find Match Words Arrcoding To The   */
/*                                        Given Input Message                 */
/*                                                                            */
/*                     fczs1            : Search New Words From Temperoy      */
/*                                        Remembered area                     */
/*                                                                            */
/*                     find_long_word2  : Find Word When Word Length is 2 or  */
/*                                        More                                */
/*                                                                            */
/*                     trs_new_word     : Transmit New Words                  */
/*                                                                            */
/*                     pre_cmp          : Do Prepare Jobs Before Search       */
/*                                                                            */
/*                     cmp_a_slbl_with_bx:Compare a Syllable With Strokens    */
/*                                                                            */
/*                     cmp_a_slbl       : Compare a Syllable                  */
/*                                                                            */
/*                     cmp_first_letter : Compare First Letter                */
/*                                                                            */
/*                     cisu_to_py       : Changed Word Cell Code ( cisu ) to  */
/*                                        Pinyin                              */
/*                                                                            */
/*                     get_head         : Get Chinese syllable internal code  */
/*                                                                            */
/*                     yjbx             : Compare a Syllable with Radicals    */
/*                                                                            */
/*                     abbr_entry       : Entry Pointer of Convert Search     */
/*                                                                            */
/*                     cmp_long_word2   : Compare Operate for Word Length     */
/*                                        More Than 1                         */
/*                                                                            */
/*                     order_result2    : Sort Results                        */
/*                                                                            */
/*                     fenli_daxie      : Sperate Capital Letters             */
/*                                                                            */
/*                     rzw              : Match Word Suffix or Prefix         */
/*                                                                            */
/*                     abbr             : Entry Point of Convert While First  */
/*                                        Letter is Caps.                     */
/*                                                                            */
/*                     sfx_proc         : Entry Point of Deals With Prefix    */
/*                                        Suffix                              */
/*                                                                            */
/*                     look_for_code    : Search If The Code Is In The Index  */
/*                                                                            */
/*                     if_code_equ      : Search If The Code In The Index     */
/*                                                                            */
/*                                                                            */
/************************* END OF SPECIFICATION *******************************/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "cned.h"
#include "cjk.h"
#include "data.h"
#include "extern.h"

#define TRUE    1
#define FALSE   0
#define NUMBER  0x20
#define FUYIN   0x21
#define YUANYIN  0x22
#define SEPERATOR  0x27
#define FIRST_T    1
#define SECOND_T   2
#define THIRD_T    3
#define FORTH_T    4

/* about search strutagy */
#define BX_FLAG         8
#define JP_FLAG         4
#define QP_FLAG         2
#define YD_FLAG         1

/* about search lib */
#define BODY_START              0
#define KZK_BODY_START          0
#define KZK_BASE                0xa000l
#define MORE_THAN_5             23
#define TMMR_REAL_LENGTH        0x1800

/* mark for test*/
#define TEST                    0


struct SLBL{
    WORD value;
    BYTE head;
    WORD length;
    BYTE tune;
    BYTE bx1;
    WORD bx2;
    BYTE flag;
}sb;

struct N_SLBL{
    BYTE buffer[30];
    int length;
}neg;

/* BYTE slbl_tab[]="ZH00\1"
"SH00\2"
"CH00\3"
"ING0\4"
"AI00\5"
"AN00\6"
"ANG0\7"
"AO00\x8"
"EI00\x9"
"EN00\xa"
"ENG0\xb"
"IA00\xc"
"IAN0\xd"
"IANG\xe"
"IAO0\xf"
"IE00\x10"
"IN00\x11"
"IU00\x12"
"ONG0\x13"
"OU00\x14"
"UA00\x15"
"UAI0\x16"
"UAN0\x17"
"UE00\x18"
"UN00\x19"
"UENG\x1a"   
"UI00\x1b"
"UO00\x1c"
"UANG\x1d"
"ER00\x1e"
"IONG\x1f"
"VE00\x18"
"UEN0\x19"
"VEN0\x19"
"UEI0\x1b"
"IOU0\x12";
*/

BYTE buffer[30];

BYTE cmp_head,cmp_state,cmp_bx,by_cchar_flag;
WORD cmp_yj,cmp_cisu;

/* about search lib */
LONG r_addr;
WORD out_svw_cnt,msx_area_cnt;
WORD search_start,search_end,kzk_search_start,kzk_search_end;
WORD item_length,kzk_item_length,last_item_name,item_addr,slib_addr;
BYTE word_lib_state;
WORD lib_w[0x800];
WORD kzk_lib_w[0x400];
BYTE auto_mode,word_source,xs_flag,sfx_attr,jiyi_pindu,system_info;
BYTE stack1_move_counter;
WORD extb_ps;

extern void RemPd1(char *);
extern void RemPd2(char *);
extern void RemPd3(char *);
extern BOOL IfAlphOrNot(BYTE);
extern IfNumberOrNot(BYTE);
extern void AbcCls();

/******************************************************************************/
/* FUNCTION    : CwpProc                                                      */
/* DESCRIPTION : Entry Point of Converting Process                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : mtype : Input String Type                                    */
/* OUTPUT      : -1 is Fault; 0 is OK                                         */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
CwpProc(mtype)
int mtype;
{
    int i, j, m;
    BYTE x;
    switch (mtype)
    {
        case 0:
            return(normal());                   /* normal pinyin convert*/
    
        case 1:
            abbr();                            /* ABBR */
            return(normal());

        case 2:                                 /* "I" capital chinese number */
        case 3:                                 /* "i" small chinese number */
            if (in.true_length==1)
            {
                in.buffer[1]='1';
                in.true_length++;
            }

            m=0;
            for(i=1; i<in.true_length; i++)
            {
                x=in.buffer[i];
                if (IfNumberOrNot(in.buffer[i]))
                {
                    if (mtype==2)
                        x=in.buffer[i]-0x30;
                }
                if (IfAlphOrNot(in.buffer[i]))
                {
                    x=in.buffer[i]&0xdf;
                    if (mtype==2)
                        if (x=='S' || x=='B' || x=='Q')
                            x=x|0x20;
                }

                for (j=0; j<160; j=j+3)    /* Search fk_tab to Translate Char.*/
                {                          /* in the in.buffer, results would */
                    if (x==fk_tab[j])      /* be sent to out_svw              */
                    {                  
                        out_svw[m++]=fk_tab[j+1];
                        out_svw[m++]=fk_tab[j+2];
                        x=0xff;              /* found it */
                        break;
                    }
                }
                if (x!=0xff)
                    goto err_back;
            } 

            group_no=1;
            unit_length=m;
            msg_type=2;
            return(1);                        /* success! */
    
        case 4:                               /* "u" user define word */
            return(user_definition());
    
        case 12:                               /* continue to change */
            return(find_next());
    
        case 13:                               /* backword */
            return(normal_1(word_back_flag));
    
        default:
err_back:
            return(-1);

    }/* switch */
}



/******************************************************************************/
/* FUNCTION    : find_next                                                    */
/* DESCRIPTION : Entry Point of Continue Converting                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : FALSE is Fault                                               */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
find_next()
{
    if (wp.yjs<=wp.xsyjw)              /* Converting Syllables <= total       */
        return(FALSE);

    normal_1(0);

}


/******************************************************************************/
/* FUNCTION    : normal                                                       */
/* DESCRIPTION : Entry Point of Regular  Convert                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : STC is Fault; CLC is OK                                      */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
normal()
{
    /* Initialize                 */
    extb_ps=0xffff;
    by_cchar_flag=0;
    wp.yjs=0;
    wp.xsyjw=0;
    wp.dw_count=0;
    wp.dw_stack[0]=0;

    if (in.info_flag==0x81)                  /* Convert Type is BY_CHAR      */
        by_cchar_flag=1;
    detail_analyse();                        /* To Do Converting Preare jobs */

    if (!convert(0))                         /* No Result is found           */
        return(STC);

    wp.xsyjs=wp.xsyjw;                       /* Set Convert Parameters       */
    wp.xsyjw+=word_long;
    wp.dw_stack[++wp.dw_count]=wp.xsyjw;

    if (by_cchar_flag!=1)                    /* Convert Type isn't BY_CHAR   */
    {
        if (wp.yjs==wp.xsyjw)
        {
            if (wp.xsyjs!=0)
            {
                sfx_attr=2;                   /* mark for finding sfx_table */
                rzw();
            }
        } 
        else
        {
            if (word_long<=1)
            {
                if (wp.xsyjs==0)
                {
                    sfx_attr=1;                /* mark for finding sfx_table */
                    rzw();
                } 
            } 
        }

    }

    if (wp.yjs<=wp.xsyjw)
        jiyi_mode=0;
    else
        jiyi_mode=1;

    return(CLC);

}

/******************************************************************************/
/* FUNCTION    : normal_1                                                     */
/* DESCRIPTION : Entry Point of Regular  Convert                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : flag                                                         */
/* OUTPUT      : STC is Fault; CLC is OK                                      */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
normal_1(flag)
int flag;
{
    if (in.info_flag==0x81)
        by_cchar_flag=1;

    if (!convert(flag))                         /* Converting                */
        return(STC);

    wp.xsyjs=wp.xsyjw;
    wp.xsyjw+=word_long;
    wp.dw_stack[++wp.dw_count]=wp.xsyjw;

    if (by_cchar_flag!=1)
    {
        if (wp.yjs==wp.xsyjw)
            if (wp.xsyjs!=0)
            {
                sfx_attr=2;                     /* mark for finding sfx_table*/
                rzw();
            }
        else
            if (word_long<=1)
                if (wp.xsyjs==0)
                {
                    sfx_attr=1;                /* mark for finding sfx_table */
                    rzw();
                }
    }


    if (wp.yjs<=wp.xsyjw)
        jiyi_mode=0;
    else
        jiyi_mode=1;

    return(CLC);

}

/******************************************************************************/
/* FUNCTION    : user_definition                                              */
/* DESCRIPTION : Produce The User Define Word                                 */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : STC is Fault; CLC is OK                                      */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
user_definition()
{
    int i,rec_cnt;

    kbf.max_length=in.max_length;
    kbf.true_length=in.true_length-1;               /*delete the word "u"*/

    for (i=0; i<kbf.true_length; i++)
        kbf.buffer[i]=in.buffer[i+1];

    read_mulu();
    if (!(rec_cnt=look_for_code()))
        return(STC);                         /* not found                    */
    if (!read_data(rec_cnt-1))               /* -1 get the real record count */
        return(STC);                         /* not found                    */

    unit_length=out_svw[0]-0x30;             /* plus 1 is plus the mark      */
    word_long=(out_svw[0]-0x30)/2;
    group_no=1;
    memmove(out_svw,&out_svw[2],unit_length);
    msg_type|=2;
    return(CLC);

}


/******************************************************************************/
/* FUNCTION    : detail_analyse                                               */
/* DESCRIPTION : Divide Input String into Convert Cell and Get cmp.type       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : STC is Fault; CLC is OK                                      */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
detail_analyse()
{
    int i=0,j=0;
    BYTE *p;

    copy_input();                   /* Copy The Struct into kbf Struct       */

    p=(BYTE *)kbf.buffer; 
    do {
        if (!slbl(p))               /* Divide Input String into Convert Cell */
            if (i==0)
                return(STC);
            else
                break;

        if (sb.length==0)
            break;

        p+=sb.length;
        if (LOBYTE(sb.value)=='V')
            if (sb.head=='J'||sb.head=='Q'||sb.head=='X')
                TO_LOW_BYTE(sb.value,'U');

        wp.syj[i]=sb.head;
        wp.bx_stack[i]=sb.bx1;
        wp.tone[i]=sb.tune;
        wp.yj[j]=sb.value;                          /* WORD transport */
        wp.yj_ps[j]=(int)(p-(BYTE *)kbf.buffer);

        i++, j++;
        if (i>=10)
            break;

        if (sb.flag==TRUE)
            break;

    } while(1);

    wp.yjs=i;

    input_msg_type();
}



/******************************************************************************/
/* FUNCTION    : slbl                                                         */
/* DESCRIPTION : Divide Input String Into Convert cell                        */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : s_buffer                                                     */
/* OUTPUT      : STC is Fault; CLC is OK                                      */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
slbl(s_buffer)
BYTE *s_buffer;
{

    int i=0,j;
    BYTE cmp_buffer[5]={ 0    };
    char *p;
    BYTE x,attr,y;

    /*    analize the initial consonant   */
    x=s_buffer[i++];
    attr=getattr(x,&x);                        /* get char's attribute */
    if (!attr)
    {
        sb.length=i-1;
        sb.flag=TRUE;
        return(1);
    }

    if (attr==NUMBER)
        return(0);                   /* error */
    if (x=='I' || x=='U' || x=='V')
        return(0);                   /* error */
    switch (attr) {                    
        case FIRST_T:
        case SECOND_T:
        case THIRD_T:
        case FORTH_T:
            return (0);            /* error */
    }                         

    sb.value=0;
    sb.head=0;
    sb.length=0;
    sb.tune=0;
    sb.bx1=0;
    sb.bx2=0;
    sb.flag=FALSE;

    sb.head=x;
    if (attr==FUYIN)
    {
        if (x=='Z' || x=='C' || x=='S' )
        {
            if ((s_buffer[i]&0xdf)==0x48)
            {
                for (j=0; j<15; j=j+5)
                    if (x==slbl_tab[j])
                    {
                        sb.value=(WORD)slbl_tab[j+4]<<8;
                        sb.head=slbl_tab[j+4];
                        break;
                    }
                i++;
            }
            else
                sb.value=(WORD)x<<8;
        }
        else
            sb.value=(WORD)x<<8;
    }
    else
    {
        i--;
        sb.value=0;
    } 


    /*    analize the simple or compound vowel of Chinese syllable   */
    x=s_buffer[i];
    attr=getattr(x,&x);
    if (!attr)
    {
        sb.length=i;
        sb.flag=TRUE;
        return(1);
    }

    if (attr==YUANYIN)
    {             /*if no vowel, goto step3 */

        TO_LOW_BYTE(sb.value,x);

        cmp_buffer[0]=x;
        i++;
        for (j=1; j<4; j++)
        {
            x=s_buffer[i++];
            attr=getattr(x,&x);
            if (!attr)
            {
                for (j=j; j<4; j++)
                    cmp_buffer[j]='0';
                i--;
                sb.flag=TRUE;
                break;
            }

            if (attr==NUMBER)
            {
                for (j=j; j<4; j++)
                    cmp_buffer[j]='0';
                i--;
                break;
            }
            else
                cmp_buffer[j]=x;
        }

        for (j=3; j>0; j--)
        {
            p=strstr(slbl_tab,cmp_buffer);    /* search the simple or compound*/                                              /*  vowel of Chinese syllable   */
            if (p!=NULL)
            {
                TO_LOW_BYTE(sb.value,*(p+4)); /* get the simple or compound   */
                                              /*vowel of Chinese syllable value*/
                break;
            }
            if (cmp_buffer[j]!=0x30)
            {
                cmp_buffer[j]='0';
                i--;
            }
        }
    }
    else 
        TO_LOW_BYTE(sb.value,0);


    /*   see if the next is vowel    */
    x=s_buffer[i];
    attr=getattr(x,&x);
    if (!attr){
        sb.length=i;
        sb.flag=TRUE;
        if (sb.value<=0xff)
            sb.value=sb.value<<8;
        return(1);
    }
    if (attr==YUANYIN)
    {
        y=s_buffer[i-1]&0xdf;
        if (y=='R'||y=='N'||y=='G')
        {
            i--;
            TO_LOW_BYTE(sb.value,0);
            for (j=3; j>0; j--)
            {
                if (cmp_buffer[j]!=0x30)
                {
                    cmp_buffer[j]='0';
                    break;
                }
            }
            for (j=3; j>0; j--)
            {
                p=strstr(slbl_tab,cmp_buffer); /*search the simple or compound*/                                               /* vowel of Chinese syllable   */
                if (p!=NULL)
                {
                    TO_LOW_BYTE(sb.value,*(p+4));/*get the simple or compound */
                                              /*vowel of Chinese syllable value*/
                    break;
                }
                if (cmp_buffer[j]!=0x30)
                {
                    cmp_buffer[j]='0';
                    i--;
                }
            }
            if (!(BYTE)sb.value)
                TO_LOW_BYTE(sb.value,cmp_buffer[0]);

        }/* if (y=='R') */
    }/* if (attr==YUANYIN) */

    if (sb.value<=0xff)
        sb.value=sb.value<<8;

    /*   analize the tune   */
    x=s_buffer[i];
    attr=getattr(x,&x);
    if (!attr){
        sb.length=i;
        sb.flag=TRUE;
        return(1);
    }
    if (attr==FIRST_T||attr==SECOND_T||attr==THIRD_T||attr==FORTH_T){
        sb.tune=attr;
        i++;
    }

    /*   analize the strock   */
    for (j=0; j<6; j++)
    {
        x=s_buffer[i++];
        attr=getattr(x,&x);
        if (!attr)
        {
            sb.flag=TRUE;
            sb.length=i-1;
            return(1);
        }

        if (attr==SEPERATOR)
        {
            sb.flag=FALSE;
            sb.length=i;
            return(1);            /* if the string has seperator, move the */
        }                         /* pointer to the beginning of next syllable */

        if (attr!=NUMBER)
        {
            do{ 
                if (attr==YUANYIN || attr==FUYIN)
                {
                    sb.flag=FALSE;
                    sb.length=i-1;
                    return(1);
                }

                x=s_buffer[i++];
                attr=getattr(x,&x);

                if (!attr)
                {
                    sb.flag=TRUE;
                    sb.length=i-1;
                    return(1);
                }
            }
            while(i<100);
            return (1);
        }      


        if (x>'0' && x<'9')
        {
            switch(j)
            {
                case 0:
                    sb.bx1=x<<4;
                    break;
                case 1:
                    sb.bx1+=x&0x0f;
                    break;
                case 2:
                    sb.bx2=(WORD)x;
                    sb.bx2<<=12;
                    break;
                case 3:
                    sb.bx2=sb.bx2+(WORD)((x&0xf)<<8);
                    break;
                case 4:
                    TO_LOW_BYTE(sb.bx2,x<<4);
                    break;
                case 5:
                    TO_LOW_BYTE(sb.bx2, sb.bx2+x&0xf);
                    break;
            }/* switch */

        }/* if(x) */

    }/* for */

    do {
        x=s_buffer[i++];
        attr=getattr(x,&x);
    
        if (!attr)
        {
            sb.flag=TRUE;
            sb.length=i-1;
            return(1);
        }
    
        if (attr==YUANYIN || attr==FUYIN)
        {
            sb.flag=FALSE;
            sb.length=i;
            return(1);
        }
    
    } while(i<100);
    
}



/******************************************************************************/
/* FUNCTION    : getattr                                                      */
/* DESCRIPTION : Get Characters attribute                                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : x : Character    p : Character pointer                       */
/* OUTPUT      : FALSE is Fault                                               */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
getattr(x,p)
BYTE x;
char *p;
{
    if (x==0)
        return(FALSE);

    if (IfNumberOrNot(x))             /* Is Number                 */
        return(NUMBER);

    if (IfAlphOrNot(x))               /* Is Alpha                  */
    {
        x=x&0xdf;
        *p=x;
        if (x=='A'||x=='E'||x=='I'||x=='O'||x=='U'||x=='V')
            return(YUANYIN);
        else
            return(FUYIN);
    }

    if (x==SEPERATOR)                /* Is Seperator Character    */
        return(SEPERATOR);


    switch(x)
    {
        case '-':
            return(FIRST_T);
        case '/':
            return(SECOND_T);
        case '~':
        case '^':
            return(THIRD_T);
        case '\\':
            return(FORTH_T);
    }
}



/******************************************************************************/
/* FUNCTION    : convert                                                      */
/* DESCRIPTION : Convert Cell in sb into Chinese                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : flag : Convert Type                                          */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
convert(flag)
int flag;
{
    int j;


    if (!flag)
        word_long=wp.yjs-wp.xsyjw;
    else
        word_long=flag;

    if (word_long>9)    
        word_long=9;   

    if (by_cchar_flag==1)
        word_long=1;

    if (word_long==1)
        return(pre_nt_w1(wp.xsyjw));

    for (j=word_long; j>1; j--)           /* Auto Match Word Operating       */
    {
        word_long=j;
        prepare_search1();                /* Read Dictionary                 */
        abbr_s1();
        if (group_no)
        {
            unit_length=j*2;
            return(CLC);                  /* Success                        */
        }
    }

    return(pre_nt_w1(wp.xsyjw));          /* Process Single Word           */
}

/******************************************************************************/
/* FUNCTION    : copy_input                                                   */
/* DESCRIPTION : Copy Struct into kbf Struct                                  */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
copy_input()
{
    int i=0, j=0;

    if (in.info_flag=='*')                  /* if "*" */
        return(0);

    kbf.true_length=in.true_length;
    if (in.buffer[0]==SEPERATOR)
    {
        kbf.true_length=in.true_length-1;
        i=1;
    }

    for (i=i; i<in.true_length+2; i++)
        kbf.buffer[j++]=in.buffer[i];

    kbf.max_length=in.max_length;
    kbf.info_flag=in.info_flag;

}

/******************************************************************************/
/* FUNCTION    : input_msg_type                                               */
/* DESCRIPTION : Get Compare Type for Each Cell                               */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
input_msg_type()
{
    int i;

    for (i=0; i<wp.yjs; i++)
    {
        wp.cmp_stack[i]=QP_FLAG;
        if (LOBYTE(wp.yj[i])==0)
            if (wp.syj[i]==HIBYTE(wp.yj[i]))
                wp.cmp_stack[i]=JP_FLAG;

        if (wp.tone[i]!=0)
            wp.cmp_stack[i]+=YD_FLAG;  

        if (wp.bx_stack[i]!=0)
            wp.cmp_stack[i]|=BX_FLAG;
    }
}

/******************************************************************************/
/* FUNCTION    : pre_nt_w1                                                    */
/* DESCRIPTION : Single Word Process                                          */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : 0 is Not Adjust Frequency                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
pre_nt_w1(ps)
int ps;
{
    unit_length=2;              /* Send Parameters                           */
    word_long=1;
    cmp_yj=wp.yj[ps];
    cmp_head=wp.syj[ps];
    cmp_bx=wp.bx_stack[ps];
    cmp_state=wp.cmp_stack[ps];

    find_one_hi();

    w1_no_tune();               /* Get Normal Single Word                    */
    return(group_no);

}

/******************************************************************************/
/* FUNCTION    : w1_no_tune                                                   */
/* DESCRIPTION : Get Normal Single Word                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
w1_no_tune()
{
    out_svw_cnt=0;
    sc_gb();
    sc_gbdy();
    group_no=out_svw_cnt;
    paidui(group_no);                     /* Sort Results                   */

}

/******************************************************************************/
/* FUNCTION    : sc_gb                                                        */
/* DESCRIPTION : Search The GB2312 SET Area                                   */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
sc_gb()
{
    BYTE x;
    int cnt,i;

    cnt=(87-15)*94;
    if (cmp_state&4)
    {
        if (cmp_bx==0)
        {
            x=HIBYTE(cmp_yj);
            if (x!='A'&&x!='O'&&x!='E')
            {
                cnt=(55-15)*94;
                cmp_state=cmp_state|0x80;
            }
            else
                cmp_state=(cmp_state&0xfb)|QP_FLAG;

        }
    }

    for (i=0; i<cnt; i++)
    {
        if (cmp_yj==cisu->t_bf1[i])
            get_the_one(i);
        else
        {
            if (cmp_state&4)
            {
                if (cmp_head==HIBYTE(cisu->t_bf1[i]))
                    get_the_one(i);
                else                     /* Deels With CH, SH, ZH            */
                    if ( cmp_head==fu_sm(HIBYTE(cisu->t_bf1[i])) )
                        get_the_one(i);
            }
        }
    }

    return(0);

}

/******************************************************************************/
/* FUNCTION    : sc_gbdy                                                      */
/* DESCRIPTION : Search Multi_Voice Area                                      */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
sc_gbdy()
{
    int cnt,i;

    cnt=cisu->t_bf_start[2]/2;
    for (i=0; i<cnt; i=i+2)
    {
        if (cmp_yj==cisu->t_bf2[i])
            get_the_one2(i);  /* Get One Chinese Char from Multy Voice area  */
        else
        {
            if (cmp_state&4)
            {
                if (cmp_head==HIBYTE(cisu->t_bf2[i]))
                    get_the_one2(i);/* Get One Chinese Char from Multy Voice area  */
                else 
                {                         /* Deels With CH, SH, ZH            */
                    if (cmp_head==fu_sm(HIBYTE(cisu->t_bf2[i])))
                        get_the_one2(i); /* Get One Chinese Char from Multy Voice area  */
                }
            }
        }
    }

    return(0);

}

/******************************************************************************/
/* FUNCTION    : get_the_one                                                  */
/* DESCRIPTION : Get One Chinese Char from Single Voice area                  */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
get_the_one(i)
int i;
{
    BYTE x;

    WORD *out_svw_p=(WORD *)out_svw;
    BYTE *msx_p=(BYTE *)msx_area;

    if (cmp_bx1(i)!=0)            /* Compare Writen Stroken                   */
        return(0);

    out_svw_p[out_svw_cnt]=i+0x2020;

    if (i>=(55-16+1)*94)
        x=0x20;
    else
        x=pindu.pd_bf1[i];

    if (cmp_state&0x80)
        if (x<=(154+50))
            return(0);
    msx_p[out_svw_cnt]=x;
    out_svw_cnt++;
    return(0);

}

/******************************************************************************/
/* FUNCTION    : cmp_bx1                                                      */
/* DESCRIPTION : Compare Writen Stroken                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
cmp_bx1(i)
int i;
{
    BYTE x;

    if (cmp_bx==0)
        return(0);

    x=spbx_tab[i];
    if (x==cmp_bx)
        return(0);

    x=x&0xf0;
    if (x==cmp_bx)
        return(0);

    return(1);

}


/******************************************************************************/
/* FUNCTION    : get_the_one2                                                 */
/* DESCRIPTION : Get One Chinese Char from Multy Voice area                   */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
get_the_one2(i)
int i;
{
    BYTE x;

    WORD *out_svw_p=(WORD *)out_svw;
    BYTE *msx_p=(BYTE *)msx_area;

    if (cmp_bx2(i)!=0)                 /* Compare Writen Stroken              */
        return(0);

    out_svw_p[out_svw_cnt]=i+0x8000;

    if ((i/2)>=pindu.pd_bf0[2])
        x=0x20;
    else
        x=pindu.pd_bf2[i/2];

    if (cmp_state&0x80)
        if (x<=(154+50))
            return(0);
    msx_p[out_svw_cnt]=x;
    out_svw_cnt++;
    return(0);

}

/******************************************************************************/
/* FUNCTION    : cmp_bx2                                                      */
/* DESCRIPTION : Compare Writen Stroken                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
cmp_bx2(i)
int i;
{
    BYTE x;
    WORD y;

    if (cmp_bx==0)
        return(0);

    i++;
    y=cisu->t_bf2[i];
    y=((BYTE)y-0xb0)*94+(HIBYTE(y)-0xa1);

    x=spbx_tab[y];
    if (x==cmp_bx)
        return(0);

    x=x&0xf0;
    if (x==cmp_bx)
        return(0);

    return(1);

}

/******************************************************************************/
/* FUNCTION    : paidui                                                       */
/* DESCRIPTION : Sort The Results                                             */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : cnt : count                                                  */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
paidui(cnt)
int cnt;
{
    int i,j,n,flag;
    BYTE x1,y1;
    WORD x,y;
    WORD *out_p;
    BYTE *msx_p=(BYTE *)msx_area;

    out_p=(WORD *)out_svw;
    if (cnt<=1)
        return(0);

    for (n=cnt-1; n>0; n--)
    {
        flag=0;
        for (i=0; i<n; i++)
        {
            if (msx_p[i]==msx_p[i+1])
            {
                if (out_p[i]>out_p[i+1])
                {
                    x=out_p[i];
                    out_p[i]=out_p[i+1];
                    out_p[i+1]=x;
                    flag++;
                }
            }
            else
            {
                if (msx_p[i]<msx_p[i+1])
                {
                    x1=msx_p[i];
                    msx_p[i]=msx_p[i+1];
                    msx_p[i+1]=x1;
                    x=out_p[i];
                    out_p[i]=out_p[i+1];
                    out_p[i+1]=x;
                    flag++;
                }
            }
        }/* for(i) */
        if (flag==0)
            break;

    }/* for(n)*/
}


/******************************************************************************/
/* FUNCTION    : fu_sm                                                        */
/* DESCRIPTION : Deels With CH, SH, ZH                                        */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : fy                                                           */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
fu_sm(fy)
BYTE fy;
{
    switch(fy)
    {
        case 1:
            return('Z');
        case 2:
            return('S');
        case 3:
            return('C');
        default:
            return(fy);
    }
}



/******************************************************************************/
/* FUNCTION    : find_one_hi                                                  */
/* DESCRIPTION :                                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
find_one_hi()
{
    WORD foh_save=0;
    int i;

    if (!cmp_bx)
        if (cmp_state&4)
            return(0);

    cp_ajust_flag=0;

    i=0;
    do {
        if (czcx(&tmmr.stack1[i]))     /* Search for Request Words From       */
                                       /* Remembered Area                     */
        {
            cmp_cisu = tmmr.stack1[i];
            if (!foh_save)
                foh_save=cmp_cisu;
            else
            {
                if (foh_save==cmp_cisu)
                {
                    cp_ajust_flag=1;
                    result_area[result_area_pointer++]=HIBYTE(cmp_cisu) & 0xBF;
                    result_area[result_area_pointer++]=LOBYTE(cmp_cisu);
                    return;
                }
            }
        }
        i++;
    } while( i < (sizeof(tmmr.stack1)) / 2 );

}

/******************************************************************************/
/* FUNCTION    : czcx                                                         */
/* DESCRIPTION : Search for Request Words From Remembered Area                */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : stact                                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
czcx(stack)
WORD *stack;
{

    cmp_cisu=stack[0];

    if (!cmp_cisu)
        return(STC);

    if (cmp_bx)
        if (HIBYTE(cmp_cisu)&0x40)
            cmp_cisu&=0xbfff;
        else
            return(STC);

    if (cmp_a_slbl_with_bx())          /* Compare a Syllable With Strokens    */
        return(CLC);
    return(STC);

}

/******************************************************************************/
/* FUNCTION    : find_multy_hi                                                */
/* DESCRIPTION : Search High Frequeny Words                                   */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
find_multy_hi()
{
    if (word_long==2)
             /* Search for High Frequeny Words of Two Chinese Characters     */
        find_two_hi(); 
    if (word_long==3)
             /* Search for High Frequeny Words of Three Chinese Characters   */
        find_three_hi();
    return(0);

}

/******************************************************************************/
/* FUNCTION    : find_two_hi                                                  */
/* DESCRIPTION : Search for High Frequeny Words of Two Chinese Characters     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
find_two_hi()
{
    int i,j;
    WORD *result_p;

    result_p=(WORD *)result_area;

    for (i=0; i<(sizeof tmmr.stack2)/2; i=i+2)
    {
        if (!tmmr.stack2[i])
        {
            cp_ajust_flag=0;
            return(0);
        }

        if (cmp_2_and_3(&tmmr.stack2[i])) /*Compare Word When Length is 2 or 3*/
        {
            cp_ajust_flag=1;
            for (j=0; j<word_long; j++)
                result_p[result_area_pointer/2+j]=tmmr.stack2[j+i]&0xbfff;
            result_area_pointer += word_long*2;
            return(0);
        }
    }

    cp_ajust_flag=0;
    return(0);

}

/******************************************************************************/
/* FUNCTION    : find_three_hi                                                */
/* DESCRIPTION : Search for High Frequeny Words of Three Chinese Characters   */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
find_three_hi()
{
    int i,j;
    WORD *result_p;

    result_p=(WORD *)result_area;

    for (i=0; i<(sizeof tmmr.stack3)/3; i=i+3)
    {
        if (!tmmr.stack3[i])
        {
            cp_ajust_flag=0;
            return(0);
        }

        if (cmp_2_and_3(&tmmr.stack3[i]))/*Compare Word When Length is 2 or 3*/
        {
            cp_ajust_flag=1;
            for (j=0; j<word_long; j++)
                result_p[result_area_pointer/2+j]=tmmr.stack3[j+i]&0xbfff;
            result_area_pointer+=word_long*2;
            return(0);
        }
    }

    cp_ajust_flag=0;
    return(0);

}

/******************************************************************************/
/* FUNCTION    : cmp_2_and_3                                                  */
/* DESCRIPTION : Compare Word When Length is 2 or 3                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
cmp_2_and_3(t_stack)
WORD *t_stack;
{
    int i,yj_p;

    yj_p=wp.xsyjw;
    for (i=0; i<word_long; i++)
    {
        cmp_cisu=t_stack[i];
        pre_cmp(yj_p);               /* Do Prepare Jobs Before Search       */
        if (word_long==2)
            if (cmp_state&4)
                if (!(HIBYTE(cmp_cisu)&0x40))
                    return(STC);
                else
                    cmp_cisu&=0xbfff;
        if (!cmp_a_slbl_with_bx())   /* Compare a Syllable With Strokens    */
            return(STC);
        yj_p++;
    }
    return(CLC);

}


/******************************************************************************/
/* FUNCTION    : FindHanzi                                                    */
/* DESCRIPTION : Get Chinese Words                                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : x : ABC internal Code                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
FindHanzi(x)
WORD x;
{
    unsigned int i;

    if (x>0xa000 || x<0x2020)
        return(x);

    if (x>=0x8000)
    {
        x=(x-0x8000)+1;
        i = cisu->t_bf2[x];
        return( ((i & 0xff00) >> 8) | ((i & 0xff) <<8));
    }

    return(((x-0x2020)/94+0xb0)*0x100+(x-0x2020)%94+0xa1);
}

/******************************************************************************/
/* FUNCTION    : prepare_search1                                              */
/* DESCRIPTION : Read Dictionary                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
prepare_search1()
{
    BYTE f_ci1,f_ci2,x;

    f_ci1=wp.syj[wp.xsyjw];
    f_ci2=wp.syj[wp.xsyjw+1];
    f_ci1=fu_sm(f_ci1);                /* Deels With CH, SH, ZH            */
    f_ci2=fu_sm(f_ci2);

    if (word_long>=5)
        f_ci2=word_long;

    /* Get Index Character and Read from  Dictionary    */
    search_and_read(f_ci1,f_ci2);
/*
 After reading, counting the search place is needed
 First, count the STD dictionary buffers
*/
    search_start=6;
    search_end=6;
    if (word_lib_state&1)
    {
        x=word_long-2;
        if (x>=3){
            x-=3;
            search_start+=4;
        }

        if (x>0)
        {
            if (sizeof lib_w<lib_w[x-1])
                search_start=sizeof lib_w;
            else
                search_start=lib_w[x-1];
        }

        search_end=lib_w[x];
        if (sizeof lib_w<search_end)
            search_end=sizeof lib_w;

    }

    /* Second, count the User dictionary area.  */

    kzk_search_start=6;
    kzk_search_end=6;
    if (!(word_lib_state&2))                /*Note exp: !word...&2 */
        return(1);                      /* and !(word&2) */

    x=word_long-2;
    if (x>=3)
    {
        x-=3;
        kzk_search_start+=4;
    }

    if (x>0)
    {
        if (sizeof kzk_lib_w<kzk_lib_w[x-1])
            kzk_search_start=sizeof kzk_lib_w;
        else
            kzk_search_start=kzk_lib_w[x-1];
    }

    kzk_search_end=kzk_lib_w[x];
    if (sizeof kzk_lib_w<kzk_search_end)
        kzk_search_end=sizeof kzk_lib_w;

}


/******************************************************************************/
/* FUNCTION    : search_and_read                                              */
/* DESCRIPTION : Get Index Character and Read from Dictionary                 */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : f_ci1: the first letter, f_ci2: the second letter            */
/* OUTPUT      : NC success; C not success                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
search_and_read(f_ci1,f_ci2)
BYTE f_ci1,f_ci2;
{
    if (if_already_in(f_ci1,f_ci2)) /* Adjust If The Page Has Already In The */
                                    /* Memory                                */
        return(0);

   /*  Count The Sub_library Address, page Address, And read/write Length   */
    count_basic_pera(f_ci1,f_ci2);

    if (item_length!=0)
        if (read_a_page(2,r_addr,item_length,(LPSTR)lib_w))
            word_lib_state=word_lib_state|1;
    read_kzk_lib();
}

/******************************************************************************/
/* FUNCTION    : if_already_in                                                */
/* DESCRIPTION : Adjust If The Page Has Already In The Memory                 */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : f_ci1: the first letter, f_ci2: the second letter            */
/* OUTPUT      : CLC success; STC not success                                 */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
if_already_in(f_ci1,f_ci2)
BYTE f_ci1,f_ci2;
{
    WORD x;

    TO_LOW_BYTE(x, f_ci2);
    x=x*0x100+f_ci1;

    if (x==last_item_name)
        return(CLC);

    if (f_ci1==(BYTE)last_item_name)
        if (f_ci2>9)
            return(STC);
        else
            if (HIBYTE(last_item_name)>9)
                return(STC);
            else
                return(CLC);

    return(STC);

}

/******************************************************************************/
/* FUNCTION    : count_basic_pera                                             */
/* DESCRIPTION : Count The Sub_library Address, page Address, And read/write  *//*               Length                                                       */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : f_ci1: the first letter, f_ci2: the second letter            */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
count_basic_pera(f_ci1,f_ci2)
BYTE f_ci1,f_ci2;
{
    BYTE x;

    /* send parameters     */
    word_lib_state=0;
    item_addr=0xffff;
    item_length=0;
    TO_LOW_BYTE(last_item_name,f_ci2);
    last_item_name=last_item_name*0x100+f_ci1;

    if (f_ci1>'I'&& f_ci1<'U')
        slib_addr=(f_ci1-0x41-1)*27;
    else
        if (f_ci1>'V')
            slib_addr=(f_ci1-0x41-3)*27;
        else
            slib_addr=(f_ci1-0x41)*27;

    r_addr=ndx.dir[slib_addr+1];
    r_addr+=ndx.body_start;
    r_addr=r_addr*16;

    if (f_ci2<'A')
        item_addr=slib_addr+MORE_THAN_5;
    else
        if (f_ci2>'I' && f_ci2<'U')
            item_addr=slib_addr+(f_ci2-0x41-1);
        else
            if (f_ci2>'V')
                item_addr=slib_addr+(f_ci2-0x41-3);
            else
                item_addr=slib_addr+(f_ci2-0x41);

    r_addr=r_addr+ndx.dir[item_addr+1+1];
    item_length=ndx.dir[item_addr+1+1+1];

    item_length-=ndx.dir[item_addr+1+1];
    return(0);

}

/******************************************************************************/
/* FUNCTION    : read_kzk_lib                                                 */
/* DESCRIPTION : Search The Expended lib                                      */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : CLC success; STC not success                                 */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
read_kzk_lib()
{

    r_addr=kzk_ndx.dir[slib_addr+1];      /* Send Parameters                 */
    r_addr+=kzk_ndx.body_start;
    r_addr=r_addr*16+KZK_BASE;
    r_addr=r_addr+kzk_ndx.dir[item_addr+1+1];
    kzk_item_length=kzk_ndx.dir[item_addr+1+1+1];
    item_length-=kzk_ndx.dir[item_addr+1+1];
    if (kzk_item_length<0)
        kzk_item_length=0;
    if (!kzk_item_length)
        return(STC);
    if (!read_a_page(1,r_addr,kzk_item_length,(LPSTR)kzk_lib_w))
        return(STC);

    word_lib_state|=2;
    return(CLC);

}


/******************************************************************************/
/* FUNCTION    : abbr_s1                                                      */
/* DESCRIPTION : Find Match Words Arrcoding to The Given Input Message        */
/*               Search order is:                                             */
/*                    Temp_rem area                                           */
/*                    Standard Dictionary                                     */
/*                    User Dictionary                                         */
/*               If there are more than one words, judge what is the suitable *//*               one                                                          */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
abbr_s1()
{
    group_no=0;
    msx_area_cnt=0;
    out_svw_cnt=0;

    /* Search temp rem_area */

    if(AbcUsrDictFileChange(USRREM, sizeof(tmmr.stack1)+sizeof(tmmr.stack2)+
      sizeof(tmmr.stack3), sizeof(tmmr.temp_rem_area), tmmr.temp_rem_area)==-1)
         return;

    fczs1(tmmr.temp_rem_area,sizeof tmmr.temp_rem_area,2);
    fczs1(tmmr.rem_area,sizeof tmmr.rem_area,0x82);

    /* Search User dictionary */
    abbr_entry((BYTE *)kzk_lib_w+kzk_search_start,(BYTE *)kzk_lib_w+kzk_search_end,4);
    /* Search stndard dictionary */
    abbr_entry((BYTE *)lib_w+search_start, (BYTE *)lib_w+search_end,0);
    if (!group_no)                      /* Without any results    */
        return(STC);
    if (group_no==1)                    /* Only one! */ 
        return(CLC);

    order_result2();                    /* Results more than one   */
    if (auto_mode)                      /* If in frenquency ajust mode    */
        find_multy_hi();                /* Search High Frequeny Words */
    return(CLC);                        /* Return OK. */
}


/******************************************************************************/
/* FUNCTION    : fczs1                                                        */
/* DESCRIPTION : Search New Words From Temperoy Remembered Area               */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : rem_p, end, area_flag                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
fczs1(rem_p,end,area_flag)
BYTE *rem_p;    
int end,area_flag;
{
    unsigned int i=0,w_long,j;
    WORD *p;

    w_long=word_long*2;

    while(i<end)
    {
        if (w_long==rem_p[i])
        {
                                 /* Find Word When Word Length is 2 or More */
            if (find_long_word2(&rem_p[i]))
            {
                group_no+=1;
                if (!trs_new_word(i,&rem_p[i],area_flag)) /* Transmit New Words */
                    return(0);
            }
        }

        if (rem_p[i]==0)
            return(0);

        if (rem_p[i]>18||(rem_p[i]&1))
        {
            p=(WORD *)&rem_p[i];
            for (j=0; j<end/2; j++)
            {
                if (p[j]!=0)
                    p[j]=0;
                else
                    return(0);
            }
        }
        i+=rem_p[i]+2;
    }/* while  */
}

/******************************************************************************/
/* FUNCTION    : find_long_word2                                              */
/* DESCRIPTION : Find Word When Word Length is 2 or More                      */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : buffer                                                       */
/* OUTPUT      : CLC success; STC not success                                 */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
find_long_word2(buffer)
BYTE *buffer;
{
    int yj_p,i;

    yj_p=wp.xsyjw;
    for (i=0; i<word_long*2; i=i+2)
    {
        cmp_cisu=buffer[i+2]*0x100+buffer[i+2+1];
        pre_cmp(yj_p);              /* Do Prepare Jobs Before Search       */
        if (!cmp_a_slbl_with_bx())  /* Compare a Syllable With Strokens    */
            return(STC);
        yj_p++;
    }

    return(CLC);

}


/******************************************************************************/
/* FUNCTION    : trs_new_word                                                 */
/* DESCRIPTION : Transmit New Words                                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : word_addr, buffer, area_flag                                 */
/* OUTPUT      : CLC success; STC not success                                 */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
trs_new_word(word_addr,buffer,area_flag)
int word_addr,area_flag;
BYTE *buffer;
{
    int i;

    if (out_svw_cnt>=sizeof out_svw)
    {
        group_no--;
        return(STC);
    }

    for (i=0;i<word_long*2; i++)                /* Send New Words to out_svw */
        out_svw[out_svw_cnt++]=buffer[i+2];

    msx_area[msx_area_cnt].pindu=0x70+(BYTE)group_no;
    msx_area[msx_area_cnt].from=area_flag;      /* come from temp_area       */
    msx_area[msx_area_cnt].addr=word_addr;
    if (buffer[1]&0x80)
        msx_area[msx_area_cnt].pindu=0x31;

    msx_area_cnt++;

    return(CLC);
}

/******************************************************************************/
/* FUNCTION    : pre_cmp                                                      */
/* DESCRIPTION : Do Prepare Jobs Before Search                                */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : x                                                            */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
pre_cmp(x)
WORD x;
{
    cmp_yj=wp.yj[x];
    cmp_head=wp.syj[x];
    cmp_state=wp.cmp_stack[x];
    cmp_bx=wp.bx_stack[x];
}

/******************************************************************************/
/* FUNCTION    : cmp_a_slbl_with_bx                                           */
/* DESCRIPTION : Compare a Syllable With Strokens                             */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : CLC success; STC not success                                 */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
cmp_a_slbl_with_bx()
{
    if (cmp_cisu<0x2020)
        return(STC);

    if (cmp_cisu>=0x8000)
    {
        if ((cmp_cisu-0x8000)>=cisu->t_bf_start[2]/2)
            return(STC);
    }
    else
    {
        if ((cmp_cisu-0x2020)>=cisu->t_bf_start[1]/2)
            return(STC);
    }

    if (!cmp_a_slbl())        /* Compare a Syllable                  */
        return(STC);

    if (!cmp_bx)
        return(CLC);

    if (!yjbx())              /* Compare a Syllable with Radicals    */
        return(STC);

    return(CLC);

}

/******************************************************************************/
/* FUNCTION    : cmp_a_slbl                                                   */
/* DESCRIPTION : Compare a Syllable                                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : CLC success; STC not success                                 */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
cmp_a_slbl()
{

    if (cmp_state&2) 
    {
        if (cmp_cisu>=0x8000)
            if (cmp_yj==cisu->t_bf2[cmp_cisu-0x8000])
                return(CLC);
            else
                return(STC);
        else
            if (cmp_yj==cisu->t_bf1[cmp_cisu-0x2020])
                return(CLC);
            else
                return(STC);
    }
    return(cmp_first_letter());
}

/******************************************************************************/
/* FUNCTION    : cmp_first_letter                                             */
/* DESCRIPTION : Compare First Letter                                         */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
cmp_first_letter()
{
    WORD py_nm;
    if (!(cmp_state&4))
        return(STC);

    py_nm=cisu_to_py();        /* Changed Word Cell Code ( cisu ) to Pinyin */
    if ((HIBYTE(py_nm)&0x5f)==cmp_head)
        return(CLC);
    else{
        TO_LOW_BYTE(py_nm,get_head(HIBYTE(py_nm)));
        if ((LOBYTE(py_nm)&0xdf)==cmp_head)
            return(CLC);
        else
            return(STC);
    }
}

/******************************************************************************/
/* FUNCTION    : cisu_to_py                                                   */
/* DESCRIPTION : Changed Word Cell Code ( cisu ) to Pinyin                    */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
cisu_to_py()
{
    if (cmp_cisu>=0x8000)
        return(cisu->t_bf2[(cmp_cisu-0x8000)]);
    else
        return(cisu->t_bf1[(cmp_cisu-0x2020)]);

}

/******************************************************************************/
/* FUNCTION    : get_head                                                     */
/* DESCRIPTION : Get Chinese syllable internal code                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
get_head(first_letter)
BYTE first_letter;
{
    if (first_letter>=0x41)              /* First Letter is Alpha */
        return(first_letter);

    first_letter=(first_letter-1)*5;
    return(slbl_tab[first_letter]);

}

/******************************************************************************/
/* FUNCTION    : yjbx                                                         */
/* DESCRIPTION : Compare a Syllable with Radicals                             */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
yjbx()
{
    BYTE bx;

    if (cmp_cisu<0x8000)
        cmp_cisu-=0x2020;
    else
    {
        cmp_cisu=cisu->t_bf2[(cmp_cisu-0x8000)+1];
        cmp_cisu=(HIBYTE(cmp_cisu)-0xa1)+(LOBYTE(cmp_cisu)-0xb0)*94;
    }
    bx=spbx_tab[cmp_cisu];
    if (cmp_bx==bx)
        return(CLC);
    else
        if (cmp_bx==(bx&0xf0))
            return(CLC);
        else
            return(STC);

}


/******************************************************************************/
/* FUNCTION    : abbr_entry                                                   */
/* DESCRIPTION : Entry Pointer of Convert Search                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : s_start: Search Start Position                               */
/*               s_end  : Search End Position                                 */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
abbr_entry(s_start,s_end)
BYTE *s_start,*s_end;                     /* Search start and Ending position */
{           
    int i;

    while (s_start<s_end)
    {
        if (cmp_long_word2(s_start))          /* Compare word by word.*/
        {                          
            if (out_svw_cnt>=sizeof out_svw)    /* If buffer out_svw is full, */                                                /* error                      */
                return(STC);
            for (i=0; i<word_long*2; i++)
                out_svw[out_svw_cnt++]=s_start[i];  /*Move the words*/

            msx_area[msx_area_cnt].pindu=s_start[i];
            msx_area[msx_area_cnt].from=word_source; /* come from ...*/
            msx_area[msx_area_cnt].addr=(WORD)s_start; /* Where is the ...*/

            msx_area_cnt++;                  /* Increae the pointer  */
            /* for the attribue area */

            group_no+=1;               /* In case of OK, increaase  results counter.*/
        }

        s_start+=(word_long+word_long+1);          /* Push down the search pointer */
        /* by word_long*2+1 */
    }

    return(CLC);                       /* The value of return is no use for */
                                       /* the routine */
}

/******************************************************************************/
/* FUNCTION    : cmp_long_word2                                               */
/* DESCRIPTION : Compare Operate for Word Length More Than 1                  */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
cmp_long_word2(buffer)
BYTE *buffer;
{
    int yj_p,i;

    yj_p=wp.xsyjw;
    for (i=0; i<word_long*2; i=i+2){
        cmp_cisu=buffer[i]*0x100+buffer[i+1];
        if (cmp_cisu<0x2020)
            return(STC);
        pre_cmp(yj_p);               /* Do Prepare Jobs Before Search       */
        if (!cmp_a_slbl_with_bx())   /* Compare a Syllable With Strokens    */
            return(STC);
        yj_p++;
    }

    return(CLC);

}

/******************************************************************************/
/* FUNCTION    : order_result2                                                */
/* DESCRIPTION : Sort Results                                                 */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
order_result2()
{
    int lng,i,j,n;
    BYTE x,flag;
    WORD y;

    lng=word_long*2;
    if (xs_flag==1)
        if (!fenli_daxie())       /* Sperate Capital Letters             */
            return(0);

    if (msx_area_cnt==1)
        return(0);


    for (i=group_no-1; i>0; i--)
    {
        flag=0;
        for (j=0; j<i; j++)
        {
            if (msx_area[j].pindu<msx_area[j+1].pindu)
            {
                for (n=j*lng; n<j*lng+lng; n++)
                {
                    x=out_svw[n];
                    out_svw[n]=out_svw[n+lng];
                    out_svw[n+lng]=x;
                }
                x=msx_area[j].pindu;
                msx_area[j].pindu=msx_area[j+1].pindu;
                msx_area[j+1].pindu=x;
                x=msx_area[j].from;
                msx_area[j].from=msx_area[j+1].from;
                msx_area[j+1].from=x;
                y=msx_area[j].addr;
                msx_area[j].addr=msx_area[j+1].addr;
                msx_area[j+1].addr=y;
                flag=1;
            }
        }
        if (!flag)
            break;
    }
}

/******************************************************************************/
/* FUNCTION    : fenli_daxie                                                  */
/* DESCRIPTION : Sperate Capital Letters                                      */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      : CLC : There is not  Caps. STC : There is Caps.               */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
fenli_daxie()
{
    int i,j,n,lng;

    j=0;
    lng=word_long*2;
    for (i=0; i<msx_area_cnt; i++)
    {
        if (msx_area[i].pindu==0x31)         /* Send Parameters             */
        {
            msx_area[j].pindu=msx_area[i].pindu;
            msx_area[j].from=msx_area[i].from;
            msx_area[j].addr=msx_area[i].addr;
            for (n=0; n<lng; n++)
                out_svw[j*lng+n]=out_svw[i*lng+n];
            j++;
        }
    }

    if (!j)
        return(CLC);            /* there is no Caps; */
    group_no=j;
    return(STC);                /* there has Caps;  */

}

/******************************************************************************/
/* FUNCTION    : rzw                                                          */
/* DESCRIPTION : Match Word Suffix or Prefix                                  */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
rzw()
{
    if (!(system_info&1))           /* if strength mode, ret */
        sfx_proc();
    return(0);

}


/*                                                                            */

/******************************************************************************/
/* FUNCTION    : abbr                                                         */
/* DESCRIPTION : Entry Point of Convert While First Letter is Caps.           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
abbr()
{
    int i;
    WORD x;
    WORD *kbf_p;

    if (in.true_length<2)
    {
        xs_flag=1;
        jiyi_pindu|=0x80;
        return(0);
    }

    for (i=0; i<in.true_length; i++)
    {
        if (in.buffer[i]&0x20)
        {                                 /* if not Caps */
            xs_flag=1;
            jiyi_pindu|=0x80;
            return(0);
        }
    }

    kbf_p=(WORD *)kbf.buffer;
    x=0x2d*0x100;

    kbf.true_length=in.true_length*2;
    for (i=0; i<in.true_length; i++)
    {
        TO_LOW_BYTE(x,in.buffer[i]);
        kbf_p[i]=((x&0xFF00)>>8)|((x&0xFF)<<8);
    }
    kbf.buffer[in.true_length*2]=0;
    in.info_flag='*';
    return(0);

}


/******************************************************************************/
/* FUNCTION    : sfx_proc                                                     */
/* DESCRIPTION : Entry Point of Deals With Prefix Suffix                      */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
sfx_proc()
{
    int i,j;
    WORD x,save_xsyjw;
    WORD *result_p;
    BYTE *sfx_p;

    result_p=(WORD *)result_area;
    sfx_p=(BYTE *)sfx_table;

    if (cp_ajust_flag==1)                /* Word Frequency Has Been Adjusted */
        return(0);
    if (word_long>=3)                    /* Word Length >= 3 Chinses Chars   */
        return(0);

    save_xsyjw=wp.xsyjw;
    wp.xsyjw=wp.xsyjs;

    i=0;
    do {                                /* Search sfx_table                */
        x=sfx_table[i];
        if (HIBYTE(x)&sfx_attr)
            if (LOBYTE(x)==word_long*2)
                /* Compare Operate for Word Length More Than 1             */
                if (cmp_long_word2(&sfx_p[i*2+2]))
                {
                    for (j=0; j<word_long; j++)
                        result_p[result_area_pointer/2+j]=sfx_table[i+1+j];
                    result_area_pointer+=word_long*2;
                    wp.xsyjw=save_xsyjw;
                    cp_ajust_flag=1;
                    return(0);
                }

        i+=LOBYTE(x)/2+1;

    } while(i<sfx_table_size);

    wp.xsyjw=save_xsyjw;

}


/******************************************************************************/
/* FUNCTION    : look_for_code                                                */
/* DESCRIPTION : Search If The Code Is In The Index                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
look_for_code()
{
    int i,rec_cnt;

    rec_cnt=0;
    for (i=0x10; i<(mulu_true_length+0x10); i=i+mulu_record_length)
    {
        if (if_code_equ(i))
            return(rec_cnt+1);   /* find the code, rec_cnt+1 in order to avoid*/
        else /*confusing with STC */
            rec_cnt++;
    }
    return(STC);                                    /* not found */

}

/******************************************************************************/
/* FUNCTION    : if_code_equ                                                  */
/* DESCRIPTION : Search If The Code In The Index                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : addr                                                         */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
if_code_equ(addr)
int addr;
{
    int i;
    BYTE *p;

    p=(BYTE *)lib_w;

    if (kbf.true_length!=(p[addr++]-0x30))  /*minuse the 0x30 in order to get the record length */
        return(STC);            /* if the length is not equal, exit */
    for (i=0; i<kbf.true_length; i++)
    {
        if ((kbf.buffer[i]!=p[addr])
            && ((kbf.buffer[i]&0xdf)!=p[addr]))
            return(STC);
        addr++;
    }
    return(CLC);                    /*find the code in the index*/
}

