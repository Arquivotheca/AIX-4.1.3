static char sccsid[] = "@(#)86  1.11.1.2  src/bos/kernext/x25/jsmbuf.c, sysxx25, bos411, 9428A410j 4/1/94 11:50:23";
/*
 * COMPONENT_NAME: (SYSXX25) X.25 Device handler module
 *
 * FUNCTIONS: x25smbuf_error, x25smbuf_malloc, x25smbuf_wmalloc, x25smbuf_free,
 *            x25smbuf_read_half_word, x25smbuf_read_word, x25smbuf_read_block,
 *            x25smbuf_set_byte, x25smbuf_set_half_word, x25smbuf_set_word,
 *            x25smbuf_set_block, x25smbuf_guarantee_size, x25smbuf_cat,
 *            x25smbuf_address, x25smbuf_length, x25smbuf_adjust_forward,
 *            x25smbuf_trim, x25smbuf_adjust_backward, x25smbuf_copyin,
 *            x25smbuf_copyout, emulate_uiomove, x25smbuf_iomovein,
 *            x25smbuf_iomoveout, x25smbuf_dump, dcheck, dmalloc, dfree,
 *            jsmlist_init, jsmlist_element_init, jsmlist_enq,
 *            jsmlist_add_to_front, jsmlist_deq, jsmlist_rem_without_access,
 *            jsmlist_empty, jsmlist_length, jsmlist_rem, jsmlist_read_head,
 *            jsmlist_iterate_first, jsmlist_iterate_next
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM corp.
 */

#include  <sys/types.h>
#include  <sys/errno.h>
#include  <sys/uio.h>
#include  <sys/trchkid.h>
#include  <sys/ddtrace.h>
#include  <net/spl.h>
#if defined(XDH)
#include  "jsmbuf.h"
#else
#include <x25/jsmbuf.h>
#endif


#if !defined(_KERNEL)
char *malloc();
#endif

byte *claimed_chain=NULL;

#define DEFAULT_MBUF_WAIT M_WAIT

/* VARARGS                                                                   */
/*****************************************************************************/
/*  Function        x25smbuf_error                                           */
/*                                                                           */
/*  Prototype       void x25smbuf_error(                                     */
/*                    char *s,*arg1,*arg2,*arg3,*arg4)                       */
/*                                                                           */
/*  Description     Signals an error with the Mbuf handling.                 */
/*                  Warning:                                                 */
/*                           This is only supported for user space work.     */
/*                           It may or may not be supported for kernel       */
/*                           programs.                                       */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      s           The printf style format string                           */
/*      arg*        The arguments for the printf function.                   */
/*****************************************************************************/
void x25smbuf_error(char *s,...)
{
  char **p=&s;
#if defined(XDH) && defined(WSD_LOCAL_BUILD)
  extern bool do_outputf;
  must_outputf("X25SMBUF_ERROR:");
  must_outputf(s,p[1],p[2],p[3],p[4]);
  must_outputf("\n");
  do_outputf=TRUE;
#else
  outputf("X25SMBUF_ERROR:");
  outputf(s,p[1],p[2],p[3],p[4]);
  outputf("\n");
#endif
  return;
}

/*****************************************************************************/
/*  Function        x25smbuf_malloc                                          */
/*                                                                           */
/*  Prototype       mbuf_t *x25smbuf_malloc(unsigned size)                   */
/*                                                                           */
/*  Description     Allocates an M buf and initialises the pointers to       */
/*                  their default values. There are three possibilities for  */
/*                  the results of this call                                 */
/*                  1) A small Mbuf is allocated.  This will happen iff      */
/*                     size < size of small Mbuf data area.                  */
/*                  2) A big Mbuf is allocated.  This uses a                 */
/*                     small Mbuf as a header element for it.  See any Mbuf  */
/*                     documentation for more details. This will happen iff  */
/*                     size < size of a big Mbuf data area.                  */
/*                  3) A chain of Mbufs are allocated.  The first Mbuf will  */
/*                     be a big one. The next Mbuf will be allocated as if   */
/*                     x25smbuf_malloc has been asked for size - big Mbuf    */
/*                     data size.                                            */
/*                                                                           */
/*  Return Code                                                              */
/*                  A pointer to the Mbuf lists.                             */
/*                                                                           */
/*  Parameters                                                               */
/*      size        The number of bytes of USER data to allocate.            */
/*****************************************************************************/
mbuf_t *x25smbuf_malloc(
  unsigned size)
{
  mbuf_t *x25smbuf_ptr;

  if (size<=MLEN)				  /* Will a small one do ?   */
  {
#if !defined(_KERNEL)
    x25smbuf_ptr=(mbuf_t *)malloc(sizeof(mbuf_t));
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
    {
      x25smbuf_ptr->m_data=x25smbuf_ptr+OFFSETOF(m_dat[0],mbuf_t);
      x25smbuf_ptr->m_type=MT_DATA;
      x25smbuf_ptr->m_next=NULL_MBUF_PTR;
      x25smbuf_ptr->m_len=size;
    }
#else
    x25smbuf_ptr=m_get(DEFAULT_MBUF_WAIT, MT_DATA);
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
      x25smbuf_ptr->m_len=size;
#endif
  }
  else if (size<=CLBYTES)
  {						  /* Lets make it a big'un   */
    /*************************************************************************/
    /* Big Mbufs are made up of a small Mbuf with additional memory          */
    /* 'hanging off' the offset, rather than pointing internally             */
    /*************************************************************************/
#if !defined(_KERNEL)
    x25smbuf_ptr=(mbuf_t *)malloc(sizeof(mbuf_t)+CLBYTES);
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
    {
      x25smbuf_ptr->m_data=(byte *)(x25smbuf_ptr+1);
      x25smbuf_ptr->m_type=MT_DATA;
      x25smbuf_ptr->m_next=NULL_MBUF_PTR;
      x25smbuf_ptr->m_len=size;
    }
#else
    x25smbuf_ptr=m_getclust(DEFAULT_MBUF_WAIT, MT_DATA);
    if (x25smbuf_ptr!=NULL)
      x25smbuf_ptr->m_len=size;
#endif
  }
  else
  {						  /* We've got to make a list*/
    /*************************************************************************/
    /* O.K., the amount of data requested is far too big to fit into one     */
    /* Mbuf.  So we'll have to allocate more than 1                          */
    /*************************************************************************/
#if !defined(_KERNEL)
    x25smbuf_ptr=(mbuf_t *)malloc(sizeof(mbuf_t)+CLBYTES);
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
    {
      x25smbuf_ptr->m_data=(byte *)(x25smbuf_ptr+1);
      x25smbuf_ptr->m_type=MT_DATA;
      x25smbuf_ptr->m_len=CLBYTES;
      x25smbuf_ptr->m_act=NULL_MBUF_PTR;
    }
#else
    x25smbuf_ptr=m_getclust(DEFAULT_MBUF_WAIT, MT_DATA);
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
      x25smbuf_ptr->m_len=CLBYTES;
#endif
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
    {
      /***********************************************************************/
      /* We have successfully claimed a single Mbuf... now, we need to       */
      /* claim the rest.                                                     */
      /***********************************************************************/
      x25smbuf_ptr->m_next=x25smbuf_malloc(size-x25smbuf_ptr->m_len);
      if (x25smbuf_ptr->m_next==NULL_MBUF_PTR)
      {
	x25smbuf_free(x25smbuf_ptr);
	x25smbuf_ptr=NULL_MBUF_PTR;
      }
    }
  }
#ifdef DEBUG
  outputf("x25smbuf_malloc size=%d rc=0x%x\n",size,x25smbuf_ptr);
#endif
  if (x25smbuf_ptr==NULL)
      x25smbuf_error("x25smbuf_malloc: failed to claim an mbuf size=%d\n",
		     size);
  DDHKWD1(HKWD_DD_X25PRF, 0xDB, 0, x25smbuf_ptr);
  return x25smbuf_ptr;
}

