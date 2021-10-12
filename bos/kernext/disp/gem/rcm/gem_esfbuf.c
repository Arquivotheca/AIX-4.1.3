static char sccsid[] = "@(#)97	1.3.2.5  src/bos/kernext/disp/gem/rcm/gem_esfbuf.c, sysxdispgem, bos411, 9428A410j 1/19/93 12:19:16";
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		gem_clear_overlay
 *		gem_protect_zbuffer
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#include "gemincl.h"
#include "gemrincl.h"
#include "gem_gai.h"
#include "gmasl.h"
#include "gem_geom.h"

/************************************************************************/
/* FUNCTION:  gem_protect_zbuffer                                       */
/*                                                                      */
/* DESCRIPTION:                                                         */
/*     Ensure that process doesn't overwrite Z-buffer values of other   */
/* windows.								*/
/************************************************************************/
gem_protect_zbuffer(gdp, pClip, fifo_num, buf_start, pBuf, buf_size)
struct _gscDev	*gdp;
gRegionPtr	pClip;
int		fifo_num;
char		*buf_start;
char		**pBuf;
int		buf_size;
{
  struct Zbuf_se {
      ushort	 length;
      ushort	 opcode;
      ulong	 flags;
      gRectangle box;
    } *pZbuf;

  int			i;		/* loop counter			*/
  int			len;
  int			n, lim;
  gRegionPtr		pOldClip;	/* pointer to prev clip region	*/
  rGemRCMPrivPtr	pDevP =		/* device private are in the RCM*/
    &(((rGemDataPtr)gdp->devHead.vttld)->GemRCMPriv);

  len = sizeof(struct Zbuf_se);

  /*
   * Initialize buffer pointer
   */
  pZbuf = (struct Zbuf_se *)(*pBuf);

  /*
   * Set protect bits for any previous clip region
   */
#ifdef GEM_DBUG
  printf("ZB: Setting: ");
#endif
  if (pDevP->old_zbuffer != NULL)
  { pOldClip = pDevP->old_zbuffer;
    n = pOldClip->numBoxes;
    lim = (buf_start + buf_size - *pBuf) / len;
    if (lim > n)
      lim = n;
    while (n > 0)
    { for (i=0; i<lim; ++i, ++pZbuf)
      { pZbuf->length = len;
	pZbuf->opcode = CE_PZBF;
	pZbuf->flags = 1;		/* set protect bits		*/
	pZbuf->box.ul.x = pOldClip->pBox[i].ul.x;
	pZbuf->box.ul.y = GM_HEIGHT - pOldClip->pBox[i].lr.y;
	pZbuf->box.width = pOldClip->pBox[i].lr.x -
	  pOldClip->pBox[i].ul.x;
	pZbuf->box.height = pOldClip->pBox[i].lr.y -
	  pOldClip->pBox[i].ul.y;
#ifdef GEM_DBUG
	printf("(%d, %d) W:%d H:%d ", pZbuf->box.ul.x, pZbuf->box.ul.y,
	       pZbuf->box.width, pZbuf->box.height);
#endif
      }
      n -= lim;
      if (n > 0)
      { flush_buf(pDevP, fifo_num, buf_start, pBuf);
	pZbuf = (struct Zbuf_se *)buf_start;
	lim = buf_size / len;
	if (lim > n)
	  lim = n;
      }
    }
#ifdef GEM_DBUG
    printf("\n");
#endif
  }
  else
  /* set protect bits for entire screen				*/
  { if (*pBuf + len > buf_start + buf_size)
    { flush_buf(pDevP, fifo_num, buf_start, pBuf);
      pZbuf = (struct Zbuf_se *)buf_start;
    }
    pZbuf->length = len;
    pZbuf->opcode = CE_PZBF;
    pZbuf->flags = 1;			/* set protect bits		*/
    pZbuf->box.ul.x = 0;
    pZbuf->box.ul.y = 0;
    pZbuf->box.width = GM_WIDTH;
    pZbuf->box.height = GM_HEIGHT;
#ifdef GEM_DBUG
    printf("(%d, %d) W:%d H:%d ", pZbuf->box.ul.x, pZbuf->box.ul.y,
	   pZbuf->box.width, pZbuf->box.height);
#endif
    ++pZbuf;
  }
  *pBuf = (char *)pZbuf;

  /*
   * Now turn off the protect bits for the new clipping region
   */
#ifdef GEM_DBUG
  printf("ZB: Clearing: ");
#endif
  if (pClip != NULL)
  { n = pClip->numBoxes;
    lim = (buf_start + buf_size - *pBuf) / len;
    if (lim > n)
      lim = n;
    while (n > 0)
    { for(i=0; i<lim; ++i, ++pZbuf)
      { pZbuf->length = len;
	pZbuf->opcode = CE_PZBF;
	pZbuf->flags = 0;		/* reset protect bits		*/
	pZbuf->box.ul.x = pClip->pBox[i].ul.x;
	pZbuf->box.ul.y = GM_HEIGHT - pClip->pBox[i].lr.y;
	pZbuf->box.width = pClip->pBox[i].lr.x - pClip->pBox[i].ul.x;
	pZbuf->box.height = pClip->pBox[i].lr.y - pClip->pBox[i].ul.y;
#ifdef GEM_DBUG
	printf("(%d, %d) W:%d H:%d ", pZbuf->box.ul.x, pZbuf->box.ul.y,
	       pZbuf->box.width, pZbuf->box.height);
#endif
      }
      n -= lim;
      if (n > 0)
      { flush_buf(pDevP, fifo_num, buf_start, pBuf);
	pZbuf = (struct Zbuf_se *)buf_start;
	lim = buf_size / len;
	if (lim > n)
	  lim = n;
      }
    }
#ifdef GEM_DBUG
    printf("\n");
#endif
  }
  else
  /* reset protect bit for entire screen				*/
  { if (*pBuf + len > buf_start + buf_size)
    { flush_buf(pDevP, fifo_num, buf_start, pBuf);
      pZbuf = (struct Zbuf_se *)buf_start;
    }
    pZbuf->length = sizeof(struct Zbuf_se);
    pZbuf->opcode = CE_PZBF;
    pZbuf->flags = 0;			/* reset protect bits		*/
    pZbuf->box.ul.x = 0;
    pZbuf->box.ul.y = 0;
    pZbuf->box.width = GM_WIDTH;
    pZbuf->box.height = GM_HEIGHT;
#ifdef GEM_DBUG
    printf("(%d, %d) W:%d H:%d ", pZbuf->box.ul.x, pZbuf->box.ul.y,
	   pZbuf->box.width, pZbuf->box.height);
#endif
    ++pZbuf;
  }

  /*
   * move clip region to old clip region
   */
  if (pDevP->old_zbuffer && pDevP->old_zbuffer->numBoxes > 0)
    rFree(pDevP->old_zbuffer->pBox);
  if (pClip == NULL)
  { if (pDevP->old_zbuffer)
      rFree(pDevP->old_zbuffer);
    pDevP->old_zbuffer = NULL;
  }
  else
  { if (pDevP->old_zbuffer == NULL)
      pDevP->old_zbuffer = (gRegionPtr)rMalloc(sizeof(gRegion));
    pDevP->old_zbuffer->numBoxes = pClip->numBoxes;
    if (pDevP->old_zbuffer->numBoxes > 0)
      pDevP->old_zbuffer->pBox =
	(gBoxPtr)rMalloc(sizeof(gBox)*pClip->numBoxes);
    for (i=0; i<pClip->numBoxes; ++i)
    { pDevP->old_zbuffer->pBox[i].ul.x = pClip->pBox[i].ul.x;
      pDevP->old_zbuffer->pBox[i].ul.y = pClip->pBox[i].ul.y;
      pDevP->old_zbuffer->pBox[i].lr.x = pClip->pBox[i].lr.x;
      pDevP->old_zbuffer->pBox[i].lr.y = pClip->pBox[i].lr.y;
    }
  }

  *pBuf = (char *)pZbuf;
}

