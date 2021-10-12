static char sccsid[] = "@(#)85	1.11  src/bos/diag/da/sdisk/dh2ela.c, dasdisk, bos411, 9428A410j 9/27/93 14:31:26";
/*
 *   COMPONENT_NAME: DASDISK
 *
 *   FUNCTIONS: add_entry
 *		analyze_error_log
 *		check_adapter_error_log
 *		check_cont_error_log
 *		check_disk_error_log
 *		check_microcode_file
 *		free_uec_list
 *		segment_count
 *		test_if_failed
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* include of header files. Note: Keep include files alphabetized */

#include "dh2.h"
#include "dh2srn.h"
#include "dh2_msg.h"
#include <diag/bit_def.h>
#include <diag/da.h>
#include <diag/dcda_msg.h>
#include <diag/diag.h>
#include <diag/diag_exit.h>
#include <diag/diago.h>
#include <diag/tm_input.h>
#include <diag/tmdefs.h>
#include <limits.h>
#include <stdio.h>
#include <sys/buf.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/devinfo.h>
#include <sys/device.h>
#include <sys/errids.h>
#include <sys/ioctl.h>
#include <sys/sysconfig.h>
#include <sys/stat.h>

/* External functions used in this file */

extern			error_log_get();
extern			struct	tm_input tm_input;/* DA test params */

/* Structure used locally in this module */

struct	uec_list {
	int	uec;
	int	count;
	struct	uec_list	*next;
};

static	struct	uec_list	*head_list = (struct uec_list *)NULL;
static	struct	uec_list	*tail_ptr = (struct uec_list *)NULL;

/*  */
/*
 * NAME: analyze_error_log()
 *
 * FUNCTION: Performs Error Log Analysis for the Serial disk subsystem.
 *
 * EXECUTION ENVIRONMENT: This function can page fault.
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: If a problem is found, an SRN will be returned.
 */

int
analyze_error_log(device_type)
int	device_type;
{
	int	check_disk_error_log();
	int	check_adapter_error_log();
	int	check_cont_error_log();

	switch (device_type) {
	case	 DISK: /* Disk */
		return( check_disk_error_log() );
	case	ADAPTER: /* adapter */
		return( check_adapter_error_log() );
	case	CONTROLLER: /* controller */
		return( check_cont_error_log() );
	default:
		return( 0 );
	}
}

/*  */
/*
 * NAME: check_adapter_error_log
 *
 * FUNCTION: Search and analyze the error log for Serial DASD adapter.
 *
 * EXECUTION ENVIRONMENT: This function can page fault.
 *
 * NOTES: Currently, this function only goes back 24 hours into
 *        the error log to search for all type of errors.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: If a problem is found, an SRN will be returned.
 */
int
check_adapter_error_log()
{
	void	 free_uec_list();
	void	 add_entry();
	struct   errdata err_data;                 /* error log data template */
	struct	 uec_list *err_list;   /* index for list searching */
	char     srch_crit[80];                    /* search criteria string  */
	register int      op_flg;           /* tells error_log_get what to do */
	register int      rc = -99;         /* return code from error_log_get */
	int	 srn = 0;
	int	 uec = 0;

	sprintf( srch_crit, SRCHSTR, tm_input.date, tm_input.dname );
	op_flg = INIT;
	do {
		/* get 1st entry in log if any */
		rc = error_log_get( op_flg, srch_crit, &err_data );
		if ( rc < 0 ) {
			op_flg = TERMI;
			rc = error_log_get( op_flg, srch_crit, &err_data );
		}
		if ( rc > 0 ) { 
                        switch ( err_data.err_id) {
			/* Unrecovered errors */
			case	ERRID_SDA_ERR1:
			case	ERRID_SDA_ERR3:
				srn = (int)(err_data.detail_data[0] << 8) |
					(int)err_data.detail_data[1];
				op_flg = TERMI;
				break;
			/* Recovered errors */
			case	ERRID_SDA_ERR2:
			case	ERRID_SDM_ERR1:
			case	ERRID_SDA_ERR4:
				uec = ( (int)(err_data.detail_data[0]) & 0x0f)
					 << 8 | (int)err_data.detail_data[1];
				add_entry(uec);
				op_flg = SUBSEQ;
				break;
			default:
				op_flg = SUBSEQ;
				break;
			}
                } else 
			op_flg = TERMI;
	} while ( op_flg != TERMI );
	rc =  error_log_get( op_flg, srch_crit, &err_data );
	if(srn == 0){
		err_list=head_list;
		while(err_list != (struct uec_list *)NULL){
			if(err_list->count >= MAX_HRD_ERRS){
				srn=err_list->uec;
				break;
			}
			err_list = err_list->next;
		}
	}
	free_uec_list();
	return( srn );
}

