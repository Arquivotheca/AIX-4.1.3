static char sccsid[] = "@(#)43  1.2  src/bos/diag/tu/sun/tposreg.c, tu_sunrise, bos411, 9437A411a 3/30/94 16:27:58";
/*
 *   COMPONENT_NAME: tu_sunrise
 *
 *   FUNCTIONS: pos_chk
 *              pos_restore
 *              pos_save
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************

Function(s) - POS Register Test

Module Name :  tposreg.c

Test writes/reads/compares byte values to check all bit positions
in writable POS registers (2-7).

*****************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/diagex.h>
#include "suntu.h"
#include "error.h"
#include "sun_tu_type.h"

extern diag_struc_t  *handle;                   /* handle for Diagex       */


/*****************************************************************************

pos_chk

Disables the adapter and writes/reads/compares all bit positions to
POS registers 2 through 7.

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to read/write/compare problem with a POS

*****************************************************************************/

int pos_chk ()
{
        unsigned char posvalue;
        unsigned char pos_save_area[8];
        int rc;
        unsigned char i, test_val;
        static unsigned char tval[] = { 0xaa, 0x55, 0xff, 0x00 };

        /* save off all the POS reg original values before writing... */
        if (rc = pos_save(pos_save_area))
           {
                return(rc);
           }

        /* disable the card by writing 0 to POS 2. */
        if (rc = diag_pos_read(handle, 2, &posvalue, NULL, PROCLEV))
          {
                LOG_POS_ERROR(ERRORPOSRW, rc, 2, posvalue);
          }
        posvalue &= 0xfe;
        if (rc = diag_pos_write(handle, 2, posvalue, NULL, PROCLEV))
           {
                LOG_POS_ERROR(ERRORPOSRW, rc, 2, posvalue);
           }

        /* make sure POS0 and POS1 (non-writable) have legal values. */
        if (rc = diag_pos_read(handle, 0, &posvalue, NULL, PROCLEV))
           {
                LOG_POS_ERROR(ERRORPOSRW, rc, 0, posvalue);
           }
        if (posvalue != INTPOS0)
           {
                LOG_REGISTER_ERROR(ERRORPOS0, 0, INTPOS0, posvalue, 0);
           }
        if (rc = diag_pos_read(handle, 1, &posvalue, NULL, PROCLEV))
           {
                LOG_POS_ERROR(ERRORPOSRW, rc, 1, posvalue);
           }
        if (posvalue != INTPOS1)
           {
                LOG_REGISTER_ERROR(ERRORPOS1, 0, INTPOS1, posvalue, 1);
           }

        /* try to write 0x55, 0xAA, 0xFF, 0x00 in pos regs. 2 through 7. */

        for (i = 0x00; i < 0x04; i++)
           {
                /* mask out card enable to insure that card remains disabled. */
                test_val = tval[i] & 0xfe;
                if (rc = diag_pos_write(handle, 2, test_val, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 2, test_val);
                   }
                if (rc = diag_pos_read(handle, 2, &posvalue, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 2, posvalue);
                   }
                if ((posvalue & 0xfe) != (test_val & 0xfe))
                   {
                        pos_restore(pos_save_area);
                        LOG_REGISTER_ERROR(ERRORPOS2, 0, (test_val & 0xfe),
                                           (posvalue & 0xfe), 2);
                   }
           }
#ifdef debugg
        detrace(0,"Testing POS3\n");
#endif
   /* To access POS #3, write 0x0 to POS #6, 0x0 to POS #7, and value to POS #3 */
   if (rc = diag_pos_write(handle, 6, 0x0, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 6, 0);
                   }
   if (rc = diag_pos_write(handle, 7, 0x0, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 7, 0);
                   }
        for (i = 0x00; i < 0x04; i++)
           {
                test_val = tval[i] ;
                if (rc = diag_pos_write(handle, 3, test_val, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 3, test_val);
                   }
                if (rc = diag_pos_read(handle, 3, &posvalue, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 3, posvalue);
                   }
                if (posvalue != test_val)
                   {
                        pos_restore(pos_save_area);
                        LOG_REGISTER_ERROR(ERRORPOS3, 0, test_val, posvalue, 3);
                   }
           }
#ifdef debugg
        detrace(0,"Testing POS4\n");
#endif
        for (i = 0x00; i < 0x04; i++)
           {
                test_val = tval[i];
                if (rc = diag_pos_write(handle, 4, test_val, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 4, test_val);
                   }
                if (rc = diag_pos_read(handle, 4, &posvalue, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 4, posvalue);
                   }
                if (posvalue != test_val)
                   {
                        pos_restore(pos_save_area);
                        LOG_REGISTER_ERROR(ERRORPOS4, 0, test_val, posvalue, 4);
                   }
           }

#ifdef debugg
        detrace(0,"Testing POS5\n");
#endif
        for (i = 0x00; i < 0x04; i++)
           {
                test_val = tval[i];
                if (rc = diag_pos_write(handle, 5, test_val, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 5, test_val);
                   }
                if (rc = diag_pos_read(handle, 5, &posvalue, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 5, posvalue);
                   }
                if ((posvalue&0x38) != (test_val&0x38))
                   {
                        pos_restore(pos_save_area);
                        LOG_REGISTER_ERROR(ERRORPOS5, 0, (test_val&0x38), (posvalue&0x38), 5);
                   }
           }

#ifdef debugg
        detrace(0,"Testing POS6\n");
#endif
        for (i = 0x00; i < 0x04; i++)
           {
                test_val = tval[i];
                if (rc = diag_pos_write(handle, 6, test_val, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 6, test_val);
                   }
                if (rc = diag_pos_read(handle, 6, &posvalue, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 6, posvalue);
                   }
                if (posvalue != test_val)
                   {
                        pos_restore(pos_save_area);
                        LOG_REGISTER_ERROR(ERRORPOS6, 0, test_val, posvalue, 6);
                   }
           }

#ifdef debugg
        detrace(0,"Testing POS7\n");
#endif
        for (i = 0x00; i < 0x04; i++)
           {
                test_val = tval[i];
                if (rc = diag_pos_write(handle, 7, test_val, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 7, test_val);
                   }
                if (rc = diag_pos_read(handle, 7, &posvalue, NULL, PROCLEV))
                   {
                        pos_restore(pos_save_area);
                        LOG_POS_ERROR(ERRORPOSRW, rc, 7, posvalue);
                   }
                if (posvalue != test_val)
                   {
                        pos_restore(pos_save_area);
                        LOG_REGISTER_ERROR(ERRORPOS7, 0, test_val, posvalue, 7);
                   }
           }

        /* prior to returning with card enabled, restore original POS reg vals. */
        if (rc = pos_restore(pos_save_area))
           {
                return(rc);
           }

        /* NOW, insure that card is enabled back. */
        if (rc = diag_pos_read(handle, 2, &posvalue, NULL, PROCLEV))
          {
                LOG_POS_ERROR(ERRORPOSRW, rc, 2, posvalue);
          }
        posvalue = posvalue | 0x01;
        if (rc = diag_pos_write(handle, 2, posvalue, NULL, PROCLEV))
           {
                LOG_POS_ERROR(ERRORPOSRW, rc, 2, posvalue);
           }
        return(OK);
    }

