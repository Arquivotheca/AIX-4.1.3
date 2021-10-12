static char sccsid[] = "@(#)91  1.9  src/bldenv/pkgtools/do_backup.c, pkgtools, bos41J, 9520A_a 5/17/95 15:01:29";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: init_backup
 *		write_backup_eot
 *		write_backup_file
 *		
 *
 *   ORIGINS: 3,27,9
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "backup_io.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <values.h>

/* The following declarations are used in the 'Huffman Encoding Program'
 * code taken from backbyname.c and included at the end of this file.
 */

#include <sys/shm.h>

#define END     256
#define BLKSIZE BUFSIZ
#define BLKING_FACTOR 1024
#define HPACKED 017436 /* <US><RS> - Unlikely value */

unsigned char pflag = 1;        /* pack files being backed up */

/* union for overlaying a long int with a set of four characters */

/* character counters */
long    count [END+1];
long    insize;
long    filesz;
long    outsize;
int     diffbytes;

/* i/o stuff */
char    *infile;                /* unpacked file */
char    inbuff [BLKSIZE];
char    outbuff [BLKSIZE+4];

/* variables associated with the tree */
int     maxlev;
int     levcount [25];
int     lastnode;
int     parent [2*END+1];

/* variables associated with the encoding process */
char    length [END+1];
long    bits [END+1];
long    mask;
long    inc;

/* the heap */
int     n;
struct  heap {
	long int count;
	int node;
} heap [END+2];
#define hmove(a,b) {(b).count = (a).count; (b).node = (a).node;}

/* End of 'Huffman Encoding Program' declarations */


int init_backup(Tapeinfo *tapeinfo)
{
  union fs_rec vol_rec;

  if ((tapeinfo->tape_fd = open(tapeinfo->device_name, O_WRONLY|O_TRUNC|O_CREAT,
				0666)) < 0) {
    fprintf(stderr, "%s \n", strerror(errno));
    return(-1);
  }
  if ((buf_start = (dword *)malloc(BLKING_FACTOR)) == NULL){
    fprintf(stderr, "ERROR: Could not allocate temporary buffer space\n");
    return (-1);
  }

  buf_ptr = buf_start;
  buf_len = btow(BLKING_FACTOR);
  strcpy(vol_rec.v.disk, "by name");
  strcpy(vol_rec.v.fsname, "by name");
  strcpy(vol_rec.v.user, "BUILD");
  vol_rec.v.h.type = FS_VOLUME;
  vol_rec.v.volnum = 1;
  vol_rec.v.incno = BYNAME;
  time(&vol_rec.v.date);
  time(&vol_rec.v.dumpdate);
  vol_rec.v.numwds = MAXINT;

  puthdr(tapeinfo, &vol_rec, NOT_PACKED);
  return (0);
}


/*
 * backup this file by name
 */
