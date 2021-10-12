static char sccsid[] = "@(#)45	1.24  src/bos/usr/bin/dosdir/dosdir.c, cmdpcdos, bos411, 9430C411a 7/21/94 16:30:47";
/*
 * COMPONENT_NAME: CMDPCDOS  routines to read dos floppies
 *
 * FUNCTIONS: dosdir dosdel dosread doswrite main checkargs flipslash
 *            dirprint prtentry printdate traceclst volid writefile
 *            readfile deletefile printspace diskfmt 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * Dos
 *      PC-DOS file utility -
 *      Functions:  read  - locate and copy a DOS file
 *                  write - create and fill a DOS file
 *                  del   - remove a DOS file
 *                  dir   - directory listing
 */

#include <stdio.h>
#include "pcdos.h"
#include <locale.h>
#include <doserrno.h>
#include <errno.h>


#include "dosdir_msg.h"

nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_DOSDIR, Num, Str)

static  char    aiws1[]  = "@(#)(C) Copyright IBM Corp. 1985, 1993";
static  char    aiws2[]  = "@(#)Licensed Material - Program Property of IBM";

char    buf[BUFSIZ];
char    *dosdev = "/dev/fd0";   /* (Default) Name of file system for DOS fs */
char    *from = "\\";           /* File name to copy from (or look for) */
char    *to = NULL;     /* File name to copy to */
char	*filename = NULL; /* File name to copy to */
char    ascii = 0;      /* If true, do CRLF translation, ^Z (SUB) is EOF */
char    verbose = 0;    /* If true, output in nauseating detail */
char    quiet = 0;      /* If true, suppress "Abort,Retry,Ignore" prompt */

DCB     *fd = 0;            /* File handle for DOS disk */

extern int dostrace;
int     fto = 1;        /* file handle for `to' - stdout is default */
int     ffrom = 0;      /* file handle for `from' */
int     command = 0;    /* Command function indicator */
int     dirflag = 0;    /* Action flags for directory listing */
int     freespace = 0;  /* Number of free bytes */

char *months[] = {
	"Jan","Feb","Mar","Apr","May","Jun",
	"Jul","Aug","Sep","Oct","Nov","Dec"
};

static char spaces[] = "                                      ";

/* Bits in command */
#define FN_READ  001
#define FN_WRITE 002
#define FN_DIR   004
#define FN_DEL   010

/* Bits in dirflag */
#define FLG_LONG   01
#define FLG_TREE   02
#define FLG_VOL    04
#define FLG_ALL   010
#define FLG_FDIR  020
#define FLG_XTRA  040

#define DOSBUFSIZE 1024*32

char inbuf[DOSBUFSIZE];
char outbuf[2*DOSBUFSIZE+1];  /* out is 2 * in + CTLZ space */

/*
 *  Determine command to execute, and dispatch.
 */
