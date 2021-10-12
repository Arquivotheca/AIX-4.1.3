static char sccsid[] = "@(#)22  1.3  src/bos/usr/bin/bprt/filter.c, libbidi, bos411, 9428A410j 11/10/93 15:28:55";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: Flush
 *              OutC
 *              Prepare
 *              main
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

/* This is a filter that processes a bidi ascii (Arabic/Hebrew) */
/* file ready for printing through the spooler.			*/

#include <stdio.h>
#include <langinfo.h>
#include <sys/lc_layout.h>
#include "prt-proc.h" 
#include "codeset.h"
#include "debug.h"

extern int optind;
extern char * optarg;

char 	* BidiAttr;       	/* for keeping the bidi attributes (as a */
				/* string)		 		 */
char 	DefBidiAttr=0;		/* indicates default bidi attributes     */
char	Buff [BUFSIZ];		/* buffer into which data is accumulated */
				/* to be processed	   		 */ 
char 	* NextP = Buff;		/* where to put next char in Buff        */
char	* EndBuff = Buff+BUFSIZ;
				/* pointer to character after the end of */
				/* Buff					 */
char	* from;			/* to keep the name of sender      	 */
char	* prt_type;		/* printer type eg. 4207-2 		 */
int     statusfile = 1;         /* we're running under the spooler 	 */
char 	PsFlag;                 /* post script flag 		   	 */
int 	PrtLength;		/* printer width 		   	 */
char 	BidiSkip = 0;     	/* to skip bidi processing all together  */
char 	Hebrew = 0, 
	Arabic = 0;
int	i = 0;

/* Maya  4/8/1993 TASHKEEL */
char    TASHKEEL=0; 
/* Maya  4/8/1993 TASHKEEL */


/******** global variables used by the BidiAp call *******/
LayoutObject    plh;  /* Maya 12/5/1993 */
char            locale_name[20]; 

int 		CodeSet;
unsigned short 	counter;
EntryPoint 	G;
unsigned long 	character;
unsigned long 	Char_Attr;
unsigned long 	PRT_PSM_ATTR;
unsigned long 	CURR_CHAR_ATTR;
unsigned char 	bidi_out_buff [4*BUFSIZ];
unsigned short 	bidi_out_buff_len=0; 

char TempStr[200];

/************ for debugging purposes *********/
unsigned char debug = 1;

int Prepare ();
void OutC ();
void Flush ();

main (int argc, char * argv[], char **env)
{    
int	val1;
int	RC;

FILE * in;

  setlocale(LC_ALL, "");
  strcpy(locale_name, setlocale(LC_CTYPE, NULL));
 
  if (RC = layout_object_create(locale_name, &plh))
  { 
/*    TRACE("/aix3.2/maya/out", "Cannot Load libi18n correctly and exit!!!\n", NULL);*/
    exit(0);                        /* cannot initialize libi18n */
  }

  if (Prepare (argc, argv))   /* if Prepare() is successful, continue */
  { 
    while ((val1 = getc(stdin)) != EOF)
      OutC (val1);
    Flush ();    /* flush whatever is left in the buffer */
  }
  layout_object_free(plh);   /* maya 12/5/1993 */
}    


/*****************************************************************************/
/* prepares the environment for bidi processing :                            */
/* gets the arguments, checks the env. etc 				     */
/*****************************************************************************/
int Prepare (int argc, char * argv [])
{
LayoutValueRec Layout[2];
int RC, index_error=0;


int i, ch, lang_len;
char * temp_str;
char * log_name;
int  w_flag = 0, d_flag = 0;

  /* get arguments sent with pipe */
  while ((ch = getopt (argc, argv, "w:d:debug:tashkeel")) != EOF)
  {
    if (strcmp (argv [optind], "-debug") == 1)
       debug = 1;
    if (strcmp (argv [optind], "-tashkeel") == 0)
       TASHKEEL = 1;
    switch (ch)
    {
	 case 'w' :
			PrtLength = atoi (optarg);
			w_flag = 1;
	    	        break;

	 case 'd' :
			if (*optarg == 'p')
			  PsFlag = 1;
			else
			  PsFlag = 0;
			d_flag = 1;
	 	        break;
    } /* switch */
  } /* while */

  if (!w_flag || !d_flag)
      return 0;

  prt_type = getenv ("PIOPTRTYPE");

  Layout[0].name  = ActiveBidirection;
  Layout[1].name  = 0;		      /* mark the end of the Layout array */	
  
  RC = layout_object_getvalue(plh, Layout, &index_error);  
  BidiSkip = !(*(Layout[0].value));
  if (RC)
    return 0;       
 
  if (!BidiSkip)  
  {
    /* Get the bidi attributes, if they exist, in PRTBIDIATTR. */
    BidiAttr = getenv ("PRTBIDIATTR");                     
    if (BidiAttr == NULL)                                  
        DefBidiAttr = 1;                                
    CodeSet = IsCurrentCodePage ();                         
  }

  switch (CodeSet)
  { 
    case BDCP_IBM856 :
    case BDCP_IBM862 :
    case BDCP_ISO8859_8 :
  		 	 Hebrew = 1;
 			 break;
    case BDCP_IBM1046 :
    case BDCP_ISO8859_6 :
  			 Arabic = 1;
                         break;
  }

  /* set default attributes :				  	       */
  /*    defaults for the Hebrew support :                              */
  /*       Visual + LTR + swapping off + Arabic numerics + no shaping  */
  /*       (i.e. TEXT_NOMINAL)					       */ 
  /*   defaults for Arabic support : 			               */
  /*       Implicit + LTR + swapping on + upon context numerics +      */
  /*       auto shaping + host mode (i.e. TEXT_SPECIAL)                */
  if (DefBidiAttr)                                              
  {                                                                 
    BidiAttr = malloc (9);                                     
    if (Hebrew)                                                 
      sprintf (BidiAttr, "00000010");                          
    else
      if (Arabic)                                                 
         sprintf (BidiAttr, "01003140");  /* maya 15/3/1993: add host mode as a default */ 
  }                                                                 

  /* initiate the bidi support processing environment */
  if (!PsFlag && !BidiSkip)                                          
      BidiAp (prt_type, BidiAttr, NULL, 0, (unsigned char) PrtLength, 0, 0);    
  
  if (DefBidiAttr)
      free (BidiAttr);                                                 

  return 1;
}

/*****************************************************************************/
/* Puts a character in acumulating buffer and flushes it in case it is full. */
/*****************************************************************************/
void OutC(ch)      
{  
  *NextP++ = (unsigned char) ch;
  if (NextP >= EndBuff)
    (void) Flush ();
}

/*****************************************************************************/
/* Bidi processes the buffer and sends it out.				     */
/*****************************************************************************/
void Flush ()
{
unsigned long i, in_buff_len;


  in_buff_len = NextP-Buff;
  if (Arabic || Hebrew)
    {
	BidiAp (prt_type, NULL, Buff, in_buff_len, CP1046, 0, 1);

  	for (i=0; i<bidi_out_buff_len; i++) 
   	   fputc (bidi_out_buff [i], stdout);

	memset (bidi_out_buff, 0, BUFSIZ*4);
	bidi_out_buff_len = 0;
    }
  else
  	for (i=0; i<in_buff_len; i++) 
   	   fputc (Buff[i], stdout);

  NextP = Buff;
}
