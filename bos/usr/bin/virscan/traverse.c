static char sccsid[] = "@(#)28	1.4  src/bos/usr/bin/virscan/traverse.c, cmdsvir, bos411, 9428A410j 4/11/91 18:44:57";
/*
 *   COMPONENT_NAME: CMDSVIR
 *
 *   FUNCTIONS: traverse
 *		findfirst
 *		findnext
 *		get_pathname
 *		is_valid_directory
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1990,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <sys/dir.h>
#include <fnmatch.h>
#include <ctype.h>

#include "vsdefs.h"

#ifdef MSG			/* AIX message catalogs		*/
#include "virscan_msg.h"
#include <nl_types.h>
nl_catd	catd;

#define MSGSTR(Num, Str) 	catgets(catd,MS_VIRSCAN, Num, Str)
#else				
#define MSGSTR(C,D)		D 
#endif

#include "default.h"			/* Default message definitions	     */
					/* these match the message strings   */	
					/* returned from a message catalog   */	

#define FIND_ERROR 	1		/* Directory search error            */
#define FIND_EOF	2		/* End of directory file search	     */


/* 
 * This is the struct that will be returned from findfirst() and
 * findnext() and will contain the filename and filetype
 */
typedef struct filebuf
{
	char	name[MAXPATHLEN+1];
	int	attr;
} FILEBUF;


/*
 * A list of subdirectories below the directory specified by the
 * 'pathname' parameter of the 'traverse()' routine is built 
 * using these structures. 
 */
typedef struct subdl
{
   char *name;
   struct subdl *next;
} SUBDL;

/*
 * Function Prototypes
 */
DIR  *opendir(char *);
char *strcat(char *, char *);
char *strcpy(char *, char *);
int  findfirst(DIR **, char *, char *, int, FILEBUF *);
int  findnext(DIR *, char *, char *, int, FILEBUF *);

/*
 * FUNCTION:    traverse()
 *
 * DESCRIPTION: Recursively traverse the directory specified by the 
 *		'pathname' parameter.  All files (not subdirectories) 
 *		matching 'filespec' have parameter 'action' applied to them.
 *	 	Subdirectories have 'traverse()' recursively applied to
 * 		them. Errors are handled by calling parameter 
 *		'error_action()', or by an error routine that eventually 
 *		calls 'error_action()'.  'filespec' can actually be a 
 *		series of file specifications, separated with comma (',') 
 *		characters.
 *
 * PASSED:
 *
 * RETURNS:
 *
 */
