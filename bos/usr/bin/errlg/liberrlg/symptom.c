static char sccsid[] = "@(#)46	1.2  src/bos/usr/bin/errlg/liberrlg/symptom.c, cmderrlg, bos411, 9428A410j 3/31/94 10:09:17";
/*******************************************************************************
*  COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
* 
*  FUNCTIONS: setup_symptom, log_symp
* 
*  ORIGINS: 27
* 
*  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
*  combined with the aggregated modules for this product)
*                   SOURCE MATERIALS
*  (C) COPYRIGHT International Business Machines Corp. 1993
*  All Rights Reserved
* 
*  US Government Users Restricted Rights - Use, duplication or
*  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*******************************************************************************/
#include <stdio.h>
#include <sys/rasprobe.h>
#include <errlg.h>
#include <sys/sstbl.h>

static struct kwd_info
{ 
	int length;
	int kwd_ndx;
	char * addr;
};


/* Hardcoded codepoints -- these should match */
/* the codepoints listed in codepoint.desc.   */
#define LONGNAME_CODEPOINT     0xEB14
#define OWNER_CODEPOINT	       0xEB23
#define APPLID_CODEPOINT       0xEB24
#define DESC_CODEPOINT	       0xEB25
#define REPORTABLE_CODEPOINT   0xEB26
#define INTERNAL_CODEPOINT     0xEB27
#define SERNO_CODEPOINT	       0xEC34
#define SEV_CODEPOINT	       0x23
#define SYMPTOM_DATA_CODEPOINT 0x14

/*******************************************************************************
* NAME:     setup_symp_code_data
* FUNCTION: This function loops through the symptom code data that was
*	    saved in the function sort_symptom_data in symp_kwd[] and
	    stores the data in the output buffer.
*	    Each keyword and its data is separated by a space with the
*	    keyword and its data separated by a '/'. The gathered data
*	    is stored together with the SYMP_CODEPT codepoint.
*******************************************************************************/
static void setup_symp_code_data(
struct sscodept *output_buf,
char **end_ptr,
int total_sympdata_kwds,
struct kwd_info *symp_kwd )
{
	char *data_ptr;
	struct sskwd *symp_kwd_ptr;
	int symp_kwd_ndx;
	int tbl_ndx;
	char asc_val[16];
	int data_len;

	output_buf->codept = get_symptom_codept(SYMP_CODEPT);
	data_ptr = output_buf->data;

	/* loop through the saved off addresses in *symp_kwd to */
	/* get all the symptom data keywords and their data     */
	for (symp_kwd_ndx=0; symp_kwd_ndx < total_sympdata_kwds; symp_kwd_ndx++)
	{

		/* point to the address of the saved off keyword */
		symp_kwd_ptr = (struct sskwd *)symp_kwd[symp_kwd_ndx].addr;
		tbl_ndx=0;

		tbl_ndx = symp_kwd[symp_kwd_ndx].kwd_ndx;

		/* ignore the keyword if not found */
		if ( kwdtbl[tbl_ndx].kwd != NULL )
		{
			/* store the keyword name in the output buffer */
			strcpy(data_ptr, kwdtbl[tbl_ndx].kwdn);
			data_ptr += strlen(kwdtbl[tbl_ndx].kwdn);

			/* separate the keyword and its data */
			*data_ptr='/';
			data_ptr++;

			/* store the keyword data */
			if ( kwdtbl[tbl_ndx].type == DT_STR )
			{
				if ( kwdtbl[tbl_ndx].len < 
				    symp_kwd[symp_kwd_ndx].length )
					data_len = kwdtbl[tbl_ndx].len;
				else
					data_len = symp_kwd[symp_kwd_ndx].length;

				strncpy(data_ptr, &symp_kwd_ptr->SSKWD_PTR,data_len);
			}
			else if ( kwdtbl[tbl_ndx].type == DT_INT )
			{
				memset(asc_val, NULL, sizeof(asc_val));
				sprintf(asc_val,"%d",symp_kwd_ptr->SSKWD_VAL);
				data_len=strlen(asc_val);
				strncpy(data_ptr, asc_val, data_len);
			}
			else
			{
				memset(asc_val,NULL,sizeof(asc_val));
				sprintf(asc_val,"%x",symp_kwd_ptr->SSKWD_VAL);
				data_len=strlen(asc_val);
				strncpy(data_ptr,asc_val,data_len);
			}

			/* Move pointer to the next available place in memory */
			data_ptr += data_len;

			/* separate the keyword's data from the next keyword */
			/* if it is not the last keyword.			*/
			if ((symp_kwd_ndx + 1) != total_sympdata_kwds )
			{
				*data_ptr = ' ';
				data_ptr++;
			}
		}
	}