main(argc,argv)
int argc;
char *argv[];
{
DCB *dmount();
register i = 0;
char	cwd[100];
int exit_err;

	setlocale( LC_ALL, "");
        catd = catopen(MF_DOSDIR, NL_CAT_LOCALE);

	exit_err = 0;
        /* save the current working directory since dosinit changes it for */
	getwd( cwd);	
	if (checkargs(argc,argv) < 0) {
		exit(-1);
	}

	if(dosinit() < 0) {
		printf(MSGSTR(INITFAIL, "DOS init failed.\n"));
		exit(-1);
	}
	chdir( cwd);	/* set the current dir back to what it should be */

	if ((fd = dmount(dosdev,0,0)) == NULL)
	{       printf(MSGSTR(INVFILESYS, 
			"DOS filesystem invalid or inaccessible.\n"));
          ipc_cleanup ();
		exit(-1);
	}

	if (verbose)
		diskfmt();

	switch (command)
	{       case FN_READ:
			exit_err = readfile();
			break;

		case FN_DIR:
			if (dirflag&FLG_VOL)
				volid();
			if ((i = _DFlocate(from,fd)) == 0)
			{       printf(MSGSTR(CANTFINDFILE, "Can't locate file %s\n"),from);
				exit_err = -1;
				break;
			}
			else
			{    if (dirflag & FLG_FDIR)
			     {   if (i != PC_ROOTDIR)
				     prtentry(dir,0);
			     }
			     else
				 dirprint(0,from);
			}
			printspace();
			break;

		case FN_WRITE:
			exit_err = writefile();
			printspace();
			break;

		case FN_DEL:
			if (deletefile()) {
				printf(MSGSTR(NOTDELETED, "%s not deleted; "),from);
				exit_err = -1;
			}
			else
				printf(MSGSTR(DELETED, "%s deleted; "),from);
			printspace();
			break;
	}
	dunmount(fd);
   ipc_cleanup ();
	exit(exit_err);
}
checkargs(argc,argv)
int argc;
char *argv[];
{
	register int c= 0;
	register int errflg = 0;
	register char *oplist= 0;
	register int devchg = 0;
	register char* cmd = 0;

	if (cmd = strrchr(argv[0], '/'))
		++cmd;
	else
		cmd = argv[0];

	if (!strcmp(cmd,"dosread"))
		command = FN_READ;
	else if (!strcmp(cmd,"dosdir"))
		command = FN_DIR;
	else if (!strcmp(cmd,"doswrite"))
		command = FN_WRITE;
	else if (!strcmp(cmd,"dosdel"))
		command = FN_DEL;
	else
		return(-1);

	oplist = (command&FN_DIR) ? "ade$D:ltv" :
		 (command&FN_DEL) ? "$D:v" : "a$D:vS" ;

	while ((c = getopt(argc,argv,oplist)) != EOF)
	{       switch(c)
		{       case 'a':
				if (command&FN_DIR)
					dirflag |= FLG_ALL;
				else
					ascii++;
				break;

			case 'D':
				dosdev = optarg;
				devchg++;
				break;

			case 'd':
				dirflag |= FLG_FDIR;
				break;

			case 'e':
				dirflag |= FLG_XTRA;
				break;

			case 'l':
				dirflag |= FLG_LONG;
				break;

			case 't':
				dirflag |= FLG_TREE;
				break;

			case 'v':
				dirflag |= FLG_VOL;
				verbose++;
				break;

			case '$':
				dostrace++;
				break;

			case 'S':
				quiet++;
				break;

			default:
				errflg++;
				break;
		}
	}
	if (errflg > 0 || argc <= optind)
	{       switch (command)
		{       case FN_READ:
				fprintf(stderr,MSGSTR(RD_USAGE, 
			        "Usage: dosread [-av] [-Ddosdev] from [to]\n"));
				return(-1);
			/*	break; */

			case FN_WRITE:
				fprintf(stderr,MSGSTR(WR_USAGE, 
			       "Usage: doswrite [-av] [-Ddosdev] from to\n"));
				return(-1);
			/*	break; */

			case FN_DIR:
				if (errflg > 0)
				{       fprintf(stderr,MSGSTR(DIR_USAGE, 
				"Usage: dosdir [-adltv] [-Ddosdev] [file]\n"));
				 return(-1); }
				break;

			default:
				fprintf(stderr,MSGSTR(DEL_USAGE, 
				"Usage: dosdel [-v] [-Ddosdev] file\n"));
				return(-1);
			/*	break; */
		}
	}
	if (argc > optind)
		from = argv[optind++];
	if (argc >= optind)
		to = argv[optind];

	if (command == FN_WRITE) {
		flipslash(to);
		if ((ffrom = open(from,0)) < 0) {
			perror(from);
			return(-1);
		}
	}
	else	/* READ, DIR & DEL cases. */
		flipslash(from);
	return(0);
}
/*
 *       Changes pathname separator from AIX slash to PC-DOS backslash.
 */
flipslash(name)
register char *name;
{
register char *p;

	p = name;
	while (*p)
	{       if (*p == '/')
			*p = '\\';
		p++;
	}
}
/*
 * Display current entry if (treated as) simple file; else if coming from a
 *      directory, display everything in the directory.  If recursive, follow
 *      up on each entry which is itself a directory.
 */
