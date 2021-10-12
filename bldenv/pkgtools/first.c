static char sccsid[] = "@(#)24  1.10  src/bldenv/pkgtools/first.c, pkgtools, bos412, GOLDA411a 6/8/94 15:32:55";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: do_dir
 *		writeRootFiles
 *		writeShareFiles
 *		write_first
 *		write_second
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
#include <stdio.h>
#include <fcntl.h>

/* 
 *===================================================================
 * Function Name : write_first
 * Description   :
 * Inputs	 :
 * Outputs       :
 * Notes         :
 *===================================================================
 */ 
int 
write_first(char	*first_filename, 
	    Tapeinfo	*tape, 
	    char	*paths[],
	    int		num_paths)
{
  FILE *fd;
  char line[255];
  InsEntry in;
  Fileinfo finfo;
  char  cksum[255];
  int notFoundFlag=0;

  if ((fd = fopen(first_filename, "r")) == NULL) {
    fprintf(stderr, "Can't open First file %s\n", first_filename);
    exit(1);
  }
  else {

    while (stripComments(fd, line) != EOF) {
      in.type = 'F';
      in.uid = 0;
      in.gid = 0;
      in.mode = 0555;
      strcpy(finfo.filename, line);
      strcpy(in.object_name, line);

      if (findfile(finfo.filename, paths, num_paths, finfo.ship_name,
		   &finfo.f_st.st, cksum) != 0) {
	fprintf(stderr, "Can't find file %s in ship trees\n", finfo.filename);
        notFoundFlag=1;
      }
      else {
	set_fileinfo(S_IFREG | in.mode, finfo.f_st.st.st_size, &finfo, &in);
	finfo.file_fd = open(finfo.ship_name, O_RDONLY);
	if (in.object_name[0] == '/')
	  strcpy(finfo.filename, ".");
	else
	  strcpy(finfo.filename, "./");
	strcat(finfo.filename, in.object_name);
       
	write_file_to_tape(tape, &finfo);
      }
    }
  }
  return(notFoundFlag);
}

/* 
 *===================================================================
 * Function Name : write_second
 * Description   : Write the lpp_name and liblpp.a files to the media.
 *	The lpp_name file should be in the current directory.
 *	Call writeRootFiles and writeShareFiles to make the appropriate
 *	liblpp.a path.  Look for a root/liblpp.a file.  If found,
 *	back up to media and create the /usr/lpp/<pathName>/inst_root
 *	directory where root files will be installed.
 * Inputs	 : 
 * Outputs       :
 * Notes         : For install images the path name of the inst_root
 *	directory is:
 *		/usr/lpp/<lppname>/inst_root
 *	For update images (vrmf non-zero length) the path name of the
 *	inst_root directory is:
 *		/usr/lpp/<lppname>/<option_name>/<vrmf>/inst_root
 *===================================================================
 */ 
