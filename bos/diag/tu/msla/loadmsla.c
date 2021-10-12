static char sccsid[] = "@(#)26  1.3.1.2  src/bos/diag/tu/msla/loadmsla.c, tu_msla, bos41J, 9517A_all 4/25/95 08:44:36";
/*
 * COMPONENT_NAME: ( loadmsla )
 *
 * FUNCTIONS :  bload, get_bus_des, get_filedes, preload_setup,
 *              post_load_setup, disable_parity, enable_parity,
 *	        disable_adapter, no_byte_swap, byte_swap, copy2bus
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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "mslafdef.h"
#include "mslaerrdef.h"
#include "msladgappl.h"
#include "msla_diag.h"
#include "mslablof.h"

unsigned int buf_size = BUFSIZ ;     /* as defined in stdio.h */
/* global pointer so that we malloc only once */
static char   *tmpbuf = 0 ;
static char   *read_bufc = 0 ;

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  bload                                                 */
/*                                                                           */
/*          PURPOSE :  bload(filename,address,fd,slot,ofset,ioaddr)          */
/*                     This function opens a file in binary mode and loads   */
/*                     the data i.e the micro-code in the adapter            */
/*                                                                           */
/*            INPUT :                                                        */
/*                    1.  binary file name                                   */
/*                    2.  load address                                       */
/*                    3.  file-descriptor of bus-open                        */
/*                    4.  ofset form start of bin-file i.e to be loaded      */
/*                        This routine can't load a binary file if ofset is  */
/*                             an odd integer and filesize in bytes is more  */
/*                             than BUFSIZ as defined in stdio.h             */
/*                        Large u-codes with ofset 0 can be loaded properly  */
/*                    5.  i/o  address                                       */
/*                                                                           */
/*    NORMAL RETURN :                                                        */
/*                                                                           */
/*    ERROR RETURNS :                                                        */
/*                                                                           */
/*****************************************************************************/

int
bload(filename,address,fd,ofset,ioaddr)
char *filename;
unsigned long address;
int fd;
int ofset;
unsigned long ioaddr;
{
    unsigned int copy2bus();
    int  pre_load_setup();
    int  post_load_setup();
    void  close_bus();
/*    char *getenv();
    char *pathstr;        */
    char pathname[100];

    char *bus_address;
    int rc, fres ;
    int bytes_read  ;

    rc = SUCCESS ;
 /*
    pathstr = getenv("DIAGDATADIR");
    if ( pathstr == NULL ) {
        rc = UCODE_FILE_NOT_FOUND ;
	return(rc);
    }
    strcpy(pathname,pathstr);
   */
    strcpy(pathname,DiagUcodeDir);
    strcat(pathname,filename);
    fres = get_filedes(pathname, O_RDONLY  );
    if (fres < 0 )
    {
        rc = UCODE_FILE_NOT_FOUND ;
    }
    else
    {
        bus_address = ( char *) address;

        /* Set the POS registers and the io-ports as per MSLA specs */
        pre_load_setup(fd,ioaddr);

        bytes_read = copy2bus(fres,address,ofset);
        post_load_setup(fd);
        close(fres);
        if  ( bytes_read  > 0 )
        {
            rc = SUCCESS;
        }
        if  ( bytes_read  < 0 )
        {
            /*  Input FILE closed due to error in reading */
            rc = FILE2MEM_XFER_ERR ;
        }
    }
    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  get_filedes()                                         */
/*                                                                           */
/*         PURPOSE  :                                                        */
/*                                                                           */
/*           INPUT  :                                                        */
/*                                                                           */
/*          OUTPUT  :                                                        */
/*                                                                           */
/*   NORMAL RETURN  :                                                        */
/*                                                                           */
/*   ERROR RETURNS  :                                                        */
/*                                                                           */
/* EXTERNAL REFERENCES = None                                                */
/*                                                                           */
/*****************************************************************************/
int 
get_filedes(filename,mode)
char *filename;
int mode;
{
    char fil[71];
    int fdes ;

    strcpy( fil, filename);
    if ( mode == O_RDONLY  )
    {
        fdes = open ( fil , O_RDONLY | O_NDELAY );
    }
    else
    {
        fdes = open ( fil , O_RDWR | O_NDELAY | O_CREAT );
    }
   
    return(fdes);

}           

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :   pre_load_setup                                       */
/*                                                                           */
/*         POURPOSE :                                                        */
/*                                                                           */
/*            INPUT :   fd, ioaddr.               		     	     */
/*                                                                           */
/*           OUTPUT :  None.						     */
/*                                                                           */
/* FUNCTIONS CALLED : reset_msla, halt_msla, enable_adapter, byte_swap.	     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

int
pre_load_setup(fd,ioaddr)
int fd;
unsigned long ioaddr;
{
    extern void reset_msla();
    extern void halt_msla();
    void byte_swap();
    int  rc;

   /* Reset MSLA */
    reset_msla(fd,ioaddr);

   /* Disable parity */
    rc = ioctl (fd, MSLA_MOD_POS, MP_DIS_PARITY);
    if (rc < 0)
    {
	printf("Preload : Disable Parity Failed\n");
	return(rc);
    }

   /* Insert a delay */
    delay(100);

   /* Halt the MSLA */
    halt_msla(fd,ioaddr);

   /* Enable the MSLA */
    rc = ioctl (fd, MSLA_MOD_POS, MP_ENA_CARD);
    if (rc < 0)
    {
	printf("Preload : Enable Card Failed\n");
	return(rc);
    }

}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  post_load_setup                                       */
/*                                                                           */
/*         POURPOSE :                                                        */
/*                                                                           */
/*            INPUT :  fd                               		     */
/*                                                                           */
/*           OUTPUT :   None.						     */
/*                                                                           */
/* FUNCTIONS CALLED : enable_parity_chk, enable_parity_chk.		     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

int
post_load_setup(fd)
int fd;
{
    int  rc;

   /* Enable parity check */
   /* enable_parity_chk(fd,slot);*/
    rc = ioctl (fd, MSLA_MOD_POS, MP_ENA_PARITY);
    if (rc < 0)
    {
	printf("Postload : Enable Parity Failed\n");
	return(rc);
    }

}

