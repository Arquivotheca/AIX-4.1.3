static char sccsid[] = "@(#)01	1.3  src/bos/usr/lib/nls/loc/CN.im/cnedabcupdt.c, ils-zh_CN, bos41J, 9510A_all 3/6/95 23:29:14";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: AbcUpdateDict
 *		AddSubLib
 *		AddWord
 *		FillNew
 *		GetHead
 *		GetPosLen
 *		GetSubSize
 *		IfCisuCode
 *		PushPageDown
 *		PushSlibDown
 *		SuccBack
 *		err_proc
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
/* MODULE NAME:        cnedabcudpt                                            */
/*                                                                            */
/* DESCRIPTIVE NAME:   Chinese Input Method For Update $(HOME).abcusrrem      */
/*                                                                            */
/* FUNCTION:           AbcUpdateDict    : Update Abc Dictionary               */
/*                                                                            */
/*                     GetSubSize       : Obtain length of a sub_lib of user  */
/*                                        dictionay file.                     */
/*                                                                            */
/*                     AddSubLib        : Append New Term in This Sublibrary  */
/*                                                                            */
/*                     IfCisuCode       : Check the cisu code                 */
/*                                                                            */
/*                     AddWord          : Append the new word                 */
/*                                                                            */
/*                     GetPosLen        : Get subpage start position and      */
/*                                        length in sublibary.                */
/*                                                                            */
/*                     PushSlibDown     : Move Sublibrary Content             */
/*                                                                            */
/*                     PushPageDown     : Get space in the subpage for        */
/*                                        appending new term.                 */
/*                                                                            */
/*                     FillNew          : Append a new term                   */
/*                                                                            */
/*                     GetHead          : Get Chinese syllable internal code  */
/*                                        table                               */
/*                                                                            */
/*                     SuccBack         : Processing Success and return       */
/*                                                                            */
/*                     err_proc         : Processing error                    */
/*                                                                            */
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

#include "stdio.h"
#include "errno.h"
#include "fcntl.h"
#include "cjk.h"
#include "data.h"

#include "chinese.h"
#include "slbl.h"

#define ZQJY_SIZE     0x400

#define rem_area_END  ZQJY_SIZE/2


/*  Memory and File Pointer */

FILE *fp_rem,*fp_usr,*fp_tmp,*fp_sys;

struct TBF *cisu;

/* Pointer and Variable    */
int hd,i,count;
HANDLE hMem;
LPSTR rem_area,p,SubBuffer,cisu_1;
WORD *pp;

WORD     SlibAddr ;
WORD     ItemAddr;
WORD     PageAddr;
WORD     KzkItemLength;
WORD     WordStart;
int mode=7;
/*  User Dictionary Index Area */
struct DEX head;

/* Error Messages */
char TMMR_OPEN_WRONG[]=  "记忆文件打开错";
char TMMR_READ_WRONG[]=  "记忆文件读错";
char TMMR_WRITE_WRONG[]= "记忆文件写错";
char USER_OPEN_WRONG[]=  "用户词库打开错";
char USER_READ_WRONG[]=  "用户词库读错";
char USER_WRITE_WRONG[]= "用户词库写错";
char read_ndx_wrong[]=   "用户词库目录操作错";
char m_short[]=          "内存不够";
char jiyi_wenjian_cuo[]= "用户记忆文件操作错!";
char LIB_FULL[]=         "Dictionary is full.";

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

/******************************************************************************/
/* FUNCTION    : AbcUpdateDict                                                */
/* DESCRIPTION : Update Abc Dictionary                                        */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : usrrem, usrdict, op_mode                                     */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

