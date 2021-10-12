static char sccsid[] = "@(#)95  1.2  src/bldenv/findfile/findfile.c, bldprocess, bos412, GOLDA411a 2/3/93 16:34:01";

/*
 *   COMPONENT_NAME: BOSBUILD
 *
 *   FUNCTIONS: main
 *		
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




#include <unistd.h>
#include <sys/access.h>
#include <stdio.h>

main(argc, argv)
int argc;
char **argv;

{
  int c, i, found;
  extern int optind;
  extern char *optarg;
  int lnum = 0;
  int inum = 0;
  char *lpath[40], *ipath[40];
  char *file, *temp_path;
  int k;

  while ((c = getopt(argc, argv, "L:I:")) != EOF) {

	switch(c) {

	case 'L':
		  lpath[lnum++] = optarg;
		  break;

	case 'I':
		  ipath[inum++] = optarg;
		  break;
	}
  } 

  for ( ; optind < argc; optind++) {

    file = argv[optind];
    found = FALSE;

    if (access(file, R_ACC) == 0)
    {
      printf("%s ", file);
      found = TRUE; 
    }

    for (k = 0; ((k < lnum) && (found == FALSE)); k++) {

     temp_path = malloc(strlen(lpath[k]) + strlen(file) + 2);
     strcpy(temp_path, lpath[k]);
     strcat(temp_path, "/");
     strcat(temp_path, file);

     if (access(temp_path, R_ACC) == 0)
     {
       printf("%s ", temp_path);
       found = TRUE; 
     }
     free(temp_path);
    }

    for (k = 0; ((k < inum) && (found == FALSE)); k++) {

     temp_path = malloc(strlen(ipath[k]) + strlen(file) + 2);
     strcpy(temp_path, ipath[k]);
     strcat(temp_path, "/");
     strcat(temp_path, file);

     if (access(temp_path, R_ACC) == 0)
     {
       printf("%s ", temp_path);
       found = TRUE;
     }
     free(temp_path);
    }
  }
exit(0);
}

