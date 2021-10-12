static char sccsid[] = "@(#)61  1.7  src/bos/usr/bin/errlg/liberrlg/dyn_ent_util.c, cmderrlg, bos411, 9428A410j 7/12/94 10:04:46";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: pack_dynamic_ent, ann_bad_buf_entry, valid_be, un_pack_dynamic_ent, 
 *            unpack_le, le_bad_msg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Package of routines to process error logging dynamic entries.
 */

#include <sys/err_rec.h>
#include <errlg.h>


void	pack_dynamic_ent(int length,char* buf,int* p);
void	ann_bad_buf_entry(struct buf_entry *be);
int		valid_be(struct buf_entry *be);
int		un_pack_dynamic_ent(char *data,int *p,int max_size);
int		unpack_le(char *buf,struct obj_errlog *lep, unsigned esize);
int		invalid_le_static(char *buf, int offset,struct le_bad *le_bad);
void	le_bad_msg(struct le_bad *le_bad);


extern char Logfile[PATH_MAX];
extern	struct log_hdr log_hdr;

 /*
  * NAME:      pack_dynamic_ent()
  * FUNCTION:  Pack supplied dynamic entry data into the supplied
  *            buffer.
  * RETURNS:   NONE
  */

void
pack_dynamic_ent(
	int	length,		/* length of supplied entry */
	char* data,		/* dynamic entry data */
	int* p)			/* pointer to buffer to pack in */
{

	memcpy(*p, &length,sizeof(int));
	*p += sizeof(int);
	if (length > 0) {
		memcpy(*p,data,length);
		*p += length;
	}

}


 /*
  * NAME:      ann_bad_buf_entry()
  * FUNCTION:  Annunciate a bad error buffer entry.  Put an entry into the
  *            error log reflecting that a bad buffer entry was recieved.  Pack
  *            the buffer entry crcid, length, timestamp, and resource name
  *			   into the error entry detail data.
  *			   NOTE:  Care must be exercised filling in the log entry as it
  *					  cannot be validated.
  * INPUTS:	   Pointer to a buffer entry.
  * RETURNS:   NONE
  */

void
ann_bad_buf_entry(struct buf_entry *be)
 {
	struct buf_entry be_err;	/* to hold ERRID_ANN_BAD_BUF_ENTRY entry */
	int det_len;				/* detail data length */
	char* le;					/* pointer to packed log entry */


	init_be_internal(&be_err,ERRID_BAD_BUF_ENTRY);

	det_len = 3*sizeof(int) + ERR_NAMESIZE;

	memcpy(&(be_err.detail_data),&be->err_rec.error_id,sizeof(int));
	memcpy(&(be_err.detail_data[sizeof(int)]),&be->erec.erec_len,sizeof(int));
	memcpy(&(be_err.detail_data[2*sizeof(int)]),&be->erec.erec_timestamp,sizeof(int));
	memcpy(&(be_err.detail_data[3*sizeof(int)]),&be->err_rec.resource_name,ERR_NAMESIZE);

	be_err.erec.erec_len += det_len;
	be_err.erec.erec_rec_len += det_len;


	/* pack a log_entry from a provided buf_entry */
	if((le=be_to_le(&be_err)) != NULL)		/* then log the entry */
	{
		logappend(le);
	}

 }


 /* NAME:      valid_be()
  * FUNCTION   Validate the supplied error buffer entry. Check the
  *            entry length, the resource name length, the timestamp
  *	       and the symptom data length.
  * RETURNS:
  *            1 - supplied entry is VALID
  *            0 - supplied entry is INVALID
  */

int
valid_be(struct buf_entry *be)
{
	int	rc;

	if(!be) 	/* bad pointer */
		rc = 0;
	else if(!in_range(sizeof(struct erec0),be->erec.erec_len,EREC_MAX))
		rc = 0;
	else if(!val_name(be->err_rec.resource_name,ERR_NAMESIZE))	/* nonempty, printable, etc. */
		rc = 0;
	else if (!in_range(0,be->erec.erec_symp_len, SSREC_MAX))
		rc = 0;
	else if (!in_range(0,be->erec.erec_rec_len,ERR_REC_MAX_SIZE))
		rc = 0;
	else
		rc = 1;

	return(rc);
}


 /*
  * NAME:      un_pack_dynamic_ent()
  * FUNCTION:  Unpack a dynamic entry, length & data, from
  *            a supplied packed errlog entry buffer and store
  *	           it in the provided data variable.  Increment 
  *	           the packed buffer pointer past the entry.
  *	RETURNS:
  *	           length of dynamic data.
  */

