static char sccsid[] = "@(#)55	1.20.4.1  src/bos/usr/bin/df/df.c, cmdfs, bos41J, 9507A 2/8/95 13:14:34";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: df
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 *      This is a user/system utility program to print out the amount of
 *      free space on various file systems.
 *
 *      Synopsis:
 *
 *              df [-P] | [-IMitv] [-s] [-k] [filesystem ...][mount dir ...]
 *
 *      By default it prints out the free space on all mounted file systems.
 *      These are found using mntctl system call.
 *
 *      The -s flag causes df to call the filesystem helper operation
 *	to recalculate disk resources directly.  Using the disk maps
 *	and superblock.
 *      
 *      If a filesystem is not currently mounted, the "statfs"
 *      filesystem helper operation is called rather than the
 *      system call.
 *
 *      If names are specified, only those file systems will be checked.
 *      The names can be either the names of the devices or of the files
 *      onto which they are mounted.
 */
#include <stdio.h>
#include <limits.h>     
#include <sys/vmount.h>
#include <sys/statfs.h>
#include <sys/param.h>

#include <IN/AFdefs.h>
#include <IN/FSdefs.h>
#include <sys/stat.h>
#include <fshelp.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include <nl_types.h>
#include "df_msg.h"
#define MSGSTR(Num, Str) catgets(catd,MS_DF,Num,Str)
nl_catd catd;

static int DebugLevel;
#ifdef DEBUG
#define debug_printf         DebugLevel && printf
#else
#define debug_printf         0 && printf          
#endif

#define		BYTESPERKB	1024		/* output block size */

struct old_fs *get_old_fs();
static int ckfs(int nmounts, struct vmount *vmthd, char *fsname);
static void print_statfs(struct statfs statbuf);
static int ckfree(char *fsname, char *devname, char *host, char *options);

extern int fshlpr_errno;

/* old_fs's are used to table the stanza's in FSfile
 * (/etc/filesystems).  The list head is OldFsList.
 */

struct old_fs
{
  char            name[PATH_MAX];
  char            vfsname[PATH_MAX];
  char            dev[PATH_MAX];

  struct old_fs  *next;
};  
struct old_fs    *OldFsList;       /* head of old_fs list.       */
char	*FSfile	=	"/etc/filesystems";
int	Mflag=0;	/* Mounted displayed in second column.   */
int	sflag = 0;	/* -s option.  fully search free list.   */
int	dflag = 0;	/* -d option.  send debug infor to file. */
int	Iflag=0;	/* list filesystem, total KB, used, free.*/
int	iflag=0;	/* shows inode stats			 */
int	kflag=0;	/* print status using 1k units		 */
int	vflag=0;	/* List all superblock information.      */
int	Pflag=0;	/* prints stats in POSIX portable format */
char	flagstr[PATH_MAX+5];

/* MSG variables to print out correct field information */

char *p_FS, 		/* Filesystem   */
	*p_MO,		/* Mounted on   */
	*p_BLK,		/* tot blocks	*/
	*p_USED,	/* USED         */
	*p_FREE,	/* FREE space   */
	*p_pUSED,	/* % FREE space */
	*p_IUSED,	/* Inode used   */
	*p_IFREE,	/* Inodes free  */
	*p_pIUSED,	/* %Inode used  */
	*p_SMO;		/* short MO     */

   /*  The lengths of the above strings */

int l_FS, l_MO, l_BLK, l_USED,
	l_FREE, l_pUSED, l_IUSED, 
	l_IFREE, l_pIUSED, l_SMO;

static int no_header_yet = 1;


/*
 * Print the header in the correct format
 */
void
print_header()
{
	printf("%s ", p_FS);
	if (Mflag)
		printf("%s ", p_MO);
	printf("%s", p_BLK);
	if (!Pflag)
	{
		if (Iflag || vflag)
			printf(" %s", p_USED);
		printf(" %s %s", p_FREE, p_pUSED);
		if (!Iflag || vflag)
			printf(" %s", p_IUSED);
		if (vflag)
			printf(" %s", p_IFREE);
		if (!Iflag || vflag)
			printf(" %s", p_pIUSED);
		if (!Mflag)
			printf(" %s", p_SMO);
	}
	else
		printf(" %s %s %s %s", p_USED, p_FREE, p_pUSED, p_SMO);
	printf ("\n");
	no_header_yet = 0;
}



