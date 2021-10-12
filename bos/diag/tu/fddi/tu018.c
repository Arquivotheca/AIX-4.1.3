static char sccsid[] = "@(#)69  1.2.2.1  src/bos/diag/tu/fddi/tu018.c, tu_fddi, bos41J, 9512A_all 3/2/95 10:32:52";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: download
 *              tu018
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************

Function(s) Test Unit 018 - Download Microcode

Module Name :  tu018.c
SCCS ID     :  1.2

Current Date:  5/23/90, 11:17:56
Newest Delta:  1/19/90, 16:31:35

This test unit will download the operational adapter microcode.

*****************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <sys/comio.h>
#include <sys/devinfo.h>
#include <sys/err_rec.h>
#include <sys/ciouser.h>
#include <sys/intr.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cf.h>
#include "diagddfddiuser.h"
#include "fdditst.h"


#ifdef debugg
extern void detrace();
#endif

extern int rw_bus_prg();
extern int findmcode();


/*****************************************************************************

download

*****************************************************************************/

int download(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {

        fddi_dwnld_t    cmd;
        int             mcode_len,      /* length of microcode file */
                        mcode_fd,       /* microcode file descriptor */
                        rc,
                        flag;
        int fd;
        char    *mcode,         /* pointer to microcode image */
                mbasename[] = "8ef4m",
                mpath[255],
                prompt[] = "fill_var";
        struct stat statbuf;
        struct htx_data *htx_sp;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

#ifdef debugg
        detrace(0,"-----  Begin Downloading Microcode  ------\n");
#endif


        flag = VERSIONING;
        if(( findmcode(mbasename,mpath,flag)) == FALSE)
        {
#ifdef debugg
                detrace(0,"findmcode function call failed\n");
                perror("findmcode failed \n");
#endif
                return(mktu_rc(tucb_ptr->header.tu,LOG_ERR,FINDMCODE_ERR));
        }

        if ((stat(mpath,&statbuf)) == -1)
        {
#ifdef debugg
            detrace(0,"download_microcode(): stat function call failed\n");
#endif
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, STAT_MICRO_ERR));
        }
        mcode_len = statbuf.st_size;

        /*
         * Open microcode file
         */

        if( (mcode_fd=open(mpath,O_RDONLY)) == -1)
          {
#ifdef debugg
            detrace(0,"download_microcode(): open on code file failed\n");
#endif
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, OPEN_MICRO_ERR));
          }

        /*
         * Set up memory allocation for microcode
         */

        if ((mcode=malloc(mcode_len)) == -1)
          {
#ifdef debugg
            detrace(0,"download_microcode(): malloc on code file failed\n");
            perror("malloc call failed\n");
#endif
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MALLOC_ERR));
          }


        /*
         * Read microcode file.
         */
        if(read(mcode_fd, mcode, mcode_len)== -1)
          {
#ifdef debugg
            detrace(0,"download_microcode(): read microcode file failed\n");
#endif
            free(mcode);
            close(mcode_fd);
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RD_MICRO_ERR));
          }

        /*
         * Close microcode file and download microcode to adapter.
         */
        close(mcode_fd);
        cmd.p_mcode=mcode;      /* addr of microcode */
        cmd.l_mcode=mcode_len;  /* microcode length */

        if(ioctl(fdes, FDDI_DWNLD_MCODE, &cmd) != 0)
          {
            free(mcode);
            if (htx_sp != NULL)
               (htx_sp->bad_others)++;
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, DOWN_MICRO_ERR));
          }
        else
            if (htx_sp != NULL)
               (htx_sp->good_others)++;

#ifdef debugg
        detrace(0,"download_microcode(): successfully and correctly loaded\n");
#endif

#ifdef debugg
        detrace(0, "-----   Download Microcode completed   -------\n");
#endif
        free(mcode);
        return(0);
   }


/*****************************************************************************

tu018

*****************************************************************************/

int tu018 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc, result;
        struct htx_data *htx_sp;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

        /*
         * Download the Microcode.
         */

        rc = download(fdes, tucb_ptr);

        /*
         * Read the NP Bus Profram Store area to see if Microcode
         * downloaded correctly.
         */

#ifdef debugg
        result = rw_bus_prg(fdes, tucb_ptr);
#endif

        return(rc);
   }
