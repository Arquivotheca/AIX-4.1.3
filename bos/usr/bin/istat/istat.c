static char sccsid [] = "@(#)50	1.11.2.6  src/bos/usr/bin/istat/istat.c, cmdfs, bos411, 9434A411a 8/19/94 15:05:10";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: istat
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* 
 * istat:
 *      program to give formatted listings of inodes
 *
 * synopsis:
 *      istat <filename>
 * or
 *      istat <inumber> <device>
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>
#include <nl_types.h>	
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <jfs/filsys.h>
#include <jfs/ino.h>
#include <libfs/libfs.h>
#include "istat_msg.h"

#define MSGSTR(Num,Str)  catgets(catd,MS_ISTAT,Num,Str)

void			print_stats (struct stat*, fdaddr_t*);
int 			read_super (int, struct superblock*, char*);
static fdaddr_t		*readinode (char*, ino_t, struct stat*, fdaddr_t*);
static char 		*protection (unsigned);
static char 		*get_time (time_t);

nl_catd 	catd;
static char 	errorline[100];

/*
 *
 *  MAIN
 *
 */

int
main (int argc,
      char **argv)
{
	struct stat	stat_info;
	fdaddr_t	faddrs[NDADDR];	/* Buffer for inode block numbers */
	fdaddr_t	*frags;


	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_ISTAT, NL_CAT_LOCALE);
	frags = (fdaddr_t *) NULL;
	
	if ((argc < 2) || (argc > 3))
	{
		fprintf (stderr, MSGSTR (USAGE,
			"Usage: istat {File | Inode Device}\n"));
		exit (1);
	}
	/*
	 * If, a file name was passed in then stat the file
	 * else read the inode of the specified filesystem.
	 * In either case save the pertinent data in a struct stat.
	 */
	if (argc == 2)
	{
		if (stat (argv[1], &stat_info) < 0)
		{
			sprintf(errorline, 
				MSGSTR (CANTSTAT, "istat: Unable to stat %s"), 
				argv[1]);
			perror (errorline);
			exit (2);
		}
	}
	else if ((frags = (fdaddr_t *)
		  readinode (*(argv + 2), atoi(*(argv + 1)),
			     &stat_info, faddrs)) == NULL)
	{
		exit (3);
	}

	print_stats (&stat_info, frags);
	exit (0);
}

/*
 *
 *  print_stats
 *
 *	Print out info in struct stat. If the user provided an inode number 
 *	then (fdaddr_t *) frags will point to a list of frag numbers to 
 * 	be printed; otherwise the list is NULL and no frag numbers are
 *	printed.
 *
 */

void
print_stats (struct stat  *st,
	     fdaddr_t *frags)
{
	register 	type;
	register dev_t 	device;
	struct passwd 	*pw;
	struct group 	*gr;
	int		j;


	printf (MSGSTR (INODEONDEV, "Inode %d on device %d/%d\t"),
		st->st_ino, major (st->st_dev), minor(st->st_dev));

	type = st->st_mode & S_IFMT;

	if (st->st_nlink == 0)
		printf (MSGSTR (UNALLOC, "Unallocated Inode\n"));
	else
	{
		switch (type)
		{ 
			case S_IFDIR:
			printf (MSGSTR (DIRECTORY, "Directory\n"));
			break;

			case S_IFBLK:
			printf (MSGSTR (BLOCKDEV, "Block Device\n"));
			frags = (fdaddr_t *) NULL;
			break;

			case S_IFCHR:
			printf (MSGSTR (CHARDEV, "Character Device\n"));
			frags = (fdaddr_t *) NULL;
			break;

			case S_IFREG:
			printf (MSGSTR (FILE, "File\n"));
			break;

			case S_IFIFO:
			printf(MSGSTR(FIFO,"FIFO\n"));
			frags = (fdaddr_t *) NULL;
			break;

			default:
			printf (MSGSTR (TYPEOFILE, "Type %o file\n"), type);
		}
		printf (MSGSTR (PROTECTION,
				"Protection: %s\t"), protection (st->st_mode));

		if (st->st_mode & S_ISUID)
			printf (MSGSTR (SETUID, "Set UID "));
		if (st->st_mode & S_ISGID)
			printf (MSGSTR (GETGID, "Set GID "));
		if (st->st_mode & S_ISVTX)
			printf (MSGSTR (STICKY, "Sticky"));
		printf ("\n");
	}

	printf (MSGSTR (OWNER,"Owner: %d(%s)\t\t"),
		st->st_uid,
		(pw = getpwuid (st->st_uid)) ? pw->pw_name : "<UNKNOWN>");
	
	printf (MSGSTR (GROUP, "Group: %d(%s)\n"),
		st->st_gid, (gr = getgrgid (st->st_gid)) ?
		gr->gr_name : "<UNKNOWN>");

	printf (MSGSTR (LINKSNLEN, "Link count: %3d\t\tLength %ld bytes\n"),
		st->st_nlink, st->st_size);

	if (type == S_IFBLK || type == S_IFCHR)
	{
		device = st->st_rdev;
		printf (MSGSTR (MAJORMINOR,
				"Major Device %d\t\tMinor Device %d\n"),
			major (device), minor (device));
	}
	
	printf ("\n");
	printf (MSGSTR (CTIME, "Last updated:\t%s"),  get_time (st->st_ctime));
	printf (MSGSTR (MTIME, "Last modified:\t%s"), get_time (st->st_mtime));
	printf (MSGSTR (ATIME, "Last accessed:\t%s"), get_time (st->st_atime));

	if (frags != NULL )
	{
		/* 
		 * Dump block pointers in hex
		 */
		printf(MSGSTR(BLOCKPTRS,"\nBlock pointers (hexadecimal):\n"));
		for (j = 0; j < NDADDR; j++)
			printf ("%-10.1x", frags[j].d);


		printf ("\n");
	}
	printf("\n");
}
/*
 *
 * readinode
 *
 *	Read the specified inode and copy the info into the stat struct; this
 *	is done so that print_stats() can use a common data struct. 
 *	Also, copy the frag numbers from the inode to a buffer of frag_t's.
 *
 *	Return:	pointer to frag_t's if successful
 *		NULL if failure.
 *
 */