dirprint(depth,name)
int depth;
char *name;
{
SRCHBLK block[1];
pc_dirent *dnext(), *t;
char path[128], curdir[128], lname[20], *p=0;

	strcpy(path,name);
	dfirst(fd, path, 0, block);
	while ( (prtentry((t=dnext(block)),depth)) != 0 ){
		if (dirflag & FLG_TREE){     /* Recursive scan called for */
			if ((t->df_attr & FIL_HDD) && (t->df_use != '.')){
				strncpy(lname,spaces,12);
				strncpy(lname,t,8);
				p = strchr(lname,' ');
				if (t->df_ext[0] != ' '){
					*p++ = '.';
					p = strncpy(p,t->df_ext,3);
				}
				p = strchr(lname,' ');
				*p = '\0';
				strcpy(curdir,path);
				if (strlen(curdir)>1)
					strcat(curdir,"\\");
				strcat(curdir,lname);
				dirprint(depth+1,curdir);
			}
		}
	}
}
/*
 *  Print current directory entry, according to flag.
 *      spaced over by depth * 2 spaces.
 */
prtentry(t,depth)
int  depth;
register pc_dirent *t;
{
register char *p = 0;
char name[13];
register int attr = 0;

	if (t == NULL)
		return(0);
	attr = t->df_attr;
	if ((attr & FIL_VOL) == 0)                      /* Ignore VOLUME ID */
	{       if ( !(dirflag & FLG_ALL))
			if (attr & (FIL_HDF | FIL_SYS))
				return(1);
		strncpy(name,spaces,12);
		strncpy(name,t,8);
		p = strchr(name,' ');
		if (t->df_ext[0] != ' ')
		{       *p++ = '.';
			p = strncpy(p,t->df_ext,3);
		}
		if ((t->df_use == '.') && (verbose==0))
			return(1);
		printf("%.*s",depth<<1,spaces);
		if (!(dirflag & (FLG_LONG | FLG_XTRA )))
                {
                        char *spc;
                        
                        if (spc = strchr(name, ' '))
                        {
                                *spc = '\0';
                        }
                        printf("%s\n", name);
                        return(1);
                }
		printf("%-12s",name);
		if (t->df_use == '.')         /* print name only for . & .. */
		{       printf("\n");
			return(1);
		}
		if (dirflag & FLG_LONG)
		{       printf("  %8ld",
				  t->df_siz0
			       | (t->df_siz1 << 8)
			       | (t->df_siz2 << 16)
			       | (t->df_siz3 << 24));
			printdate(t->df_time[0]|(t->df_time[1]<<8),
				  t->df_time[2]|(t->df_time[3]<<8));
			if (attr)
			{       printf("  [");
				if (attr&FIL_RO)
					printf("R");
				if (attr&FIL_HDF)
					printf("H");
				if (attr&FIL_HDD)
					printf("D");
				if (attr&FIL_SYS)
					printf("S");
				if (attr&FIL_AR)
					printf("A");
				printf("]");
			}
			if (dirflag&FLG_XTRA)
			{       printf(MSGSTR(CLUSTERS, "\n  Clusters: "));
				traceclst(t->df_lcl | (t->df_hcl << 8));
			}
		}
		printf("\n");
	}
	return(1);
}
printdate(time,date)
register unsigned time,date;
{
	int x;

	x = ((date >> 5) & 0xf) - 1;
	 printf("   %s %2d %d   %2.2d:%2.2d:%2.2d",
			MSGSTR(MONTH+x, months[x]),
			date & 0x1f,
			((date >> 9) & 0x1f) + 1980,
			(time >> 11) & 0x1f,
			(time >> 5) & 0x3f,
			((time << 1) & 0x3f) + 1);
}
/*
 *  Trace FAT entries, starting from initial cluster.  Print as found.
 */