int write_backup_file(Tapeinfo *tapeinfo, Fileinfo *fileinfo)
{
  char buf[BSIZE];
  off_t bytes;
  register fd, i, j;
  int aclsize, pclsize;
  struct acl  *aclbuf;
  struct pcl  *pclbuf;
  struct acl taclbuf;
  struct pcl tpclbuf;
  int size;
  int (*readfunc)();
  int issymlink;
  union fs_rec hdr_rec;
  char *name = fileinfo->filename;

  aclbuf = &taclbuf;
  pclbuf = &tpclbuf;

  /* make a header */

  issymlink = ((fileinfo->f_st.st.st_mode & S_IFMT) == S_IFLNK);

  aclsize = 16;
  memset(aclbuf, 0, 16);
  aclbuf->acl_len = 16;
  aclbuf->acl_mode = fileinfo->f_st.st.st_mode;
  aclbuf->u_access = (fileinfo->f_st.st.st_mode & 0700) >> 6;
  aclbuf->g_access = (fileinfo->f_st.st.st_mode & 070) >> 3;
  aclbuf->o_access = (fileinfo->f_st.st.st_mode & 07);
  
  pclsize = 16;
  memset(pclbuf, 0, 16);
  pclbuf->pcl_len = 16;
  pclbuf->pcl_mode = fileinfo->f_st.st.st_mode;


  hdr_rec.h.type    = FS_NAME_X;
  hdr_rec.nx.ino     = fileinfo->f_st.st.st_ino;
  hdr_rec.nx.uid     = fileinfo->f_st.st.st_uid;
  hdr_rec.nx.gid     = fileinfo->f_st.st.st_gid;
  hdr_rec.nx.mode    = fileinfo->f_st.st.st_mode;
  hdr_rec.nx.nlink   = fileinfo->f_st.st.st_nlink;
  hdr_rec.nx.size    = fileinfo->f_st.st.st_size;
  hdr_rec.nx.atime   = fileinfo->f_st.st.st_atime;
  hdr_rec.nx.mtime   = fileinfo->f_st.st.st_mtime;
  hdr_rec.nx.ctime   = fileinfo->f_st.st.st_ctime;
  hdr_rec.nx.devmaj  = major(fileinfo->f_st.st.st_dev);
  hdr_rec.nx.devmin  = minor(fileinfo->f_st.st.st_dev);
  hdr_rec.nx.rdevmaj = major(fileinfo->f_st.st.st_rdev);
  hdr_rec.nx.rdevmin = minor(fileinfo->f_st.st.st_rdev);
  hdr_rec.nx.dsize   = fileinfo->f_st.st.st_size;

  
  i = (fileinfo->f_st.st.st_mode & S_IFMT);
  if ( i == S_IFSOCK ) {
    fprintf(stderr,"Adepackage: %s socket will not be backed up.\n",name);
    return(0);
  }
  
  if((i != S_IFREG) && ( i != S_IFLNK)) {
    
    if (i == S_IFDIR)
      hdr_rec.nx.size = 0;
    
    /* write header */
    puthdr(tapeinfo,&hdr_rec, NOT_PACKED, name);
    
    /* write security header */
    putsechdr(tapeinfo, aclsize, pclsize);
      
    /* write acl information */
    putaclhdr(tapeinfo, aclbuf, aclsize);
      
    /* write pcl information */
    putpclhdr(tapeinfo, pclbuf, pclsize);
    return (0);
  }
  else
    if (i == S_IFLNK ) {
      if (strlen(fileinfo->f_st.linkname) >= NAMESZ) {
	fprintf(stderr,"Adepackage: %s filename too long\n",name);
	return 0;
      }
      if (fileinfo->f_st.st.st_size + 1 >= NAMESZ) {
	fprintf(stderr, "Adepackage: %s symbolic link too long\n",name);
	return 0;
      }
      i = strlen(fileinfo->f_st.linkname);
      hdr_rec.nx.size = i;
      
      puthdr(tapeinfo, &hdr_rec, NOT_PACKED, name);
      wmedium(tapeinfo->tape_fd, (dword *)fileinfo->f_st.linkname, btow(i));
      return (0);
    }
  
  /* Open the file that we are going to back up
   *
   * File is already opened from main
   */

  fd = fileinfo->file_fd;

  /* The code below to pack the image is taken from backbyname.c */
  /* If initpack fails or decides packing is not worth
   * the trouble then, backup the file in the normal fashion.
   * Backup according to the comments has a limitation of 24Meg.
   * Actually, the limit is a file that has 24M instances of the
   * same byte.  We make a check here to be safest.  The pack
   * algorithm should be improved to remove this limitation.
   */
  if (pflag && fileinfo->f_st.st.st_size &&
      fileinfo->f_st.st.st_size < (24 * 1024 * 1024) &&
      initpack(fd, &size))
    {
      int packread();
      extern long outsize;
      readfunc = packread;
      bytes = size;
      hdr_rec.nx.dsize = size;
      i = PACKED;
    }      			/* end of code from backbyname.c */
  else   
    {
      extern read();
      readfunc = read;
      bytes = fileinfo->f_st.st.st_size;
      i = NOT_PACKED;
    }
  
  /* write header */
  puthdr(tapeinfo, &hdr_rec, i, name);
  
  /* write security header */
  putsechdr(tapeinfo, aclsize, pclsize);
    
  /* write acl information */
  putaclhdr(tapeinfo, aclbuf, aclsize);
  
  /* write pcl information */
  putpclhdr(tapeinfo, pclbuf, pclsize);
  
  while (bytes) {
    register unsigned nwant;
    register ngot;
    
    nwant = min(BSIZE, bytes);
    ngot = (*readfunc)(fd, buf, nwant);
    if (ngot < 0) {
      fprintf(stderr,"Bad read?\n"); /*p26447*/
      perror(name);
      ngot = 0;
      perror(name);
      ngot = 0;
    }
    if (ngot < nwant)       /* changed size? */
      memset(buf+ngot, 0, nwant-ngot);
    wmedium(tapeinfo->tape_fd, (dword *)buf, btow(nwant));
    bytes -= nwant;
  }
  
  if (i == PACKED)			/* endpack is taken from backbyname.c */
    endpack();				   
  
  close(fd);
  
  return (0);
}

