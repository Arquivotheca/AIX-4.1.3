static char sccsid[] = "@(#)86  1.26  src/bldenv/pkgtools/adepackage.c, pkgtools, bos412, GOLDA411a 6/26/94 18:42:54";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: adepackage_optsize
 *		adepackage_usage
 *		main
 *		process_links
 *		process_symlinks
 *		set_fileinfo
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

#include "adepackage.h"
#include "ade.h"
#include "list.h"
#include "first.h"
#include "do_tar.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/mode.h>

char          *myname;
int           exit_status;
extern char   *optarg;
extern int    optind;

/* adepackage_optsize - interpret a size argument
 *
 * DESCRIPTION
 *
 *   Recognizes suffixes fo blocks (512-bytes), k-bytes and megabytes.
 *   Also handles simple expressions containing '+' for addition.
 *
 * PARAMETERS
 *
 *   char   *str - A pointer to the string to interpret
 *
 * RETURNS
 *
 *  Normally returns the value represented by the expression in the
 *  string
 *
 * ERRORS
 *
 *  If the string cannot be interpreted, the program will fail, since
 *  the buffering will be incorrect
 *
 */

long 
adepackage_optsize (char *str)
{
  char      *idx;
  long      number;
  long      result;

  result = 0;
  idx = str;
  for (;;) {
    number = 0;
    while (*idx >= '0' && *idx <= '9')
      number = number * 10 + *idx++ - '0';
    switch (*idx++) {
    case 'b':
      result += number * 512L;
      continue;
    case 'k':
      result += number * 1024L;
      continue;
    case 'm':
      result += number * 1024L * 1024L;
      continue;
    case '+':
      result += number;
      continue;
    case '\0':
      result += number;
      break;
    default:
      break;
    }
    break;
  }
  if (*--idx) {
    fatal("Unrecognizable value");
  }
  return(result);
}


/*  adepackage_usage - print a helpful message and exit
 *
 * DESCRIPTION
 *
 *
 * RETURNS
 *
 *  Returns an exit status of 1 to the parent process
 *
 */

void 
adepackage_usage()
{
  fprintf(stderr, "USAGE:\n");
  fprintf(stderr, "\t%s [ -b <blocksize> ] -f <device_name> -s <ship_path>\n",COMMANDNAME);
  fprintf(stderr, "\t[ -t ] -i <inslist_name> -l <lpp_name> [ -F <first_filename> ] \n");
  fprintf(stderr, "\t[ -D ] [ -L ] [ -U <vrmf level> -o <option name> ]\n");
  fprintf(stderr, "\twhere the -b, -t, -L -U, -o and -F flags are optional.\n\n");


  exit(1);
}


/* set_fileinfo - fills in the fileinfo structure with info from
 *                inslist information
 */

void 
set_fileinfo(mode_t	mode, 
	     off_t	size, 
	     Fileinfo	*fileinfo, 
	     InsEntry	*inslist)
{
  fileinfo->f_st.st.st_uid = inslist->uid;
  fileinfo->f_st.st.st_gid = inslist->gid;
  fileinfo->f_st.st.st_size = size;
  fileinfo->f_st.st.st_mode = mode;
}

/* process_links - does the writing of hard links from 
 *                 inslist information
 */