int
un_pack_dynamic_ent(char *data,		/* return data */
					int	*p,			/* packed buffer */
					int max_size)	/* maximum size of data */
{
	int	length;

	memcpy(&length,*p,sizeof(int));
	*p += sizeof(int);
	if (length > 1)
		length = limit_range(0,length,max_size);
	else
		if (length < 0)
			length = 0;
	if (length > 0) {	/* unpack the data */
		memcpy(data,*p,length);
		*p += length;
	}
	return(length);
}


 /*
  * NAME:      unpack_le()
  * FUNCTION:  Unpack the contents of a given buffer and format it as an
  *            error log entry, struct obj_errlog. The given struct
  *            obj_errlog will be zero filled before processing since
  *            the structure is reused by the calling routine.
  *            Note:
  *                If the length of dynamic entry is 0 then there is no
  *                associated dynamic data in the buffer (on the disk).
  * RETURNS:
  *            1 - good entry formatted.
  *            0 - supplied pointer(s) is NULL.
  */

int
unpack_le(char *buf, struct obj_errlog *le, unsigned entry_size)
{
	char	*p;
	int		length;
	int		rc;

	/* Note:
	 *		the entry lengths, magic, sequence, and time are validated in logget().
	 */

	if (buf != NULL && le != NULL) {
		rc = 1;
		p = buf;
		bzero((char *)le, sizeof(struct obj_errlog));
					/* length and magic not in obj_errlog */
		p += sizeof(log_entry.le_length) + sizeof(log_entry.le_magic);

		memcpy(&(le->el_sequence),p,sizeof(le->el_sequence));
		p += sizeof(le->el_sequence);
		
		memcpy(&(le->el_timestamp),p,sizeof(le->el_timestamp));
		p += sizeof(le->el_timestamp);

		memcpy(&(le->el_crcid),p,sizeof(le->el_crcid));
		p += sizeof(le->el_crcid);

		(void)un_pack_dynamic_ent(le->el_machineid,&p,LE_MACHINE_ID_MAX);
		
		(void)un_pack_dynamic_ent(le->el_nodeid,&p,LE_NODE_ID_MAX);
		
		(void)un_pack_dynamic_ent(le->el_resource,&p,LE_RESOURCE_MAX);

		(void)un_pack_dynamic_ent(le->el_vpd_ibm,&p,LE_VPD_MAX);

		(void)un_pack_dynamic_ent(le->el_vpd_user,&p,LE_VPD_MAX);

		(void)un_pack_dynamic_ent(le->el_in,&p,LE_IN_MAX);

		(void)un_pack_dynamic_ent(le->el_connwhere,&p,LE_CONN_MAX);

		(void)un_pack_dynamic_ent(le->el_rclass,&p,LE_RCLASS_MAX);

		(void)un_pack_dynamic_ent(le->el_rtype,&p,LE_RTYPE_MAX);

		le->el_detail_length = un_pack_dynamic_ent(le->el_detail_data,&p,LE_DETAIL_MAX);
		le->el_symptom_length = un_pack_dynamic_ent(le->el_symptom_data,&p,LE_SYMPTOM_MAX);
	}
	else	/* buff || le == NULL */
		rc = 0;

	return(rc);
}


 /*
  * NAME:      invalid_le_static()
  * FUNCTION:  Validate an error log entry static data.  Static
  *            data in this case means nondynamic, i.e. no length
  *            associated with the member.  Print an error message
  *            reflecting which entry was invalidated.  Return after
  *            the first invalidation.
  *	RETURNS:
  *            0 - VALID
  *            1 - INVALID length
  *            2 - INVALID lengths not equal
  *            3 - INVALID magic
  *            4 - INVALID sequence
  *            6 - INVALID machineid
  *            7 - INVALID nodeid
  */

