static char sccsid[] = "@(#)89	1.4  src/bos/usr/lib/boot/mergedev/mergedev.c, cmdcfg, bos411, 9428A410j 5/29/91 17:13:41";
/*
 * COMPONENT_NAME: (CMDCFG) Generic config support cmds
 *
 * FUNCTIONS: mergedev
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define _NO_PROTO

#include	<stdio.h>
#include	<sys/stat.h>
#include	<sys/vnode.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<dirent.h>


#define NAMSIZE	40
#define MAXLINKS	50

struct	link_info			/* Duplicate link structure	*/
	{
	ino_t	inode;			/* Inode of last link		*/
	char	fnam[20];		/* Name of first file with link */
	} llist[MAXLINKS];

char    *opath = "/dev/";
char	*npath = "/mnt/dev/";

main()
{
int     rc,count=0;
struct  DIR     *dptr;
struct  dirent	*dent;

					/***********************************
					   Check that RAM disk directory
					   can be opened, If not then return
					   without moving any files and 
					   report error to stderr
					************************************/
if((dptr=(struct DIR *) opendir(opath)) == NULL)
	{
	fprintf(stderr,"Unable to open RAM disk directory %s\n",opath);
	exit(1);
	}

        /* If a filename is supplied change npath to that */
        if (argc > 1) {
           npath = argv[1];
           printf("Changing destination directory from %s to %s\n", npath, argv[1]);
        }
					/***********************************
					  Read the filenames of all files
					  in the directory and compare them
					  with the files already on the
					  hardfile.
					************************************/

while((dent = (readdir(dptr))) != NULL)
	{
	rc = cmp_dev(dent->d_name);
	if(rc)
		{
		count++;
		printf("Changed: %s\n",dent->d_name);
		}
	}

closedir(dptr);
printf("mergedev replaced %d files in the hardfile /dev directory\n",count);
}


/*** cmp_dev ****************************************
	Compare device entry in opath with device
	entry in npath to ensure matching major/minor
	numbers for the two devices.

	RETURNS:
		0 - Files match   
		1 - Files don't match or dev entry
		    doesn't exist in npath
******************************************************/

cmp_dev(dnam)
char	*dnam;
{
struct  stat ofinfo,nfinfo;
char	opbuf[NAMSIZE];
char	npbuf[NAMSIZE];
					/* Construct full path names	*/
strcpy(opbuf,opath);
strcpy(npbuf,npath);
strcat(opbuf,dnam);
strcat(npbuf,dnam);

					/* Stat the files		*/
if(stat(opbuf,&ofinfo) != 0)
	{
	fprintf(stderr,"Could not get file status\n");
	return(0);
	}
					/* Process if special file	*/
if(ofinfo.st_type == VBLK || ofinfo.st_type == VCHR || ofinfo.st_type == VMPC)
	{
					/* Check for file name on HF	*/
	if(stat(npbuf,&nfinfo) == 0)
		{
		if(ofinfo.st_rdev == nfinfo.st_rdev && 
		    (nfinfo.st_type == VBLK || nfinfo.st_type == VCHR || ofinfo.st_type == VMPC))
			{
					/* If link count > 1 put into	*/
					/* list of linked files	but	*/
					/* Don't do anything else	*/
			if(ofinfo.st_nlink > 1)
				isalink(dnam,ofinfo.st_ino,FALSE);
			return(0);
			}
					/* Major/Minor don't match	*/
		else
					/* Get rid of old dev entry	*/
			unlink(npbuf);
		}
					/* Check if this a link or file	*/
	if(ofinfo.st_nlink > 1)
		if(isalink(dnam,ofinfo.st_ino,TRUE))
			return(1);	/* Made a link all done		*/

					/* Try to make new dev entry	*/
/* do not copy ram disk special files (having major number 0) to disk */
	if (major(ofinfo.st_rdev) != 0)
	{
		mknod(npbuf,ofinfo.st_mode,ofinfo.st_rdev);
		return(1);
	}
	
	}
	return(0);
}

/*** isalink ****************************************
	Checks to see if there is already a node for
	this inode. Put the information in the
	link_info structure if a new file. If this is
	a link of an existing file, then create the 
	link. If ialflag=TRUE the link will be made,
	otherwise the list will only be checked and
	updated.

	RETURNS:
		0 - Not in link_info, need to make node
		1 - Already in link_info link was made.
******************************************************/

isalink(fnam,inode,ialflag)
char	*fnam;
ino_t	inode;
char	ialflag;
{
static int  llptr = 0;				/* list pointer			*/
int	i,j;
char	opbuf[NAMSIZE];
char	npbuf[NAMSIZE];

						/* Look for this item in the list*/
if(llptr != 0)
	{
	for(i=0;i<llptr;i++)
		{
		if(llist[i].inode == inode)	/* Found a match	*/
			{
			if(!ialflag)		/* Already in list return*/
				return(0);
			strcpy(opbuf,npath);
			strcpy(npbuf,npath);
			strcat(opbuf,llist[i].fnam);
			strcat(npbuf,fnam);
			if(link(opbuf,npbuf) != 0)
				return(0);	/* Couldn't link try to make*/
			else
				return(1);
			}
		}
	
	llist[llptr].inode = inode;		/* Add this one to list	*/
	strcpy(llist[llptr].fnam,fnam);
	if(llptr < MAXLINKS)			/* Stay in range	*/
		llptr++;
	}
else						/* Make the first list item*/
	{
	llist[llptr].inode = inode;
	strcpy(llist[llptr].fnam,fnam);
	llptr++;
	}

return(0);
}
