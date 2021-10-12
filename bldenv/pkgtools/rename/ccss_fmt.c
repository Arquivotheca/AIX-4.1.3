static  char sccsid[] = "@(#)46  1.3  src/bldenv/pkgtools/rename/ccss_fmt.c, pkgtools, bos412, GOLDA411a 6/24/93 08:36:29";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: copy_component
 *		main
 *		usage
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*------------------------------------------------------------------------*/
/*                                                                        */
/*  Purpose:                                                              */
/*      This program reblocks a ccss format envelop for                   */
/*      transmission to Boulder or Copenhagen.                            */
/*      This program will disappear when they can handle ccss             */
/*      datastreams without 370 logical records.                          */
/*                                                                        */
/*  Syntax:                                                               */
/*    ccss_fmt   -i fn1  -o fn2  [-?|-h]                                  */
/*                                                                        */
/*  Flags:  Ref. help message                                             */
/*                                                                        */
/* Change log:                                                            */
/*     05/13/91 - Created.                                                */
/*------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>


/* file var's  */
FILE  *file_in;
FILE  *file_out;
char  *in_fname;
char  *out_fname;

/* header input area       */
char    hdr[81];
char    save_hdr[81];
char    hdr_name[81];
char    hdr_type[4];
int     hdr_size;

/* misc. vars */
int num_chars;
int rec_size;
char *comma_ptr;
char  b512[512];

/* parm var's */
char  flga[30];
char  flgb[30];

main(int argc,char  *argv[])
{
  /*  get parameters */
  if(argc == 2)
    {usage();return(0);}

  if(argc!=5)
    {printf("invalid number of parameters\n");
     usage();return(1);}

  strcpy(flga,argv[1]);
  strcpy(flgb,argv[3]);
  if((flga[0]=='-')&&(flga[1]=='i')){in_fname = argv[2];}
  if((flgb[0]=='-')&&(flgb[1]=='i')){in_fname = argv[4];}
  if((flga[0]=='-')&&(flga[1]=='o')){out_fname = argv[2];}
  if((flgb[0]=='-')&&(flgb[1]=='o')){out_fname = argv[4];}

  /*  open files */
  if((file_in=fopen(in_fname,"rb"))== NULL)
      {printf("Unable to open input file %s\n",in_fname);
       usage();return(2);
      }
  if((file_out=fopen(out_fname,"wb,lrecl=516,recfm=v,type=record"))
      == NULL)
      {printf("Unable to open output file %s\n",out_fname);
       usage();return(2);
      }

  /* read header and copy component file */
  while( (num_chars = fread(hdr , 1 , 80 , file_in)) == 80 )
      copy_component();

  /*  check for bad characters at end of file */
  if(num_chars != 0)
     {printf("Invalid characters found at end of input file\n");
      printf("%s\n",hdr);
      return(4);
     }

  return(0);
}


copy_component()
{
    /* save header for later output */
    hdr[80]='\0';
    strcpy(save_hdr, hdr);

    /*  change commas to blanks so the subsequent sscanf
        recognizes white space to delimit items in the header */
    while( (comma_ptr = strchr(hdr,',')) != NULL )
      {comma_ptr[0] = ' ';}

    /*   parse header  */
    if (sscanf(hdr,";; NAME=%s TYPE=%s SIZE=%d",
            hdr_name,hdr_type,&hdr_size) != 3)   /* must find 3 items */
       {printf("invalid header found in ccss_file:\n");
        printf("%s\n",hdr);
        exit(3);
       }

    /* write saved header to output file */
    fwrite(save_hdr , 1 , 80 , file_out);

    /* determine output record size */
    if(strcmp(hdr_type,"INF") == 0)
        rec_size = 80;               /* record size is 80 for info file */
    else
        rec_size = 512;              /* record size is 512 for all others */

    /*  copy input to output file */
    while(hdr_size >= rec_size)
     {
      fread(b512 , 1 , rec_size , file_in);
      fwrite(b512 , 1 , rec_size , file_out);
      hdr_size = hdr_size - rec_size;
     }

    if(hdr_size != 0)
     {fread(b512 , 1 , hdr_size , file_in);
      fwrite(b512 , 1 , hdr_size , file_out);
     }
  return;
}

usage()
{
 printf("%s\n", sccsid);
 printf("usage:\n");
 printf("      ccss_fmt -i fn.ft.fm -o fn.ft.fm  [-h] [-?]\n");
 printf("         where:\n");
 printf("             -i input filename.filetype.filemode\n");
 printf("             -o output filename.filetype.filemode\n");
 printf("             -h | -? usage message\n");
}
/* END OF PROGRAM */
