static char sccsid[] = "@(#)42	1.1  src/bos/usr/lib/boot/bootutil/bootutil.c, bosboot, bos411, 9428A410j 3/10/94 11:51:18";
/*
 *   COMPONENT_NAME: bosboot
 *
 *   FUNCTIONS: add_to_list
 *		dump_list
 *		list
 *		main
 *		remove_files
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This program is designed to recursively remove files from a directory
 * until a specified amount of free space has been made available.  This
 * is not meant to be a user level command, and as such, there are some
 * assumptions that have been made with regard to the design of this.
 * It is assumed that the user of this program, which should always be
 * "root", will have write access to every file in the target directory
 * structure, and search access to every directory in the target
 * directory structure.  There is no message translation and minimal
 * error checking.  Empty directories are ignored.
 *
 * Input:	-d <directory name>, name of a directory to clean
 *		-s <integer number>, amount of free space to create
 * Output:	the names of the files that are being removed
 * Exit codes:	0 when successful, 1 when unsuccessful
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/types.h>

#define KILOBYTE 1024

int list(char *name);
int add_to_list(char *name, time_t filetime);
int dump_list(void);
int remove_files(long req_space, char *targetdir, long ava_space, long kb_blk);

typedef struct fnode {
	char *filename;		/* full pathname */
	time_t amtime;		/* last access or mod time */
	struct fnode * next;		/* points to next item in list */
} fthing;

fthing *head;

/* ==========================================================================*/
main(int argc, char **argv)
{
	extern int optind;
	extern char *optarg;
	char *targetdir;
	int c, dflg, sflg;
	long kb_blk, req_space;
	struct statfs stbuf;

	c = dflg = sflg = 0;
	while ((c = getopt(argc, argv, "d:s:")) != EOF) {
		switch (c) {
			case 'd':	targetdir=optarg;
					dflg++;
					continue;
			case 's':	req_space=atol(optarg);
					sflg++;
					continue;
			case '?':	exit (1);
		}
	}

	if (!(dflg && sflg))	/* required flags */
		exit (1);

	statfs(targetdir, &stbuf);		/* stat the target directory */
	kb_blk = stbuf.f_bsize / KILOBYTE;	/* kb per filesystem block */
#ifdef BOOTDEBUG
	printf("kb_blk: %ld\nreq_space: %ld\n", kb_blk, req_space);
	printf("f_bfree: %ld\n", stbuf.f_bfree);
	printf("f_blocks: %ld\nf_bsize: %ld\n", stbuf.f_blocks, stbuf.f_bsize);
#endif
	if (((stbuf.f_bfree * kb_blk) >= req_space) ||
				((stbuf.f_blocks * kb_blk) <= req_space)) {
		/*
		 * either the requested amount of free space is already
		 * available, or the requested amount of free space
		 * exceeds the total space in the filesystem
		 */
		exit(0);
	}
	list(targetdir);	/* recursively search starting at target dir */
#ifdef BOOTDEBUG
	dump_list();		/* display the entire linked list */
#endif
	remove_files(req_space, targetdir, stbuf.f_bfree, kb_blk);
	exit(0);
}

/* ==========================================================================*/
int list(char *name)
{
/*
 * list the files in the directory
 */
	DIR *dp;
	struct dirent *dir;
	struct stat sbuf;
	char newname[1024];

	stat(name, &sbuf);
	if ((sbuf.st_mode & S_IFMT) != S_IFDIR)
		exit (1);
	if ((dp = opendir(name)) == NULL ) {
		fprintf(stderr, "cannot open directory.\n");
		exit(1);
	}

	/*
	* read the entries
	*/
	while ((dir = readdir(dp)) != NULL) {
		/*
		 * skip the removed files and filenames that
		 * consist only of dot or dotdot
		 */
		if ((dir->d_ino == 0) || (dir->d_name[0] == '.' &&
			(!dir->d_name[1] || (dir->d_name[1] == '.' &&
				!dir->d_name[2]))))
			continue;
		/* make full path */
		sprintf(newname, "%s/%s", name, dir->d_name);
		stat(newname, &sbuf);

		/*
		 * recursively search lower directories
		 */
		if ((sbuf.st_mode & S_IFMT) == S_IFDIR)
			list(newname);
		/*
		 * now add this file to the linked list. use the
		 * most recent of atime and mtime
		 */
		else add_to_list(newname, (sbuf.st_atime > sbuf.st_mtime)
					? sbuf.st_atime : sbuf.st_mtime);
	}
	closedir(dp);
	return(0);
}

/* ==========================================================================*/
int
add_to_list(char *name, time_t filetime)
{
	/*
	 * adds items to a linked list.  items are sorted from
	 * oldest to newest and accessed from the first element
	 */
	fthing * newnode;
	fthing * tmpnodeptr;

	newnode=(fthing * )malloc(sizeof(fthing));
	newnode->amtime=filetime;
	newnode->filename=(char *)malloc(strlen(name) + 1);
	strcpy(newnode->filename, name);

	/*
	 * special case: the filetime of the newnode precedes
	 * that of the first (head) node, or the case where the
	 * the newnode is the very first node created
	 */
	if ((newnode->amtime < head->amtime) || (head == (void *)0)) {
		newnode->next = head;
		head = newnode;
		return (0);
	}

	/*
	 * normal case: search the list for the correct place to
	 * insert the new node. Stop when we find a node with amtime
	 * that is greater than or equal to the amtime of newnode,
	 * or when we get to the end of the linked list
	 */
	tmpnodeptr = head;
	while ((tmpnodeptr->next->amtime < newnode->amtime) &&
					(tmpnodeptr->next != (void *)0))
		tmpnodeptr = tmpnodeptr->next;
	newnode->next = tmpnodeptr->next;
	tmpnodeptr->next = newnode;
	return (0);
}

/* ==========================================================================*/
int
dump_list(void)
{
	/*
	 * display a linked list, starting with the first element
	 */
	fthing * tmpnodeptr;

	tmpnodeptr = head;
	while (tmpnodeptr != (void *)0) {
		printf("%10ld  %s%\n", tmpnodeptr->amtime,
					tmpnodeptr->filename);
		tmpnodeptr = tmpnodeptr->next;
	}
	return (0);
}

/* ==========================================================================*/
int
remove_files(long req_space, char *targetdir, long ava_space, long kb_blk)
{
	/*
	 * loop:
	 *	remove oldest file
	 *	check free space
	 * until enough free space is available
	 */
	fthing * tmpnodeptr;
	struct statfs stbuf;

	tmpnodeptr = head;
	printf("Removing:\n");
	while ((ava_space < req_space) && (tmpnodeptr != (void *)0)) {
		/*
		 * loop until the amount of requested space has been made
		 * available, or until end of the linked list is hit
		 */
		head = tmpnodeptr->next;
		printf("\t%s%\n", tmpnodeptr->filename);
		unlink(tmpnodeptr->filename);
		tmpnodeptr = head;
		statfs(targetdir, &stbuf);
		ava_space = stbuf.f_bfree * kb_blk;
#ifdef BOOTDEBUG
		printf("ava_space=%ld\nreq_space=%ld\n", ava_space, req_space);
#endif
	}
	return (0);
}
