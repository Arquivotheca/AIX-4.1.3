static char sccsid[] = "@(#)88  1.2  src/bldenv/pkgtools/backup_io.c, pkgtools, bos412, GOLDA411a 1/29/93 16:04:47";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: buf_reset
 *		buf_write
 *		csum
 *		flushmedium
 *		putaclhdr
 *		puthdr
 *		putpclhdr
 *		putsechdr
 *		reorderacl
 *		reorderpcl
 *		wmedium
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

#include <errno.h>

/* length of headers */

long my_count = 0;

union fs_rec hdr_rec;

unsigned char hdrlen[] = {
  btow(sizeof(hdr_rec.v)),                /* FS_VOLUME    */
  btow(sizeof(hdr_rec.x)),                /* FS_VOLINDEX  */
  btow(sizeof(hdr_rec.b)),                /* FS_CLRI      */
  btow(sizeof(hdr_rec.b)),                /* FS_BITS      */
  btow(sizeof(hdr_rec.oi)),               /* FS_OINODE    */
  btow(sizeof(hdr_rec.on)-DUMNAME),       /* FS_ONAME     */
  btow(sizeof(hdr_rec.e)),                /* FS_VOLEND    */
  btow(sizeof(hdr_rec.e)),                /* FS_END       */
  btow(sizeof(hdr_rec.i)),                /* FS_DINODE    */
  btow(sizeof(hdr_rec.n)-DUMNAME),        /* FS_NAME      */
  btow(sizeof(hdr_rec.ds)),               /* FS_DS        */
  btow(sizeof(hdr_rec.nx)-DUMNAME),       /* FS_NAME_X    */
};


/*
 * write out a header
 */
/* VARARGS */
puthdr(tapeinfo,hp, pack, name)
     Tapeinfo *tapeinfo;
     register union fs_rec *hp;
     int pack;
     char *name;
{
  register sum, typ;
  register hlen, nlen;
  int i;

  hlen = hp->h.len = hdrlen[hp->h.type];
  typ = hp->h.type;
  if ((typ == FS_NAME_X) || (typ == FS_NAME)) 
    {
      nlen = btow(strlen(name)+1);
      hp->h.len += nlen;
    }

  if (pack == PACKED)
    hp->h.magic = PACKED_MAGIC;
  else
    hp->h.magic = MAGIC;

  hp->h.checksum = 0;
  sum = csum(hp, hlen);
  if ((typ == FS_NAME_X) || (typ == FS_NAME)) 
    sum += csum(name, nlen);
  hp->h.checksum = sum;
  
  /*
   * reorder the bytes to be machine-independent
   */
  wshort(hp->h.magic);
  wshort(hp->h.checksum);

  switch (typ) {
    
  case FS_VOLUME:
    wshort(hp->v.volnum);
    wlong(hp->v.date);
    wlong(hp->v.dumpdate);
    wlong(hp->v.numwds);
    wshort(hp->v.incno);
    break;
    
  case FS_NAME:
    wshort(hp->i.ino);
    wshort(hp->i.mode);
    wshort(hp->i.nlink);
    wshort(hp->i.uid);
    wshort(hp->i.gid);
    wlong(hp->i.size);
    wlong(hp->i.atime);
    wlong(hp->i.mtime);
    wlong(hp->i.ctime);
    wshort(hp->i.devmaj);
    wshort(hp->i.devmin);
    wshort(hp->i.rdevmaj);
    wshort(hp->i.rdevmin);
    wlong(hp->i.dsize);
    break;

  case FS_NAME_X:
    wshort(hp->nx.nlink);
    wlong(hp->nx.ino);
    wlong(hp->nx.mode);
    wlong(hp->nx.uid);
    wlong(hp->nx.gid);
    wlong(hp->nx.size);
    wlong(hp->nx.atime);
    wlong(hp->nx.mtime);
    wlong(hp->nx.ctime);
    wlong(hp->nx.devmaj);
    wlong(hp->nx.devmin);
    wlong(hp->nx.rdevmaj);
    wlong(hp->nx.rdevmin);
    wlong(hp->nx.dsize);
    break;
    

  case FS_END:
  case FS_VOLEND:
    /* no fields to reorder */
    break;
    
  default:
    printf("BACKUP ERROR: reorder type %d\n", typ);
  }

  wmedium(tapeinfo->tape_fd,(dword *)hp, hlen);
  if ((typ == FS_NAME_X) || (typ == FS_NAME)) 
    wmedium(tapeinfo->tape_fd, (dword *)name, nlen);
}

/*
 * return the checksum of area p, len words long
 * this must be independent of byte order, but should
 * at the same time give numbers with a reasonable
 * spread.
 */
csum(p, len)
     register unsigned char *p;
     register len;
{
  register c,sum;

  sum = 0;
  len = wtob(len);
  do {
    c = *p++;
    sum += (c << (c&7));
  } while (--len);
  
  return sum;
}

/*
 * wmedium writes the buffer b, len "words".
 */
