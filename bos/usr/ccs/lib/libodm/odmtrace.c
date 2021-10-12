static char sccsid[] = "@(#)94  1.4  src/bos/usr/ccs/lib/libodm/odmtrace.c, libodm, bos411, 9428A410j 4/18/91 21:24:14";

/*
 * COMPONENT_NAME: (LIBODM) Object Data Manager library
 *
 * FUNCTIONS:  print_odm_trace
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*-----------------------------------------------*/
/* Only include the trace if DEBUG is turned on. */
/*-----------------------------------------------*/

#include <stdio.h>
#include <sys/stat.h>

int trace_indent = 0;

char fill_buffer[] = "                                                \
                                         ";

/*
 * NAME: print_odm_trace
 *
 * FUNCTION:  Prints debug trace informatino to a file
 *
 * RETURNS:   Nothing
 */
int print_odm_trace(routine,format1,addr1,format2,addr2)
char *routine;   /* Name of the calling routine */
char *format1;   /* Format for first value */
char *format2;   /* Format for second value */
long addr1;      /* (pointer to) first value */
long addr2;      /* (pointer to) second value */
{
    char tracename[256];
    int padding[80];


    static FILE *trcfp = (FILE *) NULL;
    char outbuf[256];
    int fileindex;


    if(trcfp == (FILE *) NULL)
      {
        struct stat buf;
        for (fileindex = 0; fileindex<1000 ;fileindex++ )
          {
            if (fileindex == 0)
              {
                strcpy(tracename,"./ODMTRACE");

              }
            else
              {
                sprintf(tracename,"./ODMTRACE%d",fileindex);
              } /* endif */



            if (  stat(tracename,&buf) != 0)
              {
                trcfp = fopen(tracename,"w");
                break;
              }
          } /* endfor */
        if (trcfp == (FILE *) NULL)
          {
            printf("DELETE SOME TRACE FILES. Terminating process\n");
            exit(10);
          } /* endif */
      }

    /*-----------------------------------*/
    /* if(trcfp == (FILE *) NULL)        */
    /*     trcfp = fopen("./TRACE","w"); */
    /*-----------------------------------*/
    if (trace_indent > 75)
      {
        trace_indent = 75;
      } /* endif */

    if (trace_indent < 0)
      {
        trace_indent = 0;
      } /* endif */

    padding[0] = '\0';
    strncat(padding,fill_buffer,2*trace_indent);

    if(format2[0] != '\0')
      {
        sprintf(outbuf,"%s%s: %s, %s.\n",padding,routine,format1,format2);
        fprintf(trcfp,outbuf,addr1,addr2);
      }
    else
      {
        sprintf(outbuf,"%s%s: %s.\n",padding,routine,format1);
        fprintf(trcfp,outbuf,addr1);
      }

    fflush(trcfp);

} /* odmtrace */

