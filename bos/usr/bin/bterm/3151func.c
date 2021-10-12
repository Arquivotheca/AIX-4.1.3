static char sccsid[] = "@(#)74	1.1  src/bos/usr/bin/bterm/3151func.c, libbidi, bos411, 9428A410j 8/26/93 13:34:38";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: do_3151_pa_keys
 *		do_3151_reset_function_key
 *		do_3151_reset_function_keys
 *		do_3151_set_attributes
 *		do_3151_set_control1
 *		do_3151_set_control2
 *		do_3151_set_control3
 *		do_3151_set_control4
 *		do_3151_set_control5
 *		do_3151_set_control6
 *		do_3151_set_control7
 *		do_3151_set_function_key
 *		init_3151
 *		set_3151_atrib
 *		set_3151_grph
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
/* ibm3151 specific functions */

#include "global.h"
#include "trace.h"

void set_3151_atrib();
void set_3151_grph();
void do_3151_reset_function_keys();
int yylex();

/*------------------------------------------------------------------------*/
void init_3151()
/* to initialize 3151 functions and escapes */
{
TRACE(("initializing 3151\n"));
       TERM=IBM3151;
       ATRIB_NORMAL=0x40; /* default attribute mode */
       EraseInput="\033K";
       NoScroll="\033\0419\070\112\033\0429\044\100";
       Scroll="\033\0419\050\112\033\0429\054\110";
       Jump="\033\042A";
       yylex_function=yylex;
       /* this is the escape that select ibm3151 mode for the IBM 
          and DEC terminals emulation cartridge of the ibm3151 terminal. */
       TERM_INIT="\033 9@";

       set_atrib=set_3151_atrib;
       set_grph=set_3151_grph;
       do_reset_function_keys=do_3151_reset_function_keys;
}
/*----------------- set screen attributes --------------------------------*/
void set_3151_atrib(atrib)
int  atrib;
{
 char atts[2];

 atts[0]=0x1b;   /* escape */
 atts[1]=0x34;     /* 4 */
 atts[2]=atrib;
 array_to_hft(atts,3);
}
/*----------------- set alternate grphics mode -----------------------------*/
void set_3151_grph(grph,old_grph)
unsigned short grph;
unsigned short old_grph;
{
 char atts[2];

 atts[0]=0x1b;   /* escape */
 switch (grph)
  {
   case 0:           /* reset alternate graphics 0 and 1 */
     if (old_grph==1)
     {
       atts[1]=0x3e;   /* > */
       atts[2]=0x42;   /* B */
     }
     else {
       atts[1]=0x3c;   /* < */
       atts[2]=0x40;   /* @ */
     }
     break;
   case 1:           /* set G0 */
     atts[1]=0x3e;   /* > */
     atts[2]=0x41;   /* A */
     break;
   case 2:           /* set G1 */
     atts[1]=0x3c;   /* < */
     atts[2]=0x41;   /* A */
     break;
  }
 array_to_hft(atts,3);
}
/*-----------------------------------------------------------------------*/
void do_3151_pa_keys()
{
 unsigned char arr[4];

  STATE=0;/* identified escape sequence */
  do_bidi_end_push();
  arr[0]=0x1b;  /* escape */
  arr[1]=0x21;  /* ! */
  arr[2]=buffer[buffer_index-2];
  arr[3]=0x0d;
  array_to_pty(arr,4);
}