/*****************************************************************************/
/*  Function        x25smbuf_wmalloc                                          */
/*                                                                           */
/*  Prototype       mbuf_t *x25smbuf_wmalloc(unsigned size)                   */
/*                                                                           */
/*  Description     Allocates an M buf and initialises the pointers to       */
/*                  their default values.  This procedures waits for an      */
/*                  mbuf.  There are three posibilities for                  */
/*                  the results of this call                                 */
/*                  1) A small Mbuf is allocated.  This will happen iff      */
/*                     size < size of small Mbuf data area.                  */
/*                  2) A big Mbuf is allocated.  This uses a                 */
/*                     small Mbuf as a header element for it.  See any Mbuf  */
/*                     documentation for more details. This will happen iff  */
/*                     size < size of a big Mbuf data area.                  */
/*                  3) A chain of Mbufs are allocated.  The first Mbuf will  */
/*                     be a big one. The next Mbuf will be allocated as if   */
/*                     x25smbuf_malloc has been asked for size - big Mbuf    */
/*                     data size.                                            */
/*                                                                           */
/*  Return Code                                                              */
/*                  A pointer to the Mbuf lists.                             */
/*                                                                           */
/*  Parameters                                                               */
/*      size        The number of bytes of USER data to allocate.            */
/*****************************************************************************/
mbuf_t *x25smbuf_wmalloc(
  unsigned size)
{
  mbuf_t *x25smbuf_ptr;

  if (size<=MLEN)				  /* Will a small one do ?   */
  {
#if !defined(_KERNEL)
    x25smbuf_ptr=(mbuf_t *)malloc(sizeof(mbuf_t));
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
    {
      x25smbuf_ptr->m_data=x25smbuf_ptr+OFFSETOF(m_dat[0],mbuf_t);
      x25smbuf_ptr->m_type=MT_DATA;
      x25smbuf_ptr->m_next=NULL_MBUF_PTR;
      x25smbuf_ptr->m_len=size;
    }
#else
    x25smbuf_ptr=m_get(M_WAIT, MT_DATA);
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
      x25smbuf_ptr->m_len=size;
#endif
  }
  else if (size<=CLBYTES)
  {						  /* Lets make it a big'un   */
    /*************************************************************************/
    /* Big Mbufs are made up of a small Mbuf with additional memory          */
    /* 'hanging off' the offset, rather than pointing internally             */
    /*************************************************************************/
#if !defined(_KERNEL)
    x25smbuf_ptr=(mbuf_t *)malloc(sizeof(mbuf_t)+CLBYTES);
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
    {
      x25smbuf_ptr->m_data=(byte *)(x25smbuf_ptr+1);
      x25smbuf_ptr->m_type=MT_DATA;
      x25smbuf_ptr->m_next=NULL_MBUF_PTR;
      x25smbuf_ptr->m_len=size;
    }
#else
    x25smbuf_ptr=m_getclust(M_WAIT, MT_DATA);
    if (x25smbuf_ptr!=NULL)
      x25smbuf_ptr->m_len=size;
#endif
  }
  else
  {						  /* We've got to make a list*/
    /*************************************************************************/
    /* O.K., the amount of data requested is far too big to fit into one     */
    /* Mbuf.  So we'll have to allocate more than 1                          */
    /*************************************************************************/
#if !defined(_KERNEL)
    x25smbuf_ptr=(mbuf_t *)malloc(sizeof(mbuf_t)+CLBYTES);
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
    {
      x25smbuf_ptr->m_data=(byte *)(x25smbuf_ptr+1);
      x25smbuf_ptr->m_type=MT_DATA;
      x25smbuf_ptr->m_len=CLBYTES;
      x25smbuf_ptr->m_act=NULL_MBUF_PTR;
    }
#else
    x25smbuf_ptr=m_getclust(M_WAIT, MT_DATA);
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
      x25smbuf_ptr->m_len=CLBYTES;
#endif
    if (x25smbuf_ptr!=NULL_MBUF_PTR)
    {
      /***********************************************************************/
      /* We have successfully claimed a single Mbuf... now, we need to       */
      /* claim the rest.                                                     */
      /***********************************************************************/
      x25smbuf_ptr->m_next=x25smbuf_malloc(size-x25smbuf_ptr->m_len);
      if (x25smbuf_ptr->m_next==NULL_MBUF_PTR)
      {
	x25smbuf_free(x25smbuf_ptr);
	x25smbuf_ptr=NULL_MBUF_PTR;
      }
    }
  }
#ifdef DEBUG
  outputf("x25smbuf_malloc size=%d rc=0x%x\n",size,x25smbuf_ptr);
#endif
  DDHKWD1(HKWD_DD_X25PRF, 0xDC, 0, x25smbuf_ptr);
  return x25smbuf_ptr;
}

/*****************************************************************************/
/*  Function        x25smbuf_free                                            */
/*                                                                           */
/*  Prototype       void x25smbuf_free(                                      */
/*                    mbuf_t *mbuf_ptr)                                      */
/*                                                                           */
/*  Description     The mbuf list is put back in the Mbuf pool.              */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*****************************************************************************/
int x25bad_mbfree=0;

void x25smbuf_free(
  mbuf_t *x25smbuf_ptr)
{
 
  DDHKWD2(HKWD_DD_X25PRF, 0xDD, 0, x25smbuf_ptr, x25smbuf_ptr->m_type);

  if (x25smbuf_ptr==NULL || x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25bad_mbfree++;
    x25smbuf_error("Cannot free mbuf %x as it is invalid",x25smbuf_ptr);
    return;
  }
  else
  {
    /*************************************************************************/
    /* If the Mbuf has a large additional memory block attached, free that   */
    /* first.                                                                */
    /* For the unit test code, there is no need to free the additional       */
    /* memory as it follows the Mbuf in the malloc'd block                   */
    /*************************************************************************/
#if !defined(_KERNEL)
    /*************************************************************************/
    /* If the Mbuf is part of a chain, free the next one too.  We can't free */
    /* this Mbuf yet as we need to know what the m_next link is              */
    /*************************************************************************/
    if (x25smbuf_ptr->m_next!=NULL)
      x25smbuf_free(x25smbuf_ptr->m_next);
    /*************************************************************************/
    /* Finally, free the base bytes of the Mbuf.                             */
    /* (and the additional memory if we are unit testing                     */
    /*************************************************************************/
    free((char *)x25smbuf_ptr);
#else
#ifdef DEBUG
    outputf("x25smbuf_free buffer=0x%x\n",x25smbuf_ptr);
#endif
    m_freem(x25smbuf_ptr);
#endif
  }
  return;
}

/*****************************************************************************/
/*  Function        x25smbuf_read_byte                                       */
/*                                                                           */
/*  Prototype       byte x25smbuf_read_byte (                                */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned offset)                                       */
/*                                                                           */
/*  Description     Reads a byte from a given offset within a Mbuf list.     */
/*                  It will iterate down the m_next pointers where           */
/*                  necessary.                                               */
/*                                                                           */
/*  Return Code                                                              */
/*                  The byte at offset into the Mbuf data.                   */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to read the value from                        */
/*****************************************************************************/
byte x25smbuf_read_byte(
  mbuf_t *x25smbuf_ptr,
  unsigned offset)
{
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error(
      "Error: x25smbuf_read_byte passed a null pointer offset=%x\n",
      offset);
    return 0;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_read_byte passed a bad Mbuf pointer. offset=%x mbuf=%x\n",
      offset,
      x25smbuf_ptr);
    return 0;
  }
  
  /***************************************************************************/
  /* First, lets get x25smbuf_ptr pointing to the Mbuf with the offset in    */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  if (x25smbuf_ptr==NULL)			  /* Failed to find offset ? */
  {
    x25smbuf_error("Invalid offset %x for mbuf read byte\n",offset);
    return 0;
  }
  return mtod(x25smbuf_ptr,byte *)[offset];	  /* Return byte at offset   */
}

/*****************************************************************************/
/*  Function        x25smbuf_read_half_word                                  */
/*                                                                           */
/*  Prototype       ushort x25smbuf_read_half_word (                         */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned offset)                                       */
/*                                                                           */
/*  Description     Reads a two byte value from a given offset within a      */
/*                  Mbuf list.  It will iterate down the m_next pointers     */
/*                  where necessary.  However, the half word must be stored  */
/*                  on a half word boundary and MUST NOT cross an Mbuf       */
/*                  boundary.                                                */
/*                                                                           */
/*  Return Code                                                              */
/*                  The ushort at offset into the Mbuf data.                 */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to read the value from                        */
/*****************************************************************************/
ushort x25smbuf_read_half_word(
  mbuf_t *x25smbuf_ptr,
  unsigned offset)
{
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error(
      "Error: x25smbuf_read_half_word passed a null pointer. offset=%x\n",
      offset);
    return 0;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_read_half_word passed a bad Mbuf pointer. offset=%x mbuf=%x\n",
      offset,
      x25smbuf_ptr);
    return 0;
  }

  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Invalid offset %x for mbuf read half word\n",offset);
    return 0;
  }
  return mtod(x25smbuf_ptr,ushort *)[offset/sizeof(ushort)];
}

/*****************************************************************************/
/*  Function        x25smbuf_read_word                                       */
/*                                                                           */
/*  Prototype       unsigned x25smbuf_read_word (                            */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned offset)                                       */
/*                                                                           */
/*  Description     Reads a four byte value from a given offset within a     */
/*                  Mbuf list.  It will iterate down the m_next pointers     */
/*                  where necessary.  However, the word must be stored on a  */
/*                  word boundary and MUST NOT cross an Mbuf boundary.       */
/*                                                                           */
/*  Return Code                                                              */
/*                  The unsigned at offset into the Mbuf data.               */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to read the value from                        */
/*****************************************************************************/
unsigned x25smbuf_read_word(
  mbuf_t *x25smbuf_ptr,
  unsigned offset)
{
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error(
      "Error: x25smbuf_read_word passed a null pointer. offset=%x\n",
      offset);
    return 0;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_read_word passed a bad Mbuf pointer. offset=%x mbuf=%x\n",
      offset,
      x25smbuf_ptr);
    return 0;
  }

  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Invalid offset %x for mbuf read word\n",offset);
    return 0;
  }
  return mtod(x25smbuf_ptr,unsigned *)[offset/sizeof(unsigned)];
}