static fdaddr_t	
*readinode (char *dev,
	    ino_t ino,
	    struct stat  *i,
	    fdaddr_t  *faddrs)
{
	register int 		fd;
	struct superblock	sb;
	struct dinode		di;
	
	if ((fd = fsopen (dev, O_RDONLY)) < 0)
	{
		sprintf (errorline, 
			MSGSTR(CANTOPEN, "istat:  Cannot find or open %s"), 
			dev);
		perror(errorline);
		return ((fdaddr_t *) NULL);		
	}
	/*
         * Read and verify the superblock
	 */
	if (read_super (fd, &sb, dev) != 0)
		return ((fdaddr_t *) NULL);

	if (get_inode (fd, &di, ino) != 0)
	{
		fprintf (stderr,
			 MSGSTR (INUMBLK,
				 "Can't get block for inode %d\n"), ino);
		return ((fdaddr_t *) NULL);
	}

	fstat (fd, i);
	close (fd);

	/*
	 * copy the basic fields into the standard status structure
	 */
	i->st_dev        = i->st_rdev; /* raw dev of fs is dev of inode */
	i->st_ino        = ino;
	i->st_mode       = di.di_mode;
	i->st_nlink      = di.di_nlink;
	i->st_uid        = di.di_uid;
	i->st_gid        = di.di_gid;
	i->st_size       = di.di_size;
	i->st_atime      = di.di_atime;
	i->st_ctime      = di.di_ctime;
	i->st_mtime      = di.di_mtime;
	/*
	 * Save the block addresses out of the struct dinode
	 */
	memcpy (faddrs, di.di_rdaddr, sizeof (daddr_t) * NDADDR);
	/*
	 * Save the major/minor just in case inode is a device
	 */
	i->st_rdev = di.di_rdev;

	return (faddrs);
}
/*
 *
 * read_super
 *
 * Process return codes from get_super
 *
 */
int
read_super (int fd,
	    struct superblock *sb,
	    char  *fsname)
{
	switch (get_super (fd, sb))
	{
		case LIBFS_SUCCESS:
		return (0);

		case LIBFS_BADMAGIC:
		fprintf(stderr, MSGSTR (NOTJFS,
		"istat:  %s is not recognized as a JFS filesystem.\n"),
			fsname);
		return (1);

		case LIBFS_BADVERSION:
		fprintf(stderr, MSGSTR (NOTSUPJFS,
		"istat:  %s is not a supported JFS filesystem version.\n"),
			fsname);
		return (1);

		case LIBFS_CORRUPTSUPER:
		fprintf(stderr, MSGSTR (CORRUPTJFS,
		"istat:  The %s JFS filesystem super block is corrupted.\n"),
			fsname);
		return (1);
		
		default:
		fprintf(stderr, MSGSTR(CANTRESB, 
			"istat:  Cannot read super block on %s.\n"), fsname);
		return (1);
	}
}	

/*
 *
 * protection
 *
 * print out the protection bits associated with an Inode
 *
 */

static char
*protection (unsigned mode)
{
    register int field;
    register int mask;
    static char *bits[4];
    static char prot[256];

    /*
     * this routine was clever,
     * then it was kanjified. (multi-byte characters)
     */ 
    bits[0] = MSGSTR(READBIT,"r");
    bits[1] = MSGSTR(WRITEBIT,"w");
    bits[2] = MSGSTR(EXECBIT,"x");
    bits[3] = MSGSTR(BLANKBIT,"-");

    for (field = 0, mask = 0400; mask; mask >>= 1, field++)
	    strcat (prot, ((mode & mask) ? bits[field % 3 ] : bits[3]));

    return (prot);
}

/*
 * This is here so we can get an independent NL date string.
 *
 */

#define MAXDATELENGTH	100

static char
*get_time (time_t thetime)
{
	struct tm *tmp;
	static char buf[MAXDATELENGTH];
	int i;
	
	if ((tmp = localtime(&thetime)) == NULL)
		return((char *) NULL);

	/* we append a '\n' for historical reasons (ctime did) */
	if (i = strftime(buf, MAXDATELENGTH - 1,
			 MSGSTR(DATE_FMT,"%a %b %d %T %Y"), tmp)) 
	{
		buf[i++] = '\n';
		buf[i] = '\0'; 
        } 
	else 
	{
		buf[0] = '\n';
		buf[1] = '\0';
	}

	return (buf);
}