/*--------------------------------------------------------------------------*/
void do_3151_set_attributes()
/* sets current character attribute */
{
char  att[4];
short count;

 STATE=0;  /* recognized char, so reset state */
 att[0] = 0x1b;                       /* escape */
 att[1] = 0x34;                       /* 4 */
 att[2] = buffer[buffer_index-1];     /* attributes */
 att[3] = 0x00;               
 count = 3;
   /* check if it requires an operation specifier */ 
 if ((buffer[buffer_index-1]>=0x20) && (buffer[buffer_index-1]<0x40))
   /* check if the next char is a valid operation specifier */
    if ((buffer[buffer_index]==0x60) || 
        (buffer[buffer_index]==0x61) || 
        (buffer[buffer_index]==0x62))
   {
    att[3] = buffer[buffer_index]; 
    buffer_index++;    /* consume the op. specifier from the input stream */
    count=4;
   } 
/* set our current screen attribute */
if ((count==3) || (att[3]==0x60))  /*replace */
       SCR->_cur_atrib=att[2];
else if (att[3]==0x61)           /*OR  */
     {
       att[2]|=0x40;   /* set bit 7 */
       att[2]&=0xdf;   /* reset bit 6 */
       SCR->_cur_atrib|=att[2];
     }
else if (att[3]==0x62)           /*AND */
     {
       att[2]|=0x40;   /* set bit 7 */
       att[2]&=0xdf;   /* reset bit 6 */
       SCR->_cur_atrib&=att[2];
     }

}
/*--------- reset one specific function key to default AID value ----------*/
void do_3151_reset_function_key()
{
  int i;
 STATE=0;  /* recognized char, so reset state */
  if (buffer[buffer_index]>=0x40)   /* fnx not used */
    {
      i=buffer[buffer_index]&0x1f;
      buffer_index++;
    }
  else                             /* fnx used */
    {
      i=(buffer[buffer_index+1]&0x1f);
      i=i+31;
      buffer_index+=2;
    }
 if ((i>=1) && (i<=12))
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x61+i;   /* a..l */
     function_keys[i][2]=0x0d;     /* ^M */
     function_keys[i][3]=0x00;
   }
 else if ((i>=13) && (i<=24))
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x21;     /* ! */
     function_keys[i][2]=0x61+i-12;   /* a..l */
     function_keys[i][3]=0x0d;     /* ^M */
     function_keys[i][4]=0x00;
   }
 else if ((i>=25) && (i<=36))
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x22;     /* " */
     function_keys[i][2]=0x61+i-24;   /* a..l */
     function_keys[i][3]=0x0d;     /* ^M */
     function_keys[i][4]=0x00;
   }
}
/*----------- reset all function keys to default AID value ----------*/
void do_3151_reset_function_keys()
{
 int i;

 STATE=0;  /* recognized char, so reset state */
 for (i=0;i<12;i++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x61+i;   /* a..l */
     function_keys[i][2]=0x0d;     /* ^M */
     function_keys[i][3]=0x00;
   }
 for (i=12;i<24;i++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x21;     /* ! */
     function_keys[i][2]=0x61+i-12;   /* a..l */
     function_keys[i][3]=0x0d;     /* ^M */
     function_keys[i][4]=0x00;
   }
 for (i=24;i<36;i++)
   {
     function_keys[i][0]=0x1b;     /* escape */
     function_keys[i][1]=0x22;     /* " */
     function_keys[i][2]=0x61+i-24;   /* a..l */
     function_keys[i][3]=0x0d;     /* ^M */
     function_keys[i][4]=0x00;
   }
}
/*---- reaplce default AID value with a customized string ------------------*/
void do_3151_set_function_key()
{
  char setting[71];
  int  i,j,k;
  int key_num;

  STATE=0;  /* recognized char, so reset state */
  setting[0]=buffer[buffer_index-3];   /* ESC */
  setting[1]=buffer[buffer_index-2];   /* ! */
  setting[2]=buffer[buffer_index-1];     /* = */
  /* scan the input buffer until you find
     the end of the setting which is ESC = */
   i=3; j=buffer_index;
   for (;;)
   {
    setting[i]=buffer[j];
    if ((setting[i-1]==0x1b) && (setting[i]==0x3d))  break;
    i++; j++;
   }
   buffer_index=j+1;
/*   array_to_hft(setting,i+1); do not send to screen, we will simulate
                                this mode with function check_function_key  
                                in ttyio.c */
 /* now we need to store the new string in our buffer */
 j=5;k=0;
 if (setting[3]==0x20)    /* fn=0 so we need fnx */
    {
      key_num=setting[4]-0x21;
      j++;
    }
 else key_num=setting[3]-0x21;
 while(j<i-1) 
   {
    function_keys[key_num][k]=setting[j];
    k++;j++;
   } 
 function_keys[key_num][k]=0x00;
}
/*------------ change default screen setup  -----------------------*/
void do_3151_set_control1()
{
 char setting[6];
 int num,i;
 STATE=0;  /* recognized char, so reset state */
 setting[0]=buffer[buffer_index-3];   /* ESC */
 setting[1]=buffer[buffer_index-2];   /* space */
 setting[2]=buffer[buffer_index-1];   /* 9 */
 setting[3]=buffer[buffer_index++];     /* pa1 */
 num = 4;
 /* now check if there is another parameter as well,
    this is true if bit 7 = 0 and bit 6 = 1.
    We check that by resetting bits 8,5,4,3,2,1 and
    then checking that the value is hex 0x20         */
 if ((setting[3]&0x60)==0x20)
  {
    setting[4]=buffer[buffer_index];     /* pa2 or op */
    buffer_index++;
    num++;
    /* now check if there is another parameter as well.*/
    if ((setting[4]&0x60)==0x20)
     {
       setting[5]=buffer[buffer_index];     /* op */
       buffer_index++;
       num++;
     }
  }  
 array_to_hft(setting,num); 
}
/*------------ change default screen setup  -----------------------*/
void do_3151_set_control2()
{
 char setting[6];
 int num,i;
 STATE=0;  /* recognized char, so reset state */
 setting[0]=buffer[buffer_index-3];   /* ESC */
 setting[1]=buffer[buffer_index-2];   /* ! */
 setting[2]=buffer[buffer_index-1];   /* 9 */
 setting[3]=buffer[buffer_index++];     /* pa1 */
 num = 4;
 /* now check if there is another parameter as well,
    this is true if bit 7 = 0 and bit 6 = 1.
    We check that by resetting bits 8,5,4,3,2,1 and
    then checking that the value is hex 0x20         */
 if ((setting[3]&0x60)==0x20)
  {
    setting[4]=buffer[buffer_index];     /* pa2 or op */
    buffer_index++;
    num++;
    /* now check if there is another parameter as well.*/
    if ((setting[4]&0x60)==0x20)
     {
       setting[5]=buffer[buffer_index];     /* op */
       buffer_index++;
       num++;
     }
  }  
 array_to_hft(setting,num); 
}
/*------------ change default screen setup  -----------------------*/
void do_3151_set_control3()
{
 char setting[6];
 int num,i;
 STATE=0;  /* recognized char, so reset state */
 setting[0]=buffer[buffer_index-3];   /* ESC */
 setting[1]=buffer[buffer_index-2];   /* " */
 setting[2]=buffer[buffer_index-1];   /* 9 */
 setting[3]=buffer[buffer_index++];     /* pa1 */
 num = 4;
 /* now check if there is another parameter as well,
    this is true if bit 7 = 0 and bit 6 = 1.
    We check that by resetting bits 8,5,4,3,2,1 and
    then checking that the value is hex 0x20         */
 if ((setting[3]&0x60)==0x20)
  {
    setting[4]=buffer[buffer_index];     /* pa2 or op */
    buffer_index++;
    num++;
    /* now check if there is another parameter as well.*/
    if ((setting[4]&0x60)==0x20)
     {
       setting[5]=buffer[buffer_index];     /* op */
       buffer_index++;
       num++;
     }
  }  
 /* ------------     now update our variables ------------------ */

 /* AUTOLF is set by the second bit of pa1 */
  AUTOLF=((setting[3]&0x02)==0x02);

 /* WRAP is set by the third bit of pa1 */
  WRAP = ((setting[3]&0x04)==0x04);

 /* we need to check if there is pa2 or not */
 if (num>4) 
     /* if bits 6 and 7 are ones it is op, otherwise it is pa2 */
   if ((setting[4]&0x60)!=0x60)

   /* SCROLL is set by the fourth and fifth bits of pa2 */
    if ((setting[4]&0x18)==0x00) SCROLL = FALSE;
    else if (((setting[4]&0x18)==0x08) ||((setting[4]&0x18)==0x10)) 
           {
             SCROLL = TRUE;
             /* we don't want the screen to be really set to scrolling, 
                because this messes up our screen refreshing. So we turn
                it off anyway, and simulate it when necessary. */
            setting[4]&=0xe7;  /* reset bits 4 and 5 */
           }
 array_to_hft(setting,num); 
}
/*------------ change default screen setup  -----------------------*/
void do_3151_set_control4()
{
 char setting[5];
 int num,i;
 STATE=0;  /* recognized char, so reset state */
 setting[0]=buffer[buffer_index-3];   /* ESC */
 setting[1]=buffer[buffer_index-2];   /* # */
 setting[2]=buffer[buffer_index-1];   /* 9 */
 setting[3]=buffer[buffer_index++];     /* pa1 */
 num = 4;
 /* now check if there is another parameter as well.*/
 if ((setting[3]&0x60)==0x20)
     {
       setting[4]=buffer[buffer_index];     /* op */
       buffer_index++;
       num++;
     }
 array_to_hft(setting,num); 
}
/*------------ change default screen setup  -----------------------*/
void do_3151_set_control5()
{
 char setting[8];
 int num,i;
 STATE=0;  /* recognized char, so reset state */
 setting[0]=buffer[buffer_index-3];   /* ESC */
 setting[1]=buffer[buffer_index-2];   /* $ */
 setting[2]=buffer[buffer_index-1];   /* 9 */
 setting[3]=buffer[buffer_index++];     /* pa1 */
 num = 4;
 /* now check if there is another parameter as well,
    this is true if bit 7 = 0 and bit 6 = 1.
    We check that by resetting bits 8,5,4,3,2,1 and
    then checking that the value is hex 0x20         */
 if ((setting[3]&0x60)==0x20)
  {
    setting[4]=buffer[buffer_index];     /* pa2 or op */
    buffer_index++;
    num++;
    /* now check if there is another parameter as well.*/
    if ((setting[4]&0x60)==0x20)
     {
       setting[5]=buffer[buffer_index];     /* pa3 or op */
       buffer_index++;
       num++;
       /* now check if there is another parameter as well.*/
       if ((setting[5]&0x60)==0x20)
        {
          setting[6]=buffer[buffer_index];     /* pa4 or op */
          buffer_index++;
          num++;
          /* now check if there is another parameter as well.*/
          if ((setting[6]&0x60)==0x20)
           {
             setting[7]=buffer[buffer_index];     /* op */
             buffer_index++;
             num++;
           }
        }
     }
  }  
 array_to_hft(setting,num); 
}
/*------------ change default screen setup  -----------------------*/
void do_3151_set_control6()
{
 char setting[8];
 int num,i;
 STATE=0;  /* recognized char, so reset state */
 setting[0]=buffer[buffer_index-3];   /* ESC */
 setting[1]=buffer[buffer_index-2];   /* % */
 setting[2]=buffer[buffer_index-1];   /* 9 */
 setting[3]=buffer[buffer_index++];     /* pa1 */
 num = 4;
 /* now check if there is another parameter as well,
    this is true if bit 7 = 0 and bit 6 = 1.
    We check that by resetting bits 8,5,4,3,2,1 and
    then checking that the value is hex 0x20         */
 if ((setting[3]&0x60)==0x20)
  {
    setting[4]=buffer[buffer_index];     /* pa2 or op */
    buffer_index++;
    num++;
    /* now check if there is another parameter as well.*/
    if ((setting[4]&0x60)==0x20)
     {
       setting[5]=buffer[buffer_index];     /* pa3 or op */
       buffer_index++;
       num++;
       /* now check if there is another parameter as well.*/
       if ((setting[5]&0x60)==0x20)
        {
          setting[6]=buffer[buffer_index];     /* pa4 or op */
          buffer_index++;
          num++;
          /* now check if there is another parameter as well.*/
          if ((setting[6]&0x60)==0x20)
           {
             setting[7]=buffer[buffer_index];     /* op */
             buffer_index++;
             num++;
           }
        }
     }
  }  
 array_to_hft(setting,num); 
}
/*------------ change default screen setup  -----------------------*/
void do_3151_set_control7()
{
 char setting[7];
 int num,i;
 STATE=0;  /* recognized char, so reset state */
 setting[0]=buffer[buffer_index-3];   /* ESC */
 setting[1]=buffer[buffer_index-2];   /* & */
 setting[2]=buffer[buffer_index-1];   /* 9 */
 setting[3]=buffer[buffer_index++];     /* pa1 */
 num = 4;
 /* now check if there is another parameter as well,
    this is true if bit 7 = 0 and bit 6 = 1.
    We check that by resetting bits 8,5,4,3,2,1 and
    then checking that the value is hex 0x20         */
 if ((setting[3]&0x60)==0x20)
  {
    setting[4]=buffer[buffer_index];     /* pa2 or op */
    buffer_index++;
    num++;
    /* now check if there is another parameter as well.*/
    if ((setting[4]&0x60)==0x20)
     {
       setting[5]=buffer[buffer_index];     /* pa3 or op */
       buffer_index++;
       num++;
       /* now check if there is another parameter as well.*/
       if ((setting[5]&0x60)==0x20)
        {
          setting[6]=buffer[buffer_index];     /* op */
          buffer_index++;
          num++;
        }
     }
  }  
 array_to_hft(setting,num); 
}