void write_backup_eot(Tapeinfo *tapeinfo)
{
  union fs_rec hdr_rec;
  
  hdr_rec.h.type = FS_END;

  puthdr(tapeinfo, &hdr_rec, NOT_PACKED);
  flushmedium(tapeinfo->tape_fd);
  close(tapeinfo->tape_fd);
}


/*      Huffman encoding program
 *      Adapted April 1979, from program by T.G. Szymanski, March 1978
 */

/* gather character frequency statistics */
input ()
{
	register int i;
	register char *cp = infile;
	for (i=0; i<END; i++)
		count[i] = 0;

	for(i = 0 ; i < filesz; i++)
		count[*cp++ & 0377] += 2;
}


/* Function: void packerror(void)
 *
 * Description: The packerror() function prints out
 * a message about the occurance of an internal packing
 * error and exits with a non-zero return code.
 * 
 * Return value: The function does not return.
 *
 * Side effects: None.
 */
void
packerror(void)
{
	fprintf(stderr,  
		"do_backup: An internal packing error occurred.\n");
	exit (1);
}


/*
 * This function is coded as a co-routine of dfile.
 * The state of this thread is saved in a static variable.
 * The first time the function is called the thread produces
 * a first block of output.  The encoding tables must
 * fit in this first block, this assumption is also made
 * by restorx.c in its unpacking code.
 */
packread (fd, buf, nbytes)
int	fd;
char	*buf;
int	nbytes;
{
	static int	firsttime = 1;
	static int	c, i, inleft;
	static char	*inp;
	static char	**q, *outp;
	static int	bitsleft;
	static int	j;
	static int	chkoutsize;

	char		*endbuf = buf + nbytes;

	outp = buf;

	if (!firsttime) 
		goto resume;
	
	firsttime = 0;
	chkoutsize = 0;

	*outp++ = maxlev;
	for (i=1; i<maxlev; i++) {
		if (outp >= endbuf)
			packerror();
		*outp++ = levcount[i];
	}
	if (outp >= endbuf)
		packerror();
	*outp++ = levcount[maxlev]-2;
	for (i=1; i<=maxlev; i++)
		for (c=0; c<END; c++)
			if (length[c] == i) {
				if (outp >= endbuf)
					packerror();
				*outp++ = c;
			}

	/* output the text */

	bitsleft = 8;
	inleft = 0;

	inp = infile;

	for(i = 0; i <= filesz; i++)
	{
		c = (i == filesz)? END : *inp++ & 0377;

		mask = bits[c]<<bitsleft;
		if (bitsleft == 8)
			*outp = (mask>>24)&0377;
		else
			*outp |= (mask>>24)&0377;
		bitsleft -= length[c];
		if (bitsleft < 0) {
			j = 2;
			do {
				if (++outp >= endbuf) {
					chkoutsize += outp - buf;
					return nbytes;
				}
resume:
				*outp = (mask>>8*j)&0377;
				j--;
				bitsleft += 8;
			} while (bitsleft < 0);
		}

	}
	if (bitsleft < 8)
		outp++;

	firsttime = 1;
	chkoutsize += outp - buf;
	if (chkoutsize != outsize)
		packerror();

	return nbytes;
}