/*****************************************************************************/
/*  Function        x25smbuf_read_block                                      */
/*                                                                           */
/*  Prototype       void x25smbuf_read_block (                               */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned offset,                                       */
/*                    byte *block_ptr,                                       */
/*                    unsigned num_bytes)                                    */
/*                                                                           */
/*  Description     Reads a block of bytes from a given offset within a      */
/*                  Mbuf list. It will iterate down the m_next pointers      */
/*                  where necessary. There are no alignment or mbuf          */
/*                  crossing restrictions.                                   */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to read the block from                        */
/*      block_ptr   The location to store the block at                       */
/*      num_bytes   The number of bytes to copy                              */
/*****************************************************************************/
void x25smbuf_read_block(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  byte *block_ptr,
  unsigned num_bytes)
{
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error(
      "Error: x25smbuf_read_block passed a null pointer. offset=%x\n",
      offset);
    return;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_read_block passed a bad Mbuf pointer. offset=%x mbuf=%x\n",
      offset,
      x25smbuf_ptr);
    return;
  }

  /***************************************************************************/
  /* Skip to first Mbuf to copy into                                         */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Copy the data from the block passed in into the waiting Mbuf.           */
  /* Be careful that the Mbuf may run out before the block does.  If the     */
  /* Mbuf does not have enough space in it for the number of bytes the       */
  /* user passes, only copy the amount the user requested                    */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && READ_MBUF_LENGTH(x25smbuf_ptr)<offset+num_bytes)
  {
    /*************************************************************************/
    /* We cannot copy the whole block as there is not enough space within    */
    /* the Mbuf.  So just copy from the offset into the Mbuf to the end of   */
    /* the Mbuf.                                                             */
    /*************************************************************************/

#if !defined (_KERNEL)
    memcpy(
      block_ptr,
      &(mtod(x25smbuf_ptr,byte *)[offset]),
      READ_MBUF_LENGTH(x25smbuf_ptr)-offset);
#else
    bcopy(
      &(mtod(x25smbuf_ptr,byte *)[offset]),
      block_ptr,
      READ_MBUF_LENGTH(x25smbuf_ptr)-offset);
#endif
    /*************************************************************************/
    /* Now advance the counters.  We have a few less bytes to read, we have  */
    /* to read from further into the block.  The offset into the block is    */
    /* now at the start...                                                   */
    /*************************************************************************/
    num_bytes-=READ_MBUF_LENGTH(x25smbuf_ptr)-offset;
    block_ptr+=READ_MBUF_LENGTH(x25smbuf_ptr)-offset;
    offset=0;
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  if (x25smbuf_ptr==NULL && num_bytes!=0)	        /* Oops ? Bad Parms  */
  {
    x25smbuf_error("Invalid offset %x for mbuf read block\n",offset);
  }
  else
  {
    /*************************************************************************/
    /* Finish off the last few bytes                                         */
    /*************************************************************************/
#if !defined (_KERNEL)
    memcpy(block_ptr,&(mtod(x25smbuf_ptr,byte *)[offset]),num_bytes);
#else
    bcopy(&(mtod(x25smbuf_ptr,byte *)[offset]),block_ptr,num_bytes);
#endif
  }
}

/*****************************************************************************/
/*  Function        x25smbuf_set_byte                                        */
/*                                                                           */
/*  Prototype       void x25smbuf_set_byte (                                 */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned offset,                                       */
/*                    byte value)                                            */
/*                                                                           */
/*  Description     Set the byte at a given offset within a Mbuf list.  It   */
/*                  will iterate down the m_next pointers where necessary.   */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to store the value at                         */
/*      value       The value to store in the Mbuf                           */
/*****************************************************************************/
void x25smbuf_set_byte(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  byte value)
{
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error(
      "Error: x25smbuf_set_byte passed a null pointer. offset=%x\n",
      offset);
    return;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_set_byte passed a bad Mbuf pointer. offset=%x mbuf=%x\n",
      offset,
      x25smbuf_ptr);
    return;
  }

  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Invalid offset %x for mbuf set byte\n",offset);
  }
  else
    mtod(x25smbuf_ptr,byte *)[offset]=value;
  return ;
}

/*****************************************************************************/
/*  Function        x25smbuf_set_half_word                                   */
/*                                                                           */
/*  Prototype       void x25smbuf_set_half_word (                            */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned offset,                                       */
/*                    ushort value)                                          */
/*                                                                           */
/*  Description     Set the half word at a given offset within a Mbuf list.  */
/*                  It will iterate down the m_next pointers where           */
/*                  necessary.  However, the half word must be stored on a   */
/*                  half word boundary and MUST NOT cross an Mbuf boundary.  */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to store the value at                         */
/*      value       The value to store in the Mbuf                           */
/*****************************************************************************/
void x25smbuf_set_half_word(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  ushort value)
{
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error(
      "Error: x25smbuf_set_half_word passed a null pointer. offset=%x\n",
      offset);
    return;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_set_half_word passed a bad Mbuf pointer. offset=%x mbuf=%x\n",
      offset,
      x25smbuf_ptr);
    return;
  }
  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Invalid offset %x for mbuf set half word\n",offset);
  }
  else
    mtod(x25smbuf_ptr,ushort *)[offset/sizeof(value)]=value;
  return;
}

/*****************************************************************************/
/*  Function        x25smbuf_set_word                                        */
/*                                                                           */
/*  Prototype       void x25smbuf_set_word (                                 */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned offset,                                       */
/*                    unsigned value)                                        */
/*                                                                           */
/*  Description     Set the word at a given offset within a Mbuf list.  It   */
/*                  will iterate down the m_next pointers where necessary.   */
/*                  However, the  word must be stored on a word boundary     */
/*                  and MUST NOT cross an Mbuf boundary.                     */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to store the value at                         */
/*      value       The value to store in the Mbuf                           */
/*****************************************************************************/
void x25smbuf_set_word(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  unsigned value)
{
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error(
      "Error: x25smbuf_set_word passed a null pointer. offset=%x\n",
      offset);
    return;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_set_word passed a bad Mbuf pointer. offset=%x mbuf=%x\n",
      offset,
      x25smbuf_ptr);
    return;
  }
  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Invalid offset %x for mbuf set word\n",offset);
  }
  else
    mtod(x25smbuf_ptr,unsigned *)[offset/sizeof(value)]=value;
}

/*****************************************************************************/
/*  Function        x25smbuf_set_block                                       */
/*                                                                           */
/*  Prototype       void x25smbuf_set_block (                                */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned offset,                                       */
/*                    byte *block_ptr,                                       */
/*                    unsigned num_bytes)                                    */
/*                                                                           */
/*  Description     Stores a block of bytes at a given offset within a Mbuf  */
/*                  list. It will iterate down the m_next pointers where     */
/*                  necessary. There are no alignment or mbuf crossing       */
/*                  restrictions.                                            */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to store the block at                         */
/*      block_ptr   The location to read the block from                      */
/*      num_bytes   The number of bytes to copy                              */
/*****************************************************************************/
void x25smbuf_set_block(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  byte *block_ptr,
  unsigned num_bytes)
{
  unsigned amount_to_copy;			  /* Bytes to copy in 1 go   */

  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error(
      "Error: x25smbuf_set_block passed a null pointer. offset=%x\n",
      offset);
    return;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_set_block passed a bad Mbuf pointer. offset=%x mbuf=%x\n",
      offset,
      x25smbuf_ptr);
    return;
  }
  /***************************************************************************/
  /* First, set up x25smbuf_ptr so that it is pointing to the buffer where   */
  /* the offset starts.                                                      */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Now, handle the situation where the number of bytes to copy is LARGER   */
  /* than the space in the current mbuf.  If so, we need to copy             */
  /* only the remaining space in the mbuf, NOT the number of bytes to copy.  */
  /* We should carry on doing this until there are enough bytes space in an  */
  /* M buf to cope.                                                          */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && READ_MBUF_LENGTH(x25smbuf_ptr)<offset+num_bytes)
  {
    /*************************************************************************/
    /* Since there are too many bytes in block_ptr to copy into this         */
    /* buffer, we must arrange it so that the current Mbuf is filled with    */
    /* as much as possible.  The amount to copy is the size of the buffer    */
    /* without the offset bytes.                                             */
    /*************************************************************************/
    amount_to_copy=READ_MBUF_LENGTH(x25smbuf_ptr)-offset;
#if !defined (_KERNEL)
    memcpy(&(mtod(x25smbuf_ptr,byte *)[offset]),block_ptr,amount_to_copy);
#else
    bcopy(block_ptr,&(mtod(x25smbuf_ptr,byte *)[offset]),amount_to_copy);
#endif
    /*************************************************************************/
    /* Adjust the state variables in the right direction.  We have less bytes*/
    /* to copy, we should be copying from further into the block             */
    /*************************************************************************/
    num_bytes-=amount_to_copy;
    block_ptr+=amount_to_copy;
    offset=0;
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Oops, asked to copy a block that can't fit in the current Mbuf          */
  /***************************************************************************/
  if (x25smbuf_ptr==NULL && num_bytes!=0)
  {
    x25smbuf_error("Invalid offset %x for mbuf set block\n",offset);
  }
  else
  {
    /*************************************************************************/
    /* We have a few bytes left over now, but we know that there are         */
    /* enough bytes spare in the Mbuf to hold them (as we exited the         */
    /* loop above).  So lets copy across to the current offset               */
    /*************************************************************************/
#if !defined (_KERNEL)
    memcpy(&(mtod(x25smbuf_ptr,byte *)[offset]),block_ptr,num_bytes);
#else
    bcopy(block_ptr,&(mtod(x25smbuf_ptr,byte *)[offset]),num_bytes);
#endif
  }
}

