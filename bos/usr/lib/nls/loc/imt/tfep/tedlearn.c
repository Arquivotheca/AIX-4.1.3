static char sccsid[] = "@(#)67  1.4  src/bos/usr/lib/nls/loc/imt/tfep/tedlearn.c, libtw, bos411, 9428A410j 6/14/94 20:38:17";
/*
 * COMPONENT_NAME: LIBTW
 *
 * FUNCTIONS: tedlearn.c
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*********************************************************************/
/* Header for Learning file                                          */
/*                                                                   */
/* Identificatio ID "IBMT" ( 4 bytes )                               */
/* Free pointer  ( 4 bytes )                                         */
/*                                                                   */
/* Header for Simplified Tsang_Jye                                   */
/*       26 radicals                                                 */
/* A*B : 26*26=676                                                   */
/* AB* : 26*26=676                                                   */
/* A*  : 26                                                          */
/*       Toatl=(26+2+676*2)*4=5520 ---> 10 blocks                    */
/*                                                                   */
/* Header for Phonetic                                               */
/*       41 radicals                                                 */
/* First two phonetic radicals 41*41*4=6724 ---> 14 blocks           */
/*                                                                   */
/*       Format of the learning file                                 */
/*                                                                   */
/* +-----+----+----+-----------------------+-------------+           */
/* |     | ID |Free|                       |             |           */
/* |     +----+----+     A*B (Z*A)         |             V           */
/* |     |                                 |                         */
/* |     +---------------------------------+   Simplified For System */
/* V     |               AB* (ZA*)         |   And User Defined      */
/* Offset+---------------------------------+             A           */
/* Area  |               A*  (Z*)          |             |           */
/*(Index)+-+-+-----------------------------+-------------+           */
/* A     |1|2| ...                         |             |           */
/* |     +-+-+-+                           |             V           */
/* |     |12|13| ...                       |                         */
/* |     +--+--+                        +--+        Phonetic Area    */
/* |     |                          ... |ZZ|                         */
/* |     |                              +--+             A           */
/* |     |                                 |             |           */
/* +-----+------------------+--------------+-------------+           */
/*       |    ...           |              |             |           */
/*       +------------------+              |             V           */
/*       |                                 |                         */
/*       |                                 |   Learning Data Area    */
/*       |                                 |                         */
/*       |                                 |             A           */
/*       |                                 |             |           */
/*       +---------------------------------+-------------+           */
/*                                                                   */
/*       Format of the learning data                                 */
/*                                                                   */
/*         1. Simplified Tsang_Jye or Phonetic with one or two       */
/*                                     +--- Index          radicals  */
/*                                     |                             */
/*                                     V                             */
/*            +--+--+--+--+--+--+--+--+--+--+                        */
/*    +------>|  |  |  |  |  |  |  |  |  |  |  Learning data         */
/*    |       +--+--+--+--+--+--+--+--+--+--+                        */
/*    |                                                              */
/*    |    2. Phonetic ( >= 3 radicals )                             */
/*    |                                                              */
/*    |       +--+--+--+--+--+--+--+--+--+--+--+--+--+               */
/*    +-------| 2| 3| 4| 5| 6| 7| 8| 9|10|11|12|13|14| Offset        */
/*            +--+--+--+--+--+--+--+--+--+--+--+--+--+               */
/*                |                                                  */
/*                |                                                  */
/*                | >= 3 radicals                                    */
/*                |                                                  */
/*                |                                                  */
/*                |  +--------------+----------------+               */
/*                +->|//////////////|          |C| L |  Learning data*/
/*                   +--------------+----------------+               */
/*                    ||C : Count     L : Link pointer               */
/*                    ||            +--- Index                       */
/*                    ||            |                                */
/*                    \/            V                                */
/*                    +------+--+--+--+--+--+--+--+--+--+--+         */
/*                    | Keys |  |  |  |  |  |  |  |  |  |  |         */
/*                    +------+--+--+--+--+--+--+--+--+--+--+         */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/* V410                                                              */
/* All the routines in this files are for learning features          */
/*********************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tedinit.h"
#define BLOCKSIZE 512
#define BLOCKNO 14 /***** Number of blocks needed *****/
#define PHBUFFERSIZE BLOCKSIZE/2 /***** Area to hold Phonetic data *****/
#define LEADING      8    /* Leading bytes in learning file */
#define FILEID "IBMT"     /* ID for learning file */
#define NUMBER1 256*256*256
#define NUMBER2 256*256
#define NUMBER3 676*4
#define NUMBER4 676*4*2
#define NUMBER5 LEARN_NO*4
#define NUMBER6 26*4
#define NUMBER7 10*4
#define NUMBER8 41*4
#define NUMBER9 36*4
#define NO_PH 41
#define LEARN_OFF 0
char ph_key[NO_PH]={
                    'A','B','C','D','E','F','G','H','I','J','K','L','M',
                    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
                    '1','2','3','4','5','6','7','8','9','0','-',';',',',
                    '.','/'
                   };

