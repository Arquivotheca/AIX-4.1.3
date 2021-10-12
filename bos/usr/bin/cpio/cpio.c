static char sccsid[] = "@(#)06	1.61  src/bos/usr/bin/cpio/cpio.c, cmdarch, bos412, 9446B 11/11/94 21:53:55";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: cpio
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
 */

/*	cpio	COMPILE:	cc -O cpio.c -s -i -o cpio 
	cpio -- copy file collections
 */

#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/access.h>
#include <ctype.h>
#include <locale.h>
#include <fnmatch.h>
#include <nl_types.h>
#include "cpio_msg.h"

#define EQ(x,y)	(strcmp(x,y)==0)
#define MKSHORT(v,lv) {U.l=1L;if(U.c[0]) U.l=lv,v[0]=U.s[1],v[1]=U.s[0]; else U.l=lv,v[0]=U.s[0],v[1]=U.s[1];}
#define MAGIC	070707		/* cpio magic number */
#define IN	1		/* copy in */
#define OUT	2		/* copy out */
#define PASS	3		/* direct copy */
#define HDRSIZE	(Hdr.h_name - (char *)&Hdr)	/* header size minus filename field */
#define CHARS	76		/* ASCII header size minus filename field */
#define BUFSIZE 512		/* In u370, can't use BUFSIZ nor BSIZE */
#define CPIOBSZ 4096		/* file read/write */
#define MSGSTR(Num,Str) catgets(catd,MS_CPIO,Num,Str)

static nl_catd catd;

static struct	stat	Statb, Xstatb;
static struct utimbuf tb;
static struct linkmap {
	ino_t ino;
	dev_t dev;
	ulong_t vfs;
	ushort newino;
	ushort newdev;
	struct linkmap *next;
};				/* structure to save linked file data */
static struct linkmap *lnkmap = NULL;

	/* Cpio header format */
static struct header {
	short	h_magic,
		h_dev;
	ushort	h_ino,
		h_mode,
		h_uid,
		h_gid;
	short	h_nlink,
		h_rdev,
		h_mtime[2],
		h_namesize,
		h_filesize[2];
	char	h_name[PATH_MAX];
} Hdr;

static char		minorhi[4];		/* for high byte of minor, if any */
static unsigned	Bufsize = BUFSIZE;		/* default record size */
static short	Buf[CPIOBSZ/2], *Dbuf;
static char	BBuf[CPIOBSZ], *Cbuf, linkname[PATH_MAX];
static int	Wct, Wc;
static short	*Wp;
static char	*Cp;
static char timbuf[26];

static short	Option = 0,
	Dir = 0,
	Uncond = 0,
	Link = 0,
	Rename = 0,
	Toc = 0,
	Verbose = 0,
	Select = 0,
	Mod_time = 0,
	Acc_time = 0,
	Cflag = 0,
	fflag = 0,
	Swap = 0,
	byteswap = 0,
	bothswap = 0,
	halfswap = 0;

static int	Ifile,
	Ofile,
	Input = 0,
	Output = 1;
static long	Blocks = 0,
	Longfile,
	Longtime;

static char	Fullname[PATH_MAX],
	Name[PATH_MAX];
static int	Pathend;
static int usrmask;
static int error=0;	/* we need to tell the user if any errors occurred */

static FILE	*Rtty,
	*Wtty;

static char	*Pattern[100];
static char	Strhdr[500];
static char	*Chdr = Strhdr;
static dev_t	Dev;
static short	Uid,
	Gid,
	A_directory,
	A_special,
	Filetype = S_IFMT;

static short	 encode();
static dev_t	 decode();

static union { long l; short s[2]; char c[4]; } U;

static long 
mklong(v)
short v[];
{
	U.l = 1;
	if(U.c[0])
		U.s[0] = v[1], U.s[1] = v[0];
	else
		U.s[0] = v[0], U.s[1] = v[1];
	return (U.l);
}

