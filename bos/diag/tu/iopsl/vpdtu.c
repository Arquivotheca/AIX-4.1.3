static char sccsid[] = "@(#)43  1.2.2.1  src/bos/diag/tu/iopsl/vpdtu.c, tu_iopsl, bos411, 9428A410j 7/8/93 08:58:30";
/*
 *   COMPONENT_NAME: TU_IOPSL
 *
 *   FUNCTIONS: *               check_crc
 *              check_fields
 *              check_tm
 *              check_total_length
 *              dump_vpd
 *              field_match
 *              get_data
 *              get_machine_model_tu
 *              vpd_tu
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


/* The following tests are performed on VPD :
*
*  1. Presence of the following ASCII fields :
*     "VPD", "TM", "PN" (planar P/N), "EC", "PI", "FN",
*     "MF", "PN" (ROS P/N).
*  2. CRC.
*  3. First ASCII delimiter ("*").
*
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <ctype.h>

#include <diag/atu.h>
#include "misc.h"
#include "salioaddr.h"

#include <sys/iplcb.h>
#include <sys/rosinfo.h>

#define VPD_SIZE     255  /* in bytes - (for RSC products)   */
#define VPD_START    1    /* first valid VPD address   */
#define VPD_DELIM    '*'  /* field delimiter           */
#define EOS          '\0' /* End Of String             */
#define STRING_MATCH 0

/* fields tested :  */
enum { VPD_FLD = 0, TM_FLD, PPN_FLD, EC_FLD, PI_FLD,
	 FN_FLD, MF_FLD, RPN_FLD, VPD_FIELDS
      };

#define SAL_MACHINE_TYPE  "7011"   /* Sal */
#define CAB_MACHINE_TYPE  "7008"   /* Cab */
#define VPD_ADDR      POS6_REG
#define VPD_DATA      POS3_REG

typedef struct
{
  uchar_t id[3];
  uchar_t length[2];
  uchar_t crc[2];
} VPD_HEADER;

static struct
{
    uchar_t data[2048];         /* Allocate space to store VPD info */
    uchar_t offset[VPD_FIELDS]; /* points to field id */

} vpd;

static struct
{
  uchar_t *id;
  int err;
} vpd_fld[VPD_FIELDS] = 
              { "VPD", VPD_NOT_FOUND_ERROR,
		"TM" , TM_NOT_FOUND_ERROR,
		"PN" , PPN_NOT_FOUND_ERROR,
		"EC" , EC_NOT_FOUND_ERROR,
		"PI" , PI_NOT_FOUND_ERROR,
		"FN" , FN_NOT_FOUND_ERROR,
		"MF" , MF_NOT_FOUND_ERROR,
		"PN" , RPN_NOT_FOUND_ERROR,
	      }; 

static int get_data_power(int, uchar_t []);
static int get_data_rsc(int, int, uchar_t []);
static int check_crc(uchar_t []);
static int check_fields(uchar_t [], uchar_t []);
static int check_tm(int, uchar_t []);
static void dump_vpd(uchar_t []);
static BOOL field_match(uchar_t[], uchar_t []);

int vpd_tu(int fd, TUCB *pt)
{
  int rc;

  rc = SUCCESS;

  if(rc == SUCCESS) {
    if (power_flag) {
       rc = get_data_power(fd, &vpd.data[0]);  
    }
    else {
       rc = get_data_rsc(fd, VPD_SIZE, &vpd.data[0]);  
    }
  }

  if(rc == SUCCESS)
    rc = check_crc(&vpd.data[0]);
      
  if(rc == SUCCESS)
  {
    if(vpd.data[sizeof(VPD_HEADER)] != VPD_DELIM)
      rc = DELIM_NOT_FOUND_ERROR;
  }

  if(rc == SUCCESS)
    rc = check_fields(&vpd.data[0], &vpd.offset[0]);

  if(rc == SUCCESS)
    rc = check_tm(fd, &vpd.data[vpd.offset[TM_FLD] + 1]);

  return(rc);
}

/* Get VPD data */

/* This routine is called if using machine type POWER */

