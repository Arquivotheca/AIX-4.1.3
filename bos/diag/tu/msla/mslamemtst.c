static char sccsid[] = "@(#)67  1.4.1.1  src/bos/diag/tu/msla/mslamemtst.c, tu_msla, bos41J, 9517A_all 4/25/95 08:44:57";
/*
 * COMPONENT_NAME: ( mslamemtst ) 
 *
 * FUNCTIONS:  memtest, mslardwr, msla32bxfer, fillsrc, load_p
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME : mslamemtst.c                                          */
/*                                                                           */
/*         POURPOSE : Use the HTX interface to test MSLA  for the following  */
/*                    memtest to test the static 64KRAM on the microchannel  */
/*                    Interface Card.                                        */
/*                                                                           */
/*            INPUT : Pointer to the msla structure.    		     */
/*                                                                           */
/*           OUTPUT : Returns the proper code.				     */
/*                                                                           */
/* FUNCTIONS CALLED : mslardwr, mkerr.                                       */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include "mslaerrdef.h"
#include "mslafdef.h"
#include "mslamemdat.h"
#include "mslatu.h"
#include "msla_diag.h"

/* global declarations so that we do malloc only one time */
static    unsigned long *srcmem=0;
static    unsigned long *sinkmem=0;
static    unsigned long *srcmemp=0;
static    unsigned long *sinkmemp=0;

int
memtest(gtu)
struct mslatu *gtu;
{

    int rc;
    int fd, slot;
    unsigned long lomem, himem;
    unsigned long bussmem, ioaddr, ram_size;
    unsigned short bad_addr;
    void load_p();

    lomem = gtu->tu1;
    himem = gtu->tu2;
    bussmem = gtu->msla_membase;
    fd      = gtu->fd  ;
    ioaddr  = gtu->msla_iobase ;

    if ( lomem >= himem )
    {
        ram_size = MAX_16BIT ;  /* MAX_16BIT = 65536 */
    }
    else
    {
        bussmem = bussmem | lomem ;
        ram_size = himem - lomem ;
    }

    bad_addr = 0;

   /* bad_addr updated on error */
    rc = mslardwr(fd,bussmem,ioaddr,ram_size,&bad_addr);
    if ( rc != SUCCESS )
    {
        mkerr(MEMTEST,MSLARDWR,rc,&rc);
        return(rc);
    }


