static char sccsid[] = "@(#)25  1.1  src/bos/usr/ccs/lib/libdiag/diag_trace.c, cmddiag, bos41J, 9514A_all 4/4/95 %%";
/*
 *   COMPONENT_NAME: libdiag
 *
 *   FUNCTIONS: Diagnostic Trace
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include        <stdio.h>
#include        <fcntl.h>
#include        <diag/diag.h>
#include        <diag/diago.h>
#include        <diag/tm_input.h>
#include        <diag/tmdefs.h>
#include        <diag/da.h>
#include        <diag/da_rc.h>
#include        <diag/diag_exit.h>
#include        <diag/scsi_atu.h>
#include        <sys/scsi.h>
#include        <diag/diag_trace.h>

void dt(char *, int, ...);
void p_data(FILE *, char *, int);

/*
 * NAME: dt()
 *
 * FUNCTION: Diagnostic Trace (dt).
 *
 * PROTOTYPE:	void	dt(char *dt_id, int, ...);
 *
 * NOTES:
 *
 *	- The trace file will be called /tmp/.dt.dt_id
 *	  Where dt_id is passed as the first parameter.
 *	- The function has variable arguments and will call va_start()
 *	  and va_end() even if the trace file does not exist.
 *	- DT Type Examples:
 *		dt("scsi2", DT_DEC, "rc", rc);
 *			output:   rc = 0
 *		dt("scsi2", DT_MDEC, 3, "rc", rc, "x", x, "y", y);
 *			output:   rc = 0  x = 177  y = 43
 *		dt("scsi2", DT_MSG, "hello");
 *			output:   hello
 *		dt("scsi2", DT_MSTR, 2, "dt_id", dt_id, "location", dev_loc);
 *			output:   dt_id = scsi2  location = 00-00-00-0,0
 *		dt("scsi2", DT_BUFF, buffer, 32)
 *			output:  00 00 00 00 00 00 00     00 00 00 00 00 00 00
 *				 00 00 00 00 00 00 00     00 00 00 00 00 00 00
 *
 * RETURNS: void
 *
 */

void dt(char *dt_id, int dt_type, ...) {
	int	      i, i1 = 0, i2 = 0, rc = 0;
	char	      *s1, *s2;
	char	      fname[256];
	SCSI_TUTYPE   *dt_tucb;
	FILE	      *dtfptr;
	struct stat   file_status;
	struct tm_input dt_tmi;
	va_list       ag;

	va_start(ag, dt_type);

	if ((rc = stat("/tmp/.DIAG_TRACE",&file_status)) == -1) {
		va_end(ag);
		return; /* Debug file not present. */
	}

	sprintf(fname, "/tmp/.dt.%s", dt_id);

	if (!strcmp((char *)getenv("DIAG_DMEDIA_TRACE"),"APPEND")) {
		dtfptr = fopen(fname,"a+");
	} else {
		if (dt_type == DT_TMI) {
			dtfptr = fopen(fname,"w+");
		} else {
			dtfptr = fopen(fname,"a+");
		}
	}

	switch(dt_type) {
	case DT_TMI: /* init trace file, print some of tm_input. */
		dt_tmi = va_arg(ag, struct tm_input);
		fprintf(dtfptr,"============= start %s ============\n",
			dt_tmi.dname);
		fprintf(dtfptr,
			"CONSOLE=%d AVDANCED=%d EXENV=%d SYSTEM=%d\n",
			dt_tmi.console,
			dt_tmi.advanced,
			dt_tmi.exenv,
			dt_tmi.system);
		fprintf(dtfptr,"DMODE=%d (1=ELA 2=PD 4=REPAIR)\n",
			dt_tmi.dmode);
		fprintf(dtfptr,"LOOPMODE=%d (1=NOT 2=ENTER 4=IN 8=EXIT)\n",
			dt_tmi.loopmode);
		break;
	case DT_SCSI_TUCB: /* print dt_tucb info before call to exectu(). */
		dt_tucb = va_arg(ag, SCSI_TUTYPE *);
		fprintf(dtfptr,"SCSI TUCB CDB\n\t");
		p_data(dtfptr, dt_tucb->scsitu.scsi_cmd_blk,
			dt_tucb->scsitu.command_length);
		if ((dt_tucb->scsitu.flags == B_WRITE) &&
		    (dt_tucb->scsitu.data_length)) {
			fprintf(dtfptr,"SCSI TUCB Data Buffer\n\t");
			p_data(dtfptr, dt_tucb->scsitu.data_buffer,
			       dt_tucb->scsitu.data_length);
		}
		break;
	case DT_SCSI_TUCB_SD: /* print Sense Data */
		dt_tucb = va_arg(ag, SCSI_TUTYPE *);
		fprintf(dtfptr,
			"SCSI Sense Data, SKey %02.2X, ASC/ASCQ = %04.4X\n",
			(dt_tucb->scsiret.sense_key & 0x0F),
			dt_tucb->scsiret.sense_code);
		fprintf(dtfptr,"Sense Data Buffer\n\t");
		p_data(dtfptr, dt_tucb->scsitu.data_buffer,
		       dt_tucb->scsitu.data_length);
		break;
	case DT_DEC: /* Print integer variable (int_name = int_value). */
	case DT_HEX:
		i1 = 1;
	case DT_MDEC:
	case DT_MHEX:
		if (i1 == 0) {
			 /* Get number of int name/value pairs. */
			 i1 = va_arg(ag, int);
		}
		for (i = 0; i < i1; i++) {
			s1 = va_arg(ag, char *);
			i2 = va_arg(ag, int);
			if ((dt_type == DT_HEX) || (dt_type == DT_MHEX))
				fprintf(dtfptr, "%s = %X  ", s1, i2);
			else /* Decimal */
				fprintf(dtfptr, "%s = %d  ", s1, i2);
		}
		fprintf(dtfptr, "\n");
		break;
	case DT_MSTR: /* print  multiple strings (ptr_name = ptr). */
		i1 = va_arg(ag, int); /* Number of ptr name/value pairs. */
		for (i = 0; i < i1; i++) {
			s1 = va_arg(ag, char *);
			s2 = va_arg(ag, char *);
			fprintf(dtfptr, "%s = %s\n", s1, s2);
		}
		break;
	case DT_MSG:
		/* Print a simple string. */
		s1 = va_arg(ag, char *);
		fprintf(dtfptr, "%s\n",s1);
		break;
	case DT_BUFF: /* Display buffer data. */
		s2 = va_arg(ag, char *);
		i1 = va_arg(ag, int);
		fprintf(dtfptr, "Data Buffer\n\t");
		p_data(dtfptr, s2, i1);
		break;
	case DT_END:
		fprintf(dtfptr,"======== End of Trace ========\n");
		break;
	default:
		break;
	}
	/* fflush(dtfptr); */
	if (dtfptr != NULL)
		fclose(dtfptr);
	va_end(ag);

	return;

} /* end dt */

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

void p_data(FILE *dtfptr, char *ptr, int len) {
	int count = 0;

	/* Print data buffer, 16 bytes per line. */
	while ((len--) > 0) {
		if (count == 8) {
			fprintf(dtfptr,"    ");
		} else if (count == 16) {
			fprintf(dtfptr,"\n\t");
			count = 0;
		}
		fprintf(dtfptr,"%02.2X ", *(ptr++));
		++count;
	}
	fprintf(dtfptr,"\n");

	return;
} /* end p_data */


