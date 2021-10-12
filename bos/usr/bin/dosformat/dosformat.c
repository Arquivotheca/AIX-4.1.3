static char sccsid[] = "@(#)69	1.6  src/bos/usr/bin/dosformat/dosformat.c, cmdpcdos, bos411, 9428A410j 7/7/94 13:56:00";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dosformat
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

#include <stdio.h>
#include <dos.h>
#include <fcntl.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/fd.h>
#include <errno.h>
#include <time.h>
#include <locale.h>

/****************************************************
  GENERAL PURPOSE DEFS
 ****************************************************/

/*

/* Disk parameters
 */
struct dkparm {
    int   devtype;	/* drive type (hard disk or floppy) */
    int   bytpsec;	/* bytes per sector */
    int   secptrk;	/* sectors per track */
    int   trkpcyl;	/* tracks per cylinder */
    int   secpclu;	/* sectors per cluster */
    int   numcyls;	/* total number of cylinders */
    int   fattype;	/* FAT type: 12 or 16 bit */
    int   fatcode;	/* device code in FAT table */
    int   secpfat;	/* number of sectors in FAT table */
    int   rootsiz;	/* number of sectors in root directory */
    int   bootoff;	/* sector offset to boot sector */
    char *bootsec;	/* contents of boot sector */
    int   numsecs;	/* total number of sectors */
    int   badclus;	/* number of bad clusters */
    int   fat1off;	/* sector offset to first FAT */
    int   fat2off;	/* sector offset to second FAT */
    int   rootoff;	/* sector offset to root directory */
    int   dataoff;	/* sector offset to data area */
};
#define BOOT_SEC_SIZE   32

char boot80[BOOT_SEC_SIZE]={   /* boot sector for 5 1/4 1.2 meg disk */
0353, 0052, 0220, 0111, 0102, 0115, 0040, 0101, 
0111, 0130, 0063, 0000, 0002, 0001, 0001, 0000,
0002, 0340, 0000, 0140, 0011, 0371, 0007, 0000, 
0017, 0000, 0002, 0000, 0000, 0000, 0000, 0000
};


char boot40_9_2[BOOT_SEC_SIZE]={  /* boot sector for 5 1/4 360k  disk */
0xEB,0x2A,0x90,0x49,0x42,0x4D,0x20,0x41,
0x49,0x58,0x33,0x00,0x02,0x02,0x01,0x00,
0x02,0x70,0x00,0xD0,0x02,0xFD,0x02,0x00,
0x09,0x00,0x02,0x00,0x00,0x00,0x00,0x00
};

char boot80_3_3[BOOT_SEC_SIZE]={  /* boot sector for 3 1/2 2.88 meg disk */
0xEB,0x3C,0x90,0x49,0x42,0x4D,0x20,0x41,
0x49,0x58,0x33,0x00,0x02,0x02,0x01,0x00,
0x02,0xF0,0x00,0x80,0x16,0xF0,0x09,0x00,
0x24,0x00,0x02,0x00,0x00,0x00,0x00,0x00,
};

char boot80_3_2[BOOT_SEC_SIZE]={  /* boot sector for 3 1/2 1.44 meg disk */
0xEB,0x34,0x90,0x49,0x42,0x4D,0x20,0x41,
0x49,0x58,0x33,0x00,0x02,0x01,0x01,0x00,
0x02,0xE0,0x00,0x40,0x0B,0xF0,0x09,0x00,
0x12,0x00,0x02,0x00,0x00,0x00,0x00,0x00,
};

char boot80_3_1[BOOT_SEC_SIZE]={  /* boot sector for 3 1/2 720k disk */
0xEB,0x3C,0x90,0x49,0x42,0x4D,0x20,0x41,
0x49,0x58,0x33,0x00,0x02,0x02,0x01,0x00,
0x02,0x70,0x00,0xA0,0x05,0xF9,0x03,0x00,
0x09,0x00,0x02,0x00,0x00,0x00,0x00,0x00,
};

/* Device types
 */
#define FLOPPY80	1	/* floppy, 5 1/4 hi-density */
#define FLOPPY40_9_2	2	/* floppy, 5 1/4 lo-density */
#define FLOPPY80_3_3    3       /* floppy, 3 1/2 extended-density 2.88M */
#define FLOPPY80_3_2    4       /* floppy, 3 1/2 hi-density */
#define FLOPPY80_3_1    5       /* floppy, 3 1/2 low-denisty */

