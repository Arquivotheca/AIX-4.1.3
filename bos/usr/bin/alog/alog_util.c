static char sccsid[] = "@(#)43        1.5  src/bos/usr/bin/alog/alog_util.c, cmdalog, bos411, 9428A410j 10/18/93 17:30:15";
/*
 *   COMPONENT_NAME: CMDALOG
 *
 *   FUNCTIONS: output_log
 *		syntax
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
/* alog_util.c - This file contains support routines for the alog routine  */

#include	"alog.h"

/*
 *-------------------------------------------------------
 * output_log() - Routine to output file in correct order
 *-------------------------------------------------------
 */

output_log(log_file_name)
char	*log_file_name;
{
int	i,j;
char    c;
struct bl_head lp;
FILE	*fin;

if((fin = fopen(log_file_name,"r")) != NULL)
   {
   fread(&lp,sizeof(struct bl_head),1,fin);
   if (lp.magic != ALOG_MAGIC)
      {  /* the header is not correct */
         fprintf(stderr,MSGSTR(NON_ALOG_FILE,"alog: %s is not \
an alog file.\n"), log_file_name);
         exit(2); /* not an alog file */
      }
   fseek(fin,lp.current,0);	/* Goto starting point in file    */
   }
else				
   { /* Could not open log file */
   fprintf(stderr,MSGSTR(FILE_NOT_OPEN,"alog: Could not open \
file, %s.\n\
Possible cause(s):\n\t\
- The file does not exist.\n\t\
- The user does not have the proper authorities for the file.\n"), \
log_file_name);
   exit(2);
   }

/* Output from current to bottom */
/* If current and bottom are the same, don't print anything. */
if (lp.current != lp.bottom)
	for(i=lp.current;i<lp.bottom;i++)
   		putchar(fgetc(fin));

/* Output from top to current	  */
fseek(fin,lp.top,0);
for(i=lp.top;i<lp.current;i++)
   putchar(fgetc(fin));

fclose(fin);

}  /* end output_log */

/*
 *-------------------------------------------------------
 * syntax() - Display the syntax message
 *-------------------------------------------------------
 */

syntax()
{
fprintf(stderr,MSGSTR(USAGE,"Usage:\n\t\
alog -f File [-o] | [ [-s Size] [-q] ]\n\t\
alog -t LogType [-f File] [-o] | [ [-s Size] [-q] ]\n\t\
alog -t LogType -V\n\t\
alog -C -t LogType { [-f File] [-s Size] [-w Verbosity] }\n\t\
alog -L [-t LogType]\n\t\
alog -H\n"));
exit(1);
}  /* end syntax */
