static char sccsid[] = "@(#)281.1 src/bos/usr/lpp/bosinst/adjfs/adjfs.c, bosinst, bos411, 9433B411a 94/08/08 15:44:51";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System installation
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * NAME: adjfs
 *                                                                    
 * FUNCTION: Calculates how much space each filesystem requires to install
 *	bos. If any filesystem is not large enough, it will be expanded.
 *                                                                    
 * ORIGINS: 27
 *                                                                    
 * EXECUTION ENVIRONMENT: This progam is called by the bos.rte.pre_i
 *	shell script.  As such, it will be called only during the 
 *	migration path of BOS install.
 *                                                                   
 * RETURNS: The following values are returned by this program:
 *
 *	0 => success
 *	1 => an error occured while attempting to create the filesystem list.
 *	2 => an error occured while accounting for the new files that will
 *           be created during migration.
 *	3 => an error occured while accounting for the files that will be 
 *           deleted during migration.
 *	4 => an error occured while accounting for the directories that 
 *	     will be deleted during a migration from 3.2 to 4.1.
 *	5 => an error occured while attempting to expand a filesystem.
 *	     When this error occurs, adjfs prints (to standard out) the name 
 *	     of the filesystem mount point and the extra space required 
 *	     (in 512-byte blocks).  These two fields are separated by a space
 *	     character.
 * 	
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define		BUFFSIZE	1024

typedef struct listel	*FsListElptr;

typedef struct listel {
	char	*fsname;	/* directory the filesystem is mounted on */
	char	*fsdevname;	/* filesystem device name		  */
	int	free_space;	/* amount of free space in the filesystem */
				/* (in 512-byte blocks)			  */
	int	space_required; /* amount of space required for migration */
				/* (in 512-byte blocks)			  */
	FsListElptr	next;	/* pointer to the next element in the list */
} FsListEl; 

static int procfiles(char *, FsListElptr, int);
static int delfiles(char *, FsListElptr);
static int deldirs(char *, FsListElptr);
static int expandfs(FsListElptr);
static int initfs(char *, FsListElptr *);

enum operation {ADD, SUB};

main (argc, argv)
int	argc;
char	*argv[];

{

int	fflag=0;
int	sflag=0;
int	dflag=0;
int	xflag=0;

int	c;

char	*fsfile=NULL;	/* file containing sorted list of filesystems 
			*/

char	*newfile=NULL; 	/* file containing the list of files that
			** will be created during migration                  
			*/

char	*dirlist=NULL;	/* file that contains list of directories (mainly)
			** that will be removed during migration from 
			** AIX 3.2 to 4.1.
			*/	

char	*remfile=NULL;	/* name of file containing a list of individual 
			** files that will be removed during migration       
			*/	

FsListElptr	fshead=(FsListElptr) NULL;	/* pointer to the beginning of list of 
						** filesystems.
						*/


	while ( (c = getopt(argc, argv, "f:s:d:x:")) != EOF ) {

		switch (c) {

			case 'f': 
			/* get name of file with list of filesystems */
				fsfile = optarg;
				fflag++;
				break;	

			case 's':
			/* get name of file with list of new files that will
			** be created during migration
			*/
				newfile = optarg;
				sflag++;
				break;

			case 'd':
			/* get name of file with list of directories that
			** will be removed during migration.
			*/
				dirlist = optarg;
				dflag++;
				break;


			case 'x':
			/* get name of file with list of individual files 
			** that will be removed during migration
			*/
				remfile = optarg;
				xflag++;
				break;

		} /* end switch (c) */	

	} /* end while */


	/* initialize the filesystem data structure with the information 
	** in the file "fsfile"
	*/

	if ( fflag && initfs(fsfile, &fshead) ) {

		/* if I can't initialize the data structure.  Things
	      	** are really hosed.
		*/

	   	exit (1);  
	}

	if ( sflag ) {
	
		/* account for the new files that will be added during 
		** migration.
		*/

		if ( procfiles(newfile, fshead, ADD) ) {
	
			/* error encountered while accounting for files added
			** during migration.
			*/

			exit (2);

		}

	}

	if ( xflag ) {
	
		/* account for the individual files that will be removed during 
		** migration.
		*/

		if ( delfiles(remfile, fshead) ) {
	
			/* error encountered while accounting for files deleted
			** during migration.
			*/

			exit (3);

		}

	}

	if ( dflag ) {
	
		/* account for the directories that will be removed during 
		** migration from 3.2 to 4.1.
		*/

		if ( procfiles(dirlist, fshead, SUB) ) {
	
			/* error encountered while accounting for files deleted
			** during migration.
			*/

			exit (4);

		}

	}


	/* We have now accounted for all the files/directories that will 
	** be added/deleted during migration.  Now traverse the filesystem
	** list and expand those filesystems that will require more space
	*/

	return  expandfs(fshead) ;

}