main(argc, argv)
char **argv;
{
	register ct;
 	long	filesz;
 	long	bufsz = 0;
 	unsigned long maxint=(unsigned long)((1L << (sizeof(int) * 8)) - 1);
	register char *fullp;
	register i, j;
	int ans;
	struct stat sb;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_CPIO, NL_CAT_LOCALE);

	signal(SIGSYS, SIG_IGN);
	if(*argv[1] != '-')
		usage();
	if(argc > 2) {			/* Ignore/erase "--" argument */
	    if (EQ(argv[2], "--")) {
		for(i=2; i<argc; i++)
		    argv[i] = argv[i+1];
		argc--;
	    }
	}
	Uid = getuid();
	usrmask = umask((mode_t)0);
	umask(usrmask);
	Gid = getgid();
	Pattern[0] = "*";

	while(*++argv[1]) {
		switch(*argv[1]) {
		case 'a':		/* reset access time */
			Acc_time++;
			break;
		case 'B':		/* change record size to 5120 bytes */
 			if(bufsz)
				fprintf(stderr,
				MSGSTR(COBS,"Conflicting options, 'B' superceding 'C'\n"));
			Bufsize = 5120;
			break;
 		case 'C':
 			argv[1]++;
 			while(*argv[1] == ' ') argv[1]++;
 			if((*argv[1] >= '0') && (*argv[1] <= '9'))
 				bufsz = *(argv[1]++) - '0';
 			else usage();
 			while ((*argv[1] >= '0') && (*argv[1] <= '9')) {	
 				bufsz = (bufsz * 10) + 
 						(*(argv[1]++) - '0');
 				if(bufsz << 9 > maxint) {
 				  fprintf(stderr,
				  MSGSTR(BSTL,"Block size too large\n"));
 				  exit(2);
 				}
 			}
 			if(Bufsize != BUFSIZE)
 				fprintf(stderr,
				MSGSTR(COCS,"Conflicting options, 'C' superceding previous option\n"));
 			Bufsize = bufsz << 9;
 			while(*argv[1] == ' ') argv[1]++;
 			argv[1]--;
 			break;
		case 'i':
			Option = IN;
			if(argc > 2 ) {	/* save patterns, if any */
				for(i = 0; (i+2) < argc; ++i)
					Pattern[i] = argv[i+2];

			}
			break;
		case 'f':	/* do not consider patterns in cmd line */
			fflag++;
			break;
		case 'o':
			if(argc != 2)
				usage();
			Option = OUT;
			break;
		case 'p':
			if(argc != 3)
				usage();
			if(access(argv[2], 2) == -1) {
accerr:
				fprintf(stderr,
				MSGSTR(CWIF,"cannot write in <%s>\n"), argv[2]);
				exit(2);
			}
			strcpy(Fullname, argv[2]);	/* destination directory */
			strcat(Fullname, "/");
			lstat(Fullname, &Xstatb);
			if((Xstatb.st_mode&S_IFMT) != S_IFDIR)
				goto accerr;
			Option = PASS;
			Dev = Xstatb.st_dev;
			break;
		case 'c':		/* ASCII header */
			Cflag++;
			break;
		case 'd':		/* create directories when needed */
			Dir++;
			break;
		case 'l':		/* link files, when necessary */
			Link++;
			break;
		case 'm':		/* retain mod time */
			Mod_time++;
			break;
		case 'r':		/* rename files interactively */
			Rename++;
			Rtty = fopen("/dev/tty", "r");
			Wtty = fopen("/dev/tty", "w");
			if(Rtty==NULL || Wtty==NULL) {
				fprintf(stderr,
				MSGSTR(CRNDT,"Cannot rename (/dev/tty missing)\n"));
				exit(2);
			}
			break;
		case 'S':		/* swap halfwords */
			halfswap++;
			Swap++;
			break;
		case 's':		/* swap bytes */
			byteswap++;
			Swap++;
			break;
		case 'b':
			bothswap++;
			Swap++;
			break;
		case 't':		/* table of contents */
			Toc++;
			break;
		case 'u':		/* copy unconditionally */
			Uncond++;
			break;
		case 'v':		/* verbose table of contents */
			Verbose++;
			break;
		case '6':		/* for old, sixth-edition files */
			Filetype = 060000;
			break;
		default:
			usage();
		}
	}
	if(!Option) {
		fprintf(stderr,
		MSGSTR(OMIL,"Options must include either -o, -i, or -p\n"));
		exit(2);
	}

	if(Option == PASS) {
		if(Rename) {
			fprintf(stderr,
			MSGSTR(PREX,"Pass and Rename cannot be used together\n"));
			exit(2);
		}
 		if(Bufsize != BUFSIZE) {
 			fprintf(stderr,
			MSGSTR(BCIRR,"`B' or 'C' option is irrelevant with the '-p' option\n"));
			Bufsize = BUFSIZE;
		}

	}else  {
		bufsz = Bufsize;
		while(((Cp = malloc(Bufsize)) == NULL) &&
				Bufsize > 512) Bufsize -= CPIOBSZ;
		if(Cp == NULL) {
			fprintf(stderr,
			MSGSTR(NOMLC,"Unable to malloc buffer space\n"));
			exit(2);
		}
		if(bufsz != Bufsize)
			fprintf(stderr,
			MSGSTR(BSR,"blocksize reduced to %u\n"),Bufsize);
		if(Cflag)
		    Cbuf = Cp;
		else 
		{
		    Wp = Dbuf = (short *)Cp;
		    Cp = NULL;
		}
	}
	Wct = Bufsize >> 1;
	Wc = Bufsize;

	switch(Option) {

	case OUT:		/* get filename, copy header and file out */
		while(getname()) {
			if( mklong(Hdr.h_filesize) == 0L) {
			   if( Cflag )
				writehdr(Chdr,CHARS+Hdr.h_namesize);
			   else
				bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
			   if(Verbose)
				fprintf(stderr, "%s\n", Hdr.h_name);
			   continue;
			}
			if ((Hdr.h_mode & S_IFMT) == S_IFLNK ) {

			  if((j = readlink(Hdr.h_name,linkname,PATH_MAX)) < 0)
				{
				printf(MSGSTR(CRSL,"Cannot read symbolic link\n"));
				perror(Hdr.h_name);
				error++;
				}
			  else {
				if ( Cflag )
					writehdr(Chdr,CHARS+Hdr.h_namesize);
				else
					bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
				linkname[j] = '\0';
				Cflag ? writehdr(linkname,j): bwrite(linkname,j);
				if(Verbose)
					fprintf(stderr, "%s\n", Hdr.h_name);
			       }
			  continue;
			}
			if((Ifile = open(Hdr.h_name, 0)) < 0) {
				fprintf(stderr,MSGSTR(IHDRQ,"<%s> ?\n"), Hdr.h_name);
				error++;
				continue;
			}
			if ( Cflag )
				writehdr(Chdr,CHARS+Hdr.h_namesize);
			else
				bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
			for(filesz=mklong(Hdr.h_filesize); filesz>0; filesz-= CPIOBSZ){
				ct = filesz>CPIOBSZ ? CPIOBSZ: filesz;
				if(read(Ifile, Cflag ? BBuf: (char *)Buf, ct) == -1) {
					fprintf(stderr,MSGSTR(CRS,"Cannot read %s\n"), Hdr.h_name);
					perror("cpio");
					error++;
					continue;
				}
				Cflag ? writehdr(BBuf,ct): bwrite(Buf,ct);
			}
			close(Ifile);
			if(Acc_time){
				tb.actime = Statb.st_atime;
				tb.modtime = Statb.st_mtime;
				utime(Hdr.h_name, &tb);
			};
			if(Verbose)
				fprintf(stderr,"%s\n", Hdr.h_name);
		}

	/* copy trailer, after all files have been copied */
		strcpy(Hdr.h_name, "TRAILER!!!");
		Hdr.h_magic = MAGIC;
		MKSHORT(Hdr.h_filesize, 0L);
		Hdr.h_namesize = strlen("TRAILER!!!") + 1;
		if ( Cflag )  {
			bintochar(0L);
			writehdr(Chdr,CHARS+Hdr.h_namesize);
		}
		else
			bwrite(&Hdr, HDRSIZE+Hdr.h_namesize);
		Cflag ? writehdr(Cbuf, Bufsize): bwrite(Dbuf, Bufsize);
		if (close(Output) < 0) {
			fprintf (stderr,
				MSGSTR( WOUTPUT, "Can't write output\n"));
			perror("cpio ");
			error++;
		}
		break;

	case IN:
		pwd();
		while(gethdr()) {

			Ofile = ckname(Hdr.h_name) ? openout(Hdr.h_name): 0;
			for(filesz=mklong(Hdr.h_filesize); filesz>0; filesz-= CPIOBSZ){
				ct = filesz>CPIOBSZ ? CPIOBSZ: filesz;
				Cflag ? readhdr(BBuf,ct): bread(Buf, ct);
				if(Ofile) {
					if(Swap)
					   Cflag ? swap(BBuf,ct): swap(Buf,ct);
					if(write(Ofile, Cflag ? BBuf: (char *)Buf, ct) == -1) {
						fprintf(stderr,MSGSTR(CWF,"Cannot write %s\n"), Hdr.h_name);
						perror("cpio ");
						error++;
					 	continue;
					}
				}
			}
			if (Ofile) {
			/* only do the following if not asked for table 
			 * of content
			 */
			if (close(Ofile))
			   perror("cpio ");   /* D49215 */
			if ((Hdr.h_mode & S_IFMT) == S_IFLNK ) {
				if ((Ofile = open(Hdr.h_name,0)) < 0) {
					fprintf(stderr,MSGSTR(COPN, "Cannot Open %s\n"), Hdr.h_name);
					error++;
					continue;
				}
				if(read(Ofile,linkname,ct) == -1) {
					fprintf(stderr,MSGSTR(CRS,"Cannot Read %s\n"),Hdr.h_name);
					perror("cpio");
					error++;
					continue;
				}
				close(Ofile);
				unlink(Hdr.h_name);
				linkname[ct] = '\0';
				if ((symlink(linkname,Hdr.h_name)) < 0) {
					printf(MSGSTR(CCLN," Symlink: cannot create\n"));
					perror(Hdr.h_name);
					error++;
				}
			}
			}
			if(Ofile) {
				if(Uid) {
					i = (Hdr.h_mode & (0777000 | (~usrmask & 0777)));
				}
				else 
					i = Hdr.h_mode;
				if ((Hdr.h_mode & S_IFMT) != S_IFLNK ) {
					if(chmod(Hdr.h_name, (mode_t)i) < 0) {
						fprintf(stderr,
						MSGSTR(CCM,"Cannot chmod <%s> (errno:%d)\n"), Hdr.h_name, errno);
						error++;
					}
				}
				set_time(Hdr.h_name, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
			}
				/*	code added to ignore special files except for FIFO's if not root.
				*/
			if(A_special && (Hdr.h_mode & S_IFMT != S_IFIFO) && Uid)
			{
				fprintf ( stderr , MSGSTR ( IGNSPECIAL ,
				"cpio: special file <%s> ignored\n" ) ,
					Hdr.h_name ) ;
				continue ;
			}

			if(!Select)
				continue;
			if(Verbose)
				if(Toc)
					pentry(Hdr.h_name);
				else
					puts(Hdr.h_name);
			else if(Toc)
				puts(Hdr.h_name);
		}
		break;

	case PASS:		/* move files around */
		fullp = Fullname + strlen(Fullname);

		while(getname()) {
			if(!ckname(Hdr.h_name))
				continue;
			i = 0;
			while(Hdr.h_name[i] == '/')
				i++;
			strcpy(fullp, &(Hdr.h_name[i]));

			if(Link && !A_directory && Dev == Statb.st_dev)  {
				if(link(Hdr.h_name, Fullname) < 0) { /* missing dir.? */
					if( errno == EEXIST )
					{
						/*
						* File already exists,
						* see which is newer or
						* obey -u flag.
						*/


						sb.st_mtime = 0;
						lstat(Fullname,&sb);

						if( !Uncond && (sb.st_mtime
						> Statb.st_mtime) )
						{
							fprintf(stderr,
							MSGSTR(NEW,"<%s> newer\n"),
							Fullname);

							continue;
						}
						if(sb.st_ino == Statb.st_ino)
							goto ckverbose;
					}

					/* Unlink and try again. */

					if (Uncond || (sb.st_mtime < Statb.st_mtime))
						unlink(Fullname);
					missdir(Fullname);
					if(link(Hdr.h_name, Fullname) < 0) {
						fprintf(stderr,
						 MSGSTR(CLSTS,"Cannot link <%s> & <%s>\n"),
						 Hdr.h_name, Fullname);
						error++;
						continue;
					}
				}

/* try creating (only twice) */
				ans = 0;
				do {
					if(link(Hdr.h_name, Fullname) < 0) { /* missing dir.? */
						unlink(Fullname);
						ans += 1;
					}else {
						ans = 0;
						break;
					}
				}while(ans < 2 && missdir(Fullname) == 0);
				if(ans == 1) {
					fprintf(stderr,MSGSTR(CCD,
						"Cannot create directory for <%s> (errno:%d)\n"), Fullname, errno);
					exit(2);
				}else if(ans == 2) {
					fprintf(stderr,
					MSGSTR(CLSTS,"Cannot link <%s> & <%s>\n"), Hdr.h_name, Fullname);
					exit(2);
				}

				if( !Link ) {
					if(Uid) {
						i = (Statb.st_mode & (0777000 | (~usrmask & 0777)));
					}
					else
						i = Statb.st_mode;
					if(chmod(Hdr.h_name, (mode_t)i) < 0) {
						fprintf(stderr,
						MSGSTR(CCM,"Cannot chmod <%s> (errno:%d)\n"), Hdr.h_name, errno);
						error++;
					}
				}
				set_time(Hdr.h_name, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
				goto ckverbose;
			} else if ((Statb.st_mode & S_IFMT) == S_IFLNK ) {
				if((j = readlink(Hdr.h_name,linkname,PATH_MAX)) < 0) {
					printf(MSGSTR(CRSL,"Cannot read symbolic link\n"));
					perror(Hdr.h_name);
					error++;
					continue;
				}
				else {
					linkname[j] = '\0';
					if ((symlink(linkname,Fullname)) < 0) {
						if( errno == EEXIST )
						{
							sb.st_mtime = 0;
							lstat(Fullname,&sb);

							if( !Uncond && (sb.st_mtime
							> Statb.st_mtime) )
							{
								fprintf(stderr,
								MSGSTR(NEW,"<%s> newer\n"),
								Fullname);

								continue;
							}
						}

						/* Unlink and try again. */

						if (Uncond || (sb.st_mtime < Statb.st_mtime))
							unlink(Fullname);
						missdir(Fullname);
						if ((symlink(linkname,Fullname)) < 0) {
							printf(MSGSTR(CCLN," Symlink: cannot create\n"));
							perror(Fullname);
							error++;
							continue;
						}
					}
					if(Verbose)
						puts(Fullname);
					continue;
				}
			}

			if(!(Ofile = openout(Fullname)))
				continue;
			if((Ifile = open(Hdr.h_name, 0)) < 0) {
				fprintf(stderr,MSGSTR(IHDRQ,"<%s> ?\n"), Hdr.h_name);
				close(Ofile);
				error++;
				continue;
			}
			filesz = Statb.st_size;
			for(; filesz > 0; filesz -= CPIOBSZ) {
				ct = filesz>CPIOBSZ ? CPIOBSZ: filesz;
				if(read(Ifile, Buf, ct) == -1) {
					fprintf(stderr,MSGSTR(CRS,"Cannot read %s\n"), Hdr.h_name);
					perror("cpio");
					error++;
					break;
				}
				if(Ofile)
					if(write(Ofile, Buf, ct) == -1) {
						fprintf(stderr,MSGSTR(CWF,"Cannot write %s\n"), Hdr.h_name);
						perror("cpio ");
						error++;
						break;
					}
				Blocks += ((ct + (BUFSIZE - 1)) / BUFSIZE);
			}
			close(Ifile);
			if(Acc_time){
				tb.actime = Statb.st_atime;
				tb.modtime = Statb.st_mtime;
				utime(Hdr.h_name, &tb);
			}
			if(Ofile) {
				close(Ofile);
				if(Uid) {
					i = (Statb.st_mode & (0777000 | (~usrmask & 0777)));
				}
				else 
					i = Statb.st_mode;
				if(chmod(Fullname, (mode_t)i) < 0) {
					fprintf(stderr,
					MSGSTR(CCM,"Cannot chmod <%s> (errno:%d)\n"), Fullname, errno);
					error++;
				}
				set_time(Fullname, Statb.st_atime, mklong(Hdr.h_mtime));
ckverbose:
				if(Verbose)
					puts(Fullname);
			}
		}
	}
	/* print number of blocks actually copied */
	   fprintf(stderr,
	   MSGSTR(NOBLKCOP,"%ld blocks\n"), Blocks * (Bufsize>>9));
	if (error)
		exit(2);
	else
		exit(0);
}
static
usage()
{
 	fprintf(stderr,MSGSTR(USAGE1,"Usage: cpio -o[acv][B|C<value>]\n"));
 	fprintf(stderr,MSGSTR(USAGE2,"       cpio -i[bcdmrstuvfS6][B|C<value>] [pattern ...]\n"));
	fprintf(stderr,MSGSTR(USAGE3,"       cpio -p[adlmuv] directory\n"));
	exit(2);
}

static
getname()		/* get file name, get info for header */
{
	register char *namep = Name;
	register ushort ftype;
	long tlong;
	struct linkmap *lp;
	static ushort tmpino=1;
	static ulong tmpdev=1;

	for(;;) {
		if(gets(namep) == NULL)
			return (0);
		if(*namep == '.' && namep[1] == '/')
			namep += 2;
		strcpy(Hdr.h_name, namep);
		if(lstat(namep, &Statb) < 0) {
			fprintf(stderr,
			MSGSTR(IHDRQS,"< %s > ?\n"), Hdr.h_name);
			error++;
			continue;
		}
		/* make sure that unsigned long stat values (ie. ino,  *
		 * mode,gid,uid) do not exceed an unsigned short       */
		if (Option == OUT && !check_stat_values(Statb,Hdr.h_name))
			continue;

		ftype = Statb.st_mode & Filetype;
		A_directory = (ftype == S_IFDIR);
		A_special = (ftype == S_IFBLK)
			|| (ftype == S_IFCHR)
			|| (ftype == S_IFSOCK)
			|| (ftype == S_IFIFO);

			/*	This if statement will not allow special
			**	files to be archived. This is due to the
			**	change in the major and minor numbers
			**	from 8 bit numbers to 16 bit numbers.
			**	None of the code has been deleted which
			**	handles the archiving of special files.
			**  FIFO's will be archived
			*/
		if ( A_special  && (ftype != S_IFIFO) && Uid)	/* skip special files if not root */
		{
			fprintf ( stderr , MSGSTR ( NOSPECIAL ,
				"cpio: special file <%s> not archived\n" ) ,
				namep ) ;
			error++;
			continue ;
		}

		Hdr.h_magic = MAGIC;
		Hdr.h_namesize = strlen(Hdr.h_name) + 1;
		Hdr.h_uid = (ushort)Statb.st_uid;
		Hdr.h_gid = (ushort)Statb.st_gid;
		/* assign ino and dev internally since we only have ushorts */
		/* to work with and must insure that those values are unique */
		Hdr.h_ino = tmpino;
		Hdr.h_dev = tmpdev; 
		Hdr.h_mode = (ushort)Statb.st_mode;
		MKSHORT(Hdr.h_mtime, Statb.st_mtime);
		Hdr.h_nlink = Statb.st_nlink;
		tlong = (((Hdr.h_mode&S_IFMT) == S_IFREG) || ((Hdr.h_mode&S_IFMT) == S_IFLNK)) ? Statb.st_size: 0L;
		MKSHORT(Hdr.h_filesize, tlong);
		Hdr.h_rdev = encode(Statb.st_rdev, minorhi);
		strcpy(Hdr.h_name + Hdr.h_namesize, minorhi);

		/* use this mapping algorithm to allocate unique inode and */
		/* dev numbers for linked files only!!!                    */
		if ((Statb.st_nlink > 1) && (Option == OUT)) {
			for( lp = lnkmap; lp; lp=lp->next )
				if( lp->ino == Statb.st_ino
				&&  lp->dev == Statb.st_dev &&  lp->vfs == Statb.st_vfs ) {
					Hdr.h_ino = lp->newino;
					Hdr.h_dev = lp->newdev;
					break;
				}
			if( lp == NULL )	/* no match found */
			{
				if((lp = (struct linkmap *) malloc(sizeof(struct linkmap))) == NULL)
				{
					/* blow up */
					fprintf(stderr,
							MSGSTR(NOMLC,"Unable to malloc link structure space\n"));
					exit(2);
				}
				/* Initialize lp->ino,dev,vfs,newino,newdev */
				lp->ino = Statb.st_ino;
				lp->dev = Statb.st_dev;
				lp->vfs = Statb.st_vfs;
				lp->newino = tmpino;
				lp->newdev = tmpdev;
				/* put on front of the list */
				lp->next = lnkmap;
				lnkmap = lp;
			}
		}
		if (tmpino == USHRT_MAX) {
			tmpino = 1;
			if (tmpdev == USHRT_MAX) {
				fprintf(stderr,
						MSGSTR(FILES,"Too many files for cpio to handle.\n"));
				exit(2);
			}
			else
				tmpdev++;
		}
		else
			tmpino++;
		if( Cflag )
			bintochar(tlong);
		else
			Hdr.h_namesize += sizeof(minorhi);
		return (1);
	}
}


static
check_stat_values(statb,name)
struct stat statb;
char *name;
{
	if (statb.st_uid > USHRT_MAX) {
		fprintf(stderr, 
			MSGSTR(UID, "%s not archived: uid greater than 65535.\n"), name);
		error++;
		return(0);
	}
	if (statb.st_gid > USHRT_MAX) {
		fprintf(stderr, 
			MSGSTR(GID, "%s not archived: gid greater than 65535.\n"), name);
		error++;
		return(0);
	}
	return(1);
}

static
bintochar(t)		/* ASCII header write */
long t;
{
	sprintf(Chdr,"%.6o%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.11lo%.6ho%.11lo%s",
		MAGIC,Hdr.h_dev & 00000177777,Hdr.h_ino,Hdr.h_mode,
		Hdr.h_uid,Hdr.h_gid,Hdr.h_nlink, Hdr.h_rdev & 00000177777,
		Statb.st_mtime,(short)strlen(Hdr.h_name)+1,t,Hdr.h_name);
}

static
chartobin()		/* ASCII header read */
{
	sscanf(Chdr,"%6ho%6ho%6ho%6ho%6ho%6ho%6ho%6ho%11lo%6ho%11lo",
		&Hdr.h_magic,&Hdr.h_dev,&Hdr.h_ino,&Hdr.h_mode,&Hdr.h_uid,
		&Hdr.h_gid,&Hdr.h_nlink,&Hdr.h_rdev,&Longtime,&Hdr.h_namesize,
		&Longfile);
	MKSHORT(Hdr.h_filesize, Longfile);
	MKSHORT(Hdr.h_mtime, Longtime);
}

static
gethdr()		/* get file headers */
{
	char c;
	short found;
	int bytecount=0;
	register ushort ftype;
	int phase_startmsg_flag = FALSE, count, move;
	int out_of_phase;
	char magic_array[CHARS];

	if (Cflag) {			      /* ascii header */
		readhdr(magic_array,6);       /* read first 6 bytes into array*/
		if((magic_array[0]=='0') &&   /* test magic number */
		   (magic_array[1]=='7') &&
		   (magic_array[2]=='0') &&
		   (magic_array[3]=='7') &&
		   (magic_array[4]=='0') &&
		   (magic_array[5]=='7')) { /* if good read rest of hdr and go*/
			for(count = 6; count < CHARS; count++) {
				readhdr(&c,sizeof(c));
				magic_array[count]=c;
			}
			strncpy(Chdr,magic_array,CHARS);
               		chartobin();
			out_of_phase = FALSE;
		}
		else out_of_phase = TRUE;
	}
	else {					/* non-ascii hdr */
		bread(&Hdr.h_magic,2 );
		if (Hdr.h_magic == MAGIC) {	/* test magic number */
			bread(&Hdr.h_dev, 2);   /* if true read rest of hdr */
			bread(&Hdr.h_ino, 2);
			bread(&Hdr.h_mode, 2);
			bread(&Hdr.h_uid, 2);
			bread(&Hdr.h_gid, 2);
			bread(&Hdr.h_nlink, 2);
			bread(&Hdr.h_rdev, 2);
			bread(&Hdr.h_mtime[0], 2);
			bread(&Hdr.h_mtime[1], 2);
			bread(&Hdr.h_namesize, 2);
			bread(&Hdr.h_filesize[0], 2);
			bread(&Hdr.h_filesize[1], 2);
			out_of_phase = FALSE;
		}
		else out_of_phase = TRUE;
	}
			
	/* if out of phase (magic number test fails) then read one byte 
	   at a time(ascii header) or 2 bytes at a time(non-ascii) and 
	   test for the magic number */

	if(out_of_phase) {
               	fprintf(stderr,
			MSGSTR(PHASE,"\n\nOut of phase!\ncpio attempting to continue...\n"));
		phase_startmsg_flag = TRUE;
                found = FALSE;
                do {			/* loop thru data */
                        if (Cflag) {
				if((magic_array[0]=='0') && /* recheck magic */
				   (magic_array[1]=='7') &&
				   (magic_array[2]=='0') &&
				   (magic_array[3]=='7') &&
				   (magic_array[4]=='0') &&
				   (magic_array[5]=='7')) {
					found = TRUE;
				}
				else {     /* move one byte into magic array */
					for(move=0;move<5;move++){
						magic_array[move] = magic_array[move+1];
					}
                        		readhdr(&c, sizeof(c));
					magic_array[5]=c;
                                	bytecount++;
				}
			}
			else {
				bread(&Hdr.h_magic, 2); /* recheck header */
				if (Hdr.h_magic == MAGIC) found = TRUE;
				else bytecount+=2;
			}
                } while(found == FALSE);

		/* magic number found, now read rest of the header */

		if (Cflag) {	
			for(count = 6; count < CHARS; count++) {
				readhdr(&c,sizeof(c));
				magic_array[count]=c;
			}
			strncpy(Chdr,magic_array,CHARS);
                	chartobin();
		}
		else {
			bread(&Hdr.h_dev, 2);
			bread(&Hdr.h_ino, 2);
			bread(&Hdr.h_mode, 2);
			bread(&Hdr.h_uid, 2);
			bread(&Hdr.h_gid, 2);
			bread(&Hdr.h_nlink, 2);
			bread(&Hdr.h_rdev, 2);
			bread(&Hdr.h_mtime[0], 2);
			bread(&Hdr.h_mtime[1], 2);
			bread(&Hdr.h_namesize, 2);
			bread(&Hdr.h_filesize[0], 2);
			bread(&Hdr.h_filesize[1], 2);
		}
	}

	/* cpio records now back in phase */

	if (phase_startmsg_flag) {
		fprintf(stderr,
			MSGSTR(PHEND,"skipping %d bytes to get back in phase!\n One or more files lost and the previous file is possibly corrupt\n\n"),bytecount);
	}

	if(Cflag)
		readhdr(Hdr.h_name, Hdr.h_namesize);
	else
	{
		bread(Hdr.h_name, Hdr.h_namesize);
		if (Hdr.h_namesize == strlen(Hdr.h_name) + 1 + sizeof(minorhi))
		{
		    Hdr.h_namesize -= sizeof(minorhi);
		    sscanf (Hdr.h_name + Hdr.h_namesize, "%3s", minorhi);
		}
		else if (Hdr.h_namesize != strlen(Hdr.h_name) + 1)
		{
		    fprintf(stderr, 
		    MSGSTR(LOFDM,"Length of file name doesn't match expected length.\n"));
		    exit(2);
		}
		else
		    minorhi[0] = '\0';
	}
	if(EQ(Hdr.h_name, "TRAILER!!!"))
		return (0);
	ftype = Hdr.h_mode & Filetype;
	A_directory = (ftype == S_IFDIR);
	A_special =(ftype == S_IFBLK)
		|| (ftype == S_IFCHR)
		|| (ftype == S_IFSOCK)
		|| (ftype == S_IFIFO);
	return (1);
}

/* check filenames with patterns given on cmd line */
static
ckname(namep) 
register char *namep;
{
	char tmpname[PATH_MAX];

	Select=1;

	if(fflag ^ !nmatch(namep,Pattern)) {
		Select = 0;
		return (0);
	}
	if(Rename && !A_directory) {	/* rename interactively */
		fprintf(Wtty,
		MSGSTR(RPRPT,"Rename <%s>\n"), namep);
		fflush(Wtty);
		fgets(tmpname, PATH_MAX, Rtty);
		if(feof(Rtty))
			exit(2);
		if (tmpname[strlen(tmpname) - 1] == '\n')
			tmpname[strlen(tmpname) - 1] = '\0';
			    /* if user enters "." then keep old name. */
		if(!EQ(tmpname, ".")) {
			strcpy(namep, tmpname);
			if(EQ(namep, "")) {
				printf(MSGSTR(SKIP,"Skipped\n"));
				return (0);
			}
		}
	}
	return (!Toc);
}

static
openout(namep)	/* open files for writing, set all necessary info */
register char *namep;
{
	register f;
	register char *np;
	int ans;
	uid_t tmp_uid;
	gid_t tmp_gid;
	mode_t tmp_mode;

    /* use these tmp values so that the -p option will use unsigned longs from the Statb structure */
	if (Option==PASS) {
	   tmp_uid = Statb.st_uid;
	   tmp_gid = Statb.st_gid;
	   tmp_mode = Statb.st_mode;
	}
	else {
	   tmp_uid = Hdr.h_uid;
	   tmp_gid = Hdr.h_gid;
	   tmp_mode = Hdr.h_mode;
	}
	if(!strncmp(namep, "./", 2))
		namep += 2;
	np = namep;
	if(A_directory) {
		if(Rename
		|| EQ(namep, ".")
		|| EQ(namep, ".."))	/* do not consider . or .. files */
			return (0);
		if(lstat(namep, &Xstatb) == -1) {

			if (!Dir)	{
				fprintf(stderr,
				MSGSTR(USE_D,"Use `-d' option to copy <%s>\n"),Hdr.h_name);
				error++;
				return (0);
			}
/* try creating (only twice) */
 			missdir (namep) ;	/*  check for missing dirs  */
 			if(makdir(namep) != 0) {
				fprintf(stderr,
				MSGSTR(CCD,"Cannot create directory for <%s> (errno:%d)\n"), namep, errno);
				error++;
				return(0);
			}
		}

ret:
		if(Uid) {
				f = (tmp_mode & (0777000 | (~usrmask & 0777)));
		}
		else 
				f = tmp_mode;
		if(chmod(namep, (mode_t)f) < 0) {
			fprintf(stderr,
			MSGSTR(CCM,"Cannot chmod <%s> (errno:%d)\n"), namep, errno);
			error++;
		}
		if(Uid == 0)
			if(chown(namep, tmp_uid, tmp_gid) < 0) {
				fprintf(stderr,
				MSGSTR(CCO,"Cannot chown <%s> (errno:%d)\n"), namep, errno);
				error++;
			}
		return (0);
	}
	if(Hdr.h_nlink > 1)
		if(!postml(namep, np))
			return (0);
	if(lstat(namep, &Xstatb) == 0) {
		if(Uncond && !((!(Xstatb.st_mode & S_IWRITE) || A_special) && (Uid != 0))) {
			if(unlink(namep) < 0) {
				fprintf(stderr,
				MSGSTR(CUL,"cannot unlink current <%s> (errno:%d)\n"), namep, errno);
				error++;
			}
		}
		if(!Uncond && (mklong(Hdr.h_mtime) <= Xstatb.st_mtime)) {
		/* There's a newer version of file on destination */
			fprintf(stderr,MSGSTR(CFN,"current <%s> newer or same age\n"), np);
			return (0);
		}
	}
	if(Option == PASS
	&& Statb.st_ino == Xstatb.st_ino
	&& Statb.st_dev == Xstatb.st_dev) {

	/* This could happen under the DS environment because
	 * Hdr.h_dev is only a short, so we need to
	 * lstat the source file, and compare the device
	 * field as longs, where the upper 16 bits contain the
	 * connection id.  They should differ there for different
	 * files.
	 */
	 struct stat Pstatb;

		if(lstat(Hdr.h_name, &Pstatb) == -1) {
			fprintf(stderr,MSGSTR(IHDRQS,"< %s > ?\n"), Hdr.h_name);
			error++;
		}else {
			if (Pstatb.st_dev == Xstatb.st_dev) {
				fprintf(stderr,
				MSGSTR(ATPFTS,"Attempt to pass file to self!\n"));
				exit(2);
			}
		}
	}
	if(A_special) {
		if((tmp_mode & Filetype) == S_IFIFO) {
			Hdr.h_rdev = 0;
			minorhi[0] = '\0';
		}

/* try creating (only twice) */
		ans = 0;
		do {
			if (mknod(namep, tmp_mode, decode(Hdr.h_rdev, minorhi)) < 0) {
				ans += 1;
			}else {
				ans = 0;
				break;
			}
		}while(ans < 2 && missdir(np) == 0);
		if(ans == 1) {
			fprintf(stderr,
			MSGSTR(CCD,"Cannot create directory for <%s> (errno:%d)\n"), namep, errno);
			error++;
			return(0);
		}else if(ans == 2) {
			fprintf(stderr,
			MSGSTR(CNMN,"Cannot mknod <%s> (errno:%d)\n"), namep, errno);
			error++;
			return(0);
		}
		if(Option == PASS && Verbose)
			 puts(namep);
		goto ret;
	}

/* try creating (only twice) */
	ans = 0;
	do {
		if((f = creat(namep, tmp_mode)) < 0) {
			ans += 1;
		}else {
			ans = 0;
			break;
		}
	}while(ans < 2 && missdir(np) == 0);
	if(ans == 1) {
		fprintf(stderr,
		MSGSTR(CCD,"Cannot create directory for <%s> (errno:%d)\n"), namep, errno);
		error++;
		return(0);
	}else if(ans == 2) {
		fprintf(stderr,
		MSGSTR(CCF,"Cannot create <%s> (errno:%d)\n"), namep, errno);
		error++;
		return(0);
	}

	if(Uid == 0)
		if(chown(namep, tmp_uid, tmp_gid) < 0) {
			fprintf(stderr,
			MSGSTR(CCO,"Cannot chown <%s> (errno:%d)\n"), namep, errno);
			error++;
		}
	return (f);
}

static
bread(b, c)
register c;
register short *b;
{
	static nleft = 0;
	static short *ip;
	register int rv;
	register short *p = ip;
	register int in;

	c = (c+1)>>1;
	while(c--) {
		if(nleft == 0) {
			in = 0;
			while((rv=read(Input, &(((char *)Dbuf)[in]), Bufsize - in)) != Bufsize - in) {
				if(rv == 0)
					errno = 0;
				if(rv <= 0) {
					Input = chgreel(0, Input);
					continue;
				}
				in += rv;
				nleft += (rv >> 1);
				if((rv % 2) || (rv == 1)) {
					read(Input, &(((char *)Dbuf)[in++]),1);
					++nleft;
				}
			}
			nleft += (rv >> 1);
			p = Dbuf;
			++Blocks;
		}
		*b++ = *p++;
		--nleft;
	}
	ip = p;
}

static
readhdr(b, c)
register c;
register char *b;
{
	static nleft = 0;
	static char *ip;
	register int rv;
	register char *p = ip;
	register int in;

	while(c--)  {
		if(nleft == 0) {
			in = 0;
			while((rv=read(Input, &(((char *)Cbuf)[in]), Bufsize - in)) != Bufsize - in) {
				if(rv == 0)
					errno = 0;
				if(rv <= 0) {
					Input = chgreel(0, Input);
					continue;
				}
				in += rv;
				nleft += rv;
			}
			nleft += rv;
			p = Cbuf;
			++Blocks;
		}
		*b++ = *p++;
		--nleft;
	}
	ip = p;
}

static
bwrite(rp, c)
register short *rp;
register c;
{
	short		*wp = Wp;
	char		*buf_adr = (char *) Dbuf;
	int		wr_size = Bufsize;
	int		wr ;

	c = (c+1) >> 1;
	while(c--) {
		if(!Wct) {
again:
			wr = write (Output, buf_adr, wr_size) ;
			if (wr < 0) {
				Output = chgreel(1, Output);
				goto again;
			}
			else
				if ( wr != wr_size )
				{
					wr_size -= wr ;
					buf_adr += wr ;
					goto again ;
				}
			Wct = Bufsize >> 1;
			wp = Dbuf;
			++Blocks;
		}
		*wp++ = *rp++;
		--Wct;
	}
	Wp = wp;
}

static
writehdr(rp, c)
register char *rp;
register c;
{
	char		*cp = Cp;
	char		*buf_adr = (char *) Cbuf;
	int		wr_size = Bufsize;
	int		wr ;

	while(c--)  {
		if(!Wc)  {
again:
			wr = write(Output, buf_adr, wr_size) ;
			if( wr < 0)  {
				Output = chgreel(1,Output);
				goto again;
			}
			else
				if ( wr != wr_size )
				{
					wr_size -= wr ;
					buf_adr += wr ;
					goto again ;
				}
			Wc = Bufsize;
			cp = Cbuf;
			++Blocks;
		}
		*cp++ = *rp++;
		--Wc;
	}
	Cp = cp;
}

static
postml(namep, np)		/* linking funtion */
register char *namep, *np;
{
	register i;
	static struct ml_st {
		dev_t	m_dev;
		ino_t	m_ino;
		struct  ml_st *m_next;
		char	m_name[2];
	} *mlhead = NULL, *ml;
	char *mlp;
	int ans;
	dev_t tmp_dev;
	ino_t tmp_ino;

	if (Option == PASS) {
		tmp_dev = (dev_t)Statb.st_dev;
		tmp_ino = (ino_t)Statb.st_ino;
	}
	else {
		tmp_dev = (dev_t)Hdr.h_dev;
		tmp_ino = (ino_t)Hdr.h_ino;
	}

	for(ml = mlhead; ml; ml = ml->m_next) {
		if ((ml->m_ino==tmp_ino) && (ml->m_dev==tmp_dev)) {
			if (Verbose)
				printf(MSGSTR(FLTF,"%s linked to %s\n"),
				ml->m_name, np);
			unlink(namep);
			if((Option == IN) && (ml->m_name[0] != '/')) {
				Fullname[Pathend] = '\0';
				strcat(Fullname, ml->m_name);
				mlp = Fullname;
			}
			mlp = ml->m_name;

/* try linking (only twice) */
			ans = 0;
			do {
				if(link(mlp, namep) < 0) {
					ans += 1;
				}else {
					ans = 0;
					break;
				}
			}while(ans < 2 && missdir(np) == 0);
			if(ans == 1) {
				fprintf(stderr,
				MSGSTR(CCD,"Cannot create directory for <%s> (errno:%d)\n"), np, errno);
				error++;
				return(0);
			}else if(ans == 2) {
				fprintf(stderr,MSGSTR(CLSTS,"Cannot link <%s> & <%s>.\n"), ml->m_name, np);
				error++;
				return(0);
			}

			set_time(namep, mklong(Hdr.h_mtime), mklong(Hdr.h_mtime));
			return (0);
		}
	}
	/* If We've made it here, then no match has been found, and a */
	/* New structure needs to be allocated.                       */
	ml = (struct ml_st *) malloc(strlen(np) + 2 + sizeof(struct ml_st));
	if (ml == NULL) {
		static int first=1;

		if(first) {
			fprintf(stderr, MSGSTR(NMFL,"No memory for links\n"));
			error++;
		}
		first = 0;
		return (1);
	}
	ml->m_dev = tmp_dev;
	ml->m_ino = tmp_ino;
	strcpy(ml->m_name, np);
	/* put on front of the list */
	ml->m_next = mlhead;
	mlhead = ml;
	return (1);
}

static
pentry(namep)		/* print verbose table of contents */
register char *namep;
{

	static short lastid = -1;
#include <pwd.h>
	static struct passwd *pw;
	static char tbuf[32];

	printf("%-7o", Hdr.h_mode & 0177777);
	if(lastid == Hdr.h_uid)
		printf("%-8s ", pw->pw_name);
	else {
		setpwent();
		if(pw = getpwuid((uid_t)Hdr.h_uid)) {
			printf("%-8s ", pw->pw_name);
			lastid = Hdr.h_uid;
		} else {
			printf("%-6d", Hdr.h_uid);
			lastid = -1;
		}
	}
	printf("%7ld ", mklong(Hdr.h_filesize));
	U.l = mklong(Hdr.h_mtime);
	strftime(timbuf,(size_t)26,"%sD %T %Y",localtime((long *)&U.l));  
	printf("%s %s\n",timbuf, namep);  
}

		/* pattern matching functions */
static
nmatch(s, pat)
char *s, **pat;
{
	if(EQ(*pat, "*"))
		return (1);
	while(*pat) {
		if ((fnmatch(*pat,s,FNM_QUOTE) == 0) || 
			(**pat == '!' && (fnmatch(*pat+1,s,FNM_QUOTE) == FNM_NOMATCH)))
			return (1);
		++pat;
	}
	return (0);
}

static
makdir(namep)		/* make needed directories */
register char *namep;
{

	if ( 0 > mkdir ( namep , (mode_t)0777 ))
		if ( errno != EEXIST ) {	/*  ignore error when	*/
			perror ( "cpio" ) ;	/*  directory exists	*/
			error++;
			return (1) ;
		}
	return (0) ;
}

static
swap(buf, ct)		/* swap halfwords, bytes or both */
register ct;
register char *buf;
{
	register char c;
	register union swp { long	longw; short	shortv[2]; char charv[4]; } *pbuf;
	int savect, n, i;
	char *savebuf;
	short cc;

	savect = ct;	savebuf = buf;
	if(byteswap || bothswap) {
		if (ct % 2) buf[ct] = 0;
		ct = (ct + 1) / 2;
		while (ct--) {
			c = *buf;
			*buf = *(buf + 1);
			*(buf + 1) = c;
			buf += 2;
		}
		if (bothswap) {
			ct = savect;
			pbuf = (union swp *)savebuf;
			if (n = ct % sizeof(union swp)) {
				if(n % 2)
					for(i = ct + 1; i <= ct + (sizeof(union swp) - n); i++) 
						pbuf->charv[i] = 0;
				else
					for (i = ct; i < ct + (sizeof(union swp) - n); i++) 
						pbuf->charv[i] = 0;
			}
			ct = (ct + (sizeof(union swp) -1)) / sizeof(union swp);
			while(ct--) {
				cc = pbuf->shortv[0];
				pbuf->shortv[0] = pbuf->shortv[1];
				pbuf->shortv[1] = cc;
				++pbuf;
			}
		}
	}
	else if (halfswap) {
		pbuf = (union swp *)buf;
		if (n = ct % sizeof(union swp))
			for (i = ct; i < ct + (sizeof(union swp) - n); i++) pbuf->charv[i] = 0;
		ct = (ct + (sizeof(union swp) -1)) / sizeof(union swp);
		while (ct--) {
			cc = pbuf->shortv[0];
			pbuf->shortv[0] = pbuf->shortv[1];
			pbuf->shortv[1] = cc;
			++pbuf;
		}
	}
}
static
set_time(namep, atime, mtime)	/* set access and modification times */
register *namep;
long atime, mtime;
{
	struct utimbuf timevec;

	if(!Mod_time)
		return;
	timevec.actime = atime;
	timevec.modtime = mtime;
	utime((char *)namep, &timevec);
}
static
chgreel(x, fl)
{
	register f;
	int endofmedia;
	char str[22];
	FILE *devtty;
	struct stat statb;
	struct devinfo devinfo;

	fstat(fl, &statb);
	/* At end of media, tape and diskette devices return: */
	/* read:   read() == 0,  errno == 0     */
	/* write:  write() < 0,  errno == ENXIO */
	endofmedia = (x ? ENXIO : 0);

	if (errno == endofmedia) {
		if (((statb.st_mode&S_IFMT) == S_IFCHR) &&
		    (ioctl(fl, IOCINFO, &devinfo) > -1)) {
			if ((devinfo.devtype == DD_TAPE) || (devinfo.devtype == DD_SCTAPE)) {
				/*  end of tape for /dev/rmt*  */
				fprintf ( stderr, MSGSTR (NEXTTAPE,
					"\007cpio: End of tape.  Load next tape\n" ));
			} else {
				/*  end of device for /dev/rfd*  */
				fprintf ( stderr, MSGSTR (NEXTDISK,
					"\007cpio: End of diskette.  Insert next diskette\n" ));
			}
		} else {
			if(x){
				fprintf (stderr, MSGSTR( WOUTPUT,
					"Can't write output\n"));
				perror("cpio ");
			}
			else
				fprintf ( stderr, MSGSTR( WINPUT,
					"Can't read input\n"));
			error++;
		}
	} else switch ( errno ) {
		case EFAULT :
			fprintf(stderr,
				MSGSTR(ERRNO,"errno: %d, "), EFAULT);
			if(x)
				fprintf(stderr,MSGSTR(CBSOUTPUT,
					"Check output block size\n"));
			else
				fprintf(stderr,MSGSTR(CBSINPUT,
					"Check input block size\n"));
			exit(2);

		default :
			/* if encountered end of media during read */
			/* don't set error */
			if(x){
				fprintf (stderr, MSGSTR( WOUTPUT,
					"Can't write output\n"));
				error++;
				perror("cpio ");
			}
			else
				fprintf ( stderr, MSGSTR( WINPUT,
					"Can't read input\n"));
			break ;
	}

	if((statb.st_mode&S_IFMT) != S_IFCHR)
		exit(2);
	if (close(fl) < 0 && x) {
		fprintf (stderr, MSGSTR( WOUTPUT, "Can't write output\n"));
		perror("cpio ");
		error++;
		exit(2);
	}

again:
	fprintf(stderr, MSGSTR( GOON,
		"If you want to go on, type device/file name when ready\n"));
	devtty = fopen("/dev/tty", "r");
	fgets(str, 20, devtty);
	str[strlen(str) - 1] = '\0';
	if(!*str)
		exit(2);
	if((f = open(str, x ? 1: 0)) < 0) {
		fprintf(stderr,
		MSGSTR( COPN, "Cannot Open %s\n"), str );
		goto again;
	}
	return (f);
}
static
missdir(namep)
register char *namep;
{
	register char *np;
	register ct = 2;

	for(np = namep; *np; ++np)
		if(*np == '/') {
			if(np == namep) continue;	/* skip over 'root slash' */
			*np = '\0';
			if(lstat(namep, &Xstatb) == -1) {
				if(Dir) {
					if((ct = makdir(namep)) != 0) {
						*np = '/';
						return(ct);
					}
				}else {
					fprintf(stderr,
					MSGSTR(MISD,"missing 'd' option\n"));
					*np = '/';
					error++;
					return (-1);
				}
			}
			*np = '/';
		}
	if (ct == 2) ct = 0;		/* the file already exists */
	return (ct);
}

static
pwd()		/* get working directory */
{
	char	*buf ;
	buf = getcwd ( Fullname , sizeof (Fullname)) ;
	Pathend = strlen(Fullname);
	Fullname[Pathend - 1] = '/';
}


/*
 *  Encode the maj/min device numbers to a "machine-independent" form.
 */
static short
encode(dev, spare)
dev_t dev;
char *spare;
{
	int minhi;
	int min;
	int maj;
	short temp;

	min = minor(dev);
	maj = major(dev);
	minhi = ((min & 0xff00) >> 8);
	temp= (maj << 8) | (min & 0xff);
	sprintf (spare, "%.3ho", minhi);
	return (temp);
}

/*
 *  Decode the maj/min device numbers from a "machine-independent" form.
 */
static dev_t
decode(dev, spare)
short dev;
char *spare;
{
	short minhi = 0;
	int min;
	int maj;

	if (Cflag)
		*spare = '\0';

	sscanf (spare, "%3ho", &minhi);
	min = ((minhi << 8) & 0xff00) | (dev & 0xff);
	maj = (dev >> 8) & 0xff;

	return (makedev (maj, min));
}