wmedium(tape_fd,b, len)
     int tape_fd;
     register dword *b;
     register len;
{
  if (len == 0) {
    flushmedium(tape_fd);
    return;
  }

  while (len) {
    if (buf_full()) {
      flushmedium(tape_fd);
      buf_reset();
    }
    buf_append(b++); 
    len--;
  }
}

/*
 * flush the current output buffer
 */
flushmedium(tape_fd)
     int tape_fd;
{
  if (!buf_empty())
    buf_write(tape_fd);
}

/* write security header to medium */
putsechdr(tapeinfo,aclsize, pclsize)
     Tapeinfo *tapeinfo;
     int aclsize;
     int pclsize;
{
  struct sac_rec s;
  s.aclsize = btow(aclsize);
  wlong(s.aclsize);
  s.pclsize = btow(pclsize);
  wlong(s.pclsize);
  wmedium(tapeinfo->tape_fd,(dword *)&s, btow(sizeof(s)));
}

/* write acl header to medium */
putaclhdr(tapeinfo,ap, aclsize)
     Tapeinfo *tapeinfo;
     struct acl *ap;
     int aclsize;
{
  reorderacl(ap);
  wmedium(tapeinfo->tape_fd,(dword *)ap, btow(aclsize));
}

/* write pcl header to medium */
putpclhdr(tapeinfo, pp, pclsize)
     Tapeinfo *tapeinfo;
     struct priv *pp;
     int pclsize;
{
  reorderpcl(pp);
  wmedium(tapeinfo->tape_fd, (dword *)pp, btow(pclsize));
}

reorderacl(ap)
     struct acl *ap;
{
  struct acl_entry *acep, *nxt_acep, *ace_end;
  
  wlong(ap->acl_mode);
  wshort(ap->acl_rsvd);
  wshort(ap->u_access);
  wshort(ap->g_access);
  wshort(ap->o_access);

#define	ace_last acl_last	/* Due to an unfortunate choice of mnuemonics	*/
#define ace_nxt acl_nxt		/* the last ace is referred to in acl.h as the	*/
                                /* last acl					*/
  acep = ap->acl_ext;	/* acep points to first acl entry	*/
  ace_end = ace_last(ap);	/* ace_end points to last acl entry	*/

	/* redundant check (with for() loop) */
  if (acep >= ace_end) {
    wlong(ap->acl_len);
    return;
  }
  for ( ; acep < ace_end; acep = nxt_acep)
    {
      struct ace_id	*idp, *nxt_idp;
      struct ace_id	*idend;
      unsigned short tmp;
      struct dummy {
	unsigned short	ace_len;
	unsigned short	ace_bits;
	struct	ace_id	ace_id[1];
      } *bp;
      
      nxt_acep = ace_nxt(acep);
      idend = id_last(acep);
      bp = (struct dummy *)acep;
      wshort(bp->ace_bits);
      for (idp=acep->ace_id; idp<idend; idp=nxt_idp)
	{
	  
	  nxt_idp = id_nxt(idp);
	  wshort(idp->id_len);
	  wshort(idp->id_type);
	}
      wshort(acep->ace_len);
    }
  wlong(ap->acl_len);
}

reorderpcl(pp)
     struct pcl *pp;
{
  struct pcl_entry *pcep, *nxt_pcep, *pce_end;

  wlong(pp->pcl_mode);			/* swap flags 			*/
  wlong(pp->pcl_default.pv_priv[0]);	/* swap priviledge bits		*/
  wlong(pp->pcl_default.pv_priv[1]);

#define	pce_last pcl_last	/* Due to an unfortunate choice of mnuemonics	*/
#define pce_nxt pcl_nxt		/* the last pce is referred to in priv.h as the	*/
				/* last pcl					*/
  pcep = pp->pcl_ext;			/* pcep points to first pcl entry	*/
  pce_end = pce_last(pp);			/* pce_end points to last pcl entry	*/

	/* redundant check (with for() loop) */
  if (pcep >= pce_end) {
    wlong(pp->pcl_len);
    return;
  }
  
  for ( ; pcep < pce_end; pcep = nxt_pcep)
    {
      struct pce_id	*idp, *nxt_idp;
      struct pce_id	*idend;
      
      nxt_pcep = pce_nxt(pcep);
      idend = pcl_id_last(pcep);
      wlong(pcep->pce_privs.pv_priv[0]);	/* swap priviledge bits			*/
      wlong(pcep->pce_privs.pv_priv[1]);
      for (idp = pcep->pce_id; idp < idend; idp = nxt_idp )
	{
	  nxt_idp = pcl_id_nxt(idp);
	  wshort(idp->id_type);
	  wshort(idp->id_len);
	}
      wlong(pcep->pce_len);
    }
  wlong(pp->pcl_len);
}

buf_reset() {
  buf_ptr = buf_start;
}

buf_write(fd) {
  register int written, left;
  register dword *save_ptr, *pos_ptr;
  register daddr_t this_high;
  register nbytes = wtob(buf_len);

  if ((written = write(fd, (char *)buf_start, (unsigned)nbytes)) != nbytes) {
    printf("ERROR:  Write failure for image \n");
    exit(-1);
  }
}  
