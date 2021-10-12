static char sccsid[] = "@(#)33	1.1  src/bos/usr/ccs/lib/libunpack/unpack.c, cmdcrash, bos411, 9428A410j 4/26/91 00:56:25";
/*
 * COMPONENT_NAME: CMDCRASH
 *
 * FUNCTIONS: 
 *	get_cdtname(index)
 *	endofdata(areas,start)
 *	unpack(index,data_name,ubuf,vad,len)
 *	fill(wanted,pos,get,cnt,buf)
 *	pad(loc,count)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. date 1, date 2
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef _KERNEL
#undef _KERNEL
#endif
#include <ctype.h>
#include <sys/dump.h>
#include <sys/param.h>
#include <sys/cblock.h>
#include <unpack.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define PAD 0x0BAD

char *get_cdtname(int mem, int index);
ulong endofdata(int mem, int areas, ulong start);
int fill(int mem, ulong wanted, ulong pos, ulong get, ulong cnt, char *buf);
ulong pad(char *loc, ulong count);

/*
 * Name: get_cdtname
 * Function:	get the Component Dump Table names from a dump
 *
 * Input:	index = request the i'th cdt name to be returned
 *
 * Output:	pt to a static buffer with the name in it or
 *		NULL if no i'th cdt entry in the dump.
 *		Also, a side effect is that mem will be postioned
 *		to index+1 cdt. 
 */
char *get_cdtname(int mem, int index)
{
    static struct cdt0 head;		/* 1 head per component dump table */
    ulong t,offset;
    int i;

    lseek(mem,0,0);
    if(read(mem, &t, sizeof(t)) != sizeof(t)) {
	printf("0452-055: Cannot read dump file.\n");
	return NULL;
    }

    lseek(mem,0,0);
    /*
     *	loop reading a cdt_head
     *	then for each cdt_entry seek past its bitmap
     *	and data area.
     */
    for(i=0; i<index; i++) {

	/* read past head */
	if(read(mem,&head,sizeof(head)) != sizeof(head)) {
	    /* end of table */
	    return NULL;
	}
	if((i==0) && (head.cdt_magic != DMP_MAGIC)) {
    		lseek(mem,512,0);
		read(mem,&head,sizeof(head));
	}
	if(head.cdt_magic != DMP_MAGIC) {
#ifdef Debug
	    printf("Dump file corrupted.\n");
#endif
	    return NULL;
	}
#ifdef Debug 
	printf("head %d,name:%s,len:%d,0x%.8x\n",i,
	       head.cdt_name,head.cdt_len,head.cdt_len);
#endif
	/* 
	 * seek past data area 
	 * note that mem is postioned at first cdt_entry 
	 * thus lseek returns current position
	 */
	offset=endofdata(mem, NUM_ENTRIES(&head),lseek(mem,0L,1 /*SEEK_CUR*/));

	if(!offset) {
	    printf("0452-056: Premature end of data in dump file.\n");
	    return NULL;
	}
	lseek(mem,offset,0);
		
    }
    return head.cdt_name;
}

/* 
 * Name:	endofdata
 * Function:	find the end of this component dump table's data
 * Returns:	offset of the end of this cdt's data areas 
 *		(offset from beggining of file since "start" is 
 *		position of start of cdt_entrys).
 *		0 on error
 * Inputs:	start - where to start reading cdt_entry structs
 *		areas - number of cdt_entrys for this cdt
 *
 * Note:	this function will affect the position of 
 *		global file descripter "mem" (dumpfile).
 *		"mem" is positioned at start, which is
 *		the first cdt_entry after a cdt_head,
 *		then left no where special
 */