/*  */
/*
 * NAME: check_cont_error_log
 *
 * FUNCTION: Search and analyze the error log for Serial DASD controller.
 *
 * EXECUTION ENVIRONMENT: This function can page fault.
 *
 * NOTES: Currently, this function only goes back 24 hours into
 *        the error log to search for all type of errors.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: If a problem is found, an SRN will be returned.
 */
int
check_cont_error_log()
{
	struct   errdata err_data;                 /* error log data template */
	struct	 uec_list *err_list;   /* index for list searching */
	char     srch_crit[80];                    /* search criteria string  */
	register int      op_flg;           /* tells error_log_get what to do */
	register int      rc = -99;         /* return code from error_log_get */
	int	 srn = 0;
	int	 uec = 0;

	sprintf( srch_crit, SRCHSTR, tm_input.date, tm_input.dname );
	op_flg = INIT;
	do {
		/* get 1st entry in log if any */
		rc = error_log_get( op_flg, srch_crit, &err_data );
		if ( rc < 0 ) {
			op_flg = TERMI;
			rc = error_log_get( op_flg, srch_crit, &err_data );
		}
		if ( rc > 0 ) { 
                        switch ( err_data.err_id) {
			/* Unrecovered errors */
			case	ERRID_SDC_ERR1:
			case	ERRID_SDC_ERR2:
				srn = (int)(err_data.detail_data[40] << 8) |
					(int)err_data.detail_data[41];
				op_flg = TERMI;
				break;
			/* Recovered errors */
			case	ERRID_SDC_ERR3:
			case	ERRID_SDM_ERR1:
				uec = ( (int)(err_data.detail_data[40]) & 0x0f)
					 << 8 | (int)err_data.detail_data[41];
				add_entry(uec);
				op_flg = SUBSEQ;;
				break;
			default:
				op_flg = SUBSEQ;;
				break;
			}
                } else 
			op_flg = TERMI;
	} while ( op_flg != TERMI );
	rc =  error_log_get( op_flg, srch_crit, &err_data );
	if(srn == 0){
		err_list=head_list;
		while(err_list != (struct uec_list *) NULL){
			if(err_list->count >= MAX_HRD_ERRS){
				srn=err_list->uec;
				break;
			}
			err_list = err_list->next;
		}
	}
	free_uec_list();
	return( srn );
}
/*  */
/*
 * NAME: check_disk_error_log
 *
 * FUNCTION: Search and analyze the error log for Serial DASD.
 *
 * EXECUTION ENVIRONMENT: This function can page fault.
 *
 * NOTES: Currently, this function only goes back 24 hours into
 *        the error log for all errors.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: If a problem is found, an SRN will be returned.
 */