/* 
 *   FAT info
 */
#define FAT12		1	/* FAT has 12 bit entries */
#define FAT16		4	/* FAT has 16 bit entries */
#define FAT12SIZE	1.5	/* size of 12-bit FAT entry (in bytes) */
#define FAT16SIZE	2	/* size of 16-bit FAT entry (in bytes) */
#define BAD_CLUSTER	0xfff7	/* FAT code for bad cluster */
#define MAX_FATSIZE	64	/* max # sectors in a FAT */

#define DOS_DIRSIZ	32	/* # bytes in DOS directory entry */

/* 
 *    Error codes
 */
#define ERR_BAD_TRACK0	(-1)	/* unable to read/write track 0 */
#define ERR_DRIVE_BUSY	(-2)	/* non-sharable device already in use */
#define ERR_DRIVE_INVAL	(-3)	/* drive specification invalid */
#define ERR_GENERIC	(-4)	/* generic error */
/*#define ERR_INCOMPAT	(-5)	/* cmd line arg not applicable to hard disk */
#define ERR_INVAL_PARM	(-6)	/* unknown command line arg */
#define ERR_NOMEM	(-7)	/* 'malloc' failed */
#define ERR_NOSUPPORT	(-8)	/* command line option not supported */
#define ERR_NOT_READY	(-9)	/* no diskette in device; door not closed */
#define ERR_WRPROTECT	(-10)	/* write-protected diskette */


/* 
 *    Miscellaneous
 */
#define TRUE		1
#define FALSE		0
#define EVEN(x)		((x) % 2 == 0)
char *sect_alloc(), _doscdisk();

/****************************************************
  FLOPPY - SPECIFIC DEFS
 ****************************************************/

#define FDFILLER	0xF6	/* filler byte output by format operation */
#define FD_SECTSIZE	2	/* FDIOCFORMAT code for sector size *

/* 
 *   Debugging defs
 */
#ifdef	DEBUG
#undef TRACE
int  dostrace;
#define TRACE(expr)	if (dostrace) expr
#else
#define TRACE(expr)	if (0) expr
#endif	DEBUG

/****************************************************
  GLOBALS
 ****************************************************/



char *sectbuf, *fatbuf;
char *label;
char vollabel, lowdensity;
struct dkparm *dkp;
char  *devname;
int devfd;

/****************************************************
  FLOPPY & HARD DISK PARAMETER TABLE
   (from DOS technical reference manual)
 ****************************************************/
struct dkparm dktbl[] =
{
   { 0       ,      -1, -1,-1,-1, -1,    -1, 0xF8, -1, -1, -1, 0, },
     /* 5.25 inch 1.2meg  floppy */
   { FLOPPY80,     512, 15, 2, 1, 80, FAT12, 0xF9,  7, 14,  0, boot80},
     /* 5.25 inch 360k floppy */
   { FLOPPY40_9_2, 512,  9, 2, 2, 40, FAT12, 0xFD,  2,  7,  0, boot40_9_2},
     /* 3.5 inch 2.88meg floppy */
   { FLOPPY80_3_3, 512, 36, 2, 2, 80, FAT12, 0xF0,  9, 15,  0, boot80_3_3,},
     /* 3.5 inch 1.44meg floppy */
   { FLOPPY80_3_2, 512, 18, 2, 1, 80, FAT12, 0xF0,  9, 14,  0, boot80_3_2,},
     /* 3.5 inch 720k floppy */
   { FLOPPY80_3_1, 512, 9, 2, 2, 80, FAT12, 0xF9,  3, 7,  0, boot80_3_1,}
};

/****************************************************
  MAIN
 ****************************************************/

