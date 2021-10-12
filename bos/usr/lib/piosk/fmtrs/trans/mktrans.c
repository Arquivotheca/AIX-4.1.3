static char sccsid[] = "@(#)37  1.1  src/bos/usr/lib/piosk/fmtrs/trans/mktrans.c, cmdpiosk, bos411, 9428A410j 7/17/93 16:35:08";

/*
 *   COMPONENT_NAME: (CMDPIOSK) 
 *
 *   FUNCTIONS: main
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


static void swap(unsigned int *, unsigned int *);
static void qqsort(unsigned int **, int, int, int);

struct trans_table {		/*** Translation Table Struct ***/
    unsigned int rsv1; 		/* reserved  			*/
    unsigned int f_cp; 		/* from code point              */
    unsigned int rsv2; 		/* reserved  			*/
    unsigned int t_cp; 		/* to code point                */
} *trtable; 

#define TPATH "./"

#if defined(SJIS90TOIBM932)

#define TFROM "sjis90"
#define TTO   "IBM-932"
int num = 1;
static unsigned int	ttable[][2] = {
	{0x81ca, 0xfa54}, {0x81e6, 0xfa5b}, {0x88b1, 0xe9cb}, {0x89a7, 0xe9f2}, 
	{0x8a61, 0xe579}, {0x8a68, 0x9d98}, {0x8a96, 0xe27d}, {0x8ac1, 0x9ff3}, 
	{0x8ad0, 0xe67c}, {0x8bc4, 0xea9f}, {0x8c7a, 0xe8f2}, {0x8d56, 0xfad0}, 
	{0x8d7b, 0xe1e6}, {0x8ec7, 0xe541}, {0x9078, 0xe8d5}, {0x9147, 0xe6cb}, 
	{0x92d9, 0x9ae2}, {0x9376, 0xe1e8}, {0x938e, 0x9e8d}, {0x9393, 0x9fb7}, 
	{0x93f4, 0xe78e}, {0x9488, 0xe5a2}, {0x954f, 0x9e77}, {0x968a, 0xeaa0}, 
	{0x9699, 0x98d4}, {0x96f7, 0xe54d}, {0x9779, 0xeaa1}, {0x9855, 0xe2c4}, 
	{0x98d4, 0x9699}, {0x9ae2, 0x92d9}, {0x9d98, 0x8a68}, {0x9e77, 0x954f}, 
	{0x9e8d, 0x938e}, {0x9fb7, 0x9393}, {0x9ff3, 0x8ac1}, {0xe086, 0xeaa4}, 
	{0xe0f4, 0xeaa2}, {0xe1e6, 0x8d7b}, {0xe1e8, 0x9376}, {0xe27d, 0x8a96}, 
	{0xe2c4, 0x9855}, {0xe541, 0x8ec7}, {0xe54d, 0x96f7}, {0xe579, 0x8a61}, 
	{0xe5a2, 0x9488}, {0xe67c, 0x8ad0}, {0xe6cb, 0x9147}, {0xe78e, 0x93f4}, 
	{0xe8d5, 0x9078}, {0xe8f2, 0x8c7a}, {0xe9cb, 0x88b1}, {0xe9f2, 0x89a7}, 
	{0xea9f, 0x8bc4}, {0xeaa0, 0x968a}, {0xeaa1, 0x9779}, {0xeaa2, 0xe0f4}, 
	{0xeaa4, 0xe086}, {0xfad0, 0x8d56}
};

#elif defined (SJIS83TOIBM932)