void
traverse(char *pathname,
         char *filespec,
         void (*action)(char *),
         void (*error_action)(int))
{
	int	rc;
	char	workbuf[MAXPATHLEN+1];
	char	*workfilespec;
	char	*file_element;
	FILEBUF	*resultbuf;
	DIR	*dirp;
	SUBDL	*sdlist = NULL;
	SUBDL	*dlptr	= NULL;
	

	if ((resultbuf = (FILEBUF *)malloc(sizeof(FILEBUF))) == NULL)
	{
		perror("traverse()");
		(*error_action)(0);
	}

	if ((rc = findfirst
	    (&dirp, pathname, SCAN_ALL, S_IFDIR, resultbuf)) == FIND_ERROR)
		(*error_action)(0);

	if (rc != FIND_EOF)
	{
     	   if ((resultbuf->attr & S_IFDIR) && (resultbuf->name[0] != '.'))
       	   {
         	if ((dlptr = (SUBDL *)malloc(sizeof(SUBDL))) == NULL)
         	{ 
		   perror("traverse()");
	    	   (*error_action)(0);
		}
		if ((dlptr->name = (char *)strdup(resultbuf->name)) == NULL)
                {
                	perror("traverse()");
                       	(*error_action)(0);
                }
         	dlptr->next = sdlist;
         	sdlist = dlptr;
      	   }

	   for(;;)
      	   {
         	if ((rc = findnext(dirp, pathname, (char *)NULL, 
		    S_IFDIR, resultbuf)) == FIND_ERROR)
		   	(*error_action)(0);

		if (rc == FIND_EOF) break;  

         	if ((resultbuf->attr & S_IFDIR) && (resultbuf->name[0] != '.'))
        	{
         	   if ((dlptr = (SUBDL *)malloc(sizeof(SUBDL))) == NULL)
         	   { 
			perror("traverse()");
			(*error_action)(0);
		   }
		   if ((dlptr->name = (char *)strdup(resultbuf->name)) == NULL)
		   {
			perror("traverse()");
			(*error_action)(0);
		   }
       		   dlptr->next = sdlist;
         	   sdlist = dlptr;
         	}
     	   }		
	}
	closedir(dirp);
	free(dirp);

	/* Look for files matching each element of filespec.  If found, 
	 * apply action() to them.  Elements of filespec are separated 
	 * by ',' characters.
 	 */
	if ((workfilespec = (char *)strdup(filespec)) == NULL)
   	{
		perror("traverse()");
		(*error_action)(0);
   	}

	/*
	 * Build working file specification.
 	*/
   	for (file_element =  (char *)strtok(workfilespec,","); 
	     file_element != (char *)NULL;
             file_element =  (char *)strtok(NULL,","))
   	{
	   if ((rc = findfirst
               (&dirp, pathname, file_element, S_IFREG, resultbuf))==FIND_ERROR)
		(*error_action)(0);
	
	   if (rc != FIND_EOF)
	   {
         	strcat(strcpy(workbuf,pathname),resultbuf->name);
         	(*action)(workbuf);

		for(;;)
         	{
         	   if ((rc = findnext(dirp, pathname, (char *)NULL, 
		       S_IFREG, resultbuf)) == FIND_ERROR)
		   	(*error_action)(0);

            	   if (rc == FIND_EOF) break;
				
            	   strcat(strcpy(workbuf,pathname),resultbuf->name);
            	   (*action)(workbuf);
         	}
      	   }
	   closedir(dirp);
	   free(dirp);
	}

	free(workfilespec);
	free(resultbuf);

	dlptr=sdlist;
   	while (dlptr != NULL)
   	{
      		strcat(strcat(strcpy(workbuf,pathname),dlptr->name), "/");
      		traverse(workbuf,filespec,action,error_action);
      		sdlist = dlptr;          /* Get next element, free element */
      		dlptr = dlptr->next;
      		free(sdlist->name);
      		free(sdlist);
   	}
}


/*
 * FUNCTION:    findfirst ()
 *
 * DESCRIPTION: Gets the first file or subdirectory that exists in this
 *              directory that matches the filename parameter.  
 *
 * PASSED:   
 *
 * RETURNS:     
 *
 */
int findfirst (	DIR **dir,
		char *path, 
		char *filespec,
               	int type,
               	FILEBUF *resultbuf)
{
    char directory[MAXPATHLEN+1];

    /*
     * Malloc the structure that will contain the filenames from 
     * the directory file "."
     */
    if ((*dir = (DIR *) malloc (sizeof (DIR))) == NULL)
	perror("findfirst()");

    /* 
     * Open the directory special file that contains the file or
     * subdirectory names
     */
    if ((*dir = opendir(strcat(strcpy(directory, path), "/."))) == NULL)
    {
       fprintf (stderr, MSGSTR(CANT_OPENDIR, MSG_CANT_OPENDIR), directory);
       return (FIND_ERROR);
    }
    
    return (findnext (*dir, path, filespec, type, resultbuf));
}

/*
 * FUNCTION:    findnext ()
 *
 * DESCRIPTION: Finds the next instance in the directory
 *
 * PASSED:
 *
 * RETURNS:
 *
 */