/*****************************************************************************/
/*  Function        x25smbuf_guarantee_size                                  */
/*                                                                           */
/*  Prototype       void x25smbuf_guarantee_size (                           */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned num_bytes)                                    */
/*                                                                           */
/*  Description     This routine checks that the Mbuf passed is big enough   */
/*                  to hold num_bytes within the Mbuf chain.  If not, it     */
/*                  will allocate further bytes onto the end of the Mbuf     */
/*                  list.  There are three cases                             */
/*                  1) The mbuf is big enough .. do nothing                  */
/*                  2) The mbuf is just too small, and the last mbuf is      */
/*                     does not have additional memory attached to it.       */
/*                     So we allocate the additional memory and add it to    */
/*                     the last Mbuf, copying the current data in the mbuf   */
/*                     across to the new buffer and adjusting the pointers.  */
/*                  3) The mbuf is far too small and needs at least another  */
/*                     small mbuf to be able to hold the number of bytes     */
/*                     requested. So, we allocate the appropriate number of  */
/*                     large and small Mbufs and chain'em on the end.        */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      num_bytes   The number of bytes to check are available.              */
/*****************************************************************************/
int x25smbuf_guarantee_size(
  mbuf_t *x25smbuf_ptr,
  unsigned num_bytes)
{
  mbuf_t  *new_x25smbuf_ptr, *original_mbuf_ptr;
  unsigned max_length;
  unsigned num_bytes_copy=num_bytes; 

#ifdef DEBUG
  outputf("x25smbuf_guarantee_size: num_bytes=%d\n",num_bytes);
#endif
  
  original_mbuf_ptr = x25smbuf_ptr;

  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error(
      "Error: x25smbuf_guarantee_size passed a null pointer. num_bytes=%x\n",
      num_bytes);
    return -1;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_guarantee_size passed a bad Mbuf pointer. num_bytes=%x mbuf=%x\n",
      num_bytes,
      x25smbuf_ptr);
    return -1;
  }

  /***************************************************************************/
  /* Reduce num_bytes and step along the Mbuf chain until we have run out    */
  /* of Mbufs or we know that we have enough space for num_bytes             */
  /***************************************************************************/
  while (x25smbuf_ptr->m_next!=NULL
    && num_bytes>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    num_bytes-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  if (num_bytes<=READ_MBUF_LENGTH(x25smbuf_ptr))
    ;
  else if (x25smbuf_ptr->m_next==NULL)	  /* Mbuf sequence too small?        */
  {
    /*************************************************************************/
    /* Try and expand the Mbuf within its memory by increasing the           */
    /* data length                                                           */
    /* Can we fit the number of bytes we want to add within the current      */
    /* Mbuf sequence ? If so, expand to fill the space available             */
    /*************************************************************************/
    max_length=READ_MAX_MBUF_LENGTH(x25smbuf_ptr);
    if (num_bytes<=max_length)
    {
      SET_MBUF_LENGTH(x25smbuf_ptr,num_bytes);
    }
    else
    {
      /***********************************************************************/
      /* We do not have enough bytes in the Mbuf sequence, so lets add a     */
      /* new Mbuf to the sequence.                                           */
      /***********************************************************************/
      /***********************************************************************/
      /* 1) Claim enough mbufs for num_bytes minus the maximum that we       */
      /*    can get out of the last Mbuf in the sequence                     */
      /***********************************************************************/
      new_x25smbuf_ptr=x25smbuf_malloc(num_bytes-max_length);
      if (new_x25smbuf_ptr==NULL_MBUF_PTR)
      {
	  x25smbuf_error("Error: x25smbuf_guarantee_size could not extend num_bytes=0x%X max_length=0x%X\n",
			 num_bytes,max_length);	
	  return -1;	
      }
      /***********************************************************************/
      /* 2) Extend the current Mbuf to maximum length                        */
      /* 3) Add the new Mbuf onto the chain                                  */
      /***********************************************************************/
      SET_MBUF_LENGTH(x25smbuf_ptr,max_length);
      x25smbuf_ptr->m_next=new_x25smbuf_ptr;
    }
  }

  if (x25smbuf_length(original_mbuf_ptr)<num_bytes_copy)
  {
    x25smbuf_error(
      "Error: x25smbuf_guarantee_size has failed ... num_bytes=%x mbuf=%x length=%x\n",
      num_bytes_copy,
      x25smbuf_ptr,
      x25smbuf_length(x25smbuf_ptr));
  }

  return 0;
}

/*****************************************************************************/
/*  Function        x25smbuf_cat                                             */
/*                                                                           */
/*  Prototype       void x25smbuf_cat (                                      */
/*                    mbuf_t *base, *extra)                                  */
/*                                                                           */
/*  Description     Adds the Mbuf chain in extra onto the Mbuf chain in      */
/*                  base. No data movement occurs.                           */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      base        The start of an Mbuf list to be added to                 */
/*                  Warning: This must not be NULL.                          */
/*      extra                The list of Mbufs to add to the base            */
/*****************************************************************************/
void x25smbuf_cat(
  mbuf_t *base,
  mbuf_t *extra)
{
  if (base==NULL)
  {
    x25smbuf_error("Error: x25smbuf_cat passed a base null pointer.\n");
    return;
  }
  if (base->m_type!=MT_DATA)
  {
    x25smbuf_error("Error: x25smbuf_cat passed a bad base Mbuf pointer. mbuf=%x\n",base);
    return;
  }

  if (extra==NULL)
  {
    x25smbuf_error("Error: x25smbuf_cat passed an extra null pointer.\n");
    return;
  }
  if (extra->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_cat passed a bad extra Mbuf pointer. mbuf=%x\n",
      extra);
    return;
  }

  /***************************************************************************/
  /* Get base pointing to the last Mbuf in the sequence                      */
  /***************************************************************************/
  while (base->m_next!=NULL)
    base=base->m_next;
  /***************************************************************************/
  /* Concatenate the extra Mbufs to the last element in the Mbuf sequence    */
  /* from base                                                               */
  /***************************************************************************/
  base->m_next=extra;
}

/*****************************************************************************/
/*  Function        x25smbuf_address                                         */
/*                                                                           */
/*  Prototype       byte *x25smbuf_address (                                 */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned offset)                                       */
/*                                                                           */
/*  Description     Calculates the address of the offset within an Mbuf      */
/*                  chain.                                                   */
/*                  Warning:                                                 */
/*                           There is no guarantee that the bytes following  */
/*                           this address are contiguous.  If the offset     */
/*                           you ask for is the last byte in an Mbuf, and    */
/*                           the next link is not NULL, when you attempt to  */
/*                           read 3 bytes from the pointer returned, you     */
/*                           will overrun the end of the Mbuf with BAD       */
/*                           results.                                        */
/*                                                                           */
/*  Return Code                                                              */
/*                  The byte at offset into the Mbuf data.                   */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to read the value from                        */
/*****************************************************************************/
byte *x25smbuf_address(
  mbuf_t *x25smbuf_ptr,
  unsigned offset)
{
  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Invalid offset %x for mbuf address\n",offset);
    return NULL;
  }
  else
  {
    return &(mtod(x25smbuf_ptr,byte *)[offset]);
  }
}

/*****************************************************************************/
/*  Function        x25smbuf_length                                          */
/*                                                                           */
/*  Prototype       unsigned x25smbuf_length (                               */
/*                    mbuf_t *mbuf_ptr)                                      */
/*                                                                           */
/*  Description     Calculates the length of the data currently stored in    */
/*                  the Mbuf. This is the sum of the lengths of data in all  */
/*                  of the Mbufs in the list hanging off mbuf_ptr.           */
/*                  Note: This is not the TOTAL possible length of the Mbuf  */
/*                        chain, this is the actual length calculated from   */
/*                        the m_len's of the Mbufs. Thus if you have a large */
/*                        Mbuf, x25smbuf_length may  return 1 byte iff you   */
/*                        have only put one byte in it!                      */
/*                                                                           */
/*  Return Code                                                              */
/*                  The length of the data sections of all of the Mbufs in   */
/*                  the list                                                 */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    The mbuf list to calculate the length of                 */
/*****************************************************************************/
unsigned x25smbuf_length(
  mbuf_t *x25smbuf_ptr)
{
  unsigned length=0;
  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Error: x25smbuf_length passed a null pointer.");
  }
  else
  {
    while (x25smbuf_ptr!=NULL)
    {
      if (x25smbuf_ptr->m_type!=MT_DATA)
      {
	x25smbuf_error(
	  "Error: x25smbuf_length passed a bad Mbuf pointer. mbuf=%x\n",
	  x25smbuf_ptr);
	break;
      }
      else
      {
	length+=READ_MBUF_LENGTH(x25smbuf_ptr);
	x25smbuf_ptr=x25smbuf_ptr->m_next;
      }
    }
  }
  return length;
}

/*****************************************************************************/
/*  Function        x25smbuf_adjust_forward                                  */
/*                                                                           */
/*  Prototype       void x25smbuf_adjust_forward(                            */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned bytes_to_adjust)                              */
/*                                                                           */
/*  Description     Removes bytes_to_adjust bytes from the beginning of an   */
/*                  Mbuf chain.  This is performed by adjusting the offset   */
/*                  / length pointers, and requires no copying.              */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      bytes_to_adjust                                                      */
/*                  The number of bytes to remove from the front of the Mbuf */
/*                  chain                                                    */
/*****************************************************************************/
void x25smbuf_adjust_forward(
  mbuf_t *x25smbuf_ptr,
  unsigned bytes_to_adjust)
{
  unsigned length=READ_MBUF_LENGTH(x25smbuf_ptr);

  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Error: x25smbuf_adjust_forward passed a null pointer.\n");
    return;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_adjust_forward passed a bad Mbuf pointer.mbuf=%x\n",
      x25smbuf_ptr);
    return;
  }
  if (length<bytes_to_adjust)
  {
    if (x25smbuf_ptr->m_next!=NULL)
    {
      SET_MBUF_LENGTH(x25smbuf_ptr,0);
      x25smbuf_adjust_forward(x25smbuf_ptr->m_next,bytes_to_adjust-length);
    }
    else
    {
      x25smbuf_error("Attempted to adjust %d bytes on a %d buffer\n",
	bytes_to_adjust,
	length);
    }
  }
  else
  {
    SET_MBUF_LENGTH(
      x25smbuf_ptr,
      READ_MBUF_LENGTH(x25smbuf_ptr)-bytes_to_adjust);
    SET_MBUF_DATA(
      x25smbuf_ptr,
      READ_MBUF_DATA(x25smbuf_ptr)+bytes_to_adjust);
  }
}