AbcUpdateDict(usrrem, usrdict, op_mode)
char *usrrem;
char *usrdict;
int op_mode;
{

    unsigned short  SubLib,subsize,ChangeHappen=0;
    unsigned int    pos;
    unsigned short  count,old_size;
    char            *temp, *temp1;

    mode = op_mode;

    cisu=(struct TBF *)calloc(PTZ_LIB_LENGTH,sizeof (char));
    if (cisu==NULL)
        return err_proc("Out of Memory");

    pos = PTZ_LIB_LENGTH;

    rem_area=(LPSTR)calloc( 0x1000,sizeof (char) );
    /*Get space for update rem file */

    if ( (fp_rem = fopen(usrrem,"r+"))==NULL)  /*Open temp rem file*/
        return err_proc(TMMR_OPEN_WRONG);

    pos= fseek( fp_rem, MIDDLE_REM, 0);         /* Find right place */
    count= fread((LPSTR)rem_area, ZQJY_SIZE, 1, fp_rem);
    if(count!=1)                        /* read */
        return err_proc( TMMR_READ_WRONG);     /* Display Read error Message */

    if (!rem_area[0])
        return SuccBack(1);       /* Middle Remembered Area Empty, No Adjust */

    if (( fp_usr = fopen( usrdict, "r" ))==0)
        return  err_proc(TMMR_OPEN_WRONG);

    pos= fseek( fp_usr, LENGTH_OF_USER,0);

    count = fread( &head, NDX_REAL_LENGTH, 1, fp_usr);  /* Read Head of Index */
    if (count!=1)
        return err_proc(jiyi_wenjian_cuo);

    if ( head.body_length>= 0x2020)      /* Get Dictionary Length(In Segment) */
        return  err_proc( LIB_FULL );    /* Display Dictionary Full Message   */

    temp = (unsigned char*)malloc(strlen(usrdict) + strlen(".tmp") + 1);
    (void)sprintf(temp, "%s.tmp", usrdict);
    if ((fp_tmp = fopen(temp, "w"))==NULL)
        return err_proc(jiyi_wenjian_cuo);

    pos = fseek(fp_tmp, 0xa000l, 0);
    count=fwrite((LPSTR)&head, NDX_REAL_LENGTH,1,fp_tmp);
    if (count!=1)
        return err_proc(jiyi_wenjian_cuo);

    if ( (fp_sys = fopen(DEFAULTABCSYSDICT2, "r")) == NULL ) 
        return  err_proc("Open UABC.OVL Wrong");
    fseek(fp_sys, PTZ_LIB_START_POINTER, 0);            /* Push Down Pointer */
    count=fread((LPSTR)&cisu->t_bf_start,PTZ_LIB_LENGTH,1,fp_sys);  /* Read Word Cell Code Table */
    if (count != 1)
        return  err_proc("Read UABC.OVL Wrong!");

    for (SubLib='A';SubLib<='Z';SubLib++)
    {

        if (SubLib!='I'||SubLib!='U'||SubLib!='V')
        {

            subsize=GetSubSize(SubLib);
            SubBuffer=(LPSTR)calloc( subsize+0x1000, sizeof(char) );

            if (subsize)
            {
                count=fread( SubBuffer, subsize,1,fp_usr);
                if (count!=1)
                    return err_proc( USER_READ_WRONG );
            }    /* Display Dictionary Full Message   */

            ChangeHappen += AddSubLib(SubLib);

            if ((subsize = GetSubSize(SubLib))!=0)
            {
                count=fwrite( SubBuffer, subsize,1,fp_tmp);
                fflush(fp_tmp);
                if (count!=1)
                    return err_proc( USER_WRITE_WRONG );
            }    /* Display Dictionary Full Message   */
            free(SubBuffer);
        } 
    }  

    if (ChangeHappen)
    {
        pos=  fseek( fp_tmp, 0xa000l, 0);
        /* Write File Head */
        count=  fwrite(  (LPSTR)&head, NDX_REAL_LENGTH,1,fp_tmp);
        pos=  fseek( fp_tmp, 0l, 0);
        pos=  fseek( fp_usr, 0l, 0);
        for (i=0; i<LENGTH_OF_USER/0x1000; i++)
        {
            count =   fread( rem_area, 0x1000,1,fp_usr);
            count =   fwrite( rem_area, 0x1000,1,fp_tmp);
            if (count!=1)
                return err_proc(USER_WRITE_WRONG);
        }

        fflush( fp_tmp );
        fclose( fp_usr );
        fclose( fp_tmp );

        temp1 = (unsigned char*)malloc(strlen(usrdict) + strlen(".old") + 1);
        (void)sprintf(temp1, "%s.old", usrdict);
        remove(temp1);
        if(rename(usrdict,temp1))
            printf("ErrNO = %d\n",errno);
        rename(temp, usrdict);

        for (i=0; i<ZQJY_SIZE; i++)         /* Flush Middle Remembered Area */
            rem_area[i]=0;
        fseek( fp_rem, MIDDLE_REM, 0);

        count=fwrite(rem_area, 1, ZQJY_SIZE, fp_rem);    
        if (count!=ZQJY_SIZE)
            return err_proc( TMMR_READ_WRONG);        


        fclose( fp_rem);
        return SuccBack(2);
    }
    else 
    {
        remove(temp);
        return SuccBack(1);
    }
}
/* User Dictionary Update End */

