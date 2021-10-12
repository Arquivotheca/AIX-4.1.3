static char sccsid[] = "@(#)72  1.2.2.1  src/bos/diag/tu/fddi/tu021.c, tu_fddi, bos41J, 9512A_all 3/2/95 10:33:18";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: diag_load
 *              tu021
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

Function(s) Test Unit 021 - Download Diagnostic Microcode

Module Name :  tu021.c
SCCS ID     :  1.2

Current Date:  5/23/90, 11:17:56
Newest Delta:  1/19/90, 16:31:35

This test unit will download diagnostic adapter microcode.

*****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <cf.h>
#include <sys/comio.h>
#include <sys/devinfo.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/intr.h>
#include <sys/stat.h>
#include "diagddfddiuser.h"
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif

extern int rw_bus_prg();


/*****************************************************************************

diag_load

*****************************************************************************/

int diag_load(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {

        fddi_dwnld_t    cmd;
        int             mcode_len,      /* length of microcode file */
                        mcode_fd,       /* microcode file descriptor */
                        flag,
                        rc;
        char            *mcode,         /* pointer to microcode image */
                        mbasename[] = "fddi.diag",
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
        detrace(0,"-----  Begin Downloading Diagnostic Microcode  ------\n");
#endif

        /*
         * Get microcode filename
         */

        flag = ABSOLUTE;
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
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, STAT_DIAG_ERR));
        }
        mcode_len = statbuf.st_size;

        /*
         * Open microcode file and determine Begining of File (BOF) and
         * End of FIle (EOF).
         */

        if( (mcode_fd=open(mpath,O_RDONLY)) == -1)
          {
#ifdef debugg
            detrace(0,"download_microcode(): open on code file failed\n");
#endif
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, OPEN_DIAG_ERR));
          }

        /*
         * Set up memory allocation for microcode.
         */
        if( (mcode=malloc(mcode_len)) == NULL)
          {
#ifdef debugg
            detrace(0,"download_microcode(): malloc for microcode failed\n");
#endif
            close(mcode_fd);
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, MALLOC_ERR));
          }

#ifdef debugg
        detrace(0,"download_microcode(): Memory allocation complete.\n" );
#endif

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
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, RD_DIAG_ERR));
          }

#ifdef debugg
        detrace(0,"download_microcode(): MCode loader and MCode loaded\n");
#endif

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
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, DOWN_DIAG_ERR));
          }
        else
            if (htx_sp != NULL)
               (htx_sp->good_others)++;
#ifdef debugg
        detrace(0,"download_microcode(): successfully and correctly loaded\n");
#endif

#ifdef debugg
        detrace(0, "----  Download of Diagnostic Microcode completed  -----\n");
#endif
        free(mcode);
        return(0);
   }


/*****************************************************************************

tu021

*****************************************************************************/

int tu021 (fdes, tucb_ptr)
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

        rc = diag_load(fdes, tucb_ptr);

        /*
         * Read the NP Bus Program Store area to see if Microcode
         * downloaded correctly.
         */
#ifdef debugg
        result = rw_bus_prg(fdes, tucb_ptr);
        if (result)
           {
                detrace(0,"Read of NP Bus Program Store failed.");
                detrace(0,"Return = %x\n", result);
                return(result);
           }
#endif
        return(rc);
   }