/*****************************************************************************/
/*  Function        x25smbuf_trim                                            */
/*                                                                           */
/*  Prototype       void x25smbuf_trim (                                     */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned bytes_to_trim)                                */
/*                                                                           */
/*  Description     Removes bytes_to_trim bytes from the end of an           */
/*                  Mbuf chain.  This is performed by adjusting the offset   */
/*                  / length pointers, and requires no copying.              */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      bytes_to_trim                                                        */
/*                  The number of bytes to remove from the end of the Mbuf   */
/*                  chain                                                    */
/*****************************************************************************/
unsigned x25smbuf_trim(
  mbuf_t *x25smbuf_ptr,
  unsigned bytes_to_trim)
{
  unsigned length=READ_MBUF_LENGTH(x25smbuf_ptr);

  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Error: x25smbuf_trim passed a null pointer.");
    return 0;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error("Error: x25smbuf_trim passed a bad Mbuf pointer. mbuf=%x\n",x25smbuf_ptr);
    return 0;
  }
  /***************************************************************************/
  /* First, trim as many bytes as you can off the rest of the Mbuf chain     */
  /***************************************************************************/
  if (x25smbuf_ptr->m_next!=NULL && bytes_to_trim!=0)
    bytes_to_trim=x25smbuf_trim(x25smbuf_ptr->m_next,bytes_to_trim);
  /***************************************************************************/
  /* We still have 'bytes_to_trim' left to remove off the chain              */
  /* If we can do it in this mbuf alone, reduce the length and return that   */
  /* we've trimmed all the bytes that we need to                             */
  /***************************************************************************/
  if (length>=bytes_to_trim)
  {
    length-=bytes_to_trim;
    bytes_to_trim=0;
  }
  else
  {
    /*************************************************************************/
    /* This Mbuf does not have enough bytes in it to remove them all         */
    /* So zero this buffer and return the number of bytes still left to      */
    /* trim                                                                  */
    /*************************************************************************/
    bytes_to_trim-=length;
    length=0;
  }
  SET_MBUF_LENGTH(x25smbuf_ptr,length);
  return bytes_to_trim;
}

/*****************************************************************************/
/*  Function        x25smbuf_adjust_backward                                 */
/*                                                                           */
/*  Prototype       mbuf_t *x25smbuf_adjust_backward(mbuf_ptr,bytes_to_adjust) */
/*                    mbuf_t *mbuf_ptr,                                      */
/*                    unsigned bytes_to_adjust)                              */
/*                                                                           */
/*  Description     Adds bytes_to_adjust bytes to the beginning of an        */
/*                  Mbuf chain.  This is performed by adjusting the offset   */
/*                  / length pointers, and requires no copying.  It may      */
/*                  allocate a new Mbuf, however, so the return code is      */
/*                  important.                                               */
/*                                                                           */
/*  Return Code                                                              */
/*                  The new Mbuf chain.  This may be different from the      */
/*                  mbuf_ptr passed in.                                      */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      bytes_to_adjust                                                      */
/*                  The number of bytes to add to the front of the Mbuf      */
/*                  chain                                                    */
/*****************************************************************************/
mbuf_t *x25smbuf_adjust_backward(
  mbuf_t *x25smbuf_ptr,
  unsigned bytes_to_adjust)
{
  unsigned bytes_spare_at_top=0;
  mbuf_t *new_header_mbuf;

  /***************************************************************************/
  /* The number of bytes spare are the number of bytes between the start of  */
  /* the data area and the current offset value.                             */
  /***************************************************************************/
  if (M_HASCL(x25smbuf_ptr))
    bytes_spare_at_top=mtod(x25smbuf_ptr,byte *)-(byte *)(x25smbuf_ptr+1);
  else
    bytes_spare_at_top=mtod(x25smbuf_ptr,byte *)-x25smbuf_ptr->m_dat;
  
  /***************************************************************************/
  /* Have we got enough spare bytes in the current Mbuf so that we can       */
  /* create the header simply by adjusting offsets.                          */
  /***************************************************************************/
  x25smbuf_ptr->m_data-=bytes_spare_at_top;
  x25smbuf_ptr->m_len+=bytes_spare_at_top;
  bytes_to_adjust-=bytes_spare_at_top;
  if (bytes_spare_at_top>bytes_to_adjust)
  {
    new_header_mbuf=x25smbuf_ptr;
  }
  else
  {
    /*************************************************************************/
    /* Oh well, we'll have to create a new Mbuf ... off we go ..             */
    /*************************************************************************/
    new_header_mbuf=x25smbuf_malloc(bytes_to_adjust);
    if (new_header_mbuf==NULL)
    {
	  x25smbuf_error("x25smbuf_adjust_backward: cannot claim %d additional bytes\n", bytes_to_adjust);
    }
    else
	x25smbuf_cat(new_header_mbuf,x25smbuf_ptr);
  }
  /***************************************************************************/
  /* new_header_mbuf may be a new Mbuf.  The caller should not assume that   */
  /* the old mbuf passed in is now the front of the chain                    */
  /***************************************************************************/
  return new_header_mbuf;
}

/*****************************************************************************/
/*  Function        x25smbuf_copyin                                          */
/*                                                                           */
/*  Prototype       int x25smbuf_copyin(mbuf_ptr,offset,uaddr,len)           */
/*                                                                           */
/*  Description     Copies len bytes from the user space pointer uaddr to    */
/*                  the mbuf at offset.  The Mbuf will be expanded to hold   */
/*                  the data, if necessary.                                  */
/*                                                                           */
/*  Return Code                                                              */
/*      EFAULT      The operation failed.  See copyin() for the reasons.     */
/*      ENOBUFS     No Mbufs available                                       */
/*      X25_EOK     The operation succeeded.                                 */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to write to from                              */
/*      uaddr       Where to get the block of data from.  This will be an    */
/*                  address in user space.                                   */
/*      len         The length of the data to copy                           */
/*****************************************************************************/
int x25smbuf_copyin(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  char *uaddr,
  unsigned num_bytes)
{
  unsigned amount_to_copy;                        /* Bytes to copy in 1 go   */

  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Error: x25smbuf_copyin passed a null pointer.");
    return EINVAL;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_copyin passed a bad Mbuf pointer. mbuf=%x\n",
      x25smbuf_ptr);
    return EINVAL;
  }

  /***************************************************************************/
  /* Make Mbuf big enough to hold all of the data                            */
  /***************************************************************************/
  if (x25smbuf_guarantee_size(x25smbuf_ptr,offset+num_bytes)<0)
    return ENOBUFS;

  /***************************************************************************/
  /* First, set up x25smbuf_ptr so that it is pointing to the buffer where   */
  /* the offset starts.                                                      */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Now, handle the situation where the number of bytes to copyin is LARGER */
  /* than the space in the current mbuf.  If so, we need to copy             */
  /* only the remaining space in the mbuf, NOT the number of bytes to copy.  */
  /* We should carry on doing this until there are enough bytes space in an  */
  /* M buf to cope.                                                          */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && READ_MBUF_LENGTH(x25smbuf_ptr)<offset+num_bytes)
  {
    /*************************************************************************/
    /* Since there are too many bytes in uaddr to copy into this             */
    /* buffer, we must arrange it so that the current Mbuf is filled with    */
    /* as much as possible.  The amount to copy is the size of the buffer    */
    /* without the offset bytes.                                             */
    /*************************************************************************/
    amount_to_copy=READ_MBUF_LENGTH(x25smbuf_ptr)-offset;
#if defined(_KERNEL)
    if (copyin(uaddr,&(mtod(x25smbuf_ptr,byte *)[offset]),amount_to_copy)!=0)
      return EFAULT;
#else
    memcpy(&(mtod(x25smbuf_ptr,byte *)[offset]),uaddr,amount_to_copy);
#endif
    /*************************************************************************/
    /* Adjust the state variables in the right direction.  We have less bytes*/
    /* to copy, we should be copying from further into the block             */
    /*************************************************************************/
    num_bytes-=amount_to_copy;
    uaddr+=amount_to_copy;
    offset=0;
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Oops, asked to copy a block that can't fit in the current Mbuf          */
  /***************************************************************************/
  if (x25smbuf_ptr==NULL && num_bytes!=0)
  {
    x25smbuf_error("Invalid offset %x for mbuf copyin\n",offset);
  }
  else
  {
    /*************************************************************************/
    /* We have a few bytes left over now, but we know that there are         */
    /* enough bytes spare in the Mbuf to hold them (as we exited the         */
    /* loop above).  So lets copy across to the current offset               */
    /*************************************************************************/
#if defined(_KERNEL)
    if (copyin(uaddr,&(mtod(x25smbuf_ptr,byte *)[offset]),num_bytes)!=0)
      return EFAULT;
#else
    memcpy(&(mtod(x25smbuf_ptr,byte *)[offset]),uaddr,num_bytes);
#endif
  }
  return 0;
}