main(argc, argv)
int	argc;
char	*argv[];
{


    int another = TRUE;

    setlocale( LC_ALL, "");

    if (get_args(argc, argv)  !=  0)
	exit(1);

    while (another == TRUE)
    {

    if ( open_drive() ==  0)
	exit(1);

    if (get_drive_parms()  !=  0)
	exit(1);

    sectbuf = sect_alloc(1);  /* allocate a sector */

	printf("Insert new diskette for %s\n", devname);
	printf("and strike ENTER when ready");
        fflush(stdout);
	while (getchar() != '\n')
	    ;
	printf("\nFormatting...");
	fflush(stdout);

	if (FDreset() == 0 )
	{
	   if ( format() == 0)
	   {
	       printf("Format complete\n");
	       disk_space();
	       if (get_response("Format another (Y/N)?") == 'Y')
		  { 
		  printf("\n");
		  close(devfd);
		  }
	       else
		   another = FALSE;
	   }
        }
	else
	    printf("Format failure\n\n");
    }

    exit(0);
}

/****************************************************
  DEVICE-INDEPENDENT ROUTINES
 ****************************************************/

/* Format disk.
 */
format()
{
    TRACE(printf("format\n"));

    if (FDsoft_sector() != 0)
	return(-1);

    if (setup_FAT()  !=  0)
	return(-1);

    if (FDverify() != 0)
	return(-1);

    if (write_boot_sector()  !=  0)
	return(-1);

    if (write_root_dir()  !=  0)
	return(-1);
    
    if (write_FATs()  !=  0)
	return(-1);

    return(0);
}

/* Process command line arguments.
 */
get_args(argc, argv)
int argc;
char **argv;
{
    int opt;
    int gotdrive = FALSE;
    extern char    *optarg;    /* option's argument */

    vollabel = lowdensity = FALSE;
    devname = "/dev/fd0";

    while ((opt = getopt(argc, argv, "18V:4D:$"))  !=  EOF)
    {
	switch (opt)
	{
	case 'V':		/* give disk a volume label */
	    vollabel = TRUE;
	    label = optarg;
	    break;
	case '4':		/* 360K diskette in hi density drive */
	    lowdensity = TRUE;
	    break;
	case 'D':		/* which unix device to format */
	    if ( optarg[0] != '/') {
		usage();
		return( errmsg( ERR_DRIVE_INVAL));
	    }
	    devname = optarg;
	    break;
	case '$':		/* turn off trace feature */
#ifdef DEBUG
	    dostrace = TRUE;
#endif DEBUG
	    break;

	case '?':
	default:
	    usage();
	    return(errmsg(ERR_INVAL_PARM));
	}
    }
    if (optind != argc) {
	usage();
	return(-1);
    }

    return(0);
}
usage()
{
    fprintf( stderr, "dosformat [-V label] [-D /dev/<floppy device>] [-4]\n");
    fprintf( stderr, "    -V label, label not to exceed 11 characters\n");
    fprintf( stderr, "    -D /dev/<fd>  specify which floppy drive to format\n");
    fprintf( stderr, "    -4  format to the low density for this type of drive\n");
}

open_drive()
{
     struct devinfo info;

     TRACE(printf("open_drive\n"));

    /* Try to open raw device first.
     * Floppy I/O is lots faster on the raw device
     * (and hard disk I/O is fast either way).
         * 020 indicates synchronous I/O. (FSYNC)
         * O_NDELAY tells the floppy driver NOT (O_NONBLOCK)
         * to try to determine floppy's format.
     */
    if ((devfd = open(devname, O_RDWR | O_NDELAY | 020))  <  0) {
	perror( "dosformat");
	return( 0);
    }
    if (ioctl(devfd, IOCINFO, (char*)&info) < 0)
    {
	TRACE(perror("get_drive_info (IOCINFO)"));
	errmsg(ERR_DRIVE_INVAL);
	return( 0);
    }
    return( devfd);
 
}


/* Determine drive parameters.
 */