char ph_euc[NO_PH]={
                  /* A    B    C    D    E    F    G    H    I    J   */
                    0xe9,0xe7,0xe5,0xeb,0xd4,0xdc,0xdd,0xde,0xd9,0xdf,
                  /* K    L    M    N    O    P    Q    R    S    T   */
                    0xe0,0xe1,0xee,0xe8,0xda,0xdb,0xd2,0xd5,0xea,0xd6,
                  /* U    V    W    X    Y    Z    1    2    3    4   */
                    0xd8,0xe6,0xd3,0xe4,0xd7,0xe3,0xc7,0xc8,0xc9,0xca,
                  /* 5    6    7    8    9    0    -    ;    ,    .   */
                    0xcb,0xcc,0xcd,0xce,0xcf,0xd0,0xd1,0xe2,0xef,0xf0,
                  /* /   */
                    0xec
                   };

/*********************************************************************/
/*     Locate the radical in the ph_key                              */
/*********************************************************************/
int search_phrad(rad)
char rad;
{
 int num1;
 for(num1=0;num1 < NO_PH;num1++)
   {
    if(rad == ph_euc[num1])break;
   }
 return(num1);
}

/*********************************************************************/
/*     Transform 4-bytes char into unsigned long integer             */
/*********************************************************************/
unsigned long bint4(b)
unsigned char *b;
{
  return((unsigned long)(b[0]*NUMBER1+b[1]*NUMBER2+b[2]*256+b[3]));
}

/*********************************************************************/
/*     Transform 2-bytes char into unsigned long integer             */
/*********************************************************************/
unsigned long bint2(b)
unsigned char *b;
{
  return((unsigned long)(b[0]*256+b[1]));
}

/*********************************************************************/
/*     Set all indexes to be 0                                       */
/*********************************************************************/
reset_index(fepcb)
FEPCB *fepcb;
{
 int num1;
 for(num1=0;num1<LEARN_NO;num1++)
   {
    fepcb->learnstruct.index[num1]=num1;
   }
}

/*********************************************************************/
/*     Set all list_indexes to be 0                         Debby   */
/*********************************************************************/
reset_list_index(fepcb)
FEPCB *fepcb;
{
 int num1;
 for(num1=0;num1<LIST_NO;num1++)
   {
    fepcb->learnstruct.list_index[num1]=num1;
   }
}