int
main( argc, argv )
int	argc;
char	*argv[];
{
	register int	a;
	int		nmounts;
	struct vmount	*vmountp;
	int	errflg=0, optc;
	extern int	optind;
	extern char	*optarg;
	int 	rc=0;

	(void) setlocale (LC_ALL,"");

	catd = catopen(MF_DF, NL_CAT_LOCALE);

	while ((optc=getopt(argc, argv, "IMPd:ikstv")) != EOF && errflg == 0)
		switch(optc) {
		case 't':
		case 'I':		/* List FS, total KB, used, free    */
			Iflag++;
			if (Pflag)
				errflg++;
			break;
		case 'M':		/* List "Mounted on" col in 2nd col */
			Mflag++;
			if (Pflag)
				errflg++;
			break;
		case 'P':
			Pflag++;
			if (Mflag || Iflag || vflag || iflag)
				errflg++;
			break;
		case 'i':		/* default case, compatability      */
			iflag++;
			if (Pflag)
				errflg++;
			break;
		case 'k':
			kflag++;
			break;
		case 's':		/* Fully verify the counts          */
			sflag++;	/* Use the helper routines.         */
			strcat (flagstr," -s ");
			break;
		case 'v':		/* Verbose output, list all fields  */
			vflag++;
			if (Pflag)
				errflg++;
			break;
		case 'd':		/* Turn debugging on and send       */
			dflag++;	/*     output to a file.            */
			strcat (flagstr,"-d ");
			strcat (flagstr,optarg);
			printf ("Debugging output sent to: %s\n",flagstr);
			break;
		case '?':
			errflg++;
		}

	if (errflg) 
	{
		fprintf(stderr, MSGSTR(USAGE,
	 "Usage: df [-P] | [-IMitv] [-k] [-s] [filesystem ...] [file ...]\n"));
		exit(1);
	}

	if ((nmounts = get_stat(&vmountp)) <= 0) {
		perror(MSGSTR(NOMNTTBL,"Cannot get mount table info"));
		exit(1);
	}

/*	Get the field lengths for each and attach them to strings. */

	p_FS = MSGSTR(FILSYS,"Filesystem   ");
	l_FS  = strlen (p_FS);

	p_MO = MSGSTR(MOUNT,"Mounted on        ");
	l_MO = strlen (p_MO);

	/*
	 *  I'm using the POSIX total blocks headers to reduce confusion
	 *  about the units we are using.
	 */
	p_BLK = kflag ? MSGSTR(BLK1024, "1024-blocks") :
		        MSGSTR(BLK512,  "512-blocks");
	l_BLK = strlen(p_BLK);

	/*
	 *  used and free must be 9 chars wide
	 */
	p_USED = MSGSTR(USED, "     Used");
	l_USED = strlen(p_USED);

	/*
	 *  do we have to do "POSIX portable format" headers?
	 */
	if (Pflag)
	{
		p_FREE = MSGSTR(AVAIL, "Available");
		p_pUSED = MSGSTR(CAPACITY, "Capacity");
	}
	else
	{
		p_FREE = sflag ? MSGSTR(FREES, "    Free*") :
				 MSGSTR(FREE,  "     Free");
		p_pUSED = MSGSTR(pUSED, "%Used");
	}
	l_FREE = strlen (p_FREE);
	l_pUSED = strlen (p_pUSED);

	p_IUSED = MSGSTR(IUSED, "   Iused");
	l_IUSED = strlen (p_IUSED);

	p_IFREE = MSGSTR(IFREE, "   Ifree");
	l_IFREE = strlen (p_IFREE);

	p_pIUSED = MSGSTR(pIUSED, "%Iused");
	l_pIUSED = strlen (p_pIUSED);

	p_SMO = MSGSTR(SMO, "Mounted on");
	l_SMO = strlen (p_SMO);

	
/* Default case is to print out the filesystems that are mounted. */

	if ((argc - optind) == 0)
		while (nmounts--)
		{
			rc += ckfree(vmt2dataptr(vmountp, VMT_STUB),
				     vmt2dataptr(vmountp, VMT_OBJECT),
				     vmt2dataptr(vmountp, VMT_HOST),
				     vmt2dataptr(vmountp, VMT_ARGS));
			vmountp = (struct vmount *)((char *)vmountp +
						    vmountp->vmt_length);
		}
	else
		for( a = optind; a < argc; a++)
		    rc += ckfs(nmounts, vmountp, argv[a]);

	exit(rc);
	/*NOTREACHED*/
}