get_drive_parms()
{
    struct devinfo info;

    if (ioctl(devfd, IOCINFO, (char*)&info) < 0)
    {
	TRACE(perror("get_drive_info (IOCINFO)"));
	return(errmsg(ERR_DRIVE_INVAL));
    }

    else if (info.devtype != DD_DISK)
    {
	TRACE(printf("bad devtype: %c\n", info.devtype));
	return(errmsg(ERR_DRIVE_INVAL));
    }
  else
    {
	if (FDparms(info)  !=  0)
	    return(-1);
    }
    
    dkp->fat1off = dkp->bootoff + 1;
    dkp->fat2off = dkp->fat1off + dkp->secpfat;
    dkp->rootoff = dkp->fat2off + dkp->secpfat;
    dkp->dataoff = dkp->rootoff + dkp->rootsiz;
    dkp->numsecs = dkp->numcyls * dkp->trkpcyl * dkp->secptrk;

    TRACE(printf("*** devtype=%5d; bytpsec=%5d; secptrk=%5d; trkpcyl=%5d\n",
		 dkp->devtype,dkp->bytpsec,dkp->secptrk,dkp->trkpcyl));
    TRACE(printf("*** secpclu=%5d; numcyls=%5d; numsecs=%5d; fattype=%5d\n",
		 dkp->secpclu,dkp->numcyls,dkp->numsecs,dkp->fattype));
    TRACE(printf("*** fatcode=%5X; secpfat=%5d; rootsiz=%5d; bootoff=%5d\n",
		 dkp->fatcode,dkp->secpfat,dkp->rootsiz,dkp->bootoff));
    TRACE(printf("*** fat1off=%5d; fat2off=%5d; rootoff=%5d; dataoff=%5d\n",
		 dkp->fat1off,dkp->fat2off,dkp->rootoff,dkp->dataoff));
    
    return(0);
}

/* 
 *   Write out boot sector.
 */

#define PT_SIGa     0x55        /* magic number at end of table (lo byte) */
#define PT_SIGb     0xAA        /* magic number at end of table (hi byte) */

write_boot_sector()
{
    char     boot_sec[512];
    int		i;

    TRACE(printf("*** writing out boot sector (sect #%d)\n", dkp->bootoff));

    for ( i = 0; i < 512; i++) boot_sec[i] = 0;
    memcpy( boot_sec, dkp->bootsec, BOOT_SEC_SIZE);
    boot_sec[510] = PT_SIGa;
    boot_sec[511] = PT_SIGb;
    return( write_sector(dkp->bootoff, boot_sec) );
}

/* 
 *   Set up in-core FAT table.
 */

setup_FAT()
{
    TRACE(printf("*** setting up in-core FAT (size=%d)\n", dkp->secpfat));

    if (fatbuf == NULL)
	fatbuf = sect_alloc(dkp->secpfat);

    sect_zero(fatbuf, 0x0, dkp->secpfat);

    fatbuf[0] = dkp->fatcode;
    fatbuf[1] = 0xFF;
    fatbuf[2] = 0xFF;
    if (dkp->fattype == FAT16)
	fatbuf[3] = 0xFF;

    return(0);
}

/* Write both FATs to disk.
 */
write_FATs()
{
    int sect;
    char *fatp;


    /* Write out first FAT.
     */
    fatp = fatbuf;
    TRACE(printf("*** writing out first FAT (sect #%d)\n", dkp->fat1off));
    for (sect=0; sect<dkp->secpfat; sect++)
    {
	if (write_sector(sect + dkp->fat1off, fatp)  !=  0)
	    return(-1);
	fatp += dkp->bytpsec;
    }

    /* Write out second FAT.
     */
    fatp = fatbuf;
    TRACE(printf("*** writing out second FAT (sect #%d)\n", dkp->fat2off));
    for (sect=0; sect<dkp->secpfat; sect++)
    {
	if (write_sector(sect + dkp->fat2off, fatp)  !=  0)
	    return(-1);
	fatp += dkp->bytpsec;
    }

    return(0);
}

/* 
 *    Set up root directory.
 */
#define MAX_LABEL_LEN 11
#define OFFSET_TO_ATTRIBUTE_BYTE 11
#define VOL_ATTRIBUTE 0x08
#define DATE_TIME_INDEX 22