#define TFROM "sjis83"
#define TTO   "IBM-932"
int	num = 1;
static unsigned int	ttable[][2] = {
	{0x81ca, 0xfa54}, {0x81e6, 0xfa5b}, {0x88b1, 0xe9cb}, {0x89a7, 0xe9f2}, 
	{0x8a61, 0xe579}, {0x8a68, 0x9d98}, {0x8a96, 0xe27d}, {0x8ac1, 0x9ff3}, 
	{0x8ad0, 0xe67c}, {0x8bc4, 0xea9f}, {0x8c7a, 0xe8f2},
	{0x8d7b, 0xe1e6}, {0x8ec7, 0xe541}, {0x9078, 0xe8d5}, {0x9147, 0xe6cb}, 
	{0x92d9, 0x9ae2}, {0x9376, 0xe1e8}, {0x938e, 0x9e8d}, {0x9393, 0x9fb7}, 
	{0x93f4, 0xe78e}, {0x9488, 0xe5a2}, {0x954f, 0x9e77}, {0x968a, 0xeaa0}, 
	{0x9699, 0x98d4}, {0x96f7, 0xe54d}, {0x9779, 0xeaa1}, {0x9855, 0xe2c4}, 
	{0x98d4, 0x9699}, {0x9ae2, 0x92d9}, {0x9d98, 0x8a68}, {0x9e77, 0x954f}, 
	{0x9e8d, 0x938e}, {0x9fb7, 0x9393}, {0x9ff3, 0x8ac1},
	{0xe0f4, 0xeaa2}, {0xe1e6, 0x8d7b}, {0xe1e8, 0x9376}, {0xe27d, 0x8a96}, 
	{0xe2c4, 0x9855}, {0xe541, 0x8ec7}, {0xe54d, 0x96f7}, {0xe579, 0x8a61}, 
	{0xe5a2, 0x9488}, {0xe67c, 0x8ad0}, {0xe6cb, 0x9147}, {0xe78e, 0x93f4}, 
	{0xe8d5, 0x9078}, {0xe8f2, 0x8c7a}, {0xe9cb, 0x88b1}, {0xe9f2, 0x89a7}, 
	{0xea9f, 0x8bc4}, {0xeaa0, 0x968a}, {0xeaa1, 0x9779}, {0xeaa2, 0xe0f4}
	};

#elif defined(EUC78TOIBMEUCJP)

#define TFROM "euc78"
#define TTO   "IBM-eucJP"
int	num = 1;
static unsigned int   ttable[][2] = {
	{0xf2cd, 0xB0B3}, {0xf2f4, 0xB2A9}, {0xe9da, 0xB3C2}, {0xd9f8, 0xB3C9}, 
	{0xe3de, 0xB3F6}, {0xdef5, 0xB4C3}, {0xebdd, 0xB4D2}, {0xf0f4, 0xB7DB},
      {0x8ff4b6, 0xB9B7}, {0xe2e8, 0xB9DC}, {0xe9a2, 0xBCC9}, {0xf0d7, 0xBFD9}, 
	{0xeccd, 0xC1A8}, {0xd4e4, 0xC4DB}, {0xe2ea, 0xC5D7}, {0xdbed, 0xC5EE}, 
	{0xdeb9, 0xC5F3}, {0xedee, 0xC6F6}, {0xeaa4, 0xC7E8}, {0xdbd8, 0xC9B0}, 
	{0xd0d6, 0xCBF9}, {0xe9ae, 0xCCF9}, {0xe4c6, 0xCFB6}, {0xcbf9, 0xD0D6}, 
	{0xc4db, 0xD4E4}, {0xb3c9, 0xD9F8}, {0xc9b0, 0xDBD8}, {0xc5ee, 0xDBED}, 
	{0xc5f3, 0xDEB9}, {0xb4c3, 0xDEF5}, {0xb9dc, 0xE2E8}, {0xc5d7, 0xE2EA}, 
	{0xb3f6, 0xE3DE}, {0xcfb6, 0xE4C6}, {0xbcc9, 0xE9A2}, {0xccf9, 0xE9AE}, 
	{0xb3c2, 0xE9DA}, {0xc7e8, 0xEAA4}, {0xb4d2, 0xEBDD}, {0xc1a8, 0xECCD}, 
	{0xc6f6, 0xEDEE}, {0xbfd9, 0xF0D7}, {0xb7db, 0xF0F4}, {0xb0b3, 0xF2CD}, 
	{0xb2a9, 0xF2F4}, {0xb9b7, 0x8FF4B6}
    };

#elif defined(EUC83TOIBMEUCJP)

#define TFROM "euc83"
#define TTO   "IBM-eucJP"

int	num = 1;
static unsigned int ttable[][2] = {
        {0xb9b7, 0x8ff4b6}, {0x8ff4b6, 0xb9b7}
    };