gem_clear_overlay(pDevP, pWG, fifo_num, buf_start, pBuf, buf_size)
rGemRCMPrivPtr	pDevP;
struct _rcmWG	*pWG;
int		fifo_num;
char		*buf_start;
char		**pBuf;
int		buf_size;
{
  int			i, n, lim;
  generic_se		*pSe;
  rectangle		*pRect;
  gBoxPtr		pBox;

#ifdef GEM_DBUG
  printf("Clear Overlay ");
#endif

  if (pWG->wg.pClip != NULL)
  { pSe = (generic_se *)*pBuf;
    pSe->len = sizeof(generic_se);
    pSe->opcode = CE_SDFB;
    pSe->data = OVERLAY_PLANES;
    *pBuf += sizeof(generic_se);
    
    pSe = (generic_se *)*pBuf;
    pSe->len = sizeof(generic_se);
    pSe->opcode = CE_INCI;
    pSe->data = 0;
    *pBuf += sizeof(generic_se);
    
    n = pWG->wg.pClip->numBoxes;
    pBox = pWG->wg.pClip->pBox;
    
    while (n > 0)
    { lim = (buf_start + buf_size - *pBuf) / sizeof(rectangle);
      if (n < lim)
	lim = n;

      for (i=0; i<lim; ++i, ++pBox)
      { pRect = (rectangle *)*pBuf;
	pRect->len = sizeof(rectangle);
	pRect->opcode = CE_IPLG;
	pRect->flags = CONVEX;
	pRect->length = 0x18;
	pRect->pt[0].x = pBox->ul.x;
	pRect->pt[0].y = pBox->ul.y;
	pRect->pt[1].x = pBox->lr.x - 1;
	pRect->pt[1].y = pBox->ul.y;
	pRect->pt[2].x = pBox->lr.x - 1;
	pRect->pt[2].y = pBox->lr.y - 1;
	pRect->pt[3].x = pBox->ul.x;
	pRect->pt[3].y = pBox->lr.y - 1;
	pRect->pt[4].x = pBox->ul.x;
	pRect->pt[4].y = pBox->ul.y;
	*pBuf += sizeof(rectangle);
      }

      n -= lim;

      if (n)
	flush_buf(pDevP, fifo_num, buf_start, pBuf);
    }

    if (*pBuf + 1024 > buf_start + buf_size)
      flush_buf(pDevP, fifo_num, buf_start, pBuf);
    
    pSe = (generic_se *)*pBuf;
    pSe->len = sizeof(generic_se);
    pSe->opcode = CE_SDFB;
    pSe->data = WINDOW_PLANES;
    *pBuf += sizeof(generic_se);
  }
#ifdef GEM_DBUG
  printf("\n");
#endif
}