	/* Return the end address of the symptom data */
	*end_ptr=data_ptr;
}

/*******************************************************************************
* NAME:     setup_codept_flag_data  
* FUNCTION: This function loops through the keyword table, and store the
*	    symptom data that receives its own codepoint along with its
*	    codepoint into the output buffer. 
*******************************************************************************/
static void setup_codept_flag_data(
struct sscodept *output_buf,
char **end_ptr)
{
	char *temp_ptr;
	int data_len;
	int tbl_ndx;
	char asc_val[16];

	/* loop through keyword table to get the stored variables which   */
	/* receives its own codepoint and store it into the output buffer */
	for ( tbl_ndx=0; kwdtbl[tbl_ndx].kwd != 0; tbl_ndx++ )
	{
		/* Only get variable information if the keyword is supposed */
		/* to receive its own codepoint and their is data in it.    */
		if ( (kwdtbl[tbl_ndx].flags & CODEPT) &&
		    ( (*(char *)kwdtbl[tbl_ndx].kvar != NULL ) ||
		    (*(int *)kwdtbl[tbl_ndx].kvar != NULL  ) ) )
		{
			/* Store the keyword data */
			if ( kwdtbl[tbl_ndx].type == DT_STR )
			{
				data_len=strlen(kwdtbl[tbl_ndx].kvar) + 1;
				strcpy(output_buf->data, kwdtbl[tbl_ndx].kvar);
			}
			else
			{
				memset(asc_val, NULL, 16);
				sprintf(asc_val,"%d",*(int *)kwdtbl[tbl_ndx].kvar);
				data_len=strlen(asc_val) + 1;
				strcpy(output_buf->data, asc_val);
			}

			/* Store the keyword's codepoint */
			output_buf->codept = get_symptom_codept(kwdtbl[tbl_ndx].kwd);

			/* Move pointer to the next available place in memory */
			temp_ptr=(char *)output_buf;
			temp_ptr += data_len + sizeof(output_buf->codept);
			output_buf=(struct sscodept *)temp_ptr;
		}
	}

	/* Return the end address of the symptom data */
	*end_ptr=(char *)output_buf;
}

/*******************************************************************************
   NAME:      get_symptom_codept()
   FUNCTION:  This function receives a keyword and returns it corresponding
              codepoint value.
*******************************************************************************/
static int get_symptom_codept(
int kwd)
{
	int codept;

	switch (kwd)
	{
	case SSKWD_LONGNAME :
		codept = LONGNAME_CODEPOINT;
		break;
	case SSKWD_OWNER :
		codept = OWNER_CODEPOINT;
		break;
	case SSKWD_APPLID :
		codept = APPLID_CODEPOINT;
		break;
	case SSKWD_DESC :
		codept = DESC_CODEPOINT;
		break;
	case SSKWD_REPORTABLE :
		codept = REPORTABLE_CODEPOINT;
		break;
	case SSKWD_INTERNAL :
		codept = INTERNAL_CODEPOINT;
		break;
	case SSKWD_SEV :
		codept = SEV_CODEPOINT;
		break;
	case SSKWD_SN :
		codept = SERNO_CODEPOINT;
		break;
	case SYMP_CODEPT :
		codept = SYMPTOM_DATA_CODEPOINT;
		break;
	default :
		/* This is an unused codepoint */
		codept = 0xFFFF;
		break;
	}
	return(codept);
}