write_root_dir()
{
    register int i;


    TRACE(printf("*** writing out root dir (%d sects, sect #%d)\n",
      dkp->rootsiz, dkp->rootoff));

    sect_zero(sectbuf, FDFILLER, 1);

    for (i=0; i<dkp->bytpsec; i+=DOS_DIRSIZ)
	sectbuf[i] = 0;

    if (vollabel) {
	struct   tm  *tm;
	long         clock;
	register int timefield,
                     datefield;
	register uchar *stamp;

	strncpy( sectbuf, label, MAX_LABEL_LEN);
	sectbuf[ OFFSET_TO_ATTRIBUTE_BYTE] = VOL_ATTRIBUTE;
	clock = time(0);
	tm = localtime( &clock);
	timefield = (((tm->tm_hour << 11) & 0xf800)
	    | ((tm->tm_min << 5) & 0x07e0)
	    | ((tm->tm_sec >> 1) & 0x001f));

	datefield = ((((tm->tm_year - 80) << 9) & 0xfe00)
	    | (((tm->tm_mon + 1) << 5) & 0x01e0)
	    | (tm->tm_mday & 0x001f));

	stamp =(uchar *)&sectbuf[ DATE_TIME_INDEX];

	*stamp++ = timefield & 0xff;
	*stamp++ = (timefield >> 8) & 0xff;
	*stamp++ = datefield & 0xff;
	*stamp = (datefield >> 8) & 0xff;
    }

    for (i=0; i<dkp->rootsiz; i++)
	if (write_sector(dkp->rootoff + i, sectbuf)  !=  0)
	    return(-1);

    return(0);
}

/* Check floppy for bad sectors.
 * (We're assuming hard disk already has
 * its bad sectors mapped out).
 */
FDverify()
{
    int sect, trk;		/* sector, track counters */
    int ntrks;			/* number of tracks on diskette */
    int badtrk;			/* TRUE if entire track is bad */
    int trksize;		/* # bytes in a track */
    int thiscluster, lastcluster = -1;	/* which cluster a sector belongs to */
    static char *trkbuf=NULL;	/* for reading in track */
    register char *s;
    register int i;

    TRACE(printf("*** verifying disk (%d sects)\n", dkp->numsecs));

    if (trkbuf == NULL)
	trkbuf = sect_alloc(dkp->secptrk);
    ntrks   = dkp->trkpcyl * dkp->numcyls;
    trksize = dkp->secptrk * dkp->bytpsec;

    /* Process floppy a track at a time for efficiency's sake.
     */
    for (trk=0; trk<ntrks; trk++)
    {
	/* Read track.
	 */
	badtrk = FALSE;
	if (lseek(devfd, trk*(long)trksize, 0)  <  0
	 || read(devfd, trkbuf, trksize)  !=  trksize) 
	{
	    TRACE(perror("*** FDverify (lseek/read)"));
	    if (errno == ENOTREADY)
		return(errmsg(ERR_NOT_READY));
	    badtrk = TRUE;
	}

	/* Check each sector of track for errors.
	 * When floppy was soft-sectored, each byte
	 * of the sector was initialized to a filler
	 * value.  If it doesn't have that value now,
	 * the sector is bad.
	 */
	for (sect=0; sect<dkp->secptrk; sect++)
	{
	    if (badtrk)				/* the whole track is bad */
		goto badsect;
	    s = &trkbuf[ sect*dkp->bytpsec ];
	    for (i=0; i<dkp->bytpsec; i++)	/* check each byte */
		if (s[i] != FDFILLER)
		    goto badsect;
	    continue;

badsect:    /* Sector is bad.
	     * Find out what cluster sector is in.
	     * If cluster precedes data area on the diskette,
	     * diskette is unusable.  Otherwise, mark the cluster as bad.
	     */
	    thiscluster = sect2cluster(trk*dkp->secptrk + sect);
	    TRACE(printf("*** sec %d, trk %d (clu %d) bad\n",
	      sect, trk, thiscluster));
	    if (thiscluster < 2)
		return(errmsg(ERR_BAD_TRACK0));
	    else if (thiscluster != lastcluster)
	    {
		++dkp->badclus;
		update_FAT(thiscluster, BAD_CLUSTER);
		lastcluster = thiscluster;
	    }
	}
    }

    return(0);
}

/* Output a sector to disk.
 */
