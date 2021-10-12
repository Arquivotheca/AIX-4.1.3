static char sccsid[] = "@(#)69	1.11  src/bos/usr/bin/dosdir/dmount.c, cmdpcdos, bos41J, 9508A 2/8/95 12:11:42";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dmount getdevice getptn 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "pcdos.h"
#include "doserrno.h"
#include <sys/fd.h>

/*
*           DMOUNT( device-pathname , current dir name, current dir offset )
*           returns pointer into device table   else NULL for failure
*/

static int fatentry;
int write();
static byte buf[512];
struct fdinfo f[1];
byte devicename[512] = "/dev/fd0";

DCB *dmount(device,cdir,cd_off_t)
byte *device, *cdir;
int *cd_off_t;
{
register int idx;
register byte *p, *tptr, *suffloc, *temptr;
DCB *disk, *getdevice();
byte tname[14], temp[14];
char c;
short dotfound=0;
int read();
byte *malloc();
key_t devkey;	/* unique key for this device */


	TRACE(("DMOUNT: entry %s, curr_dir %s\n",device,cdir));

	/* save the device name for other routines. */
	strcpy(devicename, device);
	/* Generate a unique key for this device.
	 */
	if ((devkey = ftok(device, 'D'))  <  (key_t)0)
	{	doserrno = DE_NOENT;	/* device nonexistent */
		TRACE(("DMOUNT: Non existent device\n"));
		return(NULL);
	}
					     /* test if device table exists */
	if (strcmp(dev_tbl.validity,"VALID TBL")==0)
	{       for (idx=0; idx<MAXDCB; idx++)
			if (dev_tbl.dev_id[idx].devkey  ==  devkey)
			{       disk = &dev_tbl.dev_id[idx];
				TRACE(("DMOUNT: device found in table\n"));
				if (disk->magic == DCBMAGIC)
				{       disk->users++;
					return(disk);
				}
				break;
			}
	}
	else                          /* init table entries first time only */
	{       for (idx=0; idx<MAXDCB; idx++)
		{	dev_tbl.dev_id[idx].users = 0;
			dev_tbl.dev_id[idx].home = idx;
			dev_tbl.dev_id[idx].dev_name[0] = '\0';
			dev_tbl.dev_id[idx].devkey = -1;
			dev_tbl.dev_id[idx].magic = 0;
			dev_tbl.dev_id[idx].lock = 0;
		}
		strcpy(dev_tbl.validity,"VALID TBL");
		TRACE(("DMOUNT: initializing DCB tables\n"));
	}
	disk = getdevice(device,idx);
	if (disk == NULL) {
		TRACE(("DMOUNT: getdevice returned NULL\n"));
		return(NULL);
	}
	disk->users++;
	disk->magic = DCBMAGIC;
	disk->devkey = devkey;

	if (cdir && cd_off_t && *cdir)
	{
	        p = cdir;
		while(*p) p++;
		while (*p!='\\')
			--p;
		++p;
		idx = 0;
		while (*p) {
			if (*p=='.') {
				dotfound=1;
				suffloc= &(temp[idx]);
					/* suffloc points to period in
					   filename, if there is one */
			}
			temp[idx++] = toupper((int)(*p++));
		}
		temp[idx]=0;
		if(dotfound) {
			temptr=temp;
			suffloc++;
				/* suffloc points to 1st char of suffix */
			strcpy(tname,"           ");
				/* Initialize tname to 11 spaces */
			tptr = tname;
			while((c= *temptr++)!='.')
				*tptr++=c;
			idx=8;
			tptr= &(tname[idx]);
			while(*suffloc) {
				*tptr++ = *suffloc++;
				idx++;
			}
			tname[idx]=0;
		}
		else {
			temp[idx]=0;
			strcpy(tname,temp);
		}
		lseek(disk->fd,*cd_off_t,0);
		_devio(read,disk->fd,buf,idx);
		TRACE(("DMOUNT: olddir = %s, newdir = %s, len = %d\n",tname,buf,idx));
		if (strncmp(tname,buf,idx)==0)
		{       TRACE(("DMOUNT: cdir matched, complete DCB=>%8.8x\n",disk));
			return(disk);          /* pointer into device table */
		}
		*cdir = '\0';
		*cd_off_t = PC_ROOTDIR;
	}
	current.dir[disk->home].pathname = PC_ROOTDIR;
	current.dir[disk->home].start = disk->root;
	current.dir[disk->home].nxtcluster = PC_EOF;

	TRACE(("DMOUNT: reset cdir, complete DCB=>%8.8x\n",disk));
	return(disk);                            /* pointer into device table */
}
/*
 *
 */