/*********************************************************************/
/*       This routine is to create learning file.                    */
/*********************************************************************/
create_learn_file(fepcb)
FEPCB *fepcb;
{
 if((fepcb->fd.learnfd=fopen(fepcb->fname.learnname,"w+b")) == NULL)
   {
    /***** Learning can not work in this condition *****/
    fepcb->learning = LEARN_OFF;
    free(fepcb->fname.learnname);
   }
 else
   {
    /***** Initialize data in learning file *****/
    if((fepcb->learnstruct.phlearndata=(char *)calloc(BLOCKSIZE*BLOCKNO,1))
       != NULL)
      {
       /***** Set file ID *****/
       fwrite((char *)FILEID,1,4,fepcb->fd.learnfd);

       /***** Free area *****/
       fseek(fepcb->fd.learnfd,(long )4,SEEK_SET);
       fepcb->learnstruct.free=(BLOCKSIZE*BLOCKNO)*2-1;
       fwrite((char *)&fepcb->learnstruct.free,1,4,fepcb->fd.learnfd);

       /***** Offset area for Simplified Tsang_Jye *****/
       memset(fepcb->learnstruct.phlearndata,0x0,BLOCKSIZE*BLOCKNO);
       fwrite((char *)fepcb->learnstruct.phlearndata,BLOCKSIZE,BLOCKNO,
              fepcb->fd.learnfd);

       /***** Offset area for Phonetic *****/
       fwrite((char *)fepcb->learnstruct.phlearndata,BLOCKSIZE,BLOCKNO,
              fepcb->fd.learnfd);

       fflush(fepcb->fd.learnfd);
       fepcb->learnstruct.phlearndata=(char *)realloc(fepcb->learnstruct.phlearndata,
                                       PHBUFFERSIZE);
       memset(fepcb->learnstruct.phlearndata,NULL,PHBUFFERSIZE);
      }
    else
      {
       /***** Learning can not work in this condition *****/
       /* printf("****  Learning can not work in this condition ****\n"); */
       fepcb->learning = LEARN_OFF;
       free(fepcb->fname.learnname);
      }
   }
}

/*********************************************************************/
/* Initialize learning structure                                     */
/*********************************************************************/
init_learning(fepcb)
FEPCB *fepcb;
{
 char fileid[5];
 if((fepcb->fd.learnfd=fopen(fepcb->fname.learnname,"r+b")) == NULL)
   {
    create_learn_file(fepcb);
   }
 else
   {
    if((fepcb->learnstruct.phlearndata=(char *)calloc(PHBUFFERSIZE,1)) != NULL)
      {
       /***** Check file permission *****/
       /***** Check file format *****/
       memset(fepcb->learnstruct.phlearndata,NULL,PHBUFFERSIZE);
       fread((char *)fileid,1,4,fepcb->fd.learnfd);
       if(strncmp(fileid,FILEID,4) == 0)
         {
          /***** Read in free area data *****/
          fread((char *)&fepcb->learnstruct.free,1,4,fepcb->fd.learnfd);
         }
       else
         {
          /*** The format of the learning file is not correct. ***/
          /***** Learning does not support at this condition *****/
          free(fepcb->learnstruct.phlearndata);
          fepcb->learning = LEARN_OFF;
          free(fepcb->fname.learnname);
         }
      }
    else
      {
       /*** The format of the learning file is not correct. ***/
       /***** Learning does not support at this condition *****/
       /* printf("****  Learning can not work in this condition ****\n"); */
       fepcb->learning = LEARN_OFF;
       free(fepcb->fname.learnname);
      }
   }
}