/* makes a heap out of heap[i],...,heap[n] */
heapify (i)
{
	register int k;
	int lastparent;
	struct heap heapsubi;
	hmove (heap[i], heapsubi);
	lastparent = n/2;
	while (i <= lastparent) {
		k = 2*i;
		if (heap[k].count > heap[k+1].count && k < n)
			k++;
		if (heapsubi.count < heap[k].count)
			break;
		hmove (heap[k], heap[i]);
		i = k;
	}
	hmove (heapsubi, heap[i]);
}

/* return 1 after successful creation of the pack code, 0 otherwise */
int mkpackcode ()
{
	register int c, i, p;
	long bitsout;

	/* gather frequency statistics */
	input();

	/* put occurring chars in heap with their counts */
	diffbytes = -1;
	count[END] = 1;
	insize = n = 0;
	for (i=END; i>=0; i--) {
		parent[i] = 0;
		if (count[i] > 0) {
			diffbytes++;
			insize += count[i];
			heap[++n].count = count[i];
			heap[n].node = i;
		}
	}
	if (diffbytes == 1) {
		return (0);
	}
	insize >>= 1;
	for (i=n/2; i>=1; i--)
		heapify(i);

	/* build Huffman tree */
	lastnode = END;
	while (n > 1) {
		parent[heap[1].node] = ++lastnode;
		inc = heap[1].count;
		hmove (heap[n], heap[1]);
		n--;
		heapify(1);
		parent[heap[1].node] = lastnode;
		heap[1].node = lastnode;
		heap[1].count += inc;
		heapify(1);
	}
	parent[lastnode] = 0;

	/* assign lengths to encoding for each character */
	bitsout = maxlev = 0;
	for (i=1; i<=24; i++)
		levcount[i] = 0;
	for (i=0; i<=END; i++) {
		c = 0;
		for (p=parent[i]; p!=0; p=parent[p])
			c++;
		levcount[c]++;
		length[i] = c;
		if (c > maxlev)
			maxlev = c;
		bitsout += c*(count[i]>>1);
	}
	bitsout += length[END];
	if (maxlev > 24) {
		/* can't occur unless insize >= 2**24 */
		return(0);
	}

	/* don't bother if no compression results */
	outsize = ((bitsout+7)>>3)+1+maxlev+diffbytes;
	if ((insize+BLKSIZE-1)/BLKSIZE <= ((outsize+BLKSIZE-1)/BLKSIZE)+1) {
		return(0);
	}

	/* compute bit patterns for each character */
	inc = 1L << 24;
	inc >>= maxlev;
	mask = 0;
	for (i=maxlev; i>0; i--) {
		for (c=0; c<=END; c++)
			if (length[c] == i) {
				bits[c] = mask;
				mask += inc;
			}
		mask &= ~inc;
		inc <<= 1;
	}

	return 1;
}

initpack(fd, size)
int fd;
int *size;
{
	struct stat sb;

	*size = 0;
	if ((fstat(fd, &sb) < 0) ||
		(int)(infile = shmat(fd, (char *)0, SHM_MAP|SHM_RDONLY)) < 0 ||
		*(short *)infile == HPACKED)
	{
		return 0;
	}


	filesz = sb.st_size;

	if(mkpackcode()) {
		*size = outsize;
		return 1;
	}

	return 0;
}

endpack()
{
	if (shmdt (infile) == -1) {
		perror("backup: shmdt failed\n");
		exit (1);
	}
}