int 
write_second(List	*list, 
	     Tapeinfo	*tape, 
	     char	*lppname, 
	     char	**paths, 
	     int	num_paths, 
	     int	shareDataFlag, 
	     char	*vrmf,
	     char	*option_name)
{
  InsEntry in;
  Fileinfo finfo;
  char     cksum[255];
  char     tempname[200];
  int notFoundFlag=0;
  int rc=0;

  in.type = 'F';
  in.uid  = 0;
  in.gid  = 0;
  in.mode = 0555;

  strcpy(finfo.filename, "/lpp_name");
  strcpy(in.object_name, finfo.filename);

  if (findfile(finfo.filename, paths, num_paths, finfo.ship_name,
	       &finfo.f_st.st, cksum) != 0) 
  {
    fprintf(stderr, "Can't find file %s in ship trees\n",finfo.filename);
    notFoundFlag=1;
  }
  else 
  {
    set_fileinfo(S_IFREG | in.mode, finfo.f_st.st.st_size, &finfo, &in);
    finfo.file_fd = open(finfo.ship_name, O_RDONLY);
  
    if (in.object_name[0] == '/')
      strcpy(finfo.filename, ".");
    else
      strcpy(finfo.filename, "./");
    strcat(finfo.filename, in.object_name);
    
    write_file_to_tape(tape, &finfo);
  }

  if ( shareDataFlag )
  {
	if (rc = writeShareFiles(list, tape, lppname, &in, 
				 &finfo, paths, num_paths, vrmf, option_name))
            notFoundFlag=1;
	return(notFoundFlag);
  }
  else 
  {
      if (rc = writeRootFiles(list, tape, lppname, &in, 
			      &finfo, paths, num_paths, vrmf, option_name)) 
      {
	  notFoundFlag=1;
	  return(notFoundFlag);
      }
  }

  strcpy(finfo.filename, "/root/liblpp.a");
  strcpy(in.object_name, finfo.filename);

  if (findfile(finfo.filename, paths, num_paths, finfo.ship_name,
	       &finfo.f_st.st, cksum) == 0) 
  {
    
   /* 
    *-------------------------------------------------
    * Put out the inst_root directory if it has
    *	not already been done.
    *------------------------------------------------- 
    */
    
    if (!strlen(vrmf))
    {
	sprintf(tempname, "/usr/lpp/%s/inst_root", lppname) ;
    }
    else
    {
	sprintf(tempname, "/usr/lpp/%s/%s/%s/inst_root",
		lppname,
		option_name,
		vrmf) ;
    }
    do_dir(list, tape, tempname);
	
   /* 
    *----------------------------------------------------
    * Now generate the filename (depending on whether
    * this is an update or not).
    *----------------------------------------------------
    */
    if (!strlen(vrmf))
    {
	sprintf(finfo.filename, "./usr/lpp/%s/inst_root/liblpp.a", lppname) ;
    }
    else
    {
	sprintf(finfo.filename, "./usr/lpp/%s/%s/%s/inst_root/liblpp.a", 
		lppname, option_name, vrmf) ;
    }
   /* 
    *----------------------------------------------------
    * Finally set the owner, permissions, etc. and
    * put the file on the tape.
    *----------------------------------------------------
    */
    set_fileinfo(S_IFREG | in.mode, finfo.f_st.st.st_size, &finfo, &in);
    finfo.file_fd = open(finfo.ship_name, O_RDONLY);
    
    write_file_to_tape(tape, &finfo);
  }
  return(notFoundFlag);
}

/*----------------------------------------------------------------------*/
/* 
 *===================================================================
 * Function Name : do_dir
 * Description   :
 * Inputs	 :
 * Outputs       :
 * Notes         :
 *===================================================================
 */ 
void 
do_dir(List	*list, 
       Tapeinfo	*tape, 
       char	*name)
{
  InsEntry in;
  Fileinfo finfo;
  char     *x;

  strcpy(finfo.filename, ".");
  strcat(finfo.filename, name);
  
  in.type = 'D';
  in.uid  = 2;
  in.gid  = 2;
  in.mode = 0755;
  
  if ((tape->tapeflag == TAR) && 
      (finfo.filename[strlen(finfo.filename) - 1] != '/'))
  {
      strcat(finfo.filename, "/");
  }

  set_fileinfo(S_IFDIR | in.mode, 0, &finfo, &in);

  finfo.f_st.st.st_nlink = 0;

  x = malloc(strlen(finfo.filename) + 2);

if (x == NULL) printf("Can't Malloc space\n");

  strcpy(x, finfo.filename);

  listAppend(list, x);

  write_file_to_tape(tape, &finfo);
}

/* 
 *===================================================================
 * Function Name : writeShareFiles
 * Description   : Write the liblpp.a file to the media for a share
 *	data package.  The location of liblpp.a is data/liblpp.a
 *	in the full build tree and it should be backed up to
 *	/usr/share/lpp/<lppname> if it is for an install image.  
 *      For an update build, liblpp.a is located in the current
 *      directory for the PTF and it should be backed up to
 *	/usr/share/lpp/<lppname>/<option>/<vrmf>
 * Inputs	 :
 * Outputs       :
 * Notes         :
 *===================================================================
 */ 