/*********************************************************************/
/*       This routine is to update learning data.                    */
/*********************************************************************/
update_learn_file(fepcb)
FEPCB *fepcb;
{
  long toffset;
  int num1,num2,num3,num4,count;
  unsigned char *indexptr;
  unsigned int  *countptr;
  switch(fepcb->learnstruct.mode)
    {
     case ASTARTB : /* A*B and Z*A */
     case ABSTART : /* AB* and ZA* */
     case ASTART  : /* A* and Z* */
          if(fepcb->learnstruct.offset2 > 0)
            { /* Exist */
             fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset2,SEEK_SET);
             fwrite((char *)fepcb->learnstruct.index,sizeof(int),LEARN_NO,
                    fepcb->fd.learnfd);
            }
          else
            { /* Not exist */
             fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset1,SEEK_SET);
             fwrite((char *)&fepcb->learnstruct.free,sizeof(long),1,
                    fepcb->fd.learnfd); /* Write offset */
             fepcb->learnstruct.offset2=fepcb->learnstruct.free;
             fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset2,SEEK_SET);
             fwrite((char *)fepcb->learnstruct.index,sizeof(int),LEARN_NO,
                    fepcb->fd.learnfd); /* Learning data */
             fepcb->learnstruct.free=ftell(fepcb->fd.learnfd);
             fseek(fepcb->fd.learnfd,(long)4,SEEK_SET);
             fwrite((char *)&fepcb->learnstruct.free,sizeof(long),1,
                    fepcb->fd.learnfd); /* Free area offset */
            }
          fflush(fepcb->fd.learnfd); /* Force it written into disk */
          break;
     case PHONETIC_L :
          num1=fepcb->inputlen;
          if(fepcb->learnstruct.offset2 > 0 )
            {
             if(num1 == 1) /* One radicals */
               {
                fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset2,SEEK_SET);
                fwrite((char *)fepcb->learnstruct.index,sizeof(int),LEARN_NO,
                       fepcb->fd.learnfd);
                break;
               }
             if(fepcb->learnstruct.offset3[num1-2] > 0 &&
                     fepcb->learnstruct.phfound == 1) /* >= 2 radicals and found */
               { /* Exist */
                num1=num1-2; /* 2 <---> 14 */
                fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset3[num1],SEEK_SET);
                if(num1 == 0)
                  { /* Two radicals */
                   fwrite((char *)fepcb->learnstruct.index,sizeof(int),LEARN_NO,
                          fepcb->fd.learnfd);
                   break;
                  }
                /* Three radicals */
                indexptr=(char *)fepcb->learnstruct.index;
                for(num3=0;num3 < NUMBER5;num3++)
                  { /* Save learning data in phlearndata buffer */
                   fepcb->learnstruct.learnadd[num1+num3]=*indexptr;
                   indexptr++;
                  }
                fwrite((char *)fepcb->learnstruct.phlearndata,sizeof(char),
                       PHBUFFERSIZE,fepcb->fd.learnfd); /* Write phlearndata */
               }
             else
               { /* Not exist */
                if(num1 == 2)
                  { /* Two radicals */
                   fepcb->learnstruct.offset3[0]=fepcb->learnstruct.free;
                   fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset2,SEEK_SET);
                   fwrite((char *)fepcb->learnstruct.offset3,sizeof(long),PH_NO,
                          fepcb->fd.learnfd); /* Offset3 */
                   fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset3[0],SEEK_SET);
                   fwrite((char *)fepcb->learnstruct.index,sizeof(int),LEARN_NO,
                          fepcb->fd.learnfd); /* Learning data */
                   fepcb->learnstruct.free=ftell(fepcb->fd.learnfd);
                   fseek(fepcb->fd.learnfd,(long)4,SEEK_SET);
                   fwrite((char *)&fepcb->learnstruct.free,sizeof(long),1,
                          fepcb->fd.learnfd); /* Free area offset */
                   break;
                  }
                /* Three radicals */
                /* offset3 == 0 */
                if(fepcb->learnstruct.offset3[num1-2] == 0)
                  {
                   memset(fepcb->learnstruct.phlearndata,0x0,PHBUFFERSIZE);
                   num1=num1-2;
                   fepcb->learnstruct.learnadd=fepcb->learnstruct.phlearndata;
                   for(num2=0;num2 < num1;num2++)
                     { /* Save radicals */
                      fepcb->learnstruct.learnadd[num2]=fepcb->curinpbuf[num2+2];
                     }
                   indexptr=(char *)fepcb->learnstruct.index;
                   for(num3=0;num3 < NUMBER5;num3++)
                     {
                      fepcb->learnstruct.learnadd[num1+num3]=*indexptr;
                      indexptr++;
                     }
                   countptr=(unsigned int *)(fepcb->learnstruct.phlearndata+PHBUFFERSIZE
                            -(sizeof(long)+-sizeof(int)));
                   *countptr=PHBUFFERSIZE-sizeof(long)-sizeof(int);
                   fepcb->learnstruct.offset3[num1]=fepcb->learnstruct.free;
                   fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset2,SEEK_SET);
                   fwrite((char *)fepcb->learnstruct.offset3,sizeof(long),PH_NO,
                          fepcb->fd.learnfd); /* Save offset3 */
                   fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset3[num1],
                         SEEK_SET);
                   fwrite((char *)fepcb->learnstruct.phlearndata,sizeof(char),
                          PHBUFFERSIZE,fepcb->fd.learnfd); /* Save learning data */
                   fepcb->learnstruct.free=ftell(fepcb->fd.learnfd);
                   fseek(fepcb->fd.learnfd,(long)4,SEEK_SET);
                   fwrite((char *)&fepcb->learnstruct.free,sizeof(long),1,
                          fepcb->fd.learnfd); /* Free area offset */
                   break;
                  }
                /* offset3 > 0 */
                num3=num1-2+sizeof(long)*LEARN_NO;
                count=PHBUFFERSIZE-sizeof(long)-sizeof(int);
                if(num3 < count)
                  { /* Free area enough */
                   count=count-num3;
                   fepcb->learnstruct.learnadd=fepcb->learnstruct.phlearndata
                                               +PHBUFFERSIZE-count-(sizeof(int)
                                               +sizeof(long));
                   num1=num1-2;
                   for(num2=0;num2 < num1;num2++)
                     { /* Save radicals */
                      fepcb->learnstruct.learnadd[num2]=fepcb->curinpbuf[num2+2];
                     }
                   indexptr=(char *)fepcb->learnstruct.index;
                   for(num3=0;num3 < NUMBER5;num3++)
                     {
                      fepcb->learnstruct.learnadd[num1+num3]=*indexptr;
                      indexptr++;
                     }
                   countptr=(unsigned int *)(fepcb->learnstruct.phlearndata+PHBUFFERSIZE
                            -(sizeof(long)+-sizeof(int)));
                   *countptr=count;
                   fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset3[num1],
                         SEEK_SET);
                   fwrite((char *)fepcb->learnstruct.phlearndata,sizeof(char),
                          PHBUFFERSIZE,fepcb->fd.learnfd);
                  }
                else
                  { /* Free area not enough */
                   num1=num1-2;
                   indexptr=(char *)fepcb->learnstruct.phlearndata
                            +PHBUFFERSIZE-sizeof(long);
                   *indexptr=fepcb->learnstruct.free; /* Update link */
                   fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset3[num1],
                         SEEK_SET);
                   fwrite((char *)fepcb->learnstruct.phlearndata,sizeof(char),
                          PHBUFFERSIZE,fepcb->fd.learnfd);
                   memset((char *)fepcb->learnstruct.phlearndata,0x0,
                          PHBUFFERSIZE);
                   fepcb->learnstruct.learnadd=fepcb->learnstruct.phlearndata;
                   for(num2=0;num2 < num1;num2++)
                     {
                      fepcb->learnstruct.learnadd[num2]=fepcb->curinpbuf[2+num2];
                     }
                   indexptr=(char *)fepcb->learnstruct.index;
                   for(num4=0;num4 < NUMBER5;num4++)
                     {
                      fepcb->learnstruct.learnadd[num1+num4]=*indexptr;
                      indexptr++;
                     }
                   countptr=(unsigned int *)(fepcb->learnstruct.phlearndata-sizeof(long)
                            -sizeof(int));
                   *countptr=PHBUFFERSIZE-(LEARN_NO*sizeof(int)+num1);
                   fseek(fepcb->fd.learnfd,fepcb->learnstruct.free,
                         SEEK_SET);
                   fwrite((char *)fepcb->learnstruct.phlearndata,sizeof(char),
                          PHBUFFERSIZE,fepcb->fd.learnfd);
                   fepcb->learnstruct.free=ftell(fepcb->fd.learnfd);
                   fseek(fepcb->fd.learnfd,(long)4,SEEK_SET);
                   fwrite((char *)&fepcb->learnstruct.free,sizeof(long),1,
                          fepcb->fd.learnfd); /* Free area offset */
                  }
               }
            }
          else
            {
             if(num1 == 1) /* One radical */
               {
                toffset=fepcb->learnstruct.free;
                fseek(fepcb->fd.learnfd,toffset,SEEK_SET);
                fwrite((char *)fepcb->learnstruct.index,sizeof(int),LEARN_NO,
                       fepcb->fd.learnfd);
                fepcb->learnstruct.offset2=toffset;
                fepcb->learnstruct.free=ftell(fepcb->fd.learnfd);
                fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset1,SEEK_SET);
                fwrite((char *)&fepcb->learnstruct.offset2,sizeof(long),1,
                       fepcb->fd.learnfd);
                fseek(fepcb->fd.learnfd,(long)4,SEEK_SET);
                fwrite((char *)&fepcb->learnstruct.free,sizeof(long),1,
                       fepcb->fd.learnfd);
               }
             else
               { /* >= 2 radicals */
                for(num2=0;num2 < PH_NO;num2++)
                  {
                   fepcb->learnstruct.offset3[num2]=0;
                  }
                fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset1,SEEK_SET);
                fwrite((char *)&fepcb->learnstruct.free,sizeof(long),1,
                       fepcb->fd.learnfd); /* Offset2 */
                fepcb->learnstruct.offset2=fepcb->learnstruct.free;
                num2=num1-2;
                if(fepcb->inputlen == 2)
                  { /* Two radicals */
                   fepcb->learnstruct.offset3[num2]=fepcb->learnstruct.free+
                                                    PH_NO*sizeof(long);
                   fseek(fepcb->fd.learnfd,fepcb->learnstruct.free,SEEK_SET);
                   fwrite((char *)fepcb->learnstruct.offset3,sizeof(long),PH_NO,
                          fepcb->fd.learnfd);
                   fepcb->learnstruct.free=ftell(fepcb->fd.learnfd);
                   fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset3[num2],
                         SEEK_SET);
                   fwrite((char *)fepcb->learnstruct.index,sizeof(int),
                          LEARN_NO,fepcb->fd.learnfd);
                   fepcb->learnstruct.free=ftell(fepcb->fd.learnfd);
                   fseek(fepcb->fd.learnfd,(long)4,SEEK_SET);
                   fwrite((char *)&fepcb->learnstruct.free,sizeof(long),1,
                          fepcb->fd.learnfd); /* Free area offset */
                   break;
                  }
                /* >= Three radicals */
                num1=num1-2;
                memset((char *)fepcb->learnstruct.phlearndata,0x0,
                       PHBUFFERSIZE);
                fepcb->learnstruct.offset3[num1]=fepcb->learnstruct.free+
                                                 PH_NO*sizeof(long);;
                fseek(fepcb->fd.learnfd,fepcb->learnstruct.free,SEEK_SET);
                fwrite((char *)fepcb->learnstruct.offset3,sizeof(long),PH_NO,
                       fepcb->fd.learnfd);
                fepcb->learnstruct.free=ftell(fepcb->fd.learnfd);
                fepcb->learnstruct.learnadd=fepcb->learnstruct.phlearndata;
                for(num2=0;num2 < num1;num2++)
                  { /* Save radicals */
                   fepcb->learnstruct.learnadd[num2]=fepcb->curinpbuf[2+num2];
                  }
                indexptr=(char *)fepcb->learnstruct.index;
                for(num3=0;num3 < NUMBER5;num3++)
                  { /* Save indexes */
                   fepcb->learnstruct.learnadd[num1+num3]=*indexptr;
                   indexptr++;
                  }
                countptr=(unsigned int *)(fepcb->learnstruct.phlearndata+PHBUFFERSIZE-
                          (sizeof(long)+sizeof(int)));
                count=PHBUFFERSIZE-(sizeof(long)+sizeof(int))-num1-
                      LEARN_NO*sizeof(int);
                *countptr=count;
                fseek(fepcb->fd.learnfd,fepcb->learnstruct.offset3[num1],
                      SEEK_SET);
                fwrite((char *)fepcb->learnstruct.phlearndata,sizeof(char),
                       PHBUFFERSIZE,fepcb->fd.learnfd);

                fepcb->learnstruct.free=ftell(fepcb->fd.learnfd);
                fseek(fepcb->fd.learnfd,(long)4,SEEK_SET);
                fwrite((char *)&fepcb->learnstruct.free,sizeof(long),1,
                       fepcb->fd.learnfd);
               }
            }
          fflush(fepcb->fd.learnfd); /* Force it written into disk */
          break;
     default:
          break;
    } /* End switch */
          fflush(fepcb->fd.learnfd); /* Force it written into disk */
}