ulong endofdata(int mem, int areas, ulong start)
{
    struct cdt_entry cdt_e; 
    ulong next_cdt_e;
    ulong offset;		/* return this + start */
    int i,j;
    ulong npages;		/* # pages represented in bitmap */
    bitmap_t bitmap[8192];		/* allows up to 256M per data area */
    ulong addr,count,actual_cnt;

    lseek(mem,start,0);
    /* 
     * position offset at start of 
     * data areas
     */
    offset = start + (areas * sizeof(struct cdt_entry));

    for(j=1;j<=areas;j++){
	/* read in the cdt_entry */
	read(mem,&cdt_e,sizeof(cdt_e));
	next_cdt_e = lseek(mem,0,1);
#ifdef Debug
	printf("Entry %d:name:%s,len:0x%.8x,ptr:0x%.8x,nextcdte:0x%.8x\n",
	       j,cdt_e.d_name,cdt_e.d_len,cdt_e.d_ptr,next_cdt_e);
#endif
	/* read bitmap for this data area */
	lseek(mem,offset,0);
	read(mem,bitmap, BITMAPSIZE(cdt_e.d_ptr,cdt_e.d_len));

	/* move offset to where we are now */
	offset += BITMAPSIZE(cdt_e.d_ptr,cdt_e.d_len);
#ifdef Debug
	printf("bitmapsize:%d, bm[0]:0x%.2x  offset:0x%.8x\n",
	       BITMAPSIZE(cdt_e.d_ptr,cdt_e.d_len),bitmap[0],offset);
#endif

	/* here is the big loop */
	addr  = (ulong) cdt_e.d_ptr;
	count = (ulong) cdt_e.d_len;
#ifdef Debug
	printf("endofdata:npages:0x%x PAGESIZE:0x%x\n",npages,PAGESIZE);
	printf("endofdata: count:0x%.8x  addr:0x%.8x\n",count,addr);
#endif
	npages = (ulong) NPAGES(cdt_e.d_ptr,cdt_e.d_len);
	for(i = 0; i < npages; i++) {
	    actual_cnt = min(count,PAGESIZE-(addr%PAGESIZE));
#if 0
	    printf("actual_cnt=%x addr=%x\n",actual_cnt,addr);
#endif
	    if(ISBITMAP(bitmap,i)) {
		offset += actual_cnt;
	    }
	    else {
#if 0
		printf(
		       "page %d (address 0x%.8x) not in memory\n",
		       i,addr);
#endif
	    }
	    count -= actual_cnt;
	    addr  += actual_cnt;
	}
 
	/* set pointer to read nex cdt_e */
	lseek(mem,next_cdt_e,0);
    }
    lseek(mem,offset,0);
    return offset;
}


/*
 * Function:	unpack cdt's data from a dump into callers buffer,
 * 		filling in holes with 0BADD, and return bytes read
 * Returns:	NULL on error or number of bytes read
 *		
 */