/*****************************************************************************/
/*  Function        x25smbuf_copyout                                         */
/*                                                                           */
/*  Prototype       int x25smbuf_copyin(mbuf_ptr,offset,uaddr,len)           */
/*                                                                           */
/*  Description     Copies len bytes to the user space pointer uaddr from    */
/*                  the mbuf at offset.                                      */
/*                                                                           */
/*  Return Code                                                              */
/*      EFAULT      The operation failed.  See copyout() for the reasons.    */
/*      X25_EOK     The operation succeeded.                                 */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to read from                                  */
/*      uaddr       Where to put the block of data.  This will be an         */
/*                  address in user space.                                   */
/*      len         The length of the data to copy                           */
/*****************************************************************************/
int x25smbuf_copyout(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  char *uaddr,
  unsigned num_bytes)
{
  unsigned amount_to_copy;			  /* Bytes to copy in 1 go   */

  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Error: x25smbuf_copyout passed a null pointer.");
    return EINVAL;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_copyout passed a bad Mbuf pointer. mbuf=%x\n",
      x25smbuf_ptr);
    return EINVAL;
  }

  /***************************************************************************/
  /* First, set up x25smbuf_ptr so that it is pointing to the buffer where   */
  /* the offset starts.                                                      */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Now, handle the situation where the number of bytes to copy is LARGER   */
  /* than the space in the current mbuf.  If so, we need to copy             */
  /* only the remaining space in the mbuf, NOT the number of bytes to copy.  */
  /* We should carry on doing this until there are enough bytes space in an  */
  /* M buf to cope.                                                          */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && READ_MBUF_LENGTH(x25smbuf_ptr)<offset+num_bytes)
  {
    /*************************************************************************/
    /* Since there are too many bytes in uaddr to copy into this             */
    /* buffer, we must arrange it so that the current Mbuf is filled with    */
    /* as much as possible.  The amount to copy is the size of the buffer    */
    /* without the offset bytes.                                             */
    /*************************************************************************/
    amount_to_copy=READ_MBUF_LENGTH(x25smbuf_ptr)-offset;
#if defined(_KERNEL)
    if (copyout(&(mtod(x25smbuf_ptr,byte *)[offset]),uaddr,amount_to_copy)!=0)
      return EFAULT;
#else
    memcpy(uaddr,&(mtod(x25smbuf_ptr,byte *)[offset]),amount_to_copy);
#endif
    /*************************************************************************/
    /* Adjust the state variables in the right direction.  We have less bytes*/
    /* to copy, we should be copying from further into the block             */
    /*************************************************************************/
    num_bytes-=amount_to_copy;
    uaddr+=amount_to_copy;
    offset=0;
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Oops, asked to copy a block that can't fit in the current Mbuf          */
  /***************************************************************************/
  if (x25smbuf_ptr==NULL && num_bytes!=0)
  {
    x25smbuf_error("Invalid offset %x for mbuf copyout\n",offset);
  }
  else
  {
    /*************************************************************************/
    /* We have a few bytes left over now, but we know that there are         */
    /* enough bytes spare in the Mbuf to hold them (as we exited the         */
    /* loop above).  So lets copy across to the current offset               */
    /*************************************************************************/
#if defined(_KERNEL)
    if (copyout(&(mtod(x25smbuf_ptr,byte *)[offset]),uaddr,num_bytes)!=0)
      return EFAULT;
#else
    memcpy(uaddr,&(mtod(x25smbuf_ptr,byte *)[offset]),num_bytes);
#endif
  }
  return 0;
}

#if !defined(_KERNEL)
int emulate_uiomove(
  u_char *kaddr,
  unsigned num_bytes,
  enum uio_rw rw,
  struct uio *uiop)
{
  if (uiop->uio_resid<num_bytes)
  {
    x25smbuf_error(
      "Error: emulate_uiomove move %d bytes when only %d available\n",
      num_bytes,
      uiop->uio_resid);
    num_bytes=uiop->uio_resid;
  }
  switch(rw)
  {
  case UIO_READ:
    memcpy(uiop->uio_iov->iov_base+uiop->uio_offset,kaddr,num_bytes);
    break;
  case UIO_WRITE:
    memcpy(kaddr,uiop->uio_iov->iov_base+uiop->uio_offset,num_bytes);
    break;
  }
  uiop->uio_offset+=num_bytes;
  uiop->uio_resid-=num_bytes;
  return 0;
}
#define uiomove(a,b,c,d) emulate_uiomove((a),(b),(c),(d))
#endif

/*****************************************************************************/
/*  Function        x25smbuf_iomovein                                        */
/*                                                                           */
/*  Prototype       int x25smbuf_iomovein(x25smbuf_ptr,offset,uiop,len)      */
/*                                                                           */
/*  Description     Copies len bytes from the user space io vector pointer   */
/*                  uiop to the mbuf at offset.  The Mbuf will be expanded   */
/*                  to hold the data, if necessary.  For more information    */
/*                  about how this works, see uiomove() in the AIX           */
/*                  Technical Reference.                                     */
/*                                                                           */
/*  Return Code                                                              */
/*      EFAULT      The operation failed.  See uiomove() for the reasons.    */
/*      ENOBUFS     No Mbufs available                                       */
/*      X25_EOK     The operation succeeded.                                 */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to write to from                              */
/*      uoip        Where to get the block of data from.  This will be an    */
/*                  address in user space.                                   */
/*      len         The length of the data to copy                           */
/*****************************************************************************/
int x25smbuf_iomovein(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  struct uio *uiop,
  unsigned num_bytes)
{
  unsigned amount_to_copy;                        /* Bytes to copy in 1 go   */
  int rc;

  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Error: x25smbuf_iomovein passed a null pointer.");
    return EINVAL;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_iomovein passed a bad Mbuf pointer. mbuf=%x\n",
      x25smbuf_ptr);
    return EINVAL;
  }

  /***************************************************************************/
  /* Make Mbuf big enough to hold all of the data                            */
  /***************************************************************************/
  if (x25smbuf_guarantee_size(x25smbuf_ptr,offset+num_bytes)<0)
  {
    x25smbuf_error("Error: x25smbuf_iomovein guarantee size failed.\n");
    return ENOBUFS;
  }

  /***************************************************************************/
  /* First, set up x25smbuf_ptr so that it is pointing to the buffer where   */
  /* the offset starts.                                                      */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Now, handle the situation where the number of bytes to copyin is LARGER */
  /* than the space in the current mbuf.  If so, we need to copy             */
  /* only the remaining space in the mbuf, NOT the number of bytes to copy.  */
  /* We should carry on doing this until there are enough bytes space in an  */
  /* M buf to cope.                                                          */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && READ_MBUF_LENGTH(x25smbuf_ptr)<offset+num_bytes)
  {
    /*************************************************************************/
    /* Since there are too many bytes in uaddr to copy into this             */
    /* buffer, we must arrange it so that the current Mbuf is filled with    */
    /* as much as possible.  The amount to copy is the size of the buffer    */
    /* without the offset bytes.                                             */
    /*************************************************************************/
    amount_to_copy=READ_MBUF_LENGTH(x25smbuf_ptr)-offset;
    rc=uiomove(
      &(mtod(x25smbuf_ptr,byte *)[offset]),
      amount_to_copy,
      UIO_WRITE,
      uiop);
    if (rc!=0)
    {
      x25smbuf_error("Error: x25smbuf_iomovein uiomove failed rc=%d.",rc);
      return rc;
    }
    /*************************************************************************/
    /* Adjust the state variables in the right direction.  We have less bytes*/
    /* to copy, we should be copying from further into the block             */
    /*************************************************************************/
    num_bytes-=amount_to_copy;
    offset=0;
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Oops, asked to copy a block that can't fit in the current Mbuf          */
  /***************************************************************************/
  if (x25smbuf_ptr==NULL && num_bytes!=0)
  {
    x25smbuf_error("Invalid offset %x for mbuf uiomove\n",offset);
    return EIO;
  }
  else
  {
    /*************************************************************************/
    /* We have a few bytes left over now, but we know that there are         */
    /* enough bytes spare in the Mbuf to hold them (as we exited the         */
    /* loop above).  So lets copy across to the current offset               */
    /*************************************************************************/
    if (num_bytes!=0)
    {
      rc=uiomove(&(mtod(x25smbuf_ptr,byte *)[offset]),
	num_bytes,
	UIO_WRITE,
	uiop);

      if (rc!=0)
      {
	x25smbuf_error("Error: x25smbuf_iomovein uiomove2 failed rc=%d.\n",rc);
	return rc;
      }
    }
    return (0);
  }
}

