static char sccsid[] = "@(#)50	1.2  src/bos/usr/lib/nls/loc/ZH.im/zhedabcupdt.c, ils-zh_CN, bos41J, 9509A_all 2/25/95 13:38:50";
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
/* MODULE NAME:        zhedabcudpt                                            */
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
/************************* END OF SPECIFICATION *******************************/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "cjk.h"
#include "data.h"
#include "extern.h"
#include "chinese.h"


#define ZQJY_SIZE     0x400

#define up_rem_area_END  ZQJY_SIZE/2


/*  Memory and File Pointer */

int fd_rem, fd_usr, fd_tmp;
FILE *fp_sys;

struct TBF cisu;

/* Pointer and Variable    */
int hd,i,count;
HANDLE hMem;
LPSTR up_rem_area,p,SubBuffer,cisu_1;
WORD *pp;

WORD     SlibAddr ;
WORD     ItemAddr;
WORD     PageAddr;
WORD     KzkItemLength;
WORD     WordStart;
int mode=7;
/*  User Dictionary Index Area */
struct DEX head_dex;


/******************************************************************************/
/* FUNCTION    : AbcUpdateDict                                                */
/* DESCRIPTION : Update Abc User Dictionary                                   */
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

    pos = PTZ_LIB_LENGTH;

                                              /*Get space for update rem file */
    up_rem_area=(LPSTR)calloc(0x1000, sizeof(char));

    if ((fd_rem = open(usrrem,O_RDWR))== -1)  /*Open temp rem file*/
        return(err_proc());

    if ((lockf(fd_rem, F_TLOCK, 0) == -1) && (errno == EACCES))
        return(err_proc());

    pos = lseek(fd_rem, MIDDLE_REM, 0);         /* Find right place */
    count = read(fd_rem, (LPSTR)up_rem_area, ZQJY_SIZE);
    if(count != ZQJY_SIZE)                        /* read */
        return(err_proc());            /* Display Read error Message */

    if (!up_rem_area[0])
        return(SuccBack(1));       /* Middle Remembered Area Empty, No Adjust */

    if ((fd_usr = open( usrdict, O_RDWR ))==-1)
        return(err_proc());

    if ((lockf(fd_usr, F_TLOCK, 0) == -1) && (errno == EACCES))
        return(err_proc());

    pos = lseek(fd_usr, LENGTH_OF_USER,0);

    count = read(fd_usr, &head_dex, NDX_REAL_LENGTH);  /* Read Head of Index */
    if (count != NDX_REAL_LENGTH)
        return(err_proc());

    if (head_dex.body_length >= 0x2020)  /* Get Dictionary Length(In Segment) */
        return(err_proc());              /* Display Dictionary Full Message   */

    temp = (unsigned char*)malloc(strlen(usrdict) + strlen(".tmp") + 1);
    (void)sprintf(temp, "%s.tmp", usrdict);
    if ((fd_tmp = open(temp, O_RDWR|O_CREAT|O_TRUNC, 0644))==-1)
        return(err_proc());
    if ((lockf(fd_tmp, F_TLOCK, 0) == -1) && (errno == EACCES))
        return(err_proc());

    pos = lseek(fd_tmp, 0xa000l, 0);
    count = write(fd_tmp, (LPSTR)&head_dex, NDX_REAL_LENGTH);
    if (count != NDX_REAL_LENGTH)
       return(err_proc());

    if ( (fp_sys = fopen(DEFAULTABCSYSDICT3, "r")) == NULL ) 
       return(err_proc());

    cisu.t_bf_start = (WORD *)malloc(16);     /* apply single tone area */
    memset(cisu.t_bf_start, 0, 16);           /* clear single tone area */

    fseek(fp_sys, sizeof(struct FILEHEAD), 0);
    if ( fread((char *)cisu.t_bf_start, 16, 1, fp_sys) == 0 && ferror(fp_sys) != 0 )
        return(err_proc());

    cisu.t_bf1 = (WORD *)malloc(cisu.t_bf_start[1]*2);   /* apply single tone area */
    cisu.t_bf2 = (WORD *)malloc(cisu.t_bf_start[2]*2);   /* apply single tone area */
    memset(cisu.t_bf1, 0, cisu.t_bf_start[1]*2);         /* clear single tone area */
    memset(cisu.t_bf2, 0, cisu.t_bf_start[2]*2);         /* clear single tone area */

    if ( fread((char *)cisu.t_bf1, cisu.t_bf_start[1]*2, 1, fp_sys) == 0
           && ferror(fp_sys) != 0 )
       return(err_proc());

    if (fread((char *)cisu.t_bf2, cisu.t_bf_start[2]*2, 1, fp_sys) == 0 
           && ferror(fp_sys) != 0 )
       return(err_proc());

    fclose(fp_sys);

    for (SubLib='A'; SubLib<='Z'; SubLib++)
    {
        if (SubLib!='I' || SubLib!='U' || SubLib!='V')
        {

            subsize = GetSubSize(SubLib);
            SubBuffer = (LPSTR)calloc(subsize+0x1000, sizeof(char));

            if (subsize)
            {
                count = read(fd_usr, SubBuffer, subsize);
                if (count != subsize)
                    return(err_proc());
            }    /* Display Dictionary Full Message   */

            ChangeHappen += AddSubLib(SubLib);

            if ((subsize = GetSubSize(SubLib))!=0)
            {
                count = write(fd_tmp, SubBuffer, subsize);
                if (count != subsize)
                    return(err_proc());
            }    /* Display Dictionary Full Message   */
            free(SubBuffer);
        } 
    }  

    if (ChangeHappen)
    {
        pos = lseek(fd_tmp, 0xa000l, 0);
        /* Write File Head */
        count = write(fd_tmp, (LPSTR)&head_dex, NDX_REAL_LENGTH);
        pos = lseek(fd_tmp, 0l, 0);
        pos = lseek(fd_usr, 0l, 0);
        for (i=0; i<LENGTH_OF_USER/0x1000; i++)
        {
            count = read(fd_usr, up_rem_area, 0x1000);
            count = write(fd_tmp, up_rem_area, 0x1000);
            if (count != 0x1000)
                return(err_proc());
        }


        temp1 = (unsigned char*)malloc(strlen(usrdict) + strlen(".old") + 1);
        (void)sprintf(temp1, "%s.old", usrdict);
        remove(temp1);
        rename(usrdict,temp1);
        rename(temp, usrdict);

        for (i=0; i<ZQJY_SIZE; i++)         /* Flush Middle Remembered Area */
            up_rem_area[i] = 0;
        lseek(fd_rem, MIDDLE_REM, 0);

        count = write(fd_rem, up_rem_area, ZQJY_SIZE);    
        if (count != ZQJY_SIZE)
            return(err_proc());        
        return(SuccBack(2));
    }
    else 
    {
        remove(temp);
        return(SuccBack(1));
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

    if (name > 'V')  name-=2;
    if (name > 'I')  name--;
    name = name - 0x41;
    return(head_dex.dex[name].item[24]+15)/16*16;
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

    p = (unsigned short *)&up_rem_area[0];
    while(i < up_rem_area_END)
    {
        x = p[i] >> 8;
        if ((x<4) || (x>18) || (x&1))
            break;
        if ((i+x+2) >= up_rem_area_END*2)
            break;
        if (!IfCisuCode(p[i+1]))
            break;
        if (!IfCisuCode(p[i+2]))
            break;
        flag += AddWord(name, &p[i+1], x/2);
        i += x/2+1;
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

    if((cisu >= 0x2020) && (cisu < 0xffff))
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
int name;				/* Dictionary Name */
unsigned short  *pt;		
WORD WordLong;		
{
    WORD name2, x;

    if(name != GetHead(*pt) )
        return(0);

    name2 = GetHead(pt[1]);		               /* Get Word */

    if (!GetPosLen(name, name2, WordLong))
	return(0);		                       /* Get Length Position */

    if (PushPageDown(WordLong, PushSlibDown(WordLong))) /* Push Down Page */
    {
        FillNew(WordLong, pt);		               	/* Add New Word   */
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
GetPosLen(name, name2, WordLong)
int name, name2;
int WordLong;
{
    WORD x, y;

    SlibAddr = 0;			/* Initialize */
    KzkItemLength = 0;
 
    x = name - 'A';
    if (name >= 'V')
        x -= 2;
    if (name >= 'I')
        x--;
 
    SlibAddr = x;
 
    if (WordLong < 5)
    {
       y = name2 - 'A';
       if (name2 >= 'I')
	  y--;
       if (name2 >= 'V')
 	  y -= 2;
    }
    else
	y = 23;

    ItemAddr = y;

    PageAddr = head_dex.dex[x].item[y];
						/* Keep Page Address */
    KzkItemLength = head_dex.dex[x].item[y+1] - PageAddr;
    if (KzkItemLength >= 0x1800)
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
int PushSlibDown(WordLong)
int  WordLong;
{
    WORD x, y, z, bp;

    x = WordLong * 2 + 1;
	
    bp = 0 ;			

    if (!KzkItemLength)
    {
	if (WordLong < 5)
	    x = x + 6;
	else
     	    x = x + 10;
    }

    y = (head_dex.dex[SlibAddr].item[24] + 15) / 16;
    z = (head_dex.dex[SlibAddr].item[24] + 15 + x) / 16;

    if ( y < z )
    {
        bp = z - y;
        head_dex.ttl_length += bp;		
        head_dex.body_length += bp;	

/* Test all file length here (in future)   */

        for (i=SlibAddr+1; i<23; i++) head_dex.dex[i].start_pos+=bp;
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

    si = PageAddr;			         /* Get Page Address */

    di = head_dex.dex[SlibAddr].item[24];	 /* Point End Of Sublibrary */

    dx = 0;				         /* Initialize    */
    pp = (unsigned short *)&SubBuffer[si];

    if ( KzkItemLength )                         /* Sublibrary Is Empty */
    {		  
        if ( WordLong == 2 )
            dx = 6 ;
	if ( WordLong == 5 )
	    dx = 10 ;
	if ( WordLong > 5 )
	    dx = pp[WordLong-6];
	if ( WordLong==3 || WordLong==4)
	    dx = pp[WordLong-3];
    }

    dx = dx + si;
    WordStart = dx;
    x = di - dx;

    if (x >= 0)
    {
	if(x)
           for(i = 0; i < x; i++)
		SubBuffer[di + TrueLength - i - 1] = SubBuffer[di - 1-i];

        si = ItemAddr + 1 ;
        while( si < 25 )
	   head_dex.dex[SlibAddr].item[si] += TrueLength, si++;
	return(1);
    }
    return(err_proc());
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
FillNew(WordLong, pt)
WORD WordLong;
LPSTR pt;
{
    WORD cx, ax, bx, i;
    unsigned short *pp;

    pp = (unsigned short *)&SubBuffer[PageAddr];

    if(!KzkItemLength)
    {		
	if (WordLong < 5)
	    cx=3, ax=6;
	if (WordLong >= 5)
	    cx=5, ax=10;
	for(i = 0; i < cx; i++)
	    pp[i] = ax;

	WordStart += ax;
    }

    for (i = 0; i < WordLong*2; i++)
	SubBuffer[WordStart+i] = pt[i];
    if ( *(pt-1)&0x80)
	SubBuffer[WordStart+i] = '1';
    else
	SubBuffer[WordStart+i] = 'm';

/*	New word is alredy fill in, now ajust the group_index  */

    if (WordLong < 5)
	cx=3, bx=WordLong-2;
    else
	cx=5, bx=WordLong-5;

    while(bx < cx)
	pp[bx] += WordLong*2+1, bx++;  

/*  Change the group_index para is over. Now, add word number to the index. */

    head_dex.ttl_words++;		

    if ( WordLong >= 5) 
        head_dex.fiveup_words++;
    else 
    {
	if (WordLong == 2)  head_dex.two_words++;
	if (WordLong == 3)  head_dex.three_words++;
	if (WordLong == 4)  head_dex.four_words++;
    }
}

/******************************************************************************/
/* FUNCTION    : GetHead                                                      */
/* DESCRIPTION : Get Chinese syllable internal code                           */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
GetHead(cisu_code)
WORD cisu_code;
{
    int i;
    unsigned short py_code;

    if (cisu_code >= 0x8000)
        py_code = cisu.t_bf2[(cisu_code-0x8000)] & 0x3ff;
    else
        py_code = cisu.t_bf1[(cisu_code-0x2020)] & 0x3ff;

    for (i = 0; i < 24; i++)
        if ((py_code >= pyml_index[i].num)&&(py_code < pyml_index[i+1].num))
            return(pyml_index[i].head);
    return(0);
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
    if (up_rem_area != 0)
        free(up_rem_area);

    if (SubBuffer != 0)
        free(SubBuffer);

    if(fd_rem > 0)
    {
        lseek(fd_rem, 0, 0);
        lockf(fd_rem, F_ULOCK, 0);
        close(fd_rem);
    }
    if(fd_usr > 0)
    {
        lseek(fd_usr, 0, 0);
        lockf(fd_usr, F_ULOCK, 0);
        close(fd_usr);
    }
    if(fd_tmp > 0)
    {
        lseek(fd_tmp, 0, 0);
        lockf(fd_tmp, F_ULOCK, 0);
        close(fd_tmp);
    }

    if (mode == 7)
	return(OK);
    return(FALSE);
}

/******************************************************************************/
/* FUNCTION    : err_proc                                                     */
/* DESCRIPTION : Processing  error and return error message.                  */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
err_proc()
{
    if (up_rem_area != 0)
       free(up_rem_area);

    if (SubBuffer != 0)
       free(SubBuffer);

    if(fd_rem > 0)
    {
        lseek(fd_rem, 0, 0);
        lockf(fd_rem, F_ULOCK, 0);
        close(fd_rem);
    }
    if(fd_usr > 0)
    {
        lseek(fd_usr, 0, 0);
        lockf(fd_usr, F_ULOCK, 0);
        close(fd_usr);
    }
    if(fd_tmp > 0)
    {
        lseek(fd_tmp, 0, 0);
        lockf(fd_tmp, F_ULOCK, 0);
        close(fd_tmp);
    }

    return(ERROR);
}