ulong *unpack(int mem, int index, char *data_name, char *ubuf, ulong vad,
	      ulong len, int relative)
{
    struct cdt0 head;			/* 1 head per component dump table */
    struct cdt_entry cdt_e;	 	/* multiple cdt entries */
    ulong offset, next_cdt_e;		/* pointers into files */
    bitmap_t bitmap[8192];		/* allows up to 64M per data area */
    ulong npages;		/* # pages represented in bitmap */
    int i,j;
    char *buf;
    ulong addr,count,actual_cnt,unpackit=0,toread,tmp_vad;

    /* 
     * buf = pt to users buffer,next byte to store into tmp_vad =
     * current pseudo addr to start reading.  (address user is
     * intersted in) toread = # bytes left to read for caller addr =
     * 1st addr that can be read on this page.  actual_cnt = # bytes
     * that can be read on this page count = # bytes to read in this
     * cdt data area
     */
    ulong t;

    buf = ubuf;
    /* 
     * position "mem" file descriptor 
     * to cdt of interest 
     */
    if((get_cdtname(mem, index-1))==0)
	return NULL;			/* something wrong with dump file */

    /* read header */
    if(read(mem,&head,sizeof(head)) != sizeof(head)) {
	return NULL;			/* something wrong with dump file */
    }

#ifdef Debug
    printf("unpack:head name:%s,len:%d,0x%08x\n",
	   head.cdt_name,head.cdt_len,head.cdt_len);
    printf("unpack:num entries:%d\n",NUM_ENTRIES(&head));
    printf("unpack:looking for vad:0x%08x, len:%d\n",vad,len);
#endif
    /* position offset at start of data areas (at the bitmap) */
    offset = lseek(mem, 0, 1) +
	((NUM_ENTRIES(&head)*sizeof(struct cdt_entry)));

    /* 
     * unpack data area
     */
    /* for each data area in this cdt */
    for(j=1;j<=NUM_ENTRIES(&head); j++) {
	/* 
	 * read in the cdt_entry
	 */
	read(mem,&cdt_e,sizeof(cdt_e)); 
	/* 
	 * save start of next cdt_entry 
	 */
	next_cdt_e = lseek(mem,0,1); 
#ifdef Debug
	printf("Entry %d:name:%s,len:0x%.8x,ptr:0x%.8x,nextcdte:0x%.8x\n",
	       j,cdt_e.d_name,cdt_e.d_len,cdt_e.d_ptr,next_cdt_e);
#endif
	npages = (ulong) NPAGES(cdt_e.d_ptr,cdt_e.d_len);
	/* 
	 * is this the cdt_entry we are interested in ?
	 */
	if(!strcmp(cdt_e.d_name,data_name)) {
	    unpackit = 1;
	    tmp_vad = vad;
	    toread = len;
	}
	/* 
	 * read bitmap for this data area and
	 * move offset to where we are now in the file
	 */
	lseek(mem,offset,0);
	offset += read(mem,bitmap, BITMAPSIZE(cdt_e.d_ptr,cdt_e.d_len));

	addr  = (ulong) cdt_e.d_ptr;
	count = (ulong) cdt_e.d_len;
	if (relative)
	    tmp_vad += addr;
#ifdef Debug
	/* 	printf("unpack:bitmapsize:%d, bm[0]:0x%02x  offset:0x%08x\n",
		BITMAPSIZE(cdt_e.d_ptr,cdt_e.d_len),bitmap[0],offset);*/
	printf("unpack:npages:0x%x  count:0x%.8x  addr:0x%.8x\n",
	       npages,count,addr);
#endif
	/* for each page */
	for(i = 0; i < npages; i++) {
	    actual_cnt = min(count,PAGESIZE-(addr%PAGESIZE));
	    /* 
	     * if the page is in the dump 
	     * and this is the data entry we want
	     */
	    if(ISBITMAP(bitmap,i)) {
		if(unpackit) {
		    /* 
		     * if our data is on this page get it
		     */
		    /*** chk for boundry prob! */
		    if(tmp_vad>=addr&&tmp_vad<actual_cnt+addr){
			if(!fill(mem, tmp_vad,addr,toread,
				 actual_cnt,buf))
			    /* trouble reading dump file */
			    return NULL;
			t = (min(actual_cnt,toread));
			/** watch this! */
			buf = (char *)((ulong)buf + t);
			tmp_vad += min(actual_cnt,toread);
			toread -= min(actual_cnt,toread);	
		    }
		    else		/* seek ahead a page */
			lseek(mem,PAGESIZE,1);
		}
		/* increment user's data pointer
		 * and psuedo address we are reading 
		 * from for next time
		 */
		offset += actual_cnt;	/* inc file pt */
	    }
	    else {
		/* 
		 * if page is NOT in dump
		 * fill memory with some dummy value
		 */
		if(unpackit) {
		    /* 
		     * if our data is on this page pad it
		     */
		    /*** chk for boundry prob! */
		    if(tmp_vad>=addr&&tmp_vad<actual_cnt+addr){
			/* lseek(mem,offset,0); */
			/* inc in mem pt */
			pad(buf,min(actual_cnt,toread));
			printf("Address 0x%08x: page not in dump. Padding with 0x%08x.\n", tmp_vad,PAD);
			t = (min(actual_cnt,toread));
			/** watch this! */
			buf = (char *)((ulong)buf + t);
			tmp_vad += min(actual_cnt,toread);
			toread -= min(actual_cnt,toread);	
		    }
		}
	    }
	    count -= actual_cnt;
	    addr  += actual_cnt;
	    if(unpackit && (!toread))	/* finished unpacking */
		break;
	}
 
	if(unpackit) {	
	    /* we have finished */
	    if(!toread)
		return((ulong *)len);
	    /* check to see if we get all the data that
	     * was requested - if not tell user.
	     * There is a basic assumption here that 
	     * if any part of the data requested is here
	     * the beginning will be here, though not 
	     * necessarily the end.
	     */
	    printf("Dump data incomplete.Only %d bytes found out of %d.\n",
		   len - toread,len);
	    return((ulong *)(len-toread));
	}
	/* 
	 * set pointer to read nex cdt_e
	 */
	lseek(mem,next_cdt_e,0);
    }
    return NULL;
}

