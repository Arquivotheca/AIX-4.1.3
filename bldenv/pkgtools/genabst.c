/*
* COMPONENT_NAME: pkgtools
*
* FUNCTIONS: Get the specified information from the input file and put it
*            to output file. 
*
* ORIGINS: 27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with the aggregated modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business machines Corp. 1991
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/
static  char sccsid[] = "@(#)14  1.7  genabst.c, pkgtools, bos320 2/23/93 10:14:12";

#include <stdio.h>
#include <string.h>

#define LNSIZE		128
#define MAXLEN		2048
#define spaceSeparator " "
#define tabSeparator   "\t"
#define newlineSeparator "\n" 

void syntax(char*);

main (int ac, char *av[])
{
    FILE *fpInFile, *fpOutFile;
    char target[LNSIZE];
    char separator[LNSIZE];
    char buffer[MAXLEN];
    int found = 0;
    int success = 0;
    int defaultSeparator = 0;

    if (ac == 5)
    {
       if ((fpInFile = fopen(av[3], "r")) == NULL)
       {
          fprintf (stderr, "%s: ERROR: Failed to open input file %s\n",
                           av[0], av[3]);
          exit (1);
       }
       if ((fpOutFile = fopen(av[4], "a")) == NULL)
       {
          fprintf (stderr, "%s: ERROR: Failed to open output file %s\n",
                           av[0], av[4]);
          exit (4);
       }
    }
    else if (ac == 4)
    {
       defaultSeparator=1;
       if ((fpInFile = fopen(av[2], "r")) == NULL)
       {
          fprintf (stderr, "%s: ERROR: Failed to open input file %s\n",
                           av[0], av[2]);
          exit (1);
       }
       if ((fpOutFile = fopen(av[3], "a")) == NULL)
       {
          fprintf (stderr, "%s: ERROR: Failed to open output file %s\n",
                           av[0], av[3]);
          exit (4);
       }
    }
    else
    {
       fprintf (stderr, "%s: ERROR: This command requires 4 arguments.\n", av[0]);
       syntax (av[0]);
       exit (1);
    }


    memset (target, (char) 0, LNSIZE);
    strcpy (target, av[1]);
    if ((strlen(target)) <= 0)
    {
       fprintf (stderr, "%s: ERROR: The target string is needed and must be the first argument.\n", av[0]);
       syntax (av[0]);
       exit (1);
    }
    if (!defaultSeparator)
    {
       memset (separator, (char) 0, LNSIZE);
       strcpy (separator, av[2]);
       if ((strlen(separator)) <= 0)
       {
          fprintf (stderr, "%s: ERROR: The separator is needed and must be the second argument.\n", av[0]);
          syntax (av[0]);
          exit (1);
       }
    }

    while ((!feof(fpInFile)) && (success == 0)) 
    {
       /* if it is end of file or error occurs, */
       /*   then continue to read next line.    */ 
       if (!(fgets(buffer, MAXLEN, fpInFile)))
          continue;

       if (found == 0)
       {
          if (!(strncmp(buffer, target, strlen(target))))
          {
             fprintf (fpOutFile, "%s", buffer);
             found = 1;
          }
       }
       else
       {
          if (!defaultSeparator)
          {
             if (strncmp(buffer, separator, strlen(separator)))
                fprintf(fpOutFile, "%s", buffer);
             else
             {
                found = 0;
                success = 1;
             }
          }
          else
          {
             if (strncmp(buffer, spaceSeparator, strlen(spaceSeparator)) && 
                 strncmp(buffer, tabSeparator, strlen(tabSeparator)) &&
                 strncmp(buffer, newlineSeparator, strlen(newlineSeparator)))
                 fprintf(fpOutFile, "%s", buffer);
             else
             {
                found = 0;
                success = 1;
             }
          }
       }
    }    
    
    fclose (fpInFile);
    fclose (fpOutFile);

    if (success == 1)
       exit (0);
    else if (found == 0)
       exit (2);
    else if ((found == 1) && (success == 0))
       exit (3);
    
}


void syntax(char *cmd)
{
    fprintf (stderr, "%s: usage: %s <target> [separator]\n", cmd, cmd);
    fprintf (stderr, "%*c            <input file> <output file>\n", strlen(cmd), ' ');
    fprintf (stderr, "%*c You can specified your separator. If the separator is\n", strlen(cmd), ' ');
    fprintf (stderr, "%*c not provided, use the defaust: space, tab or newline.\n", strlen(cmd), ' ');
}