/*****************************************************************************

pos_save

Function saves off POS registers 2 through 7 in the passed-in unsigned char
array, "save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to read of specific POS register

*****************************************************************************/

int pos_save (uchar save_area[])
   {
        int i, rc;
        unsigned char posvalue;

        for (i = 2; i < 8; i++)
          {
                if (i==3) {
   /* To access POS #3, write 0x0 to POS #6, 0x0 to POS #7, and value to POS #3 */
                   if (rc = diag_pos_write(handle, 6, 0x0, NULL, PROCLEV))
                     {
                        LOG_POS_ERROR(ERRORPOSSAVE, rc, 6, 0);
                     }
                   if (rc = diag_pos_write(handle, 7, 0x0, NULL, PROCLEV))
                     {
                        LOG_POS_ERROR(ERRORPOSSAVE, rc, 7, 0);
                     }
                } /* endif */
                if (rc = diag_pos_read(handle, i, &posvalue, NULL, PROCLEV))
                   {
                        LOG_POS_ERROR(ERRORPOSSAVE, rc, i, posvalue);
                   }
                save_area[i - 2] = posvalue;
           }
        return(0);
   }

/*****************************************************************************

pos_restore

Function restores POS registers 2 through 7 from the passed in unsigned char
array, "save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to write of specific POS register

*****************************************************************************/

int pos_restore (uchar save_area[])
   {
        int i, rc;

        for (i=3; i < 8; i++) {
                if (i==3) {
   /* To access POS #3, write 0x0 to POS #6, 0x0 to POS #7, and value to POS #3 */
                   if (rc = diag_pos_write(handle, 6, 0x0, NULL, PROCLEV))
                     {
                        LOG_POS_ERROR(ERRORPOSRESTORE, rc, 6, 0);
                     }
                   if (rc = diag_pos_write(handle, 7, 0x0, NULL, PROCLEV))
                     {
                        LOG_POS_ERROR(ERRORPOSRESTORE, rc, 7, 0);
                     }
                } /* endif */
          if (rc = diag_pos_write(handle, i, save_area[i-2], NULL, PROCLEV))
            {
                        LOG_POS_ERROR(ERRORPOSRESTORE, rc, i, save_area[i-2]);
            }
           }

          /* restore pos#2 last since it has the card enable bit */
          if (rc = diag_pos_write(handle, 2, save_area[0], NULL, PROCLEV))
           {
                        LOG_POS_ERROR(ERRORPOSRESTORE, rc, 2, save_area[0]);
           }
        return(0);
   }
