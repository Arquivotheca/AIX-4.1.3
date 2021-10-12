static char sccsid[] = "@(#)44  1.3  src/bos/diag/tu/fddi/rw_allmem.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:09";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: rw_allmem
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

Function(s) rw_allmem - Write/Read to all memory

Module Name :  rw_allmem.c
SCCS ID     :  1.20

Current Date:  5/23/90, 11:17:55
Newest Delta:  3/26/90, 16:25:54


*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <errno.h>
#include "diagddfddiuser.h"
#include "fdditst.h"


#ifdef debugg
extern void detrace();
#endif
extern int wr_datastore();
extern int rd_datastore();


/*****************************************************************************

rw_allmem

*****************************************************************************/

int rw_allmem(fdes, wopcode, ropcode, offset, transfers, tucb_ptr)
   int fdes;
   int wopcode;
   int ropcode;
   int offset;
   int transfers;
   TUTYPE *tucb_ptr;
   {
        int             rc, i, j, k, l;
        int             num_patterns;
        int             sizebuf1, sizebuf2, sizebuf3;
        unsigned long   status;
        unsigned char   buf1[BUFF64K_SIZE], buf2[BUFF64K_SIZE];
        unsigned char   buf3[BUFF32K_SIZE], buf4[BUFF32K_SIZE];
        unsigned char   buf5[BUFF32K_SIZE], buf6[BUFF32K_SIZE];
        unsigned char   buf7[BUFF16K_SIZE], buf8[BUFF16K_SIZE];
        unsigned char   buf9[BUFF16K_SIZE], buf10[BUFF16K_SIZE];
        unsigned char   *ptr1, *ptr2, *ptr3;
        static unsigned char pattern[] =
           {
                0x00, 0x01, 0x55, 0xAA, 0xFF
           };
        fddi_mem_acc_t  ram;
        struct htx_data *htx_sp;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

        /*
         * Create patterns and place them in appropiate buffers.
         */
        num_patterns = sizeof(pattern);
        sizebuf1 = sizebuf2 = sizebuf3 = 0;
        for (i = 0; i <= num_patterns; i++)
         {
          if (wopcode == FDDI_WR_MEM_FDDI_RAM)
            {
               /*
                * Setup for read and write of FDDI RAM by writing information
                * to the control registers for the RBC which are mapped to
                * Data Store memory locations 0700 thru 070C. This is required
                * before reading and writing to the FDDI RAM Buffer.
                */

               rc = wr_datastore(fdes, tucb_ptr);
               if (rc)
                   return(rc);
            }

           switch (transfers)
            {
             case 1:       /* Case for One Buffer Transfer  */
              if (i < num_patterns)
                        /* Patterns in character array pattern[]. */
                for (j = 0; j < BUFF64K_SIZE - offset; j++)
                            buf1[j] = pattern[i];
              else      /* Incrementing pattern */
                for (j = 0; j < BUFF64K_SIZE - offset; j++)
                            buf1[j] = j;
              ram.buffer_1 = buf1;
              ram.buff_len1 = BUFF64K_SIZE - offset;
              sizebuf1 = BUFF64K_SIZE;
              ptr1 = buf2;
              break;

             case 2:     /* Case for Two Buffer Transfer   */
              if (i < num_patterns)
                        /* Patterns in character array pattern[]. */
               {
                for (j = 0; j < BUFF32K_SIZE - offset; j++)
                            buf3[j] = pattern[i];
                for (j = 0; j < BUFF32K_SIZE; j++)
                            buf4[j] = pattern[i];
               }
              else      /* Incrementing pattern */
               {
                for (j = 0; j < BUFF32K_SIZE - offset; j++)
                            buf3[j] = j;
                j = BUFF32K_SIZE - offset;
                for (l = 0; l < BUFF32K_SIZE; l++)
                            buf4[l] = j + l;
               }
              ram.buffer_1 = buf3;
              ram.buff_len1 = BUFF32K_SIZE - offset;
              ram.buffer_2 = buf4;
              ram.buff_len2 = BUFF32K_SIZE;
              sizebuf1 = BUFF32K_SIZE;
              ptr1 = buf5;
              sizebuf2 = BUFF32K_SIZE;
              ptr2 = buf6;
              break;

             case 3:    /* Case for Three Buffer Transfer   */
              if (i < num_patterns)
                        /* Patterns in character array pattern[]. */
               {
                for (j = 0; j < BUFF16K_SIZE - offset; j++)
                            buf7[j] = pattern[i];
                for (j = 0; j < BUFF32K_SIZE; j++)
                            buf3[j] = pattern[i];
                for (j = 0; j < BUFF16K_SIZE; j++)
                            buf8[j] = pattern[i];
               }
              else      /* Incrementing pattern */
               {
                for (j = 0; j < BUFF16K_SIZE - offset; j++)
                            buf7[j] = j;
                j = BUFF16K_SIZE - offset;
                for (l = 0; l < BUFF32K_SIZE; l++)
                            buf3[l] = j + l;
                j += BUFF32K_SIZE;
                for (l = 0; l < BUFF16K_SIZE; l++)
                            buf8[l] = j + l;
               }
              ram.buffer_1 = buf7;
              ram.buff_len1 = BUFF16K_SIZE - offset;
              ram.buffer_2 = buf3;
              ram.buff_len2 = BUFF32K_SIZE;
              ram.buffer_3 = buf8;
              ram.buff_len3 = BUFF16K_SIZE;
              sizebuf1 = BUFF16K_SIZE;
              ptr1 = buf9;
              sizebuf2 = BUFF32K_SIZE;
              ptr2 = buf5;
              sizebuf3 = BUFF16K_SIZE;
              ptr3 = buf10;
              break;
             }
              /*
               * Set up ioctl for write, then write buffer(s) with test pattern
               * to appropiate area.
               */

              ram.opcode = wopcode;
              ram.ram_offset = offset;
              ram.num_transfer = transfers;

              status = 0;
              rc = ioctl(fdes, FDDI_MEM_ACC, &ram);
              status = ram.status;

              if (rc)
                 {
#ifdef debugg
                   detrace(0,"Memory Transfer written ");
                   detrace(0,"incorrectly.    rc = %x\n", rc);
#endif
                   if (htx_sp != NULL)
                        (htx_sp->bad_others)++;
                   return(WRITE_ERROR);
                 }
              else
                 if (htx_sp != NULL)
                    (htx_sp->good_others)++;
#ifdef debugg
              detrace(0,"Write buffer with number of Transfers = %x.\n",
                                        transfers);
              if (i < num_patterns)
                detrace(0, "Pattern = %02x \n", pattern[i]);
              else
                detrace(0, "Incrementing Pattern. \n");
              switch (transfers)
               {
                case 1:
                   for (k = 0; k < SHOW_BYTES; k++)
                        detrace(0, "%02x ", buf1[k]);
                   break;
                case 2:
                   for (k = 0; k < SHOW_BYTES; k++)
                        detrace(0, "%02x ", buf3[k]);
                   break;
                case 3:
                   for (k = 0; k < SHOW_BYTES; k++)
                        detrace(0, "%02x ", buf7[k]);
                   break;
                default:
                   break;
               }
              detrace(0, "\n");
#endif
              /*
               * Read Memory Transfer.
               */
              ram.opcode = ropcode;
              ram.ram_offset = offset;
              ram.num_transfer = transfers;
              ram.buffer_1 = ptr1;
              ram.buff_len1 = sizebuf1 - offset;
              ram.buffer_2 = ptr2;
              ram.buff_len2 = sizebuf2;
              ram.buffer_3 = ptr3;
              ram.buff_len3 = sizebuf3;

              status = 0;
              rc = ioctl(fdes, FDDI_MEM_ACC, &ram);
              status = ram.status;

              if (rc)
                {
#ifdef debugg
                   detrace(0,"Memory Transfer read ");
                   detrace(0,"incorrectly. rc = %x\n", rc);
#endif
                   if (htx_sp != NULL)
                        (htx_sp->bad_others)++;
                   return(READ_ERROR);
                }
              else
                 if (htx_sp != NULL)
                    (htx_sp->good_others)++;
#ifdef debugg
              detrace(0,"Read buffer with number of Transfers = %x.\n",
                                transfers);
              if (i < num_patterns)
                detrace(0, "Pattern = %02x \n", pattern[i]);
              else
                detrace(0, "Incrementing Pattern. \n");
              switch (transfers)
               {
                case 1:
                   for (k = 0; k < SHOW_BYTES; k++)
                        detrace(0, "%02x ", buf2[k]);
                   break;
                case 2:
                   for (k = 0; k < SHOW_BYTES; k++)
                        detrace(0, "%02x ", buf5[k]);
                   break;
                case 3:
                   for (k = 0; k < SHOW_BYTES; k++)
                        detrace(0, "%02x ", buf9[k]);
                   break;
                default:
                   break;
               }
              detrace(0, "\n");
#endif
              /*
               * Compare data in buf1 and buf2.
               * If errors, report them and stop test.
               */

                switch (transfers)
                 {
                   case 1:      /* One Buffer Transfer  */
                        if (memcmp(buf1, buf2, BUFF64K_SIZE - offset) != 0)
                          {
                            if (htx_sp != NULL)
                              (htx_sp->bad_others)++;
                            return(COMPARE12_ERROR);
                          }
                        break;

                   case 2:      /* Two Buffer Transfers */
                        if (memcmp(buf3, buf5, BUFF32K_SIZE - offset) != 0)
                          {
                            if (htx_sp != NULL)
                              (htx_sp->bad_others)++;
                            return(COMPARE35_ERROR);
                          }

                        if (memcmp(buf4, buf6, BUFF32K_SIZE) != 0)
                          {
                            if (htx_sp != NULL)
                              (htx_sp->bad_others)++;
                            return(COMPARE46_ERROR);
                          }
                        break;

                   case 3:      /* Three Buffer Transfers */
                        if (memcmp(buf7, buf9, BUFF16K_SIZE - offset) != 0)
                          {
                            if (htx_sp != NULL)
                              (htx_sp->bad_others)++;
                            return(COMPARE79_ERROR);
                          }

                        if (memcmp(buf3, buf5, BUFF32K_SIZE) != 0)
                          {
                            if (htx_sp != NULL)
                              (htx_sp->bad_others)++;
                            return(COMPARE35_ERROR);
                          }

                        if (memcmp(buf8, buf10, BUFF16K_SIZE) != 0)
                          {
                            if (htx_sp != NULL)
                              (htx_sp->bad_others)++;
                            return(COMPARE80_ERROR);
                          }
                        break;
                 }

            if (wopcode == FDDI_WR_MEM_FDDI_RAM)
              {
                /*
                 * If testing the FDDI RAM buffer, read the RBC status
                 * registers to insure no errors occured while reading
                 * and writing to the FDDI RAM Buffer.
                 */

                rc = rd_datastore(fdes, tucb_ptr);
                if (rc)
                    return(rc);
              }

#ifdef debugg
            if (i < num_patterns)
              detrace(0,"Compared SUCCESSFULLY with pattern = %x\n",
                                   pattern[i]);
            else
              detrace(0,"Compared SUCCESSFULLY with incrementing pattern.\n");
#endif
           }
        return(0);
   }