write_sector(n, buf)
int n;
register char *buf;
{
    long offset;

    offset = n * (long) dkp->bytpsec;

    if (lseek(devfd, offset, 0)  <  0)
    {
	TRACE(perror("*** write_sector (lseek)"));
	return(errmsg(ERR_BAD_TRACK0));
    }
    if (write(devfd, buf, dkp->bytpsec)  !=  dkp->bytpsec)
    {
	TRACE(perror("*** write_sector (write)"));
	switch (errno)
	{
	case EWRPROTECT:
	    return(errmsg(ERR_WRPROTECT));
	case EBUSY:
	    return(errmsg(ERR_DRIVE_BUSY));
	case ENOTREADY:
	    return(errmsg(ERR_NOT_READY));
	default:
	    return(errmsg(ERR_BAD_TRACK0));
	}
    }
    
    return(0);
}

/* Convert disk sector number to DOS cluster number.
 * Clusters are the units of allocation in the disk's data area
 * and are numbered starting at '2'.
 */
sect2cluster(n)
register int n;
{
    n -= dkp->dataoff;
    n /= dkp->secpclu;
    n += 2;

    return(n);
}

/* Update the entry for cluster 'n'
 * in the (in-core) FAT table.
 */
update_FAT(n, val)
int n;
unsigned val;
{
    register char *fatp;	/* pointer into in-core FAT table */

    /* 16-bit FAT entries.
     */
    if (dkp->fattype == FAT16)
    {
	/* Find the entry in FAT table.
	 */
	fatp = fatbuf  +  (int) (n * FAT16SIZE);

	fatp[0] = val & 0xFF;
	fatp[1] = val>>8 & 0xFF;
    }

    /* 12-bit (1.5 byte) FAT entries.
     * These guys are a bit more complicated to handle than
     * the 16 bit (2 byte) FAT entries since they must share
     * a half byte with an adjacent entry.
     */
    else
    {
	/* Find the entry in FAT table (to the nearest whole byte).
	 */
	fatp = fatbuf  +  (int) (n * FAT12SIZE);

	if (EVEN(n))
	{
	    fatp[0]  = val & 0xFF;
	    fatp[1] &= 0xF0;
	    fatp[1] |= val>>8 & 0x0F;
	}
	else
	{
	    fatp[0] &= 0x0F;
	    fatp[0] |= val<<4 & 0xF0;
	    fatp[1]  = val>>4 & 0xFF;
	}
    }

    TRACE(printf("*** updating FAT for cluster %d (off=%d)\n", n, fatp-fatbuf));
}

disk_space()
{
    int totalbytes, badbytes;

    TRACE(printf("*** printing diskspace\n"));
    totalbytes = (dkp->numsecs - dkp->dataoff) * dkp->bytpsec;
    badbytes   = dkp->badclus * dkp->secpclu * dkp->bytpsec;

    printf("\n%10d bytes total disk space", totalbytes);
    if (badbytes)
	printf("\n%10d bytes in bad sectors", badbytes);
    printf("\n%10d bytes available on disk\n\n", totalbytes - badbytes);
}

get_response(prompt)
char *prompt;
{
    int response;
    int c;

    do {
	printf("%s", prompt);
	fflush(stdout);

	c = response = getchar();
	while (c != '\n')
	    c = getchar();
	response = toupper(response);
    } while (response != 'N' && response != 'Y');

    return(response);
}

errmsg(errcode)
int errcode;
{
    switch (errcode)
    {
    default:
	TRACE(printf("*** [UNKNOWN ERROR: %d]\n", errcode));
    case ERR_GENERIC:
	printf("\nFormat Failure\n");
	break;
    case ERR_DRIVE_INVAL:
	printf("Invalid drive specification\n");
	break;
    case ERR_BAD_TRACK0:
	printf("Invalid media or Track 0 bad - disk unusable\n");
	break;
    case ERR_INVAL_PARM:
	printf("Invalid parameter\n");
	break;
    case ERR_NOMEM:
	printf("Out of memory\n");
	break;
    case ERR_NOSUPPORT:
	printf("Unsupported parameter\n");
	break;
    case ERR_DRIVE_BUSY:
	printf("\nDrive busy\n");
	break;
    case ERR_WRPROTECT:
	printf("\nAttempted write-protect violation\n");
	break;
    case ERR_NOT_READY:
	printf("\nDrive not ready\n");
	break;
    }

    return(-1);
}

/* Zero out a sector buffer.
 */
sect_zero(buf, x, n)
register char *buf;
uchar x;
int n;
{
    register int i, count;

    count = n * dkp->bytpsec;

    for (i=0; i<count; i++)
	buf[i] = x;
}