/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     copy2bus()                                            */
/*                                                                           */
/* DESCRIPTIVE NAME =                                                        */
/*                                                                           */
/* FUNCTION         =                                                        */
/*                                                                           */
/* INPUT            =                                                        */
/*                                                                           */
/* OUTPUT           =                                                        */
/*                                                                           */
/* NORMAL RETURN    =                                                        */
/*                                                                           */
/* ERROR RETURNS    =                                                        */
/*                                                                           */
/* EXTERNAL REFERENCES = None                                                */
/*                                                                           */
/*****************************************************************************/
unsigned int 
copy2bus(fres,bus_addr,ofset)
int fres;
ulong bus_addr;
int ofset;
{
    char *malloc();
    /* char *sbrk(); */
    char  *tmp_buf,  *read_buftmp;
    unsigned short *read_buf , *bus_address ;
    signed int n, i, bytes_read;
    unsigned int rc;
    int first_time ;

    rc = FAIL ;
    n = BUFSIZ  ;
    first_time = 1;
   
    tmp_buf = (char *)NULL ;
    read_buftmp = (char *)NULL ; 
    read_buf = (unsigned short *)NULL ;
    bus_address = (unsigned short *)NULL ;

    /* malloc if this is the first time into this routine */
    if (tmpbuf == 0)
       {
	 #ifdef PRTF
	     printf("loadmsla.c: malloc \n");
	 #endif
	 tmpbuf = malloc(n);
	 if ( tmpbuf == NULL )
		 return (rc) ;
	 read_bufc = malloc(n);
	 if ( read_bufc == NULL )
	 {
		 return (rc) ;
	 }
       }      /* end malloc */
         
    /* 
    **  The whole idea of having two sets of temp buffer i.e
    **  one with a char pointer and other with short integer pointer
    **  is to avoid data xfer error to MSLA in character mode 
    **  as there is h/w bug for byte xfer in swap on/off bit
    */ 

    read_buftmp = read_bufc;

    bus_address = ( unsigned short *) bus_addr ;
    n = 0;
    bytes_read = read(fres, tmpbuf, buf_size );
    while ( bytes_read  > 0 )
    {
        tmp_buf = tmpbuf ;
        if ( first_time == 1 )
        {
            /* 
            * Takes care of stripping the top <ofset> no. 
            * of bytes of the u-code file
            */
            tmp_buf +=ofset ;
            bytes_read -= ofset ;
            first_time = 0;
        }
        read_buftmp = read_bufc;
        for ( i = 0; i < bytes_read ; i++)
        {
            *read_buftmp = *tmp_buf ; /* byte xfer */
            read_buftmp++;
            tmp_buf++ ;
        }
        read_buf = (unsigned short *)read_bufc;
        for ( i = 0; i < bytes_read ; i+=2)
        {
            *bus_address = *read_buf ;   /* word xfer */
            read_buf++;
            bus_address++ ;
        }
                  
        n +=  bytes_read ;
        bytes_read = read(fres, tmpbuf, buf_size );
    }

    if (n > 0 )
        rc = n ;
    else
        rc = FAIL ;

    return(rc);
}