int
invalid_le_static(char *buf, int offset,struct le_bad *le_bad)
{
	struct log_entry *le;

	int	rc;
	int length2;

	le = (struct log_entry *)buf;

	length2 = *(int *)(buf + le->le_length - sizeof(unsigned));

	if (!in_range(LE_MIN_SIZE,le->le_length,LE_MAX_SIZE)) {		/* length */
		le_bad->value = le->le_length;
		le_bad->offset = offset;
		le_bad->code = LE_BAD_LEN1;
		rc = LE_BAD_LEN1;
	}
	else if (le->le_length != length2) {			/* length_2 */
		le_bad->value = le->le_length;
		le_bad->value2 = length2;
		le_bad->offset = offset;
		le_bad->code = LE_BAD_LEN2;
		rc = LE_BAD_LEN2;
	}
	else if(le->le_magic != LE_MAGIC) {		/* magic */
		le_bad->value = le->le_magic;
		le_bad->offset = offset;
		le_bad->code = LE_BAD_MAGIC;
		rc = LE_BAD_MAGIC;
	}
	else if(!in_range(1,le->le_sequence,INT_MAX)) {		/* sequence_id */
		le_bad->value = le->le_sequence;
		le_bad->offset = offset;
		le_bad->code = LE_BAD_SEQ;
		rc = LE_BAD_SEQ;
	}
	else if(!val_name(&(le->le_machine_id),LE_MACHINE_ID_MAX)) { /* machine_id */
		le_bad->value = LE_BAD_MACH_ID;
		le_bad->offset = offset;
		le_bad->code = LE_BAD_MACH_ID;
		rc = LE_BAD_MACH_ID;
	}
	else if(!val_name(&(le->le_node_id),LE_NODE_ID_MAX)) { /* node_id */
		le_bad->value = LE_BAD_NODE_ID;
		le_bad->offset = offset;
		le_bad->code = LE_BAD_NODE_ID;
		rc = LE_BAD_NODE_ID;
	}
	else {
		le_bad->value = 0;
		le_bad->value2 = 0;
		le_bad->offset = 0;
		le_bad->code = 0;
		rc = 0;
	}

	return(rc);
}


 /*
  * NAME:      le_bad_msg()
  * FUNCTION:  Send out an error message about a bad log entry.
  * RETURNS:   NONE
  */

void
le_bad_msg(struct le_bad *le_bad)
{
	switch (le_bad->code) {
	case LE_BAD_LEN1:		 /* length */
		cat_eprint(CAT_LOG_BAD_SIZE,
			"%s: invalid error log entry length: %d at offset: %d\n",
			Progname,le_bad->value,le_bad->offset); 
		cat_print(CAT_LOG_CANT_PROCESS, "Unable to process error log file %s.\n",Logfile);
			break;
	case LE_BAD_LEN2: 	/* length_2 */
		cat_eprint(CAT_LOG_BAD_LENGTHS,
			"%s: invalid error log entry lengths: length: %d length2: %d at offset: %d\n",
			Progname,le_bad->value,le_bad->value2,le_bad->offset); 
		cat_print(CAT_LOG_CANT_PROCESS, "Unable to process error log file %s.\n",Logfile);
		break;
	case LE_BAD_MAGIC:		/* magic */
		cat_warn(CAT_LOG_BAD_MAGIC,
			"%s: invalid error log entry magic: %d at offset: %d\n",
			Progname,le_bad->value,le_bad->offset); 
		break;
	case LE_BAD_SEQ:		/* sequence_id */
		cat_warn(CAT_LOG_BAD_SEQ,
			"%s: invalid error log entry Sequence Number: %d at offset: %d\n",
			Progname,le_bad->value,le_bad->offset); 
		break;
	case LE_BAD_MACH_ID:		/* machine id */
		cat_warn(CAT_LOG_BAD_MACH,
			"%s: invalid error log entry Machine Id at offset: %d\n",
			Progname,le_bad->offset); 
		break;
	case LE_BAD_NODE_ID:		/* node id */
		cat_warn(CAT_LOG_BAD_NODE
			,"%s: invalid error log entry Node Id at offset: %d\n",
			Progname,le_bad->offset); 
		break;
	default:
		cat_warn(CAT_LE_BAD_CODE,
			"%s: invalid log entry bad code: %d value: %d offset: %d\n",
			Progname,le_bad->code,le_bad->value,le_bad->offset); 
		break;
	}
}
