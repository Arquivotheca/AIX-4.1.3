static char sccsid[] = "@(#)88	1.2  src/bos/diag/tu/ktat/kent_utilities.c, tu_ktat, bos41J, 9519A_all 5/3/95 15:01:07";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: bld_desc
 *		dma_cleanup
 *		hexdump
 *		swap32
 *		swap_lo16
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
 *****************************************************************************/

#include <stdio.h>
#include <memory.h>
#include <sys/diagex.h>
#include "kent_defs.h"
#include "kent_tu_type.h"

extern diag_struc_t  *diagex_hdl;
extern diagex_dds_t   dds;
/* extern *t_buf[max_desc];
extern *r_buf[max_desc];
extern w_pat[]; */
extern tx_desc_t t_desc[max_desc];
extern rx_desc_t r_desc[max_desc];
extern dma_addr;
extern ulong r_desc0_dma_addr;
extern ulong r_desc1_dma_addr;
extern ulong r_desc2_dma_addr;
extern ulong r_desc3_dma_addr;
extern ulong t_desc0_dma_addr;
extern ulong t_desc1_dma_addr;
extern ulong t_desc2_dma_addr;
extern ulong t_desc3_dma_addr;

swap32(data)
  int data;
{
  int s_data;
  s_data = ((data & 0xff000000) >> 24) | ((data & 0x00ff0000) >> 8) |
           ((data & 0x0000ff00) << 8) | ((data & 0x000000ff) << 24);
    return(s_data);
}

swap_lo16(data)
  int data;
{
  int s_data;
  s_data = ((data & 0x0000ff00) >> 8) | ((data & 0x000000ff) << 8);
    return(s_data);
}

  ulong tbuf0_dma_addr, tbuf1_dma_addr, tbuf2_dma_addr, tbuf3_dma_addr;
  ulong rbuf0_dma_addr, rbuf1_dma_addr, rbuf2_dma_addr, rbuf3_dma_addr;
  char w_pat[] = {0xaa, 0x55, 0x3c, 0xc3}; 
  char *big_buf;
  #define big_buf_size 0x6000
  #define buf_size1 0x800
  char *tbuf0, *tbuf1, *tbuf2, *tbuf3;
  char *rbuf0, *rbuf1, *rbuf2, *rbuf3;
  tx_desc_t *t_desc0, *t_desc1, *t_desc2, *t_desc3;
  rx_desc_t *r_desc0, *r_desc1, *r_desc2, *r_desc3;
  char *desc_malloc_buf;
  #define big_desc_buf_size 0x00000200
  #define desc_size 0x00000010




bld_desc()
{
  int i, j, rc, junk;
  ulong buf_start;

/* printf("Entering BLD_DESC\n"); */
/*  malloc memory for transmit and receive buffers   */

  if((big_buf = (char *)malloc(big_buf_size)) == NULL)
  {
    return(OUT_OF_MEMORY);
  }
  buf_start = (((ulong)big_buf & 0xfffff000) + 0x00001000);
/*
printf("back from  call to malloc\n");
printf("bigbuf = 0x%08x\n", big_buf);
printf("start buf = 0x%08x\n", buf_start);
scanf("%d", &junk);
*/

  tbuf0 = (char *)buf_start;
  tbuf1 = (char *)buf_start + buf_size;
  tbuf2 = tbuf1 + buf_size;
  tbuf3 = tbuf2 + buf_size;

  rbuf0 = tbuf3 + buf_size;
  rbuf1 = rbuf0 + buf_size1;
  rbuf2 = rbuf1 + buf_size1;
  rbuf3 = rbuf2 + buf_size1;
/*
printf("tbuf0  = 0x%08x\n", tbuf0);
printf("tbuf1  = 0x%08x\n", tbuf1);
printf("tbuf2  = 0x%08x\n", tbuf2);
printf("tbuf3  = 0x%08x\n", tbuf3);
printf("rbuf0  = 0x%08x\n", rbuf0);
printf("rbuf1  = 0x%08x\n", rbuf1);
printf("rbuf2  = 0x%08x\n", rbuf2);
printf("rbuf3  = 0x%08x\n", rbuf3);
scanf("%d", &junk);
*/
/*  malloc memory for transmit and receive descripters  */

  if((desc_malloc_buf = (char *)malloc(big_desc_buf_size)) == NULL)
  {
    return(OUT_OF_MEMORY);
  }
  buf_start = (((ulong)desc_malloc_buf & 0xffffff00) + 0x00000100);
  t_desc0 = (tx_desc_t *)buf_start;
  t_desc1 = t_desc0 + 1; 
  t_desc2 = t_desc1 + 1;
  t_desc3 = t_desc2 + 1; 

  r_desc0 = (rx_desc_t *)t_desc3 + 1;
  r_desc1 = r_desc0 + 1;
  r_desc2 = r_desc1 + 1;
  r_desc3 = r_desc2 + 1;

/* initialize transmit  and receive buffers   */

  memset(tbuf0, 0x01, buf_size);
  memset(tbuf1, 0x02, buf_size);
  memset(tbuf2, 0x03, buf_size);
  memset(tbuf3, 0x04, buf_size);
  memset(rbuf0, 0, buf_size1);
  memset(rbuf1, 0, buf_size1);
  memset(rbuf2, 0, buf_size1);
  memset(rbuf3, 0, buf_size1);
/*
  hexdump(rbuf1, 128);
  scanf("%d", &junk);
*/
  if(rc = dma_setup(tbuf0, &tbuf0_dma_addr, DMA_READ, buf_size))
  {
    printf("failed tbuf0\n");
    return(rc);
  }
  t_desc0 -> tbadr = swap32(tbuf0_dma_addr);
  t_desc0 -> tmd1 = tmd1_set;
  t_desc0 -> tmd2 = tmd2_set;

  if(rc = dma_setup(tbuf1, &tbuf1_dma_addr, DMA_READ, buf_size))
  {
    return(rc);
  }
  t_desc1 -> tbadr = swap32(tbuf1_dma_addr);
  t_desc1 -> tmd1 = tmd1_set;
  t_desc1 -> tmd2 = tmd2_set;

  if(rc = dma_setup(tbuf2, &tbuf2_dma_addr, DMA_READ, buf_size))
  {
    return(rc);
  }
  t_desc2 -> tbadr = swap32(tbuf2_dma_addr);
  t_desc2 -> tmd1 = tmd1_set;
  t_desc2 -> tmd2 = tmd2_set;

  if(rc = dma_setup(tbuf3, &tbuf3_dma_addr, DMA_READ, buf_size))
  {
    return(rc);
  }
  t_desc3 -> tbadr = swap32(tbuf3_dma_addr);
  t_desc3 -> tmd1 = tmd1_set;
  t_desc3 -> tmd2 = tmd2_set;


  if(rc = dma_setup(rbuf0, &rbuf0_dma_addr, DMA_READ, buf_size1))
  {
    return(rc);
  }
  r_desc0 -> rbadr = swap32(rbuf0_dma_addr);
  r_desc0 -> rmd1 = rmd1_set;
  r_desc0 -> rmd2 = rmd2_set;

  if(rc = dma_setup(rbuf1, &rbuf1_dma_addr, DMA_READ, buf_size1))
  {
    return(rc);
  }
  r_desc1 -> rbadr = swap32(rbuf1_dma_addr);
  r_desc1 -> rmd1 = rmd1_set;
  r_desc1 -> rmd2 = rmd2_set;

  if(rc = dma_setup(rbuf2, &rbuf2_dma_addr, DMA_READ, buf_size1))
  {
    return(rc);
  }
  r_desc2 -> rbadr = swap32(rbuf2_dma_addr);
  r_desc2 -> rmd1 = rmd1_set;
  r_desc2 -> rmd2 = rmd2_set;

  if(rc = dma_setup(rbuf3, &rbuf3_dma_addr, DMA_READ, buf_size1))
  {
    return(rc);
  }
  r_desc3 -> rbadr = swap32(rbuf3_dma_addr);
  r_desc3 -> rmd1 = rmd1_set;
  r_desc3 -> rmd2 = rmd2_set;

  return(0);
}  /*  end bld_desc   */