/*********************************************************************/
/*       This routine is to access learning data.                    */
/*********************************************************************/
access_learn_data(fepcb)
FEPCB *fepcb;
{
  int num1,num2,num3,num4;
  char test;
  int length;
  /* Find first file offset */
  switch(fepcb->learnstruct.mode)
    {
     case ASTARTB: /* A*B and Z*A */                        /* 104=4*26 */
          fepcb->learnstruct.offset1=(long)(fepcb->curinpbuf[0]-'A')*104;
          fepcb->learnstruct.offset1=fepcb->learnstruct.offset1
                 +(fepcb->curinpbuf[2]-'A')*4+LEADING;
          length=fepcb->stjstruct.allcandno;
          break;
     case ABSTART: /* AB* and ZA* */
          fepcb->learnstruct.offset1=LEADING+NUMBER3;
          fepcb->learnstruct.offset1=(fepcb->curinpbuf[0]-'A')*104
                                     +fepcb->learnstruct.offset1; /* 104=4*26 */
          fepcb->learnstruct.offset1=fepcb->learnstruct.offset1
                 +(fepcb->curinpbuf[2]-'A')*4;
          length=fepcb->stjstruct.allcandno;
          break;
     case ASTART: /* A* and Z* */
          fepcb->learnstruct.offset1=NUMBER4+LEADING;
          fepcb->learnstruct.offset1=(fepcb->curinpbuf[0]-'A')*4;
          length=fepcb->stjstruct.allcandno;
          break;
     case PHONETIC_L: /* Phonetic */ /* (26+2+676*2)*4 */
          length=fepcb->phstruct.allcandno;
          fepcb->learnstruct.offset1=NUMBER4+112;
          num2=0;
          num3=1;
          if(fepcb->inputlen >= 2)num3=2;
          for(num1=0;num1 < fepcb->inputlen;num1++)
            {
             num4=search_phrad(fepcb->curinpbuf[num1]);
             fepcb->curinpbuf[num1]=ph_key[num4];
             if(num1 < num3)num2=num2*41+num4;
            }
          if(num3 == 2)num2=num2+41;
          num2=num2*4;
          fepcb->learnstruct.offset1=fepcb->learnstruct.offset1+num2;
          break;
     default: /* Error */
          break;
    } /* End switch */

 /* Find second offset */
 num1=fseek(fepcb->fd.learnfd,(long)fepcb->learnstruct.offset1,SEEK_SET);
 fread((char *)&fepcb->learnstruct.offset2,sizeof(long),1,fepcb->fd.learnfd);
 if(fepcb->learnstruct.mode == PHONETIC_L)
   {
    /* Find third offset */
    if(fepcb->learnstruct.offset2 != 0)
      {
       switch(fepcb->inputlen)
         {
          case 1 : /* Only one radical */
            fseek(fepcb->fd.learnfd,(long)fepcb->learnstruct.offset2,SEEK_SET);
            fread((char *)fepcb->learnstruct.index,sizeof(int),LEARN_NO,
                  fepcb->fd.learnfd);
            break;
          default : /* >= 2 radical */
            fseek(fepcb->fd.learnfd,(long)fepcb->learnstruct.offset2,SEEK_SET);
            fread((char *)fepcb->learnstruct.offset3,sizeof(long),PH_NO,
                  fepcb->fd.learnfd);
            /* Access learning data */
            access_phdata(fepcb);
            break;
         }  /* End switch */
      }
    else
      {
       reset_index(fepcb);
       return;
      }
   }
 else
   {
    /* Access learning data for Simplified mode */
    if(fepcb->learnstruct.offset2 != 0)
      {
       fseek(fepcb->fd.learnfd,(long)fepcb->learnstruct.offset2,SEEK_SET);
       fread((char *)fepcb->learnstruct.index,sizeof(int),LEARN_NO,
             fepcb->fd.learnfd);
      }
    else
      { /* No learning */
       reset_index(fepcb);
       return;
      }
   }
 /* Reorder if necessary */
 num2=length;
 if(length > LEARN_NO)num2=LEARN_NO;
 for(num1=0;num1 < num2;num1++)
   {
    if(fepcb->learnstruct.index[num1] >= length)
      {
       reset_index(fepcb);  /* Reset data if not consistent */
       break;
      }
   }
}