void 
process_links(Fileinfo	*fileinfo, 
	      InsEntry	*inslist, 
	      Tapeinfo	*tapeinfo)
{
  int i;
  struct stat sb;

  if (inslist->numHardLinks > 0 ) {
    fileinfo->f_st.st.st_uid   = inslist->uid;
    fileinfo->f_st.st.st_gid   = inslist->gid;
    strcpy(fileinfo->f_st.linkname, ".");
    strcat(fileinfo->f_st.linkname, inslist->object_name);
    fileinfo->f_st.st.st_mode  = S_IFREG | inslist->mode;
    /*-------------------------------------------------------------------
    | st_nlink should include the source file for the link so increment	|
    | by 1.								|
    -------------------------------------------------------------------*/
    fileinfo->f_st.st.st_nlink = inslist->numHardLinks+1;

    for ( i=0; i<inslist->numHardLinks; i++ ) {
	strcpy(fileinfo->filename, ".");
	strcat(fileinfo->filename, inslist->hardLinks[i]);

   	if (tapeinfo->tapeflag == TAR) {
	   fileinfo->f_st.st.st_size = 0;
	   fileinfo->file_fd = 0;
   	}
   	else {
	   fileinfo->file_fd = open(fileinfo->ship_name, O_RDONLY);
	   stat(fileinfo->ship_name, &sb);
	   fileinfo->f_st.st.st_size = sb.st_size;
   	}
      
   	write_file_to_tape(tapeinfo, fileinfo);
    }
  }
}

/* process_symlinks - does the writing of symlinks from 
 *                 inslist information
 */

void 
process_symlinks(Fileinfo	*fileinfo, 
	      InsEntry	*inslist, 
	      Tapeinfo	*tapeinfo)
{
  int i;
  struct stat sb;

  fileinfo->f_st.st.st_uid   = inslist->uid;
  fileinfo->f_st.st.st_gid   = inslist->gid;
  strcpy(fileinfo->f_st.linkname, inslist->target);
  fileinfo->f_st.st.st_mode  = 0120777;
  fileinfo->f_st.st.st_nlink = 1;
  fileinfo->f_st.st.st_size = strlen(fileinfo->filename);
  write_file_to_tape(tapeinfo, fileinfo);
}


main(int argc, char** argv)