static int get_data_power(int fd, uchar_t vpd_data[])
{
  MACH_DD_IO    mdd;
  IPL_DIRECTORY *iplcb_dir;     /* iplcb.h */
  int rc;

  rc = SUCCESS;

   /* First obtain pointer to IPL ROS directory */
    iplcb_dir = (IPL_DIRECTORY *) malloc (sizeof(IPL_DIRECTORY));
    mdd.md_incr = MV_BYTE;
    mdd.md_addr = 128;
    mdd.md_data = (char *) iplcb_dir;
    mdd.md_size = sizeof(*iplcb_dir);
    if ( ioctl(fd, MIOIPLCB, &mdd) ) 
        rc = IO_ERROR;
    else {
	/* Copy VPD info into vpd_data array */
        mdd.md_incr = MV_BYTE;
        mdd.md_addr = iplcb_dir->system_vpd_offset+1;
        mdd.md_data = (char *) vpd_data;
        mdd.md_size = iplcb_dir->system_vpd_size;
        if ( ioctl(fd, MIOIPLCB, &mdd) ) 
           rc = IO_ERROR; 
	}

  return(rc);
}

/* This routine is called if using machine type RSC */

static int get_data_rsc(int fd, int size, uchar_t vpd_data[])
{
  int i, rc;
  uchar_t vpd_addr;

  rc = SUCCESS;

  for(i = 0, vpd_addr = VPD_START; i < size; i++, vpd_addr++)
  {
    rc = put_byte(fd, vpd_addr, VPD_ADDR, MIOCCPUT);

    if(rc == SUCCESS)
      rc = getbyte(fd, &vpd_data[i], VPD_DATA, MIOCCGET);

    if(rc != SUCCESS)
      break;
  }

  return(rc);
}


static void dump_vpd(uchar_t data[])
{
  int i;

  for(i = 0; i < VPD_SIZE; i++)
  {
    printf("\n(%02x) -- %02x", i, data[i]);

    if(isascii(data[i]))
      printf("  %c", data[i]);
  }

  return;
}

static int check_crc(uchar_t vpd_data[])
{
  int rc, calc_crc;
  VPD_HEADER *hdr;

  union
  {
    ushort_t whole;

    struct
    {
      uchar_t msb, lsb;
    } byte;
  } vpd_crc;

  rc = SUCCESS;
  hdr = (VPD_HEADER *) &vpd_data[0];
  vpd_crc.byte.msb = hdr->crc[0];
  vpd_crc.byte.lsb = hdr->crc[1];
  /* The second argument in the following call is merely the entire header 
     length value multiplied by 2. */
  calc_crc = crc_gen(&vpd_data[sizeof(VPD_HEADER)], 2 * (*(ushort *)(&hdr->length[0])));

  if(vpd_crc.whole != calc_crc)
    rc = CRC_ERROR;

  return(rc);
}


/* Test if all fields in vpd_fld[] are present */

static int check_fields(uchar_t vpd_data[], uchar_t vpd_off[])
{
  int i, rc, fld;
  BOOL found[VPD_FIELDS];
  uchar_t offset;

  rc = SUCCESS;
  memset(&found[0], FALSE, sizeof(found));

/* check if "VPD" field is present */

  if(!field_match(&vpd_data[0], vpd_fld[VPD_FLD].id))
    rc = vpd_fld[VPD_FLD].err;
  else
    found[VPD_FLD] = TRUE;

  for(i = sizeof(VPD_HEADER); i < VPD_SIZE && rc == SUCCESS;)
  {
    if(vpd_data[i] == VPD_DELIM)
    {
      i++;          /* skip delimiter */
      offset = i;
      
      for(fld = 0; fld < VPD_FIELDS; fld++)
      {
	if(found[fld])
	  continue;
      
 	if(field_match(&vpd_data[i], vpd_fld[fld].id))
	{
	  found[fld] = TRUE;
	  vpd_off[fld] = offset;
	  i += strlen(vpd_fld[fld].id);
	  break;
	}
      }
    }
    else
      i++;
  }

  for(fld = 0; fld < VPD_FIELDS; fld++)
  {
    if(!found[fld])
    {
      rc = vpd_fld[fld].err;
      break;
    }
  }

  return(rc);
}



static BOOL field_match(uchar_t vpd_id[], uchar_t ref_id[])
{
  uchar_t id[10];

  memcpy(id, &vpd_id[0], strlen(ref_id));
  id[strlen(ref_id)] = EOS;

  return((strcmp(ref_id, id) == STRING_MATCH) ? TRUE : FALSE);
}
  

/* Check machine type  */

static int check_tm(int fd, uchar_t tm_fld[])
{
  int rc, type;
  uchar_t id[10];

  rc = TM_ERROR;

  memcpy(id, &tm_fld[strlen(vpd_fld[TM_FLD].id)], 8);
  id[8] = EOS;

  /* If the TM field does not contain just zeros, we will consider it OK */
  if (((strcmp(id, "00000000")) != 0) && ((strcmp(id, "")) != 0))
     rc = SUCCESS;
 
  return(rc);
}

