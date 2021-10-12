static char sccsid[] = "@(#)55  1.1.1.3  src/bos/diag/tu/ethsal/vpdtu.c, tu_ethsal, bos411, 9428A410j 10/20/93 14:14:59";
/*
 *   COMPONENT_NAME: tu_ethsal
 *
 *   FUNCTIONS: vpd_tu, get_data_power, get_data_rsc, check_crc, find_field,
 *		field_match, check_network_address, get_network_address,
 *		find_network_address
 *
 *   ORIGINS: 27
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
*     "VPD",
*  2. CRC.
*  3. First ASCII delimiter ("*").
*
*/

#include <sys/types.h>
#include <sys/mdio.h>
#include <stdio.h>
#include <string.h>

#include <sys/rosinfo.h>
#include <sys/iplcb.h>

#include "exectu.h"
#include "tu_type.h"


#define VPD_SIZE      255  /* in bytes                  */
#define VPD_START     1    /* first valid VPD address   */
#define VPD_DELIM     '*'  /* field delimiter           */
#define EOS           '\0' /* End Of String             */
#define STRING_MATCH  0
#define VPD_ETHER_MSG "ETHERNET"


typedef struct
{
  uchar_t id[3];
  uchar_t length[2];
  uchar_t crc[2];
} VPD_HEADER;

uchar_t vpd_data[2048];   /* This should be enough space */
BOOL vpd_done = FALSE;

static struct  /* to store network address */
{
  BOOL done;
  uchar_t buff[NETW_ADDR_LENGTH];

} naddr = { FALSE };


static int get_data_power(int, uchar_t []);
static int get_data_rsc(int, int, uchar_t []);
static int check_crc(uchar_t []);
static BOOL find_field(uchar_t *, uchar_t *, int, int *);
static BOOL field_match(uchar_t [], uchar_t []);
static int check_network_address(uchar_t []);
static int get_network_address(uchar_t []);
static int find_network_address(uchar_t [], uchar_t []);

/*
 * NAME : vpd_tu
 *
 * DESCRIPTION :
 *
 *  VPD test unit. The following tests are performed :
 *
 *  1. Verifies CRC.
 *  2. Checks for the presence of the first VPD delimiter.
 *  3. Verifies that the field 'VPD' is present.
 *  4. Verifies that Ethernet network address is present.
 *  5. Verifies that the multicast bit is not on.
 *
 * INPUT :
 *
 *   None.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

int vpd_tu(void)
{
  int rc, id_addr;

  rc = SUCCESS;

  if(rc == SUCCESS) {
    if (power_flag) {
       rc = get_data_power(get_fd(MDD_FD), &vpd_data[0]);
    }
    else {
       rc = get_data_rsc(get_fd(MDD_FD), VPD_SIZE, &vpd_data[0]);
    }
  }

  if(rc == SUCCESS) {
    vpd_done = TRUE;
    rc = check_crc(&vpd_data[0]);
  }

  if(rc == SUCCESS)
  {
    if(vpd_data[sizeof(VPD_HEADER)] != VPD_DELIM)
      rc = VPD_DELIM_ERR;
  }

  if(rc == SUCCESS)
  {
    id_addr = VPD_SIZE;

    if(!find_field((uchar_t *) "VPD", &vpd_data[0], 0, &id_addr) ||
       id_addr != 0)
      rc = VPD_NOT_FOUND_ERR;
  }

  if(rc == SUCCESS)
  {
    rc = find_network_address(&vpd_data[0], &naddr.buff[0]);

    if(rc == SUCCESS)
      rc = check_network_address(&naddr.buff[0]);

    if(rc == SUCCESS)
      naddr.done = TRUE;
  }

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
        rc = READ_ERR;
    else {
        /* Copy VPD info into vpd_data array */
        mdd.md_incr = MV_BYTE;
        mdd.md_addr = iplcb_dir->system_vpd_offset+1;
        mdd.md_data = (char *) vpd_data;
        mdd.md_size = iplcb_dir->system_vpd_size;
        if ( ioctl(fd, MIOIPLCB, &mdd) )
           rc = READ_ERR;
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
    rc = put_byte(fd, vpd_addr, (ulong_t) VPD_ADDR_REG_ADDR , MIOCCPUT);

    if(rc == SUCCESS)
      rc = Get_byte(fd, &vpd_data[i], (ulong_t) VPD_DATA_REG_ADDR, MIOCCGET);

    if(rc != SUCCESS)
      break;
  }

  return(rc);
}


#ifdef DEBUG_ETHER
static void dump_vpd(uchar_t data[])
{
  int i;
  char msg [80];

  for(i = 0; i < VPD_SIZE; i++)
  {
    sprintf(msg, "i = (0x%02x)      data [i] = 0x%02x", i, data[i]);
    DEBUG_MSG(msg);

    if(isascii(data[i]))
    {
      sprintf(msg, "ascii data = %c", data[i]);
      DEBUG_MSG(msg);
    }
  }

  return;
}
#endif


/*
 * NAME : check_crc
 *
 * DESCRIPTION :
 *
 *  Verifies VPD CRC.
 *
 * INPUT :
 *
 *   1. VPD data.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

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

/* recalculate CRC. Note that CRC is calculated only on valid VPD */
/* data, not on the whole VPD space ...                           */

  calc_crc = crc_gen(&vpd_data[sizeof(VPD_HEADER)], 2 * (*(ushort *)(&hdr->length[0])));

  if(vpd_crc.whole != calc_crc)
    rc = VPD_CRC_ERR;

  return(rc);
}