/******************************************************************************/
/* FUNCTION    : GetSubSize                                                   */
/* DESCRIPTION : Obtain length of a sub_lib of user dictionay file.           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : Sublib name                                                  */
/* OUTPUT      : The outcome if nonezero.                                     */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
GetSubSize(name)
int name;
{

    if (name>'V') name-=2;
    if (name>'I') name--;
    name=name-0x41;
    return (head.dex[name].item[24]+15)/16*16;
}



/******************************************************************************/
/* FUNCTION    : AddSubLib                                                    */
/* DESCRIPTION : Append New Term in This Sublibrary                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : name                                                         */
/* OUTPUT      : flag                                                         */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

AddSubLib(name)
WORD name;
{
    unsigned short *p;
    int flag=0;
    WORD x,i=0,y;

    p=(unsigned short *)&rem_area[0];

    while(i<rem_area_END){
        x = p[i]>>8;
        if ((x<4)||(x>18)||(x&1))
            break;
        if ((i+x+2)>=rem_area_END*2)
            break;
        if (!IfCisuCode(p[i+1]))
            break;
        if (!IfCisuCode(p[i+2]))
            break;
        flag+=AddWord(name, &p[i+1], x/2);
        i+=x/2+1;
    }

    return(flag);
}

/******************************************************************************/
/* FUNCTION    : IfCisuCode                                                   */
/* DESCRIPTION : Check the cisu code                                          */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : cisu THE CODE TO BE CHECHED                                  */
/* OUTPUT      : TRUE: it is cisu code;                                       */
/*               FAULS: it is not                                             */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

IfCisuCode(cisu)
WORD cisu;
{

    if((cisu>=0x2020)&&(cisu<0xa000))
        return(1);
    else
        return(0);
}

/******************************************************************************/
/* FUNCTION    : AddWord                                                      */
/* DESCRIPTION : Append a new word                                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : name, pt, WordLong                                           */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

AddWord(name, pt, WordLong)
int name;                /* Dictionary Name */
unsigned short  *pt;        
WORD WordLong;        
{
   WORD name2,x;

    if(name != GetHead(*pt) )
           return(0);

    name2 = GetHead(pt[1]);        /* Get Word */

       if (!GetPosLen(name,name2,WordLong))
        return(0);        /* Get Length Position */


       if (PushPageDown(WordLong, PushSlibDown(WordLong))){/* Push Down Page */
        FillNew(WordLong,pt);            /* Add New Word   */
        return(1);
    }

}


/******************************************************************************/
/* FUNCTION    : GetPosLen                                                    */
/* DESCRIPTION : Get subpage start position and length in sublibary.          */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : name, name2, WordLong                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
GetPosLen(name,name2,WordLong)
int name,name2;
int WordLong;
{
   WORD x,y;

    SlibAddr = 0;            /* Initialize */
    KzkItemLength = 0;

    x=name-'A';
    if (name>='V')
        x-=2;
    if (name>='I')
        x--;

    SlibAddr = x;

    if (WordLong<5)
    {
         y=name2-'A';
        if (name2>='I')
            y--;
        if (name2>='V')
        y-=2;
    }
    else
        y=23;

    ItemAddr = y;

    PageAddr = head.dex[x].item[y];
                        /* Keep Page Address */
    KzkItemLength = head.dex[x].item[y+1] - PageAddr;
    if (KzkItemLength>=0x1800)
            return(0);
    else return(1);


}

/******************************************************************************/
/* FUNCTION    : PushSlibDown                                                 */
/* DESCRIPTION :                                                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : WordLong                                                     */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int PushSlibDown( WordLong)
int  WordLong;
{
    WORD x,y,z,bp;

    x = WordLong*2+1;
    
    bp = 0 ;            

    if (!KzkItemLength)
    {
        if (WordLong<5)
            x = x+6;
        else
            x = x+10;
    }

    y = (head.dex[SlibAddr].item[24]+15)/16;
    z = (head.dex[SlibAddr].item[24]+15+x)/16;

    if ( y < z )
    {
         bp = z - y;

         head.ttl_length += bp;        
         head.body_length += bp;    

/* Test all file length here (in future)   */

          for ( i=SlibAddr+1; i<23; i++) head.dex[i].start_pos+=bp;
    }
    return(x);
}

    
/******************************************************************************/
/* FUNCTION    : PushPageDown                                                 */
/* DESCRIPTION : Get space in the subpage for appending new term.             */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : WordLong, TrueLength                                         */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
PushPageDown( WordLong,TrueLength)
int WordLong,TrueLength;
{
    int si,di,i,x,dx;
    unsigned short  *pp;

    si=PageAddr;            /* Get Page Address */

    di = head.dex[SlibAddr].item[24];     /* Point End Of Sublibrary */

    dx = 0;                 /* Initialize    */
    pp=(unsigned short *)&SubBuffer[si];

    if ( KzkItemLength ){          /* Sublibrary Is Empty */
       if ( WordLong==2 )
          dx = 6 ;
       if ( WordLong == 5 )
          dx = 10 ;
       if ( WordLong>5 )
          dx = pp[WordLong-6];
       if ( WordLong==3||WordLong==4)
          dx = pp[WordLong-3];
    }


    dx = dx + si;
    WordStart=dx;
    x = di - dx;

    if (x>=0)
    {
        if (x)
           for(i = 0; i<x; i++)
              SubBuffer[di+TrueLength-i-1] = SubBuffer[di - 1-i];

        si = ItemAddr+1 ;
        while( si<25 )
            head.dex[SlibAddr].item[si]+=TrueLength, si++;
        return(1);
    }
    return err_proc("Words Wrong");
}