/*
 * NAME: get_stat
 *                                                                    
 * FUNCTION: 
 * 		get_stat gathers up the mount status for this local
 *		machine	using mntctl.
 *                                                                    
 * RETURN VALUE: 
 *		< 0 for -error or > 0 for number of struct vmounts in
 *		buffer which is pointed to by pointer left at *vmountpp.
 */  
 
int
get_stat(vmountpp)
register struct vmount	**vmountpp;	/* place to tell where buffer is */
{
	int			size;
	register struct vmount	*vm;
	int			nmounts;

	/* set initial size of mntctl buffer to a MAGIC NUMBER */
	size = BUFSIZ;

	/* try the operation until ok or a fatal error */

	while (1)
	{
		if ((vm = (struct vmount *)malloc((size_t)size)) == NULL)
		{
			perror(MSGSTR(MALLOC,
				      "FATAL ERROR: get_stat malloc failed\n"));
			exit(-1);
		}

		/*
		 * perform the QUERY mntctl - if it returns > 0, that is the
		 * number of vmount structures in the buffer.  If it returns
		 * -1, an error occured.  If it returned 0, then look in
		 * first word of buffer for needed size.
		 */
		if ((nmounts = mntctl(MCTL_QUERY, size, (caddr_t)vm)) > 0) 
		{
				/* OK, got it, now return */
			*vmountpp = vm;
			return(nmounts);

		}
		else if (nmounts == 0)
		{
			/* the buffer wasn't big enough .... */
			/* .... get required buffer size */
			size = *(int *)vm;
			free((void *)vm);

		}
		else
		{
			/* some other kind of error occurred */
			free((void *)vm);
			return(-1);
		}
	}
}

#define NEXT_VMT(vmtp) \
	((vmtp) = (struct vmount *)((char *)(vmtp) + (vmtp)->vmt_length))

/*
 * NAME: ckfs
 *                                                                    
 * FUNCTION: 
 *      Look up the fsname associated with a device or the device name
 *      associated with a file system.  Once a pair is found, invoke
 *      ckfree to report on the free space on that file system.
 *      If no pair is found, check for an unmounted filesystem.
 *
 * RETURN VALUE:  1 if the check failed; 0 if good
 */  