/*******************************************************************************
* NAME:      sort_symptom_data
* FUNCTION:  This function loops through all the error's symptom data
*            and stores either the address of the each keyword that does not
*            gets its own codepoint or stores the data in the keywords that
*            do get their own codepoint in the keyword tables variable field.
*  INPUT :	nsskwd - number of keywords 
*		*sympptr - pointer to the keyword data that is to be sorted
*			   for the error log report.
*  OUTPUT : 	*symp_kwd - This holds the information necessary to retrieve
*		 	    the symptom data for keywords that share the 
*	 		    SYMP_CODEPT codepoint at a later time.
*	 	*total_sympdata_kwds - number of keywords that are to be
*				       reported with the SYMP_CODEPT codepoint.
*******************************************************************************/
static void sort_symptom_data(
struct sskwd *sympptr,
int nsskwd,
struct kwd_info *symp_kwd,
int *total_sympdata_kwds)
{
	int     keywd_ctr;
	int     tbl_ndx;
	int     data_len;
	int     cpy_data_len;
	int     symp_kwd_ndx = 0;
	Boolean found;
	char    *temp_ptr;

	/* loop through entire symptom data and sort the data appropriately */
	/* to be stored in the output buffer later which is used to print   */
	/* the symptom data in the appropriate order on the error report    */
	for ( keywd_ctr=0; keywd_ctr < nsskwd; keywd_ctr++ )
	{
		tbl_ndx = 0;
		found = 0;
		/* find keyword in the keyword table, ignore */
		/* the keyword if its not in the table       */
		while ( (!found) && (kwdtbl[tbl_ndx].kwd != 0) )
		{
			if ( kwdtbl[tbl_ndx].kwd == sympptr->sskwd_id )
			{
				found = TRUE;
				/* store the keywords data in the keyword */
				/* table's variable when the keyword indi-*/
				/* cates in the kwdtbl that it is a codept*/
				/* i.e. if it does not get its own codept */
				/* then the keyword is merged together    */
				/* with other keywords and is the data for*/
				/* the SYMP_CODEPT codepoint.		  */
				if ( kwdtbl[tbl_ndx].flags & CODEPT )
				{
					if ( kwdtbl[tbl_ndx].type == DT_STR )
					{
						data_len = strlen(&sympptr->SSKWD_PTR) + 1;
						cpy_data_len = kwdtbl[tbl_ndx].len + 1;
						if ( data_len < cpy_data_len )
							cpy_data_len = data_len;
						strncpy(kwdtbl[tbl_ndx].kvar,&sympptr->SSKWD_PTR,cpy_data_len);
					}
					else
					{
						data_len = sizeof(sympptr->SSKWD_VAL);
						*(int *)kwdtbl[tbl_ndx].kvar = (int *)sympptr->SSKWD_VAL;

					}
				}
				/* save the address of the symptom data to be used to get the */
				/* data information and put into the symptom data keyword.    */
				else
				{
					if ( kwdtbl[tbl_ndx].type == DT_STR )
						data_len = strlen(&sympptr->SSKWD_PTR) + 1;
					else
						data_len = sizeof(sympptr->SSKWD_VAL);
					symp_kwd[symp_kwd_ndx].addr = (char *)sympptr;
					symp_kwd[symp_kwd_ndx].length = data_len - 1;
					symp_kwd[symp_kwd_ndx].kwd_ndx = tbl_ndx;
					symp_kwd_ndx++;
				}
				/* Move pointer to the next available place in memory */
				temp_ptr = (char *)sympptr;
				temp_ptr += data_len + sizeof(sympptr->sskwd_id);
				sympptr = (struct sskwd *)temp_ptr;
			}
			tbl_ndx++;
		}
	}
	/* Return the total number of keywords */
	*total_sympdata_kwds=symp_kwd_ndx;
}