/*
** NAME: initfs
**
** FUNCTION: Initialize the list of filesystems from a file.  Each line in the
**           file has the full pathname of the directory that the filesystem 
**	     is mounted on, the filesystem followed by the
**	     amount of free-space in that filesystem.  The file must be sorted
**	     in reverse order by filesystem. This ensures that files get
**	     accounted for in the appropriate filesystem.  For example,
**	     /mnt/usr/gorp would appear before /mnt/usr.  This will ensure that
**	     the file /mnt/usr/gorp/foo is counted as being part of the 
**	     /mnt/usr/gorp filesystem instead of the /mnt/usr filesystem.
**
**	     To create a file (foo) in this format, execute the following:
**
**    /usr/bin/df -k | awk '{print $7, $1, $3}' | grep '^/mnt' | sort -r > foo
**
** RETURNS:	0 => Success
**		>0 => Failure
*/

static int 
initfs(char *fn, FsListElptr *fshead)

{

FILE		*fp;
FsListElptr	newel, 
		fstail;
char		*fspath;
char		*fsdev;
char		linebuf[BUFFSIZE];
int		linesize;
int		free_space;

	/* open the file containing the list of filesystems
	*/

	if ( (fp = fopen(fn, "r")) == NULL ) {
		
		/* can't open the file */

		return (1);

	}

        fstail = *fshead;


	/* read the next line from the file */

	while ( fgets(linebuf, BUFFSIZE, fp) ) {
		
		linesize = strlen(linebuf);	/* calculate the size of the string 
						** that we just read in
						*/

		if ( (fspath = (char *) malloc (linesize)) == NULL ) {

			/* can't allocate memory for the file system 
			** pathname 
			*/

			return (2);
		}

		if ( (fsdev = (char *) malloc (linesize)) == NULL ) {

			/* can't allocate memory for the file system 
			** device name 
			*/

			return (2);
		}

		
	 	sscanf(linebuf, "%s %s %d", fspath, fsdev,&free_space); 
		
		/* allocate a list element  */

		if ( (newel = (FsListElptr) malloc (sizeof (FsListEl))) == NULL ) {

			/* can't allocate memory */;
			return (2);
		}

		/* initialize the new element */

		if (fstail == (FsListElptr) NULL ) {
			*fshead = newel;
		}
		else {
			fstail -> next = newel;
		}

		newel -> fsname = fspath;
		newel -> fsdevname = fsdev;
		newel -> free_space = free_space*2; 
					/* df reports KB, we want 512-byte */

		newel -> space_required = 0;
		newel -> next = (FsListElptr) NULL;
		fstail = newel;		

	}

	fclose(fp);
	return 0;
}
		
/*
** NAME: fsize
**
** FUNCTION: Calculate the size of the specified file (in 512-byte blocks).
**           Assumes that file space is allocated in 8-block chunks.
**
** RETURNS: The size of the file.  If an error is detected, a file size of
**          0 is returned.
**
*/
static
int
fsize (char *fn)

{

struct stat buf;

	if ( lstat(fn, &buf) ) {

		/* Can't access the file. The file must not exist since this
		** program runs as root.
		*/

		return (0);

	}

	if ( S_ISREG(buf.st_mode) || S_ISDIR(buf.st_mode) || S_ISLNK(buf.st_mode) ) {

		/* the file is a regular file, a directory or a symbollic link */

		return (8 * ( (buf.st_size/4096) + (((buf.st_size % 4096 ) > 0) ? 1: 0) ) );

	}

	else {

		/* the st_size field is not valid for this file */

		return (0);
	}

}

/*
** NAME: match
**
** FUNCTION:	Find the filesystem in which the specified file is stored.
**
** RETURNS:	A pointer to the element in the filesystem list corresponding
** 		to the filesystem in which the specified file is stored.
**		If the file belongs to no filesystem, NULL is returned.
**
*/ 

static 
FsListElptr
match(FsListElptr fsp, char *fnp)

{

char 	*rp;

	/* search all filesystems */

	while ( fsp != (FsListElptr) NULL ) {

		rp = strstr(fnp, fsp -> fsname);
		if ( rp == fnp ) {

			/* we have found the filesystem that the file resides in */

			return (fsp);

		}

		else { 

			/* try the next filesystem in the list */

			fsp = fsp -> next;
		}

	}

	/* Should never get here.  Means we didn't find the filesystem the file 
	** reside in.
	*/

	return ( (FsListElptr) NULL );
}

/*
** NAME: convert
**
** FUNCTION: 	Convert the first newline (\n) character in a string into 
** 		a NULL character (\0). 
**
** RETURNS:	0 => Success
**		>0 => Failure
**
*/

static
int
convert(char *s)
{

char	*sp;

	if ( (sp = strchr(s, '\n')) != NULL ) {

		*sp = '\0';
		return (0);

	}
	else {

		/* Something is very wrong.  All strings 
		** returned by fgets should have a newline
		** in them
		*/

		return (1);
	}
}