traceclst(start)
int start;
{
register int cluster;
register int i=2;

	cluster=start;
	while ((cluster&0x0fff) < 0x0ff0)
	{       printf("%3.3x", cluster);
		if (++i&0xF)
			printf(" ");
		else
			printf("\n    ");

		cluster = getnextcluster(fd,cluster);
	}
}

/*
 * Find and print volume id in the root directory.
 */
volid()
{
register int i = 0;
SRCHBLK block[1];
pc_dirent *t = 0, *dnext();

	if (dfirst(fd,"\\",0,block)<0) return;
	while ((t = dnext(block)) != NULL)
		if (t->df_attr & FIL_VOL)
		{       printf("%11.11s\n",&(t->df_use));
			break;
		}
}
/*
 *  write a file on a PC-DOS disk
 */
writefile()
{
FCB            *fildes=0, *dcreate();
register char  *fromptr=0, *toptr=0;
register int   outsize=0, insize=0;

	if (to == NULL || *to == '\0')
	{   fprintf(stderr,MSGSTR(SPECIFY, "Specify PC-DOS file name, please\n"));
	    return(-1);
	}
	toptr = to;
	fromptr = toptr;
	while( *toptr = toupper(*fromptr++))  toptr++;

	filename = malloc(strlen(to) + 1);
	strcpy (filename,to);
	if ((fildes = dcreate(fd,to,0)) == NULL)
	{
	    if (doserrno == DE_RFULL)
		    fprintf(stderr,MSGSTR(CANTWRITEDIR, "Cannot make directory entry %1$s %2$d\n"),filename,doserrno);
	    else
		    fprintf(stderr,MSGSTR(DOSCANTOPEN, "Unable to open %1$s %2$d\n"),
			filename,doserrno);
	    return(-1);
	}
	while ((insize = read(ffrom, inbuf, sizeof(inbuf))) > 0) {
		fromptr = inbuf;
		toptr = outbuf;
		if (ascii) {
			char *endptr = fromptr + insize;

			while (fromptr < endptr)
				if ((*toptr++ = *fromptr++) == 0x0a) {
					toptr[-1] = 0x0d;
					*toptr++ = 0x0a;
				}
			outsize = toptr - outbuf;
			toptr = outbuf;
		} else {
			toptr = inbuf;
			outsize = insize;
		}
		if (dwrite(fildes, toptr, outsize) != outsize) {
			if (doserrno == ENOSPC) {
				dremove(fd, to);
				printf(MSGSTR(WRT_FAIL1, "Insufficient disk space.\n"));
				}
			else printf(MSGSTR(WRT_FAIL, " DOS write failure on %s\n"),to);
			return(-1);
		}
	}
	if (ascii)
	{       outbuf[0] = 0x1a;
		if (dwrite(fildes,outbuf,1) != 1)
		{       printf(MSGSTR(WRT_FAIL, " DOS write failure on %s\n"),to);
			return(-1);
		}
	}
	if (dclose(fildes) < 0)
	{       printf(MSGSTR(CLOSE_FAIL, " DOS close failure on %s\n"),to);
		return(-1);
	}
	return(0);
}
/*
 *  read a file from a PC-DOS disk
 */