static int
ckfs(int		nmounts,
     struct vmount	*vmthd,
     char 		*fsname)
{
	struct vmount	*vmtp;
	int		i;
	struct stat	statbuf;
	struct statfs	stfsbuf;
	int rc = 0, found = 0;

	if (sflag)
	{
		if (rc = ckunmounted(fsname))
			fprintf(stderr,
				MSGSTR(NOFS, "Cannot find file system %s\n"),
				fsname);
		return rc;
	}
	
	/* look in the mount table first */
	for (vmtp = vmthd, i = 0; i < nmounts; NEXT_VMT(vmtp), i++)
	{
		if (!strcmp(fsname, vmt2dataptr(vmtp, VMT_STUB)) ||
		    !strcmp(fsname, vmt2dataptr(vmtp, VMT_OBJECT)))
		{
			rc += ckfree(vmt2dataptr(vmtp, VMT_STUB),
				     vmt2dataptr(vmtp, VMT_OBJECT),
				     vmt2dataptr(vmtp, VMT_HOST),
				     vmt2dataptr(vmtp, VMT_ARGS));
			found++;
		}
	}
	if (found)
		return rc;
	
	/* If we didn't find it and it's a block device, try unmounted.
	 * Of course, if we can't even stat it then it can't be a device
	 * or a mount point, so give up.
	 */
	if (stat(fsname, &statbuf) < 0)
	{
		fprintf(stderr,
			MSGSTR(NOFS, "Cannot find file system %s\n"), fsname);
		return 1;
	}

	if ((statbuf.st_mode & S_IFMT) == S_IFBLK)
		if (ckunmounted(fsname) == 0)
			return 0;
	
	/*
	 * When all else fails attempt to resolve the file to its filesystem
	 * by comparing vfs number from statbuf and vmount struct.
	 */
	for (vmtp = vmthd, i = 0; i < nmounts; NEXT_VMT(vmtp), i++)
		if (statbuf.st_vfs == vmtp->vmt_vfsnumber)
			return ckfree(vmt2dataptr(vmtp, VMT_STUB),
				      vmt2dataptr(vmtp, VMT_OBJECT),
				      vmt2dataptr(vmtp, VMT_HOST),
				      vmt2dataptr(vmtp, VMT_ARGS));

	fprintf(stderr, MSGSTR(NOFS, "Cannot find file system %s\n"), fsname);
	return 1;
}

/*
 * NAME: ckfree
 *
 * FUNCTION: 
 *      This is the routine that does the real work of printing
 *      out the free space on a particular file system
 *                                                                    
 *      arguments
 *              fsname  (name file system is called)
 *              devname (name of special file)
 *                                                                    
 * RETURN VALUE: 1 if statfs failed, otherwise 0
 */  
static int
ckfree(char *fsname,
       char *devname,
       char *host,
       char *options)
{
	struct	statfs	statbuf;
	char		buf[BUFSIZ];
	char		*p;

	/*
	 * if the options include "ignore", this is an automounted
	 * entry.  don't print it (since statfs'ing it would cause
	 * it to be mounted by the automounter and that's bad)
	 *
	 */
	for (p = strtok(options, ","); p != NULL; p = strtok(NULL, ","))
		if (strcmp(p, "ignore") == 0)
			return 0;

	if (*host != 0 && strcmp(host, "-") != 0)
	{
		sprintf(buf, "%s:%s", host, devname);
		devname = buf;
	}

	if (no_header_yet)
		print_header();
	
	if (Mflag)
		printf("%-*s %-*s ", l_FS, devname, l_MO, fsname);
	else
		printf("%-*s ", l_FS, devname);

	if (statfs(fsname,&statbuf) < 0)
	{
		printf("\n");	/* finish the stdout line */
		fflush(stdout);
		sprintf(buf, "df: %s", fsname);
		perror(buf);
		fflush(stderr);	/* error msg s/b near stdout line  */
		return 1;
	}
	
	print_statfs(statbuf);
	printf(" %s\n", Mflag ? "" : fsname);
	return 0;
}


/*
 * NAME: ckunmounted
 *                                                                    
 * FUNCTION: 
 *      If the file is unmounted, we can't get the information from the statfs
 *	system call.  We need to call the statfs filesystem helper operations.
 *	Or, if the sflag is set, we go here instead of using the statfs system
 *	call.
 *                                                                    
 * RETURN VALUE:
 *		 1 	no entry found in /etc/filesystems.
 *		 0	entry found and sent to fshelper routines.
 */  

int
ckunmounted(char *fsname)
{
	struct statfs   statbuf;
	struct old_fs  *ofsp;

	if (!(ofsp = get_old_fs (fsname)))
		if (dflag)
		{
			printf ("calling fshelper anyways with: %s %s\n",
				fsname,"jfs");
			if (call_helper(fsname, &statbuf, "jfs") < 0)
				return 1;
		}
		else
			return 1;
	else
		if (call_helper (ofsp->dev, &statbuf, ofsp->vfsname) < 0)
			return 1;
  
	if (no_header_yet)
		print_header();
  
	if (Mflag)
		printf("%-*s %-*s",l_FS,ofsp->dev,l_MO, fsname);
	else
		printf("%-*s ",l_FS,ofsp->dev);

	print_statfs(statbuf);
	printf(" %s\n", Mflag ? "" : fsname);
	return 0;
}
    
	
/*
 * NAME: print_statfs
 *                                                                    
 * FUNCTION: 
 * 		Prints out the statistics, taking care not to print
 * 		meaningless values (< 0)
 *
 * RETURN VALUE: None
 */  