/*
** NAME: procfiles
** 
** FUNCTION: 	Calculate the amount of space required in each filesystem to 
**		hold the files listed in fn.  Each line of fn contains:
**
**			o the pathname of a file
**			o whitespace
**			o the size of the file in 512-byte blocks
**
**		The operation code (op) determines whether these files will 
**		be added during migration (ADD) or deleted (SUB).
**
** RETURNS:	0 => Success
**		>0 => Failure
**
*/

static
int 
procfiles(char *fn, FsListElptr fshead, int op)

{

FILE		*fp;
FsListElptr	el; 
char		*fnamep;
int		required_space;
int		linesize;
char		linebuf[BUFFSIZE];


	/* open the file containing the list of filesystems
	*/

	if ( (fp = fopen(fn, "r")) == NULL ) {
		
		/* can't open the file */

		return (1);

	}

	/* read the next line from the file */

	while ( fgets(linebuf, BUFFSIZE, fp) ) {
		
		linesize = strlen(linebuf);	/* calculate the size of the string 
						** that we just read in
						*/

		if ( (fnamep = (char *) malloc (linesize)) == NULL ) {

			/* can't allocate memory for the file pathname */
			fclose(fp);
			return (2);
		}

		
	 	sscanf(linebuf, "%s %d", fnamep, &required_space); 
		
		if ( (el = match(fshead, fnamep)) == (FsListElptr) NULL ) {

			/* Something is really hosed.  Couldn't find a matching 
			** filesystem for the file, but all files should find 
			** a place (at least in the root filesystem).
			*/
			
			fclose(fp);
			return (3);

		}
	


		if ( op == ADD) { /* file being added during migration */

			el -> space_required += required_space;

		}

		else {  /* file being deleted during migration */

			el -> space_required -= required_space;

		}

		free(fnamep);

	}

	fclose(fp);

	return 0;
		
}

/*
** NAME: delfiles
** 
** FUNCTION: 	Calculate the amount of space that will be recovered from 
**		each filesystem when the individual files that are 
**		removed during migration are accounted for.  The files
**		that will be deleted are listed in fn.  Each line of fn 
**		contains the pathname of a file.
**
** RETURNS:	0 => Success
**		>0 => Failure
**
*/

static
int 
delfiles(char *fn, FsListElptr fshead)
{

FILE		*fp;
FsListElptr	el; 
int		linesize;
char		linebuf[BUFFSIZE];


	/* open the file containing the list of filesystems
	*/

	if ( (fp = fopen(fn, "r")) == NULL ) {
		
		/* can't open the file */

		return (1);

	}

	/* read the next line from the file */

	while ( fgets(linebuf, BUFFSIZE, fp) ) {
		
	 	if (convert(linebuf)) {

			/* if convert fails, something is very wrong.  
			** All lines returned by fgets should have a 
			** newline in them
			*/

			return (2);

		}
		
		if ( (el = match(fshead, linebuf)) == (FsListElptr) NULL ) {

			/* Something is really hosed.  Couldn't find a matching 
			** filesystem for the file, but all files should find 
			** a place (at least in the root filesystem).
			*/
			
			fclose(fp);
			return (3);

		}
	
		el -> space_required -= fsize(linebuf);

	}
		

	fclose(fp);

	return 0;

}

/*
** NAME: expandfs
**
** FUNCTION:	Expand any filesystem that does not have enough space for
**		the migration install to succeed.
**
** RETURNS:	0 => Success
**		>0 => Failure
**
*/

static
int 
expandfs(FsListElptr fsp)
{

int	extra_space_needed;
char	cmdbuf[BUFFSIZE];

	/* for all filesystems */

	while ( fsp != (FsListElptr) NULL ) {

		/* check if more space is required in this filesystem */
		extra_space_needed = (fsp ->space_required) 
					- (fsp -> free_space);

#ifdef DEBUG
		fprintf(stderr, "Filesystem mount point = %s\n", fsp->fsname);
		fprintf(stderr, "Filesystem device = %s\n", fsp->fsdevname);
		fprintf(stderr, "Space required = %d, free space = %d, space needed = %d \n", fsp->space_required, fsp->free_space, extra_space_needed);
#endif

		if ( extra_space_needed > 0 ) {

			sprintf(cmdbuf, "/mnt/usr/sbin/chfs -a size=+%d %s",
				extra_space_needed, fsp -> fsdevname);

			if ( system(cmdbuf) ) {

				/* Change filesystem command failed.
				** Cannot proceed with the migration.
				*/

                                printf("%s %d\n", fsp -> fsname,
                                                extra_space_needed);

				return (5);

			}

		}

		/* check the next filesystem in the list */

		fsp = fsp -> next;


	}

	return 0;
}