/* Allocate 'n' sectors worth of memory.
 */
char *sect_alloc(n)
int n;
{
    char *s;
    char *malloc();

    if ((s = malloc((unsigned)(n * dkp->bytpsec)))  ==  NULL)
    {
	errmsg(ERR_NOMEM);
	exit(1);
    }
    
    return(s);
}

/****************************************************
  FLOPPY - SPECIFIC ROUTINES
 ****************************************************/

/* Do soft-sector formatting of floppy
 * (lay out track headers, etc.)
 * using the FDIOCFORMAT ioctl.
 */
FDsoft_sector()
{
    register sect, cyl, track;
    char data[1000];
    register char *p;

    TRACE(printf("FDsoft_sector: numcyls = %d, tracks = %d,secptrk = %d\n",
	dkp->numcyls, dkp->trkpcyl,dkp->secptrk));

    for (cyl=0; cyl<dkp->numcyls; ++cyl)
    {
	for (track=0; track<dkp->trkpcyl; ++track)
	{
	    p = data;
	    for (sect=0; sect<dkp->secptrk; ++sect)
	    {
		*p++ = cyl;
		*p++ = track;
		*p++ = sect + 1;
		*p++ = FD_SECTSIZE;
	    }
	    if (ioctl(devfd, FDIOCFORMAT, data) != 0)
	    {
		TRACE(perror("FDIOCFORMAT"));
		if (errno == EWRPROTECT)
		    return(errmsg(ERR_WRPROTECT));
		else if (errno == ENOTREADY)
		    return(errmsg(ERR_NOT_READY));
		else
		    return(errmsg(ERR_GENERIC));
	    }
	}
    }

    return(0);
}

/* Get/set floppy size parameters.
 */
FDparms(info)
struct devinfo info;
{
    int type;

    if ( info.un.dk.secptrk == 36)
	type = FLOPPY80_3_3;       /* 3 1/2 2.88 meg floppy */
    else if ( info.un.dk.secptrk == 18)
	type = FLOPPY80_3_2;       /* 3 1/2 1.44 meg floppy */
    else if ( info.un.dk.secptrk == 9) {
	if ( info.un.dk.numblks == 720)
	    type = FLOPPY40_9_2;   /* 5 1/4 360k floppy */
	else
	    type = FLOPPY80_3_1;   /* 3 1/2 720k floppy */
    } else
	type = FLOPPY80;           /* 5 1/4 1.2 meg floppy */

    if ( lowdensity) {
	switch(type) {
	    case FLOPPY80_3_1:
	    case FLOPPY80_3_2:
	    case FLOPPY80_3_3:
		type = FLOPPY80_3_1;   /* format a 720k in a 1.44m drive */
		break;
	    case FLOPPY40_9_2:
	    case FLOPPY80:
		type = FLOPPY40_9_2;   /* format a 360k in a 1.2m drive */
		break;
	}
    }
    
    dkp = &dktbl[type];

    return(FDinit_drive());
}

/* Initialize floppy drive.
 */
FDinit_drive()
{
    struct fdinfo fdinfo;

    if (ioctl(devfd, FDIOCGINFO, (char*)&fdinfo) < 0)
    {
	TRACE(perror("get_drive_info (FDIOCGINFO)"));
	return(errmsg(ERR_GENERIC));
    }

    fdinfo.nsects = dkp->secptrk;
    fdinfo.sides  = dkp->trkpcyl;
    fdinfo.ncyls  = dkp->numcyls;

    if (ioctl(devfd, FDIOCSINFO, (char*)&fdinfo) < 0)
    {
	TRACE(perror("get_drive_info (FDIOCSINFO)"));
	return(errmsg(ERR_GENERIC));
    }

    return(0);
}

/* Reset floppy drive.
 * This must be done if floppy was removed from the drive
 * while we had the drive open.
 */
FDreset()
{
    TRACE(printf("*** Resetting floppy drive\n"));

    if (ioctl(devfd, FDIOCRESET, (char*)0)  !=  0)
    {
	TRACE(perror("DIOCERESET"));
	return(errmsg(ERR_GENERIC));
    }

    dkp->badclus = 0;

    return(0);
}
