static char sccsid[] = "@(#)99	1.2  src/bos/diag/da/corvette/debug.c, dascsi, bos411, 9428A410j 8/18/93 10:51:16";
/*
 *   COMPONENT_NAME: DASCSI
 *
 *   FUNCTIONS: diag_db
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
#include	"Adapters.h"
#include	"corv_atu.h"
#include	"debug.h"

/* Debug stuff */

/*
 * NAME: diag_db()
 *
 * FUNCTION: Write debug data to a file.
 *
 * NOTES:
 *      - The calls to diag_db() are for debug. If the debug file exists,
 *        debug information will be written to it. If the debug file
 *        does not exist, diag_db() will simply return. To turn on debug,
 *        touch /tmp/.DIAG_CORV_DBUG and that will be the debug file.
 *        To append to the debug file each time the DA/SA is run,
 *        export DIAG_CORV_DBUG=APPEND before running diagnostics.
 *
 *      - The function has variable arguments and will call va_start()
 *        and va_end() even if the debug file does not exist.
 *
 * RETURNS: void
 *
 */

void diag_db(int db_key, ...)
{
	int	i, i1, i2, rc;
        char	*s1, *s2;
	char	fname[256];
	FILE	*dbfptr;
	CORV_TUTYPE	*tucb;
	struct stat	file_status;
	va_list	ag;

        va_start(ag, db_key);
        strcpy(fname, "/tmp/.DIAG_CORV_DBUG");

        if ((rc = stat(fname,&file_status)) == -1)
	{
                va_end(ag);
                return; /* Debug file not present. */
        }

        if (!strcmp((char *)getenv("DIAG_CORV_DBUG"),"APPEND") || db_key!=START) 
                dbfptr = fopen(fname,"a+");
        else
		dbfptr = fopen(fname,"w+");

        switch(db_key) {
        case START: /* init dbug file */
                fprintf(dbfptr,"============= start ============\n\n");
                break;
        case TU_CALL: /* print tucb info before call to exectu(). */
                tucb = va_arg(ag, CORV_TUTYPE *);
                fprintf(dbfptr,"\nTU called with the following:\n\n");
                fprintf(dbfptr, "\ttucb->ldev_name =%s\n",tucb->ldev_name);
                fprintf(dbfptr, "\ttucb->tu =%d, tucb->loop = %d\n",tucb->tu,
			tucb->loop);
                fprintf(dbfptr, "\ttucb->io_buff =%s, tucb->isr = %d\n",
			tucb->io_buff,tucb->isr);
                break;
        case TU_RET_DATA: /* print return Data */
		rc = va_arg(ag,int);
                tucb = va_arg(ag, CORV_TUTYPE *);
                fprintf(dbfptr,
			"\nTU call returned with the following data:\n\n");
                fprintf(dbfptr, "\treturn code from exectu = %d\n",rc);
                fprintf(dbfptr, "\ttucb->ldev_name =%s\n",tucb->ldev_name);
                fprintf(dbfptr, "\ttucb->tu =%d, tucb->loop = %d\n",tucb->tu,
			tucb->loop);
                fprintf(dbfptr, "\ttucb->io_buff =%s, tucb->isr = %d\n",
			tucb->io_buff, tucb->isr);
                break;
        case HEX: /* Print multiple integers (int_name = int_value). */
        case DEC: 
                i1 = va_arg(ag, int); /* Number of int name/value pairs. */
                for (i = 0; i < i1; i++)
		{
                        s1 = va_arg(ag, char *);
                        i2 = va_arg(ag, int);
			if(db_key == HEX)
				fprintf(dbfptr, "%s = %x\n", s1, i2);
			else
				fprintf(dbfptr, "%s = %d\n", s1, i2);
                }
                break;
        case STRING:
                i1 = va_arg(ag, int); /* Number of ptr name/value pairs. */
                for (i = 0; i < i1; i++)
		{
                        s1 = va_arg(ag, char *);
                        s2 = va_arg(ag, char *);
                        fprintf(dbfptr, "%s = %s\n", s1, s2);
                }
                break;
        default:
                fprintf(dbfptr,"======== end ========\n");
                break;
        }
	fflush(dbfptr);
        if (dbfptr != NULL)
                fclose(dbfptr);
        va_end(ag);

        return;

} /* end db */