int
check_disk_error_log()
{
	uint	 segment_count();
	struct   errdata err_data;                 /* error log data template */
	char     srch_crit[80];                    /* search criteria string  */
	register int      op_flg;           /* tells error_log_get what to do */
	register int      i;                              /* counter variable */
	register int      rc = -99;         /* return code from error_log_get */
	int	 srn = 0;
	int	 uec = 0;
	int	 hard_data_errors=0;
	int	 soft_data_errors=0;
	short	 initialized=FALSE;
	int	 seg_count=0;   /* used to calculate segment count */
	int	 initcnt=0;   /* Set up for first value of segment count seen */
	int	 nextcnt=0;   /* Current segment count value seen in the log */
	int	 previous=0;  /* Previous segment count value.   */
	short	 found = FALSE;
	int	 lba;
	int	 stored_lbas[MAX_HRD_ERRS];/* Array to store up to 3 lbas */
	int	 total_lba_count = 0;

	sprintf( srch_crit, SRCHSTR, tm_input.date, tm_input.dname );
	for( i = 0; i < MAX_HRD_ERRS; i++ )
		stored_lbas[ i ] = -1;
	op_flg = INIT;
	do {
		/* get 1st entry in log if any */
		rc = error_log_get( op_flg, srch_crit, &err_data );
		if ( rc < 0 ) {
			op_flg = TERMI;
			rc = error_log_get( op_flg, srch_crit, &err_data );
		}
		if ( rc > 0 ) {
			uec = ( (int)(err_data.detail_data[40]) & 0x0f) << 8 
					    | (int)(err_data.detail_data[41]);
			/* Skip error logged 0x800 or 0x801 from */
			/* Serial 2.0GB Disk */
				
			if(uec == 0x800 || uec == 0x801)
			{
				db(10,1,"uec",uec);
				op_flg=SUBSEQ;
				
			}
			else{
			
			switch ( err_data.err_id) {
			case ERRID_DISK_ERR1:
				db(25,"SCSI ERRID_DISK_ERR1");
				db(10,2,"uec",uec,"line",__LINE__);
				if((uec == 0x0285) || (uec == 0x0290)){
				/* Unrecovered Media Error. Make sure error */
				/* occuring at the same LBA is not counted  */
				/* again.			            */			
				     if((int)err_data.detail_data[20] & 0x80){
			        	 lba=((int)err_data.detail_data[23]
							<< 24)+
				     		((int)err_data.detail_data[24]
							<< 16)+
				     		((int)err_data.detail_data[25]
							<< 8)+
				     		(int)err_data.detail_data[26];

				/* Search in lba list to see if it is in */
				          for(i=0; i< MAX_HRD_ERRS; i++)
					        if(lba == stored_lbas[i]) {
					       		found = TRUE;
						   	break;
				     		 }
			          		 if(found) /* Reset Flag */
			              			found = FALSE;
			          		 else {
				      			if(total_lba_count<
							     MAX_HRD_ERRS){
				          			stored_lbas[
								     total_lba_count] = lba;
				          			++total_lba_count;
				     		 	}
				      			++hard_data_errors;
			          		}
			       		} else /* invalid lba,count the error */
					     ++hard_data_errors;
				       if( hard_data_errors == MAX_HRD_ERRS ) {
						srn = uec;
						op_flg = TERMI;
				       } else 
			 			op_flg = SUBSEQ;
			       } else { /* Call out SRN right away for other */
					/* errors		             */
					srn = uec;
					op_flg = TERMI;
			       }			     
			       break;

			case ERRID_DISK_ERR2:
			/* Hardware Error */
				db(25,"SCSI ERRID_DISK_ERR2");
				db(10,2,"uec",uec,"line",__LINE__);
  				srn = uec;
				op_flg = TERMI;
				break;

			case ERRID_DISK_ERR4:
			/* skip on this pass */
				db(25,"SCSI ERRID_DISK_ERR4");
				op_flg = SUBSEQ;
				break;
			default:
				op_flg = SUBSEQ;
				break;
			}
		}
                } else 
			op_flg = TERMI;
	} while ( op_flg != TERMI );
	/* close out error_log_get */
	rc =  error_log_get( op_flg, srch_crit, &err_data );
	if( srn == 0 ){ /* No error seen so far, search for recovered errors */
		        /* Process error log again this time only look for   */
  		        /* soft errors and search by name only.              */

        	sprintf( srch_crit, " -N %s" , tm_input.dname );
       		op_flg = INIT;
	       	do {
	             /* get 1st entry in log if any */
                     rc = error_log_get( op_flg, srch_crit, &err_data );
                     if ( rc < 0 ) {
			op_flg = TERMI;
                       	rc = error_log_get( op_flg, srch_crit, &err_data );
                       	clean_up();
               	     }
		     if ( rc > 0 ) 
		     {
			uec = ( (int)(err_data.detail_data[40]) & 0x0f)
				<< 8 | (int)(err_data.detail_data[41]);
			/* Skip error logged 0x800 or 0x801 from */
			/* Serial 2.0GB Disk */
				
			if(uec == 0x800 || uec == 0x801)
			{
				db(10,2,"uec",uec,"ignore soft errors, line",
					__LINE__);
				op_flg=SUBSEQ;
				
			}
			else
                       	switch ( err_data.err_id) {
                      	case ERRID_DISK_ERR4:
				if((uec == 0x0282) || (uec == 0x0283)){

			/* Only threshold errors with UEC of 282 and 283.
			 * Determine total segment count by substracting the
			 * last segment count value in the log from the segment
			 * count value of the first entry. Also handle case
			 * where segment count is not in decreasing order in
			 * the log.
 			 */
			  	      if(!initialized){
						initcnt =  segment_count(err_data);
						previous = initcnt;
						initialized = TRUE;
			       	      } else {
						nextcnt = segment_count(err_data);

			/* Current segment count should always
			 * be less than previous. If not, reset has occured.
		 	 */
						if(nextcnt > previous) {

			/* compute segment count then reset initcnt.  */

						       if(initcnt == previous)
						           seg_count=seg_count + initcnt;
							else
							   seg_count=seg_count+(initcnt-previous);
							initcnt = nextcnt;
						}
						previous = nextcnt;
					}
					++soft_data_errors;
				        op_flg = SUBSEQ;
				} else {
					srn = uec;
					op_flg = TERMI;
				}
			        break;
			default:
				op_flg = SUBSEQ;
			        break;
			}
		      }
		      else { /* rc == 0 */
			 op_flg = TERMI;
			 if(( soft_data_errors > 1 ) && ( srn == 0)) {
				if(initcnt == nextcnt)
				     seg_count = seg_count + initcnt;
				else
				     seg_count = seg_count + (initcnt - nextcnt);
				if(test_if_failed( seg_count, soft_data_errors))
				     srn = ELA_FOUND_SOFT_ERRS;
			 }
		      }
		} while ( op_flg != TERMI );
                rc = error_log_get( op_flg, srch_crit, &err_data );
	} /* srn == 0 */
	return(srn);

} /* endfunction check disk error log */