static void
print_statfs(struct statfs statbuf)
{
	long 	freeblks, totblks, usedblks;
	long	ipercent, percent;
	long	tinodes, ninodes, ifree;
	uint 	cfactor;
  
	/* Skip stats if values undefined.
	 */
	if (statbuf.f_blocks == -1)
	{
		printf("\n");
		return;
	}
	
	freeblks = statbuf.f_bavail;
	totblks  = statbuf.f_blocks;
	usedblks = statbuf.f_blocks - statbuf.f_bavail;

	cfactor = statbuf.f_bsize / (kflag ? 1024 : 512);
	freeblks = freeblks * cfactor;
	totblks  = totblks  * cfactor;
	usedblks = usedblks * cfactor;
	percent  = (totblks > 0) ?
		100 * (long long)(totblks - freeblks) / totblks : -1;
	if ( (100 * (long long)(totblks - freeblks) % totblks) > 0 )
		percent++;	 /* XPG4 says round up */

	ifree    = statbuf.f_ffree;
	tinodes  = statbuf.f_files;
	ninodes  = tinodes - ifree;
	ipercent = (ninodes >= 0 && tinodes > 0) ?
		100 * (long long)ninodes / tinodes  : -1;
	if ((100 * (long long)ninodes % tinodes) > 0)
		ipercent++;	/* XPG4 says round up */

	if (freeblks < 0)
		freeblks = 0;
	
	printf("%*u", l_BLK, totblks);

	if (Pflag || Iflag || vflag)
		printf(" %*u", l_USED, usedblks);	/* used blockes */
	
	printf(" %*u %*d%%", l_FREE, freeblks, l_pUSED-1, percent);

	/* This is for filesystems for which inodes don't make sense.
	 * Dashes are printed when the field isn't applicable.
	 */

	if (!Pflag && (!Iflag || vflag))
		if (tinodes >= 0)
		{
			printf(" %*u", l_IUSED, ninodes);
			if (vflag) 
				printf(" %*u", l_IFREE, ifree);
			printf(" %*d%%", l_pIUSED-1, ipercent);
		}
		else
		{
			printf(" %*s", l_IUSED , "-");
			if (vflag) 
				printf(" %*s", l_IFREE, "-");
			printf(" %*s", l_pIUSED, "- ");
		}
}

/*
 * NAME: get_old_fs
 *                                                                    
 * FUNCTION: Given a filesystem name, finds old_fs of device.
 *
 * RETURN VALUE:  pointer to a structure containing the name of device,
 *		  type, and path.  0 if nothing was found.
 */  

struct old_fs *
get_old_fs (fsname)
     char *fsname;
{
  struct old_fs *ofsp = (struct old_fs *) 0;
  
  if (makeFStab (FSfile))
  {
	fprintf (stderr,
		 MSGSTR(NOFIND,
			"Cannot find %s filesystem in /etc/filesystems.\n"),
		 fsname);
	return ((struct old_fs *) 0);
  }
  
  for (ofsp = OldFsList; ofsp; ofsp = ofsp->next)
  {
      if (!strcmp (fsname, ofsp->name) || !strcmp (fsname, ofsp->dev))
	return (ofsp);
  }

  return ((struct old_fs *) 0);
}
  
/*
 * NAME: makeFStab
 *                                                                    
 * FUNCTION: **   Tables information in FSfile into handy list of old_fs's
 *
 * Warning:
 *  uses libIN/AF... routines
 *
 * RETURN VALUE: 0 if table has been built.
 */  