/*****************************************************************************/
/*  Function        x25smbuf_iomoveout                                       */
/*                                                                           */
/*  Prototype       int x25smbuf_iomoveout(x25smbuf_ptr,offset,uiop,len)     */
/*                                                                           */
/*  Description     Copies len bytes to the user space io vec pointer uiop   */
/*                  from the mbuf at offset.  For more information about     */
/*                  how this works, see uiomove() in the AIX Technical       */
/*                  Reference.                                               */
/*                                                                           */
/*  Return Code                                                              */
/*      EFAULT      The operation failed.  See uiomove() for the reasons.    */
/*      X25_EOK     The operation succeeded.                                 */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The offset to read from                                  */
/*      uiop        Where to put the block of data.  This will be an         */
/*                  address in user space.                                   */
/*      len         The length of the data to copy                           */
/*****************************************************************************/
int x25smbuf_iomoveout(
  mbuf_t *x25smbuf_ptr,
  unsigned offset,
  struct uio *uiop,
  unsigned num_bytes)
{
  unsigned amount_to_copy;			  /* Bytes to copy in 1 go   */
  int rc;

  if (x25smbuf_ptr==NULL)
  {
    x25smbuf_error("Error: x25smbuf_iomoveout passed a null pointer.");
    return EINVAL;
  }
  if (x25smbuf_ptr->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Error: x25smbuf_iomoveout passed a bad Mbuf pointer. mbuf=%x\n",
      x25smbuf_ptr);
    return EINVAL;
  }

  /***************************************************************************/
  /* First, set up x25smbuf_ptr so that it is pointing to the buffer where   */
  /* the offset starts.                                                      */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && offset>=READ_MBUF_LENGTH(x25smbuf_ptr))
  {
    offset-=READ_MBUF_LENGTH(x25smbuf_ptr);
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Now, handle the situation where the number of bytes to copy is LARGER   */
  /* than the space in the current mbuf.  If so, we need to copy             */
  /* only the remaining space in the mbuf, NOT the number of bytes to copy.  */
  /* We should carry on doing this until there are enough bytes space in an  */
  /* M buf to cope.                                                          */
  /***************************************************************************/
  while (x25smbuf_ptr!=NULL && READ_MBUF_LENGTH(x25smbuf_ptr)<offset+num_bytes)
  {
    /*************************************************************************/
    /* Since there are too many bytes in uaddr to copy into this             */
    /* buffer, we must arrange it so that the current Mbuf is filled with    */
    /* as much as possible.  The amount to copy is the size of the buffer    */
    /* without the offset bytes.                                             */
    /*************************************************************************/
    amount_to_copy=READ_MBUF_LENGTH(x25smbuf_ptr)-offset;
    rc=uiomove(&(mtod(x25smbuf_ptr,byte *)[offset]),amount_to_copy,UIO_READ,uiop);
    if (rc!=0)
      return rc;
    /*************************************************************************/
    /* Adjust the state variables in the right direction.  We have less bytes*/
    /* to copy, we should be copying from further into the block             */
    /*************************************************************************/
    num_bytes-=amount_to_copy;
    offset=0;
    x25smbuf_ptr=x25smbuf_ptr->m_next;
  }
  /***************************************************************************/
  /* Oops, asked to copy a block that can't fit in the current Mbuf          */
  /***************************************************************************/
  if (x25smbuf_ptr==NULL && num_bytes!=0)
  {
    x25smbuf_error("Invalid offset %x for mbuf iomoveout\n",offset);
    return EIO;
  }
  else
  {
    /*************************************************************************/
    /* We have a few bytes left over now, but we know that there are         */
    /* enough bytes spare in the Mbuf to hold them (as we exited the         */
    /* loop above).  So lets copy across to the current offset               */
    /*************************************************************************/
    if (num_bytes!=0)
    {
      rc=uiomove(
	&(mtod(x25smbuf_ptr,byte *)[offset]),
	num_bytes,
	UIO_READ,
	uiop);
    }
    else
      rc=0;
    return rc;
  }
}

#if !defined(_KERNEL)
/*****************************************************************************/
/*  Function        x25smbuf_dump                                            */
/*                                                                           */
/*  Prototype       void x25smbuf_dump(mbuf_ptr,offset)                      */
/*                  mbuf_t *mbuf_ptr;                                        */
/*                  unsigned offset;                                         */
/*                                                                           */
/*  Description     This dumps out the bytes in an Mbuf from the offset to   */
/*                  the end of the Mbuf.                                     */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      mbuf_ptr    A pointer to the Mbuf chain.                             */
/*      offset      The starting point for the dump, i.e. a value of 0 will  */
/*                  dump out all of the Mbuf.                                */
/*****************************************************************************/
void x25smbuf_dump(
  mbuf_t *x25smbuf_ptr,
  unsigned offset)
{
  unsigned mbuf_length;
  int c;
  char string[20];
  int i;
  int finished=FALSE;
  int num_bytes=0;

  string[0]= '\0';
  i=16;
  mbuf_length=x25smbuf_length(x25smbuf_ptr);
  if (mbuf_length<offset)
    x25smbuf_error("%.3X=> Exceeded Mbuf length length=%x\n",offset,mbuf_length);
  else if (mbuf_length==offset)
    x25smbuf_error("%.3X=> No data\n",offset);
  else
  {
    for (;finished==FALSE;++offset,++num_bytes)
    {
      if (offset<mbuf_length)
	c = x25smbuf_read_byte(x25smbuf_ptr,offset);
      else
	c = -1;
      if ((i==2) || (i==6) || (i==10) || i==14)
	outputf(".");
      if ((i==4) || (i==12))
	outputf("..");
      if (i==8)
	outputf("...",offset);
      if (i==16)
      {
	string[i]=0;
	if (num_bytes!=0)
	  outputf(" [%s]",string);
	string[0]= '\0';
	if (offset>=mbuf_length)
	{
	  finished=TRUE;
	  continue;
	}
	if (num_bytes!=0)
	  outputf("\n");
	outputf("%.3x=>",offset);
	i=0;
      }
      if (c>=0)
	outputf("%.2X",c);
      else
	outputf("**",c);
      if(c > 0x1f && c < 0x7f )
	string[i]=c;
      else
	string[i]='.';
      i++;
      fflush(stdout);
    }
  }
  fflush(stdout);
  outputf("\n");
}
#endif

/*****************************************************************************/
/*  Function        jsmlist_init                                             */
/*                                                                           */
/*  Prototype       void jsmlist_init(                                       */
/*                    jsmlist_t *list_addr)                                  */
/*                                                                           */
/*  Description     Initialises a list header.  This makes the length of     */
/*                  the list zero.                                           */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      list_addr   The list header to be initialised                        */
/*****************************************************************************/
void jsmlist_init(
  jsmlist_t *list_addr)
{
  list_addr->first = NULL;
  list_addr->last = NULL;
  return;
}

/*****************************************************************************/
/*  Function        jsmlist_element_init                                     */
/*                                                                           */
/*  Prototype       void jsmlist_element_init(                               */
/*                    mbuf_t *link_ptr)                                      */
/*                                                                           */
/*  Description     Initialises the list pointers within an element, about   */
/*                  to be put onto a list.  Do not use this function if the  */
/*                  element is already part of a list.                       */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      link_ptr    A pointer to an element to be initialised.               */
/*****************************************************************************/
void jsmlist_element_init(
  mbuf_t *element)
{
  SET_NEXT_JSMLIST_ELEMENT(element,NULL);
}

/*****************************************************************************/
/*  Function        jsmlist_enq                                              */
/*                                                                           */
/*  Prototype       void jsmlist_enq(                                        */
/*                    jsmlist_t *list_addr;                                  */
/*                    mbuf_t *element);                                      */
/*                                                                           */
/*  Description     Enqueues an element on the tail of a list.               */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      list_addr   The list to add to                                       */
/*      element     The item to add to the list.                             */
/*****************************************************************************/
void jsmlist_enq(
  jsmlist_t *list_addr,
  mbuf_t *element)
{
  SET_NEXT_JSMLIST_ELEMENT(element,NULL);

  if (list_addr->last != NULL)
    SET_NEXT_JSMLIST_ELEMENT(list_addr->last,element);
  else
    list_addr->first = element;			  /* Empty List              */

  list_addr->last = element;
}

/*****************************************************************************/
/*  Function        jsmlist_add_to_front                                     */
/*                                                                           */
/*  Prototype       void jsmlist_add_to_front(                               */
/*                    jsmlist_t *list_addr;                                  */
/*                    mbuf_t *element);                                      */
/*                                                                           */
/*  Description     Enqueues an element on the front of a list.              */
/*                                                                           */
/*  Return Code                                                              */
/*                  None                                                     */
/*                                                                           */
/*  Parameters                                                               */
/*      list_addr   The list to add to                                       */
/*      element     The item to add to the list.                             */
/*****************************************************************************/
void jsmlist_add_to_front(
  jsmlist_t *list_addr,
  mbuf_t *element)
{
  SET_NEXT_JSMLIST_ELEMENT(element,list_addr->first);

  list_addr->first = element;
  if (list_addr->last == NULL_LIST_PTR)
    list_addr->last=element;			  /* Empty List ?            */

  return;
}

/*****************************************************************************/
/*  Function        jsmlist_deq                                              */
/*                                                                           */
/*  Prototype       mbuf_t *jsmlist_deq(                                     */
/*                    jsmlist_t *list_addr)                                  */
/*                                                                           */
/*  Description     Removes an element from the head of the list.            */
/*                                                                           */
/*  Return Code                                                              */
/*                  A pointer to the start of the list element.  This will   */
/*                  generally be cast into the appropriate data type for     */
/*                  the call. If the list is empty, NULL is returned.        */
/*                                                                           */
/*  Parameters                                                               */
/*      list_addr   The list to remove an element from.                      */
/*****************************************************************************/
mbuf_t *jsmlist_deq(
  jsmlist_t *list_addr)
{
  mbuf_t *element=list_addr->first;

  if (element!=NULL_LIST_PTR)			  /* Empty List ?            */
  {
    list_addr->first = READ_NEXT_JSMLIST_ELEMENT(element);

    if (list_addr->first == NULL_LIST_PTR)
      list_addr->last = NULL_LIST_PTR;
  }

  return(element);
}