/*********************************************************************/
/*     Access Phonetic learning data                                 */
/*********************************************************************/
access_phdata(fepcb)
FEPCB *fepcb;
{
 int num1,num2,num3;
 char *ptr,*sptr,*temp;
 int flag; /* 0 : run  1 : find out  2 : not exist */
 unsigned long count,nexp;
 char tempdata[13];
 num1=fepcb->inputlen-2; /* Remaining length */
 fepcb->learnstruct.phfound=0;
 if(fepcb->learnstruct.offset3[num1] != 0)
   {
    /***** Search to find out the right one *****/
    if(fepcb->inputlen == 2)
      {
       fseek(fepcb->fd.learnfd,(long)fepcb->learnstruct.offset3[num1],
             SEEK_SET);
       fread((char *)fepcb->learnstruct.index,sizeof(int),LEARN_NO,
             fepcb->fd.learnfd);
       fepcb->learnstruct.phfound=1;
      }
    else
      { /* >= 3 radicals */
       flag=0;
       fepcb->learnstruct.phfound=0;
       sptr=fepcb->curinpbuf+2;
       while(flag == 0)
         {
          fseek(fepcb->fd.learnfd,(long)fepcb->learnstruct.offset3[num1],
                SEEK_SET);
          fread((char *)fepcb->learnstruct.phlearndata,1,PHBUFFERSIZE,
                fepcb->fd.learnfd);
          ptr=fepcb->learnstruct.phlearndata;
          nexp=bint4(ptr+PHBUFFERSIZE-4);
          count=bint2(ptr+PHBUFFERSIZE-6);
          num2=PHBUFFERSIZE-count-(sizeof(long)+sizeof(int))+1;

          /***** Compare the radicals *****/
          while(num2 > 0 && flag == 0)
            {
             if(strncmp(ptr,sptr,num1) == 0)
               {
                fepcb->learnstruct.learnadd=ptr;
                flag=1; /* Find out the corresponding radicals */
                ptr=ptr+fepcb->inputlen-2;
                temp=(char *)fepcb->learnstruct.index;
                for(num3=0;num3 < NUMBER5;num3++)
                  { /* Access the learning data */
                   *temp=*ptr;
                   ptr++;
                   temp++;
                  }
                fepcb->learnstruct.phfound=1;
               }
             num2=num2-num1-NUMBER5;  /* Decrease count */
             ptr=ptr+num1+NUMBER5;    /* Forward Keys plus indexes */
            } /* End while(num2 > 0 && flag == 0) */

          /***** Get next block if not found *****/
          if(flag != 1 && nexp > 0)
            {
             fepcb->learnstruct.offset3[num1]=nexp; /* Next offset */
            }
          else
            {
             if(nexp == 0 && flag == 0)flag=2; /* Not exist */
            }
        } /* End while(flag == 0) */
       if(flag == 2)
         {
          reset_index(fepcb);
         }
      }
   }
 else
   {
    reset_index(fepcb);
   }
}

/*********************************************************************/
/*     Free memory allocated by learning feature                     */
/*********************************************************************/
free_learn_mem(fepcb)
FEPCB *fepcb;
{
 free(fepcb->learnstruct.phlearndata);
 free(fepcb->fname.learnname);
 fclose(fepcb->fd.learnfd);
}