{
  Tapeinfo tapeinfo;
  Fileinfo fileinfo;
  InsEntry inslist;
  FILE     *inslist_fd;
  int      error;
  int      c;
  char     *inslist_name = (char *)NULL;
  char     *ship_path;
  char     *lpp_name;
  char     *option_name = NULL;
  char     *first_filename = (char *)NULL;
  char     *paths[20];
  char     *dirname;
  char     *vrmf = NULL;
  char     cksum[512];
  char     inst_rootPath[MAXPATHLEN+1];
  int      num_paths;
  int      i,j,k;
  List     *dirlist;
  int	   shareDataFlag=0;
  int      notFoundFlag=0;
  int      backupFlag=1;		/* back up all files in inslist */
  int	   lflag=0;
  int      YFlag=0;
  int      rc=0;

  if ((myname = strrchr(argv[0], '/')) != (char *)NULL) {
    myname++;
  }
  else {
    myname = argv[0];
  }

  tapeinfo.tapeflag = BACKUP;
  tapeinfo.blocksize = 0;

  while (( c = getopt(argc, argv, "b:f:s:ti:l:no:DF:U:LY")) != EOF) {
    switch(c) {

    case 'b':
      if ((tapeinfo.blocksize = adepackage_optsize(optarg)) == 0) {
	fatal("Bad block size");
      }
      break;

    case 'f':
      strcpy(tapeinfo.device_name, optarg);
      break;

    case 's':
      ship_path = optarg;
      i = 0;
      paths[i++] = strtok(ship_path,":");
      while ((paths[i++] = strtok(NULL,":")) != (char *)NULL);
      num_paths = --i;
      break;

    case 't':
      tapeinfo.tapeflag = TAR;
      break;

    case 'i':
      inslist_name = optarg;
      break;

    case 'l':
      lpp_name = optarg;
      break;
      
    case 'n':				/* Don't back up files from inslist */
      backupFlag=0;
      break;
      
    case 'o':
      option_name = optarg;
      break;

    case 'F':

      first_filename = optarg;
      break;
      
    case 'D':

      shareDataFlag++;
      break;

    case 'L':

      lflag++;
      break;

    case 'U':
      vrmf = optarg;
      break;
      
    case 'Y':
      YFlag++;
      break;

    default:
      adepackage_usage();
    }
  }

  if (tapeinfo.blocksize == 0)
    tapeinfo.blocksize = 20 * BLOCKSIZE;

  if (inslist_name == (char *)NULL) {
    fprintf(stderr, "ERROR: No input file specified with the -i flag\n");
    exit(1);
  }
  else {
    if((inslist_fd = fopen(inslist_name, "r")) == NULL) {
      fprintf(stderr, "ERROR: Could not open file: %s\n", inslist_name);
      exit(1);
    }
  }

  if ( strlen (vrmf) && !strlen (option_name) )
  {
     fprintf (stderr, "ERROR:  -o option required with -U option.\n");
     adepackage_usage();
  }

  if (error = init_tape(&tapeinfo))
    exit(-1);

  dirlist = listNew();
  listInit(dirlist);

  do_dir(dirlist, &tapeinfo, "/");

  if (first_filename != NULL)
    if (rc = write_first(first_filename, &tapeinfo, paths, num_paths))
    {
	/* If YFlag we don't stop when files not found */
	if ( !YFlag )
	    notFoundFlag=1;
    }
  
  if (rc = write_second(dirlist, &tapeinfo, lpp_name, paths, num_paths, shareDataFlag, vrmf, option_name))
  {
	/* If YFlag we don't stop when files not found */
	if ( !YFlag )
	    notFoundFlag=1;
  }

  /*---------------------------------------------------------------------
  | If backupFlag is zero then don't back up the files from the		|
  | inslist.  This flag is typically used for special case installs	|
  | where it is necessary to backup the liblpp.a and lpp_name info but	|
  | the files themselves are being copied and not installed.		|
  | (i.e. cdrom installation).						|
  ---------------------------------------------------------------------*/

  if ( !backupFlag )
  {
      write_tape_eot(&tapeinfo);
      exit (0);
  }

  /*---------------------------------------------------------------------
  | If readList returns non-zero there is an error in the inslist	|
  | entry and it should be ignored.					|
  ---------------------------------------------------------------------*/
  while ( (rc = readList(inslist_fd, &inslist, lflag)) != EOF) {
    if ( rc )
	continue;
    fileinfo.file_fd = 0;
    switch(inslist.type) {
    case 'F':    /* Regular Files */
    case 'A':
    case 'V':
      strcpy(fileinfo.filename, inslist.object_name);
      if (findfile(fileinfo.filename, paths, num_paths, fileinfo.ship_name,
		   &fileinfo.f_st.st, cksum) != 0) {
	fprintf(stderr, "Can not find file : %s in ship trees\n", fileinfo.filename);
	if ( YFlag )
	    continue;
        notFoundFlag=1;
      }
      else {
        if (notFoundFlag)
          continue;
	set_fileinfo(S_IFREG | inslist.mode, fileinfo.f_st.st.st_size, &fileinfo,
		     &inslist);
	fileinfo.file_fd = open(fileinfo.ship_name,O_RDONLY);
	if (inslist.object_name[0] == '/')
	  strcpy(fileinfo.filename,".");
	else
	  strcpy(fileinfo.filename, "./");
	strcat(fileinfo.filename,inslist.object_name);

	/*---------------------------------------------------------------
	| st_nlink should include the source file for the link so 	|
	| increment by 1.						|
	---------------------------------------------------------------*/
	if ((tapeinfo.tapeflag == BACKUP) && (inslist.numHardLinks > 0))
	  fileinfo.f_st.st.st_nlink = inslist.numHardLinks+1;

	write_file_to_tape(&tapeinfo, &fileinfo);

	/* process hard links */
	if ( !lflag )
	   continue;
	process_links(&fileinfo, &inslist, &tapeinfo);
      }
      break;
    case 'S':
	if ( !lflag )
	    continue;
	strcpy (fileinfo.filename, ".");
	strcat (fileinfo.filename, inslist.object_name);
	process_symlinks(&fileinfo, &inslist, &tapeinfo);
	continue;
	break;
    case 'f':    /* root Files */
    case 'a':
    case 'v':
    case 's':
      if ( !isaLink (inslist.type) )
      {
	   strcpy(fileinfo.filename, inslist.object_name);
           if (findfile(fileinfo.filename, paths, num_paths, fileinfo.ship_name,
		   &fileinfo.f_st.st, cksum) != 0) {
		fprintf(stderr, "Can not find file : %s in ship trees\n", fileinfo.filename);
		if ( YFlag )
		   continue;
		notFoundFlag=1;
           }
      }
      if (notFoundFlag)
          continue;
      /*
       *-----------------------------------------------------------------
       * fileinfo.filename should contain the inst_root path name
       * since this is a root entry.
       *-----------------------------------------------------------------
       */
       if (!strlen(vrmf))
       {
	   sprintf(fileinfo.filename, "./usr/lpp/%s/inst_root%s",
		   lpp_name, inslist.object_name) ;
           if ( lflag )
    		for ( i=0; i<inslist.numHardLinks; i++ ) 
		{
	   	    sprintf(inst_rootPath, "./usr/lpp/%s/inst_root%s",
		   	lpp_name, inslist.hardLinks[i]) ;
		    strcpy(inslist.hardLinks[i],inst_rootPath);
		}
       }
       else
       {
	   sprintf(fileinfo.filename, "./usr/lpp/%s/%s/%s/inst_root%s",
		   lpp_name, option_name, vrmf, inslist.object_name) ;
           if ( lflag )
    		for ( i=0; i<inslist.numHardLinks; i++ ) 
		{
	   	    sprintf(inst_rootPath, "./usr/lpp/%s/%s/%s/inst_root%s",
		   	lpp_name, option_name, vrmf, inslist.hardLinks[i]) ;
		    strcpy(inslist.hardLinks[i],inst_rootPath);
		}
       }

       if ( isaLink (inslist.type) )
       {
	    if ( !lflag )
		continue;
	    process_symlinks(&fileinfo, &inslist, &tapeinfo);
	    continue;
       }

       set_fileinfo(S_IFREG | inslist.mode, fileinfo.f_st.st.st_size, &fileinfo,
	     &inslist);
       fileinfo.file_fd = open(fileinfo.ship_name,O_RDONLY);

     /*
      *---------------------------------------------------------------
      * st_nlink should include the source file for the link so
      * increment by 1.
      *---------------------------------------------------------------
      */
      if ((tapeinfo.tapeflag == BACKUP) && (inslist.numHardLinks > 0))
	  fileinfo.f_st.st.st_nlink = inslist.numHardLinks + 1;

      write_file_to_tape(&tapeinfo, &fileinfo);

      /* process hard links */
      if ( !lflag )
	   continue;
      process_links(&fileinfo, &inslist, &tapeinfo);

      break;

    case 'D':     /* Directories */
      strcpy(fileinfo.filename, ".");
      strcat(fileinfo.filename, inslist.object_name);

      if ((tapeinfo.tapeflag == TAR) && 
	  (fileinfo.filename[strlen(fileinfo.filename) - 1] != '/'))
	strcat(fileinfo.filename, "/");  /* Directories for tar end in / */

      set_fileinfo(S_IFDIR | inslist.mode, 0, &fileinfo, &inslist);

      fileinfo.f_st.st.st_nlink = 0;

      if ( !listCompare (dirlist, fileinfo.filename) )
      {
	dirname = malloc (strlen(fileinfo.filename) + 1);
	strcpy (dirname, fileinfo.filename);
	listAppend(dirlist,dirname);
	write_file_to_tape(&tapeinfo, &fileinfo);
      }

      /* process hard links */
      if ( !lflag )
	   continue;
      process_links(&fileinfo, &inslist, &tapeinfo);
      break;
    
    case 'd':     /* root Directories */
      if (!strlen(vrmf))
       {
	   sprintf(fileinfo.filename, "./usr/lpp/%s/inst_root%s",
		   lpp_name, inslist.object_name) ;
           if ( lflag )
    		for ( i=0; i<inslist.numHardLinks; i++ ) 
		{
	   	    sprintf(inst_rootPath, "./usr/lpp/%s/inst_root%s",
		   	lpp_name, inslist.hardLinks[i]) ;
		    strcpy(inslist.hardLinks[i],inst_rootPath);
		}
       }
       else
       {
	   sprintf(fileinfo.filename, "./usr/lpp/%s/%s/%s/inst_root%s",
		   lpp_name, option_name, vrmf, inslist.object_name) ;
           if ( lflag )
    		for ( i=0; i<inslist.numHardLinks; i++ ) 
		{
	   	    sprintf(inst_rootPath, "./usr/lpp/%s/%s/%s/inst_root%s",
		   	lpp_name, option_name, vrmf, inslist.hardLinks[i]) ;
		    strcpy(inslist.hardLinks[i],inst_rootPath);
		}
       }

      if ((tapeinfo.tapeflag == TAR) && 
	  (fileinfo.filename[strlen(fileinfo.filename) - 1] != '/'))
	strcat(fileinfo.filename, "/");  /* Directories for tar end in / */

      set_fileinfo(S_IFDIR | inslist.mode, 0, &fileinfo, &inslist);

      fileinfo.f_st.st.st_nlink = 0;
      
      if ( !listCompare (dirlist, fileinfo.filename) )
      {
	dirname = malloc (strlen(fileinfo.filename) + 1);
	strcpy (dirname, fileinfo.filename);
	listAppend(dirlist,dirname);
	write_file_to_tape(&tapeinfo, &fileinfo);
      }

      /* process hard links */
      if ( !lflag )
	   continue;
      process_links(&fileinfo, &inslist, &tapeinfo);

      break;

    case 'P':     /* Fifos */
      strcpy(fileinfo.filename, ".");
      strcat(fileinfo.filename, inslist.object_name);
      set_fileinfo(S_IFIFO | inslist.mode, 0, &fileinfo, &inslist);
      write_file_to_tape(&tapeinfo, &fileinfo);
      break;

    case 'B':     /* Block Devices */
      strcpy(fileinfo.filename, ".");
      strcat(fileinfo.filename, inslist.object_name);
      set_fileinfo(S_IFBLK | inslist.mode, 0, &fileinfo, &inslist);
      write_file_to_tape(&tapeinfo, &fileinfo);
      break;

    case 'C':     /* Character Devices */
      strcpy(fileinfo.filename, ".");
      strcat(fileinfo.filename, inslist.object_name);
      set_fileinfo(S_IFDIR | inslist.mode, 0, &fileinfo, &inslist);
      write_file_to_tape(&tapeinfo, &fileinfo);
      break;

    case 'M':     /* Multiplex Devices */
      strcpy(fileinfo.filename, ".");
      strcat(fileinfo.filename, inslist.object_name);
      set_fileinfo(S_IFMPX | inslist.mode, 0, &fileinfo, &inslist);
      write_file_to_tape(&tapeinfo, &fileinfo);
      break;

    case 'I':     /* These files are not shipped */
    case 'i':
    case 'N':
    case 'n':
      break;

    default:
      fprintf(stderr, "Unrecognized Object type: %c \n", inslist.type);
      break;
    }
  }
  if (!notFoundFlag)
      write_tape_eot(&tapeinfo);
  else {
      close(tapeinfo.tape_fd);
      unlink(tapeinfo.device_name);
      fprintf(stderr, "One or more files could not be found in ship trees\n");
      fprintf(stderr, "The install image was not created on device/file %s\n",tapeinfo.device_name);
      exit(1);
  }
}