int
writeShareFiles(List	 *list, 
		Tapeinfo *tape, 
		char	 *lppname, 
		InsEntry *in,
		Fileinfo *finfo, 
		char	 **paths, 
		int	 num_paths, 
		char	 *vrmf,
		char	 *option_name)
{
    char tempname[200];
    char     cksum[255];
    int notFoundFlag=0;
    int updateBld ;


    updateBld = strlen(vrmf) ;		/* vrmf will exist for updates	*/
    if (updateBld == 0)
    {
	strcpy (finfo->filename, "/data/liblpp.a");
    }
    else  /* This is an update build */
    {
	strcpy (finfo->filename, "/liblpp.a");	
    }
    strcpy(in->object_name, finfo->filename);

    if ( findfile(finfo->filename, paths, num_paths, finfo->ship_name,
		  &(finfo->f_st.st), cksum) != 0) 
    {
	fprintf(stderr, "Can't find file %s in ship trees\n", 
		finfo->filename);
	notFoundFlag=1;
    }
    else 
    {
       /* 
	*------------------------------------------------------
	* Generate directory names and filenames depending
	* on whether this is an update or not.
	*------------------------------------------------------ 
	*/
        if(updateBld == 0)
        {
	    sprintf(tempname, "/usr/share/lpp/%s", lppname);
	    sprintf(finfo->filename, ".%s/liblpp.a", tempname) ;
        }
        else
        {
	    sprintf(tempname, "/usr/share/lpp/%s/%s/%s",
		lppname,
		option_name,
		vrmf);
	    sprintf(finfo->filename, ".%s/liblpp.a", tempname);
        }
       /*
        *----------------------------------------------
        * Put out the directories first 
        *----------------------------------------------
	*/
	do_dir (list, tape, "/usr/share");
	do_dir (list, tape, "/usr/share/lpp");
	do_dir(list, tape, tempname);
       /* 
	*---------------------------------------------
	* Finally write it to tape.
	*---------------------------------------------
	*/
	set_fileinfo(S_IFREG | in->mode, finfo->f_st.st.st_size, finfo, in);
	finfo->file_fd = open(finfo->ship_name, O_RDONLY);

	write_file_to_tape(tape, finfo);
    }
    return(notFoundFlag);
}

/* 
 *===================================================================
 * Function Name : writeRootFiles
 * Description   : Back up the liblpp.a for a usr or usr/root package.
 *	liblpp.a should be found in the current directory.  For an
 *	install image back up liblpp.a to the following location:
 *		/usr/lpp/<lppname>/liblpp.a
 *	For an update (non-zero length vrmf) back up liblpp.a to
 *	the following location:
 *		/usr/lpp/<lppname>/<option>/<vrmf>
 * Inputs	 :
 * Outputs       :
 * Notes         :
 *===================================================================
 */ 
int
writeRootFiles(List	*list, 
	       Tapeinfo	*tape, 
	       char	*lppname, 
	       InsEntry	*in,
	       Fileinfo	*finfo, 
	       char	**paths, 
	       int	num_paths, 
	       char	*vrmf,
	       char	*option_name)
{
  char tempname[200];
  char     cksum[255];
  int notFoundFlag=0;

  strcpy(finfo->filename, "/liblpp.a");

  strcpy(in->object_name, finfo->filename);

  if ( findfile(finfo->filename, paths, num_paths, finfo->ship_name,
	       &(finfo->f_st.st), cksum) != 0) 
  {
      fprintf(stderr, "Can't find file %s in ship trees\n", finfo->filename);
      notFoundFlag=1;
  }
  else 
  {
     /* 
      *------------------------------------------------------
      * Generate directory names and filenames depending
      * on whether this is an update or not.
      *------------------------------------------------------ 
      */
      if(! strlen(vrmf))
      {
	  sprintf(tempname, "/usr/lpp/%s", lppname);
	  sprintf(finfo->filename, ".%s/liblpp.a", tempname);
      }
      else
      {
	  sprintf(tempname, "/usr/lpp/%s/%s/%s",
		lppname,
		option_name,
		vrmf);
	  sprintf(finfo->filename, ".%s/liblpp.a", tempname);
      }
     /* 
      *---------------------------------------
      * Put out the directories first
      *---------------------------------------
      */
      do_dir(list, tape, "/usr");
      do_dir(list, tape, "/usr/lpp");
      do_dir(list, tape, tempname);
     /*
      *---------------------------------------
      * Finally get the file and write it
      * to tape.
      *---------------------------------------
      */
      set_fileinfo(S_IFREG | in->mode, finfo->f_st.st.st_size, finfo, in);
      finfo->file_fd = open(finfo->ship_name, O_RDONLY);

      write_file_to_tape(tape, finfo);
  }
  return(notFoundFlag);
}