/******************************************************************************/
/* FUNCTION    : FillNew                                                      */
/* DESCRIPTION : Append a new term                                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : WordLong, pt                                                 */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

FillNew(WordLong,pt)
WORD WordLong;
LPSTR pt;
{
WORD cx,ax,bx,i;
unsigned short *pp;

pp=(unsigned short *)&SubBuffer[PageAddr];

if(!KzkItemLength){        

    if (WordLong<5)
        cx=3, ax=6;
    if (WordLong>=5)
        cx=5, ax=10;
    for(i=0; i<cx; i++)
        pp[i] = ax;

    WordStart += ax;
       }

    for (i=0; i<WordLong*2; i++)
        SubBuffer[WordStart+i]=pt[i];
    if ( *(pt-1)&0x80)
        SubBuffer[WordStart+i] = '1';
    else
        SubBuffer[WordStart+i] = 'm';


/*    New word is alredy fill in, now ajust the group_index  */


    if (WordLong<5)
        cx=3, bx=WordLong-2;
    else
        cx=5, bx=WordLong-5;

    while(bx<cx)
        pp[bx]+= WordLong*2+1,bx++;  

/*  Change the group_index para is over. Now, add word number to the index. */

        head.ttl_words++;        

        if ( WordLong>=5) 
                head.fiveup_words++;
        else {
         if (WordLong==2) head.two_words++;
         if (WordLong==3) head.three_words++;
         if (WordLong==4) head.four_words++;
          }
}



/******************************************************************************/
/* FUNCTION    : GetHead                                                      */
/* DESCRIPTION : Get Chinese syllable internal code table                     */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : ThisCisu                                                     */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

GetHead(ThisCisu)
WORD  ThisCisu;
{
    BYTE head;

    if (ThisCisu>=0x8000)
        head=HIBYTE(cisu->t_bf2[ThisCisu-0x8000]);
    else
        head=HIBYTE(cisu->t_bf1[ThisCisu-0x2020]);

    if (head>=0x41)
        return(head);

    return(slbl_tab[(head-1)*5]);
}

/******************************************************************************/
/* FUNCTION    : SuccBack                                                     */
/* DESCRIPTION : Processing Successed and retuen                              */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : state                                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

SuccBack(state)
int state;
{
   if (cisu!=0)
      free(cisu);

   if (rem_area!=0)
      free(rem_area);

   if (SubBuffer!=0)
      free(SubBuffer);


   if(fp_rem)     fclose(fp_rem);
   if(fp_usr)     fclose(fp_usr);
   if(fp_tmp)     fclose(fp_tmp);
   if(fp_sys)     fclose(fp_sys);

   if (mode==7)
       printf("Success\n");
}



/******************************************************************************/
/* FUNCTION    : err_proc                                                     */
/* DESCRIPTION : Processing  error and return error message.                  */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       : err_msg                                                      */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

err_proc(err_msg)
LPSTR err_msg;
{
   if (cisu!=0)
      free(cisu);

   if (rem_area!=0)
      free(rem_area);

   if (SubBuffer!=0)
      free(SubBuffer);


   if(fp_rem)   fclose(fp_rem);
   if(fp_usr)   fclose(fp_usr);
   if(fp_tmp)   fclose(fp_tmp);
   if(fp_sys)   fclose(fp_sys);

                /* Jan. 12 95 Modified By B.S.Tang        */
/*
   printf( "ERR: %s\n", err_msg);
*/
}