DCB *getdevice(device,idx)
byte *device;
int idx;
{
byte    *fatptr=NULL,             /* Pointer to malloc'd FAT area */
	*p,
	*malloc();
char *fat_addr();
register int cnt, i, fd, logicalzero;
int read();
int fatsize, icfatsize, fmode, major_ver, minor_ver;
DCB *disk;
int fatd;
/*
 * Info about Floppy Disks, which don't have BPB's on-board
 *  Must look at FAT (the location of which we MUST assume) to find out
 *  which disk is being used.
 */
static struct  diskinfo {
	byte          d_type;           /* PC-DOS disk descriptor */
	int           d_sectrk;         /* Sectors per track */
	int           d_fatsiz;         /* FAT size in sectors */
	int           d_dirsiz;         /* Max root directory entries */
	int           d_csize;          /* Cluster size in sectors */
	int           d_trkcnt;         /* Tracks */
	int           d_hdcnt;          /* Heads */
} diskinfo[] = {
	{0xFD,  9, 2, 112, 2, 40, 2}, /* 5.25 inch  360k    floppy */
        {0xF0, 36, 9, 240, 2, 80, 2}, /* 3.5  inch  2.88meg floppy */
        {0xF0, 18, 9, 224, 1, 80, 2}, /* 3.5  inch  1.44meg floppy */
	{0xF9,  9, 3, 224, 2, 80, 2}, /* 3.5  inch  720k    floppy */
	{0xF9, 15, 7, 224, 1, 80, 2}  /* 5.25 inch  1.2meg  floppy */
};
#define TABLESIZE 5
register struct diskinfo *d;
int getptn();
	TRACE(("getdevice: startof getdevice \n"));
	if (idx == MAXDCB)
	{       for (idx=0; idx<MAXDCB; idx++)
			if (dev_tbl.dev_id[idx].users == 0)
				break;
	}
	disk = &dev_tbl.dev_id[idx];
/* Open DOS disk as UNIX device */
	disk->protect = (int)write;                    /* allow writability */
	fmode = O_RDWR|020;
	if ((fd = open(device,fmode)) < 0)
	{ 
		if (errno == DE_WRPROTECT  ||  errno == DE_BUSY ||
		    errno == DE_ACCES)
		{       fmode = O_RDONLY|020;             /* open as rdonly */
			disk->protect = 0;              /* if no r/w access */
		}

		if (errno == DE_FORMAT)
                  {
		  if (strcmp (device, "/dev/fd0") == 0)
                     device = (byte *) ("/dev/fd0.8");
                  
		  if (strcmp (device, "/dev/fd1") == 0)
                     device = (byte *) ("/dev/fd1.8");
                  }
                  
		if ((fd = _devio(open,device,fmode))<0)
		{       doserrno = errno;
			TRACE(("getdevice: _devio(open) failed.\n"));
			return(NULL);
		}
	}
	if (lseek(fd, 512, 0) < 0)        /* attempt to read real sector 1 */
	{       doserrno = errno;
		close(fd);
		TRACE(("getdevice: lseek() failed.\n"));
		return(NULL);
	}
	if (_devio(read,fd,buf,3) < 0) /* attempt to read 1st 3 bytes of FAT */
	{       doserrno = errno;
		close(fd);
		TRACE(("getdevice: _devio(read) failed.\n"));
		return(NULL);
	}
	fatentry = 0;                   /* default fatentry size */
	if ((buf[0] > 0xf8 | buf[0] == 0xf0) && 
	    (buf[1]==0xff) && (buf[2]==0xff))
		logicalzero = 0;                           /* it's a floppy */
	else
		if ((logicalzero = getptn(fd)) < 0)    /* calc start of ptn */
		{       close(fd);
			TRACE(("getdevice: getptn() failed.\n"));
			return(NULL);
		}
	if (logicalzero)
	{       if (lseek(fd,logicalzero,0) < 0)        /* read BOOT sector */
		{       doserrno = errno;
			close(fd);
			TRACE(("getdevice: lseek(logicalzero) failed.\n"));
			return(NULL);
		}
		if (_devio(read,fd,buf,512) < 0)
		{       doserrno = errno;
			close(fd);
			TRACE(("getdevice: _devio(read) (2) failed.\n"));
			return(NULL);
		}
		if ((buf[510] != PT_SIGa ) || (buf[511] != PT_SIGb))
		{       doserrno = DE_BADMNT;
			close(fd);
			TRACE(("getdevice: Bad mount check failure.\n"));
			return(NULL);
		}
	}
	else                                                    /* DISKETTE */
	{       
		if (lseek(fd, 512, 0) < 0)
		{       doserrno = errno;
			close(fd);
			TRACE(("getdevice: lseek() (2) failed.\n"));
			return(NULL);
		}
		if (_devio(read,fd,buf,sizeof buf) < 0)      /* attempt to read FAT */
		{       doserrno = errno;
			close(fd);
			TRACE(("getdevice: _devio(read) (3) failed.\n"));
			return(NULL);
		}
		for (i = 0;i<TABLESIZE;i++)
			if (buf[0] == diskinfo[i].d_type)
				break;
		if (i < TABLESIZE-4)
		{       d = &diskinfo[i];
			disk->bpb.pb_sectrk = d->d_sectrk;
			disk->bpb.pb_dirsiz = d->d_dirsiz;
			disk->bpb.pb_csize  = d->d_csize;
			disk->bpb.pb_fatsiz = d->d_fatsiz;
			disk->bpb.pb_secsiz = 512;
			disk->bpb.pb_fatcnt = 2;
			disk->bpb.pb_res = 0;
			disk->bpb.pb_ptnsiz = d->d_trkcnt * d->d_hdcnt
					* disk->bpb.pb_sectrk;
			disk->bpb.pb_descr = buf[0];
			disk->bpb.pb_hidsec = 0;
			disk->bpb.pb_headcnt = d->d_hdcnt;

			if (ioctl(fd, FDIOCGINFO, f)>=0)
			{    f->nsects = d->d_sectrk;
			     f->sides = d->d_hdcnt;
			     f->ncyls = d->d_trkcnt;
			     ioctl(fd, FDIOCSINFO, f);			
		        }
		}
			/* must be either 1.44m/2.88m (F0) or 720k/1.2m (F9) */
		else if ((buf[0] == 0xf0) || (buf[0] == 0xf9))
		{
			if (lseek(fd,logicalzero,0) < 0)/* read BOOT sector */
			{       doserrno = errno;
				close(fd);
				TRACE(("getdevice: lseek(logicalzero) (2) failed.\n"));
				return(NULL);
			}
			if (_devio(read,fd,buf,512) < 0)
			{       
				doserrno = errno;
				close(fd);
				TRACE(("getdevice: _devio(read) (4) failed.\n"));
				return(NULL);
			}
		}		
		else
		{       doserrno = DE_BADMNT;
			close(fd);
			TRACE(("getdevice: Bad mount check (2) failure.\n"));
			return(NULL);
		}
	}
	p = buf;
	_DFcpyn(&disk->bpb,p,11);
	p += 3;
							 /* Minor Version # */
	minor_ver = isdigit(p[7]) ? p[7] - '0' : 0;
							 /* Major Version # */
	i=5;
	while(i>=0 && isdigit(p[i]))
	    i--;
	i++;
	if (i>5)
	    major_ver = 3;
	else
	    major_ver = atoi(p+i);
	
	p += 8;
	TRACE(("getdevice: Version number is %d.%d\n", major_ver, minor_ver));
							 /* Bytes per sector*/
	disk->bpb.pb_secsiz = (p[1] << 8) | p[0];
	p += 2;
				      /* Cluster size (sectors, power of 2) */
	disk->bpb.pb_csize = *p++;
					      /* Number of reserved sectors */
	disk->bpb.pb_res = (p[1] << 8) | p[0];
	p += 2;
							  /* Number of FATs */
	disk->bpb.pb_fatcnt = *p++;
				      /* Number of (root) directory entries */
	disk->bpb.pb_dirsiz = (p[1] << 8) | p[0];
	p += 2;
				       /* Size of disk partition in sectors */
	disk->bpb.pb_ptnsiz = (p[1] << 8) | p[0];
	p += 2;
						    /* Media descriptor */
	disk->bpb.pb_descr = *p++;
						    /* Sectors per FAT */
	disk->bpb.pb_fatsiz = (p[1] << 8) | p[0];
	p += 2;
						    /* Sectors per track */
	disk->bpb.pb_sectrk = (p[1] << 8) | p[0];
	p += 2;
						    /* Number of heads */
	disk->bpb.pb_headcnt = (p[1] << 8) | p[0];
	p += 2;
	if (major_ver <= 3) {
						/* Hidden Sectors - Ver 3.x */
	    disk->bpb.pb_hidsec = (p[1] << 8) | p[0];
	} else {
						/* Hidden Sectors - Ver 4.x */
	    disk->bpb.pb_hidsec = 
		    (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
	    p += 4;
				       /* Size of disk partition in sectors */
	    if (disk->bpb.pb_ptnsiz == 0)
		disk->bpb.pb_ptnsiz =
		    (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
	}

	disk->changed = 0;
	disk->clsize = disk->bpb.pb_csize * disk->bpb.pb_secsiz;
	disk->zero = logicalzero;
	fatsize    = disk->bpb.pb_fatsiz * disk->bpb.pb_secsiz;
	TRACE(("getdevice: Fatsize = %d Fatsiz = %d Secsiz = %d\n",
	    fatsize, disk->bpb.pb_fatsiz, disk->bpb.pb_secsiz));
	disk->ccount = ((disk->bpb.pb_ptnsiz
			-(disk->bpb.pb_fatsiz * disk->bpb.pb_fatcnt)
			  -1-(disk->bpb.pb_dirsiz / 16))
			  / disk->bpb.pb_csize) + 2;
	TRACE(("getdevice: Ccount = %d Ptnsiz = %d Fatsiz = %d Fatcnt = %d\n",
	    disk->ccount, disk->bpb.pb_ptnsiz, disk->bpb.pb_fatsiz,
	    disk->bpb.pb_fatcnt));
	disk->root = logicalzero + 512 + disk->bpb.pb_fatcnt * fatsize;
	disk->data = disk->root  + ((disk->bpb.pb_dirsiz / 16)
				     * disk->bpb.pb_secsiz);

	/* Get shared-memory partition for in-core FAT.
	 */
	icfatsize = disk->ccount * sizeof(icfat)  +  3;
	if ((fatd = get_fat(device, icfatsize))  <  0)
	{       doserrno = DE_NOMEM;
		close(fd);
		TRACE(("getdevice: No memory failure.\n"));
		return(NULL);
	}
	disk->fat_desc  = fatd;
	disk->fat_ptr   = (icfat *) fat_addr(fatd);
	disk->fatentsiz = (fatentry==16 ? 16 : 12);

	/* Exclude other processes from access to this
	 * disk until we're done.
	 */
	_DFsetlock(disk);

/* Now, get entire FAT */
	if (lseek(fd,disk->zero+512,0) < 0)
	{       doserrno = errno;
		goto cleanup;
	}
	if ((fatptr = malloc(fatsize)) == 0)
	{       doserrno = DE_NOMEM;
		goto cleanup;
	}
	if (_devio(read,fd,fatptr,fatsize) < 0)
	{       doserrno = errno;
		goto cleanup;
	}
	if ((fatptr[0] != disk->bpb.pb_descr)
		       || (fatptr[1] != 0xFF)
		       || (fatptr[2] != 0xFF))
	{       doserrno = DE_BADMNT;
		goto cleanup;
	}

	TRACE(("getdevice: Validity check.\n"));
	/* If in-core FAT hasn't already been initialized
	 * by another process, initialize it from the on-disk FAT.
	 */
	if ( ! fat_isvalid(fatd) )
	{
	    p = fatptr;
	    if (fatentry == 16) {
		TRACE(("getdevice: FAT == 16. Ccount = %d\n", disk->ccount));
		for (i=0,cnt=0; cnt<disk->ccount; i += 2) {
		    disk->fat_ptr[cnt].cluster =  (p[i]|p[i+1]<<8)&0xffff;
		    disk->fat_ptr[cnt++].usecount = 0;
		}
	    } else {
		TRACE(("getdevice: FAT == 12. Ccount = %d\n", disk->ccount));
		for (i=0,cnt=0; cnt<disk->ccount; i += 3) {
		    disk->fat_ptr[cnt].cluster = p[i]|(p[i+1]<<8)&0xfff;
		    if ((disk->fat_ptr[cnt].cluster & 0x0ff0) == 0x0ff0)
			disk->fat_ptr[cnt].cluster |= 0xf000;
		    disk->fat_ptr[cnt++].usecount = 0;
		    disk->fat_ptr[cnt].cluster = (p[i+1]>>4)|(p[i+2]<<4)&0xfff;
		    if ((disk->fat_ptr[cnt].cluster & 0x0ff0) == 0x0ff0)
			disk->fat_ptr[cnt].cluster |= 0xf000;
		    disk->fat_ptr[cnt++].usecount = 0;
		}
	    }

	    if ( i>fatsize )
	    {   doserrno = DE_BADMNT;
		goto cleanup;
	    }

	    /* The in-core FAT is now ready to go.
	     * Mark it as valid so that other processes will know
	     * to use it instead of re-reading the on-disk FAT.
	     */
	    TRACE(("getdevice: Validate.\n"));
	    validate_fat(fatd);
	}

	TRACE(("getdevice: DFunlock.\n"));
	_DFunlock(disk);
	free(fatptr);
	strncpy(disk->dev_name,device,DEVLEN);
	disk->dev_name[DEVLEN] = '\0';
	disk->fd = fd;                                       /* save handle */
	return(disk);

cleanup:
	if (fatptr!=NULL) free(fatptr);
	release_fat(disk->fat_desc);
	_DFunlock(disk);
	close(fd);
	TRACE(("getdevice: Cleanup failure.\n"));
	return(NULL);
}
/*
 *  Read in partition table.
 *      locate the primary DOS partition (can be more than one in DOS 3.3),
 *      and return offset to its beginning (logicalzero).
 */
getptn(fd)
int fd;
{
register int n, m, y;
register byte *x;
register pc_ptd *d;
pc_ptt ptn;
int read();

	TRACE(("getptn: startof getptn \n"));
	if (lseek(fd, PT_ADDR, 0) < 0)
	{       doserrno = errno;
		TRACE(("getptn: lseek() failed\n"));
		return(-1);
	}
	if (_devio(read,fd, &ptn, sizeof ptn) < 0)
	{       doserrno = errno;
		TRACE(("getptn: _devio(read) failed\n"));
		return(-1);
	}
	if ((ptn.siga != PT_SIGa ) || (ptn.sigb != PT_SIGb))
	{       doserrno = DE_BADMNT;
		TRACE(("getptn: Mount check failed\n"));
		return(-1);
	}

          /* DOS versions 3.2 and earlier set the last entry in the
             partition table.  DOS versions 3.3 and later set the first 
             entry.  Therefore read from the first entry to the last entry
             in the partition table until all entries are read or a 
             non_zero entry is found. */
             
	for (n=0; n<4; n++)
	{       d = (pc_ptd *)&ptn.pt[n];
		TRACE(("getptn: Entry: %d Sysind:%d\n", n, d->pt_sysind));

              /* taken out to resolve DOS 3.3 changes */
              /********************************************
		if (d->pt_sysind == 0)
		{       doserrno = DE_BADMNT;
			return(-1);
		}
              *********************************************/
 
                  /* primary partition with 12 bit FAT */
		if (d->pt_sysind == 1)
			fatentry=12;

                  /* primary partition with 16 bit FAT */
		if ((d->pt_sysind == 4) || (d->pt_sysind == 6))
			fatentry=16;

		if ((d->pt_sysind == 1) || (d->pt_sysind == 4) ||
		    (d->pt_sysind == 6))
		{       x = (byte *)&d->pt_start;
			for (m=3, y=0; m>-1; m--)   /* calc starting offset */
			{       y <<= 8;
				y += x[m];
			}
			return( y*512 );       /* logical zero for this ptn */
		}
	}

          /* return error - no partition found */
	doserrno = DE_BADMNT;
	TRACE(("getptn: No partition found\n"));
	return(-1);
}