int
makeFStab (filsystems)
    char *filsystems;
{
  AFILE_t          attrfile;
  ATTR_t           stanza;
  struct old_fs   *ofsp = (struct old_fs *) 0;
    
  /*
  ** no use re-tabling
  */
  if (OldFsList)
      return (0);

  /*
  ** errno is set if this fails
  */
  if (!(attrfile = AFopen (filsystems, MAXREC, MAXATR)))
      return (-1);

  while (stanza = AFnxtrec (attrfile))
  {
    if (!(ofsp = (struct old_fs *) malloc ((size_t)(sizeof(struct old_fs)))))
	return (-1);
    memset ((void *)ofsp, 0, (size_t)(sizeof (struct old_fs)));
    
    /*
    ** add to the old_fs list
    */
    if (found_any (stanza, ofsp))
    {
      ofsp->next = OldFsList;
      OldFsList  = ofsp;
    }
  }
  AFclose (attrfile);
  return (0);
}
    
/*
 * NAME: found_any
 *                                                                    
 * FUNCTION: Tables stanza if the "dev" and "vfs" keywords are specified
 *
 * RETURN VALUE: 	1 - something was found.
 *			2 - nothing found.
 */  

found_any (stanza, ofsp)
     ATTR_t  stanza;
     struct old_fs *ofsp;
{
  char    *attrval;
  int      found = 0;

  if (attrval = AFgetatr (stanza, "dev"))
  {
    strncpy (ofsp->dev, attrval, (size_t)PATH_MAX);
    found++;
  }
  if (attrval = AFgetatr (stanza, "vfs"))
  {
	found++;
	strncpy (ofsp->vfsname , attrval, (size_t)PATH_MAX);
  }

  /*
  ** only worth tabling if it has relevant keywords in it
  */
  if (found)
      strncpy (ofsp->name, stanza->AT_value, (size_t)PATH_MAX);
  
  return (found);
}      

/*
 * NAME: call_helper
 *                                                                    
 * FUNCTION: Call the file system helper routines for the corresponding
 *		filesystem's type,  possibly requesting the lists to be
 *		rebuilt first
 *
 * RETURN VALUE: The return code from op_statfs.
 */  

int
call_helper (devname, statbuf, vfsname)
     char         *devname;
     struct statfs   *statbuf;
     char           *vfsname;
{
  int  mode_flags;
  int  rc=0;
  
  /*
   ** you've got to want a lot of debugging output to
   ** cause the helper to dump core on an error
   */
  mode_flags = (DebugLevel > FSHBUG_BLOOP? FSHMOD_ERRDUMP:  0) |
               (DebugLevel?                FSHMOD_PERROR:   0) |
	       (isatty (0)?                FSHMOD_INTERACT: 0);
  
  if (rc >= 0)
  {
      rc = fshelp (devname, vfsname, FSHOP_STATFS,
		   mode_flags, DebugLevel, flagstr, (caddr_t) statbuf);
  }
  
  if (dflag)
	  printf ("return code is: %d\n fs_errno is: %d\n",rc,fshlpr_errno);

  return (rc >= 0 && fshlpr_errno == 0 ? 0: -1);
}


/*
 * NAME: print_FS
 *                                                                    
 * FUNCTION: 	Print out the linked list built from reading 
 *		/etc/filesystems.  This is only used in debugging.
 *
 * RETURN VALUE: none.
 */  

print_FS ()
{

  struct old_fs  *ptr;
	ptr = OldFsList;       /* head of old_fs list */
	printf ("printing FS: \n");
	while ( ptr )
	{
		printf ("name: %s type: %s dev: %s\n",ptr->name,
				ptr->vfsname,ptr->dev);
		ptr = ptr ->next;
	}
}

/*
 * NAME: print_stanza
 *                                                                    
 * FUNCTION: 	Print out the stanza files read in by the AF routines.
 *		This is only used in debugging.
 *
 * RETURN VALUE: none.
 */  

print_stanza(stanza)
ATTR_t  stanza;
{
	printf ("stanza: %s    %s\n ",stanza->AT_name,stanza->AT_value);
}
