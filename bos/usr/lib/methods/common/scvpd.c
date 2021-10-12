static char sccsid[] = "@(#)28  1.5  src/bos/usr/lib/methods/common/scvpd.c, cfgmethods, bos411, 9432A411a 8/5/94 11:43:52";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: get_vpd_value
 *		get_inquiry_data
 *		get_tokens
 *		scvpd
 *		get_sid_lun
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <errno.h>
#include <sys/scsi.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/bootrecord.h>
#include <cf.h>
#include "cfghscsi.h"
#include "cfgdebug.h"

#define INQ_SIZE      255
/*
 *---------------------------------------------------------------
 * The following defines are used in get_vpd_value to identify
 * the offsets to place the various parts of a formatted VPD
 * value.
 *---------------------------------------------------------------
 */
#define STAR_OFFSET    0
#define KEYWORD_OFFSET 1
#define LENGTH_OFFSET  3
#define DATA_OFFSET    4
#define VPD_OVERHEAD   DATA_OFFSET

/*
 *---------------------------------------------------------
 * Forward function definitions
 *---------------------------------------------------------
 */
static void get_vpd_value(uchar*, uchar*, char*, short*, char) ;
int    get_inquiry_data(struct CuDv*, int, uchar*);
static int get_tokens(char*, char*, short*, short*, int*, int*, char*) ;

int    get_sid_lun( char*, uchar*, uchar* );
/*
 ************************************************************************
 * NAME     : SCSI VPD (scvpd)
 * FUNCTION : Get SCSI inquiry data, parse it out into IBM VPD
 *            format, and then save that in CuVPD
 * EXECUTION ENVIRONMENT : This routine is to be run from with SCSI
 *            device configure methods only.
 * NOTES :
 ************************************************************************
 */