#elif defined(JIS83TOFOLD7)

#define TFROM "jis83"
#define TTO   "fold7"

int	num = 1;
static unsigned int ttable[][2] = {
        {0x7426, 0x5f66}, {0x5f66, 0x7426}
    };

#elif defined(JIS78TOFOLD7)

#define TFROM "jis78"
#define TTO   "fold7"
static unsigned int	ttable[1][2]; 
int num = 0;

#elif defined(JIS90TOFOLD7)

#define TFROM "jis90"
#define TTO   "fold7"
static unsigned int	ttable[1][2]; 
int num = 0;

#endif

#define MAXNUM	1000

static void swap(unsigned int *x, unsigned int *y)
{
    int tmp;

    tmp = *x;
    *x = *y; 
    *y = tmp;
}

static void qqsort(unsigned int **ttp, int left, int right, int pos)
{
    int i, last;
    
    if (left >= right)
	return;
    swap(&ttp[left], &ttp[(left + right)/2]);
    last = left;
    for(i = left+1 ; i <= right; i++)
	if(ttp[i][pos] < ttp[left][pos])
	    swap(&ttp[++last], &ttp[i]);
    swap(&ttp[left], &ttp[last]);
    qqsort(ttp, left, last-1, pos);
    qqsort(ttp, last+1, right, pos);
}

main(){
    int transfd;
    long hdsize = 32;		/* header size 	                */
    long cpsize = 4;		/* code point size              */
    long reserv1 = 0;
    long reserv2 = 0;
    char tfile[256];
    unsigned int **ttp;
    int n, i;
    
    if(num){
	num= sizeof(ttable)/sizeof(ttable[0]);
	/*fprintf(stderr, "num = %d\n", num);*/
	if((trtable = (struct trans_table *)
	    malloc(sizeof(struct trans_table)*num)) == NULL ||
	   (ttp = (unsigned int **)malloc(sizeof(int **)*num)) == NULL){
	    fprintf(stderr, "malloc error\n");
	    return(-1);
	}
    }
    
    for (i = 0; i < 2; i++){
	for (n = 0; n < num; n++)
	    ttp[n] = ttable[n];
	
	qqsort(ttp, 0, num-1, i);

	switch (i){
	case 0:
	    strcpy(tfile, TPATH);
	    strcat(tfile, TFROM);
	    strcat(tfile, "_");
	    strcat(tfile, TTO);

	    for (n = 0; n < num; n++){
		trtable[n].rsv1 = 0;
		trtable[n].f_cp = ttp[n][0];
		trtable[n].rsv2 = 0;
		trtable[n].t_cp = ttp[n][1];
	    }
	    break;
	case 1:
	    strcpy(tfile, TPATH);
	    strcat(tfile, TTO);
	    strcat(tfile, "_");
	    strcat(tfile, TFROM);
	    
	    for (n = 0; n < num; n++){
		trtable[n].rsv1 = 0;
		trtable[n].f_cp = ttp[n][1];
		trtable[n].rsv2 = 0;
		trtable[n].t_cp = ttp[n][0];
	    }
	    break;
	}
	
	if((transfd = (int) open(tfile, O_CREAT | O_WRONLY, 0664)) == 0){
	    fprintf(stderr, "can't open %s\n", tfile);
	    return(-1);
	}
	/*
	 *  Write Header Information
	 */
	if(write(transfd, "PIOSMBCSXLATE000",16) < 0 ||
	   write(transfd, &hdsize, sizeof(long)) < 0 ||
	   write(transfd, &cpsize, sizeof(long)) < 0 ||
	   write(transfd, &reserv1,sizeof(long)) < 0 ||
	   write(transfd, &reserv2,sizeof(long)) < 0){
	    fprintf(stderr, "Write error!!!\n");
	    return(-1);
	}
	/*
	 * Write Data
	 */
	if(num){
	    if(write(transfd, trtable,  sizeof(struct trans_table) * num) < 0){
		fprintf(stderr, "Write error!!!\n");
		return(-1);
	    }
	}
	(void) close(transfd);
    }    
    return(0);
}