/*  */
/* NAME:segment_count()
 *
 * FUNCTION: Calculate the segment count from the detail data field
 * 	     of the error entry.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: Byte 148-151 represents the segment count.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: errdata
 *
 * RETURNS: segment count.
 */

uint segment_count(err_data)
struct	errdata err_data;
{
	uint	seg_cnt;

	seg_cnt =( (uint)( err_data.detail_data[148] << 24 )+
	    ( err_data.detail_data[149] << 16 )+
	    ( err_data.detail_data[150] << 8 )+
	    ( err_data.detail_data[151] ) );
	return(seg_cnt);
}
/*  */
/*
 * NAME:test_if_failed()
 *
 * FUNCTION: Tests if the number of soft data/equipment errors found in the
 *           error log have exceeded the limits found in the PDG.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: errdata
 *
 * RETURNS: None.
 */

int test_if_failed( segment_count, tot_errs)
int	segment_count;
int     tot_errs;
{
	int     it_failed_ela = FALSE;             /* did it fail             */
	float	error_rate, threshold;

	threshold = 25.0 / 40.0;
	if( (segment_count >= 1000) || (
	    (tot_errs >= 20) && (segment_count>=10))){
		error_rate = (float)tot_errs / (float)segment_count;
		it_failed_ela = (error_rate > threshold);
	}
	return( it_failed_ela );
} /* endfunction test_if_failed */
/*  */
/*
 * NAME:add_entry()
 *
 * FUNCTION: Set up list of UEC found in error log.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: None.
 */

void
add_entry(uec)
int	uec;
{
	int	found = FALSE;
	struct	uec_list	*entry, *list_ptr;

	if(head_list == (struct uec_list *) NULL){
	/* empty list */
		head_list = (struct uec_list *)malloc(sizeof(struct uec_list));
		if(head_list != (struct uec_list *) NULL){
			head_list->uec = uec;
			head_list->count = 1;
			head_list->next = (struct uec_list *) NULL;
			tail_ptr = head_list;/* head & tail points to same place */
		} /* Else need to return because we cannot malloc */
	} else {
		/* search to see if uec is in list, if so increment count */
		list_ptr = head_list;
		while( list_ptr != (struct uec_list *) NULL ){
			if(uec == list_ptr->uec){
				list_ptr->count++;
				found=TRUE;
				break;
			}
			list_ptr = list_ptr->next;
		}
		if(!found){
		/* uec not in list, put it in with count set to 1 */
			entry = (struct uec_list *) malloc(sizeof
						(struct uec_list) );
			if(entry != (struct uec_list *) NULL ){
				entry->next = (struct uec_list *)NULL;
				entry->uec = uec;
				entry->count = 1;
				tail_ptr->next = entry;
				tail_ptr = entry;
			}
		}
	}
}

/*  */
/*
 * NAME:free_uec_list()
 *
 * FUNCTION: free memory allocated for uec error list
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: None.
 */

void
free_uec_list()
{
	struct uec_list *next;

	while(head_list != (struct uec_list *) NULL){
		next = head_list->next;
		free( head_list );
		head_list = next;
	}
}

/*  */
/*
 * NAME:check_microcode_file()
 *
 * FUNCTION: Check for presence of microcode file.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * RETURNS: -1 if file is not present, 0, otherwise.
 */

int
check_microcode_file(device)
char	*device;
{

	int	rc;
	char	mpath[255];

	rc = findmcode("8f78.00", mpath, VERSIONING, NULL);
	db(10,1,"return code from find mcode",rc);
	return((rc == 1) ? 0 : -1);
}