int
scvpd(struct CuDv       *cudv,          /* Ptr to cudv of dvc being config'd */
      char              *vpd_str,       /* ptr to vpd str               */
      char              *def_format)    /* default VPD format string    */
{
    uchar       inq_data[INQ_SIZE] ;    /* space for inquiry data fr dvc*/
    int         inq_len       ;         /* len of inquiry data          */

    uchar       temp_buf[INQ_SIZE] ;    /* temp buffer so can calc len  */
    char        *vpd_end  ;             /* ptr to next free byte.       */
    ushort      vpd_len   ;             /* current len of formatted VPD */

    char        sstr[64]  ;             /* search string for ODM queries*/
    char        keyword[3];             /* VPD keyword.                 */
    char        more      ;             /* Loop control flag.           */
    int         code_page ;             /* inq code page value is in    */
    int         prev_page ;             /* code page of previous value  */
    char        dtype     ;             /* data type of the VPD field   */
    short       offset    ;             /* offset of value in inq data. */
    short       len       ;             /* len of value in inq data     */

    int         rc        ;             /* Rtn code from ODM routines   */
    struct PdAt attr      ;             /* PdAt for type 'V' format attr*/

    char        *format_str ;           /* Ptr to format str in use     */

    /* BEGIN scvpd */

    inq_len   = 0 ;
    prev_page = NO_PAGE ;
    code_page = 0;                      /* initialize for first inq loop*/
    vpd_len = 0   ;
    vpd_end = vpd_str;			/* Ptr to last byte written     */

   /*
    *----------------------------------------------------------------------
    * Get the 1st instance of a type='V' attribute for this device.
    * Then go into a loop so that we can handle multiple instances
    * of the type='V' attribute (loop exits when ODM returns 0 - no
    * objects returned).
    *----------------------------------------------------------------------
    */
    sprintf(sstr, "uniquetype=%s and type=V", cudv->PdDvLn_Lvalue) ;
    rc = (int)odm_get_first(PdAt_CLASS, sstr, &attr) ;
   
    do
    {
       /*
        *--------------------------------------------------------------
        * Set format_str to appropriate format string based on
        * results of getting the V attribute.
        *--------------------------------------------------------------
        */
        format_str = ((rc == 0) || (rc == -1)) ? def_format : attr.deflt ;

        more = (char)get_tokens(format_str, keyword, &offset,
                                &len, &code_page, &prev_page, &dtype) ;
        while (more)
        {
           /*
            *--------------------------------------------------------------
            * Must get inquiry data from the device if the code page
            * is different from the last code page specified in the
            * format string.
            * The initial pass, the prev_page is set to be different.
            *--------------------------------------------------------------
            */
            if (prev_page != code_page) 
            {
                inq_len = get_inquiry_data(cudv, code_page, inq_data) ;
            }
           /*
            *--------------------------------------------------------------
            * If there was an error getting inquiry data from the
            * device, inq_len will be zero; therefore, must check
            * to be sure we have inquiry data to work with.
            *--------------------------------------------------------------
            */
	    if ((offset >= 0) && ((offset + len) <= inq_len))
            {
		
                get_vpd_value(&temp_buf, inq_data + offset,
                              keyword, &len, dtype ) ;
		if ( (vpd_len + len + VPD_OVERHEAD) > VPDSIZE ) {
			/*
                         *--------------------------------------------
			 * don't have enough room for any more VPD in
			 * buffer for this keyword and the following
			 * words-- return length 
                         *--------------------------------------------
	                 */
			return (vpd_len);
		}
		else {
			bcopy( &temp_buf, vpd_end, len+VPD_OVERHEAD);
			vpd_len += len + VPD_OVERHEAD;
			vpd_end += len + VPD_OVERHEAD;
		}
            }
           /*
            *--------------------------------------------------------------
            * Get the next keyword tokens.
            * If no more tokens in this string, check the "values" field.
            * If we are already processing the "values" field, then
            * we are done with this attribute.
            *--------------------------------------------------------------
            */
            if (!get_tokens(NULL,keyword, &offset, &len,&code_page,
                            &prev_page, &dtype))
            {
                if (format_str == attr.deflt)
                {
                    format_str = attr.values ;
                    more = (char)get_tokens(format_str, keyword, &offset,
                                            &len, &code_page, &prev_page,
                                            &dtype ) ;
                }
                else
                {
                    more = FALSE ;
                }
            }
        } /* END while */

        if (format_str != def_format)
        {
           /*
            *--------------------------------------------------------
            * Check for more "type=V" attributes for this device -
            * ONLY if not using the default format string.
            *--------------------------------------------------------
            */
            rc = (int)odm_get_next(PdAt_CLASS, &attr) ;
        }
    } while ((rc != 0) && (rc != -1)) ;

    return(vpd_len) ;
} /* END scvpd */

/*
 ************************************************************************
 * NAME     : get_vpd_value
 * FUNCTION : Adds indicated inquiry data to formated VPD string with
 *            given keyword prefix.
 * EXECUTION ENVIRONMENT : This routine is with in this module only.
 * NOTES :
 * RETURNS  :
 *            Length of the formated value just added.
 ************************************************************************
 */
static void
get_vpd_value(uchar     *vpd     ,      /* addr at which to put VPD value*/
              uchar     *inq_data,      /* addr of inquiry data to add  */
              char      *keyword ,      /* keyword prefix to use        */
              short     *len,           /* length of inquiry data to add*/
              char      dtype)          /* data type                    */
{

    int       i;
    int       cnt=0;
    int       vpd_offset=0;
    char      hex_digits[] = "0123456789ABCDEF";

    /* BEGIN get_vpd_value */

    bzero(vpd, INQ_SIZE);

   /*
    *-------------------------------------------------------------
    * Set the VPD value "header".  Has the format of "*KKL"
    * where
    *   *  = character '*'
    *   KK = 2 character VPD keyword
    *   L  = 1 byte length in shorts (16 bit words).
    *
    * Then copy the inquiry data in place after the 'L'.
    *-------------------------------------------------------------
    */

    vpd[STAR_OFFSET] = '*' ;
    bcopy(keyword, &vpd[KEYWORD_OFFSET], 2)  ;

    /*
    *-------------------------------------------------------------
    *  Check the data type of the keyword field.  If it is hex 'X',
    *  change each nibble to get the ascii equivalent and increment
    *  the length.
    *-------------------------------------------------------------
    */

    if( dtype == 'X' || dtype == 'x' ) {
      for ( i=0; i< *len; i++) {

         vpd[DATA_OFFSET + vpd_offset++] = hex_digits[(inq_data[i] & 0xf0 )
                                           >> 4] ;
         vpd[DATA_OFFSET + vpd_offset++] = hex_digits[(inq_data[i] & 0x0f )];
         cnt++ ;
      }  /* end for */
      *len += cnt;                   /* increment the total length         */

    } /* end if */
    else {                           /* dtype is already ascii, no convert */

       bcopy(inq_data, &vpd[DATA_OFFSET], *len) ;

    } /* end else */

    if ( *len%2 != 0 ) {                    /* if len is odd        */
        vpd[DATA_OFFSET + *len] = 0x00;     /* pad with a null      */
        (*len)++;                           /* take one more byte   */
    }

    vpd[LENGTH_OFFSET] = (uchar)((*len +VPD_OVERHEAD) >> 1) ;

} /* END get_vpd_value */