/* Name:	fill - fill users buffer with as much data as they
 *		need from this page 
 * Returns	num of bytes read; or 0 on error;
 */
int fill(int mem, ulong wanted, ulong pos, ulong get, ulong cnt, char *buf)
{
    ulong count;

#ifdef Debug
    printf("fill:wntd:0x%08x,pos:0x%08x,get:%d,cnt:%d\n",wanted,pos,
	   get,cnt);
#endif
    if(wanted < pos) {
	printf( "Parameters to fill not valid.\n");
	return 0;
    }
    if(wanted > pos+cnt) {
	printf("Parameters to fill not valid.\n");
	return 0;
    }
    /* lseek to where to begin reading */
    if(wanted > pos){
#ifdef Debug
	printf("fill:lseek:0x%08x\n",wanted-pos);
#endif
	if(lseek(mem, wanted-pos, 1) == -1) 
	    return 0;
    }
    /* 
     * read which ever is less: 
     * end of page OR "get" bytes
     */
    count = min(cnt,get);
    read(mem,buf,count );
    /* 
     * if(get <= cnt)  we are through getting data for the caller
     * if(get>cnt) we have more to get but we have read to the end
     *	of this page in the dump 
     */
#ifdef Debug
    printf("fill: returning %d\n",count);
#endif
    return count;
}

/*
 * Name: pad
 * 
 * Function:	write cnt bytes of PAD into memory
 *		at loc
 *
 * Returns:	number of bytes xfered
 */
ulong pad(char *loc, ulong count)
{
    ulong i,*wordloc,cnt;

    cnt = count;
    /* loc is not on a word boundry so pad it with zero until it is */
    if(i=(ulong)loc%4)
	while(i--) {
	    *loc++ = 0;
	    cnt-- ;
	}
			
    /* 
     * pad each word with PAD
     */
    wordloc = (ulong *)loc;
    while(cnt/4) {
	*wordloc++ = PAD;
	cnt -= 4;
    }
    loc = (char *)wordloc;
	
    /* 
     * if we have less than a word's worth of bytes left, pad them
     * with zero
     */
    if(i=cnt%4)
	while(i--) 
	    *loc++ = 0;

    return count;
}

int name2cdt(int mem, char *name)
{
    char *p;
    int i, bos_cdt = 0;

    for(i=0; ; i++) {
	if((p = get_cdtname(mem, i))==0)
	    break;
	if(!strcmp(p, name))
	    return i;
    }
    return -1;
}

/* Read memory from the bos/kernel segment */
int read_memory(int mem, int rflag, void *buf, ulong vad, int len)
{
    static int boscdt = -1;

    if (rflag) {
	if (lseek(mem, vad, 0) < 0)
	    return 0;
	return read(mem, buf, len) == len;
    }

    if (boscdt == -1 && (boscdt = name2cdt(mem, "bos")) == -1)
	return 0;

    return unpack(mem, boscdt, "kernel", buf, vad, len, 0) == len;
}

void pdbg_clist(int mem, int rflag, char *s, struct clist *cl)
{
    struct cblock *cbp;
    struct cblock cb;
    register int actual = 0;
    int bcnt = 40;
    int len;
    char buf[64], *to = buf, *from;

    for (cbp = cl->c_cf; cbp; cbp = cbp->c_next) {
	if (!read_memory(mem, rflag, &cb, cbp, sizeof(cb)))
	    break;
	cbp = &cb;

	len = cbp->c_last - cbp->c_first;
	from = cbp->c_data + cbp->c_first;
	actual += len;
	if (len > bcnt)
	    len = bcnt;
	bcnt -= len;
	for ( ; --len >= 0; *to++ = *from++);
    }
    printf("%s cc=%d, actual=%d: ``", s, cl->c_cc, actual);
    dbg_pstr(buf, to, 40);
    printf("''\n");
}

void dbg_pstr(char *from, char *to, int bcnt)
{
    int c;

    while (from < to && --bcnt >= 0) {
	c = *from++;
	if (c & 0x80) {
	    printf("M");
	    --bcnt;
	    c &= 0x7f;
	}
	if (c < ' ' || c == 0x7f) {
	    printf("^");
	    --bcnt;
	    c ^= 0x40;
	}
	printf("%c", c);
    }
}