    rc = msla32bxfer(fd,bussmem,ioaddr,ram_size,&bad_addr) ; 
    if ( rc != SUCCESS )
    {
        mkerr(MEMTEST,MSLARDWR32,rc,&rc);
        return(rc);
    }

    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  mslardwr                                              */
/*                                                                           */
/*       MODULE No  :  1.01.01                                               */
/*                                                                           */
/*         POURPOSE :  Verify that the R2 can read nd write the MSLA ram     */
/*                     using byte operations to both odd and even addresses  */
/*                     and then verify word operations.                      */
/*                                                                           */
/*            INPUT :  fd,slot,bussmem,ioaddr,ram_size,bad_address    	     */
/*                                                                           */
/*           OUTPUT :  							     */
/*                    A1. BAD ADDRESS                                        */ 
/*                          OR XXXX ADDRESS OF FAILING MEMORY!               */ 
/*                    A2. RETURN CODE                                        */ 
/*                          0 => ALL RESULTS GOOD                            */ 
/*                          1 => ROUTINE FAILS WORD TEST                     */ 
/*                          2 => ROUTINE FAILS BYTE TEST                     */ 
/*                                                                           */
/* FUNCTIONS CALLED :  None				                     */
/*                                                                           */
/*  DATE of Creation: 10/05/1988                                             */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

int
mslardwr(fd,bussmem,ioaddr,ram_size,bad_address)
int fd;
unsigned long bussmem;
unsigned long ioaddr;
unsigned long ram_size;
unsigned short *bad_address;
/* Returns the bad address as Offset from The MSLA memory @0xF4E80000 */
{

    int rc ;
    int byte_test, word_test, word_ndx;
    unsigned int byte_ndx;
    char read_byte, loop;
    unsigned short temp_data  ;
    unsigned int address ;
    unsigned short *bus_address, *bus_addr ;
    char *bus_addr_char;


    rc = SUCCESS;
    pre_load_setup(fd,ioaddr);

    /* 
    *********************************************
    *   THE BYTE ADDRESS READ/WRITE IS TESTED   *
    *********************************************
    */

    for (byte_test = 0,loop = SUCCESS; 
        ((byte_test < NO_OF_TESTBYTES) && ( loop == SUCCESS ));
          byte_test++)
    {
	for (byte_ndx = 0; byte_ndx < ram_size; byte_ndx++)
        {
            address =  bussmem | byte_ndx ;
            bus_addr_char = (char *) address ;

            *bus_addr_char = rdwr_byte[byte_test] ;      /* write and*/
            read_byte = *bus_addr_char ;                 /* read  it */
            if ( read_byte  == rdwr_byte[byte_test])
            {
                *bus_addr_char = read_byte;                 /*fill 00*/
            }
            else
            {
                rc = MSLARDWR_BYTE_FAIL;          /* byte test fails */
                *bad_address = (short )byte_ndx  ;  /*Returning ofset*/
                loop = FAIL;
                break;                      /* no more checking */
            }
        }
    }

    if ( rc != SUCCESS )
    {
	return(rc);
    }


    /* 
    **********************************************
    *   THE WORD ADDRESS READ/WRITE IS TESTED    *
    **********************************************
    */

    for (word_test = 0,loop = SUCCESS; 
        ((word_test < NO_OF_TESTBYTES) && ( loop == SUCCESS ));
          word_test++)
    {
        load_p(bussmem,msla_detect[word_test],ram_size/2,0,fd,ioaddr);

        for(word_ndx=0, bus_addr=(unsigned short *)(bussmem | word_ndx);
            word_ndx < ram_size; word_ndx+=2)
        {
            if ( ( temp_data = *bus_addr ) == msla_detect[word_test] )
            {
                *bus_addr = NULL_DATA;
                bus_addr++ ;
                continue;
            }
            else
            {
                rc = MSLARDWR_WORD_FAIL;           /* byte test fails */
                *bad_address = (short )word_ndx ;    /*Returning ofset*/
                loop = FAIL;
                break;
            }
        }
    }

    post_load_setup(fd);
    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  msla32bxfer                                           */
/*                                                                           */
/*       MODULE No  :  1.01.01                                               */
/*                                                                           */
/*         POURPOSE :  Verify that the R2 can read and write the MSLA ram    */
/*                     using 32 bit operations.                              */
/*                                                                           */
/*            INPUT :  fd, busmem, ioaddr, ram_size, bad_address             */
/*                                                                           */
/*           OUTPUT :  							     */
/*                    A1. BAD ADDRESS                                        */ 
/*                          OR XXXX ADDRESS OF FAILING MEMORY!               */ 
/*                    A2. RETURN CODE                                        */ 
/*                          0 => ALL RESULTS GOOD                            */ 
/*                         !0 => ROUTINE FAILS  TEST                         */ 
/*                                                                           */
/* FUNCTIONS CALLED : None.						     */
/*                                                                           */
/*  DATE of Modification : 09/11/1989                                        */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

int
msla32bxfer(fd,busmem,ioaddr,ram_size,bad_address)
int fd;                           /* File descriptor of BUS       */
unsigned long busmem;             /* Starting address of memory   */
unsigned long ioaddr;             /* IO base address of adapter   */
unsigned long ram_size;           /* amont of RAM to be tested    */
unsigned short *bad_address;      /* Return the failing address   */
/* Returns the bad address as Offset from The MSLA memory @0xF4E80000 */
{
    char *malloc();
    int rc ;
    char *charmem;
    unsigned long *busaddr;
    unsigned int i;


    rc = SUCCESS;
    pre_load_setup(fd,ioaddr);

    /* Malloc space if this is the first time into this routine */
     if (srcmem==0)
       {
	 #ifdef PRTF
	   printf("mslamemtst.c: about to malloc \n");
	 #endif
	 if (( charmem = malloc( (unsigned) ram_size )) == (char *)NULL )
	 {
	     rc = SYS_MALLOC_ERR;
	     return(rc);
	 }

	 srcmem = ( unsigned long *) charmem;
	 srcmemp = ( unsigned long *) charmem;
	 charmem = (char *) NULL;
	 if (( charmem = malloc( (unsigned) ram_size )) == (char *)NULL )
	 {
	     rc = SYS_MALLOC_ERR;
	     free( (char *)srcmemp );
	     return(rc);
	 }

	 sinkmem = ( unsigned long *) charmem;
	 sinkmemp = ( unsigned long *) charmem;
       }          /*end malloc */


    rc = fillsrc(srcmem,busmem,(unsigned int)ram_size);
    if ( rc != SUCCESS )
    {
	/* The whole purpose of checking 32 bit xfer is lost
	** if its not on 4byte boundary                      */
	return(rc);
    }

   /* Write to adapter */
    busaddr = (unsigned long *) busmem;
    for ( i = 0; i < ram_size; i+=4)
    {
        *busaddr = *srcmem ;
        busaddr++ ;
        srcmem++ ;
    }

   /* Read  to adapter */
    busaddr = (unsigned long *) busmem;
    for ( i = 0; i < ram_size; i+=4)
    {
        *sinkmem = *busaddr ;
        busaddr++ ;
        sinkmem++ ;
    }

   /* Compare what was read */
    sinkmem-- ;
    srcmem-- ;
    for ( i = 0; i < ram_size/4; i++, srcmem--, sinkmem--)
    {
        if ( *sinkmem != *srcmem )
        {
            rc = MEM32B_TEST_ERR;
            *bad_address = (unsigned short )(*srcmem) ;
            break;
        }
    }
    sinkmem++ ;
    srcmem++ ;
    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME : fillsrc                                                */
/*                                                                           */
/*         POURPOSE : Fills the low and high word.			     */
/*                                                                           */
/*            INPUT :  srcmem,bussmem,ram_size				     */
/*                                                                           */
/*           OUTPUT :  Returns SUCCESS>					     */
/*                                                                           */
/* FUNCTIONS CALLED :  None.						     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

int 
fillsrc(srcmem,bussmem,ram_size)
unsigned long *srcmem;
unsigned long bussmem;
unsigned int ram_size;
{
      /*
      ** Local variable definitions
      */
    union address {
          unsigned long busmem;
          struct memaddress {
                 unsigned short himem;
                 unsigned short lomem;
          } memadr;
    } adr;

    unsigned int i;
    unsigned long tmpdata;
    int  rc;
        

    adr.busmem = bussmem;            /* copy the memory start addr */
    if (  adr.memadr.lomem % 4 != 0 )
    {
        rc = NOT_LONGWORD_BOUNDARY;
	return(rc);
    }


    /* 
    **********************************************************
    *  Each 32bit long-word is filled with an unique value   *
    *  i.e the address of the 32bit word itself since, the   *
    *  address is 16bit the upper and the lower 16bit word   *
    *  of the longword has the same value.                   *
    **********************************************************
    */
    for ( i = 0; i < ram_size; i+=4)
    {
        *srcmem = adr.memadr.lomem + i;    /* fills the lo-word */
        tmpdata = *srcmem << 16 ;
        *srcmem |= tmpdata;               /* fills the hi-word */
        srcmem++;
    }
    rc = SUCCESS;

    return(rc);
}


/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  load_p                                                */
/*                                                                           */
/*         POURPOSE :  Load a pattern to busmem with pattern of 16-bit data  */
/*                     over a size of ram_len from offset load_addr.         */
/*                                                                           */
/*            INPUT :  bussmem,pattern,ram_len,load_addr,fd,slot,ioaddr      */
/*                                                                           */
/*           OUTPUT :  None.						     */
/*                                                                           */
/* FUNCTIONS CALLED : pre_load_setup; post_load_setup; msla_enable.	     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

void
load_p(bussmem,pattern,ram_len,load_addr,fd,ioaddr)
unsigned long bussmem;
unsigned short pattern;                         /* matrix of unsigned integer */
unsigned int ram_len;                           /* no. of bytes to be loaded  */
unsigned int load_addr;                         /* offset from MSLA memory    */
int fd;
unsigned long ioaddr;
{

    int loop;
    unsigned long address;
    unsigned short *bus_address ;


    pre_load_setup(fd,ioaddr);

    address = bussmem | load_addr;
    bus_address = ( unsigned short *) address;

    for ( loop = 0; loop < ram_len; loop++)
    {
        *bus_address = pattern;
        bus_address++ ;
    }

    post_load_setup(fd);

}