int findnext (DIR  *dir,
              char *path,
              char *file,
	      int  type,
              FILEBUF *resultbuf)
{
    static char filename[MAXNAMLEN+1];
    static char pathname[MAXPATHLEN+1];
    static int  filetype;
    char path_and_file[MAXPATHLEN+1];
    struct dirent *dp;
    struct stat statbuf;
    int execute;
  
    /*
     * The filename specification is going to stay static here between
     * calls.  The only time it will change is when DosFindFirst is
     * calling us.  That call contains a new file specification and 
     * should be copied into filename.  Otherwise we'll be called with
     * a null file specifier and I shouldn't change it.
     */
    if (file != (char *)NULL)
       strcpy (filename, file); 
    if (path != (char *)NULL)
       strcpy(pathname, path);
    if (type != 0)
       filetype = type;

    for (dp = readdir (dir); dp != NULL; dp = readdir (dir)) 
    {
       if ((dp->d_name[0] == '.' && dp->d_name[1] == '\0') ||
           (dp->d_name[0] == '.' && dp->d_name[1] == '.' &&
            dp->d_name[2] == '\0'))
          continue;
       
       if (lstat (strcat(strcpy(path_and_file, pathname), dp->d_name),
           &statbuf) < 0)
          fprintf (stderr, MSGSTR(CANT_STAT, MSG_CANT_STAT), dp->d_name);
 
       #ifdef DEBUG
       printf("%s\n",dp->d_name);
       #endif

       switch (statbuf.st_mode & S_IFMT)
       {
          case S_IFDIR :
            resultbuf->attr = S_IFDIR;
            break;
   
          case S_IFCHR :
            resultbuf->attr = S_IFCHR;
            break;

          case S_IFMPX :
            resultbuf->attr = S_IFMPX;
	    break;

          case S_IFBLK :
 	    resultbuf->attr = S_IFBLK;
 	    break;

          case S_IFREG :
            resultbuf->attr = S_IFREG;
	    break;
 
          case S_IFIFO :
	    resultbuf->attr = S_IFIFO;
            break;
 
          case S_IFLNK :
            resultbuf->attr = S_IFLNK; 
            break;
    
          case S_IFSOCK :
            resultbuf->attr = S_IFSOCK;
            break;

          default :
            if (S_ISMPX(statbuf.st_mode))
               resultbuf->attr = S_IFMPX;
            else
               return (FIND_ERROR);
       }
       if (statbuf.st_mode & S_IXUSR || statbuf.st_mode & S_IXGRP ||
	   statbuf.st_mode & S_IXOTH)
	   execute = TRUE;
       else
	   execute = FALSE;

       #ifdef DEBUG
       if (execute)
	  printf("File has execute permission\n");
       #endif

       	/*
         * Stay away from the character/raw/block/multiplexed
         * device drivers.
         */
	if (!strcmp(filename, SCAN_ALL)) 	/* Scan all files 	*/ 	
	{
	   if ((filetype == S_IFDIR && resultbuf->attr == S_IFDIR) ||
	       (filetype == S_IFREG && resultbuf->attr == S_IFREG))
	   {
		strcpy(resultbuf->name, dp->d_name);
		return(0);
	   }
	   else
		continue;
	}
	else if (!strcmp(filename, SCAN_EXE))	/* Scan only executables */
	{
	   if ((filetype == S_IFDIR && execute && resultbuf->attr == S_IFDIR) ||
	       (filetype == S_IFREG && execute && resultbuf->attr == S_IFREG))
	   {
               strcpy(resultbuf->name, dp->d_name);
               return(0);
       	   }
           else
               continue;
   	}
       	else if (!fnmatch(filename, dp->d_name, FNM_PERIOD))
       	{
	   if ((filetype == S_IFDIR && resultbuf->attr == S_IFDIR) ||
	       (filetype == S_IFREG && resultbuf->attr == S_IFREG))
	   {
               strcpy(resultbuf->name, dp->d_name);
               return(0);
       	   }
           else
               continue;
       	}
   } /* end for */

   return (FIND_EOF);
}


/*
 * FUNCTION:    is_valid_directory ()
 *
 * DESCRIPTION:
 *
 * PASSED:
 *
 * RETURNS:
 *
 */
int is_valid_directory (char *path)
{
    DIR *dir;
    char directory[MAXNAMLEN+1];

    /*
     * Malloc the structure that will contain the filenames from
     * the directory file "."
     */
    if ((dir = (DIR *) malloc (sizeof (DIR))) == NULL)
	perror("is_valid_directory()");

    /*
     * Open the directory special file that contains the file or
     * subdirectory names
     */
    if ((dir = opendir (strcat(strcpy(directory, path), "/."))) == NULL)
    {
       fprintf (stderr, MSGSTR(CANT_OPENDIR, MSG_CANT_OPENDIR), directory);
       return (0);
    }
}
