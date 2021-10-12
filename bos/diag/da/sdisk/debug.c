static char sccsid[] = "@(#)52	1.1  src/bos/diag/da/sdisk/debug.c, dasdisk, bos411, 9428A410j 8/10/93 10:07:23";
/*
 *   COMPONENT_NAME: DASDISK
 *
 *   FUNCTIONS: db
 *		p_data
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
#include	<stdio.h>
#include	<sys/stat.h>
#include	<stdarg.h>
#include	"dh2.h"
#include	<diag/har2_atu.h>

/* Debug stuff */

/*
 * NAME: db()
 *
 * FUNCTION: Write debug data to a file.
 *
 * NOTES:
 *      - The calls to db() are for debug. If the debug file exists,
 *        debug information will be written to it. If the debug file
 *        does not exist, db() will simply return. To turn on debug,
 *        touch /tmp/.DIAG_HAR2_DBUG and that will be the debug file.
 *        To append to the debug file each time the DA/SA is run,
 *        export DIAG_HAR2_DBUG=APPEND before running diagnostics.
 *
 *      - The function has variable arguments and will call va_start()
 *        and va_end() even if the debug file does not exist.
 *
 * RETURNS: void
 *
 */

void db(int dbug_num, ...)
{
        int           i, i1, i2, rc;
        char          *s1, *s2;
        char          fname[256];
        HAR2_TUTYPE   *db_tucb;
        FILE          *dbfptr;
        struct stat   file_status;
        va_list       ag;

        va_start(ag, dbug_num);
        strcpy(fname, "/tmp/.DIAG_HAR2_DBUG");

        if ((rc = stat(fname,&file_status)) == -1) {
                va_end(ag);
                return; /* Debug file not present. */
        }

        if (!strcmp((char *)getenv("DIAG_HAR2_DBUG"),"APPEND")) {
                dbfptr = fopen(fname,"a+");
        } else {
                if (dbug_num == 1) {
                        dbfptr = fopen(fname,"w+");
                } else {
                        dbfptr = fopen(fname,"a+");
                }
        }

        switch(dbug_num) {
        case 1: /* init dbug file */
                fprintf(dbfptr,"============= start ============\n");
                break;
        case 2: /* print db_tucb info before call to exectu(). */
                db_tucb = va_arg(ag, HAR2_TUTYPE *);
                fprintf(dbfptr,"> HAR2 CDB (Data Out)\n\t");
                p_data(dbfptr, db_tucb->scsitu.scsi_cmd_blk,
                               db_tucb->scsitu.command_length);
                if (db_tucb->scsitu.data_length) {
                        fprintf(dbfptr,"> Param List (Data Out)\n\t");
                        p_data(dbfptr, db_tucb->scsitu.data_buffer, 64);
                }
                break;
        case 3: /* print Sense Data */
                db_tucb = va_arg(ag, HAR2_TUTYPE *);
                fprintf(dbfptr,
                        "> Sense Key %02.2X, ASC/ASCQ = %04.4X\n",
                        (db_tucb->scsiret.sense_key & 0x0F),
                        db_tucb->scsiret.sense_code);
                fprintf(dbfptr,"> Sense Data\n\t");
                p_data(dbfptr, db_tucb->scsitu.data_buffer, 128);
                break;
        case 10: /* Print multiple integers (int_name = int_value). */
        case 16:
                i1 = va_arg(ag, int); /* Number of int name/value pairs. */
                for (i = 0; i < i1; i++) {
                        s1 = va_arg(ag, char *);
                        i2 = va_arg(ag, int);
                        if (dbug_num == 16) /* Hex */
                                fprintf(dbfptr, "> %s = %X\n", s1, i2);
                        else /* Decimal */
                                fprintf(dbfptr, "> %s = %d\n", s1, i2);
                }
                break;
        case 20: /* print  multiple strings (ptr_name = ptr). */
                i1 = va_arg(ag, int); /* Number of ptr name/value pairs. */
                for (i = 0; i < i1; i++) {
                        s1 = va_arg(ag, char *);
                        s2 = va_arg(ag, char *);
                        fprintf(dbfptr, "> %s = %s\n", s1, s2);
                }
                break;
        case 25:
                /* Print a simple string. */
                s1 = va_arg(ag, char *);
                fprintf(dbfptr, "%s\n",s1);
                break;
        case 30: /* Display buffer data. */
                i1 = va_arg(ag, int);
                s2 = va_arg(ag, char *);
                fprintf(dbfptr, "> Data Buffer\n\t");
                p_data(dbfptr, s2, i1);
                break;
        case 999:
                fprintf(dbfptr,"======== end ========\n");
                break;
        default:
                break;
        }
	fflush(dbfptr);
        if (dbfptr != NULL)
                fclose(dbfptr);
        va_end(ag);

        return;

} /* end db */

/*
 * NAME: p_data()
 *
 * FUNCTION: Print hex bytes from a data buffer.
 *
 * NOTES:
 *
 * RETURNS: void
 *
 */

p_data(FILE *dbfptr, char *ptr, int len) {
        int count = 0;

        /* Print data buffer, 16 bytes per line. */
        while ((len--) > 0) {
                if (count == 8) {
                        fprintf(dbfptr,"    ");
                } else if (count == 16) {
                        fprintf(dbfptr,"\n\t");
                        count = 0;
                }
                fprintf(dbfptr,"%02.2X ", *(ptr++));
                ++count;
        }
        fprintf(dbfptr,"\n");

        return;
} /* end p_data */