/*
 * NAME : find_field
 *
 * DESCRIPTION :
 *
 *  Finds input field in VPD data.
 *
 * INPUT :
 *
 *   1. Field to be found in VPD data.
 *   2. VPD data.
 *   3. Start search address.
 *   4. Address where field is found.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   TRUE if field found, else FALSE.
 *
*/

static BOOL find_field(uchar_t id[], uchar_t vpd_data[], int start_addr, int *found_addr)
{
  int i;
  BOOL found, delim;

  for(i = start_addr, delim = TRUE, found = FALSE; i < VPD_SIZE; i++)
  {
    if(delim && field_match(&vpd_data[i], &id[0]))
    {
      found = TRUE;
      *found_addr = i;
      break;
    }

    delim = (vpd_data[i] == VPD_DELIM) ? TRUE : FALSE;
  }

  return(found);
}



/*
 * NAME : field_match
 *
 * DESCRIPTION :
 *
 *  Compares a VPD field with reference field.
 *
 * INPUT :
 *
 *   1. VPD field.
 *   2. Reference field.
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   TRUE if fields compare, else FALSE.
 *
*/

static BOOL field_match(uchar_t vpd_id[], uchar_t ref_id[])
{
  uchar_t id[10];

  memcpy(&id[0], &vpd_id[0], strlen((char *) ref_id));
  id[strlen((char *) ref_id)] = EOS;

  return((strcmp((char *) ref_id, (char *) id) == STRING_MATCH) ? TRUE : FALSE);
}



/*
 * NAME : check_network_address
 *
 * DESCRIPTION :
 *
 *  Verifies that Ethernet network's address multicast bit is not on
 *
 * INPUT :
 *
 *   1. Ethernet network address
 *
 * OUTPUT :
 *
 *   None.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/


static int check_network_address(uchar_t addr[])
{
  int rc;

  /* check that multicast bit is not on in the VPD Ethernet hardware address.*/
  /* This is a requirement for the IEEE Ethernet transceiver.                */

  rc = ((addr[0] & MULTICAST_BIT) != 0) ? ETHER_NADDR_MULTICAST_ERR : SUCCESS;

  return(rc);
}



/*
 * NAME : find_network_address
 *
 * DESCRIPTION :
 *
 *  Looks for Ethernet network address in VPD data
 *
 * INPUT :
 *
 *   1. VPD data.
 *   2. Buffer where to store network address, if found.
 *
 * OUTPUT :
 *
 *   Network address, if found.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

static int find_network_address(uchar_t vpd_data[], uchar_t net_addr[])
{
  int rc, idx;
  char *ether_msg = VPD_ETHER_MSG;

  rc = ETHER_VPD_MSG_ERR;
  idx = sizeof(VPD_HEADER);

  while(rc != SUCCESS && idx < VPD_SIZE &&
        find_field((uchar_t *) "DS", &vpd_data[0], idx, &idx))
  {

    idx += (strlen("DS") + 1);  /* skip 'DS' and length bytes */

    if(field_match(&vpd_data[idx], (uchar_t *) ether_msg))
    {
      idx += (strlen(ether_msg) + 1); /* skip msg. */

      if(field_match(&vpd_data[idx], (uchar_t *) "NA"))
      {
        idx += (strlen("NA") + 1);  /* skip 'NA' and length field */
        memcpy((void *) &net_addr[0], (void *) &vpd_data[idx], NETW_ADDR_LENGTH);
        rc = SUCCESS;
      }
      else
        rc = ETHER_NADDR_NOT_FOUND_ERR;
    }
  }

  return(rc);
}



/*
 * NAME : get_network_address
 *
 * DESCRIPTION :
 *
 *  Retrieves Ethernet network address.
 *
 * INPUT :
 *
 *   1. Buffer to store network address.
 *
 * OUTPUT :
 *
 *   Network address, if successful.
 *
 * RETURNS :
 *
 *   Error code or SUCCESS.
 *
*/

static int get_network_address(uchar_t addr[])
{
  int rc, offset;

  rc = SUCCESS;

  if(!naddr.done)
    rc = vpd_tu();  /* this will validate VPD and get address */

  if(rc == SUCCESS)
  {
    if(naddr.done)
      memcpy((void *) &addr[0], (void *) &naddr.buff[0], NETW_ADDR_LENGTH);
    else
      rc = ETHER_NADDR_NOT_FOUND_ERR;
  }

  return(rc);
}



#ifdef DEBUG_ETHER
int naddr_tu(void)
{
  int rc, i;
  uchar_t addr[NETW_ADDR_LENGTH];
  char msg[80];

  memset(&addr[0], 0, sizeof(addr));
  rc = get_network_address(&addr[0]);

  sprintf(msg, "address = 0x%s ", addr);
  DEBUG_MSG(msg);

  return (rc);
}


int dump_vpd_tu(void)
{
  int rc;

  rc = SUCCESS;

  if(!vpd_done)
    rc = vpd_tu();  /* this will get VPD data */

    dump_vpd(&vpd_data[0]);

  return(rc);
}
#endif