/*****************************************************************************/
/* This is used to remove an element from a list of Mbufs when the Mbuf      */
/* might have been deallocated and used for other purposes.  This does       */
/* not refer to the contents of the element at all.  However, it must be     */
/* provided with the previous contents of the READ_NEXT_JSMLIST_ELEMENT      */
/* for the element.  This field is normally stored within the element, but   */
/* cannot be accessed in this case because the element may have been freed   */
/* or attached to another list                                               */
/*****************************************************************************/
jsl_rc_t jsmlist_rem_without_access(
  jsmlist_t *list_addr,
  mbuf_t *element,
  mbuf_t *next_link)
{
  mbuf_t *current_element;
  mbuf_t *parent_element;

  /***************************************************************************/
  /* Search all the elements in the list until you find element              */
  /***************************************************************************/
  current_element=list_addr->first;
  parent_element=NULL;

  if (next_link!=NULL && next_link->m_type!=MT_DATA)
  {
    x25smbuf_error(
      "Cannot remove mbuf %x as the next link %x is invalid",
      element,
      next_link);
    return jsl_not_found;
  }

  while (current_element!=NULL)
  {
    if (current_element == element)		  /* Found the element?      */
    {
      if (parent_element==NULL)			  /* Front of the list ?     */
      {
	list_addr->first=next_link;
      }
      else
      {
	SET_NEXT_JSMLIST_ELEMENT(parent_element,next_link);
      }

      /***********************************************************************/
      /* If the item being removed is the last in the list, the new list     */
      /* will have the parent element as the last in the list                */
      /* This also handles the empty list case, as if the item being         */
      /* removed is the last in the list, then the parent_element is set to  */
      /* NULL                                                                */
      /***********************************************************************/
      if (list_addr->last==element)
      {
	list_addr->last= parent_element;
      }
      return(jsl_found);
    }
    /*************************************************************************/
    /* Round we go again.  The parent will be the current element and the    */
    /* current element will be the next element in the list                  */
    /*************************************************************************/
    parent_element=current_element;
    current_element=READ_NEXT_JSMLIST_ELEMENT(current_element);
  }
  return(jsl_not_found);
}

/*****************************************************************************/
/*  Function        jsmlist_empty                                            */
/*                                                                           */
/*  Prototype       bool jsmlist_empty(                                      */
/*                    jsmlist_t *list_addr)                                  */
/*                                                                           */
/*  Description     Tests to see if the list has any elements on it          */
/*                                                                           */
/*  Return Code                                                              */
/*      TRUE        The list is empty.  It is an empty list.                 */
/*      FALSE       The list is not empty.  It has elements on it.           */
/*                                                                           */
/*  Parameters                                                               */
/*      list_addr   The list to test                                         */
/*****************************************************************************/
bool jsmlist_empty(
  jsmlist_t *list_addr)
{
  return (list_addr->first == NULL_LIST_PTR);
}

/*****************************************************************************/
/*  Function        jsmlist_length                                           */
/*                                                                           */
/*  Prototype       unsigned jsmlist_length(                                 */
/*                    jsmlist_t *list_addr)                                  */
/*                                                                           */
/*  Description     Calculates the number of elements in the list pointed    */
/*                  to by list_addr.                                         */
/*                                                                           */
/*  Return Code                                                              */
/*                  The number of elements in the list.                      */
/*                                                                           */
/*  Parameters                                                               */
/*      list_addr   The list to determine the length of.                     */
/*****************************************************************************/
int jsmlist_length(
  jsmlist_t *list_addr)
{
  mbuf_t *element;
  int length=0;

  for (element=list_addr->first;
    element!=NULL;
    element=READ_NEXT_JSMLIST_ELEMENT(element))
    ++length;
  return length;
}

/*****************************************************************************/
/*  Function        jsmlist_rem                                              */
/*                                                                           */
/*  Prototype       gpl_rc_t jsmlist_rem(                                    */
/*                    jsmlist_t *list_addr,                                  */
/*                    mbuf_t *element)                                       */
/*                                                                           */
/*  Description     Removes an element from the list.  This does not         */
/*                                                                           */
/*                  have to be the top element (see jsmlist_deq), so it is   */
/*                                                                           */
/*                  a bit more flexible.                                     */
/*                                                                           */
/*  Return Code                                                              */
/*      jsl_found   The element was found on the list and removed.           */
/*      jsl_not_found                                                        */
/*                  The element was not on the list.  The list               */
/*                  is unchanged                                             */
/*                                                                           */
/*  Parameters                                                               */
/*      list_addr   The list to remove from                                  */
/*      element     The element to remove                                    */
/*****************************************************************************/
jsl_rc_t jsmlist_rem(
  jsmlist_t *list_addr,
  mbuf_t *element)
{
  if (element==NULL || element->m_type!=MT_DATA)
  {
    x25smbuf_error("Cannot remove mbuf %x as it is invalid",element);
    return jsl_not_found;
  }

  if (element!=NULL)
  {
    if (jsmlist_rem_without_access(
      list_addr,
      element,
      READ_NEXT_JSMLIST_ELEMENT(element))==jsl_found)
    {
      SET_NEXT_JSMLIST_ELEMENT(element,NULL);	  /* No longer in a list     */
      return jsl_found;
    }
  }
  return jsl_not_found;
}

/*****************************************************************************/
/*  Function        jsmlist_read_head                                        */
/*                                                                           */
/*  Prototype       mbuf_t *jsmlist_read_head(                               */
/*                    jsmlist_t *list_addr);                                 */
/*                                                                           */
/*  Description     Reads the top element of a list                          */
/*                                                                           */
/*  Return Code                                                              */
/*                  The top element of the list or NULL if the list is empty */
/*                                                                           */
/*  Parameters                                                               */
/*      list_addr   The list to read the head of                             */
/*****************************************************************************/
mbuf_t *jsmlist_read_head(
  jsmlist_t *list_addr)
{
  return list_addr->first;
}

/*****************************************************************************/
/*  Function        jsmlist_iterate_first                                    */
/*                                                                           */
/*  Prototype       mbuf_t *jsmlist_iterate_first(                           */
/*                    jsmlist_iterate_t *list_iter,                          */
/*                    jsmlist_t *list_addr)                                  */
/*                                                                           */
/*  Description     This starts up a search along a list.  This is similar   */
/*                  to DOS find_first, find_next style calls.  You call the  */
/*                  jsmlist_iterate_first function to initiate the search.   */
/*                  Then you call jsmlist_iterate_next until it returns      */
/*                  NULL. If you want to perform an action on all items in   */
/*                  a list                                                   */
/*                     1) Call iterate_first to find the first item on the list */
/*                  2) If the returned item is NULL, finish the loop         */
/*                  3) Apply the action to the item.                         */
/*                  4) Call iterate_next to go to the next item              */
/*                  5) Goto 2.                                               */
/*                  This allows iteration over the items on a list.          */
/*                                                                           */
/*  Return Code                                                              */
/*                  A pointer to the first element in the list or NULL if    */
/*                  the list is empty.                                       */
/*                                                                           */
/*                  The following example will call the                      */
/*                  some_processing_on_the_item for each item in a list.     */
/*                                                                           */
/*                    mbuf_t *item_ptr;                                      */
/*                                                                           */
/*                    jsmlist_iterate_t list_iter;                           */
/*                                                                           */
/*                                                                           */
/*                    item_ptr=jsmlist_iterate_first(                        */
/*                      &list_iter,                                          */
/*                      list_addr);                                          */
/*                                                                           */
/*                    while (iterm_ptr!=NULL)                                */
/*                    {                                                      */
/*                      some_processing_on_the_item(...);                    */
/*                      item_ptr=jsmlist_iterate_next (                      */
/*                         &list_iter);                                      */
/*                    };                                                     */
/*                                                                           */
/*                                                                           */
/*  Parameters                                                               */
/*      list_addr   The list to iterate over                                 */
/*      list_iter   An internal control block that must be passed to the     */
/*                                                                           */
/*                  jsmlist_iterate_next routine.  See the example above     */
/*                  for  usage.                                              */
/*****************************************************************************/
mbuf_t *jsmlist_iterate_first(
  jsmlist_iterate_t *list_iter,
  jsmlist_t *list_addr)
{
  /***************************************************************************/
  /* List iter holds a pointer to the next element in the list.  So whenever */
  /* list_iterate_next is called, it can                                     */
  /* a) return the next item                                                 */
  /* b) update it's contents so that it points to the next item's next item  */
  /* To start this all off, we just have to set the next_item to consider    */
  /* to be the first item, and get list_iterate_next to do the rest          */
  /***************************************************************************/
  *list_iter=list_addr->first;			  
  return jsmlist_iterate_next(list_iter);
}

/*****************************************************************************/
/*  Function        jsmlist_iterate_next                                     */
/*                                                                           */
/*  Prototype       mbuf_t *jsmlist_iterate_next(                            */
/*                    jsmlist_iterate_t *list_iter)                          */
/*                                                                           */
/*  Description     This continues a search along a list.  This is similar   */
/*                  to DOS find_first, find_next style calls.  You call the  */
/*                  jsmlist_iterate_first function to initiate the search.   */
/*                  Then you call jsmlist_iterate_next until it returns      */
/*                  NULL.                                                    */
/*                                                                           */
/*  Return Code                                                              */
/*                  A pointer to the first element in the list or NULL if    */
/*                  the list is empty.                                       */
/*                                                                           */
/*  Parameters                                                               */
/*      list_addr   The list to iterate over                                 */
/*      list_iter   An internal control block that was filled in by          */
/*                  the jsmlist_iterate_first routine.                       */
/*****************************************************************************/
mbuf_t *jsmlist_iterate_next(
  jsmlist_iterate_t *list_iter)
{
  mbuf_t *list_ptrs;
  list_ptrs= *list_iter;
  
  /***************************************************************************/
  /* Is the list empty ? If not, save the next element so that we can pick   */
  /* up where we left off on the next list iterate call                      */
  /***************************************************************************/
  if (list_ptrs!=NULL_LIST_PTR)
  {
    *list_iter=READ_NEXT_JSMLIST_ELEMENT(list_ptrs);
  }
  return list_ptrs;
}

