static char sccsid[] = "@(#)95  1.4  src/bldenv/pkgtools/findfile.c, pkgtools, bos412, GOLDA411a 5/23/94 16:51:21";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: do_cksum
 *		findfile
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/stat.h>

void do_cksum(char *, char *);

int 
findfile(char	*inname, 
	 char	*paths[], 
	 int	num_paths, 
	 char	*foundname,
	 struct	stat *f, 
	 char	*cksum)
{
    int  	k;
    char 	*temp_path;
    char	*slash = "" ;
    int		tmp_len	;

    /* check current directory first to see if file exists */
    /* if inname begins with a slash we must remove slash */

    if (inname[0] == '/') 
    {
	temp_path = &inname[1] ;
    }
    else 
    {
	temp_path = inname ;
	slash = "/" ;
    }

    if (stat(temp_path, f) != -1) 
    {
	strcpy(foundname, temp_path);
	do_cksum(foundname, cksum);
	return(0);
    }

  /* now search all the paths to find the file */

    tmp_len = strlen(inname) + strlen(slash) + 1 ;
    for (k = 0; k < num_paths; k++) 
    {
	temp_path = malloc(strlen(paths[k]) + tmp_len) ;
	sprintf(temp_path, "%s%s%s", paths[k], slash, inname) ;

	if (stat(temp_path, f) != -1) 
	{
	    strcpy(foundname, temp_path);
	    do_cksum(foundname, cksum);
	    free(temp_path);
	    return(0);
	}
	free(temp_path);
  }
  return(-1);
}


void do_cksum(char *fname, char *cksum)
{
  char sumCommand[] = "/bin/sum -r ";
  char cmd[512], tmp[512];
  int fp;

  sprintf(cmd, "%s < %s 2> /dev/null", sumCommand, fname);

  fp = popen(cmd, "r");
  fgets(tmp, sizeof(tmp), fp);
  pclose(fp);
  
  if (tmp[strlen(tmp) - 1] == '\n')
    tmp[strlen(tmp) - 1] = '\0';
  
  sprintf(cksum, "\"%s\"", tmp);
}