/*******************************************************************************
* NAME:      clear_kwdtbl
* FUNCTION:  This function initializes the keyword variables values to 0.
*******************************************************************************/
static void clear_kwdtbl()
{
	int tbl_ndx;

	/* initialize the keyword table variable to 0 if the variable is not NULL */
	for (tbl_ndx=0; kwdtbl[tbl_ndx].kwd != NULL; tbl_ndx++)
		if (kwdtbl[tbl_ndx].kvar != NULL)
			if (kwdtbl[tbl_ndx].type == DT_STR)
			   *(char *)kwdtbl[tbl_ndx].kvar = NULL;
			else
			   *(int *)kwdtbl[tbl_ndx].kvar = NULL;
			   
}

/*******************************************************************************
* NAME:      setup_symptom
* FUNCTION:  This function takes as input symptom errlog data
*            and reorders the data for use by errpt.
*******************************************************************************/
setup_symptom(
struct sympt_data *buf,
struct sscodept *output_buf,
char **end_ptr)
{
	struct kwd_info symp_kwd[MAX_SYMPTOM_KWDS];
	struct sskwd *symp_kwddata_ptr;
	int  total_sympdata_kwds;
	char *new_output_ptr;

	/* Initialize keyword table variables */
	clear_kwdtbl();

	symp_kwddata_ptr=(struct sskwd *)buf->sympdata;

	/* This function sorts the symptom data by keywords that get   */
	/* their own codepoints vs. one that share the SYMP_CODEPT     */
	/* codepoint. The keywords that get their own codepoint stores */
	/* its data in the keyword table. The keywords that do not get */
	/* their own data, stores all the necessary information to     */
	/* retrieve it later into the symp_kwd array.		       */
	sort_symptom_data(symp_kwddata_ptr, buf->nsskwd, &symp_kwd[0], &total_sympdata_kwds);

	setup_codept_flag_data(output_buf,&new_output_ptr);

	/* save original output buffer pointer */
	output_buf=(struct sscodept *)new_output_ptr;

	/* setup reportable codepoint data */
	/* When the SSKWD_REPORTABLE codepoint is set to 0   */
	/* this means that the symptom data is not to be     */
	/* forwarded to automatic problem opening facilities,*/
	/* otherwise it is.		   */
	output_buf->codept = get_symptom_codept(SSKWD_REPORTABLE);
	if ( buf->flags & SSNOSEND )
		strcpy(output_buf->data, "0");
	else
		strcpy(output_buf->data, "1");
	new_output_ptr=(char *)output_buf;
	new_output_ptr += sizeof(char) + sizeof(output_buf->codept) + 1;
	output_buf=(struct sscodept *)new_output_ptr;

	/* setup internal codepoint data */
	/* When the SSKWD_INTERNAL codepoint is set to 0 this */
	/* indicates the product is not an IBM product other- */
	/* wise it is.					      */
	output_buf->codept = get_symptom_codept(SSKWD_INTERNAL);
	if ( buf->probe_magic == SYSPROBE_MAGIC )
		strcpy(output_buf->data, "1");
	else
		strcpy(output_buf->data, "0");

	/* Move pointer to the next available place in memory */
	new_output_ptr=(char *)output_buf;
	new_output_ptr += sizeof(char) + sizeof(output_buf->codept) + 1;
	output_buf=(struct sscodept *)new_output_ptr;

	setup_symp_code_data(output_buf,&new_output_ptr,total_sympdata_kwds,
	    &symp_kwd[0]);

	/* Return the end address of the output buffer */
	*end_ptr=new_output_ptr;
	fflush(NULL);
}