/******************************************************************
* dma_cleanup
* 
* call dam_finish and free buffers
******************************************************************/

dma_cleanup()
{
  int rc, junk;

  if (rc = dma_finish(&r_desc0_dma_addr))
    return(rc);
  if (rc = dma_finish(&r_desc1_dma_addr))
    return(rc);
  if (rc = dma_finish(&r_desc2_dma_addr))
    return(rc);
  if (rc = dma_finish(&r_desc3_dma_addr))
    return(rc);
  if (rc = dma_finish(&t_desc0_dma_addr))
    return(rc);
  if (rc = dma_finish(&t_desc1_dma_addr))
    return(rc);
  if (rc = dma_finish(&t_desc2_dma_addr))
    return(rc);
  if (rc = dma_finish(&t_desc3_dma_addr))
    return(rc);

  if(rc = dma_finish(&tbuf0_dma_addr))
    return(rc);
  if(rc = dma_finish(&tbuf1_dma_addr))
    return(rc);
  if(rc = dma_finish(&tbuf2_dma_addr))
    return(rc);
  if(rc = dma_finish(&tbuf3_dma_addr))
    return(rc);

  if(rc = dma_finish(&rbuf0_dma_addr))
    return(rc);
  if(rc = dma_finish(&rbuf1_dma_addr))
    return(rc);
  if(rc = dma_finish(&rbuf2_dma_addr))
    return(rc);
  if(rc = dma_finish(&rbuf3_dma_addr))
    return(rc);

return(0);

}  /* end dma_cleanup  */

release_memory()
{
  int rc;
 
  free(big_buf);
  free(desc_malloc_buf);
return(0);
}

/****************************************************************************
 * NAME: hexdump
 *
 * FUNCTION: Display an array of type char in ASCII, and HEX.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is ONLY AVAILABLE IF COMPILED WITH DEBUG DEFINED
 *
 * RETURNS: NONE
 ****************************************************************************/

hexdump(data,len)
char *data;
long len;
{

        int     i,j,k;
        char    str[18];

        printf("hexdump(): length=%ld\n",len);
        i=j=k=0;
        while(i<len)
        {
                j=(int) data[i++];
                if(j>=32 && j<=126)
                        str[k++]=(char) j;
                else
                        str[k++]='.';
                printf("%02x",j);
                if(!(i%8))
                {
                        printf("  ");
                        str[k++]=' ';
                }
                if(!(i%16))
                {
                        str[k]='\0';
                        printf("     %s\n",str);
                        k=0;
                }
        }
        while(i%16)
        {
                if(!(i%8))
                        printf("  ");
                printf("   ");
                i++;
        }
        str[k]='\0';
        printf("       %s\n",str);
}