readfile()
{
FCB            *fildes, *dopen();
register char  *fromptr=0, *toptr=0, lastchar=0;
register int   newsize=0, size=0;
char           inbuf[512], chbuf[1];

	toptr = from;
	fromptr = toptr;
	while( *toptr = toupper(*fromptr++))  toptr++;
	if ((fildes = dopen(fd,from,DO_RDONLY)) == NULL)
	{   printf(MSGSTR(CANTOPEN, "Unable to open %s\n"),from);
	    return(-1);
	}
	else if (to && *to)
		if ((fto = creat(to,0666)) < 0) 
		{
			perror(to);
			return(-1);
		}
	while ((size = dread(fildes,inbuf,sizeof(inbuf))) > 0)
	{       if (ascii)
		{       newsize = 0;
			fromptr = inbuf;
			toptr = inbuf;
			while(size)
			{       if ( *fromptr == 0x1a)           /* CTL-Z ? */
					break;                /* done if so */
				if ((*fromptr == 0xa)               /* LF ? */
				    && (lastchar == 0xd))
					{       *(--toptr) = 0xa;
						lastchar = 0;
						fromptr++;
						toptr++;
					}
				else
				{       lastchar = *fromptr;
					newsize++;
					*toptr++ = *fromptr++;
				}
				size--;
				if ((size==0)&&(lastchar==0xd))
				{       *chbuf  = '\0';
					if (dread(fildes,chbuf,1) == 1)
					{       if (*chbuf==0xa)
						{       *(--toptr) = 0xa;
							toptr++;
							lastchar = 0;
						}
						else
						    dlseek(fildes, -1, 1 );
					}
				}
			}
			size = newsize;                 /* new buffer count */
		}
		if (write(fto,inbuf,size) != size)
		{       perror(MSGSTR(AIXWRITE, "write"));
			return(-1);
		}
	}
	close(fto);
	return(0);
}
/*
 *              Delete a file from a PC-DOS disk
 *                   returns zero for success
 */
deletefile()
{
register int i;

	if ((i = _DFlocate(from,fd)) == 0)
	{       printf(MSGSTR(CANTFINDFILE, "Can't locate file %s\n"),from);
		return(-1);
	}
	if (dir->df_attr & FIL_HDD)
		return(drmdir(fd,from));
	else
		return(dremove(fd,from));
}
/*
 *              Calculate and print freespace on PC-DOS drive.
 */
printspace()
{
register i;

	freespace = 0;
	for (i=2; i<fd->ccount; i++)
		if (fd->fat_ptr[i].cluster == 0)
			freespace += fd->clsize;
	printf(MSGSTR(FREESPACE, "Free space: %d bytes\n"),freespace);
}
/*
 *              Print format information for PC-DOS drive.
 */
diskfmt()
{
    if (fd->bpb.pb_descr >= 0xf9 | fd->bpb.pb_descr == 0xf0) /* its a floppy */
    {
	if ((fd->bpb.pb_descr==0xfc)||(fd->bpb.pb_descr==0xfe))
		printf(MSGSTR(ONESIDED, "one-sided"));
	else
		printf(MSGSTR(TWOSIDED, "two-sided"));

	if (fd->bpb.pb_descr>0xfd)
	    printf(MSGSTR(EIGHTSECTOR, ", eight sector floppy disk\n"));
	else
	    printf(MSGSTR(NINESECTOR, ", nine sector floppy disk\n"));
    }
			/* its a known device */
    if (fd->bpb.pb_descr >= 0xf8 | fd->bpb.pb_descr == 0xf0)  
    {
	printf(MSGSTR(DISKDATA, 
    "Disk data: %1$d bytes per sector, %2$d sectors per track, %3$d heads\n"),
		fd->bpb.pb_secsiz, fd->bpb.pb_sectrk,fd->bpb.pb_headcnt);
	printf(MSGSTR(DISKPART, 
	"Disk Partition size: %1$d sectors, %2$d reserved, %3$d hidden\n"),
		fd->bpb.pb_ptnsiz, fd->bpb.pb_res,fd->bpb.pb_hidsec);
	printf(MSGSTR(CLUSTSIZ, "Cluster size: %d sectors\n"),
		fd->bpb.pb_csize);
	printf(MSGSTR(FATSIZE, "FAT size: %1$d sectors, FAT count %2$d\n"),
		  fd->bpb.pb_fatsiz, fd->bpb.pb_fatcnt);
	printf(MSGSTR(DIRSIZE, "Directory size: %d entries, max\n"),
		fd->bpb.pb_dirsiz);
	printf(MSGSTR(ASSUMING, "Assuming first FAT begins on sector %d\n"),
		  fd->bpb.pb_res + fd->bpb.pb_hidsec + 1);
    }
    else
	printf(MSGSTR(DISKTYPE, 
	"Can't identify disk type: descriptor from FAT is %d\n"),
			fd->bpb.pb_descr);
}