/*
 ************************************************************************
 * NAME     : Get Inquiry Data
 * FUNCTION : Set up and request the adapter for inquiry data.
 * EXECUTION ENVIRONMENT : This routine is to be run from with SCSI
 *            device configure methods only.
 * NOTES :
 * RETURNS  :
 *            Length of data obtained.
 *            Zero indicates failure.
 ************************************************************************
 */
int
get_inquiry_data(struct CuDv *cudv ,    /* Dvc's customized object      */
                 int    code_page,      /* code page of inq data to get */
                 uchar* inq_data)       /* Rtn addr of ptr to inq bfr   */
{

    uchar       sid ;                   /* SCSI ID of device.           */
    uchar       lun ;                   /* LUN of device (usually zero) */
    int         len ;                   /* length of data returned.     */

    int         fd  ;                   /* file desc for scsi adptr     */
    int         rc  ;
    char        adp_name[32] ;          /* spec file name of scsi adptr */
    int 	error;			/* Error from issue_scsi_inquiry*/

/* BEGIN get_inquiry_data */

    len = strtoul(cudv->connwhere, NULL, 16) ;
    sid = (len >> 4) & 0x0f ;
    lun = len & 0x0f ;

    sid = strtoul(cudv->connwhere,NULL,10);
    lun = strtoul(strchr(cudv->connwhere,',')+1,NULL,10);

    sprintf(adp_name, "/dev/%s", cudv->parent) ;
    if ((fd = open(adp_name, O_RDWR)) >= 0)
    {
        if (ioctl(fd, SCIOSTART, IDLUN((int)sid, (int)lun)) == 0)
        {
            /* now get inquiry data */
            if ((rc = issue_scsi_inquiry(fd,inq_data,sid,lun,code_page,&error)) == 0)
            {
                len = inq_data[4] + 5 ; /* length of inq data is in position
                                           4; add 5 to it to get total len
                                           of inq data in the buffer       */
#ifdef CFGDEBUG
                /* look at inquiry data */
                hexdump(inq_data, (long)len);
#endif
            }
            else /* error getting inquiry data from drive */
            {
                DEBUG_0("get_inq_data: error issueing inquiry; ") ;
                DEBUG_4("rc=%d adap=%s sid=%d lun=%d\n",
                        rc, adp_name, sid, lun) ;
                len = 0 ;
            }

            if (ioctl(fd, SCIOSTOP, IDLUN((int)sid, (int)lun)) < 0)
            {
                DEBUG_2("Failed to SCIOSTOP device at scsi %d lun %d\n",
                (int)sid, (int)lun) ;
            }
        }
        else /* Error starting device */
        {
            DEBUG_3(
                "get_inq_data: Failed SCIOSTART for sid %d lun %d;errno=%d\n",
                sid,lun,errno) ;
            len = 0 ;
        }
        close(fd) ;
    }
    else /* error opening adapter */
    {
        DEBUG_2("get_inq_data: error opening %s, errno=%d\n", adp_name, errno) ;
        len = 0 ;
    }
    return(len) ;
} /* END get_inquiry_data */

/*
 ***********************************************************************
 * NAME     : get_tokens
 * FUNCTION : break out the pieces of the VPD format string that
 *            is passed in.  (see NOTES for details for the format)
 * EXECUTION ENVIRONMENT : Should only be used as part of SCSI
 *            VPD processing.
 * NOTES    :
 *     -  The format of the format string is a comma seperated
 *        string of keyword location identifiers.  The format of
 *        of the keyword location identifier is 1 of 2 formats (the
 *        2 formats can be intermixed within the same string).
 *        a) KKOOLLD
 *             where
 *               KK is the 2 character keyword name to be associated
 *                  with the data.
 *               OO is the 2 hexadecimal digit offset into the inquiry data
 *                  where the VPD keyword value resides.
 *               LL is the 2 hexadecimal digit length of the keyword value
 *                  in the inquiry data.  This length is in bytes.
 *               D  is the 1 character data type, C for char ASCII, X hex.
 *                  This will be used for conversion.
 *        b) KKOOLLPPD
 *             where
 *               KK, OO, and LL are as defined in 'a)' above.
 *               PP is the 2 hexadecimal digit inquiry code page that
 *                  contains this VPD keyword value.  If this field is
 *                  NOT present, it is assumed that the value is in
 *                  the default inquiry data.
 *               D  is the 1 character data type, C for char ASCII, X hex.
 *                  This will be used for conversion.
 ***********************************************************************
 */
static int
get_tokens(char   *format_str,          /* the format string            */
           char   *keyword,             /* return var for keyword name  */
           short  *offset ,             /* return var for offset        */
           short  *len    ,             /* return var for len of value  */
           int    *page   ,             /* return var for code page     */
           int    *prev_page,           /* return var for previous c page*/
           char   *dtype)               /* return var for string data typ*/
{
    char   *loc_id;			/* beginning of this keyword     */
    int    rc    ;
    char   part[3];
	 

    /* BEGIN get_tokens */

    bzero(part, sizeof(part) );

    loc_id = strtok(format_str, ", ") ;   /* search for next , or blank */

    if (loc_id != NULL)
    {
                                        /* get the C or X data type   */
        *dtype = loc_id[strlen(loc_id) -1] ; 
	strncpy(keyword, loc_id, 2);
	*offset = (short)strtol(strncpy(part,loc_id+2,2),NULL,16);

	*len =    (short)strtol(strncpy(part,loc_id+4,2),NULL,16);

        *prev_page = *page ;            /* save previous value     */
                                        /* to know when to switch  */
 	if (strlen(loc_id) > 7 ) {      /* there is code page info */
		*page = (int)strtol(strncpy(part,loc_id+6,2), NULL,16);
	}
	else {
		*page = NO_PAGE;
	}
        rc = TRUE ;
    }
    else
    {
        rc = FALSE ;
    }
    return(rc) ;
} /* END get_tokens */

/*
 * NAME: get_sid_lun
 *
 * FUNCTION: Extracts the sid, and lun from a SCSI address
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables 
 *    are assumed.
 * 2. The basic function is to convert the input string
 *    from the form sss-lll to uchar values (i.e. the sid, and the lun )
 *    sss  = 1 to 3 decimal characters for the sid
 *    lll  = 1 to 3 decimal characters for the lun
 *
 * RETURNS:
 * 0 for success, -1 for failure.
 *
 */
int get_sid_lun( scsiaddr, sid_addr, lun_addr )
char    *scsiaddr;
uchar   *sid_addr;
uchar   *lun_addr;
{

        if (*scsiaddr == '\0') return -1;
        if ( strchr(scsiaddr,',') == NULL) return -1;

/* We utilize the behavior of strtoul which stops converting characters at
   the first non-base character Thus after the start position is set, the
   conversion stops either at the ',' or at the NULL end-of-string */

        *sid_addr = (uchar)strtoul(scsiaddr,NULL,10);
        *lun_addr = (uchar)strtoul(strchr(scsiaddr,',')+1,NULL,10);

        return 0;
}



